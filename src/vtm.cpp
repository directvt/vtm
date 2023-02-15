// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#include "vtm.hpp"
#include "netxs/apps.hpp"

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

    auto vtmode = os::tty::vtmode();
    auto banner = [&]{ log(app::vtm::desc, ' ', app::shared::version); };
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
                errmsg = "custom pipe not specified";
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
                errmsg = "config file path not specified";
                break;
            }
        }
        else if (getopt.match("-?", "-h", "--help"))
        {
            errmsg = ansi::nil().add("show help message");
            break;
        }
        else if (getopt.match("-v", "--version"))
        {
            auto syslog = os::tty::logger(vtmode, true);
            log(app::shared::version);
            return 0;
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

    auto syslog = os::tty::logger(vtmode);
    banner();
    if (errmsg.size())
    {
        os::fail(errmsg);
        auto myname = os::process::binary<true>();
        log("\nTerminal multiplexer with window manager and session sharing.\n\n"s
            + "  Syntax:\n\n    " + myname + " [ -c <file> ] [ -p <pipe> ] [ -q ] [ -l | -m | -d | -s | -r [<app> [<args...>]] ]\n"s
            + "\n"s
            + "  Options:\n\n"s
            + "    No arguments        Run client, auto start server if it is not running.\n"s
            + "    -c | --config <..>  Use specified configuration file.\n"s
            + "    -p | --pipe   <..>  Set the pipe to connect to.\n"s
            + "    -q | --quiet        Disable logging.\n"s
            + "    -l | --listconfig   Show configuration and exit.\n"s
            + "    -m | --monitor      Monitor server log.\n"s
            + "    -d | --daemon       Run server in background.\n"s
            + "    -s | --server       Run server in interactive mode.\n"s
            + "    -r | --runapp <..>  Run standalone application.\n"s
            + "    -v | --version      Show version and exit.\n"s
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
        log("Running configuration:\n", app::shared::load::settings<true>(defaults, cfpath, os::dtvt::config()));
    }
    else if (whoami == type::logger)
    {
        auto userid = os::env::user();
        auto prefix = (vtpipe.empty() ? utf::concat(app::shared::desktopio, '_', userid) : vtpipe) + app::shared::logsuffix;
        log("main: waiting for server...");
        while (true)
        {
            if (auto stream = os::ipc::socket::open<os::client, faux>(prefix))
            {
                log("main: connected");
                while (os::io::send(stream->recv()))
                { }
                return 0;
            }
            std::this_thread::sleep_for(500ms);
        }
    }
    else if (whoami == type::runapp)
    {
        auto config = app::shared::load::settings(defaults, cfpath, os::dtvt::config());
        auto shadow = params;
        utf::to_low(shadow);
             if (shadow.starts_with(app::term::id))      log(app::term::desc,      ' ', app::shared::version);
        else if (shadow.starts_with(app::calc::id))      log(app::calc::desc,      ' ', app::shared::version);
        else if (shadow.starts_with(app::shop::id))      log(app::shop::desc,      ' ', app::shared::version);
        else if (shadow.starts_with(app::test::id))      log(app::test::desc,      ' ', app::shared::version);
        else if (shadow.starts_with(app::textancy::id))  log(app::textancy::desc,  ' ', app::shared::version);
        else if (shadow.starts_with(app::headless::id))  log(app::headless::desc,  ' ', app::shared::version);
        else if (shadow.starts_with(app::settings::id))  log(app::settings::desc,  ' ', app::shared::version);
        else if (shadow.starts_with(app::truecolor::id)) log(app::truecolor::desc, ' ', app::shared::version);
        else
        {
            params = app::term::id + " "s + params;
            log(app::term::desc, ' ', app::shared::version);
        }

        auto success = app::shared::start(params, app::vtm::id, vtmode, config);
        if (!success)
        {
            os::fail("console initialization error");
            return 1;
        }
    }
    else
    {
        auto userid = os::env::user();
        auto config = app::shared::load::settings(defaults, cfpath, os::dtvt::config());
        auto prefix = vtpipe.empty() ? utf::concat(app::shared::desktopio, '_', userid) : vtpipe;

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
                os::tty::globals().wired.init.send(client, userid, vtmode, config.utf8());;
                if (vtmode & os::vt::direct)
                {
                    os::tty::direct(client);
                }
                else
                {
                    os::logging::start(app::vtm::id);
                    os::tty::splice(client, vtmode);
                }
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
        auto logger = os::ipc::socket::open<os::server>(prefix + app::shared::logsuffix);
        if (!logger)
        {
            os::fail("can't start desktopio logger");
            return 1;
        }
        using e2 = netxs::ui::e2;
        config.cd("/config/appearance/defaults/");
        auto domain = ui::base::create<app::vtm::hall>(server, config, app::shell::id);
        auto srvlog = syslog.tee<events::try_sync>([&](auto utf8) { domain->SIGNAL(tier::general, e2::conio::logs, utf8); });
        auto thread = os::process::pool{};
        domain->autorun();

        log("main: listening socket ", server,
          "\n      user: ", userid,
          "\n      pipe: ", prefix);

        auto stdlog = std::thread{ [&]
        {
            while (auto stream = logger->meet())
            {
                thread.run([&, stream](auto session_id)
                {
                    log("logs: monitor ", stream, " connected");
                    auto tokens = subs{};
                    domain->LISTEN(tier::general, e2::conio::quit, utf8, tokens) { stream->shut(); };
                    domain->LISTEN(tier::general, e2::conio::logs, utf8, tokens) { stream->send(utf8); };
                    stream->recv();
                    log("logs: monitor ", stream, " disconnected");
                });
            }
        }};

        auto settings = config.utf8();
        while (auto client = server->meet())
        {
            if (client->auth(userid))
            {
                thread.run([&, client, settings](auto session_id)
                {
                    log("user: new gate for ", client);
                    auto config = xmls{ settings };
                    auto packet = os::tty::globals().wired.init.recv(client);
                    config.fuse(packet.config);
                    domain->invite(client, packet.user, packet.mode, config, session_id);
                    log("user: ", client, " logged out");
                });
            }
        }
        domain->SIGNAL(tier::general, e2::conio::quit, "main: server shutdown");
        domain->shutdown();
        logger->stop();
        stdlog.join();
    }
}