// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

namespace netxs::events::userland
{
    namespace tile
    {
        EVENTPACK( tile::events, ui::e2::extra::slot4 )
        {
            EVENT_XS( enlist, ui::sptr           ),
            EVENT_XS( delist, bool               ),
            GROUP_XS( ui    , input::hids        ), // Window manager command pack.

            SUBSET_XS( ui )
            {
                EVENT_XS( create  , input::hids ), // Run app if pane is empty.
                EVENT_XS( close   , input::hids ), // Close panes.
                EVENT_XS( swap    , input::hids ), // Swap panes.
                EVENT_XS( rotate  , input::hids ), // Change split orientation.
                EVENT_XS( equalize, input::hids ), // Make panes the same size.
                EVENT_XS( select  , input::hids ), // Focusize all panes.
                EVENT_XS( title   , input::hids ), // Set window manager title using clipboard.
                GROUP_XS( focus   , input::hids ), // Focusize prev/next pane.
                GROUP_XS( split   , input::hids ), // Split panes.
                GROUP_XS( grips   , twod        ), // Splitting grip modification.

                SUBSET_XS( focus )
                {
                    EVENT_XS( next    , input::hids ),
                    EVENT_XS( prev    , input::hids ),
                    EVENT_XS( nextpane, input::hids ),
                    EVENT_XS( prevpane, input::hids ),
                    EVENT_XS( nextgrip, input::hids ),
                    EVENT_XS( prevgrip, input::hids ),
                };
                SUBSET_XS( split )
                {
                    EVENT_XS( vt, input::hids ),
                    EVENT_XS( hz, input::hids ),
                };
                SUBSET_XS( grips )
                {
                    EVENT_XS( move  , twod ),
                    EVENT_XS( resize, si32 ),
                };
            };
        };
    }
}

// tile: Tiling window manager.
namespace netxs::app::tile
{
    static constexpr auto id = "tile";
    static constexpr auto name = "Tiling Window Manager";
    static constexpr auto inheritance_limit = 30; // Tiling limits.

    namespace events = netxs::events::userland::tile;

    using ui::sptr;
    using ui::wptr;

    #define proc_list \
        X(FocusNextPaneOrGrip) \
        X(FocusNextPane      ) \
        X(FocusNextGrip      ) \
        X(MoveGrip           ) \
        X(ResizeGrip         ) \
        X(RunApplication     ) \
        X(SelectAllPanes     ) \
        X(SplitPane          ) \
        X(RotateSplit        ) \
        X(SwapPanes          ) \
        X(EqualizeSplitRatio ) \
        X(SetTitle           ) \
        X(ClosePane          ) \

    struct methods
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
        si32                  window_state;

