// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_RICHTEXT_H
#define NETXS_RICHTEXT_H

#include <cstring>  // std::memcpy

#include "ansi.h"
#include "../ui/events.h"
#include "../text/logger.h"

namespace netxs::console
{
    using namespace std::literals;
    using namespace netxs::ui;

    using utf::text;
    using utf::view;
    using ansi::qiew;
    using ansi::writ;
    using id_t = bell::id_t;
    using grid = std::vector<class cell>;
    using irgb = netxs::ui::irgb<uint32_t>;

    class cell // richtext: Enriched grapheme cluster.
    {
    public:
        using bitstate = unsigned char;

    private:
        template<class V = void> // Use template in order to define statics in the header file.
        union glyf
        {
            struct mode
            {
                unsigned char count : CLUSTER_FIELD_SIZE; // grapheme cluster length (utf-8 encoded) (max GRAPHEME_CLUSTER_LIMIT)
                //todo unify with CFA https://gitlab.freedesktop.org/terminal-wg/specifications/-/issues/23
                unsigned char width : WCWIDTH_FIELD_SIZE; // 0: non-printing, 1: narrow, 2: wide:left_part, 3: wide:right_part  // 2: wide, 3: three-cell width
                unsigned char jumbo : 1;                  // grapheme cluster length overflow bit
            };

            // there is no need to reset/clear/flush the map because 
            // count of different grapheme clusters is finite
            static constexpr size_t         limit = sizeof(uint64_t);
            static std::hash<view>          coder;
            static text                     empty;
            static std::map<uint64_t, text> jumbo;

            uint64_t                        token;
            mode                            state;
            char                            glyph[limit];

            constexpr glyf()
                : token(0)
            { }

            constexpr glyf(glyf const& c)
                : token(c.token)
            { }

            glyf (char c)
                : token(0)
            {
                set(c);
            }

            glyf (glyf const& c, view const& utf8, size_t width)
                : token(c.token)
            {
                set(utf8, width);
            }

            bool operator == (glyf const& c) const
            {
                return token == c.token;
            }
            
            // Check the grapheme clusters are the same.
            bool same(glyf const& c) const
            {
                //auto mask = ~(decltype(token))0xFF;
                //return (token >> sizeof(mode)) == c.token >> sizeof(mode);
                return (token >> 8) == (c.token >> 8);
            }

            void wipe()
            {
                token = 0;
            }

            /*
            *   Width property
            *       W   Wide                    ┌-------------------------------┐
            *       Na  Narrow                  |   Narrow      ┌-------------------------------┐
            *       F   Fullwidth, Em wide      |┌-------------┐|               |   Wide        |
            *       H   Halfwidth, 1/2 Em wide  ||  Halfwidth  ||   Ambiguous	|┌-------------┐|
            *       A   Ambiguous               |└-------------┘|               ||  Fullwidth  ||
            *       N   Neutral =Not East Asian └---------------|---------------┘└-------------┘|
            *                                                   └-------------------------------┘
            *   This width takes on either of 𝐭𝐰𝐨 𝐯𝐚𝐥𝐮𝐞𝐬: 𝐧𝐚𝐫𝐫𝐨𝐰 or 𝐰𝐢𝐝𝐞. (UAX TR11)
            *   For any given operation, these six default property values resolve into
            *   only two property values, narrow and wide, depending on context.
            *
            *   width := {0 - nonprintable | 1 - Halfwidth(Narrow) | 2 - Fullwidth(Wide) }
            *
            *   ! Unicode Variation Selector 16 (U+FE0F) makes the character it combines with double-width.
            *
            *   The 0xfe0f character is "variation selector 16" that says "show the emoji version of
            *   the previous character" and 0xfe0e is "variation selector 15" to say "show the non-emoji
            *   version of the previous character"
            */

            void set(char c)
            {
                token       = 0;
                state.width = 1;
                state.count = 1;
                glyph[1]    = c;
            }
            void set(view const& utf8, size_t cwidth)
            {
                auto count = utf8.size();
                if (count >= limit)
                {
                    token = coder(utf8);
                    state.jumbo = true;
                    state.width = cwidth;
                    jumbo.insert(std::pair{ token, utf8 }); // silently ignore if it exists
                }
                else
                {
                    token = 0;
                    state.count = count;
                    state.width = cwidth;
                    std::memcpy(glyph + 1, utf8.data(), count);
                }
            }
            void set(view const& utf8)
            {
                auto cluster = utf::letter(utf8);
                set(cluster.text, cluster.attr.wcwidth);
            }
            view get() const
            {
                if (state.jumbo)
                {
                    return netxs::get_or(jumbo, token, empty);
                }
                else
                {
                    return view{ glyph + 1, state.count };
                }
            }
            void rst()
            {
                set(whitespace);
            }
        };
        union body
        {
            // there are no applicable rich text formatting attributes due to their gradual nature
            // e.g.: the degree of thickness or italiciety/oblique varies from 0 to 255, etc.,
            // and should not be represented as a flag
            //
            // In Chinese, the underline/underscore is a punctuation mark for proper names 
            // and should never be used for emphasis
            //
            // weigth := 0..255
            // italic := 0..255
            //

            uint32_t token;

            struct
            {
                union
                {
                    bitstate token;
                    struct
                    {
                        bitstate bolded : 1;
                        bitstate italic : 1;
                        bitstate unline : 1;
                        bitstate invert : 1;
                        bitstate r_to_l : 1;
                    } var;
                } shared;

                union
                {
                    bitstate token;
                    struct
                    {
                        bitstate hyphen : 1;
                        bitstate fnappl : 1;
                        bitstate itimes : 1;
                        bitstate isepar : 1;
                        bitstate inplus : 1;
                        bitstate zwnbsp : 1;
                        //todo use these bits as a underline variator
                        bitstate render : 2; // reserved
                    } var;

                } unique;
            }
            param;

            constexpr body () 
                : token(0)
            { }

            constexpr body (body const& b)
                : token(b.token)
            { }

            bool operator == (body const& b) const
            {
                return token == b.token;

                sizeof(*this);
                sizeof(param.shared.var);
                sizeof(param.unique.var);
            }
            bool operator != (body const& b) const
            {
                return !operator == (b);
            }
            bool like(body const& b) const
            {
                return param.shared.token == b.param.shared.token;
            }
            void get(body& base, ansi::esc& dest)	const
            {
                if (!like(base))
                {
                    auto& cvar =      param.shared.var;
                    auto& bvar = base.param.shared.var;
                    if (cvar.bolded != bvar.bolded)
                    {
                        dest.bld(cvar.bolded);
                    }
                    if (cvar.italic != bvar.italic)
                    {
                        dest.itc(cvar.italic);
                    }
                    if (cvar.unline != bvar.unline)
                    {
                        dest.und(cvar.unline);
                    }
                    if (cvar.invert != bvar.invert)
                    {
                        dest.inv(cvar.invert);
                    }
                    if (cvar.r_to_l != bvar.r_to_l)
                    {
                        //todo implement RTL
                    }

                    bvar = cvar;
                }
            }
            void wipe()
            {
                token = 0;
            }

            void bld (bool b) { param.shared.var.bolded = b; }
            void itc (bool b) { param.shared.var.italic = b; }
            void und (bool b) { param.shared.var.unline = b; }
            void inv (bool b) { param.shared.var.invert = b; }
            void rtl (bool b) { param.shared.var.r_to_l = b; }
            void vis (iota l) { param.unique.var.render = l; }

            bool bld () const { return param.shared.var.bolded; }
            bool itc () const { return param.shared.var.italic; }
            bool und () const { return param.shared.var.unline; }
            bool inv () const { return param.shared.var.invert; }
            bool rtl () const { return param.shared.var.r_to_l; }
            iota vis () const { return param.unique.var.render; }
        };
        struct clrs
        {
            // Concept of using default colors:
            //  if alpha is set to zero, then underlaid color should be used

            rgba bg;
            rgba fg;

            constexpr clrs ()
                : bg{}, fg{}
            { }

            constexpr clrs (clrs const& c)
                : bg{ c.bg }, fg{ c.fg }
            { }

            bool operator == (clrs const& c) const
            {
                return bg == c.bg && fg == c.fg;

                sizeof(*this);
            }
            bool operator != (clrs const& c) const
            {
                return !operator == (c);
            }

            void get(clrs& base, ansi::esc& dest)	const
            {
                if (bg != base.bg)
                {
                    base.bg = bg;
                    dest.bgc(bg);
                }
                if (fg != base.fg)
                {
                    base.fg = fg;
                    dest.fgc(fg);
                }
            }
            void wipe()
            {
                bg.wipe();
                fg.wipe();
            }
        };

        clrs       uv;     // 8U, cell: RGBA color
        glyf<void> gc;     // 8U, cell: Grapheme cluster
        body       st;     // 4U, cell: Style attributes
        id_t       id = 0; // 4U, cell: Link ID
        //id_t       pg = 0; // 4U, cell: Paragraph ID
        id_t       rsrvd0; // 4U, cell: pad, the size should be a power of 2
        id_t       rsrvd1; // 4U, cell: pad, the size should be a power of 2

    public:
        cell() = default;

        cell(char c)
            : gc{ c }
        { 
            sizeof(glyf<void>);
            sizeof(clrs);
            sizeof(body);
            sizeof(id_t);
            sizeof(id_t);

            sizeof(cell);
        }

        cell(view chr)
        { 
            gc.set(chr);
        }

        cell(cell const& base)
            : uv{ base.uv },
              gc{ base.gc },
              st{ base.st },
              id{ base.id }
              //pg{ base.pg }
        { }

        cell(cell const& base, view const& cluster, size_t wcwidth)
            : uv{ base.uv },
              st{ base.st },
              id{ base.id },
              //pg{ base.pg },
              gc{ base.gc, cluster, wcwidth }
        { }

        cell(cell const& base, char c)
            : uv{ base.uv },
              st{ base.st },
              id{ base.id },
              //pg{ base.pg },
              gc{ c }
        { }

        bool operator == (cell const& c) const
        {
            return uv == c.uv
                && st == c.st
                && gc == c.gc;
        }
        bool operator != (cell const& c) const
        {
            return !operator == (c);
        }
        auto& operator = (cell const& c)
        {
            uv = c.uv;
            gc = c.gc;
            st = c.st;
            id = c.id;
            //pg = c.pg;
            return *this;
        }

        operator bool() const { return wdt(); } // cell: Is the cell not transparent?

        bool like(cell const& c) const // cell: Precise comparisons of the two cells.
        {
            return uv == c.uv
                && st.like(c.st);
        }
        void wipe() // cell: Set colors, attributes and grapheme cluster to zero.
        {
            uv.wipe();
            gc.wipe();
            st.wipe();
        }
        cell const&	data() const{ return *this;} // cell: Return the const reference of the base cell.

