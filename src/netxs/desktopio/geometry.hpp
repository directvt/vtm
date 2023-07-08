// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#include "utf.hpp"
#include "generics.hpp"

#if defined(__linux__) || defined(__APPLE__)
    #include <stdint.h>
#endif

namespace netxs
{
    using fifo = generics::fifo<si32>;

    // geometry: 2D point template.
    template<class T = int>
    struct duplet
    {
        using type = T;

        T x;
        T y;

        bool operator == (duplet const&) const = default;

        constexpr duplet()
            : x{ 0 },
              y{ 0 }
        { }

        constexpr duplet(T const& x, T const& y)
            : x{ x },
              y{ y }
        { }

        constexpr duplet(duplet const& p)
            : duplet{ p.x,
                      p.y }
        { }

        template<class D>
        constexpr duplet(duplet<D> const& d)
            : duplet{ static_cast<T>(d.x),
                      static_cast<T>(d.y) }
        { }

        constexpr duplet(fifo& queue)
            : x{ queue(0) },
              y{ queue(0) }
        { }

        constexpr T&       operator []  (int selector)          { return selector ? y : x;          }
        constexpr T const& operator []  (int selector) const    { return selector ? y : x;          }
        constexpr explicit operator bool() const                { return x != 0 || y != 0;          }
        constexpr duplet&  operator ++  ()                      { x++; y++;           return *this; }
        constexpr duplet&  operator --  ()                      { x--; y--;           return *this; }
        constexpr duplet&  operator =   (duplet const& p)       { x  = p.x; y  = p.y; return *this; }
        constexpr void     operator +=  (duplet const& p)       { x += p.x; y += p.y;               }
        constexpr void     operator -=  (duplet const& p)       { x -= p.x; y -= p.y;               }
        constexpr void     operator *=  (duplet const& p)       { x *= p.x; y *= p.y;               }
        constexpr void     operator /=  (duplet const& p)       { x /= p.x; y /= p.y;               }
        constexpr void     operator %=  (duplet const& p)       { x %= p.x; y %= p.y;               }
        constexpr void     operator -=  (T i)                   { x -=   i; y -=   i;               }
        constexpr void     operator +=  (T i)                   { x +=   i; y +=   i;               }
        constexpr void     operator *=  (T i)                   { x *=   i; y *=   i;               }
        constexpr void     operator /=  (T i)                   { x /=   i; y /=   i;               }
        constexpr void     operator %=  (T i)                   { x %=   i; y %=   i;               }
        constexpr bool     operator <   (T i) const             { return x < i && y < i;            }
        constexpr bool     operator >   (T i) const             { return x > i && y > i;            }
        constexpr duplet   operator +   (duplet const& p) const { return { x + p.x, y + p.y };      }
        constexpr duplet   operator -   (duplet const& p) const { return { x - p.x, y - p.y };      }
        constexpr duplet   operator *   (duplet const& p) const { return { x * p.x, y * p.y };      }
        constexpr duplet   operator /   (duplet const& p) const { return { x / p.x, y / p.y };      }
        constexpr duplet   operator %   (duplet const& p) const { return { x % p.x, y % p.y };      }
        constexpr duplet   operator -   ()                const { return {      -x,-y       };      }
        constexpr duplet   operator &   (T i)             const { return {   x & i, y & i   };      }
        constexpr duplet   operator ~   ()                const { return {       y, x       };      }

        ///In C++11, signed shift left of a negative number is always undefined
        //void operator>>= (T i) { x >>=i; y >>=i; }
        //void operator<<= (T i) { x <<=i; y <<=i; }

        ///In C++11, signed shift left of a negative number is always undefined
        //duplet operator << (T i) const { return { x << i, y << i }; }
        //duplet operator >> (T i) const { return { x >> i, y >> i }; }

        template<class D> duplet<D> constexpr operator / (D i) const { return { x / i, y / i }; }
        template<class D> duplet<D> constexpr operator + (D i) const { return { x + i, y + i }; }
        template<class D> duplet<D> constexpr operator - (D i) const { return { x - i, y - i }; }
        template<class D> duplet<D> constexpr operator * (D i) const { return { x * i, y * i }; }

        bool operator () (duplet const& p)
        {
            if (*this != p)
            {
                x = p.x;
                y = p.y;
                return true;
            }
            return faux;
        }
        duplet less(duplet const& what, duplet const& if_yes, duplet const& if_no) const
        {
            return { x < what.x ? if_yes.x : if_no.x,
                     y < what.y ? if_yes.y : if_no.y };
        }
        duplet equals(duplet const& what, duplet const& if_yes, duplet const& if_no) const
        {
            return { x == what.x ? if_yes.x : if_no.x,
                     y == what.y ? if_yes.y : if_no.y };
        }
        bool inside(duplet const& p) const
        {
            if (x > 0 ? (p.x >= 0 && p.x < x) : (p.x >= x && p.x < 0))
            {
                if (y > 0 ? (p.y >= 0 && p.y < y) : (p.y >= y && p.y < 0))
                {
                    return true;
                }
            }
            return faux;
        }

