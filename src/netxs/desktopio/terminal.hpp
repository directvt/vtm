// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

#include "console.hpp"

namespace netxs::events::userland
{
    namespace terminal
    {
        EVENTPACK( terminal::events, ui::e2::extra::slot5 )
        {
            EVENT_XS( io_log,  si32 ),
            EVENT_XS( selmod,  si32 ),
            EVENT_XS( onesht,  si32 ),
            EVENT_XS( selalt,  si32 ),
            EVENT_XS( rawkbd,  si32 ),
            GROUP_XS( toggle,  si32 ),
            GROUP_XS( preview, si32 ),
            GROUP_XS( release, si32 ),
            GROUP_XS( colors,  argb ),
            GROUP_XS( layout,  si32 ),
            GROUP_XS( search,  input::hids ),

            SUBSET_XS( toggle )
            {
                EVENT_XS( cwdsync, si32 ), // preview: Request to toggle cwdsync.
            };
            SUBSET_XS( preview )
            {
                EVENT_XS( cwdsync, si32 ),
            };
            SUBSET_XS( release )
            {
                EVENT_XS( cwdsync, si32 ),
            };
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
                EVENT_XS( bg, argb ),
                EVENT_XS( fg, argb ),
            };
        };
    }
}

// terminal: Terminal UI control.
namespace netxs::ui
{
    namespace terminal
    {
        namespace event_source
        {
            static constexpr auto _counter = __COUNTER__ + 1;
            static constexpr auto keyboard  = 1 << (__COUNTER__ - _counter);
            static constexpr auto mouse     = 1 << (__COUNTER__ - _counter);
            static constexpr auto focus     = 1 << (__COUNTER__ - _counter);
            static constexpr auto format    = 1 << (__COUNTER__ - _counter);
            static constexpr auto clipboard = 1 << (__COUNTER__ - _counter);
            static constexpr auto window    = 1 << (__COUNTER__ - _counter);
            static constexpr auto system    = 1 << (__COUNTER__ - _counter);
        }
        namespace events = netxs::events::userland::terminal;
        static auto event_source_map = utf::unordered_map<text, si32>
           {{ "keyboard"s,  event_source::keyboard  },
            { "mouse"s,     event_source::mouse     },
            { "focus"s,     event_source::focus     },
            { "format"s,    event_source::format    },
            { "clipboard"s, event_source::clipboard },
            { "window"s,    event_source::window    },
            { "system"s,    event_source::system    }};
    }

