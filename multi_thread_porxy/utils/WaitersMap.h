//
// Created by yevsyukof on 17.12.2021.
//

#ifndef SINGLE_THREAD_PORXY_WAITERSMAP_H
#define SINGLE_THREAD_PORXY_WAITERSMAP_H


#include <map>
#include <string>
#include <set>
#include <memory>
#include <pthread.h>
#include "../Connections/ClientConnection.h"

class WaitersMap {
public:
    WaitersMap() {
        mutex = PTHREAD_MUTEX_INITIALIZER;
    }

    ~WaitersMap() {
        pthread_mutex_destroy(&mutex);
    }

    int lock() {
        return pthread_mutex_lock(&mutex);
    }

    int unlock() {
        return pthread_mutex_unlock(&mutex);
    }

    bool haveActiveServerConnectionFor(const std::string &url) {
        return activeServersConnections.find(url) != activeServersConnections.end();
    }

    void addNewServerConnection(const std::string &url,
                                const std::shared_ptr<ServerConnection>& serverConnection) {
        activeServersConnections[url] = serverConnection;
    }

    void removeActiveServerConnectionFor(const std::string &url) {
        activeServersConnections.erase(url);
    }

    const std::shared_ptr<ServerConnection>& getServerConnectionFor(const std::string &url) {
        return activeServersConnections[url];
    }

private:
    pthread_mutex_t mutex {};

    // url -> server connection
    std::map<std::string, std::shared_ptr<ServerConnection>> activeServersConnections;
};


#endif //SINGLE_THREAD_PORXY_WAITERSMAP_H
