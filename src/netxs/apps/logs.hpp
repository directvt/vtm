// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_APP_LOGS_HPP
#define NETXS_APP_LOGS_HPP

#include "../abstract/queue.hpp"

namespace netxs::events::userland
{
    struct logs
    {
        EVENTPACK( logs, netxs::events::userland::root::custom )
        {
            GROUP_XS( ui, input::hids ),

            SUBSET_XS( ui )
            {
                EVENT_XS( create  , input::hids ),
                GROUP_XS( split   , input::hids ),

                SUBSET_XS( split )
                {
                    EVENT_XS( hz, input::hids ),
                };
            };
        };
    };
}

// logs: ANSI-parser debugging tool.
namespace netxs::app::logs
{
    using events = netxs::events::userland::logs;

    class post_logs
        : public ui::post
    {
        pro::caret caret{ *this }; // post_logs: Text caret controller.

        text label;
        subs token;
        bool itsme = faux;

        struct log_parser
            : public bell
        {
            std::thread           input;
            ansi::esc             yield;
            netxs::mt_queue<text> queue;
            bool                  alive = true;
            bool show_codepoints = faux;
            ~log_parser()
            {
                alive = faux;
                queue.push(text{});
                if (input.joinable())
                {
                    input.join();
                }
            }
            log_parser()
            {
                SUBMIT(tier::general, e2::debug::output, shadow)
                {
                    queue.push(text{ shadow });
                };
                input = std::thread{ [&]() { worker(); } };
            }
            void enable_codepoints(bool s)
            {
                show_codepoints = s;
                auto msg = ansi::bgc(s ? greendk : yellowdk).fgc(whitelt)
                    .add(" show codepoints: ", s ? "on":"off", "\n").nil();
                SIGNAL(tier::general, e2::debug::logs, msg);
                SIGNAL(tier::release, e2::command::custom, s ? 1 : 2);
            }
            void worker()
            {
                while (alive)
                {
                    auto utf8 = queue.pop();
                    bool not_procesed = true;
                    while (not_procesed && alive)
                    {
                        if (auto lock = netxs::events::try_sync())
                        {
                            not_procesed = faux;
                            auto shadow = view{ utf8 };
                            auto parsed = read(shadow);
                            SIGNAL(tier::release, e2::debug::parsed, parsed);
                        }
                        else std::this_thread::yield();
                    }
                }
            }
            page read(view shadow)
            {
                auto w = 0;
                auto max_col = 12;
                auto wc = 5;

                yield.clear();
                yield.wrp(wrap::off)
                    .bgc(ansi::yellowlt).add(utf::repeat(' ', max_col * (wc + 3))).eol().bgc()
                    .add("STDOUT: plain text, ", shadow.size(), " bytes")
                    .eol().bgc(ansi::whitedk).fgc(blackdk)
                    .add(utf::debase(shadow))
                    .fgc().bgc().eol().eol();
                if (show_codepoints)
                {
                    yield.wrp(wrap::off).add("STDOUT: codepoints").eol();
                    auto f = [&](auto cp, view utf8, iota wide)
                    {
                        yield.fgc(ansi::blackdk);
                        if (wide)
                        {
                            yield.bgc(ansi::whitedk).add(' ', utf8);
                            if (wide == 1) yield.add(' ');
                        }
                        else yield.bgc(ansi::redlt).add(" - ");

                        yield.bgc().fgc(ansi::greendk)
                            .add(utf::adjust(utf::to_hex<true>(cp, (cp <= 0xFF   ? 2 :
                                                                    cp <= 0xFFFF ? 4 : 5)), wc, ' '))
                            .fgc().bgc();
                        if (++w == max_col) { w = 0; yield.eol(); }
                    };
                    auto iter = utf::cpit(shadow);
                    while (iter)
                    {
                        auto cp = iter.take();
                        if (cp.correct) f(cp.cdpoint, view(iter.textptr, cp.utf8len)      , cp.ucwidth);
                        else            f(cp.cdpoint, utf::REPLACEMENT_CHARACTER_UTF8_VIEW, 1         );
                        iter.step();
                    }
                    yield.eol().wrp(deco::defwrp);
                }
                yield.eol();
                return page{ yield };
            }
        };

        void update()
        {
            ui::post::recalc();
            auto new_cp = flow::cp();

            //todo unify, its too hacky
            auto new_coor = twod{ 0, std::numeric_limits<iota>::min() };
            SIGNAL(tier::release, e2::coor::set, new_coor);
            auto new_size = post::get_size();
            SIGNAL(tier::release, e2::size::set, new_size);

            caret.coor(new_cp);
        }
        void clear()
        {
            topic.clear();
            topic += "cleared...\n";
            topic += label;
            update();
        }

    public:
        sptr<log_parser> worker = netxs::shared_singleton<log_parser>();

        post_logs()
        {
            caret.show();
            caret.coor(dot_01);

            #ifndef PROD
            topic.maxlen(400);
            #else
            topic.maxlen(10000);
            #endif

            label = ansi::bgc(whitelt).fgc(blackdk).add(
                " Note: Log is limited to ", topic.maxlen(), " lines (old lines will be auto-deleted) \n"
                " Note: Use right mouse double-click to clear log \n"
                " Note: Using logs causes performance degradation \n")
                .fgc().bgc();
            topic += label;

            SUBMIT(tier::release, hids::events::mouse::button::dblclick::right, gear)
            {
                clear();
                gear.dismiss();
            };
            broadcast->SUBMIT(tier::request, e2::command::custom, status)
            {
                switch (status)
                {
                    case 1:
                        status = worker->show_codepoints ? 1 : 2;
                        break;
                    default:
                        break;
                }
            };
            broadcast->SUBMIT(tier::preview, e2::command::custom, cmd_id)
            {
                switch (cmd_id)
                {
                    case 0:
                        clear();
                        break;
                    case 1:
                    case 2:
                        itsme = true;
                        worker->enable_codepoints(cmd_id == 1 ? true : faux);
                        itsme = faux;
                        break;
                    default:
                        break;
                }
            };
            broadcast->SIGNAL(tier::release, e2::command::custom, worker->show_codepoints ? 1 : 2);
            SUBMIT(tier::preview, e2::size::set, newsize)
            {
                caret.coor(flow::cp());
            };
            SUBMIT(tier::general, e2::debug::logs, utf8)
            {
                topic += utf8;
                update();
            };
            worker->SUBMIT_T(tier::release, e2::command::custom, token, status)
            {
                broadcast->SIGNAL(tier::release, e2::command::custom, status);
            };
            worker->SUBMIT_T(tier::release, e2::debug::parsed, token, parsed_page)
            {
                topic += parsed_page;
                update();
            };
        }
    };

