// Copyright (c) NetXS Group.
// Licensed under the MIT license.

auto DirectVT = "\3\33\333DESKTOPIO\7"
R"==(
="Calc DEMO", "Calc Tooltip Message", a("Direct", "Calc App Title", "cmdline args")
)==";

#define DESKTOP_VER "v0.7.6"
#define MONOTTY_VER "Monotty Desktopio " DESKTOP_VER
#define MONOTTY_MYNAME "vtm/calc"
#define MONOTTY_DEFAPP "Calc"
#define MONOTTY_APPINF "Desktopio Terminal " DESKTOP_VER
#define PROD

#include "../apps.hpp"

using namespace netxs;
using namespace netxs::console;

int main(int argc, char* argv[])
{
    auto vtmode = os::vt_mode();
    auto syslog = os::ipc::logger(vtmode);
    auto maxfps = si32{ 60 };
    auto menusz = 1;

    //todo unify
    skin::setup(tone::kb_focus, 60);
    skin::setup(tone::brighter, 0);
    skin::setup(tone::shadower, 180);
    skin::setup(tone::shadow  , 180);
    skin::setup(tone::lucidity, 255);
    skin::setup(tone::selector, 48);
    skin::setup(tone::bordersz, dot_11);

    //todo unify
    log(MONOTTY_APPINF);

    auto success = app::shared::start(MONOTTY_DEFAPP, MONOTTY_MYNAME, vtmode, maxfps, menusz);

    if (success) return 0;
    else
    {
        log("main: app initialization error");
        return 1;
    }
}