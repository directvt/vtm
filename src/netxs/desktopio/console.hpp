// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#include "input.hpp"
#include "system.hpp"

namespace netxs::ui
{
    // console: Base class behavior extensions.
    namespace pro
    {
        // pro: Base class for plugins.
        struct skill
        {
            base& boss;
            subs  memo;

            skill(base&&) = delete;
            skill(base& boss) : boss{ boss } { }
            virtual ~skill() = default; // In order to allow man derived class via base ptr.

            template<class T>
            struct socks
            {
                struct sock : public T
                {
                    id_t    id; // sock: Hids ID.
                    si32 count; // sock: Clients count.

                    sock(id_t ctrl)
                        :    id{ ctrl },
                          count{ 0    }
                    { }

                    operator bool () { return T::operator bool(); }
                };

                std::vector<sock> items; // sock: Registered hids.
                subs              token; // sock: Hids subscriptions.

                socks(base& boss)
                {
                    boss.LISTEN(tier::general, hids::events::die, gear, token)
                    {
                        del(gear);
                    };
                    boss.LISTEN(tier::release, hids::events::notify::mouse::enter, gear, token)
                    {
                        add(gear);
                    };
                    boss.LISTEN(tier::release, hids::events::notify::mouse::leave, gear, token)
                    {
                        dec(gear);
                    };
                }
                template<bool ConstWarn = true>
                auto& take(hids& gear)
                {
                    for (auto& item : items) // Linear search, because a few items.
                    {
                        if (item.id == gear.id) return item;
                    }

                    if constexpr (ConstWarn)
                    {
                        log("sock: error: access to unregistered input device, ", gear.id);
                    }

                    return items.emplace_back(gear.id);
                }
                template<class P>
                void foreach(P proc)
                {
                    for (auto& item : items)
                    {
                        if (item) proc(item);
                    }
                }
                void add(hids& gear)
                {
                    auto& item = take<faux>(gear);
                    ++item.count;
                }
                void dec(hids& gear)
                {
                    auto& item = take(gear);
                    if (--item.count < 1) // item.count could be equal to 0 due to unregistered access.
                    {
                        if (items.size() > 1) item = items.back(); // Remove an item without allocations.
                        items.pop_back();
                    }
                }
                void del(hids& gear)
                {
                    for (auto& item : items) // Linear search, because a few items.
                    {
                        if (item.id == gear.id)
                        {
                            if (items.size() > 1) item = items.back(); // Remove an item without allocations.
                            items.pop_back();
                            return;
                        }
                    }
                }
            };
        };

        // pro: Provides resizing by dragging.
        class sizer
            : public skill
        {
            struct sock
            {
                using test = testy<twod>;

                twod origin; // sock: Grab's initial coord info.
                twod dtcoor; // sock: The form coor parameter change factor while resizing.
                twod sector; // sock: Active quadrant, x,y = {-1|+1}. Border widths.
                rect hzgrip; // sock: Horizontal grip.
                rect vtgrip; // sock: Vertical grip.
                twod widths; // sock: Grip's widths.
                bool inside; // sock: Is active.
                bool seized; // sock: Is seized.
                test lastxy; // sock: Change tracker.
                rect zoomsz; // sock: Captured area for zooming.
                dent zoomdt; // sock: Zoom step.
                bool zoomon; // sock: Zoom in progress.
                twod zoomat; // sock: Zoom pivot.

                sock()
                    : inside{ faux },
                      seized{ faux },
                      zoomon{ faux }
                { }

                operator bool () { return inside || seized; }
                auto corner(twod const& length)
                {
                    return dtcoor.less(dot_11, length, dot_00);
                }
                auto grab(base const& master, twod curpos, dent const& outer)
                {
                    if (inside)
                    {
                        origin = curpos - corner(master.base::size() + outer);
                        seized = true;
                    }
                    return seized;
                }
                auto calc(base const& master, twod curpos, dent const& outer, dent const& inner, dent const& border)
                {
                    auto area = rect{ dot_00, master.base::size() };
                    auto inner_rect = area + inner;
                    auto outer_rect = area + outer;
                    inside = !inner_rect.hittest(curpos)
                           && outer_rect.hittest(curpos);
                    auto& length = outer_rect.size;
                    curpos += outer.corner();
                    auto center = std::max(length / 2, dot_11);
                    if (!seized)
                    {
                        dtcoor = curpos.less(center + (length & 1), dot_11, dot_00);
                        sector = dtcoor.less(dot_11, -dot_11, dot_11);
                        widths = sector.less(dot_00, twod{-border.east.step,-border.foot.step },
                                                     twod{ border.west.step, border.head.step });
                    }
                    auto l = sector * (curpos - corner(length));
                    auto a = center * l / center;
                    auto b = center *~l /~center;
                    auto s = sector * std::max(a - b + center, dot_00);

                    hzgrip.coor.x = widths.x;
                    hzgrip.coor.y = 0;
                    hzgrip.size.y = widths.y;
                    hzgrip.size.x = s.x;

                    vtgrip.coor = dot_00;
                    vtgrip.size = widths;
                    vtgrip.size.y += s.y;
                    return lastxy(curpos);
                }
                auto drag(base& master, twod const& curpos, dent const& outer, bool zoom)
                {
                    if (seized)
                    {
                        auto width = master.base::size() + outer;
                        auto delta = (corner(width) + origin - curpos) * sector;
                        if (auto dxdy = master.base::sizeby(zoom ? delta * 2 : delta))
                        {
                            auto step = zoom ? -dxdy / 2 : -dxdy * dtcoor;
                            master.base::moveby(step);
                            master.SIGNAL(tier::preview, e2::form::upon::changed, dxdy);
                        }
                    }
                    return seized;
                }
                void drop()
                {
                    seized = faux;
                }
            };

            using list = socks<sock>;
            using skill::boss,
                  skill::memo;

            list items;
            dent outer;
            dent inner;
            dent width;
            bool alive; // pro::sizer: The sizer state.

        public:
            void props(dent const& outer_rect = {2,2,1,1}, dent const& inner_rect = {})
            {
                outer = outer_rect;
                inner = inner_rect;
                width = outer - inner;
            }
            auto get_props()
            {
                return std::pair{ outer, inner };
            }

            sizer(base&&) = delete;
            sizer(base& boss, dent const& outer_rect = {2,2,1,1}, dent const& inner_rect = {})
                : skill{ boss          },
                  items{ boss          },
                  outer{ outer_rect    },
                  inner{ inner_rect    },
                  width{ outer - inner },
                  alive{ true          }
            {
                boss.LISTEN(tier::release, hids::events::mouse::scroll::any, gear, memo)
                {
                    if (gear.meta(hids::anyCtrl))
                    {
                        auto& g = items.take(gear);
                        if (!g.zoomon)// && g.inside)
                        {
                            g.zoomdt = {};
                            g.zoomon = true;
                            g.zoomsz = boss.base::area();
                            g.zoomat = gear.coord;
                            gear.capture(boss.id);
                        }
                        static constexpr auto warp = dent{ 2,2,1,1 } * 2;
                        //todo respect pivot
                        auto prev = g.zoomdt;
                        auto coor = boss.coor();
                        auto deed = boss.bell::protos<tier::release>();
                        if (deed == hids::events::mouse::scroll::down.id) g.zoomdt -= warp;
                        else                                              g.zoomdt += warp;
                        gear.owner.SIGNAL(tier::request, e2::form::prop::viewport, viewport, ());
                        auto next = (g.zoomsz + g.zoomdt).clip(viewport);
                        auto step = boss.extend(next);
                        if (!step.size) // Undo if can't zoom.
                        {
                            g.zoomdt = prev;
                            boss.moveto(coor);
                        }
                    }
                };
                boss.LISTEN(tier::release, e2::config::plugins::sizer::alive, state, memo)
                {
                    alive = state;
                };
                boss.LISTEN(tier::release, e2::postrender, canvas, memo)
                {
                    if (!alive) return;
                    auto area = canvas.full() + outer;
                    auto fuse = [&](cell& c){ c.xlight(); };
                    canvas.cage(area, width, [&](cell& c){ c.link(boss.id); });
                    items.foreach([&](sock& item)
                    {
                        auto corner = item.corner(area.size);
                        auto side_x = item.hzgrip.shift(corner).normalize_itself()
                                                 .shift_itself(area.coor).clip(area);
                        auto side_y = item.vtgrip.shift(corner).normalize_itself()
                                                 .shift_itself(area.coor).clip(area);
                        canvas.fill(side_x, fuse);
                        canvas.fill(side_y, fuse);
                    });
                };
                boss.LISTEN(tier::release, e2::form::layout::swarp, warp, memo)
                {
                    auto area = boss.base::area();
                    auto next = area + warp;
                    auto step = boss.extend(next);
                };
                boss.LISTEN(tier::release, e2::config::plugins::sizer::outer, outer_rect, memo)
                {
                    outer = outer_rect;
                    width = outer - inner;
                };
                boss.LISTEN(tier::release, e2::config::plugins::sizer::inner, inner_rect, memo)
                {
                    inner = inner_rect;
                    width = outer - inner;
                };
                boss.LISTEN(tier::request, e2::config::plugins::sizer::inner, inner_rect, memo)
                {
                    inner_rect = inner;
                };
                boss.LISTEN(tier::request, e2::config::plugins::sizer::outer, outer_rect, memo)
                {
                    outer_rect = outer;
                };
                boss.LISTEN(tier::release, hids::events::mouse::move, gear, memo)
                {
                    auto& g = items.take(gear);
                    if (g.zoomon && !gear.meta(hids::anyCtrl))
                    {
                        g.zoomon = faux;
                        gear.setfree();
                    }
                    if (g.calc(boss, gear.coord, outer, inner, width))
                    {
                        boss.base::deface(); // Deface only if mouse moved.
                    }
                };
                engage<hids::buttons::left>();
                engage<hids::buttons::leftright>();
            }
            // pro::sizer: Configuring the mouse button to operate.
            template<hids::buttons Button>
            void engage()
            {
                boss.SIGNAL(tier::release, e2::form::draggable::_<Button>, true);
                boss.LISTEN(tier::release, e2::form::drag::start::_<Button>, gear, memo)
                {
                    if (items.take(gear).grab(boss, gear.coord, outer))
                    {
                        gear.dismiss();
                        boss.bell::expire<tier::release>(); // To prevent d_n_d triggering.
                    }
                };
                boss.LISTEN(tier::release, e2::form::drag::pull::_<Button>, gear, memo)
                {
                    if (items.take(gear).drag(boss, gear.coord, outer, gear.meta(hids::anyCtrl)))
                    {
                        gear.dismiss();
                    }
                };
                boss.LISTEN(tier::release, e2::form::drag::cancel::_<Button>, gear, memo)
                {
                    items.take(gear).drop();
                };
                boss.LISTEN(tier::release, e2::form::drag::stop::_<Button>, gear, memo)
                {
                    items.take(gear).drop();
                    boss.SIGNAL(tier::release, e2::form::upon::dragged, gear);
                };
            }
        };

        // pro: Provides moving by dragging.
        class mover
            : public skill
        {
            struct sock
            {
                twod origin; // sock: Grab's initial coord info.
                void grab(base const& master, twod const& curpos)
                {
                    auto center = master.base::size() / 2;
                    origin = curpos - center;
                }
                void drag(base& master, twod const& coord)
                {
                    auto delta = coord - origin;
                    auto center = master.base::size() / 2;
                    delta -= center;
                    master.base::moveby(delta);
                }
            };

            using list = socks<sock>;
            using skill::boss,
                  skill::memo;

            list       items;
            wptr<base> dest_shadow;
            sptr<base> dest_object;

        public:
            mover(base&&) = delete;
            mover(base& boss, sptr<base> subject)
                : skill{ boss },
                  items{ boss },
                  dest_shadow{ subject }
            {
                engage<hids::buttons::left>();
            }
            mover(base& boss)
                : mover{ boss, boss.This() }
            { }
            // pro::mover: Configuring the mouse button to operate.
            template<hids::buttons Button>
            void engage()
            {
                boss.SIGNAL(tier::release, e2::form::draggable::_<Button>, true);
                boss.LISTEN(tier::release, e2::form::drag::start::_<Button>, gear, memo)
                {
                    if ((dest_object = dest_shadow.lock()))
                    {
                        items.take(gear).grab(*dest_object, gear.coord);
                        gear.dismiss();
                    }
                };
                boss.LISTEN(tier::release, e2::form::drag::pull::_<Button>, gear, memo)
                {
                    if (dest_object)
                    {
                        items.take(gear).drag(*dest_object, gear.coord);
                        auto delta = gear.delta.get();
                        dest_object->SIGNAL(tier::preview, e2::form::upon::changed, delta);
                        gear.dismiss();
                    }
                };
                boss.LISTEN(tier::release, e2::form::drag::cancel::_<Button>, gear, memo)
                {
                    if (dest_object)
                    {
                        dest_object.reset();
                        gear.dismiss();
                    }
                };
                boss.LISTEN(tier::release, e2::form::drag::stop::_<Button>, gear, memo)
                {
                    if (dest_object)
                    {
                        dest_object->SIGNAL(tier::release, e2::form::upon::dragged, gear);
                        dest_object.reset();
                        gear.dismiss();
                    }
                };
            }
        };