        // cell: Merge the two cells according to visibility and other attributes.
        inline void fuse(cell const& c)
        {
            //if (c.uv.fg.chan.a) uv.fg = c.uv.fg;
            ////uv.param.fg.mix(c.uv.param.fg);
            
            if (uv.fg.chan.a == 0xFF) uv.fg.mix_one(c.uv.fg);
            else                      uv.fg.mix(c.uv.fg);

            if (uv.bg.chan.a == 0xFF) uv.bg.mix_one(c.uv.bg);
            else                      uv.bg.mix(c.uv.bg);

            st = c.st;
            if (c.wdt()) gc = c.gc;
        }
        // cell: Mix colors using alpha and copy grapheme cluster if it's exist.
        //void mix_colors	(cell const& c)
        //{
        //	uv.param.fg.mix_alpha(c.fgc());
        //	uv.param.bg.mix_alpha(c.bgc());
        //	if (c.wdt())
        //	{
        //		gc = c.gc;
        //	}
        //}
        // cell: Merge the two cells and update ID with COOR.
        void fuse(cell const& c, id_t oid)//, twod const& pos)
        {
            fuse(c);
            id = oid;
        }
        // cell: Merge the two cells and update ID with COOR.
        void fusefull(cell const& c)
        {
            fuse(c);
            if (c.id) id = c.id;
            //pg = c.pg;

            //mark paragraphs
            //if (c.pg) uv.param.bg.channel.blue = 0xff;
        }
        void meta(cell const& c)
        {
            uv = c.uv;
            st = c.st;
        }
        // cell: Get differences of the visual attributes only (ANSI CSI/SGR format).
        void scan_attr(cell& base, ansi::esc& dest) const
        {
            if (!like(base))
            {
                //todo additionally consider UNIQUE ATTRIBUTES
                uv.get(base.uv, dest);
                st.get(base.st, dest);
            }
        }
        // cell: Get differences (ANSI CSI/SGR format) of "base" and add it to "dest" and update the "base".
        void scan(cell& base, ansi::esc& dest) const
        {
            if (!like(base))
            {
                //todo additionally consider UNIQUE ATTRIBUTES
                uv.get(base.uv, dest);
                st.get(base.st, dest);
            }

            if (wdt()) dest += gc.get();
            else       dest += whitespace;
        }
        // cell: !!! Ensure that this.wdt == 2 and the next wdt == 3 and they are the same.
        bool scan(cell& next, cell& base, ansi::esc& dest) const
        {
            if (gc.same(next.gc) && like(next))
            {
                if (!like(base))
                {
                    //todo additionally consider UNIQUE ATTRIBUTES
                    uv.get(base.uv, dest);
                    st.get(base.st, dest);
                }
                dest += gc.get();
                return true;
            }
            else
            {
                return faux;
            }
        }
        // cell: Is the cell not transparent?
        //bool is_unalterable() const
        //{
        //	return vis() == unalterable;
        //}
        // cell: Delight both foreground and background.
        void xlight()
        {
            uv.fg.xlight();
            uv.bg.xlight();
        }
        // cell: Darken both foreground and background.
        void shadow(uint8_t fk, uint8_t bk) //void shadow(uint8_t k = 24)
        {
            uv.fg.shadow(fk);
            uv.bg.shadow(bk);
        }
        //todo xlight conflict
        // cell: Lighten both foreground and background.
        void bright(uint8_t fk, uint8_t bk) //void bright(uint8_t k = 24)
        {
            uv.fg.bright(fk);
            uv.bg.bright(bk);
        }
        // cell: Is the cell not transparent?
        bool is_alpha_blendable() const
        {
            return uv.bg.is_alpha_blendable();//&& uv.param.fg.is_alpha_blendable();
        }
        // cell: Set Grapheme cluster and its width.
        void set_gc (view c, size_t w) { gc.set(c, w); }
        // cell: Set Grapheme cluster.
        void set_gc (cell const& c) { gc = c.gc; }
        // cell: Reset Grapheme cluster.
        void set_gc () { gc.wipe(); }

        // cell: Copy view of the cell (Preserve ID).
        cell& set (cell const& c) { uv = c.uv;
                                    st = c.st;
                                    gc = c.gc;       return *this; }
        cell& alpha (uint8_t k)   { bga(k); fga(k);  return *this; } // cell: Set alpha/transparency (background and foreground).
        cell& bgc (rgba const& c) { uv.bg = c; return *this; } // cell: Set Background color.
        cell& fgc (rgba const& c) { uv.fg = c; return *this; } // cell: Set Foreground color.
        cell& bga (uint8_t k)     { uv.bg.chan.a = k; return *this; } // cell: Set Background alpha/transparency.
        cell& fga (uint8_t k)     { uv.fg.chan.a = k; return *this; } // cell: Set Foreground alpha/transparency.
        cell& bld (bool b)        { st.bld(b); return *this; } // cell: Set Bold attribute.
        cell& itc (bool b)        { st.itc(b); return *this; } // cell: Set Italic attribute.
        cell& und (bool b)        { st.und(b); return *this; } // cell: Set Underline/Underscore attribute.
        cell& inv (bool b)        { st.inv(b); return *this; } // cell: Set Invert attribute.
        cell& rtl (bool b)        { st.rtl(b); return *this; } // cell: Set Right-To-Left attribute.
        cell& link(id_t oid)      { id = oid;  return *this; } // cell: Set link object ID.
        //cell& para(id_t opg)      { pg = opg;  return *this; } // cell: Set paragraph ID and return the cell itself.
        cell& txt (view c)        { c.size() ? gc.set(c) : gc.wipe(); return *this; } // cell: Set Grapheme cluster.
        cell& txt (char c)        { gc.set(c); return *this; } // cell: Set Grapheme cluster from char.
        cell& clr (cell const& c) { uv = c.uv; return *this; } // cell: Set the foreground and background colors only.
        cell& wdt (iota w)        { gc.state.width = w; return *this; } // cell: Return Grapheme cluster screen width.
        cell& rst () // cell: Reset view attributes of the cell to zero.
        {
            static cell empty{ whitespace };
            uv = empty.uv;
            st = empty.st;
            gc = empty.gc;
            return *this;
        }

        void hyphen (bool b) { st.param.unique.var.hyphen = b; } // cell: Set the presence of the SOFT HYPHEN (U+00AD).
        void fnappl (bool b) { st.param.unique.var.fnappl = b; } // cell: Set the presence of the FUNCTION APPLICATION (U+2061).
        void itimes (bool b) { st.param.unique.var.itimes = b; } // cell: Set the presence of the INVISIBLE TIMES (U+2062).
        void isepar (bool b) { st.param.unique.var.isepar = b; } // cell: Set the presence of the INVISIBLE SEPARATOR (U+2063).
        void inplus (bool b) { st.param.unique.var.inplus = b; } // cell: Set the presence of the INVISIBLE PLUS (U+2064).
        void zwnbsp (bool b) { st.param.unique.var.zwnbsp = b; } // cell: Set the presence of the ZERO WIDTH NO-BREAK SPACE (U+FEFF).
        
        uint8_t     bga () const { return uv.bg.chan.a;  } // cell: Return Background alpha/transparency.
        uint8_t     fga () const { return uv.fg.chan.a;  } // cell: Return Foreground alpha/transparency.
        rgba&       bgc ()       { return uv.bg;         } // cell: Return Background color.
        rgba&       fgc ()       { return uv.fg;         } // cell: Return Foreground color.
        rgba const& bgc () const { return uv.bg;         } // cell: Return Background color.
        rgba const& fgc () const { return uv.fg;         } // cell: Return Foreground color.
        bool        bld () const { return st.bld();      } // cell: Return Bold attribute.
        bool        itc () const { return st.itc();      } // cell: Return Italic attribute.
        bool        und () const { return st.und();      } // cell: Return Underline/Underscore attribute.
        bool        inv () const { return st.inv();      } // cell: Return Negative attribute.
        id_t       link () const { return id;            } // cell: Return link object ID.
        //id_t       para () const { return pg;            } // cell: Return paragraph ID.
        view        txt () const { return gc.get();      } // cell: Return Grapheme cluster.
        size_t      len () const { return gc.state.count;} // cell: Return Grapheme cluster utf-8 length.
        size_t      wdt () const { return gc.state.width;} // cell: Return Grapheme cluster screen width.
        bool     iswide () const { return wdt() > 1;     } // cell: Return true if char is wide.
        bool     issame_visual (cell const& c) const // cell: Is the cell visually identical.
        {
            if (gc == c.gc)
            {
                if (uv.bg == c.uv.bg)
                {
                    if (wdt() == 0 || txt().front() == ' ')
                    {
                        return true;
                    }
                    else
                    {
                        return uv.fg == c.uv.fg;
                    }
                }
            }
            else return faux;
        }
    };

    /// Extern link statics
    template<class T> std::hash<view>          cell::glyf<T>::coder;
    template<class T> text                     cell::glyf<T>::empty;
    template<class T> std::map<uint64_t, text> cell::glyf<T>::jumbo;

    class poly
    {
        cell grade[256];

    public:
        poly() = default;

        poly(cell const& basis)
        {
            recalc(basis);
        }

        void recalc(cell const& basis)
        {
            for (auto k = 0; k < 256; k++)
            {
                auto& b = grade[k];

                b = basis;
                b.bga(b.bga() * k >> 8);
                b.fga(b.fga() * k >> 8);
            }
        }

        cell const& operator[](uint8_t k) const
        {
            return grade[k];
        }
    };

    class core // richtext: Canvas core grid.
    {
        // prefill canvas using brush
        core(twod const& coor, twod const& size, cell const& brush)
            : region{ coor, size },
              client{ dot_00, size },
              canvas(size.x * size.y, brush),
              marker{ brush }
        { }
        // prefill canvas by zero
        core(twod const& coor, twod const& size)
            : region{ coor, size },
              client{ dot_00, size },
              canvas(size.x * size.y)
        { }

    protected:
        iota digest = 0;
        cell marker;
        rect region;
        grid canvas;
        rect client;

    public:
        //core(core const& c) = default;
        //core(core&& c) = delete;
        core() { }; // = default;

