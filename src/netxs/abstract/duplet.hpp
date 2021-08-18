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
    using fifo = netxs::generics::fifo<iota>;

    template<class T = int>
    struct duplet
    {
        using type = T;

        T x;
        T y;

        constexpr duplet()
            : x{ 0 },
              y{ 0 }
        { }

        constexpr duplet (T const& x, T const& y)
            : x{ x },
              y{ y }
        { }

        constexpr duplet (duplet const& p)
            : duplet{ p.x,
                      p.y }
        { }

        template<class D>
        constexpr duplet (duplet<D> const& d)
            : duplet{ static_cast<T>(d.x),
                      static_cast<T>(d.y) }
        { }

        constexpr duplet (fifo& queue)
            : x{ queue(0) },
              y{ queue(0) }
        { }

        constexpr T&       operator []  (int selector)          { return selector ? x : y;          }
        constexpr T const& operator []  (int selector) const    { return selector ? x : y;          }
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
        constexpr bool     operator ==  (duplet const& p) const { return x == p.x && y == p.y;      }
        constexpr bool     operator !=  (duplet const& p) const { return x != p.x || y != p.y;      }
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

        template<class D> duplet<D> operator /(D i) const { return { x / i, y / i }; }
        template<class D> duplet<D> operator +(D i) const { return { x + i, y + i }; }
        template<class D> duplet<D> operator -(D i) const { return { x - i, y - i }; }
        template<class D> duplet<D> operator *(D i) const { return { x * i, y * i }; }

        bool operator ()(duplet const& p)
        {
            if (*this != p)
            {
                x = p.x;
                y = p.y;
                return true;
            }
            return faux;
        }
        duplet less(duplet const& what, duplet const& iftrue, duplet const& iffalse) const
        {
            return { x < what.x ? iftrue.x : iffalse.x,
                     y < what.y ? iftrue.y : iffalse.y };
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
        friend auto& operator<< (std::ostream& s, duplet const& p)
        {
            return s << "{ " << p.x << ", " << p.y << " }";
        }
    };

    using twod = duplet<iota>;

    static constexpr const twod dot_00{ 0,0 };
    static constexpr const twod dot_01{ 0,1 };
    static constexpr const twod dot_10{ 1,0 };
    static constexpr const twod dot_11{ 1,1 };
    static constexpr const twod dot_22{ 2,2 };
    static constexpr const twod dot_21{ 2,1 };
    static constexpr const twod dot_33{ 3,3 };
    static constexpr const twod dot_mx{ std::numeric_limits<iota>::max() / 2,
                                        std::numeric_limits<iota>::max() / 2 };

    static twod divround(twod const& p, iota n       ) { return { divround(p.x, n  ), divround(p.y, n  ) }; }
    static twod divround(iota n       , twod const& p) { return { divround(n  , p.x), divround(n  , p.y) }; }
    static twod divround(twod const& n, twod const& p) { return { divround(n.x, p.x), divround(n.y, p.y) }; }
    static twod divupper(twod const& n, twod const& p) { return { divupper(n.x, p.x), divupper(n.y, p.y) }; }
} // namespace netxs

namespace std
{
    template<class T = netxs::iota> static netxs::duplet<T> min  (netxs::duplet<T> const& p1, netxs::duplet<T> const& p2) { return { std::min(p1.x, p2.x), std::min(p1.y, p2.y) }; }
    template<class T = netxs::iota> static netxs::duplet<T> max  (netxs::duplet<T> const& p1, netxs::duplet<T> const& p2) { return { std::max(p1.x, p2.x), std::max(p1.y, p2.y) }; }
    template<class T = netxs::iota> static netxs::duplet<T> round(netxs::duplet<T> const& p) { return { std::round(p.x), std::round(p.y) }; }
    template<class T = netxs::iota> static netxs::duplet<T> abs  (netxs::duplet<T> const& p) { return { std::  abs(p.x), std::  abs(p.y) }; }
    template<class T = netxs::iota> static netxs::duplet<T> clamp(netxs::duplet<T> const& p, netxs::duplet<T> const& p1, netxs::duplet<T> const& p2) { return { std::clamp(p.x, p1.x, p2.x), std::clamp(p.y, p1.y, p2.y) }; }
} // namespace std

#endif // NETXS_DUPLET_HPP