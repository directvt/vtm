// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_INTMATH_HPP
#define NETXS_INTMATH_HPP

#include <optional>
#include <algorithm>
#include <limits>
#include <cmath>
#include <cfenv>
#include <cassert>
#include <bit>

#ifndef faux
    #define faux (false)
#endif

namespace netxs
{
    using byte = uint8_t;
    using ui16 = uint16_t;
    using ui32 = uint32_t;
    using ui64 = uint64_t;
    using si16 = int16_t;
    using si32 = int32_t;
    using si64 = int64_t;
    using sz_t = ui32;

    static constexpr auto maxsi32 = std::numeric_limits<si32>::max();

    constexpr size_t operator "" _sz (unsigned long long i)	{ return i; }

    struct noop { template<class ...T> constexpr void operator()(T...) {}; };

    template<class T>
    using to_signed_t = std::conditional_t<(si64)std::numeric_limits<std::remove_reference_t<T>>::max() <= std::numeric_limits<si16>::max(), si16,
                        std::conditional_t<(si64)std::numeric_limits<std::remove_reference_t<T>>::max() <= std::numeric_limits<si32>::max(), si32, si64>>;

    // intmath: Swap two bits.
    template<unsigned int p1, unsigned int p2, class T>
    auto swap_bits(T n)
    {
        auto a = 1 & (n >> p1);
        auto b = 1 & (n >> p2);
        auto x = a ^ b;
        return n ^ (x << p1 | x << p2);
    }
    // intmath: Convert LE to host endianness.
    template<class T>
    constexpr void letoh(byte* buff, T& i)
    {
        if constexpr (std::is_same_v<T, ui32>
                   || std::is_same_v<T, si32>)
        {
            i = (ui32)buff[0] <<  0 |
                (ui32)buff[1] <<  8 |
                (ui32)buff[2] << 16 |
                (ui32)buff[3] << 24;
        }
        else if constexpr (std::is_same_v<T, ui64>
                        || std::is_same_v<T, si64>)
        {
            i = (ui64)buff[0] <<  0 |
                (ui64)buff[1] <<  8 |
                (ui64)buff[2] << 16 |
                (ui64)buff[3] << 24 |
                (ui64)buff[4] << 32 |
                (ui64)buff[5] << 40 |
                (ui64)buff[6] << 48 |
                (ui64)buff[7] << 56;
        }
        else if constexpr (std::is_same_v<T, ui16>
                        || std::is_same_v<T, si16>)
        {
            i = (ui16)buff[0] << 0 |
                (ui16)buff[1] << 8;
        }
    }
    // intmath: Convert BE to host endianness.
    template<class T>
    constexpr void betoh(byte* buff, T& i)
    {
        if constexpr (std::is_same_v<T, ui32>
                   || std::is_same_v<T, si32>)
        {
            i = (ui32)buff[3] <<  0 |
                (ui32)buff[2] <<  8 |
                (ui32)buff[1] << 16 |
                (ui32)buff[0] << 24;
        }
        else if constexpr (std::is_same_v<T, ui64>
                        || std::is_same_v<T, si64>)
        {
            i = (ui64)buff[7] <<  0 |
                (ui64)buff[6] <<  8 |
                (ui64)buff[5] << 16 |
                (ui64)buff[4] << 24 |
                (ui64)buff[3] << 32 |
                (ui64)buff[2] << 40 |
                (ui64)buff[1] << 48 |
                (ui64)buff[0] << 56;
        }
        else if constexpr (std::is_same_v<T, ui16>
                        || std::is_same_v<T, si16>)
        {
            i = (ui16)buff[1] << 0 |
                (ui16)buff[0] << 8;
        }
    }
    namespace
    {
        // intmath: Invert endianness.
        template<class T>
        constexpr auto _swap_bytes(T i)
        {
            T r;
            if constexpr (std::is_same_v<T, ui32>
                       || std::is_same_v<T, si32>)
            {
                auto n = static_cast<ui32>(i);
                r = (n & 0x000000FF) << 24 |
                    (n & 0x0000FF00) <<  8 |
                    (n & 0x00FF0000) >>  8 |
                    (n & 0xFF000000) >> 24;
            }
            else if constexpr (std::is_same_v<T, ui64>
                            || std::is_same_v<T, si64>)
            {
                auto n = static_cast<ui64>(i);
                r = (n & 0x00000000000000FF) << 56 |
                    (n & 0x000000000000FF00) << 40 |
                    (n & 0x0000000000FF0000) << 24 |
                    (n & 0x00000000FF000000) <<  8 |
                    (n & 0x000000FF00000000) >>  8 |
                    (n & 0x0000FF0000000000) >> 24 |
                    (n & 0x00FF000000000000) >> 40 |
                    (n & 0xFF00000000000000) >> 56;
            }
            else if constexpr (std::is_same_v<T, ui16>
                            || std::is_same_v<T, si16>)
            {
                auto n = static_cast<ui16>(i);
                r = (n & 0x00FF) << 8 |
                    (n & 0xFF00) >> 8;
            }
            return r;
        }
    }
    // intmath: Convert LE to host endianness.
    template<class T, bool BE = std::endian::native == std::endian::big>
    constexpr auto letoh(T i)
    {
        if constexpr (BE) return _swap_bytes(i);
        else              return i;
    }
    // intmath: Convert BE to host endianness.
    template<class T, bool LE = std::endian::native == std::endian::little>
    constexpr auto betoh(T i)
    {
        if constexpr (LE) return _swap_bytes(i);
        else              return i;
    }
    // intmath: LE type wrapper. T has an LE format in memory.
    template<class T>
    class le_t
    {
        T data = {};