        constexpr auto& size() const { return region.size;   }
        auto& coor() const { return region.coor;   }
        auto& area() const { return region;        }
        auto  hash()       { return digest;        } // core: Return the digest value that associatated with the current canvas size.
        auto  data() const { return canvas.data(); }
        auto  data()       { return canvas.data(); }
        auto& pick()       { return canvas;        }
        auto  test(twod const& coord) const { return region.size.inside(coord); } // core: Check the coor inside the canvas.
        auto  data(twod const& coord)       { return  data() + coord.x + coord.y * region.size.x; } // core: Return the offset of the cell corresponding to the specified coordinates.
        auto  data(twod const& coord) const { return  data() + coord.x + coord.y * region.size.x; } // core: Return the const offset value of the cell.
        auto& data(size_t offset)           { return*(data() + offset);  } // core: Return the const offset value of the cell corresponding to the specified coordinates.
        auto& operator[] (twod const& coord){ return*(data(coord));      } // core: Return reference of the canvas cell at the specified location. It is dangerous in case of layer resizing.
        auto& mark()                        { return marker;             } // core: Return a reference to the default cell value.
        auto& mark() const                  { return marker;             } // core: Return a reference to the default cell value.
        auto& mark(cell const& c)           { marker = c; return marker; } // core: Set the default cell value.
        void  move(twod const& newcoor)     { region.coor = newcoor;     } // core: Change the location of the face.
        void  step(twod const& delta)       { region.coor += delta;      } // core: Shift location of the face by delta.
        void  link(id_t id)                 { marker.link(id);           } // core: Set the default object ID.
        auto  link(twod const& coord) const { return test(coord) ? (*(data(coord))).link() : 0; } // core: Return ID of the object in cell at the specified coordinates.
        auto  view() const                  { return client; }
        void  view(rect const& viewreg)     { client = viewreg; }
        void  size(twod const& newsize) // core: Change the size of the face.
        {
            if (region.size(newsize))
            {
                client.size = region.size;
                digest++;
                canvas.assign(region.size.x * region.size.y, marker);
            }
        }
        void  crop(iota newsizex) // core: Resize while saving the textline.
        {
            region.size.x = newsizex;
            region.size.y = 1;
            client.size = region.size;
            canvas.resize(newsizex);
            digest++;
        }
        //todo unify
        void  crop(twod const& newsize) // core: Resize while saving the bitmap.
        {
            core block{ region.coor, newsize };
            
            auto full = [](auto& dst, auto& src) { dst = src; };
            netxs::onbody(block, *this, full);
            
            region.size = newsize;
            client.size = region.size;
            swap(block);
            digest++;
        }
        void  crop(twod const& newsize, cell const& c) // core: Resize while saving the bitmap.
        {
            core block{ region.coor, newsize, c };

            auto full = [](auto& dst, auto& src) { dst = src; };
            netxs::onbody(block, *this, full);

            region.size = newsize;
            client.size = region.size;
            swap(block);
            digest++;
        }
        void  kill() // core: Collapse canvas to size zero (see para).
        {
            region.size.x = 0;
            client.size.x = 0;
            canvas.resize(0);
            digest++;
        }
        void  wipe(cell const& c) { std::fill(canvas.begin(), canvas.end(), c); } // core: Fill the canvas with the specified marker.
        void  wipe() { wipe(marker); } // core: Fill the canvas with the default color.
        void  wipe(id_t id)            // core: Fill the canvas with the specified id.
        {
            auto my = marker.link();
            marker.link(id);
            wipe(marker);
            marker.link(my);
        }
        template<class P>
        void  each(P proc) // core: Exec a proc for each cell.
        {
            for (auto& c : canvas)
            {
                proc(c);
            }
        }
        template<class P>
        void  each(rect const& region, P proc) // core: Exec a proc for each cell of the specified region.
        {
            netxs::onrect(*this, region, proc);
        }
        auto  copy(grid& target) const { target = canvas; return region.size; } // core: Copy only grid of the canvas to the specified grid bitmap.
        void  copy(core& target, bool copymetadata = faux) const // core: Copy the canvas to the specified target bitmap. The target bitmap must be the same size.
        {
            auto full = [](auto& dst, auto& src) { dst = src; };
            auto flat = [](auto& dst, auto& src) { dst.set(src); };

            copymetadata ? netxs::oncopy(target, *this, full)
                         : netxs::oncopy(target, *this, flat);

            //todo should we copy all members?
            //target.marker = marker;
            //flow::cursor
        }
        void  plot(core const& block, bool force = faux) // core: Place the specified face using its coordinates.
        {
            auto full = [](auto& dst, auto& src) { dst = src; };
            auto fuse = [](auto& dst, auto& src) { dst.fusefull(src); };

            force ? netxs::onbody(*this, block, full)
                  : netxs::onbody(*this, block, fuse);
        }
        void  fill(core const& block, bool force = faux) // core: Fill by the specified face using its coordinates.
        {
            auto flat = [](auto& dst, auto& src) { dst.set(src); };
            auto fuse = [](auto& dst, auto& src) { dst.fuse(src); };

            force ? netxs::onbody(*this, block, flat)
                  : netxs::onbody(*this, block, fuse);
        }
        void  fill(sptr<core> block_ptr, bool force = faux) // core: Fill by the specified face using its coordinates.
        {
            if (block_ptr) fill(*block_ptr, force);
        }
        void  fill(ui::rect block, cell const& brush, bool force = faux) // core: Fill the specified region with the specified color. If forced == true use direct copy instead of mixing.
        {
            auto flat = [brush](auto& dst) { dst = brush ; };
            auto fuse = [brush](auto& dst) { dst.fusefull(brush); };

            block.coor += region.coor;
            force ? netxs::onrect(*this, block, flat)
                  : netxs::onrect(*this, block, fuse);
        }
        void  fill(cell const& brush, bool force = faux) // core: Fill the client area with the specified color. If forced == true use direct copy instead of mixing.
        {
            fill(view(), brush, force);
        }
        template<class P>
        void  fill(ui::rect block, P fuse) // core: Process the specified region by the specified proc.
        {
            block.coor += region.coor;
            netxs::onrect(*this, block, fuse);
        }
        template<class P>
        void  fill(P fuse) // core: Fill the client area using lambda.
        {
            fill(view(), fuse);
        }
        void  grad(rgba const& c1, rgba const& c2) // core: Fill the specified region with the linear gradient.
        {
            auto mx = (float)region.size.x;
            auto my = (float)region.size.y;
            auto len = std::sqrt(mx * mx + my * my * 4);

            auto dr = (c2.chan.r - c1.chan.r) / len;
            auto dg = (c2.chan.g - c1.chan.g) / len;
            auto db = (c2.chan.b - c1.chan.b) / len;
            auto da = (c2.chan.a - c1.chan.a) / len;

            iota x = 0, y = 0, yy = 0;
            auto allfx = [&](cell& c) {
                auto dt = std::sqrt(x * x + yy);
                auto& chan = c.bgc().chan;
                chan.r = (uint8_t)((float)c1.chan.r + dr * dt);
                chan.g = (uint8_t)((float)c1.chan.g + dg * dt);
                chan.b = (uint8_t)((float)c1.chan.b + db * dt);
                chan.a = (uint8_t)((float)c1.chan.a + da * dt);
                ++x;
            };
            auto eolfx = [&]() {
                x = 0;
                ++y;
                yy = y * y * 4;
            };
            netxs::onrect(*this, region, allfx, eolfx);
        }
        void  swap(core& target) { canvas.swap(target.canvas); } // core: Unconditionally swap canvases.
        auto  swap(grid& target)                                 // core: Move the canvas to the specified array and return the current layout size.
        {
            if (auto size = canvas.size())
            {
                if (target.size() == size) canvas.swap(target);
                else                       target = canvas;
            }
            return region.size;
        }
        auto meta(rect region) // core: Ansify/textify content of specified region
        {
            using ansi = ansi::esc;
            ansi yield;
            cell state;
            auto badfx = [&](auto& state, auto& frame) {
                state.set_gc();
                frame.add(utf::REPLACEMENT_CHARACTER_UTF8_VIEW);
            };
            auto allfx = [&](cell& c) {
                auto width = c.wdt();
                if (width < 2) // Narrow character
                {
                    if (state.wdt() == 2) badfx(state, yield); // Left part alone

                    c.scan(state, yield);
                }
                else
                {
                    if (width == 2) // Left part
                    {
                        if (state.wdt() == 2) badfx(state, yield);  // Left part alone
                        
                        c.scan_attr(state, yield);
                        state.set_gc(c); // Save char from c for the next iteration
                    }
                    else // width == 3 // Right part
                    {
                        if (state.wdt() == 2)
                        {
                            if (state.scan(c, state, yield)) state.set_gc(); // Cleanup used t
                            else
                            {
                                badfx(state, yield); // Left part alone
                                c.scan_attr(state, yield);
                                badfx(state, yield); // Right part alone
                            }
                        }
                        else
                        {
                            c.scan_attr(state, yield);
                            badfx(state, yield); // Right part alone
                        }
                    }
                }
            };
            auto eolfx = [&]() {
                if (state.wdt() == 2) badfx(state, yield);  // Left part alone
                yield.eol();
            };

            yield.nil();
            netxs::onrect(*this, region, allfx, eolfx);
            yield.nil();

            return static_cast<utf::text>(yield);
        }
        template<class P>
        void cage(ui::rect const& area, twod const& edge, P fuse) // core: Draw the cage around specified area
        {
            auto temp = area;
            temp.size.y = edge.y; // Top
            fill(temp, fuse);
            temp.coor.y += area.size.y - edge.y; // Bottom
            fill(temp, fuse);
            temp.size.x = edge.x; // Left
            temp.size.y = area.size.y - edge.y * 2;
            temp.coor.y = area.coor.y + edge.y;
            fill(temp, fuse);
            temp.coor.x += area.size.x - edge.x; // Right
            fill(temp, fuse);
        }
        template<class TEXT>
        void  text(twod const& pos, TEXT const& txt, bool rtl = faux) // core: Put the specified text substring to the specified coordinates on the canvas.
        {
            rtl ? txt.template output<true>(*this, pos)
                : txt.template output<faux>(*this, pos);
        }
        void operator += (core const& src) // core: Append specified canvas.
        {
            //todo inbody::RTL
            auto a_size = size();
            auto b_size = src.size();
            auto new_sz = twod{ a_size.x + b_size.x, std::max(a_size.y, b_size.y) };
            core block{ region.coor, new_sz, marker };

            auto full = [](auto& dst, auto& src) { dst = src; };

            auto region = rect{ twod{0,new_sz.y - a_size.y}, a_size };
            netxs::inbody<faux>(block, *this, region, dot_00, full);
            region.coor.x += a_size.x;
            region.coor.y += new_sz.y - a_size.y;
            region.size = b_size;
            netxs::inbody<faux>(block,   src, region, dot_00, full);
            
            swap(block);
            digest++;
        }
    };

    class deco // richtext: Flow controller and state holder.
    {
    public:
        dent margin;
        bias adjust;
        bool wrapln;
        bool r_to_l;
        bool rlfeed;
        iota tablen;
        twod cursor;
        //twod origin; // deco: Relative cursor external offset.
        twod corner; // deco: Nesting offset accumulator.

        constexpr
        deco(iota const& size_x, iota const& size_y)
            : margin{ size_x, size_y },
              adjust{ left },
              wrapln{ WRAPPING },
              //wrapln{ true }, //{ faux },
              r_to_l{ faux },
              rlfeed{ faux },
              tablen{ 8    }
        { }
        constexpr
        deco(twod const& size)
            : deco{ size.x, size.y }
        { }

        void commit(deco const& f)
        { 
            cursor = f.cursor;
            adjust = f.adjust;
            wrapln = f.wrapln;
            r_to_l = f.r_to_l;
            rlfeed = f.rlfeed;
            tablen = f.tablen;
            margin = f.margin;
            //origin = f.origin;
            corner = f.corner;
        }
        void ax	(iota x)        { cursor.x  = x;          }
        void ay	(iota y)        { cursor.y  = y;          }
        void ac	(twod const& c) { ax(c.x); ay(c.y);       }
        void ox	(iota x)        { cursor.x  = x - 1;      }
        void oy	(iota y)        { cursor.y  = y - 1;      }
        void oc	(twod const& c) { ox(c.x); oy(c.y);       }
        void dx	(iota n)        { cursor.x += n;          }
        void dy	(iota n)        { cursor.y += n;          }
        void nl	(iota n)        { ax(0); dy(n);           }
        void px	(iota x)        { ax (margin.h_ratio(x)); }
        void py	(iota y)        { ay (margin.v_ratio(y)); }
        void pc	(twod const& c) { px(c.x); py(c.y);       }
        void ts	(iota n)        { tablen = n ? n : 8;     }
        void br	(bool w)        { wrapln = w;             } // DECSET: CSI ? 7 h/l Auto-wrap Mode (DECAWM) or CSI ? 45 h/l reverse wrap around mode
        void hz	(bias a)        { adjust = a;             }
        void rf	(bool f)        { rlfeed = f;             }
        void yx	(bool r)        { r_to_l = r;             }
        void wl	(iota l)        { margin.west = l;        }
        void wr	(iota r)        { margin.east = r;        }
        void wt	(iota t)        { margin.head = t;        }
        void wb	(iota b)        { margin.foot = b;        }
        void wn	(side const& s)
        {
            margin.west = s.l; 
            margin.head = s.t;
            margin.east = s.r; 
            margin.foot = s.b;
        }
        void tb	(iota n) 
        {
            if (n)
            {
                dx(tablen - cursor.x % tablen);
                if (n > 0 ? --n : ++n) dx(tablen * n);
            }
        }
        void zz	()
        {
            adjust = left;
            wrapln = WRAPPING;
            //wrapln = true; //faux;
            r_to_l = faux;
            rlfeed = faux;
            tablen = 8   ;
            cursor = dot_00;
            corner = dot_00;
            margin.west = 0;
            margin.head = 0;
            margin.east = 0;
            margin.foot = 0;
        }
        twod cp() const // deco: Return absolute cursor position.
        {
            //twod coor{ cursor + origin };
            twod coor{ cursor };

            if (adjust == right) coor.x = margin.width()  - 1 - coor.x;
            if (rlfeed == true ) coor.y = margin.height() - 1 - coor.y;
            
            coor += margin.coor();
            return coor;
        }
    };

