// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_LAYOUT_HPP
#define NETXS_LAYOUT_HPP

#include "../abstract/duplet.hpp"
#include "../abstract/hash.hpp"
#include "../text/utf.hpp"
#include "events.hpp"

#include <cstring> // std::memcpy

namespace netxs::ui::atoms
{
    using utf::text;
    using utf::view;
    using id_t = netxs::events::bell::id_t;

    static const char whitespace = 0x20;
    //static const char whitespace = '.';

    enum svga
    {
        truecolor,
        vga16    ,
        vga256   ,
    };

    enum Z_order : iota
    {
        backmost = -1,
        plain    =  0,
        topmost  =  1,
    };

    enum tint
    {
        blackdk, reddk, greendk, yellowdk, bluedk, magentadk, cyandk, whitedk,
        blacklt, redlt, greenlt, yellowlt, bluelt, magentalt, cyanlt, whitelt,
    };
    struct tint16
    {
        enum : iota
        {
            blackdk,
            blacklt,
            graydk,
            graylt,
            whitedk,
            whitelt,
            redlt,
            bluelt,
            greenlt,
            yellowlt,
            magentalt,
            reddk,
            bluedk,
            greendk,
            yellowdk,
            cyanlt,
        };
    };

    // layout: 8-bit RGBA.
    union rgba
    {
        struct rgba_t { uint8_t r, g, b, a; } chan;
        uint32_t                              token;

        constexpr rgba ()
            : token(0)
        { }

        template<class T, class A = uint8_t>
        constexpr rgba (T r, T g, T b, A a = 0xff)
            : chan{ static_cast<uint8_t>(r),
                    static_cast<uint8_t>(g),
                    static_cast<uint8_t>(b),
                    static_cast<uint8_t>(a) }
        { }

        constexpr rgba (rgba const& c)
            : token(c.token)
        { }

        constexpr rgba (tint c)
            : rgba{ color256[c] }
        { }

        constexpr rgba (uint32_t c)
            : token(c)
        { }

        rgba (fifo& queue)
        {
            static constexpr auto mode_RGB = 2;
            static constexpr auto mode_256 = 5;
            auto mode = queue.rawarg(mode_RGB);
            if (fifo::issub(mode))
            {
                switch(fifo::desub(mode))
                {
                    case mode_RGB:
                        chan.r = queue.subarg(0);
                        chan.g = queue.subarg(0);
                        chan.b = queue.subarg(0);
                        chan.a = queue.subarg(0xFF);
                        break;
                    case mode_256:
                        token = color256[queue.subarg(0)];
                        break;
                    default:
                        break;
                }
            }
            else
            {
                switch(mode)
                {
                    case mode_RGB:
                        chan.r = queue(0);
                        chan.g = queue(0);
                        chan.b = queue(0);
                        chan.a = 0xFF;
                        break;
                    case mode_256:
                        token = color256[queue(0)];
                        break;
                    default:
                        break;
                }
            }
        }

        auto operator == (rgba const& c) const
        {
            return token == c.token;
        }
        auto operator != (rgba const& c) const
        {
            return !operator == (c);
        }
        // rgba: Set all channels to zero.
        void wipe()
        {
            token = 0;
        }
        // rgba: Set color to opaque black.
        void rst()
        {
            static constexpr uint32_t colorblack = 0xFF000000;

            token = colorblack;
        }
        // rgba: Are the colors alpha blenable?
        auto is_alpha_blendable() const
        {
            if (chan.a && chan.a != 0xFF)
            {
                return true;
            }
            return faux;
        }
        // rgba: Set alpha channel.
        void alpha(uint8_t k)
        {
            chan.a = k;
        }
        // rgba: Return alpha channel.
        auto alpha() const
        {
            return chan.a;
        }
        // rgba: Colourimetric (perceptual luminance-preserving) conversion to greyscale.
        auto luma() const
        {
            return static_cast<uint8_t>(0.2627 * chan.r
                                      + 0.6780 * chan.g
                                      + 0.0593 * chan.b);
        }
        // rgba: Return 256-color 6x6x6 cube.
        auto to256cube() const
        {
            uint8_t clr;
            if (chan.r == chan.g
             && chan.r == chan.b)
            {
                clr = 232 + ((chan.r * 24) >> 8);
            }
            else
            {
                clr = 16 + 36 * ((chan.r * 6) >> 8)
                          + 6 * ((chan.g * 6) >> 8)
                              + ((chan.b * 6) >> 8);
            }
            return clr;
        }
        // rgba: Equal both to their average.
        void avg(rgba& c)
        {
            chan.r = c.chan.r = (chan.r + c.chan.r) >> 1;
            chan.g = c.chan.g = (chan.g + c.chan.g) >> 1;
            chan.b = c.chan.b = (chan.b + c.chan.b) >> 1;
        }
        // rgba: One-side alpha blending RGBA colors.
        void inline mix_one(rgba const& c)
        {
            if (c.chan.a == 0xFF)
            {
                chan = c.chan;
            }
            else if (c.chan.a)
            {
                auto blend = [](auto const& c1, auto const& c2, auto const& alpha)
                {
                    return ((c1 << 8) + (c2 - c1) * alpha) >> 8;
                };
                chan.r = blend(chan.r, c.chan.r, c.chan.a);
                chan.g = blend(chan.g, c.chan.g, c.chan.a);
                chan.b = blend(chan.b, c.chan.b, c.chan.a);

                //if (!chan.a) chan.a = c.chan.a;
            }
        }
        // rgba: Alpha blending RGBA colors.
        void inline mix(rgba const& c)
        {
            if (c.chan.a == 0xFF)
            {
                chan = c.chan;
            }
            else if (c.chan.a)
            {
                //todo consider premultiplied alpha
                auto a1 = chan.a;
                auto a2 = c.chan.a;
                unsigned const a = ((a2 + a1) << 8) - a1 * a2;
                auto blend2 = [&](auto c1, auto c2)
                {
                    auto t = c1 * a1;
                    unsigned d = (((c2 * a2 + t) << 8) - t * a2);
                    return d / a;
                    //return (((c2 * a2 + t) << 8) - t * a2) / a;
                };
                chan.r = blend2(chan.r, c.chan.r);
                chan.g = blend2(chan.g, c.chan.g);
                chan.b = blend2(chan.b, c.chan.b);
                chan.a = a >> 8;
            }
        }
        // rgba: RGBA transitional blending. Level = 0: equals c1, level = 256: equals c2.
        static auto transit(rgba const& c1, rgba const& c2, iota level)
        {
            auto inverse = 256 - level;
            return rgba{ (c2.chan.r * level + c1.chan.r * inverse) >> 8,
                         (c2.chan.g * level + c1.chan.g * inverse) >> 8,
                         (c2.chan.b * level + c1.chan.b * inverse) >> 8,
                         (c2.chan.a * level + c1.chan.a * inverse) >> 8 };
        }
        // rgba: Rough alpha blending RGBA colors.
        //void mix_alpha(rgba const& c)
        //{
        //	auto blend = [] (auto const& c1, auto const& c2, auto const& alpha)
        //	{
        //		return ((c1 << 8) + (c2 - c1) * alpha) >> 8;
        //	};
        //	auto norm = [](auto const& c2, auto const& alpha)
        //	{
        //		return (c2 * alpha) >> 8;
        //	};
        //
        //	if (chan.a)
        //	{
        //		if (c.chan.a)
        //		{
        //			auto& a1 = chan.a;
        //			auto& a2 = c.chan.a;
        //			chan.r = blend(norm(c.chan.r, a2), chan.r, a1);
        //			chan.g = blend(norm(c.chan.g, a2), chan.g, a1);
        //			chan.b = blend(norm(c.chan.b, a2), chan.b, a1);
        //			chan.a = c.chan.a;
        //		}
        //	}
        //	else
        //	{
        //		chan = c.chan;
        //	}
        //}

