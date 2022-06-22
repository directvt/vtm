// Copyright (c) NetXS Group.
// Licensed under the MIT license.

auto DirectVT = "\3\33\333DESKTOPIO\7"
R"==(
="Term", "Tooltip Message", a("Direct", "Term Title", "-r term")
="Logs", "Tooltip Message", a("Direct", "Logs Title", "-r logs")
="Gems", "Tooltip Message", a("Direct", "Gems Title", "-r gems")
="Text", "Tooltip Message", a("Direct", "Text Title", "-r text")
="Calc", "Tooltip Message", a("Direct", "Calc Title", "-r calc")
="Test", "Tooltip Message", a("Direct", "Test Title", "-r test")
)==";

#define DESKTOP_VER "v0.7.6"
#define MONOTTY_VER "Monotty Desktopio " DESKTOP_VER
#define MONOTTY_PREFIX "monotty_"
#define MONOTTY_MYNAME "vtm"
#define MONOTTY_FOLDER "/.config/" MONOTTY_MYNAME "/"
#define MONOTTY_APPDIR "/.config/" MONOTTY_MYNAME "/apps"
#define MONOTTY_DEFAPP "Term"
#define MONOTTY_APPINF "Desktopio Terminal " DESKTOP_VER

// Enable demo apps and assign Esc key to log off.
//#define DEMO

// Enable keyboard input and unassign Esc key.
#define PROD

// Enable to show all terminal input (keyboard/mouse etc).
//#define KEYLOG

#include "netxs/apps.hpp"
#include <fstream> // Get current config from vtm.conf.

using namespace netxs;
using namespace netxs::console;

enum class type
{
    client,
    server,
    runapp,
};

int main(int argc, char* argv[])
{
    auto vtmode = os::vt_mode();
    auto syslog = os::ipc::logger(vtmode);
    auto banner = [&]() { log(MONOTTY_VER); };
    auto whoami = type::client;
    auto region = text{};
    auto params = text{};
    auto maxfps = si32{ 60 };
    {
        auto daemon = faux;
        auto getopt = os::args{ argc, argv };
        while (getopt)
        {
            switch (getopt.next())
            {
                case 'r':
                    whoami = type::runapp;
                    params = getopt ? getopt.tail()
                                    : text{ MONOTTY_DEFAPP };
                    break;
                case 's': whoami = type::server; break;
                case 'd': daemon = true; break;
                default:
                    #ifndef PROD

                        if (os::get_env("SHELL").ends_with(MONOTTY_MYNAME))
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
                            os::start_log(MONOTTY_MYNAME);
                            log(error);
                            return 1;
                        }

                    #endif

                    banner();
                    log("Usage:\n\n ", argv[0], " [ -d | -s | -r [<app> [<args...>]] ]\n\n"s
                                    + " No arguments\tRun client, auto start server if is not started.\n"s
                                             + "\t-d\tRun server in background.\n"s
                                             + "\t-s\tRun server in interactive mode.\n"s
                                             + "\t-r\tRun standalone application.\n"s
                                             + "\n"s
                                             + "\tList of registered applications:\n\n"s
                                             + "\t\tTerm\tTerminal emulator (default)\n"s
                                             + "\t\tText\t(Demo) Text editor\n"s
                                             + "\t\tCalc\t(Demo) Spreadsheet calculator\n"s
                                             + "\t\tGems\t(Demo) Desktopio application manager\n"s
                                             );
                    return 0;
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
            else whoami = type::server;
        }
    }

    {
        //todo mutex
        auto config = std::ifstream{};
        config.open(os::homepath() + MONOTTY_FOLDER "settings.ini");

        if (config.is_open())
        {
            std::getline(config, region);
        }

        if (region.empty())
        {
            region = "unknown region";
        }

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

    if (whoami == type::server)
    {
        banner();
        auto userid = os::user();
        auto usernm = os::get_env("USER");
        auto hostip = os::get_env("SSH_CLIENT");
        auto prefix = utf::concat(MONOTTY_PREFIX, userid);
        auto server = os::ipc::open<os::server>(prefix);
        if (!server)
        {
            log("main: error: can't start desktop server");
            return 1;
        }
        auto srvlog = syslog.tee<events::try_sync>([](auto utf8) { SIGNAL_GLOBAL(e2::debug::logs, utf8); });
        auto ground = base::create<hall>(server, maxfps);
        auto thread = os::pool{};
        app::shared::init_app_registry(ground);

        log("main: listening socket ", server,
                         "\n\tuser: ", userid,
                         "\n\tpipe: ", prefix);

        while (auto client = server->meet())
        {
            if (!client->cred(userid))
            {
                log("main: abort: foreign users are not allowed to the session");
                continue;
            }

            thread.run([&, client](auto session_id)
            {
                auto config = console::conf(client, session_id);
                config.background_color = app::shared::background_color; //todo unify
                log("user: incoming connection:", config);

                if (auto window = ground->invite<gate>())
                {
                    log("user: new gate for ", client);
                    auto deskmenu = app::shared::creator("Desk")(utf::concat(window->id, ";", config.os_user_id));
                    auto bkground = app::shared::creator("Fone")("Gems;Demo;");
                    window->launch(client, config, deskmenu, bkground);
                    log("user: ", client, " logged out");
                }
            });
        }

        SIGNAL_GLOBAL(e2::conio::quit, "main: server shutdown");
        ground->shutdown();
    }
    else
    {
        if (whoami == type::client)
        {
            banner();
            auto direct = !!(vtmode & os::legacy::direct);
            if (!direct) os::start_log(MONOTTY_MYNAME);
            auto userid = os::user();
            auto usernm = os::get_env("USER");
            auto hostip = os::get_env("SSH_CLIENT");
            auto prefix = utf::concat(MONOTTY_PREFIX, userid);
            auto client = os::ipc::open<os::client>(prefix, 10s, [&]()
                        {
                            log("main: new desktop environment for user ", userid);
                            auto binary = view{ argv[0] };
                            utf::trim_front(binary, "-"); // Sometimes "-" appears before executable.
                            return os::exec(text{ binary }, "-d");
                        });
            if (!client)
            {
                log("main: error: no desktop server connection");
                return 1;
            }
            client->send(utf::concat(region, ";",
                                     hostip, ";",
                                     usernm, ";",
                                     userid, ";",
                                     vtmode, ";"));
            auto cons = os::tty::proxy(client);
            auto size = cons.ignite(vtmode);
            if (size.last)
            {
                os::ipc::splice(cons, vtmode);
            }
        }
        else if (whoami == type::runapp)
        {
            //todo unify
            auto menusz = 3;
            utf::to_up(utf::to_low(params), 1);
                 if (params == "Text") log("Desktopio Text Editor (DEMO) " DESKTOP_VER);
            else if (params == "Calc") log("Desktopio Spreadsheet (DEMO) " DESKTOP_VER);
            else if (params == "Gems") log("Desktopio App Manager (DEMO) " DESKTOP_VER);
            else if (params == "Test") log("Desktopio App Testing (DEMO) " DESKTOP_VER);
            else if (params == "Logs") log("Desktopio Log Console "        DESKTOP_VER);
            else
            {
                menusz = 1;
                params = MONOTTY_DEFAPP;
                log(MONOTTY_APPINF);
            }

            skin::setup(tone::brighter, 0);

            auto success = app::shared::start(params, MONOTTY_MYNAME, vtmode, maxfps, menusz);
            if (!success)
            {
                log("main: console initialization error");
                return 1;
            }
        }
    }
}