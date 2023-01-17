// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#define DESKTOPIO_VER "v0.9.8m"
#define DESKTOPIO_MYNAME "Desktopio Calc " DESKTOPIO_VER
#define DESKTOPIO_MYPATH "vtm/calc"
#define DESKTOPIO_DEFAPP "Calc"

#include "../apps.hpp"

using namespace netxs;
using namespace netxs::console;

int main(int argc, char* argv[])
{
    auto vtmode = os::tty::vtmode();
    auto syslog = os::tty::logger(vtmode);
    auto banner = [&]{ log(DESKTOPIO_MYNAME); };
    auto cfonly = faux;
    auto cfpath = text{};
    auto errmsg = text{};
    auto getopt = os::process::args{ argc, argv };
    auto params = DESKTOPIO_DEFAPP + " "s + getopt.rest();
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
        log("Spreadsheet calculator (Demo).\n\n"s
            + "  Syntax:\n\n    " + myname + " [ -c <file> ] [ -l ]\n"s
            + "\n"s
            + "  Options:\n\n"s
            + "    No arguments        Run application.\n"s
            + "    -c | --config <..>  Use specified configuration file.\n"s
            + "    -l | --listconfig   Show configuration and exit.\n"s
            + "\n"s
            + "  Configuration file precedence (descending priority):\n\n"s
            + "    1. Command line options; e.g., " + myname + " -c path/to/settings.xml\n"s
            + "    2. Environment variable; e.g., VTM_CONFIG=path/to/settings.xml\n"s
            + "    3. Hardcoded location \""s + app::shared::usr_config + "\"\n"s
            + "    4. Hardcoded configuration\n"s);
    }
    else if (cfonly)
    {
        log("Running configuration:\n", app::shared::load::settings<true>(cfpath, os::dtvt::config()));
    }
    else
    {
        auto config = app::shared::load::settings(cfpath, os::dtvt::config());
        auto result = app::shared::start(params, DESKTOPIO_MYPATH, vtmode, config);

        if (result) return 0;
        else
        {
            log("main: app initialization error");
            return 1;
        }
    }
}