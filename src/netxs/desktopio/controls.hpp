// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#include "input.hpp"

namespace netxs::ui
{
    // controls: UI extensions.
    namespace pro
    {
        // pro: Base class for extension/plugin.
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
                        log(prompt::sock, "Access to unregistered input device, ", gear.id);
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

        // pro: Resizing by dragging support.
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
                auto corner(twod length)
                {
                    return dtcoor.less(dot_11, length, dot_00);
                }
                auto grab(base const& master, twod curpos, dent outer)
                {
                    if (inside)
                    {
                        origin = curpos - corner(master.base::size() + outer);
                        seized = true;
                    }
                    return seized;
                }
                auto calc(base const& master, twod curpos, dent outer, dent inner, dent border)
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
                        widths = sector.less(dot_00, twod{-border.r,-border.b },
                                                     twod{ border.l, border.t });
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
                auto drag(base& master, twod curpos, dent outer, bool zoom)
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
            void props(dent outer_rect = {2,2,1,1}, dent inner_rect = {})
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
            sizer(base& boss, dent outer_rect = {2,2,1,1}, dent inner_rect = {})
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
                        auto step = boss.base::extend(next);
                        if (!step.size) // Undo if can't zoom.
                        {
                            g.zoomdt = prev;
                            boss.base::moveto(coor);
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
                boss.LISTEN(tier::preview, e2::form::layout::swarp, warp, memo)
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

        // pro: Moving by dragging support.
        class mover
            : public skill
        {
            struct sock
            {
                twod origin; // sock: Grab's initial coord info.
                void grab(base const& master, twod curpos)
                {
                    auto center = master.base::size() / 2;
                    origin = curpos - center;
                }
                void drag(base& master, twod coord)
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

            list items;
            wptr dest_shadow;
            sptr dest_object;

        public:
            mover(base&&) = delete;
            mover(base& boss, sptr subject)
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

        // pro: Runtime animation support (time-based).
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

        // pro: Scheduler (timeout based).
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
                boss.LISTEN(tier::release, e2::form::state::keybd::focus::count, count, conf)
                {
                    down = !count;
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
            void blink_period(span new_step = skin::globals().blink_period)
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
                        log(prompt::term, "Unsupported cursor style requested, ", mode);
                        break;
                }
            }
            void toggle()
            {
                style(!form);
                reset();
            }
            // pro::caret: Set caret position.
            void coor(twod coor)
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
                            auto view = canvas.core::view();
                            if (auto area = view.clip(body))
                            {
                                auto& test = canvas.peek(body.coor);
                                if (test.wdt() == 2) // Extend cursor to adjacent halves.
                                {
                                    if (view.hittest(body.coor + dot_10))
                                    {
                                        auto& next = canvas.peek(body.coor + dot_10);
                                        if (next.wdt() == 3 && test.same_txt(next))
                                        {
                                            area.size.x++;
                                        }
                                    }
                                }
                                else if (test.wdt() == 3)
                                {
                                    if (view.hittest(body.coor - dot_10))
                                    {
                                        auto& prev = canvas.peek(body.coor - dot_10);
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

        // pro: Title/footer support.
        class title
            : public skill
        {
            using skill::boss,
                  skill::memo;

        public:
            page head_page; // title: Owner's caption header.
            page foot_page; // title: Owner's caption footer.
            escx head_foci; // title: Original header + foci status.
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
            void recalc(twod new_size)
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
                boss.LISTEN(tier::release, e2::area, new_area, memo)
                {
                    if (boss.base::size() != new_area.size)
                    {
                        recalc(new_area.size);
                    }
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
                        auto color = rgba::vt256[4 + index % (256 - 4)];
                        auto image = ansi::fgc(color).add("\0▀"sv);
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
                boss.LISTEN(tier::general, e2::timer::any, timestamp, memo)
                {
                    if (wait && (timestamp > stop))
                    {
                        wait = faux;
                        auto shadow = boss.This();
                        log(prompt::gate, "Shutdown by double escape");
                        boss.SIGNAL(tier::preview, e2::conio::quit, deal, ());
                        memo.clear();
                    }
                };
            }
        };

        // pro: Close owner on mouse inactivity timeout.
        class watch
            : public skill
        {
            using skill::boss,
                  skill::memo;

            static constexpr auto limit = 600s; //todo unify // watch: Idle timeout in seconds.

            hook pong; // watch: Alibi subsciption token.
            hook ping; // watch: Zombie check countdown token.
            time stop; // watch: Timeout for zombies.

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
                        auto backup = boss.This();
                        log(prompt::gate, "No mouse clicking events");
                        boss.RISEUP(tier::release, e2::form::proceed::quit::one, true);
                        ping.reset();
                        memo.clear();
                    }
                };
            }
        };

        // pro: Keyboard focus support.
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
                std::list<wptr> next; // focus: Focus event next hop.

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

            void signal_state()
            {
                auto count = 0;
                for (auto& [gear_id, route] : gears)
                {
                    if (gear_id != id_t{} && route.active) ++count;
                }
                boss.SIGNAL(tier::release, e2::form::state::keybd::focus::count, count);
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
                            //if constexpr (debugmode) log(prompt::foci, "Gears cleanup boss:", boss.id, " hid:", gear.id);
                            auto& route = iter->second;
                            auto  token = std::move(route.token);
                            if (route.active) // Keep only the active branch.
                            {
                                route.active = faux;
                                gears[id_t{}] = std::move(route);
                                boss.SIGNAL(tier::release, e2::form::state::keybd::focus::off, gear.id);
                                signal_state();
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
            static void set(sptr item_ptr, T&& gear_id, solo s, flip f, bool skip = faux)
            {
                auto fire = [&](auto id)
                {
                    item_ptr->RISEUP(tier::preview, hids::events::keybd::focus::set, seed, ({ .id = id, .solo = (si32)s, .flip = (bool)f, .skip = skip }));
                    //if constexpr (debugmode) log(prompt::foci, "Focus set gear:", seed.id, " item:", item_ptr->id);
                };
                if constexpr (std::is_same_v<id_t, std::decay_t<T>>) fire(gear_id);
                else                    for (auto next_id : gear_id) fire(next_id);
            }
            template<class T>
            static void off(sptr item_ptr, T&& gear_id)
            {
                auto fire = [&](auto id)
                {
                    item_ptr->RISEUP(tier::preview, hids::events::keybd::focus::off, seed, ({ .id = id }));
                    //if constexpr (debugmode) log(prompt::foci, "Focus off gear:", seed.id, " item:", item_ptr->id);
                };
                if constexpr (std::is_same_v<id_t, std::decay_t<T>>) fire(gear_id);
                else                    for (auto next_id : gear_id) fire(next_id);
            }
            static void off(sptr item_ptr)
            {
                item_ptr->RISEUP(tier::request, e2::form::state::keybd::enlist, gear_id_list, ());
                pro::focus::off(item_ptr, gear_id_list);
                //if constexpr (debugmode) log(prompt::foci, "Full defocus item:", item_ptr->id);
            }
            static auto get(sptr item_ptr, bool remove_default = faux)
            {
                item_ptr->RISEUP(tier::request, e2::form::state::keybd::enlist, gear_id_list, ());
                for (auto next_id : gear_id_list)
                {
                    item_ptr->RISEUP(tier::preview, hids::events::keybd::focus::get, seed, ({ .id = next_id }));
                    //if constexpr (debugmode) log(prompt::foci, "Focus get gear:", seed.id, " item:", item_ptr->id);
                }
                if (remove_default)
                if (auto parent = item_ptr->parent())
                {
                    parent->RISEUP(tier::preview, hids::events::keybd::focus::dry, seed, ({ .item = item_ptr }));
                }
                return gear_id_list;
            }
            static auto test(base& item, input::hids& gear)
            {
                item.RISEUP(tier::request, e2::form::state::keybd::find, gear_test, (gear.id, 0));
                return gear_test.second;
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
                    //if constexpr (debugmode) log(prompt::foci, "data::post gear:", gear.id, " hub:", boss.id, " gears.size:", gears.size());
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
                    //if constexpr (debugmode) log(prompt::foci, text(seed.deep++ * 4, ' '), "---bus::any gear:", seed.id, " hub:", boss.id);
                    route.foreach([&](auto& nexthop){ nexthop->bell::template signal<tier::release>(deed, seed); });
                    //if constexpr (debugmode) log(prompt::foci, text(--seed.deep * 4, ' '), "----------------");
                };
                boss.LISTEN(tier::release, hids::events::keybd::focus::bus::on, seed, memo)
                {
                    //if constexpr (debugmode) log(prompt::foci, text(seed.deep * 4, ' '), "bus::on gear:", seed.id, " hub:", boss.id, " gears.size:", gears.size());
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
                        signal_state();
                    }
                    //if constexpr (debugmode) log(prompt::foci, text(seed.deep * 4, ' '), "bus::off gear:", seed.id, " hub:", boss.id);
                };
                boss.LISTEN(tier::release, hids::events::keybd::focus::bus::copy, seed, memo) // Copy default focus route if it is and activate it.
                {
                    //if constexpr (debugmode) log(prompt::foci, text(seed.deep * 4, ' '), "bus::copy gear:", seed.id, " hub:", boss.id);
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
                boss.LISTEN(tier::request, e2::form::state::keybd::focus::count, count, memo)
                {
                    //todo revise: same as e2::form::state::keybd::check
                    count = 0;
                    for (auto& [gear_id, route] : gears)
                    {
                        if (gear_id != id_t{} && route.active) ++count;
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
        // pro: Mouse support.
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

            sptr soul; // mouse: Boss cannot be removed while it has active gears.
            si32 rent; // mouse: Active gears count.
            si32 full; // mouse: All gears count. Counting to keep the entire chain of links in the visual tree.
            bool omni; // mouse: Ability to accept all hover events (true) or only directly over the object (faux).
            si32 drag; // mouse: Bitfield of buttons subscribed to mouse drag.
            list mice; // mouse: List of active mice.
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
                        boss.SIGNAL(tier::release, e2::form::state::hover, rent);
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
                        boss.SIGNAL(tier::release, e2::form::state::hover, rent);
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
                boss.LISTEN(tier::request, e2::form::state::hover, state, memo)
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
            fader(base& boss, cell default_state, cell highlighted_state, span fade_out = 250ms, sptr tracking_object = {})
                : skill{ boss },
                robo{ boss },
                fade{ fade_out },
                c1 { default_state },
                c2 { highlighted_state },
                c2_orig { highlighted_state },
                transit{ 0 }
            {
                boss.base::color(c1.fgc(), c1.bgc());
                boss.LISTEN(tier::release, e2::form::prop::filler, filler)
                {
                    if (!fake)
                    {
                        auto& fgc = filler.fgc();
                        auto& bgc = filler.bgc();
                        c1.fgc(fgc);
                        c1.bgc(bgc);
                        if (filler.fga()) c2.fgc(fgc);
                        else              c2.fgc(c2_orig.fgc());
                        if (filler.bga()) c2.bgc(bgc);
                        else              c2.bgc(c2_orig.bgc());
                        work(transit);
                    }
                };
                auto& root = tracking_object ? *tracking_object : boss;
                root.LISTEN(tier::release, e2::form::state::mouse, active, memo)
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

        // pro: UI-control cache.
        class cache
            : public skill
        {
            using skill::boss,
                  skill::memo;

            netxs::sptr<face> coreface; //todo revise necessity
            byte              lucidity; // cacheL .
            bool              usecache; // cacheL .
            face&             bosscopy; // cache: Boss bitmap cache.

        public:
            cache(base&&) = delete;
            cache(base& boss, bool rendered = true)
                : skill{ boss },
                  bosscopy{*(coreface = ptr::shared<face>())},
                  lucidity{ 0xFF },
                  usecache{ true }
            {
                bosscopy.link(boss.bell::id);
                bosscopy.size(boss.base::size());
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
                    }
                };
                boss.LISTEN(tier::release, e2::area, new_area, memo)
                {
                    if (bosscopy.size() != new_area.size)
                    {
                        bosscopy.size(new_area.size);
                    }
                };
                boss.LISTEN(tier::request, e2::form::canvas, canvas_ptr, memo)
                {
                    canvas_ptr = coreface;
                };
                if (rendered)
                {
                    boss.LISTEN(tier::release, e2::render::prerender, parent_canvas, memo)
                    {
                        if (!usecache) return;
                        if (boss.base::ruined())
                        {
                            bosscopy.wipe();
                            boss.base::ruined(faux);
                            boss.SIGNAL(tier::release, e2::render::any, bosscopy);
                        }
                        auto full = parent_canvas.full();
                        bosscopy.move(full.coor);
                        if (lucidity == 0xFF) parent_canvas.fill(bosscopy, cell::shaders::fusefull);
                        else                  parent_canvas.fill(bosscopy, cell::shaders::transparent(lucidity));
                        bosscopy.move(dot_00);
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
                    if (!alive || boss.base::filler.bga() == 0xFF) return;
                    parent_canvas.blur(width, [&](cell& c) { c.alpha(0xFF); });
                };
            }
        };

        // pro: Background highlighter.
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

        // pro: Drag&roll support.
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

        // pro: Tooltip support.
        class notes
            : public skill
        {
            using skill::boss,
                  skill::memo;

            text note;

        public:
            notes(base&&) = delete;
            notes(base& boss, view data, dent wrap = { si32max })
                : skill{ boss },
                  note { data }
            {
                boss.LISTEN(tier::release, hids::events::notify::mouse::enter, gear, memo, (wrap, full = wrap.l == si32max))
                {
                    if (gear.tooltip_set) return; // Prevent parents from setting tooltip.
                    if (full || !(boss.area() + wrap).hittest(gear.coord + boss.coor()))
                    {
                        gear.set_tooltip(note);
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

    // controls: base UI element.
    template<class T>
    class form
        : public base
    {
        std::map<std::type_index, uptr<pro::skill>> depo;
        std::map<id_t, subs> memomap; // form: Token set for dependent subscriptions.

    public:
        pro::mouse mouse{ *this }; // form: Mouse controller.
        //pro::keybd keybd{ *this }; // form: Keybd controller.

    protected:
        form(size_t nested_count = 0)
            : base{ nested_count }
        { }

    public:
        auto This() { return base::This<T>(); }
        template<class ...Args>
        static auto ctor(Args&&... args)
        {
            auto item = base::create<T>(std::forward<Args>(args)...);
            return item;
        }
        // form: Attach feature and return itself.
        template<class S, class ...Args>
        auto plugin(Args&&... args)
        {
            auto backup = This();
            depo[std::type_index(typeid(S))] = std::make_unique<S>(*backup, std::forward<Args>(args)...);
            return backup;
        }
        // form: Detach feature and return itself.
        template<class S>
        auto unplug()
        {
            auto backup = This();
            depo.erase(std::type_index(typeid(S)));
            return backup;
        }
        // form: Fill object region using parametrized fx.
        template<auto RenderOrder = e2::render::prerender, tier Tier = tier::release, class Fx, class Event = noop, bool fixed = std::is_same_v<Event, noop>>
        auto shader(Fx&& fx, Event sync = {}, sptr source_ptr = {})
        {
            if constexpr (fixed)
            {
                LISTEN(tier::release, RenderOrder, parent_canvas, -, (fx))
                {
                    parent_canvas.fill(fx);
                };
            }
            else
            {
                auto param_ptr = ptr::shared(Event::param());
                auto& param = *param_ptr;
                auto& source = source_ptr ? *source_ptr : *this;
                source.SIGNAL(tier::request, sync, param);
                source.LISTEN(Tier, sync, new_value, bell::tracker, (param_ptr))
                {
                    param = new_value;
                    base::deface();
                };
                LISTEN(tier::release, RenderOrder, parent_canvas, -, (fx))
                {
                    if (param) parent_canvas.fill(fx[param]);
                };
            }
            return This();
        }
        // form: deprecated in favor of pro::brush. Set colors and return itself.
        template<class ...Args>
        auto colors(Args&&... args)
        {
            base::color(std::forward<Args>(args)...);
            return This();
        }
        // form: Set control as root.
        auto isroot(bool master, si32 family = base::client)
        {
            base::root(master);
            base::kind(family);
            return This();
        }
        // form: Set the form visible for mouse.
        auto active(cell brush)
        {
            base::color(brush.txt(whitespace).link(bell::id));
            return This();
        }
        auto active()
        {
            return active(base::color());
        }
        // form: Return plugin reference of specified type. Add the specified plugin (using specified args) if it is missing.
        template<class S, class ...Args>
        auto& plugins(Args&&... args)
        {
            const auto it = depo.find(std::type_index(typeid(S)));
            if (it == depo.end())
            {
                plugin<S>(std::forward<Args>(args)...);
            }
            auto ptr = static_cast<S*>(depo[std::type_index(typeid(S))].get());
            return *ptr;
        }
        // form: Invoke arbitrary functor(itself/*This/boss) in place.
        template<class P>
        auto invoke(P functor)
        {
            auto backup = This();
            functor(*backup);
            return backup;
        }
        // form: Attach homeless branch and return itself.
        template<class ...Args>
        auto branch(Args&&... args)
        {
            auto backup = This();
            backup->T::attach(std::forward<Args>(args)...);
            return backup;
        }
        // form: UI-control will be detached upon destruction of the master.
        auto depend(sptr master_ptr)
        {
            auto& master = *master_ptr;
            master.LISTEN(tier::release, e2::dtor, id, memomap[master.id])
            {
                auto backup = This();
                memomap.erase(master.id);
                if (memomap.empty()) base::detach();
            };
            return This();
        }
        // form: UI-control will be detached when the last item of collection is detached.
        template<class S>
        auto depend_on_collection(S data_collection_src) //todo too heavy, don't use
        {
            auto backup = This();
            for (auto& data_src : data_collection_src)
            {
                depend(data_src);
            }
            return backup;
        }
        // form: Create and attach a new item using a template and dynamic datasource.
        template<class Property, class Sptr, class P>
        auto attach_element(Property, Sptr data_src_sptr, P item_template)
        {
            auto arg_value = typename Property::type{};

            auto backup = This();
            data_src_sptr->SIGNAL(tier::request, Property{}, arg_value);
            auto new_item = item_template(data_src_sptr, arg_value)
                                 ->depend(data_src_sptr);
            auto item_shadow = ptr::shadow(new_item);
            auto data_shadow = ptr::shadow(data_src_sptr);
            auto boss_shadow = ptr::shadow(backup);
            data_src_sptr->LISTEN(tier::release, Property{}, arg_new_value, memomap[data_src_sptr->id], (boss_shadow, data_shadow, item_shadow, item_template))
            {
                if (auto boss_ptr = boss_shadow.lock())
                if (auto data_src = data_shadow.lock())
                if (auto old_item = item_shadow.lock())
                {
                    auto new_item = item_template(data_src, arg_new_value)
                                         ->depend(data_src);
                    item_shadow = ptr::shadow(new_item); // Update current item shadow.
                    boss_ptr->update(old_item, new_item);
                }
            };
            branch(new_item);
            return new_item;
        }
        // form: Create and attach a new item using a template and dynamic datasource.
        template<class Property, class S, class P, class F = noop>
        auto attach_collection(Property, S& data_collection_src, P item_template, F proc = {})
        {
            auto backup = This();
            for (auto& data_src_sptr : data_collection_src)
            {
                auto item = attach_element(Property{}, data_src_sptr, item_template);
                proc(data_src_sptr);
            }
            return backup;
        }
        template<class BackendProp, class P>
        void publish_property(BackendProp, P setter)
        {
            LISTEN(tier::request, BackendProp{}, property_value, -, (setter))
            {
                setter(property_value);
            };
        }
        template<class BackendProp, class FrontendProp>
        auto attach_property(BackendProp, FrontendProp)
        {
            auto property_value = typename BackendProp::type{};

            auto backup = This();
            SIGNAL(tier::request, BackendProp{},  property_value);
            SIGNAL(tier::anycast, FrontendProp{}, property_value);

            LISTEN(tier::release, BackendProp{}, property_value)
            {
                this->SIGNAL(tier::anycast, FrontendProp{}, property_value);
            };
            return backup;
        }
        auto limits(twod min_sz = -dot_11, twod max_sz = -dot_11)
        {
            base::limits(min_sz, max_sz);
            return This();
        }
        auto alignment(bind atgrow, bind atcrop = { snap::none, snap::none })
        {
            base::alignment(atgrow, atcrop);
            return This();
        }
        auto setpad(dent intpad, dent extpad = {})
        {
            base::setpad(intpad, extpad);
            return This();
        }
    };

    // controls: Splitter.
    class fork
        : public form<fork>
    {
        sptr& object_1; // fork: 1st object.
        sptr& object_2; // fork: 2nd object.
        sptr& splitter; // fork: Resizing grip object.
        rect  griparea; // fork: Resizing grip region.
        axis  rotation; // fork: Fork orientation.
        si32  fraction; // fork: Ratio between objects.
        bool  adaptive; // fork: Fixed ratio.

        auto xpose(twod p)
        {
            return rotation == axis::X ? p : twod{ p.y, p.x };
        }
        void _config(axis orientation, si32 grip_width, si32 s1 = 1, si32 s2 = 1)
        {
            rotation = orientation;
            griparea.size = xpose({ std::max(0, grip_width), 0 });
            _config_ratio(s1, s2);
        }
        void _config_ratio(si32 s1, si32 s2)
        {
            if (s1 < 0) s1 = 0;
            if (s2 < 0) s2 = 0;
            auto sum = s1 + s2;
            fraction = sum ? netxs::divround(s1 * max_ratio, sum)
                           : max_ratio >> 1;
        }

    protected:
        fork(axis orientation = axis::X, si32 grip_width = 0, si32 s1 = 1, si32 s2 = 1)
            : form{ 3 },
              object_1{ base::subset[0] },
              object_2{ base::subset[1] },
              splitter{ base::subset[2] },
              rotation{ },
              fraction{ },
              adaptive{ }
        {
            _config(orientation, grip_width, s1, s2);
            LISTEN(tier::preview, e2::form::layout::swarp, warp)
            {
                adaptive = true; // Adjust the grip ratio on coming resize.
                this->bell::expire<tier::preview>(true);
            };
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                if (splitter) splitter->render(parent_canvas);
                if (object_1) object_1->render(parent_canvas);
                if (object_2) object_2->render(parent_canvas);
            };
        }
        // fork: .
        void deform(rect& new_area) override
        {
            auto region_1 = rect{};
            auto region_2 = rect{};
            auto region_3 = griparea;
            auto meter = [&](auto& newsz_x, auto& newsz_y,
                             auto& size1_x, auto& size1_y,
                             auto& coor2_x, auto& coor2_y,
                             auto& size2_x, auto& size2_y,
                             auto& coor3_x, auto& coor3_y,
                             auto& size3_x, auto& size3_y)
            {
                auto limit_x = std::max(newsz_x - size3_x, 0);
                auto split_x = netxs::divround(limit_x * fraction, max_ratio);
                auto test = [&]
                {
                    size1_x = split_x;
                    size1_y = newsz_y;
                    if (object_1)
                    {
                        object_1->base::recalc(region_1);
                        split_x = size1_x;
                        newsz_y = size1_y;
                    }
                    size2_x = limit_x - split_x;
                    size2_y = newsz_y;
                    coor2_x = split_x + size3_x;
                    coor2_y = 0;
                    auto test_size2 = region_2.size;
                    if (object_2)
                    {
                        object_2->base::recalc(region_2);
                        newsz_y = size2_y;
                    }
                    return test_size2 == region_2.size;
                };
                auto ok = test();
                split_x = newsz_x - size3_x - size2_x;
                if (!ok) test(); // Repeat if object_2 doesn't fit.
                coor3_x = split_x;
                coor3_y = 0;
                size3_y = newsz_y;
                if (adaptive) _config_ratio(split_x, size2_x);
                newsz_x = split_x + size3_x + size2_x;
            };
            auto& new_size = new_area.size;
            rotation == axis::X ? meter(new_size.x, new_size.y, region_1.size.x, region_1.size.y, region_2.coor.x, region_2.coor.y, region_2.size.x, region_2.size.y, region_3.coor.x, region_3.coor.y, region_3.size.x, region_3.size.y)
                                : meter(new_size.y, new_size.x, region_1.size.y, region_1.size.x, region_2.coor.y, region_2.coor.x, region_2.size.y, region_2.size.x, region_3.coor.y, region_3.coor.x, region_3.size.y, region_3.size.x);
            if (splitter) splitter->base::recalc(region_3);
            griparea = region_3;
        }
        // fork: .
        void inform(rect new_area) override
        {
            auto corner_2 = twod{ griparea.coor.x + griparea.size.x, 0 };
            auto region_1 = rect{ dot_00, xpose({ griparea.coor.x, griparea.size.y })};
            auto region_2 = rect{ xpose(corner_2), xpose({ new_area.size.x - corner_2.x, griparea.size.y })};
            auto region_3 = griparea;
            region_1.coor += new_area.coor;
            region_2.coor += new_area.coor;
            region_3.coor += new_area.coor;
            if (object_1) object_1->base::notify(region_1);
            if (object_2) object_2->base::notify(region_2);
            if (splitter) splitter->base::notify(region_3);
            adaptive = faux;
        }

    public:
        static constexpr auto min_ratio = si32{ 0           };
        static constexpr auto max_ratio = si32{ 0xFFFF      };
        static constexpr auto mid_ratio = si32{ 0xFFFF >> 1 };

        // fork: .
        auto get_ratio()
        {
            return fraction;
        }
        // fork: .
        auto set_ratio(si32 new_ratio = max_ratio)
        {
            fraction = new_ratio;
        }
        // fork: .
        void config(si32 s1, si32 s2 = 1)
        {
            _config_ratio(s1, s2);
            base::reflow();
        }
        // fork: .
        auto config(axis orientation, si32 grip_width, si32 s1, si32 s2)
        {
            _config(orientation, grip_width, s1, s2);
            return This();
        }
        // fork: .
        void rotate()
        {
            auto width = xpose(griparea.size).x;
            rotation = (axis)!rotation;
                 if (rotation == axis::Y && width == 2) width = 1;
            else if (rotation == axis::X && width == 1) width = 2;
            (rotation == axis::X ? griparea.size.x : griparea.size.y) = width;
            base::reflow();
        }
        // fork: .
        void swap()
        {
            std::swap(object_1, object_2);
            base::reflow();
        }
        // fork: .
        void move_slider(si32 step)
        {
            if (splitter)
            {
                auto delta = griparea.size * xpose({ step, 0 });
                splitter->SIGNAL(tier::preview, e2::form::upon::changed, delta);
            }
        }
        // fork: .
        template<class T>
        auto attach(slot Slot, T item_ptr)
        {
            if (Slot == slot::_1)
            {
                if (object_1) remove(object_1);
                object_1 = item_ptr;
            }
            else if (Slot == slot::_2)
            {
                if (object_2) remove(object_2);
                object_2 = item_ptr;
            }
            else
            {
                if (splitter) remove(splitter);
                splitter = item_ptr;
                splitter->LISTEN(tier::preview, e2::form::upon::changed, delta)
                {
                    auto split = xpose(griparea.coor + delta).x;
                    auto limit = xpose(base::size() - griparea.size).x;
                    fraction = netxs::divround(max_ratio * split, limit);
                    this->base::reflow();
                };
            }
            item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::attached, This());
            return item_ptr;
        }
        // fork: Remove nested object by it's ptr.
        void remove(sptr item_ptr) override
        {
            if (object_1 == item_ptr ? ((void)object_1.reset(), true) :
                object_2 == item_ptr ? ((void)object_2.reset(), true) :
                splitter == item_ptr ? ((void)splitter.reset(), true) : faux)
            {
                auto backup = This();
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, backup);
            }
        }
    };

    // controls: Vertical/horizontal list.
    class list
        : public form<list>
    {
        bool updown; // list: List orientation, true: vertical(default), faux: horizontal.
        sort lineup; // list: Attachment order.

    protected:
        list(axis orientation = axis::Y, sort attach_order = sort::forward)
            : updown{ orientation == axis::Y },
              lineup{ attach_order }
        {
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                auto basis = parent_canvas.full();
                auto frame = parent_canvas.view();
                auto min_y = frame.coor[updown] - basis.coor[updown];
                auto max_y = frame.size[updown] + min_y;
                auto bound = [xy = updown](auto& o){ return o ? o->base::region.coor[xy] + o->base::region.size[xy] : -dot_mx.y; };
                auto start = std::ranges::lower_bound(base::subset, min_y, {}, bound);
                while (start != base::subset.end())
                {
                    if (auto& object = *start++)
                    {
                        object->render(parent_canvas);
                        if (!object->base::hidden && bound(object) >= max_y) break;
                    }
                }
            };
        }
        // list: .
        void deform(rect& new_area) override
        {
            auto& object_area = new_area;
            auto& new_size = object_area.size;
            auto& height = object_area.coor[updown];
            auto& y_size = new_size[updown];
            auto& x_size = new_size[1 - updown];
            auto  x_temp = x_size;
            auto  y_temp = y_size;
            auto start = height;
            auto meter = [&]
            {
                height = start;
                for (auto& object : subset)
                {
                    if (!object || object->base::hidden) continue;
                    auto& entry = *object;
                    y_size = 0;
                    entry.base::recalc(object_area);
                    if (x_size > x_temp) x_temp = x_size;
                    else                 x_size = x_temp;
                    height += entry.base::socket.size[updown];
                }
            };
            meter(); if (subset.size() > 1 && x_temp != x_size) meter();
            y_size = height;
        }
        // list: .
        void inform(rect new_area) override
        {
            auto object_area = new_area;
            auto& size_y = object_area.size[updown];
            auto& coor_y = object_area.coor[updown];
            auto& lock_y = base::anchor[updown];
            auto found = faux;
            for (auto& object : subset)
            {
                if (!object || object->base::hidden) continue;
                auto& entry = *object;
                if (!found) // Looking for anchored list entry.
                {
                    auto anker = entry.base::area() + entry.base::extpad; // Use old entry position.
                    auto anker_coor_y = anker.coor[updown];
                    auto anker_size_y = anker.size[updown];
                    if (lock_y < anker_coor_y + anker_size_y || lock_y < anker_coor_y)
                    {
                        base::anchor += object_area.coor - anker.coor;
                        found = true;
                    }
                }
                size_y = entry.base::socket.size[updown];
                entry.base::notify(object_area);
                coor_y += size_y;
            }
        }

    public:
        // list: .
        void clear()
        {
            auto backup = This();
            while (subset.size())
            {
                auto item_ptr = subset.back();
                subset.pop_back();
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, backup);
            }
        }
        // list: Remove the last nested object. Return the object refrence.
        auto pop_back()
        {
            if (subset.size())
            {
                auto object = subset.back();
                auto backup = This();
                subset.pop_back();
                object->SIGNAL(tier::release, e2::form::upon::vtree::detached, backup);
                return object;
            }
            return sptr{};
        }
        // list: Attach specified item.
        template<sort Order = sort::forward, class T>
        auto attach(T object)
        {
            auto order = Order == sort::forward ? lineup : lineup == sort::reverse ? sort::forward : sort::reverse;
            if (order == sort::reverse) subset.insert(subset.begin(), object);
            else                        subset.push_back(object);
            object->SIGNAL(tier::release, e2::form::upon::vtree::attached, This());
            return object;
        }
    };

    // controls: Layered cake of objects on top of each other.
    class cake
        : public form<cake>
    {
    protected: 
        cake()
        {
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                for (auto& object : subset)
                {
                    object->render(parent_canvas);
                }
            };
        }
        // cake: .
        void deform(rect& new_area) override
        {
            auto new_coor = new_area.coor;
            auto new_size = new_area.size;
            auto meter = [&]
            {
                for (auto& object : subset)
                {
                    object->base::recalc(new_area);
                    new_area.coor = new_coor;
                }
            };
            meter();
            if (subset.size() > 1 && new_size != new_area.size)
            {
                meter();
            }
        }
        // cake: .
        void inform(rect new_area) override
        {
            for (auto& object : subset)
            {
                object->base::notify(new_area);
            }
        }

    public:
        // cake: Remove the last nested object. Return the object refrence.
        auto pop_back()
        {
            if (subset.size())
            {
                auto object = subset.back();
                auto backup = This();
                subset.pop_back();
                object->SIGNAL(tier::release, e2::form::upon::vtree::detached, backup);
                return object;
            }
            return sptr{};
        }
        // cake: Create a new item of the specified subtype and attach it.
        template<class T>
        auto attach(T object)
        {
            if (object)
            {
                subset.push_back(object);
                object->SIGNAL(tier::release, e2::form::upon::vtree::attached, This());
            }
            return object;
        }
    };

    // controls: Container for multiple objects, but only the last one is shown.
    class veer
        : public form<veer>
    {
    protected:
        veer()
        {
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                if (subset.size())
                if (auto object = subset.back())
                {
                    object->render(parent_canvas);
                }
            };
        }
        // veer: .
        void deform(rect& new_area) override
        {
            if (subset.size())
            if (auto object = subset.back())
            {
                object->base::recalc(new_area);
            }
        }
        // veer: .
        void inform(rect new_area) override
        {
            if (subset.size())
            if (auto object = subset.back())
            {
                object->base::notify(new_area);
            }
        }

    public:
        // veer: Return the last object or empty sptr.
        auto back()
        {
            return subset.size() ? subset.back()
                                 : sptr{};
        }
        // veer: Return the first object or empty sptr.
        auto front()
        {
            return subset.size() ? subset.front()
                                 : sptr{};
        }
        // veer: Return nested objects count.
        auto count()
        {
            return subset.size();
        }
        // veer: Return true if empty.
        auto empty()
        {
            return subset.empty();
        }
        // veer: Remove the last object. Return the object refrence.
        auto pop_back()
        {
            if (subset.size())
            {
                auto object = subset.back();
                auto backup = This();
                subset.pop_back();
                object->SIGNAL(tier::release, e2::form::upon::vtree::detached, backup);
                return object;
            }
            return sptr{};
        }
        // veer: Roll objects.
        void roll(si32 dt = 1)
        {
            if (dt && subset.size() > 1)
            {
                if (dt > 0) while (dt--)
                {
                    subset.insert(subset.begin(), subset.back());
                    subset.pop_back();
                }
                else while (dt++)
                {
                    subset.push_back(subset.front());
                    subset.erase(subset.begin());
                }
            }
        }
        // veer: Create a new item of the specified subtype and attach it.
        template<class T>
        auto attach(T object)
        {
            subset.push_back(object);
            object->SIGNAL(tier::release, e2::form::upon::vtree::attached, This());
            return object;
        }
    };

    // controls: Text page.
    template<auto fx>
    class postfx
        : public flow, public form<postfx<fx>>
    {
        twod square; // post: Page area.
        text source; // post: Text source.
        bool beyond; // post: Allow vertical scrolling beyond the last line.

    protected:
        postfx(bool scroll_beyond = faux)
            :   flow{ square        },
              beyond{ scroll_beyond }
        {
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                output(parent_canvas);
                //auto mark = rect{ base::anchor + base::coor(), {10,5} };
                //mark.coor += parent_canvas.view().coor; // Set client's basis
                //parent_canvas.fill(mark, [](cell& c) { c.alpha(0x80).bgc().chan.r = 0xff; });
            };
        }
        // post: .
        void deform(rect& new_area) override
        {
            square = new_area.size;
            auto entry = topic.lookup(base::anchor);
            flow::reset();
            auto publish = [&](auto& combo)
            {
                combo.coord = flow::print(combo);
                if (combo.id() == entry.id) entry.coor.y -= combo.coord.y;
            };
            topic.stream(publish);

            // Apply only vertical anchoring for this type of control.
            base::anchor.y -= entry.coor.y; // Move the central point accordingly to the anchored object

            auto& cover = flow::minmax();
            //todo move it to flow
            base::oversz = { -std::min(0, cover.l),
                              std::max(0, cover.r - square.x + 1),
                             -std::min(0, cover.t),
                              0 };
            auto height = cover.width() ? cover.height() + 1
                                        : 0;
            if (beyond) square.y += height - 1;
            else        square.y  = height;
            new_area.size.y = square.y;
        }
        // post: .
        void inform(rect new_area) override
        {
            if (square.x != new_area.size.x)
            {
            	deform(new_area);
            }
        }

    public:
        page topic; // post: Text content.

        // post: .
        auto& lyric(si32 paraid) { return *topic[paraid].lyric; }
        // post: .
        auto& content(si32 paraid) { return topic[paraid]; }
        // post: .
        auto upload(view utf8, si32 initial_width = 0) // Don't use cell link id here. Apply it to the parent (with a whole rect coverage).
        {
            source = utf8;
            topic = utf8;
            base::resize(twod{ initial_width, 0 } + base::intpad);
            base::reflow();
            return this->This();
        }
        // post: .
        auto& get_source() const
        {
            return source;
        }
        // post: .
        void output(face& canvas)
        {
            flow::reset(canvas);
            auto publish = [&](auto const& combo)
            {
                flow::print(combo, canvas, fx);
            };
            topic.stream(publish);
        }
    };

    using post = postfx<noop{}>;

    // controls: Scroller.
    class rail
        : public form<rail>
    {
        pro::robot robot{*this }; // rail: Animation controller.

        using upon = e2::form::upon;

        twod strict; // rail: Don't allow overscroll.
        twod manual; // rail: Manual scrolling (no auto align).
        twod permit; // rail: Allowed axes to scroll.
        twod siezed; // rail: Allowed axes to capture.
        twod oversc; // rail: Allow overscroll with auto correct.
        subs fasten; // rail: Subscriptions on masters to follow they state.
        rack scinfo; // rail: Scroll info.
        bool animat; // rail: Smooth scrolling.

        si32 spd       = skin::globals().spd;
        si32 pls       = skin::globals().pls;
        si32 ccl       = skin::globals().ccl;
        si32 spd_accel = skin::globals().spd_accel;
        si32 ccl_accel = skin::globals().ccl_accel;
        si32 spd_max   = skin::globals().spd_max;
        si32 ccl_max   = skin::globals().ccl_max;
        si32 switching = skin::globals().switching;
        si32 wheel_dt  = skin::globals().wheel_dt;

        si32 speed{ spd  }; // rail: Text auto-scroll initial speed component ΔR.
        si32 pulse{ pls  }; // rail: Text auto-scroll initial speed component ΔT.
        si32 cycle{ ccl  }; // rail: Text auto-scroll duration in ms.
        bool steer{ faux }; // rail: Text scroll vertical direction.

        // rail: .
        static constexpr auto xy(axes Axes)
        {
            return twod{ !!(Axes & axes::X_only), !!(Axes & axes::Y_only) };
        }
        // rail: .
        bool empty() //todo VS2019 requires bool
        {
            return base::subset.empty() || !base::subset.back();
        }

    protected:
        rail(axes allow_to_scroll = axes::all, axes allow_to_capture = axes::all, axes allow_overscroll = axes::all, bool smooth_scrolling = true)
            : permit{ xy(allow_to_scroll)  },
              siezed{ xy(allow_to_capture) },
              oversc{ xy(allow_overscroll) },
              strict{ xy(axes::all) },
              manual{ xy(axes::all) },
              animat{ smooth_scrolling }
        {
            LISTEN(tier::preview, e2::form::upon::scroll::any, info) // Receive scroll parameters from external sources.
            {
                auto delta = dot_00;
                switch (this->bell::protos<tier::preview>())
                {
                    case upon::scroll::bycoor::v.id: delta = { scinfo.window.coor - info.window.coor };        break;
                    case upon::scroll::bycoor::x.id: delta = { scinfo.window.coor.x - info.window.coor.x, 0 }; break;
                    case upon::scroll::bycoor::y.id: delta = { 0, scinfo.window.coor.y - info.window.coor.y }; break;
                    case upon::scroll::to_top::v.id: delta = { dot_mx };                                       break;
                    case upon::scroll::to_top::x.id: delta = { dot_mx.x, 0 };                                  break;
                    case upon::scroll::to_top::y.id: delta = { 0, dot_mx.y };                                  break;
                    case upon::scroll::to_end::v.id: delta = { -dot_mx };                                      break;
                    case upon::scroll::to_end::x.id: delta = { -dot_mx.x, 0 };                                 break;
                    case upon::scroll::to_end::y.id: delta = { 0, -dot_mx.y };                                 break;
                    case upon::scroll::bystep::v.id: delta = { info.vector };                                  break;
                    case upon::scroll::bystep::x.id: delta = { info.vector.x, 0 };                             break;
                    case upon::scroll::bystep::y.id: delta = { 0, info.vector.y };                             break;
                    case upon::scroll::bypage::v.id: delta = { info.vector * scinfo.window.size };             break;
                    case upon::scroll::bypage::x.id: delta = { info.vector.x * scinfo.window.size.x, 0 };      break;
                    case upon::scroll::bypage::y.id: delta = { 0, info.vector.y * scinfo.window.size.y };      break;
                    case upon::scroll::cancel::v.id: cancel<X, true>(); cancel<Y, true>();                     break;
                    case upon::scroll::cancel::x.id: cancel<X, true>();                                        break;
                    case upon::scroll::cancel::y.id: cancel<Y, true>();                                        break;
                    default:                         break;
                }
                if (delta) scroll(delta);
            };
            LISTEN(tier::request, e2::form::upon::scroll::any, req_scinfo)
            {
                req_scinfo = scinfo;
            };

            using button = hids::events::mouse::button;
            LISTEN(tier::release, hids::events::mouse::scroll::any, gear)
            {
                if (gear.meta(hids::anyCtrl)) return; // Ctrl+Wheel is reserved for zooming.
                auto dt = gear.whldt > 0;
                auto hz = permit == xy(axes::X_only)
                      || (permit == xy(axes::all) && gear.meta(hids::anyAlt | hids::anyShift));
                if (hz) wheels<X>(dt);
                else    wheels<Y>(dt);
                gear.dismiss();
            };
            LISTEN(tier::release, button::drag::start::right, gear)
            {
                auto ds = gear.delta.get();
                auto dx = ds.x;
                auto dy = ds.y * 2;
                auto vt = std::abs(dx) < std::abs(dy);

                if ((siezed[X] && !vt)
                 || (siezed[Y] &&  vt))
                {
                    if (gear.capture(bell::id))
                    {
                        manual = xy(axes::all);
                        strict = xy(axes::all) - oversc; // !oversc = dot_11 - oversc
                        gear.dismiss();
                    }
                }
            };
            LISTEN(tier::release, button::drag::pull::right, gear)
            {
                if (gear.captured(bell::id))
                {
                    auto delta = gear.mouse::delta.get();
                    auto value = permit * delta;
                    if (value) scroll(value);
                    gear.dismiss();
                }
            };
            LISTEN(tier::release, button::drag::cancel::right, gear)
            {
                if (gear.captured(bell::id))
                {
                    giveup(gear);
                }
            };
            LISTEN(tier::general, hids::events::halt, gear)
            {
                if (gear.captured(bell::id))
                {
                    giveup(gear);
                }
            };
            LISTEN(tier::release, button::drag::stop::right, gear)
            {
                if (gear.captured(bell::id))
                {
                    auto  v0 = gear.delta.avg();
                    auto& speed = v0.dS;
                    auto  cycle = datetime::round<si32>(v0.dT);
                    auto  limit = datetime::round<si32>(skin::globals().deceleration);
                    auto  start = 0;

                    if (permit[X]) actify<X>(quadratic{ speed.x, cycle, limit, start });
                    if (permit[Y]) actify<Y>(quadratic{ speed.y, cycle, limit, start });
                    //todo if (permit == xy(axes::all)) actify(quadratic{ speed, cycle, limit, start });

                    base::deface();
                    gear.setfree();
                    gear.dismiss();
                }
            };
            LISTEN(tier::release, button::click::right, gear)
            {
                if (!gear.captured(bell::id))
                {
                    if (manual[X]) cancel<X, true>();
                    if (manual[Y]) cancel<Y, true>();
                }
            };
            LISTEN(tier::release, button::down::any, gear)
            {
                cutoff();
            };
            LISTEN(tier::release, e2::form::animate::reset, id)
            {
                cutoff();
            };
            LISTEN(tier::release, e2::form::animate::stop, id)
            {
                switch (id)
                {
                    case Y: manual[Y] = true; /*scroll<Y>();*/ break;
                    case X: manual[X] = true; /*scroll<X>();*/ break;
                    default: break;
                }
                base::deface();
            };
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                if (empty()) return;
                auto& item = *base::subset.back();
                item.render(parent_canvas, faux);
            };
        }
        // rail: Resize nested object with scroll bounds checking.
        void inform(rect new_area) override
        {
            if (empty()) return;
            auto& item = *base::subset.back();
            item.base::anchor = base::anchor - item.base::region.coor;
            auto block = item.base::resize(new_area.size - item.base::extpad, faux);
            auto frame = new_area.size;
            auto delta = dot_00;
            revise(item, block, frame, delta);
            block += item.base::extpad;
            item.base::socket = block;
            item.base::accept(block);
        }

    public:
        // rail: .
        auto smooth(bool smooth_scroll = true)
        {
            animat = smooth_scroll;
            return This();
        }
        // rail: .
        template<axis Axis>
        auto follow(sptr master = {})
        {
            if (master)
            {
                master->LISTEN(tier::release, upon::scroll::bycoor::any, master_scinfo, fasten)
                {
                    auto backup_scinfo = master_scinfo;
                    this->SIGNAL(tier::preview, e2::form::upon::scroll::bycoor::_<Axis>, backup_scinfo);
                };
            }
            else fasten.clear();
            return This();
        }
        // rail: .
        void cutoff()
        {
            if (manual[X]) robot.pacify(X);
            if (manual[Y]) robot.pacify(Y);
        }
        // rail: .
        void giveup(hids& gear)
        {
            cancel<X>();
            cancel<Y>();
            base::deface();
            gear.setfree();
            gear.dismiss();
        }
        // rail: .
        template<axis Axis>
        void wheels(bool dir)
        {
            if (animat)
            {
                if (robot.active(Axis) && (steer == dir))
                {
                    speed += spd_accel;
                    cycle += ccl_accel;
                    speed = std::min(speed, spd_max);
                    cycle = std::min(cycle, ccl_max);
                }
                else
                {
                    steer = dir;
                    speed = spd;
                    cycle = ccl;
                    //todo at least one line should be
                    //move<Axis>(dir ? 1 : -1);
                }
                auto start = 0;
                auto boost = dir ? speed : -speed;
                if constexpr (Axis == X) boost *= 2;
                keepon<Axis>(quadratic<si32>(boost, pulse, cycle, start));
            }
            else
            {
                auto speed = dir ? wheel_dt : -wheel_dt;
                auto delta = Axis == X ? twod{ speed * 2, 0 }
                                       : twod{ 0, speed };
                scroll(delta);
            }
        }
        // rail: .
        template<axis Axis, class Fx>
        void keepon(Fx&& func)
        {
            strict[Axis] = true;
            robot.actify(Axis, std::forward<Fx>(func), [&](auto& p)
            {
                auto delta = Axis == X ? twod{ p, 0 }
                                       : twod{ 0, p };
                scroll(delta);
            });
        }
        // rail: Check overscroll if no auto correction.
        template<axis Axis>
        auto inside()
        {
            if (empty() || !manual[Axis]) return true;
            auto& item = *base::subset.back();
            auto frame = (base::size() - base::intpad)[Axis];
            auto block = item.base::area() + item.base::oversz;
            auto coord = block.coor[Axis];
            auto bound = std::min(frame - block.size[Axis], 0);
            auto clamp = std::clamp(coord, bound, 0);
            return clamp == coord;
        }
        // rail: .
        template<axis Axis, class Fx>
        void actify(Fx&& func)
        {
            if (inside<Axis>()) keepon<Axis>(std::forward<Fx>(func));
            else                lineup<Axis>();
        }
        // rail: .
        template<axis Axis, bool Forced = faux>
        void cancel()
        {
            if (Forced || !inside<Axis>()) lineup<Axis>();
        }
        // rail: .
        template<axis Axis>
        void lineup()
        {
            if (empty()) return;
            manual[Axis] = faux;
            auto& item = *base::subset.back();
            auto block = item.base::area();
            auto coord = block.coor[Axis];
            auto width = block.size[Axis];
            auto frame = (base::size() - base::intpad)[Axis];
            auto bound = std::min(frame - width, 0);
            auto newxy = std::clamp(coord, bound, 0);
            auto route = newxy - coord;
            auto tempo = switching;
            auto start = 0;
            auto fader = constlinearAtoB<si32>(route, tempo, start);
            keepon<Axis>(fader);
        }
        void revise(base& item, rect& block, twod frame, twod& delta)
        {
            auto coord = block.coor;
            auto width = block.size;
            auto basis = base::intpad.corner() + item.base::oversz.corner();
            frame -= base::intpad;
            coord -= basis; // Scroll origin basis.
            coord += delta;
            width += item.base::oversz;
            auto bound = std::min(frame - width, dot_00);
            auto clamp = std::clamp(coord, bound, dot_00);
            for (auto xy : { axis::X, axis::Y }) // Check overscroll if no auto correction.
            {
                if (coord[xy] != clamp[xy] && manual[xy] && strict[xy]) // Clamp if it is outside the scroll limits and no overscroll.
                {
                    delta[xy] = clamp[xy] - coord[xy];
                    coord[xy] = clamp[xy];
                }
            }
            coord += basis; // Object origin basis.
            block.coor = coord;
        }
        // rail: .
        void scroll(twod& delta)
        {
            if (empty()) return;
            auto& item = *base::subset.back();
            auto frame = base::size();
            auto block = item.base::area();
            revise(item, block, frame, delta);
            item.base::moveto(block.coor);
            base::deface();
        }
        // rail: Attach specified item.
        template<class T>
        auto attach(T object)
        {
            if (!empty()) remove(base::subset.back());
            base::subset.push_back(object);
            object->SIGNAL(tier::release, e2::form::upon::vtree::attached, This());
            object->LISTEN(tier::release, e2::area, new_area, object->relyon) // Sync scroll info.
            {
                if (empty()) return;
                auto& item = *base::subset.back();
                auto frame = base::socket.size - base::extpad - base::intpad;
                auto coord = new_area.coor;
                auto block = new_area.size + item.base::oversz;
                auto basis = base::intpad.corner() + item.base::oversz.corner();
                coord -= basis; // Scroll origin basis.
                scinfo.beyond = item.base::oversz;
                scinfo.region = block;
                scinfo.window.coor =-coord; // Viewport.
                scinfo.window.size = frame; //
                this->SIGNAL(tier::release, upon::scroll::bycoor::any, scinfo);
            };
            return object;
        }
        // rail: Detach specified object.
        void remove(sptr object) override
        {
            if (!empty() && base::subset.back() == object)
            {
                auto backup = This();
                base::subset.pop_back();
                object->SIGNAL(tier::release, e2::form::upon::vtree::detached, backup);
                scinfo.region = {};
                scinfo.window.coor = {};
                this->SIGNAL(tier::release, upon::scroll::bycoor::any, scinfo); // Reset dependent scrollbars.
                fasten.clear();
            }
            else base::subset.clear();
        }
        // rail: Update nested object.
        void update(sptr old_object, sptr new_object) override
        {
            auto object_coor = dot_00;
            if (!empty())
            {
                auto object = base::subset.back();
                object_coor = object->base::coor();
                remove(old_object);
            }
            if (new_object)
            {
                new_object->base::moveto(object_coor);
                attach(new_object);
            }
        }
    };

    // controls: Scrollbar.
    template<axis Axis, auto drawfx>
    class gripfx
        : public flow, public form<gripfx<Axis, drawfx>>
    {
        pro::timer timer{*this }; // grip: Minimize by timeout.

        using form = ui::form<gripfx<Axis, drawfx>>;
        using upon = e2::form::upon;

        enum activity
        {
            mouse_leave = 0, // faux
            mouse_hover = 1, // true
            pager_first = 10,
            pager_next  = 11,
        };

        struct math
        {
            rack  master_inf = {};                           // math: Master scroll info.
            si32& master_dir = master_inf.vector     [Axis]; // math: Master scroll direction.
            si32& master_len = master_inf.region     [Axis]; // math: Master len.
            si32& master_pos = master_inf.window.coor[Axis]; // math: Master viewport pos.
            si32& master_box = master_inf.window.size[Axis]; // math: Master viewport len.
            si32  scroll_len = 0; // math: Scrollbar len.
            si32  scroll_pos = 0; // math: Scrollbar grip pos.
            si32  scroll_box = 0; // math: Scrollbar grip len.
            si32   m         = 0; // math: Master max pos.
            si32   s         = 0; // math: Scroll max pos.
            double r         = 1; // math: Scroll/master len ratio.

            si32  cursor_pos = 0; // math: Mouse cursor position.

            // math: Calc scroll to master metrics.
            void s_to_m()
            {
                auto scroll_center = scroll_pos + scroll_box / 2.0;
                auto master_center = scroll_len ? scroll_center / r
                                                : 0;
                master_pos = (si32)std::round(master_center - master_box / 2.0);

                // Reset to extreme positions.
                if (scroll_pos == 0 && master_pos > 0) master_pos = 0;
                if (scroll_pos == s && master_pos < m) master_pos = m;
            }
            // math: Calc master to scroll metrics.
            void m_to_s()
            {
                if (master_box == 0) return;
                if (master_len == 0) master_len = master_box;
                r = (double)scroll_len / master_len;
                auto master_middle = master_pos + master_box / 2.0;
                auto scroll_middle = master_middle * r;
                scroll_box = std::max(1, (si32)(master_box * r));
                scroll_pos = (si32)std::round(scroll_middle - scroll_box / 2.0);

                // Don't place the grip behind the scrollbar.
                if (scroll_pos >= scroll_len) scroll_pos = scroll_len - 1;

                // Extreme positions are always closed last.
                s = scroll_len - scroll_box;
                m = master_len - master_box;

                if (scroll_len > 2) // Two-row hight is not suitable for this type of aligning.
                {
                    if (scroll_pos == 0 && master_pos > 0) scroll_pos = 1;
                    if (scroll_pos == s && master_pos < m) scroll_pos = s - 1;
                }
            }
            void update(rack const& scinfo)
            {
                master_inf = scinfo;
                m_to_s();
            }
            void resize(twod new_size)
            {
                scroll_len = new_size[Axis];
                m_to_s();
            }
            void stepby(si32 delta)
            {
                scroll_pos = std::clamp(scroll_pos + delta, 0, s);
                s_to_m();
            }
            void commit(rect& handle)
            {
                handle.coor[Axis]+= scroll_pos;
                handle.size[Axis] = scroll_box;
            }
            auto inside(si32 coor)
            {
                if (coor >= scroll_pos + scroll_box) return 1; // Below the grip.
                if (coor >= scroll_pos)              return 0; // Inside the grip.
                                                     return-1; // Above the grip.
            }
            auto follow()
            {
                auto dir = scroll_len > 2 ? inside(cursor_pos)
                                          : cursor_pos > 0 ? 1 // Don't stop to follow over
                                                           :-1;//    box on small scrollbar.
                return dir;
            }
            void setdir(si32 dir)
            {
                master_dir = -dir;
            }
        };

        wptr boss; // grip: .
        hook memo; // grip: .
        si32 thin; // grip: Scrollbar thickness.
        si32 init; // grip: Handle base width.
        math calc; // grip: Scrollbar calculator.
        bool wide; // grip: Is the scrollbar active.
        si32 mult; // grip: Vertical bar width multiplier.
        bool on_pager = faux; // grip: .

        template<class Event>
        void send()
        {
            if (auto master = this->boss.lock())
            {
                master->SIGNAL(tier::preview, Event::template _<Axis>, calc.master_inf);
            }
        }
        void config(si32 width)
        {
            thin = width;
            auto lims = Axis == axis::X ? twod{ -1, width }
                                        : twod{ width, -1 };
            base::limits(lims, lims);
        }
        void giveup(hids& gear)
        {
            if (on_pager)
            {
                gear.dismiss();
            }
            else
            {
                if (gear.captured(bell::id))
                {
                    if (this->form::template protos<tier::release>(hids::events::mouse::button::drag::cancel::right))
                    {
                        send<upon::scroll::cancel>();
                    }
                    base::deface();
                    gear.setfree();
                    gear.dismiss();
                }
            }
        }
        void pager(si32 dir)
        {
            calc.setdir(dir);
            send<upon::scroll::bypage>();
        }
        auto pager_repeat()
        {
            if (on_pager)
            {
                auto dir = calc.follow();
                pager(dir);
            }
            return on_pager;
        }

    protected:
        gripfx(sptr boss, si32 thickness = 1, si32 multiplier = 2)
            : boss{ boss       },
              thin{ thickness  },
              wide{ faux       },
              init{ thickness  },
              mult{ multiplier }
        {
            config(thin);

            boss->LISTEN(tier::release, upon::scroll::bycoor::any, scinfo, memo)
            {
                calc.update(scinfo);
                base::deface();
            };

            using bttn = hids::events::mouse::button;
            LISTEN(tier::release, hids::events::mouse::scroll::any, gear)
            {
                if (gear.meta(hids::anyCtrl)) return; // Ctrl+Wheel is reserved for zooming.
                if (gear.whldt)
                {
                    auto dir = gear.whldt < 0 ? 1 : -1;
                    pager(dir);
                    gear.dismiss();
                }
            };
            LISTEN(tier::release, hids::events::mouse::move, gear)
            {
                calc.cursor_pos = gear.mouse::coord[Axis];
            };
            LISTEN(tier::release, hids::events::mouse::button::dblclick::left, gear)
            {
                gear.dismiss(); // Do not pass double clicks outside.
            };
            LISTEN(tier::release, hids::events::mouse::button::down::any, gear)
            {
                if (!on_pager)
                if (this->form::template protos<tier::release>(bttn::down::left)
                 || this->form::template protos<tier::release>(bttn::down::right))
                if (auto dir = calc.inside(gear.mouse::coord[Axis]))
                {
                    if (gear.capture(bell::id))
                    {
                        on_pager = true;
                        pager_repeat();
                        gear.dismiss();

                        timer.actify(activity::pager_first, skin::globals().repeat_delay, [&](auto p)
                        {
                            if (pager_repeat())
                            {
                                timer.actify(activity::pager_next, skin::globals().repeat_rate, [&](auto d)
                                {
                                    return pager_repeat(); // Repeat until on_pager.
                                });
                            }
                            return faux; // One shot call (first).
                        });
                    }
                }
            };
            LISTEN(tier::release, hids::events::mouse::button::up::any, gear)
            {
                if (on_pager && gear.captured(bell::id))
                {
                    if (this->form::template protos<tier::release>(bttn::up::left)
                     || this->form::template protos<tier::release>(bttn::up::right))
                    {
                        gear.setfree();
                        gear.dismiss();
                        on_pager = faux;
                        timer.pacify(activity::pager_first);
                        timer.pacify(activity::pager_next);
                    }
                }
            };
            LISTEN(tier::release, hids::events::mouse::button::up::right, gear)
            {
                //if (!gear.captured(bell::id)) //todo why?
                {
                    send<upon::scroll::cancel>();
                    gear.dismiss();
                }
            };

            LISTEN(tier::release, hids::events::mouse::button::drag::start::any, gear)
            {
                if (on_pager)
                {
                    gear.dismiss();
                }
                else
                {
                    if (gear.capture(bell::id))
                    {
                        gear.dismiss();
                    }
                }
            };
            LISTEN(tier::release, hids::events::mouse::button::drag::pull::any, gear)
            {
                if (on_pager)
                {
                    gear.dismiss();
                }
                else
                {
                    if (gear.captured(bell::id))
                    {
                        if (auto delta = gear.mouse::delta.get()[Axis])
                        {
                            calc.stepby(delta);
                            send<upon::scroll::bycoor>();
                            gear.dismiss();
                        }
                    }
                }
            };
            LISTEN(tier::release, hids::events::mouse::button::drag::cancel::any, gear)
            {
                giveup(gear);
            };
            LISTEN(tier::general, hids::events::halt, gear)
            {
                giveup(gear);
            };
            LISTEN(tier::release, hids::events::mouse::button::drag::stop::any, gear)
            {
                if (on_pager)
                {
                    gear.dismiss();
                }
                else
                {
                    if (gear.captured(bell::id))
                    {
                        if (this->form::template protos<tier::release>(bttn::drag::stop::right))
                        {
                            send<upon::scroll::cancel>();
                        }
                        base::deface();
                        gear.setfree();
                        gear.dismiss();
                    }
                }
            };
            LISTEN(tier::release, e2::form::state::mouse, active)
            {
                auto apply = [&](auto active)
                {
                    wide = active;
                    if (Axis == axis::Y && mult) config(active ? init * mult // Make vertical scrollbar
                                                               : init);      // wider on hover.
                    base::reflow();
                    return faux; // One shot call.
                };

                timer.pacify(activity::mouse_leave);

                if (active) apply(activity::mouse_hover);
                else timer.actify(activity::mouse_leave, skin::globals().active_timeout, apply);
            };
            //LISTEN(tier::release, hids::events::mouse::move, gear)
            //{
            //	auto apply = [&](auto active)
            //	{
            //		wide = active;
            //		if (Axis == axis::Y) config(active ? init * 2 // Make vertical scrollbar
            //		                                   : init);   //  wider on hover
            //		base::reflow();
            //		return faux; // One shot call
            //	};
            //
            //	timer.pacify(activity::mouse_leave);
            //	apply(activity::mouse_hover);
            //	timer.template actify<activity::mouse_leave>(skin::globals().active_timeout, apply);
            //};
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                auto region = parent_canvas.view();
                auto object = parent_canvas.full();
                auto handle = region;

                calc.commit(handle);

                auto& handle_len = handle.size[Axis];
                auto& region_len = region.size[Axis];
                auto& object_len = object.size[Axis];

                handle = region.clip(handle);
                handle_len = std::max(1, handle_len);

                drawfx(*this, parent_canvas, handle, object_len, handle_len, region_len, wide);
            };
        }
        // gripfx: .
        void inform(rect new_area) override
        {
            calc.resize(new_area.size);
        }
    };

