// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

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
        static constexpr auto taskbar  = "/config/desktop/taskbar/";
        static constexpr auto item     = "item";
        static constexpr auto autorun  = "/config/desktop/taskbar/autorun/";
        static constexpr auto selected = "/config/desktop/taskbar/selected";
        static constexpr auto viewport = "/config/desktop/viewport/coor";
    }

    namespace events
    {
        EVENTPACK( vtm::events, ui::e2::extra::slot1 )
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
            fp2d  drag_origin;

        public:
            frame(base&&) = delete;
            frame(base& boss) : skill{ boss },
                robo{ boss }
            {
                boss.on(tier::mousepreview, input::key::LeftClick, memo, [&](hids& /*gear*/)
                {
                    //todo window.events(onclick)
                    boss.base::riseup(tier::preview, e2::form::layout::expose);
                });
                boss.on(tier::mousepreview, input::key::RightClick, memo, [&](hids& /*gear*/)
                {
                    //todo window.events(onclick)
                    boss.base::riseup(tier::preview, e2::form::layout::expose);
                });
                boss.on(tier::mousepreview, input::key::MouseDown, memo, [&](hids& /*gear*/)
                {
                    robo.pacify();
                });
                boss.on(tier::mouserelease, input::key::RightClick, memo, [&](hids& gear)
                {
                    auto& area = boss.base::area();
                    auto coord = gear.coord + area.coor;
                    if (!area.hittest(coord))
                    {
                        pro::focus::set(boss.This(), gear.id, solo::on);
                        appear(coord);
                    }
                    gear.dismiss();
                });
                boss.LISTEN(tier::preview, e2::form::layout::appear, newpos, memo)
                {
                    appear(newpos);
                };
                boss.LISTEN(tier::preview, e2::form::upon::changed, delta, memo)
                {
                    boss.base::riseup(tier::preview, e2::form::layout::bubble);
                };
                boss.LISTEN(tier::release, e2::form::drag::start::any, gear, memo)
                {
                    drag_origin = gear.coord;
                };
                boss.LISTEN(tier::release, e2::form::drag::pull::any, gear, memo)
                {
                    if (gear)
                    {
                        auto deed = boss.bell::protos();
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
                boss.LISTEN(tier::preview, input::events::keybd::post, gear, memo)
                {
                    if (gear.captured(boss.bell::id)) check_modifiers(gear);
                };

                boss.on(tier::mouserelease, input::key::MiddleDragStart, memo, [&](hids& gear)
                {
                    handle_init(gear);
                });
                boss.on(tier::mouserelease, input::key::RightDragStart, memo.back());
                boss.on(tier::mouserelease, input::key::MiddleDragPull, memo, [&](hids& gear)
                {
                    handle_pull(gear);
                });
                boss.on(tier::mouserelease, input::key::RightDragPull, memo.back());
                boss.on(tier::mouserelease, input::key::MiddleDragStop, memo, [&](hids& gear)
                {
                    handle_stop(gear);
                });
                boss.on(tier::mouserelease, input::key::RightDragStop, memo.back());
                boss.on(tier::mouserelease, input::key::MiddleDragCancel, memo, [&](hids& gear)
                {
                    handle_drop(gear);
                });
                boss.on(tier::mouserelease, input::key::RightDragCancel, memo.back());
                boss.dup_handler(tier::general, input::events::halt.id, memo.back());

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
                boss.LISTEN(tier::release, e2::form::drag::start::any, gear, memo)
                {
                    if (!drags && boss.size().inside(gear.coord) && !gear.meta(hids::anyMod))
                    {
                        drags = true;
                        coord = gear.coord;
                        under = {};
                    }
                };
                boss.LISTEN(tier::release, e2::form::drag::pull::any, gear, memo)
                {
                    if (!drags) return;
                    if (gear.meta(hids::anyMod)) proceed(faux, gear);
                    else                         coord = gear.coord - gear.delta.get();
                };
                boss.LISTEN(tier::release, e2::form::drag::stop::any, gear, memo)
                {
                    if (!drags) return;
                    if (gear.meta(hids::anyMod)) proceed(faux, gear);
                    else                         proceed(true, gear);
                };
                boss.LISTEN(tier::release, e2::form::drag::cancel::any, gear, memo)
                {
                    if (!drags) return;
                    if (gear.meta(hids::anyMod)) proceed(faux, gear);
                    else                         proceed(true, gear);
                };
                boss.dup_handler(tier::general, input::events::halt.id, memo.back());
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
                            if (auto old_object = boss.base::getref(under))
                            {
                                old_object->base::riseup(tier::release, vtm::events::d_n_d::abort, object);
                            }
                            if (auto new_object = boss.base::getref(new_under))
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
            si32& zorder;
            bool highlighted = faux;
            bool active = faux;
            tone color = { tone::brighter, tone::shadower };

            void window_swarp(dent warp)
            {
                base::enqueue([warp](auto& boss) // Keep the focus tree intact while processing events.
                {
                    boss.base::signal(tier::preview, e2::form::layout::swarp, warp);
                });
            }
            auto window_zorder(arch args_count, si32 state)
            {
                if (args_count != 0)
                {
                    zorder = state;
                    base::strike();
                }
                return zorder;
            }
            void window_close(id_t gear_id)
            {
                base::enqueue([gear_id](auto& boss) // Keep the focus tree intact while processing events.
                {
                    if (auto gear_ptr = boss.base::template getref<hids>(gear_id)) //todo Apple clang requires template
                    {
                        auto& gear = *gear_ptr;
                        gear.set_multihome();
                    }
                    boss.base::signal(tier::anycast, e2::form::proceed::quit::one, true);
                });
            }
            void window_state(id_t gear_id, auto state)
            {
                base::enqueue([state, gear_id](auto& boss) // Keep the focus tree intact while processing events.
                {
                    if (auto gear_ptr = boss.base::template getref<hids>(gear_id))
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
            static constexpr auto classname = basename::window;
            window_t(hall& owner, applink& what)
                : world{ owner },
                  zorder{ what.applet->base::property("applet.zorder", zpos::plain) }
            {
                base::plugin<pro::mouse>();
                base::plugin<pro::d_n_d>();
                base::plugin<pro::ghost>();
                auto& title = base::plugin<pro::title>(what.applet->base::property("window.header"), what.applet->base::property("window.footer"));
                base::plugin<pro::sizer>();
                base::plugin<pro::frame>();
                base::plugin<pro::light>();
                base::plugin<pro::focus>();
                base::plugin<pro::keybd>();
                base::limits(dot_11);
                base::kind(base::reflow_root);
                base::root(true);
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
                    else if (gui_cmd.cmd_id == syscmd::zorder)
                    {
                        auto args_count = gui_cmd.args.size();
                        window_zorder(args_count, args_count ? any_get_or(gui_cmd.args[0], zpos::plain) : zpos::plain);
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
                    if (!hit) bell::passover();
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
                on(tier::mouserelease, input::key::LeftDoubleClick, [&](hids& gear)
                {
                    base::riseup(tier::preview, e2::form::size::enlarge::maximize, gear);
                    gear.dismiss();
                });
                on(tier::mouserelease, input::key::LeftClick, [&](hids& gear)
                {
                    auto home = rect{ -dot_21, base::size() + dot_21 * 2 }; // Including resizer grips.
                    if (!home.hittest(gear.coord))
                    {
                        gear.owner.base::signal(tier::release, e2::form::layout::jumpto, *this);
                    }
                });
                //todo use input::key::MouseClick
                on(tier::mouserelease, input::key::RightClick, [&](hids& gear)
                {
                    pro::focus::set(This(), gear.id, solo::on);
                });
                on(tier::mouserelease, input::key::MiddleClick, [&](hids& gear)
                {
                    pro::focus::set(This(), gear.id, solo::on);
                });
                LISTEN(tier::release, e2::form::proceed::quit::any, fast)
                {
                    base::detach(); // The object kills itself.
                };
                auto& fast_quit = base::field(faux);
                LISTEN(tier::general, e2::conio::quit, deal) // Desktop shutdown.
                {
                    fast_quit = true;
                    base::signal(tier::anycast, e2::form::proceed::quit::one, true); // Schedule a cleanup.
                };
                LISTEN(tier::release, e2::form::upon::vtree::detached, parent_ptr)
                {
                    if (!fast_quit && parent_ptr)
                    {
                        parent_ptr->base::enqueue([&](auto& boss)
                        {
                            boss.base::cleanup();
                        });
                    }
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
                    if (zorder == zpos::backmost) // It is a region view. Just resize it.
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
                            bell::passover();
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
                        world.base::subset.push_back(This());
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
                    if (next != world.base::subset.end())
                    {
                        if (++next != world.base::subset.end() && !area.trim((*next)->region))
                        {
                            world.base::subset.erase(base::holder);
                            while (++next != world.base::subset.end() && !area.trim((*next)->region))
                            { }
                            base::holder = world.base::subset.insert(next, This());
                            base::strike();
                        }
                    }
                };
                LISTEN(tier::release, e2::form::state::mouse, hovered)
                {
                    active = hovered;
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
                    if (auto context2D = form::nested_2D_context(parent_canvas))
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
        pro::maker& maker; // hall: Window creator using drag and drop (right drag).
        pro::robot& robot; // hall: Animation controller.

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
            window.LISTEN(tier::release, e2::form::upon::vtree::detached, world_ptr, -, (menuid))
            {
                if (base::subset.size()) // Pass focus to the top most object.
                {
                    auto last_ptr = base::subset.back();
                    auto gear_id_list = window.base::riseup(tier::request, e2::form::state::keybd::enlist);
                    for (auto gear_id : gear_id_list)
                    {
                        if (auto gear_ptr = base::getref<hids>(gear_id))
                        {
                            auto gear_test = base::signal(tier::request, e2::form::state::keybd::next, { gear_id, 0 });
                            if (gear_test.second == 1) // If it is the last focused item.
                            {
                                pro::focus::set(last_ptr, gear_id, solo::off);
                            }
                        }
                    }
                }
                auto& [fixed_menu_item, inst_list] = apps_list[menuid];
                inst_list.erase(inst_list_iter);
                if (!fixed_menu_item && inst_list.empty()) // Remove non-fixed menu group if it is empty.
                {
                    apps_list.erase(menuid);
                }
                base::signal(tier::release, desk::events::apps, apps_list_ptr); // Update taskbar app list.
            };
            auto root_ptr = is_handoff ? sptr{} : what.applet;
            window_ptr->base::broadcast(tier::anycast, e2::form::upon::started, root_ptr);
            base::signal(tier::release, desk::events::apps, apps_list_ptr);
            window_ptr->base::reflow();
            return window_ptr;
        }
        auto loadspec(auto& conf_rec, auto& fallback, auto& item_ptr, text menuid, bool splitter = {})
        {
            auto& config = bell::indexer.config;
            conf_rec.splitter   = splitter;
            conf_rec.menuid     = menuid;
            conf_rec.label      = config.settings::take_value_from(item_ptr, attr::label,    fallback.label   );
            if (conf_rec.label.empty()) conf_rec.label = conf_rec.menuid;
            conf_rec.hidden     = config.settings::take_value_from(item_ptr, attr::hidden,   fallback.hidden  );
            conf_rec.tooltip    = config.settings::take_value_from(item_ptr, attr::tooltip,  fallback.tooltip );
            conf_rec.title      = config.settings::take_value_from(item_ptr, attr::title,    fallback.title   );
            conf_rec.footer     = config.settings::take_value_from(item_ptr, attr::footer,   fallback.footer  );
            conf_rec.winsize    = config.settings::take_value_from(item_ptr, attr::winsize,  fallback.winsize );
            conf_rec.wincoor    = config.settings::take_value_from(item_ptr, attr::wincoor,  fallback.wincoor );
            conf_rec.winform    = config.settings::take_value_from(item_ptr, attr::winform,  fallback.winform, shared::win::options);
            conf_rec.appcfg.cwd = config.settings::take_value_from(item_ptr, attr::cwd,      fallback.appcfg.cwd);
            conf_rec.appcfg.cfg = config.settings::take_value_from(item_ptr, attr::cfg,      ""s);
            conf_rec.appcfg.cmd = config.settings::take_value_from(item_ptr, attr::cmd,      fallback.appcfg.cmd);
            conf_rec.type       = config.settings::take_value_from(item_ptr, attr::type,     fallback.type    );
            utf::to_lower(conf_rec.type);
            auto envar          = config.settings::take_value_list_of(item_ptr, attr::env);
            if (envar.empty()) conf_rec.appcfg.env = fallback.appcfg.env;
            else for (auto& value : envar)
            {
                if (value.size())
                {
                    conf_rec.appcfg.env += value + '\0';
                }
            }
            if (conf_rec.title.empty()) conf_rec.title = conf_rec.menuid + (conf_rec.appcfg.cmd.empty() ? ""s : ": " + conf_rec.appcfg.cmd);
            if (conf_rec.appcfg.cfg.empty())
            {
                auto patch = config.settings::take_ptr_list_of(item_ptr, attr::config);
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
                        auto fragment = settings{ fallback.appcfg.cfg.size() ? fallback.appcfg.cfg
                                                                             : (*head++)->snapshot() };
                        while (head != tail)
                        {
                            auto& p = *head++;
                            fragment.settings::fuse(p->snapshot());
                        }
                        conf_rec.appcfg.cfg = fragment.settings::utf8();
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
                window.base::enqueue([&, gear_id = gear.id](auto& /*boss*/) // Keep the focus tree intact while processing events.
                {
                    pro::focus::set(window.This(), gear_id, solo::on);
                });
            }
            return true;
        }

    public:
        static constexpr auto classname = basename::desktop;
        hall(xipc server)
            : maker{ base::plugin<pro::maker>() },
              robot{ base::plugin<pro::robot>() }
        {
            auto& canal = *server;

            auto& config = bell::indexer.config;
            app::shared::get_tui_config(config, ui::skin::globals());

            base::plugin<pro::focus>(pro::focus::mode::focusable);
            base::plugin<pro::keybd>();
            auto& luafx = bell::indexer.luafx;
            auto desktop_context = config.settings::push_context("/config/events/desktop/");
            auto script_list = config.settings::take_ptr_list_for_name("script");
            auto bindings = input::bindings::load(config, script_list);
            input::bindings::keybind(*this, bindings);
            base::add_methods(basename::desktop,
            {
                { "Cleanup",            [&]
                                        {
                                            auto show_details = luafx.get_args_or(1, faux);
                                            base::cleanup(show_details);
                                            luafx.set_return();
                                        }},
                { "EventList",          [&]
                                            {
                                                log("Registered events:");
                                                auto maxlen = 0;
                                                auto events = std::vector<std::pair<view, view>>{};
                                                events.reserve(netxs::events::rtti().size());
                                                for (auto& [event_name, metadata] : netxs::events::rtti())
                                                {
                                                    events.push_back({ event_name, metadata.param_typename });
                                                    if ((si32)event_name.size() > maxlen) maxlen = (si32)event_name.size();
                                                }
                                                std::sort(events.begin(), events.end(), [](auto a, auto b){ return a.first < b.first; });
                                                auto mpad = text(maxlen, ' ');
                                                auto digits = std::to_string(netxs::events::rtti().size()).size();
                                                auto i = 0;
                                                for (auto& [event_name, type] : events)
                                                {
                                                    auto pad = view{ mpad.data(), maxlen - event_name.size() };
                                                    auto n = utf::adjust(std::to_string(++i), digits, ' ', true);
                                                    log(" %n% %event% %pad% type: %%", n, ansi::clr(tint::greenlt, event_name), pad, ansi::clr(tint::yellowlt, type));
                                                }
                                                luafx.set_return();
                                            }},
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
                                            auto& gear = luafx.get_gear();
                                            auto ok = gear.is_real();
                                            if (ok)
                                            {
                                                gear.owner.base::signal(tier::preview, e2::conio::quit);
                                                gear.set_handled();
                                            }
                                            luafx.set_return(ok);
                                        }},
                { "Run",                [&]
                                        {
                                            auto args_count = luafx.args_count();
                                            auto& gear = luafx.get_gear();
                                            auto gear_id = gear.is_real() ? gear.id : id_t{};
                                            auto appspec = desk::spec{ .hidden  = true,
                                                                       .winform = winstate::normal,
                                                                       .type    = app::vtty::id,
                                                                       .gear_id = gear_id };
                                            if (!args_count) // Get default app spec.
                                            {
                                                if (gear_id)
                                                {
                                                    auto& current_default = gear.owner.base::property("desktop.selected");
                                                    appspec = menu_list[current_default];
                                                    appspec.fixed = faux;
                                                    appspec.menuid = current_default;
                                                    appspec.gear_id = gear_id;
                                                }
                                            }
                                            else
                                            {
                                                auto utf8_xml = ansi::escx{};
                                                luafx.read_args(1, [&](qiew key, qiew val)
                                                {
                                                    //log("  %%=%%", key, utf::debase437(val));
                                                    utf8_xml += "<";
                                                    //todo just use utf::unordered_map for loadspec
                                                    utf::filter_alphanumeric(key, utf8_xml);
                                                    utf8_xml += "=\"";
                                                    utf::escape(val, utf8_xml, '"');
                                                    utf8_xml += "\"/>";
                                                });
                                                log("%%Run %%", prompt::host, ansi::hi(utf::debase437(utf8_xml)));
                                                auto appconf = settings{ utf8_xml };
                                                auto item_ptr = appconf.document.root_ptr;
                                                auto menuid = config.settings::take_value_from(item_ptr, attr::id, ""s);
                                                auto taskbar_context = config.settings::push_context(path::taskbar);
                                                if (menu_list.contains(menuid))
                                                {
                                                    auto& appbase = menu_list[menuid];
                                                    if (appbase.fixed) hall::loadspec(appspec, appbase, item_ptr, menuid);
                                                    else               hall::loadspec(appspec, appspec, item_ptr, menuid);
                                                }
                                                else
                                                {
                                                    if (menuid.empty()) menuid = "vtm.run(" + utf8_xml + ")";
                                                    hall::loadspec(appspec, appspec, item_ptr, menuid);
                                                }
                                            }
                                            auto title = appspec.title.empty() && appspec.label.empty() ? appspec.menuid
                                                       : appspec.title.empty() ? appspec.label
                                                       : appspec.label.empty() ? appspec.title : ""s;
                                            if (appspec.title.empty()) appspec.title = title;
                                            if (appspec.label.empty()) appspec.label = title;
                                            if (appspec.tooltip.empty()) appspec.tooltip = appspec.menuid;
                                            base::signal(tier::request, desk::events::exec, appspec);
                                            if (gear_id) gear.set_handled();
                                            luafx.set_return();
                                        }},
                { "FocusNextWindow",    [&]
                                        {
                                            auto& gear = luafx.get_gear();
                                            auto dir = luafx.get_args_or(1, 1);
                                            if (focus_next_window(gear, dir))
                                            {
                                                gear.set_handled();
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
            auto taskbar_context = config.settings::push_context(path::taskbar);
            auto item_ptr_list = config.settings::take_ptr_list_for_name(path::item);
            for (auto item_ptr : item_ptr_list)
            {
                auto item_context = config.settings::push_context(item_ptr);
                auto is_splitter = !config.settings::take_value_list_of(item_ptr, attr::splitter).empty();
                auto menuid = is_splitter ? "splitter_" + std::to_string(splitter_count++)
                                          : config.settings::take_value_from(item_ptr, attr::id, ""s);
                if (menuid.empty())
                {
                    menuid = "App" + std::to_string(auto_id++);
                }
                auto& proto = find(menuid);
                if (!proto.notfound) // Update existing record.
                {
                    auto& conf_rec = proto;
                    conf_rec.fixed = true;
                    hall::loadspec(conf_rec, conf_rec, item_ptr, menuid, is_splitter);
                    expand(conf_rec);
                }
                else // New item.
                {
                    auto conf_rec = desk::spec{};
                    conf_rec.fixed = true;
                    auto& dflt = dflt_spec;  // New item.
                    hall::loadspec(conf_rec, dflt, item_ptr, menuid, is_splitter);
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

            LISTEN(tier::release, e2::command::run, script)
            {
                if (script.gear_id)
                {
                    bell::passover(); // Continue release riseup.
                }
                else
                {
                    indexer.luafx.run_ext_script(*this, script);
                }
            };
            LISTEN(tier::preview, e2::command::gui, gui_cmd)
            {
                auto hit = faux;
                if (gui_cmd.cmd_id == syscmd::focusnextwindow)
                if (auto gear_ptr = base::getref<hids>(gui_cmd.gear_id))
                {
                    auto& gear = *gear_ptr;
                    auto dir = gui_cmd.args.size() ? any_get_or(gui_cmd.args[0], 1) : 1;
                    focus_next_window(gear, dir);
                    hit = true;
                }
                if (!hit) bell::passover();
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

            LISTEN(tier::request, vtm::events::apptype, what)
            {
                auto& setup = menu_list[what.menuid];
                what.type = setup.type;
            };
            LISTEN(tier::request, vtm::events::newapp, what)
            {
                auto& setup = menu_list[what.menuid];
                what.applet = app::shared::builder(setup.type)(setup.appcfg, config);
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
                        gear_ptr = base::getref<hids>(hall_focus);
                        //if (gear_ptr)
                        //{
                        //    appspec.gear_id = hall_focus;
                        //}
                    }
                    if (!gear_ptr && users.size()) // Take any existing.
                    {
                        auto gate_ptr = base::getref<gate>(users.back().first->id);
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
                else gear_ptr = base::getref<hids>(appspec.gear_id);

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

            LISTEN(tier::preview, input::events::keybd::post, gear) // Track last active gear.
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
                auto deed = bell::protos();
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
            LISTEN(tier::release, e2::render::background::prerender, parent_canvas) // Sync the hall basis with the current gate.
            {
                if (auto gate_ptr = base::getref<ui::gate>(parent_canvas.link()))
                {
                    parent_canvas.move_basis(gate_ptr->base::coor());
                }
            };
            auto& layers = base::field<std::array<std::vector<sptr>, 3>>();
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                auto clip = parent_canvas.clip();         // Draw world without clipping. Wolrd has no size.
                parent_canvas.clip(parent_canvas.area()); //

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
                        if (auto window_ptr = std::static_pointer_cast<window_t>(item_ptr))
                        {
                            fasten(window_ptr, window_ptr->highlighted, window_ptr->active, window_ptr->color, parent_canvas);
                            auto zorder = window_ptr->zorder;
                            auto i = zorder == zpos::plain   ? 1 :
                                     zorder == zpos::topmost ? 2 : 0;
                            layers[i].push_back(item_ptr);
                        }
                    }
                    for (auto& layer : layers)
                    {
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
                parent_canvas.clip(clip); // Restore drawing 2D context.
            };
            base::signal(tier::general, e2::config::fps, ui::skin::globals().maxfps);
        }

        // hall: Autorun apps from config.
        void autorun()
        {
            auto& config = bell::indexer.config;
            auto what = applink{};
            auto autorun_context = config.settings::push_context(path::autorun);
            auto apps = config.settings::take_ptr_list_for_name("run");
            auto foci = book{};
            foci.reserve(apps.size());
            for (auto app_ptr : apps)
            {
                if (app_ptr && !app_ptr->base)
                {
                    what.menuid =   config.settings::take_value_from(app_ptr, attr::id, ""s);
                    what.square = { config.settings::take_value_from(app_ptr, attr::wincoor, dot_00),
                                    config.settings::take_value_from(app_ptr, attr::winsize, twod{ 80,27 }) };
                    auto winform =  config.settings::take_value_from(app_ptr, attr::winform, winstate::normal, shared::win::options);
                    auto focused =  config.settings::take_value_from(app_ptr, attr::focused, faux);
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
        auto invite(xipc client, view userid, si32 vtmode, auto& packet, si32 session_id)
        {
            auto lock = bell::unique_lock();
            auto usergate_ptr = ui::gate::ctor(client, vtmode, userid, session_id, true);
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
                        auto deed = applet.bell::protos();
                        if (deed == e2::form::size::enlarge::maximize.id)
                        {
                            usergate.base::signal(tier::release, e2::form::size::restore);
                        }
                    };
                    applet.LISTEN(tier::preview, e2::form::size::minimize, gear, memo)
                    {
                        usergate.base::signal(tier::release, e2::form::size::restore);
                    };
                    applet.LISTEN(tier::release, e2::form::proceed::quit::one, fast, memo)
                    {
                        usergate.base::signal(tier::release, e2::form::size::restore);
                        applet.bell::passover(); // Continue event riseup().
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
                        usergate.bell::expire(); // Stop event forwarding.
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
                    base::deface();
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
            usergate.on(tier::mouserelease, input::key::LeftClick, [&](hids& gear) // Fly to another user's viewport.
            {
                if (gear.owner.id == usergate.id) return;
                auto center = usergate.base::coor() + gear.owner.base::size() / 2;
                gear.owner.base::signal(tier::release, e2::form::layout::shift, center);
            });
            usergate.LISTEN(tier::release, e2::conio::mouse, m) // Trigger to redraw all gates on mouse activity (to redraw foreign mouse cursor).
            {
                base::deface();
            };
            usergate.LISTEN(tier::release, e2::conio::winsz, w) // Trigger to redraw all gates.
            {
                base::deface();
            };
            auto& drag_origin = usergate.base::field<fp2d>();
            auto& user_mouse = usergate.base::plugin<pro::mouse>();
            user_mouse.template draggable<hids::buttons::leftright>(true);
            if (!usergate.direct) // In dtvt+gui mode the left button draggability will be activated on set_fullscreen.
            {
                user_mouse.template draggable<hids::buttons::left>(true);
            }
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
                    base::deface();
                }
            };
            usergate.LISTEN(tier::release, e2::form::drag::stop::any, gear)
            {
                if (gear.owner.id != usergate.id) return;
                robot.pacify(usergate.id);
                robot.actify(usergate.id, gear.fader<quadratic<twod>>(2s), [&](auto delta)
                {
                    usergate.base::moveby(-delta);
                    base::deface();
                });
            };

            auto& vport = base::property<twod>("desktop.viewport"); // hall: Last user's viewport position.
            auto& selected_item = base::property<text>("desktop.selected"); // hall: Last user's selected menu item.
            auto& usergate_selected_item = usergate.base::property<text>("desktop.selected");
            auto& config = bell::indexer.config;
            if (!vport)
            {
                vport = config.settings::take(path::viewport, dot_00);
            }
            if (selected_item.empty())
            {
                selected_item = config.settings::take(path::selected, selected_item);
            }
            usergate_selected_item = selected_item;
            //auto& usergate_id = usergate.base::property<id_t>("gate.id");
            //auto& usergate_os_id = usergate.base::property<text>("gate.os_id");
            auto usrcfg = eccc{ .env = packet.env,
                                .cwd = packet.cwd,
                                .cmd = packet.cmd,
                                .cfg = utf::concat(usergate.id, ";", usergate.props.os_user_id),
                                .win = packet.win };
            auto deskmenu_ptr = app::shared::builder(app::desk::id)(usrcfg, config);
            deskmenu_ptr->base::plugin<pro::keybd>();
            //todo
            //deskmenu_ptr->base::add_methods(basename::taskbar)...
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
            base::dequeue(); // Wait until all cleanups are completed.
        }
    };
}