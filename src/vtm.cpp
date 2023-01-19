// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#define DESKTOPIO_VER "v0.9.8n"
#define DESKTOPIO_MYNAME " vtm: " DESKTOPIO_VER
#define DESKTOPIO_PREFIX "desktopio_"
#define DESKTOPIO_MYPATH "vtm"
#define DESKTOPIO_DEFAPP "Term"
#define DESKTOPIO_APPINF "Desktopio Terminal " DESKTOPIO_VER

// Enable to show all terminal input (keyboard/mouse etc).
//#define KEYLOG

#include "netxs/apps.hpp"

using namespace netxs;
using namespace netxs::ui;

enum class type
{
    client,
    server,
    daemon,
    runapp,
    config,
};

int main(int argc, char* argv[])
{
    auto vtmode = os::tty::vtmode();
    auto syslog = os::tty::logger(vtmode);
    auto banner = [&]{ log(DESKTOPIO_MYNAME); };
    auto whoami = type::client;
    auto params = text{};
    auto cfpath = text{};
    auto errmsg = text{};
    auto vtpipe = text{};
    auto getopt = os::process::args{ argc, argv };
    while (getopt)
    {
        if (getopt.match("-r", "--runapp"))
        {
            whoami = type::runapp;
            params = getopt ? getopt.rest() : text{ DESKTOPIO_DEFAPP };
        }
        else if (getopt.match("-s", "--server"))
        {
            whoami = type::server;
        }
        else if (getopt.match("-d", "--daemon"))
        {
            whoami = type::daemon;
        }
        else if (getopt.match("-p", "--pipe"))
        {
            vtpipe = getopt.next();
            if (vtpipe.empty())
            {
                errmsg = "custom pipe not specified";
                break;
            }
        }
        else if (getopt.match("-l", "--listconfig"))
        {
            whoami = type::config;
        }
        else if (getopt.match("-c", "--config"))
        {
            cfpath = getopt.next();
            if (cfpath.empty())
            {
                errmsg = "config file path not specified";
                break;
            }
        }
        else if (getopt.match("-?", "-h", "--help"))
        {
            errmsg = ansi::nil().add("show help message");
            break;
        }
        else if (getopt.match("--"))
        {
            break;
        }
        else
        {
            errmsg = utf::concat("unknown option '", getopt.next(), "'");
            break;
        }
    }

    banner();
    if (errmsg.size())
    {
        os::fail(errmsg);
        auto myname = os::process::binary<true>();
        log("\nTerminal multiplexer with window manager and session sharing.\n\n"s
            + "  Syntax:\n\n    " + myname + " [ -c <file> ] [ -p <pipe> ] [ -l | -d | -s | -r [<app> [<args...>]] ]\n"s
            + "\n"s
            + "  Options:\n\n"s
            + "    No arguments        Run client, auto start server if it is not running.\n"s
            + "    -c | --config <..>  Use specified configuration file.\n"s
            + "    -p | --pipe   <..>  Set the pipe to connect to.\n"s
            + "    -l | --listconfig   Show configuration and exit.\n"s
            + "    -d | --daemon       Run server in background.\n"s
            + "    -s | --server       Run server in interactive mode.\n"s
            + "    -r | --runapp <..>  Run standalone application.\n"s
            + "    -? | -h | --help    Show usage message.\n"s
            + "\n"s
            + "  Configuration precedence (descending priority):\n\n"s
            + "    1. Command line options: " + myname + " -c path/to/settings.xml\n"s
            + "    2. Environment variable: "s + app::shared::env_config.substr(1) + "=path/to/settings.xml\n"s
            + "    3. Hardcoded location \""s  + app::shared::usr_config + "\"\n"s
            + "    4. Hardcoded configuration\n"s
            + "\n"s
            + "  Registered applications:\n\n"s
            + "    Term  Terminal emulator (default)\n"s
            + "    Text  (Demo) Text editor\n"s
            + "    Calc  (Demo) Spreadsheet calculator\n"s
            + "    Gems  (Demo) Desktopio application manager\n"s
            );
    }
    else if (whoami == type::config)
    {
        log("Running configuration:\n", app::shared::load::settings<true>(cfpath, os::dtvt::config()));
    }
    else if (whoami == type::runapp)
    {
        auto config = app::shared::load::settings(cfpath, os::dtvt::config());
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
    else
    {
        auto userid = os::env::user();
        auto usernm = os::env::get("USER");
        auto hostip = os::env::get("SSH_CLIENT");
        auto config = app::shared::load::settings(cfpath, os::dtvt::config());
        auto prefix = vtpipe.empty() ? utf::concat(DESKTOPIO_PREFIX, userid) : vtpipe;

        if (whoami == type::client)
        {
            auto client = os::ipc::socket::open<os::client>(prefix, 10s, [&]
            {
                log("main: new desktopio environment for ", userid);
                auto success = faux;
                if (os::process::fork(success, prefix, config.utf8())) whoami = type::server;
                return success;
            });
            if (client)
            {
                auto direct = !!(vtmode & os::vt::direct);
                if (!direct) os::logging::start(DESKTOPIO_MYPATH);
                auto init = directvt::binary::startdata_t{};
                init.set(hostip, usernm, utf::concat(userid), vtmode, config.utf8());
                init.send([&](auto& data){ client->send(data); });

                if (direct) os::tty::direct(client);
                else        os::tty::splice(client, vtmode);
                return 0;
            }
            else if (whoami != type::server)
            {
                os::fail("no desktopio server connection");
                return 1;
            }
        }

        if (whoami == type::daemon)
        {
            auto success = faux;
            if (os::process::fork(success, prefix, config.utf8()))
            {
                whoami = type::server;
            }
            else 
            {
                if (!success) os::fail("failed to daemonize");
                return !success;
            }
        }
        
        auto server = os::ipc::socket::open<os::server>(prefix);
        if (!server)
        {
            os::fail("can't start desktopio server");
            return 1;
        }
        using e2 = netxs::ui::e2;
        auto srvlog = syslog.tee<events::try_sync>([](auto utf8) { SIGNAL_GLOBAL(e2::debug::logs, utf8); });
        config.cd("/config/appearance/defaults/");
        auto ground = ui::base::create<ui::hall>(server, config);
        auto thread = os::process::pool{};
        app::shared::activate(ground, config);

        log("main: listening socket ", server,
          "\n      user: ", userid,
          "\n      pipe: ", prefix);

        while (auto client = server->meet())
        {
            if (!client->cred(userid))
            {
                os::fail("foreign users are not allowed to the session");
                continue;
            }

            thread.run([&, client](auto session_id)
            {
                if (auto window = ground->invite<ui::gate>(client, session_id, config))
                {
                    log("user: new gate for ", client);
                    auto patch = ""s;
                    auto deskmenu = app::shared::create::builder(ui::menuitem_t::type_Desk)("", utf::concat(window->id, ";", window->props.os_user_id, ";", window->props.selected), config, patch);
                    auto bkground = app::shared::create::builder(ui::menuitem_t::type_Fone)("", "gems;About;", config, patch);
                    window->launch(client, deskmenu, bkground);
                    log("user: ", client, " logged out");
                }
            });
        }

        SIGNAL_GLOBAL(e2::conio::quit, "main: server shutdown");
        ground->shutdown();
    }
}