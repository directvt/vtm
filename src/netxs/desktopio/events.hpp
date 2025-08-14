// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

#include "geometry.hpp"
#include "lua.hpp"
#include "xml.hpp"

//todo Workaround for i386 linux targets, https://sourceware.org/bugzilla/show_bug.cgi?id=31775
#if defined(__i386__) && defined(__linux__)
    extern long double fmodl(long double a, long double b);
    double fmod(double a, double b) { return fmodl(a, b); }
    float  fmod(float  a, float  b) { return fmodl(a, b); }
#endif

namespace netxs
{
    namespace ui
    {
        struct base;
    }
    namespace input
    {
        struct hids;
    }
}

namespace netxs::events
{
    struct tier // Keep this enumeration in a fixed order. The last bit of its index indicates the execution order 0: Forward, 1: Reverse.
    {
        // Forward execution order: Execute concrete event  first. Preserve subscription order. Forward means from particular to general: 1. event::group::item, 2. event::group::any
        // Reverse execution order: Execute global   events first. Preserve subscription order. Reverse means from general to particular: 1. event::group::any,  2. event::group::item
        static constexpr auto counter = __COUNTER__ + 1;
        static constexpr auto release = __COUNTER__ - counter; // events: Run forwrad handlers with fixed param.
        static constexpr auto preview = __COUNTER__ - counter; // events: Run reverse handlers with fixed a param intended to change.
        static constexpr auto request = __COUNTER__ - counter; // events: Run forwrad a handler that provides the current value of the param. To avoid being overridden, the handler should be the only one.
        static constexpr auto anycast = __COUNTER__ - counter; // events: Run reverse handlers along the entire visual tree.
        static constexpr auto general = __COUNTER__ - counter; // events: Run forwrad handlers for all objects.
        static constexpr auto mousepreview = __COUNTER__ - counter; // events: Run in subscription order for all objects.
        static constexpr auto mouserelease = __COUNTER__ - counter; // events: Run in subscription order for all objects.
        static constexpr auto keybdpreview = __COUNTER__ - counter; // events: Run in subscription order for all objects.
        static constexpr auto keybdrelease = __COUNTER__ - counter; // events: Run in subscription order for all objects.
        static constexpr auto unknown = __COUNTER__ - counter; // events: .
        static constexpr auto str = std::to_array({ "release"sv,
                                                    "preview"sv,
                                                    "request"sv,
                                                    "anycast"sv,
                                                    "general"sv,
                                                    "mousepreview"sv,
                                                    "mouserelease"sv,
                                                    "keybdpreview"sv,
                                                    "keybdrelease"sv,
                                                    "unknown"sv, });
        static constexpr auto order = std::to_array({ feed::fwd,
                                                      feed::rev,
                                                      feed::fwd,
                                                      feed::rev,
                                                      feed::fwd,
                                                      feed::none,
                                                      feed::none,
                                                      feed::none,
                                                      feed::none, });
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

    using context_t = std::vector<void*>;
    struct auth;

    // events: Lua scripting.
    struct luna
    {
        auth&      indexer; // luna: .
        lua_State* lua; // luna: .

        static text vtmlua_torawstring(     lua_State* lua, si32 idx, bool extended = faux);
        static si32 vtmlua_object2string(   lua_State* lua);
        static si32 vtmlua_log(             lua_State* lua);
        static si32 vtmlua_call_method(     lua_State* lua);
        static si32 vtmlua_run_with_indexer(lua_State* lua, auto proc);
        static si32 vtmlua_vtm_call(        lua_State* lua);
        static si32 vtmlua_vtm_index(       lua_State* lua);
        static si32 vtmlua_vtm_subindex(    lua_State* lua);
        static si32 vtmlua_cfg_subindex(    lua_State* lua);
        static si32 vtmlua_push_value(      lua_State* lua, auto&& v);
        si32 push_value(auto&& v);
        void set_return(auto... args);
        void set_return_array(txts const& str_list);
        si32 args_count();
        void read_args(si32 index, auto add_item);
        auto get_args_or(si32 idx, auto fallback = {});
        void set_gear(input::hids& gear);
        input::hids& get_gear();
        bool run_with_gear_wo_return(auto proc);
        void run_with_gear(auto proc);
        template<class Arg = noop>
        text run(context_t& context, view script_body, Arg&& param = {});
        text run_script(ui::base& object, view script_body);
        void run_ext_script(ui::base& object, auto& script);

