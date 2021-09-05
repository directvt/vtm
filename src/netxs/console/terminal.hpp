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

    // terminal: Scrollback internals.
    class rods
    {
    public:    
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
        };
    private:
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
            iota height(iota width) const
            {
                auto len = length();
                return len > width
                    && style.wrp() == wrap::on ? (len + width - 1) / width
                                               : 1;
            }
            auto wrapped() { return _kind == type::autowrap; }
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

        using vect = std::vector<line>;
        using ring = generics::ring<vect, true>;
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

            //todo optimize for large lines, use std::unordered_map<iota, iota>
            struct maxs : public std::vector<iota>
            {
                iota max = 0;
                maxs() : std::vector<iota>(1) { }
                void prev_max() { while(max > 0 && !at(--max)); }
            }
            lens[type::count];

            void dec_height(type kind, iota size)
            {
                if (size > width && kind == type::autowrap) vsize -= (size + width - 1) / width;
                else                                        vsize -= 1;
                //log("dec h=", vsize, " kind=", kind, " linesize=", size);
            }
            void add_height(type kind, iota size)
            {
                if (size > width && kind == type::autowrap) vsize += (size + width - 1) / width;
                else                                        vsize += 1;
                //log("add h=", vsize, " kind=", kind, " linesize=", size);
            }
            // buff: Recalc the height for unlimited scrollback without using reflow.
            void set_width(iota new_width)
            {
                //log("set_width new_width=", new_width);
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
                    while(head != tail);
                }
                auto kind = type::autowrap;
                auto& cur_lens = lens[kind];
                auto head = cur_lens.begin();
                auto tail = head + cur_lens.max + 1;
                auto c = 0;
                auto h = 1;
                //auto i = 0;
                do
                {
                    if (auto count = *head++) vsize += h * count;
                    if (++c > width)
                    {
                        c = 1;
                        ++h;
                    }
                }
                while(head != tail);
                //log(" set_width done ", " h=", vsize);
            }

            void invite(type& kind, iota& size, type new_kind, iota new_size)
            {
                //log(" invite kind=", new_kind, " new_size=", new_size);
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
                if (size != new_size || kind != new_kind)
                {
                    auto& new_lens = lens[new_kind];
                    if (new_lens.size() <= new_size) new_lens.resize(new_size * 2 + 1);

                    if (new_size < size) undock(kind, size);
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
            void undock(type kind, iota size)
            {
                auto& cur_lens =       lens[kind];
                auto cur_count = --cur_lens[size];
                if (size == cur_lens.max && cur_count == 0)
                {
                    cur_lens.prev_max();
                }
                dec_height(kind, size);
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
                //return static_cast<iota>(id - batch.front().index); // ring buffer size is never larger than max_int32.
                auto count = length();
                return static_cast<iota>(count - 1 - (back().index - id)); // ring buffer size is never larger than max_int32.
            }
            void recalc_size(iota taken_index)
            {
                auto head = begin() + std::max(0, taken_index);
                auto tail = end();
                auto& curln = *head;
                auto accum = curln.accum;
                auto i = 0;
                log("  i=", i++, " curln.accum=", accum);
                accum += curln.length() + 1;
                while(++head != tail)
                {
                    auto& curln = *head;
                    curln.accum = accum;
                    log("  i=", i++, " curln.accum=", accum);
                    accum += curln.length() + 1;
                }
                dirty = faux;
                log( " recalc_size taken_index=", taken_index);
            }
            auto get_size_in_cells()
            {
                auto& endln = back();
                auto& endid = endln.index;
                auto count = length();
                auto taken_index = static_cast<iota>(count - 1 - (endid - taken));
                if (taken != endid || dirty)
                {
                    auto& topln = front();
                    recalc_size(taken_index);
                    taken = endln.index;
                    accum = endln.accum
                          + endln.length() + 1
                          - topln.accum;
                    log(" topln.accum=", topln.accum,
                        " endln.accum=", endln.accum,
                        " vsize=", vsize,
                        " accum=", accum);
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
                    taken = l.index;
                    taken_index = curln_index;
                }
                if (ring::size - taken_index > threshold)
                {
                    recalc_size(taken_index);
                    taken = back().index;
                }
            }
        };

        // For debug
        friend auto& operator<< (std::ostream& s, rods& c)
        {
            return s << "{ " << c.batch.max<line::type::leftside>() << ","
                             << c.batch.max<line::type::rghtside>() << ","
                             << c.batch.max<line::type::centered>() << ","
                             << c.batch.max<line::type::autowrap>() << " }";
        }

    //protected:
    public:
        buff        batch; // rods: Rods inner container.
        flow        maker; // rods: . deprecated
        vect        cache; // rods: Temporary line container.
        twod const& panel; // rods: Viewport size.
        twod        coord; // rods: Viewport cursor position; 0-based.
        iota        sctop; // rods: Scrolling region top;    1-based, "0" to use top of viewport.
        iota        scend; // rods: Scrolling region bottom; 1-based, "0" to use bottom of viewport.
        iota        basis; // rods: Index of O(0, 0).
        twod        saved; // rods: Saved cursor position;
        indx        index;

        iota vsize{ 1 }; // temp
        iota view_coor{};

        rods(twod const& viewport, iota buffer_size, iota grow_step)
            : batch{ buffer_size, grow_step },
              maker{ batch.width, batch.vsize },
              panel{ viewport               },
              basis{ 0                      },
              sctop{ 0                      },
              scend{ 0                      },
              index{ std::max(1,viewport.y) }
        {
            batch.invite(0); // At least one line must exist.
            batch.set_width(viewport.x);
            index_rebuild();
        }
        //rods: Return viewport vertical oversize.
        auto resize_viewport()
        {
            batch.set_width(panel.x);
            index.clear();
            index.resize(panel.y);
            view_coor = batch.vsize;
            auto lnid = batch.current().index;
            auto head = batch.end();
            auto size = batch.length();
            auto maxn = size - batch.index();
            auto tail = head - std::max(maxn, std::min(size, panel.y));
            auto push = [&](auto i, auto o, auto r) { --view_coor; index.push_front(i, o, r); };
            auto unknown = true;
            while(head != tail && (index.size != panel.y || unknown))
            {
                auto& curln = *--head;
                auto length = curln.length();
                auto active = curln.index == lnid;
                if (curln.wrapped())
                {
                    auto remain = length ? (length - 1) % panel.x + 1 : 0;
                    auto offset = length;
                    do
                    {
                        offset -= remain;
                        push(curln.index, offset, remain);
                        if (unknown && active && offset <= batch.caret)
                        {
                            auto eq = batch.caret && length == batch.caret;
                            unknown = faux;
                            coord.y = index.length();
                            coord.x = eq ? (batch.caret - 1) % panel.x + 1
                                         :  batch.caret      % panel.x;
                        }
                        remain = panel.x;
                    }
                    while(offset > 0 && (index.size != panel.y || unknown));
                }
                else
                {
                    push(curln.index, 0, length);
                    if (active)
                    {
                        unknown = faux;
                        coord.y = index.length();
                        coord.x = batch.caret;
                    }
                }
            }
            coord.y = index.size - coord.y;
            assert(view_coor >= 0);
            log(" viewport_offset=", view_coor);
            return std::max(0, batch.vsize - (view_coor + panel.y));
        }
        void index_rebuild()
        {
            index.clear();
            auto coor = batch.vsize;
            auto head = batch.end();
            while(coor != view_coor)
            {
                auto& curln = *--head;
                auto length = curln.length();
                if (curln.wrapped())
                {
                    auto remain = length ? (length - 1) % panel.x + 1 : 0;
                    length -= remain;
                    index.push_front(curln.index, length, remain);
                    --coor;
                    while(length > 0 && coor != view_coor)
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
        }
        auto height()
        {
            auto test_vsize = 0; //sanity check
            for (auto& l : batch) test_vsize += l.height(panel.x);
            if (test_vsize != batch.vsize) log(" test_vsize=", test_vsize, " vsize=", batch.vsize);
            return batch.vsize;
        }
        auto recalc_pads(side& oversz)
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
        // rods: Return current 0-based cursor position in the viewport.
        auto get_coord()
        {
            auto& curln = batch.current();
            auto  align = curln.style.jet();
            if (align == bias::left || align == bias::none) return coord;
            auto remain = index[coord.y].width;
            if (remain == panel.x && curln.wrapped()) return coord;
            auto coor = coord;
            if    (align == bias::right )  coor.x += panel.x     - remain - 1;
            else /*align == bias::center*/ coor.x += panel.x / 2 - remain / 2;
            return coor;
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
            //if (scend && batch.length() < panel.y)
            //{
            //    add_lines(panel.y - batch.length());
            //}
            cache.resize(std::max(0, top - 1));
        }
        // rods: Return true if the scrolling region is set.
        auto scroll_region_used()
        {
            return scend || sctop;
        }
        void add_lines(iota amount)
        {
            assert(amount > 0);
            auto newid = batch.back().index;
            auto style = batch->style;
            while(amount-- > 0)
            {
                auto& l = batch.invite(++newid, style);
                index.push_back(l.index, 0, 0);
            }
        }
        void pop_lines(iota amount)
        {
            assert(amount >= 0 && amount < batch.length());
            while(amount--) batch.pop_back();
            index_rebuild();
        }
        // rods: Map the current cursor position to the scrollback.
        void set_coord()
        {
            auto style = batch->style;

            if (coord.y <= 0) coord.y = 0;
            else
            {
                auto add_count = coord.y - (index.length() - 1);
                if (add_count > 0)
                {
                    add_lines(add_count);
                    auto maxy = panel.y - 1;
                    if (coord.y > maxy) coord.y = maxy;
                }
            }

            auto& mapln = index[coord.y];
            batch.index(mapln.index);
            batch.caret = mapln.start + coord.x;
            batch->style = style;
        }
        void sync_coord()
        {
            auto& curln = batch.current();
            //auto& style = curln.style;
            //auto  wraps = style.wrp();
            //if (wraps == wrap::on || wraps == wrap::none)
            if (curln.wrapped())
            {
                auto remain = index[coord.y].width;
                auto h = height();
                auto basis = std::max(0, h - panel.y);
                //...
            }
        }
        // rods: Set viewport cursor position.
        void set_coord(twod const& new_coord)
        {
            coord = new_coord;
            set_coord();
        }


/*
        struct item_t
        {
            iota height;
            iota offset;
        };
        struct viewport : public generics::ring<std::vector<item_t>, true>
        {
            using buff = rods::buff&;
            buff batch;
            flow& maker;
            iota vsize;
            twod panel;// panel.y = summ(lines)
            iota basis;//=vertical pos in buff

            iota vertical_offset{}; // vertical offset inside viewport (for double scroll position = { global_pos, viewport_pos })

            viewport(rods::buff& batch, twod panel, flow& maker)
                : ring { panel.y, 0 },
                  batch{ batch },
                  panel{ panel },
                  basis{ 0     },
                  maker{ maker },
                  vsize{ 0 }
            { }
            //auto count() { return ring::size; }
            void rebuild()
            {
                panel.y = 0;
                auto my_it = begin();
                auto proc = [&](auto&& l)
                {
                    auto& item = *my_it++;
                    item.offset = panel.y;
                    item.height = l.height(panel.x);
                    panel.y += item.height;
                };
                batch.for_each(basis, basis + ring::size, proc);
                vsize = batch.size - ring::size + panel.y;
            }
            template<bool BOTTOM_ANCHORED>
            void resize(twod const& new_size)
            {
                auto delta_y = new_size.y - ring::size;
                auto delta_x = new_size.x - panel.x;
                ring::resize<BOTTOM_ANCHORED>(new_size.y);
                if (delta_x)
                {
                    panel.x = new_size.x;
                }
                if (delta_y > 0)
                {
                    if constexpr (BOTTOM_ANCHORED)
                    {
                        basis -= delta_y;
                        //push_front(from basis, n = - delta.y)
                        //panel.y += summ(height);
                    }
                    else
                    {
                        //push_back(from basis + panel.y, n = delta.y)
                        //panel.y += summ(height);
                    }
                }
                rebuild();
            }
            void output(face& canvas)
            {
                maker.reset(canvas);
                auto view = canvas.view();
                auto full = canvas.full();
                auto head = view.coor.y - full.coor.y;
                auto tail = head + panel.y;
                auto maxy = batch.max<line::autowrap>() / panel.x;
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
                //todo unify/optimize
                auto fill = [&](auto& area, auto chr)
                {
                    if (auto r = view.clip(area))
                        canvas.fill(r, [&](auto& c){ c.txt(chr).fgc(tint::greenlt); });
                };
                auto data_it = begin();
                while(coor.y != tail)
                {
                    auto& curln = *line_it++;
                    auto& curdt = *data_it++;
                    coor.y += curdt.height;
                    rght_rect.coor.y += curdt.height;
                    maker.ac(coor);
                    maker.go(curln, canvas);
                }
            }
            void rebase(iota new_basis)
            {
                new_basis = std::max(0, new_basis);
                auto n = new_basis - basis;
                     if (n > 0) movedn( n);
                else if (n < 0) moveup(-n);
            }
            void moveup(iota n)
            {
                if (n > basis ) n = basis;
                basis -= n;
                //push_front(n);
            }
            void movedn(iota n)
            {
                basis += n;
                //push_back(n);
            }
            void enlist(item_t& line)          { panel.y += line.height; }
            void undock(item_t& line) override { panel.y -= line.height; }
        };
*/
        void el_imp(iota n, cell const& brush)
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
                //auto blank = cell{ brush }.txt(' ');
                auto blank = cell{ brush }.bgc(greendk).bga(0x7f).txt(' ');
                //todo check_autogrow
                curln.splice<true>(start, count, blank);
                //batch->shrink(blank);
            }
        }
        void ins_imp(iota n, cell const& brush)
        {
            auto& curln = batch.current();
            //todo check_autogrow
            curln.insert(batch.caret, n, brush);
        }
        template<bool AUTO_GROW = faux>
        void ech_imp(iota n, cell const& brush)
        {
            auto& curln = batch.current();
            if constexpr (AUTO_GROW)
            {
                //todo check_autogrow
                curln.splice<true>(batch.caret, n, brush);
            }
            else curln.splice(batch.caret, n, brush);
        }
        void dch_imp(iota n, cell const& brush)
        {
            auto& curln = batch.current();
            curln.cutoff(batch.caret, n, brush, panel.x);
            //auto caret = index[coord.y].start + coord.x;
            //curln.cutoff(caret, n, brush, panel.x);
        }
        void proceed(grid& proto, iota shift)
        {
            auto& curln = batch.current();
            coord.x += shift;
            if (coord.x > panel.x && curln.wrapped())
            {
                coord.y += coord.x / panel.x;
                coord.x %= panel.x;
                if (coord.x == 0)
                {
                    coord.y--;
                    coord.x = panel.x;
                }


                curln.splice(batch.caret, proto, shift);
                batch.recalc(curln);

                /*
                auto old_height = curln.height(panel.x) - 1;
                auto new_height = curln.height(panel.x) - 1;
                if (auto delta = new_height - old_height)
                {
                    auto old_pos = coord.y + old_height;
                    auto new_pos = coord.y + new_height;
                    coord.y = new_pos;
                    auto [top, end] = get_scroll_region();
                    if (old_pos <= end && new_pos > end)
                    {
                            auto n = end - new_pos;
                            scroll_region(n, true);
                            auto delta = coord.y - batch.length() - 1;
                            if (delta > 0) coord.y -= delta; // Coz ring buffer.
                    }
                    else
                    {
                        auto n = new_pos - (panel.y - 1);
                        if (n > 0)
                        {
                            //add_lines(n);
                            auto delta = coord.y - (panel.y - 1);
                            if (delta > 0) coord.y -= delta; // Coz ring buffer.
                        }
                    }
                }
                */
            }
            else
            {
                curln.splice(batch.caret, proto, shift);
                batch.recalc(curln);
            }
            index_rebuild();
            batch.caret += shift;

            log(" scrollbuff size in cells = ", batch.get_size_in_cells());
        }
        void clear_all()
        {
            saved = dot_00;
            coord = dot_00;
            batch.clear(); //todo optimize
            batch.invite(0); // At least one line must exist.
            batch.set_width(panel.x);
            view_coor = 0;
            index_rebuild();
        }
        template<bool BOTTOM_ANCHORED = true>
        void resize(iota new_size, iota grow_by = 0)
        {
            batch.resize<BOTTOM_ANCHORED>(new_size, grow_by);
            set_scroll_region(0, 0);
            index_rebuild();
        }
        void output(face& canvas) //todo temp solution, rough output, not optimized
        {
            maker.reset(canvas);
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
            while(head != tail)
            {
                auto& curln = *head++;
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
            }
        }
        //void test_basis(face& canvas)
        //{
        //    para p{ansi::bgc(redlt).add(" ").nil()};
        //    auto coor = twod{ 0, basis };
        //    maker.ac(coor);
        //    maker.go(p, canvas);
        //}
        // rods: Remove all lines below except the current. "ED2 Erase viewport" keeps empty lines.
        void del_below()
        {
            pop_lines(batch.length() - 1 - batch.index());
            add_lines(panel.y - 1 - coord.y);
            auto& curln = batch.current();
            curln.trimto(index[coord.y].start + coord.x);
            index_rebuild();
        }
        void clear_above(ansi::mark const& brush)
        {
            // Clear all lines from the viewport top line to the current line.
            dissect(0);
            dissect(coord.y);
            //move [coord.y, panel.y) to cache
        //    auto back_index = batch.back().index;
        //    auto block_size = back_index - index[coord.y].index + 1;
        //    auto all_size = back_index - index[0].index + 1;
        //    cache.reserve(block_size);
        //    auto start = batch.end() - block_size;
        //    netxs::move_block(start, batch.end(), cache.begin());
        //    //pop upto index[0].index inclusive
        //    pop_lines(all_size);
        //    auto top_it = batch.end() - 1;
        //    //push coord.y empty lines
        //    //push cache.size lines
        //    add_lines(coord.y + block_size);
        //    //move back from cache [coord.y, panel.y)
        //    netxs::move_block(cache.begin(), cache.end(), top_it + coord.y);
        //    index_rebuild();
            


            //auto& topln = batch[get_line_index_by_id(index[0].index)];
            //topln.trimto(index[0].start);

            //auto begin = batch.begin();
            //auto cur_index = batch.index();
            ////auto top_index = get_line_index_by_id(batch[basis].bossid);
            //auto under = begin + cur_index;
            //auto top_index = std::max(0, cur_index - under->depth);
            //auto upper = begin + top_index;
            //auto count = (coord.y - basis) * panel.x + coord.x;
            //auto start = (basis - top_index) * panel.x;
            //do
            //{
            //    auto& lyric = *upper;
            //    lyric.splice(start, count, brush.spare);
            //    lyric.shrink(brush.spare);
            //    start -= panel.x;
            //}
            //while(upper++ != under);
        }
        // rods: For bug testing purposes.
        auto get_content()
        {
            text yield;
            auto i = 0;
            for(auto& l : batch)
            {
                yield += "\n =>" + std::to_string(i++) + "<= ";
                l.each([&](cell& c) { yield += c.txt(); });
            }
            return yield;
        }
        // rods: Dissect auto-wrapped lines at the specified iterator.
        void dissect(iota y_pos)
        {
            if (y_pos >= index.size) throw;

            auto& linid = index[y_pos];
            if (linid.start == 0) return;

            auto temp = std::move(batch.current());
            batch.insert(temp.index, temp.style);
            auto curit = batch.current_it();
            auto head = curit;
            auto tail = batch.end();
            do ++head++->index;
            while(head != tail);

            auto& newln = *curit;
            newln.splice(0, temp.substr(linid.start));
            batch.recalc(newln);
            if (curit != batch.begin())
            {
                --curit;
                auto& curln = *curit;
                curln = std::move(temp);
                batch.undock(curln);
                curln.trimto(linid.start);
                batch.recalc(curln);
            }
            index_rebuild();
        }
        // rods: Dissect auto-wrapped line at the current coord.
        void dissect()
        {
            dissect(coord.y);
        }
        template<class SRC>
        void zeroise(SRC begin_it, SRC end_it)
        {
            while(begin_it != end_it)
            {
                auto& curln = *begin_it;
                batch.undock(curln);
                curln.wipe();
                ++begin_it;
            }
        }
        // rods: Shift by n the scroll region.
        void scroll_region(iota n, bool use_scrollback)
        {
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
        // rods: Trim all autowrap lines by the specified size.
        void trim_to_size(twod const& new_size)
        {
            /*
            auto head = batch.begin() + basis;
            auto tail = batch.end() - std::max(0, panel.y - new_size.y);
            if (new_size.x < panel.x)
            {
                while (tail != head)
                {
                    dissect(--tail);
                    auto& curln = *tail;
                    if (curln.style.wrp() == wrap::on)
                    {
                        curln.trimto(new_size.x);
                        batch.recalc(curln);
                    }
                }
            }
            else while (--tail != head) dissect(tail);
            */
        }
    };

    // terminal: Built-in terminal app.
    class term
        : public ui::form<term>
    {
        pro::caret cursor{ *this }; // term: Caret controller.

public:
        using events = netxs::events::userland::term;
        using commands = rods::commands;

private:
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
                    owner.SUBMIT_T(tier::release, hids::events::mouse::scroll::any, token, gear)
                    {
                        gear.dismiss();
                    };
                    owner.SUBMIT_T(tier::release, hids::events::mouse::any, token, gear)
                    {
                        auto c = gear.coord - (owner.base::size() - owner.screen.size);
                        moved = coord((state & mode::over) ? c
                                                           : std::clamp(c, dot_00, owner.screen.size - dot_11));
                        auto cause = owner.bell::protos<tier::release>();
                        if (proto == sgr) serialize<sgr>(gear, cause);
                        else              serialize<x11>(gear, cause);
                        owner.write(queue);
                    };
                    owner.SUBMIT_T(tier::general, hids::events::die, token, gear)
                    {
                        log("term: hids::events::die, id = ", gear.id);
                        auto cause = hids::events::die.id;
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
                        owner.SUBMIT_T(tier::release, hids::events::notify::keybd::any, token, gear)
                        {
                            switch(owner.bell::protos<tier::release>())
                            {
                                case hids::events::notify::keybd::got .id: queue.fcs(true); break;
                                case hids::events::notify::keybd::lost.id: queue.fcs(faux); break;
                                default: break;
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
            // win_cntrl: Manage terminal window props (XTWINOPS).
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
            : public rods,
              public ansi::parser
        {
            template<class T>
            static void parser_config(T& vt)
            {
                using namespace netxs::ansi;
                vt.csier.table_space[CSI_SPC_SRC] = VT_PROC{ p->na("CSI n SP A  Shift right n columns(s)."); }; // CSI n SP A  Shift right n columns(s).
                vt.csier.table_space[CSI_SPC_SLC] = VT_PROC{ p->na("CSI n SP @  Shift left  n columns(s)."); }; // CSI n SP @  Shift left n columns(s).
                vt.csier.table_space[CSI_SPC_CST] = VT_PROC{ p->boss.cursor_style(q(1)); }; // CSI n SP q  Set cursor style (DECSCUSR).
                vt.csier.table_hash [CSI_HSH_SCP] = VT_PROC{ p->na("CSI n # P  Push current palette colors onto stack. n default is 0."); }; // CSI n # P  Push current palette colors onto stack. n default is 0.
                vt.csier.table_hash [CSI_HSH_RCP] = VT_PROC{ p->na("CSI n # Q  Pop  current palette colors onto stack. n default is 0."); }; // CSI n # Q  Pop  current palette colors onto stack. n default is 0.
                vt.csier.table_excl [CSI_EXL_RST] = VT_PROC{ p->boss.decstr( ); }; // CSI ! p  Soft terminal reset (DECSTR)

                vt.csier.table[CSI_CUU] = VT_PROC{ p->up ( q(1)); };  // CSI n A
                vt.csier.table[CSI_CUD] = VT_PROC{ p->dn ( q(1)); };  // CSI n B
                vt.csier.table[CSI_CUF] = VT_PROC{ p->cuf( q(1)); };  // CSI n C
                vt.csier.table[CSI_CUB] = VT_PROC{ p->cuf(-q(1)); };  // CSI n D

                vt.csier.table[CSI_CHT] = VT_PROC{ p->tab( q(1)); };  // CSI n I  Caret forward  n tabs, default n=1.
                vt.csier.table[CSI_CBT] = VT_PROC{ p->tab(-q(1)); };  // CSI n Z  Caret backward n tabs, default n=1.
                vt.csier.table[CSI_TBC] = VT_PROC{ p->tbc( q(1)); };  // CSI n g  Reset tabstop value.

                vt.csier.table[CSI_CUD2]= VT_PROC{ p->dn ( q(1)); };  // CSI n e  Move cursor down. Same as CUD.

                vt.csier.table[CSI_CNL] = vt.csier.table[CSI_CUD];   // CSI n E
                vt.csier.table[CSI_CPL] = vt.csier.table[CSI_CUU];   // CSI n F
                vt.csier.table[CSI_CHX] = VT_PROC{ p->chx( q(1)); };  // CSI n G  Move cursor hz absolute.
                vt.csier.table[CSI_CHY] = VT_PROC{ p->chy( q(1)); };  // CSI n d  Move cursor vt absolute.
                vt.csier.table[CSI_CUP] = VT_PROC{ p->cup( q   ); };  // CSI y ; x H (1-based)
                vt.csier.table[CSI_HVP] = VT_PROC{ p->cup( q   ); };  // CSI y ; x f (1-based)

                vt.csier.table[CSI_DCH] = VT_PROC{ p->dch( q(1)); };  // CSI n P  Delete n chars.
                vt.csier.table[CSI_ECH] = VT_PROC{ p->ech( q(1)); };  // CSI n X  Erase n chars.
                vt.csier.table[CSI_ICH] = VT_PROC{ p->ich( q(1)); };  // CSI n @  Insert n chars.

                vt.csier.table[CSI__ED] = VT_PROC{ p->ed ( q(0)); };  // CSI n J
                vt.csier.table[CSI__EL] = VT_PROC{ p->el ( q(0)); };  // CSI n K
                vt.csier.table[CSI__IL] = VT_PROC{ p->il ( q(1)); };  // CSI n L  Insert n lines.
                vt.csier.table[CSI__DL] = VT_PROC{ p->dl ( q(1)); };  // CSI n M  Delete n lines.
                vt.csier.table[CSI__SD] = VT_PROC{ p->scl( q(1)); };  // CSI n T  Scroll down by n lines, scrolled out lines are lost.
                vt.csier.table[CSI__SU] = VT_PROC{ p->scl(-q(1)); };  // CSI n S  Scroll   up by n lines, scrolled out lines are pushed to the scrollback.
                vt.csier.table[CSI_SCP] = VT_PROC{ p->scp(     ); };  // CSI   s  Save cursor position.
                vt.csier.table[CSI_RCP] = VT_PROC{ p->rcp(     ); };  // CSI   u  Restore cursor position.

                vt.csier.table[DECSTBM] = VT_PROC{ p->scr( q   ); };  // CSI r; b r  Set scrolling region (t/b: top+bottom).

                vt.csier.table[CSI_WIN] = VT_PROC{ p->boss.winprops.manage(q); };  // CSI n;m;k t  Terminal window options (XTWINOPS).

                vt.csier.table[CSI_CCC][CCC_SBS] = VT_PROC{ p->boss.scrollbuffer_size(q); };  // CCC_SBS: Set scrollback size.
                vt.csier.table[CSI_CCC][CCC_EXT] = VT_PROC{ p->boss.native(q(1)); };  // CCC_EXT: Setup extended functionality.
                vt.csier.table[CSI_CCC][CCC_RST] = VT_PROC{ p->style.glb(); p->style.wrp(WRAPPING); };  // fx_ccc_rst

                vt.intro[ctrl::ESC][ESC_IND] = VT_PROC{ p->dn(1); }; // ESC D  Caret Down.
                vt.intro[ctrl::ESC][ESC_IR ] = VT_PROC{ p->ri (); }; // ESC M  Reverse index.
                vt.intro[ctrl::ESC][ESC_HTS] = VT_PROC{ p->stb(); }; // ESC H  Place tabstop at the current cursor posistion.
                vt.intro[ctrl::ESC][ESC_RIS] = VT_PROC{ p->boss.decstr(); }; // ESC c Reset to initial state (same as DECSTR).
                vt.intro[ctrl::ESC][ESC_SC ] = VT_PROC{ p->scp(); }; // ESC 7 (same as CSI s) Save cursor position.
                vt.intro[ctrl::ESC][ESC_RC ] = VT_PROC{ p->rcp(); }; // ESC 8 (same as CSI u) Restore cursor position.

                vt.intro[ctrl::BS ] = VT_PROC{ p->cuf(-q.pop_all(ctrl::BS )); };
                vt.intro[ctrl::DEL] = VT_PROC{ p->del( q.pop_all(ctrl::DEL)); };
                vt.intro[ctrl::TAB] = VT_PROC{ p->tab( q.pop_all(ctrl::TAB)); };
                vt.intro[ctrl::CR ] = VT_PROC{ p->home(); };
                vt.intro[ctrl::EOL] = VT_PROC{ p->dn ( q.pop_all(ctrl::EOL)); };

                vt.csier.table_quest[DECSET] = VT_PROC{ p->boss.decset(q); };
                vt.csier.table_quest[DECRST] = VT_PROC{ p->boss.decrst(q); };

                vt.oscer[OSC_LABEL_TITLE] = VT_PROC{ p->boss.winprops.set(OSC_LABEL_TITLE, q); };
                vt.oscer[OSC_LABEL]       = VT_PROC{ p->boss.winprops.set(OSC_LABEL,       q); };
                vt.oscer[OSC_TITLE]       = VT_PROC{ p->boss.winprops.set(OSC_TITLE,       q); };
                vt.oscer[OSC_XPROP]       = VT_PROC{ p->boss.winprops.set(OSC_XPROP,       q); };

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

            term& boss;
            //todo magic numbers
            static constexpr iota default_tabstop = 8;
            iota tabstop = default_tabstop; // scrollbuff: Tabstop current value.

            scrollbuff(term& boss, iota max_scrollback_size, iota grow_step = 0)
                : rods(boss.screen.size, max_scrollback_size, grow_step),
                  boss{ boss }
            {
                parser::style = ansi::def_style;
            }

            // scrollbuff: Base-CSI contract (see ansi::csi_t).
            //             task(...), meta(...), data(...)
            void task(ansi::rule const& property)
            {
                parser::flush();
                log("scrollbuff: locus extensions are not supported");
                //auto& cur_line = batch.current();
                //if (cur_line.busy())
                //{
                //    add_lines(1);
                //    batch.index(batch.length() - 1);
                //} 
                //batch->locus.push(property);
            }
            void meta(deco const& old_style) override
            {
                dissect();
                auto& curln = batch.current();
                if (curln.style != parser::style)
                {
                    curln.style = parser::style;
                    batch.recalc(curln);
                }
                if (parser::style.wrp() != old_style.wrp())
                {
                    auto status = parser::style.wrp() == wrap::none ? WRAPPING
                                                                    : parser::style.wrp();
                    boss.base::broadcast->SIGNAL(tier::release, app::term::events::layout::wrapln, status);
                }
                if (parser::style.jet() != old_style.jet())
                {
                    auto status = parser::style.jet() == bias::none ? bias::left
                                                                    : parser::style.jet();
                    boss.base::broadcast->SIGNAL(tier::release, app::term::events::layout::align, status);
                }
            }
            void data(grid& proto, iota width) override
            { 
                proceed(proto, width);
            }

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
            void clear_all()
            {
                rods::clear_all();
                parser::state = {};
            }
            // scrollbuff: ESC H  Place tabstop at the current cursor posistion.
            void stb()
            {
                parser::flush();
                tabstop = std::max(1, coord.x + 1);
            }
            // scrollbuff: TAB  Horizontal tab.
            void tab(iota n)
            {
                parser::flush();
                if (n > 0)
                {
                    auto a = n * tabstop - coord.x % tabstop;
                    ech_imp<true>(a, brush);
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
            // scrollbuff: ESC 7 or CSU s  Save cursor position.
            void scp()
            {
                parser::flush();
                saved = coord;
            }
            // scrollbuff: ESC 8 or CSU u  Restore cursor position.
            void rcp()
            {
                parser::flush();
                coord = saved;
                set_coord();
            }
            // scrollbuff: CSI n T/S  Scroll down/up, scrolled up lines are pushed to the scrollback buffer.
            void scl(iota n)
            {
                parser::flush();
                scroll_region(n, n > 0 ? faux : true);
                set_coord();
            }
            // scrollbuff: CSI n L  Insert n lines. Place cursor to the begining of the current.
            void il(iota n)
            {
               /* Works only if cursor is in the scroll region.
                * Inserts n lines at the current row and removes n lines at the scroll bottom.
                */
                parser::flush();
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
            // scrollbuff: CSI n M Delete n lines. Place cursor to the begining of the current.
            void dl(iota n)
            {
               /* Works only if cursor is in the scroll region.
                * Deletes n lines at the current row and add n lines at the scroll bottom.
                */
                parser::flush();
                auto[top, end] = get_scroll_region();
                if (n > 0 && coord.y >= top && coord.y <= end)
                {
                    dissect();
                    auto old_top = sctop;
                    sctop = coord.y + 1;
                    scroll_region(-n, faux);
                    sctop = old_top;
                    set_coord();
                }
            }
            // scrollbuff: ESC M  Reverse index.
            void ri()
            {
                /*
                 * Reverse index
                 * - move cursor one line up if it is outside of scrolling region or below the top line of scrolling region.
                 * - one line scroll down if cursor is on the top line of scroll region.
                 */
                parser::flush();
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
                parser::flush();
                ins_imp(n, brush);
            }
            // scrollbuff: Shift left n columns(s).
            void shl(iota n)
            {
            }
            // scrollbuff: CSI n X  Erase/put n chars after cursor. Don't change cursor pos.
            void ech(iota n)
            {
                parser::flush();
                ech_imp(n, brush);
            }
            // scrollbuff: CSI n P  Delete (not Erase) letters under the cursor.
            void dch(iota n)
            {
                parser::flush();
                dch_imp(n, brush);
            }
            // scrollbuff: '\x7F'  Delete characters backwards.
            void del(iota n)
            {
                log("not implemented: '\\x7F' Delete characters backwards.");
            }
            // scrollbuff: Move cursor forward by n.
            void cuf(iota n)
            {
                parser::flush();
                coord.x += n;
                set_coord();
            }
            // scrollbuff: CSI n G  Absolute horizontal cursor position (1-based).
            void chx(iota n)
            {
                parser::flush();
                coord.x = n - 1;
                set_coord();
            }
            // scrollbuff: CSI n d  Absolute vertical cursor position (1-based).
            void chy(iota n)
            {
                parser::flush();
                coord.y = n - 1;
                set_coord();
            }
            // scrollbuff: CSI y; x H/F  Caret position (1-based).
            void cup(fifo& queue)
            {
                parser::flush();
                auto y = queue(1);
                auto x = queue(1);
                auto p = twod{ x, y };
                coord = std::clamp(p, dot_11, panel);
                coord-= dot_11;
                set_coord();
            }
            // scrollbuff: Line reverse feed (move cursor up).
            void up(iota n)
            {
                parser::flush();
                coord.y -= n;
                set_coord();
            }
            // scrollbuff: Line feed (move cursor down).
            void dn(iota n)
            {
                parser::flush();
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
                    coord.y += n;
                }
                set_coord();
            }
            // scrollbuff: '\r'  Go to home of visible line instead of home of para.
            void home()
            {
                parser::flush();
                coord.x = 0;
                set_coord();
            }
            // scrollbuff: CSI n J  Erase display.
            void ed(iota n)
            {
                parser::flush();
                switch (n)
                {
                    case commands::erase::display::below: // n = 0 (default)  Erase viewport after cursor.
                        del_below();
                        break;
                    case commands::erase::display::above: // n = 1  Erase viewport before cursor.
                        clear_above(brush);
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
            // scrollbuff: CSI n K  Erase line (don't move cursor).
            void el(iota n)
            {
                parser::flush();
                el_imp(n, brush);
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
            target->flush();
            normal.clear_all();
            altbuf.clear_all();
            target = &normal;
        }
        void decset(fifo& queue)
        {
            target->flush();
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
                    case 1048: // Save cursor pos.
                        target->scp();
                        break;
                    case 1047: // Use alternate screen buffer.
                    case 1049: // Save cursor pos and use alternate screen buffer, clearing it first.  This control combines the effects of the 1047 and 1048  modes.
                        altbuf.style = target->style;
                        target = &altbuf;
                        altbuf.clear_all();
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
            target->flush();
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
                    case 1048: // Restore cursor pos.
                        target->rcp();
                        break;
                    case 1047: // Use normal screen buffer.
                    case 1049: // Use normal screen buffer and restore cursor.
                        normal.style = target->style;
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
        void cursor_style(iota style)
        {
            switch (style)
            {
            case 0: // n = 0  blinking box
            case 1: // n = 1  blinking box (default)
                cursor.blink_period();
                cursor.style(true);
                break;
            case 2: // n = 2  steady box
                cursor.blink_period(period::zero());
                cursor.style(true);
                break;
            case 3: // n = 3  blinking underline
                cursor.blink_period();
                cursor.style(faux);
                break;
            case 4: // n = 4  steady underline
                cursor.blink_period(period::zero());
                cursor.style(faux);
                break;
            case 5: // n = 5  blinking I-bar
                cursor.blink_period();
                cursor.style(true);
                break;
            case 6: // n = 6  steady I-bar
                cursor.blink_period(period::zero());
                cursor.style(true);
                break;
            default:
                log("term: unsupported cursor style requested, ", style);
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
        void input_hndl(view shadow)
        {
            while (alive)
            {
                netxs::events::try_sync guard;
                if (guard)
                {
                    SIGNAL(tier::general, e2::debug::output, shadow); // Post for the Logs.
                    ansi::parse(shadow, target);
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
                SUBMIT_T(tier::general, e2::tick, shut_down_token, t)
                {
                    shut_down_token.reset();
                    base::destroy();
                };
            }
        }
        void reset_scroll_pos()
        {
            //todo cursor following
            this->SIGNAL(tier::release, e2::coor::set, -screen.coor);
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
            cursor.show();
            cursor.style(true);

            #ifdef PROD
            form::keybd.accept(true); // Subscribe to keybd offers.
            #endif
            base::broadcast->SUBMIT_T(tier::preview, app::term::events::cmd, bell::tracker, cmd)
            {
                log("term: tier::preview, app::term::cmd, ", cmd);
                reset_scroll_pos();
                switch(cmd)
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
                        target->ed(scrollbuff::commands::erase::display::viewport);
                        break;
                    default:
                        break;
                }
                input_hndl("");
            };
            base::broadcast->SUBMIT_T(tier::preview, app::term::events::data::in, bell::tracker, data)
            {
                log("term: app::term::data::in, ", utf::debase(data));
                reset_scroll_pos();
                input_hndl(data);
            };
            base::broadcast->SUBMIT_T(tier::preview, app::term::events::data::out, bell::tracker, data)
            {
                log("term: app::term::data::out, ", utf::debase(data));
                reset_scroll_pos();
                ptycon.write(data);
            };
            SUBMIT(tier::release, e2::form::upon::vtree::attached, parent)
            {
                this->base::riseup<tier::request>(e2::form::prop::header, winprops.get(ansi::OSC_TITLE));


                this->SUBMIT_T(tier::release, e2::size::set, oneshot_resize_token, new_sz)
                {
                    if (new_sz.y > 0)
                    {
                        oneshot_resize_token.reset();

                        new_sz = std::max(new_sz, dot_11);
                        if (target == &altbuf) altbuf.trim_to_size(new_sz);
                        altbuf.resize<faux>(new_sz.y);

                        screen.size = new_sz;
                        oversz.b = target->resize_viewport();

                        this->SUBMIT(tier::preview, e2::size::set, new_sz)
                        {
                            new_sz = std::max(new_sz, dot_11);
                            if (target == &altbuf) altbuf.trim_to_size(new_sz);
                            altbuf.resize<faux>(new_sz.y);

                            screen.size = new_sz;
                            oversz.b = target->resize_viewport();
                            ptycon.resize(new_sz);
                        };

                        ptycon.start(cmdline, new_sz, [&](auto utf8_shadow) { input_hndl(utf8_shadow); },
                                                      [&](auto exit_code) { shutdown_hndl(exit_code); });
                    }
                };
            };
            SUBMIT(tier::release, hids::events::keybd::any, gear)
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
            SUBMIT(tier::release, e2::form::prop::brush, brush)
            {
                target->brush.reset(brush);
            };
            SUBMIT(tier::release, e2::render::any, parent_canvas)
            {
                auto& console = *target;
                if (status.update(console))
                {
                    this->base::riseup<tier::preview>(e2::form::prop::footer, status.data);
                }
                //vsize = batch.size - ring::size + panel.y;
                auto cursor_coor = console.get_coord();
                cursor_coor.y += std::max(0, console.height() - screen.size.y) - oversz.b;
                cursor.coor(cursor_coor);
                auto adjust_pads = console.recalc_pads(oversz);
                //auto scroll_size = recalc(cursor_coor);
                auto scroll_size = screen.size;
                auto follow_view = screen.coor.y == -base::coor().y;
                //scroll_size.y = std::max({ screen.size.y, cursor_coor.y + 1, console.height() });
                //scroll_size.y = std::max({ screen.size.y, cursor_coor.y + 1 - screen.coor.y, console.height() });
                scroll_size.y = std::max({ screen.size.y, console.height() - oversz.b });
                screen.coor.y = scroll_size.y - screen.size.y;
                //if (follow_view) reset_scroll_pos();
                //if (!screen.hittest(cursor_coor)) // compat: get cursor back to the viewport if it placed outside
                //{
                //    cursor_coor = std::clamp(cursor_coor, screen.coor, screen.coor + screen.size - dot_11);
                //    target->set_coord(cursor_coor - screen.coor);
                //}
                if (scroll_size != base::size() || adjust_pads)
                {
                    this->SIGNAL(tier::release, e2::size::set, scroll_size); // Update scrollbars.
                }
                console.output(parent_canvas);


                //rods::viewport v{target->batch, screen.size, target->maker};
                //v.rebuild();
                //v.output(parent_canvas);



                //target->test_basis(parent_canvas);
            };
        }
    };
}

#endif // NETXS_TERMINAL_HPP