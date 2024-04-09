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
    template<class T = si32>
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
            : duplet{ p.x, p.y }
        { }

        template<class D>
        constexpr duplet(duplet<D> const& d)
            : duplet{ static_cast<T>(d.x), static_cast<T>(d.y) }
        { }

        constexpr duplet(fifo& queue)
            : x{ queue(0) },
              y{ queue(0) }
        { }

        constexpr T&       operator []  (si32 selector)         { return selector ? y : x;          }
        constexpr T const& operator []  (si32 selector) const   { return selector ? y : x;          }
        constexpr explicit operator bool() const                { return x != 0 || y != 0;          }
        constexpr duplet&  operator ++  ()                      { x++; y++;           return *this; }
        constexpr duplet&  operator --  ()                      { x--; y--;           return *this; }
        constexpr duplet&  operator =   (duplet const& p)       { x  = p.x; y  = p.y; return *this; }
        constexpr void     operator +=  (duplet const& p)       { x += p.x; y += p.y;               }
        constexpr void     operator -=  (duplet const& p)       { x -= p.x; y -= p.y;               }
        constexpr void     operator *=  (duplet const& p)       { x *= p.x; y *= p.y;               }
        constexpr void     operator /=  (duplet const& p)       { x /= p.x; y /= p.y;               }
        constexpr void     operator %=  (duplet const& p)       { x %= p.x; y %= p.y;               }
        constexpr void     operator -=  (T i)                   { x -= i;   y -= i;                 }
        constexpr void     operator +=  (T i)                   { x += i;   y += i;                 }
        constexpr void     operator *=  (T i)                   { x *= i;   y *= i;                 }
        constexpr void     operator /=  (T i)                   { x /= i;   y /= i;                 }
        constexpr void     operator %=  (T i)                   { x %= i;   y %= i;                 }
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

        template<class D> auto constexpr operator / (D i) const { return duplet<D>{ x / i, y / i }; }
        template<class D> auto constexpr operator + (D i) const { return duplet<D>{ x + i, y + i }; }
        template<class D> auto constexpr operator - (D i) const { return duplet<D>{ x - i, y - i }; }
        template<class D> auto constexpr operator * (D i) const { return duplet<D>{ x * i, y * i }; }

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
        duplet less(T const& what, duplet const& if_yes, duplet const& if_no) const
        {
            return { x < what ? if_yes.x : if_no.x,
                     y < what ? if_yes.y : if_no.y };
        }
        duplet equals(T const& what, duplet const& if_yes, duplet const& if_no) const
        {
            return { x == what ? if_yes.x : if_no.x,
                     y == what ? if_yes.y : if_no.y };
        }
        duplet less(T const& what, T const& if_yes, T const& if_no) const
        {
            return { x < what ? if_yes : if_no,
                     y < what ? if_yes : if_no };
        }
        duplet equals(T const& what, T const& if_yes, T const& if_no) const
        {
            return { x == what ? if_yes : if_no,
                     y == what ? if_yes : if_no };
        }
        bool inside(duplet const& p) const
        {
            return (x > 0 ? (p.x >= 0 && p.x < x) : (p.x >= x && p.x < 0))
                && (y > 0 ? (p.y >= 0 && p.y < y) : (p.y >= y && p.y < 0));
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
        friend auto   min(duplet const& p1, duplet const& p2) { return duplet{ std::min(p1.x, p2.x), std::min(p1.y, p2.y) }; }
        friend auto   max(duplet const& p1, duplet const& p2) { return duplet{ std::max(p1.x, p2.x), std::max(p1.y, p2.y) }; }
        friend auto round(duplet const& p)                    { return duplet{ std::round(p.x), std::round(p.y) }; }
        friend auto   abs(duplet const& p)                    { return duplet{ std::abs(p.x), std::abs(p.y) }; }
        friend auto clamp(duplet const& p, duplet const& p1, duplet const& p2)
        {
            return duplet{ std::clamp(p.x, p1.x, p2.x),
                           std::clamp(p.y, p1.y, p2.y) };
        }
        static constexpr auto sort(duplet p1, duplet p2)
        {
            if (p1.x > p2.x) std::swap(p1.x, p2.x);
            if (p1.y > p2.y) std::swap(p1.y, p2.y);
            return std::pair{ p1, p2 };
        }
    };

    // geometry: 2D point.
    using twod = duplet<si32>;

    static constexpr auto dot_00 = twod{ 0,0 };
    static constexpr auto dot_01 = twod{ 0,1 };
    static constexpr auto dot_10 = twod{ 1,0 };
    static constexpr auto dot_11 = twod{ 1,1 };
    static constexpr auto dot_22 = twod{ 2,2 };
    static constexpr auto dot_21 = twod{ 2,1 };
    static constexpr auto dot_33 = twod{ 3,3 };
    static constexpr auto dot_mx = twod{ si32max / 2, si32max / 2 };

    twod divround(twod p, si32 n) { return { divround(p.x, n  ), divround(p.y, n  ) }; }
    twod divround(si32 n, twod p) { return { divround(n  , p.x), divround(n  , p.y) }; }
    twod divround(twod n, twod p) { return { divround(n.x, p.x), divround(n.y, p.y) }; }
    twod divupper(twod n, twod p) { return { divupper(n.x, p.x), divupper(n.y, p.y) }; }
}