        luna(auth& indexer);
        ~luna();
    };

    struct script_ref
    {
        std::reference_wrapper<context_t> context; // Hierarchical location index of the script owner.
        sptr<text>                        script_body_ptr; // Script body sptr.

        static text to_string(context_t& context);

        script_ref(context_t& context, sptr<text> script_body_ptr)
            : context{ context },
              script_body_ptr{ script_body_ptr }
        { }
        script_ref(context_t& context, auto&& script_body_ptr)
            : script_ref{ context, ptr::shared<text>(script_body_ptr) }
        { }
    };

    template<class Arg>
    using fx = std::function<void(Arg&)>;

    template<class Arg, class FxBase>
    struct fxwrapper : FxBase, fx<Arg>
    {
        fxwrapper(fx<Arg>&& proc)
            : fx<Arg>{ std::move(proc) }
        { }
        fxwrapper(sptr<script_ref> script_ptr)
            : FxBase{ script_ptr }
        { }
    };

    struct fxbase
    {
        sptr<script_ref> script_ptr;

        fxbase() = default;
        fxbase(sptr<script_ref> script_ptr)
            : script_ptr{ script_ptr }
        { }
        virtual ~fxbase() = default;

        template<class Arg>
        auto& get_inst()
        {
            return *static_cast<fxwrapper<Arg, fxbase>*>(this);
        }
        template<class Arg>
        void call(luna& luafx, Arg& param)
        {
            if (script_ptr && script_ptr->script_body_ptr)
            {
                auto& [context, script_body_ptr] = *script_ptr;
                auto& script_body = *script_body_ptr;
                luafx.run(context, script_body, param);
            }
            else if (auto& proc = get_inst<Arg>())
            {
                proc(param);
            }
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
        template<class F>
        hook(F proc)
            : sptr<fxbase>{ ptr::shared<fxwrapper<ptr::arg0<F>, fxbase>>(std::move(proc)) }
        { }
        hook(sptr<script_ref> script_ptr)
            : sptr<fxbase>{ ptr::shared<fxwrapper<fx<char>, fxbase>>(script_ptr) }
        { }
    };

    using wook = wptr<fxbase>;
    using fmap = std::unordered_map<hint, std::list<wptr<fxbase>>>; // Functor wptr-list map by event_id.
    using fxmap = utf::unordered_map<text, std::function<void()>>; // Class methods.

    // Class methods and registered instances.
    struct vtm_class
    {
        //using deque = std::deque<std::reference_wrapper<ui::base>>;
        using list = std::list<std::reference_wrapper<ui::base>>;
        list  objects; // List of references to class objects.
        fxmap methods; // Static class methods.
    };
    using clasess_umap = utf::unordered_map<text, sptr<vtm_class>>;

    struct auth
    {
        struct callstate
        {
            static constexpr auto _counter    = __COUNTER__ + 1;
            static constexpr auto proceed     = __COUNTER__ - _counter;
            static constexpr auto fullstop    = __COUNTER__ - _counter;
            static constexpr auto not_handled = __COUNTER__ - _counter;
        };

        id_t                                      next_id;
        std::recursive_mutex                      mutex;
        std::unordered_map<id_t, std::reference_wrapper<ui::base>>  objects; // auth: Map of objects by object id.
        clasess_umap                              classes; // auth: Map of classes by classname.
        context_t                                 context; // auth: Default context.
        std::reference_wrapper<context_t>         context_ref; // auth: .
        fmap                                      general;
        generics::jobs<wptr<ui::base>>            agent;
        luna                                      luafx;
        si32                                      fps{};
        hook                                      memo;
        datetime::quartz<auth>                    quartz;
        hint                                      e2_timer_tick_id;
        si32                                      handled{}; // auth: Last notify operation result.
        std::vector<std::pair<hint, si32>>        queue; // auth: Event queue: { event_id, call state }.
        std::vector<wptr<fxbase>>                 qcopy; // auth: Copy of the current pretenders to exec on current event.
        std::vector<bool>                         gear_indexing; // auth: Gear visual indexing.
        sptr<input::hids>                         _null_gear_sptr; // auth: Fallback gear sptr.
        core                                      _null_idmap; // auth: Fallback gear idmap.
        std::reference_wrapper<input::hids>       active_gear_ref; // auth: Active gear.
        std::any                                  script_param; // auth: .
        utf::unordered_map<text, hint>            keybd_chords; // auth: Registered keyboard chords.
        hint                                      chord_index{}; // auth: Next available keybd chord index.
        hint                                      anykey_event{};
        settings                                  config; // auth: Global settings.

