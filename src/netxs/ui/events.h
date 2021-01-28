// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_EVENTS_H
#define NETXS_EVENTS_H

// Description: Compile time typesafe hierarchical delegates.

#include "../abstract/ptr.h"
#include "../abstract/hash.h"

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

namespace netxs::ui
{
    struct e2
    {
        using type = unsigned int;

    private: static const unsigned int _width = 4;
    private: static const unsigned int _mask = (1 << _width) - 1;
        template<class V>
        struct _globals
        {
            static std::recursive_mutex mutex; // e2: shared mutex
        };

    public:
        struct sync
        {
            std::lock_guard<std::recursive_mutex> lock;

            sync            (sync const&) = delete; // deleted copy constructor
            sync& operator= (sync const&) = delete; // deleted copy assignment operator

             sync() : lock(_globals<void>::mutex) { }
            ~sync() { }
        };
        struct try_sync
        {
            std::unique_lock<std::recursive_mutex> lock;

            try_sync            (try_sync const&) = delete; // deleted copy constructor
            try_sync& operator= (try_sync const&) = delete; // deleted copy assignment operator

            operator bool() { return lock.owns_lock(); }

            try_sync() : lock(_globals<void>::mutex, std::try_to_lock) {}
            ~try_sync() {}
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
            /// <summary>
            /// Forward means from particular to general: 1. event_group::item, 2. event_group::any
            /// Reverse means from general to particular: 1. event_group::any , 2. event_group::item
            /// </summary>
            release, // event: Run forwrad handlers with fixed param. Preserve subscription order.
            preview, // event: Run reverse handlers with fixed a param intended to change. Preserve subscription order.
            general, // event: Run forwrad handlers for all objects. Preserve subscription order.
            request, // event: Run forwrad a handler that provides the current value of the param. To avoid overriding, the handler should be the only one. Preserve subscription order.
        };
        
