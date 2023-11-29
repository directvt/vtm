// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

namespace netxs::events::userland
{
    struct shop
    {
        EVENTPACK( shop, netxs::events::userland::root::custom )
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

// shop: App manager.
namespace netxs::app::shop
{
    static constexpr auto id = "gems";
    static constexpr auto desc = "Application Distribution Hub (DEMO)";

    using events = netxs::events::userland::shop;

    namespace
    {
        auto get_text = []
        {
            static text desktopio_body;
            static text appstore_head;
            static std::list<text> appstore_body;

            if (appstore_head.empty())
            {
                auto textancy_logo = ansi::bgc(cyandk   ).add("▀▄");
                auto cellatix_logo = ansi::bgc(greendk  ).add("▀▄");
                auto informio_logo = ansi::bgc(magentadk).add("▀▄");
                auto ansiplex_logo = ansi::bgc(reddk    ).add("▀▄");
                auto unicodex_logo = ansi::bgc(yellowdk ).add("▀▄");
                auto appstore_logo = ansi::bgc(blacklt  ).add("▀▄");

                auto line = ansi::wrp(wrap::off).add("──────────────────────────────────────────────────────────────────────────────────────").wrp(wrap::on).eol();
                auto item = [](auto app, auto clr, auto rating, auto price, auto buy, auto desc)
                {
                    auto clr_light = rgba{ clr };
                    clr_light.mix(0xa7ffffff);

                    auto lot = ansi::nil()
                        .jet(bias::left)
                        .mgl(2).mgr(1).eol()
                        .fgc().jet(bias::left).wrp(wrap::off)
                        .bgc(clr).fgc(0xFFffffff).add("▀▄ ")
                        .fgc(0xFFffffff).add(app, ' ').nil().eol()
                        .fgc(yellowlt).add("   ★★★★").fgc(cyandk).add("★  ").fgc(yellowlt).add(rating)

                        .chx(0).jet(bias::right)
                        .fgc(yellowlt).bgc().add("   ", price, "  ")
                        .nil().eol().eol()
                        .fgc(bluedk).bgc(whitelt).add(' ', buy, ' ').nil().add("  ")

                        .fgc().chx(0).jet(bias::left)
                        .mgr(11).wrp(wrap::on)
                        .add(desc, "\n\n")
                        .nil();

                    return lot;
                };

                appstore_head =
                ansi::nil().eol().mgl(2).mgr(2)
                .bld(true).fgc(whitelt).jet(bias::left).wrp(wrap::on)
                .add("Application Distribution Hub").bld(faux).add("\n\n");

                auto textancy_text = ansi::nil().add(
                "Hello World!😎\n"
                "絵文字:\n"
                "English: /ɪˈmoʊdʒiː/;\n"
                "Japanese: [emodʑi];\n");

                appstore_body =
                {
                    item("Term", blackdk, "469", "Free ", "Get",
                    "Virtual Terminal."),

                    item("Tile", bluedk, "3", "Free ", "Get",
                    "Tiling window manager."),

                    item("Text", cyandk, "102", "Free ", "Get",
                    "Text editor for text-based desktop environment. "
                    "Basic editing tool which allows "
                    "desktop users to create documents containing rich text."),

                    item("Calc", greendk, "30", "Free ", "Get",
                    "Spreadsheet calculator."),

                    item("Task", magentadk, "311", "Free ", "Get",
                    "Task manager that displays "
                    "information about CPU, memory utilization, "
                    "and current I/O usage."),

                    item("Draw", reddk, "64", "Free ", "Get",
                    "ANSI-artwork Studio."),

                    item("Char", yellowdk, "161", "Free ", "Get",
                    "Unicode codepoints browser."),

                    item(ansi::fgc(0xFFff0000).add("File"), cyanlt, "4", "Free ", "Get",
                    "File manager."),

                    item("Time", bluedk, "4", "Free ", "Get",
                    "Calendar."),

                    item("Goto", bluedk, "4", "Free ", "Get",
                    "Internet/SSH browser."),

                    item(ansi::fgc(0xFF00FFFF).add("Doom").fgc(), reddk, "4", "Free ", "Get",
                    "Doom II source port."),

                    item("Clip", bluedk, "1", "Free ", "Get",
                    "Clipboard manager."),

                    item("Info", cyandk, "1", "Free ", "Get",
                    "Software documentation browser."),

                    item("Hood", reddk, "1", "Free ", "Get",
                    "Workspace settings configurator."),

                    item("View", cyandk, "1", "Free ", "Get",
                    "Workspace location marker."),
                };

                auto qr = escx(
                "\033[107m                                 \n"
                "  \033[40m \033[97m▄▄▄▄▄ \033[107m \033[30m▄\033[40;97m▄\033[107m \033[30m▄\033[40m \033[107m  \033[40m \033[97m▄\033[107;30m▄\033[40;97m▄▄\033[107m  \033[40m ▄▄▄▄▄ \033[107m  \n"
                "  \033[40m \033[107m \033[40m   \033[107m \033[40m \033[107m \033[40m▄   ▄\033[107m \033[40m \033[107;30m▄ \033[40m \033[107m▄\033[40;97m▄\033[107m  \033[40m \033[107m \033[40m   \033[107m \033[40m \033[107m  \n"
                "  \033[40m \033[107m \033[40m▄▄▄\033[107m \033[40m \033[107m \033[40m \033[107;30m▄ ▄ \033[40m \033[97m▄  \033[107m \033[40m \033[107;30m▄  \033[40m \033[107m \033[40;97m▄▄▄\033[107m \033[40m \033[107m  \n"
                "  \033[40m▄▄▄▄▄▄▄\033[107m \033[40m \033[107;30m▄\033[40;97m▄\033[107m \033[40m \033[107m \033[40m▄\033[107m \033[40m▄\033[107;30m▄\033[40;97m▄\033[107m \033[40m▄\033[107m \033[40m▄▄▄▄▄▄▄\033[107m  \n"
                "  \033[40m▄▄\033[107m \033[40m▄\033[107m  \033[40m▄▄\033[107m  \033[40m \033[107;30m▄ \033[40m \033[97m▄\033[107m \033[30m▄ ▄\033[40;97m▄\033[107;30m▄ \033[40m  \033[97m▄\033[107;30m▄\033[40;97m▄ \033[107;30m▄  \n"
                "  ▄\033[40;97m▄▄\033[107;30m▄▄ \033[40;97m▄\033[107;30m▄\033[40m \033[97m▄▄\033[107m  \033[40m ▄\033[107;30m▄\033[40;97m▄\033[107m \033[40m ▄\033[107;30m▄\033[40m \033[107m \033[40m \033[97m▄\033[107m \033[30m▄\033[40m \033[107m   \n"
                "  \033[40;97m▄\033[107m   \033[30m▄▄\033[40;97m▄\033[107m \033[40m \033[107m \033[30m▄▄\033[40m \033[107m ▄   ▄\033[40;97m▄  ▄▄\033[107m \033[30m▄▄\033[40;97m▄▄\033[107m  \n"
                "   \033[40m▄ \033[107;30m▄▄\033[40;97m▄▄\033[107m  \033[30m▄\033[40;97m▄\033[107m \033[40m \033[107;30m▄\033[40;97m▄\033[107m \033[30m▄▄\033[40;97m▄\033[107;30m▄▄▄▄ \033[40;97m▄▄▄▄▄\033[107m  \n"
                "     \033[40m▄\033[107;30m▄ \033[40;97m▄ ▄\033[107m \033[40m▄▄\033[107m \033[40m \033[107m \033[40m▄\033[107;30m▄\033[40m \033[107m ▄\033[40;97m▄ \033[107;30m▄\033[40m \033[107m \033[40m \033[107m ▄\033[40m \033[107m  \n"
                "  \033[40;97m▄\033[107m \033[40m \033[107;30m▄\033[40m \033[97m▄▄\033[107;30m▄▄\033[40m \033[107m▄▄ \033[40;97m▄\033[107m \033[30m▄ ▄\033[40m \033[107m ▄ \033[40;97m▄▄▄ ▄ ▄\033[107m  \n"
                "  \033[40m▄\033[107m \033[40m▄▄▄\033[107m \033[40m▄▄\033[107;30m▄\033[40m    \033[107m \033[40m \033[107m▄\033[40m \033[107m▄  \033[40m \033[97m▄▄▄ \033[107m \033[40m \033[107;30m▄\033[40m \033[107m  \n"
                "  \033[40m \033[97m▄▄▄▄▄ \033[107m \033[40m▄    \033[107;30m▄\033[40m \033[107m ▄\033[40;97m▄\033[107;30m▄▄\033[40m \033[107m \033[40;97m▄\033[107m \033[40m  \033[107;30m▄\033[40m \033[107m▄  \n"
                "  \033[40m \033[107m \033[40m   \033[107m \033[40m \033[107m ▄ ▄\033[40m \033[107m ▄\033[40;97m▄▄▄\033[107m \033[30m▄▄\033[40;97m▄▄  ▄▄\033[107;30m▄▄   \n"
                "  \033[40m \033[107m \033[40;97m▄▄▄\033[107m \033[40m \033[107m \033[30m▄\033[40;97m▄\033[107m \033[40m \033[107m \033[40m▄▄\033[107m \033[40m▄\033[107;30m▄\033[40;97m▄\033[107m \033[30m▄\033[40m \033[97m▄\033[107m \033[40m▄\033[107;30m▄\033[40;97m▄\033[107;30m▄\033[40;97m▄\033[107m  \n"
                "  \033[40m▄▄▄▄▄▄▄\033[107m \033[40m▄▄▄\033[107m \033[40m▄▄\033[107m  \033[40m▄▄\033[107m \033[40m▄▄▄\033[107m  \033[40m▄\033[107m  \033[40m▄\033[107m   \n"
                "\033[30m▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄\n").nil();

                desktopio_body = ansi::nil().eol()
                .mgl(2).mgr(2).wrp(wrap::off)
                .fgc(bluedk).jet(bias::left)
                .bgc(bluedk).fgc(0xFFFFFFFF)
                .add(" Text-based Desktop Environment "
                "\n\n")
                .fgc().bgc().jet(bias::left).wrp(wrap::on).add(
                "A text-based desktop environment is an environment "
                "in which the entire user interface is presented using text output.\n"
                "The first biggest advantage of this concept that "
                "it can be used directly over SSH connections, no additional protocol needed.\n"
                "The second is the flexible multi-user interface "
                "that serves several users engaged in collaborative applications and enables "
                "users to view a collaborating user's workspace.\n"
                "\n"
                "If you like the way we think and would like to support the project "
                "in the spirit of Bitcoin, you can donate at the following public "
                "bitcoin address:\n\n")
                .jet(bias::center).wrp(wrap::off).mgl(1).mgr(1)
                .add(qr,
                " bitcoin:1Euu4jcQ15LKijaDyZigZrnEoqwe1daTVZ\n");
            }

            return std::tuple{ appstore_head, appstore_body, desktopio_body };
        };

