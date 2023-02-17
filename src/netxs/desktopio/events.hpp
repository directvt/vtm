// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#include "geometry.hpp"

#include <vector>
#include <mutex>
#include <map>
#include <list>
#include <functional>
#include <optional>
#include <thread>
#include <condition_variable>

namespace netxs::events
{
    enum class execution_order
    {
        forward, // Execute concrete event  first. Forward means from particular to general: 1. event_group::item, 2. event_group::any
        reverse, // Execute global   events first. Reverse means from general to particular: 1. event_group::any , 2. event_group::item
    };
    enum class tier
    {
        release, // events: Run forwrad handlers with fixed param. Preserve subscription order.
        preview, // events: Run reverse handlers with fixed a param intended to change. Preserve subscription order.
        general, // events: Run forwrad handlers for all objects. Preserve subscription order.
        request, // events: Run forwrad a handler that provides the current value of the param. To avoid being overridden, the handler should be the only one. Preserve subscription order.
        anycast, // events: Run reverse handlers along the entire visual tree. Preserve subscription order.
    };

    template<class V> struct _globals { static std::recursive_mutex              mutex; };
    template<class V>                          std::recursive_mutex _globals<V>::mutex;

    struct sync
    {
        std::lock_guard<std::recursive_mutex> lock;

        sync             (sync const&) = delete; // deleted copy constructor.
        sync& operator = (sync const&) = delete; // deleted copy assignment operator.

        sync() : lock(_globals<void>::mutex) { }
       ~sync() { }
    };
    static auto unique_lock()
    {
        return std::unique_lock{ _globals<void>::mutex };
    }
    struct try_sync
    {
        std::unique_lock<std::recursive_mutex> lock;

        operator bool () { return lock.owns_lock(); }

        try_sync             (try_sync const&) = delete; // deleted copy constructor.
        try_sync& operator = (try_sync const&) = delete; // deleted copy assignment operator.

        try_sync() : lock(_globals<void>::mutex, std::try_to_lock) { }
       ~try_sync() { }
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
    static constexpr inline hint level_mask(hint event, int level = 0)
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
    template<hint Event>             constexpr auto offset = level(Event) * block;                                  // events: Item/msg bit shift.
    template<hint Event>             constexpr auto parent =          Event & ((1 << (offset<Event> - block)) - 1); // events: Event group ID.
    template<hint Event>             constexpr auto number =               (Event >> (offset<Event> - block)) - 1;  // events: Item index inside the group by its ID.
    template<hint Group, auto Index> constexpr auto entity = Group | ((Index + 1) <<  offset<Group>);               // events: Event ID of the specified item inside the group.

    template<hint Group, auto... Index>
    constexpr auto _instantiate(std::index_sequence<Index...>)
    {
        return std::array<hint, sizeof...(Index)>{ entity<Group, Index>... };
    }
    template<hint Group, auto Count> constexpr auto subset = _instantiate<Group>(std::make_index_sequence<Count>{});

    struct handler
    {
        virtual ~handler() { }
    };
    struct hook : sptr<handler>
    {
        using sptr<handler>::sptr;
        auto& operator - (si32) { return *this; }
    };

    template<execution_order Order = execution_order::forward>
    struct reactor
    {
        template<class F>
        using hndl = std::function<void(F&&)>;
        using list = std::list<wptr<handler>>;
        using vect = std::vector<wptr<handler>>;

        template<class F>
        struct wrapper : handler
        {
            hndl<F> proc;
            wrapper(hndl<F> && proc)
                : proc{ proc }
            { }
        };
        enum branch
        {
            fullstop,
            nothandled,
            proceed,
        };

        std::map<hint, list> stock; // reactor: Handlers repository.
        std::vector<hint>    queue; // reactor: Event queue.
        vect                 qcopy; // reactor: Copy of the current pretenders to exec on current event.
        branch               alive; // reactor: Current exec branch interruptor.

