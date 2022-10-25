// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_DUPLET_HPP
#define NETXS_DUPLET_HPP

#include "../text/utf.hpp"
#include "../abstract/fifo.hpp"

#if defined(__linux__) || defined(__APPLE__)
    #include <stdint.h>
#endif

namespace netxs
{
    using fifo = netxs::generics::fifo<si32>;

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
        constexpr void     operator -=  (T i)                   { x -=   i; y -=   i;               }
        constexpr void     operator +=  (T i)                   { x +=   i; y +=   i;               }
        constexpr void     operator *=  (T i)                   { x *=   i; y *=   i;               }
        constexpr void     operator /=  (T i)                   { x /=   i; y /=   i;               }
        constexpr bool     operator <   (T i) const             { return x < i && y < i;            }
        constexpr bool     operator >   (T i) const             { return x > i && y > i;            }
        constexpr duplet   operator +   (duplet const& p) const { return { x + p.x, y + p.y };      }
        constexpr duplet   operator -   (duplet const& p) const { return { x - p.x, y - p.y };      }
        constexpr duplet   operator *   (duplet const& p) const { return { x * p.x, y * p.y };      }
        constexpr duplet   operator /   (duplet const& p) const { return { x / p.x, y / p.y };      }
        constexpr duplet   operator -   ()                const { return {      -x,-y       };      }
        constexpr duplet   operator &   (T i)             const { return {   x & i, y & i   };      }
        constexpr duplet   operator ~   ()                const { return {       y, x       };      }

        ///In C++11, signed shift left of a negative number is always undefined
        //void operator>>= (T i) { x >>=i; y >>=i; }
        //void operator<<= (T i) { x <<=i; y <<=i; }

        ///In C++11, signed shift left of a negative number is always undefined
        //duplet operator << (T i) const { return { x << i, y << i }; }
        //duplet operator >> (T i) const { return { x >> i, y >> i }; }

        template<class D> duplet<D> operator / (D i) const { return { x / i, y / i }; }
        template<class D> duplet<D> operator + (D i) const { return { x + i, y + i }; }
        template<class D> duplet<D> operator - (D i) const { return { x - i, y - i }; }
        template<class D> duplet<D> operator * (D i) const { return { x * i, y * i }; }

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

    using twod = duplet<si32>;

    static constexpr const auto dot_00 = twod{ 0,0 };
    static constexpr const auto dot_01 = twod{ 0,1 };
    static constexpr const auto dot_10 = twod{ 1,0 };
    static constexpr const auto dot_11 = twod{ 1,1 };
    static constexpr const auto dot_22 = twod{ 2,2 };
    static constexpr const auto dot_21 = twod{ 2,1 };
    static constexpr const auto dot_33 = twod{ 3,3 };
    static constexpr const auto dot_mx = twod{ std::numeric_limits<si32>::max() / 2,
                                               std::numeric_limits<si32>::max() / 2 };

    static twod divround(twod const& p, si32 n       ) { return { divround(p.x, n  ), divround(p.y, n  ) }; }
    static twod divround(si32 n       , twod const& p) { return { divround(n  , p.x), divround(n  , p.y) }; }
    static twod divround(twod const& n, twod const& p) { return { divround(n.x, p.x), divround(n.y, p.y) }; }
    static twod divupper(twod const& n, twod const& p) { return { divupper(n.x, p.x), divupper(n.y, p.y) }; }
} // namespace netxs

namespace std
{
    template<class T = netxs::si32> static netxs::duplet<T> min  (netxs::duplet<T> const& p1, netxs::duplet<T> const& p2) { return { std::min(p1.x, p2.x), std::min(p1.y, p2.y) }; }
    template<class T = netxs::si32> static netxs::duplet<T> max  (netxs::duplet<T> const& p1, netxs::duplet<T> const& p2) { return { std::max(p1.x, p2.x), std::max(p1.y, p2.y) }; }
    template<class T = netxs::si32> static netxs::duplet<T> round(netxs::duplet<T> const& p) { return { std::round(p.x), std::round(p.y) }; }
    template<class T = netxs::si32> static netxs::duplet<T> abs  (netxs::duplet<T> const& p) { return { std::  abs(p.x), std::  abs(p.y) }; }
    template<class T = netxs::si32> static netxs::duplet<T> clamp(netxs::duplet<T> const& p, netxs::duplet<T> const& p1, netxs::duplet<T> const& p2) { return { std::clamp(p.x, p1.x, p2.x), std::clamp(p.y, p1.y, p2.y) }; }
} // namespace std

#endif // NETXS_DUPLET_HPP