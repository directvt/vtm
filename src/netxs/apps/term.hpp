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
            GROUP_XS( colors, rgba ),
            GROUP_XS( layout, si32 ),
            GROUP_XS( data  , si32 ),
            GROUP_XS( search, input::hids ),

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
            SUBSET_XS( search )
            {
                EVENT_XS( forward, input::hids ),
                EVENT_XS( reverse, input::hids ),
                EVENT_XS( status , si32        ),
            };
            SUBSET_XS( colors )
            {
                EVENT_XS( bg, rgba ),
                EVENT_XS( fg, rgba ),
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

        auto items = app::shared::menu_list_type
        {
            { true, "=─", " Align text lines on left side   \n"
                          " - applied to selection if it is ",
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
            { true, "─=─", " Center text lines               \n"
                           " - applied to selection if it is ",
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
            { true, "─=", " Align text lines on right side  \n"
                          " - applied to selection if it is ",
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
            { true, "Wrap", " Wrapping text lines on/off      \n"
                            " - applied to selection if it is ",
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
            { true, "Selection", " Text selection mode ",
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
                            //"Selection"
                            if (boss.client) boss.client->SIGNAL(tier::release, e2::data::text, "Selection");
                            boss.color(x3.fgc(), x3.bgc());
                            break;
                        case ui::term::xsgr::textonly:
                            //"Text only"
                            if (boss.client) boss.client->SIGNAL(tier::release, e2::data::text, "Plaintext");
                            boss.color(0xFF00ff00, x3.bgc());
                            break;
                        case ui::term::xsgr::ansitext:
                            //"Rich-Text"
                            //"+ANSI/SGR"
                            if (boss.client) boss.client->SIGNAL(tier::release, e2::data::text, "ANSI-text");
                            boss.color(0xFF00ffff, x3.bgc());
                            break;
                    }
                };
            }},
            { true, "<", " Previuos match                    \n"
                         " - using clipboard if no selection \n"
                         " - page up if no clipboard data    ",
            [](ui::pads& boss)
            {
                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::search::reverse, gear);
                    gear.dismiss(true);
                };
                boss.SUBMIT(tier::anycast, app::term::events::search::status, mode)
                {
                    //todo unify, get boss base colors, don't use x3
                    boss.color(mode & 2 ? 0xFF00ff00 : x3.fgc(), x3.bgc());
                };
            }},
            { true, ">", " Next match                        \n"
                         " - using clipboard if no selection \n"
                         " - page down if no clipboard data  ",
            [](ui::pads& boss)
            {
                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::search::forward, gear);
                    gear.dismiss(true);
                };
                boss.SUBMIT(tier::anycast, app::term::events::search::status, mode)
                {
                    //todo unify, get boss base colors, don't use x3
                    boss.color(mode & 1 ? 0xFF00ff00 : x3.fgc(), x3.bgc());
                };
            }},
            { faux, "  ", " ...empty menu block for safety ",
            [](ui::pads& boss)
            {
            }},
            { true, "Clear", " Clear TTY viewport ",
            [](ui::pads& boss)
            {
                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::cmd, ui::term::commands::ui::clear);
                    gear.dismiss(true);
                };
            }},
            { true, "Reset", " Clear scrollback and SGR-attributes ",
            [](ui::pads& boss)
            {
                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::cmd, ui::term::commands::ui::reset);
                    gear.dismiss(true);
                };
            }},
        };
        return app::shared::custom_menu(full_size, items);
    };

    namespace
    {
        auto build = [](text cwd, text arg)
        {
            auto window = ui::cake::ctor();
            auto arg_shadow = view{ arg };
            auto term_type = shared::app_class(arg_shadow);
            arg = arg_shadow;
            if (term_type == shared::app_type::normal) window->plugin<pro::focus>()
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
                                            auto gates = e2::form::state::keybd::enlist.param();
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
                            auto inst = scroll->attach(ui::term::ctor(cwd, arg.empty() ? shell + " -i"
                                                                                       : arg));

                            inst->attach_property(ui::term::events::colors::bg,      app::term::events::colors::bg)
                                ->attach_property(ui::term::events::colors::fg,      app::term::events::colors::fg)
                                ->attach_property(ui::term::events::selmod,          app::term::events::selmod)
                                ->attach_property(ui::term::events::layout::wrapln,  app::term::events::layout::wrapln)
                                ->attach_property(ui::term::events::layout::align,   app::term::events::layout::align)
                                ->attach_property(ui::term::events::search::status,  app::term::events::search::status)
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
                                    boss.SUBMIT(tier::anycast, app::term::events::colors::bg, bg)
                                    {
                                        boss.set_bg_color(bg);
                                    };
                                    boss.SUBMIT(tier::anycast, app::term::events::colors::fg, fg)
                                    {
                                        boss.set_fg_color(fg);
                                    };
                                    boss.SUBMIT(tier::anycast, e2::form::prop::colors::any, clr)
                                    {
                                        auto deed = boss.bell::template protos<tier::anycast>();
                                             if (deed == e2::form::prop::colors::bg.id) boss.SIGNAL(tier::anycast, app::term::events::colors::bg, clr);
                                        else if (deed == e2::form::prop::colors::fg.id) boss.SIGNAL(tier::anycast, app::term::events::colors::fg, clr);
                                    };
                                    boss.SUBMIT(tier::anycast, e2::form::upon::started, root)
                                    {
                                        boss.start();
                                    };
                                    boss.SUBMIT(tier::anycast, app::term::events::search::forward, gear)
                                    {
                                        boss.search(gear, feed::fwd);
                                    };
                                    boss.SUBMIT(tier::anycast, app::term::events::search::reverse, gear)
                                    {
                                        boss.search(gear, feed::rev);
                                    };
                                });
                        }
                    auto scroll_bars = layers->attach(ui::fork::ctor());
                        auto vt = scroll_bars->attach(slot::_2, ui::grip<axis::Y>::ctor(scroll));
                        auto hz = term_stat_area->attach(slot::_2, ui::grip<axis::X>::ctor(scroll));
            return window;
        };
    }

    app::shared::initialize builder{ "term", build };
}

#endif // NETXS_APP_TERM_HPP