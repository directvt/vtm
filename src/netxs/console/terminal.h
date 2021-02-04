// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_TERMINAL_H
#define NETXS_TERMINAL_H

#include "../ui/controls.h"
#include "../os/system.h"

namespace netxs::ui
{
    class rods // terminal: scrollback/altbuf internals v2 (New concept of the buffer)
        : public flow
    {
        //template<class T>
        //struct ring // rods: ring buffer
        //{
        //    using buff = std::vector<sptr<T>>;
        //    buff heap; // ring: Inner container.
        //    iota size; // ring: Limit of the ring buffer (-1: unlimited)
        //    iota head;
        //    iota tail;
        //    ring(iota size = -1)
        //        : size{ size }
        //    { }
        //};
        //
        //using heap = ring<para>;
        //heap batch;
    protected:
        using parx = sptr<para>;
        struct line
        {
            iota master; // rod: Top visible text line index.
            iota const selfid; // rod: Text line index.
            bias adjust;
            iota length;
            bool wrapln;
            parx stanza = std::make_shared<para>();

            line(iota selfid,
                 bias adjust = bias::left,
                 iota length = 0,
                 bool wrapln = WRAPPING)
                : master { selfid },
                  selfid { selfid },
                  adjust { adjust },
                  length { length },
                  wrapln { wrapln }
            { }
            auto line_height(iota width)
            {
                if (wrapln)
                {
                    auto len = length;
                    if (len && (len % width == 0)) len--;
                    return len / width + 1;
                }
                else return 1;
            }
            void cook()
            {
                auto& item = *stanza;
                item.cook();
                auto size = item.size();
                length = size.x;
                //todo revise: height is always =1
                //height = size.y;

                //todo update wrapln and adjust
                //adjust = item...;
                //wrapln = item...;
            }
        };
        //todo implement as a ring buffer of size MAX_SCROLLBACK
        using heap = std::vector<line>;
        iota        parid; // rods: Last used paragraph id.
        heap        batch; // rods: Rods inner container.
        iota const& width; // rods: Viewport width.
        iota const& viewport_height; // rods: Viewport height.
        iota        count; // rods: Scrollback height (& =batch.size()).
        side&       upset; // rods: Viewport oversize.
        //todo ?
        //twod&       anker; // rods: The position of the nearest visible paragraph.
        twod        coord; // rods: Actual caret position.
        cell        brush; // rods: Last used brush.
        parx        caret; // rods: Active insertion point.

        iota current_para; // rods: Active insertion point index (not id).
    public:
        iota        basis; // rods: O(0, 0) -> batch[index].
        rods(twod& anker, side& oversize, twod const& viewport_size)
            : flow { viewport_size.x, count   },
              parid{ 0                        },
              batch{ line(parid)              },
              width{ viewport_size.x          },
              count{ 1                        },
              //anker{ anker                    },
              upset{ oversize                 },
              caret{ batch.front().stanza     },
              basis{ 0                        },
              viewport_height{ viewport_size.y},
              current_para{ 0 }
        { }

        void set_coord(twod new_coord)
        {
            // Checking the bottom boundary
            //auto max_y = basis + viewport_height - 1;
            //auto max_y = viewport_height - 1;
            //new_coord.y = std::clamp(new_coord.y, min_y, max_y);
            auto min_y = -basis;
            new_coord.y = std::max(new_coord.y, min_y);

            coord = new_coord;
            new_coord.y += basis; // set coord inside batch

            if (new_coord.y > count - 1)
            {
                auto add_count = new_coord.y - (count - 1);
                auto brush = caret->brush;

                auto& item = batch[current_para];
                auto wrp = item.wrapln;
                auto jet = item.adjust;
                while(add_count-- > 0 ) //todo same as in finalize()
                {
                    auto& new_item = batch.emplace_back(++parid);
                    auto new_line = new_item.stanza;
                    new_line->brush = brush;
                    new_item.wrapln = wrp;
                    new_item.adjust = jet;

                    count++;
                    if (count - basis > viewport_height) basis = count - viewport_height;
                }
            }

            auto& line = batch[new_coord.y];
            auto line_id = line.master;
            auto line_len = line.length;

            auto index = line.selfid - batch.front().selfid; // current index
            brush = caret->brush;
            if (line.selfid == line.master) // no overlapped lines
            {
                caret = batch[index].stanza;
                caret->chx(new_coord.x);
                //todo implement the case when the coord is set to the outside viewport
                //     after the right side: disable wrapping (on overlapped line too)
                //     before the left side: disable wrapping + bias::right (on overlapped line too)
                current_para = index;
            }
            else
            {
                //todo implement
                auto delta = line.selfid - line.master; // master always less or eq selfid
                index -= delta;
                caret = batch[index].stanza;
                caret->chx(delta * width + new_coord.x);
                //todo implement checking hit by x

                //todo implement the case when the coord is set to the outside viewport
                //     after the right side: disable wrapping (on overlapped line too)
                //     before the left side: disable wrapping + bias::right (on overlapped line too)
                current_para = index;
            }

            caret->brush = brush;
        }
        void set_coord() { set_coord(coord); }

        void finalize()
        {
            auto point = batch.begin() + current_para;

            auto& item = *point;
            auto old_size = item.length;
            auto old_height = item.line_height(width);
            item.cook();
            auto new_size = item.length;
            auto new_height = item.line_height(width);
            //wrapln = 
            auto id = item.selfid;
            //log("current_para ",current_para, "new_height ", new_height, " old_height ", old_height);
            if (new_height > old_height)
            {
                //auto delta = new_height - old_height;
                auto dist_to_end = std::distance(point, batch.end());
                if (new_height > dist_to_end)
                {
                    auto delta = new_height - dist_to_end;
                    //todo add new lines
                    auto brush = caret->brush;
                    auto wrp = item.wrapln;
                    auto jet = item.adjust;

                    while(delta-- > 0 )
                    {
                        auto& new_item = batch.emplace_back(++parid);
                        auto new_line = new_item.stanza;
                        new_line->brush = brush;
                        new_item.wrapln = wrp;
                        new_item.adjust = jet;

                        count++;
                        if (count - basis > viewport_height) basis = count - viewport_height;
                    }
                    // point is invalidated due to batch resize
                    point = batch.begin() + current_para;
                }
                auto head = point + new_height;
                auto tail = point + old_height;
                while(head-- != tail)
                {
                    auto& item = *head;
                    // Assign iff линия не накрыта кем-то выше
                    if (item.selfid - id > 0) // In order to support id incrementing overflow
                        item.master = id; 
                    else break; // линия накрыта кем то выше
                }
            }
            else if (new_height < old_height)
            {
                auto head = point + old_height;
                auto tail = point + new_height;
                while(head-- != tail)
                {
                    auto& item2 = *head;
                    auto old_master = item2.master;
                    if (old_master != id) break; // эта линия как и все вышележащие были накрыты кем то ранее, длинной строкой выше
                    auto head2 = point;
                    item2.master = item2.selfid;
                    while(++head2 != head)
                    {
                        auto& over = *head2;
                        auto h = over.line_height(width);
                        auto d = head - head2;
                        if (h > d)
                        {
                            item2.master = (*head2).selfid;
                            break;
                        }
                    }
                }
            }

            if (item.wrapln)
            {
                auto len = caret->chx();
                //if (len && !(len % width)) len--;

                if (len && (len % width == 0))
                {
                    len--;
                    coord.x = width;
                }
                else
                {
                    coord.x = len % width;
                }
                coord.y = len / width + current_para - basis;
                //coord.x = len % width;
                //coord.y = len / width + current_para - basis;
                //log(" item: ", current_para, " coord:", coord);
            }
            else
            {
                coord.x = caret->chx();
                coord.y = current_para - basis;
            }
        }
        auto& clear(bool preserve_brush = faux)
        {
            brush = preserve_brush ? caret->brush
                                   : cell{};
            batch.clear();    
            parid = 0;
            count = 0;
            basis = 0;
            upset.set(0);
            caret = batch.emplace_back(parid).stanza;
            //caret->wipe(brush);
            caret->brush = brush;
            current_para = count++;
            if (count - basis > viewport_height) basis = count - viewport_height;
            return *this;
        }
        // rods: Add new line.
        void fork()
        {
            //caret->cook();
            finalize();
            brush = caret->brush;
            auto& item = batch[current_para];
            auto wrp = item.wrapln;
            auto jet = item.adjust;
            auto& new_item = batch.emplace_back(++parid);
            caret = new_item.stanza;
            caret->brush = brush;
            new_item.wrapln = wrp;
            new_item.adjust = jet;

            current_para = count++;
            if (count - basis > viewport_height) basis = count - viewport_height;
        }

        auto reflow()
        {
            flow::reset();
            //todo Determine page internal caret position
            auto current_coord = dot_00;
            // Output lines in backward order from bottom to top
            auto tail = batch.rbegin();
            auto head = batch.rend();
            auto coor = twod{ 0, count };
            //todo optimize: print only visible (TIA canvas offsets)
            while(tail != head)
            {
                auto& line = *tail++;
                auto& rod = *line.stanza;
                --coor.y;
                flow::cursor = coor;
                flow::wrapln = line.wrapln;
                flow::adjust = line.adjust;
                flow::up(); // Append empty lines to flow::minmax
                flow::print<faux>(rod);
                brush = rod.mark(); // current mark of the last printed fragment
            }

            flow::minmax(current_coord);
            auto& cover = flow::minmax();
            upset.set(-std::min(0, cover.l),
                       std::max(0, cover.r - width + 1),
                      -std::min(0, cover.t),
                       0);
            auto height = cover.height() + 1;
            if (auto delta = height - count)
            {
                while(delta-- > 0 ) fork();
            }
            //count = static_cast<iota>(batch.size());
            //if (height != count) throw;

            if (count - basis > viewport_height) basis = count - viewport_height;

            return twod{ width, height };
        }
        void output(face& canvas)
        {
            //todo do the same that the reflow does but using canvas

            flow::reset();
            flow::corner = canvas.corner();

            // Output lines in backward order from bottom to top
            auto tail = batch.rbegin();
            auto head = batch.rend();
            auto coor = twod{ 0, count };
            //todo optimize: print only visible (TIA canvas offsets)
            while(tail != head)
            {
                auto& line = *tail++;
                auto& rod = *line.stanza;
                --coor.y;
                flow::cursor = coor;
                flow::wrapln = line.wrapln;
                flow::adjust = line.adjust;
                flow::print<faux>(rod, canvas);
                brush = rod.mark(); // current mark of the last printed fragment
            }
        }
        //auto size() const { return count; }
        //void size(iota limit)
        //{
        //    //todo resize ring buffer
        //}
        auto height() const
        {
            return count;
        }
        void height(iota limits)
        {
            //todo resize ring buffer
        }
        auto cp()
        {
            auto pos = coord;
            pos.y += basis;
            return pos;
        }
    };