    public:
        void set(T const& v) { data = netxs::letoh(v); }
        T    get() const     { return netxs::letoh(data); }
    };

    // intmath: Summ and return TRUE in case of
    //          unsigned integer overflow and store result in accum.
    template<class T1, class T2>
    constexpr bool sum_overflow(T1& accum, T2 delta)
    {
        auto store = accum;
        accum += delta;
        return accum <= store ? true : faux;
    }

    // intmath: Clamp a value in case it exceeds its numerical limits.
    template<class T, class L>
    constexpr T clamp(L value)
    {
        static_assert(std::is_integral<T>::value, "Integral type only");
        static_assert(std::is_integral<L>::value, "Integral type only");

        if constexpr (sizeof(T) < sizeof(L))
        {
            constexpr L max = std::numeric_limits<T>::max();
            return static_cast<T>(std::min(value, max));
        }
        else
        {
            return value;
        }
    }

    template<class T1, class T2, class T3 = T2>
    constexpr T3 divround(T1 n, T2 d)
    {
        static_assert(std::is_integral<T1>::value, "Integral type only");
        static_assert(std::is_integral<T2>::value, "Integral type only");
        static_assert(std::is_integral<T3>::value, "Integral type only");

        ///In C++11, signed shift left of a negative number is always undefined
        //return d != 0 ? ((n << 1) - d + ((true && ((n < 0) ^ (d > 0))) << 1) * d) / (d << 1) : 0;

        //return d != 0 ? ((n < 0) ^ (d < 0)) ? ((n - d / 2) / d)
        //                                    : ((n + d / 2) / d)
        //              : 0;
        return d != 0 ? ((n < 0) == (d < 0)) ? ((n + d / 2) / d)
                                             : ((n - d / 2) / d)
                      : 0;
    }

    template<class T1, class T2, class T3 = T2>
    constexpr T3 divupper(T1 n, T2 d)
    {
        static_assert(std::is_integral<T1>::value, "Integral type only");
        static_assert(std::is_integral<T2>::value, "Integral type only");
        static_assert(std::is_integral<T3>::value, "Integral type only");

        return	n > 0
            ?	1 + (n - 1) / d
            :	n / d;
    }

