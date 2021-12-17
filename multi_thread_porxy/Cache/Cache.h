#ifndef SINGLE_THREAD_PORXY_CACHE_H
#define SINGLE_THREAD_PORXY_CACHE_H

#include <map>
#include <string>
#include <memory>
#include <utility>
#include "CacheEntry.h"


class Cache {
public:
    Cache() {
        mutex = PTHREAD_MUTEX_INITIALIZER;
    }

    ~Cache() {
        pthread_mutex_destroy(&mutex);
    }

    pthread_mutex_t& getMutex() {
        return mutex;
    }

    bool contains(const std::string &url) {
        return urlToCacheEntry.find(url) != urlToCacheEntry.end();
    }

    void addCacheEntry(const std::string &url,
                       const std::shared_ptr<std::vector<char>>& entryData) {
        urlToCacheEntry[url] = CacheEntry(entryData);
    }

    const CacheEntry& getCacheEntry(const std::string &url) const {
        return urlToCacheEntry.at(url); // TODO
    }

private:
    pthread_mutex_t mutex {};

    std::map<std::string, CacheEntry> urlToCacheEntry;
};


#endif //SINGLE_THREAD_PORXY_CACHE_H