    namespace drawfx
    {
        static constexpr auto xlight = [](auto& boss, auto& canvas, auto handle, auto object_len, auto handle_len, auto region_len, auto wide)
        {
            if (object_len && handle_len != region_len) // Show only if it is oversized.
            {
                // Brightener isn't suitable for white backgrounds.
                //auto bright = skin::color(tone::brighter);
                //bright.bga(bright.bga() / 2).fga(0);
                //bright.link(bell::id);

                if (wide) // Draw full scrollbar on mouse hover
                {
                    canvas.fill([&](cell& c) { c.link(boss.bell::id).xlight(); });
                }
                //canvas.fill(handle, [&](cell& c) { c.fusefull(bright); });
                canvas.fill(handle, [&](cell& c) { c.link(boss.bell::id).xlight(); });
            }
        };
        static constexpr auto underline = [](auto& boss, auto& canvas, auto handle, auto object_len, auto handle_len, auto region_len, auto wide)
        {
            if (object_len && handle_len != region_len) // Show only if it is oversized.
            {
                //canvas.fill(handle, [](cell& c) { c.und(!c.und()); });
                canvas.fill(handle, [](cell& c) { c.und(true); });
            }
        };
    }

    template<axis Axis>
    using grip = gripfx<Axis, drawfx::xlight>;