    class flow // richtext: The text line feeder.
        : public deco
    {
        deco selfcopy;        // flow: Flow state storage.
        rect viewarea;        // flow: Viewport area.
        rect pagearea;        // flow: Client area inside page margins.
        rect textline;        // flow: Textline placeholder.
        side boundary;        // flow: Affected area by the text output.
        iota curpoint = 0;    // flow: Current substring start position.
        iota fullsize = 0;    // flow: Full textline length.
        iota cursormx = 0;    // flow: Maximum x-coor value on the visible area.
        iota highness = 1;    // flow: Height of the last processed line.
        //bool arighted = faux; // flow: Is the text line right aligned.
        bool straight = faux; // flow: Text substring retrieving direction.
        //bool centered = faux; // flow: Is the text line centered.

        //twod const& size_ref;

        using hndl = void (*)(flow&, iota);

        // flow: command list
        static void exec_dx(flow& f, iota a) { f.dx(a); }
        static void exec_dy(flow& f, iota a) { f.dy(a); }
        static void exec_ax(flow& f, iota a) { f.ax(a); }
        static void exec_ay(flow& f, iota a) { f.ay(a); }
        static void exec_ox(flow& f, iota a) { f.ox(a); }
        static void exec_oy(flow& f, iota a) { f.oy(a); }
        static void exec_px(flow& f, iota a) { f.px(a); }
        static void exec_py(flow& f, iota a) { f.py(a); }
        static void exec_ts(flow& f, iota a) { f.ts(a); }
        static void exec_tb(flow& f, iota a) { f.tb(a); }
        static void exec_nl(flow& f, iota a) { f.nl(a); }
        static void exec_br(flow& f, iota a) { f.br(a); }
        static void exec_yx(flow& f, iota a) { f.yx(a); }
        static void exec_hz(flow& f, iota a) { f.hz((bias)a); }
        static void exec_rf(flow& f, iota a) { f.rf(a); }
        static void exec_wl(flow& f, iota a) { f.wl(a); }
        static void exec_wr(flow& f, iota a) { f.wr(a); }
        static void exec_wt(flow& f, iota a) { f.wt(a); }
        static void exec_wb(flow& f, iota a) { f.wb(a); }
        static void exec_sc(flow& f, iota a) { f.sc( ); }
        static void exec_rc(flow& f, iota a) { f.rc( ); }
        static void exec_zz(flow& f, iota a) { f.zz( ); }

        // flow: Draw commands (customizable)
        template<ansi::fn CMD>
        static void exec_dc(flow& f, iota a) { if (f.custom) f.custom(CMD, a); }
        //static void exec_dc(flow& f, iota a) { f.custom(CMD, a); }
        
        // flow: Abstract handler
        //       ansi::fn::ed
        //       ansi::fn::el
        //virtual void custom(ansi::fn cmd, iota arg) = 0;

        constexpr static std::array<hndl, ansi::fn_count> exec =
        {	// Order does matter, see definition of ansi::fn
            exec_dx, // horizontal delta
            exec_dy, // vertical delta
            exec_ax, // x absolute
            exec_ay, // y absolute
            exec_ox, // old format x absolute (1-based)
            exec_oy, // old format y absolute (1-based)
            exec_px, // x percent
            exec_py, // y percent
            exec_ts, // set tab size
            exec_tb, // horizontal tab
            exec_nl, // next line and reset x to west (carriage return)
            exec_br, // text wrap mode
            exec_yx, // bidi
            exec_hz, // text horizontal alignment
            exec_rf, // reverse (line) feed

            exec_wl, // set left   horizontal wrapping field
            exec_wr, // set right  horizontal wrapping field
            exec_wt, // set top      vertical wrapping field
            exec_wb, // set bottom   vertical wrapping field

            exec_sc, // save cursor position
            exec_rc, // load cursor position
            exec_zz, // all params reset to zero

            exec_dc<ansi::fn::ed>, // CSI Ps J  Erase in Display (ED), VT100.
            exec_dc<ansi::fn::el>, // CSI Ps K  Erase in Line    (EL), VT100.
        };

        // flow: Main cursor forwarding proc
        template<bool WRAP, bool RtoL, bool ReLF, class P>
        void output(P print)
        {
            //todo deprecate `origin`
            textline.coor = cursor;// +origin;
            auto printout = WRAP ? textline.trunc(pagearea.size)
                                 : textline;
            auto outwidth = WRAP ? printout.coor.x + printout.size.x - textline.coor.x
                                 : textline.size.x;
            //flow::up();
            deco::dx(outwidth);
            //flow::up();

            auto startpos = curpoint;
            curpoint += std::max(printout.size.x, 1);
            textline.size.x = fullsize - curpoint;

            if (RtoL) printout.coor.x = pagearea.size.x - (printout.coor.x + printout.size.x);
            if (ReLF) printout.coor.y = pagearea.size.y - (printout.coor.y + printout.size.y);
            else      printout.coor.y = textline.coor.y; //do not truncate y-axis?
            //todo revise: It is actually only for the coor.y that is negative.

            printout.coor += pagearea.coor;
            boundary |= printout;

            if constexpr (!std::is_same_v<P, noop>)
            if (printout)
            {
                auto& coord = printout.coor;
                auto& width = printout.size.x;
                auto& start = straight ? startpos 
                                       : textline.size.x;
                print(coord, start, width);
            }
            
            highness = textline.size.y;
        }
        //auto middle() { return (pagearea.size.x >> 1) - (textline.size.x >> 1) - origin.x; }
        auto middle() { return (pagearea.size.x >> 1) - (textline.size.x >> 1); }
        void autocr() { if (cursor.x >= cursormx) deco::nl(highness); }

        template<bool RtoL, bool ReLF, class P>
        void centred(P print)
        {
            while (textline.size.x > 0)
            {
                autocr();
                auto axis = textline.size.x >= cursormx ? 0
                                                        : middle();
                deco::ax(axis);
                output<true, RtoL, ReLF>(print);
            }
        }
        template<bool RtoL, bool ReLF, class P>
        void wrapped(P print)
        {
            while (textline.size.x > 0)
            {
                autocr();
                output<true, RtoL, ReLF>(print);
            }
        }
        template<bool RtoL, bool ReLF, class P>
        void trimmed(P print)
        {
            if (adjust == center) deco::ax(middle());
            output<faux, RtoL, ReLF>(print);
        }

        template<class T> constexpr 
        iota get_len(T p) 
        {
            if constexpr (std::is_same<T, twod>::value) return p.x;
            else                                        return static_cast<iota>(p);
        }
        template<class T> constexpr 
        rect get_vol(T p) 
        {
            if constexpr (std::is_same<T, twod>::value) return { dot_00, p };
            else                                        return { dot_00, { static_cast<iota>(p),  1 } };
        }
        template<bool RtoL, bool ReLF, class P>
        void proceed(P print)
        {
            auto centered = deco::adjust == center;
            if (deco::wrapln) if (centered) centred<RtoL, ReLF>(print);
                              else          wrapped<RtoL, ReLF>(print);
            else                            trimmed<RtoL, ReLF>(print);
        }

        // flow: Split specified textblock on the substrings
        //       and place it to the form by the specified proc.
        template<class T, class P = noop>
        void compose(T const& block, P print = P())//, twod const& offset = dot_00)
        {
            auto block_size = block.size(); // 2D size
            fullsize = get_len(block_size); // 1D length

            if (fullsize)
            {
                auto arighted = deco::adjust == bias::right;
                textline = get_vol(block_size); // 2D size
                curpoint = 0;
                straight = deco::wrapln || deco::r_to_l == arighted;
                pagearea = deco::margin;
                pagearea.coor += deco::corner;
                cursormx = pagearea.size.x;// -origin.x;

                // Move cursor down if next line is lower than previous
                if (highness > textline.size.y)
                //if (highness != textline.size.y)
                {
                    deco::dy(highness - textline.size.y);
                    highness = textline.size.y;
                }

                if (arighted)
                    if (deco::rlfeed) proceed<true, true>(print);
                    else              proceed<true, faux>(print);
                else
                    if (deco::rlfeed) proceed<faux, true>(print);
                    else              proceed<faux, faux>(print);
            }
        }
        // flow: Execute specified locus instruction list.
        auto forward(writ const& cmd)
        {
            auto& inst = *this;
            //flow::up();
            for (auto [cmd, arg] : cmd)
            {
                flow::exec[cmd](inst, arg);
            }
            return flow::up();
        }

    public:
        std::function<void(ansi::fn cmd, iota arg)> custom; // flow: Draw commands (customizable)

        flow(iota const& size_x, iota const& size_y)
            : deco     { size_x, size_y },
              selfcopy { size_x, size_y }
        { }
        flow(twod const& size )
            : flow { size.x, size.y }
        { }

        // flow: Register cursor position.
        twod up ()
        {
            auto cp = deco::cp();
            boundary |= cp; /* |= cursor*/;
            return cp;
        }
        template<class T>
        auto print(T const& block, core& canvas)
        {
            auto cp = forward(block);
            compose(block,
                [&](auto const& coord, auto start, auto width)
                {
                    canvas.text(coord, block.substr(start, width), deco::r_to_l);
                });
            return cp;
        }
        template<class T>
        auto print(T const& block)
        {
            auto cp = forward(block);
            compose(block);
            return cp;
        }
        void sc () { selfcopy.commit(*this); } // flow: Save state.
        void rc () { deco::commit(selfcopy); } // flow: Restore state.
        void reset() // flow: Reset flow state.
        {
            deco::zz(); 
            flow::sc();
            boundary = cursor;
        }
        auto& minmax() const { return boundary; } // flow: Return the output range.
        void  minmax(twod const& p) { boundary |= p; } // flow: Return the output range.
        //auto& areasize() { return size_ref; }
    };

    class shot // richtext: The shadow of the para.
    {
        static constexpr iota maxlen = std::numeric_limits<iota>::max();

        core const& basis;
        iota        start;
        iota        width;

    public:
        constexpr 
        shot(shot const&) = default;

