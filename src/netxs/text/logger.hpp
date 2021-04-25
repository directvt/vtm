// Copyright (c) NetXS Group.
// Licensed under the MIT license.

// Usage (C++17) 2013
//
// Two (or more) log targets example:
// utils::logger logger( [&](auto& a) { ipc.write_message(a); }, //  1st logger proc
//                       file_write,                             //  2nd logger proc
//                       ...);                                   //  Nth logger proc
//
// User defined formatter example:
// utils::logger::custom(   [](auto& p, auto& v)
// {
//      return utils::current_short_date_time_with_ms() + "  " + utils::concat(p, std::string("."), std::string("> ")) + v + '\n';
// });
//
// Automatic prompt changing example:
// void proc(...) {
//      AUTO_PROMPT;                                // auto prompt
//      ...code...
//      {
//          utils::logger::prompt p("subprompt");   // nested prompt
//          ...code...
//      }
//      ...code...
//      Z("log message: ", some_data);              // prompted output
//      ...code...
//      Z("log message: ", some_data, false);       // promptless output
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

namespace netxs::logger
{
    using namespace std;
    using loggers_set = unordered_map<size_t, vector<function<void(string const&) > >>;
    using func = function< string(vector<string> const&, string const&) >;

    class logger
    {
        template<class VOID>
        struct _globals
        {
            static  vector<string> _prompt;
            static            func _formatter;
            static    stringstream _builder;
            static recursive_mutex _mutex;
            static          string _buffer;
            static            bool _enabled;
            static     loggers_set _all_writers;
        };

        using _g = _globals<void>;

    public:
        static void custom(func formatter)
        {
            unique_lock<recursive_mutex> lock(_g::_mutex);
            _g::_formatter = formatter;
        }

        static void enabled(bool allowed)
        {
            unique_lock<recursive_mutex> lock(_g::_mutex);
            _g::_enabled = allowed;
        }

        struct prompt
        {
            prompt(std::string const& new_prompt)
            {
                unique_lock<recursive_mutex> lock(_g::_mutex);
                if (new_prompt.size())
                {
                    _g::_prompt.push_back(new_prompt);
                }
            }

            ~prompt()
            {
                unique_lock<recursive_mutex> lock(_g::_mutex);
                if (_g::_prompt.size())
                {
                    _g::_prompt.pop_back();
                }
            }
        };

        template <class T>
        logger(T writer)
        {
            std::unique_lock<std::recursive_mutex> lock(_g::_mutex);
            _add(writer);
        }
        template<class T, class ...Args>
        logger(T writer, Args&&... args)
        {
            std::unique_lock<std::recursive_mutex> lock(_g::_mutex);
            _writers.push_back(writer);
            _add(std::forward<Args>(args)...);
        }
        ~logger()
        {
            std::unique_lock<std::recursive_mutex> lock(_g::_mutex);
            _checkout(_token);
            _writers.clear();
        }

        template <class T>
        static void feed(T&& entity)
        {
            std::unique_lock<std::recursive_mutex> lock(_g::_mutex);
            _g::_builder << entity;
            _flush(true);
        }
        static void feed(bool prompted)
        {
            std::unique_lock<std::recursive_mutex> lock(_g::_mutex);
            _flush(prompted);
        }
        template<class T, class ...Args>
        static void feed(T&& entity, Args&&... args)
        {
            std::unique_lock<std::recursive_mutex> lock(_g::_mutex);
            _g::_builder << entity;
            feed(std::forward<Args>(args)...);
        }

    private:
        template<class T>
        using delegates = std::vector< std::function< void(T const&) > >;

        delegates<std::string> _writers;
                        size_t _token;

        size_t _checkin(delegates<string>& writers)
        {
            auto hash = reinterpret_cast<size_t>(writers.data());
            _g::_all_writers[hash] = writers;

            return hash;
        }
        void _checkout(size_t token)
        {
            _g::_all_writers.erase(token);
        }

        static string _form(string const& value)
        {
            if (_g::_formatter)
            {
                return _g::_formatter(_g::_prompt, value);
            }
            else
            {
                string result;
                if (_g::_prompt.size())
                {
                    result = _g::_prompt.back() + '>' + ' ';
                }
                result += value + '\n';
                return result;
            }
        }

        static void _flush(bool prompted)
        {
            if (_g::_enabled)
            {
                if (prompted)
                {
                    _g::_buffer += _form(_g::_builder.str());
                }
                else
                {
                    _g::_buffer += _g::_builder.str();
                }

                if (_g::_all_writers.size())
                {
                    for (auto& subset : _g::_all_writers)
                    {
                        for (auto writer : subset.second)
                        {
                            writer(_g::_buffer);
                        }
                    }
                    _g::_buffer.clear();
                }
            }

            _g::_builder.str(string());
            _g::_builder.clear();
        }

        template <class T>
        void _add(T&& writer)
        {
            _writers.push_back(writer);
            _token = _checkin(_writers);
            if (_g::_builder.tellg() > 0)
            {
                _flush(false);
            }
        }
        template<class T, class ...Args>
        void _add(T writer, Args&&... args)
        {
            _writers.push_back(writer);
            _add(std::forward<Args>(args)...);
        }

    };

    template<class T> bool                     logger::_globals<T>::_enabled{ true };
    template<class T> std::string              logger::_globals<T>::_buffer;
    template<class T> std::stringstream        logger::_globals<T>::_builder;
    template<class T> std::recursive_mutex     logger::_globals<T>::_mutex;
    template<class T> std::vector<std::string> logger::_globals<T>::_prompt;
    template<class T> loggers_set              logger::_globals<T>::_all_writers;
    template<class T> func                     logger::_globals<T>::_formatter;
}

namespace
{
    template<class ...Args>
    void Z(Args&&... args)
    {
        netxs::logger::logger::feed(std::forward<Args>(args)...);
    }

    template<class ...Args>
    void log(Args&&... args)
    {
        netxs::logger::logger::feed(std::forward<Args>(args)...);
    }

    //todo revise
    //template<>
    //void Z(bool args)
    //{
    //	utils::logger::logger::feed(args);
    //}
}

#endif // NETXS_LOGGER_HPP