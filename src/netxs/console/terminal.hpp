// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_TERMINAL_HPP
#define NETXS_TERMINAL_HPP

#include "../ui/controls.hpp"
#include "../os/system.hpp"
#include "../abstract/ring.hpp"

#include <cassert> // ::assert(msg, 0)

namespace netxs::ui
{
    class rods // terminal: scrollback/altbuf internals v2 (New concept of the buffer)
        : public flow
    {
    protected:
        //todo implement as a ring buffer of size MAX_SCROLLBACK
        using heap = std::vector<para>;
        using iter = heap::iterator;
        heap        batch; // rods: Rods inner container.
        twod const& panel; // rods: Viewport.
        side&       upset; // rods: Viewport oversize.
        cell&       spare; // rods: Shared current brush state (default brush).
        //todo move count to ring
        iota        count; // rods: Scrollback height (& =batch.size()).
        twod        coord; // rods: Actual caret position.
        cell        brush; // rods: Last used brush.
        iota        basis; // rods: Index of O(0, 0).
        iota scroll_top    = 0; // rods: Scrolling region top. 1-based, 0: use top of viewport
        iota scroll_bottom = 0; // rods: Scrolling region bottom. 1-based, 0: use bottom of viewport
        iota current_para; // rods: Active insertion point index (not id).

    public:
        bool caret_visible = faux;

        rods(twod& anker, side& oversize, twod const& viewport, cell& spare)
            : flow { viewport.x, count /*use count from batch*/       },
              batch{ para(0)                  },
              panel{ viewport                 },
              upset{ oversize                 },
              spare{ spare                    },
              count{ 1                        },
              basis{ 0                        },
              current_para{ 0                 }
        { }
        auto line_height(para const& l)
        {
            if (wrapln)
            {
                auto len = l.length();
                if (len && (len % panel.x == 0)) len--;
                return len / panel.x + 1;
            }
            else return 1;
        }
        // rods: Return 0-based srcoll region (pair)
        auto get_scroll_limits()
        {
            auto top = scroll_top    ? scroll_top    - 1 : 0;
            auto end = scroll_bottom ? scroll_bottom - 1 : panel.y - 1;
            end = std::clamp(end, 0, panel.y - 1);
            top = std::clamp(top, 0, end);
            return std::pair{ top, end };
        }
        bool inside_scroll_region()
        {
            auto[top, end] = get_scroll_limits();
            return coord.y >= top && coord.y <= end;
        }
        //todo revise
        void color(cell const& c)
        {
            batch[current_para].brush = c;
            brush = c;
        }
        void align_basis()
        {
            auto new_basis = count - panel.y;
            if (new_basis > basis) basis = new_basis; // Move basis down if scrollback grows
        }
        //template<class L>
        //void add_lines(iota amount, L const& proto)
        void add_lines(iota amount, para const& proto)
        {
            assert(amount > 0);
            auto marker = proto.brush;
            auto adjust = proto.adjust;
            auto wrapln = proto.wrapln;
            auto nextid = batch.back().id();
            count += amount;
            while(amount-- > 0 ) batch.emplace_back(++nextid, 
                                                      marker,
                                                      adjust,
                                                      wrapln);
            align_basis();
        }
        void del_lines(iota erase_count)
        {
            assert(erase_count >= 0 && erase_count < count);
            count -= erase_count;
            while(erase_count--) batch.pop_back();
        }
        auto get_line_index_by_id(iota id)
        {
                return id - batch.front().id();
        }
        void set_coord(twod new_coord)
        {
            auto min_y = -basis; // Checking bottom boundary
            new_coord.y = std::max(new_coord.y, min_y);
            coord = new_coord;
            new_coord.y += basis; // set coord inside batch

            auto& cur_state = batch[current_para];
            brush = cur_state.brush;
            if (new_coord.y > count - 1) // Add new lines
            {
                auto add_count = new_coord.y - (count - 1);
                add_lines(add_count, cur_state);
            }
            auto& new_line = batch[new_coord.y];
            auto index = get_line_index_by_id(new_line.id()); // current index inside batch
            if (new_line.id() != new_line.bossid) // bossid always less or eq selfid
            {
                auto delta = new_line.id() - new_line.bossid;
                index -= delta;
                new_coord.x += delta * panel.x;
                //todo implement checking hit by x
            }
            current_para = index;
            auto& cur_line = batch[current_para];
            cur_line.chx(new_coord.x);
            cur_line.brush = brush;
            //todo implement the case when the coord is set to the outside viewport
            //     after the right side: disable wrapping (on overlapped line too)
            //     before the left side: disable wrapping + bias::right (on overlapped line too)
        }
        void set_coord() { set_coord(coord); }
        template<class P>
        void for_each(iota from, iota upto, P proc)
        {
            auto head = batch.begin() + from;
            auto tail = batch.begin() + upto;
            if (from < upto) while(proc(*head) && head++ != tail);
            else             while(proc(*head) && head-- != tail);
        }

        void finalize()
        {
            auto& cur_line1 = batch[current_para];
            auto old_height = line_height(cur_line1);
                              cur_line1.cook();
                              cur_line1.trim(spare);
            auto new_height = line_height(cur_line1);

            if (new_height > old_height)
            {
                auto overflow = current_para + new_height - count;
                if (overflow > 0)
                {
                    add_lines(overflow, cur_line1);
                }
                // Update bossid id on overlapped below lines
                auto from = current_para + new_height - 1;
                auto upto = current_para + old_height;
                auto& cur_line2 = batch[current_para];
                for_each(from, upto, [&](para& l)
                {
                    auto open = l.bossid - cur_line2.id() > 0;
                    if  (open)  l.bossid = cur_line2.id();
                    return open;
                });
            }
            else if (new_height < old_height)
            {
                // Update bossid id on opened below lines
                auto from = current_para + old_height - 1;
                auto upto = current_para + new_height;
                for_each(from, upto, [&](para& l)
                {
                    if (l.bossid != cur_line1.id()) return faux; // this line, like all those lying above, is covered with a long super line above, don't touch it
                    // Looking for who else covers the current line below
                    auto from2 = current_para + 1;
                    auto upto2 = from--;
                    for_each(from2, upto2, [&](para& l2)
                    {
                        auto h = line_height(l2);
                        auto d = upto2 - from2++;
                        if (h > d)
                        {
                            l.bossid = l2.id();
                            return faux;
                        }
                        return true;
                    });
                    return true;
                });
            }

            auto& cur_line3 = batch[current_para];
            auto pos = twod{ cur_line3.chx(), current_para - basis };
            if (cur_line3.wrapln)
            {
                if (pos.x && pos.x == cur_line3.length() && (pos.x % panel.x == 0))
                {
                    pos.x--;
                    coord.x = panel.x;
                }
                else
                {
                    coord.x = pos.x % panel.x;
                }
                coord.y = pos.x / panel.x + pos.y;
            }
            else
            {
                coord = pos;
            }

            //todo update flow::minmax and base::upset
        }
        void clear(bool preserve_brush = faux)
        {
            brush = preserve_brush ? batch[current_para].brush
                                   : cell{};
            basis = 0;
            count = 0;
            current_para = 0;
            upset.set(0);
            batch.clear();
            batch.emplace_back(0, brush);
            count++;
            align_basis();
        }
        auto cp()
        {
            auto pos = coord;
            pos.y += basis;
            if (pos.x == panel.x && batch[current_para].wrapln)
            {
                pos.x = 0;
                pos.y++;
            }
            return pos;
        }
        template<class P = noop>
        void visualize(P print_proc = P())
        {
            // Output lines in backward order from bottom to top
            auto tail = batch.rbegin();
            auto head = batch.rend();
            auto coor = twod{ 0, count };
            //todo optimize: print only visible (TIA canvas offsets)
            while(tail != head)
            {
                auto& line = *tail++;
                --coor.y;
                flow::cursor = coor;
                flow::wrapln = line.wrapln;
                flow::adjust = line.adjust;
                compose(line, print_proc);
                brush = line.mark(); // current mark of the last printed fragment
            }
        }
        void output(face& canvas)
        {
            flow::reset();
            flow::corner = canvas.corner();
            visualize([&](auto const& coord, auto const& subblock)
                      {
                          canvas.text(coord, subblock, deco::r_to_l);
                      });
        }
        auto reflow()
        {
            flow::reset();
            visualize();

            if (caret_visible) flow::minmax(cp()); // Register current caret position

            auto& cover = flow::minmax();
            upset.set(-std::min(0, cover.l),
                       std::max(0, cover.r - panel.x + 1),
                      -std::min(0, cover.t),
                       0);

            auto delta = cover.height() + 1 - count;
            if (delta > 0) // All visible lines must exist (including blank lines under wrapped lines)
            {
                add_lines(delta, batch.back());
            }
            
            auto viewport = rect{{ 0, basis }, panel};
            flow::minmax(viewport); // Register viewport (why?)

            //todo merge with main loop here
            //todo recalc overlapping bossid id's over selfid's on resize
            rebuild_upto_id(batch.front().id());

            auto scroll_height = cover.height() + 1;
            return twod{ panel.x, scroll_height };
        }
        auto height() const
        {
            return count;
        }
        void height(iota limits)
        {
            //todo resize ring buffer
        }
        void remove_empties()
        {
            auto head = batch.begin();
            auto tail = std::prev(batch.end()); // Exclude first line
            auto iter = tail;
            while(head != iter && iter->length() == 0)
            {
                iter--;
            }
            auto erase_count = static_cast<iota>(tail - iter);
            del_lines(erase_count);
            if (current_para >= count) current_para = count - 1;
        }
        // rods: Cut all lines above and current line
        void cut_above()
        {
            auto cur_line_it = batch.begin() + current_para;
            auto master_id = cur_line_it->bossid;
            auto mas_index = get_line_index_by_id(master_id);
            auto head = batch.begin() + mas_index;
            auto tail = cur_line_it;
            do
            {
                auto required_len = (current_para - mas_index) * panel.x + coord.x; // todo optimize
                auto& line = *head;
                line.trim_to(required_len);
                mas_index++;
            }
            while(head++ != tail);
        }
        // rods: Rebuild overlaps from bottom to line with selfid=top_id (inclusive)
        void rebuild_upto_id(iota top_id)
        {
            auto head = batch.rbegin();
            auto tail = head + count - get_line_index_by_id(top_id) - 1;
            do
            {
                auto& line =*head;
                auto below = head - line_height(line) + 1;
                do  // Assign iff line isn't overlapped by somaething higher
                {   // Comparing the difference with zero In order to support id incrementing overflow
                    if (below->bossid - top_id > 0) below->bossid = line.id();
                    else                            break; // overlapped by a higher line
                }
                while(head != below++);
            }
            while(head++ != tail);
        }
        // for bug testing
        auto get_content()
        {
            text yield;
            auto i = 0;
            for(auto& l : batch)
            {
                yield += "\n =>" + std::to_string(i++) + "<= ";
                yield += l.get_utf8();
            }
            return yield;
        }
    };

