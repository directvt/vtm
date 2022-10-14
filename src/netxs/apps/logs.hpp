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
            GROUP_XS( codepoints, si32 ),

            SUBSET_XS( codepoints )
            {
                EVENT_XS( release, si32 ),
                EVENT_XS( preview, si32 ),
                EVENT_XS( request, si32 ),
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
                SUBMIT(tier::anycast, e2::debug::output, shadow)
                {
                    queue.push(text{ shadow });
                };
                input = std::thread{ [&]{ worker(); } };
            }
            void enable_codepoints(bool s)
            {
                show_codepoints = s;
                auto msg = ansi::bgc(s ? greendk : yellowdk).fgc(whitelt)
                    .add(" show codepoints: ", s ? "on":"off", "\n").nil();
                SIGNAL(tier::anycast, e2::debug::logs, msg);
                SIGNAL(tier::release, events::codepoints::release, s ? 1 : 2);
            }
            void worker()
            {
                while (alive)
                {
                    auto utf8 = queue.pop();
                    auto processed = faux;
                    while (!processed && alive)
                    {
                        if (auto lock = netxs::events::try_sync())
                        {
                            processed = true;
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
                    auto f = [&](auto cp, view utf8, si32 wide)
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

            //todo unify, it's too hacky -- use smth like a signal scroll::end
            //                              riseup<tier::preview>(e2::form::upon::scroll::to_end::y, scinfo);
            auto new_size = post::get_size();
            auto new_coor = twod{ 0, std::numeric_limits<si32>::min() };
            base::resize(new_size);
            base::moveto(new_coor);

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
        netxs::sptr<log_parser> worker = netxs::shared_singleton<log_parser>();

        post_logs()
        {
            caret.show();
            caret.coor(dot_01);
            keybd.accept(true);
            topic.maxlen(10000);

            label = ansi::bgc(whitelt).fgc(blackdk).add(
                " Note: Log is limited to ", topic.maxlen(), " lines (old lines will be auto-deleted) \n"
                " Note: Use right mouse double-click to clear log \n"
                " Note: Using logs causes performance degradation \n")
                .fgc().bgc();
            topic += label;

            SUBMIT(tier::anycast, events::codepoints::request, status)
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
            SUBMIT(tier::anycast, events::codepoints::preview, cmd_id)
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
            SUBMIT(tier::anycast, e2::form::upon::started, root)
            {
                this->SIGNAL(tier::anycast, events::codepoints::release, worker->show_codepoints ? 1 : 2);
                this->SIGNAL(tier::anycast, e2::debug::count::set, 1); // For Term.
                this->SIGNAL(tier::anycast, e2::debug::request, 1); // For gate.
            };
            SUBMIT(tier::preview, e2::size::set, newsize)
            {
                caret.coor(flow::cp());
            };
            SUBMIT(tier::anycast, e2::debug::logs, utf8)
            {
                topic += utf8;
                update();
            };
            SUBMIT(tier::release, hids::events::keybd::any, gear)
            {
                auto utf8 = gear.interpret();
                topic += utf8;
                update();
            };
            SUBMIT(tier::release, hids::events::mouse::button::click::right, gear)
            {
                auto data = gear.get_clip_data();
                if (data.utf8.size())
                {
                    topic += data.utf8;
                    update();
                    gear.dismiss();
                }
            };
            worker->SUBMIT_T(tier::release, events::codepoints::release, token, status)
            {
                this->SIGNAL(tier::anycast, events::codepoints::release, status);
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
        auto build = [](text cwd, text arg)
        {
            const static auto x3 = app::shared::x3;

            auto window = ui::cake::ctor();
            window->plugin<pro::focus>()
                  ->plugin<pro::track>()
                  ->plugin<pro::acryl>()
                  ->plugin<pro::cache>()
                  ->invoke([&](auto& boss)
                  {
                        boss.SUBMIT(tier::anycast, e2::form::quit, item)
                        {
                            boss.base::template riseup<tier::release>(e2::form::quit, item);
                        };
                  });
            auto object = window->attach(ui::fork::ctor(axis::Y))
                                ->colors(whitelt, app::shared::term_menu_bg);
                auto menu = object->attach(slot::_1, app::shared::custom_menu(true,
                    app::shared::menu_list_type{
                            //todo use it only in conjunction with the terminal
                            //{ true, "Codepoints", " Toggle button: Show or not codepoints ",
                            //[](ui::pads& boss)
                            //{
                            //    boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                            //    {
                            //        si32 status = 1;
                            //        boss.SIGNAL(tier::anycast, app::logs::events::codepoints::request, status);
                            //        boss.SIGNAL(tier::anycast, app::logs::events::codepoints::preview, status == 2 ? 1/*show*/ : 2/*hide*/);
                            //        gear.dismiss(true);
                            //    };
                            //    boss.SUBMIT(tier::anycast, app::logs::events::codepoints::release, status)
                            //    {
                            //        //todo unify, get boss base colors, don't use x3
                            //        boss.color(status == 1 ? 0xFF00ff00 : x3.fgc(), x3.bgc());
                            //    };
                            //}},
                            { true, "Clear", " Clear scrollback ",
                            [](ui::pads& boss)
                            {
                                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                                {
                                    boss.SIGNAL(tier::anycast, app::logs::events::codepoints::preview, 0);
                                    gear.dismiss(true);
                                };
                            }},
                        }));
                auto layers = object->attach(slot::_2, ui::cake::ctor());
                    auto scroll = layers->attach(ui::rail::ctor());
                    scroll->attach(base::create<post_logs>())
                          ->colors(whitelt, blackdk);
                layers->attach(app::shared::scroll_bars(scroll));
            return window;
        };
    }

    app::shared::initialize builder{ "logs", build };
}

#endif // NETXS_APP_LOGS_HPP