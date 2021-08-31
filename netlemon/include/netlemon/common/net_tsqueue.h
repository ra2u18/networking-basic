#pragma once

#include "netlemon/net_common.h"

namespace netlemon::common
{
    template<typename T>
    class TSQueue
    {
    public:
        TSQueue() = default;
        TSQueue(const TSQueue<T>&) = delete;

        virtual ~TSQueue() { clear(); }
    public:
        // Returns and maintains item at front of queue
        const T& front()
        {
            std::scoped_lock lock(muxQueue);
            return deqQueue.front();
        }
        // Returns and maintains item at back of queue
        const T& back()
        {
            std::scoped_lock lock(muxQueue);
            return deqQueue.back();
        }
        // Adds an item to the back of the queue
        const T& push_back(const T& item)
        {
            std::scoped_lock lock(muxQueue);
            return deqQueue.emplace_back(std::move(item));
        }
        // Adds an item to the front of the queue
        const T& push_front(const T& item)
        {
            std::scoped_lock lock(muxQueue);
            return deqQueue.emplace_front(std::move(item));
        }

        // Returns true if queue has no items
        bool empty()
        {
            std::scoped_lock lock(muxQueue);
            return deqQueue.empty();
        }
        uint64_t count()
        {
            std::scoped_lock lock(muxQueue);
            return deqQueue.size();
        }
        void clear()
        {
            std::scoped_lock lock(muxQueue);
            deqQueue.clear();
        }

        // Removes and returns item from front of queue
        T pop_front()
        {
            std::scoped_lock lock(muxQueue);
            auto t = std::move(deqQueue.front());
            deqQueue.pop_front();
            return t;
        }
        // Removes and returns item from back of queue
        T pop_back()
        {
            std::scoped_lock lock(muxQueue);
            auto t = std::move(deqQueue.back());
            deqQueue.pop_back();
            return t;
        }

    protected:
        std::mutex muxQueue;
        std::deque<T> deqQueue;
    };
}