    template<class T1, class T2, class T3 = T2>
    constexpr T3 divfloor(T1 n, T2 d)
    {
        static_assert(std::is_integral<T1>::value, "Integral type only");
        static_assert(std::is_integral<T2>::value, "Integral type only");
        static_assert(std::is_integral<T3>::value, "Integral type only");

        return	n < 0
            ?	1 + (n - 1) / d
            :	n / d;
    }

    template<bool B, class T>
    struct _disintegrate { using type = T; };

    template<class T>
    struct _disintegrate<faux, T> { using type = typename T::type; };

    // intmath: Deduce a scalar type from the vector type.
    template<class T>
    using disintegrate = typename _disintegrate< std::is_integral<T>::value, T >::type;

    // intmath: Quadratic fader delta sequence generator.
    //          The QUADRATIC-LAW fader from the initial velocity
    //          to stop for a given period of time.
    template<class T>
    class quadratic
    {
        using twod = T;
        using type = disintegrate<twod>;

                twod speed; // quadratic: Distance ΔR over time period ΔT.
                type limit; // quadratic: Activity period.
                type phase; // quadratic: Register.
                type scale; // quadratic: Factor.
                type start; // quadratic: Deffered start time.
        mutable twod total; // quadratic: Current point on the path.

    public:
        /*
        Quadratic fader ctor:
            speed - distance ΔR over time period ΔT
            cycle - time period ΔT
            limit - activity period
            start - deffered start time
        */
        quadratic(twod const& speed, type cycle, type limit, type start)
            :	speed{ speed         },
                limit{ limit         },
                phase{ limit * 2     },
                scale{ phase * cycle },
                start{ start         },
                total{ twod{}        }
        { }

        auto operator () (type timer) const
        {
            auto delta = std::optional<twod>{};

            if (auto t = timer - start; t >= 0)
            {
                if (t < limit)
                {
                    auto n = speed * t * (phase - t);
                    auto s = divround(n, scale);
                    delta = s - total;
                    total = s;
                }
            }
            else
            {
                delta = twod{};
            }

            return delta;
        }
    };

    // intmath: Constant speed delta sequence generator.
    //          The LINEAR-LAW fader from the initial velocity
    //          to stop for a given period of time with constant speed.
    template<class T>
    class constlinear
    {
        using twod = T;
        using type = disintegrate<twod>;

                type limit; // constlinear: Activity period.
                type phase; // constlinear: Register.
                twod speed; // constlinear: Distance ΔR over time period ΔT.
                type scale; // constlinear: Factor.
                type start; // constlinear: Deffered start time.
        mutable twod total; // constlinear: Current point on the path.

    public:
        /*
        Linear constant speed delta generator ctor:
            speed - distance ΔR over time period ΔT
            cycle - time period ΔT
            limit - activity period
            start - deffered start time
        */
        constlinear(twod const& speed, type cycle, type limit, type start)
            :	limit{ limit         },
                phase{ limit * 2     },
                speed{ speed * phase },
                scale{ cycle * phase },
                start{ start         },
                total{ twod{}        }
        { }

        auto operator () (type timer) const
        {
            auto delta = std::optional<twod>{};

            if (auto t = timer - start; t >= 0)
            {
                if (t < limit)
                {
                    auto n = speed * t;
                    auto s = divround(n, scale);
                    delta = s - total;
                    total = s;
                }
            }
            else
            {
                delta = twod{};
            }

            return delta;
        }
    };

    // intmath: Constant speed delta sequence generator.
    //          The LINEAR-LAW fader from the initial point to the destination
    //          point for a given period of time with constant speed.
    template<class T>
    class constlinearAtoB
    {
        using twod = T;
        using type = disintegrate<twod>;

                type limit; // constlinearAtoB: Activity period.
                twod range; // constlinearAtoB: Path's end point (from 0 to range).
                type start; // constlinearAtoB: Deffered start time point.
        mutable twod total; // constlinearAtoB: Current point on the path.

