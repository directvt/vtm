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
        text type{};
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
        static constexpr auto selected = "/config/desktop/taskbar/selected";
        static constexpr auto viewport = "/config/desktop/viewport/coor";
    }

    namespace events
    {
        EVENTPACK( ui::e2::extra::slot1 )
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
            si32  z_order;
            fp2d  drag_origin;

        public:
            frame(base&&) = delete;
            frame(base& boss) : skill{ boss },
                robo{ boss    },
                z_order{ zpos::plain }
            {
                boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent, memo)
                {
                    boss.base::signal(tier::release, e2::form::prop::zorder, z_order);
                };
                boss.LISTEN(tier::preview, e2::form::prop::zorder, order)
                {
                    z_order = order;
                    boss.base::signal(tier::release, e2::form::prop::zorder, z_order);
                };
                boss.LISTEN(tier::request, e2::form::prop::zorder, order)
                {
                    order = z_order;
                };
                boss.LISTEN(tier::preview, input::events::mouse::button::click::left, gear, memo)
                {
                    //todo window.events(onclick)
                    boss.base::riseup(tier::preview, e2::form::layout::expose);
                };
                boss.LISTEN(tier::preview, input::events::mouse::button::click::right, gear, memo)
                {
                    //todo window.events(onclick)
                    boss.base::riseup(tier::preview, e2::form::layout::expose);
                };
                boss.LISTEN(tier::preview, e2::form::layout::appear, newpos, memo)
                {
                    appear(newpos);
                };
                boss.LISTEN(tier::preview, e2::form::upon::changed, delta, memo)
                {
                    boss.base::riseup(tier::preview, e2::form::layout::bubble);
                };
                boss.LISTEN(tier::preview, input::events::mouse::button::down::any, gear, memo)
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
                                    boss.base::signal(tier::preview, e2::area, preview_area);
                                    boss.base::moveby(delta);
                                    boss.base::signal(tier::preview, e2::form::upon::changed, delta);
                                    boss.base::strike();
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
                        robo.actify(gear.fader<quadratic<twod>>(2s), [&](auto delta)
                        {
                            boss.base::moveby(delta);
                            boss.base::signal(tier::preview, e2::form::upon::changed, delta);
                            boss.base::strike();
                        });
                    }
                    else
                    {
                        auto boundary = gear.owner.base::signal(tier::request, e2::form::prop::viewport);
                        robo.actify(gear.fader<quadratic<twod>>(2s), [&, boundary](auto delta)
                        {
                            //todo revise: crash after window closed (bad weak ptr)
                            convey(delta, boundary);
                            boss.base::signal(tier::preview, e2::form::upon::changed, delta);
                            boss.base::strike();
                        });
                    }
                };
                boss.LISTEN(tier::release, input::events::mouse::button::click::right, gear, memo)
                {
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
                robo.actify(func, [&](auto delta)
                {
                    boss.base::moveby(delta);
                    boss.base::strike();
                });
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
                    boss.base::deface(data.slot);
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
                    slot.coor = init = step = gear.click + gear.owner.coor();
                    slot.size = dot_00;
                    boss.base::deface(slot);
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
                    if (moved || sized) boss.base::deface(slot);
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
                        gear.slot_forced = true;
                        if (gear.meta(hids::anyCtrl))
                        {
                            log(prompt::hall, "Area copied to clipboard ", gear.slot);
                            gear.owner.base::signal(tier::release, e2::command::printscreen, gear);
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
                : skill{ boss }
            {
                namespace drag = input::events::mouse::button::drag;

                boss.LISTEN(tier::preview, input::events::keybd::key::post, gear, memo)
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

                boss.LISTEN(tier::general, input::events::halt, gear, memo)
                {
                    handle_drop(gear);
                };

                boss.LISTEN(tier::release, e2::postrender, canvas, memo)
                {
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
                                auto coor = area.coor;
                                coor.y--;
                                header.move(coor);
                                canvas.fill(header, cell::shaders::contrast);
                            }
                            else
                            {
                                auto mark = cell{ skin::color(tone::brighter) }.txt(" ");
                                auto temp = canvas.clip();
                                canvas.clip(area);
                                canvas.fill(area, [&](cell& c){ c.fuse(mark); c.und(faux); });
                                canvas.blur(5, cache);
                                coder.wrp(wrap::off).add(' ').add(slot.size.x).add(" × ").add(slot.size.y).add(' ');
                                //todo optimize para
                                auto caption = para{ coder };
                                coder.clear();
                                auto header = *caption.lyric;
                                auto coor = area.coor + area.size;
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
                boss.base::signal(tier::anycast, e2::form::prop::lucidity, 0xFF); // Make target opaque.
                auto boss_ptr = boss.This();
                if (auto dest_ptr = cover.lock())
                {
                    auto& dest = *dest_ptr;
                    if (keep)
                    {
                        auto what = boss.base::signal(tier::preview, vtm::events::d_n_d::drop); // Take core.
                        if (what.applet)
                        {
                            auto gear_id_list = pro::focus::cut(what.applet);
                            what.applet->base::detach();
                            dest.base::signal(tier::release, vtm::events::d_n_d::drop, what); // Pass core.
                            pro::focus::set(what.applet, gear.id, solo::on, true); // Set unique focus.
                            boss.base::detach(); // The object kills itself.
                        }
                    }
                    else dest.base::signal(tier::release, vtm::events::d_n_d::abort, boss.This());
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
                boss.LISTEN(tier::release, input::events::mouse::button::drag::start::any, gear, memo)
                {
                    if (boss.size().inside(gear.coord) && !gear.meta(hids::anyMod))
                    if (drags || !gear.capture(boss.id)) return;
                    {
                        drags = true;
                        coord = gear.coord;
                        under = {};
                    }
                };
                boss.LISTEN(tier::release, input::events::mouse::button::drag::pull::any, gear, memo)
                {
                    if (!drags) return;
                    if (gear.meta(hids::anyMod)) proceed(faux, gear);
                    else                         coord = gear.coord - gear.delta.get();
                };
                boss.LISTEN(tier::release, input::events::mouse::button::drag::stop::any, gear, memo)
                {
                    if (!drags) return;
                    if (gear.meta(hids::anyMod)) proceed(faux, gear);
                    else                         proceed(true, gear);
                    gear.setfree();
                };
                boss.LISTEN(tier::release, input::events::mouse::button::drag::cancel::any, gear, memo)
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
                            boss.base::signal(tier::anycast, e2::form::prop::lucidity, object ? 0x80 : 0xFF); // Make it semi-transparent on success and opaque otherwise.
                            cover = object;
                            under = new_under;
                        }
                    }
                };
            }
        };
    }

    // vtm: Borderless container of floating windows.
    struct hall
        : public form<hall>
    {
    private:
        // hall: Desktop window.
        struct window_t : ui::form<window_t>
        {
            hall& world;
            bool highlighted = faux;
            si32 active = 0;
            tone color = { tone::brighter, tone::shadower };
            si32 z_order = zpos::plain;
            
            void window_swarp(dent warp)
            {
                bell::enqueue(This(), [warp](auto& boss) // Keep the focus tree intact while processing events.
                {
                    boss.base::signal(tier::preview, e2::form::layout::swarp, warp);
                });
            }
            auto window_alwaysontop(arch args_count, bool args)
            {
                auto zorder = zpos::plain;
                if (args_count == 0) // Request zpos.
                {
                    zorder = base::signal(tier::request, e2::form::prop::zorder);
                }
                else // Set zpos.
                {
                    zorder = args ? zpos::topmost : zpos::plain;
                    base::signal(tier::preview, e2::form::prop::zorder, zorder);
                }
                return zorder == zpos::topmost;
            }
            void window_close(id_t gear_id)
            {
                bell::enqueue(This(), [gear_id](auto& boss) // Keep the focus tree intact while processing events.
                {
                    if (auto gear_ptr = boss.bell::template getref<hids>(gear_id)) //todo Apple clang requires template
                    {
                        auto& gear = *gear_ptr;
                        gear.set_multihome();
                    }
                    boss.base::signal(tier::anycast, e2::form::proceed::quit::one, true);
                });
            }
            void window_state(id_t gear_id, auto state)
            {
                bell::enqueue(This(), [state, gear_id](auto& boss) // Keep the focus tree intact while processing events.
                {
                    if (auto gear_ptr = boss.bell::template getref<hids>(gear_id))
                    {
                        auto& gear = *gear_ptr;
                        gear.set_multihome();
                        boss.base::signal(tier::preview, state, gear);
                    }
                });
            }

        protected: 
            // window: .
            void deform(rect& new_area) override
            {
                if (base::subset.size())
                if (auto& applet_ptr = base::subset.back())
                {
                    applet_ptr->base::recalc(new_area);
                }
            }
            // window: .
            void inform(rect new_area) override
            {
                if (base::subset.size())
                if (auto& applet_ptr = base::subset.back())
                {
                    applet_ptr->base::notify(new_area);
                }
            }

        public:
            window_t(hall& owner, applink& what)
                : world{ owner }
            {
                base::plugin<pro::d_n_d>();
                base::plugin<pro::ghost>();
                auto& title = base::plugin<pro::title>(what.applet->base::property("window.header"), what.applet->base::property("window.footer"));
                base::plugin<pro::notes>(what.applet->base::property("window.footer"), dent{ 2,2,1,1 });
                base::plugin<pro::sizer>();
                base::plugin<pro::frame>();
                base::plugin<pro::light>();
                base::plugin<pro::focus>();
                auto& mouse = base::plugin<pro::mouse>();
                auto& keybd = base::plugin<pro::keybd>("window");
                auto& luafx = base::plugin<pro::luafx>();
                base::limits(dot_11);
                base::kind(base::reflow_root);
                base::root(true);

                auto& bindings = world.base::property<input::key::keybind_list_t>("window.bindings"); // Shared key bindings across the hall.
                if (bindings.empty()) bindings = pro::keybd::load(world.config, "window");
                keybd.bind(bindings);
                luafx.activate("window.proc_map",
                {
                    { "Warp",               [&]
                                            {
                                                auto warp = dent{ luafx.get_args_or(1, 0),   // Args...
                                                                  luafx.get_args_or(2, 0),   //
                                                                  luafx.get_args_or(3, 0),   //
                                                                  luafx.get_args_or(4, 0) }; //
                                                window_swarp(warp);
                                                if (auto gear_ptr = luafx.template get_object<hids>("gear")) gear_ptr->set_handled();
                                                luafx.set_return(); // No returns.
                                            }},
                    { "AlwaysOnTop",        [&]
                                            {
                                                auto args_count = luafx.args_count();
                                                auto state = window_alwaysontop(args_count, args_count ? luafx.get_args_or(1, faux) : faux);
                                                if (auto gear_ptr = luafx.template get_object<hids>("gear")) gear_ptr->set_handled();
                                                luafx.set_return(state);
                                            }},
                    { "Close",              [&]
                                            {
                                                auto gear_id = id_t{};
                                                if (auto gear_ptr = luafx.template get_object<hids>("gear"))
                                                {
                                                    gear_ptr->set_handled();
                                                    gear_id = gear_ptr->id;
                                                }
                                                window_close(gear_id);
                                                luafx.set_return();
                                            }},
                    { "Minimize",           [&]
                                            {
                                                if (auto gear_ptr = luafx.template get_object<hids>("gear"))
                                                {
                                                    gear_ptr->set_handled();
                                                    window_state(gear_ptr->id, e2::form::size::minimize.id);
                                                }
                                                luafx.set_return();
                                            }},
                    { "Maximize",           [&]
                                            {
                                                if (auto gear_ptr = luafx.template get_object<hids>("gear"))
                                                {
                                                    gear_ptr->set_handled();
                                                    window_state(gear_ptr->id, e2::form::size::enlarge::maximize.id);
                                                }
                                                luafx.set_return();
                                            }},
                    { "Fullscreen",         [&]
                                            {
                                                if (auto gear_ptr = luafx.template get_object<hids>("gear"))
                                                {
                                                    gear_ptr->set_handled();
                                                    window_state(gear_ptr->id, e2::form::size::enlarge::fullscreen.id);
                                                }
                                                luafx.set_return();
                                            }},
                });

                LISTEN(tier::preview, e2::command::gui, gui_cmd)
                {
                    auto hit = true;
                    if (gui_cmd.cmd_id == syscmd::warpwindow)
                    {
                        if (gui_cmd.args.size() == 4)
                        {
                            auto warp = dent{ any_get_or(gui_cmd.args[0]),
                                              any_get_or(gui_cmd.args[1]),
                                              any_get_or(gui_cmd.args[2]),
                                              any_get_or(gui_cmd.args[3]) };
                            window_swarp(warp);
                        }
                    }
                    else if (gui_cmd.cmd_id == syscmd::alwaysontop)
                    {
                        auto args_count = gui_cmd.args.size();
                        window_alwaysontop(args_count, args_count ? any_get_or(gui_cmd.args[0], faux) : faux);
                    }
                    else if (gui_cmd.cmd_id == syscmd::close)
                    {
                        window_close(gui_cmd.gear_id);
                    }
                    else if (gui_cmd.gear_id)
                    {
                        if (gui_cmd.cmd_id == syscmd::minimize)
                        {
                            window_state(gui_cmd.gear_id, e2::form::size::minimize.id);
                        }
                        else if (gui_cmd.cmd_id == syscmd::maximize)
                        {
                            window_state(gui_cmd.gear_id, e2::form::size::enlarge::maximize.id);
                        }
                        else if (gui_cmd.cmd_id == syscmd::fullscreen)
                        {
                            window_state(gui_cmd.gear_id, e2::form::size::enlarge::fullscreen.id);
                        }
                        else hit = faux;
                    }
                    else hit = faux;
                    if (!hit) bell::expire(tier::preview, true);
                };
                LISTEN(tier::preview, vtm::events::d_n_d::drop, what)
                {
                    if (base::subset.size())
                    if (auto applet_ptr = base::subset.back())
                    {
                        what.applet = applet_ptr;
                    }
                };
                auto& last_state = base::field(faux);
                LISTEN(tier::release, e2::form::layout::selected, gear)
                {
                    last_state = base::hidden;
                    base::hidden = faux; // Restore if it is hidden.
                };
                LISTEN(tier::release, e2::form::layout::unselect, gear)
                {
                    if (last_state == true) // Return to hidden state.
                    {
                        base::hidden = true;
                    }
                };
                LISTEN(tier::preview, e2::form::size::minimize, gear)
                {
                    auto window_ptr = This();
                    if (base::hidden) // Restore if it is hidden.
                    {
                        base::hidden = faux;
                        pro::focus::set(window_ptr, gear.id, gear.meta(hids::anyCtrl) ? solo::off : solo::on, true);
                    }
                    else // Hide if visible and refocus.
                    {
                        base::hidden = true;
                        auto gear_test = base::riseup(tier::request, e2::form::state::keybd::find, { gear.id, 0 });
                        if (auto parent = base::parent())
                        if (gear_test.second) // Pass the focus to the next desktop window if boss is focused.
                        {
                            gear_test = { gear.id, 0 };
                            parent->base::signal(tier::request, e2::form::state::keybd::next, gear_test);
                            if (gear_test.second == 1) // If it is the solo focused window.
                            {
                                auto viewport = gear.owner.base::area();
                                auto prev_ptr = e2::form::layout::go::prev.param();
                                auto is_hidden = true;
                                do
                                {
                                    parent->base::signal(tier::request, e2::form::layout::go::prev, prev_ptr);
                                    is_hidden = prev_ptr ? prev_ptr->hidden : true;
                                }
                                while (prev_ptr != window_ptr && (is_hidden == true || !viewport.hittest(prev_ptr->center())));
                                if (prev_ptr != window_ptr)
                                {
                                    pro::focus::set(prev_ptr, gear.id, solo::on);
                                    window_ptr.reset();
                                }
                            }
                            if (window_ptr) pro::focus::off(window_ptr, gear.id);
                        }
                    }
                };
                LISTEN(tier::release, e2::form::prop::ui::header, new_title)
                {
                    auto tooltip_body = " " + new_title + " ";
                    base::signal(tier::preview, e2::form::prop::ui::tooltip, tooltip_body);
                };
                LISTEN(tier::release, input::events::mouse::button::dblclick::left, gear)
                {
                    base::riseup(tier::preview, e2::form::size::enlarge::maximize, gear);
                    gear.dismiss();
                };
                LISTEN(tier::request, e2::form::prop::window::instance, window_ptr)
                {
                    window_ptr = This();
                };
                LISTEN(tier::request, e2::form::prop::window::fullsize, object_area)
                {
                    auto t = std::max(1, title.head_size.y);
                    auto b = std::max(1, title.foot_size.y);
                    object_area = base::area() + dent{ 2, 2, t, b };
                };
                LISTEN(tier::release, input::events::mouse::button::click::left, gear)
                {
                    auto home = rect{ -dot_21, base::size() + dot_21 * 2 }; // Including resizer grips.
                    if (!home.hittest(gear.coord))
                    {
                        gear.owner.base::signal(tier::release, e2::form::layout::jumpto, *this);
                    }
                };
                LISTEN(tier::release, input::events::mouse::button::click::right, gear)
                {
                    pro::focus::set(This(), gear.id, solo::on);
                };
                LISTEN(tier::release, input::events::mouse::button::click::middle, gear)
                {
                    pro::focus::set(This(), gear.id, solo::on);
                };
                LISTEN(tier::release, e2::form::proceed::quit::any, fast)
                {
                    mouse.reset();
                    base::detach(); // The object kills itself.
                };
                LISTEN(tier::general, e2::conio::quit, deal) // Desktop shutdown.
                {
                    base::signal(tier::anycast, e2::form::proceed::quit::one, true); // Schedule a cleanup.
                };
                LISTEN(tier::release, e2::form::upon::vtree::detached, parent_ptr)
                {
                    if constexpr (debugmode)
                    {
                        auto start = datetime::now();
                        auto [ref_count, del_count] = base::cleanup();
                        auto stop = datetime::now() - start;
                        log(prompt::hall, "Garbage collection",
                            "\n\ttime ", utf::format(stop.count()), "ns",
                            "\n\tobjs ", bell::indexer.store.size(),
                            "\n\trefs ", ref_count,
                            "\n\tdels ", del_count);
                    }
                    else base::cleanup();
                };

                auto& maximize_token = base::field<subs>();
                auto& viewport_area = base::field<rect>();
                auto& saved_area = base::field<rect>();
                auto& what_copy = base::field<applink>();
                what_copy = what;
                what_copy.applet = {};
                LISTEN(tier::preview, e2::form::size::enlarge::fullscreen, gear)
                {
                    auto window_ptr = This();
                    if (maximize_token.size()) // Restore maximized window.
                    {
                        base::signal(tier::release, e2::form::size::restore);
                    }
                    pro::focus::one(window_ptr, gear.id); // Drop all unrelated foci.
                    auto what = what_copy;
                    what.applet = window_ptr;
                    pro::focus::set(window_ptr, gear.id, solo::on, true); // Refocus to demultifocus.
                    //todo window_ptr->base::riseup(vtm::events::gate::fullscreen...
                    gear.owner.base::signal(tier::release, vtm::events::gate::fullscreen, what);
                };
                LISTEN(tier::release, e2::form::size::restore, p)
                {
                    if (maximize_token.size())
                    {
                        if (saved_area)
                        {
                            saved_area.coor += viewport_area.coor;
                            base::extend(saved_area); // Restore window size and relative coor.
                        }
                        maximize_token.clear();
                    }
                };
                LISTEN(tier::preview, e2::form::size::enlarge::maximize, gear)
                {
                    auto order = base::riseup(tier::request, e2::form::prop::zorder);
                    auto viewport = gear.owner.base::signal(tier::request, e2::form::prop::viewport);
                    auto recalc = [&](auto viewport)
                    {
                        auto new_area = viewport;
                        if (title.live)
                        {
                            title.recalc(viewport.size);
                            auto t = title.head_size.y;
                            auto b = title.foot_size.y;
                            new_area -= dent{ 0, 0, t, b };
                        }
                        if (base::area() != new_area)
                        {
                            base::extend(new_area);
                        }
                    };
                    if (order == zpos::backmost) // It is a region view. Just resize it.
                    {
                        recalc(viewport - dent{ 2, 2, 0, 0 });
                        return;
                    }

                    auto window_ptr = This();
                    if (maximize_token.size()) // Restore maximized window.
                    {
                        base::signal(tier::release, e2::form::size::restore);
                    }
                    else
                    {
                        base::riseup(tier::preview, e2::form::layout::expose); // Multiple windows coubld be maximized at the same time.
                        pro::focus::set(window_ptr, gear.id, solo::on, true);
                        saved_area = base::area();
                        saved_area.coor -= viewport.coor;
                        viewport_area = viewport;
                        recalc(viewport);
                        gear.owner.LISTEN(tier::release, e2::form::prop::viewport, viewport, maximize_token, (recalc))
                        {
                            viewport_area = viewport;
                            recalc(viewport);
                        };
                        gear.owner.LISTEN(tier::release, e2::form::size::restore, p, maximize_token)
                        {
                            base::signal(tier::release, e2::form::size::restore, p);
                        };
                        LISTEN(tier::preview, e2::form::layout::swarp, warp, maximize_token) // Restore on manual resizing.
                        {
                            saved_area = {}; // Preserve current window size.
                            base::signal(tier::release, e2::form::size::restore);
                            bell::expire(tier::preview, true);
                        };
                        LISTEN(tier::preview, e2::area, new_area, maximize_token)
                        {
                            if (new_area != base::area())
                            {
                                if (new_area.size == base::size()) // Restore saved size.
                                {
                                    auto anchor = std::clamp(base::anchor, dot_00, std::max(dot_00, new_area.size));
                                    anchor = anchor * saved_area.size / std::max(dot_11, new_area.size);
                                    saved_area.coor = base::coor() - viewport_area.coor; // Compensate header height.
                                    saved_area.coor += base::anchor - anchor; // Follow the mouse cursor.
                                }
                                else saved_area = {}; // Preserve current window size.
                                base::signal(tier::release, e2::form::size::restore);
                            }
                        };
                    }
                };
                LISTEN(tier::request, e2::form::prop::window::state, state)
                {
                    //todo unify (+fullscreen)
                    state = maximize_token.size() ? winstate::maximized
                                   : base::hidden ? winstate::minimized
                                                  : winstate::normal;
                };
                LISTEN(tier::preview, e2::form::layout::expose, r)
                {
                    if (base::holder != std::prev(world.base::subset.end()))
                    {
                        world.base::subset.push_back(this->This());
                        world.base::subset.erase(base::holder);
                        base::holder = std::prev(world.base::subset.end());
                        if (base::hidden) // Restore if window minimized.
                        {
                            base::hidden = faux;
                            base::deface();
                        }
                        else base::strike();
                    }
                };
                LISTEN(tier::preview, e2::form::layout::bubble, r)
                {
                    auto area = base::region;
                    auto next = base::holder;
                    if (++next != world.base::subset.end() && !area.trim((*next)->region))
                    {
                        world.base::subset.erase(base::holder);
                        while (++next != world.base::subset.end() && !area.trim((*next)->region))
                        { }
                        base::holder = world.base::subset.insert(next, this->This());
                        base::strike();
                    }
                };
                LISTEN(tier::release, e2::form::prop::zorder, order)
                {
                    z_order = order;
                };
                LISTEN(tier::release, e2::form::state::mouse, state)
                {
                    active = state;
                };
                LISTEN(tier::release, e2::form::state::highlight, state)
                {
                    highlighted = state;
                };
                LISTEN(tier::release, e2::form::state::color, new_color)
                {
                    color = new_color;
                };
                LISTEN(tier::release, e2::render::any, parent_canvas)
                {
                    if (auto context = form::nested_context(parent_canvas))
                    {
                        if (base::subset.size())
                        if (auto& applet_ptr = base::subset.back())
                        {
                            applet_ptr->render(parent_canvas);
                        }
                    }
                };
            }
        };

        std::list<std::pair<sptr, para>> users; // hall: Desktop users.
        netxs::generics::pool async; // hall: Thread pool for parallel task execution.
        xmls config; // hall: Resultant settings.
        pro::maker maker{*this }; // hall: Window creator using drag and drop (right drag).
        pro::robot robot{*this }; // hall: Animation controller.

        netxs::sptr<desk::apps> apps_list_ptr = ptr::shared<desk::apps>();
        netxs::sptr<desk::usrs> usrs_list_ptr = ptr::shared<desk::usrs>();
        netxs::sptr<desk::menu> menu_list_ptr = ptr::shared<desk::menu>();
        desk::apps& apps_list = *apps_list_ptr;
        desk::usrs& usrs_list = *usrs_list_ptr;
        desk::menu& menu_list = *menu_list_ptr;

        auto create_window(applink& what, bool is_handoff = faux)
        {
            if (!is_handoff)
            {
                base::signal(tier::request, vtm::events::newapp, what);
            }
            auto window_ptr = window_t::ctor(*this, what);
            attach(window_ptr);

            auto& menuid = what.applet->base::property("window.menuid");
            auto& cfg = menu_list[menuid];
            auto& [fixed_menu_item, inst_list] = apps_list[menuid];
            fixed_menu_item = !cfg.hidden;
            inst_list.push_back(window_ptr);
            auto& inst_list_iter = window_ptr->base::field(std::prev(inst_list.end()));
            if constexpr (debugmode) log(prompt::hall, "App type: ", utf::debase(cfg.type), ", menu item id: ", utf::debase(menuid));

            auto& applet_area = what.applet->base::bind_property("window.area", *window_ptr, e2::area);
                 if (applet_area)                 window_ptr->base::extend(applet_area);
            else if (cfg.winsize && !what.forced) window_ptr->base::extend({ what.square.coor, cfg.winsize });
            else if (what.square)                 window_ptr->base::extend(what.square);

            window_ptr->attach(what.applet);

            auto& window = *window_ptr;
            window.LISTEN(tier::release, e2::form::upon::vtree::detached, world_ptr)
            {
                if (base::subset.size()) // Pass focus to the top most object.
                {
                    auto last_ptr = base::subset.back();
                    auto gear_id_list = window.base::riseup(tier::request, e2::form::state::keybd::enlist);
                    for (auto gear_id : gear_id_list)
                    {
                        if (auto gear_ptr = bell::getref<hids>(gear_id))
                        {
                            auto gear_test = base::signal(tier::request, e2::form::state::keybd::next, { gear_id, 0 });
                            if (gear_test.second == 1) // If it is the last focused item.
                            {
                                pro::focus::set(last_ptr, gear_id, solo::off);
                            }
                        }
                    }
                }
                inst_list.erase(inst_list_iter);
                base::signal(tier::release, desk::events::apps, apps_list_ptr); // Update taskbar app list.
            };
            window_ptr->base::signal(tier::anycast, e2::form::upon::started, is_handoff ? sptr{} : base::This());
            base::signal(tier::release, desk::events::apps, apps_list_ptr);
            window_ptr->base::reflow();
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
        // hall: Draw a navigation string.
        void fasten(sptr object_ptr, auto highlighted, auto item_is_active, auto& color, face& canvas)
        {
            auto window = canvas.area();
            auto center = object_ptr->region.center();
            if (window.hittest(center) || object_ptr->hidden) return;
            auto origin = window.size / 2;
            center -= window.coor;
            //auto origin = twod{ 6, window.size.y - 3 };
            //header.usable = window.overlap(region);
            auto is_active = item_is_active || highlighted;
            auto& grade = skin::grade(is_active ? color.active
                                                : color.passive);
            auto obj_id = object_ptr->id;
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
        auto focus_next_window(hids& gear, si32 dir)
        {
            auto go_forward = dir > 0;
            gear.set_multihome();
            auto gear_id = gear.id;
            auto appspec = desk::spec{ .hidden  = true,
                                       .winform = winstate::normal,
                                       .type    = app::vtty::id,
                                       .gear_id = gear_id };
            if (gear.shared_event) // Give another process a chance to handle this event.
            {
                go_forward ? base::signal(tier::request, e2::form::layout::focus::next, gear_id)
                           : base::signal(tier::request, e2::form::layout::focus::prev, gear_id);
                if (!gear_id)
                {
                    return faux;
                }
            }
            gear.owner.base::signal(tier::preview, e2::form::size::restore);

            auto window_ptr = base::signal(tier::request, e2::form::layout::go::item); // Take current window.
            if (window_ptr) window_ptr->base::signal(tier::release, e2::form::layout::unselect, gear); // Hide current window if it was hidden before focusing.

            auto current = window_ptr; 
            window_ptr.reset();
            if (go_forward) base::signal(tier::request, e2::form::layout::go::prev, window_ptr); // Take prev window.
            else            base::signal(tier::request, e2::form::layout::go::next, window_ptr); // Take next window.

            if (window_ptr)
            {
                auto& window = *window_ptr;
                window.base::signal(tier::release, e2::form::layout::selected, gear);
                gear.owner.base::signal(tier::release, e2::form::layout::jumpto, window);
                bell::enqueue(window_ptr, [&, gear_id = gear.id](auto& /*boss*/) // Keep the focus tree intact while processing events.
                {
                    pro::focus::set(window.This(), gear_id, solo::on);
                });
            }
            return true;
        }

    public:
        hall(xipc server, xmls def_config)
            : config{ def_config }
        {
            auto& canal = *server;

            app::shared::get_tui_config(config, ui::skin::globals());

            base::plugin<pro::focus>(pro::focus::mode::focusable);
            auto& keybd = base::plugin<pro::keybd>("desktop");
            auto& luafx = base::plugin<pro::luafx>();
            auto bindings = pro::keybd::load(config, "desktop");
            keybd.bind(bindings);
            luafx.activate("hall.proc_map",
            {
                { "Shutdown",           [&]
                                        {
                                            auto args_count = luafx.args_count();
                                            auto ok = !args_count || !base::signal(tier::request, e2::form::layout::go::item);
                                            if (ok)
                                            {
                                                base::signal(tier::general, e2::shutdown, utf::concat(prompt::repl, "Server shutdown"));
                                            }
                                            luafx.set_return(ok);
                                        }},
                { "Disconnect",         [&] //todo Disconnect(gear_id)
                                        {
                                            auto gear_ptr = luafx.template get_object<hids>("gear");
                                            auto ok = !!gear_ptr;
                                            if (ok)
                                            {
                                                gear_ptr->owner.base::signal(tier::preview, e2::conio::quit);
                                                gear_ptr->set_handled();
                                            }
                                            luafx.set_return(ok);
                                        }},
                { "Run",                [&]
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
                                                    auto& current_default = gear_ptr->owner.base::property("desktop.selected");
                                                    appspec = menu_list[current_default];
                                                    appspec.fixed = faux;
                                                    appspec.menuid = current_default;
                                                    appspec.gear_id = gear_id;
                                                }
                                            }
                                            else
                                            {
                                                auto utf8_xml = ansi::escx{};
                                                utf8_xml += "<item>";
                                                luafx.read_args(1, [&](qiew key, qiew val)
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
                                                if (menu_list.contains(menuid))
                                                {
                                                    auto& appbase = menu_list[menuid];
                                                    if (appbase.fixed) hall::loadspec(appspec, appbase, *itemptr, menuid);
                                                    else               hall::loadspec(appspec, appspec, *itemptr, menuid);
                                                }
                                                else
                                                {
                                                    if (menuid.empty()) menuid = "vtm.run(" + utf8_xml + ")";
                                                    hall::loadspec(appspec, appspec, *itemptr, menuid);
                                                }
                                            }
                                            auto title = appspec.title.empty() && appspec.label.empty() ? appspec.menuid
                                                       : appspec.title.empty() ? appspec.label
                                                       : appspec.label.empty() ? appspec.title : ""s;
                                            if (appspec.title.empty()) appspec.title = title;
                                            if (appspec.label.empty()) appspec.label = title;
                                            if (appspec.tooltip.empty()) appspec.tooltip = appspec.menuid;
                                            base::signal(tier::request, desk::events::exec, appspec);
                                            if (gear_ptr) gear_ptr->set_handled();
                                            luafx.set_return();
                                        }},
                { "FocusNextWindow",    [&]
                                        {
                                            if (auto gear_ptr = luafx.template get_object<hids>("gear"))
                                            {
                                                auto& gear = *gear_ptr;
                                                auto dir = luafx.get_args_or(1, 1);
                                                if (focus_next_window(gear, dir))
                                                {
                                                    gear.set_handled();
                                                }
                                            }
                                            luafx.set_return();
                                        }},
            });

            auto current_module_file = os::process::binary();
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

            LISTEN(tier::release, e2::form::upon::vtree::attached, parent_ptr)
            {
                parent_ptr->base::riseup(tier::release, e2::form::proceed::multihome, This());
            };
            LISTEN(tier::release, e2::command::run, script)
            {
                luafx.set_object(This(), "desktop");
                if (script.gear_id)
                {
                    bell::expire(tier::release, true); // Continue release riseup.
                }
                else
                {
                    luafx.run_script(script);
                }
            };
            LISTEN(tier::preview, e2::runscript, gear)
            {
                luafx.set_object(This(), "desktop");
                bell::expire(tier::preview, true); // Continue preview riseup.
            };
            LISTEN(tier::preview, e2::command::gui, gui_cmd)
            {
                auto hit = faux;
                if (gui_cmd.cmd_id == syscmd::focusnextwindow)
                if (auto gear_ptr = bell::getref<hids>(gui_cmd.gear_id))
                {
                    auto& gear = *gear_ptr;
                    auto dir = any_get_or(gui_cmd.args[0], 1);
                    focus_next_window(gear, dir);
                    hit = true;
                }
                if (!hit) bell::expire(tier::preview, true);
            };
            LISTEN(tier::general, e2::shutdown, msg)
            {
                if constexpr (debugmode) log(prompt::host, msg);
                canal.stop();
            };
            LISTEN(tier::general, e2::config::creator, world_ptr)
            {
                world_ptr = base::This();
            };

            auto& user_numbering = base::field<std::vector<bool>>();
            LISTEN(tier::general, input::events::device::user::login, props)
            {
                props = 0;
                while (props < user_numbering.size() && user_numbering[props]) { props++; }
                if (props == user_numbering.size()) user_numbering.push_back(true);
                else                                user_numbering[props] = true;
            };
            LISTEN(tier::general, input::events::device::user::logout, props)
            {
                if (props < user_numbering.size()) user_numbering[props] = faux;
                else
                {
                    if constexpr (debugmode) log(prompt::host, ansi::err("User accounting error: ring size:", user_numbering.size(), " user_number:", props));
                }
            };
            LISTEN(tier::request, vtm::events::apptype, what)
            {
                auto& setup = menu_list[what.menuid];
                what.type = setup.type;
            };
            LISTEN(tier::request, vtm::events::newapp, what)
            {
                auto& setup = menu_list[what.menuid];
                auto& maker = app::shared::builder(setup.type);
                what.applet = maker(setup.appcfg, config);
                what.applet->base::property("window.menuid") = what.menuid;
                what.applet->base::bind_property<tier::preview>("window.header", *what.applet, e2::form::prop::ui::header) = setup.title;
                what.applet->base::bind_property<tier::preview>("window.footer", *what.applet, e2::form::prop::ui::footer) = setup.footer;
                app::shared::applet_kb_navigation(config, what.applet);
            };
            LISTEN(tier::general, e2::conio::logs, utf8) // Forward logs from brokers.
            {
                log<faux>(utf8);
            };
            LISTEN(tier::request, desk::events::usrs, usrs_ptr)
            {
                usrs_ptr = usrs_list_ptr;
            };
            LISTEN(tier::request, desk::events::apps, apps_ptr)
            {
                apps_ptr = apps_list_ptr;
            };
            LISTEN(tier::request, desk::events::menu, menu_ptr)
            {
                menu_ptr = menu_list_ptr;
            };
            //todo unify
            LISTEN(tier::request, e2::form::layout::go::next, next)
            {
                if (base::subset.size())
                {
                    base::subset.push_back(base::subset.front());
                    base::subset.back()->base::holder = std::prev(base::subset.end());
                    base::subset.pop_front();
                    if (auto next_ptr = base::subset.back())
                    {
                        next = next_ptr;
                    }
                }
            };
            LISTEN(tier::request, e2::form::layout::go::prev, prev)
            {
                if (base::subset.size())
                {
                    base::subset.push_front(base::subset.back());
                    base::subset.front()->base::holder = base::subset.begin();
                    base::subset.pop_back();
                    if (auto prev_ptr = base::subset.back())
                    {
                        prev = prev_ptr;
                    }
                }
            };
            LISTEN(tier::request, e2::form::layout::go::item, current_item)
            {
                if (base::subset.size()) current_item = base::subset.back();
            };
            LISTEN(tier::preview, e2::form::prop::cwd, path_utf8)
            {
                for (auto w : base::subset)
                {
                    w->base::signal(tier::anycast, e2::form::prop::cwd, path_utf8);
                }
            };

            auto& hall_focus = base::field<id_t>(); // Last active gear id.
            auto& offset = base::field<twod>(); // Last created window coords.
            LISTEN(tier::request, desk::events::exec, appspec)
            {
                auto gear_ptr = netxs::sptr<hids>{};
                if (appspec.gear_id == id_t{})
                {
                    //todo revise
                    if (hall_focus) // Take the last active keyboard.
                    {
                        gear_ptr = bell::getref<hids>(hall_focus);
                        //if (gear_ptr)
                        //{
                        //    appspec.gear_id = hall_focus;
                        //}
                    }
                    if (!gear_ptr && users.size()) // Take any existing.
                    {
                        auto gate_ptr = bell::getref<gate>(users.back().first->id);
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

                apps_list[menu_id];
                auto& appbase = menu_list[menu_id];
                auto fixed = appbase.fixed && !appspec.fixed;
                if (fixed) std::swap(appbase, appspec); // Don't modify the base menuitem by the temp appspec.
                else       appbase = appspec;

                auto what = applink{ .menuid = menu_id, .forced = true };
                auto yield = text{};
                if (gear_ptr)
                {
                    auto& gear = *gear_ptr;
                    auto viewport = gear.owner.base::signal(tier::request, e2::form::prop::viewport);
                    if (wincoor == dot_00)
                    {
                        offset = (offset + dot_21 * 2) % std::max(dot_11, viewport.size * 7 / 32);
                        wincoor = viewport.coor + offset + viewport.size * 1 / 32;
                    }
                    what.square.coor = wincoor;
                    what.square.size = winsize ? winsize : viewport.size * 3 / 4;
                    if (auto window = create_window(what))
                    {
                        //todo revise: Should the requester set focus on their own behalf?
                        pro::focus::set(window, gear_id/*requested focus*/, solo::on); // Notify pro::focus owners.
                        window->base::signal(tier::anycast, e2::form::upon::created, gear); // Tile should change the menu item.
                             if (appbase.winform == winstate::maximized)  window->base::signal(tier::preview, e2::form::size::enlarge::maximize, gear);
                        else if (appbase.winform == winstate::fullscreen) window->base::signal(tier::preview, e2::form::size::enlarge::fullscreen, gear);
                        else if (appbase.winform == winstate::minimized)  window->base::signal(tier::preview, e2::form::size::minimize, gear);
                        yield = utf::concat(window->id);
                    }
                }
                else
                {
                    if (winsize == dot_00) winsize = { 80, 27 };
                    what.square.coor = wincoor;
                    what.square.size = winsize;
                    if (auto window = create_window(what))
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
                what.menuid = gate.base::property("desktop.selected");
                if (auto window = create_window(what))
                {
                    //window->LISTEN(tier::release, e2::form::upon::vtree::detached, master)
                    //{
                    //    log(prompt::hall, "Objects count: ", base::subset.size());
                    //};
                    pro::focus::set(window, gear.id, solo::on);
                    window->base::signal(tier::anycast, e2::form::upon::created, gear); // Tile should change the menu item.
                    auto& cfg = menu_list[what.menuid];
                         if (cfg.winform == winstate::maximized)  window->base::signal(tier::preview, e2::form::size::enlarge::maximize, gear);
                    else if (cfg.winform == winstate::fullscreen) window->base::signal(tier::preview, e2::form::size::enlarge::fullscreen, gear);
                    else if (cfg.winform == winstate::minimized)  window->base::signal(tier::preview, e2::form::size::minimize, gear);
                }
            };
            LISTEN(tier::request, vtm::events::handoff, what)
            {
                create_window(what, true);
            };

            LISTEN(tier::preview, input::events::keybd::key::post, gear) // Track last active gear.
            {
                hall_focus = gear.id;
            };

            auto& switch_counter = base::field<std::unordered_map<id_t, si32>>(); // hall: Focus switch counter.
            LISTEN(tier::release, input::events::focus::set::any, seed) // Reset the focus switch counter when it is focused from outside.
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
                if (std::abs(counter) >= (si32)base::subset.size())
                {
                    counter = {};
                    gear_id = {};
                }
            };
            LISTEN(tier::general, e2::timer::any, timestamp)
            {
                if (base::ruined()) // Force all gates to redraw.
                {
                    for (auto usergate_ptr : usrs_list)
                    {
                        usergate_ptr->base::ruined(true);
                    }
                    base::ruined(faux);
                }
            };
            LISTEN(tier::release, e2::render::background::prerender, parent_canvas) // Sync hall basis with current gate.
            {
                auto gate_ptr = bell::getref<ui::gate>(parent_canvas.link());
                parent_canvas.move_basis(gate_ptr->region.coor);
            };
            auto& layers = base::field<std::array<std::vector<sptr>, 3>>();
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                if (users.size() > 1) // Draw users.
                {
                    static auto color = tone{ tone::brighter, tone::shadower };
                    for (auto& [user_ptr, uname] : users)
                    {
                        fasten(user_ptr, faux, 0, color, parent_canvas); // Draw strings.
                        if (user_ptr->id != parent_canvas.link()) // Draw a shadow of user's gate for other users.
                        {
                            auto gate_area = user_ptr->area();
                            if (parent_canvas.cmode != svga::vt16 && parent_canvas.cmode != svga::nt16) // Don't show shadow in poor color environment.
                            {
                                auto mark = skin::color(tone::shadower);
                                mark.bga(mark.bga() / 2);
                                parent_canvas.fill(gate_area, [&](cell& c){ c.blend(mark); });
                            }
                            gate_area.coor -= dot_01 + parent_canvas.coor();
                            parent_canvas.output(uname, gate_area.coor, cell::shaders::contrast);
                        }
                    }
                }
                if (base::subset.size()) // Draw windows.
                {
                    for (auto& item_ptr : base::subset)
                    {
                        if (auto window_ptr = std::dynamic_pointer_cast<window_t>(item_ptr))
                        {
                            fasten(window_ptr, window_ptr->highlighted, window_ptr->active, window_ptr->color, parent_canvas);
                            layers[std::clamp(window_ptr->z_order, zpos::plain, zpos::topmost)].push_back(item_ptr);
                        }
                    }
                    for (auto l : { zpos::backmost, zpos::plain, zpos::topmost })
                    {
                        auto& layer = layers[l];
                        for (auto& item_ptr : layer)
                        {
                            item_ptr->render<true>(parent_canvas);
                        }
                        layer.clear();
                    }
                }
                for (auto& [user_ptr, uname] : users) // Draw user mouse pointers.
                {
                    if (user_ptr->id != parent_canvas.link())
                    {
                        auto& usergate = *(std::static_pointer_cast<gate>(user_ptr));
                        if (uname.lyric) // Render foreign user names at their place.
                        {
                            auto& user_name = *uname.lyric;
                            auto  half_x = user_name.size().x / 2;
                            for (auto& [ext_gear_id, gear_ptr] : usergate.gears)
                            {
                                auto& gear = *gear_ptr;
                                if (gear.mouse_disabled) continue;
                                auto coor = twod{ gear.coord } + gear.owner.coor();
                                coor.y -= 1;
                                coor.x -= half_x;
                                user_name.move(coor);
                                parent_canvas.fill(user_name, cell::shaders::contrast);
                                usergate.fill_pointer(gear, parent_canvas);
                            }
                        }
                    }
                }
            };
            base::signal(tier::general, e2::config::fps, ui::skin::globals().maxfps);
        }

        // hall: Autorun apps from config.
        void autorun()
        {
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
                        auto window_ptr = create_window(what);
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
                auto gear_test = base::signal(tier::request, e2::form::state::keybd::next);
                if (gear_test.second) log(prompt::hall, "Autofocused items count: ", gear_test.second);
            }
        }
        // hall: .
        template<class P>
        void run(P process)
        {
            async.run(process);
        }
        // hall: Create a new user gate.
        auto invite(xipc client, view userid, si32 vtmode, eccc usrcfg, xmls app_config, si32 session_id)
        {
            auto lock = bell::unique_lock();
            auto usergate_ptr = ui::gate::ctor(client, vtmode, app_config, userid, session_id, true);
            auto& usergate = *usergate_ptr;

            auto& [user_ptr, uname] = users.emplace_back(usergate_ptr, para{});
            auto users_iter = std::prev(users.end());
            usrs_list.push_back(usergate_ptr);
            auto usrs_list_iter = std::prev(usrs_list.end());
            usergate.props.background_color.link(bell::id);
            base::signal(tier::release, desk::events::usrs, usrs_list_ptr);

            auto& memo = base::field<subs>();
            usergate.LISTEN(tier::release, e2::form::size::restore, p)
            {
                if (memo.empty()) return;
                memo.clear();
                usergate.base::riseup(tier::preview, e2::form::prop::ui::header, std::move(usergate.base::property("window.saved_header")));
                usergate.base::riseup(tier::preview, e2::form::prop::ui::footer, std::move(usergate.base::property("window.saved_footer")));
                auto applet_ptr = usergate.base::subset.back();
                auto gear_id_list = pro::focus::cut(applet_ptr);
                applet_ptr->base::detach();
                if (auto world_ptr = base::signal(tier::general, e2::config::creator))
                {
                    world_ptr->base::signal(tier::request, vtm::events::handoff, { .applet = applet_ptr });
                }
                pro::focus::set(applet_ptr, gear_id_list, solo::on, true);
            };
            usergate.LISTEN(tier::release, vtm::events::gate::fullscreen, new_fullscreen)
            {
                if (usergate.base::subset.size() > 1)
                {
                    usergate.base::signal(tier::release, e2::form::size::restore);
                }
                if (new_fullscreen.applet && !new_fullscreen.applet->base::subset.empty())
                {
                    auto gear_id_list = pro::focus::cut(new_fullscreen.applet);
                    auto window_ptr = std::exchange(new_fullscreen.applet, new_fullscreen.applet->base::subset.front()); // Drop hosting window.
                    window_ptr->base::detach();
                    auto applet_ptr = new_fullscreen.applet;
                    auto& applet = *applet_ptr;
                    applet.base::detach();
                    auto new_pos = usergate.base::area();
                    new_pos.coor -= usergate.base::coor();
                    applet.base::extend(new_pos);

                    auto newhead = applet.base::property("window.header");
                    auto newfoot = applet.base::property("window.footer");
                    usergate.base::property("window.saved_header") = usergate.base::riseup(tier::request, e2::form::prop::ui::header);
                    usergate.base::property("window.saved_footer") = usergate.base::riseup(tier::request, e2::form::prop::ui::footer);
                    usergate.base::riseup(tier::preview, e2::form::prop::ui::header, newhead);
                    usergate.base::riseup(tier::preview, e2::form::prop::ui::footer, newfoot);

                    usergate.LISTEN(tier::anycast, e2::form::proceed::quit::one, fast, memo)
                    {
                        usergate.base::signal(tier::release, e2::form::size::restore);
                    };
                    applet.LISTEN(tier::preview, e2::form::size::enlarge::any, gear, memo)
                    {
                        auto deed = applet.bell::protos(tier::preview);
                        if (deed == e2::form::size::enlarge::maximize.id)
                        {
                            usergate.base::signal(tier::release, e2::form::size::restore);
                        }
                    };
                    applet.LISTEN(tier::preview, e2::form::size::minimize, gear, memo)
                    {
                        applet.bell::expire(tier::release); // Suppress hide/minimization.
                        usergate.base::signal(tier::release, e2::form::size::restore);
                    };
                    applet.LISTEN(tier::release, e2::form::proceed::quit::one, fast, memo)
                    {
                        usergate.base::signal(tier::release, e2::form::size::restore);
                        applet.bell::expire(tier::release, true); // Continue event riseup().
                    };
                    usergate.attach(applet_ptr);
                    pro::focus::set(applet_ptr, gear_id_list, solo::on, true); // Refocus.
                }
            };
            usergate.LISTEN(tier::release, e2::conio::focus::post, seed, -, (focus_tree_map = std::unordered_map<ui64, ui64>{})) // Filter recursive focus loops. Run prior the ui::gate's e2::conio::focus::any.
            {
                if (seed.treeid)
                {
                    auto& digest = focus_tree_map[seed.treeid];
                    if (digest < seed.digest) // This is the first time this focus event has been received.
                    {
                        digest = seed.digest;
                    }
                    else // We've seen this event before.
                    {
                        usergate.bell::expire(tier::release); // Stop event forwarding.
                    }
                }
            };
            usergate.LISTEN(tier::release, e2::form::prop::name, user_name_utf8)
            {
                uname = user_name_utf8;
            };
            usergate.LISTEN(tier::request, e2::form::prop::name, user_name_utf8)
            {
                user_name_utf8 = uname.lyric->utf8();
            };
            usergate.LISTEN(tier::release, e2::form::layout::shift, newpos)
            {
                auto viewport = usergate.base::signal(tier::request, e2::form::prop::viewport);
                auto oldpos = viewport.center();
                auto path = oldpos - newpos;
                auto time = datetime::round<si32>(skin::globals().switching);
                auto init = 0;
                auto func = constlinearAtoB<twod>(path, time, init);
                robot.pacify(usergate.id);
                robot.actify(usergate.id, func, [&](auto& x)
                {
                    usergate.base::moveby(-x);
                    this->base::deface();
                });
            };
            usergate.LISTEN(tier::release, e2::form::layout::jumpto, window_inst)
            {
                auto viewport = usergate.base::signal(tier::request, e2::form::prop::viewport);
                auto object_area = window_inst.base::signal(tier::request, e2::form::prop::window::fullsize);
                auto outside = viewport | object_area;
                if (outside != viewport)
                {
                    auto coor = outside.coor.equals(object_area.coor, object_area.coor, outside.coor + outside.size - viewport.size);
                    auto center = viewport.center() + coor - viewport.coor;
                    usergate.base::signal(tier::release, e2::form::layout::shift, center);
                }
            };
            usergate.LISTEN(tier::release, input::events::mouse::button::click::left, gear) // Fly to another user's viewport.
            {
                if (gear.owner.id == usergate.id) return;
                auto center = usergate.base::coor() + gear.owner.base::size() / 2;
                gear.owner.base::signal(tier::release, e2::form::layout::shift, center);
            };
            usergate.LISTEN(tier::release, e2::conio::mouse, m) // Trigger to redraw all gates on mouse activity (to redraw foreign mouse cursor).
            {
                this->base::deface();
            };
            usergate.LISTEN(tier::release, e2::conio::winsz, w) // Trigger to redraw all gates.
            {
                this->base::deface();
            };
            auto& drag_origin = usergate.base::field<fp2d>();
            auto& user_mouse = usergate.base::plugin<pro::mouse>();
            user_mouse.template draggable<hids::buttons::leftright>(true);
            user_mouse.template draggable<hids::buttons::left>(true);
            usergate.LISTEN(tier::release, e2::form::drag::start::any, gear)
            {
                if (gear.owner.id == usergate.id)
                {
                    robot.pacify(usergate.id);
                    drag_origin = gear.coord;
                }
            };
            usergate.LISTEN(tier::release, e2::form::drag::pull::any, gear)
            {
                if (gear.owner.id != usergate.id) return;
                if (auto delta = twod{ gear.coord } - twod{ drag_origin })
                {
                    drag_origin = gear.coord;
                    usergate.base::moveby(-delta);
                    this->base::deface();
                }
            };
            usergate.LISTEN(tier::release, e2::form::drag::stop::any, gear)
            {
                if (gear.owner.id != usergate.id) return;
                robot.pacify(usergate.id);
                robot.actify(usergate.id, gear.fader<quadratic<twod>>(2s), [&](auto delta)
                {
                    usergate.base::moveby(-delta);
                    this->base::deface();
                });
            };

            auto& vport = base::property<twod>("desktop.viewport"); // hall: Last user's viewport position.
            auto& selected_item = base::property<text>("desktop.selected"); // hall: Last user's selected menu item.
            auto& usergate_selected_item = usergate.base::property<text>("desktop.selected");
            if (!vport) vport = config.take(path::viewport, dot_00);
            if (selected_item.empty()) selected_item = config.take(path::selected, selected_item);
            usergate_selected_item = selected_item;
            //auto& usergate_id = usergate.base::property<id_t>("gate.id");
            //auto& usergate_os_id = usergate.base::property<text>("gate.os_id");
            usrcfg.cfg = utf::concat(usergate.id, ";", usergate.props.os_user_id);
            auto deskmenu_ptr = app::shared::builder(app::desk::id)(usrcfg, app_config);
            deskmenu_ptr->base::plugin<pro::keybd>("taskbar");
            app::shared::applet_kb_navigation(config, deskmenu_ptr);
            usergate.attach(std::move(deskmenu_ptr));
            usergate.base::extend({ vport, usrcfg.win }); // Restore user's last position.
            pro::focus::set(This(), id_t{}, solo::off);
            lock.unlock();
            usergate.launch();
            base::deface();
            vport = usergate.base::coor();
            selected_item = usergate_selected_item;
            usrs_list.erase(usrs_list_iter);
            users.erase(users_iter);
        }
        // hall: Shutdown.
        void stop()
        {
            log(prompt::hall, "Server shutdown");
            base::signal(tier::general, e2::conio::quit); // Trigger to disconnect all users and monitors.
            async.stop(); // Wait until all users and monitors are disconnected.
            if constexpr (debugmode) log(prompt::hall, "Session control stopped");
            bell::dequeue(); // Wait until all cleanups are completed.
            auto lock = bell::sync();
            base::plugin<pro::mouse>().reset(); // Release the captured mouse.
        }
    };
}