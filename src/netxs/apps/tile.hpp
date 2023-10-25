// Copyright (c) NetXS Group.
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
                EVENT_XS( toggle  , input::hids ), // toggle window size: maximize/restore.
                EVENT_XS( swap    , input::hids ),
                EVENT_XS( rotate  , input::hids ), // change nested objects order. See tilimg manager (ui::fork).
                EVENT_XS( equalize, input::hids ),
                EVENT_XS( select  , input::hids ),
                GROUP_XS( split   , input::hids ),

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
    static constexpr auto id = "group";
    static constexpr auto desc = "Tiling Window Manager";
    static constexpr auto inheritance_limit = 30; // Tiling limits.

    using events = netxs::events::userland::tile;
    using ui::sptr;
    using ui::wptr;

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
            client = ui::list::ctor(axis::Y, ui::sort::reverse);
            client->SIGNAL(tier::release, e2::form::upon::vtree::attached, boss.This());

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
                    auto highlight_color = skin::color(tone::highlight);
                    auto c3 = highlight_color;
                    return ui::item::ctor(header.empty() ? "- no title -" : header)
                        ->setpad({ 1, 1 })
                        ->active()
                        ->shader(cell::shaders::xlight, e2::form::state::hover)
                        ->invoke([&](auto& boss)
                        {
                            auto update_focus = [](auto& boss, auto count)
                            {
                                auto highlight_color = skin::color(tone::highlight);
                                auto c3 = highlight_color;
                                auto x3 = cell{ c3 }.alpha(0x00);
                                boss.base::color(count ? 0xFF00ff00 : x3.fgc(), x3.bgc());
                            };
                            auto data_shadow = ptr::shadow(data_src_sptr);
                            boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent, boss.tracker, (data_shadow))
                            {
                                if (auto data_ptr = data_shadow.lock())
                                {
                                    data_ptr->RISEUP(tier::request, e2::form::state::keybd::focus::count, count, ());
                                    update_focus(boss, count);
                                    parent->resize();
                                }
                            };
                            boss.LISTEN(tier::release, e2::form::upon::vtree::detached, parent, boss.tracker)
                            {
                                if (parent) parent->resize(); // Rebuild list.
                            };
                            data_src_sptr->LISTEN(tier::release, e2::form::state::keybd::focus::count, count, boss.tracker)
                            {
                                update_focus(boss, count);
                            };
                            data_src_sptr->LISTEN(tier::release, events::delist, object, boss.tracker)
                            {
                                boss.detach(); // Destroy itself.
                            };
                            boss.LISTEN(tier::release, hids::events::mouse::button::any, gear, boss.tracker, (data_shadow))
                            {
                                if (auto data_ptr = data_shadow.lock())
                                {
                                    auto deed = boss.bell::template protos<tier::release>(); //todo clang 13.0.0 requires template
                                    data_ptr->template signal<tier::release>(deed, gear); //todo "template" keyword is required by gcc version 11.3.0
                                    gear.dismiss();
                                }
                            };
                            boss.LISTEN(tier::release, e2::form::state::mouse, active, boss.tracker, (data_shadow))
                            {
                                if (auto data_ptr = data_shadow.lock())
                                {
                                    data_ptr->SIGNAL(tier::release, e2::form::state::highlight, active);
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
                if constexpr (debugmode) log(prompt::tile, "Start depth %%", depth);
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
                    boss.RISEUP(tier::request, e2::form::state::keybd::find, gear_test, (gear.id, 0));
                    if (gear_test.second)
                    {
                        if (auto parent = boss.parent())
                        if (auto deed = parent->bell::template protos<tier::anycast>()) //todo "template" keyword is required by clang 13.0.0
                        {
                            switch (deed)
                            {
                                case app::tile::events::ui::create.id:
                                    boss.RISEUP(tier::request, e2::form::proceed::createby, gear);
                                    break;
                                case app::tile::events::ui::close.id:
                                    boss.RISEUP(tier::preview, e2::form::proceed::quit::one, true);
                                    break;
                                case app::tile::events::ui::toggle.id:
                                    if (boss.base::kind() == base::client) // Only apps can be maximized.
                                    if (gear.countdown > 0)
                                    {
                                        gear.countdown--; // The only one can be maximized if several are selected.
                                        boss.RISEUP(tier::release, e2::form::layout::fullscreen, gear);
                                    }
                                    break;
                                case app::tile::events::ui::swap.id:
                                    if (gear.countdown > 0)
                                    {
                                        boss.RISEUP(tier::release, app::tile::events::ui::swap, gear);
                                    }
                                    break;
                                case app::tile::events::ui::rotate.id:
                                    boss.RISEUP(tier::release, app::tile::events::ui::rotate, gear);
                                    break;
                                case app::tile::events::ui::equalize.id:
                                    boss.RISEUP(tier::release, app::tile::events::ui::equalize, gear);
                                    break;
                                case app::tile::events::ui::split::vt.id:
                                    boss.RISEUP(tier::release, app::tile::events::ui::split::vt, gear);
                                    break;
                                case app::tile::events::ui::split::hz.id:
                                    boss.RISEUP(tier::release, app::tile::events::ui::split::hz, gear);
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
                boss.RISEUP(tier::release, e2::form::layout::fullscreen, gear);
                gear.dismiss();
            };
            //boss.LISTEN(tier::release, hids::events::mouse::button::click::leftright, gear)
            //{
            //    auto backup = boss.This();
            //    boss.RISEUP(tier::release, e2::form::proceed::quit::one, true);
            //    gear.dismiss();
            //};
            //boss.LISTEN(tier::release, hids::events::mouse::button::click::middle, gear)
            //{
            //    auto backup = boss.This();
            //    boss.RISEUP(tier::release, e2::form::proceed::quit::one, true);
            //    gear.dismiss();
            //};
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

                        auto master_shadow = ptr::shadow(boss.This());
                        auto applet_shadow = ptr::shadow(what.applet);
                        boss.LISTEN(tier::release, hids::events::mouse::button::drag::start::any, gear, -, (applet_shadow, master_shadow, menuid = what.menuid))
                        {
                            if (auto master_ptr = master_shadow.lock())
                            if (auto applet_ptr = applet_shadow.lock())
                            if (applet_ptr->area().hittest(gear.coord))
                            {
                                auto& master = *master_ptr;
                                auto& applet = *applet_ptr;

                                auto deed = master.bell::template protos<tier::release>();
                                if (deed != hids::events::mouse::button::drag::start::left.id
                                 && deed != hids::events::mouse::button::drag::start::leftright.id) return;

                                // Restore if maximized. Parent can be changed.
                                master.SIGNAL(tier::release, e2::form::layout::restore, e2::form::layout::restore.param());

                                // Take current title.
                                auto what = vtm::events::handoff.param({ .menuid = menuid });
                                master.SIGNAL(tier::request, e2::form::prop::ui::header, what.header);
                                master.SIGNAL(tier::request, e2::form::prop::ui::footer, what.footer);
                                if (what.header.empty()) what.header = menuid;

                                // Find creator.
                                master.RISEUP(tier::request, e2::config::creator, world_ptr, ());

                                // Take coor and detach from the tiling wm.
                                gear.coord -= applet.base::coor(); // Localize mouse coor.
                                what.square.size = applet.base::size();
                                applet.global(what.square.coor);
                                what.square.coor = -what.square.coor;
                                what.forced = true;
                                what.applet = applet_ptr;
                                master.remove(applet_ptr);
                                applet.moveto(dot_00);

                                if (auto parent_ptr = master.parent())
                                {
                                    auto gear_id_list = pro::focus::get(parent_ptr); // Expropriate all foci.
                                    world_ptr->SIGNAL(tier::request, vtm::events::handoff, what); // Attach to the world.
                                    pro::focus::set(what.applet, gear_id_list, pro::focus::solo::off, pro::focus::flip::off, true); // Refocus.
                                    master.RISEUP(tier::release, e2::form::proceed::quit::one, true); // Destroy placeholder.
                                }

                                // Redirect this mouse event to the new world's window.
                                gear.pass<tier::release>(what.applet, dot_00);
                            }
                        };
                        boss.LISTEN(tier::anycast, e2::form::upon::started, root)
                        {
                            boss.RISEUP(tier::release, events::enlist, boss.This());
                        };
                        boss.LISTEN(tier::release, hids::events::mouse::button::click::right, gear)
                        {
                            pro::focus::set(boss.This(), gear.id, pro::focus::solo::on, pro::focus::flip::off);
                        };
                        boss.LISTEN(tier::release, hids::events::mouse::button::click::middle, gear)
                        {
                            pro::focus::set(boss.This(), gear.id, pro::focus::solo::on, pro::focus::flip::off);
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
        auto built_node = [](auto tag, auto slot1, auto slot2, auto grip_width)
        {
            auto node = tag == 'h' ? ui::fork::ctor(axis::X, grip_width == -1 ? 2 : grip_width, slot1, slot2)
                                   : ui::fork::ctor(axis::Y, grip_width == -1 ? 1 : grip_width, slot1, slot2);
            node->isroot(faux, base::node) // Set object kind to 1 to be different from others. See empty_slot::select.
                ->template plugin<pro::focus>()
                ->limits(dot_00)
                ->invoke([&](auto& boss)
                {
                    mouse_subs(boss);
                    boss.LISTEN(tier::release, app::tile::events::ui::swap     , gear) { boss.swap();       };
                    boss.LISTEN(tier::release, app::tile::events::ui::rotate   , gear) { boss.rotate();     };
                    boss.LISTEN(tier::release, app::tile::events::ui::equalize , gear) { boss.config(1, 1); };
                    boss.LISTEN(tier::release, hids::events::mouse::scroll::any, gear)
                    {
                        if (gear.meta(hids::anyCtrl))
                        {
                            switch (boss.bell::template protos<tier::release>()) // Clang 13.0.0 complains.
                            {
                                case hids::events::mouse::scroll::up.id:   boss.move_slider(-4); break;
                                case hids::events::mouse::scroll::down.id: boss.move_slider( 4); break;
                            }
                            gear.dismiss();
                        }
                    };
                });
                auto grip = node->attach(slot::_I,
                                ui::mock::ctor()
                                ->isroot(true)
                                ->template plugin<pro::mover>() //todo GCC 11 requires template keyword
                                ->template plugin<pro::focus>(pro::focus::mode::focusable)
                                ->template plugin<pro::track>(true)
                                ->template plugin<pro::shade<cell::shaders::xlight>>()
                                ->invoke([&](auto& boss)
                                {
                                    anycasting(boss);
                                    //todo implement keydb support
                                    boss.LISTEN(tier::release, hids::events::mouse::button::click::right, gear)
                                    {
                                        boss.RISEUP(tier::release, e2::form::layout::minimize, gear);
                                        gear.dismiss();
                                    };
                                })
                                ->active());
            return node;
        };
        auto empty_pane = []
        {
            auto menu_black = skin::color(tone::menu_black);
            auto cC = menu_black.fgc(whitedk);
            auto highlight_color = skin::color(tone::highlight);
            auto danger_color    = skin::color(tone::danger);
            auto c3 = highlight_color;
            auto c1 = danger_color;

            using namespace app::shared;
            auto [menu_block, cover, menu_data] = menu::mini(true, true, faux, 1,
            menu::list
            {
                { menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{{ .label = "+", .notes = " New app " }}},
                [](auto& boss, auto& item)
                {
                    boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                    {
                        boss.RISEUP(tier::request, e2::form::proceed::createby, gear);
                        gear.dismiss(true);
                    };
                }},
                { menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{{ .label = "│", .notes = " Split horizontally " }}},
                [](auto& boss, auto& item)
                {
                    boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                    {
                        boss.RISEUP(tier::release, app::tile::events::ui::split::hz, gear);
                        gear.dismiss(true);
                    };
                }},
                { menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{{ .label = "──", .notes = " Split vertically " }}},
                [](auto& boss, auto& item)
                {
                    boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                    {
                        boss.RISEUP(tier::release, app::tile::events::ui::split::vt, gear);
                        gear.dismiss(true);
                    };
                }},
                { menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{{ .label = "×", .notes = " Delete pane ", .hover = c1 }}},
                [](auto& boss, auto& item)
                {
                    boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                    {
                        auto backup = boss.This();
                        boss.RISEUP(tier::release, e2::form::proceed::quit::one, true);
                        gear.dismiss(true);
                    };
                }},
            });
            menu_data->colors(cC.fgc(), cC.bgc());
            auto menu_id = menu_block->id;
            cover->setpad({ 0,0,3,0 });
            cover->invoke([&](auto& boss)
            {
                boss.LISTEN(tier::release, e2::render::any, parent_canvas, -, (menu_id))
                {
                    parent_canvas.fill([&](cell& c) { c.txt(whitespace).link(menu_id); });
                };
            });

            return ui::cake::ctor()
                ->isroot(true, base::placeholder)
                ->active()
                ->colors(cC.fgc(), cC.bgc())
                ->limits(dot_00, -dot_11)
                ->plugin<pro::focus>(pro::focus::mode::focusable)
                ->plugin<pro::track>(true)
                ->invoke([&](auto& boss)
                {
                    anycasting(boss);
                    mouse_subs(boss);
                    boss.LISTEN(tier::release, hids::events::mouse::button::click::right, gear)
                    {
                        boss.RISEUP(tier::request, e2::form::proceed::createby, gear);
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
        auto empty_slot = [](auto&& empty_slot, auto min_state) -> netxs::sptr<ui::veer>
        {
            return ui::veer::ctor()
                ->plugin<pro::focus>(pro::focus::mode::hub/*default*/, true/*default*/, true)
                ->active()
                ->invoke([&](auto& boss)
                {
                    auto highlight = [](auto& boss, auto state)
                    {
                        auto c = state ? cell{ skin::color(tone::highlight) }.alpha(0x70)
                                       : cell{ skin::color(tone::menu_black) };
                        boss.front()->color(c.fgc(), c.bgc());
                        boss.deface();
                    };
                    boss.LISTEN(tier::release, e2::form::layout::minimize, gear, -, (saved_ratio = 1, min_ratio = 1, min_state))
                    {
                        if (auto node = std::dynamic_pointer_cast<ui::fork>(boss.base::parent()))
                        {
                            auto ratio = node->get_ratio();
                            if (ratio == min_ratio)
                            {
                                node->set_ratio(saved_ratio);
                                pro::focus::set(boss.This(), gear.id, gear.meta(hids::anyCtrl) ? pro::focus::solo::off
                                                                                               : pro::focus::solo::on, pro::focus::flip::off, true);
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
                        // Block rising up this event: DTVT object fires this event on exit.
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
                            pro::focus::off(boss.This());
                            auto gear_id_list = pro::focus::get(what.applet);
                            auto app = app_window(what);
                            boss.attach(app);
                            app->SIGNAL(tier::anycast, e2::form::upon::started, app);
                            pro::focus::set(what.applet, gear_id_list, pro::focus::solo::mix, pro::focus::flip::off, true);
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
                            auto gear_id_list = pro::focus::get(boss.This());
                            auto deleted_item = boss.pop_back();
                            if (item_ptr)
                            {
                                boss.attach(item_ptr);
                            }
                            else item_ptr = boss.This();
                            pro::focus::set(boss.back(), gear_id_list, pro::focus::solo::off, pro::focus::flip::off);
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
                                    auto gear_id_list = pro::focus::get(boss.This());
                                    item_ptr = boss.pop_back();
                                    pro::focus::set(boss.back(), gear_id_list, pro::focus::solo::off, pro::focus::flip::off);
                                }
                                else
                                {
                                    if constexpr (debugmode) log(prompt::tile, "Empty slot: defective structure, count=", boss.count());
                                }
                                if (auto parent = boss.parent())
                                {
                                    parent->bell::template expire<tier::request>();
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
                                item.SIGNAL(tier::anycast, e2::form::upon::started, item_ptr);
                            }
                        }
                    };
                    boss.LISTEN(tier::anycast, app::tile::events::ui::select, gear)
                    {
                        auto item_ptr = boss.back();
                        if (item_ptr->base::kind() != base::node) pro::focus::set(item_ptr, gear.id, pro::focus::solo::off, pro::focus::flip::off);
                        else                                      pro::focus::off(item_ptr, gear.id); // Exclude grips.
                    };
                    boss.LISTEN(tier::release, e2::form::layout::fullscreen, gear, -, (oneoff = subs{}))
                    {
                        if (boss.count() > 2 || oneoff) // It is a root or is already maximized. See build_inst::slot::_2's e2::form::proceed::attach for details.
                        {
                            boss.RISEUP(tier::release, e2::form::proceed::attach, e2::form::proceed::attach.param());
                        }
                        else
                        {
                            if (boss.count() > 1) // Preventing the empty slot from maximizing.
                            if (boss.back()->base::kind() == base::client) // Preventing the splitter from maximizing.
                            if (auto fullscreen_item = boss.pop_back())
                            {
                                auto gear_id_list = pro::focus::get(boss.This()); // Seize all foci.
                                fullscreen_item->LISTEN(tier::release, e2::form::layout::restore, item_ptr, oneoff)
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
                                boss.RISEUP(tier::release, e2::form::proceed::attach, fullscreen_item);
                                pro::focus::set(just_copy, gear_id_list, pro::focus::solo::off, pro::focus::flip::off); // Handover all foci.
                                boss.base::reflow();
                            }
                        }
                    };
                    boss.LISTEN(tier::release, app::tile::events::ui::split::any, gear)
                    {
                        if (auto deed = boss.bell::template protos<tier::release>())
                        {
                            auto depth = 0;
                            boss.diveup([&]{ depth++; });
                            if constexpr (debugmode) log(prompt::tile, "Depth ", depth);
                            if (depth > inheritance_limit) return;

                            auto heading = deed == app::tile::events::ui::split::vt.id;
                            auto newnode = built_node(heading ? 'v':'h', 1, 1, heading ? 1 : 2);
                            auto empty_1 = empty_slot(empty_slot, ui::fork::min_ratio);
                            auto empty_2 = empty_slot(empty_slot, ui::fork::max_ratio);
                            auto gear_id = pro::focus::get(boss.This()); // Seize all foci.
                            auto curitem = boss.pop_back();
                            if (boss.empty())
                            {
                                boss.attach(empty_pane());
                                empty_1->pop_back();
                            }
                            auto slot_1 = newnode->attach(slot::_1, empty_1->branch(curitem));
                            auto slot_2 = newnode->attach(slot::_2, empty_2);
                            boss.attach(newnode);
                            pro::focus::set(slot_1->back(), gear_id, pro::focus::solo::off, pro::focus::flip::off); // Handover all foci.
                            pro::focus::set(slot_2->back(), gear_id, pro::focus::solo::off, pro::focus::flip::off);
                        }
                    };
                    boss.LISTEN(tier::anycast, e2::form::proceed::quit::any, fast)
                    {
                        boss.SIGNAL(tier::preview, e2::form::proceed::quit::one, fast);
                    };
                    boss.LISTEN(tier::preview, e2::form::proceed::quit::one, fast)
                    {
                        if (boss.count() > 0 && boss.back()->base::root()) // Walking a nested visual tree.
                        {
                            boss.back()->SIGNAL(tier::anycast, e2::form::proceed::quit::one, true); // fast=true: Immediately closing (no ways to showing a closing process). Forward a quit message to hosted app in order to schedule a cleanup.
                        }
                        else boss.SIGNAL(tier::release, e2::form::proceed::quit::one, fast);
                    };
                    boss.LISTEN(tier::release, e2::form::proceed::quit::any, fast)
                    {
                        if (auto parent = boss.base::parent())
                        {
                            if (boss.count() > 1 && boss.back()->base::kind() == base::client) // Only apps can be deleted.
                            {
                                auto gear_id_list = pro::focus::get(boss.This());
                                auto deleted_item = boss.pop_back(); // Throw away.
                                pro::focus::set(boss.back(), gear_id_list, pro::focus::solo::off, pro::focus::flip::off);
                            }
                            else if (boss.count() == 1) // Remove empty slot, reorganize.
                            {
                                parent->SIGNAL(tier::request, e2::form::proceed::swap, item_ptr, (boss.This())); // sptr must be of the same type as the event argument. Casting kills all intermediaries when return.
                                if (item_ptr != boss.This()) // Parallel slot is not empty or both slots are empty (item_ptr == null).
                                {
                                    parent->RISEUP(tier::release, e2::form::proceed::swap, item_ptr);
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
                        gate.SIGNAL(tier::request, e2::data::changed, current_default, ());
                        gate.RISEUP(tier::request, vtm::events::apptype, config, ({ .menuid = current_default }));
                        if (config.kindid == netxs::app::region::id) return; // Deny any view regions inside the tiling manager.

                        gate.RISEUP(tier::request, vtm::events::newapp, config);
                        auto app = app_window(config);
                        auto gear_id_list = pro::focus::get(boss.back());
                        boss.attach(app);
                        if (auto world_ptr = gate.parent()) // Finalize app creation.
                        {
                            app->SIGNAL(tier::anycast, vtm::events::attached, world_ptr);
                        }

                        insts_count++; //todo unify, demo limits
                        config.applet->LISTEN(tier::release, e2::dtor, id)
                        {
                            insts_count--;
                            if constexpr (debugmode) log(prompt::tile, "Instance detached: id:", id, "; left:", insts_count);
                        };

                        app->SIGNAL(tier::anycast, e2::form::upon::started, app);
                        if (std::find(gear_id_list.begin(), gear_id_list.end(), gear.id) == gear_id_list.end())
                        {
                            gear_id_list.push_back(gear.id);
                        }
                        pro::focus::set(app, gear_id_list, pro::focus::solo::off, pro::focus::flip::off);
                    };
                    boss.LISTEN(tier::release, events::backup, empty_slot_list)
                    {
                        if (boss.count())
                        if (auto item_ptr = boss.back())
                        if (item_ptr->base::root())
                        {
                            empty_slot_list.push_back(boss.This());
                        }
                    };
                })
                ->branch(empty_pane());
        };
        auto parse_data = [](auto&& parse_data, view& utf8, auto min_ratio) -> netxs::sptr<ui::veer>
        {
            auto slot = empty_slot(empty_slot, min_ratio);
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
                if (auto v = utf::to_int(utf8)) // Left side ratio
                {
                    s1 = std::abs(v.value());
                    if (utf8.empty() || utf8.front() != ':') return slot;
                    utf8.remove_prefix(1);
                    if (auto v = utf::to_int(utf8)) // Right side ratio
                    {
                        s2 = std::abs(v.value());
                        utf::trim_front(utf8, " ");
                        if (!utf8.empty() && utf8.front() == ':') // Grip width.
                        {
                            utf8.remove_prefix(1);
                            if (auto v = utf::to_int(utf8))
                            {
                                w = std::abs(v.value());
                                utf::trim_front(utf8, " ");
                            }
                        }
                    }
                    else return slot;
                }
                if (utf8.empty() || utf8.front() != '(') return slot;
                utf8.remove_prefix(1);
                auto node = built_node(tag, s1, s2, w);
                auto slot1 = node->attach(ui::slot::_1, parse_data(parse_data, utf8, ui::fork::min_ratio));
                auto slot2 = node->attach(ui::slot::_2, parse_data(parse_data, utf8, ui::fork::max_ratio));
                slot->attach(node);
                utf::trim_front(utf8, ") ");
            }
            else  // Add application.
            {
                utf::trim_front(utf8, " ");
                auto menuid = utf::get_tail(utf8, " ,)").str();
                if (menuid.empty()) return slot;

                utf::trim_front(utf8, " ,");
                if (utf8.size() && utf8.front() == ')') utf8.remove_prefix(1); // pop ')';

                auto oneoff = ptr::shared(hook{});
                slot->LISTEN(tier::anycast, vtm::events::attached, world_ptr, *oneoff, (oneoff, menuid, slot))
                {
                    world_ptr->SIGNAL(tier::request, vtm::events::newapp, what, ({ .menuid = menuid }));
                    auto inst = app_window(what);
                    slot->attach(inst);
                    inst->SIGNAL(tier::anycast, vtm::events::attached, world_ptr);
                    oneoff.reset();
                };
            }
            return slot;
        };
        auto build_inst = [](text cwd, view param, xmls& config, text patch) -> sptr
        {
            auto menu_white = skin::color(tone::menu_white);
            auto cB = menu_white;
            auto highlight_color = skin::color(tone::highlight);
            auto danger_color    = skin::color(tone::danger);
            auto warning_color   = skin::color(tone::warning);
            auto c3 = highlight_color;
            auto c2 = warning_color;
            auto c1 = danger_color;

            auto object = ui::fork::ctor(axis::Y)
                ->plugin<items>()
                ->invoke([&](auto& boss)
                {
                    auto oneoff = ptr::shared(hook{});
                    boss.LISTEN(tier::anycast, e2::form::upon::created, gear, *oneoff, (oneoff))
                    {
                        auto& gate = gear.owner;
                        gate.SIGNAL(tier::request, e2::data::changed, menuid, ());
                        gate.RISEUP(tier::request, desk::events::menu, conf_list_ptr, ());
                        auto& conf_list = *conf_list_ptr;
                        auto& config = conf_list[menuid];
                        if (config.type == app::tile::id) // Reset the currently selected application to the previous one.
                        {
                            gate.SIGNAL(tier::preview, e2::data::changed, menuid); // Get previous default;
                            gate.SIGNAL(tier::release, e2::data::changed, menuid); // Set current  default;
                        }
                        oneoff.reset();
                    };
                });

            config.cd("/config/tile/", "/config/defapp/");

            using namespace app::shared;
            auto [menu_block, cover, menu_data] = menu::create(config,
                    menu::list
                    {
                        //  Green                                  ?Even    Red
                        // ┌────┐  ┌────┐  ┌─┬──┐  ┌────┐  ┌─┬──┐  ┌─┬──┐  ┌────┐  // ┌─┐  ┌─┬─┐  ┌─┬─┐  ┌─┬─┐  
                        // │Exec│  ├─┐  │  │ H  │  ├ V ─┤  │Swap│  │Fair│  │Shut│  // ├─┤  └─┴─┘  └<┴>┘  └>┴<┘  
                        // └────┘  └─┴──┘  └─┴──┘  └────┘  └─┴──┘  └─┴──┘  └────┘  // └─┘                       
                        //{ menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{{ .label = " ┐└ ", .notes = " Maximize/restore active pane " }}},
                        //[](auto& boss, auto& item)
                        //{
                        //    boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                        //    {
                        //        gear.countdown = 1;
                        //        boss.SIGNAL(tier::anycast, app::tile::events::ui::toggle, gear);
                        //        gear.dismiss(true);
                        //    };
                        //}},
                        { menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{{ .label = " + ", .notes = " Create and run a new app in active panes " }}},
                        [](auto& boss, auto& item)
                        {
                            boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::create, gear);
                                gear.dismiss(true);
                            };
                        }},
                        { menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{{ .label = ":::", .notes = " Select all panes " }}},
                        [](auto& boss, auto& item)
                        {
                            boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::select, gear);
                                gear.dismiss(true);
                            };
                        }},
                        { menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{{ .label = " │ ", .notes = " Split active panes horizontally " }}},
                        [](auto& boss, auto& item)
                        {
                            boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::split::hz, gear);
                                gear.dismiss(true);
                            };
                        }},
                        { menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{{ .label = "──", .notes = " Split active panes vertically " }}},
                        [](auto& boss, auto& item)
                        {
                            boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::split::vt, gear);
                                gear.dismiss(true);
                            };
                        }},
                        { menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{{ .label = "┌┘", .notes = " Change split orientation " }}},
                        [](auto& boss, auto& item)
                        {
                            boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::rotate, gear);
                                gear.dismiss(true);
                            };
                        }},
                        { menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{{ .label = "<->", .notes = " Swap two or more panes " }}},
                        [](auto& boss, auto& item)
                        {
                            boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::swap, gear);
                                gear.dismiss(true);
                            };
                        }},
                        { menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{{ .label = ">|<", .notes = " Equalize split ratio " }}},
                        [](auto& boss, auto& item)
                        {
                            boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::equalize, gear);
                                gear.dismiss(true);
                            };
                        }},
                        { menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{{ .label = "\"…\"", .notes = " Set tiling manager window title using clipboard data " }}},
                        [](auto& boss, auto& item)
                        {
                            boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                app::shared::set_title(boss, gear);
                            };
                        }},
                        { menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{{ .label = "×", .notes = " Close active app ", .hover = c1 }}},
                        [](auto& boss, auto& item)
                        {
                            boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::close, gear);
                                gear.dismiss(true);
                            };
                        }},
                    });
            object->attach(slot::_1, menu_block)
                  ->invoke([](auto& boss)
                  {
                      boss.LISTEN(tier::anycast, e2::form::proceed::quit::any, fast)
                      {
                          boss.RISEUP(tier::release, e2::form::proceed::quit::one, fast);
                      };
                  });
            menu_data->colors(cB.fgc(), cB.bgc())
                     ->plugin<pro::track>()
                     ->plugin<pro::acryl>();
            auto menu_id = menu_block->id;
            cover->invoke([&](auto& boss)
            {
                auto bar = cell{ "▀"sv }.link(menu_id);
                boss.LISTEN(tier::release, e2::render::any, parent_canvas, -, (bar))
                {
                    auto menu_white = skin::color(tone::menu_white);
                    auto fgc = menu_white.bgc();
                    parent_canvas.fill([&](cell& c) { c.fgc(fgc).txt(bar).link(bar); });
                };
            });
            if (cwd.size())
            {
                auto err = std::error_code{};
                fs::current_path(cwd, err);
                if (err) log("%%Failed to change current working directory to '%cwd%', error code: %error%", prompt::tile, cwd, err.value());
                else     log("%%Change current working directory to '%cwd%'", prompt::tile, cwd);
            }

            object->attach(slot::_2, parse_data(parse_data, param, ui::fork::min_ratio))
                ->invoke([&](auto& boss)
                {
                    boss.LISTEN(tier::release, e2::form::proceed::attach, fullscreen_item, -, (foci_list = gear_id_list_t{}))
                    {
                        if (boss.count() > 2)
                        {
                            auto gear_id_list = pro::focus::get(boss.This()); // Seize all foci.
                            auto item_ptr = boss.pop_back();
                            item_ptr->SIGNAL(tier::release, e2::form::layout::restore, item_ptr);
                            pro::focus::set(boss.back(), foci_list, pro::focus::solo::off, pro::focus::flip::off, true); // Restore saved foci.
                            pro::focus::set(item_ptr, gear_id_list, pro::focus::solo::off, pro::focus::flip::off); // Apply item's foci.
                            foci_list.clear();
                        }

                        if (fullscreen_item)
                        {
                            foci_list = pro::focus::get(boss.This()); // Save all foci.
                            boss.attach(fullscreen_item);
                            fullscreen_item.reset();
                        }
                        else log(prompt::tile, "Fullscreen item is empty");
                    };
                    boss.LISTEN(tier::anycast, app::tile::events::ui::any, gear)
                    {
                        if (auto deed = boss.bell::template protos<tier::anycast>()) //todo "template" keyword is required by clang 13.0.0
                        {
                            if (boss.count() > 2 && deed != app::tile::events::ui::toggle.id) // Restore the window before any action if maximized.
                            {
                                boss.RISEUP(tier::release, e2::form::proceed::attach, e2::form::proceed::attach.param());
                            }

                            if (deed == app::tile::events::ui::swap.id)
                            {
                                auto empty_slot_list = backups{};
                                auto proc = e2::form::proceed::functor.param([&](sptr item_ptr)
                                {
                                    item_ptr->SIGNAL(tier::request, e2::form::state::keybd::find, gear_test, (gear.id, 0));
                                    if (gear_test.second)
                                    {
                                        item_ptr->RISEUP(tier::release, events::backup, empty_slot_list);
                                    }
                                });
                                boss.SIGNAL(tier::general, e2::form::proceed::functor, proc);
                                auto slots_count = empty_slot_list.size();
                                log(prompt::tile, "Slots count:", slots_count);
                                if (slots_count >= 2) // Swap selected panes cyclically.
                                {
                                    log(prompt::tile, "Swap slots cyclically");
                                    auto i = 0;
                                    auto emp_slot = sptr{};
                                    auto app_slot = sptr{};
                                    auto emp_next = sptr{};
                                    auto app_next = sptr{};
                                    for (auto& s : empty_slot_list)
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
                                                app->SIGNAL(tier::release, events::delist, true);
                                            }
                                            app_next = s->pop_back();
                                            emp_next = s->pop_back();
                                        }
                                        if (emp_slot) s->attach(emp_slot);
                                        if (app_slot)
                                        {
                                            s->attach(app_slot);
                                            app_slot->RISEUP(tier::release, events::enlist, app_slot);
                                        }
                                        std::swap(emp_slot, emp_next);
                                        std::swap(app_slot, app_next);
                                    }
                                    auto& s = empty_slot_list.front();
                                    if (emp_slot) s->attach(emp_slot);
                                    if (app_slot)
                                    {
                                        s->attach(app_slot);
                                        app_slot->RISEUP(tier::release, events::enlist, app_slot);
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