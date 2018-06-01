#ifndef CONCURRENTQUEUE_H
#define CONCURRENTQUEUE_H

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template <class T>
class ConcurrentQueue {
public:
    T pop() {
        std::unique_lock<std::mutex> lock(mutex_); // RAII

        while (queue_.empty()) {
            cv_.wait(lock);
        }

        auto item = queue_.front();
        queue_.pop();
        return item;
    }

    void pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_); // RAII

        while (queue_.empty()) {
            cv_.wait(lock);
        }

        item = queue_.front();
        queue_.pop();
    }

    void push(const T& item) {
        std::unique_lock<std::mutex> lock(mutex_);

        queue_.push(item);
        lock.unlock();
        cv_.notify_one();
    }

    void push(T&& item) {
        std::unique_lock<std::mutex> lock(mutex_);

        queue_.push(std::move(item));
        lock.unlock();
        cv_.notify_one();
    }

    ConcurrentQueue() = default;
    ConcurrentQueue(const ConcurrentQueue&) = delete;            // disable copying
    ConcurrentQueue& operator=(const ConcurrentQueue&) = delete; // disable assignment

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

#endif //CONCURRENTQUEUE_H
