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

    static bool containsUrlList(const std::string &url) {
        return clientsWaitingForResponse.find(url) != clientsWaitingForResponse.end();
    }

    static void addNewWaitingUrlList(const std::string &url) {
        clientsWaitingForResponse[url];
    }

    static void removeWaitingUrlList(const std::string &url) {
        clientsWaitingForResponse.erase(url);
    }

    static void addClientAtWaitingUrlList(const std::string &url,
                                   const std::shared_ptr<ClientConnection>& clientConnection) {
        clientsWaitingForResponse[url].insert(clientConnection);
    }

private:
    pthread_mutex_t mutex {};

    // url -> clients
    static std::map<std::string, std::set<std::shared_ptr<ClientConnection>>> clientsWaitingForResponse;
};


#endif //SINGLE_THREAD_PORXY_WAITERSMAP_H
