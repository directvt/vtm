// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_APP_TERM_HPP
#define NETXS_APP_TERM_HPP

#include "../console/terminal.hpp"

namespace netxs::events::userland
{
    struct term
    {
        EVENTPACK( term, netxs::events::userland::root::custom )
        {
            EVENT_XS( cmd   , iota ),
            GROUP_XS( layout, iota ),
            GROUP_XS( data  , iota ),

            SUBSET_XS( layout )
            {
                EVENT_XS( align , bias ),
                EVENT_XS( wrapln, wrap ),
            };
            SUBSET_XS( data )
            {
                EVENT_XS( in , view ),
                EVENT_XS( out, view ),
            };
        };
    };
}

// term: Terminal Emulator.
namespace netxs::app::term
{
    using events = netxs::events::userland::term;

    const auto terminal_menu = [](bool full_size)
    {
        const static auto c3 = app::shared::c3;
        const static auto x3 = app::shared::x3;

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
                    boss.base::broadcast->SIGNAL(tier::preview, app::term::events::cmd, ui::term::commands::ui::clear);
                    gear.dismiss(true);
                };
            }},
        #endif
            std::pair<text, std::function<void(ui::pads&)>>{ "Reset",
            [](ui::pads& boss)
            {
                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                {
                    boss.base::broadcast->SIGNAL(tier::preview, app::term::events::cmd, ui::term::commands::ui::reset);
                    gear.dismiss(true);
                };
            }},
            std::pair<text, std::function<void(ui::pads&)>>{ "=─",
            [](ui::pads& boss)
            {
                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                {
                    boss.base::broadcast->SIGNAL(tier::preview, app::term::events::cmd, ui::term::commands::ui::left);
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
                    boss.base::broadcast->SIGNAL(tier::preview, app::term::events::cmd, ui::term::commands::ui::center);
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
                    boss.base::broadcast->SIGNAL(tier::preview, app::term::events::cmd, ui::term::commands::ui::right);
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
                    boss.base::broadcast->SIGNAL(tier::preview, app::term::events::cmd, ui::term::commands::ui::togglewrp);
                    gear.dismiss(true);
                };
                boss.base::broadcast->SUBMIT(tier::release, app::term::events::layout::wrapln, wrapln)
                {
                    //todo unify, get boss base colors, don't use x3
                    boss.color(wrapln == wrap::on ? 0xFF00ff00 : x3.fgc(), x3.bgc());
                };
            }},
        };
        return app::shared::custom_menu(full_size, items);
    };

    namespace
    {
        auto build = [](view v)
        {
            auto window = ui::cake::ctor();
            window->plugin<pro::focus>()
                  ->plugin<pro::track>()
                  ->plugin<pro::acryl>()
                  ->plugin<pro::cache>();
            auto object = window->attach(ui::fork::ctor(axis::Y))
                                ->colors(whitelt, app::shared::term_menu_bg);
                auto menu = object->attach(slot::_1, terminal_menu(true));
                auto term_stat_area = object->attach(slot::_2, ui::fork::ctor(axis::Y));
                    auto layers = term_stat_area->attach(slot::_1, ui::cake::ctor())
                                                ->plugin<pro::limit>(dot_11, twod{ 400,200 });
                        auto scroll = layers->attach(ui::rail::ctor());
                        {
                            scroll->plugin<pro::limit>(twod{ 10,1 }); // mc crashes when window is too small

                            #if defined(_WIN32)

                                auto inst = scroll->attach(ui::term::ctor("cmd"));

                            #else

                                auto shell = os::get_shell();
                                auto inst = scroll->attach(ui::term::ctor(shell + " -i"));

                            #endif

                            inst->colors(whitelt, blackdk)
                                ->invoke([](auto& boss)
                                {
                                    boss.SUBMIT(tier::release, ui::term::events::layout::wrapln, status)
                                    {
                                        boss.base::broadcast->SIGNAL(tier::release, app::term::events::layout::wrapln, status);
                                    };
                                    boss.SUBMIT(tier::release, ui::term::events::layout::align, status)
                                    {
                                        boss.base::broadcast->SIGNAL(tier::release, app::term::events::layout::align, status);
                                    };
                                    boss.base::broadcast->SUBMIT_T(tier::preview, app::term::events::cmd, boss.bell::tracker, cmd)
                                    {
                                        boss.exec_cmd(static_cast<ui::term::commands::ui::commands>(cmd));
                                    };
                                    boss.base::broadcast->SUBMIT_T(tier::preview, app::term::events::data::in, boss.bell::tracker, data)
                                    {
                                        boss.data_in(data);
                                    };
                                    boss.base::broadcast->SUBMIT_T(tier::preview, app::term::events::data::out, boss.bell::tracker, data)
                                    {
                                        boss.data_out(data);
                                    };
                                    boss.base::broadcast->SUBMIT_T(tier::release, e2::form::upon::started, boss.bell::tracker, root)
                                    {
                                        boss.start();
                                    };
                                });
                        }
                    auto scroll_bars = layers->attach(ui::fork::ctor());
                        auto vt = scroll_bars->attach(slot::_2, ui::grip<axis::Y>::ctor(scroll));
                        auto hz = term_stat_area->attach(slot::_2, ui::grip<axis::X>::ctor(scroll));
            return window;
        };
    }

    app::shared::initialize builder{ "Term", build };
}

#endif // NETXS_APP_TERM_HPP