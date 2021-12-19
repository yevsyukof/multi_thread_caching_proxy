#ifndef SINGLE_THREAD_PORXY_CACHE_H
#define SINGLE_THREAD_PORXY_CACHE_H

#include <map>
#include <string>
#include <memory>
#include <utility>
#include "CacheEntry.h"

#define SIZE_LIMIT 104'857'600 // 100 MB

class CacheStorage {
public:
    CacheStorage() {
        mutex = PTHREAD_MUTEX_INITIALIZER;
        size = 0;
    }

    ~CacheStorage() {
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
        if (size <= SIZE_LIMIT) {
            size += entry->getData()->size();
            urlToCacheEntry[url] = entry;
        } else {
            std::cout << "Cache storage is full: can't add new entry by url: " << url << std::endl;
        }
    }

    const std::shared_ptr<CacheEntry>& getCacheEntry(const std::string &url) const {
        return urlToCacheEntry.at(url);
    }

private:
    unsigned long long size;

    pthread_mutex_t mutex {};

    std::map<std::string, std::shared_ptr<CacheEntry>> urlToCacheEntry;
};


#endif //SINGLE_THREAD_PORXY_CACHE_H
