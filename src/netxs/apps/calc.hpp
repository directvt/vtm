// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_APP_CALC_HPP
#define NETXS_APP_CALC_HPP

namespace netxs::events::userland
{
    struct calc
    {
        EVENTPACK( calc, netxs::events::userland::root::custom )
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

namespace netxs::app
{
    using namespace ::netxs::console;

    // calc: .
    struct calc
        //: public ui::form<calc>
    {
        using events = ::netxs::events::userland::calc;
        
        //using namespace netxs;
        using slot = ui::slot;
        using axis = ui::axis;
        using axes = ui::axes;
        using snap = ui::snap;
        using id_t = netxs::input::id_t;
        
        static auto get_text()
        {
            static text cellatix_rows;
            static text cellatix_cols;
            static text cellatix_text;

            if (cellatix_text.empty())
            {
                auto step = 0x030303;
                auto topclr = 0xffffff;

                auto corner = topclr - 0x1f1f1f;
                //text cellatix_text_head = ansi::bgc(0xffffff - 0x1f1f1f).fgc(0) + "    ";
                auto cellatix_text_head = ansi::bgc(corner).fgc(0);
                for (auto c = 'A'; c <= 'Z'; c++)
                {
                    auto clr = topclr - 0x0f0f0f;
                    cellatix_text_head.bgc(clr - step * 0 /* 0xf0f0f0 */).add(" ")
                                    .bgc(clr - step * 1 /* 0xededed */).add(" ")
                                    .bgc(clr - step * 2 /* 0xeaeaea */).add(c)
                                    .bgc(clr - step * 3 /* 0xe7e7e7 */).add(" ")
                                    .bgc(clr - step * 4 /* 0xe4e4e4 */).add(" ");
                }
                auto clr = topclr;
                auto cellatix_text_01 = ansi::bgc(clr - step * 0 /* 0xffffff */).add(" ")
                                            .bgc(clr - step * 1 /* 0xfcfcfc */).add(" ")
                                            .bgc(clr - step * 2 /* 0xf9f9f9 */).add(" ")
                                            .bgc(clr - step * 3 /* 0xf6f6f6 */).add(" ")
                                            .bgc(clr - step * 4 /* 0xf3f3f3 */).add(" ");
                auto cellatix_text_00 = ansi::bgc(clr - step * 4 /* 0xf3f3f3 */).add(" ")
                                            .bgc(clr - step * 3 /* 0xf6f6f6 */).add(" ")
                                            .bgc(clr - step * 2 /* 0xf9f9f9 */).add(" ")
                                            .bgc(clr - step * 1 /* 0xfcfcfc */).add(" ")
                                            .bgc(clr - step * 0 /* 0xffffff */).add(" ");
                cellatix_cols = ansi::nil().wrp(wrap::off)
                    + cellatix_text_head;
                cellatix_text = ansi::nil().wrp(wrap::off);
                cellatix_rows = ansi::nil().wrp(wrap::off).fgc(blackdk);
                auto base = topclr - 0x1f1f1f;// 0xe0e0e0;// 0xe4e4e4;
                auto c1 = ansi::bgc(base); //ansi::bgc(0xf0f0f0);
                auto c2 = ansi::bgc(base);
                for (int i = 1; i < 100; i++)
                {
                    auto label = utf::adjust(std::to_string(i), 3, " ", true) + " ";
                    if (!(i % 2))
                    {
                        auto c0 = base;
                        for (auto i = 0; i < label.length(); i++)
                        {
                            cellatix_rows += ansi::bgc(c0) + label[i];
                            c0 += step;
                        }
                        cellatix_rows += (i == 99 ? ""s : "\n"s);
                        cellatix_text += utf::repeat(cellatix_text_01, 26)
                                    + (i == 99 ? ""s : "\n"s);
                    }
                    else
                    {
                        auto c0 = base + step * (iota)label.length();
                        for (auto i = 0; i < label.length(); i++)
                        {
                            cellatix_rows += ansi::bgc(c0) + label[i];
                            c0 -= step;
                        }
                        cellatix_rows += (i == 99 ? ""s : "\n"s);
                        cellatix_text += utf::repeat(cellatix_text_00, 26)
                                    + (i == 99 ? ""s : "\n"s);
                    }
                }
            }
            return std::tuple{ cellatix_rows, cellatix_cols, cellatix_text };
        }

        static auto build(view)
        {
                    const static auto c7 = app::shared::c7;
                    const static auto c3 = app::shared::c3;

                    auto [cellatix_rows, cellatix_cols, cellatix_text] = get_text();

                    auto window = ui::cake::ctor();
                    window->template plugin<pro::focus>()
                          ->colors(whitelt, 0x601A5f00)
                          ->template plugin<pro::limit>(twod{ 10,7 },twod{ -1,-1 })
                          ->template plugin<pro::track>()
                          ->template plugin<pro::acryl>()
                          ->template plugin<pro::cache>()
                          ->invoke([&](auto& boss)
                          {
                              boss.keybd.accept(true);
                              boss.SUBMIT(tier::release, e2::form::upon::vtree::attached, parent)
                              {
                                    static iota i = 0; i++;
                                    auto title = ansi::jet(bias::right).add("Spreadsheet\n ~/Untitled ", i, ".ods");
                                    boss.base::template riseup<tier::preview>(e2::form::prop::header, title);
                              };
                          });
                    auto object = window->attach(ui::fork::ctor(axis::Y))
                                        ->colors(whitelt, 0);
                        auto menu = object->attach(slot::_1, app::shared::main_menu());
                        auto all_rail = object->attach(slot::_2, ui::rail::ctor());
                        auto all_stat = all_rail->attach(ui::fork::ctor(axis::Y))
                                                ->template plugin<pro::limit>(twod{ -1,-1 },twod{ 136,102 });
                            auto func_body_pad = all_stat->attach(slot::_1, ui::pads::ctor(dent{ 1,1 }));
                                auto func_body = func_body_pad->attach(ui::fork::ctor(axis::Y));
                                    auto func_line = func_body->attach(slot::_1, ui::fork::ctor());
                                        auto fx_sum = func_line->attach(slot::_1, ui::fork::ctor());
                                            auto fx = fx_sum->attach(slot::_1, ui::post::ctor())
                                                            ->template plugin<pro::fader>(c7, c3, 150ms)
                                                            ->template plugin<pro::limit>(twod{ 3,-1 }, twod{ 4,-1 })
                                                            ->upload(ansi::wrp(wrap::off).add(" Fx "));
                                        auto ellipsis = func_line->attach(slot::_2, ui::post::ctor())
                                                                 ->template plugin<pro::fader>(c7, c3, 150ms)
                                                                 ->template plugin<pro::limit>(twod{ -1,1 }, twod{ 3,-1 })
                                                                 ->upload(ansi::wrp(wrap::off).add(" … "));
                                    auto body_area = func_body->attach(slot::_2, ui::fork::ctor(axis::Y));
                                        auto corner_cols = body_area->attach(slot::_1, ui::fork::ctor());
                                            auto corner = corner_cols->attach(slot::_1, ui::post::ctor())
                                                                     ->template plugin<pro::limit>(twod{ 4,1 }, twod{ 4,1 })
                                                                     ->upload(ansi::bgc(0xffffff - 0x1f1f1f).fgc(0).add("    "));
                                        auto rows_body = body_area->attach(slot::_2, ui::fork::ctor());
                                            auto layers = rows_body->attach(slot::_2, ui::cake::ctor());
                                            auto scroll = layers->attach(ui::rail::ctor())
                                                                ->template plugin<pro::limit>(twod{ -1,1 }, twod{ -1,-1 })
                                                                ->config(true, true);
                                                auto grid = scroll->attach(ui::post::ctor())
                                                                  ->colors(0xFF000000, 0xFFffffff)
                                                                  ->template plugin<pro::cell_highlight>()
                                                                  ->upload(cellatix_text);
                                            auto sum = fx_sum->attach(slot::_2, ui::post::ctor())
                                                             ->colors(0, whitelt)
                                                             ->upload(ansi::bgc(whitelt).fgc(blacklt)
                                                               .add(" =SUM(").itc(true).fgc(reddk).add("select cells by dragging").itc(faux)
                                                               .fgc(blacklt).add(")"))
                                                             ->invoke([&](ui::post& boss)
                                                             {
                                                                 grid->SUBMIT(tier::release, e2::data::text, data)
                                                                 {
                                                                    boss.upload(ansi::bgc(whitelt).fgc(blacklt).add(data));
                                                                 };
                                                             });
                                            auto cols_area = corner_cols->attach(slot::_2, ui::rail::ctor(axes::ONLY_X, axes::ONLY_X))
                                                                        ->template follow<axis::X>(scroll);
                                                auto cols = cols_area->attach(ui::post::ctor())
                                                                     ->template plugin<pro::limit>(twod{ -1,1 }, twod{ -1,1 })
                                                                     ->upload(cellatix_cols); //todo grid  A  B  C ...
                                            auto rows_area = rows_body->attach(slot::_1, ui::rail::ctor(axes::ONLY_Y, axes::ONLY_Y))
                                                                      ->template follow<axis::Y>(scroll)
                                                                      ->template plugin<pro::limit>(twod{ 4,-1 }, twod{ 4,-1 });
                                                auto rows = rows_area->attach(ui::post::ctor())
                                                                     ->upload(cellatix_rows); //todo grid  1 \n 2 \n 3 \n ...
                            auto stat_area = all_stat->attach(slot::_2, ui::rail::ctor())
                                                     ->template plugin<pro::limit>(twod{ -1,1 }, twod{ -1,1 })
                                                     ->template moveby<axis::X>(-5);
                                auto sheet_plus = stat_area->attach(ui::fork::ctor());
                                    auto sheet = sheet_plus->attach(slot::_1, ui::post::ctor())
                                                           ->template plugin<pro::limit>(twod{ -1,-1 }, twod{ 13,-1 })
                                                           ->upload(ansi::wrp(wrap::off).add("     ")
                                                             .bgc(whitelt).fgc(blackdk).add(" Sheet1 "));
                                    auto plus_pad = sheet_plus->attach(slot::_2, ui::fork::ctor());
                                        auto plus = plus_pad->attach(slot::_1, ui::post::ctor())
                                                            ->template plugin<pro::fader>(c7, c3, 150ms)
                                                            ->template plugin<pro::limit>(twod{ 3,-1 }, twod{ 3,-1 })
                                                            ->upload(ansi::wrp(wrap::off).add(" + "));
                                        auto pad = plus_pad->attach(slot::_2, ui::mock::ctor())
                                                           ->template plugin<pro::limit>(twod{ 1,1 }, twod{ 1,1 });
                            layers->attach(app::shared::scroll_bars(scroll));
            return window;
        }
    };

    auto& calc_creator = app::shared::get_creator();
    auto& calc_d = calc_creator["Calc"] = calc::build;
}

#endif // NETXS_APP_CALC_HPP