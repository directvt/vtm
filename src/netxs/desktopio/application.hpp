// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

#if defined(__clang__) || defined(__APPLE__)
    #pragma clang diagnostic ignored "-Wunused-variable"
    #pragma clang diagnostic ignored "-Wunused-function"
#endif

#include "console.hpp"
#include "system.hpp"
#include "terminal.hpp"
#include "gui.hpp"

namespace netxs::app
{
    namespace fs = std::filesystem;
    using namespace std::placeholders;
    using namespace netxs::ui;
}

namespace netxs::app::shared
{
    static const auto version = "v0.9.99.71";
    static const auto repository = "https://github.com/directvt/vtm";
    static const auto usr_config = "~/.config/vtm/settings.xml"s;
    static const auto sys_config = "/etc/vtm/settings.xml"s;

    const auto closing_on_quit = [](auto& boss)
    {
        boss.LISTEN(tier::anycast, e2::form::proceed::quit::any, fast)
        {
            boss.base::riseup(tier::release, e2::form::proceed::quit::one, fast);
        };
    };
    const auto closing_by_gesture = [](auto& boss)
    {
        boss.on(tier::mouserelease, input::key::LeftRightClick, [&](hids& gear)
        {
            auto backup = boss.This();
            boss.base::riseup(tier::release, e2::form::proceed::quit::one, true);
            gear.dismiss();
        });
        boss.on(tier::mouserelease, input::key::MiddleClick, [&](hids& gear)
        {
            auto backup = boss.This();
            boss.base::riseup(tier::release, e2::form::proceed::quit::one, true);
            gear.dismiss();
        });
    };
    const auto scroll_bars = [](auto master)
    {
        auto sb = ui::fork::ctor();
        auto bt = sb->attach(slot::_1, ui::fork::ctor(axis::Y));
        auto hz = bt->attach(slot::_2, ui::grip<axis::X>::ctor(master));
        auto vt = sb->attach(slot::_2, ui::grip<axis::Y>::ctor(master));
        return sb;
    };
    const auto underlined_hz_scrollbar = [](auto scrlrail)
    {
        auto grip = ui::grip<axis::X>::ctor(scrlrail, ui::drawfx::underline)
            ->alignment({ snap::both, snap::tail })
            ->invoke([&](auto& boss)
            {
                boss.base::hidden = true;
                scrlrail->LISTEN(tier::release, e2::form::state::mouse, hovered, -, (grip_shadow = ptr::shadow(boss.This())))
                {
                    if (auto grip_ptr = grip_shadow.lock())
                    {
                        grip_ptr->base::hidden = !hovered;
                        grip_ptr->base::reflow();
                    }
                };
            });
        return grip;
    };
    const auto scroll_bars_term = [](auto master)
    {
        auto scroll_bars = ui::fork::ctor();
        auto scroll_head = scroll_bars->attach(slot::_1, ui::fork::ctor(axis::Y));
        auto hz = scroll_head->attach(slot::_1, ui::grip<axis::X>::ctor(master));
        auto vt = scroll_bars->attach(slot::_2, ui::grip<axis::Y>::ctor(master));
        return scroll_bars;
    };
    const auto set_title = [](base& boss, input::hids& gear, bias alignment = bias::left)
    {
        auto old_title = boss.base::riseup(tier::request, e2::form::prop::ui::header);
        gear.owner.base::signal(tier::request, input::events::clipboard, gear);
        auto& data = gear.board::cargo;
        if (data.utf8.empty())
        {
            log("%%Clipboard is empty or contains non-text data", prompt::desk);
        }
        else
        {
            if (utf::is_plain(data.utf8) || alignment != bias::left) // Reset aligning to the center if text is plain.
            {
                auto align = ansi::jet(alignment);
                boss.base::riseup(tier::preview, e2::form::prop::ui::header, align);
            }
            // Copy clipboard data to title.
            auto title = data.utf8;
            boss.base::riseup(tier::preview, e2::form::prop::ui::header, title);
            if (old_title.size()) // Copy old title to clipboard.
            {
                gear.set_clipboard(dot_00, old_title, mime::ansitext);
            }
        }
    };
    const auto base_kb_navigation = [](xmls& config, ui::sptr scroll_ptr, base& boss)
    {
        auto& scroll_inst = *scroll_ptr;
        boss.base::plugin<pro::keybd>();
        auto& luafx = boss.bell::indexer.luafx;
        auto script_list = config.list("/config/events/defapp/script");
        auto bindings = input::bindings::load(config, script_list);
        input::bindings::keybind(boss, bindings);
        boss.base::add_methods(basename::defapp,
        {
            { "ScrollViewportByPage",   [&]
                                        {
                                            auto step = twod{ luafx.get_args_or(1, 0),
                                                              luafx.get_args_or(2, 0) };
                                            scroll_inst.base::riseup(tier::preview, e2::form::upon::scroll::bypage::v, { .vector = step });
                                            luafx.get_gear().set_handled();
                                            luafx.set_return(); // No returns.
                                        }},
            { "ScrollViewportByStep",   [&]
                                        {
                                            auto step = twod{ luafx.get_args_or(1, 0),
                                                              luafx.get_args_or(2, 0) };
                                            scroll_inst.base::riseup(tier::preview, e2::form::upon::scroll::bystep::v, { .vector = step });
                                            luafx.get_gear().set_handled();
                                            luafx.set_return(); // No returns.
                                        }},
            { "ScrollViewportToTop",    [&]
                                        {
                                            scroll_inst.base::riseup(tier::preview, e2::form::upon::scroll::to_top::y);
                                            luafx.get_gear().set_handled();
                                            luafx.set_return(); // No returns.
                                        }},
            { "ScrollViewportToEnd",    [&]
                                        {
                                            scroll_inst.base::riseup(tier::preview, e2::form::upon::scroll::to_end::y);
                                            luafx.get_gear().set_handled();
                                            luafx.set_return(); // No returns.
                                        }},
            { "ShowClosingPreview",     [&]
                                        {
                                            auto& closing_preview_state = boss.base::property("defapp.closing_preview_state", faux);
                                            auto args_count = luafx.args_count();
                                            if (args_count && std::exchange(closing_preview_state, luafx.get_args_or(1, faux)) != closing_preview_state)
                                            {
                                                boss.base::signal(tier::anycast, e2::form::state::keybd::command::close, closing_preview_state);
                                            }
                                            luafx.set_return(closing_preview_state);
                                        }},
            { "Close",                  [&]
                                        {
                                            boss.base::enqueue([](auto& boss) // Keep the focus tree intact while processing events.
                                            {
                                                boss.base::riseup(tier::release, e2::form::proceed::quit::one, true);
                                            });
                                            luafx.get_gear().set_handled();
                                            luafx.set_return();
                                        }},
        });
    };
    const auto applet_kb_navigation = [](xmls& config, ui::sptr boss_ptr)
    {
        auto& boss = *boss_ptr;
        boss.base::plugin<pro::focus>();
        boss.base::plugin<pro::keybd>();
        auto& luafx = boss.bell::indexer.luafx;
        auto& bindings = boss.base::property<input::bindings::vector>("applet.bindings");
        auto script_list = config.list("/config/events/applet/script");
        bindings = input::bindings::load(config, script_list);
        input::bindings::keybind(boss, bindings);
        boss.base::add_methods(basename::applet,
        {
            //{ "FocusNext",          [&]
            //                        {
            //                            auto gui_cmd = e2::command::gui.param();
            //                            auto& gear = luafx.get_gear();
            //                            if (gear.is_real())
            //                            {
            //                                gui_cmd.gear_id = gear.id;
            //                                gear.set_handled();
            //                            }
            //                            gui_cmd.cmd_id = syscmd::focusnextwindow;
            //                            gui_cmd.args.emplace_back(luafx.get_args_or(1, si32{ 1 }));
            //                            boss.base::riseup(tier::preview, e2::command::gui, gui_cmd);
            //                            luafx.set_return();
            //                        }},
            { "Warp",               [&]
                                    {
                                        auto gui_cmd = e2::command::gui.param();
                                        auto& gear = luafx.get_gear();
                                        if (gear.is_real())
                                        {
                                            gui_cmd.gear_id = gear.id;
                                            gear.set_handled();
                                        }
                                        gui_cmd.cmd_id = syscmd::warpwindow;
                                        gui_cmd.args.emplace_back(luafx.get_args_or(1, si32{ 0 }));
                                        gui_cmd.args.emplace_back(luafx.get_args_or(2, si32{ 0 }));
                                        gui_cmd.args.emplace_back(luafx.get_args_or(3, si32{ 0 }));
                                        gui_cmd.args.emplace_back(luafx.get_args_or(4, si32{ 0 }));
                                        boss.base::riseup(tier::preview, e2::command::gui, gui_cmd);
                                        luafx.set_return();
                                    }},
            { "ZOrder",            [&]
                                    {
                                        auto args_count = luafx.args_count();
                                        auto& zorder = boss.base::property("applet.zorder", zpos::plain);
                                        auto& gear = luafx.get_gear();
                                        if (args_count)
                                        {
                                            auto gui_cmd = e2::command::gui.param();
                                            zorder = luafx.get_args_or(1, zpos::plain);
                                            if (gear.is_real())
                                            {
                                                gui_cmd.gear_id = gear.id;
                                            }
                                            gui_cmd.cmd_id = syscmd::zorder;
                                            gui_cmd.args.emplace_back(zorder);
                                            boss.base::riseup(tier::release, e2::form::prop::zorder, si32{ zorder });
                                            boss.base::riseup(tier::preview, e2::command::gui, gui_cmd);
                                        }
                                        gear.set_handled();
                                        luafx.set_return(zorder);
                                    }},
            { "Close",              [&]
                                    {
                                        auto gui_cmd = e2::command::gui.param();
                                        auto& gear = luafx.get_gear();
                                        if (gear.is_real())
                                        {
                                            gui_cmd.gear_id = gear.id;
                                            gear.set_handled();
                                        }
                                        gui_cmd.cmd_id = syscmd::close;
                                        boss.base::riseup(tier::preview, e2::command::gui, gui_cmd);
                                        luafx.set_return();
                                    }},
            { "Minimize",           [&]
                                    {
                                        auto gui_cmd = e2::command::gui.param();
                                        auto& gear = luafx.get_gear();
                                        if (gear.is_real())
                                        {
                                            gui_cmd.gear_id = gear.id;
                                            gear.set_handled();
                                        }
                                        gui_cmd.cmd_id = syscmd::minimize;
                                        boss.base::riseup(tier::preview, e2::command::gui, gui_cmd);
                                        luafx.set_return();
                                    }},
            { "Maximize",           [&]
                                    {
                                        auto gui_cmd = e2::command::gui.param();
                                        auto& gear = luafx.get_gear();
                                        if (gear.is_real())
                                        {
                                            gui_cmd.gear_id = gear.id;
                                            gear.set_handled();
                                        }
                                        gui_cmd.cmd_id = syscmd::maximize;
                                        boss.base::riseup(tier::preview, e2::command::gui, gui_cmd);
                                        luafx.set_return();
                                    }},
            { "Fullscreen",         [&]
                                    {
                                        auto gui_cmd = e2::command::gui.param();
                                        auto& gear = luafx.get_gear();
                                        if (gear.is_real())
                                        {
                                            gui_cmd.gear_id = gear.id;
                                            gear.set_handled();
                                        }
                                        gui_cmd.cmd_id = syscmd::fullscreen;
                                        boss.base::riseup(tier::preview, e2::command::gui, gui_cmd);
                                        luafx.set_return();
                                    }},
            { "Restore",            [&]
                                    {
                                        auto gui_cmd = e2::command::gui.param();
                                        auto& gear = luafx.get_gear();
                                        if (gear.is_real())
                                        {
                                            gui_cmd.gear_id = gear.id;
                                            gear.set_handled();
                                        }
                                        gui_cmd.cmd_id = syscmd::restore;
                                        boss.base::riseup(tier::preview, e2::command::gui, gui_cmd);
                                        luafx.set_return();
                                    }},
        });
    };

