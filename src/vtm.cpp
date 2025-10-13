// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#include "netxs/apps.hpp"
#include "netxs/apps/desk.hpp"
#include "vtm.hpp"
#include "netxs/apps/tile.hpp"

using namespace netxs;

enum class type { client, server, daemon, logmon, runapp, config };
enum class code { noaccess, noserver, nodaemon, nosrvlog, interfer, errormsg };

int main(int argc, char* argv[])
{
    auto whoami = type::client;
    auto params = text{};
    auto cliopt = text{};
    auto errmsg = text{};
    auto vtpipe = text{};
    auto script = text{};
    auto rungui = true;
    auto system = faux;
    auto getopt = os::process::args{ argc, argv };
    if (getopt.starts("ssh"))
    {
        whoami = type::runapp;
        params = getopt.rest();
    }
    else while (getopt)
    {
        if (getopt.match("--cwd"))
        {
            auto path = getopt.next();
            if (path.size())
            {
                if (os::env::cwd(path)) log("%%Set current working directory to '%path%'", prompt::os, path);
                else                    log("%%Failed to set current working directory to '%path%'", prompt::os, ansi::err(path));
            }
        }
        else if (getopt.match("--env"))
        {
            auto var_val = getopt.next();
            if (var_val.size())
            {
                log("%%Set environment variable '%var_val%'", prompt::os, var_val);
                os::env::set(var_val);
            }
        }
        else if (getopt.match("--svc"))
        {
            auto ok = os::process::dispatch();
            return ok ? 0 : 1;
        }
        else if (getopt.match("-0", "--session0"))
        {
            system = true;
        }
        else if (getopt.match("-t", "--tui"))
        {
            rungui = faux;
        }
        else if (getopt.match("-g", "--gui"))
        {
            rungui = true;
        }
        else if (getopt.match("-r", "--", "--run"))
        {
            whoami = type::runapp;
            params = getopt.rest();
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
            whoami = type::logmon;
        }
        else if (getopt.match("-p", "--pin"))
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
            cliopt = getopt.next();
            if (cliopt.empty())
            {
                errmsg = "Config file path not specified";
                break;
            }
        }
        #if defined(__linux__) && !defined(__ANDROID__)
        else if (getopt.match("-a", "--mouse"))
        {
            auto enabled = xml::take_or(getopt.next(), true);
            os::dtvt::initialize();
            netxs::logger::wipe();
            auto syslog = os::tty::logger();
            auto ok = lixx::set_mouse_access(enabled);
            return ok;
        }
        #endif
        else if (getopt.match("-?", "-h", "--help"))
        {
            os::dtvt::initialize();
            netxs::logger::wipe();
            auto syslog = os::tty::logger();
            log("\nText-based Desktop Environment " + text{ app::shared::version } +
                "\n(virtual terminal multiplexer)"
                "\n"
                "\n  Command-line options syntax:"
                "\n"
                "\n    vtm [ -c <file> ][ -q ][ -p <id> ][ -s | -d | -m ][ -x <cmds> ]"
                "\n    vtm [ -c <file> ][ -q ][ -t | -g ][ -r [ <type> ]][ <args...> ]"
                "\n    vtm [ -c <file> ]  -l"
                #if defined(__linux__) && !defined(__ANDROID__)
                "\n    vtm -i | -u | -a [mode] | -v | -?"
                #else
                "\n    vtm -i | -u | -v | -?"
                #endif
                "\n"
                "\n    <script relay via piped redirection> | vtm [ -p <id> ]"
                "\n"
                "\n  Options:"
                "\n"
                "\n    Without options, vtm runs Desktop Server and Desktop Client."
                "\n"
                "\n    -h, -?, --help       Print command-line options."
                "\n    -v, --version        Print version."
                "\n    -l, --listconfig     Print configuration."
                "\n    -t, --tui            Force TUI mode."
                "\n    -g, --gui            Force GUI mode."
                "\n    -i, --install        Perform system-wide installation."
                #if defined(WIN32)
                " Allow Desktop Server to run in Session 0."
                #endif
                "\n    -u, --uninstall      Perform system-wide deinstallation."
                #if defined(WIN32)
                "\n    -0, --session0       Use Session 0 to run Desktop Server in background."
                #endif
                #if defined(__linux__) && !defined(__ANDROID__)
                "\n    -a, --mouse [mode]   Set/reset persistent access to mouse devices for all users."
                "\n                         Run 'sudo vtm --mouse 0' to reset access."
                #endif
                "\n    -q, --quiet          Disable logging."
                "\n    -x, --script <cmds>  Specifies script commands."
                "\n    -c, --config <file>  Specifies a settings file to load or plain xml-data to overlay."
                "\n    -p, --pin <id>       Specifies the desktop id it will be pinned to."
                "\n    -s, --server         Run Desktop Server."
                "\n    -d, --daemon         Run Desktop Server in background."
                "\n    -m, --monitor        Run Log Monitor."
                "\n    -r, --, --run        Run desktop applet standalone."
                "\n    <type>               Desktop applet to run."
                "\n    <args...>            Desktop applet arguments."
                "\n    --env <var=val>      Set environment variable."
                "\n    --cwd <path>         Set current working directory."
                "\n"
                "\n    Desktop applet             │ Type │ Arguments"
                "\n    ───────────────────────────┼──────┼─────────────────────────────────────────────────"
                "\n    Teletype Console (default) │ vtty │ CUI application with arguments to run."
                "\n    Terminal Console           │ term │ CUI application with arguments to run."
                "\n    DirectVT Gateway           │ dtvt │ DirectVT-aware application to run."
                "\n    DirectVT Gateway with TTY  │ dtty │ CUI application to run, forwarding DirectVT I/O."
                "\n"
                "\n    The following commands have a short form:"
                "\n"
                "\n      'vtm -r vtty <cui_app...>' can be shortened to 'vtm <cui_app...>'."
                "\n      'vtm -r dtty ssh <user@host dtvt_app...>' can be shortened to 'vtm ssh <user@host dtvt_app...>'."
                "\n"
                "\n    Plain xml-data can be specified in place of <file> in the '--config <file>' option,"
                "\n    as well as in the $VTM_CONFIG environment variable:"
                "\n"
                "\n      vtm -c \"<config><terminal><scrollback size=1000000/></terminal></config>\" -r term"
                "\n      or (using compact syntax)"
                "\n      vtm -c \"<config/terminal/scrollback size=1000000/>\" -r term"
                "\n");
            return 0;
        }
        else if (getopt.match("-v", "--version"))
        {
            os::dtvt::initialize();
            netxs::logger::wipe();
            auto syslog = os::tty::logger();
            log(app::shared::version);
            return 0;
        }
        else if (getopt.match("-x", "--script"))
        {
            script = getopt.next();
        }
        else
        {
            params = getopt.rest(); // params can't be empty at this point (see utf::quote()).
            if (params.front() == '-') errmsg = utf::concat("Unknown option '", params, "'");
            else                       whoami = type::runapp;
        }
    }

    auto interactive = whoami == type::runapp || whoami == type::client;
    os::dtvt::initialize(rungui, true, interactive);

    if (os::dtvt::vtmode & ui::console::redirio && (whoami == type::runapp || whoami == type::client))
    {
        whoami = type::logmon;
    }
    auto denied = faux;
    auto syslog = os::tty::logger();
    auto userid = os::env::user();
    auto prefix = vtpipe.length() ? vtpipe : utf::concat(os::path::ipc_prefix, os::process::elevated ? "!-" : "-", userid.second);
    auto prefix_log = prefix + os::path::log_suffix;
    auto failed = [&](auto cause)
    {
        os::fail(cause == code::noaccess ? "Access denied"
               : cause == code::interfer ? "Server already running"
               : cause == code::noserver ? "Failed to start server"
               : cause == code::nosrvlog ? "Failed to start session monitor"
               : cause == code::nodaemon ? "Failed to daemonize"
               : cause == code::errormsg ? errmsg.c_str()
                                         : "");
        return 1;
    };

    log(prompt::vtm, app::shared::version);
    log(getopt.show());
    if (errmsg.size())
    {
        return failed(code::errormsg);
    }
    else if (whoami == type::config)
    {
        auto config = xml::settings{};
        app::shared::load::settings(config, cliopt, true);
        log(prompt::resultant_settings, "\n", config);
    }
    else if (whoami == type::logmon)
    {
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
        auto logmsg = true;
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
                if (logmsg)
                {
                    log("%%Waiting for server...", prompt::main);
                    logmsg = faux;
                }
                os::sleep(500ms);
                syncio.lock();
            }
        }
        syncio.unlock();
    }
    else if (whoami == type::runapp)
    {
        auto& indexer = ui::tui_domain();
        app::shared::load::settings(indexer.config, cliopt);
        auto shadow = params;
        auto apname = view{};
        auto aptype = text{};
        utf::to_lower(shadow);
             if (shadow.starts_with(app::vtty::id))      { aptype = app::teletype::id;  apname = app::teletype::name;  }
        else if (shadow.starts_with(app::term::id))      { aptype = app::terminal::id;  apname = app::terminal::name;  }
        else if (shadow.starts_with(app::dtvt::id))      { aptype = app::dtvt::id;      apname = app::dtvt::name;      }
        else if (shadow.starts_with(app::dtty::id))      { aptype = app::dtty::id;      apname = app::dtty::name;      }
        //#if defined(DEBUG)
        else if (shadow.starts_with(app::calc::id))      { aptype = app::calc::id;      apname = app::calc::name;      }
        else if (shadow.starts_with(app::shop::id))      { aptype = app::shop::id;      apname = app::shop::name;      }
        else if (shadow.starts_with(app::test::id))      { aptype = app::test::id;      apname = app::test::name;      }
        else if (shadow.starts_with(app::empty::id))     { aptype = app::empty::id;     apname = app::empty::name;     }
        else if (shadow.starts_with(app::strobe::id))    { aptype = app::strobe::id;    apname = app::strobe::name;    }
        else if (shadow.starts_with(app::textancy::id))  { aptype = app::textancy::id;  apname = app::textancy::name;  }
        else if (shadow.starts_with(app::truecolor::id)) { aptype = app::truecolor::id; apname = app::truecolor::name; }

        else if (shadow.starts_with(app::app1::id)) { aptype = app::app1::id; apname = app::app1::name; }
        //#endif
        else if (shadow.starts_with("ssh"))//app::ssh::id))
        {
            params = " "s + params;
            aptype = app::dtty::id;
            apname = app::dtty::name;
        }
        else
        {
            params = " "s + params;
            aptype = app::teletype::id;
            apname = app::teletype::name;
        }
        log("%appname% %version%", apname, app::shared::version);
        auto coor = params.find(' ') + 1; // npos+1=0
        params = params.substr(coor ? coor : params.size());
        app::shared::start(params, aptype);
    }
    else
    {
        auto config = xml::settings{};
        app::shared::load::settings(config, cliopt);
        auto client = os::ipc::socket::open<os::role::client, faux>(prefix, denied);
        auto signal = ptr::shared<os::fire>(os::process::started(prefix)); // Signaling that the server is ready for incoming connections.

             if (denied)                           return failed(code::noaccess);
        else if (whoami != type::client && client) return failed(code::interfer);
        else if (whoami == type::client && !client)
        {
            log("%%New desktop session for [%userid%]", prompt::main, userid.first);
            auto [success, successor] = os::process::fork(system, prefix, config.settings::utf8());
            if (successor)
            {
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
                auto win = os::dtvt::gridsz;
                auto gui = app::shared::get_gui_config(config);
                userinit.send(client, userid.first, os::dtvt::vtmode, env, cwd, cmd, win);
                ui::tui_domain().config.swap(config);
                app::shared::splice(client, gui);
                return 0;
            }
            else return failed(denied ? code::noaccess : code::noserver);
        }

        if (whoami == type::daemon)
        {
            auto [success, successor] = os::process::fork(system, prefix, config.settings::utf8(), script);
            if (successor)
            {
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
        auto srvlog = os::ipc::socket::open<os::role::server>(prefix_log, denied);
        if (!srvlog)
        {
            if (denied) failed(code::noaccess);
            return      failed(code::nosrvlog);
        }

        signal->bell(); // Signal we are started and ready for connections.
        signal.reset();

        namespace e2 = ui::e2;
        auto& indexer = ui::tui_domain();
        indexer.config.swap(config);
        auto lock = indexer.unique_lock();
        auto desktop = app::vtm::hall::ctor(server);
        desktop->autorun();
        lock.unlock();

        log("%%Session started"
            "\n      user: %userid%"
            "\n      pipe: %prefix%", prompt::main, userid.first, prefix);

        auto stdlog = std::thread{ [&]
        {
            while (auto monitor = srvlog->meet())
            {
                desktop->run([&, monitor](auto /*task_id*/)
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
                            desktop->base::signal(tier::release, e2::command::run, onecmd);
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
                        events.command.send(monitor, onecmd.cmd); // Output reply.
                    }};
                    auto writer = netxs::logger::attach([&](auto utf8)
                    {
                        events.logs.send(monitor, ui32{}, datetime::now(), text{ utf8 });
                    });
                    desktop->LISTEN(tier::general, e2::conio::quit, deal, tokens) { monitor->shut(); };
                    os::ipc::monitors++;
                    directvt::binary::stream::reading_loop(monitor, [&](view data){ events.s11n::sync(data); });
                    os::ipc::monitors--;
                    if (id.size()) log("%%Monitor [%id%] disconnected", prompt::logs, id);
                });
            }
        }};

        auto execline = [&](qiew line){ desktop->base::signal(tier::release, e2::command::run, { .cmd = line }); };
        auto shutdown = [&]{ desktop->base::signal(tier::general, e2::shutdown, utf::concat(prompt::main, "Shutdown on signal")); };
        execline(script);
        auto readline = os::tty::readline(execline, shutdown);
        while (auto user = server->meet())
        {
            if (user->auth(userid.second))
            {
                desktop->run([&, user](auto session_id)
                {
                    auto userinit = directvt::binary::init{};
                    if (auto packet = userinit.recv(user))
                    {
                        auto id = utf::concat(*user);
                        if constexpr (debugmode) log("%%Client connected %id%", prompt::user, id);
                        os::ipc::users++;
                        desktop->invite(user, packet.user, packet.mode, packet, session_id);
                        os::ipc::users--;
                        if constexpr (debugmode) log("%%Client disconnected %id%", prompt::user, id);
                    }
                });
            }
        }
        readline.stop();
        srvlog->stop(); // Monitor listener endpoint must be closed first to prevent reconnections.
        stdlog.join();
        desktop->stop();
    }

    os::release();
}