// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

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
            auto lock = std::unique_lock{ m };
            while (count == max_count)
            {
                cv.wait(lock);
            }
            ++count;
        }

        void release()
        {
            auto guard = std::lock_guard{ m };
            --count;
            cv.notify_one();
        }
    };
};