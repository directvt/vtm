// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

namespace netxs::events::userland
{
    namespace terminal
    {
        using state_pair_t = std::pair<bool, id_t>;

        EVENTPACK( terminal::events, ui::e2::extra::slot3 )
        {
            EVENT_XS( cmd    , si32 ),
            GROUP_XS( preview, si32 ),
            GROUP_XS( release, si32 ),
            GROUP_XS( data   , si32 ),
            GROUP_XS( search , input::hids ),

            SUBSET_XS( preview )
            {
                EVENT_XS( align    , si32 ),
                EVENT_XS( wrapln   , si32 ),
                EVENT_XS( io_log   , bool ),
                EVENT_XS( cwdsync  , bool ),
                EVENT_XS( alwaysontop, state_pair_t ),
                EVENT_XS( rawkbd   , si32 ),
                GROUP_XS( selection, si32 ),
                GROUP_XS( colors   , argb ),

                SUBSET_XS( selection )
                {
                    EVENT_XS( mode, si32 ),
                    EVENT_XS( box , si32 ),
                    EVENT_XS( shot, si32 ),
                };
                SUBSET_XS( colors )
                {
                    EVENT_XS( bg, argb ),
                    EVENT_XS( fg, argb ),
                };
            };
            SUBSET_XS( release )
            {
                EVENT_XS( align    , si32 ),
                EVENT_XS( wrapln   , si32 ),
                EVENT_XS( io_log   , bool ),
                EVENT_XS( cwdsync  , bool ),
                EVENT_XS( alwaysontop, bool ),
                EVENT_XS( rawkbd   , si32 ),
                GROUP_XS( selection, si32 ),
                GROUP_XS( colors   , argb ),

                SUBSET_XS( selection )
                {
                    EVENT_XS( mode, si32 ),
                    EVENT_XS( box , si32 ),
                    EVENT_XS( shot, si32 ),
                };
                SUBSET_XS( colors )
                {
                    EVENT_XS( bg, argb ),
                    EVENT_XS( fg, argb ),
                };
            };
            SUBSET_XS( data )
            {
                EVENT_XS( in     , view        ),
                EVENT_XS( out    , view        ),
                EVENT_XS( paste  , input::hids ),
                EVENT_XS( copy   , input::hids ),
                EVENT_XS( prnscrn, input::hids ),
            };
            SUBSET_XS( search )
            {
                EVENT_XS( forward, input::hids ),
                EVENT_XS( reverse, input::hids ),
                EVENT_XS( status , si32        ),
            };
        };
    }
}

// term: Teletype Console.
namespace netxs::app::teletype
{
    static constexpr auto id = "teletype";
    static constexpr auto name = "Teletype Console";
}
// term: Terminal Console.
namespace netxs::app::terminal
{
    static constexpr auto id = "terminal";
    static constexpr auto name = "Terminal Console";

    namespace attr
    {
        static constexpr auto cwdsync   = "/config/terminal/cwdsync";
        static constexpr auto borders   = "/config/terminal/border";
    }

    namespace events = netxs::events::userland::terminal;

