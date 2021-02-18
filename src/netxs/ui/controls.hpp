// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_CONTROLS_HPP
#define NETXS_CONTROLS_HPP

#include "../console/console.hpp"

#include <vector>
#include <mutex>
#include <map>

namespace netxs::ui
{
    using namespace netxs;
    using namespace netxs::console;

    enum axis : id_t { X, Y };
    enum axes
    {
        NONE = 0,
        ONLY_X = 1 << 0,
        ONLY_Y = 1 << 1,
        ALL    = (ONLY_X | ONLY_Y),
    };

    class mold // controls.h: Flexible window frame.
        : public form
    {
    public:
        struct sock
        {
            id_t      id; // sock: Hids ID
            mold& master; // sock: Main form reference
            twod& length = master.square.size;// sock: master.square.size
            face& canvas = master.canvas;     // sock: master.canvas
            twod& center = master.center;     // sock: master.center
            bool& locked = master.locked;     // sock: master.locked
            rect& middle = master.middle;     // sock: master.middle
            twod& gripsz = master.gripsz;     // sock: master.gripsz

            twod  origin; // sock: Grab's initial coord info
            bool  wholly; // sock: Should the whole border be visible
            twod  dtcoor; // sock: The form coor parameter change factor while resizing
            twod  dtsize; // sock: The form size parameter change factor while resizing
            twod  sector; // sock: Active quadrant, x,y = {-1|+1}
            twod  corner; // sock: Coordinates of the active corner
            twod  levels; // sock: The lengths of the grips (a corner based, signed)
            bool  direct; // sock: Indirect mouse hover
            twod  widths; // sock: Border widths

            operator bool()
            {
                return !wholly;
            }
            bool operator ()(id_t hids_id)
            {
                return id == hids_id;
            }
            // sock: Assign the borders/resize-grips length
            template<class P>
            void draw(core& canvas, P fuse)
            {
                //auto c = corner.less(dot_11, dot_00, length);
                //todo revise
                //auto c = master.base::coor.get() + corner.less(dot_11, dot_00, length);
                auto c = master.base::coor.get()
                    - canvas.coor() + corner.less(dot_11, dot_00, length);

                auto side_x = rect{ c, { levels.x, widths.y } }.normalize();
                c.y += levels.y > 0 ? 1 : -1;
                auto side_y = rect{ c, { widths.x, levels.y } }.normalize();

                canvas.fill(side_x, fuse);
                canvas.fill(side_y, fuse);
            }
            // sock: Take the current coordinates of the mouse relative to the corresponding corner
            void grab(twod const& coord)
            {
                origin = coord - (wholly ? center : corner);
            }
            // sock: Mark borders if mold hilighted
            void join(hids const& gear)
            {
                direct = gear.start == master.bell::id;
            }
            // sock: Recalc (on every mouse::move or ::hover) the borders/resize-grips length and other metrics
            bool calc(hids const& gear)
            {
                auto& curpos = gear.coord;
                if (!gear.captured(master.bell::id))
                {
                    wholly = locked
                         || !direct
                         ||  middle.hittest(curpos)
                         || !length.inside (curpos);

                    dtcoor = curpos.less(center + (length & 1), dot_11, dot_00);
                    dtsize = dtcoor.less(dot_11, dot_11,-dot_11);
                    sector = dtcoor.less(dot_11,-dot_11, dot_11);
                    widths = sector * gripsz;
                }

                corner = dtcoor.less(dot_11, length - dot_11, dot_00);
                auto l = sector * (curpos - corner) + dot_11;
                auto a = (length - center) * l / center;
                auto b = (center - dot_11) *~l /~center;
                auto s = std::clamp(a - b + center, dot_22, std::max(dot_22, length));
                s.y -= 1; // To avoid grpis overlapping at the corner

                return levels(sector * s);
            }
            // sock: .
            void drag(twod const& coord)
            {
                auto delta = coord - origin;
                if (wholly)
                {
                    delta -= center;
                    master.base::moveby(delta);
                }
                else
                {
                    delta -= corner;
                    if (auto dxdy = master.base::sizeby(delta * dtsize))
                    {
                        master.base::moveby(-dxdy * dtcoor);
                    }
                }
            }
            sock(id_t ctrl, mold& boss)
                :     id{ ctrl },
                  master{ boss },
                  wholly{ faux },
                  direct{ faux }
            { }
        };

    private:
        using self = mold;
        FEATURE(pro::keybd, keybrd); // mold: Keyboard controller
        FEATURE(pro::mouse, xmouse); // mold: World image
        FEATURE(pro::robot, cyborg); // mold: Animation controller
        FEATURE(pro::frame, window); // mold: Window controller
        FEATURE(pro::align, adjust); // mold: Size linking controller
        FEATURE(pro::title, legend); // mold: Window caption and footer
        FEATURE(pro::share, shared); // mold: The shared border states
        //pro::focus<mold> xfocus{*this };	// mold: Focus controller

        using sptr = netxs::sptr<base>;

        rect region; // mold: Client area
        bool active; // mold: Keyboard focus
        bool secure; // mold: Can the object be deleted
        twod gripsz; // mold: Border width
        rect middle; // mold: The middle region of the child form
        twod center; // mold: Coordinates of the child form center
        rect square; // mold: Window size
        //rect lastsz; // mold: Last window position before fullscreen

    public:
        //todo make it private (used in vtmd::creator)
        sptr client; // mold: Client object


        bool locked; // mold: Whether the control resizable
        iota acryl = 0; // mold: Blur radius
        bool nosize = faux;

        ~mold()
        {
            log("mold: dtor ", this);
            if (client) client->base::detach();
        }
        mold()
            : active{ faux },
              locked{ faux },
              secure{ faux },
              gripsz{ skin::border_size() },
              center{ gripsz }
        {
            //todo unify
            base::limits(gripsz * 2 + dot_11, { 400,200 });
            xmouse.highlightable = true;
            base::brush.txt(whitespace);
            //base::linked = true;

            using bttn = e2::hids::mouse::button;

            {//todo only for the title test
                static auto item_number = 0_sz;
                item_number++;
                legend.header("Instance: " + std::to_string(item_number));
                //SUBMIT(e2::release, e2::form::layout::size, size)
                //{
                //	//if (!nosize)
                //	//	legend.footer("size " + std::to_string(size.x) + ":"
                //	//		+ std::to_string(size.y));
                //	if (!nosize && client)
                //	{
                //		auto& size = client->base::size.get();
                //		legend.footer("client size " + std::to_string(size.x) + ":"
                //			+ std::to_string(size.y));
                //	}
                //	else
                //	{
                //		legend.footer("window size " + std::to_string(size.x) + ":"
                //			+ std::to_string(size.y));
                //	}
                //};
            }

            SUBMIT(e2::release, e2::form::state::keybd, status)
            {
                active = status;
                base::deface();
            };

            SUBMIT(e2::release, e2::form::notify::mouse::enter, gear)
            {
                shared[gear.id].join(gear);
            };
            SUBMIT(e2::release, e2::form::notify::mouse::leave, gear)
            {
                shared.remove(gear.id);
            };
            SUBMIT(e2::release, e2::form::state::mouse, mouse_active)
            {
                base::deface();
            };

            SUBMIT(e2::preview, bttn::click::left, gear)
            {
                window.expose();
            };

            SUBMIT(e2::release, bttn::click::left, gear)
            {
                if (!square.size.inside(gear.coord))
                {
                    auto center = square.coor + (square.size / 2);
                    bell::getref(gear.id)->
                        SIGNAL(e2::release, e2::form::layout::shift, center);
                }
                base::deface();
            };
            SUBMIT(e2::release, bttn::click::right, gear)
            {
                auto coord = gear.coord + square.coor;
                if (!square.hittest(coord))
                {
                    window.appear(coord);
                }
                gear.dismiss();
            };

            SUBMIT(e2::release, e2::hids::mouse::move, gear)
            {
                shared[gear.id].calc(gear);
                base::deface();
            };

            SUBMIT(e2::release, e2::form::layout::move, coor)
            {
                square.coor = coor;
            };
            SUBMIT(e2::preview, e2::form::layout::size, size)
            {
                auto border = gripsz * 2;
                region.size = size - border;
                if (client) // Ask client about the new size (the client can override the size)
                    client->SIGNAL(e2::preview, e2::form::layout::size, region.size);

                size = region.size + border;
                // Reset canvas brush to nothing to avoid double filling.
                //form::canvas.mark(cell{}).link(bell::id);
                form::canvas.mark(whitespace).link(bell::id);
            };
            SUBMIT(e2::release, e2::form::layout::size, size)
            {
                center = std::max(size / 2, dot_11);
                square.size = size;
                middle.coor = size / 3;
                middle.size = size - (middle.coor * 2);
                if (client)
                    client->SIGNAL(e2::release, e2::form::layout::size, region.size);

                if (!nosize && client)
                {
                    auto& size = client->base::size.get();
                    legend.footer("client " + std::to_string(size.x) + ":"
                        + std::to_string(size.y));
                }
                else
                {
                    legend.footer("window " + std::to_string(size.x) + ":"
                        + std::to_string(size.y));
                }
            };

            SUBMIT(e2::release, bttn::drag::start::left, gear)
            {
                if (gear.capture(bell::id))
                {
                    shared[gear.id].grab(gear.coord);
                    cyborg.pacify();
                    gear.dismiss();
                }
            };
            SUBMIT(e2::release, bttn::drag::pull::left, gear)
            {
                if (gear.captured(bell::id))
                {
                    shared[gear.id].drag(gear.coord);
                    window.bubble();
                    gear.dismiss();
                }
            };
            SUBMIT(e2::release, bttn::drag::cancel::left, gear)
            {
                if (gear.captured(bell::id))
                {
                    //shared[gear.id].drop();
                    base::deface();
                    gear.release();
                    gear.dismiss();
                }
            };
            SUBMIT(e2::release, bttn::drag::stop::left, gear)
            {
                if (gear.captured(bell::id))
                {
                    auto& border = shared[gear.id];
                    if (border.wholly)
                    {
                        cyborg.actify(gear.fader<quadratic<twod>>(2s), [&](auto x)
                            {
                                base::moveby(x);
                            });
                    }
                    else
                    {
                        auto boundary = gear.area();
                        cyborg.actify(gear.fader<quadratic<twod>>(2s), [&, boundary](auto x)
                            {
                                window.convey(x, boundary);
                            });
                    }

                    base::deface();
                    gear.release();
                    gear.dismiss();
                }
            };

            SUBMIT(e2::release, bttn::dblclick::left, gear)
            {
                if (!secure && square.size.inside(gear.coord))
                {
                    if (adjust.seized(gear.id)) adjust.unbind();
                    else                        adjust.follow(gear.id);
                    gear.dismiss();
                }
            };
            SUBMIT(e2::release, bttn::click::leftright, gear)
            {
                if (!secure)
                {
                    base::detach();
                    gear.dismiss();
                }
            };
            SUBMIT(e2::release, bttn::click::middle, gear)
            {
                if (!secure)
                {
                    base::detach();
                    gear.dismiss();
                }
            };

            //todo revise/what is this? history?
            SUBMIT(e2::preview, e2::form::proceed::detach, shadow)
            {
                if (client == shadow)
                {
                    client.reset();
                    auto parent = This();
                    shadow->SIGNAL(e2::release, e2::form::upon::detached, parent);
                }
            };
        }
        // mold: Draw client window.
        virtual void renderproc (face& parent_canvas)
        {
            auto const& normal = base::brush;
            auto fuse_normal = [&](cell& c) { c.fuse(normal); };
            //todo unify - make pro::theme
            acryl = 5;// 100;

            auto& states = shared.states();
            //todo temporarily use locked for old menu
            //if (states.empty()) //if (loosen)
            if (locked || (states.empty() && !active)) //if (loosen)
            {
                if (base::brush.bga() == 0xFF) parent_canvas.fill(fuse_normal);
                else if (base::brush.wdt())    parent_canvas.blur(acryl, fuse_normal);
                else                           parent_canvas.blur(acryl);
            }
            else
            {
                auto bright = skin::color(tone::brighter);
                auto shadow = skin::color(tone::shadower);
                shadow.alpha(bright.bga());

                bool isnorm =
                    !active && states.end() == std::find_if(states.begin(), states.end(),
                                                [](auto& a) { return a.wholly; });
                auto guides = [&](auto bright)
                {
                    for (auto& grip : states)
                        if (grip)
                            grip.draw(parent_canvas, bright);
                };
                auto fillup = [&](auto bright, auto shadow)
                {
                    parent_canvas.fill(shadow);
                    guides(bright);
                };
                auto fuse_bright = [&](cell& c) { c.fuse(normal); c.fuse(bright); };
                auto fuse_shadow = [&](cell& c) { c.fuse(normal); c.fuse(shadow); };

                auto only_bright = [&](cell& c) { c.fuse(bright); };
                auto only_shadow = [&](cell& c) { c.fuse(shadow); };

                if (!isnorm) bright.alpha(bright.bga() >> 1); // too bright when selected
                if (normal.bgc().alpha())
                {
                    if (isnorm) fillup(fuse_bright, fuse_normal);
                    else        fillup(fuse_shadow, fuse_bright);
                }
                else
                {
                    if (isnorm) guides(fuse_bright);
                    else        fillup(only_shadow, only_bright);
                }

                // Draw kb focus
                if (active)
                {
                    // Draw the border around
                    auto area = parent_canvas.client_area();
                    auto mark = skin::color(tone::kb_focus);
                    auto fill = [&](cell& c) { c.fuse(mark); };
                    parent_canvas.cage(area, gripsz, fill);
                }

                //todo make it selectable
                //if (base::brush.bga() != 0xFF)
                {
                    parent_canvas.blur(acryl);
                }

            }

            if (base::status.invalid)
            {
                canvas.wipe();
                canvas.render(client, base::coor.get());
                SIGNAL(e2::release, e2::form::upon::redrawn, canvas); // to draw the title and footer
            }
            parent_canvas.plot(canvas);
        }

        // mold: Get client region.
        auto get_region()
        {
            return region;
        }
        // mold: Set the window undeleteble.
        void strong(bool immortal)
        {
            secure = immortal;
        }
        // mold: Set window title.
        void header(view title)
        {
            legend.header(title);
        }
        //todo unify, default = true
        void liquid(bool resizeble)
        {
            locked = !resizeble;
        }
        // mold: Create a new item of the specified subtype and attach it.
        template<class T, class ...Args>
        auto attach(Args... args)
        {
            static_assert(std::is_base_of<base, T>::value,
                "The only a derivative of the «base» class can be attached to the «mold».");

            auto item = base::create<T>(args...);
            client = item;
            auto creator = This();
            item->SIGNAL(e2::release, e2::form::upon::attached, creator);

            region.coor = gripsz;
            item->SIGNAL(e2::release, e2::form::layout::move, region.coor);
            base::reflow(); // Ask client about the new size (the client can override the size)

            //todo unify
            //std::shared_ptr<core> canvas_ptr = item->canvas.shared_from_this();
            std::shared_ptr<core> canvas_ptr = canvas.shared_from_this();
            SIGNAL(e2::general, e2::form::canvas, canvas_ptr);

            return item;
        }
    };

