// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

#include "input.hpp"

static auto lua_torawstring(auto lua, auto idx, bool extended = faux)
{
    auto crop = netxs::text{};
    auto type = ::lua_type(lua, idx);
    if (type == LUA_TBOOLEAN)
    {
        crop = ::lua_toboolean(lua, idx) ? "true" : "false";
    }
    else if (type == LUA_TNUMBER || type == LUA_TSTRING)
    {
        ::lua_pushvalue(lua, idx); // ::lua_tolstring converts value to string in place.
        auto len = size_t{};
        auto ptr = ::lua_tolstring(lua, -1, &len);
        crop = { ptr, len };
        ::lua_pop(lua, 1);
    }
    else if (type == LUA_TLIGHTUSERDATA)
    {
        if (auto object_ptr = (netxs::bell*)::lua_touserdata(lua, idx)) // Get Object_ptr.
        {
            crop = netxs::utf::concat("<object:", object_ptr->id, ">");
        }
    }
    else if (extended)
    {
             if (type == LUA_TFUNCTION) crop = "<function>";
        else if (type == LUA_TTABLE)    crop = "<table>"; //todo expand table
    }
    return crop;
}

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
            virtual ~skill() = default; // Advertise polymorphicity.

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
                    boss.LISTEN(tier::general, input::events::die, gear, token)
                    {
                        del(gear);
                    };
                    boss.LISTEN(tier::release, input::events::mouse::hover::any, gear, token)
                    {
                             if (gear.cause == input::events::mouse::hover::enter.id) add(gear);
                        else if (gear.cause == input::events::mouse::hover::leave.id) dec(gear);
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

        // pro: Resizer.
        class sizer
            : public skill
        {
            using list = socks<netxs::misc::szgrips>;
            using skill::boss,
                  skill::memo;

            list items;
            dent outer;
            dent inner;
            bool alive; // pro::sizer: The sizer state.

        public:
            void props(dent outer_rect = { 2, 2, 1, 1 }, dent inner_rect = {})
            {
                outer = outer_rect;
                inner = inner_rect;
            }
            auto get_props()
            {
                return std::pair{ outer, inner };
            }

            sizer(base&&) = delete;
            sizer(base& boss, dent outer_rect = { 2, 2, 1, 1 }, dent inner_rect = {})
                : skill{ boss          },
                  items{ boss          },
                  outer{ outer_rect    },
                  inner{ inner_rect    },
                  alive{ true          }
            {
                // Drop it in favor of changing the cell size in GUI mode.
                //boss.LISTEN(tier::release, input::events::mouse::scroll::act, gear, memo)
                //{
                //    if (gear.meta(hids::anyCtrl) && !gear.meta(hids::ScrlLock) && gear.whlsi)
                //    {
                //        auto& g = items.take(gear);
                //        if (!g.zoomon)// && g.inside)
                //        {
                //            g.zoomdt = {};
                //            g.zoomon = true;
                //            g.zoomsz = boss.base::area();
                //            g.zoomat = gear.coord;
                //            gear.capture(boss.id);
                //        }
                //        static constexpr auto warp = dent{ 2, 2, 1, 1 } * 2;
                //        //todo respect pivot
                //        auto prev = g.zoomdt;
                //        auto coor = boss.base::coor();
                //        auto deed = boss.bell::protos(tier::release);
                //        g.zoomdt += warp * gear.whlsi;
                //        auto viewport = gear.owner.base::signal(tier::request, e2::form::prop::viewport);
                //        auto next = g.zoomsz + g.zoomdt;
                //        next.size = std::max(dot_00, next.size);
                //        next.trimby(viewport);
                //        auto step = boss.base::extend(next);
                //        if (!step.size) // Undo if can't zoom.
                //        {
                //            g.zoomdt = prev;
                //            boss.base::moveto(coor);
                //        }
                //    }
                //};
                boss.LISTEN(tier::release, e2::config::plugins::sizer::alive, state, memo)
                {
                    alive = state;
                };
                boss.LISTEN(tier::release, e2::postrender, canvas, memo)
                {
                    if (!alive) return;
                    auto area = canvas.full() + outer;
                    auto bord = outer - inner;
                    canvas.cage(area, bord, [&](cell& c)
                    {
                        c.link(boss.id);
                        if (c.bga() == 0) c.bga(1); // Active transparent.
                    });
                    items.foreach([&](auto& item)
                    {
                        item.draw(canvas, area, cell::shaders::xlight);
                    });
                };
                boss.LISTEN(tier::preview, e2::form::layout::swarp, warp, memo)
                {
                    auto area = boss.base::area();
                    auto next = area + warp;
                    boss.base::extend(next);
                };
                boss.LISTEN(tier::release, e2::config::plugins::sizer::outer, outer_rect, memo)
                {
                    outer = outer_rect;
                };
                boss.LISTEN(tier::release, e2::config::plugins::sizer::inner, inner_rect, memo)
                {
                    inner = inner_rect;
                };
                boss.LISTEN(tier::request, e2::config::plugins::sizer::inner, inner_rect, memo)
                {
                    inner_rect = inner;
                };
                boss.LISTEN(tier::request, e2::config::plugins::sizer::outer, outer_rect, memo)
                {
                    outer_rect = outer;
                };
                boss.LISTEN(tier::release, input::events::mouse::move, gear, memo)
                {
                    auto& g = items.take(gear);
                    if (g.zoomon && !gear.meta(hids::anyCtrl))
                    {
                        g.zoomon = faux;
                        gear.setfree();
                    }
                    auto area = boss.base::area();
                    auto coor = area.coor + gear.coord;
                    if (g.calc(area, coor, outer, inner))
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
                boss.base::signal(tier::release, e2::form::draggable::_<Button>, true);
                boss.LISTEN(tier::release, e2::form::drag::start::_<Button>, gear, memo)
                {
                    auto area = boss.base::area();
                    auto coor = area.coor + gear.coord;
                    if (items.take(gear).grab(area, coor, outer))
                    {
                        gear.dismiss();
                        boss.bell::expire(tier::release); // To prevent d_n_d triggering.
                    }
                };
                boss.LISTEN(tier::release, e2::form::drag::pull::_<Button>, gear, memo)
                {
                    auto& g = items.take(gear);
                    if (g.seized)
                    {
                        auto zoom = gear.meta(hids::anyCtrl);
                        auto area = boss.base::area();
                        auto coor = area.coor + gear.coord;
                        auto [preview_area, size_delta] = g.drag(area, coor, outer, zoom);
                        boss.base::signal(tier::preview, e2::area, preview_area);
                        if (auto dxdy = boss.sizeby(size_delta))
                        {
                            auto step = g.move(dxdy, zoom);
                            boss.moveby(step);
                            boss.base::signal(tier::preview, e2::form::upon::changed, dxdy);
                        }
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
                    boss.base::signal(tier::release, e2::form::upon::dragged, gear);
                };
            }
        };

        // pro: Moving by dragging support.
        class mover
            : public skill
        {
            struct sock
            {
                fp2d drag_origin; // sock: Drag origin.
                twod drag_center; // sock: Master center.
                void grab(base const& master, fp2d coord)
                {
                    drag_center = master.base::size() / 2;
                    drag_origin = coord - drag_center;
                }
                auto drag(base& master, fp2d coord)
                {
                    auto center = master.base::size() / 2;
                    auto delta = twod{ coord } - twod{ drag_origin } - center;
                    if (delta)
                    {
                        drag_origin += center - drag_center; // Keep origin tied to the master center.
                        drag_center = center;
                        master.base::moveby(delta);
                    }
                    return delta;
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
                boss.base::signal(tier::release, e2::form::draggable::_<Button>, true);
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
                        if (auto delta = items.take(gear).drag(*dest_object, gear.coord))
                        {
                            dest_object->base::signal(tier::preview, e2::form::upon::changed, delta);
                        }
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
                        dest_object->base::signal(tier::release, e2::form::upon::dragged, gear);
                        dest_object.reset();
                        gear.dismiss();
                    }
                };
            }
        };

        // pro: Mouse cursor glow (it is needed to apply pro::acryl after it).
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

            //using pool = std::list<id_t>;
            using list = socks<sock>;
            using skill::boss,
                  skill::memo;

            //pool focus; // track: Is keybd focused.
            list items; // track: .
            bool alive; // track: Is active.
/*
            void add_keybd(id_t gear_id)
            {
                if (gear_id)
                {
                    auto stat = focus.empty();
                    auto iter = std::find(focus.begin(), focus.end(), gear_id);
                    if (iter == focus.end())
                    {
                        focus.push_back(gear_id);
                        if (stat) boss.base::deface();
                    }
                }
            }
            void del_keybd(id_t gear_id)
            {
                if (gear_id)
                {
                    auto stat = focus.size();
                    auto iter = std::find(focus.begin(), focus.end(), gear_id);
                    if (iter != focus.end())
                    {
                        focus.erase(iter);
                        if (stat) boss.base::deface();
                    }
                }
            }
*/
            static auto& glow_overlay()
            {
                static auto bitmap = []
                {
                    auto r = 5;
                    auto blob = core{};
                    auto area = rect{ dot_00, dot_21 * (r * 2 + 1) };
                    auto func = netxs::spline01{ 0.65f };
                    blob.core::area(area, cell{}.bgc(0xFFffffff));
                    auto iter = blob.begin();
                    for (auto y = 0; y < area.size.y; y++)
                    {
                        auto y0 = (y - area.size.y / 2) / (area.size.y - 2 -     1.6f);
                        y0 *= y0;
                        for (auto x = 0; x < area.size.x; x++)
                        {
                            auto& c = iter++->bgc();
                            auto x0 = (x - area.size.x / 2) / (area.size.x - 4 - 2 * 1.6f);
                            auto dr = std::sqrt(std::abs(x0 * x0 + y0));
                            if (dr > 1) c.chan.a = 0;
                            else
                            {
                                auto a = std::round(255.0 * func(1.0f - dr));
                                c.chan.a = (byte)std::clamp((si32)(a * 0.16f), 0, 255);
                            }
                        }
                    }
                    return blob;
                }();
                return bitmap;
            }

        public:
            track(base&&) = delete;
            track(base& boss)
                : skill{ boss },
                  items{ boss },
                  alive{ true }
            {
                // Keybd focus.
                //boss.LISTEN(tier::release, input::events::focus::set::on, seed, memo)
                //{
                //    add_keybd(seed.gear_id);
                //};
                //boss.LISTEN(tier::release, input::events::focus::set::off, seed, memo)
                //{
                //    del_keybd(seed.gear_id);
                //};
                //boss.LISTEN(tier::release, input::events::die, gear, memo) // Gen by pro::focus.
                //{
                //    del_keybd(gear.id);
                //};
                // Mouse focus.
                //if (!skin::globals().tracking) return;
                boss.LISTEN(tier::release, input::events::mouse::move, gear, memo)
                {
                    items.take(gear).calc(boss, gear.coord);
                };
                boss.LISTEN(tier::release, e2::render::background::prerender, parent_canvas, memo)
                {
                    if (!alive) return;
                    auto& glow = glow_overlay();
                    auto  coor = parent_canvas.coor();
                    auto  full = parent_canvas.full();
                    auto  base = full.coor - coor - glow.size() / 2;
                    items.foreach([&](sock& item)
                    {
                        glow.move(base + item.cursor);
                        parent_canvas.plot(glow, cell::shaders::blend);
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
                boss.base::signal(tier::release, e2::form::animate::start, ID);
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
                boss.base::signal(tier::release, e2::form::animate::stop, id);
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
                //boss.base::signal(tier::release, e2::form::animate::stop, id);
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

        // pro: Text cursor.
        class caret
            : public skill
        {
            using skill::boss,
                  skill::memo;

            subs conf; // caret: Configuration subscriptions.
            bool live; // caret: Should the cursor be drawn.
            bool done; // caret: Is the cursor already drawn.
            bool unfocused; // caret: Is the cursor suppressed (lost focus).
            rect body; // caret: Cursor position.
            si32 form; // caret: Cursor style (netxs::text_cursor).
            si32 original_form; // caret: Original cursor form.
            span step; // caret: Blink interval. span::zero() if steady.
            time next; // caret: Time of next blinking.
            cell mark; // caret: Cursor brush.

        public:
            caret(base&&) = delete;
            caret(base& boss, bool visible = faux, si32 cursor_style = text_cursor::I_bar, twod position = dot_00, span freq = skin::globals().blink_period, cell default_color = cell{})
                : skill{ boss },
                   live{ faux },
                   done{ faux },
                   unfocused{ true },
                   body{ position, dot_11 }, // Cursor is always one cell size (see the term::scrollback definition).
                   form{ cursor_style },
                   original_form{ cursor_style },
                   step{ freq },
                   mark{ default_color }
            {
                boss.LISTEN(tier::release, e2::form::state::focus::count, count, conf)
                {
                    unfocused = !count;
                };
                boss.LISTEN(tier::request, e2::config::cursor::blink, req_step, conf)
                {
                    req_step = step;
                };
                boss.LISTEN(tier::request, e2::config::cursor::style, req_style, conf)
                {
                    req_style = form;
                };
                boss.LISTEN(tier::general, e2::config::cursor::blink, new_step, conf)
                {
                    blink_period(new_step);
                };
                boss.LISTEN(tier::preview, e2::config::cursor::blink, new_step, conf)
                {
                    blink_period(new_step);
                };
                boss.LISTEN(tier::general, e2::config::cursor::style, new_style, conf)
                {
                    style(new_style);
                };
                boss.LISTEN(tier::preview, e2::config::cursor::style, new_style, conf)
                {
                    style(new_style);
                };
                if (visible) show();
            }

            operator bool () const { return memo.size(); }

            // pro::caret: Set cursor background color.
            void bgc(argb c)
            {
                if (mark.bgc() != c)
                {
                    hide();
                    mark.bgc(c);
                    show();
                }
            }
            // pro::caret: Get cursor background color.
            auto bgc()
            {
                return mark.bgc();
            }
            // pro::caret: Set cursor color.
            void color(cell c)
            {
                if (mark != c)
                {
                    hide();
                    mark = c;
                    show();
                }
            }
            // pro::caret: Set cursor style.
            void style(si32 new_form)
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
            void decscusr(si32 mode)
            {
                switch (mode)
                {
                    case 1: // n = 1  blinking box (default)
                        blink_period();
                        style(text_cursor::block);
                        break;
                    case 2: // n = 2  steady box
                        blink_period(span::zero());
                        style(text_cursor::block);
                        break;
                    case 3: // n = 3  blinking underline
                        blink_period();
                        style(text_cursor::underline);
                        break;
                    case 4: // n = 4  steady underline
                        blink_period(span::zero());
                        style(text_cursor::underline);
                        break;
                    case 0: // n = 0  blinking I-bar
                    case 5: // n = 5  blinking I-bar
                        blink_period();
                        style(text_cursor::I_bar);
                        break;
                    case 6: // n = 6  steady I-bar
                        blink_period(span::zero());
                        style(text_cursor::I_bar);
                        break;
                    default:
                        log(prompt::term, "Unsupported cursor style requested, ", mode);
                        break;
                }
            }
            void toggle()
            {
                if (original_form == text_cursor::underline) style(form != text_cursor::block ? text_cursor::block : text_cursor::underline);
                else                                         style(form != text_cursor::block ? text_cursor::block : text_cursor::I_bar);
                reset();
            }
            // pro::caret: Set cursor position.
            void coor(twod coor)
            {
                if (body.coor != coor)
                {
                    reset();
                    body.coor = coor;
                }
            }
            // pro::caret: Get cursor position.
            auto& coor() const
            {
                return body.coor;
            }
            // pro::caret: Get cursor style.
            auto style() const
            {
                return std::pair{ form, !!(*this) };
            }
            // pro::caret: Force to redraw cursor.
            void reset()
            {
                if (step != span::zero())
                {
                    live = faux;
                    next = {};
                }
            }
            // pro::caret: Enable cursor.
            void show()
            {
                if (!*this)
                {
                    done = faux;
                    auto blinking = step != span::zero();
                    if (blinking)
                    {
                        live = faux;
                        boss.LISTEN(tier::general, e2::timer::tick, timestamp, memo)
                        {
                            if (timestamp > next)
                            {
                                next = timestamp + step;
                                live = !live;
                                boss.base::deface(body);
                            }
                        };
                    }
                    else
                    {
                        live = true;
                    }
                    boss.LISTEN(tier::release, e2::postrender, canvas, memo)
                    {
                        done = true;
                        auto blinking = step != span::zero();
                        auto visible = (unfocused == blinking) || (blinking && live);
                        auto clip = canvas.core::clip();
                        auto area = clip.trim(body);
                        if (area && form != text_cursor::I_bar)
                        {
                            auto& test = canvas.peek(body.coor);
                            //todo >2x1 matrix support
                            auto [w, h, x, y] = test.whxy();
                            if (w == 2 && x == 1) // Extend cursor to adjacent halves.
                            {
                                if (clip.hittest(body.coor + dot_10))
                                {
                                    auto& next = canvas.peek(body.coor + dot_10);
                                    auto [nw, nh, nx, ny] = next.whxy();
                                    if (nw == 2 && nx == 2 && test.same_txt(next))
                                    {
                                        area.size.x++;
                                    }
                                }
                            }
                            else if (w == 2 && x == 2)
                            {
                                if (clip.hittest(body.coor - dot_10))
                                {
                                    auto& prev = canvas.peek(body.coor - dot_10);
                                    auto [pw, ph, px, py] = prev.whxy();
                                    if (pw == 2 && px == 1 && test.same_txt(prev))
                                    {
                                        area.size.x++;
                                        area.coor.x--;
                                    }
                                }
                            }
                        }
                        if (visible)
                        {
                            if (area)
                            {
                                canvas.fill(area, [&](auto& c){ c.set_cursor(form, mark); });
                            }
                            else if (area.size.y)
                            {
                                auto chr = area.coor.x ? '>' : '<';
                                area.coor.x -= area.coor.x ? 1 : 0;
                                area.size.x = 1;
                                canvas.fill(area, [&](auto& c){ c.txt(chr).fgc(cell::shaders::contrast.invert(c.bgc())).cur(text_cursor::none); });
                            }
                        }
                        else
                        {
                            if (area)
                            {
                                canvas.fill(area, [&](auto& c){ c.cur(text_cursor::none); });
                            }
                            else if (area.size.y)
                            {
                                area.size.x = 1;
                                area.coor.x -= area.coor.x ? 1 : 0;
                                canvas.fill(area, [&](auto& c){ c.cur(text_cursor::none); });
                            }
                        }
                    };
                }
            }
            // pro::caret: Disable cursor.
            void hide()
            {
                if (*this)
                {
                    memo.clear();
                    if (done)
                    {
                        boss.base::deface(body);
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

            //todo use face::calc_page_height
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
                size.y = ooooooooo.flow::minmax().size.y;
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
                boss.base::signal(tier::release, e2::form::prop::ui::footer, foot_text);
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
                boss.base::signal(tier::release, e2::form::prop::ui::header, head_text);
                boss.base::signal(tier::release, e2::form::prop::ui::title , head_foci);
            }

            title(base&&) = delete;
            title(base& boss, view title = {}, view foots = {}, bool visible = true,
                                                                bool on_header = true,
                                                                bool on_footer = true)
                : skill{ boss },
                  head_live{ on_header },
                  foot_live{ on_footer },
                  live{ visible }
            {
                head_text = title;
                foot_text = foots;
                head_page = head_text;
                foot_page = foot_text;
                boss.LISTEN(tier::anycast, e2::form::upon::started, root, memo, (tokens = subs{}))
                {
                    if (head_live) header(head_text);
                    if (foot_live) footer(foot_text);
                    tokens.clear();
                    if (auto focusable_parent = boss.base::riseup(tier::request, e2::config::plugins::focus::owner))
                    {
                        focusable_parent->LISTEN(tier::release, e2::form::state::focus::on, gear_id, tokens)
                        {
                            if (!gear_id) return;
                            auto iter = std::find_if(user_icon.begin(), user_icon.end(), [&](auto& a){ return a.gear_id == gear_id; });
                            if (iter == user_icon.end())
                            if (auto gear_ptr = boss.bell::getref<hids>(gear_id))
                            {
                                auto index = gear_ptr->user_index;
                                auto color = argb::vt256[4 + index % (256 - 4)];
                                auto image = ansi::fgc(color).add("\0▀"sv);
                                user_icon.push_front({ gear_id, image });
                                rebuild();
                            }
                        };
                        focusable_parent->LISTEN(tier::release, e2::form::state::focus::off, gear_id, tokens)
                        {
                            if (!gear_id) return;
                            auto iter = std::find_if(user_icon.begin(), user_icon.end(), [&](auto& a){ return a.gear_id == gear_id; });
                            if (iter != user_icon.end())
                            {
                                user_icon.erase(iter);
                                rebuild();
                            }
                        };
                    }
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
                        boss.base::signal(tier::preview, e2::conio::quit);
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
                boss.LISTEN(tier::preview, input::events::mouse::any, something, pong)
                {
                    stop = datetime::now() + limit;
                };
                boss.LISTEN(tier::general, e2::timer::any, something, ping)
                {
                    if (datetime::now() > stop)
                    {
                        auto backup = boss.This();
                        log(prompt::gate, "No mouse clicking events");
                        boss.base::riseup(tier::release, e2::form::proceed::quit::one, true);
                        ping.reset();
                        memo.clear();
                    }
                };
            }
        };

        // pro: Text input focus tree.
        class focus
            : public skill
        {
            //
            //     ┌─── input gate 1 (vtm::gate<ui::gate, k>) -> base::root(true)
            //     │      ↓   ↑
            //     │      │   └─ gears (input_t)...
            //     │      └─ applet (Taskbar/Standalone app (ui::base, f))... 
            //     │
            //     │   ┌─── input gate M (vtm::gate<ui::gate, k>) -> base::root(true)                        ↑ Outside
            //     │   │      ↓   ↑                                                                         ██
            //     │   │      │   └─ gears (input_t)                                                        ██
            //     │   │      │       ├─ ... ├─ keybdN                                                    ██  ░░
            //     │   │      │       ├─ ... ├─ mouseN                                                    ██  ░░ Dead (unfocused, focusable)
            //     │   │      │       ├─ ... ├─ focusN                                                    ██
            //     │   │      │       └─ ... └─ clipboardN                                                ██ Live (focused, hub)
            //     │   │      │                                                                         ██  ██
            //     │   │      └─ applet (Taskbar/Standalone app (ui::base, f))                          ██  ██ Live (focused, focusable)
            //     │   │                                                                                ██
            //     ↓ : ↓                                                                                ██ Live (focused, hub)
            // vtm desktop (vtm::hall<ui::host, f<mode::focusable>>)                                  ██  ░░
            //  ↓ ↓  : ↓                                                                              ██  ░░ Dead (unfocused, hub)
            //  │ │    └─── window 1 (ui::cake, f) -> base::kind(reflow_root), base::root(true)       ██    ▓▓
            //  │ │          ↓                                                                        ██    ▓▓
            //  │ │          └─ applet (ui::dtvt, f<mode::relay>)                                   ██  ██  ▓▓ Idle (unfocused, focusable)
            //  │ │                                                                                 ██  ██...
            //  │ └─── window N (ui::cake, f) -> base::kind(reflow_root), base::root(true)          ██
            //  │       ↓                                                                           ██ Live (focused, focusable)
            //  │       └─ applet (tile (ui::fork, f, k)...)                                         ↓ Inside
            //  │           ↓
            //  │           └─ ...
            //  └─── window N+1 (ui::cake, f) -> base::kind(reflow_root), base::root(true)
            //        ↓
            //        └─ applet (info (ui::cake, f<focused>, k)...)
            //            ↓
            //            └─ ...
            struct state
            {
                static constexpr auto dead = 0;
                static constexpr auto live = 1;
                static constexpr auto idle = 2;
            };
            struct chain_t
            {
                struct dest_t
                {
                    wptr next_wptr; // next hop wptr.
                    si32 status{}; // dead, live or idle.
                };

                si32              active{}; // focus: The endpoint focus state.
                hook              token;    // focus: Cleanup token.
                std::list<dest_t> next;     // focus: Focus next hop list.

                template<class P>
                auto foreach(P proc)
                {
                    auto head = next.begin();
                    while (head != next.end())
                    {
                        if (auto nexthop = head->next_wptr.lock(); nexthop && (proc(nexthop, head->status), nexthop))
                        {
                            head++;
                        }
                        else
                        {
                            head = next.erase(head);
                        }
                    }
                }
            };

            using umap = std::unordered_map<id_t, chain_t>; // Each gear has its own focus tree.
            using skill::boss,
                  skill::memo;

            //todo kb navigation type: transit, cyclic, plain, disabled, closed
            umap gears; // focus: Registered gears.
            si32 node_type; // focus: .
            si32 count{}; // focus: The number of active gears.
            si64 treeid = datetime::uniqueid(); // focus: .
            ui64 digest = ui64{}; // focus: .

            auto add_chain(id_t gear_id, chain_t new_chain = { .active = state::dead })
            {
                auto iter = gears.emplace(gear_id, std::move(new_chain)).first;
                if (gear_id)
                {
                    if (auto gear_ptr = boss.bell::getref<hids>(gear_id))
                    {
                        auto& chain = iter->second;
                        gear_ptr->LISTEN(tier::release, input::events::die, gear, chain.token)
                        {
                            auto iter = gears.find(gear.id);
                            if (iter != gears.end()) // Make the current branch default.
                            {
                                auto& chain = iter->second;
                                auto  token = std::move(chain.token);
                                if (notify_focus_state(state::idle, chain, gear.id))
                                {
                                    chain.active = state::live; // Keep chain state.
                                }
                                gears[id_t{}] = std::move(chain);
                                boss.base::signal(tier::release, input::events::die, gear); // Notify pro::keybd.
                                gears.erase(iter);
                            }
                        };
                    }
                }
                return iter;
            }
            auto& get_chain(id_t gear_id)
            {
                auto iter = gears.find(gear_id);
                if (iter == gears.end())
                {
                    iter = add_chain(gear_id);
                }
                return iter->second;
            }
            bool notify_focus_state(si32 active, chain_t& chain, id_t gear_id)
            {
                auto changed = (chain.active == state::live) != (active == state::live);
                chain.active = active;
                if (gear_id && changed)
                {
                    if (active == state::live)
                    {
                        count++;
                        boss.base::signal(tier::release, e2::form::state::focus::on, gear_id);
                    }
                    else
                    {
                        count--;
                        boss.base::signal(tier::release, e2::form::state::focus::off, gear_id);
                    }
                    boss.base::signal(tier::release, e2::form::state::focus::count, count);
                }
                return changed;
            }

        public:
            struct mode
            {
                static constexpr auto focusable = 0; // The object can be focused and active, it is unfocused by default. It cuts the  focus tree when focus is set on it.
                static constexpr auto focused   = 1; // The object can be focused and active, it is focused by default. It cuts the  focus tree when focus is set on it.
                static constexpr auto hub       = 2; // The object can't be focused, only active, it is inactive by default. It doesn't cut the focus tree when focus is set on it, it just activate a whole branch.
                static constexpr auto active    = 3; // The object can't be focused, only active, it is active by default. It doesn't cut the focus tree when focus is set on it, it just activate a whole branch.
                static constexpr auto relay     = 4; // The object is on the process/event domain boundary and can't be focused (gui and ui::dtvt). Always has default focus.
            };

            template<class T>
            static void set(sptr item_ptr, T&& gear_id, si32 focus_type, bool just_activate_only = faux) // just_activate_only means don't focus just activate only.
            {
                auto lock = item_ptr->bell::sync();
                auto fire = [&](auto id)
                {
                    item_ptr->base::riseup(tier::preview, input::events::focus::set::on, { .gear_id = id, .focus_type = focus_type, .just_activate_only = just_activate_only });
                };
                if constexpr (std::is_same_v<id_t, std::decay_t<T>>)
                {
                    fire(gear_id);
                }
                else
                {
                    for (auto next_id : gear_id)
                    {
                        fire(next_id);
                    }
                }
            }
            template<class T>
            static void off(sptr item_ptr, T&& gear_id)
            {
                auto lock = item_ptr->bell::sync();
                auto fire = [&](auto id)
                {
                    item_ptr->base::riseup(tier::preview, input::events::focus::set::off, { .gear_id = id });
                };
                if constexpr (std::is_same_v<id_t, std::decay_t<T>>)
                {
                    fire(gear_id);
                }
                else
                {
                    for (auto next_id : gear_id)
                    {
                        fire(next_id);
                    }
                }
            }
            static auto off(sptr item_ptr) // Used by d_n_d::drop (tile.hpp:586).
            {
                auto lock = item_ptr->bell::sync();
                auto gear_id_list = item_ptr->base::riseup(tier::request, e2::form::state::keybd::enlist);
                pro::focus::off(item_ptr, gear_id_list);
            }
            // pro::focus: Defocus all gears except specified.
            static auto one(sptr item_ptr, id_t gear_id)
            {
                auto lock = item_ptr->bell::sync();
                auto gear_id_list = item_ptr->base::riseup(tier::request, e2::form::state::keybd::enlist);
                std::erase_if(gear_id_list, [&](auto& id){ return gear_id == id; });
                pro::focus::off(item_ptr, gear_id_list);
            }
            // pro::focus: Defocus all gears and clear chains (optionally remove_default route from parent) and return deleted active gear id's.
            static auto cut(sptr item_ptr)
            {
                auto lock = item_ptr->bell::sync();
                auto gear_id_list = item_ptr->base::riseup(tier::request, e2::form::state::keybd::enlist);
                if (auto parent = item_ptr->base::parent())
                {
                    parent->base::riseup(tier::request, input::events::focus::cut, { .item = item_ptr });
                }
                return gear_id_list;
            }
            // pro::focus: Switch all foci at the predecessor hub from the prev_ptr to the next_ptr (which must have a pro::focus on board).
            static auto hop(sptr prev_item_ptr, sptr next_item_ptr)
            {
                auto lock = prev_item_ptr->bell::sync();
                if (auto parent = prev_item_ptr->base::parent())
                {
                    parent->base::riseup(tier::request, input::events::focus::hop, { .item = prev_item_ptr, .next = next_item_ptr });
                }
            }
            static auto test(base& item, input::hids& gear)
            {
                auto gear_test = item.base::riseup(tier::request, e2::form::state::keybd::find, { gear.id, 0 });
                return gear_test.second;
            }
            static auto is_focused(sptr item_ptr, id_t gear_id)
            {
                return !gear_id || !!item_ptr->base::signal(tier::request, e2::form::state::keybd::find, { gear_id, 0 }).second;
            }
            auto is_focused(id_t gear_id)
            {
                auto iter = gears.find(gear_id);
                auto result = iter != gears.end() && iter->second.active == state::live;
                return result;
            }

            focus(base&&) = delete;
            focus(base& boss, si32 focus_mode = mode::hub, bool set_default_focus = true)
                : skill{ boss },
                  node_type{ focus_mode }
            {
                if (set_default_focus)
                if (node_type == mode::focused || node_type == mode::active || node_type == mode::relay) // Pave default focus path at startup.
                {
                    boss.LISTEN(tier::anycast, e2::form::upon::started, parent_ptr, memo)
                    {
                        if (parent_ptr) // Parent_ptr is always empty when the boss is dropped via d_n_d.
                        {
                            pro::focus::set(boss.This(), id_t{}, solo::off);
                        }
                    };
                }
                // pro::focus: Return focus owner ptr.
                boss.LISTEN(tier::request, e2::config::plugins::focus::owner, owner_ptr, memo)
                {
                    owner_ptr = boss.This();
                };
                // pro::focus: Set unique focus on left click. Set group focus on Ctrl+LeftClick.
                boss.LISTEN(tier::release, input::events::mouse::button::click::left, gear, memo)
                {
                    if (gear.meta(hids::anyCtrl))
                    {
                        if (pro::focus::test(boss, gear))
                        {
                            pro::focus::off(boss.This(), gear.id);
                        }
                        else
                        {
                            pro::focus::set(boss.This(), gear.id, solo::off);
                        }
                    }
                    else
                    {
                        pro::focus::set(boss.This(), gear.id, solo::on);
                    }
                    gear.dismiss();
                };
                // pro::focus: Subscribe on keybd events.
                boss.LISTEN(tier::preview, input::events::keybd::key::post, gear, memo) // preview: Run after any.
                {
                    auto sent = faux;
                    auto& chain = get_chain(gear.id);
                    auto handled = gear.handled;
                    auto new_handled = handled;
                    chain.foreach([&](auto& nexthop, auto& status)
                    {
                        if (status == state::live)
                        {
                            sent = true;
                            gear.handled = handled;
                            nexthop->base::signal(tier::preview, input::events::keybd::key::post, gear);
                            new_handled |= gear.handled;
                        }
                    });
                    gear.handled = new_handled;
                    if (!sent && node_type != mode::relay) // Send key::post event back. The relays themselves will later send it back.
                    {
                        auto parent_ptr = boss.This();
                        while ((!gear.handled || gear.keystat == input::key::released) && parent_ptr) // Always pass released key events.
                        {
                            parent_ptr->base::signal(tier::release, input::events::keybd::key::post, gear);
                            parent_ptr = parent_ptr->base::parent();
                        }
                    }
                };
                // all tier::previews going to outside (upstream)
                // all tier::releases going to inside (downstream)
                // pro::focus: Off focus to inside.
                boss.LISTEN(tier::release, input::events::focus::set::off, seed, memo)
                {
                    auto& chain = get_chain(seed.gear_id);
                    if (notify_focus_state(state::idle, chain, seed.gear_id))
                    {
                        chain.foreach([&](auto& nexthop, auto& status)
                        {
                            if (status == state::live)
                            {
                                status = state::idle;
                                nexthop->base::signal(tier::release, input::events::focus::set::off, seed);
                            }
                        });
                    }
                };
                // pro::focus: Make copy from default.
                boss.LISTEN(tier::request, input::events::focus::dup, seed, memo)
                {
                    auto iter = gears.find(id_t{});
                    if (iter != gears.end())
                    {
                        iter = add_chain(seed.gear_id, iter->second); // Create a new chain which is based on default.
                        auto& chain = iter->second;
                        if (chain.active == state::live)
                        {
                            chain.active = state::idle;
                            notify_focus_state(state::live, chain, seed.gear_id);
                            if (node_type == mode::relay)
                            {
                                seed.item = boss.This();
                                boss.base::signal(tier::release, input::events::focus::set::on, seed);
                            }
                        }
                        chain.foreach([&](auto& nexthop, auto& /*status*/)
                        {
                            nexthop->base::signal(tier::request, input::events::focus::dup, seed);
                        });
                    }
                };
                // pro::focus: Set focus to inside.
                boss.LISTEN(tier::release, input::events::focus::set::on, seed, memo)
                {
                    auto iter = gears.find(seed.gear_id);
                    if (iter == gears.end()) // No route to inside.
                    {
                        auto first_step = !seed.item; // No focused item yet. We are in the the first riseup iteration (pro::focus::set just called and catched the first plugin<pro::focus> owner). A focus leaf is not necessarily a visual tree leaf.
                        if (seed.gear_id && first_step && (iter = gears.find(id_t{}), iter != gears.end())) // Check if the default chain exists.
                        {
                            boss.base::signal(tier::request, input::events::focus::dup, seed);
                            return;
                        }
                        else
                        {
                            iter = add_chain(seed.gear_id);
                        }
                    }
                    auto& chain = iter->second;
                    auto prev_state = chain.active;
                    notify_focus_state(state::live, chain, seed.gear_id);
                    if (node_type != mode::relay)
                    {
                        auto allow_focusize = node_type == mode::focused || node_type == mode::focusable;
                        chain.foreach([&](auto& nexthop, auto& status)
                        {
                            if (status != state::dead || (!allow_focusize && prev_state == state::dead)) // Focusing a dead item activates a whole dead branch upto a focusable item.
                            {
                                status = state::live;
                                seed.item = boss.This();
                                nexthop->base::signal(tier::release, input::events::focus::set::on, seed);
                            }
                        });
                    }
                };
                // pro::focus: Set focus to outside.
                boss.LISTEN(tier::preview, input::events::focus::set::on, seed, memo)
                {
                    seed.treeid = treeid;
                    seed.digest = ++digest;
                    auto first_step = !seed.item; // No focused item yet. We are in the the first riseup iteration (pro::focus::set just called and catched the first plugin<pro::focus> owner). A focus leaf is not necessarily a visual tree leaf.
                    if (first_step)
                    {
                        auto allow_focusize = seed.just_activate_only ? faux : (node_type == mode::focused || node_type == mode::focusable); // Ignore focusablity if it is requested.
                        if (seed.gear_id && (!allow_focusize || seed.focus_type != solo::on)) // Hub or group focus.
                        {
                            auto release_seed = seed;
                            boss.base::signal(tier::release, input::events::focus::set::on, release_seed); // Turn on a default downstream branch.
                        }
                        else
                        {
                            auto& chain = get_chain(seed.gear_id);
                            if (allow_focusize && seed.focus_type == solo::on) // Cut a downstream focus branch.
                            {
                                chain.foreach([&](auto& nexthop, auto& status)
                                {
                                    if (status == state::live)
                                    {
                                        status = state::dead;
                                        nexthop->base::signal(tier::release, input::events::focus::set::off, seed);
                                    }
                                });
                            }
                            notify_focus_state(state::live, chain, seed.gear_id);
                        }
                    }
                    else // Build focus tree (we are in the middle of the focus tree).
                    {
                        auto& chain = get_chain(seed.gear_id);
                        if (seed.focus_type == solo::on)
                        {
                            auto exists = faux;
                            chain.foreach([&](auto& nexthop, auto& status)
                            {
                                if (nexthop == seed.item)
                                {
                                    status = state::live;
                                    exists = true;
                                }
                                else
                                {
                                    status = state::dead;
                                    nexthop->base::signal(tier::release, input::events::focus::set::off, seed);
                                }
                            });
                            if (!exists)
                            {
                                chain.next.push_back({ wptr{ seed.item }, state::live });
                            }
                        }
                        else // Group focus.
                        {
                            auto iter = std::find_if(chain.next.begin(), chain.next.end(), [&](auto& n){ return n.next_wptr.lock() == seed.item; });
                            if (iter == chain.next.end())
                            {
                                chain.next.push_back({ wptr{ seed.item }, state::live });
                            }
                            else
                            {
                                iter->status = state::live;
                                if (seed.gear_id) // Seal the || branches.
                                {
                                    chain.foreach([&](auto& /*nexthop*/, auto& status)
                                    {
                                        if (status == state::idle)
                                        {
                                            status = state::dead;
                                        }
                                    });
                                }
                            }
                            if (chain.active == state::live) // Stop group focusing if the branch is already active.
                            {
                                return;
                            }
                        }
                        notify_focus_state(state::live, chain, seed.gear_id);
                    }
                    if (auto parent = boss.base::parent())
                    {
                        seed.item = boss.This();
                        parent->base::riseup(tier::preview, input::events::focus::set::on, seed);
                    }
                };
                // pro::focus: Off focus to outside. Truncate the maximum path without branches.
                boss.LISTEN(tier::preview, input::events::focus::set::off, seed, memo)
                {
                    seed.treeid = treeid;
                    seed.digest = ++digest;
                    auto first_step = !seed.item; // No unfocused item yet. We are in the the first riseup iteration (pro::focus::off just called and catched the first plugin<pro::focus> owner). A focus leaf is not necessarily a visual tree leaf.
                    auto& chain = get_chain(seed.gear_id);
                    if (first_step)
                    {
                        if (chain.active == state::live)
                        {
                            boss.base::signal(tier::release, input::events::focus::set::off, seed);
                        }
                    }
                    else //if (!first_step)
                    {
                        auto focusable = node_type == mode::focused || node_type == mode::focusable;
                        auto last_step = chain.next.size() > 1 || focusable;
                        chain.foreach([&](auto& nexthop, auto& status)
                        {
                            if (nexthop == seed.item)
                            {
                                status = last_step ? state::dead : state::idle;
                            }
                        });
                        if (last_step) // Stop unfocusing on hub or focusable.
                        {
                            boss.bell::expire(tier::preview); // Don't let the hall send the event to the gate.
                            return;
                        }
                        notify_focus_state(state::idle, chain, seed.gear_id);
                    }
                    if (auto parent_ptr = boss.base::parent())
                    {
                        seed.item = boss.This();
                        parent_ptr->base::riseup(tier::preview, input::events::focus::set::off, seed);
                    }
                };
                // pro::focus: Initiate focus setting toward outside (used by gui and dtvt).
                boss.LISTEN(tier::request, input::events::focus::add, seed, memo)
                {
                    auto& chain = get_chain(seed.gear_id);
                    notify_focus_state(state::live, chain, seed.gear_id);
                    if (auto parent = boss.base::parent())
                    {
                        seed.item = boss.This();
                        parent->base::riseup(tier::preview, input::events::focus::set::on, seed);
                    }
                };
                // pro::focus: Initiate focus unsetting toward outside (used by gui and dtvt).
                boss.LISTEN(tier::request, input::events::focus::rem, seed, memo)
                {
                    auto& chain = get_chain(seed.gear_id);
                    auto boss_ptr = boss.This();
                    if (notify_focus_state(state::idle, chain, seed.gear_id))
                    {
                        if (auto parent_ptr = boss.base::parent())
                        {
                            seed.item = boss_ptr;
                            parent_ptr->base::riseup(tier::preview, input::events::focus::set::off, seed);
                        }
                    }
                };
                // pro::focus: Drop all downlinks (toward inside) from the boss and unfocus boss. Return dropped active gears.
                boss.LISTEN(tier::request, input::events::focus::cut, seed, memo)
                {
                    seed.treeid = treeid;
                    seed.digest = ++digest;
                    for (auto& [gear_id, chain] : gears)
                    {
                        chain.next.remove_if([&, gear_id = gear_id](auto& next) // Drop all downlinks (toward inside) from the boss. //todo Apple clang can't capture gear_id by ref.
                        {
                            auto match = next.next_wptr.lock() == seed.item;
                            if (match && gear_id && next.status == state::live)
                            {
                                seed.gear_id = gear_id;
                                seed.item->base::signal(tier::release, input::events::focus::set::off, seed);
                            }
                            return match;
                        });
                    }
                };
                // pro::focus: Switch all foci from the prev_ptr to the next_ptr (which must have a pro::focus on board).
                boss.LISTEN(tier::request, input::events::focus::hop, seed, memo)
                {
                    auto prev_ptr = seed.item;
                    auto next_ptr = seed.next;
                    for (auto& [gear_id, chain] : gears)
                    {
                        auto iter = chain.next.begin();
                        while (iter != chain.next.end())
                        {
                            auto& r = *iter++;
                            auto item_ptr = r.next_wptr.lock();
                            if (!item_ptr || item_ptr == next_ptr)
                            {
                                iter = chain.next.erase(iter);
                            }
                            else if (item_ptr == prev_ptr)
                            {
                                r.next_wptr = next_ptr;
                                if (gear_id && r.status == state::live)
                                {
                                    prev_ptr->base::signal(tier::release, input::events::focus::set::off, { .gear_id = gear_id, .treeid = treeid, .digest = ++digest });
                                    next_ptr->base::signal(tier::release, input::events::focus::set::on,  { .gear_id = gear_id, .treeid = treeid, .digest = ++digest });
                                }
                            }
                        }
                    }
                };
                // pro::focus: .
                boss.LISTEN(tier::request, e2::form::state::keybd::enlist, gear_id_list, memo)
                {
                    for (auto& [gear_id, chain] : gears)
                    {
                        if (gear_id && chain.active == state::live)
                        {
                            gear_id_list.push_back(gear_id);
                        }
                    }
                };
                // pro::focus: .
                boss.LISTEN(tier::request, e2::form::state::focus::count, gear_count, memo)
                {
                    gear_count = count;
                };
                // pro::focus: .
                boss.LISTEN(tier::request, e2::form::state::keybd::find, gear_test, memo)
                {
                    auto iter = gears.find(gear_test.first);
                    if (iter != gears.end() && iter->second.active == state::live)
                    {
                        gear_test.second++;
                    }
                };
                // pro::focus: .
                boss.LISTEN(tier::request, e2::form::state::keybd::next, gear_test, memo)
                {
                    auto iter = gears.find(gear_test.first);
                    if (iter != gears.end())
                    {
                        auto& chain = iter->second;
                        if (chain.active == state::live)
                        {
                            chain.foreach([&](auto& /*nexthop*/, auto& status)
                            {
                                if (status == state::live)
                                {
                                    gear_test.second++;
                                }
                            });
                        }
                    }
                };
                // pro::focus: .
                boss.LISTEN(tier::general, e2::form::proceed::functor, function, memo)
                {
                    for (auto& [gear_id, chain] : gears)
                    {
                        //todo revise
                        if (gear_id && chain.next.empty() && chain.active == state::live)
                        {
                            function(boss.This());
                        }
                    }
                };
                // pro::focus: .
                boss.LISTEN(tier::general, ui::e2::command::request::inputfields, inputfield_request, memo)
                {
                    if (auto iter = gears.find(inputfield_request.gear_id); iter != gears.end())
                    {
                        auto& chain = iter->second;
                        if (chain.active == state::live)
                        {
                            boss.base::signal(tier::release, ui::e2::command::request::inputfields, inputfield_request);
                        }
                    }
                };
            }
        };

        // pro: Mouse events.
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
            list mice; // mouse: List of active mice.
            bool omni; // mouse: Ability to accept all hover events (true) or only directly over the object (faux).
            si32 rent; // mouse: Active gears count.
            si32 full; // mouse: All gears count. Counting to keep the entire chain of links in the visual tree.
            si32 drag; // mouse: Bitfield of buttons subscribed to mouse drag.
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
                            if (auto gear_ptr = boss.bell::getref<hids>(gear.id))
                            {
                                gear_ptr->redirect_mouse_focus(parent);
                            }
                        });
                    }
                };
                // pro::mouse: Forward preview to all parents.
                boss.LISTEN(tier::preview, input::events::mouse::any, gear, memo)
                {
                    auto offset = boss.base::coor() + boss.base::intpad.corner();
                    gear.pass(tier::preview, boss.base::parent(), offset);

                    if (gear) gear.okay(boss);
                    else      boss.bell::expire(tier::preview);
                };
                // pro::mouse: Forward all not expired mouse events to all parents.
                boss.LISTEN(tier::release, input::events::mouse::any, gear, memo)
                {
                    if ((gear && !gear.captured()) || gear.cause == input::events::mouse::hover::enter.id || gear.cause == input::events::mouse::hover::leave.id)
                    {
                        auto offset = boss.base::coor() + boss.base::intpad.corner();
                        gear.pass(tier::release, boss.base::parent(), offset);
                    }
                };
                // pro::mouse: Amplify mouse hover on any button press.
                boss.LISTEN(tier::release, input::events::mouse::button::any, gear, memo)
                {
                    if (netxs::events::subevent(gear.cause, input::events::mouse::button::down::any.id)
                     || netxs::events::subevent(gear.cause, input::events::mouse::button::up::any.id))
                    {
                        boss.base::signal(tier::release, e2::form::state::hover, rent + gear.mouse::pressed_count);
                    }
                };
                // pro::mouse: Notify about change in number of mouse hovering clients.
                boss.LISTEN(tier::release, input::events::mouse::hover::any, gear, memo)
                {
                    if (gear.cause == input::events::mouse::hover::enter.id) // Notify when the number of clients is positive.
                    {
                        if (!full++)
                        {
                            soul = boss.This();
                        }
                        if (gear.direct<true>(boss.bell::id) || omni)
                        {
                            if (!rent++)
                            {
                                boss.base::signal(tier::release, e2::form::state::mouse, rent);
                            }
                            boss.base::signal(tier::release, e2::form::state::hover, rent);
                        }
                    }
                    else if (gear.cause == input::events::mouse::hover::leave.id) // Notify when the number of clients is zero.
                    {
                        if (gear.direct<faux>(boss.bell::id) || omni)
                        {
                            if (!--rent)
                            {
                                boss.base::signal(tier::release, e2::form::state::mouse, rent);
                            }
                            boss.base::signal(tier::release, e2::form::state::hover, rent);
                        }
                        if (!--full)
                        {
                            soul->base::strike();
                            soul.reset();
                        }
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
                    auto deed = boss.bell::protos(tier::release);
                    switch (deed)
                    {
                        default:
                        case e2::form::draggable::left     .id: draggable<hids::buttons::left     >(enabled); break;
                        case e2::form::draggable::right    .id: draggable<hids::buttons::right    >(enabled); break;
                        case e2::form::draggable::middle   .id: draggable<hids::buttons::middle   >(enabled); break;
                        case e2::form::draggable::xbutton1 .id: draggable<hids::buttons::xbutton1 >(enabled); break;
                        case e2::form::draggable::xbutton2 .id: draggable<hids::buttons::xbutton2 >(enabled); break;
                        case e2::form::draggable::leftright.id: draggable<hids::buttons::leftright>(enabled); break;
                    }
                };
            }
            void reset()
            {
                auto lock = boss.bell::sync();
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
                    //namespace bttn = input::events::mouse::button; //MSVC 16.9.4 don't get it
                    boss.LISTEN(tier::release, input::events::mouse::button::drag::start::_<Button>, gear, dragmemo[Button])
                    {
                        if (gear.capture(boss.bell::id))
                        {
                            boss.base::signal(tier::release, e2::form::drag::start::_<Button>, gear);
                            gear.dismiss();
                        }
                    };
                    boss.LISTEN(tier::release, input::events::mouse::button::drag::pull::_<Button>, gear, dragmemo[Button])
                    {
                        if (gear.captured(boss.bell::id))
                        {
                            boss.base::signal(tier::release, e2::form::drag::pull::_<Button>, gear);
                            gear.dismiss();
                        }
                    };
                    boss.LISTEN(tier::release, input::events::mouse::button::drag::cancel::_<Button>, gear, dragmemo[Button])
                    {
                        if (gear.captured(boss.bell::id))
                        {
                            boss.base::signal(tier::release, e2::form::drag::cancel::_<Button>, gear);
                            gear.setfree();
                            gear.dismiss();
                        }
                    };
                    boss.LISTEN(tier::general, input::events::halt, gear, dragmemo[Button])
                    {
                        if (gear.captured(boss.bell::id))
                        {
                            boss.base::signal(tier::release, e2::form::drag::cancel::_<Button>, gear);
                            gear.setfree();
                            gear.dismiss();
                        }
                    };
                    boss.LISTEN(tier::release, input::events::mouse::button::drag::stop::_<Button>, gear, dragmemo[Button])
                    {
                        if (gear.captured(boss.bell::id))
                        {
                            boss.base::signal(tier::release, e2::form::drag::stop::_<Button>, gear);
                            gear.setfree();
                            gear.dismiss();
                        }
                    };
                }
            }
        };

        // pro: Keyboard events.
        class keybd
            : public skill
        {
            using skill::boss,
                  skill::memo;

            netxs::sptr<std::unordered_map<text, wptr>> scripting_context_ptr; // hids: Script execution context: sptr<map<$object_name_str, $object_wptr>>.
            std::unordered_map<text, std::pair<std::list<netxs::sptr<text>>, bool>, qiew::hash, qiew::equal> handlers; // Map<chord, pair<list<shared_ptr<script>>, preview>>.
            std::unordered_map<id_t, time> last_key;
            si64 instance_id;
            text boss_name_str;

            auto _get_chord_list(qiew chord_str = {}) -> std::optional<std::invoke_result_t<decltype(input::key::kmap::chord_list), qiew>>
            {
                auto chords = input::key::kmap::chord_list(chord_str);
                if (chords.size())
                {
                    return std::optional{ chords };
                }
                else
                {
                    if (chord_str) log("%%Unknown key chord: '%chord%'", prompt::user, chord_str);
                    return std::optional<decltype(chords)>{};
                }
            }
            auto _get_chords(qiew chord_list_str)
            {
                auto chord_qiew_list = utf::split<true>(chord_list_str, " | ");
                if (chord_qiew_list.size())
                {
                    auto head = chord_qiew_list.begin();
                    auto tail = chord_qiew_list.end();
                    if (auto first_chord_list = _get_chord_list(utf::trim(*head++)))
                    {
                        auto chords = first_chord_list.value();
                        while (head != tail)
                        {
                            auto chord_qiew = *head++;
                            if (auto next_chord_list = _get_chord_list(chord_qiew))
                            {
                                auto& c = next_chord_list.value();
                                chords.insert(chords.end(), c.begin(), c.end());
                            }
                        }
                        return std::optional{ chords };
                    }
                }
                return _get_chord_list();
            }
            void _dispatch(hids& gear, bool preview_mode, qiew chord)
            {
                auto iter = handlers.find(chord);
                if (iter != handlers.end())
                {
                    auto& [scripts, run_preview] = iter->second;
                    if (!preview_mode || run_preview)
                    {
                        if (!scripting_context_ptr) // Restore scripting context.
                        {
                            scripting_context_ptr = ptr::shared<decltype(scripting_context_ptr)::element_type>();
                            std::swap(gear.scripting_context_ptr, scripting_context_ptr);
                            boss.base::riseup(tier::request, e2::runscript, gear, true);
                            std::swap(gear.scripting_context_ptr, scripting_context_ptr);
                        }
                        for (auto& script_ptr : scripts)
                        {
                            if (!gear.interrupt_key_proc)
                            {
                                auto temp_script_ptr = std::exchange(gear.script_ptr, script_ptr);
                                auto temp_scripting_context_ptr = std::exchange(gear.scripting_context_ptr, scripting_context_ptr);
                                //todo cache world_ptr
                                if (auto world_ptr = boss.base::signal(tier::general, e2::config::creator))
                                {
                                    world_ptr->base::signal(tier::preview, e2::runscript, gear);
                                }
                                gear.script_ptr = temp_script_ptr;
                                gear.scripting_context_ptr = temp_scripting_context_ptr;
                            }
                        }
                    }
                    else
                    {
                        gear.touched = instance_id;
                    }
                }
            }

        public:
            keybd(base&&) = delete;
            keybd(base& boss, text boss_name = {})
                : skill{ boss },
                  instance_id{ datetime::now().time_since_epoch().count() },
                  boss_name_str{ boss_name.empty() ? utf::concat("object"s, boss.id) : boss_name }
            {
                boss.LISTEN(tier::request, e2::runscript, gear)
                {
                    if (gear.scripting_context_ptr)
                    {
                        auto& scripting_context = *gear.scripting_context_ptr;
                        scripting_context[boss_name_str] = boss.This();
                    }
                };
                boss.LISTEN(tier::release, e2::form::state::focus::count, count)
                {
                    if (count == 0)
                    {
                        scripting_context_ptr.reset();
                    }
                };
                boss.LISTEN(tier::general, input::events::die, gear, memo)
                {
                    last_key.erase(gear.id);
                };
                boss.LISTEN(tier::release, input::events::keybd::key::any, gear, memo)
                {
                    gear.shared_event = gear.touched && gear.touched != instance_id;
                    auto& timecod = last_key[gear.id];
                    if (gear.timecod > timecod)
                    {
                        timecod = gear.timecod;
                        if (gear.payload == input::keybd::type::keypress)
                        {
                            gear.interrupt_key_proc = faux;
                            if (!gear.handled) _dispatch(gear, faux, input::key::kmap::any_key);
                            if (!gear.handled) _dispatch(gear, faux, gear.vkchord);
                            if (!gear.handled) _dispatch(gear, faux, gear.chchord);
                            if (!gear.handled) _dispatch(gear, faux, gear.scchord);
                        }
                    }
                    else
                    {
                        gear.set_handled();
                    }
                };
                boss.LISTEN(tier::preview, input::events::keybd::key::any, gear, memo)
                {
                    gear.shared_event = gear.touched && gear.touched != instance_id;
                    if (gear.payload == input::keybd::type::keypress)
                    {
                        if (!gear.touched && !gear.handled) _dispatch(gear, true, gear.vkchord);
                        if (!gear.touched && !gear.handled) _dispatch(gear, true, gear.chchord);
                        if (!gear.touched && !gear.handled) _dispatch(gear, true, gear.scchord);
                        if (!gear.touched && !gear.handled) _dispatch(gear, true, input::key::kmap::any_key);
                    }
                };
                //todo scripting
                //proc("Noop",           [&](hids& gear){ gear.set_handled(); gear.interrupt_key_proc = true; });
                //proc("DropAutoRepeat", [&](hids& gear){ if (gear.keystat == input::key::repeated) { gear.set_handled(); gear.interrupt_key_proc = true; }});
            }

            auto bind(qiew chord_str, auto&& scripts, bool preview = faux)
            {
                if (!chord_str) return;
                if (auto chord_list = _get_chords(chord_str))
                {
                    auto& chords = chord_list.value();
                    auto set = [&](netxs::sptr<text> script_ptr)
                    {
                        if (script_ptr && script_ptr->size())
                        {
                            for (auto& chord : chords)
                            {
                                auto& r = handlers[chord];
                                r.first.emplace_back(script_ptr);
                                r.second = preview;
                            }
                        }
                        else // Reset all bindings for chords.
                        {
                            for (auto& chord : chords)
                            {
                                handlers.erase(chord);
                            }
                        }
                    };
                    if constexpr (std::is_same_v<char, std::decay_t<decltype(scripts[0])>>) // The case it is a plain string.
                    {
                        auto script_ptr = ptr::shared(text{ scripts });
                        set(script_ptr);
                    }
                    else
                    {
                        for (auto& script_ptr : scripts)
                        {
                            set(script_ptr);
                        }
                    }
                }
            }
            auto bind(auto& bindings)
            {
                for (auto& r : bindings)
                {
                    bind(r.chord, r.scripts, r.preview);
                }
            }
            static auto load(xmls& config, qiew section)
            {
                auto bindings = input::key::keybind_list_t{};
                if (section)
                {
                    auto path = "/config/events/" + section.str() + "/key";
                    auto keybinds = config.list(path);
                    for (auto keybind_ptr : keybinds)
                    {
                        auto chord = config.expand(keybind_ptr);
                        auto preview = keybind_ptr->take("preview", faux);
                        auto script_ptr_list = keybind_ptr->list("script");
                        bindings.push_back({ .chord = chord, .preview = preview });
                        auto& rec = bindings.back();
                        //if constexpr (debugmode) log("chord=%% preview=%%", chord, (si32)preview);
                        for (auto script_ptr : script_ptr_list)
                        {
                            rec.scripts.push_back(ptr::shared(config.expand(script_ptr)));
                            //if constexpr (debugmode) log("  script=", *rec.scripts.back());
                        }
                    }
                }
                return bindings;
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
                    auto shadow = argb{0xFF000000};
                    auto bright = argb{0xFFffffff};

                    //todo optimize - don't fill the head and foot twice
                    auto area = parent_canvas.clip();
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

                        parent_canvas.core::fill(head, [&](cell& c){ c.bgc().mix(bright); });
                        parent_canvas.core::fill(foot, [&](cell& c){ c.bgc().mix(shadow); });

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

            robot robo; // fader: .
            cell& filler;
            span  fade;
            cell  c1;
            cell  c2;
            si32  transit;

        public:
            fader(base&&) = delete;
            fader(base& boss, cell& boss_filler, cell highlighted_state, span fade_out = 250ms, sptr tracking_object = {})
                : skill{ boss },
                   robo{ boss },
                 filler{ boss_filler },
                   fade{ fade_out },
                     c1{ boss_filler },
                     c2{ highlighted_state },
                transit{ 0 }
            {
                filler = c1;
                auto& root = tracking_object ? *tracking_object : boss;
                root.LISTEN(tier::release, e2::form::state::mouse, active, memo)
                {
                    robo.pacify();
                    if (active)
                    {
                        transit = 256;
                        filler = c2;
                        boss.base::deface();
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
                                filler.avg(c1, c2, transit);
                                boss.base::deface();
                            });
                        }
                        else
                        {
                            transit = 0;
                            filler = c1;
                            boss.base::deface();
                        }
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

            face bosscopy; // cache: Boss bitmap cache.
            bool usecache; // cacheL .
            si32 lucidity; // cacheL .

        public:
            cache(base&&) = delete;
            cache(base& boss, bool rendered = true)
                : skill{ boss },
                  usecache{ true },
                  lucidity{ 0xFF }
            {
                bosscopy.link(boss.bell::id);
                bosscopy.size(boss.base::size());
                boss.LISTEN(tier::preview, e2::form::prop::ui::cache, state, memo)
                {
                    usecache = state;
                };
                boss.LISTEN(tier::anycast, e2::form::prop::lucidity, value, memo)
                {
                    if (value < 0)
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
                if (rendered)
                {
                    boss.LISTEN(tier::release, e2::render::background::prerender, parent_canvas, memo)
                    {
                        if (!usecache) return;
                        if (boss.base::ruined())
                        {
                            bosscopy.wipe();
                            boss.base::ruined(faux);
                            boss.base::signal(tier::release, e2::render::background::any, bosscopy);
                        }
                        auto full = parent_canvas.full();
                        bosscopy.move(full.coor);
                        if (lucidity == 0xFF) parent_canvas.fill(bosscopy, cell::shaders::overlay);
                        else                  parent_canvas.fill(bosscopy, cell::shaders::transparent(lucidity));
                        bosscopy.move(dot_00);
                        boss.bell::expire(tier::release);
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
            vrgb cache; // acryl: Boxblur temp buffer.

        public:
            acryl(base&&) = delete;
            acryl(base& boss, si32 size = 3)
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
                boss.LISTEN(tier::release, e2::render::background::prerender, parent_canvas, memo)
                {
                    if (!alive) return;
                    parent_canvas.blur(width, cache, [&](cell& c){ c.alpha(0xFF); });
                };
            }
        };

        //todo deprecated: use form::shader instead
        // pro: Background highlighter.
        class light
            : public skill
        {
            using skill::boss,
                  skill::memo;

            bool highlighted = faux; // light: .
            argb title_fg_color = 0xFFffffff;

        public:
            light(base&&) = delete;
            light(base& boss)
                : skill{ boss }
            {
                boss.LISTEN(tier::release, e2::form::state::highlight, state, memo)
                {
                    highlighted = state;
                    boss.base::deface();
                };
                boss.LISTEN(tier::release, e2::render::background::prerender, parent_canvas, memo)
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

        // pro: UI-control shadow.
        class ghost
            : public skill
        {
            using skill::boss,
                  skill::memo;

            auto draw_shadow(face& canvas)
            {
                if (skin::globals().shadow_enabled)
                {
                    static auto shadow = netxs::misc::shadow<core>{};
                    if (!shadow.sync) shadow.generate(skin::globals().shadow_bias,
                                                      skin::globals().shadow_opacity,
                                                      skin::globals().shadow_blur * 2,
                                                      skin::globals().shadow_offset,
                                                      dot_21,
                                                      [](cell& c, auto a){ c.alpha(a); });
                    shadow.render(canvas, canvas.area(), rect{ .size = boss.base::size() }, cell::shaders::blend);
                }
            }

        public:
            ghost(base&&) = delete;
            ghost(base& boss)
                : skill{ boss }
            {
                boss.LISTEN(tier::release, e2::render::background::prerender, parent_canvas, memo)
                {
                    draw_shadow(parent_canvas);
                };
                //test
                //boss.LISTEN(tier::release, input::events::mouse::scroll::any, gear)
                //{
                //    boss.base::deface();
                //};
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
            notes(base& boss, view data = {}, dent wrap = { si32max })
                : skill{ boss },
                  note { data }
            {
                boss.LISTEN(tier::release, input::events::mouse::hover::enter, gear, memo, (wrap, full = wrap.l == si32max))
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

        // pro: Realtime statistics.
        class debug
            : public skill
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
            X(key_chord    , "key chord"        ) \
            X(key_state    , "key state"        ) \
            X(key_payload  , "key type"         ) \
            X(ctrl_state   , "controls"         ) \
            X(k            , "k"                ) \
            X(mouse_pos    , "mouse coord"      ) \
            X(mouse_wheelsi, "wheel steps"      ) \
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

            enum prop
            {
                #define X(a, b) a,
                prop_list
                #undef X
            };

            static constexpr auto description = std::to_array(
            {
                #define X(a, b) b##sv,
                prop_list
                #undef X
            });
            #undef prop_list

            using skill::boss,
                  skill::memo;

        public:
            using skill::skill; // Inherits ctors.

            page  status;

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

            void update(bool focus_state)
            {
                status[prop::last_event] = "focus";
                status[prop::focused] = focus_state ? "active" : "lost";
            }
            void update(twod new_size)
            {
                status[prop::last_event] = "size";
                status[prop::win_size] = utf::concat(new_size.x, " x ", new_size.y);
            }
            void update(span watch, si32 delta)
            {
                track.output = watch;
                track.frsize = delta;
                track.totals+= delta;
            }
            void update(time timestamp)
            {
                track.render = datetime::now() - timestamp;
            }
            void output(face& canvas)
            {
                status[prop::render_ns] = utf::adjust(utf::format(track.output.count()), 11, " ", true) + "ns";
                status[prop::proceed_ns] = utf::adjust(utf::format (track.render.count()), 11, " ", true) + "ns";
                status[prop::frame_size] = utf::adjust(utf::format(track.frsize), 7, " ", true) + " bytes";
                status[prop::total_size] = utf::format(track.totals) + " bytes";
                track.number++;
                status.reindex();
                auto ctx = canvas.change_basis(canvas.area());
                canvas.output(status, cell::shaders::contrast);
            }
            void stop()
            {
                track = {};
                memo.clear();
            }
            void start()
            {
                status.style.wrp(wrap::off).jet(bias::left).rlf(feed::rev).mgl(4);
                status.current().locus.cup(dot_00).cnl(2);

                auto maxlen = 0_sz;
                for (auto& desc : description)
                {
                    maxlen = std::max(maxlen, desc.size());
                }
                auto attr = si32{ 0 };
                auto coder = escx{};
                for (auto& desc : description)
                {
                    status += coder.add(" ", utf::adjust(desc, maxlen, " ", true), " ").idx(attr++).nop().nil().eol();
                    coder.clear();
                }

                boss.LISTEN(tier::general, e2::config::fps, fps, memo)
                {
                    status[prop::frame_rate] = std::to_string(fps);
                    boss.base::strike();
                };
                boss.base::signal(tier::general, e2::config::fps, -1);
                boss.LISTEN(tier::release, e2::area, new_area, memo)
                {
                    update(new_area.size);
                };
                boss.LISTEN(tier::release, e2::conio::mouse, m, memo)
                {
                    status[prop::last_event] = "mouse";
                    status[prop::mouse_pos ] = (m.coordxy.x < 10000 ? std::to_string(m.coordxy.x) : "-") + " : " +
                                               (m.coordxy.y < 10000 ? std::to_string(m.coordxy.y) : "-") ;

                    auto m_buttons = std::bitset<8>(m.buttons);
                    for (auto i = 0; i < hids::numofbuttons; i++)
                    {
                        auto& state = status[prop::mouse_btn_1 + i];
                        state = m_buttons[i] ? "pressed" : "idle   ";
                    }

                    if constexpr (debugmode)
                    {
                        status[prop::k] = utf::concat(netxs::_k0, " ",
                                                      netxs::_k1, " ",
                                                      netxs::_k2, " ",
                                                      netxs::_k3);
                    }
                    status[prop::mouse_wheeldt] = m.wheelfp ? (m.wheelfp < 0 ? ""s : " "s) + std::to_string(m.wheelfp) : " -- "s;
                    status[prop::mouse_wheelsi] = m.wheelsi ? (m.wheelsi < 0 ? ""s : " "s) + std::to_string(m.wheelsi) : m.wheelfp ? " 0 "s : " -- "s;
                    status[prop::mouse_hzwheel] = m.hzwheel ? "active" : "idle  ";
                    status[prop::mouse_vtwheel] = (m.wheelfp && !m.hzwheel) ? "active" : "idle  ";
                    status[prop::ctrl_state   ] = "0x" + utf::to_hex(m.ctlstat);
                };
                boss.LISTEN(tier::release, e2::conio::focus, f, memo)
                {
                    status[prop::focused] = f.state ? "focused  " : "unfocused";
                };
                boss.LISTEN(tier::release, e2::conio::keybd, k, memo)
                {
                    static constexpr auto kstate = std::to_array({ "idle    ", "pressed ", "repeated" });
                    status[prop::last_event   ] = "keybd";
                    status[prop::key_state    ] = kstate[k.keystat % 3];
                    status[prop::ctrl_state   ] = "0x" + utf::to_hex(k.ctlstat );
                    status[prop::key_code     ] = "0x" + utf::to_hex(k.virtcod );
                    status[prop::key_scancode ] = "0x" + utf::to_hex(k.scancod );
                    status[prop::key_payload  ] = k.payload == input::keybd::type::keypress ? "keypress"
                                                : k.payload == input::keybd::type::keypaste ? "keypaste"
                                                : k.payload == input::keybd::type::imeanons ? "IME composition"
                                                : k.payload == input::keybd::type::imeinput ? "IME input"
                                                : k.payload == input::keybd::type::kblayout ? "keyboard layout" : "unknown payload";
                    if (k.vkchord.length())
                    {
                        auto t = text{};
                        if (k.vkchord.size() && k.keystat != input::key::repeated)
                        {
                            auto vkchord =     input::key::kmap::to_string(k.vkchord, faux);
                            auto scchord =     input::key::kmap::to_string(k.scchord, faux);
                            auto chchord =     input::key::kmap::to_string(k.chchord, faux);
                            auto gen_vkchord = input::key::kmap::to_string(k.vkchord, true);
                            auto gen_chchord = input::key::kmap::to_string(k.chchord, true);
                            //log("Keyboard chords: %%  %%  %%", utf::buffer_to_hex(gear.vkchord), utf::buffer_to_hex(gear.scchord), utf::buffer_to_hex(gear.chchord),
                            if (vkchord.size()) t += (t.size() ? "  " : "") + (vkchord == gen_vkchord ? vkchord : gen_vkchord + "  " + vkchord);
                            if (chchord.size()) t += (t.size() ? "  " : "") + (chchord == gen_chchord ? chchord : gen_chchord + "  " + chchord);
                            if (scchord.size()) t += (t.size() ? "  " : "") + scchord;
                        }
                        else if (k.cluster.length()) //todo revise
                        {
                            for (byte c : k.cluster)
                            {
                                     if (c <  0x20) t += "^" + utf::to_utf_from_code(c + 0x40);
                                else if (c == 0x7F) t += "\\x7F";
                                else if (c == 0x20) t += "\\x20";
                                else                t.push_back(c);
                            }
                        }
                        if (t.size()) status[prop::key_chord] = t;
                    }
                };
            }
        };

        // pro: Lua scripting.
        class luafx
            : public skill
        {
            using skill::boss,
                  skill::memo;
        public:
            template<class T>
            using fxmap = std::unordered_map<text, std::function<void(T&, luafx&)>>;

            lua_State* lua;

            luafx(base&&) = delete;
            luafx(base& boss)
                : skill{ boss },
                  lua{ boss.indexer.lua }
            { }

            template<class T>
            auto activate(fxmap<T>& proc_map)
            {
                if (!lua) return;
                auto& owner = dynamic_cast<T&>(boss);
                boss.LISTEN(tier::release, e2::luafx, lua)
                {
                    auto fx_name = ::lua_tostring(lua, lua_upvalueindex(2)); // Get fx name.
                    auto iter = proc_map.find(fx_name);
                    if (iter != proc_map.end())
                    {
                        auto& fx = iter->second;
                        fx(owner, *this); // After call, all values in the stack will be returned as a result.
                        boss.bell::expire(tier::release); // Do not pass from ui::host to vtm::hall.
                    }
                };
            }
            template<class T>
            auto get_args_or(si32 idx, T fallback = {})
            {
                static constexpr auto is_string_v = requires{ static_cast<const char*>(fallback.data()); };
                static constexpr auto is_cstring_v = requires{ static_cast<const char*>(fallback); };

                auto type = ::lua_type(lua, idx);
                if (type != LUA_TNIL)
                {
                         if constexpr (std::is_same_v<std::decay_t<T>, bool>) return (T)::lua_toboolean(lua, idx);
                    else if constexpr (is_string_v || is_cstring_v)           return ::lua_torawstring(lua, idx);
                    else if constexpr (std::is_integral_v<T>)                 return (T)::lua_tointeger(lua, idx);
                    else if constexpr (std::is_floating_point_v<T>)           return (T)::lua_tonumber(lua, idx);
                    else if constexpr (std::is_same_v<std::decay_t<T>, twod>) return twod{ ::lua_tointeger(lua, idx), ::lua_tointeger(lua, idx + 1) };
                }
                if constexpr (is_string_v || is_cstring_v) return text{ fallback };
                else                                       return fallback;
            }
            auto log_context()
            {
                log("%%vtm context:", prompt::lua);
                ::lua_getglobal(lua, "vtm");
                ::lua_pushnil(lua);
                while (::lua_next(lua, -2))
                {
                    auto var = ::lua_torawstring(lua, -2);
                    auto val = ::lua_torawstring(lua, -1, true);
                    if (val.size()) log("%%vtm.%name% = %value%", prompt::pads, var, val);
                    ::lua_pop(lua, 1); // Pop val.
                }
                ::lua_pop(lua, 1); // Pop table "vtm".
            }
            auto set_object(sptr object_ptr, qiew object_name)
            {
                if (object_ptr)
                {
                    if (::lua_getglobal(lua, "vtm") != LUA_TTABLE) // Push "vtm" table to stack.
                    {
                        ::lua_pop(lua, 1); // Pop if it is a non-table.
                        ::lua_newtable(lua); // Create and push new "vtm.*" global table.
                        ::lua_setglobal(lua, "vtm"); // Set global var "vtm". Pop "vtm".
                        ::lua_getglobal(lua, "vtm"); // Push "vtm" table again to stack.
                    }
                    ::lua_pushstring(lua, object_name.data()); // Push vtm.* var name (key).
                    ::lua_pushlightuserdata(lua, object_ptr.get()); // Object ptr (val).
                    ::luaL_setmetatable(lua, "vtmmetatable"); // Set the metatable for -1 userdata.
                    ::lua_settable(lua, -3); // Set vtm.key=val. Pop key+val.
                    ::lua_pop(lua, 1); // Pop table "vtm".
                }
            }
            template<class T = base>
            auto get_object(const char* object_name)
            {
                ::lua_getglobal(lua, "vtm");
                ::lua_pushstring(lua, object_name);
                ::lua_gettable(lua, -2);
                auto object_ptr = dynamic_cast<T*>((base*)::lua_touserdata(lua, -1));
                ::lua_pop(lua, 2); // Pop "vtm" and "object_name".
                return object_ptr;
            }
            auto push_value(auto& v)
            {
                using T = decltype(v);
                static constexpr auto is_string_v = requires{ (const char*)v.data(); };
                static constexpr auto is_cstring_v = requires{ (const char*)v[0]; };
                     if constexpr (std::is_same_v<std::decay_t<T>, bool>) ::lua_pushboolean(lua, v);
                else if constexpr (is_string_v)                           ::lua_pushlstring(lua, v.data(), v.size());
                else if constexpr (is_cstring_v)                          ::lua_pushstring(lua, v);
                else if constexpr (std::is_integral_v<T>)                 ::lua_pushinteger(lua, v);
                else if constexpr (std::is_floating_point_v<T>)           ::lua_pushnumber(lua, v);
                else if constexpr (std::is_pointer_v<T>)                  ::lua_pushlightuserdata(lua, v);
                else if constexpr (debugmode) throw;
            }
            void set_return(auto... args)
            {
                ::lua_settop(lua, 0);
                (push_value(args), ...);
            }
            auto args_count()
            {
                return ::lua_gettop(lua);
            }
            auto run_with_gear(auto proc)
            {
                auto gear_ptr = get_object<hids>("gear");
                auto ok = !!gear_ptr;
                if (ok)
                {
                    auto& gear = *gear_ptr;
                    proc(gear);
                }
                set_return(ok);
            }
            auto read_args(auto add_item)
            {
                if (!lua_istable(lua, -1)) return;
                ::lua_pushnil(lua); // Push prev key.
                while (::lua_next(lua, -2)) // Table is in the stack at index -2. { "<item " + text{ table } + " />" }
                {
                    auto key = ::lua_torawstring(lua, -2);
                    if (!key.empty()) // Allow stringable keys only.
                    {
                        auto val = ::lua_torawstring(lua, -1);
                        if (val.empty() && lua_istable(lua, -1)) // Extract item list.
                        {
                            ::lua_pushnil(lua); // Push prev key.
                            while (::lua_next(lua, -2)) // Table is in the stack at index -2. { "<key="key2=val2"/>" }
                            {
                                auto val2 = ::lua_torawstring(lua, -1);
                                auto key2_type = ::lua_type(lua, -2);
                                if (key2_type != LUA_TSTRING) // key2 is integer index.
                                {
                                    add_item(key, val2);
                                }
                                else
                                {
                                    auto key2 = ::lua_torawstring(lua, -2);
                                    add_item(key, utf::concat(key2, '=', val2));
                                }
                                ::lua_pop(lua, 1); // Pop val2.
                            }
                        }
                        else
                        {
                            add_item(key, val);
                        }
                    }
                    ::lua_pop(lua, 1); // Pop val.
                }
            }
            auto run_script(auto& script_body, auto& scripting_context)
            {
                log("  script body: ", ansi::hi(script_body));
                for (auto& [object_name, object_wptr] : scripting_context)
                {
                    if (auto object_ptr = object_wptr.lock())
                    {
                        set_object(object_ptr, object_name);
                    }
                }
                log_context();
                ::lua_settop(lua, 0);
                auto error = ::luaL_loadbuffer(lua, script_body.data(), script_body.size(), "script body")
                          || ::lua_pcall(lua, 0, 0, 0);
                auto result = text{};
                if (error)
                {
                    result = ::lua_tostring(lua, -1);
                    log("%%%msg%", prompt::lua, ansi::err(result));
                    ::lua_pop(lua, 1);  // Pop error message from stack.
                }
                else if (::lua_gettop(lua))
                {
                    result = ::lua_torawstring(lua, -1);
                    ::lua_settop(lua, 0);
                }
                //todo optimize
                ::lua_pushnil(lua);
                ::lua_setglobal(lua, "vtm"); // Wipe global context.
                return result;
            }
        };
    }

    // console: Lua scripting.
    struct luna
    {
        lua_State* lua; // luna: .

        static auto vtmlua_log(lua_State* lua)
        {
            auto n = ::lua_gettop(lua);
            auto crop = text{};
            for (auto i = 1; i <= n; i++)
            {
                auto t = ::lua_type(lua, i);
                switch (t)
                {
                    case LUA_TBOOLEAN:
                    case LUA_TNUMBER:
                    case LUA_TSTRING: crop = ::lua_torawstring(lua, i); break;
                    default:          crop = "<" + text{ ::lua_typename(lua, t) } + ">"; break;
                }
            }
            log("", crop);
            return 0;
        }
        static auto vtmlua_call(lua_State* lua) // UpValue[1]: Object_ptr. UpValue[2]: Function_name.
        {
            // Stack:
            //      lua_upvalueindex(1): Get Object_ptr.
            //      lua_upvalueindex(2): Fx name.
            //      1. args:        ...
            //      2.     :        ...
            if (auto object_ptr = (base*)::lua_touserdata(lua, lua_upvalueindex(1))) // Get Object_ptr.
            {
                object_ptr->base::signal(tier::release, e2::luafx, lua);
            }
            return ::lua_gettop(lua);
        }
        static auto vtmlua_index(lua_State* lua)
        {
            // Stack:
            //      1. userdata (or table).
            //      2. fx name (keyname).
            ::lua_pushcclosure(lua, vtmlua_call, 2);
            return 1;
        }
        static auto vtmlua_tostring(lua_State* lua)
        {
            auto crop = text{};
            if (auto object_ptr = (base*)::lua_touserdata(lua, -1)) // Get Object_ptr.
            {
                crop = utf::concat("<object:", object_ptr->id, ">");
            }
            else crop = "<object>";
            ::lua_pushstring(lua, crop.data());
            return 1;
        }

        luna()
            : lua{ ::luaL_newstate() }
        {
            ::luaL_openlibs(lua);
            ::lua_pushcclosure(lua, vtmlua_log, 0);
            ::lua_setglobal(lua, "log");
            static auto metalist = std::to_array<luaL_Reg>({{ "__index", vtmlua_index },
                                                            { "__tostring", vtmlua_tostring },
                                                            { nullptr, nullptr }});
            ::luaL_newmetatable(lua, "vtmmetatable"); // Create a new metatable in registry and push it to the stack.
            ::luaL_setfuncs(lua, metalist.data(), 0); // Assign metamethods for the table which at the top of the stack.
        }
        ~luna()
        {
            if (lua) ::lua_close(lua);
        }
    };

    auto& tui_domain()
    {
        static auto scripting = luna{};
        static auto indexer = netxs::events::auth{ scripting.lua, e2::config::fps.id, e2::timer::tick.id };
        return indexer;
    }

    // controls: base UI element.
    template<class T>
    class form
        : public base
    {
        std::map<id_t, subs> memomap; // form: Token set for dependent subscriptions.

    public:
        auto This() { return base::This<T>(); }
        template<class TT = T, class ...Args>
        static auto ctor(Args&&... args)
        {
            auto item = ui::tui_domain().template create<TT>(std::forward<Args>(args)...);
            return item;
        }
        // form: Set control as root.
        auto isroot(bool isroot, si32 ofkind = base::client)
        {
            base::root(isroot);
            base::kind(ofkind);
            return This();
        }
        // form: Attach a plugin of the specified type and return self.
        template<class S, class ...Args>
        auto plugin(Args&&... args)
        {
            base::plugin<S>(std::forward<Args>(args)...);
            return This();
        }
        // form: Detach the specified plugin and return self.
        template<class S>
        auto unplug()
        {
            base::unplug<S>();
            return This();
        }
        // form: Return plugin reference of specified type. Add the specified plugin (using specified args) if it is missing.
        template<class S, class ...Args>
        auto& plugins(Args&&... args)
        {
            return base::plugin<S>(std::forward<Args>(args)...);
        }
        // form: Fill object region using parametrized fx.
        template<auto Tier = tier::release, auto RenderOrder = e2::render::background::any>
        auto _shader(cell fx)
        {
            LISTEN(tier::release, RenderOrder, parent_canvas, -, (fx))
            {
                parent_canvas.fill(cell::shaders::fusefull(fx));
            };
            return This();
        }
        // form: Fill object region using parametrized fx.
        template<auto Tier = tier::release, auto RenderOrder = e2::render::background::any, class Fx, class Event = noop, bool fixed = std::is_same_v<Event, noop>>
        auto shader(Fx&& fx, Event sync = {}, sptr source_ptr = {}, netxs::sptr<subs> tokens_ptr = {})
        {
            static constexpr auto is_cell = std::is_same_v<cell, std::decay_t<Fx>>;
            auto& tokens = tokens_ptr ? *tokens_ptr : bell::sensors;
            if constexpr (fixed)
            {
                if constexpr (is_cell && std::is_reference_v<Fx>)
                {
                    LISTEN(tier::release, RenderOrder, parent_canvas, tokens)
                    {
                             if (fx.xy())   parent_canvas.fill(cell::shaders::fusefull(fx));
                        else if (fx.link()) parent_canvas.fill(cell::shaders::onlyid(fx.link()));
                    };
                }
                else
                {
                    LISTEN(tier::release, RenderOrder, parent_canvas, tokens, (fx))
                    {
                        parent_canvas.fill(fx);
                    };
                }
            }
            else
            {
                auto param_ptr = ptr::shared(Event::param());
                auto& param = *param_ptr;
                auto& source = source_ptr ? *source_ptr : *this;
                source.base::signal(tier::request, sync, param);
                source.LISTEN(Tier, sync, new_value, tokens, (param_ptr))
                {
                    param = new_value;
                    base::deface();
                };
                if constexpr (is_cell) fx.link(bell::id);
                LISTEN(tier::release, RenderOrder, parent_canvas, tokens, (fx))
                {
                    static constexpr auto is_func = requires{ fx(parent_canvas, param, *this); };
                    static constexpr auto is_fade = requires{ fx[param]; };
                    if (param)
                    {
                             if constexpr (is_func) fx(parent_canvas, param, *this);
                        else if constexpr (is_cell) parent_canvas.fill(cell::shaders::fuseid(fx));
                        else if constexpr (is_fade) parent_canvas.fill(fx[param]);
                        else                        parent_canvas.fill(fx);
                    }
                };
            }
            return This();
        }
        // form: Set a static color (transparent for mouse events).
        auto colors(cell brush)
        {
            return _shader(brush);
        }
        // form: Set a static color (transparent for mouse events).
        auto colors(argb fg_color, argb bg_color)
        {
            auto brush = cell{ whitespace }.fgc(fg_color).bgc(bg_color);
            return _shader(brush);
        }
        // form: Make the form visible to the mouse.
        auto active(cell brush = {})
        {
            brush.txt(whitespace).link(bell::id);
            return _shader(brush);
        }
        // form: Make the form visible to the mouse.
        auto active(argb fg_color, argb bg_color)
        {
            auto brush = cell{ whitespace }.link(bell::id).fgc(fg_color).bgc(bg_color);
            return _shader(brush);
        }
        // form: Invoke an arbitrary functor(self/*This/boss) in place.
        template<class P>
        auto invoke(P functor)
        {
            auto backup = This();
            functor(*backup);
            return backup;
        }
        // form: Attach a homeless branch and return self.
        template<class ...Args>
        auto branch(Args&&... args)
        {
            auto backup = This();
            backup->T::attach(std::forward<Args>(args)...);
            return backup;
        }
        // form: UI-control will be detached along with the master.
        auto depend(sptr master_ptr)
        {
            auto& master_inst = *master_ptr;
            master_inst.LISTEN(tier::release, e2::form::upon::vtree::detached, parent_ptr, memomap[master_inst.id])
            {
                auto backup = This();
                memomap.erase(master_inst.id);
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
            auto backup = This();
            auto arg_value = data_src_sptr->base::signal(tier::request, Property{});
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
                    boss_ptr->replace(old_item, new_item);
                    boss_ptr->base::reflow();
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
            auto backup = This();
            auto property_value = base::signal(tier::request, BackendProp{});
            base::signal(tier::anycast, FrontendProp{}, property_value);
            LISTEN(tier::release, BackendProp{}, property_value)
            {
                this->base::signal(tier::anycast, FrontendProp{}, property_value);
            };
            return backup;
        }
        auto limits(twod new_min_sz = -dot_11, twod new_max_sz = -dot_11)
        {
            base::limits(new_min_sz, new_max_sz);
            return This();
        }
        auto alignment(bind new_atgrow, bind new_atcrop = { snap::none, snap::none })
        {
            base::alignment(new_atgrow, new_atcrop);
            return This();
        }
        auto setpad(dent new_intpad, dent new_extpad = {})
        {
            base::setpad(new_intpad, new_extpad);
            return This();
        }
        auto nested_context(auto& parent_canvas)
        {
            auto basis = rect{ dot_00, base::region.size } - base::intpad;
            auto context = parent_canvas.change_basis(basis, true);
            return context;
        }

        form()
            : base{ ui::tui_domain() }
        {
            plugins<pro::mouse>();
        }
    };

    // controls: Splitter.
    class fork
        : public form<fork>
    {
        rect griparea; // fork: Resizing grip region.
        axis rotation; // fork: Fork orientation.
        si32 fraction; // fork: Ratio between objects.
        bool adaptive; // fork: Fixed ratio.

        std::list<sptr>::iterator object_1 = base::subset.end(); // fork: 1st object.
        std::list<sptr>::iterator object_2 = base::subset.end(); // fork: 2nd object.
        std::list<sptr>::iterator splitter = base::subset.end(); // fork: Resizing grip object.
        auto xpose(twod p)
        {
            return rotation == axis::X ? p : twod{ p.y, p.x };
        }
        void _set_grip_width(si32 grip_width)
        {
            griparea.size = xpose({ std::max(0, grip_width), 0 });
        }
        void _config(axis orientation, si32 grip_width, si32 s1 = 1, si32 s2 = 1)
        {
            rotation = orientation;
            _set_grip_width(grip_width);
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
                    if (object_1 != base::subset.end())
                    if (auto& o = *object_1)
                    {
                        o->base::recalc(region_1);
                        split_x = size1_x;
                        newsz_y = size1_y;
                    }
                    size2_x = limit_x - split_x;
                    size2_y = newsz_y;
                    coor2_x = split_x + size3_x;
                    coor2_y = 0;
                    auto test_size2 = region_2.size;
                    if (object_2 != base::subset.end())
                    if (auto& o = *object_2)
                    {
                        o->base::recalc(region_2);
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
            if (splitter != base::subset.end())
            if (auto& o = *splitter)
            {
                o->base::recalc(region_3);
            }
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
            if (object_1 != base::subset.end())
            if (auto& o = *object_1) o->base::notify(region_1);
            if (object_2 != base::subset.end())
            if (auto& o = *object_2) o->base::notify(region_2);
            if (splitter != base::subset.end())
            if (auto& o = *splitter) o->base::notify(region_3);
            adaptive = faux;
        }

    public:
        fork(axis orientation = axis::X, si32 grip_width = 0, si32 s1 = 1, si32 s2 = 1)
            : rotation{},
              fraction{},
              adaptive{}
        {
            _config(orientation, grip_width, s1, s2);
            LISTEN(tier::preview, e2::form::layout::swarp, warp)
            {
                adaptive = true; // Adjust the grip ratio on coming resize.
                this->bell::expire(tier::preview, true);
            };
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                if (auto context = form::nested_context(parent_canvas))
                {
                    if (splitter != base::subset.end())
                    if (auto& o = *splitter) o->render(parent_canvas);
                    if (object_1 != base::subset.end())
                    if (auto& o = *object_1) o->render(parent_canvas);
                    if (object_2 != base::subset.end())
                    if (auto& o = *object_2) o->render(parent_canvas);
                }
            };
        }

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
        auto set_grip_width(si32 new_grip_width)
        {
            _set_grip_width(new_grip_width);
            base::reflow();
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
        auto get_config()
        {
            return std::tuple{ rotation, griparea, fraction };
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
            if (splitter != base::subset.end())
            if (auto& o = *splitter)
            {
                auto delta = std::max(dot_11, griparea.size) * xpose({ step, 0 });
                o->base::signal(tier::preview, e2::form::upon::changed, delta);
            }
        }
        // fork: .
        auto attach(slot Slot, auto item_ptr)
        {
            if (Slot == slot::_1)
            {
                if (object_1 != base::subset.end())
                if (auto& o = *object_1)
                {
                    remove(o);
                }
                base::_attach(item_ptr);
                object_1 = item_ptr->holder;
            }
            else if (Slot == slot::_2)
            {
                if (object_2 != base::subset.end())
                if (auto& o = *object_2)
                {
                    remove(o);
                }
                base::_attach(item_ptr);
                object_2 = item_ptr->holder;
            }
            else if (Slot == slot::_I)
            {
                if (splitter != base::subset.end())
                if (auto& o = *splitter)
                {
                    remove(o);
                }
                base::_attach(item_ptr);
                splitter = item_ptr->holder;
                item_ptr->LISTEN(tier::preview, e2::form::upon::changed, delta, item_ptr->relyon)
                {
                    auto split = xpose(griparea.coor + delta).x;
                    auto limit = xpose(base::size() - griparea.size).x;
                    fraction = netxs::divround(max_ratio * split, limit);
                    this->base::reflow();
                };
            }
            item_ptr->base::signal(tier::release, e2::form::upon::vtree::attached, This());
            return item_ptr;
        }
        // fork: Remove nested object by it's ptr.
        void remove(sptr item_ptr) override
        {
                 if (object_1 == item_ptr->holder) object_1 = base::subset.end();
            else if (object_2 == item_ptr->holder) object_2 = base::subset.end();
            else if (splitter == item_ptr->holder) splitter = base::subset.end();
            base::remove(item_ptr);
        }
        // fork: .
        auto get(slot Slot)
        {
                 if (Slot == slot::_1 && object_1 != base::subset.end()) return *object_1;
            else if (Slot == slot::_2 && object_2 != base::subset.end()) return *object_2;
            else if (Slot == slot::_I && splitter != base::subset.end()) return *splitter;
            else return sptr{};
        }
    };

    // controls: Vertical/horizontal list.
    class list
        : public form<list>
    {
        bool updown; // list: List orientation, true: vertical(default), faux: horizontal.
        sort lineup; // list: Attachment order.

    protected:
        // list: .
        void deform(rect& new_area) override
        {
            auto& object_area = new_area;
            auto& new_size = object_area.size;
            auto& height = object_area.coor[updown];
            auto& y_size = new_size[updown];
            auto& x_size = new_size[1 - updown];
            auto  x_temp = x_size;
            auto start = height;
            auto meter = [&]
            {
                height = start;
                for (auto& object : base::subset)
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
            meter(); if (base::subset.size() > 1 && x_temp != x_size) meter();
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
            for (auto& object : base::subset)
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
        list(axis orientation = axis::Y, sort attach_order = sort::forward)
            : updown{ orientation == axis::Y },
              lineup{ attach_order }
        {
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                if (auto context = form::nested_context(parent_canvas))
                {
                    auto basis = parent_canvas.full();
                    auto frame = parent_canvas.clip();
                    auto min_y = frame.coor[updown] - basis.coor[updown];
                    auto max_y = frame.size[updown] + min_y;
                    auto bound = [xy = updown](auto& o){ return o ? o->base::region.coor[xy] + o->base::region.size[xy] : -dot_mx.y; };
                    //todo optimize for large lists
                    //todo adapt it for std::list (use stored iterator)
                    //auto start = std::ranges::lower_bound(base::subset, min_y, {}, bound);
                    auto start = base::subset.begin();
                    while (start != base::subset.end())
                    {
                        if (auto& object = *start++)
                        {
                            object->render(parent_canvas);
                            if (!object->base::hidden && bound(object) >= max_y) break;
                        }
                    }
                }
            };
        }
    };

    // controls: 2D grid.
    class grid
        : public form<grid>
    {
        struct elem
        {
            twod coor; // elem: Grid cell coordinates for placing the object.
            twod span; // elem: The number of adjacent grid cells occupied by the object.
            rect area; // elem: Object slot.
            bool done; // elem: Object resized.
        };
        struct cell
        {
            twod size; // Cell size.
            si32 span; // Cell span.
        };
        template<bool SetInner = faux>
        auto cellsz(twod coor, twod span)
        {
            auto size = coor + span;
            auto x = std::accumulate(widths.begin() + coor.x, widths.begin() + size.x, 0, [](auto x, auto& w){ return x += w.size.x; });
            auto y = std::accumulate(widths.begin() + coor.y, widths.begin() + size.y, 0, [](auto y, auto& w){ return y += w.size.y; });
            return twod{ x, y };
        }
        std::vector<cell> widths;  // grid: Grid cell metrics.
        std::list<elem>   blocks;  // grid: Geometry of stored objects.

    protected:
        // grid: .
        void deform(rect& new_area) override
        {
            widths.clear();
            auto m = dot_00;
            auto first_run = true;
            auto recalc = [&](auto object_iter, auto tail2, auto elem_iter)
            {
                auto changed = faux;
                while (object_iter != tail2)
                {
                    auto& object = *(*object_iter++);
                    auto& elem = *elem_iter++;
                    if (elem.span.x < 1 || elem.span.y < 1) continue;
                    auto dimension = elem.coor + elem.span;
                    auto max_len = std::max(dimension.x, dimension.y);
                    if (max_len > (si32)widths.size())
                    {
                        m = std::max(m, dimension);
                        widths.resize(max_len);
                    }
                    auto area_size = cellsz<true>(elem.coor, elem.span);
                    if (first_run)
                    {
                        widths[elem.coor.x].span = std::max(elem.span.x, widths[elem.coor.x].span);
                        widths[elem.coor.y].span = std::max(elem.span.y, widths[elem.coor.y].span);
                    }
                    if (elem.done && elem.area.size == area_size) continue;
                    elem.area.size = area_size;
                    elem.done = true;
                    object.base::recalc(elem.area);
                    auto delta = elem.area.size - area_size;
                    if (delta.x > 0)
                    {
                        changed = true;
                        auto head = widths.begin() + elem.coor.x;
                        auto tail = head + widths[elem.coor.x].span;
                        auto iter = head + elem.span.x - 1;
                        (*iter++).size.x += delta.x;
                        while (delta.x && iter != tail)
                        {
                            auto& w = (*iter++).size.x;
                            auto dx = std::min(w, delta.x);
                            w -= dx;
                            delta.x -= dx;
                        }
                    }
                    if (delta.y > 0)
                    {
                        changed = true;
                        auto head = widths.begin() + elem.coor.y;
                        auto tail = head + widths[elem.coor.y].span;
                        auto iter = head + elem.span.y - 1;
                        (*iter++).size.y += delta.y;
                        while (delta.y && iter != tail)
                        {
                            auto& h = (*iter++).size.y;
                            auto dy = std::min(h, delta.y);
                            h -= dy;
                            delta.y -= dy;
                        }
                    }
                }
                return changed;
            };
            auto recoor = [&](auto object_iter, auto tail, auto elem_iter)
            {
                while (object_iter != tail)
                {
                    auto& object = *(*object_iter++);
                    auto& elem = *elem_iter++;
                    elem.area.coor = cellsz(dot_00, elem.coor);
                    elem.done = {};
                    object.base::recalc(elem.area);
                }
            };
            while (recalc(base::subset.rbegin(), base::subset.rend(), blocks.rbegin()))
            { }
            recoor(base::subset.begin(), base::subset.end(), blocks.begin());
            new_area.size = std::max(new_area.size, cellsz(dot_00, m));
        }
        // grid: .
        void inform([[maybe_unused]] rect new_area) override
        {
            auto elem_ptr = blocks.begin();
            for (auto& object : base::subset)
            {
                auto& elem = *elem_ptr++;
                object->base::notify(elem.area);
            }
        }

    public:
        grid()
        {
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                if (auto context = form::nested_context(parent_canvas))
                {
                    for (auto& object : base::subset)
                    {
                        object->render(parent_canvas);
                    }
                }
            };
        }
        // grid: .
        void clear()
        {
            auto backup = This();
            base::clear();
            blocks.clear();
        }
        // grid: Attach specified item.
        auto attach(auto item_ptr, elem conf = { .span = dot_11 })
        {
            blocks.push_back(conf);
            auto& blocks_iter = item_ptr->base::property("grid.blocks_iter", blocks.end());
            blocks_iter = std::prev(blocks.end());
            base::attach(item_ptr);
            return item_ptr;
        }
        // grid: Attach item grid.
        void attach_cells(twod size, std::vector<sptr> object_list)
        {
            auto conf = elem{ .span = dot_11, .done = faux };
            auto temp = std::exchange(base::hidden, true); // Suppress reflowing during attaching.
            for (auto object : object_list)
            {
                if (object) attach(object, conf);
                if (++conf.coor.x == size.x)
                {
                    conf.coor.x = 0;
                    if (++conf.coor.y == size.y) break;
                }
            }
            base::hidden = temp;
        }
        // grid: Remove nested object.
        void remove(sptr item_ptr) override
        {
            auto backup = This();
            if (item_ptr && item_ptr->holder != base::subset.end())
            {
                auto& blocks_iter = item_ptr->base::property("grid.blocks_iter", blocks.end());
                blocks.erase(blocks_iter);
                base::remove(item_ptr);
            }
        }
    };

    // controls: Layered cake of objects on top of each other.
    class cake
        : public form<cake>
    {
    protected: 
        // cake: .
        void deform(rect& new_area) override
        {
            auto new_coor = new_area.coor;
            auto new_size = new_area.size;
            auto meter = [&]
            {
                for (auto& object : base::subset)
                {
                    object->base::recalc(new_area);
                    new_area.coor = new_coor;
                }
            };
            meter();
            if (base::subset.size() > 1 && new_size != new_area.size)
            {
                meter();
            }
        }
        // cake: .
        void inform(rect new_area) override
        {
            for (auto& object : base::subset)
            {
                object->base::notify(new_area);
            }
        }

    public:
        cake()
        {
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                if (auto context = form::nested_context(parent_canvas))
                {
                    for (auto& object : base::subset)
                    {
                        object->render(parent_canvas);
                    }
                }
            };
        }
    };

    // controls: Container for multiple objects, but only the last one is shown.
    class veer
        : public form<veer>
    {
    protected:
        // veer: .
        void deform(rect& new_area) override
        {
            if (base::subset.size())
            if (auto object = base::subset.back())
            {
                object->base::recalc(new_area);
            }
        }
        // veer: .
        void inform(rect new_area) override
        {
            if (base::subset.size())
            if (auto object = base::subset.back())
            {
                object->base::notify(new_area);
            }
        }

    public:
        veer()
        {
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                if (base::subset.size())
                if (auto context = form::nested_context(parent_canvas))
                {
                    if (auto object = base::subset.back())
                    {
                        object->render(parent_canvas);
                    }
                }
            };
        }
        // veer: Return the last object or empty sptr.
        auto back()
        {
            return base::subset.size() ? base::subset.back()
                                       : sptr{};
        }
        // veer: Return the first object or empty sptr.
        auto front()
        {
            return base::subset.size() ? base::subset.front()
                                       : sptr{};
        }
        // veer: Return nested objects count.
        auto count()
        {
            return base::subset.size();
        }
        // veer: Return true if empty.
        auto empty()
        {
            return base::subset.empty();
        }
        // veer: Roll objects.
        void roll(si32 dt = 1)
        {
            if (base::subset.size() > 1)
            {
                if (dt > 0) while (dt--)
                {
                    auto item_ptr = base::subset.back();
                    base::subset.push_front(item_ptr);
                    base::subset.pop_back();
                    item_ptr->holder = base::subset.begin();
                }
                else while (dt++)
                {
                    auto item_ptr = base::subset.front();
                    base::subset.push_back(item_ptr);
                    base::subset.pop_front();
                    item_ptr->holder = std::prev(base::subset.end());
                }
            }
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
        bool recent; // post: Paragraphs are not aligned.

    protected:
        // post: .
        void deform(rect& new_area) override
        {
            square = new_area.size;
            flow::reset();
            if (recent) // Update new paragraph's coords before resize.
            {
                auto publish = [&](auto& combo)
                {
                    combo.coord = flow::print(combo);
                };
                topic.stream(publish);
                recent = faux;
            }
            else // Sync anchor.
            {
                auto entry = topic.lookup(base::anchor);
                auto publish = [&](auto& combo)
                {
                    combo.coord = flow::print(combo);
                    if (combo.id() == entry.id) entry.coor.y -= combo.coord.y;
                };
                topic.stream(publish);
                // Apply only vertical anchoring for this type of control.
                base::anchor.y -= entry.coor.y; // Move the central point accordingly to the anchored object
            }
            auto cover = flow::minmax();
            base::oversz = { -std::min(0, cover.coor.x),
                              std::max(0, cover.coor.x + cover.size.x - square.x),
                             -std::min(0, cover.coor.y),
                              0 };
            auto height = cover.size.y;
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

        postfx(bool scroll_beyond = faux)
            :   flow{ square        },
              beyond{ scroll_beyond },
              recent{               }
        {
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                output(parent_canvas);
                //auto mark = rect{ base::anchor + base::coor(), {10,5} };
                //mark.coor += parent_canvas.clip().coor; // Set client's basis
                //parent_canvas.fill(mark, [](cell& c){ c.alpha(0x80).bgc().chan.r = 0xff; });
            };
        }
        // post: .
        auto& lyric(si32 paraid) { return *topic[paraid].lyric; }
        // post: .
        auto& content(si32 paraid) { return topic[paraid]; }
        // post: .
        auto upload(view utf8, si32 initial_width = 0) // Don't use cell link id here. Apply it to the parent (with a whole rect coverage).
        {
            recent = true;
            source = utf8;
            topic = utf8;
            if (initial_width < 0)
            {
                initial_width = topic.limits().x + base::intpad.l + base::intpad.r;
                base::limits({ initial_width, -1 });
            }
            base::resize(twod{ initial_width, 0 });
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
            flow::reset(canvas, base::intpad.corner());
            auto publish = [&](auto const& combo)
            {
                flow::print2(combo, canvas, fx);
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

        twod permit; // rail: Allowed axes to scroll.
        twod siezed; // rail: Allowed axes to capture.
        twod oversc; // rail: Allow overscroll with auto correct.
        twod strict; // rail: Don't allow overscroll.
        twod manual; // rail: Manual scrolling (no auto align).
        bool animat; // rail: Smooth scrolling.
        subs fasten; // rail: Subscriptions on masters to follow they state.
        rack scinfo; // rail: Scroll info.
        fp2d drag_origin; // rail: Drag origin.

        si32 spd       = skin::globals().spd;
        si32 pls       = skin::globals().pls;
        si32 ccl       = skin::globals().ccl;
        si32 spd_accel = skin::globals().spd_accel;
        si32 ccl_accel = skin::globals().ccl_accel;
        si32 spd_max   = skin::globals().spd_max;
        si32 ccl_max   = skin::globals().ccl_max;
        si32 switching = datetime::round<si32>(skin::globals().switching);

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
        rail(axes allow_to_scroll = axes::all, axes allow_to_capture = axes::all, axes allow_overscroll = axes::all, bool smooth_scrolling = faux)
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
                switch (this->bell::protos(tier::preview))
                {
                    case e2::form::upon::scroll::bycoor::v.id: delta = { scinfo.window.coor - info.window.coor };        break;
                    case e2::form::upon::scroll::bycoor::x.id: delta = { scinfo.window.coor.x - info.window.coor.x, 0 }; break;
                    case e2::form::upon::scroll::bycoor::y.id: delta = { 0, scinfo.window.coor.y - info.window.coor.y }; break;
                    case e2::form::upon::scroll::to_top::v.id: delta = { dot_mx };                                       break;
                    case e2::form::upon::scroll::to_top::x.id: delta = { dot_mx.x, 0 };                                  break;
                    case e2::form::upon::scroll::to_top::y.id: delta = { 0, dot_mx.y };                                  break;
                    case e2::form::upon::scroll::to_end::v.id: delta = { -dot_mx };                                      break;
                    case e2::form::upon::scroll::to_end::x.id: delta = { -dot_mx.x, 0 };                                 break;
                    case e2::form::upon::scroll::to_end::y.id: delta = { 0, -dot_mx.y };                                 break;
                    case e2::form::upon::scroll::bystep::v.id: delta = { info.vector };                                  break;
                    case e2::form::upon::scroll::bystep::x.id: delta = { info.vector.x, 0 };                             break;
                    case e2::form::upon::scroll::bystep::y.id: delta = { 0, info.vector.y };                             break;
                    case e2::form::upon::scroll::bypage::v.id: delta = { info.vector * scinfo.window.size };             break;
                    case e2::form::upon::scroll::bypage::x.id: delta = { info.vector.x * scinfo.window.size.x, 0 };      break;
                    case e2::form::upon::scroll::bypage::y.id: delta = { 0, info.vector.y * scinfo.window.size.y };      break;
                    case e2::form::upon::scroll::cancel::v.id: cancel<X, true>(); cancel<Y, true>();                     break;
                    case e2::form::upon::scroll::cancel::x.id: cancel<X, true>();                                        break;
                    case e2::form::upon::scroll::cancel::y.id: cancel<Y, true>();                                        break;
                    default: break;
                }
                if (delta) scroll(delta);
            };
            LISTEN(tier::request, e2::form::upon::scroll::any, req_scinfo)
            {
                req_scinfo = scinfo;
            };

            namespace button = input::events::mouse::button;
            LISTEN(tier::release, input::events::mouse::scroll::act, gear)
            {
                if (gear.meta(hids::anyCtrl)) return; // Ctrl+Wheel is reserved for zooming.
                if (gear.whlsi)
                {
                    auto hz = (permit[X] && (gear.hzwhl || gear.meta(hids::anyAlt | hids::anyShift)))
                           || (permit == xy(axes::X_only));
                    if (hz) wheels<X>(gear.whlsi);
                    else    wheels<Y>(gear.whlsi);
                }
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
                        drag_origin = gear.coord;
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
                    if (auto delta = twod{ gear.coord } - twod{ drag_origin })
                    {
                        drag_origin = gear.coord;
                        auto value = permit * delta;
                        if (value) scroll(value);
                    }
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
            LISTEN(tier::general, input::events::halt, gear)
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
            LISTEN(tier::release, e2::form::animate::reset, task_id)
            {
                cutoff();
            };
            LISTEN(tier::release, e2::form::animate::stop, task_id)
            {
                switch (task_id)
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
                if (auto context = form::nested_context(parent_canvas))
                {
                    auto& item = *base::subset.back();
                    item.render(parent_canvas, faux);
                }
            };
        }

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
                master->LISTEN(tier::release, e2::form::upon::scroll::bycoor::any, master_scinfo, fasten)
                {
                    auto backup_scinfo = master_scinfo;
                    this->base::signal(tier::preview, e2::form::upon::scroll::bycoor::_<Axis>, backup_scinfo);
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
        void wheels(si32 step)
        {
            auto dir = step > 0;
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
                auto delta = Axis == X ? twod{ step, 0 }
                                       : twod{ 0, step };
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
                if (auto step = twod::cast(p))
                {
                    auto delta = Axis == X ? twod{ step, 0 }
                                           : twod{ 0, step };
                    scroll(delta);
                }
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
            auto correct = Forced || !inside<Axis>();
            if (correct) lineup<Axis>();
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
        auto attach(auto object)
        {
            if (!empty()) remove(base::subset.back());
            base::attach(object);
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
                this->base::signal(tier::release, e2::form::upon::scroll::bycoor::any, scinfo);
            };
            return object;
        }
        // rail: Detach specified object.
        void remove(sptr object) override
        {
            if (!empty() && base::subset.back() == object)
            {
                auto backup = This();
                base::remove(object);
                scinfo.region = {};
                scinfo.window.coor = {};
                this->base::signal(tier::release, e2::form::upon::scroll::bycoor::any, scinfo); // Reset dependent scrollbars.
                fasten.clear();
            }
            else base::clear();
        }
        // rail: Update nested object.
        void replace(sptr old_object, sptr new_object) override
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

    namespace drawfx
    {
        static constexpr auto xlight = [](auto& boss, auto& canvas, auto handle, auto object_len, auto handle_len, auto region_len, auto wide)
        {
            if (object_len && handle_len != region_len) // Show only if it is oversized.
            {
                if (wide) // Draw full scrollbar on mouse hover
                {
                    canvas.fill([&](cell& c){ c.link(boss.bell::id).xlight(); });
                }
                canvas.fill(handle, [&](cell& c){ c.link(boss.bell::id).xlight(); });
            }
        };
        static constexpr auto underline = [](auto& /*boss*/, auto& canvas, auto handle, auto object_len, auto handle_len, auto region_len, auto /*wide*/)
        {
            if (object_len && handle_len != region_len) // Show only if it is oversized.
            {
                canvas.fill(handle, cell::shaders::underlight);
            }
        };
    }

    // controls: Scrollbar.
    template<axis Axis>
    class grip
        : public flow, public form<grip<Axis>>
    {
        pro::timer timer{*this }; // grip: Minimize by timeout.

        using form = ui::form<grip<Axis>>;

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
            si32  m          = 0; // math: Master max pos.
            si32  s          = 0; // math: Scroll max pos.
            fp64  r          = 1; // math: Scroll/master len ratio.

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
                r = (fp64)scroll_len / master_len;
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
                if (coor >= scroll_pos + scroll_box)
                {
                    return -1; // Below the grip.
                }
                if (coor >= scroll_pos)
                {
                    return 0; // Inside the grip.
                }
                else
                {
                    return 1; // Above the grip.
                }
            }
            auto follow()
            {
                auto dir = scroll_len > 2 ? inside(cursor_pos)
                                          : cursor_pos > 0 ?-1 // Don't stop to follow over
                                                           : 1;//    box on small scrollbar.
                return dir;
            }
            void setdir(si32 dir)
            {
                master_dir = dir;
            }
        };

        wptr boss; // grip: Scroll info source.
        bool wide = faux; // grip: Is the scrollbar active.
        si32 thin = 1; // grip: Scrollbar thickness.
        si32 init = 1; // grip: Handle base width.
        si32 mult = 2; // grip: Vertical bar width multiplier.
        math calc; // grip: Scrollbar calculator.
        bool on_pager = faux; // grip: .
        fp2d drag_origin; // grip: Drag origin.

        template<auto Event>
        void send()
        {
            if (auto master = this->boss.lock())
            {
                master->base::signal(tier::preview, Event, calc.master_inf);
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
                    if (this->form::protos(tier::release, input::events::mouse::button::drag::cancel::right))
                    {
                        send<e2::form::upon::scroll::cancel::_<Axis>>();
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
            send<e2::form::upon::scroll::bypage::_<Axis>>();
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
        // grip: .
        void inform(rect new_area) override
        {
            calc.resize(new_area.size);
        }

    public:
        grip(sptr boss_ptr, auto& drawfx)
            : boss{ boss_ptr }
        {
            config(thin);
            boss_ptr->LISTEN(tier::release, e2::form::upon::scroll::bycoor::any, scinfo, bell::sensors)
            {
                calc.update(scinfo);
                base::deface();
            };
            LISTEN(tier::release, input::events::mouse::scroll::act, gear)
            {
                if (gear.meta(hids::anyCtrl)) return; // Ctrl+Wheel is reserved for zooming.
                if (gear.whlsi) pager(gear.whlsi > 0 ? 1 : -1);
                gear.dismiss();
            };
            LISTEN(tier::release, input::events::mouse::move, gear)
            {
                calc.cursor_pos = twod{ gear.coord }[Axis];
            };
            LISTEN(tier::release, input::events::mouse::button::dblclick::left, gear)
            {
                gear.dismiss(); // Do not pass double clicks outside.
            };
            LISTEN(tier::release, input::events::mouse::button::down::any, gear)
            {
                if (!on_pager)
                if (this->form::protos(tier::release, input::events::mouse::button::down::left)
                 || this->form::protos(tier::release, input::events::mouse::button::down::right))
                if (auto dir = calc.inside(twod{ gear.coord }[Axis]))
                {
                    if (gear.capture(bell::id))
                    {
                        on_pager = true;
                        pager_repeat();
                        gear.dismiss();
                        timer.actify(activity::pager_first, skin::globals().repeat_delay, [&](auto)
                        {
                            if (pager_repeat())
                            {
                                timer.actify(activity::pager_next, skin::globals().repeat_rate, [&](auto)
                                {
                                    return pager_repeat(); // Repeat until on_pager.
                                });
                            }
                            return faux; // One shot call (first).
                        });
                    }
                }
            };
            LISTEN(tier::release, input::events::mouse::button::up::any, gear)
            {
                if (on_pager && gear.captured(bell::id))
                {
                    if (this->form::protos(tier::release, input::events::mouse::button::up::left)
                     || this->form::protos(tier::release, input::events::mouse::button::up::right))
                    {
                        gear.setfree();
                        gear.dismiss();
                        on_pager = faux;
                        timer.pacify(activity::pager_first);
                        timer.pacify(activity::pager_next);
                    }
                }
            };
            LISTEN(tier::release, input::events::mouse::button::up::right, gear)
            {
                //if (!gear.captured(bell::id)) //todo why?
                {
                    send<e2::form::upon::scroll::cancel::_<Axis>>();
                    gear.dismiss();
                }
            };
            LISTEN(tier::release, input::events::mouse::button::drag::start::any, gear)
            {
                if (on_pager)
                {
                    gear.dismiss();
                }
                else
                {
                    if (gear.capture(bell::id))
                    {
                        drag_origin = gear.coord;
                        gear.dismiss();
                    }
                }
            };
            LISTEN(tier::release, input::events::mouse::button::drag::pull::any, gear)
            {
                if (on_pager)
                {
                    gear.dismiss();
                }
                else
                {
                    if (gear.captured(bell::id))
                    {
                        if (auto delta = (twod{ gear.coord } - twod{ drag_origin })[Axis])
                        {
                            drag_origin = gear.coord;
                            calc.stepby(delta);
                            send<e2::form::upon::scroll::bycoor::_<Axis>>();
                            gear.dismiss();
                        }
                    }
                }
            };
            LISTEN(tier::release, input::events::mouse::button::drag::cancel::any, gear)
            {
                giveup(gear);
            };
            LISTEN(tier::general, input::events::halt, gear)
            {
                giveup(gear);
            };
            LISTEN(tier::release, input::events::mouse::button::drag::stop::any, gear)
            {
                if (on_pager)
                {
                    gear.dismiss();
                }
                else
                {
                    if (gear.captured(bell::id))
                    {
                        if (this->form::protos(tier::release, input::events::mouse::button::drag::stop::right))
                        {
                            send<e2::form::upon::scroll::cancel::_<Axis>>();
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
                    auto resize = Axis == axis::Y && mult;
                    if (resize) config(active ? init * mult // Make vertical scrollbar
                                              : init);      // wider on hover.
                    base::reflow();
                    return faux; // One-shot call.
                };
                timer.pacify(activity::mouse_leave);
                if (active)
                {
                    apply(activity::mouse_hover);
                }
                else
                {
                    timer.actify(activity::mouse_leave, skin::globals().leave_timeout, apply);
                }
            };
            //LISTEN(tier::release, input::events::mouse::move, gear)
            //{
            //    auto apply = [&](auto active)
            //    {
            //        wide = active;
            //        if (Axis == axis::Y) config(active ? init * 2 // Make vertical scrollbar
            //                                           : init);   //  wider on hover
            //        base::reflow();
            //        return faux; // One shot call
            //    };
            //
            //    timer.pacify(activity::mouse_leave);
            //    apply(activity::mouse_hover);
            //    timer.template actify<activity::mouse_leave>(skin::globals().leave_timeout, apply);
            //};
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                auto region = parent_canvas.clip();
                auto object = parent_canvas.full();
                auto handle = region;
                calc.commit(handle);
                auto& handle_len = handle.size[Axis];
                auto& region_len = region.size[Axis];
                auto& object_len = object.size[Axis];
                handle.trimby(region);
                handle_len = std::max(1, handle_len);
                drawfx(*this, parent_canvas, handle, object_len, handle_len, region_len, wide);
            };
        }
        grip(sptr boss_ptr)
            : grip{ boss_ptr, drawfx::xlight }
        { }
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
        text utf8{}; // item: Text source.
        bool flex{}; // item: Violate or not the label size.
        bool test{}; // item: Place or not(default) the Two Dot Leader when there is not enough space.
        bool ulin{}; // item: Draw full-width underline.

        // item: .
        void _set(view new_utf8)
        {
            data.parser::style.wrp(wrap::off);
            data = new_utf8;
            utf8 = new_utf8;
        }

    protected:
        // item: .
        void deform(rect& new_area) override
        {
            new_area.size.x = flex ? new_area.size.x : data.size().x;
            new_area.size.y = std::max(data.size().y, new_area.size.y);
        }

    public:
        // item: .
        template<bool Reflow = true>
        auto set(view new_utf8)
        {
            _set(new_utf8);
            if constexpr (Reflow) base::reflow();
            return This();
        }
        // item: .
        auto& get_source()
        {
            return utf8;
        }

        item(view label = {})
        {
            _set(label);
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
                    auto area = parent_canvas.clip();
                    auto size = data.size();
                    if (area.size > 0 && size.x > 0)
                    {
                        if (full.coor.x < area.coor.x)
                        {
                            auto coor = area.coor - parent_canvas.coor();
                            coor.y += std::min(area.size.y - 1, base::intpad.t);
                            parent_canvas.core::begin(coor)->txt(dots);
                        }
                        if (full.coor.x + base::intpad.l + size.x + base::intpad.r > area.coor.x + area.size.x)
                        {
                            auto coor = area.coor - parent_canvas.coor();
                            coor.x += area.size.x - 1;
                            coor.y += std::min(area.size.y - 1, base::intpad.t);
                            parent_canvas.core::begin(coor)->txt(dots);
                        }
                    }
                }
                if (ulin)
                {
                    auto area = parent_canvas.full();
                    parent_canvas.fill(area, [](cell& c)
                    {
                        auto u = c.und();
                        if (u == unln::line) c.und(unln::biline);
                        else                 c.und(unln::line);
                    });
                }
                parent_canvas.bump(context);
            };
        }
        // item: .
        auto flexible(bool b = true) { flex = b; return This(); }
        // item: .
        auto drawdots(bool b = true) { test = b; return This(); }
        // item: .
        auto accented(bool b = true) { ulin = b; return This(); }
        // item: .
        void brush(cell c)
        {
            data.parser::brush.reset(c);
        }
    };

    // controls: Textedit box.
    class edit
        : public form<edit>
    {
        page data;

    public:
        edit()
        {
        }
    };
}