        constexpr 
        shot(core const& basis, iota begin, iota piece)
            : basis{ basis },
              start{ std::max(begin, 0) },
              width{ std::min(std::max(piece, 0), basis.size().x - start) }
        {
            if (basis.size().x <= start)
            {
                start = 0;
                width = 0;
            }
        }

        constexpr
        shot(core const& basis)
            : basis{ basis },
              start{       },
              width{ basis.size().x }
        { }

        constexpr
        auto substr(iota begin, iota piece = maxlen) const
        {
            auto w = basis.size().x;
            auto a = start + std::max(begin, 0);
            return a < w ? shot{ basis, a, std::min(std::max(piece, 0), w - a) }
                         : shot{ basis, 0, 0 };
        }

                  auto& mark  () const { return  basis.mark();         }
                  auto  data  () const { return  basis.data() + start; }
        constexpr auto  size  () const { return  basis.size();         }
        constexpr auto  empty () const { return !width;                }
        constexpr auto  length() const { return  width;                }

        template<bool RtoL>
        auto output(core& canvas, twod const& pos) const  // wide: Print the source content using the specified print proc, which returns the number of characters printed.
        {
            //todo place is wrong if RtoL==true
            //rect place{ pos, { RtoL ? width, basis.size().y } };
            //auto joint = canvas.view().clip(place);
            rect place{ pos, { width, basis.size().y } };
            auto joint = canvas.view().clip(place);
            //auto joint = canvas.area().clip(place);

            if (joint)
            {
                auto fuse = [&](auto& dst, auto& src) { dst.fusefull(src); };
                
                if constexpr (RtoL)
                {
                    place.coor.x += place.size.x - joint.coor.x - joint.size.x;
                    place.coor.y  = joint.coor.y - place.coor.y;
                }
                else
                {
                    place.coor = joint.coor - place.coor;
                }
                place.coor.x += start;
                netxs::inbody<RtoL>(canvas, basis, joint, place.coor, fuse);
            }

            return width;
        }
    };

    class para // richtext: Enriched text paragraph.
    {
        friend class page;
        using corx = sptr<core>;

        cell spare;     // para: Saved brush
        cell prime;     // para: Prime brush
        grid proto;     // para: Proto lyric
        iota width = 0; // para: Length of the proto lyric
        iota caret = 0; // para: Cursor position inside lyric

        id_t parid;

    public:
        template<class T>
        using parser = ansi::parser<T>;

        text debug; // para: debug string
        cell brush; // para: Current brush
        writ locus;
        corx lyric = std::make_shared<core>();

        para()                         = default;
        para(para&&) noexcept          = default;
        para(para const&)              = default;
        para& operator = (para&&)      = default;
        para& operator = (para const&) = default;

        para(id_t parid)
            : parid{ parid }
        { }

        para(cell const& brush)
            : brush{ brush },
              prime{ brush }
        { }

        para(view utf8, cell const& brush = cell{})
            : para{ brush }
        {
            ansi::parse(utf8, this);
        }
        
        para& operator = (view utf8) // para: Replace with a new text. Preserve current attributes.
        {
            wipe(brush);
            return operator += (utf8);
        }
        para& operator += (view utf8) // para: Append a new text using current attributes.
        {
            ansi::parse(utf8, this);
            return *this;
        }
        //para& operator += (para const& p) // para: Append a new text using current attributes.
        //{
        //	lyric+= p.lyric;
        //	brush = p.brush;
        //	return *this;
        //}
        
        operator writ const& () const { return locus; }

        void decouple() { lyric = std::make_shared<core>(*lyric); } // para: Make canvas isolated copy.
        //auto&  settle() const { return  locus; } // para: Return paragraph locator.
        shot   shadow() const { return *lyric; } // para: Return paragraph shadow.
        shot   substr(iota start, iota width) const // para: Return paragraph substring shadow.
        {
            return shadow().substr(start, width);
        }
        bool   bare() const { return locus.bare();    } // para: Does the paragraph have no locator?
        auto length() const { return lyric->size().x; } // para: Return printable length.
        auto   step() const { return lyric->size().x - caret; } // para: Return step back.
        auto   size() const { return lyric->size();   } // para: Return 2D volume size.
        auto&  back() const { return brush;           } // para: Return current brush.
        bool  empty() const { return !length() && proto.empty() && (brush == prime); } // para: Is it filled?
        void   ease() { brush = spare; lyric->each([&](auto& c) { c.clr(brush); });  } // para: Reset color for all text.
        void   link(id_t id)         { lyric->each([&](auto& c) { c.link(id);   });  } // para: Set object ID for each cell.
        //void wipe_locus() { locus.kill(); }
        void   wipe(cell c = cell{}) // para: Clear the text and locus, and reset SGR attributes.
        {
            //if (entirely)
            //{
            //	brush.wipe();
            //	prime = brush;
            //}

            width = 0;
            caret = 0;
            
            //prime = spare = brush = c;
            prime = brush = c;

            debug.clear();
            proto.clear();
            locus.kill();
            lyric->kill();
        }
        void task(ansi::rule const& cmd) { if (empty()) locus.push(cmd); } // para: Add locus command. In case of text presence try to change current target otherwise abort content building.
        void post(utf::frag const& cluster)                                // para: Add grapheme cluster.
        {
            static ansi::marker<cell> marker;

            auto& utf8 = cluster.text;
            auto& attr = cluster.attr;

            if (unsigned w = attr.wcwidth)
            {
                width += w;
                brush.set_gc(utf8, w);
                proto.push_back(brush);

                debug += (debug.size() ? "_"s : ""s) + text(utf8);
            }
            else
            {
                if (auto set_prop = marker.setter[attr.control])
                {
                    if (proto.size())
                    {
                        set_prop(proto.back());
                    }
                    else
                    {
                        auto empty = brush;
                        empty.txt(whitespace).wdt(w);
                        set_prop(empty);
                        proto.push_back(empty);
                    }
                }
                else
                {
                    brush.set_gc(utf8, w);
                    proto.push_back(brush);
                }
                auto i = utf::to_hex((size_t)attr.control, 5, true);
                debug += (debug.size() ? "_<fn:"s : "<fn:"s) + i + ">"s;
            }
        }
        // para: Convert into the screen-adapted sequence (unfold, remove zerospace chars, etc.).
        void cook()
        {
            //log(" para cook ", " para id ", this);

            //	//if (front)
            //	//{
            //	//	//+ evaluate TAB etc
            //	//	//+ bidi
            //	//	//+ eliminate non-printable and with cwidth == 0 (\0, \t, \b, etc...)
            //	//	//+ while (--wide)
            //	//	//	{
            //	//	//		/* IT IS UNSAFE IF REALLOCATION OCCURS. BOOK ALWAYS */
            //	//	//		lyric.emplace_back(cluster, console::whitespace);
            //	//	//	}
            //	//	//+ convert front into the screen-like sequence (unfold, remmove zerospace chars)
            //	//

            auto merge = [&](auto fuse) {
                for (auto& c : proto)
                {
                    auto w = c.wdt();
                    if (w == 1)
                    {
                        fuse(c);
                    }
                    else if (w == 2)
                    {
                        fuse(c.wdt(2));
                        fuse(c.wdt(3));
                        w -= 2;
                    }
                    else if (w == 0)
                    {
                        //todo implemet controls/commands
                        // winsrv2019's cmd.exe sets title with a zero at the end
                        //fuse(cell{ c, whitespace });
                    }
                    else if (w > 2)
                    {
                        /// Forbid using wide characters until terminal emulators support the fragmentation attribute
                        auto dumb = c.txt(utf::REPLACEMENT_CHARACTER_UTF8_VIEW);
                        while (w--)
                        {
                            fuse(dumb);
                        }
                    }
                }
            };

            auto& lyric = *this->lyric;

            bool need_attetion = faux;

            //todo unify for size.y > 1
            auto newsz = caret + width;
            if (newsz > lyric.size().x)
            {
                //if (lyric.size().x)
                //{
                //	log("ols size ", lyric.size().x, " new size ", newsz);
                //	log("caret ", caret, " width ", width);
                //}


                //lyric.crop(twod{ newsz,1 });
                //lyric.crop(newsz);

                lyric.crop(twod{ newsz, std::max(lyric.size().y,1) });
            }
            
            auto it = lyric.data() + caret;
            merge([&](auto c) { *it++ = c; });

            caret = newsz;
            //log("cook ", caret);
            proto.clear();
            width = 0;

            /// for the lyric of rich
            //auto newsz = caret + width;
            //if (caret == lyric.size())
            //{
            //	lyric.reserve(newsz);
            //	merge([&](auto c) { lyric.push_back(c); });
            //}
            //else
            //{
            //	if (newsz > lyric.size()) lyric.resize(newsz, cell{ whitespace });
            //
            //	auto it = lyric.begin() + caret;
            //	merge([&](auto c) { *it++ = c; });
            //}
            //
            //caret = newsz;
            //proto.clear();
            //width = 0;
        }
        
        auto& set(cell const& c)  { brush.set(c); return *this;   } // para: Set the brush using the cell (keeping link and paragraph id).
        void  nil()               { brush.set(spare);             } // para: Restore saved SGR attributes.
        void  sav()               { spare.set(brush);             } // para: Save current SGR attributes.
        void  nil(cell const& c)  { brush.set(c);                 } // para: Restore saved SGR attributes.
        void  sav(cell& c)        { c.set(brush);                 } // para: Save current SGR attributes.
        void  rfg()               { brush.fgc(spare.fgc());       } // para: Reset SGR Foreground color.
        void  rbg()               { brush.bgc(spare.bgc());       } // para: Reset SGR Background color.
        void  rfg(cell const& c)  { brush.fgc(c.fgc());           } // para: Reset SGR Foreground color.
        void  rbg(cell const& c)  { brush.bgc(c.bgc());           } // para: Reset SGR Background color.
        void  fgc(rgba const& c)  { brush.fgc(c);                 } // para: FG color.
        void  bgc(rgba const& c)  { brush.bgc(c);                 } // para: BG color.
        void  bld(bool b)         { brush.bld(b);                 } // para: Bold.
        void  itc(bool b)         { brush.itc(b);                 } // para: Italic.
        void  inv(bool b)         { brush.inv(b);                 } // para: Inversion.
        void  und(bool b)         { brush.und(b);                 } // para: Underline.
        //void  chx(iota n)         { caret = n;                    } // para: Move caret to n.
        auto chx() const          { return caret;  }
        //void chx(iota new_pos)    { caret = new_pos;  }
        void chx(iota new_pos)    
        {
            //if (new_pos > 1000)
            //	throw;
            caret = new_pos;
        }

        auto  id() const          { return parid;  }
        void  id(id_t newid)      { parid = newid; }

        //void  chx(iota n) { caret = 0; } // para: Move caret to 0.
        ///void  cuf(iota n) { cook(); caret = std::max(caret+n, 0); } // para: Move caret by n.

        void trim(cell const& default_cell)
        {
            auto& lyric = *this->lyric;

            auto& data = lyric.pick();
            auto  head = data.rbegin();
            auto  tail = data.rend();
            
            while (head != tail)
            {
                auto& c = *head;

                //if (c.wdt()!=0 && c.txt().front() != ' ' && c != default_cell) break;
                if (c != default_cell) break;
                //if (!c.issame_visual(default_cell)) break;
                ++head;
            }

            //log(" trim fr ", lyric.size().x);
            //if (default_cell.txt().front() != ' ') log(" def cell: ", default_cell.txt().front());

            auto size = static_cast<iota>(tail - head);
            if (size != lyric.size().x)
            {
                //log(" trim to ", size);
                lyric.crop({ size, 1 });
            }
        }