        //// rgba: Are the colors identical.
        //bool like(rgba const& c) const
        //{
        //
        //	static constexpr uint32_t k = 0b11110000;
        //	static constexpr uint32_t threshold = 0x00 + k << 16 + k << 8 + k;
        //	return	(token & threshold) == (c.token & threshold);
        //}
        // rgba: Shift color.
        void xlight()
        {
            if (chan.r + chan.g + chan.b > 140*3) // Make it darker
            {
                auto k = 64;
                chan.r = chan.r < k ? 0x00 : chan.r - k;
                chan.g = chan.g < k ? 0x00 : chan.g - k;
                chan.b = chan.b < k ? 0x00 : chan.b - k;
            }
            else // Make it lighter
            {
                //auto k = 180;
                auto k = 48;
                chan.r = chan.r > 0xFF - k ? 0xFF : chan.r + k;
                chan.g = chan.g > 0xFF - k ? 0xFF : chan.g + k;
                chan.b = chan.b > 0xFF - k ? 0xFF : chan.b + k;
            }
        }
        // rgba: Darken the color.
        void shadow(uint8_t k = 39)//24)
        {
            if (chan.r + chan.g + chan.b > 39)//24)
            {
                chan.r = chan.r < k ? 0x00 : chan.r - k;
                chan.g = chan.g < k ? 0x00 : chan.g - k;
                chan.b = chan.b < k ? 0x00 : chan.b - k;
            }
            else
            {
                chan.r = chan.r > 0xFF - k ? 0xFF : chan.r + k;
                chan.g = chan.g > 0xFF - k ? 0xFF : chan.g + k;
                chan.b = chan.b > 0xFF - k ? 0xFF : chan.b + k;
            }
        }
        // rgba: Lighten the color.
        void bright(uint8_t k = 39)//24) reduced in order to correct highlight the cellatix
        {
            if (chan.r + chan.g + chan.b > 255*3 - 39)//24)
            {
                chan.r = chan.r < k ? 0x00 : chan.r - k;
                chan.g = chan.g < k ? 0x00 : chan.g - k;
                chan.b = chan.b < k ? 0x00 : chan.b - k;
            }
            else
            {
                chan.r = chan.r > 0xFF - k ? 0xFF : chan.r + k;
                chan.g = chan.g > 0xFF - k ? 0xFF : chan.g + k;
                chan.b = chan.b > 0xFF - k ? 0xFF : chan.b + k;
            }
        }
        // rgba: Invert the color.
        void invert()
        {
            token = (token & 0xFF000000) | ~(token & 0x00FFFFFF);
        }
        // rgba: Serialize the color.
        auto str() const
        {
            return "{" + std::to_string(chan.r) + ","
                       + std::to_string(chan.g) + ","
                       + std::to_string(chan.b) + ","
                       + std::to_string(chan.a) + "}";
        }
        static constexpr uint32_t color16[] =
        {
            0xFF000000, // 0  blackdk
            0xFF202020, // 1  blacklt
            0xFF505050, // 2  graydk
            0xFF808080, // 3  graylt
            0xFFd0d0d0, // 4  whitedk
            0xFFffffff, // 5  whitelt
            0xFF5648E6, // 6  redlt
            0xFFFF783A, // 7  bluelt
            0xFF0CC615, // 0  greenlt
            0xFFA5F1F8, // 1  yellowlt
            0xFF9E00B3, // 2  magentalt
            0xFF1F0FC4, // 3  reddk
            0xFFDB3700, // 4  bluedk
            0xFF0EA112, // 5  greendk
            0xFF009CC0, // 6  yellowdk
            0xFFD6D660, // 7  cyanlt
        };
        static constexpr uint32_t color256[] =
        {
            0xFF101010,	// 0  blackdk
            0xFF1F0FC4,	// 1  reddk
            0xFF0EA112,	// 2  greendk
            0xFF009CC0,	// 3  yellowdk
            0xFFDB3700,	// 4  bluedk
            0xFF981787,	// 5  magentadk
            0xFFDD963B,	// 6  cyandk
            0xFFBBBBBB,	// 7  whitedk
            0xFF757575,	// 8  blacklt
            0xFF5648E6,	// 9  redlt
            0xFF0CC615,	// 10 greenlt
            0xFFA5F1F8,	// 11 yellowlt
            0xFFFF783A,	// 12 bluelt
            0xFF9E00B3,	// 13 magentalt
            0xFFD6D660,	// 14 cyanlt
            0xFFF3F3F3,	// 15 whitelt
            // 6×6×6 RGB-cube (216 colors), index = 16 + 36r + 6g + b, r,g,b=[0, 5]
            0xFF000000, 0xFF5F0000, 0xFF870000, 0xFFAF0000, 0xFFD70000, 0xFFFF0000,
            0xFF005F00, 0xFF5F5F00, 0xFF875F00, 0xFFAF5F00, 0xFFD75F00, 0xFFFF5F00,
            0xFF008700, 0xFF5F8700, 0xFF878700, 0xFFAF8700, 0xFFD78700, 0xFFFF8700,
            0xFF00AF00, 0xFF5FAF00, 0xFF87AF00, 0xFFAFAF00, 0xFFD7AF00, 0xFFFFAF00,
            0xFF00D700, 0xFF5FD700, 0xFF87D700, 0xFFAFD700, 0xFFD7D700, 0xFFFFD700,
            0xFF00FF00, 0xFF5FFF00, 0xFF87FF00, 0xFFAFFF00, 0xFFD7FF00, 0xFFFFFF00,

            0xFF00005F, 0xFF5F005F, 0xFF87005F, 0xFFAF005F, 0xFFD7005F, 0xFFFF005F,
            0xFF005F5F, 0xFF5F5F5F, 0xFF875F5F, 0xFFAF5F5F, 0xFFD75F5F, 0xFFFF5F5F,
            0xFF00875F, 0xFF5F875F, 0xFF87875F, 0xFFAF875F, 0xFFD7875F, 0xFFFF875F,
            0xFF00AF5F, 0xFF5FAF5F, 0xFF87AF5F, 0xFFAFAF5F, 0xFFD7AF5F, 0xFFFFAF5F,
            0xFF00D75F, 0xFF5FD75F, 0xFF87D75F, 0xFFAFD75F, 0xFFD7D75F, 0xFFFFD75F,
            0xFF00FF5F, 0xFF5FFF5F, 0xFF87FF5F, 0xFFAFFF5F, 0xFFD7FF5F, 0xFFFFFF5F,

            0xFF000087, 0xFF5F0087, 0xFF870087, 0xFFAF0087, 0xFFD70087, 0xFFFF0087,
            0xFF005F87, 0xFF5F5F87, 0xFF875F87, 0xFFAF5F87, 0xFFD75F87, 0xFFFF5F87,
            0xFF008787, 0xFF5F8787, 0xFF878787, 0xFFAF8787, 0xFFD78787, 0xFFFF8787,
            0xFF00AF87, 0xFF5FAF87, 0xFF87AF87, 0xFFAFAF87, 0xFFD7AF87, 0xFFFFAF87,
            0xFF00D787, 0xFF5FD787, 0xFF87D787, 0xFFAFD787, 0xFFD7D787, 0xFFFFD787,
            0xFF00FF87, 0xFF5FFF87, 0xFF87FF87, 0xFFAFFF87, 0xFFD7FF87, 0xFFFFFF87,

            0xFF0000AF, 0xFF5F00AF, 0xFF8700AF, 0xFFAF00AF, 0xFFD700AF, 0xFFFF00AF,
            0xFF005FAF, 0xFF5F5FAF, 0xFF875FAF, 0xFFAF5FAF, 0xFFD75FAF, 0xFFFF5FAF,
            0xFF0087AF, 0xFF5F87AF, 0xFF8787AF, 0xFFAF87AF, 0xFFD787AF, 0xFFFF87AF,
            0xFF00AFAF, 0xFF5FAFAF, 0xFF87AFAF, 0xFFAFAFAF, 0xFFD7AFAF, 0xFFFFAFAF,
            0xFF00D7AF, 0xFF5FD7AF, 0xFF87D7AF, 0xFFAFD7AF, 0xFFD7D7AF, 0xFFFFD7AF,
            0xFF00FFAF, 0xFF5FFFAF, 0xFF87FFAF, 0xFFAFFFAF, 0xFFD7FFAF, 0xFFFFFFAF,

            0xFF0000D7, 0xFF5F00D7, 0xFF8700D7, 0xFFAF00D7, 0xFFD700D7, 0xFFFF00D7,
            0xFF005FD7, 0xFF5F5FD7, 0xFF875FD7, 0xFFAF5FD7, 0xFFD75FD7, 0xFFFF5FD7,
            0xFF0087D7, 0xFF5F87D7, 0xFF8787D7, 0xFFAF87D7, 0xFFD787D7, 0xFFFF87D7,
            0xFF00AFD7, 0xFF5FAFD7, 0xFF87AFD7, 0xFFAFAFD7, 0xFFD7AFD7, 0xFFFFAFD7,
            0xFF00D7D7, 0xFF5FD7D7, 0xFF87D7D7, 0xFFAFD7D7, 0xFFD7D7D7, 0xFFFFD7D7,
            0xFF00FFD7, 0xFF5FFFD7, 0xFF87FFD7, 0xFFAFFFD7, 0xFFD7FFD7, 0xFFFFFFD7,

            0xFF0000FF, 0xFF5F00FF, 0xFF8700FF, 0xFFAF00FF, 0xFFD700FF, 0xFFFE00FF,
            0xFF005FFF, 0xFF5F5FFF, 0xFF875FFF, 0xFFAF5FFF, 0xFFD75FFF, 0xFFFE5FFF,
            0xFF0087FF, 0xFF5F87FF, 0xFF8787FF, 0xFFAF87FF, 0xFFD787FF, 0xFFFE87FF,
            0xFF00AFFF, 0xFF5FAFFF, 0xFF87AFFF, 0xFFAFAFFF, 0xFFD7AFFF, 0xFFFEAFFF,
            0xFF00D7FF, 0xFF5FD7FF, 0xFF87D7FF, 0xFFAFD7FF, 0xFFD7D7FF, 0xFFFED7FF,
            0xFF00FFFF, 0xFF5FFFFF, 0xFF87FFFF, 0xFFAFFFFF, 0xFFD7FFFF, 0xFFFFFFFF,
            // Grayscale colors, 24 steps
            0xFF080808, 0xFF121212, 0xFF1C1C1C, 0xFF262626, 0xFF303030, 0xFF3A3A3A,
            0xFF444444, 0xFF4E4E4E, 0xFF585858, 0xFF626262, 0xFF6C6C6C, 0xFF767676,
            0xFF808080, 0xFF8A8A8A, 0xFF949494, 0xFF9E9E9E, 0xFFA8A8A8, 0xFFB2B2B2,
            0xFFBCBCBC, 0xFFC6C6C6, 0xFFD0D0D0, 0xFFDADADA, 0xFFE4E4E4, 0xFFEEEEEE,
        };
        friend auto& operator<< (std::ostream& s, rgba const& c)
        {
            return s << "{" << (int)c.chan.r
                     << "," << (int)c.chan.g
                     << "," << (int)c.chan.b
                     << "," << (int)c.chan.a
                     << "}";
        }
    };

