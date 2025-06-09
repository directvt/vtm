// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

// desk: Taskbar.
namespace netxs::app::desk
{
    static constexpr auto id = "desk";
    static constexpr auto name = "Taskbar menu";

    struct spec
    {
        text   menuid{};
        bool   hidden{}; // Hide existing item on taskbar.
        bool    fixed{}; // Item can't be updated by the new instance (see desk::events::exec).
        text    label{};
        text  tooltip{};
        text    title{};
        text   footer{};
        twod  winsize{};
        twod  wincoor{};
        si32  winform{};
        bool splitter{};
        eccc   appcfg{};
        text     type{};
        bool   folded{};
        bool notfound{};
        id_t  gear_id{};
    };

    using menu = utf::unordered_map<text, spec>;
    using usrs = std::list<ui::sptr>;
    using apps = generics::imap<text, std::pair<bool, usrs>>;

    namespace events
    {
        EVENTPACK( desk::events, ui::e2::extra::slot2 )
        {
            EVENT_XS( usrs, netxs::sptr<desk::usrs> ), // List of connected users.
            EVENT_XS( apps, netxs::sptr<desk::apps> ), // List of running apps.
            EVENT_XS( menu, netxs::sptr<desk::menu> ), // List of registered apps.
            EVENT_XS( exec, spec                    ), // Request to run app.
            EVENT_XS( quit, bool                    ), // Request to close all instances.
            GROUP_XS( ui  , text                    ),

            SUBSET_XS( ui )
            {
                EVENT_XS( sync    , bool        ),
                EVENT_XS( selected, text        ),
                EVENT_XS( toggle  , bool        ), // Request taskbar toggle.
                EVENT_XS( recalc  , bool        ), // Request taskbar recalc.
                EVENT_XS( id      , id_t        ), // Request owner id.
                GROUP_XS( focus   , input::hids ),

                SUBSET_XS( focus )
                {
                    EVENT_XS( set, input::hids ), // Request focus.
                    EVENT_XS( off, input::hids ), // Request unfocus.
                };
            };
        };
    }