    class fork // controls.h: Splitter control.
        : public base
    {
        using sptr = netxs::sptr<base>;
    public:
        using self = fork;
        FEATURE(pro::align, align); // fork: Size linking controller.
        FEATURE(pro::mouse, mouse); // fork: Mouse controller.

        enum slot { _1, _2 };
        enum orientation { horizontal, vertical };
    private:
        enum action { seize, drag, release };

        static constexpr iota MAX_RATIO = 0xFFFF;
        static constexpr iota HALF_RATIO = 0xFFFF >> 1;

        sptr client_1; // fork: Client 1 object.
        sptr client_2; // fork: Client 2 object.

        twod size1;
        twod size2;
        twod coor2;

        tint clr;
        twod size;
        rect stem;
        iota start;
        iota width;
        iota ratio;
        iota maxpos;
        bool updown;
        bool movable;

        twod xpose(twod const& pt) { return updown ? twod{ pt.y, pt.x } : pt; }
        iota get_x(twod const& pt) { return updown ? pt.y : pt.x; }
        iota get_y(twod const& pt) { return updown ? pt.x : pt.y; }

    public:
        ~fork()
        {
            if (client_1) client_1->base::detach();
            if (client_2) client_2->base::detach();
        }
        fork()
            : maxpos{ 0 },
            start{ 0 },
            width{ 0 },
            movable{ true },
            updown{ faux },
            ratio{ 0xFFFF >> 1 }
        {

            //mouse.highlightable = true;

            SUBMIT(e2::preview, e2::form::layout::size, new_size)
            {
                fork::resize(new_size);
            };
            SUBMIT(e2::release, e2::form::layout::size, new_size)
            {
                size = new_size;

                if (client_1)
                {
                    client_1->SIGNAL(e2::release, e2::form::layout::size, size1);
                }
                if (client_2)
                {
                    //todo use e2::form::layout::rect to minimize invocation overhead
                    client_2->SIGNAL(e2::release, e2::form::layout::move, coor2);
                    client_2->SIGNAL(e2::release, e2::form::layout::size, size2);
                }
            };

            //  case WM_SIZE:
            //  	entity->resize({ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
            //  	return 0;
            //  case WM_PAINT:
            //  	entity->draw();
            //  	return 0;
            //  case WM_SETCURSOR:
            //  	if (LOWORD(lParam) == HTCLIENT)
            //  	{
            //  		return entity->showcursor();
            //  	}
            //  	return 0;
            //  case WM_APP_LIMITSCHANGED:
            //  	entity->calclimits();
            //  	return 0;
            //  case WM_APP_CTXUPDATE:
            //  	entity->forward(WM_APP_CTXUPDATE);
            //  	return 0;
            //  case WM_APP_CONFIG:
            //  	entity->config((gui::orientation)wParam, LOWORD(lParam), HIWORD(lParam));
            //  	return 0;
            //  case WM_LBUTTONDBLCLK:
            //  	entity->toggle();
            //  	return 0;
            //  case WM_RBUTTONUP:
            //  	entity->rotate();
            //  	return 0;
            //  case WM_LBUTTONDOWN:
            //  	entity->slider(action::seize, { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
            //  	return 0;
            //  case WM_LBUTTONUP:
            //  	entity->slider(action::release, { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
            //  	return 0;
            //  case WM_MOUSEMOVE:
            //  	if (wParam & MK_LBUTTON)
            //  	{
            //  		entity->slider(action::drag, { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
            //  	}
            //  	return 0;
        }
        // base (virtual, auto): Draw the form composition on the specified canvas.
        virtual void renderproc (face& parent_canvas)
        {
            base::renderproc(parent_canvas);

            auto basis = base::coor.get();
            if (client_1) parent_canvas.render(client_1, basis);
            if (client_2) parent_canvas.render(client_2, basis);

            if (width)
            {
                //todo draw grips
            }
        }

        void rotate()
        {
            updown = !updown;
            //base::resize(size);
            base::reflow();

            //fork::resize(size);
            //takecursor();
        }
        //todo use axis:X/Y
        void config(orientation alignment, iota thickness, iota scale)
        {
            switch (alignment)
            {
                case orientation::horizontal:
                    updown = faux;
                    break;
                case orientation::vertical:
                    updown = true;
                    break;
                default:
                    updown = faux;
                    break;
            }
            width = std::max(thickness, 0);
            ratio = MAX_RATIO * std::clamp(scale, 0, 100) / 100;
            base::reflow();
        }

        void toggle()
        {
            if (movable)
            {
                switch (ratio)
                {
                    case 0:
                        ratio = HALF_RATIO;
                        break;
                    case MAX_RATIO:
                        ratio = HALF_RATIO + 1;
                        break;
                    default:
                        ratio = ratio > HALF_RATIO ? MAX_RATIO : 0;
                        break;
                }
                base::reflow();
            }
        }

        void resize(twod& new_size)
        {
            //todo revise/unify
            auto new_size0 = xpose(new_size);
            {
                maxpos = std::max(new_size0.x - width, 0);
                auto split = netxs::divround(maxpos * ratio, MAX_RATIO);

                size1 = xpose({ split, new_size0.y });
                if (client_1)
                {
                    auto& item = *client_1;
                    item.SIGNAL(e2::preview, e2::form::layout::size, size1);

                    split = get_x(size1);
                    new_size0.y = get_y(size1);
                }

                size2 = xpose({ maxpos - split, new_size0.y });
                auto test_size2 = size2;
                if (client_2)
                {
                    auto& item = *client_2;
                    item.SIGNAL(e2::preview, e2::form::layout::size, size2);
                    split = new_size0.x - width - get_x(size2);

                    if (test_size2 != size2) /// If the size2 is not suitable
                    {
                        new_size0.y = get_y(size2);
                        size1 = xpose({ split, new_size0.y });
                        if (client_1)
                        {
                            auto& item = *client_1;
                            item.SIGNAL(e2::preview, e2::form::layout::size, size1);

                            split = get_x(size1);
                            new_size0.y = get_y(size1);
                        }
                        size2 = xpose({ maxpos - split, new_size0.y });
                        if (client_2)
                        {
                            auto& item = *client_2;
                            item.SIGNAL(e2::preview, e2::form::layout::size, size2);
                            new_size0.y = get_y(size2);
                        }
                    }

                    coor2 = xpose({ split + width, 0 });
                }

                new_size = xpose({ split + width + get_x(size2), new_size0.y });
            }
        }

        void slider(action act, twod const& delta)
        {
            if (movable)
            {
                switch (act)
                {
                    case action::seize:
                        //control_w32::mouse(true);
                        start = get_x(stem.coor) - get_x(delta);
                        break;
                    case action::drag:
                        //if (!control_w32::mouse())
                        //{
                        //	return;
                        //}
                        break;
                    case action::release:
                        //if (!control_w32::mouse())
                        //{
                        //	return;
                        //}
                        //control_w32::mouse(false);
                        break;
                    default:
                        return;
                }

                //todo move slider

                //fork::deploy(start + xpose(delta).x, true);
                //iota newpos = start + xpose(delta).x;
                //
                //auto c1 = base::limit(ctrl::_1);
                //auto c2 = base::limit(ctrl::_2);
                //
                //auto minsplit = std::max(get_x(c1.min), maxpos - get_x(c2.max));
                //auto maxsplit = std::min(get_x(c1.max), maxpos - get_x(c2.min));
                //auto split = std::clamp(newpos, minsplit, std::max(minsplit, maxsplit));
                //
                //stem = { xpose({ split, 0 }), xpose({ width, get_y(size) }) };
                //if (client_1)
                //	client_1->extend(
                //		rect{ dot_00,                      xpose({ split,          get_y(size) }) });
                //if (client_2)
                //	client_2->extend(
                //		rect{ xpose({ split + width, 0 }), xpose({ maxpos - split, get_y(size) }) });

                ratio = std::clamp(netxs::divround(get_x(stem.coor) * MAX_RATIO, maxpos), 0, MAX_RATIO);

                base::deface();
            }
        }
        // fork: Create a new item of the specified subtype and attach it to a specified slot.
        template<class T, class ...Args>
        auto attach(slot client_slot, Args... args)
        {
            static_assert(std::is_base_of<base, T>::value,
                "The only a derivative of the «base» class can be attached to the «mold».");

            auto item = create<T>(args...);
            if (client_slot == slot::_1) client_1 = item;
            else                         client_2 = item;

            auto creator = This();
            item->SIGNAL(e2::release, e2::form::upon::attached, creator);
            base::reflow(); // Ask the client about the new size (the client can override the size)
            return item;
        }
    };