    namespace
    {
        auto build = [](view v)
        {
            const static auto x3 = app::shared::x3;

            auto window = ui::cake::ctor();
            window->plugin<pro::focus>()
                  ->plugin<pro::track>()
                  ->plugin<pro::acryl>()
                  ->plugin<pro::cache>()
                  ->invoke([&](auto& boss)
                  {
                      boss.keybd.accept(true);
                  });
            auto object = window->attach(ui::fork::ctor(axis::Y))
                                ->colors(whitelt, app::shared::term_menu_bg);
                auto menu = object->attach(slot::_1, app::shared::custom_menu(true,
                    std::list{
                            std::pair<text, std::function<void(ui::pads&)>>{ "Codepoints",
                            [](ui::pads& boss)
                            {
                                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                                {
                                    iota status = 1;
                                    boss.base::broadcast->SIGNAL(tier::request, e2::command::custom, status);
                                    boss.base::broadcast->SIGNAL(tier::preview, e2::command::custom, status == 2 ? 1/*show*/ : 2/*hide*/);
                                    gear.dismiss(true);
                                };
                                boss.base::broadcast->SUBMIT(tier::release, e2::command::custom, status)
                                {
                                    //todo unify, get boss base colors, don't use x3
                                    boss.color(status == 1 ? 0xFF00ff00 : x3.fgc(), x3.bgc());
                                };
                            }},
                            std::pair<text, std::function<void(ui::pads&)>>{ "Clear",
                            [](ui::pads& boss)
                            {
                                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                                {
                                    boss.base::broadcast->SIGNAL(tier::preview, e2::command::custom, 0);
                                    gear.dismiss(true);
                                };
                            }},
                        }));
                auto layers = object->attach(slot::_2, ui::cake::ctor());
                    auto scroll = layers->attach(ui::rail::ctor());
                    #ifndef PROD
                    scroll->attach(ui::post::ctor())
                            ->colors(whitelt, blackdk)
                            ->upload(ansi::fgc(yellowlt).mgl(4).mgr(4).wrp(wrap::off)
                            + "\n\nLogs is not availabe in DEMO mode\n\n"
                            + ansi::nil().wrp(wrap::on)
                            + "Use the full version of vtm to run Logs.");
                    #else
                    scroll->attach(base::create<post_logs>())
                            ->colors(whitelt, blackdk);
                    #endif
                layers->attach(app::shared::scroll_bars(scroll));
            return window;
        };
    }

    app::shared::initialize builder{ "Logs", build };
}

#endif // NETXS_APP_LOGS_HPP