    public:
        items(base&&) = delete;
        items(base& boss)
            : skill{ boss },
              client{ boss.attach(ui::list::ctor(axis::Y)) },// sort::reverse)) },
              window_state{ winstate::undefined }
        {
            boss.LISTEN(tier::release, e2::area, new_area, memo)
            {
                auto coor = twod{ new_area.size.x + 2/*resize grip width*/, 0 };
                client->base::moveto(coor);
            };
            boss.LISTEN(tier::release, tile::events::enlist, object, memo)
            {
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
                            auto& data_shadow = boss.base::field(ptr::shadow(data_src_sptr));
                            boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent)
                            {
                                parent->resize();
                            };
                            boss.LISTEN(tier::release, e2::form::upon::vtree::detached, parent)
                            {
                                parent->resize(); // Rebuild list.
                            };
                            data_src_sptr->LISTEN(tier::release, tile::events::delist, f, boss.sensors)
                            {
                                boss.base::detach(); // Destroy itself.
                            };
                            boss.on(tier::mouserelease, input::key::MouseAny, [&](hids& gear)
                            {
                                if ((gear.cause & 0x00FF) && !gear.dragged) // Button events only.
                                if (auto data_ptr = data_shadow.lock())
                                {
                                    auto& data_src = *data_ptr;
                                    gear.forward(tier::mouserelease, data_src);
                                    gear.dismiss();
                                }
                            });
                            boss.LISTEN(tier::release, e2::form::state::mouse, hovered)
                            {
                                if (auto data_ptr = data_shadow.lock())
                                {
                                    data_ptr->base::signal(tier::release, e2::form::state::highlight, hovered);
                                }
                            };
                        });
                };
                client->attach_element(e2::form::prop::ui::header, object, label);
            };
            boss.LISTEN(tier::release, e2::render::any, parent_canvas, memo)
            {
                if (window_state == winstate::normal)
                {
                    auto context2D = parent_canvas.bump({ 0, si32max / 2, 0, si32max / 2 });
                    client->render(parent_canvas);
                    parent_canvas.bump(context2D);
                }
            };
            boss.LISTEN(tier::anycast, e2::form::upon::started, root_ptr, memo)
            {
                client->clear();
                if (auto parent_ptr = boss.base::parent())
                {
                    window_state = parent_ptr->base::riseup(tier::request, e2::form::prop::window::state);
                }
            };
        }
    };

    namespace
    {
        auto mouse_subs = [](auto& boss)
        {
            boss.on(tier::mouserelease, input::key::LeftDoubleClick, [&](hids& gear)
            {
                boss.base::riseup(tier::preview, e2::form::size::enlarge::maximize, gear);
                gear.dismiss();
            });
        };
        auto app_window = [](auto& what)
        {
            return ui::fork::ctor(axis::Y)
                    ->template plugin<pro::title>(what.applet->base::property("window.header"), what.applet->base::property("window.footer"), true, faux, true)
                    ->template plugin<pro::light>() //todo gcc requires template keyword
                    ->template plugin<pro::focus>()
                    ->limits({ 10, -1 }, { -1, -1 })
                    ->isroot(true)
                    ->active()
                    ->invoke([&](auto& boss)
                    {
                        mouse_subs(boss);
                        if (what.applet->size() != dot_00) boss.resize(what.applet->size() + dot_01/*approx title height*/);
                        auto applet_shadow = ptr::shadow(what.applet);
                        boss.on(tier::mouserelease, input::key::LeftDragStart, [&, applet_shadow](hids& gear)
                        {
                            if (auto applet_ptr = applet_shadow.lock())
                            if (applet_ptr->area().hittest(gear.coord))
                            {
                                auto& applet = *applet_ptr;

                                // Restore if maximized. Parent can be changed after that.
                                boss.base::signal(tier::release, e2::form::size::restore);

                                // Take current title.
                                auto what = vtm::events::handoff.param();
                                auto& header = applet.base::property("window.header");
                                if (header.empty()) header = applet.base::property("window.menuid");

                                // Get creator.
                                auto world_ptr = boss.base::signal(tier::general, e2::config::creator);
                                auto& world = *world_ptr;

                                // Take coor.
                                gear.coord -= applet.base::coor(); // Rebase mouse coor.
                                gear.click -= applet.base::coor(); // Rebase mouse click.
                                auto& applet_area = applet.base::template property<rect>("window.area");
                                if (!applet_area) applet_area.size = applet.base::size();
                                auto coor = dot_00;
                                applet.base::global(coor);
                                applet_area.coor = -coor;
                                what.applet = applet_ptr;

                                // Detach from the wm.
                                auto gear_id_list = pro::focus::cut(applet_ptr);
                                //todo revise (soul, mouse event tree caching, /m.reset()?)
                                gear.redirect_mouse_focus(world);
                                boss.remove(applet_ptr);
                                applet.base::moveto(dot_00);
                                world.base::signal(tier::request, vtm::events::handoff, what); // Attach to the world.
                                pro::focus::set(applet_ptr, gear.id, solo::on, true);
                                boss.base::riseup(tier::release, e2::form::proceed::quit::one, true); // Destroy placeholder.
                                if (auto new_parent_ptr = applet.base::parent())
                                {
                                    // Redirect this mouse event to the new world's window.
                                    auto& new_parent = *new_parent_ptr;
                                    gear.pass(tier::mouserelease, new_parent, dot_00);
                                }
                            }
                        });
                        boss.on(tier::mouserelease, input::key::LeftRightDragStart);
                        boss.on(tier::mouserelease, input::key::RightClick, [&](hids& gear)
                        {
                            pro::focus::set(boss.This(), gear.id, solo::on);
                        });
                        boss.on(tier::mouserelease, input::key::MiddleClick);
                        boss.LISTEN(tier::anycast, e2::form::upon::started, root_ptr)
                        {
                            boss.base::riseup(tier::release, tile::events::enlist, boss.This());
                        };
                        boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent)
                        {
                            parent->LISTEN(tier::anycast, e2::form::prop::cwd, path, boss.relyon)
                            {
                                boss.base::signal(tier::anycast, e2::form::prop::cwd, path);
                            };
                        };
                    })
                    ->branch(slot::_1, ui::postfx<cell::shaders::contrast>::ctor()
                        ->upload(what.applet->base::property("window.header"))
                        ->shader(cell::shaders::text(cell{ whitespace }))
                        ->invoke([&](auto& boss)
                        {
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
        auto build_node = [](auto tag, auto slot1, auto slot2, auto grip_width, auto grip_bindings_ptr)
        {
            auto highlight_color = skin::color(tone::winfocus);
            auto c3 = highlight_color;

            auto node = tag == 'h' ? ui::fork::ctor(axis::X, grip_width == -1 ? 2 : grip_width, slot1, slot2)
                                   : ui::fork::ctor(axis::Y, grip_width == -1 ? 1 : grip_width, slot1, slot2);
            node->isroot(faux, base::node) // Set object kind to 1 to be different from others. See node_veer::select.
                ->template plugin<pro::focus>()
                ->limits(dot_00)
                ->invoke([&](auto& boss)
                {
                    mouse_subs(boss);
                    boss.on(tier::mouserelease, input::key::MouseWheel, [&](hids& gear)
                    {
                        if (gear.meta(hids::anyCtrl))
                        {
                            boss.move_slider(gear.whlsi);
                            gear.dismiss();
                        }
                    });
                    boss.LISTEN(tier::release, app::tile::events::ui::swap     , gear) { boss.swap();       };
                    boss.LISTEN(tier::release, app::tile::events::ui::rotate   , gear) { boss.rotate();     };
                    boss.LISTEN(tier::release, app::tile::events::ui::equalize , gear) { boss.config(1, 1); };
                    boss.LISTEN(tier::preview, app::tile::events::ui::grips::move, delta)
                    {
                        if (delta)
                        {
                            auto [orientation, griparea, ratio] = boss.get_config();
                            auto step = orientation == axis::X ? delta.x : delta.y;
                            if (step == 0) boss.bell::passover();
                            else           boss.move_slider(step);
                        }
                    };
                    boss.LISTEN(tier::preview, app::tile::events::ui::grips::resize, step)
                    {
                        if (step)
                        {
                            auto [orientation, griparea, ratio] = boss.get_config();
                            auto grip_width = orientation == axis::X ? griparea.size.x : griparea.size.y;
                            boss.set_grip_width(grip_width + step);
                        }
                    };
                });
                auto grip = node->attach(slot::_I, ui::mock::ctor()
                    ->isroot(true)
                    ->active()
                    ->plugin<pro::mouse>()
                    ->plugin<pro::mover>()
                    ->plugin<pro::focus>(pro::focus::mode::focusable)
                    ->plugin<pro::keybd>()
                    ->shader(c3, e2::form::state::focus::count)
                    ->plugin<pro::shade<cell::shaders::xlight>>()
                    ->invoke([&](auto& boss)
                    {
                        boss.on(tier::mouserelease, input::key::RightClick, [&](hids& gear)
                        {
                            boss.base::riseup(tier::preview, e2::form::size::minimize, gear);
                            gear.dismiss();
                        });
                        auto& luafx = boss.bell::indexer.luafx;
                        auto& bindings = *grip_bindings_ptr;
                        input::bindings::keybind(boss, bindings);
                        boss.base::add_methods(basename::grip, //todo self_hosted?
                        {
                            { methods::MoveGrip,        [&]
                                                        {
                                                            auto delta = luafx.get_args_or(1, dot_00);
                                                            boss.base::riseup(tier::preview, app::tile::events::ui::grips::move, delta);
                                                            auto& gear = luafx.get_gear();
                                                            gear.set_handled();
                                                            luafx.set_return();
                                                        }},
                            { methods::ResizeGrip,      [&]
                                                        {
                                                            auto delta = luafx.get_args_or(1, si32{});
                                                            boss.base::riseup(tier::preview, app::tile::events::ui::grips::resize, delta);
                                                            auto& gear = luafx.get_gear();
                                                            gear.set_handled();
                                                            luafx.set_return();
                                                        }},
                            { methods::FocusNextGrip,   [&]
                                                        {
                                                            auto& gear = luafx.get_gear();
                                                            auto ok = gear.is_real();
                                                            if (ok)
                                                            {
                                                                auto delta = luafx.get_args_or(1, si32{ 1 });
                                                                delta > 0 ? boss.base::riseup(tier::preview, app::tile::events::ui::focus::nextgrip, gear)
                                                                          : boss.base::riseup(tier::preview, app::tile::events::ui::focus::prevgrip, gear);
                                                                gear.set_handled();
                                                            }
                                                            luafx.set_return(ok);
                                                        }},
                        });
                    }));
            return node;
        };
        auto empty_slot = []
        {
            auto window_clr = skin::color(tone::window_clr);
            window_clr.bga(0x60);
            auto highlight_color = skin::color(tone::winfocus);
            auto danger_color    = skin::color(tone::danger);
            auto c3 = highlight_color.bga(0x40);
            auto c1 = danger_color;

            using namespace app::shared;
            auto [menu_block, cover, menu_data] = menu::mini(true, faux, 1,
            menu::list
            {
                { menu::item{ .alive = true, .label = "  +  ", .tooltip = " Launch application instance.                            \n"
                                                                          " The app to run can be set by RightClick on the taskbar. " },
                [](auto& boss, auto& /*item*/)
                {
                    boss.on(tier::mouserelease, input::key::LeftClick, [&](hids& gear)
                    {
                        pro::focus::set(boss.This(), gear.id, solo::on);
                        boss.base::riseup(tier::request, e2::form::proceed::createby, gear);
                        gear.dismiss(true);
                    });
                }},
                { menu::item{ .alive = true, .label = "  │  ", .tooltip = " Split horizontally " },
                [](auto& boss, auto& /*item*/)
                {
                    boss.on(tier::mouserelease, input::key::LeftClick, [&](hids& gear)
                    {
                        boss.base::riseup(tier::release, app::tile::events::ui::split::hz, gear);
                        gear.dismiss(true);
                    });
                }},
                { menu::item{ .alive = true, .label = "  ──  ", .tooltip = " Split vertically " },
                [](auto& boss, auto& /*item*/)
                {
                    boss.on(tier::mouserelease, input::key::LeftClick, [&](hids& gear)
                    {
                        boss.base::riseup(tier::release, app::tile::events::ui::split::vt, gear);
                        gear.dismiss(true);
                    });
                }},
                { menu::item{ .alive = true, .label = "  ×  ", .tooltip = " Delete pane ", .hover = c1 },
                [](auto& boss, auto& /*item*/)
                {
                    boss.on(tier::mouserelease, input::key::LeftClick, [&](hids& gear)
                    {
                        boss.base::riseup(tier::release, e2::form::proceed::quit::one, true);
                        gear.dismiss(true);
                    });
                }},
            });
            menu_data->active(window_clr);
            auto menu_id = menu_block->id;
            cover->setpad({ 0, 0, 3, 0 });
            cover->invoke([&](auto& boss)
            {
                boss.LISTEN(tier::release, e2::render::any, parent_canvas, -, (menu_id))
                {
                    parent_canvas.fill([&](cell& c){ c.txt(whitespace).link(menu_id); });
                };
            });

            return ui::cake::ctor()
                ->isroot(true, base::placeholder)
                ->limits(dot_00, -dot_11)
                ->plugin<pro::focus>(pro::focus::mode::focusable)
                ->invoke([&](auto& boss)
                {
                    mouse_subs(boss);
                    auto& default_color = boss.base::field(window_clr).link(boss.id);
                    auto& hilight_color = boss.base::field(highlight_color).alpha(0x70).link(boss.id);
                    auto& current_color = boss.base::field(default_color);
                    boss.shader(current_color)
                        ->shader(c3, e2::form::state::focus::count);
                    auto& highlight = boss.base::field([&](auto state)
                    {
                        current_color = state ? hilight_color : default_color;
                        boss.base::deface();
                    });
                    boss.LISTEN(tier::release, vtm::events::d_n_d::abort, target)
                    {
                        highlight(faux);
                    };
                    boss.LISTEN(tier::release, vtm::events::d_n_d::ask, target)
                    {
                        if (auto parent_ptr = boss.base::parent())
                        if (parent_ptr->base::subset.size() == 1) // Only empty slot available.
                        {
                            highlight(true);
                            target = boss.This();
                        }
                    };
                    boss.LISTEN(tier::release, vtm::events::d_n_d::drop, what)
                    {
                        if (auto parent_ptr = boss.base::parent())
                        if (parent_ptr->base::subset.size() == 1) // Only empty slot available.
                        {
                            highlight(faux);
                            // Solo focus will be set in pro::d_n_d::proceed.
                            //pro::focus::off(boss.back()); // Unset focus from node_veer if it is focused.
                            auto app = app_window(what);
                            parent_ptr->attach(app);
                            app->base::broadcast(tier::anycast, e2::form::upon::started);
                            app->base::reflow();
                        }
                    };
                    boss.on(tier::mouserelease, input::key::RightClick, [&](hids& gear)
                    {
                        pro::focus::set(boss.This(), gear.id, solo::on);
                        boss.base::riseup(tier::request, e2::form::proceed::createby, gear);
                        gear.dismiss(true);
                    });
                })
                ->branch
                (
                    ui::post::ctor()->upload("Empty Slot", 10)
                        ->limits({ 10, 1 }, { 10, 1 })
                        ->alignment({ snap::center, snap::center })
                )
                ->branch
                (
                    menu_block->alignment({ snap::head, snap::head })
                );
        };
        auto node_veer = [](auto&& node_veer, auto min_state, auto grip_bindings_ptr) -> netxs::sptr<ui::veer>
        {
            return ui::veer::ctor()
                ->plugin<pro::focus>()
                ->active()
                ->invoke([&](auto& boss)
                {
                    boss.LISTEN(tier::release, e2::config::plugins::sizer::alive, state)
                    {
                        // Block a rising up of this event: dtvt object fires this event on exit.
                    };
                    boss.LISTEN(tier::release, e2::form::proceed::swap, item_ptr)
                    {
                        if (boss.count() == 1) // Only empty slot available.
                        {
                            if constexpr (debugmode) log(prompt::tile, "Empty slot swap: defective structure, count=", boss.count());
                        }
                        else if (boss.count() == 2)
                        {
                            auto gear_id_list = pro::focus::cut(boss.back());
                            auto deleted_item = boss.pop_back();
                            if (item_ptr)
                            {
                                boss.attach(item_ptr);
                                item_ptr->base::broadcast(tier::anycast, e2::form::upon::started);
                                if (item_ptr->base::kind() == base::client) // Restore side list item (it was deleted on detach).
                                {
                                    item_ptr->base::riseup(tier::release, tile::events::enlist, item_ptr);
                                }
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
                                    auto gear_id_list = pro::focus::cut(boss.back());
                                    item_ptr = boss.pop_back();
                                    pro::focus::set(boss.back(), gear_id_list, solo::off);
                                }
                                else
                                {
                                    if constexpr (debugmode) log(prompt::tile, "Empty slot: defective structure, count=", boss.count());
                                }
                                if (auto parent = boss.base::parent())
                                {
                                    parent->bell::expire();
                                }
                            }
                        };
                    };
                    boss.LISTEN(tier::preview, e2::form::size::minimize, gear, -, (saved_ratio = 1, min_ratio = 1, min_state))
                    {
                        if (boss.count() > 2) // Restore if maximized.
                        {
                            boss.back()->base::signal(tier::release, e2::form::size::restore);
                        }
                        else if (auto node = std::static_pointer_cast<ui::fork>(boss.base::parent()))
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
                    boss.LISTEN(tier::preview, e2::form::size::enlarge::any, gear, -, (oneoff = subs{}))
                    {
                        pro::focus::set(boss.This(), gear.id, solo::off);
                        if (boss.count() > 2 || oneoff.size()) // It is a root or is already maximized. See build_inst::slot::_2's e2::form::proceed::attach for details.
                        {
                            boss.base::riseup(tier::release, e2::form::proceed::attach);
                        }
                        else
                        {
                            if (boss.count() > 1) // Preventing the empty slot from maximizing.
                            if (boss.back()->base::kind() == base::client) // Preventing the splitter from maximizing.
                            {
                                auto fullscreen_item = boss.back();
                                auto& fullscreen_inst = *fullscreen_item;
                                boss.base::riseup(tier::release, e2::form::proceed::attach, fullscreen_item);
                                fullscreen_item->LISTEN(tier::release, e2::form::size::restore, p, oneoff)
                                {
                                    auto item_ptr = fullscreen_inst.This();
                                    auto gear_id_list = pro::focus::cut(item_ptr);
                                    item_ptr->base::detach();
                                    boss.attach(item_ptr);
                                    item_ptr->base::broadcast(tier::anycast, e2::form::upon::started);
                                    pro::focus::set(item_ptr, gear_id_list, solo::off);
                                    boss.base::reflow();
                                    oneoff.clear();
                                };
                                fullscreen_item->LISTEN(tier::release, e2::form::upon::vtree::detached, parent_ptr, oneoff)
                                {
                                    oneoff.clear();
                                };
                                boss.base::reflow();
                            }
                        }
                    };
                    boss.LISTEN(tier::release, app::tile::events::ui::split::any, gear, -, (grip_bindings_ptr))
                    {
                        auto deed = boss.bell::protos();
                        auto depth = 0;
                        auto parent_ptr = boss.base::parent();
                        while (parent_ptr)
                        {
                            depth++;
                            parent_ptr = parent_ptr->base::parent();
                        }
                        if constexpr (debugmode) log(prompt::tile, "Depth ", depth);
                        if (depth > inheritance_limit) return;

                        auto heading = deed == app::tile::events::ui::split::vt.id;
                        auto newnode = build_node(heading ? 'v':'h', 1, 1, heading ? 1 : 2, grip_bindings_ptr);
                        auto empty_1 = node_veer(node_veer, ui::fork::min_ratio, grip_bindings_ptr);
                        auto empty_2 = node_veer(node_veer, ui::fork::max_ratio, grip_bindings_ptr);
                        auto gear_id_list = pro::focus::cut(boss.back());
                        auto curitem = boss.pop_back();
                        if (boss.empty())
                        {
                            boss.attach(empty_slot());
                            empty_1->pop_back();
                        }
                        auto slot_1 = newnode->attach(slot::_1, empty_1->branch(curitem));
                        auto slot_2 = newnode->attach(slot::_2, empty_2);
                        boss.attach(newnode);
                        newnode->base::broadcast(tier::anycast, e2::form::upon::started);
                        pro::focus::set(slot_1->back(), gear_id_list, solo::off); // Handover all foci.
                        pro::focus::set(slot_2->back(), gear_id_list, solo::off);
                        if (curitem->base::kind() == base::client) // Restore side list item (it was deleted on detach).
                        {
                            curitem->base::riseup(tier::release, tile::events::enlist, curitem);
                        }
                    };
                    boss.LISTEN(tier::anycast, e2::form::proceed::quit::any, fast)
                    {
                        boss.base::signal(tier::preview, e2::form::proceed::quit::one, fast);
                    };
                    boss.LISTEN(tier::preview, e2::form::proceed::quit::one, fast)
                    {
                        if (boss.count() > 1 && boss.back()->base::root()) // Walking a nested visual tree.
                        {
                            boss.back()->base::signal(tier::anycast, e2::form::proceed::quit::one, true); // fast=true: Immediately closing (no ways to showing a closing process). Forward a quit message to hosted app in order to schedule a cleanup.
                        }
                        else // Close an empty slot (boss.count() == 1).
                        {
                            boss.base::enqueue([&](auto& /*boss*/) // Enqueue to keep the focus tree intact while processing events.
                            {
                                boss.base::signal(tier::release, e2::form::proceed::quit::one, fast);
                            });
                        }
                    };
                    boss.LISTEN(tier::release, e2::form::proceed::quit::any, fast)
                    {
                        if (auto parent = boss.base::parent())
                        {
                            if (boss.count() > 1 && boss.back()->base::kind() == base::client) // Only apps can be deleted.
                            {
                                auto gear_id_list = pro::focus::cut(boss.back());
                                auto deleted_item = boss.pop_back(); // Throw away.
                                pro::focus::set(boss.back(), gear_id_list, solo::off);
                            }
                            else if (boss.count() == 1) // Remove empty slot, reorganize.
                            {
                                auto item_ptr = parent->base::signal(tier::request, e2::form::proceed::swap, boss.This()); // sptr must be of the same type as the event argument. Casting kills all intermediaries when return.
                                if (item_ptr != boss.This()) // Parallel slot is not empty or both slots are empty (item_ptr == null).
                                {
                                    parent->base::riseup(tier::release, e2::form::proceed::swap, item_ptr);
                                }
                            }
                            boss.base::broadcast(tier::anycast, e2::form::upon::started);
                            boss.base::deface();
                            boss.base::reflow();
                        }
                    };
                    boss.LISTEN(tier::request, e2::form::proceed::createby, gear)
                    {
                        if (boss.count() != 1) return; // Create new apps at the empty slots only.
                        auto& gate = gear.owner;
                        auto& current_default = gate.base::property("desktop.selected");
                        if (auto world_ptr = boss.base::signal(tier::general, e2::config::creator)) // Finalize app creation.
                        {
                            auto what = world_ptr->base::signal(tier::request, vtm::events::apptype, { .menuid = current_default });
                            if (what.type == netxs::app::site::id) return; // Deny any desktop viewport markers inside the tiling manager.
                            world_ptr->base::signal(tier::request, vtm::events::newapp, what);
                            auto app = app_window(what);
                            pro::focus::off(boss.back());
                            boss.attach(app);
                            app->base::signal(tier::anycast, vtm::events::attached, world_ptr);
                            auto root_ptr = what.applet;
                            app->base::broadcast(tier::anycast, e2::form::upon::started, root_ptr);
                            pro::focus::set(app, gear.id, solo::off);
                        }
                    };
                    //todo unify, demo limits
                    //static auto insts_count = 0;
                    //insts_count++;
                    //boss.LISTEN(tier::release, e2::form::upon::vtree::detached, parent_ptr)
                    //{
                    //    insts_count--;
                    //    if constexpr (debugmode) log(prompt::tile, "Instance detached: id:", id, "; left:", insts_count);
                    //};
                })
                ->branch(empty_slot());
        };
        auto parse_data = [](auto&& parse_data, view& utf8, auto min_ratio, auto grip_bindings_ptr) -> netxs::sptr<ui::veer>
        {
            auto slot_ptr = node_veer(node_veer, min_ratio, grip_bindings_ptr);
            utf::trim_front(utf8, ", ");
            if (utf8.empty()) return slot_ptr;
            auto tag = utf8.front();
            if ((tag == 'h' || tag == 'v') && utf8.find('(') < utf8.find(','))
            {
                // add split
                utf8.remove_prefix(1);
                utf::trim_front(utf8, ' ');
                auto s1 = si32{ 1 };
                auto s2 = si32{ 1 };
                auto w  = si32{-1 };
                if (auto l = utf::to_int(utf8)) // Left side ratio
                {
                    s1 = std::abs(l.value());
                    if (utf8.empty() || utf8.front() != ':') return slot_ptr;
                    utf8.remove_prefix(1);
                    if (auto r = utf::to_int(utf8)) // Right side ratio
                    {
                        s2 = std::abs(r.value());
                        utf::trim_front(utf8, ' ');
                        if (!utf8.empty() && utf8.front() == ':') // Grip width.
                        {
                            utf8.remove_prefix(1);
                            if (auto g = utf::to_int(utf8))
                            {
                                w = std::abs(g.value());
                                utf::trim_front(utf8, ' ');
                            }
                        }
                    }
                    else return slot_ptr;
                }
                if (utf8.empty() || utf8.front() != '(') return slot_ptr;
                utf8.remove_prefix(1);
                auto node = build_node(tag, s1, s2, w, grip_bindings_ptr);
                auto slot1 = node->attach(slot::_1, parse_data(parse_data, utf8, ui::fork::min_ratio, grip_bindings_ptr));
                auto slot2 = node->attach(slot::_2, parse_data(parse_data, utf8, ui::fork::max_ratio, grip_bindings_ptr));
                slot_ptr->attach(node);
                utf::trim_front(utf8, ") ");
            }
            else  // Add application.
            {
                utf::trim_front(utf8, ' ');
                auto menuid = utf::take_front(utf8, " ,)").str();
                if (menuid.empty()) return slot_ptr;

                utf::trim_front(utf8, " ,");
                if (utf8.size() && utf8.front() == ')') utf8.remove_prefix(1); // pop ')';

                auto& s = *slot_ptr;
                auto& oneshot = s.base::field(hook{});
                s.LISTEN(tier::anycast, vtm::events::attached, world_ptr, oneshot, (menuid))
                {
                    auto what = world_ptr->base::signal(tier::request, vtm::events::newapp, { .menuid = menuid });
                    auto inst_ptr = app_window(what);
                    s.attach(inst_ptr);
                    inst_ptr->base::signal(tier::anycast, vtm::events::attached, world_ptr);
                    s.base::unfield(oneshot);
                };
            }
            return slot_ptr;
        };
        namespace item_type
        {
            static constexpr auto _counter   = __COUNTER__ + 1;
            static constexpr auto empty_slot = __COUNTER__ - _counter;
            static constexpr auto applet     = __COUNTER__ - _counter;
            static constexpr auto grip       = __COUNTER__ - _counter;
        }
        auto _foreach = [](auto _foreach, sptr& root_veer_ptr, id_t gear_id, auto proc) -> void
        {
            if (auto node_veer_ptr = std::static_pointer_cast<ui::veer>(root_veer_ptr))
            {
                if (pro::focus::is_focused(node_veer_ptr, gear_id))
                {
                    auto item_ptr = node_veer_ptr->back();
                    if (node_veer_ptr->count() == 1) // Empty slot.
                    {
                        if (pro::focus::is_focused(item_ptr, gear_id))
                        {
                            proc(item_ptr, item_type::empty_slot, node_veer_ptr);
                            if (!item_ptr)
                            {
                                root_veer_ptr = {}; // Interrupt foreach.
                            }
                        }
                    }
                    else if (item_ptr->root()) // App window (ui::fork).
                    {
                        if (auto applet_host_ptr = std::static_pointer_cast<ui::fork>(item_ptr))
                        if (auto applet_ptr = applet_host_ptr->get(slot::_2))
                        if (pro::focus::is_focused(applet_ptr, gear_id))
                        {
                            proc(applet_ptr, item_type::applet, node_veer_ptr); // Applet.
                            if (!applet_ptr)
                            {
                                root_veer_ptr = {}; // Interrupt foreach.
                            }
                        }
                    }
                    else // if (!item_ptr->root()) // Node (ui::fork).
                    {
                        if (auto veer_host_ptr = std::static_pointer_cast<ui::fork>(item_ptr))
                        {
                            root_veer_ptr = veer_host_ptr->get(slot::_1);
                            _foreach(_foreach, root_veer_ptr, gear_id, proc);
                            if (!root_veer_ptr) return;
                            if (auto grip_ptr = veer_host_ptr->get(slot::_I))
                            {
                                if (pro::focus::is_focused(grip_ptr, gear_id))
                                {
                                    proc(grip_ptr, item_type::grip, node_veer_ptr); // Grip.
                                    if (!grip_ptr)
                                    {
                                        root_veer_ptr = {}; // Interrupt foreach.
                                        return;
                                    }
                                }
                            }
                            root_veer_ptr = veer_host_ptr->get(slot::_2);
                            _foreach(_foreach, root_veer_ptr, gear_id, proc);
                            if (!root_veer_ptr) return;
                        }
                    }
                }
            }
        };
        auto build_inst = [](eccc appcfg, settings& config) -> sptr
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
            //auto danger_color    = skin::color(tone::danger);
            //auto warning_color   = skin::color(tone::warning);
            //auto c3 = highlight_color;
            //auto c2 = warning_color;
            //auto c1 = danger_color;

            auto object = ui::fork::ctor(axis::Y)
                ->plugin<items>()
                ->plugin<pro::focus>()
                ->plugin<pro::keybd>();
            using namespace app::shared;
            auto tile_context = config.settings::push_context("/config/events/tile/grip/");
            auto script_list = config.settings::take_ptr_list_for_name("script");
            auto grip_bindings_ptr = ptr::shared(input::bindings::load(config, script_list));
            //config.settings::pop_context();
            tile_context = config.settings::push_context("/config/tile/");
            auto [menu_block, cover, menu_data] = menu::load(config);
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
            auto root_veer_ptr = object->attach(slot::_2, parse_data(parse_data, param, ui::fork::min_ratio, grip_bindings_ptr))
                ->invoke([&](auto& boss)
                {
                    boss.LISTEN(tier::release, e2::form::proceed::attach, fullscreen_item)
                    {
                        if (boss.count() > 2)
                        {
                            boss.back()->base::signal(tier::release, e2::form::size::restore);
                        }
                        if (fullscreen_item)
                        {
                            auto gear_id_list = pro::focus::cut(fullscreen_item);
                            fullscreen_item->base::detach();
                            pro::focus::off(boss.This());
                            boss.attach(fullscreen_item);
                            fullscreen_item->base::broadcast(tier::anycast, e2::form::upon::started);
                            pro::focus::set(fullscreen_item, gear_id_list, solo::off);
                        }
                    };
                });
            object->invoke([&](auto& boss)
                {
                    auto& root_veer = *root_veer_ptr;
                    auto& foreach = boss.base::field([&](id_t gear_id, auto proc)
                    {
                        auto root_veer_ptr = root_veer.base::This();
                        _foreach(_foreach, root_veer_ptr, gear_id, proc);
                    });
                    auto& nothing_to_iterate = boss.base::field([&]
                    {
                        return root_veer.back()->root();
                    });
                    auto& oneshot = boss.base::field(hook{});
                    boss.LISTEN(tier::anycast, e2::form::upon::created, gear, oneshot)
                    {
                        auto& gate = gear.owner;
                        auto& current_default = gate.base::property("desktop.selected");
                        auto world_ptr = boss.base::signal(tier::general, e2::config::creator);
                        auto conf_list_ptr = world_ptr->base::signal(tier::request, desk::events::menu);
                        auto& conf_list = *conf_list_ptr;
                        auto& config = conf_list[current_default];
                        if (config.type == app::tile::id) // Reset the currently selected application to the previous one.
                        {
                            auto& previous_default = gate.base::property("desktop.prev_selected");
                            current_default = previous_default;
                            gate.base::signal(tier::release, e2::data::changed, previous_default); // Signal to update UI.
                        }
                        boss.base::unfield(oneshot);
                    };
                    boss.LISTEN(tier::anycast, e2::form::upon::started, root_ptr)
                    {
                        if (root_ptr)
                        if (auto world_ptr = boss.base::signal(tier::general, e2::config::creator))
                        {
                            boss.base::signal(tier::anycast, vtm::events::attached, world_ptr);
                        }
                    };
                    boss.LISTEN(tier::request, e2::form::prop::window::state, state)
                    {
                        state = winstate::tiled;
                    };
                    boss.LISTEN(tier::preview, e2::form::prop::cwd, path)
                    {
                        boss.base::signal(tier::anycast, e2::form::prop::cwd, path);
                    };
                    boss.LISTEN(tier::request, e2::form::proceed::swap, item_ptr) // Close the tile window manager if we receive a `swap-request` from the top-level `empty-slot`.
                    {
                        boss.base::riseup(tier::release, e2::form::proceed::quit::one, true);
                    };
                    auto& luafx = boss.bell::indexer.luafx;
                    tile_context = config.settings::push_context("/config/events/tile/");
                    auto script_list = config.settings::take_ptr_list_for_name("script");
                    auto bindings = input::bindings::load(config, script_list);
                    input::bindings::keybind(boss, bindings);
                    boss.base::add_methods(basename::tile,
                    {
                        { methods::FocusNextPaneOrGrip, [&]
                                                        {
                                                            luafx.run_with_gear([&](auto& gear)
                                                            {
                                                                auto dir = luafx.get_args_or(1, si32{ 1 });
                                                                dir < 0 ? boss.base::signal(tier::preview, app::tile::events::ui::focus::prev, gear)
                                                                        : boss.base::signal(tier::preview, app::tile::events::ui::focus::next, gear);
                                                            });
                                                        }},
                        { methods::FocusNextPane,       [&]
                                                        {
                                                            luafx.run_with_gear([&](auto& gear)
                                                            {
                                                                auto dir = luafx.get_args_or(1, si32{ 1 });
                                                                dir < 0 ? boss.base::signal(tier::preview, app::tile::events::ui::focus::prevpane, gear)
                                                                        : boss.base::signal(tier::preview, app::tile::events::ui::focus::nextpane, gear);
                                                            });
                                                        }},
                        { methods::FocusNextGrip,       [&]
                                                        {
                                                            luafx.run_with_gear([&](auto& gear)
                                                            {
                                                                auto dir = luafx.get_args_or(1, si32{ 1 });
                                                                dir < 0 ? boss.base::signal(tier::preview, app::tile::events::ui::focus::prevgrip, gear)
                                                                        : boss.base::signal(tier::preview, app::tile::events::ui::focus::nextgrip, gear);
                                                            });
                                                        }},
                        { methods::RunApplication,      [&]
                                                        {
                                                            luafx.run_with_gear([&](auto& gear)
                                                            {
                                                                //todo add agrs
                                                                //auto dir = luafx.get_args_or(1, si32{ 1 });
                                                                boss.base::signal(tier::preview, app::tile::events::ui::create, gear);
                                                            });
                                                        }},
                        { methods::SelectAllPanes,      [&]
                                                        {
                                                            luafx.run_with_gear([&](auto& gear)
                                                            {
                                                                boss.base::signal(tier::preview, app::tile::events::ui::select, gear);
                                                            });
                                                        }},
                        { methods::SplitPane,           [&]
                                                        {
                                                            luafx.run_with_gear([&](auto& gear)
                                                            {
                                                                auto dir = luafx.get_args_or(1, si32{ 1 });
                                                                dir > 0 ? boss.base::signal(tier::preview, app::tile::events::ui::split::vt, gear)
                                                                        : boss.base::signal(tier::preview, app::tile::events::ui::split::hz, gear);
                                                            });
                                                        }},
                        { methods::RotateSplit,         [&]
                                                        {
                                                            luafx.run_with_gear([&](auto& gear)
                                                            {
                                                                boss.base::signal(tier::preview, app::tile::events::ui::rotate, gear);
                                                            });
                                                        }},
                        { methods::SwapPanes,           [&]
                                                        {
                                                            luafx.run_with_gear([&](auto& gear)
                                                            {
                                                                boss.base::signal(tier::preview, app::tile::events::ui::swap, gear);
                                                            });
                                                        }},
                        { methods::EqualizeSplitRatio,  [&]
                                                        {
                                                            luafx.run_with_gear([&](auto& gear)
                                                            {
                                                                boss.base::signal(tier::preview, app::tile::events::ui::equalize, gear);
                                                            });
                                                        }},
                        { methods::SetTitle,            [&]
                                                        {
                                                            luafx.run_with_gear([&](auto& gear)
                                                            {
                                                                boss.base::signal(tier::preview, app::tile::events::ui::title, gear);
                                                            });
                                                        }},
                        { methods::ClosePane,           [&]
                                                        {
                                                            luafx.run_with_gear([&](auto& gear)
                                                            {
                                                                boss.base::signal(tier::preview, app::tile::events::ui::close, gear);
                                                            });
                                                        }},
                    });

                    boss.LISTEN(tier::preview, app::tile::events::ui::any, gear)
                    {
                        if (root_veer.count() > 2)
                        {
                            root_veer.base::riseup(tier::release, e2::form::proceed::attach); // Restore the window before any action if maximized.
                        }
                    };
                    auto& switch_counter = boss.base::field(std::unordered_map<id_t, feed>{});
                    boss.LISTEN(tier::release, input::events::focus::set::any, seed) // Reset the focus switch counter when it is focused from outside.
                    {
                        switch_counter[seed.gear_id] = {};
                    };
                    //todo generalize refocusing
                    boss.LISTEN(tier::preview, app::tile::events::ui::focus::prev, gear)
                    {
                        if (nothing_to_iterate()) return;
                        auto prev_item_ptr = sptr{};
                        auto next_item_ptr = sptr{};
                        foreach(id_t{}, [&](auto& item_ptr, si32 /*item_type*/, auto)
                        {
                            if (pro::focus::is_focused(item_ptr, gear.id))
                            {
                                prev_item_ptr = next_item_ptr;
                            }
                            next_item_ptr = item_ptr;
                        });
                        auto skip = !prev_item_ptr && gear.shared_event && switch_counter[gear.id] == feed::rev;
                        if (skip) // Give another process a chance to handle this event.
                        {
                            switch_counter[gear.id] = {};
                        }
                        else
                        {
                            if (!prev_item_ptr) // Focused item is at the boundary.
                            {
                                prev_item_ptr = next_item_ptr;
                            }
                            if (prev_item_ptr)
                            {
                                auto& prev_item = *prev_item_ptr;
                                prev_item.base::enqueue([&, gear_id = gear.id](auto& /*boss*/) // Keep the focus tree intact while processing events.
                                {
                                    pro::focus::set(prev_item.This(), gear_id, solo::on);
                                });
                                gear.set_handled();
                                switch_counter[gear.id] = feed::rev;
                            }
                        }
                    };
                    boss.LISTEN(tier::preview, app::tile::events::ui::focus::next, gear)
                    {
                        if (nothing_to_iterate()) return;
                        auto prev_item_ptr = sptr{};
                        auto next_item_ptr = sptr{};
                        auto temp_item_ptr = sptr{};
                        foreach(id_t{}, [&](auto& item_ptr, si32 /*item_type*/, auto)
                        {
                            if (!temp_item_ptr)
                            {
                                temp_item_ptr = item_ptr; // Fallback item.
                            }
                            if (prev_item_ptr)
                            {
                                std::swap(next_item_ptr, item_ptr); // Interrupt foreach (empty item_ptr).
                            }
                            else if (pro::focus::is_focused(item_ptr, gear.id))
                            {
                                prev_item_ptr = item_ptr;
                            }
                        });
                        auto skip = !next_item_ptr && gear.shared_event && switch_counter[gear.id] == feed::fwd;
                        if (skip) // Give another process a chance to handle this event.
                        {
                            switch_counter[gear.id] = {};
                        }
                        else
                        {
                            if (!next_item_ptr) // Focused item is at the boundary.
                            {
                                next_item_ptr = temp_item_ptr;
                            }
                            if (next_item_ptr)
                            {
                                auto& next_item = *next_item_ptr;
                                next_item.base::enqueue([&, gear_id = gear.id](auto& /*boss*/) // Keep the focus tree intact while processing events.
                                {
                                    pro::focus::set(next_item.This(), gear_id, solo::on);
                                });
                                gear.set_handled();
                                switch_counter[gear.id] = feed::fwd;
                            }
                        }
                    };
                    boss.LISTEN(tier::preview, app::tile::events::ui::focus::prevpane, gear)
                    {
                        if (nothing_to_iterate()) return;
                        auto prev_item_ptr = sptr{};
                        auto next_item_ptr = sptr{};
                        foreach(id_t{}, [&](auto& item_ptr, si32 item_type, auto)
                        {
                            if (item_type != item_type::grip)
                            {
                                if (pro::focus::is_focused(item_ptr, gear.id))
                                {
                                    prev_item_ptr = next_item_ptr;
                                }
                                next_item_ptr = item_ptr;
                            }
                        });
                        auto skip = !prev_item_ptr && gear.shared_event && switch_counter[gear.id] == feed::rev;
                        if (skip) // Give another process a chance to handle this event.
                        {
                            switch_counter[gear.id] = {};
                        }
                        else
                        {
                            if (!prev_item_ptr) // Focused item is at the boundary.
                            {
                                prev_item_ptr = next_item_ptr;
                            }
                            if (prev_item_ptr)
                            {
                                auto& prev_item = *prev_item_ptr;
                                prev_item.base::enqueue([&, gear_id = gear.id](auto& /*boss*/) // Keep the focus tree intact while processing events.
                                {
                                    pro::focus::set(prev_item.This(), gear_id, solo::on);
                                });
                                gear.set_handled();
                                switch_counter[gear.id] = feed::rev;
                            }
                        }
                    };
                    boss.LISTEN(tier::preview, app::tile::events::ui::focus::nextpane, gear)
                    {
                        if (nothing_to_iterate()) return;
                        auto prev_item_ptr = sptr{};
                        auto next_item_ptr = sptr{};
                        auto temp_item_ptr = sptr{};
                        foreach(id_t{}, [&](auto& item_ptr, si32 item_type, auto)
                        {
                            if (item_type != item_type::grip)
                            {
                                if (!temp_item_ptr)
                                {
                                    temp_item_ptr = item_ptr; // Fallback item.
                                }
                                if (prev_item_ptr)
                                {
                                    std::swap(next_item_ptr, item_ptr); // Interrupt foreach (empty item_ptr).
                                }
                                else if (pro::focus::is_focused(item_ptr, gear.id))
                                {
                                    prev_item_ptr = item_ptr;
                                }
                            }
                        });
                        auto skip = !next_item_ptr && gear.shared_event && switch_counter[gear.id] == feed::fwd;
                        if (skip) // Give another process a chance to handle this event.
                        {
                            switch_counter[gear.id] = {};
                        }
                        else
                        {
                            if (!next_item_ptr) // Focused item is at the boundary.
                            {
                                next_item_ptr = temp_item_ptr;
                            }
                            if (next_item_ptr)
                            {
                                auto& next_item = *next_item_ptr;
                                next_item.base::enqueue([&, gear_id = gear.id](auto& /*boss*/) // Keep the focus tree intact while processing events.
                                {
                                    pro::focus::set(next_item.This(), gear_id, solo::on);
                                });
                                gear.set_handled();
                                switch_counter[gear.id] = feed::fwd;
                            }
                        }
                    };
                    boss.LISTEN(tier::preview, app::tile::events::ui::focus::prevgrip, gear)
                    {
                        if (nothing_to_iterate()) return;
                        auto prev_grip_ptr = sptr{};
                        auto next_grip_ptr = sptr{};
                        foreach(id_t{}, [&](auto& item_ptr, si32 item_type, auto)
                        {
                            if (item_type == item_type::grip)
                            {
                                if (pro::focus::is_focused(item_ptr, gear.id))
                                {
                                    prev_grip_ptr = next_grip_ptr;
                                }
                                next_grip_ptr = item_ptr;
                            }
                        });
                        if (!prev_grip_ptr) // Focused grip is at the boundary.
                        {
                            prev_grip_ptr = next_grip_ptr;
                        }
                        if (prev_grip_ptr)
                        {
                            auto& prev_grip = *prev_grip_ptr;
                            prev_grip.base::enqueue([&, gear_id = gear.id](auto& /*boss*/) // Keep the focus tree intact while processing events.
                            {
                                pro::focus::set(prev_grip.This(), gear_id, solo::on);
                            });
                            gear.set_handled();
                        }
                    };
                    boss.LISTEN(tier::preview, app::tile::events::ui::focus::nextgrip, gear)
                    {
                        if (nothing_to_iterate()) return;
                        auto prev_grip_ptr = sptr{};
                        auto next_grip_ptr = sptr{};
                        auto temp_grip_ptr = sptr{};
                        foreach(id_t{}, [&](auto& item_ptr, si32 item_type, auto)
                        {
                            if (item_type == item_type::grip)
                            {
                                if (!temp_grip_ptr)
                                {
                                    temp_grip_ptr = item_ptr; // Fallback item.
                                }
                                if (prev_grip_ptr)
                                {
                                    std::swap(next_grip_ptr, item_ptr); // Interrupt foreach (empty item_ptr).
                                }
                                else if (pro::focus::is_focused(item_ptr, gear.id))
                                {
                                    prev_grip_ptr = item_ptr;
                                }
                            }
                        });
                        if (!next_grip_ptr) // Focused item is at the boundary.
                        {
                            next_grip_ptr = temp_grip_ptr;
                        }
                        if (next_grip_ptr)
                        {
                            auto& next_grip = *next_grip_ptr;
                            next_grip.base::enqueue([&, gear_id = gear.id](auto& /*boss*/) // Keep the focus tree intact while processing events.
                            {
                                pro::focus::set(next_grip.This(), gear_id, solo::on);
                            });
                            gear.set_handled();
                        }
                    };
                    boss.LISTEN(tier::preview, app::tile::events::ui::swap, gear)
                    {
                        if (nothing_to_iterate()) return;
                        auto node_veer_list = std::vector<netxs::sptr<ui::veer>>{};
                        auto node_grip_list = std::vector<netxs::sptr<ui::veer>>{};
                        foreach(gear.id, [&](auto& /*item_ptr*/, si32 item_type, auto node_veer_ptr)
                        {
                            if (item_type == item_type::grip)
                            {
                                node_grip_list.push_back(node_veer_ptr);
                            }
                            else
                            {
                                node_veer_list.push_back(node_veer_ptr);
                            }
                        });
                        auto slots_count = node_veer_list.size();
                        if (slots_count == 1) // Swap panes in split.
                        {
                            node_veer_list.front()->back()->base::riseup(tier::release, app::tile::events::ui::swap, gear);
                        }
                        else if (slots_count)// Swap selected panes cyclically.
                        {
                            auto emp_slot = sptr{};
                            auto app_slot = sptr{};
                            auto emp_next = sptr{};
                            auto app_next = sptr{};
                            for (auto& s : node_veer_list)
                            {
                                if (s->count() == 1) // empty only
                                {
                                    app_next.reset();
                                    pro::focus::cut(s->back());
                                    emp_next = s->pop_back();
                                }
                                else if (s->count() == 2) // empty + app
                                {
                                    if (auto app = s->back())
                                    {
                                        app->base::signal(tier::release, tile::events::delist, true);
                                    }
                                    pro::focus::cut(s->back());
                                    app_next = s->pop_back();
                                    pro::focus::cut(s->back());
                                    emp_next = s->pop_back();
                                }
                                if (emp_slot)
                                {
                                    s->attach(emp_slot);
                                    if (!app_slot) pro::focus::set(emp_slot, gear.id, solo::off); // Refocus.
                                }
                                if (app_slot)
                                {
                                    s->attach(app_slot);
                                    pro::focus::set(app_slot, gear.id, solo::off); // Refocus.
                                    app_slot->base::riseup(tier::release, tile::events::enlist, app_slot);
                                }
                                std::swap(emp_slot, emp_next);
                                std::swap(app_slot, app_next);
                            }
                            auto& first_item_ptr = node_veer_list.front();
                            if (emp_slot)
                            {
                                first_item_ptr->attach(emp_slot);
                                if (!app_slot) pro::focus::set(emp_slot, gear.id, solo::off); // Refocus.
                            }
                            if (app_slot)
                            {
                                first_item_ptr->attach(app_slot);
                                pro::focus::set(app_slot, gear.id, solo::off); // Refocus.
                                app_slot->base::riseup(tier::release, tile::events::enlist, app_slot);
                            }
                        }
                        for (auto& s : node_grip_list) // Swap panes in split.
                        {
                            s->back()->base::riseup(tier::release, app::tile::events::ui::swap, gear);
                        }
                        boss.base::broadcast(tier::anycast, e2::form::upon::started);
                        gear.set_handled();
                    };
                    boss.LISTEN(tier::preview, app::tile::events::ui::create, gear)
                    {
                        foreach(gear.id, [&](auto& item_ptr, si32 item_type, auto)
                        {
                            if (item_type == item_type::empty_slot)
                            {
                                item_ptr->base::riseup(tier::request, e2::form::proceed::createby, gear);
                                gear.set_handled();
                            }
                        });
                    };
                    boss.LISTEN(tier::preview, app::tile::events::ui::select, gear)
                    {
                        foreach(id_t{}, [&](auto& item_ptr, si32 item_type, auto)
                        {
                            if (item_type != item_type::grip) pro::focus::set(item_ptr, gear.id, solo::off);
                            else                              pro::focus::off(item_ptr, gear.id);
                        });
                        gear.set_handled();
                    };
                    boss.LISTEN(tier::preview, app::tile::events::ui::split::any, gear)
                    {
                        auto deed = boss.bell::protos();
                        foreach(gear.id, [&](auto& item_ptr, si32 /*item_type*/, auto node_veer_ptr)
                        {
                            auto room = node_veer_ptr->base::size() / 3;
                            if (room.x && room.y) // Suppress split if there is no space.
                            {
                                boss.base::enqueue([&, deed, gear_id = gear.id, item_wptr = ptr::shadow(item_ptr)](auto& /*boss*/) // Enqueue to keep the focus tree intact while processing events.
                                {
                                    if (auto gear_ptr = boss.base::template getref<hids>(gear_id))
                                    if (auto item_ptr = item_wptr.lock())
                                    {
                                        item_ptr->base::raw_riseup(tier::release, deed, *gear_ptr);
                                    }
                                });
                            }
                            gear.set_handled();
                        });
                    };
                    boss.LISTEN(tier::preview, app::tile::events::ui::rotate, gear)
                    {
                        foreach(gear.id, [&](auto& item_ptr, si32 /*item_type*/, auto)
                        {
                            item_ptr->base::riseup(tier::release, app::tile::events::ui::rotate, gear);
                            gear.set_handled();
                        });
                    };
                    boss.LISTEN(tier::preview, app::tile::events::ui::equalize, gear)
                    {
                        foreach(gear.id, [&](auto& item_ptr, si32 /*item_type*/, auto)
                        {
                            item_ptr->base::riseup(tier::release, app::tile::events::ui::equalize, gear);
                            gear.set_handled();
                        });
                    };
                    boss.LISTEN(tier::preview, app::tile::events::ui::title , gear)
                    {
                        app::shared::set_title(boss, gear);
                        gear.set_handled();
                    };
                    boss.LISTEN(tier::preview, app::tile::events::ui::close, gear)
                    {
                        foreach(gear.id, [&](auto& item_ptr, si32 item_type, auto)
                        {
                            if (item_type != item_type::grip)
                            {
                                item_ptr->base::riseup(tier::preview, e2::form::proceed::quit::one, true);
                                gear.set_handled();
                            }
                        });
                    };
                });
            return object;
        };
    }

    app::shared::initialize builder{ app::tile::id, build_inst };
}