        duplet divround(type n)          const { return { netxs::divround(x, n  ), netxs::divround(y, n  ) }; }
        duplet divround(duplet const& p) const { return { netxs::divround(x, p.x), netxs::divround(y, p.y) }; }
        duplet divupper(duplet const& p) const { return { netxs::divupper(x, p.x), netxs::divupper(y, p.y) }; }

        auto str() const
        {
            return "{ " + std::to_string(x) + ", " + std::to_string(y) + " }";
        }
        friend auto& operator << (std::ostream& s, duplet const& p)
        {
            return s << "{ " << p.x << ", " << p.y << " }";
        }
        // Change endianness to LE.
        friend auto letoh(duplet const& p)
        {
            return duplet{ netxs::letoh(p.x), netxs::letoh(p.y) };
        }
        friend auto min  (duplet const& p1, duplet const& p2) { return duplet{ std::min(p1.x, p2.x), std::min(p1.y, p2.y) }; }
        friend auto max  (duplet const& p1, duplet const& p2) { return duplet{ std::max(p1.x, p2.x), std::max(p1.y, p2.y) }; }
        friend auto round(duplet const& p) { return duplet{ std::round(p.x), std::round(p.y) }; }
        friend auto abs  (duplet const& p) { return duplet{ std::  abs(p.x), std::  abs(p.y) }; }
        friend auto clamp(duplet const& p, duplet const& p1, duplet const& p2) { return duplet{ std::clamp(p.x, p1.x, p2.x), std::clamp(p.y, p1.y, p2.y) }; }
    };

    // geometry: 2D point.
    using twod = duplet<si32>;

    static constexpr const auto dot_00 = twod{ 0,0 };
    static constexpr const auto dot_01 = twod{ 0,1 };
    static constexpr const auto dot_10 = twod{ 1,0 };
    static constexpr const auto dot_11 = twod{ 1,1 };
    static constexpr const auto dot_22 = twod{ 2,2 };
    static constexpr const auto dot_21 = twod{ 2,1 };
    static constexpr const auto dot_33 = twod{ 3,3 };
    static constexpr const auto dot_mx = twod{ si32max / 2,
                                               si32max / 2 };

    static twod divround(twod const& p, si32 n       ) { return { divround(p.x, n  ), divround(p.y, n  ) }; }
    static twod divround(si32 n       , twod const& p) { return { divround(n  , p.x), divround(n  , p.y) }; }
    static twod divround(twod const& n, twod const& p) { return { divround(n.x, p.x), divround(n.y, p.y) }; }
    static twod divupper(twod const& n, twod const& p) { return { divupper(n.x, p.x), divupper(n.y, p.y) }; }
}

namespace std
{
    template<class T = netxs::si32> static netxs::duplet<T> min  (netxs::duplet<T> const& p1, netxs::duplet<T> const& p2) { return { std::min(p1.x, p2.x), std::min(p1.y, p2.y) }; }
    template<class T = netxs::si32> static netxs::duplet<T> max  (netxs::duplet<T> const& p1, netxs::duplet<T> const& p2) { return { std::max(p1.x, p2.x), std::max(p1.y, p2.y) }; }
    template<class T = netxs::si32> static netxs::duplet<T> round(netxs::duplet<T> const& p) { return { std::round(p.x), std::round(p.y) }; }
    template<class T = netxs::si32> static netxs::duplet<T> abs  (netxs::duplet<T> const& p) { return { std::  abs(p.x), std::  abs(p.y) }; }
    template<class T = netxs::si32> static netxs::duplet<T> clamp(netxs::duplet<T> const& p, netxs::duplet<T> const& p1, netxs::duplet<T> const& p2) { return { std::clamp(p.x, p1.x, p2.x), std::clamp(p.y, p1.y, p2.y) }; }
}

namespace netxs
{
    // geometry: Rectangle.
    struct rect
    {
        twod coor;
        twod size;

