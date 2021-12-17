#ifndef SINGLE_THREAD_PORXY_BUFFER_H
#define SINGLE_THREAD_PORXY_BUFFER_H


#include <utility>
#include <vector>
#include <memory>

#define SUCCESS 1

class Buffer {
public:
    // без переданных параметров - внутри создается свой пустой буффер
    Buffer(std::shared_ptr<std::vector<char>> data = std::make_shared<std::vector<char>>(std::vector<char>()))
            : data(std::move(data)),
              isReady(true) {}

    virtual ~Buffer() = default;

    virtual int lock() { return SUCCESS; } // inline?

    virtual int unlock() { return SUCCESS; }

    virtual bool haveRemainingDataFrom(long long threadOffset) {
        return (data->size() - threadOffset) > 0;
    }

    const std::shared_ptr<std::vector<char>>& getData() {
        return data;
    }

    Buffer& operator=(const Buffer& other) {
        if (this == &other) {
            return *this;
        }
        this->data = other.data;
        this->isReady = other.isReady;
        return *this;
    }

protected:
    std::shared_ptr<std::vector<char>> data;
    volatile bool isReady;
};


#endif //SINGLE_THREAD_PORXY_BUFFER_H
