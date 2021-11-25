// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#define MONOTTY_VER "Monotty Desktopio v0.5.9999a"

// Enable demo apps and assign Esc key to log off.
#define DEMO

// Enable keyboard input and unassign Esc key.
#define PROD

// Tiling nesting max level.
#ifndef PROD
    #define INHERITANCE_LIMIT 12
    #define APPS_MAX_COUNT 20
    #define APPS_DEL_TIMEOUT 1s
#else
    #define INHERITANCE_LIMIT 30
#endif

// Enable to show debug overlay.
//#define DEBUG_OVERLAY

// Enable to show all terminal input (keyboard/mouse etc).
//#define KEYLOG

// Highlight region ownership.
//#define REGIONS

#include "netxs/console/terminal.hpp"
#include "netxs/apps.hpp"
#include "netxs/abstract/queue.hpp"

#include <fstream>

//todo remove
using namespace std::placeholders;
using namespace netxs::console;
using namespace netxs;

//todo move to app::Test
enum test_topic_vars
{
    object1,
    object2,
    object3,

    canvas1,
    canvas2,

    dynamix1,
    dynamix2,
    dynamix3,
};

class post_logs
    : public ui::post
{
    pro::caret caret{ *this }; // post_logs: Text caret controller.

    text label;
    subs token;
    bool itsme = faux;

    struct log_parser
        : public bell
    {
        std::thread           input;
        ansi::esc             yield;
        netxs::mt_queue<text> queue;
        bool                  alive = true;
        bool show_codepoints = faux;
        ~log_parser()
        {
            alive = faux;
            queue.push(text{});
            if (input.joinable())
            {
                input.join();
            }
        }
        log_parser()
        {
            SUBMIT(tier::general, e2::debug::output, shadow)
            {
                queue.push(text{ shadow });
            };
            input = std::thread{ [&]() { worker(); } };
        }
        void enable_codepoints(bool s)
        {
            show_codepoints = s;
            auto msg = ansi::bgc(s ? greendk : yellowdk).fgc(whitelt)
                .add(" show codepoints: ", s ? "on":"off", "\n").nil();
            SIGNAL(tier::general, e2::debug::logs, msg);
            SIGNAL(tier::release, e2::command::custom, s ? 1 : 2);
        }
        void worker()
        {
            while (alive)
            {
                auto utf8 = queue.pop();
                bool not_procesed = true;
                while (not_procesed && alive)
                {
                    if (auto lock = events::try_sync())
                    {
                        not_procesed = faux;
                        auto shadow = view{ utf8 };
                        auto parsed = read(shadow);
                        SIGNAL(tier::release, e2::debug::parsed, parsed);
                    }
                    else std::this_thread::yield();
                }
            }
        }
        page read(view shadow)
        {
            auto w = 0;
            auto max_col = 12;
            auto wc = 5;

            yield.clear();
            yield.wrp(wrap::off)
                 .bgc(ansi::yellowlt).add(utf::repeat(' ', max_col * (wc + 3))).eol().bgc()
                 .add("STDOUT: plain text, ", shadow.size(), " bytes")
                 .eol().bgc(ansi::whitedk).fgc(blackdk)
                 .add(utf::debase(shadow))
                 .fgc().bgc().eol().eol();
            if (show_codepoints)
            {
                yield.wrp(wrap::off).add("STDOUT: codepoints").eol();
                auto f = [&](unsigned cp, view utf8, iota wide)
                {
                    yield.fgc(ansi::blackdk);
                    if (wide)
                    {
                        yield.bgc(ansi::whitedk).add(' ', utf8);
                        if (wide == 1) yield.add(' ');
                    }
                    else yield.bgc(ansi::redlt).add(" - ");

                    yield.bgc().fgc(ansi::greendk)
                         .add(utf::adjust(utf::to_hex<true>(cp, (cp <= 0xFF   ? 2 :
                                                                 cp <= 0xFFFF ? 4 : 5)), wc, ' '))
                         .fgc().bgc();
                    if (++w == max_col) { w = 0; yield.eol(); }
                };
                auto s = [&](utf::prop const& traits, view const& utf8)
                {
                    f(traits.control, "", 0);
                    return utf8;
                };
                auto y = [&](utf::frag const& cluster)
                {
                    f(cluster.attr.cdpoint, cluster.text, cluster.attr.ucwidth);
                };
                utf::decode<faux>(s, y, shadow);
                yield.wrp(deco::defwrp).eol();
            }
            yield.eol();
            return page{ yield };
        }
    };

    void update()
    {
        ui::post::recalc();
        auto new_cp = flow::cp();

        //todo unify, its too hacky
        auto new_coor = twod{ 0, std::numeric_limits<iota>::min() };
        SIGNAL(tier::release, e2::coor::set, new_coor);
        auto new_size = post::get_size();
        SIGNAL(tier::release, e2::size::set, new_size);

        caret.coor(new_cp);
    }
    void clear()
    {
        topic.clear();
        topic += "cleared...\n";
        topic += label;
        update();
    }

public:
    sptr<log_parser> worker = netxs::shared_singleton<log_parser>();

    post_logs()
    {
        caret.show();
        caret.coor(dot_01);

        #ifndef PROD
        topic.maxlen(400);
        #else
        topic.maxlen(10000);
        #endif

        label = ansi::bgc(whitelt).fgc(blackdk).add(
              " Note: Log is limited to ", topic.maxlen(), " lines (old lines will be auto-deleted) \n"
              " Note: Use right mouse double-click to clear log \n"
              " Note: Using logs causes performance degradation \n")
              .fgc().bgc();
        topic += label;

        SUBMIT(tier::release, hids::events::mouse::button::dblclick::right, gear)
        {
            clear();
            gear.dismiss();
        };
        broadcast->SUBMIT(tier::request, e2::command::custom, status)
        {
            switch (status)
            {
                case 1:
                    status = worker->show_codepoints ? 1 : 2;
                    break;
                default:
                    break;
            }
        };
        broadcast->SUBMIT(tier::preview, e2::command::custom, cmd_id)
        {
            switch (cmd_id)
            {
                case 0:
                    clear();
                    break;
                case 1:
                case 2:
                    itsme = true;
                    worker->enable_codepoints(cmd_id == 1 ? true : faux);
                    itsme = faux;
                    break;
                default:
                    break;
            }
        };
        broadcast->SIGNAL(tier::release, e2::command::custom, worker->show_codepoints ? 1 : 2);
        SUBMIT(tier::preview, e2::size::set, newsize)
        {
            caret.coor(flow::cp());
        };
        SUBMIT(tier::general, e2::debug::logs, utf8)
        {
            topic += utf8;
            update();
        };
        worker->SUBMIT_T(tier::release, e2::command::custom, token, status)
        {
            broadcast->SIGNAL(tier::release, e2::command::custom, status);
        };
        worker->SUBMIT_T(tier::release, e2::debug::parsed, token, parsed_page)
        {
            topic += parsed_page;
            update();
        };
    }
};