    class list // controls.h: Vertical/horizontal list control
        : public base
    {
        using self = list;
        FEATURE(pro::mouse, mouse); // list: Mouse controller.
        FEATURE(pro::align, align); // list: Size linking controller.

        using sptr = netxs::sptr<base>;
        using roll = std::list<std::pair<sptr, iota>>;

        roll clients;
        bool up_down = true;

    public:
        ~list()
        {
            while (clients.size())
            {
                clients.back().first->base::detach();
                clients.pop_back();
            }
        }
        list()
        {
            SUBMIT(e2::preview, e2::form::layout::size, new_sz)
            {
                iota  height;
                auto& y_size = up_down ? new_sz.y : new_sz.x;
                auto& x_size = up_down ? new_sz.x : new_sz.y;
                auto  x_temp = x_size;

                auto meter = [&]() {
                    height = 0;
                    for (auto& client : clients)
                    {
                        y_size = 0;
                        client.first->SIGNAL(e2::preview, e2::form::layout::size, new_sz);
                        client.second = y_size;
                        height += y_size;
                    }
                };
                meter(); if (x_temp != x_size) meter();

                y_size = height;
            };
            SUBMIT(e2::release, e2::form::layout::size, new_sz)
            {
                //todo optimize avoid SIGNAL if size/coor is unchanged
                auto& y_size = up_down ? new_sz.y : new_sz.x;
                auto& x_size = up_down ? new_sz.x : new_sz.y;
                twod  new_xy;
                auto& y_coor = up_down ? new_xy.y : new_xy.x;
                auto& x_coor = up_down ? new_xy.x : new_xy.y;

                auto  found = faux;
                for (auto& client : clients)
                {
                    y_size = client.second;
                    if (client.first)
                    {
                        auto& entry = *client.first;
                        if (!found)
                        {
                            //todo optimize: use the only one axis to hittest
                            //todo detect client during preview, use wptr
                            auto anker = entry.base::square();
                            if (anker.hittest(base::anchor))
                            {
                                found = true;
                                base::anchor += new_xy - anker.coor;
                            }
                        }

                        entry.SIGNAL(e2::release, e2::form::layout::move, new_xy);
                        entry.SIGNAL(e2::release, e2::form::layout::size, new_sz);
                    }
                    y_coor+= client.second;
                }
            };
        }
        // list (virtual, auto): Draw the form composition on the specified canvas.
        virtual void renderproc (face& parent_canvas)
        {
            base::renderproc(parent_canvas);

            auto basis = base::coor.get();
            for (auto& client : clients)
            {
                parent_canvas.render(client.first, basis);
            }
        }
        // list: Create a new item of the specified subtype and attach it.
        template<class T, class ...Args>
        auto attach(Args... args)
        {
            static_assert(std::is_base_of<base, T>::value,
                "The only a derivative of the «base» class can be attached to the «mold».");

            auto item = create<T>(args...);
            clients.push_back({ item, 0 });
            //heights.push_back(0);

            auto creator = This();
            item->SIGNAL(e2::release, e2::form::upon::attached, creator);
            base::reflow(); // Ask the client about the new size (the client can override the size)
            return item;
        }
    };

    class cake // controls.h: (puff) Layered cake of forms on top of each other.
        : public base
    {
        using self = cake;
        FEATURE(pro::mouse, mouse); // cake: Mouse controller.
        FEATURE(pro::align, align); // cake: Size linking controller.

        using sptr = netxs::sptr<base>;
        std::list<sptr> clients;

    public:
        ~cake()
        {
            while (clients.size())
            {
                clients.back()->base::detach();
                clients.pop_back();
            }
        }
        cake()
        {
            SUBMIT(e2::preview, e2::form::layout::size, newsz)
            {
                for (auto& client : clients)
                {
                    if (client)
                    {
                        auto& object = *client;
                        //object.base::anchor = base::anchor;
                        object.SIGNAL(e2::preview, e2::form::layout::size, newsz);
                        //base::anchor = object.base::anchor;
                    }
                }
            };
            SUBMIT(e2::release, e2::form::layout::size, newsz)
            {
                for (auto& client : clients)
                {
                    if (client)
                    {
                        auto& object = *client;
                        object.SIGNAL(e2::release, e2::form::layout::size, newsz);
                    }
                }
            };
        }
        // cake (virtual, auto): Draw the form composition on the specified canvas.
        virtual void renderproc (face& parent_canvas)
        {
            base::renderproc(parent_canvas);

            auto basis = base::coor.get();
            for (auto& client : clients)
            {
                parent_canvas.render(client, basis);
            }
        }

        // cake: Create a new item of the specified subtype and attach it.
        template<class T, class ...Args>
        auto attach(Args... args)
        {
            static_assert(std::is_base_of<base, T>::value,
                "The only a derivative of the «base» class can be attached to the «mold».");

            auto item = create<T>(args...);
            clients.push_back(item);

            auto creator = This();
            item->SIGNAL(e2::release, e2::form::upon::attached, creator);
            base::reflow(); // Ask the client about the new size (the client can override the size)
            return item;
        }
    };



    struct page_layout
    {
        struct item
        {
            id_t id;
            twod coor;
        };

        size_t len = 0;

        static const iota init_layout_len = 10000;
        std::vector<item> layout;
        page_layout()
        {
            layout.reserve(init_layout_len);
        }

        auto get_entry(twod const& anchor)
        {
            auto& anker = anchor.y;
            item pred = { 0, twod{ 0, std::numeric_limits<iota>::max() } };
            item minp = { 0, twod{ 0, std::numeric_limits<iota>::max() } };
            iota mindist = std::numeric_limits<iota>::max();

            //todo optimize, use binary search
            //start from the end
            for (auto& p : layout)
            {
                auto& post = p.coor.y;
                if (pred.coor.y <= anker && post > anker) // inside the entry
                {
                    return pred;
                }
                else
                {
                    auto dist = std::abs(anker - post);
                    if (dist < mindist)
                    {
                        minp = p;
                        mindist = dist;
                    }
                }
                pred = p;
            }
            return minp;
        }
        auto begin() { return layout.begin(); }
        auto capacity() { return layout.capacity(); }
        void reserve(size_t newsize) { layout.reserve(newsize); }
        void push_back(item const& p) { layout.push_back(p); }
        void clear() { layout.clear(); }
    };

    class post // controls.h: Rigid text page.
        : public base,
          public flow
    {
        using self = post;
        FEATURE(pro::mouse, mouse); // post: Mouse controller.

        twod width; // post: Page dimensions.
        //flow frame{ width };
        //cell brush; // post: The nearest to top paragraph.

        page_layout layout;

    public:
        page topic; // post: Text content.

        // post: Print page.
        void output(face& canvas)
        {
            flow::reset();
            //flow::origin = {};
            flow::corner = canvas.corner();
            auto publish = [&](auto const& combo)
            {
                flow::print(combo, canvas);
                //brush = combo.mark(); // current mark of the last printed fragment
            };
            topic.stream(publish);
        }
        auto get_size() const
        {
            return width;
        }
        void recalc()
        {
            if (topic.size() > layout.capacity())
                layout.reserve(topic.size() * 2);

            auto entry = layout.get_entry(base::anchor); // Take the object under central point
            layout.clear();

            flow::reset();
            auto publish = [&](auto const& combo)
            {
                auto cp = flow::print(combo);

                auto id = combo.id();
                if (id == entry.id) entry.coor.y -= cp.y;
                layout.push_back({ id,cp });
            };
            topic.stream(publish);

            // Apply only vertical anchoring for this type of control
            base::anchor.y -= entry.coor.y; // Move the central point accordingly to the anchored object

            auto& cover = flow::minmax();
            base::oversize.set(-std::min(0, cover.l),
                                std::max(0, cover.r - width.x + 1),
                               -std::min(0, cover.t),
                                0);
            width.y = cover.height() + 1;
        }
        void recalc(twod const& size)
        {
            width.x = size.x;
            recalc();
        }

        post()
            : flow{ width }
        {
            SUBMIT(e2::preview, e2::form::layout::size, size)
            {
                recalc(size);
                size.y = width.y;
            };

            SUBMIT(e2::release, e2::form::layout::size, size)
            {
                //if (width != size)
                //{
                //	recalc(size);
                //	//width.y = size.y;
                //}
                width = size;
            };
        }

        // post: Render base and output topic content.
        virtual void renderproc (face& parent_canvas)
        {
            base::renderproc(parent_canvas);
            output(parent_canvas);

            SIGNAL(e2::release, e2::form::upon::redrawn, parent_canvas);

            //auto mark = rect{ base::anchor + base::coor.get(), {10,5} };
            //mark.coor += parent_canvas.view().coor; // Set client's basis
            //parent_canvas.fill(mark, [](cell& c) { c.alpha(0x80).bgc().chan.r = 0xff; });
        }
    };

