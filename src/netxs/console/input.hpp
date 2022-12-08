// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_INPUT_HPP
#define NETXS_INPUT_HPP

#include "richtext.hpp"
#include "../ui/events.hpp"

namespace netxs::input { struct hids; }

namespace netxs::events::userland
{
    struct hids
    {
        EVENTPACK( hids, netxs::events::userland::root::hids )
        {
            EVENT_XS( die    , input::hids ), // release::global: Notify about the mouse controller is gone. Signal to delete gears inside dtvt-objects.
            EVENT_XS( halt   , input::hids ), // release::global: Notify about the mouse controller is outside.
            EVENT_XS( spawn  , input::hids ), // release::global: Notify about the mouse controller is appear.
            GROUP_XS( clipbrd, input::hids ), // release/request: Set/get clipboard data.
            GROUP_XS( keybd  , input::hids ),
            GROUP_XS( mouse  , input::hids ),
            GROUP_XS( focus  , input::hids ), // release::global: Notify about the focus got/lost.
            GROUP_XS( notify , input::hids ), // Form events that should be propagated down to the visual branch.
            GROUP_XS( upevent, input::hids ), // events streamed up (to children) of the visual tree by base::.
            GROUP_XS( device , input::hids ), // Primary device event group for forwarding purposes.

            SUBSET_XS( clipbrd )
            {
                EVENT_XS( get, input::hids ), // release: Get clipboard data.
                EVENT_XS( set, input::hids ), // release: Set clipboard data.
            };
            SUBSET_XS( upevent )
            {
                //todo make group keybd::...
                EVENT_XS( kboffer, input::hids ), // inform nested objects that the keybd focus should be taken.
                EVENT_XS( kbannul, input::hids ), // inform nested objects that the keybd focus should be released.
            };
            SUBSET_XS( notify )
            {
                GROUP_XS( mouse, input::hids ), // request context menu at specified coords.
                GROUP_XS( keybd, input::hids ), // request the prev scene window.

                SUBSET_XS( mouse )
                {
                    EVENT_XS( enter, input::hids ), // inform the form about the mouse hover.
                    EVENT_XS( leave, input::hids ), // inform the form about leaving the mouse.
                };
                SUBSET_XS( keybd )
                {
                    EVENT_XS( got , input::hids ),
                    EVENT_XS( lost, input::hids ),
                    EVENT_XS( test, input::hids ),
                };
                SUBSET_XS( focus )
                {
                    EVENT_XS( got , input::hids ),
                    EVENT_XS( lost, input::hids ),
                };
            };
            SUBSET_XS( keybd )
            {
                EVENT_XS( down   , input::hids ),
                EVENT_XS( up     , input::hids ),
                GROUP_XS( control, input::hids ),
                GROUP_XS( state  , input::hids ),

                SUBSET_XS( control )
                {
                    GROUP_XS( up  , input::hids ),
                    GROUP_XS( down, input::hids ),

                    SUBSET_XS( up )
                    {
                        EVENT_XS( alt_right  , input::hids ),
                        EVENT_XS( alt_left   , input::hids ),
                        EVENT_XS( ctrl_right , input::hids ),
                        EVENT_XS( ctrl_left  , input::hids ),
                        EVENT_XS( shift_right, input::hids ),
                        EVENT_XS( shift_left , input::hids ),
                    };
                    SUBSET_XS( down )
                    {
                        EVENT_XS( alt_right  , input::hids ),
                        EVENT_XS( alt_left   , input::hids ),
                        EVENT_XS( ctrl_right , input::hids ),
                        EVENT_XS( ctrl_left  , input::hids ),
                        EVENT_XS( shift_right, input::hids ),
                        EVENT_XS( shift_left , input::hids ),
                    };
                };
                SUBSET_XS( state )
                {
                    GROUP_XS( on , input::hids ),
                    GROUP_XS( off, input::hids ),

                    SUBSET_XS( on )
                    {
                        EVENT_XS( numlock   , input::hids ),
                        EVENT_XS( capslock  , input::hids ),
                        EVENT_XS( scrolllock, input::hids ),
                        EVENT_XS( insert    , input::hids ),
                    };
                    SUBSET_XS( off )
                    {
                        EVENT_XS( numlock   , input::hids ),
                        EVENT_XS( capslock  , input::hids ),
                        EVENT_XS( scrolllock, input::hids ),
                        EVENT_XS( insert    , input::hids ),
                    };
                };
            };
            SUBSET_XS( mouse )
            {
                EVENT_XS( move   , input::hids ),
                EVENT_XS( focus  , input::hids ),
                GROUP_XS( button , input::hids ),
                GROUP_XS( scroll , input::hids ),

                SUBSET_XS( scroll )
                {
                    EVENT_XS( up  , input::hids ),
                    EVENT_XS( down, input::hids ),
                };
                SUBSET_XS( button )
                {
                    GROUP_XS( up      , input::hids ),
                    GROUP_XS( down    , input::hids ),
                    GROUP_XS( click   , input::hids ),
                    GROUP_XS( dblclick, input::hids ),
                    GROUP_XS( tplclick, input::hids ),
                    GROUP_XS( drag    , input::hids ),

                    SUBSET_XS( up )
                    {
                        EVENT_XS( left     , input::hids ),
                        EVENT_XS( right    , input::hids ),
                        EVENT_XS( middle   , input::hids ),
                        EVENT_XS( wheel    , input::hids ),
                        EVENT_XS( win      , input::hids ),
                        EVENT_XS( leftright, input::hids ),
                    };
                    SUBSET_XS( down )
                    {
                        EVENT_XS( left     , input::hids ),
                        EVENT_XS( right    , input::hids ),
                        EVENT_XS( middle   , input::hids ),
                        EVENT_XS( wheel    , input::hids ),
                        EVENT_XS( win      , input::hids ),
                        EVENT_XS( leftright, input::hids ),
                    };
                    SUBSET_XS( click )
                    {
                        EVENT_XS( left     , input::hids ),
                        EVENT_XS( right    , input::hids ),
                        EVENT_XS( middle   , input::hids ),
                        EVENT_XS( wheel    , input::hids ),
                        EVENT_XS( win      , input::hids ),
                        EVENT_XS( leftright, input::hids ),

                        INDEX_XS( left, right, middle, wheel, win, leftright ),
                    };
                    SUBSET_XS( dblclick )
                    {
                        EVENT_XS( left     , input::hids ),
                        EVENT_XS( right    , input::hids ),
                        EVENT_XS( middle   , input::hids ),
                        EVENT_XS( wheel    , input::hids ),
                        EVENT_XS( win      , input::hids ),
                        EVENT_XS( leftright, input::hids ),
                    };
                    SUBSET_XS( tplclick )
                    {
                        EVENT_XS( left     , input::hids ),
                        EVENT_XS( right    , input::hids ),
                        EVENT_XS( middle   , input::hids ),
                        EVENT_XS( wheel    , input::hids ),
                        EVENT_XS( win      , input::hids ),
                        EVENT_XS( leftright, input::hids ),
                    };
                    SUBSET_XS( drag )
                    {
                        GROUP_XS( start , input::hids ),
                        GROUP_XS( pull  , input::hids ),
                        GROUP_XS( cancel, input::hids ),
                        GROUP_XS( stop  , input::hids ),

                        SUBSET_XS( start )
                        {
                            EVENT_XS( left     , input::hids ),
                            EVENT_XS( right    , input::hids ),
                            EVENT_XS( middle   , input::hids ),
                            EVENT_XS( wheel    , input::hids ),
                            EVENT_XS( win      , input::hids ),
                            EVENT_XS( leftright, input::hids ),

                            INDEX_XS( left, right, middle, wheel, win, leftright ),
                        };
                        SUBSET_XS( pull )
                        {
                            EVENT_XS( left     , input::hids ),
                            EVENT_XS( right    , input::hids ),
                            EVENT_XS( middle   , input::hids ),
                            EVENT_XS( wheel    , input::hids ),
                            EVENT_XS( win      , input::hids ),
                            EVENT_XS( leftright, input::hids ),

                            INDEX_XS( left, right,  middle, wheel, win, leftright ),
                        };
                        SUBSET_XS( cancel )
                        {
                            EVENT_XS( left     , input::hids ),
                            EVENT_XS( right    , input::hids ),
                            EVENT_XS( middle   , input::hids ),
                            EVENT_XS( wheel    , input::hids ),
                            EVENT_XS( win      , input::hids ),
                            EVENT_XS( leftright, input::hids ),

                            INDEX_XS( left, right, middle, wheel, win, leftright ),
                        };
                        SUBSET_XS( stop )
                        {
                            EVENT_XS( left     , input::hids ),
                            EVENT_XS( right    , input::hids ),
                            EVENT_XS( middle   , input::hids ),
                            EVENT_XS( wheel    , input::hids ),
                            EVENT_XS( win      , input::hids ),
                            EVENT_XS( leftright, input::hids ),

                            INDEX_XS( left, right, middle, wheel, win, leftright ),
                        };
                    };
                };
            };
            SUBSET_XS( focus )
            {
                EVENT_XS( set, input::hids ), // release: Set keybd focus.
                EVENT_XS( off, input::hids ), // release: Off keybd focus.
            };
            SUBSET_XS( device )
            {
                EVENT_XS( mouse, input::hids ), // release: Primary mouse event for forwarding purposes.
                //EVENT_XS( keybd, input::hids ), // release: Primary keybd event for forwarding purposes.
            };
        };
    };
}

