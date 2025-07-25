// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

namespace netxs::events::userland
{
    namespace calc
    {
        EVENTPACK( app::calc::events, netxs::events::userland::seed::custom )
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
    }
}

namespace netxs::ui
{
    namespace pro
    {
        // pro: Cell Highlighter.
        class cell_highlight
        {
            struct actor
            {
                twod curpos{}; // actor: Current coor.
                bool inside{}; // actor: Is active.
                bool seized{}; // actor: Is seized.
                rect region{}; // actor: Selected region.

                auto grab(twod coord, bool resume)
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
                auto calc(base const& owner, twod coord)
                {
                    curpos = coord;
                    auto area = owner.base::size();
                    area.x += owner.base::oversz.r;
                    inside = area.inside(curpos);
                }
                auto drag(twod coord)
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
            using umap = std::unordered_map<id_t, actor>;

            base& boss;
            subs  memo;
            umap  gears;

            auto& take(hids& gear)
            {
                auto iter = gears.find(gear.id);
                if (iter == gears.end())
                {
                    iter = gears.emplace(gear.id, actor{}).first;
                }
                return iter->second;
            }
            void recalc()
            {
                auto data = text{};
                auto step = twod{ 5, 1 };
                auto size = boss.base::size();
                size.x += boss.base::oversz.r;
                for (auto& [id, g] : gears)
                {
                    if (g.region.size)
                    {
                        auto region = g.region.normalize();
                        auto pos1 = region.coor / step;
                        auto pos2 = (region.coor + region.size) / step;
                        pos1 = std::clamp(pos1, dot_00, twod{ 25, 98 } );
                        pos2 = std::clamp(pos2, dot_00, twod{ 25, 98 } );
                        data += 'A' + (char)pos1.x;
                        data += std::to_string(pos1.y + 1);
                        data += ':';
                        data += 'A' + (char)pos2.x;
                        data += std::to_string(pos2.y + 1);
                        data += ", ";
                    }
                }
                if (data.size())
                {
                    data.pop_back(); // pop", "
                    data.pop_back(); // pop", "
                    data = " =SUM(" + ansi::fgc(bluedk).add(data).fgc(blacklt).add(")");
                }
                else data = " =SUM(" + ansi::itc(true).fgc(reddk).add("select cells by dragging").itc(faux).fgc(blacklt).add(")");
                log(prompt::calc, "DATA ", data, ansi::nil());
                boss.base::signal(tier::release, e2::data::utf8, data);
            }

        public:
            cell_highlight(base&&) = delete;
            cell_highlight(base& boss)
                : boss{ boss }
            {
                boss.on(tier::mouserelease, input::key::MouseMove, memo, [&](hids& gear)
                {
                    take(gear).calc(boss, gear.coord);
                    boss.base::deface();
                });
                boss.on(tier::mouserelease, input::key::LeftClick, memo, [&](hids& gear)
                {
                    auto& g = take(gear);
                    if (g.region.size)
                    {
                        if (gear.meta(hids::anyCtrl)) g.region.size = gear.coord - g.region.coor;
                        else                          g.region.size = dot_00;
                    }
                    recalc();
                });
                boss.on(tier::mouserelease, input::key::LeftDoubleClick, memo, [&](hids& gear)
                {
                    auto& g = take(gear);
                    auto area = boss.base::size();
                    area.x += boss.base::oversz.r;
                    g.region.coor = dot_00;
                    g.region.size = area;
                    recalc();
                    gear.dismiss();
                });
                boss.on(tier::mouserelease, input::key::MouseEnter, memo, [&](hids& gear)
                {
                    take(gear);
                });
                boss.on(tier::mouserelease, input::key::MouseLeave, memo, [&](hids& gear)
                {
                    auto& g = take(gear);
                    if (g.region.size)
                    {
                        g.inside = faux;
                    }
                    else gears.erase(gear.id);
                    recalc();
                });
                boss.LISTEN(tier::general, input::events::die, gear, memo)
                {
                    gears.erase(gear.id);
                    recalc();
                    boss.base::deface();
                };
                boss.LISTEN(tier::release, e2::form::drag::start::left, gear, memo)
                {
                    auto& g = take(gear);
                    g.calc(boss, gear.click);
                    if (g.grab(gear.click, gear.meta(hids::anyCtrl)))
                    {
                        gear.dismiss();
                    }
                };
                boss.LISTEN(tier::release, e2::form::drag::pull::left, gear, memo)
                {
                    if (take(gear).drag(gear.coord))
                    {
                        recalc();
                        gear.dismiss();
                    }
                };
                boss.LISTEN(tier::release, e2::form::drag::cancel::left, gear, memo)
                {
                    take(gear).drop();
                    recalc();
                };
                boss.LISTEN(tier::release, e2::form::drag::stop::left, gear, memo)
                {
                    take(gear).drop();
                    recalc();
                };
                boss.LISTEN(tier::release, e2::postrender, parent_canvas, memo)
                {
                    auto full = parent_canvas.full();
                    auto clip = parent_canvas.clip();
                    auto mark = cell{}.bgc(bluelt).bga(0x40);
                    auto fill = [&](cell& c){ c.fuse(mark); };
                    auto step = twod{ 5, 1 };
                    auto area = full;
                    area.size.x += boss.base::oversz.r;
                    for (auto& [id, g] : gears)
                    {
                        if (g.region.size)
                        {
                            auto region = g.region.normalize();
                            auto pos1 = region.coor / step * step;
                            auto pos2 = (region.coor + region.size + step) / step * step;
                            auto pick = rect{ full.coor + pos1, pos2 - pos1 }.trimby(area).trimby(clip);
                            parent_canvas.fill(pick, fill);
                        }
                        if (g.inside)
                        {
                            auto pos1 = g.curpos / step * step;
                            auto pick = rect{ full.coor + pos1, step }.trimby(clip);
                            parent_canvas.fill(pick, fill);
                        }
                    }
                };
                auto& mouse = boss.base::plugin<pro::mouse>();
                mouse.draggable<hids::buttons::left>(true);
            }
        };
    }
}

// calc: Spreadsheet calculator.
namespace netxs::app::calc
{
    static constexpr auto id = "calc";
    static constexpr auto name = "Spreadsheet calculator (DEMO)";

