// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

#include "geometry.hpp"
#include "logger.hpp"

namespace netxs
{
    enum class svga
    {
        vt_2D,
        vtrgb,
        vt256,
        vt16 ,
        nt16 ,
        dtvt ,
    };

    enum class bias : byte { none, left, right, center, };
    enum class wrap : byte { none, on,  off,            };
    enum class rtol : byte { none, rtl, ltr,            };

    namespace zpos
    {
        static constexpr auto backmost = -1;
        static constexpr auto plain    = 0;
        static constexpr auto topmost  = 1;
    };

    namespace unln
    {
        static constexpr auto none   = 0;
        static constexpr auto line   = 1;
        static constexpr auto biline = 2;
        static constexpr auto wavy   = 3;
        static constexpr auto dotted = 4;
        static constexpr auto dashed = 5;
    }

    namespace scale_mode
    {
        static constexpr auto inside  = 0;
        static constexpr auto outside = 1;
        static constexpr auto stretch = 2;
        static constexpr auto none    = 3;
    }
    namespace align_mode                         // horizontal            vertical
    {
        static constexpr auto center        = (si32)bias::center | ((si32)bias::center << 2);
        static constexpr auto left          = (si32)bias::left   | ((si32)bias::center << 2);
        static constexpr auto right         = (si32)bias::right  | ((si32)bias::center << 2);
        static constexpr auto middle        = (si32)bias::center | ((si32)bias::center << 2);
        static constexpr auto top           = (si32)bias::center | ((si32)bias::left   << 2);
        static constexpr auto bottom        = (si32)bias::center | ((si32)bias::right  << 2);
        static constexpr auto center_middle = (si32)bias::center | ((si32)bias::center << 2);
        static constexpr auto center_top    = (si32)bias::center | ((si32)bias::left   << 2);
        static constexpr auto center_bottom = (si32)bias::center | ((si32)bias::right  << 2);
        static constexpr auto left_middle   = (si32)bias::left   | ((si32)bias::center << 2);
        static constexpr auto left_top      = (si32)bias::left   | ((si32)bias::left   << 2);
        static constexpr auto left_bottom   = (si32)bias::left   | ((si32)bias::right  << 2);
        static constexpr auto right_middle  = (si32)bias::right  | ((si32)bias::center << 2);
        static constexpr auto right_top     = (si32)bias::right  | ((si32)bias::left   << 2);
        static constexpr auto right_bottom  = (si32)bias::right  | ((si32)bias::right  << 2);
    }

    namespace text_cursor
    {
        static constexpr auto none      = 0;
        static constexpr auto underline = 1;
        static constexpr auto block     = 2;
        static constexpr auto I_bar     = 3;
    }

    enum tint
    {
        blackdk, reddk, greendk, yellowdk, bluedk, magentadk, cyandk, whitedk,
        blacklt, redlt, greenlt, yellowlt, bluelt, magentalt, cyanlt, whitelt,
        pureblack   = 16 + 36 * 0 + 6 * 0 + 0,
        purewhite   = 16 + 36 * 5 + 6 * 5 + 5,
        purered     = 16 + 36 * 5 + 6 * 0 + 0,
        puregreen   = 16 + 36 * 0 + 6 * 5 + 0,
        pureblue    = 16 + 36 * 0 + 6 * 0 + 5,
        pureyellow  = 16 + 36 * 5 + 6 * 5 + 0,
        purecyan    = 16 + 36 * 0 + 6 * 5 + 5,
        puremagenta = 16 + 36 * 5 + 6 * 0 + 5,
    };

    struct tint16
    {
        enum : si32
        {
            blackdk,  // 0
            blacklt,  // 1
            graydk,   // 2
            graylt,   // 3
            whitedk,  // 4
            whitelt,  // 5
            reddk,    // 6
            bluedk,   // 7
            greendk,  // 8
            yellowdk, // 9
            magentalt,// 10
            cyanlt,   // 11
            redlt,    // 12
            bluelt,   // 13
            greenlt,  // 14
            yellowlt, // 15
        };
    };

    // canvas: 8-bit ARGB.
    union argb
    {
        ui32                        token;
        struct { byte b, g, r, a; } chan;

        constexpr argb()
            : token{ 0 }
        { }
        constexpr argb(argb const&) = default;
        template<class T, class A = byte>
        constexpr argb(T r, T g, T b, A a = 0xff)
            : chan{ static_cast<byte>(b),
                    static_cast<byte>(g),
                    static_cast<byte>(r),
                    static_cast<byte>(a) }
        { }
        constexpr argb(fp32 r, fp32 g, fp32 b, fp32 a = 1.f)
            : chan{ netxs::saturate_cast<byte>(b * 255.0f + 0.5f),
                    netxs::saturate_cast<byte>(g * 255.0f + 0.5f),
                    netxs::saturate_cast<byte>(r * 255.0f + 0.5f),
                    netxs::saturate_cast<byte>(a * 255.0f + 0.5f) }
        { }
        template<class T>
        constexpr argb(T const& c)
            : argb(c.b, c.g, c.r, c.a)
        { }
        constexpr argb(ui32 c)
            : token{ netxs::letoh(c) }
        { }
        constexpr argb(si32 c)
            : argb{ (ui32)c }
        { }
        constexpr argb(tint c)
            : argb{ argb::vt256[c] }
        { }
        void parse_input(fifo& q, auto eval_token_fx)
        {
            static constexpr auto mode_RGB = 2;
            static constexpr auto mode_256 = 5;
            auto mode = q.rawarg(mode_RGB);
            if (fifo::issub(mode))
            {
                switch (fifo::desub(mode))
                {
                    case mode_RGB:
                    {
                        auto r = q.subarg(-1); // Skip the case with color space: \x1b[38:2::255:255:255:::m.
                        chan.r = (byte)(r == -1 ? q.subarg(0) : r);
                        chan.g = (byte)(q.subarg(0));
                        chan.b = (byte)(q.subarg(0));
                        chan.a = (byte)(q.subarg(0xFF));
                        break;
                    }
                    case mode_256:
                        token = eval_token_fx(q.subarg(0));
                        break;
                    default:
                        break;
                }
            }
            else
            {
                switch (mode)
                {
                    case mode_RGB:
                        chan.r = (byte)(q(0));
                        chan.g = (byte)(q(0));
                        chan.b = (byte)(q(0));
                        chan.a = 0xFF;
                        break;
                    case mode_256:
                        token = eval_token_fx(q(0));
                        break;
                    default:
                        break;
                }
            }
        }
        argb(fifo& q)
        {
            parse_input(q, [](si32 i){ return netxs::letoh(argb::vt256[i & 0xFF]); });
        }

