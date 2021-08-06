// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_EVENTS_HPP
#define NETXS_EVENTS_HPP

// Description: Compile time typesafe hierarchical delegates.

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

    enum class tier
    {
        // Forward means from particular to general: 1. event_group::item, 2. event_group::any
        // Reverse means from general to particular: 1. event_group::any , 2. event_group::item
        release, // events: Run forwrad handlers with fixed param. Preserve subscription order.
        preview, // events: Run reverse handlers with fixed a param intended to change. Preserve subscription order.
        general, // events: Run forwrad handlers for all objects. Preserve subscription order.
        request, // events: Run forwrad a handler that provides the current value of the param. To avoid overriding, the handler should be the only one. Preserve subscription order.
    };

    template<class V>
    struct _globals
    {
        static std::recursive_mutex mutex;
    };
    template<class V>
    std::recursive_mutex _globals<V>::mutex;

    struct sync
    {
        std::lock_guard<std::recursive_mutex> lock;

        sync            (sync const&) = delete; // deleted copy constructor.
        sync& operator= (sync const&) = delete; // deleted copy assignment operator.

        sync() : lock(_globals<void>::mutex) { }
       ~sync() { }
    };
    struct try_sync
    {
        std::unique_lock<std::recursive_mutex> lock;

        try_sync            (try_sync const&) = delete; // deleted copy constructor.
        try_sync& operator= (try_sync const&) = delete; // deleted copy assignment operator.

        operator bool() { return lock.owns_lock(); }

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

    static constexpr auto width = 4;
    static constexpr type _mask = (1 << width) - 1;
    // events: Return item/msg level by its ID.
    constexpr static auto level(type msg)
    {
        if (msg == 0) return 0;
        iota level = 1;
        while ((msg = msg >> width))
        {
            level++;
        }
        return level;
    }
    // events: Return item/msg bit shift.
    constexpr static auto level_width(type msg)
    {
        return level(msg) * width;
    }
    // events: Return item/msg global level mask by its ID.
    constexpr static type level_mask(type msg)
    {
        unsigned int level = 0;
        while ((msg = msg >> width))
        {
            level += width;
        }
        return (1 << level) - 1;
    }
    // events: Increament level offset by width and return item's subgroup ID fof the specified level offset.
    constexpr static auto subgroup(type msg, type& itermask)
    {
        itermask = (itermask << width) + _mask;
        return msg & itermask;
    }
    constexpr static auto subgroup_fwd(type msg, type& itermask)
    {
        auto result = msg & itermask;
        itermask = (itermask >> width);
        return result;
    }
    // events: Return event's group ID.
    constexpr static auto parent(type msg)
    {
        return msg & ((1 << ((level(msg) - 1) * width)) - 1);
    }
    // events: Return the event ID of the specified item inside the group.
    constexpr static auto message(type base, type item)
    {
        return base | ((item + 1) << level_width(base));
    }
    // events: Return item index inside the group by its ID.
    constexpr static auto item(type msg)
    {
        return (msg >> ((level(msg) - 1) * width)) - 1;
    }
    template<std::size_t N, std::size_t... I>
    constexpr static auto _instantiate(type base, std::index_sequence<I...>)
    {
        return std::array<unsigned int, N>{ message(base, I)... };
    }
    template<std::size_t N>
    constexpr static auto group(type base)
    {
        return _instantiate<N>(base, std::make_index_sequence<N>{});
    }

    using hint = events::type;
    using id_t = uint32_t;
    struct reactor
    {
        struct handler
        {
            virtual ~handler() { }
        };

        template <typename F>
        using hndl = std::function<void(F&&)>;
        using hook =             sptr<handler>;
        using list = std::list  <wptr<handler>>;
        using vect = std::vector<wptr<handler>>;

        template <typename F>
        struct wrapper : handler
        {
            hndl<F> proc;

            wrapper(hndl<F> && f)
                : proc{ f }
            { }
        };

        enum exec
        {
            forward, // Execute concrete event first.
            reverse, // Execute global events first.
        };

        std::map<hint, list> stock; // reactor: handlers repository.
        std::vector<hint>    queue; // reactor: event queue.
        vect                 qcopy; // reactor: copy of the current pretenders to exec on current event.
        bool                 alive; // reactor: current exec branch interruptor.
        exec                 order; // reactor: Execution oreder.

        void merge(reactor const& r)
        {
            for (auto& [event, src_subs] : r.stock)
            {
                auto& dst_subs = stock[event];
                for (auto& s : src_subs)
                {
                    dst_subs.push_back(s);
                }
            }
        }

        reactor(exec order)
            : alive{ true },
              order{ order}
        { }

        template<class F>
        hook subscribe(hint e, hndl<F> proc)
        {
            auto proc_ptr = std::make_shared<wrapper<F>>(std::move(proc));

            events::sync lock;

            stock[e].push_back(proc_ptr);

            return proc_ptr;
        }
        void _refreshandcopy(list& target)
        {
            target.remove_if([&](auto&& a)
                            {
                                if (a.expired())
                                {
                                    return true;
                                }
                                else
                                {
                                    qcopy.emplace_back(a);
                                    return faux;
                                }
                            });
        }
        // reactor: Thread-safe invoke an event handler.
        //          Return number of active handlers.
        template<class F>
        auto notify(hint e, F&& args)
        {
            events::sync lock;

            queue.push_back(e);

            auto head = qcopy.size();

            if (order == exec::forward)
            {
                hint itermask = events::level_mask(e);
                hint subgroup = e;
                _refreshandcopy(stock[e]);
                while (subgroup)
                {
                    subgroup = events::subgroup_fwd(e, itermask);
                    _refreshandcopy(stock[subgroup]);
                }
            }
            else
            {
                hint itermask = 0;
                hint subgroup;
                do
                {
                    subgroup = events::subgroup(e, itermask);
                    _refreshandcopy(stock[subgroup]);
                }
                while (subgroup != e);
            }

            auto tail = qcopy.size();
            auto size = tail - head;
            if (size)
            {
                auto perform = [&](auto iter)
                {
                    if (auto proc_ptr = qcopy[iter].lock())
                    {
                        //if (auto compatible = dynamic_cast<wrapper<F>*>(proc_ptr.get()))
                        if (auto compatible = static_cast<wrapper<F>*>(proc_ptr.get()))
                        {
                            compatible->proc(std::forward<F>(args));
                        }
                    }
                };

                auto iter = head;
                do    perform(iter++);
                while (alive && iter != tail);

                alive = true;
                qcopy.resize(head);
            }

            queue.pop_back();
            return size;
        }
        // reactor: Thread-safe invoke an event handler.
        //          Return number of active handlers.
        template<class F>
        auto operator()(hint e, F&& args)
        {
            return notify(e, std::forward<F>(args));
        }
        // reactor: Interrupt current event branch.
        void discontinue()
        {
            alive = faux;
        }
    };

    template<class T>
    struct indexer
    {
        using imap = std::map<id_t, wptr<T>>;
        const id_t id;

        static wptr<T> empty;
        static id_t    newid;
        static imap    store;

    protected:
        indexer(indexer const&) = delete; // id is flushed out when
                                          // a copy of the object is deleted.
                                          // Thus, the original object instance
                                          // becomes invalid.
                                          // We should delete the copy ctor.
        indexer()
            : id { _counter() }
        { }
        ~indexer()
        {
            events::sync lock;
            _nullify();
        }

    private:
        static id_t _counter()
        {
            events::sync lock;
            while (netxs::on_key(store, ++newid)) {}
            return newid;
        }
        void _actuate(wptr<T> This)
        {
            store[id] = This;
        }
        void _nullify()
        {
            store.erase(id);
        }

    public:
        // indexer: Return shared_ptr of the object by its id.
        static auto getref(id_t id)
        {
            events::sync lock;
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

            events::sync lock;
            sptr<TT> inst = std::make_shared<make_shared_enabler>(std::forward<Args>(args)...);

            inst->_actuate(inst);

            sptr<T> item = inst;
            //inst->T::signal_direct(e2_base::release, e2::form::upon::created, item);
            return inst;
        }
    };

    // events: Ext link statics, unique ONLY for concrete T.
    template<class T> id_t                      indexer<T>::newid = 0;
    template<class T> wptr<T>                   indexer<T>::empty;
    template<class T> typename indexer<T>::imap indexer<T>::store;

    using hook = reactor::hook;
    class subs
    {
        std::vector<hook> tokens;

    public:
        template<class REACTOR, class EVENTS, class F>
        void operator()(REACTOR& r, EVENTS e, std::function<void(F&&)> h)
        {
            tokens.push_back(r.subscribe(e, h));
        }
        void operator()(hook& t){        tokens.push_back(t);   }
        hook& extra()           { return tokens.emplace_back(); }
        auto  count() const     { return tokens.size();         }
        void  clear()           {        tokens.clear();        }
        void  merge(subs const& memo)
        {
            tokens.reserve(tokens.size() + memo.tokens.size());
            for(auto& t : memo.tokens)
                tokens.push_back(t);
        }
    };

    template<class _parent_type, class _object_type, auto _event_id>
    struct type_clue
    {
        using type = _object_type;
        using base = _parent_type;
        static constexpr auto id = _event_id;

        template<class ...Args> constexpr type_clue(Args&&...) { }
        template<std::size_t N>
        static constexpr auto group() { return events::group<N>(id); }
        static constexpr auto index() { return events::item    (id); }
    };

    template<class ...Args>
    struct array
    {
        constexpr array(Args...) { }
        template<auto N> static constexpr
        auto at = std::get<N>( std::tuple<typename std::remove_cv<Args>::type...>{} );
    };

    #define SUBMIT(        level, item, arg)        bell::template submit2<decltype(item)>(level)        = [&] (typename decltype(item)::type && arg)
    #define SUBMIT_BYVAL(  level, item, arg)        bell::template submit2<decltype(item)>(level)        = [=] (typename decltype(item)::type && arg) mutable
    #define SUBMIT_T(      level, item, token, arg) bell::template submit2<decltype(item)>(level, token) = [&] (typename decltype(item)::type && arg)
    #define SUBMIT_BYVAL_T(level, item, token, arg) bell::template submit2<decltype(item)>(level, token) = [=] (typename decltype(item)::type && arg) mutable
    #define SUBMIT_TV(     level, item, token, hndl)bell::template submit <decltype(item)>(level, token, hndl)
    #define SUBMIT_V(      level, item, hndl)       bell::template submit <decltype(item)>(level, hndl)
    #define SIGNAL(        level, item, arg)        bell::template signal<level>(decltype(item)::id, static_cast<typename decltype(item)::type &&>(arg))
    #define SIGNAL_GLOBAL( item, arg)               bell::template signal_global(decltype(item)::id, static_cast<typename decltype(item)::type &&>(arg))
    #define SUBMIT_GLOBAL( item, token, arg)        bell::template submit_global<decltype(item)>(token) =  [&]  (typename decltype(item)::type &&  arg)

    #define EVENTPACK( name, base ) using _group_type = name; \
                                    static constexpr auto _counter_base = __COUNTER__; \
                                    public: static constexpr auto any = type_clue<_group_type, decltype(base)::type, decltype(base)::id>
    #define  EVENT_XS( name, type ) }; static constexpr auto name = type_clue<_group_type, type, decltype(any)::id | ((__COUNTER__ - _counter_base) << netxs::events::level_width(decltype(any)::id))>{ 777
    #define  GROUP_XS( name, type ) EVENT_XS( _##name, type )
    #define SUBSET_XS( name )       }; class name { EVENTPACK( name, _##name )
    #define  INDEX_XS(  ... )       }; template<auto N> static constexpr \
                                    auto _ = decltype(netxs::events::array{ __VA_ARGS__ })::at<N>; \
                                    private: static constexpr auto _dummy = { 777
    //todo unify seeding
    namespace userland
    {
        struct root
        {
            static constexpr auto root_event = type_clue<root, void, 0>{};
            EVENTPACK( root, root_event )
            {
                EVENT_XS( dtor  , const id_t ),
                EVENT_XS( base  , root ),
                EVENT_XS( hids  , root ),
                EVENT_XS( custom, root ),
            };
        };
    }

    // events: Event x-mitter.
    struct bell : public indexer<bell>
    {
        static constexpr auto noid = std::numeric_limits<id_t>::max();
        subs tracker;

    private:
        template<class V>
        struct _globals
        {
            static reactor general;
        };
        reactor& general;                     // bell: Global  events node relay.
        reactor  preview{ reactor::reverse }; // bell: Preview events node relay.
        reactor  request{ reactor::forward }; // bell: Request events node relay.
        reactor  release{ reactor::forward }; // bell: Release events node relay.

        template<class EVENT>
        struct submit_helper
        {
            bell& owner;
            tier  level;

            submit_helper(bell& owner, tier level)
                : owner{ owner }, level{ level }
            { }

            template<class F>
            void operator=(F h) { owner.submit<EVENT>(level, h); }
        };
        template<class EVENT>
        struct submit_helper_token
        {
            bell& owner;
            hook& token;
            tier  level;

            submit_helper_token(bell& owner, tier level, hook& token)
                : owner{ owner }, level{ level }, token{ token }
            { }

            template<class F>
            void operator=(F h) { owner.submit<EVENT>(level, token, h); }
        };
        template<class EVENT>
        struct submit_helper_token_global
        {
            hook& token;

            submit_helper_token_global(hook& token)
                : token{ token }
            { }

            template<class F>
            void operator=(F h)
            {
                auto handler = std::function<void(typename EVENT::type &&)>{ h };
                token = _globals<void>::general.subscribe(EVENT::id, handler);
            }
        };

    public:
        void merge(sptr<bell> source_ptr)
        {
            auto& s = *source_ptr;
            tracker.merge(s.tracker); //todo deprecate tokens copying
            preview.merge(s.preview);
            request.merge(s.request);
            release.merge(s.release);
        }
        // bell: Subscribe on a specified event
        //       of specified reaction node by defining an event
        //       handler. Return a lambda reference helper.
        template<class EVENT>
        auto submit2(tier level)
        {
            return submit_helper<EVENT>(*this, level);
        }
        //  bell: Subscribe on a specified event
        //        of specified reaction node by defining an event
        //        handler and token. Return a lambda reference helper.
        template<class EVENT>
        auto submit2(tier level, hook& token)
        {
            return submit_helper_token<EVENT>(*this, level, token);
        }
        template<class EVENT>
        auto submit2(tier level, subs& tokens)
        {
            return submit_helper_token<EVENT>(*this, level, tokens.extra());
        }
        // bell: Subscribe to an specified event on specified
        //       reaction node by defining an event handler.
        template<class EVENT>
        void submit(tier level, std::function<void(typename EVENT::type &&)> handler)
        {
            switch (level)
            {
                case tier::release: tracker(release, EVENT::id, handler); break;
                case tier::preview: tracker(preview, EVENT::id, handler); break;
                case tier::general: tracker(general, EVENT::id, handler); break;
                case tier::request: tracker(request, EVENT::id, handler); break;
                default: break;
            }
        }
        // bell: Subscribe to an specified event
        //       on specified reaction node by defining
        //       an event handler, and store the subscription
        //       in the specified token.
        template<class EVENT>
        void submit(tier level, hook& token, std::function<void(typename EVENT::type &&)> handler)
        {
            switch (level)
            {
                case tier::release: token = release.subscribe(EVENT::id, handler); break;
                case tier::preview: token = preview.subscribe(EVENT::id, handler); break;
                case tier::general: token = general.subscribe(EVENT::id, handler); break;
                case tier::request: token = request.subscribe(EVENT::id, handler); break;
                default: break;
            }
        }
        // bell: Subscribe to an specified event
        //       on global reaction node by defining
        //       an event handler, and store the subscription
        //       in the specified token.
        template<class EVENT>
        static auto submit_global(hook& token)
        {
            return submit_helper_token_global<EVENT>(token);
        }
        // bell: Rise specified event execution branch on the specified relay node.
        //       Return number of active handlers.
        template<tier TIER, class F>
        auto signal(type action, F&& data)
        {
            switch (TIER)
            {
                case tier::release: return release(action, std::forward<F>(data));
                case tier::preview: return preview(action, std::forward<F>(data));
                case tier::general: return general(action, std::forward<F>(data));
                case tier::request: return request(action, std::forward<F>(data));
                default:            return 0_sz;
            }
        }
        // bell: Rise specified event globally.
        template<class F>
        static auto signal_global(type action, F&& data)
        {
            return _globals<void>::general(action, std::forward<F>(data));
        }
        // bell: Save up external subscription token.
        void saveup(hook& token)
        {
            tracker(token);
        }
        // bell: Return an initial event of the current event execution branch.
        template<tier TIER>
        auto protos()
        {
            //todo type{ 0 }? e2::any?
            switch (TIER)
            {
                case tier::release: return release.queue.empty() ? type{ 0 } : release.queue.back();
                case tier::preview: return preview.queue.empty() ? type{ 0 } : preview.queue.back();
                case tier::general: return general.queue.empty() ? type{ 0 } : general.queue.back();
                case tier::request: return request.queue.empty() ? type{ 0 } : request.queue.back();
                default:            return                         type{ 0 };
            }
        }
        // bell: Return true if tha initial event equals to the specified.
        template<tier TIER, class EVENT>
        auto protos(EVENT)
        {
            return bell::protos<TIER>() == EVENT::id;
        }
        // bell: Get the reference to the specified relay node.
        auto& router(tier level)
        {
            switch (level)
            {
                default:
                case tier::release: return release;
                case tier::preview: return preview;
                case tier::general: return general;
                case tier::request: return request;
            }
        }
        // bell: Interrupt current event branch on the specified relay node.
        void expire(tier level)
        {
            switch (level)
            {
                case tier::release: release.discontinue(); break;
                case tier::preview: preview.discontinue(); break;
                case tier::general: general.discontinue(); break;
                case tier::request: request.discontinue(); break;
                default: break;
            }
        }
        bell()
            : general{ _globals<void>::general }
        { }
        ~bell()
        {
            events::sync lock;
            SIGNAL(tier::release, userland::root::dtor, id);
        }
        // bell: Recursively calculate global coordinate.
        virtual void global(twod& coor)
        { }
    };

    template<class T> reactor bell::_globals<T>::general{ reactor::forward };
}

#endif // NETXS_EVENTS_HPP