        void cleanup(ui64& ref_count, ui64& del_count)
        {
            auto lref = ui64{};
            auto ldel = ui64{};
            for (auto& [event, subs] : stock)
            {
                auto refs = subs.size();
                subs.remove_if([](auto&& a) { return a.expired(); });
                auto size = subs.size();
                lref += size;
                ldel += refs - size;
            }
            ref_count += lref;
            del_count += ldel;
        }
        void merge(reactor const& r)
        {
            for (auto& [event, src_subs] : r.stock)
            {
                auto& dst_subs = stock[event];
                dst_subs.insert( dst_subs.end(), src_subs.begin(), src_subs.end() );
            }
        }
        template<class F>
        hook subscribe(hint event, hndl<F> proc)
        {
            auto proc_ptr = std::make_shared<wrapper<F>>(std::move(proc));

            auto lock = sync{};
            stock[event].push_back(proc_ptr);

            return proc_ptr;
        }
        inline void _refreshandcopy(list& target)
        {
            target.remove_if([&](auto&& a) { return a.expired() ? true : (qcopy.emplace_back(a), faux); });
        }
        // reactor: Calling delegates. Returns the number of active ones.
        template<class F>
        auto notify(hint event, F&& param)
        {
            auto lock = sync{};

            alive = branch::proceed;
            queue.push_back(event);
            auto head = qcopy.size();

            if constexpr (Order == execution_order::forward)
            {
                auto itermask = events::level_mask(event);
                auto subgroup = event;
                _refreshandcopy(stock[subgroup]);
                while (itermask > 1 << events::block) // Skip root event block.
                {
                    subgroup = event & itermask;
                    itermask >>= events::block;
                    _refreshandcopy(stock[subgroup]);
                }
            }
            else
            {
                static constexpr auto mask = hint{ (1 << events::block) - 1 };
                auto itermask = mask; // Skip root event block.
                auto subgroup = hint{};
                do
                {
                    itermask = itermask << events::block | mask;
                    subgroup = event & itermask;
                    _refreshandcopy(stock[subgroup]);
                }
                while (subgroup != event);
            }

            auto tail = qcopy.size();
            auto size = tail - head;
            if (size)
            {
                auto iter = head;
                do
                {
                    if (auto proc_ptr = qcopy[iter].lock()) // qcopy can be reallocated.
                    {
                        if (auto compatible = static_cast<wrapper<F>*>(proc_ptr.get()))
                        {
                            compatible->proc(std::forward<F>(param));
                        }
                    }
                }
                while (alive == branch::proceed && ++iter != tail);
                qcopy.resize(head);
            }

            queue.pop_back();
            return alive != branch::nothandled && size;
        }
        // reactor: Interrupt current invocation branch.
        void stop()
        {
            alive = branch::fullstop;
        }
        // reactor: Skip current invocation branch.
        void skip()
        {
            alive = branch::nothandled;
        }
    };

    template<class T>
    struct indexer
    {
        const  id_t                       id;
        static id_t                    newid;
        static wptr<T>                 empty;
        static std::map<id_t, wptr<T>> store;

        // indexer: Return shared_ptr of the object by its id.
        template<class TT = T>
        static auto getref(id_t id)
        {
            auto lock = sync{};
            if (auto item_ptr = netxs::get_or(store, id, empty).lock())
            if (auto real_ptr = std::dynamic_pointer_cast<TT>(item_ptr))
            {
                return real_ptr;
            }
            return sptr<TT>{};
        }
        // indexer: Create a new object of the specified subtype and return its shared_ptr.
        template<class TT, class ...Args>
        static auto create(Args&&... args) -> sptr<TT>
        {
            // Enables the use of a protected ctor by std::make_shared<TT>.
            struct make_shared_enabler : public TT
            {
                make_shared_enabler(Args&&... args)
                    : TT{ std::forward<Args>(args)... }
                { }
            };

            auto lock = sync{};
            auto inst = std::make_shared<make_shared_enabler>(std::forward<Args>(args)...);

            store[inst->id] = inst;
            //sptr<T>  item = inst;
            //inst->T::signal_direct(e2_base::release, e2::form::upon::created, item);
            return inst;
        }

    private:
        static inline auto _counter()
        {
            auto lock = sync{};
            while (netxs::on_key(store, ++newid))
            { }
            return newid;
        }

    protected:
        indexer(indexer const&) = delete; // id is flushed out when a copy of the object is deleted.
                                          // Thus, the original object instance becomes invalid.
        indexer()
            : id{ _counter() }
        { }
       ~indexer()
        {
           auto lock = sync{};
           store.erase(id);
        }
    };

    template<class T> id_t                    indexer<T>::newid{};
    template<class T> wptr<T>                 indexer<T>::empty{};
    template<class T> std::map<id_t, wptr<T>> indexer<T>::store{};

    class subs
    {
        std::vector<hook> tokens;

