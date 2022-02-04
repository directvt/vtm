// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_TERMINAL_HPP
#define NETXS_TERMINAL_HPP

#include "../ui/controls.hpp"

namespace netxs::events::userland
{
    struct uiterm
    {
        EVENTPACK( uiterm, netxs::events::userland::root::custom )
        {
            GROUP_XS( layout, iota ),

            SUBSET_XS( layout )
            {
                EVENT_XS( align , bias ),
                EVENT_XS( wrapln, wrap ),
            };
        };
    };

}

// terminal: Terminal UI control.
namespace netxs::ui
{
    class term
        : public ui::form<term>
    {
        static constexpr iota max_length = 65535; // term: Max line length.
        static constexpr iota def_length = 20000; // term: Default scrollback history length.
        static constexpr iota def_growup = 0;     // term: Default scrollback history grow step.
        static constexpr iota def_tablen = 8;     // term: Default tab length.

    public:
        using events = netxs::events::userland::uiterm;

        struct commands
        {
            struct erase
            {
                struct line
                {
                    enum : iota
                    {
                        right = 0,
                        left  = 1,
                        all   = 2,
                    };
                };
                struct display
                {
                    enum : iota
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
                enum commands : iota
                {
                    right,
                    left,
                    center,
                    wrapon,
                    wrapoff,
                    togglewrp,
                    reset,
                    clear,
                };
            };
            struct cursor // See pro::caret.
            {
                enum : iota
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
        };
    private:

        // term: VT-buffer status.
        struct term_state
        {
            using buff = ansi::esc;
            iota size = 0;
            iota peak = 0;
            iota step = 0;
            twod area;
            buff data;
            template<class bufferbase>
            auto update(bufferbase const& scroll)
            {
                if (scroll.update_status(*this))
                {
                    data.clear();
                    data.jet(bias::right).add(size,
                                         "/", peak,
                                         "+", step,
                                         " ", area.x, ":", area.y);
                    return true;
                }
                else return faux;
            }
        };

        // term: VT-style mouse tracking functionality.
        struct m_tracking
        {
            enum mode
            {
                none = 0,
                bttn = 1 << 0,
                drag = 1 << 1,
                move = 1 << 2,
                over = 1 << 3,
                buttons_press = bttn,
                buttons_drags = bttn | drag,
                all_movements = bttn | drag | move,
                negative_args = bttn | drag | move | over,
            };
            enum prot
            {
                x11,
                sgr,
            };

            m_tracking(term& owner)
                : owner{ owner }
            { }

            void enable (mode m)
            {
                state |= m;
                if (state && !token.count()) // Do not subscribe if it is already subscribed
                {
                    owner.SUBMIT_T(tier::release, hids::events::mouse::scroll::any, token, gear)
                    {
                        gear.dismiss();
                    };
                    owner.SUBMIT_T(tier::release, hids::events::mouse::any, token, gear)
                    {
                        auto& console = *owner.target;
                        auto c = gear.coord;
                        c.y -= console.get_basis();
                        moved = coord((state & mode::over) ? c
                                                           : std::clamp(c, dot_00, console.panel - dot_11));
                        auto cause = owner.bell::protos<tier::release>();
                        if (proto == sgr) serialize<sgr>(gear, cause);
                        else              serialize<x11>(gear, cause);
                        owner.answer(queue);
                    };
                    owner.SUBMIT_T(tier::general, hids::events::die, token, gear)
                    {
                        log("term: hids::events::die, id = ", gear.id);
                        auto cause = hids::events::die.id;
                        if (proto == sgr) serialize<sgr>(gear, cause);
                        else              serialize<x11>(gear, cause);
                        owner.answer(queue);
                    };
                }
            }
            void disable(mode m) { state &= ~(m); if (!state) token.clear(); }
            void setmode(prot p) { proto = p; }

        private:
            term&       owner; // m_tracking: Terminal object reference.
            testy<twod> coord; // m_tracking: Last coord of mouse cursor.
            ansi::esc   queue; // m_tracking: Buffer.
            subs        token; // m_tracking: Subscription token.
            bool        moved = faux;
            iota        proto = prot::x11;
            iota        state = mode::none;

            void capture(hids& gear)
            {
                gear.capture(owner.id);
                gear.dismiss();
            }
            void release(hids& gear)
            {
                if (gear.captured(owner.id)) gear.release(faux);
                gear.dismiss();
            }
            template<prot PROT>
            void proceed(hids& gear, iota meta, bool ispressed = faux)
            {
                meta |= gear.meta(hids::SHIFT | hids::ALT | hids::CTRL);
                meta |= gear.meta(hids::RCTRL) ? hids::CTRL : 0;
                switch (PROT)
                {
                    case prot::x11: queue.mouse_x11(meta, coord);            break;
                    case prot::sgr: queue.mouse_sgr(meta, coord, ispressed); break;
                    default: break;
                }
            }
            // m_tracking: Serialize mouse state.
            template<prot PROT>
            void serialize(hids& gear, id_t cause)
            {
                using m = hids::events::mouse;
                using b = hids::events::mouse::button;
                constexpr static iota left = 0;
                constexpr static iota mddl = 1;
                constexpr static iota rght = 2;
                constexpr static iota btup = 3;
                constexpr static iota idle = 32;
                constexpr static iota wheel_up = 64;
                constexpr static iota wheel_dn = 65;
                constexpr static iota up_left = PROT == sgr ? left : btup;
                constexpr static iota up_rght = PROT == sgr ? rght : btup;
                constexpr static iota up_mddl = PROT == sgr ? mddl : btup;

                auto ismove = moved && state & mode::move;
                auto isdrag = moved && state & mode::drag;
                switch (cause)
                {
                    // Move
                    case b::drag::pull::leftright.id:
                    case b::drag::pull::left     .id: if (isdrag) proceed<PROT>(gear, idle + left, true); break;
                    case b::drag::pull::middle   .id: if (isdrag) proceed<PROT>(gear, idle + mddl, true); break;
                    case b::drag::pull::right    .id: if (isdrag) proceed<PROT>(gear, idle + rght, true); break;
                    case m::move                 .id: if (ismove) proceed<PROT>(gear, idle + btup, faux); break;
                    // Press
                    case b::down::leftright.id: capture(gear); break;
                    case b::down::left     .id: capture(gear); proceed<PROT>(gear, left, true); break;
                    case b::down::middle   .id: capture(gear); proceed<PROT>(gear, mddl, true); break;
                    case b::down::right    .id: capture(gear); proceed<PROT>(gear, rght, true); break;
                    // Release
                    case b::up::leftright.id:   release(gear); break;
                    case b::up::left     .id:   release(gear); proceed<PROT>(gear, up_left); break;
                    case b::up::middle   .id:   release(gear); proceed<PROT>(gear, up_mddl); break;
                    case b::up::right    .id:   release(gear); proceed<PROT>(gear, up_rght); break;
                    // Wheel
                    case m::scroll::up  .id: proceed<PROT>(gear, wheel_up, true); break;
                    case m::scroll::down.id: proceed<PROT>(gear, wheel_dn, true); break;
                    // Gone
                    case hids::events::die.id:
                        release(gear);
                        if (auto buttons = gear.buttons())
                        {
                            // Release pressed mouse buttons.
                            if (buttons | sysmouse::left)   proceed<PROT>(gear, up_left);
                            if (buttons | sysmouse::middle) proceed<PROT>(gear, up_mddl);
                            if (buttons | sysmouse::right)  proceed<PROT>(gear, up_rght);
                        }
                        break;
                    default:
                        break;
                }
            }
        };

        // term: Keyboard focus tracking functionality.
        struct f_tracking
        {
            f_tracking(term& owner)
                : owner{ owner },
                  state{ faux  }
            { }

            operator bool () { return token.operator bool(); }
            void set(bool enable)
            {
                if (enable)
                {
                    if (!token) // Do not subscribe if it is already subscribed.
                    {
                        owner.SUBMIT_T(tier::release, hids::events::notify::keybd::any, token, gear)
                        {
                            switch (owner.bell::protos<tier::release>())
                            {
                                case hids::events::notify::keybd::got .id: queue.fcs(true); break;
                                case hids::events::notify::keybd::lost.id: queue.fcs(faux); break;
                                default: break;
                            }
                            owner.answer(queue);
                        };
                    }
                }
                else token.reset();
            }
        private:
            term&       owner; // f_tracking: Terminal object reference.
            hook        token; // f_tracking: Subscription token.
            ansi::esc   queue; // f_tracking: Buffer.
            bool        state; // f_tracking: Current focus state.
        };

        // term: Terminal title tracking functionality.
        struct w_tracking
        {
            term&                             owner; // w_tracking: Terminal object reference.
            std::map<text, text>              props;
            std::map<text, std::vector<text>> stack;
            ansi::esc                         queue;

            w_tracking(term& owner)
                : owner{ owner }
            { }
            // w_tracking: Get terminal window property.
            auto& get(text const& property)
            {
                return props[property];
            }
            // w_tracking: Set terminal window property.
            void set(text const& property, view txt)
            {
                static auto jet_left = ansi::jet(bias::left);
                owner.target->flush();
                if (property == ansi::OSC_LABEL_TITLE)
                {
                                  props[ansi::OSC_LABEL] = txt;
                    auto& utf8 = (props[ansi::OSC_TITLE] = txt);
                    utf8 = jet_left + utf8;
                    owner.base::riseup<tier::preview>(e2::form::prop::header, utf8);
                }
                else
                {
                    auto& utf8 = (props[property] = txt);
                    if (property == ansi::OSC_TITLE)
                    {
                        utf8 = jet_left + utf8;
                        owner.base::riseup<tier::preview>(e2::form::prop::header, utf8);
                    }
                }
            }
            // w_tracking: CSI n n  Device status report (DSR).
            void report(iota n)
            {
                switch(n)
                {
                    default:
                    case 6: queue.report(owner.target->coord); break;
                    case 5: queue.add("OK");                   break;
                    case-1: queue.add("VT420");                break;
                }
                owner.answer(queue);
            }
            // w_tracking: CSI n c  Primary device attributes (DA1).
            void device(iota n)
            {
                switch(n)
                {
                    default:
                        queue.add("\033[?1;2c"); break;
                }
                owner.answer(queue);
            }
            // w_tracking: Manage terminal window props (XTWINOPS).
            void manage(fifo& q)
            {
                owner.target->flush();
                static constexpr iota all_title = 0;  // Sub commands.
                static constexpr iota label     = 1;  // Sub commands.
                static constexpr iota title     = 2;  // Sub commands.
                static constexpr iota set_winsz = 8;  // Set window size in characters.
                static constexpr iota maximize  = 9;  // Toggle maximize/restore.
                static constexpr iota full_scrn = 10; // Toggle fullscreen mode (todo: hide menu).
                static constexpr iota get_label = 20; // Report icon   label. (Report as OSC L label ST).
                static constexpr iota get_title = 21; // Report window title. (Report as OSC l title ST).
                static constexpr iota put_stack = 22; // Push icon label and window title to   stack.
                static constexpr iota pop_stack = 23; // Pop  icon label and window title from stack.
                switch (auto option = q(0))
                {
                    case maximize:
                    case full_scrn:
                        owner.window_resize(dot_00);
                        break;
                    case set_winsz:
                    {
                        twod winsz;
                        winsz.y = q(-1);
                        winsz.x = q(-1);
                        owner.window_resize(winsz);
                        break;
                    }
                    case get_label: owner.answer(queue.osc(ansi::OSC_LABEL_REPORT, "")); break; // Return an empty string for security reasons
                    case get_title: owner.answer(queue.osc(ansi::OSC_TITLE_REPORT, "")); break;
                    case put_stack:
                    {
                        auto push = [&](auto const& property){
                            stack[property].push_back(props[property]);
                        };
                        switch (q(all_title))
                        {
                            case title:     push(ansi::OSC_TITLE); break;
                            case label:     push(ansi::OSC_LABEL); break;
                            case all_title: push(ansi::OSC_TITLE);
                                            push(ansi::OSC_LABEL); break;
                            default: break;
                        }
                        break;
                    }
                    case pop_stack:
                    {
                        auto pop = [&](auto const& property){
                            auto& s = stack[property];
                            if (s.size())
                            {
                                set(property, s.back());
                                s.pop_back();
                            }
                        };
                        switch (q(all_title))
                        {
                            case title:     pop(ansi::OSC_TITLE); break;
                            case label:     pop(ansi::OSC_LABEL); break;
                            case all_title: pop(ansi::OSC_TITLE);
                                            pop(ansi::OSC_LABEL); break;
                            default: break;
                        }
                        break;
                    }
                    default:
                        log("CSI ", option, "... t (XTWINOPS) is not supported");
                        break;
                }
            }
        };

        // term: Terminal 16/256 color palette tracking functionality.
        struct c_tracking
        {
            using pals = std::remove_const_t<decltype(rgba::color256)>;
            using func = std::unordered_map<text, std::function<void(view)>>;

            term& owner; // c_tracking: Terminal object reference.
            pals  color; // c_tracking: 16/256 colors palette.
            func  procs; // c_tracking: Handlers.

            void reset()
            {
                std::copy(std::begin(rgba::color256), std::end(rgba::color256), std::begin(color));
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
                log(" Not supported: OSC=", property, " DATA=", data, " SIZE=", data.length(), " HEX=", utf::to_hex(data));
            }

            c_tracking(term& owner)
                : owner{ owner }
            {
                reset();
                procs[ansi::OSC_LINUX_COLOR] = [&](view data) // ESC ] P Nrrggbb
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
                procs[ansi::OSC_RESET_COLOR] = [&](view data) // ESC ] 104 ; 0; 1;...
                {
                    auto empty = true;
                    while(data.length())
                    {
                        utf::trim_front_if(data, [](char c){ return c >= '0' && c <= '9'; });
                        if (auto v = utf::to_int(data))
                        {
                            auto n = std::clamp(v.value(), 0, 255);
                            color[n] = rgba::color256[n];
                            empty = faux;
                        }
                    }
                    if (empty) reset();
                };
                procs[ansi::OSC_SET_PALETTE] = [&](view data) // ESC ] 4 ; 0;rgb:00/00/00;1;rgb:00/00/00;...
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
                    if (fails) notsupported(ansi::OSC_SET_PALETTE, data);
                };
                procs[ansi::OSC_LINUX_RESET] = [&](view data) // ESC ] R
                {
                    reset();
                };
                procs[ansi::OSC_SET_FGCOLOR] = [&](view data) // ESC ] 10 ;rgb:00/00/00
                {
                    if (auto r = record(data))
                    {
                        owner.target->brush.sfg(r.value());
                    }
                    else notsupported(ansi::OSC_SET_FGCOLOR, data);
                };
                procs[ansi::OSC_SET_BGCOLOR] = [&](view data) // ESC ] 11 ;rgb:00/00/00
                {
                    if (auto r = record(data))
                    {
                        owner.target->brush.sbg(r.value());
                    }
                    else notsupported(ansi::OSC_SET_BGCOLOR, data);
                };
                procs[ansi::OSC_RESET_FGCLR] = [&](view data)
                {
                    owner.target->brush.sfg(0);
                };
                procs[ansi::OSC_RESET_BGCLR] = [&](view data)
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
                else log(" Not supported: OSC=", property, " DATA=", data, " HEX=", utf::to_hex(data));
            }
            void fgc(tint c) { owner.target->brush.fgc(color[c]); }
            void bgc(tint c) { owner.target->brush.bgc(color[c]); }
        };

        // term: Generic terminal buffer.
        struct bufferbase
            : public ansi::parser
        {
            template<class T>
            static void parser_config(T& vt)
            {
                using namespace netxs::ansi;
                vt.csier.table_space[CSI_SPC_SRC] = VT_PROC{ p->na("CSI n SP A  Shift right n columns(s)."); }; // CSI n SP A  Shift right n columns(s).
                vt.csier.table_space[CSI_SPC_SLC] = VT_PROC{ p->na("CSI n SP @  Shift left  n columns(s)."); }; // CSI n SP @  Shift left n columns(s).
                vt.csier.table_space[CSI_SPC_CST] = VT_PROC{ p->owner.cursor.style(q(1)); }; // CSI n SP q  Set cursor style (DECSCUSR).
                vt.csier.table_hash [CSI_HSH_SCP] = VT_PROC{ p->na("CSI n # P  Push current palette colors onto stack. n default is 0."); }; // CSI n # P  Push current palette colors onto stack. n default is 0.
                vt.csier.table_hash [CSI_HSH_RCP] = VT_PROC{ p->na("CSI n # Q  Pop  current palette colors onto stack. n default is 0."); }; // CSI n # Q  Pop  current palette colors onto stack. n default is 0.
                vt.csier.table_excl [CSI_EXL_RST] = VT_PROC{ p->owner.decstr( ); }; // CSI ! p  Soft terminal reset (DECSTR)

                vt.csier.table[CSI_SGR][SGR_FG_BLK   ] = VT_PROC{ p->owner.ctrack.fgc(tint::blackdk  ); };
                vt.csier.table[CSI_SGR][SGR_FG_RED   ] = VT_PROC{ p->owner.ctrack.fgc(tint::reddk    ); };
                vt.csier.table[CSI_SGR][SGR_FG_GRN   ] = VT_PROC{ p->owner.ctrack.fgc(tint::greendk  ); };
                vt.csier.table[CSI_SGR][SGR_FG_YLW   ] = VT_PROC{ p->owner.ctrack.fgc(tint::yellowdk ); };
                vt.csier.table[CSI_SGR][SGR_FG_BLU   ] = VT_PROC{ p->owner.ctrack.fgc(tint::bluedk   ); };
                vt.csier.table[CSI_SGR][SGR_FG_MGT   ] = VT_PROC{ p->owner.ctrack.fgc(tint::magentadk); };
                vt.csier.table[CSI_SGR][SGR_FG_CYN   ] = VT_PROC{ p->owner.ctrack.fgc(tint::cyandk   ); };
                vt.csier.table[CSI_SGR][SGR_FG_WHT   ] = VT_PROC{ p->owner.ctrack.fgc(tint::whitedk  ); };
                vt.csier.table[CSI_SGR][SGR_FG_BLK_LT] = VT_PROC{ p->owner.ctrack.fgc(tint::blacklt  ); };
                vt.csier.table[CSI_SGR][SGR_FG_RED_LT] = VT_PROC{ p->owner.ctrack.fgc(tint::redlt    ); };
                vt.csier.table[CSI_SGR][SGR_FG_GRN_LT] = VT_PROC{ p->owner.ctrack.fgc(tint::greenlt  ); };
                vt.csier.table[CSI_SGR][SGR_FG_YLW_LT] = VT_PROC{ p->owner.ctrack.fgc(tint::yellowlt ); };
                vt.csier.table[CSI_SGR][SGR_FG_BLU_LT] = VT_PROC{ p->owner.ctrack.fgc(tint::bluelt   ); };
                vt.csier.table[CSI_SGR][SGR_FG_MGT_LT] = VT_PROC{ p->owner.ctrack.fgc(tint::magentalt); };
                vt.csier.table[CSI_SGR][SGR_FG_CYN_LT] = VT_PROC{ p->owner.ctrack.fgc(tint::cyanlt   ); };
                vt.csier.table[CSI_SGR][SGR_FG_WHT_LT] = VT_PROC{ p->owner.ctrack.fgc(tint::whitelt  ); };
                vt.csier.table[CSI_SGR][SGR_BG_BLK   ] = VT_PROC{ p->owner.ctrack.bgc(tint::blackdk  ); };
                vt.csier.table[CSI_SGR][SGR_BG_RED   ] = VT_PROC{ p->owner.ctrack.bgc(tint::reddk    ); };
                vt.csier.table[CSI_SGR][SGR_BG_GRN   ] = VT_PROC{ p->owner.ctrack.bgc(tint::greendk  ); };
                vt.csier.table[CSI_SGR][SGR_BG_YLW   ] = VT_PROC{ p->owner.ctrack.bgc(tint::yellowdk ); };
                vt.csier.table[CSI_SGR][SGR_BG_BLU   ] = VT_PROC{ p->owner.ctrack.bgc(tint::bluedk   ); };
                vt.csier.table[CSI_SGR][SGR_BG_MGT   ] = VT_PROC{ p->owner.ctrack.bgc(tint::magentadk); };
                vt.csier.table[CSI_SGR][SGR_BG_CYN   ] = VT_PROC{ p->owner.ctrack.bgc(tint::cyandk   ); };
                vt.csier.table[CSI_SGR][SGR_BG_WHT   ] = VT_PROC{ p->owner.ctrack.bgc(tint::whitedk  ); };
                vt.csier.table[CSI_SGR][SGR_BG_BLK_LT] = VT_PROC{ p->owner.ctrack.bgc(tint::blacklt  ); };
                vt.csier.table[CSI_SGR][SGR_BG_RED_LT] = VT_PROC{ p->owner.ctrack.bgc(tint::redlt    ); };
                vt.csier.table[CSI_SGR][SGR_BG_GRN_LT] = VT_PROC{ p->owner.ctrack.bgc(tint::greenlt  ); };
                vt.csier.table[CSI_SGR][SGR_BG_YLW_LT] = VT_PROC{ p->owner.ctrack.bgc(tint::yellowlt ); };
                vt.csier.table[CSI_SGR][SGR_BG_BLU_LT] = VT_PROC{ p->owner.ctrack.bgc(tint::bluelt   ); };
                vt.csier.table[CSI_SGR][SGR_BG_MGT_LT] = VT_PROC{ p->owner.ctrack.bgc(tint::magentalt); };
                vt.csier.table[CSI_SGR][SGR_BG_CYN_LT] = VT_PROC{ p->owner.ctrack.bgc(tint::cyanlt   ); };
                vt.csier.table[CSI_SGR][SGR_BG_WHT_LT] = VT_PROC{ p->owner.ctrack.bgc(tint::whitelt  ); };

                vt.csier.table[CSI_CUU] = VT_PROC{ p->up (q(1)); };  // CSI n A  (CUU)
                vt.csier.table[CSI_CUD] = VT_PROC{ p->dn (q(1)); };  // CSI n B  (CUD)
                vt.csier.table[CSI_CUF] = VT_PROC{ p->cuf(q(1)); };  // CSI n C  (CUF)
                vt.csier.table[CSI_CUB] = VT_PROC{ p->cub(q(1)); };  // CSI n D  (CUB)

                vt.csier.table[CSI_CHT]           = VT_PROC{ p->tab( q(1)); };  // CSI n I  Caret forward  n tabs, default n=1.
                vt.csier.table[CSI_CBT]           = VT_PROC{ p->tab(-q(1)); };  // CSI n Z  Caret backward n tabs, default n=1.
                vt.csier.table[CSI_TBC]           = VT_PROC{ p->tbc( q(0)); };  // CSI n g  Clear tabstops, default n=0.
                vt.csier.table_quest[CSI_QST_RTB] = VT_PROC{ p->rtb(     ); };  // CSI ? W  Reset tabstops to the 8 column defaults.
                vt.intro[ctrl::ESC][ESC_HTS]      = VT_PROC{ p->stb(     ); };  // ESC H    Place tabstop at the current column.

                vt.csier.table[CSI_CUD2]= VT_PROC{ p->dn ( q(1)); };  // CSI n e  Vertical position relative. Move cursor down (VPR).

                vt.csier.table[CSI_CNL] = VT_PROC{ p->cr (); p->dn (q(1)); };  // CSI n E  Move n lines down and to the leftmost column.
                vt.csier.table[CSI_CPL] = VT_PROC{ p->cr (); p->up (q(1)); };  // CSI n F  Move n lines up   and to the leftmost column.
                vt.csier.table[CSI_CHX] = VT_PROC{ p->chx( q(1)); };  // CSI n G  Move cursor hz absolute.
                vt.csier.table[CSI_CHY] = VT_PROC{ p->chy( q(1)); };  // CSI n d  Move cursor vt absolute.
                vt.csier.table[CSI_CUP] = VT_PROC{ p->cup( q   ); };  // CSI y ; x H (1-based)
                vt.csier.table[CSI_HVP] = VT_PROC{ p->cup( q   ); };  // CSI y ; x f (1-based)

                vt.csier.table[CSI_DCH] = VT_PROC{ p->dch( q(1)); };  // CSI n P  Delete n chars (DCH).
                vt.csier.table[CSI_ECH] = VT_PROC{ p->ech( q(1)); };  // CSI n X  Erase n chars (ECH).
                vt.csier.table[CSI_ICH] = VT_PROC{ p->ins( q(1)); };  // CSI n @  Insert n chars (ICH).

                vt.csier.table[CSI__ED] = VT_PROC{ p->ed ( q(0)); };  // CSI n J
                vt.csier.table[CSI__EL] = VT_PROC{ p->el ( q(0)); };  // CSI n K
                vt.csier.table[CSI__IL] = VT_PROC{ p->il ( q(1)); };  // CSI n L  Insert n lines (IL).
                vt.csier.table[CSI__DL] = VT_PROC{ p->dl ( q(1)); };  // CSI n M  Delete n lines (DL).
                vt.csier.table[CSI__SD] = VT_PROC{ p->scl( q(1)); };  // CSI n T  Scroll down by n lines, scrolled out lines are lost.
                vt.csier.table[CSI__SU] = VT_PROC{ p->scl(-q(1)); };  // CSI n S  Scroll   up by n lines, scrolled out lines are pushed to the scrollback.
                vt.csier.table[CSI_SCP] = VT_PROC{ p->scp(     ); };  // CSI   s  Save cursor position.
                vt.csier.table[CSI_RCP] = VT_PROC{ p->rcp(     ); };  // CSI   u  Restore cursor position.

                vt.csier.table[DECSTBM] = VT_PROC{ p->scr( q   ); };  // CSI r; b r  Set scrolling region (t/b: top+bottom).

                vt.csier.table[CSI_WIN] = VT_PROC{ p->owner.wtrack.manage(q   ); };  // CSI n;m;k t  Terminal window options (XTWINOPS).
                vt.csier.table[CSI_DSR] = VT_PROC{ p->owner.wtrack.report(q(6)); };  // CSI n n  Device status report (DSR).
                vt.csier.table[CSI_PDA] = VT_PROC{ p->owner.wtrack.device(q(0)); };  // CSI n c  Send device attributes (Primary DA).

                vt.csier.table[CSI_CCC][CCC_SBS] = VT_PROC{ p->owner.sbsize(q); };  // CCC_SBS: Set scrollback size.
                vt.csier.table[CSI_CCC][CCC_EXT] = VT_PROC{ p->owner.native(q(1)); };          // CCC_EXT: Setup extended functionality.
                vt.csier.table[CSI_CCC][CCC_RST] = VT_PROC{ p->style.glb(); p->style.wrp(deco::defwrp); };  // fx_ccc_rst

                vt.intro[ctrl::ESC][ESC_IND   ] = VT_PROC{ p->lf(1); };          // ESC D  Index. Caret down and scroll if needed (IND).
                vt.intro[ctrl::ESC][ESC_IR    ] = VT_PROC{ p->ri (); };          // ESC M  Reverse index (RI).
                vt.intro[ctrl::ESC][ESC_SC    ] = VT_PROC{ p->scp(); };          // ESC 7  (same as CSI s) Save cursor position.
                vt.intro[ctrl::ESC][ESC_RC    ] = VT_PROC{ p->rcp(); };          // ESC 8  (same as CSI u) Restore cursor position.
                vt.intro[ctrl::ESC][ESC_RIS   ] = VT_PROC{ p->owner.decstr(); }; // ESC c  Reset to initial state (same as DECSTR).
                vt.intro[ctrl::ESC][ESC_NEL   ] = VT_PROC{ p->cr(); p->dn(1); }; // ESC E  Move cursor down and CR. Same as CSI 1 E
                vt.intro[ctrl::ESC][ESC_DECDHL] = VT_PROC{ p->dhl(q); };         // ESC # ...  ESC # 3, ESC # 4, ESC # 5, ESC # 6, ESC # 8

                vt.intro[ctrl::ESC][ESC_APC   ] = VT_PROC{ p->msg(ESC_APC, q); };    // ESC _ ... ST  APC.
                vt.intro[ctrl::ESC][ESC_DSC   ] = VT_PROC{ p->msg(ESC_DSC, q); };    // ESC P ... ST  DSC.
                vt.intro[ctrl::ESC][ESC_SOS   ] = VT_PROC{ p->msg(ESC_SOS, q); };    // ESC X ... ST  SOS.
                vt.intro[ctrl::ESC][ESC_PM    ] = VT_PROC{ p->msg(ESC_PM , q); };    // ESC ^ ... ST  PM.

                vt.intro[ctrl::BS ] = VT_PROC{ p->cub(q.pop_all(ctrl::BS )); };
                vt.intro[ctrl::DEL] = VT_PROC{ p->del(q.pop_all(ctrl::DEL)); };
                vt.intro[ctrl::TAB] = VT_PROC{ p->tab(q.pop_all(ctrl::TAB)); };
                vt.intro[ctrl::EOL] = VT_PROC{ p->lf (q.pop_all(ctrl::EOL)); }; // LF
                vt.intro[ctrl::VT ] = VT_PROC{ p->lf (q.pop_all(ctrl::VT )); }; // VT same as LF
                vt.intro[ctrl::CR ] = VT_PROC{ p->cr ();                     }; // CR

                vt.csier.table_quest[DECSET] = VT_PROC{ p->owner.decset(q); };
                vt.csier.table_quest[DECRST] = VT_PROC{ p->owner.decrst(q); };

                vt.oscer[OSC_LABEL_TITLE] = VT_PROC{ p->owner.wtrack.set(OSC_LABEL_TITLE, q); };
                vt.oscer[OSC_LABEL      ] = VT_PROC{ p->owner.wtrack.set(OSC_LABEL,       q); };
                vt.oscer[OSC_TITLE      ] = VT_PROC{ p->owner.wtrack.set(OSC_TITLE,       q); };
                vt.oscer[OSC_XPROP      ] = VT_PROC{ p->owner.wtrack.set(OSC_XPROP,       q); };
                vt.oscer[OSC_LINUX_COLOR] = VT_PROC{ p->owner.ctrack.set(OSC_LINUX_COLOR, q); };
                vt.oscer[OSC_LINUX_RESET] = VT_PROC{ p->owner.ctrack.set(OSC_LINUX_RESET, q); };
                vt.oscer[OSC_SET_PALETTE] = VT_PROC{ p->owner.ctrack.set(OSC_SET_PALETTE, q); };
                vt.oscer[OSC_SET_FGCOLOR] = VT_PROC{ p->owner.ctrack.set(OSC_SET_FGCOLOR, q); };
                vt.oscer[OSC_SET_BGCOLOR] = VT_PROC{ p->owner.ctrack.set(OSC_SET_BGCOLOR, q); };
                vt.oscer[OSC_RESET_COLOR] = VT_PROC{ p->owner.ctrack.set(OSC_RESET_COLOR, q); };
                vt.oscer[OSC_RESET_FGCLR] = VT_PROC{ p->owner.ctrack.set(OSC_RESET_FGCLR, q); };
                vt.oscer[OSC_RESET_BGCLR] = VT_PROC{ p->owner.ctrack.set(OSC_RESET_BGCLR, q); };

                // Log all unimplemented CSI commands.
                for (auto i = 0; i < 0x100; ++i)
                {
                    auto& proc = vt.csier.table[i];
                    if (!proc)
                    {
                        proc = [i](auto& q, auto& p) { p->not_implemented_CSI(i, q); };
                    }
                }
                auto& esc_lookup = vt.intro[ctrl::ESC];
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

            using tabs = std::vector<std::pair<iota, iota>>; // Pairs of forward and reverse tabstops index.
            struct redo
            {
                using mark = ansi::mark;
                deco style{}; // Parser style state.
                mark brush{}; // Parser brush state.
                twod coord{}; // Screen coord state.
                bool decom{}; // Origin mode  state.
            };

            term& owner; // bufferbase: Terminal object reference.
            twod  panel; // bufferbase: Viewport size.
            twod  coord; // bufferbase: Viewport cursor position; 0-based.
            redo  saved; // bufferbase: Saved cursor position and rendition state.
            iota  sctop; // bufferbase: Precalculated scrolling region top    height.
            iota  scend; // bufferbase: Precalculated scrolling region bottom height.
            iota  y_top; // bufferbase: Precalculated 0-based scrolling region top    vertical pos.
            iota  y_end; // bufferbase: Precalculated 0-based scrolling region bottom vertical pos.
            iota  n_top; // bufferbase: Original      1-based scrolling region top    vertical pos (use 0 if it is not set).
            iota  n_end; // bufferbase: Original      1-based scrolling region bottom vertical pos (use 0 if it is not set).
            tabs  stops; // bufferbase: Tabstop index.
            bool  notab; // bufferbase: Tabstop index is cleared.
            bool  decom; // bufferbase: Origin mode.

            bufferbase(term& master)
                : owner{ master },
                  panel{ dot_11 },
                  coord{ dot_00 },
                  sctop{ 0      },
                  scend{ 0      },
                  y_top{ 0      },
                  y_end{ 0      },
                  n_top{ 0      },
                  n_end{ 0      },
                  stops{ {0, 0} },
                  notab{ faux   },
                  decom{ faux   }
            {
                parser::style = ansi::def_style;
            }

            virtual void output(face& canvas)                        = 0;
            virtual void scroll_region(iota top, iota end, iota n, bool use_scrollback)  = 0;
            virtual bool recalc_pads(side& oversz)                   = 0;
            virtual iota height()                                    = 0;
            virtual void del_above()                                 = 0;
            virtual void del_below()                                 = 0;
            virtual iota get_size() const                            = 0;
            virtual iota get_peak() const                            = 0;
            virtual iota get_step() const                            = 0;
                    auto get_view() const { return panel; }

            // bufferbase: . // size in cells
    //virtual bool is_need_to_correct()
            //{
            //    return faux;
            //}
            // bufferbase: .
    virtual iota get_basis()
            {
                return 0;
            }
            // bufferbase: .
    virtual iota get_slide()
            {
                return 0;
            }
            // bufferbase: .
    virtual iota set_slide(iota)
            {
                return 0;
            }
            // bufferbase: .
            void update_region()
            {
                sctop = std::max(0, n_top - 1);
                scend = n_end != 0 ? std::max(1, panel.y - n_end)
                                   : 0;

                auto y_max = panel.y - 1;
                y_end = std::clamp(y_max - scend, 0, y_max);
                y_top = std::clamp(sctop        , 0, y_end);
            }
            // bufferbase: .
    virtual void resize_viewport(twod const& new_sz)
            {
                panel = std::max(new_sz, dot_11);
                resize_tabstops(panel.x);
                update_region();
            }
            // bufferbase: Reset coord and set the scrolling region using 1-based top and bottom. Use 0 to reset.
    virtual void set_scroll_region(iota top, iota bottom)
            {
                // Sanity check.
                top    = std::clamp(top   , 0, panel.y);
                bottom = std::clamp(bottom, 0, panel.y);
                if (top    != 0 &&
                    //bottom != 0 && top > bottom) top = bottom; //todo Nobody respects that.
                    bottom != 0 && top >= bottom) top = bottom = 0;

                n_top = top    == 1       ? 0 : top;
                n_end = bottom == panel.y ? 0 : bottom;
                update_region();
                cup(dot_11);
            }
            // bufferbase: .
    virtual void set_coord(twod const& new_coord)
            {
                coord = new_coord;
            }
            // bufferbase: Return current 0-based cursor position in the viewport.
    virtual twod get_coord(twod const& origin)
            {
                return coord;
            }
            // bufferbase: Base-CSI contract (see ansi::csi_t).
            //             task(...), meta(...), data(...)
            void task(ansi::rule const& property)
            {
                parser::flush();
                log("bufferbase: locus extensions are not supported");
                //auto& cur_line = batch.current();
                //if (cur_line.busy())
                //{
                //    add_lines(1);
                //    batch.index(batch.length() - 1);
                //} 
                //batch->locus.push(property);
            }
            // bufferbase: .
    virtual void meta(deco const& old_style) override
            {
                if (parser::style.wrp() != old_style.wrp())
                {
                    auto status = parser::style.wrp() == wrap::none ? deco::defwrp
                                                                    : parser::style.wrp();
                    owner.SIGNAL(tier::release, ui::term::events::layout::wrapln, status);
                }
                if (parser::style.jet() != old_style.jet())
                {
                    auto status = parser::style.jet() == bias::none ? bias::left
                                                                    : parser::style.jet();
                    owner.SIGNAL(tier::release, ui::term::events::layout::align, status);
                }
            }
            // bufferbase: .
            template<class T>
            void na(T&& note)
            {
                log("not implemented: ", note);
            }
            void not_implemented_CSI(iota i, fifo& q)
            {
                text params;
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
                log("CSI ", params, " ", (unsigned char)i, "(", i, ") is not implemented");
            }
            void not_implemented_ESC(iota c, qiew& q)
            {
                switch (c)
                {
                    // Unexpected
                    case ansi::ESC_CSI   :
                    case ansi::ESC_OCS   :
                    case ansi::ESC_DSC   :
                    case ansi::ESC_SOS   :
                    case ansi::ESC_PM    :
                    case ansi::ESC_APC   :
                    case ansi::ESC_ST    :
                        log("ESC ", (char)c, " (", c, ") is unexpected");
                        break;
                    // Unsupported ESC + byte + rest
                    case ansi::ESC_G0SET :
                    case ansi::ESC_G1SET :
                    case ansi::ESC_G2SET :
                    case ansi::ESC_G3SET :
                    case ansi::ESC_G1xSET:
                    case ansi::ESC_G2xSET:
                    case ansi::ESC_G3xSET:
                    case ansi::ESC_CTRL  :
                    case ansi::ESC_DECDHL:
                    case ansi::ESC_CHRSET:
                    {
                        if (!q) log("ESC ", (char)c, " (", c, ") is incomplete");
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
                                log("ESC ", (char)c, " ", (char)b, " (", c, " ", b, ") is unsupported");
                                break;
                            case '%':
                            case '"':
                            {
                                if (q.size() < 2)
                                {
                                    if (q) q.pop_front();
                                    log("ESC ", (char)c, " ", (char)b, " (", c, " ", b, ") is incomplete");
                                }
                                else
                                {
                                     auto d = q.front();
                                     q.pop_front();
                                     log("ESC ", (char)c, " ", (char)b, " ", (char)d, " (", c, " ", b, " ", d, ") is unsupported");
                                }
                                break;
                            }
                            default:
                                log("ESC ", (char)c, " ", (char)b, " (", c, " ", b, ") is unknown");
                                break;
                        }
                        break;
                    }
                    // Unsupported ESC + byte
                    case ansi::ESC_DELIM :
                    case ansi::ESC_KEY_A :
                    case ansi::ESC_KEY_N :
                    case ansi::ESC_DECBI :
                    case ansi::ESC_DECFI :
                    case ansi::ESC_SC    :
                    case ansi::ESC_RC    :
                    case ansi::ESC_HTS   :
                    case ansi::ESC_NEL   :
                    case ansi::ESC_CLB   :
                    case ansi::ESC_IND   :
                    case ansi::ESC_IR    :
                    case ansi::ESC_RIS   :
                    case ansi::ESC_MEMLK :
                    case ansi::ESC_MUNLK :
                    case ansi::ESC_LS2   :
                    case ansi::ESC_LS3   :
                    case ansi::ESC_LS1R  :
                    case ansi::ESC_LS2R  :
                    case ansi::ESC_LS3R  :
                    case ansi::ESC_SS3   :
                    case ansi::ESC_SS2   :
                    case ansi::ESC_SPA   :
                    case ansi::ESC_EPA   :
                    case ansi::ESC_RID   :
                        log("ESC ", (char)c, " (", c, ") is unsupported");
                        break;
                    default:
                        log("ESC ", (char)c, " (", c, ") is unknown");
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
                        log("ESC #  is unexpected");
                        break;
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                        log("ESC # ", (char)c, " (", c, ") is unsupported");
                        break;
                    case '8':
                    {
                        set_coord(dot_00);
                        auto y = 0;
                        while (++y <= panel.y)// Fill viewport with 'E'.
                        {
                            chy(y);
                            ech(panel.x, 'E');
                        }
                        set_coord(dot_00);
                        break;
                    }
                    default:
                        log("ESC # ", (char)c, " (", c, ") is unknown");
                        break;
                }                
            }
            void msg(iota c, qiew& q)
            {
                parser::flush();
                text data;
                while (q)
                {
                    auto c = q.front();
                    data.push_back(c);
                    q.pop_front();
                         if (c == ansi::C0_BEL) break;
                    else if (c == ansi::C0_ESC)
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
                log("Unsupported Message/Command: '\\e", (char)c, utf::debase<faux>(data), "'");
            }
            // bufferbase: .
    virtual void clear_all()
            {
                parser::state = {};
                decom = faux;
                rtb();
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
            void clear_tabstops()
            {
                notab = true;
                auto auto_tabs = std::pair{ -panel.x, 0 }; // Negative means auto, not custom.
                stops.assign(panel.x, auto_tabs);
            }
            // bufferbase: Resize tabstop index.
            void resize_tabstops(iota new_size, bool forced = faux)
            {
                auto size = static_cast<iota>(stops.size());
                if (!forced && new_size <= size) return;

                auto back = stops.back();
                auto last_size = back.first > 0 ? 0 // Custom tabstop -- don't touch it.
                                                : -back.first - back.second;
                auto last_stop = forced ? 0
                                        : size - last_size;
                stops.resize(last_stop); // Trim.

                if (notab) // Preserve existing tabstops.
                {
                    auto auto_tabs = std::pair{ -new_size, last_stop };
                    stops.resize(new_size, auto_tabs);
                }
                else // Add additional default tabstops.
                {
                    stops.reserve(new_size);
                    auto step = term::def_tablen;
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
                if (coord.x <= 0 || coord.x > term::max_length) return;
                resize_tabstops(coord.x);
                auto  coor = coord.x - 1;
                auto  head = stops.begin();
                auto  tail = stops.begin() + coor;
                auto& last = tail->first;
                if (coord.x != last)
                {
                    auto size = stops.size();
                    auto base = last;
                    last = coord.x;
                    while (head != tail)
                    {
                        auto& last = (--tail)->first;
                        if (last == base) last = coord.x;
                        else break;
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
                            else break;
                        }
                    }
                }
            }
            // bufferbase: (see CSI 0 g) Remove tabstop at the current cursor posistion.
            void remove_tabstop()
            {
                auto  size = static_cast<iota>(stops.size());
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
                    else break;
                }

                tail = stops.end();
                base = stop.second;
                main = item.second;
                stop.second = main;
                while (++back != tail)
                {
                    auto& item = back->second;
                    if (base == std::abs(item)) item = main;
                    else break;
                }
            }
            // bufferbase: CSI ? W  Reset tabstops to the 8 (todo hardcoded?) column defaults.
            void rtb()
            {
                notab = faux;
                resize_tabstops(panel.x, true);
            }
            // bufferbase: Horizontal tab implementation.
            template<bool FWD>
            void tab_impl(auto size)
            {
                if constexpr (FWD)
                {
                    auto x = std::clamp(coord.x, 0, size - 1);
                    if (coord.x == x) coord.x = std::abs(stops[x].first);
                    else
                    {
                        coord.x += notab ? term::def_tablen
                                         : term::def_tablen - coord.x % term::def_tablen;
                    }
                }
                else
                {
                    auto x = std::clamp(coord.x, 1, size);
                    if (coord.x == x) coord.x = stops[x - 1].second;
                    else
                    {
                        coord.x -= notab ? term::def_tablen
                                         :(term::def_tablen + coord.x - 1) % term::def_tablen + 1;
                    }
                }
            }
            // bufferbase: TAB  Horizontal tab.
    virtual void tab(iota n)
            {
                parser::flush();
                auto size = static_cast<iota>(stops.size());
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
            void tbc(iota n)
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
    virtual void scl(iota n)
            {
                parser::flush();
                scroll_region(y_top, y_end, n, n > 0 ? faux : true);
            }
            // bufferbase: CSI n L  Insert n lines. Place cursor to the begining of the current.
    virtual void il(iota n)
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
            // bufferbase: CSI n M Delete n lines. Place cursor to the begining of the current.
    virtual void dl(iota n)
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
            // bufferbase: CSI t;b r - Set scrolling region (t/b: top+bottom).
            void scr(fifo& queue)
            {
                auto top = queue(0);
                auto end = queue(0);
                set_scroll_region(top, end);
            }
            // bufferbase: CSI n @  ICH. Insert n blanks after cursor. Don't change cursor pos.
    virtual void ins(iota n) = 0;
            // bufferbase: Shift left n columns(s).
            void shl(iota n)
            {
                log("bufferbase: SHL(n=", n, ") is not implemented.");
            }
            // bufferbase: CSI n X  Erase/put n chars after cursor. Don't change cursor pos.
    virtual void ech(iota n, char c = whitespace) = 0;
            // bufferbase: CSI n P  Delete (not Erase) letters under the cursor.
    virtual void dch(iota n) = 0;
            // bufferbase: '\x7F'  Delete characters backwards.
            void del(iota n)
            {
                log("bufferbase: not implemented: '\\x7F' Delete characters backwards.");
            }
            // bufferbase: Move cursor forward by n.
    virtual void cuf(iota n)
            {
                parser::flush();
                if (n == 0) n = 1;
                coord.x += n;
            }
            // bufferbase: Move cursor backward by n.
    virtual void cub(iota n)
            {
                parser::flush();
                if (n == 0) n = 1;
                else if (coord.x == panel.x && parser::style.wrp() == wrap::on && n > 0) ++n;
                coord.x -= n;
            }
            // bufferbase: CSI n G  Absolute horizontal cursor position (1-based).
    virtual void chx(iota n)
            {
                parser::flush();
                coord.x = n - 1;
            }
            // bufferbase: CSI n d  Absolute vertical cursor position (1-based).
    virtual void chy(iota n)
            {
                parser::flush_data();
                --n;
                if (decom)
                {
                    coord.y = std::clamp(n + y_top, y_top, y_end);
                }
                else coord.y = std::clamp(n, 0, panel.y - 1);
            }
            // bufferbase: CSI y; x H/F  Caret position (1-based).
    virtual void cup(twod p)
            {
                parser::flush_data();
                p -= dot_11;
                coord.x = std::clamp(p.x, 0, panel.x - 1);
                if (decom)
                {
                    coord.y = std::clamp(p.y + y_top, y_top, y_end);
                }
                else coord.y  = std::clamp(p.y, 0, panel.y - 1);
            }
            // bufferbase: CSI y; x H/F  Caret position (1-based).
    virtual void cup(fifo& queue)
            {
                auto y = queue(1);
                auto x = queue(1);
                cup({ x, y });
            }
            // bufferbase: Move cursor up.
    virtual void up(iota n)
            {
                parser::flush_data();
                if (n == 0) n = 1;
                auto new_coord_y = coord.y - n;
                if (new_coord_y <  y_top
                     && coord.y >= y_top)
                {
                    coord.y = y_top;
                }
                else coord.y = std::clamp(new_coord_y, 0, panel.y - 1);
            }
            // bufferbase: Move cursor down.
    virtual void dn(iota n)
            {
                parser::flush_data();
                if (n == 0) n = 1;
                auto new_coord_y = coord.y + n;
                if (new_coord_y >  y_end
                     && coord.y <= y_end)
                {
                    coord.y = y_end;
                }
                else coord.y = std::clamp(new_coord_y, 0, panel.y - 1);
            }
            // bufferbase: Line feed. Index. Scroll region up if new_coord_y > end.
    virtual void lf(iota n)
            {
                parser::flush_data();
                auto new_coord_y = coord.y + n;
                if (new_coord_y >  y_end
                     && coord.y <= y_end)
                {
                    auto n = y_end - new_coord_y;
                    scroll_region(y_top, y_end, n, true);
                    coord.y = y_end;
                }
                else coord.y = std::clamp(new_coord_y, 0, panel.y - 1);
            }
            // bufferbase: '\r'  CR Cursor return. Go to home of visible line instead of home of paragraph.
    virtual void cr()
            {
                parser::flush();
                coord.x = 0;
            }
            // bufferbase: CSI n J  Erase display.
            void ed(iota n)
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
    virtual void el(iota n) = 0;

            bool update_status(term_state& status) const
            {
                bool changed = faux;
                if (status.size != get_size()) { changed = true; status.size = get_size(); }
                if (status.peak != get_peak()) { changed = true; status.peak = get_peak(); }
                if (status.step != get_step()) { changed = true; status.step = get_step(); }
                if (status.area != get_view()) { changed = true; status.area = get_view(); }
                return changed;
            }
        };

        // term: Alternate screen buffer implementation.
        struct alt_screen
            : public bufferbase
        {
            rich canvas; // alt_screen: Terminal screen.

            alt_screen(term& boss)
                : bufferbase{ boss }
            { }

            iota get_size() const override { return panel.y; }
            iota get_peak() const override { return panel.y; }
            iota get_step() const override { return 0;       }

            // alt_screen: Resize viewport.
            void resize_viewport(twod const& new_sz) override
            {
                bufferbase::resize_viewport(new_sz);
                coord = std::clamp(coord, dot_00, panel - dot_11);
                canvas.crop(panel);
            }
            // alt_screen: Return viewport height.
            iota height() override
            {
                return panel.y;
            }
            // alt_screen: Recalc left and right oversize (Always 0 for altbuf).
            bool recalc_pads(side& oversz) override
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
            static void _el(iota n, core& canvas, twod const& coord, twod const& panel, cell const& blank)
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
            void el(iota n) override
            {
                bufferbase::flush();
                _el(n, canvas, coord, panel, brush.spc());
            }
            // alt_screen: CSI n @  ICH. Insert n blanks after cursor. No wrap. Existing chars after cursor shifts to the right. Don't change cursor pos.
            void ins(iota n) override
            {
                bufferbase::flush();
                assert(coord.y < panel.y);
                assert(coord.x >= 0);
                auto blank = brush.spc();//.bgc(reddk).bga(0x7f);
                canvas.insert(coord, n, blank);
            }
            // alt_screen: CSI n P  Delete (not Erase) letters under the cursor.
            void dch(iota n) override
            {
                bufferbase::flush();
                auto blank = brush.spc();//.bgc(cyandk).bga(0x7f);
                canvas.cutoff(coord, n, blank);
            }
            // alt_screen: CSI n X  Erase/put n chars after cursor. Don't change cursor pos.
            void ech(iota n, char c = whitespace) override
            {
                parser::flush();
                auto blank = brush;
                blank.txt(c);
                canvas.splice(coord, n, blank);
            }
            // alt_screen: Parser callback.
            void data(iota count, grid const& proto) override
            {
                assert(coord.y >= 0 && coord.y < panel.y);

                auto saved = coord;
                coord.x += count;
                //todo apply line adjusting (necessity is not clear)
                if (coord.x <= panel.x)//todo styles! || ! curln.wrapped())
                {
                    auto n = std::min(count, panel.x - std::max(0, saved.x));
                    canvas.splice(saved, n, proto);
                }
                else
                {
                    coord.y += (coord.x + panel.x - 1) / panel.x - 1;
                    coord.x  = (coord.x           - 1) % panel.x + 1;

                    if (saved.y < y_top)
                    {
                        if (coord.y >= y_top)
                        {
                            auto n = coord.x + (coord.y - y_top) * panel.x;
                            count -= n;
                            set_coord({ 0, y_top });
                            data(n, proto); // Reversed fill using the last part of the proto.
                        }
                        auto data = proto.begin();
                        auto seek = saved.x + saved.y * panel.x;
                        auto dest = canvas.iter() + seek;
                        auto tail = dest + count;
                        rich::forward_fill_proc(data, dest, tail);
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
                        rich::reverse_fill_proc(data, dest, tail);
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
                        rich::unlimit_fill_proc(data, size, dest, tail, back);
                    }
                }
            }
            // alt_screen: Clear viewport.
            void clear_all() override
            {
                canvas.wipe();
                set_scroll_region(0, 0);
                bufferbase::clear_all();
            }
            // alt_screen: Render to the target.
            void output(face& target) override
            {
                auto full = target.full();
                canvas.move(full.coor);
                target.plot(canvas, cell::shaders::fuse);
            }
            // alt_screen: Remove all lines below except the current. "ED2 Erase viewport" keeps empty lines.
            void del_below() override
            {
                canvas.del_below(coord, brush.spare);
            }
            // alt_screen: Clear all lines from the viewport top line to the current line.
            void del_above() override
            {
                auto coorx = coord.x;
                if (coorx < panel.x) ++coord.x; // Clear the cell at the current position. See ED1 description.
                canvas.del_above(coord, brush.spare);
                coord.x = coorx;
            }
            // alt_screen: Shift by n the scroll region.
            void scroll_region(iota top, iota end, iota n, bool use_scrollback = faux) override
            {
                canvas.scroll(top, end + 1, n, brush.spare);
            }
            // alt_screen: Horizontal tab.
            void tab(iota n) override
            {
                bufferbase::tab(n);
                coord.x = std::clamp(coord.x, 0, panel.x - 1);
            }
        };

        // term: Scrollback buffer implementation.
        struct scroll_buf
            : public bufferbase
        {
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
                line(id_t newid, deco const& style, span const& dt, twod const& sz)
                    : rich { dt,sz },
                      index{ newid },
                      style{ style }
                { }
                line(id_t newid, deco const& style, cell const& blank, iota length)
                    : rich { blank, length },
                      index{ newid },
                      style{ style }
                { }

                line& operator = (line&&)      = default;
                line& operator = (line const&) = default;

                id_t index{};
                //ui64 accum{}; // size in cells
                deco style{};
                iota _size{};
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
                auto wrapped() const
                {
                    assert(_kind == style.get_kind());
                    return _kind == type::autowrap;
                }
                iota height(iota width) const
                {
                    auto len = length();
                    assert(_kind == style.get_kind());
                    return len > width
                        && wrapped() ? (len + width - 1) / width
                                     : 1;
                }
                auto to_txt() // for debug
                {
                    utf::text utf8;
                    each([&](cell& c){ utf8 += c.txt(); });
                    return utf8;
                }
            };
            struct index_item
            {
                using id_t = line::id_t;
                id_t index;
                iota start;
                iota width;
                index_item() = default;
                index_item(id_t index, iota start, iota width)
                    : index{ index },
                      start{ start },
                      width{ width }
                { }
            };

            using ring = generics::ring<std::vector<line>, true>;
            using indx = generics::ring<std::vector<index_item>>;

            struct buff : public ring
            {
                using ring::ring;
                using type = line::type;
                using maps = std::map<iota, iota>[type::count];

                iota caret{}; // buff: Current line caret horizontal position.
                iota vsize{}; // buff: Scrollback vertical size (height).
                iota width{}; // buff: Viewport width.
                //id_t taken{}; // buff: size in cells: The index up to which the cells are already counted.
                //ui64 accum{}; // buff: size in cells: The total number of cells in the stored lines.
                //bool dirty{}; // buff: size in cells: Indicator that not all available cells have been counted (lazy counting).
                //bool need_to_correct{}; // size in cells: .
                iota basis{}; // buff: Working area basis. Vertical position of O(0, 0) in the scrollback.
                iota slide{}; // buff: Viewport vertical position in the scrollback.
                maps sizes{}; // buff: Line length accounting database.

            id_t anchor_id{}; // the nearest id to the slide
            iota anchor_dy{}; // distance to the slide.
            bool round{};

                static constexpr id_t threshold = 1000;
                static constexpr ui64 lnpadding = 40; // Padding to improve accuracy.

                // buff: Decrease height.
                void dec_height(iota& vsize, type kind, iota size)
                {
                    if (size > width && kind == type::autowrap) vsize -= (size + width - 1) / width;
                    else                                        vsize -= 1;
                }
                // buff: Increase height.
                void add_height(iota& vsize, type kind, iota size)
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
                void set_width(iota new_width)
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
                void invite(type& kind, iota& size, type new_kind, iota new_size)
                {
                    ++sizes[new_kind][new_size];
                    add_height(vsize, new_kind, new_size);
                    size = new_size;
                    kind = new_kind;
                }
                // buff: Refresh scrollback height.
                void recalc(type& kind, iota& size, type new_kind, iota new_size)
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
                void undock(type kind, iota size)
                {
                    auto& lens = sizes[kind];
                    auto  iter = lens.find(size); assert(iter != lens.end());
                    auto  pool = --(*iter).second;
                    if (pool == 0) lens.erase(iter);
                    dec_height(vsize, kind, size);
                }
                // buff: Check buffer size.
                void check_size(twod const new_size)
                {
                    set_width(new_size.x);
                    if (ring::peak <= new_size.y)
                    {
                        static constexpr auto BOTTOM_ANCHORED = true;
                        ring::resize<BOTTOM_ANCHORED>(new_size.y, ring::step);
                    }
                }
                // buff: Push the specified line back.
                void invite(line& l)
                {
                    //dirty = true; // size in cells
                    invite(l._kind, l._size, l.style.get_kind(), l.length());
                }
                // buff: Push a new line back.
                template<class ...Args>
                auto& invite(Args&&... args)
                {
                    //dirty = true; // size in cells
                    auto& l = ring::push_back(std::forward<Args>(args)...);
                    invite(l._kind, l._size, l.style.get_kind(), l.length());
                    return l;
                }
                // buff: Insert a new line at the specified position.
                template<class ...Args>
                auto& insert(iota at, Args&&... args)
                {
                    //dirty = true; // size in cells
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
                    if (basis < 0) basis = 0;
                    if (slide < 0)
                    {
                        anchor_id = l.index + 1;
                        anchor_dy = 0;
                        slide = 0;
                    }
                    //need_to_correct = true; // size in cells
                }
                // buff: Remove information about the specified line from accounting.
                void undock_base_back (line& l) override { undock(l._kind, l._size); }
                // buff: Return the item position in the scrollback using its id.
                auto index_by_id(ui32 id)
                {
                    //No need to disturb distant objects, it may already be in the swap.
                    auto count = length();
                    return static_cast<iota>(count - 1 - (back().index - id)); // ring buffer size is never larger than max_int32.
                }
                // buff: Return an iterator pointing to the item with the specified id.
                auto iter_by_id(ui32 id)
                {
                    return begin() + index_by_id(id);
                }
                // buff: Return the item reference using its id.
                auto& item_by_id(ui32 id)
                {
                    return ring::at(index_by_id(id));
                }
                //// buff: Refresh scrollback size in cells, starting at the specified index.
                //void recalc_size(iota taken_index)
                //{
                //    auto head = begin() + std::max(0, taken_index);
                //    auto tail = end();
                //    auto& curln = *head;
                //    auto accum = curln.accum;
                //    //auto i = 0;
                //    //log("  i=", i++, " curln.accum=", accum);
                //    accum += curln.length() + lnpadding;
                //    while (++head != tail)
                //    {
                //        auto& curln = *head;
                //        curln.accum = accum;
                //        //log("  i=", i++, " curln.accum=", accum);
                //        accum += curln.length() + lnpadding;
                //    }
                //    dirty = faux;
                //    //log( " recalc_size taken_index=", taken_index);
                //}
                //// buff: Return scrollback size in cells.
                //auto get_size_in_cells()
                //{
                //    auto& endln = back();
                //    auto& endid = endln.index;
                //    auto  count = length();
                //    auto  taken_index = static_cast<iota>(count - 1 - (endid - taken));
                //    if (taken != endid || dirty)
                //    {
                //        auto& topln = front();
                //        recalc_size(taken_index);
                //        taken = endln.index;
                //        accum = endln.accum
                //            + endln.length() + lnpadding
                //            - topln.accum;
                //        //log(" topln.accum=", topln.accum,
                //        //    " endln.accum=", endln.accum,
                //        //    " vsize=", vsize,
                //        //    " accum=", accum);
                //    }
                //    return accum;
                //}
                // buff: Refresh metrics due to modified line.
                void recalc(line& l)
                {
                    recalc(l._kind, l._size, l.style.get_kind(), l.length());
                    //// size in cells
                    //dirty = true;
                    //auto taken_index = index_by_id(taken);
                    //auto curln_index = index_by_id(l.index);
                    //if (curln_index < taken_index)
                    //{
                    //    taken       =     l.index;
                    //    taken_index = curln_index;
                    //}
                    //if (ring::size - taken_index > threshold)
                    //{
                    //    recalc_size(taken_index);
                    //    taken = back().index;
                    //}
                }
                // buff: Rewrite the indices from the specified position to the end.
                void reindex(iota from)
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
                auto remove(iota at, iota count)
                {
                    count = ring::remove(at, count);
                    reindex(at);
                    return count;
                }
                // buff: Clear scrollback, add one empty line, and reset all metrics.
                void clear()
                {
                    ring::clear();
                    caret = 0;
                    basis = 0;
                    slide = 0;
                    //accum = 0; // size in cells
                    //dirty = 0; // size in cells
                    //taken = 0; // size in cells
                    invite(0); // At least one line must exist.
                    anchor_id = back().index;
                    anchor_dy = 0;
                    set_width(width);
                }
            };

            // For debug
            friend auto& operator<< (std::ostream& s, scroll_buf& c)
            {
                return s << "{ " << c.batch.max<line::type::leftside>() << ","
                                 << c.batch.max<line::type::rghtside>() << ","
                                 << c.batch.max<line::type::centered>() << ","
                                 << c.batch.max<line::type::autowrap>() << " }";
            }

            buff batch; // scroll_buf: Scrollback container.
            indx index; // scroll_buf: Viewport line index.
            iota arena; // scroll_buf: Scrollable region height.
            face upbox; // scroll_buf:    Top margin canvas.
            face dnbox; // scroll_buf: Bottom margin canvas.
            twod upmin; // scroll_buf:    Top margin minimal size.
            twod dnmin; // scroll_buf: Bottom margin minimal size.

            scroll_buf(term& boss, iota buffer_size, iota grow_step)
                : bufferbase{ boss                   },
                       batch{ buffer_size, grow_step },
                       index{ 0                      },
                       arena{ 1                      }
            {
                batch.invite(0); // At least one line must exist.
                batch.set_width(1);
                index_rebuild();
            }
            iota get_size() const override { return batch.size;     }
            iota get_peak() const override { return batch.peak - 1; }
            iota get_step() const override { return batch.step;     }

            void print_slide(text msg)
            {
                log(msg, ": ", " batch.basis=", batch.basis, " batch.slide=", batch.slide, 
                    " anchor_id=", batch.anchor_id, " anchor_dy=", batch.anchor_dy, " round=", batch.round ? 1:0);
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
                for (auto& l : batch) test_vsize += l.height(panel.x);
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
            // scroll_buf: . // size in cells
            //bool is_need_to_correct() override
            //{
            //    auto result = batch.need_to_correct;
            //    batch.need_to_correct = faux;
            //    return result;
            //}
            // scroll_buf: .
            iota get_basis() override
            {
                return batch.basis;
            }
            // scroll_buf: .
            iota get_slide() override
            {
                return batch.slide;
            }
            // scroll_buf: .
            iota set_slide(iota new_slide) override
            {
                //log(" new_slide=", new_slide);
                if (batch.slide == new_slide)
                {
                    //log("delta == 0");
                    return new_slide;
                }

                if (batch.basis == new_slide)
                {
                    batch.round = faux;
                    auto& mapln = index.front();
                    batch.anchor_id = mapln.index;
                    batch.anchor_dy = mapln.start / panel.x;
                    batch.slide = batch.basis;
                }
                else
                {
                    auto vtpos = batch.slide - batch.anchor_dy; // current pos of anchor_id
                    auto delta = new_slide - vtpos;

                    //if (std::abs(delta) > recalc_threshold) // calc approx
                    //{
                        //auto a = new_slide < recalc_threshold;
                        //auto b = new_slide < std::abs(batch.vsize - new_slide);
                        //if (!round && (a || b))
                        //{
                        //    // Calculate the slide accurately.
                        //    // ...
                        //}
                        //else
                        //{
                        //    round = true;
                        // ...
                        //auto bytesz = batch.get_size_in_cells();
                        //auto curpos = batch.slide;
                        //auto height = batch.vsize;
                        //auto length = batch.size;
                        //auto approx_index = (ui64)length * curpos / height;
                        //auto approx_bytes = (ui64)bytesz * curpos / height;
                        //auto exactly = curpos; 
                        //auto& curln = batch[approx_index];
                        //auto  accum = curln.accum;
                    //}
                    //else // calc exactly
                    {
                        auto idpos = batch.index_by_id(batch.anchor_id);
                        //log(" idpos=", idpos, " batch.size=", batch.size);
                        auto start = batch.begin() + idpos;
                        auto found = faux;
                        if (delta < 0) // Look up.
                        {
                            //log(" delta < 0 new_slide=", new_slide);
                            auto limit = start - std::min(std::abs(delta), idpos);
                            while (start != limit)
                            {
                                auto& curln = *--start;
                                auto height = curln.height(panel.x);
                                vtpos -= height;
                                if (vtpos <= new_slide)
                                {
                                    batch.anchor_id = curln.index;
                                    batch.anchor_dy = new_slide - vtpos;
                                    //log(" delta < 0  new_slide=", new_slide, " batch.slide=", batch.slide, " anchor_id=", batch.anchor_id);
                                    batch.slide = new_slide;
                                    found = true;
                                    break;
                                }
                            }

                            //if (!found)
                            //{
                            //    ///log(" 0.1. slide reset old_slide", batch.slide, " new_slide=", batch.basis );
                            //    batch.anchor_id = batch.back().index - (batch.size - 1);
                            //    batch.anchor_dy = 0;
                            //    batch.slide = 0;
                            //    assert(batch.anchor_id == batch.front().index);
                            //}
                        }
                        else if (delta > 0) //  Look down.
                        {
                            //log(" delta > 0 new_slide=", new_slide);
                            auto limit = start + std::min(delta, batch.size - idpos - 1);
                            do
                            {
                                auto& curln = *start;
                                auto curpos = vtpos;
                                auto height = curln.height(panel.x);
                                vtpos += height;
                                if (vtpos > new_slide)
                                {
                                    batch.anchor_id = curln.index;
                                    batch.anchor_dy = new_slide - curpos;
                                    //log(" delta > 0  new_slide=", new_slide, " batch.slide=", batch.slide, " anchor_id=", batch.anchor_id);
                                    batch.slide = new_slide;
                                    found = true;
                                    break;
                                }
                            }
                            while (start++ != limit);

                            //if (!found)
                            //{
                            //    //log(" 0.2. slide reset old_slide", batch.slide, " new_slide=", batch.basis );
                            //    auto& mapln = index.front();
                            //    batch.anchor_id = mapln.index;
                            //    batch.anchor_dy = mapln.start / panel.x;
                            //    batch.slide = batch.basis;
                            //}
                        }
                        else
                        {
                            batch.slide = new_slide;
                            batch.anchor_dy = 0;
                            found = true;
                        }

                        if (batch.round)
                        {
                            auto a = new_slide < batch.threshold;
                            auto b = new_slide < std::abs(batch.vsize - new_slide);
                            if (a || b)
                            {
                                batch.round = faux;
                                // Calculate the slide accurately.
                                // ...
                            }
                        }
                    }

                }

                //print_slide("  set_slide");
                return new_slide;
            }
            void recalc_slide(bool away)
            {
                if (away)
                {
                    //log(" away");
                    // Recalc batch.slide using anchoring by para_id + para_offset
                    // PoC: (para_id + para_offset) => (batch.slide)
                    //      naive imp, bruteforce (within threshold)
                    //todo calculate it approximately
                    auto endid = batch.back().index;
                    auto count = static_cast<iota>(endid - batch.anchor_id) + 1;
                    if (count <= batch.size)
                    {
                        batch.slide = batch.vsize + batch.anchor_dy;
                        auto head = batch.end();
                        auto tail = head - count;
                        while (head != tail)
                        {
                            auto& curln = *--head;
                            auto height = curln.height(panel.x);
                            batch.slide -= height;
                        }
                             if (batch.slide > batch.basis) batch.slide = batch.basis;
                        else if (batch.slide <= 0) // Overflow.
                        {
                            //log(" Overflow on resize. batch.slide=", batch.slide);
                            //log(" 1. slide reset old_slide", batch.slide, " new_slide=", 0 );
                            batch.slide = 0;
                            batch.anchor_dy = 0;
                            batch.anchor_id = endid - (batch.size - 1);
                            assert(batch.anchor_id == batch.front().index);
                        }
                    }
                    else // Overflow.
                    {
                        //log(" Overflow on resize. Anchor id is outside. batch.slide=", batch.slide,
                        //" anchor_id=", anchor_id, " batch.front().index=", batch.front().index);
                        //log(" 2. slide reset old_slide", batch.slide, " new_slide=", 0 );
                        batch.anchor_dy = 0;
                        batch.anchor_id = endid - (batch.size - 1);
                        batch.slide = 0;
                        assert(batch.anchor_id == batch.front().index);
                    }
                }
                else
                {
                    //log(" 3. slide reset old_slide", batch.slide, " new_slide=", batch.basis );
                    batch.round = faux;
                    batch.slide = batch.basis;
                    auto& mapln = index.front();
                    batch.anchor_id = mapln.index;
                    batch.anchor_dy = mapln.start / panel.x;
                }

                //print_slide(" resize");
            }
            // scroll_buf: Resize viewport.
            void resize_viewport(twod const& new_sz) override
            {
                if (new_sz == panel) return;

                auto in_top = y_top - coord.y;
                auto in_end = coord.y - y_end;

                bufferbase::resize_viewport(new_sz);

                batch.check_size(panel);
                index.clear();

                // Preserve original content. The app that changed the margins is responsible for updating the content.
                auto upnew = std::max(upmin, twod{ panel.x, sctop });
                auto dnnew = std::max(dnmin, twod{ panel.x, scend });
                upbox.crop(upnew);
                dnbox.crop(dnnew);

                arena = y_end - y_top + 1;
                index.resize(arena); // Use a fixed ring because new lines are added much more often than a futures feed.
                auto away = batch.basis != batch.slide;

                auto& curln = batch.current();
                if (curln.wrapped() && batch.caret > curln.length()) // Dangling cursor.
                {
                    auto blank = brush.spc();
                    curln.crop(batch.caret, blank);
                    batch.recalc(curln);
                }

                if (in_top > 0 || in_end > 0) // The cursor is outside the scrolling region.
                {
                    if (in_top > 0) coord.y = std::max(0          , y_top - in_top);
                    else            coord.y = std::min(panel.y - 1, y_end + in_end);
                    coord.x = std::clamp(coord.x, 0, panel.x - 1);
                    batch.basis = std::max(0, batch.vsize - arena);
                    index_rebuild();
                    recalc_slide(away);
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
                recalc_slide(away);

                assert(batch.basis >= 0);
                assert(test_futures());
                assert(test_coord());
                assert(test_resize());
            }
            // scroll_buf: Rebuild the next avail indexes from the known index (mapln).
            template<class ITER, class INDEX_T>
            void reindex(iota avail, ITER curit, INDEX_T const& mapln)
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
            void index_rebuild_from(iota y_pos)
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
                    assert((log(" batch.basis >= batch.vsize  batch.basis=", batch.basis, " batch.vsize=", batch.vsize), true));
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
            iota height() override
            {
                assert(test_height());
                return batch.vsize;
            }
            // scroll_buf: Recalc left and right oversize.
            bool recalc_pads(side& oversz) override
            {
                auto rght = std::max({0, batch.max<line::type::leftside>() - panel.x, coord.x - panel.x + 1 }); // Take into account the cursor position.
                auto left = std::max( 0, batch.max<line::type::rghtside>() - panel.x );
                auto cntr = std::max( 0, batch.max<line::type::centered>() - panel.x );
                auto bttm = std::max( 0, batch.vsize - (batch.basis + arena)         );
                auto both = cntr >> 1;
                left = std::max(left, both);
                rght = std::max(rght, both + (cntr & 1));
                if (oversz.r != rght || oversz.l != left || oversz.b != bttm)
                {
                    oversz.r = rght;
                    oversz.l = left;
                    oversz.b = bttm;
                    return true;
                }
                else return faux;
            }
            // scroll_buf: Check if there are futures, use them when scrolling regions.
            auto feed_futures(iota query)
            {
                assert(test_futures());
                assert(test_coord());
                assert(query > 0);

                auto stash = batch.vsize - batch.basis - index.size;
                iota avail;
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
            twod get_coord(twod const& origin) override
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
            void set_coord(twod const& new_coord) override
            {
                bufferbase::set_coord(new_coord);
                sync_coord();
            }
            // scroll_buf: Map the current cursor position to the scrollback.
            template<bool ALLOW_PENDING_WRAP = true>
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
                    if constexpr (ALLOW_PENDING_WRAP)
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
                    if constexpr (ALLOW_PENDING_WRAP)
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
            void cuf (iota  n) override { bufferbase::cuf (n); sync_coord<faux>(); }
            void cub (iota  n) override { bufferbase::cub (n); sync_coord<faux>(); }
            void chx (iota  n) override { bufferbase::chx (n); sync_coord<faux>(); }
            void tab (iota  n) override { bufferbase::tab (n); sync_coord<faux>(); }
            void chy (iota  n) override { bufferbase::chy (n); sync_coord(); }
            void scl (iota  n) override { bufferbase::scl (n); sync_coord(); }
            void il  (iota  n) override { bufferbase::il  (n); sync_coord(); }
            void dl  (iota  n) override { bufferbase::dl  (n); sync_coord(); }
            void up  (iota  n) override { bufferbase::up  (n); sync_coord(); }
            void dn  (iota  n) override { bufferbase::dn  (n); sync_coord(); }
            void lf  (iota  n) override { bufferbase::lf  (n); sync_coord(); }
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
            void set_scroll_region(iota top, iota bottom) override
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
                auto pull = [&](face& block, twod origin, iota begin, iota limit)
                {
                    if (begin >= index.size) return;
                    limit = std::min(limit, index.size);

                    dissect(begin);
                    dissect(limit);
                    auto from = index[begin    ].index;
                    auto upto = index[limit - 1].index + 1;
                    auto base = batch.index_by_id(from);
                    auto head = batch.begin() + base;
                    auto size = static_cast<iota>(upto - from);
                    auto tail = head + size;
                    auto view = rect{ origin, { panel.x, limit - begin }};
                    auto full = rect{ dot_00, block.core::size()        };
                    block.full(full);
                    block.view(view);
                    block.ac(view.coor);
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

                    iota start;
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
                        auto proto = core::span{ &(*curit), static_cast<size_t>(size.x) }; // Apple Clang doesn't accept an iterator as an arg in the span ctor.
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
            void add_lines(iota amount)
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
            void add_lines(iota amount, cell const& blank)
            {
                assert(amount >= 0);
                auto newid = batch.back().index;
                while (amount-- > 0)
                {
                    auto& l = batch.invite(++newid, parser::style, blank, panel.x);
                    index.push_back(newid, 0, panel.x);
                }
            }
            // scroll_buf: . (! Check coord.y context)
            void _set_style(deco const& new_style)
            {
                auto& curln = batch.current();
                auto  wraps = curln.wrapped();
                auto  width = curln.length();
                curln.style = new_style;

                if (batch.caret > width) // Dangling cursor.
                {
                    auto blank = brush.spc();
                         width = batch.caret;
                    curln.crop(width, blank);
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
                    iota  t;
                    bool  b;
                    rich& block;
                    qt(twod& cy, iota ct, bool cb, scroll_buf& cs)
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
            void el(iota n) override
            {
                bufferbase::flush();
                auto blank = brush.spc();
                if (auto ctx = get_context(coord))
                {
                    iota  start;
                    iota  count;
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
            void ins(iota n) override
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
            void dch(iota n) override
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
            // scroll_buf: CSI n X  Erase/put n chars after cursor. Don't change cursor pos.
            void ech(iota n, char c = whitespace) override
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
            // scroll_buf: Proceed new text (parser callback).
            void data(iota count, grid const& proto) override
            {
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
                        upbox.splice(saved, n, proto);
                    }
                    else
                    {
                        coord.y += (coord.x + panel.x - 1) / panel.x - 1;
                        coord.x  = (coord.x           - 1) % panel.x + 1;

                        if (coord.y >= y_top)
                        {
                            auto n = coord.x + (coord.y - y_top) * panel.x;
                            count -= n;
                            set_coord(twod{ 0, y_top });
                            data(n, proto); // Reversed fill using the last part of the proto.
                        }
                        auto data = proto.begin();
                        auto seek = saved.x + saved.y * panel.x;
                        auto dest = upbox.iter() + seek;
                        auto tail = dest + count;
                        rich::forward_fill_proc(data, dest, tail);
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
                    if (batch.caret <= panel.x || ! curln.wrapped()) // case 0.
                    {
                        curln.splice(start, count, proto);
                        auto& mapln = index[coord.y];
                        assert(coord.x == batch.caret && mapln.index == curln.index);
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
                        coord.y += (coord.x + panel.x - 1) / panel.x - 1;
                        coord.x  = (coord.x           - 1) % panel.x + 1;

                        auto query = coord.y - (index.size - 1);
                        if (query > 0)
                        {
                            auto avail = feed_futures(query);
                            query   -= avail;
                            coord.y -= avail;
                        }
                        curln.splice(start, count, proto);
                        auto curid = curln.index;
                        if (query > 0) // case 3 - complex: Cursor is outside the viewport. 
                        {              // cursor overlaps some lines below and placed below the viewport.
                            batch.recalc(curln);
                            if (auto count = static_cast<iota>(batch.back().index - curid))
                            {
                                assert(count > 0);
                                while (count-- > 0) batch.pop_back();
                            }

                            auto width = curln.length();
                            auto trail = width - panel.x;

                            saved -= batch.basis;
                            if (saved > 0)
                            {
                                auto count = index.size - saved - 1;
                                while (count-- > 0) index.pop_back();
                                auto& mapln = index.back();
                                mapln.width = panel.x;
                                start = mapln.start + panel.x;
                            }
                            else // saved has scrolled out.
                            {
                                index.clear();
                                start = std::abs(saved) * panel.x;
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
                                auto& target = batch.item_by_id(mapln.index);
                                auto  shadow = target.wrapped() ? target.substr(mapln.start + coord.x)
                                                                : target.substr(mapln.start + coord.x, std::min(panel.x, mapln.width) - coord.x);
                                curln.splice(curln.length(), shadow);
                                batch.recalc(curln);
                                auto width = curln.length();
                                auto spoil = static_cast<iota>(mapln.index - curid);
                                assert(spoil > 0);
                                auto after = batch.index() + 1;
                                     spoil = batch.remove(after, spoil);

                                if (saved < batch.basis) index_rebuild(); // Update index. (processing lines larger than viewport)
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
                    }
                    assert(coord.y >= 0 && coord.y < arena);
                    //log(" bufferbase size in cells = ", batch.get_size_in_cells());
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
                        dnbox.splice(saved, n, proto);
                    }
                    else
                    {
                        coord.y += (coord.x + panel.x - 1) / panel.x - 1;
                        coord.x  = (coord.x           - 1) % panel.x + 1;

                        auto data = proto.begin();
                        auto size = count;
                        auto seek = saved.x + saved.y * panel.x;
                        auto dest = dnbox.iter() + seek;
                        auto tail = dnbox.iend();
                        auto back = panel.x;
                        rich::unlimit_fill_proc(data, size, dest, tail, back);
                    }
                    coord.y = std::min(coord.y + y_end + 1, panel.y - 1);
                    // Note: coord can be unsync due to scroll regions.
                }
                assert(test_coord());
            }
            // scroll_buf: Clear scrollback.
            void clear_all() override
            {
                batch.clear();
                reset_scroll_region();
                bufferbase::clear_all();
            }
            // scroll_buf: Set scrollback limits.
            void resize_history(iota new_size, iota grow_by = 0)
            {
                static constexpr auto BOTTOM_ANCHORED = true;
                batch.resize<BOTTOM_ANCHORED>(new_size, grow_by);
                index_rebuild();
            }
            // scroll_buf: Render to the canvas.
            void output(face& canvas) override
            {
                canvas.vsize(batch.vsize + sctop + scend); // Include margins and bottom oversize.
                auto view = canvas.view();
                auto full = canvas.full();
                auto coor = twod{ 0, batch.slide - batch.anchor_dy + y_top };
                auto stop = view.coor.y + view.size.y;
                auto head = batch.iter_by_id(batch.anchor_id);
                auto tail = batch.end();
                auto fill = [&](auto& area, auto chr)
                {
                    if (auto r = view.clip(area))
                        canvas.fill(r, [&](auto& c){ c.txt(chr).fgc(tint::greenlt); });
                };
                auto left_edge = view.coor.x;
                auto rght_edge = view.coor.x + view.size.x;
                auto half_size = full.size.x / 2;
                auto left_rect = rect{{ left_edge, full.coor.y + coor.y }, dot_11 };
                auto rght_rect = left_rect;
                rght_rect.coor.x += view.size.x - 1;
                //print_slide(" output");
                while (head != tail && rght_rect.coor.y < stop)
                {
                    auto& curln = *head;
                    auto height = curln.height(panel.x);
                    auto length = curln.length();
                    auto adjust = curln.style.jet();
                    canvas.output(curln, coor);

                    if (length > 0) // Highlight the lines that are not shown in full.
                    {
                        rght_rect.size.y = left_rect.size.y = height;
                        if (height == 1)
                        {
                            auto lt_dot = full.coor.x;
                            if      (adjust == bias::center) lt_dot += half_size - length / 2;
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

                auto top_coor = twod{ view.coor.x, view.coor.y + y_top - sctop };
                auto end_coor = twod{ view.coor.x, view.coor.y + y_end + 1     };
                upbox.move(top_coor);
                dnbox.move(end_coor);

                //todo make translucency configurable
                canvas.plot(upbox, [](auto& dst, auto& src){ dst.fuse(src); dst.bga(0xC0); });
                canvas.plot(dnbox, [](auto& dst, auto& src){ dst.fuse(src); dst.bga(0xC0); });

                //canvas.plot(upbox, cell::shaders::flat);
                //canvas.plot(dnbox, cell::shaders::flat);
            }
            // scroll_buf: Remove all lines below (including futures) except the current. "ED2 Erase viewport" keeps empty lines.
            void del_below() override
            {
                assert(test_futures());

                auto blank = brush.spc();
                auto clear = [&](twod const& coor)
                {
                    auto& from = index[coor.y];
                    auto topid = from.index;
                    auto start = from.start;
                    auto i = batch.index_by_id(topid);
                    auto n = batch.size - 1 - i;
                    auto m = index.size - 1 - coor.y;
                    auto p = arena      - 1 - coor.y;

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

                    add_lines(p, blank);
                    i = batch.index_by_id(topid); // The index may be outdated due to the ring.
                    auto& curln = batch[i];
                    if (fresh)
                    {
                        curln.trimto(start);
                    }
                    else
                    {
                        auto& mapln = index[coor.y];
                        mapln.width = panel.x;
                        curln.splice<true>(start + coor.x, panel.x - coor.x, blank);
                        curln.trimto(start + panel.x);
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
                    coor.x = std::clamp(coor.x, 0, panel.x);
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
                auto blank = brush.spc();
                auto clear = [&](twod const& from)
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
            void dissect(iota y_pos)
            {
                assert(y_pos >= 0 && y_pos <= arena);

                auto split = [&](id_t curid, iota start)
                {
                    auto after = batch.index_by_id(curid);
                    auto tmpln = std::move(batch[after]);
                    auto curit = batch.ring::insert(after + 1, tmpln.index, tmpln.style);
                    auto endit = batch.end();

                    auto& newln = *curit;
                    newln.splice(0, tmpln.substr(start));
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
            void scroll_region(iota top, iota end, iota n, bool use_scrollback) override
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
                        auto range = static_cast<iota>(mdlid - topid);
                        auto floor = batch.index_by_id(endid) - range;
                        batch.remove(start, range);

                        // Insert block.
                        while (count-- > 0) batch.insert(floor, id_t{}, parser::style);

                        batch.reindex(start); //todo The index may be outdated due to the ring.
                        index_rebuild();
                    }
                }
                else // Scroll text down.
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
                    auto range = static_cast<iota>(endid - mdlid);
                    auto floor = batch.index_by_id(endid) - range;
                    batch.remove(floor, range);

                    // Insert block.
                    while (count-- > 0) batch.insert(start, id_t{}, parser::style);

                    batch.reindex(start); //todo The index may be outdated due to the ring.
                    index_rebuild();
                }

                assert(test_futures());
                assert(test_coord());
            }
        };

        using buffer_ptr = bufferbase*;

        pro::caret cursor; // term: Text cursor controller.
        term_state status; // term: Screen buffer status info.
        w_tracking wtrack; // term: Terminal title tracking object.
        f_tracking ftrack; // term: Keyboard focus tracking object.
        m_tracking mtrack; // term: VT-style mouse tracking object.
        c_tracking ctrack; // term: Custom terminal palette tracking object.
        scroll_buf normal; // term: Normal    screen buffer.
        alt_screen altbuf; // term: Alternate screen buffer.
        buffer_ptr target; // term: Current   screen buffer pointer.
        os::ptydev ptycon; // term: PTY device.
        text       cmdarg; // term: Startup command line arguments.
        hook       oneoff; // term: One-shot token for start and shutdown events.
        twod       origin; // term: Viewport position.
        twod       follow; // term: Viewport follows cursor (bool: X, Y).
        bool       active; // term: Terminal lifetime.
        bool       decckm; // term: Cursor keys Application(true)/ANSI(faux) mode.
        bool       bpmode; // term: Bracketed paste mode.
        bool       onlogs; // term: Avoid logs if no subscriptions.
        bool       unsync; // term: Viewport is out of sync.
        bool       invert; // term: Inverted rendering (DECSCNM).

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
        }
        // term: Set termnail parameters. (DECSET).
        void decset(fifo& queue)
        {
            target->flush();
            while (auto q = queue(0))
            {
                switch (q)
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
                        target->cup(dot_11);
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
                        log("decset: CSI ? 9 h  X10 Mouse reporting protocol is not supported");
                        break;
                    case 1000: // Enable mouse buttons reporting mode.
                        mtrack.enable(m_tracking::buttons_press);
                        break;
                    case 1001: // Use Hilite mouse tracking mode.
                        log("decset: CSI ? 1001 h  Hilite mouse tracking mode is not supported");
                        break;
                    case 1002: // Enable mouse buttons and drags reporting mode.
                        mtrack.enable(m_tracking::buttons_drags);
                        break;
                    case 1003: // Enable all mouse events reporting mode.
                        mtrack.enable(m_tracking::all_movements);
                        break;
                    case 1004: // Enable focus tracking.
                        ftrack.set(true);
                        break;
                    case 1005: // Enable UTF-8 mouse reporting protocol.
                        log("decset: CSI ? 1005 h  UTF-8 mouse reporting protocol is not supported");
                        break;
                    case 1006: // Enable SGR mouse reporting protocol.
                        mtrack.setmode(m_tracking::sgr);
                        break;
                    case 10060:// Enable mouse reporting outside the viewport (outside+negative coordinates).
                        mtrack.enable(m_tracking::negative_args);
                        break;
                    case 1015: // Enable URXVT mouse reporting protocol.
                        log("decset: CSI ? 1015 h  URXVT mouse reporting protocol is not supported");
                        break;
                    case 1016: // Enable Pixels (subcell) mouse mode.
                        log("decset: CSI ? 1016 h  Pixels (subcell) mouse mode is not supported");
                        break;
                    case 1048: // Save cursor pos.
                        target->scp();
                        break;
                    case 1047: // Use alternate screen buffer.
                    case 1049: // Save cursor pos and use alternate screen buffer, clearing it first.  This control combines the effects of the 1047 and 1048  modes.
                        altbuf.style = target->style;
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
        }
        // term: Reset termnail parameters. (DECRST).
        void decrst(fifo& queue)
        {
            target->flush();
            while (auto q = queue(0))
            {
                switch (q)
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
                        target->cup(dot_11);
                        break;
                    case 7:    // Disable auto-wrap.
                        target->style.wrp(wrap::off);
                        break;
                    case 12:   // Disable cursor blinking.
                        cursor.blink_period(period::zero());
                        break;
                    case 25:   // Caret off.
                        cursor.hide();
                        break;
                    case 9:    // Disable X10 mouse reporting protocol.
                        log("decset: CSI ? 9 l  X10 Mouse tracking protocol is not supported");
                        break;
                    case 1000: // Disable mouse buttons reporting mode.
                        mtrack.disable(m_tracking::buttons_press);
                        break;
                    case 1001: // Don't use Hilite(c) mouse tracking mode.
                        log("decset: CSI ? 1001 l  Hilite mouse tracking mode is not supported");
                        break;
                    case 1002: // Disable mouse buttons and drags reporting mode.
                        mtrack.disable(m_tracking::buttons_drags);
                        break;
                    case 1003: // Disable all mouse events reporting mode.
                        mtrack.disable(m_tracking::all_movements);
                        break;
                    case 1004: // Disable focus tracking.
                        ftrack.set(faux);
                        break;
                    case 1005: // Disable UTF-8 mouse reporting protocol.
                        log("decset: CSI ? 1005 l  UTF-8 mouse reporting protocol is not supported");
                        break;
                    case 1006: // Disable SGR mouse reporting protocol (set X11 mode).
                        mtrack.setmode(m_tracking::x11);
                        break;
                    case 10060:// Disable mouse reporting outside the viewport (allow reporting inside the viewport only).
                        mtrack.disable(m_tracking::negative_args);
                        break;
                    case 1015: // Disable URXVT mouse reporting protocol.
                        log("decset: CSI ? 1015 l  URXVT mouse reporting protocol is not supported");
                        break;
                    case 1016: // Disable Pixels (subcell) mouse mode.
                        log("decset: CSI ? 1016 l  Pixels (subcell) mouse mode is not supported");
                        break;
                    case 1048: // Restore cursor pos.
                        target->rcp();
                        break;
                    case 1047: // Use normal screen buffer.
                    case 1049: // Use normal screen buffer and restore cursor.
                        normal.style = target->style;
                        normal.resize_viewport(target->panel); // Reset viewport to the basis.
                        target = &normal;
                        follow[axis::Y] = true;
                        break;
                    case 2004: // Disable bracketed paste mode.
                        bpmode = faux;
                        break;
                    default:
                        break;
                }
            }
        }
        // term: Set scrollback buffer size and grow step.
        void sbsize(fifo& queue)
        {
            target->flush();
            auto ring_size = queue(def_length);
            auto grow_step = queue(def_growup);
            normal.resize_history(ring_size, grow_step);
        }
        // term: Extended functionality response.
        void native(bool  are_u)
        {
            auto response = ansi::ext(are_u);
            answer(response);
        }
        // term: Write tty data and flush the queue.
        void answer(ansi::esc& queue)
        {
            if (queue.length())
            {
                ptycon.write(queue);
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
                    auto c = console.get_coord(dot_00);
                    if (origin.x != 0 || c.x != console.panel.x)
                    {
                             if (c.x >= 0  && c.x < console.panel.x) origin.x = 0;
                        else if (c.x >= -origin.x + console.panel.x) origin.x = -c.x + console.panel.x - 1;
                        else if (c.x <  -origin.x                  ) origin.x = -c.x;
                    }
                }
                origin.y = -console.get_basis();
            }
            else
            {
                origin.y = -console.get_slide();
            }
        }
        // term: Proceed terminal input.
        void ondata(view data)
        {
            while (active)
            {
                netxs::events::try_sync guard;
                if (guard)
                {
                    if (onlogs) SIGNAL_GLOBAL(e2::debug::output, data); // Post data for Logs.

                    if (follow[axis::Y]) ansi::parse(data, target);
                    else
                    {
                        auto last_basis = target->get_basis();
                        auto last_slide = target->get_slide();
                        ansi::parse(data, target);
                        auto next_basis = target->get_basis();
                        follow[axis::Y] = last_basis <= last_slide && last_slide <= next_basis
                                       || next_basis <= last_slide && last_slide <= last_basis;
                    }

                    unsync = true;
                    break;
                }
                else std::this_thread::yield();
            }
        }
        // term: Shutdown callback handler.
        void onexit(iota code)
        {
            log("term: exit code ", code);
            if (code)
            {
                text error = ansi::bgc(reddk).fgc(whitelt).add("\nterm: exit code ", code, " ");
                ondata(error);
            }
            else
            {
                log("term: submit for destruction on next frame/tick");
                SUBMIT_GLOBAL(e2::timer::any, oneoff, t)
                {
                    auto backup = This();
                    this->base::riseup<tier::release>(e2::form::quit, backup);
                    oneoff.reset();
                };
            }
        }

    public:
        void exec_cmd(commands::ui::commands cmd)
        {
            log("term: tier::preview, ui::commands, ", cmd);
            follow[axis::Y] = true;
            switch (cmd)
            {
                case commands::ui::left:
                    target->style.jet(bias::left);
                    break;
                case commands::ui::center:
                    target->style.jet(bias::center);
                    break;
                case commands::ui::right:
                    target->style.jet(bias::right);
                    break;
                case commands::ui::togglewrp:
                    target->style.wrp(target->style.wrp() == wrap::on ? wrap::off : wrap::on);
                    break;
                case commands::ui::reset:
                    decstr();
                    break;
                case commands::ui::clear:
                    target->ed(commands::erase::display::viewport);
                    break;
                default:
                    break;
            }
            ondata("");
        }
        void data_in(view data)
        {
            log("term: app::term::data::in, ", utf::debase(data));
            follow[axis::Y] = true;
            ondata(data);
        }
        void data_out(view data)
        {
            log("term: app::term::data::out, ", utf::debase(data));
            follow[axis::Y] = true;
            ptycon.write(data);
        }
        void start()
        {
            static auto unique = decltype(e2::timer::tick)::type{}; // Eliminate concurrent start actions.

            if (!ptycon && !oneoff)
            {
                SUBMIT_GLOBAL(e2::timer::any, oneoff, timer)
                {
                    if (unique != timer)
                    {
                        auto initsz = target->panel;
                        ptycon.start(cmdarg, initsz, [&](auto utf8_shadow) { ondata(utf8_shadow); },
                                                     [&](auto exit_reason) { onexit(exit_reason); } );
                        unique = timer;
                        oneoff.reset();
                    }
                };
            }
        }
        // term: Resize terminal window.
        void window_resize(twod winsz)
        {
            base::riseup<tier::preview>(e2::form::prop::window::size, winsz);
        }
       ~term(){ active = faux; }
        term(text command_line, iota max_scrollback_size = def_length, iota grow_step = def_growup)
        //term(text command_line, iota max_scrollback_size = 50, iota grow_step = 0)
            : normal{ *this, max_scrollback_size, grow_step },
              altbuf{ *this },
              cursor{ *this },
              mtrack{ *this },
              ftrack{ *this },
              wtrack{ *this },
              ctrack{ *this },
              follow{  0, 1 },
              active{  true },
              decckm{  faux },
              bpmode{  faux },
              onlogs{  faux },
              unsync{  faux },
              invert{  faux }
        {
            cmdarg = command_line;
            target = &normal;
            //cursor.style(commands::cursor::def_style); // default=blinking_box
            cursor.style(commands::cursor::blinking_underline);
            cursor.show(); //todo revise (possible bug)
            form::keybd.accept(true); // Subscribe on keybd offers.

            SUBMIT(tier::general, e2::debug::count::any, count)
            {
                onlogs = count > 0;
            };
            SIGNAL_GLOBAL(e2::debug::count::set, 0);
            SUBMIT(tier::release, e2::form::upon::vtree::attached, parent)
            {
                this->base::riseup<tier::request>(e2::form::prop::header, wtrack.get(ansi::OSC_TITLE));
            };
            SUBMIT(tier::preview, e2::coor::set, new_coor)
            {
                //todo correct if round
                //log("tier::preview, e2::coor::set, new_coor");
            };
            SUBMIT(tier::release, e2::coor::set, new_coor)
            {
                //todo use tier::preview bcz approx viewport position can be corrected
                auto& console = *target;
                origin.x = new_coor.x;
                origin.y = -console.set_slide(-new_coor.y);
                follow[axis::Y] = console.get_slide() == console.get_basis();

                //preview: new_coor = origin;

                //auto& console = *target;
                //rack scinfo{};
                //auto& thing = *this;
                //auto  block = thing.base::area();
                //scinfo.beyond = thing.oversz;  // Oversize value.
                //scinfo.region = block.size;
                //auto fullsize = console.height();
                //auto count = console.get_size();
                ////auto avg_height = (double)fullsize / count;
                //auto curline = netxs::divround(fullsize * console.anchor(), count);
                //block.coor.y = -curline;//; // Viewport.
                //scinfo.window.coor = -block.coor; // Viewport.
                //scinfo.window.size = console.panel;      //
                //this->SIGNAL(tier::release, e2::form::upon::scroll::bycoor::x, scinfo);
                //this->SIGNAL(tier::release, e2::form::upon::scroll::bycoor::y, scinfo);
            };
            SUBMIT(tier::preview, e2::size::set, new_size)
            {
                auto& console = *target;
                new_size = std::max(new_size, dot_11);

                auto scroll_coor = origin;
                console.resize_viewport(new_size);
                console.recalc_pads(base::oversz);

                scroll(origin);
                base::anchor += scroll_coor - origin;

                ptycon.resize(new_size);

                new_size.y += console.get_basis();
            };
            SUBMIT(tier::release, hids::events::keybd::any, gear)
            {
                this->riseup<tier::release>(e2::form::animate::reset, 0); // Reset scroll animation.

                follow[axis::X] = true;
                follow[axis::Y] = true;
                #ifndef PROD
                    return;
                #endif
                //todo optimize/unify
                auto data = gear.keystrokes;
                if (!bpmode)
                {
                    utf::change(data, "\033[200~", "");
                    utf::change(data, "\033[201~", "");
                }
                if (decckm)
                {
                    utf::change(data, "\033[A", "\033OA");
                    utf::change(data, "\033[B", "\033OB");
                    utf::change(data, "\033[C", "\033OC");
                    utf::change(data, "\033[D", "\033OD");
                    utf::change(data, "\033[1A", "\033OA");
                    utf::change(data, "\033[1B", "\033OB");
                    utf::change(data, "\033[1C", "\033OC");
                    utf::change(data, "\033[1D", "\033OD");
                }
                // Linux console specific.
                utf::change(data, "\033[[A", "\033OP");      // F1
                utf::change(data, "\033[[B", "\033OQ");      // F2
                utf::change(data, "\033[[C", "\033OR");      // F3
                utf::change(data, "\033[[D", "\033OS");      // F4
                utf::change(data, "\033[[E", "\033[15~");    // F5
                utf::change(data, "\033[25~", "\033[1;2P");  // Shift+F1
                utf::change(data, "\033[26~", "\033[1;2Q");  // Shift+F2
                utf::change(data, "\033[28~", "\033[1;2R");  // Shift+F3
                utf::change(data, "\033[29~", "\033[1;2S");  // Shift+F4
                utf::change(data, "\033[31~", "\033[15;2~"); // Shift+F5
                utf::change(data, "\033[32~", "\033[17;2~"); // Shift+F6
                utf::change(data, "\033[33~", "\033[18;2~"); // Shift+F7
                utf::change(data, "\033[34~", "\033[19;2~"); // Shift+F8

                ptycon.write(data);

                #ifdef KEYLOG
                    std::stringstream d;
                    view v = gear.keystrokes;
                    log("key strokes raw: ", utf::debase(v));
                    while (v.size())
                    {
                        d << (int)v.front() << " ";
                        v.remove_prefix(1);
                    }
                    log("key strokes bin: ", d.str());
                #endif
            };
            SUBMIT(tier::release, e2::form::prop::brush, brush)
            {
                target->brush.reset(brush);
            };
            SUBMIT(tier::general, e2::timer::tick, timestamp)
            {
                if (!unsync) return;
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
                    this->SIGNAL(tier::release, e2::size::set, scroll_size);
                    this->base::moveto(scroll_coor);
                }
                base::deface();
            };
            SUBMIT(tier::release, e2::render::any, parent_canvas)
            {
                auto& console = *target;
                if (status.update(console))
                {
                    this->base::riseup<tier::preview>(e2::form::prop::footer, status.data);
                }

                auto view = parent_canvas.view();
                auto full = parent_canvas.full();
                auto base = full.coor - view.coor;
                cursor.coor(console.get_coord(base));

                console.output(parent_canvas);
                if (invert) parent_canvas.fill(cell::shaders::invbit);

                if (oversz.b > 0) // Shade the viewport bottom oversize (futures).
                {
                    auto bottom_oversize = parent_canvas.full();
                    bottom_oversize.coor.y += console.get_basis() + console.panel.y - console.scend;
                    bottom_oversize.size.y  = oversz.b;
                    bottom_oversize = bottom_oversize.clip(parent_canvas.view());
                    parent_canvas.fill(bottom_oversize, cell::shaders::xlight);
                }

                // Debug: Shade active viewport.
                //{
                //    auto size = console.panel;
                //    size.y -= console.sctop + console.scend;
                //    auto vp = rect{ { 0,console.get_basis() + console.sctop }, size };
                //    vp.coor += parent_canvas.full().coor;
                //    vp = vp.clip(parent_canvas.view());
                //    parent_canvas.fill(vp, [](auto& c){ c.fuse(cell{}.bgc(magentalt).bga(50)); });
                //}
            };
        }
    };
}

#endif // NETXS_TERMINAL_HPP