        // pro: Keybd/Mouse highlighter.
        class track
            : public skill
        {
            struct sock
            {
                twod cursor{}; // sock: Coordinates of the active cursor.
                bool inside{}; // sock: Is active.

                operator bool () { return inside; }
                auto calc(base const& master, twod curpos)
                {
                    auto area = rect{ dot_00, master.base::size() };
                    cursor = curpos;
                    inside = area.hittest(curpos);
                }
            };

            using pool = std::list<id_t>;
            using list = socks<sock>;
            using skill::boss,
                  skill::memo;

            pool focus; // track: Is keybd focused.
            list items; // track: .
            bool alive; // track: Is active.

            void add_keybd(id_t gear_id)
            {
                if (gear_id != id_t{})
                {
                    auto stat = focus.empty();
                    auto iter = std::find(focus.begin(), focus.end(), gear_id);
                    if (iter == focus.end())
                    {
                        focus.push_back(gear_id);
                        if (stat) boss.deface();
                    }
                }
            }
            void del_keybd(id_t gear_id)
            {
                if (gear_id != id_t{})
                {
                    auto stat = focus.size();
                    auto iter = std::find(focus.begin(), focus.end(), gear_id);
                    if (iter != focus.end())
                    {
                        focus.erase(iter);
                        if (stat) boss.deface();
                    }
                }
            }

        public:
            track(base&&) = delete;
            track(base& boss, bool keybd_only = faux)
                : skill{ boss },
                  items{ boss },
                  alive{ true }
            {
                // Keybd focus.
                boss.LISTEN(tier::release, hids::events::keybd::focus::bus::on, seed, memo)
                {
                    add_keybd(seed.id);
                };
                boss.LISTEN(tier::release, hids::events::keybd::focus::bus::off, seed, memo)
                {
                    del_keybd(seed.id);
                };
                boss.LISTEN(tier::release, hids::events::die, gear, memo) // Gen by pro::focus.
                {
                    del_keybd(gear.id);
                };
                boss.LISTEN(tier::release, e2::render::prerender, parent_canvas, memo)
                {
                    if (focus.empty() || !alive) return;
                    static constexpr auto title_fg_color = rgba{ 0xFFffffff };
                    //todo revise, too many fillings (mold's artifacts)
                    auto normal = boss.base::color();
                    auto bright = skin::color(tone::brighter);
                    auto shadow = skin::color(tone::shadower);
                    //todo unify, make it more contrast
                    shadow.alpha(0x80);
                    bright.fgc(title_fg_color);
                    shadow.fgc(title_fg_color);
                    auto fillup = [&](auto bright, auto shadow)
                    {
                        parent_canvas.fill(shadow);
                    };
                    if (normal.bgc().alpha())
                    {
                        auto fuse_bright = [&](cell& c) { c.fuse(normal); c.fuse(bright); };
                        auto fuse_shadow = [&](cell& c) { c.fuse(normal); c.fuse(shadow); };
                        fillup(fuse_shadow, fuse_bright);
                    }
                    else
                    {
                        auto only_bright = [&](cell& c) { c.fuse(bright); };
                        auto only_shadow = [&](cell& c) { c.fuse(shadow); };
                        fillup(only_shadow, only_bright);
                    }
                    // Draw the border around
                    auto area = parent_canvas.full();
                    auto mark = skin::color(tone::kb_focus);
                    mark.fgc(title_fg_color); //todo unify, make it more contrast
                    auto fill = [&](cell& c) { c.fuse(mark); };
                    parent_canvas.cage(area, dot_21, fill);
                };
                boss.LISTEN(tier::anycast, e2::form::prop::lucidity, lucidity, memo)
                {
                    if (lucidity != -1) alive = lucidity == 0xFF;
                };
                if (keybd_only || !skin::globals().tracking) return;
                // Mouse focus.
                boss.LISTEN(tier::release, hids::events::mouse::move, gear, memo)
                {
                    items.take(gear).calc(boss, gear.coord);
                };
                boss.LISTEN(tier::release, e2::render::prerender, parent_canvas, memo)
                {
                    if (!alive) return;
                    auto full = parent_canvas.full();
                    auto view = parent_canvas.view();
                    auto mark = cell{}.bgc(0xFFffffff);
                    auto fill = [&](cell& c) { c.fuse(mark); };
                    items.foreach([&](sock& item)
                    {
                        auto area = rect{ item.cursor, dot_00 } + dent{ 6,6,3,3 };
                        area.coor += full.coor;
                        parent_canvas.fill(area.clip(full), fill);
                    });
                };
            }
        };

        // pro: Provides functionality for runtime animation (time-based).
        class robot
            : public skill
        {
            using subs = std::map<id_t, hook>;
            using skill::boss;

            subs memo;

        public:
            using skill::skill; // Inherits ctors.

            // pro::robot: Every timer tick, yield the
            //             delta from the flow and, if delta,
            //             Call the proc (millisecond precision).
            template<class P, class S>
            void actify(id_t ID, S flow, P proc)
            {
                auto init = datetime::now();
                boss.LISTEN(tier::general, e2::timer::any, p, memo[ID], (ID, proc, flow, init))
                {
                    auto now = datetime::round<si32>(p - init);
                    if (auto data = flow(now))
                    {
                        static constexpr auto zero = std::decay_t<decltype(data.value())>{};
                        auto& v = data.value();
                        if (v != zero) proc(v);
                    }
                    else
                    {
                        pacify(ID);
                    }
                };
                boss.SIGNAL(tier::release, e2::form::animate::start, ID);
            }
            // pro::robot: Optional proceed every timer tick,
            //             yield the delta from the flow and,
            //             if delta, Call the proc (millisecond precision).
            template<class P, class S>
            void actify(id_t ID, std::optional<S> flow, P proc)
            {
                if (flow)
                {
                    actify(ID, flow.value(), proc);
                }
            }
            template<class P, class S>
            void actify(S flow, P proc)
            {
                actify(bell::noid, flow, proc);
            }
            template<class P, class S>
            void actify(std::optional<S> flow, P proc)
            {
                if (flow)
                {
                    actify(bell::noid, flow.value(), proc);
                }
            }
            // pro::robot: Cancel tick activity.
            void pacify(id_t id = bell::noid)
            {
                if (id == bell::noid) memo.clear(); // Stop all animations.
                else                  memo.erase(id);
                boss.SIGNAL(tier::release, e2::form::animate::stop, id);
            }
            // pro::robot: Check activity by id.
            bool active(id_t id)
            {
                return memo.contains(id);
            }
            // pro::robot: Check any activity.
            operator bool ()
            {
                return !memo.empty();
            }
        };

        // pro: Invokes specified proc after timeout.
        class timer
            : public skill
        {
            using subs = std::map<id_t, hook>;
            using skill::boss;

            subs memo;

        public:
            using skill::skill; // Inherits ctors.

            // pro::timer: Start countdown for specified ID.
            template<class P>
            void actify(id_t ID, span timeout, P lambda)
            {
                auto alarm = datetime::now() + timeout;
                boss.LISTEN(tier::general, e2::timer::any, now, memo[ID], (ID, timeout, lambda, alarm))
                {
                    if (now > alarm)
                    {
                        alarm = now + timeout;
                        if (!lambda(ID)) pacify(ID);
                    }
                };
            }
            // pro::timer: Start countdown.
            template<class P>
            void actify(span timeout, P lambda)
            {
                actify(bell::noid, timeout, lambda);
            }
            // pro::timer: Cancel timer ('id=noid' for all).
            void pacify(id_t id = bell::noid)
            {
                if (id == bell::noid) memo.clear(); // Stop all timers.
                else                  memo.erase(id);
                //boss.SIGNAL(tier::release, e2::form::animate::stop, id);
            }
            // pro::timer: Check activity by id.
            bool active(id_t id)
            {
                return memo.contains(id);
            }
            // pro::timer: Check any activity.
            operator bool ()
            {
                return !memo.empty();
            }
        };

        // pro: The text caret controller.
        class caret
            : public skill
        {
            using skill::boss,
                  skill::memo;

            subs conf; // caret: Configuration subscriptions.
            bool live; // caret: Should the caret be drawn.
            bool done; // caret: Is the caret already drawn.
            bool down; // caret: Is the caret suppressed (lost focus).
            bool form; // caret: Caret style.
            rect body; // caret: Caret position.
            span step; // caret: Blink interval. span::zero() if steady.
            time next; // caret: Time of next blinking.

        public:
            caret(base&&) = delete;
            caret(base& boss, bool visible = faux, bool abox = faux, twod position = dot_00, span freq = skin::globals().blink_period)
                : skill{ boss },
                   live{ faux },
                   done{ faux },
                   down{ true },
                   form{ abox },
                   body{ position, dot_11 }, // Caret is always one cell size (see the term::scrollback definition).
                   step{ freq }
            {
                boss.LISTEN(tier::release, e2::form::state::keybd::focus::state, state, conf)
                {
                    down = !state;
                };
                boss.LISTEN(tier::request, e2::config::caret::blink, req_step, conf)
                {
                    req_step = step;
                };
                boss.LISTEN(tier::request, e2::config::caret::style, req_style, conf)
                {
                    req_style = form ? 1 : 0;
                };
                boss.LISTEN(tier::general, e2::config::caret::blink, new_step, conf)
                {
                    blink_period(new_step);
                };
                boss.LISTEN(tier::preview, e2::config::caret::blink, new_step, conf)
                {
                    blink_period(new_step);
                };
                boss.LISTEN(tier::general, e2::config::caret::style, new_style, conf)
                {
                    style(new_style);
                };
                boss.LISTEN(tier::preview, e2::config::caret::style, new_style, conf)
                {
                    style(new_style);
                };
                if (visible) show();
            }

            operator bool () const { return memo.count(); }

            // pro::caret: Set caret style.
            void style(bool new_form)
            {
                if (form != new_form)
                {
                    hide();
                    form = new_form;
                    show();
                }
            }
            // pro::caret: Set blink period.
            void blink_period(span const& new_step = skin::globals().blink_period)
            {
                auto changed = (step == span::zero()) != (new_step == span::zero());
                step = new_step;
                if (changed)
                {
                    hide();
                    show();
                }
            }
            void style(si32 mode)
            {
                switch (mode)
                {
                    case 0: // n = 0  blinking box
                    case 1: // n = 1  blinking box (default)
                        blink_period();
                        style(true);
                        break;
                    case 2: // n = 2  steady box
                        blink_period(span::zero());
                        style(true);
                        break;
                    case 3: // n = 3  blinking underline
                        blink_period();
                        style(faux);
                        break;
                    case 4: // n = 4  steady underline
                        blink_period(span::zero());
                        style(faux);
                        break;
                    case 5: // n = 5  blinking I-bar
                        blink_period();
                        style(true);
                        break;
                    case 6: // n = 6  steady I-bar
                        blink_period(span::zero());
                        style(true);
                        break;
                    default:
                        log("pro::caret: unsupported cursor style requested, ", mode);
                        break;
                }
            }
            void toggle()
            {
                style(!form);
                reset();
            }
            // pro::caret: Set caret position.
            void coor(twod const& coor)
            {
                if (body.coor != coor)
                {
                    reset();
                    body.coor = coor;
                }
            }
            // pro::caret: Get caret position.
            auto& coor() const
            {
                return body.coor;
            }
            // pro::caret: Get caret style.
            auto style() const
            {
                return std::pair{ form, !!(*this) };
            }
            // pro::caret: Force to redraw caret.
            void reset()
            {
                if (step != span::zero())
                {
                    live = faux;
                    next = {};
                }
            }
            // pro::caret: Enable caret.
            void show()
            {
                if (!*this)
                {
                    done = faux;
                    live = step == span::zero();
                    if (!live)
                    {
                        boss.LISTEN(tier::general, e2::timer::tick, timestamp, memo)
                        {
                            if (timestamp > next)
                            {
                                next = timestamp + step;
                                live = !live;
                                boss.deface(body);
                            }
                        };
                    }
                    boss.LISTEN(tier::release, e2::postrender, canvas, memo)
                    {
                        done = live;
                        auto state = down ? (step == span::zero() ? faux : true)
                                          : live;
                        if (state)
                        {
                            auto field = canvas.core::view();
                            auto point = body;
                            point.coor += field.coor + boss.base::coor();
                            if (auto area = field.clip(point))
                            {
                                auto& test = canvas.peek(point.coor);
                                if (test.wdt() == 2) // Extend cursor to adjacent halves.
                                {
                                    if (field.hittest(point.coor + dot_10))
                                    {
                                        auto& next = canvas.peek(point.coor + dot_10);
                                        if (next.wdt() == 3 && test.same_txt(next))
                                        {
                                            area.size.x++;
                                        }
                                    }
                                }
                                else if (test.wdt() == 3)
                                {
                                    if (field.hittest(point.coor - dot_10))
                                    {
                                        auto& prev = canvas.peek(point.coor - dot_10);
                                        if (prev.wdt() == 2 && test.same_txt(prev))
                                        {
                                            area.size.x++;
                                            area.coor.x--;
                                        }
                                    }
                                }

                                if (form)
                                {
                                    canvas.fill(area, [](cell& c)
                                    {
                                        auto b = c.bgc();
                                        auto f = c.fgc();
                                        if (c.inv()) c.bgc(f).fgc(cell::shaders::contrast.invert(f));
                                        else         c.fgc(b).bgc(cell::shaders::contrast.invert(b));
                                    });
                                }
                                else canvas.fill(area, [](cell& c) { c.und() ? c.und(0) : c.und(1); });
                            }
                            else if (area.size.y)
                            {
                                auto chr = area.coor.x ? '>' : '<';
                                area.coor.x -= area.coor.x ? 1 : 0;
                                area.size.x = 1;
                                canvas.fill(area, [&](auto& c){ c.txt(chr).fgc(cell::shaders::contrast.invert(c.bgc())); });
                            }
                        }
                    };
                }
            }
            // pro::caret: Disable caret.
            void hide()
            {
                if (*this)
                {
                    memo.clear();
                    if (done)
                    {
                        boss.deface(body);
                        done = faux;
                    }
                }
            }
        };

        // pro: Provides functionality for the title support.
        class title
            : public skill
        {
            using ansi = netxs::ansi::esc;
            using skill::boss,
                  skill::memo;

        public:
            page head_page; // title: Owner's caption header.
            page foot_page; // title: Owner's caption footer.
            ansi head_foci; // title: Original header + foci status.
            text head_text; // title: Original header.
            text foot_text; // title: Original footer.
            twod head_size; // title: Header page size.
            twod foot_size; // title: Footer page size.
            bool head_live; // title: Handle header events.
            bool foot_live; // title: Handle footer events.
            flow ooooooooo; // title: .

            struct user
            {
                id_t gear_id;
                text icon;
            };
            std::list<user> user_icon;

            bool live = true; // title: Title visibility.

            auto recalc(page& object, twod& size)
            {
                auto cp = dot_00;
                ooooooooo.flow::reset();
                ooooooooo.flow::size(size);
                auto publish = [&](auto const& combo)
                {
                    cp = ooooooooo.flow::print(combo);
                };
                object.stream(publish);
                auto& cover = ooooooooo.flow::minmax();
                size.y = cover.height() + 1;
                return cp;
            }
            void recalc(twod const& new_size)
            {
                head_size = new_size;
                foot_size = new_size;
                if (head_live) recalc(head_page, head_size);
                if (foot_live) recalc(foot_page, foot_size);
            }
            void header(view newtext)
            {
                head_text = newtext;
                rebuild();
            }
            void footer(view newtext)
            {
                foot_text = newtext;
                foot_page = foot_text;
                recalc(foot_page, foot_size);
                boss.SIGNAL(tier::release, e2::form::prop::ui::footer, foot_text);
            }
            void rebuild()
            {
                head_foci = head_text;
                if (user_icon.size())
                {
                    head_foci.add(text(user_icon.size() * 2, '\0')); // Reserv space for focus markers.
                    //if (head_live) // Add a new line if there is no space for focus markers.
                    //{
                    //    head_page = head_foci;
                    //    auto cp = recalc(head_page, head_size);
                    //    if (cp.x + user_icon.size() * 2 - 1 < head_size.x) head_foci.eol();
                    //}
                    head_foci.nop().pushsgr().chx(0).jet(bias::right);
                    for (auto& gear : user_icon)
                    {
                        head_foci.add(gear.icon);
                    }
                    head_foci.nop().popsgr();
                }
                if (head_live)
                {
                    head_page = head_foci;
                    recalc(head_page, head_size);
                }
                boss.SIGNAL(tier::release, e2::form::prop::ui::header, head_text);
                boss.SIGNAL(tier::release, e2::form::prop::ui::title , head_foci);
            }

            title(base&&) = delete;
            title(base& boss, view title = {}, view foots = {}, bool visible = true,
                                                                bool on_header = true,
                                                                bool on_footer = true)
                : skill{ boss },
                  live{ visible },
                  head_live{ on_header },
                  foot_live{ on_footer }
            {
                head_text = title;
                foot_text = foots;
                head_page = head_text;
                foot_page = foot_text;
                boss.LISTEN(tier::anycast, e2::form::upon::started, root, memo)
                {
                    if (head_live) header(head_text);
                    if (foot_live) footer(foot_text);
                    //footer(ansi::jet(bias::right).add("test\nmultiline\nfooter"));
                };
                boss.LISTEN(tier::release, e2::size::any, new_size, memo)
                {
                    recalc(new_size);
                };
                boss.LISTEN(tier::release, e2::postrender, canvas, memo)
                {
                    if (live)
                    {
                        auto saved_context = canvas.bump(dent{ 0,0,head_size.y,foot_size.y });
                        if (head_live)
                        {
                            canvas.cup(dot_00);
                            canvas.output(head_page, cell::shaders::contrast);
                        }
                        if (foot_live)
                        {
                            canvas.cup({ 0, head_size.y + boss.size().y });
                            canvas.output(foot_page, cell::shaders::contrast);
                        }
                        canvas.bump(saved_context);
                    }
                };
                boss.LISTEN(tier::release, e2::form::state::keybd::focus::on, gear_id, memo)
                {
                    if (!gear_id) return;
                    auto iter = std::find_if(user_icon.begin(), user_icon.end(), [&](auto& a){ return a.gear_id == gear_id; });
                    if (iter == user_icon.end())
                    if (auto gear_ptr = bell::getref<hids>(gear_id))
                    {
                        auto index = gear_ptr->user_index;
                        auto color = rgba::color256[4 + index % (256 - 4)];
                        auto image = netxs::ansi::fgc(color).add("\0▀"sv);
                        user_icon.push_front({ gear_id, image });
                        rebuild();
                    }
                };
                boss.LISTEN(tier::release, e2::form::state::keybd::focus::off, gear_id, memo)
                {
                    if (!gear_id) return;
                    auto iter = std::find_if(user_icon.begin(), user_icon.end(), [&](auto& a){ return a.gear_id == gear_id; });
                    if (iter != user_icon.end())
                    {
                        user_icon.erase(iter);
                        rebuild();
                    }
                };
                boss.LISTEN(tier::preview, e2::form::prop::ui::header, newtext, memo)
                {
                    header(newtext);
                };
                boss.LISTEN(tier::request, e2::form::prop::ui::header, curtext, memo)
                {
                    curtext = head_text;
                };
                boss.LISTEN(tier::request, e2::form::prop::ui::title, curtext, memo)
                {
                    curtext = head_foci;
                };
                boss.LISTEN(tier::preview, e2::form::prop::ui::footer, newtext, memo)
                {
                    footer(newtext);
                };
                boss.LISTEN(tier::request, e2::form::prop::ui::footer, curtext, memo)
                {
                    curtext = foot_text;
                };
            }
        };

