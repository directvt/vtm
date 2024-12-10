// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

namespace netxs::app::tile
{
    using backups = std::list<netxs::sptr<ui::veer>>;
}

namespace netxs::events::userland
{
    struct tile
    {
        EVENTPACK( tile, ui::e2::extra::slot4 )
        {
            EVENT_XS( backup, app::tile::backups ),
            EVENT_XS( enlist, ui::sptr           ),
            EVENT_XS( delist, bool               ),
            GROUP_XS( ui    , input::hids        ), // Window manager command pack.

            SUBSET_XS( ui )
            {
                EVENT_XS( create  , input::hids ),
                EVENT_XS( close   , input::hids ),
                EVENT_XS( toggle  , input::hids ), // Toggle window size: maximize/restore.
                EVENT_XS( swap    , input::hids ),
                EVENT_XS( rotate  , input::hids ), // Change nested objects order. See tilimg manager (ui::fork).
                EVENT_XS( equalize, input::hids ),
                EVENT_XS( select  , input::hids ),
                EVENT_XS( title   , input::hids ),
                GROUP_XS( focus   , input::hids ),
                GROUP_XS( split   , input::hids ),

                SUBSET_XS( focus )
                {
                    EVENT_XS( next, input::hids ),
                    EVENT_XS( prev, input::hids ),
                };
                SUBSET_XS( split )
                {
                    EVENT_XS( vt, input::hids ),
                    EVENT_XS( hz, input::hids ),
                };
            };
        };
    };
}

// tile: Tiling window manager.
namespace netxs::app::tile
{
    static constexpr auto id = "tile";
    static constexpr auto name = "Tiling Window Manager";
    static constexpr auto inheritance_limit = 30; // Tiling limits.

    using events = netxs::events::userland::tile;
    using ui::sptr;
    using ui::wptr;

    #define proc_list \
        X(TileFocusPrevPane     ) \
        X(TileFocusNextPane     ) \
        X(TileRunApplicatoin    ) \
        X(TileSelectAllPanes    ) \
        X(TileSplitHorizontally ) \
        X(TileSplitVertically   ) \
        X(TileSplitOrientation  ) \
        X(TileSwapPanes         ) \
        X(TileEqualizeSplitRatio) \
        X(TileSetManagerTitle   ) \
        X(TileClosePane         )

    struct action
    {
        #define X(_proc) static constexpr auto _proc = #_proc;
        proc_list
        #undef X
    };

    #undef proc_list

