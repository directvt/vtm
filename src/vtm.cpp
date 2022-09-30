// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#define DESKTOPIO_VER "v0.9.0"
#define DESKTOPIO_MYNAME "vtm " DESKTOPIO_VER
#define DESKTOPIO_PREFIX "desktopio_"
#define DESKTOPIO_MYPATH "vtm"
#define DESKTOPIO_DEFAPP "Term"
#define DESKTOPIO_APPINF "Desktopio Terminal " DESKTOPIO_VER

// Enable to show all terminal input (keyboard/mouse etc).
//#define KEYLOG

#include "netxs/apps.hpp"

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
    auto banner = [&]{ log(DESKTOPIO_MYNAME); };
    auto whoami = type::client;
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
                                    : text{ DESKTOPIO_DEFAPP };
                    break;
                case 's': whoami = type::server; break;
                case 'd': daemon = true; break;
                default:
                    banner();
                    log("Usage:\n\n ", os::current_module_file(), " [ -d | -s | -r [<app> [<args...>]] ]\n\n"s
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
            if (!os::daemonize(os::current_module_file()))
            {
                banner();
                log("main: failed to daemonize");
                return 1;
            }
            else whoami = type::server;
        }
    }

    {
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
        auto prefix = utf::concat(DESKTOPIO_PREFIX, userid);
        auto server = os::ipc::open<os::server>(prefix);
        if (!server)
        {
            log("main: error: can't start desktopio server");
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

                if (auto window = ground->invite<gate>(config))
                {
                    log("user: new gate for ", client);
                    auto deskmenu = app::shared::creator(app::shared::type_Desk)("", utf::concat(window->id, ";", config.os_user_id));
                    auto bkground = app::shared::creator(app::shared::type_Fone)("", "gems; About; ");
                    window->launch(client, deskmenu, bkground);
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
            if (!direct) os::start_log(DESKTOPIO_MYPATH);
            auto userid = os::user();
            auto usernm = os::get_env("USER");
            auto hostip = os::get_env("SSH_CLIENT");
            auto prefix = utf::concat(DESKTOPIO_PREFIX, userid);
            auto client = os::ipc::open<os::client>(prefix, 10s, [&]
                        {
                            log("main: new desktopio environment for user ", userid);
                            auto binary = os::current_module_file();
                            return os::exec(binary, "-d");
                        });
            if (!client)
            {
                log("main: error: no desktopio server connection");
                return 1;
            }
            client->send(utf::concat(hostip, ";",
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
            auto shadow = params;
            utf::to_low(shadow);
                 if (shadow.starts_with("text"))       log("Desktopio Text Editor (DEMO) " DESKTOPIO_VER);
            else if (shadow.starts_with("calc"))       log("Desktopio Spreadsheet (DEMO) " DESKTOPIO_VER);
            else if (shadow.starts_with("gems"))       log("Desktopio App Manager (DEMO) " DESKTOPIO_VER);
            else if (shadow.starts_with("test"))       log("Desktopio App Testing (DEMO) " DESKTOPIO_VER);
            else if (shadow.starts_with("logs"))       log("Desktopio Log Console "        DESKTOPIO_VER);
            else if (shadow.starts_with("term"))       log("Desktopio Terminal "           DESKTOPIO_VER);
            else if (shadow.starts_with("truecolor"))  log("Desktopio ANSI Art "           DESKTOPIO_VER);
            else if (shadow.starts_with("headless"))   log("Desktopio Headless Terminal "  DESKTOPIO_VER);
            else if (shadow.starts_with("settings"))   log("Desktopio Settings "           DESKTOPIO_VER);
            else
            {
                menusz = 1;
                params = DESKTOPIO_DEFAPP + " "s + params;
                log(DESKTOPIO_APPINF);
            }

            skin::setup(tone::brighter, 0);

            auto success = app::shared::start(params, DESKTOPIO_MYPATH, vtmode, maxfps, menusz);
            if (!success)
            {
                log("main: console initialization error");
                return 1;
            }
        }
    }
}