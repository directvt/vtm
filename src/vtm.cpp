// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#include "netxs/os/system.h"

#include <fstream> // std::ifstream

using namespace netxs;
using namespace netxs::console;
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

    //[Demo] get current region from ~./vtm/vtm.conf
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

    auto link = os::ipc::open<os::client>(path, 10s, [&]() {
        log("main: no desktop for user ", user);
        log("main: spawning desktop environment");
        return os::exec("vtmd", "-d");
        });

    if (!link) os::exit(-1, "main: open server link error");

    link->send(utf::concat(spot, ";",
                           host, ";",
                           name, ";",
                           user, ";"));

    auto gate = os::tty::proxy(link);
    gate.ignite();
    gate.output(ansi::altbuf(true) + // Switch to alternate buffer.
                ansi::vmouse(true) + // Turn mouse reporting on/off.
                ansi::cursor(faux) + // Set the cursor visibility.
                ansi::setutf(true)); // Set UTF-8 character set.
    gate.splice();
    gate.output(ansi::vmouse(faux) +
                ansi::cursor(true) +
                ansi::altbuf(faux));
    gate.revert();

    // Pause to consume/receive buffered input (e.g. mouse tracking)
    // that has just been canceled.
    std::this_thread::sleep_for(200ms);
}