        // rect: Intersect two rects. If Nested==true when use dot_00 as a base corner.
        template<bool Nested = faux>
        constexpr
        rect clip(rect block) const
        {
            auto clamp = [&](auto const& base, auto const& apex)
            {
                auto block_apex = block.coor + block.size;
                block.coor = std::clamp(block.coor, base, apex);
                block.size = std::clamp(block_apex, base, apex) - block.coor;
            };

            if constexpr (Nested) clamp(dot_00, size       );
            else                  clamp(coor  , coor + size);

            return block;
        }
        twod clip(twod point) const
        {
            return std::clamp(point, coor, coor + std::max(dot_00, size - dot_11));
        }
        bool operator == (rect const&) const = default;
        explicit operator bool ()              const { return size.x != 0 && size.y != 0;       }
        auto   center          ()              const { return coor + size / 2;                  }
        auto   area            ()              const { return size.x * size.y;                  }
        twod   map             (twod const& p) const { return p - coor;                         }
        rect   shift           (twod const& p) const { return { coor + p, size };               }
        auto&  shift_itself    (twod const& p)       { coor += p; return *this;                 }
        rect   operator &      (rect const& r) const { return clip(r);                          }
        rect   operator |      (rect const& r) const { return unite(r);                         }
        void   operator +=     (rect const& r)       { coor += r.coor; size += r.size;          }
        void   operator -=     (rect const& r)       { coor -= r.coor; size -= r.size;          }

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
        friend auto& operator << (std::ostream& s, rect const& r)
        {
            return s << '{' << r.coor << ", " << r.size << '}';
        }
        // rect: Change endianness to LE.
        friend auto letoh(rect const& r)
        {
            return rect{ netxs::letoh(r.coor), netxs::letoh(r.size) };
        }
    };

    static constexpr auto rect_00 = rect{ dot_00,dot_00 };

    // geometry: A Parallelepiped, generated by three vectors.
    struct cube
    {
        twod delta;
        rect stuff;
    };

    // geometry: A rectangle represented by the four values: Left x-coor, Right x-coor, Top y-coor, Bottom y-coor.
    struct side
    {
        si32 l, r, t, b;

        constexpr side()
            : l(0), r(0), t(0), b(0)
        { }

        constexpr side(si32 left, si32 right = 0, si32 top = 0, si32 bottom = 0)
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

        bool operator == (side const&) const = default;

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
        void set(si32 new_l, si32 new_r = 0, si32 new_t = 0, si32 new_b = 0)
        {
            l = new_l;
            r = new_r;
            t = new_t;
            b = new_b;
        }
        // side: Set left and right pads.
        void set(std::pair<si32, si32> left_right)
        {
            set(left_right.first, left_right.second);
        }
        auto topleft() const { return twod{ l, t };                       }
        auto summ   () const { return twod{ l + r, t + b };               }
        auto vsumm  () const { return b + t;                              }
        auto hsumm  () const { return r + l;                              }
        auto height () const { return b - t;                              }
        auto width  () const { return r - l;                              }
        auto area   () const { return rect{ { l, t }, { r - l, b - t } }; }
        auto str    () const
        {
            return "{ l:" + std::to_string(l) + " r: " + std::to_string(r) +
                    " t:" + std::to_string(t) + " b: " + std::to_string(b) + " }";
        }
        friend auto& operator << (std::ostream& s, side const& p)
        {
            return s << p.str();
        }
        // side: Change endianness to LE.
        friend auto letoh(side const& s)
        {
            return side{ netxs::letoh(s.l),
                         netxs::letoh(s.r),
                         netxs::letoh(s.t),
                         netxs::letoh(s.b) };
        }
    };

    // geometry: Padding, space around an element's content.
    struct dent
    {
        template<auto Just>
        struct edge
        {
            si32 step = 0;
            bool operator == (edge const&) const = default;
            constexpr inline auto get(si32 size) const
            {
                if constexpr (Just) return step;
                else                return size - step;
            }
        };
        edge<true> west = {};
        edge<faux> east = {};
        edge<true> head = {};
        edge<faux> foot = {};