    // layout: Templated RGB.
    template<class T>
    struct irgb
    {
        T r, g, b;

        irgb() = default;

        irgb(T r, T g, T b)
            : r{ r }, g{ g }, b{ b }
        { }

        irgb(rgba const& c)
            : r { c.chan.r },
              g { c.chan.g },
              b { c.chan.b }
        { }

        operator rgba() const  { return rgba{ r, g, b }; }

        template<class V>
        auto operator / (V v) const
        {
            return irgb<T>{ r / v, g / v, b / v }; // 10% faster than divround.

            //return irgb<T>{ utils::divround(r, v),
            //                utils::divround(g, v),
            //                utils::divround(b, v) };
        }

        template<class V>
        void operator *=(V v)
        {
            r *= v; g *= v; b *= v;
        }
        void operator =(irgb const& c)
        {
            r = c.r;
            g = c.g;
            b = c.b;
        }
        void operator +=(irgb const& c)
        {
            r += c.r;
            g += c.g;
            b += c.b;
        }
        void operator -=(irgb const& c)
        {
            r -= c.r;
            g -= c.g;
            b -= c.b;
        }
        void operator +=(rgba const& c)
        {
            r += c.chan.r;
            g += c.chan.g;
            b += c.chan.b;
        }
        void operator -=(rgba const& c)
        {
            r -= c.chan.r;
            g -= c.chan.g;
            b -= c.chan.b;
        }
    };

