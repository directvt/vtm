// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#include "../desktopio/application.hpp"

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

namespace netxs::ui
{
    // console: Template modules for the base class behavior extension.
    namespace pro
    {
        //todo PoC, unify, too hacky
        // pro: Cell Highlighter.
        class cell_highlight
            : public skill
        {
            struct sock
            {
                twod curpos; // sock: Current coor.
                bool inside; // sock: Is active.
                bool seized; // sock: Is seized.
                rect region; // sock: Selected region.

                sock()
                    : inside{ faux },
                      seized{ faux }
                { }
                operator bool () { return inside || seized || region.size; }
                auto grab(twod const& coord, bool resume)
                {
                    if (inside)
                    {
                        if (!(region.size && resume))
                        {
                            region.coor = coord;
                            region.size = dot_00;
                        }
                        seized = true;
                    }
                    return seized;
                }
                auto calc(base const& boss, twod const& coord)
                {
                    curpos = coord;
                    auto area = boss.size();
                    area.x += boss.oversz.r;
                    inside = area.inside(curpos);
                }
                auto drag(twod const& coord)
                {
                    if (seized)
                    {
                        region.size = coord - region.coor;
                    }
                    return seized;
                }
                void drop()
                {
                    seized = faux;
                }
            };
            using list = socks<sock>;
            using skill::boss,
                  skill::memo;

            list items;

        public:
            cell_highlight(base&&) = delete;
            cell_highlight(base& boss)
                : skill{ boss },
                  items{ boss }
            {
                boss.LISTEN(tier::release, e2::postrender, parent_canvas, memo)
                {
                    auto full = parent_canvas.full();
                    auto view = parent_canvas.view();
                    auto mark = cell{}.bgc(bluelt).bga(0x40);
                    auto fill = [&](cell& c) { c.fuse(mark); };
                    auto step = twod{ 5, 1 };
                    auto area = full;
                    area.size.x += boss.oversz.r;
                    items.foreach([&](sock& item)
                    {
                        if (item.region.size)
                        {
                            auto region = item.region.normalize();
                            auto pos1 = region.coor / step * step;
                            auto pos2 = (region.coor + region.size + step) / step * step;
                            auto pick = rect{ full.coor + pos1, pos2 - pos1 }.clip(area).clip(view);
                            parent_canvas.fill(pick, fill);
                        }
                        if (item.inside)
                        {
                            auto pos1 = item.curpos / step * step;
                            auto pick = rect{ full.coor + pos1, step }.clip(view);
                            parent_canvas.fill(pick, fill);
                        }
                    });
                };
                boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear, memo)
                {
                    auto& item = items.take(gear);
                    if (item.region.size)
                    {
                        if (gear.meta(hids::anyCtrl)) item.region.size = gear.coord - item.region.coor;
                        else                          item.region.size = dot_00;
                    }
                    recalc();
                };
                boss.LISTEN(tier::release, hids::events::mouse::button::dblclick::left, gear, memo)
                {
                    auto& item = items.take(gear);
                    auto area = boss.size();
                    area.x += boss.oversz.r;
                    item.region.coor = dot_00;
                    item.region.size = area;
                    recalc();
                    gear.dismiss();
                };
                boss.LISTEN(tier::general, hids::events::die, gear, memo)
                {
                    recalc();
                    boss.deface();
                };
                boss.LISTEN(tier::release, hids::events::notify::mouse::enter, gear, memo)
                {
                    items.add(gear);
                };
                boss.LISTEN(tier::release, hids::events::notify::mouse::leave, gear, memo)
                {
                    auto& item = items.take(gear);
                    if (item.region.size)
                    {
                        item.inside = faux;
                    }
                    else items.del(gear);
                    recalc();
                };
                engage<hids::buttons::left>();
            }
            void recalc()
            {
                auto data = text{};
                auto step = twod{ 5, 1 };
                auto size = boss.size();
                size.x += boss.oversz.r;
                items.foreach([&](sock& item)
                {
                    if (item.region.size)
                    {
                        auto region = item.region.normalize();
                        auto pos1 = region.coor / step;
                        auto pos2 = (region.coor + region.size) / step;
                        pos1 = std::clamp(pos1, dot_00, twod{ 25, 98 } );
                        pos2 = std::clamp(pos2, dot_00, twod{ 25, 98 } );
                        data += 'A'+ (char)pos1.x;
                        data += std::to_string(pos1.y + 1);
                        data += ':';
                        data += 'A' + (char)pos2.x;
                        data += std::to_string(pos2.y + 1);
                        data += ", ";
                    }
                });
                if (data.size())
                {
                    data.pop_back(); // pop", "
                    data.pop_back(); // pop", "
                    data = " =SUM(" + ansi::fgc(bluedk).add(data).fgc(blacklt).add(")");
                }
                else data = " =SUM(" + ansi::itc(true).fgc(reddk).add("select cells by dragging").itc(faux).fgc(blacklt).add(")");
                log("calc: DATA ", data, ansi::nil());
                boss.SIGNAL(tier::release, e2::data::utf8, data);
            }
            // pro::cell_highlight: Configuring the mouse button to operate.
            template<hids::buttons Button>
            void engage()
            {
                boss.SIGNAL(tier::release, e2::form::draggable::_<Button>, true);
                boss.LISTEN(tier::release, hids::events::mouse::move, gear, memo)
                {
                    items.take(gear).calc(boss, gear.coord);
                    boss.base::deface();
                };
                boss.LISTEN(tier::release, e2::form::drag::start::_<Button>, gear, memo)
                {
                    if (items.take(gear).grab(gear.coord, gear.meta(hids::anyCtrl)))
                    {
                        gear.dismiss();
                    }
                };
                boss.LISTEN(tier::release, e2::form::drag::pull::_<Button>, gear, memo)
                {
                    if (items.take(gear).drag(gear.coord))
                    {
                        recalc();
                        gear.dismiss();
                    }
                };
                boss.LISTEN(tier::release, e2::form::drag::cancel::_<Button>, gear, memo)
                {
                    items.take(gear).drop();
                    recalc();
                };
                boss.LISTEN(tier::release, e2::form::drag::stop::_<Button>, gear, memo)
                {
                    items.take(gear).drop();
                    recalc();
                };
            }
        };
    }
}