        // e2 (static): Return item/msg level by its ID.
        constexpr static unsigned int level(type msg)
        {
            unsigned int level = 0;
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
        constexpr static type subgroup(type msg, type& itermask)
        {
            itermask = (itermask << _width) + _mask;
            return msg & itermask;
        }
        constexpr static type subgroup_fwd(type msg, type& itermask)
        {
            auto result = msg & itermask;
            itermask = (itermask >> _width);
            return result;
        }
        //constexpr static type subgroup_fwd(type msg, type& level_offset)
        //{
        //    level_offset += _width;
        //    return msg & ((1 << level_offset) - 1);
        //}
        // e2 (static): Return event's group ID.
        constexpr static type parent(type msg)
        {
            return msg & ((1 << (level(msg) * _width)) - 1);
        }
        // e2 (static): Return the event ID of the specified item inside the group.
        constexpr static const type message(type base, type item)
        {
            return base | ((item + 1) << (level(base) + 1) * _width);
        }
        // e2 (static): Return item index inside the group by its ID.
        constexpr static unsigned int item(type msg)
        {
            return (msg >> (level(msg) * _width)) - 1;
        }
    private:
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

    public:
        enum : type {
            any = 0,
            _term       = any | (1 << 0),
            _form       = any | (2 << 0),
            _data       = any | (3 << 0),
            _timer      = any | (4 << 0),
            _hids       = any | (5 << 0),
            //_prop       = any | (6 << 0),
            //_rect       = any | (6 << 0), // rectangle modification event group
            _debug      = any | (6 << 0), // return info struct with telemtry data
            _config     = any | (7 << 0), // set/notify/get/global_set configuration data (e2::preview/e2::release/e2::request/e2::general)
            quit        = any | (8 << 0), // return bye msg //errcode (arg: const view)
            dtor        = any | (9 << 0), // Notify about object destruction (arg: const id_t)
            
            //todo unify
            radio       = any | (10<< 0), // return active radio id_t (arg: const id_t)
            cout        = any | (11<< 0), // Append extra data to output (arg: const text)
        };
        //private: static const unsigned int _level = _toplevel + _width;
        private: static const unsigned int _level0 = _width;
        public:
        struct debug { enum : type {
                any = e2::_debug,
                logs        = any | (1 << _level0), // logs output (arg: const text)
                output      = any | (2 << _level0), // logs has to be parsed (arg: const view)
                parsed      = any | (3 << _level0), // output parced logs (arg: const page)
        };};
        struct timer { enum : type {
                any = e2::_timer,
                tick        = any | (1 << _level0), // timer tick (arg: current moment (now))
                fps         = any | (2 << _level0), // request to set new fps (arg: new fps (iota); the value == -1 is used to request current fps)
        };};
        struct config { enum : type {
                any = e2::_config,
                _intervals  = any | (1 << _level0), // any kind of intervals property (arg: period)
                //moveto      = any | (2 << _level0), //
                //resized     = any | (3 << _level0), //
            };
            private: static const unsigned int _level1 = _level0 + _width;
            public:
            struct intervals { enum : type {
                    any = config::_intervals,
                    blink       = any | (1 << _level1), // caret blinking interval (arg: period)
                    //up          = any | (2 << _level1),
            };};
        };
        struct term { enum : type {
                any = e2::_term,
                unknown     = any | (1 << _level0),	// return platform unknown event code
                error       = any | (2 << _level0),	// return error code
                focus       = any | (3 << _level0), // order to change focus (arg: bool)
                key         = any | (4 << _level0), // keybd activity (arg: syskeybd)
                //menu        = any | (5 << _level0),
                mouse       = any | (6 << _level0), // mouse activity (arg: sysmouse)
                size        = any | (7 << _level0), // order to update terminal primary overlay (arg: newsize twod)
                layout      = any | (8 << _level0),
                preclose    = any | (9 << _level0), // signal to quit after idle timeout (arg: bool - ready to shutdown)
                quit        = any | (10<< _level0), // quit (arg: text - bye msg)
        };};
        struct hids { enum : type {
                any = e2::_hids,
                _mouse      = any | (1 << _level0),
                _keybd      = any | (2 << _level0),
                //mouse_data  = any | (3 << _level),
            };
            private: static const unsigned int _level1 = _level0 + _width;
            public:
            struct keybd { enum : type {
                    any = hids::_keybd,
                    down        = any | (1 << _level1),
                    up          = any | (2 << _level1),
                    _control    = any | (3 << _level1),
                    _state      = any | (4 << _level1),
                };
                private: static const unsigned int _level2 = _level1 + _width;
                public:
                struct control { enum : type {
                        any = keybd::_control,
                        _down       = any | (1 << _level2),
                        _up         = any | (2 << _level2),
                    };
                private: static const unsigned int _level3 = _level2 + _width;
                public:
                    struct down { enum : type {
                            any = control::_down,
                            alt_right       = any | (1 << _level3),
                            alt_left        = any | (2 << _level3),
                            ctrl_right      = any | (3 << _level3),
                            ctrl_left       = any | (4 << _level3),
                            shift_right     = any | (5 << _level3),
                            shift_left      = any | (6 << _level3),
                    };};
                    struct up { enum : type {
                            any = control::_up,
                            alt_right       = any | (1 << _level3),
                            alt_left        = any | (2 << _level3),
                            ctrl_right      = any | (3 << _level3),
                            ctrl_left       = any | (4 << _level3),
                            shift_right     = any | (5 << _level3),
                            shift_left      = any | (6 << _level3),
                    };};
                };
                struct state { enum : type {
                        any = keybd::_state,
                        _on         = any | (1 << _level2),
                        _off        = any | (2 << _level2),
                    };
                private: static const unsigned int _level3 = _level2 + _width;
                public:
                    struct on { enum : type {
                            any = state::_on,
                            numlock         = any | (1 << _level3),
                            capslock        = any | (2 << _level3),
                            scrolllock      = any | (3 << _level3),
                            insert          = any | (4 << _level3),
                    };};
                    struct off { enum : type {
                            any = state::_off,
                            numlock         = any | (1 << _level3),
                            capslock        = any | (2 << _level3),
                            scrolllock      = any | (3 << _level3),
                            insert          = any | (4 << _level3),
                    };};
                };};
            struct mouse { enum : type {
                    any = hids::_mouse,
                    _button     = any | (1 << _level1),
                    move        = any | (2 << _level1),
                    shuffle     = any | (3 << _level1), // movement within one cell
                    _scroll     = any | (4 << _level1),
                    focus       = any | (5 << _level1),
                    hover       = any | (6 << _level1),
                };
                private: static const unsigned int _level2 = _level1 + _width;
                public:
                struct scroll { enum : type {
                        any = mouse::_scroll,
                        up          = any | (1 << _level2),
                        down        = any | (2 << _level2),
                };};
                struct button { enum : type {
                        any = mouse::_button,
                        _up         = any | (1 << _level2),
                        _down       = any | (2 << _level2),
                        _click      = any | (3 << _level2),
                        _dblclick   = any | (4 << _level2),
                        _drag       = any | (5 << _level2),
                    };
                    private: static const unsigned int _level3 = _level2 + _width;
                    public:
                    struct up { enum : type {
                            any = button::_up,
                            left        = any | (1 << _level3),
                            right       = any | (2 << _level3),
                            leftright   = any | (3 << _level3),
                            middle      = any | (4 << _level3),
                            wheel       = any | (5 << _level3),
                            win         = any | (6 << _level3),
                    };};
                    struct down { enum : type {
                            any = button::_down,
                            left        = any | (1 << _level3),
                            right       = any | (2 << _level3),
                            leftright   = any | (3 << _level3),
                            middle      = any | (4 << _level3),
                            wheel       = any | (5 << _level3),
                            win         = any | (6 << _level3),
                    };};
                    struct click { enum : type {
                            any = button::_click,
                            left        = any | (1 << _level3),
                            right       = any | (2 << _level3),
                            leftright   = any | (3 << _level3),
                            middle      = any | (4 << _level3),
                            wheel       = any | (5 << _level3),
                            win         = any | (6 << _level3),
                    };};
                    struct dblclick { enum : type {
                            any = button::_dblclick,
                            left        = any | (1 << _level3),
                            right       = any | (2 << _level3),
                            leftright   = any | (3 << _level3),
                            middle      = any | (4 << _level3),
                            wheel       = any | (5 << _level3),
                            win         = any | (6 << _level3),
                    };};
                    struct drag { enum : type {
                            any = button::_drag,
                            _start      = any | (1 << _level3),
                            _pull       = any | (2 << _level3),
                            _cancel     = any | (3 << _level3),
                            _stop       = any | (4 << _level3),
                        };
                        private: static const unsigned int _level4 = _level3 + _width;
                        public:
                            struct start { enum : type {
                                    any = drag::_start,
                                    left        = any | (1 << _level4),
                                    right       = any | (2 << _level4),
                                    leftright   = any | (3 << _level4),
                                    middle      = any | (4 << _level4),
                                    wheel       = any | (5 << _level4),
                                    win         = any | (6 << _level4),
                            };};
                            struct pull { enum : type {
                                    any = drag::_pull,
                                    left        = any | (1 << _level4),
                                    right       = any | (2 << _level4),
                                    leftright   = any | (3 << _level4),
                                    middle      = any | (4 << _level4),
                                    wheel       = any | (5 << _level4),
                                    win         = any | (6 << _level4),
                            };};
                            struct cancel { enum : type {
                                    any = drag::_cancel,
                                    left        = any | (1 << _level4),
                                    right       = any | (2 << _level4),
                                    leftright   = any | (3 << _level4),
                                    middle      = any | (4 << _level4),
                                    wheel       = any | (5 << _level4),
                                    win         = any | (6 << _level4),
                            };};
                            struct stop { enum : type {
                                    any = drag::_stop,
                                    left        = any | (1 << _level4),
                                    right       = any | (2 << _level4),
                                    leftright   = any | (3 << _level4),
                                    middle      = any | (4 << _level4),
                                    wheel       = any | (5 << _level4),
                                    win         = any | (6 << _level4),
                            };};
                    };
                };
            };
        };
        struct form { enum : type {
                any = e2::_form,
                //_focus      = any | (1 << _level0),
                _layout     = any | (2 << _level0),
                _highlight  = any | (3 << _level0),
                _upon       = any | (4 << _level0),
                _proceed    = any | (5 << _level0),
                //key         = any | (6 << _level0),
                _cursor     = any | (7 << _level0),
                _animate    = any | (8 << _level0),
                //_mouse      = any | (9 << _level0),
                _prop       = any | (10<< _level0),
                //_client     = any | (11<< _level0),
                _upevent    = any | (11<< _level0), // eventss streamed up (to children) of the visual tree by base::
                _global     = any | (12<< _level0),
                _state      = any | (13<< _level0),
                canvas      = any | (14<< _level0), // request global canvas (arg: sptr<core>)
                _notify     = any | (15<< _level0), // request global canvas (arg: sptr<core>)
            };
            private: static const unsigned int _level1 = _level0 + _width;
            public: 
            // Form events that should be propagated down to the visual branch
            struct notify { enum : type {
                    any = form::_notify,
                    _mouse      = any | (1 << _level1), // request context menu at specified coords (arg: twod)
                    _keybd      = any | (2 << _level1), // request the prev scene window (arg: twod)
            };
            private: static const unsigned int _level2 = _level1 + _width;
            public:
                struct mouse { enum : type {
                        any = notify::_mouse,
                        enter       = any | (1 << _level2), // inform the form about the mouse hover (arg: hids)
                        leave       = any | (2 << _level2), // inform the form about the mouse leave (arg: hids)
                };};
                struct keybd { enum : type {
                        any = notify::_keybd,
                        got         = any | (1 << _level2),
                        lost        = any | (2 << _level2),
                };};
            };
            struct global { enum : type {
                    any = form::_global,
                    ctxmenu     = any | (1 << _level1), // request context menu at specified coords (arg: twod)
                    prev        = any | (2 << _level1), // request the prev scene window (arg: twod)
                    next        = any | (3 << _level1), // request the next scene window (arg: twod)
                    lucidity    = any | (4 << _level1), // set or request global window transparency (arg: iota: 0-255, -1 to request)
            };};
            struct upevent { enum : type {
                    any = form::_upevent,
                    kboffer     = any | (1 << _level1), // inform nested objects that the keybd focus should be taken (arg: hids)
            };};
            //struct client { enum : type {
            //		any = form::_client,
            //		rect		= any | (1 << _level1), // notify the client area has changed (arg is only release: rect)
            //		size		= any | (2 << _level1), // notify the client size has changed (arg is only release: twod)
            //		coor		= any | (3 << _level1), // notify the client coor has changed (arg is only release: twod)
            //		align		= any | (4 << _level1), // 
            //};};

            struct prop { enum : type {
                    any = form::_prop,
                    header      = any | (1 << _level1), // set the form caption header (arg: text)
                    footer      = any | (2 << _level1), // set the form caption footer (arg: text)
                    params      = any | (3 << _level1), // set the form caption params (arg: text)
            };};
            //struct mouse { enum : type {
            //		any = form::_mouse,
            //		enter		= any | (1 << _level1), // inform the form about the mouse hover (arg: hids)
            //		leave		= any | (2 << _level1), // inform the form about the mouse leave (arg: hids)
            //		//capture		= any | (3 << _level), // seize mouse button events flow if positive, and release if negative
            //		//release		= any | (4 << _level), // release a hook of the mouse events visual tree branch, -1 to release all
            //};};
            struct animate { enum : type {
                    any = form::_animate,
                    start       = any | (1 << _level1),
                    stop        = any | (2 << _level1),
            };};
            struct cursor { enum : type {
                    any = form::_cursor,
                    blink       = any | (1 << _level1),
            };};
            struct upon { enum : type {
                    any = form::_upon,
                    attached    = any | (1 << _level1), // inform that subject is attached (arg: parent bell_sptr)
                    detached    = any | (2 << _level1), // inform that subject is detached (arg: parent bell_sptr)
                    redrawn     = any | (3 << _level1), // inform about camvas is completely redrawn (arg: canvas face)
                    invalidated = any | (4 << _level1),
                    cached      = any | (5 << _level1), // inform about camvas is cached (arg: canvas face)
                    wiped       = any | (6 << _level1), // event after wipe the canvas (arg: canvas face)
                    created     = any | (7 << _level1), // event after itself creation (arg: itself bell_sptr)
                    moved       = any | (8 << _level1), // event after moveto (arg: diff bw old and new coor twod)
                    resized     = any | (9 << _level1), // event after resize (arg: diff bw old and new size twod)
                    _scroll     = any | (10<< _level1), // event after scroll (arg: rack)
            };
            private: static const unsigned int _level2 = _level1 + _width;
            public:
                struct scroll { enum : type {
                        any = upon::_scroll,
                        x           = any | (1 << _level2), // event after scroll along X (arg: rack)
                        y           = any | (2 << _level2), // event after scroll along Y (arg: rack)
                        resetx      = any | (3 << _level2), // event reset scroll along X (arg: rack)
                        resety      = any | (4 << _level2), // event reset scroll along Y (arg: rack)
                };};
            };
            struct proceed { enum : type {
                    any = form::_proceed,
                    create      = any | (1 << _level1), // return coordinates of the new object placeholder (arg: rect)
                    createby    = any | (2 << _level1), // return gear with coordinates of the new object placeholder gear::slot (arg: gear)
                    destroy     = any | (3 << _level1), // ??? bool return reference to the parent
                    render      = any | (4 << _level1), // ask children to render itself to the parent canvas (arg: function drawfx to perform drawing)
                    attach      = any | (5 << _level1), // order to attach a child (arg: parent bell_sptr)
                    detach      = any | (6 << _level1), // order to detach a child (arg: child bell_sptr)
                    //commit      = any | (3 << _level1), // order to output the targets (arg: frame number iota)
                    //multirender = any | (5 << _level1), // ask children to render itself to the set of canvases (arg: array of the face sptrs: cuts = vector<shared_ptr<face>>)
                    //draw        = any | (6 << _level1), // ????  order to render itself to the canvas (arg: canvas face)
                    //checkin     = any | (9 << _level1), // order to register an output client canvas (arg: face_sptr)
            };};
            struct highlight { enum : type {
                    any = form::_highlight,
                    on          = any | (1 << _level1),
                    off         = any | (2 << _level1),
            };};
            //struct focus { enum : type {
            //		any = form::_focus,
            //		got			= any | (1 << _level1), // notify that keybd focus has taken (release: hids)
            //		lost		= any | (2 << _level1), // notify that keybd focus got lost  (release: hids)
            //};};
            struct layout { enum : type {
                    any = form::_layout,
                    move        = any | (1 << _level1), // return client rect coor (preview: subject to change)
                    size        = any | (2 << _level1), // return client rect size (preview: subject to change)
                    //rect        = any | (3 << _level1), // return client rect (preview: subject to change)
                    show        = any | (4 << _level1), // order to make it visible (arg: bool notify or not)
                    hide        = any | (5 << _level1), // order to make it hidden (arg: bool notify or not)
                    shift       = any | (6 << _level1), // request a global shifting  with delta (const twod)
                    convey      = any | (7 << _level1), // request a global conveying with delta (Inform all children to be conveyed) (arg: cube)
                    order       = any | (8 << _level1), // return
                    local       = any | (9 << _level1), // Recursively calculate local coordinate from global (arg: twod)
                    strike      = any | (10<< _level1), // (always preview) inform about the child canvas has changed (arg: modified region rect)
                    bubble      = any | (11<< _level1), // order to popup the requested item through the visual tree (arg: form)
                    expose      = any | (12<< _level1), // order to bring the requested item on top of the visual tree (arg: form)
                    // active     = any | (12<< _level1), // notify the client is active or not. The form is active when the number of client (form::mouse::enter - mouse::leave) is not zero. (arg is only release: bool)
                    // header     = any | (13<< _level1), // notify the client has changed title (arg is only release: const rich)
                    // footer     = any | (14<< _level1), // notify the client has changed footer (arg is only release: const rich)
                    appear      = any | (15<< _level1), // fly tothe specified coords, arg: twod
                    //clientrect  = any | (15<< _level1), // notify the client area has changed (arg is only release: rect)
            };};
            struct state { enum : type {
                    any = form::_state,
                    mouse       = any | (1 << _level1), // notify the client is mouse active or not. The form is active when the number of client (form::eventa::mouse::enter - mouse::leave) is not zero. (arg is only release: bool)
                    keybd       = any | (2 << _level1), // notify the client is keybd active or not. The form is active when the number of client (form::eventa::keybd::got - keybd::lost) is not zero. (arg is only release: bool)
                    header      = any | (3 << _level1), // notify the client has changed title  (arg: para)
                    footer      = any | (4 << _level1), // notify the client has changed footer (arg: para)
                    params      = any | (5 << _level1), // notify the client has changed title params (arg: para)
                    color       = any | (6 << _level1), // notify the client has changed tone (preview to set, arg: tone)
            };};
        };
        struct data { enum : type {
                any = e2::_data,                    // return digest
                changed     = any | (1 << _level0), // return digest
                request     = any | (2 << _level0),
                disable     = any | (3 << _level0),
                flush       = any | (4 << _level0),
        };};
    };
    
