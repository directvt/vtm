// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_EVENTS_HPP
#define NETXS_EVENTS_HPP

// Description: Compile time typesafe hierarchical delegates.

#include "../abstract/ptr.hpp"
#include "../abstract/hash.hpp"

#include <vector>
#include <mutex>
#include <map>
#include <list>
#include <functional>
#include <optional>
#include <thread>

#ifndef faux
    #define faux (false)
#endif

namespace netxs::events
{
    //todo unify/automate structure replenishment
    struct e2_base
    {
        using type = unsigned int;

    protected:
        static const unsigned int _width = 4;
        static const unsigned int _mask = (1 << _width) - 1;
        template<class V>
        struct _globals
        {
            static std::recursive_mutex mutex; // e2: shared mutex.
        };

    public:
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

        enum tier
        {
            // Forward means from particular to general: 1. event_group::item, 2. event_group::any
            // Reverse means from general to particular: 1. event_group::any , 2. event_group::item
            release, // event: Run forwrad handlers with fixed param. Preserve subscription order.
            preview, // event: Run reverse handlers with fixed a param intended to change. Preserve subscription order.
            general, // event: Run forwrad handlers for all objects. Preserve subscription order.
            request, // event: Run forwrad a handler that provides the current value of the param. To avoid overriding, the handler should be the only one. Preserve subscription order.
        };

        // e2 (static): Return item/msg level by its ID.
        constexpr static iota level(type msg)
        {
            if (msg == 0) return 0;
            iota level = 1;
            while ((msg = msg >> _width))
            {
                level++;
            }
            return level;
        }
        // e2 (static): Return item/msg global level mask by its ID.
        constexpr static unsigned int level_mask(type msg)
        {
            unsigned int level = 0;
            while ((msg = msg >> _width))
            {
                level += _width;
            }
            return (1 << level) - 1;
        }

        // e2 (static): Increament level offset by width and return item's subgroup ID fof the specified level offset.
        template<class T>
        constexpr static T subgroup(T msg, type& itermask)
        {
            itermask = (itermask << _width) + _mask;
            return static_cast<T>(msg & itermask);
        }
        template<class T>
        constexpr static T subgroup_fwd(T msg, type& itermask)
        {
            auto result = msg & itermask;
            itermask = (itermask >> _width);
            return static_cast<T>(result);
        }
        //constexpr static type subgroup_fwd(type msg, type& level_offset)
        //{
        //    level_offset += _width;
        //    return msg & ((1 << level_offset) - 1);
        //}
        // e2 (static): Return event's group ID.
        template<class T>
        constexpr static T parent(T msg)
        {
            return static_cast<T>(msg & ((1 << ((level(msg) - 1) * _width)) - 1));
        }
        // e2 (static): Return the event ID of the specified item inside the group.
        template<class T>
        constexpr static const T message(T base, type item)
        {
            return static_cast<T>(base | ((item + 1) << level(base) * _width));
        }
        // e2 (static): Return item index inside the group by its ID.
        constexpr static unsigned int item(type msg)
        {
            return (msg >> ((level(msg) - 1) * _width)) - 1;
        }
    protected:
        template<std::size_t N, std::size_t... I>
        constexpr static auto _instantiate(type base, std::index_sequence<I...>)
        {
            return std::array<unsigned int, N>{ message(base, I)... };
        }
    public:
        template<std::size_t N>
        constexpr static auto group(type base)
        {
            return _instantiate<N>(base, std::make_index_sequence<N>{});
        }
    };

