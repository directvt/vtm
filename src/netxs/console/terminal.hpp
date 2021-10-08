// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_TERMINAL_HPP
#define NETXS_TERMINAL_HPP

#include "../ui/controls.hpp"

#include <cassert>

namespace netxs::events::userland
{
    struct term
    {
        EVENTPACK( term, netxs::events::userland::root::custom )
        {
            EVENT_XS( cmd   , iota ),
            GROUP_XS( layout, iota ),
            GROUP_XS( data  , iota ),

            SUBSET_XS( layout )
            {
                EVENT_XS( align , bias ),
                EVENT_XS( wrapln, wrap ),
            };
            SUBSET_XS( data )
            {
                EVENT_XS( in , view ),
                EVENT_XS( out, view ),
            };
        };
    };
}

namespace netxs::app
{
    using namespace netxs::console;

    // terminal: Built-in terminal app.
    struct term
        : public ui::form<term>
    {
        using events = netxs::events::userland::term;

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
                enum : iota
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
        static constexpr iota def_length = 20000; // term: Default scrollback history length.
        static constexpr iota def_growup = 0;     // term: Default scrollback history grow step.
        static constexpr iota def_tablen = 8;     // term: Default tab length.

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
                    data.jet(bias::right)
                        .add("size=", size,
                            " peak=", peak - 1,
                            " type=");
                    if (step) data.add("unlimited, grow by ", step);
                    else      data.add("fixed");
                    data.add(" area=", area);
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
                        c.y -= console.basis;
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
            // w_tracking: Manage terminal window props (XTWINOPS).
            void manage(fifo& q)
            {
                owner.target->flush();
                static constexpr iota get_label = 20; // Report icon   label. (Report as OSC L label ST).
                static constexpr iota get_title = 21; // Report window title. (Report as OSC l title ST).
                static constexpr iota put_stack = 22; // Push icon label and window title to   stack.
                static constexpr iota pop_stack = 23; // Pop  icon label and window title from stack.
                static constexpr iota all_title = 0;  // Sub commands.
                static constexpr iota label     = 1;  // Sub commands.
                static constexpr iota title     = 2;  // Sub commands.
                switch (auto option = q(0))
                {
                    // Return an empty string for security reasons
                    case get_label: owner.answer(queue.osc(ansi::OSC_LABEL_REPORT, "")); break;
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

                vt.csier.table[CSI_CUU] = VT_PROC{ p->up ( q(1)); };  // CSI n A
                vt.csier.table[CSI_CUD] = VT_PROC{ p->dn ( q(1)); };  // CSI n B
                vt.csier.table[CSI_CUF] = VT_PROC{ p->cuf( q(1)); };  // CSI n C
                vt.csier.table[CSI_CUB] = VT_PROC{ p->cuf(-q(1)); };  // CSI n D

                vt.csier.table[CSI_CHT] = VT_PROC{ p->tab( q(1)); };  // CSI n I  Caret forward  n tabs, default n=1.
                vt.csier.table[CSI_CBT] = VT_PROC{ p->tab(-q(1)); };  // CSI n Z  Caret backward n tabs, default n=1.
                vt.csier.table[CSI_TBC] = VT_PROC{ p->tbc( q(1)); };  // CSI n g  Reset tabstop value.

                vt.csier.table[CSI_CUD2]= VT_PROC{ p->dn ( q(1)); };  // CSI n e  Move cursor down. Same as CUD.

                vt.csier.table[CSI_CNL] = vt.csier.table[CSI_CUD];    // CSI n E
                vt.csier.table[CSI_CPL] = vt.csier.table[CSI_CUU];    // CSI n F
                vt.csier.table[CSI_CHX] = VT_PROC{ p->chx( q(1)); };  // CSI n G  Move cursor hz absolute.
                vt.csier.table[CSI_CHY] = VT_PROC{ p->chy( q(1)); };  // CSI n d  Move cursor vt absolute.
                vt.csier.table[CSI_CUP] = VT_PROC{ p->cup( q   ); };  // CSI y ; x H (1-based)
                vt.csier.table[CSI_HVP] = VT_PROC{ p->cup( q   ); };  // CSI y ; x f (1-based)

                vt.csier.table[CSI_DCH] = VT_PROC{ p->dch( q(1)); };  // CSI n P  Delete n chars.
                vt.csier.table[CSI_ECH] = VT_PROC{ p->ech( q(1)); };  // CSI n X  Erase n chars.
                vt.csier.table[CSI_ICH] = VT_PROC{ p->ins( q(1)); };  // CSI n @  Insert n chars. ICH

                vt.csier.table[CSI__ED] = VT_PROC{ p->ed ( q(0)); };  // CSI n J
                vt.csier.table[CSI__EL] = VT_PROC{ p->el ( q(0)); };  // CSI n K
                vt.csier.table[CSI__IL] = VT_PROC{ p->il ( q(1)); };  // CSI n L  Insert n lines.
                vt.csier.table[CSI__DL] = VT_PROC{ p->dl ( q(1)); };  // CSI n M  Delete n lines.
                vt.csier.table[CSI__SD] = VT_PROC{ p->scl( q(1)); };  // CSI n T  Scroll down by n lines, scrolled out lines are lost.
                vt.csier.table[CSI__SU] = VT_PROC{ p->scl(-q(1)); };  // CSI n S  Scroll   up by n lines, scrolled out lines are pushed to the scrollback.
                vt.csier.table[CSI_SCP] = VT_PROC{ p->scp(     ); };  // CSI   s  Save cursor position.
                vt.csier.table[CSI_RCP] = VT_PROC{ p->rcp(     ); };  // CSI   u  Restore cursor position.

                vt.csier.table[DECSTBM] = VT_PROC{ p->scr( q   ); };  // CSI r; b r  Set scrolling region (t/b: top+bottom).

                vt.csier.table[CSI_WIN] = VT_PROC{ p->owner.wtrack.manage(q); };  // CSI n;m;k t  Terminal window options (XTWINOPS).

                vt.csier.table[CSI_CCC][CCC_SBS] = VT_PROC{ p->owner.sbsize(q); };  // CCC_SBS: Set scrollback size.
                vt.csier.table[CSI_CCC][CCC_EXT] = VT_PROC{ p->owner.native(q(1)); };          // CCC_EXT: Setup extended functionality.
                vt.csier.table[CSI_CCC][CCC_RST] = VT_PROC{ p->style.glb(); p->style.wrp(WRAPPING); };  // fx_ccc_rst

                vt.intro[ctrl::ESC][ESC_IND] = VT_PROC{ p->dn(1); };          // ESC D  Caret Down.
                vt.intro[ctrl::ESC][ESC_IR ] = VT_PROC{ p->ri (); };          // ESC M  Reverse index.
                vt.intro[ctrl::ESC][ESC_HTS] = VT_PROC{ p->stb(); };          // ESC H  Place tabstop at the current cursor posistion.
                vt.intro[ctrl::ESC][ESC_RIS] = VT_PROC{ p->owner.decstr(); }; // ESC c Reset to initial state (same as DECSTR).
                vt.intro[ctrl::ESC][ESC_SC ] = VT_PROC{ p->scp(); };          // ESC 7 (same as CSI s) Save cursor position.
                vt.intro[ctrl::ESC][ESC_RC ] = VT_PROC{ p->rcp(); };          // ESC 8 (same as CSI u) Restore cursor position.

                vt.intro[ctrl::BS ] = VT_PROC{ p->cuf(-q.pop_all(ctrl::BS )); };
                vt.intro[ctrl::DEL] = VT_PROC{ p->del( q.pop_all(ctrl::DEL)); };
                vt.intro[ctrl::TAB] = VT_PROC{ p->tab( q.pop_all(ctrl::TAB)); };
                vt.intro[ctrl::CR ] = VT_PROC{ p->home(); };
                vt.intro[ctrl::EOL] = VT_PROC{ p->dn ( q.pop_all(ctrl::EOL)); };

                vt.csier.table_quest[DECSET] = VT_PROC{ p->owner.decset(q); };
                vt.csier.table_quest[DECRST] = VT_PROC{ p->owner.decrst(q); };

                vt.oscer[OSC_LABEL_TITLE] = VT_PROC{ p->owner.wtrack.set(OSC_LABEL_TITLE, q); };
                vt.oscer[OSC_LABEL]       = VT_PROC{ p->owner.wtrack.set(OSC_LABEL,       q); };
                vt.oscer[OSC_TITLE]       = VT_PROC{ p->owner.wtrack.set(OSC_TITLE,       q); };
                vt.oscer[OSC_XPROP]       = VT_PROC{ p->owner.wtrack.set(OSC_XPROP,       q); };

                // Log all unimplemented CSI commands.
                for (auto i = 0; i < 0x100; ++i)
                {
                    auto& proc = vt.csier.table[i];
                    if (!proc)
                    {
                        proc = [i](auto& q, auto& p) { p->not_implemented_CSI(i, q); };
                    }
                }
            }

            term& owner; // bufferbase: Terminal object reference.
            twod  panel; // bufferbase: Viewport size.
            twod  coord; // bufferbase: Viewport cursor position; 0-based.
            twod  saved; // bufferbase: Saved cursor position.
            iota  sctop; // bufferbase: Scrolling region top    height.
            iota  scend; // bufferbase: Scrolling region bottom height.
            iota  y_top; // bufferbase: 0-based actual scrolling region top    vertical pos.
            iota  y_end; // bufferbase: 0-based actual scrolling region bottom vertical pos.
            iota  basis; // bufferbase: Viewport basis. Index of O(0, 0) in the scrollback.
            iota  tabsz; // bufferbase: Tabstop current value.

            bufferbase(term& master)
                : owner{ master },
                  panel{ dot_11 },
                  coord{ dot_00 },
                  saved{ dot_00 },
                  sctop{ 0      },
                  scend{ 0      },
                  y_top{ 0      },
                  y_end{ 0      },
                  basis{ 0      },
                  tabsz{ def_tablen }
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

            // bufferbase: Get vertival oversize.
    virtual iota get_oversize()
            {
                return 0;
            }
            void update_region()
            {
                auto max = panel.y - 1;
                y_end = std::clamp(max - scend, 0, max);
                y_top = std::clamp(sctop, 0, y_end);
            }
    virtual void resize_viewport(twod const& new_sz)
            {
                panel = std::max(new_sz, dot_11);
                update_region();
            }
            // bufferbase: Reset coord and set the scrolling region using 1-based top and bottom. Use 0 to reset.
    virtual void set_scroll_region(iota top, iota bottom)
            {
                coord = dot_00;
                sctop = std::max(0, top - 1);
                scend = bottom != 0 ? std::max(0, panel.y - bottom)
                                    : 0;
                update_region();
            }
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
    virtual void meta(deco const& old_style) override
            {
                if (parser::style.wrp() != old_style.wrp())
                {
                    auto status = parser::style.wrp() == wrap::none ? WRAPPING
                                                                    : parser::style.wrp();
                    owner.base::broadcast->SIGNAL(tier::release, app::term::events::layout::wrapln, status);
                }
                if (parser::style.jet() != old_style.jet())
                {
                    auto status = parser::style.jet() == bias::none ? bias::left
                                                                    : parser::style.jet();
                    owner.base::broadcast->SIGNAL(tier::release, app::term::events::layout::align, status);
                }
            }
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
                log("CSI ", params, " ", (unsigned char)i, "(", std::to_string(i), ") is not implemented.");
            }
    virtual void clear_all()
            {
                parser::state = {};
            }
            // bufferbase: ESC H  Place tabstop at the current cursor posistion.
            void stb()
            {
                parser::flush();
                tabsz = std::max(1, coord.x + 1);
            }
            // bufferbase: TAB  Horizontal tab.
    virtual void tab(iota n)
            {
                parser::flush();
                if (n > 0)
                {
                    auto new_pos = coord.x + n * tabsz - coord.x % tabsz;
                    if (new_pos < panel.x) coord.x = new_pos;
                }
                else if (n < 0)
                {
                    n = -n - 1;
                    auto count = n * tabsz + coord.x % tabsz;
                    coord.x = std::max(0, coord.x - count);
                }
            }
            // bufferbase: CSI n g  Reset tabstop value.
            void tbc(iota n)
            {
                parser::flush();
                tabsz = def_tablen;
            }
            // bufferbase: ESC 7 or CSU s  Save cursor position.
            void scp()
            {
                parser::flush();
                saved = coord;
            }
            // bufferbase: ESC 8 or CSU u  Restore cursor position.
            void rcp()
            {
                parser::flush();
                set_coord(saved);
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
    virtual void ech(iota n) = 0;
    // Reserved for future use.
    //virtual void ech_grow(iota n) = 0;
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
                coord.x += n;
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
                parser::flush();
                coord.y = std::clamp(n, 1, panel.y) - 1;
                assert(panel.inside(coord));
            }
            // bufferbase: CSI y; x H/F  Caret position (1-based).
    virtual void cup(fifo& queue)
            {
                parser::flush();
                auto y = queue(1);
                auto x = queue(1);
                auto p = twod{ x, y };
                coord  = std::clamp(p, dot_11, panel) - dot_11;
            }
            // bufferbase: Line reverse feed (move cursor up).
    virtual void up(iota n)
            {
                parser::flush();
                coord.y = std::clamp(coord.y - n, 0, panel.y - 1);
            }
            // bufferbase: Line feed (move cursor down). Scroll region up if new_coord_y > end.
    virtual void dn(iota n)
            {
                parser::flush();
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
            // bufferbase: '\r'  Go to home of visible line instead of home of para.
    virtual void home()
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
                        tail += std::min(panel.x, coord.x);
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
            void ech(iota n) override
            {
                parser::flush();
                auto blank = brush.spc();//.bgc(greendk).bga(0x7f);
                canvas.splice(coord, n, blank);
            }
            // alt_screen: Parser callback.
            void data(iota count, grid const& proto) override
            {
                assert(coord.y >= 0 && coord.y < panel.y);

                auto old_coord = coord;
                coord.x += count;
                //todo apply line adjusting
                if (coord.x <= panel.x)//todo styles! || ! curln.wrapped())
                {
                    auto n = std::min(count, panel.x - std::max(0, old_coord.x));
                    canvas.splice(old_coord, n, proto);
                }
                else
                {
                    coord.y += (coord.x + panel.x - 1) / panel.x - 1;
                    coord.x  = (coord.x           - 1) % panel.x + 1;

                    if (old_coord.y < y_top)
                    {
                        if (coord.y >= y_top)
                        {
                            auto n = coord.x + (coord.y - y_top) * panel.x;
                            count -= n;
                            //todo optimize
                            auto saved = coord;
                            set_coord({ 0, y_top });
                            //todo use ranges
                            grid proto2{ proto.begin() + count, proto.end()};
                            data(n, proto2);
                        }
                        auto data = proto.begin();
                        auto seek = old_coord.x + old_coord.y * panel.x;
                        auto dest = canvas.iter() + seek;
                        auto tail = dest + count;
                        rich::forward_fill_proc(data, dest, tail);
                    }
                    else if (old_coord.y <= y_end)
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
                        auto seek = old_coord.x + old_coord.y * panel.x;
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
                saved = dot_00;
                coord = dot_00;
                basis = 0;
                canvas.wipe();
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
                auto size = canvas.size();
                auto data = canvas.iter();
                auto head = data + std::min<iota>(coord.x + coord.y * size.x,
                                                  panel.x + panel.y * size.x);
                auto tail = canvas.iend();
                while (head != tail) *head++ = brush.spare;
            }
            // alt_screen: Clear all lines from the viewport top line to the current line.
            void del_above() override
            {
                auto size = canvas.size();
                auto head = canvas.iter();
                auto tail = head + std::min<iota>(coord.x + coord.y * size.x,
                                                  panel.x + panel.y * size.x);
                while (head != tail) *head++ = brush.spare;
            }
            // alt_screen: Shift by n the scroll region.
            void scroll_region(iota top, iota end, iota n, bool use_scrollback = faux) override
            {
                canvas.scroll(top, end + 1, n, brush.spare);
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
                line& operator = (line&&)      = default;
                line& operator = (line const&) = default;

                id_t index{};
                ui64 accum{};
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
                auto wrapped() const { return _kind == type::autowrap; }
                iota height(iota width) const
                {
                    auto len = length();
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
                {
                    assert(index < 20000);
                }
            };

            using ring = generics::ring<std::vector<line>, true>;
            using indx = generics::ring<std::vector<index_item>>;

            struct buff : public ring
            {
                using ring::ring;
                using type = line::type;

                iota caret{};
                iota vsize{};
                iota width{};
                id_t taken{};
                ui64 accum{};
                bool dirty{};

                static constexpr id_t threshold = 1000;
                static constexpr ui64 lnpadding = 40; // Padding to improve accuracy.

                //todo optimize for large lines, use std::unordered_map<iota, iota>
                struct maxs : public std::vector<iota>
                {
                    iota max = 0;
                    maxs() : std::vector<iota>(1) { }
                    void prev_max() { while (max > 0 && !at(--max)); }
                }
                lens[type::count];

                void dec_height(type kind, iota size)
                {
                    if (size > width && kind == type::autowrap) vsize -= (size + width - 1) / width;
                    else                                        vsize -= 1;
                }
                void add_height(type kind, iota size)
                {
                    if (size > width && kind == type::autowrap) vsize += (size + width - 1) / width;
                    else                                        vsize += 1;
                }
                // buff: Recalc the height for unlimited scrollback without using reflow.
                void set_width(iota new_width)
                {
                    vsize = 0;
                    width = std::max(1, new_width);
                    for (auto kind : { type::leftside,
                                       type::rghtside,
                                       type::centered })
                    {
                        auto& cur_lens = lens[kind];
                        auto head = cur_lens.begin();
                        auto tail = head + cur_lens.max + 1;
                        do vsize += *head++;
                        while (head != tail);
                    }
                    auto kind = type::autowrap;
                    auto& cur_lens = lens[kind];
                    auto head = cur_lens.begin();
                    auto tail = head + cur_lens.max + 1;
                    auto c = 0;
                    auto h = 1;
                    do
                    {
                        if (auto count = *head++) vsize += h * count;
                        if (++c > width)
                        {
                            c = 1;
                            ++h;
                        }
                    }
                    while (head != tail);
                }

                void invite(type& kind, iota& size, type new_kind, iota new_size)
                {
                    auto& new_lens = lens[new_kind];
                    if (new_lens.size() <= new_size) new_lens.resize(new_size * 2 + 1);

                    ++new_lens[new_size];
                    add_height(new_kind, new_size);
                    if (new_lens.max < new_size) new_lens.max = new_size;

                    size = new_size;
                    kind = new_kind;
                }
                void recalc(type& kind, iota& size, type new_kind, iota new_size)
                {
                    if (size != new_size
                     || kind != new_kind)
                    {
                        auto& new_lens = lens[new_kind];
                        if (new_lens.size() <= new_size) new_lens.resize(new_size * 2 + 1);

                        if (new_size <  size
                         || new_kind != kind) undock(kind, size);
                        else
                        {
                            --lens[kind][size];
                            dec_height(kind, size);
                        }

                        ++new_lens[new_size];
                        add_height(new_kind, new_size);
                        if (new_lens.max < new_size) new_lens.max = new_size;

                        size = new_size;
                        kind = new_kind;
                    }
                }
                void undock(type& kind, iota& size)
                {
                    auto& cur_lens =       lens[kind];
                    auto cur_count = --cur_lens[size];
                    if (size == cur_lens.max && cur_count == 0)
                    {
                        cur_lens.prev_max();
                    }
                    dec_height(kind, size);
                }
                void invite(line& l)
                {
                    dirty = true;
                    invite(l._kind, l._size, l.style.get_kind(), l.length());
                }
                template<class ...Args>
                auto& invite(Args&&... args)
                {
                    dirty = true;
                    auto& l = ring::push_back(std::forward<Args>(args)...);
                    invite(l._kind, l._size, l.style.get_kind(), l.length());
                    return l;
                }
                void undock(line& l) override { undock(l._kind, l._size); }
                template<auto N> auto max() { return lens[N].max; }
                auto index_by_id(ui32 id)
                {
                    //No need to disturb distant objects, it may already be in the swap.
                    auto count = length();
                    return static_cast<iota>(count - 1 - (back().index - id)); // ring buffer size is never larger than max_int32.
                }
                auto iter_by_id(ui32 id)
                {
                    return begin() + index_by_id(id);
                }
                auto& item_by_id(ui32 id)
                {
                    return ring::at(index_by_id(id));
                }
                void recalc_size(iota taken_index)
                {
                    auto head = begin() + std::max(0, taken_index);
                    auto tail = end();
                    auto& curln = *head;
                    auto accum = curln.accum;
                    //auto i = 0;
                    //log("  i=", i++, " curln.accum=", accum);
                    accum += curln.length() + lnpadding;
                    while (++head != tail)
                    {
                        auto& curln = *head;
                        curln.accum = accum;
                        //log("  i=", i++, " curln.accum=", accum);
                        accum += curln.length() + lnpadding;
                    }
                    dirty = faux;
                    //log( " recalc_size taken_index=", taken_index);
                }
                auto get_size_in_cells()
                {
                    auto& endln = back();
                    auto& endid = endln.index;
                    auto  count = length();
                    auto  taken_index = static_cast<iota>(count - 1 - (endid - taken));
                    if (taken != endid || dirty)
                    {
                        auto& topln = front();
                        recalc_size(taken_index);
                        taken = endln.index;
                        accum = endln.accum
                            + endln.length() + lnpadding
                            - topln.accum;
                        //log(" topln.accum=", topln.accum,
                        //    " endln.accum=", endln.accum,
                        //    " vsize=", vsize,
                        //    " accum=", accum);
                    }
                    return accum;
                }
                void recalc(line& l)
                {
                    recalc(l._kind, l._size, l.style.get_kind(), l.length());
                    dirty = true;
                    auto taken_index = index_by_id(taken);
                    auto curln_index = index_by_id(l.index);
                    if (curln_index < taken_index)
                    {
                        taken       =     l.index;
                        taken_index = curln_index;
                    }
                    if (ring::size - taken_index > threshold)
                    {
                        recalc_size(taken_index);
                        taken = back().index;
                    }
                }
                auto remove(iota at, iota count)
                {
                    count = ring::remove(at, count);
                    auto head = begin() + at;
                    auto tail = end();
                    while (head != tail)
                    {
                        head->index -= count;
                        ++head;
                    }
                    return count;
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

            buff batch; // scroll_buf: Rods inner container.
            flow maker; // scroll_buf: . deprecated
            indx index; // scroll_buf: Viewport line index.
            iota vsize; // scroll_buf: Scrollback vertical size (height).
            face sctop_panel;
            face scend_panel;
            iota region_size{ 1 };

            scroll_buf(term& boss, iota buffer_size, iota grow_step)
                : bufferbase{ boss                     },
                       batch{ buffer_size, grow_step   },
                       maker{ batch.width, batch.vsize },
                       index{ 1                        },
                       vsize{ 1                        }
            {
                batch.invite(0); // At least one line must exist.
                batch.set_width(1);
                index_rebuild();
            }
            iota get_size() const override { return batch.size; }
            iota get_peak() const override { return batch.peak; }
            iota get_step() const override { return batch.step; }

            void print_index(text msg)
            {
                log(" ", msg, " index.size=", index.size, " basis=", basis);
                for (auto n = 0; auto l : index)
                {
                    log("  ", n++,". id=", l.index," offset=", l.start, " width=", l.width);
                    if (l.start % panel.x != 0) throw;
                }
                auto& mapln = index.back();
                auto& curln = batch.item_by_id(mapln.index);
                log(" last ln id=", curln.index, " curln.length()=", curln.length());
                log(" -----------------");
            }
            void print_batch(text msg)
            {
                log(" ", msg, " batch.size=", batch.size);
                for (auto n = 0; auto l : batch)
                {
                    log("  ", n++,". id=", l.index, " length()=", l.length());
                }
                log(" -----------------");
            }
            auto test_futures()
            {
                auto stash = batch.vsize - basis - index.size;
                assert(stash >= 0);
                return true;
            }
            auto test_resize()
            {
                auto c = batch.caret;
                sync_coord();
                assert(c == batch.caret);
                return true;
            }
            iota get_oversize() override
            {
                return std::max(0, batch.vsize - (basis + region_size));
            }
            //scroll_buf: Return viewport vertical oversize.
            void resize_viewport(twod const& new_sz) override
            {
                auto in_top = y_top - coord.y;
                auto in_end = coord.y - y_end;

                bufferbase::resize_viewport(new_sz);

                batch.set_width(panel.x);
                index.clear();

                if (sctop_panel.core::size().x != panel.x)
                {
                    //todo check cursor position
                    sctop_panel.crop(twod{ panel.x, sctop }, sctop_panel.mark());
                    scend_panel.crop(twod{ panel.x, scend }, scend_panel.mark());
                }
                region_size = y_end - y_top + 1;
                index.resize(region_size); // Use a fixed ring because new lines are added much more often than a futures feed.

                if (in_top > 0 || in_end > 0) // The cursor is outside the scrolling region.
                {
                    if (in_top > 0) coord.y = std::max(0          , y_top - in_top);
                    else            coord.y = std::min(panel.y - 1, y_end + in_end);
                    index_rebuild();
                    return;
                }

                basis = batch.vsize;
                auto lnid = batch.current().index;
                auto head = batch.end();
                auto maxn = batch.size - batch.index();
                auto tail = head - std::max(maxn, std::min(batch.size, region_size));
                auto push = [&](auto i, auto o, auto r) { --basis; index.push_front(i, o, r); };
                auto unknown = true;
                while (head != tail && (index.size < region_size || unknown))
                {
                    auto& curln = *--head;
                    auto length = curln.length();
                    auto active = curln.index == lnid;
                    if (curln.wrapped())
                    {
                        auto offset = length;
                        auto remain = length ? (length - 1) % panel.x + 1
                                             : 0;
                        do
                        {
                            offset -= remain;
                            push(curln.index, offset, remain);
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
                        while (offset > 0 && (index.size < region_size || unknown));
                    }
                    else
                    {
                        push(curln.index, 0, length);
                        if (active)
                        {
                            unknown = faux;
                            coord.y = index.size;
                            coord.x = batch.caret;
                        }
                    }
                }
                coord.y = index.size - coord.y + y_top;

                assert(basis >= 0);
                assert(test_futures());
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
                    index.push_back(curid, start, width - start);

                    auto& curln = *++curit;
                    width = curln.length();
                    wraps = curln.wrapped();
                    curid = curln.index;
                    start = 0;
                }
                assert(test_futures());
            }
            // scroll_buf: Rebuild index from the known index at y_pos.
            void index_rebuild_from(iota y_pos)
            {
                assert(y_pos >= 0 && y_pos < index.size);
                auto  mapit = index.begin() + y_pos++;
                auto& mapln =*mapit;
                auto  curit = batch.iter_by_id(mapln.index);
                auto  avail = std::min(batch.vsize - basis - y_pos, region_size - y_pos);
                auto  count = index.size - y_pos;
                while (count-- > 0) index.pop_back();

                reindex(avail, curit, mapln);
            }
            // scroll_buf: Rebuild index up to basis.
            void index_rebuild()
            {
                index.clear();
                auto coor = batch.vsize;
                auto head = batch.end();
                while (coor != basis)
                {
                    auto& curln = *--head;
                    auto length = curln.length();
                    if (curln.wrapped())
                    {
                        auto remain = length ? (length - 1) % panel.x + 1 : 0;
                        length -= remain;
                        index.push_front(curln.index, length, remain);
                        --coor;
                        while (length > 0 && coor != basis)
                        {
                            length -= panel.x;
                            index.push_front(curln.index, length, panel.x);
                            --coor;
                        }
                    }
                    else
                    {
                        index.push_front(curln.index, 0, length);
                        --coor;
                    }
                }
                assert(test_futures());
            }
            // scroll_buf: Return scrollback height.
            iota height() override
            {
                auto test_vsize = 0; //sanity check
                for (auto& l : batch) test_vsize += l.height(panel.x);
                if (test_vsize != batch.vsize) log(" ERROR! test_vsize=", test_vsize, " vsize=", batch.vsize);
                return batch.vsize;
            }
            // scroll_buf: Recalc left and right oversize.
            bool recalc_pads(side& oversz) override
            {
                auto left = std::max(0, batch.max<line::type::rghtside>() - panel.x);
                auto rght = std::max(0, batch.max<line::type::leftside>() - panel.x);
                auto cntr = std::max(0, batch.max<line::type::centered>() - panel.x);
                auto both = cntr >> 1;
                left = std::max(left, both);
                rght = std::max(rght, both + (cntr & 1));
                if (oversz.r != rght || oversz.l != left)
                {
                    oversz.r = rght;
                    oversz.l = left;
                    return true;
                }
                else return faux;
            }
            // scroll_buf: Check if there are futures, use them when scrolling regions.
            auto feed_futures(iota query)
            {
                assert(test_futures());
                assert(query > 0);

                auto stash = batch.vsize - basis - index.size;
                if (stash > 0)
                {
                    auto avail = std::min(stash, query);
                    basis   += avail;
                    query   -= avail;
                    coord.y -= avail;

                    auto& mapln = index.back();
                    auto  curit = batch.iter_by_id(mapln.index);

                    reindex(avail, curit, mapln);
                }

                return query;
            }
            // scroll_buf: Return current 0-based cursor position in the scrollback.
            twod get_coord(twod const& origin) override
            {
                auto coor = coord;
                if (coord.y >= y_top
                 && coord.y <= y_end)
                {
                    coor.y += basis;

                    auto visible = coor.y + origin.y;
                    if (visible < y_top // Do not show cursor behind margins.
                     || visible > y_end) return dot_mx;

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
            void sync_coord()
            {
                coord.y = std::clamp(coord.y, 0, panel.y - 1);

                if (coord.y >= y_top
                 && coord.y <= y_end)
                {
                    auto& curln = batch.current();
                    auto  curid = curln.index;
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
                        if (batch->style != parser::style) _set_style(parser::style);
                    }
                    coord.y += y_top;
                }
            }

            void cup (fifo& q) override { bufferbase::cup (q); sync_coord(); }
            void tab (iota  n) override { bufferbase::tab (n); sync_coord(); }
            void scl (iota  n) override { bufferbase::scl (n); sync_coord(); }
            void cuf (iota  n) override { bufferbase::cuf (n); sync_coord(); }
            void chx (iota  n) override { bufferbase::chx (n); sync_coord(); }
            void chy (iota  n) override { bufferbase::chy (n); sync_coord(); }
            void il  (iota  n) override { bufferbase::il  (n); sync_coord(); }
            void dl  (iota  n) override { bufferbase::dl  (n); sync_coord(); }
            void up  (iota  n) override { bufferbase::up  (n); sync_coord(); }
            void dn  (iota  n) override { bufferbase::dn  (n); sync_coord(); }
            void ri  ()        override { bufferbase::ri  ( ); sync_coord(); }
            void home()        override { bufferbase::home( ); sync_coord(); }

            // scroll_buf: Set the scrolling region using 1-based top and bottom. Use 0 to reset.
            void set_scroll_region(iota top, iota bottom) override
            {
                auto old_sctop = y_top;
                auto old_scend = panel.y - 1 - y_end;

                bufferbase::set_scroll_region(top, bottom);
                log(" top=", top, " bottom=", bottom, " panel=", panel);
                log(" old_sctop=", old_sctop, " old_scend=", old_scend);
                log(" sctop=", sctop, " scend=", scend);

                //todo if top    == 0 || 1         push sctop_panel to scroll buffer
                //todo if bottom == 0 || panel.y-1 push scend_panel to scroll buffer

                auto top_dy = sctop_panel.core::size().y - old_sctop;
                auto end_dy = scend_panel.core::size().y - old_scend;
                auto top_size = twod{ panel.x, sctop + top_dy };
                auto btm_size = twod{ panel.x, scend + end_dy };

                auto delta_top = sctop - old_sctop;
                auto delta_end = scend - old_scend;

                auto pull = [&](face& block, twod origin, iota begin, iota limit)
                {
                    //todo check bounds

                    dissect(begin);
                    dissect(limit);
                    auto from = index[begin    ].index;
                    auto upto = index[limit - 1].index + 1;
                    auto head = batch.iter_by_id(from);
                    auto size = static_cast<iota>(upto - from);
                    auto tail = head + size;
                    auto view = rect{ origin, { panel.x, limit - begin }};
                    auto full = rect{ dot_00, block.core::size()        };
                    block.full(full);
                    block.view(view);
                    block.ac(view.coor);
                    log(" view=", view, " full=", full);
                    do
                    {
                        auto& curln = *head;
                        block.output(curln);
                        block.nl(1);
                        log(" curln.id=", curln.index, " text=", curln.to_txt());
                    }
                    while (++head != tail);
                    batch.remove(from, size);
                };

                auto push = [&](face& block, iota start, iota count, iota where)
                {
                    auto curid = index[where].index;
                    // ...

                };

                if (delta_end > 0) // Bottom margin processed first (don't touch top indices).
                {
                    if (scend_panel.core::size().y == 0) scend_panel.mark(brush.spare);
                    scend_panel.crop<true>(btm_size, scend_panel.mark());
                    pull(scend_panel, dot_00, region_size - delta_end, region_size);
                }
                else if (delta_end < 0)
                {
                    push(scend_panel, 0, delta_end, region_size - 1);
                    scend_panel.crop<true>(btm_size);
                }

                if (delta_top > 0)
                {
                    if (sctop_panel.core::size().y == 0) sctop_panel.mark(brush.spare);
                    sctop_panel.crop<faux>(top_size, sctop_panel.mark());
                    pull(sctop_panel, { 0, top_dy + old_sctop }, 0, delta_top);
                }
                else if (delta_top < 0)
                {
                    push(scend_panel, sctop, delta_top, 0);
                    sctop_panel.crop<faux>(top_size);
                }

                region_size = panel.y - (scend + sctop);
                index.clear();
                index.resize(region_size);
                index_rebuild();
                sync_coord();
            }
            // scroll_buf: Push lines to the scrollback bottom.
            void add_lines(iota amount)
            {
                assert(amount >= 0);
                auto newid = batch.back().index;
                auto style = batch->style;
                while (amount-- > 0)
                {
                    auto& l = batch.invite(++newid, style);
                    index.push_back(l.index, 0, 0);
                }
            }
            // scroll_buf: . (! Check coord.y context)
            void _set_style(deco const& new_style)
            {
                auto& curln = batch.current();
                auto  wraps = curln.wrapped();
                auto  width = curln.length();
                curln.style = new_style;
                batch.recalc(curln);

                if (wraps != curln.wrapped())
                {
                    if (batch.caret >= panel.x)
                    {
                        if (wraps)
                        {
                            coord.x  =  batch.caret;
                            coord.y -= (batch.caret - 1) / panel.x + 1;
                            if (coord.y < 0)
                            {
                                basis -= std::abs(coord.y);
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
                            if (coord.y >= region_size)
                            {
                                auto limit = region_size - 1;
                                auto delta = coord.y - limit;
                                basis  += delta;
                                coord.y = limit;
                            }
                        }
                        index_rebuild();
                    }
                    else
                    {
                        auto& mapln = index[coord.y];
                        auto  width = curln.length();
                        mapln.start = 0;
                        mapln.width = curln.wrapped() ? std::min(panel.x, width)
                                                      : width;
                        index_rebuild_from(coord.y);
                    }
                }
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
                struct q //todo revise
                {
                    twod& c;
                    iota  t;
                    bool  b;
                    rich& block;
                    q(twod& cy, iota ct, bool cb, scroll_buf& cs)
                        : c{ cy },
                          t{ ct },
                          b{ cb },
                          block{ cy > cs.y_end ? t = cs.y_end + 1, cs.scend_panel
                                               :                   cs.sctop_panel }
                        { c.y -= t; }
                   ~q() { c.y += t; }
                   operator bool () { return b; }
                };
                auto inside = coord.y >= y_top
                           && coord.y <= y_end;
                return q{ pos, inside ? y_top : 0, inside, *this };
            }
            // scroll_buf: CSI n K  Erase line (don't move cursor).
            void el(iota n) override
            {
                bufferbase::flush();
                auto blank = brush.spc(); //.bgc(greendk).bga(0x7f);
                if (auto ctx = get_context(coord))
                {
                    iota  start;
                    iota  count;
                    auto  caret = std::max(0, batch.caret); //todo why?
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
                            count = caret - start;
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

                        //batch->shrink(blank); // It kills wrapped lines and as a result requires the viewport to be rebuilt.
                        batch.recalc(curln);

                        auto& mapln = index[coord.y];
                        mapln.width = std::min(panel.x, curln.length() - mapln.start);
                    }
                }
                else alt_screen::_el(n, ctx.block, coord, panel, blank);
            }
            // scroll_buf: CSI n @  ICH. Insert n blanks after cursor. Existing chars after cursor shifts to the right. Don't change cursor pos.
            void ins(iota n) override
            {
                bufferbase::flush();
                auto blank = brush.spc(); //.bgc(magentadk).bga(0x7f);
                if (auto ctx = get_context(coord))
                {
                    n = std::min(n, panel.x - coord.x);
                    auto& curln = batch.current();
                    curln.insert(batch.caret, n, blank, panel.x);
                    batch.recalc(curln); // Line front is filled by blanks. No wrapping.
                    auto& mapln = index[coord.y];
                    mapln.width = std::min(panel.x, curln.length() - mapln.start);
                }
                else ctx.block.insert(coord, n, blank);
            }
            // scroll_buf: CSI n P  Delete (not Erase) letters under the cursor. Line end is filled by blanks. Length is preserved. No wrapping.
            void dch(iota n) override
            {
                bufferbase::flush();
                auto blank = brush.spc(); //.bgc(magentadk).bga(0x7f);
                if (auto ctx = get_context(coord))
                {
                    auto& curln = batch.current();
                    curln.cutoff(batch.caret, n, blank, panel.x);
                }
                else ctx.block.cutoff(coord, n, blank);
            }
            // scroll_buf: CSI n X  Erase/put n chars after cursor. Don't change cursor pos.
            void ech(iota n) override
            {
                parser::flush();
                auto blank = brush.spc(); //.bgc(magentadk).bga(0x7f);
                if (auto ctx = get_context(coord))
                {
                    n = std::min(n, panel.x - coord.x);
                    auto& curln = batch.current();
                    curln.splice(batch.caret, n, blank);
                    batch.recalc(curln);
                    auto& mapln = index[coord.y];
                    mapln.width = std::min(panel.x, curln.length() - mapln.start);
                }
                else ctx.block.splice(coord, n, blank);
            }
            // Reserved for future use.
            // scroll_buf: Insert count blanks with scroll.
            //void ech_grow(iota n) override
            //{
            //    parser::flush();
            //    if (auto ctx = inside_scroll(coord.y))
            //    {
            //      auto& curln = batch.current();
            //      auto  blank = brush.spc(); //.bgc(magentadk).bga(0x7f);
            //      curln.splice<true>(batch.caret, n, blank);
            //      batch.recalc(curln);
            //      //todo check_autogrow
            //      //todo move cursor and auto scroll (same as data())
            //      //todo reindex
            //    }
            //}

            // scroll_buf: Proceed new text (parser callback).
            void data(iota count, grid const& proto) override
            {
                assert(coord.y >= 0 && coord.y < panel.y);
                assert(test_futures());

                if (coord.y < y_top)
                {
                    auto old_coord = coord;
                    coord.x += count;
                    //todo apply line adjusting
                    if (coord.x <= panel.x)//todo styles! || ! curln.wrapped())
                    {
                        auto n = std::min(count, panel.x - std::max(0, old_coord.x));
                        sctop_panel.splice(old_coord, n, proto);
                    }
                    else
                    {
                        coord.y += (coord.x + panel.x - 1) / panel.x - 1;
                        coord.x  = (coord.x           - 1) % panel.x + 1;

                        if (coord.y >= y_top)
                        {
                            auto n = coord.x + (coord.y - y_top) * panel.x;
                            count -= n;
                            //todo optimize
                            auto saved = coord;
                            set_coord({ 0, y_top });
                            //todo use ranges
                            grid proto2{ proto.begin() + count, proto.end()};
                            data(n, proto2);
                        }
                        auto data = proto.begin();
                        auto seek = old_coord.x + old_coord.y * panel.x;
                        auto dest = sctop_panel.iter() + seek;
                        auto tail = dest + count;
                        rich::forward_fill_proc(data, dest, tail);
                    }
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
                        auto max_y = region_size - 1;
                        auto saved = coord.y + basis;
                        coord.y += (coord.x + panel.x - 1) / panel.x - 1;
                        coord.x  = (coord.x           - 1) % panel.x + 1;

                        auto query = coord.y - (index.size - 1);
                        auto addln = query > 0 ? feed_futures(query)
                                               : 0;
                        curln.splice(start, count, proto);
                        auto curid = curln.index;
                        if (addln > 0) // case 3 - complex: Cursor is outside the viewport. 
                        {              // cursor overlaps some lines below and placed below the viewport.
                            batch.recalc(curln);
                            if (auto count = static_cast<iota>(batch.back().index - curid))
                            {
                                assert(count > 0);
                                while (count-- > 0) batch.pop_back();
                            }

                            auto width = curln.length();
                            auto trail = width - panel.x;

                            saved -= basis;
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
                                basis  += coord.y - max_y;
                                coord.y = max_y;
                            }

                            assert(test_futures());
                        } // case 3 done
                        else
                        {
                            auto& mapln = index[coord.y];
                            if (curid == mapln.index) // case 1 - plain: cursor is inside the current paragraph.
                            {
                                if (coord.x > mapln.width)
                                {
                                    mapln.width = coord.x;
                                    batch.recalc(curln);
                                }
                                else assert(curln._size == curln.length());

                                assert(test_futures());
                            } // case 1 done.
                            else // case 2 - fusion: cursor overlaps lines below but stays inside the viewport.
                            {
                                auto& target = batch.item_by_id(mapln.index);
                                auto  shadow = target.wrapped() ? target.substr(mapln.start + coord.x)
                                                                : target.substr(mapln.start + coord.x, mapln.width - coord.x);
                                curln.splice(curln.length(), shadow);
                                batch.recalc(curln);
                                auto width = curln.length();
                                auto spoil = static_cast<iota>(mapln.index - curid);
                                assert(spoil > 0);
                                auto after = batch.index() + 1;
                                     spoil = batch.remove(after, spoil);
                                // Update index.
                                {
                                    saved -= basis;
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
                                }

                                assert(test_futures());
                            } // case 2 done.
                        }
                    }
                    assert(coord.y >= 0 && coord.y < region_size);
                    //log(" bufferbase size in cells = ", batch.get_size_in_cells());
                    coord.y += y_top;
                }
                else
                {
                    coord.y -= y_end + 1;
                    auto old_coord = coord;
                    coord.x += count;
                    //todo apply line adjusting
                    if (coord.x <= panel.x)//todo styles! || ! curln.wrapped())
                    {
                        auto n = std::min(count, panel.x - std::max(0, old_coord.x));
                        scend_panel.splice(old_coord, n, proto);
                    }
                    else
                    {
                        coord.y += (coord.x + panel.x - 1) / panel.x - 1;
                        coord.x  = (coord.x           - 1) % panel.x + 1;

                        auto data = proto.begin();
                        auto size = count;
                        auto seek = old_coord.x + old_coord.y * panel.x;
                        auto dest = scend_panel.iter() + seek;
                        auto tail = scend_panel.iend();
                        auto back = panel.x;
                        rich::unlimit_fill_proc(data, size, dest, tail, back);
                    }
                    coord.y = std::min(coord.y + y_end + 1, panel.y - 1);
                }
            }
            // scroll_buf: Clear scrollback.
            void clear_all() override
            {
                saved = dot_00;
                coord = dot_00;
                batch.clear();
                batch.invite(0); // At least one line must exist.
                batch.set_width(panel.x);
                basis = 0;
                index_rebuild();
                bufferbase::clear_all();
            }
            // scroll_buf: Set scrollback limits.
            void resize_history(iota new_size, iota grow_by = 0)
            {
                static constexpr auto BOTTOM_ANCHORED = true;
                //set_scroll_region(0, 0);
                batch.resize<BOTTOM_ANCHORED>(new_size, grow_by);
                index_rebuild();
            }
            // scroll_buf: Render to the canvas.
            void output(face& canvas) override //todo temp solution, rough output, not optimized
            {
                maker.reset(canvas);
                //temp solution
                auto maker_full = maker.full();
                maker_full.coor.y += y_top;
                maker.full(maker_full);

                auto view = canvas.view();
                auto full = canvas.full();
                auto coor = dot_00;
                auto head = batch.begin();
                auto tail = batch.end();
                auto left_edge_x = view.coor.x;
                auto half_size_x = full.size.x / 2;
                auto left_rect = rect{{ left_edge_x, coor.y + full.coor.y }, dot_11 };
                auto rght_rect = left_rect;
                rght_rect.coor.x+= view.size.x - 1;
                auto rght_edge_x = rght_rect.coor.x + 1;
                auto fill = [&](auto& area, auto chr)
                {
                    if (auto r = view.clip(area))
                        canvas.fill(r, [&](auto& c){ c.txt(chr).fgc(tint::greenlt); });
                };
                while (head != tail)
                {
                    auto& curln = *head;
                    maker.ac(coor);
                    maker.go(curln, canvas);
                    auto height = curln.height(panel.x);
                    auto align = curln.style.jet();
                    if (auto length = curln.length()) // Mark lines not shown in full.
                    {
                        rght_rect.size.y = left_rect.size.y = height;
                        if (height == 1)
                        {
                            auto lt_dot = full.coor.x;
                            if      (align == bias::center) lt_dot += half_size_x - length / 2;
                            else if (align == bias::right)  lt_dot += full.size.x - length;

                            if (left_edge_x > lt_dot         ) fill(left_rect, '<');
                            if (rght_edge_x < lt_dot + length) fill(rght_rect, '>');
                        }
                        else
                        {
                            auto lt_dot = full.coor.x;
                            auto rt_dot = lt_dot + view.size.x;
                            auto remain = (length - 1) % view.size.x + 1;
                            if (left_edge_x > lt_dot)
                            {
                                if (align == bias::right  && left_edge_x <= rt_dot - remain
                                 || align == bias::center && left_edge_x <= lt_dot + half_size_x - remain / 2)
                                {
                                    --left_rect.size.y;
                                }
                                fill(left_rect, '<');
                            }
                            if (rght_edge_x < rt_dot)
                            {
                                if (align == bias::left   && rght_edge_x >= lt_dot + remain
                                 || align == bias::center && rght_edge_x >= lt_dot + remain + half_size_x - remain / 2)
                                {
                                    --rght_rect.size.y;
                                }
                                fill(rght_rect, '>');
                            }
                        }
                    }
                    coor.y           += height;
                    rght_rect.coor.y += height;
                    left_rect.coor.y = rght_rect.coor.y;
                    ++head;
                }

                {
                    auto view = canvas.view();
                    auto top_coor = twod{ view.coor.x, view.coor.y + y_top - sctop };
                    auto end_coor = twod{ view.coor.x, view.coor.y + y_end + 1     };
                    sctop_panel.move(top_coor);
                    scend_panel.move(end_coor);
                    canvas.plot(sctop_panel, [](auto& dst, auto& src){ dst.fuse(src); dst.bgc(greendk).bga(0x80); });
                    canvas.plot(scend_panel, [](auto& dst, auto& src){ dst.fuse(src); dst.bgc(reddk).bga(0x80); });

                    //canvas.plot(sctop_panel, cell::shaders::flat);
                    //canvas.plot(scend_panel, cell::shaders::flat);
                }
            }
            // scroll_buf: Remove all lines below (including futures) except the current. "ED2 Erase viewport" keeps empty lines.
            void del_below() override
            {
                //todo TIA scrolling region

                auto n = batch.size - 1 - batch.index();
                auto m = index.size - 1 - coord.y;
                auto p = panel.y    - 1 - coord.y;

                if (coord.x == 0 && batch.caret != 0) // Remove the index of the current line if the entire visible line is to be removed.
                {
                    ++m;
                    ++p;
                }

                assert(n >= 0 && n <  batch.size);
                assert(m >= 0 && m <= index.size);
                assert(p >= 0 && p <= panel.y   );

                while (n--) batch.pop_back();
                while (m--) index.pop_back();

                add_lines(p);

                auto& curln = batch.current();
                auto& mapln = index[coord.y];
                mapln.width = coord.x;
                curln.trimto(batch.caret);
                batch.recalc(curln);

                assert(test_futures());
                //print_index("del_below");
            }
            // scroll_buf: Clear all lines from the viewport top line to the current line.
            void del_above() override
            {
                //todo TIA scrolling region

                // The dirtiest and fastest solution. Just fill existing lines by blank cell.
                auto& curln = batch.current();
                auto& topln = index.front();
                auto  curit = batch.iter_by_id(topln.index);
                auto  endit = batch.iter_by_id(curln.index);
                auto  start = topln.start;
                auto  blank = brush.spc();
                if (curit == endit)
                {
                    auto width = std::min(curln.length(), batch.caret);
                    curln.splice(start, width - start, blank);
                }
                else
                {
                    auto& curln =*curit;
                    auto  width = curln.length();
                    curln.splice(start, width - start, blank);

                    while(++curit != endit) curit->core::wipe(blank);

                    if (coord.x > 0)
                    {
                        auto& curln =*curit;
                        auto  max_x = std::min<iota>(curln.length(), batch.caret);
                        if (max_x > 0)
                        {
                            auto max_w = curln.wrapped() ? (max_x - 1) % panel.x + 1
                                                         :  max_x;
                            auto width = std::min<iota>(max_w, coord.x);
                            auto start = batch.caret - coord.x;
                            curln.splice(start, width, blank);
                        }
                    }
                }
            }
            // scroll_buf: Dissect auto-wrapped lines at the specified row in scroll region (incl last line+1).
            void dissect(iota y_pos)
            {
                assert(y_pos >= 0 && y_pos <= region_size);

                auto split = [&](id_t curid, iota start)
                {
                    auto after = batch.index_by_id(curid);
                    auto tmpln = std::move(batch[after]);
                    auto curit = batch.insert(after, tmpln.index, tmpln.style);
                    auto endit = batch.end();

                    auto& newln = *curit;
                    newln.splice(0, tmpln.substr(start));
                    batch.undock(tmpln);
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
                else if (y_pos == region_size
                      && y_pos <  batch.vsize - basis)
                {
                    auto stash = batch.vsize - basis - index.size;
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
            }
            // scroll_buf: Zeroize block of lines.
            template<class SRC>
            void zeroise(SRC begin_it, SRC end_it)
            {
                while (begin_it != end_it)
                {
                    auto& curln = *begin_it;
                    curln.core::crop(0);
                    batch.recalc(curln);
                    //batch.undock(curln);
                    //curln.core::wipe(); // Fill using background.
                    ++begin_it;
                }
            }
            // scroll_buf: Shift by n the scroll region.
            void scroll_region(iota top, iota end, iota n, bool use_scrollback) override
            {
                //todo don't dissect if top==y_top on moving text block up, see dn(iota n).

                //temp solution
                if (n < 0 && top == y_top && end == y_end)
                {
                    auto required_lines = -n;
                    auto add_count = required_lines > 0 ? feed_futures(required_lines)
                                                        : 0;
                    if (add_count > 0)
                    {
                        add_lines(add_count);
                        basis += add_count;
                        coord.y -= std::min(coord.y, add_count);
                    }

                    assert(test_futures());
                }
                else
                {

                }

                /*
                if (n)
                {
                    auto[top, end] = get_scroll_region();
                    auto nul_it = batch.begin();
                    auto all_it = nul_it + basis;
                    auto end_it = nul_it + end;
                    auto top_it = nul_it + top;
                    auto top_id = std::max(0, basis - all_it->depth);
                    auto height = end - top + 1;
                    auto footer = batch.end() - end_it - 1;
                    if (footer < 0)
                    {
                        add_lines(-footer);
                        footer = 0;
                    }
                    if (n > 0) // Scroll down (move text block down).
                    {
                        n = std::min(n, height);
                        auto a = top_it - 1;
                        auto b = end_it - n;
                        auto c = end_it + 1;
                        dissect(b + 1);
                        dissect(top_it);
                        if (footer) dissect(c);
                        zeroise(b + 1, c);
                        netxs::move_block(b, a, end_it);
                        zeroise(top_it, top_it + n);
                    }
                    else // Scroll up (move text block up).
                    {
                        n = std::min(-n, height);
                        auto a = top_it + n;
                        if (use_scrollback)
                        {
                            if (top)
                            {
                                auto buffer = cache.begin();
                                if (basis) dissect(all_it);
                                dissect(top_it);
                                dissect(a);
                                netxs::move_block(all_it, top_it,       buffer    ); // Move fixed header block to the temporary cache.
                                netxs::move_block(top_it, a,            all_it    ); // Move up by the "top" the first n lines of scrolling region.
                                netxs::move_block(buffer, buffer + top, all_it + n); // Move back fixed header block from the temporary cache.
                            }
                            add_lines(n);
                            auto c = batch.end() - 1;
                            auto b = c - n;
                            auto d = b - footer;
                            dissect(d + 1);
                            netxs::move_block(b, d, c); // Move down footer block by n.
                        }
                        else
                        {
                            auto b = end_it + 1;
                            dissect(a);
                            if (footer) dissect(b);
                            zeroise(top_it, a);
                            netxs::move_block(a, b, top_it);
                        }
                    }
                }
                */
            }
        };

        using buffer_ptr = bufferbase*;

        pro::caret cursor; // term: Text cursor controller.
        term_state status; // term: Screen buffer status info.
        w_tracking wtrack; // term: Terminal title tracking object.
        f_tracking ftrack; // term: Keyboard focus tracking object.
        m_tracking mtrack; // term: VT-style mouse tracking object.
        scroll_buf normal; // term: Normal    screen buffer.
        alt_screen altbuf; // term: Alternate screen buffer.
        buffer_ptr target; // term: Current   screen buffer pointer.
        os::ptydev ptycon; // term: PTY device.
        text       cmdarg; // term: Startup command line arguments.
        hook       oneoff; // term: One-shot token for the first resize and shutdown events.
        twod       origin; // term: Viewport position.
        bool       active; // term: Terminal lifetime.
        bool       decckm; // term: Cursor keys Application(true)/ANSI(faux) mode.
        bool       bpmode; // term: Bracketed paste mode.

        // term: Soft terminal reset (DECSTR).
        void decstr()
        {
            target->flush();
            normal.clear_all();
            altbuf.clear_all();
            set_scroll_region(0, 0);
            target = &normal;
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
                        altbuf.panel = target->panel;
                        altbuf.style = target->style;
                        target = &altbuf;
                        altbuf.clear_all();
                        base::resize(altbuf.panel);
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
                        normal.panel = target->panel;
                        normal.style = target->style;
                        target = &normal;
                        base::resize(normal.panel);
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
        // term: Proceed terminal input.
        void ondata(view data)
        {
            while (active)
            {
                netxs::events::try_sync guard;
                if (guard)
                {
                    SIGNAL(tier::general, e2::debug::output, data); // Post for the Logs.

                auto force_basis = origin.y == -target->basis;

                //log(" coord=", target->coord, " data: ", utf::debase(data));

                    ansi::parse(data, target);

                scroll(force_basis);

                    base::deface();
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
                SUBMIT_T(tier::general, e2::tick, oneoff, t)
                {
                    oneoff.reset();
                    base::destroy();
                };
            }
        }
        // term: Reset viewport position.
        void scroll(bool force_basis = true)
        {
            auto& console = *target;
            auto adjust_pads = console.recalc_pads(oversz);
            auto scroll_size = console.panel;
            scroll_size.y += console.basis;
            oversz.b = console.get_oversize();
            //if (force_basis)
            {
                origin.y = -console.basis;
                this->SIGNAL(tier::release, e2::coor::set, origin);
            }
            if (scroll_size != base::size() || adjust_pads)
            {
                this->SIGNAL(tier::release, e2::size::set, scroll_size); // Update scrollbars.
            }
        }

    public:
       ~term(){ active = faux; }
        term(text command_line, iota max_scrollback_size = def_length, iota grow_step = def_growup)
            : normal{ *this, max_scrollback_size, grow_step },
              altbuf{ *this },
              cursor{ *this },
              mtrack{ *this },
              ftrack{ *this },
              wtrack{ *this },
              active{  true },
              decckm{  faux },
              bpmode{  faux }
        {
            cmdarg = command_line;
            target = &normal;
            cursor.style(commands::cursor::def_style);

            #ifdef PROD
            form::keybd.accept(true); // Subscribe to keybd offers.
            #endif
            base::broadcast->SUBMIT_T(tier::preview, app::term::events::cmd, bell::tracker, cmd)
            {
                log("term: tier::preview, app::term::cmd, ", cmd);
                scroll();
                switch (cmd)
                {
                    case term::commands::ui::left:
                        target->style.jet(bias::left);
                        break;
                    case term::commands::ui::center:
                        target->style.jet(bias::center);
                        break;
                    case term::commands::ui::right:
                        target->style.jet(bias::right);
                        break;
                    case term::commands::ui::togglewrp:
                        target->style.wrp(target->style.wrp() == wrap::on ? wrap::off : wrap::on);
                        break;
                    case term::commands::ui::reset:
                        decstr();
                        break;
                    case term::commands::ui::clear:
                        target->ed(commands::erase::display::viewport);
                        break;
                    default:
                        break;
                }
                ondata("");
            };
            base::broadcast->SUBMIT_T(tier::preview, app::term::events::data::in, bell::tracker, data)
            {
                log("term: app::term::data::in, ", utf::debase(data));
                scroll();
                ondata(data);
            };
            base::broadcast->SUBMIT_T(tier::preview, app::term::events::data::out, bell::tracker, data)
            {
                log("term: app::term::data::out, ", utf::debase(data));
                scroll();
                ptycon.write(data);
            };
            SUBMIT(tier::release, e2::coor::set, new_coor)
            {
                origin = new_coor;
            };
            SUBMIT(tier::release, e2::form::upon::vtree::attached, parent)
            {
                this->base::riseup<tier::request>(e2::form::prop::header, wtrack.get(ansi::OSC_TITLE));


                this->SUBMIT_T(tier::release, e2::size::set, oneoff, new_sz)
                {
                    if (new_sz.y > 0)
                    {
                        auto& console = *target;
                        oneoff.reset();

                        new_sz = std::max(new_sz, dot_11);
                        console.resize_viewport(new_sz);
                        oversz.b = console.get_oversize();
                        //screen.coor.y = -target->basis;;

                        this->SUBMIT(tier::preview, e2::size::set, new_sz)
                        {
                            auto& console = *target;
                auto force_basis = origin.y == -console.basis;

                            new_sz = std::max(new_sz, dot_11);
                            console.resize_viewport(new_sz);
                            oversz.b = console.get_oversize();
                            //screen.coor.y = -target->basis;;

                scroll(force_basis);

                            ptycon.resize(new_sz);

                            new_sz.y = console.basis + new_sz.y;
                        };

                        ptycon.start(cmdarg, new_sz, [&](auto utf8_shadow) { ondata(utf8_shadow);  },
                                                     [&](auto exit_reason) { onexit(exit_reason); });
                    }
                };
            };
            SUBMIT(tier::release, hids::events::keybd::any, gear)
            {
                //todo stop/finalize scrolling animations
                scroll();
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
            SUBMIT(tier::release, e2::render::any, parent_canvas)
            {
                //log(" 1. screen=", screen, " basis=", target->basis, " this.coor=", this->coor());
                auto& console = *target;
                if (status.update(console))
                {
                    this->base::riseup<tier::preview>(e2::form::prop::footer, status.data);
                }
                //vsize = batch.size - ring::size + panel.y;
                //auto cursor_coor = console.get_coord();
                //cursor_coor.y += console.basis;
                //cursor_coor.y += std::max(0, console.height() - screen.size.y) - oversz.b;
                //cursor.coor(cursor_coor);
                auto view = parent_canvas.view();
                auto full = parent_canvas.full();
                auto origin = full.coor - view.coor;
                cursor.coor(console.get_coord(origin));
                //auto adjust_pads = console.recalc_pads(oversz);
                //auto scroll_size = recalc(cursor_coor);
                //auto scroll_size = screen.size;
                //auto follow_view = screen.coor.y == -base::coor().y;
                //scroll_size.y = std::max({ screen.size.y, cursor_coor.y + 1, console.height() });
                //scroll_size.y = std::max({ screen.size.y, cursor_coor.y + 1 - screen.coor.y, console.height() });
                //scroll_size.y = std::max({ screen.size.y, console.height() - oversz.b });
                //screen.coor.y = scroll_size.y - screen.size.y;

                //if (follow_view)
                //{
                //    log(" scroll();");
                //    scroll();
                //}
                //scroll();
                //if (!screen.hittest(cursor_coor)) // compat: get cursor back to the viewport if it placed outside
                //{
                //    cursor_coor = std::clamp(cursor_coor, screen.coor, screen.coor + screen.size - dot_11);
                //    target->set_coord(cursor_coor - screen.coor);
                //}
                //if (scroll_size != base::size() || adjust_pads)
                //{
                //    //log(" render - > resize");
                //    this->SIGNAL(tier::release, e2::size::set, scroll_size); // Update scrollbars.
                //}

                console.output(parent_canvas);
                if (oversz.b > 0) // Shade the viewport bottom oversize.
                {
                    auto bottom_oversize = parent_canvas.full();
                    bottom_oversize.coor.y += console.basis + console.panel.y;//scroll_size.y;
                    bottom_oversize.size.y  = oversz.b;
                    bottom_oversize = bottom_oversize.clip(parent_canvas.view());
                    parent_canvas.fill(bottom_oversize, cell::shaders::xlight);
                }

                // Debug: Shade active viewport.
                //auto vp = rect{ { 0,console.basis }, console.panel };
                //vp.coor += parent_canvas.full().coor;
                //vp = vp.clip(parent_canvas.view());
                //parent_canvas.fill(vp, [](auto& c){ c.fuse(cell{}.bgc(greenlt).bga(80)); });

                //log(" 2. screen=", screen, " basis=", target->basis, " this.coor=", this->coor());
            };
        }
    };
}

#endif // NETXS_TERMINAL_HPP