        // pro: Deprecated. Perform graceful shutdown functionality. LIMIT in seconds, ESC_THRESHOLD in milliseconds.
        class guard
            : public skill
        {
            using skill::boss,
                  skill::memo;

            static constexpr auto threshold = 500ms; // guard: Double escape threshold.

            bool wait; // guard: Ready to close.
            time stop; // guard: Timeout for single Esc.

        public:
            guard(base&&) = delete;
            guard(base& boss) : skill{ boss },
                wait{ faux }
            {
                // Suspected early completion.
                boss.LISTEN(tier::release, e2::conio::preclose, pre_close, memo)
                {
                    if ((wait = pre_close))
                    {
                        stop = datetime::now() + threshold;
                    }
                };
                // Double escape catcher.
                boss.LISTEN(tier::general, e2::timer::any, timestamp, memo, (desc = "exit after preclose"s))
                {
                    if (wait && (timestamp > stop))
                    {
                        wait = faux;
                        auto shadow = boss.This();
                        boss.SIGNAL(tier::preview, e2::conio::quit, desc);
                        memo.clear();
                    }
                };
            }
        };

        // pro: Deprecated. Perform graceful shutdown functionality. LIMIT in seconds, ESC_THRESHOLD in milliseconds.
        class watch
            : public skill
        {
            using skill::boss,
                  skill::memo;

            static constexpr auto limit = 600s; //todo unify // watch: Idle timeout in seconds.

            hook pong; // watch: Alibi subsciption token.
            hook ping; // watch: Zombie check countdown token.
            time stop; // watch: Timeout for zombies.
            text desc = "no mouse clicking events";

        public:
            watch(base&&) = delete;
            watch(base& boss) : skill{ boss }
            {
                stop = datetime::now() + limit;

                // No mouse events watchdog.
                boss.LISTEN(tier::preview, hids::events::mouse::any, something, pong)
                {
                    stop = datetime::now() + limit;
                };
                boss.LISTEN(tier::general, e2::timer::any, something, ping)
                {
                    if (datetime::now() > stop)
                    {
                        auto shadow = boss.This();
                        boss.SIGNAL(tier::general, e2::shutdown, desc);
                        ping.reset();
                        memo.clear();
                    }
                };
            }
        };