    public:
        constlinearAtoB(twod const& range, type limit, type start)
            :	limit{ limit },
                range{ range },
                start{ start },
                total{ twod{}}
        { }

        auto operator () (type timer) const
        {
            auto delta = std::optional<twod>{};

            if (auto t = timer - start; t >= 0)
            {
                if (t < limit)
                {
                    auto s = divround(range * t, limit);
                    delta = s - total;
                    total = s;
                }
                else if (range != total)
                {
                    delta = range - total;
                    total = range;
                }
            }
            else
            {
                delta = twod{};
            }

            return delta;
        }
    };

    // intmath: Forward/Reverse (bool template arg) copy the specified
    //          sequence of cells onto the canvas at the specified offset
    //          and return count of copied cells.
    template<bool RtoL, class T1, class T2, class P>
    auto xerox (T1*& frame, T2 const& source, P handle)
    {
        auto lyric = source.data();
        auto width = source.length();

        if constexpr (RtoL) lyric += width;

        auto limit = frame + width;
        while (limit != frame)
        {
            if constexpr (RtoL) handle(*frame++, *--lyric);
            else                handle(*frame++, *lyric++);
        }

        return width;
    }

    // intmath: Fill the canvas by the stretched bitmap.
    template<class T, class P>
    void zoomin(T& canvas, T const& bitmap, P handle)
    {
        auto size1 = canvas.size();
        auto size2 = bitmap.size();
        auto data1 = canvas.data();
        auto data2 = bitmap.data();
        auto data3 = data2;
        if (size1.x * size1.y == 0
         || size2.x * size2.y == 0) return;

        auto dot_11 = size1 / size1;
        auto msize0 = size1 - dot_11;
        auto msize1 = max(dot_11, msize0);
        auto msize2 = size2 - dot_11;

        auto y = 0;
        auto h_line = [&]
        {
            auto x = 0;
            while (x != msize0.x)
            {
                auto xpos = x++ * msize2.x / msize1.x;
                auto from = data3 + xpos;
                handle(*data1++, *from);
            }
            auto ypos = ++y * msize2.y / msize1.y;
            auto from = data3 + msize2.x;
            handle(*data1++, *from);
            data3 = data2 + ypos * size2.x;
        };

        while (y != msize0.y)
        {
            h_line();
        }
        data3 = data2 + msize2.y * size2.x;
        h_line();
    }

    // intmath: Copy the bitmap to the bitmap by invoking
    //          handle(sprite1_element, sprite2_element) for each elem.
    template<class T, class P>
    void oncopy(T& bitmap1, T const& bitmap2, P handle)
    {
        auto& size1 = bitmap1.size();
        auto& size2 = bitmap2.size();

        if (size1 == size2)
        {
            auto data1 = bitmap1.data();
            auto data2 = bitmap2.data();

            auto limit = data1 + size1.y * size2.x;
            while (limit != data1)
            {
                handle(*data1++, *data2++);
            }
        }
    }

    // intmath: Intersect two sprites and
    //          invoking handle(sprite1_element, sprite2_element)
    //          for each elem in the intersection.
    template<bool RtoL, class T, class D, class R, class C, class P>
    void inbody(T& canvas, D const& bitmap, R const& region, C const& base2, P handle)
    {
        auto& base1 = region.coor;

        auto& size1 = canvas.size();
        auto& size2 = bitmap.size();

        auto  data1 = canvas.data() + base1.x + base1.y * size1.x;
        auto  data2 = bitmap.data() + base2.x + base2.y * size2.x;

        auto  skip1 = size1.x - region.size.x;
        auto  skip2 = size2.x;
        if constexpr (RtoL)
        {
            data2 += region.size.x;
            skip2 += region.size.x;
        }
        else
        {
            skip2 -= region.size.x;
        }

        auto limit = data1 + region.size.y * size1.x;
        while (limit != data1)
        {
            auto limit = data1 + region.size.x;
            while (limit != data1)
            {
                if constexpr (RtoL) handle(*data1++, *--data2);
                else                handle(*data1++, *data2++);
            }
            data1 += skip1;
            data2 += skip2;
        }
    }

