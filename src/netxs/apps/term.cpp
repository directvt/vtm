// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#define DESKTOPIO_VER "v0.9.0"
#define DESKTOPIO_MYNAME "Desktopio Terminal " DESKTOPIO_VER
#define DESKTOPIO_MYPATH "vtm/term"
#define DESKTOPIO_DEFAPP "Term"

#include "../apps.hpp"

using namespace netxs;
using namespace netxs::console;

int main(int argc, char* argv[])
{
    auto vtmode = os::vt_mode();
    auto syslog = os::ipc::logger(vtmode);
    auto maxfps = si32{ 60 };
    auto menusz = 1;
    auto getopt = os::args{ argc, argv };
    auto params = DESKTOPIO_DEFAPP + " "s + getopt.tail();

    //todo unify
    skin::setup(tone::kb_focus, 60);
    skin::setup(tone::brighter, 0);
    skin::setup(tone::shadower, 180);
    skin::setup(tone::shadow  , 180);
    skin::setup(tone::lucidity, 255);
    skin::setup(tone::selector, 48);
    skin::setup(tone::bordersz, dot_11);

    //todo unify
    log(DESKTOPIO_MYNAME);

    auto success = app::shared::start(params, DESKTOPIO_MYPATH, vtmode, maxfps, menusz);

    if (success) return 0;
    else
    {
        log("main: app initialization error");
        return 1;
    }
}