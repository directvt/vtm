// Copyright (c) NetXS Group.
// Licensed under the MIT license.

// Init:
// auto logger = netxs::logger{ [&](auto& a) { ipc.write_message(a); }, //  1st logger proc
//                              file_write,                             //  2nd logger proc
//                              ...         };                          //  Nth logger proc

#pragma once

#include <vector>
#include <mutex>
#include <functional>
#include <unordered_map>
#include <utility>

namespace netxs
{
    struct logger
    {
        using lock = std::mutex;
        using sync = std::lock_guard<lock>;
        using type = std::function<void(view)>;
        using hash = type*;
        using vect = std::vector<type>;
        using depo = std::unordered_map<hash, vect>;

        static auto globals()
        {
            class vars
            {
                friend class guard;
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
                void flush(bool newline = faux)
                {
                    block += input.str();
                    if (newline) block += '\n';
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
                void checkin(vect& proc_list)
                {
                    auto token = proc_list.data();
                    procs[token] = proc_list;
                    if (block.size()) flush();
               }
                void checkout(vect& proc_list)
                {
                    auto token = proc_list.data();
                    procs.erase(token);
                }
            };

            static auto inst = vars{};
            return guard{ inst };
        }

        vect procs;

        logger(logger&& l)
            : procs{ std::move(l.procs) }
        { }
        template<class ...Args>
        logger(Args&&... proc_list)
            : procs{{ std::forward<Args>(proc_list)... }}
        {
            auto state = globals();
            state.checkin(procs);
        }
       ~logger()
        {
            auto state = globals();
            state.checkout(procs);
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
    template<bool Newline = true, class ...Args>
    void log(Args&&... args)
    {
        auto state = netxs::logger::globals();
        if (!state.quiet)
        {
            (state.input << ... << std::forward<Args>(args));
            state.flush(Newline);
        }
    }
}