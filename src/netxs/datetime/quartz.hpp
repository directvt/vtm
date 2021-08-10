// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_QUARTZ_HPP
#define NETXS_QUARTZ_HPP

#include "../math/intmath.hpp"

#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <optional>
#include <condition_variable>

namespace netxs::datetime
{
    using tempus = std::chrono::steady_clock;
    using period = std::chrono::steady_clock::duration;
    using moment = std::chrono::time_point<std::chrono::steady_clock>;
    using namespace std::chrono_literals;

    // quartz: Round a chrono time moment in degree (def: milliseconds)
    //         units since epoch.
    template<class T, class degree = std::chrono::milliseconds>
    static T round(moment t)
    {
        return clamp<T>(std::chrono::duration_cast<degree>(t.time_since_epoch()).count());
    }
    // quartz: Round a chrono time period in degree (def: milliseconds).
    template<class T, class degree = std::chrono::milliseconds>
    static T round(period t)
    {
        return clamp<T>(std::chrono::duration_cast<degree>(t).count());
    }

    // quartz: Return a total count degree unts (def: milliseconds) since epoch.
    template<class T, class degree = std::chrono::milliseconds>
    static T now()
    {
        return round<T, degree>(tempus::now());
    }

    template<class REACTOR, class CONTEXT>
    class quartz
    {
        using cond = std::condition_variable;
        using work = std::thread;

        REACTOR & alarm;
        CONTEXT   cause;
        bool      alive;
        bool      letup;
        period    delay;
        period    pulse;
        work      fiber;
        cond      synch;
        period    watch;

        void worker()
        {
            auto mutex = std::mutex{};
            auto guard = std::unique_lock{ mutex };

            auto now = tempus::now();
            auto prior = now;

            while (alive)
            {
                watch += now - prior;
                prior =  now;

                now = tempus::now();
                alarm.notify(cause, now);

                if (letup)
                {
                    synch.wait_for(guard, delay);

                    delay = period::zero();
                    letup = faux;
                }
                else
                {
                    auto trail = pulse - now.time_since_epoch() % pulse;
                    synch.wait_for(guard, trail);
                }
            }
        }

    public:
        quartz(REACTOR& router, CONTEXT cause)
            : alarm { router         },
              cause { cause          },
              alive { faux           },
              letup { faux           },
              delay { period::zero() },
              watch { period::zero() },
              pulse { period::max()  }
        { }

        operator bool()
        {
            return alive;
        }
        void ignite(period const& interval)
        {
            pulse = interval;

            if (!alive)
            {
                alive = true;
                fiber = std::thread([&] { worker(); });
            }
        }
        void ignite(int frequency)
        {
            ignite(period{ period::period::den / frequency });
        }
        void freeze(period pause2)
        {
            delay = pause2;
            letup = true;
        }
        bool stopwatch(period const& p)
        {
            if (watch > p)
            {
                watch = period::zero();
                return true;
            }
            else
            {
                return faux;
            }
        }
        void cancel()
        {
            alive = false;
            synch.notify_all();

            if (fiber.joinable())
            {
                fiber.join();
            }
        }
        ~quartz()
        {
            cancel();
        }
    };

    // quartz: Cyclic item logger.
    template<class ITEM_T>
    class tail
    {
        struct record
        {
            moment time;
            ITEM_T item;
        };

        using report = std::vector<record>;

        report hist;
        size_t iter;
        size_t size;

        auto const& at(size_t i) const
        {
            return hist[iter >= i ? iter - i
                                  : iter + (size - i)];
        }

    public:
        period span; // tail: Period of time to be stored.
        period mint; // tail: The minimal period of time between the records stored.

        tail(period const& span, period const& mint)
            : size { 1    },
              iter { 0    },
              span { span },
              mint { mint }
        {
            hist.resize(1, { tempus::now(), ITEM_T{} });
        }

        // tail: Add new value and current time.
        void set(ITEM_T const& item)
        {
            auto now = tempus::now();

            if (++iter == size)
            {
                auto& first = hist.front();

                if (now - first.time < span)
                {
                    hist.push_back({ now, item });
                    size++;
                }
                else
                {
                    iter = 0;
                    first.time = now;
                    first.item = item;
                }
            }
            else
            {
                auto& rec = hist[iter];
                rec.time = now;
                rec.item = item;
            }
        }
        // tail: Return last value.
        auto& get() const
        {
            return hist[iter].item;
        }
        // tail: Shrink tail size to 1 and free memory.
        void dry()
        {
            size = 1;
            iter = 0;
            auto rec = hist[iter];
            hist.resize(1, rec);
            hist.shrink_to_fit();
        }
        // tail: Average velocity factors.
        auto avg()
        {
            struct velocity_factors
            {
                moment t0 = tempus::now();
                period dT = period::zero();
                ITEM_T dS = ITEM_T{};// ITEM_T::zero;
            } v;

            auto count = 0_sz;
            auto until = v.t0 - span;

            for (auto i = 0_sz; i + 1 < size; i++)
            {
                auto& rec1 = at(i);
                auto& rec2 = at(i + 1);

                if (rec2.time > until)
                {
                    v.dS += rec1.item;
                    //v.dT += rec1.time - rec2.time;
                    v.dT += std::max(rec1.time - rec2.time, mint);
                    count++;
                }
                else
                {
                    break;
                }
            }

            if (count < 2)
            {
                v.dS = ITEM_T{};// ITEM_T::zero;
            }

            return v;
        }
        template<class LAW>
        auto fader(period spell)
        {
            auto v0 = avg();

            auto speed = v0.dS;
            auto start = datetime::round<iota>(v0.t0);
            auto cycle = datetime::round<iota>(v0.dT);
            auto limit = datetime::round<iota>(spell);

            return speed
                //todo use current item's type: LAW<ITEM_T>
                //? std::optional<LAW<ITEM_T>>({ speed, cycle, limit, start })
                ? std::optional<LAW>({ speed, cycle, limit, start })
                : std::nullopt;
        }
    };
}

#endif // NETXS_QUARTZ_HPP