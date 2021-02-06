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
            void cook(cell const& spare)
            {
                auto& item = *stanza;
                item.cook();
                item.trim(spare);
                auto size = item.size();
                length = size.x;
                //todo revise: height is always =1
                //height = size.y;

                //todo update wrapln and adjust
                //adjust = item...;
                //wrapln = item...;
            }
            // Make the line no longer than maxlen
            void trim_to(iota max_len)
            {
                stanza->trim_to(max_len);
                length = max_len;
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
        twod        coord; // rods: Actual caret position.
        cell        brush; // rods: Last used brush.
        parx        caret; // rods: Active insertion point.
        cell&       spare; // rods: Shared current brush state (default brush).
        iota current_para; // rods: Active insertion point index (not id).
        iota        basis; // rods: Index of O(0, 0).

        iota scroll_top    = 0; // rods: Scrolling region top. 1-based, 0: use top of viewport
        iota scroll_bottom = 0; // rods: Scrolling region bottom. 1-based, 0: use bottom of viewport

    public:
        rods(twod& anker, side& oversize, twod const& viewport_size, cell& spare)
            : flow { viewport_size.x, count   },
              parid{ 0                        },
              batch{ line(parid)              },
              width{ viewport_size.x          },
              count{ 1                        },
              upset{ oversize                 },
              caret{ batch.front().stanza     },
              basis{ 0                        },
              viewport_height{ viewport_size.y},
              spare{ spare                    },
              current_para{ 0 }
        { }

        void set_coord(twod new_coord)
        {
            // Checking bottom boundary
            auto min_y = -basis;
            new_coord.y = std::max(new_coord.y, min_y);

            coord = new_coord;
            new_coord.y += basis; // set coord inside batch

            if (new_coord.y > count - 1) // Add new lines
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
            auto line_len = line.length;

            auto index = line.selfid - batch.front().selfid; // current index inside batch
            brush = caret->brush;
            if (line.selfid == line.master) // no overlapped lines
            {
                caret = batch[index].stanza;
                caret->chx(new_coord.x);
                //todo implement the case when the coord is set to the outside viewport
                //     after the right side: disable wrapping (on overlapped line too)
                //     before the left side: disable wrapping + bias::right (on overlapped line too)
            }
            else
            {
                auto delta = line.selfid - line.master; // master always less or eq selfid
                index -= delta;
                caret = batch[index].stanza;
                caret->chx(delta * width + new_coord.x);
                //todo implement checking hit by x

                //todo implement the case when the coord is set to the outside viewport
                //     after the right side: disable wrapping (on overlapped line too)
                //     before the left side: disable wrapping + bias::right (on overlapped line too)
            }

            current_para = index;
            caret->brush = brush;
        }
        void set_coord() { set_coord(coord); }
        auto get_line_index_by_id(iota id)
        {
                auto begin = batch.front().selfid;
                auto index = id - begin;
                return index;
        }
        void finalize()
        {
            auto point = batch.begin() + current_para;

            auto& item = *point;
            auto old_size = item.length;
            auto old_height = item.line_height(width);
            item.cook(spare);
            auto new_size = item.length;
            auto new_height = item.line_height(width);
            //wrapln = 
            auto id = item.selfid;
            //log("current_para ",current_para, " new_height ", new_height, " old_height ", old_height);
            if (new_height > old_height)
            {
                auto dist_to_end = std::distance(point, batch.end());
                if (new_height > dist_to_end)
                {
                    // Add new lines
                    auto delta = new_height - dist_to_end;
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
                    }
                    if (count - basis > viewport_height) basis = count - viewport_height;
                    // point is invalidated due to batch resize
                    point = batch.begin() + current_para;
                }
                auto head = point + new_height;
                auto tail = point + old_height;
                while(head-- != tail)
                {
                    auto& below_item = *head;
                    // Assign iff line isn't overlapped by somaething
                    if (below_item.master - id > 0) // Comparing the difference with zero In order to support id incrementing overflow
                        below_item.master = id; 
                    else break; // overlapped by something higher
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

                //if (len && (len % width == 0))
                if (len && len == item.length && (len % width == 0))
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

            //todo update flow::minmax and base::upset
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
        auto cp()
        {
            auto pos = coord;
            pos.y += basis;
            if (pos.x == width
                && batch[current_para].wrapln)
            {
                pos.x = 0;
                pos.y++;
            }
            return pos;
        }
        auto reflow()
        {
            //todo recalc overlapping master id's over selfid's on resize

            flow::reset();
            //todo Determine page internal caret position
            //auto current_coord = dot_00;
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

            flow::minmax(cp()); // Register current caret position
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

            // Register viewport
            auto viewport = rect{{ 0, basis }, { width, viewport_height }};
            flow::minmax(viewport);
            height = cover.height() + 1;

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
        void remove_empties()
        {
            auto head = batch.begin();
            auto tail = std::prev(batch.end()); // Exclude the first line
            while(head != tail)
            {
                auto& line = *tail--;
                if (line.length == 0)
                {
                    batch.pop_back();
                    count--;
                    parid--;
                }
                else break;
            }
            if (current_para >= count)
            {
                current_para = count - 1;
                caret = batch[current_para].stanza;
            }
        }
        // Cut all lines above and current line
        void cut_above()
        {
            auto cur_line_it = batch.begin() + current_para;
            auto master_id = cur_line_it->master;
            auto mas_index = get_line_index_by_id(master_id);
            auto head = batch.begin() + mas_index;
            auto tail = cur_line_it;
            do
            {
                auto required_len = (current_para - mas_index) * width + coord.x; // todo optimize
                auto& line = *head;
                line.trim_to(required_len);
                mas_index++;
            }
            while(head++ != tail);
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
                    vt::csier.table[CSI_ICH] = VT_PROC{ p->ich( q(1)); };  // CSI n @ - insert n chars

                    vt::csier.table[CSI__ED] = VT_PROC{ p->ed ( q(0)); };  // CSI n J
                    vt::csier.table[CSI__EL] = VT_PROC{ p->el ( q(0)); };  // CSI n K
                    vt::csier.table[CSI__DL] = VT_PROC{ p->dl ( q(1)); };  // CSI n M - Delete n lines
                    vt::csier.table[CSI__SD] = VT_PROC{ p->sd ( q(1)); };  // CSI n T - Scroll down by n lines, scrolled out lines are lost

                    vt::csier.table[DECSTBM] = VT_PROC{ p->scr( q   ); };  // CSI r; b r - Set scrolling region (t/b: top+bottom)

                    vt::intro[ctrl::ESC]['M']= VT_PROC{ p->ri(); }; // ESC M  Reverse index
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
            void  nil()                         { caret->nil(spare); }
            void  sav()                         { caret->sav(spare); }
            void  rfg()                         { caret->rfg(spare); }
            void  rbg()                         { caret->rbg(spare); }
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
            // wall: CSI n T  Scroll down, scrolled lines are lost
            void sd(iota n)
            {
                //todo implement
                log("SD - Scroll down \\e[nT is not implemented");
                auto top = scroll_top ? scroll_top - 1
                                      : 0;
                auto end = scroll_bottom ? scroll_bottom - 1
                                         : viewport_height - 1;
                auto count = end - top + 1;
                n = std::clamp(n, 0, count);
                if (n)
                {
                    // Move down by n all below the current
                    // one by one from the bottom

                    // Clear n first lines
                    
                    // Rebuild overlaps from bottom
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
                if (coord.y != scroll_top) coord.y--;
                else                       sd(1);
            }
            // wall: CSI t;b r - Set scrolling region (t/b: top+bottom)
            void scr(fifo& queue)
            {
                scroll_top    = queue(0);
                scroll_bottom = queue(0);
            }
            // wall: CSI n M Delete n lines. Place caret to the begining of the current.
            void dl(iota n)
            {
                //todo implement
                log("DL - \\e[M is not implemented");


                coord.x = 0;
            }
            // wall: CSI n @  Insert n blanks after cursor. Don't change cursor pos
            void ich(iota n)
            {
                /*
                *   Inserts n blanks.
                *   Don't change cursor pos.
                *   Existing chars after cursor shifts to the right.
                */
                if (n > 0)
                {
                    finalize();
                    auto size   = rods::caret->length();
                    auto pos    = rods::caret->chx();
                    auto brush  = rods::caret->brush;
                    brush.txt(whitespace);
                    //todo unify
                    if (pos < size)
                    {
                        // Move existing chars to right (backward decrement)
                        auto& lyric =*rods::caret->lyric;
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
                        auto& lyric =*rods::caret->lyric;
                        lyric.crop(pos + n);
                        // Fill blanks
                        auto dst = lyric.data() + size;
                        auto end = lyric.data() + pos + n;
                        while (dst != end)
                        {
                            *dst++ = brush;
                        }
                    }
                    caret->chx(pos);
                    finalize();
                }
            }
            // wall: CSI n X  Erase/put n chars after cursor. Don't change cursor pos
            void ech(iota n)
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
                finalize();
                auto& lyric =*rods::caret->lyric;
                auto size   = rods::caret->length();
                auto caret  = rods::caret->chx();
                auto brush  = rods::caret->brush;
                brush.txt(whitespace);

                //todo unify for size.y > 1
                //todo revise shifting to the left (right? RTL?)
                if (n > 0)
                {
                    if (caret < size)
                    {
                        //todo unify all (use para methods like para::ins() etc)
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
                            auto dst = lyric.data() + caret;
                            auto end = dst + std::min(n, size - (caret + n));
                            auto src = dst + n;
                            while (dst != end)
                            {
                                *dst++ = *src++;
                            }
                            end = lyric.data() + size;
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
                if (batch[current_para].wrapln)
                {
                    // Check the temp caret position (deffered wrap)
                    if (coord.x && (coord.x % width == 0))
                    {
                        coord.x -= width;
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
                coord.y += n;
                set_coord();
            }
            // wall: '\r'  Go to home of visible line instead of home of para
            void home()
            {
                finalize();
                //coord.x = 0;
                if (batch[current_para].wrapln)
                {
                    coord.x = 0;
                }
                else
                {
                    coord.x -= coord.x % width;
                }

                set_coord();
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
                auto current_brush = caret->brush;
                switch (n)
                {
                    case commands::erase::display::below: // n = 0  Erase viewport after caret (default).
                    {
                        // auto cur_line_it = batch.begin() + current_para;
                        // auto master_id = cur_line_it->master;
                        // // Cut all lines above and current line
                        // auto mas_index = get_line_index_by_id(master_id);
                        // auto head = batch.begin() + mas_index;
                        // auto tail = cur_line_it;
                        // do
                        // {
                        //     auto required_len = (current_para - mas_index) * width + coord.x; // todo optimize
                        //     auto& line = *head;
                        //     line.trim_to(required_len);
                        //     mas_index++;
                        // }
                        // while(head++ != tail);
                        cut_above();
                        // Remove all lines below
                        //todo unify batch.resize(current_para); // no default ctor for line
                        auto erase_count = count - (current_para + 1);
                        parid -= erase_count;
                        count = current_para + 1;
                        while(erase_count--)
                        {
                            batch.pop_back();
                        }
                        break;
                    }
                    case commands::erase::display::above: // n = 1  Erase viewport before caret.
                    {
                        // Insert spaces on all lines above including the current line,
                        //   begining from master of viewport top line
                        //   ending the current line
                        auto master_id = batch[basis].master;
                        auto mas_index = get_line_index_by_id(master_id);
                        auto head = batch.begin() + mas_index;
                        auto tail = batch.begin() + current_para;
                        auto count = coord.y * width + coord.x;
                        auto start = (basis - mas_index) * width;
                        //todo unify
                        do
                        {
                            auto& lyric = *(*head).stanza;
                            lyric.ins(start, count, spare);
                            lyric.trim(spare);
                            start -= width;
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
                caret->brush = current_brush;
                //todo preserve other attributes: wrp, jet
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
                        //if (batch[current_para].wrapln)
                        {
                            auto start = coor;
                            auto end   = (coor + width) / width * width;
                            caret->ins(end - start);
                            caret->cook();
                            caret->chx(coor);
                            caret->trim(spare);
                            //finalize();
                        }
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
                //todo recalc overlapping master id's over selfid's on resize

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

        //bool caret_is_visible=true;
        void input_hndl(view shadow)
        {
            while (ptycon)
            {
                e2::try_sync guard;
                if (guard)
                {
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