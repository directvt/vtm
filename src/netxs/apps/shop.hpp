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
                .bld(faux).fgc(whitelt).jet(bias::left).wrp(wrap::on).add(
                "Terminal Application Distribution Platform that allows "
                "users to browse and download applications developed with "
                "Desktopio Framework.\n\n");

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
                    "Meta object. Tiling window manager."),

                    item("Text", cyandk, "102", "Free ", "Get",
                    "A simple text editor for Desktopio environment "
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

                    item(ansi::fgc(0xFFff0000).add("File"), cyanlt, "4", "Free ", "Get",
                    "An orthodox file manager for Desktopio environment."),

                    item("Time", bluedk, "4", "Free ", "Get",
                    "A calendar application for Desktopio environment."),

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
                    "Workspace settings configurator."),

                    item("View", cyandk, "1", "Free ", "Get",
                    "Meta object. Workspace location marker."),
                };

                auto qr = ansi::esc(
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
                .add(" Desktopio Environment "
                "\n\n")
                .fgc().bgc().jet(bias::left).wrp(wrap::on).add(
                "Desktopio Environment is a cross-platform, full-featured desktop environment."
                " A user interface where by all output is presented in the form of text.\n"
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

        auto build = [](text cwd, text arg, xml::settings& config, text patch)
        {
            auto highlight_color = skin::color(tone::highlight);
            auto c3 = highlight_color;
            auto x3 = cell{ c3 }.alpha(0x00);

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
                        boss.SUBMIT(tier::anycast, e2::form::quit, item)
                        {
                            boss.base::template riseup<tier::release>(e2::form::quit, item);
                        };
                  });
            auto object = window->attach(ui::fork::ctor(axis::Y))
                                ->colors(whitelt, 0);
                auto menu_object = object->attach(slot::_1, ui::fork::ctor(axis::Y));
                    config.cd("/config/gems/", "/config/defapp/");
                    auto [menu_block, cover, menu_data] = app::shared::custom_menu(config, {});
                    menu_object->attach(slot::_1, menu_block);
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

    app::shared::initialize builder{ "gems", build };
}

#endif // NETXS_APP_SHOP_HPP