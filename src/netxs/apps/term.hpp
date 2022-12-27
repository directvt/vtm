// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_APP_TERM_HPP
#define NETXS_APP_TERM_HPP

#include "../console/terminal.hpp"

namespace netxs::events::userland
{
    struct term
    {
        using mime = ansi::clip::mime;

        EVENTPACK( term, netxs::events::userland::root::custom )
        {
            EVENT_XS( cmd    , si32 ),
            GROUP_XS( preview, si32 ),
            GROUP_XS( release, si32 ),
            GROUP_XS( data   , si32 ),
            GROUP_XS( search , input::hids ),

            SUBSET_XS( preview )
            {
                EVENT_XS( selmod, si32 ),
                EVENT_XS( align , si32 ),
                EVENT_XS( wrapln, si32 ),
                GROUP_XS( colors, rgba ),

                SUBSET_XS( colors )
                {
                    EVENT_XS( bg, rgba ),
                    EVENT_XS( fg, rgba ),
                };
            };
            SUBSET_XS( release )
            {
                EVENT_XS( selmod, si32 ),
                EVENT_XS( align , si32 ),
                EVENT_XS( wrapln, si32 ),
                GROUP_XS( colors, rgba ),

                SUBSET_XS( colors )
                {
                    EVENT_XS( bg, rgba ),
                    EVENT_XS( fg, rgba ),
                };
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
        };
    };
}

// term: Terminal Emulator.
namespace netxs::app::term
{
    using events = netxs::events::userland::term;
    using mime = clip::mime;