    namespace
    {
        auto app_template = [](auto& data_src, auto const& utf8)
        {
            auto tall = si32{ skin::globals().menuwide };
            auto focused_color   = skin::globals().focused;
            auto danger_color    = skin::globals().danger;
            auto active_color    = skin::globals().active;
            auto cE = active_color;
            auto c1 = danger_color;
            auto cF = focused_color;
            auto item_area = ui::fork::ctor(axis::X, 0, 1, 0);
            auto& src_wptr = item_area->base::field(ptr::shadow(data_src));
            item_area->active(cE)
                ->shader(cell::shaders::xlight, e2::form::state::hover)
                ->shader<tier::release, e2::postrender>(cell::shaders::disabled, e2::form::state::disabled)
                ->plugin<pro::notes>()
                ->setpad({ 0, 0, 0, 0 }, { 0, 0, -tall, 0 })
                ->invoke([&](auto& boss)
                {
                    boss.on(tier::mouserelease, input::key::LeftDoubleClick, [&](hids& gear)
                    {
                        if (auto data_src = src_wptr.lock())
                        {
                            auto& window = *data_src;
                            if (gear.meta(hids::anyAlt)) // Pull window.
                            {
                                window.base::riseup(tier::preview, e2::form::layout::expose);
                                auto viewport = gear.owner.base::signal(tier::request, e2::form::prop::viewport);
                                window.base::signal(tier::preview, e2::form::layout::appear, viewport.center()); // Pull window.
                                if (window.hidden) // Restore if minimized.
                                {
                                    window.base::signal(tier::preview, e2::form::size::minimize, gear);
                                }
                                else pro::focus::set(data_src, gear.id, solo::on);
                            }
                            else // Jump to window.
                            {
                                gear.owner.base::signal(tier::release, e2::form::layout::jumpto, window);
                            }
                            gear.dismiss();
                        }
                    });
                    boss.on(tier::mouserelease, input::key::LeftClick, [&](hids& gear)
                    {
                        if (auto data_src = src_wptr.lock())
                        {
                            auto& window = *data_src;
                            if (gear.meta(hids::anyCtrl)) // Toggle group focus.
                            {
                                if (pro::focus::test(window, gear))
                                {
                                    pro::focus::off(data_src, gear.id); // Remove focus if focused.
                                }
                                else // Expose and set group focus.
                                {
                                    window.base::riseup(tier::preview, e2::form::layout::expose);
                                    if (window.hidden) // Restore if minimized.
                                    {
                                        window.base::signal(tier::preview, e2::form::size::minimize, gear);
                                    }
                                    pro::focus::set(data_src, gear.id, solo::off);
                                }
                                gear.dismiss(true); // Suppress double click.
                            }
                            else if (gear.meta(hids::anyAlt)) // Skip it and wait for Alt+Dblclick.
                            {
                                gear.dismiss();
                            }
                            else // Set unique focus.
                            {
                                window.base::riseup(tier::preview, e2::form::layout::expose);
                                if (window.hidden) // Restore if minimized.
                                {
                                    window.base::signal(tier::preview, e2::form::size::minimize, gear);
                                }
                                else pro::focus::set(data_src, gear.id, solo::on);
                                gear.dismiss();
                            }
                        }
                    });
                    boss.on(tier::mouserelease, input::key::RightClick, [&](hids& /*gear*/)
                    {
                        // Reserved for context menu.
                    });
                    boss.LISTEN(tier::release, e2::form::state::mouse, hovered)
                    {
                        if (auto data_src = src_wptr.lock())
                        {
                            data_src->base::signal(tier::release, e2::form::state::highlight, hovered);
                        }
                    };
                });
            auto app_label = item_area->attach(slot::_1, ui::item::ctor(ansi::add(utf8).mgl(0).wrp(wrap::off).jet(bias::left)))
                ->active()
                ->setpad({ tall + 1, 0, tall, tall })
                ->template plugin<pro::notes>(" Running application          \n"
                                              "   LeftClick to activate      \n"
                                              "   DoubleLeftClick to fly to  ")
                ->flexible()
                ->drawdots()
                ->shader(cF, e2::form::state::focus::count, data_src);
            auto app_close = item_area->attach(slot::_2, ui::item::ctor("×"))
                ->active()
                ->shader(c1, e2::form::state::hover)
                ->setpad({ 2, 2, tall, tall })
                ->template plugin<pro::notes>(" Close application window ")
                ->invoke([&](auto& boss)
                {
                    boss.base::hidden = true;
                    item_area->LISTEN(tier::release, e2::form::state::mouse, hovered)
                    {
                        //auto unfolded = boss.base::riseup(tier::request, desk::events::ui::toggle);
                        //auto hidden = !unfolded || !hover;
                        //auto folded = item_area_inst.base::size().x <= boss.base::size().x * 2;
                        //auto hidden = folded || !hover;
                        auto hidden = !hovered;
                        if (boss.base::hidden != hidden)
                        {
                            boss.base::hidden = hidden;
                            boss.base::reflow();
                        }
                    };
                    item_area->LISTEN(tier::release, e2::form::upon::vtree::attached, parent, boss.sensors)
                    {
                        parent->LISTEN(tier::release, desk::events::quit, fast, boss.sensors)
                        {
                            if (auto data_src = src_wptr.lock())
                            {
                                data_src->base::enqueue([&](auto& data_inst) // Enqueue in order to pass focus one by one.
                                {
                                    data_inst.base::signal(tier::anycast, e2::form::proceed::quit::one, fast); // Show closing process.
                                });
                            }
                        };
                    };
                    boss.on(tier::mouserelease, input::key::LeftClick, [&](hids& gear)
                    {
                        if (auto data_src = src_wptr.lock())
                        {
                            data_src->base::signal(tier::anycast, e2::form::proceed::quit::one, faux); // Show closing process.
                            gear.dismiss(true);
                        }
                    });
                });
            return item_area;
        };
        auto apps_template = [](auto& data_src, auto& apps_map_ptr)
        {
            auto tall = si32{ skin::globals().menuwide };
            auto inactive_color  = skin::globals().inactive;
            auto selected_color  = skin::globals().selected;
            auto danger_color    = skin::globals().danger;
            auto c1 = danger_color;
            auto c9 = selected_color;
            auto cA = inactive_color;

            auto apps = ui::list::ctor()
                ->invoke([&](auto& boss)
                {
                    boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent)
                    {
                        auto current_default = boss.base::riseup(tier::request, e2::data::changed);
                        boss.base::signal(tier::anycast, desk::events::ui::selected, current_default);
                        //todo combine anycasts (update on user disconnect)
                        auto state = boss.base::riseup(tier::request, desk::events::ui::toggle);
                        boss.base::riseup(tier::anycast, desk::events::ui::recalc, state);
                    };
                });

