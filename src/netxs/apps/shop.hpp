// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_APP_SHOP_HPP
#define NETXS_APP_SHOP_HPP

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

// shop: Desktopio App Store.
namespace netxs::app::shop
{
    using events = netxs::events::userland::shop;

    namespace
    {
        auto get_text = []()
        {
            static text desktopio_body;
            static text appstore_head;
            static std::list<text> appstore_body;

            if (appstore_head.empty())
            {
                text monotty_logo  = ansi::bgc(blackdk  ).add("▀▄");
                text textancy_logo = ansi::bgc(cyandk   ).add("▀▄");
                text cellatix_logo = ansi::bgc(greendk  ).add("▀▄");
                text informio_logo = ansi::bgc(magentadk).add("▀▄");
                text ansiplex_logo = ansi::bgc(reddk    ).add("▀▄");
                text unicodex_logo = ansi::bgc(yellowdk ).add("▀▄");
                text appstore_logo = ansi::bgc(blacklt  ).add("▀▄");

                text line = ansi::wrp(wrap::off).add("──────────────────────────────────────────────────────────────────────────────────────").wrp(wrap::on).eol();
                auto item = [](auto app, auto clr, auto rating, auto price, auto buy, auto desc)
                {
                    auto clr_light = rgba{ clr };
                    clr_light.mix(0xa7ffffff);

                    text lot = ansi::nil()
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
                .bld(faux).fgc(whitelt).jet(bias::left).wrp(wrap::on).add(
                "A digital distribution platform, developed "
                "and maintained by NetXS Group, for TUI/terminal "
                "apps on its desktop environment. "
                "The store allows users to browse and download "
                "apps developed with Desktopio software "
                "development kit.\n");

                text textancy_text = ansi::nil().add(
                "Hello World!😎\n"
                "絵文字:\n"
                "English: /ɪˈmoʊdʒiː/;\n"
                "Japanese: [emodʑi];\n");

                appstore_body =
                {
                    item("Term", blackdk, "469", "Free ", "Get",
                    "Terminal emulator."),

                    item("Tile", bluedk, "3", "Free ", "Get",
                    ansi::add("Meta object. Tiling window manager preconfigurable "
                    "using environment variable ").
                    fgc(whitelt).bld(true).add("VTM_TILE").fgc().bld(faux).
                    add(".\n\nConfiguration example:\n\n").
                    mgl(2).fgc(whitelt).bgc(blacklt)
                    .add(" VTM_PROFILE_1='\"Menu label 1\", \"Window Title 1\", h1:2( v1:1(\"bash -c htop\", \"bash -c mc\"), \"bash\")' \n"
                        " VTM_PROFILE_2='\"Menu label 2\", \"Window Title 2\", h( v(\"bash -c htop\", \"bash -c mc\"), a(\"Calc\",\"\",\"\"))' ")),

                    item("Text", cyandk, "102", "Free ", "Get",
                    "A simple text editor for Monotty environment "
                    "and a basic editing tool which enables "
                    "desktop users to create documents that "
                    "contain ANSI-formatted text."),

                    item("Calc", greendk, "30", "Free ", "Get",
                    "A simple spreadsheet calculator application."),

                    item("Task", magentadk, "311", "Free ", "Get",
                    "A task manager program that displays "
                    "information about CPU, memory utilization, "
                    "and current I/O usage."),

                    item("Draw", reddk, "64", "Free ", "Get",
                    "A simple program which enables desktop "
                    "users to create sophisticated ANSI-artworks."),

                    item("Char", yellowdk, "161", "Free ", "Get",
                    "An utility that allows browsing all Unicode "
                    "codepoints and inspecting their metadata."),

                    item(ansi::fgc(0xff0000).add("File"), cyanlt, "4", "Free ", "Get",
                    "An orthodox file manager for Monotty environment."),

                    item("Time", bluedk, "4", "Free ", "Get",
                    "A calendar application made by NetXS Group for Monotty environment."),

                    item("Goto", bluedk, "4", "Free ", "Get",
                    "Internet/SSH browser."),

                    item(ansi::fgc(0xFF00FFFF).add("Doom").fgc(), reddk, "4", "Free ", "Get",
                    "Doom II source port."),

                    item("Logs", blackdk, "4096", "Free ", "Get",
                    "Application for displaying debug trace. "
                    "This is more efficient than writing to STDOUT."),

                    item("Clip", bluedk, "1", "Free ", "Get",
                    "Clipboard manager."),

                    item("Info", cyandk, "1", "Free ", "Get",
                    "Software documentation browser."),

                    item("Hood", reddk, "1", "Free ", "Get",
                    "Desktop environment settings configurator."),

                    item("View", cyandk, "1", "Free ", "Get",
                    "Meta object. Desktop location marker."),
                };

                text qr = ansi::esc(
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
                .add(" Monotty Desktopio "
                "\n\n")
                .fgc().bgc().jet(bias::left).wrp(wrap::on).add(
                "Monotty Desktopio is a cross-platform, full-featured desktop environment."
                " A user interface where by all output is presented in the form of text.\n"
                "The first biggest advantage of this desktop environment concept that "
                "it can be used directly over SSH connections, no additional protocol needed.\n"
                "The second is the flexible multi-user interface "
                "that serves several users engaged in collaborative applications and enables "
                "users to view a collaborating user’s workspace.\n"
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

        auto build = [](view v)
        {
            const static auto c3 = app::shared::c3;
            const static auto x3 = app::shared::x3;

            auto [appstore_head, appstore_body, desktopio_body] = get_text();
            auto window = ui::cake::ctor();
            window->plugin<pro::focus>()
                  ->colors(whitelt, 0x60000000)
                  ->plugin<pro::track>()
                  ->plugin<pro::acryl>()
                  ->plugin<pro::cache>()
                  ->invoke([](auto& boss)
                  {
                        boss.keybd.accept(true);
                  });
            auto object = window->attach(ui::fork::ctor(axis::Y))
                                ->colors(whitelt, 0);
                auto menu_object = object->attach(slot::_1, ui::fork::ctor(axis::Y));
                    menu_object->attach(slot::_1, app::shared::custom_menu(true, {}));
                    menu_object->attach(slot::_2, ui::post::ctor())
                               ->plugin<pro::limit>(twod{ 37,-1 }, twod{ -1,-1 })
                               ->upload(appstore_head)
                               ->active();
                auto layers = object->attach(slot::_2, ui::cake::ctor());
                    auto scroll = layers->attach(ui::rail::ctor())
                                        ->colors(whitedk, 0xFF0f0f0f)
                                        ->plugin<pro::limit>(twod{ -1,2 }, twod{ -1,-1 });
                        auto items = scroll->attach(ui::list::ctor());
                        for (auto& body : appstore_body) items->attach(ui::post::ctor())
                                                              ->upload(body)
                                                              ->plugin<pro::grade>()
                                                              ->plugin<pro::fader>(x3, c3, 250ms);
                        items->attach(ui::post::ctor())
                             ->upload(desktopio_body)
                             ->plugin<pro::grade>();
                layers->attach(app::shared::scroll_bars(scroll));
            return window;
        };
    };

    app::shared::initialize builder{ "Shop", build };
}

#endif // NETXS_APP_SHOP_HPP