    public:
        auto& operator - (si32)    { return *this;                                                           }
        operator bool () const     { return tokens.size();                                                   }
        void  admit(hook&& t)      {        tokens.push_back(std::forward<hook>(t));                         }
        hook& extra()              { return tokens.emplace_back();                                           }
        auto  count() const        { return tokens.size();                                                   }
        void  clear()              {        tokens.clear();                                                  }
        void  reset()              {        tokens.clear();                                                  }
        void  merge(subs const& m) {        tokens.insert( tokens.end(), m.tokens.begin(), m.tokens.end() ); }
    };

    template<class Parent_t, class Object_t, auto Event_id>
    struct type_clue
    {
        using type = Object_t;
        using base = Parent_t;
        static constexpr auto id = Event_id;
        template<class ...Args> constexpr type_clue(Args&&...) { }
        template<class ...Args> static constexpr auto param(Args&&... args) { return type{ std::forward<Args>(args)... }; }
                                static constexpr auto param(type&&    arg ) { return std::move(arg);                      }
        template<auto N>        static constexpr auto group()               { return events::subset<id, N>;               }
                                static constexpr auto index()               { return events::number<id>;                  }
    };

    #define ARGsEVAL(...) __VA_ARGS__
    #define ARG_EVAL(...) ARGsEVAL(__VA_ARGS__)
    #define GET_LAST(a, b, c, d, e, last, ...) last

    #define RISEUP(        level, event,   ...              ) base::template riseup<level>(event, __VA_ARGS__)
    #define SIGNAL(        level, event, param              ) bell::template signal<level>(decltype( event )::id, static_cast<typename decltype( event )::type &&>(param))
    #define LISTEN_S_BYREF(level, event, param              ) bell::template submit<level>( event )           = [&]                  (typename decltype( event )::type&& param)
    #define LISTEN_T_BYREF(level, event, param, token       ) bell::template submit<level>( event, token -0 ) = [&]                  (typename decltype( event )::type&& param)
    #define LISTEN_T_BYVAL(level, event, param, token, byval) bell::template submit<level>( event, token -0 ) = [&, ARG_EVAL byval ] (typename decltype( event )::type&& param) mutable
    #define LISTEN_X_BYREF(...) ARG_EVAL(GET_LAST(__VA_ARGS__, LISTEN_T_BYVAL, LISTEN_T_BYREF, LISTEN_S_BYREF))

    #if defined(_WIN32)
        #define LISTEN(...) ARG_EVAL(LISTEN_X_BYREF(__VA_ARGS__))ARG_EVAL((__VA_ARGS__))
    #else
        #define LISTEN(...) LISTEN_X_BYREF(__VA_ARGS__)(__VA_ARGS__)
    #endif

    //todo deprecated?
    //#define LISTEN_AND_RUN_T(level, event, token, param, arg) bell::template submit2<level,decltype( event )>( arg, token ) = [&] (typename decltype( event )::type && param)
    //#define LISTEN_AND_RUN(  level, event,        param, arg) bell::template submit2<level,decltype( event )>( arg        ) = [&] (typename decltype( event )::type && param)
    //#define SIGNAL_GLOBAL(        event, param              ) bell::template signal_global(decltype( event )::id, static_cast<typename decltype( event )::type &&>(param))
    //#define LISTEN_GLOBAL(        event, param, token       ) bell::template submit_global( event, token -0 ) = [&]                  (typename decltype( event )::type&& param)