        dent() = default;
        constexpr dent(si32 w, si32 e = 0, si32 h = 0, si32 f = 0)
            : west{ w },
              east{ e },
              head{ h },
              foot{ f }
        { }
        constexpr dent(dent const& pad)
            : west{ pad.west.step },
              east{ pad.east.step },
              head{ pad.head.step },
              foot{ pad.foot.step }
        { }
        bool operator == (dent const&) const = default;
        explicit operator bool () const { return west.step != 0
                                              || east.step != 0
                                              || head.step != 0
                                              || foot.step != 0; }
        constexpr auto& operator = (dent const& pad)
        {
            west = pad.west;
            east = pad.east;
            head = pad.head;
            foot = pad.foot;
            return *this;
        }
        constexpr auto& operator -= (dent const& pad)
        {
            west.step -= pad.west.step;
            east.step -= pad.east.step;
            head.step -= pad.head.step;
            foot.step -= pad.foot.step;
            return *this;
        }
        constexpr auto& operator += (dent const& pad)
        {
            west.step += pad.west.step;
            east.step += pad.east.step;
            head.step += pad.head.step;
            foot.step += pad.foot.step;
            return *this;
        }
        // dent: Return inner area rectangle.
        constexpr auto area(si32 size_x, si32 size_y) const
        {
            //todo RTL
            auto w = west.get(size_x);
            auto h = head.get(size_y);
            auto e = east.get(size_x);
            auto f = foot.get(size_y);
            return rect{ {w, h}, {std::max(e - w, 0), std::max(f - h, 0)} };
        }
        // dent: Return inner area rectangle.
        constexpr auto area(twod const& size) const
        {
            return area(size.x, size.y);
        }
        // dent: Return inner area rectangle.
        constexpr auto area(rect const& content) const
        {
            auto field = area(content.size.x, content.size.y);
            field.coor += content.coor;
            return field;
        }
        // dent: Return the coor of the area rectangle.
        constexpr auto corner() const
        {
            return twod{ west.step,
                         head.step };
        }
        // dent: Return inner width.
        constexpr auto width(si32 size_x) const
        {
            auto w = west.get(size_x);
            auto e = east.get(size_x);
            return std::max(e - w, 0);
        }
        // dent: Return inner height.
        constexpr auto height(si32 size_y) const
        {
            auto h = head.get(size_y);
            auto f = foot.get(size_y);
            return std::max(f - h, 0);
        }
        // dent: Return size of the inner rectangle.
        constexpr auto size(twod const& size) const
        {
            return twod{ width(size.x), height(size.y) };
        }
        // dent: Return the horizontal coordinate using percentages.
        constexpr auto h_ratio(si32 px, si32 size_x) const
        {
            auto w = west.get(size_x);
            auto e = east.get(size_x);
            return divround(px * (std::max(e - w, 1) - 1), 100);
        }
        // dent: Return the vertical coordinate using percentages.
        constexpr auto v_ratio(si32 py, si32 size_y) const
        {
            auto h = head.get(size_y);
            auto f = foot.get(size_y);
            return divround(py * (std::max(f - h, 1) - 1), 100);
        }
        constexpr void reset()
        {
            west.step = 0;
            east.step = 0;
            head.step = 0;
            foot.step = 0;
        }
        void set(fifo& q)
        {
            west.step = q(0);
            east.step = q(0);
            head.step = q(0);
            foot.step = q(0);
        }
        // dent: Unary minus operator.
        constexpr auto operator - () const
        {
            return dent{ -west.step, -east.step, -head.step, -foot.step };
        }
        // dent: Scale padding.
        constexpr auto operator * (si32 const& factor) const
        {
            return dent{ west.step * factor, east.step * factor, head.step * factor, foot.step * factor };
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
        // dent: Return area with padding.
        friend auto operator += (rect& area, dent const& pad)
        {
            return area = area + pad;
        }
        // dent: Return area without padding.
        friend auto operator -= (rect& area, dent const& pad)
        {
            return area = area - pad;
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
        // dent: Change endianness to LE.
        friend auto letoh(dent const& d)
        {
            return dent{ netxs::letoh(d.west.step),
                         netxs::letoh(d.east.step),
                         netxs::letoh(d.head.step),
                         netxs::letoh(d.foot.step) };
        }
        friend auto& operator << (std::ostream& s, dent const& d)
        {
            return s << '{' << d.west.step << ','
                            << d.east.step << ','
                            << d.head.step << ','
                            << d.foot.step << ','
                            << '}';
        }
    };
    // dent: Return difference between area.
    auto operator - (rect const& r1, rect const& r2)
    {
        auto top = r2.coor - r1.coor;
        auto end = r1.size - r2.size - top;
        return dent{ top.x, end.x,
                     top.y, end.y };
    }

    // geometry: Scroll info.
    struct rack
    {
        twod region{}; // rack: Available scroll area.
        rect window{}; // rack: Scrolling viewport.
        side beyond{}; // rack: Scroll margins outside of the scroll region.
        si32 vector{}; // rack: Scroll direction.

        auto str() const
        {
            return "{ reg:" + region.str() + " win:" + window.str() +
                    " ovr:" + beyond.str() + " }";
        }
        friend auto& operator << (std::ostream& s, rack const& p)
        {
            return s << p.str();
        }
    };

    // geometry: Extract 1D length.
    template<class T>
    static inline si32 getlen(T p)
    {
        if constexpr (std::is_same_v<T, twod>) return p.x;
        else                                   return static_cast<si32>(p);
    }
    // geometry: Extract 2D size.
    template<class T>
    static inline rect getvol(T p)
    {
        if constexpr (std::is_same_v<T, twod>) return { dot_00, p };
        else                                   return { dot_00, { static_cast<si32>(p),  1 } };
    }
}