    // layout: Enriched grapheme cluster.
    class cell
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

            // There is no need to reset/clear/flush the map because
            // count of different grapheme clusters is finite.
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

            // Check grapheme clusters equality.
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
                set(cluster.text, cluster.attr.ucwidth);
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
            // There are no applicable rich text formatting attributes due to their gradual nature
            // e.g.: the degree of thickness or italiciety/oblique varies from 0 to 255, etc.,
            // and should not be represented as a flag.
            //
            // In Chinese, the underline/underscore is a punctuation mark for proper names
            // and should never be used for emphasis.
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
                        bitstate unline : 2; // 0: no underline, 1 - single, 2 - double underline
                        bitstate invert : 1;
                        bitstate overln : 1;
                        bitstate strike : 1;
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
                // sizeof(*this);
                // sizeof(param.shared.var);
                // sizeof(param.unique.var);
            }
            bool operator != (body const& b) const
            {
                return !operator == (b);
            }
            bool like(body const& b) const
            {
                return param.shared.token == b.param.shared.token;
            }
            template<svga VGAMODE = svga::truecolor, class T>
            void get(body& base, T& dest) const
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
                        if constexpr (VGAMODE == svga::vga16) dest.inv(cvar.unline);
                        else                                  dest.und(cvar.unline);
                    }
                    if (cvar.invert != bvar.invert)
                    {
                        dest.inv(cvar.invert);
                    }
                    if (cvar.strike != bvar.strike)
                    {
                        dest.stk(cvar.strike);
                    }
                    if (cvar.overln != bvar.overln)
                    {
                        dest.ovr(cvar.overln);
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
            void und (iota n) { param.shared.var.unline = n; }
            void inv (bool b) { param.shared.var.invert = b; }
            void ovr (bool b) { param.shared.var.overln = b; }
            void stk (bool b) { param.shared.var.strike = b; }
            void rtl (bool b) { param.shared.var.r_to_l = b; }
            void vis (iota l) { param.unique.var.render = l; }

            bool bld () const { return param.shared.var.bolded; }
            bool itc () const { return param.shared.var.italic; }
            iota und () const { return param.shared.var.unline; }
            bool inv () const { return param.shared.var.invert; }
            bool ovr () const { return param.shared.var.overln; }
            bool stk () const { return param.shared.var.strike; }
            bool rtl () const { return param.shared.var.r_to_l; }
            iota vis () const { return param.unique.var.render; }
        };
        struct clrs
        {
            // Concept of using default colors:
            //  if alpha is set to zero, then underlaid color should be used.

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
                //sizeof(*this);
            }
            bool operator != (clrs const& c) const
            {
                return !operator == (c);
            }

            template<svga VGAMODE = svga::truecolor, class T>
            void get(clrs& base, T& dest)	const
            {
                if (bg != base.bg)
                {
                    base.bg = bg;
                    dest.template bgc<VGAMODE>(bg);
                }
                if (fg != base.fg)
                {
                    base.fg = fg;
                    dest.template fgc<VGAMODE>(fg);
                }
            }
            void wipe()
            {
                bg.wipe();
                fg.wipe();
            }
        };

        clrs       uv;     // 8U, cell: RGBA color.
        glyf<void> gc;     // 8U, cell: Grapheme cluster.
        body       st;     // 4U, cell: Style attributes.
        id_t       id = 0; // 4U, cell: Link ID.
        id_t       rsrvd0; // 4U, cell: pad, the size should be a power of 2.
        id_t       rsrvd1; // 4U, cell: pad, the size should be a power of 2.

    public:
        cell() = default;

        cell(char c)
            : gc{ c }
        {
            // sizeof(glyf<void>);
            // sizeof(clrs);
            // sizeof(body);
            // sizeof(id_t);
            // sizeof(id_t);
            // sizeof(cell);
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
        { }

        cell(cell const& base, view const& cluster, size_t ucwidth)
            : uv{ base.uv },
              st{ base.st },
              id{ base.id },
              gc{ base.gc, cluster, ucwidth }
        { }

        cell(cell const& base, char c)
            : uv{ base.uv },
              st{ base.st },
              id{ base.id },
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
        template<svga VGAMODE = svga::truecolor, class T>
        void scan_attr(cell& base, T& dest) const
        {
            if (!like(base))
            {
                //todo additionally consider UNIQUE ATTRIBUTES
                uv.get<VGAMODE>(base.uv, dest);
                st.get<VGAMODE>(base.st, dest);
            }
        }
        // cell: Get differences (ANSI CSI/SGR format) of "base" and add it to "dest" and update the "base".
        template<svga VGAMODE = svga::truecolor, class T>
        void scan(cell& base, T& dest) const
        {
            if (!like(base))
            {
                //todo additionally consider UNIQUE ATTRIBUTES
                uv.get<VGAMODE>(base.uv, dest);
                st.get<VGAMODE>(base.st, dest);
            }

            if (wdt()) dest += gc.get();
            else       dest += whitespace;
        }
        // cell: !!! Ensure that this.wdt == 2 and the next wdt == 3 and they are the same.
        template<svga VGAMODE = svga::truecolor, class T>
        bool scan(cell& next, cell& base, T& dest) const
        {
            if (gc.same(next.gc) && like(next))
            {
                if (!like(base))
                {
                    //todo additionally consider UNIQUE ATTRIBUTES
                    uv.get<VGAMODE>(base.uv, dest);
                    st.get<VGAMODE>(base.st, dest);
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
        // cell: Cell transitional color blending (fg/bg only).
        void avg(cell const& c1, cell const& c2, iota level)
        {
            uv.fg = rgba::transit(c1.uv.fg, c2.uv.fg, level);
            uv.bg = rgba::transit(c1.uv.bg, c2.uv.bg, level);
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
                                    gc = c.gc;          return *this; }
        cell& alpha (uint8_t k)   { bga(k); fga(k);     return *this; } // cell: Set alpha/transparency (background and foreground).
        cell& bgc (rgba const& c) { uv.bg = c;          return *this; } // cell: Set Background color.
        cell& fgc (rgba const& c) { uv.fg = c;          return *this; } // cell: Set Foreground color.
        cell& bga (uint8_t k)     { uv.bg.chan.a = k;   return *this; } // cell: Set Background alpha/transparency.
        cell& fga (uint8_t k)     { uv.fg.chan.a = k;   return *this; } // cell: Set Foreground alpha/transparency.
        cell& bld (bool b)        { st.bld(b);          return *this; } // cell: Set Bold attribute.
        cell& itc (bool b)        { st.itc(b);          return *this; } // cell: Set Italic attribute.
        cell& und (bool b)        { st.und(b ? 1 : 0);  return *this; } // cell: Set Underline attribute.
        cell& dnl (bool b)        { st.und(b ? 2 : 0);  return *this; } // cell: Set Double underline attribute.
        cell& ovr (bool b)        { st.ovr(b);          return *this; } // cell: Set Overline attribute.
        cell& inv (bool b)        { st.inv(b);          return *this; } // cell: Set Invert attribute.
        cell& stk (bool b)        { st.stk(b);          return *this; } // cell: Set Strikethrough attribute.
        cell& rtl (bool b)        { st.rtl(b);          return *this; } // cell: Set Right-To-Left attribute.
        cell& link(id_t oid)      { id = oid;           return *this; } // cell: Set link object ID.
        cell& txt (view c)        { c.size() ? gc.set(c) : gc.wipe(); return *this; } // cell: Set Grapheme cluster.
        cell& txt (char c)        { gc.set(c);          return *this; } // cell: Set Grapheme cluster from char.
        cell& clr (cell const& c) { uv = c.uv;          return *this; } // cell: Set the foreground and background colors only.
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
        bool        und () const { return st.und() == 1 ? true : faux; } // cell: Return Underline/Underscore attribute.
        bool        dnl () const { return st.und() == 2 ? true : faux; } // cell: Return Underline/Underscore attribute.
        bool        ovr () const { return st.ovr();      } // cell: Return Underline/Underscore attribute.
        bool        inv () const { return st.inv();      } // cell: Return Negative attribute.
        bool        stk () const { return st.stk();      } // cell: Return Strikethrough attribute.
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
            return faux;
        }
    };

    // Extern link statics.
    template<class T> std::hash<view>          cell::glyf<T>::coder;
    template<class T> text                     cell::glyf<T>::empty;
    template<class T> std::map<uint64_t, text> cell::glyf<T>::jumbo;

    struct bias { enum : iota { none, left, right, center, }; };
    struct wrap { enum : iota { none, on,  off,            }; };
    struct rtol { enum : iota { none, rtl, ltr,            }; };
    struct feed { enum : iota { none, rev, fwd,            }; };

    struct rect
    {
        twod coor;
        twod size;

        // rect: Intersect two rects. If NESTED==true when use dot_00 as a base corner.
        template<bool NESTED = faux>
        constexpr
        rect clip(rect block) const
        {
            auto clamp = [&](auto const& base, auto const& apex)
            {
                auto block_apex = block.coor + block.size;
                block.coor = std::clamp(block.coor, base, apex);
                block.size = std::clamp(block_apex, base, apex) - block.coor;
            };

            if constexpr (NESTED) clamp(dot_00, size       );
            else                  clamp(coor  , coor + size);

            return block;
        }
        twod clip(twod point) const
        {
            return std::clamp(point, coor, coor + std::max(dot_00, size - dot_11));
        }
        explicit operator bool()              const { return size.x != 0 && size.y != 0;       }
        auto   area           ()              const { return size.x * size.y;                  }
        twod   map            (twod const& p) const { return p - coor;                         }
        rect   shift          (twod const& p) const { return { coor + p, size };               }
        auto&  shift_itself   (twod const& p)       { coor += p; return *this;                 }
        rect   operator &     (rect const& r) const { return clip(r);                          }
        rect   operator +     (rect const& r) const { return { coor + r.coor, size + r.size }; }
        rect   operator -     (rect const& r) const { return { coor - r.coor, size - r.size }; }
        rect   operator |     (rect const& r) const { return unite(r);                         }
        bool   operator !=    (rect const& r) const { return coor != r.coor || size != r.size; }
        bool   operator ==    (rect const& r) const { return coor == r.coor && size == r.size; }
        void   operator +=    (rect const& r)       { coor += r.coor; size += r.size;          }
        void   operator -=    (rect const& r)       { coor -= r.coor; size -= r.size;          }

        // rect: Is the point inside the rect.
        bool hittest(twod const& p) const
        {
            bool test;
            if (size.x > 0)
            {
                auto t = p.x - coor.x;
                test = t >= 0 && t < size.x;
            }
            else
            {
                auto t = p.x + coor.x;
                test = t >= size.x && t < 0;
            }

            if (test)
            {
                if (size.y > 0)
                {
                    auto t = p.y - coor.y;
                    test = t >= 0 && t < size.y;
                }
                else
                {
                    auto t = p.y + coor.y;
                    test = t >= size.y && t < 0;
                }
                return test;
            }
            return faux;
        }
        rect rotate(twod const& dir) const
        {
            rect r;
            if ((dir.x ^ size.x) < 0)
            {
                r.coor.x = coor.x + size.x;
                r.size.x = -size.x;
            }
            else
            {
                r.coor.x = coor.x;
                r.size.x = size.x;
            }

            if ((dir.y ^ size.y) < 0)
            {
                r.coor.y =  coor.y + size.y;
                r.size.y = -size.y;
            }
            else
            {
                r.coor.y = coor.y;
                r.size.y = size.y;
            }
            return r;
        }
        rect normalize() const
        {
            rect r;
            if (size.x < 0)
            {
                r.coor.x =  coor.x + size.x;
                r.size.x = -size.x;
            }
            else
            {
                r.coor.x = coor.x;
                r.size.x = size.x;
            }

            if (size.y < 0)
            {
                r.coor.y =  coor.y + size.y;
                r.size.y = -size.y;
            }
            else
            {
                r.coor.y = coor.y;
                r.size.y = size.y;
            }

            return r;
        }
        auto& normalize_itself()
        {
            if (size.x < 0)
            {
                coor.x =  coor.x + size.x;
                size.x = -size.x;
            }
            else
            {
                coor.x = coor.x;
                size.x = size.x;
            }

            if (size.y < 0)
            {
                coor.y =  coor.y + size.y;
                size.y = -size.y;
            }
            else
            {
                coor.y = coor.y;
                size.y = size.y;
            }
            return *this;
        }
        // rect: Intersect the rect with rect{ dot_00, edge }.
        rect trunc(twod const& edge) const
        {
            rect r;
            r.coor = std::clamp(coor, dot_00, edge);
            r.size = std::clamp(size, -coor, edge - coor) + coor - r.coor;
            return r;
        }
        // rect: Return circumscribed rect.
        rect unite(rect const& annex) const
        {
            auto r1 = annex.normalize();
            auto r2 = normalize();
            auto tl = std::min(r1.coor, r2.coor);
            auto br = std::max(r1.coor + r1.size, r2.coor + r2.size );
            return { tl, br - tl};
        }
        // rect: Return true in case of normalized rectangles are overlapped.
        bool overlap(rect const& r) const
        {
            return coor.x          < r.coor.x + r.size.x
                && coor.y          < r.coor.y + r.size.y
                && coor.x + size.x > r.coor.x
                && coor.y + size.y > r.coor.y;
        }
        auto str() const
        {
            return "{" + coor.str() + ", " + size.str() + "}";
        }
        friend auto& operator<< (std::ostream& s, rect const& r)
        {
            return s << '{' << r.coor << ", " << r.size << '}';
        }
    };

    static constexpr const rect rect_00{ dot_00,dot_00 };

    // layout: A Parallelepiped, generated by three vectors.
    struct cube
    {
        twod delta;
        rect stuff;
    };

    // layout: A rectangle represented by the four values: Left x-coor, Right x-coor, Top y-coor, Bottom y-coor.
    struct side
    {
        iota l, r, t, b;

        constexpr side()
            : l(0), r(0), t(0), b(0)
        { }

        constexpr side(iota left, iota right = 0, iota top = 0, iota bottom = 0)
            : l(left), r(right), t(top), b(bottom)
        { }

        constexpr side(side const& s)
            : l(s.l), r(s.r), t(s.t), b(s.b)
        { }

        constexpr side(twod const& p)
            : l(p.x), r(p.x), t(p.y), b(p.y)
        { }

        constexpr side(rect const& r)
            : l(r.coor.x), r(r.coor.x + r.size.x),
              t(r.coor.y), b(r.coor.y + r.size.y)
        { }

        side(fifo& queue)
        {
            l = queue(0);
            r = queue(0);
            t = queue(0);
            b = queue(0);
        }
        // side: Unite the two rectangles.
        void operator |= (side const& s)
        {
            l = std::min(l, s.l);
            t = std::min(t, s.t);
            r = std::max(r, s.r);
            b = std::max(b, s.b);
        }
        // side: Unite the two rectangles (normalized).
        void operator |= (rect const& p)
        {
            l = std::min(l, p.coor.x);
            t = std::min(t, p.coor.y);
            r = std::max(r, p.coor.x + (p.size.x > 0 ? p.size.x - 1 : 0));
            b = std::max(b, p.coor.y + (p.size.y > 0 ? p.size.y - 1 : 0));
        }
        // side: Unite the two rectangles (0-based, normalized).
        void operator |= (twod const& p)
        {
            l = std::min(l, p.x);
            t = std::min(t, p.y);
            r = std::max(r, p.x);
            b = std::max(b, p.y);
        }
        // side: Shift rectangle by the twod.
        void operator += (twod const& p)
        {
            l += p.x;
            r += p.x;
            t += p.y;
            b += p.y;
        }
        // side: Shift rectangle by the twod.
        void operator -= (twod const& p)
        {
            l -= p.x;
            r -= p.x;
            t -= p.y;
            b -= p.y;
        }
        // side: Set paddings.
        void set(iota new_l, iota new_r = 0, iota new_t = 0, iota new_b = 0)
        {
            l = new_l;
            r = new_r;
            t = new_t;
            b = new_b;
        }
        // side: Set left and right pads.
        void set(std::pair<iota, iota> left_right)
        {
            set(left_right.first, left_right.second);
        }
        // side: Return top left corner.
        auto topleft() const { return twod{ l, t }; }

        // side: Margin summary size.
        auto summ() const { return twod{ l + r, t + b }; }
        // side: Return height.
        auto height() const { return b - t; }
        // side: Return width.
        auto width() const { return r - l; }
        // side: Return square.
        auto area() const { return rect{ { l, t }, { r - l, b - t } } ; }
        // side: Textify.
        auto str() const
        {
            return "{ l:" + std::to_string(l) + " r: " + std::to_string(r) +
                    " t:" + std::to_string(t) + " b: " + std::to_string(b) + " }";
        }
        friend auto& operator<< (std::ostream& s, side const& p)
        {
            return s << p.str();
        }
    };

    // layout: Padding, space around an element's content.
    struct dent
    {
        struct edge
        {
            bool just = faux;
            iota step = 0;

            constexpr edge(bool just, iota n = 0)
                : just { just          },
                  step { n }
            { }

            auto get(iota size) const
            {
                return just ? step : size - step;
            }
            edge& operator = (iota n)
            {
                step = n;
                return *this;
            }
            edge& operator = (edge const& e)
            {
                step = e.step;
                just = e.just;
                return *this;
            }
        };
        edge west;
        edge east;
        edge head;
        edge foot;

        constexpr dent()
            : west{ true },
              east{ faux },
              head{ true },
              foot{ faux }
        { }
        constexpr dent(iota w, iota e = 0, iota h = 0, iota f = 0)
            : west{ true, w },
              east{ faux, e },
              head{ true, h },
              foot{ faux, f }
        { }
        constexpr dent(dent const& pad)
            : west{ true, pad.west.step },
              east{ faux, pad.east.step },
              head{ true, pad.head.step },
              foot{ faux, pad.foot.step }
        { }

        auto& operator = (dent const& pad)
        {
            west = pad.west;
            east = pad.east;
            head = pad.head;
            foot = pad.foot;
            return *this;
        }
        auto& operator += (dent const& pad)
        {
            west.step += pad.west.step;
            east.step += pad.east.step;
            head.step += pad.head.step;
            foot.step += pad.foot.step;
            return *this;
        }
        // dent: Return inner area rectangle.
        auto area(iota size_x, iota size_y) const
        {
            //todo RTL
            auto w = west.get(size_x);
            auto h = head.get(size_y);
            auto e = east.get(size_x);
            auto f = foot.get(size_y);
            return rect{ {w, h}, {std::max(e - w, 0), std::max(f - h, 0)} };
        }
        // dent: Return inner area rectangle.
        auto area(twod const& size) const
        {
            return area(size.x, size.y);
        }
        // dent: Return inner area rectangle.
        auto area(rect const& content) const
        {
            rect field = area(content.size.x, content.size.y);
            field.coor += content.coor;
            return field;
        }
        // dent: Return the coor of the area rectangle.
        auto corner() const
        {
            return twod{ west.step,
                         head.step };
        }
        // dent: Return inner width.
        auto width(iota size_x) const
        {
            auto w = west.get(size_x);
            auto e = east.get(size_x);
            return std::max(e - w, 0);
        }
        // dent: Return inner height.
        auto height(iota size_y) const
        {
            auto h = head.get(size_y);
            auto f = foot.get(size_y);
            return std::max(f - h, 0);
        }
        // dent: Return size of the inner rectangle.
        auto size(twod const& size) const
        {
            return twod{ width(size.x), height(size.y) };
        }
        // dent: Return the horizontal coordinate using percentages.
        auto h_ratio(iota px, iota size_x) const
        {
            auto w = west.get(size_x);
            auto e = east.get(size_x);
            return divround(px * (std::max(e - w, 1) - 1), 100);
        }
        // dent: Return the vertical coordinate using percentages.
        auto v_ratio(iota py, iota size_y) const
        {
            auto h = head.get(size_y);
            auto f = foot.get(size_y);
            return divround(py * (std::max(f - h, 1) - 1), 100);
        }
        void reset()
        {
            west.step = 0;
            east.step = 0;
            head.step = 0;
            foot.step = 0;
        }
        void set(fifo& q)
        {
            west = q(0);
            east = q(0);
            head = q(0);
            foot = q(0);
        }
        // dent: Return size with padding.
        friend auto operator + (twod const& size, dent const& pad)
        {
            return twod{ std::max(0, size.x + (pad.west.step + pad.east.step)),
                         std::max(0, size.y + (pad.head.step + pad.foot.step)) };
        }
        // dent: Return size without padding.
        friend auto operator - (twod const& size, dent const& pad)
        {
            return twod{ std::max(0, size.x - (pad.west.step + pad.east.step)),
                         std::max(0, size.y - (pad.head.step + pad.foot.step)) };
        }
        // dent: Return area with padding.
        friend auto operator + (rect const& area, dent const& pad)
        {
            return rect{{ area.coor.x - pad.west.step,
                          area.coor.y - pad.head.step },
                        { std::max(0, area.size.x + (pad.west.step + pad.east.step)),
                          std::max(0, area.size.y + (pad.head.step + pad.foot.step)) }};
        }
        // dent: Return area without padding.
        friend auto operator - (rect const& area, dent const& pad)
        {
            return rect{ { area.coor.x + pad.west.step,
                           area.coor.y + pad.head.step },
                         { std::max(0, area.size.x - (pad.west.step + pad.east.step)),
                           std::max(0, area.size.y - (pad.head.step + pad.foot.step)) }};
        }
        // dent: Return summ of two paddings.
        friend auto operator + (dent const& pad1, dent const& pad2)
        {
            return dent{ pad1.west.step + pad2.west.step,
                         pad1.east.step + pad2.east.step,
                         pad1.head.step + pad2.head.step,
                         pad1.foot.step + pad2.foot.step };
        }
        // dent: Return diff of two paddings.
        friend auto operator - (dent const& pad1, dent const& pad2)
        {
            return dent{ pad1.west.step - pad2.west.step,
                         pad1.east.step - pad2.east.step,
                         pad1.head.step - pad2.head.step,
                         pad1.foot.step - pad2.foot.step };
        }
    };

    // layout: Scroll info.
    struct rack
    {
        twod region; // rack: Available scroll area.
        rect window; // rack: Scrolling viewport.
        side beyond; // rack: Scroll margins outside of the scroll region.
        // rack: Textify.
        auto str() const
        {
            return "{ reg:" + region.str() + " win:" + window.str() +
                    " ovr:" + beyond.str() + " }";
        }
        friend auto& operator<< (std::ostream& s, rack const& p)
        {
            return s << p.str();
        }
    };
}
namespace netxs::ui
{
    using rect = netxs::ui::atoms::rect;
}
#endif // NETXS_LAYOUT_HPP