    // intmath: Wild bitmap.
    template<class T, class Rect>
    struct raster
    {
        T    _data;
        Rect _area;
        auto  data()       { return _data.begin(); }
        auto& size()       { return _area.size;    }
        auto& area()       { return _area;         }
        auto  data() const { return _data.begin(); }
        auto& size() const { return _area.size;    }
        auto& area() const { return _area;         }
        raster(T data, Rect area)
            : _data{ data },
              _area{ area }
        { }
    };

    // intmath: Intersect two sprites and invoking
    //          handle(sprite1_element, sprite2_element)
    //          for each elem in the intersection.
    template<class T, class D, class P>
    void onbody(T& canvas, D const& bitmap, P handle)
    {
        auto& rect1 = canvas.area();
        auto& rect2 = bitmap.area();

        if (auto joint = rect1.clip(rect2))
        {
            auto basis = joint.coor - rect2.coor;
            joint.coor-= rect1.coor;

            inbody<faux>(canvas, bitmap, joint, basis, handle);
        }
    }

    // intmath: Draw the rectangle region inside the canvas by
    //          invoking handle(canvas_element)
    //          (without boundary checking).
    template<bool RtoL = faux, class T, class RECT, class P, class NEWLINEFX = noop>
    void onrect(T& canvas, RECT const& region, P handle, NEWLINEFX online = NEWLINEFX())
    {
        //using ret_t = std::template result_of_t<P(decltype(*(canvas.data())))>;
        using ret_t = std::invoke_result_t<P, decltype(*(canvas.data()))>;
        static constexpr auto plain = std::is_same_v<void, ret_t>;

        auto& place = canvas.area();
        if (auto joint = region.clip(place))
        {
            auto basis = joint.coor - place.coor;
            auto frame = place.size.x * basis.y + basis.x + canvas.data();
            auto notch = place.size.x - joint.size.x;
            auto limit = place.size.x * joint.size.y + frame;
            while (limit != frame)
            {
                auto limit = frame + joint.size.x;
                while (limit != frame)
                {
                    if constexpr (RtoL)
                    {
                        if constexpr (plain) handle(*--limit);
                        else             if (handle(*--limit)) return;
                    }
                    else
                    {
                        if constexpr (plain) handle(*frame++);
                        else             if (handle(*frame++)) return;
                    }
                }
                if constexpr (RtoL) frame += joint.size.x;
                frame += notch;
                online();
            }
        }
    }

    static inline
    bool liang_barsky(float xmin, float ymin, float xmax, float ymax,
                      float&  x1, float&  y1, float&  x2, float&  y2)
    {
        auto dx = x2 - x1;
        auto dy = y2 - y1;
        auto left = x1 - xmin;
        auto top  = y1 - ymin;

        if ((dx == 0.0f && left < 0.0f)
         || (dy == 0.0f && top  < 0.0f))
        {
            return faux; // Line is parallel to rectangle.
        }

        auto max = 0.0f;
        auto min = 1.0f;

        auto cut = [&](auto side1, auto side2, auto delta)
        {
            if (delta > 0.0f)
            {
                auto k1 =-side1 / delta;
                auto k2 = side2 / delta;
                if (max < k1) max = k1;
                if (min > k2) min = k2;
            }
            else if (delta < 0.0f)
            {
                auto k1 =-side1 / delta;
                auto k2 = side2 / delta;
                if (max < k2) max = k2;
                if (min > k1) min = k1;
            }
        };

        auto right  = xmax - x1;
        auto bottom = ymax - y1;

        cut(left, right, dx);
        cut(top, bottom, dy);

        if (max < min)
        {
            if (min != 1.0f)
            {
                x2 = x1 + dx * min;
                y2 = y1 + dy * min;
            }
            if (max != 0.0f)
            {
                x1 = x1 + dx * max;
                y1 = y1 + dy * max;
            }
            return true;
        }
        else
        {
            return faux; // Line is outside.
        }
    }