    struct e2
        : public e2_base
    {
        #define EVENT(event) event = any | (((__COUNTER__ - _counter_base) << (e2::level(any) * _width)))
        #define GROUP(group) EVENT(_##group)
        #define TOPGROUP(group) private: enum : type { _counter_base = __COUNTER__ }; \
                                private: enum : type { _ = _##group };                \
                                public:  enum : type
        #define SUBGROUP(group) }; struct group { TOPGROUP(group)

        private: static const type _root_event = 0;
        TOPGROUP(root_event)
        {
            any = _,
            GROUP( timer     ),
            GROUP( term      ),
            GROUP( form      ),
            GROUP( hids      ),
            GROUP( data      ),
            GROUP( debug     ), // return info struct with telemtry data
            GROUP( config    ), // set/notify/get/global_set configuration data (e2::preview/e2::release/e2::request/e2::general)
            GROUP( command   ), // exec UI command (arg: iota)
            GROUP( bindings  ), // Dynamic Data Bindings.
            GROUP( render    ), // release: UI-tree rendering (arg: face).
            GROUP( size      ), // release: Object size (arg: twod).
            GROUP( coor      ), // release: Object coor (arg: twod).
            GROUP( custom    ), // Custom events subset.
            EVENT( dtor      ), // Notify about object destruction, release only (arg: const id_t)
            EVENT( postrender), // release: UI-tree post-rendering (arg: face).

            SUBGROUP( size )
            {
                any = _,      // preview: checking by pro::limit (arg: twod).
                EVENT( set ), // preview: checking by object; release: apply to object (arg: twod).
            };
            SUBGROUP( coor )
            {
                any = _,      // preview any: checking by pro::limit (arg: twod).
                EVENT( set ), // preview: checking by object; release: apply to object (arg: twod).
            };
            SUBGROUP( render )
            {
                any = _,            // release any: UI-tree default rendering submission (arg: face).
                EVENT( prerender ), // release: UI-tree pre-rendering, used by pro::cache (can interrupt SIGNAL) and any kind of highlighters (arg: face).
            };
            SUBGROUP( bindings )
            {
                any = _,
                GROUP( list ), // release: UI-tree pre-rendering, used by pro::cache (can interrupt SIGNAL) and any kind of highlighters (arg: face).

                SUBGROUP( list )
                {
                    any = _,
                    EVENT( users ), // list of connected users (arg: sptr<std::list<sptr<base>>>)
                    EVENT( apps  ), // list of running apps (arg: sptr<std::map<id_t, std::list<sptr<base>>>>)
                };
            };
            SUBGROUP( debug )
            {
                any = _,
                EVENT( logs   ), // logs output (arg: const text)
                EVENT( output ), // logs has to be parsed (arg: const view)
                EVENT( parsed ), // output parced logs (arg: const page)
            };
            SUBGROUP( timer )
            {
                any = _,
                EVENT( tick ), // timer tick (arg: current moment (now))
                EVENT( fps  ), // request to set new fps (arg: new fps (iota); the value == -1 is used to request current fps)
            };
            SUBGROUP( config )
            {
                any = _,
                GROUP( caret     ), // any kind of intervals property (arg: period)
                EVENT( broadcast ), // release: broadcast source changed, args: sptr<bell>.

                SUBGROUP( caret )
                {
                    any = _,
                    EVENT( blink ), // caret blinking interval (arg: period)
                    EVENT( style ), // caret style: 0 - underline, 1 - box (arg: iota)
                };
            };
            SUBGROUP( term )
            {
                any = _,
                EVENT( unknown  ), // return platform unknown event code
                EVENT( error    ), // return error code
                EVENT( focus    ), // order to change focus (arg: bool)
                EVENT( key      ), // keybd activity (arg: syskeybd)
                EVENT( native   ), // extended functionality (arg: bool)
                EVENT( mouse    ), // mouse activity (arg: sysmouse)
                EVENT( size     ), // order to update terminal primary overlay (arg: newsize twod)
                EVENT( layout   ),
                EVENT( preclose ), // signal to quit after idle timeout (arg: bool - ready to shutdown)
                EVENT( quit     ), // quit (arg: text - bye msg)
                EVENT( pointer  ), // mouse pointer visibility (arg: bool)
                //EVENT( menu   ), 
            };
            SUBGROUP( data )
            {
                any = _,
                EVENT( changed ), // return digest
                EVENT( request ),
                EVENT( disable ),
                EVENT( flush   ),
                EVENT( text    ), // release: signaling with a text string (args: const text).
            };
            SUBGROUP( command )
            {
                any = _,
                EVENT( quit   ), // return bye msg //errcode (arg: const view)
                EVENT( cout   ), // Append extra data to output (arg: const text)
                EVENT( custom ), // Custom command (arg: cmd_id iota)
            };
            SUBGROUP( hids )
            {
                any = _,
                GROUP( keybd ),
                GROUP( mouse ),

                SUBGROUP( keybd )
                {
                    any = _,
                    GROUP( control ),
                    GROUP( state   ),
                    EVENT( down    ),
                    EVENT( up      ),

                    SUBGROUP( control )
                    {
                        any = _,
                        GROUP( up      ),
                        GROUP( down    ),

                        SUBGROUP( up )
                        {
                            any = _,
                            EVENT( alt_right   ),
                            EVENT( alt_left    ),
                            EVENT( ctrl_right  ),
                            EVENT( ctrl_left   ),
                            EVENT( shift_right ),
                            EVENT( shift_left  ),
                        };
                        SUBGROUP( down )
                        {
                            any = _,
                            EVENT( alt_right   ),
                            EVENT( alt_left    ),
                            EVENT( ctrl_right  ),
                            EVENT( ctrl_left   ),
                            EVENT( shift_right ),
                            EVENT( shift_left  ),
                        };
                    };
                    SUBGROUP( state )
                    {
                        any = _,
                        GROUP( on  ),
                        GROUP( off ),

                        SUBGROUP( on )
                        {
                            any = _,
                            EVENT( numlock    ),
                            EVENT( capslock   ),
                            EVENT( scrolllock ),
                            EVENT( insert     ),
                        };
                        SUBGROUP( off )
                        {
                            any = _,
                            EVENT( numlock    ),
                            EVENT( capslock   ),
                            EVENT( scrolllock ),
                            EVENT( insert     ),
                        };
                    };
                };
                SUBGROUP( mouse )
                {
                    any = _,
                    GROUP( button  ),
                    GROUP( scroll  ),
                    EVENT( move    ),
                    EVENT( shuffle ), // movement within one cell
                    EVENT( focus   ),
                    EVENT( gone    ), // release::global: Notify about the mouse controller is gone (args: hids).

                    SUBGROUP( scroll )
                    {
                        any = _,
                        EVENT( up   ),
                        EVENT( down ),
                    };
                    SUBGROUP( button )
                    {
                        any = _,
                        GROUP( up       ),
                        GROUP( down     ),
                        GROUP( click    ),
                        GROUP( dblclick ),
                        GROUP( drag     ),

                        SUBGROUP( up )
                        {
                            any = _,
                            EVENT( left      ),
                            EVENT( right     ),
                            EVENT( leftright ),
                            EVENT( middle    ),
                            EVENT( wheel     ),
                            EVENT( win       ),
                        };
                        SUBGROUP( down )
                        {
                            any = _,
                            EVENT( left      ),
                            EVENT( right     ),
                            EVENT( leftright ),
                            EVENT( middle    ),
                            EVENT( wheel     ),
                            EVENT( win       ),
                        };
                        SUBGROUP( click )
                        {
                            any = _,
                            EVENT( left      ),
                            EVENT( right     ),
                            EVENT( leftright ),
                            EVENT( middle    ),
                            EVENT( wheel     ),
                            EVENT( win       ),
                        };
                        SUBGROUP( dblclick )
                        {
                            any = _,
                            EVENT( left      ),
                            EVENT( right     ),
                            EVENT( leftright ),
                            EVENT( middle    ),
                            EVENT( wheel     ),
                            EVENT( win       ),
                        };
                        SUBGROUP( drag )
                        {
                            any = _,
                            GROUP( start  ),
                            GROUP( pull   ),
                            GROUP( cancel ),
                            GROUP( stop   ),

                            SUBGROUP( start )
                            {
                                any = _,
                                EVENT( left      ),
                                EVENT( right     ),
                                EVENT( leftright ),
                                EVENT( middle    ),
                                EVENT( wheel     ),
                                EVENT( win       ),
                            };
                            SUBGROUP( pull )
                            {
                                any = _,
                                EVENT( left      ),
                                EVENT( right     ),
                                EVENT( leftright ),
                                EVENT( middle    ),
                                EVENT( wheel     ),
                                EVENT( win       ),
                            };
                            SUBGROUP( cancel )
                            {
                                any = _,
                                EVENT( left      ),
                                EVENT( right     ),
                                EVENT( leftright ),
                                EVENT( middle    ),
                                EVENT( wheel     ),
                                EVENT( win       ),
                            };
                            SUBGROUP( stop )
                            {
                                any = _,
                                EVENT( left      ),
                                EVENT( right     ),
                                EVENT( leftright ),
                                EVENT( middle    ),
                                EVENT( wheel     ),
                                EVENT( win       ),
                            };

                        };
                    };
                };
            };

            SUBGROUP( form )
            {
                any = _,
                GROUP( draggable ), // signal to the form to enable draggablity for specified mouse button (arg: bool)
                GROUP( layout    ),
                GROUP( highlight ),
                GROUP( upon      ),
                GROUP( proceed   ),
                GROUP( cursor    ),
                GROUP( animate   ),
                GROUP( drag      ),
                GROUP( prop      ),
                GROUP( global    ),
                GROUP( state     ),
                GROUP( upevent   ), // eventss streamed up (to children) of the visual tree by base::
                GROUP( notify    ), // Form events that should be propagated down to the visual branch
                EVENT( canvas    ), // request global canvas (arg: sptr<core>)
                //EVENT( key       ),

                SUBGROUP( draggable )
                {
                    any = _,
                    EVENT( left      ),
                    EVENT( right     ),
                    EVENT( leftright ),
                    EVENT( middle    ),
                    EVENT( wheel     ),
                    EVENT( win       ),
                };
                SUBGROUP( layout )
                {
                    any = _,
                    EVENT( shift        ), // request a global shifting  with delta (const twod)
                    EVENT( convey       ), // request a global conveying with delta (Inform all children to be conveyed) (arg: cube)
                    EVENT( order        ), // return
                    EVENT( local        ), // Recursively calculate local coordinate from global (arg: twod)
                    EVENT( strike       ), // (always preview) inform about the child canvas has changed (arg: modified region rect)
                    EVENT( bubble       ), // order to popup the requested item through the visual tree (arg: form)
                    EVENT( expose       ), // order to bring the requested item on top of the visual tree (release: ask parent to expose specified child; preview: ask child to expose itself) (arg: base)
                    EVENT( appear       ), // fly tothe specified coords, arg: twod
                    //EVENT( coor       ), // return client rect coor (preview: subject to change)
                    //EVENT( size       ), // return client rect size (preview: subject to change)
                    //EVENT( rect       ), // return client rect (preview: subject to change)
                    //EVENT( show       ), // order to make it visible (arg: bool notify or not)
                    //EVENT( hide       ), // order to make it hidden (arg: bool notify or not)
                    //EVENT( next       ), // request client for next child object (arg is only request: sptr<base>)
                    //EVENT( prev       ), // request client for prev child object (arg is only request: sptr<base>)
                    //EVENT( footer     ), // notify the client has changed footer (arg is only release: const rich)
                    //EVENT( clientrect ), // notify the client area has changed (arg is only release: rect)
                };
                SUBGROUP( highlight )
                {
                    any = _,
                    EVENT( on  ),
                    EVENT( off ),
                };
                //SUBGROUP( focus )
                //{
                //    any = _,
                //    EVENT( got  ), // notify that keybd focus has taken (release: hids)
                //    EVENT( lost ), // notify that keybd focus got lost  (release: hids)
                //};
                SUBGROUP( upon )
                {
                    any = _,
                    GROUP( vtree       ), // visual tree events (arg: parent base_sptr)
                    GROUP( scroll      ), // event after scroll (arg: rack)
                    EVENT( redrawn     ), // inform about camvas is completely redrawn (arg: canvas face)
                    EVENT( cached      ), // inform about camvas is cached (arg: canvas face)
                    EVENT( wiped       ), // event after wipe the canvas (arg: canvas face)
                    EVENT( created     ), // event after itself creation (arg: itself bell_sptr)
                    EVENT( changed     ), // event after resize (arg: diff bw old and new size twod)
                    EVENT( dragged     ), // event after drag (arg: hids)
                    //EVENT( detached    ), // inform that subject is detached (arg: parent bell_sptr)
                    //EVENT( invalidated ), 
                    //EVENT( moved       ), // release: event after moveto (arg: diff bw old and new coor twod). preview: event after moved by somebody.

                    SUBGROUP( vtree )
                    {
                        any = _,
                        EVENT( attached ), // Child has been attached (arg: parent sptr<base>)
                        EVENT( detached ), // Child has been detached (arg: parent sptr<base>)
                    };
                    SUBGROUP( scroll )
                    {
                        any = _,
                        EVENT( x      ), // event after scroll along X (arg: rack)
                        EVENT( y      ), // event after scroll along Y (arg: rack)
                        EVENT( resetx ), // event reset scroll along X (arg: rack)
                        EVENT( resety ), // event reset scroll along Y (arg: rack)
                    };
                };
                SUBGROUP( proceed )
                {
                    any = _,
                    EVENT( create      ), // return coordinates of the new object placeholder (arg: rect)
                    EVENT( createby    ), // return gear with coordinates of the new object placeholder gear::slot (arg: gear)
                    EVENT( destroy     ), // ??? bool return reference to the parent
                    EVENT( render      ), // ask children to render itself to the parent canvas (arg: function drawfx to perform drawing)
                    EVENT( attach      ), // order to attach a child (arg: parent base_sptr)
                    EVENT( detach      ), // order to detach a child (e2::release - kill itself, e2::preview - detach the child specified in args) (arg: child  base_sptr)
                    //EVENT( commit      ), // order to output the targets (arg: frame number iota)
                    //EVENT( multirender ), // ask children to render itself to the set of canvases (arg: array of the face sptrs: cuts = vector<shared_ptr<face>>)
                    //EVENT( draw        ), // ????  order to render itself to the canvas (arg: canvas face)
                    //EVENT( checkin     ), // order to register an output client canvas (arg: face_sptr)
                };
                SUBGROUP( cursor )
                {
                    any = _,
                    EVENT(blink),
                };
                SUBGROUP( animate )
                {
                    any = _,
                    EVENT( start ),
                    EVENT( stop  ),
                };
                SUBGROUP( drag )
                {
                    any = _,
                    GROUP( start  ), // notify about mouse drag start by pro::mouse (arg: hids)
                    GROUP( pull   ), // notify about mouse drag pull by pro::mouse (arg: hids)
                    GROUP( cancel ), // notify about mouse drag cancel by pro::mouse (arg: hids)
                    GROUP( stop   ), // notify about mouse drag stop by pro::mouse (arg: hids)

                    SUBGROUP( start  )
                    {
                        any = _,
                        EVENT( left      ),
                        EVENT( right     ),
                        EVENT( leftright ),
                        EVENT( middle    ),
                        EVENT( wheel     ),
                        EVENT( win       ),
                    };
                    SUBGROUP( pull   )
                    {
                        any = _,
                        EVENT( left      ),
                        EVENT( right     ),
                        EVENT( leftright ),
                        EVENT( middle    ),
                        EVENT( wheel     ),
                        EVENT( win       ),
                    };
                    SUBGROUP( cancel )
                    {
                        any = _,
                        EVENT( left      ),
                        EVENT( right     ),
                        EVENT( leftright ),
                        EVENT( middle    ),
                        EVENT( wheel     ),
                        EVENT( win       ),
                    };
                    SUBGROUP( stop   )
                    {
                        any = _,
                        EVENT( left      ),
                        EVENT( right     ),
                        EVENT( leftright ),
                        EVENT( middle    ),
                        EVENT( wheel     ),
                        EVENT( win       ),
                    };
                };
                SUBGROUP( prop )
                {
                    any = _,
                    EVENT( header     ), // set form caption header (arg: text)
                    EVENT( footer     ), // set form caption footer (arg: text)
                    EVENT( zorder     ), // set form z-order (arg: iota: -1 backmost, 0 plain, 1 topmost)
                    EVENT( brush      ), // set form brush/color (arg: cell)
                    EVENT( fullscreen ), // set fullscreen flag (arg: bool)
                    EVENT( name       ), // user name (arg: text)
                    EVENT( viewport   ), // request: return form actual viewport (arg: rect)
                };
                SUBGROUP( global )
                {
                    any = _,
                    GROUP( object   ), // global scene objects events
                    GROUP( user     ), // global scene users events
                    EVENT( ctxmenu  ), // request context menu at specified coords (arg: twod)
                    EVENT( prev     ), // request the prev scene window (arg: twod)
                    EVENT( next     ), // request the next scene window (arg: twod)
                    EVENT( lucidity ), // set or request global window transparency (arg: iota: 0-255, -1 to request)

                    SUBGROUP( object )
                    {
                        any = _,
                        EVENT( attached ), // global: object attached to the world (args: sptr<base>)
                        EVENT( detached ), // global: object detached from the world (args: sptr<base>)
                    };
                    SUBGROUP( user )
                    {
                        any = _,
                        EVENT( attached ), // global: user attached to the world (args: sptr<base>)
                        EVENT( detached ), // global: user detached from the world (args: sptr<base>)
                    };
                };
                SUBGROUP( state )
                {
                    any = _,
                    EVENT( mouse  ), // notify the client is mouse active or not. The form is active when the number of client (form::eventa::mouse::enter - mouse::leave) is not zero. (arg is only release: iota - number of clients)
                    EVENT( keybd  ), // notify the client is keybd active or not. The form is active when the number of client (form::eventa::keybd::got - keybd::lost) is not zero. (arg is only release: bool)
                    EVENT( header ), // notify the client has changed title  (arg: para)
                    EVENT( footer ), // notify the client has changed footer (arg: para)
                    EVENT( params ), // notify the client has changed title params (arg: para)
                    EVENT( color  ), // notify the client has changed tone (preview to set, arg: tone)
                };
                SUBGROUP( upevent )
                {
                    any = _,
                    EVENT( kboffer ), // inform nested objects that the keybd focus should be taken (arg: hids)
                };
                SUBGROUP( notify )
                {
                    any = _,
                    GROUP( mouse ), // request context menu at specified coords (arg: twod)
                    GROUP( keybd ), // request the prev scene window (arg: twod)

                    SUBGROUP( mouse )
                    {
                        any = _,        // inform the form about the mouse hover (arg: hids)
                        EVENT( enter ), // inform the form about the mouse hover (arg: hids)
                        EVENT( leave ), // inform the form about the mouse leave (arg: hids)
                    };
                    SUBGROUP( keybd )
                    {
                        any = _,
                        EVENT( got  ),
                        EVENT( lost ),
                    };
                };
            };
        };

        #undef EVENT
        #undef GROUP
        #undef TOPGROUP
        #undef SUBGROUP
    };

    template<class V>
    std::recursive_mutex e2::_globals<V>::mutex; // e2: shared mutex.

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
        using hint = e2::type;

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

            e2::sync lock;

            stock[e].push_back(proc_ptr);

            return proc_ptr;
        }
        //template<class F>
        //auto subscribe(hint e)
        //{
        //	auto proc_ptr = std::make_shared<wrapper<F>>(nullptr);
        //
        //	e2::sync lock;
        //
        //	stock[e].push_back(proc_ptr);
        //
        //	return proc_ptr;
        //}

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
            e2::sync lock;