            auto def_note = text{" Application:                                  \n"
                                 "   LeftClick to start the application instance \n"
                                 "   RightClick to set it as default             \n"
                                 "   LeftDrag to move desktop viewport           "};
            auto conf_list_ptr = data_src->base::riseup(tier::request, desk::events::menu);
            if (!conf_list_ptr || !apps_map_ptr) return apps;
            auto& conf_list = *conf_list_ptr;
            auto& apps_map = *apps_map_ptr;
            //todo optimize: use a branch_template for sublist instead of recreating whole app list
            for (auto const& [class_id, stat_inst_ptr_list] : apps_map)
            {
                auto& [state, inst_ptr_list] = stat_inst_ptr_list;
                auto inst_id = class_id;
                auto conf_it = conf_list.find(class_id);
                if (conf_it == conf_list.end()) conf_it = conf_list.insert({ class_id, { .menuid = class_id }}).first; // Empty menu case (vtm.del()).
                if (conf_it->second.label.empty()) // Avoid using empty groups.
                {
                    auto& conf = conf_it->second;
                    conf.label = ansi::err(ansi::stk(1), class_id, ansi::stk(0));
                    conf.hidden = true;
                }
                auto& conf = conf_it->second;
                auto& obj_desc = conf.label;
                auto& obj_note = conf.tooltip;
                if (conf.splitter)
                {
                    auto item_area = apps->attach(ui::item::ctor(obj_desc))
                        ->flexible()
                        ->accented()
                        ->colors(cA)
                        ->setpad({ 0, 0, tall, tall }, { 0, 0, -tall, 0 })
                        ->template plugin<pro::notes>(obj_note);
                    continue;
                }
                auto head_fork = ui::fork::ctor(axis::X, 0, 1, 0);
                auto block = apps->attach(ui::list::ctor())
                    ->shader(cell::shaders::xlight, e2::form::state::hover, head_fork)
                    ->setpad({ 0, 0, 0, 0 }, { 0, 0, -tall, 0 });
                if (!state) block->depend_on_collection(inst_ptr_list); // Remove not pinned apps, like Info.
                block->attach(head_fork)
                    ->active()
                    ->template plugin<pro::notes>(obj_note.empty() ? def_note : obj_note)
                    ->invoke([&](auto& boss)
                    {
                        boss.on(tier::mouserelease, input::key::RightClick, [&, inst_id](hids& gear)
                        {
                            boss.base::signal(tier::anycast, desk::events::ui::selected, inst_id);
                            gear.dismiss(true);
                        });
                        boss.on(tier::mouserelease, input::key::LeftClick, [&, inst_id, group_focus = faux](hids& gear) mutable
                        {
                            if (gear.meta(hids::anyCtrl | hids::anyAlt | hids::anyShift | hids::anyWin)) // Not supported with any modifier but Ctrl.
                            {
                                if (gear.meta(hids::anyCtrl)) // Toggle group focus.
                                {
                                    group_focus = !group_focus;
                                    if (group_focus) boss.base::signal(tier::release, desk::events::ui::focus::set, gear);
                                    else             boss.base::signal(tier::release, desk::events::ui::focus::off, gear);
                                }
                                gear.dismiss(true);
                                return;
                            }
                            boss.base::signal(tier::anycast, desk::events::ui::selected, inst_id);
                            static auto offset = dot_00; // static: Share initial offset between all instances.
                            if (auto world_ptr = boss.base::signal(tier::general, e2::config::creator))
                            {
                                auto current_viewport = gear.owner.base::signal(tier::request, e2::form::prop::viewport);
                                offset = (offset + dot_21 * 2) % std::max(dot_11, current_viewport.size * 7 / 32);
                                gear.slot.coor = current_viewport.coor + offset + current_viewport.size * 1 / 32 + dot_11;
                                gear.slot.size = current_viewport.size * 3 / 4;
                                gear.slot_forced = faux;
                                world_ptr->base::riseup(tier::request, e2::form::proceed::createby, gear);
                            }
                            gear.dismiss(true);
                        });
                    });
                auto head = head_fork->attach(slot::_1, ui::item::ctor(obj_desc)
                    ->flexible())
                    ->setpad({ 0, 0, tall, tall })
                    ->invoke([&](auto& boss)
                    {
                        auto boss_shadow = ptr::shadow(boss.This());
                        boss.LISTEN(tier::anycast, desk::events::ui::selected, data, -, (inst_id, obj_desc, c9))
                        {
                            auto selected = inst_id == data;
                            boss.brush(selected ? c9 : cell{});
                            boss.set(obj_desc);
                            boss.base::deface();
                        };
                    });
                if (auto count = inst_ptr_list.size())
                {
                    auto& isfolded = conf.folded;
                    auto insts = block->attach(ui::list::ctor())
                        ->setpad({ 0, 0, tall, 0 }, { 0, 0, -tall * 2, 0 });
                    auto bttn_rail = head_fork->attach(slot::_2, ui::rail::ctor(axes::X_only, axes::all, axes::none))
                        ->limits({ 5, -1 }, { 5, -1 })
                        ->invoke([&](auto& boss)
                        {
                            boss.LISTEN(tier::release, e2::form::state::mouse, hovered)
                            {
                                if (!hovered)
                                {
                                    boss.base::riseup(tier::preview, e2::form::upon::scroll::to_top::v);
                                }
                            };
                        });
                    auto bttn_fork = bttn_rail->attach(ui::fork::ctor(axis::X));
                    auto fold_bttn = bttn_fork->attach(slot::_1, ui::item::ctor(isfolded ? "…" : "<"))
                        ->setpad({ 2, 2, tall, tall })
                        ->active()
                        ->shader(cell::shaders::xlight, e2::form::state::hover)
                        ->template plugin<pro::notes>(" Window list disclosure toggle                  \n"
                                                      "   LeftClick to expand/collapse the window list \n"
                                                      "   MouseWheel to switch to list closing mode    ")
                        ->invoke([&](auto& boss)
                        {
                            insts->base::hidden = isfolded;
                            auto insts_shadow = ptr::shadow(insts);
                            boss.on(tier::mouserelease, input::key::LeftClick, [&, insts_shadow](hids& gear)
                            {
                                if (auto insts = insts_shadow.lock())
                                {
                                    isfolded = !isfolded;
                                    boss.set(isfolded ? "…" : "<");
                                    insts->base::hidden = isfolded;
                                    insts->base::reflow();
                                }
                                gear.dismiss(true);
                            });
                        });
                    auto drop_bttn = bttn_fork->attach(slot::_2, ui::item::ctor("×"))
                        ->setpad({ 2, 2, tall, tall })
                        ->active()
                        ->shader(c1, e2::form::state::hover)
                        ->template plugin<pro::notes>(" Close all open windows in the group ")
                        ->invoke([&](auto& boss)
                        {
                            auto insts_shadow = ptr::shadow(insts);
                            boss.on(tier::mouserelease, input::key::LeftClick, [&, insts_shadow](hids& gear)
                            {
                                if (auto insts = insts_shadow.lock())
                                {
                                    insts->base::signal(tier::release, desk::events::quit, faux); // Show closing process.
                                }
                                gear.dismiss(true);
                            });
                        });
                    insts->attach_collection(e2::form::prop::ui::title, inst_ptr_list, app_template, [&](auto inst_ptr)
                    {
                        auto& window = *inst_ptr;
                        auto& boss = *block;
                        boss.LISTEN(tier::release, desk::events::ui::focus::any, gear, window.sensors)
                        {
                            auto deed = boss.bell::protos();
                                 if (deed == desk::events::ui::focus::set.id) pro::focus::set(window.This(), gear.id, solo::off);
                            else if (deed == desk::events::ui::focus::off.id) pro::focus::off(window.This(), gear.id);
                        };
                    });
                }
            }
            return apps;
        };

