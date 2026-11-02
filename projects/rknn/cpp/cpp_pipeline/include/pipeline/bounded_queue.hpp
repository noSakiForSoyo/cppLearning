#pragma once

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <utility>

template <typename T>
class BoundedQueue
{
public:
    explicit BoundedQueue(std::size_t cap) : cap_(cap) {}

    bool push(T &&item)
    {
        std::lock_guard<std::mutex> lock(m_);
        if (stopped_ || cap_ == 0)
            return false;

        if (q_.size() >= cap_)
            q_.pop_front();

        q_.push_back(std::move(item));
        cv_.notify_one();
        return true;
    }

    bool pop(T &item)
    {
        std::unique_lock<std::mutex> lock(m_);
        cv_.wait(lock, [this] { return stopped_ || !q_.empty(); });

        if (q_.empty())
            return false;

        item = std::move(q_.front());
        q_.pop_front();
        return true;
    }

    bool try_pop(T &item)
    {
        std::lock_guard<std::mutex> lock(m_);
        if (q_.empty())
            return false;

        item = std::move(q_.front());
        q_.pop_front();
        return true;
    }

    void stop()
    {
        {
            std::lock_guard<std::mutex> lock(m_);
            stopped_ = true;
        }
        cv_.notify_all();
    }

    std::size_t size() const
    {
        std::lock_guard<std::mutex> lock(m_);
        return q_.size();
    }

private:
    std::deque<T> q_;
    std::size_t cap_;
    mutable std::mutex m_;
    std::condition_variable cv_;
    bool stopped_ = false;
};