        // pro: Keyboard focus.
        class focus
            : public skill
        {
            using skill::boss,
                  skill::memo;

            struct config
            {
                bool active{}; // focus: The chain is under the focus.
                bool focused{}; // focus: Focused endpoint.
                hook token; // focus: Cleanup token.
                std::list<wptr<base>> next; // focus: Focus event next hop.

                template<class P>
                auto foreach(P proc)
                {
                    auto head = next.begin();
                    auto tail = next.end();
                    while (head != tail)
                    {
                        auto n = head++;
                        if (auto nexthop = n->lock()) proc(nexthop);
                        else                          next.erase(n);
                    }
                }
            };

            //todo kb navigation type: transit, cyclic, plain, disabled, closed
            bool focusable; // focus: Boss could be a focus endpoint.
            bool scope; // focus: Cutoff threshold for the focus branch.
            //todo std::list<config>??? std::unordered_map is too expensive
            std::unordered_map<id_t, config> gears;

            template<bool On = true>
            void signal_state()
            {
                if constexpr (On == faux)
                {
                    for (auto& [gear_id, route] : gears)
                    {
                        if (gear_id != id_t{} && route.active) return;
                    }
                }
                boss.SIGNAL(tier::release, e2::form::state::keybd::focus::state, On);
            }
            auto add_route(id_t gear_id, config cfg = { .active = faux, .focused = faux })
            {
                auto iter = gears.emplace(gear_id, std::move(cfg)).first;
                if (gear_id != id_t{})
                {
                    auto& route = iter->second;
                    boss.LISTEN(tier::general, hids::events::die, gear, route.token)
                    {
                        auto iter = gears.find(gear.id);
                        if (iter != gears.end())
                        {
                            //if constexpr (debugmode) log("foci: gears cleanup boss:", boss.id, " hid:", gear.id);
                            auto& route = iter->second;
                            auto  token = std::move(route.token);
                            if (route.active) // Keep only the active branch.
                            {
                                route.active = faux;
                                gears[id_t{}] = std::move(route);
                                boss.SIGNAL(tier::release, e2::form::state::keybd::focus::off, gear.id);
                                signal_state<faux>();
                            }
                            boss.SIGNAL(tier::release, hids::events::die, gear);
                            gears.erase(iter);
                        }
                    };
                }
                return iter;
            }
            auto& get_route(id_t gear_id)
            {
                auto iter = gears.find(gear_id);
                if (iter == gears.end()) iter = add_route(gear_id);
                return iter->second;
            }

        public:
            enum class mode { hub, focusable, focused, active };
            enum class solo { off, on, mix };
            enum class flip { off = faux, on = true };
            friend auto operator ==(si32 l, solo r) { return l == static_cast<std::underlying_type_t<solo>>(r); }

            template<class T>
            static void set(sptr<base> item_ptr, T&& gear_id, solo s, flip f, bool skip = faux)
            {
                auto fire = [&](auto id)
                {
                    item_ptr->RISEUP(tier::preview, hids::events::keybd::focus::set, seed, ({ .id = id, .solo = (si32)s, .flip = (bool)f, .skip = skip }));
                    //if constexpr (debugmode) log("foci: focus set gear:", seed.id, " item:", item_ptr->id);
                };
                if constexpr (std::is_same_v<id_t, std::decay_t<T>>) fire(gear_id);
                else                    for (auto next_id : gear_id) fire(next_id);
            }
            template<class T>
            static void off(sptr<base> item_ptr, T&& gear_id)
            {
                auto fire = [&](auto id)
                {
                    item_ptr->RISEUP(tier::preview, hids::events::keybd::focus::off, seed, ({ .id = id }));
                    //if constexpr (debugmode) log("foci: focus off gear:", seed.id, " item:", item_ptr->id);
                };
                if constexpr (std::is_same_v<id_t, std::decay_t<T>>) fire(gear_id);
                else                    for (auto next_id : gear_id) fire(next_id);
            }
            static void off(sptr<base> item_ptr)
            {
                item_ptr->RISEUP(tier::request, e2::form::state::keybd::enlist, gear_id_list, ());
                pro::focus::off(item_ptr, gear_id_list);
                //if constexpr (debugmode) log("foci: full defocus item:", item_ptr->id);
            }
            static auto get(sptr<base> item_ptr, bool remove_default = faux)
            {
                item_ptr->RISEUP(tier::request, e2::form::state::keybd::enlist, gear_id_list, ());
                for (auto next_id : gear_id_list)
                {
                    item_ptr->RISEUP(tier::preview, hids::events::keybd::focus::get, seed, ({ .id = next_id }));
                    //if constexpr (debugmode) log("foci: focus get gear:", seed.id, " item:", item_ptr->id);
                }
                if (remove_default)
                if (auto parent = item_ptr->parent())
                {
                    parent->RISEUP(tier::preview, hids::events::keybd::focus::dry, seed, ({ .item = item_ptr }));
                }
                return gear_id_list;
            }

            focus(base&&) = delete;
            focus(base& boss, mode m = mode::hub, bool visible = true, bool cut_scope = faux)
                : skill{ boss },
                  focusable{ m != mode::hub && m != mode::active },
                  scope{ cut_scope }
            {
                if (m == mode::focused || m == mode::active) // Pave default focus path at startup.
                {
                    boss.LISTEN(tier::anycast, e2::form::upon::started, parent_ptr, memo, (m))
                    {
                        pro::focus::set(boss.This(), id_t{}, solo::off, flip::off, m == mode::active ? true : faux);
                    };
                }
                boss.LISTEN(tier::request, e2::form::state::keybd::check, state, memo)
                {
                    state = faux;
                    for (auto& [gear_id, route] : gears)
                    {
                        state |= gear_id != id_t{} && route.active;
                        if (state) return;
                    }
                };
                // Set unique focus on left click. Set group focus on Ctrl+LeftClick.
                boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear, memo)
                {
                    if (gear.meta(hids::anyCtrl)) pro::focus::set(boss.This(), gear.id, solo::off, flip::on );
                    else                          pro::focus::set(boss.This(), gear.id, solo::on,  flip::off);
                    gear.dismiss();
                };
                // Subscribe on keybd events.
                boss.LISTEN(tier::preview, hids::events::keybd::data::post, gear, memo) // Run after keybd::data::any.
                {
                    //if constexpr (debugmode) log("foci: data::post gear:", gear.id, " hub:", boss.id, " gears.size:", gears.size());
                    if (!gear) return;
                    auto& route = get_route(gear.id);
                    if (route.active)
                    {
                        auto alive = gear.alive;
                        auto accum = alive;
                        route.foreach([&](auto& nexthop)
                        {
                            nexthop->SIGNAL(tier::preview, hids::events::keybd::data::post, gear);
                            accum &= gear.alive;
                            gear.alive = alive;
                        });
                        gear.alive = accum;
                        if (accum) boss.SIGNAL(tier::release, hids::events::keybd::data::post, gear);
                    }
                };
                // Subscribe on focus chain events.
                boss.LISTEN(tier::release, hids::events::keybd::focus::bus::any, seed, memo) // Forward the bus event up.
                {
                    auto& route = get_route(seed.id);
                    auto deed = boss.bell::template protos<tier::release>();
                    //if constexpr (debugmode) log("foci: ", text(seed.deep++ * 4, ' '), "---bus::any gear:", seed.id, " hub:", boss.id);
                    route.foreach([&](auto& nexthop){ nexthop->bell::template signal<tier::release>(deed, seed); });
                    //if constexpr (debugmode) log("foci: ", text(--seed.deep * 4, ' '), "----------------");
                };
                boss.LISTEN(tier::release, hids::events::keybd::focus::bus::on, seed, memo)
                {
                    //if constexpr (debugmode) log("foci: ", text(seed.deep * 4, ' '), "bus::on gear:", seed.id, " hub:", boss.id, " gears.size:", gears.size());
                    auto iter = gears.find(seed.id);
                    if (iter == gears.end())
                    {
                        if (!focusable && seed.id) // Restore dtvt focus after reconnection.
                        {
                            boss.SIGNAL(tier::release, hids::events::keybd::focus::bus::copy, seed);
                        }
                        auto& route = get_route(seed.id);
                        route.active = true;
                        if (seed.id) boss.SIGNAL(tier::release, e2::form::state::keybd::focus::on, seed.id);
                    }
                    else
                    {
                        auto& route = iter->second;
                        route.active = true;
                        if (seed.id) boss.SIGNAL(tier::release, e2::form::state::keybd::focus::on, seed.id);
                    }
                    if (seed.id != id_t{}) signal_state();
                };
                boss.LISTEN(tier::release, hids::events::keybd::focus::bus::off, seed, memo)
                {
                    auto& route = get_route(seed.id);
                    if (seed.id != id_t{})
                    {
                        route.active = faux;
                        boss.SIGNAL(tier::release, e2::form::state::keybd::focus::off, seed.id);
                        signal_state<faux>();
                    }
                    //if constexpr (debugmode) log("foci: ", text(seed.deep * 4, ' '), "bus::off gear:", seed.id, " hub:", boss.id);
                };
                boss.LISTEN(tier::release, hids::events::keybd::focus::bus::copy, seed, memo) // Copy default focus route if it is and activate it.
                {
                    //if constexpr (debugmode) log("foci: ", text(seed.deep * 4, ' '), "bus::copy gear:", seed.id, " hub:", boss.id);
                    if (!gears.contains(seed.id)) // gears[seed.id] = gears[id_t{}]
                    {
                        auto def_route = gears.find(id_t{}); // Check if the default route is present.
                        if (def_route != gears.end()) add_route(seed.id, def_route->second);
                        else                          add_route(seed.id, config{});
                    }
                };
                // Truncate the maximum path without branches.
                boss.LISTEN(tier::preview, hids::events::keybd::focus::cut, seed, memo)
                {
                    auto& route = get_route(seed.id);
                    auto iter = std::find_if(route.next.begin(), route.next.end(), [&](auto& n){ return n.lock() == seed.item; });
                    if (iter != route.next.end())
                    {
                        if (scope || route.next.size() != 1) // The root of the branch.
                        {
                            route.next.erase(iter);
                        }
                        else
                        {
                            if (auto parent_ptr = boss.parent())
                            {
                                seed.item = boss.This();
                                parent_ptr->RISEUP(tier::preview, hids::events::keybd::focus::cut, seed);
                            }
                            return;
                        }
                    }
                    if (seed.item)
                    {
                        seed.item->SIGNAL(tier::release, hids::events::keybd::focus::bus::off, seed);
                        boss.expire<tier::preview>();
                    }
                };
                // Subscribe on focus offers. Build a focus tree.
                boss.LISTEN(tier::preview, hids::events::keybd::focus::set, seed, memo)
                {
                    auto focusable = seed.skip ? faux : this->focusable; // Ignore focusablity if it is requested.
                    if (!focusable && !seed.item && seed.id) // Copy the default up-route for the focus hub.
                    {
                        boss.SIGNAL(tier::release, hids::events::keybd::focus::bus::copy, seed);
                    }

                    auto& route = get_route(seed.id);
                    if (!seed.item) // No focused item. We are the first.
                    {
                        if (route.active)
                        {
                            if (seed.flip) // Focus flip-off is always a truncation of the maximum path without branches.
                            {
                                if (focusable) route.focused = faux;
                                boss.SIGNAL(tier::preview, hids::events::keybd::focus::off, seed);
                                return;
                            }
                            if (seed.solo != solo::on) // Group focus.
                            {
                                route.focused = focusable;
                                return;
                            }
                            if (focusable)
                            {
                                route.foreach([&](auto& nexthop){ nexthop->SIGNAL(tier::release, hids::events::keybd::focus::bus::off, seed); });
                                route.next.clear();
                            }
                        }
                        route.focused = focusable;
                    }
                    else // Build focus tree.
                    {
                        if (seed.solo == solo::on || (seed.solo == solo::mix && !route.active))
                        {
                            if (route.active)
                            {
                                route.foreach([&](auto& nexthop){ if (nexthop != seed.item) nexthop->SIGNAL(tier::release, hids::events::keybd::focus::bus::off, seed); });
                            }
                            route.next.clear();
                            route.next.push_back(seed.item);
                        }
                        else // Group focus.
                        {
                            auto iter = std::find_if(route.next.begin(), route.next.end(), [&](auto& n){ return n.lock() == seed.item; });
                            if (iter == route.next.end()) route.next.push_back(seed.item);
                            if (route.active)
                            {
                                seed.item->SIGNAL(tier::release, hids::events::keybd::focus::bus::on, seed);
                                return;
                            }
                        }
                    }

                    if (auto parent = boss.parent())
                    {
                        seed.item = boss.This();
                        parent->RISEUP(tier::preview, hids::events::keybd::focus::set, seed);
                    }
                };
                boss.LISTEN(tier::preview, hids::events::keybd::focus::off, seed, memo)
                {
                    auto& route = get_route(seed.id);
                    if (route.active)
                    {
                        route.focused = faux;
                        if (auto parent_ptr = boss.parent())
                        {
                            auto temp = seed.item;
                            seed.item = boss.This();
                            parent_ptr->RISEUP(tier::preview, hids::events::keybd::focus::cut, seed);
                            seed.item = temp;
                        }
                    }
                };
                boss.LISTEN(tier::preview, hids::events::keybd::focus::get, seed, memo)
                {
                    boss.SIGNAL(tier::preview, hids::events::keybd::focus::off, seed);
                    gears.erase(seed.id);
                };
                boss.LISTEN(tier::preview, hids::events::keybd::focus::dry, seed, memo)
                {
                    for (auto& [gear_id, route] : gears)
                    {
                        route.next.remove_if([&](auto& next){ return next.lock() == seed.item; });
                    }
                };
                boss.LISTEN(tier::request, e2::form::state::keybd::enlist, gear_id_list, memo)
                {
                    for (auto& [gear_id, route] : gears)
                    {
                        if (gear_id != id_t{} && route.active) gear_id_list.push_back(gear_id);
                    }
                };
                boss.LISTEN(tier::request, e2::form::state::keybd::focus::state, state, memo)
                {
                    //todo revise: same as e2::form::state::keybd::check
                    state = faux;
                    for (auto& [gear_id, route] : gears)
                    {
                        state |= gear_id != id_t{} && route.active;
                        if (state) return;
                    }
                };
                boss.LISTEN(tier::request, e2::form::state::keybd::find, gear_test, memo)
                {
                    auto iter = gears.find(gear_test.first);
                    if (iter != gears.end() && iter->second.active) gear_test.second++;
                };
                boss.LISTEN(tier::request, e2::form::state::keybd::next, gear_test, memo)
                {
                    auto iter = gears.find(gear_test.first);
                    if (iter != gears.end())
                    {
                        auto& route = iter->second;
                        if (route.active)
                        {
                            route.foreach([&](auto& nexthop){ gear_test.second++; });
                        }
                    }
                };
                boss.LISTEN(tier::general, e2::form::proceed::functor, proc, memo)
                {
                    for (auto& [gear_id, route] : gears)
                    {
                        if (gear_id != id_t{} && route.next.empty() && route.active) // route.focused === route.active & route.next.empty().
                        {
                            proc(boss.This());
                        }
                    }
                };
            }
        };
/*
        // pro: Provides functionality related to keyboard input.
        class keybd
            : public skill
        {
            using skill::boss,
                  skill::memo;

            subs kb_subs{};
            //todo foci
            //std::list<id_t> saved;

        public:
            keybd(base&&) = delete;
            keybd(base& boss) : skill{ boss }
            {
                //todo deprecated
                //boss.LISTEN(tier::preview, hids::events::keybd::data, gear, memo)
                //{
                //    boss.SIGNAL(tier::release, hids::events::keybd::data, gear);
                //};
            };

            // pro::keybd: Keybd offers promoter.
            void active()
            {
                boss.LISTEN(tier::release, hids::events::mouse::button::any, gear, kb_subs)
                {
                    if (!gear) return;
                    auto deed = boss.bell::protos<tier::release>();
                    if (deed == hids::events::mouse::button::click::left.id) //todo make it configurable (left click)
                    {
                        if (gear.meta(hids::anyCtrl)) gear.kb_offer_1(boss.This());
                        else                          gear.kb_offer_5(boss.This());
                        pro::focus::set(boss.This(), gear.id, gear.meta(hids::anyCtrl) ? pro::focus::solo::off
                                                                                       : pro::focus::solo::on, pro::focus::flip::off);
                        gear.dismiss();
                    }
                    else if (deed == hids::events::mouse::button::click::right.id) //todo make it configurable (left click)
                    {
                        gear.kb_offer_1(boss.This());
                        pro::focus::set(boss.This(), gear.id, pro::focus::solo::off, pro::focus::flip::on);
                        gear.dismiss();
                    }
                };
            }
            //todo foci
            // pro::keybd: Set focus root.
            //void master()
            //{
            //    boss.LISTEN(tier::release, hids::events::upevent::kboffer, gear, kb_subs)
            //    {
            //        log("restore");
            //        //if (boss.root()) // Restore focused state.
            //        {
            //            boss.SIGNAL(tier::anycast, hids::events::upevent::kboffer, gear);
            //        }
            //        if (gear.focus_changed())
            //        {
            //            boss.bell::expire<tier::release>();
            //        }
            //    };
            //};
            // pro::keybd: Subscribe on keybd offers.
            void accept(bool value)
            {
                if (value)
                {
                    active();
                    boss.LISTEN(tier::release, hids::events::upevent::kboffer, gear, kb_subs)
                    {
                        if (!gear.focus_changed())
                        {
                            gear.set_kb_focus(boss.This());
                            //todo foci
                            //boss.SIGNAL(tier::anycast, hids::events::upevent::kbannul, gear); // Drop saved foci.
                            boss.bell::expire<tier::release>();
                        }
                    };
                    boss.LISTEN(tier::release, hids::events::upevent::kbannul, gear, kb_subs)
                    {
                        gear.remove_from_kb_focus(boss.This());
                    };

                    ////todo foci
                    //boss.LISTEN(tier::anycast, hids::events::upevent::kboffer, gear, kb_subs) //todo no upevent used
                    //{
                    //    log("restore in place boss-id=", boss.id, " gear_id=", gear.id, " saved_size=", saved.size());
                    //    for (auto gear_id : saved) // Restore saved focus.
                    //    {
                    //        if (gear_id == gear.id)
                    //        {
                    //            log(" good ");
                    //            gear.kb_offer_1(boss.This());
                    //            pro::focus::set(boss.This(), gear.id, pro::focus::solo::off, pro::focus::flip::on);
                    //        }
                    //    }
                    //};
                    //boss.LISTEN(tier::preview, hids::events::notify::keybd::lost, gear, kb_subs) //todo no upevent used
                    //{
                    //    log("save boss.id=", boss.id, " gear_id=", gear.id);
                    //    saved.push_back(gear.id);
                    //};
                    //boss.LISTEN(tier::anycast, hids::events::upevent::kbannul, gear, kb_subs) //todo no upevent used
                    //{
                    //    if (gear.focus_force_group = faux)
                    //    {
                    //        log("wipe ", boss.id);
                    //        saved.remove_if([&](auto&& gear_id) { return gear_id == gear.id; });
                    //    }
                    //};
                }
                else
                {
                    kb_subs.clear();
                }
            }
        };
*/
        // pro: Provides functionality related to mouse interaction.
        class mouse
            : public skill
        {
            struct sock
            {
                operator bool () { return true; }
            };

            using list = socks<sock>;
            using skill::boss,
                  skill::memo;

            sptr<base> soul; // mouse: Boss cannot be removed while it has active gears.
            si32       rent; // mouse: Active gears count.
            si32       full; // mouse: All gears count. Counting to keep the entire chain of links in the visual tree.
            bool       omni; // mouse: Ability to accept all hover events (true) or only directly over the object (faux).
            si32       drag; // mouse: Bitfield of buttons subscribed to mouse drag.
            list       mice; // mouse: List of active mice.
            std::map<si32, subs> dragmemo; // mouse: Draggable subs.

        public:
            mouse(base&&) = delete;
            mouse(base& boss, bool take_all_events = true)
                : skill{ boss            },
                   mice{ boss            },
                   omni{ take_all_events },
                   rent{ 0               },
                   full{ 0               },
                   drag{ 0               }
            {
                auto brush = boss.base::color();
                boss.base::color(brush.link(boss.bell::id));
                // pro::mouse: Refocus all active mice on detach (to keep the mouse event tree consistent).
                boss.LISTEN(tier::release, e2::form::upon::vtree::detached, parent_ptr, memo)
                {
                    if (parent_ptr)
                    {
                        auto& parent = *parent_ptr;
                        mice.foreach([&](auto& gear)
                        {
                            if (auto gear_ptr = bell::getref<hids>(gear.id))
                            {
                                gear_ptr->redirect_mouse_focus(parent);
                            }
                        });
                    }
                };
                // pro::mouse: Propagate form events down to the visual branch. Executed last.
                boss.LISTEN(tier::release, hids::events::notify::any, gear)
                {
                    if (auto parent_ptr = boss.parent())
                    if (auto deed = boss.bell::protos<tier::release>())
                    {
                        parent_ptr->bell::signal<tier::release>(deed, gear);
                    }
                };
                // pro::mouse: Forward preview to all parents.
                boss.LISTEN(tier::preview, hids::events::mouse::any, gear, memo)
                {
                    auto& offset = boss.base::coor();
                    gear.pass<tier::preview>(boss.parent(), offset);

                    if (gear) gear.okay(boss);
                    else      boss.bell::expire<tier::preview>();
                };
                // pro::mouse: Forward all not expired mouse events to all parents.
                boss.LISTEN(tier::release, hids::events::mouse::any, gear, memo)
                {
                    if (gear && !gear.captured())
                    {
                        auto& offset = boss.base::coor();
                        gear.pass<tier::release>(boss.parent(), offset);
                    }
                };
                // pro::mouse: Notify form::state::active when the number of clients is positive.
                boss.LISTEN(tier::release, hids::events::notify::mouse::enter, gear, memo)
                {
                    if (!full++)
                    {
                        soul = boss.This();
                    }
                    if (gear.direct<true>(boss.bell::id) || omni)
                    {
                        if (!rent++)
                        {
                            boss.SIGNAL(tier::release, e2::form::state::mouse, rent);
                        }
                    }
                    //if constexpr (debugmode) log("Enter boss:", boss.id, " full:", full);
                };
                // pro::mouse: Notify form::state::active when the number of clients is zero.
                boss.LISTEN(tier::release, hids::events::notify::mouse::leave, gear, memo)
                {
                    if (gear.direct<faux>(boss.bell::id) || omni)
                    {
                        if (!--rent)
                        {
                            boss.SIGNAL(tier::release, e2::form::state::mouse, rent);
                        }
                    }
                    //if constexpr (debugmode) log("Leave boss:", boss.id, " full:", full - 1);
                    if (!--full)
                    {
                        soul->base::strike();
                        soul.reset();
                    }
                };
                boss.LISTEN(tier::request, e2::form::state::mouse, state, memo)
                {
                    state = rent;
                };
                boss.LISTEN(tier::release, e2::form::draggable::any, enabled, memo)
                {
                    switch (auto deed = boss.bell::protos<tier::release>())
                    {
                        default:
                        case e2::form::draggable::left     .id: draggable<hids::buttons::left     >(enabled); break;
                        case e2::form::draggable::right    .id: draggable<hids::buttons::right    >(enabled); break;
                        case e2::form::draggable::middle   .id: draggable<hids::buttons::middle   >(enabled); break;
                        case e2::form::draggable::wheel    .id: draggable<hids::buttons::wheel    >(enabled); break;
                        case e2::form::draggable::win      .id: draggable<hids::buttons::win      >(enabled); break;
                        case e2::form::draggable::leftright.id: draggable<hids::buttons::leftright>(enabled); break;
                    }
                };
            }
            void reset()
            {
                auto lock = events::sync{};
                if (full)
                {
                    full = 0;
                    soul.reset();
                }
            }
            void take_all_events(bool b)
            {
                omni = b;
            }
            template<hids::buttons Button>
            void draggable(bool enabled)
            {
                if (!enabled)
                {
                    dragmemo[Button].clear();
                    drag &= ~(1 << Button);
                }
                else if (!(drag & 1 << Button))
                {
                    drag |= 1 << Button;
                    //using bttn = hids::events::mouse::button; //MSVC 16.9.4 don't get it
                    boss.LISTEN(tier::release, hids::events::mouse::button::drag::start::_<Button>, gear, dragmemo[Button])
                    {
                        if (gear.capture(boss.bell::id))
                        {
                            boss.SIGNAL(tier::release, e2::form::drag::start::_<Button>, gear);
                            gear.dismiss();
                        }
                    };
                    boss.LISTEN(tier::release, hids::events::mouse::button::drag::pull::_<Button>, gear, dragmemo[Button])
                    {
                        if (gear.captured(boss.bell::id))
                        {
                            boss.SIGNAL(tier::release, e2::form::drag::pull::_<Button>, gear);
                            gear.dismiss();
                        }
                    };
                    boss.LISTEN(tier::release, hids::events::mouse::button::drag::cancel::_<Button>, gear, dragmemo[Button])
                    {
                        if (gear.captured(boss.bell::id))
                        {
                            boss.SIGNAL(tier::release, e2::form::drag::cancel::_<Button>, gear);
                            gear.setfree();
                            gear.dismiss();
                        }
                    };
                    boss.LISTEN(tier::general, hids::events::halt, gear, dragmemo[Button])
                    {
                        if (gear.captured(boss.bell::id))
                        {
                            boss.SIGNAL(tier::release, e2::form::drag::cancel::_<Button>, gear);
                            gear.setfree();
                            gear.dismiss();
                        }
                    };
                    boss.LISTEN(tier::release, hids::events::mouse::button::drag::stop::_<Button>, gear, dragmemo[Button])
                    {
                        if (gear.captured(boss.bell::id))
                        {
                            boss.SIGNAL(tier::release, e2::form::drag::stop::_<Button>, gear);
                            gear.setfree();
                            gear.dismiss();
                        }
                    };
                }
            }
        };

