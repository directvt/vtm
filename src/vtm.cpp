// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#define MONOTTY_VER "Monotty Desktopio v0.7.2"

// Enable demo apps and assign Esc key to log off.
//#define DEMO

// Enable keyboard input and unassign Esc key.
#define PROD

// Tiling limits.
#ifndef PROD
    #define INHERITANCE_LIMIT 12
    #define APPS_MAX_COUNT    20
    #define TILE_MAX_COUNT    2
#else
    #define INHERITANCE_LIMIT 30
#endif
#define APPS_DEL_TIMEOUT  1s

#define MONOTTY_PREFIX "monotty_"
#define MONOTTY_FOLDER "/.config/vtm/"

// Enable to show debug overlay.
//#define DEBUG_OVERLAY

// Enable to show all terminal input (keyboard/mouse etc).
//#define KEYLOG

// Highlight region ownership.
//#define REGIONS

#include "netxs/apps.hpp"

#include <fstream> // Get current config from vtm.conf.

using namespace netxs::console;
using namespace netxs;

int main(int argc, char* argv[])
{
    auto syslog = logger([](auto data) { os::syslog(data); });
    auto banner = [&]() { log(MONOTTY_VER); };
    auto prefix = [](auto user) { return utf::concat(MONOTTY_PREFIX, user); }; //todo unify, use vtm.conf
    auto client = true;
    auto daemon = faux;
    {
        auto getopt = os::args{ argc, argv };
        while (getopt)
        {
            switch (getopt.next())
            {
                case 's': client = faux; break;
                case 'd': daemon = true; break;
                default:
                    #ifndef PROD

                        if (os::get_env("SHELL").ends_with("vtm"))
                        {
                            auto error = utf::text{ "main: interactive server is not allowed in demo mode" };
                            if (argc > 1)
                            {
                                auto host = os::get_env("SSH_CLIENT");
                                auto name = os::get_env("USER");
                                error += "\nblock explicit shell command invocation {" + name + ", " + host + "}";
                                for (auto i = 1; i < argc; i++)
                                {
                                    error += '\n';
                                    error += utf::text(argv[i]);
                                }
                            }
                            os::start_log("vtm");
                            log(error);
                            return 1;
                        }

                    #endif

                    banner();
                    log("Usage:\n\n ", argv[0], " [OPTION...]\n\n"s
                                    + " No arguments\tRun client, auto start server if is not started.\n"s
                                             + "\t-d\tRun server in background.\n"s
                                             + "\t-s\tRun server in interactive mode.\n"s);
                    return 0;;
            }
        }

        if (daemon)
        {
            if (!os::daemonize(argv[0]))
            {
                banner();
                log("main: failed to daemonize");
                return 1;
            }
            else client = faux;
        }
    }

    banner();

    auto user = os::user();
    auto spot = utf::text{};
    auto conf = utf::text{};
    auto path = prefix(user);
    {
        std::ifstream config;
        config.open(os::homepath() + MONOTTY_FOLDER "settings.ini");

        if (config.is_open())
            std::getline(config, spot);

        if (spot.empty())
            spot = "unknown region";

        //todo unify
        //fps
        //skin::setup(tone::lucidity, 192);
        //skin::setup(tone::shadower, 0);
        skin::setup(tone::kb_focus, 60);
        skin::setup(tone::brighter, 60);//120);
        skin::setup(tone::shadower, 180);//60);//40);// 20);
        skin::setup(tone::shadow  , 180);//5);
        skin::setup(tone::lucidity, 255);
        skin::setup(tone::selector, 48);
        skin::setup(tone::bordersz, dot_11);
    }

    if (client)
    {
        os::start_log("vtm");

        auto link = os::ipc::open<os::client>(path, 10s, [&]()
                    {
                        log("main: new desktop environment for user ", user);
                        auto binary = view{ argv[0] };
                        utf::trim_front(binary, "-"); // Sometimes "-" appears before executable.
                        return os::exec(text{ binary }, "-d");
                    });
        if (!link)
        {
            log("main: error: no desktop server connection");
            return 1;
        }

        auto host = os::get_env("SSH_CLIENT");
        auto name = os::get_env("USER");
        auto mode = os::legacy_mode();

        link->send(utf::concat(spot, ";",
                               host, ";",
                               name, ";",
                               user, ";",
                               mode, ";"));
        auto gate = os::tty::proxy(link);
        gate.ignite();
        gate.output(ansi::esc{}.save_title()
                               .altbuf(true)
                               .vmouse(true)
                               .cursor(faux)
                               .bpmode(true)
                               .setutf(true));
        gate.splice(mode);
        gate.output(ansi::esc{}.scrn_reset()
                               .vmouse(faux)
                               .cursor(true)
                               .altbuf(faux)
                               .bpmode(faux)
                               .load_title());

        std::this_thread::sleep_for(200ms); // Pause to complete consuming/receiving buffered input (e.g. mouse tracking) that has just been canceled.
    }
    else
    {
        auto link = os::ipc::open<os::server>(path);
        if (!link)
        {
            log("main: error: can't start desktop server");
            return 1;
        }

        syslog.tee<events::try_sync>([](auto utf8) { SIGNAL_GLOBAL(e2::debug::logs, utf8); });

        log("main: listening socket ", link,
                         "\n\tuser: ", user,
                         "\n\tpipe: ", path);

        auto world = base::create<host>(link);
        app::shared::init_app_registry(world);

        world->SIGNAL(tier::general, e2::config::fps, 60);

        auto session = os::pool{};
        while (auto peer = link->meet())
        {
            if (!peer->cred(user))
            {
                log("main: abort: foreign users are not allowed to the session");
                continue;
            }

            auto lock = session.lock();
            auto conf = console::conf(peer, session.next());
            log("main: incoming connection:", conf);

            conf.background_color = app::shared::background_color; //todo unify

            session.run([&, peer, conf]()
            {
                if (auto client = world->invite<ui::gate>())
                {
                    log("user: new gate for ", peer);
                    auto deskmenu = app::shared::creator("Desk")(utf::concat(client->id, ";", conf.os_user_id));
                    auto bkground = app::shared::creator("Fone")("Shop;Demo;");
                    client->run(deskmenu, bkground, peer, conf);

                    world->resign(client);
                    log("user: ", peer, " logged out");
                }
            });
        }

        SIGNAL_GLOBAL(e2::conio::quit, "main: server shutdown");
    }
}