    // controls: deprecated.
    class pads
        : public form<pads>
    {
        dent intpad;
        dent extpad;

        // pads: .
        bool empty() //todo VS2019 requires bool
        {
            return base::subset.empty() || !base::subset.back();
        }

    protected:
        pads(dent intpad_value = {}, dent extpad_value = {})
            : intpad{ intpad_value },
              extpad{ extpad_value }
        {
            LISTEN(tier::release, e2::render::prerender, parent_canvas)
            {
                auto view = parent_canvas.view();
                parent_canvas.view(view + extpad);
                this->SIGNAL(tier::release, e2::render::any, parent_canvas);
                parent_canvas.view(view);
                if (!empty())
                {
                    auto& item = *base::subset.back();
                    item.render(parent_canvas);
                }
                this->bell::expire<tier::release>();
            };
        }
        // pads: .
        void deform(rect& new_area) override
        {
            if (empty()) return;
            auto& item = *base::subset.back();
            auto object_area = new_area - intpad;
            item.base::recalc(object_area);
            new_area = object_area + intpad;
        }
        // pads: .
        void inform(rect new_area) override
        {
            if (empty()) return;
            auto& item = *base::subset.back();
            auto object_area = new_area - intpad;
            item.base::notify(object_area);
        }

    public:
        // pads: Attach specified object.
        template<class T>
        auto attach(T object)
        {
            if (!empty()) remove(base::subset.back());
            base::subset.push_back(object);
            object->SIGNAL(tier::release, e2::form::upon::vtree::attached, This());
            return object;
        }
        // pads: Remove object.
        void remove(sptr object) override
        {
            if (!empty() && base::subset.back() == object)
            {
                auto backup = This();
                base::subset.pop_back();
                object->SIGNAL(tier::release, e2::form::upon::vtree::detached, backup);
            }
            else base::subset.clear();
        }
        // pads: Update nested object.
        void update(sptr old_object, sptr new_object) override
        {
            remove(old_object);
            attach(new_object);
        }
    };

