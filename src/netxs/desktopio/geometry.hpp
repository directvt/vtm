// Copyright (c) Dmitry Sapozhnikov
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

    // geometry: Generic 2D point.
    template<class T>
    struct xy2d
    {
        using type = T;

        T x, y;

        template<class D> constexpr static auto cast(D x) requires(std::is_floating_point_v<T> || (std::is_integral_v<T> == std::is_integral_v<D>)) { return netxs::saturate_cast<T>(x); }
        template<class D> constexpr static auto cast(D x) requires(std::is_integral_v<T> && std::is_floating_point_v<D>)                            { return netxs::saturate_cast<T>(std::floor(x)); }

        constexpr xy2d(xy2d const&) = default;
        constexpr xy2d()
            : x{ },
              y{ }
        { }
        constexpr xy2d(T x, T y)
            : x{ x },
              y{ y }
        { }
        template<class D>
        constexpr xy2d(D x, D y)
            : x{ cast(x) },
              y{ cast(y) }
        { }
        template<class D>
        constexpr xy2d(xy2d<D> d)
            : xy2d{ d.x, d.y }
        { }
        constexpr xy2d(fifo& q)
            : x{ q(0) },
              y{ q(0) }
        { }

        template<class D>
        constexpr bool operator == (xy2d<D> p) const //todo Apple clang don't get auto result.
        {
            return x == cast(p.x)
                && y == cast(p.y);
        }
        template<class D>
        bool operator () (xy2d<D> p)
        {
            auto changed = *this != p;
            if (changed) *this = p;
            return changed;
        }
        constexpr explicit operator bool()   const { return x != T{} || y != T{};      }
        constexpr auto& operator [] (si32 i)       { return i ? y : x;                 }
        constexpr auto& operator [] (si32 i) const { return i ? y : x;                 }
        constexpr auto& operator ++ ()             { ++x; ++y;           return *this; }
        constexpr auto& operator -- ()             { --x; --y;           return *this; }
        constexpr auto& operator =  (xy2d p)       { x =  p.x; y =  p.y; return *this; }
        constexpr auto& operator += (xy2d p)       { x += p.x; y += p.y; return *this; }
        constexpr auto& operator -= (xy2d p)       { x -= p.x; y -= p.y; return *this; }
        constexpr auto& operator *= (xy2d p)       { x *= p.x; y *= p.y; return *this; }
        constexpr auto& operator /= (xy2d p)       { x /= p.x; y /= p.y; return *this; }
        constexpr auto& operator %= (xy2d p)       { x %= p.x; y %= p.y; return *this; }
        constexpr auto& operator -= (T i)          { x -= i;   y -= i;   return *this; }
        constexpr auto& operator += (T i)          { x += i;   y += i;   return *this; }
        constexpr auto& operator *= (T i)          { x *= i;   y *= i;   return *this; }
        constexpr auto& operator /= (T i)          { x /= i;   y /= i;   return *this; }
        constexpr auto& operator %= (T i)          { x %= i;   y %= i;   return *this; }
        constexpr auto  operator <  (T i)    const { return x < i && y < i;            }
        constexpr auto  operator >  (T i)    const { return x > i && y > i;            }
        constexpr auto  operator +  (xy2d p) const { return xy2d{ x + p.x, y + p.y };  }
        constexpr auto  operator -  (xy2d p) const { return xy2d{ x - p.x, y - p.y };  }
        constexpr auto  operator *  (xy2d p) const { return xy2d{ x * p.x, y * p.y };  }
        constexpr auto  operator /  (xy2d p) const { return xy2d{ x / p.x, y / p.y };  }
        constexpr auto  operator %  (xy2d p) const { return xy2d{ x % p.x, y % p.y };  } // Consider to use grid_mod().
        constexpr auto  operator -  ()       const { return xy2d{      -x,-y       };  }
        constexpr auto  operator &  (T i)    const { return xy2d{   x & i, y & i   };  }
        constexpr auto  operator ~  ()       const { return xy2d{       y, x       };  }

        //In C++11, signed shift left of a negative number is always undefined
        //void operator>>= (T i) { x >>=i; y >>=i; }
        //void operator<<= (T i) { x <<=i; y <<=i; }
        //
        //In C++11, signed shift left of a negative number is always undefined
        //xy2d operator << (T i) const { return { x << i, y << i }; }
        //xy2d operator >> (T i) const { return { x >> i, y >> i }; }

        template<class D, class = std::enable_if_t<std::is_arithmetic_v<D>>> constexpr auto operator +  (D i) const { return xy2d<D>{ x + i, y + i }; }
        template<class D, class = std::enable_if_t<std::is_arithmetic_v<D>>> constexpr auto operator -  (D i) const { return xy2d<D>{ x - i, y - i }; }
        template<class D, class = std::enable_if_t<std::is_arithmetic_v<D>>> constexpr auto operator *  (D i) const { return xy2d<D>{ x * i, y * i }; }
        template<class D, class = std::enable_if_t<std::is_arithmetic_v<D>>> constexpr auto operator /  (D i) const { return xy2d<D>{ x / i, y / i }; }
        template<class D, class = std::enable_if_t<std::is_arithmetic_v<D>>> constexpr auto operator +  (xy2d<D> f) const { return xy2d{ x + f.x, y + f.y }; }
        template<class D, class = std::enable_if_t<std::is_arithmetic_v<D>>> constexpr auto operator -  (xy2d<D> f) const { return xy2d{ x - f.x, y - f.y }; }
        template<class D, class = std::enable_if_t<std::is_arithmetic_v<D>>> constexpr auto operator *  (xy2d<D> f) const { return xy2d{ x * f.x, y * f.y }; }
        template<class D, class = std::enable_if_t<std::is_arithmetic_v<D>>> constexpr auto operator /  (xy2d<D> f) const { return xy2d{ x / f.x, y / f.y }; }
        template<class D, class = std::enable_if_t<std::is_arithmetic_v<D>>> constexpr auto operator += (xy2d<D> f)       { x = cast(x + f.x); y = cast(y + f.y); return *this; }
        template<class D, class = std::enable_if_t<std::is_arithmetic_v<D>>> constexpr auto operator -= (xy2d<D> f)       { x = cast(x - f.x); y = cast(y - f.y); return *this; }
        template<class D, class = std::enable_if_t<std::is_arithmetic_v<D>>> constexpr auto operator *= (xy2d<D> f)       { x = cast(x * f.x); y = cast(y * f.y); return *this; }
        template<class D, class = std::enable_if_t<std::is_arithmetic_v<D>>> constexpr auto operator /= (xy2d<D> f)       { x = cast(x / f.x); y = cast(y / f.y); return *this; }

        xy2d   less(xy2d what, xy2d if_yes, xy2d if_no) const
        {
            return { x < what.x ? if_yes.x : if_no.x,
                     y < what.y ? if_yes.y : if_no.y };
        }
        xy2d equals(xy2d what, xy2d if_yes, xy2d if_no) const
        {
            return { x == what.x ? if_yes.x : if_no.x,
                     y == what.y ? if_yes.y : if_no.y };
        }
        xy2d   less(T what, xy2d if_yes, xy2d if_no) const
        {
            return { x < what ? if_yes.x : if_no.x,
                     y < what ? if_yes.y : if_no.y };
        }
        xy2d equals(T what, xy2d if_yes, xy2d if_no) const
        {
            return { x == what ? if_yes.x : if_no.x,
                     y == what ? if_yes.y : if_no.y };
        }
        xy2d   less(T what, T if_yes, T if_no) const
        {
            return { x < what ? if_yes : if_no,
                     y < what ? if_yes : if_no };
        }
        xy2d equals(T what, T if_yes, T if_no) const
        {
            return { x == what ? if_yes : if_no,
                     y == what ? if_yes : if_no };
        }
        bool inside(xy2d p) const
        {
            return (x > 0 ? (p.x >= 0 && p.x < x) : (p.x >= x && p.x < 0))
                && (y > 0 ? (p.y >= 0 && p.y < y) : (p.y >= y && p.y < 0));
        }

        xy2d divround(T    n) const { return { netxs::divround(x, n  ), netxs::divround(y, n  ) }; }
        xy2d divround(xy2d p) const { return { netxs::divround(x, p.x), netxs::divround(y, p.y) }; }

        auto str() const
        {
            return "{ " + std::to_string(x) + ", " + std::to_string(y) + " }";
        }
        friend auto& operator << (std::ostream& s, xy2d p)
        {
            return s << "{ " << p.x << ", " << p.y << " }";
        }
        friend auto letoh(xy2d p) // Change endianness to LE.
        {
            return xy2d{ netxs::letoh(p.x), netxs::letoh(p.y) };
        }
        static constexpr auto sort(xy2d a, xy2d b)
        {
            if (a.x > b.x) std::swap(a.x, b.x);
            if (a.y > b.y) std::swap(a.y, b.y);
            return std::pair{ a, b };
        }
        friend auto   min(xy2d a, xy2d b) { return xy2d{ std::min(a.x, b.x), std::min(a.y, b.y) }; }
        friend auto   max(xy2d a, xy2d b) { return xy2d{ std::max(a.x, b.x), std::max(a.y, b.y) }; }
        friend auto   abs(xy2d p)         { return xy2d{ std::abs(p.x), std::abs(p.y) }; }
        friend auto round(xy2d p)         { return xy2d{ std::round(p.x), std::round(p.y) }; }
        friend auto clamp(xy2d p, xy2d p1, xy2d p2)
        {
            auto [a, b] = sort(p1, p2);
            return xy2d{ std::clamp(p.x, a.x, b.x),
                         std::clamp(p.y, a.y, b.y) };
        }
        constexpr auto clampby(xy2d p) const
        {
            auto [a, b] = sort(xy2d{}, p);
            return xy2d{ std::clamp(x, a.x, b.x), std::clamp(y, a.y, b.y) };
        }
    };

    // geometry: 2D point.
    using twod = xy2d<si32>;
    using fp2d = xy2d<fp32>;

    static constexpr auto dot_00 = twod{ 0,0 };
    static constexpr auto dot_01 = twod{ 0,1 };
    static constexpr auto dot_10 = twod{ 1,0 };
    static constexpr auto dot_11 = twod{ 1,1 };
    static constexpr auto dot_22 = twod{ 2,2 };
    static constexpr auto dot_21 = twod{ 2,1 };
    static constexpr auto dot_33 = twod{ 3,3 };
    static constexpr auto dot_mx = twod{ (si32)(si32max / 2.f), (si32)(si32max / 2.f) };

    twod divround(twod p, si32 n) { return { divround(p.x, n  ), divround(p.y, n  ) }; }
    twod divround(si32 n, twod p) { return { divround(n  , p.x), divround(n  , p.y) }; }
    twod divround(twod n, twod p) { return { divround(n.x, p.x), divround(n.y, p.y) }; }
    twod grid_mod(twod n, twod p) { return { grid_mod(n.x, p.x), grid_mod(n.y, p.y) }; }
}