        constexpr argb& operator = (argb const&) = default;
        constexpr explicit operator bool () const
        {
            return token;
        }
        constexpr auto operator == (argb c) const
        {
            return token == c.token;
        }
        constexpr auto operator != (argb c) const
        {
            return !operator==(c);
        }
        // argb: Get the token for the indexed color.
        static auto get_indexed_color_token(si32 i)
        {
            return netxs::letoh(argb::indexed_color + (i & 0xFF)); // Indexed color format: a=0, r=255, g=0, b=i
        }
        // argb: Set the token for the indexed color.
        static auto set_indexed_color(argb& c, si32 i)
        {
            c.token = argb::get_indexed_color_token(i);
        }
        // argb: Check if color id indexed.
        static auto is_indexed_color(argb c)
        {
            auto ok = (c.token & netxs::letoh(argb::indexed_mask)) == netxs::letoh(argb::indexed_color); // Check if it is in an indexed color format.
            return ok ? c.chan.b + 1 : 0;
        }
        auto is_indexed() const
        {
            return argb::is_indexed_color(*this);
        }
        // argb: Unpack true color.
        static auto unpack_indexed_color(argb c, auto& ext_vt256)
        {
            if (auto index = argb::is_indexed_color(c))
            {
                return argb{ ext_vt256[index - 1] };
            }
            else
            {
                return c;
            }
        }
        auto& unpack_indexed_color(auto& ext_vt256)
        {
            if (auto index = is_indexed())
            {
                token = netxs::letoh(ext_vt256[index - 1]);
            }
            return *this;
        }
        auto& unpack_indexed_color(auto& ext_vt256, argb def_clr)
        {
            if (token == 0) // argb::transparent
            {
                token = def_clr.token;
            }
            else if (auto index = is_indexed())
            {
                token = netxs::letoh(ext_vt256[index - 1]);
            }
            return *this;
        }
        auto& swap_rb()
        {
            token = ((token >>  0) & 0xFF'00'FF'00) |
                    ((token >> 16) & 0x00'00'00'FF) |
                    ((token << 16) & 0x00'FF'00'00);
            return *this;
        }
        static auto swap_rb(ui32 c)
        {
            return ((c >>  0) & 0x00'FF'00) |
                   ((c >> 16) & 0x00'00'FF) |
                   ((c << 16) & 0xFF'00'00);
        }
        // argb: Set all channels to zero.
        void wipe()
        {
            token = 0;
        }
        // argb: Set color to opaque black.
        void rst()
        {
            static constexpr auto colorblack = argb{ 0xFF000000 };
            token = colorblack.token;
        }
        // argb: Are the colors alpha blenable?
        auto is_alpha_blendable() const
        {
            return chan.a && chan.a != 0xFF;
        }
        // argb: Set alpha channel.
        auto& alpha(si32 k)
        {
            chan.a = (byte)k;
            return *this;
        }
        auto& alpha(fp32 k)
        {
            chan.a = (byte)std::clamp(k * 255.f, 0.f, 255.f);
            return *this;
        }
        // argb: Sum alpha channel.
        auto& alpha_sum(si32 k)
        {
            chan.a = (byte)std::clamp(chan.a + k, 0, 255);
            return *this;
        }
        // argb: Sum alpha channel.
        auto& alpha_sum(fp32 k)
        {
            chan.a = (byte)std::clamp(chan.a + k * 255.f, 0.f, 255.f);
            return *this;
        }
        // argb: Sum alpha channels.
        static void alpha_mix(si32 src, byte& dst)
        {
            dst = (byte)std::clamp(src + dst, 0, 255);
        }
        // argb: Return alpha channel.
        auto alpha() const
        {
            return chan.a;
        }
        // argb: Colourimetric (perceptual luminance-preserving) conversion to greyscale.
        template<class T>
        static constexpr auto luma(T r, T g, T b)
        {
            return static_cast<T>(0.2627f * r + 0.6780f * g + 0.0593f * b);
        }
        constexpr auto luma() const
        {
            //todo this requires conversion to the linear rgb space (error ~20-30%)
            auto r = (byte)(token >> 16);
            auto g = (byte)(token >>  8);
            auto b = (byte)(token >>  0);
            return argb::luma(r, g, b);
        }
        void grayscale()
        {
            auto l = luma();
            chan.r = l;
            chan.g = l;
            chan.b = l;
        }
        // argb: Return 256-color 6x6x6 cube.
        auto to_256cube() const
        {
            auto clr = 0;
            if (chan.r == chan.g && chan.r == chan.b)
            {
                clr = 232 + ((chan.r * 24) >> 8);
            }
            else
            {
                clr = 16 + 36 * ((chan.r * 6) >> 8)
                         +  6 * ((chan.g * 6) >> 8)
                              + ((chan.b * 6) >> 8);
            }
            return (byte)clr;
        }
        // argb: Equal both to their average.
        void avg(argb& c)
        {
            chan.r = c.chan.r = (byte)(((ui32)chan.r + c.chan.r) >> 1);
            chan.g = c.chan.g = (byte)(((ui32)chan.g + c.chan.g) >> 1);
            chan.b = c.chan.b = (byte)(((ui32)chan.b + c.chan.b) >> 1);
        }
        // argb: One-side alpha blending ARGB colors.
        void inline mix_one(argb c)
        {
            if (c.chan.a == 0xFF)
            {
                chan = c.chan;
            }
            else if (c.chan.a)
            {
                auto blend = [](auto c1, auto c2, auto alpha)
                {
                    return (byte)(((c1 << 8) + (c2 - c1) * alpha) >> 8);
                };
                chan.r = blend(chan.r, c.chan.r, c.chan.a);
                chan.g = blend(chan.g, c.chan.g, c.chan.a);
                chan.b = blend(chan.b, c.chan.b, c.chan.a);

                //if (!chan.a) chan.a = c.chan.a;
            }
        }
        // argb: Alpha blending ARGB colors via linear RGB space.
        void inline mix_linear(argb c, si32 cache_index = 0)
        {
            if (c.chan.a == 0xFF)
            {
                chan = c.chan;
            }
            else if (c.chan.a)
            {
                static thread_local auto cache = std::array<std::tuple<argb, argb, argb>, 3>{};
                cache_index &= 3;
                if (*this == std::get<0>(cache[cache_index]) && c == std::get<1>(cache[cache_index]))
                {
                    *this = std::get<2>(cache[cache_index]);
                    return;
                }
                std::get<0>(cache[cache_index]) = *this;
                std::get<1>(cache[cache_index]) = c;

                auto dst_lin_r = netxs::sRGB2Linear(chan.r);
                auto dst_lin_g = netxs::sRGB2Linear(chan.g);
                auto dst_lin_b = netxs::sRGB2Linear(chan.b);
                auto src_lin_r = netxs::sRGB2Linear(c.chan.r);
                auto src_lin_g = netxs::sRGB2Linear(c.chan.g);
                auto src_lin_b = netxs::sRGB2Linear(c.chan.b);
                auto bg_luma = argb::luma(dst_lin_r, dst_lin_g, dst_lin_b);
                auto a_srgb = c.chan.a / 255.0f;
                auto a_low  = netxs::sRGB2Linear(c.chan.a); // Dampened alpha for dark bg (~0.21 for a=0.5).
                auto a_high = netxs::linear2sRGB(a_srgb);   // Boosted alpha for light bg (~0.73 for a=0.5).
                auto src_alpha = a_low + (a_high - a_low) * bg_luma;
                auto dst_alpha = 1.0f - src_alpha;
                auto blended_r = src_lin_r * src_alpha + dst_lin_r * dst_alpha;
                auto blended_g = src_lin_g * src_alpha + dst_lin_g * dst_alpha;
                auto blended_b = src_lin_b * src_alpha + dst_lin_b * dst_alpha;
                // Dynamic contrast
                auto dr_diff = src_lin_r - dst_lin_r;
                auto dg_diff = src_lin_g - dst_lin_g;
                auto db_diff = src_lin_b - dst_lin_b;
                auto color_dist = std::min(1.0f, std::sqrt(dr_diff * dr_diff + dg_diff * dg_diff + db_diff * db_diff));
                color_dist = std::lerp(1.0f, color_dist, src_alpha); // Lerp color_dist between 1.0 and color_dist by a_srgb.
                auto force = 0.3f * (1.0f - color_dist);
                if (bg_luma > 0.50f) // Light background -> Darken (moving to black 0.0).
                {
                    auto t = 1.0f - force;
                    blended_r *= t;
                    blended_g *= t;
                    blended_b *= t;
                }
                else // Dark background -> Lighten (moving to white 1.0).
                {
                    blended_r = std::lerp(blended_r, 1.0f, force);
                    blended_g = std::lerp(blended_g, 1.0f, force);
                    blended_b = std::lerp(blended_b, 1.0f, force);
                }
                // Final conversion
                chan.r = netxs::saturate_cast<byte>(0.5f + 255.0f * netxs::linear2sRGB(blended_r));
                chan.g = netxs::saturate_cast<byte>(0.5f + 255.0f * netxs::linear2sRGB(blended_g));
                chan.b = netxs::saturate_cast<byte>(0.5f + 255.0f * netxs::linear2sRGB(blended_b));
                //auto a_dst = chan.a / 255.0f;
                //auto out_a = a_srgb + a_dst * (1.0f - a_srgb);
                //chan.a = netxs::saturate_cast<byte>(out_a * 255.0f + 0.5f);
                std::get<2>(cache[cache_index]) = *this;
            }
        }
        // argb: Alpha blending ARGB colors.
        void inline mix(argb c)
        {
            if (c.chan.a == 0xFF)
            {
                chan = c.chan;
            }
            else if (c.chan.a)
            {
                //todo consider premultiplied alpha
                auto a1 = ui32{ chan.a };
                auto a2 = ui32{ c.chan.a };
                auto a = ((a2 + a1) << 8) - a1 * a2;
                auto blend2 = [&](auto c1, auto c2)
                {
                    auto t = c1 * a1;
                    auto d = ((c2 * a2 + t) << 8) - t * a2;
                    return (byte)(d / a);
                    //return (((c2 * a2 + t) << 8) - t * a2) / a;
                };
                chan.r = blend2(chan.r, c.chan.r);
                chan.g = blend2(chan.g, c.chan.g);
                chan.b = blend2(chan.b, c.chan.b);
                chan.a = (byte)(a >> 8);
            }
        }
        // argb: Alpha blending ARGB colors.
        void blend(argb c)
        {
            mix(c);
        }
        // argb: ARGB transitional blending. Level = 0: equals c1, level = 256: equals c2.
        static auto transit(argb c1, argb c2, si32 level)
        {
            auto inverse = 256 - level;
            return argb{ (c2.chan.r * level + c1.chan.r * inverse) >> 8,
                         (c2.chan.g * level + c1.chan.g * inverse) >> 8,
                         (c2.chan.b * level + c1.chan.b * inverse) >> 8,
                         (c2.chan.a * level + c1.chan.a * inverse) >> 8 };
        }
        static auto transit(argb c1, argb c2, fp32 level)
        {
            auto inverse = 1.f - level;
            return argb{ (byte)std::clamp(c2.chan.r * level + c1.chan.r * inverse, 0.f, 255.f),
                         (byte)std::clamp(c2.chan.g * level + c1.chan.g * inverse, 0.f, 255.f),
                         (byte)std::clamp(c2.chan.b * level + c1.chan.b * inverse, 0.f, 255.f),
                         (byte)std::clamp(c2.chan.a * level + c1.chan.a * inverse, 0.f, 255.f) };
        }
        // argb: Alpha blending ARGB colors.
        void inline mix(argb c, si32 alpha)
        {
            if (alpha == 255) chan = c.chan;
            else if (alpha)
            {
                auto na = 256 - alpha;
                chan.r = (byte)((c.chan.r * alpha + chan.r * na) >> 8);
                chan.g = (byte)((c.chan.g * alpha + chan.g * na) >> 8);
                chan.b = (byte)((c.chan.b * alpha + chan.b * na) >> 8);
                chan.a = (byte)((c.chan.a * alpha + chan.a * na) >> 8);
            }
        }
        // argb: Rough alpha blending ARGB colors.
        //void mix_alpha(argb c)
        //{
        //    auto blend = [](auto const& c1, auto const& c2, auto const& alpha)
        //    {
        //        return ((c1 << 8) + (c2 - c1) * alpha) >> 8;
        //    };
        //    auto norm = [](auto const& c2, auto const& alpha)
        //    {
        //        return (c2 * alpha) >> 8;
        //    };
        //
        //    if (chan.a)
        //    {
        //        if (c.chan.a)
        //        {
        //            auto& a1 = chan.a;
        //            auto& a2 = c.chan.a;
        //            chan.r = blend(norm(c.chan.r, a2), chan.r, a1);
        //            chan.g = blend(norm(c.chan.g, a2), chan.g, a1);
        //            chan.b = blend(norm(c.chan.b, a2), chan.b, a1);
        //            chan.a = c.chan.a;
        //        }
        //    }
        //    else
        //    {
        //        chan = c.chan;
        //    }
        //}
        //// argb: Are the colors identical.
        //bool like(argb c) const
        //{
        //    static constexpr auto k = ui32{ 0b11110000 };
        //    static constexpr auto threshold = ui32{ 0x00 + k << 16 + k << 8 + k };
        //    return (token & threshold) == (c.token & threshold);
        //}
        // argb: Shift color.
        void xlight(si32 factor = 1)
        {
            if (chan.a == 255)
            {
                if (luma() > 140)
                {
                    auto k = (byte)std::clamp(64 * factor, 0, 0xFF);
                    chan.r = chan.r < k ? 0x00 : chan.r - k;
                    chan.g = chan.g < k ? 0x00 : chan.g - k;
                    chan.b = chan.b < k ? 0x00 : chan.b - k;
                }
                else
                {
                    auto k = (byte)std::clamp(48 * factor, 0, 0xFF);
                    chan.r = chan.r > 0xFF - k ? 0xFF : chan.r + k;
                    chan.g = chan.g > 0xFF - k ? 0xFF : chan.g + k;
                    chan.b = chan.b > 0xFF - k ? 0xFF : chan.b + k;
                }
            }
            else if (chan.a < 2)
            {
                auto k = (byte)std::clamp(48 * factor, 0, 0xFF);
                chan.r = k;
                chan.g = k;
                chan.b = k;
                chan.a = (byte)std::min(255, 2 * k);
            }
            else
            {
                auto r = (si32)chan.r * chan.a / 256;
                auto g = (si32)chan.g * chan.a / 256;
                auto b = (si32)chan.b * chan.a / 256;
                if (luma(r, g, b) > 140)
                {
                    auto k = (byte)std::clamp(64 * factor, 0, 0xFF);
                    chan.r = chan.r < k ? 0x00 : chan.r - k;
                    chan.g = chan.g < k ? 0x00 : chan.g - k;
                    chan.b = chan.b < k ? 0x00 : chan.b - k;
                    chan.a = chan.a > 0xFF - k ? 0xFF : chan.a + k;
                }
                else
                {
                    auto k = (byte)std::clamp(48 * factor, 0, 0xFF);
                    chan.r = chan.r > 0xFF - k ? 0xFF : chan.r + k;
                    chan.g = chan.g > 0xFF - k ? 0xFF : chan.g + k;
                    chan.b = chan.b > 0xFF - k ? 0xFF : chan.b + k;
                    chan.a = chan.a > 0xFF - k ? 0xFF : chan.a + k;
                }
            }
        }
        // argb: Shift color pair.
        void xlight(si32 factor, argb& second)
        {
            if (chan.a == 255)
            {
                if (luma() > 140)
                {
                    auto k = (byte)std::clamp(64 * factor, 0, 0xFF);
                    chan.r = chan.r < k ? 0x00 : chan.r - k;
                    chan.g = chan.g < k ? 0x00 : chan.g - k;
                    chan.b = chan.b < k ? 0x00 : chan.b - k;
                    second.chan.r = second.chan.r < k ? 0x00 : second.chan.r - k;
                    second.chan.g = second.chan.g < k ? 0x00 : second.chan.g - k;
                    second.chan.b = second.chan.b < k ? 0x00 : second.chan.b - k;
                }
                else
                {
                    auto k = (byte)std::clamp(48 * factor, 0, 0xFF);
                    chan.r = chan.r > 0xFF - k ? 0xFF : chan.r + k;
                    chan.g = chan.g > 0xFF - k ? 0xFF : chan.g + k;
                    chan.b = chan.b > 0xFF - k ? 0xFF : chan.b + k;
                    second.chan.r = second.chan.r > 0xFF - k ? 0xFF : second.chan.r + k;
                    second.chan.g = second.chan.g > 0xFF - k ? 0xFF : second.chan.g + k;
                    second.chan.b = second.chan.b > 0xFF - k ? 0xFF : second.chan.b + k;
                }
            }
            else if (chan.a < 2)
            {
                auto k = (byte)std::clamp(48 * factor, 0, 0xFF);
                chan.r = k;
                chan.g = k;
                chan.b = k;
                chan.a = (byte)std::min(255, 2 * k);
                second.chan.r = second.chan.r > 0xFF - k ? 0xFF : second.chan.r + k;
                second.chan.g = second.chan.g > 0xFF - k ? 0xFF : second.chan.g + k;
                second.chan.b = second.chan.b > 0xFF - k ? 0xFF : second.chan.b + k;
            }
            else
            {
                auto r = (si32)chan.r * chan.a / 256;
                auto g = (si32)chan.g * chan.a / 256;
                auto b = (si32)chan.b * chan.a / 256;
                if (luma(r, g, b) > 140)
                {
                    auto k = (byte)std::clamp(64 * factor, 0, 0xFF);
                    chan.r = chan.r < k ? 0x00 : chan.r - k;
                    chan.g = chan.g < k ? 0x00 : chan.g - k;
                    chan.b = chan.b < k ? 0x00 : chan.b - k;
                    //chan.a = chan.a > 0xFF - k ? 0xFF : chan.a + k;
                    second.chan.r = second.chan.r < k ? 0x00 : second.chan.r - k;
                    second.chan.g = second.chan.g < k ? 0x00 : second.chan.g - k;
                    second.chan.b = second.chan.b < k ? 0x00 : second.chan.b - k;
                }
                else
                {
                    auto k = (byte)std::clamp(48 * factor, 0, 0xFF);
                    chan.r = chan.r > 0xFF - k ? 0xFF : chan.r + k;
                    chan.g = chan.g > 0xFF - k ? 0xFF : chan.g + k;
                    chan.b = chan.b > 0xFF - k ? 0xFF : chan.b + k;
                    // xlight for 0x80'000000 is invisible on purewhite os desktop
                    //chan.a = chan.a > 0xFF - k ? 0xFF : chan.a + k;
                    second.chan.r = second.chan.r > 0xFF - k ? 0xFF : second.chan.r + k;
                    second.chan.g = second.chan.g > 0xFF - k ? 0xFF : second.chan.g + k;
                    second.chan.b = second.chan.b > 0xFF - k ? 0xFF : second.chan.b + k;
                }
            }
        }
        // argb: Darken the color.
        auto shadow(byte k = 39)
        {
            chan.r = chan.r < k ? 0x00 : chan.r - k;
            chan.g = chan.g < k ? 0x00 : chan.g - k;
            chan.b = chan.b < k ? 0x00 : chan.b - k;
            return *this;
        }
        // argb: Faint color.
        auto faint()
        {
            chan.r >>= 1;
            chan.g >>= 1;
            chan.b >>= 1;
            return *this;
        }
        // argb: Lighten the color.
        void bright(si32 factor = 1)
        {
            auto k = (byte)std::clamp(39 * factor, 0, 0xFF);
            chan.r = chan.r > 0xFF - k ? 0xFF : chan.r + k;
            chan.g = chan.g > 0xFF - k ? 0xFF : chan.g + k;
            chan.b = chan.b > 0xFF - k ? 0xFF : chan.b + k;
        }
        // argb: Invert the color.
        void invert()
        {
            static constexpr auto pureblack = argb{ 0xFF000000 };
            static constexpr auto antiwhite = argb{ 0x00FFFFFF };

            token = (token & pureblack.token) | ~(token & antiwhite.token);
        }
        // argb: Print channel values.
        auto str() const
        {
            return "{" + std::to_string(chan.r) + ","
                       + std::to_string(chan.g) + ","
                       + std::to_string(chan.b) + ","
                       + std::to_string(chan.a) + "}";
        }

        static constexpr auto default_color = 0x00'FF'FF'FF;
        static constexpr auto active_transparent = 0x01'000000;
        static constexpr auto transparent = 0x00'000000;
        static constexpr auto indexed_color = 0x00'FF0100u;
        static constexpr auto indexed_mask = 0xFF'FFFF00u;

        template<si32 i>
        static constexpr ui32 _vt16 = // Compile-time value assigning (sorted by enum).
            i == tint::blackdk   ? 0xFF'10'10'10 :
            i == tint::reddk     ? 0xFF'C4'0F'1F :
            i == tint::greendk   ? 0xFF'12'A1'0E :
            i == tint::yellowdk  ? 0xFF'C0'9C'00 :
            i == tint::bluedk    ? 0xFF'00'37'DB :
            i == tint::magentadk ? 0xFF'87'17'98 :
            i == tint::cyandk    ? 0xFF'3B'96'DD :
            i == tint::whitedk   ? 0xFF'BB'BB'BB :
            i == tint::blacklt   ? 0xFF'75'75'75 :
            i == tint::redlt     ? 0xFF'E6'48'56 :
            i == tint::greenlt   ? 0xFF'15'C6'0C :
            i == tint::yellowlt  ? 0xFF'F8'F1'A5 :
            i == tint::bluelt    ? 0xFF'3A'78'FF :
            i == tint::magentalt ? 0xFF'B3'00'9E :
            i == tint::cyanlt    ? 0xFF'60'D6'D6 :
            i == tint::whitelt   ? 0xFF'F3'F3'F3 : 0;
        template<si32 i>
        static constexpr ui32 _vtm16 = // Compile-time value assigning (sorted by enum).
            i == tint16::blackdk   ? 0xFF'00'00'00    :
            i == tint16::blacklt   ? 0xFF'20'20'20    :
            i == tint16::graydk    ? 0xFF'50'50'50    :
            i == tint16::graylt    ? 0xFF'80'80'80    :
            i == tint16::whitedk   ? 0xFF'D0'D0'D0    :
            i == tint16::whitelt   ? 0xFF'FF'FF'FF    :
            i == tint16::reddk     ? _vt16<reddk>     :
            i == tint16::bluedk    ? _vt16<bluedk>    :
            i == tint16::greendk   ? _vt16<greendk>   :
            i == tint16::yellowdk  ? _vt16<yellowdk>  :
            i == tint16::magentalt ? _vt16<magentalt> :
            i == tint16::cyanlt    ? _vt16<cyanlt>    :
            i == tint16::redlt     ? _vt16<redlt>     :
            i == tint16::bluelt    ? _vt16<bluelt>    :
            i == tint16::greenlt   ? _vt16<greenlt>   :
            i == tint16::yellowlt  ? _vt16<yellowlt>  : 0;

        static constexpr auto vtm16 = std::to_array(
        {
            _vtm16<0>, _vtm16<1>, _vtm16<2>,  _vtm16<3>,  _vtm16<4>,  _vtm16<5>,  _vtm16<6>,  _vtm16<7>,
            _vtm16<8>, _vtm16<9>, _vtm16<10>, _vtm16<11>, _vtm16<12>, _vtm16<13>, _vtm16<14>, _vtm16<15>,
        });
        static constexpr auto vga16 = std::to_array(
        {
            _vt16<blackdk>, _vt16<bluedk>, _vt16<greendk>, _vt16<cyandk>, _vt16<reddk>, _vt16<magentadk>, _vt16<yellowdk>, _vt16<whitedk>,
            _vt16<blacklt>, _vt16<bluelt>, _vt16<greenlt>, _vt16<cyanlt>, _vt16<redlt>, _vt16<magentalt>, _vt16<yellowlt>, _vt16<whitelt>,
        });
        static constexpr auto vt256 = std::to_array(
        {
            _vt16<0>, _vt16<1>, _vt16<2>,  _vt16<3>,  _vt16<4>,  _vt16<5>,  _vt16<6>,  _vt16<7>,
            _vt16<8>, _vt16<9>, _vt16<10>, _vt16<11>, _vt16<12>, _vt16<13>, _vt16<14>, _vt16<15>,

            // 6×6×6 RGB-cube (216 colors), index = 16 + 36r + 6g + b, r,g,b=[0, 5]
            0xFF000000, 0xFF00005F, 0xFF000087, 0xFF0000AF, 0xFF0000D7, 0xFF0000FF,
            0xFF005F00, 0xFF005F5F, 0xFF005F87, 0xFF005FAF, 0xFF005FD7, 0xFF005FFF,
            0xFF008700, 0xFF00875F, 0xFF008787, 0xFF0087AF, 0xFF0087D7, 0xFF0087FF,
            0xFF00AF00, 0xFF00AF5F, 0xFF00AF87, 0xFF00AFAF, 0xFF00AFD7, 0xFF00AFFF,
            0xFF00D700, 0xFF00D75F, 0xFF00D787, 0xFF00D7AF, 0xFF00D7D7, 0xFF00D7FF,
            0xFF00FF00, 0xFF00FF5F, 0xFF00FF87, 0xFF00FFAF, 0xFF00FFD7, 0xFF00FFFF,

            0xFF5F0000, 0xFF5F005F, 0xFF5F0087, 0xFF5F00AF, 0xFF5F00D7, 0xFF5F00FF,
            0xFF5F5F00, 0xFF5F5F5F, 0xFF5F5F87, 0xFF5F5FAF, 0xFF5F5FD7, 0xFF5F5FFF,
            0xFF5F8700, 0xFF5F875F, 0xFF5F8787, 0xFF5F87AF, 0xFF5F87D7, 0xFF5F87FF,
            0xFF5FAF00, 0xFF5FAF5F, 0xFF5FAF87, 0xFF5FAFAF, 0xFF5FAFD7, 0xFF5FAFFF,
            0xFF5FD700, 0xFF5FD75F, 0xFF5FD787, 0xFF5FD7AF, 0xFF5FD7D7, 0xFF5FD7FF,
            0xFF5FFF00, 0xFF5FFF5F, 0xFF5FFF87, 0xFF5FFFAF, 0xFF5FFFD7, 0xFF5FFFFF,

            0xFF870000, 0xFF87005F, 0xFF870087, 0xFF8700AF, 0xFF8700D7, 0xFF8700FF,
            0xFF875F00, 0xFF875F5F, 0xFF875F87, 0xFF875FAF, 0xFF875FD7, 0xFF875FFF,
            0xFF878700, 0xFF87875F, 0xFF878787, 0xFF8787AF, 0xFF8787D7, 0xFF8787FF,
            0xFF87AF00, 0xFF87AF5F, 0xFF87AF87, 0xFF87AFAF, 0xFF87AFD7, 0xFF87AFFF,
            0xFF87D700, 0xFF87D75F, 0xFF87D787, 0xFF87D7AF, 0xFF87D7D7, 0xFF87D7FF,
            0xFF87FF00, 0xFF87FF5F, 0xFF87FF87, 0xFF87FFAF, 0xFF87FFD7, 0xFF87FFFF,

            0xFFAF0000, 0xFFAF005F, 0xFFAF0087, 0xFFAF00AF, 0xFFAF00D7, 0xFFAF00FF,
            0xFFAF5F00, 0xFFAF5F5F, 0xFFAF5F87, 0xFFAF5FAF, 0xFFAF5FD7, 0xFFAF5FFF,
            0xFFAF8700, 0xFFAF875F, 0xFFAF8787, 0xFFAF87AF, 0xFFAF87D7, 0xFFAF87FF,
            0xFFAFAF00, 0xFFAFAF5F, 0xFFAFAF87, 0xFFAFAFAF, 0xFFAFAFD7, 0xFFAFAFFF,
            0xFFAFD700, 0xFFAFD75F, 0xFFAFD787, 0xFFAFD7AF, 0xFFAFD7D7, 0xFFAFD7FF,
            0xFFAFFF00, 0xFFAFFF5F, 0xFFAFFF87, 0xFFAFFFAF, 0xFFAFFFD7, 0xFFAFFFFF,

            0xFFD70000, 0xFFD7005F, 0xFFD70087, 0xFFD700AF, 0xFFD700D7, 0xFFD700FF,
            0xFFD75F00, 0xFFD75F5F, 0xFFD75F87, 0xFFD75FAF, 0xFFD75FD7, 0xFFD75FFF,
            0xFFD78700, 0xFFD7875F, 0xFFD78787, 0xFFD787AF, 0xFFD787D7, 0xFFD787FF,
            0xFFD7AF00, 0xFFD7AF5F, 0xFFD7AF87, 0xFFD7AFAF, 0xFFD7AFD7, 0xFFD7AFFF,
            0xFFD7D700, 0xFFD7D75F, 0xFFD7D787, 0xFFD7D7AF, 0xFFD7D7D7, 0xFFD7D7FF,
            0xFFD7FF00, 0xFFD7FF5F, 0xFFD7FF87, 0xFFD7FFAF, 0xFFD7FFD7, 0xFFD7FFFF,

            0xFFFF0000, 0xFFFF005F, 0xFFFF0087, 0xFFFF00AF, 0xFFFF00D7, 0xFFFF00FE,
            0xFFFF5F00, 0xFFFF5F5F, 0xFFFF5F87, 0xFFFF5FAF, 0xFFFF5FD7, 0xFFFF5FFE,
            0xFFFF8700, 0xFFFF875F, 0xFFFF8787, 0xFFFF87AF, 0xFFFF87D7, 0xFFFF87FE,
            0xFFFFAF00, 0xFFFFAF5F, 0xFFFFAF87, 0xFFFFAFAF, 0xFFFFAFD7, 0xFFFFAFFE,
            0xFFFFD700, 0xFFFFD75F, 0xFFFFD787, 0xFFFFD7AF, 0xFFFFD7D7, 0xFFFFD7FE,
            0xFFFFFF00, 0xFFFFFF5F, 0xFFFFFF87, 0xFFFFFFAF, 0xFFFFFFD7, 0xFFFFFFFF,
            // Grayscale colors, 24 steps
            0xFF080808, 0xFF121212, 0xFF1C1C1C, 0xFF262626, 0xFF303030, 0xFF3A3A3A,
            0xFF444444, 0xFF4E4E4E, 0xFF585858, 0xFF626262, 0xFF6C6C6C, 0xFF767676,
            0xFF808080, 0xFF8A8A8A, 0xFF949494, 0xFF9E9E9E, 0xFFA8A8A8, 0xFFB2B2B2,
            0xFFBCBCBC, 0xFFC6C6C6, 0xFFD0D0D0, 0xFFDADADA, 0xFFE4E4E4, 0xFFEEEEEE,
        });
        friend auto& operator << (std::ostream& s, argb c)
        {
            if (auto index = argb::is_indexed_color(c))
            {
                return s << "Color" << index - 1;
            }
            else
            {
                return s << "{" << (si32)c.chan.r
                         << "," << (si32)c.chan.g
                         << "," << (si32)c.chan.b
                         << "," << (si32)c.chan.a
                         << "}";
            }
        }
        static auto set_vtm16_palette(auto proc)
        {
            for (auto i = 0; i < 16; i++)
            {
                proc(i, argb::vtm16[i]);
            }
        }
        auto lookup(auto& fast, auto&& palette) const
        {
            auto head = fast.begin();
            auto tail = fast.end();
            auto init = head;
            while (head != tail) // Look in static table.
            {
                auto& next = *head;
                if (next.first == token)
                {
                    if (head == init) return next.second;
                    else
                    {
                        auto& prev = *(--head);
                        std::swap(next, prev); // Sort by hits. !! Not thread-safe.
                        return prev.second;
                    }
                }
                ++head;
            }
            auto dist = [](si32 c1, si32 c2)
            {
                auto dr = ((c1 & 0x0000FF) >> 0)  - ((c2 & 0x0000FF) >> 0);
                auto dg = ((c1 & 0x00FF00) >> 8)  - ((c2 & 0x00FF00) >> 8);
                auto db = ((c1 & 0xFF0000) >> 16) - ((c2 & 0xFF0000) >> 16);
                return (ui32)(dr * dr + dg * dg + db * db); // std::abs(dr) + std::abs(dg) + std::abs(db);
            };
            auto max = 1368u; // Minimal distance: between greenlt and greendk - 1.
            auto hit = std::pair{ max, 0 };
            for (auto i = 0; i < (si32)std::size(palette); i++) // Find the nearest.
            {
                if (auto d = dist(palette[i], token))
                {
                    if (d < hit.first) hit = { d, i };
                }
                else return i;
            }
            if (hit.first == max) // Fallback to grayscale.
            {
                auto l = luma();
                     if (l < 42)  hit.second = tint16::blacklt;
                else if (l < 90)  hit.second = tint16::graydk;
                else if (l < 170) hit.second = tint16::graylt;
                else if (l < 240) hit.second = tint16::whitedk;
                else              hit.second = tint16::whitelt;
            }
            return hit.second;
        }
        auto to_vga16(bool fg = true) const // argb: 4-bit Foreground color (vtm 16-color mode).
        {
            static auto cache_fg = []
            {
                auto table = std::vector<std::pair<ui32, si32>>{};
                for (auto i = 0; i < 16; i++) table.push_back({ argb::vt256[i], i });
                table.insert(table.end(),
                {
                    { 0xFF'ff'ff'ff, tint::whitelt   },
                    { 0xff'aa'aa'aa, tint::whitedk   },
                    { 0xff'80'80'80, tint::whitedk   },
                    { 0xff'55'55'55, tint::blacklt   },
                    { 0xFF'00'00'00, tint::blackdk   },

                    { 0xFF'55'00'00, tint::reddk     },
                    { 0xFF'80'00'00, tint::reddk     },
                    { 0xFF'aa'00'00, tint::reddk     },
                    { 0xFF'ff'00'00, tint::redlt     },

                    { 0xFF'00'00'55, tint::bluedk    },
                    { 0xFF'00'00'80, tint::bluedk    },
                    { 0xFF'00'00'aa, tint::bluedk    },
                    { 0xFF'00'00'ff, tint::bluelt    },

                    { 0xFF'00'aa'00, tint::greendk   },
                    { 0xFF'00'80'00, tint::greendk   },
                    { 0xFF'00'ff'00, tint::greenlt   },
                    { 0xFF'55'ff'55, tint::greenlt   },

                    { 0xFF'80'00'80, tint::magentadk },
                    { 0xFF'aa'00'aa, tint::magentadk },
                    { 0xFF'ff'55'ff, tint::magentalt },
                    { 0xFF'ff'00'ff, tint::magentalt },

                    { 0xFF'00'80'80, tint::cyandk    },
                    { 0xFF'00'aa'aa, tint::cyandk    },
                    { 0xFF'55'ff'ff, tint::cyanlt    },
                    { 0xFF'00'ff'ff, tint::cyanlt    },

                    { 0xFF'aa'55'00, tint::yellowdk  },
                    { 0xFF'80'80'00, tint::yellowdk  },
                    { 0xFF'ff'ff'00, tint::yellowlt  },
                    { 0xFF'ff'ff'55, tint::yellowlt  },
                });
                return table;
            }();
            static auto cache_bg = cache_fg;
            auto& cache = fg ? cache_fg : cache_bg; // Fg and Bg are sorted differently.
            auto c = lookup(cache, std::span{ argb::vt256.data(), 16 });
            return netxs::swap_bits<0, 2>(c); // ANSI<->DOS color scheme reindex.
        }
        auto to_vtm16(bool fg = true) const // argb: 4-bit Foreground color (vtm 16-color palette).
        {
            static auto cache_fg = []
            {
                auto table = std::vector<std::pair<ui32, si32>>{};
                for (auto i = 0u; i < std::size(argb::vtm16); i++) table.push_back({ argb::vtm16[i], i });
                table.insert(table.end(),
                {
                    { 0xFF'55'00'00,                 tint16::reddk     },
                    { 0xFF'aa'00'00,                 tint16::reddk     },
                    { 0xFF'80'00'00,                 tint16::reddk     },
                    { 0xFF'ff'00'00,                 tint16::redlt     },

                    { argb::vt256[tint::magentadk],  tint16::reddk     },
                    { 0xFF'80'00'80,                 tint16::reddk     },
                    { 0xFF'ff'55'ff,                 tint16::magentalt },
                    { 0xFF'ff'00'ff,                 tint16::magentalt },

                    { argb::vt256[tint::cyandk],     tint16::bluelt    },
                    { 0xFF'00'80'80,                 tint16::bluelt    },
                    { 0xFF'00'aa'aa,                 tint16::bluelt    },
                    { 0xFF'55'ff'ff,                 tint16::cyanlt    },
                    { 0xFF'00'ff'ff,                 tint16::cyanlt    },

                    { 0xFF'ff'ff'ff,                 tint16::whitelt   },
                    { 0xff'aa'aa'aa,                 tint16::whitedk   },
                    { 0xff'80'80'80,                 tint16::graylt    },
                    { 0xff'55'55'55,                 tint16::graydk    },
                    { 0xFF'00'00'00,                 tint16::blackdk   },

                    { 0xFF'00'00'55,                 tint16::bluedk    },
                    { 0xFF'00'00'80,                 tint16::bluedk    },
                    { 0xFF'00'00'aa,                 tint16::bluedk    },
                    { 0xFF'00'00'ff,                 tint16::bluelt    },

                    { 0xFF'00'aa'00,                 tint16::greendk   },
                    { 0xFF'00'80'00,                 tint16::greendk   },
                    { 0xFF'00'ff'00,                 tint16::greenlt   },
                    { 0xFF'55'ff'55,                 tint16::greenlt   },

                    { 0xFF'aa'55'00,                 tint16::yellowdk  },
                    { 0xFF'80'80'00,                 tint16::yellowdk  },
                    { 0xFF'ff'ff'00,                 tint16::yellowlt  },
                    { 0xFF'ff'ff'55,                 tint16::yellowlt  },
                });
                return table;
            }();
            static auto cache_bg = cache_fg;
            auto& cache = fg ? cache_fg : cache_bg; // Fg and Bg are sorted differently.
            return lookup(cache, argb::vtm16);
        }
        auto to_vtm8() const // argb: 3-bit Background color (vtm 8-color palette).
        {
            static auto cache = []
            {
                auto table = std::vector<std::pair<ui32, si32>>{};
                for (auto i = 0u; i < std::size(argb::vtm16) / 2; i++) table.push_back({ argb::vtm16[i], i });
                table.insert(table.end(),
                {
                    { argb::vt256[tint::bluelt   ],  tint16::bluedk  },
                    { argb::vt256[tint::redlt    ],  tint16::reddk   },
                    { argb::vt256[tint::cyanlt   ],  tint16::whitedk },
                    { argb::vt256[tint::cyandk   ],  tint16::graylt  },
                    { argb::vt256[tint::greenlt  ],  tint16::graylt  },
                    { argb::vt256[tint::greendk  ],  tint16::graydk  },
                    { argb::vt256[tint::yellowdk ],  tint16::graydk  },
                    { argb::vt256[tint::yellowlt ],  tint16::whitelt },
                    { argb::vt256[tint::magentalt],  tint16::reddk   },
                    { argb::vt256[tint::magentadk],  tint16::reddk   },
                    { 0xff'00'00'00,                 tint16::blackdk },
                    { 0xff'FF'00'00,                 tint16::reddk   },
                    { 0xff'00'00'FF,                 tint16::bluedk  },
                    { 0xff'FF'FF'FF,                 tint16::whitelt },
                    { 0xff'aa'aa'aa,                 tint16::whitedk },
                    { 0xff'80'80'80,                 tint16::graylt  },
                    { 0xff'55'55'55,                 tint16::graydk  },
                });
                return table;
            }();
            return lookup(cache, std::span{ argb::vtm16.data(), 8 });
        }
        static auto from_HLS(si32 h, si32 l, si32 s)
        {
            auto HueToRGB = [](fp32 v1, fp32 v2, fp32 vH)
            {
                if (vH < 0) vH += 1;
                if (vH > 1) vH -= 1;
                if ((6 * vH) < 1) return (v1 + (v2 - v1) * 6 * vH);
                if ((2 * vH) < 1) return v2;
                if ((3 * vH) < 2) return (v1 + (v2 - v1) * ((2 / 3.f) - vH) * 6);
                return v1;
            };
            auto H = h / 360.f;
            auto L = l / 100.f;
            auto S = s / 100.f;
            if (S == 0)
            {
                return argb{ L, L, L };
            }
            auto v2 = (L < 0.5f) ? (L * (1 + S)) : (L + S - L * S);
            auto v1 = 2 * L - v2;
            return argb{ HueToRGB(v1, v2, H + (1 / 3.f)),
                         HueToRGB(v1, v2, H),
                         HueToRGB(v1, v2, H - (1 / 3.f)) };
        }
        // argb: Change endianness to LE.
        friend auto letoh(argb r)
        {
            return argb{ netxs::letoh(r.token) };
        }
    };

    using pals = std::remove_const_t<decltype(argb::vt256)>;

    // canvas: Generic RGBA.
    template<class T>
    struct irgb
    {
        static constexpr auto inv_255 = 1.0f / 255.0f;

        T r, g, b, a;

        constexpr irgb() = default;
        constexpr irgb(irgb const&) = default;
        constexpr irgb(T r, T g, T b, T a)
            : r{ r }, g{ g }, b{ b }, a{ a }
        { }
        constexpr irgb(argb c) requires(std::is_integral_v<T>)
            : r{ c.chan.r },
              g{ c.chan.g },
              b{ c.chan.b },
              a{ c.chan.a }
        { }
        constexpr irgb(argb c) requires(std::is_floating_point_v<T>)
            : r{ c.chan.r * inv_255 },
              g{ c.chan.g * inv_255 },
              b{ c.chan.b * inv_255 },
              a{ c.chan.a * inv_255 }
        { }

        operator argb() const { return argb{ r, g, b, a }; }

        bool operator > (auto n) const { return r > n || g > n || b > n || a > n; }
        auto operator / (auto n) const { return irgb{ r / n, g / n, b / n, a / n }; } // 10% faster than divround.
        auto operator * (auto n) const { return irgb{ r * n, g * n, b * n, a * n }; }
        auto operator + (irgb const& c) const { return irgb{ r + c.r, g + c.g, b + c.b, a + c.a }; }
        void operator *= (auto n) { r *= n; g *= n; b *= n; a *= n; }
        void operator /= (auto n) { r /= n; g /= n; b /= n; a /= n; }
        void operator =  (irgb const& c) { r =  c.r; g =  c.g; b =  c.b; a =  c.a; }
        void operator += (irgb const& c) { r += c.r; g += c.g; b += c.b; a += c.a; }
        void operator -= (irgb const& c) { r -= c.r; g -= c.g; b -= c.b; a -= c.a; }
        void operator += (argb c) requires(std::is_integral_v<T>) { r += c.chan.r; g += c.chan.g; b += c.chan.b; a += c.chan.a; }
        void operator -= (argb c) requires(std::is_integral_v<T>) { r -= c.chan.r; g -= c.chan.g; b -= c.chan.b; a -= c.chan.a; }

        // irgb: Premultiply alpha (floating point only).
        auto& pma() requires(std::is_floating_point_v<T>)
        {
            r *= a;
            g *= a;
            b *= a;
            return *this;
        }
        // irgb: Blend with pma c (floating point only).
        auto& blend_pma(irgb c) requires(std::is_floating_point_v<T>)
        {
            auto na = 1.f - c.a;
            r = c.r + na * r;
            g = c.g + na * g;
            b = c.b + na * b;
            a = c.a + na * a;
            return *this;
        }
        // irgb: Blend with pma c (floating point only).
        auto& blend_pma(irgb c, byte alpha) requires(std::is_floating_point_v<T>)
        {
            auto factor = (T)alpha * inv_255;
            return blend_pma(c * factor);
        }
        // irgb: Blend with non-pma c (0.0-1.0).
        auto& blend_nonpma(irgb non_pma_c) requires(std::is_floating_point_v<T>)
        {
            auto factor = non_pma_c.a;
            auto inv_factor = 1.0f - factor;
            r = non_pma_c.r * factor + r * inv_factor;
            g = non_pma_c.g * factor + g * inv_factor;
            b = non_pma_c.b * factor + b * inv_factor;
            a = factor               + a * inv_factor;
            return *this;
        }
        // irgb: Blend with non-pma c (0.0-1.0) using integer alpha (0-255).
        auto& blend_nonpma(irgb non_pma_c, byte alpha) requires(std::is_floating_point_v<T>)
        {
            auto factor = ((T)alpha * inv_255) * non_pma_c.a;
            auto inv_factor = 1.0f - factor;
            r = non_pma_c.r * factor + r * inv_factor;
            g = non_pma_c.g * factor + g * inv_factor;
            b = non_pma_c.b * factor + b * inv_factor;
            a = factor + a * inv_factor;
            return *this;
        }
        // irgb: Pack extra alpha into exponent/mantissa.
        void pack_alpha(byte extra_alpha)
        {
            auto a_8bit = (ui32)(a * 255.f + 0.5f);
            auto packed = 0x40000000u | (extra_alpha << 15) | (a_8bit << 7); // extra_alpha: bits 15-22, a_8bit: bits 7-14.
            a = std::bit_cast<fp32>(packed);
        }
        // irgb: Return true if alpha channel has extra alpha value.
        bool has_extra_alpha() const
        {
            auto a_bits = std::bit_cast<ui32>(a);
            return (a_bits >> 30) == 1; // Number in the range [2.0, 4.0).
        }
        // irgb: Unpack and restore pure alpha. Return extra alpha value and normalize the current alpha channel.
        byte unpack_alpha()
        {
            auto extra = get_extra_alpha();
            restore_pure_alpha();
            return extra;
        }
        // irgb: Normalize the current alpha channel.
        void restore_pure_alpha()
        {
            auto a_bits = std::bit_cast<ui32>(a);
            auto a_8bit = (a_bits >> 7) & 0xFF;
            a = a_8bit * inv_255;
        }
        // irgb: Unpack and return an extra alpha value.
        byte get_extra_alpha() const
        {
            auto a_bits = std::bit_cast<ui32>(a);
            return (byte)((a_bits >> 15) & 0xFF);
        }
        // irgb: PMA sRGB (8-bit) -> PMA Linear (irgb).
        static auto pma_srgb_to_pma_linear(argb pma_pixel) requires(std::is_floating_point_v<T>)
        {
            auto a_b = pma_pixel.chan.a;
            if (a_b == 0) return irgb{};
            if (a_b == 255)
            {
                return irgb{ netxs::sRGB2Linear(pma_pixel.chan.r),
                             netxs::sRGB2Linear(pma_pixel.chan.g),
                             netxs::sRGB2Linear(pma_pixel.chan.b),
                             (T)1 };
            }
            auto lin_a = (T)a_b * inv_255;
            auto inv_a = (T)1 / lin_a;
            auto to_lin = [&](byte channel_pma)
            {
                auto straight_srgb = channel_pma * inv_255 * inv_a;              // Unpremultiply (in sRGB).
                auto straight_lin = netxs::sRGB2Linear(straight_srgb); // Linearize.
                return straight_lin * lin_a;                           // Premultiply (in Linear).
            };
            return irgb{ to_lin(pma_pixel.chan.r),
                         to_lin(pma_pixel.chan.g),
                         to_lin(pma_pixel.chan.b),
                         lin_a };
        }
        // irgb: PMA Linear (irgb) -> PMA sRGB (8-bit).
        static auto pma_linear_to_pma_srgb(irgb pma_pixel) requires(std::is_floating_point_v<T>)
        {
            auto a_b = pma_pixel.a;
            if (a_b == 0) return argb{};
            if (a_b == (T)1)
            {
                return argb{ (byte)(netxs::linear2sRGB(pma_pixel.r) * 255.f + 0.5f),
                             (byte)(netxs::linear2sRGB(pma_pixel.g) * 255.f + 0.5f),
                             (byte)(netxs::linear2sRGB(pma_pixel.b) * 255.f + 0.5f),
                             255 };
            }
            auto lin_a = a_b;
            auto inv_a = (T)1 / lin_a;
            auto to_srgb = [&](T channel_pma)
            {
                auto straight_lin = channel_pma * inv_a;                                      // Unpremultiply (in Linear).
                auto straight_srgb = (byte)(netxs::linear2sRGB(straight_lin) * 255.f + 0.5f); // Linearize.
                return straight_srgb * lin_a;                                                 // Premultiply (in sRGB).
            };
            return argb{ to_srgb(pma_pixel.r),
                         to_srgb(pma_pixel.g),
                         to_srgb(pma_pixel.b),
                         (byte)(lin_a * 255.f + 0.5f) };
        }
        // irgb: PMA Linear (irgb) -> non-PMA sRGB (8-bit).
        static auto pma_linear_to_nonpma_srgb(irgb pma_lin) requires(std::is_floating_point_v<T>)
        {
            if (pma_lin.a <= 0.000001f) return argb{};
            auto inv_a = (T)1 / pma_lin.a;
            auto to_srgb_byte = [&](fp32 lin_channel)
            {
                return (byte)(netxs::linear2sRGB(lin_channel * inv_a) * 255.f + 0.5f);
            };
            return argb{ to_srgb_byte(pma_lin.r),
                         to_srgb_byte(pma_lin.g),
                         to_srgb_byte(pma_lin.b),
                         (byte)(pma_lin.a * 255.f + 0.5f) };
        }
        // irgb: non-PMA sRGB (8-bit) -> PMA Linear (irgb).
        static auto nonpma_srgb_to_pma_linear(argb nonpma_pixel) requires(std::is_floating_point_v<T>)
        {
            auto a = (T)nonpma_pixel.chan.a * inv_255;
            return irgb{ netxs::sRGB2Linear(nonpma_pixel.chan.r) * a,
                         netxs::sRGB2Linear(nonpma_pixel.chan.g) * a,
                         netxs::sRGB2Linear(nonpma_pixel.chan.b) * a,
                         a };
        }
    };

    namespace imagens
    {
        // local:  stored in cells
        // global: stored in image
        #define global_attr_list \
            X(u  ) /* u-coor (0.0-1.0)                              */ \
            X(v  ) /* v-coor (0.0-1.0)                              */ \
            X(uw ) /* u-size (reset raster if changed)              */ \
            X(vh ) /* v-size (reset raster if changed)              */ \
            X(x  ) /* x                                             */ \
            X(y  ) /* y                                             */ \
            X(w  ) /* w      (reset raster if changed)              */ \
            X(h  ) /* h      (reset raster if changed)              */ \
            X(fit) /* fit    (reset raster if changed)              */ \
            X(a  ) /* align                                         */ \
            X(tr ) /* transform (reset raster if SwapXY is changed) */ \
            X(f  ) /* flip   (not stored)                           */ \
            X(rt ) /* rotate (not stored)                           */
        #define local_attr_list \
            X(W ) /* Cell canvas width  */ \
            X(H ) /* Cell canvas height */ \
            X(c ) /* Specified column   */ \
            X(r ) /* Specified row      */ \
            X(o ) /* ontop              */
        namespace gb
        {
            static constexpr auto _counter = __COUNTER__ + 1;
            #define X(_attr) static constexpr auto _attr = __COUNTER__ - _counter; // width = __COUNTER__ - _counter;
                global_attr_list
                static constexpr auto attr_count = __COUNTER__ - _counter;
            #undef X
            static constexpr auto names = std::array<view, attr_count>
            {
                #define X(_attr) #_attr, // "width",
                    global_attr_list
                #undef X
            };
            static const auto attr_index_map = utf::unordered_map<text, si32>
            {
                #define X(_attr) { #_attr, _attr },
                    global_attr_list
                #undef X
            };
        }
        namespace lc
        {
            static constexpr auto _counter = __COUNTER__ + 1;
            #define X(_attr) static constexpr auto _attr = __COUNTER__ - _counter; // row = __COUNTER__ - _counter;
                local_attr_list
                static constexpr auto attr_count = __COUNTER__ - _counter;
            #undef X
            static constexpr auto names = std::array<view, attr_count>
            {
                #define X(_attr) #_attr, // "width",
                    local_attr_list
                #undef X
            };
            static const auto attr_index_map = utf::unordered_map<text, si32>
            {
                #define X(_attr) { #_attr, _attr },
                    local_attr_list
                #undef X
            };
        }
        #undef global_attr_list
        #undef local_attr_list
        // Dihedral group D4 (the symmetries of a square) 0b[FlipY][FlipX][SwapXY].
        //    transfroms: 0   1   2   3   4   5   6   7  =xform
        //    pull-based: lt  tl  rt  tr  lb  bl  rb  br (left/right/top/bottom-left/right/top/bottom)
        //    push-based: lt  tl  rt  bl  lb  tr  rb  br (left/right/top/bottom-left/right/top/bottom)
        //                            |       |
        enum ds         { lt, tl, rt, tr, lb, bl, rb, br }; // Pull-based sequence.
        // Reconcile the coordinate systems between the "pull-based" renderer (x_form_mirror_r90) and the "push-based" raster transformer (sprite::transform). In the D4 dihedral group, when SwapXY (bit 0) is active, the order of FlipX/Y applications is non-commutative.
        // This conjugation table swaps 'Swap + FlipX' (3) with 'Swap + FlipY' (5) to synchronize the two algorithms.
        static constexpr auto xlate = std::to_array({ lt, tl, rt, bl, lb, tr, rb, br });
        enum class flips { none, hz, vt, hv };
        enum class ccw { none, r90, r180, r270 };
                                                           //direction: lt, tl, rt, tr, lb, bl, rb, br
        static constexpr auto do_flip = std::to_array({ std::to_array({ lt, tl, rt, tr, lb, bl, rb, br }),    // none
                                                        std::to_array({ rt, tr, lt, tl, rb, br, lb, bl }),    // hz_flip
                                                        std::to_array({ lb, bl, rb, br, lt, tl, rt, tr }),    // vt_flip
                                                        std::to_array({ rb, br, lb, bl, rt, tr, lt, tl }) }); // hv_flip (hz+vt)
        static constexpr auto do_rCCW = std::to_array({ std::to_array({ lt, tl, rt, tr, lb, bl, rb, br }),    // none
                                                        std::to_array({ bl, lb, tl, lt, br, rb, tr, rt }),    // CCW90
                                                        std::to_array({ rb, br, lb, bl, rt, tr, lt, tl }),    // 180
                                                        std::to_array({ tr, rt, br, rb, tl, lt, bl, lb }) }); // CCW270
        auto mirror_fx = [](auto& state, auto fx)
        {
            auto t = (si32)state & 7;
            auto f = (si32)fx & 3;
            t = do_flip[f][t];
            state = (std::decay_t<decltype(state)>)(t);
        };
        auto rotate_fx = [](auto& state, auto rx)
        {
            auto t = (si32)state & 7;
            auto r = (si32)rx & 3;
            t = do_rCCW[r][t];
            state = (std::decay_t<decltype(state)>)(t);
        };
        auto combine_transform_fx = [](auto t1, auto t2)
        {
            auto s1 = (si32)t1;
            auto s2 = (si32)t2;
            bool f1 = s1 & 4; // Extract reflection bits (4) and rotation bits (3).
            bool f2 = s2 & 4;
            auto r1 = s1 & 3;
            auto r2 = s2 & 3;
            // The new reflection bit is a simple XOR.
            auto f = (f1 ^ f2) ? 4 : 0;
            // The new rotation bit:
            // If the first state was reflected, the second rotation is subtracted.
            // Otherwise, it is added.
            auto r = (f1 ? (r1 - r2) : (r1 + r2)) & 3;
            return (decltype(t1))(f | r);
        };
        auto combine_align_fx = [](auto f_old_align, auto f_new_align)
        {
            auto old_align = (si32)f_old_align;
            auto new_align = (si32)f_new_align;
            auto res_h = (old_align & 0b0011) ? (old_align & 0b0011) : (new_align & 0b0011);
            auto res_v = (old_align & 0b1100) ? (old_align & 0b1100) : (new_align & 0b1100);
            return (decltype(f_old_align))(res_h | res_v);
        };

        static const auto gb_value_index_map = std::unordered_map<si32, utf::unordered_map<text, si32>>
        {
            { imagens::gb::fit, {{ "inside" , scale_mode::inside  },
                                 { "outside", scale_mode::outside },
                                 { "stretch", scale_mode::stretch },
                                 { "none"   , scale_mode::none    }} },
            { imagens::gb::rt, {{ "0", 0 }, { "90" , (si32)ccw::r90 }, { "180" , (si32)ccw::r180 }, { "270" , (si32)ccw::r270 }} },
            { imagens::gb::a, {{ "c", align_mode::center }, { "cm", align_mode::center_middle }, { "ct", align_mode::center_top }, { "cb", align_mode::center_bottom },
                               { "l", align_mode::left   }, { "lm", align_mode::left_middle   }, { "lt", align_mode::left_top   }, { "lb", align_mode::left_bottom   },
                               { "r", align_mode::right  }, { "rm", align_mode::right_middle  }, { "rt", align_mode::right_top  }, { "rb", align_mode::right_bottom  },
                               { "m", align_mode::middle }, { "mc", align_mode::center_middle }, { "tc", align_mode::center_top }, { "bc", align_mode::center_bottom },
                               { "t", align_mode::top    }, { "ml", align_mode::left_middle   }, { "tl", align_mode::left_top   }, { "bl", align_mode::left_bottom   },
                               { "b", align_mode::bottom }, { "mr", align_mode::right_middle  }, { "tr", align_mode::right_top  }, { "br", align_mode::right_bottom  }} },
            { imagens::gb::f, {{ "n", 0 }, { "v" , (si32)flips::vt }, { "h" , (si32)flips::hz }, { "vh" , (si32)flips::hv }, { "hv" , (si32)flips::hv }} },
        };
        static const auto lc_value_index_map = std::unordered_map<si32, utf::unordered_map<text, si32>>
        {
            //
        };
        auto parse_pair(qiew key, qiew val, auto& attr_index_map, auto& value_index_map) -> std::optional<std::pair<si32, fp32>>
        {
            if (auto key_iter = attr_index_map.find(key); key_iter != attr_index_map.end())
            {
                auto key_index = key_iter->second;
                if (val.empty())
                {
                    return std::pair{ key_index, 0.f };
                }
                else if (auto val_map_iter = value_index_map.find(key_index); val_map_iter != value_index_map.end()) // Look literals.
                {
                    auto& val_map = val_map_iter->second;
                    if (auto val_iter = val_map.find(val); val_iter != val_map.end())
                    {
                        auto val_index = (fp32)val_iter->second;
                        return std::pair{ key_index, val_index };
                    }
                }
                else if (auto v = utf::to_int<fp32>(val)) // Take a numeric value.
                {
                    auto val_index = v.value();
                    return std::pair{ key_index, val_index };
                }
            }
            return std::nullopt;
        }

        using docs = std::array<uptr<lunasvg::Document>, 3>; // Storing White/Black/Transparent variants. //todo request lunasvg to generate RGBAfp32 with A8A8
        struct image
        {
            using opt_gb_attrs_t = std::array<std::optional<fp32>, imagens::gb::attr_count>;
            using opt_lc_attrs_t = std::array<std::optional<fp32>, imagens::lc::attr_count>;
            using gb_attrs_t = std::array<fp32, imagens::gb::attr_count>;
            using lc_attrs_t = std::array<fp32, imagens::lc::attr_count>;

            static constexpr auto document_bit = 1;
            static constexpr auto sub_id_bit   = 2;
            static constexpr auto layers_bit   = 3;

            struct bitmap_t
            {
                sprite fragment{ *std::pmr::new_delete_resource() }; // Rasterized and trimmed fragment within the scaled_fragment_area. Using default resource allocator.
                rect   scaled_fragment_area; // Document full fragment area (sprite::fragment's transparent fields are trimmed).
                twod   xy; // scaled_fragment_area offset inside the target cell region: round(xy * cell_sz)

                void reset()
                {
                    fragment.reset();
                }
            };
            struct layer_t
            {
                ui16           index{};
                text           id;
                text           sub_id; // Layer document's sub-element id.
                wptr<image>    image_wptr;
                opt_gb_attrs_t opt_attrs;
                gb_attrs_t     gb_attrs{}; // Evaluated attributes.
                bitmap_t       bitmap;
                bool           touched{};
                twod           attr_WH; // The size at which the attributes are evaluated.
                si32           attr_digest{}; // Digest in which attributes are evaluated.

                void sync_attrs_with_base(image& base_image, twod image_WH, twod wh)
                {
                    if (attr_digest != base_image.attr_digest || attr_WH != image_WH) // Sync the layer attrs with the original image.
                    {
                        auto changed_bits = 0;
                        auto prev_wh = twod{ gb_attrs[imagens::gb::w], gb_attrs[imagens::gb::h] };
                        for (auto i = 0u; i < gb_attrs.size(); i++)
                        {
                            auto& a = opt_attrs[i];
                            auto changed = std::exchange(gb_attrs[i], a ? a.value() : base_image.gb_attrs[i]) != gb_attrs[i];
                            if (changed) changed_bits |= 1 << i;
                        }
                        if (gb_attrs[imagens::gb::w] == 0) gb_attrs[imagens::gb::w] = (fp32)wh.x;
                        if (gb_attrs[imagens::gb::h] == 0) gb_attrs[imagens::gb::h] = (fp32)wh.y;
                        auto same_size = prev_wh.x == gb_attrs[imagens::gb::w] && prev_wh.y == gb_attrs[imagens::gb::h];
                        auto invalidate_bitmap = !same_size || (changed_bits & ((1 << imagens::gb::uw)
                                                                              | (1 << imagens::gb::vh)
                                                                              | (1 << imagens::gb::fit)));
                        if (invalidate_bitmap)
                        {
                            bitmap.reset();
                        }
                        attr_digest = base_image.attr_digest;
                        attr_WH = image_WH;
                    }
                }
            };

            text          id;
            text          sub_id; // Document's sub-element id.
            text          document;
            si32          document_changed{};
            gb_attrs_t    gb_attrs{};
            si32          changed_gb_attrs{}; // Contains bits indicating which imagens::gb::attr have changed.
            ui16          index{};
            imagens::docs dom;
            byte          stamp{}; // Increment on image update to sync with FE.
            bitmap_t      bitmap;
            std::vector<layer_t> layers;
            si32          attr_digest{}; // Stamp to sync with layers.

            void rasters_reset()
            {
                for (auto& l : layers)
                {
                    l.bitmap.reset();
                }
                bitmap.reset();
            }
            auto empty()
            {
                return document.empty() && layers.empty();
            }
            bool set_changes(si32 new_changed_bits, many& changes, twod cellsz = {})
            {
                auto layers_updated = faux;
                if constexpr (debugmode) log("Received image update:");
                attr_digest++;
                changed_gb_attrs = new_changed_bits;
                auto mask = (ui32)changed_gb_attrs;
                auto j = 0u;
                while (mask != 0)
                {
                    auto i = std::countr_zero(mask);
                    if (i < imagens::gb::attr_count)
                    {
                        gb_attrs[i] = std::any_cast<fp32>(changes[j]);
                        if constexpr (debugmode) log("  %%='%%'", imagens::gb::names[i], gb_attrs[i]);
                    }
                    j++;
                    mask &= mask - 1;
                }
                if (changed_gb_attrs & ((1 << imagens::gb::uw)
                                      | (1 << imagens::gb::vh)
                                      | (1 << imagens::gb::w)
                                      | (1 << imagens::gb::h)
                                      | (1 << imagens::gb::fit)))
                {
                    bitmap.reset();
                }
                auto need_bitmap_reset = faux;
                if (j < changes.size() && (document_changed = std::any_cast<si32>(changes[j++])))
                {
                    if (j < changes.size() && netxs::get_bit<image::sub_id_bit>(document_changed))
                    {
                        sub_id = std::any_cast<text>(changes[j++]);
                        need_bitmap_reset = true;
                        if constexpr (debugmode) log("  sub_id='%%'", sub_id);
                    }
                    if (j < changes.size() && netxs::get_bit<image::document_bit>(document_changed))
                    {
                        document = std::any_cast<text>(changes[j++]);
                        dom = {}; // Request to regenerate DOM.
                        need_bitmap_reset = true;
                        if constexpr (debugmode) log("  document='%value%...'", document.substr(0, std::min(20, (si32)document.size())));
                    }
                    if (j < changes.size() && netxs::get_bit<image::layers_bit>(document_changed)) // Receive foreign layer indexes.
                    {
                        layers_updated = load_layers(j, changes);
                    }
                }
                if (need_bitmap_reset)
                {
                    bitmap.reset();
                }
                else
                {
                    auto x = gb_attrs[imagens::gb::x];
                    auto y = gb_attrs[imagens::gb::y];
                    bitmap.xy = twod{ std::round(fp2d{ x, y } * cellsz) };
                }
                return layers_updated;
            }
            auto get_changes()
            {
                auto changes = many{};
                auto mask = (ui32)changed_gb_attrs;
                while (mask != 0)
                {
                    auto i = std::countr_zero(mask);
                    if (i < imagens::gb::attr_count)
                    {
                        changes.push_back(gb_attrs[i]);
                    }
                    mask &= mask - 1;
                }
                if (document_changed)
                {
                    changes.push_back(document_changed);
                    if (netxs::get_bit<image::sub_id_bit>(document_changed))
                    {
                        changes.push_back(sub_id);
                    }
                    if (netxs::get_bit<image::document_bit>(document_changed))
                    {
                        changes.push_back(document);
                    }
                    if (netxs::get_bit<image::layers_bit>(document_changed))
                    {
                        pack_layers(changes);
                    }
                }
                return changes;
            }
            void pack_layers(many& changes)
            {
                changes.reserve(changes.size() + layers.size() * (3 + imagens::gb::attr_count));
                for (auto& l : layers)
                {
                    changes.push_back(l.index);
                    changes.push_back(l.sub_id);
                    auto mask = 0;
                    auto changed_layer_attr_bits_index = changes.size(); // Placeholder index.
                    changes.push_back(mask); // Reserve a placeholder.
                    for (auto i = 0u; i < l.opt_attrs.size(); i++)
                    {
                        if (auto& attr = l.opt_attrs[i])
                        {
                            mask |= 1 << i;
                            changes.push_back(attr.value());
                        }
                    }
                    changes[changed_layer_attr_bits_index] = mask; // Write to placeholder.
                }
            }
            bool load_layers(ui32 i, many& changes)
            {
                auto layers_updated = i < changes.size();
                if (layers_updated)
                {
                    if constexpr (debugmode) log("  layers update:");
                    layers.clear();
                    while (i < changes.size() - 1)
                    {
                        auto& l = layers.emplace_back(layer_t{ .index = std::any_cast<ui16>(changes[i++]) }); // Index.
                        l.sub_id = std::any_cast<text>(changes[i++]); // Sub_id.
                        if constexpr (debugmode) log("    index=%% sub_id=%%", l.index, l.sub_id);
                        auto changed_layer_attr_bits = std::any_cast<si32>(changes[i++]); // Changed bits.
                        auto mask = (ui32)changed_layer_attr_bits;
                        while (mask != 0 && i < changes.size())
                        {
                            auto j = std::countr_zero(mask);
                            if (j < imagens::gb::attr_count)
                            {
                                l.opt_attrs[j] = std::any_cast<fp32>(changes[i++]); // Attrs.
                                if constexpr (debugmode) log("    %%=%%", imagens::gb::names[j], l.opt_attrs[j].value());
                            }
                            mask &= mask - 1;
                        }
                    }
                }
                return layers_updated;
            }
            bool receive_image_attributes(many& global_attributes) // Receive full metadata set for unknown image.
            {
                auto layers_updated = faux;
                if (global_attributes.size() >= imagens::gb::attr_count + 2)
                {
                    attr_digest++;
                    auto i = 0u;
                    for (; i < imagens::gb::attr_count; i++)
                    {
                        gb_attrs[i] = std::any_cast<fp32>(global_attributes[i]);
                        if constexpr (debugmode) log("  %attr%=%value%", imagens::gb::names[i], gb_attrs[i]);
                    }
                    sub_id   = std::any_cast<text>(global_attributes[i++]);
                    document = std::any_cast<text>(global_attributes[i++]);
                    layers_updated = load_layers(i, global_attributes);
                    reset_changes();
                    if constexpr (debugmode) log("  sub_id='%%' document='%value%...'", sub_id, document.substr(0, std::min(20, (si32)document.size())));
                }
                return layers_updated;
            }
            void reset_changes()
            {
                changed_gb_attrs = {};
                document_changed = {};
            }
            void check_and_set_document(qiew doc_str, qiew sub_id_str = {})
            {
                if (doc_str && document != doc_str)
                {
                    dom = {}; // Request to regenerate DOM.
                    document = doc_str;
                    netxs::set_bit<image::document_bit>(document_changed, true);
                }
                if (sub_id != sub_id_str)
                {
                    sub_id = sub_id_str;
                    bitmap.reset();
                    netxs::set_bit<image::sub_id_bit>(document_changed, true);
                }
            }
            void check_and_set_attr(gb_attrs_t& new_gb_attrs, bool updated_layers)
            {
                attr_digest++;
                assert(new_gb_attrs.size() < sizeof(changed_gb_attrs) * 8);
                for (auto i = 0u; i < new_gb_attrs.size(); i++)
                {
                    if (new_gb_attrs[i] != gb_attrs[i])
                    {
                        changed_gb_attrs |= 1 << i;
                        gb_attrs[i] = new_gb_attrs[i];
                    }
                }
                netxs::set_bit<image::layers_bit>(document_changed, updated_layers);
            }
            auto get_global_attrs()
            {
                auto global_attributes = many{};
                global_attributes.reserve(imagens::gb::attr_count + 2 + layers.size() * (3 + imagens::gb::attr_count));
                for (auto& ga : gb_attrs)
                {
                    global_attributes.push_back(ga);
                }
                global_attributes.push_back(sub_id);
                global_attributes.push_back(document);
                pack_layers(global_attributes);
                return global_attributes;
            }
        };

        template<class T>
        struct cache
        {
        protected:
            using lock = std::recursive_mutex;
            using sync = std::lock_guard<lock>;
            using depo = std::array<netxs::sptr<T>, 65536>; // ~1MB
            using uset = std::vector<ui16>;//std::unordered_set<ui16>;
            using pool = generics::indexer<ui16>;

            lock mutex; // Object map mutex.
            depo store; // Object map.
            uset undef; // List of unknown tokens.
            pool index; // Index pool.

            struct guard : sync
            {
                depo& map;
                uset& unk;
                pool& ind;

                guard(cache& inst)
                    : sync{ inst.mutex },
                       map{ inst.store },
                       unk{ inst.undef },
                       ind{ inst.index }
                { }

                // cache: Set object.
                auto set(auto image_ptr)
                {
                    auto image_index = ind.get_new();
                    if (image_index)
                    {
                        map[image_index] = image_ptr;
                    }
                    else
                    {
                        log("The limit on the number of embedded objects has been reached");
                    }
                    return image_index;
                }
                // cache: Remove object.
                void remove(ui16 image_index)
                {
                    map[image_index].reset();
                    ind.release(image_index);
                }
                // cache: Check the object existence by token.
                auto exists(ui16 image_index)
                {
                    return map[image_index];
                }
                // cache: Request the object metadata if index is not registered.
                auto request_if_absent(ui16 image_index)
                {
                    auto iter = map.find(image_index);
                    auto okay = iter != map.end();
                    if (!okay) unk.push_back(image_index);
                    return okay;
                }
                auto begin() const { return map.begin(); }
                auto begin()       { return map.begin(); }
                auto end() const   { return map.end(); }
                auto end()         { return map.end(); }
            };

        public:
            cache() = default;
            auto storage()
            {
                return guard{ *this };
            }
        };
    }

    // canvas: Grapheme cluster.
    struct cell
    {
        static auto jumbos()
        {
            static auto cache = netxs::generics::cache<text>{};
            return cache.storage();
        }
        static auto images()
        {
            static auto cache = imagens::cache<imagens::image>{};
            return cache.storage();
        }
        static auto register_image(ui16 last_ext_index, std::array<ui16, 65536>& ext_to_int_nat)
        {
            auto images = cell::images(); // Lock. //todo ?Should we place it outside of this hot loop?
            auto image_ptr = ptr::shared(imagens::image{});
            auto last_int_index = images.set(image_ptr);
            if (last_int_index)
            {
                image_ptr->index = last_int_index;
                images.unk.push_back(last_ext_index);
                if constexpr (debugmode) log("request image index: last_int_index=%% last_ext_index=%%", last_int_index, last_ext_index);
                ext_to_int_nat[last_ext_index] = last_int_index; // Update forward map.
                //int_to_ext_nat[last_int_index] = last_ext_index; // Update reverse map.
            }
            return last_int_index;
        }
        struct glyf
        {
            static auto jumbos()
            {
                static auto cache = netxs::generics::cache<text>{};
                return cache.storage();
            }

            // If bytes[1] & 0b11'00'0000 == 0b10'00'0000 (first byte in UTF-8 cannot start with 0b10......) - If so, cluster is stored in an external map (jumbo cluster).
            // In Modified UTF-8, the null character (U+0000) uses the two-byte overlong encoding 11000000 10000000 (hexadecimal C0 80), instead of 00000000 (hexadecimal 00).
            // We do not store Nulls, it is used to create cells with an empty string. //todo If necessary, use Jumbo storage to store clusters containing nulls.
            static constexpr auto size_w_mask = netxs::letoh((ui64)0b0000'1111); // 0-based (w - 1) cell matrix width. (w: 1 - 16)  utf::matrix::kx
            static constexpr auto size_h_mask = netxs::letoh((ui64)0b0011'0000); // 0-based (h - 1) cell matrix height. (h: 1 - 4)  utf::matrix::ky
            static constexpr auto is_rtl_mask = netxs::letoh((ui64)0b0100'0000); // Cluster contains RTL text.
            static constexpr auto reserv_mask = netxs::letoh((ui64)0b1000'0000); // Reserved.
            static constexpr auto letoh_shift = netxs::endian_BE ? 8 * 7 : 0; // Left shift to get bytes[0].

            ui64 token;

            constexpr glyf()
                : token{ 0 }
            { }
            constexpr glyf(glyf const& g)
                : token{ g.token }
            { }
            constexpr glyf(char c)
                : token{ netxs::letoh((ui64)c << 8 * 1) } // bytes[1] = c;
            { }

            constexpr glyf& operator = (glyf const&) = default;
            auto operator == (glyf const& g) const
            {
                return token == g.token;
            }

            constexpr auto size_w() const { return (si32)((token & size_w_mask) >> (letoh_shift + 0)); }
            constexpr auto size_h() const { return (si32)((token & size_h_mask) >> (letoh_shift + 4)); }
            constexpr void size_w(si32 w) { token &= ~size_w_mask; token |= (ui64)w << (letoh_shift + 0); }
            constexpr void size_h(si32 h) { token &= ~size_h_mask; token |= (ui64)h << (letoh_shift + 4); }
            constexpr auto rtl()    const { return token & is_rtl_mask; }
            constexpr void rtl(bool b)    { if (b) token |= is_rtl_mask; else token &= ~is_rtl_mask; }
            constexpr auto is_jumbo() const
            {
                return (token & netxs::letoh((ui64)0b1100'0000'0000'0000)) == netxs::letoh((ui64)0b1000'0000'0000'0000); // (bytes[1] & 0b1100'0000) == 0b1000'0000;
            }
            void set_jumbo_flag()
            {
                token = (token & netxs::letoh(~(ui64)0b1100'0000'0000'0000)) | netxs::letoh((ui64)0b1000'0000'0000'0000); // bytes[1] = (bytes[1] & ~0b1100'0000) | 0b1000'0000;// First byte in UTF-8 cannot start with 0b10xx'xxxx.
            }
            auto bytes() const
            {
                return (char*)&token;
            }
            constexpr void set(ui64 t)
            {
                token = netxs::letoh(t);
            }
            constexpr void set(char c)
            {
                auto isrtl = rtl();
                token = netxs::letoh((ui64)c << 8 * 1); // bytes[1] = c;
                token |= isrtl;
            }
            constexpr void set_c0(char c)
            {
                auto isrtl = rtl();
                token = netxs::letoh(((ui64)'^' << 8 * 1) | ((ui64)('@' + (c & 0b00011111)) << 8 * 2)); // bytes[1] = '^'; bytes[2] = '@' + (c & 0b00011111);
                token |= isrtl;
                size_w(2 - 1);
            }
            constexpr auto mtx() const
            {
                return twod{ size_w() + 1, size_h() + 1 };
            }
            void mtx(si32 w, si32 h)
            {
                size_w(w ? w - 1 : 0);
                size_h(h ? h - 1 : 0);
            }
            auto jgc_token() const // Return token excluding props.
            {
                auto token_copy = token;
                token_copy &= netxs::letoh(~ui64{} << 8 * 1); // ((char*)&token_copy)[0] &= token_mask; // token_mask = (char)0b0000'0000; // Exclude RTL and matrix metadata.
                return token_copy;
            }
            void set_direct(view utf8, si32 w, si32 h)
            {
                auto count = utf8.size();
                auto isrtl = rtl();
                if (count < sizeof(token))
                {
                    token = isrtl; // token = 0;
                    mtx(w, h);
                    std::memcpy(bytes() + 1, utf8.data(), count);
                }
                else
                {
                    token = qiew::hash{}(utf8);
                    token &= ~is_rtl_mask;
                    token |= isrtl;
                    set_jumbo_flag();
                    mtx(w, h);
                    jumbos().add(jgc_token(), utf8);
                }
            }
            // glyf: Cluster length in bytes (if it is not jumbo).
            constexpr auto str_len() const
            {
                return !(token & netxs::letoh((ui64)0xFF << 8 * 1)) ? 0 : // !bytes[1] ? 0 :
                       !(token & netxs::letoh((ui64)0xFF << 8 * 2)) ? 1 : // !bytes[2] ? 1 :
                       !(token & netxs::letoh((ui64)0xFF << 8 * 3)) ? 2 : // !bytes[3] ? 2 :
                       !(token & netxs::letoh((ui64)0xFF << 8 * 4)) ? 3 : // !bytes[4] ? 3 :
                       !(token & netxs::letoh((ui64)0xFF << 8 * 5)) ? 4 : // !bytes[5] ? 4 :
                       !(token & netxs::letoh((ui64)0xFF << 8 * 6)) ? 5 : // !bytes[6] ? 5 :
                       !(token & netxs::letoh((ui64)0xFF << 8 * 7)) ? 6 : 7; // !bytes[7] ? 6 : 7;
            }
            template<svga Mode = svga::vtrgb>
            view get() const
            {
                if constexpr (Mode == svga::dtvt) return {};
                auto crop = is_jumbo() ? view{ jumbos().get(jgc_token()) }
                                       : view(bytes() + 1, str_len());
                if constexpr (Mode != svga::vt_2D)
                {
                    if (crop.size() && crop.front() == utf::matrix::stx)
                    {
                        crop.remove_prefix(1); // Drop cluster initializer.
                    }
                }
                return crop;
            }
            constexpr auto is_space() const
            {
                return (token & netxs::letoh((ui64)0xFF00)) <= netxs::letoh((ui64)whitespace << 8); // (byte)(bytes[1]) <= whitespace;
            }
            constexpr auto is_null() const
            {
                return (token & netxs::letoh((ui64)0xFF00)) == 0; // bytes[1] == 0; // Jumbo bits are nulls. Jumbo mark is the last two bits = 0b10'000000.
            }
            auto jgc() const
            {
                return !is_jumbo() || jumbos().exists(jgc_token());
            }
            // Return cluster storage length.
            constexpr auto len() const
            {
                return is_jumbo() ? sizeof(token) : 1/*first byte (props)*/ + str_len();
            }
            void rst()
            {
                set(whitespace);
            }
            void wipe()
            {
                token = 0;
            }
        };
        struct body
        {
            // Shared attributes.
            static constexpr auto bolded_mask = (ui64)0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000001; // bolded : 1;
            static constexpr auto italic_mask = (ui64)0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000010; // italic : 1;
            static constexpr auto invert_mask = (ui64)0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000100; // invert : 1;
            static constexpr auto overln_mask = (ui64)0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00001000; // overln : 1;
            static constexpr auto strike_mask = (ui64)0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00010000; // strike : 1;
            static constexpr auto unline_mask = (ui64)0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'11100000; // unline : 3; // 0: none, 1: line, 2: biline, 3: wavy, 4: dotted, 5: dashed, 6 - 7: unknown.
            static constexpr auto ucolor_mask = (ui64)0b00000000'00000000'00000000'00000000'00000000'00000000'11111111'00000000; // ucolor : 8; // Underline 256-color 6x6x6-cube index. Alpha not used - it is shared with fgc alpha. If zero - sync with fgc.
            static constexpr auto cursor_mask = (ui64)0b00000000'00000000'00000000'00000000'00000000'00000011'00000000'00000000; // cursor : 2; // 0: None, 1: Underline, 2: Block, 3: I-bar.
            static constexpr auto hplink_mask = (ui64)0b00000000'00000000'00000000'00000000'00000000'00000100'00000000'00000000; // hyperlink : 1; // cell: strore string hash.
            static constexpr auto blinks_mask = (ui64)0b00000000'00000000'00000000'00000000'00000000'00001000'00000000'00000000; // blinks : 1;
            static constexpr auto hidden_mask = (ui64)0b00000000'00000000'00000000'00000000'00000000'00010000'00000000'00000000; // hidden : 1; // Hidden character.
          //static constexpr auto reserv_mask = (ui64)0b00000000'00000000'00000000'00000000'11111111'11100000'00000000'00000000; // reserv : 11;
            static constexpr auto shadow_mask = (ui64)0b00000000'00000000'00000000'11111111'00000000'00000000'00000000'00000000; // shadow : 8; // Shadow bits.
            // Unique attributes. From 24th bit.
            static constexpr auto mosaic_mask = (ui64)0b00000000'00000000'11111111'00000000'00000000'00000000'00000000'00000000; // ui32 mosaic : 8; // High 3 bits -> y-fragment (0-4 utf::matrix::ky), low 5 bits -> x-fragment (0-16 utf::matrix::kx). // Ref:  https://gitlab.freedesktop.org/terminal-wg/specifications/-/issues/23
            static constexpr auto curbgc_mask = (ui64)0b00000000'11111111'00000000'00000000'00000000'00000000'00000000'00000000; // bgcclr : 8; // Cursor 256-color 6x6x6-cube index. Alpha not used.
            static constexpr auto curfgc_mask = (ui64)0b11111111'00000000'00000000'00000000'00000000'00000000'00000000'00000000; // fgcclr : 8; // Cursor 256-color 6x6x6-cube index. Alpha not used.

            static constexpr auto x_bits = utf::matrix::x_bits; // Character geometry x fragment selector bits (for mosaic_mask).
            static constexpr auto y_bits = utf::matrix::y_bits; // Character geometry y fragment selector bits offset (for mosaic_mask).
            static constexpr auto shared_bits = ((ui64)1 << netxs::field_offset<mosaic_mask>()) - 1;

            //todo Cf's can not be entered: even using paste from clipboard
            //don't show (drop) Cf's but allow input it in any order (app is responsible to show it somehow)

            //todo application context: word delimeters (use it in a word/line wrapping, check the last codepoint != Cf | Spc):
            // append prev: U+200C ZERO WIDTH NON-JOINER
            // append prev: U+00AD SOFT HYPHEN

            ui64 token;

            constexpr body()
                : token{ }
            { }
            constexpr body(body const& b)
                : token{ b.token }
            { }
            constexpr body(si32 mosaic)
                : token{ (ui64)(ui32)mosaic << netxs::field_offset<mosaic_mask>() }
            { }
            constexpr body(body const& b, si32 mosaic)
                : token{ (b.token & ~mosaic_mask) | ((ui64)(ui32)mosaic << netxs::field_offset<mosaic_mask>()) }
            { }

            constexpr body& operator = (body const&) = default;
            bool operator == (body const& b) const
            {
                return token == b.token;
            }
            bool operator != (body const& b) const
            {
                return !operator==(b);
            }
            bool like(body const& b) const
            {
                return (token & body::shared_bits) == (b.token & body::shared_bits);
            }
            void meta(body const& b)
            {
                token = (token & body::mosaic_mask) | (b.token & ~body::mosaic_mask); // Keep mosaic.
            }
            void meta_shadow(body const& b)
            {
                token = (token & (body::mosaic_mask | body::shadow_mask)) | (b.token & ~body::mosaic_mask); // Keep mosaic and OR'ing shadow.
            }
            void meta_shadow_matrix(body const& b)
            {
                token = (token & body::shadow_mask) | b.token; // Update meta with OR'ing shadow.
            }
            template<svga Mode = svga::vtrgb, bool UseSGR = true, class T>
            void get(body& base, T& dest) const
            {
                if constexpr (Mode == svga::dtvt) return;
                if (!like(base))
                {
                    if constexpr (UseSGR) // It is not available in the Linux and Win8 consoles.
                    {
                        if constexpr (Mode == svga::vt_2D)
                        {
                            if (auto cursor = token & cursor_mask; cursor != (base.token & cursor_mask)) dest.cursor0((si32)(cursor >> netxs::field_offset<cursor_mask>()));
                            if (auto shadow = token & shadow_mask; shadow != (base.token & shadow_mask)) dest.dim(    (si32)(shadow >> netxs::field_offset<shadow_mask>()));
                            //todo check image
                            //if (auto imageH = token & imageH_mask; imageH != (base.token & imageH_mask)) dest.imageH( (ui32)(imageH >> netxs::field_offset<imageH_mask>()));
                        }
                        if constexpr (Mode != svga::vt16) // It is not available in the Linux and Win8 consoles.
                        {
                            if (auto bolded = token & bolded_mask; bolded != (base.token & bolded_mask)) dest.bld(!!bolded);
                            if (auto italic = token & italic_mask; italic != (base.token & italic_mask)) dest.itc(!!italic);
                            if (auto invert = token & invert_mask; invert != (base.token & invert_mask)) dest.inv(!!invert);
                            if (auto overln = token & overln_mask; overln != (base.token & overln_mask)) dest.ovr(!!overln);
                            if (auto strike = token & strike_mask; strike != (base.token & strike_mask)) dest.stk(!!strike);
                            if (auto blinks = token & blinks_mask; blinks != (base.token & blinks_mask)) dest.blk(!!blinks);
                            if (auto hidden = token & hidden_mask; hidden != (base.token & hidden_mask)) dest.hid(!!hidden);
                            if (auto unline = token & unline_mask; unline != (base.token & unline_mask)) dest.und((si32)(unline >> netxs::field_offset<unline_mask>()));
                            if (auto ucolor = token & ucolor_mask; ucolor != (base.token & ucolor_mask)) dest.unc((si32)(ucolor >> netxs::field_offset<ucolor_mask>()));
                        }
                        else
                        {
                            if (auto unline = token & unline_mask; unline != (base.token & unline_mask)) dest.inv((si32)(unline >> netxs::field_offset<unline_mask>()));
                        }
                    }
                    base.token = token;
                }
            }
            void wipe()
            {
                token = 0;
            }
            void reverse()
            {
                token ^= invert_mask;
            }

            void bld(bool b)         { token &= ~bolded_mask; token |= ((ui64)b << netxs::field_offset<bolded_mask>()); }
            void itc(bool b)         { token &= ~italic_mask; token |= ((ui64)b << netxs::field_offset<italic_mask>()); }
            void inv(bool b)         { token &= ~invert_mask; token |= ((ui64)b << netxs::field_offset<invert_mask>()); }
            void ovr(bool b)         { token &= ~overln_mask; token |= ((ui64)b << netxs::field_offset<overln_mask>()); }
            void stk(bool b)         { token &= ~strike_mask; token |= ((ui64)b << netxs::field_offset<strike_mask>()); }
            void blk(bool b)         { token &= ~blinks_mask; token |= ((ui64)b << netxs::field_offset<blinks_mask>()); }
            void hid(bool b)         { token &= ~hidden_mask; token |= ((ui64)b << netxs::field_offset<hidden_mask>()); }
            void dim(si32 n)         { token &= ~shadow_mask; token |= ((ui64)(ui32)n << netxs::field_offset<shadow_mask>()); }
            void und(si32 n)         { token &= ~unline_mask; token |= ((ui64)(ui32)n << netxs::field_offset<unline_mask>()); }
            void unc(si32 c)         { token &= ~ucolor_mask; token |= ((ui64)(ui32)c << netxs::field_offset<ucolor_mask>()); }
            void cur(si32 s)         { token &= ~cursor_mask; token |= ((ui64)(ui32)s << netxs::field_offset<cursor_mask>()); }
            void mosaic(si32 m)      { token &= ~mosaic_mask; token |= ((ui64)(ui32)m << netxs::field_offset<mosaic_mask>()); }
            void  xy(ui64 m)         { token &= ~mosaic_mask; token |= m; }
            void  xy(si32 x, si32 y) { mosaic(x + (y << y_bits)); }
            void fuse_dim(si32 n)    { token |= (ui64)(ui32)n << netxs::field_offset<shadow_mask>(); }
            void cursor0(si32 c)     { token &= ~cursor_mask; token |= ((ui64)(ui32)c << netxs::field_offset<cursor_mask>()); }
            void cursor_color(argb bgc, argb fgc)
            {
                auto bg = bgc.to_256cube();
                auto fg = fgc.to_256cube();
                token &= ~(curbgc_mask | curfgc_mask);
                token |= ((ui64)bg << netxs::field_offset<curbgc_mask>());
                token |= ((ui64)fg << netxs::field_offset<curfgc_mask>());
            }
            //void hplink0(ui64 c) { token &= ~hplink_mask; token |= (ui64)(c << netxs::field_offset<hplink_mask>()); }

            constexpr bool bld()    const { return !!(token & bolded_mask); }
            constexpr bool itc()    const { return !!(token & italic_mask); }
            constexpr bool inv()    const { return !!(token & invert_mask); }
            constexpr bool ovr()    const { return !!(token & overln_mask); }
            constexpr bool stk()    const { return !!(token & strike_mask); }
            constexpr bool blk()    const { return !!(token & blinks_mask); }
            constexpr bool hid()    const { return !!(token & hidden_mask); }
            constexpr si32 und()    const { return (si32)((token & unline_mask) >> netxs::field_offset<unline_mask>()); }
            constexpr si32 dim()    const { return (si32)((token & shadow_mask) >> netxs::field_offset<shadow_mask>()); }
            constexpr si32 unc()    const { return (si32)((token & ucolor_mask) >> netxs::field_offset<ucolor_mask>()); }
            constexpr si32 cur()    const { return (si32)((token & cursor_mask) >> netxs::field_offset<cursor_mask>()); }
            //constexpr si32 cursor0() const { return (token & cursor_mask); }
            //constexpr si32 hplink0() const { return (token & hplink_mask); }
            //constexpr si32 fusion0() const { return (token & fusion_mask); }
            constexpr ui64  xy()    const { return (token & mosaic_mask); }
            constexpr si32 mosaic() const { return (si32)((token & mosaic_mask) >> netxs::field_offset<mosaic_mask>()); }
            constexpr auto cursor_color() const
            {
                auto bgi = (byte)((token & curbgc_mask) >> netxs::field_offset<curbgc_mask>());
                auto fgi = (byte)((token & curfgc_mask) >> netxs::field_offset<curfgc_mask>());
                auto bgc = bgi ? argb{ argb::vt256[bgi] } : argb{};
                auto fgc = fgi ? argb{ argb::vt256[fgi] } : argb{};
                return std::pair{ bgc, fgc };
            }
        };
        struct clrs
        {
            argb bg;
            argb fg;

            constexpr clrs() = default;
            constexpr clrs(auto colors)
                : bg{ *(colors.begin() + 0) },
                  fg{ *(colors.begin() + 1) }
            { }
            constexpr clrs(clrs const& c)
                : bg{ c.bg },
                  fg{ c.fg }
            { }

            constexpr clrs& operator = (clrs const&) = default;
            constexpr bool operator == (clrs const& c) const
            {
                return bg == c.bg
                    && fg == c.fg;
                // sizeof(*this);
            }
            constexpr bool operator != (clrs const& c) const
            {
                return !operator==(c);
            }

            static void fix_collision_vga16(auto& f) // Fix color collision in low-color mode.
            {
                assert(f < 16);
                if (f <= tint::whitedk) f += 8;
                else                    f -= 8;
            }
            static void fix_collision_vtm16(auto& f) // Fix color collision in low-color mode.
            {
                assert(f < 16);
                     if (f  < tint16::whitelt ) f += 1;
                else if (f == tint16::whitelt ) f -= 1;
                else if (f <= tint16::yellowdk) f += 6; // Make it lighter.
                else if (f <= tint16::cyanlt  ) f = tint16::graylt;
                else if (f <= tint16::yellowlt) f -= 6; // Make it darker.
            }
            static void fix_collision_vtm8(auto& f) // Fix color collision in low-color mode.
            {
                assert(f < 8);
                     if (f  < tint16::whitelt) f += 1;
                else if (f == tint16::whitelt) f -= 1;
                else                           f = tint16::whitedk;
            }
            template<svga Mode = svga::vtrgb, bool UseSGR = true, class T>
            void get(clrs& base, T& dest) const
            {
                if constexpr (Mode == svga::dtvt) return;
                if constexpr (Mode == svga::vt16)
                {
                    if (fg != base.fg || bg != base.bg)
                    {
                        if constexpr (UseSGR)
                        {
                            auto f = fg.is_indexed();
                            auto b = bg.is_indexed();
                            f = f ? argb{ argb::vt256[f - 1] }.to_vtm16() : fg.to_vtm16();
                            b = b ? argb{ argb::vt256[b - 1] }.to_vtm8()  : bg.to_vtm8();
                            if (fg != bg && f == b) // Avoid color collizions.
                            {
                                fix_collision_vtm8(f);
                                if (bg != base.bg) dest.bgc_8(b);
                                dest.fgc_16(f);
                            }
                            else
                            {
                                if (bg != base.bg) dest.bgc_8(b);
                                if (fg != base.fg) dest.fgc_16(f);
                            }
                        }
                        base.bg = bg;
                        base.fg = fg;
                    }
                }
                else
                {
                    if (bg != base.bg)
                    {
                        base.bg = bg;
                        if constexpr (UseSGR) dest.template bgc<Mode>(bg);
                    }
                    if (fg != base.fg)
                    {
                        base.fg = fg;
                        if constexpr (UseSGR) dest.template fgc<Mode>(fg);
                    }
                }
            }
            void wipe()
            {
                bg.wipe();
                fg.wipe();
            }
        };

        clrs uv; // 8U, cell: Fg and bg colors.
        glyf gc; // 8U, cell: Grapheme cluster.
        body st; // 8U, cell: Style attributes.
        ui64 px{}; // 8U, cell: Image W H c r
        ui32 p2{}; // 4U, cell: Image index and stamp.
        id_t id{}; // 4U, cell: Link ID.

        cell()
        { }
        cell(char c)
            : gc{ c },
              st{ utf::matrix::mosaic<11> }
        {
            // sizeof(glyf);
            // sizeof(clrs);
            // sizeof(body);
            // sizeof(id_t);
            // sizeof(pict);
            // sizeof(cell);
        }
        cell(view utf8)
        {
            txt(utf8);
        }
        cell(cell const& base)
            : uv{ base.uv },
              gc{ base.gc },
              st{ base.st },
              px{ base.px },
              p2{ base.p2 },
              id{ base.id }
        { }
        cell(cell const& base, char c)
            : uv{ base.uv },
              gc{ c       },
              st{ base.st, utf::matrix::mosaic<11> },
              px{ base.px },
              p2{ base.p2 },
              id{ base.id }
        { }

        auto operator == (cell const& c) const
        {
            return uv == c.uv
                && st == c.st
                && gc == c.gc
                && px == c.px
                && p2 == c.p2;
        }
        auto operator != (cell const& c) const
        {
            return !operator==(c);
        }
        auto& operator = (cell const& c)
        {
            uv = c.uv;
            gc = c.gc;
            st = c.st;
            px = c.px;
            p2 = c.p2;
            id = c.id;
            return *this;
        }

        operator bool () const { return st.xy(); } // cell: Return true if cell contains printable character.

        // Image prop masks:
        // Internal, stored within the cell:
        //  bits | value
        // ------|------
        //  16   | Image index. ui16
        //  1    | o ontop. 0/1
        //  1    | is sixel (destructible). 0/1
        //  16   | c fragment (column). ui16
        //  16   | r fragment (row). ui16
        //  16   | W cell canvas width. ui16
        //  16   | H cell canvas height. ui16
        //  8    | stamp 0..255

        static constexpr auto p2_index16_mask = (ui32)0b00000000'00000000'11111111'11111111;
        static constexpr auto p2_stamp_8_mask = (ui32)0b00000000'11111111'00000000'00000000;
        static constexpr auto p2_ontop_1_mask = (ui32)0b00000001'00000000'00000000'00000000;
        static constexpr auto p2_sixel_1_mask = (ui32)0b00000010'00000000'00000000'00000000;
      //static constexpr auto p2_reserv7_mask = (ui32)0b11111100'00000000'00000000'00000000;

        static constexpr auto px_imgX_16_mask = (ui64)0b00000000'00000000'00000000'00000000'00000000'00000000'11111111'11111111;
        static constexpr auto px_imgY_16_mask = (ui64)0b00000000'00000000'00000000'00000000'11111111'11111111'00000000'00000000;
        static constexpr auto px_imgW_16_mask = (ui64)0b00000000'00000000'11111111'11111111'00000000'00000000'00000000'00000000;
        static constexpr auto px_imgH_16_mask = (ui64)0b11111111'11111111'00000000'00000000'00000000'00000000'00000000'00000000;

        auto get_image_WH() const
        {
            auto imgW = (si32)(ui16)netxs::get_field<px_imgW_16_mask>(px);
            auto imgH = (si32)(ui16)netxs::get_field<px_imgH_16_mask>(px);
            return twod{ imgW, imgH };
        }
        auto get_image_cr() const
        {
            auto c = (si32)(ui16)netxs::get_field<px_imgX_16_mask>(px);
            auto r = (si32)(ui16)netxs::get_field<px_imgY_16_mask>(px);
            return twod{ c, r };
        }
        auto& set_image_cr(si32 c, si32 r)
        {
            netxs::set_field<px_imgX_16_mask>(c, px);
            netxs::set_field<px_imgY_16_mask>(r, px);
            return *this;
        }
        auto& set_image_WH(si32 imgW, si32 imgH)
        {
            netxs::set_field<px_imgW_16_mask>(imgW, px);
            netxs::set_field<px_imgH_16_mask>(imgH, px);
            return *this;
        }
        auto get_image_index() const { return (ui16)netxs::get_field<p2_index16_mask>(p2); }
        auto get_image_sixel() const { return       netxs::get_field<p2_sixel_1_mask>(p2); }
        auto get_image_stamp() const { return       netxs::get_field<p2_stamp_8_mask>(p2); }
        auto get_image_ontop() const
        {
            auto index = get_image_index();
            auto ontop = netxs::get_field<p2_ontop_1_mask>(p2);
            return std::pair{ index, ontop };
        }
        auto& set_image_index(si32 n) { netxs::set_field<p2_index16_mask>(n, p2); return *this; }
        auto& set_image_sixel(si32 n) { netxs::set_field<p2_sixel_1_mask>(n, p2); return *this; }
        auto&  or_image_sixel(si32 n) { netxs::set_field<p2_sixel_1_mask>(n | netxs::get_field<p2_sixel_1_mask>(p2), p2); return *this; }
        auto& set_image_ontop(si32 n) { netxs::set_field<p2_ontop_1_mask>(n, p2); return *this; }
        auto& set_image_stamp(si32 n) { netxs::set_field<p2_stamp_8_mask>(n, p2); return *this; }
        auto& inc_image_stamp(si32 n) { netxs::set_field<p2_stamp_8_mask>(get_image_stamp() + n, p2); return *this; }
        auto& set_image_attrs(imagens::image& image, imagens::image::lc_attrs_t& lc_attrs)
        {
            set_image_index(image.index);
            set_image_stamp(image.stamp);
            set_image_WH(   (si32)lc_attrs[imagens::lc::W],
                            (si32)lc_attrs[imagens::lc::H]);
            set_image_cr(   (si32)lc_attrs[imagens::lc::c],
                            (si32)lc_attrs[imagens::lc::r]);
            set_image_ontop((si32)lc_attrs[imagens::lc::o]);
            return *this;
        }

        //todo rename to has_image
        auto raw() const
        {
            return !!p2;
        }
        void reset_px()
        {
            px = {};
            p2 = {};
        }
        auto is_empty() const // cell: Return true if cell is absolutely empty.
        {
            return uv.bg.token == 0 && uv.fg.token == 0 && gc.token == 0 && st.token == 0 && id == 0 && p2 == 0;
        }
        auto same_txt(cell const& c) const // cell: Compare clusters.
        {
            return gc == c.gc;
        }
        bool like(cell const& c) const // cell: Meta comparison of the two cells.
        {
            return uv == c.uv
                && st.like(c.st)
                && (!raw() || (px == c.px && p2 == c.p2));
        }
        void wipe() // cell: Set colors, attributes and grapheme cluster to zero.
        {
            uv.wipe();
            gc.wipe();
            st.wipe();
            px = 0;
            p2 = 0;
        }
        // cell: Blend two cells according to visibility and other attributes (keep underlying image).
        auto& fuse_keep_image(cell const& c)
        {
            if (uv.fg.chan.a == 0xFF) uv.fg.mix_one(c.uv.fg);
            else                      uv.fg.mix(c.uv.fg);

            if (uv.bg.chan.a == 0xFF) uv.bg.mix_one(c.uv.bg);
            else                      uv.bg.mix(c.uv.bg);

            if (c.st.xy())
            {
                gc = c.gc;
                if (c.uv.bg.token == 0) // OR'ing the shadow if bg is completely transparent.
                {
                    st.meta_shadow_matrix(c.st);
                }
                else
                {
                    st = c.st;
                }
            }
            else
            {
                if (c.uv.bg.token == 0) // OR'ing the shadow if bg is completely transparent.
                {
                    st.meta_shadow(c.st);
                }
                else
                {
                    st.meta(c.st);
                }
            }
            return *this;
        }
        // cell: Blend two cells according to visibility and other attributes.
        auto& fuse(cell const& c)
        {
            fuse_keep_image(c);
            px = c.px;
            p2 = c.p2;
            return *this;
        }
        // cell: Blend two cells if text part != '\0'.
        inline void lite(cell const& c)
        {
            if (!c.gc.is_null()) fuse(c); // if (c.gc.bytes[1] != 0) fuse(c);
        }
        // cell: Blend cell colors.
        void mix(cell const& c)
        {
            uv.fg.mix_one(c.uv.fg);
            uv.bg.mix_one(c.uv.bg);
            if (c.st.xy())
            {
                st = c.st;
                gc = c.gc;
            }
            px = c.px;
            p2 = c.p2;
        }
        // cell: Blend cell colors.
        void blend(cell const& c)
        {
            uv.fg.mix(c.uv.fg);
            uv.bg.mix(c.uv.bg);
        }
        // cell: Blend colors using alpha.
        void mix(cell const& c, byte alpha)
        {
            uv.fg.mix(c.uv.fg, alpha);
            uv.bg.mix(c.uv.bg, alpha);
            px = c.px;
            p2 = c.p2;
            if (c.st.xy())
            {
                st = c.st;
                gc = c.gc;
            }
        }
        // cell: Blend colors using alpha.
        void mixfull(cell const& c, si32 alpha)
        {
            if (c.id) id = c.id;
            if (c.st.xy())
            {
                st = c.st;
                gc = c.gc;
                uv.fg = uv.bg; // The character must be on top of the cell background. (see block graphics)
            }
            px = c.px;
            p2 = c.p2;
            uv.fg.mix(c.uv.fg, alpha);
            uv.bg.mix(c.uv.bg, alpha);
        }
        // cell: Blend two cells and set specified id.
        void fuse(cell const& c, id_t oid)
        {
            fuse(c);
            id = oid;
        }
        // cell: Blend two cells and set id if any.
        void fusefull(cell const& c)
        {
            fuse(c);
            if (c.id) id = c.id;
        }
        // cell: Blend two cells and set id if any (keep the underlying image).
        void fusefull_keep_image(cell const& c)
        {
            fuse_keep_image(c);
            if (c.id) id = c.id;
            if (px && !c.px)
            {
                set_image_ontop(0); // Place image under the text.
            }
        }
        // cell: Blend two cells and set id if any (fg = bg * c.fg).
        void overlay(cell const& c)
        {
            auto bg_opaque = uv.bg.chan.a == 0xFF;
            if (c.st.xy() || c.st.und())
            {
                uv.fg = uv.bg;
                if (bg_opaque) uv.fg.mix_one(c.uv.fg);
                else           uv.fg.mix(c.uv.fg);
            }
            else
            {
                if (uv.fg.chan.a == 0xFF) uv.fg.mix_one(c.uv.bg);
                else                      uv.fg.mix(c.uv.bg);
            }
            gc = c.gc;
            st = c.st;
            if (bg_opaque) uv.bg.mix_one(c.uv.bg);
            else           uv.bg.mix(c.uv.bg);
            px = c.px;
            p2 = c.p2;
            if (c.id) id = c.id;
        }
        // cell: Merge two cells and set id.
        void fuseid(cell const& c)
        {
            fuse(c);
            id = c.id;
        }
        void meta(cell const& c)
        {
            uv = c.uv;
            st.meta(c.st);
            px = c.px;
            p2 = c.p2;
        }
        void skipnulls(cell const& c)
        {
            if (c.gc.is_null()) // Keep gc intact.
            {
                if (c.uv.bg.token != argb::default_color) // Completely ignore transparent nulls (do nothing, move cursor forward).
                {
                    meta(c);
                }
            }
            else
            {
                gc = c.gc;
                if (c.uv.bg.token != argb::default_color) // Update gc while keeping SGR attributes (if bgc==0x00'FF'FF'FF).
                {
                    uv = c.uv;
                    st = c.st;
                }
                else // Copy all.
                {
                    st.xy(c.st.xy());
                }
                if (get_image_sixel() || c.raw()) // Terminal output is non-destructive for images but sixels.
                {
                    px = c.px;
                    p2 = c.p2;
                }
            }
        }
        // cell: Get differences of the visual attributes only (ANSI CSI/SGR format).
        template<svga Mode = svga::vtrgb, bool UseSGR = true, class T>
        void scan_attr(cell& base, T& dest) const
        {
            if (!like(base))
            {
                //todo additionally consider UNIQUE ATTRIBUTES
                uv.get<Mode, UseSGR>(base.uv, dest);
                st.get<Mode, UseSGR>(base.st, dest);
                //todo raw bitmap
            }
        }
        // cell: Render colored whitespaces instead of "░▒▓".
        template<svga Mode = svga::vtrgb, bool UseSGR = true, class T>
        void filter(cell& base, T& dest) const
        {
            if constexpr (UseSGR && (Mode == svga::vtrgb || Mode == svga::vt_2D))
            {
                auto egc = gc.get<Mode>();
                if (egc.size() == 3 && egc[0] == '\xE2' && egc[1] == '\x96')
                {
                    auto k = 0;
                         if (egc[2] == '\x91') k = 64;  // "░"
                    else if (egc[2] == '\x92') k = 96;  // "▒"
                    else if (egc[2] == '\x93') k = 128; // "▓"
                    else
                    {
                        dest += egc;
                        return;
                    }
                    auto bgc = argb::transit(base.uv.bg, base.uv.fg, k);
                    if (bgc != base.uv.bg)
                    {
                        base.uv.bg = bgc;
                        dest.template bgc<Mode>(bgc);
                    }
                    dest += whitespace;
                }
                else dest += egc;
            }
            else dest += gc.get<Mode>();
        }
        // cell: Get differences (ANSI CSI/SGR format) of "base" and add it to "dest" and update the "base".
        template<svga Mode = svga::vtrgb, bool UseSGR = true, class T>
        void scan(cell& base, T& dest) const
        {
            if constexpr (Mode != svga::dtvt)
            {
                if (!like(base))
                {
                    //todo additionally consider UNIQUE ATTRIBUTES
                    uv.get<Mode, UseSGR>(base.uv, dest);
                    st.get<Mode, UseSGR>(base.st, dest);
                    //todo raw bitmap
                }
                if (st.xy() && !gc.is_space()) filter<Mode, UseSGR>(base, dest);
                else                           dest += whitespace;
                //if (st.xy())
                //{
                //    filter<Mode, UseSGR>(base, dest);
                //}
                //else // Allow nulls to be copyable.
                //{
                //    auto cluster = gc.get<Mode>();
                //    if (cluster.size()) dest += cluster;
                //    else                dest += emptyspace;
                //}
            }
        }
        // cell: Check that the halves belong to the same wide glyph.
        bool check_pair(cell const& next) const
        {
            return gc == next.gc && like(next);
        }
        // cell: Return cluster matrix metadata.
        auto whxy() const  { return std::tuple{ (si32)(gc.size_w() + 1),
                                                (si32)(gc.size_h() + 1),
                                                (si32)(st.mosaic() & cell::body::x_bits),
                                                (si32)(st.mosaic() >> cell::body::y_bits) }; }
        // cell: Return true if cell is at the matrix right border.
        auto matrix_end() const
        {
            auto w = gc.size_w() + 1;
            return w > 1 && w == /*x*/(st.mosaic() & cell::body::x_bits);
        }
        // cell: Get text from cell.
        template<svga Mode = svga::vtrgb, bool Select_11_only = true>
        void scan_text(text& dest) const
        {
            auto shadow = gc.get<Mode>();
            auto [w, h, x, y] = whxy();
            if (w == 0 || h == 0 || y != 1 || x != 1 || shadow.empty() || (byte)shadow.front() < 32) // 2D fragment is either non-standard or empty or C0.
            {
                if constexpr (Select_11_only)
                {
                    if (y > 1 || x > 2) dest += whitespace;
                }
                else
                {
                    dest += whitespace;
                }
            }
            else
            {
                dest += shadow;
            }
        }
        // cell: Convert to text. Ignore right half. Convert binary clusters (eg: ^C -> 0x03).
        template<bool Select_11_only = faux>
        void scan(text& dest) const
        {
            auto [w, h, x, y] = whxy();
            if constexpr (Select_11_only)
            {
                if (x == 1 && y == 1)
                {
                    auto shadow = gc.get();
                    if (shadow.size() == 2 && shadow.front() == '^')
                    {
                        dest += shadow[1] & (' ' - 1);
                    }
                    else
                    {
                        dest += shadow;
                    }
                }
            }
            else
            {
                if (w == 0 || h != 1 || x != 1)
                {
                    dest += whitespace;
                }
                else
                {
                    auto shadow = gc.get();
                    if (shadow.size() == 2 && shadow.front() == '^')
                    {
                        dest += shadow[1] & (' ' - 1);
                    }
                    else
                    {
                        dest += shadow;
                    }
                }
            }
        }
        // cell: Convert non-printable chars to escaped.
        template<class C>
        auto& c0_to_txt(C chr)
        {
            auto c = static_cast<char>(chr);
            if (c < ' ') gc.set_c0(c);
            return *this;
        }
        // cell: Highlight both foreground and background.
        auto& xlight(si32 factor = 1)
        {
            uv.bg.xlight(factor, uv.fg);
            return *this;
        }
        // cell: Highlight by underlining.
        auto& underlight(si32 factor = 1)
        {
            auto fgc = uv.fg;
            auto bgc = uv.bg;
            if (st.inv()) std::swap(fgc, bgc);
            auto index = st.unc();
            auto color = st.und() == unln::line ? index ? argb{ argb::vt256[index] }.alpha(fgc.alpha()) : fgc
                                                : bgc;
            color.xlight(factor);
            st.unc(color.to_256cube());
            st.und(unln::line);
            return *this;
        }
        // cell: Invert both foreground and background.
        void invert()
        {
            uv.fg.invert();
            uv.bg.invert();
        }
        // cell: Swap foreground and background.
        void reverse()
        {
            std::swap(uv.fg, uv.bg);
        }
        // cell: Flip inversion bit.
        void invbit()
        {
            st.reverse();
        }
        // cell: Desaturate and dim fg color.
        void disabled()
        {
            uv.fg.grayscale();
            uv.fg.shadow(78);
            uv.fg.chan.a = 0xff;
        }
        auto& dim(si32 n)
        {
            if (n == -1)
            {
                uv.fg.faint();
            }
            else
            {
                st.dim(std::clamp(n, 0, 255));
            }
            return *this;
        }
        // cell: Is the cell not transparent?
        bool is_alpha_blendable() const
        {
            return uv.bg.is_alpha_blendable();//&& uv.param.fg.is_alpha_blendable();
        }
        // cell: Cell transitional color blending (fg/bg only).
        void avg(cell const& c1, cell const& c2, si32 level)
        {
            uv.fg = argb::transit(c1.uv.fg, c2.uv.fg, level);
            uv.bg = argb::transit(c1.uv.bg, c2.uv.bg, level);
        }
        // cell: Set grapheme cluster.
        void set_gc(cell const& c)
        {
            gc = c.gc;
            st.xy(c.st.xy());
        }
        // cell: Same grapheme cluster fragment.
        auto same_fragment(cell const& c) const
        {
            return gc == c.gc && st.xy() == c.st.xy();
        }
        // cell: Reset grapheme cluster.
        void set_gc()
        {
            gc.wipe();
            st.xy(0);
        }
        // cell: Copy view of the cell (preserve ID).
        auto& set(cell const& c) { uv = c.uv;
                                   st = c.st;
                                   gc = c.gc;
                                   px = c.px;
                                   p2 = c.p2;              return *this; }
        auto& bgc(argb c)        { uv.bg = c;              return *this; } // cell: Set background color.
        auto& fgc(argb c)        { uv.fg = c;              return *this; } // cell: Set foreground color.
        auto& bga(si32 k)        { uv.bg.chan.a = (byte)k; return *this; } // cell: Set background alpha/transparency.
        auto& fga(si32 k)        { uv.fg.chan.a = (byte)k; return *this; } // cell: Set foreground alpha/transparency.
        auto& alpha(si32 k)      { uv.bg.chan.a = (byte)k;
                                   uv.fg.chan.a = (byte)k; return *this; } // cell: Set alpha/transparency (background and foreground).
        // cell: Set/Reset bold attribute. //todo ? SGR22: If b=faux and st.bld()=faux then un-dim fg color.
        auto& bld(bool b)
        {
            //if (st.bld() == faux && b == faux) // Un-dim fg color.
            //{
            //    uv.fg.bright(2);
            //}
            st.bld(b);
            return *this;
        }
        auto& itc(bool b)        { st.itc(b);              return *this; } // cell: Set italic attribute.
        auto& und(si32 n)        { st.und(n);              return *this; } // cell: Set underline attribute.
        auto& unc(argb c)        { st.unc(c.to_256cube()); return *this; } // cell: Set underline color.
        auto& unc(si32 c)        { st.unc(c);              return *this; } // cell: Set underline color.
        auto& cur(si32 s)        { st.cur(s);              return *this; } // cell: Set cursor style.
        auto& img(ui96 p)        { px = p.u64; p2 = p.u32; return *this; } // cell: Set attached image.
        auto& ovr(bool b)        { st.ovr(b);              return *this; } // cell: Set overline attribute.
        auto& inv(bool b)        { st.inv(b);              return *this; } // cell: Set invert attribute.
        auto& stk(bool b)        { st.stk(b);              return *this; } // cell: Set strikethrough attribute.
        auto& blk(bool b)        { st.blk(b);              return *this; } // cell: Set blink attribute.
        auto& hid(bool b)        { st.hid(b);              return *this; } // cell: Set hidden attribute.
        auto& rtl(bool b)        { gc.rtl(b);              return *this; } // cell: Set RTL attribute.
        auto& mtx(twod p)        { gc.mtx(p.x, p.y);       return *this; } // cell: Set glyph matrix.
        auto& xy(si32 x, si32 y) { st.xy(x, y);            return *this; } // cell: Set glyph fragment.
        auto& link(id_t oid)     { id = oid;               return *this; } // cell: Set object ID.
        auto& cursor0(si32 i)    { st.cursor0(i);          return *this; } // cell: Set cursor inside the cell.
        auto& link(cell const& c){ id = c.id;              return *this; } // cell: Set object ID.
        // cell: Set cluster unidata width.
        auto& wdt(si32 vs)
        {
            auto [w, h, x, y] = utf::matrix::whxy(vs);
            gc.mtx(w, h);
            st.xy(x, y);
            return *this;
        }
        auto& wdt(si32 w, si32 h, si32 x, si32 y)
        {
            gc.mtx(w, h);
            st.xy(x, y);
            return *this;
        }
        auto& txt(view utf8, si32 vs)
        {
            auto [w, h, x, y] = utf::matrix::whxy(vs);
            gc.set_direct(utf8, w, h);
            st.xy(x, y);
            return *this;
        }
        auto& txt(view utf8, si32 w, si32 h, si32 x, si32 y)
        {
            gc.set_direct(utf8, w, h);
            st.xy(x, y);
            return *this;
        }
        cell& txt(view utf8)
        {
            if (utf8.empty())
            {
                gc.token = 0;
                st.xy(0);
            }
            else
            {
                auto cluster = utf::cluster(utf8);
                auto [w, h, x, y] = utf::matrix::whxy(cluster.attr.cmatrix);
                gc.set_direct(cluster.text, w, h);
                st.xy(x, y);
            }
            return *this;
        }
        cell& txt2(view utf8, si32 vs)
        {
            auto [w, h, x, y] = utf::matrix::whxy(vs);
            gc.set_direct(utf8, w, h);
            st.xy(x, y);
            return *this;
        }
        auto& txt(char c)        { gc.set(c); st.mosaic(utf::matrix::mosaic<11>);   return *this; } // cell: Set grapheme cluster from char.
        auto& txt(cell const& c) { gc = c.gc;              return *this; } // cell: Set grapheme cluster from cell.
        auto& clr(cell const& c) { uv = c.uv;              return *this; } // cell: Set the foreground and background colors only.
        auto& rst() // cell: Reset view attributes of the cell to zero.
        {
            static auto empty = cell{ whitespace };
            uv = empty.uv;
            st = empty.st;
            gc = empty.gc;
            px = empty.px;
            p2 = empty.p2;
            return *this;
        }

        constexpr auto  rtl() const  { return gc.rtl();      } // cell: Return RTL attribute.
        constexpr auto  mtx() const  { return gc.mtx();      } // cell: Return cluster matrix size (in cells).
        constexpr auto  len() const  { return gc.len();      } // cell: Return grapheme cluster cell storage length (in bytes).
        constexpr auto  tkn() const  { return gc.token;      } // cell: Return grapheme cluster token.
                  bool  jgc() const  { return gc.jgc();      } // cell: Check the grapheme cluster registration (foreign jumbo clusters).
        constexpr ui64   xy() const  { return st.xy();       } // cell: Return matrix fragment metadata.
        template<svga Mode = svga::vtrgb>
        constexpr auto  txt() const  { return gc.get<Mode>(); } // cell: Return grapheme cluster.
        constexpr auto& egc()        { return gc;            } // cell: Get grapheme cluster object.
        constexpr auto& egc() const  { return gc;            } // cell: Get grapheme cluster object.
        constexpr auto  clr() const  { return uv.bg || uv.fg;} // cell: Return true if color set.
        constexpr auto  bga() const  { return uv.bg.chan.a;  } // cell: Return background alpha/transparency.
        constexpr auto  fga() const  { return uv.fg.chan.a;  } // cell: Return foreground alpha/transparency.
        constexpr auto& bgc()        { return uv.bg;         } // cell: Return background color.
        constexpr auto& fgc()        { return uv.fg;         } // cell: Return foreground color.
        constexpr auto& bgc() const  { return uv.bg;         } // cell: Return background color.
        constexpr auto& fgc() const  { return uv.fg;         } // cell: Return foreground color.
        constexpr auto  bld() const  { return st.bld();      } // cell: Return bold attribute.
        constexpr auto  itc() const  { return st.itc();      } // cell: Return italic attribute.
        constexpr auto  und() const  { return st.und();      } // cell: Return underline/Underscore attribute.
        constexpr auto  unc() const  { return st.unc();      } // cell: Return underline color.
        constexpr auto  cur() const  { return st.cur();      } // cell: Return cursor style.
        constexpr auto  img()        { return ui96{ px,p2 }; } // cell: Return attached image.
        constexpr auto  img() const  { return ui96{ px,p2 }; } // cell: Return attached image.
        constexpr auto  size_of_img() const { return (si32)(sizeof(px) + sizeof(p2)); } // cell: Return image metadata size.
        constexpr auto  ovr() const  { return st.ovr();      } // cell: Return overline attribute.
        constexpr auto  inv() const  { return st.inv();      } // cell: Return negative attribute.
        constexpr auto  stk() const  { return st.stk();      } // cell: Return strikethrough attribute.
        constexpr auto  blk() const  { return st.blk();      } // cell: Return blink attribute.
        constexpr auto  hid() const  { return st.hid();      } // cell: Return hidden attribute.
        constexpr auto  dim() const  { return st.dim();      } // cell: Return shadow attribute.
        constexpr auto& stl()        { return st.token;      } // cell: Return style token.
        constexpr auto& stl() const  { return st.token;      } // cell: Return style token.
        constexpr auto link() const  { return id;            } // cell: Return object ID.
        constexpr auto isspc() const { return gc.is_space(); } // cell: Return true if char is whitespace (null included).
        constexpr auto isnul() const { return gc.is_null();  } // cell: Return true if char is null.
        auto issame_visual(cell const& c) const // cell: Is the cell visually identical.
        {
            if (gc == c.gc || (isspc() && c.isspc()))
            {
                if (uv.bg == c.uv.bg)
                {
                    if (xy() == 0 || isspc())
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
        auto set_cursor(si32 style, cell color = {})
        {
            st.cur(style);
            st.cursor_color(color.uv.bg, color.uv.fg);
        }
        auto cursor_color() const
        {
            //todo support for multiple cursor inside the cell
            return st.cursor_color();
        }
        // cell: Return whitespace cell.
        cell spc() const
        {
            return cell{ *this }.txt(whitespace);
        }
        // cell: Return empty cell.
        cell nul() const
        {
            return cell{ *this }.txt('\0');
        }
        // cell: Return dry empty cell.
        cell dry() const
        {
            return cell{ '\0' }.clr(*this);
        }
        // cell: Find the text_only_cells{ what, size } inside the canvas and place the result offset to the &from.
        static auto lookup(auto canvas_begin, auto canvas_end, auto what_begin, auto size, auto& from)
        {
            size--;
            auto head = canvas_begin;
            auto tail = canvas_end - size;
            auto iter = head + from;
            auto base = what_begin;
            auto dest = base;
            auto&test =*base;
            while (iter != tail)
            {
                if (test.same_fragment(*iter++))
                {
                    auto init = iter;
                    auto stop = iter + size;
                    while (init != stop && init->same_fragment(*++dest))
                    {
                        ++init;
                    }
                    if (init == stop)
                    {
                        from = (si32)std::distance(head, iter) - 1;
                        return true;
                    }
                    else dest = base;
                }
            }
            return faux;
        }
        // cell: Find the text_only_cells{ what, size } inside the canvas and place the result offset to the &from.
        static auto find(std::span<cell const> canvas, std::span<cell const> what, auto&& from, feed dir = feed::fwd)
        {
            auto full = (si32)canvas.size();
            auto size = (si32)what.size();
            auto rest = full - from;
            if (dir == feed::fwd)
            {
                if (size && size <= rest)
                if (cell::lookup(canvas.begin(), canvas.end(), what.begin(), size, from))
                {
                    return true;
                }
            }
            else
            {
                if (size && size <= from)
                if (cell::lookup(canvas.rbegin(), canvas.rend(), what.rbegin(), size, rest))
                {
                    from = full - rest - 1; // Restore forward representation.
                    return true;
                }
            }
            return faux;
        }
        // cell: Clear canvas cells from the specified image bits.
        static bool remove_image_bits(auto& canvas, std::vector<ui16>& removed_image_indexes)
        {
            auto hit = faux;
            for (auto& c : canvas)
            {
                if (auto image_index = c.get_image_index())
                {
                    //if (image_index == removed_image_index) //todo ?optimize for single index
                    if (std::find(removed_image_indexes.begin(), removed_image_indexes.end(), image_index) != removed_image_indexes.end())
                    {
                        c.reset_px(); // Drop all image metadata.
                        hit = true;
                    }
                }
            }
            return hit;
        }
        // cell: Unpack indexed colors in canvas using palette.
        static void unpack_indexed_colors(auto& canvas, pals const& palette, cell defclr)
        {
            auto def_fgc = defclr.fgc();
            auto def_bgc = defclr.bgc();
            for (auto& c : canvas)
            {
                c.fgc().unpack_indexed_color(palette, def_fgc);
                c.bgc().unpack_indexed_color(palette, def_bgc);
            }
        }
        // cell: Unpack canvas indexed colors using palette and place it to the dest.
        static void unpack_indexed_colors_to(auto const& canvas, auto& dest, pals const& palette, cell defclr)
        {
            auto def_fgc = defclr.fgc();
            auto def_bgc = defclr.bgc();
            dest.size(canvas.size());
            netxs::oncopy(canvas, dest, [&](auto& src, auto& dst)
            {
                dst = src;
                dst.fgc().unpack_indexed_color(palette, def_fgc);
                dst.bgc().unpack_indexed_color(palette, def_bgc);
            });
        }
        // cell: Convert to raw utf-8 text (ignoring right halves).
        template<bool Select_11_only = true>
        static void to_utf8(auto const& canvas, text& crop)
        {
            crop.reserve(crop.size() + canvas.length());
            netxs::for_each(canvas, [&](cell const& c){ c.scan<Select_11_only>(crop); });
        }
        // cell: Convert to raw utf-8 text (ignoring right halves).
        template<bool Select_11_only = true>
        static auto to_utf8(auto const& canvas)
        {
            auto crop = text{};
            cell::to_utf8(canvas, crop);
            return crop;
        }
        // cell: Detect a word bound.
        template<feed Direction>
        static auto word(std::span<cell const> canvas, si32 start)
        {
            if (start < 0 || (size_t)start >= canvas.size()) return 0;
            static constexpr auto rev = Direction == feed::fwd ? faux : true;
            auto stop_by_zwsp = 0;
            auto is_empty = [&](auto txt)
            {
                auto test = txt.empty()
                         || txt.front() == whitespace
                         ||(txt.front() == '^' && txt.size() == 2); // C0 characters.
                if (test) stop_by_zwsp = 5; // Don't break by zwsp.
                return test;
            };
            auto empty = [&](auto txt)
            {
                return is_empty(txt);
            };
            auto alpha = [&](auto txt)
            {
                //todo revise (https://unicode.org/reports/tr29/#Word_Boundaries)
                auto c = utf::cluster<true>(txt).attr.cdpoint;
                return (c >= '0' && c <= '9')//30-39: '0'-'9'
                     ||(c >= '@' && c <= 'Z')//40-5A: '@','A'-'Z'
                     ||(c >= 'a' && c <= 'z')//5F,61-7A: '_','a'-'z'
                     || c == '_'             //60:    '`'
                     || c == 0xA0            //A0  NO-BREAK SPACE (NBSP)
                     ||(c >= 0xC0                // C0-10FFFF: "À" - ...
                     && c < 0x2000)||(c > 0x206F // General Punctuation
                     && c < 0x2200)||(c > 0x23FF // Mathematical Operators
                     && c < 0x2500)||(c > 0x25FF // Box Drawing
                     && c < 0x2E00)||(c > 0x2E7F // Supplemental Punctuation
                     && c < 0x3000)||(c > 0x303F // CJK Symbols and Punctuation
                     && c != 0x30FB              // U+30FB ( ・ ) KATAKANA MIDDLE DOT
                     && c < 0xFE50)||(c > 0xFE6F // FE50  FE6F Small Form Variants
                     && c < 0xFF00)||(c > 0xFF0F // Halfwidth and Fullwidth Forms
                     && c < 0xFF1A)||(c > 0xFF1F //
                     && c < 0xFF3B)||(c > 0xFF40 //
                     && c < 0xFF5B)|| c > 0xFF65 //
            ;};
            auto is_email = [&](auto txt)
            {
                return !txt.empty() && txt.front() == '@';
            };
            auto email = [&](auto txt)
            {
                return !txt.empty() && (alpha(txt) || txt.front() == '.');
            };
            auto is_digit = [&](auto txt)
            {
                auto c = utf::cluster(txt).attr.cdpoint;
                return (c >= '0'    && c <= '9')
                     ||(c >= 0xFF10 && c <= 0xFF19) // U+FF10 (０) FULLWIDTH DIGIT ZERO - U+FF19 (９) FULLWIDTH DIGIT NINE
                     || c == '.';
            };
            auto digit = [&](auto txt)
            {
                auto c = utf::cluster(txt).attr.cdpoint;
                return c == '.'
                    ||(c >= 'a' && c <= 'f')
                    ||(c >= 'A' && c <= 'F')
                    ||(c >= '0' && c <= '9')
                    ||(c >= 0xFF10 && c <= 0xFF19); // U+FF10 (０) FULLWIDTH DIGIT ZERO - U+FF19 (９) FULLWIDTH DIGIT NINE
            };
            auto func = [&](auto check)
            {
                static constexpr auto right_half = rev ? 1 : 2;
                auto count = 0;
                auto allfx = [&](auto& c)
                {
                    auto txt = c.txt();
                    auto has_zwsp = stop_by_zwsp <= 0 && txt.ends_with("\u200b");
                    if (has_zwsp || (stop_by_zwsp && stop_by_zwsp < 2))
                    {
                        if constexpr (rev) stop_by_zwsp += 2; // Break here.
                        else               stop_by_zwsp++;    // Include current cluster.
                    }
                    auto [w, h, x, y] = c.whxy();
                    auto not_right_half = w != 2 || x != right_half;
                    if (stop_by_zwsp == 2 || (not_right_half && !check(txt))) return true;
                    count++;
                    return faux;
                };
                start += rev ? 1 : 0;
                auto head = start;
                auto tail = rev ? 0 : (si32)canvas.size();
                if constexpr (rev) std::swap(head, tail);
                auto cell_run = canvas.subspan(head, tail - head);
                netxs::for_each<rev>(cell_run, allfx);
                if (count) count--;
                start -= rev ? count + 1 : -count;
            };

            auto test = canvas[start].txt();
            if constexpr (rev)
            {
                if (test.ends_with("\u200b")) stop_by_zwsp -= 2; // Skip zwsp in the first cell.
            }
            is_digit(test) ? func(digit) :
            is_email(test) ? func(email) :
            is_empty(test) ? func(empty) :
                             func(alpha);
            return start;
        }
        // cell: Find proc(c) == true.
        template<feed Direction>
        static auto seek(std::span<cell const> canvas, si32& start, auto proc)
        {
            if (canvas.empty()) return faux;
            static constexpr auto rev = Direction == feed::fwd ? faux : true;
            auto count = 0;
            auto found = faux;
            auto allfx = [&](auto& c)
            {
                if (proc(c))
                {
                    found = true;
                    return true;
                }
                count++;
                return faux;
            };

            start += rev ? 1 : 0;
            auto head = start;
            auto tail = rev ? 0 : (si32)canvas.size();
            if constexpr (rev) std::swap(head, tail);
            auto cell_run = canvas.subspan(head, tail - head);
            netxs::for_each<rev>(cell_run, allfx);

            if (count) count--;
            start -= rev ? count + 1 : -count;
            return found;
        }

        friend auto& operator << (std::ostream& s, cell const& c)
        {
            return s << "\n\tfgc " << c.fgc()
                     << "\n\tbgc " << c.bgc()
                     << "\n\ttxt " <<(c.isspc() ? text{ "whitespace" } : utf::debase<faux, faux>(c.txt()))
                     << "\n\tmtx " <<(c.mtx())
                     << "\n\tstk " <<(c.stk() ? "true" : "faux")
                     << "\n\titc " <<(c.itc() ? "true" : "faux")
                     << "\n\tovr " <<(c.ovr() ? "true" : "faux")
                     << "\n\tblk " <<(c.blk() ? "true" : "faux")
                     << "\n\tinv " <<(c.inv() ? "true" : "faux")
                     << "\n\tbld " <<(c.bld() ? "true" : "faux")
                     << "\n\tund " <<(c.und() == unln::none   ? "none"
                                    : c.und() == unln::line   ? "line"
                                    : c.und() == unln::biline ? "biline"
                                    : c.und() == unln::wavy   ? "wavy"
                                    : c.und() == unln::dotted ? "dotted"
                                    : c.und() == unln::dashed ? "dashed"
                                                              : "unknown");
        }

        class shaders
        {
        public:
            template<class Func>
            struct brush_t
            {
                template<class Cell>
                struct func
                {
                    Cell brush;
                    static constexpr auto f = Func{};
                    constexpr func(Cell const& c)
                        : brush{ c }
                    { }
                    template<class D>
                    inline void operator () (D& dst) const
                    {
                        f(dst, brush);
                    }
                };
            };

        private:
            struct contrast_t : public brush_t<contrast_t>
            {
                static constexpr auto threshold = argb{ tint::whitedk }.luma() - 0xF;
                template<class C>
                constexpr inline auto operator () (C brush) const
                {
                    return func<C>(brush);
                }
                static inline auto invert(argb color)
                {
                    return color.luma() >= threshold ? 0xFF000000
                                                     : 0xFFffffff;
                }
                template<class D, class S>
                inline void operator () (D& dst, S& src) const
                {
                    if (src.isnul()) return;
                    auto& fgc = src.fgc();
                    if (fgc.chan.a == 0x00)
                    {
                        auto& bgc = dst.bgc();
                        if (bgc.chan.a < 2) dst.fgc(0xFFffffff);
                        else                dst.fgc(invert(bgc));
                    }
                    dst.fusefull_keep_image(src);
                }
            };
            struct lite_t : public brush_t<lite_t>
            {
                template<class C> constexpr inline auto operator () (C brush) const { return func<C>(brush); }
                template<class D, class S>  inline void operator () (D& dst, S& src) const { dst.lite(src); }
            };
            struct flat_t : public brush_t<flat_t>
            {
                template<class C> constexpr inline auto operator () (C brush) const { return func<C>(brush); }
                template<class D, class S>  inline void operator () (D& dst, S& src) const { dst.set(src); }
            };
            struct mix_t : public brush_t<mix_t>
            {
                template<class C> constexpr inline auto operator () (C brush) const { return func<C>(brush); }
                template<class D, class S>  inline void operator () (D& dst, S& src) const { dst.mix(src); }
            };
            struct blend_t : public brush_t<blend_t>
            {
                template<class C> constexpr inline auto operator () (C brush) const { return func<C>(brush); }
                template<class D, class S>  inline void operator () (D& dst, S& src) const { dst.blend(src); }
            };
            struct blendpma_t : public brush_t<blendpma_t>
            {
                template<class C> constexpr inline auto operator () (C brush) const { return func<C>(brush); }
                template<class D, class S>  inline void operator () (D& dst, S& src) const { dst.blend_pma(src); }
            };
            struct alpha_t : public brush_t<alpha_t>
            {
                template<class C> constexpr inline auto operator () (C brush) const { return func<C>(brush); }
                template<class D, class S>  inline void operator () (D& dst, S& src) const { dst.alpha_sum(src); }
            };
            struct alphamix_t : public brush_t<alphamix_t>
            {
                template<class C> constexpr inline auto operator () (C brush) const { return func<C>(brush); }
                template<class D, class S>  inline void operator () (D& dst, S& src) const { argb::alpha_mix(src, dst); }
            };
            struct full_t : public brush_t<full_t>
            {
                template<class C> constexpr inline auto operator () (C brush) const { return func<C>(brush); }
                template<class D, class S>  inline void operator () (D& dst, S& src) const { dst = src; }
            };
            struct wipe_t
            {
                template<class D>  inline void operator () (D& dst) const { dst = {}; }
            };
            struct skipnulls_t : public brush_t<skipnulls_t>
            {
                template<class C> constexpr inline auto operator () (C brush) const { return func<C>(brush); }
                template<class D, class S>  inline void operator () (D& dst, S& src) const { dst.skipnulls(src); }
            };
            struct fuse_t : public brush_t<fuse_t>
            {
                template<class C> constexpr inline auto operator () (C brush) const { return func<C>(brush); }
                template<class D, class S>  inline void operator () (D& dst, S& src) const { dst.fuse(src); }
            };
            struct fuseid_t : public brush_t<fuseid_t>
            {
                template<class C> constexpr inline auto operator () (C brush) const { return func<C>(brush); }
                template<class D, class S>  inline void operator () (D& dst, S& src) const { dst.fuseid(src); }
            };
            struct fusefull_t : public brush_t<fusefull_t>
            {
                template<class C> constexpr inline auto operator () (C brush) const { return func<C>(brush); }
                template<class D, class S>  inline void operator () (D& dst, S& src) const { dst.fusefull(src); }
            };
            struct overlay_t : public brush_t<overlay_t>
            {
                template<class C> constexpr inline auto operator () (C brush) const { return func<C>(brush); }
                template<class D, class S>  inline void operator () (D& dst, S& src) const { dst.overlay(src); }
            };
            struct text_t : public brush_t<text_t>
            {
                template<class C> constexpr inline auto operator () (C brush) const { return func<C>(brush); }
                template<class D, class S>  inline void operator () (D& dst, S& src) const { dst.txt(src); }
            };
            struct meta_t : public brush_t<meta_t>
            {
                template<class C> constexpr inline auto operator () (C brush) const { return func<C>(brush); }
                template<class D, class S>  inline void operator () (D& dst, S& src) const { dst.meta(src); }
            };
            struct xlight_t
            {
                si32 factor; // Uninitialized.
                template<class T>
                inline auto operator [] (T param) const
                {
                    return xlight_t{ param };
                }
                template<class D> inline void operator () (D& dst) const { dst.xlight(factor); }
                template<class D, class S> inline void operator () (D& dst, S& src) const { dst.fuse(src); operator()(dst); }
            };
            struct underlight_t
            {
                si32 factor; // Uninitialized.
                template<class T>
                inline auto operator [] (T param) const
                {
                    return underlight_t{ param };
                }
                template<class D> inline void operator () (D& dst) const { dst.underlight(factor); }
                template<class D, class S> inline void operator () (D& dst, S& src) const { dst.fuse(src); operator()(dst); }
            };
            struct invert_t
            {
                template<class D> inline void operator () (D& dst) const { dst.invert(); }
                template<class D, class S> inline void operator () (D& dst, S& src) const { dst.fuse(src); operator()(dst); }
            };
            struct reverse_t
            {
                template<class D> inline void operator () (D& dst) const { dst.reverse(); }
                template<class D, class S> inline void operator () (D& dst, S& src) const { dst.fuse(src); operator()(dst); }
            };
            struct invbit_t
            {
                template<class D> inline void operator () (D& dst) const { dst.invbit(); }
            };
            struct disabled_t
            {
                template<class T>
                inline auto operator [] (T /*param*/) const
                {
                    return disabled_t{};
                }
                template<class D> inline void operator () (D& dst) const { dst.disabled(); }
            };
            struct transparent_t : public brush_t<transparent_t>
            {
                si32 alpha;
                constexpr transparent_t(si32 alpha)
                    : alpha{ alpha }
                { }
                template<class C> constexpr inline auto operator () (C brush) const { return func<C>(brush); }
                template<class D, class S>  inline void operator () (D& dst, S& src) const { dst.mixfull(src, alpha); }
            };
            struct xlucent_t
            {
                si32 alpha;
                constexpr xlucent_t(si32 alpha)
                    : alpha{ alpha }
                { }
                template<class D, class S>  inline void operator () (D& dst, S& src) const { dst.fuse(src); dst.bga(alpha); }
                template<class D>           inline void operator () (D& dst)         const { dst.bga(alpha); }
            };
            struct shadow_t
            {
                si32 shadow_index;
                constexpr shadow_t(si32 shadow_index)
                    : shadow_index{ std::clamp(shadow_index, 0, 255) }
                { }
                template<class D, class S>  inline void operator () (D& dst, S& src) const { dst.fuse(src); dst.st.fuse_dim(shadow_index); }
                template<class D>           inline void operator () (D& dst)         const { dst.st.fuse_dim(shadow_index); }
            };
            struct color_t
            {
                clrs colors;
                bool invert;
                si32 factor;
                template<class T>
                constexpr color_t(T colors, bool invert, si32 factor = 1)
                    : colors{ colors },
                      invert{ invert },
                      factor{ factor }
                { }
                constexpr color_t(cell const& brush, si32 factor = 1)
                    : colors{ brush.uv },
                      invert{ brush.inv() },
                      factor{ factor }
                { }
                template<class T>
                inline auto operator [] (T param) const
                {
                    return color_t{ colors, invert, param };
                }
                template<class D>
                inline void operator () (D& dst) const
                {
                    auto i = dst.inv() ^ invert;;
                    auto b = i ? dst.fgc() : dst.bgc();
                    dst.uv = colors;
                    dst.inv(i);
                    //if (b == colors.bg) dst.xlight();
                    if (b == colors.bg) dst.uv.bg.shadow();
                }
                template<class D, class S>
                inline void operator () (D& dst, S& src) const
                {
                    auto i = factor;
                    while(i-- > 0) dst.fuse(src);
                    operator()(dst);
                }
            };
            struct mimic_t
            {
                clrs color;
                body style;
                constexpr mimic_t(cell const& brush)
                    : color{ brush.uv },
                      style{ brush.st }
                { }
                template<class D>
                inline void operator () (D& dst) const
                {
                    dst.uv = color;
                    dst.st.meta(style);
                }
                template<class D, class S>
                inline void operator () (D& dst, S& src) const
                {
                    operator()(dst);
                    dst.fuse(src);
                }
            };
            struct onlyid_t
            {
                id_t id;
                constexpr onlyid_t(id_t id)
                    : id{ id }
                { }
                template<class D>
                inline void operator () (D& dst) const
                {
                    dst.link(id);
                }
                template<class D, class S>
                inline void operator () (D& dst, S& src) const
                {
                    dst.fuse(src, id);
                }
            };
            struct image_t : public brush_t<image_t>
            {
                template<class C> constexpr inline auto operator () (C brush) const { return func<C>(brush); }
                template<class D, class S>  inline void operator () (D& dst, S& src) const { dst.px = src.px; dst.p2 = src.p2; }
            };

        public:
            static constexpr auto       color(auto brush) { return       color_t{ brush }; }
            static constexpr auto       mimic(auto brush) { return       mimic_t{ brush }; }
            static constexpr auto transparent(si32     a) { return transparent_t{ a     }; }
            static constexpr auto     xlucent(si32     a) { return     xlucent_t{ a     }; }
            static constexpr auto      onlyid(id_t newid) { return      onlyid_t{ newid }; }
            static constexpr auto      shadow(si32 index) { return      shadow_t{ index }; }
            static constexpr auto   contrast =   contrast_t{};
            static constexpr auto   fusefull =   fusefull_t{};
            static constexpr auto    overlay =    overlay_t{};
            static constexpr auto     fuseid =     fuseid_t{};
            static constexpr auto        mix =        mix_t{};
            static constexpr auto   blendpma =   blendpma_t{};
            static constexpr auto      blend =      blend_t{};
            static constexpr auto      alpha =      alpha_t{};
            static constexpr auto   alphamix =   alphamix_t{};
            static constexpr auto       lite =       lite_t{};
            static constexpr auto       fuse =       fuse_t{};
            static constexpr auto       flat =       flat_t{};
            static constexpr auto       full =       full_t{};
            static constexpr auto       wipe =       wipe_t{};
            static constexpr auto  skipnulls =  skipnulls_t{};
            static constexpr auto       text =       text_t{};
            static constexpr auto       meta =       meta_t{};
            static constexpr auto     invert =     invert_t{};
            static constexpr auto    reverse =    reverse_t{};
            static constexpr auto     invbit =     invbit_t{};
            static constexpr auto   disabled =   disabled_t{};
            static constexpr auto     xlight =     xlight_t{ 1 };
            static constexpr auto underlight = underlight_t{ 1 };
            static constexpr auto      image =      image_t{};
        };

        auto draw_cursor()
        {
            auto [cursor_bgc, cursor_fgc] = cursor_color();
            switch (st.cur())
            {
                case text_cursor::I_bar: // Some terminals do not support colored underlining.
                case text_cursor::block:
                    if (cursor_bgc.chan.a == 0)
                    {
                        auto b = inv() ? fgc() : bgc();
                        auto f = cursor_fgc.chan.a ? cursor_fgc : b;
                        inv(faux).fgc(f).bgc(cell::shaders::contrast.invert(b));
                    }
                    else
                    {
                        auto b = cursor_bgc;
                        auto f = cursor_fgc.chan.a && cursor_fgc != cursor_bgc ? cursor_fgc : cell::shaders::contrast.invert(b);
                        inv(faux).fgc(f).bgc(b);
                    }
                    break;
                case text_cursor::underline:
                    if (cursor_bgc.chan.a == 0)
                    {
                        if (und() == unln::line)
                        {
                            und(unln::none);
                        }
                        else
                        {
                            auto b = inv() ? fgc() : bgc();
                            auto u = argb{ cell::shaders::contrast.invert(b) };
                            und(unln::line).unc(u);
                        }
                    }
                    else
                    {
                        auto u = cursor_bgc.to_256cube();
                        if (u == unc() && und() == unln::line) und(unln::none);
                        else                                   und(unln::line).unc(u);
                    }
                    break;
            }
        }
    };

    namespace mime
    {
        static constexpr auto _counter = __COUNTER__ + 1;
        static constexpr auto disabled = __COUNTER__ - _counter;
        static constexpr auto textonly = __COUNTER__ - _counter;
        static constexpr auto ansitext = __COUNTER__ - _counter;
        static constexpr auto richtext = __COUNTER__ - _counter;
        static constexpr auto htmltext = __COUNTER__ - _counter;
        static constexpr auto safetext = __COUNTER__ - _counter; // mime: Sensitive textonly data.
        static constexpr auto count    = __COUNTER__ - _counter;

        namespace tag
        {
            static constexpr auto text = "text/plain"sv;
            static constexpr auto ansi = "text/xterm"sv;
            static constexpr auto html = "text/html"sv;
            static constexpr auto rich = "text/rtf"sv;
            static constexpr auto safe = "text/protected"sv;
        }

        auto meta(twod size, si32 form) // mime: Return clipdata's meta data.
        {
            return utf::concat(form == htmltext ? tag::html
                             : form == richtext ? tag::rich
                             : form == ansitext ? tag::ansi
                             : form == safetext ? tag::safe
                                                : tag::text, "/", size.x, "/", size.y);
        }
    }

    namespace misc //todo classify
    {
        template<class T>
        struct shadow
        {
            T  bitmap{};
            bool sync{};
            bool hide{};
            twod over{};
            twod step{};

            shadow()              = default;
            shadow(shadow&&)      = default;
            shadow(shadow const&) = default;
            shadow(fp32 bias, fp32 alfa, si32 size, twod offset, twod ratio, auto fuse)
            {
                generate(bias, alfa, size, offset, ratio, fuse);
            }
            // shadow: Generate shadow sprite.
            void generate(fp32 bias, fp32 alfa, si32 size, twod offset, twod ratio, auto fuse)
            {
                //bias    += _k0 * 0.1f;
                //opacity += _k1 * 1.f;
                //size    += _k2;
                //offset  +=  ratio * _k3;
                sync = true;
                alfa = std::clamp(alfa, 0.f, 255.f);
                size = std::abs(size);
                over = ratio * (size * 2);
                step = over / 2 - offset;
                auto spline = netxs::spline01{ bias };
                auto sz = ratio * (size * 2 + 1);
                if (sz.x <= 1 || sz.y <= 1) return;
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
                        auto xy = sy * sx; // argb::gamma(sy * sx);
                        auto a = (byte)std::round(alfa * xy);
                        fuse(*it++, a);
                    }
                }
            }
            // shadow: Render a rectangular shadow for the window rectangle.
            auto render(auto&& canvas, auto clip, auto window, auto fx)
            {
                auto dst = rect{ window.coor - over / 2, window.size + over };
                if (!dst.trim(clip)) return;
                auto basis = step - window.coor;
                clip.coor += basis;
                canvas.step(basis);
                dst.coor = dot_00;
                auto src = bitmap.area();
                auto cut = std::min(dot_00, (dst.size - src.size * 2 - dot_11) / 2);
                auto off = dent{ 0, cut.x, 0, cut.y };
                src += off;
                auto mid = rect{ src.size, std::max(dot_00, dst.size - src.size * 2) };
                auto top = rect{ twod{ src.size.x, 0 }, { mid.size.x, src.size.y }};
                auto lft = rect{ twod{ 0, src.size.y }, { src.size.x, mid.size.y }};
                if (auto m = mid.trim(clip))
                {
                    auto base_shadow = bitmap[src.size - dot_11];
                    netxs::onrect(canvas, m, fx(base_shadow));
                }
                if (top)
                {
                    auto pen = rect{{ src.size.x - 1, 0 }, { 1, src.size.y }};
                    netxs::xform_scale(canvas, top, clip, bitmap, pen, fx);
                    top.coor.y += mid.size.y + top.size.y;
                    netxs::xform_scale(canvas, top, clip, bitmap, pen.rotate({ 1, -1 }), fx);
                }
                if (lft)
                {
                    auto pen = rect{{ 0, src.size.y - 1 }, { src.size.x, 1 }};
                    netxs::xform_scale(canvas, lft, clip, bitmap, pen, fx);
                    lft.coor.x += mid.size.x + lft.size.x;
                    netxs::xform_scale(canvas, lft, clip, bitmap, pen.rotate({ -1, 1 }), fx);
                }
                auto dir = dot_11;
                            netxs::xform_mirror(canvas, clip, dst.rotate(dir).coor, bitmap, src.rotate(dir), fx);
                dir = -dir; netxs::xform_mirror(canvas, clip, dst.rotate(dir).coor, bitmap, src.rotate(dir), fx);
                dir.x += 2; netxs::xform_mirror(canvas, clip, dst.rotate(dir).coor, bitmap, src.rotate(dir), fx);
                dir = -dir; netxs::xform_mirror(canvas, clip, dst.rotate(dir).coor, bitmap, src.rotate(dir), fx);
                canvas.step(-basis);
            }
        };

        struct szgrips
        {
            twod origin; // szgrips: Grab's initial coord info.
            twod dtcoor; // szgrips: The form coor parameter change factor while resizing.
            twod sector; // szgrips: Active quadrant, x,y = {-1|+1}. Border widths.
            rect hzgrip; // szgrips: Horizontal grip.
            rect vtgrip; // szgrips: Vertical grip.
            twod widths; // szgrips: Grip's widths.
            bool inside; // szgrips: Is active.
            bool seized; // szgrips: Is seized.
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
            auto corner(twod length) const
            {
                return dtcoor.less(dot_11, length, dot_00);
            }
            auto quantize(twod curpos, twod basis, twod cell_size) const
            {
                curpos -= basis;
                curpos -= (curpos + cell_size/*to avoid negative values*/) % cell_size;
                return curpos;
            }
            auto grab(rect window, twod curpos, dent outer, twod cell_size = dot_11)
            {
                if (inside)
                {
                    auto outer_rect = window + outer;
                    curpos = quantize(curpos, outer_rect.coor, cell_size);
                    origin = curpos - corner(outer_rect.size);
                    seized = true;
                }
                return seized;
            }
            auto leave()
            {
                auto inside_old = std::exchange(inside, faux);
                auto changed = inside_old != inside;
                return changed;
            }
            auto calc(rect window, twod curpos, dent outer, dent inner, twod cell_size = dot_11)
            {
                auto border = outer - inner;
                auto inside_old = inside;
                auto hzgrip_old = hzgrip;
                auto vtgrip_old = vtgrip;
                auto inner_rect = window + inner;
                auto outer_rect = window + outer;
                inside = !inner_rect.hittest(curpos) && outer_rect.hittest(curpos);
                auto& length = outer_rect.size;
                curpos = quantize(curpos, outer_rect.coor, cell_size);
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
                auto s = sector * std::max(dot_00, a - b + center + sector.less(dot_00, dot_00, cell_size));

                hzgrip.coor.x = widths.x;     // |----|
                hzgrip.coor.y = 0;            // |    |
                hzgrip.size.y = widths.y;     // |----|
                hzgrip.size.x = s.x - s.x % cell_size.x;

                vtgrip.coor = dot_00;
                vtgrip.size = widths;
                vtgrip.size.y += s.y - s.y % cell_size.y;
                auto changed = inside_old != inside || (inside && (hzgrip_old != hzgrip || vtgrip_old != vtgrip));
                return changed;
            }
            auto drag(rect window, twod curpos, dent outer, bool zoom, twod cell_size = dot_11) const
            {
                auto outer_rect = window + outer;
                curpos = quantize(curpos, outer_rect.coor, cell_size);
                auto delta = (corner(outer_rect.size) + origin - curpos) * sector;
                if (zoom) delta *= 2;
                auto preview_step = zoom ? -delta / 2 : -delta * dtcoor;
                auto preview_area = rect{ window.coor + preview_step, window.size + delta };
                return std::pair{ preview_area, delta };
            }
            auto move(twod dxdy, bool zoom) const
            {
                auto step = zoom ? -dxdy / 2 : -dxdy * dtcoor;
                return step;
            }
            void drop()
            {
                seized = faux;
            }
            auto layout(rect area) const
            {
                auto vertex = corner(area.size);
                auto side_x = hzgrip.shift(vertex).normalize_itself().shift_itself(area.coor).trim(area);
                auto side_y = vtgrip.shift(vertex).normalize_itself().shift_itself(area.coor).trim(area);
                return std::pair{ side_x, side_y };
            }
            auto draw(auto& canvas, rect area, auto fx) const
            {
                auto [side_x, side_y] = layout(area);
                netxs::onrect(canvas, side_x, fx);
                netxs::onrect(canvas, side_y, fx);
            }
        };

        void fill(auto&& canvas, auto block, auto fx) // gfx: Fill block.
        {
            block.normalize_itself();
            netxs::onrect(canvas, block, fx);
        }
        void fill(auto&& canvas, auto fx) // gfx: Fill canvas area.
        {
            netxs::onrect(canvas, canvas.area(), fx);
        }
        void cage(auto&& canvas, rect area, dent border, auto fx) // core: Draw cage inside the specified area.
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

    using vrgb = netxs::raw_vector<irgb<si32>>;

    // canvas: Core grid.
    class core
    {
        core(twod coor, twod size, cell const& brush) // Prefill canvas using brush.
            : region{ coor, size },
              client{ dot_00, size },
              canvas(size.x * size.y, brush),
              marker{ brush }
        { }
        core(twod coor, twod size) // Prefill canvas using zero.
            : region{ coor, size },
              client{ dot_00, size },
              canvas(size.x * size.y)
        { }

    public:
        using body = std::vector<cell>;

    protected:
        rect region; // core: Physical square of canvas relative to current basis (top-left corner of the current rendering object, see face::change_basis).
        rect client; // core: Active canvas area relative to current basis.
        body canvas; // core: Cell data.
        cell marker; // core: Current brush.
        si32 digest = 0; // core: Resize stamp.
        si32 hasimg{}; // core: Canvas contains Sixel cells.

    public:
        core()                         = default;
        core(core&&)                   = default;
        core(core const&)              = default;
        core& operator = (core&&)      = default;
        core& operator = (core const&) = default;
        core(std::span<cell const> cells, twod size)
            : region{ dot_00, size },
              client{ dot_00, size },
              canvas( cells.begin(), cells.end() )
        {
            assert(size.x * size.y == std::distance(cells.begin(), cells.end()));
        }
        core(cell const& fill, si32 length)
            : region{ dot_00, { length, 1 } },
              client{ dot_00, { length, 1 } },
              canvas( length, fill )
        { }
        core(cell const& fill)
            : region{ dot_00, dot_01 },
              client{ dot_00, dot_01 },
              marker{ fill }
        { }

        auto get_image_sixel()       { return hasimg; }
        auto set_image_sixel(si32 n) { hasimg = n; }
        auto  or_image_sixel(si32 n) { hasimg |= n; }
        template<class P>
        auto same(core const& c, P compare) const // core: Compare content.
        {
            if (region.size != c.region.size) return faux;
            auto dest = c.canvas.begin();
            auto head =   canvas.begin();
            auto tail =   canvas.end();
            while (head != tail)
            {
                if (!compare(*head++, *dest++)) return faux;
            }
            return true;
        }
        auto volume() const // core: Return cell count.
        {
            return canvas.size();
        }
        auto operator == (core const& c) const { return same(c, [](auto const& a, auto const& b){ return a == b;        }); }
        auto  same       (core const& c) const { return same(c, [](auto const& a, auto const& b){ return a.same_txt(b); }); }
        constexpr auto& size() const           { return region.size;                                                        }
        auto& coor() const                     { return region.coor;                                                        }
        auto& area() const                     { return region;                                                             }
        auto  area(rect new_area)              { size(new_area.size); move(new_area.coor); clip(new_area);                  }
        auto  area(rect new_area, cell c)      { size(new_area.size, c); move(new_area.coor); clip(new_area);               }
        auto& pick()                           { return canvas;                                                             }
        auto  begin()                          { return canvas.begin();                                                     }
        auto  end()                            { return canvas.end();                                                       }
        auto  begin() const                    { return canvas.begin();                                                     }
        auto  end() const                      { return canvas.end();                                                       }
        auto  begin(twod coord)                { return canvas.begin() + coord.x + coord.y * region.size.x;                 }
        auto  begin(twod coord) const          { return canvas.begin() + coord.x + coord.y * region.size.x;                 }
        auto  begin(size_t offset)             { return canvas.begin() + offset;                                            }
        auto& operator [] (twod coord)         { return*(begin(coord));                                                     }
        auto& mark()                           { return marker;                                                             } // core: Return a reference to the default cell value.
        auto& mark() const                     { return marker;                                                             } // core: Return a reference to the default cell value.
        auto& mark(cell const& new_marker)     { marker = new_marker; return marker;                                        } // core: Set the default cell value.
        void  move(twod new_coor)              { region.coor = new_coor;                                                    } // core: Change the location of the face.
        void  step(twod delta)                 { region.coor += delta;                                                      } // core: Shift location of the face by delta.
        auto& back()                           { return canvas.back();                                                      } // core: Return last cell.
        auto  link()                           { return marker.link();                                                      } // core: Return default object ID.
        void  link(id_t id)                    { marker.link(id);                                                           } // core: Set the default object ID.
        auto  link(twod coord) const           { return region.size.inside(coord) ? (*(begin(coord))).link() : 0;           } // core: Return ID of the object in cell at the specified coordinates.
        auto  clip() const                     { return client;                                                             }
        void  clip(rect new_client)            { client = new_client;                                                       }
        auto  hash() const                     { return digest;                                                             } // core: Return the digest value that associatated with the current canvas size.
        auto  hash(si32 d)                     { return digest != d ? ((void)(digest = d), true) : faux;                    } // core: Check and the digest value that associatated with the current canvas size.
        template<bool Forced = faux>
        void size(twod new_size, cell const& c) // core: Resize canvas.
        {
            auto changed = region.size(std::max(dot_00, new_size));
            if (changed || Forced)
            {
                client.size = region.size;
                digest++;
                canvas.assign(region.size.x * region.size.y, c);
            }
        }
        void size(twod new_size) // core: Resize canvas.
        {
            size(new_size, marker);
        }
        void size(si32 new_size_x, cell const& c) // core: Resize canvas.
        {
            region.size.x = new_size_x;
            region.size.y = 1;
            client.size = region.size;
            canvas.assign(new_size_x, c);
            digest++;
        }
        void crop(si32 new_size_x, cell const& c = {}) // core: Resize preserving textline.
        {
            region.size.x = new_size_x;
            region.size.y = 1;
            client.size = region.size;
            canvas.resize(new_size_x, c);
            digest++;
        }
        auto crop(si32 at, si32 length) const // core: Return 1D fragment.
        {
            auto fragment = core{ std::span{ canvas.begin() + at, canvas.begin() + at + length }, twod{ length, 1 } };
            fragment.marker = marker;
            return fragment;
        }
        void push(cell const& c) // core: Push cell back.
        {
            crop(region.size.x + 1, c);
        }
        template<bool BottomAnchored = faux>
        void crop(twod new_size, cell const& c) // core: Resize preserving bitmap.
        {
            auto block = core{ region.coor, new_size, c };
            if constexpr (BottomAnchored) block.step({ 0, region.size.y - new_size.y });
            netxs::onbody(block, *this, cell::shaders::full);
            client.size = new_size;
            swap(block);
            digest++;
        }
        template<bool BottomAnchored = faux>
        void crop(twod new_size) // core: Resize preserving bitmap.
        {
            crop<BottomAnchored>(new_size, marker);
        }
        void kill() // core: Collapse canvas to zero size (see para).
        {
            region.size.x = 0;
            client.size.x = 0;
            canvas.resize(0);
            digest++;
        }
        void wipe(cell const& c) { std::fill(canvas.begin(), canvas.end(), c); } // core: Fill canvas with specified marker.
        void wipe()              { wipe(marker); } // core: Fill canvas with default color.
        void wipe(id_t id)                         // core: Fill canvas with specified id.
        {
            auto my_id = marker.link();
            marker.link(id);
            wipe(marker);
            marker.link(my_id);
        }
        auto each(auto proc) // core: Exec a proc for each cell.
        {
            return netxs::for_each(canvas, proc);
        }
        void each(rect region, auto fx) // core: Exec a proc for each cell of the specified region.
        {
            netxs::onrect(*this, region, fx);
        }
        auto copy(body& target) const // core: Copy only body of the canvas to the specified body bitmap.
        {
            target = canvas;
            return region.size;
        }
        template<class Face>
        void copy(Face& dest) const // core: Copy only body of the canvas to the specified core.
        {
            dest.size(region.size);
            dest.canvas = canvas;
        }
        void copy(core& target, auto fx) const // core: Copy the canvas to the specified target bitmap. The target bitmap must be the same size.
        {
            netxs::oncopy(target, *this, fx);
            //todo should we copy all members?
            //target.marker = marker;
            //flow::cursor
        }
        void fill(core const& block, auto fx) // core: Fill canvas by the specified block using its coordinates.
        {
            netxs::onbody(*this, block, fx);
        }
        void zoom(core const& block, auto fx) // core: Fill canvas by the stretched block.
        {
            netxs::zoomin(*this, block, fx);
        }
        void plot(core const& block, auto fx) // core: Fill the client area by the specified block with coordinates inside the canvas area.
        {
            //todo use block.client instead of block.region
            auto joint = rect{ client.coor - region.coor, client.size };
            if (joint.trimby(block.region))
            {
                auto place = joint.coor - block.region.coor;
                netxs::inbody<faux>(*this, block, joint, place, fx);
            }
        }
        auto& peek(twod p) // core: Take the cell at the specified coor.
        {
            p -= region.coor;
            auto& c = *(canvas.begin() + p.x + p.y * region.size.x);
            return c;
        }
        void fill(rect block, auto fx) // core: Process the specified region by the specified proc.
        {
            block.normalize_itself();
            netxs::onrect(*this, block, fx);
        }
        void fill(auto fx) // core: Fill the client area using lambda.
        {
            fill(clip(), fx);
        }
        void fill(cell const& c) // core: Fill the client area using brush.
        {
            fill(clip(), cell::shaders::full(c));
        }
        void grad(argb c1, argb c2) // core: Fill the specified region with the linear gradient.
        {
            auto mx = (fp32)region.size.x;
            auto my = (fp32)region.size.y;
            auto len = std::max(1.f, std::sqrt(mx * mx + my * my * 4));

            auto dr = (c2.chan.r - c1.chan.r) / len;
            auto dg = (c2.chan.g - c1.chan.g) / len;
            auto db = (c2.chan.b - c1.chan.b) / len;
            auto da = (c2.chan.a - c1.chan.a) / len;

            auto x = si32{ 0 };
            auto y = si32{ 0 };
            auto z = si32{ 0 };
            auto allfx = [&](cell& c)
            {
                auto dt = std::sqrt(x * x + z);
                auto& chan = c.bgc().chan;
                chan.r = (byte)((fp32)c1.chan.r + dr * dt);
                chan.g = (byte)((fp32)c1.chan.g + dg * dt);
                chan.b = (byte)((fp32)c1.chan.b + db * dt);
                chan.a = (byte)((fp32)c1.chan.a + da * dt);
                ++x;
            };
            auto eolfx = [&]
            {
                x = 0;
                ++y;
                z = y * y * 4;
            };
            netxs::onrect(*this, client, allfx, eolfx);
        }
        void swap(core& other) // core: Unconditionally swap canvases.
        {
            canvas.swap(other.canvas);
            std::swap(region, other.region);
        }
        auto swap(body& target) // core: Move the canvas to the specified array and return the current layout size.
        {
            if (auto size = canvas.size())
            {
                if (target.size() == size) canvas.swap(target);
                else                       target = canvas;
            }
            return region.size;
        }
        template<feed Direction>
        auto word(twod coord) // core: Detect a word bound.
        {
            if (region)
            {
                coord = std::clamp(coord, dot_00, region.size - dot_11);
                auto start = coord.x + coord.y * region.size.x;
                auto offset = cell::word<Direction>(canvas, start);
                coord.x = (offset - 1) % region.size.x + 1;
                coord.y = std::max(0, (offset - 1) / region.size.x);
            }
            else
            {
                coord = {};
            }
            return coord;
        }
        template<feed Direction>
        auto word(si32 offset) // core: Detect a word bound.
        {
            return cell::word<Direction>(canvas, offset);
        }
        void cage(rect area, dent border, auto fx) // core: Draw the cage around specified area.
        {
            netxs::misc::cage(*this, area, border, fx);
        }
        void cage(rect area, twod border_width, auto fx) // core: Draw the cage around specified area.
        {
            cage(area, dent{ border_width.x, border_width.x, border_width.y, border_width.y }, fx);
        }
        template<bool RtoL = faux, class Text, class P = noop>
        void text(twod coord, Text const& block, P print = {}) // core: Put the specified text substring to the specified coordinates on the canvas.
        {
            block.template output<RtoL>(*this, coord, print);
        }
        template<bool RtoL = faux, class P = noop>
        void text(twod coord, std::span<cell const> subline, P print = {}) // core: Put the specified text substring to the specified coordinates on the canvas.
        {
            auto place = rect{ coord, { (si32)subline.size(), 1 }};
            auto joint = clip().trim(place);
            if (joint)
            {
                if constexpr (RtoL)
                {
                    place.coor.x -= joint.coor.x - place.size.x + joint.size.x;
                    place.coor.y  = joint.coor.y - place.coor.y;
                }
                else
                {
                    place.coor = joint.coor - place.coor;
                }
                auto skip_in_subline = place.coor.x;
                auto size_of_subline = joint.size.x;
                auto src_crop = subline.subspan(skip_in_subline, (size_t)size_of_subline);
                joint.coor -= coor();
                auto skip_in_canvas = joint.coor.x + joint.coor.y * size().x;
                auto size_of_canvas = joint.size.x;
                auto dst_crop = std::span{ canvas.begin() + skip_in_canvas, (size_t)size_of_canvas };
                if constexpr (std::is_same_v<P, noop>) netxs::oncopy<RtoL>(dst_crop, src_crop, cell::shaders::fusefull);
                else                                   netxs::oncopy<RtoL>(dst_crop, src_crop, print);
            }
        }
        template<class Si32>
        auto find(auto const& what, Si32&& from, feed dir = feed::fwd) const // core: Find the subspan and place its offset in &from.
        {
            return cell::find(canvas, what, from, dir);
        }
        auto toxy(si32 offset) const // core: Convert offset to coor.
        {
            assert(canvas.size() <= netxs::si32max);
            auto maxs = (si32)canvas.size();
            if (!maxs) return dot_00;
            offset = std::clamp(offset, 0, maxs - 1);
            auto sx = std::max(1, region.size.x);
            return twod{ offset % sx, offset / sx };
        }
        auto subline(si32 from, si32 upto) const // core: Get stripe.
        {
            if (from > upto) std::swap(from, upto);
            assert(canvas.size() <= netxs::si32max);
            auto maxs = (si32)canvas.size();
            from = std::clamp(from, 0, maxs ? maxs - 1 : 0);
            upto = std::clamp(upto, 0, maxs);
            auto size = upto - from;
            return std::span{ canvas.begin() + from, (size_t)size };
        }
        auto subline(twod p1, twod p2) const // core: Get stripe.
        {
            if (p1.y > p2.y || (p1.y == p2.y && p1.x > p2.x)) std::swap(p1, p2);
            auto from = p1.x + p1.y * region.size.x;
            auto upto = p2.x + p2.y * region.size.x + 1;
            return subline(from, upto);
        }
        auto tile(core& image, auto fx) // core: Tile with a specified bitmap.
        {
            auto step = image.size();
            auto grid = netxs::grid_mod(region.coor, step);
            auto init = region.coor - grid - region.coor.less(dot_00, step, dot_00);
            auto coor = init;
            auto stop = region.coor + region.size;
            while (coor.y < stop.y)
            {
                while (coor.x < stop.x)
                {
                    image.move(coor);
                    fill(image, fx);
                    coor.x += step.x;
                }
                coor.x = init.x;
                coor.y += step.y;
            }
        }
        void operator += (core const& src) // core: Append specified canvas.
        {
            //todo inbody::RTL
            auto a_size = size();
            auto b_size = src.size();
            auto new_sz = twod{ a_size.x + b_size.x, std::max(a_size.y, b_size.y) };
            auto block = core{ region.coor, new_sz, marker };

            auto r = rect{{ 0, new_sz.y - a_size.y }, a_size };
            netxs::inbody<faux>(block, *this, r, dot_00, cell::shaders::full);
            r.coor.x = a_size.x;
            r.coor.y = new_sz.y - b_size.y;
            r.size = b_size;
            netxs::inbody<faux>(block, src, r, dot_00, cell::shaders::full);

            swap(block);
            digest++;
        }
    };
}

namespace netxs::misc
{
    template<si32 Repeat = 2, bool InnerGlow = faux, class T = vrgb, class P = noop, si32 Ratio = 1>
    void boxblur(auto& image, si32 r, T&& cache = {}, P shade = {})
    {
        using irgb = std::decay_t<T>::value_type;

        auto area = image.area();
        auto clip = image.clip().trim(area);
        if (!clip) return;

        auto w = std::max(0, clip.size.x);
        auto h = std::max(0, clip.size.y);
        auto s = w * h;

        if (cache.size() < (size_t)s)
        {
            cache.resize(s);
        }

        auto start = clip.coor - area.coor;
        auto s_ptr = image.begin() + start.x + area.size.x * start.y;
        auto d_ptr = cache.begin();

        auto s_width = area.size.x;
        auto d_width = clip.size.x;

        auto s_point = [](auto c)->auto& { return *c; }; //->bgc(); };
        auto d_point = [](auto c)->auto& { return *c; };

        for (auto _(Repeat); _--;) // Emulate Gaussian blur.
        netxs::boxblur<irgb, InnerGlow>(s_ptr,
                                        d_ptr, w,
                                               h, r, s_width,
                                                     d_width, Ratio, s_point,
                                                                     d_point, shade);
    }

    void contour(auto& image)
    {
        static auto shadows_cache = netxs::raw_vector<fp32>{};
        static auto boxblur_cache = netxs::raw_vector<fp32>{};
        auto r = image.area();
        auto v = r.size.x * r.size.y;
        boxblur_cache.resize(v);
        shadows_cache.resize(v);
        auto shadows_image = netxs::raster<std::span<fp32>>{ shadows_cache, r };
        netxs::misc::cage(shadows_image, shadows_image.area(), dent{ 1, 0, 1, 0 }, [](auto& dst){ dst = 0.f; }); // Clear cached garbage (or uninitialized data) after previous blur (1px border at the top and left sides).
        shadows_image.step(-dot_11);
        netxs::onbody(image, shadows_image, [](auto& src, auto& dst){ dst = src ? 255.f * 3.f : 0.f; }); // Note: Pure black pixels will become invisible/transparent.
        shadows_image.step(dot_11);
        shadows_image.clip(r);
        netxs::misc::boxblur<2>(shadows_image, 1, boxblur_cache);
        netxs::oncopy(image, shadows_image, [](auto& src, auto& dst){ src.chan.a = src ? 0xFF : (byte)std::clamp(dst, 0.f, 255.f); });
    }
}