namespace std
{
    template<class T = netxs::si32> constexpr netxs::duplet<T>   min(netxs::duplet<T> const& p1, netxs::duplet<T> const& p2) { return { std::min(p1.x, p2.x), std::min(p1.y, p2.y) }; }
    template<class T = netxs::si32> constexpr netxs::duplet<T>   max(netxs::duplet<T> const& p1, netxs::duplet<T> const& p2) { return { std::max(p1.x, p2.x), std::max(p1.y, p2.y) }; }
    template<class T = netxs::si32> constexpr netxs::duplet<T> round(netxs::duplet<T> const& p)                              { return { std::round(p.x), std::round(p.y) }; }
    template<class T = netxs::si32> constexpr netxs::duplet<T>   abs(netxs::duplet<T> const& p)                              { return { std::abs(p.x), std::abs(p.y) }; }
    template<class T = netxs::si32> constexpr netxs::duplet<T> clamp(netxs::duplet<T> const& p, netxs::duplet<T> const& p1, netxs::duplet<T> const& p2) { return { std::clamp(p.x, p1.x, p2.x), std::clamp(p.y, p1.y, p2.y) }; }
}

namespace netxs
{
    // geometry: Rectangle.
    struct rect
    {
        twod coor;
        twod size;

        bool operator == (rect const&) const = default;
        explicit operator bool ()       const { return size.x != 0 && size.y != 0;            }
        auto   center          ()       const { return coor + size / 2;                       }
        auto   area            ()       const { return size.x * size.y;                       }
        twod   map             (twod p) const { return p - coor;                              }
        rect   shift           (twod p) const { return { coor + p, size };                    }
        auto&  shift_itself    (twod p)       { coor += p; return *this;                      }
        rect   operator |      (rect r) const { return unite(r, *this);                       }
        auto&  operator +=     (rect r)       { coor += r.coor; size += r.size; return *this; }
        auto&  operator -=     (rect r)       { coor -= r.coor; size -= r.size; return *this; }

