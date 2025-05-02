// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

#include "geometry.hpp"
#include "lua.hpp"

//todo Workaround for i386 linux targets, https://sourceware.org/bugzilla/show_bug.cgi?id=31775
#if defined(__i386__) && defined(__linux__)
    extern long double fmodl(long double a, long double b);
    double fmod(double a, double b) { return fmodl(a, b); }
    float  fmod(float  a, float  b) { return fmodl(a, b); }
#endif

namespace netxs::ui
{
    struct base;
}

namespace netxs::events
{
    struct tier // Fixed order.
    {
        static constexpr auto counter = __COUNTER__ + 1;
        static constexpr auto general = __COUNTER__ - counter; // events: Run forwrad handlers for all objects. Preserve subscription order.
        static constexpr auto release = __COUNTER__ - counter; // events: Run forwrad handlers with fixed param. Preserve subscription order.
        static constexpr auto preview = __COUNTER__ - counter; // events: Run reverse handlers with fixed a param intended to change. Preserve subscription order.
        static constexpr auto request = __COUNTER__ - counter; // events: Run forwrad a handler that provides the current value of the param. To avoid being overridden, the handler should be the only one. Preserve subscription order.
        static constexpr auto anycast = __COUNTER__ - counter; // events: Run reverse handlers along the entire visual tree. Preserve subscription order.
        static constexpr auto unknown = __COUNTER__ - counter; // events: .
        static constexpr auto str = std::to_array({ "general"sv,
                                                    "release"sv,
                                                    "preview"sv,
                                                    "request"sv,
                                                    "anycast"sv,
                                                    "unknown"sv, });
    };

    /*************************************************************************************************
    toplevel = 0

    32 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9  8  7  6  5  4  3  2  1
    0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  1  0
    =  =  =  =

    level = toplevel + 4 = 4
    msg = 98

    32 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9  8  7  6  5  4  3  2  1
    0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  1  1  0  0  1  0
    =  =  =  =
    **************************************************************************************************/

    static constexpr auto block = 4;

    // events: Return event level by its ID. Find the log base 2**block.
    static constexpr inline auto level(hint event)
    {
        if (event == 0) return 0;
        auto level = 1;
        while (event >>= block) { ++level; }
        return level;
    }
    // events: Return event level mask by its ID. Find the log base 2**block.
    static constexpr inline hint level_mask(hint event, si32 level = 0)
    {
        while (event >>= block) { level += block; }
        return (1 << level) - 1;
        //constexpr auto c = __COUNTER__ + 1;
        //if (!(event >>= block)) return (1 << (__COUNTER__ - c) * block) - 1;
        //if (!(event >>= block)) return (1 << (__COUNTER__ - c) * block) - 1;
        //if (!(event >>= block)) return (1 << (__COUNTER__ - c) * block) - 1;
        //if (!(event >>= block)) return (1 << (__COUNTER__ - c) * block) - 1;
        //if (!(event >>= block)) return (1 << (__COUNTER__ - c) * block) - 1;
        //if (!(event >>= block)) return (1 << (__COUNTER__ - c) * block) - 1;
        //if (!(event >>= block)) return (1 << (__COUNTER__ - c) * block) - 1;
        //if (!(event >>= block)) return (1 << (__COUNTER__ - c) * block) - 1;
        //return std::numeric_limits<hint>::max();
    }
    // events: Return true if the event belongs to the branch.
    static constexpr inline auto subevent(hint event, hint branch)
    {
        return (event & level_mask(branch, block)) == branch;
    }
    // events: Return event index inside the group.
    static constexpr inline auto subindex(hint event)
    {
        auto offset = level(event) * block;
        auto number = (si32)((event >> (offset - block)) - 1);
        return number;
    }
    // events: Return event id by group + index.
    static constexpr inline auto makeid(hint group, hint index)
    {
        auto offset = level(group) * block;
        auto entity = group | ((index + 1) <<  offset);
        return entity;
    }
    template<hint Event>             constexpr auto offset = level(Event) * block;                         // events: Item/msg bit shift.
    template<hint Event>             constexpr auto parent = Event & ((1 << (offset<Event> - block)) - 1); // events: Event group ID.
    template<hint Event>             constexpr auto number = (Event >> (offset<Event> - block)) - 1;       // events: Item index inside the group by its ID.
    template<hint Group, auto Index> constexpr auto entity = Group | ((Index + 1) <<  offset<Group>);      // events: Event ID of the specified item inside the group.