    namespace events = ::netxs::events::userland::calc;

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
                        for (auto j = 0_sz; j < label.length(); j++)
                        {
                            cellatix_rows += ansi::bgc(c0) + label[j];
                            c0 += step;
                        }
                        cellatix_rows += (i == 99 ? ""s : "\n"s);
                        cellatix_text += utf::repeat(cellatix_text_01, 26)
                                      + (i == 99 ? ""s : "\n"s);
                    }
                    else
                    {
                        auto c0 = base + step * (si32)label.length();
                        for (auto j = 0_sz; j < label.length(); j++)
                        {
                            cellatix_rows += ansi::bgc(c0) + label[j];
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
        auto build = [](eccc /*appcfg*/, settings& config)
        {
            auto highlight_color = cell{ skin::globals().winfocus };
            auto label_color     = cell{ whitespace }.fgc(blackdk).bgc(whitedk);
            auto c3 = highlight_color;
            //auto x3 = cell{ c3 }.alpha(0x00);
            auto c7 = label_color;

            auto [cellatix_rows, cellatix_cols, cellatix_text] = get_text();

            auto window = ui::cake::ctor();
            window->plugin<pro::focus>(pro::focus::mode::focused)
                  ->colors(whitelt, 0x60'00'5f'1A)
                  ->limits({ 10,7 }, { -1,-1 })
                  ->plugin<pro::keybd>()
                  ->shader(c3, e2::form::state::focus::count)
                  //->plugin<pro::acryl>()
                  ->plugin<pro::cache>()
                  ->invoke([&](auto& boss)
                  {
                      boss.LISTEN(tier::anycast, e2::form::proceed::quit::any, fast)
                      {
                          boss.base::riseup(tier::release, e2::form::proceed::quit::one, fast);
                      };
                      boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent)
                      {
                          static auto i = 0; i++;
                          auto title = ansi::jet(bias::right).add("Spreadsheet\n ~/Untitled ", i, ".ods");
                          boss.base::riseup(tier::preview, e2::form::prop::ui::header, title);
                      };
                  });
            auto object = window->attach(ui::fork::ctor(axis::Y))
                                ->colors(whitelt, 0);
            auto defapp_context = config.settings::push_context("/config/defapp/");
                auto menu = object->attach(slot::_1, app::shared::menu::demo(config));
                auto all_rail = object->attach(slot::_2, ui::rail::ctor());
                auto all_stat = all_rail->attach(ui::fork::ctor(axis::Y))
                                        ->limits({ -1,-1 },{ 136,102 });
                        auto func_body = all_stat->attach(slot::_1, ui::fork::ctor(axis::Y))
                            ->setpad({ 1,1 });
                            auto func_line = func_body->attach(slot::_1, ui::fork::ctor());
                                auto fx_sum = func_line->attach(slot::_1, ui::fork::ctor());
                                    auto fx = fx_sum->attach(slot::_1, ui::post::ctor())
                                                    ->active(c7)
                                                    ->shader(c3, e2::form::state::hover)
                                                    ->limits({ 3,-1 }, { 4,-1 })
                                                    ->upload(ansi::wrp(wrap::off).add(" Fx "));
                                auto ellipsis = func_line->attach(slot::_2, ui::post::ctor())
                                                         ->active(c7)
                                                         ->shader(c3, e2::form::state::hover)
                                                         ->limits({ -1,1 }, { 3,-1 })
                                                         ->upload(ansi::wrp(wrap::off).add(" … "));
                            auto body_area = func_body->attach(slot::_2, ui::fork::ctor(axis::Y));
                                auto corner_cols = body_area->attach(slot::_1, ui::fork::ctor());
                                    auto corner = corner_cols->attach(slot::_1, ui::post::ctor())
                                                             ->limits({ 4,1 }, { 4,1 })
                                                             ->upload(ansi::bgc(0xFFffffff - 0x1f1f1f).fgc(0xFF000000).add("    "));
                                auto rows_body = body_area->attach(slot::_2, ui::fork::ctor());
                                    auto layers = rows_body->attach(slot::_2, ui::cake::ctor());
                                    auto scroll = layers->attach(ui::rail::ctor())
                                                        ->active()
                                                        ->limits({ -1,1 }, { -1,-1 });
                                        auto sheet_body = scroll->attach(ui::post::ctor())
                                                                ->active(0xFF000000, 0xFFffffff)
                                                                ->plugin<pro::cell_highlight>()
                                                                ->upload(cellatix_text);
                                    auto sum = fx_sum->attach(slot::_2, ui::post::ctor())
                                                     ->colors(0, whitelt)
                                                     ->upload(ansi::bgc(whitelt).fgc(blacklt)
                                                     .add(" =SUM(").itc(true).fgc(reddk).add("select cells by dragging").itc(faux)
                                                     .fgc(blacklt).add(")"))
                                                     ->invoke([&](ui::post& boss)
                                                     {
                                                         sheet_body->LISTEN(tier::release, e2::data::utf8, data)
                                                         {
                                                            boss.upload(ansi::bgc(whitelt).fgc(blacklt).add(data));
                                                         };
                                                     });
                                    auto cols_area = corner_cols->attach(slot::_2, ui::rail::ctor(axes::X_only, axes::X_only))
                                                                ->follow<axis::X>(scroll);
                                        auto cols = cols_area->attach(ui::post::ctor())
                                                             ->limits({ -1,1 }, { -1,1 })
                                                             ->upload(cellatix_cols); //todo grid  A  B  C ...
                                    auto rows_area = rows_body->attach(slot::_1, ui::rail::ctor(axes::Y_only, axes::Y_only))
                                                              ->follow<axis::Y>(scroll)
                                                              ->limits({ 4,-1 }, { 4,-1 });
                                        auto rows = rows_area->attach(ui::post::ctor())
                                                             ->upload(cellatix_rows); //todo grid  1 \n 2 \n 3 \n ...
                    auto stat_area = all_stat->attach(slot::_2, ui::rail::ctor())
                                             ->limits({ -1,1 }, { -1,1 });
                        auto sheet_plus = stat_area->attach(ui::fork::ctor());
                            auto sheet = sheet_plus->attach(slot::_1, ui::post::ctor())
                                                   ->limits({ -1,-1 }, { 13,-1 })
                                                   ->upload(ansi::wrp(wrap::off).add("     ")
                                                       .bgc(whitelt).fgc(blackdk).add(" Sheet1 "));
                            auto plus_pad = sheet_plus->attach(slot::_2, ui::fork::ctor());
                                auto plus = plus_pad->attach(slot::_1, ui::post::ctor())
                                                    ->active(c7)
                                                    ->shader(c3, e2::form::state::hover)
                                                    ->limits({ 3,-1 }, { 3,-1 })
                                                    ->upload(ansi::wrp(wrap::off).add(" + "));
                                auto pad = plus_pad->attach(slot::_2, ui::mock::ctor())
                                                   ->limits({ 1,1 }, { 1,1 });
                    layers->attach(app::shared::scroll_bars(scroll));
            window->invoke([&](auto& boss)
            {
                app::shared::base_kb_navigation(config, scroll, boss);
            });
            return window;
        };
    }

    app::shared::initialize builder{ app::calc::id, build };
}