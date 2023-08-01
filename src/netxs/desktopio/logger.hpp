// Copyright (c) NetXS Group.
// Licensed under the MIT license.

// 2023
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
        using hash = void*;
        using vect = std::vector<std::function<void(view)>>;
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
                void handoff(hash new_owner, hash old_owner)
                {
                    auto mapnh = procs.extract(old_owner);
                    mapnh.key() = new_owner;
                    procs.insert(std::move(mapnh));
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

        logger(logger&& l)
        {
            auto state = globals();
            state.handoff(this, &l);
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