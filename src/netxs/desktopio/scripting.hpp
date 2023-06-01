// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#include "application.hpp"

namespace netxs::scripting
{
    using namespace ui;

    template<class Host>
    class repl
    {
        using s11n = directvt::binary::s11n;
        using pidt = os::pidt;
        using task = os::task;

        Host&      owner;

        // repl: Event handler.
        class xlat
            : public s11n
        {
            Host& owner; // xlat: Boss object reference.
            subs  token; // xlat: Subscription tokens.

        public:
            void disable()
            {
                token.clear();
            }

            void handle(s11n::xs::fullscreen          lock)
            {
                //...
            }
            void handle(s11n::xs::focus_cut           lock)
            {
                //...
            }
            void handle(s11n::xs::focus_set           lock)
            {
                //...
            }
            void handle(s11n::xs::keybd_event         lock)
            {
                //...
            };

            xlat(Host& owner)
                : s11n{ *this },
                 owner{ owner }
            {
                owner.LISTEN(tier::anycast, e2::form::prop::ui::header, utf8, token)
                {
                    //s11n::form_header.send(owner, 0, utf8);
                };
                owner.LISTEN(tier::anycast, e2::form::prop::ui::footer, utf8, token)
                {
                    //s11n::form_footer.send(owner, 0, utf8);
                };
                owner.LISTEN(tier::release, hids::events::device::mouse::any, gear, token)
                {
                    //...
                };
                owner.LISTEN(tier::general, hids::events::die, gear, token)
                {
                    //...
                };
            }
        };

        xlat stream; // repl: Event tracker.
        text curdir; // repl: Current working directory.
        text cmdarg; // repl: Startup command line arguments.
        flag active; // repl: Scripting engine lifetime.
        pidt procid; // repl: PTY child process id.
        task engine; // repl: Scripting engine instance.

        // repl: Proceed DirectVT input.
        void ondata(view data)
        {
            if (active)
            {
                log(prompt::repl, ansi::hi(utf::debase<faux, faux>(data)));
                //stream.s11n::sync(data);
            }
        }
        // repl: Shutdown callback handler.
        void onexit(si32 code)
        {
            //netxs::events::enqueue(owner.This(), [&, code](auto& boss) mutable
            //{
            //    if (code) log(ansi::bgc(reddk).fgc(whitelt).add('\n', prompt::repl, "Exit code ", utf::to_hex_0x(code), ' ').nil());
            //    else      log(prompt::repl, "Exit code 0");
            //    //backup.reset(); // Call repl::dtor.
            //});
        }

    public:
        // repl: Write client data.
        void write(view data)
        {
            log(prompt::repl, "exec: ", ansi::hi(utf::debase<faux, faux>(data)));
            engine.write(data);
        }
        // repl: Start a new process.
        void start(text cwd, text cmd)
        {
            curdir = cwd;
            cmdarg = cmd;
            if (!engine)
            {
                procid = engine.start(curdir, cmdarg, [&](auto utf8_shadow) { ondata(utf8_shadow); },
                                                      [&](auto exit_reason) { onexit(exit_reason); });
            }
        }
        void shut()
        {
            active = faux;
            if (engine) engine.shut();
        }

        repl(Host& owner)
            : owner{ owner },
              stream{owner },
              active{ true }
        {

        }
    };
}