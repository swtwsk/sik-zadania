#ifndef CONCURRENTQUEUE_H
#define CONCURRENTQUEUE_H

#include <vector>
#include <queue>
#include <set>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <utility>

template <class T>
class ConcurrentQueue {
public:
    explicit ConcurrentQueue(uint64_t max_length)
        : max_length_(max_length), queue_(std::queue<T>()), last_element_idx_(0) {}
    ConcurrentQueue(const ConcurrentQueue &) = delete;            // disable copying
    ConcurrentQueue& operator=(const ConcurrentQueue &) = delete; // disable assignment

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

    bool empty() const {
        // we use this queue in 1 producer/1 consumer pattern so nobody would "eat" elements from queue
        return queue_.empty();
    }

private:
    uint64_t max_length_;
    std::queue<T> queue_{};
    uint64_t last_element_idx_;

    std::mutex mutex_;
    std::condition_variable cv_;
};

template <class T>
class ConcurrentSet {
public:
    ConcurrentSet() = default;
    ConcurrentSet(const ConcurrentSet &) = delete;
    ConcurrentSet& operator=(const ConcurrentSet &) = delete;

    void insert(const T& val) {
        std::unique_lock<std::mutex> lock(mutex_);
        set_.insert(val);
    }

    std::vector<T> elements() {
        std::unique_lock<std::mutex> lock(mutex_);
        std::vector<T> elements(set_.begin(), set_.end());
        set_.clear();
        return elements;
    }

    bool empty() const {
        std::unique_lock<std::mutex> lock(mutex_);
        return set_.empty();
    }

private:
    std::set<T> set_;
    std::mutex mutex_;
};

#endif //CONCURRENTQUEUE_H
