#ifndef CONCURRENTQUEUE_H
#define CONCURRENTQUEUE_H

#include <vector>
#include <deque>
#include <set>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <utility>

template <class T>
class ConcurrentDeque {
public:
    using RetransmissionPairVector = std::vector<std::pair<uint64_t, std::vector<T>>>;

    explicit ConcurrentDeque(uint64_t max_length)
        : max_length_(max_length), deque_(std::deque<T>()), last_element_idx_(0) {}
    ConcurrentDeque(const ConcurrentDeque &) = delete;            // disable copying
    ConcurrentDeque& operator=(const ConcurrentDeque &) = delete; // disable assignment

    std::pair<T, uint64_t> pop() {
        std::unique_lock<std::mutex> lock(mutex_); // RAII

        while (deque_.empty()) {
            cv_.wait(lock);
        }

        auto item = deque_.front();
        auto index = last_element_idx_ - deque_.size();
        deque_.pop_front();

        return std::make_pair(item, index);
    }

    void push(const T& item) {
        std::unique_lock<std::mutex> lock(mutex_); // RAII

        if (deque_.size() > max_length_) {
            deque_.pop_front();
        }

        ++last_element_idx_;
        deque_.push_front(item);
        lock.unlock();
        cv_.notify_one();
    }

    void push(T&& item) {
        std::unique_lock<std::mutex> lock(mutex_); // RAII

        if (deque_.size() > max_length_) {
            deque_.pop_front();
        }

        ++last_element_idx_;
        deque_.push_front(std::move(item));
        lock.unlock();
        cv_.notify_one();
    }

    void push(T *items, uint64_t item_count) {
        std::unique_lock<std::mutex> lock(mutex_); // RAII

        for (auto i = 0U; i < item_count; ++i) {
            deque_.push_front(items[i]);
            ++last_element_idx_;

            if (deque_.size() > max_length_) {
                deque_.pop_front();
            }
        }

        lock.unlock();
        cv_.notify_one();
    }

    RetransmissionPairVector getPackets(const std::vector<uint64_t> &packets_idxs, uint64_t packet_data_size) {
        RetransmissionPairVector to_return;

        std::unique_lock<std::mutex> lock(mutex_); // RAII
        uint64_t f_idx = last_element_idx_ - deque_.size();

        for (auto &packet_idx : packets_idxs) {
            if (packet_idx < f_idx) {
                continue;
            }

            uint64_t deq_idx = packet_idx - f_idx;
            if (deq_idx + packet_data_size > deque_.size()) {
                break;
            }

            auto it_beg = deque_.begin() + deq_idx;
            auto it_end = it_beg + packet_data_size - 1;

            to_return.push_back(std::make_pair(packet_idx, std::vector<T>(it_beg, it_end)));
        }

        return to_return;
    }

    bool empty() const {
        // we use this queue in 1 producer/1 consumer pattern so nobody would "eat" elements from queue
        return deque_.empty();
    }

private:
    uint64_t max_length_;
    std::deque<T> deque_;
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
    mutable std::mutex mutex_;
};

#endif //CONCURRENTQUEUE_H
