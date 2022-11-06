// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_APP_TILE_HPP
#define NETXS_APP_TILE_HPP

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
            EVENT_XS( backup, app::tile::backups  ),
            EVENT_XS( enlist, sptr<console::base> ),
            EVENT_XS( delist, bool                ),
            GROUP_XS( ui    , input::hids         ), // Window manager command pack.

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

            boss.SUBMIT_T(tier::release, e2::size::any, memo, newsz)
            {
                if (client)
                {
                    auto new_coor = twod{ newsz.x + 2/*todo resize grip width*/, 0 };
                    auto new_size = twod{ client->size().x, newsz.y };
                    client->base::moveto(new_coor);
                    client->base::resize(new_size);
                }
            };
            boss.SUBMIT_T(tier::release, events::enlist, memo, object)
            {
                if (!client) return;
                auto label = [](auto data_src_sptr, auto header)
                {
                    auto highlight_color = skin::color(tone::highlight);
                    auto c3 = highlight_color;
                    auto x3 = cell{ c3 }.alpha(0x00);

                    return ui::pads::ctor(dent{ 1, 1, 0, 0 }, dent{})
                        ->plugin<pro::fader>(x3, c3, skin::timeout(tone::fader))
                        ->branch(ui::item::ctor(header.empty() ? "- no title -" : header))
                        ->invoke([&](auto& boss)
                        {
                            auto boss_shadow = ptr::shadow(boss.This());
                            auto data_shadow = ptr::shadow(data_src_sptr);

                            boss.SUBMIT_T_BYVAL(tier::release, e2::form::upon::vtree::attached, boss.tracker, parent)
                            {
                                if (auto data_ptr = data_shadow.lock())
                                {
                                    auto state = e2::form::highlight::set.param();
                                    data_ptr->SIGNAL(tier::anycast, e2::form::highlight::set, state);
                                }
                            };
                            data_src_sptr->SUBMIT_T(tier::preview, e2::form::highlight::any, boss.tracker, state)
                            {
                                auto highlight_color = skin::color(tone::highlight);
                                auto c3 = highlight_color;
                                auto x3 = cell{ c3 }.alpha(0x00);
                                boss.color(state ? 0xFF00ff00 : x3.fgc(), x3.bgc());
                            };
                            data_src_sptr->SUBMIT_T(tier::release, events::delist, boss.tracker, object)
                            {
                                boss.detach(); // Destroy itself.
                            };
                            boss.SUBMIT_T_BYVAL(tier::release, hids::events::mouse::button::any, boss.tracker, gear)
                            {
                                if (auto boss_ptr = boss_shadow.lock())
                                if (auto data_ptr = data_shadow.lock())
                                {
                                    auto deed = boss_ptr->bell::template protos<tier::release>(); //todo "template" keyword is required by FreeBSD clang 11.0.1
                                    data_ptr->template signal<tier::release>(deed, gear); //todo "template" keyword is required by gcc
                                }
                            };
                            boss.SUBMIT_T_BYVAL(tier::release, e2::form::state::mouse, boss.tracker, active)
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
            boss.SUBMIT_T(tier::release, e2::render::any, memo, parent_canvas)
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
            boss.SUBMIT_T(tier::anycast, e2::form::upon::started, memo, root)
            {
                client->clear();
                depth = 0;
                boss.base::template riseup<tier::request>(e2::depth, depth, true);
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
            boss.SUBMIT(tier::release, e2::form::upon::vtree::attached, parent)
            {
                auto parent_memo = std::make_shared<subs>();
                parent->SUBMIT_T(tier::anycast, app::tile::events::ui::any, *parent_memo, gear)
                {
                    auto gear_test = e2::form::state::keybd::find.param(gear.id, 0);
                    boss.SIGNAL(tier::anycast, e2::form::state::keybd::find, gear_test);
                    if (gear_test.second)
                    {
                        if (auto parent = boss.parent())
                        if (auto deed = parent->bell::template protos<tier::anycast>()) //todo "template" keyword is required by FreeBSD clang 11.0.1
                        {
                            switch (deed)
                            {
                                case app::tile::events::ui::create.id:
                                    gear.force_group_focus = true;
                                    boss.template riseup<tier::release>(e2::form::proceed::createby, gear);
                                    gear.force_group_focus = faux;
                                    break;
                                case app::tile::events::ui::close.id:
                                    boss.template riseup<tier::release>(e2::form::quit, boss.This());
                                    break;
                                case app::tile::events::ui::toggle.id:
                                    if (boss.base::kind() == 0) // Only apps can be maximized.
                                    if (gear.countdown > 0)
                                    {
                                        gear.countdown--;
                                        // Removing multifocus - The only one can be maximized if several are selected.
                                        gear.force_group_focus = faux;
                                        gear.kb_focus_changed = faux;
                                        boss.SIGNAL(tier::release, hids::events::upevent::kboffer, gear);
                                        boss.template riseup<tier::release>(e2::form::maximize, gear);
                                        //todo parent_memo is reset by the empty slot here (pop_back), undefined behavior from here
                                    }
                                    break;
                                case app::tile::events::ui::swap.id:
                                    if (gear.countdown > 0)
                                    {
                                        boss.template riseup<tier::release>(app::tile::events::ui::swap, gear);
                                    }
                                    break;
                                case app::tile::events::ui::rotate.id:
                                    boss.template riseup<tier::release>(app::tile::events::ui::rotate, gear);
                                    break;
                                case app::tile::events::ui::equalize.id:
                                    boss.template riseup<tier::release>(app::tile::events::ui::equalize, gear);
                                    break;
                                case app::tile::events::ui::split::vt.id:
                                    boss.template riseup<tier::release>(app::tile::events::ui::split::vt, gear);
                                    break;
                                case app::tile::events::ui::split::hz.id:
                                    boss.template riseup<tier::release>(app::tile::events::ui::split::hz, gear);
                                    break;
                            }
                        }
                    }
                };
                boss.SUBMIT_T_BYVAL(tier::release, e2::form::upon::vtree::detached, *parent_memo, parent)
                {
                    parent_memo.reset();
                };
            };
        };
        auto mouse_subs = [](auto& boss)
        {
            boss.SUBMIT(tier::release, hids::events::mouse::button::dblclick::left, gear)
            {
                boss.base::template riseup<tier::release>(e2::form::maximize, gear);
                gear.dismiss();
            };
            //boss.SUBMIT(tier::release, hids::events::mouse::button::click::leftright, gear)
            //{
            //    boss.base::template riseup<tier::release>(e2::form::quit, boss.This());
            //    gear.dismiss();
            //};
            //boss.SUBMIT(tier::release, hids::events::mouse::button::click::middle, gear)
            //{
            //    boss.base::template riseup<tier::release>(e2::form::quit, boss.This());
            //    gear.dismiss();
            //};
        };
        auto app_window = [](view header, view footer, auto branch, auto menu_item_id)
        {
            return ui::fork::ctor(axis::Y)
                    ->plugin<pro::title>(""/*not used here*/, footer, true, faux, true)
                    ->plugin<pro::limit>(twod{ 10,-1 }, twod{ -1,-1 })
                    ->plugin<pro::light>()
                    ->isroot(true)
                    ->active()
                    ->invoke([&](auto& boss)
                    {
                        anycasting(boss);
                        mouse_subs(boss);

                        if (branch->size() != dot_00) boss.resize(branch->size() + dot_01/*approx title height*/);

                        auto master_shadow = ptr::shadow(boss.This());
                        auto branch_shadow = ptr::shadow(branch);
                        boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::drag::start::any, gear)
                        {
                            if (auto branch_ptr = branch_shadow.lock())
                            if (branch_ptr->area().hittest(gear.coord))
                            if (auto master_ptr = master_shadow.lock())
                            {
                                auto& master = *master_ptr;
                                auto& branch = *branch_ptr;

                                auto deed = master.bell::template protos<tier::release>();
                                if (deed != hids::events::mouse::button::drag::start::left.id
                                 && deed != hids::events::mouse::button::drag::start::leftright.id) return;

                                // Reset restoring callback.
                                master.SIGNAL(tier::release, e2::form::restore, e2::form::restore.param());

                                // Take current title.
                                auto what = e2::form::proceed::createfrom.param();
                                what.menuid = menu_item_id;
                                master.SIGNAL(tier::request, e2::form::prop::ui::header, what.header);
                                master.SIGNAL(tier::request, e2::form::prop::ui::footer, what.footer);
                                if (what.header.empty()) what.header = menu_item_id;

                                // Take coor and detach from the tiling wm.
                                gear.coord -= branch.base::coor(); // Localize mouse coor.
                                what.square.size = branch.base::size();
                                branch.global(what.square.coor);
                                what.square.coor = -what.square.coor;
                                what.forced = true;
                                what.object = branch_ptr;
                                master.SIGNAL(tier::preview, e2::form::proceed::detach, branch_ptr);
                                branch.moveto(dot_00);

                                // Attach to the world.
                                auto world_ptr = e2::config::whereami.param();
                                SIGNAL_GLOBAL(e2::config::whereami, world_ptr);
                                world_ptr->SIGNAL(tier::release, e2::form::proceed::createfrom, what);

                                // Pass unique focus.
                                auto& object = *what.object;
                                //todo unify
                                gear.kb_focus_changed = faux;
                                gear.force_group_focus = faux;
                                gear.combine_focus = true;
                                object.SIGNAL(tier::release, hids::events::upevent::kboffer, gear);
                                gear.combine_focus = faux;
                                gear.force_group_focus = faux;

                                // Destroy placeholder.
                                master.base::template riseup<tier::release>(e2::form::quit, master_ptr);

                                // Handover mouse input.
                                master.SIGNAL(tier::release, hids::events::notify::mouse::leave, gear);
                                object.SIGNAL(tier::release, hids::events::notify::mouse::enter, gear);
                                gear.pass<tier::release>(what.object, dot_00);
                            }
                        };

                        boss.SUBMIT(tier::anycast, e2::form::upon::started, root)
                        {
                            boss.template riseup<tier::release>(events::enlist, boss.This()); //todo "template" keyword is required by gcc
                        };
                    })
                    //->branch(slot::_1, ui::post_fx<cell::shaders::contrast>::ctor()) //todo apple clang doesn't get it
                    ->branch(slot::_1,
                        ui::post_fx::ctor()
                        ->upload(header)
                        ->invoke([&](auto& boss)
                        {
                            auto shadow = ptr::shadow(boss.This());
                            boss.SUBMIT_BYVAL(tier::release, e2::form::upon::vtree::attached, parent)
                            {
                                parent->SUBMIT_BYVAL(tier::preview, e2::form::prop::ui::header, newtext)
                                {
                                    if (auto boss_ptr = shadow.lock())
                                    {
                                        boss_ptr->upload(newtext);
                                        boss_ptr->parent()->SIGNAL(tier::release, e2::form::prop::ui::header, newtext);
                                    }
                                };
                                parent->SUBMIT_BYVAL(tier::request, e2::form::prop::ui::header, curtext)
                                {
                                    if (auto ptr = shadow.lock()) curtext = ptr->get_source();
                                };
                            };
                        }))
                    ->branch(slot::_2, branch);
        };
        auto pass_focus = [](auto& gear_id_list, auto& item_ptr)
        {
            if (item_ptr)
            {
                for (auto gear_id : gear_id_list)
                {
                    if (auto ptr = bell::getref(gear_id))
                    if (auto gear_ptr = std::dynamic_pointer_cast<hids>(ptr))
                    {
                        gear_ptr->offer_kb_focus(item_ptr);
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
                    boss.SUBMIT(tier::release, app::tile::events::ui::swap    , gear) { boss.swap();       };
                    boss.SUBMIT(tier::release, app::tile::events::ui::rotate  , gear) { boss.rotate();     };
                    boss.SUBMIT(tier::release, app::tile::events::ui::equalize, gear) { boss.config(1, 1); };
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
                    boss.SUBMIT(tier::release, hids::events::mouse::button::click::right, gear)
                    {
                        boss.base::template riseup<tier::release>(e2::form::proceed::createby, gear);
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
                    auto shadow = ptr::shadow(boss.This());
                    boss.SUBMIT(tier::release, e2::config::plugins::sizer::alive, state)
                    {
                        // Block rising up this event: DTVT object fires this event on exit.
                    };
                    boss.SUBMIT(tier::release, e2::form::proceed::d_n_d::abort, target)
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
                    boss.SUBMIT(tier::release, e2::form::proceed::d_n_d::ask, target)
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
                    boss.SUBMIT(tier::release, e2::form::proceed::d_n_d::drop, what)
                    {
                        auto menu_black = skin::color(tone::menu_black);
                        auto cC = menu_black;
                        auto count = boss.count();
                        if (count == 1) // Only empty slot available.
                        {
                            //todo unify
                            boss.back()->color(cC.fgc(), cC.bgc());
                            auto app = app_window(what.header, what.footer, what.object, what.menuid);
                            boss.attach(app);
                            app->SIGNAL(tier::anycast, e2::form::upon::started, app);
                        }
                    };
                    boss.SUBMIT(tier::release, e2::form::proceed::swap, item_ptr)
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
                    boss.SUBMIT(tier::release, e2::form::upon::vtree::attached, parent)
                    {
                        //todo revise, possible parent subscription leaks when reattached
                        auto parent_memo = std::make_shared<subs>();
                        parent->SUBMIT_T(tier::request, e2::form::proceed::swap, *parent_memo, item_ptr)
                        {
                            if (item_ptr != boss.This()) // It wasn't me. It was the one-armed man.
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
                        boss.SUBMIT_T_BYVAL(tier::request /*swap specific*/, e2::form::upon::vtree::detached, *parent_memo, parent)
                        {
                            parent_memo.reset();
                        };
                    };
                    boss.SUBMIT(tier::anycast, e2::form::upon::started, root)
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
                    boss.SUBMIT(tier::anycast, app::tile::events::ui::select, gear)
                    {
                        auto& item = *boss.back();
                        if (item.base::kind() != 1)
                        {
                            //todo unify
                            gear.force_group_focus = true;
                            gear.kb_focus_changed = faux;
                            gear.combine_focus = true;
                            item.SIGNAL(tier::release, hids::events::upevent::kboffer, gear);
                            gear.combine_focus = faux;
                            gear.force_group_focus = faux;
                        }
                        else item.SIGNAL(tier::release, hids::events::upevent::kbannul, gear); // Exclude grips.
                    };
                    hook oneoff; // One-time return ticket.
                    boss.SUBMIT_BYVAL(tier::release, e2::form::maximize, gear)
                    {
                        if (auto boss_ptr = shadow.lock())
                        {
                            auto& boss = *boss_ptr;
                            auto count = boss.count();
                            if (count > 2) // It is a root.
                            {
                                auto item_ptr = boss.pop_back();
                                item_ptr->SIGNAL(tier::release, e2::form::restore, item_ptr);
                                return;
                            }
                            if (oneoff)
                            {
                                boss.template riseup<tier::release>(e2::form::proceed::attach, e2::form::proceed::attach.param()); //todo "template" is required by gcc
                                return;
                            }
                            if (count > 1) // Preventing the empty slot from maximizing.
                            {
                                //todo revise
                                if (boss.back()->base::kind() == 0) // Preventing the splitter from maximizing.
                                {
                                    // Pass the focus to the maximized window.
                                    //todo unify
                                    gear.force_group_focus = faux;
                                    gear.kb_focus_changed = faux;
                                    gear.combine_focus = true;
                                    boss.back()->SIGNAL(tier::release, hids::events::upevent::kboffer, gear);
                                    gear.combine_focus = faux;
                                    gear.force_group_focus = faux;

                                    auto fullscreen_item = boss.pop_back();
                                    if (fullscreen_item)
                                    {
                                        fullscreen_item->SUBMIT_T(tier::release, e2::form::restore, oneoff, item_ptr)
                                        {
                                            if (item_ptr)
                                            {
                                                boss.attach(item_ptr);
                                                boss.base::reflow();
                                            }
                                            oneoff.reset();
                                        };
                                        boss.base::template riseup<tier::release>(e2::form::proceed::attach, fullscreen_item);
                                        boss.base::reflow();
                                    }
                                }
                            }
                        }
                    };
                    boss.SUBMIT_BYVAL(tier::release, app::tile::events::ui::split::any, gear)
                    {
                        if (auto boss_ptr = shadow.lock())
                        {
                            auto& boss = *boss_ptr;
                            if (auto deed = boss.bell::template protos<tier::release>())
                            {
                                auto depth = e2::depth.param();
                                boss.base::template riseup<tier::request>(e2::depth, depth, true);
                                log("tile: depth=", depth);
                                if (depth > INHERITANCE_LIMIT) return;

                                auto heading = deed == app::tile::events::ui::split::vt.id;
                                auto newnode = built_node(heading ? 'v':'h', 1, 1, heading ? 1 : 2);
                                auto empty_1 = empty_slot(empty_slot);
                                auto empty_2 = empty_slot(empty_slot);
                                auto curitem = boss.pop_back(); // In order to preserve all foci.
                                gear.offer_kb_focus(empty_2);
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
                        }
                    };
                    boss.SUBMIT_BYVAL(tier::release, e2::form::quit, nested_item_ptr)
                    {
                        if (nested_item_ptr)
                        {
                            auto& item = *nested_item_ptr;
                            auto gear_id_list = e2::form::state::keybd::handover.param();
                            item.SIGNAL(tier::anycast, e2::form::state::keybd::handover, gear_id_list);

                            if (auto boss_ptr = shadow.lock())
                            {
                                auto& boss = *boss_ptr;
                                auto count = boss.count();
                                if (count > 1)
                                {
                                    if (boss.back()->base::kind() == 0) // Only apps can be deleted.
                                    {
                                        auto item = boss.pop_back(); // Throw away.
                                        pass_focus(gear_id_list, boss_ptr);
                                    }
                                }
                                else if (count == 1) // Remove empty slot, reorganize.
                                {
                                    if (auto parent = boss.base::parent())
                                    {
                                        auto item_ptr = e2::form::proceed::swap.param(boss_ptr); // sptr must be of the same type as the event argument. Casting kills all intermediaries when return.
                                        parent->SIGNAL(tier::request, e2::form::proceed::swap, item_ptr);
                                        if (item_ptr)
                                        {
                                            if (item_ptr != boss_ptr) // Parallel slot is not empty.
                                            {
                                                parent->base::template riseup<tier::release>(e2::form::proceed::swap, item_ptr);
                                                pass_focus(gear_id_list, item_ptr);
                                            }
                                            else // I'm alone.
                                            {
                                                // Nothing todo. There can be only one!
                                            }
                                        }
                                        else // Both slots are empty.
                                        {
                                            parent->base::template riseup<tier::release>(e2::form::proceed::swap, item_ptr);
                                            pass_focus(gear_id_list, item_ptr);
                                        }
                                    }
                                }
                            }
                        }
                    };
                    boss.SUBMIT(tier::release, e2::form::proceed::createby, gear)
                    {
                        static auto insts_count = 0;
                        if (boss.count() == 1) // Create new apps at the empty slots only.
                        {
                            if (gear.meta(hids::anyCtrl))
                            {
                                //todo ...
                            }
                            else
                            {
                                auto& gate = gear.owner;
                                auto current_default = e2::data::changed.param();
                                gate.SIGNAL(tier::request, e2::data::changed, current_default);

                                auto& conf_list = app::shared::get::configs();
                                auto config = conf_list[current_default];

                                auto& creator = app::shared::create::builder(config.type);
                                auto host = creator(config.cwd, config.param, config.settings);
                                auto app = app_window(config.title, "", host, current_default);
                                gear.remove_from_kb_focus(boss.back()); // Take focus from the empty slot.
                                boss.attach(app);

                                //todo unify, demo limits
                                {
                                    insts_count++;
                                    host->SUBMIT(tier::release, e2::dtor, id)
                                    {
                                        insts_count--;
                                        log("tile: inst: detached: ", insts_count, " id=", id);
                                    };
                                }
                                app->SIGNAL(tier::anycast, e2::form::upon::started, app);

                                //todo unify
                                gear.kb_focus_changed = faux;
                                host->SIGNAL(tier::release, hids::events::upevent::kboffer, gear);
                            }
                        }
                    };

                    boss.SUBMIT(tier::release, events::backup, empty_slot_list)
                    {
                        if (boss.count())
                        if (auto item_ptr = boss.back())
                        if (item_ptr->base::root())
                        {
                            empty_slot_list.push_back(boss.This());
                        }
                    };
                })
                ->branch
                (
                    empty_pane()
                );
        };
        auto parse_data = [](auto&& parse_data, view& utf8) -> sptr<ui::veer>
        {
            auto place = empty_slot(empty_slot);
            utf::trim_front(utf8, ", ");
            if (utf8.empty()) return place;
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
                    if (utf8.empty() || utf8.front() != ':') return place;
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
                    else return place;
                }
                if (utf8.empty() || utf8.front() != '(') return place;
                utf8.remove_prefix(1);
                auto node = built_node(tag, s1, s2, w);
                auto slot1 = node->attach(slot::_1, parse_data(parse_data, utf8));
                auto slot2 = node->attach(slot::_2, parse_data(parse_data, utf8));
                place->attach(node);

                utf::trim_front(utf8, ") ");
            }
            else  // Add application.
            {
                utf::trim_front(utf8, " ");
                auto app_id = utf::get_tail(utf8, " ,)");
                if (app_id.empty()) return place;

                utf::trim_front(utf8, " ,");
                if (utf8.size() && utf8.front() == ')') utf8.remove_prefix(1); // pop ')';

                auto& conf_list = app::shared::get::configs();
                auto iter = conf_list.find(app_id);
                if (iter == conf_list.end())
                {
                    log("tile: application id='", app_id, "' not found");
                    return place;
                }
                auto& config = iter->second;
                auto& creator = app::shared::create::builder(config.type);
                auto host = creator(config.cwd, config.param, config.settings);
                auto inst = app_window(config.title, config.footer, host, app_id);
                if (config.bgc)  inst->SIGNAL(tier::anycast, e2::form::prop::colors::bg,   config.bgc);
                if (config.fgc)  inst->SIGNAL(tier::anycast, e2::form::prop::colors::fg,   config.fgc);
                if (config.slimmenu) inst->SIGNAL(tier::anycast, e2::form::prop::ui::slimmenu, config.slimmenu);
                place->attach(inst);
            }
            return place;
        };
        auto build_inst = [](text cwd, view param, xml::settings& config) -> sptr<base>
        {
            auto menu_white = skin::color(tone::menu_white);
            auto cB = menu_white;

            auto object = ui::fork::ctor(axis::Y)
                        ->plugin<items>();

            object->invoke([&](auto& boss)
                {
                    auto oneoff = std::make_shared<hook>();
                    auto& conf_list = app::shared::get::configs();
                    auto objs_config_ptr = &conf_list;
                    boss.SUBMIT_T_BYVAL(tier::anycast, e2::form::upon::created, *oneoff, gear)
                    {
                        auto& gate = gear.owner;
                        auto& objs_config = *objs_config_ptr;
                        auto menu_item_id = e2::data::changed.param();
                        gate.SIGNAL(tier::request, e2::data::changed, menu_item_id);
                        //todo unify
                        auto& config = objs_config[menu_item_id];
                        if (config.type == menuitem_t::type_Region) // Reset the currently selected application to the previous one.
                        {
                            gate.SIGNAL(tier::preview, e2::data::changed, menu_item_id); // Get previous default;
                            gate.SIGNAL(tier::release, e2::data::changed, menu_item_id); // Set current  default;
                        }
                        oneoff.reset();
                    };
                });

            object->attach(slot::_1, app::shared::custom_menu(faux,
                    app::shared::menu_list_type
                    {
                        //  Green                                  ?Even    Red
                        // ┌────┐  ┌────┐  ┌─┬──┐  ┌────┐  ┌─┬──┐  ┌─┬──┐  ┌────┐  // ┌─┐  ┌─┬─┐  ┌─┬─┐  ┌─┬─┐  
                        // │Exec│  ├─┐  │  │ H  │  ├ V ─┤  │Swap│  │Fair│  │Shut│  // ├─┤  └─┴─┘  └<┴>┘  └>┴<┘  
                        // └────┘  └─┴──┘  └─┴──┘  └────┘  └─┴──┘  └─┴──┘  └────┘  // └─┘                       
                        { true,"  ┐└  ", " Maximize/restore active pane ", //  ─┐  ", //"  ▀█  ",
                        [](ui::pads& boss)
                        {
                            boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                gear.countdown = 1;
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::toggle, gear);
                                gear.dismiss(true);
                            };
                        }},
                        { true, "  +  ", " Create and run a new app in active panes ",
                        [](ui::pads& boss)
                        {
                            boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::create, gear);
                                gear.dismiss(true);
                            };
                        }},
                        { true, " ::: ", " Select all panes ",
                        [](ui::pads& boss)
                        {
                            boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::select, gear);
                                gear.dismiss(true);
                            };
                        }},
                        { true, "  │  ", " Split active panes horizontally ", // "  ║  ", - VGA Linux console doesn't support unicode glyphs
                        [](ui::pads& boss)
                        {
                            boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::split::hz, gear);
                                gear.dismiss(true);
                            };
                        }},
                        { true, " ── ", " Split active panes vertically ", // " ══ ", - VGA Linux console doesn't support unicode glyphs
                        [](ui::pads& boss)
                        {
                            boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::split::vt, gear);
                                gear.dismiss(true);
                            };
                        }},
                        { true, "  ┌┘  ", " Change split orientation ",
                        [](ui::pads& boss)
                        {
                            boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::rotate, gear);
                                gear.dismiss(true);
                            };
                        }},
                        { true, " <-> ", " Swap two or more panes ",
                        [](ui::pads& boss)
                        {
                            boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::swap, gear);
                                gear.dismiss(true);
                            };
                        }},
                        { true, " >|< ", " Equalize split ratio ",
                        [](ui::pads& boss)
                        {
                            boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::equalize, gear);
                                gear.dismiss(true);
                            };
                        }},
                        { true, "  ×  ", " Close active app or remove pane if there is no running app ",
                        [](ui::pads& boss)
                        {
                            boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::close, gear);
                                gear.dismiss(true);
                            };
                        }},
                    }))
                    ->colors(cB.fgc(), cB.bgc())
                    ->plugin<pro::focus>()
                    ->plugin<pro::track>()
                    ->plugin<pro::acryl>()
                    ->invoke([](auto& boss)
                    {
                        boss.keybd.active();
                        boss.SUBMIT(tier::anycast, e2::form::quit, item)
                        {
                            boss.base::template riseup<tier::release>(e2::form::quit, item);
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
                    boss.SUBMIT(tier::release, e2::form::proceed::attach, fullscreen_item)
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
                    boss.SUBMIT(tier::anycast, app::tile::events::ui::any, gear)
                    {
                        if (auto deed = boss.bell::template protos<tier::anycast>()) //todo "template" keyword is required by FreeBSD clang 11.0.1
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
                                        item_ptr->riseup<tier::release>(events::backup, empty_slot_list);
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
                                            app_slot->template riseup<tier::release>(events::enlist, app_slot);
                                        }
                                        std::swap(emp_slot, emp_next);
                                        std::swap(app_slot, app_next);
                                    }
                                    auto& s = empty_slot_list.front();
                                    if (emp_slot) s->attach(emp_slot);
                                    if (app_slot)
                                    {
                                        s->attach(app_slot);
                                        app_slot->template riseup<tier::release>(events::enlist, app_slot);
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
                    boss.SUBMIT(tier::release, hids::events::upevent::kboffer, gear)
                    {
                        // Set focus to all panes.
                        boss.SIGNAL(tier::anycast, app::tile::events::ui::select, gear);
                        gear.dismiss(true);
                    };
                });
            return object;
        };
    }

    app::shared::initialize builder{ menuitem_t::type_Group, build_inst };
}

#endif // NETXS_APP_TILE_HPP