    class rail // controls.h: Scroller.
        : public base
    {
        using sptr = netxs::sptr<base>;
        using self = rail;
        FEATURE(pro::mouse, mouse); // rail: Mouse controller.
        FEATURE(pro::robot, robot); // rail: Animation controller.
        FEATURE(pro::align, align); // rail: Size linking controller.

        static constexpr hint events[] = { e2::form::upon::scroll::x,
                                           e2::form::upon::scroll::y,
                                           e2::form::upon::scroll::resetx,
                                           e2::form::upon::scroll::resety };
        bool locked = faux; // rail: Client is under resizing.
        sptr client; // rail: Client instance.
        subs tokens; // rail: Subscriptions on client moveto and resize.
        rack scinfo; // rail: Scroll info.
        bool strict[2] = { true, true }; // rail: Don't allow overscroll.
        bool manual[2] = { true, true }; // rail: Manaul scrolling (no auto align).
        axes permit; // rail: Allowed axes to scroll.
        axes siezed; // rail: Allowed axes to capture.

        iota speed; // rail: Text auto-scroll initial speed component ΔR.
        iota pulse; // rail: Text auto-scroll initial speed component ΔT.
        iota cycle; // rail: Text auto-scroll duration in ms.
        bool steer; // rail: Text scroll vertical direction.

    public:
        bool overscroll[2] = { true, true }; // rail: Allow overscroll with auto correct.

        rail(axes allow_to_scroll = axes::ALL, axes allow_to_capture = axes::ALL)
            : speed{ SPD  },
              pulse{ PLS  },
              cycle{ CCL  },
              steer{ faux },
            permit{ allow_to_scroll },
            siezed{ allow_to_capture }
        {
            // Receive scroll parameters from external source
            SUBMIT(e2::preview, e2::form::upon::scroll::any, info)
            {
                if (client)
                {
                    auto& item = *client;
                    switch (bell::protos<e2::preview>())
                    {
                        case events[X]:
                            scroll<X>(scinfo.window.coor.x - info.window.coor.x);
                            break;
                        case events[Y]:
                            scroll<Y>(scinfo.window.coor.y - info.window.coor.y);
                            break;
                        case events[X + 2]:
                            cancel<X, true>();
                            break;
                        case events[Y + 2]:
                            cancel<Y, true>();
                            break;
                    }
                }
            };
            SUBMIT(e2::request, e2::form::upon::scroll::any, req_scinfo)
            {
                req_scinfo = scinfo;
            };
            SUBMIT(e2::release, e2::form::layout::size, new_size)
            {
                if (client)
                {
                    locked = true; // see subscription in the attach()
                    auto delta = client->base::resize(new_size, base::anchor);
                    locked = faux;

                    scroll<X>(delta.x);
                    scroll<Y>(delta.y);
                }
            };

            using bttn = e2::hids::mouse::button;
            SUBMIT(e2::release, e2::hids::mouse::scroll::any, gear)
            {
                auto dir = gear.whldt > 0;
                if (permit == axes::ONLY_X || gear.meta(hids::ANYCTRL |
                                                        hids::SHIFT ))
                     wheels<X>(dir);
                else wheels<Y>(dir);

                gear.dismiss();
            };
            SUBMIT(e2::release, bttn::drag::start::right, gear)
            {
                auto ds = gear.delta.get();
                auto dx = ds.x;
                auto dy = ds.y * 2;
                auto vt = std::abs(dx) < std::abs(dy);

                if (((siezed & axes::ONLY_X) && !vt) ||
                    ((siezed & axes::ONLY_Y) &&  vt))
                {
                    if (gear.capture(bell::id))
                    {
                        manual[X] = true;
                        manual[Y] = true;
                        strict[X] = !overscroll[X];
                        strict[Y] = !overscroll[Y];
                        gear.dismiss();
                    }
                }
            };
            SUBMIT(e2::release, bttn::drag::pull::right, gear)
            {
                if (gear.captured(bell::id))
                {
                    auto delta = gear.mouse::delta.get();
                    if (permit & axes::ONLY_X) scroll<X>(delta.x);
                    if (permit & axes::ONLY_Y) scroll<Y>(delta.y);
                    gear.dismiss();
                }
            };
            SUBMIT(e2::release, bttn::drag::cancel::right, gear)
            {
                if (gear.captured(bell::id))
                {
                    cancel<X>();
                    cancel<Y>();
                    base::deface();
                    gear.release();
                    gear.dismiss();
                }
            };
            SUBMIT(e2::release, bttn::drag::stop::right, gear)
            {
                if (gear.captured(bell::id))
                {
                    auto  v0 = gear.delta.avg();
                    auto& speed = v0.dS;
                    auto  start = datetime::round<iota>(v0.t0);
                    auto  cycle = datetime::round<iota>(v0.dT);
                    auto  limit = datetime::round<iota>(STOPPING_TIME);

                    if (permit & axes::ONLY_X) actify<X>(quadratic{ speed.x, cycle, limit, start });
                    if (permit & axes::ONLY_Y) actify<Y>(quadratic{ speed.y, cycle, limit, start });

                    base::deface();
                    gear.release();
                    gear.dismiss();
                }
            };
            SUBMIT(e2::release, bttn::down::any, gear)
            {
                if (manual[X]) robot.pacify(X);
                if (manual[Y]) robot.pacify(Y);
            };
            SUBMIT(e2::release, bttn::click::right, gear)
            {
                if (!gear.captured(bell::id))
                {
                    if (manual[X]) cancel<X, true>();
                    if (manual[Y]) cancel<Y, true>();
                    gear.dismiss();
                }
            };
            SUBMIT(e2::release, e2::form::animate::stop, id)
            {
                switch (id)
                {
                    case X:
                        manual[X] = true;
                        scroll<X>();
                        break;
                    case Y:
                        manual[Y] = true;
                        scroll<Y>();
                        break;
                    default:
                        break;
                }
                deface();
            };
        }
        template<axis AXIS>
        void wheels(bool dir)
        {
            if (robot.active(AXIS) && (steer == dir))
            {
                speed += SPD_ACCEL;
                cycle += CCL_ACCEL;
                speed = std::min(speed, SPD_MAX);
                cycle = std::min(cycle, CCL_MAX);
            }
            else
            {
                steer = dir;
                speed = SPD;
                cycle = CCL;
            }
            scroll<AXIS>(dir ? 1 : -1);
            keepon<AXIS>(quadratic<iota>(dir ? speed : -speed, pulse, cycle, now<iota>()));
        }
        template<axis AXIS, class FX>
        void keepon(FX&& func)
        {
            strict[AXIS] = true;
            robot.actify<AXIS>(func, [&](auto& p)
                {
                    scroll<AXIS>(p);
                });
        }
        template<axis AXIS, class FX>
        void actify(FX&& func)
        {
            auto inside = scroll<AXIS>();
            if  (inside)  keepon<AXIS>(func);
            else          lineup<AXIS>();
        }
        template<axis AXIS, bool FORCED = faux>
        void cancel()
        {
            if constexpr (FORCED) lineup<AXIS>();
            else
            {
                auto inside = scroll<AXIS>();
                if (!inside)  lineup<AXIS>();
            }
        }
        template<axis AXIS>
        void lineup()
        {
            if (client)
            {
                manual[AXIS] = faux;
                auto level = AXIS == X;
                auto block = client->base::square();
                auto coord = level ? block.coor.x
                                   : block.coor.y;
                auto bound = level ? std::min(base::size.get().x - block.size.x, 0)
                                   : std::min(base::size.get().y - block.size.y, 0);
                auto newxy = std::clamp(coord, bound, 0);

                auto route = newxy - coord;
                iota tempo = SWITCHING_TIME;
                auto fader = constlinearAtoB<iota>(route, tempo, now<iota>());
                keepon<AXIS>(fader);
            }
        }
        template<axis AXIS>
        bool scroll(iota delta = {})
        {
            bool inside = true;

            if (client)
            {
                auto& thing = *client;
                auto  block = thing.base::square();
                auto  basis = thing.oversize.topleft();
                block.coor -= basis; // Scroll origin basis
                block.size += thing.oversize.summ();
                auto& frame = base::size.get();
                auto  level = AXIS == X;
                auto  bound = level ? std::min(frame.x - block.size.x, 0)
                                    : std::min(frame.y - block.size.y, 0);
                auto& coord = level ? block.coor.x
                                    : block.coor.y;
                coord += delta;

                if (manual[AXIS]) // check overscroll if no auto correction
                {
                    auto clamp = std::clamp(coord, bound, 0);
                    inside = clamp == coord;
                    if (!inside && strict[AXIS]) // if outside the scroll limits
                    {                            // and overscroll is not allowed
                            coord = clamp;
                    }
                }

                scinfo.beyond = thing.oversize;  // Oversize value
                scinfo.region = block.size;
                scinfo.window.coor =-block.coor; // Viewport
                scinfo.window.size = frame;      //
                SIGNAL(e2::release, events[AXIS], scinfo);

                block.coor += basis; // Client origin basis
                locked = true;
                thing.base::moveto(block.coor);
                locked = faux;
                deface();
            }

            return inside;
        }
        // rail (virtual, auto): Draw the form composition on the specified canvas.
        virtual void renderproc (face& parent_canvas)
        {
            base::renderproc(parent_canvas);
            if (client) parent_canvas.render<faux>(client, base::coor.get());
        }
        // rail: Create a new item of the specified subtype and attach it.
        template<class T, class ...Args>
        auto attach(Args... args)
        {
            static_assert(std::is_base_of<base, T>::value,
                "The only a derivative of the «base» class can be attached to the «mold».");

            auto item = create<T>(args...);
            client = item;

            tokens.clear();
            item->SUBMIT_T(e2::release, e2::form::layout::size, tokens.extra(), size)
            {
                if (!locked)
                {
                    scroll<X>();
                    scroll<Y>();
                }
            };
            item->SUBMIT_T(e2::release, e2::form::upon::detached, tokens.extra(), p)
            {
                scinfo.region = {};
                scinfo.window.coor = {};
                SIGNAL(e2::release, events[axis::X], scinfo);
                SIGNAL(e2::release, events[axis::Y], scinfo);
                tokens.clear();
            };

            auto creator = This();
            item->SIGNAL(e2::release, e2::form::upon::attached, creator);
            base::reflow(); // Ask the client about the new size (the client can override the size)
            return item;
        }
    };

