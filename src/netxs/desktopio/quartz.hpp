// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

#include "intmath.hpp"

namespace netxs
{
    using span = std::chrono::steady_clock::duration;
    using time = std::chrono::time_point<std::chrono::steady_clock>;
    using namespace std::chrono_literals;

    static constexpr auto maxspan = span::max();
    static constexpr auto maxtime = time{ maxspan };
}

namespace netxs::datetime
{
    // quartz: Round a chrono time moment in degree (def: milliseconds)
    //         units since epoch.
    template<class T, class degree = std::chrono::milliseconds>
    constexpr T round(time t)
    {
        return netxs::saturate_cast<T>(std::chrono::duration_cast<degree>(t.time_since_epoch()).count());
    }
    // quartz: Round a chrono time period in degree (def: milliseconds).
    template<class T, class degree = std::chrono::milliseconds>
    constexpr T round(span t)
    {
        return netxs::saturate_cast<T>(std::chrono::duration_cast<degree>(t).count());
    }

    // quartz: Return a total count degree unts (def: milliseconds) since epoch.
    template<class T, class degree = std::chrono::milliseconds>
    T _now()
    {
        return round<T, degree>(std::chrono::steady_clock::now());
    }
    // quartz: Return current moment.
    auto now()
    {
        return std::chrono::steady_clock::now(); // Note: steady_clock does not contain the current time (it is a monotonic clock).
    }
    // quartz: Return unique ui64 id.
    auto uniqueid()
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(now().time_since_epoch()).count();
    }
    auto breakdown(span t)
    {
        auto days         = datetime::round<ui32, std::chrono::        days>(t);
        auto hours        = datetime::round<ui32, std::chrono::       hours>(t -= std::chrono::        days{ days    });
        auto minutes      = datetime::round<ui32, std::chrono::     minutes>(t -= std::chrono::       hours{ hours   });
        auto seconds      = datetime::round<ui32, std::chrono::     seconds>(t -= std::chrono::     minutes{ minutes });
        auto milliseconds = datetime::round<ui32, std::chrono::milliseconds>(t -= std::chrono::     seconds{ seconds });
        auto microseconds = datetime::round<ui32, std::chrono::microseconds>(t -= std::chrono::milliseconds{ milliseconds });
        return std::tuple{ days, hours, minutes, seconds, milliseconds, microseconds };
    }
    auto breakdown(time t)
    {
        auto d = t.time_since_epoch();
        auto hours        = datetime::round<ui32, std::chrono::       hours>(d);
        auto minutes      = datetime::round<ui32, std::chrono::     minutes>(d -= std::chrono::       hours{ hours        });
        auto seconds      = datetime::round<ui32, std::chrono::     seconds>(d -= std::chrono::     minutes{ minutes      });
        auto milliseconds = datetime::round<ui32, std::chrono::milliseconds>(d -= std::chrono::     seconds{ seconds      });
        auto microseconds = datetime::round<ui32, std::chrono::microseconds>(d -= std::chrono::milliseconds{ milliseconds });
        return std::tuple{ hours % 24, minutes, seconds, milliseconds, microseconds };
    }
    auto milliseconds(time t)
    {
        auto period = t.time_since_epoch();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(period).count();
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(period -= std::chrono::seconds{ seconds });
        return milliseconds;
    }

    template<class T>
    class quartz
    {
        using cond = std::condition_variable;
        using work = std::thread;

        T&   owner;
        flag alive;
        flag letup;
        span delay;
        span watch;
        span pulse;
        work fiber;
        cond synch;

        void worker()
        {
            auto mutex = std::mutex{};
            auto guard = std::unique_lock{ mutex };

            auto now = datetime::now();
            auto prior = now;

            while (alive)
            {
                watch += now - prior;
                prior =  now;

                now = datetime::now();
                owner.timer(now);

                if (letup.exchange(faux))
                {
                    synch.wait_for(guard, delay);
                    delay = span::zero();
                }
                else
                {
                    auto trail = pulse - now.time_since_epoch() % pulse;
                    synch.wait_for(guard, trail);
                }
            }
        }

    public:
        quartz(T& owner)
            : owner{ owner        },
              alive{ faux         },
              letup{ faux         },
              delay{ span::zero() },
              watch{ span::zero() },
              pulse{ span::max()  }
        { }

        operator bool ()
        {
            return alive;
        }
        void ignite(span interval)
        {
            pulse = interval;
            if (!alive.exchange(true))
            {
                fiber = std::thread{ &quartz::worker, this };
            }
        }
        void ignite(si32 frequency)
        {
            ignite(span{ span::period::den / frequency });
        }
        void freeze(span pause2)
        {
            delay = pause2;
            letup = true;
        }
        bool stopwatch(span p)
        {
            if (watch > p)
            {
                watch = span::zero();
                return true;
            }
            else
            {
                return faux;
            }
        }
        void stop()
        {
            if (alive.exchange(faux))
            {
                synch.notify_all();
                if (fiber.joinable())
                {
                    fiber.join();
                }
            }
        }
    };

    // quartz: Cyclic item logger.
    template<class Item>
    class tail
    {
        struct record
        {
            time moment;
            Item object;
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
        span period; // tail: Period of time to be stored.
        span minint; // tail: The minimal period of time between the records stored.

        tail(span period = 75ms, span minint = 4ms) //todo unify the minint=1/fps
            : iter{ 0 },
              size{ 1 },
              period{ period },
              minint{ minint }
        {
            hist.resize(1, { datetime::now(), Item{} });
        }

        // tail: Add new value and current time.
        void set(Item const& item)
        {
            auto now = datetime::now();

            if (++iter == size)
            {
                auto& first = hist.front();

                if (now - first.moment < period)
                {
                    hist.push_back({ now, item });
                    size++;
                }
                else
                {
                    iter = 0;
                    first.moment = now;
                    first.object = item;
                }
            }
            else
            {
                auto& rec = hist[iter];
                rec.moment = now;
                rec.object = item;
            }
        }
        // tail: Update last time stamp.
        void set()
        {
            set(hist[iter].object);
        }
        // tail: Return last value.
        auto& get() const
        {
            return hist[iter].object;
        }
        // tail: Return last time stamp.
        auto& stamp() const
        {
            return hist[iter].moment;
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
                span dT = span::zero();
                Item dS = Item{};// Item_t::zero;
            } v;

            auto count = 0_sz;
            auto until = datetime::now() - period;

            for (auto i = 0_sz; i + 1 < size; i++)
            {
                auto& rec1 = at(i);
                auto& rec2 = at(i + 1);

                if (rec2.moment > until)
                {
                    v.dS += rec1.object;
                    v.dT += std::max(rec1.moment - rec2.moment, minint);
                    count++;
                }
                else
                {
                    break;
                }
            }

            if (count < 2)
            {
                v.dS = Item{};
            }

            return v;
        }
        template<class Law>
        auto fader(span spell)
        {
            auto v0 = avg();

            auto speed = v0.dS;
            auto cycle = datetime::round<si32>(v0.dT);
            auto limit = datetime::round<si32>(spell);
            auto start = 0;

            return speed
                //todo use current item's type: Law<Item_t>
                //? std::optional<Law<Item_t>>({ speed, cycle, limit, start })
                ? std::optional<Law>({ speed, cycle, limit, start })
                : std::nullopt;
        }
    };
}