    class term // terminal: Built-in terminal app
        : public base
    {
        using self = term;

        #ifndef DEMO
        FEATURE(pro::keybd, keybd); // term: Keyboard controller.
        #endif
        FEATURE(pro::caret, caret); // term: Caret controller.
        FEATURE(pro::mouse, mouse); // term: Mouse controller.

        struct commands
        {
            struct erase
            {
                struct line
                {
                    enum : int
                    {
                        right = 0,
                        left  = 1,
                        all   = 2,
                    };
                };
                struct display
                {
                    enum : int
                    {
                        below      = 0,
                        above      = 1,
                        viewport   = 2,
                        scrollback = 3,
                    };
                };
            };
        };

        struct mtracking
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

            mtracking(term& owner)
                : owner{ owner }
            { }

            void enable (mode m)
            {
                state |= m;
                if (state && !token) // Do not subscribe if it is already subscribed
                {
                    owner.SUBMIT_T(e2::release, e2::hids::mouse::any, token, gear)
                    {
                        moved = coord((state & mode::over) ? gear.coord
                                                           : std::clamp(gear.coord, dot_00, owner.viewport.size - dot_11));
                        if (proto == sgr) serialize<sgr>(gear);
                        else              serialize<x11>(gear);
                        owner.write(queue);
                    };
                }
            }
            void disable(mode m) { state &= ~(m); if (!state) token.reset(); }
            void setmode(prot p) { proto = p; }

        private:
            term&       owner;
            testy<twod> coord;
            ansi::esc   queue; // mtracking: Buffer.
            hook        token; // mtracking: Subscription token.
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
                    case prot::x11: queue.mouse_x11(meta, coord); break;
                    case prot::sgr: queue.mouse_sgr(meta, coord, ispressed); break;
                    default:
                        break;
                }
            }
            // mtracking: Serialize mouse state.
            template<prot PROT>
            void serialize(hids& gear)
            {
                using m = e2::hids::mouse;
                using b = e2::hids::mouse::button;
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
                switch (owner.bell::protos<e2::release>())
                {
                    // Move
                    case b::drag::pull::leftright:
                    case b::drag::pull::left  : if (isdrag) proceed<PROT>(gear, idle + left, true); break;
                    case b::drag::pull::middle: if (isdrag) proceed<PROT>(gear, idle + mddl, true); break;
                    case b::drag::pull::right : if (isdrag) proceed<PROT>(gear, idle + rght, true); break;
                    case e2::hids::mouse::move: if (ismove) proceed<PROT>(gear, idle, faux); break;
                    // Press
                    case b::down::leftright: capture(gear); break;
                    case b::down::left     : capture(gear); proceed<PROT>(gear, left, true); break;
                    case b::down::middle   : capture(gear); proceed<PROT>(gear, mddl, true); break;
                    case b::down::right    : capture(gear); proceed<PROT>(gear, rght, true); break;
                    // Release
                    case b::up::leftright:   release(gear); break;
                    case b::up::left     :   release(gear); proceed<PROT>(gear, up_left); break;
                    case b::up::middle   :   release(gear); proceed<PROT>(gear, up_mddl); break;
                    case b::up::right    :   release(gear); proceed<PROT>(gear, up_rght); break;
                    // Wheel
                    case m::scroll::up  : proceed<PROT>(gear, wheel_up); break;
                    case m::scroll::down: proceed<PROT>(gear, wheel_dn); break;
                    default:
                        break;
                }
            }
        }
        mtracker;

        struct ftracking
        {
            ftracking(term& owner)
                : owner{ owner }
            { }

            operator bool () { return token.operator bool(); }
            void set(bool enable)
            {
                if (enable)
                {
                    if (!token) // Do not subscribe if it is already subscribed)
                    {
                        owner.SUBMIT_T(e2::release, e2::form::notify::keybd::any, token, gear)
                        {
                            switch(owner.bell::protos<e2::release>())
                            {
                                case e2::form::notify::keybd::got:  queue.fcs(true); break;
                                case e2::form::notify::keybd::lost: queue.fcs(faux); break;
                                default:
                                    break;
                            }
                            owner.write(queue);
                        };
                    }
                }
                else token.reset();
            }
        private:
            term&       owner;
            hook        token; // ftracking: Subscription token.
            ansi::esc   queue; // ftracking: Buffer.
            bool        state = faux;
        }
        ftracker;

        struct wall // term: VT-behavior for the rods
            : public rods
        {
            template<class T>
            struct parser : public ansi::parser<T>
            {
                using vt = ansi::parser<T>;
                parser() : vt()
                {
                    using namespace netxs::console::ansi;
                    vt::csier.table[CSI_CUU] = VT_PROC{ p->up ( q(1)); };  // CSI n A
                    vt::csier.table[CSI_CUD] = VT_PROC{ p->dn ( q(1)); };  // CSI n B
                    vt::csier.table[CSI_CUF] = VT_PROC{ p->cuf( q(1)); };  // CSI n C
                    vt::csier.table[CSI_CUB] = VT_PROC{ p->cuf(-q(1)); };  // CSI n D

                    vt::csier.table[CSI_CNL] = vt::csier.table[CSI_CUD];   // CSI n E
                    vt::csier.table[CSI_CPL] = vt::csier.table[CSI_CUU];   // CSI n F
                    vt::csier.table[CSI_CHX] = VT_PROC{ p->chx( q(1)); };  // CSI n G  Move caret hz absolute
                    vt::csier.table[CSI_CHY] = VT_PROC{ p->chy( q(1)); };  // CSI n d  Move caret vt absolute
                    vt::csier.table[CSI_CUP] = VT_PROC{ p->cup( q   ); };  // CSI y ; x H (1-based)
                    vt::csier.table[CSI_HVP] = VT_PROC{ p->cup( q   ); };  // CSI y ; x f (1-based)

                    vt::csier.table[CSI_DCH] = VT_PROC{ p->dch( q(1)); };  // CSI n P  Delete n chars
                    vt::csier.table[CSI_ECH] = VT_PROC{ p->ech( q(1)); };  // CSI n X  Erase n chars
                    vt::csier.table[CSI_ICH] = VT_PROC{ p->ich( q(1)); };  // CSI n @  Insert n chars

                    vt::csier.table[CSI__ED] = VT_PROC{ p->ed ( q(0)); };  // CSI n J
                    vt::csier.table[CSI__EL] = VT_PROC{ p->el ( q(0)); };  // CSI n K
                    vt::csier.table[CSI__IL] = VT_PROC{ p->il ( q(1)); };  // CSI n L  Insert n lines
                    vt::csier.table[CSI__DL] = VT_PROC{ p->dl ( q(1)); };  // CSI n M  Delete n lines
                    vt::csier.table[CSI__SD] = VT_PROC{ p->scl( q(1)); };  // CSI n T  Scroll down by n lines, scrolled out lines are lost
                    vt::csier.table[CSI__SU] = VT_PROC{ p->scl(-q(1)); };  // CSI n S  Scroll   up by n lines, scrolled out lines are lost

                    vt::csier.table[DECSTBM] = VT_PROC{ p->scr( q   ); };  // CSI r; b r  Set scrolling region (t/b: top+bottom)

                    vt::csier.table[CSI_WIN] = VT_PROC{ p->boss.winopt(q); };  // CSI n;m;k t  Terminal window options (XTWINOPT)

                    vt::intro[ctrl::ESC]['M']= VT_PROC{ p->ri(); }; // ESC M  Reverse index
                    vt::intro[ctrl::ESC]['H']= VT_PROC{ p->na("ESC H  Place tabstop at the current caret posistion"); }; // ESC H  Place tabstop at the current caret posistion
                    vt::intro[ctrl::ESC]['c']= VT_PROC{ p->boss.decstr(); }; // ESC c (same as CSI ! p) Full reset (RIS)

                    vt::intro[ctrl::BS ]     = VT_PROC{ p->cuf(-q.pop_all(ctrl::BS )); };
                    vt::intro[ctrl::DEL]     = VT_PROC{ p->del( q.pop_all(ctrl::DEL)); };
                    vt::intro[ctrl::TAB]     = VT_PROC{ p->tab( q.pop_all(ctrl::TAB)); };
                    vt::intro[ctrl::CR ]     = VT_PROC{ p->home(); };
                    vt::intro[ctrl::EOL]     = VT_PROC{ p->dn ( q.pop_all(ctrl::EOL)); };

                    vt::csier.table_quest[DECSET] = VT_PROC{ p->boss.decset(p, q); };
                    vt::csier.table_quest[DECRST] = VT_PROC{ p->boss.decrst(p, q); };
                    vt::csier.table_excl [DECSTR] = VT_PROC{ p->boss.decstr(); }; // CSI ! p  Soft terminal reset (DECSTR)

                    vt::csier.table[CSI_SGR][SGR_RST] = VT_PROC{ p->nil(); }; // fx_sgr_rst       ;
                    vt::csier.table[CSI_SGR][SGR_SAV] = VT_PROC{ p->sav(); }; // fx_sgr_sav       ;
                    vt::csier.table[CSI_SGR][SGR_FG ] = VT_PROC{ p->rfg(); }; // fx_sgr_fg_def    ;
                    vt::csier.table[CSI_SGR][SGR_BG ] = VT_PROC{ p->rbg(); }; // fx_sgr_bg_def    ;

                    vt::csier.table[CSI_CCC][CCC_JET] = VT_PROC{ p->jet(q(0)); }; // fx_ccc_jet  default=bias::left=0
                    vt::csier.table[CSI_CCC][CCC_WRP] = VT_PROC{ p->wrp(q(1)); }; // fx_ccc_wrp  default=true
                    vt::csier.table[CSI_CCC][CCC_RTL] = nullptr; // fx_ccc_rtl //todo implement
                    vt::csier.table[CSI_CCC][CCC_RLF] = nullptr; // fx_ccc_rlf

                    vt::oscer[OSC_LABEL_TITLE] = VT_PROC{ p->boss.prop(OSC_LABEL_TITLE, q); };
                    vt::oscer[OSC_LABEL]       = VT_PROC{ p->boss.prop(OSC_LABEL,       q); };
                    vt::oscer[OSC_TITLE]       = VT_PROC{ p->boss.prop(OSC_TITLE,       q); };
                    vt::oscer[OSC_XPROP]       = VT_PROC{ p->boss.prop(OSC_XPROP,       q); };

                    // Log all unimplemented CSI commands
                    for (auto i = 0; i < 0x100; ++i)
                    {
                        auto& proc = vt::csier.table[i];
                        if (!proc)
                        {
                           proc = [i](auto& q, auto& p) { p->not_implemented_CSI(i, q); };
                        }
                    }
                }
            };

            term& boss;

            wall(term& boss)
                : rods( boss.base::anchor,
                        boss.base::oversize,
                        boss.viewport.size,
                        boss.base::color()),
                  boss{ boss }
            { }

            // Implement base-CSI contract (see ansi::csi_t)
            void task(ansi::rule const& cmd)
            {
                finalize();
                auto& cur_line = batch[current_para];
                if (!cur_line.empty())
                {
                    add_lines(1, cur_line);
                    current_para = count - 1;
                } 
                batch[current_para].locus.push(cmd);
            }
            void post(utf::frag const& cluster) { batch[current_para].post(cluster); }
            void cook()                         { finalize(); }
            void  nil()                         { batch[current_para].nil(spare); }
            void  sav()                         { batch[current_para].sav(spare); }
            void  rfg()                         { batch[current_para].rfg(spare); }
            void  rbg()                         { batch[current_para].rbg(spare); }
            void  fgc(rgba const& c)            { batch[current_para].fgc(c); }
            void  bgc(rgba const& c)            { batch[current_para].bgc(c); }
            void  bld(bool b)                   { batch[current_para].bld(b); }
            void  itc(bool b)                   { batch[current_para].itc(b); }
            void  inv(bool b)                   { batch[current_para].inv(b); }
            void  stk(bool b)                   { batch[current_para].stk(b); }
            void  und(bool b)                   { batch[current_para].und(b); }
            void  dnl(bool b)                   { batch[current_para].dnl(b); }
            void  ovr(bool b)                   { batch[current_para].ovr(b); }
            void  wrp(bool b)                   { batch[current_para].wrp(b); }
            void  jet(iota n)                   { batch[current_para].jet(n); }
            void  rtl(bool b)                   { batch[current_para].rtl(b); }

            // Implement text manipulation procs
            template<class T>
            void na(T&& note)
            {
                log("not implemented: ", note);
            }
            void not_implemented_CSI(iota i, fifo& q)
            {
                text params;
                while(q)
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
            void tab(iota n) { batch[current_para].ins(n); }
            // wall: CSI n T/S  Scroll down/up, scrolled out lines are lost
            void scl(iota n)
            {
                finalize();
                auto[top, end] = get_scroll_limits();
                auto scroll_top_index = basis + top;
                auto scroll_end_index = basis + end;
                auto bossid = batch[scroll_top_index].bossid;
                auto count = end - top + 1;
                if (n > 0) // Scroll down
                {
                    n = std::min(n, count);
                    // Move down by n all below the current
                    // one by one from the bottom
                    auto dst = batch.begin() + scroll_end_index;
                    auto src = dst - n;
                    auto s = count - n;
                    while(s--)
                    {
                        (*dst--).move(std::move(*src--));
                    }
                    // Clear n first lines
                    auto head = batch.begin() + scroll_top_index;
                    auto tail = head + n;
                    while(head != tail)
                    {
                        (*head++).trim_to(0);
                    }
                }
                else if (n < 0) // Scroll up
                {
                    n = -n;
                    n = std::min(n, count);
                    // Move up by n=-n all below the current
                    // one by one from the top
                    auto dst = batch.begin() + scroll_top_index;
                    auto src = dst + n;
                    auto s = count - n;
                    while(s--)
                    {
                        (*dst++).move(std::move(*src++));
                    }
                    // Clear n last lines
                    auto head = batch.begin() + scroll_end_index;
                    auto tail = head - n;
                    while(head != tail)
                    {
                        (*head--).trim_to(0);
                    }
                }
                // Rebuild overlaps from bottom to id
                rods::rebuild_upto_id(bossid);
                set_coord();
            }
            // wall: CSI n L  Insert n lines. Place caret to the begining of the current.
            void il(iota n)
            {
               /* Works only if caret is in the scroll region.
                * Inserts n lines at the current row and removes n lines at the scroll bottom.
                */
                finalize();
                if (n > 0 && inside_scroll_region())
                {
                    auto old_top = scroll_top;
                    scroll_top = coord.y + 1;
                    scl(n);
                    scroll_top = old_top;
                    coord.x = 0;
                    set_coord();
                }
            }
            // wall: CSI n M Delete n lines. Place caret to the begining of the current.
            void dl(iota n)
            {
               /* Works only if caret is in the scroll region.
                * Deletes n lines at the current row and add n lines at the scroll bottom.
                */
                finalize();
                if (n > 0 && inside_scroll_region())
                {
                    auto old_top = scroll_top;
                    scroll_top = coord.y + 1;
                    scl(-n);
                    scroll_top = old_top;
                    coord.x = 0;
                    set_coord();
                }
            }
            // wall: ESC M  Reverse index
            void ri()
            {
                /*
                 * Reverse index
                 * - move caret one line up if it is outside of scrolling region or below the top line of scrolling region.
                 * - one line scroll down if caret is on the top line of scroll region.
                 */
                finalize();
                auto[top, end] = get_scroll_limits();
                if (coord.y != top)
                {
                    coord.y--;
                    set_coord();
                }
                else scl(1);
            }
            // wall: CSI t;b r - Set scrolling region (t/b: top+bottom)
            void scr(fifo& queue)
            {
                scroll_top    = queue(0);
                scroll_bottom = queue(0);
            }
            // wall: CSI n @  Insert n blanks after cursor. Don't change cursor pos
            void ich(iota n)
            {
                /*
                *   Inserts n blanks.
                *   Don't change cursor pos.
                *   Existing chars after cursor shifts to the right.
                */
                finalize();
                if (n > 0)
                {
                    auto size   = rods::batch[current_para].para::length();
                    auto pos    = rods::batch[current_para].chx();
                    auto brush  = rods::batch[current_para].brush;
                    brush.txt(whitespace);
                    //todo unify
                    if (pos < size)
                    {
                        // Move existing chars to right (backward decrement)
                        auto& lyric = *batch[current_para].lyric;
                        lyric.crop(size + n);
                        auto dst = lyric.data() + size + n;
                        auto end = lyric.data() + pos + n;
                        auto src = lyric.data() + size;
                        while (dst != end)
                        {
                            *--dst = *--src;
                        }
                        // Fill blanks
                        dst = lyric.data() + pos;
                        end = dst + n;
                        while (dst != end)
                        {
                            *dst++ = brush;
                        }
                    }
                    else
                    {
                        auto& lyric = *batch[current_para].lyric;
                        lyric.crop(pos + n);
                        // Fill blanks
                        auto dst = lyric.data() + size;
                        auto end = lyric.data() + pos + n;
                        while (dst != end)
                        {
                            *dst++ = brush;
                        }
                    }
                    batch[current_para].chx(pos);
                    finalize();
                }
            }
            // wall: CSI n X  Erase/put n chars after cursor. Don't change cursor pos
            void ech(iota n)
            {
                if (n > 0)
                {
                    finalize();
                    auto pos = batch[current_para].chx();
                    batch[current_para].ins(n);
                    finalize();
                    batch[current_para].chx(pos);
                }
            }
            // wall: CSI n P  Delete (not Erase) letters under the caret.
            void dch(iota n)
            {
                /* del:
                 *    As characters are deleted, the remaining characters
                 *    between the cursor and right margin move to the left.
                 *    Character attributes move with the characters.
                 *    The terminal adds blank characters at the right margin.
                 */
                finalize();
                auto& lyric =*rods::batch[current_para].lyric;
                auto size   = rods::batch[current_para].length();
                auto caret  = rods::batch[current_para].chx();
                auto brush  = rods::batch[current_para].brush;
                brush.txt(whitespace);

                //todo unify for size.y > 1
                if (n > 0)
                {
                    if (caret < size)
                    {
                        auto max_n = panel.x - caret % panel.x;
                        n = std::min(n, max_n);
                        auto right_margin = max_n + caret;

                        //todo unify all
                        if (n >= size - caret)
                        {
                            auto dst = lyric.data() + caret;
                            auto end = lyric.data() + size;
                            while (dst != end)
                            {
                                *dst++ = brush;
                            }
                        }
                        else
                        {
                            if (size < right_margin) lyric.crop(right_margin);

                            auto dst = lyric.data() + caret;
                            auto src = lyric.data() + caret + n;
                            auto end = dst + (max_n - n);
                            while (dst != end)
                            {
                                *dst++ = *src++;
                            }
                            end = lyric.data() + right_margin;
                            while (dst != end)
                            {
                                *dst++ = brush;
                            }
                        }
                    }
                }
                else
                {
                    //todo support negative n
                }
            }
            // wall: '\x7F'  Delete characters backwards.
            void del(iota n)
            {
                log("not implemented: '\\x7F' Delete characters backwards.");

                // auto& layer = rods::caret;
                // if (layer->chx() >= n)
                // {
                //     layer->chx(layer->chx() - n);
                //     layer->del(n);
                // }
                // else
                // {
                //     auto  here = layer->chx();
                //     auto there = n - here;
                //     layer->chx(0);
                //     if (here) layer->del(here);
                //     {
                //         if (layer != rods::batch.begin())
                //         {
                //             if (!layer->length())
                //             {
                //                 if (layer->locus.bare())
                //                 {
                //                     layer = std::prev(page::batch.erase(layer));
                //                 }
                //                 else
                //                 {
                //                     layer->locus.pop();
                //                 }
                //                 there--;
                //             }
                //             else
                //             {
                //                 auto prev = std::prev(layer);
                //                 *(*prev).lyric += *(*layer).lyric;
                //                 page::batch.erase(layer);
                //                 layer = prev;
                //                 layer->chx(layer->length());
                //             }
                //         }
                //     }
                // }
            }
            // wall: Move caret forward by n.
            void cuf(iota n)
            {
                finalize();
                auto posx = batch[current_para].chx();
                batch[current_para].chx(posx += n);
            }
            // wall: CSI n G  Absolute horizontal caret position (1-based)
            void chx(iota n)
            {
                finalize();
                coord.x = n - 1;
                set_coord();
            }
            // wall: CSI n d  Absolute vertical caret position (1-based)
            void chy(iota n)
            {
                finalize();
                coord.y = n - 1;
                set_coord();
            }
            // wall: CSI y; x H/F  Caret position (1-based)
            void cup(fifo& queue)
            {
                finalize();
                auto y = queue(1);
                auto x = queue(1);
                auto p = twod{ x, y };
                coord = std::clamp(p, dot_11, panel);
                coord-= dot_11;
                set_coord();
            }
            // wall: Line feed up
            template<bool PRESERVE_BRUSH = true>
            void up(iota n)
            {
                finalize();
                if (batch[current_para].wrapln)
                {
                    // Check the temp caret position (deffered wrap)
                    if (coord.x && (coord.x % panel.x == 0))
                    {
                        coord.x -= panel.x;
                        --n;
                    }
                }
                coord.y -= n;
                set_coord();
            }
            // wall: Line feed down
            void dn(iota n)
            {
                finalize();
                if (batch[current_para].wrapln
                 && coord.x == panel.x)
                {
                    coord.x = 0;
                }
                // Scroll regions up if coord.y == scroll_bottom and scroll region are defined
                auto[top, end] = get_scroll_limits();
                if (n > 0 && (scroll_top || scroll_bottom) && coord.y == end)
                {
                    scl(-n);
                }
                else
                {
                    coord.y += n;
                    set_coord();
                }
            }
            // wall: '\r'  Go to home of visible line instead of home of para
            void home()
            {
                finalize();
                auto posx = batch[current_para].chx();
                if (batch[current_para].wrapln) posx -= posx % panel.x;
                else                            posx = 0;
                batch[current_para].chx(posx);
                coord.x = 0;
            }
            // wall: '\n' || '\r\n'  Carriage return + Line feed
            void eol(iota n)
            {
                finalize();
                //todo Check the temp caret position (deffered wrap)
                coord.x = 0;
                coord.y += n;
                set_coord();
            }
            // wall: CSI n J  Erase display
            void ed(iota n)
            {
                finalize();
                auto current_brush = batch[current_para].brush;
                switch (n)
                {
                    case commands::erase::display::below: // n = 0(default)  Erase viewport after caret.
                        cut_above(); // Cut all lines above and current line
                        del_lines(count - (current_para + 1)); // Remove all lines below
                        break;
                    case commands::erase::display::above: // n = 1  Erase viewport before caret.
                    {
                        // Insert spaces on all lines above including the current line,
                        //   begining from bossid of viewport top line
                        //   ending the current line
                        auto master_id = batch[basis].bossid;
                        auto mas_index = get_line_index_by_id(master_id);
                        auto head = batch.begin() + mas_index;
                        auto tail = batch.begin() + current_para;
                        auto count = coord.y * panel.x + coord.x;
                        auto start = (basis - mas_index) * panel.x;
                        //todo unify
                        do
                        {
                            auto& lyric = *head;
                            lyric.ins(start, count, spare);
                            lyric.trim(spare);
                            start -= panel.x;
                        }
                        while(head++ != tail);
                        break;
                    }
                    case commands::erase::display::viewport: // n = 2  Erase viewport.
                        set_coord(dot_00);
                        ed(commands::erase::display::below);
                    break;
                    case commands::erase::display::scrollback: // n = 3  Erase scrollback.
                        rods::clear(true);
                    break;
                    default:
                        break;
                }
                batch[current_para].brush = current_brush;
                //todo preserve other attributes: wrp, jet
            }
            // wall: CSI n K  Erase line (don't move caret)
            void el(iota n)
            {
                finalize();
                auto& lyric = *batch[current_para].lyric;

                switch (n)
                {
                    default:
                    case commands::erase::line::right: // Ps = 0  ⇒  Erase to Right (default).
                    {
                        //todo optimize
                        auto brush = batch[current_para].brush;
                        brush.txt(' ');
                        auto coor = batch[current_para].chx();
                        //if (batch[current_para].wrapln)
                        {
                            auto right = panel.x - (coor + panel.x) % panel.x;
                            batch[current_para].ins(right);
                            batch[current_para].cook();
                            batch[current_para].trim(spare);
                            batch[current_para].chx(coor);
                        }
                        break;
                    }
                    case commands::erase::line::left: // Ps = 1  ⇒  Erase to Left.
                    {
                        auto brush = batch[current_para].brush;
                        brush.txt(' ');
                        auto coor = batch[current_para].chx();
                        auto width = boss.viewport.size.x;
                        if (coor < width)
                        {
                            lyric.each([&](cell& c) {if (coor > 0) { coor--; c.fuse(brush); } });
                        }
                        else
                        {
                            auto left_edge = coor - coor % width;
                            lyric.crop({ left_edge,1 }, brush);
                            lyric.crop({ left_edge + width,1 }, brush);
                        }
                        break;
                    }
                    case commands::erase::line::all: // Ps = 2  ⇒  Erase All.
                    {
                        //todo optimize
                        auto coor  = batch[current_para].chx();
                        auto brush = batch[current_para].brush;
                        brush.txt(' ');
                        auto width = boss.viewport.size.x;
                        auto left_edge = coor - coor % width;
                        lyric.crop({ left_edge,1 }, brush);
                        lyric.crop({ left_edge + width,1 }, brush);
                        break;
                    }
                }
            }
        };

        wall     scroll{ *this };
        wall     altbuf{ *this };
        wall*    target{ &scroll };
        os::cons ptycon;

        bool mode_DECCKM{ faux }; // todo unify
        std::map<text, text> props;
        std::map<text, std::vector<text>> props_stack;
        ansi::esc   output_queue;

        rect viewport = { dot_00, dot_11 }; // term: Viewport area
        bool bracketed_paste_mode = faux;

        // term: Write tty data and flush the queue.
        void write(text& queue)
        {
            if (queue.length())
            {
                ptycon.write(queue);
                queue.clear();
            }
        }
        // term: Set terminal window props.
        void prop(text const& cmd, view txt)
        {
            target->finalize();
            if (cmd == ansi::OSC_LABEL_TITLE)
            {
                              props[ansi::OSC_LABEL] = txt;
                auto& utf8 = (props[ansi::OSC_TITLE] = txt);
                base::riseup<e2::preview, e2::form::prop::header>(utf8);
            }
            else
            {
                auto& utf8 = (props[cmd] = txt);
                if (cmd == ansi::OSC_TITLE)
                    base::riseup<e2::preview, e2::form::prop::header>(utf8);
            }
        }
        // term: Manage terminal window props (XTWINOPS)
        void winopt(fifo& queue)
        {
            target->finalize();
            static constexpr iota get_label = 20; // Report icon   label. (Report as OSC L label ST)
            static constexpr iota get_title = 21; // Report window title. (Report as OSC l title ST)
            static constexpr iota put_stack = 22; // Push icon label and window title to   stack.
            static constexpr iota pop_stack = 23; // Pop  icon label and window title from stack.
            static constexpr iota all_title = 0; // Sub commands
            static constexpr iota label     = 1; // Sub commands
            static constexpr iota title     = 2; // Sub commands

            switch(auto option = queue(0))
            {
                // Return an empty string for security reasons
                case get_label: write(output_queue.osc(ansi::OSC_LABEL_REPORT, "")); break;
                case get_title: write(output_queue.osc(ansi::OSC_TITLE_REPORT, "")); break;
                case put_stack:
                {
                    auto push = [&](auto const& cmd){
                        auto& stack = props_stack[cmd];
                        stack.push_back(props[cmd]);
                    };
                    switch(queue(all_title))
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
                    auto pop = [&](auto const& cmd){
                        auto& stack = props_stack[cmd];
                        if (stack.size())
                        {
                            prop(cmd, stack.back());
                            stack.pop_back();
                        }
                    };
                    switch(queue(all_title))
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
        // term: Soft terminal reset (DECSTR)
        void decstr()
        {
            target->finalize();
            scroll.clear(true);
            altbuf.clear(true);
            target = &scroll;
        }
        void decset(wall*& p, fifo& queue)
        {
            target->finalize();
            while (auto q = queue(0))
            {
                switch (q)
                {
                    case 1:    // Cursor keys application mode.
                        mode_DECCKM = true;
                        break;
                    case 7:    // Enable auto-wrap
                        target->wrp(true);
                        break;
                    case 25:   // Caret on.
                        caret.show();
                        target->caret_visible = true; //todo unify
                        break;
                    case 9:    // Enable X10 mouse reporting protocol.
                        log("decset: CSI ? 9 h  X10 Mouse reporting protocol is not supported");
                        break;
                    case 1000: // Enable mouse buttons reporting mode.
                        mtracker.enable(mtracking::buttons_press);
                        break;
                    case 1001: // Use Hilite mouse tracking mode.
                        log("decset: CSI ? 1001 h  Hilite mouse tracking mode is not supported");
                        break;
                    case 1002: // Enable mouse buttons and drags reporting mode.
                        mtracker.enable(mtracking::buttons_drags);
                        break;
                    case 1003: // Enable all mouse events reporting mode.
                        mtracker.enable(mtracking::all_movements);
                        break;
                    case 1004: // Enable focus tracking
                        ftracker.set(true);
                        break;
                    case 1005: // Enable UTF-8 mouse reporting protocol.
                        log("decset: CSI ? 1005 h  UTF-8 mouse reporting protocol is not supported");
                        break;
                    case 1006: // Enable SGR mouse reporting protocol.
                        mtracker.setmode(mtracking::sgr);
                        break;
                    case 10060:// Enable mouse reporting outside the viewport (outside+negative coordinates).
                        mtracker.enable(mtracking::negative_args);
                        break;
                    case 1015: // Enable URXVT mouse reporting protocol.
                        log("decset: CSI ? 1015 h  URXVT mouse reporting protocol is not supported");
                        break;
                    case 1016: // Enable Pixels (subcell) mouse mode.
                        log("decset: CSI ? 1016 h  Pixels (subcell) mouse mode is not supported");
                        break;
                    case 1048: // Save cursor
                        break;
                    case 1047: // Use alternate screen buffer
                    case 1049: // Save cursor and Use alternate screen buffer, clearing it first.  This control combines the effects of the 1047 and 1048  modes.
                        target->finalize();
                        target = &altbuf;
                        break;
                    case 2004: // Set bracketed paste mode.
                        bracketed_paste_mode = true;
                        break;
                    default:
                        break;
                }
            }
        }
        void decrst(wall*& p, fifo& queue)
        {
            target->finalize();
            while (auto q = queue(0))
            {
                switch (q)
                {
                    case 1:    // Cursor keys ANSI mode.
                        mode_DECCKM = faux;
                        break;
                    case 7:    // Disable auto-wrap
                        target->wrp(faux);
                        break;
                    case 25:   // Caret off.
                        caret.hide();
                        target->caret_visible = faux; //todo unify
                        break;
                    case 9:    // Disable X10 mouse reporting protocol.
                        log("decset: CSI ? 9 l  X10 Mouse tracking protocol is not supported");
                        break;
                    case 1000: // Disable mouse buttons reporting mode.
                        mtracker.disable(mtracking::buttons_press);
                        break;
                    case 1001: // Don't use Hilite(c) mouse tracking mode.
                        log("decset: CSI ? 1001 l  Hilite mouse tracking mode is not supported");
                        break;
                    case 1002: // Disable mouse buttons and drags reporting mode.
                        mtracker.disable(mtracking::buttons_drags);
                        break;
                    case 1003: // Disable all mouse events reporting mode.
                        mtracker.disable(mtracking::all_movements);
                        break;
                    case 1004: // Disable focus tracking.
                        ftracker.set(faux);
                        break;
                    case 1005: // Disable UTF-8 mouse reporting protocol.
                        log("decset: CSI ? 1005 l  UTF-8 mouse reporting protocol is not supported");
                        break;
                    case 1006: // Disable SGR mouse reporting protocol (set X11 mode).
                        mtracker.setmode(mtracking::x11);
                        break;
                    case 10060:// Disable mouse reporting outside the viewport (allow reporting inside the viewport only).
                        mtracker.disable(mtracking::negative_args);
                        break;
                    case 1015: // Disable URXVT mouse reporting protocol.
                        log("decset: CSI ? 1015 l  URXVT mouse reporting protocol is not supported");
                        break;
                    case 1016: // Disable Pixels (subcell) mouse mode.
                        log("decset: CSI ? 1016 l  Pixels (subcell) mouse mode is not supported");
                        break;
                    case 1048: // Restore cursor
                        break;
                    case 1047: // Use normal screen nuffer
                    case 1049: // Use normal screen nuffer and restore cursor
                        target->finalize();
                        target = &scroll;
                        break;
                    case 2004: // Disable bracketed paste mode.
                        bracketed_paste_mode = faux;
                        break;
                    default:
                        break;
                }
            }
        }

    public:
        term(twod winsz, text cmdline)
            : mtracker{ *this },
              ftracker{ *this }
        {
            caret.show();

            SUBMIT(e2::release, e2::form::upon::attached, parent)
            {
                base::riseup<e2::request, e2::form::prop::header>(props[ansi::OSC_TITLE]);
            };

            #ifdef DEMO
            //pane::scrollable = faux;
            #endif

            //todo unify
            //t.scroll.maxlen(2000);

            #ifndef DEMO
            keybd.accept(true); // Subscribe on keybd offers
            #endif

            SUBMIT(e2::release, e2::hids::keybd::any, gear)
            {
                //todo reset scroll position on keypress
                auto caret_xy = caret.coor();
                if (!viewport.hittest(caret_xy))
                {
                    //todo revise
                    auto old_caret_pos = viewport.coor + viewport.size - dot_11;
                    auto anchor_delta = caret_xy - old_caret_pos;
                    auto new_coor = base::coor.get() - anchor_delta;
                    SIGNAL(e2::release, base::move_event, new_coor);
                }

                //todo optimize/unify
                auto data = gear.keystrokes;
                if (!bracketed_paste_mode)
                {
                    utf::change(data, "\033[200~", "");
                    utf::change(data, "\033[201~", "");
                }
                if (mode_DECCKM)
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

            SUBMIT(e2::preview, e2::form::layout::size, new_size)
            {
                //todo recalc overlapping bossid id's over selfid's on resize

                if (viewport.size != new_size)
                {
                    viewport.size = new_size;
                    if (target == &altbuf)
                    {
                        altbuf.ed(commands::erase::display::scrollback);
                    }
                    else
                    {
                        target->remove_empties();
                    }
                }

                auto new_pty_size = new_size;
                auto scrollback_size = target->reflow();
                new_size = std::max(new_size, scrollback_size); // Use the max size

                ptycon.resize(new_pty_size); // set viewport size
            };
            SUBMIT(e2::release, e2::form::layout::move, new_coor)
            {
                viewport.coor = -new_coor;
            };

            ptycon.start(cmdline, winsz, [&](auto d) { input_hndl(d); });
        }

        ~term()
        {
            ptycon.close();
        }

        void input_hndl(view shadow)
        {
            while (ptycon)
            {
                e2::try_sync guard;
                if (guard)
                {
                    //log(" 1. target content: ", target->get_content());

                    SIGNAL(e2::general, e2::debug::output, shadow);

                    auto old_caret_pos = caret.coor();
                    auto caret_is_visible = viewport.hittest(old_caret_pos);

                    ansi::parse(shadow, target); // Append using default insertion point

                    //if (target == &altbuf)
                    //{
                    //	altbuf.hz_trim(viewport.size.x);
                    //}

                    //todo remove rods::reflow(), take new_size only
                    //     all calcs are made already in rods::finalize()
                    auto new_size = target->reflow();
                    auto caret_xy = target->cp();

                    caret.coor(caret_xy);

                    if (caret_is_visible && !viewport.hittest(caret_xy))
                    {
                        auto anchor_delta = caret_xy - old_caret_pos;
                        auto new_coor = base::coor.get() - anchor_delta;
                        SIGNAL(e2::release, base::move_event, new_coor);
                    }

                    SIGNAL(e2::release, base::size_event, new_size);

                    base::deface();
                    //log(" 2. target content: ", target->get_content());

                    break;
                }
                else std::this_thread::yield();
            }
        }
        // term/base: Draw on canvas.
        virtual void renderproc (face& parent_canvas)
        {
            base::renderproc(parent_canvas);
            target->output(parent_canvas);

            // in order to show cursor/caret
            SIGNAL(e2::release, e2::form::upon::redrawn, parent_canvas);
        }
        virtual void color(rgba const& fg_color, rgba const& bg_color)
        {
            base::color(fg_color, bg_color);
            target->color(base::color());
        }
    };
}

#endif // NETXS_TERMINAL_HPP