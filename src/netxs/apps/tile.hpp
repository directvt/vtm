// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_APP_TILE_HPP
#define NETXS_APP_TILE_HPP

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

            boss.SUBMIT_T(tier::release, e2::size::set, memo, newsz)
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
                    return ui::pads::ctor(dent{ 1, 1, 0, 0 }, dent{})
                        ->plugin<pro::fader>(app::shared::x3, app::shared::c3, 150ms)
                        ->branch(ui::item::ctor(header.empty() ? "- no title -" : header))
                        ->invoke([&](auto& boss)
                        {
                            auto boss_shadow = ptr::shadow(boss.This());
                            auto data_shadow = ptr::shadow(data_src_sptr);

                            boss.SUBMIT_T_BYVAL(tier::release, e2::form::upon::vtree::attached, boss.tracker, parent)
                            {
                                if (auto data_ptr = data_shadow.lock())
                                {
                                    auto state = decltype(e2::form::highlight::any)::type{};
                                    data_ptr->SIGNAL(tier::anycast, e2::form::highlight::any, state);
                                }
                            };
                            data_src_sptr->SUBMIT_T(tier::preview, e2::form::highlight::any, boss.tracker, state)
                            {
                                boss.color(state ? 0xFF00ff00 : app::shared::x3.fgc(), app::shared::x3.bgc());
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
                client->attach_element(e2::form::prop::header, object, label);
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
                log(" start depth=", depth);
            };
        }
        ~items()
        {
            if (client)
            {
                netxs::events::sync lock;
                auto empty = decltype(e2::form::upon::vtree::detached)::type{};
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
                    auto gear_test = decltype(e2::form::state::keybd::find)::type{ gear.id, 0 };
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
                                        gear.kb_focus_taken = faux;
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
            branch->SIGNAL(tier::anycast, e2::form::prop::menusize, 1);
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
                        boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::drag::start::left, gear)
                        {
                            if (auto branch_ptr = branch_shadow.lock())
                            if (branch_ptr->area().hittest(gear.coord))
                            if (auto master_ptr = master_shadow.lock())
                            {
                                auto& master = *master_ptr;
                                auto& branch = *branch_ptr;

                                // Reset restoring callback.
                                master.SIGNAL(tier::release, e2::form::restore, decltype(e2::form::restore)::type{});

                                // Take current title.
                                auto what = decltype(e2::form::proceed::createfrom)::type{};
                                what.menuid = menu_item_id;
                                master.SIGNAL(tier::request, e2::form::prop::header, what.header);
                                master.SIGNAL(tier::request, e2::form::prop::footer, what.footer);
                                if (what.header.empty()) what.header = menu_item_id;
                                 
                                // Take coor and detach from the tiling wm.
                                gear.coord -= branch.base::coor(); // Localize mouse coor.
                                what.square.size = branch.base::size();
                                branch.global(what.square.coor);
                                what.square.coor = -what.square.coor;
                                what.object = branch_ptr;
                                master.SIGNAL(tier::preview, e2::form::proceed::detach, branch_ptr);
                                branch.moveto(dot_00);

                                // Attach to the world.
                                auto world_ptr = decltype(e2::config::whereami)::type{};
                                SIGNAL_GLOBAL(e2::config::whereami, world_ptr);
                                world_ptr->SIGNAL(tier::release, e2::form::proceed::createfrom, what);

                                // Pass unique focus.
                                auto& object = *what.object;
                                //todo unify
                                gear.kb_focus_taken = faux;
                                gear.force_group_focus = faux;
                                gear.combine_focus = true;
                                object.SIGNAL(tier::release, hids::events::upevent::kboffer, gear);

                                // Destroy placeholder.
                                master.base::template riseup<tier::release>(e2::form::quit, master_ptr);

                                // Handover mouse input.
                                master.SIGNAL(tier::release, hids::events::notify::mouse::leave,             gear);
                                object.SIGNAL(tier::release, hids::events::notify::mouse::enter,             gear);
                                object.SIGNAL(tier::release, hids::events::mouse::button::drag::start::left, gear);
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
                                parent->SUBMIT_BYVAL(tier::preview, e2::form::prop::header, newtext)
                                {
                                    if (auto boss_ptr = shadow.lock())
                                    {
                                        boss_ptr->upload(newtext);
                                        boss_ptr->parent()->SIGNAL(tier::release, e2::form::prop::header, newtext);
                                    }
                                };
                                parent->SUBMIT_BYVAL(tier::request, e2::form::prop::header, curtext)
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
                    if (auto gate_ptr = bell::getref(gear_id))
                    {
                        gate_ptr->SIGNAL(tier::preview, e2::form::proceed::focus, item_ptr);
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
        auto empty_pane = []()
        {
            return ui::park::ctor()
                ->isroot(true, 2)
                ->colors(blacklt, app::shared::term_menu_bg)
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
                    boss.SUBMIT(tier::release, e2::form::proceed::d_n_d::abort, target)
                    {
                        auto count = boss.count();
                        if (count == 1) // Only empty slot available.
                        {
                            //todo unify
                            boss.back()->color(blacklt, app::shared::term_menu_bg);
                        }
                    };
                    boss.SUBMIT(tier::release, e2::form::proceed::d_n_d::ask, target)
                    {
                        auto count = boss.count();
                        if (count == 1) // Only empty slot available.
                        {
                            //todo unify
                            auto fg = app::shared::c3.fgc();
                            auto bg = app::shared::c3.bgc();
                            fg.alpha(0x70);
                            bg.alpha(0x70);
                            boss.back()->color(fg, bg);
                            target = boss.This();
                        }
                    };
                    boss.SUBMIT(tier::release, e2::form::proceed::d_n_d::drop, what)
                    {
                        auto count = boss.count();
                        if (count == 1) // Only empty slot available.
                        {
                            //todo unify
                            boss.back()->color(blacklt, app::shared::term_menu_bg);
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
                            log(" empty_slot swap: defective structure, count=", count);
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
                        else log(" empty_slot swap: defective structure, count=", count);
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
                                else log(" empty_slot: defective structure, count=", count);
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
                        auto& item =*boss.back();
                        if (item.base::kind() != 1)
                        {
                            //todo unify
                            gear.force_group_focus = true;
                            gear.kb_focus_taken = faux;
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
                                boss.template riseup<tier::release>(e2::form::proceed::attach, decltype(e2::form::proceed::attach)::type{}); //todo "template" is required by gcc
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
                                    gear.kb_focus_taken = faux;
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
                                if (auto gate_ptr = bell::getref(gear.id))
                                {
                                    using type = decltype(e2::depth)::type;
                                    auto depth = type{};
                                    boss.base::template riseup<tier::request>(e2::depth, depth, true);
                                    log(" depth=", depth);
                                    if (depth > INHERITANCE_LIMIT) return;

                                    auto heading = deed == app::tile::events::ui::split::vt.id;
                                    auto newnode = built_node(heading ? 'v':'h', 1, 1, heading ? 1 : 2);
                                    auto empty_1 = empty_slot(empty_slot);
                                    auto empty_2 = empty_slot(empty_slot);
                                    auto curitem = boss.pop_back(); // In order to preserve all foci.
                                    gate_ptr->SIGNAL(tier::preview, e2::form::proceed::focus,   empty_2);
                                    gate_ptr->SIGNAL(tier::preview, e2::form::proceed::unfocus, curitem);
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
                        }
                    };
                    boss.SUBMIT_BYVAL(tier::release, e2::form::quit, nested_item_ptr)
                    {
                        if (nested_item_ptr)
                        {
                            auto& item = *nested_item_ptr;
                            using type = decltype(e2::form::state::keybd::handover)::type;
                            type gear_id_list;
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
                                        using type = decltype(e2::form::proceed::swap)::type;
                                        type item_ptr = boss_ptr; // sptr must be of the same type as the event argument. Casting kills all intermediaries when return.
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
                            if (gear.meta(hids::ANYCTRL))
                            {
                                //todo ...
                            }
                            else
                            {
                                if (auto gate_ptr = bell::getref(gear.id))
                                {
                                    auto& gate = *gate_ptr;
                                    auto current_default = decltype(e2::data::changed)::type{};
                                    gate.SIGNAL(tier::request, e2::data::changed, current_default);
                                    auto config = app::shared::objs_config[current_default];

                                    auto& creator = app::shared::creator(config.group);
                                    auto host = creator(config.param);
                                    auto app = app_window(config.title, "", host, current_default);
                                    gear.remove_from_kb_focus(boss.back()); // Take focus from the empty slot.
                                    boss.attach(app);

                                    //todo unify, demo limits
                                    {
                                        insts_count++;
                                        #ifndef PROD
                                            if (insts_count > APPS_MAX_COUNT)
                                            {
                                                log("tile: inst: max count reached");
                                                auto timeout = tempus::now() + APPS_DEL_TIMEOUT;
                                                auto w_frame = ptr::shadow(host);
                                                host->SUBMIT_BYVAL(tier::general, e2::timer::any, timestamp)
                                                {
                                                    if (timestamp > timeout)
                                                    {
                                                        log("tile: inst: timebomb");
                                                        if (auto host = w_frame.lock())
                                                        {
                                                            host->riseup<tier::release>(e2::form::quit, host);
                                                            //host->base::detach();
                                                            log("tile: inst: frame detached: ", insts_count);
                                                        }
                                                    }
                                                };
                                            }
                                        #endif
                                        host->SUBMIT(tier::release, e2::dtor, id)
                                        {
                                            insts_count--;
                                            log("tile: inst: detached: ", insts_count, " id=", id);
                                        };
                                    }
                                    app->SIGNAL(tier::anycast, e2::form::upon::started, app);

                                    //todo unify
                                    gear.kb_focus_taken = faux;
                                    host->SIGNAL(tier::release, hids::events::upevent::kboffer, gear);
                                }
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
            if (tag == '\"') //todo deprecated - use a("Term"...
            {
                // add term
                auto cmdline = utf::get_quote(utf8, '\"');
                if (cmdline.empty()) return place;
                log(" node cmdline=", cmdline);
                auto menu_item_id = "Term"s;
                auto& creator = app::shared::creator(menu_item_id);
                auto host = creator(cmdline);
                auto inst = app_window("Headless TE", "", host, menu_item_id);
                place->attach(inst);
            }
            else if (tag == 'a')
            {
                // add app
                utf8.remove_prefix(1);
                utf::trim_front(utf8, " ");
                if (utf8.empty() || utf8.front() != '(') return place;
                utf8.remove_prefix(1);
                auto app_id  = utf::get_quote(utf8, '\"', ", ");
                if (app_id.empty()) return place;
                auto app_title = utf::get_quote(utf8, '\"', ", ");
                auto app_data = utf::get_quote(utf8, '\"', ") ");
                log(" app_id=", app_id, " app_title=", app_title, " app_data=", app_data);

                auto& creator = app::shared::creator(app_id);
                auto host = creator(app_data);
                auto inst = app_window(app_title, "", host, app_id);
                place->attach(inst);
            }
            else if (tag == 'h' || tag == 'v')
            {
                // add split
                utf8.remove_prefix(1);
                utf::trim_front(utf8, " ");
                si32 s1 = 1;
                si32 s2 = 1;
                si32 w = -1;
                if (auto param = utf::to_int(utf8))
                {
                    s1 = std::abs(param.value());
                    if (utf8.empty() || utf8.front() != ':') return place;
                    utf8.remove_prefix(1);
                    if (auto param = utf::to_int(utf8))
                    {
                        s2 = std::abs(param.value());
                        utf::trim_front(utf8, " ");
                        if (!utf8.empty() && utf8.front() == ':') // Grip width.
                        {
                            utf8.remove_prefix(1);
                            if (auto param = utf::to_int(utf8))
                            {
                                w = std::abs(param.value());
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
            return place;
        };
        auto build_inst = [](view data) -> sptr<base>
        {
            view envvar_data;
            text window_title;
            auto a = data.find('=');
            if (a != text::npos)
            {
                auto b = data.begin();
                auto e = data.end();
                auto t = b + a;
                //auto envvar_name = view{ b, t }; //todo apple clang doesn't get it
                auto envvar_name = view{ &(*b), (size_t)(t - b) };
                log(" envvar_name=", envvar_name);
                b = t + 1;
                if (b != e)
                {
                    //envvar_data = view{ b, e }; //todo apple clang doesn't get it
                    envvar_data = view{ &(*b), (size_t)(e - b) };
                    log(" envvar_data=", envvar_data);
                    auto menu_name = utf::get_quote(envvar_data, '\"');
                    window_title   = utf::get_quote(envvar_data, '\"', ", ");
                    log(" menu_name=", menu_name);
                    log(" window_title=", window_title);
                    log(" layout_data=", envvar_data);
                    //if (window_title.length()) window_title += '\n';
                }
            }

            auto object = ui::fork::ctor(axis::Y)
                        ->plugin<items>();

            #ifndef PROD
                if (app::shared::tile_count < TILE_MAX_COUNT)
                {
                    auto c = &app::shared::tile_count; (*c)++;
                    object->SUBMIT_BYVAL(tier::release, e2::dtor, item_id)
                    {
                        (*c)--;
                        log("main: tile manager destoyed");
                    };
                }
                else
                {
                    auto& creator = app::shared::creator("Empty");
                    object->attach(slot::_1, creator(""));
                    app::shared::app_limit(object, "Reached The Limit");
                    return object;
                }
            #endif

            object->invoke([&](auto& boss)
                {
                    auto oneoff = std::make_shared<hook>();
                    auto objs_config_ptr = &app::shared::objs_config;
                    boss.SUBMIT_T_BYVAL(tier::anycast, e2::form::upon::created, *oneoff, gear)
                    {
                        if (auto gate_ptr = bell::getref(gear.id))
                        {
                            auto& gate = *gate_ptr;
                            auto& objs_config = *objs_config_ptr;
                            auto menu_item_id = decltype(e2::data::changed)::type{};
                            gate.SIGNAL(tier::request, e2::data::changed, menu_item_id);
                            //todo unify
                            auto config = objs_config[menu_item_id];
                            if (config.group == "Tile") // Reset the currently selected application to the previous one.
                            {
                                gate.SIGNAL(tier::preview, e2::data::changed, menu_item_id); // Get previous default;
                                gate.SIGNAL(tier::release, e2::data::changed, menu_item_id); // Set current  default;
                            }
                        }
                        oneoff.reset();
                    };
                    boss.SUBMIT_BYVAL(tier::release, e2::form::upon::vtree::attached, parent)
                    {
                        auto title = ansi::add(window_title);// + utf::debase(data));
                        log(" attached title=", window_title);
                        parent->base::riseup<tier::preview>(e2::form::prop::header, title);
                    };
                });

            object->attach(slot::_1, app::shared::custom_menu(true,
                std::list{
                        //  Green                                  ?Even    Red
                        // ┌────┐  ┌────┐  ┌─┬──┐  ┌────┐  ┌─┬──┐  ┌─┬──┐  ┌────┐  // ┌─┐  ┌─┬─┐  ┌─┬─┐  ┌─┬─┐  
                        // │Exec│  ├─┐  │  │ H  │  ├ V ─┤  │Swap│  │Fair│  │Shut│  // ├─┤  └─┴─┘  └<┴>┘  └>┴<┘  
                        // └────┘  └─┴──┘  └─┴──┘  └────┘  └─┴──┘  └─┴──┘  └────┘  // └─┘                       
                        std::pair<text, std::function<void(ui::pads&)>>{"  ┐└  ",//  ─┐  ", //"  ▀█  ",
                        [](ui::pads& boss)
                        {
                            boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                gear.countdown = 1;
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::toggle, gear);
                                gear.dismiss(true);
                            };
                        }},
                        std::pair<text, std::function<void(ui::pads&)>>{ "  +  ",
                        [](ui::pads& boss)
                        {
                            boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::create, gear);
                                gear.dismiss(true);
                            };
                        }},
                        std::pair<text, std::function<void(ui::pads&)>>{ " ::: ",
                        [](ui::pads& boss)
                        {
                            boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::select, gear);
                                gear.dismiss(true);
                            };
                        }},
                        std::pair<text, std::function<void(ui::pads&)>>{ "  │  ", // "  ║  ", - VGA Linux console doesn't support unicode glyphs
                        [](ui::pads& boss)
                        {
                            boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::split::hz, gear);
                                gear.dismiss(true);
                            };
                        }},
                        std::pair<text, std::function<void(ui::pads&)>>{  " ── ", // " ══ ", - VGA Linux console doesn't support unicode glyphs
                        [](ui::pads& boss)
                        {
                            boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::split::vt, gear);
                                gear.dismiss(true);
                            };
                        }},
                        std::pair<text, std::function<void(ui::pads&)>>{ "  ┌┘  ",
                        [](ui::pads& boss)
                        {
                            boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::rotate, gear);
                                gear.dismiss(true);
                            };
                        }},
                        std::pair<text, std::function<void(ui::pads&)>>{ " <-> ",
                        [](ui::pads& boss)
                        {
                            boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::swap, gear);
                                gear.dismiss(true);
                            };
                        }},
                        std::pair<text, std::function<void(ui::pads&)>>{ " >|< ",
                        [](ui::pads& boss)
                        {
                            boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::equalize, gear);
                                gear.dismiss(true);
                            };
                        }},
                        std::pair<text, std::function<void(ui::pads&)>>{ "  ×  ",
                        [](ui::pads& boss)
                        {
                            boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.SIGNAL(tier::anycast, app::tile::events::ui::close, gear);
                                gear.dismiss(true);
                            };
                        }},
                    }))
                    ->colors(whitelt, app::shared::term_menu_bg)
                    ->plugin<pro::focus>()
                    ->plugin<pro::track>()
                    ->plugin<pro::acryl>()
                    ->invoke([](auto& boss)
                    {
                        boss.keybd.accept(true);
                    });

            object->attach(slot::_2, parse_data(parse_data, envvar_data))
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
                        else log("fullscreen_item is empty");
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
                                backups empty_slot_list;
                                auto proc = decltype(e2::form::proceed::functor)::type{[&](sptr<base> item_ptr)
                                {
                                    auto gear_test = decltype(e2::form::state::keybd::find)::type{ gear.id, 0 };
                                    item_ptr->SIGNAL(tier::request, e2::form::state::keybd::find, gear_test);
                                    if (gear_test.second)
                                    {
                                        item_ptr->riseup<tier::release>(events::backup, empty_slot_list);
                                    }
                                }};
                                boss.SIGNAL(tier::general, e2::form::proceed::functor, proc);
                                auto slots_count = empty_slot_list.size();
                                log("tile: slots count=", slots_count);
                                if (slots_count >= 2) // Swap selected panes cyclically.
                                {
                                    using slot = sptr<base>;
                                    log("tile: Swap slots cyclically");
                                    auto i = 0;
                                    slot emp_slot;
                                    slot app_slot;
                                    slot emp_next;
                                    slot app_next;
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
                });
            return object;
        };
    }

    app::shared::initialize builder{ "Tile", build_inst };
}

#endif // NETXS_APP_TILE_HPP