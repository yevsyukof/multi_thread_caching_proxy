#ifndef SINGLE_THREAD_PORXY_CACHEENTRY_H
#define SINGLE_THREAD_PORXY_CACHEENTRY_H

#include <utility>
#include <vector>
#include <memory>
#include "Buffer.h"

class CacheEntry : public Buffer {
public:
    CacheEntry() : Buffer() {
        isReady = false;
        mutex = PTHREAD_MUTEX_INITIALIZER;
    }

//    explicit CacheEntry(const std::shared_ptr<std::vector<char>> &entryData) {
//        data = entryData;
//    }

    ~CacheEntry() override {
        pthread_mutex_destroy(&mutex);
    }

    int lock() override {
        if (isReady) {
            return SUCCESS;
        } else {
            return pthread_mutex_lock(&mutex);
        }
    }

    int unlock() override {
        if (isReady) {
            return SUCCESS;
        } else {
            return pthread_mutex_unlock(&mutex);
        }
    }

    bool haveRemainingDataFrom(long long threadOffset) override {
        if (isReady) {
            return (data->size() - threadOffset) > 0;
        } else {
            return true;
            // TODO работа с условной переменной
        }
    }

private:
    pthread_mutex_t mutex {};
};


#endif //SINGLE_THREAD_PORXY_CACHEENTRY_H
