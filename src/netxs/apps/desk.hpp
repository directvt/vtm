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

    namespace events
    {
        EVENTPACK( desk::events, ui::e2::extra::slot2 )
        {
            EVENT_XS( usrs, netxs::sptr<desk::usrs> ), // List of connected users.
            EVENT_XS( menu, netxs::sptr<desk::menu> ), // List of registered apps.
            EVENT_XS( exec, spec                    ), // Request to run app.
            EVENT_XS( quit, bool                    ), // Request to close all instances.
            GROUP_XS( apps, netxs::ui::sptr         ),
            GROUP_XS( ui  , text                    ),

            SUBSET_XS( apps )
            {
                EVENT_XS( getmodel, netxs::ui::sptr ), // request: Request the current taskbar model (root_sptr).
                EVENT_XS( created , netxs::ui::sptr ), // release: App window created.
                EVENT_XS( removed , netxs::ui::sptr ), // release: App window removed.
                EVENT_XS( enlist  , netxs::ui::sptr ), // release: App group added by new_appmodel_ptr.
                EVENT_XS( delist  , netxs::ui::sptr ), // release: App group removed by new_appmodel_ptr.
                EVENT_XS( title   , text            ), // release: Window title changed.
            };
            SUBSET_XS( ui )
            {
                EVENT_XS( sync    , bool        ),
                EVENT_XS( selected, text        ),
                EVENT_XS( toggle  , bool        ), // Request taskbar toggle.
                EVENT_XS( recalc  , bool        ), // Request taskbar recalc.
                EVENT_XS( id      , id_t        ), // Request owner id.
                EVENT_XS( activate, input::hids ), // Release: Activate object (same as click in most cases).
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
        static constexpr auto weight_app_group = 100;
        static constexpr auto weight_app_label = 50;
        static constexpr auto weight_ui_button = 10;

        auto app_template = [](auto new_appmodel_ptr)
        {
            auto tall = si32{ skin::globals().menuwide };
            auto focused_color   = skin::globals().focused;
            auto danger_color    = skin::globals().danger;
            auto active_color    = skin::globals().active;
            auto highlight_color = cell{ skin::globals().winfocus };
            auto c3 = highlight_color;
            auto cE = active_color;
            auto c1 = danger_color;
            auto cF = focused_color;
            auto& new_app = *new_appmodel_ptr;
            auto& current_title = new_app.base::template property<ansi::escx>("window.title"); //todo Apple clang requires templtate
            auto& window_wptr = new_app.base::template property<ui::wptr>("window.wptr");      //
            auto window_ptr = window_wptr.lock();
            auto item_area_ptr = ui::fork::ctor(axis::X, 0, 1, 0);
            item_area_ptr->depend(window_ptr);
            auto& item_area = *item_area_ptr;
            auto& window = *window_ptr;
            item_area_ptr->active(cE)
                ->shader(cell::shaders::xlight, e2::form::state::hover)
                ->shader<tier::release, e2::postrender>(cell::shaders::disabled, e2::form::state::disabled)
                ->plugin<pro::notes>()
                ->setpad({ 0, 0, 0, 0 }, { 0, 0, -tall, 0 })
                ->invoke([&](auto& boss)
                {
                    boss.on(tier::mouserelease, input::key::LeftDoubleClick, [&](hids& gear)
                    {
                        if (gear.meta(hids::anyAlt)) // Pull window.
                        {
                            window.base::riseup(tier::preview, e2::form::layout::expose);
                            auto viewport = gear.owner.base::signal(tier::request, e2::form::prop::viewport);
                            window.base::signal(tier::preview, e2::form::layout::appear, viewport.center()); // Pull window.
                            if (window.hidden) // Restore if minimized.
                            {
                                window.base::signal(tier::preview, e2::form::size::minimize, gear);
                            }
                            else pro::focus::set(window.This(), gear.id, solo::on);
                        }
                        else // Jump to window.
                        {
                            gear.owner.base::signal(tier::release, e2::form::layout::jumpto, window);
                        }
                        gear.dismiss();
                    });
                    boss.on(tier::mouserelease, input::key::LeftClick, [&](hids& gear)
                    {
                        if (gear.meta(hids::anyCtrl)) // Toggle group focus.
                        {
                            if (pro::focus::test(window, gear))
                            {
                                pro::focus::off(window.This(), gear.id); // Remove focus if focused.
                            }
                            else // Expose and set group focus.
                            {
                                window.base::riseup(tier::preview, e2::form::layout::expose);
                                if (window.hidden) // Restore if minimized.
                                {
                                    window.base::signal(tier::preview, e2::form::size::minimize, gear);
                                }
                                pro::focus::set(window.This(), gear.id, solo::off);
                            }
                            gear.dismiss(true); // Suppress double click.
                        }
                        else if (gear.meta(hids::anyAlt)) // Skip it and wait for Alt+Dblclick.
                        {
                            gear.dismiss();
                        }
                        else // Set unique focus.
                        {
                            boss.base::signal(tier::release, desk::events::ui::activate, gear);
                            gear.dismiss();
                        }
                    });
                    boss.LISTEN(tier::release, desk::events::ui::activate, gear) // Set unique focus.
                    {
                        window.base::riseup(tier::preview, e2::form::layout::expose);
                        if (window.hidden) // Restore if minimized.
                        {
                            window.base::signal(tier::preview, e2::form::size::minimize, gear);
                        }
                        else
                        {
                            pro::focus::set(window.This(), gear.id, solo::on);
                        }
                    };
                    boss.on(tier::mouserelease, input::key::RightClick, [&](hids& /*gear*/)
                    {
                        // Reserved for context menu.
                    });
                    boss.LISTEN(tier::release, e2::form::state::mouse, hovered)
                    {
                        window.base::signal(tier::release, e2::form::state::highlight, hovered);
                    };
                });
            auto app_label = item_area.attach(slot::_1, ui::item::ctor(current_title))
                ->active()
                //todo taskbar keybd navigation
                ->template plugin<pro::focus>(pro::focus::mode::focused, true, faux, weight_app_label)
                ->template plugin<pro::keybd>()
                ->shader(c3, e2::form::state::focus::count)
                ->setpad({ tall + 1, 0, tall, tall })
                ->template plugin<pro::notes>(skin::globals().NsTaskbarAppsApp_tooltip)
                ->flexible()
                ->drawdots()
                ->shader(cF, e2::form::state::focus::count, window_ptr)
                ->invoke([&](auto& boss)
                {
                    new_app.LISTEN(tier::release, desk::events::apps::title, app_title, boss.sensors)
                    {
                        boss.set(app_title);
                    };
                });
            auto app_close = item_area.attach(slot::_2, ui::item::ctor("×"))
                ->active()
                //todo taskbar keybd navigation
                ->template plugin<pro::focus>(pro::focus::mode::focused, true, faux, weight_ui_button)
                ->template plugin<pro::keybd>()
                ->shader(c1, e2::form::state::focus::count)
                ->shader(c1, e2::form::state::hover)
                ->setpad({ 2, 2, tall, tall })
                ->template plugin<pro::notes>(skin::globals().NsTaskbarAppsClose_tooltip)
                ->invoke([&](auto& boss)
                {
                    boss.base::hidden = true;
                    auto& reveal_button = boss.base::field([&](bool visible)
                    {
                        //auto unfolded = boss.base::riseup(tier::request, desk::events::ui::toggle);
                        //auto hidden = !unfolded || !hover;
                        //auto folded = item_area_inst.base::size().x <= boss.base::size().x * 2;
                        //auto hidden = folded || !hover;
                        auto hidden = !visible;
                        if (boss.base::hidden != hidden)
                        {
                            boss.base::hidden = hidden;
                            boss.base::reflow();
                        }
                    });
                    item_area.LISTEN(tier::release, e2::form::state::mouse, hovered)
                    {
                        reveal_button(hovered);
                    };
                    boss.LISTEN(tier::release, e2::form::state::focus::count, count)
                    {
                        reveal_button(count);
                    };
                    item_area.LISTEN(tier::release, e2::form::upon::vtree::attached, app_list_block, boss.sensors)
                    {
                        app_list_block->LISTEN(tier::release, desk::events::quit, fast, boss.sensors) // Close all apps in a block.
                        {
                            window.base::enqueue([&](auto& /*boss*/) // Enqueue in order to pass focus one by one.
                            {
                                window.base::signal(tier::anycast, e2::form::proceed::quit::one, fast); // Show closing process.
                            });
                        };
                    };
                    boss.on(tier::mouserelease, input::key::LeftClick, [&](hids& gear)
                    {
                        boss.base::signal(tier::release, desk::events::ui::activate, gear);
                        gear.dismiss(true);
                    });
                    boss.LISTEN(tier::release, desk::events::ui::activate, gear)
                    {
                        window.base::signal(tier::anycast, e2::form::proceed::quit::one, faux); // Show closing process.
                    };
                });
            return item_area_ptr;
        };
        auto app_group_template = [](auto menumodel_item_ptr)
        {
            auto tall = si32{ skin::globals().menuwide };
            auto inactive_color  = skin::globals().inactive;
            auto selected_color  = skin::globals().selected;
            auto danger_color    = skin::globals().danger;
            auto highlight_color = cell{ skin::globals().winfocus };
            auto c3 = highlight_color;
            auto c1 = danger_color;
            auto c9 = selected_color;
            auto cA = inactive_color;
            auto def_note = skin::globals().NsTaskbarApps_deftooltip;
            auto& menuid_prop = menumodel_item_ptr->base::property("window.menuid");
            auto& appcfg_prop = menumodel_item_ptr->base::template property<desk::spec>("window.appcfg"); //todo Apple clang requires templtate
            auto& menumodel_item = *menumodel_item_ptr;
            auto inst_id = menuid_prop;
            auto& conf = appcfg_prop;
            auto& obj_desc = conf.label;
            auto& obj_note = conf.tooltip;
            auto menuitem_ptr = ui::sptr{};
            if (conf.splitter)
            {
                menuitem_ptr = ui::item::ctor(obj_desc)
                    ->active() // Set active to enable tooltips.
                    ->flexible()
                    ->accented()
                    ->colors(cA)
                    ->setpad({ 0, 0, tall, tall }, { 0, 0, -tall, 0 })
                    ->template plugin<pro::notes>(obj_note);
            }
            else
            {
                auto head_fork_ptr = ui::fork::ctor(axis::X, 0, 1, 0);
                auto block_ptr = ui::list::ctor()
                    ->shader(cell::shaders::xlight, e2::form::state::hover, head_fork_ptr)
                    ->setpad({ 0, 0, 0, 0 }, { 0, 0, -tall, 0 });
                block_ptr->attach(head_fork_ptr);
                auto head_ptr = head_fork_ptr->attach(slot::_1, ui::item::ctor(obj_desc))
                    ->flexible()
                    ->setpad({ 0, 0, tall, tall })
                    ->active()
                    //todo taskbar keybd navigation
                    ->template plugin<pro::focus>(pro::focus::mode::focused, true, faux, weight_app_group)
                    ->template plugin<pro::keybd>()
                    ->shader(c3, e2::form::state::focus::count)
                    ->template plugin<pro::notes>(obj_note.empty() ? def_note : obj_note)
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
                                    if (group_focus) boss.base::riseup(tier::release, desk::events::ui::focus::set, gear);
                                    else             boss.base::riseup(tier::release, desk::events::ui::focus::off, gear);
                                }
                                gear.dismiss(true);
                                return;
                            }
                            boss.base::signal(tier::release, desk::events::ui::activate, gear);
                            gear.dismiss(true);
                        });
                        boss.LISTEN(tier::release, desk::events::ui::activate, gear, -, (inst_id))
                        {
                            boss.base::signal(tier::anycast, desk::events::ui::selected, inst_id);
                            static auto offset = dot_00; // static: Share initial offset between all instances.
                            auto current_viewport = gear.owner.base::signal(tier::request, e2::form::prop::viewport);
                            offset = (offset + dot_21 * 2) % std::max(dot_11, current_viewport.size * 7 / 32);
                            gear.slot.coor = current_viewport.coor + offset + current_viewport.size * 1 / 32 + dot_11;
                            gear.slot.size = current_viewport.size * 3 / 4;
                            gear.slot_forced = faux;
                            menumodel_item.base::signal(tier::request, e2::form::proceed::createby, gear);
                        };
                    });
                auto& isfolded = conf.folded;
                auto insts_ptr = block_ptr->attach(ui::list::ctor())
                    ->template plugin<pro::focus>()
                    ->setpad({ 0, 0, tall, 0 }, { 0, 0, -tall * 2, 0 });
                auto& insts = *insts_ptr;
                auto bttn_rail_ptr = head_fork_ptr->attach(slot::_2, ui::rail::ctor(axes::X_only, axes::all, axes::none))
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
                auto& bttn_rail = *bttn_rail_ptr;
                auto bttn_fork_ptr = bttn_rail.attach(ui::fork::ctor(axis::X));
                auto& bttn_fork = *bttn_fork_ptr;
                bttn_rail.base::hidden = !menumodel_item.subset.size();
                auto fold_bttn_ptr = bttn_fork.attach(slot::_1, ui::item::ctor(isfolded ? "…" : "<"))
                    ->setpad({ 2, 2, tall, tall })
                    ->active()
                    //todo taskbar keybd navigation
                    ->template plugin<pro::focus>(bttn_rail.base::hidden ? pro::focus::mode::focusable : pro::focus::mode::focused, true, faux, weight_ui_button) // Skip (make it just focusable) this item when moving focus if there are no apps running.
                    ->template plugin<pro::keybd>()
                    ->shader(c3, e2::form::state::focus::count)
                    ->shader(cell::shaders::xlight, e2::form::state::hover)
                    ->template plugin<pro::notes>(skin::globals().NsTaskbarApps_toggletooltip)
                    ->invoke([&](auto& boss)
                    {
                        insts.base::hidden = isfolded;
                        boss.on(tier::mouserelease, input::key::LeftClick, [&](hids& gear)
                        {
                            boss.base::signal(tier::release, desk::events::ui::activate, gear);
                            gear.dismiss(true);
                        });
                        boss.LISTEN(tier::release, desk::events::ui::activate, gear)
                        {
                            isfolded = !isfolded;
                            boss.set(isfolded ? "…" : "<");
                            insts.base::hidden = isfolded;
                            insts.base::reflow();
                        };
                        insts.LISTEN(tier::release, e2::form::state::focus::count, count, boss.sensors)
                        {
                            if (isfolded && count)
                            {
                                isfolded = !isfolded;
                                boss.set(isfolded ? "…" : "<");
                                insts.base::hidden = isfolded;
                                insts.base::reflow();
                            }
                        };
                    });
                auto drop_bttn_ptr = bttn_fork.attach(slot::_2, ui::item::ctor("×"))
                    ->setpad({ 2, 2, tall, tall })
                    ->active()
                    //todo taskbar keybd navigation
                    ->template plugin<pro::focus>(bttn_rail.base::hidden ? pro::focus::mode::focusable : pro::focus::mode::focused, true, faux, weight_ui_button) // Skip (make it just focusable) this item when moving focus if there are no apps running.
                    ->template plugin<pro::keybd>()
                    ->shader(c1, e2::form::state::focus::count)
                    ->shader(c1, e2::form::state::hover)
                    ->template plugin<pro::notes>(skin::globals().NsTaskbarApps_groupclosetooltip)
                    ->invoke([&](auto& boss)
                    {
                        boss.on(tier::mouserelease, input::key::LeftClick, [&](hids& gear)
                        {
                            boss.base::signal(tier::release, desk::events::ui::activate, gear);
                            gear.dismiss(true);
                        });
                        boss.LISTEN(tier::release, desk::events::ui::activate, gear)
                        {
                            insts.base::signal(tier::release, desk::events::quit, faux); // Show closing process.
                        };
                        boss.LISTEN(tier::release, e2::form::state::focus::count, count)
                        {
                            if (count == 0)
                            {
                                boss.base::riseup(tier::preview, e2::form::upon::scroll::to_top::v);
                            }
                        };
                    });
                for (auto& new_appmodel_ptr : menumodel_item.subset)
                {
                    insts.attach(app_template(new_appmodel_ptr));
                }
                auto& block = *block_ptr;
                auto& fold_bttn_focus = fold_bttn_ptr->base::template plugin<pro::focus>(); //todo Apple clang requires templtate
                auto& drop_bttn_focus = drop_bttn_ptr->base::template plugin<pro::focus>(); //
                auto& update_focusability = menumodel_item.base::field([&]
                {
                    if (std::exchange(bttn_rail.base::hidden, !menumodel_item.subset.size()) != bttn_rail.base::hidden)
                    {
                        auto bttn_focusability = bttn_rail.base::hidden ? pro::focus::mode::focusable : pro::focus::mode::focused;
                        fold_bttn_focus.set_mode(bttn_focusability);
                        drop_bttn_focus.set_mode(bttn_focusability);
                    }
                });
                menumodel_item.LISTEN(tier::release, desk::events::apps::created, new_appmodel_ptr, block.sensors)
                {
                    auto running_app_label_ptr = insts.attach(app_template(new_appmodel_ptr));
                    update_focusability();
                    running_app_label_ptr->base::reflow();
                };
                menumodel_item.LISTEN(tier::release, desk::events::apps::removed, new_appmodel_ptr, block.sensors)
                {
                    //todo pass focus to the prev item
                    update_focusability();
                    block.base::enqueue([&](auto& /*boss*/)
                    {
                        block.base::reflow();
                    });
                };
                menuitem_ptr = block_ptr;
            }
            menuitem_ptr->base::property("window.menuid") = menuid_prop;
            return menuitem_ptr;
        };
        auto create_app_list = [](auto& world)
        {
            auto apps_ptr = ui::list::ctor();
            auto& app_model = *world.base::signal(tier::request, desk::events::apps::getmodel);
            for (auto& menumodel_item_ptr : app_model.subset)
            {
                apps_ptr->attach(app_group_template(menumodel_item_ptr));
            }
            apps_ptr->invoke([&](auto& boss)
            {
                boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent)
                {
                    auto current_default = boss.base::riseup(tier::request, e2::data::changed);
                    boss.base::signal(tier::anycast, desk::events::ui::selected, current_default);
                    //todo combine anycasts (update on user disconnect)
                    auto state = boss.base::riseup(tier::request, desk::events::ui::toggle);
                    boss.base::riseup(tier::anycast, desk::events::ui::recalc, state);
                };
                app_model.LISTEN(tier::release, desk::events::apps::enlist, menumodel_item_ptr, boss.sensors)
                {
                    boss.attach(app_group_template(menumodel_item_ptr));
                };
                app_model.LISTEN(tier::release, desk::events::apps::delist, menumodel_item_ptr, boss.sensors)
                {
                    //todo pass focus to the prev item
                    auto& menuid_prop = menumodel_item_ptr->base::property("window.menuid");
                    for (auto menuitem_ptr : boss.subset)
                    {
                        if (menuitem_ptr->base::property("window.menuid") == menuid_prop)
                        {
                            boss.remove(menuitem_ptr);
                            boss.base::enqueue([&](auto& /*boss*/)
                            {
                                boss.base::reflow();
                            });
                            break;
                        }
                    }
                };
            });
            return apps_ptr;
        };

        auto build = [](eccc usrcfg, settings& config)
        {
            auto tall = si32{ skin::globals().menuwide };
            auto inactive_color  = skin::globals().inactive;
            auto danger_color    = skin::globals().danger;
            auto highlight_color = cell{ skin::globals().winfocus };
            auto c8 = cell{}.bgc(argb::active_transparent).fgc(highlight_color.bgc());
            auto c3 = highlight_color;
            auto cA = inactive_color;
            auto c1 = danger_color;

            auto menu_bg_color = config.settings::take("/config/desktop/taskbar/colors/bground", cell{}.fgc(whitedk).bgc(0x60202020));
            auto menu_min_conf = config.settings::take("/config/desktop/taskbar/width/folded",   si32{ 5  });
            auto menu_max_conf = config.settings::take("/config/desktop/taskbar/width/expanded", si32{ 40 });
            auto bttn_min_size = twod{ 40, 1 + tall * 2 };
            auto bttn_max_size = twod{ -1, 1 + tall * 2 };

            auto desklayout_ptr = ui::fork::ctor(axis::Y, 0, 0, 1);
            auto panel_top = config.settings::take("/config/desktop/panel/height", 1);
            auto panel_env = config.settings::take("/config/desktop/panel/env", ""s);
            auto panel_cwd = config.settings::take("/config/desktop/panel/cwd", ""s);
            auto panel_cmd = config.settings::take("/config/desktop/panel/cmd", ""s);
            auto panel = desklayout_ptr->attach(slot::_1, ui::cake::ctor());
            if (panel_cmd.size())
            {
                auto panel_cfg = eccc{ .env = panel_env,
                                       .cwd = panel_cwd,
                                       .cmd = panel_cmd };
                panel_top = std::max(1, panel_top);
                panel->limits({ -1, panel_top }, { -1, panel_top })
                     ->attach(app::shared::builder(app::vtty::id)(panel_cfg, config));
            }

            auto user_info = utf::split(usrcfg.cfg, ";");
            auto& user_id__view = user_info[0];
            auto& username_view = user_info[1];
            log("%%User %name% connected", prompt::desk, username_view);

            auto my_id = id_t{};
            if (auto value = utf::to_int(user_id__view)) my_id = value.value();
            else
            {
                log(prompt::desk, "Bad user ID=", user_id__view);
                return desklayout_ptr;
            }

            auto user_template = [my_id](auto& data_src, auto const& utf8)
            {
                auto tall = si32{ skin::globals().menuwide };
                auto active_color    = skin::globals().active;
                auto highlight_color = cell{ skin::globals().winfocus };
                auto c3 = highlight_color;
                auto cE = active_color;
                auto user = ui::item::ctor(escx(" &").nil().add(" ").wrp(wrap::off)
                        .fgx(data_src->id == my_id ? cE.fgc() : argb{}).add(utf8).nil())
                    ->flexible()
                    ->setpad({ 1, 0, tall, tall }, { 0, 0, -tall, 0 })
                    ->active()
                    //todo taskbar keybd navigation
                    ->template plugin<pro::focus>(pro::focus::mode::focused, true, faux, weight_app_label)
                    ->template plugin<pro::keybd>()
                    ->shader(c3, e2::form::state::focus::count)
                    ->shader(cell::shaders::xlight, e2::form::state::hover)
                    ->template plugin<pro::notes>(skin::globals().NsUser_tooltip);
                return user;
            };
            auto user_list_template = [user_template](auto& /*data_src*/, auto& usr_list)
            {
                auto tall = si32{ skin::globals().menuwide };
                auto users = ui::list::ctor()
                    ->setpad({ 0, 0, tall, 0 }, { 0, 0, -tall, 0 })
                    ->attach_collection(e2::form::prop::name, *usr_list, user_template);
                return users;
            };

            auto& size_config = desklayout_ptr->base::field(std::tuple{ menu_max_conf, menu_min_conf, faux });
            //todo Apple Clang don't get it.
            //auto& [menu_max_size, menu_min_size, active] = size_config;
            auto& menu_max_size = std::get<0>(size_config);
            auto& menu_min_size = std::get<1>(size_config);
            auto& active        = std::get<2>(size_config);

            auto world_ptr = desklayout_ptr->base::signal(tier::general, e2::config::creator);
            if (!world_ptr) return desklayout_ptr;
            auto& world = *world_ptr;
            desklayout_ptr->invoke([&](auto& boss) mutable
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
            auto ground = desklayout_ptr->attach(slot::_2, ui::cake::ctor());
            auto ver_label = ground->attach(ui::item::ctor(utf::concat(app::shared::version)))
                ->active(cell{}.fgc(whitedk).bgc(argb::active_transparent))
                ->shader(c8, e2::form::state::hover)
                ->limits({}, { -1, 1 })
                ->alignment({ snap::tail, snap::tail })
                ->template plugin<pro::notes>(skin::globals().NsInfo_tooltip)
                ->invoke([&](auto& boss)
                {
                    auto infospec = spec{ .menuid = "vtm_info_page", .hidden = true, .label = skin::globals().NsInfo_label, .title = skin::globals().NsInfo_title, .type = "info" };
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
            auto taskbar_grips_ptr = taskbar_viewport->attach(slot::_1, ui::fork::ctor(axis::X));
            auto& taskbar_grips = *taskbar_grips_ptr;
            auto& change_taskbar_width = taskbar_grips.base::field([&](si32 delta)
            {
                taskbar_grips.base::min_sz.x = std::max(1, taskbar_grips.base::min_sz.x + delta);
                taskbar_grips.base::max_sz.x = taskbar_grips.base::min_sz.x;
                active ? menu_max_size = taskbar_grips.base::min_sz.x
                       : menu_min_size = taskbar_grips.base::min_sz.x;
                taskbar_grips.base::reflow();
            });
            taskbar_grips_ptr->limits({ menu_min_size, -1 }, { menu_min_size, -1 })
                //todo taskbar keybd navigation
                ->plugin<pro::focus>()
                ->plugin<pro::keybd>()
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
                    boss.LISTEN(tier::release, e2::form::state::focus::count, count)
                    {
                        auto is_active = !!count;
                        boss.base::riseup(tier::preview, desk::events::ui::toggle, is_active);
                    };
                    auto& timer = boss.base::template plugin<pro::timer>(); //todo Apple clang requires templtate
                    boss.LISTEN(tier::release, e2::form::state::mouse, hovered)
                    {
                        if (hovered)
                        {
                            timer.pacify(faux);
                        }
                        else
                        {
                            auto count = boss.base::signal(tier::request, e2::form::state::focus::count);
                            if (count == 0) // Only when mouse leaving and unfocused.
                            {
                                auto toggle = [&](auto state)
                                {
                                    boss.base::riseup(tier::preview, desk::events::ui::toggle, state);
                                    return faux; // One shot call.
                                };
                                timer.actify(faux, skin::globals().menu_timeout, toggle);
                            }
                        }
                    };
                });
            auto grips = taskbar_grips.attach(slot::_2, ui::mock::ctor())
                ->limits({ 1, -1 }, { 1, -1 })
                ->template plugin<pro::notes>(skin::globals().NsTaskbarGrips_tooltip)
                ->active()
                //todo taskbar keybd navigation
                //->template plugin<pro::focus>(pro::focus::mode::focusable) //todo revise: no need to focus taskbar resize grip
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
                        if (auto delta = (twod{ gear.coord } - twod{ drag_origin })[axis::X])
                        {
                            change_taskbar_width(delta);
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
            auto taskbar_park = taskbar_grips.attach(slot::_1, ui::cake::ctor());
            auto taskbar = taskbar_park->attach(ui::fork::ctor(axis::Y)->alignment({ snap::head, snap::head }, { snap::head, snap::tail }));
            auto apps_users = taskbar->attach(slot::_1, ui::fork::ctor(axis::Y, 0, 100))
                ->setpad({}, { 0, 0, 0, -tall }); // To place above Disconnect button.
            auto applist_area = apps_users->attach(slot::_1, ui::cake::ctor());
            auto tasks_scrl_ptr = applist_area->attach(ui::rail::ctor(axes::Y_only))
                ->plugin<pro::notes>(skin::globals().NsTaskbar_tooltip)
                ->active()
                ->invoke([&](auto& boss)
                {
                    boss.LISTEN(tier::anycast, e2::form::upon::started, root_ptr)
                    {
                        auto app_list_ptr = create_app_list(world);
                        boss.attach(app_list_ptr);
                        auto& app_list = *app_list_ptr;
                        boss.base::enqueue([&](auto& /*boss*/) // Set default focus.
                        {
                            auto current_default = boss.base::riseup(tier::request, e2::data::changed);
                            for (auto& menuitem_ptr : app_list.subset) // Set default focus to the default menu item.
                            {
                                auto& item_menuid = menuitem_ptr->base::property("window.menuid");
                                if (item_menuid == current_default && menuitem_ptr->subset.size())
                                {
                                    auto& head_fork_ptr = menuitem_ptr->subset.front();
                                    if (head_fork_ptr->subset.size()) // Focus app group label.
                                    {
                                        auto& head_ptr = head_fork_ptr->subset.front();
                                        pro::focus::set(head_ptr, id_t{}, solo::on, true);
                                    }
                                    break;
                                }
                            }
                            pro::focus::set(world.This(), id_t{}, solo::on, true); // Then set default focus to the world.
                        });
                    };
                });
            auto& tasks_scrl = *tasks_scrl_ptr;
            auto users_area = apps_users->attach(slot::_2, ui::list::ctor());
            auto label_bttn = users_area->attach(ui::fork::ctor(axis::X))
                                        ->active() // Make it active for tooltip.
                                        ->plugin<pro::notes>(skin::globals().NsUserList_tooltip);
            auto label = label_bttn->attach(slot::_1, ui::item::ctor(os::process::elevated ? skin::globals().NsAdmins_label : skin::globals().NsUsers_label))
                ->flexible()
                ->accented()
                ->colors(cA)
                ->setpad({ 0, 0, tall, tall })
                ->limits({ 5, -1 });
            auto userlist_hidden = true;
            auto bttn_ptr = label_bttn->attach(slot::_2, ui::item::ctor(userlist_hidden ? "…" : "<"))
                ->active()
                //todo taskbar keybd navigation
                ->template plugin<pro::focus>(pro::focus::mode::focused, true, faux, weight_app_group)
                ->template plugin<pro::keybd>()
                ->shader(c3, e2::form::state::focus::count)
                ->shader(cell::shaders::xlight, e2::form::state::hover)
                ->plugin<pro::notes>(skin::globals().NsToggle_tooltip)
                ->setpad({ 2, 2, tall, tall });
            auto& bttn = *bttn_ptr;
            auto userlist_area_ptr = users_area->attach(ui::cake::ctor())
                ->setpad({}, { 0, 0, -tall, 0 }) // To place above the admins/users label.
                ->template plugin<pro::focus>()
                ->invoke([&](auto& boss)
                {
                    boss.base::hidden = userlist_hidden;
                    boss.LISTEN(tier::anycast, e2::form::upon::started, root_ptr, -, (user_list_template))
                    {
                        auto world_ptr = world.This();
                        auto users = boss.attach_element(desk::events::usrs, world_ptr, user_list_template);
                    };
                    boss.LISTEN(tier::release, e2::form::state::focus::count, count)
                    {
                        auto& hidden = boss.base::hidden;
                        if (hidden && count)
                        {
                            hidden = !hidden;
                            bttn.set(hidden ? "…" : "<");
                            boss.base::reflow();
                        }
                    };
                });
            bttn_ptr->invoke([&](auto& boss)
            {
                auto& userlist_area = *userlist_area_ptr;
                boss.on(tier::mouserelease, input::key::LeftClick, [&](hids& gear)
                {
                    boss.base::signal(tier::release, desk::events::ui::activate, gear);
                    gear.dismiss(true);
                });
                boss.LISTEN(tier::release, desk::events::ui::activate, gear)
                {
                    auto& hidden = userlist_area.base::hidden;
                    hidden = !hidden;
                    bttn.set(hidden ? "…" : "<");
                    userlist_area.base::reflow();
                };
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
            auto disconnect_park_ptr = bttns->attach(slot::_1, ui::cake::ctor())
                ->active()
                //todo taskbar keybd navigation
                ->template plugin<pro::focus>(pro::focus::mode::focused, true, faux, weight_app_group)
                ->template plugin<pro::keybd>()
                ->shader(c3, e2::form::state::focus::count)
                ->shader(cell::shaders::xlight, e2::form::state::hover)
                ->plugin<pro::notes>(skin::globals().NsDisconnect_tooltip)
                ->invoke([&, name = text{ username_view }](auto& boss)
                {
                    boss.on(tier::mouserelease, input::key::LeftClick, [&](hids& gear)
                    {
                        boss.base::signal(tier::release, desk::events::ui::activate, gear);
                        gear.dismiss(true);
                    });
                    boss.LISTEN(tier::release, desk::events::ui::activate, gear, -, (name))
                    {
                        log("%%User %name% disconnected", prompt::desk, name);
                        gear.owner.base::signal(tier::preview, e2::conio::quit);
                    };
                });
            auto& disconnect_park = *disconnect_park_ptr;
            auto disconnect = disconnect_park.attach(ui::item::ctor(skin::globals().NsDisconnect_label))
                ->setpad({ 1 + tall, 1 + tall, tall, tall })
                ->alignment({ snap::head, snap::center });
            auto shutdown_park = bttns->attach(slot::_2, ui::cake::ctor())
                ->active()
                //todo taskbar keybd navigation
                ->template plugin<pro::focus>(pro::focus::mode::focused, true, faux, weight_ui_button)
                ->template plugin<pro::keybd>()
                ->shader(c1, e2::form::state::focus::count)
                ->shader(c1, e2::form::state::hover)
                ->plugin<pro::notes>(skin::globals().NsShutdown_tooltip)
                ->invoke([&](auto& boss)
                {
                    boss.on(tier::mouserelease, input::key::LeftClick, [&](hids& gear)
                    {
                        boss.base::signal(tier::release, desk::events::ui::activate, gear);
                        gear.dismiss(true);
                    });
                    boss.LISTEN(tier::release, desk::events::ui::activate, gear)
                    {
                        boss.base::signal(tier::general, e2::shutdown, utf::concat(prompt::desk, "Server shutdown"));
                    };
                });
            auto shutdown = shutdown_park->attach(ui::item::ctor(skin::globals().NsShutdown_label))
                ->setpad({ 1 + tall, 1 + tall, tall, tall })
                ->alignment({ snap::tail, snap::center });

            taskbar_grips.invoke([&](auto& boss)
            {
                auto& luafx = boss.bell::indexer.luafx;
                auto& focus = boss.base::template plugin<pro::focus>(); //todo Apple clang requires templtate
                auto& bindings = boss.base::template property<input::bindings::vector>("taskbar.bindings"); //todo Apple clang requires template
                auto applet_context = config.settings::push_context("/config/events/taskbar/");
                auto script_list = config.settings::take_ptr_list_for_name("script");
                bindings = input::bindings::load(config, script_list);
                input::bindings::keybind(boss, bindings);
                boss.base::add_methods(basename::taskbar,
                {
                    { "FocusNextItem",      [&] // (si32 n, si32 min_w, si32 max_w = si32max)
                                            {
                                                auto& gear = luafx.get_gear();
                                                auto count = luafx.get_args_or(1, si32{ 1 });
                                                auto min_w = luafx.get_args_or(2, si32{ 0 });
                                                auto max_w = luafx.get_args_or(3, si32max);
                                                focus.for_first_focused_leaf(gear, [&](auto& focused_item)
                                                {
                                                    auto& item_focus = focused_item.base::template plugin<pro::focus>(); //todo Apple clang requires templtate
                                                    item_focus.focus_next(gear, count, min_w, max_w);
                                                });
                                                gear.set_handled();
                                                luafx.set_return();
                                            }},
                    { "FocusTop",           [&]
                                            {
                                                auto& gear = luafx.get_gear();
                                                tasks_scrl.base::riseup(tier::preview, e2::form::upon::scroll::to_top::v);
                                                if (tasks_scrl.subset.size())
                                                if (auto app_list_ptr = tasks_scrl.subset.front())
                                                {
                                                    auto& app_list = *app_list_ptr;
                                                    for (auto& menuitem_ptr : app_list.subset) // Select the first focusable item from the app list.
                                                    {
                                                        if (menuitem_ptr && menuitem_ptr->subset.size())
                                                        {
                                                            auto& head_fork_ptr = menuitem_ptr->subset.front();
                                                            if (head_fork_ptr && head_fork_ptr->subset.size()) // Focus app group label.
                                                            {
                                                                auto& head_ptr = head_fork_ptr->subset.front();
                                                                if (head_ptr && head_ptr->base::has_plugin<pro::focus>())
                                                                {
                                                                    pro::focus::set(head_ptr, gear.id, solo::on);
                                                                    break;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                                gear.set_handled();
                                                luafx.set_return();
                                            }},
                    { "FocusEnd",           [&]
                                            {
                                                auto& gear = luafx.get_gear();
                                                tasks_scrl.base::riseup(tier::preview, e2::form::upon::scroll::to_end::v);
                                                pro::focus::set(disconnect_park.This(), gear.id, solo::on);
                                                gear.set_handled();
                                                luafx.set_return();
                                            }},
                    { "ChangeWidthByStep",  [&] // (-1/1)
                                            {
                                                auto& gear = luafx.get_gear();
                                                auto delta = luafx.get_args_or(1, si32{ 1 });
                                                change_taskbar_width(delta);
                                                gear.set_handled();
                                                luafx.set_return();
                                            }},
                    { "ActivateItem",       [&]
                                            {
                                                auto& gear = luafx.get_gear();
                                                focus.for_first_focused_leaf(gear, [&](auto& focused_item)
                                                {
                                                    focused_item.base::riseup(tier::release, desk::events::ui::activate, gear);
                                                });
                                                gear.set_handled();
                                                luafx.set_return();
                                            }},
                    { "GetHeight",          [&]
                                            {
                                                auto& gear = luafx.get_gear();
                                                auto viewport = gear.owner.base::signal(tier::request, e2::form::prop::viewport);
                                                auto h1 = std::max(viewport.size.y, 1);     // Taskbar height.
                                                auto h2 = skin::globals().menuwide ? 2 : 1; // Taskbar line height.
                                                luafx.set_return(h1, h2);
                                            }},
                    { "GetFocusedWeight",   [&]
                                            {
                                                auto& gear = luafx.get_gear();
                                                auto focused_weight = 0;
                                                focus.for_first_focused_leaf(gear, [&](auto& focused_item)
                                                {
                                                    focused_weight = focused_item.base::template plugin<pro::focus>().get_weight(); //todo Apple clang requires templtate
                                                });
                                                luafx.set_return(focused_weight);
                                            }},
                });
            });
            return desklayout_ptr;
        };
    }

    app::shared::initialize builder{ app::desk::id, build };
}