    class lane // terminal: scrollback/altbuf internals
        : public page,
          public flow
    {
        iota const& width; // lane: Lane dimension.
        cell brush;        // lane: Current brush.
        id_t piece = 1;    // lane: The nearest to top paragraph.
        //todo revise
        bool decoy = true; // lane: Is the cursor inside the viewport?

        twod& anker;       // lane: The position of the nearest visible paragraph.
        side& oversize;

        page_layout layout;

    public:
        iota height = 0;   // lane: Lane dimension.
        twod current_coord;

        lane(twod& anker, side& oversize, twod const& viewport_size)
            : flow    { viewport_size.x, height },
              width   { viewport_size.x },
              anker   { anker },
              oversize{ oversize }
        { }

        // lane: Reflow text page on canvas and hold the position
        //       of the top visible paragraph while resizing.
        auto reflow()
        {
            if (page::size() > layout.capacity()) layout.reserve(page::size() * 2);

            auto entry = layout.get_entry(anker); // Take the entry under central point
            layout.clear();

            // Active paragraph id
            auto current_parid = page::layer->id();
            //log(" page::layer->id(): ", page::layer->id());

            flow::reset();
            auto publish = [&](auto const& combo)
            {
                auto cp = flow::print(combo);
                //log(" cp: ", cp);

                auto id = combo.id();

                // Determine page internal caret position
                if (id == current_parid)
                {
                    if (flow::wrapln)
                    {
                        //todo take wrapping into account
                        auto mx = width;
                        current_coord = cp;
                        //log(" cur_pos: ", current_coord);

                        current_coord.x += combo.caret();

                        if (auto dy = current_coord.x / mx)
                        {
                            current_coord.y += dy;
                            current_coord.x = current_coord.x % mx;
                        }
                    }
                    else
                    {
                        current_coord = cp;
                        //log(" cp: ", cp);
                        current_coord.x += combo.caret();
                        //log(" current_coord: ", current_coord);
                    }
                }

                // Forming page vertical layout
                if (id == entry.id) entry.coor.y -= cp.y;
                layout.push_back({ id,cp });
            };
            page::stream(publish);

            // Apply only vertical anchoring for this type of control
            anker.y -= entry.coor.y; // Move the central point accordingly to the anchored object

            //auto cur_pos = flow::up();
            //log(" cur_pos: ", cur_pos);
            flow::minmax(current_coord);
            //log(" cur_pos: ", current_coord);

            //if (caret_show) flow::minmax(current_coord);

            auto& cover = flow::minmax();
            //log(" reflow oversize: ", cover);
            oversize.set(-std::min(0, cover.l),
                          std::max(0, cover.r - width + 1),
                         -std::min(0, cover.t),
                          0);
            height = cover.height() + 1;
            return twod{ width, height };
        }
        //auto set_width(iota new_width)
        //{
        //	width = new_width;
        //	return reflow();
        //}
        // lane: Print page.
        void output(face& canvas)//, page const& textpage)
        {
            flow::reset();
            flow::corner = canvas.corner();

            auto publish = [&](auto const& combo)
            {
                flow::print(combo, canvas);
                brush = combo.mark(); // current mark of the last printed fragment
            };

            //todo output only subset of page
            page::stream(publish);
        }
        void clear(bool preserve_brush = faux)
        {
            page::clear(preserve_brush);
            flow::reset();
        }
        auto cp()
        {
            return current_coord;
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

                    vt::csier.table[CSI_CNL] = vt::csier.table[CSI_CUD]; // CSI n E
                    vt::csier.table[CSI_CPL] = vt::csier.table[CSI_CUU]; // CSI n F
                    vt::csier.table[CSI_CHX] = VT_PROC{ p->chx( q(1)); };  // CSI n G - hz absolute
                    vt::csier.table[CSI_CHY] = VT_PROC{ p->chy( q(1)); };  // CSI n d - vt absolute 
                    vt::csier.table[CSI_CUP] = VT_PROC{ p->cup( q   ); };  // CSI y ; x H (1-based)
                    vt::csier.table[CSI_HVP] = VT_PROC{ p->cup( q   ); };  // CSI y ; x f (1-based)

                    vt::csier.table[CSI_DCH] = VT_PROC{ p->dch( q(1)); };  // CSI n P - del n chars
                    vt::csier.table[CSI_ECH] = VT_PROC{ p->ech( q(1)); };  // CSI n X - erase n chars
                    vt::csier.table[CSI__ED] = VT_PROC{ p->ed ( q(0)); };  // CSI Ps J
                    vt::csier.table[CSI__EL] = VT_PROC{ p->el ( q(0)); };  // CSI Ps K

                    vt::intro[ctrl::BS ]     = VT_PROC{ p->cuf(-q.pop_all(ctrl::BS )); };
                    vt::intro[ctrl::DEL]     = VT_PROC{ p->del( q.pop_all(ctrl::DEL)); };
                    vt::intro[ctrl::TAB]     = VT_PROC{ p->tab( q.pop_all(ctrl::TAB)); };
                    vt::intro[ctrl::EOL]     = VT_PROC{ p->eol( q.pop_all(ctrl::EOL)); };
                    vt::intro[ctrl::CR ]     = VT_PROC{
                            if (q.pop_if(ctrl::EOL)) p->eol(1);
                            else                     p->home(); };

                    vt::csier.table_quest[DECSET] = VT_PROC{ p->boss.decset(p, q); };
                    vt::csier.table_quest[DECRST] = VT_PROC{ p->boss.decrst(p, q); };
                    vt::csier.table_excl [DECSTR] = VT_PROC{ p->boss.decstr(); }; // CSI ! p = Soft terminal reset (DECSTR)

                    vt::csier.table[CSI_SGR][SGR_RST] = VT_PROC{ p->nil(); }; // fx_sgr_rst       ;
                    vt::csier.table[CSI_SGR][SGR_SAV] = VT_PROC{ p->sav(); }; // fx_sgr_sav       ;
                    vt::csier.table[CSI_SGR][SGR_FG ] = VT_PROC{ p->rfg(); }; // fx_sgr_fg_def    ;
                    vt::csier.table[CSI_SGR][SGR_BG ] = VT_PROC{ p->rbg(); }; // fx_sgr_bg_def    ;

                    vt::csier.table[CSI_CCC][CCC_JET] = VT_PROC{ p->jet(q(0)); }; // fx_ccc_jet  default=bias::left=0
                    vt::csier.table[CSI_CCC][CCC_WRP] = VT_PROC{ p->wrp(q(1)); }; // fx_ccc_wrp  default=true
                    vt::csier.table[CSI_CCC][CCC_RTL] = nullptr; // fx_ccc_rtl //todo implement
                    vt::csier.table[CSI_CCC][CCC_RLF] = nullptr; // fx_ccc_rlf

                    vt::oscer[OSC_0] = VT_PROC{ p->boss.prop(OSC_0, q); };
                    vt::oscer[OSC_1] = VT_PROC{ p->boss.prop(OSC_1, q); };
                    vt::oscer[OSC_2] = VT_PROC{ p->boss.prop(OSC_2, q); };
                    vt::oscer[OSC_3] = VT_PROC{ p->boss.prop(OSC_3, q); };
                }
            };

