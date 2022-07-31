// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_UTF_HPP
#define NETXS_UTF_HPP

#include "unidata.hpp"
#include "../math/intmath.hpp"

#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <algorithm>
#include <charconv>
#include <optional>
#include <sstream>

#define GRAPHEME_CLUSTER_LIMIT (31) // Limits the number of code points in a grapheme cluster to a number sufficient for any possible linguistic situation.
#define CLUSTER_FIELD_SIZE     (5)
#define WCWIDTH_FIELD_SIZE     (2)
#define WCWIDTH_CLAMP(wcwidth) (wcwidth & (0XFF >> (8 - WCWIDTH_FIELD_SIZE)))

namespace netxs::utf
{
    using view = std::string_view;
    using text = std::string;
    using wide = std::wstring;
    using flux = std::stringstream;
    using utfx = uint32_t;
    using ctrl = unidata::cntrls::type;
    using namespace std::literals;

    static constexpr utfx        REPLACEMENT_CHARACTER = 0x0000FFFD;
    static constexpr char const* REPLACEMENT_CHARACTER_UTF8 = "\uFFFD";	// 0xEF 0xBF 0xBD (efbfbd) "�"
    static constexpr size_t	     REPLACEMENT_CHARACTER_UTF8_LEN = 3;
    static constexpr view        REPLACEMENT_CHARACTER_UTF8_VIEW = view(REPLACEMENT_CHARACTER_UTF8, REPLACEMENT_CHARACTER_UTF8_LEN); // '�'
    static constexpr view        WHITESPACE_CHARACTER_UTF8_VIEW = view(" ", 1); // ' '

    // utf: A grapheme cluster decoded from UTF-8.
    struct prop : public unidata::unidata
    {
        //todo size_t is too much for that
        size_t utf8len;
        bool   correct;
        size_t cpcount;
        utfx   cdpoint;

        constexpr
        prop(size_t size)
            : unidata (),
              utf8len { size },
              correct { faux },
              cpcount { 0    },
              cdpoint { 0    }
        { }

        prop(utfx code, size_t size)
            : unidata ( code ),
              utf8len { size },
              correct { true },
              cpcount { 0    },
              cdpoint { code }
        { }

        constexpr
        prop(prop const& attr)
            : unidata (attr),
              utf8len {attr.utf8len},
              correct {attr.correct},
              cpcount {attr.cpcount},
              cdpoint {attr.cdpoint}
        { }

        auto combine(prop const& next)
        {
            if (next.utf8len && cpcount < GRAPHEME_CLUSTER_LIMIT && next.allied(brgroup))
            {
                ucwidth  = std::max(ucwidth, next.ucwidth);
                utf8len += next.utf8len;
                cpcount += 1;

                return 0_sz;
            }
            else
            {
                return utf8len;
            }
        }
    };

    // utf: First byte based UTF-8 codepoint lengths.
    static constexpr int utf8lengths[] =
    {	//      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
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
    } ;

    // utf: Codepoint iterator.
    struct cpit
    {
        using chrptr = view::const_pointer;

        chrptr textptr;
        size_t balance;
        size_t utf8len;

        cpit(view const& utf8)
            : textptr { utf8.data() },
              balance { utf8.size() },
              utf8len { 0           }
        { }