        //todo deprecate cursor movement
        void  cuf(iota n) // para: Move caret by n.
        { 
            cook();
            //log("caret=", caret, " step=", n);
            caret = std::max(caret+n, 0);
        }
        void  del(iota n) // para: Delete (not Erase) letters under the caret.
        {
            /*
            As characters are deleted, the remaining characters
            between the cursor and right margin move to the left. 
            Character attributes move with the characters. 
            The terminal adds blank characters at the right margin.
            */

            cook();

            //shot shadow = lyric;
            //iota size = lyric.size();
            //if (caret < size)
            //{
            //	auto p1 = shadow.substr(0, caret);
            //	auto p2 = shadow.substr(std::min(caret + n, size));
            //	rich t;
            //	t.reserve(p1.size() + p2.size());
            //	t = p1;
            //	t += p2;
            //	lyric = std::move(t);
            //}

            auto& lyric = *this->lyric;

            //todo unify for size.y > 1
            if (n > 0)
            {
                //todo revise shifting to the left (right? RTL?)
                auto size = lyric.size();
                if (caret + n < size.x)
                {
                    auto dst = lyric.data() + caret;

                    auto end = dst + (size.x - (caret + n));
                    auto src = dst + n;
                    while (dst != end)
                    {
                        *dst++ = *src++;
                    }
                    lyric.crop(size.x - n);
                }
                else if (caret < size.x)
                {
                    lyric.crop(caret);
                }

                //add empty cell at the end
                //todo optimize realloc
                auto d = lyric.size();
                d.y = std::max(d.y, 1);
                d.x += n;
                auto b = brush;
                b.txt(whitespace);
                //b.txt('.');
                lyric.crop(d, b);
            }
            else
            {
                //todo negative n support
            }
        }
        // Insert n spaces (= Erase n, CSI n X)
        void ins(iota n)
        {
            static const utf::frag space = utf::frag
            {
                utf::WHITESPACE_CHARACTER_UTF8_VIEW,
                utf::prop(0x20, 1)
            };
            
            for (auto i = 0; i < n; ++i)
                post(space);
        }
    };

    class rope // richtext: Cascade of the identical paragraphs.
    {
        using iter = typename std::list<para>::const_iterator;
        iter source;
        iter finish;
        iota prefix;
        iota suffix;
        twod volume; // Rope must consist of text lines of the same height

        rope(iter& source, iota prefix, iter& finish, iota suffix, twod const& volume)
            : source{ source },
              prefix{ prefix },
              finish{ finish },
              suffix{ suffix },
              volume{ volume }
        { }

    public:
        rope(iter const& head, iter const& tail, twod const& size)
            : source{ head },
              finish{ tail },
              prefix{ 0    },
              suffix{ 0    },
              volume{ size }
        { }

        operator writ const& () const { return *source; }

        // rope: Return a substring rope the source content. 
        //       ! No checking of boundaries !
        rope substr(iota start, iota width) const
        {
            auto first = source;
            auto piece = first->size().x;

            while (piece <= start)
            {
                start -= piece;
                piece = (++first)->size().x;
            }

            auto end = first;
            piece -= start;
            while (piece < width)
            {
                piece += (++end)->size().x;
            }
            piece -= width;

            return rope{ first, start, end, piece, { width, volume.y } };
        }
        // rope: Print the source content using the specified print proc, 
        //       which returns the number of characters printed.
        template<bool RtoL>
        void output(core& canvas, twod locate) const
        {
            auto total = volume.x;

            auto draw = [&](auto& item, auto start, auto width) 
            {
                auto line = item.substr(start, width);
                auto size = line.template output<RtoL>(canvas, locate);
                locate.x += size;
                return size;
            };

            if constexpr (RtoL)
            {
                auto crop = [](auto piece, auto total, auto& start, auto& width)
                {
                    if (piece > total)
                    {
                        start = piece - total;
                        width = total;
                    }
                    else
                    {
                        start = 0;
                        width = piece;
                    }
                };

                auto refer = finish;
                auto& item = *refer;
                auto piece = item.size().x - suffix;
                
                iota start, width, yield;
                crop(piece, total, start, width);
                yield = draw(item, start, width);

                while (total -= yield)
                {
                    auto& item = *--refer;
                    piece = item.size().x;

                    crop(piece, total, start, width);
                    yield = draw(item, start, width);
                }
            }
            else
            {
                auto refer = source;
                auto& item = *refer;
                auto yield = draw(item, prefix, total);

                while (total -= yield)
                {
                    auto& item = *++refer;
                    yield = draw(item, 0, total);
                }
            }
        }
        constexpr 
        auto  size  () const { return volume;        } // rope: Return volume of the source content.
        auto  length() const { return volume.x;      } // rope: Return the length of the source content.
        auto& mark  () const { return finish->brush; } // rope: Return the the last paragraph brush state.
        auto  id    () const { return source->id();  } // rope: Return paragraph id.
        auto  caret () const { return source->chx(); } // rope: Return interal paragraph caret.
        //auto  locus () const { return source->settle(); } // rope: Return coor instructions.
    };

    class page // richtext: Enriched text page.
    {
        using list = std::list<para>;
        using iter = list::iterator;
        using imap = std::map<iota, iter>;

    protected:
        //cell  spare_fallback;
        //cell& spare;

        id_t  parid = 1;             // page: Current paragraph id.
        list  batch = { para(parid) };    // page: The list of the rich-text paragraphs.
        iter  layer = batch.begin(); // page: Current paragraph.
        imap  parts;                 // page: Embedded text blocks.
        iota  limit = std::numeric_limits<iota>::max(); // page: Paragraphs number limit.

        // page: Remove over limit paragraphs.
        void shrink()
        {
            auto size = batch.size();
            if (size > limit)
            {
                auto item = static_cast<iota>(std::distance(batch.begin(), layer));

                while (batch.size() > limit)
                {
                    batch.pop_front();
                }
                batch.front().locus.clear();
                // Update current layer tr if it gets out
                if (item < size - limit) layer = batch.begin();
            }
        }

    public:
        cell spare;

        template<class T>
        struct parser : public ansi::parser<T>
        {
            using base = ansi::parser<T>;
            parser() : base()
            {
                using namespace netxs::console::ansi;
                // base::intro[ctrl::BS ] = VT_PROC{ p->cuf(-q.pop_all(ctrl::BS )); };
                // base::intro[ctrl::DEL] = VT_PROC{ p->bsp( q.pop_all(ctrl::DEL)); };
                base::intro[ctrl::CR ] = VT_PROC{ q.pop_if(ctrl::EOL); p->task({ fn::nl,1 }); };
                base::intro[ctrl::TAB] = VT_PROC{ p->task({ fn::tb,q.pop_all(ctrl::TAB) }); };
                base::intro[ctrl::EOL] = VT_PROC{ p->task({ fn::nl,q.pop_all(ctrl::EOL) }); };
                base::csier.table[CSI__ED] = VT_PROC{ p->task({ fn::ed, q(0) }); }; // CSI Ps J
                base::csier.table[CSI__EL] = VT_PROC{ p->task({ fn::el, q(0) }); }; // CSI Ps K
                //base::csier.table[CSI__EL] = VT_PROC{ p->  el(q(0)); }; // CSI Ps K
                // base::csier.table[CSI_DCH] = VT_PROC{ p-> del(q(1)); };
                // base::csier.table[CSI_ECH] = VT_PROC{ p-> del(q(1)); };
                base::csier.table[CSI_CCC][CCC_NOP] = VT_PROC{ p->fork(); };
                base::csier.table[CSI_CCC][CCC_IDX] = VT_PROC{ p->fork(q(0)); };
                base::csier.table[CSI_CCC][CCC_REF] = VT_PROC{ p->bind(q(0)); };
            }
        };

        page& operator = (page const&) = default;
        page ()                        = default;
        page (page&&)                  = default;
        page (page const&)             = default;
        page (view const& utf8)
            : page()
        {
            ansi::parse(utf8, this);
        }

        // page: Acquire para by id.
        auto& operator [] (iota index)
        {
            if (netxs::on_key(parts, index))
            {
                return *parts[index];
            }
            else
            {
                fork(index);
                return *layer;
            }
        }
        // page: Wipe current content and store parsed UTF-8 text string.
        auto& operator =  (view utf8)
        {
            clear();
            ansi::parse(utf8, this);
            return *this;
        }
        // page: Parse UTF-8 text string and appends result.
        auto& operator += (view utf8)
        {
            //layer->cook();
            ansi::parse(utf8, this);
            return *this;
        }

        // page: Append another page. Move semantic.
        page& operator += (page const& p)
        {
            //parts.insert(p.parts.begin(), p.parts.end()); // Part id should be unique across pages
            //batch.splice(std::next(layer), p.batch);

            for (auto& a: p.batch)
            {
                batch.push_back(a);
                batch.back().id(++parid);
            }
            shrink();
            layer = std::prev(batch.end());
            return *this;
        }


        //todo implement pararaph's id synchronization
        // page: Append another page. Move semantic.
        //page& operator += (page& p)
        //{
        //	parts.insert(p.parts.begin(), p.parts.end()); /// Part id should be unique across pages
        //	batch.splice(std::next(layer), p.batch);
        //	shrink();
        //	layer = std::prev(batch.end());
        //	return *this;
        //}
        // page: Append rich-textline. Dont copy - Move semantic.
        //page& operator += (para& p)
        //{
        //	layer->cook();
        //	batch.insert(std::next(layer), std::move(p));
        //	layer = std::prev(batch.end());
        //	return *this;
        //}
        // page: Set the limit of paragraphs.
        void maxlen(iota m)
        {
            limit = std::max(1, m);
            shrink();
        }
        // page: Get the limit of paragraphs.
        auto maxlen()
        {
            return limit;
        }
        //void hz_trim(iota max_width)
        //{
        //	auto s = twod{ max_width, 1 };
        //	for (auto& p : page::batch)
        //	{
        //		p.lyric->crop(s);
        //	}
        //}
        // page: Clear the list of paragraphs.
        page& clear (bool preserve_brush = faux)
        {
            cell brush = preserve_brush ? layer->brush : cell{};
            parts.clear();
            batch.resize(1);
            layer = batch.begin();
            parid = 1;
            batch.front().id(parid);
            //brush.para(parid);
            layer->wipe(brush);
            return *this;
        }
        
