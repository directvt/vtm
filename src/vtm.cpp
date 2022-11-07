// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#define DESKTOPIO_VER "v0.9.6f"
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
    auto cfpath = text{};
    auto daemon = faux;
    auto getopt = os::args{ argc, argv };
    while (getopt)
    {
        switch (getopt.next())
        {
            case 'r':
                whoami = type::runapp;
                params = getopt ? getopt.tail() : text{ DESKTOPIO_DEFAPP };
                break;
            case 's':
                whoami = type::server;
                break;
            case 'd':
                daemon = true;
                break;
            case 'l':
                log(app::shared::load::settings(cfpath, os::legacy::get_setup()).document->show());
                return 0;
            case 'c':
                cfpath = getopt.param();
                if (cfpath.size()) break;
                else os::fail("config file path not specified");
            default:
                banner();
                log("Usage:\n\n ", os::current_module_file(), " [ -c <config_file> ] [ -l | -d | -s | -r [<app> [<args...>]] ]\n"s
                    + "\n"s
                    + " No arguments\tRun client, auto start server if is not started.\n"s
                        + "\t-c <..>\tUse specified configuration file.\n"s
                        + "\t-l\tShow configuration and exit.\n"s
                        + "\t-d\tRun server in background.\n"s
                        + "\t-s\tRun server in interactive mode.\n"s
                        + "\t-r <..>\tRun standalone application.\n"s
                        + "\n"s
                        + "\tConfiguration file location precedence (descending priority):\n\n"s
                        + "\t\t1. Command line options; e.g., vtm -c path/to/settings.xml\n"s
                        + "\t\t2. Environment variable; e.g., VTM_CONFIG=path/to/settings.xml\n"s
                        + "\t\t3. Hardcoded location \""s + app::shared::usr_config + "\"\n"s
                        + "\t\t4. Default configuration\n"s
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
        if (!os::daemonize(os::current_module_file(), cfpath.empty() ? "-s"s : "-c " + cfpath + " -s"s))
        {
            banner();
            os::fail("failed to daemonize");
            return 1;
        }
        else whoami = type::server;
    }

    banner();
    auto config = app::shared::load::settings(cfpath, os::legacy::get_setup());

    if (whoami == type::server)
    {
        auto userid = os::user();
        auto usernm = os::get_env("USER");
        auto hostip = os::get_env("SSH_CLIENT");
        auto prefix = utf::concat(DESKTOPIO_PREFIX, userid);
        auto server = os::ipc::open<os::server>(prefix);
        if (!server)
        {
            os::fail("can't start desktopio server");
            return 1;
        }
        auto srvlog = syslog.tee<events::try_sync>([](auto utf8) { SIGNAL_GLOBAL(e2::debug::logs, utf8); });
        config.cd("/config/appearance/defaults/");
        auto ground = base::create<hall>(server, config);
        auto thread = os::pool{};
        app::shared::activate(ground);

        log("main: listening socket ", server,
                         "\n\tuser: ", userid,
                         "\n\tpipe: ", prefix);

        while (auto client = server->meet())
        {
            if (!client->cred(userid))
            {
                os::fail("foreign users are not allowed to the session");
                continue;
            }

            thread.run([&, client](auto session_id)
            {
                if (auto window = ground->invite<gate>(client, session_id, config))
                {
                    log("user: new gate for ", client);
                    auto deskmenu = app::shared::create::builder(menuitem_t::type_Desk)("", utf::concat(window->id, ";", window->props.os_user_id, ";", window->props.selected), config);
                    auto bkground = app::shared::create::builder(menuitem_t::type_Fone)("", "gems; About; ", config);
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
                os::fail("no desktopio server connection");
                return 1;
            }

            auto runcfg = utf::base64(config.utf8());
            client->send(utf::concat(hostip, ";",
                                     usernm, ";",
                                     userid, ";",
                                     vtmode, ";",
                                     runcfg, ";"));
            auto cons = os::tty::proxy(client);
            auto size = cons.ignite(vtmode);
            if (size.last)
            {
                os::ipc::splice(cons, vtmode);
            }
        }
        else if (whoami == type::runapp)
        {
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
                params = DESKTOPIO_DEFAPP + " "s + params;
                log(DESKTOPIO_APPINF);
            }

            auto success = app::shared::start(params, DESKTOPIO_MYPATH, vtmode, config);
            if (!success)
            {
                os::fail("console initialization error");
                return 1;
            }
        }
    }
}