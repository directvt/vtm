// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_APPS_HPP
#define NETXS_APPS_HPP

#include "ui/controls.hpp"

namespace netxs::app::shared
{
    using namespace std::placeholders;
    using namespace netxs::console;
    using namespace netxs;

    using slot = ui::slot;
    using axis = ui::axis;
    using axes = ui::axes;
    using snap = ui::snap;
    using id_t = netxs::input::id_t;

    //static iota    max_count = 20;// 50;
    static iota    max_vtm = 3;
    static iota    vtm_count = 0;
    static iota    tile_count = 0;
    //constexpr auto del_timeout = 1s;

    //auto const highlight_color2 = tint::blackdk ;
    //auto const highlightdk_color = tint::bluedk  ;
    auto const highlight_color   = tint::bluelt  ;
    auto const warning_color     = tint::yellowdk;
    auto const danger_color      = tint::redlt   ;
    auto const action_color      = tint::greenlt ;
    auto background_color = cell{}.fgc(whitedk).bgc(0xFF000000 /* blackdk */);

    const static auto c7 = cell{}.bgc(whitedk).fgc(blackdk);
    const static auto c6 = cell{}.bgc(action_color).fgc(whitelt);
    const static auto x6 = cell{ c6 }.bga(0x00).fga(0x00);
    const static auto c5 = cell{}.bgc(danger_color).fgc(whitelt);
    const static auto x5 = cell{ c5 }.bga(0x00).fga(0x00);
    const static auto c4 = cell{}.bgc(highlight_color);
    const static auto x4 = cell{ c4 }.bga(0x00);
    const static auto c3 = cell{}.bgc(highlight_color).fgc(0xFFffffff);
    const static auto x3 = cell{ c3 }.bga(0x00).fga(0x00);
    const static auto c2 = cell{}.bgc(warning_color).fgc(whitelt);
    const static auto x2 = cell{ c2 }.bga(0x00);
    const static auto c1 = cell{}.bgc(danger_color).fgc(whitelt);
    const static auto x1 = cell{ c1 }.bga(0x00);
    const static auto x0 = cell{ c3 }.bgc(0xFF000000); //todo make it as desktop bg
    const static auto term_menu_bg = rgba{ 0x80404040 };

    auto scroll_bars = [](auto master)
    {
        auto scroll_bars = ui::fork::ctor();
            auto scroll_down = scroll_bars->attach(slot::_1, ui::fork::ctor(axis::Y));
                auto hz = scroll_down->attach(slot::_2, ui::grip<axis::X>::ctor(master));
                auto vt = scroll_bars->attach(slot::_2, ui::grip<axis::Y>::ctor(master));
        return scroll_bars;
    };
    auto scroll_bars_term = [](auto master)
    {
        auto scroll_bars = ui::fork::ctor();
            auto scroll_head = scroll_bars->attach(slot::_1, ui::fork::ctor(axis::Y));
                auto hz = scroll_head->attach(slot::_1, ui::grip<axis::X>::ctor(master));
                auto vt = scroll_bars->attach(slot::_2, ui::grip<axis::Y>::ctor(master));
        return scroll_bars;
    };

