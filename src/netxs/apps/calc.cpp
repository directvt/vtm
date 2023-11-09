// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#include "calc.hpp"

using namespace netxs;

int main(int argc, char* argv[])
{
    auto defaults = 
    #include "calc.xml"

    auto syslog = os::tty::logger();
    auto banner = []{ log(app::calc::desc, ' ', app::shared::version); };
    auto cfonly = faux;
    auto cfpath = text{};
    auto errmsg = text{};
    auto getopt = os::process::args{ argc, argv };
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
            log(app::shared::version);
            return 0;
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
    auto params = getopt.rest();

    banner();
    if (errmsg.size())
    {
        os::fail(errmsg);
        log("\n"
            "  Syntax:\n"
            "\n"
            "    " + os::process::binary<true>() + " [ -c <file> ] [ -l ]\n"
            "\n"
            "  Options:\n"
            "\n"
            "    No arguments       Run application.\n"
            "    -c, --config <..>  Load specified settings file.\n"
            "    -l, --listconfig   Show configuration and exit.\n"
            "\n"
            "  Settings loading and merging order:\n"
            "\n"
            "    - Initialize hardcoded settings\n"
            "    - Merge with explicitly specified settings from --config <file>\n"
            "    - If the --config option is not used or <file> cannot be loaded:\n"
            "        - Merge with system-wide settings from " + os::path::expand(app::shared::sys_config).second + "\n"
            "        - Merge with user-wise settings from "   + os::path::expand(app::shared::usr_config).second + "\n"
            "        - Merge with DirectVT packet received from the parent process (dtvt-mode only)\n");
    }
    else if (cfonly)
    {
        log(prompt::resultant_settings, "\n", app::shared::load::settings<true>(defaults, cfpath, os::dtvt::config));
    }
    else
    {
        auto config = app::shared::load::settings(defaults, cfpath, os::dtvt::config);
        app::shared::start(params, app::calc::id, os::dtvt::vtmode, os::dtvt::win_sz, config);
    }
}