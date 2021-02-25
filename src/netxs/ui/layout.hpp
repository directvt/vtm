// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_LAYOUT_HPP
#define NETXS_LAYOUT_HPP

#include "../abstract/duplet.hpp"

namespace netxs::ui
{
    static const char whitespace = 0x20;
    //static const char whitespace = '.';

    enum tint
    {
        blackdk, reddk, greendk, yellowdk, bluedk, magentadk, cyandk, whitedk,
        blacklt, redlt, greenlt, yellowlt, bluelt, magentalt, cyanlt, whitelt,
    };

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

        bool operator == (rgba const& c) const
        {
            return token == c.token;
        }
        bool operator != (rgba const& c) const
        {
            return !operator == (c);
        }
        // rgba: Set all channels to zero.
        void wipe()
        {
            token = 0;
        }
        // rgba: Set the color to opaque black.
        void rst()
        {
            static constexpr uint32_t colorblack = 0xFF000000;

            token = colorblack;
        }
        // rgba: Are the colors alpha blenable? </summary>
        bool is_alpha_blendable() const
        {
            if (chan.a && chan.a != 0xFF)
            {
                return true;
            }
            return faux;
        }
        // rgba: Set the alpha channel.
        void alpha(uint8_t k)
        {
            chan.a = k;
        }
        // rgba: Return the alpha channel.
        uint8_t alpha() const
        {
            return chan.a;
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
        // rgba: Invert color.
        void invert()
        {
            token = (token & 0xFF000000) | ~(token & 0x00FFFFFF);
        }
        // rgba: Serialize color.
        auto str() const
        {
            return "{" + std::to_string(chan.r) + ","
                       + std::to_string(chan.g) + ","
                       + std::to_string(chan.b) + ","
                       + std::to_string(chan.a) + "}";
        }

        static constexpr uint32_t color256[] =
        {
            0xFF000000,	// black
            0xFF1F0FC4,	// red
            0xFF0EA112,	// green
            0xFF009CC0,	// yellow
            0xFFDB3700,	// blue
            0xFF981787,	// magenta
            0xFFDD963B,	// cyan
            0xFFBBBBBB,	// white
            0xFF757575,	// blacklt
            0xFF5648E6,	// redlt
            0xFF0CC615,	// greenlt
            0xFFA5F1F8,	// yellowlt
            0xFFFF783A,	// bluelt
            0xFF9E00B3,	// magentalt
            0xFFD6D660,	// cyanlt
            0xFFF3F3F3,	// whitelt
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
        friend std::ostream& operator << (std::ostream& s, rgba const& c)
        {
            return s << "{" + std::to_string(c.chan.r)
                      + "," + std::to_string(c.chan.g)
                      + "," + std::to_string(c.chan.b)
                      + "," + std::to_string(c.chan.a)
                      + "}";
        }
    };

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
            return irgb<T>{ r / v, g / v, b / v }; // 10% faster than divround

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

    enum bias
    {
        left, right, center,
    };

    struct rect
    {
        twod coor;
        twod size;

        // rect: Intersect two rects. If NESTED==true when use dot_00 as a base corner.
        template<bool NESTED = faux>
        constexpr
        rect clip (rect block) const
        {
            auto clamp = [&](auto const& base, auto const& apex)
            {
                auto block_apex = block.coor + block.size;
                block.coor = std::clamp(block.coor, base, apex);
                block.size = std::clamp(block_apex, base, apex) - block.coor;
            };

            if constexpr (NESTED) clamp(dot_00, size);
            else                  clamp(coor,   coor + size);

            return block;
        }

        operator bool     ()              const { return size.x != 0 && size.y != 0;       }
        auto   area       ()              const { return size.x * size.y;                  }
        twod   map        (twod const& p) const { return p - coor;                         }
        rect   operator & (rect const& r) const { return clip(r);                          }
        rect   operator + (rect const& r) const { return { coor + r.coor, size + r.size }; }
        rect   operator - (rect const& r) const { return { coor - r.coor, size - r.size }; }
        rect   operator | (rect const& r) const { return unite(r);                         }
        bool   operator!= (rect const& r) const { return coor != r.coor || size != r.size; }
        bool   operator== (rect const& r) const { return coor == r.coor && size == r.size; }

        // rect: Is the point inside the rect.
        bool hittest (twod const& p) const
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
        rect rotate (twod const& dir) const
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
        rect normalize () const
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
        // rect: Intersect the rect with rect{ dot_00, edge }.
        rect trunc (twod const& edge) const
        {
            rect r;
            r.coor = std::clamp(coor, dot_00, edge);
            r.size = std::clamp(size, -coor, edge - coor) + coor - r.coor;
            return r;
        }
        // rect: Return circumscribed rect.
        rect unite (rect const& annex) const
        {
            auto r1 = annex.normalize();
            auto r2 = normalize();

            auto tl = std::min(r1.coor, r2.coor);
            auto br = std::max(r1.coor + r1.size, r2.coor + r2.size );

            return { tl, br - tl};
        }
        // rect: Return true in case of normalized rectangles are overlapped.
        bool overlap (rect const& r) const
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
        friend std::ostream& operator << (std::ostream& s, rect const& r)
        {
            return s << r.str();
        }
    };

    static constexpr const rect rect_00{ dot_00,dot_00 };

    // layout: The Parallelepiped, generated by three vectors.
    struct cube
    {
        twod delta;
        rect stuff;
    };

    // layout: The rectangle represented by the four values: Left x-coor, Right x-coor, Top y-coor, Bottom y-coor.
    struct side
    {
        iota l, r, t, b;

        constexpr side ()
            : l(0), r(0), t(0), b(0)
        { }

        constexpr side (iota left, iota right = 0, iota top = 0, iota bottom = 0)
            : l(left), r(right), t(top), b(bottom)
        { }

        constexpr side (side const& s)
            : l(s.l), r(s.r), t(s.t), b(s.b)
        { }

        constexpr side (twod const& p)
            : l(p.x), r(p.x), t(p.y), b(p.y)
        { }

        constexpr side (rect const& r)
            : l(r.coor.x), r(r.coor.x + r.size.x),
              t(r.coor.y), b(r.coor.y + r.size.y)
        { }

        side (fifo& queue)
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
        // side: Set and return true if changed.
        void set(iota new_l, iota new_r = 0, iota new_t = 0, iota new_b = 0)
        {
            l = new_l;
            r = new_r;
            t = new_t;
            b = new_b;

            //if (l != new_l ||
            //	r != new_r ||
            //	t != new_t ||
            //	b != new_b)
            //{
            //	l = new_l;
            //	r = new_r;
            //	t = new_t;
            //	b = new_b;
            //	return true;
            //}
            //else return faux;
        }
        // side: Return top left corner.
        auto topleft() const { return twod{ l, t }; }

        // side: Margin summary size.
        auto summ() const { return twod{ l + r, t + b }; }
        // side: Return height.
        auto height() const { return b - t; }
        // side: Return width.
        auto width() const { return r - l; }
        // side: Textify
        auto str() const
        {
            return "{ l:" + std::to_string(l) + " r: " + std::to_string(r) +
                    " t:" + std::to_string(t) + " b: " + std::to_string(b) + " }";
        }
        friend std::ostream& operator << (std::ostream& s, side const& p)
        {
            return s << p.str();
        }
    };

    struct dent
    {
        class edge
        {
            iota const& size;

        public:
            iota step = 0;
            bool just = faux;
            bool flip = faux;

            constexpr edge(iota const& size, bool just)
                : size { size },
                  just { just },
                  flip { faux },
                  step { 0    }
            { }

            operator iota () const
            {
                return (just != flip) ? step : size - step;
            }
            edge& operator = (iota n)
            {
                step = (flip = n < 0) ? -n : n;
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

        constexpr
        dent(iota const& size_x, iota const& size_y)
            : west (size_x, true),
              east (size_x, faux),
              head (size_y, true),
              foot (size_y, faux)
        { }

        dent& operator = (dent const& margin)
        {
            west = margin.west;
            east = margin.east;
            head = margin.head;
            foot = margin.foot;
            return *this;
        }
        // dent: Return inner area rectangle.
        operator rect () const
        {
            //todo RTL
            iota w = west;
            iota h = head;
            iota e = east;
            iota f = foot;
            return { {w, h}, {std::max(e - w, 0), std::max(f - h, 0)} };
        }
        // dent: Return the coor of the area rectangle.
        twod coor() const
        {
            return twod{ west,head };
        }
        // dent: Return inner width.
        iota width() const
        {
            iota w = west;
            iota e = east;
            return std::max(e - w, 0);
        }
        // dent: Return inner height.
        iota height() const
        {
            iota h = head;
            iota f = foot;
            return std::max(f - h, 0);
        }
        // dent: Return the horizontal coordinate using percentages.
        iota h_ratio(iota px) const
        {
            iota w = west;
            iota e = east;
            return divround(px * (std::max(e - w, 1) - 1), 100);
        }
        // dent: Return the vertical coordinate using percentages.
        iota v_ratio(iota py) const
        {
            iota h = head;
            iota f = foot;
            return divround(py * (std::max(f - h, 1) - 1), 100);
        }

        //void set_size(twod const& formsize)
        //{
        //	west.set(formsize.x, true);
        //	east.set(formsize.x, faux);
        //	head.set(formsize.y, true);
        //	foot.set(formsize.y, faux);
        //}
    };

    struct rack // scroll info
    {
        twod region; // rack: Available scroll area
        rect window; // rack: Scrolling viewport
        side beyond; // rack: Scroll margins outside of the scroll region
    };

    //struct limits
    //{
    //	twod min = { 0, 0 };
    //	twod max = { maxval(), maxval() };
    //
    //	static int maxval() { return 0xFFFF; }
    //
    //	bool set(limits newlimits)
    //	{
    //		bool result = faux;
    //		if (max != newlimits.max)
    //		{
    //			max.x = std::clamp(newlimits.max.x, 0, maxval());
    //			max.y = std::clamp(newlimits.max.y, 0, maxval());
    //			result = true;
    //		}
    //		if (min != newlimits.min)
    //		{
    //			min.x = std::clamp(newlimits.min.x, 0, maxval());
    //			min.y = std::clamp(newlimits.min.y, 0, maxval());
    //			result = true;
    //		}
    //		return result;
    //	}
    //	void apply(int& x, int& y)
    //	{
    //		x = std::clamp(x, min.x, max.x);
    //		y = std::clamp(y, min.y, max.y);
    //	}
    //};
    //
    //enum class orientation	{ horizontal, vertical };
    //
    //struct resource
    //{
    //	enum type
    //	{
    //		undefined,
    //		accel,
    //		icon,
    //		font,
    //		menu,
    //		imagelist,
    //		cstring,
    //		ptr,
    //	};
    //
    //	resource	()						 : spec(type::undefined) { }
    //	resource	(type spec)				 : spec(spec)			 { }
    //	resource	(void * hndl, type spec) : spec(spec)			 { rsrc = handle{ hndl, null_deleter() }; }
    //
    //	virtual ~resource()	{ }
    //
    //	template<class T>
    //	operator T ();
    //
    //protected:
    //	using handle = std::shared_ptr<void>;
    //
    //	handle	rsrc;
    //	type	spec;
    //};
    //
    //struct rsrcpack_i
    //{
    //	virtual ~rsrcpack_i() { }
    //	virtual std::vector<resource> get(int selector, int filter = -1) = 0;
    //};
    //
    //using rsrcpack = std::shared_ptr<rsrcpack_i>;
    //using rcidlist = std::vector<std::pair<int, std::vector<utils::cstr>>>;
    //
    //utils::cstr	load_string		(int string_id,					utils::intptr instance = 0);
    //resource	load_imagelist	(std::vector<resource> const& icons);
    //resource	load_icon		(utils::cstr const& icon_id,	utils::intptr instance = 0);
    //resource	load_accl		(utils::cstr const& accl_id,	utils::intptr instance = 0);
    //resource	load_cursor		(utils::cstr const& curs_id,	utils::intptr instance = 0);
    //resource	load_menu		(utils::cstr const& menu_id,	utils::intptr instance = 0);
    //resource	load_font		(utils::cstr const& fontname = utils::cstr(), uint32_t pitch = 0, int height = 0, int width = 0);
    //rsrcpack	load_iconpack	(utils::intptr instance, rcidlist const& iconlist);
    //resource	load_cstr		(utils::cstr const& txt = 0);
    //resource	load_ptr		(void * ptr = nullptr);
}

#endif // NETXS_LAYOUT_HPP