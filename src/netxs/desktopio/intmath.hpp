// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#include <array>
#include <optional>
#include <algorithm>
#include <limits>
#include <cmath>
#include <cfenv>
#include <cassert>
#include <bit>
#include <atomic>
#include <cstring> // std::memcpy
#include <utility> // std::cmp_equal

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
    using hint = uint32_t;
    using id_t = uint32_t;
    using arch = size_t;
    using sz_t = ui32;
    using fp32 = float;
    using fp64 = double;
    using flag = std::atomic<bool>;

    constexpr size_t operator "" _sz (unsigned long long i) { return static_cast<size_t>(i); }
    static constexpr auto si32max = std::numeric_limits<si32>::max();
    static constexpr auto ui32max = std::numeric_limits<ui32>::max();
    static constexpr auto si16max = std::numeric_limits<si16>::max();
    static constexpr auto ui16max = std::numeric_limits<ui16>::max();
    static constexpr auto si32min = std::numeric_limits<si32>::min();
    static constexpr auto ui32min = std::numeric_limits<ui32>::min();
    static constexpr auto si16min = std::numeric_limits<si16>::min();
    static constexpr auto ui16min = std::numeric_limits<ui16>::min();
    static constexpr auto debugmode
        #if defined(DEBUG)
        = true;
        #else
        = faux;
        #endif

    static auto _k0 = 0; // LCtrl+Wheel.
    static auto _k1 = 0; // Alt+Wheel.
    static auto _k2 = 0; // LCtrl+Alt+Wheel.
    static auto _k3 = 0; // RCtrl+Wheel.

    struct noop { template<class ...T> constexpr auto operator()(T...) { return faux; }; };

    enum class feed : unsigned char { none, rev, fwd, };

    template<class T>
    using to_signed_t = std::conditional_t<(si64)std::numeric_limits<std::remove_reference_t<T>>::max() <= si16max, si16,
                        std::conditional_t<(si64)std::numeric_limits<std::remove_reference_t<T>>::max() <= si32max, si32, si64>>;

    // intmath: Set a single p-bit to v.
    template<unsigned int P, class T>
    void set_bit(T&& n, bool v)
    {
        n = (n & ~(1 << P)) | (v << P);
    }
    // intmath: Swap two bits.
    template<unsigned int P1, unsigned int P2, class T>
    auto swap_bits(T n)
    {
        auto a = 1 & (n >> P1);
        auto b = 1 & (n >> P2);
        auto x = a ^ b;
        return n ^ (x << P1 | x << P2);
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
            else assert(faux);
            return r;
        }
    }
    // intmath: Convert LE to host endianness.
    template<class T, bool BE = std::endian::native == std::endian::big>
    constexpr auto letoh(T i)
    {
        if constexpr (BE && sizeof(T) > 1) return _swap_bytes(i);
        else                               return i;
    }
    // intmath: Convert BE to host endianness.
    template<class T, bool LE = std::endian::native == std::endian::little>
    constexpr auto betoh(T i)
    {
        if constexpr (LE && sizeof(T) > 1) return _swap_bytes(i);
        else                               return i;
    }
    // intmath: Get the aligned integral value.
    template<class T>
    constexpr auto aligned(void const* ptr)
    {
        auto i = T{};
        std::memcpy((void*)&i, ptr, sizeof(T));
        return letoh(i);
    };
    // intmath: LE type wrapper. T has an LE format in memory.
    template<class T>
    class le_t
    {
        T data = {};

    public:
        void set(T const& v) { data = netxs::letoh(v); }
        T    get() const     { return netxs::letoh(data); }
    };

    // intmath: Value change tracker.
    template<class T>
    struct testy
    {
        T    prev = {};
        T    last = {};
        bool test = faux;

        bool test_and_set(T newvalue)
        {
            prev = last;
            test = last != newvalue;
            if (test) last = newvalue;
            return test;
        }
        bool operator () (T newvalue)
        {
            return test_and_set(newvalue);
        }
        operator auto& ()       { return last; }
        operator auto& () const { return last; }
        auto reset()
        {
            auto temp = test;
            test = faux;
            return temp;
        }
        testy()                          = default;
        testy(testy&&)                   = default;
        testy(testy const&)              = default;
        testy& operator = (testy const&) = default;
        testy(T const& value)
            : prev{ value },
              last{ value },
              test{ faux  }
        { }
    };

    // intmath: Summ and return TRUE in case of
    //          unsigned integer overflow and store result in accum.
    template<class T1, class T2>
    constexpr bool sum_overflow(T1& accum, T2 delta)
    {
        auto prev = accum;
        accum = (T1)(accum + delta);
        return accum <= prev;
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
    struct quadratic
    {
    private:
        using twod = T;
        using type = disintegrate<twod>;

                twod speed; // quadratic: Distance ΔR over time period ΔT.
                type limit; // quadratic: Activity period.
                type phase; // quadratic: Register.
                type scale; // quadratic: Factor.
                type start; // quadratic: Deffered start time.
        mutable twod total; // quadratic: Current point on the path.

    public:
        // Quadratic fader:
        //     speed - distance ΔR over time period ΔT
        //     cycle - time period ΔT
        //     limit - activity period
        //     start - deffered start time
        quadratic(twod speed, type cycle, type limit, type start)
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
    struct constlinear
    {
    private:
        using twod = T;
        using type = disintegrate<twod>;

                type limit; // constlinear: Activity period.
                type phase; // constlinear: Register.
                twod speed; // constlinear: Distance ΔR over time period ΔT.
                type scale; // constlinear: Factor.
                type start; // constlinear: Deffered start time.
        mutable twod total; // constlinear: Current point on the path.

    public:
        // Linear constant speed delta generator:
        //     speed - distance ΔR over time period ΔT
        //     cycle - time period ΔT
        //     limit - activity period
        //     start - deffered start time
        constlinear(twod speed, type cycle, type limit, type start)
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
    struct constlinearAtoB
    {
    private:
        using twod = T;
        using type = disintegrate<twod>;

                type limit; // constlinearAtoB: Activity period.
                twod range; // constlinearAtoB: Path's end point (from 0 to range).
                type start; // constlinearAtoB: Deffered start time point.
        mutable twod total; // constlinearAtoB: Current point on the path.

    public:
        constlinearAtoB(twod range, type limit, type start)
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

    // Cubic spline for three points {0, 0}, {r, 0.5}, {1,1}. For x < 0: F(x) = 0, For x > 1: F(x) = 1. 0 < r < 1.
    // x <= 0 :     S(x) = 0
    // 0 < x < x1 : S(x) = d1 * x^3
    // x <= 1:      S(x) = a2 + b2 * x + c2 * x^2 + d2 * x^3
    // x > 1:       S(x) = 1
    //
    // 0 < r < 1
    // 1. S(0) = 0       a1 = 0
    // 2. S(1) = 1
    // 3. S'(0) = 0      b1 = 0
    // 4. S'(1) = 0
    // 5. S(r) = S(r)
    // 6. S'(r) = S'(r)
    // 7. S''(0) = 0     c1 = 0
    // 8. S''(1) = 0
    template<class T = fp32>
    struct spline01
    {
    private:
        T r;
        T d1, d2;
        T a2, b2, c2;

    public:
        spline01(T r)
            :  r{ r },
              d1{ 1 / (r * r)},
              d2{ 1 / ((r - 1) * (r - 1)) },
              a2{ 1 - d2 }, 
              b2{ 3 * d2},
              c2{ -3 * d2}
        { }

        auto operator () (T x) const
        {
            auto y = T{ x <= 0 ? 0
                      : x < r  ? d1 * x * x * x
                      : x < 1  ? a2 + b2 * x + c2 * x * x  + d2 * x * x * x
                      : 1 };
            return y;
        }
    };
    template<class T = fp32>
    struct spline
    {
    private:
        static constexpr auto zero = T{};
        static constexpr auto one = (T{} + 1) / (T{} + 1);
        T r;
        T d1, d2;
        T a2, b2, c2;
        T offset_x, offset_y, scale_x, scale_y;

    public:
        spline(T r, T offset_x = zero, T offset_y = zero, T scale_x = one, T scale_y = one)
            : r{ r },
              d1{ 1 / (r * r)},
              d2{ 1 / ((r - 1) * (r - 1)) },
              a2{ 1 - d2 }, 
              b2{ 3 * d2},
              c2{ -3 * d2},
            offset_x{ offset_x },
            offset_y{ offset_y },
            scale_x{ scale_x },
            scale_y{ scale_y }
        { }

        auto operator () (T x) const
        {
            //auto y = T{ x <= 0 ? 0
            //          : x < x1 ? d1 * x * x * x
            //          : x < 1  ? a2 + b2 * x + c2 * x * x  + d2 * x * x * x
            //          : 1 };
            //todo precalc abcd
            x -= offset_x;
            x /= scale_x;
            auto y = T{ x <= 0 ? 0
                      : x <  r ? d1 * x * x * x
                      : x <  1 ? a2 + b2 * x + c2 * x * x  + d2 * x * x * x
                      : 1 };
            y *= scale_y;
            y += offset_y;
            return y;
        }
    };

    // intmath: Forward/Reverse (bool template arg) copy the specified
    //          sequence of cells onto the canvas at the specified offset
    //          and return count of copied cells.
    template<bool RtoL, class T1, class T2, class P>
    auto xerox(T1*& frame, T2 const& source, P handle)
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
        auto data1 = canvas.begin();
        auto data2 = bitmap.begin();
        auto data3 = data2;
        if (size1.x == 0 || size1.y == 0
         || size2.x == 0 || size2.y == 0) return;

        auto size11 = decltype(size1){ 1, 1 };
        auto msize0 = size1 - size11;
        auto msize1 = max(size11, msize0);
        auto msize2 = size2 - size11;

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

    // intmath: Project bitmap_view to the canvas_view (with nearest-neighbor interpolation and negative bitmap_size support for mirroring).
    template<class P, class NewlineFx = noop>
    void xform_scale(auto& canvas, auto canvas_rect, auto const& bitmap, auto bitmap_rect, P handle, NewlineFx online = {})
    {
        auto dst_size = canvas.size();
        auto src_size = bitmap.size();
        canvas_rect.coor -= canvas.coor();
        bitmap_rect.coor -= bitmap.coor();
        auto dst_view = canvas_rect.trunc(dst_size);
        auto src_view = bitmap_rect.trunc(src_size);

        if (dst_view.size.x == 0 || dst_view.size.y == 0
         || src_view.size.x == 0 || src_view.size.y == 0) return;

        auto s_00 = src_view.coor.x + src_view.coor.y * src_size.x;
        auto sptr = bitmap.begin();
        auto dptr = canvas.begin() + dst_view.coor.x + dst_view.coor.y * dst_size.x;
        auto step = (src_view.size * 65536) / dst_view.size;
        auto half = step / 2; // Centrify by pixel half.
        dst_view.size -= 1;
        auto tail = dptr + dst_view.size.x;
        auto stop = tail + dst_view.size.y * dst_size.x;
        auto y = half.y;
        while (true)
        {
            auto x = half.x;
            auto s = sptr + (s_00 + (y >> 16) * src_size.x);
            while (true)
            {
                auto xptr = s + (x >> 16);
                handle(*dptr, *xptr);
                if (dptr == tail) break;
                dptr++;
                x += step.x;
            }
            online();
            if (dptr == stop) break;
            tail += dst_size.x;
            dptr = tail - dst_view.size.x;
            y += step.y;
        }
    }

    // intmath: Project bitmap_rect to the canvas_rect_coor (with nearest-neighbor interpolation and support for negative bitmap_rect.size to mirroring/flipping).
    template<class P, class NewlineFx = noop>
    void xform_mirror(auto& canvas, auto canvas_rect_coor, auto const& bitmap, auto bitmap_rect, P handle, NewlineFx online = {})
    {
        auto dst_size = canvas.size();
        auto src_size = bitmap.size();
        canvas_rect_coor -= canvas.coor();
        bitmap_rect.coor -= bitmap.coor();
        auto src_view = bitmap_rect.trunc(src_size);
        bitmap_rect.coor = canvas_rect_coor;
        auto dst_view = bitmap_rect.normalize().trunc(dst_size);

        if (dst_view.size.x == 0 || dst_view.size.y == 0
         || src_view.size.x == 0 || src_view.size.y == 0) return;

        auto dx = 1;
        auto dy = std::abs(src_size.x);
        if (src_view.size.x < 0) { dx = -dx; src_view.coor.x -= 1; }
        if (src_view.size.y < 0) { dy = -dy; src_view.coor.y -= 1; }
        
        dst_view.size -= 1;
        auto sptr = bitmap.begin() + (src_view.coor.x + src_view.coor.y * src_size.x);
        auto dptr = canvas.begin() + (dst_view.coor.x + dst_view.coor.y * dst_size.x);
        auto tail = dptr + dst_view.size.x;
        auto stop = tail + dst_view.size.y * dst_size.x;
        while (true)
        {
            auto xptr = sptr;
            while (true)
            {
                handle(*dptr, *xptr);
                if (dptr == tail) break;
                dptr++;
                xptr += dx;
            }
            online();
            if (dptr == stop) break;
            sptr += dy;
            tail += dst_size.x;
            dptr = tail - dst_view.size.x;
        }
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
            auto data1 = bitmap1.begin();
            auto data2 = bitmap2.begin();

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
    template<bool RtoL, class T, class D, class R, class C, class P, class NewlineFx = noop>
    void inbody(T& canvas, D const& bitmap, R const& region, C const& base2, P handle, NewlineFx online = {})
    {
        if (region.size.y == 0) return;
        auto& base1 = region.coor;

        auto& size1 = canvas.size();
        auto& size2 = bitmap.size();

        auto  data1 = canvas.begin() + base1.x + base1.y * size1.x;
        auto  data2 = bitmap.begin() + base2.x + base2.y * size2.x;

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

        auto bound = data1 + region.size.x;
        auto limit = bound + (region.size.y - 1) * size1.x;
        while (true)
        {
            while (data1 != bound)
            {
                if constexpr (RtoL) handle(*data1++, *--data2);
                else                handle(*data1++, *data2++);
            }
            online();
            if (data1 == limit) break;
            bound += size1.x;//= data1 + region.size.x;
            data1 += skip1;
            data2 += skip2;
        }
    }

    // intmath: Wild bitmap.
    template<class T, class Rect>
    struct raster
    {
        using base = T;
        base _data;
        Rect _area;
        auto  length() const { return _data.length(); }
        auto  begin()        { return _data.begin();  }
        auto  end()          { return _data.end();    }
        auto  begin()  const { return _data.begin();  }
        auto  end()    const { return _data.end();    }
        auto& size()         { return _area.size;     }
        auto& area()         { return _area;          }
        auto& size()   const { return _area.size;     }
        auto& area()   const { return _area;          }
        raster() = default;
        raster(T data, Rect area)
            : _data{ data },
              _area{ area }
        { }
        void resize(auto new_size, auto filler = {})
        {
            _area.size = new_size;
            _data.resize(new_size.x * new_size.y, filler);
        }
    };

    // intmath: Intersect two sprites and invoking
    //          handle(sprite1_element, sprite2_element)
    //          for each elem in the intersection.
    template<class T, class D, class P, class NewlineFx = noop>
    void onbody(T& canvas, D const& bitmap, P handle, NewlineFx online = {})
    {
        auto& rect1 = canvas.area();
        auto& rect2 = bitmap.area();

        if (auto joint = rect1.clip(rect2))
        {
            auto basis = joint.coor - rect2.coor;
            joint.coor-= rect1.coor;

            inbody<faux>(canvas, bitmap, joint, basis, handle, online);
        }
    }

    // intmath: Draw the rectangle region inside the canvas by
    //          invoking handle(canvas_element)
    //          (without boundary checking).
    template<bool RtoL = faux, class T, class Rect, class P, class NewlineFx = noop, bool Plain = std::is_same_v<void, std::invoke_result_t<P, decltype(*(std::declval<T&>().begin()))>>>
    void onrect(T& canvas, Rect const& region, P handle, NewlineFx online = {})
    {
        auto& place = canvas.area();
        if (auto joint = region.clip(place))
        {
            auto basis = joint.coor - place.coor;
            auto frame = place.size.x * basis.y + basis.x + canvas.begin();
            auto notch = place.size.x - joint.size.x;
            auto limit = place.size.x * (joint.size.y - 1) + frame + joint.size.x;
            while (true)
            {
                auto bound = frame + joint.size.x;
                while (bound != frame)
                {
                    if constexpr (RtoL)
                    {
                        if constexpr (Plain) handle(*--bound);
                        else             if (handle(*--bound)) return;
                    }
                    else
                    {
                        if constexpr (Plain) handle(*frame++);
                        else             if (handle(*frame++)) return;
                    }
                }
                if constexpr (RtoL) frame += joint.size.x;
                online();
                if (frame == limit) break;
                frame += notch;
            }
        }
    }

    static inline
    bool liang_barsky(fp32 xmin, fp32 ymin, fp32 xmax, fp32 ymax,
                      fp32&  x1, fp32&  y1, fp32&  x2, fp32&  y2)
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

        auto gain = static_cast<ui16>( 0 );
        auto   dx = static_cast<si32>( p1.x - p0.x );
        auto   dy = static_cast<si32>( p1.y - p0.y );
        auto   lx = static_cast<ui32>( std::abs(dx) );
        auto   ly = static_cast<ui32>( std::abs(dy) );

        rect = rect.normalize();
        auto& coor = rect.coor;
        auto& size = rect.size;
        p0 -= coor;
        p1 -= coor;

        auto set = [&](auto const& p, auto k)
        {
            if (size.inside(p))
            {
                pset(p + coor, k);
            }
        };
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
            auto x1 = (fp32)p0.x; auto y1 = (fp32)p0.y;
            auto x2 = (fp32)p1.x; auto y2 = (fp32)p1.y;
            auto minx = -1.0f;// One element wide margin for antialiasing.
            auto miny = -1.0f;//
            auto maxx = size.x + 1.0f;
            auto maxy = size.y + 1.0f;

            if (liang_barsky(minx, miny, maxx, maxy, x1, y1, x2, y2))
            {
                auto align = [](auto& oldx, auto& oldy, auto maxx, auto maxy, auto newy, bool dirx)
                {
                    auto delta = (maxx << 16) / maxy * std::abs(newy - oldy);
                    auto error = delta >> 16;
                    dirx ? oldx += error
                         : oldx -= error;
                    oldy = newy;
                    return (ui16)delta;
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

    // 1D box-blur.
    // To achieve a 2D blur, it needs to apply it again and swap the X with Y, and source with destination.
    //
    // Accum_t       Point accumulator type to avoid overflow.
    // Calc          Whether do the division in the current run. Performance burst by 40% !
    // InnerGlow     Using the left/rightmost pixel value to approximate the image boundary.
    // s_ptr         Source bitmap pointer.
    // d_ptr         Destination bitmap pointer.
    // w             Bitmap width.
    // h             Bitmap height.
    // r             Horizontal blur radius.
    // s_dtx         Index step along X in the source.
    // s_dty         Index step along Y in the source.
    // d_dtx         Index step along X in the destination.
    // d_dty         Index step along Y in the destination.
    // P_Base s_ref  Lambda to convert the src pointer to the reference.
    // P_Dest d_ref  Lambda to convert the dst pointer to the reference.
    // PostFx shade  Lambda for postprocessing.
    template<class Accum_t, bool InnerGlow, bool Calc,
        class Src_t,
        class Dst_t, class Int_t,
        class P_Base, class P_Dest, class PostFx = noop>
    void boxblur1d(Src_t s_ptr,
                   Dst_t d_ptr, Int_t w,
                                Int_t h, Int_t r, Int_t s_dtx, Int_t s_dty,
                                                  Int_t d_dtx, Int_t d_dty, auto count,
        P_Base s_ref, P_Dest d_ref, PostFx shade = {})
    {
        auto r1 = r + 1;
        auto limit = s_ptr + s_dty * (h - 1);
        auto s_hop = s_dtx * (w - 1);
        auto d_hop = d_dtx * (w - 1);
        auto width = r1 + r;
        auto debug = [&]([[maybe_unused]] auto& accum)
        {
            if constexpr (debugmode)
            {
                auto n = accum / count;
                if (n > 255) throw;
            }
        };
        if (w <= r1) // All pixels on a line have the same average value.
        {
            auto s_end = s_ptr + s_hop;
            auto d_end = d_ptr + d_hop;;
            while (true)
            {
                auto accum = Accum_t{};
                auto s_cur = s_ptr;
                auto d_cur = d_ptr;
                while (true)
                {
                    accum += s_ref(s_cur);
                    if (s_cur == s_end) break;
                    s_cur += s_dtx;
                }
                auto value = accum * width / w;
                if constexpr (Calc) value /= count;
                while (true)
                {
                    d_ref(d_cur) = value;
                    shade(*d_cur);
                    if (d_cur == d_end) break;
                    d_cur += d_dtx;
                }
                if (s_ptr == limit) break;
                s_ptr += s_dty;
                d_ptr += d_dty;
                s_end += s_dty;
                d_end += d_dty;
            }
        }
        else // if (w > r + 1)
        {
            auto small = width >= w;
            auto s_r0x = r * s_dtx;
            auto d_r0x = r * d_dtx;
            auto w_r_1 = w - r1;
            auto s_w1r = w_r_1 * s_dtx;
            auto d_w1r = w_r_1 * d_dtx;
            auto s_r1x = s_r0x + s_dtx;
            auto s_wdx = s_r1x + s_r0x;//width * s_dtx;
            auto d_rev = (width - w) * d_dtx;
            auto d_fwd = -(d_rev + d_dtx);
            auto d_val = small ? d_w1r : d_r0x;
            while (true)
            {
                // Find the average on the left side.
                auto accum = Accum_t{};
                auto l_val = Accum_t{};
                auto r_val = Accum_t{};
                auto s_cur = s_ptr;
                auto s_end = s_ptr + s_r0x;
                if constexpr (InnerGlow) l_val = s_ref(s_cur);
                while (true)
                {
                    accum += s_ref(s_cur);
                    if (s_cur == s_end) break;
                    s_cur += s_dtx;
                }
                if constexpr (!InnerGlow) l_val = accum / r1;
                // Find the average on the right side.
                s_end = s_ptr + s_hop;
                if constexpr (InnerGlow) r_val = s_ref(s_end);
                else
                {
                    s_cur = s_ptr + s_w1r;
                    while (true)
                    {
                        r_val += s_ref(s_cur);
                        if (s_cur == s_end) break;
                        s_cur += s_dtx;
                    }
                    r_val /= r1;
                }
                auto d_cur = d_ptr;
                auto d_end = d_cur;
                accum += l_val * r; // Leftmost pixel value.
                // Sub l_val, add src.
                s_cur = s_ptr + s_r1x;
                d_end += d_val;//d_w1r;
                debug(accum);
                d_ref(d_cur) = Calc ? accum / count : accum;
                shade(*d_cur);
                d_cur += d_dtx;
                while (true)
                {
                    accum -= l_val;
                    accum += s_ref(s_cur);
                    debug(accum);
                    d_ref(d_cur) = Calc ? accum / count : accum;
                    shade(*d_cur);
                    if (d_cur == d_end) break;
                    s_cur += s_dtx;
                    d_cur += d_dtx;
                }
                if (small) // Sub l_val, add r_val.
                {
                    d_cur += d_dtx;
                    d_end = d_cur + d_rev;
                    while (true)
                    {
                        accum -= l_val;
                        accum += r_val;
                        debug(accum);
                        d_ref(d_cur) = Calc ? accum / count : accum;
                        shade(*d_cur);
                        if (d_cur == d_end) break;
                        d_cur += d_dtx;
                    }
                    s_cur = s_ptr;
                }
                else // Sub src, add src.
                {
                    d_cur += d_dtx;
                    d_end = d_cur + d_fwd;
                    auto s_fwd = s_ptr + s_wdx;
                    s_cur = s_ptr;
                    accum -= s_ref(s_cur);
                    accum += s_ref(s_fwd);
                    debug(accum);
                    d_ref(d_cur) = Calc ? accum / count : accum;
                    shade(*d_cur);
                    while (d_cur != d_end)
                    {
                        s_cur += s_dtx;
                        s_fwd += s_dtx;
                        d_cur += d_dtx;
                        accum -= s_ref(s_cur);
                        accum += s_ref(s_fwd);
                        debug(accum);
                        d_ref(d_cur) = Calc ? accum / count : accum;
                        shade(*d_cur);
                    }
                    s_cur += s_dtx;
                    d_cur += d_dtx;
                }
                // Sub src, add r_val.
                d_end = d_ptr + d_hop;
                while (true)
                {
                    accum -= s_ref(s_cur);
                    accum += r_val;
                    debug(accum);
                    d_ref(d_cur) = Calc ? accum / count : accum;
                    shade(*d_cur);
                    if (d_cur == d_end) break;
                    s_cur += s_dtx;
                    d_cur += d_dtx;
                }
                if (s_ptr == limit) break;
                s_ptr += s_dty;
                d_ptr += d_dty;
            }
        }
    }

    // intmath: Move block to the specified destination. If begin_it > end_it (exclusive) decrement is used.
    template<bool Fwd, class Src, class Dst, class P>
    void proc_block(Src begin_it, Src end_it, Dst dest_it, P proc)
    {
            while (begin_it != end_it)
            {
                if constexpr (Fwd)
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
    template<bool Fwd = true, class Src, class Dst>
    void move_block(Src begin_it, Src end_it, Dst dest_it)
    {
        proc_block<Fwd>(begin_it, end_it, dest_it, [](auto& src, auto& dst){ dst = std::move(src); });
    }
    template<bool Fwd = true, class Src, class Dst>
    void swap_block(Src begin_it, Src end_it, Dst dest_it)
    {
        proc_block<Fwd>(begin_it, end_it, dest_it, [](auto& src, auto& dst){ std::swap(src, dst); });
    }

    // Boxblur.
    //
    // Accum_t       Point accumulator type.
    // InnerGlow     Using the left/rightmost pixel value to approximate the image boundary.
    // s_ptr         Source bitmap array pointer.
    // d_ptr         Destination bitmap array pointer.
    // w             Bitmap width.
    // h             Bitmap height.
    // r             Bokeh radius.
    // s_dty         Index step along Y in the source.
    // d_dty         Index step along Y in the destination.
    // ratio         X/Y axis ratio (2 for text cells, 1 for pixels).
    // P_Base s_ref  Lambda to convert src pointer to the reference.
    // P_Dest d_ref  Lambda to convert dst pointer to the reference.
    // PostFx shade  Lambda for postprocessing.
    template<class Accum_t, bool InnerGlow = faux,
        class Src_t,
        class Dst_t, class Int_t,
        class P_Base, class P_Dest, class PostFx = noop>
    void boxblur(Src_t s_ptr,
                 Dst_t d_ptr, Int_t w,
                              Int_t h, Int_t r, Int_t s_dty,
                                                Int_t d_dty, Int_t ratio,
        P_Base s_ref, P_Dest d_ref, PostFx shade = {})
    {
        if (h <= 0 || w <= 0 || r <= 0) return;
        auto rx = r * ratio;
        auto ry = r;
        //for (auto i = 0; i < 1000; i++)                                                // Performance test: int main(int argc, char* argv[]) {
        {                                                                                //     auto a = ::CommandLineToArgvW(GetCommandLineW(), &argc);
        auto count = rx + rx + 1;                                                        //     auto x = utf::to_int(utf::to_utf(a[1])).value();
        boxblur1d<Accum_t, InnerGlow, 0>(s_ptr,    // blur horizontally and place        //     auto y = utf::to_int(utf::to_utf(a[2])).value();
                                         d_ptr, w, // result to the temp buffer (d_ptr)  //     auto t = utf::to_int(utf::to_utf(a[3])).value();
                                                h, rx, 1, s_dty,                         //     std::cout << "mx: " << x << " my: " << y << " threads: " << t << "\n";
                                                       1, d_dty, count, s_ref,           //     auto w = [&]{ auto test = ui::face{};
                                                                        d_ref);          //                   test.size({ x, y });
        count *= ry + ry + 1;                                                            //                   auto start = datetime::now();
        boxblur1d<Accum_t, InnerGlow, 1>(d_ptr,    // blur vertically and place          //                   test.blur(1);
                                         s_ptr, h, // result back to the source (s_ptr)  //                   auto delta = datetime::now() - start;
                                                w, ry, d_dty, 1,                         //                   std::cout << datetime::round<si32>(delta) << "ms\n"; };
                                                       s_dty, 1, count, d_ref,           //     auto works = std::list<std::thread>{};
                                                                        s_ref, shade);   //     while (t--) works.emplace_back(w);
        }                                                                                //     for (auto& t : works) t.join();
    }
    template<class T, bool InnerGlow = faux>
    void boxblur(T& bitmap, si32 r, auto area, auto clip)
    {
        using type = std::decay_t<decltype(*bitmap.begin())>;
        auto w = std::max(0, clip.size.x);
        auto h = std::max(0, clip.size.y);
        auto s = w * h;
        auto buffer = T::base(s);
        auto coor = clip.coor - area.coor;
        auto s_ptr = bitmap.begin() + coor.x * coor.y;
        auto d_ptr = buffer.begin();
        auto s_width = area.size.x;
        auto d_width = clip.size.x;
        auto d_point = [](type* c)->auto& { return *c; };
        netxs::boxblur<type, InnerGlow>(s_ptr,
                                        d_ptr, w,
                                               h, r, s_width,
                                                     d_width, 1, d_point,
                                                                 d_point);
    }
}