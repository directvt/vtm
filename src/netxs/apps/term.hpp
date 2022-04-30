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
            EVENT_XS( cmd   , si32 ),
            EVENT_XS( selmod, si32 ),
            EVENT_XS( colors, cell ),
            GROUP_XS( layout, si32 ),
            GROUP_XS( data  , si32 ),

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
                    boss.SIGNAL(tier::anycast, app::term::events::data::out, data);
                    gear.dismiss(true);
                };
            }},
            std::pair<text, std::function<void(ui::pads&)>>{ "T2",
            [](ui::pads& boss)
            {
                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                {
                    auto data = "ping -c 3 127.0.0.1 | ccze -A\n"s;
                    boss.SIGNAL(tier::anycast, app::term::events::data::out, data);
                    gear.dismiss(true);
                };
            }},
            std::pair<text, std::function<void(ui::pads&)>>{ "T3",
            [](ui::pads& boss)
            {
                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                {
                    auto data = "curl wttr.in\n"s;
                    boss.SIGNAL(tier::anycast, app::term::events::data::out, data);
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
                    boss.SIGNAL(tier::anycast, app::term::events::cmd, ui::term::commands::ui::clear);
                    gear.dismiss(true);
                };
            }},
        #endif
            std::pair<text, std::function<void(ui::pads&)>>{ "Reset",
            [](ui::pads& boss)
            {
                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::cmd, ui::term::commands::ui::reset);
                    gear.dismiss(true);
                };
            }},
            std::pair<text, std::function<void(ui::pads&)>>{ "=─",
            [](ui::pads& boss)
            {
                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::cmd, ui::term::commands::ui::left);
                    gear.dismiss(true);
                };
                boss.SUBMIT(tier::anycast, app::term::events::layout::align, align)
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
                    boss.SIGNAL(tier::anycast, app::term::events::cmd, ui::term::commands::ui::center);
                    gear.dismiss(true);
                };
                boss.SUBMIT(tier::anycast, app::term::events::layout::align, align)
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
                    boss.SIGNAL(tier::anycast, app::term::events::cmd, ui::term::commands::ui::right);
                    gear.dismiss(true);
                };
                boss.SUBMIT(tier::anycast, app::term::events::layout::align, align)
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
                    boss.SIGNAL(tier::anycast, app::term::events::cmd, ui::term::commands::ui::togglewrp);
                    gear.dismiss(true);
                };
                boss.SUBMIT(tier::anycast, app::term::events::layout::wrapln, wrapln)
                {
                    //todo unify, get boss base colors, don't use x3
                    boss.color(wrapln == wrap::on ? 0xFF00ff00 : x3.fgc(), x3.bgc());
                };
            }},
            //todo unify
            std::pair<text, std::function<void(ui::pads&)>>{ "Selection",
            [](ui::pads& boss)
            {
                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::cmd, ui::term::commands::ui::togglesel);
                    gear.dismiss(true);
                };
                boss.SUBMIT(tier::anycast, app::term::events::selmod, selmod)
                {
                    //todo unify, get boss base colors, don't use x3, make it configurable
                    switch (selmod)
                    {
                        default:
                        case ui::term::xsgr::disabled:
                            //boss.attach(ui::item::ctor("Selection", faux, true));
                            if (boss.client) boss.client->SIGNAL(tier::release, e2::data::text, "Selection");
                            boss.color(x3.fgc(), x3.bgc());
                            break;
                        case ui::term::xsgr::textonly:
                            //boss.attach(ui::item::ctor("Text only", faux, true));
                            if (boss.client) boss.client->SIGNAL(tier::release, e2::data::text, "Plaintext");
                            boss.color(0xFF00ff00, x3.bgc());
                            break;
                        case ui::term::xsgr::ansitext:
                            //boss.attach(ui::item::ctor("Rich-Text", faux, true));
                            //boss.attach(ui::item::ctor("+ANSI/SGR", faux, true));
                            if (boss.client) boss.client->SIGNAL(tier::release, e2::data::text, "ANSI-text");
                            boss.color(0xFF00ffff, x3.bgc());
                            break;
                    }
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
                            auto min_size = twod{ 12,1 }; // mc crashes when window is too small
                            auto max_size = -dot_11;
                            auto forced_clamp = faux;
                            auto forced_resize = true;
                            scroll->plugin<pro::limit>(min_size, max_size, forced_clamp, forced_resize)
                                  ->invoke([](auto& boss)
                                  {
                                    boss.SUBMIT(tier::preview, e2::form::prop::window::size, new_size)
                                    {
                                        // Axis x/y (see XTWINOPS):
                                        //   -1 -- preserve
                                        //    0 -- maximize (toggle)
                                        if (new_size == dot_00) // Toggle maximize/restore terminal window (only if it is focused by someone).
                                        {
                                            auto gates = decltype(e2::form::state::keybd::enlist)::type{};
                                            boss.SIGNAL(tier::anycast, e2::form::state::keybd::enlist, gates);
                                            if (gates.size())
                                            if (auto gate_ptr = bell::getref(gates.back()))
                                            {
                                                gate_ptr->SIGNAL(tier::release, e2::form::proceed::onbehalf, [&](auto& gear)
                                                {
                                                    boss.template riseup<tier::release>(e2::form::maximize, gear);
                                                });
                                            }
                                        }
                                        else
                                        {
                                            auto size = boss.size();
                                            new_size = new_size.less(dot_11, size, std::max(dot_11, new_size));
                                            boss.SIGNAL(tier::release, e2::form::prop::window::size, new_size);
                                        }
                                    };
                                  });

                            auto shell = os::get_shell();
                            auto inst = scroll->attach(ui::term::ctor(v.empty() ? shell + " -i"
                                                                                : text{ v }));

                            inst->attach_property(ui::term::events::colors,         app::term::events::colors)
                                ->attach_property(ui::term::events::selmod,         app::term::events::selmod)
                                ->attach_property(ui::term::events::layout::wrapln, app::term::events::layout::wrapln)
                                ->attach_property(ui::term::events::layout::align,  app::term::events::layout::align)
                                ->invoke([](auto& boss)
                                {
                                    boss.SUBMIT(tier::anycast, app::term::events::cmd, cmd)
                                    {
                                        boss.exec_cmd(static_cast<ui::term::commands::ui::commands>(cmd));
                                    };
                                    boss.SUBMIT(tier::anycast, app::term::events::data::in, data)
                                    {
                                        boss.data_in(data);
                                    };
                                    boss.SUBMIT(tier::anycast, app::term::events::data::out, data)
                                    {
                                        boss.data_out(data);
                                    };
                                    //todo add color picker to the menu
                                    boss.SUBMIT(tier::anycast, app::term::events::colors, brush)
                                    {
                                        boss.set_color(brush);
                                    };

                                    boss.SUBMIT(tier::anycast, e2::form::upon::started, root)
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