int main(int argc, char* argv[])
{
    // Initialize global logger.
    netxs::logger::logger logger(
        [](auto const& utf8)
        {
            static text buff;
            os::syslog(utf8);
            if (auto sync = events::try_sync{})
            {
                if (buff.size())
                {
                    SIGNAL_GLOBAL(e2::debug::logs, view{ buff });
                    buff.clear();
                }
                SIGNAL_GLOBAL(e2::debug::logs, view{ utf8 });
            }
            else buff += utf8;
        });

    {
        auto banner = [&]() { log(MONOTTY_VER"\nDesktop Environment Server"); };
        bool daemon = faux;
        auto getopt = os::args{ argc, argv };
        while (getopt)
        {
            switch (getopt.next())
            {
                case 'd':
                    daemon = true;
                    break;
                default:
                    banner();
                    log("Usage:\n\t", argv[0], " [ -d ]\n\n\t-d\tRun as a daemon.");
                    os::exit(1);
            }
        }

        if (daemon && !os::daemonize(argv[0]))
        {
            banner();
            os::exit(1, "main: failed to daemonize");
        }

        banner();
    }

    //todo Get current config from vtm.conf.
    text config;
    {
        std::ifstream conf;
        conf.open("vtm.conf");
        if (conf.is_open()) std::getline(conf, config);
        if (config.empty()) config = "empty config";
    }

    // vtm: Get user defined tiling layouts.
    auto tiling_profiles = os::get_envars("VTM_PROFILE");
    if (auto size = tiling_profiles.size())
    {
        iota i = 0;
        log("main: tiling profile", size > 1 ? "s":"", " found");
        for (auto& p : tiling_profiles)
        {
            log(" ", i++, ". profile: ", utf::debase(p));
        }
    }

    {
        skin::setup(tone::kb_focus, 60);
        skin::setup(tone::brighter, 60);//120);
        //skin::setup(tone::shadower, 0);
        skin::setup(tone::shadower, 180);//60);//40);// 20);
        skin::setup(tone::shadow, 180);//5);
        //skin::setup(tone::lucidity, 192);
        skin::setup(tone::lucidity, 255);
        skin::setup(tone::selector, 48);
        skin::setup(tone::bordersz, dot_11);

        auto world = base::create<host>([&](auto reason) { os::exit(0, reason); });

        log("host: created");

        using slot = ui::slot;
        using axis = ui::axis;
        using axes = ui::axes;
        using snap = ui::snap;
        using id_t = netxs::input::id_t;

        auto create_app = [&](auto&& create_app, auto type, view data) mutable -> sptr<base>
        {
            //todo use XAML for that
            switch (type)
            {
                default:
                case app::shared::objs::Test:
                {
                    text topic;
                    {
                        auto clr = 0xFFFFFFFF;
                        text msg = "The quick brown fox jumps over the lazy dog.";
                        text msg_rtl = ansi::rtl(rtol::rtl).add("RTL: ", msg).rtl(rtol::ltr);
                        text msg_ltr = "LTR: " + msg;
                        text testline = ansi::jet(bias::center).rtl(rtol::rtl)
                            .add("RTL: centered text.\n\n")
                            .rtl(rtol::ltr)
                            .add("centered text\n\n")
                            .add("another ").nop().fgc(redlt).add("inlined").nop().nil().add(" segment")
                            .jet(bias::left).bgc(blackdk)
                            .add(" affix \n\n")
                            .nil()
                            .add(msg_ltr, "\n\n",
                                msg_rtl, "\n\n")
                            .jet(bias::right).rtl(rtol::ltr)
                            .add(msg_ltr, "\n\n", msg_rtl);

                        iota mrgin = 4;
                        auto l1 = ansi::mgl(mrgin * 1).mgr(1).fgc(whitelt).und(true);
                        auto l2 = ansi::mgl(mrgin * 2).mgr(1);
                        auto l3 = ansi::mgl(mrgin * 3).mgr(1);
                        auto c1 = bluelt;// 0xffff00;
                        auto c2 = whitedk;//0xffffff;
                        text intro = ansi::mgl(0).mgr(0)
                            .add(" ")
                            //+ ansi::jet(bias::right).mgl(1).mgr(1).wrp(true)
                            //+ "https://github.com/netxs-group/vtm\n\n"
                            .jet(bias::center).wrp(wrap::off).fgc(whitelt).mgl(0).mgr(0).eol()
                            .fgc(c1).bgc(c2).add("▄")
                            .fgc(c2).bgc(c1).add("▄")
                            .fgc(clr).bgc().add("  Monotty Desktopio\n")
                            .fgc().bgc().add("Test Page    \n\n")

                            .nil().jet(bias::left).mgl(4).mgr(4).wrp(wrap::off)
                            .fgc(0xff000000).bgc(0xff00FF00).add(" ! ", "\n")
                            .cuu(1).chx(0).mgl(9).fgc().bgc().wrp(wrap::on)
                            .add("Test page for testing text formatting functionality. \n"
                            "The following text doesn't make much sense, "
                            "it's just a bunch of text samples.\n"
                            "\n")
                            .nil().jet(bias::left).mgl(4).mgr(4).wrp(wrap::off)
                            .fgc(0xffFFFFFF).bgc(0xff0000FF).add(" ! ", "\n")
                            .cuu(1).chx(0).mgl(9).fgc().bgc().wrp(wrap::on)
                            .add("Make sure your terminal supports mouse reporting.\n"
                            "\n")
                            .nil().jet(bias::left).mgl(4).mgr(4).wrp(wrap::off)
                            .fgc(0xffFFFFFF).bgc(0xff0000FF).add(" ! ", "\n")
                            .cuu(1).chx(0).mgl(9).fgc().bgc().wrp(wrap::on)
                            .add("At the moment terminal "
                            "emulators are not able to display wide characters "
                            "in parts, as well as independently color the left "
                            "and right parts. In every case, when such a wide "
                            "character separation is required, we have to replace "
                            "the corresponding parts with 'REPLACEMENT CHARACTER' "
                            "(U+FFFD).\n"
                            "\n")

                            .jet(bias::center).wrp(wrap::off).fgc(whitelt).mgl(1).mgr(0)
                            .add("Test Samples\n\n")
                            .jet(bias::left).wrp(wrap::off).fgc(whitelt).mgl(1).mgr(0)
                            .add("User Interface Commands (outdated)\n")
                            .jet(bias::left).mgl(1).mgr(0).wrp(wrap::off).eol()
                            .fgc(whitelt).bld(true)
                            .add("Mouse:").nil().eol()
                            .add(l1).wrp(wrap::off)
                            .add("left").nil().eol()
                                .add(l2).fgc(blackdk).bgc(clr)
                                .add("click").nil().eol()
                                .add(l2).wrp(wrap::on)
                                .add("on the label in the upper left list of objects:\n"
                                    "on the connecting line of an object:\n")
                                    .add(l3, "- while holding down the Ctrl key, set/unset keyboard focus.\n")
                                    .add(l3, "- move the viewport to the center of the object.\n")
                                .add(l2).fgc(blackdk).bgc(clr).wrp(wrap::off)
                                .add("drag ").nil().eol()
                                .add(l2).wrp(wrap::on)
                                .add("outside of any objects:\n")
                                    .add(l3, "- panoramic navigation.\n")
                            .add(l1).wrp(wrap::off)
                            .add("right").nil().eol()
                                .add(l2).fgc(blackdk).bgc(clr).wrp(wrap::off)
                                .add("click").nil().eol()
                                .add(l2).wrp(wrap::on)
                                .add("on the connecting line of an object:\n")
                                    .add(l3, "- move object to mouse cursor.\n")
                                .add(l2)
                                .add("outside of any objects:\n")
                                    .add(l3, "- move the menu window to mouse cursor.\n")
                                .add(l2).fgc(blackdk).bgc(clr).wrp(wrap::off)
                                .add("drag ").nil().eol()
                                .add(l2).wrp(wrap::on)
                                .add("outside of any objects:\n")
                                    .add(l3, "- while holding down the Ctrl key, copies the selected area to the clipboard.\n")
                                    .add(l3, "- create a new object of the type selected in the menu (20 max).\n")
                                .add(l2)
                                .add("the scrollable content:\n")
                                    .add(l3, "- scrolling and kinetic scrolling on release, like a mouse wheel, but faster and more convenient.\n")
                            .add(l1).wrp(wrap::off)
                            .add("middle").nil().eol()
                                .add(l2).fgc(blackdk).bgc(clr)
                                .add("click").nil().eol()
                                .add(l2).wrp(wrap::on)
                                .add("on the label in the upper left list of objects:\n")
                                .add("on the connecting line of an object:\n")
                                .add("on the object itself:\n")
                                    .add(l3, "- destroy the object (except menu window).\n")
                                .add(l2).fgc(blackdk).bgc(clr).wrp(wrap::off)
                                .add("drag ").nil().eol()
                                .add(l2).wrp(wrap::on)
                                .add("inside the viewport:\n")
                                    .add(l3, "- while holding down the Ctrl key, copies the selected area to the clipboard.\n")
                                    .add(l3, "- create a new object of the type selected in the menu (20 max).\n")
                            .add(l1).wrp(wrap::off)
                            .add("left + right").nil().wrp(wrap::off).eol()
                                .add(l2).fgc(blackdk).bgc(clr).wrp(wrap::off)
                                .add("drag ").nil().wrp(wrap::on).eol()
                                .add("inside the viewport:\n")
                                    .add(l3, "- panoramic navigation.\n")
                                .add(l2).fgc(blackdk).bgc(clr)
                                .add("click").nil().eol()
                                .add(l2).wrp(wrap::on)
                                .add("inside the object:\n")
                                    .add(l3, "- destroy the object (except menu window).\n")
                            .add(l1).wrp(wrap::off)
                            .add("wheel").nil().wrp(wrap::off).eol()
                                .add(l2).fgc(blackdk).bgc(clr).wrp(wrap::off)
                                .add("scroll ").nil().wrp(wrap::on).eol()
                                    .add(l3, "- vertical scrolling.\n")
                                .add(l2).wrp(wrap::on)
                                .add("scroll + ctrl\n")
                                    .add(l3, "- horizontal scrolling.\n")
                                .add(l2)

                            .mgl(1).mgr(0)
                            .fgc(whitelt).bld(true).add("Keyboard:").nil().wrp(wrap::off).eol()
                            .add("    ").fgc(whitelt).und(true).add("Esc")             .nil().wrp(wrap::on).add(" - Quit/disconnect.\n")
                            .add("    ").fgc(whitelt).und(true).add("Ctrl")            .nil().wrp(wrap::on).add(" - Combine with the left mouse button to set/unset keyboard focus; combining with dragging right/middle mouse buttons copies the selected area to the clipboard.\n")
                            .add("    ").fgc(whitelt).und(true).add("Ctrl + PgUp/PgDn").nil().wrp(wrap::on).add(" - Navigation between windows.\n")
                            .eol()
                            .fgc(whitelt).bld(true).add("Taskbar menu:").nil().wrp(wrap::off).add(" (outdated)\n")
                            .add("    "            ).fgc(whitelt).und(true).add("Midnight Commander"  ).nil().wrp(wrap::off).add(" - live instance of Midnight Commander.\n")
                            .add("       "         ).fgc(whitelt).und(true).add("Truecolor image"     ).nil().wrp(wrap::off).add(" - true color ANSI/ASCII image.\n")
                            .add("          "      ).fgc(whitelt).und(true).add("Refresh Rate"        ).nil().wrp(wrap::off).add(" - terminal screen refresh rate selector (all users affected).\n")
                            .add("                ").fgc(whitelt).und(true).add("Strobe"              ).nil().wrp(wrap::off).add(" - an empty resizable window that changes background color every frame.\n")
                            .add("  "              ).fgc(whitelt).und(true).add("Recursive connection").nil().wrp(wrap::off).add(" - is limited to 3 connections in Demo mode.\n")
                            .eol()
                            .add("    ").fgc(whitelt).und(true).add("Disconnect").nil().wrp(wrap::off).add(" - Disconnect current user.\n")
                            .add("    ").fgc(whitelt).und(true).add("Shutdown"  ).nil().wrp(wrap::off).add("   - Disconnect all connected users and shutdown.\n\n")
                            .eol()
                            .wrp(wrap::on).mgl(0).mgr(0).eol()
                            .eol();

                        text data = ansi::nil()
                            .jet(bias::center).wrp(wrap::off).fgc(whitelt).add("Test Layouts\n\n")
                            .wrp(wrap::off).jet(bias::left).und(true)
                            .add("Text wrapping is OFF:\n\n").nil().wrp(wrap::off)
                            .add(testline)
                            .add("\n\n")

                            .wrp(wrap::off).jet(bias::left).und(true).fgc(whitelt)
                            .add("Text wrapping is ON:\n\n").nil().wrp(wrap::on)
                            .add(testline)
                            .add("\n\n")

                            .jet(bias::left)
                            .add("text: ").idx(test_topic_vars::object1).sav().fgc(whitelt).bgc(reddk)
                            .add("some_text").nop().nil().add(" some text\n\n") // inline text object test
                            .jet(bias::center)
                            .add("text: ").idx(test_topic_vars::object2).sav().fgc(whitelt).bgc(reddk)
                            .add("some_text").nop().nil().add(" some text\n\n") // inline text object test
                            .jet(bias::right)
                            .add("text: ").idx(test_topic_vars::object3).sav().fgc(whitelt).bgc(reddk)
                            .add("some_text").nop().nil().add(" some text\n\n") // inline text object test

                            .jet(bias::left).wrp(wrap::on)
                            .add("text text text ").idx(test_topic_vars::canvas1).wrp(wrap::on)
                            .nop().nil().add(" text text text")
                            .add("\n\n\n")

                            .jet(bias::center).wrp(wrap::off).fgc(clr)
                            .add("Variable-width/-height Characters\n\n")
                            .jet(bias::left).wrp(wrap::on).fgc()
                            .add("left aligned\n").ref(test_topic_vars::canvas2).nop()
                                                .ref(test_topic_vars::canvas2).nop()
                                                .ref(test_topic_vars::canvas2).nop()
                                                .ref(test_topic_vars::canvas2).nop()
                                                .ref(test_topic_vars::canvas2).nop()
                            .nop().nil().eol().eol()

                            .jet(bias::center).wrp(wrap::on)
                            .add("centered\n").ref(test_topic_vars::canvas2).nop()
                                            .ref(test_topic_vars::canvas2).nop()
                                            .ref(test_topic_vars::canvas2).nop()
                                            .ref(test_topic_vars::canvas2).nop()
                                            .ref(test_topic_vars::canvas2).nop()
                            .nop().nil().eol().eol()

                            .jet(bias::right).wrp(wrap::on)
                            .add("right aligned\n").ref(test_topic_vars::canvas2).nop()
                                                .ref(test_topic_vars::canvas2).nop()
                                                .ref(test_topic_vars::canvas2).nop()
                                                .ref(test_topic_vars::canvas2).nop()
                                                .ref(test_topic_vars::canvas2).nop()
                            .nop().nil().eol().eol()

                            .jet(bias::center).wrp(wrap::off).fgc(clr)
                            .add("Embedded Content\n\n")
                            .jet(bias::left).wrp(wrap::on)
                            .idx(test_topic_vars::dynamix1).add("<").fgc(reddk).add("placeholder").fgc().add(">")//.nop()
                            .idx(test_topic_vars::dynamix2).add("<").fgc(reddk).add("placeholder").fgc().add(">")//.nop()
                            .idx(test_topic_vars::dynamix3).add("<").fgc(reddk).add("placeholder").fgc().add(">")//.nop()
                            .nil().eol();


                        // Wikipedia run
                        text wiki = ansi::esc("\n\n")
                            .jet(bias::center).wrp(wrap::off).fgc(clr)
                            .add("Sample Article\n\n")

                            .jet(bias::left).fgc(clr).wrp(wrap::on).add("ANSI escape code\n\n")

                            .nil().add("From Wikipedia, the free encyclopedia\n")
                            .add("  (Redirected from ANSI CSI)\n\n")

                            .jet(bias::center).itc(true)
                            .add("\"ANSI code\" redirects here.\n")
                            .add("For other uses, see ANSI (disambiguation).\n\n")

                            .jet(bias::left).itc(faux).fgc(clr).add("ANSI escape sequences").nil()
                            .add(" are a standard for ").fgc(clr).add("in-band signaling").nil()
                            .add(" to control the cursor location, color, and other options on video ")
                            .fgc(clr).add("text terminals").nil().add(" and ")
                            .fgc(clr).add("terminal emulators").nil().add(". Certain sequences of ")
                            .fgc(clr).add("bytes").nil().add(", most starting with ")
                            .fgc(clr).add("Esc").nil().add(" and '[', are embedded into the text, ")
                            .add("which the terminal looks for and interprets as commands, not as ")
                            .fgc(clr).add("character codes").nil().add(".\n\n")

                            .add("ANSI sequences were introduced in the 1970s to replace vendor-specific sequences "
                                "and became widespread in the computer equipment market by the early 1980s. "
                                "They were used in development, scientific and commercial applications and later by "
                                "the nascent ").fgc(clr).add("bulletin board systems").nil()
                            .add(" to offer improved displays ").ref(test_topic_vars::canvas2).nop()
                            .add("compared to earlier systems lacking cursor movement, "
                                "a primary reason they became a standard adopted by all manufacturers.\n\n")

                            .add("Although hardware text terminals have become increasingly rare in the 21st century, "
                                "the relevance of the ANSI standard persists because most terminal emulators interpret "
                                "at least some of the ANSI escape sequences in output text. A notable exception was ")
                            .fgc(clr).add("DOS").nil().add(" and older versions of the ")
                            .fgc(clr).add("Win32 console").nil().add(" of ")
                            .fgc(clr).add("Microsoft Windows").nil().add(".");

                        text wiki_ru = ansi::eol().eol()
                            .fgc(clr).add("Управляющие символы ANSI").nil()
                            .add(" (англ. ANSI escape code) — символы, встраиваемые в текст, для управления форматом,")
                            .add(" цветом и другими опциями вывода в ")
                            .fgc(clr).add("текстовом терминале").nil().add(".")
                            .add(" Почти все ").fgc(clr).add("эмуляторы терминалов").nil()
                            .add(", предназначенные для отображения текстового вывода с удалённого компьютера и (за исключением ")
                            .fgc(clr).add("Microsoft Windows").nil().add(") для отображения текстового вывода")
                            .add(" локального программного обеспечения, способны интерпретировать по крайней мере некоторые")
                            .add(" управляющие последовательности ANSI.");

                        text wiki_emoji = ansi::fgc(clr).add("\n\n")
                            .add("Emoji\n").nil()
                            .add("Emoji (Japanese: 絵文字, English: /ɪˈmoʊdʒiː/; Japanese: [emodʑi];"
                                " singular emoji, plural emoji or emojis) are ideograms and "
                                "smileys used in electronic messages and web pages. "
                                "Some examples of emoji are 😃, 😭, and 😈. Emoji exist "
                                "in various genres, including facial expressions, common objects, "
                                "places and types of weather, and animals. They are much like "
                                "emoticons, but emoji are pictures rather than typographic "
                                "approximations; the term \"emoji\" in the strict sense refers "
                                "to such pictures which can be represented as encoded characters, "
                                "but it is sometimes applied to messaging stickers by extension."
                                " Originally meaning pictograph, the word emoji comes from "
                                "Japanese e (絵, \"picture\") + moji (文字, \"character\"); "
                                "the resemblance to the English words emotion and emoticon is "
                                "purely coincidental. The ISO 15924 script code for emoji is Zsye."
                                "\n")
                            .fgc(clr).wrp(wrap::off).add("\nSmileys (wrap OFF)\n").nil()
                                .add("😀😃😄😁😆😅😂🤣☺️😊😇🙂🙃😉😌😍\n"
                                    "😏😒😞😔😟😕🙁☹️😣😖😫😩🥺😢😭😤\n"
                                    "🥰😘😗😙😚😋😛😝😜🤪🤨🧐🤓😎🤩🥳\n"
                                    "😡🤬🤯😳🥵🥶😱😨😰😥😓🤗🤔🤭🤫🤥\n"
                                    "😶😐😑😬🙄😯😦😧😮😲🥱😴🤤😪😵🤐\n"
                                    "🥴🤢🤮🤧😷🤒🤕🤑🤠😈👿👹👺🤡💩👻\n"
                                    "💀☠️👽👾🤖🎃😺😸😹😻😼😽🙀😿😾😠\n")
                            .fgc(clr).wrp(wrap::on).add("\nSmileys (wrap ON)\n").nil()
                                .add("😀😃😄😁😆😅😂🤣☺️😊😇🙂🙃😉😌😍"
                                    "😏😒😞😔😟😕🙁☹️😣😖😫😩🥺😢😭😤😠"
                                    "🥰😘😗😙😚😋😛😝😜🤪🤨🧐🤓😎🤩🥳"
                                    "😡🤬🤯😳🥵🥶😱😨😰😥😓🤗🤔🤭🤫🤥"
                                    "😶😐😑😬🙄😯😦😧😮😲🥱😴🤤😪😵🤐"
                                    "🥴🤢🤮🤧😷🤒🤕🤑🤠😈👿👹👺🤡💩👻"
                                    "💀☠️👽👾🤖🎃😺😸😹😻😼😽🙀😿😾\n");

                        text wiki_cjk = ansi::wrp(wrap::off).fgc(clr).eol().eol()
                            .add("端末エミュレータ\n").nil()
                            .add("出典: フリー百科事典『ウィキペディア（Wikipedia）』\n\n")
                            .wrp(wrap::on).fgc(clr).add("端末エミュレータ").nil()
                            .add("（たんまつエミュレータ）")
                            .fgc(clr).add("・端末模倣プログラム").nil()
                            .add("とは、端末として動作するソフトウェアである。端末エミュレータといった場合は、"
                                "DEC VT100のエミュレーションをするソフトウェアをさすことが多い。別称と"
                                "して")
                            .fgc(clr).add("ターミナルエミュレータ").nil()
                            .add("、また特にグラフィカルユーザインタフェース "
                                "(GUI) 環境で用いるものを")
                            .fgc(clr).add("端末ウィンドウ").nil()
                            .add("と呼ぶことがある。キャラクタユーザインタフェースを提供する。\n\n")
                            .fgc(clr).add("概要\n").nil()
                            .add("端末エミュレータは、専用端末の機能を、パーソナルコンピュータ (PC) "
                                "やUnixワークステーションなどで代替するためのソフトウェア。通常はキャ"
                                "ラクタベースのビデオ端末をエミュレートするが、グラフィック端末（xterm"
                                "がTektronix 4014をエミュレートする）やプリンタのエミュレーションを行"
                                "うものもある。\n\n")
                            .add("端末エミュレータを動作させるコンピュータがウィンドウシステムを搭載して"
                                "いる場合、これを利用して一つのコンピュータ上で複数の端末エミュレータを"
                                "同時に稼働させることができることが多い。これは殆どの専用端末では実現で"
                                "きない機能である。\n\n")
                            .add("TCP / IPを介した端末エミュレータの接続にはSSH、Telnet、rlogin等の機"
                                "能を用いる。rloginとTelnetは、パスワードなども含めて、すべての通信内容"
                                "を平文（暗号化されていない状態）で送受信する。極めて限定された用途であれ"
                                "ば、それが必ずしも悪いわけではないが、インターネットを介した接続ではあま"
                                "りに危険な行為である。したがって、近年は、SSHによる接続が一般的である。\n\n")
                            .add("2015年ごろまで、Windows用のSSHクライアントは公式に提供されていなかっ"
                                "たため、端末エミュレータはSSHクライアントを統合したものが多かった。現在"
                                "ではOpenSSH in Windowsが提供されており、PowerShellやコマンドプロン"
                                "プトなどのコマンドラインツールから利用することができる。\n\n")
                            .fgc(clr).add("エミュレートする端末\n").nil()
                            .add("実際の端末における、画面制御やキーボード制御、プリンタ制御など、入出力処"
                                "理には統一された規格が存在しない。現在、端末エミュレータを使用する接続先"
                                "はUnixが多いため、Unixで事実上の標準となっているDEC社のVT100やその上位"
                                "機種のエミュレータが多い。VT100の端末エミュレータやその機能を「VT100互換"
                                "」と呼称する。\n\n"
                                "接続先がメインフレームであれば、IBM 3270、富士通、日立製作所の端末を、"
                                "接続先がIBM AS / 400であればIBM 5250を、エミュレートすることになる。"
                                "それぞれのメーカーから純正のエミュレータが発売されているが、サードパーティ"
                                "製もある。メインフレームの端末の多くは、RS - 232のような単純なシリアルイ"
                                "ンターフェースではなく、インテリジェントなものだったが、その後、シリアル接"
                                "続やイーサネット接続も可能となっている。"
                                "\n\n"
                                "多くの端末はキャラクタしか扱えないが、グラフィックを扱うことができるグラ"
                                "フィック端末もある。例えばxtermがエミュレートするTektronix 4014がその一"
                                "例で、キャラクタとグラフィックのどちらも扱うことができる。日本では、ヤマハ"
                                "のYIS(YGT - 100)もよく知られている。また、コンピュータグラフィックスの黎"
                                "明期には、多くのメインフレームにオプションとして専用のグラフィック端末が用意"
                                "されていた。\n");

                        topic += intro;
                        topic += data;
                        topic += wiki;
                        topic += wiki_ru;
                        topic += wiki_emoji;
                        topic += wiki_cjk;
                    }
                    auto window = ui::cake::ctor();
                    window->template plugin<pro::focus>()
                          ->template plugin<pro::track>()
                          ->template plugin<pro::acryl>()
                          ->template plugin<pro::cache>();
                    auto object0 = window->attach(ui::fork::ctor(axis::Y))
                                         ->colors(whitelt, 0xA0db3700);
                        auto menu = object0->attach(slot::_1, app::shared::custom_menu(true, {}));
                        auto test_stat_area = object0->attach(slot::_2, ui::fork::ctor(axis::Y));
                            auto layers = test_stat_area->attach(slot::_1, ui::cake::ctor());
                                auto scroll = layers->attach(ui::rail::ctor())
                                                    ->colors(cyanlt, bluedk)
                                                    ->config(true, true);
                                    auto object = scroll->attach(ui::post::ctor())
                                                        ->upload(topic)
                                                        ->invoke([&](auto& self)
                                                        {
                                                            self.SUBMIT(tier::release, e2::postrender, canvas)
                                                            {
                                                                static auto counter = 0; counter++;
                                                                static auto textclr =  ansi::bgc(reddk).fgc(whitelt);
                                                                self.content(test_topic_vars::object1) = textclr + " inlined #1: " + std::to_string(counter) + " hits ";
                                                                self.content(test_topic_vars::object2) = textclr + " inlined #2: " + canvas.area().size.str() + " ";
                                                                self.content(test_topic_vars::object3) = textclr + " inlined #3: " + canvas.full().coor.str() + " ";
                                                            };
                                                            //todo
                                                            //self.SUBMIT(tier::general, e2::form::canvas, canvas_ptr)
                                                            //{
                                                            //    self.content(test_topic_vars::dynamix1).lyric = self.content(test_topic_vars::dynamix2).lyric;
                                                            //    self.content(test_topic_vars::dynamix2).lyric = self.content(test_topic_vars::dynamix3).lyric;
                                                            //    self.content(test_topic_vars::dynamix3).lyric = canvas_ptr;
                                                            //};
                                                        });
                                auto scroll_bars = layers->attach(ui::fork::ctor());
                                    auto vt = scroll_bars->attach(slot::_2, ui::grip<axis::Y>::ctor(scroll));
                                    auto hz = test_stat_area->attach(slot::_2, ui::grip<axis::X>::ctor(scroll));
                        auto& a = object->lyric(test_topic_vars::canvas1);
                            a.mark().fgc(0xFF000000);
                            a.size({ 40, 9 });
                            a.grad(rgba{ 0xFFFFFF00 }, rgba{ 0x40FFFFFF });
                            para t{ "ARBITRARY SIZE BLOCK" };
                            a.text((a.size() - twod{ t.length(), 0 }) / 2, t.shadow());
                        auto& b = object->lyric(test_topic_vars::canvas2);
                            b.mark().fgc(0xFF000000);
                            b.size({ 6, 2 });
                            b.grad(rgba{ 0xFFFFFF00 }, rgba{ 0x40FFFFFF });
                            b[{5, 0}].alpha(0);
                            b[{5, 1}].alpha(0);
                    return window;
                    break;
                }
                case app::shared::objs::Strobe:
                {
                    auto window = ui::cake::ctor();
                    auto strob = window->template plugin<pro::focus>()
                          ->attach(ui::mock::ctor());
                    auto strob_shadow = ptr::shadow(strob);
                    bool stobe_state = true;
                    strob->SUBMIT_BYVAL(tier::general, e2::tick, now)
                    {
                        stobe_state = !stobe_state;
                        if (auto strob = strob_shadow.lock())
                        {
                            strob->color(0x00, stobe_state ? 0xFF000000 : 0xFFFFFFFF);
                            strob->deface();
                        }
                    };
                    return window;
                    break;
                }
                case app::shared::objs::RefreshRate:
                {
                    auto window = ui::cake::ctor();
                    window->template plugin<pro::focus>()
                          ->attach(ui::stem_rate<tier::general, decltype(e2::config::fps)>::ctor("Set frame rate", 1, 200, "fps"))
                          ->colors(0xFFFFFFFF, bluedk)
                          ->invoke([&](auto& boss)
                          {
                              boss.keybd.accept(true);
                          });
                    return window;
                    break;
                }
                case app::shared::objs::Truecolor:
                {
                #pragma region samples
                    //todo put all ansi art into external files
                    text r_grut00 = ansi::wrp(wrap::off).rlf(feed::fwd).jet(bias::center).add(
                        "\033[0m\033[s"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;1;1;1m▄\033[48;2;0;0;0m\033[38;2;9;8;8m▄\033[48;2;0;0;0m\033[38;2;20;17;15m▄\033[48;2;0;0;0m\033[38;2;19;16;13m▄\033[48;2;0;0;0m\033[38;2;7;6;5m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;1;1;1m▄\033[48;2;0;0;0m\033[38;2;7;7;6m▄\033[48;2;0;0;0m\033[38;2;6;6;5m▄\033[48;2;0;0;0m\033[38;2;1;1;1m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;2;2;2m▄\033[48;2;3;3;3m\033[38;2;25;23;21m▄\033[48;2;21;18;17m\033[38;2;80;60;45m▄\033[48;2;64;49;38m\033[38;2;124;88;64m▄\033[48;2;89;64;48m\033[38;2;136;94;68m▄\033[48;2;84;61;46m\033[38;2;134;95;69m▄\033[48;2;45;35;28m\033[38;2;99;72;53m▄\033[48;2;8;7;7m\033[38;2;32;26;23m▄\033[48;2;0;0;0m\033[38;2;2;2;1m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;1;1;1m▄\033[48;2;4;4;4m\033[38;2;26;24;21m▄\033[48;2;34;31;27m\033[38;2;104;79;61m▄\033[48;2;77;62;49m\033[38;2;136;100;74m▄\033[48;2;73;58;47m\033[38;2;137;101;75m▄\033[48;2;36;32;27m\033[38;2;107;80;61m▄\033[48;2;9;9;8m\033[38;2;58;47;40m▄\033[48;2;0;0;0m\033[38;2;14;13;12m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;8;8;8m▄\033[48;2;17;16;15m\033[38;2;54;43;37m▄\033[48;2;67;50;41m\033[38;2;104;78;60m▄\033[48;2;121;89;67m\033[38;2;134;103;82m▄\033[48;2;141;105;78m\033[38;2;144;115;90m▄\033[48;2;152;113;84m\033[38;2;153;122;95m▄\033[48;2;151;111;83m\033[38;2;148;117;92m▄\033[48;2;126;91;67m\033[38;2;134;104;80m▄\033[48;2;65;52;42m\033[38;2;92;70;54m▄\033[48;2;9;9;8m\033[38;2;22;20;18m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;6;6;5m\033[38;2;14;14;13m▄\033[48;2;61;50;40m\033[38;2;88;68;52m▄\033[48;2;134;98;72m\033[38;2;144;108;83m▄\033[48;2;145;109;82m\033[38;2;149;120;95m▄\033[48;2;153;117;89m\033[38;2;159;132;106m▄\033[48;2;140;105;79m\033[38;2;157;129;103m▄\033[48;2;112;83;63m\033[38;2;144;111;86m▄\033[48;2;58;48;39m\033[38;2;110;82;62m▄\033[48;2;9;9;8m\033[38;2;43;37;33m▄\033[48;2;0;0;0m\033[38;2;5;5;4m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;1;1;1m▄\033[48;2;3;3;3m\033[38;2;17;15;13m▄\033[48;2;33;28;24m\033[38;2;73;57;45m▄\033[48;2;94;72;55m\033[38;2;125;100;79m▄\033[48;2;126;101;81m\033[38;2;127;109;92m▄\033[48;2;129;107;89m\033[38;2;113;99;87m▄\033[48;2;140;117;97m\033[38;2;123;106;93m▄\033[48;2;143;120;100m\033[38;2;132;112;97m▄\033[48;2;136;113;94m\033[38;2;132;114;97m▄\033[48;2;132;108;87m\033[38;2;139;116;97m▄\033[48;2;116;89;69m\033[38;2;133;103;81m▄\033[48;2;38;33;28m\033[38;2;57;47;41m▄\033[48;2;1;1;1m\033[38;2;4;4;4m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;1;0m\033[38;2;1;1;1m▄\033[48;2;25;23;20m\033[38;2;41;35;30m▄\033[48;2;106;80;61m\033[38;2;119;90;67m▄\033[48;2;144;114;90m\033[38;2;135;109;89m▄\033[48;2;148;124;103m\033[38;2;144;123;104m▄\033[48;2;150;129;110m\033[38;2;140;122;107m▄\033[48;2;153;133;113m\033[38;2;139;122;107m▄\033[48;2;151;126;103m\033[38;2;142;123;106m▄\033[48;2;141;109;84m\033[38;2;146;122;99m▄\033[48;2;86;64;50m\033[38;2;125;94;71m▄\033[48;2;29;27;25m\033[38;2;70;55;45m▄\033[48;2;2;2;2m\033[38;2;16;15;15m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;5;5;5m\033[38;2;19;16;15m▄\033[48;2;43;36;30m\033[38;2;82;64;50m▄\033[48;2;112;87;67m\033[38;2;130;105;82m▄\033[48;2;138;114;95m\033[38;2;128;109;92m▄\033[48;2;111;94;84m\033[38;2;89;77;70m▄\033[48;2;99;87;79m\033[38;2;88;76;70m▄\033[48;2;104;91;81m\033[38;2;98;87;79m▄\033[48;2;129;111;98m\033[38;2;122;107;96m▄\033[48;2;137;119;104m\033[38;2;124;108;97m▄\033[48;2;148;127;108m\033[38;2;142;122;106m▄\033[48;2;143;113;89m\033[38;2;151;123;97m▄\033[48;2;74;60;50m\033[38;2;91;70;54m▄\033[48;2;8;8;8m\033[38;2;14;13;13m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;3;3;3m\033[38;2;5;5;5m▄\033[48;2;54;44;36m\033[38;2;61;49;39m▄\033[48;2;123;95;71m\033[38;2;118;93;72m▄\033[48;2;125;103;86m\033[38;2;118;99;84m▄\033[48;2;134;116;101m\033[38;2;113;99;91m▄\033[48;2;130;114;101m\033[38;2;117;102;94m▄\033[48;2;128;111;99m\033[38;2;122;106;96m▄\033[48;2;134;117;102m\033[38;2;125;109;98m▄\033[48;2;145;125;107m\033[38;2;134;117;104m▄\033[48;2;145;122;99m\033[38;2;140;121;104m▄\033[48;2;104;78;61m\033[38;2;130;109;90m▄\033[48;2;47;39;35m\033[38;2;87;67;53m▄\033[48;2;5;5;5m\033[38;2;29;25;22m▄\033[48;2;0;0;0m\033[38;2;2;2;2m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;3;3;3m\033[38;2;17;16;16m▄\033[48;2;48;41;36m\033[38;2;83;64;51m▄\033[48;2;116;88;67m\033[38;2;127;102;80m▄\033[48;2;134;111;89m\033[38;2;117;100;82m▄\033[48;2;105;91;78m\033[38;2;93;80;69m▄\033[48;2;75;65;59m\033[38;2;78;69;64m▄\033[48;2;85;76;70m\033[38;2;91;82;74m▄\033[48;2;91;81;74m\033[38;2;85;75;69m▄\033[48;2;107;94;85m\033[38;2;97;86;78m▄\033[48;2;122;108;98m\033[38;2;115;101;92m▄\033[48;2;134;116;104m\033[38;2;135;118;105m▄\033[48;2;152;124;101m\033[38;2;151;126;103m▄\033[48;2;108;82;63m\033[38;2;125;98;75m▄\033[48;2;21;20;18m\033[38;2;33;28;26m▄\033[48;2;0;0;0m\033[38;2;1;1;1m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;8;8;8m\033[38;2;11;10;9m▄\033[48;2;68;55;45m\033[38;2;74;61;50m▄\033[48;2;106;87;72m\033[38;2;104;87;73m▄\033[48;2;108;93;81m\033[38;2;92;81;72m▄\033[48;2;100;87;79m\033[38;2;93;81;73m▄\033[48;2;104;91;84m\033[38;2;97;85;79m▄\033[48;2;116;102;93m\033[38;2;110;96;89m▄\033[48;2;118;103;94m\033[38;2;111;98;90m▄\033[48;2;129;111;100m\033[38;2;120;103;94m▄\033[48;2;129;112;98m\033[38;2;120;103;92m▄\033[48;2;131;113;97m\033[38;2;124;108;96m▄\033[48;2;118;96;76m\033[38;2;125;105;88m▄\033[48;2;59;46;37m\033[38;2;92;71;56m▄\033[48;2;12;12;11m\033[38;2;35;30;26m▄\033[48;2;0;0;0m\033[38;2;3;3;3m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;5;5;5m\033[38;2;19;18;18m▄\033[48;2;52;44;39m\033[38;2;89;67;52m▄\033[48;2;110;85;66m\033[38;2;119;95;74m▄\033[48;2;110;93;77m\033[38;2;88;77;66m▄\033[48;2;96;84;73m\033[38;2;81;72;65m▄\033[48;2;86;75;67m\033[38;2;78;69;63m▄\033[48;2;78;69;65m\033[38;2;79;71;67m▄\033[48;2;80;72;67m \033[48;2;84;75;69m\033[38;2;88;79;73m▄\033[48;2;96;85;78m\033[38;2;106;93;86m▄\033[48;2;107;94;86m\033[38;2;113;99;91m▄\033[48;2;133;115;104m\033[38;2;130;112;102m▄\033[48;2;148;126;107m\033[38;2;146;125;108m▄\033[48;2;134;106;82m\033[38;2;144;115;91m▄\033[48;2;50;42;35m\033[38;2;67;53;42m▄\033[48;2;3;3;3m\033[38;2;7;7;7m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;14;13;12m\033[38;2;19;17;15m▄\033[48;2;82;67;55m\033[38;2;94;74;58m▄\033[48;2;104;88;74m\033[38;2;115;94;76m▄\033[48;2;88;78;68m\033[38;2;95;82;71m▄\033[48;2;81;69;63m\033[38;2;87;74;66m▄\033[48;2;91;79;73m\033[38;2;90;78;72m▄\033[48;2;108;94;86m\033[38;2;105;91;84m▄\033[48;2;112;97;90m\033[38;2;115;100;92m▄\033[48;2;110;96;88m\033[38;2;114;100;92m▄\033[48;2;109;94;85m\033[38;2;107;93;86m▄\033[48;2;115;101;91m\033[38;2;108;94;86m▄\033[48;2;120;103;89m\033[38;2;116;101;90m▄\033[48;2;116;93;72m\033[38;2;127;107;87m▄\033[48;2;70;56;45m\033[38;2;103;79;61m▄\033[48;2;14;13;12m\033[38;2;38;33;29m▄\033[48;2;0;0;0m\033[38;2;2;2;2m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;4;4;4m\033[38;2;12;12;12m▄\033[48;2;48;40;34m\033[38;2;81;62;47m▄\033[48;2;118;87;64m\033[38;2;131;100;74m▄\033[48;2;106;87;70m\033[38;2;93;79;67m▄\033[48;2;77;68;60m\033[38;2;69;61;56m▄\033[48;2;70;62;57m\033[38;2;65;58;53m▄\033[48;2;67;59;56m\033[38;2;62;55;49m▄\033[48;2;75;68;63m\033[38;2;75;68;62m▄\033[48;2;90;82;77m\033[38;2;93;84;78m▄\033[48;2;91;81;76m\033[38;2;98;84;79m▄\033[48;2;113;99;91m\033[38;2;112;98;91m▄\033[48;2;118;103;95m\033[38;2;118;102;95m▄\033[48;2;128;110;100m\033[38;2;123;106;99m▄\033[48;2;146;124;108m\033[38;2;149;127;110m▄\033[48;2;154;124;100m\033[38;2;153;124;100m▄\033[48;2;86;66;50m\033[38;2;108;83;63m▄\033[48;2;13;13;12m\033[38;2;21;19;17m▄\033[48;2;0;0;0m\033[38;2;1;1;1m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;1;1;1m \033[48;2;25;22;19m\033[38;2;29;26;23m▄\033[48;2;104;83;66m\033[38;2;107;85;67m▄\033[48;2;121;98;78m\033[38;2;124;99;79m▄\033[48;2;110;94;79m\033[38;2;102;86;73m▄\033[48;2;86;73;65m\033[38;2;92;78;69m▄\033[48;2;90;77;71m\033[38;2;93;80;72m▄\033[48;2;101;87;80m\033[38;2;94;80;73m▄\033[48;2;119;103;95m\033[38;2;113;96;88m▄\033[48;2;121;106;99m\033[38;2;122;107;99m▄\033[48;2;109;94;87m\033[38;2;116;101;94m▄\033[48;2;107;94;86m\033[38;2;107;93;85m▄\033[48;2;102;87;79m\033[38;2;110;94;86m▄\033[48;2;123;105;90m\033[38;2;127;107;93m▄\033[48;2;116;90;69m\033[38;2;134;106;83m▄\033[48;2;69;56;47m\033[38;2;97;72;53m▄\033[48;2;10;9;9m\033[38;2;33;30;27m▄\033[48;2;0;0;0m\033[38;2;1;1;0m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;2;2;2m\033[38;2;8;8;8m▄\033[48;2;36;31;27m\033[38;2;67;52;42m▄\033[48;2;107;81;61m\033[38;2;123;93;70m▄\033[48;2;136;107;82m\033[38;2;138;111;88m▄\033[48;2;86;73;64m\033[38;2;91;78;65m▄\033[48;2;60;53;49m\033[38;2;62;55;50m▄\033[48;2;61;54;49m\033[38;2;64;55;50m▄\033[48;2;68;60;54m\033[38;2;65;55;51m▄\033[48;2;78;69;63m\033[38;2;81;70;64m▄\033[48;2;87;76;70m\033[38;2;84;71;64m▄\033[48;2;104;91;84m\033[38;2;95;81;74m▄\033[48;2;102;85;79m\033[38;2;106;87;81m▄\033[48;2;121;102;96m\033[38;2;128;108;101m▄\033[48;2;129;111;104m\033[38;2;135;117;109m▄\033[48;2;142;124;109m\033[38;2;133;116;103m▄\033[48;2;146;121;97m\033[38;2;136;114;95m▄\033[48;2;122;95;74m\033[38;2;129;103;82m▄\033[48;2;32;27;24m\033[38;2;43;36;31m▄\033[48;2;1;1;1m\033[38;2;3;3;3m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "");
                    text r_grut01 = ""\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;1;1;1m\033[38;2;2;2;2m▄\033[48;2;35;30;26m\033[38;2;38;33;28m▄\033[48;2;114;90;71m\033[38;2;116;94;73m▄\033[48;2;127;101;80m\033[38;2;128;103;81m▄\033[48;2;103;85;73m\033[38;2;116;95;79m▄\033[48;2;93;79;69m\033[38;2;94;80;69m▄\033[48;2;91;78;71m\033[38;2;87;73;66m▄\033[48;2;94;80;73m\033[38;2;92;76;69m▄\033[48;2;111;93;84m\033[38;2;105;87;79m▄\033[48;2;128;108;97m\033[38;2;130;109;98m▄\033[48;2;124;106;98m\033[38;2;135;113;104m▄\033[48;2;127;109;102m\033[38;2;139;118;109m▄\033[48;2;111;92;85m\033[38;2;119;99;88m▄\033[48;2;127;105;93m\033[38;2;128;106;91m▄\033[48;2;148;120;97m\033[38;2;147;120;99m▄\033[48;2;123;93;69m\033[38;2;130;100;77m▄\033[48;2;76;64;54m\033[38;2;98;73;56m▄\033[48;2;12;12;11m\033[38;2;50;46;42m▄\033[48;2;0;0;0m\033[38;2;5;5;4m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;1;1;1m▄\033[48;2;3;3;3m\033[38;2;19;18;17m▄\033[48;2;34;30;27m\033[38;2;73;60;49m▄\033[48;2;102;78;60m\033[38;2;119;89;65m▄\033[48;2;136;105;80m\033[38;2;135;106;80m▄\033[48;2;141;116;92m\033[38;2;142;118;95m▄\033[48;2;99;85;72m\033[38;2;110;95;81m▄\033[48;2;69;60;55m\033[38;2;77;67;59m▄\033[48;2;63;54;50m\033[38;2;67;57;51m▄\033[48;2;66;56;51m\033[38;2;67;56;51m▄\033[48;2;77;64;58m\033[38;2;75;62;56m▄\033[48;2;89;75;68m\033[38;2;90;75;68m▄\033[48;2;95;79;71m\033[38;2;94;76;69m▄\033[48;2;116;96;89m\033[38;2;120;99;92m▄\033[48;2;130;108;100m\033[38;2;134;111;103m▄\033[48;2;135;116;108m\033[38;2;132;115;106m▄\033[48;2;135;118;104m\033[38;2;133;116;104m▄\033[48;2;127;108;92m\033[38;2;133;113;96m▄\033[48;2;134;109;88m\033[38;2;143;117;93m▄\033[48;2;55;44;36m\033[38;2;63;50;41m▄\033[48;2;5;5;5m\033[38;2;6;6;6m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;2;2;2m\033[38;2;1;1;1m▄\033[48;2;38;33;28m\033[38;2;36;30;26m▄\033[48;2;118;95;74m\033[38;2;118;93;72m▄\033[48;2;124;100;80m\033[38;2;127;103;81m▄\033[48;2;120;99;82m\033[38;2;109;89;74m▄\033[48;2;102;86;73m\033[38;2;102;87;74m▄\033[48;2;88;74;66m\033[38;2;90;77;67m▄\033[48;2;87;73;65m\033[38;2;80;66;59m▄\033[48;2;100;82;74m\033[38;2;92;75;68m▄\033[48;2;116;94;86m\033[38;2;109;86;79m▄\033[48;2;141;116;105m\033[38;2;127;100;92m▄\033[48;2;144;120;110m\033[38;2;141;114;104m▄\033[48;2;128;106;92m\033[38;2;141;117;100m▄\033[48;2;129;105;86m\033[38;2;141;117;95m▄\033[48;2;137;111;89m\033[38;2;129;104;81m▄\033[48;2;121;93;71m\033[38;2;110;84;64m▄\033[48;2;103;75;55m\033[38;2;99;73;54m▄\033[48;2;94;76;62m\033[38;2;111;82;63m▄\033[48;2;40;38;35m\033[38;2;95;79;68m▄\033[48;2;3;3;3m\033[38;2;29;28;26m▄\033[48;2;0;0;0m\033[38;2;2;2;1m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;9;9;8m▄\033[48;2;14;13;12m\033[38;2;59;50;43m▄\033[48;2;65;57;50m\033[38;2;107;83;67m▄\033[48;2;105;82;64m\033[38;2;106;78;59m▄\033[48;2;116;86;62m\033[38;2;105;80;60m▄\033[48;2;123;98;75m\033[38;2;113;90;69m▄\033[48;2;137;116;93m\033[38;2;107;89;72m▄\033[48;2;119;102;86m\033[38;2;126;109;95m▄\033[48;2;93;80;70m\033[38;2;115;100;86m▄\033[48;2;81;69;60m\033[38;2;86;70;60m▄\033[48;2;67;56;50m\033[38;2;62;48;44m▄\033[48;2;70;56;51m\033[38;2;59;45;42m▄\033[48;2;75;60;55m\033[38;2;69;54;50m▄\033[48;2;94;75;69m\033[38;2;89;70;65m▄\033[48;2;117;94;88m\033[38;2;114;92;86m▄\033[48;2;131;110;102m\033[38;2;126;106;97m▄\033[48;2;131;115;105m\033[38;2;136;119;108m▄\033[48;2;138;121;107m\033[38;2;153;134;118m▄\033[48;2;150;128;105m\033[38;2;153;129;106m▄\033[48;2;150;123;96m\033[38;2;146;119;92m▄\033[48;2;65;52;42m\033[38;2;60;48;39m▄\033[48;2;7;7;7m\033[38;2;6;6;6m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;1;1;1m\033[38;2;1;1;0m▄\033[48;2;32;27;24m\033[38;2;25;22;19m▄\033[48;2;118;92;70m\033[38;2;112;88;67m▄\033[48;2;134;106;82m\033[38;2;140;109;83m▄\033[48;2;118;93;76m\033[38;2;127;100;80m▄\033[48;2;99;83;71m\033[38;2;101;85;71m▄\033[48;2;92;78;69m\033[38;2;90;78;68m▄\033[48;2;74;60;54m\033[38;2;72;59;52m▄\033[48;2;80;63;57m\033[38;2;69;52;48m▄\033[48;2;103;81;74m\033[38;2;89;69;63m▄\033[48;2;121;94;87m\033[38;2;117;90;83m▄\033[48;2;135;106;96m\033[38;2;137;108;97m▄\033[48;2;147;121;105m\033[38;2;142;115;101m▄\033[48;2;148;127;105m\033[38;2;139;116;96m▄\033[48;2;127;104;83m\033[38;2;127;103;82m▄\033[48;2;105;80;61m\033[38;2;131;100;74m▄\033[48;2;99;72;55m\033[38;2;144;105;76m▄\033[48;2;125;90;68m\033[38;2;173;130;94m▄\033[48;2;130;97;75m\033[38;2;167;123;90m▄\033[48;2;76;63;53m\033[38;2;127;94;71m▄\033[48;2;12;11;11m\033[38;2;60;51;44m▄\033[48;2;0;0;0m\033[38;2;16;16;15m▄\033[48;2;0;0;0m\033[38;2;4;4;4m▄\033[48;2;0;0;0m\033[38;2;3;3;3m▄\033[48;2;0;0;0m\033[38;2;3;3;2m▄\033[48;2;0;0;0m\033[38;2;3;2;2m▄\033[48;2;0;0;0m\033[38;2;3;2;2m▄\033[48;2;0;0;0m\033[38;2;3;3;3m▄\033[48;2;0;0;0m\033[38;2;3;3;3m▄\033[48;2;0;0;0m\033[38;2;4;4;4m▄\033[48;2;0;0;0m\033[38;2;6;6;6m▄\033[48;2;0;0;0m\033[38;2;9;8;7m▄\033[48;2;0;0;0m\033[38;2;13;12;11m▄\033[48;2;0;0;0m\033[38;2;24;22;20m▄\033[48;2;4;4;4m\033[38;2;54;49;42m▄\033[48;2;43;40;37m\033[38;2;134;106;82m▄\033[48;2;128;96;73m\033[38;2;182;136;98m▄\033[48;2;143;107;82m\033[38;2;187;140;101m▄\033[48;2;121;90;70m\033[38;2;165;125;91m▄\033[48;2;98;74;58m\033[38;2;119;91;71m▄\033[48;2;98;78;61m \033[48;2;110;93;77m\033[38;2;96;79;63m▄\033[48;2;139;124;109m\033[38;2;128;111;93m▄\033[48;2;126;110;95m\033[38;2;132;113;94m▄\033[48;2;85;68;59m\033[38;2;95;77;66m▄\033[48;2;58;44;40m\033[38;2;61;46;43m▄\033[48;2;57;43;40m\033[38;2;58;45;42m▄\033[48;2;66;50;47m\033[38;2;68;50;49m▄\033[48;2;85;64;61m\033[38;2;86;65;63m▄\033[48;2;111;88;83m\033[38;2;105;82;78m▄\033[48;2;121;101;93m\033[38;2;112;95;86m▄\033[48;2;136;120;108m\033[38;2;131;114;99m▄\033[48;2;152;132;114m\033[38;2;159;133;109m▄\033[48;2;157;130;104m\033[38;2;161;128;99m▄\033[48;2;144;118;92m\033[38;2;131;105;80m▄\033[48;2;52;44;37m\033[38;2;43;39;34m▄\033[48;2;4;5;4m\033[38;2;3;3;2m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;17;15;14m\033[38;2;9;9;8m▄\033[48;2;100;80;62m\033[38;2;82;68;56m▄\033[48;2;145;114;86m\033[38;2;149;119;91m▄\033[48;2;131;104;82m\033[38;2;144;116;91m▄\033[48;2;103;86;71m\033[38;2;118;98;81m▄\033[48;2;86;74;63m\033[38;2;85;72;62m▄\033[48;2;75;62;54m\033[38;2;77;64;55m▄\033[48;2;66;51;46m\033[38;2;70;55;49m▄\033[48;2;84;65;58m\033[38;2;81;62;56m▄\033[48;2;110;85;75m\033[38;2;102;78;67m▄\033[48;2;140;113;99m\033[38;2;132;104;87m▄\033[48;2;143;116;100m\033[38;2;143;115;94m▄\033[48;2;136;110;90m\033[38;2;144;112;87m▄\033[48;2;140;108;81m\033[38;2;176;136;101m▄\033[48;2;172;129;94m\033[38;2;201;156;114m▄\033[48;2;193;145;103m\033[38;2;203;155;113m▄\033[48;2;199;153;113m\033[38;2;197;150;109m▄\033[48;2;190;145;105m\033[38;2;192;143;102m▄\033[48;2;167;125;90m\033[38;2;187;140;99m▄\033[48;2;132;104;83m\033[38;2;174;129;91m▄\033[48;2;85;71;60m\033[38;2;159;117;84m▄\033[48;2;56;45;37m\033[38;2;143;103;72m▄\033[48;2;48;38;31m\033[38;2;135;97;68m▄\033[48;2;45;37;29m\033[38;2;123;87;62m▄\033[48;2;42;33;26m\033[38;2;113;79;56m▄\033[48;2;40;32;26m\033[38;2;121;87;63m▄\033[48;2;44;36;31m\033[38;2;133;98;70m▄\033[48;2;49;38;29m\033[38;2;142;103;71m▄\033[48;2;57;45;34m\033[38;2;152;110;76m▄\033[48;2;64;51;40m\033[38;2;154;112;78m▄\033[48;2;72;58;45m\033[38;2;155;113;78m▄\033[48;2;85;69;54m\033[38;2;153;112;77m▄\033[48;2;105;85;66m\033[38;2;155;113;79m▄\033[48;2;140;107;79m\033[38;2;160;117;81m▄\033[48;2;178;133;95m\033[38;2;168;123;85m▄\033[48;2;187;139;98m\033[38;2;164;119;82m▄\033[48;2;188;141;100m\033[38;2;167;123;85m▄\033[48;2;181;138;100m\033[38;2;170;126;91m▄\033[48;2;156;118;88m\033[38;2;161;121;87m▄\033[48;2;104;80;61m\033[38;2;123;94;70m▄\033[48;2;83;66;51m\033[38;2;86;67;52m▄\033[48;2;108;89;71m\033[38;2;90;71;55m▄\033[48;2;123;103;82m\033[38;2;109;87;69m▄\033[48;2;92;76;63m\033[38;2;83;67;55m▄\033[48;2;63;49;45m\033[38;2;59;47;42m▄\033[48;2;56;42;41m\033[38;2;54;41;39m▄\033[48;2;60;45;44m\033[38;2;58;44;43m▄\033[48;2;79;60;57m\033[38;2;70;54;52m▄\033[48;2;96;75;71m\033[38;2;85;69;65m▄\033[48;2;108;92;83m\033[38;2;109;91;79m▄\033[48;2;134;113;96m\033[38;2;136;110;89m▄\033[48;2;158;129;102m\033[38;2;157;125;97m▄\033[48;2;164;129;97m\033[38;2;130;98;73m▄\033[48;2;109;85;65m\033[38;2;91;69;54m▄\033[48;2;42;39;36m\033[38;2;48;41;34m▄\033[48;2;3;3;3m\033[38;2;4;4;4m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;5;5;4m\033[38;2;4;4;4m▄\033[48;2;68;61;54m\033[38;2;62;54;47m▄\033[48;2;140;110;85m\033[38;2;151;126;102m▄\033[48;2;152;123;96m\033[38;2;140;107;83m▄\033[48;2;134;110;89m\033[38;2;141;112;88m▄\033[48;2;97;82;69m\033[38;2;118;98;79m▄\033[48;2;68;56;49m\033[38;2;76;63;54m▄\033[48;2;69;56;49m\033[38;2;63;51;46m▄\033[48;2;72;55;50m\033[38;2;68;54;49m▄\033[48;2;91;70;60m\033[38;2;81;63;56m▄\033[48;2;119;91;75m\033[38;2;112;86;71m▄\033[48;2;136;104;81m\033[38;2;145;112;87m▄\033[48;2;165;128;97m\033[38;2;183;143;107m▄\033[48;2;198;156;117m\033[38;2;198;150;110m▄\033[48;2;204;158;114m\033[38;2;198;148;106m▄\033[48;2;199;149;106m\033[38;2;202;153;110m▄\033[48;2;195;146;104m\033[38;2;200;150;109m▄\033[48;2;195;146;104m\033[38;2;201;153;111m▄\033[48;2;192;143;101m\033[38;2;196;148;106m▄\033[48;2;186;136;94m\033[38;2;189;140;99m▄\033[48;2;175;126;85m\033[38;2;176;126;87m▄\033[48;2;162;113;75m\033[38;2;163;114;77m▄\033[48;2;152;105;70m\033[38;2;152;102;68m▄\033[48;2;139;93;62m\033[38;2;144;96;63m▄\033[48;2;137;93;64m\033[38;2;161;110;75m▄\033[48;2;164;115;78m\033[38;2;181;127;85m▄\033[48;2;176;125;83m\033[38;2;187;136;91m▄\033[48;2;181;130;89m\033[38;2;193;142;99m▄\033[48;2;182;132;90m\033[38;2;188;138;95m▄\033[48;2;180;132;92m\033[38;2;187;139;97m▄\033[48;2;175;127;87m\033[38;2;185;136;96m▄\033[48;2;170;123;85m\033[38;2;181;133;93m▄\033[48;2;164;119;82m\033[38;2;168;122;83m▄\033[48;2;160;116;81m\033[38;2;159;115;81m▄\033[48;2;154;110;76m\033[38;2;155;113;81m▄\033[48;2;152;109;75m\033[38;2;151;108;76m▄\033[48;2;147;104;71m\033[38;2;143;103;71m▄\033[48;2;149;107;74m\033[38;2;135;95;65m▄\033[48;2;147;110;76m\033[38;2;131;93;65m▄\033[48;2;132;100;72m\033[38;2;134;99;71m▄\033[48;2;107;84;65m\033[38;2;118;90;67m▄\033[48;2;85;66;51m\033[38;2;97;75;57m▄\033[48;2;87;68;54m\033[38;2;78;61;48m▄\033[48;2;68;53;45m\033[38;2;61;47;41m▄\033[48;2;53;41;38m\033[38;2;51;40;37m▄\033[48;2;51;40;38m\033[38;2;48;37;35m▄\033[48;2;55;44;42m\033[38;2;50;39;37m▄\033[48;2;66;52;50m\033[38;2;60;48;46m▄\033[48;2;79;65;60m\033[38;2;86;73;65m▄\033[48;2;112;93;78m\033[38;2;130;110;91m▄\033[48;2;135;108;85m\033[38;2;141;114;89m▄\033[48;2;136;107;82m\033[38;2;114;87;69m▄\033[48;2;82;60;50m\033[38;2;101;79;66m▄\033[48;2;122;100;81m\033[38;2;154;130;104m▄\033[48;2;59;49;40m\033[38;2;57;47;39m▄\033[48;2;6;6;5m\033[38;2;5;5;5m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;4;4;4m\033[38;2;3;3;3m▄\033[48;2;60;49;41m\033[38;2;56;49;42m▄\033[48;2;149;125;100m\033[38;2;142;117;90m▄\033[48;2;135;109;91m\033[38;2;134;109;88m▄\033[48;2;147;116;91m\033[38;2;147;118;95m▄\033[48;2;138;111;88m\033[38;2;158;132;107m▄\033[48;2;106;89;74m\033[38;2;126;104;86m▄\033[48;2;72;59;54m\033[38;2;80;65;57m▄\033[48;2;65;51;47m\033[38;2;70;56;50m▄\033[48;2;77;61;55m\033[38;2;78;61;54m▄\033[48;2;111;85;73m\033[38;2;123;94;77m▄\033[48;2;165;130;100m\033[38;2;174;134;100m▄\033[48;2;193;147;108m\033[38;2;195;148;108m▄\033[48;2;197;148;107m\033[38;2;203;155;115m▄\033[48;2;207;159;118m\033[38;2;207;159;119m▄\033[48;2;207;159;117m\033[38;2;205;159;117m▄\033[48;2;205;156;115m\033[38;2;209;163;121m▄\033[48;2;206;157;117m\033[38;2;211;166;124m▄\033[48;2;203;155;113m\033[38;2;199;151;108m▄\033[48;2;200;151;110m\033[38;2;198;149;107m▄\033[48;2;182;131;93m\033[38;2;190;141;102m▄\033[48;2;169;119;82m\033[38;2;178;128;91m▄\033[48;2;149;99;66m\033[38;2;157;107;74m▄\033[48;2;153;104;70m\033[38;2;172;118;81m▄\033[48;2;180;125;84m\033[38;2;190;136;93m▄\033[48;2;191;137;93m\033[38;2;200;148;103m▄\033[48;2;195;143;99m\033[38;2;197;145;100m▄\033[48;2;195;144;100m\033[38;2;198;145;101m▄\033[48;2;189;140;98m\033[38;2;197;150;108m▄\033[48;2;186;138;98m\033[38;2;184;138;98m▄\033[48;2;182;134;94m\033[38;2;173;127;89m▄\033[48;2;172;124;86m\033[38;2;170;125;90m▄\033[48;2;164;119;84m\033[38;2;165;121;87m▄\033[48;2;158;116;84m\033[38;2;152;110;77m▄\033[48;2;148;107;75m\033[38;2;153;112;80m▄\033[48;2;152;111;81m\033[38;2;152;111;80m▄\033[48;2;147;107;77m\033[38;2;144;105;74m▄\033[48;2;138;99;71m\033[38;2;136;97;68m▄\033[48;2;126;88;61m\033[38;2;131;93;66m▄\033[48;2;123;86;60m\033[38;2;125;89;63m▄\033[48;2;126;95;69m\033[38;2;122;87;61m▄\033[48;2;107;82;60m\033[38;2;118;88;65m▄\033[48;2;86;67;53m\033[38;2;112;87;66m▄\033[48;2;66;51;44m\033[38;2;88;69;56m▄\033[48;2;50;39;36m\033[38;2;59;46;41m▄\033[48;2;45;35;33m\033[38;2;60;50;45m▄\033[48;2;59;49;45m\033[38;2;66;54;48m▄\033[48;2;64;53;49m\033[38;2;73;60;53m▄\033[48;2;96;81;72m\033[38;2;101;85;73m▄\033[48;2;140;118;98m\033[38;2;142;117;95m▄\033[48;2;151;123;97m\033[38;2;158;126;97m▄\033[48;2;145;115;91m\033[38;2;175;142;111m▄\033[48;2;145;115;93m\033[38;2;164;126;95m▄\033[48;2;157;128;102m\033[38;2;133;103;78m▄\033[48;2;54;46;38m\033[38;2;50;41;34m▄\033[48;2;3;4;3m\033[38;2;3;3;3m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;3;3;3m\033[38;2;3;3;2m▄\033[48;2;53;47;39m\033[38;2;51;44;36m▄\033[48;2;137;105;78m\033[38;2;135;101;72m▄\033[48;2;146;112;85m\033[38;2;163;123;89m▄\033[48;2;155;126;99m\033[38;2;163;127;96m▄\033[48;2;157;132;108m\033[38;2;149;120;96m▄\033[48;2;138;115;97m\033[38;2;136;112;94m▄\033[48;2;102;85;73m\033[38;2;107;88;75m▄\033[48;2;97;81;70m\033[38;2;110;90;76m▄\033[48;2;115;93;76m\033[38;2;165;133;105m▄\033[48;2;165;127;95m\033[38;2;190;146;108m▄\033[48;2;192;146;106m\033[38;2;193;146;106m▄\033[48;2;199;151;111m\033[38;2;193;146;106m▄\033[48;2;200;152;112m\033[38;2;183;139;103m▄\033[48;2;191;146;109m\033[38;2;167;129;99m▄\033[48;2;189;144;104m\033[38;2;178;140;102m▄\033[48;2;199;154;112m\033[38;2;189;141;99m▄\033[48;2;204;157;116m\033[38;2;195;147;105m▄\033[48;2;190;141;99m\033[38;2;181;134;93m▄\033[48;2;190;142;100m\033[38;2;187;139;98m▄\033[48;2;190;141;101m\033[38;2;191;143;102m▄\033[48;2;178;130;91m\033[38;2;179;131;91m▄\033[48;2;168;117;82m\033[38;2;180;129;89m▄\033[48;2;189;136;95m\033[38;2;199;147;103m▄\033[48;2;199;146;102m\033[38;2;203;152;107m▄\033[48;2;200;149;103m\033[38;2;202;150;106m▄\033[48;2;199;147;102m\033[38;2;199;146;102m▄\033[48;2;199;149;102m\033[38;2;208;157;112m▄\033[48;2;198;153;110m\033[38;2;204;160;119m▄\033[48;2;184;140;101m\033[38;2;178;138;101m▄\033[48;2;168;125;90m\033[38;2;160;124;93m▄\033[48;2;160;119;87m\033[38;2;144;111;85m▄\033[48;2;153;115;85m\033[38;2;141;107;80m▄\033[48;2;152;114;82m\033[38;2;141;104;76m▄\033[48;2;150;109;78m\033[38;2;140;99;70m▄\033[48;2;143;101;69m\033[38;2;137;97;67m▄\033[48;2;138;97;66m\033[38;2;137;97;67m▄\033[48;2;138;98;68m\033[38;2;140;98;68m▄\033[48;2;137;97;68m\033[38;2;139;98;68m▄\033[48;2;133;95;67m\033[38;2;138;98;68m▄\033[48;2;129;93;66m\033[38;2;131;95;67m▄\033[48;2;126;93;67m\033[38;2;126;91;63m▄\033[48;2;126;97;72m\033[38;2;125;91;65m▄\033[48;2;109;85;66m\033[38;2;127;98;73m▄\033[48;2;82;65;54m\033[38;2;114;91;71m▄\033[48;2;80;66;56m\033[38;2;96;79;64m▄\033[48;2;84;72;62m\033[38;2;79;67;57m▄\033[48;2;66;54;46m\033[38;2;65;54;46m▄\033[48;2;112;93;78m\033[38;2;114;95;78m▄\033[48;2;160;128;100m\033[38;2;159;127;98m▄\033[48;2;170;131;98m\033[38;2;170;128;94m▄\033[48;2;170;130;96m\033[38;2;166;122;87m▄\033[48;2;159;117;84m\033[38;2;140;101;71m▄\033[48;2;116;87;64m\033[38;2;82;59;45m▄\033[48;2;40;32;26m\033[38;2;19;16;14m▄\033[48;2;2;2;2m\033[38;2;0;0;0m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;1;1;1m \033[48;2;42;35;30m\033[38;2;25;22;20m▄\033[48;2;132;99;69m\033[38;2;116;87;63m▄\033[48;2;170;127;89m\033[38;2;171;127;89m▄\033[48;2;166;126;90m\033[38;2;162;122;85m▄\033[48;2;149;116;88m\033[38;2;144;111;82m▄\033[48;2;127;102;82m\033[38;2;121;94;73m▄\033[48;2;118;95;77m\033[38;2;129;101;80m▄\033[48;2;150;120;96m\033[38;2;180;142;110m▄\033[48;2;192;151;115m\033[38;2;196;149;111m▄\033[48;2;195;148;109m\033[38;2;187;138;101m▄\033[48;2;193;145;106m\033[38;2;175;129;94m▄\033[48;2;171;128;94m\033[38;2;159;119;89m▄\033[48;2;140;106;81m\033[38;2;119;91;73m▄\033[48;2;146;118;93m\033[38;2;126;103;85m▄\033[48;2;160;125;94m\033[38;2;144;110;83m▄\033[48;2;183;133;92m\033[38;2;191;142;101m▄\033[48;2;192;142;100m\033[38;2;191;140;98m▄\033[48;2;178;131;91m\033[38;2;175;125;87m▄\033[48;2;188;140;100m\033[38;2;183;136;96m▄\033[48;2;196;150;109m\033[38;2;189;144;104m▄\033[48;2;183;135;95m\033[38;2;182;134;94m▄\033[48;2;183;134;93m\033[38;2;193;143;104m▄\033[48;2;206;155;110m\033[38;2;201;152;109m▄\033[48;2;204;153;109m\033[38;2;200;151;108m▄\033[48;2;201;149;105m\033[38;2;195;142;101m▄\033[48;2;200;147;103m\033[38;2;199;146;102m▄\033[48;2;210;157;112m\033[38;2;197;144;100m▄\033[48;2;210;166;126m\033[38;2;198;153;113m▄\033[48;2;168;130;95m\033[38;2;156;120;88m▄\033[48;2;156;127;99m\033[38;2;120;97;77m▄\033[48;2;135;108;85m\033[38;2;100;82;67m▄\033[48;2;133;103;79m\033[38;2;94;74;59m▄\033[48;2;132;100;77m\033[38;2;118;91;72m▄\033[48;2;133;95;68m\033[38;2;138;107;80m▄\033[48;2;130;91;63m\033[38;2;141;105;78m▄\033[48;2;130;91;65m\033[38;2;128;92;65m▄\033[48;2;135;95;67m\033[38;2;123;86;60m▄\033[48;2;139;97;68m\033[38;2;130;91;64m▄\033[48;2;139;97;68m\033[38;2;142;101;73m▄\033[48;2;131;91;63m\033[38;2;137;96;67m▄\033[48;2;129;93;65m\033[38;2;129;93;64m▄\033[48;2;122;86;61m\033[38;2;123;88;61m▄\033[48;2;121;90;64m\033[38;2;119;85;61m▄\033[48;2;120;93;70m\033[38;2;117;88;65m▄\033[48;2;101;81;63m\033[38;2;116;91;70m▄\033[48;2;75;61;52m\033[38;2;83;66;53m▄\033[48;2;72;59;50m\033[38;2;73;60;50m▄\033[48;2;94;78;63m\033[38;2;77;62;51m▄\033[48;2;130;101;77m\033[38;2;98;73;56m▄\033[48;2;152;112;81m\033[38;2;122;86;61m▄\033[48;2;142;101;71m\033[38;2;120;86;62m▄\033[48;2;103;78;62m\033[38;2;70;61;55m▄\033[48;2;38;32;28m\033[38;2;10;9;9m▄\033[48;2;4;4;4m\033[38;2;0;0;0m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;9;9;8m\033[38;2;2;2;2m▄\033[48;2;82;66;54m\033[38;2;43;41;38m▄\033[48;2;157;115;79m\033[38;2;140;109;83m▄\033[48;2;158;119;84m\033[38;2;133;96;66m▄\033[48;2;129;97;72m\033[38;2;114;85;63m▄\033[48;2;104;77;60m\033[38;2;111;83;62m▄\033[48;2;143;109;84m\033[38;2;158;118;87m▄\033[48;2;188;144;107m\033[38;2;189;141;104m▄\033[48;2;188;140;101m\033[38;2;166;120;86m▄\033[48;2;171;122;89m\033[38;2;151;116;94m▄\033[48;2;172;131;100m\033[38;2;154;131;110m▄\033[48;2;165;132;105m\033[38;2;139;121;104m▄\033[48;2;150;124;102m\033[38;2;122;107;94m▄\033[48;2;145;121;100m\033[38;2;147;127;109m▄\033[48;2;156;122;95m\033[38;2;156;124;97m▄\033[48;2;190;143;104m\033[38;2;177;134;100m▄\033[48;2;183;133;92m\033[38;2;179;129;92m▄\033[48;2;177;127;88m\033[38;2;168;117;80m▄\033[48;2;180;134;95m\033[38;2;170;122;85m▄\033[48;2;182;137;99m\033[38;2;176;130;93m▄\033[48;2;181;135;96m\033[38;2;175;129;93m▄\033[48;2;184;135;94m\033[38;2;176;127;88m▄\033[48;2;183;134;94m\033[38;2;177;128;90m▄\033[48;2;186;137;98m\033[38;2;167;118;82m▄\033[48;2;185;133;94m\033[38;2;173;123;87m▄\033[48;2;195;142;99m\033[38;2;181;132;93m▄\033[48;2;185;136;95m\033[38;2;179;134;98m▄\033[48;2;180;137;99m\033[38;2;171;134;99m▄\033[48;2;142;107;78m\033[38;2;140;109;82m▄\033[48;2;91;71;56m\033[38;2;83;66;51m▄\033[48;2;65;52;43m\033[38;2;60;50;42m▄\033[48;2;74;58;48m\033[38;2;65;56;49m▄\033[48;2;109;84;68m\033[38;2;67;58;51m▄\033[48;2;121;94;72m\033[38;2;67;57;50m▄\033[48;2;109;81;63m\033[38;2;76;63;52m▄\033[48;2;119;88;63m\033[38;2;88;67;51m▄\033[48;2;117;83;58m\033[38;2;96;68;50m▄\033[48;2;118;82;58m\033[38;2;105;72;53m▄\033[48;2;132;95;69m\033[38;2;119;83;59m▄\033[48;2;147;106;77m\033[38;2;129;89;63m▄\033[48;2;138;98;70m\033[38;2;133;94;68m▄\033[48;2;128;91;64m\033[38;2;126;90;64m▄\033[48;2;119;85;59m\033[38;2;121;86;61m▄\033[48;2;108;77;55m\033[38;2;107;75;54m▄\033[48;2;126;97;76m\033[38;2;102;73;53m▄\033[48;2;115;91;71m\033[38;2;117;89;68m▄\033[48;2;70;56;46m\033[38;2;85;65;51m▄\033[48;2;69;54;44m\033[38;2;66;52;42m▄\033[48;2;85;63;49m\033[38;2;72;55;44m▄\033[48;2;104;71;51m\033[38;2;86;61;46m▄\033[48;2;97;70;50m\033[38;2;85;67;55m▄\033[48;2;34;30;26m\033[38;2;23;21;20m▄\033[48;2;1;1;1m\033[38;2;0;0;0m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;18;18;17m\033[38;2;8;8;8m▄\033[48;2;115;95;79m\033[38;2;100;92;83m▄\033[48;2;119;84;60m\033[38;2;134;103;79m▄\033[48;2;107;79;58m\033[38;2;123;90;66m▄\033[48;2;130;97;71m\033[38;2;154;112;80m▄\033[48;2;175;130;95m\033[38;2;184;136;98m▄\033[48;2;186;137;99m\033[38;2;171;127;91m▄\033[48;2;128;96;74m\033[38;2;68;56;50m▄\033[48;2;75;66;58m\033[38;2;42;33;31m▄\033[48;2;44;37;34m\033[38;2;101;79;69m▄\033[48;2;54;46;44m\033[38;2;86;80;80m▄\033[48;2;67;61;58m\033[38;2;52;48;47m▄\033[48;2;116;103;92m\033[38;2;78;70;63m▄\033[48;2;141;114;92m\033[38;2;107;88;72m▄\033[48;2;160;122;94m\033[38;2;139;105;79m▄\033[48;2;166;117;83m\033[38;2;158;113;80m▄\033[48;2;165;117;81m\033[38;2;161;116;83m▄\033[48;2;163;117;82m\033[38;2;154;112;81m▄\033[48;2;161;116;81m\033[38;2;148;107;75m▄\033[48;2;159;114;80m\033[38;2;150;107;74m▄\033[48;2;164;118;83m\033[38;2;160;117;85m▄\033[48;2;158;113;81m\033[38;2;138;97;69m▄\033[48;2;151;108;77m\033[38;2;132;94;67m▄\033[48;2;152;109;78m\033[38;2;127;90;65m▄\033[48;2;147;106;75m\033[38;2;113;82;59m▄\033[48;2;152;116;86m\033[38;2;111;84;64m▄\033[48;2;136;107;82m\033[38;2;87;68;53m▄\033[48;2;115;92;72m\033[38;2;74;61;51m▄\033[48;2;79;67;54m\033[38;2;48;43;38m▄\033[48;2;40;36;33m\033[38;2;24;24;23m▄\033[48;2;39;39;41m\033[38;2;60;58;61m▄\033[48;2;27;27;30m\033[38;2;37;38;42m▄\033[48;2;26;24;25m\033[38;2;38;34;33m▄\033[48;2;33;28;26m\033[38;2;66;48;40m▄\033[48;2;40;35;30m\033[38;2;33;29;27m▄\033[48;2;69;53;43m\033[38;2;40;35;33m▄\033[48;2;90;63;47m\033[38;2;72;53;43m▄\033[48;2;112;79;56m\033[38;2;105;75;55m▄\033[48;2;121;86;61m\033[38;2;117;84;59m▄\033[48;2;129;92;65m\033[38;2;120;84;59m▄\033[48;2;126;90;64m\033[38;2;119;84;59m▄\033[48;2;120;86;61m\033[38;2;117;84;59m▄\033[48;2;112;79;57m\033[38;2;113;81;59m▄\033[48;2;98;69;48m\033[38;2;101;71;50m▄\033[48;2;99;70;51m\033[38;2;90;61;44m▄\033[48;2;91;68;51m\033[38;2;92;66;49m▄\033[48;2;67;51;40m\033[38;2;78;58;45m▄\033[48;2;65;50;40m\033[38;2;65;49;38m▄\033[48;2;79;58;45m\033[38;2;71;52;40m▄\033[48;2;85;71;62m\033[38;2;79;65;55m▄\033[48;2;19;18;17m\033[38;2;21;20;18m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;6;6;5m\033[38;2;8;7;7m▄\033[48;2;89;84;79m\033[38;2;90;85;78m▄\033[48;2;132;105;84m\033[38;2;114;83;64m▄\033[48;2;127;91;66m\033[38;2;129;90;64m▄\033[48;2;164;117;83m\033[38;2;171;122;85m▄\033[48;2;187;137;98m\033[38;2;184;134;94m▄\033[48;2;165;123;89m\033[38;2;161;123;94m▄\033[48;2;77;66;60m\033[38;2;105;91;82m▄\033[48;2;79;62;55m\033[38;2;63;50;46m▄\033[48;2;130;97;77m\033[38;2;118;79;57m▄\033[48;2;73;69;70m\033[38;2;101;71;54m▄\033[48;2;59;49;44m\033[38;2;90;66;53m▄\033[48;2;61;52;48m\033[38;2;69;56;49m▄\033[48;2;83;67;57m\033[38;2;87;67;55m▄\033[48;2;122;90;67m\033[38;2;152;112;83m▄\033[48;2;155;113;84m\033[38;2;152;114;86m▄\033[48;2;150;112;83m\033[38;2;135;102;77m▄\033[48;2;142;105;77m\033[38;2;129;96;72m▄\033[48;2;138;99;71m\033[38;2;133;98;73m▄\033[48;2;144;105;75m\033[38;2;130;97;70m▄\033[48;2;139;102;75m\033[38;2;116;84;62m▄\033[48;2;127;91;67m\033[38;2;108;78;58m▄\033[48;2;115;80;58m\033[38;2;111;80;59m▄\033[48;2;112;78;56m\033[38;2;111;78;57m▄\033[48;2;83;60;46m\033[38;2;92;65;48m▄\033[48;2;72;55;43m\033[38;2;56;42;34m▄\033[48;2;57;45;37m\033[38;2;48;38;33m▄\033[48;2;40;34;32m\033[38;2;37;31;29m▄\033[48;2;27;25;25m\033[38;2;27;26;25m▄\033[48;2;21;21;21m\033[38;2;21;21;22m▄\033[48;2;45;36;35m\033[38;2;27;24;25m▄\033[48;2;44;35;34m\033[38;2;40;31;28m▄\033[48;2;53;40;33m\033[38;2;47;35;30m▄\033[48;2;85;57;45m\033[38;2;45;36;34m▄\033[48;2;34;30;29m\033[38;2;48;43;41m▄\033[48;2;46;42;40m\033[38;2;75;64;54m▄\033[48;2;60;48;41m\033[38;2;80;63;49m▄\033[48;2;89;66;50m\033[38;2;68;50;40m▄\033[48;2;107;77;56m\033[38;2;77;56;42m▄\033[48;2;113;80;58m\033[38;2;99;69;52m▄\033[48;2;112;79;58m\033[38;2;104;73;54m▄\033[48;2;113;80;57m\033[38;2;105;73;53m▄\033[48;2;112;80;58m\033[38;2;108;76;55m▄\033[48;2;103;72;52m\033[38;2;104;72;52m▄\033[48;2;95;66;48m\033[38;2;97;66;48m▄\033[48;2;83;57;41m\033[38;2;89;60;45m▄\033[48;2;84;62;46m\033[38;2;78;54;41m▄\033[48;2;69;50;37m\033[38;2;77;55;41m▄\033[48;2;72;52;40m\033[38;2;80;56;41m▄\033[48;2;77;60;49m\033[38;2;81;61;49m▄\033[48;2;25;22;20m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;11;11;10m\033[38;2;10;9;9m▄\033[48;2;91;80;71m\033[38;2;86;72;62m▄\033[48;2;107;70;51m\033[38;2;126;86;62m▄\033[48;2;139;94;66m\033[38;2;141;95;67m▄\033[48;2;174;124;85m\033[38;2;170;118;82m▄\033[48;2;183;133;92m\033[38;2;175;124;85m▄\033[48;2;144;105;78m\033[38;2;145;101;73m▄\033[48;2;139;118;101m\033[38;2;179;142;112m▄\033[48;2;52;50;49m\033[38;2;152;127;106m▄\033[48;2;49;43;41m\033[38;2;108;90;76m▄\033[48;2;55;45;41m\033[38;2;97;81;70m▄\033[48;2;61;51;47m\033[38;2;126;100;80m▄\033[48;2;104;79;64m\033[38;2;141;108;83m▄\033[48;2;152;113;87m\033[38;2;138;105;83m▄\033[48;2;149;112;85m\033[38;2;132;101;78m▄\033[48;2;133;99;77m\033[38;2;131;100;77m▄\033[48;2;128;97;74m\033[38;2;123;93;70m▄\033[48;2;126;96;73m\033[38;2;114;84;62m▄\033[48;2;120;90;67m\033[38;2;108;81;62m▄\033[48;2;112;83;62m\033[38;2;91;69;54m▄\033[48;2;102;75;56m\033[38;2;91;69;55m▄\033[48;2;98;71;54m\033[38;2;94;70;55m▄\033[48;2;100;73;54m\033[38;2;95;69;53m▄\033[48;2;105;76;55m\033[38;2;97;70;51m▄\033[48;2;107;78;57m\033[38;2;96;69;50m▄\033[48;2;76;56;44m\033[38;2;98;71;52m▄\033[48;2;54;42;35m\033[38;2;79;58;44m▄\033[48;2;53;41;33m\033[38;2;68;51;40m▄\033[48;2;52;42;36m\033[38;2;75;57;46m▄\033[48;2;46;39;37m\033[38;2;81;65;54m▄\033[48;2;45;41;39m\033[38;2;92;76;63m▄\033[48;2;53;48;44m\033[38;2;102;85;71m▄\033[48;2;60;52;49m\033[38;2;138;116;97m▄\033[48;2;82;72;64m\033[38;2;136;111;90m▄\033[48;2;94;76;61m\033[38;2;135;108;84m▄\033[48;2;103;82;62m\033[38;2;114;87;64m▄\033[48;2;101;76;57m\033[38;2;100;74;55m▄\033[48;2;80;57;42m\033[38;2;91;65;50m▄\033[48;2;62;44;34m\033[38;2;88;61;45m▄\033[48;2;83;58;45m\033[38;2;87;60;44m▄\033[48;2;97;68;51m\033[38;2;94;64;47m▄\033[48;2;105;74;55m\033[38;2;102;71;52m▄\033[48;2;107;75;54m\033[38;2;106;74;54m▄\033[48;2;102;70;50m\033[38;2;103;71;51m▄\033[48;2;99;68;50m\033[38;2;92;62;44m▄\033[48;2;86;60;43m\033[38;2;84;58;41m▄\033[48;2;73;50;38m\033[38;2;72;49;36m▄\033[48;2;86;62;47m\033[38;2;82;58;44m▄\033[48;2;97;70;51m\033[38;2;97;71;54m▄\033[48;2;91;74;60m\033[38;2;83;68;58m▄\033[48;2;24;22;21m\033[38;2;18;16;15m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;3;2;2m\033[38;2;0;0;0m▄\033[48;2;50;45;42m\033[38;2;9;9;9m▄\033[48;2;131;103;82m\033[38;2;80;69;61m▄\033[48;2;152;108;78m\033[38;2;141;107;84m▄\033[48;2;166;115;79m\033[38;2;158;109;75m▄\033[48;2;171;118;80m\033[38;2;176;123;84m▄\033[48;2;166;116;82m\033[38;2;188;137;97m▄\033[48;2;180;138;105m\033[38;2;178;131;96m▄\033[48;2;169;135;106m\033[38;2;181;146;116m▄\033[48;2;146;117;94m\033[38;2;176;142;114m▄\033[48;2;145;116;92m\033[38;2;154;123;100m▄\033[48;2;137;105;82m\033[38;2;121;96;78m▄\033[48;2;123;96;75m\033[38;2;111;88;71m▄\033[48;2;120;92;72m\033[38;2;110;85;65m▄\033[48;2;124;97;74m\033[38;2;111;86;66m▄\033[48;2;116;89;68m\033[38;2;105;82;64m▄\033[48;2;107;80;61m\033[38;2;94;72;57m▄\033[48;2;100;75;59m\033[38;2;86;68;55m▄\033[48;2;93;71;57m\033[38;2;79;63;52m▄\033[48;2;90;71;56m\033[38;2;80;63;52m▄\033[48;2;85;67;53m\033[38;2;79;64;52m▄\033[48;2;86;66;54m\033[38;2;79;62;51m▄\033[48;2;92;69;54m\033[38;2;84;64;51m▄\033[48;2;93;68;50m\033[38;2;87;64;49m▄\033[48;2;87;63;47m\033[38;2;83;60;46m▄\033[48;2;87;62;46m\033[38;2;79;56;42m▄\033[48;2;90;65;48m\033[38;2;83;59;45m▄\033[48;2;80;58;43m\033[38;2;91;66;50m▄\033[48;2;89;66;49m\033[38;2;93;67;50m▄\033[48;2;100;78;61m\033[38;2;101;76;58m▄\033[48;2;117;95;77m\033[38;2;120;95;76m▄\033[48;2;114;92;76m\033[38;2;109;86;67m▄\033[48;2;126;102;82m\033[38;2;100;76;60m▄\033[48;2;118;94;75m\033[38;2;99;76;59m▄\033[48;2;113;87;66m\033[38;2;93;69;51m▄\033[48;2;101;74;55m\033[38;2;91;65;48m▄\033[48;2;92;65;49m\033[38;2;87;61;45m▄\033[48;2;87;60;46m\033[38;2;90;63;46m▄\033[48;2;98;68;49m\033[38;2;113;80;58m▄\033[48;2;103;73;52m\033[38;2;114;81;56m▄\033[48;2;102;71;50m\033[38;2;108;75;53m▄\033[48;2;101;70;50m\033[38;2;106;74;53m▄\033[48;2;109;76;57m\033[38;2;108;76;56m▄\033[48;2;105;73;53m\033[38;2;108;76;55m▄\033[48;2;95;64;45m\033[38;2;101;70;51m▄\033[48;2;84;57;41m\033[38;2;87;58;43m▄\033[48;2;73;49;37m\033[38;2;78;53;39m▄\033[48;2;78;54;40m\033[38;2;85;60;45m▄\033[48;2;98;75;60m\033[38;2;100;77;61m▄\033[48;2;74;66;60m\033[38;2;67;59;53m▄\033[48;2;13;13;12m\033[38;2;11;10;10m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "";
                    text r_grut02 = ""\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;26;23;22m\033[38;2;4;4;4m▄\033[48;2;94;77;65m\033[38;2;48;42;38m▄\033[48;2;153;111;81m\033[38;2;140;108;84m▄\033[48;2;188;138;97m\033[38;2;192;144;104m▄\033[48;2;202;151;110m\033[38;2;205;156;115m▄\033[48;2;175;123;86m\033[38;2;188;138;99m▄\033[48;2;179;138;106m\033[38;2;165;127;101m▄\033[48;2;163;131;106m\033[38;2;136;125;117m▄\033[48;2;127;107;91m\033[38;2;96;90;86m▄\033[48;2;109;93;81m\033[38;2;81;75;71m▄\033[48;2;104;87;75m\033[38;2;76;71;67m▄\033[48;2;104;87;73m\033[38;2;75;70;67m▄\033[48;2;99;83;69m\033[38;2;84;81;77m▄\033[48;2;87;71;60m\033[38;2;85;80;77m▄\033[48;2;78;63;52m\033[38;2;82;75;71m▄\033[48;2;77;62;52m\033[38;2;69;59;53m▄\033[48;2;75;61;51m\033[38;2;68;58;50m▄\033[48;2;71;57;48m\033[38;2;62;50;44m▄\033[48;2;74;60;50m\033[38;2;65;53;44m▄\033[48;2;78;63;52m\033[38;2;73;59;49m▄\033[48;2;78;61;49m\033[38;2;76;60;48m▄\033[48;2;80;60;46m\033[38;2;75;57;45m▄\033[48;2;78;57;43m\033[38;2;77;56;43m▄\033[48;2;75;55;41m\033[38;2;70;51;37m▄\033[48;2;76;54;42m\033[38;2;72;52;41m▄\033[48;2;87;63;48m\033[38;2;75;55;41m▄\033[48;2;95;69;52m\033[38;2;89;65;49m▄\033[48;2;96;71;53m\033[38;2;97;71;52m▄\033[48;2;103;78;60m\033[38;2;98;71;52m▄\033[48;2;100;76;57m\033[38;2;95;69;50m▄\033[48;2;90;67;51m\033[38;2;86;63;49m▄\033[48;2;89;66;50m\033[38;2;86;63;47m▄\033[48;2;88;64;47m\033[38;2;87;63;46m▄\033[48;2;88;63;47m\033[38;2;92;66;48m▄\033[48;2;90;63;47m\033[38;2;99;70;51m▄\033[48;2;100;72;51m\033[38;2;108;78;55m▄\033[48;2;121;87;62m\033[38;2;123;89;63m▄\033[48;2;123;87;61m\033[38;2;127;92;65m▄\033[48;2;117;83;57m\033[38;2;120;86;60m▄\033[48;2;111;77;55m\033[38;2;115;81;58m▄\033[48;2;113;80;59m\033[38;2;115;83;62m▄\033[48;2;113;81;59m\033[38;2;117;85;63m▄\033[48;2;108;77;55m\033[38;2;118;86;63m▄\033[48;2;94;66;49m\033[38;2;107;77;57m▄\033[48;2;86;59;44m\033[38;2;94;65;48m▄\033[48;2;94;68;51m\033[38;2;104;75;56m▄\033[48;2;103;81;67m\033[38;2;106;84;70m▄\033[48;2;75;69;64m\033[38;2;83;75;70m▄\033[48;2;14;13;13m\033[38;2;22;20;18m▄\033[48;2;0;0;0m\033[38;2;1;1;1m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;1;1;1m \033[48;2;29;25;23m\033[38;2;28;24;21m▄\033[48;2;123;96;74m\033[38;2;125;97;74m▄\033[48;2;194;148;109m\033[38;2;196;152;113m▄\033[48;2;213;167;125m\033[38;2;208;167;127m▄\033[48;2;180;143;112m\033[38;2;104;93;86m▄\033[48;2;93;87;80m\033[38;2;53;54;55m▄\033[48;2;59;59;58m\033[38;2;48;49;49m▄\033[48;2;52;52;51m\033[38;2;46;46;48m▄\033[48;2;49;49;49m\033[38;2;45;45;47m▄\033[48;2;47;46;48m\033[38;2;41;41;43m▄\033[48;2;46;46;46m\033[38;2;37;37;39m▄\033[48;2;47;47;47m\033[38;2;35;36;37m▄\033[48;2;51;50;50m\033[38;2;36;36;36m▄\033[48;2;65;64;62m\033[38;2;43;43;43m▄\033[48;2;77;75;72m\033[38;2;53;53;52m▄\033[48;2;63;59;55m\033[38;2;54;53;52m▄\033[48;2;60;52;46m\033[38;2;52;46;42m▄\033[48;2;58;47;40m\033[38;2;50;41;35m▄\033[48;2;64;51;43m\033[38;2;54;44;37m▄\033[48;2;71;56;45m\033[38;2;68;54;45m▄\033[48;2;74;57;45m\033[38;2;72;56;45m▄\033[48;2;75;57;43m\033[38;2;72;55;44m▄\033[48;2;68;48;36m\033[38;2;70;52;40m▄\033[48;2;69;51;39m \033[48;2;72;53;41m\033[38;2;67;49;38m▄\033[48;2;84;62;47m\033[38;2;76;56;43m▄\033[48;2;91;64;47m\033[38;2;88;63;47m▄\033[48;2;96;69;49m\033[38;2;95;67;47m▄\033[48;2;92;66;48m\033[38;2;93;65;47m▄\033[48;2;84;59;44m\033[38;2;90;63;46m▄\033[48;2;87;61;46m\033[38;2;88;61;46m▄\033[48;2;87;60;44m\033[38;2;89;62;45m▄\033[48;2;93;65;46m\033[38;2;97;69;50m▄\033[48;2;100;70;50m\033[38;2;107;76;55m▄\033[48;2;113;81;58m\033[38;2;116;84;60m▄\033[48;2;128;93;67m\033[38;2;132;98;70m▄\033[48;2;135;100;72m\033[38;2;138;102;74m▄\033[48;2;127;91;65m\033[38;2;130;95;68m▄\033[48;2;116;82;58m\033[38;2;119;86;62m▄\033[48;2;109;77;56m\033[38;2;111;79;58m▄\033[48;2;114;84;61m\033[38;2;112;81;60m▄\033[48;2;118;86;63m\033[38;2;113;81;58m▄\033[48;2;119;87;66m\033[38;2;120;88;67m▄\033[48;2;109;78;58m\033[38;2;123;88;67m▄\033[48;2;106;76;57m\033[38;2;116;83;62m▄\033[48;2;101;74;59m\033[38;2;101;71;53m▄\033[48;2;86;72;63m\033[38;2;91;71;60m▄\033[48;2;41;38;36m\033[38;2;58;51;48m▄\033[48;2;4;3;3m\033[38;2;10;10;9m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;2;2;2m\033[38;2;6;5;5m▄\033[48;2;38;33;27m\033[38;2;60;50;42m▄\033[48;2;138;107;80m\033[38;2;157;121;90m▄\033[48;2;198;153;113m\033[38;2;185;144;107m▄\033[48;2;161;133;107m\033[38;2;92;81;72m▄\033[48;2;56;55;56m\033[38;2;55;55;57m▄\033[48;2;54;55;57m\033[38;2;67;67;69m▄\033[48;2;54;55;57m\033[38;2;60;61;64m▄\033[48;2;53;54;57m\033[38;2;54;54;58m▄\033[48;2;60;61;64m\033[38;2;66;66;71m▄\033[48;2;58;58;61m\033[38;2;59;60;64m▄\033[48;2;37;37;40m\033[38;2;38;40;42m▄\033[48;2;32;32;34m\033[38;2;32;32;35m▄\033[48;2;32;32;32m\033[38;2;31;32;33m▄\033[48;2;34;35;35m\033[38;2;31;32;31m▄\033[48;2;43;43;43m\033[38;2;37;37;37m▄\033[48;2;45;45;45m\033[38;2;48;47;48m▄\033[48;2;43;41;39m\033[38;2;42;41;40m▄\033[48;2;45;39;35m\033[38;2;39;35;33m▄\033[48;2;53;43;37m\033[38;2;48;40;34m▄\033[48;2;63;50;41m\033[38;2;56;46;38m▄\033[48;2;69;55;44m\033[38;2;64;52;43m▄\033[48;2;71;57;46m\033[38;2;70;57;47m▄\033[48;2;72;56;43m\033[38;2;72;56;45m▄\033[48;2;70;52;42m\033[38;2;67;50;40m▄\033[48;2;67;49;39m\033[38;2;67;50;40m▄\033[48;2;69;50;39m\033[38;2;69;50;40m▄\033[48;2;80;57;43m\033[38;2;73;53;40m▄\033[48;2;91;64;47m\033[38;2;87;63;48m▄\033[48;2;98;70;52m\033[38;2;91;65;48m▄\033[48;2;93;65;46m\033[38;2;94;67;49m▄\033[48;2;91;63;45m\033[38;2;96;67;48m▄\033[48;2;96;67;49m\033[38;2;98;67;48m▄\033[48;2;100;71;51m\033[38;2;100;70;50m▄\033[48;2;107;76;55m\033[38;2;109;79;56m▄\033[48;2;123;91;66m\033[38;2;125;92;68m▄\033[48;2;132;98;70m\033[38;2;125;92;67m▄\033[48;2;134;99;72m\033[38;2;127;93;67m▄\033[48;2;126;91;65m\033[38;2;121;88;62m▄\033[48;2;118;85;60m\033[38;2;115;83;59m▄\033[48;2;109;77;57m\033[38;2;108;78;55m▄\033[48;2;117;86;64m\033[38;2;116;87;65m▄\033[48;2;122;90;67m\033[38;2;126;94;70m▄\033[48;2;123;88;66m\033[38;2;126;90;67m▄\033[48;2;126;91;68m\033[38;2;126;91;66m▄\033[48;2;125;91;68m\033[38;2;126;92;65m▄\033[48;2;114;80;60m\033[38;2;120;85;62m▄\033[48;2;102;76;60m\033[38;2;107;76;56m▄\033[48;2;79;68;60m\033[38;2;95;75;62m▄\033[48;2;33;30;28m\033[38;2;63;54;48m▄\033[48;2;4;4;3m\033[38;2;18;17;15m▄\033[48;2;0;0;0m\033[38;2;1;1;1m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;16;15;14m\033[38;2;35;31;27m▄\033[48;2;95;78;64m\033[38;2;133;107;85m▄\033[48;2;174;133;100m\033[38;2;190;148;111m▄\033[48;2;142;114;90m\033[38;2;125;102;84m▄\033[48;2;64;61;58m\033[38;2;42;38;37m▄\033[48;2;46;45;47m\033[38;2;22;21;22m▄\033[48;2;55;56;57m\033[38;2;24;24;26m▄\033[48;2;52;53;56m\033[38;2;37;37;40m▄\033[48;2;45;46;50m\033[38;2;39;39;42m▄\033[48;2;50;51;55m\033[38;2;42;42;45m▄\033[48;2;44;44;48m\033[38;2;29;29;32m▄\033[48;2;27;28;30m\033[38;2;16;17;18m▄\033[48;2;22;23;25m\033[38;2;12;13;15m▄\033[48;2;22;23;24m\033[38;2;14;15;17m▄\033[48;2;24;24;25m\033[38;2;22;22;23m▄\033[48;2;27;27;27m\033[38;2;25;24;24m▄\033[48;2;43;44;44m\033[38;2;35;36;36m▄\033[48;2;34;35;34m\033[38;2;34;34;34m▄\033[48;2;40;37;36m\033[38;2;45;44;42m▄\033[48;2;41;36;32m\033[38;2;42;36;33m▄\033[48;2;51;43;37m\033[38;2;50;42;37m▄\033[48;2;59;48;41m\033[38;2;56;47;41m▄\033[48;2;69;56;47m\033[38;2;62;51;43m▄\033[48;2;73;59;48m\033[38;2;67;55;46m▄\033[48;2;67;51;42m\033[38;2;64;50;42m▄\033[48;2;64;48;39m\033[38;2;62;48;39m▄\033[48;2;69;51;41m\033[38;2;66;51;41m▄\033[48;2;71;53;41m\033[38;2;71;55;44m▄\033[48;2;80;59;46m\033[38;2;81;61;48m▄\033[48;2;89;66;49m\033[38;2;94;72;56m▄\033[48;2;95;69;52m\033[38;2;102;77;59m▄\033[48;2;102;74;55m\033[38;2;107;81;61m▄\033[48;2;103;73;52m\033[38;2;110;80;60m▄\033[48;2;101;71;51m\033[38;2;106;76;56m▄\033[48;2;108;77;56m\033[38;2;106;75;56m▄\033[48;2;116;85;63m\033[38;2;111;80;60m▄\033[48;2;118;87;66m\033[38;2;109;78;57m▄\033[48;2;118;86;62m\033[38;2;109;77;57m▄\033[48;2;117;84;60m\033[38;2;110;79;57m▄\033[48;2;114;82;58m\033[38;2;112;80;58m▄\033[48;2;111;80;57m\033[38;2;114;82;59m▄\033[48;2;115;85;64m\033[38;2;118;87;64m▄\033[48;2;127;95;71m\033[38;2;133;101;77m▄\033[48;2;124;90;65m\033[38;2;125;90;65m▄\033[48;2;124;89;63m\033[38;2;125;90;66m▄\033[48;2;122;87;61m\033[38;2;123;88;63m▄\033[48;2;120;84;60m\033[38;2;123;86;62m▄\033[48;2;118;82;59m\033[38;2;129;91;67m▄\033[48;2;108;75;53m\033[38;2;130;93;66m▄\033[48;2;89;69;55m\033[38;2;115;82;61m▄\033[48;2;43;38;34m\033[38;2;73;60;50m▄\033[48;2;9;8;8m\033[38;2;28;25;23m▄\033[48;2;0;0;0m\033[38;2;2;2;2m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;1;1;1m\033[38;2;7;7;7m▄\033[48;2;61;53;46m\033[38;2;98;86;75m▄\033[48;2;165;129;100m\033[38;2;189;152;121m▄\033[48;2;192;151;115m\033[38;2;190;155;125m▄\033[48;2;127;107;92m\033[38;2;145;126;111m▄\033[48;2;46;43;43m\033[38;2;54;50;49m▄\033[48;2;24;24;25m\033[38;2;22;22;23m▄\033[48;2;21;21;23m\033[38;2;16;17;19m▄\033[48;2;27;27;29m\033[38;2;24;25;26m▄\033[48;2;38;38;40m\033[38;2;36;36;37m▄\033[48;2;37;37;39m\033[38;2;32;32;34m▄\033[48;2;26;26;28m \033[48;2;15;15;17m\033[38;2;18;18;20m▄\033[48;2;12;13;15m\033[38;2;13;14;16m▄\033[48;2;14;15;17m\033[38;2;13;14;16m▄\033[48;2;24;24;25m\033[38;2;17;17;19m▄\033[48;2;23;23;23m\033[38;2;20;20;20m▄\033[48;2;28;28;28m\033[38;2;22;22;22m▄\033[48;2;44;45;44m\033[38;2;32;32;32m▄\033[48;2;47;44;42m\033[38;2;52;49;47m▄\033[48;2;42;38;35m\033[38;2;45;41;39m▄\033[48;2;48;41;38m\033[38;2;42;38;36m▄\033[48;2;46;38;35m\033[38;2;43;38;33m▄\033[48;2;53;44;39m\033[38;2;49;42;36m▄\033[48;2;56;46;38m\033[38;2;56;46;40m▄\033[48;2;64;52;44m\033[38;2;54;44;38m▄\033[48;2;62;49;41m\033[38;2;63;50;43m▄\033[48;2;68;54;45m\033[38;2;67;53;45m▄\033[48;2;71;55;45m\033[38;2;72;58;49m▄\033[48;2;75;58;47m\033[38;2;72;58;48m▄\033[48;2;88;69;55m\033[38;2;76;60;49m▄\033[48;2;97;75;57m\033[38;2;79;62;51m▄\033[48;2;105;79;60m\033[38;2;99;78;60m▄\033[48;2;110;83;62m\033[38;2;109;82;63m▄\033[48;2;112;82;62m\033[38;2;113;84;64m▄\033[48;2;106;77;57m\033[38;2;110;82;61m▄\033[48;2;107;76;56m\033[38;2;103;75;54m▄\033[48;2;105;75;55m\033[38;2;98;71;51m▄\033[48;2;103;74;54m\033[38;2;96;69;50m▄\033[48;2;108;78;58m\033[38;2;105;75;55m▄\033[48;2;116;84;62m\033[38;2;119;87;64m▄\033[48;2;120;87;63m\033[38;2;129;95;70m▄\033[48;2;125;92;67m\033[38;2;129;96;71m▄\033[48;2;138;106;80m\033[38;2;129;98;70m▄\033[48;2;130;95;69m\033[38;2;126;91;65m▄\033[48;2;121;85;59m\033[38;2;120;84;57m▄\033[48;2;119;85;59m\033[38;2;122;86;60m▄\033[48;2;122;87;61m\033[38;2;132;96;70m▄\033[48;2;131;94;67m\033[38;2;145;106;79m▄\033[48;2;140;103;75m\033[38;2;150;111;82m▄\033[48;2;133;94;65m\033[38;2;146;105;74m▄\033[48;2;102;76;58m\033[38;2;125;88;62m▄\033[48;2;57;49;42m\033[38;2;89;68;55m▄\033[48;2;12;11;10m\033[38;2;32;27;24m▄\033[48;2;0;0;0m\033[38;2;3;3;3m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;2;1;2m▄\033[48;2;19;19;18m\033[38;2;44;41;39m▄\033[48;2;136;115;99m\033[38;2;165;138;115m▄\033[48;2;198;159;127m\033[38;2;199;159;127m▄\033[48;2;186;155;129m\033[38;2;178;149;126m▄\033[48;2;148;132;121m\033[38;2;145;131;124m▄\033[48;2;82;78;79m\033[38;2;141;136;137m▄\033[48;2;38;38;39m\033[38;2;64;62;63m▄\033[48;2;41;41;44m\033[38;2;61;61;63m▄\033[48;2;39;39;40m\033[38;2;42;42;43m▄\033[48;2;32;32;33m\033[38;2;29;28;29m▄\033[48;2;22;22;24m\033[38;2;20;19;20m▄\033[48;2;24;24;26m\033[38;2;24;23;24m▄\033[48;2;22;22;23m\033[38;2;21;20;21m▄\033[48;2;22;22;24m\033[38;2;20;20;21m▄\033[48;2;21;21;22m\033[38;2;24;24;25m▄\033[48;2;24;24;24m\033[38;2;29;29;29m▄\033[48;2;22;22;22m\033[38;2;26;26;27m▄\033[48;2;29;28;29m\033[38;2;41;40;40m▄\033[48;2;48;47;46m\033[38;2;59;58;56m▄\033[48;2;62;58;55m\033[38;2;64;60;57m▄\033[48;2;50;46;45m\033[38;2;54;50;49m▄\033[48;2;44;40;38m\033[38;2;46;42;40m▄\033[48;2;41;36;33m\033[38;2;41;37;35m▄\033[48;2;45;39;35m\033[38;2;43;38;35m▄\033[48;2;50;41;36m\033[38;2;48;41;36m▄\033[48;2;55;45;38m\033[38;2;52;44;37m▄\033[48;2;58;48;42m \033[48;2;67;57;48m\033[38;2;64;55;47m▄\033[48;2;68;57;48m\033[38;2;66;56;48m▄\033[48;2;72;60;50m\033[38;2;65;56;48m▄\033[48;2;65;53;45m\033[38;2;54;45;41m▄\033[48;2;60;49;41m\033[38;2;50;42;36m▄\033[48;2;67;55;44m\033[38;2;48;39;34m▄\033[48;2;99;77;60m\033[38;2;75;59;48m▄\033[48;2;109;82;63m\033[38;2;102;78;61m▄\033[48;2;107;80;60m\033[38;2;104;77;58m▄\033[48;2;100;73;54m\033[38;2;98;71;52m▄\033[48;2;91;65;48m\033[38;2;90;63;46m▄\033[48;2;96;68;51m\033[38;2;95;68;50m▄\033[48;2;108;79;59m\033[38;2;104;77;57m▄\033[48;2;119;88;65m\033[38;2;116;86;64m▄\033[48;2;127;94;69m\033[38;2;123;91;68m▄\033[48;2;130;96;71m\033[38;2;126;92;67m▄\033[48;2;124;89;64m\033[38;2;122;88;63m▄\033[48;2;128;94;69m\033[38;2;121;87;64m▄\033[48;2;126;92;67m\033[38;2;119;86;62m▄\033[48;2;127;91;66m\033[38;2;121;87;63m▄\033[48;2;135;100;73m\033[38;2;130;94;68m▄\033[48;2;144;106;77m\033[38;2;140;102;73m▄\033[48;2;151;112;81m\033[38;2;153;113;83m▄\033[48;2;152;111;79m\033[38;2;153;112;79m▄\033[48;2;137;95;65m\033[38;2;139;96;65m▄\033[48;2;109;79;59m\033[38;2;115;82;60m▄\033[48;2;54;44;38m\033[38;2;64;52;43m▄\033[48;2;10;9;8m\033[38;2;14;12;10m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;5;5;5m\033[38;2;13;13;12m▄\033[48;2;78;71;65m\033[38;2;116;102;91m▄\033[48;2;176;139;112m\033[38;2;185;142;110m▄\033[48;2;195;154;121m\033[38;2;192;149;117m▄\033[48;2;172;146;127m\033[38;2;177;152;137m▄\033[48;2;147;136;133m\033[38;2;152;143;143m▄\033[48;2;175;171;172m\033[38;2;173;168;169m▄\033[48;2;126;125;126m\033[38;2;174;174;176m▄\033[48;2;46;43;45m\033[38;2;97;91;94m▄\033[48;2;31;30;31m\033[38;2;39;34;36m▄\033[48;2;26;25;25m\033[38;2;29;26;27m▄\033[48;2;22;20;21m\033[38;2;27;25;26m▄\033[48;2;23;21;21m\033[38;2;23;21;22m▄\033[48;2;25;22;23m\033[38;2;26;24;25m▄\033[48;2;21;21;22m\033[38;2;29;26;27m▄\033[48;2;27;27;28m\033[38;2;27;26;27m▄\033[48;2;25;24;25m\033[38;2;30;30;30m▄\033[48;2;29;29;30m\033[38;2;39;39;39m▄\033[48;2;48;47;47m\033[38;2;52;52;51m▄\033[48;2;58;56;55m\033[38;2;61;60;58m▄\033[48;2;67;63;60m\033[38;2;62;59;58m▄\033[48;2;62;59;56m\033[38;2;60;58;55m▄\033[48;2;49;46;44m\033[38;2;54;50;48m▄\033[48;2;43;39;37m\033[38;2;51;46;44m▄\033[48;2;44;38;36m\033[38;2;48;42;40m▄\033[48;2;48;41;37m\033[38;2;42;36;33m▄\033[48;2;49;41;36m\033[38;2;56;49;42m▄\033[48;2;64;52;46m\033[38;2;64;53;46m▄\033[48;2;65;55;50m\033[38;2;60;52;47m▄\033[48;2;59;52;46m\033[38;2;52;45;43m▄\033[48;2;56;49;44m\033[38;2;46;41;39m▄\033[48;2;50;44;40m\033[38;2;40;36;34m▄\033[48;2;38;34;31m\033[38;2;30;29;28m▄\033[48;2;32;28;26m\033[38;2;27;24;24m▄\033[48;2;51;42;35m\033[38;2;39;32;30m▄\033[48;2;87;67;54m\033[38;2;77;60;49m▄\033[48;2;104;78;60m\033[38;2;104;79;60m▄\033[48;2;102;75;56m\033[38;2;101;75;56m▄\033[48;2;92;65;48m\033[38;2;92;65;47m▄\033[48;2;92;65;48m\033[38;2;89;62;45m▄\033[48;2;99;72;53m\033[38;2;91;65;46m▄\033[48;2;112;82;62m\033[38;2;103;74;55m▄\033[48;2;116;85;63m\033[38;2;106;75;55m▄\033[48;2;114;81;58m\033[38;2;103;72;51m▄\033[48;2;111;80;58m\033[38;2;97;69;49m▄\033[48;2;109;76;56m\033[38;2;105;75;56m▄\033[48;2;113;79;58m\033[38;2;108;74;55m▄\033[48;2;117;83;60m\033[38;2;118;83;61m▄\033[48;2;131;95;69m\033[38;2;139;102;75m▄\033[48;2;141;102;74m\033[38;2;145;105;76m▄\033[48;2;151;111;81m\033[38;2;146;103;74m▄\033[48;2;152;109;77m\033[38;2;147;103;72m▄\033[48;2;141;97;66m\033[38;2;138;94;64m▄\033[48;2;117;82;59m\033[38;2;118;81;58m▄\033[48;2;70;56;47m\033[38;2;78;63;53m▄\033[48;2;17;16;14m\033[38;2;25;22;20m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;1;1;1m▄\033[48;2;24;23;22m\033[38;2;38;35;33m▄\033[48;2;143;122;106m\033[38;2;156;125;105m▄\033[48;2;187;141;105m\033[38;2;185;132;96m▄\033[48;2;195;146;111m\033[38;2;203;135;107m▄\033[48;2;201;156;137m\033[38;2;206;99;110m▄\033[48;2;181;157;159m\033[38;2;201;104;129m▄\033[48;2;183;176;178m\033[38;2;186;154;164m▄\033[48;2;178;178;180m\033[38;2;182;175;179m▄\033[48;2;138;133;137m\033[38;2;144;136;140m▄\033[48;2;75;70;71m\033[38;2;81;73;76m▄\033[48;2;42;40;41m\033[38;2;56;49;52m▄\033[48;2;35;34;35m\033[38;2;42;39;40m▄\033[48;2;29;27;28m\033[38;2;40;35;36m▄\033[48;2;30;27;28m\033[38;2;53;44;44m▄\033[48;2;36;34;35m\033[38;2;45;42;43m▄\033[48;2;34;32;33m \033[48;2;36;34;35m\033[38;2;39;37;38m▄\033[48;2;50;48;49m\033[38;2;49;49;49m▄\033[48;2;65;65;63m\033[38;2;58;57;57m▄\033[48;2;71;71;67m\033[38;2;71;70;70m▄\033[48;2;66;65;63m\033[38;2;73;72;68m▄\033[48;2;67;65;62m\033[38;2;62;58;56m▄\033[48;2;60;57;53m\033[38;2;65;60;56m▄\033[48;2;52;48;45m\033[38;2;56;52;49m▄\033[48;2;38;33;32m\033[38;2;46;42;40m▄\033[48;2;39;34;31m\033[38;2;44;39;37m▄\033[48;2;50;43;39m\033[38;2;46;41;37m▄\033[48;2;57;48;43m\033[38;2;47;42;39m▄\033[48;2;56;48;45m\033[38;2;48;43;41m▄\033[48;2;47;42;40m\033[38;2;42;39;37m▄\033[48;2;40;38;36m\033[38;2;37;36;34m▄\033[48;2;33;31;31m\033[38;2;33;32;32m▄\033[48;2;30;28;29m\033[38;2;32;31;31m▄\033[48;2;22;21;22m\033[38;2;21;21;22m▄\033[48;2;32;28;26m\033[38;2;24;22;21m▄\033[48;2;67;55;45m\033[38;2;53;44;37m▄\033[48;2;101;78;61m\033[38;2;91;71;55m▄\033[48;2;102;77;57m\033[38;2;97;74;56m▄\033[48;2;96;71;51m\033[38;2;89;68;51m▄\033[48;2;87;61;44m\033[38;2;75;54;41m▄\033[48;2;85;59;43m\033[38;2;78;55;41m▄\033[48;2;90;63;46m\033[38;2;80;57;41m▄\033[48;2;93;65;48m\033[38;2;81;56;41m▄\033[48;2;89;62;44m\033[38;2;80;54;40m▄\033[48;2;81;54;39m\033[38;2;84;57;42m▄\033[48;2;88;59;42m\033[38;2;97;67;49m▄\033[48;2;107;74;54m\033[38;2;118;83;61m▄\033[48;2;128;93;68m\033[38;2;132;95;70m▄\033[48;2;141;103;75m\033[38;2;139;99;71m▄\033[48;2;141;99;70m\033[38;2;137;94;66m▄\033[48;2;140;96;67m\033[38;2;136;92;64m▄\033[48;2;136;92;63m\033[38;2;129;85;58m▄\033[48;2;128;83;56m\033[38;2;117;74;50m▄\033[48;2;112;74;52m\033[38;2;107;73;52m▄\033[48;2;83;63;52m\033[38;2;83;67;57m▄\033[48;2;34;29;26m\033[38;2;37;33;30m▄\033[48;2;2;2;2m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;2;2;1m\033[38;2;2;2;2m▄\033[48;2;49;44;41m\033[38;2;52;47;43m▄\033[48;2;161;124;100m\033[38;2;160;119;95m▄\033[48;2;192;126;99m\033[38;2;199;105;103m▄\033[48;2;211;100;106m\033[38;2;220;67;105m▄\033[48;2;218;63;105m\033[38;2;222;58;111m▄\033[48;2;214;65;112m\033[38;2;220;60;119m▄\033[48;2;198;91;126m\033[38;2;216;61;119m▄\033[48;2;188;137;153m\033[38;2;204;65;114m▄\033[48;2;159;130;140m\033[38;2;185;78;112m▄\033[48;2;108;85;91m\033[38;2;161;71;98m▄\033[48;2;79;59;65m\033[38;2;136;57;77m▄\033[48;2;65;43;49m\033[38;2;128;46;67m▄\033[48;2;79;51;57m\033[38;2;129;42;63m▄\033[48;2;104;61;68m\033[38;2;137;42;64m▄\033[48;2;64;43;48m\033[38;2;140;50;71m▄\033[48;2;53;41;46m\033[38;2;118;50;68m▄\033[48;2;45;40;41m\033[38;2;98;45;60m▄\033[48;2;50;48;49m\033[38;2;93;51;62m▄\033[48;2;60;58;58m\033[38;2;85;55;61m▄\033[48;2;75;73;74m\033[38;2;78;61;64m▄\033[48;2;70;67;63m\033[38;2;68;57;56m▄\033[48;2;60;55;52m\033[38;2;68;58;56m▄\033[48;2;64;59;56m\033[38;2;66;57;55m▄\033[48;2;58;53;50m\033[38;2;69;60;57m▄\033[48;2;51;45;43m\033[38;2;68;59;56m▄\033[48;2;39;35;34m\033[38;2;63;55;53m▄\033[48;2;53;49;45m\033[38;2;58;50;49m▄\033[48;2;49;45;42m\033[38;2;65;59;56m▄\033[48;2;47;43;42m\033[38;2;53;46;44m▄\033[48;2;43;41;40m\033[38;2;48;42;40m▄\033[48;2;40;39;38m\033[38;2;38;35;33m▄\033[48;2;38;37;37m\033[38;2;32;32;31m▄\033[48;2;35;33;34m\033[38;2;33;31;31m▄\033[48;2;32;32;31m\033[38;2;35;34;33m▄\033[48;2;24;23;23m\033[38;2;32;30;29m▄\033[48;2;44;36;32m\033[38;2;43;36;32m▄\033[48;2;79;61;48m\033[38;2;66;50;39m▄\033[48;2;89;69;52m\033[38;2;68;51;39m▄\033[48;2;70;54;43m\033[38;2;60;45;35m▄\033[48;2;71;52;40m\033[38;2;70;50;38m▄\033[48;2;81;58;45m\033[38;2;75;52;39m▄\033[48;2;80;57;42m\033[38;2;76;54;40m▄\033[48;2;78;54;40m \033[48;2;83;57;43m\033[38;2;88;61;45m▄\033[48;2;96;67;49m\033[38;2;102;72;52m▄\033[48;2;108;76;55m\033[38;2;111;78;56m▄\033[48;2;121;85;61m\033[38;2;119;83;60m▄\033[48;2;130;93;66m\033[38;2;120;83;58m▄\033[48;2;133;93;65m\033[38;2;119;80;54m▄\033[48;2;127;86;58m\033[38;2;116;76;49m▄\033[48;2;123;80;54m\033[38;2;109;68;43m▄\033[48;2;114;71;47m\033[38;2;99;60;39m▄\033[48;2;105;66;44m\033[38;2;97;60;40m▄\033[48;2;104;73;54m\033[38;2;96;62;43m▄\033[48;2;84;69;60m\033[38;2;88;64;51m▄\033[48;2;41;37;34m\033[38;2;56;47;42m▄\033[48;2;3;3;3m\033[38;2;8;6;6m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;1;1;0m\033[38;2;0;0;0m▄\033[48;2;37;34;31m\033[38;2;20;18;17m▄\033[48;2;154;121;106m\033[38;2;138;97;96m▄\033[48;2;208;85;104m\033[38;2;213;53;87m▄\033[48;2;223;58;102m\033[38;2;223;53;98m▄\033[48;2;226;61;114m\033[38;2;228;56;108m▄\033[48;2;224;57;118m\033[38;2;229;55;115m▄\033[48;2;216;53;114m\033[38;2;223;83;138m▄\033[48;2;196;53;97m\033[38;2;195;67;109m▄\033[48;2;207;58;107m\033[38;2;211;66;113m▄\033[48;2;200;49;103m\033[38;2;215;55;115m▄\033[48;2;169;41;84m\033[38;2;199;49;106m▄\033[48;2;161;38;75m\033[38;2;189;43;91m▄\033[48;2;150;34;71m\033[38;2;183;40;88m▄\033[48;2;147;33;65m\033[38;2;177;37;82m▄\033[48;2;160;35;68m\033[38;2;173;34;75m▄\033[48;2;158;31;66m\033[38;2;170;30;70m▄\033[48;2;154;33;68m\033[38;2;167;28;68m▄\033[48;2;147;35;66m\033[38;2;156;29;66m▄\033[48;2;142;40;66m\033[38;2;155;32;65m▄\033[48;2;126;43;62m\033[38;2;161;35;67m▄\033[48;2;115;43;60m\033[38;2;162;25;62m▄\033[48;2;72;48;51m\033[38;2;79;32;43m▄\033[48;2;70;52;53m\033[38;2;53;30;34m▄\033[48;2;75;57;56m\033[38;2;44;28;31m▄\033[48;2;84;65;62m\033[38;2;58;37;40m▄\033[48;2;91;73;70m\033[38;2;66;42;45m▄\033[48;2;77;62;61m\033[38;2;58;40;41m▄\033[48;2;63;51;50m\033[38;2;49;34;34m▄\033[48;2;54;39;39m\033[38;2;56;37;37m▄\033[48;2;47;34;33m\033[38;2;58;42;39m▄\033[48;2;25;22;20m\033[38;2;36;30;26m▄\033[48;2;14;15;16m\033[38;2;13;14;14m▄\033[48;2;22;21;23m\033[38;2;18;18;20m▄\033[48;2;22;22;23m\033[38;2;18;17;19m▄\033[48;2;33;31;29m\033[38;2;33;30;29m▄\033[48;2;47;40;36m\033[38;2;55;45;39m▄\033[48;2;66;49;39m\033[38;2;75;56;43m▄\033[48;2;73;54;40m\033[38;2;84;61;46m▄\033[48;2;75;54;40m\033[38;2;82;57;42m▄\033[48;2;73;51;38m\033[38;2;79;55;41m▄\033[48;2;74;51;38m\033[38;2;79;57;43m▄\033[48;2;77;55;40m\033[38;2;85;61;46m▄\033[48;2;81;56;41m\033[38;2;87;61;45m▄\033[48;2;89;62;45m\033[38;2;88;61;44m▄\033[48;2;99;68;48m\033[38;2;98;66;47m▄\033[48;2;107;73;51m\033[38;2;109;74;53m▄\033[48;2;111;75;51m\033[38;2;110;73;49m▄\033[48;2;111;74;49m\033[38;2;110;74;49m▄\033[48;2;114;77;52m \033[48;2;114;74;49m\033[38;2;107;69;46m▄\033[48;2;104;64;40m\033[38;2;95;58;38m▄\033[48;2;94;57;36m\033[38;2;90;55;35m▄\033[48;2;91;56;37m\033[38;2;89;55;35m▄\033[48;2;93;58;40m\033[38;2;94;60;41m▄\033[48;2;101;75;61m\033[38;2;107;81;65m▄\033[48;2;68;59;53m\033[38;2;74;64;58m▄\033[48;2;8;7;6m\033[38;2;9;8;7m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;10;10;10m\033[38;2;12;12;12m▄\033[48;2;119;62;74m\033[38;2;126;43;65m▄\033[48;2;209;47;83m\033[38;2;213;68;109m▄\033[48;2;220;59;105m\033[38;2;222;72;119m▄\033[48;2;228;57;110m\033[38;2;232;80;134m▄\033[48;2;235;66;134m\033[38;2;238;79;146m▄\033[48;2;231;91;153m\033[38;2;227;73;142m▄\033[48;2;217;86;137m\033[38;2;229;100;152m▄\033[48;2;219;76;125m\033[38;2;229;90;142m▄\033[48;2;218;55;115m\033[38;2;216;60;118m▄\033[48;2;210;45;106m\033[38;2;208;54;108m▄\033[48;2;203;40;96m\033[38;2;206;48;100m▄\033[48;2;201;41;96m\033[38;2;211;48;108m▄\033[48;2;181;35;84m\033[38;2;194;41;99m▄\033[48;2;163;29;68m\033[38;2;177;38;87m▄\033[48;2;172;32;71m\033[38;2;174;34;75m▄\033[48;2;179;35;74m\033[38;2;187;38;80m▄\033[48;2;172;33;75m\033[38;2;191;40;86m▄\033[48;2;164;38;76m\033[38;2;188;44;92m▄\033[48;2;173;45;84m\033[38;2;184;40;86m▄\033[48;2;183;24;70m\033[38;2;189;23;71m▄\033[48;2;136;31;60m\033[38;2;160;28;64m▄\033[48;2;59;26;33m\033[38;2;75;27;39m▄\033[48;2;36;19;23m\033[38;2;63;25;34m▄\033[48;2;45;24;29m\033[38;2;66;31;38m▄\033[48;2;53;28;33m\033[38;2;72;41;44m▄\033[48;2;61;40;43m\033[38;2;80;54;57m▄\033[48;2;64;44;44m\033[38;2;89;61;60m▄\033[48;2;76;53;52m\033[38;2;95;66;65m▄\033[48;2;74;57;51m\033[38;2;55;40;40m▄\033[48;2;45;38;33m\033[38;2;21;19;20m▄\033[48;2;15;15;17m\033[38;2;14;15;17m▄\033[48;2;17;17;19m\033[38;2;18;18;20m▄\033[48;2;20;19;21m\033[38;2;22;22;22m▄\033[48;2;37;34;32m\033[38;2;38;35;34m▄\033[48;2;62;50;42m \033[48;2;83;62;47m\033[38;2;80;59;44m▄\033[48;2;87;62;47m\033[38;2;80;57;41m▄\033[48;2;82;57;42m\033[38;2;76;54;40m▄\033[48;2;80;57;42m\033[38;2;82;60;46m▄\033[48;2;83;59;44m\033[38;2;91;67;51m▄\033[48;2;90;65;49m\033[38;2;93;67;50m▄\033[48;2;88;62;45m\033[38;2;95;67;49m▄\033[48;2;89;62;45m\033[38;2;101;71;51m▄\033[48;2;104;72;52m\033[38;2;101;70;49m▄\033[48;2;109;75;52m\033[38;2;101;66;44m▄\033[48;2;111;75;51m\033[38;2;107;71;48m▄\033[48;2;111;74;48m\033[38;2;108;72;48m▄\033[48;2;105;68;45m\033[38;2;98;62;40m▄\033[48;2;96;60;40m\033[38;2;89;54;34m▄\033[48;2;90;56;37m\033[38;2;90;57;38m▄\033[48;2;90;56;37m\033[38;2;95;59;40m▄\033[48;2;89;55;35m\033[38;2;94;58;38m▄\033[48;2;92;57;38m\033[38;2;97;63;44m▄\033[48;2;103;72;55m\033[38;2;110;85;71m▄\033[48;2;76;61;53m\033[38;2;71;62;56m▄\033[48;2;9;9;7m\033[38;2;6;6;5m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;17;12;13m\033[38;2;18;10;11m▄\033[48;2;147;30;55m\033[38;2;149;35;59m▄\033[48;2;216;50;91m\033[38;2;218;50;90m▄\033[48;2;223;63;111m\033[38;2;226;59;107m▄\033[48;2;237;101;151m\033[38;2;236;88;140m▄\033[48;2;244;118;173m\033[38;2;239;69;130m▄\033[48;2;238;88;160m\033[38;2;234;52;131m▄\033[48;2;232;79;149m\033[38;2;227;78;142m▄\033[48;2;224;70;137m\033[38;2;222;80;137m▄\033[48;2;210;59;118m\033[38;2;213;59;117m▄\033[48;2;194;48;98m\033[38;2;209;52;106m▄\033[48;2;196;45;91m\033[38;2;208;54;102m▄\033[48;2;207;43;96m\033[38;2;208;42;94m▄\033[48;2;203;38;98m\033[38;2;202;35;91m▄\033[48;2;191;37;93m\033[38;2;198;36;91m▄\033[48;2;193;39;91m\033[38;2;196;34;88m▄\033[48;2;204;47;98m\033[38;2;197;34;89m▄\033[48;2;199;42;92m\033[38;2;195;34;90m▄\033[48;2;194;39;91m\033[38;2;195;36;89m▄\033[48;2;190;32;82m\033[38;2;196;46;93m▄\033[48;2;189;31;77m\033[38;2;189;27;76m▄\033[48;2;161;27;64m\033[38;2;165;27;68m▄\033[48;2;96;29;48m\033[38;2;108;29;51m▄\033[48;2;88;30;44m\033[38;2;100;35;50m▄\033[48;2;90;42;51m\033[38;2;106;53;62m▄\033[48;2;95;56;60m\033[38;2;119;69;75m▄\033[48;2;99;62;65m\033[38;2;124;75;79m▄\033[48;2;108;71;72m\033[38;2;119;78;79m▄\033[48;2;104;72;70m\033[38;2;98;71;67m▄\033[48;2;65;49;45m\033[38;2;49;40;36m▄\033[48;2;22;20;21m\033[38;2;20;19;19m▄\033[48;2;17;18;18m\033[38;2;19;20;21m▄\033[48;2;19;20;19m \033[48;2;23;23;23m\033[38;2;25;24;23m▄\033[48;2;33;30;28m\033[38;2;41;35;31m▄\033[48;2;55;43;35m\033[38;2;62;46;36m▄\033[48;2;76;56;42m\033[38;2;74;55;41m▄\033[48;2;78;56;41m\033[38;2;73;51;38m▄\033[48;2;75;53;39m\033[38;2;77;55;41m▄\033[48;2;82;59;45m\033[38;2;83;61;45m▄\033[48;2;88;63;46m \033[48;2;92;65;47m\033[38;2;88;61;43m▄\033[48;2;96;67;47m\033[38;2;96;66;46m▄\033[48;2;98;66;46m\033[38;2;100;67;46m▄\033[48;2;95;62;42m\033[38;2;96;62;42m▄\033[48;2;96;61;41m\033[38;2;99;64;44m▄\033[48;2;97;62;42m\033[38;2;94;59;39m▄\033[48;2;92;56;37m\033[38;2;87;51;33m▄\033[48;2;87;52;33m\033[38;2;89;54;35m▄\033[48;2;89;54;34m\033[38;2;92;57;37m▄\033[48;2;92;57;37m\033[38;2;96;60;40m▄\033[48;2;97;60;41m\033[38;2;99;62;42m▄\033[48;2;96;59;40m \033[48;2;97;62;43m\033[38;2;96;61;42m▄\033[48;2;101;75;61m\033[38;2;97;70;55m▄\033[48;2;53;44;39m\033[38;2;49;39;34m▄\033[48;2;3;2;2m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;9;7;8m\033[38;2;2;2;2m▄\033[48;2;105;31;47m\033[38;2;47;23;29m▄\033[48;2;211;47;86m\033[38;2;181;43;75m▄\033[48;2;225;63;111m\033[38;2;222;56;99m▄\033[48;2;227;62;118m\033[38;2;223;56;106m▄\033[48;2;230;57;114m\033[38;2;224;53;106m▄\033[48;2;230;51;112m\033[38;2;226;56;111m▄\033[48;2;223;54;113m\033[38;2;225;55;113m▄\033[48;2;217;58;114m\033[38;2;222;55;114m▄\033[48;2;216;55;110m\033[38;2;222;52;115m▄\033[48;2;215;50;106m\033[38;2;221;48;118m▄\033[48;2;212;49;102m\033[38;2;208;39;103m▄\033[48;2;208;40;92m\033[38;2;202;35;93m▄\033[48;2;204;34;88m\033[38;2;207;36;91m▄\033[48;2;200;36;93m\033[38;2;208;42;102m▄\033[48;2;191;35;94m\033[38;2;191;36;99m▄\033[48;2;191;35;93m\033[38;2;191;37;97m▄\033[48;2;193;35;94m\033[38;2;197;42;101m▄\033[48;2;196;32;86m\033[38;2;204;45;95m▄\033[48;2;201;45;95m\033[38;2;201;41;92m▄\033[48;2;201;30;91m\033[38;2;200;26;83m▄\033[48;2;168;34;79m\033[38;2;144;31;66m▄\033[48;2;120;33;57m\033[38;2;124;38;60m▄\033[48;2;112;43;58m\033[38;2;124;54;69m▄\033[48;2;122;64;74m\033[38;2;136;74;84m▄\033[48;2;140;81;90m\033[38;2;140;83;90m▄\033[48;2;132;81;85m\033[38;2;125;75;78m▄\033[48;2;111;73;73m\033[38;2;87;61;58m▄\033[48;2;60;48;47m\033[38;2;28;26;27m▄\033[48;2;28;27;27m\033[38;2;21;21;23m▄\033[48;2;19;19;20m\033[38;2;21;21;22m▄\033[48;2;21;21;23m\033[38;2;21;21;21m▄\033[48;2;22;22;22m\033[38;2;22;21;22m▄\033[48;2;29;27;26m\033[38;2;30;28;27m▄\033[48;2;44;35;30m\033[38;2;49;38;31m▄\033[48;2;65;48;36m\033[38;2;65;47;35m▄\033[48;2;70;51;38m\033[38;2;65;46;34m▄\033[48;2;70;50;36m\033[38;2;67;48;35m▄\033[48;2;78;56;42m\033[38;2;80;58;43m▄\033[48;2;85;62;46m\033[38;2;90;65;48m▄\033[48;2;87;62;44m\033[38;2;91;63;45m▄\033[48;2;87;60;42m\033[38;2;90;60;42m▄\033[48;2;94;64;45m\033[38;2;92;61;43m▄\033[48;2;98;66;45m\033[38;2;94;63;42m▄\033[48;2;93;60;39m\033[38;2;92;60;39m▄\033[48;2;94;59;39m\033[38;2;93;58;38m▄\033[48;2;91;56;36m\033[38;2;90;56;37m▄\033[48;2;89;54;35m\033[38;2;92;57;37m▄\033[48;2;94;58;39m\033[38;2;95;60;40m▄\033[48;2;100;64;42m\033[38;2;105;67;45m▄\033[48;2;100;63;41m\033[38;2;103;64;40m▄\033[48;2;96;59;38m\033[38;2;88;53;33m▄\033[48;2;88;53;34m\033[38;2;80;47;31m▄\033[48;2;90;57;39m\033[38;2;82;50;34m▄\033[48;2;100;75;61m\033[38;2;91;64;49m▄\033[48;2;56;48;43m\033[38;2;68;55;49m▄\033[48;2;4;3;3m\033[38;2;9;7;7m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "";
                    text r_grut03 = ansi::esc(
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;15;13;13m\033[38;2;4;4;4m▄\033[48;2;134;38;60m\033[38;2;92;25;39m▄\033[48;2;215;47;85m\033[38;2;199;39;76m▄\033[48;2;223;45;93m\033[38;2;224;43;92m▄\033[48;2;223;48;102m\033[38;2;225;43;102m▄\033[48;2;226;51;114m\033[38;2;223;47;115m▄\033[48;2;223;48;116m\033[38;2;222;53;123m▄\033[48;2;224;50;115m\033[38;2;222;53;120m▄\033[48;2;226;46;119m\033[38;2;232;51;125m▄\033[48;2;222;45;125m\033[38;2;236;52;137m▄\033[48;2;213;43;117m\033[38;2;223;52;131m▄\033[48;2;203;42;103m\033[38;2;215;58;119m▄\033[48;2;210;43;99m\033[38;2;214;49;107m▄\033[48;2;209;40;100m\033[38;2;208;38;98m▄\033[48;2;196;37;99m\033[38;2;201;36;96m▄\033[48;2;196;36;95m\033[38;2;196;33;89m▄\033[48;2;199;34;92m\033[38;2;199;36;90m▄\033[48;2;201;35;85m\033[38;2;198;36;86m▄\033[48;2;194;31;79m\033[38;2;192;31;77m▄\033[48;2;174;23;67m\033[38;2;153;27;63m▄\033[48;2;133;33;61m\033[38;2;122;39;59m▄\033[48;2;126;47;66m\033[38;2;132;61;75m▄\033[48;2;132;67;80m\033[38;2;137;75;85m▄\033[48;2;138;79;87m\033[38;2;129;74;79m▄\033[48;2;126;73;79m\033[38;2;114;67;70m▄\033[48;2;102;65;66m\033[38;2;62;45;46m▄\033[48;2;33;29;30m\033[38;2;16;16;18m▄\033[48;2;20;20;22m\033[38;2;21;21;23m▄\033[48;2;19;19;21m\033[38;2;22;22;23m▄\033[48;2;22;22;24m\033[38;2;23;23;24m▄\033[48;2;23;23;23m\033[38;2;27;25;26m▄\033[48;2;28;27;26m\033[38;2;33;31;30m▄\033[48;2;32;28;27m\033[38;2;36;32;28m▄\033[48;2;54;41;32m\033[38;2;53;39;30m▄\033[48;2;59;43;32m\033[38;2;56;40;30m▄\033[48;2;61;43;31m\033[38;2;58;41;29m▄\033[48;2;68;49;36m\033[38;2;71;51;38m▄\033[48;2;82;59;45m\033[38;2;84;60;45m▄\033[48;2;95;68;50m\033[38;2;92;65;46m▄\033[48;2;98;69;49m\033[38;2;97;66;46m▄\033[48;2;100;68;47m\033[38;2;101;68;47m▄\033[48;2;98;65;45m\033[38;2;97;63;42m▄\033[48;2;96;63;41m\033[38;2;99;63;41m▄\033[48;2;92;58;37m\033[38;2;95;58;37m▄\033[48;2;92;57;37m\033[38;2;90;54;34m▄\033[48;2;93;59;40m\033[38;2;95;59;40m▄\033[48;2;94;59;39m\033[38;2;106;69;46m▄\033[48;2;100;64;42m\033[38;2;113;74;48m▄\033[48;2;107;70;46m\033[38;2;109;68;44m▄\033[48;2;105;66;42m\033[38;2;95;59;38m▄\033[48;2;82;48;31m\033[38;2;77;46;30m▄\033[48;2;75;44;30m\033[38;2;70;41;27m▄\033[48;2;79;47;32m\033[38;2;75;45;30m▄\033[48;2;86;55;40m\033[38;2;84;52;38m▄\033[48;2;80;61;52m\033[38;2;92;71;61m▄\033[48;2;15;13;12m\033[38;2;29;24;22m▄\033[48;2;0;0;0m\033[38;2;1;1;1m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;48;19;25m\033[38;2;24;20;20m▄\033[48;2;164;40;71m\033[38;2;121;71;79m▄\033[48;2;218;37;90m\033[38;2;201;65;100m▄\033[48;2;223;38;98m\033[38;2;222;47;100m▄\033[48;2;224;50;114m\033[38;2;227;55;110m▄\033[48;2;225;58;124m\033[38;2;226;55;113m▄\033[48;2;225;58;126m\033[38;2;229;57;121m▄\033[48;2;232;54;130m\033[38;2;236;60;136m▄\033[48;2;239;53;139m\033[38;2;237;56;140m▄\033[48;2;232;52;134m\033[38;2;227;50;127m▄\033[48;2;220;45;115m\033[38;2;217;41;108m▄\033[48;2;211;37;100m\033[38;2;204;32;93m▄\033[48;2;206;34;94m\033[38;2;207;35;95m▄\033[48;2;195;35;87m\033[38;2;191;30;80m▄\033[48;2;193;35;86m\033[38;2;190;32;80m▄\033[48;2;195;34;85m\033[38;2;193;32;85m▄\033[48;2;194;31;82m\033[38;2;189;33;87m▄\033[48;2;184;29;80m\033[38;2;148;30;71m▄\033[48;2;136;32;61m\033[38;2;144;50;75m▄\033[48;2;130;52;69m\033[38;2;149;75;90m▄\033[48;2;145;77;88m\033[38;2;148;81;89m▄\033[48;2;137;77;84m\033[38;2;147;94;98m▄\033[48;2;117;68;72m\033[38;2;109;76;75m▄\033[48;2;93;57;59m\033[38;2;50;39;39m▄\033[48;2;25;24;25m\033[38;2;29;28;28m▄\033[48;2;22;22;23m\033[38;2;30;29;30m▄\033[48;2;25;25;26m\033[38;2;30;29;29m▄\033[48;2;25;25;26m\033[38;2;27;27;27m▄\033[48;2;25;25;25m\033[38;2;29;28;28m▄\033[48;2;27;26;25m\033[38;2;32;29;27m▄\033[48;2;25;22;22m\033[38;2;35;28;25m▄\033[48;2;41;31;26m\033[38;2;44;32;25m▄\033[48;2;50;36;27m\033[38;2;51;36;27m▄\033[48;2;54;39;30m\033[38;2;57;41;31m▄\033[48;2;63;45;34m\033[38;2;72;52;38m▄\033[48;2;74;54;40m\033[38;2;79;57;41m▄\033[48;2;85;59;44m\033[38;2;83;57;40m▄\033[48;2;96;67;46m\033[38;2;95;64;44m▄\033[48;2;99;67;46m\033[38;2;99;67;45m▄\033[48;2;98;64;42m \033[48;2;100;65;43m\033[38;2;96;61;41m▄\033[48;2;98;62;40m\033[38;2;97;61;40m▄\033[48;2;95;58;36m\033[38;2;99;62;39m▄\033[48;2;97;62;40m\033[38;2;100;64;40m▄\033[48;2;102;64;42m\033[38;2;104;67;43m▄\033[48;2;105;67;43m\033[38;2;106;68;44m▄\033[48;2;111;72;47m\033[38;2;103;65;43m▄\033[48;2;109;69;46m\033[38;2;98;62;41m▄\033[48;2;90;56;35m\033[38;2;82;50;33m▄\033[48;2;72;43;30m\033[38;2;67;40;27m▄\033[48;2;69;41;28m\033[38;2;67;40;27m▄\033[48;2;74;44;30m\033[38;2;71;42;28m▄\033[48;2;79;47;32m\033[38;2;75;45;30m▄\033[48;2;83;54;39m\033[38;2;83;51;34m▄\033[48;2;58;43;36m\033[38;2;84;56;41m▄\033[48;2;10;8;7m\033[38;2;41;31;26m▄\033[48;2;0;0;0m\033[38;2;5;5;4m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;16;14;13m\033[38;2;11;10;9m▄\033[48;2;98;77;68m\033[38;2;80;66;57m▄\033[48;2;180;99;94m\033[38;2;162;114;86m▄\033[48;2;214;64;96m\033[38;2;189;102;84m▄\033[48;2;224;49;97m\033[38;2;207;65;86m▄\033[48;2;224;47;100m\033[38;2;215;44;88m▄\033[48;2;227;50;111m\033[38;2;217;45;98m▄\033[48;2;229;52;122m\033[38;2;220;46;106m▄\033[48;2;226;49;122m\033[38;2;221;54;116m▄\033[48;2;215;44;110m\033[38;2;215;52;109m▄\033[48;2;214;40;104m\033[38;2;222;48;116m▄\033[48;2;207;36;96m\033[38;2;212;40;101m▄\033[48;2;212;41;104m\033[38;2;207;47;103m▄\033[48;2;201;36;90m\033[38;2;170;41;72m▄\033[48;2;178;28;71m\033[38;2;136;30;56m▄\033[48;2;169;31;76m\033[38;2;122;34;57m▄\033[48;2;148;34;73m\033[38;2;122;44;64m▄\033[48;2;128;41;66m\033[38;2;144;67;86m▄\033[48;2;161;78;98m\033[38;2;179;106;122m▄\033[48;2;164;90;105m\033[38;2;179;103;118m▄\033[48;2;165;92;100m\033[38;2;201;128;136m▄\033[48;2;195;151;150m\033[38;2;224;183;173m▄\033[48;2;129;111;105m\033[38;2;136;113;103m▄\033[48;2;41;36;36m\033[38;2;45;40;40m▄\033[48;2;37;35;34m\033[38;2;39;35;35m▄\033[48;2;33;31;32m\033[38;2;31;29;29m▄\033[48;2;29;28;29m\033[38;2;28;26;26m▄\033[48;2;30;29;28m\033[38;2;31;28;27m▄\033[48;2;34;31;30m\033[38;2;34;28;25m▄\033[48;2;34;27;25m\033[38;2;36;26;23m▄\033[48;2;39;31;25m\033[38;2;40;31;25m▄\033[48;2;47;33;25m\033[38;2;50;37;28m▄\033[48;2;54;39;30m\033[38;2;57;42;31m▄\033[48;2;61;44;33m\033[38;2;65;46;34m▄\033[48;2;75;54;39m\033[38;2;79;57;39m▄\033[48;2;84;59;42m\033[38;2;92;65;46m▄\033[48;2;94;67;46m\033[38;2;107;77;55m▄\033[48;2;92;61;41m\033[38;2;96;64;43m▄\033[48;2;98;65;44m\033[38;2;95;61;40m▄\033[48;2;102;67;43m\033[38;2;104;68;43m▄\033[48;2;97;63;41m\033[38;2;104;68;44m▄\033[48;2;99;64;43m\033[38;2;105;69;47m▄\033[48;2;111;73;50m\033[38;2;115;77;52m▄\033[48;2;115;76;50m\033[38;2;120;79;51m▄\033[48;2;114;75;49m\033[38;2;114;73;47m▄\033[48;2;109;71;46m\033[38;2;108;69;44m▄\033[48;2;92;57;37m\033[38;2;90;57;37m▄\033[48;2;76;46;30m\033[38;2;68;42;28m▄\033[48;2;68;42;28m\033[38;2;65;42;29m▄\033[48;2;66;40;27m\033[38;2;68;43;29m▄\033[48;2;69;42;29m\033[38;2;74;46;31m▄\033[48;2;74;44;30m\033[38;2;82;51;34m▄\033[48;2;76;46;30m\033[38;2;86;52;34m▄\033[48;2;83;50;33m\033[38;2;89;54;34m▄\033[48;2;92;58;40m\033[38;2;95;58;38m▄\033[48;2;76;53;40m\033[38;2;93;59;40m▄\033[48;2;31;24;20m\033[38;2;72;48;35m▄\033[48;2;4;3;3m\033[38;2;30;22;18m▄\033[48;2;0;0;0m\033[38;2;4;3;2m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;4;4;4m\033[38;2;1;1;1m▄\033[48;2;57;49;45m\033[38;2;36;32;29m▄\033[48;2;150;116;93m\033[38;2;128;99;81m▄\033[48;2;176;122;84m\033[38;2;173;123;86m▄\033[48;2;186;121;84m\033[38;2;184;126;84m▄\033[48;2;191;115;86m\033[38;2;187;128;85m▄\033[48;2;205;93;99m\033[38;2;193;131;89m▄\033[48;2;216;65;108m\033[38;2;202;137;101m▄\033[48;2;218;71;117m\033[38;2;206;146;111m▄\033[48;2;212;85;110m\033[38;2;211;160;124m▄\033[48;2;222;116;127m\033[38;2;215;172;134m▄\033[48;2;208;125;122m\033[38;2;211;170;137m▄\033[48;2;175;96;97m\033[38;2;130;92;86m▄\033[48;2;143;68;74m\033[38;2;101;65;66m▄\033[48;2;130;53;71m\033[38;2;128;75;86m▄\033[48;2;139;61;78m\033[38;2;178;101;119m▄\033[48;2;156;78;96m\033[38;2;188;109;127m▄\033[48;2;170;93;110m\033[38;2;192;116;133m▄\033[48;2;186;112;126m\033[38;2;188;116;130m▄\033[48;2;188;114;127m\033[38;2;171;106;113m▄\033[48;2;212;158;160m\033[38;2;146;107;105m▄\033[48;2;214;186;165m\033[38;2;136;117;101m▄\033[48;2;90;72;64m\033[38;2;54;46;40m▄\033[48;2;38;32;32m\033[38;2;35;33;32m▄\033[48;2;32;30;29m\033[38;2;26;25;25m▄\033[48;2;30;28;28m\033[38;2;41;37;36m▄\033[48;2;37;33;32m\033[38;2;49;43;39m▄\033[48;2;37;31;28m\033[38;2;35;29;25m▄\033[48;2;32;24;21m\033[38;2;31;25;21m▄\033[48;2;37;29;25m\033[38;2;37;30;24m▄\033[48;2;38;28;23m\033[38;2;40;31;25m▄\033[48;2;51;37;29m\033[38;2;50;36;27m▄\033[48;2;60;44;33m\033[38;2;60;43;31m▄\033[48;2;73;52;37m\033[38;2;73;52;36m▄\033[48;2;89;63;44m\033[38;2;90;62;42m▄\033[48;2;103;72;51m\033[38;2;98;66;45m▄\033[48;2;102;70;48m\033[38;2;102;66;44m▄\033[48;2;102;68;45m\033[38;2;103;67;44m▄\033[48;2;97;62;41m\033[38;2;98;62;40m▄\033[48;2;105;68;44m \033[48;2;105;69;43m\033[38;2;111;74;49m▄\033[48;2;98;63;41m\033[38;2;89;57;36m▄\033[48;2;121;83;55m\033[38;2;96;64;43m▄\033[48;2;108;70;44m\033[38;2;101;66;43m▄\033[48;2;104;66;42m\033[38;2;97;62;40m▄\033[48;2;105;67;43m\033[38;2;95;60;38m▄\033[48;2;88;54;35m\033[38;2;77;47;30m▄\033[48;2;66;41;27m\033[38;2;64;40;26m▄\033[48;2;60;38;25m\033[38;2;63;39;27m▄\033[48;2;67;42;29m\033[38;2;66;41;28m▄\033[48;2;77;48;32m\033[38;2;72;45;30m▄\033[48;2;87;54;35m\033[38;2;81;51;34m▄\033[48;2;91;56;37m\033[38;2;91;57;38m▄\033[48;2;94;57;36m\033[38;2;97;60;38m▄\033[48;2;97;58;37m\033[38;2;100;61;39m▄\033[48;2;103;64;43m\033[38;2;106;68;44m▄\033[48;2;99;64;43m\033[38;2;106;69;44m▄\033[48;2;74;51;38m\033[38;2;97;64;43m▄\033[48;2;28;21;17m\033[38;2;62;43;32m▄\033[48;2;2;1;1m\033[38;2;12;9;7m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;17;15;14m\033[38;2;7;7;6m▄\033[48;2;101;84;72m\033[38;2;77;68;61m▄\033[48;2;170;127;96m\033[38;2;159;122;95m▄\033[48;2;185;129;87m\033[38;2;181;126;84m▄\033[48;2;189;130;87m\033[38;2;195;139;94m▄\033[48;2;196;138;94m\033[38;2;199;144;100m▄\033[48;2;200;144;101m\033[38;2;203;151;108m▄\033[48;2;208;157;117m\033[38;2;206;156;115m▄\033[48;2;210;163;124m\033[38;2;204;156;117m▄\033[48;2;207;164;128m\033[38;2;199;154;119m▄\033[48;2;196;156;121m\033[38;2;178;138;107m▄\033[48;2;106;83;85m\033[38;2;88;74;75m▄\033[48;2;171;135;128m\033[38;2;117;96;95m▄\033[48;2;150;109;107m\033[38;2;138;114;111m▄\033[48;2;165;118;125m\033[38;2;147;123;120m▄\033[48;2;180;127;139m\033[38;2;141;117;119m▄\033[48;2;176;123;134m\033[38;2;113;88;90m▄\033[48;2;181;126;130m\033[38;2;153;129;123m▄\033[48;2;155;117;114m\033[38;2;98;83;80m▄\033[48;2;130;111;104m\033[38;2;77;70;67m▄\033[48;2;53;46;45m\033[38;2;41;39;38m▄\033[48;2;42;39;39m\033[38;2;44;43;42m▄\033[48;2;37;35;34m\033[38;2;41;39;39m▄\033[48;2;37;35;34m\033[38;2;61;55;51m▄\033[48;2;65;58;53m\033[38;2;89;78;68m▄\033[48;2;58;51;45m\033[38;2;55;46;40m▄\033[48;2;35;28;24m\033[38;2;46;38;31m▄\033[48;2;36;28;23m\033[38;2;45;35;27m▄\033[48;2;33;26;20m\033[38;2;41;30;23m▄\033[48;2;40;31;24m\033[38;2;42;30;23m▄\033[48;2;48;34;25m\033[38;2;51;35;25m▄\033[48;2;60;41;29m\033[38;2;63;42;29m▄\033[48;2;73;50;34m\033[38;2;80;54;38m▄\033[48;2;91;62;43m\033[38;2;92;63;43m▄\033[48;2;93;62;41m\033[38;2;94;63;41m▄\033[48;2;98;64;43m\033[38;2;97;63;42m▄\033[48;2;105;68;45m\033[38;2;107;70;47m▄\033[48;2;95;59;38m\033[38;2;97;60;38m▄\033[48;2;101;65;42m\033[38;2;103;67;45m▄\033[48;2;111;71;46m\033[38;2;114;74;49m▄\033[48;2;99;64;41m\033[38;2;99;62;41m▄\033[48;2;91;59;39m\033[38;2;91;56;37m▄\033[48;2;96;62;42m\033[38;2;93;59;39m▄\033[48;2;84;52;33m\033[38;2;82;51;33m▄\033[48;2;82;52;33m\033[38;2;78;50;32m▄\033[48;2;79;50;33m\033[38;2;85;56;38m▄\033[48;2;76;49;34m\033[38;2;89;59;40m▄\033[48;2;72;47;32m\033[38;2;85;57;38m▄\033[48;2;70;45;31m\033[38;2;90;59;40m▄\033[48;2;79;51;34m\033[38;2;93;61;41m▄\033[48;2;91;60;40m\033[38;2;103;69;46m▄\033[48;2;101;66;45m\033[38;2;109;74;49m▄\033[48;2;102;66;43m\033[38;2;109;72;47m▄\033[48;2;103;65;42m\033[38;2;108;70;45m▄\033[48;2;102;64;41m\033[38;2;105;68;43m▄\033[48;2;105;68;43m\033[38;2;109;72;46m▄\033[48;2;105;68;44m\033[38;2;110;72;46m▄\033[48;2;89;59;41m\033[38;2;103;67;45m▄\033[48;2;30;21;15m\033[38;2;56;38;27m▄\033[48;2;1;0;0m\033[38;2;5;3;2m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;3;3;3m\033[38;2;1;1;1m▄\033[48;2;60;53;48m\033[38;2;39;35;32m▄\033[48;2;148;116;92m\033[38;2;131;106;87m▄\033[48;2;181;127;86m\033[38;2;180;129;92m▄\033[48;2;196;140;96m\033[38;2;196;141;97m▄\033[48;2;202;148;105m\033[38;2;201;148;104m▄\033[48;2;204;153;111m\033[38;2;203;151;109m▄\033[48;2;204;155;113m\033[38;2;204;156;116m▄\033[48;2;200;153;114m\033[38;2;194;147;108m▄\033[48;2;185;141;106m\033[38;2;173;129;95m▄\033[48;2;165;129;102m\033[38;2;156;118;93m▄\033[48;2;68;59;59m\033[38;2;120;104;93m▄\033[48;2;64;54;57m\033[38;2;52;46;47m▄\033[48;2;84;74;77m\033[38;2;55;51;52m▄\033[48;2;115;105;107m\033[38;2;59;55;57m▄\033[48;2;108;99;102m\033[38;2;51;49;50m▄\033[48;2;73;64;65m\033[38;2;40;37;38m▄\033[48;2;73;63;62m\033[38;2;34;31;29m▄\033[48;2;45;40;40m\033[38;2;31;28;27m▄\033[48;2;34;31;30m\033[38;2;33;30;28m▄\033[48;2;36;35;33m\033[38;2;54;49;45m▄\033[48;2;43;40;39m\033[38;2;95;85;78m▄\033[48;2;70;61;58m\033[38;2;136;120;107m▄\033[48;2;112;98;86m\033[38;2;144;125;109m▄\033[48;2;108;92;78m\033[38;2;136;118;101m▄\033[48;2;72;58;48m\033[38;2;120;102;87m▄\033[48;2;70;58;46m\033[38;2;103;87;72m▄\033[48;2;63;49;37m\033[38;2;87;68;52m▄\033[48;2;56;39;28m\033[38;2;72;51;37m▄\033[48;2;45;30;22m\033[38;2;56;37;27m▄\033[48;2;54;36;25m\033[38;2;60;39;27m▄\033[48;2;66;43;30m\033[38;2;71;45;31m▄\033[48;2;83;55;37m\033[38;2;84;54;36m▄\033[48;2;94;64;43m\033[38;2;95;62;42m▄\033[48;2;99;66;43m\033[38;2;99;64;42m▄\033[48;2;94;59;37m\033[38;2;95;59;37m▄\033[48;2;102;66;43m\033[38;2;98;62;39m▄\033[48;2;99;62;39m\033[38;2;96;61;39m▄\033[48;2;102;66;44m\033[38;2;94;60;39m▄\033[48;2;104;66;42m\033[38;2;90;56;36m▄\033[48;2;89;55;35m\033[38;2;82;50;32m▄\033[48;2;78;48;31m\033[38;2;77;48;32m▄\033[48;2;86;55;36m\033[38;2;80;50;33m▄\033[48;2;87;55;36m\033[38;2;93;61;42m▄\033[48;2;85;55;36m\033[38;2;94;62;42m▄\033[48;2;93;63;43m\033[38;2;103;70;48m▄\033[48;2;101;69;47m\033[38;2;109;75;50m▄\033[48;2;96;65;43m\033[38;2;111;76;50m▄\033[48;2;104;70;48m\033[38;2;115;80;54m▄\033[48;2;106;71;47m\033[38;2;123;87;59m▄\033[48;2;111;76;51m\033[38;2;123;87;59m▄\033[48;2;118;82;56m\033[38;2;127;91;62m▄\033[48;2;121;83;56m\033[38;2;132;95;65m▄\033[48;2;119;80;52m\033[38;2;129;88;58m▄\033[48;2;114;77;49m\033[38;2;123;84;53m▄\033[48;2;112;76;49m\033[38;2;115;78;51m▄\033[48;2;112;74;48m\033[38;2;110;74;48m▄\033[48;2;109;72;47m\033[38;2;108;72;46m▄\033[48;2;83;56;37m\033[38;2;100;68;44m▄\033[48;2;19;12;8m\033[38;2;49;33;21m▄\033[48;2;0;0;0m\033[38;2;2;0;0m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;18;16;15m\033[38;2;9;8;7m▄\033[48;2;106;91;80m\033[38;2;87;77;69m▄\033[48;2;177;133;99m\033[38;2;172;134;104m▄\033[48;2;196;143;101m\033[38;2;195;143;100m▄\033[48;2;204;151;107m\033[38;2;203;153;110m▄\033[48;2;202;150;109m\033[38;2;202;153;113m▄\033[48;2;199;150;110m\033[38;2;192;145;105m▄\033[48;2;181;135;98m\033[38;2;179;131;94m▄\033[48;2;162;118;85m\033[38;2;175;127;92m▄\033[48;2;149;108;79m\033[38;2;160;115;85m▄\033[48;2;159;130;108m\033[38;2;161;125;98m▄\033[48;2;128;113;101m\033[38;2;165;134;111m▄\033[48;2;69;60;57m\033[38;2;163;135;115m▄\033[48;2;53;46;43m\033[38;2;172;149;133m▄\033[48;2;45;39;37m\033[38;2;172;153;140m▄\033[48;2;43;38;36m\033[38;2;173;155;142m▄\033[48;2;44;39;36m\033[38;2;177;158;143m▄\033[48;2;60;54;49m\033[38;2;181;159;141m▄\033[48;2;106;94;86m\033[38;2;187;163;143m▄\033[48;2;144;127;115m\033[38;2;190;167;149m▄\033[48;2;168;148;132m\033[38;2;193;174;154m▄\033[48;2;174;155;138m\033[38;2;191;173;155m▄\033[48;2;158;140;123m\033[38;2;178;161;143m▄\033[48;2;157;139;120m\033[38;2;174;159;141m▄\033[48;2;157;140;124m\033[38;2;174;160;144m▄\033[48;2;136;118;102m\033[38;2;158;141;125m▄\033[48;2;112;91;71m\033[38;2;137;113;92m▄\033[48;2;89;65;45m\033[38;2;107;79;58m▄\033[48;2;74;49;32m\033[38;2;93;63;42m▄\033[48;2;71;46;31m\033[38;2;87;56;35m▄\033[48;2;82;53;35m\033[38;2;91;58;37m▄\033[48;2;88;57;37m\033[38;2;95;61;39m▄\033[48;2;94;60;40m\033[38;2;94;59;38m▄\033[48;2;101;65;43m\033[38;2;103;67;46m▄\033[48;2;100;63;42m\033[38;2;104;66;44m▄\033[48;2;100;64;41m\033[38;2;102;66;43m▄\033[48;2;94;59;37m\033[38;2;86;53;33m▄\033[48;2;85;53;33m\033[38;2;81;50;32m▄\033[48;2;80;48;31m\033[38;2;80;49;33m▄\033[48;2;86;54;36m\033[38;2;85;52;34m▄\033[48;2;82;51;33m\033[38;2;89;56;37m▄\033[48;2;84;55;36m\033[38;2;99;66;45m▄\033[48;2;101;68;46m\033[38;2;107;73;49m▄\033[48;2;109;76;52m\033[38;2;123;87;59m▄\033[48;2;118;83;56m\033[38;2;126;88;60m▄\033[48;2;119;82;56m\033[38;2;131;94;67m▄\033[48;2;118;83;55m\033[38;2;127;89;62m▄\033[48;2;126;91;63m\033[38;2;131;94;65m▄\033[48;2;139;101;69m\033[38;2;132;95;63m▄\033[48;2;143;104;71m\033[38;2;139;101;70m▄\033[48;2;136;97;66m\033[38;2;140;102;72m▄\033[48;2;135;97;67m\033[38;2;136;98;69m▄\033[48;2;137;98;67m\033[38;2;134;97;66m▄\033[48;2;125;86;55m\033[38;2;130;92;62m▄\033[48;2;116;79;51m\033[38;2;121;84;55m▄\033[48;2;113;78;51m\033[38;2;116;80;52m▄\033[48;2;115;81;54m\033[38;2;117;83;56m▄\033[48;2;112;79;53m\033[38;2;117;85;60m▄\033[48;2;68;48;32m\033[38;2;79;58;42m▄\033[48;2;4;2;0m\033[38;2;7;4;2m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;7;6;6m\033[38;2;7;7;6m▄\033[48;2;77;68;62m\033[38;2;80;71;64m▄\033[48;2;168;130;101m\033[38;2;169;131;100m▄\033[48;2;194;144;102m\033[38;2;193;144;103m▄\033[48;2;203;154;111m\033[38;2;204;155;114m▄\033[48;2;203;156;115m\033[38;2;204;157;115m▄\033[48;2;196;148;107m\033[38;2;198;150;109m▄\033[48;2;192;143;102m\033[38;2;197;149;109m▄\033[48;2;187;139;101m\033[38;2;196;150;112m▄\033[48;2;182;138;105m\033[38;2;201;162;130m▄\033[48;2;186;150;120m\033[38;2;199;165;136m▄\033[48;2;173;137;110m\033[38;2;187;151;121m▄\033[48;2;170;136;110m\033[38;2;191;158;130m▄\033[48;2;202;175;152m\033[38;2;216;190;169m▄\033[48;2;215;192;171m\033[38;2;223;200;179m▄\033[48;2;210;188;169m\033[38;2;214;189;167m▄\033[48;2;209;184;164m\033[38;2;207;177;154m▄\033[48;2;205;177;153m\033[38;2;208;178;152m▄\033[48;2;204;179;156m\033[38;2;206;178;153m▄\033[48;2;199;174;152m\033[38;2;202;173;146m▄\033[48;2;199;178;156m\033[38;2;206;183;160m▄\033[48;2;205;191;174m\033[38;2;212;197;179m▄\033[48;2;195;182;161m\033[38;2;206;191;171m▄\033[48;2;187;173;154m\033[38;2;201;187;170m▄\033[48;2;189;175;160m\033[38;2;199;186;169m▄\033[48;2;173;156;139m\033[38;2;183;162;142m▄\033[48;2;152;128;107m\033[38;2;169;144;122m▄\033[48;2;127;97;72m\033[38;2;148;117;90m▄\033[48;2;113;79;53m\033[38;2;134;100;72m▄\033[48;2;103;67;43m\033[38;2;120;83;56m▄\033[48;2;100;64;39m\033[38;2;114;75;47m▄\033[48;2;101;64;40m\033[38;2;112;73;46m▄\033[48;2;101;63;40m\033[38;2;106;68;42m▄\033[48;2;105;68;46m\033[38;2;101;64;42m▄\033[48;2;104;67;45m\033[38;2;98;62;40m▄\033[48;2;93;57;35m\033[38;2;87;53;34m▄\033[48;2;81;50;31m\033[38;2;76;46;28m▄\033[48;2;74;44;29m\033[38;2;70;42;28m▄\033[48;2;79;50;33m\033[38;2;82;53;35m▄\033[48;2;92;59;39m\033[38;2;94;63;41m▄\033[48;2;97;64;43m\033[38;2;107;71;49m▄\033[48;2;106;71;47m\033[38;2;117;80;54m▄\033[48;2;120;84;57m\033[38;2;125;87;58m▄\033[48;2;138;100;69m\033[38;2;142;103;71m▄\033[48;2;140;102;70m\033[38;2;151;112;80m▄\033[48;2;149;109;77m\033[38;2;157;118;84m▄\033[48;2;151;110;77m\033[38;2;157;117;83m▄\033[48;2;140;101;70m\033[38;2;158;119;86m▄\033[48;2;139;101;70m\033[38;2;151;113;80m▄\033[48;2;147;110;78m\033[38;2;150;111;79m▄\033[48;2;151;113;81m\033[38;2;156;116;83m▄\033[48;2;148;109;76m\033[38;2;152;110;76m▄\033[48;2;139;100;68m\033[38;2;136;95;62m▄\033[48;2;130;91;61m\033[38;2;128;88;58m▄\033[48;2;124;86;57m\033[38;2;122;84;55m▄\033[48;2;120;84;56m\033[38;2;119;83;56m▄\033[48;2;120;85;58m\033[38;2;119;84;57m▄\033[48;2;116;83;56m \033[48;2;87;62;43m\033[38;2;96;68;47m▄\033[48;2;16;10;7m\033[38;2;30;19;12m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;9;9;8m \033[48;2;90;78;69m\033[38;2;91;81;73m▄\033[48;2;173;133;101m\033[38;2;175;140;111m▄\033[48;2;193;144;102m\033[38;2;194;147;107m▄\033[48;2;202;155;114m\033[38;2;203;157;117m▄\033[48;2;202;155;114m\033[38;2;204;160;120m▄\033[48;2;201;153;113m\033[38;2;205;163;124m▄\033[48;2;202;157;118m\033[38;2;206;166;131m▄\033[48;2;206;165;130m\033[38;2;213;182;154m▄\033[48;2;212;180;153m\033[38;2;218;195;172m▄\033[48;2;207;176;149m\033[38;2;214;191;167m▄\033[48;2;202;169;139m\033[38;2;211;184;159m▄\033[48;2;209;180;155m\033[38;2;219;197;176m▄\033[48;2;225;203;185m\033[38;2;229;211;195m▄\033[48;2;225;201;181m\033[38;2;225;203;183m▄\033[48;2;216;188;163m\033[38;2;217;190;164m▄\033[48;2;212;182;157m\033[38;2;213;183;158m▄\033[48;2;212;182;156m\033[38;2;213;184;160m▄\033[48;2;208;181;154m\033[38;2;211;189;165m▄\033[48;2;209;183;157m\033[38;2;212;191;169m▄\033[48;2;212;189;166m\033[38;2;211;190;169m▄\033[48;2;207;187;166m\033[38;2;209;190;168m▄\033[48;2;218;203;184m\033[38;2;229;216;199m▄\033[48;2;212;197;179m\033[38;2;220;205;186m▄\033[48;2;206;189;169m\033[38;2;215;197;174m▄\033[48;2;194;170;144m\033[38;2;207;184;159m▄\033[48;2;184;158;131m\033[38;2;198;176;152m▄\033[48;2;169;140;114m\033[38;2;187;164;140m▄\033[48;2;151;118;90m\033[38;2;165;138;112m▄\033[48;2;139;102;72m\033[38;2;160;127;100m▄\033[48;2;131;91;60m\033[38;2;150;111;81m▄\033[48;2;120;79;51m\033[38;2;132;93;62m▄\033[48;2;112;72;45m\033[38;2;121;80;52m▄\033[48;2;106;67;44m\033[38;2;117;77;52m▄\033[48;2;94;58;36m\033[38;2;100;62;39m▄\033[48;2;83;51;32m\033[38;2;85;52;33m▄\033[48;2;72;42;27m\033[38;2;71;41;26m▄\033[48;2;70;42;28m\033[38;2;76;46;30m▄\033[48;2;91;60;41m\033[38;2;101;67;45m▄\033[48;2;97;64;43m\033[38;2;103;68;44m▄\033[48;2;118;80;55m\033[38;2;132;92;63m▄\033[48;2;125;86;58m\033[38;2;141;100;69m▄\033[48;2;124;85;57m\033[38;2;137;97;67m▄\033[48;2;147;108;76m\033[38;2;155;115;81m▄\033[48;2;161;122;89m\033[38;2;162;123;89m▄\033[48;2;166;128;93m\033[38;2;162;126;93m▄\033[48;2;162;123;89m\033[38;2;163;126;93m▄\033[48;2;162;123;88m\033[38;2;167;131;96m▄\033[48;2;153;114;80m\033[38;2;165;126;90m▄\033[48;2;144;106;74m\033[38;2;152;113;79m▄\033[48;2;150;112;80m\033[38;2;149;112;79m▄\033[48;2;153;114;79m\033[38;2;150;112;78m▄\033[48;2;141;100;67m\033[38;2;142;102;71m▄\033[48;2;128;87;57m\033[38;2;130;90;60m▄\033[48;2;118;80;51m\033[38;2;114;78;49m▄\033[48;2;114;79;52m\033[38;2;109;74;48m▄\033[48;2;113;80;54m\033[38;2;102;69;46m▄\033[48;2;107;76;51m\033[38;2;90;61;41m▄\033[48;2;87;61;42m\033[38;2;60;41;28m▄\033[48;2;27;17;11m\033[38;2;8;6;3m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;8;8;8m\033[38;2;14;13;12m▄\033[48;2;88;81;74m\033[38;2;101;91;83m▄\033[48;2;179;148;124m\033[38;2;183;153;125m▄\033[48;2;195;153;113m\033[38;2;198;156;118m▄\033[48;2;203;160;118m\033[38;2;204;161;121m▄\033[48;2;207;165;127m\033[38;2;212;176;141m▄\033[48;2;210;174;139m\033[38;2;217;191;165m▄\033[48;2;213;184;158m\033[38;2;220;202;184m▄\033[48;2;217;197;178m\033[38;2;224;208;193m▄\033[48;2;220;203;184m\033[38;2;220;202;185m▄\033[48;2;217;196;176m\033[38;2;219;200;180m▄\033[48;2;218;195;173m\033[38;2;218;197;176m▄\033[48;2;225;207;190m\033[38;2;228;212;197m▄\033[48;2;235;221;207m\033[38;2;235;224;212m▄\033[48;2;227;206;186m\033[38;2;228;209;190m▄\033[48;2;218;190;164m\033[38;2;219;189;162m▄\033[48;2;213;182;155m\033[38;2;213;177;146m▄\033[48;2;214;185;162m\033[38;2;214;185;161m▄\033[48;2;214;192;171m\033[38;2;213;191;169m▄\033[48;2;214;194;173m\033[38;2;208;186;165m▄\033[48;2;203;182;159m\033[38;2;205;185;163m▄\033[48;2;228;213;196m\033[38;2;233;218;201m▄\033[48;2;232;216;198m\033[38;2;234;222;205m▄\033[48;2;229;218;201m\033[38;2;231;222;206m▄\033[48;2;222;207;186m\033[38;2;224;210;192m▄\033[48;2;215;197;177m\033[38;2;218;200;180m▄\033[48;2;205;187;167m\033[38;2;211;192;172m▄\033[48;2;195;175;155m\033[38;2;203;186;168m▄\033[48;2;181;161;140m\033[38;2;197;181;163m▄\033[48;2;175;151;128m\033[38;2;193;176;159m▄\033[48;2;165;136;109m\033[38;2;181;160;139m▄\033[48;2;148;113;83m\033[38;2;165;133;107m▄\033[48;2;130;91;63m\033[38;2;140;104;78m▄\033[48;2;118;80;55m\033[38;2;102;66;43m▄\033[48;2;102;65;42m\033[38;2;93;59;39m▄\033[48;2;78;46;28m\033[38;2;72;41;26m▄\033[48;2;71;40;25m\033[38;2;79;48;30m▄\033[48;2;83;51;33m\033[38;2;102;67;44m▄\033[48;2;107;71;46m\033[38;2;123;82;53m▄\033[48;2;113;75;49m\033[38;2;127;86;56m▄\033[48;2;141;101;68m\033[38;2;150;110;76m▄\033[48;2;154;111;76m\033[38;2;161;119;84m▄\033[48;2;149;107;73m\033[38;2;163;119;84m▄\033[48;2;153;112;77m\033[38;2;157;114;79m▄\033[48;2;163;125;90m\033[38;2;165;126;91m▄\033[48;2;162;125;91m\033[38;2;169;132;97m▄\033[48;2;167;132;99m\033[38;2;168;132;98m▄\033[48;2;169;133;99m\033[38;2;163;127;94m▄\033[48;2;169;131;94m\033[38;2;168;132;97m▄\033[48;2;159;119;83m\033[38;2;163;124;89m▄\033[48;2;154;116;81m\033[38;2;155;117;81m▄\033[48;2;152;113;78m\033[38;2;150;111;76m▄\033[48;2;144;104;73m\033[38;2;138;97;66m▄\033[48;2;126;87;57m\033[38;2;121;83;55m▄\033[48;2;112;75;48m\033[38;2;105;70;45m▄\033[48;2;103;69;44m\033[38;2;91;59;38m▄\033[48;2;87;57;36m\033[38;2;81;52;34m▄\033[48;2;67;42;28m\033[38;2;63;40;27m▄\033[48;2;33;21;15m\033[38;2;21;12;9m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;1;0;0m▄\033[48;2;27;25;24m\033[38;2;33;30;28m▄\033[48;2;122;107;94m\033[38;2;131;115;101m▄\033[48;2;190;158;128m\033[38;2;192;161;134m▄\033[48;2;202;162;127m\033[38;2;202;168;135m▄\033[48;2;208;172;138m\033[38;2;210;183;158m▄\033[48;2;216;190;165m\033[38;2;218;202;186m▄\033[48;2;221;204;187m\033[38;2;221;209;197m▄\033[48;2;222;208;193m\033[38;2;222;209;196m▄\033[48;2;222;206;191m\033[38;2;220;204;190m▄\033[48;2;217;199;181m\033[38;2;219;203;186m▄\033[48;2;220;199;179m\033[38;2;219;200;180m▄\033[48;2;219;197;179m\033[38;2;223;205;189m▄\033[48;2;231;219;206m\033[38;2;230;218;206m▄\033[48;2;233;221;209m\033[38;2;229;216;205m▄\033[48;2;223;204;186m\033[38;2;222;201;184m▄\033[48;2;213;179;150m\033[38;2;216;188;163m▄\033[48;2;212;176;145m\033[38;2;213;185;160m▄\033[48;2;213;185;159m\033[38;2;214;189;165m▄\033[48;2;212;190;167m\033[38;2;210;186;163m▄\033[48;2;204;179;155m\033[38;2;212;188;164m▄\033[48;2;220;202;184m\033[38;2;227;212;196m▄\033[48;2;234;223;207m\033[38;2;234;224;211m▄\033[48;2;234;224;208m\033[38;2;233;224;208m▄\033[48;2;230;219;204m\033[38;2;230;218;201m▄\033[48;2;223;208;187m\033[38;2;227;212;192m▄\033[48;2;220;202;179m\033[38;2;225;210;191m▄\033[48;2;215;197;175m\033[38;2;216;201;183m▄\033[48;2;206;191;173m\033[38;2;210;198;181m▄\033[48;2;200;186;169m\033[38;2;203;191;174m▄\033[48;2;197;184;167m\033[38;2;200;187;170m▄\033[48;2;182;160;140m\033[38;2;184;162;143m▄\033[48;2;162;128;101m\033[38;2;164;133;109m▄\033[48;2;129;92;66m\033[38;2;130;93;67m▄\033[48;2;99;62;40m\033[38;2;103;65;40m▄\033[48;2;77;45;29m\033[38;2;81;49;31m▄\033[48;2;97;61;39m\033[38;2;112;73;48m▄\033[48;2;92;58;37m\033[38;2;105;68;43m▄\033[48;2;117;81;56m\033[38;2;126;89;63m▄\033[48;2;144;100;68m\033[38;2;159;117;83m▄\033[48;2;139;95;62m\033[38;2;153;109;75m▄\033[48;2;149;108;75m\033[38;2;152;111;78m▄\033[48;2;166;124;90m\033[38;2;174;135;100m▄\033[48;2;173;131;94m\033[38;2;178;137;100m▄\033[48;2;161;118;83m\033[38;2;165;123;86m▄\033[48;2;167;128;92m\033[38;2;169;129;93m▄\033[48;2;178;143;107m\033[38;2;179;142;107m▄\033[48;2;181;146;111m\033[38;2;180;145;109m▄\033[48;2;174;138;103m\033[38;2;167;129;94m▄\033[48;2;166;130;95m\033[38;2;162;125;91m▄\033[48;2;163;125;90m\033[38;2;160;121;87m▄\033[48;2;153;113;78m\033[38;2;150;110;75m▄\033[48;2;143;103;69m\033[38;2;133;95;64m▄\033[48;2;128;88;57m\033[38;2;113;75;50m▄\033[48;2;111;74;48m\033[38;2;99;64;42m▄\033[48;2;97;63;41m\033[38;2;88;57;38m▄\033[48;2;85;55;35m\033[38;2;88;60;40m▄\033[48;2;81;53;35m\033[38;2;86;58;39m▄\033[48;2;70;46;32m\033[38;2;73;49;34m▄\033[48;2;25;15;10m\033[38;2;32;21;14m▄\033[48;2;0;0;0m\033[38;2;1;0;0m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;1;0;0m▄\033[48;2;30;28;27m\033[38;2;31;30;29m▄\033[48;2;131;121;110m\033[38;2;130;119;108m▄\033[48;2;194;168;144m\033[38;2;195;168;144m▄\033[48;2;202;170;142m\033[38;2;199;164;134m▄\033[48;2;208;184;165m\033[38;2;205;183;165m▄\033[48;2;220;209;199m\033[38;2;219;210;201m▄\033[48;2;222;212;201m\033[38;2;223;213;204m▄\033[48;2;222;211;199m\033[38;2;224;214;205m▄\033[48;2;221;208;195m\033[38;2;222;211;200m▄\033[48;2;220;205;190m\033[38;2;218;204;190m▄\033[48;2;220;201;182m\033[38;2;219;203;184m▄\033[48;2;226;209;193m\033[38;2;225;207;189m▄\033[48;2;227;213;199m\033[38;2;225;208;191m▄\033[48;2;227;211;196m\033[38;2;223;205;186m▄\033[48;2;224;205;188m\033[38;2;221;198;177m▄\033[48;2;218;194;173m\033[38;2;217;193;172m▄\033[48;2;218;197;177m\033[38;2;220;202;185m▄\033[48;2;216;196;175m\033[38;2;217;200;182m▄\033[48;2;214;192;170m\033[38;2;216;197;176m▄\033[48;2;218;198;177m\033[38;2;217;200;182m▄\033[48;2;231;218;205m\033[38;2;229;217;203m▄\033[48;2;235;225;211m\033[38;2;235;225;210m▄\033[48;2;232;223;208m\033[38;2;234;225;208m▄\033[48;2;230;217;199m\033[38;2;229;214;195m▄\033[48;2;229;215;195m\033[38;2;231;218;201m▄\033[48;2;231;220;205m\033[38;2;230;221;207m▄\033[48;2;221;211;196m\033[38;2;225;217;202m▄\033[48;2;215;205;190m\033[38;2;217;208;194m▄\033[48;2;210;200;184m\033[38;2;213;204;188m▄\033[48;2;202;190;174m\033[38;2;204;193;177m▄\033[48;2;189;173;155m\033[38;2;189;173;156m▄\033[48;2;171;149;128m\033[38;2;170;149;129m▄\033[48;2;143;109;84m\033[38;2;153;118;93m▄\033[48;2;112;71;46m\033[38;2;120;77;49m▄\033[48;2;101;65;42m\033[38;2;127;85;55m▄\033[48;2;116;77;51m\033[38;2;136;96;65m▄\033[48;2;134;92;62m\033[38;2;149;107;75m▄\033[48;2;146;108;78m\033[38;2;159;121;89m▄\033[48;2;175;134;99m\033[38;2;181;141;105m▄\033[48;2;173;129;91m\033[38;2;185;140;101m▄\033[48;2;171;129;91m\033[38;2;180;138;97m▄\033[48;2;173;133;97m\033[38;2;176;136;100m▄\033[48;2;181;140;104m\033[38;2;175;133;98m▄\033[48;2;175;134;97m\033[38;2;175;134;98m▄\033[48;2;167;126;92m\033[38;2;165;125;90m▄\033[48;2;174;137;103m\033[38;2;172;135;101m▄\033[48;2;175;140;106m\033[38;2;180;144;109m▄\033[48;2;167;128;94m\033[38;2;163;123;89m▄\033[48;2;160;122;89m\033[38;2;156;119;87m▄\033[48;2;156;118;85m\033[38;2;156;119;87m▄\033[48;2;142;103;71m\033[38;2;140;102;71m▄\033[48;2;124;86;58m\033[38;2;115;78;52m▄\033[48;2;98;64;42m\033[38;2;93;60;40m▄\033[48;2;88;57;37m\033[38;2;89;59;39m▄\033[48;2;88;59;40m\033[38;2;92;63;43m▄\033[48;2;91;62;42m\033[38;2;94;64;44m▄\033[48;2;90;62;42m\033[38;2;96;67;46m▄\033[48;2;79;53;37m\033[38;2;86;59;41m▄\033[48;2;48;32;21m\033[38;2;63;42;30m▄\033[48;2;8;5;3m\033[38;2;35;22;16m▄\033[48;2;0;0;0m\033[38;2;11;6;4m▄\033[48;2;0;0;0m\033[38;2;1;0;0m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;2;1;1m▄\033[0m"\
                        "\033[0m\033[u\033[B\033[s\033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;2;2;2m \033[0m\033[48;2;49;47;44m \033[0m\033[48;2;147;133;120m \033[0m\033[48;2;192;164;138m \033[0m\033[48;2;197;158;122m \033[0m\033[48;2;203;174;150m \033[0m\033[48;2;213;200;190m \033[0m\033[48;2;223;214;206m \033[0m\033[48;2;226;217;209m \033[0m\033[48;2;220;207;197m \033[0m\033[48;2;217;203;189m \033[0m\033[48;2;222;207;192m \033[0m\033[48;2;225;207;190m \033[0m\033[48;2;222;203;185m \033[0m\033[48;2;220;199;179m \033[0m\033[48;2;217;190;167m \033[0m\033[48;2;213;189;168m \033[0m\033[48;2;217;198;180m \033[0m\033[48;2;216;198;180m \033[0m\033[48;2;216;198;181m \033[0m\033[48;2;217;202;186m \033[0m\033[48;2;230;219;204m \033[0m\033[48;2;233;222;205m \033[0m\033[48;2;234;219;199m \033[0m\033[48;2;231;214;194m \033[0m\033[48;2;233;222;205m \033[0m\033[48;2;229;220;206m \033[0m\033[48;2;224;215;201m \033[0m\033[48;2;217;208;194m \033[0m\033[48;2;210;200;184m \033[0m\033[48;2;202;189;172m \033[0m\033[48;2;182;162;141m \033[0m\033[48;2;163;134;108m \033[0m\033[48;2;152;111;81m \033[0m\033[48;2;147;98;63m \033[0m\033[48;2;156;109;72m \033[0m\033[48;2;164;120;83m \033[0m\033[48;2;167;127;94m \033[0m\033[48;2;175;139;108m \033[0m\033[48;2;182;145;113m \033[0m\033[48;2;194;156;119m \033[0m\033[48;2;184;142;103m \033[0m\033[48;2;176;136;100m \033[0m\033[48;2;171;130;95m \033[0m\033[48;2;174;134;98m \033[0m\033[48;2;172;131;96m \033[0m\033[48;2;164;124;89m \033[0m\033[48;2;172;134;99m \033[0m\033[48;2;160;118;85m \033[0m\033[48;2;146;109;78m \033[0m\033[48;2;145;108;76m \033[0m\033[48;2;128;90;62m \033[0m\033[48;2;105;70;48m \033[0m\033[48;2;96;64;44m \033[0m\033[48;2;96;65;45m \033[0m\033[48;2;96;65;44m \033[0m\033[48;2;101;70;48m \033[0m\033[48;2;103;72;51m \033[0m\033[48;2;92;63;44m \033[0m\033[48;2;68;45;34m \033[0m\033[48;2;55;38;28m \033[0m\033[48;2;41;27;19m \033[0m\033[48;2;26;16;11m \033[0m\033[48;2;13;8;6m \033[0m\033[48;2;12;10;8m \033[0m\033[48;2;32;28;24m \033[0m"\
                        "").nop();

                    auto clr = 0xFFFFFFFF;
                    text wiki00 = ansi::wrp(wrap::on).jet(bias::left).fgc(clr).add("ANSI escape code\n\n")

                        .nil().add("From Wikipedia, the free encyclopedia\n"
                                "  (Redirected from ANSI CSI)\n\n")

                        .jet(bias::center).itc(true).add("\"ANSI code\" redirects here.\n"
                                                        "For other uses, see ANSI (disambiguation).\n\n")

                        .jet(bias::left).itc(faux).fgc(clr).add("ANSI escape sequences").nil()
                        .add(" are a standard for ").fgc(clr).add("in-band signaling").nil()
                        .add(" to control the cursor location, color, and other options on video ")
                        .fgc(clr).add("text terminals").nil().add(" and ")
                        .fgc(clr).add("terminal emulators").nil().add(". Certain sequences of ")
                        .fgc(clr).add("bytes").nil().add(", most starting with ")
                        .fgc(clr).add("Esc").nil().add(" and '[', are embedded into the text, "
                        "which the terminal looks for and interprets as commands, not as ")
                        .fgc(clr).add("character codes").nil().add(".\n");

                    text wiki01 = ansi::wrp(wrap::on).jet(bias::left).add("\n\n\n"
                        "ANSI sequences were introduced in the 1970s to replace vendor-specific sequences "
                        "and became widespread in the computer equipment market by the early 1980s. "
                        "They were used in development, scientific and commercial applications and later by "
                        "the nascent ").fgc(clr).add("bulletin board systems").nil()
                        .add(" to offer improved displays compared to earlier systems lacking cursor movement, "
                        "a primary reason they became a standard adopted by all manufacturers.\n\n"

                        "Although hardware text terminals have become increasingly rare in the 21st century, "
                        "the relevance of the ANSI standard persists because most terminal emulators interpret "
                        "at least some of the ANSI escape sequences in output text. A notable exception was ")
                        .fgc(clr).add("DOS").nil().add(" and older versions of the ")
                        .fgc(clr).add("Win32 console").nil().add(" of ")
                        .fgc(clr).add("Microsoft Windows").nil().add(".\n");

                #pragma endregion

                    text truecolor;
                    truecolor += wiki00;
                    truecolor += r_grut00;
                    truecolor += r_grut01;
                    truecolor += r_grut02;
                    truecolor += r_grut03;
                    truecolor += wiki01;

                    auto window = ui::cake::ctor();
                    window->template plugin<pro::focus>()
                          ->template plugin<pro::track>()
                          ->template plugin<pro::acryl>()
                          ->template plugin<pro::cache>();
                    auto object = window->attach(ui::fork::ctor(axis::Y))
                                        ->colors(whitelt, 0xA01f0fc4);
                        auto menu = object->attach(slot::_1, app::shared::custom_menu(true, {}));
                        auto test_stat_area = object->attach(slot::_2, ui::fork::ctor(axis::Y));
                            auto layers = test_stat_area->attach(slot::_1, ui::cake::ctor());
                                auto scroll = layers->attach(ui::rail::ctor())
                                                    ->config(true, true)
                                                    ->colors(whitelt, reddk);
                                            scroll->attach(ui::post::ctor())
                                                  ->upload(truecolor);
                                auto scroll_bars = layers->attach(ui::fork::ctor());
                                    auto vt = scroll_bars->attach(slot::_2, ui::grip<axis::Y>::ctor(scroll));
                                    auto hz = test_stat_area->attach(slot::_2, ui::grip<axis::X>::ctor(scroll));
                    return window;
                    break;
                }
                case app::shared::objs::Empty:
                {
                    auto window = ui::cake::ctor();
                    window->template plugin<pro::focus>()
                          ->template plugin<pro::track>()
                          ->template plugin<pro::acryl>()
                          ->invoke([&](auto& boss)
                          {
                              boss.SUBMIT(tier::release, e2::form::upon::vtree::attached, parent)
                              {
                                    static iota i = 0; i++;
                                    auto title = ansi::mgl(1).mgr(1).add("Empty Instance \nid: ", parent->id);
                                    boss.base::template riseup<tier::preview>(e2::form::prop::header, title);
                              };
                          });
                    auto object = window->attach(ui::mock::ctor())
                                        ->colors(0,0); //todo mouse tracking
                    return window;
                    break;
                }
                case app::shared::objs::Shop:
                {
                    text monotty_logo  = ansi::bgc(blackdk  ).add("▀▄");
                    text textancy_logo = ansi::bgc(cyandk   ).add("▀▄");
                    text cellatix_logo = ansi::bgc(greendk  ).add("▀▄");
                    text informio_logo = ansi::bgc(magentadk).add("▀▄");
                    text ansiplex_logo = ansi::bgc(reddk    ).add("▀▄");
                    text unicodex_logo = ansi::bgc(yellowdk ).add("▀▄");
                    text appstore_logo = ansi::bgc(blacklt  ).add("▀▄");

                    text line = ansi::wrp(wrap::off).add("──────────────────────────────────────────────────────────────────────────────────────").wrp(wrap::on).eol();
                    auto item = [](auto app, auto clr, auto rating, auto price, auto buy, auto desc)
                    {
                        auto clr_light = rgba{ clr };
                        clr_light.mix(0xa7ffffff);

                        text lot = ansi::nil()
                            .jet(bias::left)
                            .mgl(2).mgr(1).eol()
                            .fgc().jet(bias::left).wrp(wrap::off)
                            .bgc(clr).fgc(0xFFffffff).add("▀▄ ")
                            .fgc(0xFFffffff).add(app, ' ').nil().eol()
                            .fgc(yellowlt).add("   ★★★★").fgc(cyandk).add("★  ").fgc(yellowlt).add(rating)

                            .chx(0).jet(bias::right)
                            .fgc(yellowlt).bgc().add("   ", price, "  ")
                            .nil().eol().eol()
                            .fgc(bluedk).bgc(whitelt).add(' ', buy, ' ').nil().add("  ")

                            .fgc().chx(0).jet(bias::left)
                            .mgr(11).wrp(wrap::on)
                            .add(desc, "\n\n")
                            .nil();

                        return lot;
                    };

                    text appstore_head =
                    ansi::nil().eol().mgl(2).mgr(2)
                    .bld(faux).fgc(whitelt).jet(bias::left).wrp(wrap::on).add(
                    "A digital distribution platform, developed "
                    "and maintained by NetXS Group, for TUI/terminal "
                    "apps on its desktop environment. "
                    "The store allows users to browse and download "
                    "apps developed with Desktopio software "
                    "development kit.\n");

                    text textancy_text = ansi::nil().add(
                    "Hello World!😎\n"
                    "絵文字:\n"
                    "English: /ɪˈmoʊdʒiː/;\n"
                    "Japanese: [emodʑi];\n");

                    std::list<text> appstore_body =
                    {
                        item("Term", blackdk, "469", "Free ", "Get",
                        "Terminal emulator."),

                        item("Tile", bluedk, "3", "Free ", "Get",
                        ansi::add("Meta object. Tiling window manager preconfigurable "
                        "using environment variable ").
                        fgc(whitelt).bld(true).add("VTM_TILE").fgc().bld(faux).
                        add(".\n\nConfiguration example:\n\n").
                        mgl(2).fgc(whitelt).bgc(blacklt)
                        .add(" VTM_PROFILE_1='\"Menu label 1\", \"Window Title 1\", h1:2( v1:1(\"bash -c htop\", \"bash -c mc\"), \"bash\")' \n"
                            " VTM_PROFILE_2='\"Menu label 2\", \"Window Title 2\", h( v(\"bash -c htop\", \"bash -c mc\"), a(\"Calc\",\"\",\"\"))' ")),

                        item("Text", cyandk, "102", "Free ", "Get",
                        "A simple text editor for Monotty environment "
                        "and a basic editing tool which enables "
                        "desktop users to create documents that "
                        "contain ANSI-formatted text."),

                        item("Calc", greendk, "30", "Free ", "Get",
                        "A simple spreadsheet calculator application."),

                        item("Task", magentadk, "311", "Free ", "Get",
                        "A task manager program that displays "
                        "information about CPU, memory utilization, "
                        "and current I/O usage."),

                        item("Draw", reddk, "64", "Free ", "Get",
                        "A simple program which enables desktop "
                        "users to create sophisticated ANSI-artworks."),

                        item("Char", yellowdk, "161", "Free ", "Get",
                        "An utility that allows browsing all Unicode "
                        "codepoints and inspecting their metadata."),

                        item(ansi::fgc(0xff0000).add("File"), cyanlt, "4", "Free ", "Get",
                        "An orthodox file manager for Monotty environment."),

                        item("Time", bluedk, "4", "Free ", "Get",
                        "A calendar application made by NetXS Group for Monotty environment."),

                        item("Goto", bluedk, "4", "Free ", "Get",
                        "Internet/SSH browser."),

                        item(ansi::fgc(0xFF00FFFF).add("Doom").fgc(), reddk, "4", "Free ", "Get",
                        "Doom II source port."),

                        item("Logs", blackdk, "4096", "Free ", "Get",
                        "Application for displaying debug trace. "
                        "This is more efficient than writing to STDOUT."),

                        item("Clip", bluedk, "1", "Free ", "Get",
                        "Clipboard manager."),

                        item("Info", cyandk, "1", "Free ", "Get",
                        "Software documentation browser."),

                        item("Hood", reddk, "1", "Free ", "Get",
                        "Desktop environment settings configurator."),

                        item("View", cyandk, "1", "Free ", "Get",
                        "Meta object. Desktop location marker."),
                    };

                    text qr = ansi::esc(
                    "\033[107m                                 \n"
                    "  \033[40m \033[97m▄▄▄▄▄ \033[107m \033[30m▄\033[40;97m▄\033[107m \033[30m▄\033[40m \033[107m  \033[40m \033[97m▄\033[107;30m▄\033[40;97m▄▄\033[107m  \033[40m ▄▄▄▄▄ \033[107m  \n"
                    "  \033[40m \033[107m \033[40m   \033[107m \033[40m \033[107m \033[40m▄   ▄\033[107m \033[40m \033[107;30m▄ \033[40m \033[107m▄\033[40;97m▄\033[107m  \033[40m \033[107m \033[40m   \033[107m \033[40m \033[107m  \n"
                    "  \033[40m \033[107m \033[40m▄▄▄\033[107m \033[40m \033[107m \033[40m \033[107;30m▄ ▄ \033[40m \033[97m▄  \033[107m \033[40m \033[107;30m▄  \033[40m \033[107m \033[40;97m▄▄▄\033[107m \033[40m \033[107m  \n"
                    "  \033[40m▄▄▄▄▄▄▄\033[107m \033[40m \033[107;30m▄\033[40;97m▄\033[107m \033[40m \033[107m \033[40m▄\033[107m \033[40m▄\033[107;30m▄\033[40;97m▄\033[107m \033[40m▄\033[107m \033[40m▄▄▄▄▄▄▄\033[107m  \n"
                    "  \033[40m▄▄\033[107m \033[40m▄\033[107m  \033[40m▄▄\033[107m  \033[40m \033[107;30m▄ \033[40m \033[97m▄\033[107m \033[30m▄ ▄\033[40;97m▄\033[107;30m▄ \033[40m  \033[97m▄\033[107;30m▄\033[40;97m▄ \033[107;30m▄  \n"
                    "  ▄\033[40;97m▄▄\033[107;30m▄▄ \033[40;97m▄\033[107;30m▄\033[40m \033[97m▄▄\033[107m  \033[40m ▄\033[107;30m▄\033[40;97m▄\033[107m \033[40m ▄\033[107;30m▄\033[40m \033[107m \033[40m \033[97m▄\033[107m \033[30m▄\033[40m \033[107m   \n"
                    "  \033[40;97m▄\033[107m   \033[30m▄▄\033[40;97m▄\033[107m \033[40m \033[107m \033[30m▄▄\033[40m \033[107m ▄   ▄\033[40;97m▄  ▄▄\033[107m \033[30m▄▄\033[40;97m▄▄\033[107m  \n"
                    "   \033[40m▄ \033[107;30m▄▄\033[40;97m▄▄\033[107m  \033[30m▄\033[40;97m▄\033[107m \033[40m \033[107;30m▄\033[40;97m▄\033[107m \033[30m▄▄\033[40;97m▄\033[107;30m▄▄▄▄ \033[40;97m▄▄▄▄▄\033[107m  \n"
                    "     \033[40m▄\033[107;30m▄ \033[40;97m▄ ▄\033[107m \033[40m▄▄\033[107m \033[40m \033[107m \033[40m▄\033[107;30m▄\033[40m \033[107m ▄\033[40;97m▄ \033[107;30m▄\033[40m \033[107m \033[40m \033[107m ▄\033[40m \033[107m  \n"
                    "  \033[40;97m▄\033[107m \033[40m \033[107;30m▄\033[40m \033[97m▄▄\033[107;30m▄▄\033[40m \033[107m▄▄ \033[40;97m▄\033[107m \033[30m▄ ▄\033[40m \033[107m ▄ \033[40;97m▄▄▄ ▄ ▄\033[107m  \n"
                    "  \033[40m▄\033[107m \033[40m▄▄▄\033[107m \033[40m▄▄\033[107;30m▄\033[40m    \033[107m \033[40m \033[107m▄\033[40m \033[107m▄  \033[40m \033[97m▄▄▄ \033[107m \033[40m \033[107;30m▄\033[40m \033[107m  \n"
                    "  \033[40m \033[97m▄▄▄▄▄ \033[107m \033[40m▄    \033[107;30m▄\033[40m \033[107m ▄\033[40;97m▄\033[107;30m▄▄\033[40m \033[107m \033[40;97m▄\033[107m \033[40m  \033[107;30m▄\033[40m \033[107m▄  \n"
                    "  \033[40m \033[107m \033[40m   \033[107m \033[40m \033[107m ▄ ▄\033[40m \033[107m ▄\033[40;97m▄▄▄\033[107m \033[30m▄▄\033[40;97m▄▄  ▄▄\033[107;30m▄▄   \n"
                    "  \033[40m \033[107m \033[40;97m▄▄▄\033[107m \033[40m \033[107m \033[30m▄\033[40;97m▄\033[107m \033[40m \033[107m \033[40m▄▄\033[107m \033[40m▄\033[107;30m▄\033[40;97m▄\033[107m \033[30m▄\033[40m \033[97m▄\033[107m \033[40m▄\033[107;30m▄\033[40;97m▄\033[107;30m▄\033[40;97m▄\033[107m  \n"
                    "  \033[40m▄▄▄▄▄▄▄\033[107m \033[40m▄▄▄\033[107m \033[40m▄▄\033[107m  \033[40m▄▄\033[107m \033[40m▄▄▄\033[107m  \033[40m▄\033[107m  \033[40m▄\033[107m   \n"
                    "\033[30m▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄\n").nil();

                    text desktopio_body = ansi::nil().eol()
                    .mgl(2).mgr(2).wrp(wrap::off)
                    .fgc(bluedk).jet(bias::left)
                    .bgc(bluedk).fgc(0xFFFFFFFF)
                    .add(" Monotty Desktopio "
                    "\n\n")
                    .fgc().bgc().jet(bias::left).wrp(wrap::on).add(
                    "Monotty Desktopio is a cross-platform, full-featured desktop environment."
                    " A user interface where by all output is presented in the form of text.\n"
                    "The first biggest advantage of this desktop environment concept that "
                    "it can be used directly over SSH connections, no additional protocol needed.\n"
                    "The second is the flexible multi-user interface "
                    "that serves several users engaged in collaborative applications and enables "
                    "users to view a collaborating user’s workspace.\n"
                    "\n"
                    "If you like the way we think and would like to support the project "
                    "in the spirit of Bitcoin, you can donate at the following public "
                    "bitcoin address:\n\n")
                    .jet(bias::center).wrp(wrap::off).mgl(1).mgr(1)
                    .add(qr,
                    " bitcoin:1Euu4jcQ15LKijaDyZigZrnEoqwe1daTVZ\n");

                    const static auto c3 = app::shared::c3;
                    const static auto x3 = app::shared::x3;

                    auto window = ui::cake::ctor();
                    window->template plugin<pro::focus>()
                          ->colors(whitelt, 0x60000000)
                          ->template plugin<pro::track>()
                          ->template plugin<pro::acryl>()
                          ->template plugin<pro::cache>();
                    auto object = window->attach(ui::fork::ctor(axis::Y))
                                        ->colors(whitelt, 0);
                        auto menu_object = object->attach(slot::_1, ui::fork::ctor(axis::Y));
                            menu_object->attach(slot::_1, app::shared::custom_menu(true, {}));
                            menu_object->attach(slot::_2, ui::post::ctor())
                                       ->template plugin<pro::limit>(twod{ 37,-1 }, twod{ -1,-1 })
                                       ->upload(appstore_head)
                                       ->active();
                        auto layers = object->attach(slot::_2, ui::cake::ctor());
                            auto scroll = layers->attach(ui::rail::ctor())
                                                ->colors(whitedk, 0xFF0f0f0f)
                                                ->template plugin<pro::limit>(twod{ -1,2 }, twod{ -1,-1 })
                                                ->config(true, true);
                                auto items = scroll->attach(ui::list::ctor());
                                for (auto& body : appstore_body) items->attach(ui::post::ctor())
                                                                      ->upload(body)
                                                                      ->template plugin<pro::grade>()
                                                                      ->template plugin<pro::fader>(x3, c3, 250ms);
                                items->attach(ui::post::ctor())
                                     ->upload(desktopio_body)
                                     ->template plugin<pro::grade>();
                        layers->attach(app::shared::scroll_bars(scroll));
                    return window;
                    break;
                }
                case app::shared::objs::Calc:
                {
                    auto& creator = app::shared::get_creator()["Calc"];
                    return creator(data);
                    break;
                }
                case app::shared::objs::Text:
                {
                    text topic3 = R"( 
Plain text vs. rich text

There are important differences between plain text (created and edited by 
text editors) and rich text (such as that created by word processors or 
desktop publishing software).

Plain text exclusively consists of character representation. Each character 
is represented by a fixed-length sequence of one, two, or four bytes, or as a 
variable-length sequence of one to four bytes, in accordance to specific 
character encoding conventions, such as ASCII, ISO/IEC 2022, UTF-8, or 
Unicode. These conventions define many printable characters, but also 
non-printing characters that control the flow of the text, such as space, 
line break, and page break. Plain text contains no other information about 
the text itself, not even the character encoding convention employed. Plain 
text is stored in text files, although text files do not exclusively store 
plain text. In the early days of computers, plain text was displayed using a 
monospace font, such that horizontal alignment and columnar formatting were 
sometimes done using whitespace characters. For compatibility reasons, this 
tradition has not changed.

Rich text, on the other hand, may contain metadata, character formatting data 
(e.g. typeface, size, weight and style), paragraph formatting data (e.g. 
indentation, alignment, letter and word distribution, and space between lines 
or other paragraphs), and page specification data (e.g. size, margin and 
reading direction). Rich text can be very complex. Rich text can be saved in 
binary format (e.g. DOC), text files adhering to a markup language (e.g. RTF 
or HTML), or in a hybrid form of both (e.g. Office Open XML).

Text editors are intended to open and save text files containing either plain 
text or anything that can be interpreted as plain text, including the markup 
for rich text or the markup for something else (e.g. SVG).

History

Before text editors existed, computer text was punched into cards with 
keypunch machines. Physical boxes of these thin cardboard cards were then 
inserted into a card-reader. Magnetic tape and disk "card-image" files 
created from such card decks often had no line-separation characters at all, 
and assumed fixed-length 80-character records. An alternative to cards was 
punched paper tape. It could be created by some teleprinters (such as the 
Teletype), which used special characters to indicate ends of records.

The first text editors were "line editors" oriented to teleprinter- or 
typewriter-style terminals without displays. Commands (often a single 
keystroke) effected edits to a file at an imaginary insertion point called 
the "cursor". Edits were verified by typing a command to print a small 
section of the file, and periodically by printing the entire file. In some 
line editors, the cursor could be moved by commands that specified the line 
number in the file, text strings (context) for which to search, and 
eventually regular expressions. Line editors were major improvements over 
keypunching. Some line editors could be used by keypunch; editing commands 
could be taken from a deck of cards and applied to a specified file. Some 
common line editors supported a "verify" mode in which change commands 
displayed the altered lines.

When computer terminals with video screens became available, screen-based 
text editors (sometimes called just "screen editors") became common. One of 
the earliest full-screen editors was O26, which was written for the operator 
console of the CDC 6000 series computers in 1967. Another early full-screen 
editor was vi. Written in the 1970s, it is still a standard editor on Unix 
and Linux operating systems. Also written in the 1970s was the UCSD Pascal 
Screen Oriented Editor, which was optimized both for indented source code as 
well as general text. Emacs, one of the first free and open source software 
projects, is another early full-screen or real-time editor, one that was 
ported to many systems. A full-screen editor's ease-of-use and speed 
(compared to the line-based editors) motivated many early purchases of video 
terminals.

The core data structure in a text editor is the one that manages the string 
(sequence of characters) or list of records that represents the current state 
of the file being edited. While the former could be stored in a single long 
consecutive array of characters, the desire for text editors that could more 
quickly insert text, delete text, and undo/redo previous edits led to the 
development of more complicated sequence data structures. A typical text 
editor uses a gap buffer, a linked list of lines (as in PaperClip), a piece 
table, or a rope, as its sequence data structure.

Types of text editors

Some text editors are small and simple, while others offer broad and complex 
functions. For example, Unix and Unix-like operating systems have the pico 
editor (or a variant), but many also include the vi and Emacs editors. 
Microsoft Windows systems come with the simple Notepad, though many 
people—especially programmers—prefer other editors with more features. Under 
Apple Macintosh's classic Mac OS there was the native SimpleText, which was 
replaced in Mac OS X by TextEdit, which combines features of a text editor 
with those typical of a word processor such as rulers, margins and multiple 
font selection. These features are not available simultaneously, but must be 
switched by user command, or through the program automatically determining 
the file type.

Most word processors can read and write files in plain text format, allowing 
them to open files saved from text editors. Saving these files from a word 
processor, however, requires ensuring the file is written in plain text 
format, and that any text encoding or BOM settings won't obscure the file for 
its intended use. Non-WYSIWYG word processors, such as WordStar, are more 
easily pressed into service as text editors, and in fact were commonly used 
as such during the 1980s. The default file format of these word processors 
often resembles a markup language, with the basic format being plain text and 
visual formatting achieved using non-printing control characters or escape 
sequences. Later word processors like Microsoft Word store their files in a 
binary format and are almost never used to edit plain text files.

Some text editors can edit unusually large files such as log files or an 
entire database placed in a single file. Simpler text editors may just read 
files into the computer's main memory. With larger files, this may be a slow 
process, and the entire file may not fit. Some text editors do not let the 
user start editing until this read-in is complete. Editing performance also 
often suffers in nonspecialized editors, with the editor taking seconds or 
even minutes to respond to keystrokes or navigation commands. Specialized 
editors have optimizations such as only storing the visible portion of large 
files in memory, improving editing performance.

Some editors are programmable, meaning, e.g., they can be customized for 
specific uses. With a programmable editor it is easy to automate repetitive 
tasks or, add new functionality or even implement a new application within 
the framework of the editor. One common motive for customizing is to make a 
text editor use the commands of another text editor with which the user is 
more familiar, or to duplicate missing functionality the user has come to 
depend on. Software developers often use editor customizations tailored to 
the programming language or development environment they are working in. The 
programmability of some text editors is limited to enhancing the core editing 
functionality of the program, but Emacs can be extended far beyond editing 
text files—for web browsing, reading email, online chat, managing files or 
playing games and is often thought of as a Lisp execution environment with a 
Text User Interface. Emacs can even be programmed to emulate Vi, its rival in 
the traditional editor wars of Unix culture.

An important group of programmable editors uses REXX as a scripting language. 
These "orthodox editors" contain a "command line" into which commands and 
macros can be typed and text lines into which line commands and macros can be 
typed. Most such editors are derivatives of ISPF/PDF EDIT or of XEDIT, IBM's 
flagship editor for VM/SP through z/VM. Among them are THE, KEDIT, X2, 
Uni-edit, and SEDIT.

A text editor written or customized for a specific use can determine what the 
user is editing and assist the user, often by completing programming terms 
and showing tooltips with relevant documentation. Many text editors for 
software developers include source code syntax highlighting and automatic 
indentation to make programs easier to read and write. Programming editors 
often let the user select the name of an include file, function or variable, 
then jump to its definition. Some also allow for easy navigation back to the 
original section of code by storing the initial cursor location or by 
displaying the requested definition in a popup window or temporary buffer. 
Some editors implement this ability themselves, but often an auxiliary 
utility like ctags is used to locate the definitions.

)";

                    auto window = ui::cake::ctor();
                    window->template plugin<pro::focus>()
                          ->template plugin<pro::track>()
                          ->template plugin<pro::acryl>()
                          ->template plugin<pro::cache>()
                          ->invoke([&](auto& boss)
                          {
                              boss.keybd.accept(true);
                              boss.SUBMIT(tier::release, e2::form::upon::vtree::attached, parent)
                              {
                                    static iota i = 0; i++;
                                    auto title = ansi::jet(bias::center).add("Text Editor\n ~/Untitled ", i, ".txt");
                                    boss.base::template riseup<tier::preview>(e2::form::prop::header, title);
                              };
                          });
                    auto object = window->attach(ui::fork::ctor(axis::Y))
                                        ->colors(whitelt, 0xA05f1a00);
                        auto menu = object->attach(slot::_1, app::shared::main_menu());
                        auto body_area = object->attach(slot::_2, ui::fork::ctor(axis::Y));
                            auto fields = body_area->attach(slot::_1, ui::pads::ctor(dent{ 1,1 }));
                                auto layers = fields->attach(ui::cake::ctor());
                                    auto scroll = layers->attach(ui::rail::ctor())
                                                        ->template plugin<pro::limit>(twod{ 4,3 }, twod{ -1,-1 });
                                        auto edit_box = scroll->attach(ui::post::ctor(true))
                                                              ->template plugin<pro::caret>(true, twod{ 25,1 }, true)
                                                              ->colors(blackdk, whitelt)
                                                              ->upload(ansi::wrp(wrap::off).mgl(1)
                                                                .add(topic3)
                                                                .fgc(app::shared::highlight_color)
                                                                .add("From Wikipedia, the free encyclopedia"));
                            auto status_line = body_area->attach(slot::_2, ui::post::ctor())
                                                        ->template plugin<pro::limit>(twod{ 1,1 }, twod{ -1,1 })
                                                        ->upload(ansi::wrp(wrap::off).mgl(1).mgr(1).jet(bias::right).fgc(whitedk)
                                                           .add("INS  Sel: 0:0  Col: 26  Ln: 2/148").nil());
                                layers->attach(app::shared::scroll_bars(scroll));
                    return window;
                    break;
                }
                case app::shared::objs::vtm:
                {
                    auto window = ui::cake::ctor();
                    window->template plugin<pro::focus>()
                          ->template plugin<pro::track>()
                          ->template plugin<pro::acryl>()
                          ->template plugin<pro::cache>();
                    auto object = window->attach(ui::fork::ctor(axis::Y))
                                        ->colors(whitelt, app::shared::term_menu_bg);
                        auto menu = object->attach(slot::_1, app::shared::custom_menu(faux, {}));
                        auto layers = object->attach(slot::_2, ui::cake::ctor())
                                            ->template plugin<pro::limit>(dot_11, twod{ 400,200 });
                            auto scroll = layers->attach(ui::rail::ctor());
                            if (app::shared::vtm_count < app::shared::max_vtm)
                            {
                                auto c = &app::shared::vtm_count; (*c)++;
                                scroll->attach(app::term::ctor("vtm"))
                                      ->colors(whitelt, blackdk)
                                      ->SUBMIT_BYVAL(tier::release, e2::dtor, item_id)
                                        {
                                            (*c)--;
                                            log("main: vtm recursive conn destoyed");
                                        };
                            }
                            else
                            {
                                scroll->attach(ui::post::ctor())
                                      ->colors(whitelt, blackdk)
                                      ->upload(ansi::fgc(yellowlt).mgl(4).mgr(4).wrp(wrap::off)
                                        .add("\n\nconnection rejected\n\n")
                                        .nil().wrp(wrap::on)
                                        .add("Reached the limit of recursive connections, destroy existing recursive instances to create new ones."));
                            }
                        layers->attach(app::shared::scroll_bars(scroll));
                    return window;
                    break;
                }
                case app::shared::objs::Far:
                {
                    auto window = ui::cake::ctor();
                    window->template plugin<pro::focus>()
                          ->template plugin<pro::track>()
                          ->template plugin<pro::acryl>()
                          ->template plugin<pro::cache>();
                    auto object = window->attach(ui::fork::ctor(axis::Y))
                                        ->colors(whitelt, app::shared::term_menu_bg);
                        auto menu = object->attach(slot::_1, app::shared::custom_menu(true, {}));
                        auto layers = object->attach(slot::_2, ui::cake::ctor())
                                            ->template plugin<pro::limit>(dot_11, twod{ 400,200 });
                            auto scroll = layers->attach(ui::rail::ctor());
                            scroll->attach(app::term::ctor("far"))
                                  ->colors(whitelt, blackdk);
                        layers->attach(app::shared::scroll_bars_term(scroll));
                    return window;
                    break;
                }
                case app::shared::objs::MC:
                {
                    auto window = ui::cake::ctor();
                    window->template plugin<pro::focus>()
                          ->template plugin<pro::track>()
                          ->template plugin<pro::acryl>()
                          ->template plugin<pro::cache>();
                    auto object = window->attach(ui::fork::ctor(axis::Y))
                                        ->colors(whitelt, app::shared::term_menu_bg);
                        auto menu = object->attach(slot::_1, app::shared::custom_menu(faux, {}));
                        auto layers = object->attach(slot::_2, ui::cake::ctor())
                                            ->template plugin<pro::limit>(dot_11, twod{ 400,200 });
                            auto scroll = layers->attach(ui::rail::ctor())
                                                ->template plugin<pro::limit>(twod{ 10,1 }); // mc crashes when window is too small
                            // -c -- force color support
                            // -x -- force xtrem functionality

                            #if defined(_WIN32)

                                auto inst = scroll->attach(app::term::ctor("wsl mc"));

                            #elif defined(__linux__)
                                #ifndef PROD
                                    auto inst = scroll->attach(app::term::ctor("bash -c 'LC_ALL=en_US.UTF-8 mc -c -x -d'"));
                                #else
                                    auto inst = scroll->attach(app::term::ctor("bash -c 'LC_ALL=en_US.UTF-8 mc -c -x'"));
                                #endif
                            #elif defined(__APPLE__)

                                auto inst = scroll->attach(app::term::ctor("zsh -c 'LC_ALL=en_US.UTF-8 mc -c -x'"));

                            #elif defined(__FreeBSD__)

                                auto inst = scroll->attach(app::term::ctor("csh -c 'LC_ALL=en_US.UTF-8 mc -c -x'"));

                            #elif defined(__unix__)

                                auto inst = scroll->attach(app::term::ctor("sh -c 'LC_ALL=en_US.UTF-8 mc -c -x'"));

                            #endif

                            inst->colors(whitelt, blackdk);
                        layers->attach(app::shared::scroll_bars(scroll));
                    return window;
                    break;
                }
                case app::shared::objs::Bash:
                case app::shared::objs::Term:
                {
                    auto window = ui::cake::ctor();
                    window->template plugin<pro::focus>()
                          ->template plugin<pro::track>()
                          ->template plugin<pro::acryl>()
                          ->template plugin<pro::cache>();
                    auto object = window->attach(ui::fork::ctor(axis::Y))
                                        ->colors(whitelt, app::shared::term_menu_bg);
                        auto menu = object->attach(slot::_1, app::shared::terminal_menu(true));
                        auto term_stat_area = object->attach(slot::_2, ui::fork::ctor(axis::Y));
                            auto layers = term_stat_area->attach(slot::_1, ui::cake::ctor())
                                                        ->template plugin<pro::limit>(dot_11, twod{ 400,200 });
                                auto scroll = layers->attach(ui::rail::ctor());
                                {
                                    #ifdef DEMO
                                        scroll->template plugin<pro::limit>(twod{ 20,1 }); // mc crashes when window is too small
                                    #endif

                                    #if defined(_WIN32)

                                        auto inst = scroll->attach(app::term::ctor("bash -i"));

                                    #elif defined(__linux__)

                                        auto inst = scroll->attach(app::term::ctor("bash -i"));

                                    #elif defined(__APPLE__)

                                        auto inst = scroll->attach(app::term::ctor("zsh"));

                                    #elif defined(__FreeBSD__)

                                        auto inst = scroll->attach(app::term::ctor("csh"));

                                    #elif defined(__unix__)

                                        auto inst = scroll->attach(app::term::ctor("sh"));

                                    #endif

                                    inst->colors(whitelt, blackdk);
                                }
                            auto scroll_bars = layers->attach(ui::fork::ctor());
                                auto vt = scroll_bars->attach(slot::_2, ui::grip<axis::Y>::ctor(scroll));
                                auto hz = term_stat_area->attach(slot::_2, ui::grip<axis::X>::ctor(scroll));
                    return window;
                    break;
                }
                case app::shared::objs::PowerShell:
                {
                    auto window = ui::cake::ctor();
                    window->template plugin<pro::focus>()
                          ->template plugin<pro::track>()
                          ->template plugin<pro::acryl>()
                          ->template plugin<pro::cache>();
                    auto object = window->attach(ui::fork::ctor(axis::Y))
                                        ->colors(whitelt, app::shared::term_menu_bg);
                        auto menu = object->attach(slot::_1, app::shared::custom_menu(true,
                            std::list{
                                    std::pair<text, std::function<void(ui::pads&)>>{ ansi::esc("C").und(true).add("l").nil().add("ear"),
                                    [](ui::pads& boss)
                                    {
                                        boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                                        {
                                            boss.base::broadcast->SIGNAL(tier::preview, app::term::events::cmd, app::term::commands::ui::clear);
                                            gear.dismiss(true);
                                        };
                                    }},
                                    std::pair<text, std::function<void(ui::pads&)>>{ ansi::esc("R").und(true).add("e").nil().add("set"),
                                    [](ui::pads& boss)
                                    {
                                        boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                                        {
                                            boss.base::broadcast->SIGNAL(tier::preview, app::term::events::cmd, app::term::commands::ui::reset);
                                            gear.dismiss(true);
                                        };
                                    }},
                                }));
                        auto term_stat_area = object->attach(slot::_2, ui::fork::ctor(axis::Y));
                            auto layers = term_stat_area->attach(slot::_1, ui::cake::ctor())
                                                ->template plugin<pro::limit>(dot_11, twod{ 400,200 });
                                auto scroll = layers->attach(ui::rail::ctor())
                                                    ->colors(whitelt, 0xFF560000);
                                    scroll->attach(app::term::ctor("powershell"))
                                          ->colors(whitelt, 0xFF562401);
                            auto scroll_bars = layers->attach(ui::fork::ctor());
                                auto vt = scroll_bars->attach(slot::_2, ui::grip<axis::Y>::ctor(scroll));
                                auto hz = term_stat_area->attach(slot::_2, ui::grip<axis::X>::ctor(scroll));
                    return window;
                    break;
                }
                case app::shared::objs::CommandPrompt:
                {
                    auto window = ui::cake::ctor();
                    window->template plugin<pro::focus>()
                          ->template plugin<pro::track>()
                          ->template plugin<pro::acryl>()
                          ->template plugin<pro::cache>();
                    auto object = window->attach(ui::fork::ctor(axis::Y))
                                        ->colors(whitelt, app::shared::term_menu_bg);
                        auto menu = object->attach(slot::_1, app::shared::custom_menu(true,
                            std::list{
                                    std::pair<text, std::function<void(ui::pads&)>>{ ansi::esc("C").und(true).add("l").nil().add("ear"),
                                    [](ui::pads& boss)
                                    {
                                        boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                                        {
                                            boss.base::broadcast->SIGNAL(tier::preview, app::term::events::cmd, app::term::commands::ui::clear);
                                            gear.dismiss(true);
                                        };
                                    }},
                                    std::pair<text, std::function<void(ui::pads&)>>{ ansi::esc("R").und(true).add("e").nil().add("set"),
                                    [](ui::pads& boss)
                                    {
                                        boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                                        {
                                            boss.base::broadcast->SIGNAL(tier::preview, app::term::events::cmd, app::term::commands::ui::reset);
                                            gear.dismiss(true);
                                        };
                                    }},
                                }));
                        auto term_stat_area = object->attach(slot::_2, ui::fork::ctor(axis::Y));
                            auto layers = term_stat_area->attach(slot::_1, ui::cake::ctor())
                                                ->template plugin<pro::limit>(dot_11, twod{ 400,200 });
                                auto scroll = layers->attach(ui::rail::ctor());
                        #ifdef DEMO
                            scroll->template plugin<pro::limit>(twod{ 20,1 }); // mc crashes when window is too small
                        #endif

                            #if defined(_WIN32)
                                auto inst = scroll->attach(app::term::ctor("cmd"));
                            #elif defined(__linux__)
                                auto inst = scroll->attach(app::term::ctor("bash -i"));
                            #elif defined(__APPLE__)
                                auto inst = scroll->attach(app::term::ctor("zsh"));
                            #elif defined(__FreeBSD__)
                                auto inst = scroll->attach(app::term::ctor("csh"));
                            #elif defined(__unix__)
                                auto inst = scroll->attach(app::term::ctor("sh"));
                            #endif

                                inst->colors(whitelt, blackdk);

                        auto scroll_bars = layers->attach(ui::fork::ctor());
                            auto vt = scroll_bars->attach(slot::_2, ui::grip<axis::Y>::ctor(scroll));
                            auto hz = term_stat_area->attach(slot::_2, ui::grip<axis::X>::ctor(scroll));
                    return window;
                    break;
                }
                case app::shared::objs::Logs:
                {
                    const static auto x3 = app::shared::x3;

                    auto window = ui::cake::ctor();
                    window->template plugin<pro::focus>()
                          ->template plugin<pro::track>()
                          ->template plugin<pro::acryl>()
                          ->template plugin<pro::cache>()
                          ->invoke([&](auto& boss)
                          {
                              boss.keybd.accept(true);
                          });
                    auto object = window->attach(ui::fork::ctor(axis::Y))
                                        ->colors(whitelt, app::shared::term_menu_bg);
                        auto menu = object->attach(slot::_1, app::shared::custom_menu(true,
                            std::list{
                                    std::pair<text, std::function<void(ui::pads&)>>{ "Codepoints",
                                    [](ui::pads& boss)
                                    {
                                        boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                                        {
                                            iota status = 1;
                                            boss.base::broadcast->SIGNAL(tier::request, e2::command::custom, status);
                                            boss.base::broadcast->SIGNAL(tier::preview, e2::command::custom, status == 2 ? 1/*show*/ : 2/*hide*/);
                                            gear.dismiss(true);
                                        };
                                        boss.base::broadcast->SUBMIT(tier::release, e2::command::custom, status)
                                        {
                                            //todo unify, get boss base colors, don't use x3
                                            boss.color(status == 1 ? 0xFF00ff00 : x3.fgc(), x3.bgc());
                                        };
                                    }},
                                    std::pair<text, std::function<void(ui::pads&)>>{ "Clear",
                                    [](ui::pads& boss)
                                    {
                                        boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                                        {
                                            boss.base::broadcast->SIGNAL(tier::preview, e2::command::custom, 0);
                                            gear.dismiss(true);
                                        };
                                    }},
                                }));
                        auto layers = object->attach(slot::_2, ui::cake::ctor());
                            auto scroll = layers->attach(ui::rail::ctor());
                            #ifndef PROD
                            scroll->attach(ui::post::ctor())
                                  ->colors(whitelt, blackdk)
                                  ->upload(ansi::fgc(yellowlt).mgl(4).mgr(4).wrp(wrap::off)
                                    + "\n\nLogs is not availabe in DEMO mode\n\n"
                                    + ansi::nil().wrp(wrap::on)
                                    + "Use the full version of vtm to run Logs.");
                            #else
                            scroll->attach(base::create<post_logs>())
                                  ->colors(whitelt, blackdk);
                            #endif
                        layers->attach(app::shared::scroll_bars(scroll));
                    return window;
                    break;
                }
                case app::shared::objs::View:
                {
                    auto window = ui::cake::ctor();
                    window->invoke([&](auto& boss)
                          {
                              //todo reimplement (tiling/window)
                              //boss.SUBMIT(tier::release, hids::events::mouse::button::dblclick::left, gear)
                              //{
                              //    auto outer = decltype(e2::config::plugins::sizer::outer)::type{};
                              //    boss.base::template riseup<tier::request>(e2::config::plugins::sizer::outer, outer);
                              //    auto actual_rect = rect{ dot_00, boss.base::size() } + outer;
                              //    if (actual_rect.hittest(gear.coord))
                              //    {
                              //        if (auto gate_ptr = bell::getref(gear.id))
                              //        {
                              //            rect viewport;
                              //            gate_ptr->SIGNAL(tier::request, e2::form::prop::viewport, viewport);
                              //            boss.base::extend(viewport);
                              //        }
                              //        gear.dismiss();
                              //    }
                              //};
                              boss.SUBMIT(tier::release, e2::render::prerender, parent_canvas)
                              {
                                  rgba title_fg_color = 0xFFffffff;
                                  auto area = parent_canvas.full();
                                  auto mark = skin::color(tone::shadower);
                                  mark.fgc(title_fg_color).link(boss.bell::id);
                                  auto fill = [&](cell& c) { c.fusefull(mark); };
                                  parent_canvas.cage(area, dot_21, fill);
                              };
                              boss.SUBMIT(tier::release, e2::form::upon::vtree::attached, parent)
                              {
                                  static iota i = 0; i++;
                                  auto title = ansi::jet(bias::center).add("View \n Region ", i);
                                  boss.base::template riseup<tier::preview>(e2::form::prop::header, title);
                                  
                                  auto outer = dent{ 2,2,1,1 };
                                  auto inner = dent{ -4,-4,-2,-2 };
                                  boss.base::template riseup<tier::release>(e2::config::plugins::sizer::outer, outer);
                                  boss.base::template riseup<tier::release>(e2::config::plugins::sizer::inner, inner);
                                  boss.base::template riseup<tier::release>(e2::config::plugins::align, faux);
                                  boss.base::template riseup<tier::preview>(e2::form::prop::zorder, Z_order::backmost);
                              };
                          });
                    return window;
                    break;
                }
                case app::shared::objs::Tile:
                {
                    auto& creator = app::shared::get_creator()["Tile"];
                    return creator(data);

                    //return app::build(create_app, data);
                    //return build(create_app, data);
                    break;
                }
            }
        };
        //app::init_apps();
        world->SUBMIT(tier::release, e2::form::proceed::createat, what)
        {
            auto menu_item_id = what.menu_item_id;
            auto location = what.location;

            assert(menu_item_id < app::shared::objs_config.size());

            auto config = app::shared::objs_config[menu_item_id];
            sptr<ui::cake> window = ui::cake::ctor()
                ->plugin<pro::title>(config.title)
                ->plugin<pro::limit>(dot_11, twod{ 400,200 }) //todo unify, set via config
                ->plugin<pro::sizer>()
                ->plugin<pro::frame>()
                ->plugin<pro::light>()
                ->plugin<pro::align>()
                ->invoke([&](ui::cake& boss)
                {
                    boss.SUBMIT(tier::release, hids::events::mouse::button::dblclick::left, gear)
                    {
                        boss.base::template riseup<tier::release>(e2::form::maximize, gear);
                        gear.dismiss();
                    };
                    boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                    {
                        auto& area = boss.base::area();
                        if (!area.size.inside(gear.coord))
                        {
                            auto center = area.coor + (area.size / 2);
                            bell::getref(gear.id)->
                                SIGNAL(tier::release, e2::form::layout::shift, center);
                        }
                        boss.base::deface();
                    };
                    boss.SUBMIT(tier::release, hids::events::mouse::button::click::leftright, gear)
                    {
                        auto backup = boss.This();
                        boss.base::detach();
                        gear.dismiss();
                    };
                    boss.SUBMIT(tier::release, hids::events::mouse::button::click::middle, gear)
                    {
                        auto backup = boss.This();
                        boss.base::detach();
                        gear.dismiss();
                    };
                    boss.SUBMIT(tier::release, e2::form::proceed::detach, backup)
                    {
                        boss.base::detach(); // The object kills itself.
                    };
                    boss.SUBMIT(tier::release, e2::form::quit, nested_item)
                    {
                        if (nested_item) boss.base::detach(); // The object kills itself.
                    };
                });

            window->extend(location);
            window->attach(create_app(create_app, config.type, config.data));
            log(" world create type=", config.type);
            world->branch(config.type, window);

            what.frame = window;
        };
        world->SUBMIT(tier::general, e2::form::global::lucidity, alpha)
        {
            if (alpha == -1)
            {
                alpha = skin::shady();
            }
            else
            {
                alpha = std::clamp(alpha, 0, 255);
                skin::setup(tone::lucidity, alpha);
                world->SIGNAL(tier::preview, e2::form::global::lucidity, alpha);
            }
        };

        // Init registry/menu list.
        {
            sptr<registry_t> menu_list_ptr;
            world->SIGNAL(tier::request, e2::bindings::list::apps, menu_list_ptr);
            auto& menu_list = *menu_list_ptr;
            auto b = app::shared::objs_config.begin();
            auto e = app::shared::objs_config.end();
            auto find = [&](auto type_id)
            {
                //auto iter = std::find_if(b, e, [&](auto& item){ return item.first == type_id; });
                //return iter != e ? std::distance(b, iter) : 0;
                iota i = 0;
                for (auto& a : app::shared::objs_config)
                {
                    if (a.type == type_id) break;
                    ++i;
                }
                return i;
            };
            #ifdef DEMO
                #ifdef PROD
                    //objs_config[objs::Tile].data = "VTM_PROFILE_1=\"Tile\", \"Tiling Window Manager\", h1:1(v1:1(\"bash -c 'LC_ALL=en_US.UTF-8 mc -c -x; bash'\", h1:1(\"bash -c 'ls /bin | nl | ccze -A; bash'\", a(\"RefreshRate\",\"\",\"\"))), a(\"Calc\",\"app title\",\"app data\"))";
                    //objs_config[objs::Tile].data = "VTM_PROFILE_1=\"Tile\", \"Tiling Window Manager\", h1:1(v1:1(\"bash -c 'LC_ALL=en_US.UTF-8 mc -c -x; bash'\", h1:1(\"bash -c 'ls /bin | nl | ccze -A; bash'\", a(\"Text\",\"app title\",\"app data\"))), a(\"Calc\",\"app title\",\"app data\"))";
                    //objs_config[objs::Tile].data = "VTM_PROFILE_1=\"Tile\", \"Tiling Window Manager\", h1:1:1(v1:1:2(\"bash -c 'LC_ALL=en_US.UTF-8 mc -c -x -d; cat'\", h1:1:0(\"bash -c 'ls /bin | nl | ccze -A; bash'\", a(\"RefreshRate\",\"\",\"\"))), a(\"Calc\",\"\",\"\"))";
                    app::shared::objs_config[app::shared::objs::Tile].data = "VTM_PROFILE_1=\"Tile\", \"Tiling Window Manager\", h(v(\"bash -c 'LC_ALL=en_US.UTF-8 mc -c -x -d; cat'\", h(\"bash -c 'ls /bin | nl | ccze -A; bash'\", a(\"RefreshRate\",\"\",\"\"))), a(\"Calc\",\"\",\"\"))";
                #else
                    app::shared::objs_config[app::shared::objs::Tile].data = "VTM_PROFILE_1=\"Tile\", \"Tiling Window Manager\", h1:1(v1:1(\"bash -c 'LC_ALL=en_US.UTF-8 mc -c -x -d; cat'\", h1:1(\"bash -c 'ls /bin | nl | ccze -A; bash'\", a(\"RefreshRate\",\"\",\"\"))), a(\"Calc\",\"\",\"\"))";
                #endif

                for (auto i = app::shared::objs_config.size(); i-- != 0;)
                    menu_list[static_cast<id_t>(i)];
            #else
                #ifdef _WIN32
                    menu_list[find(app::shared::objs::CommandPrompt)];
                    menu_list[find(app::shared::objs::PowerShell)];
                    menu_list[find(app::shared::objs::Tile)];
                    menu_list[find(app::shared::objs::Logs)];
                    menu_list[find(app::shared::objs::View)];
                    menu_list[find(app::shared::objs::RefreshRate)];
                #else
                    menu_list[find(app::shared::objs::Term)];
                    menu_list[find(app::shared::objs::Tile)];
                    menu_list[find(app::shared::objs::Logs)];
                    menu_list[find(app::shared::objs::View)];
                    menu_list[find(app::shared::objs::RefreshRate)];
                    menu_list[find(app::shared::objs::vtm)];
                #endif
                // Add custom commands to the menu.
                for (auto& p : tiling_profiles)
                {
                    //todo rewrite
                    auto v = view{ p };
                    auto name = utf::get_quote(v, '\"');
                    if (!name.empty())
                    {
                        menu_list[static_cast<id_t>(app::shared::objs_config.size())];
                        auto m = app::shared::menu_item{};
                        m.type = app::shared::objs::Tile;
                        m.name = text{ name };
                        m.title = text{ name }; // Use the same title as the menu label.
                        m.data = text{ p };
                        app::shared::objs_config.push_back(m);
                    }
                }
            #endif

            #ifdef DEMO
                auto creator = [&](id_t menu_item_id, rect area)
                {
                    auto what = decltype(e2::form::proceed::createat)::type{};
                    what.menu_item_id = menu_item_id;
                    what.location = area;
                    world->SIGNAL(tier::release, e2::form::proceed::createat, what);
                };
                auto sub_pos = twod{ 12+17, 0 };
                creator(find(app::shared::objs::Test), { twod{ 22     , 1  } + sub_pos, { 70, 21 } });
                creator(find(app::shared::objs::Shop), { twod{ 4      , 6  } + sub_pos, { 82, 38 } });
                creator(find(app::shared::objs::Calc), { twod{ 15     , 15 } + sub_pos, { 65, 23 } });
                creator(find(app::shared::objs::Text), { twod{ 30     , 22 } + sub_pos, { 59, 26 } });
                creator(find(app::shared::objs::MC),   { twod{ 49     , 28 } + sub_pos, { 63, 22 } });
                creator(find(app::shared::objs::Term), { twod{ 34     , 38 } + sub_pos, { 64, 16 } });
                creator(find(app::shared::objs::Term), { twod{ 44 + 85, 35 } + sub_pos, { 64, 15 } });
                creator(find(app::shared::objs::Term), { twod{ 40 + 85, 42 } + sub_pos, { 64, 15 } });
                creator(find(app::shared::objs::Tile), { twod{ 40 + 85,-10 } + sub_pos, {160, 42 } });

                creator(find(app::shared::objs::View), { twod{ 0, 7 } + twod{ -120, 60 }, { 120, 52 } });
                creator(find(app::shared::objs::View), { twod{ 0,-1 } + sub_pos, { 120, 52 } });

                sub_pos = twod{-120, 60};
                creator(find(app::shared::objs::Truecolor),   { twod{ 20, 15 } + sub_pos, { 70, 30 } });
                creator(find(app::shared::objs::Logs),        { twod{ 52, 33 } + sub_pos, { 45, 12 } });
                creator(find(app::shared::objs::RefreshRate), { twod{ 60, 41 } + sub_pos, { 35, 10 } });
            #endif

        }

        world->SIGNAL(tier::general, e2::config::fps, 60);

        iota usr_count = 0;
        auto user = os::user();
        auto path = utf::concat("monotty_", user);
        log("user: ", user);
        log("pipe: ", path);

        if (auto link = os::ipc::open<os::server>(path))
        {
            log("sock: listening socket ", link);

            while (auto peer = link->meet())
            {
                if (!peer->cred(user))
                {
                    log("sock: other users are not allowed to the session, abort");
                    continue;
                }

                auto _region = peer->line(';');
                auto _ip     = peer->line(';');
                auto _user   = peer->line(';');
                auto _name   = peer->line(';');
                auto _mode   = peer->line(';');
                log("peer: region= ", _region,
                        ", ip= "    , _ip,
                        ", user= "  , _user,
                        ", name= "  , _name,
                        ", mode= "  , _mode);
                text c_ip;
                text c_port;
                auto c_info = utf::divide(_ip, " ");
                if (c_info.size() > 0) c_ip   = c_info[0];
                if (c_info.size() > 1) c_port = c_info[1];

                utf::change(_ip, " ", ":");

                //todo Move user's viewport to the last saved position
                auto user_coor = twod{};

                //todo distinguish users by config, enumerate if no config
                _name = "[" + _name + ":" + std::to_string(usr_count++) + "]";
                log("main: creating a new thread for user ", _name);

                std::thread{ [=]
                {
                    iota uibar_min_size = 4;
                    iota uibar_full_size = 32;
                    log("user: session name ", peer);

                    #ifndef PROD
                        auto username = "[User." + utf::remain(c_ip) + ":" + c_port + "]";
                    #else
                        auto username = _name;
                    #endif

                    auto lock = std::make_unique<events::sync>();
                    iota legacy_mode = os::legacy::clean;
                    if (auto mode = utf::to_int(view(_mode)))
                    {
                        legacy_mode = mode.value();
                    }
                    auto client = world->invite<ui::gate>(username, legacy_mode);
                    auto client_shadow = ptr::shadow(client);
                    auto world_shadow = ptr::shadow(world);
                    auto my_id = client->id;

                    // Taskbar Layout (PoC)

                    #ifdef _WIN32
                        auto current_default = app::shared::objs::CommandPrompt;
                        //auto current_default = app::shared::objs::PowerShell;
                    #else
                        auto current_default = app::shared::objs::Term;
                    #endif
                    auto previous_default = current_default;

                    //todo unify
                    client->SUBMIT(tier::request, e2::data::changed, data)
                    {
                        data = current_default;
                    };
                    client->SUBMIT(tier::preview, e2::data::changed, data)
                    {
                        data = previous_default;
                    };
                    client->SUBMIT(tier::release, e2::data::changed, data)
                    {
                        auto new_default = static_cast<app::shared::objs>(data);
                        if (current_default != new_default)
                        {
                            previous_default = current_default;
                            current_default = new_default;
                        }
                    };

                    auto app_template = [&](auto& data_src, auto const& utf8)
                    {
                        const static auto c4 = app::shared::c4;
                        const static auto x4 = app::shared::x4;
                        const static auto c5 = app::shared::c5;
                        const static auto x5 = app::shared::x5;

                        auto item_area = ui::pads::ctor(dent{ 1,0,1,0 }, dent{ 0,0,0,1 })
                                ->plugin<pro::fader>(x4, c4, 0ms)//150ms)
                                ->invoke([&](auto& boss)
                                {
                                    auto data_src_shadow = ptr::shadow(data_src);
                                    boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::click::left, gear)
                                    {
                                        if (auto data_src = data_src_shadow.lock())
                                        {
                                            auto& inst = *data_src;
                                            inst.SIGNAL(tier::preview, e2::form::layout::expose, inst);
                                            auto& area = inst.base::area();
                                            auto center = area.coor + (area.size / 2);
                                            bell::getref(gear.id)->
                                                SIGNAL(tier::release, e2::form::layout::shift, center);  // Goto to the window.
                                            gear.pass_kb_focus(inst);
                                            gear.dismiss();
                                        }
                                    };
                                    boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::click::right, gear)
                                    {
                                        if (auto data_src = data_src_shadow.lock())
                                        {
                                            auto& inst = *data_src;
                                            inst.SIGNAL(tier::preview, e2::form::layout::expose, inst);
                                            auto& area = gear.area();
                                            auto center = area.coor + (area.size / 2);
                                            inst.SIGNAL(tier::preview, e2::form::layout::appear, center); // Pull window.
                                            gear.pass_kb_focus(inst);
                                            gear.dismiss();
                                        }
                                    };
                                    boss.SUBMIT_BYVAL(tier::release, e2::form::state::mouse, hits)
                                    {
                                        if (auto data_src = data_src_shadow.lock())
                                        {
                                            data_src->SIGNAL(tier::release, e2::form::highlight::any, !!hits);
                                        }
                                    };
                                });
                            auto label_area = item_area->attach(ui::fork::ctor());
                                auto mark_app = label_area->attach(slot::_1, ui::fork::ctor());
                                    auto mark = mark_app->attach(slot::_1, ui::pads::ctor(dent{ 2,1,0,0 }, dent{ 0,0,0,0 }))
                                                        ->attach(ui::item::ctor(ansi::fgc4(0xFF00ff00).add("‣"), faux));
                                    auto app_label = mark_app->attach(slot::_2,
                                                ui::item::ctor(ansi::fgc(whitelt).add(utf8).mgl(0).wrp(wrap::off).jet(bias::left), true, true));
                                auto app_close_area = label_area->attach(slot::_2, ui::pads::ctor(dent{ 0,0,0,0 }, dent{ 0,0,1,1 }))
                                                                ->template plugin<pro::fader>(x5, c5, 150ms)
                                                                ->invoke([&](auto& boss)
                                                                {
                                                                    auto data_src_shadow = ptr::shadow(data_src);
                                                                    boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::click::left, gear)
                                                                    {
                                                                        if (auto data_src = data_src_shadow.lock())
                                                                        {
                                                                            data_src->SIGNAL(tier::release, e2::form::proceed::detach, data_src);
                                                                            gear.dismiss();
                                                                        }
                                                                    };
                                                                });
                                    auto app_close = app_close_area->attach(ui::item::ctor("  ×  ", faux));
                        return item_area;
                    };
                    auto apps_template = [&](auto& data_src, auto& apps_map)
                    {
                        const static auto c3 = app::shared::c3;
                        const static auto x3 = app::shared::x3;

                        auto apps = ui::list::ctor();
                        //todo loops are not compatible with Declarative UI
                        for (auto const& [class_id, inst_ptr_list] : *apps_map)
                        {
                            auto inst_id  = class_id;
                            auto obj_desc = app::shared::objs_config[class_id].name;
                            if (inst_ptr_list.size())
                            {
                                auto selected = class_id == current_default;
                                auto item_area = apps->attach(ui::pads::ctor(dent{ 0,0,0,1 }, dent{ 0,0,1,0 }))
                                                    ->template plugin<pro::fader>(x3, c3, 0ms)
                                                    ->depend_on_collection(inst_ptr_list)
                                                    ->invoke([&](auto& boss)
                                                    {
                                                        boss.mouse.take_all_events(faux);
                                                        auto data_src_shadow = ptr::shadow(data_src);
                                                        boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::click::left, gear)
                                                        {
                                                            if (auto data_src = data_src_shadow.lock())
                                                            {
                                                                sptr<registry_t> registry_ptr;
                                                                data_src->SIGNAL(tier::request, e2::bindings::list::apps, registry_ptr);
                                                                auto& app_list = (*registry_ptr)[inst_id];
                                                                // Rotate list forward.
                                                                app_list.push_back(app_list.front());
                                                                app_list.pop_front();
                                                                // Expose window.
                                                                auto& inst = *app_list.back();
                                                                inst.SIGNAL(tier::preview, e2::form::layout::expose, inst);
                                                                auto& area = inst.base::area();
                                                                auto center = area.coor + (area.size / 2);
                                                                bell::getref(gear.id)->
                                                                SIGNAL(tier::release, e2::form::layout::shift, center);  // Goto to the window.
                                                                gear.pass_kb_focus(inst);
                                                                gear.dismiss();
                                                            }
                                                        };
                                                        boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::click::right, gear)
                                                        {
                                                            if (auto data_src = data_src_shadow.lock())
                                                            {
                                                                sptr<registry_t> registry_ptr;
                                                                data_src->SIGNAL(tier::request, e2::bindings::list::apps, registry_ptr);
                                                                auto& app_list = (*registry_ptr)[inst_id];
                                                                // Rotate list forward.
                                                                app_list.push_front(app_list.back());
                                                                app_list.pop_back();
                                                                // Expose window.
                                                                auto& inst = *app_list.back();
                                                                inst.SIGNAL(tier::preview, e2::form::layout::expose, inst);
                                                                auto& area = inst.base::area();
                                                                auto center = area.coor + (area.size / 2);
                                                                bell::getref(gear.id)->
                                                                SIGNAL(tier::release, e2::form::layout::shift, center);  // Goto to the window.
                                                                gear.pass_kb_focus(inst);
                                                                gear.dismiss();
                                                            }
                                                        };
                                                    });
                                    auto block = item_area->attach(ui::fork::ctor(axis::Y));
                                        auto head_area = block->attach(slot::_1, ui::pads::ctor(dent{ 0,0,0,0 }, dent{ 0,0,1,1 }));
                                            auto head = head_area->attach(ui::item::ctor(obj_desc, true));
                                        auto list_pads = block->attach(slot::_2, ui::pads::ctor(dent{ 0,0,0,0 }, dent{ 0,0,0,0 }));
                                auto insts = list_pads->attach(ui::list::ctor())
                                                      ->attach_collection(e2::form::prop::header, inst_ptr_list, app_template);
                            }
                        }
                        return apps;
                    };
                    auto menuitems_template = [&](auto& data_src, auto& apps_map)
                    {
                        const static auto c3 = app::shared::c3;
                        const static auto x3 = app::shared::x3;

                        auto menuitems = ui::list::ctor();
                        //todo loops are not compatible with Declarative UI
                        for (auto const& [class_id, inst_ptr_list] : *apps_map)
                        {
                            auto id = class_id;
                            auto obj_desc = app::shared::objs_config[class_id].name;

                            auto selected = class_id == current_default;
                            auto item_area = menuitems->attach(ui::pads::ctor(dent{ 0,0,0,1 }, dent{ 0,0,1,0 }))
                                                    ->plugin<pro::fader>(x3, c3, 0ms)
                                                    ->invoke([&](auto& boss)
                                                    {
                                                        boss.mouse.take_all_events(faux);
                                                        boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::click::left, gear)
                                                        {
                                                            if (auto client = client_shadow.lock())
                                                            {
                                                                client->SIGNAL(tier::release, e2::data::changed, id);
                                                                gear.dismiss();
                                                            }
                                                        };
                                                        boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::dblclick::left, gear)
                                                        {
                                                            if (auto world = world_shadow.lock())
                                                            {
                                                                static iota random = 0;
                                                                random = (random + 2) % 10;
                                                                auto offset = twod{ random * 2, random };
                                                                auto viewport = gear.area();
                                                                gear.slot.coor = viewport.coor + viewport.size / 4 + offset;
                                                                gear.slot.size = viewport.size / 2;
                                                                world->SIGNAL(tier::release, e2::form::proceed::createby, gear);
                                                            }
                                                        };
                                                    });
                                auto block = item_area->attach(ui::fork::ctor(axis::X));
                                    auto mark_area = block->attach(slot::_1, ui::pads::ctor(dent{ 1,1,0,0 }, dent{ 0,0,0,0 }));
                                        auto mark = mark_area->attach(ui::item::ctor(ansi::fgc4(selected ? 0xFF00ff00
                                                                                                         : 0xFF000000).add("██"), faux))
                                                    ->invoke([&](auto& boss)
                                                    {
                                                        if (auto client = client_shadow.lock())
                                                        {
                                                            auto mark_shadow = ptr::shadow(boss.This());
                                                            client->SUBMIT_T_BYVAL(tier::release, e2::data::changed, boss.tracker, data)
                                                            {
                                                                auto selected = id == data;
                                                                if (auto mark = mark_shadow.lock())
                                                                {
                                                                    mark->set(ansi::fgc4(selected ? 0xFF00ff00 : 0xFF000000).add("██"));
                                                                    mark->deface();
                                                                }
                                                            };
                                                        }
                                                    });
                                    auto label_area = block->attach(slot::_2, ui::pads::ctor(dent{ 1,1,0,0 }, dent{ 0,0,0,0 }));
                                        auto label = label_area->attach(ui::item::ctor(ansi::fgc4(0xFFffffff).add(obj_desc), true, true));
                        }
                        return menuitems;
                    };
                    auto user_template = [&](auto& data_src, auto const& utf8)
                    {
                        const static auto c3 = app::shared::c3;
                        const static auto x3 = app::shared::x3;

                        auto item_area = ui::pads::ctor(dent{ 1,0,0,1 }, dent{ 0,0,1,0 })
                                             ->plugin<pro::fader>(x3, c3, 150ms);
                            auto user = item_area->attach(
                                ui::item::ctor(ansi::esc(" &").nil().add(" ")
                                    .fgc4(data_src->id == my_id ? rgba::color256[whitelt] : 0x00).add(utf8), true));
                        return item_area;
                    };
                    auto branch_template = [&](auto& data_src, auto& usr_list)
                    {
                        auto users = ui::list::ctor()
                            ->attach_collection(e2::form::prop::name, *usr_list, user_template);
                        return users;
                    };
                    {
                    auto window = client->attach(ui::cake::ctor());
                        auto taskbar_viewport = window->attach(ui::fork::ctor(axis::X))
                                                ->invoke([&](auto& boss)
                                                {
                                                    boss.broadcast->SUBMIT(tier::request, e2::form::prop::viewport, viewport)
                                                    {
                                                        viewport = boss.base::area();
                                                    };
                                                });
                        auto taskbar = taskbar_viewport->attach(slot::_1, ui::fork::ctor(axis::Y))
                                            ->colors(whitedk, 0x60202020)
                                            ->plugin<pro::limit>(twod{ uibar_min_size,-1 }, twod{ uibar_min_size,-1 })
                                            ->plugin<pro::timer>()
                                            ->plugin<pro::acryl>()
                                            ->plugin<pro::cache>()
                                            ->invoke([&](auto& boss)
                                            {
                                                boss.mouse.template draggable<sysmouse::left>();
                                                boss.SUBMIT(tier::release, e2::form::drag::pull::_<sysmouse::left>, gear)
                                                {
                                                    auto& limits = boss.template plugins<pro::limit>();
                                                    auto lims = limits.get();
                                                    lims.min.x += gear.delta.get().x;
                                                    lims.max.x = uibar_full_size = lims.min.x;
                                                    limits.set(lims.min, lims.max);
                                                    boss.base::reflow();
                                                };
                                                boss.SUBMIT(tier::release, e2::form::state::mouse, active)
                                                {
                                                    auto apply = [&](auto active)
                                                    {
                                                        auto& limits = boss.template plugins<pro::limit>();
                                                        auto size = active ? uibar_full_size : std::min(uibar_full_size, uibar_min_size);
                                                        auto lims = twod{ size,-1 };
                                                        limits.set(lims, lims);
                                                        boss.base::reflow();
                                                        return faux; // One shot call.
                                                    };
                                                    auto& timer = boss.template plugins<pro::timer>();
                                                    timer.pacify(faux);
                                                    if (active) apply(true);
                                                    else timer.actify(faux, MENU_TIMEOUT, apply);
                                                };
                                                boss.broadcast->SUBMIT(tier::request, e2::form::prop::viewport, viewport)
                                                {
                                                    viewport.coor.x += uibar_min_size;
                                                    viewport.size.x -= uibar_min_size;
                                                };
                                            });
                            auto apps_users = taskbar->attach(slot::_1, ui::fork::ctor(axis::Y, 0, 100));
                            {
                                const static auto c3 = app::shared::c3;
                                const static auto x3 = app::shared::x3;
                                const static auto c6 = app::shared::c6;
                                const static auto x6 = app::shared::x6;

                                auto apps_area = apps_users->attach(slot::_1, ui::fork::ctor(axis::Y));
                                {
                                    auto label_pads = apps_area->attach(slot::_1, ui::pads::ctor(dent{ 0,0,1,1 }, dent{ 0,0,0,0 }))
                                                               ->plugin<pro::fader>(x3, c3, 150ms);
                                        auto label_bttn = label_pads->attach(ui::fork::ctor());
                                            auto label = label_bttn->attach(slot::_1,
                                                ui::item::ctor(ansi::fgc(whitelt).add("  ≡ "), faux, faux));
                                            auto bttn_area = label_bttn->attach(slot::_2, ui::fork::ctor());
                                                auto bttn_pads = bttn_area->attach(slot::_2, ui::pads::ctor(dent{ 2,2,0,0 }, dent{ 0,0,1,1 }))
                                                                          ->plugin<pro::fader>(x6, c6, 150ms);
                                                    auto bttn = bttn_pads->attach(ui::item::ctor(">", faux));
                                    auto applist_area = apps_area->attach(slot::_2, ui::pads::ctor(dent{ 0,0,1,0 }, dent{}))
                                                                 ->attach(ui::cake::ctor());
                                        auto task_menu_area = applist_area->attach(ui::fork::ctor(axis::Y, 0, 0));
                                            auto menu_scrl = task_menu_area->attach(slot::_1, ui::rail::ctor(axes::ONLY_Y))
                                                                           ->colors(0x00, 0x00); //todo mouse events passthrough
                                                auto menuitems = menu_scrl->attach_element(e2::bindings::list::apps, world, menuitems_template);
                                            auto tasks_scrl = task_menu_area->attach(slot::_2, ui::rail::ctor(axes::ONLY_Y))
                                                                            ->colors(0x00, 0x00); //todo mouse events passthrough
                                                auto apps = tasks_scrl->attach_element(e2::bindings::list::apps, world, apps_template);
                                    label_pads->invoke([&](auto& boss)
                                                {
                                                    auto task_menu_area_shadow = ptr::shadow(task_menu_area);
                                                    auto bttn_shadow = ptr::shadow(bttn);
                                                    boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::click::left, gear)
                                                    {
                                                        if (auto bttn = bttn_shadow.lock())
                                                        if (auto task_menu_area = task_menu_area_shadow.lock())
                                                        {
                                                            auto state = task_menu_area->get_ratio();
                                                            bttn->set(state ? ">" : "<");
                                                            if (state) task_menu_area->config(0, 1);
                                                            else       task_menu_area->config(1, 0);
                                                            gear.dismiss();
                                                        }
                                                    };
                                                });
                                    apps_area->invoke([&](auto& boss)
                                                {
                                                    auto task_menu_area_shadow = ptr::shadow(task_menu_area);
                                                    auto bttn_shadow = ptr::shadow(bttn);
                                                    boss.SUBMIT_BYVAL(tier::release, e2::form::state::mouse, active)
                                                    {
                                                        if (!active)
                                                        if (auto bttn = bttn_shadow.lock())
                                                        if (auto task_menu_area = task_menu_area_shadow.lock())
                                                        {
                                                            if (auto state = task_menu_area->get_ratio())
                                                            {
                                                                bttn->set(">");
                                                                task_menu_area->config(0);
                                                            }
                                                        }
                                                    };
                                                });
                                    //todo make some sort of highlighting at the bottom and top
                                    //scroll_bars_left(items_area, items_scrl);
                                }
                                auto users_area = apps_users->attach(slot::_2, ui::fork::ctor(axis::Y));
                                {
                                    auto label_pads = users_area->attach(slot::_1, ui::pads::ctor(dent{ 0,0,1,1 }, dent{ 0,0,0,0 }))
                                                                ->plugin<pro::fader>(x3, c3, 150ms);
                                        auto label_bttn = label_pads->attach(ui::fork::ctor());
                                            auto label = label_bttn->attach(slot::_1,
                                                ui::item::ctor(ansi::fgc(whitelt).add("Users"), faux, faux));
                                            auto bttn_area = label_bttn->attach(slot::_2, ui::fork::ctor());
                                                auto bttn_pads = bttn_area->attach(slot::_2, ui::pads::ctor(dent{ 2,2,0,0 }, dent{ 0,0,1,1 }))
                                                                          ->plugin<pro::fader>(x6, c6, 150ms);
                                                    auto bttn = bttn_pads->attach(ui::item::ctor("<", faux));
                                    auto userlist_area = users_area->attach(slot::_2, ui::pads::ctor())
                                                                   ->plugin<pro::limit>();
                                        auto users = userlist_area->attach_element(e2::bindings::list::users, world, branch_template);
                                        //auto users_rail = userlist_area->attach(ui::rail::ctor());
                                        //auto users = users_rail->attach_element(e2::bindings::list::users, world, branch_template);
                                    //todo unify
                                    bttn_pads->invoke([&](auto& boss)
                                                {
                                                    auto userlist_area_shadow = ptr::shadow(userlist_area);
                                                    auto bttn_shadow = ptr::shadow(bttn);
                                                    boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::click::left, gear)
                                                    {
                                                        static bool state = faux;
                                                        if (auto bttn = bttn_shadow.lock())
                                                        if (auto userlist = userlist_area_shadow.lock())
                                                        {
                                                            state = !state;
                                                            bttn->set(state ? ">" : "<");
                                                            auto& limits = userlist->plugins<pro::limit>();
                                                            auto lims = limits.get();
                                                            lims.min.y = lims.max.y = state ? 0 : -1;
                                                            limits.set(lims, true);
                                                            userlist->base::reflow();
                                                        }
                                                    };
                                                });
                                }
                            }
                            auto bttns_area = taskbar->attach(slot::_2, ui::fork::ctor(axis::X));
                            {
                                const static auto c2 = app::shared::c2;
                                const static auto x2 = app::shared::x2;
                                const static auto c1 = app::shared::c1;
                                const static auto x1 = app::shared::x1;

                                auto bttns = bttns_area->attach(slot::_1, ui::fork::ctor(axis::X));
                                    auto disconnect_area = bttns->attach(slot::_1, ui::pads::ctor(dent{ 2,3,1,1 }))
                                                                ->plugin<pro::fader>(x2, c2, 150ms)
                                                                ->invoke([&](auto& boss)
                                                                {
                                                                    boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                                                                    {
                                                                        if (auto owner = base::getref(gear.id))
                                                                        {
                                                                            owner->SIGNAL(tier::release, e2::conio::quit, "taskbar: logout by button");
                                                                            gear.dismiss();
                                                                        }
                                                                    };
                                                                });
                                        auto disconnect = disconnect_area->attach(ui::item::ctor("× Disconnect"));
                                    auto shutdown_area = bttns->attach(slot::_2, ui::pads::ctor(dent{ 2,3,1,1 }))
                                                              ->plugin<pro::fader>(x1, c1, 150ms)
                                                              ->invoke([&](auto& boss)
                                                              {
                                                                  boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                                                                  {
                                                                      //todo unify, see system.h:1614
                                                                      #if defined(__APPLE__) || defined(__FreeBSD__)
                                                                      auto path2 = "/tmp/" + path + ".sock";
                                                                      ::unlink(path2.c_str());
                                                                      #endif
                                                                      os::exit(0, "taskbar: shutdown by button");
                                                                  };
                                                              });
                                        auto shutdown = shutdown_area->attach(ui::item::ctor("× Shutdown"));
                            }
                    }
                    client->color(app::shared::background_color.fgc(), app::shared::background_color.bgc());
                    text header = username;
                    text footer = ansi::mgr(1).mgl(1).add(MONOTTY_VER);
                    client->SIGNAL(tier::release, e2::form::prop::name, header);
                    client->SIGNAL(tier::preview, e2::form::prop::header, header);
                    client->SIGNAL(tier::preview, e2::form::prop::footer, footer);
                    client->base::moveby(user_coor);
                    lock.reset();
                    log("user: new gate for ", peer);

                    #ifndef PROD
                    client->proceed(peer, _region);
                    #else
                    client->proceed(peer, username);
                    #endif

                    lock = std::unique_ptr<events::sync>();
                    log("user: ", peer, " has logged out");
                    client->detach();
                    log("user: ", peer, " is diconnected");
                    log("user: client.use_count() ", client.use_count());
                    client.reset();
                    lock.reset();
                } }.detach();

                log("main: new thread constructed for ", peer);
            }

            world->SIGNAL(tier::general, e2::config::fps, 0);
        }
    }
    os::exit(0, "bye!");
}