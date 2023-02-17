// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

// Tiling limits.
#define INHERITANCE_LIMIT 30

namespace netxs::app::tile
{
    using backups = std::list<sptr<ui::veer>>;
}

namespace netxs::events::userland
{
    struct tile
    {
        EVENTPACK( tile, netxs::events::userland::root::custom )
        {
            EVENT_XS( backup, app::tile::backups ),
            EVENT_XS( enlist, sptr<ui::base>     ),
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

    using events = netxs::events::userland::tile;

    // tile: Right-side item list.
    class items
        : public pro::skill
    {
        using depth_t = decltype(e2::depth)::type;
        using skill::boss,
              skill::memo;

        sptr<ui::list> client;
        depth_t        depth;

    public:
        items(base&&) = delete;
        items(base& boss)
            : skill{ boss },
              depth{ 0    }
        {
            client = ui::list::ctor(axis::Y, ui::sort::reverse);

            client->SIGNAL(tier::release, e2::form::upon::vtree::attached, boss.This());

            boss.LISTEN(tier::release, e2::size::any, newsz, memo)
            {
                if (client)
                {
                    auto new_coor = twod{ newsz.x + 2/*todo resize grip width*/, 0 };
                    auto new_size = twod{ client->size().x, newsz.y };
                    client->base::moveto(new_coor);
                    client->base::resize(new_size);
                }
            };
            boss.LISTEN(tier::release, events::enlist, object, memo)
            {
                if (!client) return;
                auto label = [](auto data_src_sptr, auto header)
                {
                    auto highlight_color = skin::color(tone::highlight);
                    auto c3 = highlight_color;
                    auto x3 = cell{ c3 }.alpha(0x00);

                    return ui::pads::ctor(dent{ 1, 1, 0, 0 }, dent{})
                        ->plugin<pro::fader>(x3, c3, skin::globals().fader_time)
                        ->branch(ui::item::ctor(header.empty() ? "- no title -" : header))
                        ->invoke([&](auto& boss)
                        {
                            auto data_shadow = ptr::shadow(data_src_sptr);
                            boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent, boss.tracker, (data_shadow))
                            {
                                if (auto data_ptr = data_shadow.lock())
                                {
                                    auto state = e2::form::highlight::set.param();
                                    data_ptr->SIGNAL(tier::anycast, e2::form::highlight::set, state);
                                }
                            };
                            data_src_sptr->LISTEN(tier::preview, e2::form::highlight::any, state, boss.tracker)
                            {
                                auto highlight_color = skin::color(tone::highlight);
                                auto c3 = highlight_color;
                                auto x3 = cell{ c3 }.alpha(0x00);
                                boss.color(state ? 0xFF00ff00 : x3.fgc(), x3.bgc());
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
                                    data_ptr->template signal<tier::release>(deed, gear); //todo "template" keyword is required by gcc version 10.4.0
                                    gear.dismiss();
                                }
                            };
                            boss.LISTEN(tier::release, e2::form::state::mouse, active, boss.tracker, (data_shadow))
                            {
                                if (auto data_ptr = data_shadow.lock())
                                {
                                    data_ptr->SIGNAL(tier::release, e2::form::highlight::any, active);
                                }
                            };
                        });
                };
                client->attach_element(e2::form::prop::ui::header, object, label);
            };
            boss.LISTEN(tier::release, e2::render::any, parent_canvas, memo)
            {
                //todo magic numbers
                if (depth < 4 && client)
                {
                    auto& basis = boss.base::coor();
                    auto canvas_view = parent_canvas.core::view();
                    auto canvas_area = parent_canvas.core::area();
                    canvas_area.coor = dot_00;
                    parent_canvas.core::view(canvas_area);
                    parent_canvas.render<faux>(client, basis);
                    parent_canvas.core::view(canvas_view);
                }
            };
            boss.LISTEN(tier::anycast, e2::form::upon::started, root, memo)
            {
                client->clear();
                depth = 0;
                boss.RISEUP(tier::request, e2::depth, depth, true);
                log("tile: start depth=", depth);
            };
        }
       ~items()
        {
            if (client)
            {
                auto lock = netxs::events::sync{};
                auto empty = e2::form::upon::vtree::detached.param();
                client->SIGNAL(tier::release, e2::form::upon::vtree::detached, empty);
            }
        }
    };