        auto build = [](eccc usrcfg, settings& config)
        {
            auto tall = si32{ skin::globals().menuwide };
            auto inactive_color  = skin::globals().inactive;
            auto danger_color    = skin::globals().danger;
            auto cA = inactive_color;
            auto c1 = danger_color;

            auto menu_bg_color = config.settings::take("/config/desktop/taskbar/colors/bground", cell{}.fgc(whitedk).bgc(0x60202020));
            auto menu_min_conf = config.settings::take("/config/desktop/taskbar/width/folded",   si32{ 5  });
            auto menu_max_conf = config.settings::take("/config/desktop/taskbar/width/expanded", si32{ 32 });
            auto bttn_min_size = twod{ 31, 1 + tall * 2 };
            auto bttn_max_size = twod{ -1, 1 + tall * 2 };

            auto window = ui::fork::ctor(axis::Y, 0, 0, 1);
            auto panel_top = config.settings::take("/config/desktop/panel/height", 1);
            auto panel_env = config.settings::take("/config/desktop/panel/env", ""s);
            auto panel_cwd = config.settings::take("/config/desktop/panel/cwd", ""s);
            auto panel_cmd = config.settings::take("/config/desktop/panel/cmd", ""s);
            auto panel = window->attach(slot::_1, ui::cake::ctor());
            if (panel_cmd.size())
            {
                auto panel_cfg = eccc{ .env = panel_env,
                                       .cwd = panel_cwd,
                                       .cmd = panel_cmd };
                panel_top = std::max(1, panel_top);
                panel->limits({ -1, panel_top }, { -1, panel_top })
                     ->attach(app::shared::builder(app::vtty::id)(panel_cfg, config));
            }

            auto highlight_color = skin::color(tone::winfocus);
            auto c8 = cell{}.bgc(argb::active_transparent).fgc(highlight_color.bgc());

            auto user_info = utf::split(usrcfg.cfg, ";");
            auto& user_id__view = user_info[0];
            auto& username_view = user_info[1];
            log("%%User %name% connected", prompt::desk, username_view);

            auto my_id = id_t{};
            if (auto value = utf::to_int(user_id__view)) my_id = value.value();
            else
            {
                log(prompt::desk, "Bad user ID=", user_id__view);
                return window;
            }

            auto user_template = [my_id](auto& data_src, auto const& utf8)
            {
                auto tall = si32{ skin::globals().menuwide };
                auto active_color    = skin::globals().active;
                auto cE = active_color;
                auto user = ui::item::ctor(escx(" &").nil().add(" ").wrp(wrap::off)
                        .fgx(data_src->id == my_id ? cE.fgc() : argb{}).add(utf8).nil())
                    ->flexible()
                    ->setpad({ 1, 0, tall, tall }, { 0, 0, -tall, 0 })
                    ->active()
                    ->shader(cell::shaders::xlight, e2::form::state::hover)
                    ->template plugin<pro::notes>(" Connected user ");
                return user;
            };
            auto branch_template = [user_template](auto& /*data_src*/, auto& usr_list)
            {
                auto tall = si32{ skin::globals().menuwide };
                auto users = ui::list::ctor()
                    ->setpad({ 0, 0, tall, 0 }, { 0, 0, -tall, 0 })
                    ->attach_collection(e2::form::prop::name, *usr_list, user_template);
                return users;
            };

            auto& size_config = window->base::field(std::tuple{ menu_max_conf, menu_min_conf, faux });
            //todo Apple Clang don't get it.
            //auto& [menu_max_size, menu_min_size, active] = size_config;
            auto& menu_max_size = std::get<0>(size_config);
            auto& menu_min_size = std::get<1>(size_config);
            auto& active        = std::get<2>(size_config);

            auto world_ptr = window->base::signal(tier::general, e2::config::creator);
            if (!world_ptr) return window;
            auto& world = *world_ptr;
            window->invoke([&](auto& boss) mutable
            {
                boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent_ptr, -, (usrcfg))
                {
                    if (!parent_ptr) return;
                    auto& parent = *parent_ptr; //todo This is ui::gate.

                    parent.base::riseup(tier::release, e2::form::proceed::multihome, world.This()); // Register multi-parent.

                    auto& current_default = parent.base::property<text>("desktop.selected");
                    auto& previous_default = parent.base::property<text>("desktop.prev_selected");
                    previous_default = current_default;
                    parent.base::signal(tier::anycast, desk::events::ui::selected, current_default);
                    parent.LISTEN(tier::request, e2::data::changed, data, boss.relyon)
                    {
                        data = current_default;
                    };
                    parent.LISTEN(tier::release, e2::data::changed, new_default, boss.relyon)
                    {
                        boss.base::signal(tier::anycast, desk::events::ui::selected, new_default);
                    };
                    parent.LISTEN(tier::anycast, desk::events::ui::selected, new_default, boss.relyon)
                    {
                        if (current_default != new_default)
                        {
                            previous_default = std::exchange(current_default, new_default);
                        }
                    };
                    boss.LISTEN(tier::release, e2::area, new_area)
                    {
                        auto viewport = new_area - dent{ menu_min_size };
                        boss.base::riseup(tier::release, e2::form::prop::viewport, viewport);
                    };
                    parent.LISTEN(tier::request, e2::form::prop::viewport, viewport, boss.relyon)
                    {
                        viewport -= dent{ menu_min_size };
                    };
                    boss.LISTEN(tier::request, desk::events::ui::id, owner_id, boss.relyon)
                    {
                        owner_id = parent.id;
                    };
                    auto& oneshot = boss.base::template field<hook>();
                    parent.LISTEN(tier::release, input::events::focus::set::any, seed, oneshot, (usrcfg))
                    {
                        usrcfg.win = {};
                        usrcfg.gear_id = seed.gear_id;
                        boss.base::riseup(tier::release, e2::command::run, usrcfg);
                        boss.base::unfield(oneshot);
                    };
                };
            });
            auto ground = window->attach(slot::_2, ui::cake::ctor());
            auto ver_label = ground->attach(ui::item::ctor(utf::concat(app::shared::version)))
                ->active(cell{}.fgc(whitedk).bgc(argb::active_transparent))
                ->shader(c8, e2::form::state::hover)
                ->limits({}, { -1, 1 })
                ->alignment({ snap::tail, snap::tail })
                ->template plugin<pro::notes>(" Info ")
                ->invoke([&](auto& boss)
                {
                    auto infospec = spec{ .menuid = "vtm_info_page", .hidden = true, .label = "Info", .title = "Info", .type = "info" };
                    boss.on(tier::mouserelease, input::key::LeftClick, [&, infospec](hids& gear) mutable
                    {
                        infospec.gear_id = gear.id;
                        world.base::signal(tier::request, desk::events::exec, infospec);
                        gear.dismiss(true);
                    });
                });
            ground->attach(world_ptr);
            auto taskbar_viewport = ground->attach(ui::fork::ctor(axis::X));
            auto viewport = taskbar_viewport->attach(slot::_2, ui::mock::ctor())
                ->invoke([&](auto& boss)
                {
                    boss.LISTEN(tier::release, e2::postrender, parent_canvas) // Draw a shadow to the right of the taskbar.
                    {
                        auto vert_line = parent_canvas.clip();
                        vert_line.size.x = 1;
                        parent_canvas.fill(vert_line, cell::shaders::shadow(ui::pro::ghost::x1y1_x1y2_x1y3));
                    };
                });
            auto taskbar_grips = taskbar_viewport->attach(slot::_1, ui::fork::ctor(axis::X))
                ->limits({ menu_min_size, -1 }, { menu_min_size, -1 })
                ->plugin<pro::timer>()
                ->plugin<pro::acryl>()
                ->plugin<pro::cache>()
                ->active(menu_bg_color)
                ->invoke([&](auto& boss)
                {
                    boss.LISTEN(tier::request, desk::events::ui::toggle, state)
                    {
                        state = active;
                    };
                    boss.LISTEN(tier::preview, desk::events::ui::toggle, state)
                    {
                        active = state;
                        auto size = active ? menu_max_size
                                           : menu_min_size;
                        auto lims = twod{ size, -1 };
                        boss.base::limits(lims, lims);
                        boss.base::signal(tier::anycast, desk::events::ui::recalc, state);
                        boss.base::deface();
                        boss.base::reflow();
                    };
                    boss.on(tier::mouserelease, input::key::MouseClick, [&](hids& /*gear*/)
                    {
                        boss.base::riseup(tier::preview, desk::events::ui::toggle, !active);
                    });
                    boss.LISTEN(tier::release, e2::form::state::mouse, hovered)
                    {
                        auto& timer = boss.base::template plugin<pro::timer>();
                        if (hovered)
                        {
                            timer.pacify(faux);
                            return;
                        }
                        // Only when mouse leaving.
                        auto toggle = [&](auto state)
                        {
                            boss.base::riseup(tier::preview, desk::events::ui::toggle, state);
                            return faux; // One shot call.
                        };
                        timer.actify(faux, skin::globals().menu_timeout, toggle);
                    };
                });
            auto grips = taskbar_grips->attach(slot::_2, ui::mock::ctor())
                ->limits({ 1, -1 }, { 1, -1 })
                ->template plugin<pro::notes>(" LeftDrag to adjust taskbar width ")
                ->active()
                //->template plugin<pro::focus>(pro::focus::mode::focusable)
                //->shader(c3, e2::form::state::focus::count)
                ->shader(cell::shaders::xlight, e2::form::state::hover)
                ->invoke([&](auto& boss)
                {
                    auto& drag_origin = boss.base::template field<fp2d>();
                    auto& mouse = boss.base::template plugin<pro::mouse>();
                    mouse.template draggable<hids::buttons::left>(true);
                    boss.LISTEN(tier::release, e2::form::drag::start::_<hids::buttons::left>, gear)
                    {
                        drag_origin = gear.coord;
                    };
                    boss.LISTEN(tier::release, e2::form::drag::pull::_<hids::buttons::left>, gear)
                    {
                        if (auto taskbar_grips = boss.base::parent())
                        {
                            if (auto delta = (twod{ gear.coord } - twod{ drag_origin })[axis::X])
                            {
                                taskbar_grips->base::min_sz.x = std::max(1, taskbar_grips->base::min_sz.x + delta);
                                taskbar_grips->base::max_sz.x = taskbar_grips->base::min_sz.x;
                                active ? menu_max_size = taskbar_grips->base::min_sz.x
                                       : menu_min_size = taskbar_grips->base::min_sz.x;
                                taskbar_grips->base::reflow();
                            }
                        }
                    };
                    boss.LISTEN(tier::release, desk::events::ui::sync, state)
                    {
                        if (menu_min_size > menu_max_size)
                        {
                            active ? menu_min_size = menu_max_size
                                   : menu_max_size = menu_min_size;
                        }
                    };
                    boss.LISTEN(tier::release, e2::form::drag::cancel::_<hids::buttons::left>, gear)
                    {
                        boss.base::signal(tier::release, desk::events::ui::sync, true);
                    };
                    boss.LISTEN(tier::release, e2::form::drag::stop::_<hids::buttons::left>, gear)
                    {
                        boss.base::signal(tier::release, desk::events::ui::sync, true);
                    };
                });
            auto taskbar_park = taskbar_grips->attach(slot::_1, ui::cake::ctor());
            auto taskbar = taskbar_park->attach(ui::fork::ctor(axis::Y)->alignment({ snap::head, snap::head }, { snap::head, snap::tail }));
            auto apps_users = taskbar->attach(slot::_1, ui::fork::ctor(axis::Y, 0, 100))
                ->setpad({}, { 0, 0, 0, -tall }); // To place above Disconnect button.
            auto applist_area = apps_users->attach(slot::_1, ui::cake::ctor());
            auto tasks_scrl = applist_area->attach(ui::rail::ctor(axes::Y_only))
                ->plugin<pro::notes>(" Desktop Taskbar                     \n"
                                     "   RightDrag to scroll menu up/down  \n"
                                     "   LeftDrag to move desktop viewport ")
                ->active()
                ->invoke([&](auto& boss)
                {
                    boss.LISTEN(tier::anycast, e2::form::upon::started, root_ptr)
                    {
                        auto world_ptr = world.This();
                        auto apps = boss.attach_element(desk::events::apps, world_ptr, apps_template);
                    };
                });
            auto users_area = apps_users->attach(slot::_2, ui::list::ctor());
            auto label_bttn = users_area->attach(ui::fork::ctor(axis::X))
                                        ->plugin<pro::notes>(" List of active connections ");
            auto label = label_bttn->attach(slot::_1, ui::item::ctor(os::process::elevated ? "admins" : "users"))
                ->flexible()
                ->accented()
                ->colors(cA)
                ->setpad({ 0, 0, tall, tall })
                ->limits({ 5, -1 });
            auto userlist_hidden = true;
            auto bttn = label_bttn->attach(slot::_2, ui::item::ctor(userlist_hidden ? "…" : "<"))
                ->active()
                ->shader(cell::shaders::xlight, e2::form::state::hover)
                ->plugin<pro::notes>(" Show/hide user list ")
                ->setpad({ 2, 2, tall, tall });
            auto userlist_area = users_area->attach(ui::cake::ctor())
                ->setpad({}, { 0, 0, -tall, 0 }) // To place above the admins/users label.
                ->invoke([&](auto& boss)
                {
                    boss.base::hidden = userlist_hidden;
                    boss.LISTEN(tier::anycast, e2::form::upon::started, root_ptr)
                    {
                        auto world_ptr = world.This();
                        auto users = boss.attach_element(desk::events::usrs, world_ptr, branch_template);
                    };
                });
            bttn->invoke([&](auto& boss)
            {
                auto userlist_area_shadow = ptr::shadow(userlist_area);
                auto bttn_shadow = ptr::shadow(bttn);
                boss.on(tier::mouserelease, input::key::LeftClick, [&, userlist_area_shadow, bttn_shadow](hids& gear)
                {
                    if (auto bttn = bttn_shadow.lock())
                    if (auto userlist_area = userlist_area_shadow.lock())
                    {
                        auto& hidden = userlist_area->base::hidden;
                        hidden = !hidden;
                        bttn->set(hidden ? "…" : "<");
                        userlist_area->base::reflow();
                    }
                    gear.dismiss(true);
                });
            });
            auto bttns_cake = taskbar->attach(slot::_2, ui::cake::ctor());
            auto bttns_area = bttns_cake->attach(ui::rail::ctor(axes::X_only))
                ->limits({ -1, 1 + tall * 2 }, { -1, 1 + tall * 2 })
                ->invoke([&](auto& boss)
                {
                    boss.LISTEN(tier::release, e2::form::state::mouse, hovered)
                    {
                        if (hovered)
                        {
                            boss.base::riseup(tier::preview, desk::events::ui::toggle, hovered);
                        }
                    };
                });
            bttns_cake->attach(app::shared::underlined_hz_scrollbar(bttns_area));
            auto bttns = bttns_area->attach(ui::fork::ctor(axis::X))
                ->limits(bttn_min_size, bttn_max_size);
            auto disconnect_park = bttns->attach(slot::_1, ui::cake::ctor())
                ->active()
                ->shader(cell::shaders::xlight, e2::form::state::hover)
                ->plugin<pro::notes>(" Leave current session ")
                ->invoke([&, name = text{ username_view }](auto& boss)
                {
                    boss.on(tier::mouserelease, input::key::LeftClick, [&, name](hids& gear)
                    {
                        log("%%User %name% disconnected", prompt::desk, name);
                        gear.owner.base::signal(tier::preview, e2::conio::quit);
                        gear.dismiss(true);
                    });
                });
            auto disconnect = disconnect_park->attach(ui::item::ctor("× Disconnect"))
                ->setpad({ 1 + tall, 1 + tall, tall, tall })
                ->alignment({ snap::head, snap::center });
            auto shutdown_park = bttns->attach(slot::_2, ui::cake::ctor())
                ->active()
                ->shader(c1, e2::form::state::hover)
                ->plugin<pro::notes>(" Disconnect all users and shutdown ")
                ->invoke([&](auto& boss)
                {
                    boss.on(tier::mouserelease, input::key::LeftClick, [&](hids& gear)
                    {
                        boss.base::signal(tier::general, e2::shutdown, utf::concat(prompt::desk, "Server shutdown"));
                        gear.dismiss(true);
                    });
                });
            auto shutdown = shutdown_park->attach(ui::item::ctor("× Shutdown"))
                ->setpad({ 1 + tall, 1 + tall, tall, tall })
                ->alignment({ snap::tail, snap::center });
            return window;
        };
    }

    app::shared::initialize builder{ app::desk::id, build };
}