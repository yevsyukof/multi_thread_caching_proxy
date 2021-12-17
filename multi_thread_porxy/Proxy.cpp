#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <iostream>
#include <netdb.h>
#include <cstring>
#include <pthread.h>

#include "Proxy.h"
#include "CurTime.h"


Proxy::Proxy(int listeningPort) : isInterrupt(false) {
    int reuseFlag = 1;
    sockaddr_in socketAddress{};
    socketAddress.sin_family = AF_INET;
    socketAddress.sin_addr.s_addr = INADDR_ANY;
    socketAddress.sin_port = htons(listeningPort);

    listeningSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(listeningSocketFd, SOL_SOCKET, SO_REUSEADDR, &reuseFlag, sizeof(reuseFlag));

    // биндим порт только на наш сокет
    if (bind(listeningSocketFd, (struct sockaddr *) &socketAddress, sizeof(socketAddress)) == -1) {
        std::cout << "--------Proxy(): BIND LISTENING SOCKET WRONG_REQUEST--------" << std::endl;
        exit(EXIT_FAILURE);
    }

    // помечаем сокет как слушающий
    if (listen(listeningSocketFd, SOMAXCONN) == -1) {
        std::cout << "--------Proxy(): LISTEN FAIL--------" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void *Proxy::handleNewServerConnection(void *arg) {
    std::shared_ptr<ServerConnection *> serverConnection =
            std::make_shared<ServerConnection *>(reinterpret_cast<ServerConnection *>(arg));

    (*serverConnection)->sendRequest();
    if ((*serverConnection)->getState() != ServerConnectionState::CONNECTION_ERROR) {
        (*serverConnection)->receiveAnswer();

        activeServersConnections.lock();
        {
            if ((*serverConnection)->getState() != ServerConnectionState::CONNECTION_ERROR
                    && (*serverConnection)->isCachingAnswerReceived()) {
                cacheStorage.lock();
                {
                    cacheStorage.addCacheEntry((*serverConnection)->getRequestUrl(),
                                               (*serverConnection)->getServerAnswerBuffer());
                }
                cacheStorage.unlock();
            }
            activeServersConnections.removeActiveServerConnectionFor((*serverConnection)->getRequestUrl());
        }
        activeServersConnections.unlock();
    } else {
        (*serverConnection)->handleConnectionError();
        activeServersConnections.lock();
        {
            activeServersConnections.removeActiveServerConnectionFor((*serverConnection)->getRequestUrl());
        }
        activeServersConnections.unlock();
    }

    (*serverConnection)->close();
    pthread_exit(reinterpret_cast<void *>(EXIT_SUCCESS));
}

void Proxy::handleArrivalOfClientRequest(const std::shared_ptr<ClientConnection *> &clientConnection) {
    if ((*clientConnection)->getState() == ClientConnectionStates::WRONG_REQUEST) {
        switch ((*clientConnection)->getError()) {
            case ClientRequestErrors::ERROR_400: {
                (*clientConnection)->setBuffer(std::make_shared<Buffer>(
                        Buffer(std::make_shared<std::vector<char>>(
                                std::vector<char>(ERROR_MESSAGE_400.begin(), ERROR_MESSAGE_400.end())))));
                break;
            }
            case ClientRequestErrors::ERROR_405: {
                (*clientConnection)->setBuffer(std::make_shared<Buffer>(
                        Buffer(std::make_shared<std::vector<char>>(
                                std::vector<char>(ERROR_MESSAGE_405.begin(), ERROR_MESSAGE_405.end())))));
                break;
            }
            case ClientRequestErrors::ERROR_500: {
                (*clientConnection)->setBuffer(std::make_shared<Buffer>(
                        Buffer(std::make_shared<std::vector<char>>(
                                std::vector<char>(ERROR_MESSAGE_500.begin(), ERROR_MESSAGE_500.end())))));
                break;
            }
            case ClientRequestErrors::ERROR_501: {
                (*clientConnection)->setBuffer(std::make_shared<Buffer>(
                        Buffer(std::make_shared<std::vector<char>>(
                                std::vector<char>(ERROR_MESSAGE_501.begin(), ERROR_MESSAGE_501.end())))));
                break;
            }
            case ClientRequestErrors::ERROR_504: {
                (*clientConnection)->setBuffer(std::make_shared<Buffer>(
                        Buffer(std::make_shared<std::vector<char>>(
                                std::vector<char>(ERROR_MESSAGE_504.begin(), ERROR_MESSAGE_504.end())))));
                break;
            }
            case ClientRequestErrors::ERROR_505: {
                (*clientConnection)->setBuffer(std::make_shared<Buffer>(
                        Buffer(std::make_shared<std::vector<char>>(
                                std::vector<char>(ERROR_MESSAGE_505.begin(), ERROR_MESSAGE_505.end())))));
                break;
            }
        }
    } else {
        activeServersConnections.lock();
        {
            if (activeServersConnections.haveActiveServerConnectionFor((*clientConnection)->getRequestUrl())) {
                (*clientConnection)->setBuffer(
                        activeServersConnections
                                .getServerConnectionFor(
                                        (*clientConnection)->getRequestUrl())->getServerAnswerBuffer());
            } else {
                cacheStorage.lock();
                {
                    if (cacheStorage.contains((*clientConnection)->getRequestUrl())) {
                        (*clientConnection)->setBuffer(
                                cacheStorage.getCacheEntry((*clientConnection)->getRequestUrl()));
                    } else { // записи нет и ее никто не качает
                        int newServerConnectionSocketFd = resolveRequiredHost((*clientConnection)->getRequiredHost());

                        if (newServerConnectionSocketFd >= 0) {
                            std::shared_ptr<CacheEntry> maybeNewEntry = std::make_shared<CacheEntry>(CacheEntry());
                            auto *newServerConnection =
                                    new ServerConnection(newServerConnectionSocketFd,
                                                         (*clientConnection)->getRequestUrl(),
                                                         (*clientConnection)->getProcessedRequestForServer(),
                                                         maybeNewEntry);
                            (*clientConnection)->setBuffer(maybeNewEntry);

                            pthread_t newThreadId;
                            pthread_create(&newThreadId,
                                           nullptr, handleNewServerConnection,
                                           newServerConnection);
                            pthread_detach(newThreadId);
                        } else { // не можем подключиться к хосту
                            (*clientConnection)->setBuffer(std::make_shared<Buffer>(
                                    Buffer(std::make_shared<std::vector<char>>(
                                            std::vector<char>(ERROR_MESSAGE_504.begin(),
                                                              ERROR_MESSAGE_504.end())))));
                        }
                    }
                }
                cacheStorage.unlock();
            }
        }
        activeServersConnections.unlock();
    }
}

void *Proxy::handleNewClientConnection(void *arg) {
//    std::shared_ptr<ClientConnection> clientConnection
//            = std::move(*reinterpret_cast<std::shared_ptr<ClientConnection> *>(arg));
/// а так можно ли вообще делать?

    std::shared_ptr<ClientConnection *> clientConnection =
            std::make_shared<ClientConnection *>(reinterpret_cast<ClientConnection *>(arg));

    (*clientConnection)->receiveRequest();
    if ((*clientConnection)->getState() != ClientConnectionStates::CONNECTION_ERROR) {
        handleArrivalOfClientRequest(clientConnection);
        (*clientConnection)->sendAnswer();
        (*clientConnection)->close();
    } else {
        (*clientConnection)->close();
        pthread_exit(reinterpret_cast<void *>(EXIT_FAILURE));
    }

    pthread_exit(reinterpret_cast<void *>(EXIT_SUCCESS));
}

void Proxy::run() {
    while (!isInterrupt) {
        int acceptedSockFd = accept(listeningSocketFd, nullptr, nullptr);

        if (acceptedSockFd < 0) {
            std::cout << "--------Proxy::handleNewClientConnection(): Error accepting connection--------" << std::endl;
            continue;
        }

        std::cout << "~~~~~~~~~~~~~~~~ NEW CONNECTION: " << acceptedSockFd
                  << " ~~~~~~~~~~~~~~~~" << " " << getTime() << std::endl;

        pthread_t newThreadId;
        pthread_create(&newThreadId, nullptr, handleNewClientConnection,
                       new ClientConnection(acceptedSockFd));
        // хэндлу передается указатель на адрес new CC в куче
        pthread_detach(newThreadId);
    }
}

void Proxy::shutdown() {
    isInterrupt = true;
}

int Proxy::resolveRequiredHost(const std::string &host) {
    std::cout << "RESOLVING HOST = " << host << " " << getTime() << std::endl;

    addrinfo hints{};
    addrinfo *resolvedList = nullptr;

    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET;    /* Разрешены IPv4*/
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(host.c_str(), nullptr, &hints, &resolvedList) != 0) {
        std::cout << "getaddrinfo() ERROR" << std::endl;
        return -1;
    }

    int found_socket = -1;
    for (addrinfo *nextFoundAddress = resolvedList; nextFoundAddress != nullptr;
         nextFoundAddress = nextFoundAddress->ai_next) {

        ((sockaddr_in *) nextFoundAddress->ai_addr)->sin_port = htons(80);

        if ((found_socket = socket(nextFoundAddress->ai_family, nextFoundAddress->ai_socktype,
                                   nextFoundAddress->ai_protocol)) == -1) {
            std::cout << "resolveRequiredHost(): Socket error" << std::endl;
            break;
        }

        if (connect(found_socket, nextFoundAddress->ai_addr, nextFoundAddress->ai_addrlen) == 0) {
            std::cout << "resolveRequiredHost(): connect" << " " << getTime() << std::endl;
            break;
        }

        close(found_socket);
        found_socket = -1;
    }
    std::cout << "END RESOLVE ADDRESS" << " " << getTime() << std::endl;

    freeaddrinfo(resolvedList);
    return found_socket;
}
