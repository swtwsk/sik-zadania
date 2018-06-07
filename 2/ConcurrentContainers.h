#ifndef CONCURRENT_CONTAINERS_H
#define CONCURRENT_CONTAINERS_H

#include <vector>
#include <deque>
#include <set>
#include <thread>
#include <mutex>
#include <utility>

template <class T>
class ConcurrentDeque {
private:
    using NumType = uint64_t;
    using RetransmissionPairVector = std::vector<std::pair<NumType, std::vector<T>>>;

public:
    explicit ConcurrentDeque(NumType max_length)
        : max_length_(max_length), deque_(std::deque<T>()), last_element_idx_(0) {}
    ConcurrentDeque(const ConcurrentDeque &) = delete;             // disable copying
    ConcurrentDeque& operator=(const ConcurrentDeque &) = delete;  // disable assignment

    void push(T *items, NumType item_count) {
        std::unique_lock<std::mutex> lock(mutex_);  // RAII

        for (auto i = 0U; i < item_count; ++i) {
            deque_.push_back(items[i]);
            ++last_element_idx_;

            if (deque_.size() > max_length_) {
                deque_.pop_front();
            }
        }
    }

    RetransmissionPairVector getPackets(const std::vector<NumType> &packets_idxs, NumType packet_data_size) {
        RetransmissionPairVector to_return;
        std::unique_lock<std::mutex> lock(mutex_);  // RAII

        NumType f_idx = last_element_idx_ - deque_.size();

        for (auto &packet_idx : packets_idxs) {
            if (packet_idx < f_idx) {
                continue;
            }

            NumType deq_idx = packet_idx - f_idx;
            if (deq_idx + packet_data_size > deque_.size()) {
                break;
            }

            auto it_beg = deque_.begin() + deq_idx;
            auto it_end = it_beg + packet_data_size - 1;

            to_return.push_back(std::make_pair(packet_idx, std::vector<T>(it_beg, it_end)));
        }

        return to_return;
    }

private:
    NumType max_length_;
    std::deque<T> deque_{};
    NumType last_element_idx_;

    std::mutex mutex_;
};

template <class T>
class ConcurrentSet {
public:
    explicit ConcurrentSet() = default;
    explicit ConcurrentSet(const ConcurrentSet &) = delete;
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
    std::set<T> set_{};
    mutable std::mutex mutex_;
};

#endif //CONCURRENT_CONTAINERS_H