    template<axis AXIS>
    class grip // controls.h: Scroll bar.
        : public base
    {
        using wptr = netxs::wptr<bell>;
        using sptr = netxs::sptr<bell>;

        using self = grip;
        FEATURE(pro::mouse, mouse); // grip: Mouse events controller.
        FEATURE(pro::align, align); // grip: Size linking controller.
        FEATURE(pro::timer, timer); // grip: Minimize by timeout.

        enum activity
        {
            mouse_leave = 0, // faux
            mouse_hover = 1, // true
            pager_first = 10,
            pager_next  = 11,
        };
        static constexpr hint events[] = { e2::form::upon::scroll::x,
                                           e2::form::upon::scroll::y,
                                           e2::form::upon::scroll::resetx,
                                           e2::form::upon::scroll::resety };
        static inline auto  xy(twod const& p) { return AXIS == axis::X ? p.x : p.y; }
        static inline auto  yx(twod const& p) { return AXIS == axis::Y ? p.x : p.y; }
        static inline auto& xy(twod&       p) { return AXIS == axis::X ? p.x : p.y; }
        static inline auto& yx(twod&       p) { return AXIS == axis::Y ? p.x : p.y; }

        struct math
        {
            rack  master_inf = {};                        // math: Master scroll info.
            iota& master_len = xy(master_inf.region);      // math: Master len.
            iota& master_pos = xy(master_inf.window.coor); // math: Master viewport pos.
            iota& master_box = xy(master_inf.window.size); // math: Master viewport len.
            iota  scroll_len = 0; // math: Scrollbar len.
            iota  scroll_pos = 0; // math: Scrollbar grip pos.
            iota  scroll_box = 0; // math: Scrollbar grip len.
            iota   m         = 0; // math: Master max pos.
            iota   s         = 0; // math: Scroll max pos.
            double r         = 1; // math: Scroll/master len ratio.

            iota  cursor_pos = 0; // math: Mouse cursor position.

            // Calc scroll to master metrics
            void s_to_m()
            {
                auto scroll_center = scroll_pos + scroll_box / 2.0;
                auto master_center = scroll_len ? scroll_center / r
                                                : 0;
                master_pos = (iota)std::round(master_center - master_box / 2.0);

                // Reset to extreme positions
                if (scroll_pos == 0 && master_pos > 0) master_pos = 0;
                if (scroll_pos == s && master_pos < m) master_pos = m;

                //log (" master max pos ", m);
                //log (" scroll max pos ", s);
                //log (" ratio ", r);
                //
                //log ("master_pos ", master_pos);
                //log ("scroll_pos ", scroll_pos);
                //log ("--------------------------------------");
            }
            // Calc master to scroll metrics
            void m_to_s()
            {
                r = (double)scroll_len / master_len;
                auto master_middle = master_pos + master_box / 2.0;
                auto scroll_middle = master_middle * r;
                //scroll_box = 1 + (iota)(master_box * r);
                //scroll_box = (iota)(master_box * r);
                scroll_box = std::max(1, (iota)(master_box * r));
                scroll_pos = (iota)std::round(scroll_middle - scroll_box / 2.0);

                // Don't place the grip behind the scrollbar
                if (scroll_pos >= scroll_len) scroll_pos = scroll_len - 1;

                // Extreme positions are always closed last
                s = scroll_len - scroll_box;
                m = master_len - master_box;

                //log (" scroll_len ", scroll_len);
                //log (" scroll_box ", scroll_box);
                //log (" master_len ", master_len);
                //log (" master_box ", master_box);
                //log (" --------------------------- m_to_s");

                if (scroll_len > 2) // Two-row hight is not suitable for this type of aligning
                {
                    if (scroll_pos == 0 && master_pos > 0) scroll_pos = 1;
                    if (scroll_pos == s && master_pos < m) scroll_pos = s - 1;
                }
            }
            void update(rack const& scinfo)
            {
                master_inf = scinfo;
                //if (AXIS == axis::X)
                //{
                //	log (" --------------------------- scinfo");
                //	log(" region: ", scinfo.region);
                //	log(" window: ", scinfo.window);
                //	log(" beyond: ", scinfo.beyond);
                //}
                m_to_s();
            }
            void resize(twod const& new_size)
            {
                scroll_len = xy(new_size);
                m_to_s();
            }
            void stepby(iota delta)
            {
                scroll_pos = std::clamp(scroll_pos + delta, 0, s);
                //scroll_pos = scroll_pos + delta;
                s_to_m();
            }
            void commit(rect& handle)
            {
                xy(handle.coor)+= scroll_pos;
                xy(handle.size) = scroll_box;
            }
            auto inside(iota coor)
            {
                if (coor >= scroll_pos + scroll_box) return 1; // Below the grip
                if (coor >= scroll_pos)              return 0; // Inside the grip
                                                     return-1; // Above the grip
            }
            void pager(iota dir)
            {
                master_pos += master_box * dir;
                m_to_s();
            }
            auto follow()
            {
                auto dir = scroll_len > 2 ? inside(cursor_pos)
                                          : cursor_pos > 0 ? 1 // Don't stop to follow over
                                                           :-1;//    box on small scrollbar
                if (dir)
                {
                    pager(dir);
                    return true;
                }
                return faux;
            }
        };

        wptr boss;
        hook memo;
        iota thin; // grip: Scrollbar thickness.
        iota init; // grip: Handle base width.
        math calc; // grip: Scrollbar calculator.
        bool wide; // grip: Is the scrollbar active.

        bool on_pager = faux;

        template<hint EVENT = events[AXIS]>
        void send()
        {
            if (auto master = this->boss.lock())
            {
                master->SIGNAL(e2::preview, EVENT, calc.master_inf);
            }
        }
        void gohome()
        {
            send<events[AXIS + 2]>();
        }
        void config(iota width)
        {
            thin = width;
            auto limit = twod{ xy({ -1,thin }), yx({ -1,thin }) };
            base::limits(limit, limit);
        }

        auto pager_repeat()
        {
            if (on_pager && calc.follow())
            {
                send<events[AXIS]>();
            }
            return on_pager;
        }
    public:
        grip(sptr boss, iota thin = 1)
            : boss{ boss },
              thin{ thin },
              wide{ faux },
              init{ thin }
        {
            mouse.highlightable = true;

            config(thin);

            boss->SUBMIT_T(e2::release, events[AXIS], memo, scinfo)
            {
                calc.update(scinfo);
                base::deface();
            };

            SUBMIT(e2::release, e2::form::layout::size, new_size)
            {
                calc.resize(new_size);
            };

            using bttn = e2::hids::mouse::button;
            SUBMIT(e2::release, e2::hids::mouse::scroll::any, gear)
            {
                if (gear.whldt)
                {
                    auto dir = gear.whldt < 0 ? 1 : -1;
                    calc.pager(dir);
                    send<events[AXIS]>();
                    gear.dismiss();
                }
            };
            SUBMIT(e2::release, e2::hids::mouse::move, gear)
            {
                calc.cursor_pos = xy(gear.mouse::coord);
            };
            SUBMIT(e2::release, bttn::dblclick::left, gear)
            {
                gear.dismiss(); // Do not pass double clicks outside
            };
            //SUBMIT(e2::release, bttn::down::left, gear)
            SUBMIT(e2::release, bttn::down::any, gear)
            {
                if (!on_pager)
                if (bell::protos<e2::release>(bttn::down::left ) ||
                    bell::protos<e2::release>(bttn::down::right))
                if (auto dir = calc.inside(xy(gear.mouse::coord)))
                {
                    if (gear.capture(bell::id))
                    {
                        on_pager = true;
                        pager_repeat();
                        gear.dismiss();

                        timer.template actify<activity::pager_first>(REPEAT_DELAY, [&](auto p)
                        {
                            if (pager_repeat())
                            {
                                timer.template actify<activity::pager_next>(REPEAT_RATE, [&](auto d)
                                {
                                    return pager_repeat(); // Repeat until on_pager
                                });
                            }
                            return faux; // One shot call (first)
                        });
                    }
                }
            };
            SUBMIT(e2::release, bttn::up::any, gear)
            {
                if (on_pager && gear.captured(bell::id))
                {
                    if (bell::protos<e2::release>(bttn::up::left) ||
                        bell::protos<e2::release>(bttn::up::right))
                    {
                        gear.release();
                        gear.dismiss();
                        on_pager = faux;
                        timer.pacify(activity::pager_first);
                        timer.pacify(activity::pager_next);
                    }
                }
            };
            SUBMIT(e2::release, bttn::up::right, gear)
            {
                //if (!gear.captured(bell::id)) //todo why?
                {
                    gohome();
                    gear.dismiss();
                }
            };

            SUBMIT(e2::release, bttn::drag::start::any, gear)
            {
                if (on_pager) gear.dismiss();
                else
                {
                    if (gear.capture(bell::id))
                    {
                        gear.dismiss();
                    }
                }
            };
            SUBMIT(e2::release, bttn::drag::pull::any, gear)
            {
                if (on_pager) gear.dismiss();
                else
                {
                    if (gear.captured(bell::id))
                    {
                        if (auto delta = xy(gear.mouse::delta.get()))
                        {
                            calc.stepby(delta);
                            send<events[AXIS]>();
                            gear.dismiss();
                        }
                    }
                }
            };
            SUBMIT(e2::release, bttn::drag::cancel::any, gear)
            {
                if (on_pager) gear.dismiss();
                else
                {
                    if (gear.captured(bell::id))
                    {
                        if (bell::protos<e2::release>(bttn::drag::cancel::right))
                        {
                            gohome();
                        }
                        base::deface();
                        gear.release();
                        gear.dismiss();
                    }
                }
            };
            SUBMIT(e2::release, bttn::drag::stop::any, gear)
            {
                if (on_pager) gear.dismiss();
                else
                {
                    if (gear.captured(bell::id))
                    {
                        if (bell::protos<e2::release>(bttn::drag::stop::right))
                        {
                            gohome();
                        }
                        base::deface();
                        gear.release();
                        gear.dismiss();
                    }
                }
            };
            SUBMIT(e2::release, e2::form::state::mouse, active)
            {
                auto apply = [&](auto active)
                {
                    wide = active;
                    if (AXIS == axis::Y) config(active ? init * 2 // Make vertical scrollbar
                                                       : init);   //  wider on hover
                    base::reflow();
                    return faux; // One shot call
                };

                timer.pacify(activity::mouse_leave);

                if (active)          apply(activity::mouse_hover);
                else timer.template actify<activity::mouse_leave>(ACTIVE_TIMEOUT, apply);
            };
            //SUBMIT(e2::release, e2::hids::mouse::move, gear)
            //{
            //	auto apply = [&](auto active)
            //	{
            //		wide = active;
            //		if (AXIS == axis::Y) config(active ? init * 2 // Make vertical scrollbar
            //		                                   : init);   //  wider on hover
            //		base::reflow();
            //		return faux; // One shot call
            //	};
            //
            //	timer.pacify(activity::mouse_leave);
            //	apply(activity::mouse_hover);
            //	timer.template actify<activity::mouse_leave>(ACTIVE_TIMEOUT, apply);
            //};
        }
        // grip (virtual, auto): Draw the form composition on the specified canvas.
        virtual void renderproc (face& parent_canvas)
        {
            base::renderproc(parent_canvas);
            auto region = parent_canvas.view();
            auto handle = region;

            calc.commit(handle);

            auto& handle_len = xy(handle.size);
            auto& region_len = xy(region.size);

            handle = region.clip(handle);
            handle_len = std::max(1, handle_len);

            if (handle_len != region_len) // Show only if it is oversized
            {
                // Brightener isn't suitable for white backgrounds
                //auto bright = skin::color(tone::brighter);
                //bright.bga(bright.bga() / 2).fga(0);
                //bright.link(bell::id);

                if (wide) // Draw full scrollbar on mouse hover
                {
                    parent_canvas.fill([&](cell& c) { c.link(bell::id).xlight(); });
                }
                //parent_canvas.fill(handle, [&](cell& c) { c.fusefull(bright); });
                parent_canvas.fill(handle, [&](cell& c) { c.link(bell::id).xlight(); });
            }
        }
    };

    // DEPRECATED STUFF