namespace netxs::input
{
    using namespace netxs::ui::atoms;
    using namespace netxs::datetime;
    using hint = netxs::events::type;
    using netxs::events::bell;
    using netxs::events::subs;
    using netxs::events::tier;
    using netxs::events::hook;
    using netxs::events::id_t;
    using netxs::ansi::clip;
    using sysmouse = ansi::dtvt::binary::sysmouse_t;
    using syskeybd = ansi::dtvt::binary::syskeybd_t;
    using sysfocus = ansi::dtvt::binary::sysfocus_t;

    // console: Mouse tracker.
    struct mouse
    {
        using mouse_event = netxs::events::userland::hids::mouse;
        using click = mouse_event::button::click;

        enum buttons
        {
            left      = click::left     .index(),
            right     = click::right    .index(),
            middle    = click::middle   .index(),
            wheel     = click::wheel    .index(),
            win       = click::win      .index(),
            leftright = click::leftright.index(),
            numofbuttons,
        };
        enum stat
        {
            ok,
            halt,
            die,
        };
        struct knob_t
        {
            bool pressed; // knod: Button pressed state.
            bool dragged; // knod: The button is in a dragging state.
            bool blocked; // knod: The button is blocked (leftright disables left and right).
        };
        struct hist_t // Timer for successive double-clicks, e.g. triple-clicks.
        {
            using time = moment;
            time fired; // hist: .
            twod coord; // hist: .
            si32 count; // hist: .
        };

