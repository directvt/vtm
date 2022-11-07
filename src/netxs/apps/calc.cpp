// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#define DESKTOPIO_VER "v0.9.6f"
#define DESKTOPIO_MYNAME "Desktopio Calc " DESKTOPIO_VER
#define DESKTOPIO_MYPATH "vtm/calc"
#define DESKTOPIO_DEFAPP "Calc"

#include "../apps.hpp"

using namespace netxs;
using namespace netxs::console;

int main(int argc, char* argv[])
{
    auto vtmode = os::vt_mode();
    auto syslog = os::ipc::logger(vtmode);
    auto banner = [&]{ log(DESKTOPIO_MYNAME); };
    auto getopt = os::args{ argc, argv };
    auto params = DESKTOPIO_DEFAPP + " "s + getopt.tail();
    auto cfpath = text{};
    auto getopt = os::args{ argc, argv };
    while (getopt)
    {
        switch (getopt.next())
        {
            case 'l':
                log(app::shared::load::settings(cfpath, os::legacy::get_setup()).document->show());
                return 0;
            case 'c':
                cfpath = getopt.param();
                if (cfpath.size()) break;
                else os::fail("config file path not specified");
            default:
                banner();
                log("Usage:\n\n ", os::current_module_file(), " [ -c <config_file> ] [ -l ]\n"s
                    + "\n"s
                        + "\t-c\tUse specified configuration file.\n"s
                        + "\t-l\tShow configuration and exit.\n"s
                        + "\n"s
                        + "\tConfiguration file location precedence (descending priority):\n\n"s
                        + "\t\t1. Command line options; e.g., vtm -c path/to/settings.xml\n"s
                        + "\t\t2. Environment variable; e.g., VTM_CONFIG=path/to/settings.xml\n"s
                        + "\t\t3. Hardcoded location \""s + app::shared::usr_config + "\"\n"s
                        + "\t\t4. Default configuration\n"s
                        );
                return 0;
        }
    }

    banner();
    auto config = app::shared::load::settings(cfpath, os::legacy::get_setup());
    auto result = app::shared::start(params, DESKTOPIO_MYPATH, vtmode, config);

    if (result) return 0;
    else
    {
        log("main: app initialization error");
        return 1;
    }
}