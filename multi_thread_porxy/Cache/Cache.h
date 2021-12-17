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

    int lock() {
        return pthread_mutex_lock(&mutex);
    }

    int unlock() {
        return pthread_mutex_unlock(&mutex);
    }

    bool contains(const std::string &url) {
        return urlToCacheEntry.find(url) != urlToCacheEntry.end();
    }

    void addCacheEntry(const std::string &url,
                       const std::shared_ptr<CacheEntry>& entry) {
        urlToCacheEntry[url] = entry;
    }

    const std::shared_ptr<CacheEntry>& getCacheEntry(const std::string &url) const {
        return urlToCacheEntry.at(url);
    }

private:
    pthread_mutex_t mutex {};

    std::map<std::string, std::shared_ptr<CacheEntry>> urlToCacheEntry;
};


#endif //SINGLE_THREAD_PORXY_CACHE_H