        // rect: Return rect trimmed by r.
        template<bool Relative = faux>
        constexpr rect trim(rect r) const
        {
            if constexpr (Relative) r.coor += coor;
            auto r_apex = r.coor + r.size;
            auto [min, max] = twod::sort(coor, coor + size);
            r.coor = std::clamp(r.coor, min, max);
            r.size = std::clamp(r_apex, min, max) - r.coor;
            return r;
        }
        // rect: Trim by the specified rect.
        template<bool Relative = faux>
        constexpr auto& trimby(rect r)
        {
            if constexpr (Relative) coor += r.coor;
            auto apex = coor + size;
            auto [min, max] = twod::sort(r.coor, r.coor + r.size);
            coor = std::clamp(coor, min, max);
            size = std::clamp(apex, min, max) - coor;
            return *this;
        }
        // rect: Return clamped point.
        constexpr twod clamp(twod point) const
        {
            auto [min, max] = twod::sort(coor, coor + size);
            return std::clamp(point, min, max - dot_11);
        }
        // rect: Is the point inside the rect.
        constexpr bool hittest(twod p) const
        {
            auto test = faux;
            if (size.x > 0) { auto t = p.x - coor.x; test = t >= 0 && t < size.x; }
            else            { auto t = p.x + coor.x; test = t >= size.x && t < 0; }

            if (test)
            {
                if (size.y > 0) { auto t = p.y - coor.y; test = t >= 0 && t < size.y; }
                else            { auto t = p.y + coor.y; test = t >= size.y && t < 0; }
            }
            return test;
        }
        // rect: Return rect with specified orientation.
        constexpr rect rotate(twod dir) const
        {
            auto sx = (dir.x ^ size.x) < 0;
            auto sy = (dir.y ^ size.y) < 0;
            return {{ sx ?  coor.x + size.x : coor.x, sy ?  coor.y + size.y : coor.y },
                    { sx ? -size.x          : size.x, sy ? -size.y          : size.y }};
        }
        // rect: Change orientation.
        constexpr auto& rotate_itself(twod dir)
        {
            if ((dir.x ^ size.x) < 0) { coor.x += size.x; size.x = -size.x; }
            if ((dir.y ^ size.y) < 0) { coor.y += size.y; size.y = -size.y; }
            return *this;
        }
        // rect: Return rect with top-left orientation.
        constexpr rect normalize() const
        {
            auto sx = size.x < 0;
            auto sy = size.y < 0;
            return {{ sx ?  coor.x + size.x : coor.x, sy ?  coor.y + size.y : coor.y },
                    { sx ? -size.x : size.x         , sy ? -size.y : size.y          }};
        }
        // rect: Set top-left orientation.
        constexpr auto& normalize_itself()
        {
            if (size.x < 0) { coor.x += size.x; size.x = -size.x; }
            if (size.y < 0) { coor.y += size.y; size.y = -size.y; }
            return *this;
        }
        // rect: Intersect the rect with rect{ dot_00, edge }.
        constexpr rect trunc(twod edge) const
        {
            auto r = rect{};
            r.coor = std::clamp(coor, dot_00, edge);
            r.size = std::clamp(size, -coor, edge - coor) + coor - r.coor;
            return r;
        }
        // rect: Return circumscribed rect.
        static constexpr rect unite(rect r1, rect r2)
        {
            r1.normalize_itself();
            r2.normalize_itself();
            auto tl = std::min(r1.coor, r2.coor);
            auto br = std::max(r1.coor + r1.size, r2.coor + r2.size );
            return { tl, br - tl};
        }
        // rect: Return true in case of normalized rectangles are overlapped.
        constexpr bool overlap(rect r) const
        {
            return coor.x          < r.coor.x + r.size.x
                && coor.y          < r.coor.y + r.size.y
                && coor.x + size.x > r.coor.x
                && coor.y + size.y > r.coor.y;
        }
        // rect: To string.
        auto str() const
        {
            return "{" + coor.str() + ", " + size.str() + "}";
        }
        friend auto& operator << (std::ostream& s, rect r)
        {
            return s << '{' << r.coor << ", " << r.size << '}';
        }
        // rect: Change endianness to LE.
        friend auto letoh(rect r)
        {
            return rect{ netxs::letoh(r.coor), netxs::letoh(r.size) };
        }
    };

    static constexpr auto rect_00 = rect{ dot_00,dot_00 };
    static constexpr auto rect_11 = rect{ dot_00,dot_11 };

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

