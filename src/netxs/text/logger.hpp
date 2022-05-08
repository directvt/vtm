// Copyright (c) NetXS Group.
// Licensed under the MIT license.

// Usage (C++17) 2013
//
// Two (or more) log targets example:
// netxs::logger logger( [&](auto& a) { ipc.write_message(a); }, //  1st logger proc
//                       file_write,                             //  2nd logger proc
//                       ...);                                   //  Nth logger proc
//
// User defined formatter example:
// netxs::logger::custom( [](auto& p, auto& v)
// {
//      return netxs::current_short_date_time_with_ms() + "  " + utf::concat(p, text("."), text("> ")) + v + '\n';
// });
//
// Automatic prompt changing example:
// void proc(...) {
//      AUTO_PROMPT;                                // auto prompt
//      ...code...
//      {
//          netxs::logger::prompt p("subprompt");   // nested prompt
//          ...code...
//      }
//      ...code...
//      Z("log message: ", some_data);              // prompted output
//      ...code...
//      Z("log message: ", some_data, !true);       // promptless output
//      ...code...
// }

#ifndef NETXS_LOGGER_HPP
#define NETXS_LOGGER_HPP

#include <sstream>
#include <vector>
#include <mutex>
#include <functional>
#include <unordered_map>

#define AUTO_PROMPT const netxs::logger::prompt __func__##_auto_prompt(__func__)

namespace netxs
{
    class logger
    {
        using text = std::string;
        using view = std::string_view;
        using flux = std::stringstream;
        using vect = std::vector<text>;
        using lock = std::recursive_mutex;
        using type = std::function<void(view)>;
        using hash = type*;
        using list = std::vector<type>;
        using depo = std::unordered_map<hash, list>;
        using func = std::function<text(vect const&, view)>;

        list writers;
        hash token;

        template<class VOID>
        struct globals
        {
            static vect prompt;
            static func formatter;
            static flux builder;
            static lock mutex;
            static text buffer;
            static bool enabled;
            static depo all_writers;

            static auto form(text const& value)
            {
                if (formatter)
                {
                    return formatter(prompt, value);
                }
                else
                {
                    text result;
                    if (prompt.size())
                    {
                        result = prompt.back() + '>' + ' ';
                    }
                    result += value + '\n';
                    return result;
                }
            }
            static void flush(bool prompted)
            {
                if (enabled)
                {
                    if (prompted) buffer += form(builder.str());
                    else          buffer +=      builder.str();

                    if (all_writers.size())
                    {
                        for (auto& subset : all_writers)
                        {
                            for (auto& writer : subset.second)
                            {
                                writer(buffer);
                            }
                        }
                        buffer.clear();
                    }
                }
                builder.str(text{});
                builder.clear();
            }
            static auto checkin(list& writers)
            {
                //todo revise
                auto digest = writers.data();
                all_writers[digest] = writers;
                if (builder.tellg() > 0)
                {
                    flush(!true);
                }
                return digest;
            }
            static void checkout(hash token)
            {
                all_writers.erase(token);
            }
        };

        using g = globals<void>;

        static auto guard()
        {
            return std::lock_guard{ g::mutex };
        }
        template <class T>
        void add(T&& writer)
        {
            writers.emplace_back(std::forward<T>(writer));
            token = g::checkin(writers);
        }
        template<class T, class ...Args>
        void add(T&& writer, Args&&... args)
        {
            writers.emplace_back(std::forward<T>(writer));
            add(std::forward<Args>(args)...);
        }

    public:
        struct prompt
        {
            prompt(view new_prompt)
            {
                auto sync = guard();
                if (new_prompt.size())
                {
                    g::prompt.emplace_back(new_prompt);
                }
            }
            ~prompt()
            {
                auto sync = guard();
                if (g::prompt.size())
                {
                    g::prompt.pop_back();
                }
            }
        };

        logger(logger&& l)
            : writers{ std::    move(l.writers)   },
              token  { std::exchange(l.token, {}) }
        { }
        template<class ...Args>
        logger(Args&&... writers)
        {
            auto sync = guard();
            add(std::forward<Args>(writers)...);
        }
        ~logger()
        {
            auto sync = guard();
            if (token) g::checkout(token);
            writers.clear();
        }

        static void custom(func formatter)
        {
            auto sync = guard();
            g::formatter = formatter;
        }
        static void enabled(bool allowed)
        {
            auto sync = guard();
            g::enabled = allowed;
        }
        template <class T>
        static void feed(T&& entity)
        {
            auto sync = guard();
            g::builder << std::forward<T>(entity);
            g::flush(true);
        }
        static void feed(bool prompted)
        {
            auto sync = guard();
            g::flush(prompted);
        }
        template<class T, class ...Args>
        static void feed(T&& entity, Args&&... args)
        {
            auto sync = guard();
            g::builder << std::forward<T>(entity);
            feed(std::forward<Args>(args)...);
        }
        template<class SYNC, class P>
        static auto tee(P writer)
        {
            auto inst = logger([writer, buff = text{}](auto utf8) mutable
            {
                if (auto sync = SYNC{})
                {
                    if (buff.size())
                    {
                        writer(view{ buff });
                        buff.clear();
                    }
                    writer(utf8);
                }
                else buff += utf8;
            });
            return inst;
        }
    };

    template<class T> bool         logger::globals<T>::enabled{ true };
    template<class T> logger::text logger::globals<T>::buffer;
    template<class T> logger::flux logger::globals<T>::builder;
    template<class T> logger::lock logger::globals<T>::mutex;
    template<class T> logger::vect logger::globals<T>::prompt;
    template<class T> logger::depo logger::globals<T>::all_writers;
    template<class T> logger::func logger::globals<T>::formatter;
}

namespace
{
    template<class ...Args>
    void Z(Args&&... args)
    {
        netxs::logger::feed(std::forward<Args>(args)...);
    }

    template<class ...Args>
    void log(Args&&... args)
    {
        netxs::logger::feed(std::forward<Args>(args)...);
    }
}

#endif // NETXS_LOGGER_HPP