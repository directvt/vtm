// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#define DESKTOPIO_VER "v0.9.8l"
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
    daemon,
    runapp,
    config,
};

int main(int argc, char* argv[])
{
    auto vtmode = os::vt_mode();
    auto syslog = os::ipc::logger(vtmode);
    auto banner = [&]{ log(DESKTOPIO_MYNAME); };
    auto whoami = type::client;
    auto params = text{};
    auto cfpath = text{};
    auto errmsg = text{};
    auto vtpipe = text{};
    auto getopt = os::args{ argc, argv };
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
        else if (getopt.match("-p", "--pipe"))
        {
            vtpipe = getopt.next();
            if (vtpipe.empty())
            {
                errmsg = "custom pipe not specified";
                break;
            }
        }
        else if (getopt.match("-?", "-h", "--help"))
        {
            errmsg = ansi::nil().add("show usage message");
            break;
        }
        else if (getopt.match("--"))
        {
            break;
        }
        else
        {
            errmsg = utf::concat("unknown command line parameter '", getopt.next(), "'");
            break;
        }
    }

    banner();
    if (errmsg.size())
    {
        os::fail(errmsg);
        auto myname = os::current_module_file<true>();
        log("\nUsage:\n\n " + myname + " [ -c <file> ] [ -p <pipe> ] [ -l | -d | -s | -r [<app> [<args...>]] ]\n"s
            + "\n"s
                + "\tNo arguments        Run client, auto start server if it is not running.\n"s
                + "\t-c | --config <..>  Use specified configuration file.\n"s
                + "\t-p | --pipe   <..>  Set the pipe to connect to.\n"s
                + "\t-l | --listconfig   Show configuration and exit.\n"s
                + "\t-d | --daemon       Run server in background.\n"s
                + "\t-s | --server       Run server in interactive mode.\n"s
                + "\t-r | --runapp <..>  Run standalone application.\n"s
                + "\t-? | -h | --help    Show usage message.\n"s
                + "\n"s
                + "\tConfiguration file location precedence (descending priority):\n\n"s
                + "\t\t1. Command line options; e.g., " + myname + " -c path/to/settings.xml\n"s
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
    }
    else if (whoami == type::config)
    {
        log("Running configuration:\n", app::shared::load::settings<true>(cfpath, os::legacy::get_setup()));
    }
    else if (whoami == type::runapp)
    {
        auto config = app::shared::load::settings(cfpath, os::legacy::get_setup());
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
        auto userid = os::user();
        auto usernm = os::env::get("USER");
        auto hostip = os::env::get("SSH_CLIENT");
        auto config = app::shared::load::settings(cfpath, os::legacy::get_setup());
        auto prefix = vtpipe.empty() ? utf::concat(DESKTOPIO_PREFIX, userid) : vtpipe;

        if (whoami == type::client)
        {
            auto direct = !!(vtmode & os::legacy::direct);
            if (!direct) os::start_log(DESKTOPIO_MYPATH);
            auto client = os::ipc::open<os::client>(prefix, 10s, [&]
                        {
                            log("main: new desktopio environment for user ", userid);
                            auto cmdarg = utf::concat(os::current_module_file(), " ",
                                                      vtpipe.size() ? "-p " + vtpipe + " " : ""s,
                                                      cfpath.size() ? "-c " + cfpath + " " : ""s, "-d");
                            //todo use fork for POSIX
                            return os::exec(cmdarg); //todo win32 pass config
                        });
            if (!client)
            {
                os::fail("no desktopio server connection");
                return 1;
            }
            auto init = ansi::dtvt::binary::startdata_t{};
            init.set(hostip, usernm, utf::concat(userid), vtmode, config.utf8());
            init.send([&](auto& data){ client->send(data); });

            if (direct) os::tty::direct(client);
            else
            {
                auto size = os::tty::ignite(vtmode, client);
                if (size.last)
                {
                    os::tty::splice(vtmode);
                }
            }
        }
        else // type::server/daemon 
        {
            //todo win32: load parent config
            if (whoami == type::daemon)
            {
                auto cmdarg = utf::concat("-p ", prefix, " -s");
                if (!os::daemonize(cmdarg)) //todo win32: pass config
                {
                    os::fail("failed to daemonize");
                    return 1;
                }
            }
            
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
            app::shared::activate(ground, config);

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
                        auto patch = ""s;
                        auto deskmenu = app::shared::create::builder(menuitem_t::type_Desk)("", utf::concat(window->id, ";", window->props.os_user_id, ";", window->props.selected), config, patch);
                        auto bkground = app::shared::create::builder(menuitem_t::type_Fone)("", "gems;About;", config, patch);
                        window->launch(client, deskmenu, bkground);
                        log("user: ", client, " logged out");
                    }
                });
            }

            SIGNAL_GLOBAL(e2::conio::quit, "main: server shutdown");
            ground->shutdown();
        }
    }
}