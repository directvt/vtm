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

#define MONOTTY_PREFIX_ "monotty_"

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
    auto prefix = [](auto user) { return utf::concat(MONOTTY_PREFIX_, user); }; //todo unify, use vtm.conf
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
    auto path = prefix(user);
    auto spot = utf::text{};
    auto conf = utf::text{};
    {
        std::ifstream config;
        config.open(os::homepath() + "/.config/vtm/settings.ini");

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
            log("main: error: can't start new desktop server (check if another instance of the server is running)");
            return 1;
        }

        syslog.tee<events::try_sync>([](auto utf8) { SIGNAL_GLOBAL(e2::debug::logs, utf8); });

        log("main: listening socket ", link,
                         "\n\tuser: ", user,
                         "\n\tpipe: ", path);

        auto world = base::create<host>(link);
        app::shared::init_app_registry(world);

        world->SIGNAL(tier::general, e2::config::fps, 60);

        os::pool sessions;
        while (auto peer = link->meet())
        {
            auto lock = sessions.lock();

            if (!peer->cred(user))
            {
                log("main: abort: foreign users are not allowed to the session");
                continue;
            }

            //todo unify
            auto _region = peer->line(';');
            auto _ip     = peer->line(';');
            auto _user   = peer->line(';');
            auto _name   = peer->line(';');
            auto _mode   = peer->line(';');
            log("main: new user:",
                   "\n\t    ip: ", _ip,
                   "\n\tregion: ", _region,
                   "\n\t  user: ", _user,
                   "\n\t  name: ", _name,
                   "\n\t  mode: ", _mode);

            //todo unify
            text c_ip;
            text c_port;
            auto c_info = utf::divide(_ip, " ");
            if (c_info.size() > 0) c_ip   = c_info[0];
            if (c_info.size() > 1) c_port = c_info[1];
            utf::change(_ip, " ", ":");

            //todo Move user's viewport to the last saved position
            auto user_coor = twod{};

            //todo distinguish users by config, enumerate if no config
            _name = "[" + _name + ":" + std::to_string(sessions.next()) + "]";
            log("main: creating a new session for user ", _name);

            sessions.check_in([&sessions,
                               &world,
                                _name,
                                _mode,
                                _region,
                                peer,
                                user,
                                c_ip,
                                c_port,
                                user_coor]()
            {
                log("user: session name ", peer);

                #ifndef PROD
                    auto username = "[User." + utf::remain(c_ip) + ":" + c_port + "]";
                #else
                    auto username = _name;
                #endif

                auto lock = std::make_unique<events::sync>();
                    auto legacy_mode = os::legacy::clean;
                    if (auto mode = utf::to_int(view(_mode)))
                    {
                        legacy_mode = mode.value();
                    }
                    auto client = world->invite<ui::gate>(username, legacy_mode);

                    auto& menu_builder = app::shared::creator("Desk");
                    auto deskmenu = menu_builder(utf::concat(client->id, ";", user));

                    auto& fone_builder = app::shared::creator("Fone");
                    auto bkground = fone_builder(
                    //todo
                    //#ifndef PROD
                        "Shop;Demo;"
                    //#else
                    //    "HeadlessTerm;Info;ssh info@netxs.online"
                    //#endif
                    );
            
                    client->attach(deskmenu);
                    client->ground(bkground);
                    client->color(app::shared::background_color.fgc(), app::shared::background_color.bgc());
                    text header = username;
                    client->SIGNAL(tier::release, e2::form::prop::name, header);
                    client->SIGNAL(tier::preview, e2::form::prop::header, header);
                    client->base::moveby(user_coor);
                lock.reset();
                log("user: new gate for ", peer);

                client->proceed(peer,
                    #ifndef PROD
                        _region
                    #else
                        username
                    #endif
                    );

                lock = std::make_unique<events::sync>();
                    log("user: ", peer, " has logged out");
                    client->detach();
                    log("user: ", peer, " is diconnected");
                    client.reset();
                lock.reset();

                sessions.check_out();
            });
        }

        SIGNAL_GLOBAL(e2::conio::quit, "main: server shutdown");
    }
}