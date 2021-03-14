// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_SEMAPHORE_HPP
#define NETXS_SEMAPHORE_HPP

#include <mutex>
#include <condition_variable>

namespace utils
{
    class semaphore
    {
    private:
        int max_count;
        int count = 0;
        std::mutex m;
        std::condition_variable cv;

    public:
        explicit semaphore(int max_count) : max_count(max_count) {}

        void acquire()
        {
            std::unique_lock<std::mutex> lock(m);
            while (count == max_count)
            {
                cv.wait(lock);
            }
            ++count;
        }

        void release()
        {
            std::lock_guard<std::mutex> lock(m);

            --count;
            cv.notify_one();
        }
    };
};

#endif // NETXS_SEMAPHORE_HPP