        auto get_kbchord_hint(qiew chord)
        {
            auto iter = keybd_chords.find(chord);
            if (iter == keybd_chords.end())
            {
                iter = keybd_chords.try_emplace(chord, ++chord_index).first;
            }
            auto chord_hint = iter->second;
            return chord_hint;
        }
        auto take_gear_available_index()
        {
            auto iter = std::find(gear_indexing.begin(), gear_indexing.end(), faux);
            if (iter == gear_indexing.end()) iter = gear_indexing.emplace(iter, true);
            else                            *iter = true;
            auto n = (si32)(iter - gear_indexing.begin());
            return n;
        }
        auto release_gear_index(si32 n)
        {
            if (n >= 0 && n < (si32)gear_indexing.size()) gear_indexing[n] = faux;
            else
            {
                if constexpr (debugmode) log(prompt::host, ansi::err("Gear accounting error: ring size:", gear_indexing.size(), " gear_number:", n));
            }
        }
        void _cleanup(fmap& reactor, ui64& ref_count, ui64& del_count)
        {
            auto lref = ui64{};
            auto ldel = ui64{};
            for (auto& [event, fxlist] : reactor)
            {
                auto refs = fxlist.size();
                fxlist.remove_if([](auto&& a){ return a.expired(); });
                auto size = fxlist.size();
                lref += size;
                ldel += refs - size;
            }
            ref_count += lref;
            del_count += ldel;
        }
        auto tier_mask(si32 Tier)
        {
            return (hint)Tier << (8 * sizeof(hint) - 4); // Use the last four bits for tier.
        }
        void _subscribe_copy(si32 Tier, fmap& reactor, hint event, hook& proc_ptr)
        {
            auto& target_reactor = Tier == tier::general ? general : reactor;
            target_reactor[event | tier_mask(Tier)].push_back(proc_ptr);
        }
        template<class Arg>
        auto _subscribe(si32 Tier, fmap& reactor, hint event, fx<Arg>&& proc)
        {
            auto proc_ptr = hook{ ptr::shared<fxwrapper<Arg, fxbase>>(std::move(proc)) };
            _subscribe_copy(Tier, reactor, event, proc_ptr);
            return proc_ptr;
        }
        auto _subscribe(si32 Tier, fmap& reactor, hint event, sptr<script_ref> script_ptr)
        {
            auto proc_ptr = hook{ ptr::shared<fxwrapper<char, fxbase>>(script_ptr) };
            _subscribe_copy(Tier, reactor, event, proc_ptr);
            return proc_ptr;
        }

        auth(bool use_timer = faux);

