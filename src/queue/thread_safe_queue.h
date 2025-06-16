#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class ThreadSafeQueue {
public:
    void push(const T& item) {
        std::lock_guard<std::mutex> lock(mtx_);
        queue_.push(item);
        cond_.notify_one();
    }

    bool try_pop(T& value) {
	    std::lock_guard<std::mutex> lock(mtx_);
	    if (queue_.empty())
	        return false;
	    value = std::move(queue_.front());
	    queue_.pop();
	    return true;
    }


    T pop() {
        std::unique_lock<std::mutex> lock(mtx_);
        cond_.wait(lock, [this] { return !queue_.empty(); });
        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.empty();
    }

private:
    mutable std::mutex mtx_;
    std::queue<T> queue_;
    std::condition_variable cond_;
};