        // page: Disintegrate the page content into atomic contiguous pieces - ropes.
        //       Call publish(rope{first, last, length}): 
        //       a range of [ first,last ] is the uniform consecutive paragraphs set.
        //       Length is the sum of the length of each paragraph.
        template<class F>
        void stream(F publish) const
        {
            twod next;
            auto last = batch.begin();
            auto tail = batch.end();
            while (last != tail)
            {
                auto size = last->size();
                auto head = last;
                while (++last != tail
                      && last->bare()
                      && size.y == (next = last->size()).y)
                {
                    size.x += next.x;
                }

                publish(rope{ head, std::prev(last), size });
            }
        }
    //private:
    //protected:
        // page: Split the text run.
        void fork()
        {
            layer->cook();
            layer = batch.insert(std::next(layer), para{ layer->brush });
            layer->id(++parid);
            //layer->brush.para(++parid);
            shrink();
            //log("fork");
        }
        // page: Split the text run and associate the next paragraph with id.
        void fork(iota id)
        {
            fork();
            parts[id] = layer;
        }
        // page: Make a shared copy of an existing paragraph,
        //       or create a new one if it doesn't exist.
        void bind(iota id)
        {
            if (!layer->empty()) fork();

            auto it = parts.find(id);
            if (it != parts.end())
                layer->lyric = (*it).second->lyric;
            else
                parts.emplace(id, layer);
        }
        // page: Add locus command. In case of text presence to change 
        //       current target otherwise abort content building.
        void task(ansi::rule const& cmd)
        {
            if (!layer->empty())
            {
                fork();
            }
            layer->locus.push(cmd);
        }
        
        //// page: Backspace char, move caret back.
        //void backspace_chr(iota n) { layer->cuf(-n); }

        // page: Backspace key, delete n previous characters.
        // void  bsp(iota n) 
        // {
        // 	if (layer->caret >= n )
        // 	{
        // 		layer->caret -= n;
        // 		layer->del(n);
        // 	}
        // 	else
        // 	{
        // 		auto  here = layer->caret;
        // 		auto there = n - here;
        // 	
        // 		layer->caret = 0;
        // 		if (here) layer->del(here);
        // 	
        // 		{
        // 			if (layer != batch.begin())
        // 			{
        // 				if (!layer->length())
        // 				{
        // 					if (layer->locus.bare())
        // 					{
        // 						layer = std::prev(batch.erase(layer));
        // 					}
        // 					else
        // 					{
        // 						layer->locus.pop();
        // 					}
        // 					there--;
        // 				}
        // 				else
        // 				{
        // 					auto prev = std::prev(layer);
        // 					*(*prev).lyric += *(*layer).lyric;
        // 					batch.erase(layer);
        // 					layer = prev;
        // 					layer->caret = layer->length();
        // 				}
        // 			}
        // 		}
        // 	
        // 		//while (there)
        // 		//{
        // 		//	//todo concat layer with previous and try to move caret back then del(n)
        // 		//
        // 		//	while (layer != batch.begin())
        // 		//	{
        // 		//		layer = std::prev(batch.erase(layer));
        // 		//		if (layer->length())
        // 		//		{
        // 		//			layer->lyric.pop_back();
        // 		//			break;
        // 		//		}
        // 		//	}
        // 		//}
        // 	}
        // }

        //void bsp(iota n)
        //{
        //	//log("page bsp: ", n);
        //	//layer->bsp(n);
        //
        //	if (layer->caret >= n )
        //	{
        //		layer->caret -= n;;
        //		layer->del(n);
        //	}
        //	else
        //	{
        //		//todo concat layer with previous and try to move caret back then del(n)
        //
        //		//while (layer != batch.begin())
        //		//{
        //		//	layer = std::prev(batch.erase(layer));
        //		//	if (layer->lyric.size())
        //		//	{
        //		//		layer->lyric.pop_back();
        //		//		break;
        //		//	}
        //		//}
        //	}
        //
        //	//if (layer->lyric.size())
        //	//{
        //	//	layer->lyric.pop_back();
        //	//}
        //	//else
        //	//{
        //	//	while (layer != batch.begin())
        //	//	{
        //	//		layer = std::prev(batch.erase(layer));
        //	//		if (layer->lyric.size())
        //	//		{
        //	//			layer->lyric.pop_back();
        //	//			break;
        //	//		}
        //	//	}
        //	//}
        //
        //	//if (layer->lyric.size())
        //	//{
        //	//	layer->lyric.pop_back();
        //	//}
        //	//else 
        //	//{
        //	//	if (layer != batch.begin())
        //	//	{
        //	//		layer = std::prev(batch.erase(layer));
        //	//		if (layer->lyric.size())
        //	//		{
        //	//			layer->lyric.pop_back();
        //	//		}
        //	//		else
        //	//		{
        //	//			if (layer != batch.begin())
        //	//			{
        //	//				layer = std::prev(batch.erase(layer));
        //	//				if (layer->lyric.size())
        //	//				{
        //	//					layer->lyric.pop_back();
        //	//				}
        //	//				else
        //	//				{
        //	//					//todo fall into a loop
        //	//				}
        //	//			}
        //	//		}
        //	//
        //	//		//bool same_line = layer->locus.bare();
        //	//		//layer = std::prev(batch.erase(layer));
        //	//		//if (same_line && layer->lyric.size())
        //	//		//{
        //	//		//	layer->lyric.pop_back();
        //	//		//}
        //	//	}
        //	//}
        //}
        void post(utf::frag const& cluster) { layer->post(cluster); }
        void cook() { layer->cook(); }
        auto size() const { return static_cast<iota>(batch.size()); }
        auto& current()       { return *layer; } // page: Access to the current paragraph.
        auto& current() const { return *layer; } // page: RO access to the current paragraph.

        void  nil() { layer->nil(spare); }
        void  sav() { layer->sav(spare); }
        void  rfg() { layer->rfg(spare); }
        void  rbg() { layer->rbg(spare); }

        void  fgc(rgba const& c) { layer->fgc(c); } // para: Color.
        void  bgc(rgba const& c) { layer->bgc(c); } // para: Color.
        void  bld(bool b) { layer->bld(b); }
        void  itc(bool b) { layer->itc(b); }
        void  inv(bool b) { layer->inv(b); }
        void  und(bool b) { layer->und(b); }

        void  chx(iota n) { layer->chx(n); } // page: Move caret to n.
        void  cuf(iota n) { layer->cuf(n); } // page: Move caret forward by n.

        //void  del(iota n) { layer->del(n); } // page: CSI Delete n letters.

        //void   el(iota n)
        //{
        //	cook();
        //
        //	auto& lyric = *layer->lyric;
        //
        //	switch (n)
        //	{
        //		default:
        //		case 0: /// Ps = 0  ⇒  Erase to Right (default).
        //			if (layer->caret >= lyric.size().x)
        //			{
        //				task(ansi::rule{ ansi::fn::el, 0 });
        //			}
        //			else
        //			{
        //				//layer->lyric.resize(layer->caret);
        //				//lyric.crop(layer->caret);
        //				lyric.crop({ layer->caret,1 });
        //				//why?
        //				//task(ansi::rule{ ansi::fn::el, 0 });
        //			}
        //			break;
        //		case 1: /// Ps = 1  ⇒  Erase to Left.
        //			task(ansi::rule{ ansi::fn::el, 1 });
        //			break;
        //		case 2: // Ps = 2  ⇒  Erase All.
        //			task(ansi::rule{ ansi::fn::el, 2 });
        //			break;
        //	}
        //}


        // page: Inset tabs via space
        void tab(iota n)
        {
            layer->ins(n);
        }
    };