        auto build = [](text env, text cwd, text arg, xmls& config, text patch)
        {
            auto highlight_color = skin::color(tone::highlight);
            auto c3 = highlight_color;
            auto x3 = cell{ c3 }.alpha(0x00);

            auto [appstore_head, appstore_body, desktopio_body] = get_text();
            auto window = ui::cake::ctor();
            window->plugin<pro::focus>(pro::focus::mode::focused)
                  ->colors(whitelt, 0x60000000)
                  ->plugin<pro::track>()
                  ->plugin<pro::acryl>()
                  ->plugin<pro::cache>()
                  ->invoke([](auto& boss)
                  {
                        //boss.keybd.accept(true);
                        boss.LISTEN(tier::anycast, e2::form::proceed::quit::any, fast)
                        {
                            boss.RISEUP(tier::release, e2::form::proceed::quit::one, fast);
                        };
                  });
            auto object = window->attach(ui::fork::ctor(axis::Y))
                                ->colors(whitelt, 0);
                auto menu_object = object->attach(slot::_1, ui::fork::ctor(axis::Y));
                    config.cd("/config/gems/", "/config/defapp/");
                    auto [menu_block, cover, menu_data] = app::shared::menu::create(config, {});
                    menu_object->attach(slot::_1, menu_block);
                    menu_object->attach(slot::_2, ui::post::ctor())
                               ->upload(appstore_head)
                               ->active();
                auto layers = object->attach(slot::_2, ui::cake::ctor());
                    auto scroll = layers->attach(ui::rail::ctor())
                                        ->active()
                                        ->colors(whitedk, 0xFF0f0f0f)
                                        ->limits({ -1,-1 }, { -1,-1 });
                        auto items = scroll->attach(ui::list::ctor());
                        for (auto& body : appstore_body) items->attach(ui::post::ctor())
                                                              ->upload(body)
                                                              ->active()
                                                              ->plugin<pro::focus>()
                                                              ->plugin<pro::grade>()
                                                              ->shader(cell::shaders::xlight, e2::form::state::hover)
                                                              ->shader(cell::shaders::color(c3), e2::form::state::keybd::focus::count);
                        items->attach(ui::post::ctor())
                             ->upload(desktopio_body)
                             ->plugin<pro::grade>();
                layers->attach(app::shared::scroll_bars(scroll));
            return window;
        };
    };

    app::shared::initialize builder{ app::shop::id, build };
}