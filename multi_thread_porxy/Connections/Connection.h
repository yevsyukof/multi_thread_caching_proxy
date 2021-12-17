#ifndef SINGLE_THREAD_PORXY_CONNECTION_H
#define SINGLE_THREAD_PORXY_CONNECTION_H

#include <vector>
#include <string>
#include <memory>
#include <unistd.h>
#include "sys/socket.h"
#include "/home/yevsyukof/CLionProjects/multi_thread_porxy/Cache/Buffer.h"

class Connection {
public:
    Connection(int connectionSocketFd) : connectionSocketFd(connectionSocketFd) {}

    int close() {
        return ::close(connectionSocketFd);
    }

    bool operator<(const Connection &other) const {
        return this->connectionSocketFd < other.connectionSocketFd;
    }

    bool operator==(const Connection &other) const {
        return this->connectionSocketFd == other.connectionSocketFd;
    }

    void setBuffer(const Buffer& sourceBuffer) {
        this->buffer = sourceBuffer;
    }

    const Buffer& getBuffer() const {
        return buffer;
    }

protected:
    int connectionSocketFd;
    std::string requestUrl;

    Buffer buffer;
};

#endif //SINGLE_THREAD_PORXY_CONNECTION_H