namespace std
{
    template<class T> constexpr netxs::xy2d<T>   min(netxs::xy2d<T> p1, netxs::xy2d<T> p2) { return { std::min(p1.x, p2.x), std::min(p1.y, p2.y) }; }
    template<class T> constexpr netxs::xy2d<T>   max(netxs::xy2d<T> p1, netxs::xy2d<T> p2) { return { std::max(p1.x, p2.x), std::max(p1.y, p2.y) }; }
    template<class T> constexpr netxs::xy2d<T> round(netxs::xy2d<T> p)                     { return { std::round(p.x), std::round(p.y) }; }
    template<class T> constexpr netxs::xy2d<T>   abs(netxs::xy2d<T> p)                     { return { std::abs(p.x), std::abs(p.y) }; }
    template<class T> constexpr netxs::xy2d<T> floor(netxs::xy2d<T> p)                     { return { std::floor(p.x), std::floor(p.y) }; }
    template<class T> constexpr netxs::xy2d<T>  ceil(netxs::xy2d<T> p)                     { return { std::ceil(p.x), std::ceil(p.y) }; }
    template<class T> constexpr netxs::xy2d<T> clamp(netxs::xy2d<T> p, netxs::xy2d<T> p1, netxs::xy2d<T> p2) { return { std::clamp(p.x, p1.x, p2.x), std::clamp(p.y, p1.y, p2.y) }; }
}