    const auto terminal_menu = [](xml::settings& config)
    {
        auto highlight_color = skin::color(tone::highlight);
        auto c3 = highlight_color;
        auto x3 = cell{ c3 }.alpha(0x00);

        config.cd("/config/term/", "/config/defapp/");
        auto menudata = config.list("menu/item");

        using namespace app::shared;

        static auto brand_options = std::unordered_map<text, menu::item::type>
           {{ menu::type::Splitter, menu::item::Splitter },
            { menu::type::Command,  menu::item::Command  },
            { menu::type::Option,   menu::item::Option   }};

        #define PROC_LIST \
            X(Noop                      ) /* */ \
            X(ClipboardWipe             ) /* */ \
            X(TerminalQuit              ) /* */ \
            X(TerminalMaximize          ) /* */ \
            X(TerminalRestart           ) /* */ \
            X(TerminalSendKey           ) /* */ \
            X(TerminalPaste             ) /* */ \
            X(TerminalSelectionMode     ) /* */ \
            X(TerminalSelectionType     ) /* Linear/Boxed*/ \
            X(TerminalSelectionClear    ) /* */ \
            X(TerminalSelectionCopy     ) /* */ \
            X(TerminalWrapMode          ) /* */ \
            X(TerminalAlignMode         ) /* */ \
            X(TerminalFindNext          ) /* */ \
            X(TerminalFindPrev          ) /* */ \
            X(TerminalOutput            ) /* */ \
            X(TerminalViewportPageUp    ) /* */ \
            X(TerminalViewportPageDown  ) /* */ \
            X(TerminalViewportLineUp    ) /* */ \
            X(TerminalViewportLineDown  ) /* */ \
            X(TerminalViewportPageLeft  ) /* */ \
            X(TerminalViewportPageRight ) /* */ \
            X(TerminalViewportCharLeft  ) /* */ \
            X(TerminalViewportCharRight ) /* */ \
            X(TerminalViewportTop       ) /* */ \
            X(TerminalViewportEnd       ) /* */ \
            X(TerminalViewportCopy      ) /* */ \
            X(TerminalLogStart          ) /* */ \
            X(TerminalLogPause          ) /* */ \
            X(TerminalLogStop           ) /* */ \
            X(TerminalLogAbort          ) /* */ \
            X(TerminalLogRestart        ) /* */ \
            X(TerminalVideoRecStart     ) /* */ \
            X(TerminalVideoRecStop      ) /* */ \
            X(TerminalVideoRecPause     ) /* */ \
            X(TerminalVideoRecAbort     ) /* */ \
            X(TerminalVideoRecRestart   ) /* */ \
            X(TerminalVideoPlay         ) /* */ \
            X(TerminalVideoPause        ) /* */ \
            X(TerminalVideoStop         ) /* */ \
            X(TerminalVideoForward      ) /* */ \
            X(TerminalVideoBackward     ) /* */ \
            X(TerminalVideoHome         ) /* */ \
            X(TerminalVideoEnd          ) /* */

        enum func
        {
            #define X(_proc) _proc,
            PROC_LIST
            #undef X
        };

        static const auto route_options = std::unordered_map<text, func>
        {
            #define X(_proc) { #_proc, func::_proc },
            PROC_LIST
            #undef X
        };

        static const auto _on_leftclick = [](ui::pads& boss, auto& item, auto proc)
        {
            boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
            {
                proc(boss, item, gear);
                gear.dismiss(true);
            };
        };
        static const auto _update_ui = [](ui::pads& boss, menu::item& item)
        {
            auto& look = item.views[item.taken];
            if (boss.client)
            {
                auto& item = *boss.client;
                item.SIGNAL(tier::release, e2::data::text,              look.label);
                boss.SIGNAL(tier::preview, e2::form::prop::ui::tooltip, look.notes);
                item.reflow();
            }
        };
        static const auto _update_to = [](ui::pads& boss, menu::item& item, si32 i)
        {
            item.select(i);
            _update_ui(boss, item);
        };

        struct disp
        {
            using preview = app::term::events::preview;
            using release = app::term::events::release;

            static void Noop(ui::pads& boss, menu::item& item) { }
            static void TerminalWrapMode(ui::pads& boss, menu::item& item)
            {
                item.reindex([](auto& utf8){ return xml::take<bool>(utf8).value() ? wrap::on : wrap::off; });
                _on_leftclick(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, preview::wrapln, item.views[item.taken].value);
                });
                boss.SUBMIT(tier::anycast, release::wrapln, wrapln)
                {
                    _update_to(boss, item, wrapln);
                };
            }
            static void TerminalAlignMode(ui::pads& boss, menu::item& item)
            {
                item.reindex([](auto& utf8){ return netxs::get_or(xml::options::align, utf8, bias::left); });
                _on_leftclick(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, preview::align, item.views[item.taken].value);
                });
                boss.SUBMIT(tier::anycast, release::align, align)
                {
                    _update_to(boss, item, align);
                };
            }
            static void TerminalSelectionMode(ui::pads& boss, menu::item& item)
            {
                item.reindex([](auto& utf8){ return netxs::get_or(xml::options::selmod, utf8, mime::disabled); });
                _on_leftclick(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, preview::selmod, item.views[item.taken].value);
                });
                boss.SUBMIT(tier::anycast, release::selmod, mode)
                {
                    _update_to(boss, item, mode);
                };
            }
            static void TerminalFindPrev(ui::pads& boss, menu::item& item)
            {
                item.reindex([](auto& utf8){ return xml::take<bool>(utf8).value(); });
                _on_leftclick(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::search::reverse, gear);
                });
                boss.SUBMIT(tier::anycast, app::term::events::search::status, status)
                {
                    _update_to(boss, item, (status & 2) ? 1 : 0);
                };
            }
            static void TerminalFindNext(ui::pads& boss, menu::item& item)
            {
                item.reindex([](auto& utf8){ return xml::take<bool>(utf8).value(); });
                _on_leftclick(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::search::forward, gear);
                });
                boss.SUBMIT(tier::anycast, app::term::events::search::status, status)
                {
                    _update_to(boss, item, (status & 1) ? 1 : 0);
                };
            }
            static void TerminalOutput(ui::pads& boss, menu::item& item)
            {
                _on_leftclick(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::data::in, view{ item.views[item.taken].param });
                    if (item.brand == menu::item::Option) _update_ui(boss, item);
                });
            }
            static void TerminalSendKey(ui::pads& boss, menu::item& item)
            {
                _on_leftclick(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::data::out, view{ item.views[item.taken].param });
                    if (item.brand == menu::item::Option) _update_ui(boss, item);
                });
            }
            static void ClipboardWipe(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalQuit(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalMaximize(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalRestart(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalPaste(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalSelectionType(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalSelectionClear(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalSelectionCopy(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalViewportPageUp(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalViewportPageDown(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalViewportLineUp(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalViewportLineDown(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalViewportPageLeft(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalViewportPageRight(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalViewportCharLeft(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalViewportCharRight(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalViewportTop(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalViewportEnd(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalViewportCopy(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalLogStart(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalLogPause(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalLogStop(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalLogAbort(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalLogRestart(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalVideoRecStart(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalVideoRecStop(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalVideoRecPause(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalVideoRecAbort(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalVideoRecRestart(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalVideoPlay(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalVideoPause(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalVideoStop(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalVideoForward(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalVideoBackward(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalVideoHome(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalVideoEnd(ui::pads& boss, menu::item& item)
            {

            }
        };
        using submit_proc = std::function<void(ui::pads&, menu::item&)>;
        static const auto proc_map = std::unordered_map<func, submit_proc>
        {
            #define X(_proc) { func::_proc, &disp::_proc },
            PROC_LIST
            #undef X
        };
        #undef PROC_LIST

        auto list = menu::list{};
        auto defs = menu::item::look{};
        for (auto data_ptr : menudata)
        {
            auto item_ptr = std::make_shared<menu::item>();
            auto& data = *data_ptr;
            auto& item = *item_ptr;
            auto route = data.take(menu::attr::route, func::Noop,          route_options);
            item.brand = data.take(menu::attr::brand, menu::item::Command, brand_options);
            defs.notes = data.take(menu::attr::notes, ""s);
            defs.param = data.take(menu::attr::param, ""s);
            defs.onkey = data.take(menu::attr::onkey, ""s);
            item.alive = route != func::Noop && item.brand != menu::item::Splitter;
            for (auto label : data.list(menu::attr::label))
            {
                item.views.push_back(
                {
                    .label = label->value(),
                    .notes = label->take(menu::attr::notes, defs.notes),
                    .param = label->take(menu::attr::param, defs.param),
                    .onkey = label->take(menu::attr::onkey, defs.onkey),
                });
            }
            if (item.views.empty())
            {
                log("term: drop menu item without label");
                continue;
            }
            auto setup = [route](ui::pads& boss, menu::item& item)
            {
                if (item.brand == menu::item::Option)
                {
                    boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                    {
                        item.taken = (item.taken + 1) % item.views.size();
                    };
                }
                auto& initproc = proc_map.find(route)->second;
                initproc(boss, item);
            };
            list.push_back({ item_ptr, setup });
        }
        return menu::create(config, list);
    };

    namespace
    {
        auto build = [](text cwd, text arg, xml::settings& config, text patch)
        {
            auto menu_white = skin::color(tone::menu_white);
            auto cB = menu_white;

            auto window = ui::cake::ctor();
            auto arg_shadow = view{ arg };
            auto term_type = shared::app_class(arg_shadow);
            arg = arg_shadow;
            if (term_type == shared::app_type::normal)
            {
                window->plugin<pro::focus>()
                      ->plugin<pro::track>()
                      ->plugin<pro::acryl>()
                      ->plugin<pro::cache>();
            }
            else window->plugin<pro::focus>(faux);

            auto object = window->attach(ui::fork::ctor(axis::Y))
                                ->colors(cB.fgc(), cB.bgc());
            auto term_stat_area = object->attach(slot::_2, ui::fork::ctor(axis::Y));
                    auto layers = term_stat_area->attach(slot::_1, ui::cake::ctor())
                                                ->plugin<pro::limit>(dot_11, twod{ 400,200 });
                        auto scroll = layers->attach(ui::rail::ctor());
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

                            auto shell = os::get_shell() + " -i";
                            auto inst = scroll->attach(ui::term::ctor(cwd, arg.empty() ? shell : arg, config));

                            auto scroll_bars = layers->attach(ui::fork::ctor());
                                auto vt = scroll_bars->attach(slot::_2, ui::grip<axis::Y>::ctor(scroll));
                                auto hz = term_stat_area->attach(slot::_2, ui::grip_fx2<axis::X>::ctor(scroll))
                                                        ->plugin<pro::limit>(twod{ -1,1 }, twod{ -1,1 })
                                                        ->invoke([&](auto& boss)
                                                        {
                                                            boss.SUBMIT(tier::anycast, app::term::events::release::colors::bg, bg)
                                                            {
                                                                boss.color(boss.color().bgc(bg).txt(""));
                                                            };
                                                        });

            auto [slot1, cover, menu_data] = terminal_menu(config);
            auto menu = object->attach(slot::_1, slot1);
            auto menu_id = slot1->id;
            cover->invoke([&](auto& boss)
            {
                boss.colors(cell{ cB }.inv(true).txt("▀"sv).link(menu_id));
                boss.SUBMIT(tier::anycast, app::term::events::release::colors::bg, bg)
                {
                    boss.color(boss.color().fgc(bg));
                };
            });

            inst->attach_property(ui::term::events::colors::bg,      app::term::events::release::colors::bg)
                ->attach_property(ui::term::events::colors::fg,      app::term::events::release::colors::fg)
                ->attach_property(ui::term::events::selmod,          app::term::events::release::selmod)
                ->attach_property(ui::term::events::layout::wrapln,  app::term::events::release::wrapln)
                ->attach_property(ui::term::events::layout::align,   app::term::events::release::align)
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
                    boss.SUBMIT(tier::anycast, app::term::events::preview::colors::bg, bg)
                    {
                        boss.set_bg_color(bg);
                    };
                    boss.SUBMIT(tier::anycast, app::term::events::preview::colors::fg, fg)
                    {
                        boss.set_fg_color(fg);
                    };
                    boss.SUBMIT(tier::anycast, e2::form::prop::colors::any, clr)
                    {
                        auto deed = boss.bell::template protos<tier::anycast>();
                             if (deed == e2::form::prop::colors::bg.id) boss.SIGNAL(tier::anycast, app::term::events::preview::colors::bg, clr);
                        else if (deed == e2::form::prop::colors::fg.id) boss.SIGNAL(tier::anycast, app::term::events::preview::colors::fg, clr);
                    };
                    boss.SUBMIT(tier::anycast, app::term::events::preview::selmod, selmod)
                    {
                        boss.set_selmod(selmod);
                    };
                    boss.SUBMIT(tier::anycast, app::term::events::preview::wrapln, wrapln)
                    {
                        boss.set_wrapln(wrapln);
                    };
                    boss.SUBMIT(tier::anycast, app::term::events::preview::align, align)
                    {
                        boss.set_align(align);
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
            return window;
        };
    }

    app::shared::initialize builder{ "term", build };
}

#endif // NETXS_APP_TERM_HPP