        using hist = std::array<hist_t, numofbuttons>;
        using knob = std::array<knob_t, numofbuttons>;
        using tail = netxs::datetime::tail<twod>;
        using time = netxs::datetime::period;

        static constexpr auto dragstrt = mouse_event::button::drag::start:: any.group<numofbuttons>();
        static constexpr auto dragpull = mouse_event::button::drag::pull::  any.group<numofbuttons>();
        static constexpr auto dragcncl = mouse_event::button::drag::cancel::any.group<numofbuttons>();
        static constexpr auto dragstop = mouse_event::button::drag::stop::  any.group<numofbuttons>();
        static constexpr auto released = mouse_event::button::up::          any.group<numofbuttons>();
        static constexpr auto pushdown = mouse_event::button::down::        any.group<numofbuttons>();
        static constexpr auto sglclick = mouse_event::button::click::       any.group<numofbuttons>();
        static constexpr auto dblclick = mouse_event::button::dblclick::    any.group<numofbuttons>();
        static constexpr auto tplclick = mouse_event::button::tplclick::    any.group<numofbuttons>();
        static constexpr auto scrollup = mouse_event::scroll::up.id;
        static constexpr auto scrolldn = mouse_event::scroll::down.id;
        static constexpr auto movement = mouse_event::move.id;
        static constexpr auto noactive = si32{ -1 };

        twod prime = {}; // mouse: System mouse cursor coordinates.
        twod coord = {}; // mouse: Relative mouse cursor coordinates.
        tail delta = {}; // mouse: History of mouse movements for a specified period of time.
        bool reach = {}; // mouse: Has the event tree relay reached the mouse event target.
        bool nodbl = {}; // mouse: Whether single click event processed (to prevent double clicks).
        bool scrll = {}; // mouse: Vertical scrolling.
        bool hzwhl = {}; // mouse: Horizontal scrolling.
        si32 whldt = {}; // mouse: Scroll delta.
        si32 locks = {}; // mouse: State of the captured buttons (bit field).
        si32 index = {}; // mouse: Index of the active button. -1 if the buttons are not involed.
        id_t swift = {}; // mouse: Delegate's ID of the current mouse owner.
        id_t hover = {}; // mouse: Hover control ID.
        id_t start = {}; // mouse: Initiator control ID.
        hint cause = {}; // mouse: Current event id.
        hist stamp = {}; // mouse: Recorded intervals between successive button presses to track double-clicks.
        time delay = {}; // mouse: Double-click threshold.
        knob bttns = {}; // mouse: Extended state of mouse buttons.
        sysmouse m = {}; // mouse: Device state.

        // mouse: Forward the extended mouse event.
        virtual void fire(hint cause, si32 index = mouse::noactive) = 0;
        // mouse: Try to forward the mouse event intact.
        virtual bool fire_fast() = 0;

