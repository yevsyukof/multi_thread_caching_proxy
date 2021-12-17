#ifndef SINGLE_THREAD_PORXY_BUFFER_H
#define SINGLE_THREAD_PORXY_BUFFER_H


#include <utility>
#include <vector>
#include <memory>

class Buffer {
public:
    // без переданных параметров - внутри создается свой пустой буффер
    Buffer(std::shared_ptr<std::vector<char>> data = std::make_shared<std::vector<char>>(std::vector<char>()))
            : data(std::move(data)),
              isReady(true) {}

    virtual ~Buffer() = default;

    virtual void lock() {  } // inline?

    virtual void unlock() {  }

    bool virtual haveRemainingDataFrom(long long threadOffset) {
        return (data->size() - threadOffset) > 0;
    }

    const std::shared_ptr<std::vector<char>>& getData() {
        return data;
    }

protected:
    std::shared_ptr<std::vector<char>> data;
    bool isReady;
};


#endif //SINGLE_THREAD_PORXY_BUFFER_H