    // intmath: Draw an antialiased line inside the specified rect
    //          between p1 and p2 by invoking pset(x, y, k).
    //          Return TRUE if line is visible.
    template<class R, class T, class P>
    bool online(R rect, T p0, T p1, P pset)
    {
        using twod = T;
        using type = disintegrate<twod>;

        ui16 gain = 0;
        si32   dx = p1.x - p0.x;
        si32   dy = p1.y - p0.y;
        ui32   lx = std::abs(dx);
        ui32   ly = std::abs(dy);

        rect = rect.normalize();
        twod& coor = rect.coor;
        twod& size = rect.size;
        p0 -= coor;
        p1 -= coor;

        auto set = [&](auto const& p, auto k) { if (size.inside(p)) pset(p + coor, k); };
        auto draw = [&](auto set)
        {
            if (dx == 0)
            {
                if (dy > 0)	do { set(p0, 255); } while (p0.y++ != p1.y);
                else		do { set(p0, 255); } while (p0.y-- != p1.y);
            }
            else if (dy == 0)
            {
                if (dx > 0)	do { set(p0, 255); } while (p0.x++ != p1.x);
                else		do { set(p0, 255); } while (p0.x-- != p1.x);
            }
            else
            {
                auto line = [&](auto& head, auto tail, auto incx, auto incy)
                {
                    T mold = head;

                    auto loop = [&](auto  size_x, auto  size_y,
                                    auto  incr_x, auto  incr_y,
                                    auto& head_x, auto& head_y,
                                    auto& mold_x, auto& mold_y,
                                                  auto  tail_y)
                    {
                        auto step = (size_x << 16) / size_y;
                        incr_x(mold_x);
                        while (true)
                        {
                            auto k = gain >> 8;
                            set(head, k ^ 255);
                            set(mold, k);

                            if (head_y == tail_y) break; // with last pixel
                            incr_y(head_y);
                            //if (head_y == tail_y) break; // w/o last pixel
                            incr_y(mold_y);
                            if (sum_overflow(gain, step))
                            {
                                incr_x(head_x);
                                incr_x(mold_x);
                            }
                        }
                    };

                    if (ly > lx) loop(lx, ly, incx, incy, head.x, head.y, mold.x, mold.y, tail.y);
                    else         loop(ly, lx, incy, incx, head.y, head.x, mold.y, mold.x, tail.x);
                };

                auto incr = [](auto& a){ ++a; };
                auto decr = [](auto& a){ --a; };

                if (dy > 0)
                {
                    if (dx > 0) line(p0, p1, incr, incr);
                    else        line(p0, p1, decr, incr);
                }
                else
                {
                    if (dx > 0) line(p0, p1, incr, decr);
                    else        line(p0, p1, decr, decr);
                }
            }
        };

        if (!size.inside(p0) || !size.inside(p1))
        {
            float x1 = static_cast<float>(p0.x); float y1 = static_cast<float>(p0.y);
            float x2 = static_cast<float>(p1.x); float y2 = static_cast<float>(p1.y);
            float minx = -1.0f;// One element wide margin for antialiasing.
            float miny = -1.0f;//
            float maxx = size.x + 1.0f;
            float maxy = size.y + 1.0f;

            if (liang_barsky(minx, miny, maxx, maxy, x1, y1, x2, y2))
            {
                auto align = [](auto& oldx, auto& oldy, auto maxx, auto maxy, auto newy, bool dirx)
                {
                    auto delta = (maxx << 16) / maxy * std::abs(newy - oldy);
                    auto error = delta >> 16;
                    dirx ? oldx += error
                         : oldx -= error;
                    oldy = newy;
                    return delta;
                };

                p1.x = static_cast<type>(x2);
                p1.y = static_cast<type>(y2);
                auto new_x = static_cast<type>(x1);
                auto new_y = static_cast<type>(y1);

                if (p0.x != new_x || p0.y != new_y)
                {
                    if (ly > lx)
                    {
                        gain = align(p0.x, p0.y, lx, ly, new_y, dx > 0);
                    }
                    else if (lx > ly)
                    {
                        gain = align(p0.y, p0.x, ly, lx, new_x, dy > 0);
                    }
                    else
                    {
                        p0.x = new_x;
                        p0.y = new_y;
                    }
                }
            }
            else
            {
                return faux;
            }
        }

        draw(set);

        return true;
    }