namespace netxs
{
    // geometry: Rectangle.
    struct rect
    {
        twod coor, size;

        bool operator == (rect const&) const = default;
        explicit operator bool ()       const { return size.x != 0 && size.y != 0;            }
        auto   center          ()       const { return coor + size / 2;                       }
        auto   length          ()       const { return size.x * size.y;                       }
        twod   map             (twod p) const { return p - coor;                              }
        rect   shift           (twod p) const { return { coor + p, size };                    }
        auto&  shift_itself    (twod p)       { coor += p; return *this;                      }
        auto&  moveto          (twod p)       { coor = p;  return *this;                      }
        rect   operator /      (twod p) const { return { coor / p, size / p };                }
        rect   operator |      (rect r) const { return unite(r, *this);                       }
        auto&  operator |=     (rect r)       { return unitewith(r);                          }
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
        // rect: Unite with rect (normalized only).
        constexpr rect& unitewith(rect r)
        {
            auto new_coor = std::min(coor, r.coor);
            size = std::max(coor + size, r.coor + r.size ) - new_coor;
            coor = new_coor;
            return *this;
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
        constexpr bool nearby(rect r) const
        {
            return coor.x          <= r.coor.x + r.size.x
                && coor.y          <= r.coor.y + r.size.y
                && coor.x + size.x >= r.coor.x
                && coor.y + size.y >= r.coor.y;
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

    using regs = std::vector<rect>;

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
        side(fifo& q)
        {
            l = q.subarg(0);
            r = q.subarg(0);
            t = q.subarg(0);
            b = q.subarg(0);
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

    // geometry: Padding around the rect.
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
            l = q.subarg(0);
            r = q.subarg(0);
            t = q.subarg(0);
            b = q.subarg(0);
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
        friend constexpr auto operator + (twod size, dent pad)
        {
            return twod{ std::max(0, size.x + (pad.l + pad.r)),
                         std::max(0, size.y + (pad.t + pad.b)) };
        }
        // dent: Return size without padding.
        friend constexpr auto operator - (twod size, dent pad)
        {
            return twod{ std::max(0, size.x - (pad.l + pad.r)),
                         std::max(0, size.y - (pad.t + pad.b)) };
        }
        // dent: Return area with padding.
        friend constexpr rect operator + (rect area, dent pad) //todo msvc 17.10.1 don't get auto as return type
        {
            if (area.size.x < 0) { area.coor.x += pad.l; area.size.x -= pad.l + pad.r; }
            else                 { area.coor.x -= pad.l; area.size.x += pad.l + pad.r; }
            if (area.size.y < 0) { area.coor.y += pad.t; area.size.y -= pad.t + pad.b; }
            else                 { area.coor.y -= pad.t; area.size.y += pad.t + pad.b; }
            return area;
        }
        // dent: Return area without padding.
        friend constexpr auto operator - (rect area, dent pad)
        {
            if (area.size.x < 0) { area.coor.x -= pad.l; area.size.x += pad.l + pad.r; }
            else                 { area.coor.x += pad.l; area.size.x -= pad.l + pad.r; }
            if (area.size.y < 0) { area.coor.y -= pad.t; area.size.y += pad.t + pad.b; }
            else                 { area.coor.y += pad.t; area.size.y -= pad.t + pad.b; }
            return area;
        }
        // dent: Return area with padding.
        friend constexpr auto operator += (rect& area, dent pad)
        {
            return area = area + pad;
        }
        // dent: Return area without padding.
        friend constexpr auto operator -= (rect& area, dent pad)
        {
            return area = area - pad;
        }
        // dent: Return size with padding.
        friend constexpr auto operator += (twod& size, dent pad)
        {
            return size = size + pad;
        }
        // dent: Return size without padding.
        friend constexpr auto operator -= (twod& size, dent pad)
        {
            return size = size - pad;
        }
        // dent: Return summ of two paddings.
        friend constexpr auto operator + (dent pad1, dent pad2)
        {
            pad1.l += pad2.l;
            pad1.r += pad2.r;
            pad1.t += pad2.t;
            pad1.b += pad2.b;
            return pad1;
        }
        // dent: Return diff of two paddings.
        friend constexpr auto operator - (dent pad1, dent pad2)
        {
            pad1.l -= pad2.l;
            pad1.r -= pad2.r;
            pad1.t -= pad2.t;
            pad1.b -= pad2.b;
            return pad1;
        }
        auto bump(rect src) const
        {
            auto dst = src + *this;
            if (dst.size.x < 0)
            {
                dst.coor.x = std::clamp(dst.coor.x, src.coor.x, src.coor.x + src.size.x);
                dst.size.x = 0;
            }
            if (dst.size.y < 0)
            {
                dst.coor.y = std::clamp(dst.coor.y, src.coor.y, src.coor.y + src.size.y);
                dst.size.y = 0;
            }
            return dst;
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
        friend constexpr auto min(dent d1, dent d2) { return dent{ std::min(d1.l, d2.l), std::min(d1.r, d2.r), std::min(d1.t, d2.t), std::min(d1.b, d2.b) }; }
        friend constexpr auto max(dent d1, dent d2) { return dent{ std::max(d1.l, d2.l), std::max(d1.r, d2.r), std::max(d1.t, d2.t), std::max(d1.b, d2.b) }; }
    };
    // dent: Return difference between area.
    auto operator - (rect r1, rect r2)
    {
        auto top = r2.coor - r1.coor;
        auto end = r1.size - r2.size - top;
        return dent{ top.x, end.x,
                     top.y, end.y };
    }
    // dent: Exclude r2 from r1.
    auto operator / (rect r1, rect r2)
    {
        r2 = r1.trim(r2);
        auto top = r2.coor - r1.coor;
        auto end = r1.size - r2.size - top;
        if (top.y == 0 && end.y == 0)
        {
            if (top.x > 0)
            {
                r1.size.x = std::min(r1.size.x, top.x);
            }
            else
            {
                auto dx = std::max(0, r1.size.x - end.x);
                r1.coor.x += dx;
                r1.size.x -= dx;
            }
        }
        else if (top.x == 0 && end.x == 0)
        {
            if (top.y > 0)
            {
                r1.size.y = std::min(r1.size.y, top.y);
            }
            else
            {
                auto dy = std::max(0, r1.size.y - end.y);
                r1.coor.y += dy;
                r1.size.y -= dy;
            }
        }
        return r1;
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