            queue.push_back(e);

            auto head = qcopy.size();

            if (order == exec::forward)
            {
                hint itermask = e2::level_mask(e);
                hint subgroup = e;
                _refreshandcopy(stock[e]);
                while (subgroup)
                {
                    subgroup = e2::subgroup_fwd(e, itermask);
                    _refreshandcopy(stock[subgroup]);
                }
            }
            else
            {
                hint itermask = 0;
                hint subgroup;
                do
                {
                    subgroup = e2::subgroup(e, itermask);
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
        using id_t = uint32_t;
        using imap = std::map<id_t, wptr<T>>;
        const id_t id;

        static wptr<T> empty;
        static id_t    newid;
        static imap    store;

    protected:
        indexer(indexer const&) = delete;	// id is flushed out when
                                            // a copy of the object is deleted.
                                            // Thus, the original object instance
                                            // becomes invalid.
                                            // We should delete the copy ctr.
        indexer()
            : id { _counter() }
        { }
        ~indexer()
        {
            e2::sync lock;
            _nullify();
        }

    private:
        static id_t _counter()
        {
            e2::sync lock;
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
            e2::sync lock;
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

            e2::sync lock;
            sptr<TT> inst = std::make_shared<make_shared_enabler>(std::forward<Args>(args)...);

            inst->_actuate(inst);

            sptr<T> item = inst;
            inst->T::signal_direct(e2::release, e2::form::upon::created, item);
            return inst;
        }
    };

    // events: Ext link statics, unique ONLY for concrete T.
    template<class T> typename indexer<T>::id_t indexer<T>::newid = 0;
    template<class T> typename indexer<T>::imap indexer<T>::store;
    template<class T> wptr<T>                   indexer<T>::empty;

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
        void operator()(hook& t) { tokens.push_back(t); }
        hook& extra()       { return tokens.emplace_back(); }
        auto  count() const { return tokens.size();         }
        void  clear()       {        tokens.clear();        }
        void  merge(subs const& memo)
        {
            tokens.reserve(tokens.size() + memo.tokens.size());
            for(auto& t : memo.tokens)
                tokens.push_back(t);
        }
    };