        // mouse: Forward the specified button event.
        template<class T>
        void fire(T const& event_subset, si32 index)
        {
            fire(event_subset[index], index);
        }
        // mouse: Pack the button state into a bitset.
        auto take_button_state()
        {
            auto bitstat = ui32{};
            auto pressed = 1 << 0;
            auto dragged = 1 << 1;
            auto blocked = 1 << 2;
            for (auto& b : bttns)
            {
                if (b.pressed) bitstat |= pressed;
                if (b.dragged) bitstat |= dragged;
                if (b.blocked) bitstat |= blocked;
                pressed <<= 3;
                dragged <<= 3;
                blocked <<= 3;
            }
            return bitstat;
        }
        // mouse: Load the button state from a bitset.
        auto load_button_state(ui32 bitstat)
        {
            auto pressed = 1 << 0;
            auto dragged = 1 << 1;
            auto blocked = 1 << 2;
            for (auto& b : bttns)
            {
                b.pressed = bitstat & pressed;
                b.dragged = bitstat & dragged;
                b.blocked = bitstat & blocked;
                pressed <<= 3;
                dragged <<= 3;
                blocked <<= 3;
            }
        }
        // mouse: Return a kinetic animator.
        template<class LAW>
        auto fader(period spell)
        {
            //todo use current item's type: LAW<twod>
            return delta.fader<LAW>(spell);
        }
        // mouse: Extended mouse event generation.
        void update(sysmouse& m0)
        {
            auto m_buttons = std::bitset<8>(m0.buttons);
            // Interpret button combinations.
            //todo possible bug in Apple's Terminal - it does not return the second release
            //                                        in case when two buttons are pressed.
            m_buttons[leftright] = (bttns[leftright].pressed && (m_buttons[left] || m_buttons[right]))
                                                             || (m_buttons[left] && m_buttons[right]);
            m0.buttons = static_cast<ui32>(m_buttons.to_ulong());
            m.set(m0);
            auto busy = captured();
            if (busy && fire_fast()) //todo fire_fast on mods press
            {
                delta.set(m.coordxy - prime);
                coord = m.coordxy;
                prime = m.coordxy;
                fire(movement); // Update mouse enter/leave state.
                return;
            }
            if (m_buttons[leftright]) // Cancel left and right dragging if it is.
            {
                if (bttns[left].dragged)
                {
                    bttns[left].dragged = faux;
                    fire(dragcncl, left);
                }
                if (bttns[right].dragged)
                {
                    bttns[right].dragged = faux;
                    fire(dragcncl, right);
                }
                m_buttons[left ] = faux;
                m_buttons[right] = faux;
                m.buttons = static_cast<ui32>(m_buttons.to_ulong());
            }

            // Suppress left and right to avoid single button tracking (click, pull, etc)
            bttns[left ].blocked = m_buttons[leftright] || bttns[leftright].pressed;
            bttns[right].blocked = bttns[left].blocked;

            if (m.coordxy != prime)
            {
                delta.set(m.coordxy - prime);
                auto active = si32{};
                auto genptr = std::begin(bttns);
                for (auto i = 0; i < numofbuttons; i++)
                {
                    auto& genbtn = *genptr++;
                    if (genbtn.pressed && !genbtn.blocked)
                    {
                        if (!genbtn.dragged)
                        {
                            fire(dragstrt, i);
                            genbtn.dragged = true;
                        }
                        active |= 1 << i;
                    }
                }
                coord = m.coordxy;
                prime = m.coordxy;
                for (auto i = 0; active; ++i)
                {
                    if (active & 0x1)
                    {
                        fire(dragpull, i);
                    }
                    active >>= 1;
                }
                fire(movement);
            }

            if (!busy && fire_fast()) return;

            auto genptr = std::begin(bttns);
            for (auto i = 0; i < numofbuttons; i++)
            {
                auto& genbtn = *genptr++;
                auto  sysbtn = m_buttons[i];
                if (genbtn.pressed != sysbtn)
                {
                    genbtn.pressed = sysbtn;
                    if (genbtn.pressed)
                    {
                        fire(pushdown, i);
                    }
                    else
                    {
                        if (genbtn.dragged)
                        {
                            genbtn.dragged = faux;
                            fire(dragstop, i);
                        }
                        else
                        {
                            if (!genbtn.blocked)
                            {
                                fire(sglclick, i);
                            }
                            if (!nodbl)
                            {
                                // Fire a double/triple-click if delay is not expired
                                // and the mouse is at the same position.
                                auto& s = stamp[i];
                                auto fired = tempus::now();
                                if (fired - s.fired < delay && s.coord == coord)
                                {
                                    if (!genbtn.blocked)
                                    {
                                        if (s.count == 1)
                                        {
                                            fire(dblclick, i);
                                            s.fired = fired;
                                            s.count = 2;
                                        }
                                        else if (s.count == 2)
                                        {
                                            fire(tplclick, i);
                                            s.fired = {};
                                            s.count = {};
                                        }
                                    }
                                }
                                else
                                {
                                    s.fired = fired;
                                    s.coord = coord;
                                    s.count = 1;
                                }
                            }
                        }
                        fire(released, i);
                    }
                }
            }

            coord = m.coordxy;
            if (m.wheeled || m.hzwheel)
            {
                scrll = m.wheeled;
                hzwhl = m.hzwheel;
                whldt = m.wheeldt;
                fire(m.wheeldt > 0 ? scrollup : scrolldn);
                m.wheeled = {}; // Clear one-shot events.
                m.hzwheel = {};
                m.wheeldt = {};
                m.doubled = {};
            }
        }
        // mouse: Initiator of visual tree informing about mouse enters/leaves.
        template<bool Entered>
        bool direct(id_t asker)
        {
            if constexpr (Entered)
            {
                if (!start) // The first one to track the mouse will assign itslef to be a root.
                {
                    start = asker;
                    return true;
                }
                return start == asker;
            }
            else
            {
                if (start == asker)
                {
                    start = 0;
                    return true;
                }
                return faux;
            }
        }
        // mouse: Is the mouse seized/captured by asker?
        bool captured(id_t asker) const
        {
            return swift == asker;
        }
        // mouse: Is the mouse seized/captured?
        bool captured() const
        {
            return swift;
        }
        // mouse: Seize specified mouse control.
        bool capture(id_t asker)
        {
            if (!swift || swift == asker)
            {
                swift = asker;
                if (index != mouse::noactive) locks |= 1 << index;
                return true;
            }
            return faux;
        }
        // mouse: Release specified mouse control.
        void setfree(bool forced = true)
        {
            forced |= index == mouse::noactive;
            locks = forced ? 0
                           : locks & ~(1 << index);
            if (!locks) swift = {};
        }
    };

