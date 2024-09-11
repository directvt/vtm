// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

#include "../desktopio/application.hpp"
#include "../desktopio/terminal.hpp"

namespace netxs::events::userland
{
    struct terminal
    {
        EVENTPACK( terminal, ui::e2::extra::slot3 )
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
    };
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
        static constexpr auto cwdsync = "cwdsync";
        static constexpr auto borders = "layout/border";
    }

    using events = netxs::events::userland::terminal;

    namespace
    {
        using namespace app::shared;
        auto _update(ui::item& boss, menu::item& item)
        {
            auto& look = item.views[item.taken];
            boss.SIGNAL(tier::release, e2::data::utf8,              look.label);
            boss.SIGNAL(tier::preview, e2::form::prop::ui::tooltip, look.notes);
            boss.reflow();
        }
        auto _update_gear(ui::item& boss, menu::item& item, hids& gear)
        {
            auto& look = item.views[item.taken];
            gear.set_tooltip(look.notes, true);
            _update(boss, item);
        }
        auto _update_to(ui::item& boss, menu::item& item, si32 i)
        {
            item.select(i);
            _update(boss, item);
        }
        template<bool AutoUpdate = faux, class P>
        auto _submit(ui::item& boss, menu::item& item, P proc)
        {
            if (item.brand == menu::item::Repeat)
            {
                auto& tick = boss.plugins<pro::timer>();
                boss.LISTEN(tier::release, hids::events::mouse::button::down::left, gear)
                {
                    if (item.views.size())
                    {
                        item.taken = (item.taken + 1) % item.views.size();
                    }
                    if (gear.capture(boss.id))
                    {
                        proc(boss, item, gear);
                        tick.actify(0, skin::globals().repeat_delay, [&, proc](auto)
                        {
                            proc(boss, item, gear);
                            tick.actify(1, skin::globals().repeat_rate, [&, proc](auto)
                            {
                                proc(boss, item, gear);
                                return true; // Repeat forever.
                            });
                            return faux; // One shot call (first).
                        });
                        gear.dismiss(true);
                    }
                    if (item.views.size())
                    {
                        _update_gear(boss, item, gear);
                    }
                };
                boss.LISTEN(tier::release, hids::events::mouse::button::up::left, gear)
                {
                    tick.pacify();
                    gear.setfree();
                    gear.dismiss(true);
                    if (item.views.size() && item.taken)
                    {
                        item.taken = 0;
                        _update_gear(boss, item, gear);
                    }
                };
                boss.LISTEN(tier::release, e2::form::state::mouse, active)
                {
                    if (!active && tick)
                    {
                        tick.pacify();
                        if (item.views.size() && item.taken)
                        {
                            item.taken = 0;
                            _update(boss, item);
                        }
                    }
                };
            }
            else
            {
                boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                {
                    proc(boss, item, gear);
                    if constexpr (AutoUpdate)
                    {
                        if (item.brand == menu::item::Option) _update_gear(boss, item, gear);
                    }
                    gear.nodbl = true;
                };
            }
        };
        auto construct_menu(xmls& config)
        {
            //auto highlight_color = skin::color(tone::highlight);
            //auto c3 = highlight_color;

            config.cd("/config/term/", "/config/defapp/");
            auto menudata = config.list("menu/item");

            static auto brand_options = std::unordered_map<text, menu::item::type>
               {{ menu::type::Splitter, menu::item::Splitter },
                { menu::type::Command,  menu::item::Command  },
                { menu::type::Option,   menu::item::Option   },
                { menu::type::Repeat,   menu::item::Repeat   }};

            #define proc_list \
                X(Noop                      ) /* */ \
                X(TerminalQuit              ) /* */ \
                X(TerminalCwdSync           ) /* */ \
                X(TerminalFullscreen        ) /* */ \
                X(TerminalRestart           ) /* */ \
                X(TerminalSendKey           ) /* */ \
                X(TerminalWrapMode          ) /* */ \
                X(TerminalAlignMode         ) /* */ \
                X(TerminalOutput            ) /* */ \
                X(TerminalFindNext          ) /* */ \
                X(TerminalFindPrev          ) /* */ \
                X(TerminalUndo              ) /* Undo/Redo for cooked read on win32 */ \
                X(TerminalRedo              ) /* */ \
                X(TerminalClipboardPaste    ) /* */ \
                X(TerminalClipboardWipe     ) /* */ \
                X(TerminalSelectionCopy     ) /* */ \
                X(TerminalSelectionMode     ) /* */ \
                X(TerminalSelectionRect     ) /* Linear/Rectangular */ \
                X(TerminalSelectionClear    ) /* */ \
                X(TerminalSelectionOneShot  ) /* One-shot toggle to copy text while mouse tracking is active */ \
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
                X(TerminalStdioLog          ) /* */ \
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
                proc_list
                #undef X
            };

            static const auto route_options = std::unordered_map<text, func>
            {
                #define X(_proc) { #_proc, func::_proc },
                proc_list
                #undef X
            };

            struct disp
            {
                using preview = terminal::events::preview;
                using release = terminal::events::release;

                static void Noop(ui::item& /*boss*/, menu::item& /*item*/) { }
                static void TerminalWrapMode(ui::item& boss, menu::item& item)
                {
                    item.reindex([](auto& utf8){ return xml::take<bool>(utf8).value() ? wrap::on : wrap::off; });
                    _submit(boss, item, [](auto& boss, auto& item, auto& /*gear*/)
                    {
                        boss.SIGNAL(tier::anycast, preview::wrapln, item.views[item.taken].value);
                    });
                    boss.LISTEN(tier::anycast, release::wrapln, wrapln)
                    {
                        _update_to(boss, item, wrapln);
                    };
                }
                static void TerminalAlignMode(ui::item& boss, menu::item& item)
                {
                    item.reindex([](auto& utf8){ return netxs::get_or(xml::options::align, utf8, bias::left); });
                    _submit(boss, item, [](auto& boss, auto& item, auto& /*gear*/)
                    {
                        boss.SIGNAL(tier::anycast, preview::align, item.views[item.taken].value);
                    });
                    boss.LISTEN(tier::anycast, release::align, align)
                    {
                        _update_to(boss, item, align);
                    };
                }
                static void TerminalFindPrev(ui::item& boss, menu::item& item)
                {
                    item.reindex([](auto& utf8){ return xml::take<bool>(utf8).value(); });
                    _submit(boss, item, [](auto& boss, auto& /*item*/, auto& gear)
                    {
                        boss.SIGNAL(tier::anycast, terminal::events::search::reverse, gear);
                    });
                    boss.LISTEN(tier::anycast, terminal::events::search::status, status)
                    {
                        _update_to(boss, item, (status & 2) ? 1 : 0);
                    };
                }
                static void TerminalFindNext(ui::item& boss, menu::item& item)
                {
                    item.reindex([](auto& utf8){ return xml::take<bool>(utf8).value(); });
                    _submit(boss, item, [](auto& boss, auto& /*item*/, auto& gear)
                    {
                        boss.SIGNAL(tier::anycast, terminal::events::search::forward, gear);
                    });
                    boss.LISTEN(tier::anycast, terminal::events::search::status, status)
                    {
                        _update_to(boss, item, (status & 1) ? 1 : 0);
                    };
                }
                static void TerminalOutput(ui::item& boss, menu::item& item)
                {
                    _submit<true>(boss, item, [](auto& boss, auto& item, auto& /*gear*/)
                    {
                        boss.SIGNAL(tier::anycast, terminal::events::data::in, view{ item.views[item.taken].param });
                    });
                }
                static void TerminalSendKey(ui::item& boss, menu::item& item)
                {
                    _submit<true>(boss, item, [](auto& boss, auto& item, auto& /*gear*/)
                    {
                        boss.SIGNAL(tier::anycast, terminal::events::data::out, view{ item.views[item.taken].param });
                    });
                }
                static void TerminalQuit(ui::item& boss, menu::item& item)
                {
                    _submit<true>(boss, item, [](auto& boss, auto& /*item*/, auto& /*gear*/)
                    {
                        boss.SIGNAL(tier::anycast, terminal::events::cmd, ui::term::commands::ui::commands::sighup);
                    });
                }
                static void TerminalFullscreen(ui::item& boss, menu::item& item)
                {
                    _submit<true>(boss, item, [](auto& boss, auto& /*item*/, auto& gear)
                    {
                        boss.RISEUP(tier::preview, e2::form::size::enlarge::fullscreen, gear);
                    });
                }
                static void TerminalRestart(ui::item& boss, menu::item& item)
                {
                    _submit<true>(boss, item, [](auto& boss, auto& /*item*/, auto& /*gear*/)
                    {
                        boss.SIGNAL(tier::anycast, terminal::events::cmd, ui::term::commands::ui::commands::restart);
                    });
                }
                static void TerminalUndo(ui::item& boss, menu::item& item)
                {
                    _submit<true>(boss, item, [](auto& boss, auto& /*item*/, auto& /*gear*/)
                    {
                        boss.SIGNAL(tier::anycast, terminal::events::cmd, ui::term::commands::ui::commands::undo);
                    });
                }
                static void TerminalRedo(ui::item& boss, menu::item& item)
                {
                    _submit<true>(boss, item, [](auto& boss, auto& /*item*/, auto& /*gear*/)
                    {
                        boss.SIGNAL(tier::anycast, terminal::events::cmd, ui::term::commands::ui::commands::redo);
                    });
                }
                static void TerminalClipboardPaste(ui::item& boss, menu::item& item)
                {
                    _submit<true>(boss, item, [](auto& boss, auto& /*item*/, auto& gear)
                    {
                        boss.SIGNAL(tier::anycast, terminal::events::data::paste, gear);
                    });
                }
                static void TerminalClipboardWipe(ui::item& boss, menu::item& item)
                {
                    _submit<true>(boss, item, [](auto& /*boss*/, auto& /*item*/, auto& gear)
                    {
                        gear.clear_clipboard();
                    });
                }
                static void TerminalSelectionCopy(ui::item& boss, menu::item& item)
                {
                    _submit<true>(boss, item, [](auto& boss, auto& /*item*/, auto& gear)
                    {
                        boss.SIGNAL(tier::anycast, terminal::events::data::copy, gear);
                    });
                }
                static void TerminalSelectionMode(ui::item& boss, menu::item& item)
                {
                    item.reindex([](auto& utf8){ return netxs::get_or(xml::options::format, utf8, mime::disabled); });
                    _submit(boss, item, [](auto& boss, auto& item, auto& /*gear*/)
                    {
                        boss.SIGNAL(tier::anycast, preview::selection::mode, item.views[item.taken].value);
                    });
                    boss.LISTEN(tier::anycast, release::selection::mode, mode)
                    {
                        _update_to(boss, item, mode);
                    };
                }
                static void TerminalSelectionOneShot(ui::item& boss, menu::item& item)
                {
                    item.reindex([](auto& utf8){ return netxs::get_or(xml::options::format, utf8, mime::disabled); });
                    _submit(boss, item, [](auto& boss, auto& item, auto& /*gear*/)
                    {
                        boss.SIGNAL(tier::anycast, preview::selection::shot, item.views[item.taken].value);
                    });
                    boss.LISTEN(tier::anycast, release::selection::shot, mode)
                    {
                        _update_to(boss, item, mode);
                    };
                }
                static void TerminalSelectionRect(ui::item& boss, menu::item& item)
                {
                    item.reindex([](auto& utf8){ return xml::take<bool>(utf8).value(); });
                    _submit(boss, item, [](auto& boss, auto& item, auto& /*gear*/)
                    {
                        boss.SIGNAL(tier::anycast, preview::selection::box, item.views[item.taken].value);
                    });
                    boss.LISTEN(tier::anycast, release::selection::box, selbox)
                    {
                        _update_to(boss, item, selbox);
                    };
                }
                static void TerminalSelectionClear(ui::item& boss, menu::item& item)
                {
                    _submit<true>(boss, item, [](auto& boss, auto& /*item*/, auto& /*gear*/)
                    {
                        boss.SIGNAL(tier::anycast, terminal::events::cmd, ui::term::commands::ui::commands::deselect);
                    });
                }
                static void TerminalViewportCopy(ui::item& boss, menu::item& item)
                {
                    _submit<true>(boss, item, [](auto& boss, auto& /*item*/, auto& gear)
                    {
                        boss.SIGNAL(tier::anycast, terminal::events::data::prnscrn, gear);
                    });
                }
                static void TerminalViewportPageUp(ui::item& boss, menu::item& item)
                {
                    _submit<true>(boss, item, [](auto& boss, auto& /*item*/, auto& /*gear*/)
                    {
                        boss.SIGNAL(tier::anycast, e2::form::upon::scroll::bypage::y, info, ({ .vector = dot_01 }));
                    });
                }
                static void TerminalViewportPageDown(ui::item& boss, menu::item& item)
                {
                    _submit<true>(boss, item, [](auto& boss, auto& /*item*/, auto& /*gear*/)
                    {
                        boss.SIGNAL(tier::anycast, e2::form::upon::scroll::bypage::y, info, ({ .vector = -dot_01 }));
                    });
                }
                static void TerminalViewportLineUp(ui::item& boss, menu::item& item)
                {
                    item.reindex([](auto& utf8){ auto v = xml::take<si32>(utf8); return v ? v.value() : 1; });
                    _submit<true>(boss, item, [](auto& boss, auto& item, auto& /*gear*/)
                    {
                        boss.SIGNAL(tier::anycast, e2::form::upon::scroll::bystep::y, info, ({ .vector = { 0, std::abs(item.views[item.taken].value) }}));
                    });
                }
                static void TerminalViewportLineDown(ui::item& boss, menu::item& item)
                {
                    item.reindex([](auto& utf8){ auto v = xml::take<si32>(utf8); return v ? v.value() : 1; });
                    _submit<true>(boss, item, [](auto& boss, auto& item, auto& /*gear*/)
                    {
                        boss.SIGNAL(tier::anycast, e2::form::upon::scroll::bystep::y, info, ({ .vector = { 0, -std::abs(item.views[item.taken].value) }}));
                    });
                }
                static void TerminalViewportTop(ui::item& boss, menu::item& item)
                {
                    _submit<true>(boss, item, [](auto& boss, auto& /*item*/, auto& /*gear*/)
                    {
                        boss.SIGNAL(tier::anycast, e2::form::upon::scroll::to_top::y, info, ());
                    });
                }
                static void TerminalViewportEnd(ui::item& boss, menu::item& item)
                {
                    _submit<true>(boss, item, [](auto& boss, auto& /*item*/, auto& /*gear*/)
                    {
                        boss.SIGNAL(tier::anycast, e2::form::upon::scroll::to_end::y, info, ());
                    });
                }
                static void TerminalViewportPageLeft(ui::item& boss, menu::item& item)
                {
                    _submit<true>(boss, item, [](auto& boss, auto& /*item*/, auto& /*gear*/)
                    {
                        boss.SIGNAL(tier::anycast, e2::form::upon::scroll::bypage::x, info, ({ .vector = dot_10 }));
                    });
                }
                static void TerminalViewportPageRight(ui::item& boss, menu::item& item)
                {
                    _submit<true>(boss, item, [](auto& boss, auto& /*item*/, auto& /*gear*/)
                    {
                        boss.SIGNAL(tier::anycast, e2::form::upon::scroll::bypage::x, info, ({ .vector = -dot_10 }));
                    });
                }
                static void TerminalViewportCharLeft(ui::item& boss, menu::item& item)
                {
                    item.reindex([](auto& utf8){ auto v = xml::take<si32>(utf8); return v ? v.value() : 1; });
                    _submit<true>(boss, item, [](auto& boss, auto& item, auto& /*gear*/)
                    {
                        boss.SIGNAL(tier::anycast, e2::form::upon::scroll::bystep::x, info, ({ .vector = { std::abs(item.views[item.taken].value), 0 }}));
                    });
                }
                static void TerminalViewportCharRight(ui::item& boss, menu::item& item)
                {
                    item.reindex([](auto& utf8){ auto v = xml::take<si32>(utf8); return v ? v.value() : 1; });
                    _submit<true>(boss, item, [](auto& boss, auto& item, auto& /*gear*/)
                    {
                        boss.SIGNAL(tier::anycast, e2::form::upon::scroll::bystep::x, info, ({ .vector = { -std::abs(item.views[item.taken].value), 0 } }));
                    });
                }
                static void TerminalStdioLog(ui::item& boss, menu::item& item)
                {
                    item.reindex([](auto& utf8){ return xml::take<bool>(utf8).value(); });
                    _submit<true>(boss, item, [](auto& boss, auto& item, auto& /*gear*/)
                    {
                        boss.SIGNAL(tier::anycast, preview::io_log, item.views[item.taken].value);
                    });
                    boss.LISTEN(tier::anycast, release::io_log, state)
                    {
                        _update_to(boss, item, state);
                    };
                }
                static void TerminalCwdSync(ui::item& boss, menu::item& item)
                {
                    item.reindex([](auto& utf8){ return xml::take<bool>(utf8).value(); });
                    _submit(boss, item, [](auto& boss, auto& item, auto& /*gear*/)
                    {
                        boss.SIGNAL(tier::anycast, preview::cwdsync, item.views[item.taken].value);
                    });
                    boss.LISTEN(tier::anycast, release::cwdsync, state)
                    {
                        _update_to(boss, item, state);
                    };
                }
                static void TerminalLogStart(ui::item& /*boss*/, menu::item& /*item*/)
                {

                }
                static void TerminalLogPause(ui::item& /*boss*/, menu::item& /*item*/)
                {

                }
                static void TerminalLogStop(ui::item& /*boss*/, menu::item& /*item*/)
                {

                }
                static void TerminalLogAbort(ui::item& /*boss*/, menu::item& /*item*/)
                {

                }
                static void TerminalLogRestart(ui::item& /*boss*/, menu::item& /*item*/)
                {

                }
                static void TerminalVideoRecStart(ui::item& /*boss*/, menu::item& /*item*/)
                {

                }
                static void TerminalVideoRecStop(ui::item& /*boss*/, menu::item& /*item*/)
                {

                }
                static void TerminalVideoRecPause(ui::item& /*boss*/, menu::item& /*item*/)
                {

                }
                static void TerminalVideoRecAbort(ui::item& /*boss*/, menu::item& /*item*/)
                {

                }
                static void TerminalVideoRecRestart(ui::item& /*boss*/, menu::item& /*item*/)
                {

                }
                static void TerminalVideoPlay(ui::item& /*boss*/, menu::item& /*item*/)
                {

                }
                static void TerminalVideoPause(ui::item& /*boss*/, menu::item& /*item*/)
                {

                }
                static void TerminalVideoStop(ui::item& /*boss*/, menu::item& /*item*/)
                {

                }
                static void TerminalVideoForward(ui::item& /*boss*/, menu::item& /*item*/)
                {

                }
                static void TerminalVideoBackward(ui::item& /*boss*/, menu::item& /*item*/)
                {

                }
                static void TerminalVideoHome(ui::item& /*boss*/, menu::item& /*item*/)
                {

                }
                static void TerminalVideoEnd(ui::item& /*boss*/, menu::item& /*item*/)
                {

                }
            };
            using submit_proc = std::function<void(ui::item&, menu::item&)>;
            static const auto proc_map = std::unordered_map<func, submit_proc>
            {
                #define X(_proc) { func::_proc, &disp::_proc },
                proc_list
                #undef X
            };
            #undef proc_list

            auto list = menu::list{};
            auto defs = menu::item::look{};
            for (auto data_ptr : menudata)
            {
                auto item = menu::item{};
                auto& data = *data_ptr;
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
                if (item.views.empty()) continue; // Menu item without label.
                auto setup = [route](ui::item& boss, menu::item& item)
                {
                    if (item.brand == menu::item::Option)
                    {
                        boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                        {
                            item.taken = (item.taken + 1) % item.views.size();
                        };
                    }
                    auto& initproc = proc_map.find(route)->second;
                    initproc(boss, item);
                };
                list.push_back({ item, setup });
            }
            return menu::create(config, list);
        }
    }

    auto ui_term_events = [](ui::term& boss, eccc& appcfg)
    {
        boss.LISTEN(tier::anycast, e2::form::proceed::quit::any, fast)
        {
            boss.SIGNAL(tier::preview, e2::form::proceed::quit::one, fast);
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
            auto deed = boss.bell::template protos<tier::anycast>();
                 if (deed == e2::form::prop::colors::bg.id) boss.SIGNAL(tier::anycast, terminal::events::preview::colors::bg, clr);
            else if (deed == e2::form::prop::colors::fg.id) boss.SIGNAL(tier::anycast, terminal::events::preview::colors::fg, clr);
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
        boss.LISTEN(tier::release, e2::form::upon::started, root, -, (appcfg))
        {
            if (root) // root is empty when d_n_d.
            {
                boss.start(appcfg);
            }
        };
        boss.LISTEN(tier::anycast, e2::form::upon::started, root)
        {
            boss.SIGNAL(tier::release, e2::form::upon::started, root);
        };
        boss.LISTEN(tier::anycast, terminal::events::search::forward, gear)
        {
            boss.search(gear, feed::fwd);
        };
        boss.LISTEN(tier::anycast, terminal::events::search::reverse, gear)
        {
            boss.search(gear, feed::rev);
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
            auto deed = boss.bell::template protos<tier::anycast>();
            boss.base::template raw_riseup<tier::request>(e2::form::upon::scroll::any.id, info);
            info.vector = i.vector;
            boss.base::template raw_riseup<tier::preview>(deed, info);
        };
    };
    auto build_teletype = [](eccc appcfg, xmls& config)
    {
        auto menu_white = skin::color(tone::menu_white);
        auto cB = menu_white;

        auto window = ui::cake::ctor()
            ->plugin<pro::focus>(pro::focus::mode::active)
            ->invoke([&](auto& boss)
            {
                closing_on_quit(boss);
            });
        window//->plugin<pro::track>()
            //->plugin<pro::acryl>()
            ->plugin<pro::cache>();
        config.cd("/config/term/color/default/");
        auto def_fcolor = config.take("fgc", argb{ whitelt });
        auto def_bcolor = config.take("bgc", argb{ blackdk });
        auto layers = window->attach(ui::cake::ctor())
                            ->colors(cB)
                            ->limits(dot_11);
        auto scroll = layers->attach(ui::rail::ctor())
                            ->limits({ 10,1 }); // mc crashes when window is too small
        if (appcfg.cmd.empty()) appcfg.cmd = os::env::shell();//todo revise + " -i";
        auto inst = scroll->attach(ui::term::ctor(config))
            ->plugin<pro::focus>(pro::focus::mode::focused)
            ->colors(def_fcolor, def_bcolor)
            ->invoke([&](auto& boss)
            {
                ui_term_events(boss, appcfg);
            });
        layers->attach(app::shared::scroll_bars(scroll));
        return window;
    };
    auto build_terminal = [](eccc appcfg, xmls& config)
    {
        auto menu_white = skin::color(tone::menu_white);
        auto cB = menu_white;

        config.cd("/config/term/");
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
                bground.size({ mx, my }, skin::color(tone::kb_focus));
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
        window->plugin<pro::focus>(pro::focus::mode::hub)
            ->plugin<pro::track>()
            ->plugin<pro::cache>()
            ->shader(gradient, e2::form::state::keybd::focus::count);

        auto object = window->attach(ui::fork::ctor(axis::Y));
        auto term_stat_area = object->attach(slot::_2, ui::fork::ctor(axis::Y))
            ->setpad(borders)
            ->invoke([&](auto& boss)
            {
                if (borders)
                boss.LISTEN(tier::release, e2::render::background::any, parent_canvas, -, (borders, cB)) // Shade left/right borders.
                {
                    auto full = parent_canvas.full();
                    parent_canvas.cage(full, borders, [&](cell& c){ c.fuse(cB); });
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
                        boss.RISEUP(tier::request, e2::form::state::keybd::enlist, gates, ());
                        if (gates.size())
                        if (auto gate_ptr = boss.bell::getref(gates.back()))
                        {
                            gate_ptr->SIGNAL(tier::release, e2::form::proceed::onbehalf, [&](auto& gear)
                            {
                                boss.RISEUP(tier::preview, e2::form::size::enlarge::fullscreen, gear);
                            });
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
                        boss.RISEUP(tier::preview, e2::form::layout::swarp, warp);
                    }
                };
                boss.LISTEN(tier::release, e2::area, new_area)
                {
                    boss.base::locked = faux; // Unlock resizing.
                };
            });

        if (appcfg.cmd.empty()) appcfg.cmd = os::env::shell();//todo revise + " -i";
        auto inst = scroll->attach(ui::term::ctor(config))
            ->plugin<pro::focus>(pro::focus::mode::focused)
            ->invoke([&](auto& boss)
            {
                auto cwd_commands = config.take(attr::cwdsync, ""s);
                auto cwd_sync_ptr = ptr::shared<bool>();
                auto cwd_path_ptr = ptr::shared<os::fs::path>();
                auto& cwd_sync = *cwd_sync_ptr;
                auto& cwd_path = *cwd_path_ptr;
                boss.LISTEN(tier::anycast, terminal::events::preview::cwdsync, state, -, (cwd_commands))
                {
                    if (cwd_sync != state)
                    {
                        cwd_sync = state;
                        boss.SIGNAL(tier::anycast, terminal::events::release::cwdsync, state);
                        if (cwd_sync)
                        {
                            auto cmd = cwd_commands;
                            utf::replace_all(cmd, "$P", ".");
                            boss.data_out(cmd); // Trigger command prompt reprint.
                        }
                    }
                };
                boss.LISTEN(tier::preview, e2::form::prop::cwd, path, -, (cwd_sync_ptr, cwd_path_ptr))
                {
                    if (cwd_sync)
                    {
                        boss.template expire<tier::preview>(true);
                        cwd_path = path;
                    }
                };
                if (cwd_commands.size())
                {
                    boss.LISTEN(tier::anycast, e2::form::prop::cwd, path, -, (cwd_commands))
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
        static constexpr auto drawfx = [](auto& boss, auto& canvas, auto handle, auto /*object_len*/, auto handle_len, auto region_len, auto wide)
        {
            static auto box1 = "▄"sv;
            static auto box2 = ' ';
            auto menu_white = skin::color(tone::menu_white);
            auto cB = menu_white;
            auto term_bgc = boss.base::color().bgc();
            if (handle_len != region_len) // Show only if it is oversized.
            {
                if (wide) // Draw full scrollbar on mouse hover.
                {
                    canvas.fill([&](cell& c){ c.txt(box2).link(boss.bell::id).xlight().bgc().mix(cB.bgc()); });
                    canvas.fill(handle, [&](cell& c){ c.bgc().xlight(2); });
                }
                else
                {
                    canvas.fill([&](cell& c){ c.txt(box1).fgc(c.bgc()).bgc(term_bgc).fgc().mix(cB.bgc()); });
                    canvas.fill(handle, [&](cell& c){ c.link(boss.bell::id).fgc().xlight(2); });
                }
            }
            else canvas.fill([&](cell& c){ c.txt(box1).fgc(c.bgc()).bgc(term_bgc).fgc().mix(cB.bgc()); });
        };
        auto hz = term_stat_area->attach(slot::_2, ui::gripfx<axis::X, drawfx>::ctor(scroll))
            ->limits({ -1,1 }, { -1,1 })
            ->invoke([&](auto& boss)
            {
                boss.color(boss.color().bgc(inst->color().bgc()));
                inst->LISTEN(tier::release, e2::form::prop::filler, brush, -)
                {
                    boss.color(boss.color().bgc(brush.bgc()));
                };
            });

        auto [slot1, cover, menu_data] = construct_menu(config);
        auto menu = object->attach(slot::_1, slot1)
            ->colors(cB)
            ->invoke([&](auto& boss)
            {
                boss.LISTEN(tier::release, e2::area, new_area, -, (menu_height))
                {
                    *menu_height = new_area.size.y;
                };
            });
        cover->invoke([&, &slot1 = slot1](auto& boss) //todo clang 15.0.0 still disallows capturing structured bindings (wait for clang 16.0.0)
        {
            auto bar = cell{ "▀"sv }.link(slot1->id);
            auto term_bgc_ptr = ptr::shared(inst->color().bgc());
            auto& term_bgc = *term_bgc_ptr;
            auto winsz = ptr::shared(dot_00);
            auto visible = ptr::shared(slot1->back() != boss.This());
            auto check_state = ptr::function([state = testy<bool>{ true }, winsz, visible](base& boss) mutable
            {
                if (state(*visible || winsz->y != 1))
                {
                    boss.RISEUP(tier::preview, e2::form::prop::ui::cache, state.last);
                }
            });
            boss.LISTEN(tier::release, e2::form::state::visible, menu_visible, -, (visible, check_state))
            {
                *visible = menu_visible;
                (*check_state)(boss);
            };
            boss.LISTEN(tier::anycast, e2::form::upon::resized, new_area, -, (winsz, check_state))
            {
                *winsz = new_area.size;
                (*check_state)(boss);
            };
            inst->LISTEN(tier::release, e2::form::prop::filler, clr, -, (term_bgc_ptr))
            {
                term_bgc = clr.bgc();
            };
            boss.LISTEN(tier::release, e2::render::any, parent_canvas, -, (bar, winsz, term_bgc_ptr, borders))
            {
                auto full = parent_canvas.full();
                if (winsz->y != 1 && borders)
                {
                    parent_canvas.cage(full, borders, [&](cell& c){ c.txt(whitespace).link(bar); });
                    full -= borders;
                }
                auto bgc = winsz->y != 1 ? term_bgc : 0;
                parent_canvas.fill(full, [&](cell& c){ c.fgc(c.bgc()).bgc(bgc).txt(bar).link(bar); });
            };
        });

        inst->attach_property(ui::term::events::colors::bg,      terminal::events::release::colors::bg)
            ->attach_property(ui::term::events::colors::fg,      terminal::events::release::colors::fg)
            ->attach_property(ui::term::events::selmod,          terminal::events::release::selection::mode)
            ->attach_property(ui::term::events::onesht,          terminal::events::release::selection::shot)
            ->attach_property(ui::term::events::selalt,          terminal::events::release::selection::box)
            ->attach_property(ui::term::events::io_log,          terminal::events::release::io_log)
            ->attach_property(ui::term::events::layout::wrapln,  terminal::events::release::wrapln)
            ->attach_property(ui::term::events::layout::align,   terminal::events::release::align)
            ->attach_property(ui::term::events::search::status,  terminal::events::search::status)
            ->invoke([&](auto& boss)
            {
                ui_term_events(boss, appcfg);
            });
        return window;
    };

    app::shared::initialize teletype_builder{ app::teletype::id, build_teletype };
    app::shared::initialize terminal_builder{ app::terminal::id, build_terminal };
}