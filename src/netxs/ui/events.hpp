// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_EVENTS_HPP
#define NETXS_EVENTS_HPP

#include "../abstract/ptr.hpp"
#include "../abstract/hash.hpp"
#include "../abstract/duplet.hpp"

#include <vector>
#include <mutex>
#include <map>
#include <list>
#include <functional>
#include <optional>
#include <thread>

namespace netxs::events
{
    using type = unsigned int;
    using id_t = uint32_t;

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
    constexpr static inline auto level(type event)
    {
        if (event == 0) return 0;
        auto level = 1;
        while (event >>= block) { ++level; }
        return level;
    }
    // events: Return event level mask by its ID. Find the log base 2**block.
    constexpr static inline type level_mask(type event)
    {
        auto level = 0;
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
        //return std::numeric_limits<type>::max();
    }
    template<type event>             constexpr auto offset = level(event) * block;                                  // events: Item/msg bit shift.
    template<type event>             constexpr auto parent =          event & ((1 << (offset<event> - block)) - 1); // events: Event group ID.
    template<type event>             constexpr auto number =               (event >> (offset<event> - block)) - 1;  // events: Item index inside the group by its ID.
    template<type group, auto index> constexpr auto entity = group | ((index + 1) <<  offset<group>);               // events: Event ID of the specified item inside the group.

    template<type group, auto... index>
    constexpr auto _instantiate(std::index_sequence<index...>)
    {
        return std::array<type, sizeof...(index)>{ entity<group, index>... };
    }
    template<type group, auto count> constexpr auto subset = _instantiate<group>(std::make_index_sequence<count>{});

    struct handler
    {
        virtual ~handler() { }
    };
    using hook = sptr<handler>;

    template<execution_order ORDER = execution_order::forward>
    struct reactor
    {
        template <class F>
        using hndl = std::function<void(F&&)>;
        using list = std::list<wptr<handler>>;
        using vect = std::vector<wptr<handler>>;

        template <class F>
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

        std::map<type, list> stock; // reactor: Handlers repository.
        std::vector<type>    queue; // reactor: Event queue.
        vect                 qcopy; // reactor: Copy of the current pretenders to exec on current event.
        branch               alive; // reactor: Current exec branch interruptor.

