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
    auto banner = [&]{ log(prompt::vtm, app::shared::version); };
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
            auto syslog = os::tty::logger(true);
            log(app::shared::version);
            return 0;
        }
        else if (getopt.match("--onlylog"))
        {
            vtmode |= os::vt::onlylog;
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

    auto syslog = os::tty::logger();
    auto direct = !!(vtmode & os::vt::direct);
    banner();
    if (errmsg.size())
    {
        os::fail(errmsg);
        auto myname = os::process::binary<true>();
        log("\nVirtual terminal multiplexer with window manager and session sharing.\n\n"s
            + "  Syntax:\n\n    " + myname + " [ -c <file> ] [ -p <pipe> ] [ -q ] [ -l | -m | -d | -s | -r [<app> [<args...>]] ]\n"s
            + "\n"s
            + "  Options:\n\n"s
            + "    No arguments       Run client, auto start server if it is not running.\n"s
            + "    -c, --config <..>  Use specified configuration file.\n"s
            + "    -p, --pipe   <..>  Set the pipe to connect to.\n"s
            + "    -q, --quiet        Disable logging.\n"s
            + "    -l, --listconfig   Show configuration and exit.\n"s
            + "    -m, --monitor      Monitor server log.\n"s
            + "    -d, --daemon       Run server in background.\n"s
            + "    -s, --server       Run server in interactive mode.\n"s
            + "    -r, --runapp <..>  Run standalone application.\n"s
            + "    -v, --version      Show version and exit.\n"s
            + "    -?, -h, --help     Show usage message.\n"s
            + "    --onlylog          Disable interactive user input.\n"s
            + "\n"s
            + "  Configuration precedence (descending priority):\n\n"s
            + "    1. Command line options: " + myname + " -c path/to/settings.xml\n"s
            + "    2. Environment variable: "s + app::shared::env_config.substr(1) + "=path/to/settings.xml\n"s
            + "    3. Hardcoded location \""s  + app::shared::usr_config + "\"\n"s
            + "    4. Hardcoded configuration\n"s
            + "\n"s
            + "  Registered applications:\n\n"s
            + "    Term  Terminal emulator (default)\n"s
            + "    DTVT  DirectVT Proxy Console\n"s
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
        log(prompt::main, "Waiting for server...");
        while (true)
        {
            if (auto stream = os::ipc::socket::open<os::role::client, faux>(prefix))
            {
                log(prompt::main, "Connected");
                auto readln = std::thread{ [&]
                {
                    auto onlylog = vtmode & os::vt::onlylog;
                    if (faux && !onlylog)
                    {
                        os::tty::readline::ignite();
                        auto buffer = text{};
                        while (stream->send(os::tty::readline::readln(buffer))) { }
                        stream->shut();
                    }
                }};
                while (os::io::send(stream->recv()))
                { }
                os::tty::readline::finish();
                readln.join();
                return 0;
            }
            std::this_thread::sleep_for(500ms);
        }
    }
    else if (whoami == type::runapp)
    {
        auto config = app::shared::load::settings(defaults, cfpath, os::dtvt::config());
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
        log(apname, ' ', app::shared::version);
        params = utf::remain(params, ' ');

        auto [client, server] = os::ipc::xlink();
        auto thread = std::thread{[&, &client = client] //todo clang 15.0.0 still disallows capturing structured bindings (wait for clang 16.0.0)
        {
            os::tty::forward(client, aclass);
        }};
        //if (!config.cd("/config/" + aclass)) config.cd("/config/appearance/");
        config.cd("/config/appearance/runapp/", "/config/appearance/defaults/");
        auto domain = ui::base::create<ui::host>(server, config);
        auto applet = app::shared::builder(aclass)("", (direct ? "" : "!") + params, config, /*patch*/(direct ? ""s : "<config isolated=1/>"s)); // ! - means simple (i.e. w/o plugins)
        domain->invite(server, applet, vtmode);
        events::dequeue();
        domain->shutdown();
        thread.join();
    }
    else
    {
        auto userid = os::env::user();
        auto config = app::shared::load::settings(defaults, cfpath, os::dtvt::config());
        auto prefix = vtpipe.empty() ? utf::concat(app::shared::desktopio, '_', userid) : vtpipe;

        if (whoami == type::client)
        {
            auto client = os::ipc::socket::open<os::role::client>(prefix, 10s, [&]
            {
                log(prompt::main, "New vtm session for ", userid);
                auto success = faux;
                if (os::process::fork(success, prefix, config.utf8()))
                {
                    vtmode |= os::vt::onlylog;
                    whoami = type::server;
                }
                return success;
            });
            if (client)
            {
                os::tty::globals().wired.init.send(client, userid, vtmode, config.utf8());;
                os::tty::forward(client, app::vtm::id);
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
                vtmode |= os::vt::onlylog;
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
        auto logger = os::ipc::socket::open<os::role::server>(prefix + app::shared::logsuffix);
        if (!logger)
        {
            os::fail("Can't start vtm logger");
            return 1;
        }
        using e2 = netxs::ui::e2;
        config.cd("/config/appearance/defaults/");
        auto domain = ui::base::create<app::vtm::hall>(server, config, app::shell::id);
        auto srvlog = netxs::logger{ events::synced(domain, [&](auto utf8) { domain->SIGNAL(tier::general, e2::conio::logs, utf8); }) };
        auto thread = os::process::pool{};
        domain->autorun();

        log(prompt::main, "Server started",
          "\n      user: ", userid,
          "\n      pipe: ", prefix);

        auto stdlog = std::thread{ [&]
        {
            while (auto monitor = logger->meet())
            {
                thread.run([&, monitor](auto session_id)
                {
                    auto id = utf::concat(*monitor);
                    log(prompt::logs, "Monitor connected ", id);
                    auto tokens = subs{};
                    domain->LISTEN(tier::general, e2::conio::quit, utf8, tokens) { monitor->shut(); };
                    domain->LISTEN(tier::general, e2::conio::logs, utf8, tokens) { monitor->send(utf8); };
                    monitor->recv();
                    while (auto line = monitor->recv())
                    {
                        domain->SIGNAL(tier::release, e2::conio::readline, line);
                    }
                    log(prompt::logs, "Monitor disconnected ", id);
                });
            }
        }};

        auto settings = config.utf8();
        auto readln = std::thread{ [&]
        {
            auto onlylog = vtmode & os::vt::onlylog;
            if (faux && !onlylog)
            {
                os::tty::readline::ignite();
                auto buffer = text{};
                while (auto line = os::tty::readline::readln(buffer))
                {
                    domain->SIGNAL(tier::release, e2::conio::readline, line);
                }
            }
        }};

        while (auto client = server->meet())
        {
            if (client->auth(userid))
            {
                thread.run([&, client, settings](auto session_id)
                {
                    auto id = utf::concat(*client);
                    log(prompt::user, "Client connected ", id);
                    auto config = xmls{ settings };
                    auto packet = os::tty::globals().wired.init.recv(client);
                    config.fuse(packet.config);
                    domain->invite(client, packet.user, packet.mode, config, session_id);
                    log(prompt::user, "Client disconnected ", id);
                });
            }
        }
        os::tty::readline::finish();
        readln.join();
        logger->stop(); // Logger must be stopped first to prevent reconnection.
        domain->SIGNAL(tier::general, e2::conio::quit, msg, (utf::concat(prompt::main, "Server shutdown")));
        events::dequeue();
        domain->shutdown();
        stdlog.join();
    }
}