    template<class V>
    std::recursive_mutex e2::_globals<V>::mutex; // e2: shared mutex.

    struct reactor
    {
        struct handler
        {
            virtual ~handler(){}
        };

        template <typename F>
        using hndl = std::function<void(F&)>;
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
            {}
        };

        enum exec
        {
            forward, // Execute concrete event first
            reverse, // Execute global events first
        };

        std::map<hint, list> stock; // reactor: handlers repository
        std::vector<hint>    queue; // reactor: event queue
        vect                 qcopy; // reactor: copy of the current pretenders to exec on current event
        bool                 alive; // reactor: current exec branch interruptor
        exec                 order; // reactor: Execution oreder

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
                                    return false;
                                }
                            });
        }
        // reactor: Thread-safe invoke an event handler.
        //          Return number of active handlers.
        template<class F>
        auto notify(hint e, F& args)
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
            //if (head != tail)
            if (size)
            {
                auto perform = [&](auto iter)
                {
                    if (auto proc_ptr = qcopy[iter].lock())
                    {
                        if (auto compatible = dynamic_cast<wrapper<F>*>(proc_ptr.get()))
                        {
                            compatible->proc(args);
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
        auto operator()(hint e, F& args)
        {
            return notify(e, args);
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
        using wptr = netxs::wptr<T>;
        using imap = std::map<id_t, wptr>;
        const id_t id;

        static wptr empty;
        static id_t newid;
        static imap store;

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
        void _actuate(wptr This)
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
        static auto create(Args... args)
        {
            struct make_shared_enabler : public TT
            {
                make_shared_enabler(Args... args)
                    : TT{ args... }
                { }
            };

            e2::sync lock;
            sptr<TT> inst = std::make_shared<make_shared_enabler>(args...);
            inst->_actuate(inst);

            //todo move to the bell
            sptr<T> item = inst;
            inst->T::signal_direct(e2::release, e2::form::upon::created, item);
            //item->T::signal<e2::release>(e2::form::upon::created, item);
            return inst;
        }
        //todo smell
        //  indexer: Wait until the object is released by other threads and destroy it afterwards in this thread.
        //template<class TT, class ...Args>
        //static void destroy(sptr<TT>& item)
        //{
        //	if (item.use_count())
        //	{
        //		std::cout << "client active copies: " << item.use_count() << "\n" << std::flush;
        //		while (item.use_count() > 1)
        //		{
        //			std::this_thread::yield();
        //		}
        //		std::cout << "client going to destroy in " << std::this_thread::get_id() << "\n" << std::flush;
        //		item.reset();
        //		std::cout << "client destroyed in " << std::this_thread::get_id() << "\n" << std::flush;
        //	}
        //}
    };

    // utils::ui: Ext link statics, unique ONLY for concrete T.
    template<class T> typename indexer<T>::id_t indexer<T>::newid = 0;
    template<class T> typename indexer<T>::imap indexer<T>::store;
    template<class T> typename indexer<T>::wptr indexer<T>::empty;

    // utils::ui: Event x-mitter.
    struct bell : public indexer<bell>
    {
        using hook = reactor::hook;

    private:
        template<class V>
        struct _globals
        {
            static reactor general; // bell: Ext link static.
        };
        reactor& general;                   // bell: Global  events node relay.
        reactor  preview{reactor::reverse}; // bell: Preview events node relay.
        reactor  request{reactor::forward}; // bell: Request events node relay.
        reactor  release{reactor::forward}; // bell: Release events node relay.

        //todo simplify the request reactor. remove the map.
        struct
        {
            std::vector<hook> tokens;

            void operator()(hook& t)
            {
                tokens.push_back(t);
            }

            template<class REACTOR, class EVENTS, class F>
            void operator()(REACTOR& r, EVENTS e, std::function<void(F&)> h)
            {
                tokens.push_back(r.subscribe(e, h));
            }

            //template<class F, class REACTOR, class EVENTS>
            //auto& reg(REACTOR& r, EVENTS e)
            //{
            //	auto func = r.template subscribe<F>(e);
            //	tokens.push_back(func);
            //	return func->proc;
            //}
        } 
        tracker;

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
                auto handler = std::function<void(typename EVENT::param&)>{ h };
                token = _globals<void>::general.subscribe(EVENT::cause, handler);
            }
        };

    public:
        class subs
        {
            std::vector<hook> memo;

        public:
            hook& extra()       { return memo.emplace_back(); }
            auto  count() const { return memo.size();         }
            void  clear()       {        memo.clear();        }
        };

        // bell: Subscribe on a specified event 
        //       of specified reaction node by defining an event 
        //       handler. Return a lambda reference helper.
        template<class EVENT>//, class F>
        auto submit2(e2::tier level)//, F&)
        {
            return submit_helper<EVENT>(*this, level);
        }
        //  bell: Subscribe on a specified event 
        //        of specified reaction node by defining an event 
        //        handler and token. Return a lambda reference helper.
        template<class EVENT>//, class F>
        auto submit2(e2::tier level, hook& token)//, F&)
        {
            return submit_helper_token<EVENT>(*this, level, token);
        }
        template<class EVENT>//, class F>
        auto submit2(e2::tier level, subs& tokens)//, F&)
        {
            return submit_helper_token<EVENT>(*this, level, tokens.extra());
        }

        // bell: Subscribe to an specified event on specified
        //       reaction node by defining an event handler.
        template<class EVENT>
        void submit(e2::tier level, std::function<void(typename EVENT::param &)> handler)
        {
            switch (level)
            {
                case e2::tier::release:
                    tracker(release, EVENT::cause, handler);
                    break;
                case e2::tier::preview:
                    tracker(preview, EVENT::cause, handler);
                    break;
                case e2::tier::general:
                    tracker(general, EVENT::cause, handler);
                    break;
                case e2::tier::request:
                    tracker(request, EVENT::cause, handler);
                    break;
                default:
                    break;
            }
        }
        // bell: Subscribe to an specified event
        //       on specified reaction node by defining
        //       an event handler, and store the subscription
        //       in the specified token.
        template<class EVENT>
        void submit(e2::tier level, hook& token, std::function<void(typename EVENT::param &)> handler)
        {
            switch (level)
            {
                case e2::tier::release:
                    token = release.subscribe(EVENT::cause, handler);
                    break;
                case e2::tier::preview:
                    token = preview.subscribe(EVENT::cause, handler);
                    break;
                case e2::tier::general:
                    token = general.subscribe(EVENT::cause, handler);
                    break;
                case e2::tier::request:
                    token = request.subscribe(EVENT::cause, handler);
                    break;
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
            //_globals<void>::general.subscribe(EVENT::cause, handler);
            return submit_helper_token_global<EVENT>(token);
        }

        // bell: Change the global ownership of the specified
        //       event and subscribe to emit the event "gone"
        //       the next time the owner changes.
        //template<class EVENT>
        //void change(hook& token, e2::type gone, typename EVENT::param& p)
        //{
        //	if (!token)
        //	{
        //		//signal(e2::general, EVENT::cause, p);
        //		signal<e2::general>(EVENT::cause, p);
        //		submit<EVENT>(e2::general, token, [&, gone](auto d)
        //					  {
        //						  token.reset();
        //						  //signal(e2::general, gone, d);
        //						  //signal(e2::preview, gone, d);
        //						  signal<e2::general>(gone, d);
        //						  signal<e2::preview>(gone, d);
        //					  });
        //		//signal(e2::preview, EVENT::cause, p);
        //		signal<e2::preview>(EVENT::cause, p);
        //	}
        //}
        //todo used only with indexer::create
        // bell: Rise specified evench execution branch on the specified relay node.
        template<class F>
        void signal_direct(e2::tier level, e2::type action, F& data)
        {
            switch (level)
            {
                case e2::tier::release:
                    release(action, data);
                    break;
                case e2::tier::preview:
                    preview(action, data);
                    break;
                case e2::tier::general:
                    general(action, data);
                    break;
                case e2::tier::request:
                    request(action, data);
                    break;
                default:
                    break;
            }
        }
        // bell: Rise specified event execution branch on the specified relay node.
        //       Return number of active handlers.
        template<e2::tier TIER, class F>
        auto signal(e2::type action, F& data)
        {
            switch (TIER)
            {
                case e2::tier::release:
                    return release(action, data);
                case e2::tier::preview:
                    return preview(action, data);
                case e2::tier::general:
                    return general(action, data);
                case e2::tier::request:
                    return request(action, data);
                default:
                    return 0_sz;
            }
        }
        // bell: Rise specified event globally.
        template<class F>
        static auto signal_global(e2::type action, F& data)
        {
            return _globals<void>::general(action, data);
        }
        // bell: Save up external subscription token.
        void saveup(hook& token)
        {
            tracker(token);
        }
        // bell: Return an initial event of the current event execution branch.
        auto protos(e2::tier level) -> std::optional<e2::type>
        {
            switch (level)
            {
                case e2::tier::release:
                    return release.queue.empty() ? std::nullopt 
                                                 : std::optional<e2::type>{ release.queue.back() };
                case e2::tier::preview:
                    return preview.queue.empty() ? std::nullopt 
                                                 : std::optional<e2::type>{ preview.queue.back() };
                case e2::tier::general:
                    return general.queue.empty() ? std::nullopt 
                                                 : std::optional<e2::type>{ general.queue.back() };
                case e2::tier::request:
                    return request.queue.empty() ? std::nullopt 
                                                 : std::optional<e2::type>{ request.queue.back() };
                default:
                    break;
            }
            return std::nullopt;
        }
        // bell: Return true if initial event equal to the specified.
        auto protos(e2::tier level, e2::type action)
        {
            if (auto deal = bell::protos(level))
            {
                if (deal.value() == action)
                {
                    return true;
                }
            }
            return faux;
        }
        // bell: Get the reference to the specified relay node.
        reactor& router(e2::tier level)
        {
            switch (level)
            {
                case e2::tier::release:
                    return release;
                case e2::tier::preview:
                    return preview;
                case e2::tier::general:
                    return general;
                case e2::tier::request:
                    return request;
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
                case e2::tier::release:
                    release.discontinue();
                    break;
                case e2::tier::preview:
                    preview.discontinue();
                    break;
                case e2::tier::general:
                    general.discontinue();
                    break;
                case e2::tier::request:
                    request.discontinue();
                    break;
                default:
                    break;
            }
        }
        bell()
            : general{ _globals<void>::general }
        { }
        ~bell()
        {
            signal<e2::release>(e2::dtor, id);
        }
    };
    
    template<class T>
    reactor bell::_globals<T>::general{ reactor::forward };

    #define EVENT_NS     \
    template<e2::type T> \
    struct type_clue {};

    #define EVENT_BIND(event_item, param_type)        \
    template<>                                        \
    struct type_clue<event_item>                      \
    {                                                 \
        using                     param = param_type; \
        static constexpr e2::type cause = event_item; \
    };
    
    #define EVENT_SAME(event_master, event_item)                          \
    template<>                                                            \
    struct type_clue<event_item>                                          \
    {                                                                     \
        using                     param = type_clue<event_master>::param; \
        static constexpr e2::type cause = event_item;                     \
    };

    #define ARGTYPE(event_item) typename type_clue<event_item>::param

    //#define SUBMIT0(event_level, event_item, event_arg) \
    //	bell::submit<type_clue<event_item>>(event_level, [&] (ARGTYPE(event_item)& event_arg)

    // Usage: SUBMIT(tier, item, arg) { ...expression; };
    #define SUBMIT(event_level, event_item, event_arg)    \
        bell::template submit2<type_clue<event_item>>(event_level) \
            = [&] (ARGTYPE(event_item)& event_arg)

    //#define SUBMIT_BYVAL0(event_level, event_item, event_arg) \
    //	bell::submit<type_clue<event_item>>(event_level, [=] (ARGTYPE(event_item)& event_arg)

    // Usage: SUBMIT_BYVAL(tier, item, arg) { ...expression; };
    #define SUBMIT_BYVAL(event_level, event_item, event_arg) \
        bell::template submit2<type_clue<event_item>>(event_level)    \
            = [=] (ARGTYPE(event_item)& event_arg)

    // Usage: SUBMIT_BYVAL_T(tier, item, event_token, arg) { ...expression; };
    #define SUBMIT_BYVAL_T(event_level, event_item, event_token, event_arg) \
        bell::template submit2<type_clue<event_item>>(event_level, event_token)    \
            = [=] (ARGTYPE(event_item)& event_arg)

    #define SUBMIT_V(event_level, event_item, event_hndl) \
        bell::template submit<type_clue<event_item>>(event_level, event_hndl)
    
    #define SUBMIT_TV(event_level, event_item, event_token, event_hndl) \
        bell::template submit<type_clue<event_item>>(event_level, event_token, event_hndl)
    
    //#define SUBMIT_T0(event_level, event_item, event_token, event_arg) \
    //	bell::submit<type_clue<event_item>>(event_level, event_token, [&] (ARGTYPE(event_item)& event_arg)

    // Usage: SUBMIT_BYVAL(tier, item, token/tokens, arg) { ...expression; };
    #define SUBMIT_T(event_level, event_item, event_token, event_arg)  \
        bell::template submit2<type_clue<event_item>>(event_level, event_token) \
            = [&] (ARGTYPE(event_item)& event_arg)

    //todo deprecated: used only with pro::focus
    //#define CHANGE(event_item_got, event_token, evemt_item_lost, event_arg) \
    //	bell::template change<type_clue<event_item_got>>(event_token, evemt_item_lost, event_arg);

    //#define SIGNAL(event_level, event_item, event_arg) \
    //	bell::signal(event_level, event_item, static_cast<ARGTYPE(event_item) &>(event_arg));

    #define SIGNAL(event_level, event_item, event_arg) \
        bell::template signal<event_level>(event_item, static_cast<ARGTYPE(event_item) &>(event_arg))

    #define SIGNAL_GLOBAL(event_item, event_arg) \
        bell::template signal_global(event_item, static_cast<ARGTYPE(event_item) &>(event_arg))
    
    #define SUBMIT_GLOBAL(event_item, event_token, event_arg)  \
        bell::template submit_global<type_clue<event_item>>(event_token) \
            = [&] (ARGTYPE(event_item)& event_arg)

}

#endif // NETXS_EVENTS_H