// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

#include "netxs/desktopio/application.hpp"
#include "netxs/apps/desk.hpp"

namespace netxs::app::vtm
{
    static constexpr auto id = "vtm";
    using ui::sptr;
    using ui::wptr;

    struct applink
    {
        text menuid{};
        text kindid{};
        text header{};
        text footer{};
        rect square{};
        bool forced{};
        sptr applet{};
    };

    namespace attr
    {
        static constexpr auto id       = "id";
        static constexpr auto alias    = "alias";
        static constexpr auto hidden   = "hidden";
        static constexpr auto label    = "label";
        static constexpr auto tooltip  = "tooltip";
        static constexpr auto title    = "title";
        static constexpr auto footer   = "footer";
        static constexpr auto winsize  = "winsize";
        static constexpr auto wincoor  = "wincoor";
        static constexpr auto winform  = "winform";
        static constexpr auto focused  = "focused";
        static constexpr auto type     = "type";
        static constexpr auto env      = "env";
        static constexpr auto cwd      = "cwd";
        static constexpr auto cmd      = "cmd";
        static constexpr auto cfg      = "cfg";
        static constexpr auto splitter = "splitter";
        static constexpr auto config   = "config";
    }
    namespace path
    {
        static constexpr auto item     = "/config/desktop/taskbar/item";
        static constexpr auto autorun  = "/config/desktop/taskbar/autorun/item";
        static constexpr auto viewport = "/config/desktop/viewport/coor";
    }

    struct events
    {
        EVENTPACK( events, ui::e2::extra::slot1 )
        {
            EVENT_XS( newapp  , applink ), // request: Create new object using specified meniid.
            EVENT_XS( apptype , applink ), // request: Ask app type.
            EVENT_XS( handoff , applink ), // general: Attach spcified intance and return sptr.
            EVENT_XS( attached, sptr    ), // anycast: Inform that the object tree is attached to the world.
            GROUP_XS( d_n_d   , sptr    ), // Drag&drop functionality. See tiling manager empty slot and pro::d_n_d.
            GROUP_XS( gate    , sptr    ),

            SUBSET_XS(d_n_d)
            {
                EVENT_XS( ask  , sptr    ),
                EVENT_XS( abort, sptr    ),
                EVENT_XS( drop , applink ),
            };
            SUBSET_XS(gate)
            {
                EVENT_XS( fullscreen, applink ), // release: Toggle fullscreen mode.
                //EVENT_XS( restore   , applink ), // release: Restore from fullscreen.
            };
        };
    };

    namespace pro
    {
        using namespace netxs::ui::pro;

        // pro: Provides functionality for manipulating objects with a frame structure.
        class frame
            : public skill
        {
            using skill::boss,
                  skill::memo;

            robot robo;
            zpos  seat;
            fp2d  drag_origin;

        public:
            frame(base&&) = delete;
            frame(base& boss, zpos z_order = zpos::plain) : skill{ boss },
                robo{ boss    },
                seat{ z_order }
            {
                boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent, memo)
                {
                    boss.bell::signal(tier::release, e2::form::prop::zorder, seat);
                };
                boss.LISTEN(tier::preview, e2::form::prop::zorder, order)
                {
                    seat = order;
                    boss.bell::signal(tier::release, e2::form::prop::zorder, seat);
                };
                boss.LISTEN(tier::request, e2::form::prop::zorder, order)
                {
                    order = seat;
                };
                boss.LISTEN(tier::preview, hids::events::mouse::button::click::left, gear, memo)
                {
                    //todo window.events(onclick)
                    boss.base::riseup(tier::preview, e2::form::layout::expose);
                };
                boss.LISTEN(tier::preview, hids::events::mouse::button::click::right, gear, memo)
                {
                    //todo window.events(onclick)
                    boss.base::riseup(tier::preview, e2::form::layout::expose);
                };
                boss.LISTEN(tier::preview, e2::form::layout::appear, newpos, memo)
                {
                    appear(newpos);
                };
                //boss.LISTEN(tier::preview, e2::form::upon::moved, delta, memo)
                //{
                //    bubble();
                //};
                boss.LISTEN(tier::preview, e2::form::upon::changed, delta, memo)
                {
                    boss.base::riseup(tier::preview, e2::form::layout::bubble);
                };
                boss.LISTEN(tier::preview, hids::events::mouse::button::down::any, gear, memo)
                {
                    robo.pacify();
                };
                boss.LISTEN(tier::release, e2::form::drag::start::any, gear, memo)
                {
                    drag_origin = gear.coord;
                };
                boss.LISTEN(tier::release, e2::form::drag::pull::any, gear, memo)
                {
                    if (gear)
                    {
                        auto deed = boss.bell::protos(tier::release);
                        switch (deed)
                        {
                            case e2::form::drag::pull::left.id:
                            case e2::form::drag::pull::leftright.id:
                            {
                                if (auto delta = twod{ gear.coord } - twod{ drag_origin })
                                {
                                    boss.base::anchor = drag_origin; // See pro::align unbind.
                                    auto preview_area = rect{ boss.base::coor() + delta, boss.base::size() };
                                    boss.bell::signal(tier::preview, e2::area, preview_area);
                                    boss.base::moveby(delta);
                                    boss.bell::signal(tier::preview, e2::form::upon::changed, delta);
                                }
                                gear.dismiss();
                                break;
                            }
                            default: break;
                        }
                    }
                };
                boss.LISTEN(tier::release, e2::form::upon::dragged, gear, memo)
                {
                    if (gear.meta(hids::anyCtrl))
                    {
                        robo.actify(gear.fader<quadratic<twod>>(2s), [&](auto x)
                        {
                            boss.base::moveby(x);
                            boss.strike();
                        });
                    }
                    else
                    {
                        auto boundary = gear.owner.bell::signal(tier::request, e2::form::prop::viewport);
                        robo.actify(gear.fader<quadratic<twod>>(2s), [&, boundary](auto x)
                        {
                            //todo revise: crash after window closed (bad weak ptr)
                            convey(x, boundary);
                            boss.strike();
                        });
                    }
                };
                boss.LISTEN(tier::release, hids::events::mouse::button::click::right, gear, memo)
                {
                    auto owner_id = boss.bell::signal(tier::request, e2::form::state::maximized);
                    if (owner_id) return; // Don't move maximized window.
                    auto& area = boss.base::area();
                    auto coord = gear.coord + area.coor;
                    if (!area.hittest(coord))
                    {
                        pro::focus::set(boss.This(), gear.id, solo::on);
                        appear(coord);
                    }
                    gear.dismiss();
                };
            };

