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
        conditionVar = PTHREAD_COND_INITIALIZER;
    }

//    explicit CacheEntry(const std::shared_ptr<std::vector<char>> &entryData) {
//        data = entryData;
//    }

    ~CacheEntry() override {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&conditionVar);
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
        bool haveData;
        lock();
        {
            if ((data->size() - threadOffset) > 0) {
                haveData = true;
            } else if (!isReady) {
                while ((data->size() - threadOffset) == 0 && !isReady) {
                    pthread_cond_wait(&conditionVar, &mutex);
                }
                haveData = (data->size() - threadOffset) > 0;
            } else {
                haveData = false;
            }
        }
        unlock();
        return haveData;
    }

    void setReadyState(bool state) {
        isReady = state;
    }

    int notifyHolders() {
        return pthread_cond_broadcast(&conditionVar);
    }

private:
    pthread_mutex_t mutex {};
    pthread_cond_t conditionVar {};
};


#endif //SINGLE_THREAD_PORXY_CACHEENTRY_H
