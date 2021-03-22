// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_CONSOLE_HPP
#define NETXS_CONSOLE_HPP

#include "richtext.hpp"
#include "../datetime/quartz.hpp"
#include "../abstract/iterator.hpp"
#include "../text/logger.hpp"

#include <iostream>

#define SPD 10               // console.h: Auto-scroll initial speed component ΔR.
#define PLS 167              // console.h: Auto-scroll initial speed component ΔT.
#define CCL 120              // console.h: Auto-scroll duration in ms.
#define SPD_ACCEL 3          // console.h: Auto-scroll speed accelation.
#define CCL_ACCEL 30         // console.h: Auto-scroll additional duration in ms.
#define SPD_MAX 100          // console.h: Auto-scroll max speed.
#define CCL_MAX 1000         // console.h: Auto-scroll max duration in ms.

#define STOPPING_TIME  2s    // console.h: Object state stopping duration in s.
#define SWITCHING_TIME 200   // console.h: Object state switching duration in ms.
#define BLINK_PERIOD   400ms // console.h: Period in ms between the blink states of the cursor.

#define ACTIVE_TIMEOUT  1s   // console.h: Timeout off the active object.
#define REPEAT_DELAY  500ms  // console.h: Repeat delay.
#define REPEAT_RATE    30ms  // console.h: Repeat rate.

namespace netxs::console
{
    using namespace netxs::datetime;
    using namespace netxs;

    class syskeybd;
    class sysmouse;
    class hids;
    class base;
    class form;
    class link;
    class host;
    class site;

    using hook = bell::hook;
    using subs = bell::subs;
    using id_t = bell::id_t;
    using hint = e2::type;
    using xipc = os::xipc;

    using drawfx = std::function<bool(face&, page const&)>;

    EVENT_NS
    EVENT_BIND(e2::timer::any, moment)
        EVENT_BIND(e2::timer::tick, moment)
        EVENT_BIND(e2::timer::fps,  iota)

    EVENT_BIND(e2::quit, const view)
    EVENT_BIND(e2::dtor, const id_t)
    EVENT_BIND(e2::radio, iota)
    EVENT_BIND(e2::cout, const text)

    EVENT_BIND(e2::debug::any, const view)
        EVENT_BIND(e2::debug::logs   , const view)
        EVENT_BIND(e2::debug::output , const view)
        EVENT_BIND(e2::debug::parsed , const page)

    EVENT_BIND(e2::term::any, iota)
        EVENT_BIND(e2::term::unknown , iota)
        EVENT_BIND(e2::term::error   , iota)
        EVENT_BIND(e2::term::focus   , bool)
        EVENT_BIND(e2::term::mouse   , sysmouse)
        EVENT_BIND(e2::term::key     , syskeybd)
        EVENT_BIND(e2::term::size    , twod)
        EVENT_BIND(e2::term::layout  , const twod)
        EVENT_BIND(e2::term::preclose, const bool)
        EVENT_BIND(e2::term::quit    , const view)

    EVENT_BIND(e2::config::any, iota)
        EVENT_BIND(e2::config::intervals::any, period)
            EVENT_SAME(e2::config::intervals::any, e2::config::intervals::blink)

    EVENT_BIND(e2::data::any, iota)
        EVENT_BIND(e2::data::changed, iota)
        EVENT_BIND(e2::data::request, iota)
        EVENT_BIND(e2::data::disable, iota)
        EVENT_BIND(e2::data::flush  , iota)

    EVENT_BIND(e2::form::any, bool)
        EVENT_BIND(e2::form::upevent::any, hids)
            EVENT_BIND(e2::form::upevent::kboffer , hids)

        EVENT_BIND(e2::form::canvas, sptr<core>)
        EVENT_BIND(e2::form::global::any, twod)
            EVENT_BIND(e2::form::global::ctxmenu , twod)
            EVENT_BIND(e2::form::global::lucidity, iota)

        EVENT_BIND(e2::form::notify::any                 , hids)
            EVENT_BIND(e2::form::notify::mouse::any      , hids)
                EVENT_BIND(e2::form::notify::mouse::enter, hids)
                EVENT_BIND(e2::form::notify::mouse::leave, hids)
            EVENT_BIND(e2::form::notify::keybd::any      , hids)
                EVENT_BIND(e2::form::notify::keybd::got  , hids)
                EVENT_BIND(e2::form::notify::keybd::lost , hids)

        EVENT_BIND(e2::form::prop::any, text)
            EVENT_BIND(e2::form::prop::header, text)
            EVENT_BIND(e2::form::prop::footer, text)
            //EVENT_BIND(e2::form::prop::params, text)

        // use e2::form::events instead
        //EVENT_BIND(e2::form::mouse::any, hids)
        //	EVENT_BIND(e2::form::mouse::enter, hids)
        //	EVENT_BIND(e2::form::mouse::leave, hids)
        //	//EVENT_BIND(e2::form::mouse::capture, iota)
        //	//EVENT_BIND(e2::form::mouse::release, iota)
        //
        //EVENT_BIND(e2::form::focus::any			, hook)
        //	EVENT_BIND(e2::form::focus::got		, hook)
        //	EVENT_BIND(e2::form::focus::lost	, hook)
        //EVENT_BIND(e2::form::focus::any			, bool)
        //	EVENT_BIND(e2::form::focus::got		, hids)
        //	EVENT_BIND(e2::form::focus::lost	, hids)

        EVENT_BIND(e2::form::layout::any, const twod)
            EVENT_BIND(e2::form::layout::move  , twod)
            EVENT_BIND(e2::form::layout::size  , twod)
            EVENT_BIND(e2::form::layout::show  , bool)
            EVENT_BIND(e2::form::layout::hide  , bool)
            EVENT_BIND(e2::form::layout::shift , const twod)
            EVENT_BIND(e2::form::layout::convey, cube)
            EVENT_BIND(e2::form::layout::local , twod)
            EVENT_BIND(e2::form::layout::strike, const rect)
            EVENT_BIND(e2::form::layout::bubble, base)
            EVENT_BIND(e2::form::layout::expose, base)
            EVENT_BIND(e2::form::layout::appear, twod)

        EVENT_BIND(e2::form::state::any, const twod)
            EVENT_BIND(e2::form::state::mouse , bool)
            EVENT_BIND(e2::form::state::keybd , bool)
            EVENT_BIND(e2::form::state::header, para)
            EVENT_BIND(e2::form::state::footer, para)
            EVENT_BIND(e2::form::state::params, para)
            EVENT_BIND(e2::form::state::color , tone)

        EVENT_BIND(e2::form::highlight::any, bool)
            EVENT_BIND(e2::form::highlight::on , bool)
            EVENT_BIND(e2::form::highlight::off, bool)

        EVENT_BIND(e2::form::upon::any, bool)
            EVENT_BIND(e2::form::upon::attached   , sptr<base>)
            EVENT_BIND(e2::form::upon::detached   , sptr<base>)
            EVENT_BIND(e2::form::upon::redrawn    , face)
            EVENT_BIND(e2::form::upon::invalidated, bool)
            EVENT_BIND(e2::form::upon::cached     , face)
            EVENT_BIND(e2::form::upon::wiped      , face)
            EVENT_BIND(e2::form::upon::created    , sptr<base>)
            EVENT_BIND(e2::form::upon::moved      , twod)
            EVENT_BIND(e2::form::upon::resized    , twod)

            EVENT_BIND(e2::form::upon::scroll::any, rack)
                EVENT_BIND(e2::form::upon::scroll::x     , rack)
                EVENT_BIND(e2::form::upon::scroll::y     , rack)
                EVENT_BIND(e2::form::upon::scroll::resetx, rack)
                EVENT_BIND(e2::form::upon::scroll::resety, rack)

        EVENT_BIND(e2::form::proceed::any, bool)
            EVENT_BIND(e2::form::proceed::create  , rect)
            EVENT_BIND(e2::form::proceed::createby, hids)
            EVENT_BIND(e2::form::proceed::destroy , base)
            //EVENT_BIND(e2::form::proceed::commit	, iota)
            EVENT_BIND(e2::form::proceed::render  , drawfx)
            //EVENT_BIND(e2::form::proceed::multirender, cuts)
            //EVENT_BIND(e2::form::proceed::draw  , face)
            EVENT_BIND(e2::form::proceed::attach  , sptr<base>)
            EVENT_BIND(e2::form::proceed::detach  , sptr<base>)
            //EVENT_BIND(e2::form::proceed::checkin, sptr<face>)

        EVENT_BIND(e2::form::cursor::any, bool)
            EVENT_BIND(e2::form::cursor::blink, bool)

        EVENT_BIND(e2::form::animate::any, id_t)
            EVENT_BIND(e2::form::animate::start, id_t)
            EVENT_BIND(e2::form::animate::stop , id_t)

    EVENT_BIND(e2::hids::any, hids)
        EVENT_SAME(e2::hids::any, e2::hids::mouse::any)
            EVENT_SAME(e2::hids::any, e2::hids::mouse::scroll::any)
                EVENT_SAME(e2::hids::any, e2::hids::mouse::scroll::up)
                EVENT_SAME(e2::hids::any, e2::hids::mouse::scroll::down)

            EVENT_SAME(e2::hids::any, e2::hids::mouse::move)
            EVENT_SAME(e2::hids::any, e2::hids::mouse::shuffle)
            EVENT_SAME(e2::hids::any, e2::hids::mouse::focus)