    namespace _private
    {
        ///<summary> intmath:
        ///          Bitmap 1D box-blurring.
        ///          To achieve a 2D blur, it needs to apply it again and swap the X with Y,
        ///          and source with destination.
        /// </summary>
        /// <typeparam name="RGB_T"> Point value accumulator type. </typeparam>
        /// <typeparam name="CALC" > Whether do the division in the current round. Performance burst by 40% ! </typeparam>
        /// <param name="s_ptr"> Source bitmap array pointer. </param>
        /// <param name="d_ptr"> Destination bitmap array pointer. </param>
        /// <param name="w"    > Bitmap width. </param>
        /// <param name="h"    > Bitmap height. </param>
        /// <param name="rad_0"> Horizontal blur radius. </param>
        /// <param name="rad_x"> Vertical blur radius (for the second round). </param>
        /// <param name="s_dtx"> Index step along X in the source. </param>
        /// <param name="s_dty"> Index step along Y in the source. </param>
        /// <param name="d_dtx"> Index step along X in the destination. </param>
        /// <param name="d_dty"> Index step along Y in the destination. </param>
        /// <param name="P_BASE s_ref"> Lambda to convert source pointer to the reference. </param>
        /// <param name="P_DEST d_ref"> Lambda to convert destination pointer to the reference. </param>
        /// <param name="POSTFX shade"> Lambda for further processing. </param>
        template<class RGB_T, bool CALC,
            class SRC_T,
            class DST_T, class INT_T,
            class P_BASE, class P_DEST, class POSTFX = noop>
        void blur1d(SRC_T s_ptr,
                    DST_T d_ptr, INT_T w,
                                 INT_T h, INT_T rad_0,
                                          INT_T rad_x, INT_T s_dtx, INT_T s_dty,
                                                       INT_T d_dtx, INT_T d_dty,
            P_BASE s_ref, P_DEST d_ref, POSTFX shade = POSTFX())
        {
            auto rad_1 = rad_0 + 1;
            auto count = rad_0 + rad_1;
            auto beg_x = s_dtx * rad_0;
            auto end_x = s_dtx * (w - count);
            auto limit = s_ptr + s_dty * h;

            if constexpr (CALC) count *= rad_x + rad_x + 1;

            while (s_ptr < limit)
            {
                auto& first = s_ref(s_ptr);
                RGB_T accum = first;
                accum *= rad_1;

                auto front = s_ptr;
                auto until = s_ptr + beg_x;
                while (front < until)
                {
                    accum += s_ref(front);
                    front += s_dtx;
                }

                auto caret = d_ptr;
                until += beg_x;
                while (front <= until)
                {
                    accum -= first;
                    accum += s_ref(front);
                    auto& point = d_ref(caret);
                    point = CALC ? accum / count : accum;
                    shade(*caret);
                    front += s_dtx;
                    caret += d_dtx;
                }

                auto after = s_ptr;
                until = s_ptr + end_x;
                while (after < until)
                {
                    accum -= s_ref(after);
                    accum += s_ref(front);
                    auto& point = d_ref(caret);
                    point = CALC ? accum / count : accum;
                    shade(*caret);
                    after += s_dtx;
                    front += s_dtx;
                    caret += d_dtx;
                }

                auto& final = s_ref(front - s_dtx);
                until += beg_x;
                while (after < until)
                {
                    accum -= s_ref(after);
                    accum += final;
                    auto& point = d_ref(caret);
                    point = CALC ? accum / count : accum;
                    shade(*caret);
                    after += s_dtx;
                    caret += d_dtx;
                }

                s_ptr += s_dty;
                d_ptr += d_dty;
            }
        }
        // intmath: Move block to the specified destination. If begin_it > end_it (exclusive) decrement is used.
        template<bool FWD, class SRC, class DST, class P>
        void proc_block(SRC begin_it, SRC end_it, DST dest_it, P proc)
        {
                while (begin_it != end_it)
                {
                    if constexpr (FWD)
                    {
                        proc(*begin_it, *dest_it);
                        ++begin_it;
                        ++dest_it;
                    }
                    else
                    {
                        proc(*begin_it, *dest_it);
                        --begin_it;
                        --dest_it;
                    }
            }
        }
    }