        void cleanup(ui64& ref_count, ui64& del_count)
        {
            ui64 lref{};
            ui64 ldel{};
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
        hook subscribe(type event, hndl<F> proc)
        {
            auto proc_ptr = std::make_shared<wrapper<F>>(std::move(proc));

            sync lock;
            stock[event].push_back(proc_ptr);

            return proc_ptr;
        }
        inline void _refreshandcopy(list& target)
        {
            target.remove_if([&](auto&& a) { return a.expired() ? true : (qcopy.emplace_back(a), faux); });
        }
        // reactor: Calling delegates. Returns the number of active ones.
        template<class F>
        auto notify(type event, F&& param)
        {
            sync lock;

            alive = branch::proceed;
            queue.push_back(event);
            auto head = qcopy.size();

            if constexpr (ORDER == execution_order::forward)
            {
                type itermask = events::level_mask(event);
                type subgroup = event;
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
                static constexpr type mask = (1 << events::block) - 1;
                type itermask = mask; // Skip root event block.
                type subgroup;
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
        static auto getref(id_t id)
        {
            sync lock;
            return netxs::get_or(store, id, empty).lock();
        }
        // indexer: Create a new object of the specified subtype and return its shared_ptr.
        template<class TT, class ...Args>
        static auto create(Args&&... args)
        {
            // Enables the use of a protected ctor by std::make_shared<TT>.
            struct make_shared_enabler : public TT
            {
                make_shared_enabler(Args&&... args)
                    : TT{ std::forward<Args>(args)... }
                { }
            };

            sync lock;
            sptr<TT> inst = std::make_shared<make_shared_enabler>(std::forward<Args>(args)...);

            store[inst->id] = inst;
            //sptr<T>  item = inst;
            //inst->T::signal_direct(e2_base::release, e2::form::upon::created, item);
            return inst;
        }

    private:
        static inline auto _counter()
        {
            sync lock;
            while (netxs::on_key(store, ++newid)) { }
            return newid;
        }

    protected:
        indexer(indexer const&) = delete; // id is flushed out when a copy of the object is deleted.
                                          // Thus, the original object instance becomes invalid.
        indexer() : id { _counter() } { }
       ~indexer() { sync lock; store.erase(id); }
    };

    template<class T> id_t                    indexer<T>::newid{};
    template<class T> wptr<T>                 indexer<T>::empty{};
    template<class T> std::map<id_t, wptr<T>> indexer<T>::store{};

    class subs
    {
        std::vector<hook> tokens;

    public:
        void  admit(hook&& t)      {        tokens.push_back(std::forward<hook>(t));                         }
        hook& extra()              { return tokens.emplace_back();                                           }
        auto  count() const        { return tokens.size();                                                   }
        void  clear()              {        tokens.clear();                                                  }
        void  merge(subs const& m) {        tokens.insert( tokens.end(), m.tokens.begin(), m.tokens.end() ); }
    };

    template<class _parent_type, class _object_type, auto _event_id>
    struct type_clue
    {
        using type = _object_type;
        using base = _parent_type;
        static constexpr auto id = _event_id;
        template<class ...Args> constexpr type_clue(Args&&...) { }
        template<auto N> static constexpr auto group() { return events::subset<id, N>; }
                         static constexpr auto index() { return events::number<id>;    }
    };

    #define SUBMIT(        level, event,        param) bell::template submit<level, decltype( event )>()        = [&] (typename decltype( event )::type &&  param)
    #define SUBMIT_BYVAL(  level, event,        param) bell::template submit<level, decltype( event )>()        = [=] (typename decltype( event )::type &&  param) mutable
    #define SUBMIT_T(      level, event, token, param) bell::template submit<level, decltype( event )>( token ) = [&] (typename decltype( event )::type &&  param)
    #define SUBMIT_T_BYVAL(level, event, token, param) bell::template submit<level, decltype( event )>( token ) = [=] (typename decltype( event )::type &&  param) mutable
    #define SUBMIT_TV(     level, event, token, proxy) bell::template submit<level, decltype( event )>( token, proxy )
    #define SUBMIT_V(      level, event,        proxy) bell::template submit<level, decltype( event )>(        proxy )
    #define SIGNAL(        level, event,        param) bell::template signal<level>(decltype( event )::id, static_cast<typename decltype( event )::type &&>(param))
    #define SIGNAL_GLOBAL(        event,        param) bell::template signal_global(decltype( event )::id, static_cast<typename decltype( event )::type &&>(param))
    #define SUBMIT_GLOBAL(        event, token, param) bell::template submit_global<decltype( event )>( token ) = [&] (typename decltype( event )::type &&  param)

    //todo deprecated?
    #define SUBMIT_AND_RUN_T(level, event, token, param, arg) bell::template submit2<level,decltype( event )>( arg, token ) = [&] (typename decltype( event )::type && param)
    #define SUBMIT_AND_RUN(  level, event,        param, arg) bell::template submit2<level,decltype( event )>( arg        ) = [&] (typename decltype( event )::type && param)

    #define EVENTPACK( name, base ) using _group_type = name; \
                                    static constexpr auto _counter_base = __COUNTER__; \
                                    public: static constexpr auto any = type_clue<_group_type, decltype(base)::type, decltype(base)::id>
    #define  EVENT_XS( name, type ) }; static constexpr auto name = type_clue<_group_type, type, decltype(any)::id | ((__COUNTER__ - _counter_base) << netxs::events::offset<decltype(any)::id>)>{ 777
    #define  GROUP_XS( name, type ) EVENT_XS( _##name, type )
    #define SUBSET_XS( name )       }; class name { EVENTPACK( name, _##name )
    #define  INDEX_XS(  ... )       }; template<auto N> static constexpr \
                                    auto _ = std::get<N>( std::tuple{ __VA_ARGS__ } ); \
                                    private: static constexpr auto _dummy = { 777

    class bell;
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
            static constexpr auto root_event = type_clue<root, void, 0>{};
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
    class bell : public indexer<bell>
    {
    public:
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
        template<tier TIER, class EVENT>
        struct submit_helper2
        {
            using type = typename EVENT::type;
            bell& owner;
            type& p;
            submit_helper2(bell& owner, type& p)
                : owner{ owner },
                  p{p}
            { }
            template<class F>
            void operator = (F h)
            {
                owner.submit<TIER, EVENT>(h);
                h(static_cast<type&&>(p));
            }
        };
        //todo deprecated?
        template<tier TIER, class EVENT>
        struct submit_helper2_token
        {
            using type = typename EVENT::type;
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
                owner.submit<TIER, EVENT>(token, h);
                h(static_cast<type&&>(p));
            }
        };
        template<tier TIER, class EVENT>
        struct submit_helper
        {
            bell& owner;
            submit_helper(bell& owner)
                : owner{ owner }
            { }
            template<class F>
            void operator = (F h)
            {
                owner.submit<TIER, EVENT>(h);
            }
        };
        template<tier TIER, class EVENT>
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
                owner.submit<TIER, EVENT>(token, h);
            }
        };
        template<class EVENT>
        struct submit_helper_token_global
        {
            hook& token;
            submit_helper_token_global(hook& token)
                : token{ token }
            { }
            template<class F>
            void operator = (F h)
            {
                token = _globals<void>::general.subscribe(EVENT::id, std::function<void(typename EVENT::type &&)>{ h });
            }
        };

    public:
        //todo deprecated?
        template<tier TIER, class EVENT> auto submit2(typename EVENT::type & p)               { return submit_helper2      <TIER, EVENT>(*this, p);                 }
        template<tier TIER, class EVENT> auto submit2(typename EVENT::type & p, subs& tokens) { return submit_helper2_token<TIER, EVENT>(*this, p, tokens.extra()); }

        template<tier TIER, class EVENT> auto submit()             { return submit_helper      <TIER, EVENT>(*this);                 }
        template<tier TIER, class EVENT> auto submit(hook& token)  { return submit_helper_token<TIER, EVENT>(*this, token);          }
        template<tier TIER, class EVENT> auto submit(subs& tokens) { return submit_helper_token<TIER, EVENT>(*this, tokens.extra()); }
        template<tier TIER, class EVENT>
        void submit(std::function<void(typename EVENT::type &&)> handler)
        {
                 if constexpr (TIER == tier::preview) tracker.admit(preview.subscribe(EVENT::id, handler));
            else if constexpr (TIER == tier::general) tracker.admit(general.subscribe(EVENT::id, handler));
            else if constexpr (TIER == tier::request) tracker.admit(request.subscribe(EVENT::id, handler));
            else if constexpr (TIER == tier::release) tracker.admit(release.subscribe(EVENT::id, handler));
            else                                      tracker.admit(anycast.subscribe(EVENT::id, handler));
        }
        template<tier TIER, class EVENT>
        void submit(hook& token, std::function<void(typename EVENT::type &&)> handler)
        {
                 if constexpr (TIER == tier::preview) token = preview.subscribe(EVENT::id, handler);
            else if constexpr (TIER == tier::general) token = general.subscribe(EVENT::id, handler);
            else if constexpr (TIER == tier::request) token = request.subscribe(EVENT::id, handler);
            else if constexpr (TIER == tier::release) token = release.subscribe(EVENT::id, handler);
            else                                      token = anycast.subscribe(EVENT::id, handler);
        }
        template<tier TIER, class F>
        auto signal(type event, F&& data)
        {
                 if constexpr (TIER == tier::preview) return preview.notify(event, std::forward<F>(data));
            else if constexpr (TIER == tier::general) return general.notify(event, std::forward<F>(data));
            else if constexpr (TIER == tier::request) return request.notify(event, std::forward<F>(data));
            else if constexpr (TIER == tier::release) return release.notify(event, std::forward<F>(data));
            else            /* TIER == tier::anycast */
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
        template<class EVENT> static auto submit_global(hook& token)           { return submit_helper_token_global<EVENT>(token); }
        template<class F>     static auto signal_global(type  event, F&& data) { return _globals<void>::general.notify(event, std::forward<F>(data)); }
        // bell: Return initial event of the current event execution branch.
        template<tier TIER>
        auto protos()
        {
                 if constexpr (TIER == tier::preview) return preview.queue.empty() ? type{} : preview.queue.back();
            else if constexpr (TIER == tier::general) return general.queue.empty() ? type{} : general.queue.back();
            else if constexpr (TIER == tier::request) return request.queue.empty() ? type{} : request.queue.back();
            else if constexpr (TIER == tier::release) return release.queue.empty() ? type{} : release.queue.back();
            else                                      return anycast.queue.empty() ? type{} : anycast.queue.back();
        }
        template<tier TIER, class EVENT> auto protos(EVENT) { return bell::protos<TIER>() == EVENT::id; }
        template<tier TIER>
        auto& router()
        {
                 if constexpr (TIER == tier::preview) return preview;
            else if constexpr (TIER == tier::general) return general;
            else if constexpr (TIER == tier::request) return request;
            else if constexpr (TIER == tier::release) return release;
            else                                      return anycast;
        }
        template<tier TIER>
        void expire()
        {
                 if constexpr (TIER == tier::preview) return preview.stop();
            else if constexpr (TIER == tier::general) return general.stop();
            else if constexpr (TIER == tier::request) return request.stop();
            else if constexpr (TIER == tier::release) return release.stop();
            else                                      return anycast.stop();
        }

        bell()
        {
            SUBMIT_GLOBAL(userland::root::cleanup, tracker.extra(), counter)
            {
                counter.obj_count++;
                preview.cleanup(counter.ref_count, counter.del_count);
                request.cleanup(counter.ref_count, counter.del_count);
                release.cleanup(counter.ref_count, counter.del_count);
                anycast.cleanup(counter.ref_count, counter.del_count);
            };
        }
       ~bell() { SIGNAL(tier::release, userland::root::dtor, id); }

        virtual void  global(twod& coor) { } // bell: Recursively calculate global coordinate.
        virtual sptr<bell> gettop() { return sptr<bell>(this, noop{}); } // bell: Recursively find the root of the visual tree.
    };

    template<class T> bell::fwd_reactor bell::_globals<T>::general;
}

#endif // NETXS_EVENTS_HPP