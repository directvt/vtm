// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_APP_DESK_HPP
#define NETXS_APP_DESK_HPP

namespace netxs::events::userland
{
    struct desk
    {
        EVENTPACK( desk, netxs::events::userland::root::custom )
        {
            GROUP_XS( ui, text ),

            SUBSET_XS( ui )
            {
                EVENT_XS( selected, text ),
            };
        };
    };
}

// desk: Sidebar menu.
namespace netxs::app::desk
{
    using events = ::netxs::events::userland::desk;
    
    namespace
    {
        auto app_template = [](auto& data_src, auto const& utf8)
        {
            const static auto c4 = app::shared::c4;
            const static auto x4 = app::shared::x4;
            const static auto c5 = app::shared::c5;
            const static auto x5 = app::shared::x5;

            auto item_area = ui::pads::ctor(dent{ 1,0,1,0 }, dent{ 0,0,0,1 })
                    ->plugin<pro::fader>(x4, c4, 0ms)//150ms)
                    ->plugin<pro::notes>(" Running instance:                          \n"
                                         "   Left click to go to running instance     \n"
                                         "   Right click to pull the running instance ")
                    ->invoke([&](auto& boss)
                    {
                        auto data_src_shadow = ptr::shadow(data_src);
                        boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::click::left, gear)
                        {
                            if (auto data_src = data_src_shadow.lock())
                            {
                                auto& inst = *data_src;
                                inst.SIGNAL(tier::preview, e2::form::layout::expose, inst);
                                auto& area = inst.base::area();
                                auto center = area.coor + (area.size / 2);
                                bell::getref(gear.id)->SIGNAL(tier::release, e2::form::layout::shift, center);  // Goto to the window.
                                gear.pass_kb_focus(inst);
                                gear.dismiss();
                            }
                        };
                        boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::click::right, gear)
                        {
                            if (auto data_src = data_src_shadow.lock())
                            {
                                auto& inst = *data_src;
                                inst.SIGNAL(tier::preview, e2::form::layout::expose, inst);
                                auto& area = gear.area();
                                auto center = area.coor + (area.size / 2);
                                inst.SIGNAL(tier::preview, e2::form::layout::appear, center); // Pull window.
                                gear.pass_kb_focus(inst);
                                gear.dismiss();
                            }
                        };
                        boss.SUBMIT_BYVAL(tier::release, e2::form::state::mouse, hits)
                        {
                            if (auto data_src = data_src_shadow.lock())
                            {
                                data_src->SIGNAL(tier::release, e2::form::highlight::any, !!hits);
                            }
                        };
                    });
                auto label_area = item_area->attach(ui::fork::ctor());
                    auto mark_app = label_area->attach(slot::_1, ui::fork::ctor());
                        auto mark = mark_app->attach(slot::_1, ui::pads::ctor(dent{ 2,1,0,0 }, dent{ 0,0,0,0 }))
                                            ->attach(ui::item::ctor(ansi::fgc4(0xFF00ff00).add("‣"), faux));
                        auto app_label = mark_app->attach(slot::_2,
                                    ui::item::ctor(ansi::fgc(whitelt).add(utf8).mgl(0).wrp(wrap::off).jet(bias::left), true, true));
                    auto app_close_area = label_area->attach(slot::_2, ui::pads::ctor(dent{ 0,0,0,0 }, dent{ 0,0,1,1 }))
                                                    ->template plugin<pro::fader>(x5, c5, 150ms)
                                                    ->invoke([&](auto& boss)
                                                    {
                                                        auto data_src_shadow = ptr::shadow(data_src);
                                                        boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::click::left, gear)
                                                        {
                                                            if (auto data_src = data_src_shadow.lock())
                                                            {
                                                                data_src->SIGNAL(tier::release, e2::form::proceed::detach, data_src);
                                                                gear.dismiss();
                                                            }
                                                        };
                                                    });
                        auto app_close = app_close_area->attach(ui::item::ctor("  ×  ", faux));
            return item_area;
        };
        auto apps_template = [](auto& data_src, auto& apps_map)
        {
            const static auto c3 = app::shared::c3;
            const static auto x3 = app::shared::x3;

            auto apps = ui::list::ctor()
                ->invoke([&](auto& boss)
                {
                    boss.SUBMIT(tier::release, e2::form::upon::vtree::attached, parent)
                    {
                        auto current_default = decltype(e2::data::changed)::type{};
                        boss.template riseup<tier::request>(e2::data::changed, current_default); //todo "template" required by gcc (ubuntu 18.04)
                        boss.SIGNAL(tier::anycast, events::ui::selected, current_default);
                    };
                });

            for (auto const& [class_id, stat_inst_ptr_list] : *apps_map)
            {
                auto& [state, inst_ptr_list] = stat_inst_ptr_list;
                auto inst_id  = class_id;
                auto obj_desc = app::shared::objs_config[class_id].label;
                auto item_area = apps->attach(ui::pads::ctor(dent{ 0,0,0,1 }, dent{ 0,0,1,0 }))
                                     ->template plugin<pro::fader>(x3, c3, 0ms)
                                     ->template plugin<pro::notes>(" Menu item:                           \n"
                                                                   "   Left click to start a new instance \n"
                                                                   "   Right click to set default app     ")
                                     ->invoke([&](auto& boss)
                                     {
                                         boss.mouse.take_all_events(faux);
                                         auto boss_shadow = ptr::shadow(boss.This());
                                         auto data_src_shadow = ptr::shadow(data_src);
                                         boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::click::right, gear)
                                         {
                                             if (auto boss = boss_shadow.lock())
                                             {
                                                 boss->SIGNAL(tier::anycast, events::ui::selected, inst_id);
                                             }
                                         };
                                         boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::click::left, gear)
                                         {
                                             if (auto boss = boss_shadow.lock())
                                             {
                                                 boss->SIGNAL(tier::anycast, events::ui::selected, inst_id);
                                             }
                                             auto world_ptr = decltype(e2::config::whereami)::type{};
                                             SIGNAL_GLOBAL(e2::config::whereami, world_ptr);
                                             if (world_ptr)
                                             {
                                                 static si32 random = 0;
                                                 random = (random + 2) % 10;
                                                 auto offset = twod{ random * 2, random };
                                                 auto viewport = gear.area();
                                                 gear.slot.coor = viewport.coor + viewport.size / 8 + offset;
                                                 gear.slot.size = viewport.size * 3 / 4;
                                                 world_ptr->SIGNAL(tier::release, e2::form::proceed::createby, gear);
                                                 gear.dismiss();
                                             }
                                         };
                                         //boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::scroll::any, gear)
                                         //{
                                         //   if (auto boss = boss_shadow.lock())
                                         //   if (auto data_src = data_src_shadow.lock())
                                         //   {
                                         //       sptr<registry_t> registry_ptr;
                                         //       data_src->SIGNAL(tier::request, e2::bindings::list::apps, registry_ptr);
                                         //       auto& app_list = (*registry_ptr)[inst_id];
                                         //       if (app_list.size())
                                         //       {
                                         //           auto deed = boss->bell::protos<tier::release>();
                                         //           if (deed == hids::events::mouse::scroll::down.id) // Rotate list forward.
                                         //           {
                                         //               app_list.push_back(app_list.front());
                                         //               app_list.pop_front();
                                         //           }
                                         //           else // Rotate list backward.
                                         //           {
                                         //               app_list.push_front(app_list.back());
                                         //               app_list.pop_back();
                                         //           }
                                         //           // Expose window.
                                         //           auto& inst = *app_list.back();
                                         //           inst.SIGNAL(tier::preview, e2::form::layout::expose, inst);
                                         //           auto& area = inst.base::area();
                                         //           auto center = area.coor + (area.size / 2);
                                         //           bell::getref(gear.id)->
                                         //           SIGNAL(tier::release, e2::form::layout::shift, center);  // Goto to the window.
                                         //           gear.pass_kb_focus(inst);
                                         //           gear.dismiss();
                                         //       }
                                         //   }
                                         //};
                                     });
                if (!state) item_area->depend_on_collection(inst_ptr_list); // Remove not pinned apps, like Info.
                auto block = item_area->attach(ui::fork::ctor(axis::Y));
                    auto head_area = block->attach(slot::_1, ui::pads::ctor(dent{ 0,0,0,0 }, dent{ 0,0,1,1 }));
                        auto head = head_area->attach(ui::item::ctor(obj_desc, true))
                            ->invoke([&](auto& boss)
                            {
                                auto boss_shadow = ptr::shadow(boss.This());
                                boss.SUBMIT_BYVAL(tier::anycast, events::ui::selected, data)
                                {
                                    auto selected = inst_id == data;
                                    if (auto boss = boss_shadow.lock())
                                    {
                                        boss->set(ansi::fgc4(selected ? 0xFF00ff00 : 0x00000000).add(obj_desc));
                                        boss->deface();
                                    }
                                };
                            });
                    auto list_pads = block->attach(slot::_2, ui::pads::ctor(dent{ 0,0,0,0 }, dent{ 0,0,0,0 }));
                    auto insts = list_pads->attach(ui::list::ctor())
                                          ->attach_collection(e2::form::prop::ui::header, inst_ptr_list, app_template);
            }
            return apps;
        };

        auto build = [](view v)
        {
            auto lock = netxs::events::sync{}; // Protect access to the world.

            si32 uibar_min_size = 4;
            si32 uibar_full_size = 31;

            auto window = ui::cake::ctor();

            auto my_id = id_t{};

            auto user_info = utf::divide(v, ";");
            if (user_info.size() < 2)
            {
                log("desk: bad window arguments: args=", utf::debase(v));
                return window;
            }
            auto& user_id___view = user_info[0];
            auto& user_name_view = user_info[1];
            log("desk: id: ", user_id___view, ", user name: ", user_name_view);

            if (auto value = utf::to_int(user_id___view)) my_id = value.value();
            else return window;

            if (auto client = bell::getref(my_id))
            {
                // Taskbar Layout.
                auto client_shadow = ptr::shadow(client);
                auto user_template = [my_id](auto& data_src, auto const& utf8)
                {
                    const static auto c3 = app::shared::c3;
                    const static auto x3 = app::shared::x3;

                    auto item_area = ui::pads::ctor(dent{ 1,0,0,1 }, dent{ 0,0,1,0 })
                                            ->plugin<pro::fader>(x3, c3, 150ms);
                        auto user = item_area->attach(ui::item::ctor(ansi::esc(" &").nil().add(" ")
                                    .fgc4(data_src->id == my_id ? rgba::color256[whitelt] : 0x00).add(utf8), true));
                    return item_area;
                };
                auto branch_template = [user_template](auto& data_src, auto& usr_list)
                {
                    auto users = ui::list::ctor()
                        ->attach_collection(e2::form::prop::name, *usr_list, user_template);
                    return users;
                };

                window->invoke([uibar_full_size, uibar_min_size](auto& boss) mutable
                    {
                        //#ifdef _WIN32
                        //    auto current_default_sptr = std::make_shared<text>(app::shared::objs_lookup["CommandPrompt"]);
                        //    //auto current_default = app::shared::objs_lookup["PowerShell"];
                        //#else
                            auto current_default_sptr = std::make_shared<text>(app::shared::objs_lookup["Term"]);
                        //#endif
                        auto previous_default_sptr = std::make_shared<text>(*current_default_sptr);
                        auto subs_sptr = std::make_shared<subs>();
                        auto shadow = ptr::shadow(boss.This());

                        boss.SUBMIT_BYVAL(tier::release, e2::form::upon::vtree::attached, parent)
                        {
                            parent->SIGNAL(tier::anycast, events::ui::selected, *current_default_sptr);

                            parent->SUBMIT_T(tier::request, e2::data::changed, *subs_sptr, data)
                            {
                                if (current_default_sptr) data = *current_default_sptr;
                            };
                            parent->SUBMIT_T(tier::preview, e2::data::changed, *subs_sptr, data)
                            {
                                if (previous_default_sptr) data = *previous_default_sptr;
                            };
                            parent->SUBMIT_T_BYVAL(tier::release, e2::data::changed, *subs_sptr, data)
                            {
                                if (auto boss = shadow.lock())
                                {
                                    boss->SIGNAL(tier::anycast, events::ui::selected, data);
                                }
                            };
                            parent->SUBMIT_T(tier::anycast, events::ui::selected, *subs_sptr, data)
                            {
                                if (previous_default_sptr && current_default_sptr)
                                {
                                    auto new_default = data;
                                    if (*current_default_sptr != new_default)
                                    {
                                        *previous_default_sptr = *current_default_sptr;
                                        *current_default_sptr = new_default;
                                    }
                                }
                            };
                            parent->SUBMIT_T(tier::release, e2::form::upon::vtree::detached, *subs_sptr, p)
                            {
                                current_default_sptr.reset();
                                previous_default_sptr.reset();
                                subs_sptr.reset();
                            };
                        };
                    });
                    auto taskbar_viewport = window->attach(ui::fork::ctor(axis::X))
                                            ->invoke([](auto& boss)
                                            {
                                                boss.SUBMIT(tier::anycast, e2::form::prop::viewport, viewport)
                                                {
                                                    viewport = boss.base::area();
                                                };
                                            });
                    auto taskbar = taskbar_viewport->attach(slot::_1, ui::fork::ctor(axis::Y))
                                        ->colors(whitedk, 0x60202020)
                                        ->plugin<pro::notes>(" LeftDrag to adjust menu width                   \n"
                                                             " RightDrag or scroll wheel to slide menu up/down ")
                                        ->plugin<pro::limit>(twod{ uibar_min_size,-1 }, twod{ uibar_min_size,-1 })
                                        ->plugin<pro::timer>()
                                        ->plugin<pro::acryl>()
                                        ->plugin<pro::cache>()
                                        ->invoke([&](auto& boss)
                                        {
                                            boss.mouse.template draggable<sysmouse::left>(true);
                                            auto boss_shadow = ptr::shadow(boss.This());
                                            auto size_config = std::make_shared<std::pair<si32, si32>>(uibar_full_size, uibar_min_size);
                                            boss.SUBMIT_BYVAL(tier::release, e2::form::drag::pull::_<sysmouse::left>, gear)
                                            {
                                                if (auto boss_ptr = boss_shadow.lock())
                                                {
                                                    auto& boss = *boss_ptr;
                                                    auto& [uibar_full_size, uibar_min_size] = *size_config;
                                                    auto& limits = boss.template plugins<pro::limit>();
                                                    auto lims = limits.get();
                                                    lims.min.x += gear.delta.get().x;
                                                    lims.max.x = uibar_full_size = lims.min.x;
                                                    limits.set(lims.min, lims.max);
                                                    boss.base::reflow();
                                                }
                                            };
                                            boss.SUBMIT_BYVAL(tier::release, e2::form::state::mouse, active)
                                            {
                                                if (auto boss_ptr = boss_shadow.lock())
                                                {
                                                    auto apply = [=](auto active)
                                                    {
                                                        if (auto boss_ptr = boss_shadow.lock())
                                                        {
                                                            auto& boss = *boss_ptr;
                                                            auto& [uibar_full_size, uibar_min_size] = *size_config;
                                                            auto& limits = boss.template plugins<pro::limit>();
                                                            auto size = active ? uibar_full_size : std::min(uibar_full_size, uibar_min_size);
                                                            auto lims = twod{ size,-1 };
                                                            limits.set(lims, lims);
                                                            boss.base::reflow();
                                                        }
                                                        return faux; // One shot call.
                                                    };
                                                    auto& timer = boss_ptr->template plugins<pro::timer>();
                                                    timer.pacify(faux);
                                                    if (active) apply(true);
                                                    else timer.actify(faux, MENU_TIMEOUT, apply);
                                                }
                                            };
                                            boss.SUBMIT_BYVAL(tier::anycast, e2::form::prop::viewport, viewport)
                                            {
                                                auto& [uibar_full_size, uibar_min_size] = *size_config;
                                                viewport.coor.x += uibar_min_size;
                                                viewport.size.x -= uibar_min_size;
                                            };
                                        });
                        auto apps_users = taskbar->attach(slot::_1, ui::fork::ctor(axis::Y, 0, 100));
                        {
                            const static auto c3 = app::shared::c3;
                            const static auto x3 = app::shared::x3;
                            const static auto c6 = app::shared::c6;
                            const static auto x6 = app::shared::x6;

                            auto world_ptr = decltype(e2::config::whereami)::type{};
                            SIGNAL_GLOBAL(e2::config::whereami, world_ptr);
                            {
                                auto applist_area = apps_users->attach(slot::_1, ui::pads::ctor(dent{ 0,0,1,0 }, dent{}))
                                                              ->attach(ui::cake::ctor());
                                if (world_ptr)
                                {
                                    auto tasks_scrl = applist_area->attach(ui::rail::ctor(axes::Y_ONLY))
                                                                  ->colors(0x00, 0x00); //todo mouse events passthrough
                                    auto apps = tasks_scrl->attach_element(e2::bindings::list::apps, world_ptr, apps_template);
                                }
                            }
                            {
                                auto users_area = apps_users->attach(slot::_2, ui::fork::ctor(axis::Y));
                                auto label_pads = users_area->attach(slot::_1, ui::pads::ctor(dent{ 0,0,1,1 }, dent{ 0,0,0,0 }))
                                                            ->plugin<pro::fader>(x3, c3, 150ms)
                                                            ->plugin<pro::notes>(" List of connected users ");
                                    auto label_bttn = label_pads->attach(ui::fork::ctor());
                                        auto label = label_bttn->attach(slot::_1,
                                            ui::item::ctor(ansi::fgc(whitelt).add("Users"), faux, faux));
                                        auto bttn_area = label_bttn->attach(slot::_2, ui::fork::ctor());
                                            auto bttn_pads = bttn_area->attach(slot::_2, ui::pads::ctor(dent{ 2,2,0,0 }, dent{ 0,0,1,1 }))
                                                                      ->plugin<pro::fader>(x6, c6, 150ms)
                                                                      ->plugin<pro::notes>(" Show/hide users list ");
                                                auto bttn = bttn_pads->attach(ui::item::ctor("<", faux));
                                auto userlist_area = users_area->attach(slot::_2, ui::pads::ctor())
                                                               ->plugin<pro::limit>();
                                    if (world_ptr)
                                    {
                                        auto users = userlist_area->attach_element(e2::bindings::list::users, world_ptr, branch_template);
                                    }
                                bttn_pads->invoke([&](auto& boss)
                                            {
                                                auto userlist_area_shadow = ptr::shadow(userlist_area);
                                                auto bttn_shadow = ptr::shadow(bttn);
                                                auto state_ptr = std::make_shared<bool>(faux);
                                                boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::click::left, gear)
                                                {
                                                    if (auto bttn = bttn_shadow.lock())
                                                    if (auto userlist = userlist_area_shadow.lock())
                                                    {
                                                        auto& state = *state_ptr;
                                                        state = !state;
                                                        bttn->set(state ? ">" : "<");
                                                        auto& limits = userlist->plugins<pro::limit>();
                                                        auto lims = limits.get();
                                                        lims.min.y = lims.max.y = state ? 0 : -1;
                                                        limits.set(lims, true);
                                                        userlist->base::reflow();
                                                    }
                                                };
                                            });
                            }
                        }
                        {
                            const static auto c2 = app::shared::c2;
                            const static auto x2 = app::shared::x2;
                            const static auto c1 = app::shared::c1;
                            const static auto x1 = app::shared::x1;

                            auto bttns_cake = taskbar->attach(slot::_2, ui::cake::ctor());
                            auto bttns_area = bttns_cake->attach(ui::rail::ctor(axes::X_ONLY))
                                                        ->plugin<pro::limit>(twod{ -1, 3 }, twod{ -1, 3 });
                                bttns_cake->attach(app::shared::underlined_hz_scrollbars(bttns_area));
                            auto bttns = bttns_area->attach(ui::fork::ctor(axis::X))
                                                   ->plugin<pro::limit>(twod{ uibar_full_size, 3 }, twod{ -1, 3 });
                                auto disconnect_park = bttns->attach(slot::_1, ui::park::ctor())
                                                            ->plugin<pro::fader>(x2, c2, 150ms)
                                                            ->plugin<pro::notes>(" Leave current session ")
                                                            ->invoke([&](auto& boss)
                                                            {
                                                                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                                                                {
                                                                    if (auto owner = base::getref(gear.id))
                                                                    {
                                                                        owner->SIGNAL(tier::release, e2::conio::quit, "taskbar: logout by button");
                                                                        gear.dismiss();
                                                                    }
                                                                };
                                                            });
                                auto disconnect_area = disconnect_park->attach(snap::head, snap::center, ui::pads::ctor(dent{ 2,3,1,1 }));
                                auto disconnect = disconnect_area->attach(ui::item::ctor("× Disconnect"));
                                auto shutdown_park = bttns->attach(slot::_2, ui::park::ctor())
                                                          ->plugin<pro::fader>(x1, c1, 150ms)
                                                          ->plugin<pro::notes>(" Disconnect all users and shutdown the server ")
                                                          ->invoke([&](auto& boss)
                                                          {
                                                              boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                                                              {
                                                                  SIGNAL_GLOBAL(e2::shutdown, "desk: server shutdown");
                                                              };
                                                          });
                                auto shutdown_area = shutdown_park->attach(snap::tail, snap::center, ui::pads::ctor(dent{ 2,3,1,1 }));
                                auto shutdown = shutdown_area->attach(ui::item::ctor("× Shutdown"));
                        }
            }
            return window;
        };
    }

    app::shared::initialize builder{ "Desk", build };
}

#endif // NETXS_APP_DESK_HPP