    // events: Event x-mitter.
    struct bell : public indexer<bell>
    {
        static constexpr id_t noid = std::numeric_limits<id_t>::max();
        subs tracker;

    private:
        template<class V>
        struct _globals
        {
            static reactor general; // bell: Ext link static.
        };
        reactor& general;                     // bell: Global  events node relay.
        reactor  preview{ reactor::reverse }; // bell: Preview events node relay.
        reactor  request{ reactor::forward }; // bell: Request events node relay.
        reactor  release{ reactor::forward }; // bell: Release events node relay.

        template<class EVENT>
        struct submit_helper
        {
            bell&    boss;
            e2::tier level;

            submit_helper(bell& boss, e2::tier level)
                : boss{ boss }, level{ level }
            { }

            template<class F>
            void operator=(F h) { boss.submit<EVENT>(level, h); }
        };
        template<class EVENT>
        struct submit_helper_token
        {
            bell&    boss;
            hook&    token;
            e2::tier level;

            submit_helper_token(bell& boss, e2::tier level, hook& token)
                : boss{ boss }, level{ level }, token{ token }
            { }

            template<class F>
            void operator=(F h) { boss.submit<EVENT>(level, token, h); }
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
                auto handler = std::function<void(typename EVENT::param &&)>{ h };
                token = _globals<void>::general.subscribe(EVENT::cause, handler);
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
        auto submit2(e2::tier level)
        {
            return submit_helper<EVENT>(*this, level);
        }
        //  bell: Subscribe on a specified event
        //        of specified reaction node by defining an event
        //        handler and token. Return a lambda reference helper.
        template<class EVENT>
        auto submit2(e2::tier level, hook& token)
        {
            return submit_helper_token<EVENT>(*this, level, token);
        }
        template<class EVENT>
        auto submit2(e2::tier level, subs& tokens)
        {
            return submit_helper_token<EVENT>(*this, level, tokens.extra());
        }