    struct term
        : public ui::form<term>
    {
        static constexpr auto classname = basename::terminal;
        static constexpr auto event_source_name = std::to_array(
        {
            "keyboard",
            "mouse",
            "focus",
            "format",
            "clipboard",
            "window",
            "system",
        });

        #define proc_list \
            X(KeyEvent             ) /* */ \
            X(ExclusiveKeyboardMode) /* */ \
            X(FindNextMatch        ) /* */ \
            X(ScrollViewportByPage ) /* */ \
            X(ScrollViewportByCell ) /* */ \
            X(ScrollViewportToTop  ) /* */ \
            X(ScrollViewportToEnd  ) /* */ \
            X(SendKey              ) /* */ \
            X(Print                ) /* */ \
            X(PrintLn              ) /* */ \
            X(CopyViewport         ) /* */ \
            X(CopySelection        ) /* */ \
            X(PasteClipboard       ) /* */ \
            X(ClearClipboard       ) /* */ \
            X(ClipboardFormat      ) /* */ \
            X(SelectionForm        ) /* Linear/Rectangular */ \
            X(ClearSelection       ) /* */ \
            X(OneShotSelection     ) /* One-shot toggle to copy text while mouse tracking is active */ \
            X(UndoReadline         ) /* Undo for cooked read on win32 */ \
            X(RedoReadline         ) /* Redo for cooked read on win32 */ \
            X(CwdSync              ) /* */ \
            X(LineWrapMode         ) /* */ \
            X(LineAlignMode        ) /* */ \
            X(LogMode              ) /* */ \
            X(AltbufMode           ) /* */ \
            X(ForwardKeys          ) /* */ \
            X(ClearScrollback      ) /* */ \
            X(ScrollbackSize       ) /* */ \
            X(EventReporting       ) /* */ \
            X(Restart              ) /* */ \
            X(Quit                 ) /* */ \

        struct methods
        {
            #define X(_proc) static constexpr auto _proc = #_proc;
            proc_list
            #undef X
        };

        #undef proc_list

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
                    toggleraw,
                    togglewrp,
                    togglejet,
                    togglesel,
                    toggleselalt,
                    restart,
                    sighup,
                    undo,
                    redo,
                    deselect,
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
            using pals = std::remove_const_t<decltype(argb::vt256)>;

            si32 def_mxline;
            si32 def_length;
            si32 def_growdt;
            si32 def_growmx;
            wrap def_wrpmod;
            si32 def_tablen;
            si32 def_lucent;
            si32 def_margin;
            si32 def_border;
            si32 def_atexit;
            cell def_curclr;
            argb def_fcolor;
            argb def_bcolor;
            argb def_filler;
            si32 def_selmod;
            si32 def_cursor;
            bool def_selalt;
            bool def_cur_on;
            bool resetonkey;
            bool resetonout;
            bool def_io_log;
            bool allow_logs;
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

            bool def_alt_on;

            text send_input;

            utf::unordered_map<text, ui32> color_names;

            static void recalc_buffer_metrics(si32& def_length, si32& def_growdt, si32& def_growmx)
            {
                if (def_growdt == 0)
                {
                    if (def_length == 0) def_length = def_growmx;
                    else                 def_growmx = def_length;
                }
                if (def_growmx == 0 && def_growdt != 0)
                {
                    def_growmx = std::exchange(def_length, def_growdt);
                }
            }
            termconfig(settings& config)
            {
                static auto atexit_options = utf::unordered_map<text, commands::atexit::codes>
                    {{ "auto",    commands::atexit::smart   },
                     { "ask",     commands::atexit::ask     },
                     { "close",   commands::atexit::close   },
                     { "restart", commands::atexit::restart },
                     { "retry",   commands::atexit::retry   }};
                static auto fx_options = utf::unordered_map<text, commands::fx::shader>
                    {{ "xlight",  commands::fx::xlight  },
                     { "coolor",  commands::fx::color   },
                     { "invert",  commands::fx::invert  },
                     { "reverse", commands::fx::reverse }};

                send_input =             config.settings::take("/config/terminal/sendinput",                 text{});
                def_mxline = std::max(1, config.settings::take("/config/terminal/scrollback/maxline",        si32{ 65535 }));
                def_length = std::max(1, config.settings::take("/config/terminal/scrollback/size",           si32{ 40000 }));
                def_growdt = std::max(0, config.settings::take("/config/terminal/scrollback/growstep",       si32{ 0 }    ));
                def_growmx = std::max(0, config.settings::take("/config/terminal/scrollback/growlimit",      si32{ 0 }    ));
                recalc_buffer_metrics(def_length, def_growdt, def_growmx);
                def_wrpmod =             config.settings::take("/config/terminal/scrollback/wrap",            deco::defwrp == wrap::on) ? wrap::on : wrap::off;
                resetonkey =             config.settings::take("/config/terminal/scrollback/reset/onkey",     true);
                resetonout =             config.settings::take("/config/terminal/scrollback/reset/onoutput",  faux);
                def_alt_on =             config.settings::take("/config/terminal/scrollback/altscroll",       true);
                def_lucent = std::max(0, config.settings::take("/config/terminal/scrollback/oversize/opacity",si32{ 0xC0 } ));
                def_margin = std::max(0, config.settings::take("/config/terminal/scrollback/oversize",        si32{ 0 }    ));
                def_tablen = std::max(1, config.settings::take("/config/terminal/tablen",                     si32{ 8 }    ));
                def_border = std::max(0, config.settings::take("/config/terminal/border",                     si32{ 0 }    ));
                def_selmod =             config.settings::take("/config/terminal/selection/mode",             mime::textonly, xml::options::format);
                def_selalt =             config.settings::take("/config/terminal/selection/rect",             faux);
                def_cur_on =             config.settings::take("/config/cursor/show",                     true);
                def_cursor =             config.settings::take("/config/cursor/style",                    text_cursor::I_bar, xml::options::cursor);
                def_curclr =             config.settings::take("/config/cursor/color",                    cell{});
                def_period =             config.settings::take("/config/cursor/blink",                    span{ skin::globals().blink_period });
                def_io_log =             config.settings::take("/config/debug/logs",        faux);
                allow_logs =             true; // Disallowed for dtty.
                def_atexit =             config.settings::take("/config/terminal/atexit",                     commands::atexit::smart, atexit_options);
                def_fcolor =             config.settings::take("/config/terminal/colors/default/fgc",         argb{ whitelt });
                def_bcolor =             config.settings::take("/config/terminal/colors/default/bgc",         argb{ blackdk });
                def_filler =             config.settings::take("/config/terminal/colors/bground",             argb{ argb::default_color });

                def_safe_c =             config.settings::take("/config/terminal/colors/selection/protected", cell{}.bgc(bluelt)    .fgc(whitelt));
                def_ansi_c =             config.settings::take("/config/terminal/colors/selection/ansi",      cell{}.bgc(bluelt)    .fgc(whitelt));
                def_rich_c =             config.settings::take("/config/terminal/colors/selection/rich",      cell{}.bgc(bluelt)    .fgc(whitelt));
                def_html_c =             config.settings::take("/config/terminal/colors/selection/html",      cell{}.bgc(bluelt)    .fgc(whitelt));
                def_text_c =             config.settings::take("/config/terminal/colors/selection/text",      cell{}.bgc(bluelt)    .fgc(whitelt));
                def_none_c =             config.settings::take("/config/terminal/colors/selection/none",      cell{}.bgc(blacklt)   .fgc(whitedk));
                def_find_c =             config.settings::take("/config/terminal/colors/match",               cell{}.bgc(0xFF007F00).fgc(whitelt));

                def_safe_f =             config.settings::take("/config/terminal/colors/selection/protected/fx", commands::fx::color,  fx_options);
                def_ansi_f =             config.settings::take("/config/terminal/colors/selection/ansi/fx",      commands::fx::xlight, fx_options);
                def_rich_f =             config.settings::take("/config/terminal/colors/selection/rich/fx",      commands::fx::xlight, fx_options);
                def_html_f =             config.settings::take("/config/terminal/colors/selection/html/fx",      commands::fx::xlight, fx_options);
                def_text_f =             config.settings::take("/config/terminal/colors/selection/text/fx",      commands::fx::color,  fx_options);
                def_none_f =             config.settings::take("/config/terminal/colors/selection/none/fx",      commands::fx::color,  fx_options);
                def_find_f =             config.settings::take("/config/terminal/colors/match/fx",               commands::fx::color,  fx_options);

                {
                    auto color_names_context = config.settings::push_context("/config/terminal/colors/names");
                    auto color_name_ptr_list = config.settings::take_ptr_list_for_name("name");
                    for (auto& name_ptr : color_name_ptr_list)
                    {
                        auto name = utf::name2token(config.take_value(name_ptr));
                        auto rgba = config.take_value_from(name_ptr, "rgb", argb{});
                        color_names[name] = rgba.token;
                    }
                }

                std::copy(std::begin(argb::vt256), std::end(argb::vt256), std::begin(def_colors));
                for (auto i = 0; i < 16; i++)
                {
                    def_colors[i] = config.settings::take("/config/terminal/colors/color" + std::to_string(i), def_colors[i]);
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
                    data.jet(bias::right);
                    auto total = std::max(mxsz, peak);
                    //todo optimize?
                    if (total % 1000)
                    {
                        data.add(size, "/"sv, total);
                    }
                    else // Apply decimal prefixes to terminal scrollback size.
                    {
                        if (size % 1000) data.add(size);
                        else
                        {
                            if (!(size % 1000000)) data.add(size / 1000000, 'M');
                            else                   data.add(size / 1000, 'K');
                        }
                        data.add("/"sv);
                        if (!(total % 1000000)) data.add(total / 1000000, 'M');
                        else                    data.add(total / 1000, 'K');
                    }
                    //if (mxsz && step && size != mxsz) data.add("+", step);
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

            term& owner; // m_tracking: Terminal object reference.
            fp2d  coord; // m_tracking: Last coord of mouse cursor.
            subs  token; // m_tracking: Subscription token.
            prot  encod; // m_tracking: Mouse encoding protocol.
            mode  state; // m_tracking: Mouse reporting mode.
            si32  smode; // m_tracking: Selection mode state backup.

            m_tracking(term& owner)
                : owner{ owner                   },
                  encod{ prot::x11               },
                  state{ mode::none              },
                  smode{ owner.defcfg.def_selmod }
            { }

            operator bool () { return state != mode::none; }

            void check_focus(hids& gear) // Set keybd focus on any click if it is not set.
            {
                auto m = std::bitset<8>{ (ui32)gear.m_sys.buttons };
                auto s = std::bitset<8>{ (ui32)gear.m_sav.buttons };
                if (m[hids::buttons::right] && !s[hids::buttons::right])
                {
                    auto gear_test = owner.base::riseup(tier::request, e2::form::state::keybd::find, { gear.id, 0 });
                    if (gear_test.second == 0)
                    {
                        pro::focus::set(owner.This(), gear.id, solo::off);
                    }
                    owner.base::riseup(tier::preview, e2::form::layout::expose);
                }
                else if ((m[hids::buttons::left  ] && !s[hids::buttons::left  ])
                      || (m[hids::buttons::middle] && !s[hids::buttons::middle]))
                {
                    auto gear_test = owner.base::riseup(tier::request, e2::form::state::keybd::find, { gear.id, 0 });
                    if (gear_test.second == 0)
                    {
                        if (pro::focus::test(owner, gear)) pro::focus::off(owner.This(), gear.id);
                        else                               pro::focus::set(owner.This(), gear.id, gear.meta(hids::anyCtrl) ? solo::off : solo::on);
                    }
                    owner.base::riseup(tier::preview, e2::form::layout::expose);
                }
            }
            void enable(mode m)
            {
                state = (mode)(state | m);
                if (state && !token.size()) // Do not subscribe if it is already subscribed.
                {
                    owner.on(tier::mouserelease, input::key::MouseLeave, token, [&](hids& gear)
                    {
                        if (owner.selmod == mime::disabled)
                        {
                            coord = { fp32nan, fp32nan }; // Forward a mouse halt event.
                            owner.ipccon.mouse(gear, true, coord, encod, state);
                        }
                    });
                    owner.bell::dup_handler(tier::general, input::events::halt.id, token.back());
                    owner.LISTEN(tier::release, input::events::device::mouse::any, gear, token)
                    {
                        check_focus(gear);
                        if (owner.selmod == mime::disabled)
                        {
                            if (gear.captured(owner.id))
                            {
                                if (!gear.m_sys.buttons) gear.setfree();
                            }
                            else if (gear.m_sys.buttons) gear.capture(owner.id);
                            auto& console = *owner.target;
                            auto c = gear.m_sys.coordxy;
                            c.y -= console.get_basis();
                            auto moved = coord((state & mode::over) ? c
                                                                    : std::clamp(c, fp2d{ dot_00 }, fp2d{ console.panel - dot_11 }));
                            if (gear.m_sav.changed != gear.m_sys.changed)
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
                auto gates = owner.base::riseup(tier::request, e2::form::state::keybd::enlist); // Take all foci.
                for (auto gate_id : gates) // Reset double click state for all gears.
                {
                    if (auto gear_ptr = owner.base::getref<hids>(gate_id))
                    {
                        for (auto& [bttn_id, s] : gear_ptr->stamp) // Reset double click state. The issue is related to Far Manager, which changes the mouse tracking mode before releasing the button when double-clicking.
                        {
                            s.count = !!s.count; // Set to 1 if non zero.
                        }
                    }
                }
            }
            void setmode(prot p) { encod = p; }
        };

        // term: Keyboard focus tracking functionality.
        struct f_tracking
        {
            using prot = input::focus::prot;

            term& owner; // f_tracking: Terminal object reference.
            hook  token; // f_tracking: Subscription token.
            prot  encod; // f_tracking: Focus encoding mode.
            bool  state; // f_tracking: Current focus state.

            f_tracking(term& owner)
                : owner{ owner },
                  encod{ prot::w32 }
            {
                owner.LISTEN(tier::release, e2::form::state::focus::count, count, token)
                {
                    auto focused = !!count;
                    if (std::exchange(state, focused) != state)
                    {
                        owner.ipccon.focus(focused, encod);
                        if (!focused && owner.ime_on) owner.ime_on = faux;
                    }
                };
                state = owner.base::signal(tier::request, e2::form::state::focus::count);
            }

            operator bool () { return state; }
            void set(bool enable)
            {
                encod = enable ? prot::dec : prot::w32;
            }
        };

        // term: Terminal title tracking functionality.
        struct w_tracking
        {
            term&                owner; // w_tracking: Terminal object reference.
            std::map<text, text> props;
            std::map<text, txts> stack;
            escx                 queue;

            w_tracking(term& owner)
                : owner{ owner }
            { }
            // w_tracking: Get terminal window property.
            auto& get(text const& property)
            {
                auto& utf8 = props[property];
                if (property == ansi::osc_title)
                {
                    owner.base::riseup(tier::request, e2::form::prop::ui::header, utf8);
                }
                return utf8;
            }
            // w_tracking: Set terminal window property.
            void set(text const& property, qiew txt = {})
            {
                if (txt.empty()) txt = owner.appcfg.cmd; // Deny empty titles.
                owner.target->flush();
                if (property == ansi::osc_label_title)
                {
                                  props[ansi::osc_label] = txt;
                    auto& utf8 = (props[ansi::osc_title] = txt);
                    owner.base::riseup(tier::preview, e2::form::prop::ui::header, utf8);
                }
                else
                {
                    auto& utf8 = (props[property] = txt);
                    if (property == ansi::osc_title)
                    {
                        owner.base::riseup(tier::preview, e2::form::prop::ui::header, utf8);
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
                        // 61: VT Level 1 conformance
                        // 22: Color text
                        // 28: Rectangular area operations
                        // 52: Clipboard operations
                        // 10060: VT2D
                        queue.add("\x1b[?61;22;28;52;10060c");
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
            using pals = std::remove_const_t<decltype(argb::vt256)>;
            using func = utf::unordered_map<text, std::function<void(view)>>;

            enum class type { invalid, rgbcolor, request };

            term& owner; // c_tracking: Terminal object reference.
            pals  color; // c_tracking: 16/256 colors palette.
            func  procs; // c_tracking: Handlers.
            escx  reply; // c_tracking: Reply buffer.

            void reset()
            {
                std::copy(std::begin(owner.defcfg.def_colors), std::end(owner.defcfg.def_colors), std::begin(color));
            }
            auto to_byte(char c)
            {
                     if (c >= '0' && c <= '9') return c - '0';
                else if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                else if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                else                           return 0;
            }
            auto record(view& data) -> std::pair<type, ui32> // ; rgb:.../.../...
            {
                utf::trim_front(data, " ;");
                if (data.size() && data.front() == '?') // If a "?" is given rather than a name or RGB specification, termimal should reply with a control sequence of the same form.
                {
                    data.remove_prefix(1);
                    return { type::request, 0 };
                }
                else if (data.starts_with("rgb:"))
                {
                    auto get_color = [&](auto n)
                    {
                        auto r1 = to_byte(data[4]);
                        auto r2 = to_byte(data[5]);
                        auto g1 = to_byte(data[4 + n + 1]);
                        auto g2 = to_byte(data[5 + n + 1]);
                        auto b1 = to_byte(data[4 + (n + 1) * 2]);
                        auto b2 = to_byte(data[5 + (n + 1) * 2]);
                        data.remove_prefix(n * 3 + 4/*rgb:*/ + 2/*//*/);
                        return (b1 << 4 ) + (b2      )
                             + (g1 << 12) + (g2 << 8 )
                             + (r1 << 20) + (r2 << 16)
                             + 0xFF000000;
                    };
                    if (data.length() >= 12 && data[6] == '/' && data[9] == '/') // ; rgb:00/00/00
                    {
                        return { type::rgbcolor, get_color(2) };
                    }
                    else if (data.length() >= 15 && data[7] == '/' && data[11] == '/') // ; rgb:000/000/000
                    {
                        return { type::rgbcolor, get_color(3) };
                    }
                    else if (data.length() >= 18 && data[8] == '/' && data[13] == '/') // ; rgb:0000/0000/0000
                    {
                        return { type::rgbcolor, get_color(4) };
                    }
                }
                else if (data.starts_with("0x")) // ; 0xbbggrr
                {
                    if (data.length() >= 8)
                    {
                        auto b1 = to_byte(data[2]);
                        auto b2 = to_byte(data[3]);
                        auto g1 = to_byte(data[4]);
                        auto g2 = to_byte(data[5]);
                        auto r1 = to_byte(data[6]);
                        auto r2 = to_byte(data[7]);
                        data.remove_prefix(8); // sizeof 0xbbggrr
                        auto c = (b1 << 4 ) + (b2      )
                               + (g1 << 12) + (g2 << 8 )
                               + (r1 << 20) + (r2 << 16)
                               + 0xFF000000;
                        return { type::rgbcolor, c };
                    }
                }
                else if (data.starts_with("#")) // ; #rrggbb
                {
                    if (data.length() >= 7)
                    {
                        auto r1 = to_byte(data[1]);
                        auto r2 = to_byte(data[2]);
                        auto g1 = to_byte(data[3]);
                        auto g2 = to_byte(data[4]);
                        auto b1 = to_byte(data[5]);
                        auto b2 = to_byte(data[6]);
                        data.remove_prefix(7); // sizeof #rrggbb
                        auto c = (b1 << 4 ) + (b2      )
                               + (g1 << 12) + (g2 << 8 )
                               + (r1 << 20) + (r2 << 16)
                               + 0xFF000000;
                        return { type::rgbcolor, c };
                    }
                }
                else // Lookup custom color names stored in settings.xml.
                {
                    auto shadow = data;
                    auto color_name = utf::take_front<faux>(shadow, ";");
                    utf::trim(color_name);
                    auto name_str = utf::name2token(color_name);
                    auto iter = owner.defcfg.color_names.find(name_str);
                    if (iter != owner.defcfg.color_names.end())
                    {
                        data = shadow;
                        return std::pair{ type::rgbcolor, iter->second };
                    }
                }
                return { type::invalid, 0 };
            }
            void notsupported(text const& property, view full_data, view unkn_data)
            {
                log("%%Not supported: OSC=%property%\n\tDATA=%data%\n\tSIZE=%length%\n\tHEX=%hexdata%\n\tUNKN=%%", prompt::term, property, full_data, full_data.length(), utf::buffer_to_hex(full_data), ansi::err(unkn_data));
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
                        color[n] = (b1 << 4 ) + (b2      )
                                 + (g1 << 12) + (g2 << 8 )
                                 + (r1 << 20) + (r2 << 16)
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
                            color[n] = argb::vt256[n];
                            empty = faux;
                        }
                    }
                    if (empty) reset();
                };
                procs[ansi::osc_set_palette] = [&](view data) // ESC ] 4 ; 0;rgb:00/00/00;1;rgb:00/00/00;...
                {
                    auto fails = faux;
                    auto full_data = data;
                    while (data.length())
                    {
                        utf::trim_front(data, " ;");
                        if (data.empty()) break;
                        if (auto v = utf::to_int(data))
                        {
                            auto n = std::clamp(v.value(), 0, 255);
                            auto [t, r] = record(data);
                            if (t == type::request)
                            {
                                auto c = argb{ color[n] };
                                reply.osc(ansi::osc_set_palette, utf::fprint("%n%;rgb:%r%/%g%/%b%", n, utf::to_hex(c.chan.r),
                                                                                                       utf::to_hex(c.chan.g),
                                                                                                       utf::to_hex(c.chan.b)));
                            }
                            else if (t == type::rgbcolor)
                            {
                                color[n] = r;
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
                    if (fails) notsupported(ansi::osc_set_palette, full_data, data);
                };
                procs[ansi::osc_linux_reset] = [&](view /*data*/) // ESC ] R
                {
                    reset();
                };
                procs[ansi::osc_set_fgcolor] = [&](view data) // ESC ] 10 ;rgb:00/00/00
                {
                    auto full_data = data;
                    auto [t, r] = record(data);
                    if (t == type::request)
                    {
                        auto c = owner.target->brush.sfg();
                        reply.osc(ansi::osc_set_fgcolor, utf::fprint("rgb:%r%/%g%/%b%", utf::to_hex(c.chan.r),
                                                                                        utf::to_hex(c.chan.g),
                                                                                        utf::to_hex(c.chan.b)));
                    }
                    else if (t == type::rgbcolor)
                    {
                        owner.target->brush.sfg(r);
                    }
                    else notsupported(ansi::osc_set_fgcolor, full_data, data);
                };
                procs[ansi::osc_set_bgcolor] = [&](view data) // ESC ] 11 ;rgb:00/00/00
                {
                    auto full_data = data;
                    auto [t, r] = record(data);
                    if (t == type::request)
                    {
                        auto c = owner.target->brush.sbg();
                        reply.osc(ansi::osc_set_bgcolor, utf::fprint("rgb:%r%/%g%/%b%", utf::to_hex(c.chan.r),
                                                                                        utf::to_hex(c.chan.g),
                                                                                        utf::to_hex(c.chan.b)));
                    }
                    else if (t == type::rgbcolor)
                    {
                        owner.target->brush.sbg(r);
                    }
                    else notsupported(ansi::osc_set_bgcolor, full_data, data);
                };
                procs[ansi::osc_caret_color] = [&](view data) // ESC ] 12 ;rgb:00/00/00
                {
                    auto full_data = data;
                    auto [t, r] = record(data);
                    if (t == type::request)
                    {
                        auto c = owner.caret.bgc();
                        reply.osc(ansi::osc_caret_color, utf::fprint("rgb:%r%/%g%/%b%", utf::to_hex(c.chan.r),
                                                                                        utf::to_hex(c.chan.g),
                                                                                        utf::to_hex(c.chan.b)));
                    }
                    else if (t == type::rgbcolor)
                    {
                        owner.caret.bgc(r);
                    }
                    else notsupported(ansi::osc_caret_color, full_data, data);
                };
                procs[ansi::osc_reset_crclr] = [&](view /*data*/)
                {
                    owner.caret.color(owner.defcfg.def_curclr);
                };
                procs[ansi::osc_reset_fgclr] = [&](view /*data*/)
                {
                    owner.target->brush.sfg(0);
                };
                procs[ansi::osc_reset_bgclr] = [&](view /*data*/ )
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
                    if (reply.size())
                    {
                        owner.answer(reply);
                        reply.clear();
                    }
                }
                else log("%%Not supported: OSC=%property% DATA=%data% HEX=%hexdata%", prompt::term, property, data, utf::buffer_to_hex(data));
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
                #define V []([[maybe_unused]] auto& q, [[maybe_unused]] auto& p)
                auto& parser = ansi::get_parser<bufferbase>();
                autocr ? parser.intro[ansi::ctrl::eol] = V{ p->cr(); p->lf(q.pop_all(ansi::ctrl::eol)); }
                       : parser.intro[ansi::ctrl::eol] = V{          p->lf(q.pop_all(ansi::ctrl::eol)); };
                #undef V
            }
            template<class T>
            static void parser_config(T& vt)
            {
                using namespace netxs::ansi;
                #define V []([[maybe_unused]] auto& q, [[maybe_unused]] auto& p)
                vt.csier.table_space[csi_spc_src] = V{ p->na("CSI n SP A  Shift right n columns(s)"); }; // CSI n SP A  Shift right n columns(s).
                vt.csier.table_space[csi_spc_slc] = V{ p->na("CSI n SP @  Shift left  n columns(s)"); }; // CSI n SP @  Shift left n columns(s).
                vt.csier.table_space[csi_spc_cst] = V{ p->owner.caret.decscusr(q(1)); }; // CSI n SP q  Set cursor style (DECSCUSR).
                vt.csier.table_hash [csi_hsh_scp] = V{ p->na("CSI n # P  Push current palette colors onto stack, n default is 0"); }; // CSI n # P  Push current palette colors onto stack. n default is 0.
                vt.csier.table_hash [csi_hsh_rcp] = V{ p->na("CSI n # Q  Pop  current palette colors onto stack, n default is 0"); }; // CSI n # Q  Pop  current palette colors onto stack. n default is 0.
                vt.csier.table_hash [csi_hsh_psh] = V{ p->pushsgr(); }; // CSI # {  Push current SGR attributes onto stack.
                vt.csier.table_hash [csi_hsh_pop] = V{ p->popsgr();  }; // CSI # }  Pop  current SGR attributes from stack.
                vt.csier.table_excl [csi_exl_rst] = V{ p->owner.decstr( ); }; // CSI ! p  Soft terminal reset (DECSTR).

                vt.csier.table_dollarsn[csi_dlr_fra] = V{ p->fra(q); }; // CSI Char ; Top ; Left ; Bottom ; Right $ x  — Fill rectangular area (DECFRA).
                vt.csier.table_dollarsn[csi_dlr_cra] = V{ p->owner.deccra(q); }; // CSI srcTop ; srcLeft ; srcBottom ; srcRight ; srcBuffIndex ; dstTop ; dstLeft ; dstBuffIndex $ v  — Copy rectangular area (DECCRA). BuffIndex: 1..6, 1 is default index. All coords are 1-based.

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

                vt.csier.table[csi_cht]           = V{ p->tab( q(1)); }; // CSI n I  Cursor forward  n tabs, default n=1.
                vt.csier.table[csi_cbt]           = V{ p->tab(-q(1)); }; // CSI n Z  Cursor backward n tabs, default n=1.
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

                // Do not use non-standard vt.
                //vt.csier.table[csi_ccc][ccc_cup] = V{ p->cup0(q); }; // CCC_CUP
                //vt.csier.table[csi_ccc][ccc_chx] = V{ p->chx0(q.subarg(0)); }; // CCC_СHX
                //vt.csier.table[csi_ccc][ccc_chy] = V{ p->chy0(q.subarg(0)); }; // CCC_СHY
                vt.csier.table[csi_ccc][ccc_sbs] = V{ p->owner.sbsize(q); }; // CCC_SBS: Set scrollback size.
                vt.csier.table[csi_ccc][ccc_rst] = V{ p->owner.setdef();  }; // CCC_RST: Reset to defaults.
                vt.csier.table[csi_ccc][ccc_sgr] = V{ p->owner.setsgr(q); }; // CCC_SGR: Set default SGR.
                vt.csier.table[csi_ccc][ccc_lsr] = V{ p->owner.setlsr(q.subarg(1)); };           // CCC_LSR: Enable line style reporting.
                vt.csier.table[csi_ccc][ccc_sel] = V{ p->owner.selection_selmod(q.subarg(0)); }; // CCC_SEL: Set selection mode.
                vt.csier.table[csi_ccc][ccc_pad] = V{ p->setpad(q.subarg(-1)); };                // CCC_PAD: Set left/right padding for scrollback.

                vt.intro[ctrl::esc][esc_ind   ] = V{ p->lf(1); };          // ESC D  Index. Cursor down and scroll if needed (IND).
                vt.intro[ctrl::esc][esc_ir    ] = V{ p->ri();  };          // ESC M  Reverse index (RI).
                vt.intro[ctrl::esc][esc_sc    ] = V{ p->scp(); };          // ESC 7  (same as CSI s) Save cursor position.
                vt.intro[ctrl::esc][esc_rc    ] = V{ p->rcp(); };          // ESC 8  (same as CSI u) Restore cursor position.
                vt.intro[ctrl::esc][esc_ris   ] = V{ p->owner.decstr(); }; // ESC c  Reset to initial state (same as DECSTR).
                vt.intro[ctrl::esc][esc_nel   ] = V{ p->cr(); p->dn(1); }; // ESC E  Move cursor down and CR. Same as CSI 1 E
                vt.intro[ctrl::esc][esc_decdhl] = V{ p->dhl(q); };         // ESC # ...  ESC # 3, ESC # 4, ESC # 5, ESC # 6, ESC # 8

                vt.intro[ctrl::esc][esc_apc   ] = V{ p->apc(q); };          // ESC _ ... ST  APC.
                vt.intro[ctrl::esc][esc_dcs   ] = V{ p->msg(esc_dcs, q); }; // ESC P ... ST  DCS.
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
                vt.csier.table[dec_set] = V{ p->owner.modset(q); }; // ESC [ n h
                vt.csier.table[dec_rst] = V{ p->owner.modrst(q); }; // ESC [ n l

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
                vt.oscer[osc_caret_color] = V{ p->owner.ctrack.set(osc_caret_color, q); };
                vt.oscer[osc_reset_fgclr] = V{ p->owner.ctrack.set(osc_reset_fgclr, q); };
                vt.oscer[osc_reset_bgclr] = V{ p->owner.ctrack.set(osc_reset_bgclr, q); };
                vt.oscer[osc_reset_crclr] = V{ p->owner.ctrack.set(osc_reset_crclr, q); };
                vt.oscer[osc_clipboard  ] = V{ p->owner.forward_clipboard(q);           };
                vt.oscer[osc_term_notify] = V{ p->owner.osc_notify(q);                  };
                vt.oscer[osc_semantic_fx] = V{ p->owner.osc_marker(q);                  };
                #undef V

                // Log all unimplemented CSI commands.
                for (auto i = 0; i < 0x100; ++i)
                {
                    auto& proc = vt.csier.table[i];
                    if (!proc)
                    {
                        proc = [i](auto& q, auto& p){ p->not_implemented_CSI(i, q); };
                    }
                }
                // Log all unimplemented SGR attributes.
                auto& vt_csier_table_csi_sgr = vt.csier.table[csi_sgr];
                for (auto i = 0; i < (si32)vt_csier_table_csi_sgr.size(); ++i)
                {
                    auto& proc = vt_csier_table_csi_sgr[i];
                    if (!proc)
                    {
                        proc = [i](auto&, auto&){ log("%%SGR %val% attribute is not supported", prompt::term, i); };
                    }
                }
                auto& esc_lookup = vt.intro[ctrl::esc];
                // Log all unimplemented ESC+rest.
                for (auto i = 0; i < 0x100; ++i)
                {
                    auto& proc = esc_lookup[i];
                    if (!proc)
                    {
                        proc = [i](auto& q, auto& p){ p->not_implemented_ESC(i, q); };
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
                    : rich{ l       },
                     index{ l.index },
                     style{ l.style }
                { }
                line(id_t line_id, deco const& line_style, span dt, twod sz)
                    : rich{ dt, sz     },
                     index{ line_id    },
                     style{ line_style }
                { }
                line(id_t line_id, deco const& line_style, cell const& blank)
                    : rich{ blank      },
                     index{ line_id    },
                     style{ line_style }
                { }
                line(id_t line_id, deco const& line_style, cell const& blank, si32 length)
                    : rich{ blank, length },
                     index{ line_id       },
                     style{ line_style    }
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
                si32 height(si32 panel_x) const
                {
                    auto len = length();
                    assert(_kind == style.get_kind());
                    return len > panel_x && wrapped() ? (len + panel_x - 1) / panel_x
                                                      : 1;
                }
            };
            struct redo
            {
                using mark = ansi::mark;
                using sgrs = std::list<mark>;

                deco style{}; // Parser style state.
                mark brush{}; // Parser brush state.
                si32 decsg{}; // Parser DEC Special Graphcs Mode.
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
            bool  uirev; // bufferbase: Prev button highlighted.
            bool  uifwd; // bufferbase: Next button highlighted.
            ui64  alive; // bufferbase: Selection is active (digest).
            line  match; // bufferbase: Search pattern for highlighting.

            rich  tail_frag; // bufferbase: IRM cached fragment.
            rich  char_2d; // bufferbase: 2D char image.

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
            virtual void selection_bymark(twod coor)                      = 0;
            virtual void selection_selall()                               = 0;
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
            virtual void selection_setjet(bias /*align*/ = {})
            {
                // Do nothing by default.
            }
            virtual void selection_setwrp(wrap /*wrapping*/ = {})
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
                    auto nothing = match.each([](auto& c){ return !c.isspc(); });
                    if (nothing) match = {};
                }
                alive = datetime::uniqueid();
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
            void selection_selbox(bool new_state)
            {
                if (new_state) grant = true;
                if (grant) boxed = new_state;
            }
            // bufferbase: Return selection mode.
            bool selection_selbox() const
            {
                return boxed;
            }

            //virtual text get_current_line()                                             = 0;
            virtual cell cell_under_cursor()                                            = 0;
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
    virtual si32 get_origin(bool /*follow*/)
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
    virtual void setpad(si32 /*new_value*/)
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
    virtual void resize_viewport(twod new_sz, bool /*forced*/ = faux)
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
                top    = std::clamp(top,    0, panel.y);
                bottom = std::clamp(bottom, 0, panel.y);
                if (top != 0 && bottom != 0 && top >= bottom) // && top > bottom) top = bottom; //todo Nobody respects that.
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
    virtual twod get_coord(twod /*origin*/)
            {
                return coord;
            }
            // bufferbase: Base-CSI contract (see ansi::csi_t).
            //             task(...), meta(...), data(...)
            void task(ansi::rule property)
            {
                parser::flush();
                log("%%DirectVT extensions are not supported: arg=%%, cmd=%%", prompt::term, property.arg, property.cmd);
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
                    auto w = parser::style.wrp() == wrap::none ? (si32)owner.defcfg.def_wrpmod
                                                               : (si32)parser::style.wrp();
                    owner.base::signal(tier::release, terminal::events::layout::wrapln, w);
                    changed = true;
                }
                if (parser::style.jet() != old_style.jet())
                {
                    auto a = parser::style.jet() == bias::none ? (si32)bias::left
                                                               : (si32)parser::style.jet();
                    owner.base::signal(tier::release, terminal::events::layout::align, a);
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
                log("%%CSI %params% %char%, (%val%) is not implemented", prompt::term, params, (byte)i, i);
            }
            void not_implemented_ESC(si32 c, qiew& q)
            {
                switch (c)
                {
                    // Unexpected
                    case ansi::esc_csi   :
                    case ansi::esc_ocs   :
                    case ansi::esc_dcs   :
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
            void apc(qiew& q)
            {
                parser::flush();
                auto script_body = qiew{};
                auto head = q.begin();
                auto tail = q.end();
                while (head != tail)
                {
                    auto c = *head++;
                    if (c == ansi::c0_bel)
                    {
                        script_body = qiew{ q.begin(), std::prev(head) };
                        break;
                    }
                    else if (c == ansi::c0_esc && head != tail && *head == '\\')
                    {
                        script_body = qiew{ q.begin(), std::prev(head) };
                        head++;
                        break;
                    }
                }
                q = { head, tail };
                if (script_body)
                {
                    auto& luafx = owner.bell::indexer.luafx;
                    luafx.run_script(owner, script_body);
                }
            }
            void msg(si32 cmd, qiew& q)
            {
                parser::flush();
                auto data = text{};
                while (q)
                {
                    auto c = (char)q.front();
                    data.push_back(c);
                    q.pop_front();
                         if (c == ansi::c0_bel) break;
                    else if (c == ansi::c0_esc)
                    {
                        c = (char)q.front();
                        if (q && c == '\\')
                        {
                            data.push_back(c);
                            q.pop_front();
                            break;
                        }
                    }
                }
                log("%%Unsupported message/command: '\\e%char%%data%'", prompt::term, (char)cmd, utf::debase<faux>(data));
            }
            // bufferbase: Clear buffer.
    virtual void clear_all()
            {
                parser::state = {};
                parser::decsg = {};
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
                auto size = (si32)stops.size();
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
                    auto step = owner.defcfg.def_tablen;
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
                if (coord.x <= 0 || coord.x > owner.defcfg.def_mxline) return;
                resize_tabstops(coord.x);
                auto  coor = coord.x - 1;
                auto  head = stops.begin();
                auto  tail = stops.begin() + coor;
                auto& last = tail->first;
                if (coord.x != last)
                {
                    auto size = (si32)stops.size();
                    auto base = last;
                    last = coord.x;
                    while (head != tail)
                    {
                        auto& tab = (--tail)->first;
                        if (tab == base) tab = coord.x;
                        else             break;
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
                            auto& tab = head->second;
                            if (tab == prev) tab = coord.x;
                            else             break;
                        }
                    }
                }
            }
            // bufferbase: (see CSI 0 g) Remove tabstop at the current cursor posistion.
            void remove_tabstop()
            {
                auto  size = (si32)stops.size();
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
                    auto& tab = tail->first;
                    if (base == std::abs(tab)) tab = main;
                    else                       break;
                }

                tail = stops.end();
                base = stop.second;
                main = item.second;
                stop.second = main;
                while (++back != tail)
                {
                    auto& tab = back->second;
                    if (base == std::abs(tab)) tab = main;
                    else                       break;
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
                        coord.x += notab ? owner.defcfg.def_tablen
                                         : owner.defcfg.def_tablen - netxs::grid_mod(coord.x, owner.defcfg.def_tablen);
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
                        coord.x -= notab ? owner.defcfg.def_tablen
                                         :(owner.defcfg.def_tablen + coord.x - 1) % owner.defcfg.def_tablen + 1;
                    }
                }
            }
            // bufferbase: TAB  Horizontal tab.
    virtual void tab(si32 n)
            {
                parser::flush();
                auto size = (si32)stops.size();
                if (n > 0) while (n-- > 0) tab_impl<true>(size);
                else       while (n++ < 0) tab_impl<faux>(size);
            }
            void print_tabstops(text msg)
            {
                log(msg, ":\n", "index size = ", stops.size());
                auto i = 0u;
                auto data = utf::adjust("coor:", 5, " ", faux);
                while (i < stops.size()) data += utf::adjust(std::to_string(i++), 4, " ", true);
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
                    parser::assign(n, c);
                }
            }
            // bufferbase: CSI Char ; Top ; Left ; Bottom ; Right $ x  Fill rectangular area (DECFRA).
            void fra(fifo& q)
            {
                parser::flush();
                auto c = q(' ');
                auto t = q(1);
                auto l = q(1);
                auto b = q(panel.y);
                auto r = q(panel.x);
                if (t > b) t = b;
                if (l > r) l = r;
                l -= 1;
                t -= 1;
                auto area = rect{{ l, t }, { r - l, b - t }};
                area.trunc(panel);
                if (c < ' ') c = ' ';
                auto sym = utf::to_utf_from_code(c);
                auto tmp = parser::brush;
                parser::brush.txt(sym);
                auto [w, h, x, y] = parser::brush.whxy();
                if (w > 1)
                {
                    area.size.x /= w;
                }
                if (area)
                {
                    auto sav = coord;
                    while (area.size.y--)
                    {
                        set_coord(area.coor);
                        rep<faux>(area.size.x);
                        area.coor.y++;
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
                          .decsg = parser::decsg,
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
                parser::decsg = saved.decsg;
                parser::flush(); // Proceed new style.
            }
            // bufferbase: CSI n T/S  Scroll down/up, scrolled up lines are pushed to the scrollback buffer.
    virtual void scl(si32 n)
            {
                parser::flush();
                scroll_region(y_top, y_end, n, n > 0 ? faux : true);
            }
            // bufferbase: CSI n L  Insert n lines. Place cursor to the beginning of the current.
    virtual void il(si32 n)
            {
                parser::flush();
                // Works only if cursor is in the scroll region.
                // Inserts n lines at the current row and removes n lines at the scroll bottom.
                if (n > 0 && coord.y >= y_top
                          && coord.y <= y_end)
                {
                    scroll_region(coord.y, y_end, n, faux);
                    coord.x = 0;
                }
            }
            // bufferbase: CSI n M  Delete n lines. Place cursor to the beginning of the current.
    virtual void dl(si32 n)
            {
                parser::flush();
                // Works only if cursor is in the scroll region.
                // Deletes n lines at the current row and add n lines at the scroll bottom.
                if (n > 0 && coord.y >= y_top && coord.y <= y_end)
                {
                    scroll_region(coord.y, y_end, -n, faux);
                    coord.x = 0;
                }
            }
            // bufferbase: Reverse index with using scrollback.
    virtual void _ri(si32 n)
            {
                // Reverse index
                // - move cursor up if it is outside of scrolling region or below the top line of scrolling region.
                // - scroll down if cursor is on the top line of scroll region.
                auto new_coord_y = coord.y - n;
                if (new_coord_y < y_top && coord.y >= y_top)
                {
                    auto dy = y_top - new_coord_y;
                    scroll_region(y_top, y_end, dy, true);
                    coord.y = y_top;
                }
                else coord.y = std::clamp(new_coord_y, 0, panel.y - 1);
            }
            // bufferbase: ESC M  Reverse index.
    virtual void ri()
            {
                parser::flush();
                // Reverse index
                // - move cursor one line up if it is outside of scrolling region or below the top line of scrolling region.
                // - one line scroll down if cursor is on the top line of scroll region.
                if (coord.y != y_top)
                {
                    coord.y--;
                }
                else scroll_region(y_top, y_end, 1, faux); // vi does not expect scrollback data (use_scrollback = true).
            }
            // bufferbase: CSI t;b r  Set scrolling region (t/b: top+bottom).
            void scr(fifo& q)
            {
                auto top = q(0);
                auto end = q(0);
                set_scroll_region(top, end);
            }
            // bufferbase: CSI n @  ICH. Insert n blanks after cursor. Don't change cursor pos.
    virtual void ins(si32 n) = 0;
            // bufferbase: Shift left n columns(s).
            void shl(si32 n)
            {
                log("%%SHL(%n%) is not implemented", prompt::term, n);
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
            // bufferbase: Absolute horizontal cursor position (0-based).
    virtual void chx0(si32 n)
            {
                parser::flush();
                coord.x = n;
            }
            // bufferbase: CSI n G  Absolute horizontal cursor position (1-based).
    virtual void chx(si32 n)
            {
                parser::flush();
                coord.x = n - 1;
            }
            // bufferbase: Absolute vertical cursor position (0-based).
    virtual void chy0(si32 n)
            {
                parser::flush_data();
                if (decom) coord.y = std::clamp(n + y_top, y_top, y_end);
                else       coord.y = std::clamp(n, 0, panel.y - 1);
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
            // bufferbase: CSI y; x H/F  Cursor position (1-based).
    virtual void cup(twod p)
            {
                parser::flush_data();
                _cup(p - dot_11);
            }
            // bufferbase: Cursor position (0-based).
    virtual void cup0(twod p)
            {
                parser::flush_data();
                _cup(p);
            }
            // bufferbase: Cursor position (0-based) w/o data_flush.
    virtual void cup2(twod p)
            {
                _cup(p);
            }
            // bufferbase: CSI y; x H/F  Cursor position (1-based).
    virtual void cup(fifo& q)
            {
                auto y = q(1);
                auto x = q(1);
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
            // bufferbase: Move cursor back.
    virtual void _cub(si32 n)
            {
                coord.x -= n;
            }
            // bufferbase: Line feed. Index. Scroll region up if new_coord_y > end.
    virtual void _lf(si32 n)
            {
                auto new_coord_y = coord.y + n;
                if (new_coord_y > y_end && coord.y <= y_end)
                {
                    auto dy = y_end - new_coord_y;
                    scroll_region(y_top, y_end, dy, true);
                    coord.y = y_end;
                }
                else coord.y = std::clamp(new_coord_y, 0, panel.y - 1);
                if (coord.x < 0 || coord.x > panel.x) coord.x = 0; // Auto cr if the cursor is outside the viewport (to prevent infinitely long lines when autocr is disabled).
            }
            // bufferbase: Line feed. Index. Scroll region up if new_coord_y > end.
    virtual void lf(si32 n)
            {
                parser::flush_data();
                _lf(n);
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
                    case mime::ansitext: _shade(owner.defcfg.def_ansi_f, owner.defcfg.def_ansi_c, work); break;
                    case mime::richtext: _shade(owner.defcfg.def_rich_f, owner.defcfg.def_rich_c, work); break;
                    case mime::htmltext: _shade(owner.defcfg.def_html_f, owner.defcfg.def_html_c, work); break;
                    case mime::textonly: _shade(owner.defcfg.def_text_f, owner.defcfg.def_text_c, work); break;
                    case mime::safetext: _shade(owner.defcfg.def_safe_f, owner.defcfg.def_safe_c, work); break;
                    default:             _shade(owner.defcfg.def_none_f, owner.defcfg.def_none_c, work); break;
                }
            }
            // bufferbase: Rasterize selection with grips.
            void selection_raster(face& dest, auto curtop, auto curend, bool /*ontop*/ = true, bool /*onend*/ = true)
            {
                if (selection_active())
                {
                    auto mode = owner.selmod;
                    auto clip = dest.clip();
                    auto grip_1 = rect{ curtop, dot_11 };
                    auto grip_2 = rect{ curend, dot_11 };
                    grip_1.coor.x += clip.coor.x; // Compensate scrollback's hz movement.
                    grip_2.coor.x += clip.coor.x; //
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
                            west.coor.x += clip.coor.x; // Compensate scrollback's hz movement.
                            east.coor.x += clip.coor.x; //
                            west.trimby(clip);
                            east.trimby(clip);
                            dest.fill(west, fill);
                            dest.fill(east, fill);
                        }
                        square.trimby(clip);
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
                    auto length = std::max(1, canvas.size().x);
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
    virtual si32 selection_button(twod /*delta*/ = {})
            {
                auto forward_is_available = uifwd ? 1 << 0 : 0;
                auto reverse_is_available = uirev ? 1 << 1 : 0;
                return forward_is_available | reverse_is_available;
            }
            // bufferbase: Clear scrollback keeping current line.
    virtual void clear_scrollback() = 0;
            // bufferbase: Proceed 2d text.
            template<class Span>
            void data_2d(twod block_size, Span const& proto, auto print_stripe)
            {
                assert(block_size.y > 1);
                char_2d.unpack2d(proto, block_size);
                auto clip = char_2d.clip();
                auto size = char_2d.size();
                auto wrapln = parser::style.wrp() == wrap::on;
                auto is_first = coord.x == 0 || (coord.x >= panel.x && wrapln);
                auto print_stripes = [&]
                {
                    auto head = char_2d.begin() + clip.coor.y * size.x;
                    auto tail = head + clip.size.y * size.x;
                    auto rest = size.x - (clip.coor.x + clip.size.x);
                    while (true)
                    {
                        head += clip.coor.x;
                        auto next = head + clip.size.x;
                        auto line = std::span(head, next);
                        print_stripe(line);
                        head = next + rest;
                        if (head != tail)
                        {
                            _cub(clip.size.x);
                            _lf(1);
                        }
                        else break;
                    }
                };

                if (!is_first) // Go up to make room for 2D char.
                {
                    _ri(block_size.y - 1);
                }
                if (wrapln)
                {
                    auto left = clip;
                    while (left.size.x > 0)
                    {
                        clip.coor.x = left.coor.x;
                        clip.size.x = std::min(panel.x - coord.x % panel.x, left.size.x);
                        print_stripes();
                        left.coor.x += clip.size.x;
                        left.size.x -= clip.size.x;
                        if (left.size.x > 0)
                        {
                            _cub(coord.x);
                            _lf(1);
                        }
                    }
                }
                else
                {
                    print_stripes();
                }
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
            void resize_viewport(twod new_sz, bool /*forced*/ = faux) override
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
            bool recalc_pads(dent& new_oversz) override
            {
                auto left = 0;
                auto rght = 0;
                if (new_oversz.r != rght
                 || new_oversz.l != left)
                {
                    new_oversz.r = rght;
                    new_oversz.l = left;
                    return true;
                }
                else return faux;
            }
            static void _el(si32 n, core& canvas, twod coord, twod panel, cell const& blank)
            {
                assert(coord.y < panel.y);
                assert(coord.x >= 0);
                auto size = canvas.size();
                auto head = canvas.begin() + coord.y * size.x;
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
                _el(n, canvas, coord, panel, brush.spc());
            }
            // alt_screen: CSI n @  ICH. Insert n blanks after cursor. No wrap. Existing chars after cursor shifts to the right. Don't change cursor pos.
            void ins(si32 n) override
            {
                bufferbase::flush();
                assert(coord.y < panel.y);
                assert(coord.x >= 0);
                canvas.insert(coord, n, brush.spc());
            }
            // alt_screen: CSI n P  Delete (not Erase) letters under the cursor.
            void dch(si32 n) override
            {
                bufferbase::flush();
                canvas.cutoff(coord, n, brush.spc());
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
                canvas.backsp(coord, n, brush.spc());
                if (coord.y < 0) coord = dot_00;
            }
            // alt_screen: Move cursor by n in line.
            void move(si32 n) override
            {
                bufferbase::flush();
                coord.x += n;
                if      (coord.x < 0)       wrapup();
                else if (coord.x > panel.x) wrapdn();
                if      (coord.y < 0)        coord = dot_00;
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

                auto start = coord;
                coord.x += count;
                //todo apply line adjusting (necessity is not clear)
                if (coord.x <= panel.x)//todo styles! || ! curln.wrapped())
                {
                    auto n = std::min(count, panel.x - std::max(0, start.x));
                    canvas.splice<Copy>(start, n, proto, fuse);
                }
                else
                {
                    wrapdn();
                    if (start.y < y_top)
                    {
                        if (coord.y >= y_top)
                        {
                            auto n = coord.x + (coord.y - y_top) * panel.x;
                            count -= n;
                            set_coord({ 0, y_top });
                            _data<Copy>(n, proto, fuse); // Reversed fill using the last part of the proto.
                        }
                        auto data = proto.begin();
                        auto seek = start.x + start.y * panel.x;
                        auto dest = canvas.begin() + seek;
                        auto tail = dest + count;
                        rich::forward_fill_proc<Copy>(data, dest, tail, fuse);
                    }
                    else if (start.y <= y_end)
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

                        auto dest = canvas.begin() + seek;
                        auto tail = dest - count;
                        auto data = proto.end();
                        rich::reverse_fill_proc<Copy>(data, dest, tail, fuse);
                    }
                    else
                    {
                        if (coord.y >= panel.y) coord.y = panel.y - 1;

                        auto data = proto.begin();
                        auto size = count;
                        auto seek = start.x + start.y * panel.x;
                        auto dest = canvas.begin() + seek;
                        auto tail = canvas.end();
                        auto back = panel.x;
                        rich::unlimit_fill_proc<Copy>(data, size, dest, tail, back, fuse);
                    }
                }
            }
            // alt_screen: .
            auto& _fragment_from_current_coord(si32 left_cells)
            {
                canvas.copy_piece(tail_frag, coord.x + coord.y * panel.x, left_cells);
                return tail_frag;
            }
            // alt_screen: .
            template<bool Copy, class Span, class Shader>
            void _data_direct_fill(si32 count, Span const& proto, Shader fuse)
            {
                auto fill = [&](auto start_iter, auto seek)
                {
                    auto dest = start_iter + seek;
                    assert(count <= panel.x - coord.x);
                    auto tail = dest + count;
                    auto data = proto.begin();
                    rich::forward_fill_proc<Copy>(data, dest, tail, fuse);
                };
                fill(canvas.begin(), coord.x + coord.y * panel.x);
            }
            // alt_screen: Insert new text using the specified cell shader.
            template<class Span, class Shader>
            void _data_insert(si32 count, Span const& proto, Shader fuse)
            {
                auto next_x = coord.x + count;
                if (next_x < panel.x)
                {
                    auto left_cells = panel.x - next_x;
                    tail_frag = _fragment_from_current_coord(left_cells);
                    _data_direct_fill<faux>(count, proto, fuse);
                    coord.x = next_x;
                    if (tail_frag.size())
                    {
                        _data_direct_fill<true>(tail_frag.length(), tail_frag, fuse);
                    }
                }
                else
                {
                    _data(count, proto, fuse);
                }
            }
            // alt_screen: Parser callback.
            void data(si32 width, si32 height, core::body const& proto) override
            {
                if (width)
                {
                    if (height == 1)
                    {
                        owner.insmod ? _data_insert(width, proto, cell::shaders::skipnulls)
                                     : _data(width, proto, cell::shaders::skipnulls);
                    }
                    else // We do not support insmod for data_2d().
                    {
                        data_2d({ width, height }, proto, [&](auto& l){ _data<true>((si32)l.size(), l, cell::shaders::skipnulls); });
                    }
                }
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
                auto find = selection_active()
                         && match.length()
                         && owner.selmod == mime::textonly;
                canvas.move(full.coor - dest.coor());
                dest.plot(canvas, cell::shaders::flat);
                if (auto area = canvas.area())
                {
                    if (find)
                    {
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
                        _shade(owner.defcfg.def_find_f, owner.defcfg.def_find_c, work);
                    }
                    selection_render(dest);
                }
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
            void scroll_region(si32 top, si32 end, si32 n, [[maybe_unused]] bool use_scrollback = faux) override
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
                auto clip = dest.clip().trim(full);
                dest.clip(clip);
                dest.plot(canvas, cell::shaders::full);
            }
            // alt_screen: Return cell state under cursor.
            cell cell_under_cursor() override
            {
                auto coor = std::clamp(coord, dot_00, panel - dot_11);
                auto c = canvas[coor];
                return c;
            }
            // alt_screen: Clear scrollback keeping current line.
            void clear_scrollback() override
            {
                if (coord.y > 0 && panel.y > 1)
                {
                    scroll_region(0, panel.y - 1, -coord.y);
                }
                canvas.del_below({ 0, 1 }, brush.spare.dry());
                set_coord({ coord.x, 0 });
            }
            //text get_current_line() override
            //{
            //    auto crop = escx{};
            //    auto cy = std::clamp(coord.y, 0, panel.y - 1);
            //    auto p1 = twod{ 0, cy };
            //    auto p2 = twod{ panel.x, cy };
            //    auto stripe = canvas.line(p1, p2);
            //    auto brush_state = cell{};
            //    if (owner.selmod == mime::textonly
            //     || owner.selmod == mime::safetext
            //     || owner.selmod == mime::disabled)
            //    {
            //        utf::trim(crop.s11n<faux, true, faux>(stripe, brush_state));
            //    }
            //    else
            //    {
            //        crop.s11n<true, true, faux>(stripe, brush_state);
            //    }
            //    return crop;
            //}

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
                auto ok = selection_active();
                if (ok)
                {
                    auto limits = panel - dot_11;
                    selend = std::clamp(coor, dot_00, limits);
                    selection_selbox(mode);
                    selection_update();
                }
                return ok;
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
                        if ((coor.x > c.x) == (seltop.x > selend.x)) std::swap(seltop.x, selend.x);
                        if ((coor.y > c.y) == (seltop.y > selend.y)) std::swap(seltop.y, selend.y);
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
            // alt_screen: Select line.
            void selection_byline(twod coor) override
            {
                seltop.y = selend.y = coor.y;
                seltop.x = 0;
                selend.x = panel.x - 1;
                selection_locked(faux);
                selection_selbox(faux);
                selection_update(faux);
            }
            // alt_screen: Select all.
            void selection_selall() override
            {
                seltop.y = 0;
                seltop.x = 0;
                selend = panel - dot_11;
                selection_locked(faux);
                selection_selbox(true);
                selection_update(faux);
            }
            // alt_screen: Select lines between OSC marks.
            void selection_bymark(twod /*coor*/) override
            {
                selection_selall();
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
                return data;
            }
            // alt_screen: Highlight selection.
            void selection_render(face& dest) override
            {
                auto clip = dest.clip();
                auto limits = panel - dot_11;
                auto curtop = std::clamp(seltop, dot_00, limits) - twod{ clip.coor.x, 0 }; // Compensate scrollback's hz movement.
                auto curend = std::clamp(selend, dot_00, limits) - twod{ clip.coor.x, 0 }; //
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

                si32 caret{}; // buff: Current line cursor horizontal position.
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
                void dec_height(si32& block_vsize, type line_kind, si32 line_size)
                {
                    if (line_size > width && line_kind == type::autowrap) block_vsize -= (line_size + width - 1) / width;
                    else                                                  block_vsize -= 1;
                }
                // buff: Increase height.
                void add_height(si32& block_vsize, type line_kind, si32 line_size)
                {
                    if (line_size > width && line_kind == type::autowrap) block_vsize += (line_size + width - 1) / width;
                    else                                                  block_vsize += 1;
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
                        for (auto [line_size, pool] : sizes[kind])
                        {
                            assert(pool > 0);
                            vsize += pool;
                        }
                    }
                    auto kind = type::autowrap;
                    for (auto [line_size, pool] : sizes[kind])
                    {
                        if (line_size > width) vsize += pool * ((line_size + width - 1) / width);
                        else                   vsize += pool;
                    }
                }
                // buff: Register a new line.
                void invite(type& line_kind, si32& line_size, type new_kind, si32 new_size)
                {
                    ++sizes[new_kind][new_size];
                    add_height(vsize, new_kind, new_size);
                    line_size = new_size;
                    line_kind = new_kind;
                }
                // buff: Refresh scrollback height.
                void recalc(type& line_kind, si32& line_size, type new_kind, si32 new_size)
                {
                    if (line_size != new_size
                     || line_kind != new_kind)
                    {
                        undock(line_kind, line_size);
                        ++sizes[new_kind][new_size];
                        add_height(vsize, new_kind, new_size);
                        line_size = new_size;
                        line_kind = new_kind;
                    }
                }
                // buff: Discard the specified metrics.
                void undock(type line_kind, si32 line_size)
                {
                    auto& lens = sizes[line_kind];
                    auto  iter = lens.find(line_size); assert(iter != lens.end());
                    auto  pool = --(*iter).second;
                    if (pool == 0) lens.erase(iter);
                    dec_height(vsize, line_kind, line_size);
                }
                // buff: Check buffer size.
                bool check_size(twod new_size)
                {
                    auto old_value = vsize;
                    set_width(new_size.x);
                    if (ring::peak <= new_size.y)
                    {
                        static constexpr auto BottomAnchored = true;
                        ring::resize<BottomAnchored>(new_size.y);
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
                    auto line_kind = l._kind;
                    auto line_size = l._size;
                    undock(line_kind, line_size);
                    dec_height(basis, line_kind, line_size);
                    dec_height(slide, line_kind, line_size);
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
                auto index_by_id(ui32 item_id) const
                {
                    //No need to disturb distant objects, it may already be in the swap.
                    auto total = length();
                    return (si32)(total - 1 - (back().index - item_id)); // ring buffer size is never larger than max_int32.
                }
                // buff: Return an iterator pointing to the item with the specified id.
                auto iter_by_id(ui32 line_id) -> ring::iter<ring> //todo MSVC 17.7.0 requires return type
                {
                    return begin() + index_by_id(line_id);
                }
                // buff: Return the item reference using its id.
                auto& item_by_id(ui32 line_id)
                {
                    return ring::at(index_by_id(line_id));
                }
                // buff: Refresh metrics due to modified line.
                void recalc(line& l)
                {
                    recalc(l._kind, l._size, l.style.get_kind(), l.length());
                }
                // buff: Rewrite the indices from the specified position to the end or to the top (negative from).
                void reindex(si32 from)
                {
                    if (from >= 0)
                    {
                        auto a = begin() + from;
                        auto b = end();
                        auto i = from == 0 ? 0 : (a - 1)->index + 1;
                        while (a != b)
                        {
                            a->index = i++;
                            ++a;
                        }
                    }
                    else
                    {
                        auto a = begin();
                        auto b = a + std::abs(from);
                        auto i = b->index - std::abs(from);
                        while (a != b)
                        {
                            a->index = i++;
                            ++a;
                        }
                    }
                }
                // buff: Remove the specified number of lines at the specified position (inclusive).
                auto remove(si32 at, si32 amount)
                {
                    amount = ring::remove(at, amount);
                    reindex(at);
                    return amount;
                }
                // buff: Clear scrollback, add one empty line, and reset all metrics.
                void clear()
                {
                    auto auto_wrap = current().wrapped();
                    ring::clear();
                    caret = 0;
                    basis = 0;
                    slide = 0;
                    invite(0, deco{}.wrp(auto_wrap), cell{}); // At least one line must exist.
                    ancid = back().index;
                    ancdy = 0;
                    set_width(width);
                }
                // buff: Clear scrollback keeping current line.
                void clear_but_current()
                {
                    auto backup = current();
                    backup.index = 0;
                    ring::clear();
                    auto& curln = ring::push_back(backup); // Keep current line.
                    basis = 0;
                    slide = 0;
                    invite(curln); // Sync current line state (length, wrap mode).
                    ancid = curln.index;
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
                       batch{ boss.defcfg.def_length, boss.defcfg.def_growdt, boss.defcfg.def_growmx },
                       index{ 1    },
                       place{      },
                       shore{ boss.defcfg.def_margin }
            {
                parser::style.wrp(boss.defcfg.def_wrpmod);
                batch.invite(0, deco{}.wrp(boss.defcfg.def_wrpmod == wrap::on), cell{}); // At least one line must exist.
                batch.set_width(1);
                index_rebuild();

                auto c = cell{ '\0' }.fgc(boss.defcfg.def_fcolor).bgc(boss.defcfg.def_bcolor).link(boss.id);
                boss.defclr = c;
                parser::brush.reset(c);
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
                #if defined(DEBUG)
                    auto m = index.front().index;
                    for (auto& i : index)
                    {
                        assert(i.index >= m && i.index - m < 2);
                        m = i.index;
                    }
                #endif
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
                assert(coord.y < y_top || coord.y > y_end || (batch.caret - coord.x/*wrapped_block*/) % panel.x == 0);
                return true;
            }
            auto test_resize()
            {
                #if defined(DEBUG)
                    auto c = batch.caret;
                    sync_coord();
                    assert(c == batch.caret);
                #else
                    sync_coord();
                #endif
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
                if (new_value < 0) new_value = owner.defcfg.def_margin;
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
                        auto count1 = (si32)(under.index - batch.ancid);
                        auto count2 = (si32)(batch.ancid - front.index);
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
                            batch.ancid = front.index + (id_t)netxs::divround(batch.size * count1, count2);
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
                    auto range1 = (si32)(under.index - batch.ancid);
                    auto range2 = (si32)(batch.ancid - front.index);
                    batch.round = faux;
                    if (range1 < batch.size)
                    {
                        if (approx_threshold < std::min(range1, range2))
                        {
                            auto& mapln = index.front();
                            auto c1 = (ui64)(si32)(mapln.index - front.index);
                            auto c2 = (ui64)range2;
                            auto fresh_slide = (si32)netxs::divround(batch.vsize * c2, c1);
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
                upbox.crop(upnew, brush.dry());
                dnbox.crop(dnnew, brush.dry());

                index.resize(arena); // Use a fixed ring because new lines are added much more often than a futures feed.
                auto away = batch.basis != batch.slide;

                auto& curln = batch.current();
                if (curln.wrapped() && batch.caret > curln.length()) // Dangling cursor.
                {
                    curln.crop(batch.caret, brush.dry());
                    batch.recalc(curln);
                }

                if (!owner.bottom_anchored || in_top > 0 || in_end > 0) // The cursor is outside the scrolling region.
                {
                         if (in_top > 0) coord.y = std::max(0,           y_top - in_top);
                    else if (in_end > 0) coord.y = std::min(panel.y - 1, y_end + in_end);
                    coord = std::clamp(coord, dot_00, panel - dot_11);
                    if (owner.bottom_anchored)
                    {
                        batch.basis = std::max(0, batch.vsize - arena);
                    }
                    else // Try to keep batch.basis as is.
                    {
                        batch.basis = std::clamp(batch.basis, 0, std::max(0, batch.vsize - 1));
                    }
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
                    auto& line = *--head;
                    auto lineid = line.index;
                    auto length = line.length();
                    auto active = lnid == lineid;
                    if (line.wrapped())
                    {
                        auto offset = length;
                        auto remain = length ? (length - 1) % panel.x + 1
                                             : 0;
                        do
                        {
                            offset -= remain;
                            push(lineid, offset, remain);
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
                        push(lineid, 0, length);
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

                    auto& line = *++curit;
                    width = line.length();
                    wraps = line.wrapped();
                    curid = line.index;
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
                    auto& line = *++curit;
                    width = line.length();
                    wraps = line.wrapped();
                    curid = line.index;
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
                auto  drops = index.size - y_pos - 1;
                while (drops-- > 0) index.pop_back();

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
            bool recalc_pads(dent& oversz_ref) override
            {
                auto coor = get_coord();
                auto rght = std::max({0, batch.max<line::type::leftside>() - panel.x, coor.x - panel.x + 1 }); // Take into account the cursor position.
                auto left = std::max( 0, batch.max<line::type::rghtside>() - panel.x);
                auto cntr = std::max( 0, batch.max<line::type::centered>() - panel.x);
                auto bttm = std::max( 0, batch.vsize - batch.basis - arena          );
                auto both = cntr >> 1;
                left = shore + std::max(left, both + (cntr & 1));
                rght = shore + std::max(rght, both);
                if (oversz_ref.r != rght
                 || oversz_ref.l != left
                 || oversz_ref.b != bttm)
                {
                    oversz_ref.r = rght;
                    oversz_ref.l = left;
                    oversz_ref.b = bttm;
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

                if (coord.y >= y_top && coord.y <= y_end)
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

            void  cup(fifo& q) override { bufferbase:: cup(q); sync_coord<faux>(); }
            void  cup(twod  p) override { bufferbase:: cup(p); sync_coord<faux>(); }
            void cup0(twod  p) override { bufferbase::cup0(p); sync_coord<faux>(); }
            void cup2(twod  p) override { bufferbase::cup2(p); sync_coord<faux>(); }
            void  cuf(si32  n) override { bufferbase:: cuf(n); sync_coord<faux>(); }
            void  cub(si32  n) override { bufferbase:: cub(n); sync_coord<faux>(); }
            void _cub(si32  n) override { bufferbase::_cub(n); batch.caret -= n;   }
            void  chx(si32  n) override { bufferbase:: chx(n); sync_coord<faux>(); }
            void chx0(si32  n) override { bufferbase::chx0(n); sync_coord<faux>(); }
            void  tab(si32  n) override { bufferbase:: tab(n); sync_coord<faux>(); }
            void  chy(si32  n) override { bufferbase:: chy(n); sync_coord(); }
            void chy0(si32  n) override { bufferbase::chy0(n); sync_coord(); }
            void  scl(si32  n) override { bufferbase:: scl(n); sync_coord(); }
            void   il(si32  n) override { bufferbase::  il(n); sync_coord(); }
            void   dl(si32  n) override { bufferbase::  dl(n); sync_coord(); }
            void   up(si32  n) override { bufferbase::  up(n); sync_coord(); }
            void   dn(si32  n) override { bufferbase::  dn(n); sync_coord(); }
            void   lf(si32  n) override { bufferbase::  lf(n); sync_coord(); }
            void  _lf(si32  n) override { bufferbase:: _lf(n); sync_coord(); }
            void  _ri(si32  n) override { bufferbase:: _ri(n); sync_coord(); }
            void   ri()        override { bufferbase::  ri();  sync_coord(); }
            void   cr()        override { bufferbase::  cr();  sync_coord(); }

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
                    auto size = (si32)(upto - from);
                    auto tail = head + size;
                    auto area = block.area();
                    block.full(area);
                    block.clip(area);
                    block.ac(origin);
                    do
                    {
                        auto& curln = *head;
                        block.output(curln, cell::shaders::fuse);
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
                            while (count-- > 0) batch.invite(++curid, parser::style, parser::brush);
                            start = batch.size;
                        }
                    }
                    else
                    {
                        dissect(0);
                        start = batch.index_by_id(index.front().index);
                    }

                    auto curit = block.begin();
                    auto width = twod{ size.x, 1 };
                    auto curid = start == 0 ? batch.front().index
                                            : batch[start - 1].index + 1;
                    auto style = ansi::def_style;
                    style.wrp(wrap::off);
                    while (size.y-- > 0)
                    {
                        auto oldsz = batch.size;
                        auto proto = core::span{ curit, (size_t)size.x };
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
                    if (batch.size == 0) batch.invite(0, parser::style, parser::brush);
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
                auto line_id = batch.back().index;
                while (amount-- > 0)
                {
                    batch.invite(++line_id, parser::style, parser::brush);
                    index.push_back(line_id, 0, 0);
                }
            }
            // scroll_buf: Push filled lines to the scrollback bottom.
            void add_lines(si32 amount, cell const& blank)
            {
                assert(amount >= 0);
                auto line_id = batch.back().index;
                while (amount-- > 0)
                {
                    batch.invite(++line_id, parser::style, blank, panel.x);
                    index.push_back(line_id, 0, panel.x);
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
                        wraps = curln.wrapped();
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
                auto blank = brush.spc();
                if (auto ctx = get_context(coord))
                {
                    auto  start = si32{};
                    auto  count = si32{};
                    auto cursor = std::max(0, batch.caret);
                    auto& curln = batch.current();
                    auto  width = curln.length();
                    auto  wraps = curln.wrapped();
                    switch (n)
                    {
                        default:
                        case commands::erase::line::right: // n = 0 (default)  Erase to Right.
                            start = cursor;
                            count = wraps ? coord.x == panel.x ? 0 : panel.x - (cursor + panel.x) % panel.x
                                          : std::max(0, std::max(panel.x, width) - cursor);
                            break;
                        case commands::erase::line::left: // n = 1  Erase to Left.
                            start = wraps ? cursor - cursor % panel.x
                                          : 0;
                            count = cursor - start + 1; // +1 to include the current cell.
                            break;
                        case commands::erase::line::all: // n = 2  Erase All.
                            start = wraps ? cursor - cursor % panel.x
                                          : 0;
                            count = wraps ? panel.x
                                          : std::max(panel.x, batch->length());
                            break;
                        case commands::erase::line::wraps: // n = 3  Erase wrapped line.
                            start = cursor;
                            count = width - cursor;
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
                auto blank = brush.spc();
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
                auto blank = brush.spc();
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
                coord.x += count;
                if (count > 0)
                {
                    if (coord.y < y_top)
                    {
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
                                    if (batch.basis < 0) // Scroll down by pushing -batch.basis empty lines to front.
                                    {
                                        auto n = -batch.basis;
                                        while (n-- > 0) batch.insert(0, id_t{}, parser::style, parser::brush);
                                        batch.reindex(batch.basis); // Reindex backward.
                                        batch.basis = 0;
                                    }
                                    coord.y = y_top;
                                    index_rebuild();
                                }
                            }
                        }
                    }
                    else
                    {
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
                    curln.splice<faux>(batch.caret, n, brush.spc());
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
                    if (line.wrapped()) curln.splice(coor, line                   , cell::shaders::full, brush.spc());
                    else                curln.splice(coor, line.substr(0, panel.x), cell::shaders::full, brush.spc());
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
                    auto start = coord;
                    coord.x += count;
                    //todo apply line adjusting (necessity is not clear)
                    if (coord.x <= panel.x)//todo styles! || ! curln.wrapped())
                    {
                        auto n = std::min(count, panel.x - std::max(0, start.x));
                        upbox.splice<Copy>(start, n, proto, fuse);
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
                        auto seek = start.x + start.y * panel.x;
                        auto dest = upbox.begin() + seek;
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
                        curln.splice<Copy>(start, count, proto, fuse, brush.spc());
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
                        auto cur_y = coord.y + batch.basis;
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
                            curln.resize(batch.caret, brush.spc());
                            batch.recalc(curln);
                            if (auto n = (si32)(batch.back().index - curid))
                            {
                                if constexpr (mixer) _merge(curln, oldsz, curid, n);
                                assert(n > 0);
                                while (n-- > 0) batch.pop_back();
                            }

                            auto w = curln.length();
                            auto a = panel.x;
                            auto b = w - a;

                            cur_y -= batch.basis;
                            if (cur_y > 0)
                            {
                                auto n = index.size - cur_y - 1;
                                while (n-- > 0) index.pop_back();
                                auto& mapln = index.back();
                                mapln.width = panel.x;
                                a += mapln.start;
                            }
                            else // cur_y has scrolled out.
                            {
                                index.clear();
                                a *= std::abs(cur_y);
                            }

                            while (a < b)
                            {
                                index.push_back(curid, a, panel.x);
                                a += panel.x;
                            }
                            index.push_back(curid, a, w - a);

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
                                curln.resize(batch.caret, brush.spc());
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

                                if constexpr (mixer) curln.resize(batch.caret +shadow.length(), brush.spc());
                                else                 curln.splice(batch.caret, shadow, cell::shaders::full, brush.spc());

                                batch.recalc(curln);
                                auto w = curln.length();
                                auto spoil = (si32)(mapln.index - curid);
                                assert(spoil > 0);

                                if constexpr (mixer) _merge(curln, oldsz, curid, spoil);

                                auto after = batch.index() + 1;
                                spoil = batch.remove(after, spoil);

                                if (cur_y < batch.basis)
                                {
                                    index_rebuild(); // Update index. (processing lines larger than viewport)
                                }
                                else
                                {
                                    cur_y -= batch.basis;
                                    auto idx_a = index.begin() + cur_y;
                                    auto idx_b = index.end();
                                    auto a = idx_a->start;
                                    auto b = w - panel.x;
                                    while (idx_a != idx_b && a < b) // Update for current line.
                                    {
                                        auto& i =*idx_a;
                                        i.index = curid;
                                        i.start = a;
                                        i.width = panel.x;
                                        a += panel.x;
                                        ++idx_a;
                                    }
                                    if (idx_a != idx_b)
                                    {
                                        auto& i = *idx_a;
                                        i.index = curid;
                                        i.start = a;
                                        i.width = w - a;
                                        ++idx_a;
                                        while (idx_a != idx_b) // Update the rest.
                                        {
                                            auto& j = *idx_a;
                                            j.index -= spoil;
                                            ++idx_a;
                                        }
                                    }
                                    assert(test_index());
                                }
                                assert(test_futures());
                            } // case 2 done.
                        }
                        batch.current().splice<Copy>(start, count, proto, fuse, brush.spc());
                    }
                    assert(coord.y >= 0 && coord.y < arena);
                    coord.y += y_top;
                }
                else
                {
                    coord.y -= y_end + 1;
                    auto start = coord;
                    coord.x += count;
                    //todo apply line adjusting
                    if (coord.x <= panel.x)//todo styles! || ! curln.wrapped())
                    {
                        auto n = std::min(count, panel.x - std::max(0, start.x));
                        dnbox.splice<Copy>(start, n, proto, fuse);
                    }
                    else
                    {
                        wrapdn();
                        auto data = proto.begin();
                        auto size = count;
                        auto seek = start.x + start.y * panel.x;
                        auto dest = dnbox.begin() + seek;
                        auto tail = dnbox.end();
                        auto back = panel.x;
                        rich::unlimit_fill_proc<Copy>(data, size, dest, tail, back, cell::shaders::full);
                    }
                    coord.y = std::min(coord.y + y_end + 1, panel.y - 1);
                    // Note: coord can be unsync due to scroll regions.
                }
                assert(test_coord());
            }
            // scroll_buf: .
            template<bool Copy, class Span, class Shader>
            void _data_direct_fill(si32 count, Span const& proto, Shader fuse)
            {
                auto fill = [&](auto start_iter, auto seek)
                {
                    auto dest = start_iter + seek;
                    assert(count <= panel.x - coord.x);
                    auto tail = dest + count;
                    auto data = proto.begin();
                    rich::forward_fill_proc<Copy>(data, dest, tail, fuse);
                };
                if (coord.y < y_top)
                {
                    fill(upbox.begin(), coord.x + coord.y * panel.x);
                }
                else if (coord.y <= y_end)
                {
                    auto& curln = batch.current();
                    auto  start = batch.caret;
                    auto newlen = batch.caret + count;
                    if (newlen > curln.length())
                    {
                        curln.crop(newlen, brush.spc());
                        auto& mapln = index[coord.y - y_top];
                        mapln.width = newlen % panel.x;
                        batch.recalc(curln);
                    }
                    fill(curln.begin(), start);
                }
                else
                {
                    fill(dnbox.begin(), coord.x + (coord.y - (y_end + 1)) * panel.x);
                }
            }
            // scroll_buf: .
            auto& _fragment_from_current_coord(si32 left_cells)
            {
                if (coord.y < y_top)
                {
                    upbox.copy_piece(tail_frag, coord.x + coord.y * panel.x, left_cells);
                }
                else if (coord.y <= y_end)
                {
                    auto& curln = batch.current();
                    auto  start = batch.caret;
                    curln.copy_piece(tail_frag, start, left_cells);
                }
                else
                {
                    dnbox.copy_piece(tail_frag, coord.x + (coord.y - (y_end + 1)) * panel.x, left_cells);
                }
                return tail_frag;
            }
            // scroll_buf: Insert text using the specified cell shader.
            template<class Span, class Shader>
            void _data_insert(si32 count, Span const& proto, Shader fuse)
            {
                auto next_x = coord.x + count;
                if (next_x < panel.x)
                {
                    auto left_cells = panel.x - next_x;
                    tail_frag = _fragment_from_current_coord(left_cells);
                    _data_direct_fill<faux>(count, proto, fuse);
                    coord.x = next_x;
                    sync_coord();
                    if (tail_frag.size())
                    {
                        _data_direct_fill<true>(tail_frag.length(), tail_frag, fuse);
                    }
                }
                else
                {
                    _data(count, proto, fuse);
                }
            }
            // scroll_buf: Proceed new text (parser callback).
            void data(si32 width, si32 height, core::body const& proto) override
            {
                if (width)
                {
                    if (height == 1)
                    {
                        owner.insmod ? _data_insert(width, proto, cell::shaders::skipnulls)
                                     : _data(width, proto, cell::shaders::skipnulls);
                    }
                    else // We do not support insmod for data_2d().
                    {
                        data_2d({ width, height }, proto, [&](auto& l){ _data<true>((si32)l.size(), l, cell::shaders::skipnulls); });
                    }
                }
                else sync_coord();
            }
            // scroll_buf: Clear scrollback.
            void clear_all() override
            {
                batch.clear();
                reset_scroll_region();
                bufferbase::clear_all();
                resize_history(owner.defcfg.def_length, owner.defcfg.def_growdt, owner.defcfg.def_growmx);
            }
            // scroll_buf: Set scrollback limits.
            void resize_history(si32 new_size, si32 grow_by = 0, si32 grow_mx = 0)
            {
                static constexpr auto BOTTOM_ANCHORED = true;
                new_size = std::max(new_size, panel.y);
                termconfig::recalc_buffer_metrics(new_size, grow_by, grow_mx);
                batch.resize<BOTTOM_ANCHORED>(new_size, grow_by, grow_mx);
                index_rebuild();
            }
            // scroll_buf: Render to the canvas.
            void output(face& dest) override
            {
                dest.vsize(batch.vsize + sctop + scend); // Include margins and bottom oversize.
                auto clip = dest.clip();
                if (!clip) return;
                auto full = dest.full();
                auto coor = twod{ 0, batch.slide - batch.ancdy + y_top };
                auto head = batch.iter_by_id(batch.ancid);
                auto tail = batch.end();
                auto find = selection_active() && match.length() && owner.selmod == mime::textonly;
                auto clip2 = clip;
                clip2.coor.y += sctop;
                clip2.size.y = std::max(0, clip2.size.y - sctop - scend);
                auto stop = clip2.coor.y + clip2.size.y;
                dest.clip(clip2);
                auto fill = [&](auto& area, auto chr)
                {
                    if (auto r = clip2.trim(area))
                    {
                        dest.fill(r, [&](auto& c){ c.txt(chr).fgc(tint::greenlt); });
                    }
                };
                auto left_edge = clip.coor.x;
                auto rght_edge = clip.coor.x + clip.size.x;
                auto half_size = full.size.x / 2;
                auto left_rect = rect{{ left_edge, full.coor.y + coor.y }, dot_11 };
                auto rght_rect = left_rect;
                rght_rect.coor.x += clip.size.x - 1;
                while (head != tail && rght_rect.coor.y < stop)
                {
                    auto& curln = *head;
                    auto height = curln.height(panel.x);
                    auto length = curln.length();
                    auto adjust = curln.style.jet();
                    dest.output(curln, coor, cell::shaders::flat);
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
                        _shade(owner.defcfg.def_find_f, owner.defcfg.def_find_c, work);
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
                            auto rt_dot = lt_dot + clip.size.x;
                            auto remain = (length - 1) % clip.size.x + 1;
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
                dest.clip(clip);

                if (panel.y != arena) // The scrolling region is set.
                {
                    auto destcoor = clip.coor - dest.coor();
                    auto top_coor = twod{ 0, y_top - sctop } + destcoor;
                    auto end_coor = twod{ 0, y_end + 1     } + destcoor;
                    upbox.move(top_coor);
                    dnbox.move(end_coor);
                    dest.plot(upbox, cell::shaders::xlucent(owner.defcfg.def_lucent));
                    dest.plot(dnbox, cell::shaders::xlucent(owner.defcfg.def_lucent));
                    if (find)
                    {
                        auto draw = [&](auto const& block)
                        {
                            if (auto area = block.area())
                            {
                                auto block_clip = clip;
                                block_clip.size.x = area.size.x; // Follow wrapping for matches.
                                dest.full(block_clip);
                                area.coor -= destcoor;
                                auto offset = si32{};
                                auto marker = [&](auto shader)
                                {
                                    while (block.find(match, offset))
                                    {
                                        auto c = block.toxy(offset) + area.coor;
                                        dest.output(match, c, shader);
                                        offset += match.length();
                                    }
                                };
                                _shade(owner.defcfg.def_find_f, owner.defcfg.def_find_c, marker);
                            }
                        };
                        draw(upbox);
                        draw(dnbox);
                        dest.full(full);
                    }
                }

                selection_render(dest);
            }
            // scroll_buf: Remove all lines below (including futures) except the current. "ED2 Erase viewport" keeps empty lines.
            void del_below() override
            {
                assert(test_futures());

                auto blank = brush.dry().link(parser::brush.link());
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
                        curln.trimto(start, brush.spc());
                    }
                    else
                    {
                        auto& mapln = index[coor.y];
                        if (fills)
                        {
                            mapln.width = panel.x;
                            auto x = std::min(coor.x, panel.x); // Trim unwrapped lines by viewport.
                            curln.splice<true>(start + x, panel.x - x, blank);
                            curln.trimto(start + panel.x, brush.spc());
                        }
                        else
                        {
                            mapln.width = coor.x;
                            curln.trimto(start + coor.x, brush.spc());
                        }
                        assert(mapln.start == 0 || curln.wrapped());
                    }
                    batch.recalc(curln);

                    sync_coord();

                    assert(batch.vsize - batch.basis - index.size == 0); // stash
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
                    auto curit = batch.ring::insert(after + 1, tmpln.index, tmpln.style, parser::brush);
                    auto endit = batch.end();

                    auto& newln = *curit;
                    newln.splice(0, tmpln.substr(start), cell::shaders::full, brush.spc());
                    batch.undock_base_back(tmpln);
                    batch.invite(newln);

                    if (curit != batch.begin())
                    {
                        auto& curln = *(curit - 1);
                        curln = std::move(tmpln);
                        curln.trimto(start, brush.spc());
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
            // scroll_buf: Scroll the specified region by n lines (basis - n). The scrollback can only be used with the whole scrolling region.
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
                    if (top == y_top && end == y_end && use_scrollback)
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
                        auto range = (si32)(mdlid - topid);
                        auto floor = batch.index_by_id(endid) - range;
                        batch.remove(start, range);

                        // Insert block.
                        while (count-- > 0) batch.insert(floor, id_t{}, parser::style, parser::brush);

                        batch.reindex(start); //todo revise ? The index may be outdated due to the ring.
                        index_rebuild();
                    }
                }
                else // Scroll text down.
                {
                    if (top == y_top && end == y_end && use_scrollback && batch.basis >= n) // Just move the viewport up.
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
                        auto range = (si32)(endid - mdlid);
                        auto floor = batch.index_by_id(endid) - range;
                        batch.remove(floor, range);

                        // Insert block.
                        while (count-- > 0) batch.insert(start, id_t{}, parser::style, parser::brush);

                        batch.reindex(start); //todo revise ? The index may be outdated due to the ring.
                    }
                    index_rebuild();
                }

                assert(test_futures());
                assert(test_coord());
            }
            // scroll_buf: Return cell state under cursor.
            cell cell_under_cursor() override
            {
                auto& curln = batch.current();
                auto c = curln.length() && batch.caret <= curln.length() ? curln.at(std::clamp(batch.caret, 0, curln.length() - 1)) : parser::brush;
                return c;
            }
            // scroll_buf: Clear scrollback keeping current line.
            void clear_scrollback() override
            {
                batch.clear_but_current();
                resize_viewport(panel, true);
            }
            //text get_current_line() override
            //{
            //    auto crop = escx{};
            //    auto& stripe = batch.current();
            //    auto brush_state = cell{};
            //    if (owner.selmod == mime::textonly
            //     || owner.selmod == mime::safetext
            //     || owner.selmod == mime::disabled)
            //    {
            //        utf::trim(crop.s11n<faux, true, faux>(stripe, brush_state));
            //    }
            //    else
            //    {
            //        crop.s11n<true, true, faux>(stripe, brush_state);
            //    }
            //    return crop;
            //}

            // scroll_buf: Calc grip position by coor.
            auto selection_coor_to_grip(twod coor, grip::type role = grip::base)
            {
                auto link = batch.front().index;
                if (coor.y < 0)
                {
                    return grip{ .link = link,
                                 .coor = coor,
                                 .role = role };
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
                    if ((coor.y >= vtpos && coor.y < newpos) || ++start == limit)
                    {
                        link = curln.index;
                        break;
                    }
                    vtpos = newpos;
                }
                coor.y -= vtpos;
                return grip{ .link = link,
                             .coor = coor,
                             .role = role };
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
                                auto& line = *head;
                                auto line_height = line.height(panel.x);
                                if (ypos < line_height || ++head == limit)
                                {
                                    grip.link = line.index;
                                    grip.coor.y = ypos;
                                    break;
                                }
                                ypos -= line_height;
                            }
                        }
                        coor.y = vtpos + grip.coor.y;
                    }
                };
                // Check the buffer ring.
                if      (i_top < 0)           upmid.link = topid;
                else if (i_top >= batch.size) upmid.link = endid;
                if      (i_end < 0)           dnmid.link = topid;
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

                //auto square = rect{ -owner.base::oversz.corner(), owner.base::size() + owner.base::oversz };
                auto square = rect{ .size = owner.base::size() } + owner.base::oversz;
                auto minlim = square.coor;
                auto maxlim = minlim + std::max(dot_00, square.size - dot_11);
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
            bool selection_extend(twod coor, bool mode) override
            {
                auto x2 = coor.x;
                auto ok = selection_active();
                if (ok)
                {
                    selection_selbox(mode);
                    auto scrolling_margin = batch.slide + y_top;
                    auto edge1 = twod{ dot_mx.x, y_top - 1 };
                    auto edge2 = twod{-dot_mx.x, scrolling_margin };
                    auto edge3 = twod{ dot_mx.x, scrolling_margin + arena - 1 };
                    auto edge4 = twod{-dot_mx.x, 0 };
                    auto set_grip_coor_and_role = [](twod c, grip::type r)
                    {
                        return grip{ .coor = c, .role = r };
                    };
                    if (coor.y < scrolling_margin) // Hit the top margin.
                    {
                        coor -= {-owner.origin.x, batch.slide };
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
                        dntop.coor = coor; dntop.role = grip::base;
                        place = part::top;
                    }
                    else if (coor.y < scrolling_margin + arena) // Hit the scrolling region.
                    {
                        if (place == part::mid)
                        {
                            dnmid = selection_coor_to_grip(coor, grip::base);
                        }
                        else if (place == part::top)
                        {
                            if (uptop.role == grip::base)
                            {
                                dntop = set_grip_coor_and_role(edge1, grip::join);
                                upmid = selection_coor_to_grip(edge2, grip::join);
                                dnmid = selection_coor_to_grip(coor , grip::base);
                                upend.role = dnend.role             = grip::idle;
                            }
                            else if (uptop.role == grip::join)
                            {
                                uptop.role = dntop.role            = grip::idle;
                                dnmid = selection_coor_to_grip(coor, grip::base);
                            }
                        }
                        else if (place == part::end)
                        {
                            if (upend.role == grip::base)
                            {
                                uptop.role = dntop.role             = grip::idle;
                                dnmid = selection_coor_to_grip(coor , grip::base);
                                upmid = selection_coor_to_grip(edge3, grip::join);
                                dnend = set_grip_coor_and_role(edge4, grip::join);
                            }
                            else if (upend.role == grip::join)
                            {
                                upend.role = dnend.role            = grip::idle;
                                dnmid = selection_coor_to_grip(coor, grip::base);
                            }
                        }
                        place = part::mid;
                    }
                    else // Hit the bottom margin.
                    {
                        coor -= {-owner.origin.x, scrolling_margin + arena };
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
                        dnend.coor = coor; dnend.role = grip::base;
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
                return ok;
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
                            swap = (coor.y < scrolling_margin) == order;
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
                            swap = (idcur > cy) == (idtop > idend);
                        }
                        else // idend == idtop
                        {
                            if (idend == idcur)
                            {
                                if (dnmid.coor.y != upmid.coor.y)
                                {
                                    auto cy = (dnmid.coor.y + upmid.coor.y) / 2;
                                    swap = (check.coor.y > cy) == (upmid.coor.y > dnmid.coor.y);
                                }
                                else
                                {
                                    swap = (upmid.coor.y == check.coor.y ? std::abs(dnmid.coor.x - check.coor.x) > std::abs(upmid.coor.x - check.coor.x)
                                                                         : std::abs(dnmid.coor.y - check.coor.y) > std::abs(upmid.coor.y - check.coor.y));
                                }
                            }
                            else swap = (idcur > idend) == (upmid.coor.y > dnmid.coor.y);
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
                        if ((x > c) == (upmid.coor.x > dnmid.coor.x))
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
            // scroll_buf: Select line.
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
            // scroll_buf: Select all (ignore non-scrolling regions).
            void selection_selall() override
            {
                place = scend ? part::end : part::mid; // Last active region.
                uptop.role = dntop.role = grip::idle;
                upmid.role = dnmid.role = grip::base;
                upend.role = dnend.role = grip::idle;
                if (y_top != 0)
                {
                    uptop.role = grip::base;
                    dntop.role = upmid.role = grip::join;
                    uptop.coor = { 0, sctop - y_top };
                    dntop.coor = { panel.x - 1, sctop - 1 };
                }
                auto dyend = (panel.y - 1) - y_end;
                if (dyend > 0)
                {
                    dnmid.role = upend.role = grip::join;
                    dnend.role = grip::base;
                    upend.coor = { 0, 0 };
                    dnend.coor = { panel.x - 1, dyend };
                }
                auto& topln = batch.front();
                auto& endln = batch.back();
                auto x = std::max(0, endln.length() - 1);
                upmid = { .link = topln.index, .coor = offset_to_screen(topln, 0), .role = upmid.role };
                dnmid = { .link = endln.index, .coor = offset_to_screen(endln, x), .role = dnmid.role };
                selection_locked(faux);
                selection_selbox(faux);
                selection_update(faux);
            }
            // scroll_buf: Select lines between OSC marks.
            void selection_bymark(twod coor) override
            {
                auto scrolling_margin = batch.slide + y_top;
                if (coor.y < scrolling_margin) // Inside the top margin.
                {
                    place = part::top;
                    upmid.role = dnmid.role = grip::idle;
                    upend.role = dnend.role = grip::idle;
                    uptop.role = grip::base;
                    uptop.coor = { 0, sctop - y_top };
                    dntop.coor = { panel.x - 1, sctop - 1 };
                }
                else if (coor.y < scrolling_margin + arena) // Inside the scrolling region.
                {
                    upmid = selection_coor_to_grip(coor, grip::base);
                    dnmid = upmid;
                    auto curit = batch.iter_by_id(upmid.link);
                    auto& line = *curit;
                    auto start = screen_to_offset(line, upmid.coor);
                    auto group = 0xFF & (line.empty() ? line.link() : line.at(start).link()); // The semantic marker is placed in the low byte of the identifier.
                    auto check = [&](auto& c){ return (c.link() & 0xFF) != group; };
                    if (!group) // Semantic markers are not used.
                    {
                        selection_selall();
                    }
                    else
                    {
                        place = part::mid;
                        auto offup = start;
                        auto offdn = start;
                        auto up_rc = line.seek<feed::rev>(offup, check);
                        auto dn_rc = line.seek<feed::fwd>(offdn, check);
                        if (up_rc) // We are inside the command line.
                        {
                            upmid.coor = offset_to_screen(line, offup);
                            dnmid.coor = offset_to_screen(line, offdn);
                        }
                        else // We are inside the output or prompt.
                        {
                            auto head = batch.begin();
                            auto tail = batch.end();
                            auto iter = curit;
                            upmid.link = batch.front().index;
                            while (head != iter)
                            {
                                auto& curln = *--iter;
                                auto found = curln.empty() ? (curln.link() & 0xFF) != group : !curln.each(check);
                                if (found)
                                {
                                    upmid.link = curln.index + 1;
                                    break;
                                }
                            }
                            if (!dn_rc)
                            {
                                auto& backln = batch.back();
                                dnmid.link = backln.index;
                                iter = curit;
                                while (tail != ++iter)
                                {
                                    auto& curln = *iter;
                                    auto found = curln.empty() ? (curln.link() & 0xFF) != group : !curln.each(check);
                                    if (found)
                                    {
                                        dnmid.link = curln.index - 1;
                                        break;
                                    }
                                }
                            }
                            auto& upline = batch.item_by_id(upmid.link);
                            auto& dnline = batch.item_by_id(dnmid.link);
                            upmid.coor = offset_to_screen(upline, 0);
                            dnmid.coor = offset_to_screen(dnline, dn_rc ? offdn : (dnline.empty() ? 0 : dnline.length() - 1));
                        }
                        uptop.role = dntop.role = grip::idle;
                        upend.role = dnend.role = grip::idle;
                    }
                }
                else // Inside the bottom margin.
                {
                    place = part::end;
                    upmid.role = dnmid.role = grip::idle;
                    uptop.role = dntop.role = grip::idle;
                    upend.role = grip::base;
                    upend.coor = { 0, 0 };
                    dnend.coor = { panel.x - 1, (panel.y - 1) - y_end };
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
                auto clip = dest.clip().trim(full);
                dest.clip(clip);
                auto vpos = clip.coor.y - y_top;
                if (vpos >= 0 && vpos < arena)
                {
                    auto& mapln = index[vpos];
                    auto  ancid = mapln.index;
                    auto  ancdy = mapln.start / panel.x;
                    auto  limit = clip.coor.y + clip.size.y;
                    auto head = batch.iter_by_id(ancid);
                    auto tail = batch.end();
                    auto coor = twod{ 0, clip.coor.y - ancdy };
                    while (head != tail && coor.y < limit)
                    {
                        auto& curln = *head++;
                        auto height = curln.height(panel.x);
                        dest.output(curln, coor);
                        coor.y += height;
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
                    auto clip = rect{ dot_00, area.size };
                    auto full = rect{ -area.coor, { panel.x, area.coor.y + area.size.y }};
                    dest.core::size(area.size, brush.dry());
                    dest.core::clip(clip);
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
                    auto accum = cell{};
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
                            block.s11n<faux, faux, faux>(curln, field, accum);
                            if (block.size() > 0) yield.add(block);
                            else                  yield.eol();
                        });
                    }
                    else
                    {
                        auto s = deco{};
                        build([&](auto& curln)
                        {
                            if (s != curln.style)
                            {
                                if (auto wrp = curln.style.wrp(); s.wrp() != wrp) yield.wrp(wrp);
                                if (auto jet = curln.style.jet(); s.jet() != jet) yield.jet(jet);
                                s = curln.style;
                            }
                            auto block = escx{};
                            block.s11n<true, faux, faux>(curln, field, accum);
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
                auto selbox = selection_selbox();
                if (!selection_active()) return yield;
                if (selmod != mime::textonly
                 && selmod != mime::safetext) yield.nil();
                auto len = yield.size();
                if (uptop.role != grip::idle)
                {
                    bufferbase::selection_pickup(yield, upbox, uptop.coor, dntop.coor, selmod, selbox);
                }
                if (upmid.role != grip::idle)
                {
                    if (std::exchange(len, yield.size()) != len) yield.eol();
                    scroll_buf::selection_pickup(yield, selmod);
                }
                if (upend.role != grip::idle)
                {
                    if (std::exchange(len, yield.size()) != len) yield.eol();
                    bufferbase::selection_pickup(yield, dnbox, upend.coor, dnend.coor, selmod, selbox);
                }
                if (selbox && std::exchange(len, yield.size()) != len) yield.eol();
                return yield;
            }
            // scroll_buf: Highlight selection.
            void selection_render(face& dest) override
            {
                if (selection_active())
                {
                    auto full = dest.full();
                    auto clip = dest.clip();
                    auto cntx = dest.change_basis(full);
                    auto mode = owner.selmod;
                    if (panel.y != arena) // Draw fixed regions.
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
                        auto dytop = y_top;
                        auto dyend = (panel.y - 1) - y_end;
                        if (dytop > 0) draw_area(uptop, dntop, batch.slide - (sctop - y_top));
                        if (dyend > 0) draw_area(upend, dnend, batch.slide + y_end + 1);
                    }
                    if (upmid.role == grip::idle) return;
                    auto scrolling_region = rect{{ -dot_mx.x / 2, batch.slide + y_top }, { dot_mx.x, arena }};
                    clip.trimby(scrolling_region);
                    //todo Clang 15 don't get it
                    //auto [curtop, curend] = selection_take_grips();
                    auto tempvr = selection_take_grips();
                    auto curtop = tempvr.first;
                    auto curend = tempvr.second;
                    auto grip_1 = rect{ curtop, dot_11 };
                    auto grip_2 = rect{ curend, dot_11 };
                    if (selection_selbox())
                    {
                        auto area = grip_1 | grip_2;
                        auto proc = [&](auto fx)
                        {
                            dest.fill(area.trim(clip), fx);
                        };
                        _shade_selection(mode, proc);
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
                            auto draw = [&](auto const& coord, auto const& subblock, auto /*isr_to_l*/)
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
                                        block.trimby(bound);
                                    }
                                    else if (coord.y == curend.y)
                                    {
                                        auto bound = rect{ curend, { -dot_mx.x, 1 }}.normalize();
                                        bound.size.x += 1;
                                        block.trimby(bound);
                                    }
                                    block.trimby(clip);
                                    dest.fill(block, fill);
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
                    status.coor.y = 1 + std::abs((si32)(dnmid.link - upmid.link));
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
            void selection_setjet(bias a = bias::none) override
            {
                //todo unify setwrp and setjet
                if (selection_active())
                {
                    if (upmid.role == grip::idle) return;
                    auto i_top = std::clamp(batch.index_by_id(upmid.link), 0, batch.size);
                    auto j = batch[i_top].style.jet();
                    auto align = a != bias::none ? a
                                                 : j == bias::left   ? bias::right
                                                 : j == bias::right  ? bias::center
                                                                     : bias::left;
                    selection_foreach([&](auto& curln)
                    {
                        curln.style.jet(align);
                        batch.recalc(curln);
                    });
                    if (a != bias::none) style.jet(a);
                    resize_viewport(panel, true); // Recalc batch.basis.
                }
                else
                {
                    auto j = style.jet();
                    style.jet(a != bias::none ? a
                                              : j == bias::left   ? bias::right
                                              : j == bias::right  ? bias::center
                                                                  : bias::left);
                }
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
                auto dir = (si32)(id2 - id1);
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
            twod selection_center(id_t line_id, twod coor)
            {
                auto base = selection_viewport_center();
                auto dist = selection_outrun(line_id, coor, batch.ancid, base);
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
                                auto& line = proc(head);
                                from = ahead ? 0 : line.length();
                                if (resx(line))
                                {
                                    delta.y += ahead ?-accum
                                                     : accum + line.height(panel.x);
                                    return true;
                                }
                                accum += line.height(panel.x);
                            }
                            return faux;
                        };
                        if (ahead)
                        {
                            uirev = faux;
                            done = find(batch.end() - 1, [](auto& head) -> auto& { return *++head; });
                            if (!done && sctop)
                            {
                                from = si32{ 0 };
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
                            done = find(batch.begin(), [](auto& head) -> auto& { return *--head; });
                            if (!done && scend)
                            {
                                from = upbox.size().x * upbox.size().y;
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
                    if (done)
                    {
                        if (ahead) uirev = true;
                        else       uifwd = true;
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

        std::array<face, 5> pocket; // term: Buffers for DECCRA.
        termconfig defcfg; // term: Terminal settings.
        scroll_buf normal; // term: Normal    screen buffer.
        alt_screen altbuf; // term: Alternate screen buffer.
        buffer_ptr target; // term: Current   screen buffer pointer.
        pro::caret& caret; // term: Text cursor controller.
        pro::timer& timer; // term: Linear animation controller.
        pro::robot& robot; // term: Linear animation controller.
        m_tracking mtrack; // term: VT-style mouse tracking object.
        f_tracking ftrack; // term: Keyboard focus tracking object.
        w_tracking wtrack; // term: Terminal title tracking object.
        c_tracking ctrack; // term: Custom terminal palette tracking object.
        term_state status; // term: Screen buffer status info.
        cell       defclr; // term: Default/current colors (SGR49/39).
        twod       origin; // term: Viewport position.
        twod       follow; // term: Viewport follows cursor (bool: X, Y).
        bool       insmod; // term: Insert/replace mode.
        bool       decckm; // term: Cursor keys Application(true)/ANSI(faux) mode.
        bool       bpmode; // term: Bracketed paste mode.
        bool       unsync; // term: Viewport is out of sync.
        bool       invert; // term: Inverted rendering (DECSCNM).
        bool       styled; // term: Line style reporting.
        bool       io_log; // term: Stdio logging.
        bool       selalt; // term: Selection form (rectangular/linear).
        flag       resume; // term: Restart scheduled.
        flag       forced; // term: Forced shutdown.
        si32       selmod; // term: Selection mode.
        si32       onesht; // term: Selection one-shot mode.
        si32       altscr; // term: Alternate scroll mode.
        prot       kbmode; // term: Keyboard input mode.
        escx       w32key; // term: win32-input-mode forward buffer.
        bool       ime_on; // term: IME composition is active.
        para       imebox; // term: IME composition preview render.
        text       imetxt; // term: IME composition preview source.
        flow       imefmt; // term: IME composition preview layout.
        eccc       appcfg; // term: Application startup inits.
        os::fdrw   fdlink; // term: Optional DirectVT uplink.
        hook       onerun; // term: One-shot token for restart session.
        bool       rawkbd; // term: Exclusive keyboard access.
        bool       bottom_anchored; // term: Anchor scrollback content when resizing (default is anchor at bottom).
        ui32       event_sources; // term: vt-input-mode event reporting bit-field.
        vtty       ipccon; // term: IPC connector. Should be destroyed first.

        // term: Place rectangle block to the scrollback buffer.
        template<class S, class P>
        auto write_block(S& scrollback, core const& block, twod coor, rect trim, P fuse)
        {
            auto size = block.size();
            auto clip = block.clip();
            auto dest = rect{ coor, clip.size };
            trim.trimby(dest);
            clip -= dest - trim;
            coor = trim.coor;
            auto head = block.begin() + clip.coor.y * size.x;
            auto tail = head + clip.size.y * size.x;
            auto rest = size.x - (clip.coor.x + clip.size.x);
            auto save = scrollback.coord;
            assert(rest >= 0);
            while (head != tail)
            {
                head += clip.coor.x;
                auto next = head + clip.size.x;
                auto line = std::span(head, next);
                scrollback.cup2(coor);
                scrollback.template _data<true>(clip.size.x, line, fuse);
                head = next + rest;
                coor.y++;
            }
            scrollback.cup2(save);
        }
        // term: CSI srcTop ; srcLeft ; srcBottom ; srcRight ; srcBuffIndex ; dstTop ; dstLeft ; dstBuffIndex $ v  — Copy rectangular area (DECCRA). BuffIndex: 1..6, 1 is default index. All coords are 1-based (inclusive).
        void deccra(fifo& q)
        {
            auto srcTop       = q(0);
            auto srcLeft      = q(0);
            auto srcBottom    = q(0);
            auto srcRight     = q(0);
            auto srcBuffIndex = std::clamp(q(0), 1, 6) - 2; // Pocket buffers are 2..6 (0..4). -1 it is a real buffer.
            auto dstTop       = q(0);
            auto dstLeft      = q(0);
            auto dstBuffIndex = std::clamp(q(0), 1, 6) - 2; // Pocket buffers are 2..6 (0..4). -1 it is a real buffer.
            auto fragment = face{};
            auto& console = *target;
            auto size = srcBuffIndex == -1 ? console.panel : pocket[srcBuffIndex].size();
            srcRight  += srcRight  ? 0 : size.x;
            srcBottom += srcBottom ? 0 : size.y;
            srcLeft   -= srcLeft   ? 1 : 0;
            srcTop    -= srcTop    ? 1 : 0;
            dstLeft   -= dstLeft   ? 1 : 0;
            dstTop    -= dstTop    ? 1 : 0;
            auto area = rect{{ srcLeft, srcTop }, { srcRight - srcLeft, srcBottom - srcTop }};
            auto src_area = rect{ dot_00, size };
            area.trimby(src_area);
            fragment.full(src_area);
            fragment.core::crop(area.size, defclr);
            fragment.core::area(area);
            // Take fragment.
            if (srcBuffIndex == -1) console.do_viewport_copy(fragment);
            else                    netxs::onbody(fragment, pocket[srcBuffIndex], cell::shaders::full);
            // Put fragment.
            auto coor = twod{ dstLeft, dstTop };
            if (dstBuffIndex == -1) // Dst is real buffer.
            {
                area.coor = {};
                fragment.area(area);
                if (target == &normal) write_block(normal, fragment, coor, src_area, cell::shaders::full);
                else
                {
                    auto& target_buffer = *(alt_screen*)target;
                    write_block(target_buffer, fragment, coor, src_area, cell::shaders::full);
                }
            }
            else
            {
                area.coor = coor;
                fragment.area(area);
                auto& dst = pocket[dstBuffIndex];
                dst.crop(console.panel, defclr);
                dst.plot(fragment, cell::shaders::full);
            }
        }
        // term: Set semantic marker (OSC 133).
        void osc_marker(view data)
        {
            auto type = data.size() ? data.front() : 0;
            auto& console = *target;
            auto new_id = type | (console.brush.link() & ~0xFF);
            console.brush.link(new_id);
            if (io_log) log("\tOSC %% semantic marker: %type%", ansi::osc_semantic_fx, type);
        }
        // term: Terminal notification (OSC 9).
        void osc_notify(view data)
        {
            auto delimpos = data.find(';');
            if (delimpos != text::npos)
            {
                static constexpr auto cwdsync = "9"sv;
                auto type = data.substr(0, delimpos++);
                if (io_log) log("\tOSC %%;%type% notify: ", ansi::osc_term_notify, type, ansi::hi(utf::debase<faux, faux>(data.substr(delimpos))));
                if (type == cwdsync)
                {
                    auto path = text{ data.substr(delimpos) };
                    base::enqueue([&, path](auto& /*boss*/) mutable
                    {
                        base::riseup(tier::preview, e2::form::prop::cwd, path);
                    });
                }
            }
        }
        // term: Forward clipboard data (OSC 52).
        void forward_clipboard(view data)
        {
            auto clipdata = input::clipdata{};
            auto delimpos = data.find(';');
            if (delimpos != text::npos)
            {
                clipdata.meta = data.substr(0, delimpos++);
                clipdata.utf8.clear();
                utf::unbase64(data.substr(delimpos), clipdata.utf8);
                clipdata.form = mime::disabled;
                clipdata.size = target->panel;
                clipdata.hash = datetime::now();
                auto gates = base::riseup(tier::request, e2::form::state::keybd::enlist); // Take all foci.
                for (auto gate_id : gates) // Signal them to set the clipboard data.
                {
                    if (auto gear_ptr = base::getref<hids>(gate_id))
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
            altscr = defcfg.def_alt_on;
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
                    caret.blink_period();
                    break;
                case 25:   // Cursor on.
                    caret.show();
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
                    altscr = defcfg.def_alt_on;
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
        void decset(fifo& q)
        {
            target->flush();
            while (auto next = q(0)) _decset(next);
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
        // term: Reset terminal parameters. (DECRST).
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
                    caret.blink_period(span::zero());
                    break;
                case 25:   // Cursor off.
                    caret.hide();
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
        void decrst(fifo& q)
        {
            target->flush();
            while (auto next = q(0)) _decrst(next);
        }
        // term: Set termnail parameters.
        void _modset(si32 n)
        {
            switch (n)
            {
                case 4:     // Insert/Replace Mode (IRM) on.
                    insmod = true;
                    break;
                case 20:    // LNM—Line Feed/New Line Mode on.
                    target->set_autocr(true);
                    break;
                default:
                    break;
            }
        }
        // term: Reset termnail parameters.
        void _modrst(si32 n)
        {
            switch (n)
            {
                case 4:     // Insert/Replace Mode (IRM) off.
                    insmod = faux;
                    break;
                case 20:    // LNM—Line Feed/New Line Mode off.
                    target->set_autocr(faux);
                    break;
                default:
                    break;
            }
        }
        // term: Set termnail parameters.
        void modset(fifo& q)
        {
            target->flush();
            while (auto next = q(0)) _modset(next);
        }
        // term: Reset termnail parameters.
        void modrst(fifo& q)
        {
            target->flush();
            while (auto next = q(0)) _modrst(next);
        }
        // term: Set scrollback buffer size and grow step.
        void sbsize(fifo& q)
        {
            target->flush();
            auto ring_size = q.subarg(defcfg.def_length);
            auto grow_step = q.subarg(defcfg.def_growdt);
            auto grow_mxsz = q.subarg(defcfg.def_growmx);
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
        void scroll(twod& origin_ref)
        {
            auto& console = *target;
            if (follow[axis::Y])
            {
                if (follow[axis::X])
                {
                    follow[axis::X] = faux;
                    auto pos = console.get_coord(dot_00).x;
                    origin_ref.x = bufferbase::reset_viewport(origin_ref.x, pos, console.panel.x);
                }
            }
            origin_ref.y = console.get_origin(follow[axis::Y]);
        }
        // term: Proceed terminal changes.
        template<class P>
        void update(P proc)
        {
            auto lock = bell::sync();
            if (defcfg.resetonout) follow[axis::Y] = true;
            if (follow[axis::Y])
            {
                unsync |= proc();
            }
            else
            {
                auto last_basis = target->get_basis();
                auto last_slide = target->get_slide();
                auto is_changed = proc();
                unsync |= is_changed;
                if (is_changed)
                {
                    auto next_basis = target->get_basis();
                    follow[axis::Y] = (last_basis <= last_slide && last_slide <= next_basis)
                                   || (next_basis <= last_slide && last_slide <= last_basis);
                }
            }
        }
        // term: Proceed terminal input.
        template<bool Forced = faux>
        auto ondata_direct(view data = {}, bufferbase* target_buffer = {})
        {
            auto& console_ptr = target_buffer ? target_buffer : target;
            if (data.size())
            {
                if (io_log) log(prompt::cout, "\n\t", utf::replace_all(ansi::hi(utf::debase(data)), "\n", ansi::pushsgr().nil().add("\n\t").popsgr()));
                ansi::parse(data, console_ptr);
                return true;
            }
            else
            {
                console_ptr->parser::flush(); // Update line style, etc.
                return Forced;
            }
        }
        // term: Proceed terminal input.
        template<bool Forced = faux>
        void ondata(view data = {}, bufferbase* target_buffer = {})
        {
            update([&]
            {
                return ondata_direct<Forced>(data, target_buffer);
            });
        }
        // term: Reset to defaults.
        void setdef()
        {
            auto& console = *target;
            defclr.txt('\0').fgc(defcfg.def_fcolor).bgc(defcfg.def_bcolor).link(base::id);
            console.brush.reset(defclr);
            console.style.reset();
            console.style.wrp(defcfg.def_wrpmod);
            console.setpad(defcfg.def_margin);
            selection_selmod(defcfg.def_selmod);
            caret.style(defcfg.def_cursor);
        }
        // term: Set terminal background.
        void setsgr(fifo& q)
        {
            struct marker
            {
                ansi::deco style;
                ansi::mark brush;
                void task(ansi::rule const& /*cmd*/) { }
            };
            static auto parser = ansi::csi_t<marker, true>{};

            auto mark = marker{};
            mark.brush = defclr;
            auto param = q.front(ansi::sgr_rst);
            if (q.issub(param))
            {
                auto ptr = &mark;
                q.settop(q.desub(param));
                parser.table[ansi::csi_sgr].execute(q, ptr);
            }
            else mark.brush = cell{ '\0' }.fgc(defcfg.def_fcolor).bgc(defcfg.def_bcolor);
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
            base::riseup(tier::preview, e2::form::upon::scroll::bystep::v, info);
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
            base::signal(tier::release, e2::form::draggable::left, selection_passed());
            base::signal(tier::release, terminal::events::selmod, selmod);
        }
        // term: Run one-shot selection.
        void selection_oneshot(si32 newmod)
        {
            onesht = newmod;
            base::signal(tier::release, terminal::events::onesht, onesht);
            selection_selmod(newmod);
        }
        // term: Set selection form.
        void selection_selalt(bool boxed)
        {
            selalt = boxed;
            base::signal(tier::release, e2::form::draggable::left, selection_passed());
            base::signal(tier::release, terminal::events::selalt, selalt);
        }
        // term: Set the next selection mode.
        void selection_selmod()
        {
            auto newmod = (selmod + 1) % mime::count;
            selection_selmod(newmod);
        }
        // term: Toggle selection form.
        void selection_toggle_selalt()
        {
            selection_selalt(!selalt);
            target->selection_locked(faux);
            target->selection_selbox(selalt);
            target->selection_update();
            base::deface();
        }
        auto selection_cancel()
        {
            auto active = target->selection_cancel();
            if (active)
            {
                auto& console = *target;
                timer.pacify();
                auto shore = console.getpad();
                auto delta = dot_00;
                     if (origin.x <= oversz.l && origin.x > oversz.l - shore) delta = {-1, oversz.l - shore };
                else if (origin.x >=-oversz.r && origin.x < shore - oversz.r) delta = { 1, shore - oversz.r };
                if (delta.x)
                {
                    auto limit = delta.y;
                    delta.y = 0;
                    timer.actify(commands::ui::center, 0ms, [&, delta, shore, limit](auto /*id*/) mutable // 0ms = current FPS ticks/sec.
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
            gear.owner.base::signal(tier::request, input::events::clipboard, gear);
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
        auto _paste(auto& data)
        {
            //todo pasting must be ready to be interruped by any pressed key (to interrupt a huge paste).
            follow[axis::X] = true;
            follow[axis::Y] = true;
            ipccon.paste(data, bpmode, kbmode);
        }
        auto paste(hids& gear)
        {
            auto data = get_clipboard_text(gear);
            if (data.size())
            {
                pro::focus::set(This(), gear.id, solo::off);
                _paste(data);
                return true;
            }
            return faux;
        }
        auto _copy(hids& gear, text const& data)
        {
            auto form = selmod == mime::disabled ? mime::textonly : selmod;
            pro::focus::set(This(), gear.id, solo::off);
            gear.set_clipboard(target->panel, data, form);
        }
        auto copy(hids& gear)
        {
            auto data = target->selection_pickup(selmod);
            if (data.size())
            {
                _copy(gear, data);
            }
            auto ctrl_pressed = gear.meta(hids::anyCtrl);
            if (onesht != mime::disabled && !ctrl_pressed)
            {
                selection_oneshot(mime::disabled);
            }
            if (ctrl_pressed || selection_cancel()) // Keep selection if Ctrl is pressed.
            {
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
            base::signal(tier::release, e2::render::any, canvas);
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
            auto gear_test = base::riseup(tier::request, e2::form::state::keybd::find, { gear.id, 0 });
            if (!gear_test.second) // Set exclusive focus on right click.
            {
                pro::focus::set(This(), gear.id, solo::on);
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
                utf8 = get_clipboard_text(gear);
            }
            if (utf8.size())
            {
                pro::focus::set(This(), gear.id, solo::off);
                follow[axis::X] = true;
                if (bpmode)
                {
                    auto temp = text{};
                    temp.reserve(utf8.size() + ansi::paste_begin.size() + ansi::paste_end.size());
                    temp += ansi::paste_begin;
                    temp += utf8;
                    temp += ansi::paste_end;
                    std::swap(utf8, temp);
                }
                data_out(utf8);
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
            }
            else selection_cancel();
        }
        void selection_dblclk(hids& gear)
        {
            target->selection_byword(gear.coord);
            gear.dismiss();
            base::deface();
        }
        void selection_tplclk(hids& gear)
        {
                 if (gear.clicked == 3) target->selection_byline(gear.coord);
            else if (gear.clicked == 4) target->selection_bymark(gear.coord);
            else if (gear.clicked == 5) target->selection_selall();
            gear.dismiss();
            base::deface();
        }
        void selection_create(hids& gear)
        {
            auto& console = *target;
            auto boxed = selalt ^ !!gear.meta(hids::anyAlt);
            auto go_on = gear.meta(hids::anyCtrl);
            console.selection_follow(gear.click, go_on);
            if (go_on) console.selection_extend(gear.click, boxed);
            else       console.selection_create(gear.click, boxed);
            base::deface();
        }
        void selection_moveto(twod delta)
        {
            if (delta)
            {
                auto path = delta;
                auto time = datetime::round<si32>(skin::globals().switching);
                auto init = 0;
                auto func = constlinearAtoB<twod>(path, time, init);
                robot.actify(func, [&](twod& step)
                {
                    scrollby(step);
                    base::deface();
                });
            }
            else timer.pacify();

        }
        void selection_extend(hids& gear)
        {
            // Check bounds and scroll if needed.
            auto& console = *target;
            auto boxed = selalt ^ !!gear.meta(hids::anyAlt);
            auto coord = twod{ gear.coord };
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
                timer.actify(0ms, [&, delta, coord, boxed](auto) mutable // 0ms = current FPS ticks/sec.
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
            else timer.pacify();

            if (console.selection_extend(coord, boxed))
            {
                base::deface();
            }
        }
        void selection_finish(hids& /*gear*/)
        {
            //todo option: copy on select
            //...
            timer.pacify();
            base::deface();
        }
        void selection_submit()
        {
            LISTEN(tier::release, e2::form::drag::start ::left, gear){ if (selection_passed()) selection_create(gear); };
            LISTEN(tier::release, e2::form::drag::pull  ::left, gear){ if (selection_passed()) selection_extend(gear); };
            LISTEN(tier::release, e2::form::drag::stop  ::left, gear){                         selection_finish(gear); };
            LISTEN(tier::release, e2::form::drag::cancel::left, gear){                         selection_cancel();     };
            on(tier::mouserelease, input::key::RightClick,                [&](hids& gear){                         selection_pickup(gear); });
            on(tier::mouserelease, input::key::LeftClick,                 [&](hids& gear){                         selection_lclick(gear); });
            on(tier::mouserelease, input::key::MiddleClick,               [&](hids& gear){                         selection_mclick(gear); });
            on(tier::mouserelease, input::key::LeftDoubleClick,           [&](hids& gear){ if (selection_passed()) selection_dblclk(gear); });
            on(tier::mouserelease, input::key::LeftMultiClick,            [&](hids& gear){ if (selection_passed()) selection_tplclk(gear); });
            on(tier::mouserelease, input::key::MouseWheel, [&](hids& gear)
            {
                if (gear.captured()) // Forward mouse wheel events to all parents. Wheeling while button pressed.
                {
                    auto& offset = base::coor();
                    if (auto parent_ptr = base::parent())
                    {
                        auto& parent = *parent_ptr;
                        gear.pass(tier::mouserelease, parent, offset);
                    }
                }
                else
                {
                    if (gear.meta(hids::anyCtrl)) return; // Ctrl+Wheel is reserved for zooming.
                    if (altscr && target != &normal)
                    {
                        if (gear.whlsi)
                        {
                            auto count = std::abs(gear.whlsi);
                            auto arrow = decckm ? gear.whlsi > 0 ? "\033OA"sv : "\033OB"sv
                                                : gear.whlsi > 0 ? "\033[A"sv : "\033[B"sv;
                            data_out(utf::repeat(arrow, count));
                        }
                        gear.dismiss();
                    }
                }
            });
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
                gear.owner.base::signal(tier::request, input::events::clipboard, gear);
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
            base::signal(tier::release, terminal::events::search::status, console.selection_button(delta));
            if (target == &normal && delta)
            {
                selection_moveto(delta);
            }
        }
        auto& get_color()
        {
            return defclr;
        }
        void set_color(cell brush)
        {
            auto& console = *target;
            brush.link(base::id);
            defclr = brush;
            brush.link(console.brush.link());
            console.brush.reset(brush);
        }
        void set_bg_color(argb bg)
        {
            auto& console = *target;
            auto brush = defclr;
            brush.bgc(bg);
            brush.link(base::id);
            defclr = brush;
            brush.link(console.brush.link());
            console.brush.reset(brush);
            base::signal(tier::release, terminal::events::colors::bg, bg);
        }
        void set_fg_color(argb fg)
        {
            auto& console = *target;
            auto brush = defclr;
            brush.fgc(fg);
            brush.link(base::id);
            defclr = brush;
            brush.link(console.brush.link());
            console.brush.reset(brush);
            base::signal(tier::release, terminal::events::colors::fg, fg);
        }
        void set_rawkbd(si32 state = {})
        {
            if (!state) rawkbd = !rawkbd;
            else        rawkbd = state - 1;
            base::signal(tier::release, terminal::events::rawkbd, rawkbd);
        }
        void set_wrapln(si32 wrapln = {})
        {
            target->selection_setwrp((wrap)wrapln);
            if (!target->selection_active())
            {
                follow[axis::Y] = true; // Reset viewport.
            }
            ondata<true>(); // Trigger to rise event.
        }
        void set_align(si32 align = {})
        {
            target->selection_setjet((bias)align);
            if (!target->selection_active())
            {
                follow[axis::Y] = true; // Reset viewport.
            }
            ondata<true>(); // Trigger to rise event.
        }
        void set_selmod(si32 mode)
        {
            selection_selmod(mode);
        }
        void set_oneshot(si32 mode)
        {
            selection_oneshot(mode);
        }
        void set_selalt(bool boxed)
        {
            selection_selalt(boxed);
        }
        void set_log(bool state)
        {
            if (defcfg.allow_logs)
            {
                io_log = state;
                base::signal(tier::release, terminal::events::io_log, state);
            }
        }
        void clear_scrollback()
        {
            target->clear_scrollback();
            ondata<true>();
        }
        void exec_cmd(commands::ui::commands cmd)
        {
            if constexpr (debugmode) log(prompt::term, "Command: ", cmd);
            auto& console = *target;
            switch (cmd)
            {
                case commands::ui::toggleraw:    set_rawkbd();                        break;
                case commands::ui::togglewrp:    console.selection_setwrp();          break;
                case commands::ui::togglejet:    console.selection_setjet();          break;
                case commands::ui::togglesel:    selection_selmod();                  break;
                case commands::ui::toggleselalt: selection_toggle_selalt();           break;
                case commands::ui::restart:      restart();                           break;
                case commands::ui::sighup:       close(true);                         break;
                case commands::ui::undo:         ipccon.undo(true);                   break;
                case commands::ui::redo:         ipccon.undo(faux);                   break;
                case commands::ui::deselect:     selection_cancel();                  break;
                default: break;
            }
            if (cmd != commands::ui::togglesel && !console.selection_active())
            {
                follow[axis::Y] = true; // Reset viewport.
            }
            ondata<true>();
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
        void onexit(si32 code, text msg = {}, bool exit_after_sighup = faux)
        {
            if (exit_after_sighup) close();
            else base::enqueue<faux>([&, code, msg, backup = This()](auto& /*boss*/) mutable
            {
                ipccon.payoff(io_log);
                auto lock = bell::sync();
                auto error = [&]
                {
                    auto byemsg = escx{};
                    if (target != &normal) byemsg.locate({ 0, target->panel.y - 1 });
                    byemsg.bgc(code ? argb{ reddk } : argb{}).fgc(whitelt).add(msg)
                          .add("\r\nProcess exited with code ", os::exitcode(code)).nil()
                          .add("\r\n\n");
                    return byemsg;
                };
                auto query = [&]
                {
                    auto byemsg = error().add("Press Esc to close or press Enter to restart the session.\r\n")
                                         .add("\n");
                    ondata(byemsg);
                    LISTEN(tier::release, input::events::keybd::post, gear, onerun)
                    {
                        if (gear.keystat)
                        {
                            switch (gear.keybd::generic())
                            {
                                //todo key
                                case key::Esc:      close(); onerun.reset(); break;
                                case key::KeyEnter: start(); onerun.reset(); break;
                            }
                        }
                    };
                    base::riseup(tier::release, e2::form::global::sysstart, 0);
                };
                auto renew = [&]
                {
                    auto byemsg = error().add("\n");
                    ondata(byemsg);
                    start();
                };
                     if (forced)                close();
                else if (resume.exchange(faux)) renew();
                else switch (defcfg.def_atexit)
                {
                    case commands::atexit::smart: code ? query()
                                                       : close(); break;
                    case commands::atexit::retry: code ? renew()
                                                       : close(); break;
                    case commands::atexit::ask:          query(); break;
                    case commands::atexit::close:        close(); break;
                    case commands::atexit::restart:      renew(); break;
                }
                backup.reset(); // Backup should dtored under the lock.
            });
        }
        void start()
        {
            base::riseup(tier::release, e2::form::upon::started, This());
        }
        void start(eccc cfg, os::fdrw fds = {})
        {
            appcfg = cfg;
            fdlink = fds;
            if (!ipccon)
            {
                base::enqueue([&, backup = This()](auto& /*boss*/) mutable // We can't request the title before conio.run(), so we queue the request.
                {
                    auto& title = wtrack.get(ansi::osc_title);
                    if (title.empty())
                    {
                        wtrack.set(ansi::osc_title); // Set default title if it is empty.
                    }
                    if (defcfg.send_input.size())
                    {
                        ipccon.write<faux>(defcfg.send_input);
                    }
                    backup.reset(); // Backup should dtored under the lock.
                });
                appcfg.win = target->panel;
                ipccon.runapp(*this, appcfg, fdlink);
            }
        }
        void restart()
        {
            resume.exchange(true);
            ipccon.sighup(faux);
        }
        void close(bool fast = true, bool notify = true)
        {
            if (notify)
            {
                base::signal(tier::request, e2::form::proceed::quit::one, fast);
            }
            forced = fast;
            if (ipccon)
            {
                if (ipccon.sighup())
                {
                    base::enqueue<faux>([&, backup = This()](auto& /*boss*/) mutable // This backup is to keep the task active.
                    {
                        ipccon.payoff(io_log); // Wait child process.
                        auto lock = bell::sync();
                        base::riseup(tier::release, e2::form::proceed::quit::one, forced);
                        backup.reset(); // Backup should dtored under the lock.
                    });
                }
            }
            else // Child process exited with non-zero code and term waits keypress.
            {
                auto lock = bell::sync();
                onerun.reset();
                base::enqueue([&, backup = This()](auto& /*boss*/) mutable // The termlink trailer (calling ui::term::close()) should be joined before ui::term dtor.
                {
                    base::riseup(tier::release, e2::form::proceed::quit::one, forced);
                    backup.reset(); // Backup should dtored under the lock.
                });
            }
        }
        // term: Resize terminal window.
        void window_resize(twod winsz)
        {
            base::riseup(tier::preview, e2::form::prop::window::size, winsz);
        }
        // term: Custom data output (ConSrv cooked read callback).
        template<class Fx>
        void data(rich& cooked, Fx fx)
        {
            if (auto width = cooked.length())
            {
                auto& proto = cooked.pick();
                auto& brush = target->parser::brush;
                cooked.each([&](cell& c){ c.meta(brush); });
                //todo split by char height and do _data2d(...) for each
                if (target == &normal) normal._data(width, proto, fx);
                else
                {
                    auto& target_buffer = *(alt_screen*)target;
                    target_buffer._data(width, proto, fx);
                }
            }
        }
        // term: Move composition cursor (imebox.caret) inside viewport with wordwrapping.
        auto calc_composition_cursor(twod viewport_square_size, twod viewport_cursor)
        {
            auto composit_cursor = viewport_cursor;
            auto step = 0;
            auto next = 0;
            auto test = [&](auto const& coord, auto const& subblock, auto /*isr_to_l*/)
            {
                next = step + subblock.length();
                if (imebox.caret >= step && imebox.caret <= next) // It could be evaluated twice if the cursor is wrapped.
                {
                    composit_cursor = coord;
                    composit_cursor.x += imebox.caret - step;
                }
                step = next;
            };
            imefmt.flow::reset();
            imefmt.flow::size({ viewport_square_size.x, viewport_square_size.y * 2 });
            imefmt.flow::ac(viewport_cursor);
            imefmt.flow::compose<faux>(imebox, test);
            return composit_cursor;
        }
        void key_event(hids& gear, bool forced_event = faux)
        {
            if (!forced_event && gear.touched && !rawkbd) return;
            switch (gear.payload)
            {
                case keybd::type::keypress:
                    if (defcfg.resetonkey && gear.doinput())
                    {
                        base::riseup(tier::release, e2::form::animate::reset, 0); // Reset scroll animation.
                        unsync = true;
                        follow[axis::X] = true;
                        follow[axis::Y] = true;
                    }
                    ipccon.keybd(gear, decckm, kbmode);
                    if (forced_event || !gear.touched || gear.keystat != input::key::released || rawkbd) gear.set_handled(faux);
                    break;
                case keybd::type::imeinput:
                case keybd::type::keypaste:
                    _paste(gear.cluster);
                    gear.dismiss();
                    break;
                case keybd::type::imeanons:
                    if (imetxt != gear.cluster)
                    {
                        imetxt = gear.cluster;
                        imebox.wipe();
                        ansi::parse(gear.cluster, &imebox);
                        ime_on = imebox.length();
                        if (ime_on)
                        {
                            imebox.style.wrp(wrap::on);
                            //if (imebox.locus.size())
                            auto iter = std::find_if(imebox.locus.begin(), imebox.locus.end(), [](auto cmd){ return cmd.cmd == ansi::fn::sc; });
                            if (iter != imebox.locus.end())
                            {
                                //auto [cmd, arg] = imebox.locus.back();
                                auto [cmd, arg] = *iter;
                                if (cmd == ansi::fn::sc) imebox.caret = arg;
                            }
                        }
                        if (io_log) log(prompt::key, "IME composition preview: ", ansi::hi(ansi::s11n(imebox.content(), rect{.size = imebox.size()})));
                        unsync = true;
                    }
                    else unsync = std::exchange(ime_on, imebox.length()) != ime_on;
                    break;
                case keybd::type::kblayout:
                    //todo
                    break;
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

    public:
        term()
            : defcfg{ bell::indexer.config },
              normal{ *this },
              altbuf{ *this },
              target{ &normal },
               caret{ base::plugin<pro::caret>(defcfg.def_cur_on, defcfg.def_cursor, dot_00, defcfg.def_period, defcfg.def_curclr) },
               timer{ base::plugin<pro::timer>() },
               robot{ base::plugin<pro::robot>() },
              mtrack{ *this },
              ftrack{ *this },
              wtrack{ *this },
              ctrack{ *this },
              follow{ 0, 1 },
              insmod{ faux },
              decckm{ faux },
              bpmode{ faux },
              unsync{ faux },
              invert{ faux },
              styled{ faux },
              io_log{ defcfg.def_io_log },
              selalt{ defcfg.def_selalt },
              resume{ faux },
              forced{ faux },
              selmod{ defcfg.def_selmod },
              onesht{ mime::disabled },
              altscr{ defcfg.def_alt_on },
              kbmode{ prot::vt },
              ime_on{ faux },
              rawkbd{ faux },
              bottom_anchored{ true },
              event_sources{}
        {
            set_fg_color(defcfg.def_fcolor);
            set_bg_color(defcfg.def_bcolor);
            selection_submit();
            selection_selmod(defcfg.def_selmod);

            auto& mouse = base::plugin<pro::mouse>();
            mouse.draggable<hids::buttons::left>(selection_passed());

            base::plugin<pro::keybd>();
            auto& luafx = bell::indexer.luafx;
            auto& config = bell::indexer.config;
            auto terminal_context = config.settings::push_context("/config/events/terminal/");
            auto script_list = config.settings::take_ptr_list_for_name("script");
            auto bindings = input::bindings::load(config, script_list);
            input::bindings::keybind(*this, bindings);
            base::add_methods(basename::terminal,
            {
                { methods::KeyEvent,                [&]
                                                    {
                                                        luafx.run_with_gear([&](auto& gear)
                                                        {
                                                            auto backup = syskeybd{};
                                                            backup.set(gear);
                                                            auto args_count = luafx.args_count();
                                                            auto index = 0;
                                                            while (index < args_count)
                                                            {
                                                                //log("key args:");
                                                                auto k = syskeybd{};
                                                                k.syncto(gear);
                                                                gear.ctlstat = backup.ctlstat;
                                                                luafx.read_args(++index, [&](qiew key, qiew val)
                                                                {
                                                                         if (key == "keystat") gear.keystat = xml::take_or(val, input::key::released);
                                                                    else if (key == "ctlstat") gear.ctlstat = xml::take_or(val, backup.ctlstat);
                                                                    else if (key == "virtcod") gear.virtcod = xml::take_or(val, 0);
                                                                    else if (key == "scancod") gear.scancod = xml::take_or(val, 0);
                                                                    else if (key == "keycode") gear.keycode = xml::take_or(val, 0);
                                                                    else if (key == "extflag") gear.extflag = xml::take_or(val, 0);
                                                                    else if (key == "cluster") gear.cluster = val;
                                                                    else log("%%Unknown key event parameters %%=%%", prompt::lua, key, utf::debase437(val));
                                                                    //log("  %%=%%", key, utf::debase437(val));
                                                                });
                                                                key_event(gear);
                                                            }
                                                            backup.syncto(gear);
                                                            gear.set_handled();
                                                        });
                                                    }},
                { methods::ExclusiveKeyboardMode,   [&]
                                                    {
                                                        luafx.run_with_gear_wo_return([&](auto& gear){ gear.set_handled(); });
                                                        auto args_count = luafx.args_count();
                                                        if (!args_count)
                                                        {
                                                            auto state = (si32)rawkbd;
                                                            luafx.set_return(state);
                                                        }
                                                        else
                                                        {
                                                            auto state = luafx.get_args_or(1, si32{ 0 });
                                                            set_rawkbd(1 + (si32)!!state);
                                                            luafx.set_return();
                                                        }
                                                    }},
                { methods::FindNextMatch,           [&]
                                                    {
                                                        luafx.run_with_gear([&](auto& gear)
                                                        {
                                                            auto dir = luafx.get_args_or(1, si32{ 1 });
                                                            selection_search(gear, dir > 0 ? feed::fwd : feed::rev);
                                                            gear.set_handled();
                                                        });
                                                    }},
                { methods::ScrollViewportByPage,    [&]
                                                    {
                                                        luafx.run_with_gear([&](auto& gear)
                                                        {
                                                            if (target != &normal) return;
                                                            auto vector = luafx.get_args_or(1, dot_00);
                                                            base::riseup(tier::preview, e2::form::upon::scroll::bypage::v, { .vector = vector });
                                                            gear.set_handled();
                                                        });
                                                    }},
                { methods::ScrollViewportByCell,    [&]
                                                    {
                                                        luafx.run_with_gear([&](auto& gear)
                                                        {
                                                            if (target != &normal) return;
                                                            auto vector = luafx.get_args_or(1, dot_00);
                                                            base::riseup(tier::preview, e2::form::upon::scroll::bystep::v, { .vector = vector });
                                                            gear.set_handled();
                                                        });
                                                    }},
                { methods::ScrollViewportToTop,     [&]
                                                    {
                                                        luafx.run_with_gear([&](auto& gear)
                                                        {
                                                            if (target != &normal) return;
                                                            base::riseup(tier::preview, e2::form::upon::scroll::to_top::y);
                                                            gear.set_handled();
                                                        });
                                                    }},
                { methods::ScrollViewportToEnd,     [&]
                                                    {
                                                        luafx.run_with_gear([&](auto& gear)
                                                        {
                                                            if (target != &normal) return;
                                                            base::riseup(tier::preview, e2::form::upon::scroll::to_end::y);
                                                            gear.set_handled();
                                                        });
                                                    }},
                { methods::SendKey,                 [&]
                                                    {
                                                        luafx.run_with_gear([&](auto& gear)
                                                        {
                                                            auto crop = luafx.get_args_or(1, ""s);
                                                            if (crop.size()) data_out(crop);
                                                            gear.set_handled();
                                                        });
                                                    }},
                { methods::Print,                   [&]
                                                    {
                                                        luafx.run_with_gear([&](auto& gear)
                                                        {
                                                            auto args_count = luafx.args_count();
                                                            auto crop = text{};
                                                            for (auto i = 1; i <= args_count; i++)
                                                            {
                                                                crop += luafx.get_args_or(i, ""s);
                                                            }
                                                            if (crop.size()) data_in(crop);
                                                            gear.set_handled();
                                                        });
                                                    }},
                { methods::PrintLn,                 [&]
                                                    {
                                                        luafx.run_with_gear([&](auto& gear)
                                                        {
                                                            auto args_count = luafx.args_count();
                                                            auto crop = text{};
                                                            for (auto i = 1; i <= args_count; i++)
                                                            {
                                                                crop += luafx.get_args_or(i, ""s);
                                                            }
                                                            crop += "\n\r";
                                                            data_in(crop);
                                                            gear.set_handled();
                                                        });
                                                    }},
                { methods::CopyViewport,            [&]
                                                    {
                                                        luafx.run_with_gear([&](auto& gear)
                                                        {
                                                            prnscrn(gear);
                                                            gear.set_handled();
                                                        });
                                                    }},
                { methods::CopySelection,           [&]
                                                    {
                                                        luafx.run_with_gear([&](auto& gear)
                                                        {
                                                            if (selection_active())
                                                            {
                                                                copy(gear);
                                                                gear.set_handled();
                                                            }
                                                            else if (auto v = ipccon.get_current_line())
                                                            {
                                                                _copy(gear, v.value());
                                                                gear.set_handled();
                                                            }
                                                        });
                                                    }},
                { methods::PasteClipboard,          [&]
                                                    {
                                                        luafx.run_with_gear([&](auto& gear)
                                                        {
                                                            paste(gear);
                                                            gear.set_handled();
                                                        });
                                                    }},
                { methods::ClearClipboard,          [&]
                                                    {
                                                        luafx.run_with_gear([&](auto& gear)
                                                        {
                                                            gear.clear_clipboard();
                                                            gear.set_handled();
                                                        });
                                                    }},
                { methods::ClipboardFormat,         [&]
                                                    {
                                                        luafx.run_with_gear_wo_return([&](auto& gear){ gear.set_handled(); });
                                                        auto args_count = luafx.args_count();
                                                        if (!args_count)
                                                        {
                                                            auto state = (si32)selmod;
                                                            luafx.set_return(state);
                                                        }
                                                        else
                                                        {
                                                            auto state = luafx.get_args_or(1, si32{ 1 });
                                                            set_selmod(state % mime::count);
                                                            luafx.set_return();
                                                        }
                                                    }},
                { methods::SelectionForm,           [&]
                                                    {
                                                        luafx.run_with_gear_wo_return([&](auto& gear){ gear.set_handled(); });
                                                        auto args_count = luafx.args_count();
                                                        if (!args_count)
                                                        {
                                                            auto state = (si32)selalt;
                                                            luafx.set_return(state);
                                                        }
                                                        else
                                                        {
                                                            auto state = luafx.get_args_or(1, si32{ 0 });
                                                            set_selalt(state);
                                                            luafx.set_return();
                                                        }
                                                    }},
                { methods::ClearSelection,          [&]
                                                    {
                                                        luafx.run_with_gear([&](auto& gear)
                                                        {
                                                            if (selection_active())
                                                            {
                                                                exec_cmd(commands::ui::deselect);
                                                                gear.set_handled();
                                                            }
                                                        });
                                                    }},
                { methods::OneShotSelection,        [&]
                                                    {
                                                        luafx.run_with_gear([&](auto& /*gear*/)
                                                        {
                                                            auto format = luafx.get_args_or(1, ""s);
                                                            if (format.empty())
                                                            {
                                                                set_oneshot(mime::textonly);
                                                            }
                                                            else
                                                            {
                                                                set_oneshot(netxs::get_or(xml::options::format, format, mime::textonly));
                                                            }
                                                        });
                                                    }},
                { methods::UndoReadline,            [&]
                                                    {
                                                        luafx.run_with_gear([&](auto& gear)
                                                        {
                                                            exec_cmd(commands::ui::undo);
                                                            gear.set_handled();
                                                        });
                                                    }},
                { methods::RedoReadline,            [&]
                                                    {
                                                        luafx.run_with_gear([&](auto& gear)
                                                        {
                                                            exec_cmd(commands::ui::redo);
                                                            gear.set_handled();
                                                        });
                                                    }},
                { methods::CwdSync,                 [&]
                                                    {
                                                        auto& cwd_sync = base::property("terminal.cwd_sync", 0);
                                                        luafx.run_with_gear_wo_return([&](auto& gear){ gear.set_handled(); });
                                                        auto args_count = luafx.args_count();
                                                        if (!args_count)
                                                        {
                                                            auto state = (si32)cwd_sync;
                                                            luafx.set_return(state);
                                                        }
                                                        else
                                                        {
                                                            auto state = luafx.get_args_or(1, si32{ 0 });
                                                            base::riseup(tier::preview, terminal::events::toggle::cwdsync, state);
                                                            luafx.set_return();
                                                        }
                                                    }},
                { methods::LineWrapMode,            [&]
                                                    {
                                                        luafx.run_with_gear_wo_return([&](auto& gear){ gear.set_handled(); });
                                                        auto args_count = luafx.args_count();
                                                        if (!args_count)
                                                        {
                                                            auto state = target->style.wrp() == wrap::off ? 0 : 1;
                                                            luafx.set_return(state);
                                                        }
                                                        else
                                                        {
                                                            auto state = luafx.get_args_or(1, si32{ 1 });
                                                            set_wrapln(1 + (si32)!state);
                                                            luafx.set_return();
                                                        }
                                                    }},
                { methods::LineAlignMode,           [&]
                                                    {
                                                        luafx.run_with_gear_wo_return([&](auto& gear){ gear.set_handled(); });
                                                        auto args_count = luafx.args_count();
                                                        if (!args_count)
                                                        {
                                                            auto align = target->style.jet();
                                                            auto state = align == bias::left || align == bias::none  ? 0 :
                                                                                                align == bias::right ? 1 : 2;
                                                            luafx.set_return(state);
                                                        }
                                                        else
                                                        {
                                                            auto state = luafx.get_args_or(1, si32{ 0 });
                                                            set_align(1 + std::clamp(state, 0, 2));
                                                            luafx.set_return();
                                                        }
                                                    }},
                { methods::LogMode,                 [&]
                                                    {
                                                        luafx.run_with_gear_wo_return([&](auto& gear){ gear.set_handled(); });
                                                        auto args_count = luafx.args_count();
                                                        if (!args_count)
                                                        {
                                                            auto state = (si32)io_log;
                                                            luafx.set_return(state);
                                                        }
                                                        else
                                                        {
                                                            auto state = luafx.get_args_or(1, si32{ 0 });
                                                            set_log(state);
                                                            luafx.set_return();
                                                        }
                                                    }},
                { methods::AltbufMode,              [&]
                                                    {
                                                        auto args_count = luafx.args_count();
                                                        if (!args_count)
                                                        {
                                                            auto is_altbuf = target != &normal;
                                                            luafx.set_return(is_altbuf);
                                                        }
                                                        else
                                                        {
                                                            auto state = luafx.get_args_or(1, faux);
                                                            state ? _decset(1049) : _decrst(1049);
                                                            luafx.set_return();
                                                        }
                                                    }},
                { methods::ForwardKeys,             [&]
                                                    {
                                                        luafx.run_with_gear([&](auto& gear)
                                                        {
                                                            key_event(gear, true);
                                                        });
                                                    }},
                { methods::ClearScrollback,         [&]
                                                    {
                                                        luafx.run_with_gear_wo_return([&](auto& gear){ gear.set_handled(); });
                                                        if (selection_active())
                                                        {
                                                            exec_cmd(commands::ui::deselect);
                                                        }
                                                        clear_scrollback();
                                                        luafx.set_return();
                                                    }},
                { methods::ScrollbackSize,          [&]
                                                    {
                                                        luafx.run_with_gear_wo_return([&](auto& gear){ gear.set_handled(); });
                                                        target->flush();
                                                        auto args_count = luafx.args_count();
                                                        if (!args_count)
                                                        {
                                                            luafx.set_return(defcfg.def_length, defcfg.def_growdt, defcfg.def_growmx);
                                                        }
                                                        else
                                                        {
                                                            auto ring_size = luafx.get_args_or(1, defcfg.def_length);
                                                            auto grow_step = luafx.get_args_or(2, defcfg.def_growdt);
                                                            auto grow_mxsz = luafx.get_args_or(3, defcfg.def_growmx);
                                                            normal.resize_history(ring_size, grow_step, grow_mxsz);
                                                            luafx.set_return();
                                                        }
                                                    }},
                { methods::EventReporting,          [&]
                                                    {
                                                        luafx.run_with_gear_wo_return([&](auto& gear){ gear.set_handled(); });
                                                        auto args_count = luafx.args_count();
                                                        if (!args_count)
                                                        {
                                                            auto src_list = txts{};
                                                            auto i = 0;
                                                            auto e = event_sources;
                                                            while (e)
                                                            {
                                                                if (e & 1) src_list.push_back(event_source_name[i]);
                                                                i++;
                                                                e >>= 1;
                                                            }
                                                            luafx.set_return_array(src_list);
                                                        }
                                                        else
                                                        {
                                                            auto prev_event_sources = event_sources;
                                                            for (auto i = 1; i <= args_count; i++)
                                                            {
                                                                auto src = luafx.get_args_or(i, ""s);
                                                                if (src.empty())
                                                                {
                                                                    event_sources = {};
                                                                }
                                                                else
                                                                {
                                                                    auto iter = ui::terminal::event_source_map.find(src);
                                                                    if (iter != ui::terminal::event_source_map.end())
                                                                    {
                                                                        event_sources |= iter->second;
                                                                    }
                                                                    else
                                                                    {
                                                                        log("%%Unknown event source: '%%'", prompt::term, src);
                                                                    }
                                                                }
                                                            }
                                                            auto mouse_tracking = event_sources & ui::terminal::event_source::mouse;
                                                            if ((prev_event_sources & ui::terminal::event_source::mouse) != mouse_tracking)
                                                            {
                                                                base::enqueue([&, mouse_tracking](auto& /*boss*/) // Perform switching outside of Lua script context.
                                                                {
                                                                    mouse_tracking ? mtrack.enable(input::mouse::mode::vt_input_mode)
                                                                                   : mtrack.disable(input::mouse::mode::vt_input_mode);
                                                                });
                                                            }
                                                            luafx.set_return();
                                                        }
                                                    }},
                { methods::Restart,                 [&]
                                                    {
                                                        luafx.run_with_gear([&](auto& gear)
                                                        {
                                                            exec_cmd(commands::ui::restart);
                                                            gear.set_handled();
                                                        });
                                                    }},
                { methods::Quit,                    [&]
                                                    {
                                                        luafx.run_with_gear([&](auto& gear)
                                                        {
                                                            exec_cmd(commands::ui::sighup);
                                                            gear.set_handled();
                                                        });
                                                    }},
            });

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
                        base::signal(tier::release, e2::area, new_area);
                        base::region = new_area;
                    }
                    base::deface();
                }
            };
            LISTEN(tier::release, ui::e2::command::request::inputfields, inputfield_request)
            {
                auto& console = *target;
                auto composit_cursor = console.get_coord(origin) + origin;
                if (auto parent = base::parent())
                {
                    auto parent_size = parent->size();
                    if (parent_size.inside(composit_cursor))
                    {
                        if (ime_on) composit_cursor = calc_composition_cursor(parent_size, composit_cursor);
                        auto r = rect{ composit_cursor, { parent_size.x - composit_cursor.x, 1 }};
                        auto offset = dot_00;
                        parent->global(offset);
                        r.coor -= offset;
                        inputfield_request.set_value(r);
                    }
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
            LISTEN(tier::release, input::events::keybd::post, gear)
            {
                key_event(gear);
            };
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                auto& console = *target;
                if (status.update(console))
                {
                    base::riseup(tier::preview, e2::form::prop::ui::footer, status.data);
                }

                auto clip = parent_canvas.clip();
                auto full = parent_canvas.full();
                auto original_cursor = console.get_coord(origin); // base::coor() and origin are the same.

                auto brush = defclr;
                if (defcfg.def_filler != argb::default_color) brush.bgc(defcfg.def_filler); // Unsync with SGR default background.
                parent_canvas.fill(cell::shaders::fusefull(brush));

                if (ime_on) // Draw IME composition overlay.
                {
                    if (auto parent = base::parent())
                    {
                        brush.fuse(console.cell_under_cursor());
                        auto viewport_square = parent->area();
                        auto viewport_cursor = original_cursor + origin;
                        if (viewport_square.size.inside(viewport_cursor))
                        {
                            auto composit_cursor = calc_composition_cursor(viewport_square.size, viewport_cursor);

                            auto dy = std::max(0, composit_cursor.y - (viewport_square.size.y - 1));
                            if (dy) // Shift parent_canvas down if needed.
                            {
                                auto scrolldown = full;
                                composit_cursor.y -= dy;
                                viewport_cursor.y -= dy;
                                scrolldown.coor.y -= dy;
                                parent_canvas.full(scrolldown);
                            }
                            console.output(parent_canvas);
                            if (dy) parent_canvas.full(full); // Return parent_canvas back.

                            viewport_square.coor -= origin;
                            if (auto context2D = parent_canvas.change_basis(viewport_square))
                            {
                                parent_canvas.output<faux>(imebox, viewport_cursor, cell::shaders::mimic(brush));
                            }
                            composit_cursor -= origin; // Convert to original (scrollback based) basis.
                            caret.coor(composit_cursor);
                        }
                        else // Original cursor is outside the viewport.
                        {
                            caret.coor(original_cursor);
                            console.output(parent_canvas);
                        }
                    }
                }
                else
                {
                    caret.coor(original_cursor);
                    if (brush.bga() != 0xFF) parent_canvas.fill(rect{ caret.coor(), dot_11 }, [&](cell& c){ c.fgc(console.brush.fgc()); }); // Prefill the cursor cell placeholder in the case of transparent background.
                    console.output(parent_canvas);
                }
                if (invert) parent_canvas.fill(cell::shaders::invbit);

                if (oversz.b > 0) // Shade the viewport bottom oversize (futures).
                {
                    auto bottom_oversize = full;
                    bottom_oversize.coor.x -= oversz.l;
                    bottom_oversize.coor.y += console.get_basis() + console.panel.y - console.scend;
                    bottom_oversize.size.y  = oversz.b;
                    bottom_oversize.size.x += oversz.l + oversz.r;
                    bottom_oversize = bottom_oversize.trim(clip);
                    parent_canvas.fill(bottom_oversize, cell::shaders::xlight);
                }

                //if (clip.coor.x) // Shade left and right margins.
                //{
                //    auto west = full;
                //    west.size = dot_mx;
                //    west.coor.y -= dot_mx.y / 2;
                //    auto east = west;
                //    auto pads = console.getpad();
                //    west.coor.x -= oversz.l - pads + dot_mx.x;
                //    east.coor.x += oversz.r - pads + console.panel.x;
                //    west = west.trim(clip);
                //    east = east.trim(clip);
                //    parent_canvas.fill(west, cell::shaders::xlucent(defcfg.def_lucent));
                //    parent_canvas.fill(east, cell::shaders::xlucent(defcfg.def_lucent));
                //}

                // Debug: Shade active viewport.
                //{
                //    auto size = console.panel;
                //    size.y -= console.sctop + console.scend;
                //    auto vp = rect{{ 0,console.get_basis() + console.sctop }, size };
                //    vp.coor += parent_canvas.full().coor;
                //    vp = vp.clip(parent_canvas.clip());
                //    parent_canvas.fill(vp, [](auto& c){ c.fuse(cell{}.bgc(magentalt).bga(50)); });
                //}
            };
        }
    };

    struct dtvt
        : public ui::form<dtvt>
    {
        static constexpr auto classname = basename::dtvt;

        std::unordered_map<id_t, netxs::sptr<input::tooltip_t>> tooltips;
        auto& get_tooltip_reference(id_t gear_id)
        {
            auto iter = tooltips.find(gear_id);
            if (iter == tooltips.end())
            {
                iter = tooltips.emplace(gear_id, ptr::shared<input::tooltip_t>()).first;
            }
            auto& tooltip_sptr = iter->second;
            return tooltip_sptr;
        }

        // dtvt: Event handler.
        struct link : s11n, input_fields_handler
        {
            using input_fields_handler::handle;

            dtvt& owner; // link: Terminal object reference.

            void handle(s11n::xs::bitmap_dtvt       /*lock*/)
            {
                owner.base::enqueue([&](auto& /*boss*/) mutable
                {
                    owner.base::deface();
                });
            }
            void handle(s11n::xs::jgc_list            lock)
            {
                s11n::receive_jgc(lock);
                owner.base::enqueue([&](auto& /*boss*/) mutable
                {
                    owner.base::deface();
                });
            }
            void handle(s11n::xs::tooltips            lock)
            {
                owner.base::enqueue([&, tooltips = lock.thing](auto& /*boss*/) mutable
                {
                    for (auto& tooltip : tooltips)
                    {
                        auto& tooltip_sptr = owner.get_tooltip_reference(tooltip.gear_id);
                        tooltip_sptr->set(tooltip.utf8);
                    }
                });
            }
            void handle(s11n::xs::fullscrn            lock)
            {
                auto m = lock.thing;
                lock.unlock();
                if (owner.active)
                {
                    auto guard = owner.sync();
                    if (auto gear_ptr = owner.base::getref<hids>(m.gear_id))
                    if (auto parent_ptr = owner.base::parent())
                    {
                        auto& gear = *gear_ptr;
                        gear.set_multihome();
                        if (gear.captured(owner.id)) gear.setfree();
                        parent_ptr->base::riseup(tier::preview, e2::form::size::enlarge::fullscreen, gear);
                    }
                }
            }
            void handle(s11n::xs::maximize            lock)
            {
                auto m = lock.thing;
                lock.unlock();
                if (owner.active)
                {
                    auto guard = owner.sync();
                    if (auto gear_ptr = owner.base::getref<hids>(m.gear_id))
                    if (auto parent_ptr = owner.base::parent())
                    {
                        auto& gear = *gear_ptr;
                        gear.set_multihome();
                        if (gear.captured(owner.id)) gear.setfree();
                        parent_ptr->base::riseup(tier::preview, e2::form::size::enlarge::maximize, gear);
                    }
                }
            }
            void handle(s11n::xs::sysfocus            lock)
            {
                auto f = lock.thing;
                lock.unlock();
                if (owner.active)
                {
                    auto guard = owner.sync(); // Guard the owner.This() call.
                    auto owner_ptr = owner.This();
                    if (f.state)
                    {
                        owner.base::signal(tier::request, input::events::focus::add, { .gear_id = f.gear_id, .focus_type = f.focus_type });
                    }
                    else
                    {
                        owner.base::signal(tier::request, input::events::focus::rem, { .gear_id = f.gear_id });
                    }
                }
            }
            void handle(s11n::xs::syskeybd            lock)
            {
                auto k = lock.thing;
                lock.unlock();
                if (owner.active)
                {
                    auto guard = owner.sync();
                    if (auto gear_ptr = owner.base::getref<hids>(k.gear_id))
                    {
                        auto& gear = *gear_ptr;
                        gear.set_multihome();
                        gear.keybd::vkevent = owner.indexer.get_kbchord_hint(k.vkchord);
                        gear.keybd::scevent = owner.indexer.get_kbchord_hint(k.scchord);
                        gear.keybd::chevent = owner.indexer.get_kbchord_hint(k.chchord);
                        k.syncto(gear);
                        //owner.base::riseup(tier::release, input::events::keybd::post, gear, true);
                        pro::keybd::forward_release(owner, gear);
                    }
                }
            };
            void handle(s11n::xs::mouse_event         lock)
            {
                auto m = lock.thing;
                lock.unlock();
                if (owner.active)
                {
                    auto guard = owner.sync();
                    if (auto gear_ptr = owner.base::getref<hids>(m.gear_id))
                    if (auto parent_ptr = owner.base::parent())
                    {
                        auto& gear = *gear_ptr;
                        auto& parent = *parent_ptr;
                        gear.set_multihome();
                        if (gear.captured(owner.id)) gear.setfree();
                        auto basis = gear.owner.base::coor();
                        owner.global(basis);
                        gear.replay(parent, m.cause, m.coord - basis, m.click - basis, m.delta, m.buttons, m.bttn_id, m.dragged, m.ctlstat, m.whlfp, m.whlsi, m.hzwhl);
                    }
                }
            }
            void handle(s11n::xs::minimize            lock)
            {
                owner.base::enqueue([&, m = lock.thing](auto& /*boss*/)
                {
                    if (auto gear_ptr = owner.base::getref<hids>(m.gear_id))
                    {
                        auto& gear = *gear_ptr;
                        gear.set_multihome();
                        owner.base::riseup(tier::preview, e2::form::size::minimize, gear);
                    }
                });
            }
            void handle(s11n::xs::expose            /*lock*/)
            {
                owner.base::enqueue([&](auto& /*boss*/)
                {
                    owner.base::riseup(tier::preview, e2::form::layout::expose);
                });
            }
            void handle(s11n::xs::clipdata            lock)
            {
                auto c = lock.thing;
                lock.unlock();
                if (owner.active)
                {
                    auto guard = owner.sync();
                    if (auto gear_ptr = owner.base::getref<hids>(c.gear_id))
                    {
                        gear_ptr->set_multihome();
                        gear_ptr->set_clipboard(c);
                    }
                }
            }
            void handle(s11n::xs::clipdata_request    lock)
            {
                auto c = lock.thing;
                lock.unlock();
                if (owner.active)
                {
                    auto guard = owner.sync();
                    if (auto gear_ptr = owner.base::getref<hids>(c.gear_id))
                    {
                        auto& gear = *gear_ptr;
                        gear.set_multihome();
                        gear.owner.base::signal(tier::request, input::events::clipboard, gear);
                        auto& data = gear.board::cargo;
                        if (data.hash != c.hash)
                        {
                            s11n::clipdata.send(owner, c.gear_id, data.hash, data.size, data.utf8, data.form, data.meta);
                            return;
                        }
                    }
                    else log(prompt::dtvt, ansi::err("Unregistered input device id: ", c.gear_id));
                    s11n::clipdata.send(owner, c.gear_id, c.hash, dot_00, text{}, mime::ansitext, text{});
                }
            }
            void handle(s11n::xs::header              lock)
            {
                owner.base::enqueue([&, /*id = h.window_id,*/ header = lock.thing.utf8](auto& /*boss*/) mutable
                {
                    owner.base::riseup(tier::preview, e2::form::prop::ui::header, header);
                });
            }
            void handle(s11n::xs::footer              lock)
            {
                owner.base::enqueue([&, /*id = f.window_id,*/ footer = lock.thing.utf8](auto& /*boss*/) mutable
                {
                    owner.base::riseup(tier::preview, e2::form::prop::ui::footer, footer);
                });
            }
            void handle(s11n::xs::header_request      lock)
            {
                auto c = lock.thing;
                lock.unlock();
                if (owner.active)
                {
                    //todo use window_id
                    auto guard = owner.sync();
                    auto header_utf8 = owner.base::riseup(tier::request, e2::form::prop::ui::header);
                    s11n::header.send(owner, c.window_id, header_utf8);
                }
            }
            void handle(s11n::xs::footer_request      lock)
            {
                auto c = lock.thing;
                lock.unlock();
                if (owner.active)
                {
                    //todo use window_id
                    auto guard = owner.sync();
                    auto footer_utf8 = owner.base::riseup(tier::request, e2::form::prop::ui::footer);
                    s11n::footer.send(owner, c.window_id, footer_utf8);
                }
            }
            void handle(s11n::xs::warping             lock)
            {
                owner.base::enqueue([&, /*id = w.window_id,*/ warp = lock.thing.warpdata](auto& /*boss*/)
                {
                    //todo use window_id
                    owner.base::riseup(tier::preview, e2::form::layout::swarp, warp);
                });
            }
            void handle(s11n::xs::logs                lock)
            {
                s11n::recycle_log(lock, os::process::id.second);
            }
            void handle(s11n::xs::sysclose            lock)
            {
                lock.unlock();
                auto guard = owner.sync();
                owner.active.exchange(faux);
                owner.stop(true);
            }
            void handle(s11n::xs::sysstart          /*lock*/)
            {
                owner.base::enqueue([&](auto& /*boss*/)
                {
                    owner.base::riseup(tier::release, e2::form::global::sysstart, 1);
                });
            }
            void handle(s11n::xs::cwd                 lock)
            {
                owner.base::enqueue([&, path = lock.thing.path](auto& /*boss*/)
                {
                    owner.base::riseup(tier::preview, e2::form::prop::cwd, path);
                });
            }
            void handle(s11n::xs::gui_command         lock)
            {
                owner.base::enqueue([&, gui_cmd = lock.thing](auto& /*boss*/)
                {
                    if (auto gear_ptr = owner.base::getref<hids>(gui_cmd.gear_id))
                    {
                        gear_ptr->set_multihome();
                        owner.base::riseup(tier::preview, e2::command::gui, gui_cmd);
                    }
                });
            }

            link(dtvt& owner)
                : s11n{ *this, owner.id },
                  input_fields_handler{ owner },
                  owner{ owner }
            { }
        };

        struct msgs
        {
            static constexpr auto no_signal    = "NO SIGNAL"sv;
            static constexpr auto disconnected = "Disconnected"sv;
        };

        using vtty = os::dtvt::vtty;

        link stream; // dtvt: Event handler.
        flag active; // dtvt: Terminal lifetime.
        si32 opaque; // dtvt: Object transparency on d_n_d (no pro::cache).
        si32 nodata; // dtvt: Show splash "No signal".
        face splash; // dtvt: "No signal" splash.
        page errmsg; // dtvt: Overlay error message.
        vtty ipccon; // dtvt: IPC connector. Should be destroyed first.

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
            base::enqueue([&](auto& /*boss*/) mutable // Unexpected disconnection.
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
        void start(text config, auto connect)
        {
            if (ipccon)
            {
                active.exchange(faux); // Do not show "Disconnected".
                ipccon.payoff();
            }
            errmsg = genmsg(msgs::no_signal);
            nodata = {};
            stream.syswinsz.freeze().thing.winsize = {};
            active.exchange(true);
            auto receiver = [&](view utf8)
            {
                if (active)
                {
                    stream.sync(utf8);
                    stream.request_jgc(*this);
                }
            };
            ipccon.runapp(config, base::size(), connect, receiver, [&]{ onexit(); });
        }
        // dtvt: Close dtvt-object.
        void stop(bool fast, bool notify = true)
        {
            if (notify)
            {
                base::signal(tier::request, e2::form::proceed::quit::one, fast);
            }
            auto nodtvt = [&]
            {
                auto lock = stream.bitmap_dtvt.freeze();
                auto& canvas = lock.thing.image;
                return !canvas.hash(); // Canvas never resized/received.
            }();
            if (nodtvt) // Terminate a non-dtvt-aware application that has never sent its canvas.
            {
                ipccon.abort();
            }
            else if (fast && active.exchange(faux) && ipccon) // Notify and queue closing immediately.
            {
                stream.sysclose.send(*this, fast);
            }
            else if (active && ipccon) // Just notify if active. Queue closing if not.
            {
                stream.sysclose.send(*this, fast);
                return;
            }
            base::enqueue<faux>([&, backup = This()](auto& /*boss*/) mutable
            {
                ipccon.payoff();
                auto lock = bell::sync();
                base::riseup(tier::release, e2::form::proceed::quit::one, true);
                backup.reset(); // Backup should dtored under the lock.
            });
        }
        // dtvt: Splash screen if there is no next frame.
        void fallback(core const& canvas, bool forced = faux, bool show_msg = true)
        {
            auto size = base::size();
            if (splash.size() != size || forced)
            {
                splash.size(size);
                auto parent_id = id_t{}; // Handover control to the parent if no response.
                if (auto parent = base::parent()) parent_id = parent->id;
                if (canvas.size())
                {
                    if (show_msg)
                    {
                        auto tmpbuf = vrgb{};
                        splash.zoom(canvas, cell::shaders::onlyid(parent_id));
                        splash.output(errmsg);
                        splash.blur(2, tmpbuf, [](cell& c){ c.fgc(argb::transit(c.bgc(), c.fgc(), 127)); });
                        splash.output(errmsg);
                    }
                    else
                    {
                        splash.fill(canvas, cell::shaders::onlyid(parent_id));
                    }
                }
                else splash.wipe(cell{}.link(parent_id).fgc(blacklt).bgc(blacklt).alpha(0x40));
            }
        }
        // dtvt: Render next frame.
        void fill(core& parent_canvas, core const& canvas)
        {
            if (opaque == 0xFF) parent_canvas.fill(canvas, cell::shaders::overlay);
            else                parent_canvas.fill(canvas, cell::shaders::transparent(opaque));
        }

        dtvt()
            : stream{*this },
              active{ true },
              opaque{ 0xFF },
              nodata{      }
        {
            LISTEN(tier::release, input::events::device::mouse::any, gear)
            {
                if (gear.captured(base::id))
                {
                    if (!gear.m_sys.buttons) gear.setfree();
                }
                else if (gear.m_sys.buttons) gear.capture(base::id);
                gear.m_sys.gear_id = gear.id;
                stream.sysmouse.send(*this, gear.m_sys);
                gear.dismiss();
            };
            LISTEN(tier::general, input::events::die, gear)
            {
                gear.setfree();
                gear.m_sys.gear_id = gear.id;
                gear.m_sys.enabled = hids::stat::die;
                stream.sysmouse.send(*this, gear.m_sys);
            };
            on(tier::mouserelease, input::key::MouseHover, [&](hids& gear)
            {
                auto& tooltip_sptr = get_tooltip_reference(gear.id);
                gear.tooltip.set(tooltip_sptr); // Set tooltip reference.
            });
            on(tier::mouserelease, input::key::MouseLeave, [&](hids& gear)
            {
                gear.m_sys.gear_id = gear.id;
                gear.m_sys.enabled = hids::stat::halt;
                stream.sysmouse.send(*this, gear.m_sys);
            });
            bell::dup_handler(tier::general, input::events::halt.id);
            LISTEN(tier::release, input::events::focus::set::any, seed)
            {
                auto deed = bell::protos();
                auto state = deed == input::events::focus::set::on.id;
                stream.sysfocus.send(*this, seed.gear_id, state, seed.focus_type, seed.treeid, seed.digest);
            };
            LISTEN(tier::preview, input::events::keybd::any, gear)
            {
                gear.gear_id = gear.id;
                stream.syskeybd.send(*this, gear);
                gear.dismiss();
            };
            LISTEN(tier::anycast, e2::form::prop::cwd, path)
            {
                stream.cwd.send(*this, path);
            };
            LISTEN(tier::release, e2::area, new_area)
            {
                //todo implement deform/inform (for incoming XTWINOPS/swarp)
                auto winsize = stream.syswinsz.freeze().thing.winsize;
                if (ipccon && winsize != new_area.size)
                {
                    stream.syswinsz.send(*this, 0, new_area.size, faux);
                }
            };
            LISTEN(tier::anycast, e2::form::prop::lucidity, value)
            {
                if (value == -1) value = opaque;
                else             opaque = value;
            };
            auto& maxoff = base::field(span{ span::period::den / std::max(1, ui::skin::globals().maxfps) }); // dtvt: Max delay before showing "No signal".
            LISTEN(tier::general, e2::config::fps, fps)
            {
                maxoff = span{ span::period::den / std::max(1, fps) };
                if (fps > 0)
                {
                    stream.fps.send(*this, fps);
                }
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
                            fallback(canvas, faux, faux);
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