    // console: Keybd tracker.
    struct keybd
    {
        enum key
        {
            Backspace = 0x08,
            Tab       = 0x09,
            //CLEAR     = 0x0C,
            Enter     = 0x0D,
            Shift     = 0x10,
            Control   = 0x11,
            Alt       = 0x12,
            Pause     = 0x13,
            Escape    = 0x1B,
            PageUp    = 0x21,
            PageDown  = 0x22,
            End       = 0x23,
            Home      = 0x24,
            Left      = 0x25,
            Up        = 0x26,
            Right     = 0x27,
            Down      = 0x28,
            Insert    = 0x2D,
            Delete    = 0x2E,
            F1        = 0x70,
            F2        = 0x71,
            F3        = 0x72,
            F4        = 0x73,
            F5        = 0x74,
            F6        = 0x75,
            F7        = 0x76,
            F8        = 0x77,
            F9        = 0x78,
            F10       = 0x79,
            F11       = 0x7A,
            F12       = 0x7B,
        };

        wchr winchar = {}; // MS Windows specific.
        text cluster = {};
        bool pressed = {};
        ui16 imitate = {};
        ui16 virtcod = {};
        ui16 scancod = {};
        hint cause = netxs::events::userland::hids::keybd::any.id;

        void update(syskeybd& k)
        {
            pressed = k.pressed;
            imitate = k.imitate;
            virtcod = k.virtcod;
            scancod = k.scancod;
            cluster = k.cluster;
            winchar = k.winchar;
            fire_keybd();
        }

        virtual void fire_keybd() = 0;
    };

