// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#include "term.hpp"

using namespace netxs;

int main(int argc, char* argv[])
{
    auto defaults = 
    #include "term.xml"

    auto vtmode = os::tty::vtmode();
    auto syslog = os::tty::logger(vtmode);
    auto banner = [&]{ log(app::term::desc, ' ', app::shared::version); };
    auto cfonly = faux;
    auto cfpath = text{};
    auto errmsg = text{};
    auto getopt = os::process::args{ argc, argv };
    auto params = app::term::id + " "s + getopt.rest();
    getopt.reset();
    while (getopt)
    {
        if (getopt.match("-l", "--listconfig"))
        {
            cfonly = true;
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

    banner();
    if (errmsg.size())
    {
        os::fail(errmsg);
        auto myname = os::process::binary<true>();
        log("\nTerminal emulator.\n\n"s
            + "  Syntax:\n\n    " + myname + " [ -c <file> ] [ -l ]\n"s
            + "\n"s
            + "  Options:\n\n"s
            + "    No arguments        Run application.\n"s
            + "    -c | --config <..>  Use specified configuration file.\n"s
            + "    -l | --listconfig   Show configuration and exit.\n"s
            + "\n"s
            + "  Configuration precedence (descending priority):\n\n"s
            + "    1. Command line options: " + myname + " -c path/to/settings.xml\n"s
            + "    2. Environment variable: "s + app::shared::env_config.substr(1) + "=path/to/settings.xml\n"s
            + "    3. Hardcoded location \""s  + app::shared::usr_config + "\"\n"s
            + "    4. Hardcoded configuration\n"s);
    }
    else if (cfonly)
    {
        log("Running configuration:\n", app::shared::load::settings<true>(defaults, cfpath, os::dtvt::config()));
    }
    else
    {
        auto config = app::shared::load::settings(defaults, cfpath, os::dtvt::config());
        auto result = app::shared::start(params, app::term::id, vtmode, config);

        if (result) return 0;
        else
        {
            log("main: app initialization error");
            return 1;
        }
    }
}