// calc: Spreadsheet calculator.
namespace netxs::app::calc
{
    static constexpr auto id = "calc";
    static constexpr auto desc = "Desktopio Spreadsheet (DEMO)";

    using events = ::netxs::events::userland::calc;

    namespace
    {
        auto get_text = []
        {
            static text cellatix_rows;
            static text cellatix_cols;
            static text cellatix_text;

            if (cellatix_text.empty())
            {
                auto step = 0x030303;
                auto topclr = 0xFFffffff;

                auto corner = topclr - 0x1f1f1f;
                //text cellatix_text_head = ansi::bgc(0xFFffffff - 0x1f1f1f).fgc(0) + "    ";
                auto cellatix_text_head = ansi::bgc(corner).fgc(0xFF000000);
                for (auto c = 'A'; c <= 'Z'; c++)
                {
                    auto clr = topclr - 0x0f0f0f;
                    cellatix_text_head.bgc(clr - step * 0 /* 0xFFf0f0f0 */).add(" ")
                                      .bgc(clr - step * 1 /* 0xFFededed */).add(" ")
                                      .bgc(clr - step * 2 /* 0xFFeaeaea */).add(c)
                                      .bgc(clr - step * 3 /* 0xFFe7e7e7 */).add(" ")
                                      .bgc(clr - step * 4 /* 0xFFe4e4e4 */).add(" ");
                }
                auto clr = topclr;
                auto cellatix_text_01 = ansi::bgc(clr - step * 0 /* 0xFFffffff */).add(" ")
                                             .bgc(clr - step * 1 /* 0xFFfcfcfc */).add(" ")
                                             .bgc(clr - step * 2 /* 0xFFf9f9f9 */).add(" ")
                                             .bgc(clr - step * 3 /* 0xFFf6f6f6 */).add(" ")
                                             .bgc(clr - step * 4 /* 0xFFf3f3f3 */).add(" ");
                auto cellatix_text_00 = ansi::bgc(clr - step * 4 /* 0xFFf3f3f3 */).add(" ")
                                             .bgc(clr - step * 3 /* 0xFFf6f6f6 */).add(" ")
                                             .bgc(clr - step * 2 /* 0xFFf9f9f9 */).add(" ")
                                             .bgc(clr - step * 1 /* 0xFFfcfcfc */).add(" ")
                                             .bgc(clr - step * 0 /* 0xFFffffff */).add(" ");
                cellatix_cols = ansi::nil().wrp(wrap::off)
                    + cellatix_text_head;
                cellatix_text = ansi::nil().wrp(wrap::off);
                cellatix_rows = ansi::nil().wrp(wrap::off).fgc(blackdk);
                auto base = topclr - 0x1f1f1f;// 0xe0e0e0;// 0xe4e4e4;
                auto c1 = ansi::bgc(base); //ansi::bgc(0xFFf0f0f0);
                auto c2 = ansi::bgc(base);
                for (auto i = 1; i < 100; i++)
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
                        auto c0 = base + step * (si32)label.length();
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
        };
        auto build = [](text cwd, text arg, xmls& config, text patch)
        {
            auto highlight_color = skin::globals().highlight;
            auto label_color     = skin::globals().label;
            auto c3 = highlight_color;
            auto x3 = cell{ c3 }.alpha(0x00);
            auto c7 = label_color;

            auto [cellatix_rows, cellatix_cols, cellatix_text] = get_text();

            auto window = ui::cake::ctor();
            window->plugin<pro::focus>()
                  ->colors(whitelt, 0x601A5f00)
                  ->plugin<pro::limit>(twod{ 10,7 },twod{ -1,-1 })
                  ->plugin<pro::track>()
                  ->plugin<pro::acryl>()
                  ->plugin<pro::cache>()
                  ->invoke([&](auto& boss)
                  {
                      boss.keybd.accept(true);
                      boss.LISTEN(tier::anycast, e2::form::quit, item)
                      {
                          boss.RISEUP(tier::release, e2::form::quit, item);
                      };
                      boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent)
                      {
                          static auto i = 0; i++;
                          auto title = ansi::jet(bias::right).add("Spreadsheet\n ~/Untitled ", i, ".ods");
                          boss.RISEUP(tier::preview, e2::form::prop::ui::header, title);
                      };
                  });
            auto fader = skin::globals().fader_time;
            auto object = window->attach(ui::fork::ctor(axis::Y))
                                ->colors(whitelt, 0);
                auto menu = object->attach(slot::_1, app::shared::menu::demo(config));
                auto all_rail = object->attach(slot::_2, ui::rail::ctor());
                auto all_stat = all_rail->attach(ui::fork::ctor(axis::Y))
                                        ->plugin<pro::limit>(twod{ -1,-1 },twod{ 136,102 });
                    auto func_body_pad = all_stat->attach(slot::_1, ui::pads::ctor(dent{ 1,1 }));
                        auto func_body = func_body_pad->attach(ui::fork::ctor(axis::Y));
                            auto func_line = func_body->attach(slot::_1, ui::fork::ctor());
                                auto fx_sum = func_line->attach(slot::_1, ui::fork::ctor());
                                    auto fx = fx_sum->attach(slot::_1, ui::post::ctor())
                                                    ->plugin<pro::fader>(c7, c3, fader)
                                                    ->plugin<pro::limit>(twod{ 3,-1 }, twod{ 4,-1 })
                                                    ->upload(ansi::wrp(wrap::off).add(" Fx "));
                                auto ellipsis = func_line->attach(slot::_2, ui::post::ctor())
                                                         ->plugin<pro::fader>(c7, c3, fader)
                                                         ->plugin<pro::limit>(twod{ -1,1 }, twod{ 3,-1 })
                                                         ->upload(ansi::wrp(wrap::off).add(" … "));
                            auto body_area = func_body->attach(slot::_2, ui::fork::ctor(axis::Y));
                                auto corner_cols = body_area->attach(slot::_1, ui::fork::ctor());
                                    auto corner = corner_cols->attach(slot::_1, ui::post::ctor())
                                                             ->plugin<pro::limit>(twod{ 4,1 }, twod{ 4,1 })
                                                             ->upload(ansi::bgc(0xFFffffff - 0x1f1f1f).fgc(0xFF000000).add("    "));
                                auto rows_body = body_area->attach(slot::_2, ui::fork::ctor());
                                    auto layers = rows_body->attach(slot::_2, ui::cake::ctor());
                                    auto scroll = layers->attach(ui::rail::ctor())
                                                        ->plugin<pro::limit>(twod{ -1,1 }, twod{ -1,-1 });
                                        auto grid = scroll->attach(ui::post::ctor())
                                                          ->colors(0xFF000000, 0xFFffffff)
                                                          ->plugin<pro::cell_highlight>()
                                                          ->upload(cellatix_text);
                                    auto sum = fx_sum->attach(slot::_2, ui::post::ctor())
                                                     ->colors(0, whitelt)
                                                     ->upload(ansi::bgc(whitelt).fgc(blacklt)
                                                     .add(" =SUM(").itc(true).fgc(reddk).add("select cells by dragging").itc(faux)
                                                     .fgc(blacklt).add(")"))
                                                     ->invoke([&](ui::post& boss)
                                                     {
                                                         grid->LISTEN(tier::release, e2::data::utf8, data)
                                                         {
                                                            boss.upload(ansi::bgc(whitelt).fgc(blacklt).add(data));
                                                         };
                                                     });
                                    auto cols_area = corner_cols->attach(slot::_2, ui::rail::ctor(axes::X_ONLY, axes::X_ONLY))
                                                                ->follow<axis::X>(scroll);
                                        auto cols = cols_area->attach(ui::post::ctor())
                                                             ->plugin<pro::limit>(twod{ -1,1 }, twod{ -1,1 })
                                                             ->upload(cellatix_cols); //todo grid  A  B  C ...
                                    auto rows_area = rows_body->attach(slot::_1, ui::rail::ctor(axes::Y_ONLY, axes::Y_ONLY))
                                                              ->follow<axis::Y>(scroll)
                                                              ->plugin<pro::limit>(twod{ 4,-1 }, twod{ 4,-1 });
                                        auto rows = rows_area->attach(ui::post::ctor())
                                                             ->upload(cellatix_rows); //todo grid  1 \n 2 \n 3 \n ...
                    auto stat_area = all_stat->attach(slot::_2, ui::rail::ctor())
                                             ->plugin<pro::limit>(twod{ -1,1 }, twod{ -1,1 });
                        auto sheet_plus = stat_area->attach(ui::fork::ctor());
                            auto sheet = sheet_plus->attach(slot::_1, ui::post::ctor())
                                                   ->plugin<pro::limit>(twod{ -1,-1 }, twod{ 13,-1 })
                                                   ->upload(ansi::wrp(wrap::off).add("     ")
                                                       .bgc(whitelt).fgc(blackdk).add(" Sheet1 "));
                            auto plus_pad = sheet_plus->attach(slot::_2, ui::fork::ctor());
                                auto plus = plus_pad->attach(slot::_1, ui::post::ctor())
                                                    ->plugin<pro::fader>(c7, c3, fader)
                                                    ->plugin<pro::limit>(twod{ 3,-1 }, twod{ 3,-1 })
                                                    ->upload(ansi::wrp(wrap::off).add(" + "));
                                auto pad = plus_pad->attach(slot::_2, ui::mock::ctor())
                                                   ->plugin<pro::limit>(twod{ 1,1 }, twod{ 1,1 });
                    layers->attach(app::shared::scroll_bars(scroll));
            return window;
        };
    }

    app::shared::initialize builder{ app::calc::id, build };
}