#ifndef SINGLE_THREAD_PORXY_CONNECTION_H
#define SINGLE_THREAD_PORXY_CONNECTION_H

#include <vector>
#include <string>
#include <memory>
#include <unistd.h>
#include "sys/socket.h"


class Connection {
public:
    Connection(int connectionSocketFd, int inPollListIdx) : connectionSocketFd(connectionSocketFd) {}

    int close() {
        return ::close(connectionSocketFd);
    }

    bool operator<(const Connection &other) const {
        return this->connectionSocketFd < other.connectionSocketFd;
    }

    bool operator==(const Connection &other) const {
        return this->connectionSocketFd == other.connectionSocketFd;
    }

protected:
    int connectionSocketFd;
};

#endif //SINGLE_THREAD_PORXY_CONNECTION_H
