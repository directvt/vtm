// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#include "../desktopio/application.hpp"
#include "../desktopio/terminal.hpp"

namespace netxs::events::userland
{
    struct term
    {
        EVENTPACK( term, ui::e2::extra::slot3 )
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
                GROUP_XS( selection, si32 ),
                GROUP_XS( colors   , rgba ),

                SUBSET_XS( selection )
                {
                    EVENT_XS( mode, si32 ),
                    EVENT_XS( box , si32 ),
                };
                SUBSET_XS( colors )
                {
                    EVENT_XS( bg, rgba ),
                    EVENT_XS( fg, rgba ),
                };
            };
            SUBSET_XS( release )
            {
                EVENT_XS( align    , si32 ),
                EVENT_XS( wrapln   , si32 ),
                EVENT_XS( io_log   , bool ),
                GROUP_XS( selection, si32 ),
                GROUP_XS( colors   , rgba ),

                SUBSET_XS( selection )
                {
                    EVENT_XS( mode, si32 ),
                    EVENT_XS( box , si32 ),
                };
                SUBSET_XS( colors )
                {
                    EVENT_XS( bg, rgba ),
                    EVENT_XS( fg, rgba ),
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
// term: Terminal Emulator.
namespace netxs::app::term
{
    static constexpr auto id = "term";
    static constexpr auto desc = "Terminal Emulator";

    using events = netxs::events::userland::term;

    namespace
    {
        using namespace app::shared;
        static auto _update(ui::item& boss, menu::item& item)
        {
            auto& look = item.views[item.taken];
            boss.SIGNAL(tier::release, e2::data::utf8,              look.label);
            boss.SIGNAL(tier::preview, e2::form::prop::ui::tooltip, look.notes);
            boss.reflow();
        }
        static auto _update_gear(ui::item& boss, menu::item& item, hids& gear)
        {
            auto& look = item.views[item.taken];
            gear.set_tooltip(look.notes, true);
            _update(boss, item);
        }
        static auto _update_to(ui::item& boss, menu::item& item, si32 i)
        {
            item.select(i);
            _update(boss, item);
        }
        template<bool AutoUpdate = faux, class P>
        static auto _submit(ui::item& boss, menu::item& item, P proc)
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
                        tick.actify(0, skin::globals().repeat_delay, [&, proc](auto p)
                        {
                            proc(boss, item, gear);
                            tick.actify(1, skin::globals().repeat_rate, [&, proc](auto d)
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
                    gear.dismiss(true);
                };
            }
        };
    }

    const auto terminal_menu = [](xmls& config)
    {
        auto highlight_color = skin::color(tone::highlight);
        auto c3 = highlight_color;

        config.cd("/config/term/", "/config/defapp/");
        auto menudata = config.list("menu/item");

        using namespace app::shared;

        static auto brand_options = std::unordered_map<text, menu::item::type>
           {{ menu::type::Splitter, menu::item::Splitter },
            { menu::type::Command,  menu::item::Command  },
            { menu::type::Option,   menu::item::Option   },
            { menu::type::Repeat,   menu::item::Repeat   }};

        #define proc_list \
            X(Noop                      ) /* */ \
            X(TerminalQuit              ) /* */ \
            X(TerminalFullscreen        ) /* */ \
            X(TerminalRestart           ) /* */ \
            X(TerminalSendKey           ) /* */ \
            X(TerminalWrapMode          ) /* */ \
            X(TerminalAlignMode         ) /* */ \
            X(TerminalOutput            ) /* */ \
            X(TerminalFindNext          ) /* */ \
            X(TerminalFindPrev          ) /* */ \
            X(TerminalUndo              ) /* Undo/Redo for cooked read under win32 */ \
            X(TerminalRedo              ) /* */ \
            X(TerminalClipboardPaste    ) /* */ \
            X(TerminalClipboardWipe     ) /* */ \
            X(TerminalSelectionCopy     ) /* */ \
            X(TerminalSelectionMode     ) /* */ \
            X(TerminalSelectionRect     ) /* Linear/Rectangular */ \
            X(TerminalSelectionClear    ) /* */ \
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
            using preview = app::term::events::preview;
            using release = app::term::events::release;

            static void Noop(ui::item& boss, menu::item& item) { }
            static void TerminalWrapMode(ui::item& boss, menu::item& item)
            {
                item.reindex([](auto& utf8){ return xml::take<bool>(utf8).value() ? wrap::on : wrap::off; });
                _submit(boss, item, [](auto& boss, auto& item, auto& gear)
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
                _submit(boss, item, [](auto& boss, auto& item, auto& gear)
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
                _submit(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::search::reverse, gear);
                });
                boss.LISTEN(tier::anycast, app::term::events::search::status, status)
                {
                    _update_to(boss, item, (status & 2) ? 1 : 0);
                };
            }
            static void TerminalFindNext(ui::item& boss, menu::item& item)
            {
                item.reindex([](auto& utf8){ return xml::take<bool>(utf8).value(); });
                _submit(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::search::forward, gear);
                });
                boss.LISTEN(tier::anycast, app::term::events::search::status, status)
                {
                    _update_to(boss, item, (status & 1) ? 1 : 0);
                };
            }
            static void TerminalOutput(ui::item& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::data::in, view{ item.views[item.taken].param });
                });
            }
            static void TerminalSendKey(ui::item& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::data::out, view{ item.views[item.taken].param });
                });
            }
            static void TerminalQuit(ui::item& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::cmd, ui::term::commands::ui::commands::sighup);
                });
            }
            static void TerminalFullscreen(ui::item& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.RISEUP(tier::preview, e2::form::size::enlarge::fullscreen, gear);
                });
            }
            static void TerminalRestart(ui::item& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::cmd, ui::term::commands::ui::commands::restart);
                });
            }
            static void TerminalUndo(ui::item& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::cmd, ui::term::commands::ui::commands::undo);
                });
            }
            static void TerminalRedo(ui::item& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::cmd, ui::term::commands::ui::commands::redo);
                });
            }
            static void TerminalClipboardPaste(ui::item& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::data::paste, gear);
                });
            }
            static void TerminalClipboardWipe(ui::item& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    gear.clear_clipboard();
                });
            }
            static void TerminalSelectionCopy(ui::item& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::data::copy, gear);
                });
            }
            static void TerminalSelectionMode(ui::item& boss, menu::item& item)
            {
                item.reindex([](auto& utf8){ return netxs::get_or(xml::options::format, utf8, mime::disabled); });
                _submit(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, preview::selection::mode, item.views[item.taken].value);
                });
                boss.LISTEN(tier::anycast, release::selection::mode, mode)
                {
                    _update_to(boss, item, mode);
                };
            }
            static void TerminalSelectionRect(ui::item& boss, menu::item& item)
            {
                item.reindex([](auto& utf8){ return xml::take<bool>(utf8).value(); });
                _submit(boss, item, [](auto& boss, auto& item, auto& gear)
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
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::cmd, ui::term::commands::ui::commands::deselect);
                });
            }
            static void TerminalViewportCopy(ui::item& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::data::prnscrn, gear);
                });
            }
            static void TerminalViewportPageUp(ui::item& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, e2::form::upon::scroll::bypage::y, info, ({ .vector = dot_01 }));
                });
            }
            static void TerminalViewportPageDown(ui::item& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, e2::form::upon::scroll::bypage::y, info, ({ .vector = -dot_01 }));
                });
            }
            static void TerminalViewportLineUp(ui::item& boss, menu::item& item)
            {
                item.reindex([](auto& utf8){ auto v = xml::take<si32>(utf8); return v ? v.value() : 1; });
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, e2::form::upon::scroll::bystep::y, info, ({ .vector = { 0, std::abs(item.views[item.taken].value) }}));
                });
            }
            static void TerminalViewportLineDown(ui::item& boss, menu::item& item)
            {
                item.reindex([](auto& utf8){ auto v = xml::take<si32>(utf8); return v ? v.value() : 1; });
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, e2::form::upon::scroll::bystep::y, info, ({ .vector = { 0, -std::abs(item.views[item.taken].value) }}));
                });
            }
            static void TerminalViewportTop(ui::item& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, e2::form::upon::scroll::to_top::y, info, ());
                });
            }
            static void TerminalViewportEnd(ui::item& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, e2::form::upon::scroll::to_end::y, info, ());
                });
            }
            static void TerminalViewportPageLeft(ui::item& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, e2::form::upon::scroll::bypage::x, info, ({ .vector = dot_10 }));
                });
            }
            static void TerminalViewportPageRight(ui::item& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, e2::form::upon::scroll::bypage::x, info, ({ .vector = -dot_10 }));
                });
            }
            static void TerminalViewportCharLeft(ui::item& boss, menu::item& item)
            {
                item.reindex([](auto& utf8){ auto v = xml::take<si32>(utf8); return v ? v.value() : 1; });
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, e2::form::upon::scroll::bystep::x, info, ({ .vector = { std::abs(item.views[item.taken].value), 0 }}));
                });
            }
            static void TerminalViewportCharRight(ui::item& boss, menu::item& item)
            {
                item.reindex([](auto& utf8){ auto v = xml::take<si32>(utf8); return v ? v.value() : 1; });
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, e2::form::upon::scroll::bystep::x, info, ({ .vector = { -std::abs(item.views[item.taken].value), 0 } }));
                });
            }
            static void TerminalStdioLog(ui::item& boss, menu::item& item)
            {
                item.reindex([](auto& utf8){ return xml::take<bool>(utf8).value(); });
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, preview::io_log, item.views[item.taken].value);
                });
                boss.LISTEN(tier::anycast, release::io_log, state)
                {
                    _update_to(boss, item, state);
                };
            }
            static void TerminalLogStart(ui::item& boss, menu::item& item)
            {

            }
            static void TerminalLogPause(ui::item& boss, menu::item& item)
            {

            }
            static void TerminalLogStop(ui::item& boss, menu::item& item)
            {

            }
            static void TerminalLogAbort(ui::item& boss, menu::item& item)
            {

            }
            static void TerminalLogRestart(ui::item& boss, menu::item& item)
            {

            }
            static void TerminalVideoRecStart(ui::item& boss, menu::item& item)
            {

            }
            static void TerminalVideoRecStop(ui::item& boss, menu::item& item)
            {

            }
            static void TerminalVideoRecPause(ui::item& boss, menu::item& item)
            {

            }
            static void TerminalVideoRecAbort(ui::item& boss, menu::item& item)
            {

            }
            static void TerminalVideoRecRestart(ui::item& boss, menu::item& item)
            {

            }
            static void TerminalVideoPlay(ui::item& boss, menu::item& item)
            {

            }
            static void TerminalVideoPause(ui::item& boss, menu::item& item)
            {

            }
            static void TerminalVideoStop(ui::item& boss, menu::item& item)
            {

            }
            static void TerminalVideoForward(ui::item& boss, menu::item& item)
            {

            }
            static void TerminalVideoBackward(ui::item& boss, menu::item& item)
            {

            }
            static void TerminalVideoHome(ui::item& boss, menu::item& item)
            {

            }
            static void TerminalVideoEnd(ui::item& boss, menu::item& item)
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
    };

    namespace
    {
        auto build = [](text env, text cwd, text arg, xmls& config, text patch)
        {
            auto menu_white = skin::color(tone::menu_white);
            auto cB = menu_white;

            auto window = ui::cake::ctor();
            if (os::dtvt::active)
            {
                //todo revise focus
                window->plugin<pro::focus>()
                      ->plugin<pro::track>()
                      ->plugin<pro::acryl>()
                      ->plugin<pro::cache>();
            }
            else window->plugin<pro::focus>(pro::focus::mode::focusable);

            auto object = window->attach(ui::fork::ctor(axis::Y))
                                ->colors(cB);
            auto term_stat_area = object->attach(slot::_2, ui::fork::ctor(axis::Y));
            auto layers = term_stat_area->attach(slot::_1, ui::cake::ctor())
                                        ->limits(dot_11, { 400,200 });
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
                            if (auto gate_ptr = bell::getref(gates.back()))
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

            auto shell = os::env::shell() + " -i";
            auto cmd = arg.empty() ? shell : arg;
            auto inst = scroll->attach(ui::term::ctor(config))
                              ->plugin<pro::focus>(pro::focus::mode::focused);
            auto scroll_bars = layers->attach(ui::fork::ctor());
            auto vt = scroll_bars->attach(slot::_2, ui::grip<axis::Y>::ctor(scroll));
            static constexpr auto drawfx = [](auto& boss, auto& canvas, auto handle, auto object_len, auto handle_len, auto region_len, auto wide)
            {
                // Brightener isn't suitable for white backgrounds.
                auto brighter = skin::color(tone::selector);
                brighter.bga(std::min(0xFF, brighter.bga() * 3));
                auto brighter_bgc = brighter.bgc();
                auto boss_bgc = boss.base::color().bgc();

                if (handle_len != region_len) // Show only if it is oversized.
                {
                    if (wide) // Draw full scrollbar on mouse hover
                    {
                        static auto box = ' ';
                        canvas.fill([&](cell& c) { c.txt(box).link(boss.bell::id).xlight(); });
                        canvas.fill(handle, [&](cell& c) { c.bgc().mix(brighter_bgc); });
                    }
                    else
                    {
                        static auto box = "▄"sv; //"▀"sv;
                        canvas.fill(handle, [&](cell& c) { c.link(boss.bell::id).bgc().mix(brighter_bgc); });
                        canvas.fill([&](cell& c) { c.inv(true).txt(box).fgc(boss_bgc); });
                    }
                }
                else
                {
                    static auto box = "▄"sv;
                    canvas.fill([&](cell& c) { c.inv(true).txt(box).fgc(boss_bgc); });
                }
            };
            auto hz = term_stat_area->attach(slot::_2, ui::gripfx<axis::X, drawfx>::ctor(scroll))
                ->limits({ -1,1 }, { -1,1 })
                ->invoke([&](auto& boss)
                {
                    boss.LISTEN(tier::anycast, app::term::events::release::colors::bg, bg)
                    {
                        boss.color(boss.color().bgc(bg).txt(""));
                    };
                });

            auto [slot1, cover, menu_data] = terminal_menu(config);
            auto menu = object->attach(slot::_1, slot1);
            cover->invoke([&, &slot1 = slot1](auto& boss) //todo clang 15.0.0 still disallows capturing structured bindings (wait for clang 16.0.0)
            {
                auto bar = cell{ "▀"sv }.link(slot1->id);
                auto brush = ptr::shared(cell{ cB }.inv(true).txt("▀"sv).link(slot1->id));
                auto winsz = ptr::shared(dot_00);
                auto visible = ptr::shared(slot1->back() != boss.This());
                auto check_state = ptr::function([state = testy<bool>{ true }, winsz, visible](base& boss) mutable
                {
                    if (state(*visible || winsz->y != 1))
                    {
                        boss.RISEUP(tier::preview, e2::form::prop::ui::acryl, state.last);
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
                boss.LISTEN(tier::anycast, app::term::events::release::colors::bg, bg, -, (brush))
                {
                    brush->fgc(bg);
                };
                boss.LISTEN(tier::release, e2::render::any, parent_canvas, -, (bar, winsz, brush))
                {
                    if (winsz->y != 1)
                    {
                        auto& b = *brush;
                        parent_canvas.fill([&](cell& c) { c.fusefull(b); });
                    }
                    else
                    {
                        parent_canvas.fill([&](cell& c) { c.fgc(c.bgc()).fga(0xff).bgc(0).txt(bar).link(bar); });
                    }
                };
            });

            inst->attach_property(ui::term::events::colors::bg,      app::term::events::release::colors::bg)
                ->attach_property(ui::term::events::colors::fg,      app::term::events::release::colors::fg)
                ->attach_property(ui::term::events::selmod,          app::term::events::release::selection::mode)
                ->attach_property(ui::term::events::selalt,          app::term::events::release::selection::box)
                ->attach_property(ui::term::events::io_log,          app::term::events::release::io_log)
                ->attach_property(ui::term::events::layout::wrapln,  app::term::events::release::wrapln)
                ->attach_property(ui::term::events::layout::align,   app::term::events::release::align)
                ->attach_property(ui::term::events::search::status,  app::term::events::search::status)
                ->invoke([&](auto& boss)
                {
                    boss.LISTEN(tier::anycast, e2::form::proceed::quit::any, fast)
                    {
                        boss.SIGNAL(tier::preview, e2::form::proceed::quit::one, fast);
                    };
                    boss.LISTEN(tier::preview, e2::form::proceed::quit::one, fast)
                    {
                        boss.close(fast);
                    };
                    boss.LISTEN(tier::anycast, app::term::events::cmd, cmd)
                    {
                        boss.exec_cmd(static_cast<ui::term::commands::ui::commands>(cmd));
                    };
                    boss.LISTEN(tier::anycast, app::term::events::data::in, data)
                    {
                        boss.data_in(data);
                    };
                    boss.LISTEN(tier::anycast, app::term::events::data::out, data)
                    {
                        boss.data_out(data);
                    };
                    //todo add color picker to the menu
                    boss.LISTEN(tier::anycast, app::term::events::preview::colors::bg, bg)
                    {
                        boss.set_bg_color(bg);
                    };
                    boss.LISTEN(tier::anycast, app::term::events::preview::colors::fg, fg)
                    {
                        boss.set_fg_color(fg);
                    };
                    boss.LISTEN(tier::anycast, e2::form::prop::colors::any, clr)
                    {
                        auto deed = boss.bell::template protos<tier::anycast>();
                             if (deed == e2::form::prop::colors::bg.id) boss.SIGNAL(tier::anycast, app::term::events::preview::colors::bg, clr);
                        else if (deed == e2::form::prop::colors::fg.id) boss.SIGNAL(tier::anycast, app::term::events::preview::colors::fg, clr);
                    };
                    boss.LISTEN(tier::anycast, app::term::events::preview::selection::mode, selmod)
                    {
                        boss.set_selmod(selmod);
                    };
                    boss.LISTEN(tier::anycast, app::term::events::preview::selection::box, selbox)
                    {
                        boss.set_selalt(selbox);
                    };
                    boss.LISTEN(tier::anycast, app::term::events::preview::wrapln, wrapln)
                    {
                        boss.set_wrapln(wrapln);
                    };
                    boss.LISTEN(tier::anycast, app::term::events::preview::io_log, state)
                    {
                        boss.set_log(state);
                    };
                    boss.LISTEN(tier::anycast, app::term::events::preview::align, align)
                    {
                        boss.set_align(align);
                    };
                    boss.LISTEN(tier::release, e2::form::upon::started, root, -, (cmd, cwd, env))
                    {
                        boss.start(cmd, cwd, env);
                    };
                    boss.LISTEN(tier::anycast, e2::form::upon::started, root)
                    {
                        boss.SIGNAL(tier::release, e2::form::upon::started, root);
                    };
                    boss.LISTEN(tier::anycast, app::term::events::search::forward, gear)
                    {
                        boss.search(gear, feed::fwd);
                    };
                    boss.LISTEN(tier::anycast, app::term::events::search::reverse, gear)
                    {
                        boss.search(gear, feed::rev);
                    };
                    boss.LISTEN(tier::anycast, app::term::events::data::paste, gear)
                    {
                        boss.paste(gear);
                    };
                    boss.LISTEN(tier::anycast, app::term::events::data::copy, gear)
                    {
                        boss.copy(gear);
                    };
                    boss.LISTEN(tier::anycast, app::term::events::data::prnscrn, gear)
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
                });
            return window;
        };
    }

    app::shared::initialize builder{ app::term::id, build };
}