    // Menu bar (shrinkable on right-click).
    auto custom_menu = [&](bool full_size, std::list<std::pair<text, std::function<void(ui::pads&)>>> menu_items)
    {
        auto menu_block = ui::park::ctor()
            ->plugin<pro::limit>(twod{ -1, full_size ? 3 : 1 }, twod{ -1, full_size ? 3 : 1 })
            ->invoke([](ui::park& boss)
            {
                boss.SUBMIT(tier::release, hids::events::mouse::button::click::right, gear)
                {
                    auto& limit = boss.plugins<pro::limit>();
                    auto limits = limit.get();
                    limits.min.y = limits.max.y = limits.min.y == 1 ? 3 : 1;
                    limit.set(limits);
                    boss.reflow();
                    gear.dismiss();
                };
                boss.base::broadcast->SUBMIT(tier::release, e2::form::prop::menusize, size)
                {
                    auto& limit = boss.plugins<pro::limit>();
                    auto limits = limit.get();
                    switch (size)
                    {
                        default:
                        case 2: limits.min.y = limits.max.y = 3; break;
                        case 1: limits.min.y = limits.max.y = 1; break;
                        case 0: limits.min.y = limits.max.y = 0; break;
                    }
                    limit.set(limits);
                    boss.reflow();
                };
            });
        auto menu_area = menu_block->attach(snap::stretch, snap::center, ui::fork::ctor())
                                    ->active();
            auto inner_pads = dent{ 1,2,1,1 };
            auto menu_list = menu_area->attach(slot::_1, ui::fork::ctor());
                                        
                menu_list->attach(slot::_1, ui::pads::ctor(inner_pads, dent{ 0 }))
                            ->plugin<pro::fader>(x3, c3, 150ms)
                            ->invoke([&](ui::pads& boss)
                            {
                                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                                {
                                    boss.base::template riseup<tier::release>(e2::form::maximize, gear);
                                    gear.dismiss();
                                };
                            })
                        ->attach(ui::item::ctor(" ≡", faux, true));
                auto scrl_list = menu_list->attach(slot::_2, ui::rail::ctor(axes::ONLY_X, axes::ONLY_X))
                                            ->attach(ui::list::ctor(axis::X));
            for (auto& body : menu_items) scrl_list->attach(ui::pads::ctor(inner_pads, dent{ 1 }))
                                                    ->plugin<pro::fader>(x3, c3, 150ms)
                                                    ->invoke(body.second)
                                                    ->attach(ui::item::ctor(body.first, faux, true));
            menu_area->attach(slot::_2, ui::pads::ctor(dent{ 2,2,1,1 }, dent{}))
                        ->plugin<pro::fader>(x1, c1, 150ms)
                        ->invoke([&](auto& boss)
                        {
                            boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.base::template riseup<tier::release>(e2::form::quit, boss.This());
                                gear.dismiss();
                            };
                        })
                        ->attach(ui::item::ctor("×"));
        return menu_block;
    };

    auto main_menu = [&]()
    {
        auto items = std::list
        {
            std::pair<text, std::function<void(ui::pads&)>>{ ansi::und(true).add("F").nil().add("ile"), [&](auto& boss){ } },
            std::pair<text, std::function<void(ui::pads&)>>{ ansi::und(true).add("E").nil().add("dit"), [&](auto& boss){ } },
            std::pair<text, std::function<void(ui::pads&)>>{ ansi::und(true).add("V").nil().add("iew"), [&](auto& boss){ } },
            std::pair<text, std::function<void(ui::pads&)>>{ ansi::und(true).add("D").nil().add("ata"), [&](auto& boss){ } },
            std::pair<text, std::function<void(ui::pads&)>>{ ansi::und(true).add("H").nil().add("elp"), [&](auto& boss){ } },
        };
        return custom_menu(true, items);
    };

    auto terminal_menu = [&](bool full_size)
    {
        auto items = std::list
        {
        #ifdef DEMO
            std::pair<text, std::function<void(ui::pads&)>>{ "T1",
            [](ui::pads& boss)
            {
                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                {
                    auto data = "ls /bin\n"s;
                    boss.base::broadcast->SIGNAL(tier::preview, app::term::events::data::out, data);
                    gear.dismiss(true);
                };
            }},
            std::pair<text, std::function<void(ui::pads&)>>{ "T2",
            [](ui::pads& boss)
            {
                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                {
                    auto data = "ping -c 3 127.0.0.1 | ccze -A\n"s;
                    boss.base::broadcast->SIGNAL(tier::preview, app::term::events::data::out, data);
                    gear.dismiss(true);
                };
            }},
            std::pair<text, std::function<void(ui::pads&)>>{ "T3",
            [](ui::pads& boss)
            {
                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                {
                    auto data = "curl wttr.in\n"s;
                    boss.base::broadcast->SIGNAL(tier::preview, app::term::events::data::out, data);
                    gear.dismiss(true);
                };
            }},
        #endif
        #ifdef PROD
            std::pair<text, std::function<void(ui::pads&)>>{ "Clear",
            [](ui::pads& boss)
            {
                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                {
                    boss.base::broadcast->SIGNAL(tier::preview, app::term::events::cmd, app::term::commands::ui::clear);
                    gear.dismiss(true);
                };
            }},
        #endif
            std::pair<text, std::function<void(ui::pads&)>>{ "Reset",
            [](ui::pads& boss)
            {
                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                {
                    boss.base::broadcast->SIGNAL(tier::preview, app::term::events::cmd, app::term::commands::ui::reset);
                    gear.dismiss(true);
                };
            }},
            std::pair<text, std::function<void(ui::pads&)>>{ "=─",
            [](ui::pads& boss)
            {
                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                {
                    boss.base::broadcast->SIGNAL(tier::preview, app::term::events::cmd, app::term::commands::ui::left);
                    gear.dismiss(true);
                };
                boss.base::broadcast->SUBMIT(tier::release, app::term::events::layout::align, align)
                {
                    //todo unify, get boss base colors, don't use x3
                    boss.color(align == bias::left ? 0xFF00ff00 : x3.fgc(), x3.bgc());
                };
            }},
            std::pair<text, std::function<void(ui::pads&)>>{ "─=─",
            [](ui::pads& boss)
            {
                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                {
                    boss.base::broadcast->SIGNAL(tier::preview, app::term::events::cmd, app::term::commands::ui::center);
                    gear.dismiss(true);
                };
                boss.base::broadcast->SUBMIT(tier::release, app::term::events::layout::align, align)
                {
                    //todo unify, get boss base colors, don't use x3
                    boss.color(align == bias::center ? 0xFF00ff00 : x3.fgc(), x3.bgc());
                };
            }},
            std::pair<text, std::function<void(ui::pads&)>>{ "─=",
            [](ui::pads& boss)
            {
                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                {
                    boss.base::broadcast->SIGNAL(tier::preview, app::term::events::cmd, app::term::commands::ui::right);
                    gear.dismiss(true);
                };
                boss.base::broadcast->SUBMIT(tier::release, app::term::events::layout::align, align)
                {
                    //todo unify, get boss base colors, don't use x3
                    boss.color(align == bias::right ? 0xFF00ff00 : x3.fgc(), x3.bgc());
                };
            }},
            std::pair<text, std::function<void(ui::pads&)>>{ "Wrap",
            [](ui::pads& boss)
            {
                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                {
                    boss.base::broadcast->SIGNAL(tier::preview, app::term::events::cmd, app::term::commands::ui::togglewrp);
                    gear.dismiss(true);
                };
                boss.base::broadcast->SUBMIT(tier::release, app::term::events::layout::wrapln, wrapln)
                {
                    //todo unify, get boss base colors, don't use x3
                    boss.color(wrapln == wrap::on ? 0xFF00ff00 : x3.fgc(), x3.bgc());
                };
            }},
        };
        return custom_menu(full_size, items);
    };

}

#include "apps/term.hpp"
#include "apps/tile.hpp"
#include "apps/calc.hpp"
#include "apps/text.hpp"
#include "apps/shop.hpp"
#include "apps/logs.hpp"
#include "apps/test.hpp"

#endif // NETXS_APPS_HPP