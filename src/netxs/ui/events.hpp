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
    struct e2
    {
        using type = unsigned int;

    private: static const unsigned int _width = 4;
    private: static const unsigned int _mask = (1 << _width) - 1;
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
            _command    = any | (8 << 0), // exec UI command (arg: iota)
            dtor        = any | (9 << 0), // Notify about object destruction, release only (arg: const id_t)
            //radio       = any | (10<< 0), // return active radio id_t (arg: const id_t)
            _bindings   = any | (11<< 0), // Dynamic Data Bindings.
            _render     = any | (12<< 0), // release: UI-tree rendering (arg: face).
            postrender  = any | (13<< 0), // release: UI-tree post-rendering (arg: face).
            _size       = any | (14<< 0), // release: Object size (arg: twod).
            _coor       = any | (15<< 0), // release: Object coor (arg: twod).
        };
        //private: static const unsigned int _level = _toplevel + _width;
        private: static const unsigned int _level0 = _width;
        public:
        struct size { enum : type {
                any = e2::_size,                    // preview: checking by pro::limit (arg: twod).
                set         = any | (1 << _level0), // preview: checking by object; release: apply to object (arg: twod).
        };};
        struct coor { enum : type {
                any = e2::_coor,                    // preview: checking by pro::limit (arg: twod).
                set         = any | (1 << _level0), // preview: checking by object; release: apply to object (arg: twod).
        };};
        struct render { enum : type {
                any = e2::_render,                  // release: UI-tree default rendering submission (arg: face).
                prerender   = any | (1 << _level0), // release: UI-tree pre-rendering, used by pro::cache (can interrupt SIGNAL) and any kind of highlighters (arg: face).
        };};
        struct bindings { enum : type {
                any = e2::_bindings,
                _list       = any | (1 << _level0),
            };
            private: static const unsigned int _level1 = _level0 + _width;
            public:
            struct list { enum : type {
                    any = bindings::_list,
                    users       = any | (1 << _level1), // list of connected users (arg: sptr<std::list<sptr<base>>>)
                    apps        = any | (2 << _level1), // list of running apps (arg: sptr<std::map<id_t, std::list<sptr<base>>>>)
                };
            };
        };
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
                _caret      = any | (1 << _level0), // any kind of intervals property (arg: period)
                broadcast   = any | (2 << _level0), // release: broadcast source changed, args: sptr<bell>.
                //resized     = any | (3 << _level0), //
            };
            private: static const unsigned int _level1 = _level0 + _width;
            public:
            struct caret { enum : type {
                    any = config::_caret,
                    blink       = any | (1 << _level1), // caret blinking interval (arg: period)
                    style       = any | (2 << _level1), // caret style: 0 - underline, 1 - box (arg: iota)
            };};
        };
        struct term { enum : type {
                any = e2::_term,
                unknown     = any | (1 << _level0),	// return platform unknown event code
                error       = any | (2 << _level0),	// return error code
                focus       = any | (3 << _level0), // order to change focus (arg: bool)
                key         = any | (4 << _level0), // keybd activity (arg: syskeybd)
                native      = any | (5 << _level0), // extended functionality (arg: bool)
                mouse       = any | (6 << _level0), // mouse activity (arg: sysmouse)
                size        = any | (7 << _level0), // order to update terminal primary overlay (arg: newsize twod)
                layout      = any | (8 << _level0),
                preclose    = any | (9 << _level0), // signal to quit after idle timeout (arg: bool - ready to shutdown)
                quit        = any | (10<< _level0), // quit (arg: text - bye msg)
                pointer     = any | (11<< _level0), // mouse pointer visibility (arg: bool)
                //menu      = any | (11 << _level0),
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
                    gone        = any | (6 << _level1), // release::global: Notify about the mouse controller is gone (args: hids).
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
                _draggable  = any | (1 << _level0), // signal to the form to enable draggablity for specified mouse button (arg: bool)
                _layout     = any | (2 << _level0),
                _highlight  = any | (3 << _level0),
                _upon       = any | (4 << _level0),
                _proceed    = any | (5 << _level0),
                //key         = any | (6 << _level0),
                _cursor     = any | (7 << _level0),
                _animate    = any | (8 << _level0),
                _drag       = any | (9 << _level0),
                _prop       = any | (10<< _level0),
                _upevent    = any | (11<< _level0), // eventss streamed up (to children) of the visual tree by base::
                _global     = any | (12<< _level0),
                _state      = any | (13<< _level0),
                canvas      = any | (14<< _level0), // request global canvas (arg: sptr<core>)
                _notify     = any | (15<< _level0), // request global canvas (arg: sptr<core>)
            };
            private: static const unsigned int _level1 = _level0 + _width;
            public:
            struct drag { enum : type {
                    any = form::_drag,
                    _start      = any | (1 << _level1), // notify about mouse drag start by pro::mouse (arg: hids)
                    _pull       = any | (2 << _level1), // notify about mouse drag pull by pro::mouse (arg: hids)
                    _cancel     = any | (3 << _level1), // notify about mouse drag cancel by pro::mouse (arg: hids)
                    _stop       = any | (4 << _level1), // notify about mouse drag stop by pro::mouse (arg: hids)
                };
                private: static const unsigned int _level2 = _level1 + _width;
                public:
                struct start { enum : type {
                        any = drag::_start,
                        left        = any | (1 << _level2),
                        right       = any | (2 << _level2),
                        leftright   = any | (3 << _level2),
                        middle      = any | (4 << _level2),
                        wheel       = any | (5 << _level2),
                        win         = any | (6 << _level2),
                };};
                struct pull { enum : type {
                        any = drag::_pull,
                        left        = any | (1 << _level2),
                        right       = any | (2 << _level2),
                        leftright   = any | (3 << _level2),
                        middle      = any | (4 << _level2),
                        wheel       = any | (5 << _level2),
                        win         = any | (6 << _level2),
                };};
                struct cancel { enum : type {
                        any = drag::_cancel,
                        left        = any | (1 << _level2),
                        right       = any | (2 << _level2),
                        leftright   = any | (3 << _level2),
                        middle      = any | (4 << _level2),
                        wheel       = any | (5 << _level2),
                        win         = any | (6 << _level2),
                };};
                struct stop { enum : type {
                        any = drag::_stop,
                        left        = any | (1 << _level2),
                        right       = any | (2 << _level2),
                        leftright   = any | (3 << _level2),
                        middle      = any | (4 << _level2),
                        wheel       = any | (5 << _level2),
                        win         = any | (6 << _level2),
                };};
            };
            // Form events that should be propagated down to the visual branch
            struct notify { enum : type {
                    any = form::_notify,
                    _mouse      = any | (1 << _level1), // request context menu at specified coords (arg: twod)
                    _keybd      = any | (2 << _level1), // request the prev scene window (arg: twod)
                };
                private: static const unsigned int _level2 = _level1 + _width;
                public:
                struct mouse { enum : type {
                        any = notify::_mouse,               // inform the form about the mouse hover (arg: hids)
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
                    _object     = any | (5 << _level1), // global scene objects events
                    _user       = any | (6 << _level1), // global scene users events
                };
                private: static const unsigned int _level2 = _level1 + _width;
                public:
                struct object { enum : type {
                        any = global::_object,
                        attached        = any | (1 << _level2), // global: object attached to the world (args: sptr<base>)
                        detached        = any | (2 << _level2), // global: object detached from the world (args: sptr<base>)
                };};
                struct user { enum : type {
                        any = global::_user,
                        attached        = any | (1 << _level2), // global: user attached to the world (args: sptr<base>)
                        detached        = any | (2 << _level2), // global: user detached from the world (args: sptr<base>)
                };};
            };
            struct upevent { enum : type {
                    any = form::_upevent,
                    kboffer     = any | (1 << _level1), // inform nested objects that the keybd focus should be taken (arg: hids)
            };};
            struct draggable { enum : type {
                    any = form::_draggable,
                    left        = any | (1 << _level1),
                    right       = any | (2 << _level1),
                    leftright   = any | (3 << _level1),
                    middle      = any | (4 << _level1),
                    wheel       = any | (5 << _level1),
                    win         = any | (6 << _level1),
            };};
            struct prop { enum : type {
                    any = form::_prop,
                    header      = any | (1 << _level1), // set form caption header (arg: text)
                    footer      = any | (2 << _level1), // set form caption footer (arg: text)
                    zorder      = any | (3 << _level1), // set form z-order (arg: iota: -1 backmost, 0 plain, 1 topmost)
                    brush       = any | (4 << _level1), // set form brush/color (arg: cell)
                    fullscreen  = any | (5 << _level1), // set fullscreen flag (arg: bool)
                    name        = any | (6 << _level1), // user name (arg: text)
                    viewport    = any | (7 << _level1), // request: return form actual viewport (arg: rect)
            };};
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
                    _vtree      = any | (1 << _level1), // visual tree events (arg: parent base_sptr)
                    //detached    = any | (2 << _level1), // inform that subject is detached (arg: parent bell_sptr)
                    redrawn     = any | (3 << _level1), // inform about camvas is completely redrawn (arg: canvas face)
                    //invalidated = any | (4 << _level1),
                    cached      = any | (5 << _level1), // inform about camvas is cached (arg: canvas face)
                    wiped       = any | (6 << _level1), // event after wipe the canvas (arg: canvas face)
                    created     = any | (7 << _level1), // event after itself creation (arg: itself bell_sptr)
                    //moved       = any | (8 << _level1), // release: event after moveto (arg: diff bw old and new coor twod). preview: event after moved by somebody.
                    changed     = any | (9 << _level1), // event after resize (arg: diff bw old and new size twod)
                    _scroll     = any | (10<< _level1), // event after scroll (arg: rack)
                    dragged     = any | (11<< _level1), // event after drag (arg: hids)
                };
                private: static const unsigned int _level2 = _level1 + _width;
                public:
                struct vtree { enum : type {
                        any = upon::_vtree,
                        attached    = any | (1 << _level2), // Child has been attached (arg: parent sptr<base>)
                        detached    = any | (2 << _level2), // Child has been detached (arg: parent sptr<base>)
                };};
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
                    attach      = any | (5 << _level1), // order to attach a child (arg: parent base_sptr)
                    detach      = any | (6 << _level1), // order to detach a child (e2::release - kill itself, e2::preview - detach the child specified in args) (arg: child  base_sptr)
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
                    //coor        = any | (1 << _level1), // return client rect coor (preview: subject to change)
                    //size        = any | (2 << _level1), // return client rect size (preview: subject to change)
                    //rect        = any | (3 << _level1), // return client rect (preview: subject to change)
                    //show        = any | (3 << _level1), // order to make it visible (arg: bool notify or not)
                    //hide        = any | (4 << _level1), // order to make it hidden (arg: bool notify or not)
                    shift       = any | (5 << _level1), // request a global shifting  with delta (const twod)
                    convey      = any | (6 << _level1), // request a global conveying with delta (Inform all children to be conveyed) (arg: cube)
                    order       = any | (7 << _level1), // return
                    local       = any | (8 << _level1), // Recursively calculate local coordinate from global (arg: twod)
                    strike      = any | (9 << _level1), // (always preview) inform about the child canvas has changed (arg: modified region rect)
                    bubble      = any | (10<< _level1), // order to popup the requested item through the visual tree (arg: form)
                    expose      = any | (11<< _level1), // order to bring the requested item on top of the visual tree (release: ask parent to expose specified child; preview: ask child to expose itself) (arg: base)
                    //next        = any | (12<< _level1), // request client for next child object (arg is only request: sptr<base>)
                    //prev        = any | (13<< _level1), // request client for prev child object (arg is only request: sptr<base>)
                    // footer     = any | (14<< _level1), // notify the client has changed footer (arg is only release: const rich)
                    appear      = any | (14<< _level1), // fly tothe specified coords, arg: twod
                    //clientrect  = any | (15<< _level1), // notify the client area has changed (arg is only release: rect)
            };};
            struct state { enum : type {
                    any = form::_state,
                    mouse       = any | (1 << _level1), // notify the client is mouse active or not. The form is active when the number of client (form::eventa::mouse::enter - mouse::leave) is not zero. (arg is only release: iota - number of clients)
                    keybd       = any | (2 << _level1), // notify the client is keybd active or not. The form is active when the number of client (form::eventa::keybd::got - keybd::lost) is not zero. (arg is only release: bool)
                    header      = any | (3 << _level1), // notify the client has changed title  (arg: para)
                    footer      = any | (4 << _level1), // notify the client has changed footer (arg: para)
                    params      = any | (5 << _level1), // notify the client has changed title params (arg: para)
                    color       = any | (6 << _level1), // notify the client has changed tone (preview to set, arg: tone)
            };};
        };
        struct data { enum : type {
                any = e2::_data,
                changed     = any | (1 << _level0), // return digest
                request     = any | (2 << _level0),
                disable     = any | (3 << _level0),
                flush       = any | (4 << _level0),
                text        = any | (5 << _level0), // release: signaling with a text string (args: const text).
        };};
        struct command { enum : type {
                any = e2::_command,
                quit        = any | (1 << _level0), // return bye msg //errcode (arg: const view)
                cout        = any | (2 << _level0), // Append extra data to output (arg: const text)
                custom      = any | (3 << _level0), // Custom command (arg: cmd_id iota)
        };};
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

    #define EVENT_NS     \
    template<e2::type T> \
    struct type_clue {};

    #define EVENT_BIND(item, item_t)              \
    template<>                                    \
    struct type_clue<item>                        \
    {                                             \
        using                     param = item_t; \
        static constexpr e2::type cause = item;   \
    };

    #define EVENT_SAME(master, item)                                \
    template<>                                                      \
    struct type_clue<item>                                          \
    {                                                               \
        using                     param = type_clue<master>::param; \
        static constexpr e2::type cause = item;                     \
    };

    #define ARGTYPE(item) typename type_clue<item>::param

    // Usage: SUBMIT(tier, item, arg) { ...expression; };
    #define SUBMIT(level, item, arg) \
        bell::template submit2<type_clue<item>>(level) = [&] (ARGTYPE(item)&& arg)

    // Usage: SUBMIT_BYVAL(tier, item, arg) { ...expression; };
    //        Note: It is a mutable closure!
    #define SUBMIT_BYVAL(level, item, arg) \
        bell::template submit2<type_clue<item>>(level) = [=] (ARGTYPE(item)&& arg) mutable

    // Usage: SUBMIT_BYVAL_T(tier, item, token, arg) { ...expression; };
    //        Note: It is a mutable closure!
    #define SUBMIT_BYVAL_T(level, item, token, arg) \
        bell::template submit2<type_clue<item>>(level, token) = [=] (ARGTYPE(item)&& arg) mutable

    #define SUBMIT_V(level, item, hndl) \
        bell::template submit<type_clue<item>>(level, hndl)

    #define SUBMIT_TV(level, item, token, hndl) \
        bell::template submit<type_clue<item>>(level, token, hndl)

    // Usage: SUBMIT_BYVAL(tier, item, token/tokens, arg) { ...expression; };
    #define SUBMIT_T(level, item, token, arg) \
        bell::template submit2<type_clue<item>>(level, token) = [&] (ARGTYPE(item)&& arg)

    #define SIGNAL(level, item, arg) \
        bell::template signal<level>(item, static_cast<ARGTYPE(item)&&>(arg))

    #define SIGNAL_GLOBAL(item, arg) \
        bell::template signal_global(item, static_cast<ARGTYPE(item)&&>(arg))

    #define SUBMIT_GLOBAL(item, token, arg) \
        bell::template submit_global<type_clue<item>>(token) = [&] (ARGTYPE(item)&& arg)
}

#endif // NETXS_EVENTS_HPP