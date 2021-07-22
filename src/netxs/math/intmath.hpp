// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_INTMATH_HPP
#define NETXS_INTMATH_HPP

#include <optional>
#include <algorithm>
#include <limits>
#include <cmath>
#include <cfenv>

#ifndef faux
    #define faux (false)
#endif

namespace netxs
{
    constexpr size_t operator "" _sz (unsigned long long i)	{ return i; }

    struct noop { template<class ...T> void operator()(T...) {}; };

    template <class T>
    using to_signed_t = std::conditional_t<(int64_t)std::numeric_limits<std::remove_reference_t<T>>::max() <= std::numeric_limits<int16_t>::max(), int16_t,
                        std::conditional_t<(int64_t)std::numeric_limits<std::remove_reference_t<T>>::max() <= std::numeric_limits<int32_t>::max(), int32_t, int64_t>>;

    // intmath: Summ and return TRUE in case of
    //          unsigned integer overflow and store result in accum.
    template<class T1, class T2>
    bool sum_overflow(T1& accum, T2 delta)
    {
        auto store = accum;
        accum += delta;
        return accum <= store ? true : faux;
    }

    // intmath: Clamp a value in case it exceeds its numerical limits.
    template<class T, class L>
    T clamp(L value)
    {
        static_assert(std::is_integral<T>::value, "Integral type only");
        static_assert(std::is_integral<L>::value, "Integral type only");

        if constexpr (sizeof(T) < sizeof(L))
        {
            static constexpr L max = std::numeric_limits<T>::max();
            return static_cast<T>(std::min(value, max));
        }
        else
        {
            return value;
        }
    }

    template<class T1, class T2, class T3 = T2>
    T3 divround(T1 n, T2 d)
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
    T3 divupper(T1 n, T2 d)
    {
        static_assert(std::is_integral<T1>::value, "Integral type only");
        static_assert(std::is_integral<T2>::value, "Integral type only");
        static_assert(std::is_integral<T3>::value, "Integral type only");

        return	n > 0
            ?	1 + (n - 1) / d
            :	n / d;
    }

    template<class T1, class T2, class T3 = T2>
    T3 divfloor(T1 n, T2 d)
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

    // intmath: Delta sequence generator.
    //          The QUADRATIC-LAW fader from the initial velocity
    //          to stop for a given period of time.
    template<class T>
    class quadratic
    {
        using twod = T;
        using iota = disintegrate<twod>;

        twod speed;
        iota limit;
        iota phase;
        iota scale;
        iota start; // quadratic: Deffered start time.

        mutable twod total;

    public:
        /*
        Quadratic fader ctor:
            speed - distance ΔR over time period ΔT
            cycle - time period ΔT
            limit - activity period
            start - deffered start time
        */
        quadratic(twod const& speed, iota cycle, iota limit, iota start)
            :	speed{ speed         },
                limit{ limit         },
                phase{ limit * 2     },
                scale{ phase * cycle },
                start{ start         },
                total{ twod{}        }
        { }

        auto operator ()(iota timer) const
        {
            std::optional<twod> delta;

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

    // intmath: Delta sequence generator.
    //          The LINEAR-LAW fader from the initial velocity
    //          to stop for a given period of time with constant speed.
    template<class T>
    class constlinear
    {
        using twod = T;
        using iota = disintegrate<twod>;

        iota limit;
        iota phase;
        twod speed;
        iota scale;
        iota start; // constlinear: Deffered start time.

        mutable twod total;

    public:
        /*
        Linear constant speed delta generator ctor:
            speed - distance ΔR over time period ΔT
            cycle - time period ΔT
            limit - activity period
            start - deffered start time
        */
        constlinear(twod const& speed, iota cycle, iota limit, iota start)
            :	limit{ limit         },
                phase{ limit * 2     },
                speed{ speed * phase },
                scale{ cycle * phase },
                start{ start         },
                total{ twod{}        }
        { }

        auto operator ()(iota timer) const
        {
            std::optional<twod> delta;

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

    // intmath: Delta sequence generator.
    //          The LINEAR-LAW fader from the initial coord to the destination
    //          coord for a given period of time with constant speed.
    template<class T>
    class constlinearAtoB
    {
        using twod = T;
        using iota = disintegrate<twod>;

        iota limit;
        twod range;
        iota start; // constlinear: Deffered start time.

        mutable twod total;

    public:
        /*
        Linear constant speed delta generator ctor:
        range - all path (from 0 to range)
        limit - activity period
        start - deffered start time
        */
        constlinearAtoB(twod const& range, iota limit, iota start)
            :	limit{ limit },
                range{ range },
                start{ start },
                total{ twod{}}
        { }

        auto operator ()(iota timer) const
        {
            std::optional<twod> delta;

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

        auto  limit = frame + width;
        while(limit!= frame)
        {
            if constexpr (RtoL) handle(*frame++, *--lyric);
            else                handle(*frame++, *lyric++);
        }

        return width;
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

            auto  limit = data1 + size1.y * size2.x;
            while(limit!= data1)
            {
                handle(*data1++, *data2++);
            }
        }
    }

    // intmath: Intersect two sprites and
    //          invoking handle(sprite1_element, sprite2_element)
    //          for each elem in the intersection.
    template<bool RtoL, class T, class R, class C, class P>
    void inbody(T& canvas, T const& bitmap, R const& region, C const& base2, P handle)
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

        auto  limit = data1 + region.size.y * size1.x;
        while(limit!= data1)
        {
            auto  limit = data1 + region.size.x;
            while(limit!= data1)
            {
                if constexpr (RtoL) handle(*data1++, *--data2);
                else                handle(*data1++, *data2++);
            }
            data1 += skip1;
            data2 += skip2;
        }
    }

    // intmath: Intersect two sprites and invoking
    //          handle(sprite1_element, sprite2_element)
    //          for each elem in the intersection.
    template<class T, class P>
    void onbody(T& canvas, T const& bitmap, P handle)
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
    template<class T, class RECT, class P, class NEWLINEFX = noop>
    void onrect(T& canvas, RECT const& region, P handle, NEWLINEFX online = NEWLINEFX())
    {
        auto& place = canvas.area();

        if (auto joint = region.clip(place))
        {
            auto basis = joint.coor - place.coor;
            auto frame = place.size.x * basis.y + basis.x + canvas.data();
            auto notch = place.size.x - joint.size.x;

            auto  limit = place.size.x * joint.size.y + frame;
            while(limit!= frame)
            {
                auto  limit = frame + joint.size.x;
                while(limit!= frame)
                {
                    handle(*frame++);
                }
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

        if ((dx == 0.0f && left < 0.0f) ||
            (dy == 0.0f && top  < 0.0f)) return faux; // Line is parallel to rectangle.

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
        using iota = disintegrate<twod>;

        uint16_t gain = 0;
        int32_t  dx = p1.x - p0.x;
        int32_t  dy = p1.y - p0.y;
        uint32_t lx = std::abs(dx);
        uint32_t ly = std::abs(dy);

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
                    if (dx > 0) line(p0, p1, incr, incr);
                    else        line(p0, p1, decr, incr);
                else
                    if (dx > 0) line(p0, p1, incr, decr);
                    else        line(p0, p1, decr, decr);
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

                p1.x = static_cast<iota>(x2);
                p1.y = static_cast<iota>(y2);
                auto new_x = static_cast<iota>(x1);
                auto new_y = static_cast<iota>(y1);

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