    // console: Human interface device controller.
    struct hids
        : public mouse,
          public keybd,
          public bell
    {
        using events = netxs::events::userland::hids;
        using list = std::list<wptr<bell>>;
        using xmap = netxs::console::core;

        id_t        relay; // hids: Mouse routing call stack initiator.
        xmap const& idmap; // hids: Area of the main form. Primary or relative region of the mouse coverage.
        list        kb_focus; // hids: Keyboard subscribers.
        bool        alive; // hids: Whether event processing is complete.
        period&     tooltip_timeout; // hids: .
        bool&       simple_instance; // hids: .

        //todo unify
        text        tooltip_data; // hids: Tooltip data.
        ui32        digest = 0; // hids: Tooltip digest.
        testy<ui32> digest_tracker = 0; // hids: Tooltip changes tracker.
        ui32        tooltip_digest = 0; // hids: Tooltip digest.
        moment      tooltip_time = {}; // hids: The moment to show tooltip.
        bool        tooltip_show = faux; // hids: Show tooltip or not.
        bool        tooltip_stop = faux; // hids: Disable tooltip.
        testy<twod> tooltip_coor = {}; // hids: .

        static constexpr auto enter_event   = events::notify::mouse::enter.id;
        static constexpr auto leave_event   = events::notify::mouse::leave.id;
        static constexpr auto keybd_take    = events::notify::keybd::got  .id;
        static constexpr auto keybd_lost    = events::notify::keybd::lost .id;
        static constexpr auto keybd_test    = events::notify::keybd::test .id;
        static constexpr auto focus_take    = events::notify::focus::got  .id;
        static constexpr auto focus_lost    = events::notify::focus::lost .id;
        static constexpr auto kboffer_event = events::upevent::kboffer    .id;
        static constexpr auto kbannul_event = events::upevent::kbannul    .id;
        //todo revise (focus_set/off)
        static constexpr auto focus_set     = events::focus::set          .id;
        static constexpr auto focus_off     = events::focus::off          .id;
        static constexpr auto halt_event    = events::halt                .id;
        static constexpr auto die_event     = events::die                 .id;
        static constexpr auto m_device      = events::device::mouse       .id;
        //static constexpr auto k_device = events::device::keybd.id;

    public:
        bell& owner;
        ui32 ctlstate = 0;
        ui32 winctrl = {}; // MS Windows specific.

        //todo unify
        rect slot; // slot for pro::maker and e2::createby.
        bool slot_forced = faux; // .

        //todo unify
        bool disabled = faux;
        bool kb_focus_changed = faux;
        bool force_group_focus = faux;
        bool combine_focus = faux;
        bool kb_focus_set = faux;
        si32 countdown = 0;

        virtual bool clear_clip_data()                                                      = 0;
        virtual void set_clip_data(clip const& data, bool forward = true) = 0;
        virtual clip get_clip_data()                                                        = 0;

        auto tooltip_enabled()
        {
            return !mouse::m.buttons
                && !disabled
                && !tooltip_stop
                && tooltip_show
                && tooltip_data.size()
                && !captured();
        }
        void set_tooltip(id_t src_id, view data)
        {
            if (src_id == 0 || tooltip_data.empty())
            {
                tooltip_data = data;
                digest++;
            }
        }
        void set_tooltip(view data, ui32 set_digest)
        {
            tooltip_data = data;
            digest = set_digest;
        }
        auto is_tooltip_changed()
        {
            return digest_tracker(digest);
        }
        auto get_tooltip()
        {
            return qiew{ tooltip_data };
        }
        void tooltip_recalc(hint deed)
        {
            if (tooltip_digest != digest) // Welcome new object.
            {
                tooltip_stop = faux;
            }

            if (!tooltip_stop)
            {
                if (deed == hids::events::mouse::move.id)
                {
                    if (tooltip_coor(mouse::coord) || (tooltip_show && tooltip_digest != digest)) // Do nothing on shuffle.
                    {
                        if (tooltip_show && tooltip_digest == digest) // Drop tooltip if moved.
                        {
                            tooltip_stop = true;
                        }
                        else
                        {
                            tooltip_time = tempus::now() + tooltip_timeout;
                            tooltip_show = faux;
                        }
                    }
                }
                else // Drop tooltip on any other event.
                {
                    tooltip_stop = true;
                    tooltip_digest = digest;
                }
            }
        }
        auto tooltip_check(moment now)
        {
            if (!tooltip_stop
             && !tooltip_show
             &&  tooltip_time < now
             && !captured())
            {
                tooltip_show = true;
                if (tooltip_digest != digest)
                {
                    tooltip_digest = digest;
                    return true;
                }
            }
            else if (tooltip_show == true && captured())
            {
                tooltip_show = faux;
                tooltip_stop = faux;
            }
            return faux;
        }

        void replay(hint cause, twod const& coord, twod const& delta, ui32 button_state)
        {
            static constexpr auto mask = netxs::events::level_mask(hids::events::mouse::button::any.id);
            static constexpr auto base = mask & hids::events::mouse::button::any.id;
            alive = true;
            mouse::coord = coord;
            mouse::cause = (cause & ~mask) | base; // Remove the dependency on the event tree root.
            mouse::delta.set(delta);
            mouse::load_button_state(button_state);
        }
        auto state()
        {
            return std::tuple{ force_group_focus,
                                kb_focus_changed,
                                   combine_focus,
                                       countdown };
        }
        template<class T>
        void state(T const& s)
        {
            force_group_focus = std::get<0>(s);
            kb_focus_changed  = std::get<1>(s);
            combine_focus     = std::get<2>(s);
            countdown         = std::get<3>(s);
        }

        enum modifiers : ui32
        {
            LShift   = 1 <<  0, // ⇧ Shift, Left  Shift
            RShift   = 1 <<  1, //          Right Shift
            LAlt     = 1 <<  2, // ⎇ Alt, ⌥ Option,   Left  Alt
            RAlt     = 1 <<  3, // ⇮ AltGr, Alt Graph, Right Alt
            LCtrl    = 1 <<  4, // ⌃ Ctrl, Left  Ctrl
            RCtrl    = 1 <<  5, //         Right Ctrl
            Meta     = 1 <<  6, // ◆ Meta, ⊞ Win, ⌘ Cmd (Apple key), ❖ Super
            Fn       = 1 <<  7, //
            CapsLock = 1 <<  8, // ⇪ Caps Lock
            NumLock  = 1 <<  9, // ⇭ Num Lock
            ScrlLock = 1 << 10, // ⇳ Scroll Lock (⤓)
            anyCtrl  = LCtrl  | RCtrl,
            anyAlt   = LAlt   | RAlt,
            anyShift = LShift | RShift,
        };

        auto meta(ui32 ctl_key = -1) { return hids::ctlstate & ctl_key; }
        auto kbmod()
        {
            return meta(hids::anyCtrl | hids::anyAlt | hids::anyShift);
        }

        hids(bell& owner, xmap const& idmap, time& dblclick_timeout, time& tooltip_timeout, bool& simple_instance)
            : relay{ 0     },
              owner{ owner },
              idmap{ idmap },
              alive{ faux  },
              tooltip_timeout{ tooltip_timeout },
              simple_instance{ simple_instance }
        {
            mouse::prime = dot_mx;
            mouse::coord = dot_mx;
            mouse::delay = dblclick_timeout;
        }
       ~hids()
        {
            auto lock = netxs::events::sync{};
            mouse_leave(mouse::hover, mouse::start);
            clear_kb_focus();
            bell::signal_global(halt_event, *this);
            bell::signal_global(die_event, *this);
        }

        // hids: Stop handeling this event.
        void dismiss(bool set_nodbl = faux)
        {
            alive = faux;
            if (set_nodbl) nodbl = true;
        }

        // hids: Whether event processing is complete.
        operator bool () const
        {
            return alive;
        }
        void take(sysmouse& m)
        {
            ctlstate = m.ctlstat;
            winctrl  = m.winctrl;
            disabled = faux;
            mouse::update(m);
        }
        void take(syskeybd& k)
        {
            ctlstate = k.ctlstat;
            winctrl  = k.winctrl;
            keybd::update(k);
        }
        void take(sysfocus& f)
        {
            kb_focus_changed = faux; //todo unify, see base::upevent handler
            if (f.enabled)
            {
                auto s = state();
                force_group_focus = f.force_group_focus;
                combine_focus     = f.combine_focus    ;
                owner.bell::template signal<tier::release>(kboffer_event, *this);
                state(s);
            }
            else
            {
                owner.bell::template signal<tier::release>(kbannul_event, *this);
            }
        }

        auto& area() const { return idmap.area(); }

        template<tier TIER, class T>
        void pass(sptr<T> object, twod const& offset, bool relative = faux)
        {
            if (object)
            {
                auto temp = mouse::coord;
                mouse::coord += offset;
                if (relative)
                {
                    object->global(coord);
                }
                object->bell::template signal<TIER>(mouse::cause, *this);
                mouse::coord = temp;
            }
        }
        void mouse_leave(id_t last_id, id_t start_id)
        {
            if (last_id)
            {
                if (auto last = bell::getref(last_id))
                {
                    auto start = mouse::start;
                    mouse::start = start_id;
                    last->bell::template signal<tier::release>(leave_event, *this);
                    mouse::start = start;
                }
                else log("hids: error condition: Clients count is broken, dangling ", last_id);
            }
        }
        void take_mouse_focus(bell& boss)
        {
            if (mouse::hover != boss.id) // The mouse cursor is over the new object.
            {
                if (tooltip_data.size())
                {
                    digest++;
                    tooltip_data.clear();
                }

                // Firing the leave event right after the enter allows us
                // to avoid flickering the parent object state when focus
                // acquired by children.
                auto start_l = mouse::start;
                mouse::start = 0; // The first one to track the mouse will assign itself by calling gear.direct<true>(id).
                boss.bell::template signal<tier::release>(enter_event, *this);
                mouse_leave(mouse::hover, start_l);
                mouse::hover = boss.id;
            }
        }
        void deactivate()
        {
            mouse::load_button_state(0);
            mouse::m.buttons = {};
            take_mouse_focus(owner);
            bell::signal_global(halt_event, *this);
            disabled = true;
        }
        void okay(bell& boss)
        {
            if (boss.id == relay)
            {
                take_mouse_focus(boss);
                boss.bell::template signal<tier::release>(mouse::cause, *this);
            }
        }
        void fire(hint cause, si32 index = mouse::noactive)
        {
            if (disabled) return;

            alive = true;
            mouse::index = index;
            mouse::cause = cause;
            mouse::coord = mouse::prime;
            mouse::nodbl = faux;

            auto& offset = idmap.coor();
            if (mouse::swift)
            {
                auto next = bell::getref(mouse::swift);
                if (next)
                {
                    take_mouse_focus(*next);
                    pass<tier::release>(next, offset, true);

                    if (alive && !captured()) // Pass unhandled event to the gate.
                    {
                        owner.bell::template signal<tier::release>(cause, *this);
                    }
                }
                else mouse::setfree();
            }
            else
            {
                tooltip_recalc(cause);
                owner.bell::template signal<tier::preview>(cause, *this);

                if (!alive) return;

                auto next = idmap.link(mouse::coord);
                if (next != owner.id)
                {
                    relay = next;
                    pass<tier::preview>(bell::getref(next), offset, true);
                    relay = 0;

                    if (!alive) return;
                }

                owner.bell::template signal<tier::release>(cause, *this); // Pass unhandled event to the gate.
            }
        }
        bool fire_fast()
        {
            if (disabled) return true;
            alive = true;
            auto next_id = mouse::swift ? mouse::swift
                                        : idmap.link(m.coordxy);
            if (next_id != owner.id)
            {
                if (auto next_ptr = bell::getref(next_id))
                {
                    auto& next = *next_ptr;
                    auto  temp = m.coordxy;
                    m.coordxy += idmap.coor();
                    next.global(m.coordxy);
                    next.bell::template signal<tier::release>(m_device, *this);
                    m.coordxy = temp;
                    if (!alive) // Clear one-shot events on success.
                    {
                        m.wheeled = {};
                        m.wheeldt = {};
                        m.hzwheel = {};
                        m.doubled = {};
                    }
                }
                else if (mouse::swift == next_id) mouse::setfree();
            }
            return !alive;
        }
        void fire_keybd()
        {
            alive = true;
            owner.bell::template signal<tier::preview>(keybd::cause, *this);

            auto iter = kb_focus.begin();
            while (alive && iter != kb_focus.end())
            {
                if (auto next = iter++->lock())
                {
                    next->bell::template signal<tier::preview>(keybd::cause, *this);
                }
                else kb_focus.erase(std::prev(iter));
            }
        }
        template<class P = noop>
        bool _check_kb_focus(sptr<bell> item, P proc = {})
        {
            auto iter = kb_focus.begin();
            while (iter != kb_focus.end())
            {
                if (auto next = iter->lock())
                {
                    if (item == next)
                    {
                        proc(iter);
                        return true;
                    }
                }
                iter++;
            }
            return faux;
        }
        void _add_kb_focus(sptr<bell> item)
        {
            kb_focus.push_back(item);
            item->bell::template signal<tier::release>(keybd_take, *this);
        }
        bool remove_from_kb_focus(sptr<bell> item)
        {
            return _check_kb_focus(item, [&](auto iter)
            {
                auto next = iter->lock();
                next->bell::template signal<tier::release>(keybd_lost, *this);
                kb_focus.erase(iter);
            });
        }
        void add_single_kb_focus(sptr<bell> item)
        {
            auto keep = true;
            auto iter = kb_focus.begin();
            while (iter != kb_focus.end())
            {
                if (auto next = iter->lock())
                {
                    if (item == next)
                    {
                        keep = faux;
                        iter++;
                        continue;
                    }
                    next->bell::template signal<tier::release>(keybd_lost, *this);
                }
                iter++;
                kb_focus.erase(std::prev(iter));
            }

            if (keep) _add_kb_focus(item);
        }
        auto add_group_kb_focus_or_release_captured(sptr<bell> item)
        {
            if (!remove_from_kb_focus(item))
            {
                _add_kb_focus(item);
                return true;
            }
            else return faux;
        }
        void set_kb_focus(sptr<bell> item)
        {
            kb_focus_changed = true;
            kb_focus_set = true;
            if (!simple_instance && (hids::meta(anyCtrl) || force_group_focus))
            {
                if (combine_focus)
                {
                    if (!_check_kb_focus(item)) _add_kb_focus(item);
                }
                else
                {
                    kb_focus_set = add_group_kb_focus_or_release_captured(item);
                }
            }
            else
            {
                add_single_kb_focus(item);
            }
            item->bell::template signal<tier::release>(keybd_test, *this);
        }
        void clear_kb_focus()
        {
            auto iter = kb_focus.begin();
            while (iter != kb_focus.end())
            {
                if (auto next = iter->lock())
                {
                    next->bell::template signal<tier::release>(keybd_lost, *this);
                }
                iter++;
                kb_focus.erase(std::prev(iter));
                //kb_focus.erase(iter);
            }
        }
        bool focus_changed()
        {
            return kb_focus_changed;
        }
        void pass_kb_focus(bell& inst)
        {
            clear_kb_focus();
            kb_focus_changed = faux;
            inst.bell::template signal<tier::release>(kboffer_event, *this);
        }
        void offer_kb_focus(sptr<bell> inst_ptr)
        {
            //todo signal to the owner to do a focus relay
            force_group_focus = true;
            kb_focus_changed = faux;
            inst_ptr->bell::template signal<tier::release>(kboffer_event, *this);
            force_group_focus = faux;
            owner.bell::template signal<tier::release>(focus_set, *this);
        }
        void annul_kb_focus(sptr<bell> inst_ptr)
        {
            kb_focus_changed = faux; //todo used in base::upevent handler
            inst_ptr->bell::template signal<tier::release>(kbannul_event, *this);
            owner.bell::template signal<tier::release>(focus_off, *this);
        }
        auto interpret()
        {
            auto textline = text{};
            auto ctrl = [&](auto pure, auto f, auto suffix)
            {
                textline = "\033";
                if (ctlstate & hids::anyShift)
                {
                    textline += f;
                    textline += ";2";
                }
                else if (ctlstate & hids::anyAlt)
                {
                    textline += f;
                    textline += ";3";
                }
                else if (ctlstate & hids::anyCtrl)
                {
                    textline += f;
                    textline += ";5";
                }
                else textline += pure;
                textline += suffix;
            };
            if (pressed)
            {
                switch (virtcod)
                {
                    //todo Ctrl+Space
                    //     Ctrl+Backspace
                    //     Alt+0..9
                    //     Ctrl/Shift+Enter
                    case key::Backspace: textline = "\177"; break;
                    case key::Tab:
                        textline = ctlstate & hids::anyShift ? "\033[Z"
                                                             : "\t";
                        break;
                    case key::PageUp:    ctrl("[5",  "[5",  "~"); break;
                    case key::PageDown:  ctrl("[6",  "[6",  "~"); break;
                    case key::Insert:    ctrl("[2",  "[2",  "~"); break;
                    case key::Delete:    ctrl("[3",  "[3",  "~"); break;
                    case key::End:       ctrl("[",   "[1",  "F"); break;
                    case key::Home:      ctrl("[",   "[1",  "H"); break;
                    case key::Up:        ctrl("[",   "[1",  "A"); break;
                    case key::Down:      ctrl("[",   "[1",  "B"); break;
                    case key::Right:     ctrl("[",   "[1",  "C"); break;
                    case key::Left:      ctrl("[",   "[1",  "D"); break;
                    case key::F1:        ctrl("O",   "[1",  "P"); break;
                    case key::F2:        ctrl("O",   "[1",  "Q"); break;
                    case key::F3:        ctrl("O",   "[1",  "R"); break;
                    case key::F4:        ctrl("O",   "[1",  "S"); break;
                    case key::F5:        ctrl("[15", "[15", "~"); break;
                    case key::F6:        ctrl("[17", "[17", "~"); break;
                    case key::F7:        ctrl("[18", "[18", "~"); break;
                    case key::F8:        ctrl("[19", "[19", "~"); break;
                    case key::F9:        ctrl("[20", "[20", "~"); break;
                    case key::F10:       ctrl("[21", "[21", "~"); break;
                    case key::F11:       ctrl("[23", "[23", "~"); break;
                    case key::F12:       ctrl("[24", "[24", "~"); break;
                    default:
                        textline = cluster;
                        break;
                }
            }
            return textline;
        }
    };
}

#endif // NETXS_INPUT_HPP