    template<bool FWD = true, class SRC, class DST>
    void move_block(SRC begin_it, SRC end_it, DST dest_it)
    {
        _private::proc_block<FWD>(begin_it, end_it, dest_it, [](auto& src, auto& dst){ dst = std::move(src); });
    }
    template<bool FWD = true, class SRC, class DST>
    void swap_block(SRC begin_it, SRC end_it, DST dest_it)
    {
        _private::proc_block<FWD>(begin_it, end_it, dest_it, [](auto& src, auto& dst){ std::swap(src, dst); });
    }

    /// <summary> intmath:
    ///           Bokeh (acryllic, blur) approximation.
    ///           Edge points are multiplied by r in order to form inner glow.
    /// </summary>
    /// <typeparam name="RGB_T"> Point accumulator type. </typeparam>
    /// <param name="s_ptr"> Source bitmap array pointer. </param>
    /// <param name="d_ptr"> Destination bitmap array pointer. </param>
    /// <param name="w"> Bitmap width. </param>
    /// <param name="h"> Bitmap height. </param>
    /// <param name="r"> Bokeh radius. </param>
    /// <param name="s_dty"> Index step along Y in the source. </param>
    /// <param name="d_dty"> Index step along Y in the destination. </param>
    /// <param name="P_BASE s_ref"> Lambda to convert source pointer to the reference. </param>
    /// <param name="P_DEST d_ref"> Lambda to convert destination pointer to the reference. </param>
    /// <param name="POSTFX shade"> Lambda for further processing. </param>
    /// <exmpla>
    ///		see console:pro::panel::blur()
    /// </exmpla>
    template<class RGB_T,
        class SRC_T,
        class DST_T, class INT_T,
        class P_BASE, class P_DEST, class POSTFX = noop>
    void bokefy(SRC_T s_ptr,
                DST_T d_ptr, INT_T w,
                             INT_T h, INT_T r, INT_T s_dty,
                                               INT_T d_dty,
        P_BASE s_ref, P_DEST d_ref, POSTFX shade = POSTFX())
    {
        //auto rx = std::min(r + r, w - 1) >> 1;
        auto rx = std::min((r + r) << 1, w - 1) >> 1; // x2 to preserve 2:1 text proportions
        auto ry = std::min(r + r, h - 1) >> 1;

        //for (auto i = 0; i < 1000; i++) //test performance
        {
        _private::blur1d<RGB_T, 0>(s_ptr,    // blur horizontally and place
                                   d_ptr, w, // result to the temp buffer
                                          h, rx,
                                             0,  1, s_dty,
                                                 1, d_dty, s_ref,
                                                           d_ref);
        _private::blur1d<RGB_T, 1>(d_ptr,    // blur vertically and place
                                   s_ptr, h, // result back to the source
                                          w, ry,
                                             rx, d_dty, 1,
                                                 s_dty, 1, d_ref,
                                                           s_ref, shade);
        }
    }
}

#endif // NETXS_INTMATH_HPP