        void redo(view const& utf8)
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
                auto data = reinterpret_cast<unsigned char const*>(textptr);
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
                        else return	prop(utf8len = balance);
                        break;
                }
            }
            else return prop(utf8len = 0);

            return prop{ cp, utf8len };
        }

        auto next()
        {
            auto cpoint = take();
                          step();
            return cpoint;
        }

        operator bool ()
        {
            return balance > 0;
        }
    };

    // utf: Grapheme cluster with properties.
    struct frag
    {
        view text;
        prop attr;
    };

    // utf: Break the text into the enriched grapheme clusters.
    //      Forward the result to the dest using the "serve" and "yield" lambdas.
    //      serve: Handle escaped control sequences.
    //             auto s = [&](utf::prop const& traits, view const& utf8) -> view;
    //      yield: Handle grapheme clusters.
    //             auto y = [&](frag const& cluster){};
    //      EGC: parse by grapheme clusters (true) or codepoints (faux)
    template<bool EGC = true, class S, class Y>
    void decode(S serve, Y yield, view utf8)
    {
        if (auto code = cpit{ utf8 })
        {
            auto next = code.take();
            do
            {
                if (next.is_cmd())
                {
                    code.step();
                    auto rest = code.rest();
                    auto chars = serve(next, rest);
                    code.redo(chars);
                    next = code.take();
                }
                else
                {
                    if constexpr (EGC)
                    {
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
                                    auto crop = frag{ view(head, size), left };
                                    yield(crop);
                                    break;
                                }
                            }
                            else
                            {
                                next.utf8len = left.utf8len;
                                auto crop = frag{ REPLACEMENT_CHARACTER_UTF8_VIEW, next };
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
                            auto crop = frag{ view(code.textptr,next.utf8len), next };
                            yield(crop);
                        }
                        else
                        {
                            auto crop = frag{ REPLACEMENT_CHARACTER_UTF8_VIEW, next };
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

    // utf: Return the first grapheme cluster and its Unicode attributes.
    static auto letter(view const& utf8)
    {
        if (auto code = cpit{ utf8 })
        {
            auto next = code.take();
            do
            {
                if (next.is_cmd())
                {
                    //todo revise
                    //code.step();
                    return frag{ REPLACEMENT_CHARACTER_UTF8_VIEW, next };
                }
                else
                {
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
                            return frag{ REPLACEMENT_CHARACTER_UTF8_VIEW, next };
                        }
                    }
                    while (true);
                }
            }
            while (code);
        }

        return frag{ REPLACEMENT_CHARACTER_UTF8_VIEW, prop{ 0 } };
    }

    struct qiew : public view
    {
        auto     pop_front () { auto c = view::front(); view::remove_prefix(1); return c; }
        si32     front     () const { return static_cast<unsigned char>(view::front()); }
        operator bool      () const { return view::length(); }
        operator text      () const { return text{ static_cast<view>(*this) }; }

        constexpr qiew() noexcept : view() { }
        constexpr qiew(view const& v) noexcept : view(v) { }
                  qiew(text const& v) noexcept : view(v) { }
        template<class INT>
        constexpr qiew(char const* ptr, INT len) noexcept: view(ptr, len) { }
        constexpr qiew& operator = (qiew const&) noexcept = default;

        // Pop front a sequence of the same control points and return their count + 1.
        auto pop_all(ctrl cmd)
        {
            auto n = si32{ 1 };
            auto next = utf::letter(*this);
            while (next.attr.control == cmd)
            {
                view::remove_prefix(next.attr.utf8len);
                next = utf::letter(*this);
                n++;
            }
            return n;
        }
        // Pop front a sequence of the same control points and return their count + 1.
        auto pop_all(char c)
        {
            auto n = si32{ 1 };
            while (length() && view::front() == c)
            {
                view::remove_prefix(1);
                n++;
            }
            return n;
        }
        // Return true and pop front control point when it is equal to cmd.
        auto pop_if(ctrl cmd)
        {
            auto next = utf::letter(*this);
            if (next.attr.control == cmd)
            {
                view::remove_prefix(next.attr.utf8len);
                return true;
            }
            return faux;
        }
        // Return true and pop front when it is equal to c.
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

    template<class A = si32, class VIEW, class = std::enable_if_t<std::is_base_of<view, VIEW>::value == true, VIEW>>
    inline std::optional<A> to_int(VIEW& ascii)
    {
        auto num = A{};
        auto top = ascii.data();
        auto end = ascii.length() + top;

        if (auto [pos, err] = std::from_chars(top, end, num); err == std::errc())
        {
            ascii.remove_prefix(pos - top);
            return num;
        }
        else return std::nullopt;
    }
    template<class A = si32, class T, class = std::enable_if_t<std::is_base_of<view, T>::value == faux, T>>
    inline auto to_int(T&& utf8)
    {
        auto shadow = view{ std::forward<T>(utf8) };
        return to_int<A>(shadow);
    }
    template<class T, class A>
    inline auto to_int(T&& utf8, A fallback)
    {
        auto result = to_int<A>(std::forward<T>(utf8));
        return result ? result.value() : fallback;
    }
    enum codepage
    {
        cp866,
        cp1251,
        count
    };

    struct letter_sync
    {
        int lb;
        int rb;
        int cp;
    };

    static constexpr letter_sync utf8_cp866[] =
    {
        { 0x0410, 0x043F, 0x80 }, // А - п
        { 0x0440, 0x044F, 0xE0 }, // р - я
        { 0x0401, 0x0401, 0xF0 }, // Ё
        { 0x0451, 0x0451, 0xF1 }, // ё
        { 0x0404, 0x0404, 0xF2 }, // Є
        { 0x0454, 0x0454, 0xF3 }, // є
        { 0x0407, 0x0407, 0xF4 }, // Ї
        { 0x0457, 0x0457, 0xF5 }, // ї
        { 0x040E, 0x040E, 0xF6 }, // Ў
        { 0x045E, 0x045E, 0xF7 }, // ў
        { 0x00B0, 0x00B0, 0xF8 }, // °
        { 0x2219, 0x2219, 0xF9 }, // ∙
        { 0x00B7, 0x00B7, 0xFA }, // ·
        { 0x221A, 0x221A, 0xFB }, // √
        { 0x2116, 0x2116, 0xFC }, // №
        { 0x00A4, 0x00A4, 0xFD }, // ¤
        { 0x25A0, 0x25A0, 0xFE }, // ■
        { 0x00A0, 0x00A0, 0xFF }, //  
        { 0x2591, 0x2591, 0xB0 }, // ░
        { 0x2592, 0x2592, 0xB1 }, // ▒
        { 0x2593, 0x2593, 0xB2 }, // ▓
        { 0x2502, 0x2502, 0xB3 }, // │
        { 0x2524, 0x2524, 0xB4 }, // ┤
        { 0x2561, 0x2561, 0xB5 }, // ╡
        { 0x2562, 0x2562, 0xB6 }, // ╢
        { 0x2556, 0x2556, 0xB7 }, // ╖
        { 0x2555, 0x2555, 0xB8 }, // ╕
        { 0x2563, 0x2563, 0xB9 }, // ╣
        { 0x2551, 0x2551, 0xBA }, // ║
        { 0x2557, 0x2557, 0xBB }, // ╗
        { 0x255D, 0x255D, 0xBC }, // ╝
        { 0x255C, 0x255C, 0xBD }, // ╜
        { 0x255B, 0x255B, 0xBE }, // ╛
        { 0x2510, 0x2510, 0xBF }, // ┐
        { 0x2514, 0x2514, 0xC0 }, // └
        { 0x2534, 0x2534, 0xC1 }, // ┴
        { 0x252C, 0x252C, 0xC2 }, // ┬
        { 0x251C, 0x251C, 0xC3 }, // ├
        { 0x2500, 0x2500, 0xC4 }, // ─
        { 0x253C, 0x253C, 0xC5 }, // ┼
        { 0x255E, 0x255E, 0xC6 }, // ╞
        { 0x255F, 0x255F, 0xC7 }, // ╟
        { 0x255A, 0x255A, 0xC8 }, // ╚
        { 0x2554, 0x2554, 0xC9 }, // ╔
        { 0x2569, 0x2569, 0xCA }, // ╩
        { 0x2566, 0x2566, 0xCB }, // ╦
        { 0x2560, 0x2560, 0xCC }, // ╠
        { 0x2550, 0x2550, 0xCD }, // ═
        { 0x256C, 0x256C, 0xCE }, // ╬
        { 0x2567, 0x2567, 0xCF }, // ╧
        { 0x2568, 0x2568, 0xD0 }, // ╨
        { 0x2564, 0x2564, 0xD1 }, // ╤
        { 0x2565, 0x2565, 0xD2 }, // ╥
        { 0x2559, 0x2559, 0xD3 }, // ╙
        { 0x2558, 0x2558, 0xD4 }, // ╘
        { 0x2552, 0x2552, 0xD5 }, // ╒
        { 0x2553, 0x2553, 0xD6 }, // ╓
        { 0x256B, 0x256B, 0xD7 }, // ╫
        { 0x256A, 0x256A, 0xD8 }, // ╪
        { 0x2518, 0x2518, 0xD9 }, // ┘
        { 0x250C, 0x250C, 0xDA }, // ┌
        { 0x2588, 0x2588, 0xDB }, // █
        { 0x2584, 0x2584, 0xDC }, // ▄
        { 0x258C, 0x258C, 0xDD }, // ▌
        { 0x2590, 0x2590, 0xDE }, // ▐
        { 0x2580, 0x2580, 0xDF }, // ▀
    };

    static constexpr letter_sync utf8_cp1251[] =
    {
        { 0x0410, 0x044F, 0xC0 }, // А - я
        { 0x0402, 0x0402, 0x80 }, // Ђ
        { 0x0403, 0x0403, 0x81 }, // Ѓ
        { 0x201A, 0x201A, 0x82 }, // ‚
        { 0x0453, 0x0453, 0x83 }, // ѓ
        { 0x201E, 0x201E, 0x84 }, // „
        { 0x2026, 0x2026, 0x85 }, // …
        { 0x2020, 0x2020, 0x86 }, // †
        { 0x2021, 0x2021, 0x87 }, // ‡
        { 0x20AC, 0x20AC, 0x88 }, // €
        { 0x2030, 0x2030, 0x89 }, // ‰
        { 0x0409, 0x0409, 0x8A }, // Љ
        { 0x2039, 0x2039, 0x8B }, // ‹
        { 0x040A, 0x040A, 0x8C }, // Њ
        { 0x040C, 0x040C, 0x8D }, // Ќ
        { 0x040B, 0x040B, 0x8E }, // Ћ
        { 0x040F, 0x040F, 0x8F }, // Џ
        { 0x0452, 0x0452, 0x90 }, // ђ
        { 0x2018, 0x2018, 0x91 }, // ‘
        { 0x2019, 0x2019, 0x92 }, // ’
        { 0x201C, 0x201C, 0x93 }, // “
        { 0x201D, 0x201D, 0x94 }, // ”
        { 0x2022, 0x2022, 0x95 }, // •
        { 0x2013, 0x2013, 0x96 }, // –
        { 0x2014, 0x2014, 0x97 }, // —
        { 0x00A0, 0x00A0, 0x98 }, //  
        { 0x2122, 0x2122, 0x99 }, // ™
        { 0x0459, 0x0459, 0x9A }, // љ
        { 0x203A, 0x203A, 0x9B }, // ›
        { 0x045A, 0x045A, 0x9C }, // њ
        { 0x045C, 0x045C, 0x9D }, // ќ
        { 0x045B, 0x045B, 0x9E }, // ћ
        { 0x045F, 0x045F, 0x9F }, // џ
        { 0x00A0, 0x00A0, 0xA0 }, //  
        { 0x040E, 0x040E, 0xA1 }, // Ў
        { 0x045E, 0x045E, 0xA2 }, // ў
        { 0x0408, 0x0408, 0xA3 }, // Ј
        { 0x00A4, 0x00A4, 0xA4 }, // ¤
        { 0x0490, 0x0490, 0xA5 }, // Ґ
        { 0x00A6, 0x00A6, 0xA6 }, // ¦
        { 0x00A7, 0x00A7, 0xA7 }, // §
        { 0x0401, 0x0401, 0xA8 }, // Ё
        { 0x00A9, 0x00A9, 0xA9 }, // ©
        { 0x0404, 0x0404, 0xAA }, // Є
        { 0x00AB, 0x00AB, 0xAB }, // «
        { 0x00AC, 0x00AC, 0xAC }, // ¬
        { 0x00AD, 0x00AD, 0xAD }, //  
        { 0x00AE, 0x00AE, 0xAE }, // ®
        { 0x0407, 0x0407, 0xAF }, // Ї
        { 0x00B0, 0x00B0, 0xB0 }, // °
        { 0x00B1, 0x00B1, 0xB1 }, // ±
        { 0x0406, 0x0406, 0xB2 }, // І
        { 0x0456, 0x0456, 0xB3 }, // і
        { 0x0491, 0x0491, 0xB4 }, // ґ
        { 0x00B5, 0x00B5, 0xB5 }, // µ
        { 0x00B6, 0x00B6, 0xB6 }, // ¶
        { 0x00B7, 0x00B7, 0xB7 }, // ·
        { 0x0451, 0x0451, 0xB8 }, // ё
        { 0x2116, 0x2116, 0xB9 }, // №
        { 0x0454, 0x0454, 0xBA }, // є
        { 0x00BB, 0x00BB, 0xBB }, // »
        { 0x0458, 0x0458, 0xBC }, // ј
        { 0x0405, 0x0405, 0xBD }, // Ѕ
        { 0x0455, 0x0455, 0xBE }, // ѕ
        { 0x0457, 0x0457, 0xBF }, // ї
    };

    //todo revise, use cp_table as an arg instead of a template parameter
    template<auto cp_table, class TEXT_OR_VIEW>
    auto cp_convert(TEXT_OR_VIEW const& utf8_text, char invalid_char)
    {
        auto crop = text{};
        auto count = sizeof(cp_table) / sizeof(letter_sync);
        auto data = utf8_text.data();
        auto size = utf8_text.size();

        for (auto i = 0UL; i < size && data[i] != 0; ++i)
        {
            auto c = data[i];
            if ((c & 0x80) == 0)
            {
                crop.push_back(c);
            }
            else if ((~c) & 0x20 && ++i < size)
            {
                auto next = data[i];
                auto code = utfx{ ((c & 0x1F) << 6) + (next & 0x3F) };
                c = invalid_char;
                for (auto j = 0UL; j < count; ++j)
                {
                    if (code >= cp_table[j].lb
                     && code <= cp_table[j].rb)
                    {
                        c = static_cast<char>(code - cp_table[j].lb + cp_table[j].cp);
                        break;
                    }
                }
                crop.push_back(c);
            }
        }
        return crop;
    }

    template<class TEXT_OR_VIEW>
    text to_dos(TEXT_OR_VIEW const& utf8, char invalid_char = '_', codepage cp = codepage::cp866)
    {
        switch (cp)
        {
            case codepage::cp866:  return cp_convert<utf8_cp866>(utf8, invalid_char);
            case codepage::cp1251: return cp_convert<utf8_cp1251>(utf8, invalid_char);
            default:               return utf8;
        }
    }

    //todo deprecate
    template<class T>
    auto to_view(T* c_str, size_t limit)
    {
        auto iter = c_str;
        auto end = c_str + limit;
        while (iter < end && *iter != 0) { ++iter; }

        return std::basic_string_view<T>(c_str, iter - c_str);
    }

    static wide to_utf(char const* utf8, size_t size)
    {
        // � The standard also recommends replacing each error with the replacement character.
        //
        // In terms of the newline, Unicode introduced U+2028 LINE SEPARATOR
        // and U+2029 PARAGRAPH SEPARATOR.
        //  c̳̻͚̻̩̻͉̯̄̏͑̋͆̎͐ͬ͑͌́͢h̵͔͈͍͇̪̯͇̞͖͇̜͉̪̪̤̙ͧͣ̓̐̓ͤ͋͒ͥ͑̆͒̓͋̑́͞ǎ̡̮̤̤̬͚̝͙̞͎̇ͧ͆͊ͅo̴̲̺͓̖͖͉̜̟̗̮̳͉̻͉̫̯̫̍̋̿̒͌̃̂͊̏̈̏̿ͧ́ͬ̌ͥ̇̓̀͢͜s̵̵̘̹̜̝̘̺̙̻̠̱͚̤͓͚̠͙̝͕͆̿̽ͥ̃͠͡

        auto wide_text = wide{};
        wide_text.reserve(size);
        auto code = utfx{};
        auto tail = utf8 + size;
        while (utf8 < tail)
        {
            auto c = static_cast<unsigned char>(*utf8++);

                 if (c < 0x80) code = c;
            else if (c < 0xc0) code =(c & 0x3f) | code << 6;
            else if (c < 0xe0) code = c & 0x1f;
            else if (c < 0xf0) code = c & 0x0f;
            else               code = c & 0x07;

            if (code <= 0x10ffff)
            {
                if (utf8 == tail || (*utf8 & 0xc0) != 0x80)
                {
                    if (code < 0xd800 || code >= 0xe000
                        || sizeof(wchar_t) > 2) // single | wchar_t == char32_t
                    {
                        wide_text.push_back(static_cast<wchar_t>(code));
                    }
                    else if (code > 0xffff) // surrogate pair
                    {
                        wide_text.append({ static_cast<wchar_t>(0xd800 + (code >> 10)),
                                           static_cast<wchar_t>(0xdc00 + (code & 0x03ff)) });
                    }
                }
            }
            else wide_text.push_back(REPLACEMENT_CHARACTER);
        }
        //wide_text.shrink_to_fit();
        return wide_text;
    }
    static inline utfx tocode(wchar_t c)
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

    namespace
    {
        static inline void _to_utf(text& utf8, utfx code)
        {
            if (code <= 0x007f)
            {
                utf8.push_back(static_cast<char>(code));
            }
            else if (code <= 0x07ff)
            {
                utf8.push_back(static_cast<char>(0xc0 | ((code >> 0x06) & 0x1f)));
                utf8.push_back(static_cast<char>(0x80 | ( code          & 0x3f)));
            }
            else if (code <= 0xffff)
            {
                utf8.push_back(static_cast<char>(0xe0 | ((code >> 0x0c) & 0x0f)));
                utf8.push_back(static_cast<char>(0x80 | ((code >> 0x06) & 0x3f)));
                utf8.push_back(static_cast<char>(0x80 | ( code          & 0x3f)));
            }
            else
            {
                utf8.push_back(static_cast<char>(0xf0 | ((code >> 0x12) & 0x07)));
                utf8.push_back(static_cast<char>(0x80 | ((code >> 0x0c) & 0x3f)));
                utf8.push_back(static_cast<char>(0x80 | ((code >> 0x06) & 0x3f)));
                utf8.push_back(static_cast<char>(0x80 | ( code          & 0x3f)));
            }
        }
    }
    static auto to_utf_from_code(utfx code)
    {
        auto utf8 = text{};
        _to_utf(utf8, code);
        return utf8;
    }
    static void to_utf_from_code(utfx code, text& utf8_out)
    {
        _to_utf(utf8_out, code);
    }
    static text to_utf(wchar_t const* wide_text, size_t size)
    {
        auto utf8 = text{};
        utf8.reserve(size << 2);
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
        //utf8.shrink_to_fit();
        return utf8;
    }
    template<class TEXT_OR_VIEW>
    auto to_utf(TEXT_OR_VIEW&& str)
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
    auto to_utf(wchar_t wc)
    {
        return to_utf(&wc, 1);
    }

    template<class TEXT_OR_VIEW>
    auto length(TEXT_OR_VIEW&& utf8)
    {
        auto length = si32{ 0 };
        for (auto c : utf8)
        {
            length += (c & 0xc0) != 0x80;
        }
        return length;
    }
    // utf: Check utf-8 integrity (last codepoint) and cut off the invalid bytes at the end.
    template<class TEXT_OR_VIEW>
    void purify(TEXT_OR_VIEW& utf8)
    {
        if (auto size = utf8.size())
        {
            auto is_first = [](auto c) { return (c & 0xc0) != 0x80; };
            auto first = faux;

            while (size && !(first = is_first(utf8[--size]))) // Find first byte.
            { }

            if (first) // Check codepoint.
            {
                auto l = utf::letter(utf8.substr(size));
                if (!l.attr.correct)
                {
                    utf8 = utf8.substr(0, size);
                }
            }
            else // Bad UTF-8 encoding (size == 0).
            {
                //Recycle all bad bytes (log?).
            }
        }
    }
    //todo deprecate cus too specific
    static si32 shrink(view& utf8)
    {
        auto length = si32{ 0 };
        auto size = utf8.size();
        auto i = 0_sz;

        while (i < size)
        {
            if ((utf8[i] & 0xc0) != 0x80)
            {
                if (utf8[i] > 0 && utf8[i] < 32)
                {
                    break;
                }
                length++;
            }
            i++;
        }
        utf8.remove_suffix(size - i);
        return length;
    }

    template<class TEXT_OR_VIEW>
    auto substr(TEXT_OR_VIEW&& utf8, size_t start, size_t length = text::npos)
    {
        if (length == 0) return text{};

        auto data = text{ utf8 };
        auto head = data.data();
        auto stop = head + data.size();
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

        start = head - data.data();
        if (length != text::npos)
        {
            auto tail = head;
            calc(tail, length, stop);
            return data.substr(start, tail - head);
        }
        else return data.substr(start);
    }

    template<class TEXT_OR_VIEW>
    auto repeat(TEXT_OR_VIEW&& utf8, size_t count)
    {
        auto what = view{ utf8 };
        auto result = text{};
        result.reserve(what.length() * count);
        while (count--)
        {
            result += what;
        }
        return result;
    }
    auto inline repeat(char letter, size_t count)
    {
        return text(count, letter);
    }

    template<class TEXT_OR_VIEW, class C>
    auto remove(TEXT_OR_VIEW&& from, C&& what)
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

    template<class W, class R>
    static void change(text& utf8, W const& what, R const& replace)
    {
        auto frag = view{ what };
        auto fill = view{ replace };

        auto const& npos = text::npos;
        auto spot = 0_sz;
        auto line_sz = utf8.length();
        auto what_sz = frag.length();
        auto repl_sz = fill.length();

        if (!what_sz || line_sz < what_sz) return;

        if (what_sz == repl_sz)
        {
            while ((spot = utf8.find(frag, spot)) != npos)
            {
                utf8.replace(spot, what_sz, fill);
                spot += what_sz;
            }
        }
        else
        {
            auto last = 0_sz;
            if (what_sz < repl_sz)
            {
                auto temp = text{};
                temp.reserve((line_sz / what_sz + 1) * repl_sz); // In order to avoid allocations.
                auto shadow = view{ utf8 };
                while ((spot = utf8.find(frag, last)) != npos)
                {
                    temp += shadow.substr(last, spot - last);
                    temp += fill;
                    spot += what_sz;
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
                while ((spot = utf8.find(frag, last)) != npos)
                {
                    if (last) copy(base + last, dest, spot - last);
                    else      dest += spot;

                    copy(repl, dest, repl_sz);

                    spot += what_sz;
                    last  = spot;
                }

                copy(base + last, dest, line_sz - last);

                utf8.resize(dest - base);
            }
        }
    }

    template<class TEXT_OR_VIEW, class T>
    auto remain(TEXT_OR_VIEW&& utf8, T const& delimiter)
    {
        auto crop = std::remove_cvref_t<TEXT_OR_VIEW>{};
        auto what = view{ delimiter };
        auto coor = utf8.find(what);
        if (coor != text::npos)
        {
            crop = utf8.substr(coor + what.size(), text::npos);
        }
        return crop;
    }
    template<class TEXT_OR_VIEW>
    auto remain(TEXT_OR_VIEW&& utf8, char delimiter = '.')
    {
        auto what = view{ &delimiter, 1 };
        return remain(std::forward<TEXT_OR_VIEW>(utf8), what);
    }

    // utf: Return left substring (from begin) until delimeter (lazy=faux: from left, true: from right).
    template<class T>
    T cutoff(T const& txt, T const& delimiter = T{ '.' }, bool lazy = true)
    {
        return txt.substr(0, lazy ? txt.find(delimiter) : txt.rfind(delimiter));
    }
    template<class T>
    T cutoff(T const& txt, char delimiter, bool lazy = true)
    {
        return txt.substr(0, lazy ? txt.find(delimiter) : txt.rfind(delimiter));
    }
    template<class T>
    inline T domain(T const& txt)
    {
        return remain(txt);
    }

    template<class TEXT_OR_VIEW, class F>
    auto adjust(TEXT_OR_VIEW&& utf8, size_t required_width, F const& fill_char, bool right_aligned = faux)
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
            auto size = length(crop);
            if (required_width > size)
            {
                if (right_aligned) crop = repeat(fill_char, required_width - size) + crop;
                else               crop = crop + repeat(fill_char, required_width - size);
            }
        }
        return crop;
    }

    template<class INT_T, class T = char>
    text format(INT_T number, size_t by_group = 3, T const& delimiter = ' ')
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

    template <bool UCASE = faux, class V, class = typename std::enable_if<std::is_integral<V>::value>::type>
    auto to_hex(V number, size_t width = sizeof(V) * 2, char filler = '0')
    {
        static constexpr auto nums = UCASE ? "0123456789ABCDEF"
                                           : "0123456789abcdef";
        auto crop = text(width, filler);
        auto head = crop.begin();
        auto tail = head + width;
        auto part = -4 + 4*width;
        while (head != tail)
        {
            *head++ = nums[(number >> part) & 0x0f];
             part  -= 4;
        }
        return crop;
    }
    // utf: to_hex without allocations (the crop should has a reserved capacity).
    template <bool UCASE = faux, class V, class = typename std::enable_if<std::is_integral<V>::value>::type>
    auto to_hex(text& crop, V number, size_t width = sizeof(V) * 2, char filler = '0')
    {
        static constexpr auto nums = UCASE ? "0123456789ABCDEF"
                                           : "0123456789abcdef";
        auto part = -4 + 4*width;
        while (width--)
        {
            crop.push_back(nums[(number >> part) & 0x0f]);
            part  -= 4;
        }
        return crop;
    }
    template <bool UCASE = faux>
    auto to_hex(view buffer, bool formatted = faux)
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

                    crop += i < frag ? ' ' + to_hex<UCASE>(buffer[addr + i], 2, true)
                                     : "   ";
                }

                crop += "   ";

                for (auto i = addr; i < addr + frag; i++)
                {
                    auto c = buffer[i];
                    crop += (c < 33) ? '.' : c;
                }

                crop += '\n';
                addr += 16;
            }
            return crop;
        }
        else
        {
            static constexpr auto nums = UCASE ? "0123456789ABCDEF"
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

    template<class V1, class P>
    auto divide(V1 const& utf8, char delimiter, P proc)
    {
        auto cur = 0_sz;
        auto pos = 0_sz;
        while ((pos = utf8.find(delimiter, cur)) != V1::npos)
        {
            proc(view{ utf8.data() + cur, pos - cur });
            cur = pos + 1;
        }
        auto end = view{ utf8.data() + cur, utf8.size() - cur };
        proc(end);
    }
    template<class V1, class V2>
    auto divide(V1 const& utf8, V2 const& delimiter)
    {
        auto mark = text(delimiter);
        auto crop = std::vector<view>{};

        if (auto len = mark.size())
        {
            auto num = 0_sz;
            auto cur = 0_sz;
            auto pos = 0_sz;
            while ((pos = utf8.find(mark, cur)) != V1::npos)
            {
                ++num;
                cur = pos + len;
            }
            crop.reserve(++num);
            cur = 0_sz;
            pos = 0_sz;
            while ((pos = utf8.find(mark, cur)) != V1::npos)
            {
                crop.push_back(view{ utf8.data() + cur, pos - cur });
                cur = pos + len;
            }
            auto end = view{ utf8.data() + cur, utf8.size() - cur };
            crop.push_back(end);
        }
        return crop;
    }

    template <class Container>
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

    class filler
    {
        std::map<text, text> dict;

    public:
        auto& operator [] (text const& s)
        {
            return dict[s];
        }
        auto operator () (text s)
        {
            for (auto& var : dict) utf::change(s, var.first, var.second);
            for (auto& var : dict) utf::change(s, var.first, var.second);
            return s;
        }
    };

    namespace
    {
        template <class T>
        void _concat(flux& s, T&& item)
        {
            s << item;
        }
        template<class T, class ...Args>
        void _concat(flux& s, T&& item, Args&&... args)
        {
            s << item;
            _concat(s, std::forward<Args>(args)...);
        }
    }
    template<class ...Args>
    auto concat(Args&&... args)
    {
        auto s = flux{};
        _concat(s, std::forward<Args>(args)...);
        return s.str();
    }

    auto base64(view utf8)
    {
        static constexpr auto code = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        auto data = text{};
        if (auto size = utf8.size())
        {
            data.resize(((size + 2) / 3) << 2);
            auto iter = utf8.begin();
            auto tail = utf8.end();
            auto dest = data.begin();
            do
            {
                auto crop = (unsigned char)*iter++ << 16;
                if (iter != tail) //todo move ifs to the outside of loop (optimization)
                {
                    crop += (unsigned char)*iter++ << 8;
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
                    crop += (unsigned char)*iter++;
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

    auto unbase64(view bs64)
    {
        static constexpr auto code = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        auto is64 = [](auto c) { return (c > 0x2E && c < 0x3A) // '/' and digits
                                     || (c > 0x40 && c < 0x5B) // Uppercase letters
                                     || (c > 0x60 && c < 0x7B) // Lowercase letters
                                     || (c == 0x2B); };        // '+'
        auto data = text{};
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
        return data;
    }

    // utf: Return a string without control chars (replace all ctrls with printables).
    template<bool SPLIT = true>
    auto debase(view utf8)
    {
        auto buff = text{};
        auto size = utf8.size();
        buff.reserve(size * 2);
        auto head  = size - 1; // Begining with ESC is a special case.
        auto s = [&](prop const& traits, view& utf8)
        {
            switch (traits.cdpoint)
            {
                case 033:
                    if constexpr (SPLIT)
                    {
                        if (head == utf8.size()) buff += "\\e";
                        else                     buff += "\n\\e";
                    }
                    else buff += "\\e";
                    break;
                case '\n':
                    if constexpr (SPLIT)
                    {
                        if (utf8.size() && utf8.front() == '\033')
                        {
                            buff += "\\n\n\\e";
                            utf8.remove_prefix(1);
                        }
                        else buff += "\\n\n";
                    }
                    else buff += "\\n\n";
                    break;
                case '\r': buff += "\\r"; break;
                case 8:    buff += "\\b"; break;
                case 9:    buff += "\\t"; break;
                default:
                {
                    auto cp = traits.cdpoint;
                    if (cp < 0x100000) { buff += "\\u"; to_hex<true>(buff, cp, 4); }
                    else               { buff += "\\U"; to_hex<true>(buff, cp, 8); }
                }
            }
            return utf8;
        };
        auto y = [&](frag const& cluster)
        {
            if (cluster.text.front() == '\\')     buff += "\\\\";
            //else if (cluster.text.front() == ' ') buff += "\x20";
            else                                  buff += cluster.text;
        };
        decode<faux>(s, y, utf8);

        return buff;
    }

    template<class ITER>
    auto find_char(ITER head, ITER tail, view delims)
    {
        while (head != tail)
        {
            auto c = *head;
                 if (delims.find(c) != view::npos) break;
            else if (c == '\\' && head != tail) ++head;
            ++head;
        }
        return head;
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
    void trim_front(view& utf8, view delims)
    {
        trim_front_if(utf8, [&](char c){ return delims.find(c) == text::npos; });
    }
    auto trim(view utf8, char space = ' ')
    {
        while (!utf8.empty() && utf8.front() == space) utf8.remove_prefix(1);
        while (!utf8.empty() && utf8. back() == space) utf8.remove_suffix(1);
        return utf8;
    }
    auto get_quote(view& utf8, view delims, view skip = {})
    {
        auto head = utf8.begin();
        auto tail = utf8.end();
        auto coor = find_char(head, tail, delims);
        if (std::distance(coor, tail) < 2)
        {
            utf8 = view{};
            return text{};
        }
        ++coor;
        auto stop = find_char(coor, tail, delims);
        if (stop == tail)
        {
            utf8 = view{};
            return text{};
        }
        utf8.remove_prefix(std::distance(head, stop) + 1);
        auto str = text{ coor, stop };
        if (!skip.empty()) trim_front(utf8, skip);
        return str;
    }
    auto get_tail(view& utf8, view delims)
    {
        auto head = utf8.begin();
        auto tail = utf8.end();
        auto stop = find_char(head, tail, delims);
        if (stop == tail)
        {
            utf8 = view{};
            return text{};
        }
        auto str = text{ head, stop };
        utf8.remove_prefix(std::distance(head, stop));
        return str;
    }
    template<class TEXT_or_VIEW>
    auto is_plain(TEXT_or_VIEW&& utf8)
    {
        auto test = utf8.find('\033');
        return test == text::npos;
    }
    auto& to_low(text& utf8, size_t size = text::npos)
    {
        auto head = utf8.begin();
        auto tail = head + std::min(utf8.size(), size);
        std::transform(head, tail, head, [](unsigned char c) { return c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c; });
        return utf8;
    }
    auto& to_up(text& utf8, size_t size = text::npos)
    {
        auto head = utf8.begin();
        auto tail = head + std::min(utf8.size(), size);
        std::transform(head, tail, head, [](unsigned char c) { return c >= 'a' && c <= 'z' ? c - ('a' - 'A') : c; });
        return utf8;
    }
}

#endif // NETXS_UTF_HPP