    using builder_t = std::function<ui::sptr(eccc, xmls&)>;

    namespace win
    {
        namespace type
        {
            static const auto undefined  = "undefined"s;
            static const auto normal     = "normal"s;
            static const auto minimized  = "minimized"s;
            static const auto maximized  = "maximized"s;
            static const auto fullscreen = "fullscreen"s;
        }

        static auto options = utf::unordered_map<text, si32>
           {{ type::undefined,  winstate::normal     },
            { type::normal,     winstate::normal     },
            { type::minimized,  winstate::minimized  },
            { type::maximized,  winstate::maximized  },
            { type::fullscreen, winstate::fullscreen }};
    }

    namespace menu
    {
        struct item
        {
            bool alive{};
            text label{};
            text tooltip{};
            cell hover{};
            cell focus{};
            input::bindings::vector bindings;
        };

        using action_map_t = utf::unordered_map<text, std::function<void(ui::item&, menu::item&)>>;
        using link = std::tuple<item, std::function<void(ui::item&, item&)>>;
        using list = std::list<link>;

        static auto mini(bool autohide, bool slimsize, si32 custom, list menu_items) // Menu bar (shrinkable on right-click).
        {
            //auto highlight_color = skin::color(tone::highlight);
            auto danger_color    = skin::color(tone::danger);
            //auto action_color    = skin::color(tone::action);
            //auto warning_color   = skin::color(tone::warning);
            //auto c6 = action_color;
            //auto c3 = highlight_color;
            //auto c2 = warning_color;
            auto c1 = danger_color;
            auto macstyle = skin::globals().macstyle;
            auto menuveer = ui::veer::ctor();
            auto menufork = ui::fork::ctor()
                //todo
                //->alignment({ snap::both, snap::both }, { macstyle ? snap::head : snap::tail, snap::both })
                ->active();
            auto makeitem = [&](auto& config)
            {
                auto& props = std::get<0>(config);
                auto& setup = std::get<1>(config);
                auto& alive = props.alive;
                auto& label = props.label;
                auto& tooltip = props.tooltip;
                auto& hover = props.hover;
                auto button = ui::item::ctor(label)->drawdots();
                button->active(); // Always active for tooltips.
                if (alive)
                {
                    if (hover.clr()) button->shader(cell::shaders::mimic(hover), e2::form::state::hover);
                    else             button->shader(cell::shaders::xlight,       e2::form::state::hover);
                }
                button->template plugin<pro::notes>(tooltip)
                    ->setpad({ 0, 0, !slimsize, !slimsize })
                    ->invoke([&](auto& boss) // Store shared ptr to the menu item config.
                    {
                        auto& item_props = boss.base::field(std::move(props));
                        setup(boss, item_props);
                    });
                return button;
            };
            auto ctrlslot = macstyle ? slot::_1 : slot::_2;
            auto menuslot = macstyle ? slot::_2 : slot::_1;
            auto ctrllist = menufork->attach(ctrlslot, ui::list::ctor(axis::X));
            if (custom) // Apply a custom menu controls.
            {
                while (custom--)
                {
                    auto button = makeitem(menu_items.back());
                    ctrllist->attach<sort::reverse>(button);
                    menu_items.pop_back();
                }
            }
            else // Add standard menu controls.
            {
                auto control = std::vector<link>
                {
                    { menu::item{ .alive = true, .label = "  —  ", .tooltip = " Minimize " },//, .hover = c2 }, //todo too funky
                    [](auto& boss, auto& /*item*/)
                    {
                        boss.on(tier::mouserelease, input::key::LeftClick, [&](hids& gear)
                        {
                            boss.base::riseup(tier::preview, e2::form::size::minimize, gear);
                            gear.dismiss();
                        });
                    }},
                    { menu::item{ .alive = true, .label = "  □  ", .tooltip = " Maximize " },//, .hover = c6 },
                    [](auto& boss, auto& /*item*/)
                    {
                        boss.on(tier::mouserelease, input::key::LeftClick, [&](hids& gear)
                        {
                            boss.base::riseup(tier::preview, e2::form::size::enlarge::maximize, gear);
                            gear.dismiss();
                        });
                    }},
                    { menu::item{ .alive = true, .label = "  ×  ", .tooltip = " Close ", .hover = c1 },
                    [c1](auto& boss, auto& /*item*/)
                    {
                        boss.template shader<tier::anycast>(cell::shaders::color(c1), e2::form::state::keybd::command::close);
                        boss.on(tier::mouserelease, input::key::LeftClick, [&](hids& gear)
                        {
                            auto backup = boss.This(); //todo revise backup
                            boss.base::signal(tier::anycast, e2::form::proceed::quit::one, faux); // fast=faux: Show closing process.
                            gear.dismiss();
                        });
                    }},
                };
                if (macstyle)
                {
                    ctrllist->attach(makeitem(control[2]));
                    ctrllist->attach(makeitem(control[0]));
                    ctrllist->attach(makeitem(control[1]));
                }
                else
                {
                    ctrllist->attach(makeitem(control[0]));
                    ctrllist->attach(makeitem(control[1]));
                    ctrllist->attach(makeitem(control[2]));
                }
            }
            auto scrlarea = menufork->attach(menuslot, ui::cake::ctor());
            auto scrlrail = scrlarea->attach(ui::rail::ctor(axes::X_only, axes::all));
            auto scrllist = scrlrail->attach(ui::list::ctor(axis::X));

            auto scrlcake = ui::cake::ctor();
            auto scrlhint = scrlcake->attach(underlined_hz_scrollbar(scrlrail));
            auto scrlgrip = scrlarea->attach(scrlcake);

            for (auto& body : menu_items)
            {
                scrllist->attach(makeitem(body));
            }

            auto menucake = menuveer->attach(ui::cake::ctor()->branch(menufork))
                ->invoke([&](auto& boss)
                {
                    auto& slim_status = boss.base::field(slimsize);
                    boss.LISTEN(tier::anycast, e2::form::upon::resized, new_area)
                    {
                        if (!slim_status)
                        {
                            auto height = boss.base::min_sz.y;
                            if (new_area.size.y < 3)
                            {
                                if (height != new_area.size.y)
                                {
                                    boss.base::limits({ -1, new_area.size.y }, { -1, new_area.size.y });
                                }
                            }
                            else if (height != 3)
                            {
                                boss.base::limits({ -1, 3 }, { -1, 3 });
                            }
                        }
                    };
                    boss.LISTEN(tier::anycast, e2::form::prop::ui::slimmenu, slim)
                    {
                        slim_status = slim;
                        auto height = slim ? 1 : 3;
                        boss.base::limits({ -1, height }, { -1, height });
                        boss.reflow();
                    };
                });
            auto menutent = menuveer->attach(ui::mock::ctor()->limits({ -1,1 }, { -1,1 }));
            if (autohide == faux) menuveer->roll();
            menuveer->limits({ -1, slimsize ? 1 : 3 }, { -1, slimsize ? 1 : 3 })
                ->invoke([&](auto& boss)
                {
                    if (autohide)
                    {
                        auto menutent_shadow = ptr::shadow(menutent);
                        auto menucake_shadow = ptr::shadow(menucake);
                        boss.LISTEN(tier::release, e2::form::state::mouse, hovered, -, (menucake_shadow, menutent_shadow))
                        {
                            if (auto menucake = menucake_shadow.lock())
                            {
                                auto menu_visible = boss.back() != menucake;
                                if (hovered == menu_visible)
                                {
                                    boss.roll();
                                    boss.reflow();
                                    if (auto menutent = menutent_shadow.lock())
                                    {
                                        menutent->base::signal(tier::release, e2::form::state::visible, menu_visible);
                                    }
                                }
                            }
                        };
                    }
                });

            return std::tuple{ menuveer, menutent, menucake };
        };
        const auto create = [](xmls& config, list menu_items)
        {
            auto autohide = config.take("menu/autohide", faux);
            auto slimsize = config.take("menu/slim"    , true);
            return mini(autohide, slimsize, 0, menu_items);
        };
        const auto load = [](xmls& config)
        {
            auto list = menu::list{};
            auto menudata = config.list("menu/item");
            for (auto data_ptr : menudata)
            {
                auto item = menu::item{};
                auto& data = *data_ptr;
                auto script_list = data.list("script");
                item.alive = script_list.size();
                item.bindings = input::bindings::load(config, script_list);
                auto classname_list = config.expand_list(data_ptr, "id");
                auto label_list = data.list("label");
                item.label = label_list.size() ? config.expand(label_list.front()) : "empty"s;
                item.tooltip = data.take("tooltip", ""s);
                auto setup = [classname_list = std::move(classname_list)](ui::item& boss, menu::item& item)
                {
                    auto& luafx = boss.bell::indexer.luafx;
                    for (auto& classname : classname_list)
                    {
                        boss.bell::indexer.add_base_class(classname, boss);
                    }
                    input::bindings::keybind(boss, item.bindings);
                    boss.base::add_methods(basename::item,
                    {
                        { "Label",      [&]
                                        {
                                            auto args_count = luafx.args_count();
                                            if (args_count) // Set label.
                                            {
                                                auto new_label = luafx.get_args_or(1, "label"s);
                                                boss.set(new_label);
                                                luafx.set_return();
                                            }
                                            else // Get label.
                                            {
                                                auto current_label = boss.get();
                                                luafx.set_return(current_label);
                                            }
                                        }},
                        { "Tooltip",    [&]
                                        {
                                            
                                            auto args_count = luafx.args_count();
                                            if (args_count) // Set tooltip.
                                            {
                                                auto new_tooltip = luafx.get_args_or(1, ""s);
                                                boss.base::signal(tier::preview, e2::form::prop::ui::tooltip, new_tooltip);
                                                luafx.set_return();
                                            }
                                            else // Get tooltip.
                                            {
                                                auto current_tooltip = boss.base::signal(tier::request, e2::form::prop::ui::tooltip);
                                                luafx.set_return(current_tooltip);
                                            }
                                        }},
                        { "Deface",     [&]
                                        {
                                            boss.base::deface();
                                            luafx.set_return();
                                        }},
                    });
                };
                list.push_back({ item, setup });
            }
            return menu::create(config, list);
        };
        const auto demo = [](xmls& config)
        {
            //auto highlight_color = skin::color(tone::highlight);
            //auto c3 = highlight_color;
            auto items = list
            {
                { item{ .alive = true, .label = ansi::add("  ").und(true).add("F").nil().add("ile  "), .tooltip = " File menu item " }, [&](auto& /*boss*/, auto& /*item*/){ }},
                { item{ .alive = true, .label = ansi::add("  ").und(true).add("E").nil().add("dit  "), .tooltip = " Edit menu item " }, [&](auto& /*boss*/, auto& /*item*/){ }},
                { item{ .alive = true, .label = ansi::add("  ").und(true).add("V").nil().add("iew  "), .tooltip = " View menu item " }, [&](auto& /*boss*/, auto& /*item*/){ }},
                { item{ .alive = true, .label = ansi::add("  ").und(true).add("D").nil().add("ata  "), .tooltip = " Data menu item " }, [&](auto& /*boss*/, auto& /*item*/){ }},
                { item{ .alive = true, .label = ansi::add("  ").und(true).add("H").nil().add("elp  "), .tooltip = " Help menu item " }, [&](auto& /*boss*/, auto& /*item*/){ }},
            };
            auto [menu, cover, menu_data] = create(config, items);
            return menu;
        };
    }
    namespace
    {
        auto& creator()
        {
            static auto creator = std::map<text, builder_t>{};
            return creator;
        }
    }
    auto& builder(text app_typename)
    {
        static builder_t empty =
        [&](eccc, xmls&) -> ui::sptr
        {
            auto window = ui::cake::ctor()
                ->plugin<pro::focus>()
                //->plugin<pro::track>()
                ->plugin<pro::acryl>()
                ->invoke([&](auto& boss)
                {
                    closing_on_quit(boss);
                    closing_by_gesture(boss);
                    boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent)
                    {
                        auto title = "error"s;
                        boss.base::riseup(tier::preview, e2::form::prop::ui::header, title);
                    };
                });
            auto msg = ui::post::ctor()
                ->colors(whitelt, argb{ 0x7F404040 })
                ->upload(ansi::fgc(yellowlt).mgl(4).mgr(4).wrp(wrap::off) +
                    "\n"
                    "\nUnsupported application type"
                    "\n" + ansi::nil().wrp(wrap::on) +
                    "\nOnly the following application types are supported:"
                    "\n" + ansi::nil().wrp(wrap::off).fgc(whitedk) +
                    "\n   type = vtty"
                    "\n   type = term"
                    "\n   type = dtvt"
                    "\n   type = dtty"
                    "\n   type = tile"
                    "\n   type = site"
                    "\n   type = info"
                    "\n"
                    "\n" + ansi::nil().wrp(wrap::on).fgc(whitelt)
                    .add(prompt::apps, "See logs for details."));
            auto placeholder = ui::cake::ctor()
                ->colors(whitelt, argb{ 0x7F404040 })
                ->attach(msg->alignment({ snap::head, snap::head }));
            window->attach(ui::rail::ctor())
                ->attach(placeholder)
                ->active();
            return window;
        };
        auto& map = creator();
        const auto it = map.find(app_typename);
        if (it == map.end())
        {
            log("%%Unknown app type - '%app_typename%'", prompt::apps, app_typename);
            return empty;
        }
        else return it->second;
    };
    namespace load
    {
        auto log_load(view src_path)
        {
            log("%%Loading settings from %path%...", prompt::apps, src_path);
        }
        auto load_from_file(xml::document& config, qiew file_path)
        {
            auto [config_path, config_path_str] = os::path::expand(file_path);
            if (!config_path.empty())
            {
                log_load(config_path_str);
                auto ec = std::error_code{};
                auto config_file = fs::directory_entry(config_path, ec);
                if (!ec && (config_file.is_regular_file(ec) || config_file.is_symlink(ec)))
                {
                    auto file = std::ifstream(config_file.path(), std::ios::binary | std::ios::in);
                    if (!file.seekg(0, std::ios::end).fail())
                    {
                        auto size = file.tellg();
                        auto buff = text((size_t)size, '\0');
                        file.seekg(0, std::ios::beg);
                        file.read(buff.data(), size);
                        config.load(buff, config_path_str);
                        log("%%Loaded %count% bytes", prompt::pads, size);
                        return true;
                    }
                }
                log(prompt::pads, "Not found");
            }
            return faux;
        }
        auto attach_file_list(xml::document& defcfg, xml::document& cfg)
        {
            if (cfg)
            {
                auto file_list = cfg.take<true>("/file");
                if (file_list.size())
                {
                    log("%%Update settings source files from %src%", prompt::apps, cfg.page.file);
                    for (auto& file : file_list) if (file && !file->base) log("%%%file%", prompt::pads, file->take_value());
                    defcfg.attach("/", file_list);
                }
            }
        }
        auto overlay_config(xml::document& defcfg, xml::document& cfg)
        {
            if (cfg)
            {
                auto config_data = cfg.take("/config/");
                if (config_data.size())
                {
                    log(prompt::pads, "Merging settings from ", cfg.page.file);
                    defcfg.overlay(config_data.front(), "");
                }
            }
        }
        auto settings(qiew cliopt, bool print = faux)
        {
            static auto defaults = utf::replace_all(
                #include "../../vtm.xml"
                , "\n\n", "\n");
            auto envopt = os::env::get("VTM_CONFIG");
            auto defcfg = xml::document{ defaults };
            auto envcfg = xml::document{};
            auto dvtcfg = xml::document{};
            auto clicfg = xml::document{};

            auto show_cfg = [&](auto& cfg){ if (print && cfg) log("%source%:\n%config%", cfg.page.file, cfg.page.show()); };

            if (envopt.size()) // Load settings from the environment variable (plain xml data or file path).
            {
                log_load("$VTM_CONFIG=" + envopt);
                if (envopt.starts_with("<")) // The case with a plain xml data.
                {
                    envcfg.load(envopt, "settings from the environment variable");
                }
                else
                {
                    load_from_file(envcfg, envopt); // The case with a file path.
                }
                show_cfg(envcfg);
            }

            if (os::dtvt::config.size()) // Load settings from the received directvt packet.
            {
                log_load("the received DirectVT packet");
                dvtcfg.load(os::dtvt::config, "dtvt");
                show_cfg(dvtcfg);
            }

            if (cliopt.size()) // Load settings from the specified '-c' cli option (memory mapped source, plain xml data or file path).
            {
                log_load(utf::concat("the specified '-c ", cliopt, "'"));
                if (cliopt.starts_with("<")) // The case with a plain xml data.
                {
                    clicfg.load(cliopt, "settings from the specified '-c' cli option");
                }
                else if (cliopt.starts_with(":")) // Receive configuration via memory mapping.
                {
                    cliopt.remove_prefix(1);
                    auto utf8 = os::process::memory::get(cliopt);
                    clicfg.load(utf8, cliopt);
                }
                else load_from_file(clicfg, cliopt); // The case with a file path.
                show_cfg(clicfg);
            }

            attach_file_list(defcfg, envcfg);
            attach_file_list(defcfg, dvtcfg);
            attach_file_list(defcfg, clicfg);

            auto config_sources = defcfg.take("/file");
            for (auto& file_rec : config_sources) if (file_rec && !file_rec->base) // Overlay configs from the specified sources if it is.
            {
                auto src_file = file_rec->take_value();
                auto src_conf = xml::document{};
                load_from_file(src_conf, src_file);
                show_cfg(src_conf);
                overlay_config(defcfg, src_conf);
            }

            overlay_config(defcfg, envcfg);
            overlay_config(defcfg, dvtcfg);
            overlay_config(defcfg, clicfg);

            auto resultant = xmls{ defcfg };
            return resultant;
        }
    }

    struct initialize
    {
        initialize(text app_typename, builder_t builder)
        {
            auto& map = creator();
            map[app_typename] = builder;
        }
    };
    struct gui_config_t
    {
        si32 winstate{};
        bool aliasing{};
        span blinking{};
        twod wincoord{};
        twod gridsize{};
        si32 cellsize{};
        std::list<text> fontlist;
    };

    auto get_gui_config(xmls& config)
    {
        auto gui_config = gui_config_t{ .winstate = config.take("/config/gui/winstate", winstate::normal, app::shared::win::options),
                                        .aliasing = config.take("/config/gui/antialiasing", faux),
                                        .blinking = config.take("/config/gui/blinkrate", span{ 400ms }),
                                        .wincoord = config.take("/config/gui/wincoor", dot_mx),
                                        .gridsize = config.take("/config/gui/gridsize", dot_mx),
                                        .cellsize = std::clamp(config.take("/config/gui/cellheight", si32{ 20 }), 0, 256) };
        if (gui_config.cellsize == 0) gui_config.cellsize = 20;
        if (gui_config.gridsize.x == 0 || gui_config.gridsize.y == 0) gui_config.gridsize = dot_mx;
        auto recs = config.list("/config/gui/fonts/font");
        for (auto& f : recs)
        {
            //todo implement 'fonts/font/file' - font file path/url
            gui_config.fontlist.push_back(config.expand(f));
        }
        return gui_config;
    }
    auto get_tui_config(xmls& config, ui::skin& g)
    {
        using namespace std::chrono;
        g.window_clr     = config.take("/config/colors/window"     , cell{ whitespace });
        g.winfocus       = config.take("/config/colors/focus"      , cell{ whitespace });
        g.brighter       = config.take("/config/colors/brighter"   , cell{ whitespace });
        g.shadower       = config.take("/config/colors/shadower"   , cell{ whitespace });
        g.warning        = config.take("/config/colors/warning"    , cell{ whitespace });
        g.danger         = config.take("/config/colors/danger"     , cell{ whitespace });
        g.action         = config.take("/config/colors/action"     , cell{ whitespace });
        g.selected       = config.take("/config/desktop/taskbar/colors/selected"  , cell{ whitespace });
        g.active         = config.take("/config/desktop/taskbar/colors/active"    , cell{ whitespace });
        g.focused        = config.take("/config/desktop/taskbar/colors/focused"   , cell{ whitespace });
        g.inactive       = config.take("/config/desktop/taskbar/colors/inactive"  , cell{ whitespace });
        g.spd            = config.take("/config/timings/kinetic/spd"      , 10  );
        g.pls            = config.take("/config/timings/kinetic/pls"      , 167 );
        g.spd_accel      = config.take("/config/timings/kinetic/spd_accel", 1   );
        g.spd_max        = config.take("/config/timings/kinetic/spd_max"  , 100 );
        g.ccl            = config.take("/config/timings/kinetic/ccl"      , 120 );
        g.ccl_accel      = config.take("/config/timings/kinetic/ccl_accel", 30  );
        g.ccl_max        = config.take("/config/timings/kinetic/ccl_max"  , 1   );
        g.switching      = config.take("/config/timings/switching"        , span{ 200ms });
        g.deceleration   = config.take("/config/timings/deceleration"     , span{ 2s    });
        g.blink_period   = config.take("/config/cursor/blink"             , span{ 400ms });
        g.menu_timeout   = config.take("/config/desktop/taskbar/timeout"  , span{ 250ms });
        g.leave_timeout  = config.take("/config/timings/leave_timeout"    , span{ 1s    });
        g.repeat_delay   = config.take("/config/timings/repeat_delay"     , span{ 500ms });
        g.repeat_rate    = config.take("/config/timings/repeat_rate"      , span{ 30ms  });
        g.maxfps         = config.take("/config/timings/fps"              , 60);
        g.max_value      = config.take("/config/desktop/windowmax"        , twod{ 3000, 2000  });
        g.macstyle       = config.take("/config/desktop/macstyle"         , faux);
        g.menuwide       = config.take("/config/desktop/taskbar/wide"     , faux);
        g.shadow_enabled = config.take("/config/desktop/shadow/enabled", true);
        g.shadow_bias    = config.take("/config/desktop/shadow/bias"   , 0.37f);
        g.shadow_blur    = config.take("/config/desktop/shadow/blur"   , 3);
        g.shadow_opacity = config.take("/config/desktop/shadow/opacity", 105.5f);
        g.shadow_offset  = config.take("/config/desktop/shadow/offset" , dot_21);
        if (g.maxfps <= 0) g.maxfps = 60;
    }
    void splice(xipc client, gui_config_t& gc)
    {
        if (os::dtvt::active || !(os::dtvt::vtmode & ui::console::gui))
        {
            os::tty::splice(client);
        }
        else
        {
            os::dtvt::client = client;
            auto connect = [&]
            {
                auto gui_event_domain = netxs::events::auth{};
                auto window = gui_event_domain.create<gui::window>(gui_event_domain, gc.fontlist, gc.cellsize, gc.aliasing, gc.blinking, dot_21);
                window->connect(gc.winstate, gc.wincoord, gc.gridsize);
            };
            if (os::stdout_fd != os::invalid_fd)
            {
                auto runcmd = directvt::binary::command{};
                auto readln = os::tty::readline([&](auto line){ runcmd.send(client, line); }, [&]{ if (client) client->shut(); });
                connect();
                readln.stop();
            }
            else
            {
                connect();
            }
        }
    }
    void start(text cmd, text aclass, xmls& config)
    {
        //todo revise
        auto [client, server] = os::ipc::xlink();
        auto& indexer = ui::tui_domain();
        auto config_lock = indexer.unique_lock(); // Sync multithreaded access to config.
        auto gui_config = app::shared::get_gui_config(config);
        app::shared::get_tui_config(config, ui::skin::globals());
        auto thread = std::thread{ [&, &client = client] //todo clang 15.0.0 still disallows capturing structured bindings (wait for clang 16.0.0)
        {
            app::shared::splice(client, gui_config);
        }};
        auto gate_ptr = ui::gate::ctor(server, os::dtvt::vtmode, config);
        auto& gate = *gate_ptr;
        gate.base::resize(os::dtvt::gridsz);
        gate.base::signal(tier::general, e2::config::fps, ui::skin::globals().maxfps);
        auto appcfg = eccc{ .cmd = cmd };
        auto applet_ptr = app::shared::builder(aclass)(appcfg, config);
        auto& applet = *applet_ptr;
        applet.base::kind(base::reflow_root);
        app::shared::applet_kb_navigation(config, applet_ptr);
        gate.attach(std::move(applet_ptr));
        config_lock.unlock();
        gate.launch();
        gate.base::dequeue();
        config_lock.lock();
        gate_ptr.reset();
        server->shut();
        client->shut();
        thread.join();
    }
}
