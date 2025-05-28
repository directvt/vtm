// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

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
    };
    auto build_teletype = [](eccc appcfg, settings& config)
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
    auto build_terminal = [](eccc appcfg, settings& config)
    {
        //log(prompt::resultant_settings, "\n", config);
        auto window_clr = skin::color(tone::window_clr);
        auto border = std::max(0, config.settings::take(attr::borders, 0));
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
                            if (auto gear_ptr = boss.base::template getref<hids>(gear_id)) //todo Apple clang requires template
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
        auto terminal_context = config.settings::push_context("/config/terminal/");
        auto term = scroll->attach(ui::term::ctor(config))
            ->plugin<pro::focus>(pro::focus::mode::focused)
            ->invoke([&](auto& boss)
            {
                auto& cwd_commands = boss.base::property("terminal.cwd_commands", config.settings::take(attr::cwdsync, ""s));
                auto& cwd_sync = boss.base::property("terminal.cwd_sync", 0);
                auto& cwd_path = boss.base::property("terminal.cwd_path", os::fs::path{});
                boss.LISTEN(tier::preview, ui::terminal::events::toggle::cwdsync, state)
                {
                    boss.base::signal(tier::anycast, ui::terminal::events::preview::cwdsync, state);
                };
                boss.LISTEN(tier::anycast, ui::terminal::events::preview::cwdsync, state)
                {
                    if (cwd_sync != state)
                    {
                        cwd_sync = state;
                        boss.base::signal(tier::anycast, ui::terminal::events::release::cwdsync, state);
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
        //config.settings::pop_context();
        term->invoke([&](auto& boss)
        {
            ui_term_events(boss, appcfg);
        });
        return window;
    };

    app::shared::initialize teletype_builder{ app::teletype::id, build_teletype };
    app::shared::initialize terminal_builder{ app::terminal::id, build_terminal };
}