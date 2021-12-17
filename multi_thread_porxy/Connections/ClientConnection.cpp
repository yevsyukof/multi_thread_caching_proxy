#include <string>
#include <iostream>
#include "ClientConnection.h"

#define RECV_BUF_SIZE 32768

ClientConnection::ClientConnection(int connectionSocketFd)
        : Connection(connectionSocketFd),
          processedRequestForServer(nullptr),
          sendAnswerOffset(0) {}


void ClientConnection::parseRequestAndCheckValidity() {
    httpparser::HttpRequestParser httpRequestParser;
    httpparser::HttpRequestParser::ParseResult parseResult = httpRequestParser
            .parse(clientHttpRequest, recvBuf.data(), recvBuf.data() + recvBuf.size());

    if (parseResult != httpparser::HttpRequestParser::ParsingCompleted) {
        std::cout << "--------CAN'T PARSE HTTP REQUEST--------" << std::endl;
        connectionState = ClientConnectionStates::WRONG_REQUEST;
        requestValidatorState = ClientRequestErrors::ERROR_501;
        return;
    }

    else {
        std::cout << "**************** received request: ****************" << std::endl;
        std::cout << clientHttpRequest.inspect() << std::endl;
        std::cout << "**************** end request ****************" << std::endl;
    }

    if (clientHttpRequest.versionMajor != 1 || clientHttpRequest.versionMinor != 0) {
        std::cout << "--------WRONG HTTP VERSION--------" << std::endl;

        connectionState = ClientConnectionStates::WRONG_REQUEST;
        requestValidatorState = ClientRequestErrors::ERROR_505;
        return;
    }


    if (clientHttpRequest.method != "GET" && clientHttpRequest.method != "HEAD") {
        std::cout << "--------NOT IMPLEMENTED HTTP METHOD--------" << std::endl;

        connectionState = ClientConnectionStates::WRONG_REQUEST;
        requestValidatorState = ClientRequestErrors::ERROR_405;
        return;
    }

    std::stringstream ss;
    ss << clientHttpRequest.method << " " << clientHttpRequest.uri << " HTTP/1.0" << "\r\n";

    bool hasHostHeader = false;
    for (const auto &header: clientHttpRequest.headers) {
        if (header.name == "Host") {
            requiredHost = header.value;
            hasHostHeader = true;
            continue; // http/1.0
        } else if (header.name == "Connection") {
            continue;
        }
        ss << header.name << ": " << header.value << "\r\n";
    }

    if (!hasHostHeader) {
        std::cout << "--------HASN'T HOST--------" << std::endl;

        connectionState = ClientConnectionStates::WRONG_REQUEST;
        requestValidatorState = ClientRequestErrors::ERROR_400;
        return;
    } else {
        ss << "\r\n";
    }

    connectionState = ClientConnectionStates::WAITING_FOR_RESPONSE;
    requestValidatorState = ClientRequestErrors::WITHOUT_ERRORS;

    processedRequestForServer = std::make_shared<std::string>(ss.str());

    requestUrl = clientHttpRequest.method + clientHttpRequest.uri;
}

void ClientConnection::receiveRequest() {
    char buf[RECV_BUF_SIZE];
    ssize_t recvCount;

    bool hasRequestEnd = false;
    while (!hasRequestEnd) {
        if ((recvCount = recv(connectionSocketFd, buf, RECV_BUF_SIZE, 0)) <= 0) {
            std::cout << "--------RECEIVE FROM CLIENT SOCKET ERROR--------" << std::endl;

            connectionState = ClientConnectionStates::CONNECTION_ERROR;
            hasRequestEnd = true;
        } else {
            recvBuf.insert(recvBuf.end(), buf, buf + recvCount);

            if (recvBuf.size() >= 4 && recvBuf[recvBuf.size() - 4] == '\r'
                && recvBuf[recvBuf.size() - 3] == '\n'
                && recvBuf[recvBuf.size() - 2] == '\r'
                && recvBuf[recvBuf.size() - 1] == '\n') {

                parseRequestAndCheckValidity(); // внутри установлилось новое состояние соединения
                hasRequestEnd = true;
            } else {
                connectionState = ClientConnectionStates::RECEIVING_REQUEST;
            }
        }
    }
}

bool ClientConnection::sendAnswer() {
    while (buffer->haveRemainingDataFrom(sendAnswerOffset)) {
        buffer->lock();
        {
            long long sendCount;
            if ((sendCount = send(connectionSocketFd,
                                  buffer->getData()->data() + sendAnswerOffset,
                                  buffer->getData()->size() - sendAnswerOffset, 0)) == -1) {
                std::cout << "--------sendAnswer(): SEND ERROR--------" << std::endl;

                connectionState = ClientConnectionStates::CONNECTION_ERROR;
                return false;
            } else {
                sendAnswerOffset += sendCount;
            }
        }
        buffer->unlock();
    }
    std::cout << "--------sendAnswer(): CLIENT RECEIVE ANSWER--------" << std::endl;
    return true;
}
