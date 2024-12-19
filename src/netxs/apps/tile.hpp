// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

namespace netxs::events::userland
{
    struct tile
    {
        EVENTPACK( tile, ui::e2::extra::slot4 )
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
        X(TileFocusPrev         ) \
        X(TileFocusNext         ) \
        X(TileFocusPrevPane     ) \
        X(TileFocusNextPane     ) \
        X(TileFocusPrevGrip     ) \
        X(TileFocusNextGrip     ) \
        X(TileRunApplicatoin    ) \
        X(TileSelectAllPanes    ) \
        X(TileSplitHorizontally ) \
        X(TileSplitVertically   ) \
        X(TileSplitOrientation  ) \
        X(TileSwapPanes         ) \
        X(TileEqualizeSplitRatio) \
        X(TileSetManagerTitle   ) \
        X(TileClosePane         ) \
        X(TileMoveGrip          ) \
        X(TileResizeGrip        ) \

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
                                boss.base::detach(); // Destroy itself.
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
                                boss.bell::signal(tier::release, e2::form::size::restore);

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

                                auto gear_id_list = pro::focus::cut(applet_ptr);
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
        auto build_node = [](auto tag, auto slot1, auto slot2, auto grip_width, auto grip_bindings_ptr)
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
                    boss.LISTEN(tier::preview, app::tile::events::ui::grips::move, delta)
                    {
                        if (delta)
                        {
                            auto [orientation, griparea, ratio] = boss.get_config();
                            auto step = orientation == axis::X ? delta.x : delta.y;
                            if (step == 0) boss.bell::expire(tier::preview, true);
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
                auto grip = node->attach(slot::_I, ui::mock::ctor())
                    ->isroot(true)
                    ->active()
                    ->template plugin<pro::mover>() //todo GCC 11 requires template keyword
                    ->template plugin<pro::focus>(pro::focus::mode::focusable)
                    ->template plugin<pro::keybd>()
                    ->shader(c3, e2::form::state::focus::count)
                    ->template plugin<pro::shade<cell::shaders::xlight>>()
                    ->invoke([&](auto& boss)
                    {
                        boss.LISTEN(tier::release, hids::events::mouse::button::click::right, gear)
                        {
                            boss.base::riseup(tier::release, e2::form::size::minimize, gear);
                            gear.dismiss();
                        };
                        auto& keybd = boss.template plugins<pro::keybd>();
                        keybd.proc(action::TileMoveGrip     , [&](hids& gear){ gear.set_handled(); boss.base::riseup(tier::preview, app::tile::events::ui::grips::move,   gear.get_args_or(twod{})); });
                        keybd.proc(action::TileResizeGrip   , [&](hids& gear){ gear.set_handled(); boss.base::riseup(tier::preview, app::tile::events::ui::grips::resize, gear.get_args_or(si32{})); });
                        keybd.proc(action::TileFocusPrevGrip, [&](hids& gear){ boss.base::riseup(tier::preview, app::tile::events::ui::focus::prevgrip, gear); });
                        keybd.proc(action::TileFocusNextGrip, [&](hids& gear){ boss.base::riseup(tier::preview, app::tile::events::ui::focus::nextgrip, gear); });
                        keybd.bind(*grip_bindings_ptr);
                    });
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
                        pro::focus::set(boss.This(), gear.id, solo::on);
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
                    mouse_subs(boss);
                    boss.LISTEN(tier::release, hids::events::mouse::button::click::right, gear)
                    {
                        pro::focus::set(boss.This(), gear.id, solo::on);
                        boss.base::riseup(tier::request, e2::form::proceed::createby, gear);
                        gear.dismiss(true);
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
        auto node_veer = [](auto&& node_veer, auto min_state, auto grip_bindings_ptr) -> netxs::sptr<ui::veer>
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
                            auto gear_id_list = pro::focus::cut(boss.back());
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
                                    auto gear_id_list = pro::focus::cut(boss.back());
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
                    boss.LISTEN(tier::preview, e2::form::size::enlarge::any, gear, -, (oneoff = subs{}))
                    {
                        pro::focus::set(boss.This(), gear.id, solo::off);
                        if (boss.count() > 2 || oneoff) // It is a root or is already maximized. See build_inst::slot::_2's e2::form::proceed::attach for details.
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
                                fullscreen_item->LISTEN(tier::release, e2::form::size::restore, empty_ptr, oneoff)
                                {
                                    auto item_ptr = fullscreen_inst.This();
                                    auto gear_id_list = pro::focus::cut(item_ptr);
                                    item_ptr->base::detach();
                                    boss.attach(item_ptr);
                                    pro::focus::set(item_ptr, gear_id_list, solo::off);
                                    boss.base::reflow();
                                    oneoff.reset();
                                };
                                fullscreen_item->LISTEN(tier::release, e2::dtor, item_ptr, oneoff)
                                {
                                    oneoff.reset();
                                };
                                boss.base::riseup(tier::release, e2::form::proceed::attach, fullscreen_item);
                                boss.base::reflow();
                            }
                        }
                    };
                    boss.LISTEN(tier::release, app::tile::events::ui::split::any, gear, -, (grip_bindings_ptr))
                    {
                        if (auto deed = boss.bell::protos(tier::release))
                        {
                            auto depth = 0;
                            boss.diveup([&]{ depth++; });
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
                                auto gear_id_list = pro::focus::cut(boss.back());
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
                })
                ->branch(empty_slot());
        };
        auto parse_data = [](auto&& parse_data, view& utf8, auto min_ratio, auto grip_bindings_ptr) -> netxs::sptr<ui::veer>
        {
            auto slot = node_veer(node_veer, min_ratio, grip_bindings_ptr);
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
                auto node = build_node(tag, s1, s2, w, grip_bindings_ptr);
                auto slot1 = node->attach(slot::_1, parse_data(parse_data, utf8, ui::fork::min_ratio, grip_bindings_ptr));
                auto slot2 = node->attach(slot::_2, parse_data(parse_data, utf8, ui::fork::max_ratio, grip_bindings_ptr));
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
        namespace item_type
        {
            static constexpr auto _counter   = __COUNTER__ + 1;
            static constexpr auto empty_slot = __COUNTER__ - _counter;
            static constexpr auto applet     = __COUNTER__ - _counter;
            static constexpr auto grip       = __COUNTER__ - _counter;
        }
        auto _foreach = [](auto _foreach, sptr& root_veer_ptr, id_t gear_id, auto proc) -> void
        {
            if (auto node_veer_ptr = std::dynamic_pointer_cast<ui::veer>(root_veer_ptr))
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
                    else if (item_ptr->root()) // App window.
                    {
                        auto applet_ptr = item_ptr->base::subset[1];
                        if (pro::focus::is_focused(applet_ptr, gear_id))
                        {
                            proc(applet_ptr, item_type::applet, node_veer_ptr); // Applet.
                            if (!applet_ptr)
                            {
                                root_veer_ptr = {}; // Interrupt foreach.
                            }
                        }
                    }
                    else // if (!item_ptr->root()) // Node.
                    {
                        root_veer_ptr = item_ptr->base::subset[0];
                        _foreach(_foreach, root_veer_ptr, gear_id, proc);
                        if (!root_veer_ptr) return;

                        auto grip_ptr = item_ptr->base::subset[2];
                        if (pro::focus::is_focused(grip_ptr, gear_id))
                        {
                            proc(grip_ptr, item_type::grip, node_veer_ptr); // Grip.
                            if (!grip_ptr)
                            {
                                root_veer_ptr = {}; // Interrupt foreach.
                                return;
                            }
                        }

                        root_veer_ptr = item_ptr->base::subset[1];
                        _foreach(_foreach, root_veer_ptr, gear_id, proc);
                        if (!root_veer_ptr) return;
                    }
                }
            }
        };
        auto foreach = [](sptr& root_veer_ptr, id_t gear_id, auto proc)
        {
            _foreach(_foreach, root_veer_ptr, gear_id, proc);
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
            //auto danger_color    = skin::color(tone::danger);
            //auto warning_color   = skin::color(tone::warning);
            //auto c3 = highlight_color;
            //auto c2 = warning_color;
            //auto c1 = danger_color;

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
                    keybd.proc(action::TileFocusPrev         , [&](hids& gear){ boss.bell::signal(tier::preview, app::tile::events::ui::focus::prev,     gear); });
                    keybd.proc(action::TileFocusNext         , [&](hids& gear){ boss.bell::signal(tier::preview, app::tile::events::ui::focus::next,     gear); });
                    keybd.proc(action::TileFocusPrevPane     , [&](hids& gear){ boss.bell::signal(tier::preview, app::tile::events::ui::focus::prevpane, gear); });
                    keybd.proc(action::TileFocusNextPane     , [&](hids& gear){ boss.bell::signal(tier::preview, app::tile::events::ui::focus::nextpane, gear); });
                    keybd.proc(action::TileFocusPrevGrip     , [&](hids& gear){ boss.bell::signal(tier::preview, app::tile::events::ui::focus::prevgrip, gear); });
                    keybd.proc(action::TileFocusNextGrip     , [&](hids& gear){ boss.bell::signal(tier::preview, app::tile::events::ui::focus::nextgrip, gear); });
                    keybd.proc(action::TileRunApplicatoin    , [&](hids& gear){ boss.bell::signal(tier::preview, app::tile::events::ui::create,          gear); });
                    keybd.proc(action::TileSelectAllPanes    , [&](hids& gear){ boss.bell::signal(tier::preview, app::tile::events::ui::select,          gear); });
                    keybd.proc(action::TileSplitHorizontally , [&](hids& gear){ boss.bell::signal(tier::preview, app::tile::events::ui::split::hz,       gear); });
                    keybd.proc(action::TileSplitVertically   , [&](hids& gear){ boss.bell::signal(tier::preview, app::tile::events::ui::split::vt,       gear); });
                    keybd.proc(action::TileSplitOrientation  , [&](hids& gear){ boss.bell::signal(tier::preview, app::tile::events::ui::rotate,          gear); });
                    keybd.proc(action::TileSwapPanes         , [&](hids& gear){ boss.bell::signal(tier::preview, app::tile::events::ui::swap,            gear); });
                    keybd.proc(action::TileEqualizeSplitRatio, [&](hids& gear){ boss.bell::signal(tier::preview, app::tile::events::ui::equalize,        gear); });
                    keybd.proc(action::TileSetManagerTitle   , [&](hids& gear){ boss.bell::signal(tier::preview, app::tile::events::ui::title ,          gear); });
                    keybd.proc(action::TileClosePane         , [&](hids& gear){ boss.bell::signal(tier::preview, app::tile::events::ui::close,           gear); });
                    auto bindings = pro::keybd::load(config, "tile");
                    keybd.bind(bindings);

                    boss.LISTEN(tier::preview, app::tile::events::ui::any, gear)
                    {
                        auto root_veer_ptr = std::dynamic_pointer_cast<ui::veer>(boss.base::subset[1]);
                        if (root_veer_ptr->count() > 2)
                        {
                            root_veer_ptr->base::riseup(tier::release, e2::form::proceed::attach); // Restore the window before any action if maximized.
                        }
                    };
                    auto switch_counter_ptr = ptr::shared<std::unordered_map<id_t, feed>>();
                    auto& switch_counter = *switch_counter_ptr;
                    boss.LISTEN(tier::release, hids::events::focus::set::any, seed, -, (switch_counter_ptr)) // Reset the focus switch counter when it is focused from outside.
                    {
                        switch_counter[seed.gear_id] = {};
                    };
                    //todo generalize refocusing
                    boss.LISTEN(tier::preview, app::tile::events::ui::focus::prev, gear, -, (switch_counter_ptr))
                    {
                        auto root_veer_ptr = boss.base::subset[1];
                        auto nothing_to_iterate = std::dynamic_pointer_cast<ui::veer>(root_veer_ptr)->back()->root();
                        if (nothing_to_iterate) return;
                        auto prev_item_ptr = sptr{};
                        auto next_item_ptr = sptr{};
                        foreach(root_veer_ptr, id_t{}, [&](auto& item_ptr, si32 /*item_type*/, auto)
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
                                boss.bell::enqueue(prev_item_ptr, [&, gear_id = gear.id](auto& /*boss*/) // Keep the focus tree intact while processing key events.
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
                        auto root_veer_ptr = boss.base::subset[1];
                        auto nothing_to_iterate = std::dynamic_pointer_cast<ui::veer>(root_veer_ptr)->back()->root();
                        if (nothing_to_iterate) return;
                        auto prev_item_ptr = sptr{};
                        auto next_item_ptr = sptr{};
                        auto temp_item_ptr = sptr{};
                        foreach(root_veer_ptr, id_t{}, [&](auto& item_ptr, si32 /*item_type*/, auto)
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
                                boss.bell::enqueue(next_item_ptr, [&, gear_id = gear.id](auto& /*boss*/) // Keep the focus tree intact while processing key events.
                                {
                                    pro::focus::set(next_item.This(), gear_id, solo::on);
                                });
                                gear.set_handled();
                                switch_counter[gear.id] = feed::fwd;
                            }
                        }
                    };
                    boss.LISTEN(tier::preview, app::tile::events::ui::focus::prevpane, gear, -, (switch_counter_ptr))
                    {
                        auto root_veer_ptr = boss.base::subset[1];
                        auto nothing_to_iterate = std::dynamic_pointer_cast<ui::veer>(root_veer_ptr)->back()->root();
                        if (nothing_to_iterate) return;
                        auto prev_item_ptr = sptr{};
                        auto next_item_ptr = sptr{};
                        foreach(root_veer_ptr, id_t{}, [&](auto& item_ptr, si32 item_type, auto)
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
                                boss.bell::enqueue(prev_item_ptr, [&, gear_id = gear.id](auto& /*boss*/) // Keep the focus tree intact while processing key events.
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
                        auto root_veer_ptr = boss.base::subset[1];
                        auto nothing_to_iterate = std::dynamic_pointer_cast<ui::veer>(root_veer_ptr)->back()->root();
                        if (nothing_to_iterate) return;
                        auto prev_item_ptr = sptr{};
                        auto next_item_ptr = sptr{};
                        auto temp_item_ptr = sptr{};
                        foreach(root_veer_ptr, id_t{}, [&](auto& item_ptr, si32 item_type, auto)
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
                                boss.bell::enqueue(next_item_ptr, [&, gear_id = gear.id](auto& /*boss*/) // Keep the focus tree intact while processing key events.
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
                        auto root_veer_ptr = boss.base::subset[1];
                        auto nothing_to_iterate = std::dynamic_pointer_cast<ui::veer>(root_veer_ptr)->back()->root();
                        if (nothing_to_iterate) return;
                        auto prev_grip_ptr = sptr{};
                        auto next_grip_ptr = sptr{};
                        foreach(root_veer_ptr, id_t{}, [&](auto& item_ptr, si32 item_type, auto)
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
                            boss.bell::enqueue(prev_grip_ptr, [&, gear_id = gear.id](auto& /*boss*/) // Keep the focus tree intact while processing key events.
                            {
                                pro::focus::set(prev_grip.This(), gear_id, solo::on);
                            });
                            gear.set_handled();
                        }
                    };
                    boss.LISTEN(tier::preview, app::tile::events::ui::focus::nextgrip, gear)
                    {
                        auto root_veer_ptr = boss.base::subset[1];
                        auto nothing_to_iterate = std::dynamic_pointer_cast<ui::veer>(root_veer_ptr)->back()->root();
                        if (nothing_to_iterate) return;
                        auto prev_grip_ptr = sptr{};
                        auto next_grip_ptr = sptr{};
                        auto temp_grip_ptr = sptr{};
                        foreach(root_veer_ptr, id_t{}, [&](auto& item_ptr, si32 item_type, auto)
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
                            boss.bell::enqueue(next_grip_ptr, [&, gear_id = gear.id](auto& /*boss*/) // Keep the focus tree intact while processing key events.
                            {
                                pro::focus::set(next_grip.This(), gear_id, solo::on);
                            });
                            gear.set_handled();
                        }
                    };
                    boss.LISTEN(tier::preview, app::tile::events::ui::swap, gear)
                    {
                        auto root_veer_ptr = boss.base::subset[1];
                        auto nothing_to_iterate = std::dynamic_pointer_cast<ui::veer>(root_veer_ptr)->back()->root();
                        if (nothing_to_iterate) return;
                        auto node_veer_list = std::vector<netxs::sptr<ui::veer>>{};
                        auto node_grip_list = std::vector<netxs::sptr<ui::veer>>{};
                        foreach(root_veer_ptr, gear.id, [&](auto& /*item_ptr*/, si32 item_type, auto node_veer_ptr)
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
                                        app->bell::signal(tier::release, events::delist, true);
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
                                    app_slot->base::riseup(tier::release, events::enlist, app_slot);
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
                                app_slot->base::riseup(tier::release, events::enlist, app_slot);
                            }
                        }
                        for (auto& s : node_grip_list) // Swap panes in split.
                        {
                            s->back()->base::riseup(tier::release, app::tile::events::ui::swap, gear);
                        }
                        gear.set_handled();
                    };
                    boss.LISTEN(tier::preview, app::tile::events::ui::create, gear)
                    {
                        auto root_veer_ptr = boss.base::subset[1];
                        foreach(root_veer_ptr, gear.id, [&](auto& item_ptr, si32 item_type, auto)
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
                        auto root_veer_ptr = boss.base::subset[1];
                        foreach(root_veer_ptr, id_t{}, [&](auto& item_ptr, si32 item_type, auto)
                        {
                            if (item_type != item_type::grip) pro::focus::set(item_ptr, gear.id, solo::off);
                            else                              pro::focus::off(item_ptr, gear.id);
                        });
                        gear.set_handled();
                    };
                    boss.LISTEN(tier::preview, app::tile::events::ui::split::any, gear)
                    {
                        auto deed = boss.bell::protos(tier::preview);
                        auto root_veer_ptr = boss.base::subset[1];
                        foreach(root_veer_ptr, gear.id, [&](auto& item_ptr, si32 /*item_type*/, auto node_veer_ptr)
                        {
                            auto room = node_veer_ptr->base::size() / 3;
                            if (room.x && room.y) // Suppress split if there is no space.
                            {
                                boss.bell::enqueue(boss.This(), [&, deed, gear_id = gear.id, item_wptr = ptr::shadow(item_ptr)](auto& /*boss*/) // Enqueue to keep the focus tree intact while processing key events.
                                {
                                    if (auto gear_ptr = boss.bell::template getref<hids>(gear_id))
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
                        auto root_veer_ptr = boss.base::subset[1];
                        foreach(root_veer_ptr, gear.id, [&](auto& item_ptr, si32 /*item_type*/, auto)
                        {
                            item_ptr->base::riseup(tier::release, app::tile::events::ui::rotate, gear);
                            gear.set_handled();
                        });
                    };
                    boss.LISTEN(tier::preview, app::tile::events::ui::equalize, gear)
                    {
                        auto root_veer_ptr = boss.base::subset[1];
                        foreach(root_veer_ptr, gear.id, [&](auto& item_ptr, si32 /*item_type*/, auto)
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
                        auto root_veer_ptr = boss.base::subset[1];
                        foreach(root_veer_ptr, gear.id, [&](auto& item_ptr, si32 item_type, auto)
                        {
                            if (item_type != item_type::grip)
                            {
                                item_ptr->base::riseup(tier::preview, e2::form::proceed::quit::one, true);
                                gear.set_handled();
                            }
                        });
                    };
                });
            static auto on_left_click = [](auto& boss, auto& event)
            {
                boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                {
                    gear.dismiss(true);
                    boss.bell::expire(tier::release);
                    boss.base::riseup(tier::preview, event, gear);
                };
            };
            using namespace app::shared;
            static const auto proc_map = menu::action_map_t
            {
                { tile::action::TileFocusPrev         , [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::focus::prev    ); }},
                { tile::action::TileFocusNext         , [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::focus::next    ); }},
                { tile::action::TileFocusPrevPane     , [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::focus::prevpane); }},
                { tile::action::TileFocusNextPane     , [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::focus::nextpane); }},
                { tile::action::TileFocusPrevGrip     , [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::focus::prevgrip); }},
                { tile::action::TileFocusNextGrip     , [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::focus::nextgrip); }},
                { tile::action::TileRunApplicatoin    , [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::create         ); }},
                { tile::action::TileSelectAllPanes    , [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::select         ); }},
                { tile::action::TileSplitHorizontally , [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::split::hz      ); }},
                { tile::action::TileSplitVertically   , [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::split::vt      ); }},
                { tile::action::TileSplitOrientation  , [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::rotate         ); }},
                { tile::action::TileSwapPanes         , [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::swap           ); }},
                { tile::action::TileEqualizeSplitRatio, [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::equalize       ); }},
                { tile::action::TileSetManagerTitle   , [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::title          ); }},
                { tile::action::TileClosePane         , [](auto& boss, auto& /*item*/){ on_left_click(boss, app::tile::events::ui::close          ); }},
            };
            config.cd("/config/tile", "/config/defapp");
            auto grip_bindings_ptr = ptr::shared(pro::keybd::load(config, "tile/grips"));
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
            object->attach(slot::_2, parse_data(parse_data, param, ui::fork::min_ratio, grip_bindings_ptr))
                ->invoke([&](auto& boss)
                {
                    boss.LISTEN(tier::release, e2::form::proceed::attach, fullscreen_item)
                    {
                        if (boss.count() > 2)
                        {
                            boss.back()->bell::signal(tier::release, e2::form::size::restore);
                        }
                        if (fullscreen_item)
                        {
                            auto gear_id_list = pro::focus::cut(fullscreen_item);
                            fullscreen_item->base::detach();
                            pro::focus::off(boss.This());
                            boss.attach(fullscreen_item);
                            pro::focus::set(fullscreen_item, gear_id_list, solo::off);
                        }
                    };
                });
            return object;
        };
    }

    app::shared::initialize builder{ app::tile::id, build_inst };
}