        // pro: Glow gradient filter.
        class grade
            : public skill
        {
            using skill::boss,
                  skill::memo;

        public:
            grade(base&&) = delete;
            grade(base& boss) : skill{ boss }
            {
                boss.LISTEN(tier::release, e2::postrender, parent_canvas, memo)
                {
                    auto size = si32{ 5 }; // grade: Vertical gradient size.
                    auto step = si32{ 2 }; // grade: Vertical gradient step.
                    //cell shadow{ cell{}.vis(cell::highlighter) };
                    //cell bright{ cell{}.vis(cell::darklighter) };
                    auto shadow = rgba{0xFF000000};
                    auto bright = rgba{0xFFffffff};

                    //todo optimize - don't fill the head and foot twice
                    auto area = parent_canvas.view();
                    auto n = std::clamp(size, 0, area.size.y / 2) + 1;
                    //auto n = std::clamp(size, 0, boss.base::size().y / 2) + 1;

                    auto head = area;
                    head.size.y = 1;
                    auto foot = head;
                    head.coor.y = area.coor.y + n - 2;
                    foot.coor.y = area.coor.y + area.size.y - n + 1;

                    for (auto i = 1; i < n; i++)
                    {
                        bright.alpha(i * step);
                        shadow.alpha(i * step);

                        parent_canvas.core::fill(head, [&](cell& c) { c.bgc().mix(bright); });
                        parent_canvas.core::fill(foot, [&](cell& c) { c.bgc().mix(shadow); });

                        head.coor.y--;
                        foot.coor.y++;
                    }
                };
            }
        };

        // pro: Fader animation.
        class fader
            : public skill
        {
            using skill::boss,
                  skill::memo;

            robot  robo;   // fader: .
            span fade;
            si32 transit;
            cell c1;
            cell c2;
            cell c2_orig;
            bool fake = faux;

            //todo use lambda
            void work(si32 transit)
            {
                auto brush = boss.base::color();
                brush.avg(c1, c2, transit);
                fake = true;
                boss.base::color(brush);
                fake = faux;
                boss.base::deface();
            }

        public:
            fader(base&&) = delete;
            fader(base& boss, cell default_state, cell highlighted_state, span fade_out = 250ms)
                : skill{ boss },
                robo{ boss },
                fade{ fade_out },
                c1 { default_state },
                c2 { highlighted_state },
                c2_orig { highlighted_state },
                transit{ 0 }
            {
                boss.base::color(c1.fgc(), c1.bgc());
                boss.LISTEN(tier::release, e2::form::prop::brush, brush)
                {
                    if (!fake)
                    {
                        auto& fgc = brush.fgc();
                        auto& bgc = brush.bgc();
                        c1.fgc(fgc);
                        c1.bgc(bgc);
                        if (brush.fga()) c2.fgc(fgc);
                        else             c2.fgc(c2_orig.fgc());
                        if (brush.bga()) c2.bgc(bgc);
                        else             c2.bgc(c2_orig.bgc());
                        work(transit);
                    }
                };
                boss.LISTEN(tier::release, e2::form::state::mouse, active, memo)
                {
                    robo.pacify();
                    if (active)
                    {
                        transit = 256;
                        work(transit);
                    }
                    else
                    {
                        if (fade != fade.zero())
                        {
                            auto range = transit;
                            auto limit = datetime::round<si32>(fade);
                            auto start = 0;
                            robo.actify(constlinearAtoB<si32>(range, limit, start), [&](auto step)
                            {
                                transit -= step;
                                work(transit);
                            });
                        }
                        else work(transit = 0);
                    }
                };
            }
        };

        // pro: Limits manager.
        class limit
            : public skill
        {
            using skill::boss,
                  skill::memo;

            struct lims_t
            {
                twod min = skin::globals().min_value;
                twod max = skin::globals().max_value;
                void fixed_size(twod const& m)
                {
                    min = max = std::clamp(m, min, max);;
                }
            }
            lims;
            bool sure; // limit: Reepeat size checking afetr all.

        public:
            limit(base&&) = delete;
            limit(base& boss, twod const& min_size = -dot_11, twod const& max_size = -dot_11, bool forced_clamp = faux, bool forced_resize = faux)
                : skill{ boss }
            {
                set(min_size, max_size, forced_clamp);
                // Clamping before all.
                boss.LISTEN(tier::preview, e2::size::any, new_size, memo)
                {
                    new_size = std::clamp(new_size, lims.min, lims.max);
                };
                // Clamping after all.
                boss.LISTEN(tier::preview, e2::size::set, new_size, memo)
                {
                    if (sure)
                    {
                        new_size = std::clamp(new_size, lims.min, lims.max);
                    }
                };
                if (forced_resize)
                {
                    boss.LISTEN(tier::release, e2::form::prop::window::size, new_size, memo)
                    {
                        auto reserv = lims;
                        lims.fixed_size(new_size);
                        boss.template riseup<tier::release>(e2::form::prop::fixedsize, true, true); //todo unify - Inform ui::fork to adjust ratio.
                        boss.base::template reflow<true>();
                        boss.template riseup<tier::release>(e2::form::prop::fixedsize, faux, true);
                        lims = reserv;
                    };
                }
            }
            // pro::limit: Set size limits (min, max). Preserve current value if specified arg less than 0.
            void set(twod const& min_size, twod const& max_size = -dot_11, bool forced_clamp = faux)
            {
                sure = forced_clamp;
                lims.min = min_size.less(dot_00, skin::globals().min_value, min_size);
                lims.max = max_size.less(dot_00, skin::globals().max_value, max_size);
            }
            // pro::limit: Set resize limits (min, max). Preserve current value if specified arg less than 0.
            void set(lims_t const& new_limits, bool forced_clamp = faux)
            {
                set(new_limits.min, new_limits.max, forced_clamp);
            }
            auto& get() const
            {
                return lims;
            }
        };

