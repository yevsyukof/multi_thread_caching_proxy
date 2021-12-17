#ifndef SINGLE_THREAD_PORXY_PROXY_H
#define SINGLE_THREAD_PORXY_PROXY_H

#include <map>
#include <queue>
#include <set>
#include <list>
#include <memory>
#include <vector>
#include <sys/socket.h>
#include "Constants.h"
#include "Cache/Cache.h"
#include "Connections/ClientConnection.h"
#include "Connections/ServerConnection.h"
#include "utils/WaitersMap.h"


//static WaitersMap waitersMap = WaitersMap();

static Cache cacheStorage = Cache();

class Proxy {
public:
    Proxy() = default;

    explicit Proxy(int listeningPort);

    ~Proxy() = default;

    void run();

    void shutdown();

private:
    void* handleClientConnection(void *arg);

    static int resolveRequiredHost(const std::string &host);

private:
    bool isInterrupt;
    int listeningSocketFd;
};

#endif //SINGLE_THREAD_PORXY_PROXY_H
