// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#include "term.hpp"

using namespace netxs;

int main(int argc, char* argv[])
{
    auto defaults = 
    #include "term.xml"

    os::dtvt::initialize();
    auto syslog = os::tty::logger();
    auto banner = []{ log(app::terminal::name, ' ', app::shared::version); };
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

    os::dtvt::checkpoint();
    banner();
    if (errmsg.size())
    {
        os::fail(errmsg);
        log("\n"
            "\n  Syntax:"
            "\n"
            "\n    " + os::process::binary<true>() + " [ -c <file> ][ -l ]"
            "\n"
            "\n  Options:"
            "\n"
            "\n    No arguments         Run application."
            "\n    -c, --config <file>  Specifies the settings file to load."
            "\n    -l, --listconfig     Print configuration."
            "\n");
    }
    else if (cfonly)
    {
        log(prompt::resultant_settings, "\n", app::shared::load::settings<true>(defaults, cfpath, os::dtvt::config));
    }
    else
    {
        auto config = app::shared::load::settings(defaults, cfpath, os::dtvt::config);
        app::shared::start(params, app::terminal::id, config);
    }
}