    // controls: Pluggable dummy object.
    class mock
        : public form<mock>
    { };

    // controls: Text label.
    class item
        : public form<item>
    {
        static constexpr auto dots = "…"sv;
        para data{}; // item: Label content.
        bool flex{}; // item: Violate or not the label size.
        bool test{}; // item: Place or not(default) the Two Dot Leader when there is not enough space.
        bool unln{}; // item: Draw full-width underline.

    protected:
        item(view label)
            : data{ label }
        {
            LISTEN(tier::release, e2::data::utf8, utf8)
            {
                set(utf8);
            };
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                auto full = parent_canvas.full();
                auto context = parent_canvas.bump(-base::intpad, faux);
                parent_canvas.cup(dot_00);
                parent_canvas.output(data);
                if (test)
                {
                    auto area = parent_canvas.view();
                    auto size = data.size();
                    if (area.size > 0 && size.x > 0)
                    {
                        if (full.coor.x < area.coor.x)
                        {
                            auto coor = area.coor - parent_canvas.coor();
                            coor.y += std::min(area.size.y - 1, base::intpad.t);
                            parent_canvas.core::data(coor)->txt(dots);
                        }
                        if (full.coor.x + base::intpad.l + size.x + base::intpad.r > area.coor.x + area.size.x)
                        {
                            auto coor = area.coor - parent_canvas.coor();
                            coor.x += area.size.x - 1;
                            coor.y += std::min(area.size.y - 1, base::intpad.t);
                            parent_canvas.core::data(coor)->txt(dots);
                        }
                    }
                }
                if (unln)
                {
                    auto area = parent_canvas.full();
                    parent_canvas.fill(area, [](cell& c)
                    {
                        auto u = c.und();
                        if (u == 1) c.und(2);
                        else        c.und(1);
                    });
                }
                parent_canvas.bump(context);
            };
        }
        // item: .
        void deform(rect& new_area) override
        {
            new_area.size.x = flex ? new_area.size.x : data.size().x;
            new_area.size.y = std::max(data.size().y, new_area.size.y);
        }