        // bell: Subscribe to an specified event on specified
        //       reaction node by defining an event handler.
        template<class EVENT>
        void submit(e2::tier level, std::function<void(typename EVENT::param &&)> handler)
        {
            switch (level)
            {
                case e2::tier::release: tracker(release, EVENT::cause, handler); break;
                case e2::tier::preview: tracker(preview, EVENT::cause, handler); break;
                case e2::tier::general: tracker(general, EVENT::cause, handler); break;
                case e2::tier::request: tracker(request, EVENT::cause, handler); break;
                default:
                    break;
            }
        }
        // bell: Subscribe to an specified event
        //       on specified reaction node by defining
        //       an event handler, and store the subscription
        //       in the specified token.
        template<class EVENT>
        void submit(e2::tier level, hook& token, std::function<void(typename EVENT::param &&)> handler)
        {
            switch (level)
            {
                case e2::tier::release: token = release.subscribe(EVENT::cause, handler); break;
                case e2::tier::preview: token = preview.subscribe(EVENT::cause, handler); break;
                case e2::tier::general: token = general.subscribe(EVENT::cause, handler); break;
                case e2::tier::request: token = request.subscribe(EVENT::cause, handler); break;
                default:
                    break;
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
        //todo used only with indexer::create
        // bell: Rise specified evench execution branch on the specified relay node.
        template<class F>
        void signal_direct(e2::tier level, e2::type action, F&& data)
        {
            switch (level)
            {
                case e2::tier::release: release(action, std::forward<F>(data)); break;
                case e2::tier::preview: preview(action, std::forward<F>(data)); break;
                case e2::tier::general: general(action, std::forward<F>(data)); break;
                case e2::tier::request: request(action, std::forward<F>(data)); break;
                default:
                    break;
            }
        }
        // bell: Rise specified event execution branch on the specified relay node.
        //       Return number of active handlers.
        template<e2::tier TIER, class F>
        auto signal(e2::type action, F&& data)
        {
            switch (TIER)
            {
                case e2::tier::release: return release(action, std::forward<F>(data));
                case e2::tier::preview: return preview(action, std::forward<F>(data));
                case e2::tier::general: return general(action, std::forward<F>(data));
                case e2::tier::request: return request(action, std::forward<F>(data));
                default:
                    return 0_sz;
            }
        }
        // bell: Rise specified event globally.
        template<class F>
        static auto signal_global(e2::type action, F&& data)
        {
            return _globals<void>::general(action, std::forward<F>(data));
        }
        // bell: Save up external subscription token.
        void saveup(hook& token)
        {
            tracker(token);
        }
        // bell: Return an initial event of the current event execution branch.
        template<e2::tier TIER>
        auto protos() -> e2::type
        {
            switch (TIER)
            {
                case e2::tier::release:
                    return release.queue.empty() ? e2::any
                                                 : release.queue.back();
                case e2::tier::preview:
                    return preview.queue.empty() ? e2::any
                                                 : preview.queue.back();
                case e2::tier::general:
                    return general.queue.empty() ? e2::any
                                                 : general.queue.back();
                case e2::tier::request:
                    return request.queue.empty() ? e2::any
                                                 : request.queue.back();
                default:
                    break;
            }
            return e2::any;
        }
        // bell: Return true if tha initial event equals to the specified.
        template<e2::tier TIER>
        auto protos(e2::type action)
        {
            return bell::protos<TIER>() == action;
        }
        // bell: Get the reference to the specified relay node.
        reactor& router(e2::tier level)
        {
            switch (level)
            {
                case e2::tier::release: return release;
                case e2::tier::preview: return preview;
                case e2::tier::general: return general;
                case e2::tier::request: return request;
                default:
                    break;
            }
            return release;
        }
        // bell: Interrupt current event branch on the specified relay node.
        void expire(e2::tier level)
        {
            switch (level)
            {
                case e2::tier::release: release.discontinue(); break;
                case e2::tier::preview: preview.discontinue(); break;
                case e2::tier::general: general.discontinue(); break;
                case e2::tier::request: request.discontinue(); break;
                default:
                    break;
            }
        }
        bell()
            : general{ _globals<void>::general }
        { }
        ~bell()
        {
            e2::sync lock;
            signal<e2::release>(e2::dtor, id);
        }
    };

    template<class T>
    reactor bell::_globals<T>::general{ reactor::forward };

    template<auto T>
    struct type_clue {};

    #define EVENT_BIND(item, item_t)                                   \
    template<>                                                         \
    struct netxs::events::type_clue<item>                              \
    {                                                                  \
        using param = item_t;                                          \
        static constexpr e2::type cause = static_cast<e2::type>(item); \
    };

    #define EVENT_SAME(master, item)                           \
    template<>                                                 \
    struct netxs::events::type_clue<item>                      \
    {                                                          \
        using param = netxs::events::type_clue<master>::param; \
        static constexpr e2::type cause = item;                \
    };

    #define ARGTYPE(item) typename netxs::events::type_clue<item>::param

    // Usage: SUBMIT(tier, item, arg) { ...expression; };
    #define SUBMIT(level, item, arg) \
        bell::template submit2<netxs::events::type_clue<item>>(level) = [&] (ARGTYPE(item)&& arg)

    // Usage: SUBMIT_BYVAL(tier, item, arg) { ...expression; };
    //        Note: It is a mutable closure!
    #define SUBMIT_BYVAL(level, item, arg) \
        bell::template submit2<netxs::events::type_clue<item>>(level) = [=] (ARGTYPE(item)&& arg) mutable

    // Usage: SUBMIT_BYVAL_T(tier, item, token, arg) { ...expression; };
    //        Note: It is a mutable closure!
    #define SUBMIT_BYVAL_T(level, item, token, arg) \
        bell::template submit2<netxs::events::type_clue<item>>(level, token) = [=] (ARGTYPE(item)&& arg) mutable

    #define SUBMIT_V(level, item, hndl) \
        bell::template submit<netxs::events::type_clue<item>>(level, hndl)

    #define SUBMIT_TV(level, item, token, hndl) \
        bell::template submit<netxs::events::type_clue<item>>(level, token, hndl)

    // Usage: SUBMIT_BYVAL(tier, item, token/tokens, arg) { ...expression; };
    #define SUBMIT_T(level, item, token, arg) \
        bell::template submit2<netxs::events::type_clue<item>>(level, token) = [&] (ARGTYPE(item)&& arg)

    #define SIGNAL(level, item, arg) \
        bell::template signal<level>(item, static_cast<ARGTYPE(item)&&>(arg))

    #define SIGNAL_GLOBAL(item, arg) \
        bell::template signal_global(item, static_cast<ARGTYPE(item)&&>(arg))

    #define SUBMIT_GLOBAL(item, token, arg) \
        bell::template submit_global<netxs::events::type_clue<item>>(token) = [&] (ARGTYPE(item)&& arg)
}

#endif // NETXS_EVENTS_HPP