    auto ui_term_events = [](ui::term& boss, eccc& appcfg)
    {
        boss.LISTEN(tier::anycast, e2::form::proceed::quit::any, fast)
        {
            boss.base::signal(tier::preview, e2::form::proceed::quit::one, fast);
        };
        boss.LISTEN(tier::preview, e2::form::proceed::quit::one, fast)
        {
            boss.close(fast);
        };
        boss.LISTEN(tier::anycast, terminal::events::cmd, cmd)
        {
            boss.exec_cmd(static_cast<ui::term::commands::ui::commands>(cmd));
        };
        boss.LISTEN(tier::anycast, terminal::events::data::in, data)
        {
            boss.data_in(data);
        };
        boss.LISTEN(tier::anycast, terminal::events::data::out, data)
        {
            boss.data_out(data);
        };
        //todo add color picker to the menu
        boss.LISTEN(tier::anycast, terminal::events::preview::colors::bg, bg)
        {
            boss.set_bg_color(bg);
        };
        boss.LISTEN(tier::anycast, terminal::events::preview::colors::fg, fg)
        {
            boss.set_fg_color(fg);
        };
        boss.LISTEN(tier::anycast, e2::form::prop::colors::any, clr)
        {
            auto deed = boss.bell::protos();
                 if (deed == e2::form::prop::colors::bg.id) boss.base::signal(tier::anycast, terminal::events::preview::colors::bg, clr);
            else if (deed == e2::form::prop::colors::fg.id) boss.base::signal(tier::anycast, terminal::events::preview::colors::fg, clr);
        };
        boss.LISTEN(tier::anycast, terminal::events::preview::selection::mode, selmod)
        {
            boss.set_selmod(selmod);
        };
        boss.LISTEN(tier::anycast, terminal::events::preview::selection::shot, selmod)
        {
            boss.set_oneshot(selmod);
        };
        boss.LISTEN(tier::anycast, terminal::events::preview::selection::box, selbox)
        {
            boss.set_selalt(selbox);
        };
        boss.LISTEN(tier::anycast, terminal::events::preview::rawkbd, rawkbd)
        {
            boss.set_rawkbd(rawkbd + 1);
        };
        boss.LISTEN(tier::anycast, terminal::events::preview::wrapln, wrapln)
        {
            boss.set_wrapln(wrapln);
        };
        boss.LISTEN(tier::anycast, terminal::events::preview::io_log, state)
        {
            boss.set_log(state);
        };
        boss.LISTEN(tier::anycast, terminal::events::preview::align, align)
        {
            boss.set_align(align);
        };
        boss.LISTEN(tier::release, e2::form::upon::started, root_ptr, -, (appcfg))
        {
            if (root_ptr) // root_ptr is empty when d_n_d.
            {
                boss.start(appcfg);
            }
        };
        boss.LISTEN(tier::anycast, e2::form::upon::started, root_ptr)
        {
            boss.base::signal(tier::release, e2::form::upon::started, root_ptr);
        };
        boss.LISTEN(tier::anycast, terminal::events::search::forward, gear)
        {
            boss.selection_search(gear, feed::fwd);
        };
        boss.LISTEN(tier::anycast, terminal::events::search::reverse, gear)
        {
            boss.selection_search(gear, feed::rev);
        };
        boss.LISTEN(tier::anycast, terminal::events::data::paste, gear)
        {
            boss.paste(gear);
        };
        boss.LISTEN(tier::anycast, terminal::events::data::copy, gear)
        {
            boss.copy(gear);
        };
        boss.LISTEN(tier::anycast, terminal::events::data::prnscrn, gear)
        {
            boss.prnscrn(gear);
        };
        boss.LISTEN(tier::anycast, e2::form::upon::scroll::any, i)
        {
            auto info = e2::form::upon::scroll::bypage::y.param();
            auto deed = boss.bell::protos();
            boss.base::raw_riseup(tier::request, e2::form::upon::scroll::any.id, info);
            info.vector = i.vector;
            boss.base::raw_riseup(tier::preview, deed, info);
        };
    };
    auto build_teletype = [](eccc appcfg, xmls& config)
    {
        auto window_clr = skin::color(tone::window_clr);
        auto window = ui::cake::ctor()
            ->plugin<pro::focus>()
            ->invoke([&](auto& boss)
            {
                app::shared::closing_on_quit(boss);
            });
        window//->plugin<pro::track>()
            //->plugin<pro::acryl>()
            ->plugin<pro::cache>();
        auto layers = window->attach(ui::cake::ctor())
                            ->shader(cell::shaders::fuse(window_clr))
                            ->limits(dot_11);
        auto scroll = layers->attach(ui::rail::ctor())
                            ->limits({ 10,1 }); // mc crashes when window is too small
        if (appcfg.cmd.empty()) appcfg.cmd = os::env::shell();//todo revise + " -i";
        auto term = scroll->attach(ui::term::ctor(config))
            ->plugin<pro::focus>(pro::focus::mode::focused)
            ->invoke([&](auto& boss)
            {
                ui_term_events(boss, appcfg);
            });
        layers->attach(app::shared::scroll_bars(scroll));
        return window;
    };
    auto build_terminal = [](eccc appcfg, xmls& config)
    {
        auto window_clr = skin::color(tone::window_clr);
        auto border = std::max(0, config.take(attr::borders, 0));
        auto borders = dent{ border, border, 0, 0 };
        auto menu_height = ptr::shared(0);
        auto gradient = [menu_height, borders, bground = core{}](face& parent_canvas, si32 /*param*/, base& /*boss*/) mutable
        {
            static constexpr auto grad_vsize = 32;
            auto full = parent_canvas.full();
            auto clip = parent_canvas.clip();
            auto region = full;
            if (region.size.x != bground.size().x)
            {
                auto spline = netxs::spline01{ -0.30f };
                auto mx = std::max(2, region.size.x);
                auto my = std::min(3, region.size.y);
                bground.size({ mx, my }, skin::color(tone::winfocus));
                auto it = bground.begin();
                for (auto y = 0.f; y < my; y++)
                {
                    auto y0 = (y + 1) / grad_vsize;
                    auto sy = spline(y0);
                    for (auto x = 0.f; x < mx; x++)
                    {
                        auto& c = it++->bgc();
                        auto mirror = x < mx / 2.f ? x : mx - x;
                        auto x0 = (mirror + 2) / (mx - 1.f);
                        auto sx = spline(x0);
                        auto xy = sy * sx;
                        c.chan.a = (byte)std::round(255.0 * (1.f - xy));
                    }
                }
            }
            auto menu_size = twod{ region.size.x, std::min(bground.size().y, *menu_height) };
            auto stat_size = twod{ region.size.x, 1 };
            // Menu background.
            auto dest = clip.trim({ region.coor, menu_size });
            parent_canvas.clip(dest);
            bground.move(region.coor);
            parent_canvas.plot(bground, cell::shaders::blend);
            // Hz scrollbar background.
            bground.step({ 0, region.size.y - 1 });
            parent_canvas.clip(clip);
            parent_canvas.plot(bground, cell::shaders::blend);
            // Left/right border background.
            auto color = bground[dot_00];
            full -= dent{ 0, 0, menu_size.y, stat_size.y };
            parent_canvas.cage(full, borders, cell::shaders::blend(color));
            // Restore clipping area.
            parent_canvas.clip(clip);
        };

        auto window = ui::cake::ctor();
        window->plugin<pro::focus>()
            //->plugin<pro::track>()
            //->plugin<pro::acryl>()
            ->plugin<pro::cache>()
            ->shader(gradient, e2::form::state::focus::count);

        auto object = window->attach(ui::fork::ctor(axis::Y));
        auto term_stat_area = object->attach(slot::_2, ui::fork::ctor(axis::Y))
            ->setpad(borders)
            ->invoke([&](auto& boss)
            {
                if (borders)
                boss.LISTEN(tier::release, e2::render::background::any, parent_canvas, -, (borders, window_clr)) // Shade left/right borders.
                {
                    auto full = parent_canvas.full();
                    parent_canvas.cage(full, borders, [&](cell& c){ c.fuse(window_clr); });
                };
            });
        auto layers = term_stat_area->attach(slot::_1, ui::cake::ctor())
                                    ->limits(dot_11);
        auto scroll = layers->attach(ui::rail::ctor()->smooth(faux));
        auto min_size = twod{ 12,1 }; // mc crashes when window is too small
        auto max_size = -dot_11;
        scroll->limits(min_size, max_size)
            ->invoke([](auto& boss)
            {
                boss.LISTEN(tier::preview, e2::form::prop::window::size, new_size)
                {
                    // Axis x/y (see XTWINOPS):
                    //   -1 -- preserve
                    //    0 -- maximize (toggle)
                    if (new_size == dot_00) // Toggle fullscreen terminal (only if it is focused by someone).
                    {
                        auto gear_id_list = boss.base::riseup(tier::request, e2::form::state::keybd::enlist);
                        for (auto gear_id : gear_id_list)
                        {
                            if (auto gear_ptr = boss.bell::template getref<hids>(gear_id)) //todo Apple clang requires template
                            {
                                auto& gear = *gear_ptr;
                                boss.base::riseup(tier::preview, e2::form::size::enlarge::fullscreen, gear);
                                break;
                            }
                        }
                    }
                    else if (boss.base::size() != new_size)
                    {
                        auto panel = boss.base::size();
                        new_size = new_size.less(dot_11, panel, std::max(dot_11, new_size));
                        auto warp = rect{ dot_00, new_size } - rect{ dot_00, panel };
                        boss.base::locked = faux; // Unlock resizing.
                        boss.base::resize(new_size);
                        boss.base::locked = true; // Lock resizing until reflow is complete.
                        boss.base::riseup(tier::preview, e2::form::layout::swarp, warp);
                    }
                };
                boss.LISTEN(tier::release, e2::area, new_area)
                {
                    boss.base::locked = faux; // Unlock resizing.
                };
            });

        if (appcfg.cmd.empty()) appcfg.cmd = os::env::shell();//todo revise + " -i";
        auto term = scroll->attach(ui::term::ctor(config))
            ->plugin<pro::focus>(pro::focus::mode::focused)
            ->invoke([&](auto& boss)
            {
                //todo scripting: it is a temporary solution (until scripting is implemented)
                auto& zorder = window->base::property("applet.zorder", zpos::plain);
                boss.LISTEN(tier::anycast, terminal::events::preview::alwaysontop, state_pair)
                {
                    auto [state, gear_id] = state_pair;
                    auto new_zorder = state ? zpos::topmost : zpos::plain;
                    if (zorder != new_zorder)
                    {
                        auto gui_cmd = e2::command::gui.param();
                        gui_cmd.gear_id = gear_id;
                        gui_cmd.cmd_id = syscmd::zorder;
                        gui_cmd.args.emplace_back(new_zorder);
                        boss.base::riseup(tier::preview, e2::command::gui, gui_cmd);
                    }
                };
                auto& window_inst = *window;
                window_inst.LISTEN(tier::preview, e2::command::gui, gui_cmd) // Sync alwaysontop state with UI.
                {
                    if (gui_cmd.cmd_id == syscmd::zorder && gui_cmd.args.size())
                    {
                        auto new_zorder = any_get_or(gui_cmd.args[0], zpos::plain);
                        zorder = new_zorder;
                        auto state = new_zorder == zpos::topmost;
                        boss.base::signal(tier::anycast, terminal::events::release::alwaysontop, state);
                    }
                    window_inst.bell::passover();
                };

                auto& cwd_commands = boss.base::property("terminal.cwd_commands", config.take(attr::cwdsync, ""s));
                auto& cwd_sync = boss.base::property("terminal.cwd_sync", faux);
                auto& cwd_path = boss.base::property("terminal.cwd_path", os::fs::path{});
                boss.LISTEN(tier::preview, ui::tty::events::toggle::cwdsync, state)
                {
                    boss.base::signal(tier::anycast, terminal::events::preview::cwdsync, state);
                };
                boss.LISTEN(tier::anycast, terminal::events::preview::cwdsync, state)
                {
                    if (cwd_sync != state)
                    {
                        cwd_sync = state;
                        boss.base::signal(tier::anycast, terminal::events::release::cwdsync, state);
                        if (cwd_sync)
                        {
                            auto cmd = cwd_commands;
                            utf::replace_all(cmd, "$P", ".");
                            boss.data_out(cmd); // Trigger command prompt reprint.
                        }
                    }
                };
                boss.LISTEN(tier::preview, e2::form::prop::cwd, path)
                {
                    if (cwd_sync)
                    {
                        boss.bell::passover();
                        cwd_path = path;
                    }
                };
                if (cwd_commands.size())
                {
                    boss.LISTEN(tier::anycast, e2::form::prop::cwd, path)
                    {
                        if (cwd_sync && path.size() && cwd_path != path)
                        {
                            cwd_path = path;
                            auto cwd = cwd_path.string();
                            if (cwd.find(' ') != text::npos) cwd = '\"' + cwd + '\"';
                            auto cmd = cwd_commands;
                            utf::replace_all(cmd, "$P", cwd);
                            boss.data_out(cmd);
                        }
                    };
                }
            });
        auto sb = layers->attach(ui::fork::ctor());
        auto vt = sb->attach(slot::_2, ui::grip<axis::Y>::ctor(scroll));
        auto& term_bgc = term->get_color().bgc();
        auto& drawfx = term->base::field([&](auto& boss, auto& canvas, auto handle, auto /*object_len*/, auto handle_len, auto region_len, auto wide)
        {
            static auto box1 = "▄"sv;
            static auto box2 = ' ';
            auto window_clr = skin::color(tone::window_clr);
            if (handle_len != region_len) // Show only if it is oversized.
            {
                if (wide) // Draw full scrollbar on mouse hover.
                {
                    canvas.fill([&](cell& c){ c.txt(box2).link(boss.bell::id).xlight().bgc().mix(window_clr.bgc()); });
                    canvas.fill(handle, [&](cell& c){ c.bgc().xlight(2); });
                }
                else
                {
                    canvas.fill([&](cell& c){ c.txt(box1).fgc(c.bgc()).bgc(term_bgc).fgc().mix(window_clr.bgc()); });
                    canvas.fill(handle, [&](cell& c){ c.link(boss.bell::id).fgc().xlight(2); });
                }
            }
            else canvas.fill([&](cell& c){ c.txt(box1).fgc(c.bgc()).bgc(term_bgc).fgc().mix(window_clr.bgc()); });
        });
        auto hz = term_stat_area->attach(slot::_2, ui::grip<axis::X>::ctor(scroll, drawfx))
            ->limits({ -1, 1 }, { -1, 1 });

        config.cd("/config/terminal", "/config/defapp");
        auto [slot1, cover, menu_data] = app::shared::menu::load(config);
        auto menu = object->attach(slot::_1, slot1)
            ->shader(cell::shaders::fuse(window_clr))
            ->invoke([&](auto& boss)
            {
                boss.LISTEN(tier::release, e2::area, new_area, -, (menu_height))
                {
                    *menu_height = new_area.size.y;
                };
            });
        cover->invoke([&, &slot1 = slot1](auto& boss) //todo clang 15.0.0 still disallows capturing structured bindings (wait for clang 16.0.0)
        {
            auto& bar = boss.base::field(cell{ "▀"sv }.link(slot1->id));
            auto& winsz = boss.base::field(dot_00);
            auto& visible = boss.base::field(slot1->back() != boss.This());
            auto& check_state = boss.base::field([state = true, &winsz, &visible](base& boss) mutable
            {
                if (std::exchange(state, visible || winsz.y != 1) != state)
                {
                    boss.base::riseup(tier::preview, e2::form::prop::ui::cache, state);
                }
            });
            boss.LISTEN(tier::release, e2::form::state::visible, menu_visible)
            {
                visible = menu_visible;
                check_state(boss);
            };
            boss.LISTEN(tier::anycast, e2::form::upon::resized, new_area)
            {
                winsz = new_area.size;
                check_state(boss);
            };
            boss.LISTEN(tier::release, e2::render::any, parent_canvas, -, (borders))
            {
                auto full = parent_canvas.full();
                if (winsz.y != 1 && borders)
                {
                    parent_canvas.cage(full, borders, [&](cell& c){ c.txt(whitespace).link(bar); });
                    full -= borders;
                }
                auto bgc = winsz.y != 1 ? term_bgc : 0;
                parent_canvas.fill(full, [&](cell& c){ c.fgc(c.bgc()).bgc(bgc).txt(bar).link(bar); });
            };
        });

        //term->attach_property(ui::tty::events::colors::bg,      terminal::events::release::colors::bg)
        //    ->attach_property(ui::tty::events::colors::fg,      terminal::events::release::colors::fg)
        //    ->attach_property(ui::tty::events::selmod,          terminal::events::release::selection::mode)
        //    ->attach_property(ui::tty::events::onesht,          terminal::events::release::selection::shot)
        //    ->attach_property(ui::tty::events::selalt,          terminal::events::release::selection::box)
        //    ->attach_property(ui::tty::events::rawkbd,          terminal::events::release::rawkbd)
        //    ->attach_property(ui::tty::events::io_log,          terminal::events::release::io_log)
        //    ->attach_property(ui::tty::events::layout::wrapln,  terminal::events::release::wrapln)
        //    ->attach_property(ui::tty::events::layout::align,   terminal::events::release::align)
        //    ->attach_property(ui::tty::events::search::status,  terminal::events::search::status)
        term->invoke([&](auto& boss)
            {
                ui_term_events(boss, appcfg);
            });
        return window;
    };

    app::shared::initialize teletype_builder{ app::teletype::id, build_teletype };
    app::shared::initialize terminal_builder{ app::terminal::id, build_terminal };
}