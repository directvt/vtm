// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_TERMINAL_HPP
#define NETXS_TERMINAL_HPP

#include "../ui/controls.hpp"
#include "../os/system.hpp"
#include "../abstract/ring.hpp"

#include <cassert>

namespace netxs::ui
{
    // terminal: scrollback/altbuf internals.
    class rods
        : public flow
    {
        struct line
            : public para
        {
            enum type : iota
            {
                centered,
                leftside,
                rghtside,
                autowrap,
                count,
            };
            iota cur_size{ 0              };
            type cur_type{ type::leftside };

            line()                         = default;
            line(line&&)                   = default;
            line(line const&)              = default;
            line& operator = (line&&)      = default;
            line& operator = (line const&) = default;
            line(deco const& style)
                : para{ style }
            { }
            line(iota newid, deco const& style = {})
                : para{ newid, style }
            { }
            auto get_type()
            {
                return style.wrapln == wrap::on    ? type::autowrap :
                       style.adjust == bias::left  ? type::leftside :
                       style.adjust == bias::right ? type::rghtside :
                                                     type::centered ;
            }
            void wipe(cell const& brush)
            {
                para::wipe(brush);
                cur_size = 0;
                cur_type = type::leftside;
            }
            void resite(line& p)
            {
                cur_size = p.cur_size;
                cur_type = p.cur_type;
                p.cur_size = 0;
                p.cur_type = type::leftside;
                para::resite(p);
            }
        };

        struct rama
        {
            struct maxs : public std::vector<iota>
            {
                iota max = 0;
                maxs() : std::vector<iota>(1) { }
                void prev_max() { while(max > 0 && !at(--max)); }
            };
            using type = line::type;
            std::array<maxs, type::count> lens;

            void take(type new_type, iota new_size, type& cur_type, iota& cur_size)
            {
                if (cur_size != new_size || cur_type != new_type)
                {
                    auto& new_lens = lens[new_type];
                    if (new_lens.size() <= new_size) new_lens.resize(new_size * 2 + 1);

                    if (new_size < cur_size) drop(cur_type, cur_size);
                    else                   --lens[cur_type][cur_size];

                    ++new_lens[new_size];
                    if (new_lens.max < new_size) new_lens.max = new_size;

                    cur_size = new_size;
                    cur_type = new_type;
                }
            }
            void drop(type cur_type, iota cur_size)
            {
                auto& cur_lens =       lens[cur_type];
                auto cur_count = --cur_lens[cur_size];
                if (cur_size == cur_lens.max && cur_count == 0)
                {
                    cur_lens.prev_max();
                }
            }
            void take(line& l) { take(l.get_type(), l.length(), l.cur_type, l.cur_size); }
            void drop(line& l) { drop(l.cur_type, l.cur_size); }
            template<type T>
            auto max() { return lens[T].max; }
        };

        // For debug
        friend auto& operator<< (std::ostream& s, rods& c)
        {
            return s << "{ " << c.xsize.lens[0].max << ","
                             << c.xsize.lens[1].max << ","
                             << c.xsize.lens[2].max << ","
                             << c.xsize.lens[3].max << " }";
        }

    protected:
        using vect = std::vector<line>;
        using buff = netxs::generics::ring<vect, std::function<void(line&)>>;
        using mark = ansi::mark;
        using deco = ansi::deco;

        //todo unify
        static constexpr iota max_line_len = 65536;

        rama        xsize; // rods: Oversize manager.
        buff        batch; // rods: Rods inner container.
        vect        cache; // rods: Temporary container for scrolling regions.
        twod const& panel; // rods: Viewport size.
        twod        coord; // rods: Actual caret position inside viewport(panel). 0-based.
        iota        sctop; // rods: Scrolling region top;    1-based, "0" to use top of viewport.
        iota        scend; // rods: Scrolling region bottom; 1-based, "0" to use bottom of viewport.
        iota        basis; // rods: Index of O(0, 0).
        twod        saved; // rods: Saved caret position;

    public:
        mark        brush; // rods: Current brush for parser (default fg/bg-colors).
        deco        style; // rods: Parser style state.

        rods(twod const& viewport, iota buffer_size, iota grow_step)
            : flow { viewport.x, batch.size },
              batch{ buffer_size, grow_step, [&](auto& line){ xsize.drop(line); } },
              panel{ viewport               },
              basis{ 0                      },
              sctop{ 0                      },
              scend{ 0                      }
        {
            style.glb();
            batch.push(style); // At least one row must exist.
        }
        auto& height() const
        {
            return batch.size;
        }
        auto recalc_pads(side& oversz)
        {
            auto left = std::max(0, xsize.max<line::rghtside>() - panel.x);
            auto rght = std::max(0, xsize.max<line::leftside>() - panel.x);
            auto cntr = std::max(0, xsize.max<line::centered>() - panel.x);
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
        // rods: Map caret position from scrollback to viewport.
        auto get_coord()
        {
            auto& curln =*batch;
            auto vt_pos = batch.get() - basis;
            auto hz_pos = curln.chx();
            if (hz_pos >= panel.x && curln.style.wrapln == wrap::on)
            {
                auto x = hz_pos % panel.x;
                auto y = hz_pos / panel.x + vt_pos;
                auto right_most = x == 0
                               && hz_pos == curln.length()
                               && (coord.x != x || coord.y != y);
                if (right_most) coord = { panel.x, y - 1 };
                else            coord = { x      , y     };
            }
            else coord = { hz_pos, vt_pos };
        }
        // rods: Return current 0-based caret position in the scrollback.
        auto cp()
        {
            auto pos = coord;
            auto style = batch->style;
            if (pos.x == panel.x && style.wrapln == wrap::on)
            {
                if (pos.y != panel.y - 1) // The last position on the screen(c).
                {
                    pos.x = 0;
                    pos.y++;
                }
                else pos.x--;
            }
            pos.y += basis;
            if      (style.adjust == bias::right)  pos.x = panel.x - pos.x - 1;
            else if (style.adjust == bias::center) pos.x = (panel.x + pos.x) / 2;
            return pos;
        }
        auto line_height(para const& l)
        {
            auto length = l.length();
            return length > panel.x
                && l.style.wrapln == wrap::on ? (length + panel.x - 1) / panel.x
                                              : 1;
        }
        // rods: Return 0-based scroll region pair, inclusive.
        auto get_scroll_region()
        {
            auto top = sctop ? sctop - 1 : 0;
            auto end = scend ? scend - 1 : panel.y - 1;
            end = std::clamp(end, 0, panel.y - 1);
            top = std::clamp(top, 0, end);
            return std::pair{ top, end };
        }
        // rods: Set the scrolling region using 1-based top and bottom. Use 0 to reset.
        void set_scroll_region(iota top, iota bottom)
        {
            sctop = top;
            scend = bottom;
            if (scend && batch.length() < panel.y)
            {
                add_lines(panel.y - batch.length());
            }
            cache.resize(std::max(0, top - 1));
        }
        // rods: Return true if the scrolling region is set.
        auto scroll_region_used()
        {
            return scend || sctop;
        }
        void align_basis()
        {
            basis = std::max(0, batch.length() - panel.y);
        }
        void add_lines(iota amount)
        {
            assert(amount > 0);
            auto newid = batch.back().selfid;
            while(amount-- > 0 ) batch.push(++newid, style);
            align_basis();
        }
        void pop_lines(iota amount)
        {
            assert(amount >= 0 && amount < batch.length());
            while(amount--) batch.pop();
            align_basis();
        }
        auto get_line_index_by_id(iota id)
        {
            return id - batch.front().selfid;
        }
        // rods: Map caret position from viewport to scrollback (set insertion point).
        void set_coord(twod new_coord)
        {
            new_coord.y = std::max(new_coord.y, -basis);
            coord = new_coord;
            new_coord.y += basis; // place coord inside the batch
            auto add_count = new_coord.y + 1 - batch.length();
            if (add_count > 0) add_lines(add_count);
            new_coord.y = std::min(new_coord.y, batch.length() - 1); // The batch can remain the same size (cuz ring)
            auto& new_line = batch[new_coord.y];
            auto index = get_line_index_by_id(new_line.selfid); // current index inside batch
            if (new_line.selfid != new_line.bossid) // bossid always less or eq selfid
            {
                auto delta = new_line.selfid - new_line.bossid;
                index -= delta;
                new_coord.x += delta * panel.x;
                //todo implement checking hit by x
            }
            batch.set(index);
            batch->chx(new_coord.x);
            //todo implement the case when the coord is set to the viewport outside
            //     after the right side: disable wrapping (on overlapped line too)
            //     before the left side: disable wrapping + bias::right (on overlapped line too)
        }
        // rods: Map caret pos from viewport to scrollback.
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
            auto& cur_line1 = *batch;
            auto myid = cur_line1.selfid;
            cur_line1.style = style;
            auto old_height = line_height(cur_line1);
                              cur_line1.cook();
                              //todo revise
                              //cur_line1.trim(brush.spare, max_line_len);
            auto new_height = line_height(cur_line1);
            auto caret = batch.get();
            if (new_height > old_height)
            {
                //Scroll current region if needed (same as in scrollbuffer::dn())
                auto n = new_height - old_height;
                auto[top, end] = get_scroll_region();
                if (n > 0 && scroll_region_used() && coord.y <= end
                                                  && coord.y + n > end)
                {
                    n -= end - coord.y;
                    coord.y = end;
                    scroll_region(-n, true);
                }

                // Add empty lines if needed
                auto overflow = caret + new_height - batch.length();
                if (overflow > 0) add_lines(overflow);

                // Update bossid id on overlapped below lines
                auto from = caret + new_height - 1;
                auto upto = caret + old_height;
                for_each(from, upto, [&](para& l)
                {
                    auto open = l.bossid - myid > 0;
                    if  (open)  l.bossid = myid;
                    return open;
                });
            }
            else if (new_height < old_height)
            {
                // Update bossid id on opened below lines
                auto from = caret + old_height - 1;
                auto upto = caret + new_height;
                for_each(from, upto, [&](para& l)
                {
                    if (l.bossid != myid) return faux; // this line, like all those lying above, is covered with a long super line above, don't touch it
                    // Looking for who else covers the current line below
                    auto from2 = caret + 1;
                    auto upto2 = from--;
                    for_each(from2, upto2, [&](para& l2)
                    {
                        auto h = line_height(l2);
                        auto d = upto2 - from2++;
                        if (h > d)
                        {
                            l.bossid = l2.selfid;
                            return faux;
                        }
                        return true;
                    });
                    return true;
                });
            }

            get_coord();
            auto& cur_line3 = *batch;
            xsize.take(cur_line3);
        }
        void clear_all(bool preserve_brush = faux)
        {
            if (!preserve_brush) brush.reset();
            style.glb();
            saved = dot_00;
            basis = 0;
            batch.clear();
            batch.push(0);
            align_basis();
        }
        template<bool BOTTOM_ANCHORED = true>
        void resize(iota newsize, iota grow_by = 0)
        {
            batch.resize<BOTTOM_ANCHORED>(newsize, grow_by);
            set_scroll_region(0, 0);
            align_basis();
        }
        void output(face& canvas)
        {
            flow::reset(canvas);
            auto view = canvas.view();
            auto full = canvas.full();
            auto head = view.coor.y - full.coor.y;
            auto tail = head + panel.y;
            auto maxy = xsize.max<line::autowrap>() / panel.x;
            head = std::clamp(head - maxy, 0, batch.size);
            tail = std::clamp(tail,        0, batch.size);
            auto coor = twod{ 0, tail };
            auto line_it = batch.begin() + coor.y;
            auto left_edge_x = view.coor.x;
            auto half_size_x = full.size.x / 2;
            auto left_rect = rect{{ left_edge_x, coor.y + full.coor.y }, dot_11 };
            auto rght_rect = left_rect;
            rght_rect.coor.x+= view.size.x - 1;
            auto rght_edge_x = rght_rect.coor.x + 1;
            while(coor.y != head)
            {
                --coor.y;
                --rght_rect.coor.y;
                flow::ac(coor);
                auto& cur_line = *--line_it;
                flow::go(cur_line, canvas);
                if (auto length = cur_line.length()) // Mark lines not shown in full.
                {
                    rght_rect.size.y = line_height(cur_line);
                    if (rght_rect.size.y == 1)
                    {
                        auto left_dot = full.coor.x;
                        if      (cur_line.style.adjust == bias::center) left_dot += half_size_x - length / 2;
                        else if (cur_line.style.adjust == bias::right)  left_dot += full.size.x - length;

                        auto rght_dot = left_dot + length;
                        if (left_dot < left_edge_x)
                        {
                            left_rect.coor.y = rght_rect.coor.y;
                            left_rect.size.y = rght_rect.size.y;
                            canvas.fill(left_rect, [](auto& c){ c.txt('<').fgc(tint::greenlt); });
                        }
                        if (rght_dot > rght_edge_x)
                        {
                            canvas.fill(rght_rect, [](auto& c){ c.txt('>').fgc(tint::greenlt); });
                        }
                    }
                    else
                    {
                        auto left_dot = full.coor.x;
                        auto rght_dot = left_dot + view.size.x;
                        if (left_dot < left_edge_x)
                        {
                            left_rect.coor.y = rght_rect.coor.y;
                            left_rect.size.y = rght_rect.size.y;
                            if (cur_line.style.adjust == bias::center)
                            {
                                auto l = length % view.size.x;
                                auto scnd_left_dot = left_dot + half_size_x - l / 2;
                                if (scnd_left_dot >= left_edge_x) --left_rect.size.y;
                            }
                            else if (cur_line.style.adjust == bias::right)
                            {
                                auto l = length % view.size.x;
                                auto scnd_left_dot = rght_dot - l;
                                if (scnd_left_dot >= left_edge_x) --left_rect.size.y;
                            }
                            canvas.fill(left_rect, [](auto& c){ c.txt('<').fgc(tint::greenlt); });
                        }
                        if (rght_dot > rght_edge_x)
                        {
                            if (cur_line.style.adjust == bias::center)
                            {
                                auto l = length % view.size.x;
                                auto scnd_rght_dot = left_dot + half_size_x - l / 2 + l;
                                if (scnd_rght_dot <= rght_edge_x) --rght_rect.size.y;
                            }
                            else if (cur_line.style.adjust == bias::left)
                            {
                                auto l = length % view.size.x;
                                auto scnd_rght_dot = left_dot + l;
                                if (scnd_rght_dot <= rght_edge_x) --rght_rect.size.y;
                            }
                            canvas.fill(rght_rect, [](auto& c){ c.txt('>').fgc(tint::greenlt); });
                        }
                    }
                }
            }
        }
        void test_basis(face& canvas)
        {
            para p{ansi::bgc(redlt).add(" ").nil()};
            auto coor = twod{ 0, basis };
            flow::ac(coor);
            flow::go(p, canvas);
        }
        //void remove_empties()
        //{
        //    auto head = batch.begin();
        //    auto tail = batch.end();
        //    //while(head != --tail && tail->length() == 0) batch.pop();
        //    while(head != --tail
        //       && tail->length() == 0
        //       && tail->selfid == tail->bossid) batch.pop();
        //}
        // rods: Trim all lines above and current line.
        void trim_to_current()
        {
            auto cur_index = std::min(batch.length() - 1, basis + coord.y);
            auto line_iter = batch.begin() + cur_index;
            auto master_id = line_iter->bossid;
            auto mas_index = get_line_index_by_id(master_id);
            auto head = batch.begin() + mas_index;
            auto tail = line_iter;
            do
            {
                auto required_len = (cur_index - mas_index) * panel.x + coord.x; // todo optimize
                auto& line = *head;
                line.trimto(required_len);
                mas_index++;
            }
            while(head++ != tail);
        }
        // rods: Remove all lines below except the current.
        void del_below()
        {
            //auto cur_index = batch.get();
            //pop_lines(batch.length() - (cur_index + 1));

            // "ED2 Erase viewport" MUST keep empty lines.
            auto cur_index = std::min(batch.length() - 1, basis + coord.y); //batch.get();
            auto begin = batch.begin();
            auto end = batch.end();
            auto current = begin + cur_index;
            while(current != end)
            {
                auto& lyric = *current++;
                lyric.wipe(brush.spare);
                lyric.open();
            }
            set_coord();
        }
        void clear_above()
        {
            // Insert spaces on all lines above including the current line,
            //   begining from bossid of the viewport top line
            //   ending the current line
            auto top_index = get_line_index_by_id(batch[basis].bossid);
            auto cur_index = batch.get();
            auto begin = batch.begin();
            auto upper = begin + top_index;
            auto under = begin + cur_index;
            auto count = coord.y * panel.x + coord.x;
            auto start = (basis - top_index) * panel.x;
            do
            {
                auto& lyric = *upper;
                lyric.ins(start, count, brush.spare);
                lyric.trim(brush.spare);
                start -= panel.x;
            }
            while(upper++ != under);
        }
        // rods: Rebuild overlaps from bottom to line with selfid=top_id (inclusive).
        void rebuild_upto_id(iota top_id)
        {
            auto tail = batch.end();
            auto head = tail - (batch.length() - std::max(0, get_line_index_by_id(top_id)));
            do
            {
                auto& line =*--tail;
                line.trim(brush.spare, max_line_len);
                auto below = tail + (line_height(line) - 1);
                auto over = below - batch.end();
                if (over >= 0) add_lines(++over);
                do  // Assign iff line isn't overlapped by somaething higher
                {   // Comparing the difference with zero In order to support id incrementing overflow
                    below->bossid = line.selfid;
                }
                while(tail != below--);
            }
            while(tail != head);
        }
        // rods: Rebuild overlaps from bottom to the top visible line.
        void rebuild_viewport()
        {
            align_basis();
            auto maxy = xsize.max<line::autowrap>() / panel.x;
            auto head = std::max(0, basis - maxy);
            rebuild_upto_id(batch[head].selfid);
            get_coord();
        }
        // rods: For bug testing purposes.
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
        // rods: Dissect auto-wrapped lines at the specified iterator.
        void dissect(buff::iter line_iter)
        {
            auto& trim_line = *line_iter;
            if (trim_line.bossid != trim_line.selfid)
            {
                auto mas_index = get_line_index_by_id(trim_line.bossid);
                auto max_width = iota{ 0 };
                auto head_iter = batch.begin() + mas_index;
                //todo the case when wrap::on is over the wrap::off and the remainder is wider than panel.x
                do
                {
                    auto& line_inst = *--line_iter;
                    max_width += panel.x;
                    if (line_inst.style.wrapln == wrap::on)
                    {
                        auto remainder = line_inst.length() - max_width;
                        if (remainder > 0)
                        {
                            trim_line.insert(0, line_inst.substr(max_width, remainder));
                            line_inst.trimto(max_width);
                        }
                    }
                }
                while (line_iter != head_iter);
            }
        }
        // rods: Dissect auto-wrapped line at the current coord.
        void dissect()
        {
            auto current_it = batch.begin() + basis + coord.y;
            dissect(current_it);
            rebuild_upto_id(current_it->selfid);
            set_coord();
        }
        // rods: Move block to the specified destination. If begin_it > end_it decrement is used.
        template<class SRC, class DST>
        void move_to(SRC begin_it, SRC end_it, DST dest_it)
        {
            ptr::move_block(begin_it, end_it, dest_it, [](auto& s, auto& d) { d.resite(s); });
        }
        template<class SRC>
        void zeroise(SRC begin_it, SRC end_it)
        {
            while(begin_it != end_it)
            {
                auto& line = *begin_it;
                xsize.drop(line);
                line.wipe(brush.spare);
                ++begin_it;
            }
        }
        // rods: Shift by n the scroll region.
        void scroll_region(iota n, bool use_scrollback)
        {
            if (n)
            {
                auto[top, end] = get_scroll_region();
                auto nul_it = batch.begin();
                auto all_it = nul_it + basis;
                auto end_it = all_it + end;
                auto top_it = all_it + top;
                auto bossid = all_it->bossid;
                auto height = end - top + 1;
                auto footer = batch.end() - end_it - 1;
                if (footer < 0)
                {
                    add_lines(-footer);
                    footer = 0;
                }
                if (n > 0) // Scroll down (move down the text block).
                {
                    n = std::min(n, height);
                    auto a = top_it - 1;
                    auto b = end_it - n;
                    auto c = end_it + 1;
                    dissect(b + 1);
                    dissect(top_it);
                    if (footer) dissect(c);
                    zeroise(b + 1, c);
                    move_to(b, a, end_it);
                    zeroise(top_it, top_it + n);
                }
                else // Scroll up (move up the text block).
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
                            move_to(all_it, top_it,       buffer    ); // Move fixed header block to the temporary cache.
                            move_to(top_it, a,            all_it    ); // Move up by the "top" the first n lines of scrolling region.
                            move_to(buffer, buffer + top, all_it + n); // Move back fixed header block from the temporary cache.
                        }
                        add_lines(n);
                        auto c = batch.end() - 1;
                        auto b = c - n;
                        auto d = b - footer;
                        dissect(d + 1);
                        move_to(b, d, c); // Move down footer block by n.
                    }
                    else
                    {
                        auto b = end_it + 1;
                        dissect(a);
                        if (footer) dissect(b);
                        zeroise(top_it, a);
                        move_to(a, b, top_it);
                    }
                }
                rebuild_upto_id(bossid);
            }
        }
        // rods: Trim all autowrap lines by the specified size.
        void trim_to_size(twod const& new_size)
        {
            auto head = batch.begin() + basis;
            auto tail = batch.end() - std::max(0, panel.y - new_size.y);
            if (new_size.x < panel.x)
            {
                while (tail != head)
                {
                    --tail;
                    dissect(tail);
                    auto& line = *tail;
                    if (line.style.wrapln == wrap::on) line.trimto(new_size.x);
                }
            }
            else while (--tail != head) dissect(tail);
        }
    };

    // terminal: Built-in terminal app.
    class term
        : public form<term>
    {
        pro::caret caret{ *this }; // term: Caret controller.

        // term: VT-mouse tracking functionality.
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
                if (state && !token.count()) // Do not subscribe if it is already subscribed
                {
                    owner.SUBMIT_T(e2::release, e2::hids::mouse::scroll::any, token, gear)
                    {
                        gear.dismiss();
                    };
                    owner.SUBMIT_T(e2::release, e2::hids::mouse::any, token, gear)
                    {
                        auto c = gear.coord - (owner.base::size() - owner.screen.size);
                        moved = coord((state & mode::over) ? c
                                                           : std::clamp(c, dot_00, owner.screen.size - dot_11));
                        auto cause = owner.bell::protos<e2::release>();
                        if (proto == sgr) serialize<sgr>(gear, cause);
                        else              serialize<x11>(gear, cause);
                        owner.write(queue);
                    };
                    owner.SUBMIT_T(e2::general, e2::hids::mouse::gone, token, gear)
                    {
                        log("term: e2::hids::mouse::gone, id = ", gear.id);
                        auto cause = e2::hids::mouse::gone;
                        if (proto == sgr) serialize<sgr>(gear, cause);
                        else              serialize<x11>(gear, cause);
                        owner.write(queue);
                    };
                }
            }
            void disable(mode m) { state &= ~(m); if (!state) token.clear(); }
            void setmode(prot p) { proto = p; }

        private:
            term&       owner;
            testy<twod> coord;
            ansi::esc   queue; // mtracking: Buffer.
            subs        token; // mtracking: Subscription token.
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
            void serialize(hids& gear, id_t cause)
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
                switch (cause)
                {
                    // Move
                    case b::drag::pull::leftright:
                    case b::drag::pull::left  : if (isdrag) proceed<PROT>(gear, idle + left, true); break;
                    case b::drag::pull::middle: if (isdrag) proceed<PROT>(gear, idle + mddl, true); break;
                    case b::drag::pull::right : if (isdrag) proceed<PROT>(gear, idle + rght, true); break;
                    case e2::hids::mouse::move: if (ismove) proceed<PROT>(gear, idle + btup, faux); break;
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
                    case m::scroll::up  : proceed<PROT>(gear, wheel_up, true); break;
                    case m::scroll::down: proceed<PROT>(gear, wheel_dn, true); break;
                    // Gone
                    case m::gone:
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
        }
        mtracker; // term: VT-mouse tracking object.

        // term: Keyboard focus tracking functionality.
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
        ftracker; // term: Keyboard focus tracking object.

        // term: Terminal window options control.
        struct win_cntrl
        {
            term&                             owner;
            std::map<text, text>              props;
            std::map<text, std::vector<text>> stack;
            ansi::esc                         queue;

            win_cntrl(term& boss)
                : owner{ boss }
            { }
            // win_cntrl: Get terminal window property.
            auto& get(text const& property)
            {
                return props[property];
            }
            // win_cntrl: Set terminal window property.
            void set(text const& property, view txt)
            {
                static auto jet_left = ansi::jet(bias::left);
                owner.target->finalize();
                if (property == ansi::OSC_LABEL_TITLE)
                {
                                  props[ansi::OSC_LABEL] = txt;
                    auto& utf8 = (props[ansi::OSC_TITLE] = txt);
                    utf8 = jet_left + utf8;
                    owner.base::riseup<e2::preview, e2::form::prop::header>(utf8);
                }
                else
                {
                    auto& utf8 = (props[property] = txt);
                    if (property == ansi::OSC_TITLE)
                    {
                        utf8 = jet_left + utf8;
                        owner.base::riseup<e2::preview, e2::form::prop::header>(utf8);
                    }
                }
            }
            // win_cntrl: Manage terminal window props (XTWINOPS).
            void manage(fifo& q)
            {
                owner.target->finalize();
                static constexpr iota get_label = 20; // Report icon   label. (Report as OSC L label ST).
                static constexpr iota get_title = 21; // Report window title. (Report as OSC l title ST).
                static constexpr iota put_stack = 22; // Push icon label and window title to   stack.
                static constexpr iota pop_stack = 23; // Pop  icon label and window title from stack.
                static constexpr iota all_title = 0;  // Sub commands.
                static constexpr iota label     = 1;  // Sub commands.
                static constexpr iota title     = 2;  // Sub commands.
                switch(auto option = q(0))
                {
                    // Return an empty string for security reasons
                    case get_label: owner.write(queue.osc(ansi::OSC_LABEL_REPORT, "")); break;
                    case get_title: owner.write(queue.osc(ansi::OSC_TITLE_REPORT, "")); break;
                    case put_stack:
                    {
                        auto push = [&](auto const& property){
                            stack[property].push_back(props[property]);
                        };
                        switch(q(all_title))
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
                        switch(q(all_title))
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
        }
        winprops; // term: Terminal window options repository.

        // term: VT-behavior of the rods.
        struct scrollbuff
            : public rods
        {
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

            template<class T>
            struct parser : public ansi::parser<T>
            {
                using vt = ansi::parser<T>;
                parser() : vt()
                {
                    using namespace netxs::console::ansi;
                    vt::csier.table_space[CSI_SPC_SRC] = VT_PROC{ p->na("CSI n SP A  Shift right n columns(s)."); }; // CSI n SP A  Shift right n columns(s).
                    vt::csier.table_space[CSI_SPC_SLC] = VT_PROC{ p->na("CSI n SP @  Shift left  n columns(s)."); }; // CSI n SP @  Shift left n columns(s).
                    vt::csier.table_space[CSI_SPC_CST] = VT_PROC{ p->boss.caret_style(q(1)); }; // CSI n SP q  Set caret style (DECSCUSR).
                    vt::csier.table_hash [CSI_HSH_SCP] = VT_PROC{ p->na("CSI n # P  Push current palette colors onto stack. n default is 0."); }; // CSI n # P  Push current palette colors onto stack. n default is 0.
                    vt::csier.table_hash [CSI_HSH_RCP] = VT_PROC{ p->na("CSI n # Q  Pop  current palette colors onto stack. n default is 0."); }; // CSI n # Q  Pop  current palette colors onto stack. n default is 0.
                    vt::csier.table_excl [CSI_EXL_RST] = VT_PROC{ p->boss.decstr( ); }; // CSI ! p  Soft terminal reset (DECSTR)

                    vt::csier.table[CSI_CUU] = VT_PROC{ p->up ( q(1)); };  // CSI n A
                    vt::csier.table[CSI_CUD] = VT_PROC{ p->dn ( q(1)); };  // CSI n B
                    vt::csier.table[CSI_CUF] = VT_PROC{ p->cuf( q(1)); };  // CSI n C
                    vt::csier.table[CSI_CUB] = VT_PROC{ p->cuf(-q(1)); };  // CSI n D

                    vt::csier.table[CSI_CHT] = VT_PROC{ p->tab( q(1)); };  // CSI n I  Caret forward  n tabs, default n=1.
                    vt::csier.table[CSI_CBT] = VT_PROC{ p->tab(-q(1)); };  // CSI n Z  Caret backward n tabs, default n=1.
                    vt::csier.table[CSI_TBC] = VT_PROC{ p->tbc( q(1)); };  // CSI n g  Reset tabstop value.

                    vt::csier.table[CSI_CUD2]= VT_PROC{ p->dn ( q(1)); };  // CSI n e  Move caret down. Same as CUD.

                    vt::csier.table[CSI_CNL] = vt::csier.table[CSI_CUD];   // CSI n E
                    vt::csier.table[CSI_CPL] = vt::csier.table[CSI_CUU];   // CSI n F
                    vt::csier.table[CSI_CHX] = VT_PROC{ p->chx( q(1)); };  // CSI n G  Move caret hz absolute.
                    vt::csier.table[CSI_CHY] = VT_PROC{ p->chy( q(1)); };  // CSI n d  Move caret vt absolute.
                    vt::csier.table[CSI_CUP] = VT_PROC{ p->cup( q   ); };  // CSI y ; x H (1-based)
                    vt::csier.table[CSI_HVP] = VT_PROC{ p->cup( q   ); };  // CSI y ; x f (1-based)

                    vt::csier.table[CSI_DCH] = VT_PROC{ p->dch( q(1)); };  // CSI n P  Delete n chars.
                    vt::csier.table[CSI_ECH] = VT_PROC{ p->ech( q(1)); };  // CSI n X  Erase n chars.
                    vt::csier.table[CSI_ICH] = VT_PROC{ p->ich( q(1)); };  // CSI n @  Insert n chars.

                    vt::csier.table[CSI__ED] = VT_PROC{ p->ed ( q(0)); };  // CSI n J
                    vt::csier.table[CSI__EL] = VT_PROC{ p->el ( q(0)); };  // CSI n K
                    vt::csier.table[CSI__IL] = VT_PROC{ p->il ( q(1)); };  // CSI n L  Insert n lines.
                    vt::csier.table[CSI__DL] = VT_PROC{ p->dl ( q(1)); };  // CSI n M  Delete n lines.
                    vt::csier.table[CSI__SD] = VT_PROC{ p->scl( q(1)); };  // CSI n T  Scroll down by n lines, scrolled out lines are lost.
                    vt::csier.table[CSI__SU] = VT_PROC{ p->scl(-q(1)); };  // CSI n S  Scroll   up by n lines, scrolled out lines are pushed to the scrollback.
                    vt::csier.table[CSI_SCP] = VT_PROC{ p->scp(     ); };  // CSI   s  Save caret position.
                    vt::csier.table[CSI_RCP] = VT_PROC{ p->rcp(     ); };  // CSI   u  Restore caret position.

                    vt::csier.table[DECSTBM] = VT_PROC{ p->scr( q   ); };  // CSI r; b r  Set scrolling region (t/b: top+bottom).

                    vt::csier.table[CSI_WIN] = VT_PROC{ p->boss.winprops.manage(q); };  // CSI n;m;k t  Terminal window options (XTWINOPS).

                    vt::csier.table[CSI_CCC][CCC_RST] = VT_PROC{ p->style.glb();    };  // fx_ccc_rst
                    vt::csier.table[CSI_CCC][CCC_SBS] = VT_PROC{ p->boss.scrollbuffer_size(q); };  // CCC_SBS: Set scrollback size.
                    vt::csier.table[CSI_CCC][CCC_EXT] = VT_PROC{ p->boss.native(q(1)); };  // CCC_EXT: Setup extended functionality.
                    vt::csier.table[CSI_CCC][CCC_WRP] = VT_PROC{ p->wrp(q(0)); };  // CCC_WRP

                    vt::intro[ctrl::ESC][ESC_IND] = VT_PROC{ p->dn(1); }; // ESC D  Caret Down.
                    vt::intro[ctrl::ESC][ESC_IR ] = VT_PROC{ p->ri (); }; // ESC M  Reverse index.
                    vt::intro[ctrl::ESC][ESC_HTS] = VT_PROC{ p->stb(); }; // ESC H  Place tabstop at the current caret posistion.
                    vt::intro[ctrl::ESC][ESC_RIS] = VT_PROC{ p->boss.decstr(); }; // ESC c Reset to initial state (same as DECSTR).
                    vt::intro[ctrl::ESC][ESC_SC ] = VT_PROC{ p->scp(); }; // ESC 7 (same as CSI s) Save caret position.
                    vt::intro[ctrl::ESC][ESC_RC ] = VT_PROC{ p->rcp(); }; // ESC 8 (same as CSI u) Restore caret position.

                    vt::intro[ctrl::BS ] = VT_PROC{ p->cuf(-q.pop_all(ctrl::BS )); };
                    vt::intro[ctrl::DEL] = VT_PROC{ p->del( q.pop_all(ctrl::DEL)); };
                    vt::intro[ctrl::TAB] = VT_PROC{ p->tab( q.pop_all(ctrl::TAB)); };
                    vt::intro[ctrl::CR ] = VT_PROC{ p->home(); };
                    vt::intro[ctrl::EOL] = VT_PROC{ p->dn ( q.pop_all(ctrl::EOL)); };

                    vt::csier.table_quest[DECSET] = VT_PROC{ p->boss.decset(q); };
                    vt::csier.table_quest[DECRST] = VT_PROC{ p->boss.decrst(q); };

                    vt::oscer[OSC_LABEL_TITLE] = VT_PROC{ p->boss.winprops.set(OSC_LABEL_TITLE, q); };
                    vt::oscer[OSC_LABEL]       = VT_PROC{ p->boss.winprops.set(OSC_LABEL,       q); };
                    vt::oscer[OSC_TITLE]       = VT_PROC{ p->boss.winprops.set(OSC_TITLE,       q); };
                    vt::oscer[OSC_XPROP]       = VT_PROC{ p->boss.winprops.set(OSC_XPROP,       q); };

                    // Log all unimplemented CSI commands.
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
            //todo magic numbers
            static constexpr iota default_tabstop = 8;
            iota tabstop = default_tabstop; // scrollbuff: Tabstop current value.

            scrollbuff(term& boss, iota max_scrollback_size, iota grow_step = 0)
                : rods(boss.screen.size, max_scrollback_size, grow_step),
                  boss{ boss }
            { }

            // scrollbuff: Base-CSI contract (see ansi::csi_t).
            //             task(...), post(...), cook(...)
            void task(ansi::rule const& property)
            {
                finalize();
                auto& cur_line = *batch;
                if (cur_line.busy())
                {
                    add_lines(1);
                    batch.set(batch.length() - 1);
                } 
                batch->locus.push(property);
            }
            void post(utf::frag const& cluster) { batch->post(cluster, rods::brush); }
            void cook()                         { finalize(); }

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
            // scrollbuff: CCC_WRP:  Set autowrap mode.
            void wrp(iota w)
            {
                finalize();
                dissect();
                style.wrp(w);
            }
            // scrollbuff: ESC H  Place tabstop at the current caret posistion.
            void stb()
            {
                finalize();
                tabstop = std::max(1, coord.x + 1);
            }
            // scrollbuff: TAB  Horizontal tab.
            void tab(iota n)
            {
                finalize();
                if (n > 0)
                {
                    auto a = n * tabstop - coord.x % tabstop;
                    batch->ins(a, rods::brush);
                }
                else if (n < 0)
                {
                    n = -n - 1;
                    auto a = n * tabstop + coord.x % tabstop;
                    coord.x = std::max(0, coord.x - a);
                    set_coord();
                }
            }
            // scrollbuff: CSI n g  Reset tabstop value.
            void tbc(iota n)
            {
                tabstop = default_tabstop;
            }
            // scrollbuff: ESC 7 or CSU s  Save caret position.
            void scp()
            {
                finalize();
                saved = coord;
            }
            // scrollbuff: ESC 8 or CSU u  Restore caret position.
            void rcp()
            {
                finalize();
                coord = saved;
                set_coord();
            }
            // scrollbuff: CSI n T/S  Scroll down/up, scrolled up lines are pushed to the scrollback buffer.
            void scl(iota n)
            {
                finalize();
                scroll_region(n, n > 0 ? faux : true);
                set_coord();
            }
            // scrollbuff: CSI n L  Insert n lines. Place caret to the begining of the current.
            void il(iota n)
            {
               /* Works only if caret is in the scroll region.
                * Inserts n lines at the current row and removes n lines at the scroll bottom.
                */
                finalize();
                auto[top, end] = get_scroll_region();
                if (n > 0 && coord.y >= top && coord.y <= end)
                {
                    auto old_top = sctop;
                    sctop = coord.y + 1;
                    scroll_region(n, faux);
                    sctop = old_top;
                    coord.x = 0;
                    set_coord();
                }
            }
            // scrollbuff: CSI n M Delete n lines. Place caret to the begining of the current.
            void dl(iota n)
            {
               /* Works only if caret is in the scroll region.
                * Deletes n lines at the current row and add n lines at the scroll bottom.
                */
                finalize();
                auto[top, end] = get_scroll_region();
                if (n > 0 && coord.y >= top && coord.y <= end)
                {
                    auto old_top = sctop;
                    sctop = coord.y + 1;
                    scroll_region(-n, faux);
                    sctop = old_top;
                    coord.x = 0;
                    set_coord();
                }
            }
            // scrollbuff: ESC M  Reverse index.
            void ri()
            {
                /*
                 * Reverse index
                 * - move caret one line up if it is outside of scrolling region or below the top line of scrolling region.
                 * - one line scroll down if caret is on the top line of scroll region.
                 */
                finalize();
                auto[top, end] = get_scroll_region();
                if (coord.y != top)
                {
                    coord.y--;
                }
                else scroll_region(1, true);
                set_coord();
            }
            // scrollbuff: CSI t;b r - Set scrolling region (t/b: top+bottom).
            void scr(fifo& queue)
            {
                auto top = queue(0);
                auto end = queue(0);
                set_scroll_region(top, end);
            }
            // scrollbuff: CSI n @  Insert n blanks after cursor. Don't change cursor pos.
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
                    auto size  = batch->length();
                    auto pos   = batch->chx();
                    auto brush = rods::brush;
                    brush.txt(whitespace);
                    //todo unify
                    if (pos < size)
                    {
                        // Move existing chars to right (backward decrement).
                        auto& lyric = *(batch->lyric);
                        lyric.crop(size + n);
                        auto dst = lyric.data() + size + n;
                        auto end = lyric.data() + pos + n;
                        auto src = lyric.data() + size;
                        while (dst != end)
                        {
                            *--dst = *--src;
                        }
                        // Fill blanks.
                        dst = lyric.data() + pos;
                        end = dst + n;
                        while (dst != end)
                        {
                            *dst++ = brush;
                        }
                    }
                    else
                    {
                        auto& lyric = *(batch->lyric);
                        lyric.crop(pos + n);
                        // Fill blanks.
                        auto dst = lyric.data() + size;
                        auto end = lyric.data() + pos + n;
                        while (dst != end)
                        {
                            *dst++ = brush;
                        }
                    }
                    batch->chx(pos);
                    finalize();
                }
            }
            // scrollbuff: Shift left n columns(s).
            void shl(iota n)
            {
            }
            // scrollbuff: CSI n X  Erase/put n chars after cursor. Don't change cursor pos.
            void ech(iota n)
            {
                if (n > 0)
                {
                    finalize();
                    auto pos = batch->chx();
                    batch->ins(n, rods::brush);
                    finalize();
                    batch->chx(pos);
                }
            }
            // scrollbuff: CSI n P  Delete (not Erase) letters under the caret.
            void dch(iota n)
            {
                /* del:
                 *    As characters are deleted, the remaining characters
                 *    between the cursor and right margin move to the left.
                 *    Character attributes move with the characters.
                 *    The terminal adds blank characters at the right margin.
                 */
                finalize();
                auto& frag =*(batch->lyric);
                auto  size = batch->length();
                auto  coor = batch->chx();
                auto  mark = rods::brush;
                mark.txt(whitespace);
                //todo unify for size.y > 1
                if (n > 0)
                {
                    if (coor < size)
                    {
                        auto max_n = panel.x - coor % panel.x;
                        n = std::min(n, max_n);
                        auto right_margin = max_n + coor;
                        //todo unify all
                        if (n >= size - coor)
                        {
                            auto dst = frag.data() + coor;
                            auto end = frag.data() + size;
                            while (dst != end)
                            {
                                *dst++ = mark;
                            }
                        }
                        else
                        {
                            if (size < right_margin) frag.crop(right_margin);
                            auto dst = frag.data() + coor;
                            auto src = frag.data() + coor + n;
                            auto end = dst + (max_n - n);
                            while (dst != end)
                            {
                                *dst++ = *src++;
                            }
                            end = frag.data() + right_margin;
                            while (dst != end)
                            {
                                *dst++ = mark;
                            }
                        }
                    }
                }
                else
                {
                    //todo support negative n
                }
            }
            // scrollbuff: '\x7F'  Delete characters backwards.
            void del(iota n)
            {
                log("not implemented: '\\x7F' Delete characters backwards.");
            }
            // scrollbuff: Move caret forward by n.
            void cuf(iota n)
            {
                finalize();
                coord.x += n;
                set_coord();
            }
            // scrollbuff: CSI n G  Absolute horizontal caret position (1-based).
            void chx(iota n)
            {
                finalize();
                coord.x = n - 1;
                set_coord();
            }
            // scrollbuff: CSI n d  Absolute vertical caret position (1-based).
            void chy(iota n)
            {
                finalize();
                coord.y = n - 1;
                set_coord();
            }
            // scrollbuff: CSI y; x H/F  Caret position (1-based).
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
            // scrollbuff: Line reverse feed (move caret up).
            void up(iota n)
            {
                finalize();
                //log("up 1 batch id ", batch->id(), " coord ", coord);
                if (coord.x == panel.x && batch->style.wrapln == wrap::on)
                {
                    coord.x = 0;
                    --n;
                }
                coord.y -= n;
                //log("up 2 batch id ", batch->id(), " coord ", coord);
                set_coord();
                //log("up 3 batch id ", batch->id(), " coord ", coord);
            }
            // scrollbuff: Line feed (move caret down).
            void dn(iota n)
            {
                finalize();
                //log("dn 1 batch id ", batch->id(), " coord ", coord);
                if (coord.x == panel.x && batch->style.wrapln == wrap::on)
                {
                    coord.x = 0;
                }
                // Scroll regions up if coord.y == scend and scroll region are defined.
                auto[top, end] = get_scroll_region();
                if (n > 0 && scroll_region_used() && coord.y    <= end
                                                  && coord.y + n > end)
                {
                    n -= end - coord.y;
                    coord.y = end;
                    scroll_region(-n, true);
                }
                else
                {
                    //log("dn 2.0 coord ", coord);
                    coord.y += n;
                    //log("dn 2.1 coord ", coord);
                }
                //log("dn 2 batch id ", batch->id(), " coord ", coord);
                set_coord();
                //log("dn 3 batch id ", batch->id(), " coord ", coord);
            }
            // scrollbuff: '\r'  Go to home of visible line instead of home of para.
            void home()
            {
                //log("home 0 batch id ", batch->id(), " coord ", coord);
                finalize();
                //log("home 1 batch id ", batch->id(), " coord ", coord);
                coord.x = 0;
                set_coord();
                ///log("home 2 batch id ", batch->id(), " coord ", coord);
            }
            // scrollbuff: '\n' || '\r\n'  Carriage return + Line feed.
            void eol(iota n)
            {
                finalize();
                //todo Check the temp caret position (deffered wrap)
                //log("cr 1 batch id ", batch->id(), " chx ", batch->chx(), " coord ", coord);
                coord.x = 0;
                coord.y += n;
                //log("cr 2 batch id ", batch->id(), " chx ", batch->chx(), " coord ", coord);
                set_coord();
                //log("cr 3 batch id ", batch->id(), " chx ", batch->chx(), " coord ", coord);
            }
            // scrollbuff: CSI n J  Erase display.
            void ed(iota n)
            {
                finalize();
                switch (n)
                {
                    case commands::erase::display::below: // n = 0 (default)  Erase viewport after caret.
                        trim_to_current();
                        del_below();
                        break;
                    case commands::erase::display::above: // n = 1  Erase viewport before caret.
                        clear_above();
                        break;
                    case commands::erase::display::viewport: // n = 2  Erase viewport.
                        set_coord(dot_00);
                        ed(commands::erase::display::below);
                    break;
                    case commands::erase::display::scrollback: // n = 3  Erase scrollback.
                        clear_all(true);
                    break;
                    default:
                        break;
                }
            }
            // scrollbuff: CSI n K  Erase line (don't move caret).
            void el(iota n)
            {
                finalize();
                iota start;
                iota count;
                auto caret = std::max(0, batch->chx());
                auto wraps = batch->style.wrapln == wrap::on;
                switch (n)
                {
                    default:
                    case commands::erase::line::right: // n = 0 (default)  Erase to Right.
                        start = caret;
                        count = wraps ? panel.x - (caret + panel.x) % panel.x
                                      : std::max(0, std::max(panel.x, batch->length()) - caret);
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
                auto blank = cell{ brush }.txt(' ');
                //log("el 1 batch id ", batch->id(), " \\e[K from ", start, " count ", count);
                batch->ins<true>(start, count, blank);
                batch->trim(brush.spare);
                //log("el 2 batch id ", batch->id(), " chx ", batch->chx(), " coord ", coord);
            }

            struct info
            {
                iota size = 0;
                iota peak = 0;
                iota step = 0;
                twod area;
                ansi::esc data;
                auto update(scrollbuff const& scroll)
                {
                    if (scroll.update_status(*this))
                    {
                        data.clear();
                        data.jet(bias::right)
                            .add("size=", size,
                                " peak=", peak,
                                " type=");
                        if (step) data.add("unlimited, grow by ", step);
                        else      data.add("fixed");
                        data.add(" area=", area);
                        return true;
                    }
                    else return faux;
                }
            };
            bool update_status(info& status) const
            {
                bool changed = faux;
                if (status.size != batch.size) { changed = true; status.size = batch.size; }
                if (status.peak != batch.peak) { changed = true; status.peak = batch.peak; }
                if (status.step != batch.step) { changed = true; status.step = batch.step; }
                if (status.area != panel     ) { changed = true; status.area = panel;      }
                return changed;
            }
        }
        normal, // term: Normal screen buffer.
        altbuf, // term: Alternate screen buffer.
       *target; // term: Current screen buffer pointer.

        //struct viewport_cntrl
        //    : public rect
        //{
        //    viewport_cntrl()
        //        : rect{ dot_00, dot_11 }
        //    { }
        //}
        //viewport; // term: Viewport controller.
        rect screen = { dot_00, dot_11 }; // term: Viewport.
        os::cons ptycon; // term: PTY device.
        scrollbuff::info status; // term: Status info.
        hook oneshot_resize_token; // term: First resize subscription token.
        text cmdline;
        hook shut_down_token; // term: One shot shutdown token.
        bool alive = true;

        static constexpr iota default_size = 20000;
        static constexpr iota default_step = 0;

        bool mode_DECCKM = faux; // term: Cursor keys Application(true)/ANSI(faux) mode.
        bool bracketed_paste_mode = faux; // term: .

        // term: Soft terminal reset (DECSTR).
        void decstr()
        {
            target->finalize();
            normal.clear_all(true);
            altbuf.clear_all(true);
            target = &normal;
        }
        void decset(fifo& queue)
        {
            target->finalize();
            while (auto q = queue(0))
            {
                switch (q)
                {
                    case 1:    // Cursor keys application mode.
                        mode_DECCKM = true;
                        break;
                    case 7:    // Enable auto-wrap.
                        target->style.wrp(wrap::on);
                        break;
                    case 12:   // Enable caret blinking.
                        caret.blink_period();
                        break;
                    case 25:   // Caret on.
                        caret.show();
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
                    case 1004: // Enable focus tracking.
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
                    case 1048: // Save cursor.
                        break;
                    case 1047: // Use alternate screen buffer.
                    case 1049: // Save cursor and Use alternate screen buffer, clearing it first.  This control combines the effects of the 1047 and 1048  modes.
                        target->finalize();
                        target = &altbuf;
                        altbuf.clear_all(true);
                        break;
                    case 2004: // Set bracketed paste mode.
                        bracketed_paste_mode = true;
                        break;
                    default:
                        break;
                }
            }
        }
        void decrst(fifo& queue)
        {
            target->finalize();
            while (auto q = queue(0))
            {
                switch (q)
                {
                    case 1:    // Cursor keys ANSI mode.
                        mode_DECCKM = faux;
                        break;
                    case 7:    // Disable auto-wrap.
                        target->style.wrp(wrap::off);
                        break;
                    case 12:   // Disable caret blinking.
                        caret.blink_period(period::zero());
                        break;
                    case 25:   // Caret off.
                        caret.hide();
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
                    case 1048: // Restore cursor.
                        break;
                    case 1047: // Use normal screen buffer.
                    case 1049: // Use normal screen buffer and restore cursor.
                        target->finalize();
                        target = &normal;
                        break;
                    case 2004: // Disable bracketed paste mode.
                        bracketed_paste_mode = faux;
                        break;
                    default:
                        break;
                }
            }
        }
        void caret_style(iota style)
        {
            switch (style)
            {
            case 0: // n = 0  blinking box
            case 1: // n = 1  blinking box (default)
                caret.blink_period();
                caret.style(true);
                break;
            case 2: // n = 2  steady box
                caret.blink_period(period::zero());
                caret.style(true);
                break;
            case 3: // n = 3  blinking underline
                caret.blink_period();
                caret.style(faux);
                break;
            case 4: // n = 4  steady underline
                caret.blink_period(period::zero());
                caret.style(faux);
                break;
            case 5: // n = 5  blinking I-bar
                caret.blink_period();
                caret.style(true);
                break;
            case 6: // n = 6  steady I-bar
                caret.blink_period(period::zero());
                caret.style(true);
                break;
            default:
                log("term: unsupported caret style requested, ", style);
                break;
            }
        }

        // term: Set scrollback buffer size and grow_by step.
        void scrollbuffer_size(fifo& q)
        {
            iota max_scrollback_size = q(default_size);
            iota grow_step           = q(default_step);
            normal.resize<faux>(max_scrollback_size, grow_step);
        }
        // term: Extended functionality response.
        void native(bool b)
        {
            auto response = ansi::ext(b);
            write(response);
        }
        // term: Write tty data and flush the queue.
        void write(ansi::esc& queue)
        {
            if (queue.length())
            {
                ptycon.write(queue);
                queue.clear();
            }
        }
        auto recalc()
        {
            auto cursor_coor = target->cp();
            auto scroll_size = screen.size;
            auto follow_view = screen.coor.y == -base::coor().y;
            //todo unify, Allow overscroll at the bottom.
            //scroll_size.y = std::max({ screen.size.y + basis, cursor_coor.y + 1, target->height() });
            scroll_size.y = std::max({ screen.size.y, cursor_coor.y + 1, target->height() });
            screen.coor.y = scroll_size.y - screen.size.y;
            if (follow_view) reset_scroll_pos();
            if (!screen.hittest(cursor_coor)) // compat: get caret back to the viewport if it placed outside
            {
                cursor_coor = std::clamp(cursor_coor, screen.coor, screen.coor + screen.size - dot_11);
                target->set_coord(cursor_coor - screen.coor);
            }
            caret.coor(cursor_coor);
            return scroll_size;
        }
        void input_hndl(view shadow)
        {
            while (alive)
            {
                e2::try_sync guard;
                if (guard)
                {
                    SIGNAL(e2::general, e2::debug::output, shadow); // Post for the Logs.
                    ansi::parse(shadow, target); // Append target using current insertion point.
                    auto adjust_pads = target->recalc_pads(oversz);
                    auto scroll_size = recalc();
                    if (scroll_size != base::size() || adjust_pads)
                    {
                        SIGNAL(e2::release, e2::size::set, scroll_size); // Update scrollbars.
                    }
                    base::deface();
                    break;
                }
                else std::this_thread::yield();
            }
        }
        void shutdown_hndl(iota code)
        {
            log("term: exit code ", code);
            if (code)
            {
                text error = ansi::bgc(reddk).fgc(whitelt).add("\nterm: exit code ", code, " ");
                input_hndl(error);
            }
            else
            {
                log("term: submit for destruction on next frame/tick");
                SUBMIT_T(e2::general, e2::timer::tick, shut_down_token, t)
                {
                    shut_down_token.reset();
                    base::destroy();
                };
            }
        }
        void reset_scroll_pos()
        {
            //todo caret following
            this->SIGNAL(e2::release, e2::coor::set, -screen.coor);
        }
    public:
        ~term(){ alive = faux; }
        term(text cmd_line, iota max_scrollback_size = default_size, iota grow_step = default_step)
            : mtracker{ *this },
              ftracker{ *this },
              winprops{ *this },
              cmdline { cmd_line },
              normal  { *this, max_scrollback_size, grow_step },
              altbuf  { *this, 1                  , 0         },
              target  { &normal }
        {
            caret.show();

            #ifdef PROD
            form::keybd.accept(true); // Subscribe to keybd offers.
            #endif
            base::broadcast->SUBMIT_T(e2::preview, e2::data::text, bell::tracker, data)
            {
                reset_scroll_pos();
                input_hndl(data);
            };
            base::broadcast->SUBMIT_T(e2::release, e2::data::text, bell::tracker, data)
            {
                reset_scroll_pos();
                ptycon.write(data);
            };
            SUBMIT(e2::release, e2::form::upon::vtree::attached, parent)
            {
                this->base::riseup<e2::request, e2::form::prop::header>(winprops.get(ansi::OSC_TITLE));
                this->SUBMIT_T(e2::release, e2::size::set, oneshot_resize_token, new_sz)
                {
                    if (new_sz.y > 0)
                    {
                        oneshot_resize_token.reset();
                        altbuf.resize<faux>(new_sz.y);

                        this->SUBMIT(e2::preview, e2::size::set, new_sz)
                        {
                            new_sz = std::max(new_sz, dot_11);
                            if (target == &altbuf) altbuf.trim_to_size(new_sz);
                            screen.size = new_sz;
                            altbuf.resize<faux>(new_sz.y);
                            target->rebuild_viewport();
                            target->recalc_pads(oversz);
                            new_sz = recalc();
                            ptycon.resize(screen.size);
                        };
                        ptycon.start(cmdline, new_sz, [&](auto utf8_shadow) { input_hndl(utf8_shadow); },
                                                      [&](auto exit_code) { shutdown_hndl(exit_code); });
                    }
                };
            };
            SUBMIT(e2::release, e2::hids::keybd::any, gear)
            {
                reset_scroll_pos();
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
            SUBMIT(e2::release, e2::form::prop::brush, brush)
            {
                target->brush.reset(brush);
            };
            SUBMIT(e2::release, e2::render::any, parent_canvas)
            {
                if (status.update(*target))
                {
                    this->base::riseup<e2::preview, e2::form::prop::footer>(status.data);
                }
                target->output(parent_canvas);
                //target->test_basis(parent_canvas);
            };
        }
    };
}

#endif // NETXS_TERMINAL_HPP