        ui::base* get_target(context_t& source_ctx, view object_name);
        // auth: .
        void _refresh_and_copy(fmap::mapped_type& fxlist)
        {
            fxlist.remove_if([&](auto& f){ return f.expired() ? true : (qcopy.emplace_back(f), faux); });
        }
        // auth: .
        auto _select(si32 Tier, fmap& reactor, hint event, feed order)
        {
            auto tiermask = tier_mask(Tier);
            auto head = qcopy.size();
            if (order == feed::fwd)
            {
                auto itermask = events::level_mask(event);
                auto subgroup = event;
                _refresh_and_copy(reactor[subgroup | tiermask]);
                while (itermask > (1 << events::block)) // Skip root event block.
                {
                    subgroup = event & itermask;
                    itermask >>= events::block;
                    _refresh_and_copy(reactor[subgroup | tiermask]);
                }
            }
            else if (order == feed::rev)
            {
                static constexpr auto mask = hint{ (1 << events::block) - 1 };
                auto itermask = mask; // Skip root event block.
                auto subgroup = hint{};
                do
                {
                    itermask = (itermask << events::block) | mask;
                    subgroup = event & itermask;
                    _refresh_and_copy(reactor[subgroup | tiermask]);
                }
                while (subgroup != event);
            }
            else
            {
                _refresh_and_copy(reactor[event | tiermask]);
            }
            auto tail = qcopy.size();
            return std::pair{ head, tail };
        }
        // auth: Calling delegates. Returns the number of active ones.
        void _notify(si32 Tier, fmap& reactor, hint event, auto& param)
        {
            auto order = tier::order[Tier];
            auto [head, tail] = _select(Tier, reactor, event, order);
            if (head != tail)
            {
                queue.emplace_back(event, callstate::not_handled);
                auto iter = head;
                do
                {
                    if (auto fx_ptr = qcopy[iter].lock()) // qcopy can be reallocated.
                    {
                        auto& state = queue.back().second; // queue can be reallocated.
                        state = callstate::proceed;
                        fx_ptr->call(luafx, param);
                    }
                }
                while (queue.back().second/*callstate*/ != callstate::fullstop && ++iter != tail);
                qcopy.resize(head);
                handled = queue.back().second/*callstate*/ != callstate::not_handled;
                queue.pop_back();
            }
            else
            {
                handled = faux;
            }
        }
        void notify(si32 Tier, fmap& reactor, hint event, auto& param)
        {
            auto& target_reactor = Tier == tier::general ? general : reactor;
            _notify(Tier, target_reactor, event, param);
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
            _notify(tier::general, general, e2_timer_tick_id, now);
        }
        // auth: Delete object instance.
        template<class T>
        static void deleter(T* inst_ptr)
        {
            auto& indexer = inst_ptr->indexer;
            auto lock = indexer.sync(); // Sync with all dtors.
            // Remove metadata reference.
            for (auto& [classname, refs] : inst_ptr->base_classes)
            {
                auto& class_metadata = *(refs.class_metadata);
                class_metadata.objects.erase(refs.class_iterator);
                //log("Deleted: '%%' with id: %%", classname, inst_ptr->id);
            }
            // Remove object.
            auto id = inst_ptr->id;
            delete inst_ptr;
            indexer.objects.erase(id);
        }
        // auth: Add additional base class. Must run before anycast, e2::form::upon::started.
        void add_base_class(qiew classname, auto& inst)
        {
            if (inst.base_classes.find(classname) == inst.base_classes.end()) // Register only if it is not registered.
            {
                auto iter = classes.find(classname);
                if (iter == classes.end())
                {
                    iter = classes.emplace(classname, ptr::shared<vtm_class>()).first;
                }
                auto& class_metadata = iter->second;
                auto& class_objects = class_metadata->objects;
                auto class_iterator = class_objects.emplace(class_objects.end(), inst);
                // Update local references.
                auto iter2 = inst.base_classes.try_emplace(classname).first;
                auto& empty_refs = iter2->second;
                empty_refs.class_metadata = class_metadata;
                empty_refs.class_iterator = class_iterator;
            }
        }
        // auth: Create a new object of the specified subtype and return its sptr.
        template<class T, class ...Args>
        auto create(Args&&... args)
        {
            auto lock = sync();
            auto inst_ptr = sptr<T>(new T(std::forward<Args>(args)...), &deleter<T>); // Use new/delete to be able sync on destruction.
            auto& inst = *inst_ptr;
            //log("Create '%%' with id: %id%", T::classname, inst.id);
            objects.try_emplace(inst.id, inst);
            return inst_ptr;
        }
        // auth: Returns the next available id. The default gear has id = 0.
        auto new_id()
        {
            while (netxs::on_key(objects, next_id))
            {
                next_id++;
            }
            return next_id++;
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
        static auto rttidata = utf::unordered_map<text, metadata_t>{ 512 };
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
        static constexpr auto noid = std::numeric_limits<id_t>::max();

        auth& indexer; // bell: Global object indexer.
        fmap  reactor; // bell: Local subscriptions.
        subs  sensors; // bell: Event subscription tokens.
        const id_t id; // bell: Object id.

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

        template<class Event> auto submit(si32 Tier, Event)               { return submit_helper      <Event>(Tier, *this);                        }
        template<class Event> auto submit(si32 Tier, Event, si32)         { return submit_helper      <Event>(Tier, *this);                        }
        template<class Event> auto submit(si32 Tier, Event, hook& token)  { return submit_helper_token<Event>(Tier, *this, token);                 }
        template<class Event> auto submit(si32 Tier, Event, subs& tokens) { return submit_helper_token<Event>(Tier, *this, tokens.emplace_back()); }
        template<class Event, class Arg = Event::type>
        void submit(si32 Tier, Event, fx<Arg>&& handler)
        {
            auto lock = indexer.sync();
            sensors.push_back(indexer._subscribe(Tier, reactor, Event::id, std::move(handler)));
        }
        //todo unify
        void submit_generic(si32 Tier, si32 event_id, auto&& fx_or_script_ptr)
        {
            auto lock = indexer.sync();
            sensors.emplace_back(indexer._subscribe(Tier, reactor, event_id, std::move(fx_or_script_ptr)));
        }
        void submit_generic(si32 Tier, si32 event_id, subs& tokens, auto&& fx_or_script_ptr)
        {
            auto lock = indexer.sync();
            tokens.emplace_back(indexer._subscribe(Tier, reactor, event_id, std::move(fx_or_script_ptr)));
        }

        template<class Event, class Arg = Event::type>
        void submit(si32 Tier, Event, hook& token, fx<Arg>&& handler)
        {
            auto lock = indexer.sync();
            token = indexer._subscribe(Tier, reactor, Event::id, std::move(handler));
        }
        void dup_handler(si32 Tier, hint event_id, hook& token)
        {
            auto lock = indexer.sync();
            indexer._subscribe_copy(Tier, reactor, event_id, token);
        }
        void dup_handler(si32 Tier, hint event_id)
        {
            auto lock = indexer.sync();
            if (sensors.size())
            {
                indexer._subscribe_copy(Tier, reactor, event_id, sensors.back());
            }
        }
        auto has_handlers(si32 tier_id, hint event_id)
        {
            auto event_key = event_id | indexer.tier_mask(tier_id);
            auto& r = tier_id == tier::general ? indexer.general : reactor;
            auto iter = r.find(event_key);
            return iter != r.end() ? iter->second.size() : 0;
        }
        // bell: Erase all script handlers for the specified event.
        void erase_script_handlers(si32 tier_id, hint event_id)
        {
            auto event_key = event_id | indexer.tier_mask(tier_id);
            auto& r = tier_id == tier::general ? indexer.general : reactor;
            auto iter = r.find(event_key);
            if (iter != r.end())
            {
                auto& fx_list = iter->second;
                std::erase_if(fx_list, [&](auto& fx_wptr) // Clear handlers.
                {
                    if (auto fx_sptr = fx_wptr.lock())
                    {
                        if (fx_sptr->script_ptr)
                        {
                            fx_sptr->script_ptr.reset();
                            return true; // Erase if exists.
                        }
                        return faux;
                    }
                    else
                    {
                        return true;
                    }
                });
                if (fx_list.empty())
                {
                    r.erase(iter);
                }
                std::erase_if(sensors, [&](auto& fx_sptr) // Wipe sensors.
                {
                    if (!fx_sptr || (!fx_sptr->script_ptr && !fx_sptr->template get_inst<char>())) //todo template keyword required by gcc
                    {
                        return true; // Erase token if empty.
                    }
                    return faux;
                });
            }
        }
        // bell: .
        void _signal(si32 Tier, hint event, auto& param)
        {
            indexer.notify(Tier, reactor, event, param);
        }
        auto accomplished()
        {
            return indexer.handled;
        }
        // bell: Return original event id of the current event execution branch.
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

        bell(auth& indexer)
            : indexer{ indexer },
              id{ indexer.new_id() }
        { }
        virtual ~bell() = default;
    };
}
namespace netxs
{
    using netxs::events::vtm_class;
    using netxs::events::fxmap;
    using netxs::events::bell;
    using netxs::events::subs;
    using netxs::events::tier;
    using netxs::events::hook;
    using netxs::events::wook;
    //using netxs::events::sref;
    using netxs::events::script_ref;
}
namespace std
{
    template<>
    struct less<netxs::events::context_t>
    {
        using context_t = netxs::events::context_t;
        bool operator () (context_t const& l, context_t const& r) const
        {
            return std::lexicographical_compare(l.begin(), l.end(), r.begin(), r.end());
        }
    };
}