    public:
        // item: .
        auto flexible(bool b = true) { flex = b; return This(); }
        // item: .
        auto drawdots(bool b = true) { test = b; return This(); }
        // item: .
        auto accented(bool b = true) { unln = b; return This(); }
        // item: .
        void set(view utf8)
        {
            data.parser::style.wrp(wrap::off);
            data = utf8;
            base::reflow();
        }
    };

    // controls: Textedit box.
    class edit
        : public form<edit>
    {
        page data;

    protected:
        edit()
        {
        }
    };

    // DEPRECATED STUFF

    class stem_rate_grip
        : public form<stem_rate_grip>
    {
        //todo cache specific
        netxs::sptr<face> coreface;
        face& canvas;

    public:
        page topic; // stem_rate_grip: Text content.

        bool enabled;
        text sfx_str;
        si32 sfx_len;
        text pin_str;
        si32 cur_val;
        twod box_len;

        enum
        {
            txt_id,
            pin_id,
        };

        void set_pen(byte hilight)
        {
            auto& pen = canvas.mark().bga(hilight);
        }
        void recalc()
        {
            auto cur_str = std::to_string(cur_val);
            auto cur_len = utf::length(cur_str);
            auto pin_pos = std::max(cur_len, sfx_len) + 1;
            box_len.x = 1 + 2 * pin_pos;
            box_len.y = 4;

            topic[txt_id] = cur_str + " " + sfx_str;
            topic[pin_id] = pin_str;
            topic[txt_id].locus.kill().chx(pin_pos - cur_len).cud(1);
            topic[pin_id].locus.kill().chx(pin_pos);
            topic.reindex();

            base::resize(box_len);
            deface();
        }
        auto set_val(si32 new_val, view pin_chr)
        {
            cur_val = new_val;
            pin_str = pin_chr;
            recalc();
            return box_len;
        }

    protected:
        // stem_rate_grip: .
        void deform(rect& new_area) override
        {
            new_area.size = box_len; // Suppress resize.
        }

        stem_rate_grip(view sfx_string)
            : sfx_str{ sfx_string }, canvas{*(coreface = ptr::shared<face>())}
        {
            //todo cache specific
            canvas.link(bell::id);
            LISTEN(tier::release, e2::area, new_area)
            {
                if (canvas.size() != new_area.size)
                {
                    canvas.size(new_area.size);
                }
            };
            LISTEN(tier::request, e2::form::canvas, canvas) { canvas = coreface; };

            sfx_len = utf::length(sfx_str);

            canvas.mark().bgc(whitelt);
            topic = ansi::idx(txt_id).nop().eol()
                         .idx(pin_id).nop();

            set_pen(0);

            LISTEN(tier::release, e2::form::state::mouse, active)
            {
                set_pen(active ? 80 : 0);
                recalc();
            };
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                if (base::ruined())
                {
                    canvas.wipe();
                    canvas.output(topic);
                    base::ruined(faux);
                }
                parent_canvas.fill(canvas, cell::shaders::fusefull);
            };
        }
    };

    template<tier Tier, class Event>
    class stem_rate
        : public form<stem_rate<Tier, Event>>
    {
        pro::robot robot{*this }; // stem_rate: Animation controller.

        //todo cache specific
        netxs::sptr<face> coreface;
        face& canvas;

        using tail = netxs::datetime::tail<si32>;

    public:
        page topic; // stem_rate: Text content.

        netxs::sptr<stem_rate_grip> grip_ctl;

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
        si32 pin_pos = 0;
        si32 bar_len = 0;
        si32 cur_val = 0;
        si32 min_val = 0;
        si32 max_val = 0;
        si32 origin = 0;
        si32 deltas = 0;

        //todo unify mint = 1/fps60 = 16ms
        //it seems that 4ms is enough, there is no need to be tied to fps (an open question)
        tail bygone{ 75ms, 4ms };

        text grip_suffix;
        text label_text;
        si32 pad = 5;
        si32 bgclr = 4;

        void recalc()
        {
            bar_len = std::max(0, base::size().x - (pad + 1) * 2);
            auto pin_abs = netxs::divround((bar_len + 1) * (cur_val - min_val),
                (max_val - min_val));
            auto pin_str = text{};
                 if (pin_abs == 0)           pin_str = "├";
            else if (pin_abs == bar_len + 1) pin_str = "┤";
            else                             pin_str = "┼";

            pin_len = grip_ctl->set_val(cur_val, pin_str);
            pin_pos = pad + pin_abs - pin_len.x / 2;
            topic[bar_id] = "└" + utf::repeat("─", bar_len) + "┘";
            topic[bar_id].locus.kill().chx(pad);
            topic.reindex();
        }
        si32 next_val(si32 delta)
        {
            auto dm = max_val - min_val;
            auto p = divround((bar_len + 1) * (origin - min_val), dm);
            auto c = divround((p + delta) * dm, (bar_len + 1)) + min_val;
            bygone.set(c - cur_val);
            return c;
        }
        bool _move_grip(si32 new_val)
        {
            new_val = std::clamp(new_val, min_val, max_val);
            if (new_val != cur_val)
            {
                cur_val = new_val;
                recalc();
                base::deface();

                return true;
            }
            return faux;
        }
        void move_grip(si32 new_val)
        {
            if (_move_grip(new_val))
            {
                RISEUP(Tier, Event{}, cur_val);
            }
        }
        void giveup(hids& gear)
        {
            if (gear.captured(grip_ctl->id))
            {
                deltas = 0;
                move_grip(origin);
                gear.setfree();
                gear.dismiss();
            }
        }

    protected:
        stem_rate(text const& caption, si32 min_value, si32 max_value, view suffix)
            : min_val{ min_value },
              max_val{ max_value },
              grip_suffix{ suffix },
              canvas{*(coreface = ptr::shared<face>())}
        {
            //todo cache specific
            canvas.link(bell::id);
            LISTEN(tier::release, e2::area, new_area)
            {
                if (canvas.size() != new_area.size)
                {
                    canvas.size(new_area.size);
                }
                recalc();
            };
            LISTEN(tier::request, e2::form::canvas, canvas) { canvas = coreface; };

            cur_val = -1;
            RISEUP(Tier, Event{}, cur_val);

            base::limits(twod{ utf::length(caption) + (pad + 2) * 2, 10 });

            topic = ansi::wrp(wrap::off).jet(bias::left)
                .cpy(50).chx(pad + 2).cuu(3).add(caption).cud(3)
                .idx(bar_id).nop().eol()
                .idx(min_id).nop().idx(max_id).nop();

            topic[min_id] = std::to_string(min_val);
            topic[max_id] = std::to_string(max_val);
            topic[max_id].style.jet(bias::right);
            topic[max_id].locus.chx(pad);
            topic[min_id].locus.chx(pad);

            LISTEN(tier::general, e2::form::global::lucidity, alpha)
            {
                if (alpha >= 0 && alpha < 256)
                {
                    canvas.mark().alpha(alpha);
                    base::deface();
                }
            };
            LISTEN(tier::general, Event{}, cur_val)
            {
                if (cur_val >= min_val)
                {
                    _move_grip(cur_val);
                }
            };
            LISTEN(tier::release, e2::form::upon::vtree::attached, parent)
            {
                grip_ctl = base::create<stem_rate_grip>(grip_suffix);
                grip_ctl->SIGNAL(tier::release, e2::form::upon::vtree::attached, base::This());

                grip_ctl->LISTEN(tier::release, hids::events::mouse::button::drag::start::left, gear)
                {
                    if (gear.capture(grip_ctl->id))
                    {
                        origin = cur_val;
                        gear.dismiss();
                    }
                };
                grip_ctl->LISTEN(tier::release, hids::events::mouse::button::drag::pull::left, gear)
                {
                    if (gear.captured(grip_ctl->id))
                    {
                        deltas += gear.mouse::delta.get().x;
                        move_grip(next_val(deltas));
                        gear.dismiss();
                    }
                };
                grip_ctl->LISTEN(tier::release, hids::events::mouse::button::drag::cancel::left, gear)
                {
                    giveup(gear);
                };
                grip_ctl->LISTEN(tier::general, hids::events::halt, gear)
                {
                    giveup(gear);
                };
                grip_ctl->LISTEN(tier::release, hids::events::mouse::button::drag::stop::left, gear)
                {
                    if (gear.captured(grip_ctl->id))
                    {
                        deltas = 0;
                        gear.setfree();
                        base::deface();
                        robot.actify(bygone.fader<quadratic<si32>>(750ms), [&](auto& delta)
                        {
                            move_grip(cur_val + delta);
                        });
                        gear.dismiss();
                    }
                };
                grip_ctl->LISTEN(tier::release, hids::events::mouse::scroll::up, gear)
                {
                    if (gear.meta(hids::anyCtrl)) return; // Ctrl+Wheel is reserved for zooming.
                    move_grip(cur_val - 1);
                    gear.dismiss();
                };
                grip_ctl->LISTEN(tier::release, hids::events::mouse::scroll::down, gear)
                {
                    if (gear.meta(hids::anyCtrl)) return; // Ctrl+Wheel is reserved for zooming.
                    move_grip(cur_val + 1);
                    gear.dismiss();
                };
                recalc();
            };
            LISTEN(tier::release, hids::events::mouse::button::click::right, gear)
            {
                base::color(canvas.mark().fgc(), (tint)((++bgclr) % 16));
                base::deface();
                gear.dismiss();
            };
            LISTEN(tier::release, hids::events::mouse::scroll::up, gear)
            {
                if (gear.meta(hids::anyCtrl)) return; // Ctrl+Wheel is reserved for zooming.
                move_grip(cur_val - 10);
                gear.dismiss();
            };
            LISTEN(tier::release, hids::events::mouse::scroll::down, gear)
            {
                if (gear.meta(hids::anyCtrl)) return; // Ctrl+Wheel is reserved for zooming.
                move_grip(cur_val + 10);
                gear.dismiss();
            };
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                if (base::ruined())
                {
                    base::ruined(faux); // The object may be invalidated again during rendering. (see pro::d_n_d)
                    canvas.wipe(base::color());
                    canvas.output(topic);
                    auto cp = canvas.cp();
                    cp.x = pin_pos;
                    cp.y -= 3;
                    grip_ctl->base::moveto(cp);
                    grip_ctl->render(canvas);
                }
                parent_canvas.fill(canvas, cell::shaders::fusefull);
            };
        }
    };
}