    // tile: Right-side item list.
    class items
        : public pro::skill
    {
        using skill::boss,
              skill::memo;

        netxs::sptr<ui::list> client;
        si32                  depth;

    public:
        items(base&&) = delete;
        items(base& boss)
            : skill{ boss },
              depth{ 0    }
        {
            client = ui::list::ctor(axis::Y, sort::reverse);
            client->bell::signal(tier::release, e2::form::upon::vtree::attached, boss.This());

            boss.LISTEN(tier::release, e2::area, new_area, memo)
            {
                if (client)
                {
                    auto coor = twod{ new_area.size.x + 2/*resize grip width*/, 0 };
                    client->base::moveto(coor);
                }
            };
            boss.LISTEN(tier::release, events::enlist, object, memo)
            {
                if (!client) return;
                auto label = [](auto data_src_sptr, auto header)
                {
                    auto active_color = skin::color(tone::active);
                    auto focused_color = skin::color(tone::focused);
                    auto cF = focused_color;
                    auto cE = active_color;
                    return ui::item::ctor(header.empty() ? "- no title -" : header)
                        ->setpad({ 1, 1 })
                        ->active(cE)
                        ->shader(cF, e2::form::state::focus::count, data_src_sptr)
                        ->shader(cell::shaders::xlight, e2::form::state::hover)
                        ->invoke([&](auto& boss)
                        {
                            auto data_shadow = ptr::shadow(data_src_sptr);
                            boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent)
                            {
                                parent->resize();
                            };
                            boss.LISTEN(tier::release, e2::form::upon::vtree::detached, parent)
                            {
                                if (parent) parent->resize(); // Rebuild list.
                            };
                            data_src_sptr->LISTEN(tier::release, events::delist, object, boss.tracker)
                            {
                                boss.detach(); // Destroy itself.
                            };
                            boss.LISTEN(tier::release, hids::events::mouse::button::any, gear, boss.tracker, (data_shadow))
                            {
                                if (auto data_ptr = data_shadow.lock())
                                {
                                    auto deed = boss.bell::protos(tier::release);
                                    data_ptr->bell::signal(tier::release, deed, gear);
                                    gear.dismiss();
                                }
                            };
                            boss.LISTEN(tier::release, e2::form::state::mouse, active, boss.tracker, (data_shadow))
                            {
                                if (auto data_ptr = data_shadow.lock())
                                {
                                    data_ptr->bell::signal(tier::release, e2::form::state::highlight, active);
                                }
                            };
                        });
                };
                client->attach_element(e2::form::prop::ui::header, object, label);
            };
            boss.LISTEN(tier::release, e2::render::any, parent_canvas, memo)
            {
                if (depth < 4/*we're not in the tile manager*/ && client)
                {
                    auto context = parent_canvas.bump({ 0, si32max / 2, 0, si32max / 2 });
                    client->render(parent_canvas);
                    parent_canvas.bump(context);
                }
            };
            boss.LISTEN(tier::anycast, e2::form::upon::started, root, memo)
            {
                client->clear();
                depth = 0;
                boss.diveup([&]{ depth++; });
                if constexpr (debugmode) log("%%Start depth %%", prompt::tile, depth);
            };
        }
    };

    namespace
    {
        auto anycasting = [](auto& boss)
        {
            boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent)
            {
                parent->LISTEN(tier::anycast, app::tile::events::ui::any, gear, boss.relyon)
                {
                    auto gear_test = boss.base::riseup(tier::request, e2::form::state::keybd::find, { gear.id, 0 });
                    if (gear_test.second)
                    {
                        if (auto parent = boss.parent())
                        if (auto deed = parent->bell::protos(tier::anycast))
                        {
                            switch (deed)
                            {
                                case app::tile::events::ui::create.id:
                                    boss.base::riseup(tier::request, e2::form::proceed::createby, gear);
                                    break;
                                case app::tile::events::ui::close.id:
                                    boss.base::riseup(tier::preview, e2::form::proceed::quit::one, true);
                                    break;
                                case app::tile::events::ui::toggle.id:
                                    if (boss.base::kind() == base::client) // Only apps can be maximized.
                                    if (gear.countdown > 0)
                                    {
                                        gear.countdown--; // The only one can be maximized if several are selected.
                                        boss.base::riseup(tier::preview, e2::form::size::enlarge::maximize, gear);
                                    }
                                    break;
                                case app::tile::events::ui::swap.id:
                                    if (gear.countdown > 0)
                                    {
                                        boss.base::riseup(tier::release, app::tile::events::ui::swap, gear);
                                    }
                                    break;
                                case app::tile::events::ui::rotate.id:
                                    boss.base::riseup(tier::release, app::tile::events::ui::rotate, gear);
                                    break;
                                case app::tile::events::ui::equalize.id:
                                    boss.base::riseup(tier::release, app::tile::events::ui::equalize, gear);
                                    break;
                                case app::tile::events::ui::split::vt.id:
                                    boss.base::riseup(tier::release, app::tile::events::ui::split::vt, gear);
                                    break;
                                case app::tile::events::ui::split::hz.id:
                                    boss.base::riseup(tier::release, app::tile::events::ui::split::hz, gear);
                                    break;
                            }
                        }
                    }
                };
            };
        };
        auto mouse_subs = [](auto& boss)
        {
            boss.LISTEN(tier::release, hids::events::mouse::button::dblclick::left, gear)
            {
                boss.base::riseup(tier::preview, e2::form::size::enlarge::maximize, gear);
                gear.dismiss();
            };
        };
        auto app_window = [](auto& what)
        {
            return ui::fork::ctor(axis::Y)
                    ->template plugin<pro::title>(what.header, what.footer, true, faux, true)
                    ->template plugin<pro::light>()
                    ->template plugin<pro::focus>()
                    ->limits({ 10,-1 }, { -1,-1 })
                    ->isroot(true)
                    ->active()
                    ->invoke([&](auto& boss)
                    {
                        anycasting(boss);
                        mouse_subs(boss);
                        if (what.applet->size() != dot_00) boss.resize(what.applet->size() + dot_01/*approx title height*/);
                        auto applet_shadow = ptr::shadow(what.applet);
                        boss.LISTEN(tier::release, hids::events::mouse::button::drag::start::any, gear, -, (applet_shadow, menuid = what.menuid))
                        {
                            if (auto applet_ptr = applet_shadow.lock())
                            if (applet_ptr->area().hittest(gear.coord))
                            {
                                auto& applet = *applet_ptr;

                                auto deed = boss.bell::protos(tier::release);
                                if (deed != hids::events::mouse::button::drag::start::left.id
                                 && deed != hids::events::mouse::button::drag::start::leftright.id) return;

                                // Restore if maximized. Parent can be changed after that.
                                boss.bell::signal(tier::release, e2::form::size::restore, e2::form::size::restore.param());

                                // Take current title.
                                auto what = vtm::events::handoff.param({ .menuid = menuid });
                                boss.bell::signal(tier::request, e2::form::prop::ui::header, what.header);
                                boss.bell::signal(tier::request, e2::form::prop::ui::footer, what.footer);
                                if (what.header.empty()) what.header = menuid;

                                // Find creator.
                                auto world_ptr = boss.base::riseup(tier::request, e2::config::creator);

                                // Take coor and detach from the tiling wm.
                                gear.coord -= applet.base::coor(); // Rebase mouse coor.
                                gear.click -= applet.base::coor(); // Rebase mouse click.
                                what.square.size = applet.base::size();
                                applet.global(what.square.coor);
                                what.square.coor = -what.square.coor;
                                what.forced = true;
                                what.applet = applet_ptr;

                                auto gear_id_list = pro::focus::cut(boss.This());
                                boss.remove(applet_ptr);
                                applet.moveto(dot_00);
                                world_ptr->bell::signal(tier::request, vtm::events::handoff, what); // Attach to the world.
                                pro::focus::set(applet_ptr, gear_id_list, solo::off, true); // Refocus.
                                boss.base::riseup(tier::release, e2::form::proceed::quit::one, true); // Destroy placeholder.
                                if (auto new_parent_ptr = applet.parent())
                                {
                                    // Redirect this mouse event to the new world's window.
                                    gear.pass(tier::release, new_parent_ptr, dot_00);
                                }
                            }
                        };
                        boss.LISTEN(tier::anycast, e2::form::upon::started, root)
                        {
                            boss.base::riseup(tier::release, events::enlist, boss.This());
                        };
                        boss.LISTEN(tier::release, hids::events::mouse::button::click::right, gear)
                        {
                            pro::focus::set(boss.This(), gear.id, solo::on);
                        };
                        boss.LISTEN(tier::release, hids::events::mouse::button::click::middle, gear)
                        {
                            pro::focus::set(boss.This(), gear.id, solo::on);
                        };
                        boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent)
                        {
                            parent->LISTEN(tier::anycast, e2::form::prop::cwd, path, boss.relyon)
                            {
                                boss.bell::signal(tier::anycast, e2::form::prop::cwd, path);
                            };
                        };
                    })
                    ->branch(slot::_1, ui::postfx<cell::shaders::contrast>::ctor()
                        ->upload(what.header)
                        ->invoke([&](auto& boss)
                        {
                            boss.color(0, 0);
                            boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent)
                            {
                                auto shadow = ptr::shadow(boss.This());
                                parent->LISTEN(tier::release, e2::form::prop::ui::title, head_foci, -, (shadow))
                                {
                                    if (auto boss_ptr = shadow.lock())
                                    {
                                        boss_ptr->upload(head_foci);
                                    }
                                };
                            };
                        }))
                    ->branch(slot::_2, what.applet);
        };
        auto build_node = [](auto tag, auto slot1, auto slot2, auto grip_width)
        {
            auto highlight_color = skin::color(tone::winfocus);
            auto c3 = highlight_color.bga(0x40);

            auto node = tag == 'h' ? ui::fork::ctor(axis::X, grip_width == -1 ? 2 : grip_width, slot1, slot2)
                                   : ui::fork::ctor(axis::Y, grip_width == -1 ? 1 : grip_width, slot1, slot2);
            node->isroot(faux, base::node) // Set object kind to 1 to be different from others. See node_veer::select.
                ->template plugin<pro::focus>()
                ->limits(dot_00)
                ->invoke([&](auto& boss)
                {
                    mouse_subs(boss);
                    //boss.LISTEN(tier::request, app::tile::events::ui::prev, gear)
                    //{
                    //    if (slot1 is focused)
                    //    {
                    //        if (auto parent_ptr = boss.parent()) // Pass it to the lower nesting level.
                    //        {
                    //            if (parent is node)
                    //            {
                    //                parent_ptr->base::riseup(tier::request, app::tile::events::ui::prev, gear);
                    //            }
                    //            else
                    //            {
                    //                // signal focus slot2
                    //            }
                    //        }
                    //    }
                    //    else if (grip is focused)
                    //    {
                    //        // signal focus slot1
                    //    }
                    //    else if (slot2 is focused)
                    //    {
                    //        // focus grip
                    //    }
                    //};
                    //boss.LISTEN(tier::request, app::tile::events::ui::next, gear)
                    //{
                    //    if (slot2 is focused)
                    //    {
                    //        if (auto parent_ptr = boss.parent()) // Pass it to the lower nesting level.
                    //        {
                    //            if (parent is node)
                    //            {
                    //                parent_ptr->base::riseup(tier::request, app::tile::events::ui::next, gear);
                    //            }
                    //            else
                    //            {
                    //                // signal focus slot1
                    //            }
                    //        }
                    //    }
                    //    else if (grip is focused)
                    //    {
                    //        // signal focus slot2
                    //    }
                    //    else if (slot1 is focused)
                    //    {
                    //        // focus grip
                    //    }
                    //};
                    boss.LISTEN(tier::release, app::tile::events::ui::swap     , gear) { boss.swap();       };
                    boss.LISTEN(tier::release, app::tile::events::ui::rotate   , gear) { boss.rotate();     };
                    boss.LISTEN(tier::release, app::tile::events::ui::equalize , gear) { boss.config(1, 1); };
                    boss.LISTEN(tier::release, hids::events::mouse::scroll::act, gear)
                    {
                        if (gear.meta(hids::anyCtrl))
                        {
                            boss.move_slider(gear.whlsi);
                            gear.dismiss();
                        }
                    };
                });
                auto grip = node->attach(slot::_I,
                                ui::mock::ctor()
                                ->isroot(true)
                                ->template plugin<pro::mover>() //todo GCC 11 requires template keyword
                                ->template plugin<pro::focus>(pro::focus::mode::focusable)
                                ->shader(c3, e2::form::state::focus::count)
                                ->template plugin<pro::shade<cell::shaders::xlight>>()
                                ->invoke([&](auto& boss)
                                {
                                    anycasting(boss);
                                    //todo implement keydb support
                                    boss.LISTEN(tier::release, hids::events::mouse::button::click::right, gear)
                                    {
                                        boss.base::riseup(tier::release, e2::form::size::minimize, gear);
                                        gear.dismiss();
                                    };
                                })
                                ->active());
            return node;
        };
        auto empty_slot = []
        {
            auto window_clr = skin::color(tone::window_clr);
            auto highlight_color = skin::color(tone::winfocus);
            auto danger_color    = skin::color(tone::danger);
            auto c3 = highlight_color.bga(0x40);
            auto c1 = danger_color;

            using namespace app::shared;
            auto [menu_block, cover, menu_data] = menu::mini(true, faux, 1,
            menu::list
            {
                //todo make it configurable
                { menu::item{ menu::type::Command, true, 0, std::vector<menu::item::look>{{ .label = "+", .tooltip = " Launch application instance.                            \n"
                                                                                                                     " The app to run can be set by RightClick on the taskbar. " }}},
                [](auto& boss, auto& /*item*/)
                {
                    boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                    {
                        boss.base::riseup(tier::request, e2::form::proceed::createby, gear);
                        gear.dismiss(true);
                    };
                }},
                { menu::item{ menu::type::Command, true, 0, std::vector<menu::item::look>{{ .label = "│", .tooltip = " Split horizontally " }}},
                [](auto& boss, auto& /*item*/)
                {
                    boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                    {
                        boss.base::riseup(tier::release, app::tile::events::ui::split::hz, gear);
                        gear.dismiss(true);
                    };
                }},
                { menu::item{ menu::type::Command, true, 0, std::vector<menu::item::look>{{ .label = "──", .tooltip = " Split vertically " }}},
                [](auto& boss, auto& /*item*/)
                {
                    boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                    {
                        boss.base::riseup(tier::release, app::tile::events::ui::split::vt, gear);
                        gear.dismiss(true);
                    };
                }},
                { menu::item{ menu::type::Command, true, 0, std::vector<menu::item::look>{{ .label = "×", .tooltip = " Delete pane ", .hover = c1 }}},
                [](auto& boss, auto& /*item*/)
                {
                    boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                    {
                        boss.base::riseup(tier::release, e2::form::proceed::quit::one, true);
                        gear.dismiss(true);
                    };
                }},
            });
            menu_data->active(window_clr);
            auto menu_id = menu_block->id;
            cover->setpad({ 0,0,3,0 });
            cover->invoke([&](auto& boss)
            {
                boss.LISTEN(tier::release, e2::render::any, parent_canvas, -, (menu_id))
                {
                    parent_canvas.fill([&](cell& c){ c.txt(whitespace).link(menu_id); });
                };
            });

            return ui::cake::ctor()
                ->isroot(true, base::placeholder)
                ->active(window_clr)
                ->limits(dot_00, -dot_11)
                ->plugin<pro::focus>(pro::focus::mode::focusable)
                ->shader(c3, e2::form::state::focus::count)
                ->invoke([&](auto& boss)
                {
                    anycasting(boss);
                    mouse_subs(boss);
                    boss.LISTEN(tier::release, hids::events::mouse::button::click::right, gear)
                    {
                        boss.base::riseup(tier::request, e2::form::proceed::createby, gear);
                        gear.dismiss();
                    };
                })
                ->branch
                (
                    ui::post::ctor()->upload("Empty Slot", 10)
                        ->limits({ 10,1 }, { 10,1 })
                        ->alignment({ snap::center, snap::center })
                )
                ->branch
                (
                    menu_block->alignment({ snap::head, snap::head })
                );
        };
        auto node_veer = [](auto&& node_veer, auto min_state) -> netxs::sptr<ui::veer>
        {
            return ui::veer::ctor()
                ->plugin<pro::focus>()
                ->active()
                ->invoke([&](auto& boss)
                {
                    auto highlight = [](auto& boss, auto state)
                    {
                        auto window_clr = skin::color(tone::window_clr);
                        auto highlight_color = skin::color(tone::winfocus);
                        auto c3 = highlight_color.alpha(0x70);
                        auto c = state ? c3 : window_clr;
                        boss.front()->color(c.fgc(), c.bgc());
                        boss.deface();
                    };
                    boss.LISTEN(tier::release, e2::form::size::minimize, gear, -, (saved_ratio = 1, min_ratio = 1, min_state))
                    {
                        if (auto node = std::dynamic_pointer_cast<ui::fork>(boss.base::parent()))
                        {
                            auto ratio = node->get_ratio();
                            if (ratio == min_ratio)
                            {
                                node->set_ratio(saved_ratio);
                                pro::focus::set(boss.This(), gear.id, gear.meta(hids::anyCtrl) ? solo::off : solo::on, true);
                            }
                            else
                            {
                                saved_ratio = ratio;
                                node->set_ratio(min_state);
                                min_ratio = node->get_ratio();
                                pro::focus::off(boss.This(), gear.id);
                            }
                            node->base::reflow();
                        }
                    };
                    boss.LISTEN(tier::release, e2::config::plugins::sizer::alive, state)
                    {
                        // Block a rising up of this event: dtvt object fires this event on exit.
                    };
                    boss.LISTEN(tier::release, vtm::events::d_n_d::abort, target)
                    {
                        if (boss.count())
                        {
                            highlight(boss, faux);
                        }
                    };
                    boss.LISTEN(tier::release, vtm::events::d_n_d::ask, target)
                    {
                        if (boss.count() == 1) // Only empty slot available.
                        {
                            highlight(boss, true);
                            target = boss.This();
                        }
                    };
                    boss.LISTEN(tier::release, vtm::events::d_n_d::drop, what)
                    {
                        if (boss.count() == 1) // Only empty pane/slot available.
                        {
                            highlight(boss, faux);
                            pro::focus::off(boss.back()); // Unset focus from node_veer if it is focused.
                            auto app = app_window(what);
                            boss.attach(app);
                            app->bell::signal(tier::anycast, e2::form::upon::started);
                        }
                    };
                    boss.LISTEN(tier::release, e2::form::proceed::swap, item_ptr)
                    {
                        if (boss.count() == 1) // Only empty slot available.
                        {
                            if constexpr (debugmode) log(prompt::tile, "Empty slot swap: defective structure, count=", boss.count());
                        }
                        else if (boss.count() == 2)
                        {
                            auto gear_id_list = pro::focus::cut(boss.This());
                            auto deleted_item = boss.pop_back();
                            if (item_ptr)
                            {
                                boss.attach(item_ptr);
                            }
                            else item_ptr = boss.This();
                            pro::focus::set(boss.back(), gear_id_list, solo::off);
                        }
                        else
                        {
                            if constexpr (debugmode) log(prompt::tile, "Empty slot swap: defective structure, count=", boss.count());
                        }
                    };
                    boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent)
                    {
                        parent->LISTEN(tier::request, e2::form::proceed::swap, item_ptr, boss.relyon)
                        {
                            if (item_ptr != boss.This())
                            {
                                if (boss.count() == 1) // Only empty slot available.
                                {
                                    item_ptr.reset();
                                }
                                else if (boss.count() == 2)
                                {
                                    auto gear_id_list = pro::focus::cut(boss.This());
                                    item_ptr = boss.pop_back();
                                    pro::focus::set(boss.back(), gear_id_list, solo::off);
                                }
                                else
                                {
                                    if constexpr (debugmode) log(prompt::tile, "Empty slot: defective structure, count=", boss.count());
                                }
                                if (auto parent = boss.parent())
                                {
                                    parent->bell::expire(tier::request);
                                }
                            }
                        };
                    };
                    boss.LISTEN(tier::anycast, e2::form::upon::started, root)
                    {
                        if (auto item_ptr = boss.back())
                        {
                            auto& item = *item_ptr;
                            if (item.base::root())
                            {
                                item.bell::signal(tier::anycast, e2::form::upon::started, root);
                            }
                        }
                    };
                    boss.LISTEN(tier::anycast, app::tile::events::ui::select, gear)
                    {
                        auto item_ptr = boss.back();
                        if (item_ptr->base::kind() != base::node) pro::focus::set(item_ptr, gear.id, solo::off);
                        else                                      pro::focus::off(item_ptr, gear.id); // Exclude grips.
                    };
                    //boss.LISTEN(tier::request, app::tile::events::ui::select, gear)
                    //{
                    //    auto item_ptr = boss.back();
                    //    if (item_ptr->base::kind() != base::node) pro::focus::set(item_ptr, gear.id, solo::off);
                    //    else                                      pro::focus::off(item_ptr, gear.id); // Exclude grips.
                    //};
                    boss.LISTEN(tier::preview, e2::form::size::enlarge::any, gear, -, (oneoff = subs{}))
                    {
                        if (boss.count() > 2 || oneoff) // It is a root or is already maximized. See build_inst::slot::_2's e2::form::proceed::attach for details.
                        {
                            boss.base::riseup(tier::release, e2::form::proceed::attach, e2::form::proceed::attach.param());
                        }
                        else
                        {
                            if (boss.count() > 1) // Preventing the empty slot from maximizing.
                            if (boss.back())
                            if (boss.back()->base::kind() == base::client) // Preventing the splitter from maximizing.
                            {
                                auto fullscreen_item = boss.pop_back();
                                auto gear_id_list = pro::focus::cut(boss.This());
                                fullscreen_item->LISTEN(tier::release, e2::form::size::restore, item_ptr, oneoff)
                                {
                                    if (item_ptr)
                                    {
                                        boss.attach(item_ptr);
                                        boss.base::reflow();
                                    }
                                    oneoff.reset();
                                };
                                fullscreen_item->LISTEN(tier::release, e2::dtor, item_ptr, oneoff)
                                {
                                    oneoff.reset();
                                };
                                auto just_copy = fullscreen_item;
                                boss.base::riseup(tier::release, e2::form::proceed::attach, fullscreen_item);
                                pro::focus::set(just_copy, gear_id_list, solo::off); // Handover all foci.
                                boss.base::reflow();
                            }
                        }
                    };
                    boss.LISTEN(tier::release, app::tile::events::ui::split::any, gear)
                    {
                        if (auto deed = boss.bell::protos(tier::release))
                        {
                            auto depth = 0;
                            boss.diveup([&]{ depth++; });
                            if constexpr (debugmode) log(prompt::tile, "Depth ", depth);
                            if (depth > inheritance_limit) return;

                            auto heading = deed == app::tile::events::ui::split::vt.id;
                            auto newnode = build_node(heading ? 'v':'h', 1, 1, heading ? 1 : 2);
                            auto empty_1 = node_veer(node_veer, ui::fork::min_ratio);
                            auto empty_2 = node_veer(node_veer, ui::fork::max_ratio);
                            auto gear_id_list = pro::focus::cut(boss.This());
                            auto curitem = boss.pop_back();
                            if (boss.empty())
                            {
                                boss.attach(empty_slot());
                                empty_1->pop_back();
                            }
                            auto slot_1 = newnode->attach(slot::_1, empty_1->branch(curitem));
                            auto slot_2 = newnode->attach(slot::_2, empty_2);
                            boss.attach(newnode);
                            pro::focus::set(slot_1->back(), gear_id_list, solo::off); // Handover all foci.
                            pro::focus::set(slot_2->back(), gear_id_list, solo::off);
                        }
                    };
                    boss.LISTEN(tier::anycast, e2::form::proceed::quit::any, fast)
                    {
                        boss.bell::signal(tier::preview, e2::form::proceed::quit::one, fast);
                    };
                    boss.LISTEN(tier::preview, e2::form::proceed::quit::one, fast)
                    {
                        if (boss.count() > 1 && boss.back()->base::root()) // Walking a nested visual tree.
                        {
                            boss.back()->bell::signal(tier::anycast, e2::form::proceed::quit::one, true); // fast=true: Immediately closing (no ways to showing a closing process). Forward a quit message to hosted app in order to schedule a cleanup.
                        }
                        else // Close an empty slot (boss.count() == 1).
                        {
                            boss.bell::enqueue(boss.This(), [&](auto& /*boss*/) // Enqueue to keep the focus tree intact while processing key events.
                            {
                                boss.bell::signal(tier::release, e2::form::proceed::quit::one, fast);
                            });
                        }
                    };
                    boss.LISTEN(tier::release, e2::form::proceed::quit::any, fast)
                    {
                        if (auto parent = boss.base::parent())
                        {
                            if (boss.count() > 1 && boss.back()->base::kind() == base::client) // Only apps can be deleted.
                            {
                                auto gear_id_list = pro::focus::cut(boss.This());
                                auto deleted_item = boss.pop_back(); // Throw away.
                                pro::focus::set(boss.back(), gear_id_list, solo::off);
                            }
                            else if (boss.count() == 1) // Remove empty slot, reorganize.
                            {
                                auto item_ptr = parent->bell::signal(tier::request, e2::form::proceed::swap, boss.This()); // sptr must be of the same type as the event argument. Casting kills all intermediaries when return.
                                if (item_ptr != boss.This()) // Parallel slot is not empty or both slots are empty (item_ptr == null).
                                {
                                    parent->base::riseup(tier::release, e2::form::proceed::swap, item_ptr);
                                }
                            }
                            boss.deface();
                        }
                    };
                    boss.LISTEN(tier::request, e2::form::proceed::createby, gear)
                    {
                        static auto insts_count = 0;

                        if (boss.count() != 1) return; // Create new apps at the empty slots only.
                        auto& gate = gear.owner;
                        auto current_default = gate.bell::signal(tier::request, e2::data::changed);
                        auto config = gate.base::riseup(tier::request, vtm::events::apptype, { .menuid = current_default });
                        if (config.kindid == netxs::app::site::id) return; // Deny any desktop viewport markers inside the tiling manager.

                        gate.base::riseup(tier::request, vtm::events::newapp, config);
                        auto app = app_window(config);
                        pro::focus::off(boss.back());
                        boss.attach(app);
                        if (auto world_ptr = gate.parent()) // Finalize app creation.
                        {
                            app->bell::signal(tier::anycast, vtm::events::attached, world_ptr);
                        }

                        insts_count++; //todo unify, demo limits
                        config.applet->LISTEN(tier::release, e2::dtor, applet_id)
                        {
                            insts_count--;
                            if constexpr (debugmode) log(prompt::tile, "Instance detached: id:", id, "; left:", insts_count);
                        };

                        app->bell::signal(tier::anycast, e2::form::upon::started, app);
                        pro::focus::set(app, gear.id, solo::off);
                    };
                    boss.LISTEN(tier::release, events::backup, node_veer_list)
                    {
                        if (boss.count())
                        if (auto item_ptr = boss.back())
                        if (item_ptr->base::root())
                        {
                            node_veer_list.push_back(boss.This());
                        }
                    };
                })
                ->branch(empty_slot());
        };
        auto parse_data = [](auto&& parse_data, view& utf8, auto min_ratio) -> netxs::sptr<ui::veer>
        {
            auto slot = node_veer(node_veer, min_ratio);
            utf::trim_front(utf8, ", ");
            if (utf8.empty()) return slot;
            auto tag = utf8.front();
            if ((tag == 'h' || tag == 'v') && utf8.find('(') < utf8.find(','))
            {
                // add split
                utf8.remove_prefix(1);
                utf::trim_front(utf8, " ");
                auto s1 = si32{ 1 };
                auto s2 = si32{ 1 };
                auto w  = si32{-1 };
                if (auto l = utf::to_int(utf8)) // Left side ratio
                {
                    s1 = std::abs(l.value());
                    if (utf8.empty() || utf8.front() != ':') return slot;
                    utf8.remove_prefix(1);
                    if (auto r = utf::to_int(utf8)) // Right side ratio
                    {
                        s2 = std::abs(r.value());
                        utf::trim_front(utf8, " ");
                        if (!utf8.empty() && utf8.front() == ':') // Grip width.
                        {
                            utf8.remove_prefix(1);
                            if (auto g = utf::to_int(utf8))
                            {
                                w = std::abs(g.value());
                                utf::trim_front(utf8, " ");
                            }
                        }
                    }
                    else return slot;
                }
                if (utf8.empty() || utf8.front() != '(') return slot;
                utf8.remove_prefix(1);
                auto node = build_node(tag, s1, s2, w);
                auto slot1 = node->attach(slot::_1, parse_data(parse_data, utf8, ui::fork::min_ratio));
                auto slot2 = node->attach(slot::_2, parse_data(parse_data, utf8, ui::fork::max_ratio));
                slot->attach(node);
                utf::trim_front(utf8, ") ");
            }
            else  // Add application.
            {
                utf::trim_front(utf8, " ");
                auto menuid = utf::take_front(utf8, " ,)").str();
                if (menuid.empty()) return slot;

                utf::trim_front(utf8, " ,");
                if (utf8.size() && utf8.front() == ')') utf8.remove_prefix(1); // pop ')';

                auto oneoff = ptr::shared(hook{});
                slot->LISTEN(tier::anycast, vtm::events::attached, world_ptr, *oneoff, (oneoff, menuid, slot))
                {
                    auto what = world_ptr->bell::signal(tier::request, vtm::events::newapp, { .menuid = menuid });
                    auto inst = app_window(what);
                    slot->attach(inst);
                    inst->bell::signal(tier::anycast, vtm::events::attached, world_ptr);
                    oneoff.reset();
                };
            }
            return slot;
        };
        auto foreach = [](std::vector<sptr>& node_veer_stack, bool with_grips, id_t focused_by_gear_id, auto proc)
        {
            while (node_veer_stack.size())
            {
                if (auto node_veer_ptr = std::dynamic_pointer_cast<ui::veer>(node_veer_stack.back()))
                {
                    node_veer_stack.pop_back();
                    if (!focused_by_gear_id || node_veer_ptr->plugins<pro::focus>().is_focused(focused_by_gear_id))
                    {
                        auto item = node_veer_ptr->back();
                        if (node_veer_ptr->count() == 1) // Empty slot.
                        {
                            proc(item, 0);
                        }
                        else if (item->root()) // App window.
                        {
                            proc(item->base::subset[1], 1); // Applet.
                        }
                        else // if (!item->root()) // Node.
                        {
                            if (with_grips)
                            {
                                proc(item->base::subset[2], 2); // Grip.
                            }
                            node_veer_stack.push_back(item->base::subset[0]);
                            node_veer_stack.push_back(item->base::subset[1]);
                        }
                    }
                }
                else
                {
                    node_veer_stack.clear();
                }
            }
        };
        auto build_inst = [](eccc appcfg, xmls& config) -> sptr
        {
            // tile (ui::fork, f, k)
            //  │ │
            //  │ └─ slot::_1 ─ menu_block
            //  └─── slot::_2 ─ parse_data
            //        │
            //        └─ node_veer (ui::veer, f)
            //            │
            //            ├─ empty_slot (ui::cake, f)->isroot(true, base::placeholder)
            //            │   │
            //            │   ├─ ui::post ("Empty Slot")
            //            │   └─ menu (" +  |  ─  x ")
            //
            //            └─ app_window (ui::fork, f)->isroot(true)
            //                │ │
            //           or   │ └─ slot::_1: ui::postfx<cell::shaders::contrast>("Title")
            //                └─── slot::_2: what.applet
            //            └─ node (ui::fork, f)->isroot(faux, base::node)
            //                │ │ │
            //                │ │ └─ slot::_1: parse_data...
            //                │ └─── slot::_I: (ui::mock, f)->isroot(true)
            //                └───── slot::_2: parse_data...
            //            :
            //            └─ maximized node_veer...

            auto param = view{ appcfg.cmd };
            auto window_clr = skin::color(tone::window_clr);
            //auto highlight_color = skin::color(tone::highlight);
            auto danger_color    = skin::color(tone::danger);
            //auto warning_color   = skin::color(tone::warning);
            //auto c3 = highlight_color;
            //auto c2 = warning_color;
            auto c1 = danger_color;

            auto object = ui::fork::ctor(axis::Y)
                ->plugin<items>()
                ->plugin<pro::focus>()
                ->plugin<pro::keybd>()
                ->invoke([&](auto& boss)
                {
                    auto oneoff = ptr::shared(hook{});
                    boss.LISTEN(tier::anycast, e2::form::upon::created, gear, *oneoff, (oneoff))
                    {
                        auto& gate = gear.owner;
                        auto menuid = gate.bell::signal(tier::request, e2::data::changed);
                        auto conf_list_ptr = gate.base::riseup(tier::request, desk::events::menu);
                        auto& conf_list = *conf_list_ptr;
                        auto& config = conf_list[menuid];
                        if (config.type == app::tile::id) // Reset the currently selected application to the previous one.
                        {
                            gate.bell::signal(tier::preview, e2::data::changed, menuid); // Get previous default;
                            gate.bell::signal(tier::release, e2::data::changed, menuid); // Set current  default;
                        }
                        oneoff.reset();
                    };
                    boss.LISTEN(tier::preview, e2::form::prop::cwd, path)
                    {
                        boss.bell::signal(tier::anycast, e2::form::prop::cwd, path);
                    };
                    boss.LISTEN(tier::request, e2::form::proceed::swap, item_ptr) // Close the tile window manager if we receive a `swap-request` from the top-level `empty-slot`.
                    {
                        boss.base::riseup(tier::release, e2::form::proceed::quit::one, true);
                    };
                    auto& keybd = boss.template plugins<pro::keybd>();
                    keybd.proc(action::TileFocusPrevPane     , [&](hids& gear, txts& /*args*/){ boss.bell::signal(tier::request, app::tile::events::ui::focus::prev, gear); });
                    keybd.proc(action::TileFocusNextPane     , [&](hids& gear, txts& /*args*/){ boss.bell::signal(tier::request, app::tile::events::ui::focus::next, gear); });
                    keybd.proc(action::TileRunApplicatoin    , [&](hids& gear, txts& /*args*/){ boss.bell::signal(tier::request, app::tile::events::ui::create,      gear); });
                    keybd.proc(action::TileSelectAllPanes    , [&](hids& gear, txts& /*args*/){ boss.bell::signal(tier::request, app::tile::events::ui::select,      gear); });
                    keybd.proc(action::TileSplitHorizontally , [&](hids& gear, txts& /*args*/){ boss.bell::signal(tier::request, app::tile::events::ui::split::hz,   gear); });
                    keybd.proc(action::TileSplitVertically   , [&](hids& gear, txts& /*args*/){ boss.bell::signal(tier::request, app::tile::events::ui::split::vt,   gear); });
                    keybd.proc(action::TileSplitOrientation  , [&](hids& gear, txts& /*args*/){ boss.bell::signal(tier::request, app::tile::events::ui::rotate,      gear); });
                    keybd.proc(action::TileSwapPanes         , [&](hids& gear, txts& /*args*/){ boss.bell::signal(tier::request, app::tile::events::ui::swap,        gear); });
                    keybd.proc(action::TileEqualizeSplitRatio, [&](hids& gear, txts& /*args*/){ boss.bell::signal(tier::request, app::tile::events::ui::equalize,    gear); });
                    keybd.proc(action::TileSetManagerTitle   , [&](hids& gear, txts& /*args*/){ boss.bell::signal(tier::request, app::tile::events::ui::title ,      gear); });
                    keybd.proc(action::TileClosePane         , [&](hids& gear, txts& /*args*/){ boss.bell::signal(tier::request, app::tile::events::ui::close,       gear); });
                    auto bindings = pro::keybd::load(config, "tile");
                    keybd.bind(bindings);

                    boss.LISTEN(tier::request, app::tile::events::ui::focus::prev, gear)
                    {
                        log("ui::prev");
                    };
                    boss.LISTEN(tier::request, app::tile::events::ui::focus::next, gear)
                    {
                        log("ui::next");
                    };
                    boss.LISTEN(tier::request, app::tile::events::ui::create, gear)
                    {
                        log("ui::create");
                    };
                    boss.LISTEN(tier::request, app::tile::events::ui::select, gear, -, (node_veer_stack = std::vector<sptr>{}))
                    {
                        log("ui::select");
                        auto node_veer_ptr = boss.base::subset[1];
                        auto with_grips = true;
                        auto by_gear_id = id_t{};
                        node_veer_stack.push_back(node_veer_ptr);
                        foreach(node_veer_stack, with_grips, by_gear_id, [&](auto& item_ptr, si32 item_type)
                        {
                            if (item_type != 2) pro::focus::set(item_ptr, gear.id, solo::off);
                            else                pro::focus::off(item_ptr, gear.id); // Exclude grips.
                        });
                    };
                    boss.LISTEN(tier::request, app::tile::events::ui::split::hz, gear)
                    {
                        log("ui::split::hz");
                    };
                    boss.LISTEN(tier::request, app::tile::events::ui::split::vt, gear)
                    {
                        log("ui::split::vt");
                    };
                    boss.LISTEN(tier::request, app::tile::events::ui::rotate,    gear)
                    {
                        log("ui::rotate");
                    };
                    boss.LISTEN(tier::request, app::tile::events::ui::swap,      gear)
                    {
                        log("ui::swap");
                    };
                    boss.LISTEN(tier::request, app::tile::events::ui::equalize,  gear)
                    {
                        log("ui::equalize");
                    };
                    boss.LISTEN(tier::request, app::tile::events::ui::title ,    gear)
                    {
                        log("ui::title");
                        app::shared::set_title(boss, gear);
                    };
                    boss.LISTEN(tier::request, app::tile::events::ui::close,     gear)
                    {
                        log("ui::close");
                        boss.base::riseup(tier::preview, e2::form::proceed::quit::one, true);
                    };
                });
            static auto on_left_click = [](auto& boss, auto& event)
            {
                boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                {
                    gear.nodbl = true;
                    boss.bell::expire(tier::release);
                    boss.base::riseup(tier::request, event, gear);
                };
            };
            using namespace app::shared;
            static const auto proc_map = menu::action_map_t
            {
                { tile::action::TileFocusPrevPane     , [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::focus::prev); }},
                { tile::action::TileFocusNextPane     , [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::focus::next); }},
                { tile::action::TileRunApplicatoin    , [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::create     ); }},
                { tile::action::TileSelectAllPanes    , [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::select     ); }},
                { tile::action::TileSplitHorizontally , [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::split::hz  ); }},
                { tile::action::TileSplitVertically   , [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::split::vt  ); }},
                { tile::action::TileSplitOrientation  , [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::rotate     ); }},
                { tile::action::TileSwapPanes         , [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::swap       ); }},
                { tile::action::TileEqualizeSplitRatio, [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::equalize   ); }},
                { tile::action::TileSetManagerTitle   , [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::title      ); }},
                { tile::action::TileClosePane         , [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::close      ); }},
            };
            config.cd("/config/tile", "/config/defapp");
            auto [menu_block, cover, menu_data] = menu::load(config, proc_map);
            object->attach(slot::_1, menu_block)
                ->invoke([](auto& boss)
                {
                    boss.LISTEN(tier::anycast, e2::form::proceed::quit::any, fast)
                    {
                        boss.base::riseup(tier::release, e2::form::proceed::quit::one, fast);
                    };
                });
            menu_data->active(window_clr)
                     //->plugin<pro::track>()
                     ->plugin<pro::acryl>();
            auto menu_id = menu_block->id;
            cover->invoke([&](auto& boss)
            {
                auto bar = cell{ "▀"sv }.link(menu_id);
                boss.LISTEN(tier::release, e2::render::any, parent_canvas, -, (bar))
                {
                    auto window_clr = skin::color(tone::window_clr);
                    auto fgc = window_clr.bgc();
                    parent_canvas.fill([&](cell& c){ c.fgc(fgc).txt(bar).link(bar); });
                };
            });
            if (appcfg.cwd.size())
            {
                auto err = std::error_code{};
                fs::current_path(appcfg.cwd, err);
                if (err) log("%%Failed to change current directory to '%cwd%', error code: %error%", prompt::tile, appcfg.cwd, err.value());
                else     log("%%Change current directory to '%cwd%'", prompt::tile, appcfg.cwd);
            }
            object->attach(slot::_2, parse_data(parse_data, param, ui::fork::min_ratio))
                ->invoke([&](auto& boss)
                {
                    boss.LISTEN(tier::release, e2::form::proceed::attach, fullscreen_item, -, (foci_list = gear_id_list_t{}))
                    {
                        if (boss.count() > 2)
                        {
                            auto gear_id_list = pro::focus::cut(boss.This());
                            auto item_ptr = boss.pop_back();
                            item_ptr->bell::signal(tier::release, e2::form::size::restore, item_ptr);
                            pro::focus::set(boss.back(), foci_list, solo::off, true); // Restore saved foci.
                            pro::focus::set(item_ptr, gear_id_list, solo::off); // Apply item's foci.
                            foci_list.clear();
                        }

                        if (fullscreen_item)
                        {
                            foci_list = pro::focus::cut(boss.This());
                            boss.attach(fullscreen_item);
                            fullscreen_item.reset();
                        }
                        else log(prompt::tile, "Fullscreen item is empty");
                    };
                    boss.LISTEN(tier::anycast, app::tile::events::ui::any, gear)
                    {
                        if (auto deed = boss.bell::protos(tier::anycast))
                        {
                            if (boss.count() > 2 && deed != app::tile::events::ui::toggle.id) // Restore the window before any action if maximized.
                            {
                                boss.base::riseup(tier::release, e2::form::proceed::attach, e2::form::proceed::attach.param());
                            }

                            if (deed == app::tile::events::ui::swap.id)
                            {
                                //todo reimplement (avoid tier::general usage)
                                auto node_veer_list = backups{};
                                auto proc = e2::form::proceed::functor.param([&](sptr item_ptr)
                                {
                                    auto gear_test = item_ptr->bell::signal(tier::request, e2::form::state::keybd::find, { gear.id, 0 });
                                    if (gear_test.second)
                                    {
                                        item_ptr->base::riseup(tier::release, events::backup, node_veer_list);
                                    }
                                });
                                boss.bell::signal(tier::general, e2::form::proceed::functor, proc);
                                auto slots_count = node_veer_list.size();
                                log(prompt::tile, "Slots count:", slots_count);
                                if (slots_count >= 2) // Swap selected panes cyclically.
                                {
                                    log(prompt::tile, "Swap slots cyclically");
                                    auto emp_slot = sptr{};
                                    auto app_slot = sptr{};
                                    auto emp_next = sptr{};
                                    auto app_next = sptr{};
                                    for (auto& s : node_veer_list)
                                    {
                                        if (s->count() == 1) // empty only
                                        {
                                            app_next.reset();
                                            emp_next = s->pop_back();
                                        }
                                        else if (s->count() == 2) // empty + app
                                        {
                                            if (auto app = s->back())
                                            {
                                                app->bell::signal(tier::release, events::delist, true);
                                            }
                                            app_next = s->pop_back();
                                            emp_next = s->pop_back();
                                        }
                                        if (emp_slot) s->attach(emp_slot);
                                        if (app_slot)
                                        {
                                            s->attach(app_slot);
                                            app_slot->base::riseup(tier::release, events::enlist, app_slot);
                                        }
                                        std::swap(emp_slot, emp_next);
                                        std::swap(app_slot, app_next);
                                    }
                                    auto& s = node_veer_list.front();
                                    if (emp_slot) s->attach(emp_slot);
                                    if (app_slot)
                                    {
                                        s->attach(app_slot);
                                        app_slot->base::riseup(tier::release, events::enlist, app_slot);
                                    }
                                    gear.countdown = 0; // Interrupt swapping.
                                }
                                else // Swap panes in split.
                                {
                                    gear.countdown = 1;
                                }
                            }
                        }
                    };
                });
            return object;
        };
    }

    app::shared::initialize builder{ app::tile::id, build_inst };
}