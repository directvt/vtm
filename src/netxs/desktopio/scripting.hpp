// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#include "application.hpp"

namespace netxs::scripting
{
    using namespace ui;

    namespace path
    {
        static constexpr auto scripting = "/config/scripting/";
    }
    namespace attr
    {
        static constexpr auto cwd = "cwd";
        static constexpr auto cmd = "cmd";
        static constexpr auto run = "run";
        static constexpr auto tty = "tty";
        static constexpr auto rse = "engine"; // Runtime Scripting Engine.
    }

    struct faketerm
    {
        using face = ui::face; // Reference for consrv.
        struct bufferbase : public ansi::parser
        {
            twod panel = {1000, 1000};
            twod coord = dot_00; // bufferbase: Viewport cursor position; 0-based.
            template<class T>
            void resize_viewport(T a) { }
            template<class T>
            void set_autocr(T a) { }
            void cup0(twod p) { }
            template<bool Copy = faux, class Span, class Shader>
            void _data(si32 count, Span const& proto, Shader fuse) { }
            void clear_all() { }
            void ed(si32) { }
            template<class T>
            void do_viewport_copy(T mirror) { }
            void cr() { }
            void lf(si32 n) { }
            void move(si32 n) { }
        };
        struct alt_screen : public bufferbase
        {
            alt_screen() = default;
            alt_screen(faketerm&) { }
        };
        struct scrollback : public bufferbase
        { };
        // repl: Terminal 16/256 color palette tracking functionality.
        struct c_tracking
        {
            using pals = std::remove_const_t<decltype(rgba::color256)>;
            using func = std::unordered_map<text, std::function<void(view)>>;
            pals  color; // c_tracking: 16/256 colors palette.
            func  procs; // c_tracking: Handlers.
        };
        // repl: Terminal title tracking functionality.
        struct w_tracking
        {
            std::map<text, text>              props;
            std::map<text, std::vector<text>> stack;
            // w_tracking: Get terminal window property.
            auto& get(text const& property)
            {
                auto& utf8 = props[property];
                return utf8;
            }
            // w_tracking: Set terminal window property.
            void set(text const& property, qiew txt) { }
        };
        // repl: VT-style mouse tracking functionality.
        struct m_tracking
        {
            enum mode
            {
                none = 0,
                bttn = 1 << 0,
                drag = 1 << 1,
                move = 1 << 2,
                over = 1 << 3,
                utf8 = 1 << 4,
                buttons_press = bttn,
                buttons_drags = bttn | drag,
                all_movements = bttn | drag | move,
                negative_args = bttn | drag | move | over,
            };
            enum prot
            {
                x11,
                sgr,
                w32,
            };
            void enable(mode m) { }
            void disable(mode m) { }
            void setmode(prot p) { }
        };
        struct caret
        {
            template<class T = si32>
            void style(T m) { }
            auto style() const { return std::pair{ true, faux }; }
            void hide() { }
            void show() { }
            void toggle() { }
        };
        alt_screen  altbuf;
        scrollback  normal;
        bufferbase* target{&normal};
        c_tracking  ctrack; // repl: Custom terminal palette tracking object.
        m_tracking  mtrack; // repl: .
        w_tracking  wtrack; // repl: .
        caret       cursor; // repl: Text cursor controller.
        bool        io_log{ faux }; // repl: Stdio logging.
        void sb_min(si32 size_y) { }
        void window_resize(twod windowsz) { }
        template<class T>
        void reset_to_altbuf(T& console) { }
        template<class T>
        void reset_to_normal(T a) { }
    };

    template<class Host>
    struct repl : public faketerm
    {
        using s11n = directvt::binary::s11n;
        using pidt = os::pidt;
        using vtty = sptr<os::runspace::basetty>;

        Host& owner;
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
        vtty engine; // repl: Scripting engine instance.

        // repl: Proceed input.
        void ondata(view data)
        {
            //if (active)
            {
                log(data, faux);
                //stream.s11n::sync(data);
            }
        }
        // repl: Proceed input.
        template<class T>
        void ondata(view data, T target)
        {
            //log(prompt::repl, ansi::hi(utf::debase<faux, faux>(data)));
            log(data, faux);
        }
        // repl: Shutdown callback handler.
        void onexit(si32 code, view msg = {})
        {
            //netxs::events::enqueue(owner.This(), [&, code](auto& boss) mutable
            //{
            //    if (code) log(ansi::bgc(reddk).fgc(whitelt).add('\n', prompt::repl, "Exit code ", utf::to_hex_0x(code), ' ').nil());
            //    else      log(prompt::repl, "Exit code 0");
            //    //backup.reset(); // Call repl::dtor.
            //});
        }
        // repl: .
        template<class P>
        void update(P proc)
        {
            proc();
        }
        // repl: Write client data.
        void write(view data)
        {
            if (!engine) return;
            log(prompt::repl, "exec: ", ansi::hi(utf::debase<faux, faux>(data)));
            engine->write(data);
        }
        // repl: Start a new process.
        void start(text cwd, text cmd)
        {
            if (!engine) return;
            curdir = cwd;
            cmdarg = cmd;
            if (!engine->connected())
            {
                procid = engine->start(curdir, cmdarg, normal.panel, [&](auto utf8_shadow) { ondata(utf8_shadow); },
                                                                     [&](auto code, auto msg) { onexit(code, msg); });
            }
        }
        void shut()
        {
            active = faux;
            if (!engine) return;
            if (engine->connected()) engine->shut();
        }

        repl(Host& owner)
            : owner{ owner },
              stream{owner },
              active{ true }
        {
            auto& config = owner.config;
            if (config.take(path::scripting, faux))
            {
                config.pushd(path::scripting);
                auto rse = config.take(attr::rse, ""s);
                config.cd(rse);
                auto cwd = config.take(attr::cwd, ""s);
                auto cmd = config.take(attr::cmd, ""s);
                auto run = config.take(attr::run, ""s);
                auto tty = config.take(attr::tty, faux);
                if (tty) engine = ptr::shared<os::runspace::tty<repl>>(*this);
                else     engine = ptr::shared<os::runspace::raw>();
                start(cwd, cmd);
                //todo run integration script
                if (run.size()) write(run + "\n");
                config.popd();
            }
        }
    };
}