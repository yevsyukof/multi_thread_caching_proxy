#ifndef SINGLE_THREAD_PORXY_SERVERCONNECTION_H
#define SINGLE_THREAD_PORXY_SERVERCONNECTION_H

#include "Connection.h"
#include "/home/yevsyukof/CLionProjects/multi_thread_porxy/Cache/CacheEntry.h"

enum class ServerConnectionState {
    CONNECTION_ERROR,
    SENDING_REQUEST,
    RECEIVING_ANSWER,
    CACHING_ANSWER_RECEIVED,
    NOT_CACHING_ANSWER_RECEIVED
};

class ServerConnection : public Connection {
public:
    ServerConnection(int connectionSocketFd,
                     const std::string &requestUrl,
                     std::shared_ptr<std::string>  processedRequestForServer,
                     std::shared_ptr<CacheEntry> answerBuffer);

    ServerConnectionState getState() const {
        return connectionState;
    }

    const std::shared_ptr<CacheEntry>& getServerAnswerBuffer() const {
        return serverAnswerBuffer;
    }

    void sendRequest();

    void receiveAnswer();

    bool isCachingAnswerReceived() const {
        return isResponseStatusIs200();
    }

private:
    bool isResponseStatusIs200() const;

private:
    std::shared_ptr<std::string> processedRequestForServer;
    long long sendRequestOffset;

    ServerConnectionState connectionState;

    std::shared_ptr<CacheEntry> serverAnswerBuffer;
};


#endif //SINGLE_THREAD_PORXY_SERVERCONNECTION_H