            term& boss;
            cell& mark; // wall: Shared current brush state (default brush).

            wall(term& boss)
                : rods( boss.base::anchor, boss.base::oversize, boss.viewport.size ),
                  mark{ boss.base::color() },
                  boss{ boss }
            { }

            // Implement base-CSI contract (see ansi::csi_t)
            void task(ansi::rule const& cmd)
            {
                if (!caret->empty())
                {
                    fork();
                }
                caret->locus.push(cmd);
            }
            void post(utf::frag const& cluster)
            { 
                caret->post(cluster); 
            }
            //void cook()                         { caret->cook(); }
            void cook()                         { finalize(); }
            void  nil()                         { caret->nil(mark); }
            void  sav()                         { caret->sav(mark); }
            void  rfg()                         { caret->rfg(mark); }
            void  rbg()                         { caret->rbg(mark); }
            void  fgc(rgba const& c)            { caret->fgc(c); }
            void  bgc(rgba const& c)            { caret->bgc(c); }
            void  bld(bool b)                   { caret->bld(b); }
            void  itc(bool b)                   { caret->itc(b); }
            void  inv(bool b)                   { caret->inv(b); }
            void  und(bool b)                   { caret->und(b); }

            void  wrp(bool b)                   { batch[current_para].wrapln = b; }
            void  jet(iota n)                   { batch[current_para].adjust = (bias)n; }

