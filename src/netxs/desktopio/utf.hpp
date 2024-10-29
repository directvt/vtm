// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

#include "intmath.hpp"
#include "unidata.hpp"

#include <string>
#include <string_view>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <charconv>
#include <optional>
#include <sstream>
#include <span>
#include <bitset>

namespace netxs
{
    using view = std::string_view;
    using text = std::string;
    using wide = std::wstring;
    using wiew = std::wstring_view;
    using wchr = wchar_t;
    using flux = std::stringstream;
    using utfx = uint32_t;
    using namespace std::literals;

    static constexpr auto whitespaces = " \n\r\t"sv;
    static constexpr auto alphabetic  = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"sv;
    static constexpr auto base64code  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    static constexpr auto whitespace  = ' '; // '.';
    static constexpr auto emptyspace  = "\0"sv; //"\xC0\x80"sv; // In Modified UTF-8, the null character (U+0000) uses the two-byte overlong encoding 11000000 10000000 (hexadecimal C0 80), instead of 00000000 (hexadecimal 00).
}

namespace netxs::utf
{
    using ctrl = unidata::cntrls;

    static constexpr auto c0_view = { "·"sv, "☺"sv, "☻"sv, "♥"sv, "♦"sv, "♣"sv, "♠"sv, "•"sv, "◘"sv, "○"sv, "◙"sv, "♂"sv, "♀"sv, "♪"sv, "♫"sv, "☼"sv,
                                      "►"sv, "◄"sv, "↕"sv, "‼"sv, "¶"sv, "§"sv, "▬"sv, "↨"sv, "↑"sv, "↓"sv, "→"sv, "←"sv, "∟"sv, "↔"sv, "▲"sv, "▼"sv,
                                      "⌂"sv };
    static constexpr auto c0_wchr = { L'\0',L'☺', L'☻', L'♥', L'♦', L'♣', L'♠', L'•', L'◘', L'○', L'◙', L'♂', L'♀', L'♪', L'♫', L'☼',
                                      L'►', L'◄', L'↕', L'‼', L'¶', L'§', L'▬', L'↨', L'↑', L'↓', L'→', L'←', L'∟', L'↔', L'▲', L'▼',
                                      L'⌂' };
    static constexpr auto replacement_code = utfx{ 0x0000'FFFD };
    static constexpr auto vs04_code = utfx{ 0x0000'FE03 };
    static constexpr auto vs05_code = utfx{ 0x0000'FE04 };
    static constexpr auto vs06_code = utfx{ 0x0000'FE05 };
    static constexpr auto vs07_code = utfx{ 0x0000'FE06 };
    static constexpr auto vs08_code = utfx{ 0x0000'FE07 };
    static constexpr auto vs09_code = utfx{ 0x0000'FE08 };
    static constexpr auto vs10_code = utfx{ 0x0000'FE09 };
    static constexpr auto vs11_code = utfx{ 0x0000'FE0A };
    static constexpr auto vs12_code = utfx{ 0x0000'FE0B };
    static constexpr auto vs13_code = utfx{ 0x0000'FE0C };
    static constexpr auto vs14_code = utfx{ 0x0000'FE0D };
    static constexpr auto vs15_code = utfx{ 0x0000'FE0E };
    static constexpr auto vs16_code = utfx{ 0x0000'FE0F };

    template<utfx code>
    static constexpr auto utf8bytes = code <= 0x007f ? std::array<char, 4>{ static_cast<char>(code) }
                                    : code <= 0x07ff ? std::array<char, 4>{ static_cast<char>(0xc0 | ((code >> 0x06) & 0x1f)), static_cast<char>(0x80 | ( code & 0x3f)) }
                                    : code <= 0xffff ? std::array<char, 4>{ static_cast<char>(0xe0 | ((code >> 0x0c) & 0x0f)), static_cast<char>(0x80 | ((code >> 0x06) & 0x3f)), static_cast<char>(0x80 | ( code & 0x3f)) }
                                                     : std::array<char, 4>{ static_cast<char>(0xf0 | ((code >> 0x12) & 0x07)), static_cast<char>(0x80 | ((code >> 0x0c) & 0x3f)), static_cast<char>(0x80 | ((code >> 0x06) & 0x3f)), static_cast<char>(0x80 | ( code & 0x3f)) };
    template<utfx code>
    static constexpr auto utf8view = view{ utf8bytes<code>.data(), code <= 0x007f ? 1u : code <= 0x07ff ? 2u : code <= 0xffff ? 3u : 4u };
    static constexpr auto replacement = utf8view<replacement_code>; // \uFFFD = 0xEF 0xBF 0xBD (efbfbd) "�"
    static constexpr auto vs04 = utf8view<vs04_code>;
    static constexpr auto vs05 = utf8view<vs05_code>;
    static constexpr auto vs06 = utf8view<vs06_code>;
    static constexpr auto vs07 = utf8view<vs07_code>;
    static constexpr auto vs08 = utf8view<vs08_code>;
    static constexpr auto vs09 = utf8view<vs09_code>;
    static constexpr auto vs10 = utf8view<vs10_code>;
    static constexpr auto vs11 = utf8view<vs11_code>;
    static constexpr auto vs12 = utf8view<vs12_code>;
    static constexpr auto vs13 = utf8view<vs13_code>;
    static constexpr auto vs14 = utf8view<vs14_code>;
    static constexpr auto vs15 = utf8view<vs15_code>;
    static constexpr auto vs16 = utf8view<vs16_code>;

    // utf: Unicode Character Size Modifiers
    namespace matrix
    {
        template<si32 xy>
        static auto mosaic = []{ return xy / 10 + ((xy % 10) << 4); }();
        static auto p = [](auto x){ return x * (x + 1) / 2; }; // ref: https://github.com/directvt/vtm/assets/11535558/88bf5648-533e-4786-87de-b3dc4103273c
        static constexpr auto kx = 8;
        static constexpr auto ky = 4;
        static constexpr auto mx = p(kx + 1);
        static constexpr auto my = p(ky + 1);
        static auto s = [](auto w, auto h, auto x, auto y){ return p(w) + x + (p(h) + y) * mx; };
        template<si32 wh, si32 xy = 0>
        static constexpr auto vs = []
        {
            auto w = wh / 10;
            auto h = wh % 10;
            auto x = xy / 10;
            auto y = xy % 10;
            auto v = p(w) + x + (p(h) + y) * mx;
            return v;
        }();
        static constexpr auto vs_block = 0xD0000;
        template<si32 wh, si32 xy = 0>
        static constexpr auto vs_code = vs_block + vs<wh, xy>;
        template<si32 wh, si32 xy = 00, auto code = vs_code<wh, xy>>
        static constexpr auto vss = utf8view<code>;

        auto vs_runtime(si32 w, si32 h, si32 x = {}, si32 y = {})
        {
            return vs_block + p(w) + x + (p(h) + y) * mx;
        }
        auto whxy(si32 vs_value)
        {
            static auto lut = []
            {
                struct r
                {
                    si32 w : 8;
                    si32 h : 8;
                    si32 x : 8;
                    si32 y : 8;
                };
                auto v = std::vector(mx * my, r{});
                for (auto w = 1; w <= kx; w++)
                for (auto h = 1; h <= ky; h++)
                for (auto y = 0; y <= h; y++)
                for (auto x = 0; x <= w; x++)
                {
                    v[p(w) + x + (p(h) + y) * mx] = { w, h, x, y };
                }
                return v;
            }();
            return lut[vs_value];
        }
    }
    static constexpr auto mtx = std::to_array({ matrix::vs<00,00>, matrix::vs<11,11>, matrix::vs<21,00> });

    // utf: Grapheme cluster properties.
    struct prop : public unidata::unidata
    {
        size_t utf8len;
        size_t cpcount;
        bool   correct;
        utfx   cdpoint;
        si32   cmatrix;

        constexpr prop(size_t size)
            : unidata{      },
              utf8len{ size },
              cpcount{ 0    },
              correct{ faux },
              cdpoint{ 0    },
              cmatrix{ mtx[unidata::ucwidth] }
        { }
        prop(utfx code, size_t size)
            : unidata{ code },
              utf8len{ size },
              cpcount{ 1    },
              correct{ true },
              cdpoint{ code },
              cmatrix{ mtx[unidata::ucwidth] }
        { }
        constexpr prop(prop const& attr)
            : unidata{ attr         },
              utf8len{ attr.utf8len },
              cpcount{ attr.cpcount },
              correct{ attr.correct },
              cdpoint{ attr.cdpoint },
              cmatrix{ attr.cmatrix }
        { }
        constexpr prop& operator = (prop const&) = default;

        auto combine(prop const& next)
        {
            if (next.utf8len && unidata::allied(next))
            {
                if (next.cdpoint >= matrix::vs_code<00,00> && next.cdpoint <= matrix::vs_code<84,84>) // Set matrix size and drop VS-wh_xy modificator.
                {
                    cmatrix = (si32)(next.cdpoint - matrix::vs_block);
                }
                else
                {
                    if (next.ucwidth > unidata::ucwidth)
                    {
                        unidata::ucwidth = next.ucwidth;
                        cmatrix = mtx[unidata::ucwidth];
                    }
                    utf8len += next.utf8len;
                    cpcount += 1;
                }
                return 0_sz;
            }
            else
            {
                return utf8len;
            }
        }
    };

    // utf: First byte based UTF-8 codepoint lengths.
    static constexpr auto utf8lengths = std::to_array(
    {   //      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
        /* 0 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        /* 1 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        /* 2 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        /* 3 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        /* 4 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        /* 5 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        /* 6 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        /* 7 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        /* 8 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 9 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* A */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* B */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* C */ 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        /* D */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        /* E */ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        /* F */ 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    });

    // utf: Codepoint iterator.
    struct cpit
    {
        using chrptr = view::const_pointer;

        chrptr textptr;
        size_t balance;
        size_t utf8len;

        operator bool () { return balance > 0; }
        cpit(view utf8)
            : textptr{ utf8.data() },
              balance{ utf8.size() },
              utf8len{ 0           }
        { }

        auto front()
        {
            return *textptr;
        }
        void redo(view utf8)
        {
            textptr = utf8.data();
            balance = utf8.size();
        }
        view rest()
        {
            return view{ textptr, balance };
        }
        void step()
        {
            textptr += utf8len;
            balance -= utf8len;
            utf8len = 0;
        }
        prop take()
        {
            utfx cp;
            utfx c2;
            utfx c3;
            utfx c4;

            if (balance)
            {
                auto data = reinterpret_cast<byte const*>(textptr);
                cp = *data;
                switch (utf8lengths[cp])
                {
                    case 1:
                        utf8len = 1U;
                        break;
                    case 0:
                        return prop(utf8len = 1);
                        break;
                    case 2:
                        if (balance > 1)
                        {
                            if (c2 = *++data; (c2 & 0xC0) == 0x80)
                            {
                                utf8len = 2U;
                                cp = ((cp & 0b00011111) << 6)
                                    | (c2 & 0b00111111);
                            }
                            else return prop(utf8len = 1);
                        }
                        else return prop(utf8len = 1);
                        break;
                    case 3:
                        if (balance > 2)
                        {
                            if (c2 = *++data; (c2 & 0xC0) == 0x80)
                            {
                                if (c3 = *++data; (c3 & 0xC0) == 0x80)
                                {
                                    utf8len = 3U;
                                    cp = ((cp & 0b00001111) << 12)
                                        |((c2 & 0b00111111) << 6)
                                        | (c3 & 0b00111111);

                                    if (cp >= 0xfdd0 && (cp < 0xfdf0 || ((cp & 0xfffe) == 0xfffe)))
                                    {
                                        return prop(utf8len);
                                    }
                                }
                                else return prop(utf8len = 2);
                            }
                            else return prop(utf8len = 1);
                        }
                        else return prop(utf8len = balance);
                        break;
                    default: // case 4
                        if (balance > 3)
                        {
                            if (c2 = *++data; (c2 & 0xC0) == 0x80)
                            {
                                if (c3 = *++data; (c3 & 0xC0) == 0x80)
                                {
                                    if (c4 = *++data; (c4 & 0xC0) == 0x80)
                                    {
                                        utf8len = 4U;
                                        cp = ((cp & 0b00000111) << 18)
                                            |((c2 & 0b00111111) << 12)
                                            |((c3 & 0b00111111) << 6)
                                            | (c4 & 0b00111111);

                                        if (cp > 0x10ffff)
                                        {
                                            return prop(utf8len);
                                        }
                                    }
                                    else return prop(utf8len = 3);
                                }
                                else return prop(utf8len = 2);
                            }
                            else return prop(utf8len = 1);
                        }
                        else return prop(utf8len = balance);
                        break;
                }
            }
            else return prop(utf8len = 0);

            return prop(cp, utf8len);
        }
        auto next()
        {
            auto cpoint = take();
                          step();
            return cpoint;
        }
    };

    // utf: Grapheme cluster with properties.
    struct frag
    {
        view text;
        prop attr;

        template<bool AllowControls = faux>
        static auto take_cluster(view utf8)
        {
            if (auto code = cpit{ utf8 })
            {
                auto next = code.take();
                if (next.is_cmd())
                {
                    if constexpr (AllowControls) return frag{ view(code.textptr, code.utf8len), next };
                    else
                    {
                        if (next.cdpoint == 0x02 /*ansi::c0_stx*/) // Custom cluster initiator.
                        {
                            code.step();
                            next = code.take();
                            auto head = code.textptr;
                            auto left = next;
                            auto utf8len = 0_sz;
                            auto cpcount = 0;
                            while (next.correct && (next.cdpoint < matrix::vs_code<00,00> || next.cdpoint > matrix::vs_code<84,84>)) // Eat all until VS.
                            {
                                utf8len += next.utf8len;
                                cpcount += 1;
                                code.step();
                                next = code.take();
                            }
                            if (next.correct)
                            {
                                left.utf8len = utf8len;
                                left.cpcount = cpcount;
                                left.cmatrix = next.cdpoint - matrix::vs_block;
                                return frag{ view(head, left.utf8len), left };
                            }
                            else return frag{ replacement, left };
                        }
                        else return frag{ replacement, next };
                    }
                }
                auto head = code.textptr;
                auto left = next;
                do
                {
                    code.step();
                    if (next.correct)
                    {
                        next = code.take();
                        if (auto size = left.combine(next))
                        {
                            return frag{ view(head, size), left };
                        }
                    }
                    else
                    {
                        next.utf8len = left.utf8len;
                        return frag{ replacement, next };
                    }
                }
                while (true);
            }
            return frag{ replacement, prop{ 0 } };
        }

        frag(view utf8, prop const& attr)
            : text{ utf8 },
              attr{ attr }
        { }
        frag(view utf8)
            : frag{ take_cluster(utf8) }
        { }
    };

    // utf: Return the first grapheme cluster and its Unicode attributes.
    template<bool AllowControls = faux>
    auto cluster(view utf8)
    {
        return frag::take_cluster<AllowControls>(utf8);
    }

    // utf: Break the text into the enriched grapheme clusters.
    //      Forward the result to the dest using the "serve" and "yield" lambdas.
    //      serve: Handle escaped control sequences.
    //             auto s = [&](utf::prop const& traits, view utf8) -> view;
    //      yield: Handle grapheme clusters.
    //             auto y = [&](frag const& cluster){};
    //      Clusterize: parse by grapheme clusters (true) or codepoints (faux)
    template<bool Clusterize = true, class S, class Y, class A>
    void decode(S serve, Y yield, A ascii,  view utf8, si32& decsg)
    {
        static const auto dec_sgm_lookup = std::vector<frag> // DEC Special Graphics mode lookup table.
        {//  _      `      a      b       c       d       e      f      g      h       i       j      k      l      m      n     o       p      q      r      s      t      u      v      w      x      y      z      {      |      }      ~                                 };
            "w"sv, "♦"sv, "▒"sv, "␉"sv, "␌"sv, "␍"sv, "␊"sv, "°"sv, "±"sv, "␤"sv, "␋"sv, "┘"sv, "┐"sv, "┌"sv, "└"sv, "┼"sv, "⎺"sv, "⎻"sv, "─"sv, "⎼"sv, "⎽"sv, "├"sv, "┤"sv, "┴"sv, "┬"sv, "│"sv, "≤"sv, "≥"sv, "π"sv, "≠"sv, "£"sv, "·"sv,
        };
        if (auto code = cpit{ utf8 })
        {
            auto next = code.take();
            do
            {
                if (next.is_cmd())
                {
                    code.step();
                    if (next.cdpoint == 0x02 /*ansi::c0_stx*/) // Custom cluster initiator.
                    {
                        next = code.take();
                        auto rest = code.rest();
                        auto left = next;
                        auto utf8len = 0_sz;
                        auto cpcount = 0;
                        while (next.correct && (next.cdpoint < matrix::vs_code<00,00> || next.cdpoint > matrix::vs_code<84,84>)) // Eat all until VS.
                        {
                            utf8len += next.utf8len;
                            cpcount += 1;
                            code.step();
                            next = code.take();
                        }
                        if (next.correct)
                        {
                            left.utf8len = utf8len;
                            left.cpcount = cpcount;
                            left.cmatrix = next.cdpoint - matrix::vs_block;
                            auto crop = frag{ rest.substr(0, left.utf8len), left };
                            yield(crop);
                            code.step(); // Drop VS codepoint.
                            next = code.take();
                        }
                        else // Silently ignore STX.
                        {
                            code.redo(rest);
                            next = left;
                        }
                    }
                    else // Proceed general control.
                    {
                        auto rest = code.rest();
                        auto chars = serve(next, rest);
                        code.redo(chars);
                        next = code.take();
                    }
                }
                else
                {
                    auto is_plain = [](char c){ return c >= 0x20 && (byte)(c - 0x7f) > 0x20; };
                    if (decsg && next.cdpoint <= 0x7e && next.cdpoint >= 0x5f) // ['_' - '~']
                    {
                        yield(dec_sgm_lookup[next.cdpoint - 0x5f]);
                        code.step();
                        next = code.take();
                        continue;
                    }
                    else if (is_plain(code.front()))
                    {
                        auto rest = code.rest();
                        auto head = rest.begin();
                        auto iter = head;
                        auto tail = rest.end();
                        while (++iter != tail && is_plain(*iter))
                        { }
                        auto plain = view{ head, iter };
                        if (iter == tail)
                        {
                            ascii(plain);
                            break;
                        }
                        code.redo(view{ iter, tail });
                        auto left = next;
                        next = code.take();
                        if (left.allied(next)) // Check if the next codepoint is some kind of joiner/modifier.
                        {
                            --iter;
                            if (head != iter)
                            {
                                plain = view{ head, iter };
                                ascii(plain);
                            }
                            code.redo(view{ iter, tail });
                            code.utf8len = 1; // ASCII.
                            next = left;
                            // Fall to clusterize.
                        }
                        else
                        {
                            ascii(plain);
                            continue;
                        }
                    }
                    if constexpr (Clusterize)
                    {
                        auto head = code.textptr;
                        auto left = next;
                        do
                        {
                            code.step();
                            if (next.correct)
                            {
                                if (!code || (next = code.take(), left.combine(next)))
                                {
                                    auto crop = frag{ view(head, left.utf8len), left };
                                    yield(crop);
                                    break;
                                }
                            }
                            else
                            {
                                next.utf8len = left.utf8len;
                                auto crop = frag{ replacement, next };
                                yield(crop);
                                next = code.take();
                                break;
                            }
                        }
                        while (true);
                    }
                    else
                    {
                        if (next.correct)
                        {
                            auto crop = frag{ view(code.textptr, next.utf8len), next };
                            yield(crop);
                        }
                        else
                        {
                            auto crop = frag{ replacement, next };
                            yield(crop);
                        }
                        code.step();
                        next = code.take();
                    }
                }
            }
            while (code);
        }
    }

    struct qiew : public view
    {
        using view::view;

        struct hash
        {
            auto operator()(qiew key) const { return std::hash<view>{}(key); }
        };
        struct equal
        {
            auto operator()(qiew lhs, qiew rhs) const { return lhs.compare(rhs) == 0; }
        };

        constexpr qiew(qiew const&) = default;
        constexpr qiew(char const& v) noexcept : view(&v, 1) { }
        constexpr qiew(view const& v) noexcept : view(v) { }
                  qiew(text const& v) noexcept : view(v) { }
        constexpr qiew(char const* ptr, auto&&... len) noexcept : view(ptr, std::forward<decltype(len)>(len)...) { }
        constexpr qiew& operator = (qiew const&) noexcept = default;

                 operator text () const { return text{ data(), size() }; }
        explicit operator bool () const { return view::length(); }

        // qiew: Clear.
        constexpr auto clear()
        {
            view::operator=({});
        }
        // qiew: Return substring.
        constexpr auto substr(size_t offset = 0, size_t count = npos) const
        {
            return qiew{ view::substr(offset, count) };
        }
        // qiew: Convert to text.
        auto str() const { return operator text(); }
        // qiew: Peek front char.
        si32 front() const { return (byte)view::front(); }
        // qiew: Pop front.
        auto pop_front()
        {
            auto c = view::front();
            view::remove_prefix(1);
            return c;
        }
        // qiew: Pop the front sequence of the same control points and return their count + 1.
        auto pop_all(ctrl cmd)
        {
            auto n = 1;
            auto next = utf::cluster<true>(*this);
            while (next.attr.control == cmd)
            {
                view::remove_prefix(next.attr.utf8len);
                next = utf::cluster<true>(*this);
                n++;
            }
            return n;
        }
        // qiew: Pop the front sequence of the same chars and return their count + 1.
        auto pop_all(char c)
        {
            auto n = 1;
            while (length() && view::front() == c)
            {
                view::remove_prefix(1);
                n++;
            }
            return n;
        }
        // qiew: Return true and pop the front control point when it is equal to cmd.
        auto pop_if(ctrl cmd)
        {
            auto next = utf::cluster<true>(*this);
            if (next.attr.control == cmd)
            {
                view::remove_prefix(next.attr.utf8len);
                return true;
            }
            return faux;
        }
        // qiew: Return true and pop the front char when it is equal to c.
        auto pop_if(char c)
        {
            if (length() && view::front() == c)
            {
                view::remove_prefix(1);
                return true;
            }
            return faux;
        }
    };

    template<class A = si32, si32 Base = 10, class View, class = std::enable_if_t<std::is_base_of_v<view, View>>>
    std::optional<A> to_int(View& ascii)
    {
        auto num = A{};
        auto top = ascii.data();
        auto end = top + ascii.length();
        if constexpr (std::is_floating_point_v<A>)
        {
            //todo neither clang nor apple clang support from_chars with floating point (ver < 15.0)
            //if (auto [pos, err] = std::from_chars(top, end, num); err == std::errc())
            auto integer = si64{};
            if (auto [pos, err] = std::from_chars(top, end, integer, Base); err == std::errc())
            {
                ascii.remove_prefix(pos - top);
                num = (A)integer;
                if (ascii.size() && ascii.front() == '.')
                {
                    ascii.pop_front();
                    top = ascii.data();
                    if (auto [mpos, merr] = std::from_chars(top, end, integer, Base); merr == std::errc())
                    {
                        auto len = mpos - top;
                        num += (A)(integer * std::pow(10, -len));
                        ascii.remove_prefix(len);
                    }
                }
                return num;
            }
        }
        else
        {
            if (auto [pos, err] = std::from_chars(top, end, num, Base); err == std::errc())
            {
                ascii.remove_prefix(pos - top);
                return num;
            }
        }
        return std::nullopt;
    }
    template<class A = si32, si32 Base = 10, class T>
    auto to_int(T&& utf8)
    {
        auto shadow = view{ utf8 };
        return to_int<A, Base>(shadow);
    }
    template<si32 Base = 10, class T, class A>
    auto to_int(T&& utf8, A fallback)
    {
        auto result = to_int<A, Base>(std::forward<T>(utf8));
        return result ? result.value() : fallback;
    }

    void capacity(auto& target, auto additional)
    {
        auto required = target.size() + additional;
        if (target.capacity() < required) target.reserve(std::max(target.size(), additional));
    }
    void to_utf(char const* utf8, size_t size, wide& wide_text)
    {
        // � The standard also recommends replacing each error with the replacement character.
        //
        // In terms of the newline, Unicode introduced U+2028 LINE SEPARATOR
        // and U+2029 PARAGRAPH SEPARATOR.
        //  c̳̻͚̻̩̻͉̯̄̏͑̋͆̎͐ͬ͑͌́͢h̵͔͈͍͇̪̯͇̞͖͇̜͉̪̪̤̙ͧͣ̓̐̓ͤ͋͒ͥ͑̆͒̓͋̑́͞ǎ̡̮̤̤̬͚̝͙̞͎̇ͧ͆͊ͅo̴̲̺͓̖͖͉̜̟̗̮̳͉̻͉̫̯̫̍̋̿̒͌̃̂͊̏̈̏̿ͧ́ͬ̌ͥ̇̓̀͢͜s̵̵̘̹̜̝̘̺̙̻̠̱͚̤͓͚̠͙̝͕͆̿̽ͥ̃͠͡
        capacity(wide_text, size);
        auto code = utfx{};
        auto tail = utf8 + size;
        while (utf8 < tail)
        {
            auto c = (byte)*utf8++;

                 if (c < 0x80) code = c;
            else if (c < 0xc0) code =(c & 0x3f) | code << 6;
            else if (c < 0xe0) code = c & 0x1f;
            else if (c < 0xf0) code = c & 0x0f;
            else               code = c & 0x07;

            if (code <= 0x10ffff)
            {
                if (utf8 == tail || (*utf8 & 0xc0) != 0x80)
                {
                    if (code < 0xd800 || (code >= 0xe000 && code <= 0xffff) || sizeof(wchr) > 2) // single | wchr == char32_t
                    {
                        wide_text.push_back((wchr)code);
                    }
                    else if (code > 0xffff) // surrogate pair
                    {
                        wide_text.append({ (wchr)(0xd800 + ((code - 0x10000) >> 10)),
                                           (wchr)(0xdc00 + (code & 0x03ff)) });
                    }
                }
            }
            else wide_text.push_back(replacement_code);
        }
        //wide_text.shrink_to_fit();
    }
    template<template<class...> class TextOrView, class ...Args>
    void to_utf(TextOrView<char, Args...> const& utf8, wide& wide_text)
    {
        to_utf(utf8.data(), utf8.size(), wide_text);
    }
    void to_utf(std::vector<prop> const& codepoints, wide& wide_text)
    {
        for (auto& r : codepoints)
        {
            auto code = r.cdpoint;
            if (code < 0xD800 || (code >= 0xE000 && code <= 0xFFFF))
            {
                wide_text.push_back((wchr)code);
            }
            else if (code > 0xFFFF && code <= 0x10FFFF) // surrogate pair
            {
                wide_text.append({ (wchr)(0xD800 + ((code - 0x10000) >> 10)),
                                   (wchr)(0xDC00 + (code & 0x03FF)) });
            }
            else wide_text.push_back(replacement_code);
        }
    }
    wide to_utf(char const* utf8, size_t size)
    {
        auto wide_text = wide{};
        to_utf(utf8, size, wide_text);
        return wide_text;
    }
    utfx to_code(wchr c)
    {
        utfx code;
        if (c >= 0xd800 && c <= 0xdbff)
        {
            code = ((c - 0xd800) << 10) + 0x10000;
        }
        else
        {
            if (c >= 0xdc00 && c <= 0xdfff) code = c - 0xdc00;
            else                            code = c;
        }
        return code;
    }
    // Return faux only on first part of surrogate pair.
    bool to_code(wchr c, utfx& code)
    {
        auto first_part = c >= 0xd800 && c <= 0xdbff;
        if (first_part) // First part of surrogate pair.
        {
            code = ((c - 0xd800) << 10) + 0x10000;
        }
        else
        {
            if (c >= 0xdc00 && c <= 0xdfff) // Second part of surrogate pair.
            {
                if (code) code |= c - 0xdc00;
                else      code = utf::replacement_code; // Broken pair.
            }
            else code = c;
        }
        return !first_part;
    }
    namespace
    {
        void _to_utf(utfx code, auto push)
        {
            if (code <= 0x007f)
            {
                push(static_cast<char>(code));
            }
            else if (code <= 0x07ff)
            {
                push(static_cast<char>(0xc0 | ((code >> 0x06) & 0x1f)));
                push(static_cast<char>(0x80 | ( code          & 0x3f)));
            }
            else if (code <= 0xffff)
            {
                push(static_cast<char>(0xe0 | ((code >> 0x0c) & 0x0f)));
                push(static_cast<char>(0x80 | ((code >> 0x06) & 0x3f)));
                push(static_cast<char>(0x80 | ( code          & 0x3f)));
            }
            else
            {
                push(static_cast<char>(0xf0 | ((code >> 0x12) & 0x07)));
                push(static_cast<char>(0x80 | ((code >> 0x0c) & 0x3f)));
                push(static_cast<char>(0x80 | ((code >> 0x06) & 0x3f)));
                push(static_cast<char>(0x80 | ( code          & 0x3f)));
            }
        }
        void _to_utf(text& utf8, utfx code)
        {
            _to_utf(code, [&](char c){ utf8.push_back(c); });
        }
        void _to_utf(auto& iter, utfx code)
        {
            _to_utf(code, [&](char c){ *iter++ = c; });
        }
    }
    auto to_utf_from_code(utfx code)
    {
        auto utf8 = text{};
        _to_utf(utf8, code);
        return utf8;
    }
    void to_utf_from_code(utfx code, text& utf8_out)
    {
        _to_utf(utf8_out, code);
    }
    void to_utf(wchr const* wide_text, size_t size, text& utf8)
    {
        capacity(utf8, size * 3/*worst case*/);
        auto code = utfx{ 0 };
        auto tail = wide_text + size;
        while (wide_text < tail)
        {
            auto c = *wide_text++;
            if (c >= 0xd800 && c <= 0xdbff)
            {
                code = ((c - 0xd800) << 10) + 0x10000;
            }
            else
            {
                if (c >= 0xdc00 && c <= 0xdfff) code |= c - 0xdc00;
                else                            code  = c;
                _to_utf(utf8, code);
                code = 0;
            }
        }
    }
    template<template<class...> class WideTextOrView, class ...Args>
    void to_utf(WideTextOrView<wchr, Args...> const& wide_text, text& utf8)
    {
        to_utf(wide_text.data(), wide_text.size(), utf8);
    }
    void to_utf(std::span<wchr> wide_text, text& utf8)
    {
        to_utf(wide_text.data(), wide_text.size(), utf8);
    }
    text to_utf(wchr const* wide_text, size_t size)
    {
        auto utf8 = text{};
        to_utf(wide_text, size, utf8);
        return utf8;
    }
    template<class WideOrUTF8>
    auto to_utf(WideOrUTF8&& str)
    {
        return to_utf(str.data(), str.size());
    }
    template<class T>
    auto to_utf(T* c_str)
    {
        auto iter = c_str;
        while (*iter != 0) { ++iter; }
        return to_utf(c_str, iter - c_str);
    }
    auto to_utf(wchr wc)
    {
        return to_utf(&wc, 1);
    }
    void to_utf(wchr wc, text& utf8)
    {
        to_utf(&wc, 1, utf8);
    }
    auto to_utf(wchr wc, wide& pair, text& utf8)
    {
        if (wc >= 0xd800 && wc <= 0xdbff) // First part of surrogate pair.
        {
            pair.clear();
            pair.push_back(wc);
            return 0_sz;
        }
        else
        {
            auto step = utf8.size();
            if (wc >= 0xdc00 && wc <= 0xdfff) // Second part of surrogate pair.
            {
                if (pair.empty()) // Broken surrogate pair.
                {
                    utf8 += utf::replacement;
                }
                else
                {
                    pair.push_back(wc);
                    to_utf(pair, utf8);
                    pair.clear();
                }
            }
            else to_utf(wc, utf8);

            return utf8.size() - step;
        }
    }
    template<class Result = si32>
    auto length(view utf8)
    {
        auto length = Result{ 0 };
        for (auto c : utf8)
        {
            length += (c & 0xc0) != 0x80;
        }
        return length;
    }
    // utf: Check utf-8 integrity (last codepoint) and cut off the invalid bytes at the end.
    void purify(view& utf8)
    {
        auto head = utf8.rend();
        auto tail = utf8.rbegin();
        while (tail != head && (*tail & 0xc0) == 0x80) // Find first byte.
        {
            ++tail;
        }
        if (tail != head) // Check codepoint.
        {
            auto p = head - tail - 1;
            auto l = utf::cluster<true>(utf8.substr(p));
            if (!l.attr.correct)
            {
                utf8 = utf8.substr(0, p);
            }
        }
        else // Bad UTF-8 encoding
        {
            //Recycle all bad bytes (log?).
        }
    }
    auto substr(qiew utf8, size_t start, size_t length = text::npos)
    {
        if (length == 0) return qiew{};
        auto head = utf8.data();
        auto stop = head + utf8.size();
        auto calc = [](auto& it, auto& count, auto limit)
        {
            while (it != limit)
            {
                if ((*it & 0xc0) != 0x80)
                {
                    if (!count--) break;
                }
                ++it;
            }
        };

        if (start) calc(head, start, stop);

        start = head - utf8.data();
        if (length != text::npos)
        {
            auto tail = head;
            calc(tail, length, stop);
            return utf8.substr(start, tail - head);
        }
        else return utf8.substr(start);
    }
    auto repeat(view what, size_t count)
    {
        auto result = text{};
        result.reserve(what.length() * count);
        while (count--)
        {
            result += what;
        }
        return result;
    }
    auto repeat(char letter, size_t count)
    {
        return text(count, letter);
    }
    template<class TextOrView, class C>
    auto remove(TextOrView&& from, C&& what)
    {
        auto _what = view{ what };
        auto s_size = from.size();
        auto c_size =_what.size();
        if (c_size)
        {
            while (s_size >= c_size && from.substr(s_size - c_size, c_size) == what)
            {
                s_size -= c_size;
            }
        }
        return from.substr(0, s_size);
    }
    void replace_all(text& utf8, auto const& from, auto const& to)
    {
        auto frag = view{ from };
        auto fill = view{ to };
        auto spot = 0_sz;
        auto line_sz = utf8.length();
        auto from_sz = frag.length();
        auto repl_sz = fill.length();
        if (!from_sz || line_sz < from_sz) return;
        if (from_sz == repl_sz)
        {
            while ((spot = utf8.find(frag, spot)) != text::npos)
            {
                utf8.replace(spot, from_sz, fill);
                spot += from_sz;
            }
        }
        else
        {
            auto last = 0_sz;
            if (from_sz < repl_sz)
            {
                auto temp = text{};
                temp.reserve((line_sz / from_sz + 1) * repl_sz); // In order to avoid reallocations.
                auto shadow = view{ utf8 };
                while ((spot = utf8.find(frag, last)) != text::npos)
                {
                    temp += shadow.substr(last, spot - last);
                    temp += fill;
                    spot += from_sz;
                    last  = spot;
                }
                temp += shadow.substr(last);
                utf8 = temp; // Assign to perform simultaneous shrinking and copying.
            }
            else
            {
                auto base = utf8.data();
                auto repl = fill.data();
                auto dest = base;
                auto copy = [](auto base, auto& dest, auto size)
                {
                    auto stop = base + size;
                    while (base != stop) { *dest++ = *base++; }
                };
                while ((spot = utf8.find(frag, last)) != text::npos)
                {
                    if (last) copy(base + last, dest, spot - last);
                    else      dest += spot;
                    copy(repl, dest, repl_sz);
                    spot += from_sz;
                    last  = spot;
                }
                copy(base + last, dest, line_sz - last);
                utf8.resize(dest - base);
            }
        }
    }
    auto replace_all(qiew utf8, auto const& from, auto const& to)
    {
        auto crop = utf8.str(); //todo avoid allocation
        replace_all(crop, from, to);
        return crop;
    }
    template<class TextOrView, class F>
    auto adjust(TextOrView&& utf8, size_t required_width, F const& fill_char, bool right_aligned = faux)
    {
        auto crop = text{};
        auto data = view{ utf8 };
        if (data.empty())
        {
            crop = repeat(fill_char, required_width);
        }
        else
        {
            if (required_width > 0)
            {
                crop = substr(data, 0, required_width);
            }
            auto size = length<size_t>(crop);
            if (required_width > size)
            {
                if (right_aligned) crop = repeat(fill_char, required_width - size) + crop;
                else               crop = crop + repeat(fill_char, required_width - size);
            }
        }
        return crop;
    }
    template<class Int_t, class T = char>
    text format(Int_t number, size_t by_group = 3, T const& delimiter = ' ')
    {
        if (by_group)
        {
            auto utf8 = std::to_string(std::abs(number));
            auto size = utf8.length();
            auto coor = (size - 1) % by_group + 1;
            auto crop = utf8.substr(0, coor);

            for (; coor < size; coor += by_group)
            {
                crop += delimiter;
                crop += utf8.substr(coor, by_group);
            }

            return number < 0 ? '-' + crop : crop;
        }
        else return std::to_string(number);
    }
    template<class T, int L = std::numeric_limits<T>::digits>
    auto to_bin(T n)
    {
        return std::bitset<L>(n).to_string();
    }
    template<bool UpperCase = faux>
    auto _to_hex(auto number, size_t width, auto push)
    {
        static constexpr auto nums = UpperCase ? "0123456789ABCDEF"
                                               : "0123456789abcdef";
        auto part = width * 4;
        while (std::exchange(part, part - 4))
        {
            push(nums[(number >> part) & 0x0f]);
        }
    }
    template<bool UpperCase = faux, class V, class = std::enable_if_t<std::is_integral_v<V>>>
    auto to_hex(V number, size_t width = sizeof(V) * 2, char filler = '0')
    {
        auto crop = text(width, filler);
        auto head = crop.begin();
        _to_hex<UpperCase>(number, width, [&](char c){ *head++ = c; });
        return crop;
    }
    template<class T, class ...Args>
    auto to_hex(T* ptr, Args&&... args)
    {
        return to_hex(reinterpret_cast<std::uintptr_t>(ptr), std::forward<Args>(args)...);
    }
    // utf: to_hex without allocations (the crop should has a reserved capacity).
    template<bool UpperCase = faux, class T, class = std::enable_if_t<std::is_integral_v<T>>>
    auto to_hex(T number, text& crop, size_t width = sizeof(T) * 2)
    {
        _to_hex<UpperCase>(number, width, [&](char c){ crop.push_back(c); });
        return crop;
    }
    auto to_hex_0x(auto const& n)
    {
        auto h = [](auto const& n){ return (flux{} << std::showbase << std::hex << n).str(); };
        if constexpr (std::is_same_v<wchr, std::decay_t<decltype(n)>>) return h((si32)n);
        else                                                           return h(n);
    }
    template<bool UpperCase = faux>
    auto buffer_to_hex(view buffer, bool formatted = faux)
    {
        if (formatted)
        {
            auto size = buffer.size();
            auto addr = 0_sz;
            auto crop = text{};
            while (addr < size)
            {
                auto frag = (size - addr > 16) ? 16
                                               : size - addr;
                crop += adjust(std::to_string(addr), 4, '0', true);
                for (auto i = 0_sz; i < 16; i++)
                {
                    if (i % 8 == 0)
                    {
                        crop += i < frag ? " -" : "  ";
                    }
                    crop += i < frag ? ' ' + utf::to_hex<UpperCase>(buffer[addr + i], 2)
                                     : "   ";
                }
                crop += "   ";
                for (auto i = addr; i < addr + frag; i++)
                {
                    auto c = (byte)buffer[i];
                    crop += (c < 0x21 || c > 0x7e) ? '.' : c;
                }
                crop += '\n';
                addr += 16;
            }
            return crop;
        }
        else
        {
            static constexpr auto nums = UpperCase ? "0123456789ABCDEF"
                                                   : "0123456789abcdef";
            auto size = buffer.size() << 1;
            auto buff = buffer.begin();
            auto crop = text(size, '0');
            auto head = crop.begin();
            auto tail = head + size;
            while (head != tail)
            {
                *head++ = nums[(*buff >> 4) & 0x0f];
                *head++ = nums[(*buff)      & 0x0f];
                buff++;
            }
            return crop;
        }
    }
    template<bool SkipEmpty = faux, feed Direction = feed::fwd, class P, bool Plain = std::is_same_v<void, std::invoke_result_t<P, view>>>
    auto split(view utf8, char delimiter, P proc)
    {
        if constexpr (Direction == feed::fwd)
        {
            auto cur = 0_sz;
            auto pos = 0_sz;
            while ((pos = utf8.find(delimiter, cur)) != text::npos)
            {
                auto frag = view{ utf8.data() + cur, pos - cur };
                if constexpr (SkipEmpty) if (frag.empty()) { cur = pos + 1; continue; }
                if constexpr (Plain) proc(frag);
                else            if (!proc(frag)) return faux;
                cur = pos + 1;
            }
            auto end = view{ utf8.data() + cur, utf8.size() - cur };
            if constexpr (SkipEmpty)
            {
                if (end.empty())
                {
                    if constexpr (Plain) return;
                    else                 return true;
                }
            }
            return proc(end);
        }
        else
        {
            auto cur = utf8.size();
            auto pos = utf8.size();
            while (cur && (pos = utf8.rfind(delimiter, cur - 1)) != text::npos)
            {
                auto next = pos + 1;
                auto frag = view{ utf8.data() + next, cur - next };
                if constexpr (SkipEmpty) if (frag.empty()) { cur = pos; continue; }
                if constexpr (Plain) proc(frag);
                else            if (!proc(frag)) return faux;
                cur = pos;
            }
            auto end = view{ utf8.data(), cur };
            if constexpr (SkipEmpty)
            {
                if (end.empty())
                {
                    if constexpr (Plain) return;
                    else                 return true;
                }
            }
            return proc(end);
        }
    }
    template<bool SkipEmpty = faux, class Container = std::vector<qiew>, class T>
    auto split(view utf8, T const& delimiter)
    {
        auto crop = Container{};
        auto mark = qiew(delimiter);
        if (auto len = mark.size())
        {
            auto cur = 0_sz;
            auto pos = 0_sz;
            if constexpr (requires{ crop.reserve(1); })
            {
                auto num = 0_sz;
                while ((pos = utf8.find(mark, cur)) != text::npos)
                {
                    ++num;
                    cur = pos + len;
                }
                crop.reserve(++num);
                cur = 0_sz;
                pos = 0_sz;
            }
            while ((pos = utf8.find(mark, cur)) != text::npos)
            {
                auto frag = qiew{ utf8.data() + cur, pos - cur };
                auto push = !SkipEmpty || !frag.empty();
                if (push) crop.push_back(frag);
                cur = pos + len;
            }
            auto tail = qiew{ utf8.data() + cur, utf8.size() - cur };
            auto push = !SkipEmpty || !tail.empty();
            if (push) crop.push_back(tail);
        }
        return crop;
    }
    template<class Container>
    auto maxlen(Container const& set)
    {
        auto len = 0_sz;
        for (auto& obj : set)
        {
            auto val = utf::length(obj);
            if (val > len) len = val;
        }
        return len;
    }
    template<class ...Args>
    auto& operator << (auto&& s, std::list<Args...> const& list)
    { 
        s << "{ ";
        for (auto delim = ""; auto& item : list) s << std::exchange(delim, ", ") << item;
        s << " }";
        return s;
    }
    template<class ...Args>
    auto concat(Args&&... args)
    {
        auto s = flux{};
        (s << ... << std::forward<Args>(args));
        return s.str();
    }
    auto base64(view utf8)
    {
        auto code = base64code;
        auto data = text{};
        if (auto size = utf8.size())
        {
            data.resize(((size + 2) / 3) << 2);
            auto iter = utf8.begin();
            auto tail = utf8.end();
            auto dest = data.begin();
            do
            {
                auto crop = (byte)*iter++ << 16;
                if (iter != tail) //todo move ifs to the outside of loop (optimization)
                {
                    crop += (byte)*iter++ << 8;
                }
                else
                {
                    *dest++ = code[0x3F & crop >> 18];
                    *dest++ = code[0x3F & crop >> 12];
                    *dest++ = '=';
                    *dest++ = '=';
                    break;
                }
                if (iter != tail)
                {
                    crop += (byte)*iter++;
                }
                else
                {
                    *dest++ = code[0x3F & crop >> 18];
                    *dest++ = code[0x3F & crop >> 12];
                    *dest++ = code[0x3F & crop >>  6];
                    *dest++ = '=';
                    break;
                }
                *dest++ = code[0x3F & crop >> 18];
                *dest++ = code[0x3F & crop >> 12];
                *dest++ = code[0x3F & crop >>  6];
                *dest++ = code[0x3F & crop      ];
            }
            while (iter != tail);
        }
        return data;
    }
    auto unbase64(view bs64, text& data)
    {
        auto code = base64code;
        auto is64 = [](auto c){ return (c > 0x2E && c < 0x3A) // '/' and digits
                                    || (c > 0x40 && c < 0x5B) // Uppercase letters
                                    || (c > 0x60 && c < 0x7B) // Lowercase letters
                                    || (c == 0x2B); };        // '+'
        auto look = view{ code };
        //todo reserv data
        if (auto size = bs64.size())
        {
            byte buff[4];
            auto head = bs64.begin();
            auto tail = head + size;
            auto step = 0;
            while (head != tail)
            {
                auto c = *head++;
                if (!is64(c) || c == '=') break;

                buff[step] = c;
                if (++step == 4)
                {
                    step = 0;
                    for (auto& a : buff) a = static_cast<byte>(look.find(a));
                    data.push_back(( buff[0]         << 2) + ((buff[1] & 0x30) >> 4));
                    data.push_back(((buff[1] & 0x0F) << 4) + ((buff[2] & 0x3C) >> 2));
                    data.push_back(((buff[2] & 0x03) << 6) +   buff[3]);
                }
            }

            if (step != 0)
            {
                auto temp = step;
                while (temp < 4) buff[temp++] = 0;
                for (auto& a : buff) a = static_cast<byte>(look.find(a));
                if (step > 1) data.push_back(( buff[0]         << 2) + ((buff[1] & 0x30) >> 4));
                if (step > 2) data.push_back(((buff[1] & 0x0F) << 4) + ((buff[2] & 0x3C) >> 2));
            }
        }
    }
    auto unbase64(view bs64)
    {
        auto data = text{};
        unbase64(bs64, data);
        return data;
    }
    // utf: Escape control chars (replace all ctrls with printables), put it to the buff, and return the buff's delta.
    template<bool Split = true, bool Multiline = true>
    auto debase(qiew utf8, text& buff)
    {
        auto mode = si32{};
        auto init = buff.size();
        auto size = utf8.size();
        capacity(buff, size * 2);
        auto head  = size - 1; // Begining with ESC is a special case.
        auto s = [&](prop const& traits, view& utf8)
        {
            switch (traits.cdpoint)
            {
                case 033:
                    if constexpr (Split)
                    {
                        if (head == utf8.size()) buff += "\\e";
                        else                     buff += "\n\\e";
                    }
                    else buff += "\\e";
                    break;
                case '\n':
                    if constexpr (Split)
                    {
                        if (utf8.size() && utf8.front() == '\033')
                        {
                            if constexpr (Multiline) buff += "\\n\n\\e";
                            else                     buff += "\\n\\e";
                            utf8.remove_prefix(1);
                            break;
                        }
                    }
                    if constexpr (Multiline) buff += "\\n\n";
                    else                     buff += "\\n";
                    break;
                case '\r': buff += "\\r"; break;
                case 8:    buff += "\\b"; break;
                case 9:    buff += "\\t"; break;
                default:
                {
                    auto cp = traits.cdpoint;
                    buff += "\\u{";
                    utf::to_hex<true>(cp, buff, cp < 0x100000 ? 4 : 8);
                    buff += "}";
                }
            }
            return utf8;
        };
        auto a = [&](view plain)
        {
            for (auto c : plain)
            {
                if (c == '\\') buff += "\\\\";
                else           buff += c;
            }
        };
        auto y = [&](frag const& cluster)
        {
                 if (cluster.text.front() == '\\') buff += "\\\\";
            else if (cluster.text.front() == '\0') buff += "\\0"sv;
            //else if (cluster.text.front() == ' ') buff += "\x20";
            else                                   buff += cluster.text;
        };
        decode<faux>(s, y, a, utf8, mode);
        return static_cast<si32>(buff.length() - init);
    }
    // utf: Return a string without control chars (replace all ctrls with printables).
    template<bool Split = true, bool Multiline = true>
    auto debase(qiew utf8)
    {
        auto buff = text{};
        debase<Split, Multiline>(utf8, buff);
        return buff;
    }
    // utf: Replace all ctrls lower than 0x20 with cp437.
    auto debase437(qiew utf8, text& buff)
    {
        if (auto code = cpit{ utf8 })
        {
            auto next = code.take();
            do
            {
                auto c = next.cdpoint;
                if (c < 0x20 || c == 0x7F) buff += *(utf::c0_view.begin() + std::min<size_t>(c, utf::c0_view.size() - 1));
                else                       buff += view(code.textptr, code.utf8len);
                code.step();
                next = code.take();
            }
            while (code);
        }
    }
    // utf: Return a string without control chars (replace all ctrls with "cp437" glyphs).
    auto debase437(qiew utf8)
    {
        auto buff = text{};
        debase437(utf8, buff);
        return buff;
    }
    // utf: Find char position ignoring backslashed.
    auto _find_char(auto head, auto tail, auto hittest)
    {
        while (head != tail)
        {
            if (hittest(head) || (*head == '\\' && ++head == tail)) break;
            ++head;
        }
        return head;
    }
    // utf: Find char position ignoring backslashed.
    template<class Iter>
    auto find_char(Iter head, Iter tail, view delims)
    {
        return _find_char(head, tail, [&](auto iter){ return delims.find(*iter) != view::npos; });
    }
    // utf: Find substring position ignoring backslashed.
    auto find_substring(view& utf8, auto... delims)
    {
        auto head = utf8.begin();
        auto tail = utf8.end();
        return _find_char(head, tail, [&](auto iter){ return (view{ iter, tail }.starts_with(delims) || ...); });
    }
    // utf: Find char position ignoring backslashed.
    template<class Iter>
    auto find_char(Iter head, Iter tail, char delim)
    {
        return _find_char(head, tail, [&](auto iter){ return *iter == delim; });
    }
    auto check_any(view shadow, view delims)
    {
        auto p = utf::find_char(shadow.begin(), shadow.end(), delims);
        return p != shadow.end();
    }
    template<class P>
    void trim_front_if(view& utf8, P pred)
    {
        auto head = utf8.begin();
        auto tail = utf8.end();
        while (head != tail)
        {
            auto c = *head;
            if (pred(c)) break;
            ++head;
        }
        utf8.remove_prefix(std::distance(utf8.begin(), head));
    }
    template<class P>
    void trim_back_if(view& utf8, P pred)
    {
        auto head = utf8.rbegin();
        auto tail = utf8.rend();
        while (head != tail)
        {
            auto c = *head;
            if (pred(c)) break;
            ++head;
        }
        utf8.remove_suffix(std::distance(utf8.rbegin(), head));
    }
    auto trim_front(view& utf8, view delims)
    {
        auto temp = utf8;
        trim_front_if(utf8, [&](char c){ return delims.find(c) == text::npos; });
        return temp.substr(0, temp.size() - utf8.size());
    }
    auto trim_front(view& utf8, char c = ' ')
    {
        auto head = utf8.begin();
        auto tail = utf8.end();
        while (head != tail && *head == c)
        {
            ++head;
        }
        utf8.remove_prefix(std::distance(utf8.begin(), head));
    }
    auto trim_front(view&& utf8, char c = ' ')
    {
        utf::trim_front(utf8, c);
        return utf8;
    }
    auto trim_back(view& utf8, view delims)
    {
        auto temp = utf8;
        trim_back_if(utf8, [&](char c){ return delims.find(c) == text::npos; });
        return temp.substr(utf8.size(), temp.size() - utf8.size());
    }
    auto trim(view utf8, char space = ' ')
    {
        while (!utf8.empty() && utf8.front() == space) utf8.remove_prefix(1);
        while (!utf8.empty() && utf8. back() == space) utf8.remove_suffix(1);
        return utf8;
    }
    auto trim(view utf8, view delims)
    {
        trim_front(utf8, delims);
        trim_back (utf8, delims);
        return utf8;
    }
    void _escape(qiew line, auto& iter, auto... x)
    {
        while (line)
        {
            auto c = line.pop_front();
            if constexpr (sizeof...(x))
            if (((c == x && (*iter++ = '\\', *iter++ = x, true))||...))
            {
                continue;
            }
            switch (c)
            {
                case '\033': *iter++ = '\\'; *iter++ = 'e' ; break;
                case   '\\': *iter++ = '\\'; *iter++ = '\\'; break;
                case   '\n': *iter++ = '\\'; *iter++ = 'n' ; break;
                case   '\r': *iter++ = '\\'; *iter++ = 'r' ; break;
                case   '\t': *iter++ = '\\'; *iter++ = 't' ; break;
                case   '\a': *iter++ = '\\'; *iter++ = 'a' ; break;
                default:     *iter++ = c; break;
            }
        }
    }
    auto _unescape(auto head, auto tail, auto& iter)
    {
        while (head != tail)
        {
            auto c = *head++;
            if (c == '\\' && head != tail)
            {
                c = *head++;
                switch (c)
                {
                    case  'e': *iter++ = '\x1b'; break;
                    case  't': *iter++ = '\t'  ; break;
                    case  'r': *iter++ = '\r'  ; break;
                    case  'n': *iter++ = '\n'  ; break;
                    case  'a': *iter++ = '\a'  ; break;
                    case '\\': *iter++ = '\\'  ; break;
                    case  'u': if (head != tail)
                    {
                        auto next = head;
                        auto d = *next;
                        auto quoted = d == '{';
                        if (quoted) ++next;
                        auto shadow = qiew{ next, tail };
                        if (auto v = to_int<si32, 16>(shadow))
                        if (auto codepoint = v.value(); codepoint >= 0 && codepoint <= 0x10FFFF)
                        if (!quoted || (shadow.size() && shadow.pop_front() == '}'))
                        {
                            _to_utf(iter, codepoint); // This expansion cannot cause overflow.
                            head = tail - shadow.size();
                            break;
                        }
                    }
                    [[fallthrough]];
                    default: *iter++ = c; break;
                }
            }
            else *iter++ = c;
        }
    }
    auto escape(qiew line, text& dest, auto... x)
    {
        dest.resize(dest.size() + line.size() * 2);
        auto iter = dest.begin();
        _escape(line, iter, x...);
        dest.resize(iter - dest.begin());
    }
    auto unescape(text& utf8) // Unescape in place.
    {
        auto iter = utf8.begin();
        _unescape(utf8.begin(), utf8.end(), iter);
        utf8.resize(iter - utf8.begin());
    }
    auto unescape(qiew utf8, text& dest) // Unescape to dest.
    {
        auto start = dest.size();
        dest.resize(start + utf8.size());
        auto iter = dest.begin() + start;
        _unescape(utf8.begin(), utf8.end(), iter);
        dest.resize(iter - dest.begin());
    }
    auto unescape(qiew utf8) // Return unescaped string.
    {
        auto dest = text{};
        unescape(utf8, dest);
        return dest;
    }
    void quote(view utf8, text& dest, char quote) // Escape, add quotes around and append the result to the dest.
    {
        auto start = dest.size();
        dest.resize(start + utf8.size() * 2 + 2);
        auto iter = dest.begin() + start;
        *iter++ = quote;
        _escape(utf8, iter, quote);
        *iter++ = quote;
        dest.resize(iter - dest.begin());
    }
    template<bool Lazy = true>
    auto take_front(view& utf8, view delims)
    {
        auto head = utf8.begin();
        auto tail = utf8.end();
        auto stop = find_char(head, tail, delims);
        if (stop == tail)
        {
            if constexpr (Lazy)
            {
                utf8 = {};
                return qiew{ utf8 };
            }
            else
            {
                auto crop = qiew{ utf8 };
                utf8 = {};
                return crop;
            }
        }
        auto str = qiew{ head, stop };
        utf8.remove_prefix(str.size());
        return str;
    }
    template<bool Lazy = true, class ...ViewList>
    auto take_front(view& utf8, std::tuple<ViewList...> const& delims)
    {
        auto head = utf8.begin();
        auto tail = utf8.end();
        auto args = std::tuple_cat(std::make_tuple(utf8), delims);
        auto stop = std::apply(find_substring<ViewList...>, args);
        if (stop == tail)
        {
            if constexpr (Lazy)
            {
                utf8 = {};
                return qiew{ utf8 };
            }
            else
            {
                auto crop = qiew{ utf8 };
                utf8 = {};
                return crop;
            }
        }
        auto str = qiew{ head, stop };
        utf8.remove_prefix(str.size());
        return str;
    }
    auto take_quote(view& utf8, char delim) // Take the fragment inside the quotes (shadow).
    {
        if (utf8.size() < 2)
        {
            utf8 = {};
            return utf8;
        }
        auto head = std::next(utf8.begin());
        auto tail = utf8.end();
        auto stop = find_char(head, tail, delim);
        if (stop == tail)
        {
            utf8 = {};
            return utf8;
        }
        auto crop = view{ head, stop };
        utf8.remove_prefix(crop.size() + 2);
        return crop;
    }
    auto get_quote(view& utf8) // Get the quoted fragment, including quotes.
    {
        if (utf8.size() < 2)
        {
            utf8 = {};
            return utf8;
        }
        auto quot = utf8.front();
        auto head = utf8.begin();
        auto tail = utf8.end();
        auto stop = find_char(head + 1, tail, quot);
        if (stop == tail)
        {
            utf8 = {};
            return utf8;
        }
        auto crop = view{ head, stop + 1 };
        utf8.remove_prefix(crop.size());
        return crop;
    }
    auto get_word(view& utf8, view delims = " ")
    {
        return take_front<faux>(utf8, delims);
    }
    // utf: Split text line into quoted tokens.
    auto tokenize(view utf8, auto&& args)
    {
        utf8 = utf::trim(utf8);
        while (utf8.size())
        {
            auto c = utf8.front();
            if (c == '\'' || c == '"') args.emplace_back(utf::unescape(utf::take_quote(utf8, c)));
            else                       args.emplace_back(utf::get_word(utf8));
            utf::trim_front(utf8);
        }
        return args;
    }
    auto eat_tail(view& utf8, view delims)
    {
        auto head = utf8.begin();
        auto tail = utf8.end();
        auto stop = find_char(head, tail, delims);
        if (stop == tail) utf8 = view{};
        else              utf8.remove_prefix(std::distance(head, stop));
    }
    template<class View>
    auto pop_front(View&& line, auto size)
    {
        auto crop = line.substr(0, size);
        line.remove_prefix(size);
        return crop;
    }
    template<class View>
    auto pop_back(View&& line, auto size)
    {
        auto crop = line.substr(line.size() - size);
        line.remove_suffix(size);
        return crop;
    }
    auto is_plain(auto&& utf8)
    {
        auto test = utf8.find('\033');
        return test == text::npos;
    }
    auto to_low(char c)
    {
        return c >= 'A' && c <= 'Z' ? (char)(c + ('a' - 'A')) : c;
    }
    auto& to_low(text& utf8, size_t size = text::npos)
    {
        auto head = utf8.begin();
        auto tail = head + std::min(utf8.size(), size);
        std::transform(head, tail, head, [](auto c){ return to_low(c); });
        return utf8;
    }
    auto to_low(text&& utf8)
    {
        return to_low(utf8);
    }
    auto& to_up(text& utf8, size_t size = text::npos)
    {
        auto head = utf8.begin();
        auto tail = head + std::min(utf8.size(), size);
        std::transform(head, tail, head, [](char c){ return c >= 'a' && c <= 'z' ? (char)(c - ('a' - 'A')) : c; });
        return utf8;
    }
    auto to_up(text&& utf8)
    {
        return to_up(utf8);
    }
    template<class W, class P>
    void for_each(text& utf8, W const& what, P proc)
    {
        auto frag = view{ what };
        auto spot = 0_sz;
        auto line_sz = utf8.length();
        auto what_sz = frag.length();

        if (!what_sz || line_sz < what_sz) return;

        while ((spot = utf8.find(frag, spot)) != text::npos)
        {
            auto fill = proc();
            utf8.replace(spot, what_sz, fill);
            spot += what_sz;
        }
    }
    auto trunc(view utf8, size_t maxy) // Returns a string trimmed at maxy lines.
    {
        auto crop = 0_sz;
        while (maxy--)
        {
            crop = utf8.find('\n', crop);
            if (crop != text::npos) crop++;
            else break;
        }
        return qiew{ utf8.substr(0, crop) };
    }
    void print2(auto& input, view& format)
    {
        input << format;
    }
    void print2(auto& input, view& format, auto&& arg, auto&&... args)
    {
        auto crop = [](view& format)
        {
            static constexpr auto delimiter = '%';
            auto crop = format;
            auto head = format.find(delimiter);
            if (head == netxs::text::npos) format = {};
            else
            {
                auto tail = format.find(delimiter, head + 1);
                if (tail != netxs::text::npos)
                {
                    crop = format.substr(0, head); // Take leading substring.
                    format.remove_prefix(tail + 1);
                }
            }
            return crop;
        };
        input << crop(format) << std::forward<decltype(arg)>(arg);
        if (format.length()) print2(input, format, std::forward<decltype(args)>(args)...);
        else                 (void)(input << ...<< std::forward<decltype(args)>(args));
    }
    template<class ...Args>
    auto fprint(view format, Args&&... args)
    {
        auto input = flux{};
        print2(input, format, std::forward<Args>(args)...);
        return input.str();
    }
}

namespace netxs
{
    using qiew = utf::qiew;
}