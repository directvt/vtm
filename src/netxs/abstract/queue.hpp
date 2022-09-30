// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_QUEUE_HPP
#define NETXS_QUEUE_HPP

#include <mutex>
#include <condition_variable>
#include <deque>
#include <list>
#include <thread>

namespace netxs
{
    template <class T>
    class mt_queue
    {
        std::mutex              d_mutex;
        std::condition_variable d_condition;
        std::deque<T>           d_queue;

    public:
        void push(T const& value)
        {
            auto guard = std::lock_guard{ d_mutex };
            d_queue.push_front(value);
            d_condition.notify_one();
        }
        T pop()
        {
            auto lock = std::unique_lock{ d_mutex };
            d_condition.wait(lock, [this] { return !d_queue.empty(); });
            T rc(std::move(d_queue.back()));
            d_queue.pop_back();
            return rc;
        }
        bool try_pop(T& v, std::chrono::milliseconds timeout)
        {
            auto lock = std::unique_lock{ d_mutex };
            if (!d_condition.wait_for(lock, timeout, [this] { return !d_queue.empty(); }))
            {
                return !true;
            }
            v = d_queue.back();
            d_queue.pop_back();
            return true;
        }
        size_t size()
        {
            auto guard = std::lock_guard{ d_mutex };
            return d_queue.size();
        }
        void clear()
        {
            auto guard = std::lock_guard{ d_mutex };
            d_queue.clear();
        }

        mt_queue() { }
        mt_queue(mt_queue<T>&& x)
        {
            auto guard = std::lock_guard{ d_mutex };
            d_queue = std::move(x.d_queue);
        }
        mt_queue<T>& operator = (mt_queue<T>&& x)
        {
            //todo *this is not a MT safe (only x.d_mutex is locked)
            auto guard = std::lock_guard{ d_mutex };
            d_queue = std::move(x.d_queue);
            return *this;
        }
    };

    // queue: Separate thread for executing deferred tasks.
    template<class T>
    class jobs
    {
    public:
        using token = T;
        using func = std::function<void(token&)>;
        using item = std::pair<token, func>;

        std::mutex              mutex;
        std::condition_variable synch;
        std::list<item>         queue;
        bool                    alive;
        std::thread             agent;

        template<class P>
        void cancel(P&& deactivate)
        {
            auto guard = std::unique_lock{ mutex };
            for (auto& job : queue)
            {
                auto& token = job.first;
                deactivate(token);
            }
        }
        void worker()
        {
            auto guard = std::unique_lock{ mutex };
            while (alive)
            {
                if (queue.empty()) synch.wait(guard);
                while (alive && queue.size())
                {
                    auto& [token, proc] = queue.front();
                    guard.unlock();
                    proc(token);
                    guard.lock();
                    queue.pop_front();
                }
            }
        }

        jobs()
            : alive{ true },
              agent{ &jobs::worker, this }
        { }
       ~jobs()
        {
            mutex.lock();
            alive = faux;
            synch.notify_one();
            mutex.unlock();
            agent.join();
        }
        template<class TT, class P>
        void add(TT&& token, P&& proc)
        {
            auto guard = std::lock_guard{ mutex };
            if constexpr (std::is_copy_constructible_v<P>)
            {
                queue.emplace_back(std::forward<TT>(token), std::forward<P>(proc));
            }
            else
            {
                //todo issue with MSVC: Generalized lambda capture does't work.
                auto proxy = std::make_shared<std::decay_t<P>>(std::forward<P>(proc));
                queue.emplace_back(std::forward<TT>(token), [proxy](auto&&... args)->decltype(auto)
                {
                    return (*proxy)(decltype(args)(args)...);
                });
            }
            synch.notify_one();
        }
    };
}

#endif // NETXS_QUEUE_HPP