            // pro::frame: Fly to the specified position.
            void appear(twod target)
            {
                auto screen = boss.base::area();
                auto oldpos = screen.coor;
                auto newpos = target - screen.size / 2;

                auto path = newpos - oldpos;
                auto time = datetime::round<si32>(skin::globals().switching);
                auto init = 0;
                auto func = constlinearAtoB<twod>(path, time, init);

                robo.pacify();
                robo.actify(func, [&](twod& x){ boss.base::moveby(x); boss.strike(); });
            }
            /*
            // pro::frame: Search for a non-overlapping form position in
            //             the visual tree along a specified direction.
            rect bounce(rect block, twod dir)
            {
                auto result = block.rotate(dir);
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
            */
            // pro::frame: Move the form no further than the parent canvas.
            void convey(twod delta, rect boundary)//, bool notify = true)
            {
                auto& r0 = boss.base::area();
                if (delta && r0.trim(boundary))
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
                        auto new_coor = c.normalize().coor + r2.coor;
                        boss.moveto(new_coor);
                    }
                    else if (!r2.trim(r0))
                    {
                        boss.moveby(delta);
                    }
                }
            }
        };

        // pro: Form generator.
        class maker
            : public skill
        {
            using skill::boss,
                  skill::memo;

            cell mark;

            struct slot_t
            {
                rect slot{};
                fp2d step{};
                fp2d init{};
                bool ctrl{};
            };
            std::unordered_map<id_t, slot_t> slots;
            escx coder;
            vrgb cache;

            void check_modifiers(hids& gear)
            {
                auto& data = slots[gear.id];
                auto state = !!gear.meta(hids::anyCtrl);
                if (data.ctrl != state)
                {
                    data.ctrl = state;
                    boss.deface(data.slot);
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

                    data.ctrl = gear.meta(hids::anyCtrl);
                    slot.coor = init = step = gear.click;
                    slot.size = dot_00;
                    boss.deface(slot);
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
                    step += gear.delta.get();
                    auto moved = slot.coor(std::min(init, step));
                    auto dsize = twod{ step } - twod{ init };
                    auto sized = slot.size(std::max(std::abs(dsize), dot_00));
                    if (moved || sized) boss.deface(slot);
                    gear.dismiss();
                }
            }
            void handle_drop(hids& gear)
            {
                if (gear.captured(boss.bell::id))
                {
                    slots.erase(gear.id);
                    if (slots.empty()) cache = {};
                    gear.dismiss();
                    gear.setfree();
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
                        gear.slot.coor += boss.base::coor();
                        gear.slot_forced = true;
                        if (gear.meta(hids::anyCtrl))
                        {
                            log(prompt::hall, "Area copied to clipboard ", gear.slot);
                            gear.owner.bell::signal(tier::release, e2::command::printscreen, gear);
                        }
                        else
                        {
                            boss.base::riseup(tier::request, e2::form::proceed::createby, gear);
                        }
                    }
                    slots.erase(gear.id);
                    if (slots.empty()) cache = {};
                    gear.dismiss();
                    gear.setfree();
                }
            }

        public:
            maker(base&&) = delete;
            maker(base& boss)
                : skill{ boss },
                   mark{ cell{ skin::color(tone::brighter) }.txt(" ") }
            {
                using drag = hids::events::mouse::button::drag;

                boss.LISTEN(tier::preview, hids::events::keybd::key::post, gear, memo)
                {
                    if (gear.captured(boss.bell::id)) check_modifiers(gear);
                };

                //todo unify - args... + template?
                //middle button
                //todo revise boss.LISTEN(tier::preview, drag::start::middle, gear, memo)
                boss.LISTEN(tier::release, drag::start::middle, gear, memo)
                {
                    handle_init(gear);
                };
                boss.LISTEN(tier::release, drag::pull::middle, gear, memo)
                {
                    handle_pull(gear);
                };
                boss.LISTEN(tier::release, drag::cancel::middle, gear, memo)
                {
                    handle_drop(gear);
                };
                boss.LISTEN(tier::release, drag::stop::middle, gear, memo)
                {
                    handle_stop(gear);
                };

                //todo unify
                //right button
                boss.LISTEN(tier::release, drag::start::right, gear, memo)
                {
                    handle_init(gear);
                };
                boss.LISTEN(tier::release, drag::pull::right, gear, memo)
                {
                    handle_pull(gear);
                };
                boss.LISTEN(tier::release, drag::cancel::right, gear, memo)
                {
                    handle_drop(gear);
                };
                boss.LISTEN(tier::release, drag::stop::right, gear, memo)
                {
                    handle_stop(gear);
                };

                boss.LISTEN(tier::general, hids::events::halt, gear, memo)
                {
                    handle_drop(gear);
                };

                boss.LISTEN(tier::release, e2::postrender, canvas, memo)
                {
                    //todo Highlighted area drawn twice
                    for (auto const& [key, data] : slots)
                    {
                        auto slot = data.slot;
                        slot.coor -= canvas.coor();
                        if (auto area = canvas.area().trim<true>(slot))
                        {
                            if (data.ctrl)
                            {
                                area.coor -= dot_11;
                                area.size += dot_22;
                                auto mark = skin::color(tone::winfocus);
                                auto fill = [&](cell& c){ c.fuse(mark); };
                                canvas.cage(area, dot_11, fill);
                                coder.wrp(wrap::off).add("capture area: ", slot);
                                //todo optimize para
                                auto caption = para(coder);
                                coder.clear();
                                auto header = *caption.lyric;
                                auto coor = area.coor + canvas.coor();
                                coor.y--;
                                header.move(coor);
                                canvas.fill(header, cell::shaders::contrast);
                            }
                            else
                            {
                                auto temp = canvas.clip();
                                canvas.clip(area);
                                canvas.fill(area, [&](cell& c){ c.fuse(mark); c.und(faux); });
                                canvas.blur(5, cache);
                                coder.wrp(wrap::off).add(' ').add(slot.size.x).add(" × ").add(slot.size.y).add(' ');
                                //todo optimize para
                                auto caption = para(coder);
                                coder.clear();
                                auto header = *caption.lyric;
                                auto coor = area.coor + area.size + canvas.coor();
                                coor.x -= caption.length() - 1;
                                header.move(coor);
                                canvas.fill(header, cell::shaders::contrast);
                                canvas.clip(temp);
                            }
                        }
                    }
                };
            }
        };

        // pro: Drag&drop functionality.
        class d_n_d
            : public skill
        {
            using skill::boss,
                  skill::memo;

            bool drags;
            id_t under;
            fp2d coord;
            wptr cover;

            void proceed(bool keep, hids& gear)
            {
                drags = faux;
                boss.bell::signal(tier::anycast, e2::form::prop::lucidity, 0xFF); // Make target opaque.
                auto boss_ptr = boss.This();
                if (auto dest_ptr = cover.lock())
                {
                    auto& dest = *dest_ptr;
                    if (keep)
                    {
                        auto what = boss.bell::signal(tier::preview, vtm::events::d_n_d::drop); // Take core.
                        if (what.applet)
                        {
                            auto gear_id_list = pro::focus::cut(what.applet);
                            what.applet->base::detach();
                            dest.bell::signal(tier::release, vtm::events::d_n_d::drop, what); // Pass core.
                            pro::focus::set(what.applet, gear.id, solo::on, true); // Set unique focus.
                            boss.base::detach(); // The object kills itself.
                        }
                    }
                    else dest.bell::signal(tier::release, vtm::events::d_n_d::abort, boss.This());
                }
                cover.reset();
                under = {};
            }

        public:
            d_n_d(base&&) = delete;
            d_n_d(base& boss)
                : skill{ boss },
                  drags{ faux },
                  under{      }
            {
                boss.LISTEN(tier::release, hids::events::mouse::button::drag::start::any, gear, memo)
                {
                    if (boss.size().inside(gear.coord) && !gear.meta(hids::anyMod))
                    if (drags || !gear.capture(boss.id)) return;
                    {
                        drags = true;
                        coord = gear.coord;
                        under = {};
                    }
                };
                boss.LISTEN(tier::release, hids::events::mouse::button::drag::pull::any, gear, memo)
                {
                    if (!drags) return;
                    if (gear.meta(hids::anyMod)) proceed(faux, gear);
                    else                         coord = gear.coord - gear.delta.get();
                };
                boss.LISTEN(tier::release, hids::events::mouse::button::drag::stop::any, gear, memo)
                {
                    if (!drags) return;
                    if (gear.meta(hids::anyMod)) proceed(faux, gear);
                    else                         proceed(true, gear);
                    gear.setfree();
                };
                boss.LISTEN(tier::release, hids::events::mouse::button::drag::cancel::any, gear, memo)
                {
                    if (!drags) return;
                    if (gear.meta(hids::anyMod)) proceed(faux, gear);
                    else                         proceed(true, gear);
                    gear.setfree();
                };
                boss.LISTEN(tier::release, e2::render::background::prerender, parent_canvas, memo)
                {
                    if (!drags) return;
                    auto area = parent_canvas.core::area();
                    auto coor = coord - area.coor;
                    if (area.size.inside(coor))
                    {
                        auto& c = parent_canvas[coor];
                        auto new_under = c.link();
                        if (under != new_under)
                        {
                            auto object = vtm::events::d_n_d::ask.param();
                            if (auto old_object = boss.bell::getref<base>(under))
                            {
                                old_object->base::riseup(tier::release, vtm::events::d_n_d::abort, object);
                            }
                            if (auto new_object = boss.bell::getref<base>(new_under))
                            {
                                new_object->base::riseup(tier::release, vtm::events::d_n_d::ask, object);
                            }
                            boss.bell::signal(tier::anycast, e2::form::prop::lucidity, object ? 0x80 : 0xFF); // Make it semi-transparent on success and opaque otherwise.
                            cover = object;
                            under = new_under;
                        }
                    }
                };
            }
        };
    }

    // vtm: User gate.
    struct gate
        : public ui::gate
    {
        wptr    saved;// align: .
        applink what; // align: Original app window properties.
        rect    prev; // align: Window size before the fullscreen has applied.
        twod    coor; // align: Coor tracking.
        subs    maxs; // align: Fullscreen event subscription token.
        subs    memo; // align: .
        struct restoration_type
        {
            static constexpr auto _counter   = __COUNTER__ + 1;
            static constexpr auto full = __COUNTER__ - _counter;
            static constexpr auto size = __COUNTER__ - _counter;
            static constexpr auto coor = __COUNTER__ - _counter;
        };
        void follow(applink& new_what)
        {
            if (new_what.applet->subset.empty()) return;
            auto gear_id_list = pro::focus::cut(new_what.applet);
            prev = new_what.applet->base::area();
            auto window_ptr = std::exchange(new_what.applet, new_what.applet->subset.front()); // Drop hosting window.
            window_ptr->base::detach();
            what = new_what;
            auto applet_ptr = what.applet;
            saved = std::exchange(nexthop, applet_ptr);
            applet_ptr->base::detach();
            auto new_pos = base::area();
            new_pos.coor -= base::coor();
            applet_ptr->base::extend(new_pos);
            coor = applet_ptr->base::coor();

            auto newhead = std::move(what.header);
            auto newfoot = std::move(what.footer);
            base::riseup(tier::request, e2::form::prop::ui::header, what.header);
            base::riseup(tier::request, e2::form::prop::ui::footer, what.footer);
            base::riseup(tier::preview, e2::form::prop::ui::header, newhead);
            base::riseup(tier::preview, e2::form::prop::ui::footer, newfoot);

            LISTEN(tier::anycast, e2::form::proceed::quit::one, fast, memo)
            {
                unbind();
            };
            LISTEN(tier::release, e2::dtor, p, memo)
            {
                unbind();
            };
            LISTEN(tier::release, e2::area, new_area, memo)
            {
                if (new_area.coor != base::coor()) unbind();
                else what.applet->base::resize(new_area.size);
            };
            LISTEN(tier::preview, e2::form::proceed::action::restore, gear, memo)
            {
                unbind();
                bell::expire(tier::preview);
            };
            applet_ptr->LISTEN(tier::preview, e2::form::size::enlarge::any, gear, memo)
            {
                auto deed = what.applet->bell::protos(tier::preview);
                if (deed == e2::form::size::enlarge::maximize.id)
                {
                    unbind();
                }
            };
            applet_ptr->LISTEN(tier::release, e2::form::size::minimize, gear, memo)
            {
                what.applet->bell::expire(tier::release); // Suppress hide/minimization.
                unbind();
            };
            applet_ptr->LISTEN(tier::release, e2::form::proceed::quit::one, fast, memo)
            {
                unbind();
                bell::expire(tier::release, true); //todo revise: applet_ptr(what.applet) or boss?
            };
            applet_ptr->LISTEN(tier::release, e2::area, new_area, memo)
            {
                if (coor != new_area.coor) unbind(restoration_type::size);
            };
            applet_ptr->LISTEN(tier::preview, e2::area, new_area, memo)
            {
                if (coor != new_area.coor) unbind(restoration_type::size);
            };
            applet_ptr->bell::signal(tier::release, e2::form::upon::vtree::attached, This());
            applet_ptr->bell::signal(tier::anycast, vtm::events::attached, This());
            pro::focus::set(applet_ptr, gear_id_list, solo::on, true); // Refocus.
        }
        void unbind(si32 restore = restoration_type::full)
        {
            if (!memo) return;
            nexthop = std::exchange(saved, wptr{});
            memo.clear();
            auto prev_header = std::move(what.header);
            auto prev_footer = std::move(what.footer);
            base::riseup(tier::request, e2::form::prop::ui::header, what.header);
            base::riseup(tier::request, e2::form::prop::ui::footer, what.footer);
            base::riseup(tier::preview, e2::form::prop::ui::header, prev_header);
            base::riseup(tier::preview, e2::form::prop::ui::footer, prev_footer);
            if (auto world_ptr = bell::signal(tier::general, e2::config::creator))
            {
                auto gear_id_list = pro::focus::cut(what.applet);
                what.applet->base::detach();
                world_ptr->bell::signal(tier::request, vtm::events::handoff, what);
                pro::focus::set(what.applet, gear_id_list, solo::on, true);
            }
            if (auto window_ptr = what.applet->base::parent())
            {
                auto& window = *window_ptr;
                switch (restore)
                {
                    case restoration_type::full: window.base::extend(prev); break; // Restore previous position.
                    case restoration_type::coor: window.base::moveto(prev.coor); break;
                    case restoration_type::size:
                    {
                        auto window_size = window.base::size();
                        auto window_anchor = std::clamp(window.base::anchor, dot_00, std::max(dot_00, window_size));
                        window_anchor = window_anchor * prev.size / std::max(dot_11, window_size);
                        prev.coor = base::coor();
                        prev.coor += window.base::anchor - window_anchor; // Follow the mouse cursor. See pro::frame pull.
                        window.base::extend(prev);
                        break;
                    }
                }
            }
            what.applet.reset();
        }

        gate(xipc uplink, view userid, si32 vtmode, xmls& config, si32 session_id)
            : ui::gate{ uplink, vtmode, config, userid, session_id, true }
        {
            //todo local=>nexthop
            local = faux;
            //todo scripting
            //keybd.proc("RunScript", [&](hids& gear){ base::riseup(tier::preview, e2::form::proceed::action::runscript, gear); });
            auto& keybd = plugins<pro::keybd>();
            auto bindings = pro::keybd::load(config, "desktop"); //todo rename "desktop" to "gate"?
            keybd.bind(bindings);

            LISTEN(tier::release, vtm::events::gate::fullscreen, new_what, maxs)
            {
                if (what.applet)
                {
                    unbind();
                }
                if (new_what.applet)
                {
                    follow(new_what);
                }
            };
            //LISTEN(tier::request, vtm::events::gate::fullscreen, ask_what, maxs)
            //{
            //    ask_what = what;
            //};

            LISTEN(tier::release, hids::events::focus::set::any, seed) // Any: To run prior the ui::gate's hids::events::focus::any.
            {
                if (seed.treeid)
                {
                    if (auto target = nexthop.lock())
                    {
                        auto deed = this->bell::protos(tier::release);
                        target->bell::signal(tier::request, deed, seed); // Request to filter recursive loops.
                        this->bell::expire(tier::release); // Do not pass the event to the ui::gate.
                    }
                }
                else
                {
                    this->bell::expire(tier::release, true);
                }
            };
            //todo mimic pro::focus
            LISTEN(tier::request, hids::events::focus::cut, seed, -, (treeid = datetime::uniqueid(), digest = ui64{}))
            {
                if (what.applet)
                {
                    seed.treeid = treeid;
                    seed.digest = ++digest;
                    for (auto& [ext_gear_id, gear_ptr] : gears)
                    {
                        if (ext_gear_id && !gear_ptr->keybd_disabled) // Ignore default and halted gears.
                        {
                            if (auto gear_id = gear_ptr->id)
                            {
                                seed.gear_id = gear_id;
                                what.applet->bell::signal(tier::release, hids::events::focus::set::off, seed);
                            }
                        }
                    }
                }
            };

            LISTEN(tier::release, e2::form::upon::vtree::attached, world_ptr)
            {
                nexthop = world_ptr;
            };

            LISTEN(tier::release, e2::render::any, canvas)
            {
                if (&canvas != &xmap) // Draw a shadow of user's gate for other users.
                {
                    auto gate_area = canvas.full();
                    if (canvas.cmode != svga::vt16 && canvas.cmode != svga::nt16) // Don't show shadow in poor color environment.
                    {
                        //todo revise
                        auto mark = skin::color(tone::shadower);
                        mark.bga(mark.bga() / 2);
                        canvas.fill(gate_area, [&](cell& c){ c.fuse(mark); });
                    }
                    auto saved_context = canvas.bump(dent{ 0,0,1,0 });
                    canvas.output(uname, dot_00, cell::shaders::contrast);
                    canvas.bump(saved_context);
                }
            };
            LISTEN(tier::release, e2::postrender, parent_canvas)
            {
                if (&parent_canvas != &xmap)
                {
                    if (uname.lyric) // Render foreign user names at their place.
                    {
                        draw_foreign_names(parent_canvas);
                    }
                    draw_mouse_pointer(parent_canvas);
                }
            };
        }
    };

    // vtm: Desktop Workspace.
    struct hall
        : public form<hall>
    {
    private:
        struct node // hall: Adapter for the object that going to be attached to the world.
        {
            bool highlighted = faux;
            si32 active = 0;
            tone color = { tone::brighter, tone::shadower };
            sptr object;
            zpos z_order = zpos::plain;
            id_t monoid = {};
            subs tokens;

            node(sptr item)
                : object{ item }
            {
                auto& inst = *item;
                inst.LISTEN(tier::release, e2::form::state::maximized, gear_id, tokens)
                {
                    monoid = gear_id;
                };
                inst.LISTEN(tier::request, e2::form::state::maximized, gear_id, tokens)
                {
                    gear_id = monoid;
                };
                inst.LISTEN(tier::release, e2::form::prop::zorder, order, tokens)
                {
                    z_order = order;
                };
                inst.LISTEN(tier::release, e2::form::state::mouse, state, tokens)
                {
                    active = state;
                };
                inst.LISTEN(tier::release, e2::form::state::highlight, state, tokens)
                {
                    highlighted = state;
                };
                inst.LISTEN(tier::release, e2::form::state::color, new_color, tokens)
                {
                    color = new_color;
                };
                inst.bell::signal(tier::request, e2::form::state::mouse, active);
                inst.bell::signal(tier::request, e2::form::state::color, color);
            }
            // hall::node: Draw a navigation string.
            void fasten(face& canvas)
            {
                auto window = canvas.area();
                auto center = object->region.center();
                if (window.hittest(center) || object->hidden) return;
                auto origin = window.size / 2;
                center -= window.coor;
                //auto origin = twod{ 6, window.size.y - 3 };
                //header.usable = window.overlap(region);
                auto is_active = active || highlighted;
                auto& grade = skin::grade(is_active ? color.active
                                                    : color.passive);
                auto obj_id = object->id;
                auto pset = [&](twod p, si32 k)
                {
                    //canvas[p].fuse(grade[k], obj_id, p - offset);
                    //canvas[p].fuse(grade[k], obj_id);
                    auto g = grade[k & 0xFF].bgc();
                    auto& c = canvas[p];
                    c.link(obj_id);
                    c.bgc().mix_one(g);
                    c.fgc().mix_one(g);
                };
                window.coor = dot_00;
                netxs::online(window, origin, center, pset);
            }
        };
        struct list // hall: List of objects that can be reordered, etc.
        {
            std::list<netxs::sptr<node>> items;

            template<class D>
            auto search(D head, D tail, id_t id)
            {
                if (items.size())
                {
                    auto test = [id](auto& a){ return a->object->id == id; };
                    return std::find_if(head, tail, test);
                }
                return tail;
            }

            operator bool () { return items.size(); }
            auto size()      { return items.size(); }
            auto back()      { return items.back()->object; }
            void append(sptr window_ptr)
            {
                auto& window = *window_ptr;
                window.LISTEN(tier::preview, e2::form::layout::expose, area, -)
                {
                    area = expose(window.id);
                    if (area) window.base::riseup(tier::release, e2::form::layout::expose, area);
                };
                window.LISTEN(tier::preview, e2::form::layout::bubble, area, -)
                {
                    area = bubble(window.id);
                    if (area) window.base::riseup(tier::release, e2::form::layout::bubble, area);
                };
                items.push_back(ptr::shared<node>(window_ptr));
            }
            //hall::list: Draw backpane for spectators.
            void prerender(face& canvas)
            {
                if (size() > 1)
                for (auto& item : items) item->fasten(canvas); // Draw strings.
                for (auto& item : items) item->object->render(canvas, true, true, faux); // Draw shadows without postrendering.
            }
            //hall::list: .
            auto visible(auto& item, face& canvas)
            {
                return !item->monoid || item->monoid == canvas.link();
            }
            //hall::list: .
            auto maximized(auto& item, face& canvas)
            {
                return item->monoid == canvas.link();
            }
            //hall::list: Draw windows.
            void render(face& canvas)
            {
                if (items.empty()) return;
                auto head = items.begin();
                auto tail = items.end();
                auto iter = items.end();
                while (iter != head)
                {
                    auto& item = *--iter;
                    if (maximized(item, canvas)) break;
                }
                // Hide all windows behind maximized window.
                auto has_maximized = head != iter || maximized(*iter, canvas);
                if (!has_maximized)
                {
                    head = iter;
                    while (head != tail) { auto& item = *head++; if (visible(item, canvas)) item->fasten(canvas); }
                }
                head = iter; while (head != tail) { auto& item = *head++; if (visible(item, canvas) && item->z_order == zpos::backmost) item->object->render<true>(canvas); }
                head = iter; while (head != tail) { auto& item = *head++; if (visible(item, canvas) && item->z_order == zpos::plain   ) item->object->render<true>(canvas); }
                head = iter; while (head != tail) { auto& item = *head++; if (visible(item, canvas) && item->z_order == zpos::topmost ) item->object->render<true>(canvas); }
            }
            //hall::list: Draw spectator's mouse pointers.
            void postrender(face& canvas)
            {
                for (auto& item : items) if (visible(item, canvas)) item->object->render(canvas, true, faux, true);
            }
            //hall::list: Delete all items.
            void reset()
            {
                items.clear();
            }
            rect remove(id_t item_id)
            {
                auto area = rect{};
                auto head = items.begin();
                auto tail = items.end();
                auto item = search(head, tail, item_id);
                if (item != tail)
                {
                    area = (**item).object->region;
                    items.erase(item);
                }
                return area;
            }
            rect bubble(id_t item_id)
            {
                auto head = items.rbegin();
                auto tail = items.rend();
                auto item = search(head, tail, item_id);

                if (item != head && item != tail)
                {
                    auto area = (**item).object->region;
                    if (!area.trim((**std::prev(item)).object->region))
                    {
                        auto shadow = *item;
                        items.erase(std::next(item).base());

                        while (--item != head
                            && !area.trim((**std::prev(item)).object->region))
                        { }

                        items.insert(item.base(), shadow);
                        return area;
                    }
                }

                return rect_00;
            }
            rect expose(id_t item_id)
            {
                auto head = items.rbegin();
                auto tail = items.rend();
                auto item = search(head, tail, item_id);

                if (item != head && item != tail)
                {
                    auto shadow = *item;
                    items.erase(std::next(item).base());
                    items.push_back(shadow);
                    if (shadow->object->hidden) // Restore if window minimized.
                    {
                        shadow->object->hidden = faux;
                    }
                    return shadow->object->region;
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
        struct depo // hall: Actors registry.
        {
            netxs::sptr<desk::apps> apps_ptr = ptr::shared(desk::apps{});
            netxs::sptr<desk::usrs> usrs_ptr = ptr::shared(desk::usrs{});
            netxs::sptr<desk::menu> menu_ptr = ptr::shared(desk::menu{});
            desk::apps& apps = *apps_ptr;
            desk::usrs& usrs = *usrs_ptr;
            desk::menu& menu = *menu_ptr;

            void append(sptr user)
            {
                usrs.push_back(user);
            }
            auto remove(sptr item_ptr)
            {
                auto found = faux;
                for (auto& [class_id, fxd_app_list] : apps) // Remove app.
                {
                    auto& [fixed, app_list] = fxd_app_list;
                    auto head = app_list.begin();
                    auto tail = app_list.end();
                    auto iter = std::find_if(head, tail, [&](auto& c){ return c == item_ptr; });
                    if (iter != tail)
                    {
                        app_list.erase(iter);
                        if (app_list.empty() && !fixed)
                        {
                            apps.erase(class_id);
                        }
                        found = true;
                        break;
                    }
                }
                { // Remove user.
                    auto head = usrs.begin();
                    auto tail = usrs.end();
                    auto iter = std::find_if(head, tail, [&](auto& c){ return c == item_ptr; });
                    if (iter != tail)
                    {
                        usrs.erase(iter);
                        found = true;
                    }
                }
                return found;
            }
            void reset()
            {
                apps.clear();
            }
        };

        using idls = std::vector<id_t>;
        using pool = netxs::generics::pool;

        list items; // hall: Child visual tree.
        list users; // hall: Scene spectators.
        depo dbase; // hall: Actors registry.
        twod vport; // hall: Last user's viewport position.
        pool async; // hall: Thread pool for parallel task execution.
        id_t focus; // hall: Last active gear id.
        text selected_item; // hall: Override default menu item (if not empty).
        std::unordered_map<id_t, si32> switch_counter; // hall: Focus switch counter.
        input::key::keybind_list_t window_bindings;
        xmls config; // hall: Resultant settings.
        subs tokens; // hall: Subscription tokens.
        flag active; // hall: Host is available for connections.
        std::vector<bool> user_numbering; // hall: .
        pro::maker maker{*this }; // hall: Window creator using drag and drop (right drag).
        pro::robot robot{*this }; // hall: Animation controller.

        auto window(applink& what)
        {
            return ui::cake::ctor()
                ->plugin<pro::d_n_d>()
                ->plugin<pro::ghost>()
                ->plugin<pro::title>(what.header, what.footer)
                ->plugin<pro::notes>(what.header, dent{ 2,2,1,1 })
                ->plugin<pro::sizer>()
                ->plugin<pro::frame>()
                ->plugin<pro::light>()
                ->plugin<pro::focus>()
                ->plugin<pro::keybd>("window")
                ->plugin<pro::luafx>()
                ->limits(dot_11)
                ->invoke([&](auto& boss)
                {
                    auto& mouse = boss.template plugins<pro::mouse>();
                    auto& keybd = boss.template plugins<pro::keybd>();
                    auto& luafx = boss.template plugins<pro::luafx>();
                    keybd.bind(window_bindings);

                    static auto proc_map = pro::luafx::fxmap<base>
                    {
                        { "WarpWindow",         [](auto& boss, auto& luafx)
                                                {
                                                    auto warp = dent{ luafx.get_args_or(1, 0),   // Args...
                                                                      luafx.get_args_or(2, 0),   //
                                                                      luafx.get_args_or(3, 0),   //
                                                                      luafx.get_args_or(4, 0) }; //
                                                    boss.bell::enqueue(boss.This(), [warp](auto& boss) // Keep the focus tree intact while processing key events.
                                                    {
                                                        boss.bell::signal(tier::preview, e2::form::layout::swarp, warp);
                                                    });
                                                    if (auto gear_ptr = luafx.template get_object<hids>("gear")) gear_ptr->set_handled();
                                                    luafx.set_return(); // No returns.
                                                }},
                        { "AlwaysOnTop",        [](auto& boss, auto& luafx)
                                                {
                                                    auto args_count = luafx.args_count();
                                                    auto zorder = zpos::plain;
                                                    if (args_count == 0) // Request zpos.
                                                    {
                                                        zorder = boss.bell::signal(tier::request, e2::form::prop::zorder);
                                                    }
                                                    else // Set zpos.
                                                    {
                                                        zorder = luafx.get_args_or(1, faux) ? zpos::topmost : zpos::plain;
                                                        boss.bell::signal(tier::preview, e2::form::prop::zorder, zorder);
                                                    }
                                                    if (auto gear_ptr = luafx.template get_object<hids>("gear")) gear_ptr->set_handled();
                                                    luafx.set_return(zorder == zpos::topmost);
                                                }},
                        { "Close",              [](auto& boss, auto& luafx)
                                                {
                                                    boss.bell::enqueue(boss.This(), [](auto& boss) // Keep the focus tree intact while processing key events.
                                                    {
                                                        boss.bell::signal(tier::anycast, e2::form::proceed::quit::one, true);
                                                    });
                                                    if (auto gear_ptr = luafx.template get_object<hids>("gear")) gear_ptr->set_handled();
                                                    luafx.set_return();
                                                }},
                        { "ShowClosingPreview", [](auto& boss, auto& luafx)
                                                {
                                                    auto preview_state = luafx.get_args_or(1, faux);
                                                    boss.bell::signal(tier::anycast, e2::form::state::keybd::command::close, preview_state);
                                                    luafx.set_return();
                                                }},
                        { "MinimizeWindow",     [](auto& boss, auto& luafx)
                                                {
                                                    if (auto gear_ptr = luafx.template get_object<hids>("gear"))
                                                    {
                                                        gear_ptr->set_handled();
                                                        boss.bell::enqueue(boss.This(), [gear_id = gear_ptr->id](auto& boss) // Keep the focus tree intact while processing key events.
                                                        {
                                                            if (auto gear_ptr = boss.bell::template getref<hids>(gear_id))
                                                            {
                                                                auto& gear = *gear_ptr;
                                                                boss.bell::signal(tier::release, e2::form::size::minimize, gear);
                                                            }
                                                        });
                                                    }
                                                    luafx.set_return();
                                                }},
                        { "MaximizeWindow",     [](auto& boss, auto& luafx)
                                                {
                                                    if (auto gear_ptr = luafx.template get_object<hids>("gear"))
                                                    {
                                                        gear_ptr->set_handled();
                                                        boss.bell::enqueue(boss.This(), [gear_id = gear_ptr->id](auto& boss) // Keep the focus tree intact while processing key events.
                                                        {
                                                            if (auto gear_ptr = boss.bell::template getref<hids>(gear_id))
                                                            {
                                                                auto& gear = *gear_ptr;
                                                                boss.bell::signal(tier::preview, e2::form::size::enlarge::maximize, gear);
                                                            }
                                                        });
                                                    }
                                                    luafx.set_return();
                                                }},
                        { "Fullscreen",         [](auto& boss, auto& luafx)
                                                {
                                                    if (auto gear_ptr = luafx.template get_object<hids>("gear"))
                                                    {
                                                        gear_ptr->set_handled();
                                                        boss.bell::enqueue(boss.This(), [gear_id = gear_ptr->id](auto& boss) // Keep the focus tree intact while processing key events.
                                                        {
                                                            if (auto gear_ptr = boss.bell::template getref<hids>(gear_id))
                                                            {
                                                                auto& gear = *gear_ptr;
                                                                boss.bell::signal(tier::preview, e2::form::size::enlarge::fullscreen, gear);
                                                            }
                                                        });
                                                    }
                                                    luafx.set_return();
                                                }},
                    };
                    luafx.activate(proc_map);

                    boss.base::kind(base::reflow_root);
                    boss.LISTEN(tier::preview, vtm::events::d_n_d::drop, what, -, (menuid = what.menuid))
                    {
                        if (boss.subset.size())
                        if (auto applet = boss.subset.back())
                        {
                            boss.bell::signal(tier::request, e2::form::prop::ui::header, what.header);
                            boss.bell::signal(tier::request, e2::form::prop::ui::footer, what.footer);
                            what.applet = applet;
                            what.menuid = menuid;
                        }
                    };
                    auto last_state = ptr::shared(faux);
                    boss.LISTEN(tier::release, e2::form::layout::selected, gear, -, (last_state))
                    {
                        *last_state = boss.hidden;
                        boss.hidden = faux; // Restore if it is hidden.
                    };
                    boss.LISTEN(tier::release, e2::form::layout::unselect, gear, -, (last_state))
                    {
                        if (*last_state == true) // Return to hidden state.
                        {
                            boss.hidden = true;
                        }
                    };
                    boss.LISTEN(tier::release, e2::form::size::minimize, gear)
                    {
                        auto This = boss.This();
                        if (boss.hidden) // Restore if it is hidden.
                        {
                            boss.hidden = faux;
                            pro::focus::set(This, gear.id, gear.meta(hids::anyCtrl) ? solo::off : solo::on, true);
                        }
                        else // Hide if visible and refocus.
                        {
                            boss.hidden = true;
                            auto gear_test = boss.base::riseup(tier::request, e2::form::state::keybd::find, { gear.id, 0 });
                            if (auto parent = boss.base::parent())
                            if (gear_test.second) // If it is focused pass the focus to the next desktop window.
                            {
                                gear_test = { gear.id, 0 };
                                parent->bell::signal(tier::request, e2::form::state::keybd::next, gear_test);
                                if (gear_test.second == 1) // If it is the last focused item.
                                {
                                    auto viewport = gear.owner.base::area();
                                    auto window = e2::form::layout::go::prev.param();
                                    auto hidden = true;
                                    auto gear_id = id_t{};
                                    do
                                    {
                                        parent->bell::signal(tier::request, e2::form::layout::go::prev, window);
                                        if (window)
                                        {
                                            window->bell::signal(tier::request, e2::form::state::maximized, gear_id);
                                            hidden = window->hidden;
                                        }
                                        else hidden = true;
                                    }
                                    while (window != This && ((gear_id && gear_id != gear.owner.id) || (hidden == true || !viewport.hittest(window->center()))));
                                    if (window != This)
                                    {
                                        pro::focus::set(window, gear.id, solo::on);
                                        This.reset();
                                    }
                                }
                                if (This) pro::focus::off(This, gear.id);
                            }
                        }
                    };
                    boss.LISTEN(tier::release, e2::form::prop::ui::header, title)
                    {
                        auto tooltip_body = " " + title + " ";
                        boss.bell::signal(tier::preview, e2::form::prop::ui::tooltip, tooltip_body);
                    };
                    boss.LISTEN(tier::release, hids::events::mouse::button::dblclick::left, gear)
                    {
                        boss.base::riseup(tier::preview, e2::form::size::enlarge::maximize, gear);
                        gear.dismiss();
                    };
                    boss.LISTEN(tier::request, e2::form::prop::window::instance, window_ptr)
                    {
                        window_ptr = boss.This();
                    };
                    boss.LISTEN(tier::request, e2::form::prop::window::fullsize, object_area)
                    {
                        auto& title = boss.template plugins<pro::title>();
                        auto t = std::max(1, title.head_size.y);
                        auto b = std::max(1, title.foot_size.y);
                        object_area = boss.base::area() + dent{ 2, 2, t, b };
                    };
                    boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                    {
                        auto home = rect{ -dot_21, boss.base::size() + dot_21 * 2 }; // Including resizer grips.
                        if (!home.hittest(gear.coord))
                        {
                            gear.owner.bell::signal(tier::release, e2::form::layout::jumpto, boss);
                        }
                    };
                    boss.LISTEN(tier::release, hids::events::mouse::button::click::right, gear)
                    {
                        pro::focus::set(boss.This(), gear.id, solo::on);
                    };
                    boss.LISTEN(tier::release, hids::events::mouse::button::click::middle, gear)
                    {
                        pro::focus::set(boss.This(), gear.id, solo::on);
                    };
                    boss.LISTEN(tier::release, e2::form::proceed::quit::any, fast)
                    {
                        mouse.reset();
                        boss.base::detach(); // The object kills itself.
                    };
                    boss.LISTEN(tier::general, e2::conio::quit, deal) // Desktop shutdown.
                    {
                        boss.bell::signal(tier::anycast, e2::form::proceed::quit::one, true); // Schedule a cleanup.
                    };
                    boss.LISTEN(tier::release, e2::dtor, p)
                    {
                        auto start = datetime::now();
                        auto counter = boss.bell::signal(tier::general, e2::cleanup);
                        auto stop = datetime::now() - start;
                        if constexpr (debugmode) log(prompt::hall, "Garbage collection",
                                                    "\n\ttime ", utf::format(stop.count()), "ns",
                                                    "\n\tobjs ", counter.obj_count,
                                                    "\n\trefs ", counter.ref_count,
                                                    "\n\tdels ", counter.del_count);
                    };

                    auto maximize_token_ptr = ptr::shared<subs>();
                    auto viewport_area_ptr = ptr::shared<rect>();
                    auto saved_area_ptr = ptr::shared<rect>();
                    auto& maximize_token = *maximize_token_ptr;
                    auto& viewport_area = *viewport_area_ptr;
                    auto& saved_area = *saved_area_ptr;
                    auto what_copy = what;
                    what_copy.applet = {};
                    boss.LISTEN(tier::preview, e2::form::size::enlarge::fullscreen, gear, -, (what_copy, maximize_token_ptr, saved_area_ptr, viewport_area_ptr))
                    {
                        auto window_ptr = boss.This();
                        if (maximize_token) // Restore maximized window.
                        {
                            boss.bell::signal(tier::release, e2::form::size::restore, window_ptr);
                        }
                        pro::focus::one(window_ptr, gear.id); // Drop all unrelated foci.
                        auto what = what_copy;
                        what.applet = window_ptr;
                        pro::focus::set(window_ptr, gear.id, solo::on, true); // Refocus to demultifocus.
                        window_ptr->base::riseup(tier::request, e2::form::prop::ui::header, what.header);
                        window_ptr->base::riseup(tier::request, e2::form::prop::ui::footer, what.footer);
                        //todo window_ptr->base::riseup(vtm::events::gate::fullscreen...
                        gear.owner.bell::signal(tier::release, vtm::events::gate::fullscreen, what);
                    };
                    boss.LISTEN(tier::release, e2::form::size::restore, item_ptr)
                    {
                        if (maximize_token)
                        {
                            if (saved_area)
                            {
                                saved_area.coor += viewport_area.coor;
                                boss.base::extend(saved_area); // Restore window size and relative coor.
                            }
                            maximize_token.reset();
                            boss.bell::signal(tier::release, e2::form::state::maximized, id_t{});
                        }
                    };
                    boss.LISTEN(tier::preview, e2::form::size::enlarge::maximize, gear)
                    {
                        auto order = boss.base::riseup(tier::request, e2::form::prop::zorder);
                        auto viewport = gear.owner.bell::signal(tier::request, e2::form::prop::viewport);
                        auto recalc = [](auto& boss, auto viewport)
                        {
                            auto new_area = viewport;
                            auto& title = boss.template plugins<pro::title>();
                            if (title.live)
                            {
                                title.recalc(viewport.size);
                                auto t = title.head_size.y;
                                auto b = title.foot_size.y;
                                new_area -= dent{ 0, 0, t, b };
                            }
                            if (boss.base::area() != new_area)
                            {
                                boss.base::extend(new_area);
                            }
                        };
                        if (order == zpos::backmost) // It is a region view. Just resize it.
                        {
                            recalc(boss, viewport - dent{ 2, 2, 0, 0 });
                            return;
                        }

                        auto window_ptr = boss.This();
                        if (maximize_token) // Restore maximized window.
                        {
                            boss.bell::signal(tier::release, e2::form::size::restore, window_ptr);
                        }
                        else
                        {
                            boss.base::riseup(tier::preview, e2::form::layout::expose); // Multiple windows coubld be maximized at the same time.
                            pro::focus::set(window_ptr, gear.id, solo::on, true);
                            auto owner_id = gear.owner.id;
                            saved_area = boss.base::area();
                            saved_area.coor -= viewport.coor;
                            viewport_area = viewport;
                            recalc(boss, viewport);
                            gear.owner.LISTEN(tier::release, e2::form::prop::viewport, viewport, maximize_token)
                            {
                                viewport_area = viewport;
                                recalc(boss, viewport);
                            };
                            gear.owner.LISTEN(tier::release, e2::dtor, p, maximize_token)
                            {
                                boss.bell::signal(tier::release, e2::form::size::restore, boss.This());
                            };
                            boss.LISTEN(tier::preview, e2::area, new_area, maximize_token)
                            {
                                if (new_area != boss.base::area())
                                {
                                    if (new_area.size == boss.base::size()) // Restore saved size.
                                    {
                                        auto anchor = std::clamp(boss.base::anchor, dot_00, std::max(dot_00, new_area.size));
                                        anchor = anchor * saved_area.size / std::max(dot_11, new_area.size);
                                        saved_area.coor = boss.base::coor() - viewport_area.coor; // Compensating header height.
                                        saved_area.coor += boss.base::anchor - anchor; // Follow the mouse cursor.
                                    }
                                    else saved_area = {}; // Preserve current window layout.
                                    boss.bell::signal(tier::release, e2::form::size::restore, boss.This());
                                }
                            };
                            boss.bell::signal(tier::release, e2::form::state::maximized, owner_id);
                        }
                    };
                    boss.LISTEN(tier::request, e2::form::prop::window::state, state)
                    {
                        //todo unify (+fullscreen)
                        state = maximize_token ? winstate::maximized
                                 : boss.hidden ? winstate::minimized
                                               : winstate::normal;
                    };

                    boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent_ptr)
                    {
                        auto& parent = *parent_ptr;
                        parent.LISTEN(tier::preview, e2::form::prop::cwd, path_utf8, boss.relyon)
                        {
                            boss.bell::signal(tier::anycast, e2::form::prop::cwd, path_utf8);
                        };
                    };
                });
        }
        auto create(applink& what)
        {
            bell::signal(tier::request, vtm::events::newapp, what);
            auto window_ptr = hall::window(what);
            auto& cfg = dbase.menu[what.menuid];
            if (cfg.winsize && !what.forced) window_ptr->extend({ what.square.coor, cfg.winsize });
            else                             window_ptr->extend(what.square);
            window_ptr->attach(what.applet);
            if constexpr (debugmode) log(prompt::hall, "App type: ", utf::debase(cfg.type), ", menu item id: ", utf::debase(what.menuid));
            this->branch(what.menuid, window_ptr, !cfg.hidden);
            window_ptr->bell::signal(tier::anycast, e2::form::upon::started, this->This());
            return window_ptr;
        }
        auto loadspec(auto& conf_rec, auto& fallback, auto& item, text menuid, bool splitter = {}, text alias = {})
        {
            conf_rec.splitter   = splitter;
            conf_rec.menuid     = menuid;
            conf_rec.alias      = alias;
            conf_rec.label      = item.take(attr::label,    fallback.label   );
            if (conf_rec.label.empty()) conf_rec.label = conf_rec.menuid;
            conf_rec.hidden     = item.take(attr::hidden,   fallback.hidden  );
            conf_rec.tooltip    = item.take(attr::tooltip,  fallback.tooltip );
            conf_rec.title      = item.take(attr::title,    fallback.title   );
            conf_rec.footer     = item.take(attr::footer,   fallback.footer  );
            conf_rec.winsize    = item.take(attr::winsize,  fallback.winsize );
            conf_rec.wincoor    = item.take(attr::wincoor,  fallback.wincoor );
            conf_rec.winform    = item.take(attr::winform,  fallback.winform, shared::win::options);
            conf_rec.appcfg.cwd = item.take(attr::cwd,      fallback.appcfg.cwd);
            conf_rec.appcfg.cfg = item.take(attr::cfg,      ""s);
            conf_rec.appcfg.cmd = item.take(attr::cmd,      fallback.appcfg.cmd);
            conf_rec.type       = item.take(attr::type,     fallback.type    );
            utf::to_lower(conf_rec.type);
            auto envar          = item.list(attr::env);
            if (envar.empty()) conf_rec.appcfg.env = fallback.appcfg.env;
            else for (auto& v : envar)
            {
                auto value = v->take_value();
                if (value.size())
                {
                    conf_rec.appcfg.env += value + '\0';
                }
            }
            if (conf_rec.title.empty()) conf_rec.title = conf_rec.menuid + (conf_rec.appcfg.cmd.empty() ? ""s : ": " + conf_rec.appcfg.cmd);
            if (conf_rec.appcfg.cfg.empty())
            {
                auto patch = item.list(attr::config);
                if (patch.size())
                {
                    if (fallback.appcfg.cfg.empty() && patch.size() == 1)
                    {
                        conf_rec.appcfg.cfg = patch.front()->snapshot();
                    }
                    else // Merge new configurations with the previous one if it is.
                    {
                        auto head = patch.begin();
                        auto tail = patch.end();
                        auto settings = xml::settings{ fallback.appcfg.cfg.size() ? fallback.appcfg.cfg
                                                                                  : (*head++)->snapshot() };
                        while (head != tail)
                        {
                            auto& p = *head++;
                            settings.fuse(p->snapshot());
                        }
                        conf_rec.appcfg.cfg = settings.utf8();
                    }
                }
            }
        }
        auto vtm_selected(eccc& /*script*/, qiew args)
        {
            if (args)
            {
                selected_item = args;
                for (auto user : dbase.usrs)
                {
                    user->bell::signal(tier::release, e2::data::changed, selected_item);
                }
                return "ok"s;
            }
            else return "skip: id required"s;
        }
        auto vtm_set(eccc& /*script*/, qiew args)
        {
            auto appconf = xml::settings{ "<item " + text{ args } + " />" };
            appconf.cd("item");
            auto itemptr = appconf.homelist.front();
            auto appspec = desk::spec{ .fixed   = true,
                                       .winform = winstate::normal,
                                       .type    = app::vtty::id };
            auto menuid = itemptr->take(attr::id, ""s);
            if (menuid.empty())
            {
                return "skip: 'id=' not specified"s;
            }
            else
            {
                auto splitter = itemptr->take(attr::splitter, faux);
                hall::loadspec(appspec, appspec, *itemptr, menuid, splitter);
                if (!appspec.hidden)
                {
                    auto& [stat, list] = dbase.apps[menuid];
                    stat = true;
                }
                dbase.menu[menuid] = appspec;
                bell::signal(tier::release, desk::events::apps, dbase.apps_ptr);
                return "ok"s;
            }
        }
        auto vtm_del(eccc& /*script*/, qiew args)
        {
            if (args.empty())
            {
                for (auto& [menuid, conf] : dbase.menu)
                {
                    if (dbase.apps.contains(menuid))
                    {
                        auto& [stat, list] = dbase.apps[menuid];
                        if (list.empty()) dbase.apps.erase(menuid);
                        else              stat = faux;
                    }
                }
                dbase.menu.clear();
                bell::signal(tier::release, desk::events::apps, dbase.apps_ptr);
                return "ok"s;
            }
            else
            {
                auto menuid = text{ args };
                if (dbase.menu.contains(menuid))
                {
                    if (dbase.apps.contains(menuid))
                    {
                        auto& [stat, list] = dbase.apps[menuid];
                        if (list.empty()) dbase.apps.erase(menuid);
                        else              stat = faux;
                    }
                    dbase.menu.erase(menuid);
                    bell::signal(tier::release, desk::events::apps, dbase.apps_ptr);
                    return "ok"s;
                }
                else
                {
                    return "skip: 'id=" + menuid + "' not found";
                }
            }
        }
        auto vtm_dtvt(eccc& script, qiew args)
        {
            auto appspec = desk::spec{ .hidden  = true,
                                       .type    = app::dtvt::id,
                                       .gear_id = script.gear_id };
            appspec.appcfg.env = script.env;
            appspec.appcfg.cwd = script.cwd;
            appspec.appcfg.cmd = args;
            appspec.title = args;
            appspec.label = args;
            appspec.tooltip = args;
            bell::signal(tier::request, desk::events::exec, appspec);
            return "ok " + appspec.appcfg.cmd;
        }

    public:
        hall(xipc server, xmls def_config)
            : focus{ id_t{} },
              config{ def_config },
              active{ true }
        {
            auto& canal = *server;

            auto& g = ui::skin::globals();
            app::shared::get_tui_config(config, g);

            plugins<pro::focus>(pro::focus::mode::focusable, faux);
            plugins<pro::keybd>("desktop");
            window_bindings = pro::keybd::load(config, "window");
            auto& luafx = base::plugin<pro::luafx>();
            static auto proc_map = pro::luafx::fxmap<hall>
            {
                { "Shutdown",           [](auto& boss, auto& luafx)
                                        {
                                            auto args_count = luafx.args_count();
                                            auto ok = !args_count || !boss.bell::signal(tier::request, e2::form::layout::go::item);
                                            if (ok)
                                            {
                                                boss.bell::signal(tier::general, e2::shutdown, utf::concat(prompt::repl, "Server shutdown"));
                                            }
                                            luafx.set_return(ok);
                                        }},
                { "Disconnect",         [](auto& /*boss*/, auto& luafx) //todo Disconnect(gear_id)
                                        {
                                            auto gear_ptr = luafx.template get_object<hids>("gear");
                                            auto ok = !!gear_ptr;
                                            if (ok)
                                            {
                                                gear_ptr->owner.bell::signal(tier::preview, e2::conio::quit);
                                                gear_ptr->set_handled();
                                            }
                                            luafx.set_return(ok);
                                        }},
                { "Run",                [](auto& boss, auto& luafx)
                                        {
                                            auto args_count = luafx.args_count();
                                            auto gear_ptr = luafx.template get_object<hids>("gear");
                                            auto gear_id = gear_ptr ? gear_ptr->id : id_t{};
                                            auto appspec = desk::spec{ .hidden  = true,
                                                                       .winform = winstate::normal,
                                                                       .type    = app::vtty::id,
                                                                       .gear_id = gear_id };
                                            if (!args_count) // Get default app spec.
                                            {
                                                if (gear_ptr)
                                                {
                                                    auto menuid = gear_ptr->owner.bell::signal(tier::request, e2::data::changed);
                                                    appspec = boss.dbase.menu[menuid];
                                                    appspec.fixed = faux;
                                                    appspec.menuid = menuid;
                                                    appspec.gear_id = gear_id;
                                                }
                                            }
                                            else
                                            {
                                                auto utf8_xml = ansi::escx{};
                                                utf8_xml += "<item>";
                                                luafx.read_args([&](qiew key, qiew val)
                                                {
                                                    //log("  %%=%%", key, utf::debase437(val));
                                                    utf8_xml += "<";
                                                    utf::filter_alphanumeric(key, utf8_xml);
                                                    utf8_xml += "=\"";
                                                    utf::escape(val, utf8_xml, '"');
                                                    utf8_xml += "\"/>";
                                                });
                                                utf8_xml += "</item>";
                                                log("%%Run %%", prompt::host, ansi::hi(utf::debase437(utf8_xml)));
                                                auto appconf = xml::settings{ utf8_xml };
                                                appconf.cd("item");
                                                auto itemptr = appconf.homelist.front();
                                                auto menuid = itemptr->take(attr::id, ""s);
                                                if (boss.dbase.menu.contains(menuid))
                                                {
                                                    auto& appbase = boss.dbase.menu[menuid];
                                                    if (appbase.fixed) boss.hall::loadspec(appspec, appbase, *itemptr, menuid);
                                                    else               boss.hall::loadspec(appspec, appspec, *itemptr, menuid);
                                                }
                                                else
                                                {
                                                    if (menuid.empty()) menuid = "vtm.run(" + utf8_xml + ")";
                                                    boss.hall::loadspec(appspec, appspec, *itemptr, menuid);
                                                }
                                            }
                                            auto title = appspec.title.empty() && appspec.label.empty() ? appspec.menuid
                                                       : appspec.title.empty() ? appspec.label
                                                       : appspec.label.empty() ? appspec.title : ""s;
                                            if (appspec.title.empty()) appspec.title = title;
                                            if (appspec.label.empty()) appspec.label = title;
                                            if (appspec.tooltip.empty()) appspec.tooltip = appspec.menuid;
                                            boss.bell::signal(tier::request, desk::events::exec, appspec);
                                            if (gear_ptr) gear_ptr->set_handled();
                                            luafx.set_return();
                                        }},
                { "FocusNextWindow",    [](auto& boss, auto& luafx)
                                        {
                                            auto go_forward = luafx.get_args_or(1, 1) > 0;
                                            auto gear_ptr = luafx.template get_object<hids>("gear");
                                            if (!gear_ptr)
                                            {
                                                luafx.set_return();
                                                return;
                                            }
                                            auto& gear = *gear_ptr;
                                            auto gear_id = gear.id;
                                            auto appspec = desk::spec{ .hidden  = true,
                                                                       .winform = winstate::normal,
                                                                       .type    = app::vtty::id,
                                                                       .gear_id = gear_id };
                                            if (gear.shared_event) // Give another process a chance to handle this event.
                                            {
                                                go_forward ? boss.bell::signal(tier::request, e2::form::layout::focus::next, gear_id)
                                                           : boss.bell::signal(tier::request, e2::form::layout::focus::prev, gear_id);
                                                if (!gear_id)
                                                {
                                                    luafx.set_return();
                                                    return;
                                                }
                                            }
                                            gear.owner.bell::signal(tier::preview, e2::form::proceed::action::restore, gear);

                                            auto window_ptr = boss.bell::signal(tier::request, e2::form::layout::go::item); // Take current window.
                                            if (window_ptr) window_ptr->bell::signal(tier::release, e2::form::layout::unselect, gear);

                                            auto current = window_ptr; 
                                            auto maximized = faux;
                                            auto owner_id = id_t{};
                                            do
                                            {
                                                window_ptr.reset();
                                                owner_id = id_t{};
                                                if (go_forward) boss.bell::signal(tier::request, e2::form::layout::go::prev, window_ptr); // Take prev window.
                                                else            boss.bell::signal(tier::request, e2::form::layout::go::next, window_ptr); // Take next window.
                                                if (window_ptr) window_ptr->bell::signal(tier::request, e2::form::state::maximized, owner_id);
                                                maximized = owner_id == gear.owner.id;
                                                if (!owner_id || maximized) break;
                                            }
                                            while (window_ptr != current); // Skip all foreign maximized windows.

                                            if (window_ptr && (!owner_id || maximized))
                                            {
                                                auto& window = *window_ptr;
                                                window.bell::signal(tier::release, e2::form::layout::selected, gear);
                                                if (!maximized)
                                                {
                                                    gear.owner.bell::signal(tier::release, e2::form::layout::jumpto, window);
                                                }
                                                boss.bell::enqueue(window_ptr, [&, gear_id = gear.id](auto& /*boss*/) // Keep the focus tree intact while processing key events.
                                                {
                                                    pro::focus::set(window.This(), gear_id, solo::on);
                                                });
                                            }
                                            gear.set_handled();
                                            luafx.set_return();
                                        }},
            };
            luafx.activate(proc_map);

            auto current_module_file = os::process::binary();
            auto& apps_list = dbase.apps;
            auto& menu_list = dbase.menu;
            auto  free_list = std::list<std::pair<text, desk::spec>>{};
            auto  temp_list = free_list;
            auto  dflt_spec = desk::spec{ .hidden   = faux,
                                          .winform  = winstate::normal,
                                          .type     = app::vtty::id,
                                          .notfound = true };
            auto find = [&](auto const& menuid) -> auto&
            {
                auto test = [&](auto& p){ return p.first == menuid; };

                auto iter_free = std::find_if(free_list.begin(), free_list.end(), test);
                if (iter_free != free_list.end()) return iter_free->second;

                auto iter_temp = std::find_if(temp_list.begin(), temp_list.end(), test);
                if (iter_temp != temp_list.end()) return iter_temp->second;

                return dflt_spec;
            };

            auto splitter_count = 0;
            auto auto_id = 0;
            auto expand = [&](auto& conf_rec)
            {
                utf::replace_all(conf_rec.title,      "$0", current_module_file);
                utf::replace_all(conf_rec.footer,     "$0", current_module_file);
                utf::replace_all(conf_rec.label,      "$0", current_module_file);
                utf::replace_all(conf_rec.tooltip,    "$0", current_module_file);
                utf::replace_all(conf_rec.appcfg.cmd, "$0", current_module_file);
                utf::replace_all(conf_rec.appcfg.env, "$0", current_module_file);
            };
            for (auto item_ptr : config.list(path::item))
            {
                auto& item = *item_ptr;
                auto splitter = item.take(attr::splitter, faux);
                auto menuid = splitter ? "splitter_" + std::to_string(splitter_count++)
                                       : item.take(attr::id, ""s);
                if (menuid.empty()) menuid = "App" + std::to_string(auto_id++);
                auto alias = item.take(attr::alias, ""s);

                auto& proto = find(menuid);
                if (!proto.notfound) // Update existing record.
                {
                    auto& conf_rec = proto;
                    conf_rec.fixed = true;
                    hall::loadspec(conf_rec, conf_rec, item, menuid, splitter, alias);
                    expand(conf_rec);
                }
                else // New item.
                {
                    auto conf_rec = desk::spec{};
                    conf_rec.fixed = true;
                    auto& dflt = alias.size() ? find(alias) // New based on alias_id.
                                              : dflt_spec;  // New item.
                    hall::loadspec(conf_rec, dflt, item, menuid, splitter, alias);
                    expand(conf_rec);
                    if (conf_rec.hidden) temp_list.emplace_back(std::move(conf_rec.menuid), std::move(conf_rec));
                    else                 free_list.emplace_back(std::move(conf_rec.menuid), std::move(conf_rec));
                }
            }
            for (auto& [menuid, conf_rec] : free_list)
            {
                apps_list[menuid];
                menu_list.emplace(std::move(menuid), std::move(conf_rec));
            }
            for (auto& [menuid, conf_rec] : temp_list)
            {
                menu_list.emplace(std::move(menuid), std::move(conf_rec));
            }

            //todo move to luafx/keybd/ui::base/indexer
            LISTEN(tier::release, e2::command::run, script)
            {
                auto scripting_context = std::unordered_map<text, wptr>{};
                auto shadow = utf::trim(script.cmd, " \r\n\t\f");
                if (shadow.empty()) return;
                luafx.set_object(This(), "desktop");
                if (script.gear_id)
                if (auto gear_ptr = bell::getref<hids>(script.gear_id))
                {
                    luafx.set_object(gear_ptr, "gear");
                }
                auto result = luafx.run_script(script.cmd, scripting_context);
                if (result.empty()) result = "ok";
                log(ansi::clr(yellowlt, shadow), "\n", prompt::lua, result);
                script.cmd = utf::concat(shadow, "\n", prompt::lua, result);
            };
            LISTEN(tier::preview, e2::runscript, gear)
            {
                if (!gear.script_ptr) return;
                if (!gear.scripting_context_ptr) return;
                auto& script_body = *gear.script_ptr;
                auto& scripting_context = *gear.scripting_context_ptr;
                luafx.set_object(gear.This(), "gear");
                luafx.run_script(script_body, scripting_context);
            };

            LISTEN(tier::general, e2::shutdown, msg, tokens)
            {
                if constexpr (debugmode) log(prompt::host, msg);
                active.exchange(faux); // To prevent new applications from launching.
                canal.stop();
            };
            LISTEN(tier::general, e2::cleanup, counter, tokens)
            {
                this->router(tier::general).cleanup(counter.ref_count, counter.del_count);
            };
            LISTEN(tier::general, e2::config::creator, world_ptr, tokens)
            {
                world_ptr = base::This();
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
                else
                {
                    if constexpr (debugmode) log(prompt::host, ansi::err("User accounting error: ring size:", user_numbering.size(), " user_number:", props));
                }
            };
            LISTEN(tier::request, hids::events::focus::set::any, seed, tokens, (focus_tree_map = std::unordered_map<ui64, ui64>{})) // Filter recursive focus loops.
            {
                auto is_recursive = faux;
                if (seed.treeid)
                {
                    auto& digest = focus_tree_map[seed.treeid];
                    if (digest < seed.digest) // This is the first time this focus event has been received.
                    {
                        digest = seed.digest;
                    }
                    else // We've seen this event before.
                    {
                        is_recursive = true;
                    }
                }
                if (!is_recursive)
                {
                    auto deed = this->bell::protos(tier::request);
                    this->bell::signal(tier::release, deed, seed);
                }
            };

            LISTEN(tier::request, vtm::events::apptype, what)
            {
                auto& setup = dbase.menu[what.menuid];
                what.kindid = setup.type;
            };
            LISTEN(tier::request, vtm::events::newapp, what)
            {
                auto& setup = dbase.menu[what.menuid];
                auto& maker = app::shared::builder(setup.type);
                what.applet = maker(setup.appcfg, config);
                what.header = setup.title;
                what.footer = setup.footer;
            };
            LISTEN(tier::general, e2::conio::logs, utf8) // Forward logs from brokers.
            {
                log<faux>(utf8);
            };
            LISTEN(tier::release, e2::form::layout::bubble, area)
            {
                //auto region = items.bubble(inst.bell::id);
                //host::denote(area);
                base::deface();
            };
            LISTEN(tier::release, e2::form::layout::expose, area)
            {
                //auto area = items.expose(inst.bell::id);
                //host::denote(area);
                base::deface();
            };
            LISTEN(tier::request, desk::events::usrs, usrs_ptr)
            {
                usrs_ptr = dbase.usrs_ptr;
            };
            LISTEN(tier::request, desk::events::apps, apps_ptr)
            {
                apps_ptr = dbase.apps_ptr;
            };
            LISTEN(tier::request, desk::events::menu, menu_ptr)
            {
                menu_ptr = dbase.menu_ptr;
            };
            //todo unify
            LISTEN(tier::request, e2::form::layout::go::next, next)
            {
                if (items)
                if (auto next_ptr = items.rotate_next())
                {
                    next = next_ptr->object;
                }
            };
            LISTEN(tier::request, e2::form::layout::go::prev, prev)
            {
                if (items)
                if (auto prev_ptr = items.rotate_prev())
                {
                    prev = prev_ptr->object;
                }
            };
            LISTEN(tier::request, e2::form::layout::go::item, current_item)
            {
                if (items) current_item = items.back();
            };
            LISTEN(tier::request, desk::events::exec, appspec)
            {
                static auto offset = dot_00;
                auto gear_ptr = netxs::sptr<hids>{};
                if (appspec.gear_id == id_t{})
                {
                    //todo revise
                    if (hall::focus) // Take the last active keyboard.
                    {
                        gear_ptr = bell::getref<hids>(hall::focus);
                        //if (gear_ptr)
                        //{
                        //    appspec.gear_id = hall::focus;
                        //}
                    }
                    if (!gear_ptr && users.size()) // Take any existing.
                    {
                        auto gate_ptr = bell::getref<gate>(users.back()->id);
                        auto& gears = gate_ptr->gears;
                        for (auto& [ext_gear_id, _gear_ptr] : gears)
                        {
                            if (ext_gear_id)
                            {
                                gear_ptr = _gear_ptr;
                            }
                        }
                    }
                }
                else gear_ptr = bell::getref<hids>(appspec.gear_id);

                auto gear_id = appspec.gear_id;
                auto menu_id = appspec.menuid;
                auto wincoor = appspec.wincoor;
                auto winsize = appspec.winsize;

                dbase.apps[menu_id];
                auto& appbase = dbase.menu[menu_id];
                auto fixed = appbase.fixed && !appspec.fixed;
                if (fixed) std::swap(appbase, appspec); // Don't modify the base menuitem by the temp appspec.
                else       appbase = appspec;

                auto what = applink{ .menuid = menu_id, .forced = true };
                auto yield = text{};
                if (gear_ptr)
                {
                    auto& gear = *gear_ptr;
                    auto viewport = gear.owner.bell::signal(tier::request, e2::form::prop::viewport);
                    if (wincoor == dot_00)
                    {
                        offset = (offset + dot_21 * 2) % std::max(dot_11, viewport.size * 7 / 32);
                        wincoor = viewport.coor + offset + viewport.size * 1 / 32;
                    }
                    what.square.coor = wincoor;
                    what.square.size = winsize ? winsize : viewport.size * 3 / 4;
                    if (auto window = create(what))
                    {
                        //todo revise: Should the requester set focus on their own behalf?
                        pro::focus::set(window, gear_id/*requested focus*/, solo::on); // Notify pro::focus owners.
                        window->bell::signal(tier::anycast, e2::form::upon::created, gear); // Tile should change the menu item.
                             if (appbase.winform == winstate::maximized)  window->bell::signal(tier::preview, e2::form::size::enlarge::maximize, gear);
                        else if (appbase.winform == winstate::fullscreen) window->bell::signal(tier::release, e2::form::size::enlarge::fullscreen, gear);
                        else if (appbase.winform == winstate::minimized)  window->bell::signal(tier::release, e2::form::size::minimize, gear);
                        yield = utf::concat(window->id);
                    }
                }
                else
                {
                    if (winsize == dot_00) winsize = { 80, 27 };
                    what.square.coor = wincoor;
                    what.square.size = winsize;
                    if (auto window = create(what))
                    {
                        pro::focus::set(window, id_t{}, solo::on);
                        yield = utf::concat(window->id);
                    }
                }
                if (fixed) std::swap(appbase, appspec);
                if (yield.size()) appspec.appcfg.cmd = yield;
            };
            LISTEN(tier::request, e2::form::proceed::createby, gear)
            {
                auto& gate = gear.owner;
                auto what = applink{ .square = gear.slot, .forced = gear.slot_forced };
                gate.bell::signal(tier::request, e2::data::changed, what.menuid);
                if (auto window = create(what))
                {
                    //window->LISTEN(tier::release, e2::form::upon::vtree::detached, master)
                    //{
                    //    log(prompt::hall, "Objects count: ", items.size());
                    //};
                    pro::focus::set(window, gear.id, solo::on);
                    window->bell::signal(tier::anycast, e2::form::upon::created, gear); // Tile should change the menu item.
                    auto& cfg = dbase.menu[what.menuid];
                         if (cfg.winform == winstate::maximized)  window->bell::signal(tier::preview, e2::form::size::enlarge::maximize, gear);
                    else if (cfg.winform == winstate::fullscreen) window->bell::signal(tier::release, e2::form::size::enlarge::fullscreen, gear);
                    else if (cfg.winform == winstate::minimized)  window->bell::signal(tier::release, e2::form::size::minimize, gear);
                }
            };
            LISTEN(tier::request, vtm::events::handoff, what)
            {
                auto& cfg = dbase.menu[what.menuid];
                auto window_ptr = window(what);
                if (what.square) window_ptr->extend(what.square);
                window_ptr->attach(what.applet);
                this->branch(what.menuid, window_ptr, !cfg.hidden);
                window_ptr->bell::signal(tier::anycast, e2::form::upon::started);
            };
            LISTEN(tier::preview, hids::events::keybd::key::post, gear) // Track last active gear.
            {
                hall::focus = gear.id;
            };
            //todo mimic pro::focus
            LISTEN(tier::release, hids::events::keybd::any, gear) // Last resort for unhandled kb events. Forward the keybd event to the gate for sending it to the outside.
            {
                if (!gear.handled)
                {
                    gear.owner.bell::signal(tier::release, hids::events::keybd::key::post, gear);
                }
            };
            //todo mimic pro::focus (hall has no parent)
            LISTEN(tier::preview, hids::events::focus::set::any, seed) // Forward focus events to the gate for sending it to the outside.
            {
                if (seed.gear_id)
                {
                    if (auto gear_ptr = bell::getref<hids>(seed.gear_id))
                    {
                        auto& gear = *gear_ptr;
                        auto deed = this->bell::protos(tier::preview);
                        gear.owner.bell::signal(tier::preview, deed, seed);
                    }
                }
            };
            LISTEN(tier::release, hids::events::focus::set::any, seed) // Reset the focus switch counter when it is focused from outside.
            {
                switch_counter[seed.gear_id] = {};
            };
            LISTEN(tier::request, e2::form::layout::focus::any, gear_id)
            {
                auto& counter = switch_counter[gear_id];
                auto deed = this->bell::protos(tier::request);
                auto forward = deed == e2::form::layout::focus::next.id;
                if (forward != (counter > 0)) counter = {}; // Reset if direction has changed.
                forward ? counter++ : counter--;
                if (std::abs(counter) >= (si32)items.size())
                {
                    counter = {};
                    gear_id = {};
                }
            };
            //todo deduplicate (ui::gate)
            LISTEN(tier::general, e2::config::fps, fps)
            {
                if (fps > 0)
                {
                    g.maxfps = fps;
                    log(prompt::hall, "Rendering refresh rate: ", g.maxfps, " fps");
                }
                else if (fps < 0)
                {
                    fps = g.maxfps;
                }
            };
            bell::signal(tier::general, e2::config::fps, g.maxfps);
        }

        // hall: Autorun apps from config.
        void autorun()
        {
            vport = config.take(path::viewport, dot_00);
            auto what = applink{};
            auto apps = config.list(path::autorun);
            auto foci = book{};
            foci.reserve(apps.size());
            for (auto app_ptr : apps)
            {
                auto& app = *app_ptr;
                if (!app.fake)
                {
                    what.menuid =   app.take(attr::id, ""s);
                    what.square = { app.take(attr::wincoor, dot_00),
                                    app.take(attr::winsize, twod{ 80,27 }) };
                    auto winform =  app.take(attr::winform, winstate::normal, shared::win::options);
                    auto focused =  app.take(attr::focused, faux);
                    what.forced = !!what.square.size;
                    if (what.menuid.size())
                    {
                        auto window_ptr = create(what);
                        if (winform == winstate::minimized) window_ptr->base::hidden = true;
                        else if (focused) foci.push_back(window_ptr);
                    }
                    else log(prompt::hall, "Unexpected empty app id in autorun configuration");
                }
            }
            auto count = 0;
            for (auto& window_ptr : foci)
            {
                pro::focus::set(window_ptr, id_t{}, count++ ? solo::off : solo::on); // Reset all foci on the first item.
            }
            if constexpr (debugmode)
            {
                auto gear_test = bell::signal(tier::request, e2::form::state::keybd::next);
                if (gear_test.second) log(prompt::hall, "Autofocused items count: ", gear_test.second);
            }
        }
        // hall: .
        void redraw(face& canvas)
        {
            users.prerender(canvas);  // Draw backpane for spectators.
            items.render(canvas);     // Draw objects of the world.
            users.postrender(canvas); // Draw spectator's mouse pointers.
        }
        // hall: .
        template<class P>
        void run(P process)
        {
            async.run(process);
        }
        // hall: Attach a new item to the scene.
        void branch(text const& menuid, sptr item, bool fixed = true)
        {
            if (!active) return;
            items.append(item);
            item->base::root(true);
            auto& [stat, list] = dbase.apps[menuid];
            stat = fixed;
            list.push_back(item);
            item->bell::signal(tier::release, e2::form::upon::vtree::attached, base::This());
            item->bell::signal(tier::anycast, vtm::events::attached, base::This());
            this->bell::signal(tier::release, desk::events::apps, dbase.apps_ptr);
        }
        // hall: Create a new user gate.
        auto invite(xipc client, view userid, si32 vtmode, eccc usrcfg, xmls app_config, si32 session_id)
        {
            if (selected_item.size()) app_config.set("/config/desktop/taskbar/selected", selected_item);
            auto lock = bell::unique_lock();
            auto user_ptr = hall::ctor<gate>(client, userid, vtmode, app_config, session_id);
            auto& user = *user_ptr;
            users.append(user_ptr);
            dbase.append(user_ptr);
            os::ipc::users = users.size();
            user.bell::signal(tier::release, e2::form::upon::vtree::attached, base::This());
            this->bell::signal(tier::release, desk::events::usrs, dbase.usrs_ptr);

            user.LISTEN(tier::release, e2::form::layout::shift, newpos)
            {
                auto viewport = user.bell::signal(tier::request, e2::form::prop::viewport);
                auto oldpos = viewport.center();
                auto path = oldpos - newpos;
                auto time = datetime::round<si32>(skin::globals().switching);
                auto init = 0;
                auto func = constlinearAtoB<twod>(path, time, init);
                robot.pacify(user.id);
                robot.actify(user.id, func, [&](auto& x)
                {
                    user.base::moveby(-x);
                    user.base::strike();
                });
            };
            user.LISTEN(tier::release, e2::form::layout::jumpto, window_inst)
            {
                auto viewport = user.bell::signal(tier::request, e2::form::prop::viewport);
                auto object_area = window_inst.bell::signal(tier::request, e2::form::prop::window::fullsize);
                auto outside = viewport | object_area;
                if (outside != viewport)
                {
                    auto coor = outside.coor.equals(object_area.coor, object_area.coor, outside.coor + outside.size - viewport.size);
                    auto center = viewport.center() + coor - viewport.coor;
                    user.bell::signal(tier::release, e2::form::layout::shift, center);
                }
            };
            user.LISTEN(tier::release, hids::events::mouse::button::click::left, gear) // Go to another user's viewport.
            {
                if (gear.owner.id == user.id) return;
                auto center = user.base::coor() + gear.owner.base::size() / 2;
                gear.owner.bell::signal(tier::release, e2::form::layout::shift, center);
            };
            //todo move it to the desk (dragging)
            auto drag_origin_ptr = ptr::shared<fp2d>();
            auto& drag_origin = *drag_origin_ptr;
            auto& user_mouse = user.plugins<pro::mouse>();
            user_mouse.template draggable<hids::buttons::leftright>(true);
            user_mouse.template draggable<hids::buttons::left>(true);
            user.LISTEN(tier::release, e2::form::drag::start::any, gear, -, (drag_origin_ptr))
            {
                if (gear.owner.id != user.id) return;
                robot.pacify(user.id);
                drag_origin = gear.coord;
            };
            user.LISTEN(tier::release, e2::form::drag::pull::any, gear)
            {
                if (gear.owner.id != user.id) return;
                if (auto delta = twod{ gear.coord } - twod{ drag_origin })
                {
                    drag_origin = gear.coord;
                    user.base::moveby(-delta);
                    user.base::deface();
                }
            };
            user.LISTEN(tier::release, e2::form::drag::stop::any, gear)
            {
                if (gear.owner.id != user.id) return;
                robot.pacify(user.id);
                robot.actify(user.id, gear.fader<quadratic<twod>>(2s), [&](auto& x)
                {
                    user.base::moveby(-x);
                    user.base::deface();
                });
            };
            user.LISTEN(tier::release, e2::conio::winsz, new_size)
            {
                // Do not wait timer tick.
                auto damaged = true;
                if (damaged)
                {
                    auto& canvas = user.xmap;
                    canvas.wipe(this->id);
                    if (user.what.applet)
                    {
                        if (auto context = canvas.change_basis(user.base::area()))
                        {
                            user.what.applet->render(canvas);
                        }
                    }
                    else
                    {
                        if (user.props.background_image.size())
                        {
                            //todo cache background
                            canvas.tile(user.props.background_image, cell::shaders::fuse);
                        }
                        redraw(canvas); // Put the rest of the world on my canvas.
                        if (user.applet) // Render main menu/application.
                        if (auto context = canvas.change_basis(user.base::area()))
                        {
                            user.applet->render(canvas);
                        }
                    }
                }
                user.rebuild_scene(damaged);
            };
            user.LISTEN(tier::general, e2::timer::any, timestamp)
            {
                auto damaged = base::ruined();//!host::debris.empty();
                //host::debris.clear();
                if (damaged)
                {
                    auto& canvas = user.xmap;
                    canvas.wipe(this->id);
                    if (user.what.applet)
                    {
                        if (auto context = canvas.change_basis(user.base::area()))
                        {
                            user.what.applet->render(canvas);
                        }
                    }
                    else
                    {
                        if (user.props.background_image.size())
                        {
                            //todo cache background
                            canvas.tile(user.props.background_image, cell::shaders::fuse);
                        }
                        redraw(canvas); // Put the rest of the world on my canvas.
                        if (user.applet) // Render main menu/application.
                        if (auto context = canvas.change_basis(user.base::area()))
                        {
                            user.applet->render(canvas);
                        }
                    }
                }
                user.rebuild_scene(damaged);
            };
            usrcfg.cfg = utf::concat(user.id, ";", user.props.os_user_id, ";", user.props.selected);
            auto deskmenu = app::shared::builder(app::desk::id)(usrcfg, app_config);
            user.attach(deskmenu);
            user.base::resize(usrcfg.win);
            if (vport) user.base::moveto(vport); // Restore user's last position.
            lock.unlock();
            user.launch();
        }
        // hall: Detach user/window.
        void remove(sptr item_ptr) override
        {
            auto& inst = *item_ptr;
            auto del1 = items.remove(inst.id);
            //host::denote(del1);
            base::deface();
            auto block = users.remove(inst.id);
            os::ipc::users = users.size();
            if (block) // Save user's viewport last position.
            {
                //host::denote(block);
                base::deface();
                vport = block.coor;
            }
            if (dbase.remove(item_ptr))
            {
                inst.bell::signal(tier::release, e2::form::upon::vtree::detached, This());
            }
            if (items.size() && !block/*don't refocus on user disconnect*/) // Pass focus to the top most object.
            {
                auto last_ptr = items.back();
                auto gear_id_list = item_ptr->base::riseup(tier::request, e2::form::state::keybd::enlist);
                for (auto gear_id : gear_id_list)
                {
                    if (auto gear_ptr = bell::getref<hids>(gear_id))
                    {
                        auto gear_test = bell::signal(tier::request, e2::form::state::keybd::next, { gear_id, 0 });
                        if (gear_test.second == 1) // If it is the last focused item.
                        {
                            auto owner_id = last_ptr->bell::signal(tier::request, e2::form::state::maximized);
                            if (owner_id && owner_id != gear_ptr->owner.id) continue;
                            pro::focus::set(last_ptr, gear_id, solo::off);
                        }
                    }
                }
            }
            bell::signal(tier::release, desk::events::apps, dbase.apps_ptr); // Update taskbar app list.
        }
        // hall: Shutdown.
        void stop()
        {
            log(prompt::hall, "Server shutdown");
            bell::signal(tier::general, e2::conio::quit); // Trigger to disconnect all users and monitors.
            async.stop(); // Wait until all users and monitors are disconnected.
            if constexpr (debugmode) log(prompt::hall, "Session control stopped");
            bell::dequeue(); // Wait until all cleanups are completed.
            auto lock = bell::sync();
            plugins<pro::mouse>().reset(); // Release the captured mouse.
            tokens.reset();
            dbase.reset();
            items.reset();
        }
    };
}