// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#define DESKTOPIO_VER "v0.9.8l"
#define DESKTOPIO_MYNAME "Desktopio Terminal " DESKTOPIO_VER
#define DESKTOPIO_MYPATH "vtm/term"
#define DESKTOPIO_DEFAPP "Term"

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
    auto getopt = os::args{ argc, argv };
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
        log("\nUsage:\n\n " + myname + " [ -c <file> ] [ -l ]\n"s
            + "\n"s
                + "\t-c | --config <..>  Use specified configuration file.\n"s
                + "\t-l | --listconfig   Show configuration and exit.\n"s
                + "\n"s
                + "\tConfiguration file location precedence (descending priority):\n\n"s
                + "\t\t1. Command line options; e.g., " + myname + " -c path/to/settings.xml\n"s
                + "\t\t2. Environment variable; e.g., VTM_CONFIG=path/to/settings.xml\n"s
                + "\t\t3. Hardcoded location \""s + app::shared::usr_config + "\"\n"s
                + "\t\t4. Default configuration\n"s
                );
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