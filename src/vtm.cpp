// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#include "netxs/os/system.hpp"

#include <fstream> // std::ifstream

using namespace netxs;
using namespace std::chrono_literals;

int main(int argc, char* argv[])
{
    auto host = os::get_env("SSH_CLIENT");
    auto name = os::get_env("USER");

    os::start_log("vtm");
    netxs::logger::logger logger([&](auto&& data) { os::syslog(data); });

    if (argc > 1)
    {
        auto error = utf::text{ "block explicit bash command invocation " };
        error += "{" + name +", " + host + "}";
        for (auto i = 1; i < argc; i++)
        {
            error += '\n';
            error += utf::text(argv[i]);
        }
        os::exit(1, error);
    }

    // Demo: Get current region from "~./vtm/vtm.conf".
    utf::text spot;
    {
        std::ifstream config;
        config.open("vtm.conf");

        if (config.is_open())
            std::getline(config, spot);

        if (spot.empty())
            spot = "unknown region";
    }

    auto user = os::user();
    auto path = utf::concat("monotty_", user); //todo unify, use vtm.conf
    auto link = os::ipc::open<os::client>(path, 10s, [&]()
        {
            log("main: new desktop environment for user ", user);
            return os::exec("vtmd", "-d");
        });

    if (!link) os::exit(-1, "main: desktop server connection error");

    auto mode = os::legacy_mode();

    link->send(utf::concat(spot, ";",
                           host, ";",
                           name, ";",
                           user, ";",
                           mode, ";"));

    auto gate = os::tty::proxy(link);
    gate.ignite();
    gate.output(ansi::esc{}.save_title()
                           .altbuf(true)
                           .vmouse(true)
                           .cursor(faux)
                           .bpmode(true)
                           .setutf(true));
    gate.splice(mode);

    gate.output(ansi::esc{}.scrn_reset()
                           .vmouse(faux)
                           .cursor(true)
                           .altbuf(faux)
                           .bpmode(faux)
                           .load_title());

    // Pause to complete consuming/receiving buffered input (e.g. mouse tracking)
    // that has just been canceled.
    std::this_thread::sleep_for(200ms);
}