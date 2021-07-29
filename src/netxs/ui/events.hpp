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
        #define EVENTXS(event) event = any | (((__COUNTER__ - _counter_base) << (e2::level(any) * _width)))
        #define GROUPXS(group) EVENTXS(_##group)
        #define TOPGROUPXS(group) private: enum : type { _counter_base = __COUNTER__ }; \
                                private: enum : type { _ = _##group };                \
                                public:  enum : type
        #define SUBGROUPXS(group) }; struct group { TOPGROUPXS(group)

        private: static const type _root_event = 0;
        TOPGROUPXS(root_event)
        {
            any = _,
            GROUPXS( timer      ),
            GROUPXS( term       ),
            GROUPXS( form       ),
            GROUPXS( hids       ),
            GROUPXS( data       ),
            GROUPXS( debug      ), // return info struct with telemtry data
            GROUPXS( config     ), // set/notify/get/global_set configuration data (e2::preview/e2::release/e2::request/e2::general)
            GROUPXS( command    ), // exec UI command (arg: iota)
            GROUPXS( bindings   ), // Dynamic Data Bindings.
            GROUPXS( render     ), // release: UI-tree rendering (arg: face).
            GROUPXS( size       ), // release: Object size (arg: twod).
            GROUPXS( coor       ), // release: Object coor (arg: twod).
            GROUPXS( custom     ), // Custom events subset.
            EVENTXS( dtor       ), // Notify about object destruction, release only (arg: const id_t)
            EVENTXS( postrender ), // release: UI-tree post-rendering (arg: face).

            SUBGROUPXS( size )
            {
                any = _,      // preview: checking by pro::limit (arg: twod).
                EVENTXS( set ), // preview: checking by object; release: apply to object (arg: twod).
            };
            SUBGROUPXS( coor )
            {
                any = _,      // preview any: checking by pro::limit (arg: twod).
                EVENTXS( set ), // preview: checking by object; release: apply to object (arg: twod).
            };
            SUBGROUPXS( render )
            {
                any = _,            // release any: UI-tree default rendering submission (arg: face).
                EVENTXS( prerender ), // release: UI-tree pre-rendering, used by pro::cache (can interrupt SIGNAL) and any kind of highlighters (arg: face).
            };
            SUBGROUPXS( bindings )
            {
                any = _,
                GROUPXS( list ), // release: UI-tree pre-rendering, used by pro::cache (can interrupt SIGNAL) and any kind of highlighters (arg: face).

                SUBGROUPXS( list )
                {
                    any = _,
                    EVENTXS( users ), // list of connected users (arg: sptr<std::list<sptr<base>>>)
                    EVENTXS( apps  ), // list of running apps (arg: sptr<std::map<id_t, std::list<sptr<base>>>>)
                };
            };
            SUBGROUPXS( debug )
            {
                any = _,
                EVENTXS( logs   ), // logs output (arg: const text)
                EVENTXS( output ), // logs has to be parsed (arg: const view)
                EVENTXS( parsed ), // output parced logs (arg: const page)
            };
            SUBGROUPXS( timer )
            {
                any = _,
                EVENTXS( tick ), // timer tick (arg: current moment (now))
                EVENTXS( fps  ), // request to set new fps (arg: new fps (iota); the value == -1 is used to request current fps)
            };
            SUBGROUPXS( config )
            {
                any = _,
                GROUPXS( caret     ), // any kind of intervals property (arg: period)
                EVENTXS( broadcast ), // release: broadcast source changed, args: sptr<bell>.

                SUBGROUPXS( caret )
                {
                    any = _,
                    EVENTXS( blink ), // caret blinking interval (arg: period)
                    EVENTXS( style ), // caret style: 0 - underline, 1 - box (arg: iota)
                };
            };
            SUBGROUPXS( term )
            {
                any = _,
                EVENTXS( unknown  ), // return platform unknown event code
                EVENTXS( error    ), // return error code
                EVENTXS( focus    ), // order to change focus (arg: bool)
                EVENTXS( key      ), // keybd activity (arg: syskeybd)
                EVENTXS( native   ), // extended functionality (arg: bool)
                EVENTXS( mouse    ), // mouse activity (arg: sysmouse)
                EVENTXS( size     ), // order to update terminal primary overlay (arg: newsize twod)
                EVENTXS( layout   ),
                EVENTXS( preclose ), // signal to quit after idle timeout (arg: bool - ready to shutdown)
                EVENTXS( quit     ), // quit (arg: text - bye msg)
                EVENTXS( pointer  ), // mouse pointer visibility (arg: bool)
                //EVENTXS( menu   ), 
            };
            SUBGROUPXS( data )
            {
                any = _,
                EVENTXS( changed ), // return digest
                EVENTXS( request ),
                EVENTXS( disable ),
                EVENTXS( flush   ),
                EVENTXS( text    ), // release: signaling with a text string (args: const text).
            };
            SUBGROUPXS( command )
            {
                any = _,
                EVENTXS( quit   ), // return bye msg //errcode (arg: const view)
                EVENTXS( cout   ), // Append extra data to output (arg: const text)
                EVENTXS( custom ), // Custom command (arg: cmd_id iota)
            };
            SUBGROUPXS( hids )
            {
                any = _,
                GROUPXS( keybd ),
                GROUPXS( mouse ),

                SUBGROUPXS( keybd )
                {
                    any = _,
                    GROUPXS( control ),
                    GROUPXS( state   ),
                    EVENTXS( down    ),
                    EVENTXS( up      ),

                    SUBGROUPXS( control )
                    {
                        any = _,
                        GROUPXS( up   ),
                        GROUPXS( down ),

                        SUBGROUPXS( up )
                        {
                            any = _,
                            EVENTXS( alt_right   ),
                            EVENTXS( alt_left    ),
                            EVENTXS( ctrl_right  ),
                            EVENTXS( ctrl_left   ),
                            EVENTXS( shift_right ),
                            EVENTXS( shift_left  ),
                        };
                        SUBGROUPXS( down )
                        {
                            any = _,
                            EVENTXS( alt_right   ),
                            EVENTXS( alt_left    ),
                            EVENTXS( ctrl_right  ),
                            EVENTXS( ctrl_left   ),
                            EVENTXS( shift_right ),
                            EVENTXS( shift_left  ),
                        };
                    };
                    SUBGROUPXS( state )
                    {
                        any = _,
                        GROUPXS( on  ),
                        GROUPXS( off ),

                        SUBGROUPXS( on )
                        {
                            any = _,
                            EVENTXS( numlock    ),
                            EVENTXS( capslock   ),
                            EVENTXS( scrolllock ),
                            EVENTXS( insert     ),
                        };
                        SUBGROUPXS( off )
                        {
                            any = _,
                            EVENTXS( numlock    ),
                            EVENTXS( capslock   ),
                            EVENTXS( scrolllock ),
                            EVENTXS( insert     ),
                        };
                    };
                };
                SUBGROUPXS( mouse )
                {
                    any = _,
                    GROUPXS( button  ),
                    GROUPXS( scroll  ),
                    EVENTXS( move    ),
                    EVENTXS( shuffle ), // movement within one cell
                    EVENTXS( focus   ),
                    EVENTXS( gone    ), // release::global: Notify about the mouse controller is gone (args: hids).

                    SUBGROUPXS( scroll )
                    {
                        any = _,
                        EVENTXS( up   ),
                        EVENTXS( down ),
                    };
                    SUBGROUPXS( button )
                    {
                        any = _,
                        GROUPXS( up       ),
                        GROUPXS( down     ),
                        GROUPXS( click    ),
                        GROUPXS( dblclick ),
                        GROUPXS( drag     ),

                        SUBGROUPXS( up )
                        {
                            any = _,
                            EVENTXS( left      ),
                            EVENTXS( right     ),
                            EVENTXS( leftright ),
                            EVENTXS( middle    ),
                            EVENTXS( wheel     ),
                            EVENTXS( win       ),
                        };
                        SUBGROUPXS( down )
                        {
                            any = _,
                            EVENTXS( left      ),
                            EVENTXS( right     ),
                            EVENTXS( leftright ),
                            EVENTXS( middle    ),
                            EVENTXS( wheel     ),
                            EVENTXS( win       ),
                        };
                        SUBGROUPXS( click )
                        {
                            any = _,
                            EVENTXS( left      ),
                            EVENTXS( right     ),
                            EVENTXS( leftright ),
                            EVENTXS( middle    ),
                            EVENTXS( wheel     ),
                            EVENTXS( win       ),
                        };
                        SUBGROUPXS( dblclick )
                        {
                            any = _,
                            EVENTXS( left      ),
                            EVENTXS( right     ),
                            EVENTXS( leftright ),
                            EVENTXS( middle    ),
                            EVENTXS( wheel     ),
                            EVENTXS( win       ),
                        };
                        SUBGROUPXS( drag )
                        {
                            any = _,
                            GROUPXS( start  ),
                            GROUPXS( pull   ),
                            GROUPXS( cancel ),
                            GROUPXS( stop   ),

                            SUBGROUPXS( start )
                            {
                                any = _,
                                EVENTXS( left      ),
                                EVENTXS( right     ),
                                EVENTXS( leftright ),
                                EVENTXS( middle    ),
                                EVENTXS( wheel     ),
                                EVENTXS( win       ),
                            };
                            SUBGROUPXS( pull )
                            {
                                any = _,
                                EVENTXS( left      ),
                                EVENTXS( right     ),
                                EVENTXS( leftright ),
                                EVENTXS( middle    ),
                                EVENTXS( wheel     ),
                                EVENTXS( win       ),
                            };
                            SUBGROUPXS( cancel )
                            {
                                any = _,
                                EVENTXS( left      ),
                                EVENTXS( right     ),
                                EVENTXS( leftright ),
                                EVENTXS( middle    ),
                                EVENTXS( wheel     ),
                                EVENTXS( win       ),
                            };
                            SUBGROUPXS( stop )
                            {
                                any = _,
                                EVENTXS( left      ),
                                EVENTXS( right     ),
                                EVENTXS( leftright ),
                                EVENTXS( middle    ),
                                EVENTXS( wheel     ),
                                EVENTXS( win       ),
                            };
                        };
                    };
                };
            };

            SUBGROUPXS( form )
            {
                any = _,
                GROUPXS( draggable ), // signal to the form to enable draggablity for specified mouse button (arg: bool)
                GROUPXS( layout    ),
                GROUPXS( highlight ),
                GROUPXS( upon      ),
                GROUPXS( proceed   ),
                GROUPXS( cursor    ),
                GROUPXS( animate   ),
                GROUPXS( drag      ),
                GROUPXS( prop      ),
                GROUPXS( global    ),
                GROUPXS( state     ),
                GROUPXS( upevent   ), // eventss streamed up (to children) of the visual tree by base::
                GROUPXS( notify    ), // Form events that should be propagated down to the visual branch
                EVENTXS( canvas    ), // request global canvas (arg: sptr<core>)
                //EVENTXS( key       ),

                SUBGROUPXS( draggable )
                {
                    any = _,
                    EVENTXS( left      ),
                    EVENTXS( right     ),
                    EVENTXS( leftright ),
                    EVENTXS( middle    ),
                    EVENTXS( wheel     ),
                    EVENTXS( win       ),
                };
                SUBGROUPXS( layout )
                {
                    any = _,
                    EVENTXS( shift        ), // request a global shifting  with delta (const twod)
                    EVENTXS( convey       ), // request a global conveying with delta (Inform all children to be conveyed) (arg: cube)
                    EVENTXS( order        ), // return
                    EVENTXS( local        ), // Recursively calculate local coordinate from global (arg: twod)
                    EVENTXS( strike       ), // (always preview) inform about the child canvas has changed (arg: modified region rect)
                    EVENTXS( bubble       ), // order to popup the requested item through the visual tree (arg: form)
                    EVENTXS( expose       ), // order to bring the requested item on top of the visual tree (release: ask parent to expose specified child; preview: ask child to expose itself) (arg: base)
                    EVENTXS( appear       ), // fly tothe specified coords, arg: twod
                    //EVENTXS( coor       ), // return client rect coor (preview: subject to change)
                    //EVENTXS( size       ), // return client rect size (preview: subject to change)
                    //EVENTXS( rect       ), // return client rect (preview: subject to change)
                    //EVENTXS( show       ), // order to make it visible (arg: bool notify or not)
                    //EVENTXS( hide       ), // order to make it hidden (arg: bool notify or not)
                    //EVENTXS( next       ), // request client for next child object (arg is only request: sptr<base>)
                    //EVENTXS( prev       ), // request client for prev child object (arg is only request: sptr<base>)
                    //EVENTXS( footer     ), // notify the client has changed footer (arg is only release: const rich)
                    //EVENTXS( clientrect ), // notify the client area has changed (arg is only release: rect)
                };
                SUBGROUPXS( highlight )
                {
                    any = _,
                    EVENTXS( on  ),
                    EVENTXS( off ),
                };
                //SUBGROUPXS( focus )
                //{
                //    any = _,
                //    EVENTXS( got  ), // notify that keybd focus has taken (release: hids)
                //    EVENTXS( lost ), // notify that keybd focus got lost  (release: hids)
                //};
                SUBGROUPXS( upon )
                {
                    any = _,
                    GROUPXS( vtree       ), // visual tree events (arg: parent base_sptr)
                    GROUPXS( scroll      ), // event after scroll (arg: rack)
                    EVENTXS( redrawn     ), // inform about camvas is completely redrawn (arg: canvas face)
                    EVENTXS( cached      ), // inform about camvas is cached (arg: canvas face)
                    EVENTXS( wiped       ), // event after wipe the canvas (arg: canvas face)
                    EVENTXS( created     ), // event after itself creation (arg: itself bell_sptr)
                    EVENTXS( changed     ), // event after resize (arg: diff bw old and new size twod)
                    EVENTXS( dragged     ), // event after drag (arg: hids)
                    //EVENTXS( detached    ), // inform that subject is detached (arg: parent bell_sptr)
                    //EVENTXS( invalidated ), 
                    //EVENTXS( moved       ), // release: event after moveto (arg: diff bw old and new coor twod). preview: event after moved by somebody.

                    SUBGROUPXS( vtree )
                    {
                        any = _,
                        EVENTXS( attached ), // Child has been attached (arg: parent sptr<base>)
                        EVENTXS( detached ), // Child has been detached (arg: parent sptr<base>)
                    };
                    SUBGROUPXS( scroll )
                    {
                        any = _,
                        EVENTXS( x      ), // event after scroll along X (arg: rack)
                        EVENTXS( y      ), // event after scroll along Y (arg: rack)
                        EVENTXS( resetx ), // event reset scroll along X (arg: rack)
                        EVENTXS( resety ), // event reset scroll along Y (arg: rack)
                    };
                };
                SUBGROUPXS( proceed )
                {
                    any = _,
                    EVENTXS( create      ), // return coordinates of the new object placeholder (arg: rect)
                    EVENTXS( createby    ), // return gear with coordinates of the new object placeholder gear::slot (arg: gear)
                    EVENTXS( destroy     ), // ??? bool return reference to the parent
                    EVENTXS( render      ), // ask children to render itself to the parent canvas (arg: function drawfx to perform drawing)
                    EVENTXS( attach      ), // order to attach a child (arg: parent base_sptr)
                    EVENTXS( detach      ), // order to detach a child (e2::release - kill itself, e2::preview - detach the child specified in args) (arg: child  base_sptr)
                    //EVENTXS( commit      ), // order to output the targets (arg: frame number iota)
                    //EVENTXS( multirender ), // ask children to render itself to the set of canvases (arg: array of the face sptrs: cuts = vector<shared_ptr<face>>)
                    //EVENTXS( draw        ), // ????  order to render itself to the canvas (arg: canvas face)
                    //EVENTXS( checkin     ), // order to register an output client canvas (arg: face_sptr)
                };
                SUBGROUPXS( cursor )
                {
                    any = _,
                    EVENTXS(blink),
                };
                SUBGROUPXS( animate )
                {
                    any = _,
                    EVENTXS( start ),
                    EVENTXS( stop  ),
                };
                SUBGROUPXS( drag )
                {
                    any = _,
                    GROUPXS( start  ), // notify about mouse drag start by pro::mouse (arg: hids)
                    GROUPXS( pull   ), // notify about mouse drag pull by pro::mouse (arg: hids)
                    GROUPXS( cancel ), // notify about mouse drag cancel by pro::mouse (arg: hids)
                    GROUPXS( stop   ), // notify about mouse drag stop by pro::mouse (arg: hids)

                    SUBGROUPXS( start )
                    {
                        any = _,
                        EVENTXS( left      ),
                        EVENTXS( right     ),
                        EVENTXS( leftright ),
                        EVENTXS( middle    ),
                        EVENTXS( wheel     ),
                        EVENTXS( win       ),
                    };
                    SUBGROUPXS( pull )
                    {
                        any = _,
                        EVENTXS( left      ),
                        EVENTXS( right     ),
                        EVENTXS( leftright ),
                        EVENTXS( middle    ),
                        EVENTXS( wheel     ),
                        EVENTXS( win       ),
                    };
                    SUBGROUPXS( cancel )
                    {
                        any = _,
                        EVENTXS( left      ),
                        EVENTXS( right     ),
                        EVENTXS( leftright ),
                        EVENTXS( middle    ),
                        EVENTXS( wheel     ),
                        EVENTXS( win       ),
                    };
                    SUBGROUPXS( stop )
                    {
                        any = _,
                        EVENTXS( left      ),
                        EVENTXS( right     ),
                        EVENTXS( leftright ),
                        EVENTXS( middle    ),
                        EVENTXS( wheel     ),
                        EVENTXS( win       ),
                    };
                };
                SUBGROUPXS( prop )
                {
                    any = _,
                    EVENTXS( header     ), // set form caption header (arg: text)
                    EVENTXS( footer     ), // set form caption footer (arg: text)
                    EVENTXS( zorder     ), // set form z-order (arg: iota: -1 backmost, 0 plain, 1 topmost)
                    EVENTXS( brush      ), // set form brush/color (arg: cell)
                    EVENTXS( fullscreen ), // set fullscreen flag (arg: bool)
                    EVENTXS( name       ), // user name (arg: text)
                    EVENTXS( viewport   ), // request: return form actual viewport (arg: rect)
                };
                SUBGROUPXS( global )
                {
                    any = _,
                    GROUPXS( object   ), // global scene objects events
                    GROUPXS( user     ), // global scene users events
                    EVENTXS( ctxmenu  ), // request context menu at specified coords (arg: twod)
                    EVENTXS( prev     ), // request the prev scene window (arg: twod)
                    EVENTXS( next     ), // request the next scene window (arg: twod)
                    EVENTXS( lucidity ), // set or request global window transparency (arg: iota: 0-255, -1 to request)

                    SUBGROUPXS( object )
                    {
                        any = _,
                        EVENTXS( attached ), // global: object attached to the world (args: sptr<base>)
                        EVENTXS( detached ), // global: object detached from the world (args: sptr<base>)
                    };
                    SUBGROUPXS( user )
                    {
                        any = _,
                        EVENTXS( attached ), // global: user attached to the world (args: sptr<base>)
                        EVENTXS( detached ), // global: user detached from the world (args: sptr<base>)
                    };
                };
                SUBGROUPXS( state )
                {
                    any = _,
                    EVENTXS( mouse  ), // notify the client is mouse active or not. The form is active when the number of client (form::eventa::mouse::enter - mouse::leave) is not zero. (arg is only release: iota - number of clients)
                    EVENTXS( keybd  ), // notify the client is keybd active or not. The form is active when the number of client (form::eventa::keybd::got - keybd::lost) is not zero. (arg is only release: bool)
                    EVENTXS( header ), // notify the client has changed title  (arg: para)
                    EVENTXS( footer ), // notify the client has changed footer (arg: para)
                    EVENTXS( params ), // notify the client has changed title params (arg: para)
                    EVENTXS( color  ), // notify the client has changed tone (preview to set, arg: tone)
                };
                SUBGROUPXS( upevent )
                {
                    any = _,
                    EVENTXS( kboffer ), // inform nested objects that the keybd focus should be taken (arg: hids)
                };
                SUBGROUPXS( notify )
                {
                    any = _,
                    GROUPXS( mouse ), // request context menu at specified coords (arg: twod)
                    GROUPXS( keybd ), // request the prev scene window (arg: twod)

                    SUBGROUPXS( mouse )
                    {
                        any = _,        // inform the form about the mouse hover (arg: hids)
                        EVENTXS( enter ), // inform the form about the mouse hover (arg: hids)
                        EVENTXS( leave ), // inform the form about the mouse leave (arg: hids)
                    };
                    SUBGROUPXS( keybd )
                    {
                        any = _,
                        EVENTXS( got  ),
                        EVENTXS( lost ),
                    };
                };
            };
        };
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