    class pane
        : public form
    {
    public:
        using self = pane;
        FEATURE(pro::mouse, mouse);   // pane: Mouse controller.
        FEATURE(pro::robot, robot);   // pane: Animation controller.
        FEATURE(pro::align, adjust);  // pane: Size linking controller.
        //FEATURE(pro::keybd, keybd);   // pane: Keyboard controller.
        //FEATURE(pro::focus, focus);   // pane: Focus controller.
        //FEATURE(pro::align, align);   // pane: Align controller.
        //FEATURE(pro::caret, caret);   // pane: Cursor controller.

        //using sptr = utils::sptr<base>;

        page topic; // pane: Text content.
        //sptr client;

        iota speed; // pane: Text auto-scroll initial speed component ΔR.
        iota pulse; // pane: Text auto-scroll initial speed component ΔT.
        iota cycle; // pane: Text auto-scroll duration in ms.
        bool sdown; // pane: Text scroll vertical direction.

        hook logic; // pane/data: Data bind token.
        text keyinput;
        iota echosize = 2;
        iota bgclr = 0;

        iota iteration = 0;

        bool cursor_on = faux;
        bool scrollable = true;
        bool colored = true;     // reaction on mouse right click - change bg color
        side scroll_info;

        template<class T>
        auto& lyric(T paraid) { return *topic[paraid].lyric; }
        //auto& lyric(T paraid) { return topic[paraid].lyric; }

        //bool scrollable = true;

        //twod mouse_coord;
        ~pane()
        {
            log("pane: dtor ", this);
        }
        pane()
            : speed(SPD),
            pulse(PLS),
            cycle(CCL),
            sdown(faux)
        {
            //base::linked = true;

            //mouse.skipall(true);

            //transparent for mouse events
            //topic.last().link(0);

            using bttn = e2::hids::mouse::button;

            SUBMIT(e2::release, e2::form::upon::attached, p)
            {
                p->SUBMIT(e2::preview, e2::form::global::lucidity, alpha)
                {
                    auto a = std::clamp(alpha, 0, 255);
                    canvas.mark().alpha(a);
                    base::deface();
                    SIGNAL(e2::release, e2::form::global::lucidity, alpha);
                };

                //guide.show();
            };

            //SUBMIT(e2::release, e2::form::highlight::on, p)
            //{
            //	//invert();
            //};
            //SUBMIT(e2::release, e2::form::highlight::off, p)
            //{
            //	//revert();
            //};
            //SUBMIT(e2::release, e2::form::focus::got, p)
            //{
            //	//revert();
            //};

            SUBMIT(e2::release, e2::hids::mouse::scroll::up, gear)
            {
                if (scrollable)
                {
                    //todo unify
                    if (robot && !sdown)
                    {
                        speed += 3;
                        cycle += 30;
                    }
                    else
                    {
                        sdown = faux;
                        speed = SPD;
                        cycle = CCL;
                    }
                    scroll({ 0,1 });
                    auto func = quadratic<twod>({ 0,speed }, pulse, cycle, now<iota>());
                    robot.actify(func, [&](auto& x)
                        {
                            scroll(x);
                        });

                    gear.dismiss();
                }
            };
            SUBMIT(e2::release, e2::hids::mouse::scroll::down, gear)
            {
                if (scrollable)
                {
                    //todo unify
                    if (robot && sdown)
                    {
                        speed += 3;
                        cycle += 30;
                    }
                    else
                    {
                        sdown = true;
                        speed = SPD;
                        cycle = CCL;
                    }
                    scroll({ 0,-1 });
                    auto func = quadratic<twod>({ 0,-speed }, pulse, cycle, now<iota>());
                    robot.actify(func, [&](auto& x)
                        {
                            scroll(x);
                        });
                    gear.dismiss();
                }
            };
            SUBMIT(e2::release, bttn::drag::stop::left, gear)
            {
                if (gear.captured(bell::id))
                {
                    //todo revise
                    //cursor(gear.mouse.coord);
                }
            };
            SUBMIT(e2::release, bttn::click::left, gear)
            {
                //todo revise
                //cursor(gear.mouse.coord);
            };

#ifdef DEMO
            SUBMIT(e2::release, bttn::click::right, gear)
            {
                if (colored)
                {
                    color(canvas.mark().fgc(), (tint)((++bgclr) % 16));

                    base::deface();
                    gear.dismiss();
                }
            };
#endif

            SUBMIT(e2::release, e2::form::layout::size, size)
            {
                canvas.moved = true;

                //if (need_reflow)
                //{
                //	canvas.reflow(topic);
                //}

            };
            SUBMIT(e2::release, bttn::drag::start::right, gear)
            {
                if (scrollable)
                {
                    if (gear.capture(bell::id))
                    {
                        gear.dismiss();
                    }
                }
            };
            SUBMIT(e2::release, bttn::drag::pull::right, gear)
            {
                if (gear.captured(bell::id))
                {
                    scroll(gear.mouse::delta.get());
                    gear.dismiss();
                }
            };
            SUBMIT(e2::release, bttn::drag::cancel::right, gear)
            {
                if (gear.captured(bell::id))
                {
                    base::deface();
                    gear.release();
                    gear.dismiss();
                }
            };
            SUBMIT(e2::release, bttn::drag::stop::right, gear)
            {
                if (gear.captured(bell::id))
                {
                    robot.actify(gear.mouse::fader<quadratic<twod>>(2s), [&](auto& x)
                        {
                            scroll(x);
                        });

                    base::deface();
                    gear.release();
                    gear.dismiss();
                }
            };
            SUBMIT(e2::release, bttn::down::any, p)
            {
                robot.pacify();
            };

            SUBMIT(e2::release, e2::form::animate::stop, p)
            {
                deface();
            };

            //SUBMIT(e2::release, e2::hids::mouse::move, gear)
            //{
            //	log("== pane mouse move!");
            //};


            //SUBMIT(e2::release, e2::hids::mouse::move, p)
            //{
            //	deface();
            //});
            //SUBMIT(e2::release, e2::hids::mouse::any, gear)
            //{
            //	mouse_coord = gear.mouse.coord;
            //});
            //SUBMIT(e2::release, e2::hids::keybd::any, gear)
            //{
            //	topic += gear.keystrokes;
            //	deface();
            //
            //	//if (gear.keybd::down)
            //	//{
            //	//	gear.keybd::keystrokes = utf::to_utf(std::wstring(1, gear.keybd::widechar));
            //	//
            //	//	if (gear.keybd::keystrokes[0] == 'q')
            //	//	{
            //	//		if (focus.holder)
            //	//		{
            //	//			caret.hide();
            //	//		}
            //	//	}
            //	//	else
            //	//	{
            //	//		topic += gear.keybd::keystrokes;
            //	//		deface();
            //	//	}
            //	//}
            //});
        }

        virtual void redraw()
        {
            canvas.reflow(topic);

            /// In order to update mutable vars in the topic
            SIGNAL(e2::release, e2::form::upon::wiped, canvas);

            canvas.output(topic, true);

            //canvas.render(client);

            /// In order to show the cursor/caret
            SIGNAL(e2::release, e2::form::upon::redrawn, canvas);
        }

        void scroll(twod const& delta)
        {
            if (scrollable)
            {
                //auto cover = topic.minmax;

                auto cover = canvas.minmax();
                //todo revise
                cover.t++;

                //auto& basis = canvas.origin();
                auto basis = dot_00;// canvas.origin();

                if (delta.y > 0)
                {
                    auto& size = base::size.get();
                    basis.y += delta.y;
                    auto limit = size.y - cover.t;
                    if (basis.y > limit)
                    {
                        basis.y = limit;
                        robot.pacify();
                    }
                    deface();
                }
                else if (delta.y < 0)
                {
                    basis.y += delta.y;
                    auto limit = -cover.b;
                    if (basis.y < limit)
                    {
                        basis.y = limit;
                        robot.pacify();
                    }
                    deface();
                }

                scroll_info = cover;
                scroll_info.t -= basis.y;
                scroll_info.b -= basis.y;
            }
        }

        void invert()
        {
            canvas.mark().inv(true);
            deface();
        }
        void revert()
        {
            canvas.mark().inv(faux);
            deface();
        }
    };

    class chat
        : public pane
    {
        using self = pane;
        #ifndef DEMO
        FEATURE(pro::keybd, keybd); // chat: Keyboard controller.
        #endif // DEMO
        FEATURE(pro::caret, caret); // chat: Caret controller

    public:

        chat()
        {
#ifndef DEMO
            keybd.accept(true); // Subscribe on keybd offers
#endif
            SUBMIT(e2::release, e2::hids::keybd::any, gear)
            {
                //page input{ gear.keystrokes };
                //
                //auto& data = input.inwards();
                //
                //if (data.size() > 1)
                //{
                //	auto i = data.begin();
                //	topic[2] += *i;
                //
                //	while (++i != data.end())
                //	{
                //		topic += chat_text;
                //		topic[2] += *i;
                //	}
                //
                //}
                //else
                //{
                //	topic[2] += input.first();
                //}

                topic += gear.keystrokes;

                std::stringstream d;
                view v = gear.keystrokes;
                while (v.size())
                {
                    auto c = v.front();
                    d << (int)c << " ";
                    v.remove_prefix(1);
                }

                //log("key strokes: ", d.str());

                deface();
            };
        }

    };

    class button
        : public form
    {
        using self = button;
        FEATURE(pro::mouse, mouse);

    public:
        page topic; // button: Text content.

        cell hilight;
        text sticker;

        enum
        {
            pos_id,
            txt_id,
        };

        void recalc()
        {
            //topic[txt_id].set(hilight) = "  " + sticker;
            topic[txt_id] = "  " + sticker;
            topic[txt_id].link(bell::id);
            deface();
        }

        button(view label)
            : sticker{ label }
        {
            hilight = canvas.mark().alpha(0);
            //hilight = canvas.mark().vis(cell::highlighter).alpha(0);
            topic = ansi::idx(pos_id).idx(txt_id);

            recalc();

            using bttn = e2::hids::mouse::button;

            SUBMIT(e2::release, e2::form::state::mouse, active)
            {
                //hilight = canvas.mark().bgc(active ? 0xafffffff : 0);
                hilight = canvas.mark().bgc(active ? 0x4fffffff : 0);
                //hilight = canvas.mark().bga(active ? 0x5f : 0);
                recalc();
            };

            SUBMIT(e2::release, e2::form::layout::size, newsize)
            {
                auto center = twod{ newsize.x, newsize.y } / 2;
                auto length = static_cast<iota>(sticker.size());
                center.x -= 3 + length / 2;
                //center.x -= 3 + (iota)sticker.size() / 2;
                topic[pos_id].locus.kill().cup(center);
            };
        }

        virtual void redraw()
        {
            canvas.wipe();
            canvas.output(topic);
        }

    };

    class stem_elem : public form
    {
        using self = stem_elem;
        FEATURE(pro::mouse, mouse);

    public:
        page topic; // stem_elem: Text content.

        cell checked;
        cell hilight;
        text sticker;
        bool enabled;
        iota idx;

        enum
        {
            pos_id,
            box_id,
            txt_id,
        };