        constexpr side(si32 l = 0, si32 r = 0, si32 t = 0, si32 b = 0)
            : l{ l }, r{ r }, t{ t }, b{ b }
        { }
        constexpr side(side const&) = default;
        constexpr side(twod p)
            : l{ p.x }, r{ p.x }, t{ p.y }, b{ p.y }
        { }
        constexpr side(rect a)
            : l{ a.coor.x }, r{ a.coor.x + a.size.x },
              t{ a.coor.y }, b{ a.coor.y + a.size.y }
        { }
        side(fifo& queue)
        {
            l = queue(0);
            r = queue(0);
            t = queue(0);
            b = queue(0);
        }
        constexpr side& operator = (side const&) = default;
        bool operator == (side const&) const = default;
        // side: Unite the two rectangles.
        void operator |= (side s)
        {
            l = std::min(l, s.l);
            t = std::min(t, s.t);
            r = std::max(r, s.r);
            b = std::max(b, s.b);
        }
        // side: Unite the two rectangles (normalized).
        void operator |= (rect a)
        {
            l = std::min(l, a.coor.x);
            t = std::min(t, a.coor.y);
            r = std::max(r, a.coor.x + (a.size.x > 0 ? a.size.x - 1 : 0));
            b = std::max(b, a.coor.y + (a.size.y > 0 ? a.size.y - 1 : 0));
        }
        // side: Unite the two rectangles (0-based, normalized).
        void operator |= (twod p)
        {
            l = std::min(l, p.x);
            t = std::min(t, p.y);
            r = std::max(r, p.x);
            b = std::max(b, p.y);
        }
        // side: Shift rectangle by the twod.
        void operator += (twod p)
        {
            l += p.x;
            r += p.x;
            t += p.y;
            b += p.y;
        }
        // side: Shift rectangle by the twod.
        void operator -= (twod p)
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
        auto height() const { return b - t; }
        auto width()  const { return r - l; }
        auto area()   const { return rect{{ l, t }, { r - l, b - t }}; }
        auto str() const
        {
            return "{ l:" + std::to_string(l) + " r: " + std::to_string(r) +
                    " t:" + std::to_string(t) + " b: " + std::to_string(b) + " }";
        }
        friend auto& operator << (std::ostream& s, side p)
        {
            return s << p.str();
        }
        // side: Change endianness to LE.
        friend auto letoh(side s)
        {
            return side{ netxs::letoh(s.l),
                         netxs::letoh(s.r),
                         netxs::letoh(s.t),
                         netxs::letoh(s.b) };
        }
    };

    // geometry: Padding, space around object.
    struct dent
    {
        si32 l, r, t, b;

        constexpr dent(si32 l = 0, si32 r = 0, si32 t = 0, si32 b = 0)
            : l{ l }, r{ r }, t{ t }, b{ b }
        { }
        constexpr dent(dent const&) = default;
        constexpr dent& operator = (dent const&) = default;
        constexpr bool operator == (dent const&) const = default;
        explicit operator bool () const { return l != 0
                                              || r != 0
                                              || t != 0
                                              || b != 0; }
        constexpr auto& operator -= (dent pad)
        {
            l -= pad.l;
            r -= pad.r;
            t -= pad.t;
            b -= pad.b;
            return *this;
        }
        constexpr auto& operator += (dent pad)
        {
            l += pad.l;
            r += pad.r;
            t += pad.t;
            b += pad.b;
            return *this;
        }
        // dent: Return inner area rectangle.
        constexpr auto area(si32 size_x, si32 size_y) const
        {
            //todo RTL
            return rect{{ l, t }, { std::max(0, size_x - (r + l)), std::max(0, size_y - (b + t)) }};
        }
        // dent: Return inner area rectangle.
        constexpr auto area(twod size) const
        {
            return area(size.x, size.y);
        }
        // dent: Return inner area rectangle.
        constexpr auto area(rect content) const
        {
            auto field = area(content.size.x, content.size.y);
            field.coor += content.coor;
            return field;
        }
        // dent: Return the coor of the area rectangle.
        constexpr auto corner() const
        {
            return twod{ l, t };
        }
        // dent: Return the coor of the area rectangle.
        constexpr auto coor(twod c) const
        {
            return twod{ c.x - l, c.y - t };
        }
        // dent: Return inner width.
        constexpr auto width(si32 size_x) const
        {
            return std::max(0, size_x - (r + l));
        }
        // dent: Return inner height.
        constexpr auto height(si32 size_y) const
        {
            return std::max(0, size_y - (b + t));
        }
        // dent: Return size of the inner rectangle.
        constexpr auto size(twod size) const
        {
            return twod{ width(size.x), height(size.y) };
        }
        // dent: Return the horizontal coordinate using percentages.
        constexpr auto h_ratio(si32 px, si32 size_x) const
        {
            return divround(px * (std::max(1, size_x - (r + l)) - 1), 100);
        }
        // dent: Return the vertical coordinate using percentages.
        constexpr auto v_ratio(si32 py, si32 size_y) const
        {
            return divround(py * (std::max(1, size_y - (b + t)) - 1), 100);
        }
        void set(fifo& q)
        {
            l = q(0);
            r = q(0);
            t = q(0);
            b = q(0);
        }
        // dent: Unary minus operator.
        constexpr auto operator - () const
        {
            return dent{ -l, -r, -t, -b };
        }
        // dent: Scale padding.
        constexpr auto operator * (si32 factor) const
        {
            return dent{ l * factor, r * factor, t * factor, b * factor };
        }
        // dent: Return size with padding.
        friend auto operator + (twod size, dent pad)
        {
            return twod{ std::max(0, size.x + (pad.l + pad.r)),
                         std::max(0, size.y + (pad.t + pad.b)) };
        }
        // dent: Return size without padding.
        friend auto operator - (twod size, dent pad)
        {
            return twod{ std::max(0, size.x - (pad.l + pad.r)),
                         std::max(0, size.y - (pad.t + pad.b)) };
        }
        // dent: Return area with padding.
        friend auto operator + (rect area, dent pad)
        {
            if (area.size.x < 0) { area.coor.x += pad.l; area.size.x -= pad.l + pad.r; }
            else                 { area.coor.x -= pad.l; area.size.x += pad.l + pad.r; }
            if (area.size.y < 0) { area.coor.y += pad.t; area.size.y -= pad.t + pad.b; }
            else                 { area.coor.y -= pad.t; area.size.y += pad.t + pad.b; }
            return area;
        }
        // dent: Return area without padding.
        friend auto operator - (rect area, dent pad)
        {
            if (area.size.x < 0) { area.coor.x -= pad.l; area.size.x += pad.l + pad.r; }
            else                 { area.coor.x += pad.l; area.size.x -= pad.l + pad.r; }
            if (area.size.y < 0) { area.coor.y -= pad.t; area.size.y += pad.t + pad.b; }
            else                 { area.coor.y += pad.t; area.size.y -= pad.t + pad.b; }
            return area;
        }
        // dent: Return area with padding.
        friend auto operator += (rect& area, dent pad)
        {
            return area = area + pad;
        }
        // dent: Return area without padding.
        friend auto operator -= (rect& area, dent pad)
        {
            return area = area - pad;
        }
        // dent: Return size with padding.
        friend auto operator += (twod& size, dent pad)
        {
            return size = size + pad;
        }
        // dent: Return size without padding.
        friend auto operator -= (twod& size, dent pad)
        {
            return size = size - pad;
        }
        // dent: Return summ of two paddings.
        friend auto operator + (dent pad1, dent pad2)
        {
            pad1.l += pad2.l;
            pad1.r += pad2.r;
            pad1.t += pad2.t;
            pad1.b += pad2.b;
            return pad1;
        }
        // dent: Return diff of two paddings.
        friend auto operator - (dent pad1, dent pad2)
        {
            pad1.l -= pad2.l;
            pad1.r -= pad2.r;
            pad1.t -= pad2.t;
            pad1.b -= pad2.b;
            return pad1;
        }
        // dent: Change endianness to LE.
        friend auto letoh(dent d)
        {
            return dent{ netxs::letoh(d.l),
                         netxs::letoh(d.r),
                         netxs::letoh(d.t),
                         netxs::letoh(d.b) };
        }
        auto str() const
        {
            return '{' + std::to_string(l) + ','
                       + std::to_string(r) + ','
                       + std::to_string(t) + ','
                       + std::to_string(b) + '}';
        }
        friend auto& operator << (std::ostream& s, dent d)
        {
            return s << d.str();
        }
        friend auto min(dent d1, dent d2) { return dent{ std::min(d1.l, d2.l), std::min(d1.r, d2.r), std::min(d1.t, d2.t), std::min(d1.b, d2.b) }; }
        friend auto max(dent d1, dent d2) { return dent{ std::max(d1.l, d2.l), std::max(d1.r, d2.r), std::max(d1.t, d2.t), std::max(d1.b, d2.b) }; }
    };
    // dent: Return difference between area.
    auto operator - (rect r1, rect r2)
    {
        auto top = r2.coor - r1.coor;
        auto end = r1.size - r2.size - top;
        return dent{ top.x, end.x,
                     top.y, end.y };
    }

    // geometry: Scroll info.
    struct rack
    {
        twod region; // rack: Available scroll area.
        rect window; // rack: Scrolling viewport.
        dent beyond; // rack: Scroll margins outside of the scroll region.
        twod vector; // rack: Scroll direction.

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

namespace netxs::misc //todo classify
{
    template<class T, auto fx>
    struct shadow
    {
        T  bitmap{};
        bool sync{};
        twod over{};
        twod step{};

        void generate(fp32 bias, fp32 alfa, si32 size, twod offset, twod ratio, auto fuse)
        {
            //bias    += _k0 * 0.1f;
            //opacity += _k1 * 1.f;
            //size    += _k2;
            //offset  +=  ratio * _k3;
            sync = true;
            alfa = std::clamp(alfa, 0.f, 255.f);
            size = std::abs(size) * 2;
            over = ratio * (size * 2);
            step = over / 2 - offset;
            auto spline = netxs::spline01{ bias };
            auto sz = ratio * (size * 2 + 1);
            bitmap.size(sz);
            auto it = bitmap.begin();
            for (auto y = 0.f; y < sz.y; y++)
            {
                auto y0 = y / (sz.y - 1.f);
                auto sy = spline(y0);
                for (auto x = 0.f; x < sz.x; x++)
                {
                    auto x0 = x / (sz.x - 1.f);
                    auto sx = spline(x0);
                    auto xy = sy * sx;
                    auto a = (byte)std::round(alfa * xy);
                    fuse(*it++, a);
                }
            }
        }
        auto render(auto& canvas, auto win_size)
        {
            canvas.step(step);
            auto src = bitmap.area();
            auto dst = rect{ dot_00, win_size + over };
            auto cut = std::min(dot_00, (dst.size - src.size * 2 - dot_11) / 2);
            auto off = dent{ 0, cut.x, 0, cut.y };
            src += off;
            auto mid = rect{ src.size, std::max(dot_00, dst.size - src.size * 2) };
            auto top = rect{ twod{ src.size.x, 0 }, { mid.size.x, src.size.y }};
            auto lft = rect{ twod{ 0, src.size.y }, { src.size.x, mid.size.y }};
            if (mid)
            {
                auto base_shadow = bitmap[src.size - dot_11];
                netxs::onrect(canvas, mid, fx(base_shadow));
            }
            if (top)
            {
                auto pen = rect{{ src.size.x - 1, 0 }, { 1, src.size.y }};
                netxs::xform_scale(canvas, top, bitmap, pen, fx);
                top.coor.y += mid.size.y + top.size.y;
                netxs::xform_scale(canvas, top, bitmap, pen.rotate({ 1, -1 }), fx);
            }
            if (lft)
            {
                auto pen = rect{{ 0, src.size.y - 1 }, { src.size.x, 1 }};
                netxs::xform_scale(canvas, lft, bitmap, pen, fx);
                lft.coor.x += mid.size.x + lft.size.x;
                netxs::xform_scale(canvas, lft, bitmap, pen.rotate({ -1, 1 }), fx);
            }
            auto dir = dot_11;
                        netxs::xform_mirror(canvas, dst.rotate(dir).coor, bitmap, src.rotate(dir), fx);
            dir = -dir; netxs::xform_mirror(canvas, dst.rotate(dir).coor, bitmap, src.rotate(dir), fx);
            dir.x += 2; netxs::xform_mirror(canvas, dst.rotate(dir).coor, bitmap, src.rotate(dir), fx);
            dir = -dir; netxs::xform_mirror(canvas, dst.rotate(dir).coor, bitmap, src.rotate(dir), fx);
            canvas.step(-step);
        }
    };
    struct szgrips
    {
        using test = testy<twod>;

        twod origin; // szgrips: Grab's initial coord info.
        twod dtcoor; // szgrips: The form coor parameter change factor while resizing.
        twod sector; // szgrips: Active quadrant, x,y = {-1|+1}. Border widths.
        rect hzgrip; // szgrips: Horizontal grip.
        rect vtgrip; // szgrips: Vertical grip.
        twod widths; // szgrips: Grip's widths.
        bool inside; // szgrips: Is active.
        bool seized; // szgrips: Is seized.
        test lastxy; // szgrips: Change tracker.
        rect zoomsz; // szgrips: Captured area for zooming.
        dent zoomdt; // szgrips: Zoom step.
        bool zoomon; // szgrips: Zoom in progress.
        twod zoomat; // szgrips: Zoom pivot.

        szgrips()
            : inside{ faux },
              seized{ faux },
              zoomon{ faux }
        { }

        operator bool () { return inside || seized; }
        auto corner(twod length)
        {
            return dtcoor.less(dot_11, length, dot_00);
        }
        auto grab(rect window, twod curpos, dent outer)
        {
            if (inside)
            {
                origin = curpos - corner(window.size + outer);
                seized = true;
            }
            return seized;
        }
        auto calc(rect window, twod curpos, dent outer, dent inner, twod cell_size = dot_11)
        {
            auto border = outer - inner;
            auto area = rect{ dot_00, window.size };
            auto inner_rect = area + inner;
            auto outer_rect = area + outer;
            inside = !inner_rect.hittest(curpos)
                   && outer_rect.hittest(curpos);
            auto& length = outer_rect.size;
            curpos += outer.corner();
            auto center = std::max(length / 2, dot_11);
            if (!seized)
            {
                dtcoor = curpos.less(center + (length & 1), dot_11, dot_00);
                sector = dtcoor.less(dot_11, -dot_11, dot_11);
                widths = sector.less(dot_00, twod{-border.r,-border.b },
                                             twod{ border.l, border.t });
            }
            auto l = sector * (curpos - corner(length));
            auto a = center * l / center;
            auto b = center *~l /~center;
            auto s = sector * std::max(a - b + center, dot_00);

            hzgrip.coor.x = widths.x;
            hzgrip.coor.y = 0;
            hzgrip.size.y = widths.y;
            hzgrip.size.x = s.x - s.x % cell_size.x;

            vtgrip.coor = dot_00;
            vtgrip.size = widths;
            vtgrip.size.y += s.y - s.y % cell_size.y;
            return lastxy(curpos);
        }
        auto drag(rect window, twod curpos, dent outer, bool zoom)
        {
            auto boxsz = window.size + outer;
            auto delta = (corner(boxsz) + origin - curpos) * sector;
            if (zoom) delta *= 2;
            auto preview_step = zoom ? -delta / 2 : -delta * dtcoor;
            auto preview_area = rect{ window.coor + preview_step, window.size + delta };
            return std::pair{ preview_area, delta };
        }
        auto move(twod dxdy, bool zoom)
        {
            auto step = zoom ? -dxdy / 2 : -dxdy * dtcoor;
            return step;
        }
        void drop()
        {
            seized = faux;
        }
        auto draw(auto& canvas, rect area, auto fx)
        {
            auto vertex = corner(area.size);
            auto side_x = hzgrip.shift(vertex).normalize_itself().shift_itself(area.coor).trim(area);
            auto side_y = vtgrip.shift(vertex).normalize_itself().shift_itself(area.coor).trim(area);
            netxs::onrect(canvas, side_x, fx);
            netxs::onrect(canvas, side_y, fx);
            return std::pair{ side_x, side_y };
        }
    };

    void fill(auto& canvas, auto block, auto fx) // gfx: Fill block.
    {
        block.normalize_itself();
        netxs::onrect(canvas, block, fx);
    }
    void cage(auto& canvas, rect area, dent border, auto fx) // core: Draw the cage around specified area.
    {
        auto temp = area;
        temp.size.y = std::max(0, border.t); // Top
        fill(canvas, temp.trim(area), fx);
        temp.coor.y += area.size.y - border.b; // Bottom
        temp.size.y = std::max(0, border.b);
        fill(canvas, temp.trim(area), fx);
        temp.size.x = std::max(0, border.l); // Left
        temp.size.y = std::max(0, area.size.y - border.t - border.b);
        temp.coor.y = area.coor.y + border.t;
        fill(canvas, temp.trim(area), fx);
        temp.coor.x += area.size.x - border.r; // Right
        temp.size.x = std::max(0, border.r);
        fill(canvas, temp.trim(area), fx);
    }
}