    template<hint Group, auto... Index>
    constexpr auto _instantiate(std::index_sequence<Index...>)
    {
        return std::array<hint, sizeof...(Index)>{ entity<Group, Index>... };
    }
    template<hint Group, auto Count> constexpr auto subset = _instantiate<Group>(std::make_index_sequence<Count>{});

    template<class Arg>
    using fx = std::function<void(Arg&)>;

    template<class Arg, class FxBase>
    struct fxwrapper : FxBase, fx<Arg>
    {
        fxwrapper(fx<Arg>&& proc)
            : fx<Arg>{ std::move(proc) }
        { }
        //fxwrapper(sptr<text> script_ptr)
        //    : FxBase{ script_ptr }
        //{ }
    };

    struct fxbase
    {
        //sptr<text> script_ptr;
        //fxbase() = default;
        //fxbase(sptr<text> script_ptr)
        //    : script_ptr{ script_ptr }
        //{ }
        virtual ~fxbase() = default;

        template<class Arg>
        void call(Arg& param)
        {
            auto& proc = *static_cast<fxwrapper<Arg, fxbase>*>(this);
            proc(param);

            //if (script_ptr)
            //{
            //    log("run script: ", ansi::hi(*script_ptr), " with param: ", &param);
            //    //move to base::signal: run_script(*script);
            //}
            //else
            //{
            //    auto& proc = *static_cast<fxwrapper<Arg, fxbase>*>(this);
            //    proc(param);
            //}
        }
    };

    struct hook : sptr<fxbase>
    {
        using sptr<fxbase>::sptr;
        auto& operator - (si32) { return *this; }

        template<class ...F>
        hook(std::shared_ptr<F...> proc_ptr)
            : sptr<fxbase>{ proc_ptr }
        { }
        template<class F>//, class Arg = ptr::arg0<F>>
        hook(F proc)
            : sptr<fxbase>{ std::make_shared<fxwrapper<ptr::arg0<F>, fxbase>>(std::move(proc)) }
        { }
        //hook(sptr<text> script_ptr)
        //    : sptr<fxbase>{ std::make_shared<fxwrapper<fx<void*>, fxbase>>(script_ptr) }
        //{ }
    };

    using wook = wptr<fxbase>;

    struct auth
    {
        struct reactor
        {
            using umap = std::unordered_map<hint, std::list<wptr<fxbase>>>;

            // Forward execution order: Execute concrete event  first. Forward means from particular to general: 1. event::group::item, 2. event::group::any
            // Reverse execution order: Execute global   events first. Reverse means from general to particular: 1. event::group::any,  2. event::group::item
            auth& indexer;   // reactor: .
            bool  order;     // reactor: Execution order. True means Forward.
            umap  stock;     // reactor: Handlers repository.

            void cleanup(ui64& ref_count, ui64& del_count)
            {
                auto lref = ui64{};
                auto ldel = ui64{};
                for (auto& [event, subs] : stock)
                {
                    auto refs = subs.size();
                    subs.remove_if([](auto&& a){ return a.expired(); });
                    auto size = subs.size();
                    lref += size;
                    ldel += refs - size;
                }
                ref_count += lref;
                del_count += ldel;
            }
            template<class Arg>
            hook subscribe(hint event, fx<Arg>&& proc)
            {
                auto proc_ptr = std::make_shared<fxwrapper<Arg, fxbase>>(std::move(proc));
                stock[event].push_back(proc_ptr);
                return proc_ptr;
            }
            void subscribe_copy(hint event, hook& proc_ptr)
            {
                stock[event].push_back(proc_ptr);
            }
        };

        struct callstate
        {
            static constexpr auto _counter    = __COUNTER__ + 1;
            static constexpr auto proceed     = __COUNTER__ - _counter;
            static constexpr auto fullstop    = __COUNTER__ - _counter;
            static constexpr auto not_handled = __COUNTER__ - _counter;
        };