        void recalc()
        {
            if (enabled) checked.bgc(0x00ff00).bga(0xff);
            else         checked.bgc(0x404040).bga(0x6f);
            //if (enabled) checked.vis(cell::unalterable).bga(0xff);
            //else         checked.vis(cell::darklighter).bga(0x2f);

            topic[box_id].set(checked) = "  "; //"██";
            //topic[txt_id].set(hilight) = "  " + sticker;
            topic[txt_id] = "  " + sticker;
            topic[box_id].link(bell::id);
            topic[txt_id].link(bell::id);
            deface();
        }

        stem_elem(bool state, view label, iota index)
            : enabled{ state },
            sticker{ label },
            idx{ index }
        {
            checked.bgc(0x00ff00);
            //hilight = canvas.mark().vis(cell::highlighter).alpha(0);
            hilight = canvas.mark().alpha(0);
            topic = ansi::idx(pos_id).idx(box_id).idx(txt_id);

            recalc();

            //mouse.skipall(true);

            using bttn = e2::hids::mouse::button;
            SUBMIT(e2::release, e2::form::upon::attached, p)
            {
                //todo unify
                p->SUBMIT(e2::release, e2::data::changed, data)
                {
                    enabled = idx == data;
                    recalc();
                };
                p->SUBMIT(e2::preview, e2::data::changed, data)
                {
                    enabled = idx == data;
                    recalc();
                };

            };

            SUBMIT(e2::release, e2::form::state::mouse, active)
            {
                //hilight = canvas.mark().bgc(active ? 0xafffffff : 0);
                hilight = canvas.mark().bgc(active ? 0x4fffffff : 0);
                //hilight = canvas.mark().bga(active ? 0x5f : 0);
                recalc();
            };

            SUBMIT(e2::release, e2::form::layout::size, newsize)
            {
                auto center = newsize.y / 2;
                topic[pos_id].locus.kill().cup({ 2,center });
            };

            SUBMIT(e2::release, bttn::click::left, gear)
            {
                if (auto p = base::parent.lock())
                {
                    p->SIGNAL(e2::release, e2::data::changed, idx);
                }

                gear.dismiss();
            };
        }

        virtual void redraw()
        {
            canvas.wipe();
            canvas.output(topic);
        }

    };
    class stem_bsu : public form
    {
        using self = stem_bsu;
        FEATURE(pro::mouse, mouse);   // stem_bsu: Mouse controller.
        //FEATURE(pro::align, align); // stem_bsu: Align controller. Auto sized by parent.
        //FEATURE(pro::frame, frame); // stem_bsu: Window controller

    public:
        page topic; // stem_bsu: Text content.

        std::vector<sptr<base>> clients;
        iota bgclr = 0;

        stem_bsu()
        {
            using bttn = e2::hids::mouse::button;

            //todo unify: e2::preview +/or/and e2::release
            SUBMIT(e2::release, bttn::click::right, gear) //child event
            {
                color(canvas.mark().fgc(), (tint)((++bgclr) % 16));
                deface();

                gear.dismiss();
            };

            //todo unify
            SUBMIT(e2::general, e2::form::global::lucidity, alpha)
            {
                if (alpha >= 0 && alpha < 256)
                {
                    //log("alpha=", alpha);
                    canvas.mark().alpha(alpha);
                    base::deface();
                }
            };
        }

        virtual void redraw()
        {
            canvas.wipe();
            canvas.output(topic);

            auto basis = base::coor.get();
            for (auto& obj : clients)
            {
                canvas.render(obj, basis);
            }
        }

        // stem_bsu: Create a new item of the specified subtype and attach it.
        template<class T, class ...Args>
        auto attach(Args... args)
        {
            static_assert(std::is_base_of<base, T>::value,
                "The only a derivative of the «base» class can be attached to the «mold».");

            auto item = base::create<T>(args...);
            clients.push_back(item);
            auto root = This();
            item->SIGNAL(e2::release, e2::form::upon::attached, root);
            return item;
        }

        twod create_list(std::vector<text> const& objs_desc,
            twod start,
            iota step,
            iota height,
            iota on_item)
        {
            auto size = twod{ 0, height };
            ///  Get max width
            for (auto& item_text : objs_desc)
            {
                auto len = static_cast<iota>(item_text.size());
                size.x = std::max<iota>(size.x, len);
            }
            size.x += 8; // "  ██  " + 2
            auto i = 0;
            for (auto& item_text : objs_desc)
            {
                auto item = attach<stem_elem>(i == on_item ? true : faux, item_text, i);
                item->extend({ start, size });
                start.y += step;
                i++;
            }

            return { size.x, start.y };
        }
    };
    class stem_rate_grip : public form
    {
        using self = stem_rate_grip;
        FEATURE(pro::mouse, mouse); // stem_rate_grip: Mouse controller.

    public:
        page topic; // stem_rate_grip: Text content.

        bool enabled;
        text sfx_str;
        iota sfx_len;
        text pin_str;
        iota cur_val;
        twod box_len;

        //iota clients = 0;

        enum
        {
            txt_id,
            pin_id,
        };

        void set_pen(uint8_t hilight)
        {
            auto& pen = canvas.mark().bga(hilight);
            //topic[txt_id].set(pen);
            //topic[pin_id].set(pen);
        }
        void recalc()
        {
            text cur_str = std::to_string(cur_val);
            auto cur_len = utf::length(cur_str);
            auto pin_pos = std::max(cur_len, sfx_len) + 1;
            box_len.x = 1 + 2 * pin_pos;
            box_len.y = 4;

            topic[txt_id] = cur_str + " " + sfx_str;
            topic[pin_id] = pin_str;
            topic[txt_id].locus.kill().chx(pin_pos - cur_len).cud(1);
            topic[pin_id].locus.kill().chx(pin_pos);

            base::resize(box_len);
            deface();
        }
        auto set_val(iota new_val, view pin_chr)
        {
            cur_val = new_val;
            pin_str = pin_chr;
            recalc();
            return box_len;
        }

        stem_rate_grip(view sfx_string)
            : sfx_str{ sfx_string }
        {
            //mouse.skipall(true);
            mouse.highlightable = true;

            sfx_len = utf::length(sfx_str);

            canvas.mark().bgc(whitelt);
            topic = ansi::idx(txt_id).nop().eol()
                + ansi::idx(pin_id).nop();

            set_pen(0);

            SUBMIT(e2::preview, e2::form::layout::size, size)
            {
                size = box_len; //suppress resize
            };
            SUBMIT(e2::release, e2::form::state::mouse, active)
            {
                set_pen(active ? 80 : 0);
                recalc();
            };
        }

        virtual void redraw()
        {
            canvas.wipe();
            canvas.output(topic);
        }
    };

    template<e2::tier TIER, e2::type EVENT>
    class stem_rate : public form
    {
        using self = stem_rate;
        FEATURE(pro::robot, robot); // stem_rate: Animation controller.
        FEATURE(pro::mouse, mouse); // stem_rate: Mouse controller.
        //FEATURE(pro::align, align); // stem_rate: Align controller (Inform parent about size changes).

        using tail = netxs::datetime::tail<iota>;

    public:
        page	topic; // stem_rate: Text content.

        sptr<stem_rate_grip> grip_ctl;

        enum
        {
            //   cur_id + "sfx_str"
            //  ────█─────────bar_id──
            // min_id            max_id

            bar_id,
            min_id,
            max_id,
        };

        twod pin_len;
        iota pin_pos = 0;
        iota bar_len = 0;
        iota cur_val = 0;
        iota min_val = 0;
        iota max_val = 0;
        iota origin = 0;
        iota deltas = 0;

        //todo unify mint = 1/fps60 = 16ms
        //seems that the 4ms is enough, no need to bind with fps (opened question)
        tail bygone{ 75ms, 4ms };

        //twod minsize;
        text grip_suffix;
        text label_text;
        iota pad = 5;
        iota bgclr = 4;

        void recalc()
        {
            bar_len = std::max(0, base::size.get().x - (pad + 1) * 2);
            //auto pin_abs = (bar_len + 1) * (cur_val - min_val) / (max_val - min_val);
            auto pin_abs = netxs::divround((bar_len + 1) * (cur_val - min_val),
                (max_val - min_val));
            text pin_str;
            if (pin_abs == 0)           pin_str = "├";
            else if (pin_abs == bar_len + 1) pin_str = "┤";
            else                             pin_str = "┼";

            pin_len = grip_ctl->set_val(cur_val, pin_str);
            pin_pos = pad + pin_abs - pin_len.x / 2;// +1;
            topic[bar_id] = "└" + utf::repeat("─", bar_len) + "┘";
            topic[bar_id].locus.kill().chx(pad);
        }

        iota next_val(iota delta)
        {
            auto dm = max_val - min_val;
            auto p = divround((bar_len + 1) * (origin - min_val), dm);
            auto c = divround((p + delta) * dm, (bar_len + 1)) + min_val;
            bygone.set(c - cur_val);
            return c;
        }

        bool _move_grip(iota new_val)
        {
            new_val = std::clamp(new_val, min_val, max_val);
            if (new_val != cur_val)
            {
                cur_val = new_val;
                recalc();
                deface();

                return true;
            }
            return faux;
        }
        void move_grip(iota new_val)
        {
            if (_move_grip(new_val))
            {
                //SIGNAL(e2::general, e2::timer::fps, cur_val);
                SIGNAL(TIER, EVENT, cur_val);
            }
        }