        // pro: UI-control cache.
        class cache
            : public skill
        {
            using skill::boss,
                  skill::memo;

            sptr<face> coreface;
            byte       lucidity;
            bool       usecache;

        public:
            face& canvas; // cache: Bitmap cache.

            cache(base&&) = delete;
            cache(base& boss, bool rendered = true)
                : skill{ boss },
                  canvas{*(coreface = ptr::shared<face>())},
                  lucidity{ 0xFF },
                  usecache{ true }
            {
                canvas.link(boss.bell::id);
                canvas.move(boss.base::coor());
                canvas.size(boss.base::size());
                boss.LISTEN(tier::preview, e2::form::prop::ui::cache, state, memo)
                {
                    usecache = state;
                };
                boss.LISTEN(tier::anycast, e2::form::prop::lucidity, value, memo)
                {
                    if (value == -1)
                    {
                        value = lucidity;
                    }
                    else
                    {
                        lucidity = value;
                        //boss.deface();
                    }
                };
                boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent_ptr, memo)
                {
                    boss.SIGNAL(tier::general, e2::form::canvas, canvas.shared_from_this());
                };
                boss.LISTEN(tier::release, e2::coor::any, new_xy,        memo) { canvas.move(new_xy); };
                boss.LISTEN(tier::release, e2::size::any, new_sz,        memo) { canvas.size(new_sz); };
                boss.LISTEN(tier::request, e2::form::canvas, canvas_ptr, memo) { canvas_ptr = coreface; };
                if (rendered)
                {
                    boss.LISTEN(tier::release, e2::render::prerender, parent_canvas, memo)
                    {
                        if (!usecache) return;
                        if (boss.base::ruined())
                        {
                            canvas.wipe();
                            boss.base::ruined(faux);
                            boss.SIGNAL(tier::release, e2::render::any, canvas);
                        }
                        if (lucidity == 0xFF) parent_canvas.fill(canvas, cell::shaders::fusefull);
                        else                  parent_canvas.fill(canvas, cell::shaders::transparent(lucidity));
                        boss.bell::expire<tier::release>();
                    };
                }
            }
        };

        // pro: Acrylic blur.
        class acryl
            : public skill
        {
            using skill::boss,
                  skill::memo;

            si32 width; // acryl: Blur radius.
            bool alive; // acryl: Is active.

        public:
            acryl(base&&) = delete;
            acryl(base& boss, si32 size = 5)
                : skill{ boss },
                  width{ size },
                  alive{ true }
            {
                boss.LISTEN(tier::preview, e2::form::prop::ui::acryl, state, memo)
                {
                    alive = state;
                };
                boss.LISTEN(tier::anycast, e2::form::prop::lucidity, lucidity, memo)
                {
                    if (lucidity != -1) alive = lucidity == 0xFF;
                };
                boss.LISTEN(tier::release, e2::render::prerender, parent_canvas, memo)
                {
                    if (!alive) return;
                    auto brush = boss.base::color();
                    if (brush.wdt()) parent_canvas.blur(width, [&](cell& c) { c.alpha(0xFF).fuse(brush); });
                    else             parent_canvas.blur(width, [&](cell& c) { c.alpha(0xFF); });
                };
            }
        };

        // pro: Background Highlighter.
        class light
            : public skill
        {
            using skill::boss,
                  skill::memo;

            bool highlighted = faux; // light: .
            rgba title_fg_color = 0xFFffffff;

        public:
            light(base&&) = delete;
            light(base& boss, bool track_mouse = faux)
                : skill{ boss }
            {
                boss.LISTEN(tier::release, e2::form::state::highlight, state, memo)
                {
                    highlighted = state;
                    boss.base::deface();
                };
                boss.LISTEN(tier::release, e2::render::prerender, parent_canvas, memo)
                {
                    if (highlighted)
                    {
                        auto area = parent_canvas.full();
                        auto mark = skin::color(tone::brighter);
                             mark.fgc(title_fg_color); //todo unify, make it more contrast
                        auto fill = [&](cell& c) { c.fuse(mark); };
                        parent_canvas.fill(area, fill);
                    }
                };
            }
        };

        // pro: Custom highlighter.
        template<auto fx>
        class shade
            : public skill
        {
            using skill::boss,
                  skill::memo;

            bool highlighted = faux; // light2: .

        public:
            shade(base&&) = delete;
            shade(base& boss)
                : skill{ boss }
            {
                boss.LISTEN(tier::release, e2::form::state::mouse, active, memo)
                {
                    highlighted = active;
                    boss.base::deface();
                };
                boss.LISTEN(tier::release, e2::postrender, parent_canvas, memo)
                {
                    if (highlighted)
                    {
                        auto area = parent_canvas.full();
                        parent_canvas.fill(area, fx);
                    }
                };
            }
        };

        // pro: Drag&roll.
        class glide
            : public skill
        {
            using skill::boss,
                  skill::memo;

        public:
            glide(base&&) = delete;
            glide(base& boss)
                : skill{ boss }
            {

            }
        };

        // pro: Tooltip.
        class notes
            : public skill
        {
            using skill::boss,
                  skill::memo;

            text note;

        public:
            notes(base&&) = delete;
            notes(base& boss, view data, dent wrap = { maxsi32 })
                : skill{ boss },
                  note { data }
            {
                boss.LISTEN(tier::release, hids::events::notify::mouse::enter, gear, memo, (wrap, full = wrap.west.step == maxsi32))
                {
                    if (full || !(boss.area() + wrap).hittest(gear.coord + boss.coor()))
                    {
                         gear.set_tooltip(boss.id, note);
                    }
                };
                boss.LISTEN(tier::preview, e2::form::prop::ui::tooltip, new_note, memo)
                {
                    note = new_note;
                };
            }
            void update(view new_note)
            {
                note = new_note;
            }
        };
    }

    // console: Client gate.
    class gate
        : public base
    {
        // gate: Data decoder.
        struct link
            : public s11n
        {
            struct relay_t
            {
                using lock = std::recursive_mutex;
                using cond = std::condition_variable_any;

                struct clip_t
                {
                    lock mutex{};
                    cond synch{};
                    bool ready{};
                    twod block{};
                    clip chunk{};
                };
                using umap = std::unordered_map<id_t, clip_t>;

                umap depot{};
                lock mutex{};

                void set(id_t id, view utf8, clip::mime kind)
                {
                    auto lock = std::lock_guard{ mutex };
                    auto iter = depot.find(id);
                    if (iter != depot.end())
                    {
                        auto& item = iter->second;
                        auto  lock = std::lock_guard{ item.mutex };
                        item.chunk.utf8 = utf8;
                        item.chunk.kind = kind;
                        item.ready = true;
                        item.synch.notify_all();
                    }
                }
            };

            pipe&    canal; // link: Data highway.
            base&    owner; // link: Link owner.
            relay_t  relay; // link: Clipboard relay.

            // link: Send data outside.
            void output(view data)
            {
                canal.output(data);
            }
            // link: .
            auto request_clip_data(id_t ext_gear_id, clip& clip_rawdata)
            {
                relay.mutex.lock();
                auto& selected_depot = relay.depot[ext_gear_id]; // If rehashing occurs due to the insertion, all iterators are invalidated.
                relay.mutex.unlock();
                auto lock = std::unique_lock{ selected_depot.mutex };
                selected_depot.ready = faux;
                request_clipboard.send(canal, ext_gear_id);
                auto maxoff = 100ms; //todo magic numbers
                auto received = std::cv_status::timeout != selected_depot.synch.wait_for(lock, maxoff);
                if (received)
                {
                    clip_rawdata = selected_depot.chunk;
                }
                return received;
            }

            link(pipe& canal, base& owner)
                : s11n{ *this },
                 canal{ canal },
                 owner{ owner }
            { }

            // link: Send an event message to the link owner.
            template<tier Tier = tier::release, class E, class T>
            void notify(E, T&& data)
            {
                netxs::events::enqueue(owner.This(), [d = data](auto& boss) mutable
                {
                    //boss.SIGNAL(Tier, E{}, d); // VS2022 17.4.1 doesn't get it for some reason (nested lambdas + static_cast + decltype(...)::type).
                    boss.bell::template signal<Tier>(E::id, static_cast<typename E::type &&>(d));
                });
            }
            void handle(s11n::xs::focusbus    lock)
            {
                auto& focus = lock.thing;
                auto deed = netxs::events::makeid(hids::events::keybd::focus::bus::any.id, focus.cause);
                if (focus.guid != os::process::id.second || deed != hids::events::keybd::focus::bus::copy.id) // To avoid focus tree infinite looping.
                netxs::events::enqueue(owner.This(), [d = focus, deed](auto& boss) mutable
                {
                    auto seed = hids::events::keybd::focus::bus::on.param({ .id = d.gear_id });
                    boss.bell::template signal<tier::release>(deed, seed);
                });
            }
            void handle(s11n::xs::sysfocus    lock)
            {
                auto& focus = lock.thing;
                notify(e2::conio::focus, focus);
            }
            void handle(s11n::xs::winsz       lock)
            {
                auto& item = lock.thing;
                notify(e2::conio::winsz, item.winsize);
            }
            void handle(s11n::xs::clipdata    lock)
            {
                auto& item = lock.thing;
                relay.set(item.gear_id, item.data, static_cast<clip::mime>(item.mimetype));
            }
            void handle(s11n::xs::osclipdata  lock)
            {
                auto& item = lock.thing;
                notify(e2::conio::clipdata, clip{ dot_00, item.data, static_cast<clip::mime>(item.mimetype) });
            }
            void handle(s11n::xs::syskeybd    lock)
            {
                auto& keybd = lock.thing;
                notify(e2::conio::keybd, keybd);
            }
            void handle(s11n::xs::plain       lock)
            {
                auto k = s11n::syskeybd.freeze();
                auto& keybd = k.thing;
                auto& item = lock.thing;
                keybd.wipe();
                keybd.gear_id = item.gear_id;
                keybd.cluster = item.utf8txt;
                keybd.pressed = true;
                notify(e2::conio::keybd, keybd);
                keybd.pressed = faux;
                notify(e2::conio::keybd, keybd);
            }
            void handle(s11n::xs::ctrls       lock)
            {
                auto k = s11n::syskeybd.freeze();
                auto& keybd = k.thing;
                auto& item = lock.thing;
                keybd.wipe();
                keybd.gear_id = item.gear_id;
                keybd.ctlstat = item.ctlstat;
                keybd.pressed = faux;
                notify(e2::conio::keybd, keybd);
            }
            void handle(s11n::xs::sysmouse    lock)
            {
                auto& mouse = lock.thing;
                notify(e2::conio::mouse, mouse);
            }
            void handle(s11n::xs::mouse_show  lock)
            {
                auto& item = lock.thing;
                notify(e2::conio::pointer, item.mode);
            }
            void handle(s11n::xs::request_gc  lock)
            {
                auto& items = lock.thing;
                auto list = jgc_list.freeze();
                for (auto& gc : items)
                {
                    auto cluster = cell::gc_get_data(gc.token);
                    list.thing.push(gc.token, cluster);
                }
                list.thing.sendby(canal);
            }
            void handle(s11n::xs::fps         lock)
            {
                auto& item = lock.thing;
                notify(e2::config::fps, item.frame_rate);
            }
            void handle(s11n::xs::bgc         lock)
            {
                auto& item = lock.thing;
                notify<tier::anycast>(e2::form::prop::colors::bg, item.color);
            }
            void handle(s11n::xs::fgc         lock)
            {
                auto& item = lock.thing;
                notify<tier::anycast>(e2::form::prop::colors::fg, item.color);
            }
            void handle(s11n::xs::slimmenu    lock)
            {
                auto& item = lock.thing;
                notify<tier::anycast>(e2::form::prop::ui::slimmenu, item.menusize);
            }
            void handle(s11n::xs::form_header lock)
            {
                auto& item = lock.thing;
                notify<tier::preview>(e2::form::prop::ui::header, item.new_header); //todo window_id
            }
            void handle(s11n::xs::form_footer lock)
            {
                auto& item = lock.thing;
                notify<tier::preview>(e2::form::prop::ui::footer, item.new_footer); //todo window_id
            }
        };

        // gate: Bitmap forwarder.
        struct diff
        {
            using work = std::thread;
            using lock = std::mutex;
            using cond = std::condition_variable_any;

            struct stat
            {
                span watch{}; // diff::stat: Duration of the STDOUT rendering.
                sz_t delta{}; // diff::stat: Last ansi-rendered frame size.
            };

            pipe& canal;
            lock  mutex; // diff: Mutex between renderer and committer threads.
            cond  synch; // diff: Synchronization between renderer and committer.
            core  cache; // diff: The current content buffer which going to be checked and processed.
            flag  alive; // diff: Working loop state.
            flag  ready; // diff: Conditional variable to avoid spurious wakeup.
            flag  abort; // diff: Abort building current frame.
            work  paint; // diff: Rendering thread.
            stat  debug; // diff: Debug info.

            // diff: Render current buffer to the screen.
            template<class Bitmap>
            void render()
            {
                log("diff: id: ", std::this_thread::get_id(), " rendering thread started");
                auto start = time{};
                auto image = Bitmap{};
                auto guard = std::unique_lock{ mutex };
                while ((void)synch.wait(guard, [&]{ return !!ready; }), alive)
                {
                    start = datetime::now();
                    ready = faux;
                    abort = faux;
                    auto winid = id_t{ 0xddccbbaa };
                    auto coord = dot_00;
                    image.set(winid, coord, cache, abort, debug.delta);
                    if (debug.delta)
                    {
                        canal.isbusy = true; // It's okay if someone resets the busy flag before sending.
                        image.sendby(canal);
                        canal.isbusy.wait(true); // Successive frames must be discarded until the current frame is delivered (to prevent unlimited buffer growth).
                    }
                    debug.watch = datetime::now() - start;
                }
                log("diff: id: ", std::this_thread::get_id(), " rendering thread ended");
            }
            // diff: Get rendering statistics.
            auto status()
            {
                return debug;
            }
            // diff: Discard current frame.
            void cancel()
            {
                abort = true;
            }
            // diff: Obtain new content to render.
            auto commit(core const& canvas)
            {
                if (abort)
                {
                    while (alive) // Try to send a new frame as soon as possible (e.g. after resize).
                    {
                        auto lock = std::unique_lock{ mutex, std::try_to_lock };
                        if (lock.owns_lock())
                        {
                            cache = canvas;
                            ready = true;
                            synch.notify_one();
                            return true;
                        }
                        else std::this_thread::yield();
                    }
                }
                else
                {
                    auto lock = std::unique_lock{ mutex, std::try_to_lock };
                    if (lock.owns_lock())
                    {
                        cache = canvas;
                        ready = true;
                        synch.notify_one();
                        return true;
                    }
                }
                return faux;
            }

            diff(pipe& dest, svga vtmode)
                : canal{ dest },
                  alive{ true },
                  ready{ faux },
                  abort{ faux }
            {
                using namespace netxs::directvt;
                paint = work([&, vtmode]
                {
                    //todo revise (bitmap/bitmap_t)
                         if (vtmode == svga::dtvt     ) render<binary::bitmap_t>               ();
                    else if (vtmode == svga::truecolor) render< ascii::bitmap<svga::truecolor>>();
                    else if (vtmode == svga::vga16    ) render< ascii::bitmap<svga::vga16    >>();
                    else if (vtmode == svga::vga256   ) render< ascii::bitmap<svga::vga256   >>();
                });
            }
            void stop()
            {
                if (!alive.exchange(faux)) return;
                auto id = paint.get_id();
                while (true)
                {
                    auto guard = std::unique_lock{ mutex, std::try_to_lock };
                    if (guard.owns_lock())
                    {
                        ready = true;
                        synch.notify_all();
                        break;
                    }
                    canal.isbusy = faux;
                    canal.isbusy.notify_all();
                    std::this_thread::yield();
                }
                paint.join();
                log("diff: id: ", id, " rendering thread joined");
            }
        };

        // gate: Application properties.
        struct props_t
        {
            //todo revise
            text os_user_id;
            text title;
            text selected;
            span clip_preview_time;
            cell clip_preview_clrs;
            byte clip_preview_alfa;
            bool clip_preview_show;
            twod clip_preview_size;
            si32 clip_preview_glow;
            cell background_color;
            face background_image;
            si32 legacy_mode;
            si32 session_id;
            span dblclick_timeout; // conf: Double click timeout.
            span tooltip_timeout; // conf: Timeout for tooltip.
            cell tooltip_colors; // conf: Tooltip rendering colors.
            bool tooltip_enabled; // conf: Enable tooltips.
            bool glow_fx; // conf: Enable glow effect in main menu.
            bool debug_overlay; // conf: Enable to show debug overlay.
            text debug_toggle; // conf: Debug toggle shortcut.
            bool show_regions; // conf: Highlight region ownership.
            bool simple; // conf: .
            svga vtmode; // conf: .

            void read(xmls& config)
            {
                config.cd("/config/client/");
                clip_preview_clrs = config.take("clipboard/preview"        , cell{}.bgc(bluedk).fgc(whitelt));
                clip_preview_time = config.take("clipboard/preview/timeout", span{ 3s });
                clip_preview_alfa = config.take("clipboard/preview/alpha"  , 0xFF);
                clip_preview_glow = config.take("clipboard/preview/shadow" , 7);
                clip_preview_show = config.take("clipboard/preview/enabled", true);
                clip_preview_size = config.take("clipboard/preview/size"   , twod{ 80,25 });
                dblclick_timeout  = config.take("mouse/dblclick"           , span{ 500ms });
                tooltip_colors    = config.take("tooltips"                 , cell{}.bgc(0xFFffffff).fgc(0xFF000000));
                tooltip_timeout   = config.take("tooltips/timeout"         , span{ 2000ms });
                tooltip_enabled   = config.take("tooltips/enabled"         , true);
                debug_overlay     = config.take("debug/overlay"            , faux);
                debug_toggle      = config.take("debug/toggle"             , "🐞"s);
                show_regions      = config.take("regions/enabled"          , faux);
                clip_preview_glow = std::clamp(clip_preview_glow, 0, 10);
            }

            props_t(pipe& canal, view userid, si32 mode, bool isvtm, si32 session_id, xmls& config)
            {
                read(config);
                legacy_mode = mode;
                if (isvtm)
                {
                    this->session_id  = session_id;
                    os_user_id        = utf::concat("[", userid, ":", session_id, "]");
                    title             = os_user_id;
                    selected          = config.take("/config/menu/selected", ""s);
                    background_color  = cell{}.fgc(config.take("background/fgc", rgba{ whitedk }))
                                              .bgc(config.take("background/bgc", rgba{ 0xFF000000 }));
                    auto utf8_tile = config.take("background/tile", ""s);
                    if (utf8_tile.size())
                    {
                        auto block = page{ utf8_tile };
                        background_image.size(block.limits());
                        background_image.output(block);
                    }
                    glow_fx           = config.take("glowfx", true);
                    simple            = faux;
                }
                else
                {
                    simple            = !(legacy_mode & os::vt::direct);
                    glow_fx           = faux;
                    title             = "";
                }
                vtmode = legacy_mode & os::vt::vga16  ? svga::vga16
                       : legacy_mode & os::vt::vga256 ? svga::vga256
                       : legacy_mode & os::vt::direct ? svga::dtvt
                                                      : svga::truecolor;                
            }

            friend auto& operator << (std::ostream& s, props_t const& c)
            {
                return s << "\n\tuser: " << c.os_user_id
                         << "\n\tmode: " << os::vt::str(c.legacy_mode);
            }
        };

        // gate: .
        struct input_t
        {
            using depo = std::unordered_map<id_t, sptr<hids>>;
            using lock = std::recursive_mutex;

            template<class T>
            void forward(T& device)
            {
                auto gear_it = gears.find(device.gear_id);
                if (gear_it == gears.end())
                {
                    gear_it = gears.emplace(device.gear_id, bell::create<hids>(boss.props, device.gear_id == 0, boss, xmap)).first;
                }
                auto& [_id, gear_ptr] = *gear_it;
                gear_ptr->hids::take(device);
                boss.strike();
            }

            gate& boss;
            subs  memo;
            face  xmap;
            lock  sync;
            depo  gears;

            input_t(props_t& props, gate& boss)
                : boss{ boss }
            {
                xmap.cmode = props.vtmode;
                xmap.mark(props.background_color.txt(whitespace).link(boss.bell::id));
                xmap.move(boss.base::coor());
                xmap.size(boss.base::size());
                boss.LISTEN(tier::release, e2::command::printscreen, gear, memo)
                {
                    auto data = ansi::esc{};
                    data.s11n(xmap, gear.slot);
                    if (data.length())
                    {
                        gear.set_clip_data(clip{ gear.slot.size, data, clip::ansitext });
                    }
                };
                boss.LISTEN(tier::release, e2::form::prop::brush, brush, memo)
                {
                    auto guard = std::lock_guard{ sync }; // Syncing with diff::render thread.
                    xmap.mark(brush);
                };
                boss.LISTEN(tier::release, e2::size::any, newsize, memo)
                {
                    auto guard = std::lock_guard{ sync }; // Syncing with diff::render thread.
                    xmap.size(newsize);
                };
                boss.LISTEN(tier::release, e2::coor::any, newcoor, memo)
                {
                    auto guard = std::lock_guard{ sync }; // Syncing with diff::render thread.
                    xmap.move(newcoor);
                };
                boss.LISTEN(tier::release, e2::conio::mouse, m, memo)
                {
                    if (m.enabled != hids::stat::ok)
                    {
                        auto gear_it = gears.find(m.gear_id);
                        if (gear_it != gears.end())
                        {
                            switch (m.enabled)
                            {
                                case hids::stat::ok:   break;
                                case hids::stat::halt: gear_it->second->deactivate(); break;
                                case hids::stat::die:  gears.erase(gear_it);          break;
                            }
                        }
                        boss.strike();
                    }
                    else forward(m);
                };
                boss.LISTEN(tier::release, e2::conio::keybd, k, memo)
                {
                    forward(k);
                };
                boss.LISTEN(tier::release, e2::conio::focus, f, memo)
                {
                    forward(f);
                };
            }
            void fire(hint event_id)
            {
                for (auto& [id, gear_ptr] : gears)
                {
                    auto& gear = *gear_ptr;
                    gear.fire_fast();
                    gear.fire(event_id);
                }
            }
            auto get_foreign_gear_id(id_t gear_id)
            {
                for (auto& [foreign_id, gear_ptr] : gears)
                {
                    if (gear_ptr->id == gear_id) return std::pair{ foreign_id, gear_ptr };
                }
                return std::pair{ id_t{}, sptr<hids>{} };
            }
            auto set_clip_data(clip const& clipdata)
            {
                if (gears.empty())
                {
                    gears.emplace(0, bell::create<hids>(boss.props, true, boss, xmap));
                }
                for (auto& [id, gear_ptr] : gears)
                {
                    auto& gear = *gear_ptr;
                    gear.set_clip_data(clipdata, faux);
                }
            }
        };

        // gate: Realtime telemetry.
        struct debug_t
        {
            #define prop_list                     \
            X(total_size   , "total sent"       ) \
            X(proceed_ns   , "rendering time"   ) \
            X(render_ns    , "stdout time"      ) \
            X(frame_size   , "frame size"       ) \
            X(frame_rate   , "frame rate"       ) \
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
            X(mouse_btn_3  , "middle button"    ) \
            X(mouse_btn_4  , "4th button"       ) \
            X(mouse_btn_5  , "5th button"       ) \
            X(mouse_btn_6  , "left+right combo" ) \
            X(last_event   , "event"            )

            #define X(a, b) a,
            enum prop { prop_list count };
            #undef X

            #define X(a, b) b,
            text description[prop::count] = { prop_list };
            #undef X
            #undef prop_list

            base& boss;
            subs tokens;
            cell alerts;
            cell stress;
            page status;
            ansi::esc coder;
            bool bypass = faux;

            struct
            {
                span render = span::zero();
                span output = span::zero();
                si32 frsize = 0;
                si64 totals = 0;
                si32 number = 0;    // info: Current frame number
                //bool   onhold = faux; // info: Indicator that the current frame has been successfully STDOUT
            }
            track; // debug: Textify the telemetry data for debugging purpose.

            void shadow()
            {
                for (auto i = 0; i < prop::count; i++)
                {
                    status[i].ease();
                }
            }

            debug_t(base& boss)
                : boss{ boss }
            { }

            operator bool () const { return tokens.count(); }

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
            void update(span const& watch, si32 delta)
            {
                track.output = watch;
                track.frsize = delta;
                track.totals+= delta;
            }
            void update(time const& timestamp)
            {
                track.render = datetime::now() - timestamp;
            }
            void output(face& canvas)
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
            }
            void stop()
            {
                track = {};
                tokens.clear();
            }
            void start()
            {
                //todo use skin
                stress = cell{}.fgc(whitelt);
                alerts = cell{}.fgc(rgba{ 0xFFd0d0FFu });

                status.style.wrp(wrap::on).jet(bias::left).rlf(feed::rev).mgl(4);
                status.current().locus.cup(dot_00).cnl(2);

                auto maxlen = 0_sz;
                for (auto& desc : description)
                {
                    maxlen = std::max(maxlen, desc.size());
                }
                auto attr = si32{ 0 };
                for (auto& desc : description)
                {
                    status += coder.add(" ", utf::adjust(desc, maxlen, " ", true), " ").idx(attr++).nop().nil().eol();
                    coder.clear();
                }

                boss.LISTEN(tier::general, e2::config::fps, fps, tokens)
                {
                    status[prop::frame_rate].set(stress) = std::to_string(fps);
                    boss.base::strike();
                };
                boss.SIGNAL(tier::general, e2::config::fps, e2::config::fps.param(-1));
                boss.LISTEN(tier::release, e2::conio::focus, f, tokens)
                {
                    update(f.state);
                    boss.base::strike();
                };
                boss.LISTEN(tier::release, e2::size::any, newsize, tokens)
                {
                    update(newsize);
                };
                boss.LISTEN(tier::release, e2::conio::mouse, m, tokens)
                {
                    if (bypass) return;
                    shadow();
                    status[prop::last_event].set(stress) = "mouse";
                    status[prop::mouse_pos ].set(stress) =
                        (m.coordxy.x < 10000 ? std::to_string(m.coordxy.x) : "-") + " : " +
                        (m.coordxy.y < 10000 ? std::to_string(m.coordxy.y) : "-") ;

                    auto m_buttons = std::bitset<8>(m.buttons);
                    for (auto i = 0; i < hids::numofbuttons; i++)
                    {
                        auto& state = status[prop::mouse_btn_1 + i].set(stress);
                        state = m_buttons[i] ? "pressed" : "idle   ";
                    }

                    status[prop::mouse_wheeldt].set(stress) = m.wheeldt ? std::to_string(m.wheeldt) :  " -- "s;
                    status[prop::mouse_hzwheel].set(stress) = m.hzwheel ? "active" : "idle  ";
                    status[prop::mouse_vtwheel].set(stress) = m.wheeled ? "active" : "idle  ";
                    status[prop::ctrl_state   ].set(stress) = "0x" + utf::to_hex(m.ctlstat);
                };
                boss.LISTEN(tier::release, e2::conio::keybd, k, tokens)
                {
                    shadow();
                    status[prop::last_event   ].set(stress) = "keybd";
                    status[prop::key_pressed  ].set(stress) = k.pressed ? "pressed" : "idle";
                    status[prop::ctrl_state   ].set(stress) = "0x" + utf::to_hex(k.ctlstat );
                    status[prop::key_code     ].set(stress) = "0x" + utf::to_hex(k.virtcod );
                    status[prop::key_scancode ].set(stress) = "0x" + utf::to_hex(k.scancod );

                    if (k.cluster.length())
                    {
                        auto t = k.cluster;
                        for (auto i = 0; i < 0x20; i++)
                        {
                            utf::change(t, text{ (char)i }, "^" + utf::to_utf_from_code(i + 0x40));
                        }
                        utf::change(t, text{ (char)0x7f }, "\\x7F");
                        utf::change(t, text{ (char)0x20 }, "\\x20");
                        status[prop::key_character].set(stress) = t;
                    }
                };
                boss.LISTEN(tier::release, e2::conio::error, e, tokens)
                {
                    shadow();
                    status[prop::last_event].set(stress) = "error";
                    throw;
                };
            }
        };

    public:
        pro::mouse mouse{*this }; // gate: Mouse controller.
        pro::robot robot{*this }; // gate: Animation controller.
        pro::title title{*this }; // gate: Window title/footer.
        pro::limit limit{*this }; // gate: Limit size to dot_11.

        pipe& canal;
        bool  yield; // gate: Indicator that the current frame has been successfully STDOUT'd.
        para  uname; // gate: Client name.
        text  uname_txt; // gate: Client name (original).
        props_t props; // gate: Application properties.
        input_t input; // gate: Input event handler.
        debug_t debug; // gate: Debug telemetry.
        sptr<base> applet; // gate: .
        diff  paint; // gate: Render.
        link  conio; // gate: Data IO.
        subs  tokens; // gate: Subscription tokens.
        bool direct; // gate: .
        bool local; // gate: .
        wptr<base> nexthop;
        hook oneoff_focus; // gate: .

        void draw_foreign_names(face& parent_canvas)
        {
            auto& header = *uname.lyric;
            auto  basexy = base::coor();
            auto  half_x = (si32)header.size().x / 2;
            for (auto& [id, gear_ptr] : input.gears)
            {
                auto& gear = *gear_ptr;
                if (gear.disabled) continue;
                auto coor = basexy;
                coor += gear.coord;
                coor.y--;
                coor.x -= half_x;
                //todo unify header coords
                header.move(coor);
                parent_canvas.fill(header, cell::shaders::fuse);
            }
        }
        void draw_mouse_pointer(face& canvas)
        {
            auto brush = cell{};
            auto coor = base::coor();
            auto area = rect{ coor, dot_11 };
            auto base = canvas.core::coor();
            for (auto& [id, gear_ptr] : input.gears)
            {
                auto& gear = *gear_ptr;
                if (gear.disabled) continue;
                area.coor = coor + gear.coord;
                area.coor -= base;
                if (gear.m.buttons) brush.txt(64 + gear.m.buttons).bgc(reddk).fgc(0xFFffffff);
                else                brush.txt("\u2588"/* █ */).bgc(0x00).fgc(0xFF00ff00);
                canvas.fill(area, cell::shaders::fuse(brush));
            }
        }
        void draw_clip_preview(face& canvas, time const& stamp)
        {
            for (auto& [id, gear_ptr] : input.gears)
            {
                auto& gear = *gear_ptr;
                gear.clip_printed = !gear.disabled &&
                                    (props.clip_preview_time == span::zero() ||
                                     props.clip_preview_time > stamp - gear.delta.stamp());
                if (gear.clip_printed)
                {
                    auto coor = gear.coord + dot_21 * 2;
                    auto full = gear.clip_preview.full();
                    gear.clip_preview.move(coor - full.coor);
                    canvas.plot(gear.clip_preview, cell::shaders::mix);
                }
            }
        }
        void draw_tooltips(face& canvas)
        {
            auto full = canvas.full();
            for (auto& [id, gear_ptr] : input.gears)
            {
                auto& gear = *gear_ptr;
                if (gear.disabled) continue;
                if (gear.tooltip_enabled())
                {
                    auto tooltip_data = gear.get_tooltip();
                    if (tooltip_data)
                    {
                        //todo optimize
                        auto tooltip_page = page{ tooltip_data };
                        auto area = full;
                        area.coor = std::max(dot_00, gear.coord - twod{ 4, tooltip_page.size() + 1 });
                        canvas.full(area);
                        canvas.cup(dot_00);
                        canvas.output(tooltip_page, cell::shaders::color(props.tooltip_colors));
                        canvas.full(full);
                    }
                }
            }
        }
        void send_tooltips(link& conio)
        {
            auto list = conio.tooltips.freeze();
            for (auto& [gear_id, gear_ptr] : input.gears /* use filter gear.is_tooltip_changed()*/)
            {
                auto& gear = *gear_ptr;
                if (gear.disabled) continue;
                if (gear.is_tooltip_changed())
                {
                    list.thing.push(gear_id, gear.get_tooltip());
                }
            }
            list.thing.sendby<true>(conio);
        }
        void check_tooltips(time now)
        {
            auto result = faux;
            for (auto& [gear_id, gear_ptr] : input.gears)
            {
                auto& gear = *gear_ptr;
                if (gear.disabled) continue;
                result |= gear.tooltip_check(now);
            }
            if (result) base::strike();
        }

        // gate: Attach a new item.
        auto attach(sptr<base>& item)
        {
            std::swap(applet, item);
            if (local) nexthop = applet;
            applet->SIGNAL(tier::release, e2::form::upon::vtree::attached, This());
        }
        // gate: .
        void _rebuild_scene(bool damaged)
        {
            auto stamp = datetime::now();
            auto& canvas = input.xmap;
            if (damaged)
            {
                if (props.legacy_mode & os::vt::mouse) // Render our mouse pointer.
                {
                    draw_mouse_pointer(canvas);
                }
                if (!direct && props.clip_preview_show)
                {
                    draw_clip_preview(canvas, stamp);
                }
                if (props.tooltip_enabled)
                {
                    if (direct) send_tooltips(conio);
                    else        draw_tooltips(canvas);
                }
                if (debug)
                {
                    debug.output(canvas);
                }
                if (props.show_regions)
                {
                    canvas.each([](cell& c)
                    {
                        auto mark = rgba{ rgba::color256[c.link() % 256] };
                        auto bgc = c.bgc();
                        mark.alpha(64);
                        bgc.mix(mark);
                        c.bgc(bgc);
                    });
                }
            }
            else
            {
                if (props.clip_preview_time != span::zero()) // Check clipboard preview timeout.
                {
                    for (auto& [id, gear_ptr] : input.gears)
                    {
                        auto& gear = *gear_ptr;
                        if (gear.clip_printed && props.clip_preview_time < stamp - gear.delta.stamp())
                        {
                            base::deface();
                            return;
                        }
                    }
                }
                if (yield) return;
            }

            // Note: We have to fire a mouse move event every frame,
            //       because in the global frame the mouse can stand still,
            //       but any form can move under the cursor, so for the form itself,
            //       the mouse cursor moves inside the form.
            if (debug)
            {
                debug.bypass = true;
                input.fire(hids::events::mouse::move.id);
                debug.bypass = faux;
                yield = paint.commit(canvas);
                if (yield)
                {
                    auto d = paint.status();
                    debug.update(d.watch, d.delta);
                }
                debug.update(stamp);
            }
            else
            {
                input.fire(hids::events::mouse::move.id);
                yield = paint.commit(canvas); // Try output my canvas to the my console.
            }
        }
        // gate: .
        virtual void rebuild_scene(base& world, bool damaged)
        {
            if (damaged)
            {
                auto& canvas = input.xmap;
                canvas.wipe(world.bell::id);
                canvas.render(applet, base::coor());
            }
            _rebuild_scene(damaged);
        }
        // gate: Main loop.
        void launch()
        {
            SIGNAL(tier::anycast, e2::form::upon::started, This());
            directvt::binary::stream::reading_loop(canal, [&](view data){ conio.sync(data); });
            SIGNAL(tier::release, e2::conio::quit, "exit from a stream reading loop");
        }

    protected:
        //todo revise
        gate(sptr<pipe> uplink, si32 vtmode, xmls& config, view userid = {}, si32 session_id = 0, bool isvtm = faux)
            : canal{ *uplink },
              props{ canal, userid, vtmode, isvtm, session_id, config },
              input{ props, *this },
             paint{ canal, props.vtmode },
             conio{ canal, *this  },
             debug{*this },
             direct{ props.vtmode == svga::dtvt },
             local{ true }
        {
            base::root(true);
            limit.set(dot_11);
            title.live = faux;

            LISTEN(tier::release, hids::events::focus::set, gear, oneoff_focus) // Restore all foci for the first user.
            {
                //if (auto target = local ? applet : base::parent())
                if (auto target = nexthop.lock())
                {
                    pro::focus::set(target, gear.id, pro::focus::solo::off, pro::focus::flip::off, true);
                }
                oneoff_focus.reset();
            };
            LISTEN(tier::preview, hids::events::keybd::data::post, gear, tokens) // Start of kb event propagation.
            {
                if (gear)
                //if (auto target = local ? applet : base::parent())
                if (auto target = nexthop.lock())
                {
                    target->SIGNAL(tier::preview, hids::events::keybd::data::post, gear);
                }
            };
            if (!direct)
            {
                LISTEN(tier::release, hids::events::focus::set, gear) // Conio focus tracking.
                {
                    //if (auto target = local ? applet : base::parent())
                    if (auto target = nexthop.lock())
                    {
                        target->SIGNAL(tier::release, hids::events::keybd::focus::bus::on, seed, ({ .id = gear.id }));
                    }
                };
                LISTEN(tier::release, hids::events::focus::off, gear)
                {
                    //if (auto target = local ? applet : base::parent())
                    if (auto target = nexthop.lock())
                    {
                        target->SIGNAL(tier::release, hids::events::keybd::focus::bus::off, seed, ({ .id = gear.id }));
                    }
                };
            }
            //todo deprecated
            //LISTEN(tier::release, hids::events::notify::focus::got, from_gear, tokens)
            //{
            //    auto myid = from_gear.id;
            //    auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(myid);
            //    if (!gear_ptr) return;
            //    auto& gear = *gear_ptr;
            //    gear.kb_offer_4(applet);
            //    pro::focus::set(applet, gear.id, pro::focus::solo::off, pro::focus::flip::on);
            //    if (gear.focus_changed()) gear.dismiss();
            //};
            ////todo revise: nobody signal it
            ////todo deprecated
            //LISTEN(tier::release, hids::events::notify::focus::lost, from_gear, tokens)
            //{
            //    auto myid = from_gear.id;
            //    auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(myid);
            //    if (gear_ptr)
            //    {
            //        auto& gear = *gear_ptr;
            //        gear.kb_offer_5(applet);
            //        pro::focus::set(applet, gear.id, pro::focus::solo::off, pro::focus::flip::off);
            //    }
            //};

            LISTEN(tier::release, hids::events::keybd::focus::bus::any, seed, tokens)
            {
                //todo use input::forward<focus>
                if (seed.id != id_t{}) // Translate only the real foreign gear id.
                {
                    auto gear_it = input.gears.find(seed.id);
                    if (gear_it == input.gears.end())
                    {
                        gear_it = input.gears.emplace(seed.id, bell::create<hids>(props, seed.id == 0, *this, input.xmap)).first;
                    }
                    auto& [_id, gear_ptr] = *gear_it;
                    seed.id = gear_ptr->id;
                }

                auto deed = this->bell::template protos<tier::release>();
                //if constexpr (debugmode) log("foci: ", text(seed.deep++ * 4, ' '), "foci: ---gate bus::any gear:", seed.id, " hub:", this->id);
                //if (auto target = local ? applet : base::parent())
                if (auto target = nexthop.lock())
                {
                    target->bell::template signal<tier::release>(deed, seed);
                }
                //if constexpr (debugmode) log("foci: ", text(--seed.deep * 4, ' '), "foci: ----------------gate");
            };
            LISTEN(tier::preview, hids::events::keybd::focus::cut, seed, tokens)
            {
                if (direct)
                {
                    auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(seed.id);
                    if (!gear_ptr) return;
                    conio.focus_cut.send(conio, ext_gear_id);
                }
                else
                {
                    //todo revise see preview::focus::set
                    ////if (auto target = local ? applet : base::parent())
                    if (auto target = base::parent())
                    //if (auto target = nexthop.lock())
                    {
                        target->SIGNAL(tier::release, hids::events::keybd::focus::bus::off, seed);
                    }
                }
            };
            LISTEN(tier::preview, hids::events::keybd::focus::set, seed, tokens)
            {
                if (direct)
                {
                    auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(seed.id);
                    if (!gear_ptr) return;
                    conio.focus_set.send(conio, ext_gear_id, seed.solo);
                }
                else
                {
                    if (seed.item)
                    {
                        seed.item->SIGNAL(tier::release, hids::events::keybd::focus::bus::on, seed);
                    }
                }
            };
            if (direct) // Forward unhandled events outside.
            {
                //todo deprecated
                //LISTEN(tier::preview, hids::events::notify::focus::any, from_gear, tokens)
                //{
                //    auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(from_gear.id);
                //    if (!gear_ptr) return;
                //    auto cause = this->bell::protos<tier::preview>();
                //    auto state = cause == hids::events::notify::focus::got.id;
                //    conio.focus.send(conio, ext_gear_id, state, from_gear.focus_combine, from_gear.focus_force_group);
                //};
                LISTEN(tier::release, hids::events::keybd::data::any, gear) // Return back unhandled keybd events.
                {
                    if (gear)
                    {
                        auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(gear.id);
                        if (gear_ptr)
                        {
                            conio.keybd_event.send(conio, ext_gear_id,
                                                          gear.ctlstate,
                                                          gear.winctrl,
                                                          gear.virtcod,
                                                          gear.scancod,
                                                          gear.pressed,
                                                          gear.imitate,
                                                          gear.cluster,
                                                          gear.winchar,
                                                          gear.handled);
                        }
                    }
                };
            }


            LISTEN(tier::release, e2::form::proceed::quit::any, initiator, tokens)
            {
                auto msg = ansi::add("gate: quit message from: ", initiator->id);
                canal.shut();
                this->SIGNAL(tier::general, e2::shutdown, msg);
            };
            LISTEN(tier::release, e2::form::prop::name, user_name, tokens)
            {
                uname = uname_txt = user_name;
            };
            LISTEN(tier::request, e2::form::prop::name, user_name, tokens)
            {
                user_name = uname_txt;
            };
            LISTEN(tier::request, e2::form::prop::viewport, viewport, tokens)
            {
                this->SIGNAL(tier::anycast, e2::form::prop::viewport, viewport);
                viewport.coor += base::coor();
            };
            //todo unify creation (delete simple create wo gear)
            LISTEN(tier::preview, e2::form::proceed::create, region, tokens)
            {
                region.coor += base::coor();
                this->RISEUP(tier::release, e2::form::proceed::create, region);
            };
            LISTEN(tier::release, e2::form::proceed::onbehalf, proc, tokens)
            {
                //todo hids
                //proc(input.gear);
            };
            LISTEN(tier::preview, hids::events::keybd::data::any, gear, tokens)
            {
                //todo unify
                if (gear.keystrokes == props.debug_toggle)
                {
                    debug ? debug.stop()
                          : debug.start();
                }
            };
            LISTEN(tier::preview, hids::events::mouse::button::click::leftright, gear, tokens)
            {
                if (gear.clear_clip_data())
                {
                    this->bell::template expire<tier::release>();
                    gear.dismiss();
                }
            };
            LISTEN(tier::release, e2::conio::winsz, newsize, tokens)
            {
                if (applet) applet->SIGNAL(tier::anycast, e2::form::upon::resize, newsize);
                auto delta = base::resize(newsize);
                if (delta && direct)
                if (auto world_ptr = base::parent())
                {
                    paint.cancel();
                    rebuild_scene(*world_ptr, true);
                }
            };
            LISTEN(tier::release, e2::size::any, newsize, tokens)
            {
                if (applet) applet->base::resize(newsize);
            };
            LISTEN(tier::release, e2::conio::pointer, pointer, tokens)
            {
                props.legacy_mode |= pointer ? os::vt::mouse : 0;
            };
            LISTEN(tier::release, e2::conio::clipdata, clipdata, tokens)
            {
                if (!direct)
                {
                    clipdata.size = base::size() / 2;
                    input.set_clip_data(clipdata);
                    base::deface();
                }
            };
            LISTEN(tier::release, e2::conio::error, errcode, tokens)
            {
                auto msg = ansi::bgc(reddk).fgc(whitelt).add("\n\rgate: Term error: ", errcode, "\r\n");
                log("gate: error byemsg: ", msg);
                canal.shut();
            };
            LISTEN(tier::release, e2::conio::quit, msg, tokens)
            {
                this->SIGNAL(tier::preview, e2::form::proceed::quit::one, this->This());
                log("gate: ", msg);
                canal.shut();
                paint.stop();
                mouse.reset(); // Reset active mouse clients to avoid hanging pointers.
                base::detach();
                tokens.reset();
            };
            LISTEN(tier::preview, e2::conio::quit, msg, tokens)
            {
                log("gate: ", msg);
                canal.shut();
            };
            LISTEN(tier::general, e2::conio::quit, msg, tokens)
            {
                log("gate: global shutdown: ", msg);
                canal.shut();
            };
            LISTEN(tier::anycast, e2::form::upon::started, item_ptr, tokens)
            {
                if (props.debug_overlay) debug.start();
                this->SIGNAL(tier::release, e2::form::prop::name, props.title);
                this->SIGNAL(tier::preview, e2::form::prop::ui::header, props.title);
            };
            LISTEN(tier::release, e2::form::prop::ui::footer, newfooter, tokens)
            {
                if (direct)
                {
                    auto window_id = 0;
                    conio.form_footer.send(canal, window_id, newfooter);
                }
            };
            LISTEN(tier::release, e2::form::prop::ui::header, newheader, tokens)
            {
                if (direct)
                {
                    auto window_id = 0;
                    conio.form_header.send(canal, window_id, newheader);
                }
                else
                {
                    auto temp = text{};
                    temp.reserve(newheader.length());
                    para{ newheader }.lyric->utf8(temp);
                    log("gate: title changed to '", temp, ansi::nil().add("'"));
                    conio.output(ansi::header(temp));
                }
            };
            LISTEN(tier::release, hids::events::clipbrd::set, from_gear, tokens)
            {
                auto myid = from_gear.id;
                auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(myid);
                if (!gear_ptr) return;
                auto& gear =*gear_ptr;
                auto& data = gear.clip_rawdata;
                if (direct) conio.set_clipboard.send(canal, ext_gear_id, data.size, data.utf8, data.kind);
                else        conio.output(ansi::clipbuf(                  data.size, data.utf8, data.kind));
            };
            LISTEN(tier::release, hids::events::clipbrd::get, from_gear, tokens)
            {
                if (!direct) return;
                auto myid = from_gear.id;
                auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(myid);
                if (gear_ptr && !conio.request_clip_data(ext_gear_id, gear_ptr->clip_rawdata))
                {
                    log("gate: timeout: no clipboard data reply");
                }
            };
            LISTEN(tier::preview, hids::events::mouse::button::tplclick::leftright, gear, tokens)
            {
                if (debug)
                {
                    props.show_regions = true;
                    debug.stop();
                }
                else
                {
                    if (props.show_regions) props.show_regions = faux;
                    else                    debug.start();
                }
                gear.dismiss();
            };
            if (props.tooltip_enabled)
            {
                LISTEN(tier::general, e2::timer::any, now, tokens)
                {
                    check_tooltips(now);
                };
            }
            if (direct) // Forward unhandled events outside.
            {
                LISTEN(tier::release, e2::form::layout::minimize, gear, tokens)
                {
                    auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(gear.id);
                    if (gear_ptr) conio.minimize.send(canal, ext_gear_id);
                };
                LISTEN(tier::release, hids::events::mouse::scroll::any, gear, tokens, (isvtm))
                {
                    auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(gear.id);
                    if (gear_ptr) conio.mouse_event.send(canal, ext_gear_id, gear.mouse::cause, gear.coord, gear.delta.get(), gear.take_button_state());
                    gear.dismiss();
                };
                LISTEN(tier::release, hids::events::mouse::button::any, gear, tokens, (isvtm))
                {
                    using button = hids::events::mouse::button;
                    auto forward = faux;
                    auto cause = gear.mouse::cause;
                    if (isvtm && (gear.index == hids::leftright // Reserved for dragging nested vtm.
                              ||  gear.index == hids::right)    // Reserved for creation inside nested vtm.
                     && events::subevent(cause, button::drag::any.id)) return;
                    if (events::subevent(cause, button::click     ::any.id)
                     || events::subevent(cause, button::dblclick  ::any.id)
                     || events::subevent(cause, button::tplclick  ::any.id)
                     || events::subevent(cause, button::drag::pull::any.id))
                    {
                        gear.setfree();
                        forward = true;
                    }
                    else if (events::subevent(cause, button::drag::start::any.id))
                    {
                        gear.capture(bell::id); // To avoid unhandled mouse pull processing.
                        forward = true;
                    }
                    else if (events::subevent(cause, button::drag::cancel::any.id)
                          || events::subevent(cause, button::drag::stop  ::any.id))
                    {
                        gear.setfree();
                    }
                    if (forward)
                    {
                        auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(gear.id);
                        if (gear_ptr) conio.mouse_event.send(canal, ext_gear_id, cause, gear.coord, gear.delta.get(), gear.take_button_state());
                        gear.dismiss();
                    }
                };
                LISTEN(tier::general, e2::conio::logs, utf8, tokens)
                {
                    conio.logs.send(canal, os::process::id.first, os::process::id.second, text{ utf8 });
                };
                LISTEN(tier::release, e2::config::fps, fps, tokens)
                {
                    if (fps > 0) this->SIGNAL(tier::general, e2::config::fps, fps);
                };
                LISTEN(tier::preview, e2::config::fps, fps, tokens)
                {
                    conio.fps.send(conio, fps);
                };
                LISTEN(tier::preview, hids::events::mouse::button::click::any, gear, tokens)
                {
                    conio.expose.send(conio);
                };
                LISTEN(tier::anycast, e2::form::layout::expose, item, tokens)
                {
                    conio.expose.send(conio);
                };
                LISTEN(tier::preview, e2::form::layout::swarp, warp, tokens)
                {
                    conio.warping.send(conio, 0, warp);
                };
                LISTEN(tier::release, e2::form::layout::fullscreen, gear, tokens)
                {
                    auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(gear.id);
                    if (gear_ptr) conio.fullscreen.send(conio, ext_gear_id);
                };
            }
        }
    };

    // console: World aether.
    class host
        : public base
    {
    protected:
        using tick = datetime::quartz<events::reactor<>, hint>;
        using list = std::vector<rect>;
        using gptr = sptr<gate>;

        //pro::keybd keybd{*this }; // host: Keyboard controller.
        pro::mouse mouse{*this }; // host: Mouse controller.
        pro::focus focus; // host: Focus controller.

        tick quartz; // host: Frame rate synchronizator.
        si32 maxfps; // host: Frame rate.
        list debris; // host: Wrecked regions.
        xmls config; // host: Running configuration.
        gptr client; // host: Standalone app.
        subs tokens; // host: Subscription tokens.

        std::vector<bool> user_numbering; // host: .

        virtual void nextframe(bool damaged)
        {
            if (client) client->rebuild_scene(*this, damaged);
        }

    public:
        host(sptr<pipe> server, xmls config, pro::focus::mode m = pro::focus::mode::hub)
            :  focus{*this, m, faux },
              quartz{ bell::router<tier::general>(), e2::timer::tick.id },
              config{ config }
        {
            using namespace std::chrono;
            auto& canal = *server;
            auto& g = skin::globals();
            g.brighter       = config.take("brighter"              , cell{});//120);
            g.kb_focus       = config.take("kb_focus"              , cell{});//60
            g.shadower       = config.take("shadower"              , cell{});//180);//60);//40);// 20);
            g.shadow         = config.take("shadow"                , cell{});//180);//5);
            g.selector       = config.take("selector"              , cell{});//48);
            g.highlight      = config.take("highlight"             , cell{});
            g.warning        = config.take("warning"               , cell{});
            g.danger         = config.take("danger"                , cell{});
            g.action         = config.take("action"                , cell{});
            g.label          = config.take("label"                 , cell{});
            g.inactive       = config.take("inactive"              , cell{});
            g.menu_white     = config.take("menu_white"            , cell{});
            g.menu_black     = config.take("menu_black"            , cell{});
            g.lucidity       = config.take("lucidity");
            g.tracking       = config.take("tracking"              , faux);
            g.bordersz       = config.take("bordersz"              , dot_11);
            g.spd            = config.take("timings/spd"           , 10  );
            g.pls            = config.take("timings/pls"           , 167 );
            g.spd_accel      = config.take("timings/spd_accel"     , 1   );
            g.spd_max        = config.take("timings/spd_max"       , 100 );
            g.ccl            = config.take("timings/ccl"           , 120 );
            g.ccl_accel      = config.take("timings/ccl_accel"     , 30  );
            g.ccl_max        = config.take("timings/ccl_max"       , 1   );
            g.switching      = config.take("timings/switching"     , 200 );
            g.deceleration   = config.take("timings/deceleration"  , span{ 2s    });
            g.blink_period   = config.take("timings/blink_period"  , span{ 400ms });
            g.menu_timeout   = config.take("timings/menu_timeout"  , span{ 250ms });
            g.active_timeout = config.take("timings/active_timeout", span{ 1s    });
            g.repeat_delay   = config.take("timings/repeat_delay"  , span{ 500ms });
            g.repeat_rate    = config.take("timings/repeat_rate"   , span{ 30ms  });
            g.fader_time     = config.take("timings/fader/duration", span{ 150ms });
            g.fader_fast     = config.take("timings/fader/fast"    , span{ 0ms   });
            g.max_value      = config.take("limits/window/size"    , twod{ 2000, 1000  });

            maxfps = config.take("fps");
            if (maxfps <= 0) maxfps = 60;

            LISTEN(tier::general, e2::timer::any, timestamp, tokens)
            {
                auto damaged = !debris.empty();
                debris.clear();
                nextframe(damaged);
            };
            LISTEN(tier::request, e2::config::creator, world_ptr, tokens)
            {
                world_ptr = base::This();
            };
            LISTEN(tier::general, e2::config::fps, fps, tokens)
            {
                if (fps > 0)
                {
                    maxfps = fps;
                    quartz.ignite(maxfps);
                }
                else if (fps == -1)
                {
                    fps = maxfps;
                }
                else
                {
                    quartz.cancel();
                }
            };
            LISTEN(tier::general, e2::cleanup, counter, tokens)
            {
                this->template router<tier::general>().cleanup(counter.ref_count, counter.del_count);
            };
            LISTEN(tier::general, hids::events::halt, gear, tokens)
            {
                if (gear.captured(bell::id))
                {
                    gear.setfree();
                    gear.dismiss();
                }
            };
            LISTEN(tier::general, e2::shutdown, msg, tokens)
            {
                log("host: shutdown: ", msg);
                canal.stop();
            };
            LISTEN(tier::general, hids::events::device::user::login, props, tokens)
            {
                props = 0;
                while (props < user_numbering.size() && user_numbering[props]) { props++; }
                if (props == user_numbering.size()) user_numbering.push_back(true);
                else                                user_numbering[props] = true;
            };
            LISTEN(tier::general, hids::events::device::user::logout, props, tokens)
            {
                if (props < user_numbering.size()) user_numbering[props] = faux;
                else log(ansi::err("hall: user accounting error: ring size:", user_numbering.size(), " user_number:", props));
            };

            quartz.ignite(maxfps);
            log("host: started at ", maxfps, "fps");
        }
        // host: Mark dirty region.
        void denote(rect const& updateregion)
        {
            if (updateregion)
            {
                debris.push_back(updateregion);
            }
        }
        void deface(rect const& region) override
        {
            base::deface(region);
            denote(region);
        }
        // host: Create a new root of the specified subtype and attach it.
        auto invite(sptr<pipe> uplink, sptr<base>& applet, si32 vtmode)
        {
            {
                auto lock = events::sync{};
                client = base::create<gate>(uplink, vtmode, host::config);
                client->SIGNAL(tier::release, e2::form::upon::vtree::attached, base::This());
                client->attach(applet);
            }
            client->launch();
        }
        // host: Shutdown.
        void shutdown()
        {
            auto lock = events::sync{};
            client.reset();
            mouse.reset();
            tokens.reset();
        }
    };
}