// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_TERMINAL_H
#define NETXS_TERMINAL_H

#include "../ui/controls.h"
#include "../os/system.h"

namespace netxs::ui
{
    class lane
        : public page,
          public flow
    {
        //twod width; // post: Lane dimension.
        iota height = 0; // post: Lane dimension.
        iota const& width; // post: Lane dimension.

        cell brush;     // post: Current brush.
        id_t piece = 1; // post: The nearest to top paragraph.
        //todo revise
        bool decoy = true;   // post: Is the cursor inside the viewport?

        twod& anker;     // post: The position of the nearest visible paragraph.
        side& oversize;

        page_layout layout;

    public:
        twod current_coord;
        //bool caret_show = faux;
        //bool caret = faux;   // post: Cursor visibility.

        lane(twod& anker, side& oversize, twod const& viewport_size)
            //: flow    { width },
            : flow    { viewport_size.x, height },
              width   { viewport_size.x },
              anker   { anker },
              oversize{ oversize }
        { }

        // post: Reflow text page on canvas and hold the position
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
        // post: Print page.
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

    class term
        : public base
    {
        using self = term;

        #ifndef DEMO
        FEATURE(pro::keybd, keybd); // term: Keyboard controller
        #endif
        FEATURE(pro::caret, caret); // term: Caret controller
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

        struct wall
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
                    base::csier.table[CSI_CUP] = VT_PROC{ p->cup( q   ); };  // CSI n ; m H
                    base::csier.table[CSI_HVP] = VT_PROC{ p->cup( q   ); };  // CSI n ; m f

                    base::csier.table[CSI_DCH] = VT_PROC{ p->del( q(1)); };  // CSI n P - del n chars
                    base::csier.table[CSI_ECH] = VT_PROC{ p->ins( q(1)); };  // CSI n X - erase n chars
                    base::csier.table[CSI__ED] = VT_PROC{ p->ed ( q(0)); };  // CSI Ps J
                    base::csier.table[CSI__EL] = VT_PROC{ p->el ( q(0)); };  // CSI Ps K

                    base::intro[ctrl::BS ]     = VT_PROC{ p->cuf(-q.pop_all(ctrl::BS )); };
                    base::intro[ctrl::DEL]     = VT_PROC{ p->bsp( q.pop_all(ctrl::DEL)); };
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

            //twod wallsize; // term: Scrollback size =flow::boundary::minmax()

            term& boss;
            cell& spare; // wall: Shared current brush.

            wall(term& boss)
                : lane  ( boss.base::anchor, boss.base::oversize, boss.viewport.size ),
                  spare { boss.base::color() },
                  //spare { boss.term::spare },
                  boss  { boss }

            { }

            void nil() { page::layer->nil(spare); }
            void sav() { page::layer->sav(spare); }
            void rfg() { page::layer->rfg(spare); }
            void rbg() { page::layer->rbg(spare); }

            void ins(iota n) // wall: Erase letters after caret. CSI n X
            {
                /*
                erase:
                    Put n chars after cursor. Don't change cursor pos
                */

                if (n > 0)
                {
                    page::cook();
                    auto pos = page::layer->chx();
                    page::layer->ins(n);
                    page::cook();
                    page::layer->chx(pos);
                }

            }
            void del(iota n) // wall: Delete (not Erase) letters under the caret. CSI n P
            {
                /*
                * del:
                As characters are deleted, the remaining characters
                between the cursor and right margin move to the left.
                Character attributes move with the characters.
                The terminal adds blank characters at the right margin.
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
                    //todo negative n support
                }
            }
            void bsp(iota n)
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
            void chx(iota n)
            {
                if (n-- > 0) // 1-based coord
                {
                    //page::chx(n);
                    page::layer->chx(n);
                    //log(" CHX: ", n);
                }
            }
            void chy(iota n)
            {
                if (n > 0) // 1-based coord
                {
                    page::cook();
                    page::layer->trim(spare);

                    auto height = boss.viewport.size.y;
                    auto width  = boss.viewport.size.x;
                    auto current_brush = page::layer->brush;

                    auto x = page::layer->chx() % width;

                    n = std::min(n, height);
                    page::layer = std::prev(page::batch.end());
                    page::cook();
                    page::layer->trim(spare);

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
            void cup(fifo& queue) // 1-based coords
            {
                auto p = twod(queue(1), queue(1));
                //chx(p.x);
                //chy(p.y);

                p = std::clamp(p, dot_11, boss.viewport.size);

                auto x = p.x - 1;
                auto y = p.y - 1;

                page::cook();
                page::layer->trim(spare);

                auto current_brush = page::layer->brush;
                auto width  = boss.viewport.size.x;
                auto height = boss.viewport.size.y;

                page::layer = std::prev(page::batch.end());
                page::cook();
                page::layer->trim(spare);

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

            // page: Line feed up
            template<bool PRESERVE_BRUSH = true>
            void  up(iota n)
            {
                if (n > 0)
                {
                    page::cook();
                    page::layer->trim(spare);
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
            // term: append empty lines to the page
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
                        page::layer->trim(spare);
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
                    page::layer = current_layer;
                }
            }

            // page: Line feed down
            template<bool PRESERVE_BRUSH = true>
            void  dn(iota n)
            {
                if (n > 0)
                {
                    page::cook();
                    page::layer->trim(spare);

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

                    //auto item = static_cast<iota>(
                    //	std::distance(layer, batch.end()));
                    //
                    //if (n < item)
                    //{
                    //	while (n--)
                    //	{
                    //		page::layer++;
                    //	}
                    //}
                    //else
                    //{
                    //	auto dist = item;
                    //	while (--dist)
                    //	{
                    //		page::layer++;
                    //		//page::layer->brush = current_brush;
                    //	}
                    //	
                    //	//if (PRESERVE_BRUSH) page::layer->brush = current_brush;
                    //	page::layer->brush = current_brush;
                    //	adds(n - item + 1);
                    //	//while (item++ <= n)
                    //	//{
                    //	//	page::fork();
                    //	//	page::layer->locus.push({ ansi::fn::nl, 1 });
                    //	//}
                    //
                    //}
                    page::layer->chx(c);
                    page::layer->brush = current_brush;
                }
            }

            void home()
            {
                //todo ctrl::CR(13) should go to home of visible line instead of home of para
                page::cook();

                auto& width = boss.viewport.size.x;
                auto c = page::layer->chx();
                //if (flow::wrapln)
                //{
                //	page::layer->chx(0);
                //}
                //else
                {
                    //log(" home: ", c, " width: ", width);
                    // if (c && !(c % width))
                    // //if (c && (!(c % width) || !((c-1) % width)))
                    // {
                    // 	--c;
                    // 	log(" home decreased: ", c);
                    // }
                
                    c = c - c % width;
                    //log(" home new value: ", c);
                    page::layer->chx(c);
                }

                //page::layer->chx(0);

                //log(" home: coor ", c);
            }

            // page: Carriage return + Line feed
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
            // CSI n J  Erase display
            void   ed(iota n)
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
            // CSI n K  Erase line (don't move caret)
            void   el(iota n)
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
                        auto mark = page::layer->brush;
                        mark.txt(' ');
                        auto coor = page::layer->chx();
                        auto width = boss.viewport.size.x;
                        //auto sss = width - coor;
                        lyric.crop({ coor,1 }, mark);
                        lyric.crop({ coor + (width - coor % width),1 }, mark);

                        page::layer->trim(spare);


                        // ConPTY doesn't move caret (is it ok?)
                        //page::layer->chx(coor + (width - coor % width));
                        break;
                    }
                    case commands::erase::line::left: // Ps = 1  ⇒  Erase to Left.
                    {
                        //todo implement
                        auto mark = page::layer->brush;
                        mark.txt(' ');
                        auto coor = page::layer->chx();
                        auto width = boss.viewport.size.x;
                        //auto sss = width - coor;
                        if (coor < width)
                        {
                            lyric.each([&](cell& c) {if (coor > 0) { coor--; c.fuse(mark); } });
                        }
                        else
                        {
                            auto left_edge = coor - coor % width;

                            lyric.crop({ left_edge,1 }, mark);
                            lyric.crop({ left_edge + width,1 }, mark);
                        }
                        //page::task(ansi::rule{ ansi::fn::el, 1 });
                        break;
                    }
                    case commands::erase::line::all: // Ps = 2  ⇒  Erase All.
                    {
                        //todo optimize
                        auto coor = page::layer->chx();
                        auto mark = page::layer->brush;
                        mark.txt(' ');
                        auto width = boss.viewport.size.x;
                        //auto sss = width - coor;

                        auto left_edge = coor - coor % width;

                        lyric.crop({ left_edge,1 }, mark);
                        lyric.crop({ left_edge + width,1 }, mark);

                        //lyric.crop({ 0,1 }, mark);
                        //lyric.crop({ width,1 }, mark);

                        //page::task(ansi::rule{ ansi::fn::el, 2 });
                        break;
                    }
                }
            }
            //void crop_to_viewport(twod limits)
            //{
            //	auto tail = page::batch.rbegin();
            //	auto size = std::min(limits.y, page::size());
            //	while (size--)
            //	{
            //		auto& layer = *tail++;
            //		if (layer.size().x > limits.x)
            //		{
            //			layer.lyric->crop(limits.x);
            //		}
            //	}
            //}
            // Remove all empty lines at the end
            //void refresh()
            //{
            //	//auto l = std::prev(page::batch.end());
            //	//while (l != page::layer && l != page::batch.begin())
            //	//{
            //	//	page::layer->trim(page::spare);
            //	//	if (!(l->length()))
            //	//	{
            //	//		--l;
            //	//		//page::batch.pop_back();
            //	//		//l = std::prev(page::batch.end());
            //	//	}
            //	//	else break;
            //	//}
            //}
        };

        wall     scroll{ *this };
        wall     altbuf{ *this };
        wall*    target{ &scroll };
        os::cons ptycon;

    //public:
    //	cell spare;
    //private:

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
            //altbuf.clear();
            //scroll.clear();
            altbuf.ed(commands::erase::display::scrollback);
            scroll.ed(commands::erase::display::scrollback);
            target = &scroll;
        }
        void decset(wall*& p, fifo& queue)
        {
            while (auto q = queue(0))
            {
                switch (q)
                {
                    case 1: // Cursor keys application mode.
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
                        //canvas.mark()
                        //	.bgc(blackdk)
                        //	.fgc(whitelt)
                        //	.alpha(skin::shady());
                        //canvas.wipe();
                        
                        // canvas.wipe();
                        // canvas.moved = faux;
                        // canvas.origin(dot_00);

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
                    case 1: // Cursor keys ANI mode.
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
                //log ("old viewport.size.y: ", viewport.size.y);
                if (viewport.size != new_size)
                {
                    //#if defined(_WIN32)
                    //
                    //	//todo ConPTY bug: 
                    //	//     ConPTY does not emit anything to vanish right side on down sizing
                    //	//     cropping all visible line in order to match to the viewport width
                    //	if (viewport.size.x > new_size.x)
                    //	{
                    //		//target->crop_to_viewport(new_size);
                    //	}
                    //
                    //#endif

                    if (auto news = new_size.y - viewport.size.y)
                    {
                        //todo calc visible lines TIA wrapping and lyric.size

                        //target->adds(news);
                        altbuf.page::maxlen(new_size.y);
                        if (new_size.y > altbuf.size())
                            altbuf.adds(new_size.y - altbuf.size());

                        if (new_size.y != altbuf.size())
                            throw;

                        scroll.adds(news);
                    }

                    viewport.size = new_size;
                    if (target == &altbuf)
                    {
                        //altbuf.ed(commands::erase::display::viewport);
                        altbuf.ed(commands::erase::display::scrollback);
                    }
                    //log("altbuf size: ", altbuf.size());
                    log("scroll size: ", scroll.size());
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
                    SIGNAL(e2::general, e2::debug::output, shadow);

                    ansi::parse(shadow, target); // Append using default insertion point

                    // Follow to the caret
                    //todo revise
                    auto old_caret_pos = caret.coor();
                    auto caret_is_visible = viewport.hittest(old_caret_pos);

                    //if (target == &altbuf)
                    //{
                    //	altbuf.hz_trim(viewport.size.x);
                    //}

                    auto new_size = target->reflow();
                    auto caret_xy = target->cp();
                    caret.coor(caret_xy);

                    // Follow to the caret
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
            //parent_canvas.reflow(scroll);
            // parent_canvas.rst();
            // parent_canvas.output(scroll, true);
            target->output(parent_canvas);

            SIGNAL(e2::release, e2::form::upon::redrawn, parent_canvas);

            // if (base::status.invalid)
            // {
            // 	//redraw();
            // 	if (target == &altbuf)
            // 	{
            // 		//todo unify
            // 		//boss.caret.away(canvas);
            // 
            // 		canvas.output(altbuf);
            // 		altbuf.clear(true);
            // 	}
            // 	else
            // 	{
            // 		canvas.reflow(scroll);
            // 		canvas.output(scroll, true);
            // 	}
            // 
            // 	// in order to show cursor/caret
            // 	SIGNAL(e2::release, e2::form::upon::redrawn, canvas);
            // 	base::status.invalid = faux;
            // }
            // parent_canvas.plot(canvas);

            //if (target == &altbuf)
            //{
            //	//todo unify
            //	//boss.caret.away(canvas);
            //
            //	canvas.output(altbuf);
            //	//altbuf.clear(true);
            //}
            //else
            //{
            //	canvas.reflow(scroll);
            //	canvas.output(scroll, true);
            //}
            //
            //// in order to show cursor/caret
            //SIGNAL(e2::release, e2::form::upon::redrawn, canvas);
            //base::status.invalid = faux;
        }
        // virtual void redraw()
        // {
        // 	//terminal.render();
        // 	if (target == &altbuf)
        // 	{
        // 		//todo unify
        // 		//boss.caret.away(canvas);
        // 
        // 		canvas.output(altbuf);
        // 		altbuf.clear(true);
        // 	}
        // 	else
        // 	{
        // 		canvas.reflow(scroll);
        // 		canvas.output(scroll, true);
        // 	}
        // 
        // 	// in order to show cursor/caret
        // 	SIGNAL(e2::release, e2::form::upon::redrawn, canvas);
        // 	base::status.invalid = faux;
        // }
    };
}

#endif // NETXS_TERMINAL_H