    class face // richtext: Textographical canvas.
        : public core, protected flow, public std::enable_shared_from_this<face>
    {
        using vrgb = std::vector<irgb>;

        cell brush;
        twod anker;     // face: The position of the nearest visible paragraph.
        id_t piece = 1; // face: The nearest to top paragraph.

        twod cover; // face: Text output region
        vrgb cache; // face: BlurFX temp buffer

        // face: Is the c inside the viewport?
        bool inside(twod const& c)
        {
            return c.y >= 0 && c.y < region.size.y;
            //todo X-axis
        }

    public:
        //todo tie cover and flow::corner
        face()
            : flow{ cover }
        {
            flow::custom = [&](ansi::fn cmd, iota arg) { face::custom(cmd, arg); };
        }

        //todo revise
        bool caret = faux; // face: Cursor visibility.
        bool moved = faux; // face: Is reflow required?
        bool decoy = true; // face: Is the cursor inside the viewport?

        // face: Print paragraph.
        void output(para const& block)
        {
            flow::print(block, *this);
        }
        // face: Print page.
        void output(page const& textpage)
        {
            auto publish = [&](auto const& combo)
            {
                flow::print(combo, *this);
                brush = combo.mark(); // current mark of the last printed fragment
            };
            textpage.stream(publish);
        }
        // face: Print page with holding top visible paragraph on its own place.
        void output(page const& textpage, bool reset)
        {
            //todo if cursor is visible when tie to the cursor position
            //     else tie to the first visible text line

            bool done = faux;
            // Get vertical position of the nearest paragraph to the top.
            auto gain = [&](auto const& combo)
            {
                auto pred = flow::print(combo, *this);
                brush = combo.mark(); // current mark of the last printed fragment

                auto post = deco::cp();
                if (!done)
                {
                    if (pred.y <= 0 && post.y >= 0)
                    {
                        anker.y = pred.y;
                        piece = combo.id();
                        done = true;
                    }
                    else
                    {
                        if (std::abs(anker.y) > std::abs(pred.y))
                        {
                            anker.y = pred.y;
                            piece = combo.id();
                        }
                    }
                }
            }; 

            // Get vertical position of the specified paragraph.
            auto find = [&](auto const& combo)
            {
                auto cp = flow::print(combo);
                if (combo.id() == piece) anker = cp;
            };

            if (reset)
            {
                anker.y = std::numeric_limits<iota>::max();
                textpage.stream(gain);

                decoy = caret && inside(deco::cp());
            }
            else
            { 
                textpage.stream(find);
            }
        }
        // face: Reflow text page on the canvas and hold position 
        //       of the top visible paragraph while resizing.
        void reflow(page& textpage)
        {
            if (moved)
            {
                flow::zz(); //flow::sc();

                auto delta = anker;
                output(textpage, faux);
                std::swap(delta, anker);

                auto  cover = flow::minmax();
                //auto& basis = flow::origin;
                auto basis = dot_00;// flow::origin;
                basis.y += anker.y - delta.y;

                if (decoy)
                {
                    // Don't tie the first line if it's the only one. Make one step forward.
                    if (anker.y == 0 &&
                        anker.y == deco::cp().y &&
                        cover.height() > 1)
                    {
                        // the increment is removed bcos it shifts mc one row down on Ctrl+O and back
                        //basis.y++;
                    }

                    auto newcp = deco::cp();
                    if (!inside(newcp))
                    {
                        if (newcp.y < 0) basis.y -= newcp.y;
                        else             basis.y -= newcp.y - region.size.y + 1;
                    }
                }
                else
                {
                    basis.y = std::clamp(basis.y, -cover.b, region.size.y - cover.t - 1);
                }

                moved = faux;
            }

            wipe();
        }
        //todo revise flow inheritance acces mode
        // face: Return flow::corner reference.
        twod& corner()
        {
            return flow::corner;
        }
        // face: Set flow::corner.
        void corner(twod const& newcorner)
        {
            flow::corner = newcorner;
        }
        // face: Return relative cursor offset.
        //twod& origin()
        //{
        //	return flow::origin;
        //}
        //// face: Set relative cursor offset.
        //void origin(twod const& delta)
        //{
        //	flow::origin = delta;
        //}
        // face: Return affected area by the text output.
        side minmax()
        {
            return flow::minmax();
        }
        // face: Forward call to the core and reset cursor.
        template<class ...Args>
        void wipe(Args... args)
        {
            core::wipe(args...);
            flow::reset();
        }

        auto& cup () const        { return flow::cursor;        } // face: Get cursor position.
        face& cup (twod const& p) { flow::ac( p); return *this; } // face: Cursor 0-based absolute position.
        face& chx (iota x)        { flow::ax( x); return *this; } // face: Cursor 0-based horizontal absolute.
        face& chy (iota y)        { flow::ay( y); return *this; } // face: Cursor 0-based vertical absolute.
        face& cpp (twod const& p) { flow::pc( p); return *this; } // face: Cursor percent position.
        face& cpx (iota x)        { flow::px( x); return *this; } // face: Cursor H percent position.
        face& cpy (iota y)        { flow::py( y); return *this; } // face: Cursor V percent position.
        face& cuu (iota n = 1)    { flow::dy(-n); return *this; } // face: cursor up.
        face& cud (iota n = 1)    { flow::dy( n); return *this; } // face: Cursor down.
        face& cuf (iota n = 1)    { flow::dx( n); return *this; } // face: Cursor forward.
        face& cub (iota n = 1)    { flow::dx(-n); return *this; } // face: Cursor backward.
        face& cnl (iota n = 1)    { flow::dx(-n); return *this; } // face: Cursor next line.
        face& cpl (iota n = 1)    { flow::dx(-n); return *this; } // face: Cursor previous line.

        face& ocp (twod const& p) { flow::oc( p); return *this; } // face: Cursor 1-based absolute position.
        face& ocx (iota x)        { flow::ox( x); return *this; } // face: Cursor 1-based horizontal absolute.
        face& ocy (iota y)        { flow::oy( y); return *this; } // face: Cursor 1-based vertical absolute.

        face& scp ()              { flow::sc(  ); return *this; } // face: Save cursor position.
        face& rcp ()              { flow::rc(  ); return *this; } // face: Restore cursor position.
        face& mgn (side const& s) { flow::wn( s); return *this; } // face: Margin left, right, top, bottom.
        face& mgl (iota n)        { flow::wl( n); return *this; } // face: Margin left	╮.
        face& mgr (iota n)        { flow::wr( n); return *this; } // face: Margin right	│ positive - native binding.
        face& mgt (iota n)        { flow::wt( n); return *this; } // face: Margin top	│ negative - oppisite binding.
        face& mgb (iota n)        { flow::wb( n); return *this; } // face: Margin bottom	╯.
        face& tab (iota n = 1)    { flow::tb( n); return *this; } // face: Proceed the \t.
        face& eol (iota n = 1)    { flow::nl( n); return *this; } // face: Proceed the \r || \n || \r\n.
        face& jet (bias j)        { flow::hz( j); return *this; } // face: Text alignment (bias).
        face& tbs (iota n)        { flow::ts( n); return *this; } // face: Tab step length.
        face& wrp (bool w)        { flow::br( w); return *this; } // face: Text wrapping on/off.
        face& rlf (bool f)        { flow::rf( f); return *this; } // face: Reverse line feed on/off.
        face& rtl (bool m)        { flow::yx( m); return *this; } // face: Text right-to-left on/off.
        auto& rtl () const        { return flow::r_to_l;        } // face: Text right-to-left on/off.
        face& rst ()  { flow::zz(  ); flow::sc(); return *this; } // face: Reset to zero all cursor params.
        //todo revise
        auto cp	() { return flow::cp(); } // face: Return relative cursor position.


        //todo unify all core::size
        auto& size () // face: Return the size of the face/core.
        {
            return core::size();
        }
        void  size (twod const& newsize) // face: Change the size of the face/core.
        {
            core::size(newsize);
            cover = newsize;
        }
        void client(twod const& area)
        {
            cover = area;
        }
        auto& client() const
        {
            return cover;
        }
        rect client_area() const
        {
            return { flow::corner, cover };
        }

        // face: BACKSPACE Erase previous cell in line 
        //       and move the cursor one step back.
        //auto& bsp (iota n)
        //{
        //	//todo EGC should we erase last codepoint 
        //	//     in grapheme cluster instead of wiping the cell?
        //
        //	auto caret = flow::cp();
        //	auto block = rect{ caret, {-n,1} }.normalize();
        //	//auto brush = last;
        //
        //	flow::dx(-n);
        //
        //	log("backspace ", n, " times");
        //
        //	//todo implement wrap around
        //	//todo trim the lyric
        //
        //	//core::fill(block, brush.txt(whitespace));
        //	core::fill(block, brush);
        //	return *this;
        //}

        //virtual void custom(ansi::fn cmd, iota arg)
        void custom(ansi::fn cmd, iota arg)
        {
            auto caret = flow::cp();
            switch (cmd)
            {
                case ansi::fn::ed: // CSI Ps J  Erase in Display (ED).
                {
                    auto block = rect{ flow::margin.coor(), { flow::margin.width(),0 } };
                    auto brush = core::marker;
                    //auto color = face::brush;// last used brush
                    //color.txt(whitespace);
                    switch (arg)
                    {
                        default:
                        case 0:	// Erase Below (default).
                            block.size.y = std::max(flow::margin.height() - caret.y, 0);
                            block.coor.y = caret.y;
                            break;

                        case 1:	// Erase Above.
                            block.size.y = std::max(caret.y - flow::margin.coor().y, 0);
                            break;

                        case 3:	//todo implement Erase Scrollback 
                        case 2:	// Erase All.
                            block.size.y = flow::margin.height();
                            break;
                    }
                    core::fill(block, brush.txt(whitespace));
                    break;
                }
                case ansi::fn::el: // CSI Ps K  Erase in Line (EL). Cursor position does not change (https://en.wikipedia.org/wiki/ANSI_escape_code#CSI_sequences).
                {
                    auto block = rect{ caret, {0,1} };
                    switch (arg)
                    {
                        default:
                        case 0:	// Erase to Right (default).
                            block.size.x = std::max(flow::margin.width() - caret.x, 0);
                            break;
                        case 1:	// Erase to Left.
                            block.size.x = -std::max(caret.x - flow::margin.coor().x, 0);
                            block = block.normalize();
                            break;
                        case 2:	// Erase Line.
                            block.size.x = flow::margin.width();
                            block.coor.x = flow::margin.coor().x;
                            break;
                    }
                    auto color = face::brush;// last used brush
                    color.txt(whitespace);

                    //auto color = cell{ whitespace };
                    //color.bgc(face::brush.bga() ? face::brush.bgc()
                    //                            : core::marker.bgc());// last used brush
                    //color.fgc(face::brush.fga() ? face::brush.fgc()
                    //                            : core::marker.fgc());// last used brush
                    
                    //log ("face::brush  fg/bg ", face::brush.fgc(), face::brush.bgc());
                    //log ("core::marker fg/bg ", core::marker.fgc(), core::marker.bgc());
                    //color.bgc(core::marker.bgc());// last used brush
                    //color.fgc(core::marker.fgc());// last used brush
                    //color.bgc(face::brush.bgc());// last used brush
                    //color.fgc(face::brush.fgc());// last used brush
                    
                                                  //color.fuse(face::brush);
                    //color.txt(whitespace);
                    
                    //auto color = face::brush;// last used brush
                    //color.fuse(core::marker);
                    //color.txt(whitespace);

                    core::fill(block, color);
                    break;
                }
                default:
                    break;
            };
        }

        template<class P = noop>
        void blur(iota r, P shade = P())
        {
            auto view = core::view();
            auto size = core::size();

            auto w = view.size.x;
            auto h = view.size.y;

            if (auto size = w * h; cache.size() < size) cache.resize(size);

            auto s_ptr = core::data(view.coor);
            auto d_ptr = cache.data();

            auto s_width = size.x;
            auto d_width = view.size.x;

            auto s_point = [](cell* c)->auto& { return c->bgc(); };
            auto d_point = [](irgb* c)->auto& { return *c;       };

            netxs::bokefy<irgb>(s_ptr,
                                d_ptr, w,
                                       h, r, s_width,
                                             d_width, s_point,
                                                      d_point, shade);
        }

        // face: Render nested object to the canvas using renderproc. TRIM = trim viewport to the client area.
        template<bool TRIM = true, class T>
        void render(sptr<T> nested_ptr, twod const& basis)
        {
            if (nested_ptr)
            {
                auto& nested = *nested_ptr;
                face::render<TRIM>(nested, basis);
            }
        }
        // face: Render nested object to the canvas using renderproc. TRIM = trim viewport to the client area.
        template<bool TRIM = true, class T>
        void render(T& nested, twod const& basis)
        {
            auto canvas_view = core::view();
            auto corner_coor = face::corner();

            auto nested_area = nested.square();
            auto nested_coor = nested_area.coor;
            nested_area.coor += corner_coor;

            auto nested_view = canvas_view.clip(nested_area);
            if (TRIM ? nested_view : canvas_view)
            {
                auto client_size = face::client();
                auto canvas_coor = core::coor();
                auto offset_coor = basis;
                if (TRIM) core::view(nested_view);
                face::corner(corner_coor + nested_coor);
                face::client(nested_area.size);
                core::move  (canvas_coor - offset_coor);

                nested.renderproc(*this);
                nested.postrender(*this);

                if (TRIM) core::view(canvas_view); // restore canvas.view
                face::corner(corner_coor); // restore canvas.corner
                face::client(client_size); // restore canvas.client_size
                core::move  (canvas_coor); // restore canvas.coor
            }
        }
        // face: Render itself to the canvas using renderproc.
        template<class T>
        void render(T& object)
        {
            auto canvas_view = core::view();
            auto corner_coor = face::corner();

            auto nested_area = object.square();
            nested_area.coor -= core::coor();
            nested_area.coor += corner_coor;

            if (auto nested_view = canvas_view.clip(nested_area))
            {
                auto client_size = face::client();
                core::view  (nested_view);
                face::corner(corner_coor + nested_area.coor);
                face::client(nested_area.size);

                object.renderproc(*this);
                object.postrender(*this);

                core::view  (canvas_view); // restore canvas.view
                face::corner(corner_coor); // restore canvas.corner
                face::client(client_size); // restore canvas.client_size
            }
        }
    };

    //todo revise
    //struct meta ///<summary> richtext: . </summary>
    //{
    //	bool rigid;
    //	cell brush;
    //	rect field;

        // core: Fill the specified region with the its own color and copying method.
        //void	draw (mesh const& shape)
        //{
        //	for (auto& part : shape)
        //	{
        //		fill(part.field, part.brush, part.rigid);
        //	}
        //}
        // core: Fill the specified region with the specified color.
        //void	draw (mesh const& shape, cell const& elem)
        //{
        //	for (auto& part : shape)
        //	{
        //		fill(part.field, elem, part.rigid);
        //	}
        //}

    //};
    //
    //using mesh = std::vector<meta>;

    class tone
    {
    public:

        #define PROP_LIST                       \
        X(kb_focus , "Keyboard focus indicator")\
        X(brighter , "Highlighter modificator") \
        X(shadower , "Darklighter modificator") \
        X(shadow   , "Light Darklighter modificator") \
        X(lucidity , "Global transparency")     \
        X(selector , "Selection overlay")       \
        X(bordersz , "Border size")

        #define X(a, b) a,
        enum prop { PROP_LIST count };
        #undef X

        //#define X(a, b) b,
        //text description[prop::count] = { PROP_LIST };
        //#undef X
        #undef PROP_LIST

        prop active  = prop::brighter;
        prop passive = prop::shadower;
    };
}

#endif // NETXS_RICHTEXT_H