        id_t                                     newid{};
        wptr<ui::base>                           empty;
        std::recursive_mutex                     mutex;
        std::unordered_map<id_t, wptr<ui::base>> store;
        generics::jobs<wptr<ui::base>>           agent;
        reactor                                  general{ *this, true };
        lua_State*                               lua;
        si32                                     fps{};
        hook                                     memo;
        datetime::quartz<auth>                   quartz;
        hint                                     e2_timer_tick_id;
        si32                                     handled{}; // auth: Last notify operation result.
        std::vector<std::pair<hint, si32>>       queue; // auth: Event queue: { event_id, call state }.
        std::vector<wptr<fxbase>>                qcopy; // auth: Copy of the current pretenders to exec on current event.

        auth(lua_State* lua = {}, hint e2_config_fps_id = {}, hint e2_timer_tick_id = {})
            : lua{ lua },
              quartz{ *this },
              e2_timer_tick_id{ e2_timer_tick_id }
        {
            if (e2_config_fps_id)
            {
                memo = general.subscribe(e2_config_fps_id, fx<si32>{ [&](si32& new_fps)
                {
                    if (new_fps > 0)
                    {
                        fps = new_fps;
                        quartz.ignite(fps);
                        log(prompt::auth, "Rendering refresh rate: ", fps, " fps");
                    }
                    else if (new_fps < 0)
                    {
                        new_fps = fps;
                    }
                    else
                    {
                        quartz.stop();
                    }
                }});
            }
        }

        // auth: .
        void _refresh_and_copy(auto& target)
        {
            target.remove_if([&](auto& a){ return a.expired() ? true : (qcopy.emplace_back(a), faux); });
        }
        // auth: .
        auto _select(auto& stock, hint event, bool order)
        {
            auto head = qcopy.size();
            if (order)
            {
                auto itermask = events::level_mask(event);
                auto subgroup = event;
                _refresh_and_copy(stock[subgroup]);
                while (itermask > (1 << events::block)) // Skip root event block.
                {
                    subgroup = event & itermask;
                    itermask >>= events::block;
                    _refresh_and_copy(stock[subgroup]);
                }
            }
            else
            {
                static constexpr auto mask = hint{ (1 << events::block) - 1 };
                auto itermask = mask; // Skip root event block.
                auto subgroup = hint{};
                do
                {
                    itermask = (itermask << events::block) | mask;
                    subgroup = event & itermask;
                    _refresh_and_copy(stock[subgroup]);
                }
                while (subgroup != event);
            }
            auto tail = qcopy.size();
            return std::pair{ head, tail };
        }
        // auth: Calling delegates. Returns the number of active ones.
        void notify(auto& stock, bool order, hint event, auto& param)
        {
            auto [head, tail] = _select(stock, event, order);
            if (head != tail)
            {
                auto& state = queue.emplace_back(event, callstate::not_handled).second;
                auto iter = head;
                do
                {
                    if (auto fx_ptr = qcopy[iter].lock()) // qcopy can be reallocated.
                    {
                        state = callstate::proceed;
                        fx_ptr->call(param);
                    }
                }
                while (state != callstate::fullstop && ++iter != tail);
                qcopy.resize(head);
                queue.pop_back();
                handled = state != callstate::not_handled;
            }
            else
            {
                handled = faux;
            }
        }
        // auth: Interrupt current invocation.
        void expire()
        {
            if (queue.size())
            {
                auto& state = queue.back().second;
                state = callstate::fullstop;
            }
        }
        // auth: Bypass current invocation.
        void bypass()
        {
            if (queue.size())
            {
                auto& state = queue.back().second;
                state = callstate::not_handled;
            }
        }
        // auth: .
        auto sync()
        {
            return std::lock_guard{ mutex };
        }
        // auth: .
        auto try_sync()
        {
            struct try_sync_t : std::unique_lock<std::recursive_mutex>
            {
                using unique_lock::unique_lock;
                operator bool () { return unique_lock::owns_lock(); }
            };
            return try_sync_t{ mutex, std::try_to_lock };
        }
        // auth: .
        auto unique_lock()
        {
            return std::unique_lock{ mutex };
        }
        // auth: .
        void timer(time now)
        {
            auto lock = sync();
            notify(general.stock, general.order, e2_timer_tick_id, now);
        }
        // auth: Return sptr of the object by its id.
        template<class T = ui::base>
        auto getref(id_t id)
        {
            auto lock = sync();
            if (auto item_ptr = netxs::get_or(store, id, empty).lock())
            if (auto real_ptr = std::dynamic_pointer_cast<T>(item_ptr))
            {
                return real_ptr;
            }
            return sptr<T>{};
        }
        // auth: Create a new object of the specified subtype and return its sptr.
        template<class T, class ...Args>
        auto create(Args&&... args) -> sptr<T>
        {
            auto lock = sync();
            // Use new/delete to be able lock before destruction.
            auto inst = std::shared_ptr<T>(new T(std::forward<Args>(args)...), [](T* inst)
                                                                               {
                                                                                    auto& indexer = inst->indexer;
                                                                                    auto lock = indexer.sync(); // Sync with all dtors.
                                                                                    auto id = inst->id;
                                                                                    delete inst;
                                                                                    indexer.store.erase(id);
                                                                               });
            store[inst->id] = inst;
            return inst;
        }
        // auth: Return next available id.
        auto new_id()
        {
            while (netxs::on_key(store, ++newid))
            { }
            return newid;
        }
        // auth: .
        template<bool Sync = true>
        void enqueue(wptr<ui::base> object_wptr, fx<ui::base> proc)
        {
            agent.add(object_wptr, [&, proc](auto& object_wptr) mutable
            {
                auto lock = unique_lock();
                if (auto object_ptr = object_wptr.lock())
                {
                    if constexpr (!Sync) lock.unlock();
                    proc(*object_ptr);
                    if constexpr (!Sync) lock.lock();
                }
            });
        }
        // auth: .
        void stop()
        {
            agent.stop();
            auto lock = sync();
            quartz.stop();
        }
        // auth: .
        template<class T, class P>
        auto synced(T& object_sptr, P proc)
        {
            using buff = generics::buff<text>;
            return [&, proc, buffer = buff{}](auto utf8) mutable
            {
                auto lock = buffer.freeze();
                lock.block += utf8;
                if (!lock.await)
                {
                    if (auto sync = try_sync())
                    {
                        proc(view{ lock.block });
                        lock.block.clear();
                    }
                    else
                    {
                        lock.await = true;
                        enqueue(object_sptr, [&](auto& /*boss*/)
                        {
                            auto lock = buffer.freeze();
                            lock.await = faux;
                            proc(view{ lock.block });
                            lock.block.clear();
                        });
                    }
                }
            };
        }
    };

