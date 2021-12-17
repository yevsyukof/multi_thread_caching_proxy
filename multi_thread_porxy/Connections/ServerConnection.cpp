#include <iostream>
#include <utility>
#include <sys/socket.h>
#include "ServerConnection.h"

#include "../HttpParser/httpresponseparser.h"

#define RECV_BUF_SIZE 32768

ServerConnection::ServerConnection(int connectionSocketFd,
                                   const std::string &requestUrl,
                                   std::shared_ptr<std::string> processedRequestForServer,
                                   std::shared_ptr<CacheEntry> answerBuffer)
        : Connection(connectionSocketFd),
          processedRequestForServer(std::move(processedRequestForServer)),
          sendRequestOffset(0),
          connectionState(ServerConnectionState::SENDING_REQUEST) {
    this->serverAnswerBuffer = std::move(answerBuffer);
    this->requestUrl = requestUrl;
}

void ServerConnection::sendRequest() {
    bool isAllSent = false;
    while (!isAllSent) {
        int sendCount;
        if ((sendCount = send(connectionSocketFd,
                              processedRequestForServer->c_str() + sendRequestOffset,
                              processedRequestForServer->size() - sendRequestOffset, 0)) == -1) {

            std::cout << "\n----------------SERVER SEND REQUEST ERROR----------------\n" << std::endl;
            connectionState = ServerConnectionState::CONNECTION_ERROR;
            isAllSent = true;
        } else {
            sendRequestOffset += sendCount;
            if (sendRequestOffset == processedRequestForServer->length()) {
                connectionState = ServerConnectionState::RECEIVING_ANSWER;
                isAllSent = true;
            } else {
                connectionState = ServerConnectionState::SENDING_REQUEST;
            }
        }
    }
}

bool ServerConnection::isResponseStatusIs200() const {
    httpparser::HttpResponseParser httpResponseParser;
    httpparser::Response parsedResponse;

    httpparser::HttpResponseParser::ParseResult parseResult = httpResponseParser
            .parse(parsedResponse,
                   serverAnswerBuffer->getData()->data(),
                   serverAnswerBuffer->getData()->data() + serverAnswerBuffer->getData()->size());

    return parseResult != httpparser::HttpResponseParser::ParsingError && parsedResponse.statusCode == 200;
}

void ServerConnection::receiveAnswer() {
    serverAnswerBuffer->lock();
    {

    }
    serverAnswerBuffer->unlock();
}
