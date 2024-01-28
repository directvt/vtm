// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#include "netxs/apps.hpp"
#include "netxs/apps/desk.hpp"
#include "vtm.hpp"
#include "netxs/apps/tile.hpp"

using namespace netxs;

enum class type { client, server, daemon, logger, runapp, config };
enum class code { noaccess, noserver, nodaemon, nologger, interfer, errormsg };

int main(int argc, char* argv[])
{
    auto defaults = 
    #include "vtm.xml"

    auto whoami = type::client;
    auto params = text{};
    auto cfpath = text{};
    auto errmsg = text{};
    auto vtpipe = text{};
    auto script = text{};
    auto getopt = os::process::args{ argc, argv };
    if (getopt.starts(app::ssh::id))
    {
        whoami = type::runapp;
        params = getopt.rest();
    }
    else while (getopt)
    {
        if (getopt.match("--svc"))
        {
            auto ok = os::process::dispatch();
            return ok ? 0 : 1;
        }
        else if (getopt.match("-r", "--runapp"))
        {
            whoami = type::runapp;
            if (getopt) params = getopt.rest();
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
        else if (getopt.match("-u", "--uninstall"))
        {
            os::dtvt::initialize();
            netxs::logger::wipe();
            auto syslog = os::tty::logger();
            auto ok = os::process::uninstall();
            if (ok) log("%vtm% is uninstalled.", app::vtm::id);
            return ok ? 0 : 1;
        }
        else if (getopt.match("-i", "--install"))
        {
            os::dtvt::initialize();
            netxs::logger::wipe();
            auto syslog = os::tty::logger();
            auto ok = os::process::install();
            if (ok) log("%vtm% %ver% is installed.", app::vtm::id, app::shared::version);
            return ok ? 0 : 1;
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
            errmsg = ansi::nil().add("Print command-line options");
            break;
        }
        else if (getopt.match("-v", "--version"))
        {
            os::dtvt::initialize();
            netxs::logger::wipe();
            auto syslog = os::tty::logger();
            log(app::shared::version);
            return 0;
        }
        else if (getopt.match("--onlylog"))
        {
            os::dtvt::vtmode |= ui::console::onlylog;
        }
        else if (getopt.match("--script"))
        {
            script = xml::unescape(getopt.next());
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

    os::dtvt::initialize();
    os::dtvt::checkpoint();

    if (os::dtvt::vtmode & ui::console::redirio
     && (whoami == type::runapp || whoami == type::client))
    {
        whoami = type::logger;
    }
    auto denied = faux;
    auto syslog = os::tty::logger();
    auto userid = os::env::user();
    auto prefix = vtpipe.length() ? vtpipe : utf::concat(os::path::ipc_prefix, os::process::elevated ? "!-" : "-", userid.second);;
    auto prefix_log = prefix + os::path::log_suffix;
    auto failed = [&](auto cause)
    {
        os::fail(cause == code::noaccess ? "Access denied"
               : cause == code::interfer ? "Server already running"
               : cause == code::noserver ? "Failed to start server"
               : cause == code::nologger ? "Failed to start logger"
               : cause == code::nodaemon ? "Failed to daemonize"
               : cause == code::errormsg ? errmsg.c_str()
                                         : "");
        return 1;
    };

    log(prompt::vtm, app::shared::version);
    log(getopt.show());
    if (errmsg.size())
    {
        failed(code::errormsg);
        log("\nText-based desktop environment " + text{ app::shared::version } +
            "\n"
            "\n  Syntax:"
            "\n"
            "\n    " + os::process::binary<true>() + " [ -c <file> ] [ -p <pipe> ] [ -i | -u ] [ -q ] [ -l | -m | -d | -s | -r [<app> [<args...>]] ]"
            "\n"
            "\n  Options:"
            "\n"
            "\n    No arguments         Connect to the desktop (autostart new if not running)."
            "\n    -c, --config <file>  Load the specified settings file."
            "\n    -p, --pipe <name>    Specify the desktop session connection point."
            "\n    -q, --quiet          Disable logging."
            "\n    -l, --listconfig     Print configuration."
            "\n    -m, --monitor        Run desktop session log monitor."
            "\n    -d, --daemon         Run desktop server in background."
            "\n    -s, --server         Run desktop server in interactive mode."
            "\n    -r, --runapp <args>  Run the specified application in standalone mode."
            "\n    -i, --install        System-wide installation."
            "\n    -u, --uninstall      System-wide deinstallation."
            "\n    -v, --version        Print version."
            "\n    -?, -h, --help       Print command-line options."
            "\n    --onlylog            Disable interactive user input for desktop server."
            "\n    --script <body>      Run the specified script on ready."
            "\n"
            "\n  Settings loading order:"
            "\n"
            "\n    - Initialize hardcoded settings"
            "\n    - Merge with explicitly specified settings from --config <file>"
            "\n    - If the --config option is not used or <file> cannot be loaded:"
            "\n        - Merge with system-wide settings from " + os::path::expand(app::shared::sys_config).second +
            "\n        - Merge with user-wise settings from "   + os::path::expand(app::shared::usr_config).second +
            "\n        - Merge with DirectVT packet received from the parent process (dtvt-mode)"
            "\n"
            "\n  Built-in applications:"
            "\n"
            "\n    Term      Terminal emulator to run cli applications.       'vtm -r term [cli_application]'"
            "\n    Headless  Terminal emulator without UI.                    'vtm -r headless [cli_application]'"
            "\n    DTVT      DirectVT proxy to run dtvt-apps in text console. 'vtm -r dtvt [dtvt_application]'"
            "\n    XLVT      DTVT with controlling terminal.                  'vtm -r xlvt ssh <user@host dtvt_application>'"
            "\n"
            "\n  The following commands have a short form:"
            "\n"
            "\n    'vtm -r xlvt ssh <user@host dtvt_application>' can be shortened to 'vtm ssh <user@host dtvt_application>'."
            "\n    'vtm -r headless [cli_application]' can be shortened to 'vtm -r [cli_application]'."
            "\n"
            "\n  Usage examples:"
            "\n"
            "\n    Run the built-in terminal inside the current console:"
            "\n"
            "\n        vtm -r [term]"
            "\n"
            "\n    Run the application inside the built-in terminal:"
            "\n"
            "\n        vtm -r [term] </path/to/console/app>"
            "\n"
            "\n    Run the console application remotely via SSH:"
            "\n"
            "\n        vtm ssh <user@server> vtm -r [term] </path/to/console/app>"
            "\n"
            "\n    Run vtm desktop:"
            "\n"
            "\n        vtm"
            "\n"
            "\n    Run vtm desktop remotely via SSH:"
            "\n"
            "\n        vtm ssh <user@server> vtm"
            "\n"
            "\n    Run vtm desktop and reconfigure the taskbar menu:"
            "\n"
            "\n        vtm --script \"vtm.del(); vtm.set(splitter id=Apps); vtm.set(id=Term)\""
            "\n"
            "\n    Reconfigure the taskbar of the running desktop:"
            "\n"
            "\n        echo \"vtm.del(); vtm.set(splitter id=Apps); vtm.set(id=Term)\" | vtm"
            "\n"
            "\n    Run a terminal window in the running desktop session:"
            "\n"
            "\n        echo \"vtm.run()\" | vtm"
            "\n        echo \"vtm.dtvt(vtm -r term)\" | vtm"
            "\n"
            "\n    Run tiling window manager with three terminals on board:"
            "\n"
            "\n        echo \"vtm.run(type=group title=Terminals cmd='v(h(Term,Term),Term)')\" | vtm"
            "\n"
            "\n    Run an application window in the running desktop session:"
            "\n"
            "\n        echo \"vtm.run(title='Console \\nApplication' cmd=</path/to/app>)\" | vtm"
            "\n"
            "\n    Terminate the running desktop session:"
            "\n"
            "\n        echo \"vtm.shutdown()\" | vtm"
            "\n"
            );
    }
    else if (whoami == type::config)
    {
        log(prompt::resultant_settings, "\n", app::shared::load::settings<true>(defaults, cfpath, os::dtvt::config));
    }
    else if (whoami == type::logger)
    {
        log("%%Waiting for server...", prompt::main);
        auto result = std::atomic<int>{};
        auto events = os::tty::binary::logger{ [&](auto&, auto& reply)
        {
            if (reply.size() && os::dtvt::vtmode & ui::console::redirio)
            {
                os::io::send(reply);
            }
            --result;
        }};
        auto online = flag{ true };
        auto active = flag{ faux };
        auto locker = std::mutex{};
        auto syncio = std::unique_lock{ locker };
        auto buffer = std::list{ script };
        auto stream = sptr<os::ipc::socket>{};
        auto readln = os::tty::readline([&](auto line)
        {
            auto sync = std::lock_guard{ locker };
            if (active)
            {
                ++result;
                events.command.send(stream, line);
            }
            else
            {
                log("%%No server connected: %cmd%", prompt::main, utf::debase<faux, faux>(line));
                buffer.push_back(line);
            }
        }, [&]
        {
            auto sync = std::lock_guard{ locker };
            online.exchange(faux);
            if (active) while (result && active) std::this_thread::yield();
            if (active && stream) stream->shut();
        });
        while (online)
        {
            auto iolink = os::ipc::socket::open<os::role::client, faux>(prefix_log, denied);
            if (denied)
            {
                syncio.unlock();
                return failed(code::noaccess);
            }
            if (iolink)
            {
                std::swap(stream, iolink);
                result += 3;
                events.command.send(stream, utf::concat(os::process::id.first)); // First command is the monitor id.
                events.command.send(stream, os::env::add());
                events.command.send(stream, os::env::cwd());
                for (auto& line : buffer)
                {
                    ++result;
                    events.command.send(stream, line);
                }
                buffer.clear();
                active.exchange(true);
                syncio.unlock();
                directvt::binary::stream::reading_loop(stream, [&](view data){ events.s11n::sync(data); });
                syncio.lock();
                active.exchange(faux);
                break;
            }
            else
            {
                syncio.unlock();
                os::sleep(500ms);
                syncio.lock();
            }
        }
        syncio.unlock();
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
        else if (shadow.starts_with(app::xlvt::id))      { aclass = app::xlvt::id;      apname = app::xlvt::desc;      }
        else if (shadow.starts_with(app::xlinkvt::id))   { aclass = app::xlinkvt::id;   apname = app::xlinkvt::desc;   }
        else if (shadow.starts_with(app::directvt::id))  { aclass = app::directvt::id;  apname = app::directvt::desc;  }
        else if (shadow.starts_with(app::headless::id))  { aclass = app::headless::id;  apname = app::headless::desc;  }
        #if defined(DEBUG)
        else if (shadow.starts_with(app::calc::id))      { aclass = app::calc::id;      apname = app::calc::desc;      }
        else if (shadow.starts_with(app::shop::id))      { aclass = app::shop::id;      apname = app::shop::desc;      }
        else if (shadow.starts_with(app::test::id))      { aclass = app::test::id;      apname = app::test::desc;      }
        else if (shadow.starts_with(app::textancy::id))  { aclass = app::textancy::id;  apname = app::textancy::desc;  }
        else if (shadow.starts_with(app::settings::id))  { aclass = app::settings::id;  apname = app::settings::desc;  }
        else if (shadow.starts_with(app::truecolor::id)) { aclass = app::truecolor::id; apname = app::truecolor::desc; }
        #endif
        else if (shadow.starts_with(app::ssh::id))
        {
            params = " "s + params;
            aclass = app::xlvt::id;
            apname = app::xlvt::desc;
        }
        else
        {
            params = " "s + params;
            aclass = app::headless::id;
            apname = app::headless::desc;
        }
        log("%appname% %version%", apname, app::shared::version);
        params = utf::remain(params, ' ');
        app::shared::start(params, aclass, os::dtvt::vtmode, os::dtvt::win_sz, config);
    }
    else
    {
        auto config = app::shared::load::settings(defaults, cfpath, os::dtvt::config);
        auto client = os::ipc::socket::open<os::role::client, faux>(prefix, denied);
        auto signal = ptr::shared<os::fire>(os::process::started(prefix)); // Signaling that the server is ready for incoming connections.

             if (denied)                           return failed(code::noaccess);
        else if (whoami != type::client && client) return failed(code::interfer);
        else if (whoami == type::client && !client)
        {
            log("%%New vtm session for [%userid%]", prompt::main, userid.first);
            auto [success, successor] = os::process::fork(prefix, config.utf8());
            if (successor)
            {
                os::dtvt::vtmode |= ui::console::onlylog;
                whoami = type::server;
                script = {};
            }
            else
            {
                if (success) signal->wait(10s); // Waiting for confirmation of receiving the configuration.
                else         return failed(code::noserver);
            }
        }

        if (whoami == type::client)
        {
            signal.reset();
            if (client || (client = os::ipc::socket::open<os::role::client>(prefix, denied)))
            {
                auto userinit = directvt::binary::init{};
                auto env = os::env::add();
                auto cwd = os::env::cwd();
                auto cmd = script;
                auto cfg = config.utf8();
                auto win = os::dtvt::win_sz;
                userinit.send(client, userid.first, os::dtvt::vtmode, env, cwd, cmd, cfg, win);
                os::tty::splice(client);
                return 0;
            }
            else return failed(denied ? code::noaccess : code::noserver);
        }

        if (whoami == type::daemon)
        {
            auto [success, successor] = os::process::fork(prefix, config.utf8(), script);
            if (successor)
            {
                os::dtvt::vtmode |= ui::console::onlylog;
                whoami = type::server;
            }
            else 
            {
                if (success)
                {
                    signal->wait(10s); // Waiting for confirmation of receiving the configuration.
                    return 0;
                }
                else return failed(code::nodaemon);
            }
        }
        
        os::ipc::prefix = prefix;
        auto server = os::ipc::socket::open<os::role::server>(prefix, denied);
        if (!server)
        {
            if (denied) failed(code::noaccess);
            return      failed(code::noserver);
        }
        auto logger = os::ipc::socket::open<os::role::server>(prefix_log, denied);
        if (!logger)
        {
            if (denied) failed(code::noaccess);
            return      failed(code::nologger);
        }

        signal->bell(); // Signal we are started and ready for connections.
        signal.reset();

        using e2 = netxs::ui::e2;
        config.cd("/config/appearance/defaults/");
        auto domain = ui::base::create<app::vtm::hall>(server, config, app::shell::id);
        domain->plugin<scripting::host>();
        domain->autorun();

        log("%%Session started"
          "\n      user: %userid%"
          "\n      pipe: %prefix%", prompt::main, userid.first, prefix);

        auto stdlog = std::thread{ [&]
        {
            while (auto monitor = logger->meet())
            {
                domain->run([&, monitor](auto /*task_id*/)
                {
                    auto id = text{};
                    auto active = faux;
                    auto tokens = subs{};
                    auto onecmd = eccc{};
                    auto events = os::tty::binary::logger{ [&, init = 0](auto& events, auto& cmd) mutable
                    {
                        if (active)
                        {
                            onecmd.cmd = cmd;
                            domain->SIGNAL(tier::release, scripting::events::invoke, onecmd);
                        }
                        else
                        {
                                 if (init == 0) id = cmd;
                            else if (init == 1) onecmd.env = cmd;
                            else if (init == 2)
                            {
                                active = true;
                                onecmd.cwd = cmd;
                                log("%%Monitor [%id%] connected", prompt::logs, id);
                            }
                            init++;
                        }
                        events.command.send(monitor, onecmd.cmd);
                    }};
                    auto writer = netxs::logger::attach([&](auto utf8)
                    {
                        events.logs.send(monitor, ui32{}, datetime::now(), text{ utf8 });
                    });
                    domain->LISTEN(tier::general, e2::conio::quit, deal, tokens) { monitor->shut(); };
                    os::ipc::monitors++;
                    directvt::binary::stream::reading_loop(monitor, [&](view data){ events.s11n::sync(data); });
                    os::ipc::monitors--;
                    if (id.size()) log("%%Monitor [%id%] disconnected", prompt::logs, id);
                });
            }
        }};

        auto settings = config.utf8();
        auto execline = [&](qiew line){ domain->SIGNAL(tier::release, scripting::events::invoke, onecmd, ({ .cmd = line })); };
        auto shutdown = [&]{ domain->SIGNAL(tier::general, e2::shutdown, msg, (utf::concat(prompt::main, "Shutdown on signal"))); };
        execline(script);
        auto readline = os::tty::readline(execline, shutdown);
        while (auto user = server->meet())
        {
            if (user->auth(userid.second))
            {
                domain->run([&, user, settings](auto session_id)
                {
                    auto userinit = directvt::binary::init{};
                    if (auto packet = userinit.recv(user))
                    {
                        auto id = utf::concat(*user);
                        if constexpr (debugmode) log("%%Client connected %id%", prompt::user, id);
                        auto usrcfg = eccc{ .env = packet.env, .cwd = packet.cwd, .cmd = packet.cmd, .win = packet.win };
                        auto config = xmls{ settings };
                        config.fuse(packet.cfg);
                        domain->invite(user, packet.user, packet.mode, usrcfg, config, session_id);
                        if constexpr (debugmode) log("%%Client disconnected %id%", prompt::user, id);
                    }
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