            EVENT_SAME(e2::hids::any, e2::hids::mouse::button::any)
                EVENT_SAME(e2::hids::any, e2::hids::mouse::button::up::any)
                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::up::left)
                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::up::right)
                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::up::middle)
                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::up::wheel)
                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::up::win)
                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::up::leftright)

                EVENT_SAME(e2::hids::any, e2::hids::mouse::button::down::any)
                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::down::left)
                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::down::right)
                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::down::middle)
                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::down::wheel)
                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::down::win)
                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::down::leftright)

                EVENT_SAME(e2::hids::any, e2::hids::mouse::button::click::any)
                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::click::left)
                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::click::right)
                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::click::middle)
                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::click::wheel)
                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::click::win)
                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::click::leftright)

                EVENT_SAME(e2::hids::any, e2::hids::mouse::button::dblclick::any)
                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::dblclick::left)
                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::dblclick::right)
                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::dblclick::middle)
                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::dblclick::wheel)
                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::dblclick::win)
                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::dblclick::leftright)

                EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::any)
                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::start::any)
                        EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::start::left)
                        EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::start::right)
                        EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::start::middle)
                        EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::start::wheel)
                        EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::start::win)
                        EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::start::leftright)

                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::pull::any)
                        EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::pull::left)
                        EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::pull::right)
                        EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::pull::middle)
                        EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::pull::wheel)
                        EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::pull::win)
                        EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::pull::leftright)

                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::cancel::any)
                        EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::cancel::left)
                        EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::cancel::right)
                        EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::cancel::middle)
                        EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::cancel::wheel)
                        EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::cancel::win)
                        EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::cancel::leftright)

                    EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::stop::any)
                        EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::stop::left)
                        EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::stop::right)
                        EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::stop::middle)
                        EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::stop::wheel)
                        EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::stop::win)
                        EVENT_SAME(e2::hids::any, e2::hids::mouse::button::drag::stop::leftright)

        EVENT_SAME(e2::hids::any, e2::hids::keybd::any)
            EVENT_SAME(e2::hids::any, e2::hids::keybd::down)
            EVENT_SAME(e2::hids::any, e2::hids::keybd::up)

            EVENT_SAME(e2::hids::any, e2::hids::keybd::control::any)
                EVENT_SAME(e2::hids::any, e2::hids::keybd::control::down::any)
                    EVENT_SAME(e2::hids::any, e2::hids::keybd::control::down::alt_right)
                    EVENT_SAME(e2::hids::any, e2::hids::keybd::control::down::alt_left)
                    EVENT_SAME(e2::hids::any, e2::hids::keybd::control::down::ctrl_right)
                    EVENT_SAME(e2::hids::any, e2::hids::keybd::control::down::ctrl_left)
                    EVENT_SAME(e2::hids::any, e2::hids::keybd::control::down::shift_right)
                    EVENT_SAME(e2::hids::any, e2::hids::keybd::control::down::shift_left)

                EVENT_SAME(e2::hids::any, e2::hids::keybd::control::up::any)
                    EVENT_SAME(e2::hids::any, e2::hids::keybd::control::up::alt_right)
                    EVENT_SAME(e2::hids::any, e2::hids::keybd::control::up::alt_left)
                    EVENT_SAME(e2::hids::any, e2::hids::keybd::control::up::ctrl_right)
                    EVENT_SAME(e2::hids::any, e2::hids::keybd::control::up::ctrl_left)
                    EVENT_SAME(e2::hids::any, e2::hids::keybd::control::up::shift_right)
                    EVENT_SAME(e2::hids::any, e2::hids::keybd::control::up::shift_left)

                EVENT_SAME(e2::hids::any, e2::hids::keybd::state::any)
                    EVENT_SAME(e2::hids::any, e2::hids::keybd::state::on::any)
                        EVENT_SAME(e2::hids::any, e2::hids::keybd::state::on::numlock)
                        EVENT_SAME(e2::hids::any, e2::hids::keybd::state::on::capslock)
                        EVENT_SAME(e2::hids::any, e2::hids::keybd::state::on::scrolllock)
                        EVENT_SAME(e2::hids::any, e2::hids::keybd::state::on::insert)

                    EVENT_SAME(e2::hids::any, e2::hids::keybd::state::off::any)
                        EVENT_SAME(e2::hids::any, e2::hids::keybd::state::off::numlock)
                        EVENT_SAME(e2::hids::any, e2::hids::keybd::state::off::capslock)
                        EVENT_SAME(e2::hids::any, e2::hids::keybd::state::off::scrolllock)
                        EVENT_SAME(e2::hids::any, e2::hids::keybd::state::off::insert)

    class sysmouse
    {
        using usable = e2::hids::mouse::button::click;

    public:
        constexpr static int numofbutton = 6;

        constexpr static int left      = e2::item(usable::left     );
        constexpr static int right     = e2::item(usable::right    );
        constexpr static int leftright = e2::item(usable::leftright);
        constexpr static int middle    = e2::item(usable::middle   );
        constexpr static int wheel     = e2::item(usable::wheel    );
        constexpr static int win       = e2::item(usable::win      );

        twod  coor = dot_mx;            // sysmouse: Cursor coordinates.
        bool  button[numofbutton] = {}; // sysmouse: Button states.

        bool  ismoved = faux;           // sysmouse: Movement through the cells.
        bool  shuffle = faux;           // sysmouse: Movement inside the cell.
        bool  doubled = faux;           // sysmouse: Double click.
        bool  wheeled = faux;           // sysmouse: Vertical scroll wheel.
        bool  hzwheel = faux;           // sysmouse: Horizontal scroll wheel.
        iota  wheeldt = 0;              // sysmouse: Scroll delta.

        uint32_t ctlstate = 0;

            bool operator !=(sysmouse const& m) const
        {
            bool result;
            if ((result = coor == m.coor))
            {
                for (int i = 0; i < numofbutton && result; i++)
                {
                    result &= button[i] == m.button[i];
                }
                result &= ismoved == m.ismoved
                       && doubled == m.doubled
                       && wheeled == m.wheeled
                       && hzwheel == m.hzwheel
                       && wheeldt == m.wheeldt;
            }
            return !result;
        }
    };

    class syskeybd
    {
    public:
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

        bool     down = faux;
        uint16_t repeatcount = 0;
        uint16_t virtcode = 0;
        uint16_t scancode = 0;
        wchar_t  character = 0;
        //char    ascii = 0;
        uint32_t ctlstate = 0;
        text     textline;
    };

    class mouse
    {
        using tail = netxs::datetime::tail<twod>;
        using idxs = std::vector<iota>;
        using mouse_event = e2::hids::mouse;

        constexpr static int total = sysmouse::numofbutton;
        constexpr static int first = sysmouse::left;
        constexpr static int midst = sysmouse::middle;
        constexpr static int other = sysmouse::right;
        constexpr static int third = sysmouse::wheel;
        constexpr static int extra = sysmouse::win;
        constexpr static int joint = sysmouse::leftright;

        constexpr static auto dragstrt = e2::group<total>(mouse_event::button::drag::start::any);
        constexpr static auto dragpull = e2::group<total>(mouse_event::button::drag::pull::any);
        constexpr static auto dragcncl = e2::group<total>(mouse_event::button::drag::cancel::any);
        constexpr static auto dragstop = e2::group<total>(mouse_event::button::drag::stop::any);
        constexpr static auto released = e2::group<total>(mouse_event::button::up::any);
        constexpr static auto pushdown = e2::group<total>(mouse_event::button::down::any);
        constexpr static auto sglclick = e2::group<total>(mouse_event::button::click::any);
        constexpr static auto dblclick = e2::group<total>(mouse_event::button::dblclick::any);
        constexpr static auto movement = mouse_event::move;
        constexpr static auto idleness = mouse_event::shuffle;
        constexpr static auto scrollup = mouse_event::scroll::up;
        constexpr static auto scrolldn = mouse_event::scroll::down;

    public:
        static constexpr iota none = -1; // mouse: No active buttons

        struct knob
        {
            testy<bool> pressed = { faux };
            bool        flipped = { faux };
            bool        dragged = { faux };
            bool        succeed = { true };
        };

        template<class LAW>
        auto fader(period spell)
        {
            //todo use current item's type: LAW<twod>
            return delta.fader<LAW>(spell);
        }

        twod   prime = dot_mx;  // mouse: System mouse cursor coordinates
        twod   coord = dot_mx;  // mouse: Relative mouse cursor coordinates
        //todo unify the mint=1/fps
        tail   delta = { 75ms, 4ms }; // mouse: History of mouse movements for a specified period of time
        bool   wheel = faux;
        bool   hzwhl = faux;
        iota   whldt = 0;
        bool   reach = faux;    // mouse: Has the event tree relay reached the mouse event target
        hint   cause = e2::any; // mouse: Current event id
        iota   index = none;    // mouse: Index of the active button. -1 if the buttons are not involed
        bool   nodbl = faux;    // mouse: Whether single click event processed (to prevent double clicks)
        iota   locks = 0;       // mouse: State of the captured buttons (bit field).
        id_t   swift = 0;       // mouse: Delegate's ID of the current mouse owner
        id_t   hover = 0;       // mouse: Hover control ID
        id_t   start = 0;       // mouse: Initiator control ID

        struct
        {
            moment fired;
            twod   coord;
        }
        stamp[sysmouse::numofbutton] = {}; // mouse: Recorded intervals between successive button presses to track double-clicks
        static constexpr period delay = 500ms;   // mouse: Double-click threshold

        knob   button[sysmouse::numofbutton];

        idxs  pressed_list;
        idxs  flipped_list;

        void update	(sysmouse& m)
        {
            //if (m.shuffle)
            //{
            //	//mouse.coord = m.coor;
            //	//action(idleness);
            //}
            //else
            {
                // Interpret button combinations
                if ((m.button[joint] = (m.button[first]         & m.button[other])
                                     | (  button[joint].pressed & m.button[first])
                                     | (  button[joint].pressed & m.button[other])))
                {
                    if (button[first].dragged)
                    {
                        button[first].dragged = faux;
                        action(dragcncl, first);
                    }
                    if (button[other].dragged)
                    {
                        button[other].dragged = faux;
                        action(dragcncl, other);
                    }
                }
                // In order to avoid single button tracking (Click, Pull, etc)
                button[first].succeed = !(m.button[joint] || button[joint].pressed);
                button[other].succeed = button[first].succeed;

                auto sysptr = std::begin(m.button);
                auto genptr = std::begin(  button);
                if (m.ismoved)
                {
                    delta.set(m.coor - prime);
                    for (auto i = 0; i < total; i++)
                    {
                        auto& genbtn = *genptr++;
                        auto& sysbtn = *sysptr++;
                        if (sysbtn)
                        {
                            if (genbtn.flipped)
                            {
                                genbtn.flipped = faux;
                                if (button[i].succeed)
                                {
                                    genbtn.dragged = true;
                                    action(dragstrt, i);
                                }
                            }
                            pressed_list.push_back(i);
                        }
                    }

                    //delta.set(m.coor - prime);

                    coord = m.coor;
                    prime = m.coor;

                    if (pressed_list.size())
                    {
                        for (auto i : pressed_list)
                        {
                            if (button[i].succeed) action(dragpull, i);
                        }
                        pressed_list.clear();
                    }

                    action(movement);

                    sysptr = std::begin(m.button);
                    genptr = std::begin(  button);
                }

                for (auto i = 0; i < total; i++)
                {
                    auto& genbtn = *genptr++;
                    auto& sysbtn = *sysptr++;

                    if ((genbtn.flipped = genbtn.pressed(sysbtn)))
                    {
                        flipped_list.push_back(i);
                    }
                    if (sysbtn)
                    {
                        pressed_list.push_back(i);
                    }
                }

                coord = m.coor;
#ifdef DEBUG_OVERLAY // Overlay needs current values for every frame
                wheel = m.wheeled;
                hzwhl = m.hzwheel;
                whldt = m.wheeldt;
#endif
                // Double clicks is a win32 console only story.
                // We catch them ourselves.
                //if (m.doubled && pressed_list.size())
                //{
                //	for (auto i : pressed_list)
                //	{
                //		action(dblclick, i);
                //	}
                //}
                //else if (m.wheeled)
                if (m.wheeled)
                {
#ifndef DEBUG_OVERLAY
                    wheel = m.wheeled;
                    hzwhl = m.hzwheel;
                    whldt = m.wheeldt;
#endif
                    action( m.wheeldt > 0 ? scrollup : scrolldn);
#ifndef DEBUG_OVERLAY
                    wheel = faux;
                    hzwhl = faux;
                    whldt = 0;
#endif
                }
                else
                {
                    for (auto i : flipped_list)
                    {
                        auto& b = button[i];
                        if (b.pressed)
                        {
                            action(pushdown, i);
                        }
                        else
                        {
                            if (b.dragged)
                            {
                                b.dragged = faux;
                                action(dragstop, i);
                            }
                            else
                            {
                                if (b.succeed) action(sglclick, i);
                                if (!nodbl)
                                {
                                    // Fire double-click if delay is not expired
                                    // and the same mouseposition
                                    auto& s = stamp[i];
                                    auto fired = tempus::now();
                                    if (fired - s.fired < delay
                                        && s.coord == coord)
                                    {
                                        s.fired = {}; // To avoid successive double-clicks if triple-click
                                        if (b.succeed) action(dblclick, i);
                                    }
                                    else
                                    {
                                        s.fired = fired;
                                        s.coord = coord;
                                    }
                                }
                            }
                            action(released, i);
                        }
                    }
                }
                if (flipped_list.size()) flipped_list.clear();
                if (pressed_list.size()) pressed_list.clear();
            }
        }
        template<class TT>
        void action (TT const& event_subset, iota _index)
        {
            index = _index;
            action(event_subset[index]);
            index = mouse::none;
        }
        void action (e2::type cause)
        {
            fire(cause);
        }

        virtual void fire(e2::type cause) = 0;

        // mouse: Initiator of visual tree informing about mouse enters/leaves
        template<bool ENTERED>
        bool direct(id_t asker)
        {
            if constexpr (ENTERED)
            {
                if (!start)
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
        // mouse: Is the mouse seized/captured?
        bool captured (id_t asker) const
        {
            return swift == asker;
        }
        // mouse: Seize specified mouse control
        bool capture (id_t asker)
        {
            if (!swift || swift == asker)
            {
                swift = asker;
                if (index != mouse::none) locks |= 1 << index;
                return true;
            }
            return faux;
        }
        // mouse: Release specified mouse control
        void release (bool force = true)
        {
            force = force || index == mouse::none;
            locks = force ? 0
                          : locks & ~(1 << index);
            if (!locks) swift = {};
        }
        //todo revise
        // mouse: Bit buttons. Used only for foreign mouse pointer in the gate (pro::input)
        iota buttons ()
        {
            iota bitfield = 0;
            for (auto i = 0; i < sysmouse::numofbutton; i++)
            {
                if (mouse::button[i].pressed) bitfield |= 1 << i;
            }
            return bitfield;
        }
    };

    class keybd
    {
    public:
        text        keystrokes;
        bool        down = faux;
        uint16_t    repeatcount = 0;
        uint16_t    virtcode = 0;
        uint16_t    scancode = 0;
        wchar_t     character = 0;
        e2::type    cause = e2::hids::keybd::any;
        e2::type    focus_got  = e2::form::notify::keybd::got;
        e2::type    focus_lost = e2::form::notify::keybd::lost;

        void update	(syskeybd& k)
        {
            virtcode    = k.virtcode;
            down        = k.down;
            repeatcount = k.repeatcount;
            scancode    = k.scancode;
            character   = k.character;
            keystrokes  = k.textline;

            fire_keybd();
        }

        virtual void fire_keybd() = 0;
    };

    class hids
        : public mouse,
          public keybd
    {
        using list = std::list<wptr<bell>>;

        bell&       owner;
        id_t        relay; // hids: Mouse routing call stack initiator
        core const& idmap; // hids: Area of the main form. Primary or relative region of the mouse coverage
        list        kb_focus; // hids: keyboard subscribers
        bool        alive; // hids: Whether event processing is complete
        //todo revise
        uint32_t ctlstate = 0;

    public:
        id_t const& id;    // hids: Owner/gear ID

        //todo unify
        rect slot; // slot for pro::maker and e2::createby

        bool kb_focus_taken = faux;

        enum modifiers : unsigned
        {
            SHIFT = 1 << 2,
            ALT   = 1 << 3,
            CTRL  = 1 << 4,
            RCTRL = 1 << 5,
            ANYCTRL = CTRL | RCTRL,
        };

        auto meta(unsigned ctl_key)
        {
            return hids::ctlstate & ctl_key;
        }
        auto meta()
        {
            return hids::ctlstate;
        }

        template<class T>
        hids(T& owner, core const& idmap)
            : relay { 0        },
              owner { owner    },
              id    { owner.id },
              idmap { idmap    },
              alive { faux     }
        { }
        ~hids()
        {
            if (auto last = bell::getref(mouse::hover))
            {
                last->SIGNAL(e2::release, e2::form::notify::mouse::leave, *this);
            }
            clear_kb_focus();
        }

        // hids: Stop handeling this event.
        void dismiss ()
        {
            alive = faux;
        }

        // hids: Whether event processing is complete.
        operator bool() const
        {
            return alive;
        }
        auto take(sysmouse& m)
        {
            ctlstate = m.ctlstate;
            mouse::update(m);
            return mouse::buttons();
        }
        void take(syskeybd& k)
        {
            ctlstate = k.ctlstate;
            keybd::update(k);
        }

        rect const& area() const { return idmap.area(); }

        template<e2::tier TIER, class T>
        void pass(sptr<T> object, twod const& offset, bool relative = faux)
        {
            if (object)
            {
                auto temp = coord;
                coord += offset;
                if (relative)
                {
                    object->SIGNAL(e2::request, e2::form::layout::local, coord);
                }
                object->bell::template signal<TIER>(mouse::cause, *this);
                coord = temp;
            }
        }
        void okay(bell& boss)
        {
            if (boss.id == relay)
            {
                if (mouse::hover != boss.id)
                {
                    if (auto last = bell::getref(mouse::hover))
                    {
                        last->SIGNAL(e2::release, e2::form::notify::mouse::leave, *this);
                    }
                    mouse::hover = boss.id;
                    mouse::start = 0;
                    boss.SIGNAL(e2::release, e2::form::notify::mouse::enter, *this);
                }
                boss.bell::template signal<e2::release>(mouse::cause, *this);
            }
        }
        void fire(e2::type cause)
        {
            alive = true;
            mouse::cause = cause;
            mouse::coord = mouse::prime;
            mouse::nodbl = faux;

            auto& offset = idmap.coor();
            if (mouse::swift)
            {
                auto next = bell::getref(mouse::swift);
                if (next) pass<e2::release>(next, offset, true);
                else      mouse::release();
            }
            else
            {
                owner.bell::template signal<e2::preview>(cause, *this);

                if (!alive) return;

                auto next = idmap.link(mouse::coord);
                if (next != id)
                {
                    relay = next;
                    pass<e2::preview>(bell::getref(next), offset, true);
                    relay = 0;

                    if (!alive) return;
                }

                owner.bell::template signal<e2::release>(cause, *this);
            }
        }
        void fire_keybd()
        {
            alive = true;
            owner.bell::template signal<e2::preview>(keybd::cause, *this);

            auto iter = kb_focus.begin();
            while (alive && iter != kb_focus.end())
            {
                if (auto next = iter++->lock())
                {
                    next->bell::template signal<e2::preview>(keybd::cause, *this);
                }
                else kb_focus.erase(std::prev(iter));
            }
        }
        void _add_kb_focus(sptr<bell> item)
        {
            kb_focus.push_back(item);
            item->bell::template signal<e2::release>(keybd::focus_got, *this);
        }
        bool remove_from_kb_focus(sptr<bell> item)
        {
            auto iter = kb_focus.begin();
            while (iter != kb_focus.end())
            {
                if (auto next = iter->lock())
                {
                    if (item == next)
                    {
                        next->bell::template signal<e2::release>(keybd::focus_lost, *this);
                        kb_focus.erase(iter);
                        return true;
                    }
                }
                iter++;
            }
            return faux;
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
                    next->bell::template signal<e2::release>(keybd::focus_lost, *this);
                }
                iter++;
                kb_focus.erase(std::prev(iter));
            }

            if (keep) _add_kb_focus(item);
        }
        void add_group_kb_focus_or_release_captured(sptr<bell> item)
        {
            if (!remove_from_kb_focus(item))
            {
                _add_kb_focus(item);
            }
        }
        void set_kb_focus(sptr<bell> item)
        {
            kb_focus_taken = true;
            if (hids::meta(ANYCTRL))
                add_group_kb_focus_or_release_captured(item);
            else
                add_single_kb_focus(item);
        }
        void clear_kb_focus()
        {
            auto iter = kb_focus.begin();
            while (iter != kb_focus.end())
            {
                if (auto next = iter->lock())
                {
                    next->bell::template signal<e2::release>(keybd::focus_lost, *this);
                }
                iter++;
                kb_focus.erase(std::prev(iter));
                //kb_focus.erase(iter);
            }
        }
        bool focus_taken()
        {
            return kb_focus_taken;
        }
    };

    //todo OMG!, make it in another way
    class skin
    {
        //todo revise
    public:
        //#define PROP_LIST                       \
        //X(brighter , "Highlighter modificator") \
        //X(shadower , "Darklighter modificator") \
        //X(shadow   , "Light Darklighter modificator") \
        //X(lucidity , "Global transparency")     \
        //X(selector , "Selection overlay")       \
        //X(bordersz , "Border size")
        //
        //#define X(a, b) a,
        //enum prop { PROP_LIST count };
        //#undef X
        //
        //#define X(a, b) b,
        //text description[prop::count] = { PROP_LIST };
        //#undef X
        //#undef PROP_LIST

        cell kb_colors;
        poly kb_grades;

        cell hi_colors;
        poly hi_grades;

        cell lo_colors;
        poly lo_grades;

        cell sh_colors;
        poly sh_grades;

        cell sl_colors;
        poly sl_grades;

        twod border = dot_11;
        iota opaque = 0xFF;

        template<class V>
        struct _globals
        {
            static skin global; //extn link: skin: shared gradients.
        };

        static void setup(tone::prop parameter, uint8_t value)
        {
            auto& global = _globals<void>::global;
            switch (parameter)
            {
                case tone::prop::kb_focus:
                    //todo is it necessary to .bga(value)?
                    global.kb_colors.bgc(rgba(0, 0xff, 0xff, value)).bga(value)
                                    .fgc(rgba(0, 0xff, 0xff, value)).fga(value);
                    global.kb_grades.recalc(global.kb_colors);
                    break;
                case tone::prop::brighter:
                    global.hi_colors.bgc(rgba(0xFF, 0xFF, 0xFF, value))
                                    .fgc(rgba(0xFF, 0xFF, 0xFF, value)).alpha(value);
                    global.hi_grades.recalc(global.hi_colors);
                    break;
                case tone::prop::shadower:
                    global.lo_colors.bgc(rgba(0, 0, 0, value)).bga(value);
                    global.lo_grades.recalc(global.lo_colors);
                    break;
                case tone::prop::shadow:
                    global.sh_colors.bgc(rgba(0, 0, 0, value)).bga(value);
                    global.sh_grades.recalc(global.sh_colors);
                    break;
                case tone::prop::selector:
                    global.sl_colors.txt(whitespace)
                        .bgc(rgba(0xFF, 0xFF, 0xFF, value)).bga(value);
                    global.sl_grades.recalc(global.sl_colors);
                    break;
                case tone::prop::lucidity:
                    global.opaque = value;
                    break;
                default:
                    break;
            }
        }
        static void setup(tone::prop parameter, twod const& size)
        {
            auto& global = _globals<void>::global;
            switch (parameter)
            {
                case tone::prop::bordersz:
                    global.border = size;
                    break;
                default:
                    break;
            }
        }

        // skin:: Return global brighter/shadower color (cell)
        static cell const& color(iota property)
        {
            auto& global = _globals<void>::global;
            switch (property)
            {
                case tone::prop::kb_focus:
                    return global.kb_colors;
                    break;
                case tone::prop::brighter:
                    return global.hi_colors;
                    break;
                case tone::prop::shadower:
                    return global.lo_colors;
                    break;
                case tone::prop::shadow:
                    return global.sh_colors;
                    break;
                case tone::prop::selector:
                    return global.sl_colors;
                    break;
                default:
                    return global.hi_colors;
            }
        }
        // skin:: Return global gradient for brighter/shadower
        static poly const& grade(iota property)
        {
            auto& global = _globals<void>::global;
            switch (property)
            {
                case tone::prop::kb_focus:
                    return global.kb_grades;
                    break;
                case tone::prop::brighter:
                    return global.hi_grades;
                    break;
                case tone::prop::shadower:
                    return global.lo_grades;
                    break;
                case tone::prop::shadow:
                    return global.sh_grades;
                    break;
                case tone::prop::selector:
                    return global.sl_grades;
                    break;
                default:
                    return global.hi_grades;
            }
        }
        // skin:: Return global border size
        static twod const& border_size()
        {
            auto& global = _globals<void>::global;
            return global.border;
        }
        // skin:: Return global transparency
        static iota const& shady()
        {
            auto& global = _globals<void>::global;
            return global.opaque;
        }
    };

    template<class V>
    skin skin::_globals<V>::global;

    class base
        : public bell, public std::enable_shared_from_this<base>
    {
        using wptr = std::weak_ptr<base>;

        //console: Dependency property container
        //         (can validate parameters before applying and forward it further).
        template<e2::type EVENT>
        class bind
        {
            using type = ARGTYPE(EVENT);

            bell& owner;
            type  value;

        public:
            bind(bell& master)
                : owner{ master }
            {
                owner.SUBMIT(e2::release, EVENT, new_value) { value = new_value; };
                owner.SUBMIT(e2::request, EVENT, empty) { empty = value; };
            }

            //bind: Get the const reference of the current value.
            type const& get() const
            {
                return value;
            }
            //bind: Set (preview then release) new value,
            //      override argument, and return the
            //      difference with old value.
            type set(type& new_value)
            {
                auto oldval = value;

                owner.SIGNAL(e2::preview, EVENT, new_value);
                if (value != new_value)
                {
                    owner.SIGNAL(e2::release, EVENT, new_value);
                }
                return value - oldval;
            }
            // bind: Dry run (preview then release) current value.
            type dry()
            {
                auto new_value = value;
                auto old_value = value;
                owner.SIGNAL(e2::preview, EVENT, new_value);
                owner.SIGNAL(e2::release, EVENT, new_value);
                // We should fire release even without changes of the value
                //if (value != new_value)
                //{
                //	owner.SIGNAL(e2::release, EVENT, new_value);
                //}
                return value - old_value;
            }

            type const& operator =  (type& newval) { set(newval); return value; }
            type const& operator += (type const& p) { auto tmp = value + p; set(tmp); return value; }
            operator type const& () const { return get(); }
        };

    public:
        constexpr static e2::type size_event = e2::form::layout::size;
        constexpr static e2::type move_event = e2::form::layout::move;

        bind<move_event> coor {*this };
        bind<size_event> size {*this };

        struct
        {
            bool visible = faux; // base status: Is the form intended for composing?
            bool exposed = faux; // base status: Is the form on top of the visual tree?
            bool invalid = true; // base status: It is necessary to redraw the form?
            bool wrecked = true; // base status: It is necessary to compose this form?
        }
        status;

        //todo unify
        struct
        {
            twod min{ 0,0 };
            twod max{ 1920,1080 };
        }
        limit;

        cell brush;
        bool glow = faux;
        tone colors;
        bool linked = faux; // Whether the size is tied to the size of the clients.
        wptr parent; // base: Parental visual tree weak-pointer.
        side oversize; // base: Oversize, margin.

        twod anchor; // base: Object balance point. Center point for any transform (on preview).

        hook kb_offer;

        auto This () { return shared_from_this(); }

        virtual void color(rgba const& fg_color, rgba const& bg_color)
        {
            base::brush.bgc(bg_color)
                       .fgc(fg_color)
                       .txt(whitespace);
        }
        auto& color()
        {
            return base::brush;
        }
        // base: Draw the form composition on the specified canvas.
        virtual void renderproc (face& parent_canvas)
        {
            if (base::brush.wdt())
                parent_canvas.fill([&](cell& c) { c.fusefull(base::brush); });
            //else
            //	parent_canvas.fill([&](cell& c) { c.link(bell::id); });
        }
        // base: Draw the glow over the form.
        virtual void postrender (face& parent_canvas)
        {
            //todo apply glow to the parent_canvas
            if (glow)
            {
                iota size = 5; // pro::panel: vertical gradient size
                iota step = 2; // pro::panel: vertical gradient step
                //cell shadow{ cell{}.vis(cell::highlighter) };
                //cell bright{ cell{}.vis(cell::darklighter) };
                auto shadow = rgba{0xFF000000};
                auto bright = rgba{0xFFffffff};

                //todo optimize - don't fill the head and foot twice
                auto area = parent_canvas.view();
                auto n = std::clamp(size, 0, area.size.y / 2) + 1;
                //auto n = std::clamp(size, 0, boss.base::size.get().y / 2) + 1;

                auto head = area;
                head.size.y = 1;
                auto foot = head;
                head.coor.y = area.coor.y + n - 2;
                foot.coor.y = area.coor.y + area.size.y - n + 1;

                for (int i = 1; i < n; i++)
                {
                    bright.alpha(i * step);
                    shadow.alpha(i * step);

                    parent_canvas.core::fill(head, [&](cell& c) { c.bgc().mix(bright); });
                    parent_canvas.core::fill(foot, [&](cell& c) { c.bgc().mix(shadow); });

                    head.coor.y--;
                    foot.coor.y++;
                }
            }

            //SIGNAL(e2::release, e2::form::upon::redrawn, parent_canvas); /// to draw the title and footer
        }

    protected:
        base()
        {
            base::brush.link(bell::id);

            SUBMIT(e2::release, e2::form::upon::attached, boss)
            {
                parent = boss;

                // Propagate form events up to the visual branch
                // Exec after all subscriptions.
                boss->SUBMIT_T(e2::release, e2::form::upevent::any, kb_offer, gear)
                {
                    if (auto parent_ptr = parent.lock())
                    {
                        if (gear.focus_taken())
                        {
                            parent_ptr->bell::expire(e2::release);
                        }
                        else
                        {
                            if (auto deed = parent_ptr->bell::protos<e2::release>())
                            {
                                //SIGNAL(e2::release, e2::form::kboffer, gear);
                                bell::signal<e2::release>(deed, gear);
                            }
                        }
                    }
                };

            };
            SUBMIT(e2::release, e2::form::upon::detached, boss)
            {
                kb_offer.reset();
            };

            // Propagate form events down to the visual branch
            // Exec after all subscriptions.
            SUBMIT(e2::release, e2::form::notify::any, gear)
            {
                if (auto parent_ptr = parent.lock())
                {
                    if (auto deed = bell::protos<e2::release>())
                    {
                        parent_ptr->bell::signal<e2::release>(deed, gear);
                    }
                }
                //strike();
            };

            // Validate the new size.
            SUBMIT(e2::preview, base::size_event, check_size)
            {
                check_size = std::clamp(check_size, limit.min, limit.max);
            };
            SUBMIT(e2::preview, e2::form::layout::show, notify)
            {
                showup(notify);
            };
            SUBMIT(e2::preview, e2::form::layout::hide, notify)
            {
                vanish(notify);
            };
            SUBMIT(e2::preview, e2::form::layout::strike, region)
            {
                //todo check child's region
                deface();
            };
            // Invalide old region (before resize).
            SUBMIT(e2::release, base::size_event, apply_size)
            {
                deface();
            };
            // Invalide old region (before moveto).
            SUBMIT(e2::release, base::move_event, apply_coor)
            {
                strike();
            };
            // Recursively calculate global coordinate.
            SUBMIT(e2::request, e2::form::layout::local, coor)
            {
                global(coor);
            };

            // Update form hilightning colors.
            SUBMIT(e2::preview, e2::form::state::color, color)
            {
                enamel(color);
            };
            SUBMIT(e2::request, e2::form::state::color, color)
            {
                color = colors;
            };
        }

    public:
        // base: Make the form visible.
        void showup (bool notify = true)
        {
            if (!status.visible)
            {
                status.visible = true;
                if (notify)
                {
                    SIGNAL(e2::release, e2::form::layout::show, notify);
                    strike();
                }
            }
        }
        // base: Hide the form.
        void vanish (bool notify = true)
        {
            if (status.visible)
            {
                status.visible = faux;
                if (notify)
                {
                    SIGNAL(e2::release, e2::form::layout::hide, notify);
                    strike();
                }
            }
        }
        // base: Return the rectangle of the canvas.
        rect square ()
        {
            auto& s = base::size.get();
            auto& c = base::coor.get();
            return { c, s };
        }
        // base: Check that point hits the canvas.
        bool inside (twod const& p)
        {
            auto& s = base::size.get();
            auto& c = base::coor.get();
            return s.inside(p - c);
        }
        // base: Check that point hits the only size (coor = zero).
        bool within (twod const& p)
        {
            auto& s = base::size.get();
            return s.inside(p);
        }
        // base: Mark the form and its parent's subtree as wrecked
        //       and had to be recomposed.
        //       If "just == faux" then recompose a subtree only.
        void strike ()
        {
            //todo why status.wrecked? why not status.invalid
            status.wrecked = true;
            //status.invalid = true;
            if (auto parent_ptr = parent.lock())
            {
                parent_ptr->SIGNAL(e2::preview, e2::form::layout::strike, square());
            }
        }
        // base: Mark the form as invalid and going to be redrawn.
        void deface ()
        {
            status.invalid = true;
            SIGNAL(e2::release, e2::form::upon::invalidated, status.invalid);
            strike();
        }
        // base: Move the form to the new place, and return delta.
        twod moveto (twod newcoor)
        {
            auto delta = base::coor.set(newcoor);
            if (delta)
            {
                SIGNAL(e2::release, e2::form::upon::moved, delta);
            }
            //todo strike set
            return delta;
        }
        // base: Move the canvas by the specified step.
        //       Return the coor delta.
        twod moveby(twod const& step)
        {
            auto delta = moveto(base::coor.get() + step);
            return delta;
        }
        // base: Resize the form, and return the size delta.
        twod resize (twod newsize)
        {
            auto delta = base::size.set(newsize);
            if (delta)
            {
                SIGNAL(e2::release, e2::form::upon::resized, delta);
            }
            //todo strike set
            return delta;
        }
        // base: Resize the form relative the center point.
        //       Return the offset of the center point.
        //       The object is responsible for correcting
        //       the center point during resizing.
        twod resize (twod newsize, twod point)
        {
            point -= base::coor.get();
            base::anchor = point;
            base::resize(newsize);
            return point - base::anchor;
        }
        // base: Retest current size, ask parent if it is linked.
        void reflow ()
        {
            auto parent_ptr = parent.lock();
            if (parent_ptr && parent_ptr->linked)
            {
                parent_ptr->base::reflow();
            }
            else
            {
                if (auto delta = base::size.dry())
                {
                    SIGNAL(e2::release, e2::form::upon::resized, delta);
                }
            }
        }
        // base: Resize the form by step, and return delta.
        twod sizeby (twod const& step)
        {
            auto delta = resize(base::size.get() + step);
            return delta;
        }
        // base: Resize and move the form, and return delta.
        rect extend (rect newloc)
        {
            return { moveto(newloc.coor), resize(newloc.size) };
        }
        // base: Remove the form from the visual tree.
        void detach ()
        {
            if (auto parent_ptr = parent.lock())
            {
                auto shadow = This();
                parent_ptr->SIGNAL(e2::preview, e2::form::proceed::detach, shadow);
                strike();
                //parent_strike();
            }
        }
        // base: Recursively calculate global coordinate.
        void global(twod& coor)
        {
            coor -= base::coor.get();
            if (auto parent_ptr = parent.lock())
            {
                parent_ptr->SIGNAL(e2::request, e2::form::layout::local, coor);
            }
        }

        void enamel(tone const& color)
        {
            colors = color;
            SIGNAL(e2::release, e2::form::state::color, colors);
        }
        // base: Invoke a lambda with parent as a parameter.
        // Usage example:
        //     toboss([&](auto& parent_ptr) { c.fuse(parent.brush); });
        template<class T>
        void toboss(T proc)
        {
            if (auto parent_ptr = parent.lock())
            {
                proc(*parent_ptr);
            }
        }
        // base: Set resize limits (min, max).
        //       Preserve current value if specified arg less than 0
        void limits(twod const& min_size, twod const& max_size = -dot_11)
        {
            limit.min = min_size.less(dot_00, limit.min, min_size);
            limit.max = max_size.less(dot_00, limit.max, max_size);
        }
        // base: Return current limits.
        auto& limits() const
        {
            return limit;
        }
        // base: Fire an event on yourself and pass it parent if not handled.
        // Usage example:
        //          base::riseup<e2::preview, e2::form::prop::header>(txt);
        template<e2::tier TIER, e2::type EVENT, class T>
        void riseup(T& data)
        {
            if (!SIGNAL(TIER, EVENT, data))
            {
                base::toboss([&](auto& boss) {
                    boss.base::template riseup<TIER, EVENT>(data);
                    });
            }
        }
        //todo bug: DO NOT SET OVERSIZE ON RESIZE: base::size.get() its WRONG
        // base: Update oversize info
        //void beyond(side const& minmax)
        //{
        //    auto& size = base::size.get();
        //    base::oversize.set(-std::min(0, minmax.l),
        //                        std::max(0, minmax.r - size.x),
        //                       -std::min(0, minmax.t),
        //                        0);
        //
        //    //return base::oversize.set(-std::min(0, minmax.l),
        //    //                           std::max(0, minmax.r - size.x),
        //    //                          -std::min(0, minmax.t),
        //    //                           0);
        //
        //    //base::oversize.l =-std::min(0, minmax.l);
        //    //base::oversize.t =-std::min(0, minmax.t);
        //    //base::oversize.r = std::max(0, minmax.r - size.x);
        //    //base::oversize.b = 0;
        //}
    };

    class form // console: Form with cached canvas.
        : public base
    {
        sptr<face> coreface;

    public:
        face& canvas; // form: Form bitmap.

        auto get_canvas() { return canvas.shared_from_this(); }

        form() : canvas{*(coreface = std::make_shared<face>())}
        {
            canvas.link(bell::id);
            SUBMIT(e2::release, base::size_event, new_sz) { canvas.size(new_sz); };
            SUBMIT(e2::release, base::move_event, new_xy) { canvas.move(new_xy); };
            SUBMIT(e2::request, e2::form::canvas, canvas) { canvas = coreface; };
        }

        // form: Set canvas default color and char as a whitespace (preserve all other attrs).
        virtual void color(rgba const& fg_color, rgba const& bg_color)
        {
            base::color(fg_color, bg_color);
            canvas.mark().bgc(bg_color)
                         .fgc(fg_color)
                         .txt(whitespace);
        }
        // form: Default render proc.
        virtual void redraw()
        {

        }
        // form: Draw the form composition on the specified canvas.
        virtual void renderproc (face& parent_canvas)
        {
            if (base::status.invalid)
            {
                canvas.wipe();
                base::renderproc(canvas);
                redraw();
            }
        }
        // form: Draw the form composition on the specified canvas.
        //       Executed right after the renderproc().
        virtual void postrender (face& parent_canvas)
        {
            if (base::status.invalid)
            {
                base::postrender(canvas);
                base::status.invalid = faux;
            }
            parent_canvas.plot(canvas);
        }
    };

    namespace pro // console: Template modules for the base class behavior extension.
    {
        // pro: Assign `using self = _class_name_;` in order to use FEATURE() marco.
        #define FEATURE(feature, object) feature<self> object{ *this }

        // pro: Provides shared storage for the states of type T::state_t.
        template<class T>
        class share
        {
            using list = std::list<typename T::sock>;

            T&   boss;
            list depo;

        public:
            share(T&&) = delete;
            share(T& boss) : boss{ boss }
            { }

            auto& operator [](bell::id_t hids_id)
            {
                for (auto& item : depo)
                {
                    if (item(hids_id))
                    {
                        return item;
                    }
                }
                auto& item = depo.emplace_back(hids_id, boss);
                return item;
            }
            auto& states()
            {
                return depo;
            }
            void remove(id_t hids_id)
            {
                auto item = depo.begin();
                while (item != depo.end())
                {
                    if ((*item)(hids_id))
                    {
                        depo.erase(item);
                        break;
                    }
                    ++item;
                }
            }
            //bool active()
            //{
            //	return depo.size();
            //}
        };

        // pro: Provides size-binding functionality for child objects
        //      after attaching to the parent.
        template<class T>
        class align
        {
            using gptr = wptr<bell>;

            T&   boss;
            rect last; // pro::align: Window size before the fullscreen has applied
            text head; // pro::align: Main window title the fullscreen has applied
            subs memo; // pro::align: Subscriptions on master activity
            id_t weak; // pro::align: Master id
            rect body; // pro::align: For current coor/size tracking

        public:
            auto seized(id_t master)
            {
                return weak == master;
            }
            void follow(id_t master)
            {
                if (auto gate_ptr = bell::getref(master))
                {
                    auto& gate = *gate_ptr;

                    rect area;
                    gate.SIGNAL(e2::request, e2::form::layout::size, area.size);
                    gate.SIGNAL(e2::request, e2::form::layout::move, area.coor);
                    last = boss.base::square();
                    area.coor -= dot_11;
                    area.size += dot_22;
                    body = {}; // In oder to unbind previous subscription if it is
                    boss.base::extend(area);
                    body = area;

                    text newhead;
                    gate.SIGNAL(e2::request, e2::form::prop::header, head);
                    boss.SIGNAL(e2::request, e2::form::prop::header, newhead);
                    gate.SIGNAL(e2::preview, e2::form::prop::header, newhead);

                    gate.SUBMIT_T(e2::release, e2::form::layout::size, memo, size)
                    {
                        body.size = size + dot_22;
                        boss.base::resize(body.size);
                    };
                    gate.SUBMIT_T(e2::release, e2::form::layout::move, memo, coor)
                    {
                        unbind();
                    };
                    gate.SUBMIT_T(e2::release, e2::dtor, memo, master_id)
                    {
                        unbind();
                    };

                    boss.SUBMIT_T(e2::release, e2::form::layout::size, memo, size)
                    {
                        if (body.size != size) unbind(faux);
                    };
                    boss.SUBMIT_T(e2::release, e2::form::layout::move, memo, coor)
                    {
                        if (body.coor != coor) unbind();
                    };

                    weak = master;
                    boss.SUBMIT_BYVAL_T(e2::release, e2::form::prop::header, memo, newhead)
                    {
                        if (auto gate_ptr = bell::getref(weak))
                        {
                            gate_ptr->SIGNAL(e2::preview, e2::form::prop::header, newhead);
                        }
                        else unbind();
                    };
                }
            }
            void unbind(bool restor_size = true)
            {
                if (memo.count())
                {
                    memo.clear();
                    if (auto gate_ptr = bell::getref(weak))
                    {
                        gate_ptr->SIGNAL(e2::preview, e2::form::prop::header, head);
                    }
                }
                if (restor_size) boss.base::extend(last); // Restore previous position
                weak = {};
            }

            ~align()
            {
                unbind(faux);
            }
            align(T&&) = delete;
            align(T& boss)
                : boss{ boss },
                  weak{      }
            {
                boss.base::linked = true;

                //boss.SUBMIT_T(e2::release, e2::form::client::rect, memo, area)
                //{
                //	boss.base::coor.set(area.coor);
                //	boss.base::size.set(area.size);
                //};
                //boss.SUBMIT_T(e2::release, e2::form::client::size, memo, size)
                //{
                //	boss.base::size.set(size);
                //};
                //boss.SUBMIT_T(e2::release, e2::form::client::coor, memo, coor)
                //{
                //	boss.base::coor.set(coor);
                //};
                //
                //boss.SUBMIT_T(e2::request, e2::form::client::rect, memo, area)
                //{
                //	area.coor = boss.base::coor.get();
                //	area.size = boss.base::size.get();
                //};
                //boss.SUBMIT_T(e2::request, e2::form::client::size, memo, size)
                //{
                //	size = boss.base::size.get();
                //};
                //boss.SUBMIT_T(e2::request, e2::form::client::coor, memo, coor)
                //{
                //	coor = boss.base::coor.get();
                //};

                //boss.SUBMIT_T(e2::release, e2::form::upon::attached, memo, parent_ptr)
                //{
                //	auto& gode = *parent_ptr;
                //	gode.SUBMIT_T(e2::release, e2::form::client::rect, lock, client_area)
                //	{
                //		boss.base::coor.set(client_area.coor);
                //		boss.base::size.set(client_area.size);
                //	});
                //
                //	rect client_area;
                //	gode.SIGNAL(e2::request, e2::form::client::rect, client_area);
                //	boss.base::extend(client_area);
                //});
                //boss.SUBMIT_T(e2::release, e2::form::upon::detached, memo, parent)
                //{
                //	lock.reset();
                //});
            }
            /// Ask the client about the new size (the client can override the size) and return delta
            //auto accord(twod& newsize)
            //{
            //	return boss.base::size.set(newsize);
            //}
        };

        //todo deprecated: here is only one focus is possible
        ////pro: Provides functionality to capture focus.
        //template<class T>
        //class focus
        //{
        //	T&   boss;
        //	subs memo;
        //
        //public:
        //	hook holder; //todo used in pane
        //
        //	focus(T&&) = delete;
        //	focus(T& boss) : boss{ boss }
        //	{
        //		//boss.SUBMIT_T(e2::release, e2::form::upon::attached, memo, parent)
        //		//{
        //		//	this->parent = parent;
        //		//});
        //		///<summary> focus: Got focus and apply it for the visual tree behind. </summary>
        //		boss.SUBMIT_T(e2::preview, e2::form::focus::got, memo, holder)
        //		{
        //			if (auto parent_ptr = boss.base::parent.lock())
        //			//if (boss.base::parent)
        //			{
        //				//boss.base::parent->signal(e2::preview, e2::form::focus::got, holder);
        //				parent_ptr->SIGNAL(e2::preview, e2::form::focus::got, holder);
        //			}
        //			boss.SIGNAL(e2::release, e2::form::focus::got, holder);
        //		};
        //		///<summary> focus: Lost focus and apply it for the visual tree behind. </summary>
        //		boss.SUBMIT_T(e2::preview, e2::form::focus::lost, memo, holder)
        //		{
        //			boss.SIGNAL(e2::release, e2::form::focus::lost, holder);
        //			if (auto parent_ptr = boss.base::parent.lock())
        //			{
        //				//boss.base::parent->signal(e2::preview, e2::form::focus::lost, holder);
        //				parent_ptr->SIGNAL(e2::preview, e2::form::focus::lost, holder);
        //			}
        //		};
        //	}
        //
        //	///<summary> focus: Forced focus seizing. </summary>
        //	void seize ()
        //	{
        //		boss.CHANGE(e2::form::focus::got, holder, e2::form::focus::lost, holder);
        //	}
        //};

        //pro: Provides keyboard focus functionality.
        //template<class T>
        //class focus
        //{
        //	T&   boss;
        //	subs memo;
        //
        //	iota size = 0;
        //
        //public:
        //	focus(T&&) = delete;
        //	focus(T& boss) : boss{ boss }
        //	{
        //		//// focus: Got focus and apply it for the visual tree behind
        //		//boss.SUBMIT_T(e2::preview, e2::form::focus::got, memo, holder)
        //		//{
        //		//	if (auto parent_ptr = boss.base::parent.lock())
        //		//	//if (boss.base::parent)
        //		//	{
        //		//		//boss.base::parent->signal(e2::preview, e2::form::focus::got, holder);
        //		//		parent_ptr->SIGNAL(e2::preview, e2::form::focus::got, holder);
        //		//	}
        //		//	boss.SIGNAL(e2::release, e2::form::focus::got, holder);
        //		//};
        //		//// focus: Lost focus and apply it for the visual tree behind
        //		//boss.SUBMIT_T(e2::preview, e2::form::focus::lost, memo, holder)
        //		//{
        //		//	boss.SIGNAL(e2::release, e2::form::focus::lost, holder);
        //		//	if (auto parent_ptr = boss.base::parent.lock())
        //		//	{
        //		//		//boss.base::parent->signal(e2::preview, e2::form::focus::lost, holder);
        //		//		parent_ptr->SIGNAL(e2::preview, e2::form::focus::lost, holder);
        //		//	}
        //		//};
        //	}
        //
        //	//// focus: Forced focus seizing
        //	//void seize ()
        //	//{
        //	//	boss.CHANGE(e2::form::focus::got, holder, e2::form::focus::lost, holder);
        //	//}
        //};

        // pro: Provides functionality for runtime animation (time-based).
        template<class T>
        class robot
        {
            using subs = std::map<id_t, hook>;
            static constexpr id_t noid = std::numeric_limits<id_t>::max();

            T&   boss;
            subs memo;

        public:
            robot(T&&) = delete;
            robot(T& boss) : boss{ boss }
            { }

            // aminate: Every timer tick, yield the
            //          delta from the flow and, if delta,
            //          Call the proc (millisecond precision).
            template<id_t ID = noid, class P, class S>
            //todo revise ref and val - (S& flow, P proc)
            //void actify (S& flow, P proc)
            void actify (S flow, P proc)
            {
                auto& token = memo[ID];
                auto handler = [&, proc, flow](auto p)
                {
                    auto now = datetime::round<iota>(p);
                    if (auto data = flow(now))
                    {
                        proc(data.value());
                    }
                    else
                    {
                        pacify(ID);
                    }
                };
                boss.SUBMIT_TV(e2::general, e2::timer::any, token, handler);
                boss.SIGNAL(e2::release, e2::form::animate::start, ID);
            }
            // animate: Optional proceed every timer tick,
            //          yield the delta from the flow and,
            //          if delta, Call the proc (millisecond precision).
            template<id_t ID = noid, class P, class S>
            void actify (std::optional<S> flow, P proc)
            //void actify (std::optional<S>& flow, P proc)
            {
                if (flow)
                {
                    actify<ID>(flow.value(), proc);
                }
            }
            // aminate: Cancel tick activity.
            void pacify (id_t id = noid)
            {
                if (id == noid) memo.clear();   // Stop all animations
                else            memo.erase(id);
                boss.SIGNAL(e2::release, e2::form::animate::stop, id);
            }
            // aminate: Check activity by id.
            bool active (id_t id)
            {
                return memo.find(id) != memo.end(); //todo use ::contains (C++20)
            }
            // aminate: Check any activity.
            operator bool ()
            {
                return !memo.empty();
            }
        };

        // pro: Invokes specified proc after timeout.
        template<class T>
        class timer
        {
            using subs = std::map<id_t, hook>;
            static constexpr id_t noid = std::numeric_limits<id_t>::max();

            T&   boss;
            subs memo;

        public:
            timer(T&&) = delete;
            timer(T& boss) : boss{ boss }
            { }

            // pro::timer: Start countdown
            template<id_t ID = noid, class P>
            void actify (period timeout, P lambda)
            {
                auto  alarm = tempus::now() + timeout;
                auto& token = memo[ID];
                auto handler = [&, timeout, lambda, alarm](auto now)
                {
                    if (now > alarm)
                    {
                        if (!lambda(ID)) pacify(ID);
                    }
                };
                boss.SUBMIT_TV(e2::general, e2::timer::any, token, handler);
                //auto id = ID;
                //boss.SIGNAL(e2::release, e2::form::animate::start, id);
            }
            // pro::timer: Cancel timer ('id=noid' for all).
            void pacify (id_t id = noid)
            {
                if (id == noid) memo.clear();   // Stop all timers
                else            memo.erase(id);
                //boss.SIGNAL(e2::release, e2::form::animate::stop, id);
            }
            // aminate: Check activity by id.
            bool active (id_t id)
            {
                return memo.find(id) != memo.end(); //todo use ::contains (C++20)
            }
            // aminate: Check any activity.
            operator bool ()
            {
                return !memo.empty();
            }
        };

        // pro: Provides functionality for manipulating objects
        //      with a frame structure.
        template<class T>
        class frame
        {
            T&       boss;
            subs     memo;
            subs     link;
            robot<T> robo;

        public:
            frame(T&&) = delete;
            frame(T& boss)
                :	boss { boss },
                    robo { boss }
            {
                boss.SUBMIT_T(e2::release, e2::form::upon::attached, memo, parent)
                {
                    parent->SUBMIT_T(e2::preview, e2::form::global::lucidity, link, alpha)
                    {
                        boss.SIGNAL(e2::preview, e2::form::global::lucidity, alpha);
                    };
                    parent->SUBMIT_T(e2::preview, e2::form::layout::convey, link, convey_data)
                    {
                        convey(convey_data.delta, convey_data.stuff);
                    };
                    parent->SUBMIT_T(e2::preview, e2::form::layout::shift, link, delta)
                    {
                        boss.base::coor += delta;
                    };

                    parent->SUBMIT_T(e2::preview, e2::form::upon::detached, link, p)
                    {
                        frame::link.clear();
                    };

                    //todo verify release
                    //parent->SUBMIT_T(e2::release, e2::form::layout::expose, parent_memo, p)
                    //{
                    //	expose();
                    //});
                };

                boss.SUBMIT_T(e2::preview, e2::form::layout::appear, memo, newpos)
                {
                    appear(newpos);
                };
            };

            // frame: Fly to the specified position.
            void appear(twod const& target)
            {
                auto screen = boss.base::square();
                auto oldpos = screen.coor;
                auto newpos = target - screen.size / 2;;

                auto path = newpos - oldpos;
                iota time = SWITCHING_TIME;
                auto func = constlinearAtoB<twod>(path, time, now<iota>());

                robo.pacify();
                robo.actify(func, [&](twod& x)
                    {
                        boss.base::moveby(x);
                    });
            }

            // frame: Search for a non-overlapping form position in
            //        the visual tree along a specified direction.
            rect bounce (rect const& block, twod const& dir)
            {
                rect result = block.rotate(dir);
                auto parity = std::abs(dir.x) > std::abs(dir.y);

                for (auto xy : { parity, !parity })
                {
                    auto ray = result;
                    ray.coor[xy] += ray.size[xy];
                    ray.size[xy] = dir[xy] > 0 ? std::numeric_limits<int>::max()
                                               : std::numeric_limits<int>::min();

                    if (auto shadow = ray.trunc(boss.base::size))
                    {
                        auto direct = shadow.rotate(dir);
                        auto nearby = direct.coor[xy] + direct.size[xy];

                        foreach(boss.branch, boss.status.is.visible, [&](auto item)
                                {
                                    if (auto s = shadow.clip(item->square()))
                                    {
                                        auto next = dir[xy] > 0 ? s.coor[xy] : -(s.coor[xy] + s.size[xy]);
                                        if (next < nearby) nearby = next;
                                    }
                                });

                        result.size[xy] = (dir[xy] > 0 ? nearby : -nearby) - result.coor[xy];
                    }
                }

                return result;
            }
            // frame: Move the form no further than the parent canvas.
            void convey (twod const& delta, rect const& boundary)//, bool notify = true)
            {
                auto r0 = boss.base::square();
                if (delta && r0.clip(boundary))
                {
                    auto r1 = r0;
                    auto r2 = boundary;
                    r1.coor -= r2.coor;

                    auto c = r1.rotate(-delta);
                    auto s = r2.size;
                    auto o = delta.less(dot_00, dot_00, dot_11);
                    if ((s + o).twod::inside(c.coor))
                    {
                        c.coor = std::clamp(c.coor + delta, dot_00, s);
                        auto newcoor = c.normalize().coor + r2.coor;
                        boss.base::coor.set(newcoor);
                    }
                    else if (!r2.clip(r0))
                    {
                        boss.base::coor += delta;
                    }
                }
            }
            // frame: Check if it is under the rest, and moves it to the
            //        top of the visual tree.
            //        Return "true" if it is NOT under the rest.
            bool expose (bool subsequent = faux)
            {
                if (auto parent_ptr = boss.parent.lock())
                {
                    parent_ptr->SIGNAL(e2::release, e2::form::layout::expose, boss);
                }
                return boss.status.exposed;
            }
            // frame: Place the form in front of the visual tree
            //        among neighbors.
            void bubble ()
            {
                if (auto parent_ptr = boss.parent.lock())
                {
                    parent_ptr->SIGNAL(e2::release, e2::form::layout::bubble, boss);
                }
            }
        };

        // pro: Form generator functionality.
        template<class T>
        class maker
        {
            T&   boss;
            subs memo;
            cell mark;

            struct slot_t
            {
                rect slot;
                twod step;
                twod init;
                bool ctrl = faux;
            };
            std::map<id_t, slot_t> slots;

            void check_modifiers(hids& gear)
            {
                auto& data = slots[gear.id];
                auto state = !!gear.meta(hids::ANYCTRL);
                if (data.ctrl != state)
                {
                    data.ctrl = state;
                    boss.SIGNAL(e2::preview, e2::form::layout::strike, data.slot);
                }
            }
            void handle_init(hids& gear)
            {
                if (gear.capture(boss.bell::id))
                {
                    auto& data = slots[gear.id];
                    auto& slot = data.slot;
                    auto& init = data.init;
                    auto& step = data.step;

                    data.ctrl = gear.meta(hids::ANYCTRL);
                    slot.coor = init = step = gear.mouse::coord;
                    slot.size = dot_00;
                    boss.SIGNAL(e2::preview, e2::form::layout::strike, slot);
                    gear.dismiss();
                }
            }
            void handle_pull(hids& gear)
            {
                if (gear.captured(boss.bell::id))
                {
                    check_modifiers(gear);
                    auto& data = slots[gear.id];
                    auto& slot = data.slot;
                    auto& init = data.init;
                    auto& step = data.step;

                    step += gear.mouse::delta.get();
                    slot.coor = std::min(init, step);
                    slot.size = std::max(std::abs(step - init), dot_00);
                    boss.SIGNAL(e2::preview, e2::form::layout::strike, slot);
                    gear.dismiss();
                }
            }
            void handle_drop(hids& gear)
            {
                if (gear.captured(boss.bell::id))
                {
                    slots.erase(gear.id);
                    gear.dismiss();
                    gear.release();
                }
            }
            void handle_stop(hids& gear)
            {
                if (gear.captured(boss.bell::id))
                {
                    check_modifiers(gear);
                    auto& data = slots[gear.id];
                    if (data.slot)
                    {
                        gear.slot = data.slot;
                        boss.SIGNAL(e2::preview, e2::form::proceed::createby, gear);
                    }
                    slots.erase(gear.id);
                    gear.dismiss();
                    gear.release();
                }
            }

        public:
            maker(T&&) = delete;
            maker(T& boss)
                :	boss { boss },
                    mark { skin::color(tone::selector) }
            {
                using drag = e2::hids::mouse::button::drag;

                boss.SUBMIT_T(e2::preview, e2::hids::keybd::any, memo, gear)
                {
                    if (gear.captured(boss.bell::id)) check_modifiers(gear);
                };

                //todo unify - args... + template?
                //middle button
                boss.SUBMIT_T(e2::preview, drag::start::middle, memo, gear)
                {
                    handle_init(gear);
                };
                boss.SUBMIT_T(e2::release, drag::pull::middle, memo, gear)
                {
                    handle_pull(gear);
                };
                boss.SUBMIT_T(e2::release, drag::cancel::middle, memo, gear)
                {
                    handle_drop(gear);
                };
                boss.SUBMIT_T(e2::release, drag::stop::middle, memo, gear)
                {
                    handle_stop(gear);
                };

                //todo unify
                //right button
                boss.SUBMIT_T(e2::release, drag::start::right, memo, gear)
                {
                    handle_init(gear);
                };
                boss.SUBMIT_T(e2::release, drag::pull::right, memo, gear)
                {
                    handle_pull(gear);
                };
                boss.SUBMIT_T(e2::release, drag::cancel::right, memo, gear)
                {
                    handle_drop(gear);
                };
                boss.SUBMIT_T(e2::release, drag::stop::right, memo, gear)
                {
                    handle_stop(gear);
                };

                boss.SUBMIT_T(e2::release, e2::form::upon::redrawn, memo, canvas)
                {
                    for (auto const& [key, data] : slots)
                    {
                        if (auto slot = data.slot)
                        {
                            auto area = canvas.area().clip<true>(slot);
                            if (data.ctrl)
                            {
                                area.coor -= dot_11;
                                area.size += dot_22;

                                // Calc average bg brightness
                                auto count = 0;
                                auto light = 0;
                                auto sumfx = [&](cell& c)
                                {
                                    count++;
                                    auto& clr = c.bgc();
                                    light += clr.chan.r + clr.chan.g + clr.chan.b;
                                };
                                auto head = area;
                                head.size.y = 1;
                                canvas.each(head, sumfx);
                                auto b = count ? light / (count * 3) : 0;

                                // Draw the frame
                                auto mark = skin::color(tone::kb_focus);
                                auto fill = [&](cell& c) { c.fuse(mark); };
                                canvas.cage(area, dot_11, fill);

                                auto size = para(ansi::fgc(b > 130 ? 0xFF000000
                                                                   : 0xFFFFFFFF) + "capture area: " + slot.str());
                                canvas.cup(area.coor);
                                canvas.output(size);
                            }
                            else
                            {
                                auto temp = canvas.view();
                                canvas.view(area);
                                canvas.fill(area, [&](cell& c) { c.fuse(mark); c.und(faux); });
                                canvas.blur(10);
                                canvas.view(temp);
                            }
                        }
                    }
                };
            }
        };

        // pro: The text caret controller.
        template<class T>
        class caret
        {
            T&     boss;
            subs   memo;
            subs   conf; // caret: Configuration subscriptions.
            bool   live; // caret: Should the caret be drawn.
            bool   done; // caret: Is the caret already drawn.
            rect   body; // caret: Caret position.
            period step; // caret: Blink interval.
            moment next; // caret: Time of next blinking.
            //iota form; // caret: Set caret style (box, line, bar)

        public:
            caret(T&&) = delete;
            caret(T& boss)
                : boss{ boss },
                  live{ faux },
                  done{ faux },
                  body{ dot_00, dot_11 }, // Caret is always one cell size (see the term::wall definition)
                  step{ BLINK_PERIOD }
            {
                boss.SUBMIT_T(e2::request, e2::config::intervals::blink, conf, req_step)
                {
                    req_step = step;
                };
                boss.SUBMIT_T(e2::general, e2::config::intervals::blink, conf, new_step)
                {
                    step = new_step;
                };
                boss.SUBMIT_T(e2::preview, e2::config::intervals::blink, conf, new_step)
                {
                    step = new_step;
                };
            }

            operator bool() const { return memo.count(); }

            // caret: Set caret position.
            void coor(twod const& coor)
            {
                if (body.coor != coor)
                {
                    reset();
                    body.coor = coor;
                }
            }
            // caret: Set caret position.
            auto coor() const
            {
                return body.coor;
            }
            // caret: Force to show the caret.
            void reset()
            {
                live = faux;
                next = {};
            }
            // caret: Enable the caret.
            void show()
            {
                //reset();
                if (!*this)
                {
                    done = faux;
                    live = faux;
                    boss.SUBMIT_T(e2::general, e2::timer::tick, memo, timestamp)
                    {
                        if (timestamp > next)
                        {
                            next = timestamp + step;
                            live = !live;
                            //log("tick ", live ? "1":"0", " item: ", this);
                            boss.SIGNAL(e2::preview, e2::form::layout::strike, body);
                        }
                    };
                    boss.SUBMIT_T(e2::release, e2::form::upon::redrawn, memo, canvas)
                    {
                        done = live;
                        if (live)
                        {
                            auto field = canvas.core::view();
                            auto point = body;
                            point.coor += field.coor + boss.base::coor.get();
                            if (auto area = field.clip(point))
                            {
                                //log("blink inside ", this);
                                canvas.fill(area, [](cell& c) { c.und(!c.und()); });
                            }

                            //log("blink ", this);
                        }
                    };
                }
            }
            // caret: Disable the caret.
            void hide()
            {
                if (*this)
                {
                    memo.clear();
                    if (done)
                    {
                        boss.SIGNAL(e2::preview, e2::form::layout::strike, body);
                        done = faux;
                    }
                }
            }
        };

        // pro: Textify the telemetry data for debugging purpose.
        template<class T>
        class debug
        {
            T&   boss;
            subs memo;

            #define PROP_LIST                     \
            X(total_size   , "total sent"       ) \
            X(proceed_ns   , "rendering time"   ) \
            X(render_ns    , "stdout time"      ) \
            X(frame_size   , "frame size"       ) \
            X(focused      , "focus"            ) \
            X(win_size     , "win size"         ) \
            X(key_code     , "key virt"         ) \
            X(key_scancode , "key scan"         ) \
            X(key_character, "key char"         ) \
            X(key_pressed  , "key push"         ) \
            X(ctrl_state   , "controls"         ) \
            X(mouse_pos    , "mouse coord"      ) \
            X(mouse_wheeldt, "wheel delta"      ) \
            X(mouse_hzwheel, "H wheel"          ) \
            X(mouse_vtwheel, "V wheel"          ) \
            X(mouse_btn_1  , "left button"      ) \
            X(mouse_btn_2  , "right button"     ) \
            X(mouse_btn_3  , "left+right combo" ) \
            X(mouse_btn_4  , "middle button"    ) \
            X(mouse_btn_5  , "4th button"       ) \
            X(mouse_btn_6  , "5th button"       ) \
            X(last_event   , "event"            )

            //X(key_repeat   , "key copy"         )
            //X(menu_id      , "menu id"          )

            #define X(a, b) a,
            enum prop { PROP_LIST count };
            #undef X

            #define X(a, b) b,
            text description[prop::count] = { PROP_LIST };
            #undef X
            #undef PROP_LIST

            cell alerts;
            cell stress;
            page status;

            struct
            {
                period    render = period::zero();
                period    output = period::zero();
                iota      frsize = 0;
                long long totals = 0;
                //bool      onhold = faux; // info: Indicator that the current frame has been successfully STDOUT
                iota      number = 0;    // info: Current frame number
            }
            track; // site: Textify the telemetry data for debugging purpose

            void shadow()
            {
                for (int i = 0; i < prop::count; i++)
                {
                    status[i].ease();
                }
            }

        public:
            bool bypass = faux;
            void update(bool focus_state)
            {
                shadow();
                status[prop::last_event].set(stress) = "focus";
                status[prop::focused].set(stress) = focus_state ? "active" : "lost";
            }
            void update(twod const& new_size)
            {
                shadow();
                status[prop::last_event].set(stress) = "size";

                status[prop::win_size].set(stress) =
                    std::to_string(new_size.x) + " x " +
                    std::to_string(new_size.y);
            }
            void update(period const& watch, iota delta)
            {
                track.output = watch;
                track.frsize = delta;
                track.totals+= delta;
            }
            void update(moment const& timestamp)
            {
                track.render = tempus::now() - timestamp;
            }

            debug(T&&) = delete;
            debug(T& boss) : boss{ boss }
            {
                //todo use skin
                stress = cell{}.fgc(whitelt);
                alerts = cell{}.fgc(rgba{ 0xFFd0d0FFu });

                status.style.wrp(wrap::on).jet(bias::left).rlf(feed::rev);
                status.current().locus.cup(dot_00).cnl(2);

                auto maxlen = 0_sz;
                for (auto& desc : description)
                {
                    maxlen = std::max(maxlen, desc.size());
                }
                iota attr = 0;
                for (auto& desc : description)
                {
                    status += " " + utf::adjust(desc, maxlen, " ", true) + " "
                        + ansi::idx(attr++).nop().nil().eol();
                }

                boss.SUBMIT_T(e2::release, e2::form::upon::redrawn, memo, canvas)
                {
                    status[prop::render_ns].set(track.output > 12ms ? alerts : stress) =
                        utf::adjust(utf::format(track.output.count()), 11, " ", true) + "ns";

                    status[prop::proceed_ns].set(track.render > 12ms ? alerts : stress) =
                        utf::adjust(utf::format (track.render.count()), 11, " ", true) + "ns";

                    status[prop::frame_size].set(stress) =
                        utf::adjust(utf::format(track.frsize), 7, " ", true) + " bytes";

                    status[prop::total_size].set(stress) =
                        utf::format(track.totals) + " bytes";

                    track.number++;
                    canvas.output(status);
                };

                //boss.SUBMIT_T(e2::release, e2::debug, owner::memo, track)
                //{
                //	status[prop::render_ns].set(track.output > 12ms ? alerts : stress) =
                //		utf::adjust(utf::format(track.output.count()), 11, " ", true) + "ns";
                //
                //	status[prop::proceed_ns].set(track.render > 12ms ? alerts : stress) =
                //		utf::adjust(utf::format (track.render.count()), 11, " ", true) + "ns";
                //
                //	status[prop::frame_size].set(stress) =
                //		utf::adjust(utf::format(track.frsize), 7, " ", true) + " bytes";
                //});

                //boss.SUBMIT_T(e2::release, e2::term::size, owner::memo, newsize)
                //{
                //	shadow();
                //	status[prop::last_event].set(stress) = "size";
                //
                //	status[prop::win_size].set(stress) =
                //		std::to_string(newsize.x) + " x " +
                //		std::to_string(newsize.y);
                //});

                boss.SUBMIT_T(e2::release, e2::term::focus, memo, focusstate)
                {
                    update(focusstate);
                    boss.base::strike(); // to update debug info
                };
                boss.SUBMIT_T(e2::release, e2::form::layout::size, memo, newsize)
                {
                    update(newsize);
                };

                boss.SUBMIT_T(e2::preview, e2::hids::mouse::any, memo, gear)
                {
                    if (bypass) return;
                    shadow();
                    auto& m = gear;
                    status[prop::last_event].set(stress) = "mouse";
                    status[prop::mouse_pos ].set(stress) =
                        (m.coord.x < 10000 ? std::to_string(m.coord.x) : "-") + " : " +
                        (m.coord.y < 10000 ? std::to_string(m.coord.y) : "-") ;

                    for (int btn = 0; btn < sysmouse::numofbutton; btn++)
                    {
                        auto& state = status[prop::mouse_btn_1 + btn].set(stress);

                        state = m.button[btn].pressed ? "pressed" : "";
                        if (m.button[btn].flipped)
                            state += state.length() ? " | flipped" : "flipped";

                        if (m.button[btn].dragged)
                            state += state.length() ? " | dragged" : "dragged";

                        state += state.length() ? "" : "idle";
                    }
                    status[prop::mouse_wheeldt].set(stress) = std::to_string(m.whldt);
                    status[prop::mouse_hzwheel].set(stress) = m.hzwhl ? "active" : "idle";
                    status[prop::mouse_vtwheel].set(stress) = m.wheel ? "active" : "idle";
                    status[prop::ctrl_state   ].set(stress) = "0x" + utf::to_hex(m.meta());
                };

                //boss.SUBMIT_T(e2::release, e2::term::menu, memo, iface)
                //{
                //	//shadow();
                //	status[prop::last_event].set(stress) = "UI";
                //	status[prop::menu_id].set(stress) = "UI:" + std::to_string(iface);
                //};

                boss.SUBMIT_T(e2::release, e2::term::key, memo, gear)
                {
                    shadow();
                    auto& k = gear;
#ifdef KEYLOG
                    log("debug fired ", k.character);
#endif
                    status[prop::last_event   ].set(stress) = "key";
                    status[prop::key_pressed  ].set(stress) = k.down ? "pressed" : "idle";
                    status[prop::ctrl_state   ].set(stress) = "0x" + utf::to_hex(k.ctlstate );
                    status[prop::key_code     ].set(stress) = "0x" + utf::to_hex(k.virtcode );
                    status[prop::key_scancode ].set(stress) = "0x" + utf::to_hex(k.scancode );
                    status[prop::key_character].set(stress) = "0x" + utf::to_hex(k.character);
                    //status[prop::key_repeat   ].set(stress) = std::to_string(k.repeatcount);

                    if (!k.character && k.textline.length())
                    {
                        auto t = k.textline;
                        for (auto i = 0; i < 0x20; i++)
                        {
                            utf::change(t, text{ (char)i }, "^" + utf::to_utf_from_code(i + 0x40));
                        }
                        utf::change(t, text{ (char)0x7f }, "\\x7F");
                        utf::change(t, text{ (char)0x20 }, "\\x20");
                        status[prop::key_character].set(stress) = t;
                    }
                };

                //boss.SUBMIT_T(e2::release, e2::term::focus, owner::memo, f)
                //{
                //	shadow();
                //	status[prop::last_event].set(stress) = "focus";
                //	status[prop::focused].set(stress) = f ? "active" : "lost";
                //});

                boss.SUBMIT_T(e2::release, e2::term::error, memo, e)
                {
                    shadow();
                    status[prop::last_event].set(stress) = "error";
                    throw;
                };
            }
        };

        // pro: Provides functionality for the title support.
        template<class T>
        class title
        {
            T&   boss;
            subs memo;
            page logo; // title: Owner's caption
            text name; // title: Preserve original title

            #define PROP_LIST                    \
            X(head, "Window title." )            \
            X(foot, "Window status.")
            //X(body, "Window title properties." )

            #define X(a, b) a,
            enum prop { PROP_LIST count };
            #undef X

            #define X(a, b) b,
            text description[prop::count] = { PROP_LIST };
            #undef X
            #undef PROP_LIST

        public:
            bool live = true;

            title(T&&) = delete;
            title(T& boss) : boss{ boss }
            {
                //logo += ansi::idx(prop::body).nop()
                //      + ansi::wrp(wrap::off).mgr(1).mgl(1)
                //      + ansi::rlf(feed::fwd).jet(bias::left).cup(dot_00)
                //      + ansi::idx(prop::head) + ansi::nop()
                //      + ansi::rlf(feed::rev).jet(bias::right).cup(dot_00)
                //      + ansi::idx(prop::foot) + ansi::nop()
                //      + ansi::rlf(feed::fwd).jet(bias::left).cup(dot_00).mgr(0).mgl(0);
                logo += ansi::cup(dot_00)
                      + ansi::wrp(wrap::off).rtl(rtol::ltr).rlf(feed::fwd).jet(bias::left).mgr(1).mgl(1)
                      + ansi::idx(prop::head) + ansi::nop()
                      + ansi::cup(dot_00).rlf(feed::rev).jet(bias::right)
                      + ansi::idx(prop::foot);// + ansi::nop()
                      //+ ansi::rlf(feed::fwd).jet(bias::left).cup(dot_00).mgr(0).mgl(0);

                boss.SUBMIT_T(e2::release, e2::form::upon::redrawn, memo, canvas)
                {
                    if (live) canvas.output(logo);
                };
                boss.SUBMIT_T(e2::preview, e2::form::prop::header, memo, newtext)
                {
                    header(newtext);
                };
                boss.SUBMIT_T(e2::preview, e2::form::prop::footer, memo, newtext)
                {
                    footer(newtext);
                };
                //boss.SUBMIT_T(e2::preview, e2::form::prop::params, memo, newtext)
                //{
                //    params(newtext);
                //};

                boss.SUBMIT_T(e2::request, e2::form::prop::header, memo, curtext)
                {
                    curtext = name;
                };

                boss.SUBMIT_T(e2::request, e2::form::state::header, memo, caption)
                {
                    caption = header();
                };
                boss.SUBMIT_T(e2::request, e2::form::state::footer, memo, caption)
                {
                    caption = footer();
                };
                //boss.SUBMIT_T(e2::request, e2::form::state::params, memo, caption)
                //{
                //    caption = params();
                //};
            }

            auto& titles() const
            {
                return logo;
            }
            auto& header()
            {
                return logo[prop::head];
            }
            auto& footer()
            {
                return logo[prop::foot];
            }
            //auto& params()
            //{
            //    return logo[prop::body];
            //}
            void header(view newtext)
            {
                name = newtext;
                auto& textline = header();
                textline = newtext;
                textline.style.rtl_or(rtol::ltr);
                textline.style.rlf_or(feed::fwd);
                textline.style.wrp_or(wrap::off);
                textline.style.jet_or(bias::left);
                textline.link(boss.id);
                boss.SIGNAL(e2::release, e2::form::state::header, textline);
            }
            void footer(view newtext)
            {
                auto& textline = footer();
                textline = newtext;
                textline.style.rtl_or(rtol::ltr);
                textline.style.rlf_or(feed::rev);
                textline.style.wrp_or(wrap::off);
                textline.style.jet_or(bias::right);
                textline.link(boss.id);
                boss.SIGNAL(e2::release, e2::form::state::footer, textline);
            }
            //void params(view newtext)
            //{
            //    auto& textline = params();
            //    textline = newtext;
            //    textline.link(boss.id);
            //    boss.SIGNAL(e2::release, e2::form::state::params, textline);
            //}
        };

        // pro: Provides functionality for the scene objects manipulations.
        template<class T>
        class scene
        {
            class node // console: Helper-class for the pro::scene. Adapter for the object that going to be attached to the scene.
            {
                struct ward
                {
                    enum states
                    {
                        unused_hidden, // 00
                        unused_usable, // 01
                        active_hidden, // 10
                        active_usable, // 11
                        count
                    };

                    para title[states::count];
                    cell brush[states::count];
                    para basis;
                    bool usable = faux;
                    bool active = faux;
                    tone color;

                    operator bool ()
                    {
                        return basis.size();
                    }
                    void set(para const& caption)
                    {
                        basis = caption;
                        basis.decouple();
                        recalc();
                    }
                    auto& get()
                    {
                        return title[(active ? 2 : 0) + usable];
                    }
                    void recalc()
                    {
                        brush[active_hidden] = skin::color(color.active);
                        brush[active_usable] = skin::color(color.active);
                        brush[unused_hidden] = skin::color(color.passive);
                        brush[unused_usable] = skin::color(color.passive);

                        brush[unused_usable].bga(brush[unused_usable].bga() << 1);

                        int i = 0;
                        for (auto& label : title)
                        {
                            auto& c = brush[i++];
                            label = basis;
                            label.decouple();

                            //todo unify clear formatting/aligning in header
                            label.locus.kill();
                            label.style.rst();
                            label.lyric->each([&](auto& a) { a.meta(c); });
                        }
                    }
                };

                using sptr = netxs::sptr<base>;

                ward header;

            public:
                rect region;
                sptr object;
                id_t obj_id;

                node(sptr item)
                    : object{ item }
                {
                    auto& inst = *item;
                    obj_id = inst.bell::id;

                    inst.SUBMIT(e2::release, e2::form::layout::size, size)
                    {
                        region.size = size;
                    };
                    inst.SUBMIT(e2::release, e2::form::layout::move, coor)
                    {
                        region.coor = coor;
                    };
                    inst.SUBMIT(e2::release, e2::form::state::mouse, state)
                    {
                        header.active = state;
                    };
                    inst.SUBMIT(e2::release, e2::form::state::header, caption)
                    {
                        header.set(caption);
                    };
                    inst.SUBMIT(e2::release, e2::form::state::color, color)
                    {
                        header.color = color;
                    };

                    inst.SIGNAL(e2::request, e2::form::layout::size,  region.size);
                    inst.SIGNAL(e2::request, e2::form::layout::move,  region.coor);
                    inst.SIGNAL(e2::request, e2::form::state::mouse,  header.active);
                    inst.SIGNAL(e2::request, e2::form::state::header, header.basis);
                    inst.SIGNAL(e2::request, e2::form::state::color,  header.color);

                    header.recalc();
                }
                // node: Check equality.
                bool equals(id_t id)
                {
                    return obj_id == id;
                }
                // node: Draw the anchor line func and return true
                //       if the mold is outside the canvas area.
                void fasten(face& canvas)
                {
                    auto window = canvas.area();
                    auto origin = window.size / 2;
                    //auto origin = twod{ 6, window.size.y - 3 };
                    auto offset = region.coor - window.coor;
                    auto center = offset + (region.size / 2);
                    header.usable = window.overlap(region);

                    auto& grade = skin::grade(header.active ? header.color.active
                                                            : header.color.passive);
                    auto pset = [&](twod const& p, uint8_t k)
                    {
                        //canvas[p].fuse(grade[k], obj_id, p - offset);
                        //canvas[p].fuse(grade[k], obj_id);
                        canvas[p].link(obj_id).bgc().mix_one(grade[k].bgc());
                    };
                    window.coor = dot_00;
                    netxs::online(window, origin, center, pset);
                }
                // node: Output the title to the canvas.
                void enlist(face& canvas)
                {
                    if (header)
                    {
                        auto& title = header.get();
                        canvas.output(title);
                        canvas.eol();
                    }
                }
                // node: Visualize the underlying object.
                void render(face& canvas)
                {
                    canvas.render(*object);
                }

                void postrender(face& canvas)
                {
                    object->postrender(canvas);
                }
            };

            class list // console: Helper-class for the pro::scene. List of objects that can be reordered, etc.
            {
                std::list<sptr<node>> items;

                template<class D>
                auto search(D head, D tail, id_t id)
                {
                    if (items.size())
                    {
                        auto test = [id](auto& a) { return a->equals(id); };
                        return std::find_if(head, tail, test);
                    }
                    return tail;
                }

            public:
                operator bool () { return items.size(); }

                void append(sptr<base> item)
                {
                    items.push_back(std::make_shared<node>(item));
                }
                // Draw backpane for spectators.
                void prerender(face& canvas)
                {
                    for (auto& item : items)          item->fasten(canvas); // Draw strings
                    for (auto& item : items)          item->render(canvas); // Draw shadows
                    canvas.cup(dot_00).jet(bias::right);
                    for (auto& item : items)          item->enlist(canvas); // Draw a list of objects
                }
                // Draw windows.
                void render(face& canvas)
                {
                    for (auto& item : items)          item->fasten(canvas);
                    canvas.cup(dot_00).jet(bias::left);
                    for (auto& item : reverse(items)) item->enlist(canvas);
                    for (auto& item : items)          item->render(canvas);
                }
                // Draw spectator's mouse pointers.
                void postrender(face& canvas)
                {
                    for (auto& item : items)          item->postrender(canvas);
                }

                rect remove(id_t id)
                {
                    rect area;
                    auto head = items.begin();
                    auto tail = items.end();
                    auto item = search(head, tail, id);

                    if (item != tail)
                    {
                        area = (**item).region;
                        items.erase(item);
                    }

                    return area;
                }
                rect bubble(id_t id)
                {
                    auto head = items.rbegin();
                    auto tail = items.rend();
                    auto item = search(head, tail, id);

                    if (item != head && item != tail)
                    {
                        auto& area = (**item).region;
                        if (!area.clip((**std::prev(item)).region))
                        {
                            auto shadow = *item;
                            items.erase(std::next(item).base());

                            while (--item != head
                                && !area.clip((**std::prev(item)).region))
                            {
                            }

                            items.insert(item.base(), shadow);
                            return area;
                        }
                    }

                    return rect_00;
                }
                rect expose(id_t id)
                {
                    auto head = items.rbegin();
                    auto tail = items.rend();
                    auto item = search(head, tail, id);

                    if (item != head && item != tail)
                    {
                        auto shadow = *item;
                        items.erase(std::next(item).base());
                        items.push_back(shadow);
                        return shadow->region;
                    }

                    return rect_00;
                }
                auto rotate_next()
                {
                    items.push_back(items.front());
                    items.pop_front();
                    return items.back();
                }
                auto rotate_prev()
                {
                    items.push_front(items.back());
                    items.pop_back();
                    return items.back();
                }
            };

            using proc = drawfx;
            using time = moment;
            using area = std::vector<rect>;

            T&   boss;
            subs memo;
            area edges; // scene: wrecked regions history
            proc paint; // scene: Render all child items to the specified canvas
            list items; // scene: Child visual tree
            list users; // scene: Scene spectators

        public:
            scene(T&&) = delete;
            scene(T& boss)
                : boss { boss }
            {
                paint = [&](face& canvas, page const& titles) -> bool
                {
                    if (edges.size())
                    {
                        canvas.wipe(boss.id);
                        canvas.output(titles);
                        //todo revise
                        users.prerender (canvas); // Draw backpane for spectators
                        items.render    (canvas); // Draw objects of the world
                        users.postrender(canvas); // Draw spectator's mouse pointers
                        return true;
                    }
                    else return faux;
                };

                boss.SUBMIT_T(e2::preview, e2::form::proceed::detach, memo, item)
                {
                    auto& inst = *item;
                    denote(items.remove(inst.id));
                    denote(users.remove(inst.id));
                    inst.SIGNAL(e2::release, e2::form::upon::detached, boss.This());
                };
                boss.SUBMIT_T(e2::preview, e2::form::layout::strike, memo, region)
                {
                    denote(region);
                };
                boss.SUBMIT_T(e2::release, e2::form::layout::bubble, memo, inst)
                {
                    auto region = items.bubble(inst.bell::id);
                    denote(region);
                };
                boss.SUBMIT_T(e2::release, e2::form::layout::expose, memo, inst)
                {
                    auto region = items.expose(inst.bell::id);
                    denote(region);
                    inst.status.exposed = region;
                };

                ///// Pass the paint procedure to custom client drawing
                //boss.SUBMIT_T(e2::request, e2::form::proceed::render, owner::memo, empty_fx)
                //{
                //	empty_fx = paint;
                //});

                // pro::scene: Proceed request for available objects (next)
                boss.SUBMIT_T(e2::request, e2::form::proceed::attach, memo, next)
                {
                    if (items)
                        if (auto next_ptr = items.rotate_next())
                            next = next_ptr->object;
                };
                // pro::scene: Proceed request for available objects (prev)
                boss.SUBMIT_T(e2::request, e2::form::proceed::detach, memo, prev)
                {
                    if (items)
                        if (auto prev_ptr = items.rotate_prev())
                            prev = prev_ptr->object;
                };
            }

            // scene: .
            void redraw()
            {
                boss.SIGNAL(e2::release, e2::form::proceed::render, paint);
                edges.clear();
            }
            // scene: Mark dirty region.
            void denote(rect const& updateregion)
            {
                if (updateregion)
                {
                    edges.push_back(updateregion);
                }
            }
            // scene: Create a new item of the specified subtype
            //        and attach it to the scene.
            template<class S, class ...Args>
            auto attach(Args&&... args)
            {
                auto item = boss.indexer<bell>::create<S>(std::forward<Args>(args)...);
                items.append(item);
                item->SIGNAL(e2::release, e2::form::upon::attached, boss.base::This()); // Send creator
                return item;
            }
            // scene: Create a new user of the specified subtype
            //        and invite him to the scene.
            template<class S, class ...Args>
            auto invite(Args&&... args)
            {
                auto user = boss.indexer<bell>::create<S>(std::forward<Args>(args)...);
                users.append(user);

                user->SIGNAL(e2::release, e2::form::upon::attached, boss.base::This()); // Send creator

                //todo unify
                tone color{ tone::brighter, tone::shadow};
                user->SIGNAL(e2::preview, e2::form::state::color, color);

                return user;
            }
        };

        // pro: Perform graceful shutdown functionality. LIMIT in seconds, ESC_THRESHOLD in milliseconds.
        template<class T>
        class guard
        {
            constexpr static e2::type QUIT_MSG = e2::term::quit;
            constexpr static int ESC_THRESHOLD = 500; // Double escape threshold in ms

            T&     boss;
            subs   memo;
            bool   wait; // Ready to close
            moment stop; // Timeout for single Esc
            text   desc = "exit after preclose";

        public:
            guard(T&&) = delete;
            guard(T& boss)
                : boss { boss },
                  wait { faux }
            {
                // Suspected early completion
                boss.SUBMIT_T(e2::release, e2::term::preclose, memo, pre_close)
                {
                    if ((wait = pre_close))
                    {
                        stop = tempus::now() + std::chrono::milliseconds(ESC_THRESHOLD);
                    }
                };

                // Double escape catcher
                boss.SUBMIT_T(e2::general, e2::timer::tick, memo, timestamp)
                {
                    if (wait && (timestamp > stop))
                    {
                        wait = faux;
                        auto shadow = boss.This();
                        boss.SIGNAL(e2::release, QUIT_MSG, desc);
                        memo.clear();
                    }
                };
            }
        };

        // pro: Perform graceful shutdown functionality. LIMIT in seconds, ESC_THRESHOLD in milliseconds.
        template<class T>
        class watch
        {
            constexpr static e2::type EXCUSE_MSG = e2::hids::mouse::button::down::any;
            constexpr static e2::type QUIT_MSG   = e2::quit;
            //todo unify
            constexpr static int LIMIT = 60 * 5; // Idle timeout in seconds

            T&     boss;
            subs   memo;
            hook   pong; // Alibi subsciption token
            hook   ping; // Zombie check countdown token
            moment stop; // Timeout for zombies
            text   desc = "no mouse clicking events";

        public:
            watch(T&&) = delete;
            watch(T& boss)
                : boss { boss }
            {
                stop = tempus::now() + std::chrono::seconds(LIMIT);

                // No mouse events watchdog
                boss.SUBMIT_T(e2::preview, EXCUSE_MSG, pong, timestamp)
                {
                    stop = tempus::now() + std::chrono::seconds(LIMIT);

                    //doubt.reset();
                    //alibi.reset();
                };

                boss.SUBMIT_T(e2::general, e2::timer::tick, ping, timestamp)
                {
                    if (tempus::now() > stop)
                    {
                        auto shadow = boss.This();
                        boss.SIGNAL(e2::release, QUIT_MSG, desc);
                        ping.reset();
                        memo.clear();
                    }
                };
            }
        };

        // pro: Provides functionality related to keyboard input.
        template<class T>
        class keybd
        {
            subs memo;
            hook accept_kbd;
            iota clients = 0;
            //bool active = faux;
            T& boss;

        public:
            bool focusable = true;

            keybd(T&&) = delete;
            keybd(T& boss)
                : boss{ boss }
            {
                using bttn = e2::hids::mouse::button;

                boss.SUBMIT_T(e2::release, bttn::click::left, memo, gear)
                {
                    // Propagate throughout nested objects by base::
                    gear.kb_focus_taken = faux;
                    boss.SIGNAL(e2::release, e2::form::upevent::kboffer, gear);

                    //gear.set_kb_focus(boss.This());
                    if (gear.focus_taken()) gear.dismiss();

                    //if (!square.size.inside(gear.coord))
                    //{
                    //	auto center = square.coor + (square.size / 2);
                    //	bell::getref(gear.id)->
                    //		SIGNAL(e2::release, e2::form::layout::shift, center);
                    //}
                };

                // pro::keybd: Notify form::state::kbfocus when the number of clients is positive.
                boss.SUBMIT_T(e2::release, e2::form::notify::keybd::got, memo, gear)
                {
                    //if (!highlightable || gear.begin_inform(boss.bell::id))
                    {
                        if (!clients++)
                        {
                            //auto active = true;
                            boss.SIGNAL(e2::release, e2::form::state::keybd, true);
                        }
                    }
                };
                // pro::keybd: Notify form::state::active_kbd when the number of clients is zero.
                boss.SUBMIT_T(e2::release, e2::form::notify::keybd::lost, memo, gear)
                {
                    //if (!highlightable || gear.end_inform(boss.bell::id))
                    {
                        if (!--clients)
                        {
                            //auto active = faux;
                            boss.SIGNAL(e2::release, e2::form::state::keybd, faux);
                        }
                    }
                };
                boss.SUBMIT_T(e2::request, e2::form::state::keybd, memo, state)
                {
                    state = !!clients;
                };
                boss.SUBMIT_T(e2::preview, e2::hids::keybd::any, memo, gear)
                {
                    #ifdef KEYLOG
                    log("keybd fired virtcode: ", gear.virtcode,
                                      " chars: ", utf::debase(gear.keystrokes),
                                       " meta: ", gear.meta());
                    #endif

                    boss.SIGNAL(e2::release, e2::hids::keybd::any, gear);
                };
            };

            // pro::keybd: Subscribe on keybd offers.
            void accept(bool value)
            {
                if (value)
                {
                    boss.SUBMIT_T(e2::release, e2::form::upevent::kboffer, accept_kbd, gear)
                    {
                        if (!gear.focus_taken())
                        {
                            gear.set_kb_focus(boss.This());
                            boss.bell::expire(e2::release);
                        }
                    };
                }
                else
                {
                    accept_kbd.reset();
                }
            }
        };

        // pro: Provides functionality related to mouse interaction.
        template<class T>
        class mouse
        {
            subs memo;
            iota clients = 0;
            bool active = faux;

        public:
            bool highlightable = faux;

            mouse(T&&) = delete;
            mouse(T& boss)
            {
                // pro::mouse: Forward preview to all parents.
                boss.SUBMIT_T(e2::preview, e2::hids::mouse::any, memo, gear)
                {
                    auto& offset = boss.base::coor.get();
                    gear.pass<e2::preview>(boss.parent.lock(), offset);

                    if (gear) gear.okay(boss);
                    else      boss.bell::expire(e2::preview);
                };

                // pro::mouse: Forward all not expired mouse events to all parents.
                boss.SUBMIT_T(e2::release, e2::hids::mouse::any, memo, gear)
                {
                    if (gear && !gear.locks)
                    {
                        auto& offset = boss.base::coor.get();
                        gear.pass<e2::release>(boss.parent.lock(), offset);
                    }
                };

                // pro::mouse: Notify form::state::active when the number of clients is positive.
                boss.SUBMIT_T(e2::release, e2::form::notify::mouse::enter, memo, gear)
                {
                    if (!highlightable || gear.direct<true>(boss.bell::id))
                    {
                        if (!clients++)
                        {
                            active = true;
                            boss.SIGNAL(e2::release, e2::form::state::mouse, active);
                        }
                    }
                };
                // pro::mouse: Notify form::state::active when the number of clients is zero.
                boss.SUBMIT_T(e2::release, e2::form::notify::mouse::leave, memo, gear)
                {
                    if (!highlightable || gear.direct<faux>(boss.bell::id))
                    {
                        if (!--clients)
                        {
                            active = faux;
                            boss.SIGNAL(e2::release, e2::form::state::mouse, active);
                        }
                    }
                };
                boss.SUBMIT_T(e2::request, e2::form::state::mouse, memo, state)
                {
                    state = active;
                };
            }
            operator bool()
            {
                return active;
            }
        };

        // pro: Provides functionality related to keyboard interaction.
        template<class T>
        class input : public hids
        {
            subs memo;
            face xmap;

        public:
            iota push = 0; // Mouse pressed buttons bits (Used only for foreign mouse pointer in the gate)

            input(T&&) = delete;
            input(T& boss)
                : hids{ boss, xmap }
            {
                boss.SUBMIT_T(e2::release, e2::form::layout::size, memo, newsize)
                {
                    xmap.size(newsize);
                };
                boss.SUBMIT_T(e2::release, e2::form::layout::move, memo, newcoor)
                {
                    xmap.move(newcoor);
                };
                boss.SUBMIT_T(e2::release, e2::term::mouse, memo, mousestate)
                {
                    push = hids::take(mousestate);
                    boss.strike();
                };
                boss.SUBMIT_T(e2::release, e2::term::key, memo, keybdstate)
                {
                    hids::take(keybdstate);
                    boss.strike();
                };
            };

            // pro::input: Return read-only canvas grid reference.
            auto& freeze()
            {
                return xmap.pick();
            }
        };
    }

    class host // console: World internals.
        : public base
    {
        using self = host;
#ifdef DEMO
        FEATURE(pro::watch, zombi); // host: Zombie protection
#endif /// DEMO
        FEATURE(pro::robot, robot); // host: Amination controller
        FEATURE(pro::keybd, keybd); // host: Keyboard controller
        FEATURE(pro::mouse, mouse); // host: Mouse controller
        //FEATURE(pro::focus, focus); // host: Focus controller
        FEATURE(pro::scene, scene); // host: Scene controller
        //FEATURE(pro::caret, caret); // host: Cursor controller

        using tick = quartz<reactor, e2::type>;
        using hndl = std::function<void(view)>;

        tick synch; // host: Frame rate synchronizator
        iota frate; // host: Frame rate value
        hndl close; // host: Quit procedure

    public:
        // host: Create a new item of the specified subtype and attach it.
        template<class T, class ...Args>
        auto attach(Args&&... args)
        {
            return scene.attach<T>(std::forward<Args>(args)...);
        }

        //todo unify
        template<class T, class ...Args>
        auto invite(Args&&... args)
        {
            return scene.invite<T>(std::forward<Args>(args)...);
        }

    protected:
        host(hndl exit_proc)
            : synch(router(e2::general), e2::timer::tick),
              frate{ 0 },
              close{ exit_proc }
        {
            using bttn = e2::hids::mouse::button;

            keybd.accept(true); // Subscribe on keybd offers
            //focus.seize();

            SUBMIT(e2::general, e2::timer::tick, timestamp)
            {
                scene.redraw();
            };

            //test
            //SUBMIT(e2::preview, bttn::click::left, gear)
            //{
            //	static iota i = 0;
            //	text data = "click " + std::to_string(i++) + "\n";
            //	SIGNAL_GLOBAL(e2::debug, data);
            //};

            SUBMIT(e2::release, bttn::click::right, gear)
            {
                //auto newpos = gear.mouse.coord + gear.xview.coor;
                SIGNAL(e2::general, e2::form::global::ctxmenu, gear.coord);
            };

            SUBMIT(e2::release, bttn::drag::start::left, gear)
            {
                if (gear.capture(bell::id))
                {
                    robot.pacify();
                    gear.dismiss();
                }
            };
            SUBMIT(e2::release, bttn::drag::pull::left, gear)
            {
                if (gear.captured(bell::id))
                {
                    auto data = cube{ gear.mouse::delta.get(), gear.area() };
                    SIGNAL(e2::preview, e2::form::layout::convey, data);
                    gear.dismiss();
                }
            };
            SUBMIT(e2::release, bttn::drag::cancel::left, gear)
            {
                if (gear.captured(bell::id))
                {
                    gear.release();
                    gear.dismiss();
                }
            };
            SUBMIT(e2::release, bttn::drag::stop::left, gear)
            {
                if (gear.captured(bell::id))
                {
                    auto boundary = gear.area();
                    gear.release();
                    robot.actify(gear.mouse::fader<quadratic<twod>>(2s), [&, boundary](auto& x)
                                 {
                                     auto data = cube{ x, boundary };
                                     SIGNAL(e2::preview, e2::form::layout::convey, data);
                                 });
                    gear.dismiss();
                }
            };
            SUBMIT(e2::general, e2::timer::fps, fps)
            {
                if (fps > 0)
                {
                    frate = fps;
                    synch.ignite(frate);
                }
                else if (fps == -1)
                {
                    fps = frate;
                }
                else
                {
                    synch.cancel();
                }
            };
            SUBMIT(e2::release, e2::quit, reason)
            {
                if (close)
                {
                    close(reason);
                }
            };

            //SUBMIT(e2::release, bttn::click::left, p)
            //{
            //	focus.seize();
            //});

            //SUBMIT(e2::release, e2::hids::keybd::any, gear)
            //{
            //	//if (gear.keybd.scancode == 1 || gear.keybd.scancode == 68)
            //	//{
            //	//	//SIGNAL(e2::general, e2::quit, exit_codes::nocode);
            //	//	SIGNAL(e2::general, e2::quit, "Escape pressed.");
            //	//	bell::expire(e2::release);
            //	//}
            //});
        }

        ~host()
        {
            synch.cancel();

            //todo why it is never called?
            //reject();
        }
    };

    class link
    {
        using work = std::thread;
        using lock = std::recursive_mutex;
        using cond = std::condition_variable_any;

        bell&     owner; // pipe: Boss.
        xipc      canal; // pipe: Data highway.
        work      input; // pipe: Reading thread.
        cond      synch; // pipe: Thread sync cond variable.
        lock      mutex; // pipe: Thread sync mutex.
        bool      alive; // pipe: Working loop state.
        bool      ready; // pipe: To avoid spuritous wakeup (cv).
        bool      focus; // pipe: Terminal window focus state.
        iota      iface; // pipe: Platform specific UI code.
        sysmouse  mouse; // pipe: Mouse state.
        syskeybd  keybd; // pipe: Keyboard state.
        bool      close; // pipe: Pre closing condition.
        text      chunk; // pipe: The next received chunk of data input.
        text      total; // pipe: Accumulated unparsed input.

        void reader()
        {
            log("link: std_input thread started");
            while (auto yield = canal->recv())
            {
                //text tmp;
                //tmp += yield;
                //utf::change(tmp, "\033", "^");
                //log("link: receive\n", tmp);

                std::lock_guard guard{ mutex };

                chunk.resize(yield.length());
                std::copy(yield.begin(), yield.end(), chunk.data());

                ready = true;
                synch.notify_one();
            }

            if (alive)
            {
                log("link: signaling to close read channel ", canal);
                //alive = faux;
                //canal->shut(); // Terminate all blocking calls.
                owner.SIGNAL(e2::release, e2::term::quit, "link: read channel is closed");
                log("link: sig to close read channel complete", canal);
            }
            log("link: std_input thread is going to close");
        }

    public:
        link(bell& boss, xipc sock)
            : owner { boss },
              canal { sock },
              alive { true },
              ready { faux },
              focus { faux },
              close { faux },
              iface { 0    }
        { }

        ~link()
        {
            canal->shut(); // Terminate all blocking calls.
            if (input.joinable())
            {
                input.join();
            }
            log("link: std_input thread joined");
        }

        void output (view buffer)
        {
            canal->send(buffer);
        }

        void session(text title)
        {
            auto is_digit = [](auto c) { return c >= '0' && c <= '9'; };
            std::unique_lock guard{ mutex };

            input = std::thread([&] { reader(); });

            if (title.size()) output(ansi::tag(title));

            while ((void)synch.wait(guard, [&] { return ready; }), alive)
            {
                ready = faux;

                total += chunk;
                //todo why?
                //todo separate commands and keypress
                //
                // commands are:
                // - esc
                // - ctl keys and Fns
                // - mouse tracking - '\e[< Ctrl; Px; Py M' '\e[< Ctrl; Px; Py m'    M - pressed, m - released
                //	Ctrl:	7654 3210
                //	              ||└---
                //                |└----
                //	              └----- Ctrl
                // - cursor

                view strv = total;

                #ifdef DEMO
                if (close)
                {
                    close = faux;
                    owner.SIGNAL(e2::release, e2::term::preclose, close);
                    if (strv.front() == '\x1b') // two consecutive escapes
                    {
                        log("\t - two consecutive escapes: \n\tstrv:        ", strv);

                        owner.SIGNAL(e2::release, e2::term::quit, "pipe two consecutive escapes");
                        return;
                    }
                }
                #endif /// DEMO
                //int g = 0;


                //todo unify (it is just a proof of concept)
                while (auto len = strv.size())
                {
                    auto pos = 0_sz;
                    bool unk = faux;

                    if (strv.at(0) == '\x1b')
                    {
                        ++pos;

                        #ifdef DEMO
                        if (pos == len) // the only one esc
                        {
                            close = true;
                            total = strv;
                            log("\t - preclose: ", canal);
                            owner.SIGNAL(e2::release, e2::term::preclose, close);
                            break;
                        }
                        else if (strv.at(pos) == '\x1b') // two consecutive escapes
                        {
                            total.clear();
                            log("\t - two consecutive escapes: ", canal);
                            owner.SIGNAL(e2::release, e2::term::quit, "pipe2: two consecutive escapes");
                            break;
                        }
                        #else
                        if (pos == len) // the only one esc
                        {
                            ///  Pass Esc
                            keybd.textline = strv.substr(0, 1);
                            owner.SIGNAL(e2::release, e2::term::key, keybd);
                            total.clear();
                            //strv = total;
                            break;
                        }
                        else if (strv.at(pos) == '\x1b') // two consecutive escapes
                        {
                            ///  Pass Esc
                            keybd.textline = strv.substr(0, 1);
                            owner.SIGNAL(e2::release, e2::term::key, keybd);
                            total = strv.substr(1);
                            //strv = total;
                            break;
                        }
                        #endif /// DEMO
                        else if (strv.at(pos) == '[')
                        {
                            if (++pos == len) { total = strv; break; }//incomlpete
                            if (strv.at(pos) == 'I')
                            {
                                focus = true;
                                owner.SIGNAL(e2::release, e2::term::focus, focus);
                                log("\t - focus on ", canal);
                                ++pos;
                            }
                            else if (strv.at(pos) == 'O')
                            {
                                focus = faux;
                                owner.SIGNAL(e2::release, e2::term::focus, focus);
                                log("\t - focus off: ", canal);
                                ++pos;
                            }
                            else if (strv.at(pos) == '<') // "\033[<0;x;yM/m"
                            {
                                if (++pos == len) { total = strv; break; }// incomlpete sequence

                                auto tmp = strv.substr(pos);
                                auto l = tmp.size();
                                if (auto ctrl = utf::to_int(tmp))
                                {
                                    pos += l - tmp.size();
                                    if (pos == len) { total = strv; break; }// incomlpete sequence
                                    {
                                        if (++pos == len) { total = strv; break; }// incomlpete sequence

                                        view tmp = strv.substr(pos);
                                        auto l = tmp.size();
                                        if (auto pos_x = utf::to_int(tmp))
                                        {
                                            pos += l - tmp.size();
                                            if (pos == len) { total = strv; break; }// incomlpete sequence
                                            {
                                                if (++pos == len) { total = strv; break; }// incomlpete sequence

                                                view tmp = strv.substr(pos);
                                                auto l = tmp.size();
                                                if (auto pos_y = utf::to_int(tmp))
                                                {
                                                    pos += l - tmp.size();
                                                    if (pos == len) { total = strv; break; }// incomlpete sequence
                                                    {
                                                        auto ispressed = (strv.at(pos) == 'M');
                                                        ++pos;

                                                        auto clamp = [](auto a) { return std::clamp(a,
                                                            std::numeric_limits<iota>::min() / 2,
                                                            std::numeric_limits<iota>::max() / 2); };

                                                        auto x = clamp(pos_x.value() - 1);
                                                        auto y = clamp(pos_y.value() - 1);
                                                        auto ctl = ctrl.value();

                                                        // ks & 0x10 ? f + ";2" // shift
                                                        // ks & 0x02 || ks & 0x01 ? f + ";3" // alt
                                                        // ks & 0x04 || ks & 0x08 ? f + ";5" // ctrl
                                                        // 00000 000
                                                        //   ||| |||
                                                        //   ||| |------ btn state
                                                        //   |---------- ctl state
                                                        bool k_shift = ctl & 0x4;
                                                        bool k_alt   = ctl & 0x8;
                                                        bool k_ctrl  = ctl & 0x10;
                                                        mouse.ctlstate = (k_shift ? hids::SHIFT : 0)
                                                                       + (k_alt   ? hids::ALT   : 0)
                                                                       + (k_ctrl  ? hids::CTRL  : 0);
                                                        //if ( mouse.ctlstate ) log(" mouse.ctlstate =",  mouse.ctlstate );
                                                        ctl = ctl & ~0b00011100;

                                                        mouse.wheeled = faux;
                                                        mouse.wheeldt = 0;
                                                        mouse.shuffle = faux;

                                                        bool fire = true;

                                                        constexpr static int total = sysmouse::numofbutton;
                                                        constexpr static int first = sysmouse::left;
                                                        constexpr static int midst = sysmouse::middle;
                                                        constexpr static int other = sysmouse::right;
                                                        constexpr static int winbt = sysmouse::win;
                                                        constexpr static int wheel = sysmouse::wheel;
                                                        constexpr static int joint = sysmouse::leftright;

                                                        // Moving should be fired first
                                                        if ((mouse.ismoved = mouse.coor({ x, y })))
                                                        {
                                                            owner.SIGNAL(e2::release, e2::term::mouse, mouse);
                                                            mouse.ismoved = faux;
                                                        }

                                                        switch (ctl)
                                                        {
                                                            case 0:
                                                                mouse.button[first] = ispressed;
                                                                break;
                                                            case 1:
                                                                mouse.button[midst] = ispressed;
                                                                break;
                                                            case 2:
                                                                mouse.button[other] = ispressed;
                                                                break;
                                                            case 3:
                                                                mouse.button[winbt] = ispressed;
                                                                //if (!ispressed) // WinSrv2019 vtmouse bug workaround
                                                                //{               //  - release any button always fires winbt release
                                                                //	mouse.button[first] = ispressed;
                                                                //	mouse.button[midst] = ispressed;
                                                                //	mouse.button[other] = ispressed;
                                                                //}
                                                                break;
                                                            case 64:
                                                                mouse.wheeled = true;
                                                                mouse.wheeldt = 1;
                                                                break;
                                                            case 65:
                                                                mouse.wheeled = true;
                                                                mouse.wheeldt = -1;
                                                                break;
                                                            default:
                                                                fire = faux;
                                                                mouse.shuffle = !mouse.ismoved;
                                                                break;
                                                        }

                                                        if (fire)
                                                        {
                                                            owner.SIGNAL(e2::release, e2::term::mouse, mouse);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            else if (is_digit(strv.at(pos)))
                            {
again:
                                view tmp = strv.substr(pos);
                                auto l = tmp.size();
                                auto event_id = utf::to_int(tmp).value();
                                if (event_id > ansi::W32_START_EVENT
                                 && event_id < ansi::W32_FINAL_EVENT)
                                {
                                    //log("win32input: ", strv);
                                    pos += l - tmp.size();
                                    if (pos == len) { total = strv; break; }// incomlpete sequence
                                    {
                                        auto take = [&]() {
                                            view tmp = strv.substr(pos);
                                            if (auto l = tmp.size())
                                            {
                                                tmp.remove_prefix(1); // pop ':'
                                                if (tmp.size())
                                                {
                                                    if (tmp.at(0) == ':')
                                                    {
                                                        pos++;
                                                        return 0;
                                                    }
                                                    auto p = utf::to_int(tmp).value();
                                                    pos += l - tmp.size();
                                                    return p;
                                                }
                                            }
                                            return 0;
                                        };
                                        switch (event_id)
                                        {
                                            case ansi::W32_MOUSE_EVENT:
                                            {
                                                iota id    = take();
                                                iota bttns = take();
                                                iota ctrls = take();
                                                iota flags = take();
                                                iota wheel = take();
                                                iota xcoor = take();
                                                iota ycoor = take();

                                                auto coord = twod{ xcoor, ycoor };

                                                mouse.button[0] = bttns & (1 << 0); // FROM_LEFT_1ST_BUTTON_PRESSED
                                                mouse.button[1] = bttns & (1 << 1); // RIGHTMOST_BUTTON_PRESSED;
                                                mouse.button[3] = bttns & (1 << 2); // FROM_LEFT_2ND_BUTTON_PRESSED;
                                                mouse.button[2] = bttns & (1 << 3); // FROM_LEFT_3RD_BUTTON_PRESSED;
                                                mouse.button[4] = bttns & (1 << 4); // FROM_LEFT_4TH_BUTTON_PRESSED;

                                                mouse.ismoved = mouse.coor(coord);
                                                mouse.shuffle = !mouse.ismoved && (flags & (1 << 0)); // MOUSE_MOVED
                                                // Makes no sense (ignored)
                                                mouse.doubled = flags & (1 << 1); // DOUBLE_CLICK;
                                                mouse.wheeled = flags & (1 << 2); // MOUSE_WHEELED;
                                                mouse.hzwheel = flags & (1 << 3); // MOUSE_HWHEELED;
                                                mouse.wheeldt = wheel;

                                                // Windows Terminal Reported mouse ctrlstate is broken
                                                bool k_ralt  = ctrls & 0x1;
                                                bool k_alt   = ctrls & 0x2;
                                                bool k_rctrl = ctrls & 0x4;
                                                bool k_ctrl  = ctrls & 0x8;
                                                bool k_shift = ctrls & 0x10;
                                                mouse.ctlstate = (k_shift ? hids::SHIFT : 0)
                                                               + (k_alt   ? hids::ALT   : 0)
                                                               + (k_ralt  ? hids::ALT   : 0)
                                                               + (k_rctrl ? hids::RCTRL : 0)
                                                               + (k_ctrl  ? hids::CTRL  : 0);

                                                if (!mouse.shuffle)
                                                    owner.SIGNAL(e2::release, e2::term::mouse, mouse);
                                                break;
                                            }
                                            case ansi::W32_KEYBD_EVENT:
                                            {
                                                iota id = take();
                                                iota kc = take();
                                                iota sc = take();
                                                iota kd = take();
                                                iota ks = take();
                                                iota rc = take();
                                                iota uc = take();
                                                keybd.virtcode    = kc;
                                                keybd.ctlstate    = ks;
                                                keybd.down        = kd;
                                                keybd.repeatcount = rc;
                                                keybd.scancode    = sc;
                                                keybd.character   = uc;
                                                auto ctrl = [ks](text f, auto e)
                                                {
                                                    auto b = ks & 0x10 ? f + ";2" // shift
                                                           : ks & 0x02 || ks & 0x01 ? f + ";3" // alt
                                                           : ks & 0x04 || ks & 0x08 ? f + ";5" // ctrl
                                                           : f;
                                                    return "\033[" + b + e;
                                                };
                                                using key = syskeybd;
                                                if (keybd.down)
                                                {
                                                    switch (kc)
                                                    {
                                                        //todo Ctrl+Space
                                                        //     Ctrl+Backspace
                                                        //     Alt+0..9
                                                        //     Ctrl/Shift+Enter
                                                        case key::Backspace: keybd.textline = "\177"; break;
                                                        case key::Tab:       keybd.textline = ks & 0x10 ? "\033[Z" : "\t"; break;
                                                        case key::PageUp:    keybd.textline = ctrl("5",  "~"); break;
                                                        case key::PageDown:  keybd.textline = ctrl("6",  "~"); break;
                                                        case key::End:       keybd.textline = ctrl("1",  "F"); break;
                                                        case key::Home:      keybd.textline = ctrl("1",  "H"); break;
                                                        case key::Insert:    keybd.textline = ctrl("2",  "~"); break;
                                                        case key::Delete:    keybd.textline = ctrl("3",  "~"); break;
                                                        case key::Up:        keybd.textline = ctrl("1",  "A"); break;
                                                        case key::Down:      keybd.textline = ctrl("1",  "B"); break;
                                                        case key::Right:     keybd.textline = ctrl("1",  "C"); break;
                                                        case key::Left:      keybd.textline = ctrl("1",  "D"); break;
                                                        case key::F1:        keybd.textline = ctrl("1",  "P"); break;
                                                        case key::F2:        keybd.textline = ctrl("1",  "Q"); break;
                                                        case key::F3:        keybd.textline = ctrl("1",  "R"); break;
                                                        case key::F4:        keybd.textline = ctrl("1",  "S"); break;
                                                        case key::F5:        keybd.textline = ctrl("15", "~"); break;
                                                        case key::F6:        keybd.textline = ctrl("17", "~"); break;
                                                        case key::F7:        keybd.textline = ctrl("18", "~"); break;
                                                        case key::F8:        keybd.textline = ctrl("19", "~"); break;
                                                        case key::F9:        keybd.textline = ctrl("20", "~"); break;
                                                        case key::F10:       keybd.textline = ctrl("21", "~"); break;
                                                        case key::F11:       keybd.textline = ctrl("23", "~"); break;
                                                        case key::F12:       keybd.textline = ctrl("24", "~"); break;
                                                        default:
                                                            //log("uc = ", uc);
                                                            if (uc)
                                                                keybd.textline = utf::to_utf_from_code(uc);
                                                            break;
                                                    }
                                                }
                                                else
                                                {
                                                    keybd.textline.clear();
                                                }
                                                owner.SIGNAL(e2::release, e2::term::key, keybd);
                                                break;
                                            }
                                            case ansi::W32_WINSZ_EVENT:
                                            {
                                                iota xsize = take();
                                                iota ysize = take();
                                                twod winsz{ xsize,ysize };
                                                owner.SIGNAL(e2::release, e2::term::size, winsz);
                                                break;
                                            }
                                            case ansi::W32_FOCUS_EVENT:
                                            {
                                                //todo clear pressed keys on lost focus
                                                iota id    = take();
                                                bool focus = take();
                                                owner.SIGNAL(e2::release, e2::term::focus, focus);
                                                break;
                                            }
                                            default:
                                                break;
                                        }
                                        // pop '_' or ';'
                                        if (strv.at(pos) == ';')
                                        {
                                            pos++;
                                            goto again;
                                        }
                                        else pos++; // pop '_'
                                    }
                                }
                                else
                                {
                                    unk = true;
                                    pos = 0_sz;
                                }
                            }
                            else
                            {
                                unk = true;
                                pos = 0_sz;
                            }
                        }
                        else if (strv.at(pos) == ']')
                        {
                            if (++pos == len) { total = strv; break; }//incomlpete

                            auto tmp = strv.substr(pos);
                            auto l = tmp.size();
                            if (auto pos_x = utf::to_int(tmp))
                            {
                                pos += l - tmp.size();
                                if (pos == len) { total = strv; break; }//incomlpete
                                {
                                    if (++pos == len) { total = strv; break; }//incomlpete

                                    auto tmp = strv.substr(pos);
                                    auto l = tmp.size();
                                    if (auto pos_y = utf::to_int(tmp))
                                    {
                                        pos += l - tmp.size();
                                        if (pos == len) { total = strv; break; }//incomlpete
                                        {
                                            ++pos;

                                            auto x = pos_x.value();
                                            auto y = pos_y.value();

                                            twod winsz{ x,y };
                                            owner.SIGNAL(e2::release, e2::term::size, winsz);
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            unk = true;
                            pos = 0_sz;
                        }
                    }

                    if (!unk)
                    {
                        total = strv.substr(pos);
                        strv = total;
                    }

                    if (auto size = strv.size())
                    {
                        auto i = unk ? 1_sz : 0_sz;
                        while (i != size && (strv.at(i) != '\x1b'))
                        {
                            // Pass SIGINT inside the desktop
                            //if (strv.at(i) == 3 /*3 - SIGINT*/)
                            //{
                            //	log(" - SIGINT in stdin");
                            //	owner.SIGNAL(e2::release, e2::term::quit, "pipe: SIGINT");
                            //	return;
                            //}
                            i++;
                        }

                        if (i)
                        {
                            keybd.textline = strv.substr(0, i);
                            owner.SIGNAL(e2::release, e2::term::key, keybd);
                            total = strv.substr(i);
                            strv = total;
                        }
                    }
                }
            }

            log("link: std_input thread ended");
        }
        // pipe: Interrupt the run only.
        void shutdown ()
        {
            mutex.lock();
            canal->shut(); // Terminate all blocking calls.

            alive = faux;
            ready = true;
            synch.notify_one(); //to interrupt session
            mutex.unlock();
        }
    };

    class diff
    {
        using work = std::thread;
        using lock = std::recursive_mutex;
        using cond = std::condition_variable_any;
        using ansi = ansi::esc;
        using span = period;
        using pair = std::optional<std::pair<span, iota>>;

        link& conio;
        lock  mutex; // diff: Mutex between renderer and committer threads.
        cond  synch; // diff: The synchronization mechanism between the renderer and the committer.

        //todo deprecate bsu/esu
        //todo unify bsu/esu
        enum mode_type { none, escp, decs, last };
        enum mode_cmd { bsu, esu };
        std::vector<std::vector<text>> bsuesu;
        lock  bsu_mutex; // diff: Mutex between renderer and committer threads.
        iota  smode; // diff: synchronous mode

        grid& cache; // diff: The current content buffer which going to be checked and processed.
        grid  front; // diff: The Shadow copy of the terminal/screen.

        iota  rhash; // diff: Rendered buffer genus. The genus changes when the size of the buffer changes.
        iota  dhash; // diff: Unchecked buffer genus. The genus changes when the size of the buffer changes.
        twod  field; // diff: Current terminal/screen window size.
        twod  midst; // diff: Current terminal/screen window center.
        span  watch; // diff: Duration of the STDOUT rendering.
        iota  delta; // diff: Last ansi-rendered frame size.
        ansi  frame; // diff: Text screen representation.
        iota  empty;
        bool  alive; // diff: Working loop state.
        bool  ready; // diff: Conditional variable to avoid spurious wakeup.
        bell  radio;
        work  paint; // diff: Rendering thread.
        pair  debug; // diff: Debug info.

        text  extra; // diff: Extra data to cout.

        // diff: Render current buffer to the screen.
        void render()
        {
            using time = moment;

            log("rend: thread started");

            auto fallback = [&](auto& c, auto& state, auto& frame)
            {
                auto dumb = c;
                dumb.txt(utf::REPLACEMENT_CHARACTER_UTF8_VIEW);
                dumb.scan(state, frame);
            };

            std::unique_lock guard{ mutex };

            cell state;
            time start;

            //todo unify (it is just a proof of concept)
            while ((void)synch.wait(guard, [&]{ return ready; }), alive)
            {
                ready = faux;
                start = tempus::now();

                if (rhash != dhash)
                {
                    log("diff: frame resize: ", field);

                    rhash = dhash;
                    front = cache;

                    auto src = front.data();
                    auto end = src + front.size();
                    auto row = 0;

                    while (row++ < field.y)
                    {
                        frame.locate(1, row);
                        auto end = src + field.x;
                        while (src != end)
                        {
                            //src++->scan(state, frame);

                            auto& c = *(src++);
                            if (c.wdt() < 2)
                            {
                                c.scan(state, frame);
                            }
                            else
                            {
                                if (c.wdt() == 2)
                                {
                                    if (src != end)
                                    {
                                        auto& d = *(src++);
                                        if (d.wdt() < 3)
                                        {
                                            /// Left part alone
                                            fallback(c, state, frame);
                                            src--; /// Repeat all for d again
                                        }
                                        else
                                        {
                                            if (!c.scan(d, state, frame))
                                            {
                                                /// Left part alone
                                                fallback(c, state, frame);
                                                src--; /// Repeat all for d again
                                            }
                                        }
                                    }
                                    else
                                    {
                                        /// Left part alone
                                        fallback(c, state, frame);
                                    }
                                }
                                else
                                {
                                    /// Right part alone
                                    fallback(c, state, frame);
                                }
                            }
                        }
                    }
                }
                else
                {
                    auto src = cache.data();
                    auto dst = front.data();
                    auto beg = src;
                    auto end = src;
                    iota row = 0;

                    while (row++ < field.y)
                    {
                        end += field.x;

                        while (src != end)
                        {
                            auto& fore = *src++;
                            auto& back = *dst++;

                            auto w = fore.wdt();
                            if (w < 2)
                            {
                                if (back != fore)
                                {
                                    auto col = static_cast<iota>(src - beg);
                                    frame.locate(col, row);

                                    back = fore;
                                    fore.scan(state, frame);

                                    /* optimizations */
                                    while (src != end)
                                    {
                                        auto& fore = *src++;
                                        auto& back = *dst++;

                                        auto w = fore.wdt();
                                        if (w < 2)
                                        {
                                            if (back == fore) break;
                                            else
                                            {
                                                back = fore;
                                                fore.scan(state, frame);
                                            }
                                        }
                                        else if (w == 2) /// Check left part
                                        {
                                            if (src != end)
                                            {
                                                if (back == fore)
                                                {
                                                    auto& d = *(src++);
                                                    auto& g = *(dst++);
                                                    if (g == d) break;
                                                    else
                                                    {
                                                        if (d.wdt() < 3)
                                                        {
                                                            /// Left part alone
                                                            fallback(fore, state, frame);
                                                            src--; /// Repeat all for d again
                                                            dst--; /// Repeat all for g again
                                                        }
                                                        else /// d.wdt() == 3
                                                        {
                                                            if (!fore.scan(d, state, frame))
                                                            {
                                                                /// Left part alone
                                                                fallback(fore, state, frame);
                                                                /// Right part alone
                                                                fallback(d, state, frame);
                                                            }
                                                            g = d;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    back = fore;

                                                    auto& d = *(src++);
                                                    auto& g = *(dst++);
                                                    if (d.wdt() < 3)
                                                    {
                                                        /// Left part alone
                                                        fallback(fore, state, frame);
                                                        src--; /// Repeat all for d again
                                                        dst--; /// Repeat all for g again
                                                    }
                                                    else /// d.wdt() == 3
                                                    {
                                                        if (!fore.scan(d, state, frame))
                                                        {
                                                            /// Left part alone
                                                            fallback(fore, state, frame);
                                                            /// Right part alone
                                                            fallback(d, state, frame);
                                                        }
                                                        g = d;
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                if (back != fore) back = fore;
                                                /// Left part alone
                                                fallback(fore, state, frame);
                                            }
                                        }
                                        else /// w == 3
                                        {
                                            if (back != fore) back = fore;
                                            /// Right part alone
                                            fallback(fore, state, frame);
                                        }
                                    }
                                    /* optimizations */
                                }
                            }
                            else
                            {
                                if (w == 2) /// Left part has changed
                                {
                                    if (back != fore)
                                    {
                                        back = fore;

                                        auto col = static_cast<iota>(src - beg);
                                        frame.locate(col, row);

                                        if (src != end)
                                        {
                                            auto& d = *(src++);
                                            auto& g = *(dst++);
                                            if (d.wdt() < 3)
                                            {
                                                /// Left part alone
                                                fallback(fore, state, frame);
                                                src--; /// Repeat all for d again
                                                dst--; /// Repeat all for g again
                                            }
                                            else /// d.wdt() == 3
                                            {
                                                if (!fore.scan(d, state, frame))
                                                {
                                                    /// Left part alone
                                                    fallback(fore, state, frame);
                                                    /// Right part alone
                                                    fallback(d, state, frame);
                                                }
                                                g = d;
                                            }
                                        }
                                        else
                                        {
                                            /// Left part alone
                                            fallback(fore, state, frame);
                                        }
                                    }
                                    else /// Check right part
                                    {
                                        if (src != end)
                                        {
                                            auto& d = *(src++);
                                            auto& g = *(dst++);
                                            if (d.wdt() < 3)
                                            {
                                                auto col = static_cast<iota>(src - beg - 1);
                                                frame.locate(col, row);
                                                /// Left part alone
                                                fallback(fore, state, frame);
                                                src--; /// Repeat all for d again
                                                dst--; /// Repeat all for g again
                                            }
                                            else /// d.wdt() == 3
                                            {
                                                if (g != d)
                                                {
                                                    g = d;
                                                    auto col = static_cast<iota>(src - beg - 1);
                                                    frame.locate(col, row);

                                                    if (!fore.scan(d, state, frame))
                                                    {
                                                        /// Left part alone
                                                        fallback(fore, state, frame);
                                                        /// Right part alone
                                                        fallback(d, state, frame);
                                                    }
                                                }
                                            }
                                        }
                                        else
                                        {
                                            auto col = static_cast<iota>(src - beg);
                                            frame.locate(col, row);
                                            /// Left part alone
                                            fallback(fore, state, frame);
                                        }
                                    }
                                }
                                else /// w == 3 /// Right part has changed
                                {
                                    auto col = static_cast<iota>(src - beg);
                                    frame.locate(col, row);
                                    back = fore;
                                    /// Right part alone
                                    fallback(fore, state, frame);
                                }
                            }
                        }

                        beg += field.x;
                    }
                }

                auto size = static_cast<iota>(frame.size());
                if (size != empty)
                {
                    delta = size;
                    //todo deprecate BSU/ESU
                    //frame.locate(midst + twod{ 0,1 });
                    //auto& synch_sfx = bsuesu[smode];
                    //frame.add(synch_sfx[mode_cmd::esu]);
                    conio.output(frame);
                    frame.clear();
                    //frame.locate(midst);
                    //frame.add(synch_sfx[mode_cmd::bsu]);

                    empty = static_cast<iota>(frame.size());
                }

                watch = tempus::now() - start;
            }
        }

    public:
        // diff: Obtain new content to render.
        pair commit(core& canvas) // Run inside the e2::sync
        {
            std::unique_lock guard(mutex, std::try_to_lock);
            if (guard.owns_lock())
            {
                dhash = canvas.hash();
                field = canvas.swap(cache); // Use one surface for reading, one for writing.
                //field = canvas.copy(cache);
                //midst = field / 2;
                midst = field;
                debug = { watch, delta };

                if (extra.length())
                {
                    frame.add(extra);
                    extra.clear();
                }

                ready = true;
                synch.notify_one();
                return debug;
            }
            return std::nullopt;
        }

        diff(link& conio, grid& cache)
            : rhash{ 0 },
              dhash{ 0 },
              delta{ 0 },
              watch{ 0 },
              empty{ 0 },
              alive{ true },
              ready{ faux },
              conio{ conio },
              cache{ cache }
        {
            log("diff: ctor start");

            bsuesu.resize(4);
            bsuesu[mode_type::none].resize(2);
            bsuesu[mode_type::escp].resize(2);
            bsuesu[mode_type::decs].resize(2);
            bsuesu[mode_type::last].resize(2);
            bsuesu[mode_type::escp][mode_cmd::bsu] = "\033P=1s\033\\";
            bsuesu[mode_type::escp][mode_cmd::esu] = "\033P=2s\033\\";
            bsuesu[mode_type::decs][mode_cmd::bsu] = "\033[?2026h";
            bsuesu[mode_type::decs][mode_cmd::esu] = "\033[?2026l";
            bsuesu[mode_type::last][mode_cmd::bsu] = "=BSU";
            bsuesu[mode_type::last][mode_cmd::esu] = "=ESU";

            smode = -1;
            radio.SIGNAL(e2::general, e2::radio, smode);
            radio.SUBMIT(e2::general, e2::radio, mode)
            {
                if (mode >= 0)
                {
                    smode = std::min<iota>(mode, mode_type::last);
                }
            };

            paint = work([&] { render(); });
            log("diff: ctor complete");
        }

        ~diff()
        {
            log("diff: dtor");
            if (paint.joinable())
            {
                mutex.lock();
                alive = faux;
                ready = true;
                synch.notify_all();
                mutex.unlock();
                paint.join();
                log("diff: render thread joined");
            }
        }

        void append(view utf8)
        {
            extra = utf8;
        }
    };

    class gate // console: VTM client viewport.
        : public form
    {
        using self = gate;
        FEATURE(pro::keybd, keybd); // gate: Keyboard controller
        FEATURE(pro::robot, robot); // gate: Amination controller
        FEATURE(pro::maker, maker); // gate: Form generator
        //FEATURE(pro::caret, caret); // gate: Cursor controller
        FEATURE(pro::title, title); // gate: NetXS Group logo watermark
        FEATURE(pro::guard, guard); // gate: Watch dog against robots and single Esc detector
        FEATURE(pro::input, input); // gate: User input event handler
        #ifdef DEBUG_OVERLAY
        FEATURE(pro::debug, debug); // gate: Debug telemetry controller.
        #endif

        using pair = std::optional<std::pair<period, iota>>;
        pair  yield; // gate: Indicator that the current frame has been successfully STDOUT.

        para uname;

    public:
        // The client main loop.
        void proceed(xipc media /*session socket*/, text title)
        {
            if (auto world = parent.lock())
            {
                link conio{ *this, media };          // gate: Terminal IO.
                diff paint{ conio, input.freeze() }; // gate: Rendering loop.
                subs token;                          // gate: Subscription tokens array.

                // conio events
                SUBMIT_T(e2::release, e2::term::size, token, newsize)
                {
                    base::resize(newsize);
                };
                SUBMIT_T(e2::release, e2::term::unknown, token, unkstate)
                {
                };
                SUBMIT_T(e2::release, e2::term::focus, token, unkstate)
                {
                };
                SUBMIT_T(e2::release, e2::term::error, token, errcode)
                {
                    text msg = "\n\rgate: Term error: " + std::to_string(errcode) + "\r\n";
                    log("gate: stop byemsg: ", msg);
                    conio.shutdown();
                };
                SUBMIT_T(e2::release, e2::term::quit, token, msg)
                {
                    log("gate: stop byemsg: ", msg);
                    conio.shutdown();
                };

                //SUBMIT_T(e2::preview, e2::form::prop::header, token, newheader)
                SUBMIT_T(e2::release, e2::form::state::header, token, newheader)
                {
                    //para tmp{ newheader };
                    text title;
                    newheader.lyric->each([&](auto c) { title += c.txt(); });
                    conio.output(ansi::tag(title));
                    //log("title preview: ", title);
                };

                SUBMIT_T(e2::release, e2::cout, token, extra_data)
                {
                    paint.append(extra_data);
                };

                world->SUBMIT_T(e2::release, e2::form::proceed::render, token, render_scene)
                {
                    auto stamp = tempus::now();
                    if (render_scene(form::canvas, gate::title.titles()) || !yield) // Put the world on my canvas
                    {
                        // Update objects under mouse cursor
                        //input.fire(e2::hids::mouse::hover);
#ifdef DEBUG_OVERLAY
                        debug.bypass = true;
                        //input.fire(e2::hids::mouse::hover);
                        input.fire(e2::hids::mouse::move);
                        debug.bypass = faux;
#else
                        input.fire(e2::hids::mouse::move);
#endif // DEBUG_OVERLAY

                        // in order to draw debug overlay, maker, titles, etc
                        SIGNAL(e2::release, e2::form::upon::redrawn, form::canvas);
                        #ifdef DEBUG_OVERLAY
                            if ((yield = paint.commit(canvas)))
                            {
                                auto& watch = yield.value().first;
                                auto& delta = yield.value().second;
                                debug.update(watch, delta);
                            }
                            debug.update(stamp);
                        #else
                            yield = paint.commit(form::canvas); // Try to output my canvas to my console
                        #endif // DEBUG_OVERLAY
                    }
                };

                conio.session(title);
            }
        }

    protected:
        gate(view user_name)
        {
            //todo unify
            uname = user_name;

            using bttn = e2::hids::mouse::button;
            using keyb = e2::hids::keybd;

            title.live = faux;

            //todo unify creation (delete simple create wo gear)
            SUBMIT(e2::preview, e2::form::proceed::create, region)
            {
                if (auto world = parent.lock())
                {
                    region.coor += base::coor.get();
                    world->SIGNAL(e2::release, e2::form::proceed::create, region);
                }
            };
            SUBMIT(e2::preview, e2::form::proceed::createby, gear)
            {
                if (auto world = parent.lock())
                {
                    gear.slot.coor += base::coor.get();
                    world->SIGNAL(e2::release, e2::form::proceed::createby, gear);
                }
            };

            SUBMIT(e2::preview, keyb::any, gear)
            {
                //todo unify
                //if (gear.meta(hids::CTRL | hids::RCTRL))
                {
                    //todo unify
                    auto pgup = gear.keystrokes == "\033[5;5~"s;
                    auto pgdn = gear.keystrokes == "\033[6;5~"s;
                    if (pgup || pgdn)
                    {
                        if (auto world = parent.lock())
                        {
                            sptr<base> item_ptr;
                            if (pgdn) world->SIGNAL(e2::request, e2::form::proceed::detach, item_ptr); // Take prev item
                            else      world->SIGNAL(e2::request, e2::form::proceed::attach, item_ptr); // Take next item

                            if (item_ptr)
                            {
                                auto& item = *item_ptr;
                                auto square = item.square();
                                auto center = square.coor + (square.size / 2);
                                SIGNAL(e2::release, e2::form::layout::shift, center);

                                //todo unify
                                gear.clear_kb_focus();
                                gear.kb_focus_taken = faux;
                                item.SIGNAL(e2::release, e2::form::upevent::kboffer, gear);
                            }
                            gear.dismiss();
                        }
                    }
                }
            };

            SUBMIT(e2::release, bttn::drag::start::leftright, gear)
            {
                if (gear.capture(bell::id))
                {
                    robot.pacify();
                    gear.dismiss();
                }
            };
            SUBMIT(e2::release, bttn::drag::pull::leftright, gear)
            {
                if (gear.captured(bell::id))
                {
                    base::moveby(-gear.delta.get());
                    gear.dismiss();
                }
            };
            SUBMIT(e2::release, bttn::drag::cancel::leftright, gear)
            {
                if (gear.captured(bell::id))
                {
                    gear.release();
                    gear.dismiss();
                }
            };
            SUBMIT(e2::release, bttn::drag::stop::leftright, gear)
            {
                if (gear.captured(bell::id))
                {
                    gear.release();
                    robot.pacify();
                    robot.actify(gear.mouse::fader<quadratic<twod>>(2s), [&](auto& x)
                                 {
                                    base::moveby(-x);
                                 });
                    gear.dismiss();
                }
            };

            SUBMIT(e2::release, e2::form::layout::shift, newpos)
            {
                auto window = canvas.area();
                auto oldpos = window.coor + (window.size / 2);

                auto path = oldpos - newpos;
                iota time = SWITCHING_TIME;
                auto func = constlinearAtoB<twod>(path, time, now<iota>());

                robot.pacify();
                robot.actify(func, [&](auto& x) {
                                     moveby(-x);
                                 });
            };
        }

        // gate: Draw the form composition on the specified canvas.
        virtual void renderproc (face& parent)
        {
            //base::renderproc(parent_canvas);

            // Draw a shadow of user's terminal window for other users (spectators)
            // see pro::scene
            if (&parent != &canvas)
            {
                auto area = canvas.area();
                area.coor-= parent.area().coor;
                parent.fill(area, skin::color(tone::shadow));
            }
        }
        // gate: .
        virtual void postrender (face& parent_canvas)
        {
            if (&parent_canvas != &canvas)
            {
                auto area = canvas.area();
                area.coor -= parent_canvas.area().coor;

                area.coor += input.coord;
                area.size = dot_11;
                cell brush;
                if (input.push)
                {
                    brush.txt(64 + input.push).bgc(reddk).fgc(whitelt);
                }
                else
                {
                    brush.txt(whitespace).bgc(greenlt);
                }
                parent_canvas.fill(area, brush);

                //if (parent.test(area.coor))
                //{
                //	auto hover_id = parent[area.coor].link();
                //	log ("---- hover id ", hover_id);
                //}
                //auto& header = *title.header().lyric;
                auto& header = *uname.lyric;
                area.coor += parent_canvas.area().coor;
                area.coor.y--;
                area.coor.x -= (iota)header.size().x / 2;
                //todo unify header coords
                header.move(area.coor);
                parent_canvas.fill(header);
            }
        }
    };
}

#endif // NETXS_CONSOLE_HPP