            // Implement text manipalation procs
            //
            void tab(iota n) { caret->ins(n); }
            // wall: CSI n X  Erase/put n chars after cursor. Don't change cursor pos
            void ech(iota n) // wall: Erase letters after caret. CSI n X
            {
                if (n > 0)
                {
                    finalize();
                    auto pos = caret->chx();
                    caret->ins(n);
                    caret->chx(pos);
                    finalize();
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
                cook();
                auto& lyric = *rods::caret->lyric;
                auto size = lyric.size().x;
                auto caret = rods::caret->chx();
                //todo unify for size.y > 1
                //todo revise shifting to the left (right? RTL?)
                if (n > 0)
                {
                    //if (size < caret + n)
                    //{
                    //	lyric.crop(caret + n);
                    //}
                    if (caret < size)
                    {
                        if (n >= size - caret)
                        {
                            auto dst = lyric.data() + caret;
                            auto end = dst + (size - caret);
                            auto b = rods::caret->brush;
                            b.txt(whitespace);
                            while (dst != end)
                            {
                                *dst++ = b;
                            }
                        }
                        else
                        {
                            auto dst = lyric.data() + caret;
                            auto end = dst + std::min(n, size - caret);
                            auto src = dst + n;
                            while (dst != end)
                            {
                                *dst++ = *src++;
                            }
                            auto b = rods::caret->brush;
                            b.txt(whitespace);
                            end = lyric.data() + size;
                            while (dst != end)
                            {
                                *dst++ = b;
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
                coord.x += n;
                set_coord();
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
                auto p = twod(queue(1), queue(1));
                coord = p - dot_11;
                set_coord();
            }
            // wall: Line feed up
            template<bool PRESERVE_BRUSH = true>
            void up(iota n)
            {
                finalize();
                coord.y -= n;
                set_coord();
            }
            // wall: Line feed down
            void dn(iota n)
            {
                finalize();
                coord.y += n;
                set_coord();
            }
            // wall: '\r'  Go to home of visible line instead of home of para
            void home()
            {
                finalize();
                coord.x = 0;
                set_coord();
            }
            // wall: '\n' || '\r\n'  Carriage return + Line feed
            void eol(iota n)
            {
                finalize();
                coord.x = 0;
                coord.y += n;
                set_coord();
            }
            // wall: CSI n J  Erase display
            void ed(iota n)
            {
                switch (n)
                {
                    case commands::erase::display::below: // Ps = 0  ⇒  Erase Below (default).
                        //todo implement
                        finalize();
                        log("\\e[J is not implemented");
                        break;
                    case commands::erase::display::above: // Ps = 1  ⇒  Erase Above.
                        //todo implement
                        finalize();
                        log("\\e[1J is not implemented");
                        break;
                    case commands::erase::display::viewport: // Ps = 2  ⇒  Erase All.
                    {
                        finalize();
                        auto current_brush = caret->brush;

                        log("\\e[2J is not implemented");

                        caret->brush = current_brush;
                    }
                    break;
                    case commands::erase::display::scrollback: // Ps = 3  ⇒  Erase Scrollback
                    {
                        rods::clear();
                    }
                    break;
                    default:
                        break;
                }
            }
            // wall: CSI n K  Erase line (don't move caret)
            void el(iota n)
            {
                finalize();
                auto& lyric = *caret->lyric;

                switch (n)
                {
                    default:
                    case commands::erase::line::right: // Ps = 0  ⇒  Erase to Right (default).
                    {
                        // ConPTY doesn't move caret (is it ok?)
                        //todo optimize
                        auto brush = caret->brush;
                        brush.txt(' ');
                        auto coor = caret->chx();
                        auto width = boss.viewport.size.x;
                        lyric.crop({ coor,1 }, brush);
                        lyric.crop({ coor + (width - coor % width),1 }, brush);
                        caret->trim(mark);
                        //caret->chx(coor + (width - coor % width));
                        break;
                    }
                    case commands::erase::line::left: // Ps = 1  ⇒  Erase to Left.
                    {
                        auto brush = caret->brush;
                        brush.txt(' ');
                        auto coor = caret->chx();
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
                        auto coor  = caret->chx();
                        auto brush = caret->brush;
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

        struct wall_old // term: VT-behavior for the lane
            : public lane
        {
            template<class T>
            struct parser : public page::parser<T>
            {
                using base = page::parser<T>;
                parser() : base()
                {
                    //todo every caret movement cause to change the page input point
                    //todo calc a page input point by the caret coord

                    // 1.not! every viewport's scanline has a SINGLE row
                    // 3.not! viewport bound at the scrollback bottom

                    // 2. row can be wrapped across multiple scanlines
                    // 4. rows don't have absolute positioning commands, only generic flow
                    // 5. page_layout is automatically sorted in ascending order
                    // 6. visible rows detection can be done by binary search
                    // 7. adding a new row allowed only on the last row
                    // 8. caret is always one cell size

                    using namespace netxs::console::ansi;
                    base::csier.table[CSI_CUU] = VT_PROC{ p->up ( q(1)); };  // CSI n A
                    base::csier.table[CSI_CUD] = VT_PROC{ p->dn ( q(1)); };  // CSI n B
                    base::csier.table[CSI_CUF] = VT_PROC{ p->cuf( q(1)); };  // CSI n C
                    base::csier.table[CSI_CUB] = VT_PROC{ p->cuf(-q(1)); };  // CSI n D

                    base::csier.table[CSI_CNL] = base::csier.table[CSI_CUD]; // CSI n E
                    base::csier.table[CSI_CPL] = base::csier.table[CSI_CUU]; // CSI n F
                    base::csier.table[CSI_CHX] = VT_PROC{ p->chx( q(1)); };  // CSI n G - hz absolute
                    base::csier.table[CSI_CHY] = VT_PROC{ p->chy( q(1)); };  // CSI n d - vt absolute 
                    base::csier.table[CSI_CUP] = VT_PROC{ p->cup( q   ); };  // CSI y ; x H (1-based)
                    base::csier.table[CSI_HVP] = VT_PROC{ p->cup( q   ); };  // CSI y ; x f (1-based)

                    base::csier.table[CSI_DCH] = VT_PROC{ p->dch( q(1)); };  // CSI n P - del n chars
                    base::csier.table[CSI_ECH] = VT_PROC{ p->ech( q(1)); };  // CSI n X - erase n chars
                    base::csier.table[CSI__ED] = VT_PROC{ p->ed ( q(0)); };  // CSI Ps J
                    base::csier.table[CSI__EL] = VT_PROC{ p->el ( q(0)); };  // CSI Ps K

                    base::intro[ctrl::BS ]     = VT_PROC{ p->cuf(-q.pop_all(ctrl::BS )); };
                    base::intro[ctrl::DEL]     = VT_PROC{ p->del( q.pop_all(ctrl::DEL)); };
                    base::intro[ctrl::TAB]     = VT_PROC{ p->tab( q.pop_all(ctrl::TAB)); };
                    base::intro[ctrl::EOL]     = VT_PROC{ p->eol( q.pop_all(ctrl::EOL)); };
                    base::intro[ctrl::CR ]     = VT_PROC{
                            if (q.pop_if(ctrl::EOL)) p->eol(1);
                            else                     p->home(); };

                    base::csier.table_quest[DECSET] = VT_PROC{ p->boss.decset(p, q); };
                    base::csier.table_quest[DECRST] = VT_PROC{ p->boss.decrst(p, q); };
                    base::csier.table_excl [DECSTR] = VT_PROC{ p->boss.decstr(); }; // CSI ! p = Soft terminal reset (DECSTR)

                    base::csier.table[CSI_SGR][SGR_RST] = VT_PROC{ p->nil(); }; // fx_sgr_rst       ;
                    base::csier.table[CSI_SGR][SGR_SAV] = VT_PROC{ p->sav(); }; // fx_sgr_sav       ;
                    base::csier.table[CSI_SGR][SGR_FG ] = VT_PROC{ p->rfg(); }; // fx_sgr_fg_def    ;
                    base::csier.table[CSI_SGR][SGR_BG ] = VT_PROC{ p->rbg(); }; // fx_sgr_bg_def    ;

                    base::oscer[OSC_0] = VT_PROC{ p->boss.prop(OSC_0, q); };
                    base::oscer[OSC_1] = VT_PROC{ p->boss.prop(OSC_1, q); };
                    base::oscer[OSC_2] = VT_PROC{ p->boss.prop(OSC_2, q); };
                    base::oscer[OSC_3] = VT_PROC{ p->boss.prop(OSC_3, q); };
                }
            };

            term& boss;
            cell& mark; // wall: Shared current brush state (default brush).

            wall_old(term& boss)
                : lane( boss.base::anchor, boss.base::oversize, boss.viewport.size ),
                  mark{ boss.base::color() },
                  boss{ boss }

            { }

            // wall: SGR 0  Reset SGR attributes
            void nil() { page::layer->nil(mark); }
            // wall: SGR 10  Save SGR attributes
            void sav() { page::layer->sav(mark); }
            // wall: SGR 39  Reset fg color
            void rfg() { page::layer->rfg(mark); }
            // wall: SGR 49  Reset bg color
            void rbg() { page::layer->rbg(mark); }
            // wall: CSI n X  Erase/put n chars after cursor. Don't change cursor pos
            void ech(iota n) // wall: Erase letters after caret. CSI n X
            {
                if (n > 0)
                {
                    page::cook();
                    auto pos = page::layer->chx();
                    page::layer->ins(n);
                    page::cook();
                    page::layer->chx(pos);
                }

            }
            // wall: CSI n P  Delete (not Erase) letters under the caret.
            void dch(iota n)
            {
                /*
                 * del:
                 *    As characters are deleted, the remaining characters
                 *    between the cursor and right margin move to the left.
                 *    Character attributes move with the characters.
                 *    The terminal adds blank characters at the right margin.
                 */

                page::cook();

                auto& lyric = *page::layer->lyric;
                auto size = lyric.size().x;

                auto caret = page::layer->chx();

                //todo unify for size.y > 1
                //todo revise shifting to the left (right? RTL?)
                if (n > 0)
                {
                    //if (size < caret + n)
                    //{
                    //	lyric.crop(caret + n);
                    //}

                    if (caret < size)
                    {
                        if (n >= size - caret)
                        {
                            auto dst = lyric.data() + caret;
                            auto end = dst + (size - caret);
                            auto b = page::layer->brush;
                            b.txt(whitespace);
                            while (dst != end)
                            {
                                *dst++ = b;
                            }
                        }
                        else
                        {
                            auto dst = lyric.data() + caret;
                            auto end = dst + std::min(n, size - caret);
                            auto src = dst + n;
                            while (dst != end)
                            {
                                *dst++ = *src++;
                            }

                            auto b = page::layer->brush;
                            b.txt(whitespace);
                            end = lyric.data() + size;
                            while (dst != end)
                            {
                                *dst++ = b;
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
                auto& layer = page::layer;
                if (layer->chx() >= n)
                {
                    layer->chx(layer->chx() - n);
                    layer->del(n);
                }
                else
                {
                    auto  here = layer->chx();
                    auto there = n - here;

                    layer->chx(0);
                    if (here) layer->del(here);

                    {
                        if (layer != page::batch.begin())
                        {
                            if (!layer->length())
                            {
                                if (layer->locus.bare())
                                {
                                    layer = std::prev(page::batch.erase(layer));
                                }
                                else
                                {
                                    layer->locus.pop();
                                }
                                there--;
                            }
                            else
                            {
                                auto prev = std::prev(layer);
                                *(*prev).lyric += *(*layer).lyric;
                                page::batch.erase(layer);
                                layer = prev;
                                layer->chx(layer->length());
                            }
                        }
                    }
                }
            }
            // wall: CSI n G  Absolute horizontal caret position (1-based)
            void chx(iota n)
            {
                if (n-- > 0)
                {
                    //page::chx(n);
                    page::layer->chx(n);
                    //log(" CHX: ", n);
                }
            }
            // wall: CSI n d  Absolute vertical caret position (1-based)
            void chy(iota n)
            {
                if (n > 0)
                {
                    page::cook();
                    page::layer->trim(mark);

                    auto height = boss.viewport.size.y;
                    auto width  = boss.viewport.size.x;
                    auto current_brush = page::layer->brush;

                    auto x = page::layer->chx() % width;

                    n = std::min(n, height);
                    page::layer = std::prev(page::batch.end());
                    page::cook();
                    page::layer->trim(mark);

                    auto sz = page::layer->size().x;
                    auto dy = std::max(0, sz - 1) / width;
                    auto xy = dy * width + x;
                    page::layer->chx(xy);


                    if (auto pos = height - n)
                    {
                        up<faux>(pos);
                    }

                    page::layer->brush = current_brush;
                }
            }
            // wall: CSI y; x H/F  Caret position (1-based)
            void cup(fifo& queue)
            {
                auto p = twod(queue(1), queue(1));
                p = std::clamp(p, dot_11, boss.viewport.size);

                auto x = p.x - 1;
                auto y = p.y - 1;

                page::cook();
                page::layer->trim(mark);

                auto current_brush = page::layer->brush;
                auto width  = boss.viewport.size.x;
                auto height = boss.viewport.size.y;

                page::layer = std::prev(page::batch.end());
                page::cook();
                page::layer->trim(mark);

                auto sz = page::layer->size().x;
                auto dy = std::max(0, sz - 1) / width;
                auto xy = dy * width + x;
                auto dx = x;
                page::layer->chx(xy);

                if (auto pos = height - p.y)
                {
                    up<faux>(pos);
                }

                page::layer->brush = current_brush;
            }
            // wall: Line feed up
            template<bool PRESERVE_BRUSH = true>
            void up(iota n)
            {
                if (n > 0)
                {
                    page::cook();
                    page::layer->trim(mark);
                    auto& width = boss.viewport.size.x;
                    auto current_brush = page::layer->brush;
                    
                    //auto sz = page::layer->size();
                    auto xy = page::layer->chx();
                    auto dx = xy % width;
                    auto dy = xy / width;
                    do
                    {
                        if (n > dy)
                        {
                            n -= dy + 1;
                            if (page::layer != page::batch.begin())
                            {
                                page::layer--;
                                auto sz = page::layer->size().x;
                                dy = std::max(0, sz - 1) / width;
                                xy = dy * width + dx;
                            }
                            else
                            {
                                xy = dx;
                                n = 0;
                            }
                        }
                        else
                        {
                            xy = (dy - n) * width + dx;
                            n = 0;
                        }
                    }
                    while (n > 0);
                    
                    page::layer->chx(xy);
                    if (PRESERVE_BRUSH)
                        page::layer->brush = current_brush;
                }
            }
            // wall: append empty lines to the page
            void adds(iota n)
            {
                if (n > 0) // Add new lines
                {
                    auto current_layer = page::layer;
                    page::layer = std::prev(page::batch.end());
                    
                    while (n-- > 0)
                    {
                        page::fork();
                        page::layer->locus.push({ ansi::fn::nl, 1 });
                    }
                    page::layer = current_layer;
                }
                else if (n < 0) // Remove empty lines at the end
                {
                    auto current_layer = page::layer;
                    page::layer = std::prev(page::batch.end());
                    while (n++ < 0)
                    {
                        if (page::size() > 1)
                        {
                             page::layer->trim(mark);
                             if (page::layer->size().x == 0)
                             {
                                 if (current_layer == std::prev(page::batch.end()))
                                 {
                                     --current_layer;
                                 }
                                 page::batch.pop_back();
                                 page::layer = std::prev(page::batch.end());
                             }
                            else break;
                        }
                        else break;
                    }
                    page::layer = current_layer;
                }
            }
            // wall: Line feed down
            void dn(iota n)
            {
                if (n > 0)
                {
                    page::cook();
                    page::layer->trim(mark);

                    auto c = page::layer->chx();
                    auto current_brush = page::layer->brush;

                    while (n--)
                    {
                        if (page::layer != std::prev(page::batch.end()))
                        {
                            page::layer++;
                        }
                        else
                        {
                            adds(n + 1);
                            page::layer = std::prev(page::batch.end());
                            break;
                        }
                    }
                    page::layer->chx(c);
                    page::layer->brush = current_brush;
                }
            }
            // wall: '\r'  Go to home of visible line instead of home of para
            void home()
            {
                page::cook();
                auto& width = boss.viewport.size.x;
                auto c = page::layer->chx();
                //if (flow::wrapln)
                c = c - c % width;
                page::layer->chx(c);
            }
            // wall: '\n' || '\r\n'  Carriage return + Line feed
            void eol(iota n)
            {
                page::cook();

                // trim all after cursor
                //auto& lyric = *page::layer->lyric;
                //auto coor = page::layer->chx();
                //auto mark = page::layer->brush;
                //lyric.crop({ coor,1 }, mark);

                page::layer->chx(0);

                dn(n);
            }
            // wall: CSI n J  Erase display
            void ed(iota n)
            {
                switch (n)
                {
                    case commands::erase::display::below: // Ps = 0  ⇒  Erase Below (default).
                        //todo implement
                        log("\\e[J is not implemented");
                        break;
                    case commands::erase::display::above: // Ps = 1  ⇒  Erase Above.
                        //todo implement
                        log("\\e[1J is not implemented");
                        break;
                    case commands::erase::display::viewport: // Ps = 2  ⇒  Erase All.
                    {
                        page::cook();
                        auto current_brush = page::layer->brush;

                        page::layer = page::batch.end();
                        auto size = boss.viewport.size.y;
                        //auto size = std::min<iota>(page::size(), boss.viewport.size.y);
                        //log("boss.viewport.size.y: ", boss.viewport.size.y);
                        //log("page::size(): ", page::size());
                        while (size--)
                        {
                            (*--page::layer).lyric->crop(0);
                        }
                        page::layer->chx(0);
                        page::layer->brush = current_brush;
                    }
                    break;
                    case commands::erase::display::scrollback: // Ps = 3  ⇒  Erase Scrollback
                    {
                        if (page::size() > boss.viewport.size.y)
                        {
                            auto current_limit = page::maxlen();
                            page::maxlen(boss.viewport.size.y);
                            page::maxlen(current_limit);
                        }
                        ed(commands::erase::display::viewport);
                        //log("erase scrollback: scroll size: ", page::size());
                    }
                    break;
                    default:
                        break;
                }
            }
            // wall: CSI n K  Erase line (don't move caret)
            void el(iota n)
            {
                //log(" el ", n);
                page::cook();

                auto& lyric = *page::layer->lyric;

                switch (n)
                {
                    default:
                    case commands::erase::line::right: // Ps = 0  ⇒  Erase to Right (default).
                    {
                        //todo optimize
                        auto brush = page::layer->brush;
                        brush.txt(' ');
                        auto coor = page::layer->chx();
                        auto width = boss.viewport.size.x;
                        //auto sss = width - coor;
                        lyric.crop({ coor,1 }, brush);
                        lyric.crop({ coor + (width - coor % width),1 }, brush);

                        page::layer->trim(mark);


                        // ConPTY doesn't move caret (is it ok?)
                        //page::layer->chx(coor + (width - coor % width));
                        break;
                    }
                    case commands::erase::line::left: // Ps = 1  ⇒  Erase to Left.
                    {
                        //todo implement
                        auto brush = page::layer->brush;
                        brush.txt(' ');
                        auto coor = page::layer->chx();
                        auto width = boss.viewport.size.x;
                        //auto sss = width - coor;
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
                        //page::task(ansi::rule{ ansi::fn::el, 1 });
                        break;
                    }
                    case commands::erase::line::all: // Ps = 2  ⇒  Erase All.
                    {
                        //todo optimize
                        auto coor = page::layer->chx();
                        auto brush = page::layer->brush;
                        brush.txt(' ');
                        auto width = boss.viewport.size.x;
                        //auto sss = width - coor;

                        auto left_edge = coor - coor % width;

                        lyric.crop({ left_edge,1 }, brush);
                        lyric.crop({ left_edge + width,1 }, brush);

                        //lyric.crop({ 0,1 }, brush);
                        //lyric.crop({ width,1 }, brush);

                        //page::task(ansi::rule{ ansi::fn::el, 2 });
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
        std::map<iota, text> props;

        rect viewport = { {}, dot_11 }; // term: Viewport area

        // term: Apply page props.
        void prop(iota cmd, view txt)
        {
            auto& utf8 = (props[cmd] = txt);
            switch (cmd)
            {
                case ansi::OSC_0: // prop_name::icon_title:
                case ansi::OSC_1: // prop_name::icon:
                case ansi::OSC_2: // prop_name::x_prop:
                case ansi::OSC_3: // prop_name::title:
                    base::riseup<e2::preview, e2::form::prop::header>(utf8);
                    break;
                default:
                    break;
            }
        }

        // term: Soft terminal reset (DECSTR)
        void decstr()
        {
            //todo revise
            //altbuf.ed(commands::erase::display::scrollback);
            //scroll.ed(commands::erase::display::scrollback);
            scroll.clear();
            altbuf.clear();
            target = &scroll;
        }
        void decset(wall*& p, fifo& queue)
        {
            while (auto q = queue(0))
            {
                switch (q)
                {
                    case 1:    // Cursor keys application mode.
                        mode_DECCKM = true;
                        break;
                    case 25:   // Caret on.
                        caret.show();
                        break;
                    case 1000: // Send Mouse X & Y on button press and release.
                    case 1001: // Use Hilite Mouse Tracking.
                    case 1002: // Use Cell Motion Mouse Tracking.
                    case 1003: // Use All Motion Mouse Tracking.
                    case 1005: // Enable UTF - 8 Mouse Mode.
                        break;
                    case 1006: // Enable SGR Mouse Mode.
                        break;

                    case 1004: // Send FocusIn / FocusOut events.
                        break;

                    case 1048: // Save cursor
                        break;
                    case 1047: // Use Alternate Screen Buffer
                    case 1049: // Save cursor and Use Alternate Screen Buffer, clearing it first.  This control combines the effects of the 1 0 4 7 and 1 0 4 8  modes.
                        target->cook();
                        target = &altbuf;
                        break;
                    default:
                        break;
                }
            }
        }
        void decrst(wall*& p, fifo& queue)
        {
            while (auto q = queue(0))
            {
                switch (q)
                {
                    case 1:    // Cursor keys ANI mode.
                        mode_DECCKM = faux;
                        break;
                    case 25:   // Caret off.
                        caret.hide();
                        break;
                    case 1000: // Dont send Mouse X & Y on button press and release.
                    case 1001: // Dont use Hilite(c) Mouse Tracking.
                    case 1002: // Dont use Cell Motion Mouse Tracking.
                    case 1003: // Dont use All Motion Mouse Tracking.
                    case 1005: // Disable UTF - 8 Mouse Mode.
                        break;
                    case 1006: // Disable SGR Mouse Mode.
                        break;

                    case 1004: // Dont Send FocusIn / FocusOut events.
                        break;

                    case 1048: // Restore cursor
                        break;
                    case 1047: // Use Normal Screen Buffer
                    case 1049: // Use Normal Screen Buffer and restore cursor
                        target->cook();
                        target = &scroll;
                        break;
                    default:
                        break;
                }
            }
        }

    public:
        term(twod winsz, text cmdline)
        {
            caret.show();

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
                if (mode_DECCKM)
                {
                    auto trans = gear.keystrokes;
                    utf::change(trans, "\033[A", "\033OA");
                    utf::change(trans, "\033[B", "\033OB");
                    utf::change(trans, "\033[C", "\033OC");
                    utf::change(trans, "\033[D", "\033OD");
                    ptycon.write(trans);
                }
                else
                {
                    ptycon.write(gear.keystrokes);
                }

                #ifdef KEYLOG
                    std::stringstream d;
                    view v = gear.keystrokes;
                    log("key strokes raw: ", v);
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
                //log("1. caret pos: ", target->cp());
                //log ("old viewport.size.y: ", viewport.size.y);
                if (viewport.size != new_size)
                {
                    viewport.size = new_size;
                    if (target == &altbuf)
                    {
                        //altbuf.ed(commands::erase::display::viewport);
                        altbuf.ed(commands::erase::display::scrollback);
                    }
                    //log("altbuf size: ", altbuf.size());
                    //log("scroll size: ", scroll.size());
                }

                auto new_pty_size = new_size;
                //log("new_pty_size: ", new_pty_size);

                auto scrollback_size = target->reflow();
                new_size = std::max(new_size, scrollback_size); // Use the max size

                //log("2. caret pos: ", target->cp());
                ptycon.resize(new_pty_size); // set viewport size
            };
            SUBMIT(e2::release, e2::form::layout::move, new_coor)
            {
                viewport.coor = -new_coor;
                //log("new viewport coor: ", viewport.coor);
            };

            ptycon.start(cmdline, winsz, [&](auto d) { input_hndl(d); });
        }

        ~term()
        {
            ptycon.close();
        }

        //bool caret_is_visible=true;
        void input_hndl(view shadow)
        {
            while (ptycon)
            {
                e2::try_sync guard;
                if (guard)
                {
                    SIGNAL(e2::general, e2::debug::output, shadow);

                    //auto old_caret_pos = target->cp();
                    auto old_caret_pos = caret.coor();
                    auto caret_is_visible = viewport.hittest(old_caret_pos);

                    ansi::parse(shadow, target); // Append using default insertion point

                    //if (target == &altbuf)
                    //{
                    //	altbuf.hz_trim(viewport.size.x);
                    //}

                    auto new_size = target->reflow();
                    auto caret_xy = target->cp();

                    // Place caret to the begining of the new line
                    //   in case it is at the end of line and it is wrapped
                    if (caret_xy.x == viewport.size.x
                        && target->wrapln)
                    {
                        caret_xy.x = 0;
                        caret_xy.y++;
                    }

                    caret.coor(caret_xy);

                    if (caret_is_visible && !viewport.hittest(caret_xy))
                    {
                        //auto old_caret_pos = viewport.coor + viewport.size - dot_11;
                        auto anchor_delta = caret_xy - old_caret_pos;
                        auto new_coor = base::coor.get() - anchor_delta;
                        SIGNAL(e2::release, base::move_event, new_coor);
                    }

                    SIGNAL(e2::release, base::size_event, new_size);

                    base::deface();
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
    };
}

#endif // NETXS_TERMINAL_H