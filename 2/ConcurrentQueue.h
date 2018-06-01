#ifndef CONCURRENTQUEUE_H
#define CONCURRENTQUEUE_H

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <utility>

template <class T>
class ConcurrentQueue {
public:
    std::pair<T, uint64_t> pop() {
        std::unique_lock<std::mutex> lock(mutex_); // RAII

        while (queue_.empty()) {
            cv_.wait(lock);
        }

        auto item = queue_.front();
        auto index = last_element_idx_ - queue_.size();
        queue_.pop();

        return std::make_pair(item, index);
    }

    void push(const T& item) {
        std::unique_lock<std::mutex> lock(mutex_); // RAII

        if (queue_.size() > max_length_) {
            queue_.pop();
        }

        ++last_element_idx_;
        queue_.push(item);
        lock.unlock();
        cv_.notify_one();
    }

    void push(T&& item) {
        std::unique_lock<std::mutex> lock(mutex_); // RAII

        if (queue_.size() > max_length_) {
            queue_.pop();
        }

        ++last_element_idx_;
        queue_.push(std::move(item));
        lock.unlock();
        cv_.notify_one();
    }

    explicit ConcurrentQueue(int max_length) : max_length_(max_length), queue_(std::queue<T>()), last_element_idx_(0) {}
    ConcurrentQueue(const ConcurrentQueue&) = delete;            // disable copying
    ConcurrentQueue& operator=(const ConcurrentQueue&) = delete; // disable assignment

private:
    int max_length_;
    std::queue<T> queue_;
    uint64_t last_element_idx_;

    std::mutex mutex_;
    std::condition_variable cv_;
};

#endif //CONCURRENTQUEUE_H