    #define EVENTPACK( name, base ) using _group_type = name; \
                                    static constexpr auto _counter_base = __COUNTER__; \
                                    public: static constexpr auto any = netxs::events::type_clue<_group_type, decltype(base)::type, decltype(base)::id>
    #define  EVENT_XS( name, type ) }; static constexpr auto name = netxs::events::type_clue<_group_type, type, decltype(any)::id | ((__COUNTER__ - _counter_base) << netxs::events::offset<decltype(any)::id>)>{ 777
    #define  GROUP_XS( name, type ) EVENT_XS( _##name, type )
    #define SUBSET_XS( name )       }; class name { EVENTPACK( name, _##name )
    #define  INDEX_XS(  ... )       }; template<auto N> static constexpr \
                                    auto _ = std::get<N>( std::tuple{ __VA_ARGS__ } ); \
                                    private: static constexpr auto _dummy = { 777

    struct bell;
    using ftor = std::function<bool(sptr<bell>)>;

    struct ref_count_t
    {
        ui64 obj_count{};
        ui64 ref_count{};
        ui64 del_count{};
    };

    //todo unify seeding
    namespace userland
    {
        struct root
        {
            static constexpr auto root_event = type_clue<root, si32, 0>{};
            EVENTPACK( root, root_event )
            {
                EVENT_XS( dtor   , const id_t ),
                EVENT_XS( cascade, ftor ),
                EVENT_XS( base   , root ),
                EVENT_XS( hids   , root ),
                EVENT_XS( custom , root ),
                EVENT_XS( cleanup, ref_count_t ), // Garbage collection.
            };
        };
    }

    // events: Event x-mitter.
    struct bell : public indexer<bell>
    {
        static constexpr auto noid = std::numeric_limits<id_t>::max();
        subs tracker;

    private:
        using fwd_reactor = reactor<execution_order::forward>;
        using rev_reactor = reactor<execution_order::reverse>;

        template<class V> struct _globals { static fwd_reactor general; };

        fwd_reactor& general{ _globals<void>::general };
        fwd_reactor  release;
        fwd_reactor  request;
        rev_reactor  preview;
        rev_reactor  anycast;

        //todo deprecated?
        template<tier Tier, class Event>
        struct submit_helper2
        {
            using type = typename Event::type;
            bell& owner;
            type& p;
            submit_helper2(bell& owner, type& p)
                : owner{ owner },
                  p{p}
            { }
            template<class F>
            void operator = (F h)
            {
                owner.submit<Tier>(Event{}, h);
                h(static_cast<type&&>(p));
            }
        };
        //todo deprecated?
        template<tier Tier, class Event>
        struct submit_helper2_token
        {
            using type = typename Event::type;
            bell& owner;
            type& p;
            hook& token;
            submit_helper2_token(bell& owner, type& p, hook& token)
                : owner{ owner },
                  p{p},
                  token{ token }
            { }
            template<class F>
            void operator = (F h)
            {
                owner.submit<Tier>(Event{}, token, h);
                h(static_cast<type&&>(p));
            }
        };
        template<tier Tier, class Event>
        struct submit_helper
        {
            bell& owner;
            submit_helper(bell& owner)
                : owner{ owner }
            { }
            template<class F>
            void operator = (F h)
            {
                owner.submit<Tier>(Event{}, h);
            }
        };
        template<tier Tier, class Event>
        struct submit_helper_token
        {
            bell& owner;
            hook& token;
            submit_helper_token(bell& owner, hook& token)
                : owner{ owner },
                  token{ token }
            { }
            template<class F>
            void operator = (F h)
            {
                owner.submit<Tier>(Event{}, token, h);
            }
        };
        template<class Event>
        struct submit_helper_token_global
        {
            hook& token;
            submit_helper_token_global(hook& token)
                : token{ token }
            { }
            template<class F>
            void operator = (F h)
            {
                token = _globals<void>::general.subscribe(Event::id, std::function<void(typename Event::type &&)>{ h });
            }
        };

    public:
        //todo deprecated?
        template<tier Tier, class Event> auto submit2(typename Event::type & p)               { return submit_helper2      <Tier, Event>(*this, p);                 }
        template<tier Tier, class Event> auto submit2(typename Event::type & p, subs& tokens) { return submit_helper2_token<Tier, Event>(*this, p, tokens.extra()); }

        template<tier Tier, class Event> auto submit(Event)               { return submit_helper      <Tier, Event>(*this);                 }
        template<tier Tier, class Event> auto submit(Event, si32)         { return submit_helper      <Tier, Event>(*this);                 }
        template<tier Tier, class Event> auto submit(Event, hook& token)  { return submit_helper_token<Tier, Event>(*this, token);          }
        template<tier Tier, class Event> auto submit(Event, subs& tokens) { return submit_helper_token<Tier, Event>(*this, tokens.extra()); }
        template<tier Tier, class Event>
        void submit(Event, std::function<void(typename Event::type &&)> handler)
        {
                 if constexpr (Tier == tier::preview) tracker.admit(preview.subscribe(Event::id, handler));
            else if constexpr (Tier == tier::general) tracker.admit(general.subscribe(Event::id, handler));
            else if constexpr (Tier == tier::request) tracker.admit(request.subscribe(Event::id, handler));
            else if constexpr (Tier == tier::release) tracker.admit(release.subscribe(Event::id, handler));
            else                                      tracker.admit(anycast.subscribe(Event::id, handler));
        }
        template<tier Tier, class Event>
        void submit(Event, hook& token, std::function<void(typename Event::type &&)> handler)
        {
                 if constexpr (Tier == tier::preview) token = preview.subscribe(Event::id, handler);
            else if constexpr (Tier == tier::general) token = general.subscribe(Event::id, handler);
            else if constexpr (Tier == tier::request) token = request.subscribe(Event::id, handler);
            else if constexpr (Tier == tier::release) token = release.subscribe(Event::id, handler);
            else                                      token = anycast.subscribe(Event::id, handler);
        }
        template<tier Tier, class F>
        auto signal(hint event, F&& data)
        {
                 if constexpr (Tier == tier::preview) return preview.notify(event, std::forward<F>(data));
            else if constexpr (Tier == tier::general) return general.notify(event, std::forward<F>(data));
            else if constexpr (Tier == tier::request) return request.notify(event, std::forward<F>(data));
            else if constexpr (Tier == tier::release) return release.notify(event, std::forward<F>(data));
            else            /* Tier == tier::anycast */
            {
                auto root = gettop();
                ftor proc = [&](auto boss_ptr) -> bool
                {
                    boss_ptr->anycast.notify(event, std::forward<F>(data));
                    return true;
                };
                return root->release.notify(userland::root::cascade.id, proc);
            }
        }
        //todo deprecated
        //template<class F>     static auto signal_global(hint event, F&& data) { return _globals<void>::general.notify(event, std::forward<F>(data)); }
        //template<class Event> static auto submit_global(Event, hook& token)   { return submit_helper_token_global<Event>(token); }
        //template<class Event> static auto submit_global(Event, subs& tokens)  { return submit_helper_token_global<Event>(tokens.extra()); }
        // bell: Return initial event of the current event execution branch.
        template<tier Tier>
        auto protos()
        {
                 if constexpr (Tier == tier::preview) return preview.queue.empty() ? hint{} : preview.queue.back();
            else if constexpr (Tier == tier::general) return general.queue.empty() ? hint{} : general.queue.back();
            else if constexpr (Tier == tier::request) return request.queue.empty() ? hint{} : request.queue.back();
            else if constexpr (Tier == tier::release) return release.queue.empty() ? hint{} : release.queue.back();
            else                                      return anycast.queue.empty() ? hint{} : anycast.queue.back();
        }
        template<tier Tier, class Event> auto protos(Event) { return bell::protos<Tier>() == Event::id; }
        template<tier Tier>
        auto& router()
        {
                 if constexpr (Tier == tier::preview) return preview;
            else if constexpr (Tier == tier::general) return general;
            else if constexpr (Tier == tier::request) return request;
            else if constexpr (Tier == tier::release) return release;
            else                                      return anycast;
        }
        template<tier Tier>
        void expire()
        {
                 if constexpr (Tier == tier::preview) return preview.stop();
            else if constexpr (Tier == tier::general) return general.stop();
            else if constexpr (Tier == tier::request) return request.stop();
            else if constexpr (Tier == tier::release) return release.stop();
            else                                      return anycast.stop();
        }
        // bell: Sync with UI thread.
        template<class P>
        void trysync(bool& active, P proc)
        {
            while (active)
            {
                if (auto guard = netxs::events::try_sync{})
                {
                    proc();
                    break;
                }
                std::this_thread::yield();
            }            
        }

        bell()
        {
            LISTEN(tier::general, userland::root::cleanup, counter)
            {
                counter.obj_count++;
                preview.cleanup(counter.ref_count, counter.del_count);
                request.cleanup(counter.ref_count, counter.del_count);
                release.cleanup(counter.ref_count, counter.del_count);
                anycast.cleanup(counter.ref_count, counter.del_count);
            };
        }
       ~bell()
        {
            SIGNAL(tier::release, userland::root::dtor, id);
        }
        virtual sptr<bell> gettop() { return sptr<bell>(this, noop{}); } // bell: Recursively find the root of the visual tree.
    };

    namespace
    {
        template<class T>
        auto& _agent()
        {
            static auto agent = generics::jobs<netxs::wptr<bell>>{};
            return agent;
        }
    }
    template<class T>
    void enqueue(netxs::wptr<bell> object_wptr, T&& proc)
    {
        auto& agent = _agent<void>();
        agent.add(object_wptr, [proc](auto& object_wptr) mutable
        {
            auto lock = events::sync{};
            if (auto object_ptr = object_wptr.lock())
            {
                proc(*object_ptr);
            }
        });
    }

    template<class T> bell::fwd_reactor bell::_globals<T>::general;
}
namespace netxs
{
    using netxs::events::bell;
    using netxs::events::subs;
    using netxs::events::tier;
    using netxs::events::hook;
}