    using subs = std::vector<hook>;
    constexpr auto& operator - (subs& tokens, si32) { return tokens; }

    struct metadata_t
    {
        hint event_id{};
        view param_typename;
    };
    auto& rtti()
    {
        static auto rttidata = std::unordered_map<text, metadata_t, qiew::hash, qiew::equal>{};
        return rttidata;
    }
    auto rtti(hint event_id, qiew event, qiew param_typename)
    {
        rtti()[event] = metadata_t{ event_id, param_typename };
        return event_id;
    }
    template<class Parent, auto Event_str, auto Event_id = hint{}, class Type = si32, auto Type_str = netxs::utf::cat("si32")>
    struct type_clue
    {
        struct metadata_t
        {
            struct bytes_t
            {
                static constexpr auto event = netxs::utf::cat(Parent::metadata.bytes.event, Event_str);
                static constexpr auto param = netxs::utf::cat(Type_str);
            };
            static constexpr auto bytes = bytes_t{};
            static constexpr auto event = view{ bytes.event.data(), bytes.event.size() };
            static constexpr auto param = view{ bytes.param.data(), bytes.param.size() };
        };
        static constexpr auto metadata = metadata_t{};

        using type = Type;
        using base = Parent;
        static constexpr auto id = Event_id;
        template<class ...Args> constexpr type_clue(Args&&...) { }
        template<class ...Args> static constexpr auto param(Args&&... args) { return type{ std::forward<Args>(args)... }; }
                                static constexpr auto param(type&&    arg ) { return std::move(arg);                      }
        template<auto N>        static constexpr auto group()               { return events::subset<id, N>;               }
                                static constexpr auto index()               { return events::number<id>;                  }
    };

    #define ARGsEVAL_XS(...) __VA_ARGS__
    #define ARG_EVAL_XS(...) ARGsEVAL_XS(__VA_ARGS__)
    #define GET_END1_XS(a, b, c, d, e, last, ...) last
    #define GET_END2_XS(a, b, c, d,    last, ...) last

