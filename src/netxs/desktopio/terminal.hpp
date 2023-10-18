// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#include "console.hpp"

namespace netxs::events::userland
{
    struct uiterm
    {
        EVENTPACK( uiterm, ui::e2::extra::slot5 )
        {
            EVENT_XS( io_log, bool ),
            EVENT_XS( selmod, si32 ),
            EVENT_XS( selalt, si32 ),
            GROUP_XS( colors, rgba ),
            GROUP_XS( layout, si32 ),
            GROUP_XS( search, input::hids ),

            SUBSET_XS( layout )
            {
                EVENT_XS( align , si32 ),
                EVENT_XS( wrapln, si32 ),
            };
            SUBSET_XS( search )
            {
                EVENT_XS( forward, input::hids ),
                EVENT_XS( reverse, input::hids ),
                EVENT_XS( status , si32        ),
            };
            SUBSET_XS( colors )
            {
                EVENT_XS( bg, rgba ),
                EVENT_XS( fg, rgba ),
            };
        };
    };
}

// terminal: Terminal UI control.
namespace netxs::ui
{
    struct term
        : public ui::form<term>
    {
        using events = netxs::events::userland::uiterm;

        struct commands
        {
            struct erase
            {
                struct line
                {
                    enum : si32
                    {
                        right = 0,
                        left  = 1,
                        all   = 2,
                        wraps = 3,
                    };
                };
                struct display
                {
                    enum : si32
                    {
                        below      = 0,
                        above      = 1,
                        viewport   = 2,
                        scrollback = 3,
                    };
                };
            };
            struct ui
            {
                enum commands : si32
                {
                    center,
                    togglewrp,
                    togglesel,
                    restart,
                    sighup,
                    undo,
                    redo,
                    deselect,
                    look_fwd,
                    look_rev,
                };
            };
            struct cursor // See pro::caret.
            {
                enum : si32
                {
                    def_style          = 0, // blinking box
                    blinking_box       = 1, // blinking box (default)
                    steady_box         = 2, // steady box
                    blinking_underline = 3, // blinking underline
                    steady_underline   = 4, // steady underline
                    blinking_I_bar     = 5, // blinking I-bar
                    steady_I_bar       = 6, // steady I-bar
                };
            };
            struct atexit
            {
                enum codes : si32
                {
                    ask,     // Stay open.
                    smart,   // Stay open if exit code != 0.
                    close,   // Always quit.
                    restart, // Restart session.
                    retry,   // Restart session if exit code != 0.
                };
            };
            struct fx
            {
                enum shader : si32
                {
                    xlight,
                    color,
                    invert,
                    reverse,
                };
            };
        };

        // term: Terminal configuration.
        struct termconfig
        {
            using pals = std::remove_const_t<decltype(rgba::vt256)>;

            si32 def_mxline;
            si32 def_length;
            si32 def_growdt;
            si32 def_growmx;
            wrap def_wrpmod;
            si32 def_tablen;
            si32 def_lucent;
            si32 def_margin;
            si32 def_atexit;
            rgba def_fcolor;
            rgba def_bcolor;
            si32 def_selmod;
            bool def_selalt;
            bool def_cursor;
            bool def_cur_on;
            bool resetonkey;
            bool resetonout;
            bool def_io_log;
            span def_period;
            pals def_colors;

            cell def_safe_c;
            cell def_ansi_c;
            cell def_rich_c;
            cell def_html_c;
            cell def_text_c;
            cell def_none_c;
            cell def_find_c;

            si32 def_safe_f;
            si32 def_ansi_f;
            si32 def_rich_f;
            si32 def_html_f;
            si32 def_text_f;
            si32 def_none_f;
            si32 def_find_f;

            si32 def_altscr;
            bool def_alt_on;

            termconfig(xmls& config)
            {
                static auto atexit_options = std::unordered_map<text, commands::atexit::codes>
                    {{ "auto",    commands::atexit::smart   },
                     { "ask",     commands::atexit::ask     },
                     { "close",   commands::atexit::close   },
                     { "restart", commands::atexit::restart },
                     { "retry",   commands::atexit::retry   }};
                static auto fx_options = std::unordered_map<text, commands::fx::shader>
                    {{ "xlight",  commands::fx::xlight  },
                     { "coolor",  commands::fx::color   },
                     { "invert",  commands::fx::invert  },
                     { "reverse", commands::fx::reverse }};

                config.cd("/config/term/");
                def_mxline = std::max(1, config.take("scrollback/maxline",   si32{ 65535 }));
                def_length = std::max(1, config.take("scrollback/size",      si32{ 40000 }));
                def_growdt = std::max(0, config.take("scrollback/growstep",  si32{ 0 }    ));
                def_growmx = std::max(0, config.take("scrollback/growlimit", si32{ 0 }    ));
                def_wrpmod =             config.take("scrollback/wrap",      deco::defwrp == wrap::on) ? wrap::on : wrap::off;
                resetonkey =             config.take("scrollback/reset/onkey",     true);
                resetonout =             config.take("scrollback/reset/onoutput",  faux);
                def_altscr = std::max(1, config.take("scrollback/altscroll/step", si32{ 1 }));
                def_alt_on =             config.take("scrollback/altscroll/enabled", true);
                def_tablen = std::max(1, config.take("tablen",               si32{ 8 }    ));
                def_lucent = std::max(0, config.take("fields/lucent",        si32{ 0xC0 } ));
                def_margin = std::max(0, config.take("fields/size",          si32{ 0 }    ));
                def_selmod =             config.take("selection/mode",       mime::textonly, xml::options::selmod);
                def_selalt =             config.take("selection/rect",       faux);
                def_cur_on =             config.take("cursor/show",          true);
                def_cursor =             config.take("cursor/style",         true, xml::options::cursor);
                def_period =             config.take("cursor/blink",         span{ skin::globals().blink_period });
                def_io_log =             config.take("logs",                 faux);
                def_atexit =             config.take("atexit",               commands::atexit::smart, atexit_options);
                def_fcolor =             config.take("color/default/fgc",    rgba{ whitelt });
                def_bcolor =             config.take("color/default/bgc",    rgba{ blackdk });

                def_safe_c =             config.take("color/selection/protected", cell{}.bgc(bluelt)    .fgc(whitelt));
                def_ansi_c =             config.take("color/selection/ansi",      cell{}.bgc(bluelt)    .fgc(whitelt));
                def_rich_c =             config.take("color/selection/rich",      cell{}.bgc(bluelt)    .fgc(whitelt));
                def_html_c =             config.take("color/selection/html",      cell{}.bgc(bluelt)    .fgc(whitelt));
                def_text_c =             config.take("color/selection/text",      cell{}.bgc(bluelt)    .fgc(whitelt));
                def_none_c =             config.take("color/selection/none",      cell{}.bgc(blacklt)   .fgc(whitedk));
                def_find_c =             config.take("color/match",               cell{}.bgc(0xFF007F00).fgc(whitelt));

                def_safe_f =             config.take("color/selection/protected/fx", commands::fx::color,  fx_options);
                def_ansi_f =             config.take("color/selection/ansi/fx",      commands::fx::xlight, fx_options);
                def_rich_f =             config.take("color/selection/rich/fx",      commands::fx::xlight, fx_options);
                def_html_f =             config.take("color/selection/html/fx",      commands::fx::xlight, fx_options);
                def_text_f =             config.take("color/selection/text/fx",      commands::fx::color,  fx_options);
                def_none_f =             config.take("color/selection/none/fx",      commands::fx::color,  fx_options);
                def_find_f =             config.take("color/match/fx",               commands::fx::color,  fx_options);

                std::copy(std::begin(rgba::vt256), std::end(rgba::vt256), std::begin(def_colors));
                for (auto i = 0; i < 16; i++)
                {
                    def_colors[i] = config.take("color/color" + std::to_string(i), def_colors[i]);
                }
            }
        };

        // term: VT-buffer status.
        struct term_state
        {
            enum type
            {
                empty,
                lines,
                block,
            };

            si32 size{}; // term_state: Terminal scrollback current size.
            si32 peak{}; // term_state: Terminal scrollback limit.
            si32 step{}; // term_state: Terminal scrollback increase step.
            si32 mxsz{}; // term_state: Terminal scrollback increase limit.
            twod area{}; // term_state: Terminal viewport size.
            escx data{}; // term_state: Status string.
            type mode{}; // term_state: Selection mode.
            twod coor{}; // term_state: Selection coor.
            ui64 body{}; // term_state: Selection rough volume.
            ui64 hash{}; // term_state: Selection update indicator.

            template<class BufferBase>
            auto update(BufferBase const& scroll)
            {
                if (scroll.update_status(*this))
                {
                    data.clear();
                    if (hash) data.scp();
                    data.jet(bias::right).add(size, "/", peak);
                    if (step && size != mxsz) data.add("+", step);
                    data.add(" ", area.x, ":", area.y);
                    if (hash)
                    {
                        data.rcp().jet(bias::left);
                        if (mode == type::block)
                        {
                            data.add(coor.x, ":", coor.y, " ");
                        }
                        else
                        {
                            data.add(coor.y, coor.y == 1 ? " row " : " rows ");
                                 if (body == 1) data.add("1 cell ");
                            else if (body <100) data.add     (body, " cells ");
                            else                data.add("~", body, " cells ");
                        }
                    }
                    return true;
                }
                else return faux;
            }
        };

        // term: VT-style mouse tracking functionality.
        struct m_tracking
        {
            using mode = input::mouse::mode;
            using prot = input::mouse::prot;

            term&       owner; // m_tracking: Terminal object reference.
            testy<twod> coord; // m_tracking: Last coord of mouse cursor.
            subs        token; // m_tracking: Subscription token.
            prot        encod; // m_tracking: Mouse encoding protocol.
            mode        state; // m_tracking: Mouse reporting mode.
            si32        smode; // m_tracking: Selection mode state backup.
            si32        bttns; // m_tracking: Last buttons state.

            m_tracking(term& owner)
                : owner{ owner                   },
                  encod{ prot::x11               },
                  state{ mode::none              },
                  smode{ owner.config.def_selmod },
                  bttns{ 0                       }
            { }

            operator bool () { return state != mode::none; }

            void check_focus(hids& gear) // Set keybd focus on any click if it is not set.
            {
                auto m = std::bitset<8>{ gear.m.buttons };
                auto s = std::bitset<8>{ gear.s.buttons };
                if (m[hids::buttons::right] && !s[hids::buttons::right])
                {
                    owner.RISEUP(tier::request, e2::form::state::keybd::find, gear_test, (gear.id, 0));
                    if (gear_test.second == 0)
                    {
                        pro::focus::set(owner.This(), gear.id, pro::focus::solo::off, pro::focus::flip::off);
                    }
                    owner.RISEUP(tier::preview, e2::form::layout::expose, area, ());
                }
                else if ((m[hids::buttons::left  ] && !s[hids::buttons::left  ])
                      || (m[hids::buttons::middle] && !s[hids::buttons::middle]))
                {
                    owner.RISEUP(tier::request, e2::form::state::keybd::find, gear_test, (gear.id, 0));
                    if (gear_test.second == 0)
                    {
                        pro::focus::set(owner.This(), gear.id, gear.meta(hids::anyCtrl) ? pro::focus::solo::off
                                                                                        : pro::focus::solo::on, pro::focus::flip::on);
                    }
                    owner.RISEUP(tier::preview, e2::form::layout::expose, area, ());
                }
            }
            void enable(mode m)
            {
                state = (mode)(state | m);
                if (state && !token.count()) // Do not subscribe if it is already subscribed.
                {
                    owner.LISTEN(tier::release, hids::events::device::mouse::any, gear, token)
                    {
                        check_focus(gear);
                        if (owner.selmod == mime::disabled)
                        {
                            if (gear.captured(owner.id))
                            {
                                if (!gear.m.buttons) gear.setfree(true);
                            }
                            else if (gear.m.buttons) gear.capture(owner.id);
                            auto& console = *owner.target;
                            auto c = gear.m.coordxy;
                            c.y -= console.get_basis();
                            auto moved = coord((state & mode::over) ? c
                                                                    : std::clamp(c, dot_00, console.panel - dot_11));
                            if (gear.s.changed != gear.m.changed)
                            {
                                owner.ipccon.mouse(gear, moved, coord, encod, state);
                            }
                            gear.dismiss();
                        }
                    };
                    smode = owner.selmod;
                }
                owner.selection_selmod(mime::disabled);
            }
            void disable(mode m)
            {
                state = (mode)(state & ~(m));
                if (!state) token.clear();
                owner.selection_selmod(smode);
            }
            void setmode(prot p) { encod = p; }
        };

        // term: Keyboard focus tracking functionality.
        struct f_tracking
        {
            using prot = input::focus::prot;

            term&       owner; // f_tracking: Terminal object reference.
            hook        token; // f_tracking: Subscription token.
            prot        encod; // f_tracking: Focus encoding mode.
            testy<bool> state; // f_tracking: Current focus state.

            f_tracking(term& owner)
                : owner{ owner },
                  encod{ prot::w32 }
            {
                owner.LISTEN(tier::release, e2::form::state::keybd::focus::state, s, token)
                {
                    if (state(s))
                    {
                        owner.ipccon.focus(s, encod);
                    }
                };
                owner.SIGNAL(tier::request, e2::form::state::keybd::check, state.last);
            }

            operator bool () { return state.last; }
            void set(bool enable)
            {
                encod = enable ? prot::dec : prot::w32;
            }
        };

        // term: Terminal title tracking functionality.
        struct w_tracking
        {
            term&                             owner; // w_tracking: Terminal object reference.
            std::map<text, text>              props;
            std::map<text, std::vector<text>> stack;
            escx                              queue;

            w_tracking(term& owner)
                : owner{ owner }
            { }
            // w_tracking: Get terminal window property.
            auto& get(text const& property)
            {
                auto& utf8 = props[property];
                if (property == ansi::osc_title)
                {
                    owner.RISEUP(tier::request, e2::form::prop::ui::header, utf8);
                }
                return utf8;
            }
            // w_tracking: Set terminal window property.
            void set(text const& property, qiew txt)
            {
                if (txt.empty()) txt = owner.cmdarg; // Deny empty titles.
                owner.target->flush();
                if (property == ansi::osc_label_title)
                {
                                  props[ansi::osc_label] = txt;
                    auto& utf8 = (props[ansi::osc_title] = txt);
                    owner.RISEUP(tier::preview, e2::form::prop::ui::header, utf8);
                }
                else
                {
                    auto& utf8 = (props[property] = txt);
                    if (property == ansi::osc_title)
                    {
                        owner.RISEUP(tier::preview, e2::form::prop::ui::header, utf8);
                    }
                }
            }
            // w_tracking: CSI n n  Device status report (DSR).
            void report(si32 n)
            {
                switch (n)
                {
                    default:
                    case 6: queue.report(owner.target->coord); break;
                    case 5: queue.add("OK");                   break;
                    case-1: queue.add("VT420");                break;
                }
                owner.answer(queue);
            }
            // w_tracking: CSI n c  Primary device attributes (DA1).
            void device(si32 n)
            {
                switch (n)
                {
                    case 0:
                    default:
                        queue.add("\033[?1;2c");
                        break;
                }
                owner.answer(queue);
            }
            // w_tracking: Manage terminal window props (XTWINOPS).
            void manage(fifo& q)
            {
                owner.target->flush();
                static constexpr auto all_title = si32{ 0  }; // Sub commands.
                static constexpr auto label     = si32{ 1  }; // Sub commands.
                static constexpr auto title     = si32{ 2  }; // Sub commands.
                static constexpr auto set_winsz = si32{ 8  }; // Set window size in characters.
                static constexpr auto maximize  = si32{ 9  }; // Toggle maximize/restore.
                static constexpr auto full_scrn = si32{ 10 }; // Toggle fullscreen mode.
                static constexpr auto view_size = si32{ 18 }; // Report viewport size.
                static constexpr auto get_label = si32{ 20 }; // Report icon   label. (Report as OSC L label ST).
                static constexpr auto get_title = si32{ 21 }; // Report window title. (Report as OSC l title ST).
                static constexpr auto put_stack = si32{ 22 }; // Push icon label and window title to   stack.
                static constexpr auto pop_stack = si32{ 23 }; // Pop  icon label and window title from stack.
                switch (auto option = q(0))
                {
                    case maximize:
                    case full_scrn:
                        owner.window_resize(dot_00);
                        break;
                    case set_winsz:
                    {
                        auto winsz = twod{};
                        winsz.y = q(-1);
                        winsz.x = q(-1);
                        owner.window_resize(winsz);
                        break;
                    }
                    case view_size: owner.answer(queue.win_sz(owner.target->panel)); break;
                    case get_label: owner.answer(queue.osc(ansi::osc_label_report, "")); break; // Return an empty string for security reasons
                    case get_title: owner.answer(queue.osc(ansi::osc_title_report, "")); break;
                    case put_stack:
                    {
                        auto push = [&](auto const& property)
                        {
                            stack[property].push_back(props[property]);
                        };
                        switch (q(all_title))
                        {
                            case title:     push(ansi::osc_title); break;
                            case label:     push(ansi::osc_label); break;
                            case all_title: push(ansi::osc_title);
                                            push(ansi::osc_label); break;
                            default: break;
                        }
                        break;
                    }
                    case pop_stack:
                    {
                        auto pop = [&](auto const& property)
                        {
                            auto& s = stack[property];
                            if (s.size())
                            {
                                set(property, s.back());
                                s.pop_back();
                            }
                        };
                        switch (q(all_title))
                        {
                            case title:     pop(ansi::osc_title); break;
                            case label:     pop(ansi::osc_label); break;
                            case all_title: pop(ansi::osc_title);
                                            pop(ansi::osc_label); break;
                            default: break;
                        }
                        break;
                    }
                    default:
                        log("%%CSI %option%... t (XTWINOPS) is not supported", prompt::term, option);
                        break;
                }
            }
        };

        // term: Terminal 16/256 color palette tracking functionality.
        struct c_tracking
        {
            using pals = std::remove_const_t<decltype(rgba::vt256)>;
            using func = std::unordered_map<text, std::function<void(view)>>;

            term& owner; // c_tracking: Terminal object reference.
            pals  color; // c_tracking: 16/256 colors palette.
            func  procs; // c_tracking: Handlers.

            void reset()
            {
                std::copy(std::begin(owner.config.def_colors), std::end(owner.config.def_colors), std::begin(color));
            }
            auto to_byte(char c)
            {
                if (c >= '0' && c <= '9') return c - '0';
                if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                                          return 0;
            }
            std::optional<ui32> record(view& data) // ; rgb:00/00/00
            {
                //todo implement request "?"
                utf::trim_front(data, " ;");
                if (data.length() >= 12 && data.starts_with("rgb:"))
                {
                    auto r1 = to_byte(data[ 4]);
                    auto r2 = to_byte(data[ 5]);
                    auto g1 = to_byte(data[ 7]);
                    auto g2 = to_byte(data[ 8]);
                    auto b1 = to_byte(data[10]);
                    auto b2 = to_byte(data[11]);
                    data.remove_prefix(12); // rgb:00/00/00
                    return { (r1 << 4 ) + (r2      )
                           + (g1 << 12) + (g2 << 8 )
                           + (b1 << 20) + (b2 << 16)
                           + 0xFF000000 };
                }
                return {};
            }
            void notsupported(text const& property, view data)
            {
                log("%%Not supported: OSC=%property% DATA=%data% SIZE=%length% HEX=%hexdata%", prompt::term, property, data, data.length(), utf::to_hex(data));
            }

            c_tracking(term& owner)
                : owner{ owner }
            {
                reset();
                procs[ansi::osc_linux_color] = [&](view data) // ESC ] P Nrrggbb
                {
                    if (data.length() >= 7)
                    {
                        auto n  = to_byte(data[0]);
                        auto r1 = to_byte(data[1]);
                        auto r2 = to_byte(data[2]);
                        auto g1 = to_byte(data[3]);
                        auto g2 = to_byte(data[4]);
                        auto b1 = to_byte(data[5]);
                        auto b2 = to_byte(data[6]);
                        color[n] = (r1 << 4 ) + (r2      )
                                 + (g1 << 12) + (g2 << 8 )
                                 + (b1 << 20) + (b2 << 16)
                                 + 0xFF000000;
                    }
                };
                procs[ansi::osc_reset_color] = [&](view data) // ESC ] 104 ; 0; 1;...
                {
                    auto empty = true;
                    while (data.length())
                    {
                        utf::trim_front_if(data, [](char c){ return c >= '0' && c <= '9'; });
                        if (auto v = utf::to_int(data))
                        {
                            auto n = std::clamp(v.value(), 0, 255);
                            color[n] = rgba::vt256[n];
                            empty = faux;
                        }
                    }
                    if (empty) reset();
                };
                procs[ansi::osc_set_palette] = [&](view data) // ESC ] 4 ; 0;rgb:00/00/00;1;rgb:00/00/00;...
                {
                    auto fails = faux;
                    while (data.length())
                    {
                        utf::trim_front(data, " ;");
                        if (auto v = utf::to_int(data))
                        {
                            auto n = std::clamp(v.value(), 0, 255);
                            if (auto r = record(data))
                            {
                                color[n] = r.value();
                            }
                            else
                            {
                                fails = true;
                                break;
                            }
                        }
                        else
                        {
                            fails = true;
                            break;
                        }
                    }
                    if (fails) notsupported(ansi::osc_set_palette, data);
                };
                procs[ansi::osc_linux_reset] = [&](view data) // ESC ] R
                {
                    reset();
                };
                procs[ansi::osc_set_fgcolor] = [&](view data) // ESC ] 10 ;rgb:00/00/00
                {
                    if (auto r = record(data))
                    {
                        owner.target->brush.sfg(r.value());
                    }
                    else notsupported(ansi::osc_set_fgcolor, data);
                };
                procs[ansi::osc_set_bgcolor] = [&](view data) // ESC ] 11 ;rgb:00/00/00
                {
                    if (auto r = record(data))
                    {
                        owner.target->brush.sbg(r.value());
                    }
                    else notsupported(ansi::osc_set_bgcolor, data);
                };
                procs[ansi::osc_reset_fgclr] = [&](view data)
                {
                    owner.target->brush.sfg(0);
                };
                procs[ansi::osc_reset_bgclr] = [&](view data)
                {
                    owner.target->brush.sbg(0);
                };
            }

            void set(text const& property, view data)
            {
                auto proc = procs.find(property);
                if (proc != procs.end())
                {
                    proc->second(data);
                }
                else log("%%Not supported: OSC=%property% DATA=%data% HEX=%hexdata%", prompt::term, property, data, utf::to_hex(data));
            }
            void fgc(tint c) { owner.target->brush.fgc(color[c]); }
            void bgc(tint c) { owner.target->brush.bgc(color[c]); }
        };

        // term: Generic terminal buffer.
        struct bufferbase
            : public ansi::parser
        {
            static void set_autocr(bool autocr)
            {
                #define V [](auto& q, auto& p)
                auto& parser = ansi::get_parser<bufferbase>();
                autocr ? parser.intro[ansi::ctrl::eol] = V{ p->cr(); p->lf(q.pop_all(ansi::ctrl::eol)); }
                       : parser.intro[ansi::ctrl::eol] = V{          p->lf(q.pop_all(ansi::ctrl::eol)); };
                #undef V
            }
            template<class T>
            static void parser_config(T& vt)
            {
                using namespace netxs::ansi;
                #define V [](auto& q, auto& p)
                vt.csier.table_space[csi_spc_src] = V{ p->na("CSI n SP A  Shift right n columns(s)."); }; // CSI n SP A  Shift right n columns(s).
                vt.csier.table_space[csi_spc_slc] = V{ p->na("CSI n SP @  Shift left  n columns(s)."); }; // CSI n SP @  Shift left n columns(s).
                vt.csier.table_space[csi_spc_cst] = V{ p->owner.cursor.style(q(1)); }; // CSI n SP q  Set cursor style (DECSCUSR).
                vt.csier.table_hash [csi_hsh_scp] = V{ p->na("CSI n # P  Push current palette colors onto stack. n default is 0."); }; // CSI n # P  Push current palette colors onto stack. n default is 0.
                vt.csier.table_hash [csi_hsh_rcp] = V{ p->na("CSI n # Q  Pop  current palette colors onto stack. n default is 0."); }; // CSI n # Q  Pop  current palette colors onto stack. n default is 0.
                vt.csier.table_hash [csi_hsh_psh] = V{ p->pushsgr(); }; // CSI # {  Push current SGR attributes onto stack.
                vt.csier.table_hash [csi_hsh_pop] = V{ p->popsgr();  }; // CSI # }  Pop  current SGR attributes from stack.
                vt.csier.table_excl [csi_exl_rst] = V{ p->owner.decstr( ); }; // CSI ! p  Soft terminal reset (DECSTR).

                vt.csier.table_dollarsn[csi_dlr_fra] = V{ p->fra(q); }; // CSI Char ; Top ; Left ; Bottom ; Right $ x  — Fill rectangular area (DECFRA).

                vt.csier.table[csi_sgr][sgr_fg_blk   ] = V{ p->owner.ctrack.fgc(tint::blackdk  ); };
                vt.csier.table[csi_sgr][sgr_fg_red   ] = V{ p->owner.ctrack.fgc(tint::reddk    ); };
                vt.csier.table[csi_sgr][sgr_fg_grn   ] = V{ p->owner.ctrack.fgc(tint::greendk  ); };
                vt.csier.table[csi_sgr][sgr_fg_ylw   ] = V{ p->owner.ctrack.fgc(tint::yellowdk ); };
                vt.csier.table[csi_sgr][sgr_fg_blu   ] = V{ p->owner.ctrack.fgc(tint::bluedk   ); };
                vt.csier.table[csi_sgr][sgr_fg_mgt   ] = V{ p->owner.ctrack.fgc(tint::magentadk); };
                vt.csier.table[csi_sgr][sgr_fg_cyn   ] = V{ p->owner.ctrack.fgc(tint::cyandk   ); };
                vt.csier.table[csi_sgr][sgr_fg_wht   ] = V{ p->owner.ctrack.fgc(tint::whitedk  ); };
                vt.csier.table[csi_sgr][sgr_fg_blk_lt] = V{ p->owner.ctrack.fgc(tint::blacklt  ); };
                vt.csier.table[csi_sgr][sgr_fg_red_lt] = V{ p->owner.ctrack.fgc(tint::redlt    ); };
                vt.csier.table[csi_sgr][sgr_fg_grn_lt] = V{ p->owner.ctrack.fgc(tint::greenlt  ); };
                vt.csier.table[csi_sgr][sgr_fg_ylw_lt] = V{ p->owner.ctrack.fgc(tint::yellowlt ); };
                vt.csier.table[csi_sgr][sgr_fg_blu_lt] = V{ p->owner.ctrack.fgc(tint::bluelt   ); };
                vt.csier.table[csi_sgr][sgr_fg_mgt_lt] = V{ p->owner.ctrack.fgc(tint::magentalt); };
                vt.csier.table[csi_sgr][sgr_fg_cyn_lt] = V{ p->owner.ctrack.fgc(tint::cyanlt   ); };
                vt.csier.table[csi_sgr][sgr_fg_wht_lt] = V{ p->owner.ctrack.fgc(tint::whitelt  ); };
                vt.csier.table[csi_sgr][sgr_bg_blk   ] = V{ p->owner.ctrack.bgc(tint::blackdk  ); };
                vt.csier.table[csi_sgr][sgr_bg_red   ] = V{ p->owner.ctrack.bgc(tint::reddk    ); };
                vt.csier.table[csi_sgr][sgr_bg_grn   ] = V{ p->owner.ctrack.bgc(tint::greendk  ); };
                vt.csier.table[csi_sgr][sgr_bg_ylw   ] = V{ p->owner.ctrack.bgc(tint::yellowdk ); };
                vt.csier.table[csi_sgr][sgr_bg_blu   ] = V{ p->owner.ctrack.bgc(tint::bluedk   ); };
                vt.csier.table[csi_sgr][sgr_bg_mgt   ] = V{ p->owner.ctrack.bgc(tint::magentadk); };
                vt.csier.table[csi_sgr][sgr_bg_cyn   ] = V{ p->owner.ctrack.bgc(tint::cyandk   ); };
                vt.csier.table[csi_sgr][sgr_bg_wht   ] = V{ p->owner.ctrack.bgc(tint::whitedk  ); };
                vt.csier.table[csi_sgr][sgr_bg_blk_lt] = V{ p->owner.ctrack.bgc(tint::blacklt  ); };
                vt.csier.table[csi_sgr][sgr_bg_red_lt] = V{ p->owner.ctrack.bgc(tint::redlt    ); };
                vt.csier.table[csi_sgr][sgr_bg_grn_lt] = V{ p->owner.ctrack.bgc(tint::greenlt  ); };
                vt.csier.table[csi_sgr][sgr_bg_ylw_lt] = V{ p->owner.ctrack.bgc(tint::yellowlt ); };
                vt.csier.table[csi_sgr][sgr_bg_blu_lt] = V{ p->owner.ctrack.bgc(tint::bluelt   ); };
                vt.csier.table[csi_sgr][sgr_bg_mgt_lt] = V{ p->owner.ctrack.bgc(tint::magentalt); };
                vt.csier.table[csi_sgr][sgr_bg_cyn_lt] = V{ p->owner.ctrack.bgc(tint::cyanlt   ); };
                vt.csier.table[csi_sgr][sgr_bg_wht_lt] = V{ p->owner.ctrack.bgc(tint::whitelt  ); };

                vt.csier.table[csi_cuu] = V{ p-> up(q(1)); }; // CSI n A  (CUU)
                vt.csier.table[csi_cud] = V{ p-> dn(q(1)); }; // CSI n B  (CUD)
                vt.csier.table[csi_cuf] = V{ p->cuf(q(1)); }; // CSI n C  (CUF)  Negative values can wrap to the prev line.
                vt.csier.table[csi_cub] = V{ p->cub(q(1)); }; // CSI n D  (CUB)  Negative values can wrap to the next line.

                vt.csier.table[csi_cht]           = V{ p->tab( q(1)); }; // CSI n I  Caret forward  n tabs, default n=1.
                vt.csier.table[csi_cbt]           = V{ p->tab(-q(1)); }; // CSI n Z  Caret backward n tabs, default n=1.
                vt.csier.table[csi_tbc]           = V{ p->tbc( q(0)); }; // CSI n g  Clear tabstops, default n=0.
                vt.csier.table[csi_rep]           = V{ p->rep( q(1)); }; // CSI n b  Repeat the preceding character n times, default n=1.
                vt.csier.table_quest[csi_qst_rtb] = V{ p->rtb(     ); }; // CSI ? W  Reset tabstops to the 8 column defaults.
                vt.intro[ctrl::esc][esc_hts]      = V{ p->stb(     ); }; // ESC H    Place tabstop at the current column.

                vt.csier.table[csi_cud2]= V{ p->dn ( q(1)); }; // CSI n e  Vertical position relative. Move cursor down (VPR).

                vt.csier.table[csi_cnl] = V{ p->cr(); p->dn(q(1)); }; // CSI n E  Move n lines down and to the leftmost column.
                vt.csier.table[csi_cpl] = V{ p->cr(); p->up(q(1)); }; // CSI n F  Move n lines up   and to the leftmost column.
                vt.csier.table[csi_chx] = V{ p->chx( q(1)); }; // CSI n G  Move cursor hz absolute.
                vt.csier.table[csi_chy] = V{ p->chy( q(1)); }; // CSI n d  Move cursor vt absolute.
                vt.csier.table[csi_cup] = V{ p->cup( q   ); }; // CSI y ; x H (1-based)
                vt.csier.table[csi_hvp] = V{ p->cup( q   ); }; // CSI y ; x f (1-based)

                vt.csier.table[csi_dch] = V{ p->dch( q(1)); };  // CSI n P  Delete n chars (DCH).
                vt.csier.table[csi_ech] = V{ p->ech( q(1)); };  // CSI n X  Erase n chars (ECH).
                vt.csier.table[csi_ich] = V{ p->ins( q(1)); };  // CSI n @  Insert n chars (ICH).

                vt.csier.table[csi__ed] = V{ p-> ed( q(0)); }; // CSI n J
                vt.csier.table[csi__el] = V{ p-> el( q(0)); }; // CSI n K
                vt.csier.table[csi__il] = V{ p-> il( q(1)); }; // CSI n L  Insert n lines (IL).
                vt.csier.table[csi__dl] = V{ p-> dl( q(1)); }; // CSI n M  Delete n lines (DL).
                vt.csier.table[csi__sd] = V{ p->scl( q(1)); }; // CSI n T  Scroll down by n lines, scrolled out lines are lost.
                vt.csier.table[csi__su] = V{ p->scl(-q(1)); }; // CSI n S  Scroll   up by n lines, scrolled out lines are pushed to the scrollback.
                vt.csier.table[csi_scp] = V{ p->scp(     ); }; // CSI   s  Save cursor position.
                vt.csier.table[csi_rcp] = V{ p->rcp(     ); }; // CSI   u  Restore cursor position.

                vt.csier.table[decstbm] = V{ p->scr( q   ); }; // CSI r; b r  Set scrolling region (t/b: top+bottom).

                vt.csier.table[csi_win] = V{ p->owner.wtrack.manage(q   ); }; // CSI n;m;k t  Terminal window options (XTWINOPS).
                vt.csier.table[csi_dsr] = V{ p->owner.wtrack.report(q(6)); }; // CSI n n  Device status report (DSR).
                vt.csier.table[csi_pda] = V{ p->owner.wtrack.device(q(0)); }; // CSI n c  Send device attributes (Primary DA).

                vt.csier.table[csi_ccc][ccc_sbs] = V{ p->owner.sbsize(q); }; // CCC_SBS: Set scrollback size.
                vt.csier.table[csi_ccc][ccc_rst] = V{ p->owner.setdef();  }; // CCC_RST: Reset to defaults.
                vt.csier.table[csi_ccc][ccc_sgr] = V{ p->owner.setsgr(q); }; // CCC_SGR: Set default SGR.
                vt.csier.table[csi_ccc][ccc_lsr] = V{ p->owner.setlsr(q(1)); };           // CCC_LSR: Enable line style reporting.
                vt.csier.table[csi_ccc][ccc_sel] = V{ p->owner.selection_selmod(q(0)); }; // CCC_SEL: Set selection mode.
                vt.csier.table[csi_ccc][ccc_pad] = V{ p->setpad(q(-1)); };                // CCC_PAD: Set left/right padding for scrollback.

                vt.intro[ctrl::esc][esc_ind   ] = V{ p->lf(1); };          // ESC D  Index. Caret down and scroll if needed (IND).
                vt.intro[ctrl::esc][esc_ir    ] = V{ p->ri();  };          // ESC M  Reverse index (RI).
                vt.intro[ctrl::esc][esc_sc    ] = V{ p->scp(); };          // ESC 7  (same as CSI s) Save cursor position.
                vt.intro[ctrl::esc][esc_rc    ] = V{ p->rcp(); };          // ESC 8  (same as CSI u) Restore cursor position.
                vt.intro[ctrl::esc][esc_ris   ] = V{ p->owner.decstr(); }; // ESC c  Reset to initial state (same as DECSTR).
                vt.intro[ctrl::esc][esc_nel   ] = V{ p->cr(); p->dn(1); }; // ESC E  Move cursor down and CR. Same as CSI 1 E
                vt.intro[ctrl::esc][esc_decdhl] = V{ p->dhl(q); };         // ESC # ...  ESC # 3, ESC # 4, ESC # 5, ESC # 6, ESC # 8

                vt.intro[ctrl::esc][esc_apc   ] = V{ p->msg(esc_apc, q); }; // ESC _ ... ST  APC.
                vt.intro[ctrl::esc][esc_dsc   ] = V{ p->msg(esc_dsc, q); }; // ESC P ... ST  DSC.
                vt.intro[ctrl::esc][esc_sos   ] = V{ p->msg(esc_sos, q); }; // ESC X ... ST  SOS.
                vt.intro[ctrl::esc][esc_pm    ] = V{ p->msg(esc_pm , q); }; // ESC ^ ... ST  PM.

                vt.intro[ctrl::bs ] = V{ p->cub(q.pop_all(ctrl::bs )); };
                vt.intro[ctrl::del] = V{ p->del(q.pop_all(ctrl::del)); }; // Move backward and delete character under cursor with wrapping.
                vt.intro[ctrl::tab] = V{ p->tab(q.pop_all(ctrl::tab)); };
                vt.intro[ctrl::eol] = V{ p-> lf(q.pop_all(ctrl::eol)); }; // LF
                vt.intro[ctrl::vt ] = V{ p-> lf(q.pop_all(ctrl::vt )); }; // VT same as LF
                vt.intro[ctrl::ff ] = V{ p-> lf(q.pop_all(ctrl::ff )); }; // FF same as LF
                vt.intro[ctrl::cr ] = V{ p-> cr();                     }; // CR

                vt.csier.table_quest[dec_set] = V{ p->owner.decset(q); };
                vt.csier.table_quest[dec_rst] = V{ p->owner.decrst(q); };

                vt.oscer[osc_label_title] = V{ p->owner.wtrack.set(osc_label_title, q); };
                vt.oscer[osc_label      ] = V{ p->owner.wtrack.set(osc_label,       q); };
                vt.oscer[osc_title      ] = V{ p->owner.wtrack.set(osc_title,       q); };
                vt.oscer[osc_xprop      ] = V{ p->owner.wtrack.set(osc_xprop,       q); };
                vt.oscer[osc_linux_color] = V{ p->owner.ctrack.set(osc_linux_color, q); };
                vt.oscer[osc_linux_reset] = V{ p->owner.ctrack.set(osc_linux_reset, q); };
                vt.oscer[osc_set_palette] = V{ p->owner.ctrack.set(osc_set_palette, q); };
                vt.oscer[osc_set_fgcolor] = V{ p->owner.ctrack.set(osc_set_fgcolor, q); };
                vt.oscer[osc_set_bgcolor] = V{ p->owner.ctrack.set(osc_set_bgcolor, q); };
                vt.oscer[osc_reset_color] = V{ p->owner.ctrack.set(osc_reset_color, q); };
                vt.oscer[osc_reset_fgclr] = V{ p->owner.ctrack.set(osc_reset_fgclr, q); };
                vt.oscer[osc_reset_bgclr] = V{ p->owner.ctrack.set(osc_reset_bgclr, q); };
                vt.oscer[osc_clipboard  ] = V{ p->owner.forward_clipboard(q);           };
                #undef V

                // Log all unimplemented CSI commands.
                for (auto i = 0; i < 0x100; ++i)
                {
                    auto& proc = vt.csier.table[i];
                    if (!proc)
                    {
                        proc = [i](auto& q, auto& p) { p->not_implemented_CSI(i, q); };
                    }
                }
                auto& esc_lookup = vt.intro[ctrl::esc];
                // Log all unimplemented ESC+rest.
                for (auto i = 0; i < 0x100; ++i)
                {
                    auto& proc = esc_lookup[i];
                    if (!proc)
                    {
                        proc = [i](auto& q, auto& p) { p->not_implemented_ESC(i, q); };
                    }
                }
            }

            using tabs = std::vector<std::pair<si32, si32>>; // Pairs of forward and reverse tabstops index.

            struct line
                : public rich
            {
                using rich::rich;
                using type = deco::type;
                using id_t = ui32;

                line(line&& l)
                    : rich { std::forward<rich>(l) },
                      index{ l.index }
                {
                    style = l.style;
                    _size = l._size;
                    _kind = l._kind;
                    l._size = {};
                    l._kind = {};
                }
                line(line const& l)
                    : rich { l       },
                      index{ l.index },
                      style{ l.style }
                { }
                line(id_t newid, deco const& style = {})
                    : index{ newid },
                      style{ style }
                { }
                line(id_t newid, deco const& style, span dt, twod sz)
                    : rich { dt,sz },
                      index{ newid },
                      style{ style }
                { }
                line(id_t newid, deco const& style, cell const& blank, si32 length)
                    : rich { blank, length },
                      index{ newid },
                      style{ style }
                { }
                line(core&& s)
                    : rich{ std::forward<core>(s) }
                { }
                line(netxs::view utf8)
                    : rich{ para{ utf8 }.content() }
                { }

                line& operator = (line&&)      = default;
                line& operator = (line const&) = default;

                id_t index{};
                deco style{};
                si32 _size{};
                type _kind{};

                friend void swap(line& lhs, line& rhs)
                {
                    std::swap<rich>(lhs, rhs);
                    std::swap(lhs.index, rhs.index);
                    std::swap(lhs.style, rhs.style);
                    std::swap(lhs._size, rhs._size);
                    std::swap(lhs._kind, rhs._kind);
                }
                void wipe()
                {
                    rich::kill();
                    _size = {};
                    _kind = {};
                }
                bool wrapped() const
                {
                    assert(_kind == style.get_kind());
                    return _kind == type::autowrap;
                }
                si32 height(si32 width) const
                {
                    auto len = length();
                    assert(_kind == style.get_kind());
                    return len > width && wrapped() ? (len + width - 1) / width
                                                    : 1;
                }
            };
            struct redo
            {
                using mark = ansi::mark;
                using sgrs = std::list<mark>;

                deco style{}; // Parser style state.
                mark brush{}; // Parser brush state.
                twod coord{}; // Screen coord state.
                bool decom{}; // Origin mode  state.
                sgrs stack{}; // Stach for saved sgr attributes.
            };

            term& owner; // bufferbase: Terminal object reference.
            twod  panel; // bufferbase: Viewport size.
            twod  coord; // bufferbase: Viewport cursor position; 0-based.
            redo  saved; // bufferbase: Saved cursor position and rendition state.
            si32  arena; // bufferbase: Scrollable region height.
            si32  sctop; // bufferbase: Precalculated scrolling region top    height.
            si32  scend; // bufferbase: Precalculated scrolling region bottom height.
            si32  y_top; // bufferbase: Precalculated 0-based scrolling region top    vertical pos.
            si32  y_end; // bufferbase: Precalculated 0-based scrolling region bottom vertical pos.
            si32  n_top; // bufferbase: Original      1-based scrolling region top    vertical pos (use 0 if it is not set).
            si32  n_end; // bufferbase: Original      1-based scrolling region bottom vertical pos (use 0 if it is not set).
            tabs  stops; // bufferbase: Tabstop index.
            bool  notab; // bufferbase: Tabstop index is cleared.
            bool  decom; // bufferbase: Origin mode.

            bool  boxed; // bufferbase: Box selection mode.
            bool  grant; // bufferbase: Is it allowed to change box selection mode.
            ui64  alive; // bufferbase: Selection is active (digest).
            line  match; // bufferbase: Search pattern for highlighting.
            bool  uirev; // bufferbase: Prev button highlighted.
            bool  uifwd; // bufferbase: Next button highlighted.

            bufferbase(term& master)
                : owner{ master },
                  panel{ dot_11 },
                  coord{ dot_00 },
                  arena{ 1      },
                  sctop{ 0      },
                  scend{ 0      },
                  y_top{ 0      },
                  y_end{ 0      },
                  n_top{ 0      },
                  n_end{ 0      },
                  notab{ faux   },
                  decom{ faux   },
                  boxed{ faux   },
                  grant{ faux   },
                  uirev{ faux   },
                  uifwd{ faux   },
                  alive{ 0      }
            {
                parser::style = ansi::def_style;
            }

            // bufferbase: Make a viewport screen copy.
            virtual void do_viewport_copy(face& dest) = 0;

            virtual void selection_create(twod coor, bool mode)           = 0;
            virtual bool selection_extend(twod coor, bool mode)           = 0;
            virtual void selection_follow(twod coor, bool lock)           = 0;
            virtual void selection_byword(twod coor)                      = 0;
            virtual void selection_byline(twod coor)                      = 0;
            virtual text selection_pickup(si32 selmod)                    = 0;
            virtual void selection_render(face& dest)                     = 0;
            virtual void selection_status(term_state& status) const       = 0;
            virtual twod selection_gonext(feed direction)                 = 0;
            virtual twod selection_gofind(feed direction, view data = {}) = 0;
            virtual twod selection_search(feed direction, view data = {})
            {
                auto delta = dot_00;
                if (data.empty()) // Find next selection match.
                {
                    if (match.length())
                    {
                        delta = selection_gonext(direction);
                    }
                }
                else
                {
                    delta = selection_gofind(direction, data);
                }
                return delta;
            }
            virtual void selection_setjet(bias align)
            {
                // Do nothing by default.
            }
            virtual void selection_setwrp(wrap w = {})
            {
                // Do nothing by default.
            }
            // bufferbase: Cancel text selection.
            virtual bool selection_cancel()
            {
                auto active = alive;
                if (alive)
                {
                    alive = {};
                    match = {};
                }
                return active;
            }
            // bufferbase: Set selection mode lock state.
            void selection_locked(bool lock)
            {
                grant = !lock;
            }
            // bufferbase: Return selection mode lock state.
            auto selection_locked()
            {
                return !grant;
            }
            // bufferbase: Ping selection state.
            virtual void selection_update(bool despace = true)
            {
                if (despace) // Exclude whitespce.
                {
                    auto empty = match.each([](auto& c) { return !c.isspc(); });
                    if (empty) match = {};
                }
                ++alive;
            }
            // bufferbase: Ping selection state if is available.
            void selection_review()
            {
                if (alive) ++alive;
            }
            // bufferbase: Return true if selection is active.
            auto selection_active() const
            {
                return alive;
            }
            // bufferbase: Set selection mode (boxed = true).
            void selection_selbox(bool state)
            {
                if (state) grant = true;
                if (grant) boxed = state;
            }
            // bufferbase: Return selection mode.
            bool selection_selbox() const
            {
                return boxed;
            }

            virtual void scroll_region(si32 top, si32 end, si32 n, bool use_scrollback) = 0;
            virtual bool recalc_pads(dent& oversz)                                      = 0;
            virtual void output(face& canvas)                                           = 0;
            virtual si32 height()                                                       = 0;
            virtual void del_above()                                                    = 0;
            virtual void del_below()                                                    = 0;
            virtual si32 get_size() const                                               = 0;
            virtual si32 get_peak() const                                               = 0;
            virtual si32 get_step() const                                               = 0;
            virtual si32 get_mxsz() const                                               = 0;
                    auto get_view() const { return panel; }
                    auto get_mode() const { return !selection_active() ? term_state::type::empty:
                                                    selection_selbox() ? term_state::type::block:
                                                                         term_state::type::lines; }
            // bufferbase: Get viewport position.
    virtual si32 get_origin(bool follow)
            {
                return 0;
            }
            // bufferbase: Get viewport basis.
    virtual si32 get_basis()
            {
                return 0;
            }
            // bufferbase: Get viewport position.
    virtual si32 get_slide()
            {
                return 0;
            }
            // bufferbase: Set viewport position and return whether the viewport is reset.
    virtual bool set_slide(si32&)
            {
                return true;
            }
            // bufferbase: Set left/right scrollback additional padding.
    virtual void setpad(si32 new_value)
            { }
            // bufferbase: Get left/right scrollback additional padding.
    virtual si32 getpad()
            {
                return 0;
            }
            // bufferbase: Update scrolling region.
            void update_region()
            {
                sctop = std::max(0, n_top - 1);
                scend = n_end != 0 ? std::max(1, panel.y - n_end)
                                   : 0;

                auto y_max = panel.y - 1;
                y_end = std::clamp(y_max - scend, 0, y_max);
                y_top = std::clamp(sctop        , 0, y_end);
            }
            // bufferbase: Resize viewport.
    virtual void resize_viewport(twod new_sz, bool forced = faux)
            {
                panel = std::max(new_sz, dot_11);
                resize_tabstops(panel.x);
                update_region();
                selection_review();
                arena = y_end - y_top + 1; // Can be changed at the scrollbuff::set_scroll_region(si32 top, si32 bottom).
            }
            // bufferbase: Reset coord and set the scrolling region using 1-based top and bottom. Use 0 to reset.
    virtual void set_scroll_region(si32 top, si32 bottom)
            {
                // Sanity check.
                top    = std::clamp(top,    0, panel.y);
                bottom = std::clamp(bottom, 0, panel.y);
                if (top    != 0 &&
                    bottom != 0 && top >= bottom)
                    //bottom != 0 && top > bottom) top = bottom; //todo Nobody respects that.
                {
                    top = bottom = 0;
                }

                n_top = top    == 1       ? 0 : top;
                n_end = bottom == panel.y ? 0 : bottom;
                update_region();
                cup0(dot_00);
            }
            // bufferbase: Set cursor position.
    virtual void set_coord(twod new_coord)
            {
                coord = new_coord;
            }
            // bufferbase: Return current 0-based cursor position in the viewport.
    virtual twod get_coord(twod origin)
            {
                return coord;
            }
            // bufferbase: Base-CSI contract (see ansi::csi_t).
            //             task(...), meta(...), data(...)
            void task(ansi::rule property)
            {
                parser::flush();
                log(prompt::term, "Desktopio vt-extensions are not supported");
                //auto& cur_line = batch.current();
                //if (cur_line.busy())
                //{
                //    add_lines(1);
                //    batch.index(batch.length() - 1);
                //}
                //batch->locus.push(property);
            }
            // bufferbase: Update current SGR attributes.
    virtual void meta(deco const& old_style) override
            {
                auto changed = faux;
                if (parser::style.wrp() != old_style.wrp())
                {
                    auto status = parser::style.wrp() == wrap::none ? (si32)owner.config.def_wrpmod
                                                                    : (si32)parser::style.wrp();
                    owner.SIGNAL(tier::release, ui::term::events::layout::wrapln, status);
                    changed = true;
                }
                if (parser::style.jet() != old_style.jet())
                {
                    auto status = parser::style.jet() == bias::none ? (si32)bias::left
                                                                    : (si32)parser::style.jet();
                    owner.SIGNAL(tier::release, ui::term::events::layout::align, status);
                    changed = true;
                }
                if (changed && owner.styled)
                {
                    owner.ipccon.style(parser::style, owner.kbmode);
                }
            }
            template<class T>
            void na(T&& note)
            {
                log(prompt::term, "Not implemented: ", note);
            }
            void not_implemented_CSI(si32 i, fifo& q)
            {
                auto params = text{};
                while (q)
                {
                    params += std::to_string(q(0));
                    if (q)
                    {
                        auto is_sub_arg = q.issub(q.front());
                        auto delim = is_sub_arg ? ':' : ';';
                        params.push_back(delim);
                    }
                }
                log("%%CSI %params% %char%, (%val%) is not implemented", prompt::term, params, (unsigned char)i, i);
            }
            void not_implemented_ESC(si32 c, qiew& q)
            {
                switch (c)
                {
                    // Unexpected
                    case ansi::esc_csi   :
                    case ansi::esc_ocs   :
                    case ansi::esc_dsc   :
                    case ansi::esc_sos   :
                    case ansi::esc_pm    :
                    case ansi::esc_apc   :
                    case ansi::esc_st    :
                        log("%%ESC %char% (%val%) is unexpected", prompt::term, (char)c, c);
                        break;
                    // Unsupported ESC + byte + rest
                    case ansi::esc_g0set :
                    case ansi::esc_g1set :
                    case ansi::esc_g2set :
                    case ansi::esc_g3set :
                    case ansi::esc_g1xset:
                    case ansi::esc_g2xset:
                    case ansi::esc_g3xset:
                    case ansi::esc_ctrl  :
                    case ansi::esc_decdhl:
                    case ansi::esc_chrset:
                    {
                        if (!q) log("%%ESC %char% (%val%) is incomplete", prompt::term, (char)c, c);
                        auto b = q.front();
                        q.pop_front();
                        switch (b)
                        {
                            case 'B':
                            case 'A':
                            case '0':
                            case '1':
                            case '2':
                            case '<':
                            case '4':
                            case '5':
                            case 'C':
                            case 'R':
                            case 'f':
                            case 'Q':
                            case 'K':
                            case 'Y':
                            case 'E':
                            case '6':
                            case 'Z':
                            case '7':
                            case 'H':
                            case '=':
                            case '>':
                            case '9':
                            case '`':
                            case 'U':
                                log("%%ESC %char% %char% (%val% %val%) is unsupported", prompt::term, (char)c, (char)b, c, b);
                                break;
                            case '%':
                            case '"':
                            {
                                if (q.size() < 2)
                                {
                                    if (q) q.pop_front();
                                    log("%%ESC %char% %char% (%val% %val%) is incomplete", prompt::term, (char)c, (char)b, c, b);
                                }
                                else
                                {
                                     auto d = q.front();
                                     q.pop_front();
                                     log("%%ESC %char% %char% %char% (%val% %val% %val%) is unsupported", prompt::term, (char)c, (char)b, (char)d, c, b, d);
                                }
                                break;
                            }
                            default:
                                log("%%ESC %char% %char% (%val% %val%) is unknown", prompt::term, (char)c, (char)b, c, b);
                                break;
                        }
                        break;
                    }
                    // Unsupported ESC + byte
                    case ansi::esc_delim :
                    case ansi::esc_key_a :
                    case ansi::esc_key_n :
                    case ansi::esc_decbi :
                    case ansi::esc_decfi :
                    case ansi::esc_sc    :
                    case ansi::esc_rc    :
                    case ansi::esc_hts   :
                    case ansi::esc_nel   :
                    case ansi::esc_clb   :
                    case ansi::esc_ind   :
                    case ansi::esc_ir    :
                    case ansi::esc_ris   :
                    case ansi::esc_memlk :
                    case ansi::esc_munlk :
                    case ansi::esc_ls2   :
                    case ansi::esc_ls3   :
                    case ansi::esc_ls1r  :
                    case ansi::esc_ls2r  :
                    case ansi::esc_ls3r  :
                    case ansi::esc_ss3   :
                    case ansi::esc_ss2   :
                    case ansi::esc_spa   :
                    case ansi::esc_epa   :
                    case ansi::esc_rid   :
                        log("%%ESC %char% (%val%) is unsupported", prompt::term, (char)c, c);
                        break;
                    default:
                        log("%%ESC %char% (%val%) is unknown", prompt::term, (char)c, c);
                        break;
                }
            }
            void dhl(qiew& q)
            {
                parser::flush();
                auto c = q ? q.front()
                           : -1;
                if (q) q.pop_front();
                switch (c)
                {
                    case -1:
                        log(prompt::term, "ESC #  is unexpected");
                        break;
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                        log("%%ESC # %char% (%val%) is unsupported", prompt::term, (char)c, c);
                        break;
                    case '8':
                    {
                        set_coord(dot_00);
                        auto y = 0;
                        while (++y <= panel.y) // Fill viewport with 'E'.
                        {
                            chy(y);
                            ech(panel.x, 'E');
                        }
                        set_coord(dot_00);
                        break;
                    }
                    default:
                        log("%%ESC # %char% (%val%) is unknown", prompt::term, (char)c, c);
                        break;
                }
            }
            void msg(si32 c, qiew& q)
            {
                parser::flush();
                auto data = text{};
                while (q)
                {
                    auto c = q.front();
                    data.push_back(c);
                    q.pop_front();
                         if (c == ansi::c0_bel) break;
                    else if (c == ansi::c0_esc)
                    {
                        auto c = q.front();
                        if (q && c == '\\')
                        {
                            data.push_back(c);
                            q.pop_front();
                            break;
                        }
                    }
                }
                log("%%Unsupported message/command: '\\e%char%%data%'", prompt::term, (char)c, utf::debase<faux>(data));
            }
            // bufferbase: Clear buffer.
    virtual void clear_all()
            {
                parser::state = {};
                decom = faux;
                rtb();
                selection_cancel();
            }
            // tabstops index, tablen = 3, vector<pair<fwd_idx, rev_idx>>:
            // coor.x      -2-1 0 1 2 3 4 5 6 7 8 9
            // size = 9         0 1 2 3 4 5 6 7 8
            //             ----------------------
            // custom: fwd_idx  3 3 3 6 6 6 9 9 9
            //         rev_idx  0 0 0 3 3 3 6 6 6  coord.x - 1
            //
            // auto:   fwd_idx -3-3-3-6-6-6-9-9-9
            //         rev_idx  0 0 0 3 3 3 6 6 6  coord.x - 1
            //
            // empty:  fwd_idx -9-9-9-9-9-9-9-9-9
            //         rev_idx  0 0 0 0 0 0 0 0 0  coord.x - 1
            //
            // bufferbase: Clear tabstops.
            void clear_tabstops()
            {
                notab = true;
                auto auto_tabs = std::pair{ -panel.x, 0 }; // Negative means auto, not custom.
                stops.assign(panel.x, auto_tabs);
            }
            // bufferbase: Resize tabstop index.
            void resize_tabstops(si32 new_size, bool forced = faux)
            {
                auto size = static_cast<si32>(stops.size());
                if (!forced && new_size <= size) return;

                auto last_stop = si32{};
                if (!stops.empty())
                {
                    auto back = stops.back();
                    auto last_size = back.first > 0 ? 0 // Custom tabstop -- don't touch it.
                                                    : -back.first - back.second;
                    last_stop = forced ? 0
                                       : size - last_size;
                    stops.resize(last_stop); // Trim.
                }

                if (notab) // Preserve existing tabstops.
                {
                    auto auto_tabs = std::pair{ -new_size, last_stop };
                    stops.resize(new_size, auto_tabs);
                }
                else // Add additional default tabstops.
                {
                    stops.reserve(new_size);
                    auto step = owner.config.def_tablen;
                    auto next = last_stop / step * step;
                    auto add_count = new_size - step;
                    while (next < add_count)
                    {
                        auto prev = next;
                        next += step;
                        auto auto_tabs = std::pair{ -next, prev };
                        stops.resize(stops.size() + step, auto_tabs);
                    }
                    auto auto_tabs = std::pair{ -new_size, next };
                    stops.resize(new_size, auto_tabs);
                }
            }
            // bufferbase: ESC H  Place tabstop at the current cursor posistion.
            void stb()
            {
                parser::flush();
                if (coord.x <= 0 || coord.x > owner.config.def_mxline) return;
                resize_tabstops(coord.x);
                auto  coor = coord.x - 1;
                auto  head = stops.begin();
                auto  tail = stops.begin() + coor;
                auto& last = tail->first;
                if (coord.x != last)
                {
                    auto size = static_cast<si32>(stops.size());
                    auto base = last;
                    last = coord.x;
                    while (head != tail)
                    {
                        auto& last = (--tail)->first;
                        if (last == base) last = coord.x;
                        else              break;
                    }
                    if (coord.x < size)
                    {
                        head += coord.x;
                        tail = stops.end();
                        auto& next = head->second;
                        auto  prev = next;
                        next = coord.x;
                        while (++head != tail)
                        {
                            auto& next = head->second;
                            if (next == prev) next = coord.x;
                            else              break;
                        }
                    }
                }
            }
            // bufferbase: (see CSI 0 g) Remove tabstop at the current cursor posistion.
            void remove_tabstop()
            {
                auto  size = static_cast<si32>(stops.size());
                if (coord.x <= 0 || coord.x >= size) return;
                auto  head = stops.begin();
                auto  tail = stops.begin() + coord.x;
                auto  back = tail;
                auto& stop = *tail;
                auto& item = *--tail;
                auto  main = stop.first;
                auto  base = std::abs(item.first);

                if (base == std::abs(main)) return;

                item.first = main;
                while (head != tail)
                {
                    --tail;
                    auto& item = tail->first;
                    if (base == std::abs(item)) item = main;
                    else                        break;
                }

                tail = stops.end();
                base = stop.second;
                main = item.second;
                stop.second = main;
                while (++back != tail)
                {
                    auto& item = back->second;
                    if (base == std::abs(item)) item = main;
                    else                        break;
                }
            }
            // bufferbase: CSI ? W  Reset tabstops to defaults.
            void rtb()
            {
                notab = faux;
                resize_tabstops(panel.x, true);
            }
            // bufferbase: Horizontal tab implementation.
            template<bool Fwd, class T>
            void tab_impl(T size)
            {
                if constexpr (Fwd)
                {
                    auto x = std::clamp(coord.x, 0, size ? size - 1 : 0);
                    if (coord.x == x)
                    {
                        coord.x = std::abs(stops[x].first);
                    }
                    else
                    {
                        coord.x += notab ? owner.config.def_tablen
                                         : owner.config.def_tablen - coord.x % owner.config.def_tablen;
                    }
                }
                else
                {
                    auto x = std::clamp(coord.x, 1, size);
                    if (coord.x == x)
                    {
                        coord.x = stops[x - 1].second;
                    }
                    else
                    {
                        coord.x -= notab ? owner.config.def_tablen
                                         :(owner.config.def_tablen + coord.x - 1) % owner.config.def_tablen + 1;
                    }
                }
            }
            // bufferbase: TAB  Horizontal tab.
    virtual void tab(si32 n)
            {
                parser::flush();
                auto size = static_cast<si32>(stops.size());
                if (n > 0) while (n-- > 0) tab_impl<true>(size);
                else       while (n++ < 0) tab_impl<faux>(size);
            }
            void print_tabstops(text msg)
            {
                log(msg, ":\n", "index size = ", stops.size());
                auto i = 0;
                auto data = utf::adjust("coor:", 5, " ", faux);
                for (auto [fwd, rev] : stops) data += utf::adjust(std::to_string(i++), 4, " ", true);
                data += '\n' + utf::adjust("fwd:", 5, " ", faux);
                for (auto [fwd, rev] : stops) data += utf::adjust(std::to_string(fwd), 4, " ", true);
                data += '\n' + utf::adjust("rev:", 5, " ", faux);
                for (auto [fwd, rev] : stops) data += utf::adjust(std::to_string(rev), 4, " ", true);
                log(data);
            }
            // bufferbase: CSI n g  Reset tabstop value.
            void tbc(si32 n)
            {
                switch (n)
                {
                    case 0: // Remove tab stop from the current column.
                        parser::flush();
                        remove_tabstop();
                        break;
                    case 3: // Clear all tab stops.
                        clear_tabstops();
                        break;
                    default: // Test: print tab stops.
                        print_tabstops("Tabstops index: `CSI " + std::to_string(n) + " g`");
                        break;
                }
            }
            // bufferbase: CSI n b  Repeat prev character n times.
            template<bool Flush = true>
            void rep(si32 n)
            {
                if constexpr (Flush) parser::flush();
                n = std::clamp<si32>(n, 0, si16max);
                if (n)
                {
                    auto c = cell{ parser::brush };
                    if (c.wdt() != 1)
                    {
                        c.wdt(2);
                    }
                    parser::proto.assign(n, c);
                    data(n * c.wdt(), proto);
                    parser::proto.clear();
                }
            }
            // bufferbase: CSI Char ; Top ; Left ; Bottom ; Right $ x  Fill rectangular area (DECFRA).
            void fra(fifo& queue)
            {
                parser::flush();
                auto c = queue(' ');
                auto t = queue(1);
                auto l = queue(1);
                auto b = queue(panel.y);
                auto r = queue(panel.x);
                if (t > b) t = b;
                if (l > r) l = r;
                l -= 1;
                t -= 1;
                auto region = rect{{ l, t }, { r - l, b - t }};
                region.trunc(panel);
                if (c < ' ') c = ' ';
                auto sym = utf::to_utf_from_code(c);
                auto tmp = parser::brush;
                parser::brush.txt(sym);
                if (parser::brush.wdt() != 1)
                {
                    region.size.x /= 2;
                }
                if (region)
                {
                    auto sav = coord;
                    while (region.size.y--)
                    {
                        set_coord(region.coor);
                        rep<faux>(region.size.x);
                        region.coor.y++;
                    }
                    set_coord(sav);
                }
                parser::brush = tmp;
            }
            // bufferbase: CSI # {  Push SGR attributes.
            void pushsgr()
            {
                parser::flush();
                saved.stack.push_back(parser::brush);
                if (saved.stack.size() == 10) saved.stack.pop_front();
            }
            // bufferbase: CSI # }  Pop SGR attributes.
            void popsgr()
            {
                parser::flush();
                if (saved.stack.size())
                {
                    parser::brush = saved.stack.back();
                    saved.stack.pop_back();
                }
            }
            // bufferbase: ESC 7 or CSI s  Save cursor position.
            void scp()
            {
                parser::flush();
                saved = { .style = parser::style,
                          .brush = parser::brush,
                          .coord = coord,
                          .decom = decom };
                if (decom) saved.coord.y -= y_top;
                assert(saved.coord.y >= 0);
            }
            // bufferbase: ESC 8 or CSI u  Restore cursor position.
            void rcp()
            {
                parser::flush();
                decom = saved.decom;
                auto coor = saved.coord;
                if (decom) coor.y += y_top;
                set_coord(coor);
                parser::style = saved.style;
                parser::brush = saved.brush;
                parser::flush(); // Proceed new style.
            }
            // bufferbase: CSI n T/S  Scroll down/up, scrolled up lines are pushed to the scrollback buffer.
    virtual void scl(si32 n)
            {
                parser::flush();
                scroll_region(y_top, y_end, n, n > 0 ? faux : true);
            }
            // bufferbase: CSI n L  Insert n lines. Place cursor to the begining of the current.
    virtual void il(si32 n)
            {
                parser::flush();
               /* Works only if cursor is in the scroll region.
                * Inserts n lines at the current row and removes n lines at the scroll bottom.
                */
                if (n > 0 && coord.y >= y_top
                          && coord.y <= y_end)
                {
                    scroll_region(coord.y, y_end, n, faux);
                    coord.x = 0;
                }
            }
            // bufferbase: CSI n M  Delete n lines. Place cursor to the begining of the current.
    virtual void dl(si32 n)
            {
                parser::flush();
               /* Works only if cursor is in the scroll region.
                * Deletes n lines at the current row and add n lines at the scroll bottom.
                */
                if (n > 0 && coord.y >= y_top
                          && coord.y <= y_end)
                {
                    scroll_region(coord.y, y_end, -n, faux);
                    coord.x = 0;
                }
            }
            // bufferbase: ESC M  Reverse index.
    virtual void ri()
            {
                parser::flush();
               /*
                * Reverse index
                * - move cursor one line up if it is outside of scrolling region or below the top line of scrolling region.
                * - one line scroll down if cursor is on the top line of scroll region.
                */
                if (coord.y != y_top)
                {
                    coord.y--;
                }
                else scroll_region(y_top, y_end, 1, true);
            }
            // bufferbase: CSI t;b r  Set scrolling region (t/b: top+bottom).
            void scr(fifo& queue)
            {
                auto top = queue(0);
                auto end = queue(0);
                set_scroll_region(top, end);
            }
            // bufferbase: CSI n @  ICH. Insert n blanks after cursor. Don't change cursor pos.
    virtual void ins(si32 n) = 0;
            // bufferbase: Shift left n columns(s).
            void shl(si32 n)
            {
                log("%%SHL(%n%) is not implemented.", prompt::term, n);
            }
            // bufferbase: CSI n X  Erase/put n chars after cursor. Don't change cursor pos.
    virtual void ech(si32 n, char c = '\0') = 0;
            // bufferbase: CSI n P  Delete (not Erase) letters under the cursor.
    virtual void dch(si32 n) = 0;
            // bufferbase: '\x7F'  Delete characters backwards.
    virtual void del(si32 n) = 0;
            // bufferbase: Move cursor by n in line.
    virtual void move(si32 n) = 0;
            // bufferbase: Move cursor forward by n.
    virtual void cuf(si32 n)
            {
                if (n < 0) move(n);
                else
                {
                    parser::flush();
                    if (n == 0) n = 1;
                    coord.x += n;
                }
            }
            // bufferbase: Move cursor backward by n.
    virtual void cub(si32 n)
            {
                if (n < 0) move(-n);
                else
                {
                    parser::flush();
                    if (n == 0) n = 1;
                    else if (coord.x == panel.x && parser::style.wrp() == wrap::on && n > 0) ++n;
                    coord.x -= n;
                }
            }
            // bufferbase: CSI n G  Absolute horizontal cursor position (1-based).
    virtual void chx(si32 n)
            {
                parser::flush();
                coord.x = n - 1;
            }
            // bufferbase: CSI n d  Absolute vertical cursor position (1-based).
    virtual void chy(si32 n)
            {
                parser::flush_data();
                --n;
                if (decom) coord.y = std::clamp(n + y_top, y_top, y_end);
                else       coord.y = std::clamp(n, 0, panel.y - 1);
            }
            // bufferbase: Set caret position (0-based).
            void _cup(twod p)
            {
                coord.x = std::clamp(p.x, 0, panel.x - 1);
                if (decom) coord.y = std::clamp(p.y + y_top, y_top, y_end);
                else       coord.y = std::clamp(p.y, 0, panel.y - 1);
            }
            // bufferbase: CSI y; x H/F  Caret position (1-based).
    virtual void cup(twod p)
            {
                parser::flush_data();
                _cup(p - dot_11);
            }
            // bufferbase: Caret position (0-based).
    virtual void cup0(twod p)
            {
                parser::flush_data();
                _cup(p);
            }
            // bufferbase: CSI y; x H/F  Caret position (1-based).
    virtual void cup(fifo& queue)
            {
                auto y = queue(1);
                auto x = queue(1);
                cup({ x, y });
            }
            // bufferbase: Move cursor up.
    virtual void up(si32 n)
            {
                parser::flush_data();
                if (n == 0) n = 1;
                auto new_coord_y = coord.y - n;
                if (new_coord_y < y_top && coord.y >= y_top)
                {
                    auto dy = y_top - new_coord_y;
                    scroll_region(y_top, y_end, dy, true);
                    coord.y = y_top;
                }
                else coord.y = std::clamp(new_coord_y, 0, panel.y - 1);
            }
            // bufferbase: Move cursor down.
    virtual void dn(si32 n)
            {
                parser::flush_data();
                if (n == 0) n = 1;
                auto new_coord_y = coord.y + n;
                if (new_coord_y > y_end && coord.y <= y_end)
                {
                    auto dy = new_coord_y - y_end;
                    scroll_region(y_top, y_end, -dy, true);
                    coord.y = y_end;
                }
                else coord.y = std::clamp(new_coord_y, 0, panel.y - 1);
            }
            // bufferbase: Line feed. Index. Scroll region up if new_coord_y > end.
    virtual void lf(si32 n)
            {
                parser::flush_data();
                auto new_coord_y = coord.y + n;
                if (new_coord_y > y_end && coord.y <= y_end)
                {
                    auto n = y_end - new_coord_y;
                    scroll_region(y_top, y_end, n, true);
                    coord.y = y_end;
                }
                else coord.y = std::clamp(new_coord_y, 0, panel.y - 1);
                if (coord.x < 0 || coord.x > panel.x) coord.x = 0; // Auto cr if the cursor is outside the viewport (to prevent infinitely long lines when autocr is disabled).
            }
            // bufferbase: '\r'  CR Cursor return. Go to home of visible line instead of home of paragraph.
    virtual void cr()
            {
                parser::flush();
                coord.x = 0;
            }
            // bufferbase: CSI n J  Erase display.
            void ed(si32 n)
            {
                parser::flush();
                switch (n)
                {
                    case commands::erase::display::below: // n = 0 (default)  Erase viewport after cursor.
                        del_below();
                        break;
                    case commands::erase::display::above: // n = 1  Erase viewport before cursor.
                        del_above();
                        break;
                    case commands::erase::display::viewport: // n = 2  Erase viewport.
                        set_coord(dot_00);
                        ed(commands::erase::display::below);
                        break;
                    case commands::erase::display::scrollback: // n = 3  Erase scrollback.
                        clear_all();
                        break;
                    default:
                        break;
                }
            }
            // bufferbase: CSI n K  Erase line (don't move cursor).
    virtual void el(si32 n) = 0;
            // bufferbase: Helper to reset viewport horizontal position.
            static auto reset_viewport(si32 origin_x, si32 x, si32 panel_x)
            {
                if (origin_x != 0 || x != panel_x)
                {
                         if (x >= 0  &&   x < panel_x) origin_x = 0;
                    else if (x >= -origin_x + panel_x) origin_x = 0 - x + panel_x - 1;
                    else if (x <  -origin_x          ) origin_x = 0 - x;
                }
                return origin_x;
            };
            // bufferbase: Select shader.
            template<class P>
            void _shade(si32 fx, cell const& c, P work)
            {
                switch (fx)
                {
                    case commands::fx::color:   work(cell::shaders::color(c)); break;
                    case commands::fx::xlight:  work(cell::shaders::xlight);   break;
                    case commands::fx::invert:  work(cell::shaders::invert);   break;
                    case commands::fx::reverse: work(cell::shaders::reverse);  break;
                }
            }
            // bufferbase: Shade selection.
            template<class P>
            auto _shade_selection(si32 mode, P work)
            {
                switch (owner.ftrack ? mode : mime::disabled)
                {
                    case mime::ansitext: _shade(owner.config.def_ansi_f, owner.config.def_ansi_c, work); break;
                    case mime::richtext: _shade(owner.config.def_rich_f, owner.config.def_rich_c, work); break;
                    case mime::htmltext: _shade(owner.config.def_html_f, owner.config.def_html_c, work); break;
                    case mime::textonly: _shade(owner.config.def_text_f, owner.config.def_text_c, work); break;
                    case mime::safetext: _shade(owner.config.def_safe_f, owner.config.def_safe_c, work); break;
                    default:             _shade(owner.config.def_none_f, owner.config.def_none_c, work); break;
                }
            }
            // bufferbase: Rasterize selection with grips.
            void selection_raster(face& dest, auto curtop, auto curend, bool ontop = true, bool onend = true)
            {
                if (selection_active())
                {
                    auto mode = owner.selmod;
                    auto view = dest.view();
                    auto grip_1 = rect{ curtop, dot_11 };
                    auto grip_2 = rect{ curend, dot_11 };
                    grip_1.coor.x += view.coor.x; // Compensate scrollback's hz movement.
                    grip_2.coor.x += view.coor.x; //
                    auto square = grip_1 | grip_2;
                    square.normalize_itself();
                    auto work = [&](auto fill)
                    {
                        if (!selection_selbox())
                        {
                            auto size_0 = square.size - dot_01;
                            auto size_1 = curtop.x + curtop.y * panel.x;
                            auto size_2 = curend.x + curend.y * panel.x;
                            if (size_1 > size_2) std::swap(curtop, curend);
                            auto a = curtop.x;
                            auto b = curend.x + 1;
                            if (curtop.x > curend.x)
                            {
                                square.coor += dot_11;
                                square.size -= dot_11 + dot_11;
                                std::swap(a, b);
                            }
                            auto west = rect{{ 0, curtop.y + 1 }, { a,           size_0.y }}.normalize();
                            auto east = rect{{ b, curtop.y     }, { panel.x - b, size_0.y }}.normalize();
                            west.coor.x += view.coor.x; // Compensate scrollback's hz movement.
                            east.coor.x += view.coor.x; //
                            west = west.clip(view);
                            east = east.clip(view);
                            dest.fill(west, fill);
                            dest.fill(east, fill);
                        }
                        square = square.clip(view);
                        dest.fill(square, fill);
                    };
                    _shade_selection(mode, work);
                }
            }
            // bufferbase: Pickup selected data from canvas.
            void selection_pickup(escx& buffer, rich& canvas, twod seltop, twod selend, si32 selmod, bool selbox)
            {
                auto limits = panel - dot_11;
                auto curtop = std::clamp(seltop, dot_00, limits);
                auto curend = std::clamp(selend, dot_00, limits);
                auto grip_1 = rect{ curtop, dot_11 };
                auto grip_2 = rect{ curend, dot_11 };
                grip_1.coor += canvas.coor();
                grip_2.coor += canvas.coor();
                auto square = grip_1 | grip_2;
                square.normalize_itself();
                if (selbox || grip_1.coor.y == grip_2.coor.y)
                {
                    selmod == mime::disabled ||
                    selmod == mime::textonly ||
                    selmod == mime::safetext ? buffer.s11n<faux>(canvas, square)
                                             : buffer.s11n<true>(canvas, square);
                }
                else
                {
                    if (grip_1.coor.y > grip_2.coor.y) std::swap(grip_1, grip_2);
                    auto part_1 = rect{ grip_1.coor,             { panel.x - grip_1.coor.x, 1              }};
                    auto part_2 = rect{{ 0, grip_1.coor.y + 1 }, { panel.x, std::max(0, square.size.y - 2) }};
                    auto part_3 = rect{{ 0, grip_2.coor.y     }, { grip_2.coor.x + 1, 1                    }};
                    if (selmod == mime::textonly
                     || selmod == mime::safetext
                     || selmod == mime::disabled)
                    {
                        buffer.s11n<faux, true, faux>(canvas, part_1);
                        buffer.s11n<faux, faux, faux>(canvas, part_2);
                        buffer.s11n<faux, faux, true>(canvas, part_3);
                    }
                    else
                    {
                        buffer.s11n<true, true, faux>(canvas, part_1);
                        buffer.s11n<true, faux, faux>(canvas, part_2);
                        buffer.s11n<true, faux, true>(canvas, part_3);
                    }
                }
            }
            // bufferbase: Find the next match in the specified canvas and return true if found.
            auto selection_search(rich const& canvas, si32 from, feed direction, twod& seltop, twod& selend)
            {
                auto find = [&](auto a, auto b, auto& uinext, auto& uiprev)
                {
                    auto length = canvas.size().x;
                    auto offset = from + a;
                    if (canvas.find(match, offset, direction))
                    {
                        seltop = { offset % length,
                                   offset / length };
                        offset += a - b + 1;
                        selend = { offset % length,
                                   offset / length };
                        offset += a;
                        uinext = canvas.find(match, offset, direction); // Try to find next next.
                        uiprev = true;
                        return true;
                    }
                    else
                    {
                        uinext = faux;
                        return faux;
                    }
                };
                return direction == feed::fwd ? find(match.length(), 2, uifwd, uirev)
                                              : find(0, match.length(), uirev, uifwd);
            }
            // bufferbase: Return match navigation state.
    virtual si32 selection_button(twod delta = {})
            {
                auto forward_is_available = uifwd ? 1 << 0 : 0;
                auto reverse_is_available = uirev ? 1 << 1 : 0;
                return forward_is_available | reverse_is_available;
            }

            // bufferbase: Update terminal status.
            bool update_status(term_state& status) const
            {
                auto changed = faux;
                if (auto v = get_size(); status.size != v) { changed = true; status.size = v; }
                if (auto v = get_peak(); status.peak != v) { changed = true; status.peak = v; }
                if (auto v = get_mxsz(); status.mxsz != v) { changed = true; status.mxsz = v; }
                if (auto v = get_step(); status.step != v) { changed = true; status.step = v; }
                if (auto v = get_view(); status.area != v) { changed = true; status.area = v; }
                if (auto v = selection_active(); status.hash != v)
                {
                    changed = true;
                    status.hash = v;
                    if (status.hash)
                    {
                        status.mode = get_mode();
                        selection_status(status);
                    }
                }
                return changed;
            }
            void wrapdn() // Only for coord.x > panel.x.
            {
                coord.y += (coord.x + (panel.x - 1)) / panel.x - 1;
                coord.x  = (coord.x - 1) % panel.x + 1;
            }
            void wrapup() // Only for negative coord.x.
            {
                coord.y += (coord.x - (panel.x - 1)) / panel.x + 1 - 1;
                coord.x  = (coord.x + 1) % panel.x + panel.x - 1;
            }
        };

        // term: Alternate screen buffer implementation.
        struct alt_screen
            : public bufferbase
        {
            rich canvas; // alt_screen: Terminal screen.
            twod seltop; // alt_screen: Selected area head.
            twod selend; // alt_screen: Selected area tail.

            alt_screen(term& boss)
                : bufferbase{ boss }
            { }

            si32 get_size() const override { return panel.y; }
            si32 get_peak() const override { return panel.y; }
            si32 get_mxsz() const override { return panel.y; }
            si32 get_step() const override { return 0;       }

            // alt_screen: Resize viewport.
            void resize_viewport(twod new_sz, bool forced = faux) override
            {
                bufferbase::resize_viewport(new_sz);
                coord = std::clamp(coord, dot_00, panel - dot_11);
                canvas.crop(panel, brush.dry());
            }
            // alt_screen: Return viewport height.
            si32 height() override
            {
                return panel.y;
            }
            // alt_screen: Recalc left and right oversize (Always 0 for altbuf).
            bool recalc_pads(dent& oversz) override
            {
                auto left = 0;
                auto rght = 0;
                if (oversz.r != rght
                 || oversz.l != left)
                {
                    oversz.r = rght;
                    oversz.l = left;
                    return true;
                }
                else return faux;
            }
            static void _el(si32 n, core& canvas, twod coord, twod panel, cell const& blank)
            {
                assert(coord.y < panel.y);
                assert(coord.x >= 0);
                auto size = canvas.size();
                auto head = canvas.iter() + coord.y * size.x;
                auto tail = head;
                switch (n)
                {
                    default:
                    case commands::erase::line::right: // n = 0 (default)  Erase to Right.
                        head += std::min(panel.x, coord.x);
                        tail += panel.x;
                        break;
                    case commands::erase::line::left: // n = 1  Erase to Left.
                        tail += std::min(panel.x, coord.x + 1); // +1 to include the current cell.
                        break;
                    case commands::erase::line::all: // n = 2  Erase All.
                        tail += panel.x;
                        break;
                }
                while (head != tail) *head++ = blank;
            }
            // alt_screen: CSI n K  Erase line (don't move cursor).
            void el(si32 n) override
            {
                bufferbase::flush();
                _el(n, canvas, coord, panel, brush.nul());
            }
            // alt_screen: CSI n @  ICH. Insert n blanks after cursor. No wrap. Existing chars after cursor shifts to the right. Don't change cursor pos.
            void ins(si32 n) override
            {
                bufferbase::flush();
                assert(coord.y < panel.y);
                assert(coord.x >= 0);
                canvas.insert(coord, n, brush.nul());
            }
            // alt_screen: CSI n P  Delete (not Erase) letters under the cursor.
            void dch(si32 n) override
            {
                bufferbase::flush();
                canvas.cutoff(coord, n, brush.nul());
            }
            // alt_screen: '\x7F'  Delete letter backward.
            void del(si32 n) override
            {
                bufferbase::flush();
                n = std::max(0, n);
                coord.x -= n;
                if (coord.x < 0)
                {
                    wrapup();
                }
                canvas.backsp(coord, n, brush.nul());
                if (coord.y < 0) coord = dot_00;
            }
            // alt_screen: Move cursor by n in line.
            void move(si32 n) override
            {
                bufferbase::flush();
                coord.x += n;
                     if (coord.x < 0)       wrapup();
                else if (coord.x > panel.x) wrapdn();
                     if (coord.y < 0)        coord = dot_00;
                else if (coord.y >= panel.y) coord = panel - dot_01;
            }
            // alt_screen: CSI n X  Erase/put n chars after cursor. Don't change cursor pos.
            void ech(si32 n, char c = '\0') override
            {
                parser::flush();
                auto blank = brush;
                blank.txt(c);
                canvas.splice(coord, n, blank);
            }
            // alt_screen: Proceed new text using specified cell shader.
            template<bool Copy = faux, class Span, class Shader>
            void _data(si32 count, Span const& proto, Shader fuse)
            {
                assert(coord.y >= 0 && coord.y < panel.y);

                auto saved = coord;
                coord.x += count;
                //todo apply line adjusting (necessity is not clear)
                if (coord.x <= panel.x)//todo styles! || ! curln.wrapped())
                {
                    auto n = std::min(count, panel.x - std::max(0, saved.x));
                    canvas.splice<Copy>(saved, n, proto, fuse);
                }
                else
                {
                    wrapdn();
                    if (saved.y < y_top)
                    {
                        if (coord.y >= y_top)
                        {
                            auto n = coord.x + (coord.y - y_top) * panel.x;
                            count -= n;
                            set_coord({ 0, y_top });
                            _data<Copy>(n, proto, fuse); // Reversed fill using the last part of the proto.
                        }
                        auto data = proto.begin();
                        auto seek = saved.x + saved.y * panel.x;
                        auto dest = canvas.iter() + seek;
                        auto tail = dest + count;
                        rich::forward_fill_proc<Copy>(data, dest, tail, fuse);
                    }
                    else if (saved.y <= y_end)
                    {
                        if (coord.y > y_end)
                        {
                            auto dy = y_end - coord.y;
                            coord.y = y_end;
                            canvas.scroll(y_top, y_end + 1, dy, brush.spare);
                        }

                        auto seek = coord.x + coord.y * panel.x;
                        auto miny = seek - y_top * panel.x;
                        if (count > miny) count = miny;

                        auto dest = canvas.iter() + seek;
                        auto tail = dest - count;
                        auto data = proto.end();
                        rich::reverse_fill_proc<Copy>(data, dest, tail, fuse);
                    }
                    else
                    {
                        if (coord.y >= panel.y) coord.y = panel.y - 1;

                        auto data = proto.begin();
                        auto size = count;
                        auto seek = saved.x + saved.y * panel.x;
                        auto dest = canvas.iter() + seek;
                        auto tail = canvas.iend();
                        auto back = panel.x;
                        rich::unlimit_fill_proc<Copy>(data, size, dest, tail, back, fuse);
                    }
                }
            }
            // alt_screen: Parser callback.
            void data(si32 count, grid const& proto) override
            {
                //_data(count, proto, cell::shaders::full);
                _data(count, proto, cell::shaders::skipnuls);
            }
            // alt_screen: Clear viewport.
            void clear_all() override
            {
                canvas.wipe(brush.dry());
                set_scroll_region(0, 0);
                bufferbase::clear_all();
            }
            // alt_screen: Render to the dest.
            void output(face& dest) override
            {
                auto full = dest.full();
                auto view = dest.view();
                auto find = selection_active()
                         && match.length()
                         && owner.selmod == mime::textonly;
                canvas.move(full.coor);
                dest.fill(canvas, cell::shaders::fuse);
                if (find)
                {
                    if (auto area = canvas.area())
                    {
                        dest.full(area);
                        auto offset = si32{};
                        auto work = [&](auto shader)
                        {
                            while (canvas.find(match, offset))
                            {
                                auto c = canvas.toxy(offset);
                                dest.output(match, c, shader);
                                offset += match.length();
                            }
                        };
                        _shade(owner.config.def_find_f, owner.config.def_find_c, work);
                        dest.full(full);
                    }
                }

                selection_render(dest);
            }
            // alt_screen: Remove all lines below except the current. "ED2 Erase viewport" keeps empty lines.
            void del_below() override
            {
                canvas.del_below(coord, brush.spare.dry());
            }
            // alt_screen: Clear all lines from the viewport top line to the current line.
            void del_above() override
            {
                auto coorx = coord.x;
                if (coorx < panel.x) ++coord.x; // Clear the cell at the current position. See ED1 description.
                canvas.del_above(coord, brush.spare.dry());
                coord.x = coorx;
            }
            // alt_screen: Shift by n the scroll region.
            void scroll_region(si32 top, si32 end, si32 n, bool use_scrollback = faux) override
            {
                seltop.y += n;
                selend.y += n;
                canvas.scroll(top, end + 1, n, brush.spare.dry());
            }
            // alt_screen: Horizontal tab.
            void tab(si32 n) override
            {
                bufferbase::tab(n);
                coord.x = std::clamp(coord.x, 0, panel.x - 1);
            }
            // alt_screen: Make a viewport screen copy.
            void do_viewport_copy(face& dest) override
            {
                auto full = dest.full();
                auto view = dest.view().clip(full);
                dest.view(view);
                dest.plot(canvas, cell::shaders::full);
            }
            // alt_screen: Start text selection.
            void selection_create(twod coor, bool mode) override
            {
                auto limits = panel - dot_11;
                auto curtop = std::clamp(seltop, dot_00, limits);
                auto curend = std::clamp(selend, dot_00, limits);
                if (selection_active())
                {
                    if (coor == curtop)
                    {
                        seltop = curend;
                        selend = coor;
                    }
                    else if (coor != curend)
                    {
                        seltop = std::clamp(coor, dot_00, limits);
                        selend = curtop;
                    }
                }
                else
                {
                    seltop = std::clamp(coor, dot_00, limits);
                    selend = curtop;
                }
                selection_selbox(mode);
                selection_update();
            }
            // alt_screen: Extend text selection.
            bool selection_extend(twod coor, bool mode) override
            {
                auto state = selection_active();
                if (state)
                {
                    auto limits = panel - dot_11;
                    selend = std::clamp(coor, dot_00, limits);
                    selection_selbox(mode);
                    selection_update();
                }
                return state;
            }
            // alt_screen: Set selection orientation.
            void selection_follow(twod coor, bool lock) override
            {
                selection_locked(lock);
                if (selection_active())
                {
                    auto limits = panel - dot_11;
                    coor = std::clamp(coor, dot_00, limits);
                    if (selection_selbox())
                    {
                        auto c = (selend + seltop) / 2;
                        if (coor.x > c.x == seltop.x > selend.x) std::swap(seltop.x, selend.x);
                        if (coor.y > c.y == seltop.y > selend.y) std::swap(seltop.y, selend.y);
                    }
                    else
                    {
                        auto swap = selend.y == seltop.y ? std::abs(selend.x - coor.x) > std::abs(seltop.x - coor.x)
                                                         : std::abs(selend.y - coor.y) > std::abs(seltop.y - coor.y);
                        if (swap) std::swap(seltop, selend);
                    }
                }
            }
            // alt_screen: Select one word.
            void selection_byword(twod coor) override
            {
                seltop = selend = coor;
                seltop.x = canvas.word<feed::rev>(coor);
                selend.x = canvas.word<feed::fwd>(coor);
                selection_locked(faux);
                selection_selbox(faux);
                selection_update(faux);
            }
            // alt_screen: Select one line.
            void selection_byline(twod coor) override
            {
                seltop.y = selend.y = coor.y;
                seltop.x = 0;
                selend.x = panel.x - 1;
                selection_locked(faux);
                selection_selbox(faux);
                selection_update(faux);
            }
            // alt_screen: Take selected data.
            text selection_pickup(si32 selmod) override
            {
                auto data = escx{};
                if (selection_active())
                {
                    auto selbox = selection_selbox();
                    bufferbase::selection_pickup(data, canvas, seltop, selend, selmod, selbox);
                    if (selbox && !data.empty()) data.eol();
                }
                return std::move(data);
            }
            // alt_screen: Highlight selection.
            void selection_render(face& dest) override
            {
                auto limits = panel - dot_11;
                auto curtop = std::clamp(seltop, dot_00, limits);
                auto curend = std::clamp(selend, dot_00, limits);
                selection_raster(dest, curtop, curend);
            }
            // alt_screen: Update selection status.
            void selection_status(term_state& status) const override
            {
                status.coor = std::abs(selend - seltop);
                status.body = status.coor.y * panel.x + status.coor.x + 1;
                status.coor+= dot_11;
            }
            // alt_screen: Update selection internals.
            void selection_update(bool despace = true) override
            {
                if (selection_selbox()
                 && seltop.y != selend.y)
                {
                    match = {};
                    uirev = faux;
                    uifwd = faux;
                }
                else
                {
                    match = { canvas.core::line(seltop, selend) };
                    auto p1 = seltop;
                    auto p2 = selend;
                    if (p1.y > p2.y || (p1.y == p2.y && p1.x > p2.x)) std::swap(p1, p2);
                    auto offset = p1.x + p1.y * panel.x;
                    uifwd = canvas.find(match, offset + match.length(), feed::fwd); // Try to find next next.
                    uirev = canvas.find(match, offset - 1,              feed::rev); // Try to find next prev.
                }
                bufferbase::selection_update(despace);
            }
            // alt_screen: Search data and return distance to it.
            twod selection_gofind(feed direction, view data = {}) override
            {
                if (data.empty()) return dot_00;
                match = line{ data };
                seltop = direction == feed::fwd ? twod{-match.length(), 0 }
                                                : twod{ panel.x, panel.y - 1 };
                selend = seltop;
                uirev = faux;
                uifwd = faux;
                selection_gonext(direction);
                if ((direction == feed::fwd && uirev == faux)
                 || (direction == feed::rev && uifwd == faux))
                {
                    selection_cancel();
                }
                return dot_00;
            }
            // alt_screen: Search prev/next selection match and return distance to it.
            twod selection_gonext(feed direction) override
            {
                auto p1 = seltop;
                auto p2 = selend;
                if (p1.y > p2.y || (p1.y == p2.y && p1.x > p2.x)) std::swap(p1, p2);

                auto from = p1.x + p1.y * panel.x;
                bufferbase::selection_search(canvas, from, direction, seltop, selend);
                bufferbase::selection_update(faux);
                return dot_00;
            }
            // alt_screen: Cancel text selection.
            bool selection_cancel() override
            {
                bufferbase::uirev = faux;
                bufferbase::uifwd = faux;
                return bufferbase::selection_cancel();
            }
        };

        // term: Scrollback buffer implementation.
        struct scroll_buf
            : public bufferbase
        {
            struct index_item
            {
                using id_t = line::id_t;
                id_t index;
                si32 start;
                si32 width;
                index_item() = default;
                index_item(id_t index, si32 start, si32 width)
                    : index{ index },
                      start{ start },
                      width{ width }
                { }
            };
            struct grip
            {
                enum type
                {
                    idle,
                    base,
                    join,
                };
                id_t link{}; // scroll_buf::grip: Anchor id.
                twod coor{}; // scroll_buf::grip: 2D offset relative to link.
                type role{}; // scroll_buf::grip: Grip category.

                static void sort(grip& g1, grip& g2)
                {
                    if (g1.coor.y > g2.coor.y
                    || (g1.coor.y == g2.coor.y && g1.coor.x > g2.coor.x))
                    {
                        std::swap(g1, g2);
                    }
                }
            };
            enum class part
            {
                top, mid, end,
            };
            using ring = generics::ring<std::vector<line>, true>;
            using indx = generics::ring<std::vector<index_item>>;

            struct buff : public ring
            {
                using ring::ring;
                using type = line::type;
                using maps = std::map<si32, si32>[type::count];

                si32 caret{}; // buff: Current line caret horizontal position.
                si32 vsize{}; // buff: Scrollback vertical size (height).
                si32 width{}; // buff: Viewport width.
                si32 basis{}; // buff: Working area basis. Vertical position of O(0, 0) in the scrollback.
                si32 slide{}; // buff: Viewport vertical position in the scrollback.
                maps sizes{}; // buff: Line length accounting database.
                id_t ancid{}; // buff: The nearest line id to the slide.
                si32 ancdy{}; // buff: Slide's top line offset.
                bool round{}; // buff: Is the slide position approximate.
                bool rolls{}; // buff: The scrollback buffer ring was scrolled.

                // buff: Decrease height.
                void dec_height(si32& vsize, type kind, si32 size)
                {
                    if (size > width && kind == type::autowrap) vsize -= (size + width - 1) / width;
                    else                                        vsize -= 1;
                }
                // buff: Increase height.
                void add_height(si32& vsize, type kind, si32 size)
                {
                    if (size > width && kind == type::autowrap) vsize += (size + width - 1) / width;
                    else                                        vsize += 1;
                }
                // buff: Return max line length of the specified type.
                template<auto N>
                auto max()
                {
                    return sizes[N].empty() ? 0
                                            : sizes[N].rbegin()->first;
                }
                // buff: Recalculate unlimited scrollback height without reflow.
                void set_width(si32 new_width)
                {
                    vsize = 0;
                    width = std::max(1, new_width);
                    for (auto kind : { type::leftside,
                                       type::rghtside,
                                       type::centered })
                    {
                        for (auto [size, pool] : sizes[kind])
                        {
                            assert(pool > 0);
                            vsize += pool;
                        }
                    }
                    auto kind = type::autowrap;
                    for (auto [size, pool] : sizes[kind])
                    {
                        if (size > width) vsize += pool * ((size + width - 1) / width);
                        else              vsize += pool;
                    }
                }
                // buff: Register a new line.
                void invite(type& kind, si32& size, type new_kind, si32 new_size)
                {
                    ++sizes[new_kind][new_size];
                    add_height(vsize, new_kind, new_size);
                    size = new_size;
                    kind = new_kind;
                }
                // buff: Refresh scrollback height.
                void recalc(type& kind, si32& size, type new_kind, si32 new_size)
                {
                    if (size != new_size
                     || kind != new_kind)
                    {
                        undock(kind, size);
                        ++sizes[new_kind][new_size];
                        add_height(vsize, new_kind, new_size);
                        size = new_size;
                        kind = new_kind;
                    }
                }
                // buff: Discard the specified metrics.
                void undock(type kind, si32 size)
                {
                    auto& lens = sizes[kind];
                    auto  iter = lens.find(size); assert(iter != lens.end());
                    auto  pool = --(*iter).second;
                    if (pool == 0) lens.erase(iter);
                    dec_height(vsize, kind, size);
                }
                // buff: Check buffer size.
                bool check_size(twod new_size)
                {
                    auto old_value = vsize;
                    set_width(new_size.x);
                    if (ring::peak <= new_size.y)
                    {
                        static constexpr auto BOTTOM_ANCHORED = true;
                        ring::resize<BOTTOM_ANCHORED>(new_size.y);
                    }
                    return old_value != vsize;
                }
                // buff: Push the specified line back.
                void invite(line& l)
                {
                    invite(l._kind, l._size, l.style.get_kind(), l.length());
                }
                // buff: Push a new line back.
                template<class ...Args>
                auto& invite(Args&&... args)
                {
                    auto& l = ring::push_back(std::forward<Args>(args)...);
                    invite(l._kind, l._size, l.style.get_kind(), l.length());
                    return l;
                }
                // buff: Insert a new line at the specified position.
                template<class ...Args>
                auto& insert(si32 at, Args&&... args)
                {
                    auto& l = *ring::insert(at, std::forward<Args>(args)...);
                    invite(l._kind, l._size, l.style.get_kind(), l.length());
                    return l;
                }
                // buff: Remove specified line info from accounting and update metrics based on scroll height.
                void undock_base_front(line& l) override
                {
                    auto kind = l._kind;
                    auto size = l._size;
                    undock(kind, size);
                    dec_height(basis, kind, size);
                    dec_height(slide, kind, size);
                    if (basis < 0)
                    {
                        basis = 0;
                    }
                    if (slide < 0)
                    {
                        ancid = l.index + 1;
                        ancdy = 0;
                        slide = 0;
                    }
                    rolls = true;
                }
                // buff: Remove information about the specified line from accounting.
                void undock_base_back(line& l) override { undock(l._kind, l._size); }
                // buff: Return the item position in the scrollback using its id.
                auto index_by_id(ui32 id) const
                {
                    //No need to disturb distant objects, it may already be in the swap.
                    auto count = length();
                    return static_cast<si32>(count - 1 - (back().index - id)); // ring buffer size is never larger than max_int32.
                }
                // buff: Return an iterator pointing to the item with the specified id.
                auto iter_by_id(ui32 id) -> ring::iter<ring> //todo MSVC 17.7.0 requires return type
                {
                    return begin() + index_by_id(id);
                }
                // buff: Return the item reference using its id.
                auto& item_by_id(ui32 id)
                {
                    return ring::at(index_by_id(id));
                }
                // buff: Refresh metrics due to modified line.
                void recalc(line& l)
                {
                    recalc(l._kind, l._size, l.style.get_kind(), l.length());
                }
                // buff: Rewrite the indices from the specified position to the end.
                void reindex(si32 from)
                {
                    assert(from >= 0);
                    auto head = begin() + from;
                    auto tail = end();
                    auto indx = from == 0 ? 0
                                          : (head - 1)->index + 1;
                    while (head != tail)
                    {
                        head->index = indx++;
                        ++head;
                    }
                }
                // buff: Remove the specified number of lines at the specified position (inclusive).
                auto remove(si32 at, si32 count)
                {
                    count = ring::remove(at, count);
                    reindex(at);
                    return count;
                }
                // buff: Clear scrollback, add one empty line, and reset all metrics.
                void clear()
                {
                    auto auto_wrap = current().wrapped();
                    ring::clear();
                    caret = 0;
                    basis = 0;
                    slide = 0;
                    invite(0, deco{}.wrp(auto_wrap)); // At least one line must exist.
                    ancid = back().index;
                    ancdy = 0;
                    set_width(width);
                }
            };

            friend auto& operator << (std::ostream& s, scroll_buf& c) // For debug.
            {
                return s << "{ " << c.batch.max<line::type::leftside>() << ","
                                 << c.batch.max<line::type::rghtside>() << ","
                                 << c.batch.max<line::type::centered>() << ","
                                 << c.batch.max<line::type::autowrap>() << " }";
            }

            buff batch; // scroll_buf: Scrollback container.
            indx index; // scroll_buf: Viewport line index.
            face upbox; // scroll_buf:    Top margin canvas.
            face dnbox; // scroll_buf: Bottom margin canvas.
            twod upmin; // scroll_buf:    Top margin minimal size.
            twod dnmin; // scroll_buf: Bottom margin minimal size.
            grip upmid; // scroll_buf: Selection first grip inside the scrolling region.
            grip dnmid; // scroll_buf: Selection second grip inside the scrolling region.
            grip uptop; // scroll_buf: Selection first grip inside the top margin.
            grip dntop; // scroll_buf: Selection second grip inside the top margin.
            grip upend; // scroll_buf: Selection first grip inside the bottom margin.
            grip dnend; // scroll_buf: Selection second grip inside the bottom margin.
            part place; // scroll_buf: Selection last active region.
            si32 shore; // scroll_buf: Left and right scrollbuffer additional indents.

            static constexpr auto approx_threshold = si32{ 10000 }; //todo make it configurable

            scroll_buf(term& boss)
                : bufferbase{ boss },
                       batch{ boss.config.def_length, boss.config.def_growdt, boss.config.def_growmx },
                       index{ 1    },
                       place{      },
                       shore{ boss.config.def_margin }
            {
                parser::style.wrp(boss.config.def_wrpmod);
                batch.invite(0, deco{}.wrp(boss.config.def_wrpmod == wrap::on)); // At least one line must exist.
                batch.set_width(1);
                index_rebuild();

                auto brush = cell{ '\0' }.fgc(boss.config.def_fcolor).bgc(boss.config.def_bcolor).link(boss.id);
                boss.base::color(brush);
                parser::brush.reset(brush);
            }
            si32 get_size() const override { return batch.size;     }
            si32 get_peak() const override { return batch.peak - 1; }
            si32 get_step() const override { return batch.step;     }
            si32 get_mxsz() const override { return batch.mxsz;     }

            void print_slide(text msg)
            {
                log(msg, ": ", " batch.basis=", batch.basis, " batch.slide=", batch.slide,
                    " ancid=", batch.ancid, " ancdy=", batch.ancdy, " round=", batch.round ? 1:0);
            }
            void print_index(text msg)
            {
                log(" ", msg, " index.size=", index.size, " basis=", batch.basis, " panel=", panel);
                for (auto n = 0; auto& l : index)
                {
                    log("  ", n++,". id=", l.index," start=", l.start, " width=", l.width, l.start % panel.x != 0 ? " <-- BAD INDEX l.start % panel.x != 0":"");
                }
                auto& mapln = index.back();
                auto& curln = batch.item_by_id(mapln.index);
                log(" last ln id=", curln.index, " curln.length()=", curln.length());
                log(" -----------------");
            }
            void print_batch(text msg)
            {
                log(" ", msg,
                " batch.size=",  batch.size,
                " batch.vsize=", batch.vsize,
                " batch.basis=", batch.basis,
                " batch.slide=", batch.slide,
                " coord=", coord
                );
                for (auto n = 0; auto& l : batch)
                {
                    log("  ", n++,". id=", l.index, " length()=", l.length(), " wrapped=", l.wrapped() ? "true" : "faux");
                }
                log(" -----------------");
            }
            bool test_index()
            {
                auto m = index.front().index;
                for (auto& i : index)
                {
                    auto step = i.index - m;
                    assert(step >= 0 && step < 2);
                    m = i.index;
                }
                return true;
            }
            bool test_futures()
            {
                auto stash = batch.vsize - batch.basis - index.size;
                if (stash < 0)
                {
                    print_batch("test_basis");
                    print_index("test_basis");
                }
                assert(stash >= 0);
                test_basis();
                return true;
            }
            bool test_coord()
            {
                auto wrapped_block = batch.caret - coord.x;
                assert(coord.y < y_top || coord.y > y_end || wrapped_block % panel.x == 0);
                return true;
            }
            auto test_resize()
            {
                auto c = batch.caret;
                sync_coord();
                assert(c == batch.caret);
                return true;
            }
            auto test_height()
            {
                auto test_vsize = 0;
                for (auto& l : batch)
                {
                    test_vsize += l.height(panel.x);
                }
                if (test_vsize != batch.vsize) log(" ERROR! test_vsize=", test_vsize, " vsize=", batch.vsize);
                return test_vsize == batch.vsize;
            }
            bool test_basis()
            {
                if (batch.basis >= batch.vsize)
                {
                    assert((log(" batch.basis >= batch.vsize  batch.basis=", batch.basis, " batch.vsize=", batch.vsize), true));
                }

                auto index_front = index.front();
                auto temp = index_front;
                auto coor = batch.vsize;
                auto head = batch.end();
                while (coor != batch.basis)
                {
                    auto& curln = *--head;
                    auto  curid = curln.index;
                    auto length = curln.length();
                    if (curln.wrapped())
                    {
                        auto remain = length ? (length - 1) % panel.x + 1 : 0;
                        length -= remain;
                        index_front = { curid, length, remain };
                        --coor;
                        while (length > 0 && coor != batch.basis)
                        {
                            length -= panel.x;
                            index_front = { curid, length, panel.x };
                            --coor;
                        }
                    }
                    else
                    {
                        index_front = { curid, 0, length };
                        --coor;
                    }
                }
                auto result = index_front.index == temp.index
                           && index_front.start == temp.start
                           && index_front.width == temp.width;
                if (!result)
                {
                    print_batch("test_basis");
                    print_index("test_basis");
                }
                assert(result);
                return result;
            }
            // scroll_buf: Get viewport position.
            si32 get_origin(bool follow) override
            {
                auto coor_y = follow ? batch.basis
                                     : batch.slide;
                scroll_buf::set_slide(coor_y); // Update slide id anchoring.
                return -coor_y;
            }
            // scroll_buf: Get viewport basis.
            si32 get_basis() override
            {
                return batch.basis;
            }
            // scroll_buf: Get viewport position.
            si32 get_slide() override
            {
                return batch.slide;
            }
            // scroll_buf: Set left/right scrollback additional padding.
            void setpad(si32 new_value) override
            {
                if (new_value < 0) new_value = owner.config.def_margin;
                shore = std::min(new_value, 255);
            }
            // scroll_buf: Get left/right scrollback additional padding.
            si32 getpad() override
            {
                return shore;
            }
            // scroll_buf: Set viewport position and return whether the viewport is reset.
            bool set_slide(si32& fresh_slide) override
            {
                if (batch.slide == fresh_slide && !batch.rolls)
                {
                    return batch.slide == batch.basis;
                }
                batch.rolls = faux;

                if (batch.basis == fresh_slide)
                {
                    auto& mapln = index.front();
                    batch.ancid = mapln.index;
                    batch.ancdy = mapln.start / panel.x;
                    batch.slide = batch.basis;
                    batch.round = faux;
                }
                else
                {
                    auto& front = batch.front();
                    auto& under = batch.back();
                    auto  vtpos = batch.slide - batch.ancdy;
                    auto delta1 = fresh_slide - vtpos;       // Compare fresh_slide with batch.slide.
                    auto delta2 = fresh_slide - 0;           // Compare fresh_slide with 0.
                    auto delta3 = batch.vsize - fresh_slide; // Compare fresh_slide with batch.vsize.
                    auto range1 = std::abs(delta1);
                    auto range2 = std::abs(delta2);
                    auto range3 = std::abs(delta3);
                    auto lookup = [&]
                    {
                        auto idpos = batch.index_by_id(batch.ancid);
                        auto start = batch.begin() + idpos;
                        auto found = faux;
                        if (delta1 < 0) // Look up.
                        {
                            auto limit = start - std::min(range1, idpos);
                            while (start != limit)
                            {
                                auto& curln = *--start;
                                auto height = curln.height(panel.x);
                                vtpos -= height;
                                if (vtpos <= fresh_slide)
                                {
                                    batch.slide = fresh_slide;
                                    batch.ancid = curln.index;
                                    batch.ancdy = fresh_slide - vtpos;
                                    found = true;
                                    break;
                                }
                            }

                            if (!found)
                            {
                                batch.ancid = under.index - (batch.size - 1);
                                batch.ancdy = 0;
                                batch.slide = 0;
                                fresh_slide = 0;
                                batch.round = faux;
                                assert(batch.ancid == front.index);
                            }
                        }
                        else if (delta1 > 0) // Look down.
                        {
                            auto limit = start + std::min(delta1, batch.size - idpos - 1);
                            do
                            {
                                auto& curln = *start;
                                auto curpos = vtpos;
                                auto height = curln.height(panel.x);
                                vtpos += height;
                                if (vtpos > fresh_slide)
                                {
                                    batch.slide = fresh_slide;
                                    batch.ancid = curln.index;
                                    batch.ancdy = fresh_slide - curpos;
                                    found = true;
                                    break;
                                }
                            }
                            while (start++ != limit);

                            if (!found)
                            {
                                auto& mapln = index.front();
                                batch.slide = batch.basis;
                                fresh_slide = batch.basis;
                                batch.ancid = mapln.index;
                                batch.ancdy = mapln.start / panel.x;
                                batch.round = faux;
                            }
                        }
                        else
                        {
                            batch.slide = fresh_slide;
                            batch.ancdy = 0;
                            found = true;
                        }
                    };

                    if (batch.round && range1 < panel.y * 2)
                    {
                        lookup();
                        auto count1 = static_cast<si32>(under.index - batch.ancid);
                        auto count2 = static_cast<si32>(batch.ancid - front.index);
                        auto min_dy = std::min(count1, count2);

                        if (min_dy < approx_threshold) // Refine position to absolute value.
                        {
                            if (count1 < count2)
                            {
                                batch.slide = batch.ancdy + batch.vsize;
                                auto tail = batch.end();
                                auto head = tail - (count1 + 1);
                                while (head != tail)
                                {
                                    auto& curln = *--tail;
                                    batch.slide -= curln.height(panel.x);
                                }
                            }
                            else
                            {
                                batch.slide = batch.ancdy;
                                auto head = batch.begin();
                                auto tail = head + (count2 + 1);
                                while (head != tail)
                                {
                                    auto& curln = *head++;
                                    batch.slide += curln.height(panel.x);
                                }
                            }
                            batch.round = faux;
                            fresh_slide = batch.slide;
                        }
                    }
                    else
                    {
                        auto min_dy = std::min({ range1, range2, range3 });
                        if (min_dy > approx_threshold) // Calc approx.
                        {
                            ui64 count1 = std::min(std::max(0, fresh_slide), batch.vsize);
                            ui64 count2 = batch.vsize;
                            batch.ancid = front.index + static_cast<id_t>(netxs::divround(batch.size * count1, count2));
                            batch.ancdy = 0;
                            batch.slide = fresh_slide;
                            batch.round = batch.vsize != batch.size;
                        }
                        else if (min_dy == range2 || fresh_slide <= 0) // Calc from the batch top.
                        {
                            if (delta2 <= 0) // Above 0.
                            {
                                batch.ancid = front.index;
                                batch.ancdy = delta2;
                            }
                            else if (delta2 > 0)
                            {
                                auto vpos = 0;
                                auto head = batch.begin();
                                auto tail = batch.end();
                                do
                                {
                                    auto& curln = *head;
                                    auto newpos = vpos + curln.height(panel.x);
                                    if (newpos > fresh_slide) break;
                                    else                      vpos = newpos;
                                }
                                while (++head != tail);
                                assert(vpos <= fresh_slide);
                                batch.ancid = head->index;
                                batch.ancdy = fresh_slide - vpos;
                            }
                            batch.slide = fresh_slide;
                            batch.round = faux;
                        }
                        else if (min_dy == range3 || fresh_slide >= batch.vsize) // Calc from the batch bottom.
                        {
                            if (delta3 <= 0) // Below batch.vsize.
                            {
                                batch.ancid = under.index;
                                batch.ancdy = under.height(panel.x) - delta3;
                            }
                            else if (delta3 > 0)
                            {
                                auto vpos = batch.vsize;
                                auto head = batch.begin();
                                auto tail = batch.end();
                                while (head != tail && vpos > fresh_slide)
                                {
                                    auto& curln = *--tail;
                                    vpos -= curln.height(panel.x);
                                }
                                assert(vpos <= fresh_slide);
                                batch.ancid = tail->index;
                                batch.ancdy = fresh_slide - vpos;
                            }
                            batch.slide = fresh_slide;
                            batch.round = faux;
                        }
                        else if (min_dy == range1) // Calc relative to ancid.
                        {
                            lookup();
                        }
                    }
                }

                return batch.slide == batch.basis;
            }
            // scroll_buf: Recalc batch.slide using anchoring by para_id + para_offset.
            void recalc_slide(bool away)
            {
                if (away)
                {
                    auto& front = batch.front();
                    auto& under = batch.back();
                    auto range1 = static_cast<si32>(under.index - batch.ancid);
                    auto range2 = static_cast<si32>(batch.ancid - front.index);
                    batch.round = faux;
                    if (range1 < batch.size)
                    {
                        if (approx_threshold < std::min(range1, range2))
                        {
                            auto& mapln = index.front();
                            auto c1 = static_cast<ui64>(static_cast<si32>(mapln.index - front.index));
                            auto c2 = static_cast<ui64>(range2);
                            auto fresh_slide = static_cast<si32>(netxs::divround(batch.vsize * c2, c1));
                            batch.slide = batch.ancdy + fresh_slide;
                            batch.round = batch.vsize != batch.size;
                        }
                        else if (range1 < range2)
                        {
                            batch.slide = batch.ancdy + batch.vsize;
                            auto head = batch.end();
                            auto tail = head - (range1 + 1);
                            while (head != tail)
                            {
                                auto& curln = *--head;
                                batch.slide -= curln.height(panel.x);
                            }
                        }
                        else
                        {
                            batch.slide = batch.ancdy;
                            auto head = batch.begin();
                            auto tail = head + (range2 + 1);
                            while (head != tail)
                            {
                                auto& curln = *head++;
                                batch.slide += curln.height(panel.x);
                            }
                        }
                             if (batch.slide > batch.basis) batch.slide = batch.basis;
                        else if (batch.slide <= 0) // Overflow.
                        {
                            batch.ancid = front.index;
                            batch.ancdy = 0;
                            batch.slide = 0;
                            assert(batch.ancid == front.index);
                        }
                    }
                    else // Overflow.
                    {
                        batch.ancid = front.index;
                        batch.ancdy = 0;
                        batch.slide = 0;
                        assert(batch.ancid == front.index);
                    }
                }
                else
                {
                    auto& mapln = index.front();
                    batch.ancid = mapln.index;
                    batch.ancdy = mapln.start / panel.x;
                    batch.slide = batch.basis;
                    batch.round = faux;
                }
            }
            // scroll_buf: Resize viewport.
            void resize_viewport(twod new_sz, bool forced = faux) override
            {
                if (new_sz == panel && !forced) return;

                auto in_top = y_top - coord.y;
                auto in_end = coord.y - y_end;

                bufferbase::resize_viewport(new_sz);

                auto vsized = batch.check_size(panel);
                index.clear();

                // Preserve original content. The app that changed the margins is responsible for updating the content.
                auto upnew = std::max(upmin, twod{ panel.x, sctop });
                auto dnnew = std::max(dnmin, twod{ panel.x, scend });
                upbox.crop(upnew);
                dnbox.crop(dnnew);

                index.resize(arena); // Use a fixed ring because new lines are added much more often than a futures feed.
                auto away = batch.basis != batch.slide;

                auto& curln = batch.current();
                if (curln.wrapped() && batch.caret > curln.length()) // Dangling cursor.
                {
                    curln.crop(batch.caret, brush.dry());
                    batch.recalc(curln);
                }

                if (in_top > 0 || in_end > 0) // The cursor is outside the scrolling region.
                {
                    if (in_top > 0) coord.y = std::max(0,           y_top - in_top);
                    else            coord.y = std::min(panel.y - 1, y_end + in_end);
                    coord.x = std::clamp(coord.x, 0, panel.x - 1);
                    batch.basis = std::max(0, batch.vsize - arena);
                    index_rebuild();
                    if (vsized || !away) recalc_slide(away);
                    return;
                }

                batch.basis = batch.vsize;
                auto lnid = curln.index;
                auto head = batch.end();
                auto maxn = batch.size - batch.index();
                auto tail = head - std::max(maxn, std::min(batch.size, arena));
                auto push = [&](auto i, auto o, auto r) { --batch.basis; index.push_front(i, o, r); };
                auto unknown = true;
                while (head != tail && (index.size < arena || unknown))
                {
                    auto& curln = *--head;
                    auto  curid = curln.index;
                    auto length = curln.length();
                    auto active = curid == lnid;
                    if (curln.wrapped())
                    {
                        auto offset = length;
                        auto remain = length ? (length - 1) % panel.x + 1
                                             : 0;
                        do
                        {
                            offset -= remain;
                            push(curid, offset, remain);
                            if (unknown && active && offset <= batch.caret)
                            {
                                auto eq = batch.caret && length == batch.caret;
                                unknown = faux;
                                coord.y = index.size;
                                coord.x = eq ? (batch.caret - 1) % panel.x + 1
                                             :  batch.caret      % panel.x;
                            }
                            remain = panel.x;
                        }
                        while (offset > 0 && (index.size < arena || unknown));
                    }
                    else
                    {
                        push(curid, 0, length);
                        if (active)
                        {
                            unknown = faux;
                            coord.y = index.size;
                            coord.x = batch.caret;
                        }
                    }
                }
                coord.y = index.size - coord.y + y_top;
                if (vsized || !away) recalc_slide(away);

                assert(batch.basis >= 0);
                assert(test_futures());
                assert(test_coord());
                assert(test_resize());
            }
            // scroll_buf: Rebuild the next avail indexes from the known index (mapln).
            template<class Iter, class Index>
            void reindex(si32 avail, Iter curit, Index const& mapln)
            {
                auto& curln =*curit;
                auto  width = curln.length();
                auto  wraps = curln.wrapped();
                auto  curid = curln.index;
                auto  start = mapln.start + mapln.width;

                assert(curid == mapln.index);
                if (start == width) // Go to the next line.
                {
                    assert(curit != batch.end() - 1);

                    auto& curln = *++curit;
                    width = curln.length();
                    wraps = curln.wrapped();
                    curid = curln.index;
                    start = 0;
                }
                else assert(mapln.width == panel.x);

                assert(start % panel.x == 0);
                while (true)
                {
                    if (wraps)
                    {
                        auto trail = width - panel.x;
                        while (start < trail && avail-- > 0)
                        {
                            index.push_back(curid, start, panel.x);
                            start += panel.x;
                        }
                    }
                    if (avail-- <= 0) break;

                    assert(start == 0 || wraps);
                    index.push_back(curid, start, width - start);

                    if (avail == 0) break;

                    assert(curit != batch.end() - 1);
                    auto& curln = *++curit;
                    width = curln.length();
                    wraps = curln.wrapped();
                    curid = curln.index;
                    start = 0;
                }
                assert(test_index());
                assert(test_futures());
            }
            // scroll_buf: Rebuild index from the known index at y_pos.
            void index_rebuild_from(si32 y_pos)
            {
                assert(y_pos >= 0 && y_pos < index.size);

                auto& mapln = index[y_pos];
                auto  curit = batch.iter_by_id(mapln.index);
                auto  avail = std::min(batch.vsize - batch.basis, arena) - y_pos - 1;
                auto  count = index.size - y_pos - 1;
                while (count-- > 0) index.pop_back();

                if (avail > 0) reindex(avail, curit, mapln);
            }
            // scroll_buf: Rebuild index up to basis.
            void index_rebuild()
            {
                if (batch.basis >= batch.vsize)
                {
                    assert((log(prompt::term, "batch.basis >= batch.vsize  batch.basis=", batch.basis, " batch.vsize=", batch.vsize), true));
                    batch.basis = batch.vsize - 1;
                }

                index.clear();
                auto coor = batch.vsize;
                auto head = batch.end();
                while (coor != batch.basis)
                {
                    auto& curln = *--head;
                    auto  curid = curln.index;
                    auto length = curln.length();
                    if (curln.wrapped())
                    {
                        auto remain = length ? (length - 1) % panel.x + 1 : 0;
                        length -= remain;
                        index.push_front(curid, length, remain);
                        --coor;
                        while (length > 0 && coor != batch.basis)
                        {
                            length -= panel.x;
                            index.push_front(curid, length, panel.x);
                            --coor;
                        }
                    }
                    else
                    {
                        index.push_front(curid, 0, length);
                        --coor;
                    }
                }
                assert(test_futures());
            }
            // scroll_buf: Return scrollback height.
            si32 height() override
            {
                assert(test_height());
                return batch.vsize;
            }
            // scroll_buf: Recalc left and right oversize.
            bool recalc_pads(dent& oversz) override
            {
                auto coor = get_coord();
                auto rght = std::max({0, batch.max<line::type::leftside>() - panel.x, coor.x - panel.x + 1 }); // Take into account the cursor position.
                auto left = std::max( 0, batch.max<line::type::rghtside>() - panel.x);
                auto cntr = std::max( 0, batch.max<line::type::centered>() - panel.x);
                auto bttm = std::max( 0, batch.vsize - batch.basis - arena          );
                auto both = cntr >> 1;
                left = shore + std::max(left, both + (cntr & 1));
                rght = shore + std::max(rght, both);
                if (oversz.r != rght
                 || oversz.l != left
                 || oversz.b != bttm)
                {
                    oversz.r = rght;
                    oversz.l = left;
                    oversz.b = bttm;
                    return true;
                }
                else return faux;
            }
            // scroll_buf: Check if there are futures, use them when scrolling regions.
            auto feed_futures(si32 query)
            {
                assert(test_futures());
                assert(test_coord());
                assert(query > 0);

                auto stash = batch.vsize - batch.basis - index.size;
                auto avail = si32{};
                if (stash > 0)
                {
                    avail = std::min(stash, query);
                    batch.basis += avail;

                    auto& mapln = index.back();
                    auto  curit = batch.iter_by_id(mapln.index);

                    reindex(avail, curit, mapln);
                }
                else avail = 0;

                return avail;
            }
            // scroll_buf: Return current 0-based cursor position in the scrollback.
            twod get_coord(twod origin = {}) override
            {
                auto coor = coord;
                if (coor.y >= y_top
                 && coor.y <= y_end)
                {
                    coor.y += batch.basis;

                    auto visible = coor.y + origin.y;
                    if (visible < y_top // Do not show cursor behind margins when the scroll region is dragged by mouse.
                     || visible > y_end)
                    {
                        coor.y = dot_mx.y;
                    }

                    auto& curln = batch.current();
                    auto  align = curln.style.jet();

                    if (align == bias::left
                     || align == bias::none) return coor;

                    auto curidx = coord.y - y_top;
                    auto remain = index[curidx].width;
                    if (remain == panel.x && curln.wrapped()) return coor;

                    if    (align == bias::right )  coor.x += panel.x     - remain - 1;
                    else /*align == bias::center*/ coor.x += panel.x / 2 - remain / 2;
                }
                else
                {
                    coor -= origin;
                }
                return coor;
            }
            // scroll_buf: Set cursor position and sync it with buffer.
            void set_coord(twod new_coord) override
            {
                bufferbase::set_coord(new_coord);
                sync_coord();
            }
            // scroll_buf: Map the current cursor position to the scrollback.
            template<bool AllowPendingWrap = true>
            void sync_coord()
            {
                coord.y = std::clamp(coord.y, 0, panel.y - 1);
                if (coord.x < 0) coord.x = 0;

                if (coord.y >= y_top
                 && coord.y <= y_end)
                {
                    auto& curln = batch.current();
                    auto  wraps = curln.wrapped();
                    auto  curid = curln.index;
                    if constexpr (AllowPendingWrap)
                    {
                        if (coord.x > panel.x && wraps) coord.x = panel.x;
                    }
                    else
                    {
                        if (coord.x >= panel.x && wraps) coord.x = panel.x - 1;
                    }

                    coord.y -= y_top;

                    if (index.size <= coord.y)
                    {
                        auto add_count = coord.y - (index.size - 1);
                        add_lines(add_count);
                    }

                    auto& mapln = index[coord.y];
                    batch.caret = mapln.start + coord.x;

                    if (curid != mapln.index)
                    {
                        auto newix = batch.index_by_id(mapln.index);
                        batch.index(newix);

                        if (batch->style != parser::style)
                        {
                            _set_style(parser::style);
                            assert(newix == batch.index_by_id(index[coord.y].index));
                        }
                    }
                    coord.y += y_top;

                    assert((batch.caret - coord.x) % panel.x == 0);
                }
                else // Always wraps inside margins.
                {
                    if constexpr (AllowPendingWrap)
                    {
                        if (coord.x > panel.x) coord.x = panel.x;
                    }
                    else
                    {
                        if (coord.x >= panel.x) coord.x = panel.x - 1;
                    }
                }
            }

            void cup (fifo& q) override { bufferbase::cup (q); sync_coord<faux>(); }
            void cup (twod  p) override { bufferbase::cup (p); sync_coord<faux>(); }
            void cup0(twod  p) override { bufferbase::cup0(p); sync_coord<faux>(); }
            void cuf (si32  n) override { bufferbase::cuf (n); sync_coord<faux>(); }
            void cub (si32  n) override { bufferbase::cub (n); sync_coord<faux>(); }
            void chx (si32  n) override { bufferbase::chx (n); sync_coord<faux>(); }
            void tab (si32  n) override { bufferbase::tab (n); sync_coord<faux>(); }
            void chy (si32  n) override { bufferbase::chy (n); sync_coord(); }
            void scl (si32  n) override { bufferbase::scl (n); sync_coord(); }
            void il  (si32  n) override { bufferbase::il  (n); sync_coord(); }
            void dl  (si32  n) override { bufferbase::dl  (n); sync_coord(); }
            void up  (si32  n) override { bufferbase::up  (n); sync_coord(); }
            void dn  (si32  n) override { bufferbase::dn  (n); sync_coord(); }
            void lf  (si32  n) override { bufferbase::lf  (n); sync_coord(); }
            void ri  ()        override { bufferbase::ri  ( ); sync_coord(); }
            void cr  ()        override { bufferbase::cr  ( ); sync_coord(); }

            // scroll_buf: Reset the scrolling region.
            void reset_scroll_region()
            {
                upmin = dot_00;
                dnmin = dot_00;
                upbox.crop(upmin);
                dnbox.crop(dnmin);
                arena = panel.y;
                index.resize(arena);
                index_rebuild();
                bufferbase::set_scroll_region(0, 0);
                sync_coord();
            }
            // scroll_buf: Set the scrolling region using 1-based top and bottom. Use 0 to reset.
            void set_scroll_region(si32 top, si32 bottom) override
            {
                auto old_sctop = sctop;
                auto old_scend = scend;

                bufferbase::set_scroll_region(top, bottom); // coord -> dot_00 -- coord is unsynced
                if (old_sctop == sctop && old_scend == scend)
                {
                    sync_coord();
                    return;
                }

                // Trim the existing margin content if any. The app that changed the margins is responsible for updating the content.
                upmin = { panel.x, sctop };
                dnmin = { panel.x, scend };
                auto delta_top = sctop - old_sctop;
                auto delta_end = scend - old_scend;

                // Take lines from the scrollback.
                auto pull = [&](face& block, twod origin, si32 begin, si32 limit)
                {
                    if (begin >= index.size) return;
                    limit = std::min(limit, index.size);
                    dissect(begin);
                    dissect(limit);
                    auto from = index[begin    ].index;
                    auto upto = index[limit - 1].index + 1;
                    auto base = batch.index_by_id(from);
                    auto head = batch.begin() + base;
                    auto size = static_cast<si32>(upto - from);
                    auto tail = head + size;
                    auto area = block.area();
                    block.full(area);
                    block.view(area);
                    block.ac(origin);
                    do
                    {
                        auto& curln = *head;
                        block.output(curln);
                        block.nl(1);
                    }
                    while (++head != tail);
                    batch.remove(base, size);
                };
                // Return lines to the scrollback iif margin is disabled.
                auto push = [&](face& block, bool at_bottom)
                {
                    auto size = block.size();
                    if (size.y <= 0) return;

                    auto start = si32{};
                    if (at_bottom)
                    {
                        auto stash = batch.vsize - batch.basis - arena;
                        if (stash > 0)
                        {
                            dissect(arena);
                            start = batch.index_by_id(index.back().index) + 1;
                        }
                        else // Add new lines if needed.
                        {
                            auto count = arena - index.size;
                            auto curid = batch.back().index;
                            while (count-- > 0) batch.invite(++curid, parser::style);
                            start = batch.size;
                        }
                    }
                    else
                    {
                        dissect(0);
                        start = batch.index_by_id(index.front().index);
                    }

                    auto curit = block.iter();
                    auto width = twod{ size.x, 1 };
                    auto curid = start == 0 ? batch.front().index
                                            : batch[start - 1].index + 1;
                    auto style = ansi::def_style;
                    style.wrp(wrap::off);
                    while (size.y-- > 0)
                    {
                        auto oldsz = batch.size;
                        //auto proto = core::span{ curit, (size_t)size.x };
                        auto proto = core::span{ &(*curit), (size_t)size.x }; //todo Clang 13.0.0 doesn't accept an iterator as an arg in the span ctor.
                        auto curln = line{ curid++, style, proto, width };
                        curln.shrink(block.mark());
                        batch.insert(start, std::move(curln));
                        start += batch.size - oldsz; // Due to circulation in the ring.
                        assert(start <= batch.size);
                        curit += size.x;
                    }
                    batch.reindex(start);
                };

                if (delta_end > 0)
                {
                    if (old_scend == 0) dnbox.mark(brush.spare);
                    dnbox.crop<true>(dnmin);
                    pull(dnbox, dot_00, arena - delta_end, arena);
                }
                else
                {
                    if (scend == 0 && old_scend > 0) push(dnbox, true);
                    dnbox.crop<true>(dnmin);
                }

                if (delta_top > 0)
                {
                    if (old_sctop == 0) upbox.mark(brush.spare);
                    upbox.crop<faux>(upmin);
                    pull(upbox, { 0, old_sctop }, 0, delta_top);
                    if (batch.size == 0) batch.invite(0, parser::style);
                }
                else
                {
                    if (sctop == 0 && old_sctop > 0) push(upbox, faux);
                    upbox.crop<faux>(upmin);
                }

                arena = panel.y - (scend + sctop);
                index.clear();
                index.resize(arena);
                index_rebuild();
                sync_coord();
            }
            // scroll_buf: Push empty lines to the scrollback bottom.
            void add_lines(si32 amount)
            {
                assert(amount >= 0);
                auto newid = batch.back().index;
                while (amount-- > 0)
                {
                    auto& l = batch.invite(++newid, parser::style);
                    index.push_back(newid, 0, 0);
                }
            }
            // scroll_buf: Push filled lines to the scrollback bottom.
            void add_lines(si32 amount, cell const& blank)
            {
                assert(amount >= 0);
                auto newid = batch.back().index;
                while (amount-- > 0)
                {
                    auto& l = batch.invite(++newid, parser::style, blank, panel.x);
                    index.push_back(newid, 0, panel.x);
                }
            }
            // scroll_buf: .
            template<feed Dir>
            auto xconv(si32 x, bias align, si32 remain)
            {
                // forward: screen -> offset
                // reverse: offset -> screen
                auto map = [](auto& a, auto b)
                {
                    Dir == feed::fwd ? a -= b
                                     : a += b;
                };
                switch (align)
                {
                    case bias::none:
                    case bias::left:   break;
                    case bias::right:  map(x, panel.x     - remain    ); break;
                    case bias::center: map(x, panel.x / 2 - remain / 2); break;
                };
                return x;
            }
            // scroll_buf: .
            auto screen_to_offset(line& curln, twod coor)
            {
                auto length = curln.length();
                auto adjust = curln.style.jet();
                if (curln.wrapped() && length > panel.x)
                {
                    auto endpos = length - 1;
                    auto height = endpos / panel.x;
                    auto remain = endpos % panel.x + 1;
                    coor.x = coor.y < height ? std::clamp(coor.x, 0, panel.x - 1)
                                             : std::clamp(xconv<feed::fwd>(coor.x, adjust, remain), 0, remain - 1);
                    coor.x+= coor.y * panel.x;
                }
                else
                {
                    coor.x = std::clamp(xconv<feed::fwd>(coor.x, adjust, length), 0, length ? length - 1 : 0);
                }
                return coor.x;
            }
            // scroll_buf: .
            auto offset_to_screen(line& curln, si32 offset)
            {
                auto size = curln.length();
                auto last = size ? size - 1 : 0;
                auto coor = twod{ std::clamp(offset, 0, last), 0 };
                if (size > 1 && curln.wrapped())
                {
                    coor.y = coor.x / panel.x;
                    coor.x = coor.x % panel.x;
                    if (coor.y < last / panel.x) return coor;
                    size = last % panel.x + 1;
                }
                coor.x = xconv<feed::rev>(coor.x, curln.style.jet(), size);
                return coor;
            }
            // scroll_buf: Update current SGR attributes. (! Check coord.y context)
            void _set_style(deco const& new_style)
            {
                auto& curln = batch.current();
                auto  wraps = curln.wrapped();
                auto  width = curln.length();
                curln.style = new_style;

                if (batch.caret > width) // Dangling cursor.
                {
                    if (!wraps || coord.x <= panel.x) // Extend the line if the cursor is inside the viewport.
                    {
                        width = batch.caret;
                        curln.crop(width, brush.dry());
                    }
                    else // Move coord.x inside viewport for wrapped lines (cursor came from another (unwrapped) line).
                    {
                        auto& mapln = index[coord.y];
                        batch.caret = mapln.start + panel.x;
                        mapln.width = panel.x;
                        coord.x = panel.x;
                        curln.crop(batch.caret, brush.dry());
                    }
                }

                batch.recalc(curln);

                if (wraps != curln.wrapped())
                {
                    if (batch.caret >= panel.x)
                    {
                        if (wraps)
                        {
                            if (coord.x == 0) coord.y -= 1;
                            coord.x  =  batch.caret;
                            coord.y -= (batch.caret - 1) / panel.x;
                            if (coord.y < 0)
                            {
                                batch.basis -= std::abs(coord.y);
                                assert(batch.basis >= 0);
                                coord.y = 0;
                            }
                        }
                        else
                        {
                            if (batch.caret == width)
                            {
                                coord.x  = (batch.caret - 1) % panel.x + 1;
                                coord.y += (batch.caret - 1) / panel.x;
                            }
                            else
                            {
                                coord.x  = batch.caret % panel.x;
                                coord.y += batch.caret / panel.x;
                            }

                            if (coord.y >= arena)
                            {
                                auto limit = arena - 1;
                                auto delta = coord.y - limit;
                                batch.basis += delta;
                                coord.y = limit;
                            }
                        }
                        index_rebuild();
                    }
                    else
                    {
                        auto& mapln = index[coord.y];
                        auto  wraps = curln.wrapped();
                        mapln.start = 0;
                        mapln.width = wraps ? std::min(panel.x, width)
                                            : width;
                        index_rebuild_from(coord.y);
                    }
                }
                assert(test_index());
            }
            // scroll_buf: Proceed style update (parser callback).
            void meta(deco const& old_style) override
            {
                if (batch->style != parser::style)
                {
                    if (coord.y >= y_top
                     && coord.y <= y_end)
                    {
                        coord.y -= y_top;
                        _set_style(parser::style);
                        coord.y += y_top;
                    }
                }
                bufferbase::meta(old_style);
            }
            [[ nodiscard ]]
            auto get_context(twod& pos)
            {
                struct qt
                {
                    twod& c;
                    si32  t;
                    bool  b;
                    rich& block;
                    qt(twod& cy, si32 ct, bool cb, scroll_buf& cs)
                        : c{ cy },
                          t{ ct },
                          b{ cb },
                      block{ c.y > cs.y_end ? (void)(t = cs.y_end + 1), cs.dnbox
                                            :                           cs.upbox }
                         { c.y -= t; }
                   ~qt() { c.y += t; }
                    operator bool () { return b; }
                };
                auto inside = coord.y >= y_top
                           && coord.y <= y_end;
                return qt{ pos, inside ? y_top : 0, inside, *this };
            }
            // scroll_buf: CSI n K  Erase line (don't move cursor).
            void el(si32 n) override
            {
                bufferbase::flush();
                //todo revise - nul() or dry()
                //auto blank = brush.dry();
                auto blank = brush.nul();
                if (auto ctx = get_context(coord))
                {
                    auto  start = si32{};
                    auto  count = si32{};
                    auto  caret = std::max(0, batch.caret);
                    auto& curln = batch.current();
                    auto  width = curln.length();
                    auto  wraps = curln.wrapped();
                    switch (n)
                    {
                        default:
                        case commands::erase::line::right: // n = 0 (default)  Erase to Right.
                            start = caret;
                            count = wraps ? coord.x == panel.x ? 0 : panel.x - (caret + panel.x) % panel.x
                                          : std::max(0, std::max(panel.x, width) - caret);
                            break;
                        case commands::erase::line::left: // n = 1  Erase to Left.
                            start = wraps ? caret - caret % panel.x
                                          : 0;
                            count = caret - start + 1; // +1 to include the current cell.
                            break;
                        case commands::erase::line::all: // n = 2  Erase All.
                            start = wraps ? caret - caret % panel.x
                                          : 0;
                            count = wraps ? panel.x
                                          : std::max(panel.x, batch->length());
                            break;
                        case commands::erase::line::wraps: // n = 3  Erase wrapped line.
                            start = caret;
                            count = width - caret;
                            break;
                    }
                    if (count)
                    {
                        curln.splice<true>(start, count, blank);

                        batch.recalc(curln);
                        width = curln.length();
                        auto& mapln = index[coord.y];
                        mapln.width = wraps ? std::min(panel.x, width - mapln.start)
                                            : width;

                        //curln.shrink(blank); //todo revise: It kills wrapped lines and as a result requires the viewport to be rebuilt.
                        //batch.recalc(curln); //             The cursor may be misplaced when resizing because curln.length is less than batch.caret.
                        //index_rebuild();
                        //print_batch(" el");
                    }
                }
                else alt_screen::_el(n, ctx.block, coord, panel, blank);
            }
            // scroll_buf: CSI n @  ICH. Insert n blanks after cursor. Existing chars after cursor shifts to the right. Don't change cursor pos.
            void ins(si32 n) override
            {
                bufferbase::flush();
                auto blank = brush.nul();
                if (auto ctx = get_context(coord))
                {
                    n = std::min(n, panel.x - coord.x);
                    auto& curln = batch.current();
                    curln.insert(batch.caret, n, blank, panel.x);
                    batch.recalc(curln); // Line front is filled by blanks. No wrapping.
                    auto  width = curln.length();
                    auto  wraps = curln.wrapped();
                    auto& mapln = index[coord.y];
                    mapln.width = wraps ? std::min(panel.x, width - mapln.start)
                                        : width;
                }
                else ctx.block.insert(coord, n, blank);
            }
            // scroll_buf: CSI n P  Delete (not Erase) letters under the cursor. Line end is filled by blanks. Length is preserved. No wrapping.
            void dch(si32 n) override
            {
                bufferbase::flush();
                auto blank = brush.nul();
                if (auto ctx = get_context(coord))
                {
                    auto& curln = batch.current();
                    curln.cutoff(batch.caret, n, blank, panel.x);
                    batch.recalc(curln);
                }
                else ctx.block.cutoff(coord, n, blank);
            }
            // scroll_buf: Move internal caret by count with wrapping.
            void _fwd(si32 count)
            {
                if (count > 0)
                {
                    if (coord.y < y_top)
                    {
                        coord.x += count;
                        if (coord.x > panel.x)
                        {
                            wrapdn();
                            if (coord.y >= y_top)
                            {
                                count -= coord.x + (coord.y - y_top) * panel.x;
                                set_coord(twod{ 0, y_top });
                                _fwd(count);
                            }
                        }
                    }
                    else if (coord.y <= y_end)
                    {
                        auto& curln = batch.current();
                        coord.x += count;
                        if (coord.x >= panel.x && curln.wrapped())
                        {
                            wrapdn();
                            if (coord.y > y_end)
                            {
                                batch.basis += coord.y - y_end;
                                coord.y = y_end;
                                index_rebuild();
                            }
                        }
                    }
                    else
                    {
                        coord.x += count;
                        if (coord.x > panel.x)
                        {
                            wrapdn();
                            if (coord.y >= panel.y) coord = panel - dot_01;
                        }
                    }
                }
                else if (count < 0)
                {
                    if (coord.y < y_top)
                    {
                        coord.x += count;
                        if (coord.x < 0)
                        {
                            wrapup();
                            if (coord.y < 0)
                            {
                                coord.y += coord.x / panel.x;
                                coord.x  = coord.x % panel.x;
                                if (coord.y < 0) coord = dot_00;
                            }
                        }
                    }
                    else if (coord.y <= y_end)
                    {
                        auto& curln = batch.current();
                        coord.x += count;
                        if (coord.x < 0 && curln.wrapped())
                        {
                            wrapup(); //failed for CUF(-(panel.x + 1)) at coord.x = 1
                            if (coord.y < y_top)
                            {
                                coord.y += coord.x / panel.x;
                                coord.x  = coord.x % panel.x;
                                if (coord.y < y_top)
                                {
                                    batch.basis -= y_top - coord.y;
                                    if (batch.basis < 0) batch.basis = 0;
                                    coord.y = y_top;
                                    index_rebuild();
                                }
                            }
                        }
                    }
                    else
                    {
                        coord.x += count;
                        if (coord.x < 0)
                        {
                            wrapup();
                            if (coord.y <= y_end)
                            {
                                count += coord.x + (coord.y - y_end) * panel.x;
                                set_coord(twod{ panel.x, y_end });
                                _fwd(count);
                            }
                        }
                    }
                }

                if (count) sync_coord();
                auto& curln = batch.current();
                if (coord.x == panel.x && curln.wrapped() && batch.caret < curln.length())
                {
                    coord.x = 0;
                    coord.y++;
                    if (coord.y > y_end)
                    {
                        batch.basis += coord.y - y_end;
                        coord.y = y_end;
                        index_rebuild();
                    }
                }
                assert(test_coord());
            }
            // scroll_buf: '\x7F'  Delete letters backward and move cursor back.
            void del(si32 n) override
            {
                bufferbase::flush();
                n = std::min(n, batch.caret);
                if (batch.caret > 0 && n > 0)
                {
                    _fwd(-n);
                    auto& curln = batch.current();
                    curln.splice<faux>(batch.caret, n, brush.nul());
                }
            }
            // scroll_buf: Move cursor by n in line.
            void move(si32 n) override
            {
                bufferbase::flush();
                _fwd(n);
            }
            // scroll_buf: CSI n X  Erase/put n chars after cursor. Don't change cursor pos.
            void ech(si32 n, char c = '\0') override
            {
                parser::flush();
                auto blank = brush;
                blank.txt(c);
                if (auto ctx = get_context(coord))
                {
                    n = std::min(n, panel.x - coord.x);
                    auto& curln = batch.current();
                    //todo revise (brush != default ? see windows console)
                    //if (c == whitespace) curln.splice<faux>(batch.caret, n, blank);
                    //else                 curln.splice<true>(batch.caret, n, blank);
                    curln.splice<true>(batch.caret, n, blank);
                    batch.recalc(curln);
                    auto& mapln = index[coord.y];
                    auto  width = curln.length();
                    auto  wraps = curln.wrapped();
                    mapln.width = wraps ? std::min(panel.x, curln.length() - mapln.start)
                                        : width;
                }
                else ctx.block.splice(coord, n, blank);
            }
            // scroll_buf: Merge curln with its neighbors.
            void _merge(line& curln, si32 oldsz, ui32 curid, si32 count)
            {
                auto coor = oldsz + panel.x - (oldsz - 1) % panel.x - 1;
                auto iter = batch.iter_by_id(curid);
                while (count-- > 0)
                {
                    auto& line = *++iter;
                    //todo respect line alignment
                    if (line.wrapped()) curln.splice(coor, line                   , cell::shaders::full);
                    else                curln.splice(coor, line.substr(0, panel.x), cell::shaders::full);
                    coor += line.height(panel.x) * panel.x;
                }
            }
            // scroll_buf: Proceed new text using specified cell shader.
            template<bool Copy = faux, class Span, class Shader>
            void _data(si32 count, Span const& proto, Shader fuse)
            {
                static constexpr auto mixer = !std::is_same_v<Shader, decltype(cell::shaders::full)>;

                assert(coord.y >= 0 && coord.y < panel.y);
                assert(test_futures());
                assert(test_coord());

                if (coord.y < y_top)
                {
                    auto saved = coord;
                    coord.x += count;
                    //todo apply line adjusting (necessity is not clear)
                    if (coord.x <= panel.x)//todo styles! || ! curln.wrapped())
                    {
                        auto n = std::min(count, panel.x - std::max(0, saved.x));
                        upbox.splice<Copy>(saved, n, proto, fuse);
                    }
                    else
                    {
                        wrapdn();
                        if (coord.y >= y_top)
                        {
                            auto n = coord.x + (coord.y - y_top) * panel.x;
                            count -= n;
                            set_coord(twod{ 0, y_top });
                            _data<Copy>(n, proto, fuse); // Reversed fill using the last part of the proto.
                        }
                        auto data = proto.begin();
                        auto seek = saved.x + saved.y * panel.x;
                        auto dest = upbox.iter() + seek;
                        auto tail = dest + count;
                        rich::forward_fill_proc<Copy>(data, dest, tail, fuse);
                    }
                    // Note: coord can be unsync due to scroll regions.
                }
                else if (coord.y <= y_end)
                {
                    coord.y -= y_top;
                    auto& curln = batch.current();
                    auto  start = batch.caret;
                    batch.caret += count;
                    coord.x     += count;
                    if (batch.caret <= panel.x || !curln.wrapped()) // case 0.
                    {
                        curln.splice<Copy>(start, count, proto, fuse);
                        auto& mapln = index[coord.y];
                        assert(coord.x % panel.x == batch.caret % panel.x && mapln.index == curln.index);
                        if (coord.x > mapln.width)
                        {
                            mapln.width = coord.x;
                            batch.recalc(curln);
                        }
                        else assert(curln._size == curln.length());
                    } // case 0 - done.
                    else
                    {
                        auto max_y = arena - 1;
                        auto saved = coord.y + batch.basis;
                        wrapdn();

                        auto query = coord.y - (index.size - 1);
                        if (query > 0)
                        {
                            auto avail = feed_futures(query);
                            query   -= avail;
                            coord.y -= avail;
                        }

                        auto oldsz = curln.length();
                        auto curid = curln.index;
                        if (query > 0) // case 3 - complex: Cursor is outside the viewport.
                        {              // cursor overlaps some lines below and placed below the viewport.
                            curln.resize(batch.caret);
                            batch.recalc(curln);
                            if (auto count = static_cast<si32>(batch.back().index - curid))
                            {
                                if constexpr (mixer) _merge(curln, oldsz, curid, count);
                                assert(count > 0);
                                while (count-- > 0) batch.pop_back();
                            }

                            auto width = curln.length();
                            auto trail = width - panel.x;
                            auto start = panel.x;

                            saved -= batch.basis;
                            if (saved > 0)
                            {
                                auto count = index.size - saved - 1;
                                while (count-- > 0) index.pop_back();
                                auto& mapln = index.back();
                                mapln.width = panel.x;
                                start += mapln.start;
                            }
                            else // saved has scrolled out.
                            {
                                index.clear();
                                start *= std::abs(saved);
                            }

                            while (start < trail)
                            {
                                index.push_back(curid, start, panel.x);
                                start += panel.x;
                            }
                            index.push_back(curid, start, width - start);

                            if (coord.y > max_y)
                            {
                                batch.basis += coord.y - max_y;
                                coord.y = max_y;
                            }

                            assert(test_futures());
                        } // case 3 done
                        else
                        {
                            auto& mapln = index[coord.y];
                            if (curid == mapln.index) // case 1 - plain: cursor is inside the current paragraph.
                            {
                                curln.resize(batch.caret);
                                if (batch.caret - coord.x == mapln.start)
                                {
                                    if (coord.x > mapln.width)
                                    {
                                        mapln.width = coord.x;
                                        batch.recalc(curln);
                                    }
                                    else assert(curln._size == curln.length());
                                }
                                else // The case when the current line completely fills the viewport (arena == 1).
                                {
                                    assert(arena == 1);
                                    mapln.start = batch.caret - coord.x;
                                    mapln.width = coord.x;
                                    batch.recalc(curln);
                                }
                                assert(test_futures());
                            } // case 1 done.
                            else // case 2 - fusion: cursor overlaps lines below but stays inside the viewport.
                            {
                                auto& destln = batch.item_by_id(mapln.index);
                                //todo respect destln alignment
                                auto  shadow = destln.wrapped() ? destln.substr(mapln.start + coord.x)
                                                                : destln.substr(mapln.start + coord.x, std::min(panel.x, mapln.width) - coord.x);

                                if constexpr (mixer) curln.resize(batch.caret +shadow.length());
                                else                 curln.splice(batch.caret, shadow, cell::shaders::full);

                                batch.recalc(curln);
                                auto width = curln.length();
                                auto spoil = static_cast<si32>(mapln.index - curid);
                                assert(spoil > 0);

                                if constexpr (mixer) _merge(curln, oldsz, curid, spoil);

                                auto after = batch.index() + 1;
                                     spoil = batch.remove(after, spoil);

                                if (saved < batch.basis)
                                {
                                    index_rebuild(); // Update index. (processing lines larger than viewport)
                                }
                                else
                                {
                                    saved -= batch.basis;
                                    auto indit = index.begin() + saved;
                                    auto endit = index.end();
                                    auto start = indit->start;
                                    auto trail = width - panel.x;
                                    while (indit != endit && start < trail) // Update for current line.
                                    {
                                        auto& i =*indit;
                                        i.index = curid;
                                        i.start = start;
                                        i.width = panel.x;
                                        start  += panel.x;
                                        ++indit;
                                    }
                                    if (indit != endit)
                                    {
                                        auto& i =*indit;
                                        i.index = curid;
                                        i.start = start;
                                        i.width = width - start;
                                        ++indit;
                                        while (indit != endit) // Update the rest.
                                        {
                                            auto& i = *indit;
                                            i.index -= spoil;
                                            ++indit;
                                        }
                                    }
                                    assert(test_index());
                                }
                                assert(test_futures());
                            } // case 2 done.
                        }
                        batch.current().splice<Copy>(start, count, proto, fuse);
                    }
                    assert(coord.y >= 0 && coord.y < arena);
                    coord.y += y_top;
                }
                else
                {
                    coord.y -= y_end + 1;
                    auto saved = coord;
                    coord.x += count;
                    //todo apply line adjusting
                    if (coord.x <= panel.x)//todo styles! || ! curln.wrapped())
                    {
                        auto n = std::min(count, panel.x - std::max(0, saved.x));
                        dnbox.splice<Copy>(saved, n, proto, fuse);
                    }
                    else
                    {
                        wrapdn();
                        auto data = proto.begin();
                        auto size = count;
                        auto seek = saved.x + saved.y * panel.x;
                        auto dest = dnbox.iter() + seek;
                        auto tail = dnbox.iend();
                        auto back = panel.x;
                        rich::unlimit_fill_proc<Copy>(data, size, dest, tail, back, cell::shaders::full);
                    }
                    coord.y = std::min(coord.y + y_end + 1, panel.y - 1);
                    // Note: coord can be unsync due to scroll regions.
                }
                assert(test_coord());
            }
            // scroll_buf: Proceed new text (parser callback).
            void data(si32 count, grid const& proto) override
            {
                _data(count, proto, cell::shaders::skipnuls);
            }
            // scroll_buf: Clear scrollback.
            void clear_all() override
            {
                batch.clear();
                reset_scroll_region();
                bufferbase::clear_all();
                resize_history(owner.config.def_length, owner.config.def_growdt, owner.config.def_growmx);
            }
            // scroll_buf: Set scrollback limits.
            void resize_history(si32 new_size, si32 grow_by = 0, si32 grow_mx = 0)
            {
                static constexpr auto BOTTOM_ANCHORED = true;
                batch.resize<BOTTOM_ANCHORED>(std::max(new_size, panel.y), grow_by, grow_mx);
                index_rebuild();
            }
            // scroll_buf: Render to the canvas.
            void output(face& dest) override
            {
                dest.vsize(batch.vsize + sctop + scend); // Include margins and bottom oversize.
                auto view = dest.view();
                auto full = dest.full();
                auto coor = twod{ 0, batch.slide - batch.ancdy + y_top };
                auto stop = view.coor.y + view.size.y;
                auto head = batch.iter_by_id(batch.ancid);
                auto tail = batch.end();
                auto find = selection_active() && match.length() && owner.selmod == mime::textonly;
                auto fill = [&](auto& area, auto chr)
                {
                    if (auto r = view.clip(area))
                    {
                        dest.fill(r, [&](auto& c){ c.txt(chr).fgc(tint::greenlt); });
                    }
                };
                auto left_edge = view.coor.x;
                auto rght_edge = view.coor.x + view.size.x;
                auto half_size = full.size.x / 2;
                auto left_rect = rect{{ left_edge, full.coor.y + coor.y }, dot_11 };
                auto rght_rect = left_rect;
                rght_rect.coor.x += view.size.x - 1;

                while (head != tail && rght_rect.coor.y < stop)
                {
                    auto& curln = *head;
                    auto height = curln.height(panel.x);
                    auto length = curln.length();
                    auto adjust = curln.style.jet();
                    dest.output(curln, coor);
                    //dest.output_proxy(curln, coor, [&](auto const& coord, auto const& subblock, auto isr_to_l)
                    //{
                    //    dest.text(coord, subblock, isr_to_l, cell::shaders::fusefull);
                    //});
                    if (find)
                    {
                        match.style.wrp(curln.style.wrp());
                        auto offset = si32{ 0 };
                        auto work = [&](auto shader)
                        {
                            while (curln.find(match, offset))
                            {
                                auto c = coor + offset_to_screen(curln, offset);
                                dest.output(match, c, shader);
                                offset += match.length();
                            }
                        };
                        _shade(owner.config.def_find_f, owner.config.def_find_c, work);
                    }

                    if (length > 0) // Highlight the lines that are not shown in full.
                    {
                        rght_rect.size.y = left_rect.size.y = height;
                        if (height == 1)
                        {
                            auto lt_dot = full.coor.x;
                                 if (adjust == bias::center) lt_dot += half_size - length / 2;
                            else if (adjust == bias::right)  lt_dot += full.size.x - length;

                            if (left_edge > lt_dot         ) fill(left_rect, '<');
                            if (rght_edge < lt_dot + length) fill(rght_rect, '>');
                        }
                        else
                        {
                            auto lt_dot = full.coor.x;
                            auto rt_dot = lt_dot + view.size.x;
                            auto remain = (length - 1) % view.size.x + 1;
                            if (left_edge > lt_dot)
                            {
                                if ((adjust == bias::right  && left_edge <= rt_dot - remain)
                                 || (adjust == bias::center && left_edge <= lt_dot + half_size - remain / 2))
                                {
                                    --left_rect.size.y;
                                }
                                fill(left_rect, '<');
                            }
                            if (rght_edge < rt_dot)
                            {
                                if ((adjust == bias::left   && rght_edge >= lt_dot + remain)
                                 || (adjust == bias::center && rght_edge >= lt_dot + remain + half_size - remain / 2))
                                {
                                    --rght_rect.size.y;
                                }
                                fill(rght_rect, '>');
                            }
                        }
                    }
                              coor.y += height;
                    rght_rect.coor.y += height;
                    left_rect.coor.y = rght_rect.coor.y;
                    ++head;
                }

                auto top_coor = twod{ view.coor.x, view.coor.y + y_top - sctop }; //todo ? replace view.coor.y with batch.slide
                auto end_coor = twod{ view.coor.x, view.coor.y + y_end + 1     };
                upbox.move(top_coor);
                dnbox.move(end_coor);
                dest.fill(upbox, cell::shaders::xlucent(owner.config.def_lucent));
                dest.fill(dnbox, cell::shaders::xlucent(owner.config.def_lucent));
                if (find && panel.y != arena)
                {
                    auto draw = [&](auto const& block)
                    {
                        if (auto area = block.area())
                        {
                            dest.full(area);
                            auto offset = si32{};
                            auto work = [&](auto shader)
                            {
                                while (block.find(match, offset))
                                {
                                    auto c = block.toxy(offset);
                                    dest.output(match, c, shader);
                                    offset += match.length();
                                }
                            };
                            _shade(owner.config.def_find_f, owner.config.def_find_c, work);
                        }
                    };
                    draw(upbox);
                    draw(dnbox);
                    dest.full(full);
                }

                selection_render(dest);
            }
            // scroll_buf: Remove all lines below (including futures) except the current. "ED2 Erase viewport" keeps empty lines.
            void del_below() override
            {
                assert(test_futures());

                auto blank = brush.dry();
                auto clear = [&](twod coor)
                {
                    auto& from = index[coor.y];
                    auto topid = from.index;
                    auto start = from.start;
                    auto i = batch.index_by_id(topid);
                    auto n = batch.size - 1 - i;      // The number of paragraphs below that should pop.
                    auto m = index.size - 1 - coor.y; // The number of visible rows to zero out.
                    auto p = arena      - 1 - coor.y; // The number of new empty rows to add.

                    auto fresh = coor.x == 0 && start != 0;
                    if (fresh) // Remove the index of the current line if the entire visible line is going to be removed.
                    {
                        ++m;
                        ++p;
                    }

                    assert(n >= 0 && n <  batch.size);
                    assert(m >= 0 && m <= index.size);
                    assert(p >= 0 && p <= arena);

                    while (n--) batch.pop_back();
                    while (m--) index.pop_back();

                    auto fills = blank.bgc() != brush.fresh.bgc();
                    if (fills) add_lines(p, blank); // Fill with non-default background.
                    else       add_lines(p);

                    i = batch.index_by_id(topid); // The index may be outdated due to the ring.
                    auto& curln = batch[i];
                    if (fresh)
                    {
                        curln.trimto(start);
                    }
                    else
                    {
                        auto& mapln = index[coor.y];
                        if (fills)
                        {
                            mapln.width = panel.x;
                            auto x = std::min(coor.x, panel.x); // Trim unwrapped lines by viewport.
                            curln.splice<true>(start + x, panel.x - x, blank);
                            curln.trimto(start + panel.x);
                        }
                        else
                        {
                            mapln.width = coor.x;
                            curln.trimto(start + coor.x);
                        }
                        assert(mapln.start == 0 || curln.wrapped());
                    }
                    batch.recalc(curln);

                    sync_coord();

                    auto stash = batch.vsize - batch.basis - index.size;
                    assert(stash == 0);
                    assert(test_futures());
                    dnbox.wipe(blank);
                };

                auto coor = coord;
                if (coor.y < y_top)
                {
                    assert(coor.x + coor.y * upbox.size().x < sctop * upbox.size().x);
                    upbox.del_below(coor, blank);
                    clear(dot_00);
                }
                else if (coor.y <= y_end)
                {
                    coor.x = std::max(0, coor.x);
                    coor.y -= y_top;
                    clear(coor);
                }
                else
                {
                    coor.y -= y_end + 1;
                    assert(coor.x + coor.y * dnbox.size().x < scend * dnbox.size().x);
                    dnbox.del_below(coor, blank);
                }
            }
            // scroll_buf: Clear all lines from the viewport top line to the current line.
            void del_above() override
            {
                auto blank = brush.dry();
                auto clear = [&](twod from)
                {
                    auto head = index.begin();
                    auto tail = head + from.y;
                    while (head != tail)
                    {
                        auto& mapln = *head++;
                        auto& curln = batch.item_by_id(mapln.index);
                        mapln.width = panel.x;
                        curln.splice<true>(mapln.start, panel.x, blank);
                        batch.recalc(curln);
                    }
                    if (from.x > 0)
                    {
                        auto& mapln = *head;
                        auto& curln = batch.item_by_id(mapln.index);
                        mapln.width = std::max(mapln.width, from.x);
                        curln.splice<true>(mapln.start, from.x, blank);
                        batch.recalc(curln);
                    }
                    upbox.wipe(blank);
                };

                auto coor = coord;
                if (coor.x < panel.x) coor.x += 1; // Clear the cell at the current position. See ED1 description.
                if (coor.y < y_top)
                {
                    assert(coor.x + coor.y * upbox.size().x < sctop * upbox.size().x);
                    upbox.del_above(coor, blank);
                }
                else if (coor.y <= y_end)
                {
                    coor.x = std::clamp(coor.x, 0, panel.x);
                    coor.y -= y_top;
                    clear(coor);
                }
                else
                {
                    coor.y -= y_end + 1;
                    assert(coor.x + coor.y * dnbox.size().x < scend * dnbox.size().x);
                    dnbox.del_above(coor, blank);
                    clear(twod{ panel.x , arena - 1 });
                }
            }
            // scroll_buf: Dissect auto-wrapped lines above the specified row in scroll region (incl last line+1).
            void dissect(si32 y_pos)
            {
                assert(y_pos >= 0 && y_pos <= arena);

                auto split = [&](id_t curid, si32 start)
                {
                    auto after = batch.index_by_id(curid);
                    auto tmpln = std::move(batch[after]);
                    auto curit = batch.ring::insert(after + 1, tmpln.index, tmpln.style);
                    auto endit = batch.end();

                    auto& newln = *curit;
                    newln.splice(0, tmpln.substr(start), cell::shaders::full);
                    batch.undock_base_back(tmpln);
                    batch.invite(newln);

                    if (curit != batch.begin())
                    {
                        auto& curln = *(curit - 1);
                        curln = std::move(tmpln);
                        curln.trimto(start);
                        batch.invite(curln);
                    }

                    do  ++(curit++->index);
                    while (curit != endit);

                    assert(test_index());
                };

                if (y_pos < index.size)
                {
                    auto& mapln = index[y_pos];
                    auto  start = mapln.start;
                    auto  curid = mapln.index;
                    if (start == 0) return;

                    split(curid, start);

                    mapln.index++;
                    mapln.start = 0;
                    index_rebuild_from(y_pos);

                    sync_coord();
                }
                else if (y_pos == arena
                      && y_pos <  batch.vsize - batch.basis)
                {
                    auto stash = batch.vsize - batch.basis - index.size;
                    assert(stash >= 0);
                    if (stash == 0) return;

                    auto& mapln = index.back();
                    auto  curid = mapln.index;
                    auto& curln = batch.item_by_id(curid);
                    auto  start = mapln.start + mapln.width;
                    if (start < curln.length())
                    {
                        split(curid, start);
                    }
                }

                assert(test_futures());
                // Note: coord is unsynced -- see set_scroll_region()
            }
            // scroll_buf: Scroll the specified region by n lines. The scrollback can only be used with the whole scrolling region.
            void scroll_region(si32 top, si32 end, si32 n, bool use_scrollback) override
            {
                assert(top >= y_top && end <= y_end);

                if (n == 0) return;

                auto stash = arena - (batch.vsize - batch.basis);
                if (stash > 0) add_lines(stash); // Fill-up the scrolling region in order to simplify implementation (dissect() requirement).
                assert(arena == index.size);

                auto count = std::abs(n);
                if (n < 0) // Scroll text up.
                {
                    if (top == y_top &&
                        end == y_end && use_scrollback)
                    {
                        count -= feed_futures(count);
                        if (count > 0)
                        {
                            add_lines(count);
                            // Cut, as the ring is used.
                            batch.basis = std::min(batch.basis + count, batch.vsize - arena);
                        }
                    }
                    else
                    {
                        top -= y_top;
                        end -= y_top - 1;

                        auto max = end - top;
                        if (count > max) count = max;

                        auto mdl = top + count;
                        dissect(top);
                        dissect(mdl);
                        dissect(end);

                        // Delete block.
                        auto topid = index[top    ].index;
                        auto mdlid = index[mdl - 1].index + 1;
                        auto endid = index[end - 1].index + 1;
                        auto start = batch.index_by_id(topid);
                        auto range = static_cast<si32>(mdlid - topid);
                        auto floor = batch.index_by_id(endid) - range;
                        batch.remove(start, range);

                        // Insert block.
                        while (count-- > 0) batch.insert(floor, id_t{}, parser::style);

                        batch.reindex(start); //todo revise ? The index may be outdated due to the ring.
                        index_rebuild();
                    }
                }
                else // Scroll text down.
                {
                    if (top == y_top &&
                        end == y_end && use_scrollback && batch.basis >= n) // Just move the viewport up.
                    {
                        batch.basis -= n;
                    }
                    else
                    {
                        top -= y_top;
                        end -= y_top - 1;

                        auto max = end - top;
                        if (count > max) count = max;

                        auto mdl = end - count;
                        dissect(top);
                        dissect(mdl);
                        dissect(end);

                        // Delete block.
                        auto topid = index[top    ].index;
                        auto endid = index[end - 1].index + 1;
                        auto mdlid = mdl > 0 ? index[mdl - 1].index + 1 // mdl == 0 or mdl == top when count == max (full arena).
                                             : topid;
                        auto start = batch.index_by_id(topid);
                        auto range = static_cast<si32>(endid - mdlid);
                        auto floor = batch.index_by_id(endid) - range;
                        batch.remove(floor, range);

                        // Insert block.
                        while (count-- > 0) batch.insert(start, id_t{}, parser::style);

                        batch.reindex(start); //todo revise ? The index may be outdated due to the ring.
                    }
                    index_rebuild();
                }

                assert(test_futures());
                assert(test_coord());
            }

            // scroll_buf: Calc grip position by coor.
            auto selection_coor_to_grip(twod coord, grip::type state = grip::base)
            {
                auto index = batch.front().index;
                if (coord.y < 0)
                {
                    return grip{ .link = index,
                                 .coor = coord,
                                 .role = state };
                }
                auto i_cur = batch.index_by_id(batch.ancid);
                assert(i_cur < batch.size);
                auto vtpos = batch.slide - batch.ancdy + y_top;
                auto mxpos = batch.slide + panel.y;
                auto start = batch.begin() + i_cur;
                auto limit = batch.end();
                while (vtpos < mxpos)
                {
                    auto& curln = *start;
                    auto newpos = vtpos + curln.height(panel.x);
                    if ((coord.y >= vtpos && coord.y < newpos) || ++start == limit)
                    {
                        index = curln.index;
                        break;
                    }
                    vtpos = newpos;
                }
                coord.y -= vtpos;
                return grip{ .link = index,
                             .coor = coord,
                             .role = state };
            }
            // scroll_buf: Return scrollbuffer grips.
            auto selection_take_grips()
            {
                if (upmid.role == grip::idle) return std::pair{ dot_mx, dot_mx };
                auto i_cur = batch.index_by_id(batch.ancid);
                auto i_top = batch.index_by_id(upmid.link);
                auto i_end = batch.index_by_id(dnmid.link);
                if (i_top < 0 && i_end < 0)
                {
                    selection_cancel();
                    return std::pair{ dot_mx, dot_mx };
                }
                auto coor1 = upmid.coor;
                auto coor2 = dnmid.coor;
                auto start = batch.begin() + i_cur;
                auto limit = batch.end();
                auto topid = batch.front().index;
                auto endid = batch.back().index;
                auto vtpos = batch.slide - batch.ancdy + y_top;
                auto mxpos = batch.slide + panel.y;
                auto done1 = true;
                auto done2 = true;
                auto check = [&](auto height, auto& curln, auto& undone, auto& grip, auto& coor)
                {
                    if (undone && grip.link == curln.index)
                    {
                        undone = faux;
                        if (grip.coor.y >= height && grip.link != endid) // Try to re anchor it.
                        {
                            auto head = start + 1;
                            auto ypos = grip.coor.y - height;
                            assert(head != limit);
                            while (true)
                            {
                                auto& curln = *head;
                                auto height = curln.height(panel.x);
                                if (ypos < height || ++head == limit)
                                {
                                    grip.link = curln.index;
                                    grip.coor.y = ypos;
                                    break;
                                }
                                ypos -= height;
                            }
                        }
                        coor.y = vtpos + grip.coor.y;
                    }
                };
                // Check the buffer ring.
                     if (i_top < 0)           upmid.link = topid;
                else if (i_top >= batch.size) upmid.link = endid;
                     if (i_end < 0)           dnmid.link = topid;
                else if (i_end >= batch.size) dnmid.link = endid;

                coor1.y = i_top < i_cur ? -dot_mx.y : dot_mx.y;
                coor2.y = i_end < i_cur ? -dot_mx.y : dot_mx.y;

                while (start != limit && vtpos < mxpos && (done1 || done2))
                {
                    auto& curln = *start;
                    auto height = curln.height(panel.x);
                    check(height, curln, done1, upmid, coor1);
                    check(height, curln, done2, dnmid, coor2);
                    vtpos += height;
                    ++start;
                }

                auto square = owner.actual_area<faux>();
                auto minlim = square.coor;
                auto maxlim = square.coor + std::max(dot_00, square.size - dot_11);
                coor1 = std::clamp(coor1, minlim, maxlim);
                coor2 = std::clamp(coor2, minlim, maxlim);
                return std::pair{ coor1, coor2 };
            }
            // scroll_buf: Start text selection.
            void selection_create(twod coor, bool mode) override
            {
                auto nohits = [&](auto upcoor, auto dncoor, auto& top, auto& end)
                {
                    if (coor == upcoor && top.role == grip::base)// std::swap(top, end);
                    {
                        std::swap(uptop, dntop);
                        std::swap(upmid, dnmid);
                        std::swap(upend, dnend);
                    }
                    else if (coor != dncoor || end.role != grip::base) return true;
                    selection_locked(true);
                    return faux;
                };
                auto scrolling_margin = batch.slide + y_top;
                if (coor.y < scrolling_margin) // Inside the top margin.
                {
                    place = part::top;
                    coor -= {-owner.origin.x, batch.slide };
                    if (!selection_active() || nohits(uptop.coor, dntop.coor, uptop, dntop))
                    {
                        upmid.role = dnmid.role = grip::idle;
                        upend.role = dnend.role = grip::idle;
                        uptop.role = grip::base;
                        uptop.coor = coor;
                        dntop = uptop;
                    }
                }
                else if (coor.y < scrolling_margin + arena) // Inside the scrolling region.
                {
                    place = part::mid;
                    auto [seltop, selend] = selection_take_grips();
                    if (!selection_active() || nohits(seltop, selend, upmid, dnmid))
                    {
                        uptop.role = dntop.role = grip::idle;
                        upend.role = dnend.role = grip::idle;
                        upmid = selection_coor_to_grip(coor, grip::base);
                        dnmid = upmid;
                    }
                }
                else // Inside the bottom margin.
                {
                    place = part::end;
                    coor -= {-owner.origin.x, scrolling_margin + arena };
                    if (!selection_active() || nohits(upend.coor, dnend.coor, upend, dnend))
                    {
                        upmid.role = dnmid.role = grip::idle;
                        uptop.role = dntop.role = grip::idle;
                        upend.role = grip::base;
                        upend.coor = coor;
                        dnend = upend;
                    }
                }
                selection_selbox(mode);
                selection_update();
            }
            // scroll_buf: Extend text selection.
            bool selection_extend(twod coord, bool mode) override
            {
                auto x2 = coord.x;
                auto state = selection_active();
                if (state)
                {
                    selection_selbox(mode);
                    auto scrolling_margin = batch.slide + y_top;
                    auto edge0 = dot_00;
                    auto edge1 = twod{ dot_mx.x, y_top - 1 };
                    auto edge2 = twod{-dot_mx.x, scrolling_margin };
                    auto edge3 = twod{ dot_mx.x, scrolling_margin + arena - 1 };
                    auto edge4 = twod{-dot_mx.x, 0 };
                    auto set_grip_coor_and_role = [](twod c, grip::type r)
                    {
                        return grip{ .coor = c, .role = r };
                    };
                    if (coord.y < scrolling_margin) // Hit the top margin.
                    {
                        coord -= {-owner.origin.x, batch.slide };
                        if (place == part::mid)
                        {
                            if (uptop.role == grip::base)
                            {
                                upmid.role = dnmid.role = grip::idle;
                                upend.role = dnend.role = grip::idle;
                            }
                            else if (upmid.role == grip::base || upend.role != grip::idle)
                            {
                                uptop = set_grip_coor_and_role(edge1, grip::join);
                                dnmid = selection_coor_to_grip(edge2, grip::join);
                            }
                        }
                        else if (place == part::end)
                        {
                            if (uptop.role == grip::base)
                            {
                                upmid.role = dnmid.role = grip::idle;
                                upend.role = dnend.role = grip::idle;
                            }
                            else if (upmid.role == grip::base)
                            {
                                uptop = set_grip_coor_and_role(edge1, grip::join);
                                dnmid = selection_coor_to_grip(edge2, grip::join);
                                upend.role = dnend.role             = grip::idle;
                            }
                            else if (upend.role == grip::base)
                            {
                                uptop = set_grip_coor_and_role(edge1, grip::join);
                                dnmid = selection_coor_to_grip(edge2, grip::join);
                                upmid = selection_coor_to_grip(edge3, grip::join);
                                dnend = set_grip_coor_and_role(edge4, grip::join);
                            }
                        }
                        else if (place == part::top && upmid.role != grip::idle)
                        {
                            dnmid = selection_coor_to_grip(edge2, grip::join);
                        }
                        dntop.coor = coord; dntop.role = grip::base;
                        place = part::top;
                    }
                    else if (coord.y < scrolling_margin + arena) // Hit the scrolling region.
                    {
                        if (place == part::mid)
                        {
                            dnmid = selection_coor_to_grip(coord, grip::base);
                        }
                        else if (place == part::top)
                        {
                            if (uptop.role == grip::base)
                            {
                                dntop = set_grip_coor_and_role(edge1, grip::join);
                                upmid = selection_coor_to_grip(edge2, grip::join);
                                dnmid = selection_coor_to_grip(coord, grip::base);
                                upend.role = dnend.role             = grip::idle;
                            }
                            else if (uptop.role == grip::join)
                            {
                                uptop.role = dntop.role             = grip::idle;
                                dnmid = selection_coor_to_grip(coord, grip::base);
                            }
                        }
                        else if (place == part::end)
                        {
                            if (upend.role == grip::base)
                            {
                                uptop.role = dntop.role             = grip::idle;
                                dnmid = selection_coor_to_grip(coord, grip::base);
                                upmid = selection_coor_to_grip(edge3, grip::join);
                                dnend = set_grip_coor_and_role(edge4, grip::join);
                            }
                            else if (upend.role == grip::join)
                            {
                                upend.role = dnend.role             = grip::idle;
                                dnmid = selection_coor_to_grip(coord, grip::base);
                            }
                        }
                        place = part::mid;
                    }
                    else // Hit the bottom margin.
                    {
                        coord -= {-owner.origin.x, scrolling_margin + arena };
                        if (place == part::mid)
                        {
                            if (upend.role == grip::base)
                            {
                                upmid.role = dnmid.role = grip::idle;
                                uptop.role = dntop.role = grip::idle;
                            }
                            else if (dnmid.role == grip::base || uptop.role != grip::idle)
                            {
                                dnmid = selection_coor_to_grip(edge3, grip::join);
                                upend = set_grip_coor_and_role(edge4, grip::join);
                            }
                        }
                        else if (place == part::top)
                        {
                            if (upend.role == grip::base)
                            {
                                upmid.role = dnmid.role = grip::idle;
                                uptop.role = dntop.role = grip::idle;
                            }
                            else if (upmid.role == grip::base)
                            {
                                uptop.role = dntop.role             = grip::idle;
                                dnmid = selection_coor_to_grip(edge3, grip::join);
                                dnend = set_grip_coor_and_role(edge4, grip::join);
                            }
                            else if (uptop.role == grip::base)
                            {
                                dntop = set_grip_coor_and_role(edge1, grip::join);
                                upmid = selection_coor_to_grip(edge2, grip::join);
                                dnmid = selection_coor_to_grip(edge3, grip::join);
                                upend = set_grip_coor_and_role(edge4, grip::join);
                            }
                        }
                        else if (place == part::end && upmid.role != grip::idle)
                        {
                            dnmid = selection_coor_to_grip(edge3, grip::join);
                        }
                        dnend.coor = coord; dnend.role = grip::base;
                        place = part::end;
                    }

                    if (panel.y != arena)
                    {
                        if (selection_selbox())
                        {
                            auto x1 = upmid.role == grip::base ? upmid.coor.x :
                                      uptop.role == grip::base ? uptop.coor.x - owner.origin.x:
                                                                 upend.coor.x - owner.origin.x;
                            if (upmid.role != grip::base) upmid.coor.x = x1;
                            if (dnmid.role != grip::base) dnmid.coor.x = x2;
                            if (uptop.role != grip::base) uptop.coor.x = x1 + owner.origin.x;
                            if (dntop.role != grip::base) dntop.coor.x = x2 + owner.origin.x;
                            if (upend.role != grip::base) upend.coor.x = x1 + owner.origin.x;
                            if (dnend.role != grip::base) dnend.coor.x = x2 + owner.origin.x;
                        }
                        else
                        {
                            if (dnmid.role == grip::join)
                            {
                                if (upend.role == grip::join)
                                {
                                    dnmid.coor.x = dot_mx.x;
                                    upend.coor.x =-dot_mx.x;
                                }
                                if (uptop.role == grip::join)
                                {
                                    dnmid.coor.x =-dot_mx.x;
                                    uptop.coor.x = dot_mx.x;
                                }
                            }
                            if (upmid.role == grip::join)
                            {
                                if (dnend.role == grip::join)
                                {
                                    upmid.coor.x = dot_mx.x;
                                    dnend.coor.x =-dot_mx.x;
                                }
                                if (dntop.role == grip::join)
                                {
                                    upmid.coor.x =-dot_mx.x;
                                    dntop.coor.x = dot_mx.x;
                                }
                            }
                        }
                    }

                    selection_update();
                }
                return state;
            }
            // scroll_buf: Set selection orientation.
            void selection_follow(twod coor, bool lock) override
            {
                selection_locked(lock);
                if (selection_active())
                {
                    auto swap = faux;
                    auto scrolling_margin = batch.slide + y_top;
                    if (uptop.role == grip::base
                     && dntop.role == grip::base)
                    {
                        auto p = coor - twod{-owner.origin.x, batch.slide };
                        swap = dntop.coor.y == uptop.coor.y ? std::abs(dntop.coor.x - p.x) > std::abs(uptop.coor.x - p.x)
                                                            : std::abs(dntop.coor.y - p.y) > std::abs(uptop.coor.y - p.y);
                    }
                    else if (upend.role == grip::base
                          && dnend.role == grip::base)
                    {
                        auto p = coor - twod{-owner.origin.x, scrolling_margin + arena };
                        swap = dnend.coor.y == upend.coor.y ? std::abs(dnend.coor.x - p.x) > std::abs(upend.coor.x - p.x)
                                                            : std::abs(dnend.coor.y - p.y) > std::abs(upend.coor.y - p.y);
                    }
                    else if (coor.y < scrolling_margin || coor.y >= scrolling_margin + arena)
                    {
                        if (((dntop.role == grip::join || upend.role == grip::join) && coor.y < scrolling_margin)
                         || ((uptop.role == grip::join || dnend.role == grip::join) && coor.y >= scrolling_margin + arena))
                        {
                            swap = true;
                        }
                        else
                        {
                            auto idtop = batch.index_by_id(upmid.link);
                            auto idend = batch.index_by_id(dnmid.link);
                            auto order = idtop != idend ? idend > idtop
                                                        : dnmid.coor.y != upmid.coor.y ? dnmid.coor.y > upmid.coor.y
                                                                                       : dnmid.coor.x > upmid.coor.x;
                            swap = coor.y < scrolling_margin == order;
                        }
                    }
                    else
                    {
                        auto check = selection_coor_to_grip(coor);
                        auto idtop = batch.index_by_id(upmid.link);
                        auto idend = batch.index_by_id(dnmid.link);
                        auto idcur = batch.index_by_id(check.link);

                        if (idtop != idend)
                        {
                            auto cy = (idend + idtop) / 2;
                            swap = idcur > cy == idtop > idend;
                        }
                        else // idend == idtop
                        {
                            if (idend == idcur)
                            {
                                if (dnmid.coor.y != upmid.coor.y)
                                {
                                    auto cy = (dnmid.coor.y + upmid.coor.y) / 2;
                                    swap = check.coor.y > cy == upmid.coor.y > dnmid.coor.y;
                                }
                                else
                                {
                                    swap = (upmid.coor.y == check.coor.y ? std::abs(dnmid.coor.x - check.coor.x) > std::abs(upmid.coor.x - check.coor.x)
                                                                         : std::abs(dnmid.coor.y - check.coor.y) > std::abs(upmid.coor.y - check.coor.y));
                                }
                            }
                            else swap = idcur > idend == upmid.coor.y > dnmid.coor.y;
                        }
                    }

                    if (swap)
                    {
                        std::swap(uptop, dntop);
                        std::swap(upmid, dnmid);
                        std::swap(upend, dnend);
                        place = dntop.role == grip::base ? part::top
                              : dnmid.role == grip::base ? part::mid
                                                         : part::end;
                    }
                    if (selection_selbox())
                    {
                        auto x = coor.x + owner.origin.x;
                        auto c = (upmid.coor.x + dnmid.coor.x) / 2;
                        if (x > c == upmid.coor.x > dnmid.coor.x)
                        {
                            std::swap(uptop.coor.x, dntop.coor.x);
                            std::swap(upmid.coor.x, dnmid.coor.x);
                            std::swap(upend.coor.x, dnend.coor.x);
                        }
                    }
                }
            }
            // scroll_buf: Select one word.
            void selection_byword(twod coor) override
            {
                auto scrolling_margin = batch.slide + y_top;
                if (coor.y < scrolling_margin) // Inside the top margin.
                {
                    place = part::top;
                    coor -= {-owner.origin.x, batch.slide };
                    upmid.role = dnmid.role = grip::idle;
                    upend.role = dnend.role = grip::idle;
                    uptop.role = grip::base;
                    uptop.coor = coor;
                    dntop = uptop;
                    uptop.coor.x = upbox.word<feed::rev>(coor);
                    dntop.coor.x = upbox.word<feed::fwd>(coor);
                }
                else if (coor.y < scrolling_margin + arena) // Inside the scrolling region.
                {
                    place = part::mid;
                    uptop.role = dntop.role = grip::idle;
                    upend.role = dnend.role = grip::idle;
                    upmid = selection_coor_to_grip(coor, grip::base);
                    dnmid = upmid;
                    auto& line = batch.item_by_id(upmid.link);
                    auto start = screen_to_offset(line, upmid.coor);
                    auto offup = line.word<feed::rev>({ start, 0 });
                    auto offdn = line.word<feed::fwd>({ start, 0 });
                    upmid.coor = offset_to_screen(line, offup);
                    dnmid.coor = offset_to_screen(line, offdn);
                }
                else // Inside the bottom margin.
                {
                    place = part::end;
                    coor -= {-owner.origin.x, scrolling_margin + arena };
                    upmid.role = dnmid.role = grip::idle;
                    uptop.role = dntop.role = grip::idle;
                    upend.role = grip::base;
                    upend.coor = coor;
                    dnend = upend;
                    upend.coor.x = dnbox.word<feed::rev>(coor);
                    dnend.coor.x = dnbox.word<feed::fwd>(coor);
                }
                selection_locked(faux);
                selection_selbox(faux);
                selection_update(faux);
            }
            // scroll_buf: Select one line.
            void selection_byline(twod coor) override
            {
                auto scrolling_margin = batch.slide + y_top;
                if (coor.y < scrolling_margin) // Inside the top margin.
                {
                    place = part::top;
                    coor -= {-owner.origin.x, batch.slide };
                    upmid.role = dnmid.role = grip::idle;
                    upend.role = dnend.role = grip::idle;
                    uptop.role = grip::base;
                    uptop.coor = coor;
                    dntop = uptop;
                    uptop.coor.x = 0;
                    dntop.coor.x = panel.x - 1;
                }
                else if (coor.y < scrolling_margin + arena) // Inside the scrolling region.
                {
                    place = part::mid;
                    uptop.role = dntop.role = grip::idle;
                    upend.role = dnend.role = grip::idle;
                    upmid = selection_coor_to_grip(coor, grip::base);
                    dnmid = upmid;
                    upmid.coor = dot_00;
                    auto& curln = batch.item_by_id(upmid.link);
                    auto limit = std::max(0, curln.length() - 1);
                    upmid.coor = offset_to_screen(curln, 0);
                    dnmid.coor = offset_to_screen(curln, limit);
                }
                else // Inside the bottom margin.
                {
                    place = part::end;
                    coor -= {-owner.origin.x, scrolling_margin + arena };
                    upmid.role = dnmid.role = grip::idle;
                    uptop.role = dntop.role = grip::idle;
                    upend.role = grip::base;
                    upend.coor = coor;
                    dnend = upend;
                    upend.coor.x = 0;
                    dnend.coor.x = panel.x - 1;
                }
                selection_locked(faux);
                selection_selbox(faux);
                selection_update(faux);
            }
            // scroll_buf: Return the indexes and a grips copy.
            auto selection_get_it() const
            {
                auto upcur = upmid;
                auto dncur = dnmid;
                auto i_top = batch.index_by_id(upcur.link);
                auto i_end = batch.index_by_id(dncur.link);
                if (i_top < 0)
                {
                    if (i_end < 0) return std::tuple{-1,-1, upcur, dncur };
                    upcur.coor = dot_00;
                }
                else if (i_end < 0)
                {
                    dncur.coor = dot_00;
                }

                i_top = std::clamp(i_top, 0, batch.size - 1);
                i_end = std::clamp(i_end, 0, batch.size - 1);
                if (i_top >  i_end
                || (i_top == i_end && (upcur.coor.y >  dncur.coor.y
                                   || (upcur.coor.y == dncur.coor.y && (upcur.coor.x > dncur.coor.x)))))
                {
                    std::swap(i_top, i_end);
                    std::swap(upcur, dncur);
                }
                return std::tuple{ i_top, i_end, upcur, dncur };
            }
            // scroll_buf: Calc selection height in dislay lines.
            template<class T>
            auto selection_height(T head, T tail, grip const& upcur, grip const& dncur) const
            {
                auto vpos = -upcur.coor.y;
                while (head != tail)
                {
                    vpos += head->height(panel.x);
                    ++head;
                }
                vpos += 1 + dncur.coor.y;
                return vpos;
            }
            // scroll_buf: Calc selection volume in cells.
            template<class T>
            auto selection_volume(T head, T tail, grip const& upcur, grip const& dncur) const
            {
                auto& top = *head;
                auto& end = *tail;
                auto summ = -std::min(top.length(), upcur.coor.y * panel.x + std::max(0, upcur.coor.x));
                auto vpos = -upcur.coor.y;
                while (head != tail)
                {
                    auto& line = *head;
                    vpos += line.height(panel.x);
                    summ += line.length();
                    ++head;
                }
                summ += std::min(end.length(), 1 + dncur.coor.y * panel.x + std::max(0, dncur.coor.x));
                vpos += 1 + dncur.coor.y;
                return std::pair{ vpos, summ };
            }
            // scroll_buf: Calc selection offset in cells.
            auto selection_offset(auto& curln, auto coor, auto close)
            {
                auto align = curln.style.jet();
                auto wraps = curln.style.wrp();
                auto width = curln.length();
                if (wraps == wrap::on)
                {
                    coor.x = std::clamp(coor.x, -close, -close + panel.x);
                    if (align != bias::left && coor.y == width / panel.x)
                    {
                        if (auto remain = width % panel.x)
                        {
                            if (align == bias::right)    coor.x = std::max(0,      coor.x - panel.x     + remain);
                            else      /* bias::center */ coor.x = std::max(-close, coor.x - panel.x / 2 + remain / 2);
                        }
                    }
                }
                else
                {
                    coor.y = 0;
                    if (align != bias::left)
                    {
                        if (align == bias::right)    coor.x -= panel.x     - width;
                        else      /* bias::center */ coor.x -= panel.x / 2 - width / 2;
                    }
                }
                return coor.x + coor.y * panel.x + close;
            }
            // scroll_buf: Make a viewport screen copy.
            void do_viewport_copy(face& dest) override
            {
                auto full = dest.full();
                auto view = dest.view().clip(full);
                dest.view(view);
                auto vpos = view.coor.y - y_top;
                if (vpos >= 0 && vpos < arena)
                {
                    auto& mapln = index[vpos];
                    auto  ancid = mapln.index;
                    auto  ancdy = mapln.start / panel.x;
                    auto  coord = twod{ 0, view.coor.y - ancdy };
                    auto  limit = view.coor.y + view.size.y;
                    auto head = batch.iter_by_id(ancid);
                    auto tail = batch.end();
                    while (head != tail && coord.y < limit)
                    {
                        auto& curln = *head++;
                        auto height = curln.height(panel.x);
                        dest.output(curln, coord);
                        coord.y += height;
                    }
                }
                upbox.move({ 0, y_top - sctop });
                dnbox.move({ 0, y_end + 1     });
                dest.fill(upbox, cell::shaders::full);
                dest.fill(dnbox, cell::shaders::full);
            }
            // scroll_buf: Materialize selection of the scrollbuffer part.
            void selection_pickup(escx& yield, si32 selmod)
            {
                //todo Clang 15 don't get it
                //auto [i_top, i_end, upcur, dncur] = selection_get_it();
                auto tempvr = selection_get_it();
                auto i_top = std::get<0>(tempvr);
                auto i_end = std::get<1>(tempvr);
                auto upcur = std::get<2>(tempvr);
                auto dncur = std::get<3>(tempvr);

                if (i_top == -1) return;

                auto data = batch.begin();
                auto head = data + i_top;
                auto tail = data + i_end;

                if (selection_selbox())
                {
                    auto dest = face{};
                    auto mark = cell{};
                    auto coor = dot_00;
                    auto area = rect{{ std::min(upcur.coor.x,  dncur.coor.x), upcur.coor.y },
                                     { std::abs(upcur.coor.x - dncur.coor.x) + 1, selection_height(head, tail, upcur, dncur) }};
                    auto view = rect{ dot_00, area.size };
                    auto full = rect{ -area.coor, { panel.x, area.coor.y + area.size.y }};
                    dest.core::size(area.size, brush.dry());
                    dest.core::view(view);
                    dest.flow::full(full);
                    do
                    {
                        auto& curln = *head;
                        dest.output(curln, coor);
                        coor.y += curln.height(panel.x);
                    }
                    while (head++ != tail);
                    selmod == mime::disabled ||
                    selmod == mime::textonly ||
                    selmod == mime::safetext ? yield.s11n<faux, faux, true>(dest, mark)
                                             : yield.s11n<true, faux, true>(dest, mark);
                }
                else
                {
                    auto field = rect{ dot_00, dot_01 };
                    auto state = cell{};
                    auto style = deco{};
                    auto build = [&](auto print)
                    {
                        if (i_top == i_end)
                        {
                            auto& headln = *head++;
                            field.coor.x = selection_offset(headln, upcur.coor, 0);
                            field.size.x = selection_offset(headln, dncur.coor, 1);
                            field.size.x = field.size.x - field.coor.x;
                            print(headln);
                        }
                        else
                        {
                            auto& headln = *head++;
                            field.coor.x = selection_offset(headln, upcur.coor, 0);
                            field.size.x = dot_mx.x;
                            print(headln);
                            field.coor.x = 0;
                            while (head != tail) print(*head++);
                            auto& lastln = *head++;
                            field.size.x = selection_offset(lastln, dncur.coor, 1);
                            print(lastln);
                        }
                        if (yield.length()) yield.pop_back(); // Pop last eol.
                    };
                    if (selmod == mime::textonly
                     || selmod == mime::safetext
                     || selmod == mime::disabled)
                    {
                        build([&](auto& curln)
                        {
                            auto block = escx{};
                            block.s11n<faux, faux, faux>(curln, field, state);
                            if (block.size() > 0) yield.add(block);
                            else                  yield.eol();
                        });
                    }
                    else
                    {
                        build([&](auto& curln)
                        {
                            if (style != curln.style)
                            {
                                if (auto wrp = curln.style.wrp(); style.wrp() != wrp) yield.wrp(wrp);
                                if (auto jet = curln.style.jet(); style.jet() != jet) yield.jet(jet);
                                style = curln.style;
                            }
                            auto block = escx{};
                            block.s11n<true, faux, faux>(curln, field, state);
                            if (block.size() > 0) yield.add(block);
                            else                  yield.eol();
                        });
                        yield.nil();
                    }
                }
            }
            // scroll_buf: Materialize selection.
            text selection_pickup(si32 selmod) override
            {
                auto yield = escx{};
                auto len = testy<si64>{};
                auto selbox = selection_selbox();
                if (!selection_active()) return std::move(yield);
                if (selmod != mime::textonly
                 && selmod != mime::safetext) yield.nil();
                len = yield.size();
                if (uptop.role != grip::idle)
                {
                    bufferbase::selection_pickup(yield, upbox, uptop.coor, dntop.coor, selmod, selbox);
                }
                if (upmid.role != grip::idle)
                {
                    if (len(yield.size())) yield.eol();
                    scroll_buf::selection_pickup(yield, selmod);
                }
                if (upend.role != grip::idle)
                {
                    if (len(yield.size())) yield.eol();
                    bufferbase::selection_pickup(yield, dnbox, upend.coor, dnend.coor, selmod, selbox);
                }
                if (selbox && len(yield.size())) yield.eol();
                return std::move(yield);
            }
            // scroll_buf: Highlight selection.
            void selection_render(face& dest) override
            {
                if (selection_active())
                {
                    auto mode = owner.selmod;
                    auto view = dest.view();
                    if (panel.y != arena)
                    {
                        auto draw_area = [&](auto grip_1, auto grip_2, auto offset)
                        {
                            if (grip_1.role != grip::idle)
                            {
                                grip_1.coor.y += offset;
                                grip_2.coor.y += offset;
                                grip::sort(grip_1, grip_2);
                                bufferbase::selection_raster(dest, grip_1.coor, grip_2.coor, grip_1.role == grip::base, grip_2.role == grip::base);
                            }
                        };
                        draw_area(uptop, dntop, batch.slide);
                        draw_area(upend, dnend, batch.slide + y_top + arena);
                    }
                    if (upmid.role == grip::idle) return;
                    auto scrolling_region = rect{{ -dot_mx.x / 2, batch.slide + y_top }, { dot_mx.x, arena }};
                    view = view.clip(scrolling_region);
                    //todo Clang 15 don't get it
                    //auto [curtop, curend] = selection_take_grips();
                    auto tempvr = selection_take_grips();
                    auto curtop = tempvr.first;
                    auto curend = tempvr.second;
                    auto grip_1 = rect{ curtop, dot_11 };
                    auto grip_2 = rect{ curend, dot_11 };
                    if (selection_selbox())
                    {
                        auto square = grip_1 | grip_2;
                        auto filler = [&](auto fill)
                        {
                            dest.fill(square.clip(view), fill);
                        };
                        _shade_selection(mode, filler);
                    }
                    else
                    {
                        if (curtop.y >  curend.y
                        || (curtop.y == curend.y && curtop.x > curend.x))
                        {
                            std::swap(curtop, curend);
                        }
                        dest.vsize(batch.vsize + sctop + scend); // Include margins and bottom oversize.
                        auto coor = twod{ 0, batch.slide - batch.ancdy + y_top };
                        auto stop = batch.slide + arena + y_top;
                        auto head = batch.iter_by_id(batch.ancid);
                        auto tail = batch.end();
                        auto work = [&](auto fill)
                        {
                            auto draw = [&](auto const& coord, auto const& subblock, auto isr_to_l)
                            {
                                     if (coord.y < curtop.y) return;
                                else if (coord.y > curend.y) coor.y = stop;
                                else
                                {
                                    auto block = rect{ coord, { subblock.length(), 1 }};
                                    if (coord.y == curtop.y)
                                    {
                                        auto width = curtop.y == curend.y ? curend.x - curtop.x + 1
                                                                          : dot_mx.x;
                                        auto bound = rect{ curtop, { width, 1 }}.normalize();
                                        block = block.clip(bound);
                                    }
                                    else if (coord.y == curend.y)
                                    {
                                        auto bound = rect{ curend, { -dot_mx.x, 1 }}.normalize();
                                        bound.size.x += 1;
                                        block = block.clip(bound);
                                    }
                                    dest.fill(block.clip(view), fill);
                                }
                            };
                            while (head != tail && coor.y < stop)
                            {
                                auto& curln = *head;
                                auto length = curln.length();
                                auto height = curln.height(panel.x);
                                if (length)
                                {
                                    dest.output_proxy(curln, coor, draw);
                                }
                                else
                                {
                                    auto align = curln.style.jet();
                                    auto coord = coor;
                                    switch (align)
                                    {
                                        case bias::none:
                                        case bias::left:   break;
                                        case bias::right:  coord.x += panel.x - 1; break;
                                        case bias::center: coord.x += panel.x / 2; break;
                                    }
                                    struct { auto length() const { return 1; }} empty;
                                    draw(coord, empty, faux);
                                }
                                coor.y += height;
                                ++head;
                            }
                        };
                        _shade_selection(mode, work);
                    }
                }
            }
            // scroll_buf: Update selection status.
            void selection_status(term_state& status) const override
            {
                status.coor.x = 1 + std::abs(dnmid.coor.x - upmid.coor.x);
                if (upmid.role != grip::idle)
                {
                    status.coor.y = 1 + std::abs(static_cast<si32>(dnmid.link - upmid.link));
                    if (status.coor.y < approx_threshold)
                    {
                        auto [i_top, i_end, upcur, dncur] = selection_get_it();
                        auto data = batch.begin();
                        auto head = data + i_top;
                        auto tail = data + i_end;
                        auto [height, volume] = selection_volume(head, tail, upcur, dncur);
                        if (selection_selbox()) status.coor.y = height;
                        else                    status.body   = volume;
                    }
                    else
                    {
                        if (!selection_selbox()) status.body = status.coor.y * panel.x;
                    }
                }
                else
                {
                    status.coor.y = 0;
                    status.body   = 0;
                }
                if (panel.y != arena)
                {
                    auto calc = [&](auto top, auto end)
                    {
                        auto dt = twod{};
                        top.x = std::clamp(top.x, 0, panel.x - 1);
                        end.x = std::clamp(end.x, 0, panel.x - 1);
                        if (top.y == end.y)
                        {
                            dt = { std::abs(end.x - top.x) + 1, 1 };
                        }
                        else
                        {
                            if (top.y > end.y) std::swap(top, end);
                            dt.y = end.y - top.y + 1;
                            dt.x += panel.x * (dt.y - 2);
                            dt.x += panel.x + end.x - top.x + 1;
                        }
                        status.coor.y += dt.y;
                        status.body   += dt.x;
                    };
                    if (uptop.role != grip::idle) calc(uptop.coor, dntop.coor);
                    if (upend.role != grip::idle) calc(upend.coor, dnend.coor);
                }
            }
            // scroll_buf: Loop through selected lines.
            template<class P>
            void selection_foreach(P proc)
            {
                auto [i_top, i_end, upcur, dncur] = selection_get_it();
                if (i_top == -1)
                {
                    selection_cancel();
                }
                else
                {
                    auto data = batch.begin();
                    auto head = data + i_top;
                    auto tail = data + i_end;
                    do proc(*head);
                    while (head++ != tail);
                }
            }
            // scroll_buf: Sel alignment for selected lines.
            void selection_setjet(bias align) override
            {
                if (upmid.role == grip::idle) return;
                selection_foreach([&](auto& curln)
                {
                    curln.style.jet(align);
                    batch.recalc(curln);
                });
                resize_viewport(panel, true); // Recalc batch.basis.
            }
            // scroll_buf: Sel wrapping mode for selected lines.
            void selection_setwrp(wrap w = wrap::none) override
            {
                if (selection_active())
                {
                    if (upmid.role == grip::idle) return;
                    auto i_top = std::clamp(batch.index_by_id(upmid.link), 0, batch.size);
                    auto wraps = w == wrap::none ? batch[i_top].style.wrp() == wrap::on ? wrap::off : wrap::on
                                                 : w;
                    upmid.coor.y = dnmid.coor.y = 0;
                    selection_foreach([&](auto& curln)
                    {
                        curln.style.wrp(wraps);
                        batch.recalc(curln);
                    });
                    if (w != wrap::none) style.wrp(w);
                    resize_viewport(panel, true); // Recalc batch.basis.
                }
                else
                {
                    style.wrp(w == wrap::none ? style.wrp() == wrap::on ? wrap::off : wrap::on
                                              : w);
                }
            }
            // scroll_buf: Update selection internals.
            void selection_update(bool despace = true) override
            {
                if (upmid.role == grip::base
                 && dnmid.role == grip::base
                 && upmid.link == dnmid.link
                 && (!selection_selbox() || (upmid.link   == dnmid.link
                                          && upmid.coor.y == dnmid.coor.y)))
                {
                    auto& curln = batch.item_by_id(upmid.link);
                    auto p1 = upmid.coor;
                    auto p2 = dnmid.coor;
                    if (p1.y > p2.y || (p1.y == p2.y && p1.x > p2.x)) std::swap(p1, p2);
                    auto head = selection_offset(curln, p1, 0);
                    auto tail = selection_offset(curln, p2, 1);
                    match = { curln.core::line(head, tail) };
                }
                else if (uptop.role == grip::base
                      && dntop.role == grip::base
                      && (!selection_selbox() || uptop.coor.y == dntop.coor.y))
                {
                    match = { upbox.core::line(uptop.coor, dntop.coor) };
                }
                else if (upend.role == grip::base
                      && dnend.role == grip::base
                      && (!selection_selbox() || upend.coor.y == dnend.coor.y))
                {
                    match = { dnbox.core::line(upend.coor, dnend.coor) };
                }
                else match = {};

                bufferbase::selection_update(despace);
            }
            // scroll_buf: Search data and return distance to it.
            twod selection_gofind(feed direction, view data = {}) override
            {
                if (data.empty()) return dot_00;
                match = line{ data };

                auto ahead = direction == feed::fwd;
                if (ahead)
                {
                    uirev = faux;
                    uptop.coor = dntop.coor = {};
                    uptop.role = dntop.role = grip::base;
                    upmid.role = dnmid.role = grip::idle;
                    upend.role = dnend.role = grip::idle;
                }
                else
                {
                    uifwd = faux;
                    uptop.role = dntop.role = grip::idle;
                    upmid.role = dnmid.role = grip::idle;
                    upend.role = dnend.role = grip::base;
                    upend.coor = dnend.coor = dnbox.size() - dot_01;
                }
                auto delta = selection_gonext(direction);

                if ((ahead && uirev == faux)
                 ||(!ahead && uifwd == faux))
                {
                    selection_cancel();
                    delta = {};
                }
                return delta;
            }
            // scroll_buf: Retrun distance between lines.
            auto selection_outrun(id_t id1, twod coor1, id_t id2, twod coor2)
            {
                auto dir = static_cast<si32>(id2 - id1);
                if (dir < 0)
                {
                    std::swap(id1, id2);
                    std::swap(coor1, coor2);
                }
                auto dist = coor2;
                auto head = batch.iter_by_id(id1);
                auto tail = batch.iter_by_id(id2);
                while (head != tail)
                {
                    dist.y += head->height(panel.x);
                    ++head;
                }
                dist -= coor1;
                return dir < 0 ? -dist
                               :  dist;
            }
            // scroll_buf: Retrun viewport center.
            auto selection_viewport_center()
            {
                return twod{ panel.x / 2 - owner.origin.x, arena / 2 + batch.ancdy };
            }
            // scroll_buf: Retrun distance to the center of viewport.
            twod selection_center(id_t id, twod coor)
            {
                auto base = selection_viewport_center();
                auto dist = selection_outrun(id, coor, batch.ancid, base);
                return dist;
            }
            // scroll_buf: Search prev/next selection match and return distance to it.
            twod selection_gonext(feed direction) override
            {
                if (match.empty()) return dot_00;

                auto delta = dot_00;
                auto ahead = direction == feed::fwd;
                auto probe = [&](auto startid, auto coord)
                {
                    auto& curln = batch.item_by_id(startid);
                    auto from = selection_offset(curln, coord, 0);
                    auto mlen = match.length();
                    auto step = ahead ? mlen : 0;
                    auto back = ahead ? 2 : mlen;
                    auto resx = [&](auto& curln)
                    {
                        if (curln.find(match, from, direction))
                        {
                            upmid.link = curln.index;
                            dnmid.link = curln.index;
                            upmid.coor = offset_to_screen(curln, from);
                            from += step - back + 1;
                            dnmid.coor = offset_to_screen(curln, from);
                            delta += coord - upmid.coor;
                            uptop.role = dntop.role = grip::idle;
                            upmid.role = dnmid.role = grip::base;
                            upend.role = dnend.role = grip::idle;
                            return true;
                        }
                        else return faux;
                    };
                    from += step;
                    auto done = resx(curln);
                    if (!done)
                    {
                        auto head = batch.iter_by_id(startid);
                        auto find = [&](auto tail, auto proc)
                        {
                            auto accum = ahead ? curln.height(panel.x)
                                               : si32{0};
                            while (head != tail)
                            {
                                auto& curln = proc(head);
                                from = ahead ? 0 : curln.length();
                                if (resx(curln))
                                {
                                    delta.y += ahead ?-accum
                                                     : accum + curln.height(panel.x);
                                    return true;
                                }
                                accum += curln.height(panel.x);
                            }
                            return faux;
                        };
                        if (ahead)
                        {
                            uirev = faux;
                            done = find(batch.end() - 1, [](auto& head) -> auto& { return *++head; });
                            if (done)
                            {
                                uirev = true;
                            }
                            else if (sctop)
                            {
                                auto from = si32{ 0 };
                                done = bufferbase::selection_search(dnbox, from, direction, upend.coor, dnend.coor);
                                if (done)
                                {
                                    uptop.role = dntop.role = grip::idle;
                                    upmid.role = dnmid.role = grip::idle;
                                    upend.role = dnend.role = grip::base;
                                }
                            }
                        }
                        else
                        {
                            uifwd = faux;
                            done = find(batch.begin(),   [](auto& head) -> auto& { return *--head; });
                            if (done)
                            {
                                uifwd = true;
                            }
                            else if (scend)
                            {
                                auto from = upbox.size().x * upbox.size().y;
                                done = bufferbase::selection_search(upbox, from, direction, uptop.coor, dntop.coor);
                                if (done)
                                {
                                    uptop.role = dntop.role = grip::base;
                                    upmid.role = dnmid.role = grip::idle;
                                    upend.role = dnend.role = grip::idle;
                                }
                            }
                        }
                    }
                    return done;
                };

                if (upmid.role == grip::base)
                {
                    auto init = upmid;
                    auto stop = dnmid;
                    if (init.coor.y >  stop.coor.y
                    || (init.coor.y == stop.coor.y && init.coor.x > stop.coor.x)) std::swap(init, stop);

                    auto center = selection_center(init.link, init.coor);
                    delta += center; // Always centered.
                    probe(upmid.link, init.coor);
                }
                else
                {
                    if (uptop.role == grip::base)
                    {
                        auto p1 = uptop.coor;
                        auto p2 = dntop.coor;
                        if (p1.y > p2.y || (p1.y == p2.y && p1.x > p2.x)) std::swap(p1, p2);

                        auto from = p1.x + p1.y * upbox.size().x;
                        auto done = bufferbase::selection_search(upbox, from, direction, uptop.coor, dntop.coor);
                        if (!done && ahead)
                        {
                            // Get first visible line.
                            auto fromxy = twod{ 0, batch.ancdy };
                            if (probe(batch.ancid, fromxy))
                            {
                                auto center = fromxy - selection_viewport_center();
                                delta -= center;
                            }
                        }
                        else delta = dot_00;
                    }
                    else if (upend.role == grip::base)
                    {
                        auto p1 = upend.coor;
                        auto p2 = dnend.coor;
                        if (p1.y > p2.y || (p1.y == p2.y && p1.x > p2.x)) std::swap(p1, p2);

                        auto from = p1.x + p1.y * dnbox.size().x;
                        auto done = bufferbase::selection_search(dnbox, from, direction, upend.coor, dnend.coor);
                        if (!done && !ahead)
                        {
                            // Get last visible line.
                            auto vpos =-batch.ancdy;
                            auto head = batch.iter_by_id(batch.ancid);
                            auto tail = batch.end();
                            while (head != tail)
                            {
                                auto& curln = *head++;
                                auto newpos = vpos + curln.height(panel.x);
                                if (newpos >= arena) break;
                                vpos = newpos;
                            }
                            auto& curln = *--head;
                            auto coorxy = twod{ panel.x - owner.origin.x, arena - vpos };
                            auto offset = screen_to_offset(curln, coorxy);
                            auto fromxy = offset_to_screen(curln, offset);
                            if (probe(curln.index, fromxy))
                            {
                                auto center = selection_viewport_center();
                                delta.x -= panel.x / 2 - coorxy.x - fromxy.x;
                                delta.y -= vpos - center.y;
                            }
                        }
                        else delta = dot_00;
                    }
                }

                if (upmid.role == grip::base)
                {
                    auto new_origin_x = owner.origin.x + delta.x;
                    delta.x -= new_origin_x - reset_viewport(new_origin_x, upmid.coor.x, panel.x);
                }
                else delta = dot_00;

                bufferbase::selection_update(faux);
                return delta;
            }
            // scroll_buf: Return match navigation state.
            si32 selection_button(twod delta = {}) override
            {
                auto forward_is_available = si32{};
                auto reverse_is_available = si32{};
                if (match.empty())
                {
                    forward_is_available = batch.slide - delta.y >= batch.vsize - arena ? 0 : 1 << 0;
                    reverse_is_available = batch.slide - delta.y <= 0                   ? 0 : 1 << 1;
                }
                else
                {
                    forward_is_available = uifwd ? 1 << 0 : 0;
                    reverse_is_available = uirev ? 1 << 1 : 0;
                }
                return forward_is_available | reverse_is_available;
            }
        };

        using prot = input::keybd::prot;
        using buffer_ptr = bufferbase*;
        using vtty = os::vt::vtty;

        termconfig config; // term: Terminal settings.
        pro::timer worker; // term: Linear animation controller.
        pro::robot dynamo; // term: Linear animation controller.
        pro::caret cursor; // term: Text cursor controller.
        term_state status; // term: Screen buffer status info.
        w_tracking wtrack; // term: Terminal title tracking object.
        f_tracking ftrack; // term: Keyboard focus tracking object.
        m_tracking mtrack; // term: VT-style mouse tracking object.
        c_tracking ctrack; // term: Custom terminal palette tracking object.
        scroll_buf normal; // term: Normal    screen buffer.
        alt_screen altbuf; // term: Alternate screen buffer.
        buffer_ptr target; // term: Current   screen buffer pointer.
        escx       w32key; // term: win32-input-mode forward buffer.
        text       cmdarg; // term: Startup command line arguments.
        text       curdir; // term: Current working directory.
        hook       onerun; // term: One-shot token for restart session.
        twod       origin; // term: Viewport position.
        twod       follow; // term: Viewport follows cursor (bool: X, Y).
        bool       decckm; // term: Cursor keys Application(true)/ANSI(faux) mode.
        bool       bpmode; // term: Bracketed paste mode.
        bool       unsync; // term: Viewport is out of sync.
        bool       invert; // term: Inverted rendering (DECSCNM).
        bool       selalt; // term: Selection form (rectangular/linear).
        bool       io_log; // term: Stdio logging.
        bool       styled; // term: Line style reporting.
        flag       resume; // term: Restart scheduled.
        flag       forced; // term: Forced shutdown.
        prot       kbmode; // term: Keyboard input mode.
        si32       selmod; // term: Selection mode.
        si32       altscr; // term: Alternate scroll mode.
        vtty       ipccon; // term: IPC connector. Should be destroyed first.

        // term: Forward clipboard data (OSC 52).
        void forward_clipboard(view data)
        {
            auto clipdata = input::clipdata{};
            auto delimpos = data.find(';');
            if (delimpos != text::npos)
            {
                clipdata.utf8 = data.substr(0, ++delimpos);
                utf::unbase64(data.substr(delimpos), clipdata.utf8);
                clipdata.form = mime::disabled;
                clipdata.size = target->panel;
                clipdata.hash = datetime::now();
                RISEUP(tier::request, e2::form::state::keybd::enlist, gates, ()); // Take all foci.
                for (auto gate_id : gates) // Signal them to set the clipboard data.
                {
                    if (auto gear_ptr = bell::getref<hids>(gate_id))
                    {
                        gear_ptr->set_clipboard(clipdata);
                    }
                }
            }
        }
        // term: Soft terminal reset (DECSTR).
        void decstr()
        {
            target->flush();
            normal.clear_all();
            altbuf.clear_all();
            target = &normal;
            invert = faux;
            decckm = faux;
            bpmode = faux;
            altscr = config.def_alt_on ? config.def_altscr : 0;
            normal.brush.reset();
            ipccon.reset();
        }
        // term: Set termnail parameters. (DECSET).
        void _decset(si32 n)
        {
            switch (n)
            {
                case 1:    // Cursor keys application mode.
                    decckm = true;
                    break;
                case 3:    // Set 132 column window size (DECCOLM).
                    window_resize({ 132, 0 });
                    target->ed(commands::erase::display::viewport);
                    break;
                case 5:    // Inverted rendering (DECSCNM).
                    invert = true;
                    break;
                case 6:    // Enable origin mode (DECOM).
                    target->decom = true;
                    target->cup0(dot_00);
                    break;
                case 7:    // Enable auto-wrap.
                    target->style.wrp(wrap::on);
                    break;
                case 12:   // Enable cursor blinking.
                    cursor.blink_period();
                    break;
                case 25:   // Caret on.
                    cursor.show();
                    break;
                case 9:    // Enable X10 mouse reporting protocol.
                    log(prompt::term, "CSI ? 9 h  X10 Mouse reporting protocol is not supported");
                    break;
                case 1000: // Enable mouse buttons reporting mode.
                    mtrack.enable(input::mouse::mode::buttons_press);
                    break;
                case 1001: // Use Hilite mouse tracking mode.
                    log(prompt::term, "CSI ? 1001 h  Hilite mouse tracking mode is not supported");
                    break;
                case 1002: // Enable mouse buttons and drags reporting mode.
                    mtrack.enable(input::mouse::mode::buttons_drags);
                    break;
                case 1003: // Enable all mouse events reporting mode.
                    mtrack.enable(input::mouse::mode::all_movements);
                    break;
                case 1004: // Enable focus tracking.
                    ftrack.set(true);
                    break;
                case 1005: // Enable UTF8 mousee position encoding.
                    mtrack.enable(input::mouse::mode::utf8);
                    break;
                case 1006: // Enable SGR mouse reporting protocol.
                    mtrack.setmode(input::mouse::prot::sgr);
                    break;
                case 1007: // Enable alternate scroll mode.
                    altscr = config.def_altscr;
                    break;
                case 10060:// Enable mouse reporting outside the viewport (outside+negative coordinates).
                    mtrack.enable(input::mouse::mode::negative_args);
                    break;
                case 1015: // Enable URXVT mouse reporting protocol.
                    log(prompt::term, "CSI ? 1015 h  URXVT mouse reporting protocol is not supported");
                    break;
                case 1016: // Enable Pixels (subcell) mouse mode.
                    log(prompt::term, "CSI ? 1016 h  Pixels (subcell) mouse mode is not supported");
                    break;
                case 1048: // Save cursor pos.
                    target->scp();
                    break;
                case 1047: // Use alternate screen buffer.
                case 1049: // Save cursor pos and use alternate screen buffer, clearing it first.  This control combines the effects of the 1047 and 1048  modes.
                    if (target != &normal && target != &altbuf) break; // Suppress mode change for additional screen buffers (windows console).
                    altbuf.style = target->style;
                    altbuf.brush = target->brush;
                    altbuf.clear_all();
                    altbuf.resize_viewport(target->panel); // Reset viewport to the basis.
                    target = &altbuf;
                    follow[axis::Y] = true;
                    break;
                case 2004: // Set bracketed paste mode.
                    bpmode = true;
                    break;
                default:
                    break;
            }
        }
        // term: Set termnail parameters. (DECSET).
        void decset(si32 n)
        {
            target->flush();
            _decset(n);
        }
        // term: Set termnail parameters. (DECSET).
        void decset(fifo& queue)
        {
            target->flush();
            while (auto q = queue(0)) _decset(q);
        }
        // term: Switch buffer to normal and reset viewport to the basis.
        template<class T>
        void reset_to_normal(T& a)
        {
            normal.resize_viewport(a.panel);
            target = &normal;
            follow[axis::Y] = true;
        }
        // term: Switch buffer to altbuf.
        template<class T>
        void reset_to_altbuf(T& altbuf)
        {
            altbuf.resize_viewport(target->panel);
            target = &altbuf;
        }
        // term: Reset termnail parameters. (DECRST).
        void _decrst(si32 n)
        {
            switch (n)
            {
                case 1:    // Cursor keys ANSI mode.
                    decckm = faux;
                    break;
                case 3:    // Set 80 column window size (DECCOLM).
                    window_resize({ 80, 0 });
                    target->ed(commands::erase::display::viewport);
                    break;
                case 5:    // Inverted rendering (DECSCNM).
                    invert = faux;
                    break;
                case 6:    // Disable origin mode (DECOM).
                    target->decom = faux;
                    target->cup0(dot_00);
                    break;
                case 7:    // Disable auto-wrap.
                    target->style.wrp(wrap::off);
                    break;
                case 12:   // Disable cursor blinking.
                    cursor.blink_period(span::zero());
                    break;
                case 25:   // Caret off.
                    cursor.hide();
                    break;
                case 9:    // Disable X10 mouse reporting protocol.
                    log(prompt::term, "CSI ? 9 l  X10 Mouse tracking protocol is not supported");
                    break;
                case 1000: // Disable mouse buttons reporting mode.
                    mtrack.disable(input::mouse::mode::buttons_press);
                    break;
                case 1001: // Don't use Hilite(c) mouse tracking mode.
                    log(prompt::term, "CSI ? 1001 l  Hilite mouse tracking mode is not supported");
                    break;
                case 1002: // Disable mouse buttons and drags reporting mode.
                    mtrack.disable(input::mouse::mode::buttons_drags);
                    break;
                case 1003: // Disable all mouse events reporting mode.
                    mtrack.disable(input::mouse::mode::all_movements);
                    break;
                case 1004: // Disable focus tracking.
                    ftrack.set(faux);
                    break;
                case 1005: // Disable UTF-8 mouse reporting protocol.
                    mtrack.disable(input::mouse::mode::utf8);
                    break;
                case 1006: // Disable SGR mouse reporting protocol (set X11 mode).
                    mtrack.setmode(input::mouse::prot::x11);
                    mtrack.disable(input::mouse::mode::all_movements);
                    break;
                case 1007: // Disable alternate scroll mode.
                    altscr = 0;
                    break;
                case 10060:// Disable mouse reporting outside the viewport (allow reporting inside the viewport only).
                    mtrack.disable(input::mouse::mode::negative_args);
                    break;
                case 1015: // Disable URXVT mouse reporting protocol.
                    log(prompt::term, "CSI ? 1015 l  URXVT mouse reporting protocol is not supported");
                    break;
                case 1016: // Disable Pixels (subcell) mouse mode.
                    log(prompt::term, "CSI ? 1016 l  Pixels (subcell) mouse mode is not supported");
                    break;
                case 1048: // Restore cursor pos.
                    target->rcp();
                    break;
                case 1047: // Use normal screen buffer.
                case 1049: // Use normal screen buffer and restore cursor.
                    if (target != &normal && target != &altbuf) break; // Suppress mode change for additional screen buffers (windows console).
                    normal.style = target->style;
                    normal.brush = target->brush;
                    reset_to_normal(*target);
                    break;
                case 2004: // Disable bracketed paste mode.
                    bpmode = faux;
                    break;
                default:
                    break;
            }
        }
        // term: Reset termnail parameters. (DECRST).
        void decrst(si32 n)
        {
            target->flush();
            _decrst(n);
        }
        // term: Reset termnail parameters. (DECRST).
        void decrst(fifo& queue)
        {
            target->flush();
            while (auto q = queue(0)) _decrst(q);
        }
        // term: Set scrollback buffer size and grow step.
        void sbsize(fifo& queue)
        {
            target->flush();
            auto ring_size = queue(config.def_length);
            auto grow_step = queue(config.def_growdt);
            auto grow_mxsz = queue(config.def_growmx);
            normal.resize_history(ring_size, grow_step, grow_mxsz);
        }
        // term: Check and update scrollback buffer limits.
        void sb_min(si32 min_length)
        {
            target->flush();
            if (normal.batch.step == 0 && normal.batch.peak < min_length)
            {
                normal.resize_history(min_length);
            }
        }
        // term: Write input data.
        void write(view data)
        {
            ipccon.write(data);
        }
        // term: Write tty data and flush the queue.
        void answer(escx& queue)
        {
            if (queue.length())
            {
                write(queue);
                queue.clear();
            }
        }
        // term: Reset viewport position.
        void scroll(twod& origin)
        {
            auto& console = *target;
            if (follow[axis::Y])
            {
                if (follow[axis::X])
                {
                    follow[axis::X] = faux;
                    auto pos = console.get_coord(dot_00).x;
                    origin.x = bufferbase::reset_viewport(origin.x, pos, console.panel.x);
                }
            }
            origin.y = console.get_origin(follow[axis::Y]);
        }
        // term: Proceed terminal changes.
        template<class P>
        void update(P proc)
        {
            auto done = bell::trysync(true, [&]
            {
                if (config.resetonout) follow[axis::Y] = true;
                if (follow[axis::Y])
                {
                    proc();
                }
                else
                {
                    auto last_basis = target->get_basis();
                    auto last_slide = target->get_slide();
                    proc();
                    auto next_basis = target->get_basis();
                    follow[axis::Y] = (last_basis <= last_slide && last_slide <= next_basis)
                                   || (next_basis <= last_slide && last_slide <= last_basis);
                }
                unsync = true;
            });
        }
        // term: Proceed terminal input.
        void ondata(view data, bufferbase* target = {})
        {
            update([&]
            {
                if (io_log && data.size()) log(prompt::cout, "\n\t", utf::change(ansi::hi(utf::debase(data)), "\n", ansi::pushsgr().nil().add("\n\t").popsgr()));
                ansi::parse(data, target ? target : this->target);
            });
        }
        // term: Reset to defaults.
        void setdef()
        {
            auto& console = *target;
            console.style.reset();
            console.style.wrp(config.def_wrpmod);
            console.setpad(config.def_margin);
            selection_selmod(config.def_selmod);
            auto brush = base::color();
            brush = cell{ '\0' }.fgc(config.def_fcolor).bgc(config.def_bcolor).link(brush.link());
            base::color(brush);
            cursor.style(config.def_cursor);
        }
        // term: Set terminal background.
        void setsgr(fifo& queue)
        {
            struct marker
            {
                ansi::deco style;
                ansi::mark brush;
                void task(ansi::rule const& cmd) { }
            };
            static auto parser = ansi::csi_t<marker, true>{};

            auto mark = marker{};
            mark.brush = base::color();
            auto param = queue.front(ansi::sgr_rst);
            if (queue.issub(param))
            {
                auto ptr = &mark;
                queue.settop(queue.desub(param));
                parser.table[ansi::csi_sgr].execute(queue, ptr);
            }
            else mark.brush = cell{ '\0' }.fgc(config.def_fcolor).bgc(config.def_bcolor);
            set_color(mark.brush);
        }
        // term: CCC_LSR: Enable line style reporting.
        void setlsr(bool state)
        {
            styled = state;
            if (styled) ipccon.style(target->parser::style, kbmode);
        }
        // term: Request to scroll inside viewport and return actual delta.
        auto scrollby(twod delta)
        {
            auto coor = base::coor();
            auto info = e2::form::upon::scroll::bystep::v.param({ .vector = delta });
            RISEUP(tier::preview, e2::form::upon::scroll::bystep::v, info);
            return base::coor() - coor;
        }
        // term: Is the selection allowed.
        auto selection_passed()
        {
            return selmod != mime::disabled;
        }
        // term: Set selection mode.
        void selection_selmod(si32 newmod)
        {
            selmod = newmod;
            SIGNAL(tier::release, e2::form::draggable::left, selection_passed());
            SIGNAL(tier::release, ui::term::events::selmod, selmod);
            if (mtrack && selmod == mime::disabled)
            {
                follow[axis::Y] = true; // Reset viewport.
                ondata("");             // Recalc trigger.
            }
        }
        // term: Set selection form.
        void selection_selalt(bool boxed)
        {
            selalt = boxed;
            SIGNAL(tier::release, e2::form::draggable::left, selection_passed());
            SIGNAL(tier::release, ui::term::events::selalt, selalt);
            if (mtrack && selmod == mime::disabled)
            {
                follow[axis::Y] = true; // Reset viewport.
                ondata("");             // Recalc trigger.
            }
        }
        // term: Set the next selection mode.
        void selection_selmod()
        {
            auto newmod = (selmod + 1) % mime::count;
            selection_selmod(newmod);
        }
        auto selection_cancel()
        {
            auto active = target->selection_cancel();
            if (active)
            {
                auto& console = *target;
                worker.pacify();
                auto shore = console.getpad();
                auto delta = dot_00;
                     if (origin.x <= oversz.l && origin.x > oversz.l - shore) delta = {-1, oversz.l - shore };
                else if (origin.x >=-oversz.r && origin.x < shore - oversz.r) delta = { 1, shore - oversz.r };
                if (delta.x)
                {
                    auto limit = delta.y;
                    delta.y = 0;
                    worker.actify(commands::ui::center, 0ms, [&, delta, shore, limit](auto id) mutable // 0ms = current FPS ticks/sec.
                    {
                        auto shift = scrollby(delta);
                        return shore-- && (origin.x != limit && !!shift);
                    });
                }
                base::deface();
            }
            return active;
        }
        auto get_clipboard_text(hids& gear)
        {
            gear.owner.RISEUP(tier::request, hids::events::clipbrd, gear);
            auto& data = gear.board::cargo;
            if (data.utf8.size())
            {
                if (data.form == mime::richtext)
                {
                    auto post = page{ data.utf8 };
                    return post.to_rich();
                }
                else if (data.form == mime::htmltext)
                {
                    auto post = page{ data.utf8 };
                    auto [html, code] = post.to_html();
                    return code;
                }
            }
            return data.utf8;
        }
        auto paste(hids& gear)
        {
            auto& console = *target;
            auto data = get_clipboard_text(gear);
            if (data.size())
            {
                pro::focus::set(this->This(), gear.id, pro::focus::solo::off, pro::focus::flip::off);
                follow[axis::X] = true;
                if (bpmode)
                {
                    data = "\033[200~" + data + "\033[201~";
                }
                //todo paste is a special type operation like a mouse reporting.
                //todo pasting must be ready to be interruped by any pressed key (to interrupt a huge paste).
                data_out(data);
                return true;
            }
            return faux;
        }
        auto _copy(hids& gear, text const& data)
        {
            auto form = selmod == mime::disabled ? mime::textonly
                                                 : selmod;
            pro::focus::set(this->This(), gear.id, pro::focus::solo::off, pro::focus::flip::off);
            gear.set_clipboard(target->panel, data, form);
        }
        auto copy(hids& gear)
        {
            auto data = target->selection_pickup(selmod);
            if (data.size())
            {
                _copy(gear, data);
            }
            if (gear.meta(hids::anyCtrl) || selection_cancel()) // Keep selection if Ctrl is pressed.
            {
                base::expire<tier::release>();
                return true;
            }
            return faux;
        }
        auto prnscrn(hids& gear)
        {
            auto selbox = true;
            auto square = target->panel;
            auto seltop = dot_00;
            auto selend = square - dot_11;
            auto buffer = escx{};
            auto canvas = e2::render::any.param();
            canvas.size(square);
            canvas.full({ origin, square });
            SIGNAL(tier::release, e2::render::any, canvas);
            target->bufferbase::selection_pickup(buffer, canvas, seltop, selend, selmod, selbox);
            if (buffer.size()) buffer.eol();
            _copy(gear, buffer);
        }
        auto selection_active()
        {
            return target->selection_active();
        }
        void selection_pickup(hids& gear)
        {
            RISEUP(tier::request, e2::form::state::keybd::find, gear_test, (gear.id, 0));
            if (!gear_test.second) // Set exclusive focus on right click.
            {
                pro::focus::set(This(), gear.id, pro::focus::solo::on, pro::focus::flip::off);
            }
            if ((selection_active() && copy(gear))
             || (selection_passed() && paste(gear)))
            {
                gear.dismiss();
            }
        }
        void selection_mclick(hids& gear)
        {
            auto& console = *target;
            auto utf8 = text{};
            if (console.selection_active()) // Paste from selection.
            {
                utf8 = console.match.utf8();
            }
            else if (selection_passed()) // Paste from clipboard.
            {
                gear.owner.RISEUP(tier::request, hids::events::clipbrd, gear);
                utf8 = gear.board::cargo.utf8;
            }
            if (utf8.size())
            {
                pro::focus::set(this->This(), gear.id, pro::focus::solo::off, pro::focus::flip::off);
                follow[axis::X] = true;
                if (bpmode) data_out("\033[200~" + utf8 + "\033[201~");
                else        data_out(utf8);
                gear.dismiss();
            }
        }
        void selection_lclick(hids& gear)
        {
            auto& console = *target;
            auto go_on = gear.meta(hids::anyCtrl);
            if (go_on && console.selection_active())
            {
                console.selection_follow(gear.coord, go_on);
                selection_extend(gear);
                gear.dismiss();
                base::expire<tier::release>();
            }
            else selection_cancel();
        }
        void selection_dblclk(hids& gear)
        {
            target->selection_byword(gear.coord);
            gear.dismiss();
            base::expire<tier::release>();
            base::deface();
        }
        void selection_tplclk(hids& gear)
        {
            target->selection_byline(gear.coord);
            gear.dismiss();
            base::expire<tier::release>();
            base::deface();
        }
        void selection_create(hids& gear)
        {
            auto& console = *target;
            auto boxed = selalt ^ !!gear.meta(hids::anyAlt);
            auto go_on = gear.meta(hids::anyCtrl);
            console.selection_follow(gear.coord, go_on);
            if (go_on) console.selection_extend(gear.coord, boxed);
            else       console.selection_create(gear.coord, boxed);
            base::deface();
        }
        void selection_moveto(twod delta)
        {
            if (delta)
            {
                auto path = delta;
                auto time = skin::globals().switching;
                auto init = 0;
                auto func = constlinearAtoB<twod>(path, time, init);
                dynamo.actify(func, [&](twod& step)
                {
                    scrollby(step);
                    base::deface();
                });
            }
            else worker.pacify();

        }
        void selection_extend(hids& gear)
        {
            // Check bounds and scroll if needed.
            auto& console = *target;
            auto boxed = selalt ^ !!gear.meta(hids::anyAlt);
            auto coord = gear.coord;
            auto vport = rect{ -origin, console.panel };
            auto delta = dot_00;
            for (auto a : { axis::X, axis::Y })
            {
                     if (coord[a] <  vport.coor[a])                 delta[a] = vport.coor[a] - coord[a];
                else if (coord[a] >= vport.coor[a] + vport.size[a]) delta[a] = vport.coor[a] + vport.size[a] - coord[a] - 1;
            }
            if (delta)
            {
                auto shift = scrollby(delta);
                coord += delta - shift;
                delta -= delta * 3 / 4; // Decrease scrolling speed.
                worker.actify(0ms, [&, delta, coord, boxed](auto id) mutable // 0ms = current FPS ticks/sec.
                                    {
                                        auto shift = scrollby(delta);
                                        coord -= shift;
                                        if (console.selection_extend(coord, boxed))
                                        {
                                            base::deface();
                                            return !!shift;
                                        }
                                        else return faux;
                                    });
            }
            else worker.pacify();

            if (console.selection_extend(coord, boxed))
            {
                base::deface();
            }
        }
        void selection_finish(hids& gear)
        {
            //todo option: copy on select
            //...
            worker.pacify();
            base::deface();
        }
        void selection_submit()
        {
            SIGNAL(tier::release, e2::form::draggable::left, selection_passed());
            LISTEN(tier::release, hids::events::mouse::scroll::any, gear)
            {
                if (gear.captured()) // Forward mouse wheel events to all parents. Wheeling while button pressed.
                {
                    auto& offset = this->base::coor();
                    gear.pass<tier::release>(this->parent(), offset);
                }
            };
            //todo make it configurable
            LISTEN(tier::release, e2::form::drag::start                ::left,  gear) { if (selection_passed()) selection_create(gear); };
            LISTEN(tier::release, e2::form::drag::pull                 ::left,  gear) { if (selection_passed()) selection_extend(gear); };
            LISTEN(tier::release, e2::form::drag::stop                 ::left,  gear) {                         selection_finish(gear); };
            LISTEN(tier::release, e2::form::drag::cancel               ::left,  gear) {                         selection_cancel();     };
            LISTEN(tier::release, hids::events::mouse::button::click   ::right, gear) {                         selection_pickup(gear); };
            LISTEN(tier::release, hids::events::mouse::button::click   ::left,  gear) {                         selection_lclick(gear); };
            LISTEN(tier::release, hids::events::mouse::button::click   ::middle,gear) {                         selection_mclick(gear); };
            LISTEN(tier::release, hids::events::mouse::button::dblclick::left,  gear) { if (selection_passed()) selection_dblclk(gear); };
            LISTEN(tier::release, hids::events::mouse::button::tplclick::left,  gear) { if (selection_passed()) selection_tplclk(gear); };
            LISTEN(tier::release, hids::events::mouse::scroll::any, gear)
            {
                if (gear.meta(hids::anyCtrl)) return; // Ctrl+Wheel is reserved for zooming.
                if (altscr && target == &altbuf)
                {
                    auto deed = this->bell::template protos<tier::release>();
                    switch (deed)
                    {
                        case hids::events::mouse::scroll::up.id:   data_out(utf::repeat("\033[A"sv, altscr)); break;
                        case hids::events::mouse::scroll::down.id: data_out(utf::repeat("\033[B"sv, altscr)); break;
                    }
                    gear.dismiss();
                }
            };
        }
        void selection_search(hids& gear, feed dir)
        {
            auto& console = *target;
            auto delta = dot_00;
            auto fwd = dir == feed::fwd;
            if (console.selection_active())
            {
                if (console.match.empty())
                {
                    delta.y = fwd ? -console.arena // Page by page scrolling if nothing to search.
                                  :  console.arena;
                }
                else delta = console.selection_search(dir);
            }
            else
            {
                gear.owner.RISEUP(tier::request, hids::events::clipbrd, gear);
                auto& data = gear.board::cargo;
                if (data.utf8.size())
                {
                    delta = console.selection_search(dir, data.utf8);
                }
                else // Page by page scrolling if nothing to search.
                {
                    delta.y = fwd ? -console.arena
                                  :  console.arena;
                }
            }
            SIGNAL(tier::release, ui::term::events::search::status, console.selection_button(delta));
            if (target == &normal && delta)
            {
                selection_moveto(delta);
            }
        }
        void search(hids& gear, feed dir)
        {
            selection_search(gear, dir);
        }
        void set_color(cell brush)
        {
            //todo remove base::color dependency (background is colorized twice! use transparent target->brush)
            brush.link(base::id);
            base::color(brush);
            target->brush.reset(brush);
        }
        void set_bg_color(rgba bg)
        {
            //todo remove base::color dependency (background is colorized twice! use transparent target->brush)
            auto brush = base::color();
            brush.bgc(bg);
            brush.link(base::id);
            base::color(brush);
            target->brush.reset(brush);
            SIGNAL(tier::release, ui::term::events::colors::bg, bg);
        }
        void set_fg_color(rgba fg)
        {
            //todo remove base::color dependency (background is colorized twice! use transparent target->brush)
            auto brush = base::color();
            brush.fgc(fg);
            brush.link(base::id);
            base::color(brush);
            target->brush.reset(brush);
            SIGNAL(tier::release, ui::term::events::colors::fg, fg);
        }
        void set_wrapln(si32 wrapln)
        {
            target->selection_setwrp((wrap)wrapln);
            if (!target->selection_active())
            {
                follow[axis::Y] = true; // Reset viewport.
            }
            ondata(""); // Recalc trigger.
        }
        void set_align(si32 align)
        {
            if (target->selection_active())
            {
                target->selection_setjet((bias)align);
            }
            else
            {
                target->style.jet((bias)align);
                follow[axis::Y] = true; // Reset viewport.
            }
            ondata(""); // Recalc trigger.
        }
        void set_selmod(si32 mode)
        {
            selection_selmod(mode);
            if (faux == target->selection_active()) follow[axis::Y] = true; // Reset viewport.
            ondata(""); // Recalc trigger.
        }
        void set_selalt(bool boxed)
        {
            selection_selalt(boxed);
            if (faux == target->selection_active()) follow[axis::Y] = true; // Reset viewport.
            ondata(""); // Recalc trigger.
        }
        void set_log(bool state)
        {
            io_log = state;
            SIGNAL(tier::release, ui::term::events::io_log, state);
        }
        void exec_cmd(commands::ui::commands cmd)
        {
            if constexpr (debugmode) log(prompt::term, "Command: ", cmd);
            auto& console = *target;
            switch (cmd)
            {
                case commands::ui::togglewrp: console.selection_setwrp();          break;
                case commands::ui::togglesel: selection_selmod();                  break;
                case commands::ui::restart:   restart();                           break;
                case commands::ui::sighup:    sighup(true);                        break;
                case commands::ui::undo:      ipccon.undo(true);                   break;
                case commands::ui::redo:      ipccon.undo(faux);                   break;
                case commands::ui::deselect:  selection_cancel();                  break;
                case commands::ui::look_fwd:  console.selection_search(feed::fwd); break;
                case commands::ui::look_rev:  console.selection_search(feed::rev); break;
                default: break;
            }
            if (!console.selection_active())
            {
                follow[axis::Y] = true; // Reset viewport.
            }
            ondata(""); // Recalc trigger.
        }
        void data_in(view data)
        {
            follow[axis::Y] = true;
            ondata(data);
        }
        void data_out(view data)
        {
            follow[axis::Y] = true;
            write(data);
        }
        void onexit(si32 code, text msg = {})
        {
            netxs::events::enqueue<faux>(This(), [&, code, msg, backup = This()](auto& boss)
            {
                ipccon.cleanup(io_log);
                auto lock = netxs::events::sync{};
                auto error = [&]
                {
                    auto byemsg = escx{};
                    if (target != &normal) byemsg.locate({ 0, target->panel.y - 1 });
                    byemsg.bgc(code ? rgba{ reddk } : rgba{}).fgc(whitelt).add(msg)
                          .add("\r\nProcess exited with code ", utf::to_hex_0x(code)).nil()
                          .add("\r\n\n");
                    return byemsg;
                };
                auto query = [&]
                {
                    auto byemsg = error().add("Press Esc to close or press Enter to restart the session.\r\n")
                                         .add("\n");
                    ondata(byemsg);
                    this->LISTEN(tier::release, hids::events::keybd::data::post, gear, onerun) //todo VS2019 requires `this`
                    {
                        if (gear.pressed && gear.cluster.size())
                        {
                            switch (gear.cluster.front())
                            {
                                case ansi::c0_esc: close(); onerun.reset(); break;
                                case ansi::c0_cr:  start(); onerun.reset(); break;
                            }
                        }
                        //if (gear.pressed)
                        //{
                        //    switch (gear.keybd::generic())
                        //    {
                        //        case key::Esc:   close(); onerun.reset(); break;
                        //        case key::Enter: start(); onerun.reset(); break;
                        //    }
                        //}
                    };
                };
                auto renew = [&]
                {
                    auto byemsg = error().add("\n");
                    ondata(byemsg);
                    start();
                };
                     if (forced)                close();
                else if (resume.exchange(faux)) renew();
                else switch (config.def_atexit)
                {
                    case commands::atexit::smart: code ? query()
                                                       : close(); break;
                    case commands::atexit::retry: code ? renew()
                                                       : close(); break;
                    case commands::atexit::ask:          query(); break;
                    case commands::atexit::close:        close(); break;
                    case commands::atexit::restart:      renew(); break;
                }
            });
        }
        void start()
        {
            if (!ipccon)
            {
                ipccon.start(*this, curdir, cmdarg, target->panel);
            }
        }
        void close()
        {
            this->RISEUP(tier::release, e2::form::proceed::quit::one, forced); //todo VS2019 requires `this`
        }
        void restart()
        {
            resume.exchange(true);
            ipccon.sighup(faux);
        }
        void sighup(bool fast)
        {
            forced = fast;
            if (ipccon)
            {
                if (ipccon.sighup())
                {
                    netxs::events::enqueue<faux>(This(), [&, backup = This()](auto& boss) mutable
                    {
                        ipccon.cleanup(io_log); // Wait child process.
                        close();
                    });
                }
            }
            else // Child process exited with non-zero code and term waits keypress.
            {
                onerun.reset();
                close();
            }
        }
        // term: Resize terminal window.
        void window_resize(twod winsz)
        {
            RISEUP(tier::preview, e2::form::prop::window::size, winsz);
        }
        // term: Custom data output (ConSrv callback).
        template<class Fx>
        void data(rich& cooked, Fx fx)
        {
            if (auto count = cooked.size().x)
            {
                auto& proto = cooked.pick();
                auto& brush = target == &normal ? normal.parser::brush
                                                : altbuf.parser::brush;
                cooked.each([&](cell& c) { c.meta(brush); });
                if (target == &normal) normal._data(count, proto, fx);
                else                   altbuf._data(count, proto, fx);
            }
        }

    protected:
        // term: Recalc metrics for the new viewport size.
        void deform(rect& new_area) override
        {
            new_area += base::intpad;
            auto& console = *target;
            auto scroll_coor = origin;
            new_area.size = std::max(new_area.size, dot_11);
            console.resize_viewport(new_area.size);
            console.recalc_pads(base::oversz);
            scroll(origin);
            base::anchor += scroll_coor - origin;
            ipccon.resize(new_area.size);
            new_area.size.y += console.get_basis();
            new_area -= base::intpad;
        }
        term(text cwd, text cmd, xmls& xml_config)
            : config{ xml_config },
              normal{ *this },
              altbuf{ *this },
              target{&normal},
              cursor{ *this, config.def_cur_on, config.def_cursor, dot_00, config.def_period },
              worker{ *this },
              dynamo{ *this },
              mtrack{ *this },
              ftrack{ *this },
              wtrack{ *this },
              ctrack{ *this },
              follow{  0, 1 },
              decckm{  faux },
              bpmode{  faux },
              unsync{  faux },
              invert{  faux },
              resume{  faux },
              forced{  faux },
              styled{  faux },
              curdir{ cwd   },
              cmdarg{ cmd   },
              io_log{ config.def_io_log },
              selmod{ config.def_selmod },
              selalt{ config.def_selalt },
              altscr{ config.def_altscr },
              kbmode{ prot::vt }
        {
            selection_submit();
            publish_property(ui::term::events::io_log,         [&](auto& v){ v = io_log; });
            publish_property(ui::term::events::selmod,         [&](auto& v){ v = selmod; });
            publish_property(ui::term::events::selalt,         [&](auto& v){ v = selalt; });
            publish_property(ui::term::events::colors::bg,     [&](auto& v){ v = target->brush.bgc(); });
            publish_property(ui::term::events::colors::fg,     [&](auto& v){ v = target->brush.fgc(); });
            publish_property(ui::term::events::layout::wrapln, [&](auto& v){ v = (si32)target->style.wrp(); });
            publish_property(ui::term::events::layout::align,  [&](auto& v){ v = (si32)target->style.jet(); });
            publish_property(ui::term::events::search::status, [&](auto& v){ v = target->selection_button(); });
            selection_selmod(config.def_selmod);

            LISTEN(tier::general, e2::timer::tick, timestamp) // Update before world rendering.
            {
                if (unsync)
                {
                    unsync = faux;
                    auto& console = *target;
                    auto scroll_size = console.panel;
                    scroll_size.y += console.get_basis();
                    auto scroll_coor = origin;
                    scroll(scroll_coor);
                    auto adjust_pads = console.recalc_pads(base::oversz);
                    if (scroll_size != base::size() // Update scrollbars.
                     || scroll_coor != origin
                     || adjust_pads)
                    {
                        auto new_area = rect{ scroll_coor, scroll_size };
                        this->SIGNAL(tier::release, e2::area, new_area);
                        base::region = new_area;
                    }
                    base::deface();
                }
            };
            LISTEN(tier::release, e2::area, new_area)
            {
                if (new_area.coor != base::coor())
                {
                    auto& console = *target;
                    auto fresh_coor = -new_area.coor.y;
                    follow[axis::Y] = console.set_slide(fresh_coor);
                    new_area.coor.y = -fresh_coor;
                    origin = new_area.coor;
                }
            };
            LISTEN(tier::release, hids::events::keybd::data::post, gear)
            {
                //todo configurable Ctrl+Ins, Shift+Ins etc.
                if (gear.handled) return; // Don't pass registered keyboard shortcuts.
                if (gear.cluster.size())
                {
                    this->RISEUP(tier::release, e2::form::animate::reset, 0); // Reset scroll animation.
                }

                if (gear.pressed && config.resetonkey
                && (gear.cluster.size() || !gear.kbmod()))
                {
                    unsync = true;
                    follow[axis::X] = true;
                    follow[axis::Y] = true;
                }

                //if (input::key::map::name(gear.keycode) == input::key::undef)
                //{
                //    log("gear.keycode: ",  gear.keycode,
                //        " pressed: ", gear.pressed,
                //        " virtcod: ", gear.virtcod,
                //        " scancod: ", gear.scancod);
                //}
                if (io_log) log(prompt::key, ansi::hi(input::key::map::name(gear.keycode)));

                ipccon.keybd(gear, decckm, bpmode, kbmode);
            };
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                auto& console = *target;
                if (status.update(console))
                {
                    this->RISEUP(tier::preview, e2::form::prop::ui::footer, status.data);
                }

                auto view = parent_canvas.view();
                auto full = parent_canvas.full();
                auto base = full.coor - view.coor;
                cursor.coor(console.get_coord(base));

                console.output(parent_canvas);
                if (invert) parent_canvas.fill(cell::shaders::invbit);

                if (oversz.b > 0) // Shade the viewport bottom oversize (futures).
                {
                    auto bottom_oversize = full;
                    bottom_oversize.coor.x -= oversz.l;
                    bottom_oversize.coor.y += console.get_basis() + console.panel.y - console.scend;
                    bottom_oversize.size.y  = oversz.b;
                    bottom_oversize.size.x += oversz.l + oversz.r;
                    bottom_oversize = bottom_oversize.clip(view);
                    parent_canvas.fill(bottom_oversize, cell::shaders::xlight);
                }

                if (view.coor.x) // Shade left and right margins.
                {
                    auto west = full;
                    west.size = dot_mx;
                    west.coor.y -= dot_mx.y / 2;
                    auto east = west;
                    auto pads = console.getpad();
                    west.coor.x -= oversz.l - pads + dot_mx.x;
                    east.coor.x += oversz.r - pads + console.panel.x;
                    west = west.clip(view);
                    east = east.clip(view);
                    parent_canvas.fill(west, cell::shaders::xlucent(config.def_lucent));
                    parent_canvas.fill(east, cell::shaders::xlucent(config.def_lucent));
                }

                // Debug: Shade active viewport.
                //{
                //    auto size = console.panel;
                //    size.y -= console.sctop + console.scend;
                //    auto vp = rect{{ 0,console.get_basis() + console.sctop }, size };
                //    vp.coor += parent_canvas.full().coor;
                //    vp = vp.clip(parent_canvas.view());
                //    parent_canvas.fill(vp, [](auto& c){ c.fuse(cell{}.bgc(magentalt).bga(50)); });
                //}
            };
        }
    };

    class dtvt
        : public ui::form<dtvt>
    {
        using s11n = directvt::binary::s11n;

        // dtvt: Event handler.
        struct evnt
            : public s11n
        {
            dtvt& master; // evnt: Terminal object reference.
            subs  tokens; // evnt: Subscription tokens.
            escx  output; // evnt: Logs buffer.
            escx  prompt; // evnt: Logs prompt.

            void disable()
            {
                s11n::stop();
                tokens.clear();
            }

            void handle(s11n::xs::bitmap_dtvt         lock)
            {
                auto& bitmap = lock.thing;
                if (bitmap.newgc.size())
                {
                    auto list = s11n::request_gc.freeze();
                    for (auto& gc_map : bitmap.newgc)
                    {
                        list.thing.push(gc_map.first);
                    }
                    bitmap.newgc.clear();
                    list.thing.sendby(master);
                }
                lock.unlock();
                netxs::events::enqueue(master.This(), [&](auto& boss) mutable
                {
                    master.base::deface();
                });
            }
            void handle(s11n::xs::tooltips            lock)
            {
                auto copy = lock.thing;
                netxs::events::enqueue(master.This(), [tooltips = std::move(copy)](auto& boss) mutable
                {
                    for (auto& tooltip : tooltips)
                    {
                        if (auto gear_ptr = bell::getref<hids>(tooltip.gear_id))
                        {
                            gear_ptr->set_tooltip(tooltip.tip_text, tooltip.update);
                        }
                    }
                });
            }
            void handle(s11n::xs::jgc_list            lock)
            {
                for (auto& jgc : lock.thing)
                {
                    cell::gc_set_data(jgc.token, jgc.cluster);
                    if constexpr (debugmode) log(prompt::dtvt, "New gc token: ", jgc.token, " cluster size ", jgc.cluster.size(), " data: ", jgc.cluster);
                }
                netxs::events::enqueue(master.This(), [&](auto& boss) mutable
                {
                    master.base::deface();
                });
            }
            void handle(s11n::xs::fullscreen          lock)
            {
                auto& m = lock.thing;
                master.trysync(master.active, [&]
                {
                    if (auto gear_ptr = bell::getref<hids>(m.gear_id))
                    if (auto parent_ptr = master.base::parent())
                    {
                        auto& gear = *gear_ptr;
                        if (gear.captured(master.id)) gear.setfree(true);
                        parent_ptr->RISEUP(tier::release, e2::form::layout::fullscreen, gear);
                    }
                });
            }
            void handle(s11n::xs::focus_cut           lock)
            {
                auto& k = lock.thing;
                master.trysync(master.active, [&]
                {
                    if (auto gear_ptr = bell::getref<hids>(k.gear_id))
                    if (auto parent_ptr = master.base::parent())
                    {
                        parent_ptr->RISEUP(tier::preview, hids::events::keybd::focus::cut, seed, ({ .id = k.gear_id, .item = master.This() }));
                    }
                });
            }
            void handle(s11n::xs::focus_set           lock)
            {
                auto& k = lock.thing;
                master.trysync(master.active, [&]
                {
                    if (auto gear_ptr = bell::getref<hids>(k.gear_id))
                    if (auto parent_ptr = master.base::parent())
                    {
                        parent_ptr->RISEUP(tier::preview, hids::events::keybd::focus::set, seed, ({ .id = k.gear_id, .solo = k.solo, .item = master.This() }));
                    }
                });
            }
            void handle(s11n::xs::keybd_event         lock)
            {
                auto& k = lock.thing;
                master.trysync(master.active, [&]
                {
                    if (auto gear_ptr = bell::getref<hids>(k.gear_id))
                    if (auto parent_ptr = master.base::parent())
                    {
                        auto& gear = *gear_ptr;
                        //todo use temp gear object
                        gear.alive    = true;
                        gear.ctlstate = k.ctlstat;
                        gear.extflag  = k.extflag;
                        gear.virtcod  = k.virtcod;
                        gear.scancod  = k.scancod;
                        gear.pressed  = k.pressed;
                        gear.cluster  = k.cluster;
                        gear.handled  = k.handled;
                        do
                        {
                            parent_ptr->SIGNAL(tier::release, hids::events::keybd::data::post, gear);
                            parent_ptr = parent_ptr->parent();
                        }
                        while (gear && parent_ptr);
                    }
                });
            };
            void handle(s11n::xs::mouse_event         lock)
            {
                auto& m = lock.thing;
                master.trysync(master.active, [&]
                {
                    if (auto gear_ptr = bell::getref<hids>(m.gear_id))
                    if (auto parent_ptr = master.base::parent())
                    {
                        auto& gear = *gear_ptr;
                        if (gear.captured(master.id)) gear.setfree(true);
                        auto basis = gear.owner.base::coor();
                        master.global(basis);
                        gear.replay(m.cause, m.coord - basis, m.delta, m.buttons, m.ctlstat);
                        gear.pass<tier::release>(parent_ptr, gear.owner.base::coor(), true);
                    }
                });
            }
            void handle(s11n::xs::minimize            lock)
            {
                auto& m = lock.thing;
                netxs::events::enqueue(master.This(), [&](auto& boss)
                {
                    if (auto gear_ptr = bell::getref<hids>(m.gear_id))
                    {
                        auto& gear = *gear_ptr;
                        master.RISEUP(tier::release, e2::form::layout::minimize, gear);
                    }
                });
            }
            void handle(s11n::xs::expose              lock)
            {
                netxs::events::enqueue(master.This(), [&](auto& boss)
                {
                    master.RISEUP(tier::preview, e2::form::layout::expose, area, ());
                });
            }
            void handle(s11n::xs::clipdata            lock)
            {
                auto& c = lock.thing;
                master.trysync(master.active, [&]
                {
                    if (auto gear_ptr = bell::getref<hids>(c.gear_id))
                    {
                        gear_ptr->set_clipboard(c);
                    }
                });
            }
            void handle(s11n::xs::clipdata_request    lock)
            {
                auto& c = lock.thing;
                master.trysync(master.active, [&]
                {
                    if (auto gear_ptr = bell::getref<hids>(c.gear_id))
                    {
                        auto& gear = *gear_ptr;
                        gear.owner.RISEUP(tier::request, hids::events::clipbrd, gear);
                        auto& data = gear.board::cargo;
                        if (data.hash != c.hash)
                        {
                            s11n::clipdata.send(master, c.gear_id, data.hash, data.size, data.utf8, data.form);
                            return;
                        }
                    }
                    else log(prompt::dtvt, ansi::err("Unregistered input device id: ", c.gear_id));
                    s11n::clipdata.send(master, c.gear_id, c.hash, dot_00, text{}, mime::ansitext);
                });
            }
            //void handle(s11n::xs::focus               lock)
            //{
            //    auto& f = lock.thing;
            //    master.trysync(master.active, [&]
            //    {
            //        if (auto gear_ptr = bell::getref<hids>(f.gear_id))
            //        {
            //            auto& gear = *gear_ptr;
            //            if (f.state) gear.kb_offer_8(master.This(), f.focus_force_group);
            //            else         gear.remove_from_kb_focus(master.This());
            //            if (f.state) pro::focus::set(master.This(), gear.id, f.focus_force_group ? pro::focus::solo::off : pro::focus::solo::on, pro::focus::flip::off);
            //            else         pro::focus::off(master.This(), gear.id);
            //        }
            //    });
            //}
            void handle(s11n::xs::header              lock)
            {
                auto& h = lock.thing;
                netxs::events::enqueue(master.This(), [&, id = h.window_id, header = h.utf8](auto& boss) mutable
                {
                    master.RISEUP(tier::preview, e2::form::prop::ui::header, header);
                });
            }
            void handle(s11n::xs::footer              lock)
            {
                auto& f = lock.thing;
                netxs::events::enqueue(master.This(), [&, id = f.window_id, footer = f.utf8](auto& boss) mutable
                {
                    master.RISEUP(tier::preview, e2::form::prop::ui::footer, footer);
                });
            }
            void handle(s11n::xs::header_request      lock)
            {
                auto& c = lock.thing;
                master.trysync(master.active, [&]
                {
                    //todo use window_id
                    master.RISEUP(tier::request, e2::form::prop::ui::header, header, ());
                    s11n::header.send(master, c.window_id, header);
                });
            }
            void handle(s11n::xs::footer_request      lock)
            {
                auto& c = lock.thing;
                master.trysync(master.active, [&]
                {
                    //todo use window_id
                    master.RISEUP(tier::request, e2::form::prop::ui::footer, footer, ());
                    s11n::footer.send(master, c.window_id, footer);
                });
            }
            void handle(s11n::xs::warping             lock)
            {
                auto& w = lock.thing;
                netxs::events::enqueue(master.This(), [&, id = w.window_id, warp = w.warpdata](auto& boss)
                {
                    //todo use window_id
                    master.RISEUP(tier::preview, e2::form::layout::swarp, warp);
                });
            }
            void handle(s11n::xs::vt_command          lock)
            {
                auto& w = lock.thing;
                //todo implement
            }
            void handle(s11n::xs::fps                 lock)
            {
                netxs::events::enqueue(master.This(), [&, fps = lock.thing.frame_rate](auto& boss) mutable
                {
                    boss.SIGNAL(tier::general, e2::config::fps, fps);
                });
            }
            void handle(s11n::xs::logs                lock)
            {
                if (lock.thing.guid != os::process::id.second) // To avoid overflow on recursive dtvt connections.
                {
                    auto utf8 = view{ lock.thing.data };
                    if (utf8.size() && utf8.back() == '\n') utf8.remove_suffix(1);
                    if (lock.thing.id == 0) // Message from our process.
                    {
                        log(utf8);
                    }
                    else
                    {
                        prompt.add(netxs::prompt::pads, lock.thing.id, ": "); // Local host pid and remote host pid can be different. It is different if sshed.
                        utf::divide(utf8, '\n', [&](auto line)
                        {
                            output.add(prompt, line, '\n');
                        });
                        log<faux>(output);
                        output.clear();
                        prompt.clear();
                    }
                }
            }
            void handle(s11n::xs::fatal               lock)
            {
                netxs::events::enqueue(master.This(), [&](auto& boss)
                {
                    auto utf8 = view{ lock.thing.err_msg };
                    master.errmsg = master.genmsg(utf8);
                    master.deface();
                });
            }
            void handle(s11n::xs::sysclose            lock)
            {
                master.active.exchange(faux);
                master.stop(true);
            }

            evnt(dtvt& master)
                :   s11n{ *this, master.id },
                  master{ master           }
            {
                master.LISTEN(tier::release, hids::events::device::mouse::any, gear, tokens)
                {
                    if (gear.captured(master.id))
                    {
                        if (!gear.m.buttons) gear.setfree(true);
                    }
                    else if (gear.m.buttons) gear.capture(master.id);
                    gear.m.gear_id = gear.id;
                    s11n::sysmouse.send(master, gear.m);
                    gear.dismiss();
                };
                master.LISTEN(tier::general, hids::events::die, gear, tokens)
                {
                    gear.setfree(true);
                    gear.m.gear_id = gear.id;
                    gear.m.enabled = hids::stat::die;
                    s11n::sysmouse.send(master, gear.m);
                };
                master.LISTEN(tier::general, hids::events::halt, gear, tokens)
                {
                    gear.m.gear_id = gear.id;
                    gear.m.enabled = hids::stat::halt;
                    s11n::sysmouse.send(master, gear.m);
                };
                master.LISTEN(tier::release, hids::events::notify::mouse::leave, gear, tokens)
                {
                    gear.m.gear_id = gear.id;
                    gear.m.enabled = hids::stat::halt;
                    s11n::sysmouse.send(master, gear.m);
                };
                master.LISTEN(tier::release, hids::events::keybd::focus::bus::any, seed, tokens)
                {
                    auto deed = master.bell::template protos<tier::release>();
                    if (seed.guid == decltype(seed.guid){}) // To avoid focus tree infinite looping.
                    {
                        seed.guid = os::process::id.second;
                    }
                    s11n::focusbus.send(master, seed.id, seed.guid, netxs::events::subindex(deed));
                };
                master.LISTEN(tier::release, hids::events::keybd::data::post, gear, tokens)
                {
                    s11n::syskeybd.send(master, gear.id,
                                                gear.ctlstate,
                                                gear.extflag,
                                                gear.virtcod,
                                                gear.scancod,
                                                gear.pressed,
                                                gear.cluster,
                                                gear.handled,
                                                gear.keycode);
                    gear.dismiss();
                };
                //master.LISTEN(tier::release, hids::events::upevent::kboffer, gear, tokens)
                //{
                //    auto focus_state = true;
                //    s11n::sysfocus.send(master, gear.id,
                //                                focus_state,
                //                                gear.focus_combine,
                //                                gear.focus_force_group);
                //};
                //master.LISTEN(tier::release, hids::events::upevent::kbannul, gear, tokens)
                //{
                //    gear.remove_from_kb_focus(master.This());
                //    auto focus_state = faux;
                //    s11n::sysfocus.send(master, gear.id,
                //                                focus_state,
                //                                gear.focus_combine,
                //                                gear.focus_force_group);
                //};
                //master.LISTEN(tier::release, hids::events::notify::keybd::lost, gear, tokens)
                //{
                //    auto focus_state = faux;
                //    s11n::sysfocus.send(master, gear.id,
                //                                focus_state,
                //                                gear.focus_combine,
                //                                gear.focus_force_group);
                //};
                master.LISTEN(tier::general, e2::config::fps, frame_rate, tokens)
                {
                    if (frame_rate > 0)
                    {
                        s11n::fps.send(master, frame_rate);
                    }
                };
                master.LISTEN(tier::anycast, e2::form::prop::ui::slimmenu, slim, tokens)
                {
                    s11n::slimmenu.send(master, slim);
                };
                master.LISTEN(tier::anycast, e2::form::prop::colors::any, clr, tokens)
                {
                    auto deed = master.bell::template protos<tier::anycast>();
                         if (deed == e2::form::prop::colors::bg.id) s11n::bgc.send(master, clr);
                    else if (deed == e2::form::prop::colors::fg.id) s11n::fgc.send(master, clr);
                };
                master.LISTEN(tier::release, e2::area, new_area, tokens)
                {
                    //todo implement deform/inform (for incoming XTWINOPS/swarp)
                    auto winsize = s11n::syswinsz.freeze().thing.winsize;
                    if (master.ipccon && winsize != new_area.size)
                    {
                        s11n::syswinsz.send(master, 0, new_area.size);
                    }
                };
            }
        };

        struct msgs
        {
            static constexpr auto no_signal    = "NO SIGNAL"sv;
            static constexpr auto disconnected = "Disconnected"sv;
        };

        using vtty = os::dtvt::vtty;

        evnt stream; // dtvt: Event handler.
        text curdir; // dtvt: Current working directory.
        text cmdarg; // dtvt: Startup command line arguments.
        text xmlcfg; // dtvt: Startup config.
        flag active; // dtvt: Terminal lifetime.
        si32 nodata; // dtvt: Show splash "No signal".
        face splash; // dtvt: "No signal" splash.
        page errmsg; // dtvt: Overlay error message.
        span maxoff; // dtvt: Max delay before showing "No signal".
        byte opaque; // dtvt: Object transparency on d_n_d (no pro::cache).
        vtty ipccon; // dtvt: IPC connector. Should be destroyed first.

        // dtvt: Proceed DirectVT output.
        void ondata(view data)
        {
            if (active)
            {
                stream.s11n::sync(data);
            }
        }

    public:
        // dtvt: Format error message overlay.
        auto genmsg(view utf8) -> page
        {
            auto data = para{ utf8 };
            auto kant = text(2, ' ');
            auto pads = text(data.length() + kant.size() * 2, ' ');
            return page{ ansi::bgc(reddk).fgc(whitelt).jet(bias::center).wrp(wrap::off).cup(dot_00).cpp({50,50}).cuu(1)
                              .add(pads,       '\n',
                             kant, utf8, kant, '\n',
                                   pads,       '\n') };
        }
        // dtvt: Shutdown callback handler.
        void onexit()
        {
            if (!active.exchange(faux)) return;
            netxs::events::enqueue(This(), [&](auto& boss) mutable // Unexpected disconnection.
            {
                auto lock = stream.bitmap_dtvt.freeze();
                auto& canvas = lock.thing.image;
                nodata = canvas.hash();
                errmsg = genmsg(msgs::disconnected);
                fallback(canvas, true);
                base::deface();
            });
        }
        // dtvt: Send client data.
        void output(view data)
        {
            ipccon.output(data);
        }
        // dtvt: Attach a new process.
        void start()
        {
            if (!ipccon)
            {
                auto winsize = base::size();
                ipccon.start(curdir, cmdarg, xmlcfg, winsize, [&](view utf8) { ondata(utf8); },
                                                              [&]            { onexit();     });
            }
        }
        // dtvt: Close dtvt-object.
        void stop(bool fast)
        {
            if (fast && active.exchange(faux) && ipccon) // Notify and queue closing immediately.
            {
                stream.s11n::sysclose.send(*this, fast);
            }
            else if (active && ipccon) // Just notify if active. Queue closing if not.
            {
                stream.s11n::sysclose.send(*this, fast);
                return;
            }
            netxs::events::enqueue<faux>(This(), [&, backup = This()](auto& boss) mutable
            {
                ipccon.cleanup();
                this->RISEUP(tier::release, e2::form::proceed::quit::one, true); // MSVC2019
            });
        }
        // dtvt: Splash screen if there is no next frame.
        void fallback(core const& canvas, bool forced = faux)
        {
            auto size = base::size();
            if (splash.size() != size || forced)
            {
                splash.size(size);
                auto parent_id = id_t{}; // Handover control to the parent if no response.
                if (auto parent = base::parent()) parent_id = parent->id;
                if (canvas.size())
                {
                    splash.zoom(canvas, cell::shaders::fullid(parent_id));
                    splash.output(errmsg);
                    splash.blur(2, [](cell& c) { c.fgc(rgba::transit(c.bgc(), c.fgc(), 127)); });
                    splash.output(errmsg);
                }
                else splash.wipe(cell{}.link(parent_id).bgc(blacklt).bga(0x40));
            }
        }
        // dtvt: Render next frame.
        void fill(core& parent_canvas, core const& canvas)
        {
            if (opaque == 0xFF) parent_canvas.fill(canvas, cell::shaders::fusefull);
            else                parent_canvas.fill(canvas, cell::shaders::transparent(opaque));
        }

    protected:
        dtvt(text cwd, text cmd, text cfg)
            : stream{*this },
              active{ true },
              opaque{ 0xFF },
              nodata{      },
              curdir{ cwd  },
              cmdarg{ cmd  },
              xmlcfg{ cfg  },
              errmsg{ genmsg(msgs::no_signal) }
        {
            //todo make it configurable (max_drops)
            static constexpr auto max_drops = 1;
            SIGNAL(tier::general, e2::config::fps, fps, (-1));
            maxoff = max_drops * span{ span::period::den / fps };
            LISTEN(tier::general, e2::config::fps, fps)
            {
                maxoff = max_drops * span{ span::period::den / fps };
            };
            LISTEN(tier::anycast, e2::form::prop::lucidity, value)
            {
                if (value == -1) value = opaque;
                else             opaque = value;
            };
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                auto size = base::size();
                auto lock = stream.bitmap_dtvt.freeze();
                auto& canvas = lock.thing.image;
                if (nodata == canvas.hash()) // " No signal " on timeout > 1/60s
                {
                    fallback(canvas);
                    fill(parent_canvas, splash);
                    return;
                }
                else if (size == canvas.size())
                {
                    fill(parent_canvas, canvas);
                }
                else if (canvas.size())
                {
                    while (size != canvas.size()) // Always waiting for the correct frame.
                    {
                        if (!active) return;
                        if (std::cv_status::timeout == lock.wait_for(maxoff)
                         && size != canvas.size())
                        {
                            nodata = canvas.hash();
                            fallback(canvas);
                            fill(parent_canvas, splash);
                            return;
                        }
                    }
                    fill(parent_canvas, canvas);
                }
            };
        }
    };
}