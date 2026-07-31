#pragma once
#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
class ThreadSafeQueue {
public:
    void push(T value) {
        { std::lock_guard<std::mutex> lock(mutex_); queue_.push(std::move(value)); }
        cv_.notify_one();
    }
    T waitPop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&]{ return !queue_.empty(); });
        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
};
