// Copyright (c) NetXS Group.
// Licensed under the MIT license.

// 2023
// auto logger = netxs::logger::attach([&](auto& a) { ipc.write_message(a); }, //  1st logger proc
//                                     file_write,                             //  2nd logger proc
//                                     ...         );                          //  Nth logger proc
// log("Text message with %parameter1 and %parameter2. ", p1.str(), p2.str(), rest.str());
// 
#pragma once

#include <vector>
#include <mutex>
#include <functional>
#include <unordered_map>

namespace netxs
{
    struct logger
    {
        using lock = std::mutex;
        using sync = std::lock_guard<lock>;
        using hash = void*;
        using vect = std::vector<std::function<void(view)>>;
        using depo = std::unordered_map<hash, vect>;

        static auto globals()
        {
            struct vars
            {
                friend struct guard;
                lock mutex{};
                flux input{};
                text block{};
                bool quiet{};
                depo procs{};
            };

            struct guard : sync
            {
                flux& input;
                text& block;
                bool& quiet;
                depo& procs;

                guard(vars& inst)
                    : sync{ inst.mutex },
                     input{ inst.input },
                     block{ inst.block },
                     quiet{ inst.quiet },
                     procs{ inst.procs }
                { }

                void reset()
                {
                    input = {};
                    block = {};
                    quiet = {};
                    procs = {};
                }
                void flush()
                {
                    block += utf::change(input.str(), "\n", "\r\n"); // We have disabled console post-processing.
                    if (procs.size())
                    {
                        auto shadow = view{ block };
                        for (auto& subset : procs)
                        for (auto& writer : subset.second)
                        {
                            writer(shadow);
                        }
                        block.clear();
                    }
                    input.str({});
                }
                void checkin(hash owner, vect&& proc_list)
                {
                    procs[owner] = std::move(proc_list);
                    if (block.size()) flush();
                }
                void checkout(hash owner)
                {
                    procs.erase(owner);
                }
            };

            static auto inst = vars{};
            return guard{ inst };
        }

        template<class ...Args>
        logger(Args&&... proc_list)
        {
            auto state = globals();
            state.checkin(this, { std::forward<Args>(proc_list)... });
        }
       ~logger()
        {
            auto state = globals();
            state.checkout(this);
        }

        template<class ...Args>
        static auto attach(Args&&... proc_list)
        {
            return ptr::shared<netxs::logger>(std::forward<Args>(proc_list)...);
        }
        static void enabled(bool active)
        {
            auto state = globals();
            state.quiet = !active;
        }
        static void wipe()
        {
            auto state = globals();
            state.block.clear();
        }
    };
}

namespace
{
    namespace
    {
        netxs::view crop(netxs::view& format)
        {
            static constexpr auto delimiter = '%';
            auto crop = format;
            auto head = format.find(delimiter);
            if (head == netxs::text::npos) format = {};
            else
            {
                auto tail = format.find(delimiter, head + 1);
                if (tail != netxs::text::npos)
                {
                    crop = format.substr(0, head); // Take leading substring.
                    format.remove_prefix(tail + 1);
                }
            }
            return crop;
        }
        void print(auto& input, netxs::view& format)
        {
            input << format;
        }
        void print(auto& input, netxs::view& format, auto&& arg, auto&& ...args)
        {
            input << crop(format) << std::forward<decltype(arg)>(arg);
            if (format.length()) print(input, format, std::forward<decltype(args)>(args)...);
            else                (void)(input << ...<< std::forward<decltype(args)>(args));
        }
    }
    template<bool Newline = true, class ...Args>
    void log(netxs::view format, Args&&... args)
    {
        auto state = netxs::logger::globals();
        if (state.quiet) return;
        print(state.input, format, std::forward<Args>(args)...);
        if constexpr (Newline) state.input << '\n';
        state.flush();
    }
}