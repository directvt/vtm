// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#include "netxs/apps.hpp"
#include "netxs/apps/desk.hpp"
#include "vtm.hpp"
#include "netxs/apps/tile.hpp"

using namespace netxs;

enum class type
{
    client,
    server,
    daemon,
    runapp,
    config,
    logger,
};

int main(int argc, char* argv[])
{
    auto defaults = 
    #include "vtm.xml"

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
            params = getopt ? getopt.rest() : text{ app::term::id };
        }
        else if (getopt.match("-s", "--server"))
        {
            whoami = type::server;
        }
        else if (getopt.match("-d", "--daemon"))
        {
            whoami = type::daemon;
        }
        else if (getopt.match("-m", "--monitor"))
        {
            whoami = type::logger;
        }
        else if (getopt.match("-p", "--pipe"))
        {
            vtpipe = getopt.next();
            if (vtpipe.empty())
            {
                errmsg = "Custom pipe not specified";
                break;
            }
        }
        else if (getopt.match("-q", "--quiet"))
        {
            netxs::logger::enabled(faux);
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
                errmsg = "Config file path not specified";
                break;
            }
        }
        else if (getopt.match("-?", "-h", "--help"))
        {
            errmsg = ansi::nil().add("Show help message");
            break;
        }
        else if (getopt.match("-v", "--version"))
        {
            netxs::logger::wipe();
            auto syslog = os::tty::logger();
            log(app::shared::version);
            return 0;
        }
        else if (getopt.match("--onlylog"))
        {
            os::dtvt::vtmode |= ui::console::onlylog;
        }
        else if (getopt.match("--"))
        {
            break;
        }
        else
        {
            errmsg = utf::concat("Unknown option '", getopt.next(), "'");
            break;
        }
    }

    auto direct = os::dtvt::active;
    auto syslog = os::tty::logger();
    auto userid = os::env::user();
    auto prefix = vtpipe.length() ? vtpipe : utf::concat(app::shared::desktopio, os::process::elevated ? "!_" : "_", userid);;
    auto prefix_log = prefix + app::shared::logsuffix;

    log(prompt::vtm, app::shared::version);
    if (errmsg.size())
    {
        os::fail(errmsg);
        log("\n"
            "Virtual terminal multiplexer with window manager and session sharing.\n"
            "\n"
            "  Syntax:\n"
            "\n"
            "    " + os::process::binary<true>() + " [ -c <file> ] [ -p <pipe> ] [ -q ] [ -l | -m | -d | -s | -r [<app> [<args...>]] ]\n"
            "\n"
            "  Options:\n"
            "\n"
            "    No arguments       Run client, auto start server if it is not running.\n"
            "    -c, --config <..>  Load specified settings file.\n"
            "    -p, --pipe   <..>  Set the pipe to connect to.\n"
            "    -q, --quiet        Disable logging.\n"
            "    -l, --listconfig   Show configuration and exit.\n"
            "    -m, --monitor      Monitor server log.\n"
            "    -d, --daemon       Run server in background.\n"
            "    -s, --server       Run server in interactive mode.\n"
            "    -r, --runapp <..>  Run standalone application.\n"
            "    -v, --version      Show version and exit.\n"
            "    -?, -h, --help     Show usage message.\n"
            "    --onlylog          Disable interactive user input.\n"
            "\n"
            "  Settings loading and merging order:\n"
            "\n"
            "    - Initialize hardcoded settings\n"
            "    - Merge with explicitly specified settings from --config <file>\n"
            "    - If the --config option is not used or <file> cannot be loaded:\n"
            "        - Merge with system-wide settings from " + os::path::expand(app::shared::sys_config).second + "\n"
            "        - Merge with user-wise settings from "   + os::path::expand(app::shared::usr_config).second + "\n"
            "        - Merge with DirectVT packet received from the parent process (dtvt-mode only)\n"
            "\n"
            "  Registered applications:\n"
            "\n"
            "    Term  Terminal emulator (default)\n"
            "    DTVT  DirectVT Proxy Console\n"
            "    Text  (Demo) Text editor\n"
            "    Calc  (Demo) Spreadsheet calculator\n"
            "    Gems  (Demo) Application distribution hub\n"
            );
    }
    else if (whoami == type::config)
    {
        log(prompt::resultant_settings, "\n", app::shared::load::settings<true>(defaults, cfpath, os::dtvt::config));
    }
    else if (whoami == type::logger)
    {
        log("%%Waiting for server...", prompt::main);
        auto online = flag{ true };
        auto active = flag{ faux };
        auto stream = sptr<os::ipc::socket>{};
        auto readln = os::tty::readline([&](auto line)
        {
            if (active) stream->send(line);
            else log("%%No server connected", prompt::main);
        }, [&]
        {
            online.exchange(faux);
            if (active) stream->shut();
        });
        while (online)
        {
            stream = os::ipc::socket::open<os::role::client, faux>(prefix_log);
            if (stream)
            {
                log("%%Connected", prompt::main);
                stream->send(utf::concat(os::process::id.first));
                active.exchange(true);
                while (online)
                {
                    if (auto data = stream->recv()) log<faux>(data);
                    else online.exchange(faux);
                }
            }
            else os::sleep(500ms);
        }
    }
    else if (whoami == type::runapp)
    {
        auto config = app::shared::load::settings(defaults, cfpath, os::dtvt::config);
        auto shadow = params;
        auto apname = view{};
        auto aclass = text{};
        utf::to_low(shadow);
             if (shadow.starts_with(app::term::id))      { aclass = app::term::id;      apname = app::term::desc;      }
        else if (shadow.starts_with(app::dtvt::id))      { aclass = app::dtvt::id;      apname = app::dtvt::desc;      }
        else if (shadow.starts_with(app::calc::id))      { aclass = app::calc::id;      apname = app::calc::desc;      }
        else if (shadow.starts_with(app::shop::id))      { aclass = app::shop::id;      apname = app::shop::desc;      }
        else if (shadow.starts_with(app::test::id))      { aclass = app::test::id;      apname = app::test::desc;      }
        else if (shadow.starts_with(app::directvt::id))  { aclass = app::directvt::id;  apname = app::directvt::desc;  }
        else if (shadow.starts_with(app::textancy::id))  { aclass = app::textancy::id;  apname = app::textancy::desc;  }
        else if (shadow.starts_with(app::headless::id))  { aclass = app::headless::id;  apname = app::headless::desc;  }
        else if (shadow.starts_with(app::settings::id))  { aclass = app::settings::id;  apname = app::settings::desc;  }
        else if (shadow.starts_with(app::truecolor::id)) { aclass = app::truecolor::id; apname = app::truecolor::desc; }
        else
        {
            params = " "s + params;
            aclass = app::term::id;
            apname = app::term::desc;
        }
        log("%appname% %version%", apname, app::shared::version);
        params = utf::remain(params, ' ');
        app::shared::start(params, aclass, os::dtvt::vtmode, os::dtvt::win_sz, config);
    }
    else
    {
        auto config = app::shared::load::settings(defaults, cfpath, os::dtvt::config);
        if (whoami == type::client)
        {
            auto client = os::ipc::socket::open<os::role::client>(prefix, 10s, [&]
            {
                log("%%New vtm session for %userid%", prompt::main, userid);
                auto success = faux;
                if (os::process::fork(success, prefix, config.utf8()))
                {
                    os::dtvt::vtmode |= ui::console::onlylog;
                    whoami = type::server;
                }
                return success;
            });
            if (client)
            {
                os::tty::stream.init.send(client, userid, os::dtvt::vtmode, os::dtvt::win_sz, config.utf8());
                os::tty::splice(client);
                return 0;
            }
            else if (whoami != type::server)
            {
                os::fail("No vtm server connection");
                return 1;
            }
        }

        if (whoami == type::daemon)
        {
            auto success = faux;
            if (os::process::fork(success, prefix, config.utf8()))
            {
                os::dtvt::vtmode |= ui::console::onlylog;
                whoami = type::server;
            }
            else 
            {
                if (!success) os::fail("Failed to daemonize");
                return !success;
            }
        }
        
        auto server = os::ipc::socket::open<os::role::server>(prefix);
        if (!server)
        {
            os::fail("Can't start vtm server");
            return 1;
        }
        auto logger = os::ipc::socket::open<os::role::server>(prefix_log);
        if (!logger)
        {
            os::fail("Can't start vtm logger");
            return 1;
        }
        using e2 = netxs::ui::e2;
        config.cd("/config/appearance/defaults/");
        auto domain = ui::base::create<app::vtm::hall>(server, config, app::shell::id);
        domain->plugin<scripting::host>();
        domain->autorun();

        log("%%Server started"
          "\n      user: %userid%"
          "\n      pipe: %prefix%", prompt::main, userid, prefix);

        auto stdlog = std::thread{ [&]
        {
            while (auto monitor = logger->meet())
            {
                domain->run([&, monitor](auto session_id)
                {
                    auto id = monitor->recv().str();
                    log("%%Monitor [%id%] connected", prompt::logs, id);
                    auto tokens = subs{};
                    auto writer = netxs::logger::attach([&](auto utf8) { monitor->send(utf8); });
                    domain->LISTEN(tier::general, e2::conio::quit, deal, tokens) { monitor->shut(); };
                    //todo send/receive dtvt events and signals
                    while (auto line = monitor->recv())
                    {
                        domain->SIGNAL(tier::release, e2::conio::readline, line);
                    }
                    log("%%Monitor [%id%] disconnected", prompt::logs, id);
                });
            }
        }};

        auto settings = config.utf8();
        auto readline = os::tty::readline([&](auto line){ domain->SIGNAL(tier::release, e2::conio::readline, line); },
                                          [&]{ domain->SIGNAL(tier::general, e2::shutdown, msg, (utf::concat(prompt::main, "Shutdown on signal"))); });
        while (auto client = server->meet())
        {
            if (client->auth(userid))
            {
                domain->run([&, client, settings](auto session_id)
                {
                    auto id = utf::concat(*client);
                    if constexpr (debugmode) log("%%Client connected %id%", prompt::user, id);
                    auto config = xmls{ settings };
                    auto packet = os::tty::stream.init.recv(client);
                    config.fuse(packet.config);
                    domain->invite(client, packet.user, packet.mode, packet.winsz, config, session_id);
                    if constexpr (debugmode) log("%%Client disconnected %id%", prompt::user, id);
                });
            }
        }
        readline.stop();
        logger->stop(); // Monitor listener endpoint must be closed first to prevent reconnections.
        stdlog.join();
        domain->stop();
    }

    os::release();
}