        stem_rate(text const& caption, iota min_value, iota max_value, view suffix)
            : //cur_val{ cur_value },
            min_val{ min_value },
            max_val{ max_value },
            grip_suffix{ suffix }
        {
            cur_val = -1;
            //SIGNAL(e2::general, e2::timer::fps, cur_val);
            SIGNAL(TIER, EVENT, cur_val);

            //in order to make it transparent for mouse events
            //canvas.link(0);
            //
            //mouse.skipall(true);

            //todo use pin_size
            base::limits({ utf::length(caption) + (pad + 2) * 2,
                           10 });

            topic = ansi::wrp(faux).jet(bias::left)
                .cpy(50).chx(pad + 2).cuu(3) + caption + ansi::cud(3)
                + ansi::idx(bar_id).nop().eol()
                + ansi::idx(min_id).nop() + ansi::idx(max_id).nop();

            topic[min_id] = std::to_string(min_val);
            topic[max_id] = std::to_string(max_val);
            topic[max_id].locus.jet(bias::right).chx(pad);
            topic[min_id].locus.chx(pad);

            using bttn = e2::hids::mouse::button;

            //todo unify
            SUBMIT(e2::general, e2::form::global::lucidity, alpha)
            {
                if (alpha >= 0 && alpha < 256)
                {
                    //log("alpha=", alpha);
                    canvas.mark().alpha(alpha);
                    base::deface();
                }
            };


            //SUBMIT(e2::general, e2::timer::fps, cur_val)
            SUBMIT(TIER, EVENT, cur_val)
            {
                if (cur_val >= min_val)
                {
                    _move_grip(cur_val);
                }
            };

            SUBMIT(e2::release, e2::form::upon::attached, parent)
            {
                grip_ctl = create<stem_rate_grip>(grip_suffix);
                auto root = This();
                grip_ctl->SIGNAL(e2::release, e2::form::upon::attached, root);
                //grip->SIGNAL(e2::release, e2::form::upon::attached, This());

                grip_ctl->SUBMIT(e2::release, bttn::drag::start::left, gear)
                    //grip->SUBMIT(e2::preview, bttn::drag::start::left, gear)
                {
                    if (gear.capture(grip_ctl->id))
                    {
                        origin = cur_val;
                        //grip->bell::expire(e2::release);
                        //grip->bell::expire(e2::preview);
                        gear.dismiss();
                    }
                };
                grip_ctl->SUBMIT(e2::release, bttn::drag::pull::left, gear)
                {
                    if (gear.captured(grip_ctl->id))
                    {
                        deltas += gear.mouse::delta.get().x;
                        move_grip(next_val(deltas));
                        //bell::expire(e2::release);
                        gear.dismiss();
                    }
                };
                grip_ctl->SUBMIT(e2::release, bttn::drag::cancel::left, gear)
                {
                    if (gear.captured(grip_ctl->id))
                    {
                        deltas = 0;
                        move_grip(origin);
                        gear.release();
                        //bell::expire(e2::release);
                        gear.dismiss();
                    }
                };
                grip_ctl->SUBMIT(e2::release, bttn::drag::stop::left, gear)
                {
                    if (gear.captured(grip_ctl->id))
                    {
                        deltas = 0;
                        gear.release();
                        deface();
                        robot.actify(bygone.fader<quadratic<iota>>(750ms), [&](auto& delta)
                            {
                                move_grip(cur_val + delta);
                            });
                        //bell::expire(e2::release);
                        gear.dismiss();
                    }
                };
                grip_ctl->SUBMIT(e2::release, e2::hids::mouse::scroll::up, gear)
                {
                    move_grip(cur_val - 1);
                    //bell::expire(e2::preview);
                    gear.dismiss();
                };
                grip_ctl->SUBMIT(e2::release, e2::hids::mouse::scroll::down, gear)
                {
                    move_grip(cur_val + 1);
                    //bell::expire(e2::preview);
                    gear.dismiss();
                };

                SUBMIT(e2::release, e2::form::layout::size, size)
                {
                    recalc();
                };

                recalc();
            };

            //SUBMIT(e2::preview, e2::form::layout::size, size)
            //{
            //	size = std::max(size, minsize);
            //};

            SUBMIT(e2::release, bttn::click::right, gear)
            {
                color(canvas.mark().fgc(), (tint)((++bgclr) % 16));
                deface();

                gear.dismiss();
                //bell::expire(e2::release);
            };

            SUBMIT(e2::release, e2::hids::mouse::scroll::up, gear)
            {
                move_grip(cur_val - 10);
                //bell::expire(e2::release);
                gear.dismiss();
            };
            SUBMIT(e2::release, e2::hids::mouse::scroll::down, gear)
            {
                move_grip(cur_val + 10);
                //bell::expire(e2::release);
                gear.dismiss();
            };
        }

        virtual void redraw()
        {
            canvas.wipe();
            canvas.output(topic);
            auto cp = canvas.cp();
            cp.x = pin_pos;
            cp.y -= 3;
            grip_ctl->base::moveto(cp);
            canvas.render(grip_ctl, base::coor.get());
            SIGNAL(e2::release, e2::form::upon::redrawn, canvas);
        }
    };

    struct ticker
        : public form
    {
        twod	offset; // ticker: Ticker position.
        para	matter; // ticker: Ticker text line.

        //todo unspecial!
        //pro::print cursor; // ticker: Cursor controller.
        //pro::align<ticker>

        virtual void render(face& parent_canvas)
        {
            draw();
            parent_canvas.plot(canvas);
        }
        void draw()
        {
            offset.x--;
            paint();
        }
        void paint()
        {
            if (matter.size())
            {
                twod curpos = offset;

                //purify();
                canvas.wipe();
                //canvas.rst();
                canvas.jet(bias::left).wrp(false);

                auto& size = base::size.get();
                while (curpos.x < size.x)
                {
                    canvas.cup(curpos);
                    canvas.output(matter);

                    curpos.x += static_cast<netxs::iota>(matter.size());
                    if (curpos.x <= 0)
                    {
                        offset.x = curpos.x;
                    }
                }
                ///auto rend = utf::adjust(utf::format(counters.render.count()), 11, " ", true);
                ///auto outp = utf::adjust(utf::format(counters.output.count()), 11, " ", true);
                ///auto outs = utf::adjust(utf::format(counters.frsize), 9, " ", true);
                ///
                /////statline.edit(" process:" + rend + "ns, render:" + outp + "ns ");
                ///statline = " process:" + rend
                ///		 + "ns, render:" + outp
                ///		 + "ns, frame size:" + outs + " bytes ";
                ///statline.locus.jet(bias::center).wrp(false).cpp({ 50,0 });
                ///
                ///guide.output(statline);

                //twod g = { -1,1 };
                //iota a = 13;
                //iota b = -54;
                //for (int i = 0; i < 10000000; i++)
                //	//iota c = a ^ b;
                //	//layer.rotate(g);
                //	layer.trunc(g);
            }
        }
        //void tick()
        //{
        //	position.x--;
        //}

        twod mysize;

        ticker()
            //: cursor{*this }
        {
            canvas.mark(cell{ whitespace }.bgc(bluedk).fgc(cyanlt));

            SUBMIT(e2::release, e2::form::upon::redrawn, canvas)
            {
                status.invalid = true;
                status.wrecked = true;
            };

            SUBMIT(e2::release, e2::form::upon::attached, parent)
            {
                //todo via align behavior
                parent->SUBMIT(e2::release, e2::form::layout::size, basesize)
                {
                    extend({ { 0, basesize.y - 1 }, { basesize.x, 1} });
                    //extend({ { 0, 0 }, { basesize.x, 1} });
                    //paint();
                };
            };
        }
    };
}

// deprecated but can be useful later
//struct datablock : public block
//{
//	twod v;
//	utils::random rnd;
//	std::shared_ptr<oscillator> data_ptr;
//	int mx = 2;
//	int my = 1;
//
//	void setposition()
//	{
//		if (auto base = parent.lock())
//		{
//			move({ rnd(0, std::max(0,base->layer.size.x - 1 - mx)),
//				 rnd(0, std::max(0,base->layer.size.y - 1 - my)) });
//		}
//	}
//	void start()
//	{
//		if (auto base = parent.lock())
//		{
//			trace(base->relay.release, e2::form::layout::size, [&]
//					  {
//						  setposition();
//					  });
//		}
//		setposition();
//		resize( { mx, my } );
//		showup();
//
//		trace(data_ptr->relay, e2::data::changed, [&]
//				  {
//					  if (auto base = parent.lock())
//					  {
//						  move(layer.coor + v);
//						  if (layer.coor.x + layer.size.x >= base->layer.size.x || layer.coor.x <= 0)
//						  {
//							  v.x = -v.x;
//						  }
//						  if (layer.coor.y + layer.size.y >= base->layer.size.y || layer.coor.y <= 0)
//						  {
//							  v.y = -v.y;
//						  }
//					  }
//				  });
//	}
//
//	datablock(std::shared_ptr<oscillator> data_ptr)
//		: data_ptr(data_ptr)
//	{
//		v = { rnd(-5, 5), rnd(-5, 5) };
//
//		marker.priming({ tint::greenlt, tint::green });
//
//		trace(relay.release, e2::form::attached, [&]
//				  {
//					  start();
//				  });
//	}
//};
//
//struct hive
//{
//	utils::random rnd;
//	rect          area{ dot_00, {1000,1000} };
//	twod          realm_size;
//
//	struct bug
//	{
//		twod v;
//		rect pos;
//
//		bug& move()
//		{
//			pos.coor += v;
//			return *this;
//		}
//	};
//	std::vector<bug> swarm;
//
//	hive(utils::iota count)
//	{
//		swarm.resize(count);
//		for (auto& bug : swarm)
//		{
//			int vxmin = area.size.x / 120;
//			int vxmax = area.size.x / 40;
//			int vymin = area.size.y / 120;
//			int vymax = area.size.y / 40;
//			bug.v = { rnd(vxmin, vxmax), rnd(vymin, vymax) };
//			bug.pos.size = {rnd(1, 4), rnd(1, 2) };
//			bug.pos.coor = { rnd(0, area.size.x - 1 - bug.pos.size.x), rnd(0, area.size.y - 1 - bug.pos.size.y) };
//		}
//	}
//	void checkbound(bug& b)
//	{
//		if (!area.hittest(b.pos.coor))
//		{
//			auto visible = area.clip(b.pos);
//			if (visible.size.x != b.pos.size.x) b.v.x = -b.v.x;
//			if (visible.size.y != b.pos.size.y) b.v.y = -b.v.y;
//		}
//	}
//	void tick()
//	{
//		for (auto& bug : swarm)
//		{
//			checkbound(bug.move());
//		}
//	}
//};
//
//struct datasrc : public bell
//{
//	hive	balls;
//	reactor	debug{ reactor::forward };
//	period	pause;
//
//	datetime::quartz<reactor, e2::type>	clock;
//	datetime::moment	present;
//
//	datasrc(utils::iota count, datetime::period speed, period delay = period::zero())
//		:	balls(count),
//			clock(router(e2::release), e2::timer::tick),
//			pause(delay)
//	{
//		SUBMIT(e2::release, e2::timer::tick, now)
//		{
//			balls.tick();
//			utils::iota dd = 0;
//			//signal(e2::release, e2::data::changed, dd);
//			SIGNAL(e2::release, e2::data::changed, dd);
//
//			if (pause != period::zero())
//			{
//				if (now - present > pause)
//				{
//					debug(e2::data::changed, dd);
//
//					clock.freeze(pause / 2);
//					present = now;
//				}
//			}
//		});
//
//		present = datetime::tempus::now();
//		clock.ignite(speed);
//	}
//	void stop()
//	{
//		clock.cancel();
//	}
//	~datasrc()
//	{
//		stop();
//	}
//};
//using data_ptr = std::shared_ptr<datasrc>;
//
//struct chaos : public pane
//{
//	data_ptr	data;
//	cell		base;
//
//	chaos(data_ptr datasrc)
//		:	data(datasrc)
//	{
//		base.txt("█").bgc(0x00u).fgc(0x00u);
//
//		data->SUBMIT_T(e2::release, e2::data::changed, logic, p)
//		{
//			deface();
//		});
//	}
//
//	virtual void render(face& parent_canvas)
//	{
//		draw();
//		parent_canvas.plot(canvas);
//	}
//
//	void draw()
//	{
//		pane::redraw();
//
//		auto& size = base::size.get();
//		for (auto& bug : data->balls.swarm)
//		{
//			auto bug_scaled = rect{ bug.pos.coor * size / data->balls.area.size, bug.pos.size };
//
//			///bug_scaled = bug_scaled.clip({ dot_00, canvas.size });
//
//
//			canvas.fill(bug_scaled, base);
//		}
//	}
//};

#endif // NETXS_CONTROLS_HPP