    namespace
    {
        auto anycasting = [](auto& boss)
        {
            boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent)
            {
                auto parent_memo = ptr::shared(subs{});
                parent->LISTEN(tier::anycast, app::tile::events::ui::any, gear, *parent_memo)
                {
                    auto gear_test = e2::form::state::keybd::find.param(gear.id, 0);
                    boss.SIGNAL(tier::anycast, e2::form::state::keybd::find, gear_test);
                    if (gear_test.second)
                    {
                        if (auto parent = boss.parent())
                        if (auto deed = parent->bell::template protos<tier::anycast>()) //todo "template" keyword is required by clang 13.0.0
                        {
                            switch (deed)
                            {
                                case app::tile::events::ui::create.id:
                                    gear.force_group_focus = true;
                                    boss.RISEUP(tier::request, e2::form::proceed::createby, gear);
                                    gear.force_group_focus = faux;
                                    break;
                                case app::tile::events::ui::close.id:
                                    boss.RISEUP(tier::preview, e2::form::quit, boss.This());
                                    break;
                                case app::tile::events::ui::toggle.id:
                                    if (boss.base::kind() == 0) // Only apps can be maximized.
                                    if (gear.countdown > 0)
                                    {
                                        gear.countdown--;
                                        // Removing multifocus - The only one can be maximized if several are selected.
                                        gear.kb_offer_11(boss);
                                        boss.RISEUP(tier::release, e2::form::maximize, gear);
                                        //todo parent_memo is reset by the empty slot here (pop_back), undefined behavior from here
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
                boss.LISTEN(tier::release, e2::form::upon::vtree::detached, parent, *parent_memo, (parent_memo))
                {
                    parent_memo.reset();
                };
                // Forward keybd events.
                //todo foci
                //parent->LISTEN(tier::anycast, hids::events::upevent::kboffer, gear, *parent_memo)
                //{
                //    log("forward kboffer");
                //    boss.SIGNAL(tier::anycast, hids::events::upevent::kboffer, gear);
                //};
                //parent->LISTEN(tier::anycast, hids::events::upevent::kbannul, gear, *parent_memo)
                //{
                //    log("forward kbannul");
                //    boss.SIGNAL(tier::anycast, hids::events::upevent::kbannul, gear);
                //};
            };
        };
        auto mouse_subs = [](auto& boss)
        {
            boss.LISTEN(tier::release, hids::events::mouse::button::dblclick::left, gear)
            {
                boss.RISEUP(tier::release, e2::form::maximize, gear);
                gear.dismiss();
            };
            //boss.LISTEN(tier::release, hids::events::mouse::button::click::leftright, gear)
            //{
            //    boss.RISEUP(tier::release, e2::form::quit, boss.This());
            //    gear.dismiss();
            //};
            //boss.LISTEN(tier::release, hids::events::mouse::button::click::middle, gear)
            //{
            //    boss.RISEUP(tier::release, e2::form::quit, boss.This());
            //    gear.dismiss();
            //};
        };
        auto app_window = [](auto& what)
        {
            return ui::fork::ctor(axis::Y)
                    ->template plugin<pro::title>(""/*not used here*/, what.footer, true, faux, true)
                    ->template plugin<pro::limit>(twod{ 10,-1 }, twod{ -1,-1 })
                    ->template plugin<pro::light>()
                    ->isroot(true)
                    ->active()
                    ->invoke([&](auto& boss)
                    {
                        boss.keybd.active();
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
                                //todo master == boss
                                auto& master = *master_ptr;
                                auto& applet = *applet_ptr;

                                auto deed = master.bell::template protos<tier::release>();
                                if (deed != hids::events::mouse::button::drag::start::left.id
                                 && deed != hids::events::mouse::button::drag::start::leftright.id) return;

                                // Reset restoring callback.
                                master.SIGNAL(tier::release, e2::form::restore, e2::form::restore.param());

                                // Take current title.
                                auto what = vtm::events::handoff.param({ .menuid = menuid });
                                master.SIGNAL(tier::request, e2::form::prop::ui::header, what.header);
                                master.SIGNAL(tier::request, e2::form::prop::ui::footer, what.footer);
                                if (what.header.empty()) what.header = menuid;

                                // Find creator.
                                auto world_ptr = e2::config::creator.param();
                                master.RISEUP(tier::request, e2::config::creator, world_ptr);

                                // Take coor and detach from the tiling wm.
                                gear.coord -= applet.base::coor(); // Localize mouse coor.
                                what.square.size = applet.base::size();
                                applet.global(what.square.coor);
                                what.square.coor = -what.square.coor;
                                what.forced = true;
                                what.applet = applet_ptr;
                                master.SIGNAL(tier::preview, e2::form::proceed::detach, applet_ptr);
                                applet.moveto(dot_00);

                                // Attach to the world.
                                world_ptr->SIGNAL(tier::request, vtm::events::handoff, what);
                                
                                gear.kb_offer_9(what.applet); // Pass focus.
                                gear.annul_kb_focus(master_ptr); // Remove focus.

                                // Destroy placeholder.
                                master.RISEUP(tier::release, e2::form::quit, master_ptr);

                                // Handover mouse input.
                                master.SIGNAL(tier::release, hids::events::notify::mouse::leave, gear);
                                applet.SIGNAL(tier::release, hids::events::notify::mouse::enter, gear);
                                gear.pass<tier::release>(what.applet, dot_00);
                            }
                        };
                        boss.LISTEN(tier::anycast, e2::form::upon::started, root)
                        {
                            boss.RISEUP(tier::release, events::enlist, boss.This()); //todo "template" keyword is required by gcc
                        };
                    })
                    //->branch(slot::_1, ui::post_fx<cell::shaders::contrast>::ctor()) //todo apple clang doesn't get it
                    ->branch(slot::_1,
                        ui::post_fx::ctor()
                        ->upload(what.header)
                        ->invoke([&](auto& boss)
                        {
                            auto shadow = ptr::shadow(boss.This());
                            boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent, -, (shadow))
                            {
                                parent->LISTEN(tier::preview, e2::form::prop::ui::header, newtext, -, (shadow))
                                {
                                    if (auto boss_ptr = shadow.lock())
                                    {
                                        boss_ptr->upload(newtext);
                                        boss_ptr->parent()->SIGNAL(tier::release, e2::form::prop::ui::header, newtext);
                                    }
                                };
                                parent->LISTEN(tier::request, e2::form::prop::ui::header, curtext, -, (shadow))
                                {
                                    if (auto boss_ptr = shadow.lock()) curtext = boss_ptr->get_source();
                                };
                            };
                        }))
                    ->branch(slot::_2, what.applet);
        };
        auto pass_focus = [](auto& gear_id_list, auto item_ptr)
        {
            if (item_ptr)
            {
                for (auto gear_id : gear_id_list)
                {
                    if (auto gear_ptr = bell::getref<hids>(gear_id))
                    {
                        gear_ptr->kb_offer_9(item_ptr);
                    }
                }
            }
        };
        auto built_node = [](auto tag, auto slot1, auto slot2, auto grip_width)
        {
            auto node = tag == 'h' ? ui::fork::ctor(axis::X, grip_width == -1 ? 2 : grip_width, slot1, slot2)
                                   : ui::fork::ctor(axis::Y, grip_width == -1 ? 1 : grip_width, slot1, slot2);
            node->isroot(faux, 1) // Set object kind to 1 to be different from others. See empty_slot::select.
                ->template plugin<pro::limit>(dot_00)
                ->invoke([&](auto& boss)
                {
                    mouse_subs(boss);
                    boss.LISTEN(tier::release, app::tile::events::ui::swap    , gear) { boss.swap();       };
                    boss.LISTEN(tier::release, app::tile::events::ui::rotate  , gear) { boss.rotate();     };
                    boss.LISTEN(tier::release, app::tile::events::ui::equalize, gear) { boss.config(1, 1); };
                });
                auto grip = node->attach(slot::_I,
                                ui::mock::ctor()
                                ->isroot(true)
                                ->template plugin<pro::mover>()
                                ->template plugin<pro::focus>()
                                //->template plugin<pro::shade<cell::shaders::xlight>>() //todo apple clang doesn't get it
                                ->template plugin<pro::shade>()
                                ->invoke([&](auto& boss)
                                {
                                    boss.keybd.accept(true);
                                    anycasting(boss);
                                    //todo implement keydb support
                                })
                                ->active());
            return node;
        };
        auto empty_pane = []
        {
            auto menu_black = skin::color(tone::menu_black);
            auto cC = menu_black;

            return ui::park::ctor()
                ->isroot(true, 2)
                ->colors(cC.fgc(), cC.bgc())
                ->plugin<pro::limit>(dot_00, -dot_11)
                ->plugin<pro::focus>()
                ->invoke([&](auto& boss)
                {
                    boss.keybd.accept(true);
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
                    snap::center, snap::center,
                    ui::post::ctor()->upload("Empty Slot", 10)
                );
        };
        auto empty_slot = [](auto&& empty_slot) -> sptr<ui::veer>
        {
            return ui::veer::ctor()
                ->invoke([&](auto& boss)
                {
                    boss.LISTEN(tier::release, e2::config::plugins::sizer::alive, state)
                    {
                        // Block rising up this event: DTVT object fires this event on exit.
                    };
                    boss.LISTEN(tier::release, vtm::events::d_n_d::abort, target)
                    {
                        auto menu_black = skin::color(tone::menu_black);
                        auto cC = menu_black;
                        auto count = boss.count();
                        if (count == 1) // Only empty slot available.
                        {
                            //todo unify
                            boss.back()->color(cC.fgc(), cC.bgc());
                            boss.deface();
                        }
                    };
                    boss.LISTEN(tier::release, vtm::events::d_n_d::ask, target)
                    {
                        auto count = boss.count();
                        if (count == 1) // Only empty slot available.
                        {
                            auto highlight_color = skin::color(tone::highlight);
                            auto c3 = highlight_color;
                            auto x3 = cell{ c3 }.alpha(0x00);
                            auto fg = c3.fgc();
                            auto bg = c3.bgc();
                            fg.alpha(0x70);
                            bg.alpha(0x70);
                            boss.back()->color(fg, bg);
                            target = boss.This();
                            boss.deface();
                        }
                    };
                    boss.LISTEN(tier::release, vtm::events::d_n_d::drop, what)
                    {
                        auto menu_black = skin::color(tone::menu_black);
                        auto cC = menu_black;
                        auto count = boss.count();
                        if (count == 1) // Only empty slot available.
                        {
                            //todo unify
                            boss.back()->color(cC.fgc(), cC.bgc());
                            auto app = app_window(what);
                            boss.attach(app);
                            app->SIGNAL(tier::anycast, e2::form::upon::started, app);
                        }
                    };
                    boss.LISTEN(tier::release, e2::form::proceed::swap, item_ptr)
                    {
                        auto count = boss.count();
                        if (count == 1) // Only empty slot available.
                        {
                            log("tile: empty_slot swap: defective structure, count=", count);
                        }
                        else if (count == 2)
                        {
                            auto my_item_ptr = boss.pop_back();
                            if (item_ptr)
                            {
                                boss.attach(item_ptr);
                            }
                            else item_ptr = boss.This(); // Heir to the focus.
                        }
                        else log("tile: empty_slot swap: defective structure, count=", count);
                    };
                    boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent)
                    {
                        auto parent_memo = std::make_shared<subs>();
                        parent->LISTEN(tier::request, e2::form::proceed::swap, item_ptr, *parent_memo)
                        {
                            if (item_ptr != boss.This())
                            {
                                auto count = boss.count();
                                if (count == 1) // Only empty slot available.
                                {
                                    item_ptr.reset();
                                }
                                else if (count == 2)
                                {
                                    item_ptr = boss.pop_back();
                                }
                                else log("tile:  empty_slot: defective structure, count=", count);
                                if (auto parent = boss.parent())
                                {
                                    parent->bell::template expire<tier::request>();
                                }
                            }
                        };
                        boss.LISTEN(tier::request /*swap specific*/, e2::form::upon::vtree::detached, parent, *parent_memo, (parent_memo))
                        {
                            parent_memo.reset();
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
                        if (item_ptr->base::kind() != 1) gear.kb_offer_9(item_ptr);
                        else                             gear.annul_kb_focus(item_ptr); // Exclude grips.
                    };
                    boss.LISTEN(tier::release, e2::form::maximize, gear, -, (oneoff = subs{}))
                    {
                        auto count = boss.count();
                        if (count > 2) // It is a root.
                        {
                            auto item_ptr = boss.pop_back();
                            item_ptr->SIGNAL(tier::release, e2::form::restore, item_ptr);
                            return;
                        }
                        if (oneoff)
                        {
                            boss.RISEUP(tier::release, e2::form::proceed::attach, e2::form::proceed::attach.param());
                            return;
                        }
                        if (count > 1) // Preventing the empty slot from maximizing.
                        {
                            //todo revise
                            if (boss.back()->base::kind() == 0) // Preventing the splitter from maximizing.
                            {
                                // Pass the focus to the maximized window.
                                gear.kb_offer_15(boss.back());
                                auto fullscreen_item = boss.pop_back();
                                if (fullscreen_item)
                                {
                                    fullscreen_item->LISTEN(tier::release, e2::form::restore, item_ptr, oneoff)
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
                                    boss.RISEUP(tier::release, e2::form::proceed::attach, fullscreen_item);
                                    boss.base::reflow();
                                }
                            }
                        }
                    };
                    boss.LISTEN(tier::release, app::tile::events::ui::split::any, gear)
                    {
                        if (auto deed = boss.bell::template protos<tier::release>())
                        {
                            auto depth = e2::depth.param();
                            boss.RISEUP(tier::request, e2::depth, depth, true);
                            log("tile: depth=", depth);
                            if (depth > INHERITANCE_LIMIT) return;

                            auto heading = deed == app::tile::events::ui::split::vt.id;
                            auto newnode = built_node(heading ? 'v':'h', 1, 1, heading ? 1 : 2);
                            auto empty_1 = empty_slot(empty_slot);
                            auto empty_2 = empty_slot(empty_slot);
                            auto curitem = boss.pop_back(); // In order to preserve all foci.
                            gear.kb_offer_4(empty_2);
                            gear.annul_kb_focus(curitem);
                            if (boss.empty())
                            {
                                boss.attach(empty_pane());
                                empty_1->pop_back();
                            }
                            auto slot_1 = newnode->attach(slot::_1, empty_1->branch(curitem));
                            auto slot_2 = newnode->attach(slot::_2, empty_2);
                            boss.attach(newnode);
                        }
                    };
                    boss.LISTEN(tier::anycast, e2::form::quit, nested_item_ptr)
                    {
                        boss.SIGNAL(tier::preview, e2::form::quit, nested_item_ptr);
                    };
                    boss.LISTEN(tier::preview, e2::form::quit, nested_item_ptr)
                    {
                        if (boss.count() > 1 && boss.back()->base::kind() == 0)
                        {
                            boss.back()->SIGNAL(tier::anycast, e2::form::quit, nested_item_ptr);
                        }
                        else boss.SIGNAL(tier::release, e2::form::quit, nested_item_ptr);
                    };
                    boss.LISTEN(tier::release, e2::form::quit, nested_item_ptr)
                    {
                        if (nested_item_ptr)
                        {
                            boss.deface();
                            auto& item = *nested_item_ptr;
                            auto gear_id_list = e2::form::state::keybd::enlist.param();
                            item.SIGNAL(tier::anycast, e2::form::state::keybd::enlist, gear_id_list);
                            auto count = boss.count();
                            if (count > 1)
                            {
                                if (boss.back()->base::kind() == 0) // Only apps can be deleted.
                                {
                                    auto item = boss.pop_back(); // Throw away.
                                    pass_focus(gear_id_list, boss.back());
                                }
                            }
                            else if (count == 1) // Remove empty slot, reorganize.
                            {
                                if (auto parent = boss.base::parent())
                                {
                                    auto item_ptr = e2::form::proceed::swap.param(boss.This()); // sptr must be of the same type as the event argument. Casting kills all intermediaries when return.
                                    parent->SIGNAL(tier::request, e2::form::proceed::swap, item_ptr);
                                    if (item_ptr)
                                    {
                                        if (item_ptr != boss.This()) // Parallel slot is not empty.
                                        {
                                            parent->RISEUP(tier::release, e2::form::proceed::swap, item_ptr);
                                            pass_focus(gear_id_list, item_ptr);
                                        }
                                        else // I'm alone.
                                        {
                                            // Nothing todo. There can be only one!
                                        }
                                    }
                                    else // Both slots are empty.
                                    {
                                        parent->RISEUP(tier::release, e2::form::proceed::swap, item_ptr);
                                        pass_focus(gear_id_list, item_ptr);
                                    }
                                }
                            }
                        }
                    };
                    boss.LISTEN(tier::request, e2::form::proceed::createby, gear)
                    {
                        static auto insts_count = 0;
                        if (boss.count() == 1) // Create new apps at the empty slots only.
                        {
                            auto& gate = gear.owner;
                            auto current_default = e2::data::changed.param();
                            gate.SIGNAL(tier::request, e2::data::changed, current_default);
                            auto config = vtm::events::newapp.param({ .menuid = current_default });
                            gate.RISEUP(tier::request, vtm::events::newapp, config);
                            auto app = app_window(config);
                            gear.remove_from_kb_focus(boss.back()); // Take focus from the empty slot.
                            boss.attach(app);
                            if (auto world_ptr = gate.parent())
                            {
                                app->SIGNAL(tier::anycast, vtm::events::attached, world_ptr);
                            }

                            insts_count++; //todo unify, demo limits
                            config.applet->LISTEN(tier::release, e2::dtor, id)
                            {
                                insts_count--;
                                log("tile: inst: detached: ", insts_count, " id=", id);
                            };

                            app->SIGNAL(tier::anycast, e2::form::upon::started, app);
                            if (gear.meta(hids::anyCtrl)) gear.kb_offer_4(app);
                            else                          gear.kb_offer_10(app);
                        }
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
        auto parse_data = [](auto&& parse_data, view& utf8) -> sptr<ui::veer>
        {
            auto slot = empty_slot(empty_slot);
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
                auto slot1 = node->attach(ui::slot::_1, parse_data(parse_data, utf8));
                auto slot2 = node->attach(ui::slot::_2, parse_data(parse_data, utf8));
                slot->attach(node);

                utf::trim_front(utf8, ") ");
            }
            else  // Add application.
            {
                utf::trim_front(utf8, " ");
                auto menuid = text{ utf::get_tail(utf8, " ,)") };
                if (menuid.empty()) return slot;

                utf::trim_front(utf8, " ,");
                if (utf8.size() && utf8.front() == ')') utf8.remove_prefix(1); // pop ')';

                auto oneoff = ptr::shared(hook{});
                slot->LISTEN(tier::anycast, vtm::events::attached, world_ptr, *oneoff, (oneoff, menuid, slot))
                {
                    auto what = vtm::events::newapp.param({ .menuid = menuid });
                    world_ptr->SIGNAL(tier::request, vtm::events::newapp, what);
                    auto inst = app_window(what);
                    slot->attach(inst);
                    inst->SIGNAL(tier::anycast, vtm::events::attached, world_ptr);
                    oneoff.reset();
                };
            }
            return slot;
        };
        auto build_inst = [](text cwd, view param, xmls& config, text patch) -> sptr<base>
        {
            auto menu_white = skin::color(tone::menu_white);
            auto cB = menu_white;

            auto object = ui::fork::ctor(axis::Y)
                //todo foci
                //->plugin<pro::focus>(faux)
                ->plugin<items>()
                ->invoke([&](auto& boss)
                {
                    //todo foci
                    //boss.keybd.master();
                    auto oneoff = ptr::shared(hook{});
                    boss.LISTEN(tier::anycast, e2::form::upon::created, gear, *oneoff, (oneoff))
                    {
                        auto& gate = gear.owner;
                        auto menuid = e2::data::changed.param();
                        gate.SIGNAL(tier::request, e2::data::changed, menuid);
                        auto conf_list_ptr = desk::events::menu.param();
                        gate.RISEUP(tier::request, desk::events::menu, conf_list_ptr);
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
                        { std::make_shared<menu::item>(menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{ { .label = "  ┐└  ", .notes = " Maximize/restore active pane " } }}),
                        [](ui::pads& boss, auto& item)
                        {
                            boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                gear.countdown = 1;
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::toggle, gear);
                                gear.dismiss(true);
                            };
                        }},
                        { std::make_shared<menu::item>(menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{ { .label = "  +  ", .notes = " Create and run a new app in active panes " } }}),
                        [](ui::pads& boss, auto& item)
                        {
                            boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::create, gear);
                                gear.dismiss(true);
                            };
                        }},
                        { std::make_shared<menu::item>(menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{ { .label = " ::: ", .notes = " Select all panes " } }}),
                        [](ui::pads& boss, auto& item)
                        {
                            boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::select, gear);
                                gear.dismiss(true);
                            };
                        }},
                        { std::make_shared<menu::item>(menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{ { .label = "  │  ", .notes = " Split active panes horizontally " } }}),
                        [](ui::pads& boss, auto& item)
                        {
                            boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::split::hz, gear);
                                gear.dismiss(true);
                            };
                        }},
                        { std::make_shared<menu::item>(menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{ { .label = " ── ", .notes = " Split active panes vertically " } }}),
                        [](ui::pads& boss, auto& item)
                        {
                            boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::split::vt, gear);
                                gear.dismiss(true);
                            };
                        }},
                        { std::make_shared<menu::item>(menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{ { .label = "  ┌┘  ", .notes = " Change split orientation " } }}),
                        [](ui::pads& boss, auto& item)
                        {
                            boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::rotate, gear);
                                gear.dismiss(true);
                            };
                        }},
                        { std::make_shared<menu::item>(menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{ { .label = " <-> ", .notes = " Swap two or more panes " } }}),
                        [](ui::pads& boss, auto& item)
                        {
                            boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::swap, gear);
                                gear.dismiss(true);
                            };
                        }},
                        { std::make_shared<menu::item>(menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{ { .label = " >|< ", .notes = " Equalize split ratio " } }}),
                        [](ui::pads& boss, auto& item)
                        {
                            boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::equalize, gear);
                                gear.dismiss(true);
                            };
                        }},
                        { std::make_shared<menu::item>(menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{ { .label = "  ×  ", .notes = " Close active app or remove pane if there is no running app " } }}),
                        [](ui::pads& boss, auto& item)
                        {
                            boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::close, gear);
                                gear.dismiss(true);
                            };
                        }},
                    });
            object->attach(slot::_1, menu_block)
                  ->plugin<pro::focus>()
                  ->invoke([](auto& boss)
                  {
                      boss.keybd.active();
                      boss.LISTEN(tier::anycast, e2::form::quit, item)
                      {
                          boss.RISEUP(tier::release, e2::form::quit, item);
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
                if (err) log("tile: failed to change current working directory to '", cwd, "', error code: ", err.value());
                else     log("tile: change current working directory to '", cwd, "'");
            }

            object->attach(slot::_2, parse_data(parse_data, param))
                ->invoke([&](auto& boss)
                {
                    boss.LISTEN(tier::release, e2::form::proceed::attach, fullscreen_item)
                    {
                        if (boss.count() > 2)
                        {
                            auto item_ptr = boss.pop_back();
                            item_ptr->SIGNAL(tier::release, e2::form::restore, item_ptr);
                        }

                        if (fullscreen_item)
                        {
                            boss.attach(fullscreen_item);
                            fullscreen_item.reset();
                        }
                        else log("tile: fullscreen_item is empty");
                    };
                    boss.LISTEN(tier::anycast, app::tile::events::ui::any, gear)
                    {
                        if (auto deed = boss.bell::template protos<tier::anycast>()) //todo "template" keyword is required by clang 13.0.0
                        {
                            if (boss.count() > 2 && deed != app::tile::events::ui::toggle.id) // Restore the window before any action if maximized.
                            {
                                auto item_ptr = boss.pop_back();
                                item_ptr->SIGNAL(tier::release, e2::form::restore, item_ptr);
                            }

                            if (deed == app::tile::events::ui::swap.id)
                            {
                                auto empty_slot_list = backups{};
                                auto proc = e2::form::proceed::functor.param([&](sptr<base> item_ptr)
                                {
                                    auto gear_test = e2::form::state::keybd::find.param(gear.id, 0);
                                    item_ptr->SIGNAL(tier::request, e2::form::state::keybd::find, gear_test);
                                    if (gear_test.second)
                                    {
                                        item_ptr->RISEUP(tier::release, events::backup, empty_slot_list);
                                    }
                                });
                                boss.SIGNAL(tier::general, e2::form::proceed::functor, proc);
                                auto slots_count = empty_slot_list.size();
                                log("tile: slots count=", slots_count);
                                if (slots_count >= 2) // Swap selected panes cyclically.
                                {
                                    using slot = sptr<base>;
                                    log("tile: Swap slots cyclically");
                                    auto i = 0;
                                    auto emp_slot = slot{};
                                    auto app_slot = slot{};
                                    auto emp_next = slot{};
                                    auto app_next = slot{};
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
                    boss.LISTEN(tier::release, hids::events::upevent::kboffer, gear)
                    {
                        // Set focus to all panes.
                        boss.SIGNAL(tier::anycast, app::tile::events::ui::select, gear);
                        gear.dismiss(true);
                        //todo foci
                        //todo restore focused state
                        //log("kb offer");
                    };
                    //todo foci
                    //boss.LISTEN(tier::release, e2::form::state::keybd::lost, gear)
                    //{
                    //    //log("kb lost");
                    //};
                    //boss.LISTEN(tier::release, e2::form::state::keybd::got, gear)
                    //{
                    //    //log("kb got");
                    //};
                });
            return object;
        };
    }

    app::shared::initialize builder{ app::tile::id, build_inst };
}