    #define LISTEN_S(level, event, param              ) bell::submit(level, event)            = [&]                   ([[maybe_unused]] typename decltype( event )::type& param)
    #define LISTEN_T(level, event, param, token       ) bell::submit(level, event, token - 0) = [&]                   ([[maybe_unused]] typename decltype( event )::type& param)
    #define LISTEN_V(level, event, param, token, byval) bell::submit(level, event, token - 0) = [&, ARG_EVAL_XS byval]([[maybe_unused]] typename decltype( event )::type& param) mutable
    #define LISTEN_X(...) ARG_EVAL_XS(GET_END1_XS(__VA_ARGS__, LISTEN_V, LISTEN_T, LISTEN_S))
    #define LISTEN(...) LISTEN_X(__VA_ARGS__)(__VA_ARGS__)

    #define SUBEVENTS( name )          static constexpr auto _counter_base = __COUNTER__; \
                                       static constexpr auto           any = netxs::events::type_clue<decltype(name), netxs::utf::cat("::any"), decltype(name)::id, decltype(name)::type, decltype(name)::metadata.bytes.param>{}; \
                                       static           auto     _rtti_any = netxs::events::rtti(any.id, any.metadata.event, any.metadata.param); namespace
    #define EVENTPACK( name, base )    static constexpr auto         _root = netxs::events::type_clue<netxs::events::userland::seed::root, netxs::utf::cat(#name), base.id>{}; \
                                       SUBEVENTS( _root )
    #define  EVENT_XS( name, type ) }; static constexpr auto          name = netxs::events::type_clue<decltype(any)::base, netxs::utf::cat("::", #name), decltype(any)::id | ((__COUNTER__ - _counter_base) << netxs::events::offset<decltype(any)::id>), type, netxs::utf::cat(#type)>{}; \
                                       static           auto  _rtti_##name = netxs::events::rtti(name.id, name.metadata.event, name.metadata.param) + (si32)!noop{ 777
    #define  GROUP_XS( name, type ) }; static constexpr auto       _##name = netxs::events::type_clue<decltype(any)::base, netxs::utf::cat("::", #name), decltype(any)::id | ((__COUNTER__ - _counter_base) << netxs::events::offset<decltype(any)::id>), type, netxs::utf::cat(#type)>{ 777
    #define SUBSET_XS( name )       }; namespace name { SUBEVENTS( _##name )
    #define  INDEX_XS(  ... )       }; template<auto N> static constexpr \
                                    auto _ = std::get<N>( std::tuple{ __VA_ARGS__ } ); \
                                    static constexpr auto _dummy = { 777

    namespace userland
    {
        namespace seed
        {
            struct root
            {
                struct metadata_t
                {
                    struct bytes_t
                    {
                        static constexpr auto event = netxs::utf::cat("");
                        static constexpr auto param = netxs::utf::cat("");
                    };
                    static constexpr auto bytes = bytes_t{};
                };
                static constexpr auto id = hint{};
                static constexpr auto metadata = metadata_t{};
            };
            EVENTPACK( seed for root, root{} )
            {
                EVENT_XS( e2       , si32 ),
                EVENT_XS( input    , si32 ),
                EVENT_XS( custom   , si32 ),
            };
        }
    }

    // events: Event x-mitter.
    struct bell
    {
        using reactor_set = std::array<std::reference_wrapper<auth::reactor>, 5>;

        static constexpr auto noid = std::numeric_limits<id_t>::max();

        auth&          indexer;
        auth::reactor& general;
        const id_t     id;      // bell: Object id.
        subs           sensors; // bell: Event subscriptions.
        auth::reactor  release{ indexer, true };
        auth::reactor  preview{ indexer, faux };
        auth::reactor  request{ indexer, true };
        auth::reactor  anycast{ indexer, faux };
        reactor_set    reactors = { std::ref(general),
                                    std::ref(release),
                                    std::ref(preview),
                                    std::ref(request),
                                    std::ref(anycast) };

        template<class Event, class Arg = Event::type>
        struct submit_helper
        {
            bell& owner;
            si32  level;
            submit_helper(si32 level, bell& owner)
                : owner{ owner },
                  level{ level }
            { }
            void operator = (fx<Arg> handler)
            {
                owner.submit(level, Event{}, std::move(handler));
            }
        };
        template<class Event, class Arg = Event::type>
        struct submit_helper_token
        {
            bell& owner;
            hook& token;
            si32  level;
            submit_helper_token(si32 level, bell& owner, hook& token)
                : owner{ owner },
                  token{ token },
                  level{ level }
            { }
            void operator = (fx<Arg> handler)
            {
                owner.submit(level, Event{}, token, std::move(handler));
            }
        };

    public:
        auto& router(si32 Tier)
        {
            return reactors[std::clamp(Tier, 0, tier::unknown - 1)].get();
        }
        template<class Event> auto submit(si32 Tier, Event)               { return submit_helper      <Event>(Tier, *this);                        }
        template<class Event> auto submit(si32 Tier, Event, si32)         { return submit_helper      <Event>(Tier, *this);                        }
        template<class Event> auto submit(si32 Tier, Event, hook& token)  { return submit_helper_token<Event>(Tier, *this, token);                 }
        template<class Event> auto submit(si32 Tier, Event, subs& tokens) { return submit_helper_token<Event>(Tier, *this, tokens.emplace_back()); }
        template<class Event, class Arg = Event::type>
        void submit(si32 Tier, Event, fx<Arg>&& handler)
        {
            auto lock = indexer.sync();
            sensors.push_back(router(Tier).subscribe(Event::id, std::move(handler)));
        }
        void submit_generic(si32 Tier, si32 event_id, sptr<text> script_ptr) // Generic event handler. //todo set script body instead of fx<char>
        {
            auto lock = indexer.sync();
            auto handler = fx<char>{ [&, script_ptr](auto& n)
            {
                auto& t = *reinterpret_cast<time*>(&n);
                log("calling script: n=%% ", utf::to_hex_0x(t.time_since_epoch().count()), ansi::hi(*script_ptr));
            }};
            sensors.push_back(router(Tier).subscribe(event_id, std::move(handler)));
        }
        template<class Event, class Arg = Event::type>
        void submit(si32 Tier, Event, hook& token, fx<Arg>&& handler)
        {
            auto lock = indexer.sync();
            token = router(Tier).subscribe(Event::id, std::move(handler));
        }
        template<class Event>
        void dup_handler(si32 Tier, Event, hook& token)
        {
            auto lock = indexer.sync();
            router(Tier).subscribe_copy(Event::id, token);
        }
        template<class Event>
        void dup_handler(si32 Tier, Event)
        {
            auto lock = indexer.sync();
            if (sensors.size())
            {
                router(Tier).subscribe_copy(Event::id, sensors.back());
            }
        }
        void _signal(si32 Tier, hint event, auto& param)
        {
            auto& r = router(Tier);
            indexer.notify(r.stock, r.order, event, param);
        }
        auto accomplished()
        {
            return indexer.handled;
        }
        // bell: Return initial event of the current event execution branch.
        auto protos()
        {
            return indexer.queue.empty() ? hint{} : indexer.queue.back().first;
        }
        template<class Event>
        auto protos(Event)
        {
            return bell::protos() == Event::id;
        }
        void expire()
        {
            indexer.expire();
        }
        void passover()
        {
            indexer.bypass();
        }
        // bell: Create a new object of the specified subtype and return its sptr.
        template<class T, class ...Args>
        auto create(Args&&... args) -> sptr<T>
        {
            return indexer.create<T>(indexer, std::forward<Args>(args)...);
        }
        // bell: .
        void dequeue()
        {
            indexer.stop();
        }
        // bell: .
        template<bool Sync = true, class ...Args>
        void enqueue(Args&&... args)
        {
            indexer.enqueue<Sync>(std::forward<Args>(args)...);
        }
        // bell: .
        auto sync()
        {
            return indexer.sync();
        }
        // bell: .
        auto try_sync()
        {
            return indexer.try_sync();
        }
        // bell: .
        auto unique_lock()
        {
            return indexer.unique_lock();
        }
        // bell: Return sptr of the object by its id.
        template<class T = bell>
        auto getref(id_t id)
        {
            return indexer.getref<T>(id);
        }

        bell(auth& indexer)
            : indexer{ indexer },
              general{ indexer.general },
              id{ indexer.new_id() }
        { }
        virtual ~bell() = default;
    };
}
namespace netxs
{
    using netxs::events::bell;
    using netxs::events::subs;
    using netxs::events::tier;
    using netxs::events::hook;
    using netxs::events::wook;
}