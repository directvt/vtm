// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

#include "quartz.hpp"
#include "unidata.hpp"

namespace netxs
{
    using view = std::string_view;
    using text = std::string;
    using wide = std::wstring;
    using wiew = std::wstring_view;
    using wchr = wchar_t;
    using flux = std::stringstream;
    using utfx = uint32_t;
    using txts = std::vector<text>;
    using namespace std::literals;

    static constexpr auto whitespaces = " \t\r\n\v\f"sv;
    static constexpr auto onlydigits  = "0123456789"sv;
    static constexpr auto sharpdigit  = "0123456789#"sv;
    static constexpr auto alphabetic  = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"sv;
    static constexpr auto base64code  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    static constexpr auto whitespace  = ' '; // '.';
    static constexpr auto emptyspace  = "\0"sv; //"\xC0\x80"sv; // In Modified UTF-8, the null character (U+0000) uses the two-byte overlong encoding 11000000 10000000 (hexadecimal C0 80), instead of 00000000 (hexadecimal 00).
    static consteval auto make_ui32(view four_bytes){ return ((ui32)four_bytes[0] << 24)
                                                           | ((ui32)four_bytes[1] << 16)
                                                           | ((ui32)four_bytes[2] << 8)
                                                           | ((ui32)four_bytes[3] << 0); }
}

namespace netxs::utf
{
    using ctrl = unidata::cntrls;

    template<size_t N>
    struct _str2array
    {
        std::array<char, N - 1> array;

        template<auto... I>
        static constexpr auto make_array(auto& arr, std::index_sequence<I...>)
        {
            return std::array<char, sizeof...(I)>{ arr[I]... };
        }
        constexpr _str2array(char const (&str)[N]) //constexpr _str2array(auto&& str)
            : array{ make_array(str, std::make_index_sequence<sizeof(str) - 1>{}) }
        { }
        constexpr _str2array(std::array<char, N - 1> const& str)
            : array{ str }
        { }
    };
    template<auto M>
    _str2array(std::array<char, M>) -> _str2array<M + 1>;
    // Returns an array of concatenated string literals.
    constexpr auto cat(auto&&... strs) // (const char (&...strs)[N])
    {
        return std::apply([](auto... c){ return std::array<char, sizeof...(c)>{ c... }; }, std::tuple_cat(std::tuple_cat(utf::_str2array{ strs }.array)...));
    }

    static constexpr auto c0_view = std::to_array({ "·"sv, "☺"sv, "☻"sv, "♥"sv, "♦"sv, "♣"sv, "♠"sv, "•"sv, "◘"sv, "○"sv, "◙"sv, "♂"sv, "♀"sv, "♪"sv, "♫"sv, "☼"sv,
                                                    "►"sv, "◄"sv, "↕"sv, "‼"sv, "¶"sv, "§"sv, "▬"sv, "↨"sv, "↑"sv, "↓"sv, "→"sv, "←"sv, "∟"sv, "↔"sv, "▲"sv, "▼"sv,
                                                    "⌂"sv });
    static constexpr auto c0_wchr = std::to_array({ L'\0',L'☺', L'☻', L'♥', L'♦', L'♣', L'♠', L'•', L'◘', L'○', L'◙', L'♂', L'♀', L'♪', L'♫', L'☼',
                                                    L'►', L'◄', L'↕', L'‼', L'¶', L'§', L'▬', L'↨', L'↑', L'↓', L'→', L'←', L'∟', L'↔', L'▲', L'▼',
                                                    L'⌂' });
    static constexpr auto ext_437 = std::to_array({ "Ç"sv, "ü"sv, "é"sv, "â"sv, "ä"sv, "à"sv, "å"sv, "ç"sv, "ê"sv, "ë"sv, "è"sv, "ï"sv, "î"sv, "ì"sv, "Ä"sv, "Å"sv,
                                                    "É"sv, "æ"sv, "Æ"sv, "ô"sv, "ö"sv, "ò"sv, "û"sv, "ù"sv, "ÿ"sv, "Ö"sv, "Ü"sv, "¢"sv, "£"sv, "¥"sv, "₧"sv, "ƒ"sv,
                                                    "á"sv, "í"sv, "ó"sv, "ú"sv, "ñ"sv, "Ñ"sv, "ª"sv, "º"sv, "¿"sv, "⌐"sv, "¬"sv, "½"sv, "¼"sv, "¡"sv, "«"sv, "»"sv,
                                                    "░"sv, "▒"sv, "▓"sv, "│"sv, "┤"sv, "╡"sv, "╢"sv, "╖"sv, "╕"sv, "╣"sv, "║"sv, "╗"sv, "╝"sv, "╜"sv, "╛"sv, "┐"sv,
                                                    "└"sv, "┴"sv, "┬"sv, "├"sv, "─"sv, "┼"sv, "╞"sv, "╟"sv, "╚"sv, "╔"sv, "╩"sv, "╦"sv, "╠"sv, "═"sv, "╬"sv, "╧"sv,
                                                    "╨"sv, "╤"sv, "╥"sv, "╙"sv, "╘"sv, "╒"sv, "╓"sv, "╫"sv, "╪"sv, "┘"sv, "┌"sv, "█"sv, "▄"sv, "▌"sv, "▐"sv, "▀"sv,
                                                    "α"sv, "ß"sv, "Γ"sv, "π"sv, "Σ"sv, "σ"sv, "µ"sv, "τ"sv, "Φ"sv, "Θ"sv, "Ω"sv, "δ"sv, "∞"sv, "φ"sv, "ε"sv, "∩"sv,
                                                    "≡"sv, "±"sv, "≥"sv, "≤"sv, "⌠"sv, "⌡"sv, "÷"sv, "≈"sv, "°"sv, "∙"sv, "·"sv, "√"sv, "ⁿ"sv, "²"sv, "■"sv, "·"sv, });
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

    static constexpr auto fc_LRE_code = utfx{ 0x0000'202A }; // Format characters 202A..202E
    static constexpr auto fc_RLE_code = utfx{ 0x0000'202B }; //
    static constexpr auto fc_PDF_code = utfx{ 0x0000'202C }; //
    static constexpr auto fc_LRO_code = utfx{ 0x0000'202D }; //
    static constexpr auto fc_RLO_code = utfx{ 0x0000'202E }; //

    static constexpr auto fc_LRI_code = utfx{ 0x0000'2066 }; // Format characters 2066..2069
    static constexpr auto fc_RLI_code = utfx{ 0x0000'2067 }; //
    static constexpr auto fc_FSI_code = utfx{ 0x0000'2068 }; //
    static constexpr auto fc_PDI_code = utfx{ 0x0000'2069 }; //

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
        static constexpr auto x_bits = (byte)0b0001'1111; // Character geometry x fragment selector bits (for mosaic_mask).
        static constexpr auto y_bits = 5; // y-fragment selector bits offset.
        template<si32 xxy>
        static auto mosaic = []{ return xxy / 10 + ((xxy % 10) << y_bits); }();
        static auto p = [](auto x){ return x * (x + 1) / 2; }; // ref: https://github.com/directvt/vtm/assets/11535558/88bf5648-533e-4786-87de-b3dc4103273c
        static constexpr auto stx = utfx{ 0x02 }; // Custom cluster initiator (STX).
        static constexpr auto kx = 16;
        static constexpr auto ky = 4;
        static constexpr auto mx = p(kx + 1);
        static constexpr auto my = p(ky + 1);
        static auto s = [](auto w, auto h, auto x, auto y){ return p(w) + x + (p(h) + y) * mx; };
        template<si32 wwh, si32 xxy = 0>
        static constexpr auto vs = [] // wwh: (00 0 -> 16 4); xxy: (00 0 -> 16 4)
        {
            auto w = wwh / 10;
            auto h = wwh % 10;
            auto x = xxy / 10;
            auto y = xxy % 10;
            auto v = p(w) + x + (p(h) + y) * mx;
            return v;
        }();
        static constexpr auto vs_block = 0xD0000;
        template<si32 wwh, si32 xxy = 0>
        static constexpr auto vs_code = vs_block + vs<wwh, xxy>;
        template<si32 wwh, si32 xxy = 0, auto code = vs_code<wwh, xxy>>
        static constexpr auto vss = utf8view<code>;
        static constexpr auto min_vs_code = vs_code<00, 00>;
        static constexpr auto max_vs_code = vs_code<164, 164>;

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

    bool non_control(utfx code)
    {
        auto is_not_control =
        //nul                                       , //   0 0x0   NULL
        //soh                                       , //   1 0x1   START OF HEADING
        //stx                                       , //   2 0x2   START OF TEXT
        //etx                                       , //   3 0x3   END OF TEXT
        //eot                                       , //   4 0x4   END OF TRANSMISSION
        //enq                                       , //   5 0x5   ENQUIRY
        //ack                                       , //   6 0x6   ACKNOWLEDGE
        //bel                                       , //   7 0x7   ALERT
        //bs                                        , //   8 0x8   BACKSPACE
        //tab                                       , //   9 0x9   CHARACTER TABULATION
        //eol                                       , //  10 0xA   LINE FEED
        //vt                                        , //  11 0xB   LINE TABULATION
        //ff                                        , //  12 0xC   FORM FEED
        //cr                                        , //  13 0xD   CARRIAGE RETURN
        //so                                        , //  14 0xE   SHIFT OUT
        //si                                        , //  15 0xF   SHIFT IN
        //dle                                       , //  16 0x10  DATA LINK ESCAPE
        //dc1                                       , //  17 0x11  DEVICE CONTROL ONE
        //dc2                                       , //  18 0x12  DEVICE CONTROL TWO
        //dc3                                       , //  19 0x13  DEVICE CONTROL THREE
        //dc4                                       , //  20 0x14  DEVICE CONTROL FOUR
        //nak                                       , //  21 0x15  NEGATIVE ACKNOWLEDGE
        //syn                                       , //  22 0x16  SYNCHRONOUS IDLE
        //etb                                       , //  23 0x17  END OF TRANSMISSION BLOCK
        //can                                       , //  24 0x18  CANCEL
        //em                                        , //  25 0x19  END OF MEDIUM
        //sub                                       , //  26 0x1A  SUBSTITUTE
        //esc                                       , //  27 0x1B  ESCAPE
        //fs                                        , //  28 0x1C  INFORMATION SEPARATOR FOUR
        //gs                                        , //  29 0x1D  INFORMATION SEPARATOR THREE
        //rs                                        , //  30 0x1E  INFORMATION SEPARATOR TWO
        //us                                        , //  31 0x1F  INFORMATION SEPARATOR ONE
        (code > 0x1Fu && code < 0x7Fu)
        //del                                       , //  32 0x7F  DELETE
        //pad                                       , //  33 0x80  PADDING CHARACTER
        //hop                                       , //  34 0x81  HIGH OCTET PRESET
        //bph                                       , //  35 0x82  BREAK PERMITTED HERE
        //nbh                                       , //  36 0x83  NO BREAK HERE
        //ind                                       , //  37 0x84  INDEX
        //nel                                       , //  38 0x85  NEXT LINE
        //ssa                                       , //  39 0x86  START OF SELECTED AREA
        //esa                                       , //  40 0x87  END OF SELECTED AREA
        //hts                                       , //  41 0x88  CHARACTER TABULATION SET
        //htj                                       , //  42 0x89  CHARACTER TABULATION WITH JUSTIFICATION
        //vts                                       , //  43 0x8A  LINE TABULATION SET
        //pld                                       , //  44 0x8B  PARTIAL LINE FORWARD
        //plu                                       , //  45 0x8C  PARTIAL LINE BACKWARD
        //ri                                        , //  46 0x8D  REVERSE LINE FEED
        //ss2                                       , //  47 0x8E  SINGLE SHIFT TWO
        //ss3                                       , //  48 0x8F  SINGLE SHIFT THREE
        //dcs                                       , //  49 0x90  DEVICE CONTROL STRING
        //pu1                                       , //  50 0x91  PRIVATE USE ONE
        //pu2                                       , //  51 0x92  PRIVATE USE TWO
        //sts                                       , //  52 0x93  SET TRANSMIT STATE
        //cch                                       , //  53 0x94  CANCEL CHARACTER
        //mw                                        , //  54 0x95  MESSAGE WAITING
        //spa                                       , //  55 0x96  START OF GUARDED AREA
        //epa                                       , //  56 0x97  END OF GUARDED AREA
        //sos                                       , //  57 0x98  START OF STRING
        //sgc                                       , //  58 0x99  SINGLE GRAPHIC CHARACTER INTRODUCER
        //sci                                       , //  59 0x9A  SINGLE CHARACTER INTRODUCER
        //csi                                       , //  60 0x9B  CONTROL SEQUENCE INTRODUCER
        //st                                        , //  61 0x9C  STRING TERMINATOR
        //osc                                       , //  62 0x9D  OPERATING SYSTEM COMMAND
        //pm                                        , //  63 0x9E  PRIVACY MESSAGE
        //apc                                       , //  64 0x9F  APPLICATION PROGRAM COMMAND
        || (code > 0x9Fu
        //paragraph_separator                       , //  65 0x2029 PARAGRAPH SEPARATOR
        //non_control                               , //  66 -1    NON CONTROL
        && code != 0xADu
        //shy                                       , //  67 0xAD  SOFT HYPHEN
        ////alm                                       , //  68 0x61C ARABIC LETTER MARK
        && code != 0x180Eu
        //mvs                                       , //  69 0x180E MONGOLIAN VOWEL SEPARATOR
        ////lrm                                       , //  70 0x200E LEFT-TO-RIGHT MARK
        ////rlm                                       , //  71 0x200F RIGHT-TO-LEFT MARK
        && code < 0x2028u)
        //line_separator                            , //  72 0x2028 LINE SEPARATOR
        //lre                                       , //  73 0x202A LEFT-TO-RIGHT EMBEDDING
        //rle                                       , //  74 0x202B RIGHT-TO-LEFT EMBEDDING
        //pdf                                       , //  75 0x202C POP DIRECTIONAL FORMATTING
        //lro                                       , //  76 0x202D LEFT-TO-RIGHT OVERRIDE
        //rlo                                       , //  77 0x202E RIGHT-TO-LEFT OVERRIDE
        || (code > 0x202Eu && code < 0x2060u)
        //wj                                        , //  78 0x2060 WORD JOINER
        //function_application                      , //  79 0x2061 FUNCTION APPLICATION
        //invisible_times                           , //  80 0x2062 INVISIBLE TIMES
        //invisible_separator                       , //  81 0x2063 INVISIBLE SEPARATOR
        //invisible_plus                            , //  82 0x2064 INVISIBLE PLUS
        //lri                                       , //  83 0x2066 LEFT-TO-RIGHT ISOLATE
        //rli                                       , //  84 0x2067 RIGHT-TO-LEFT ISOLATE
        //fsi                                       , //  85 0x2068 FIRST STRONG ISOLATE
        //pdi                                       , //  86 0x2069 POP DIRECTIONAL ISOLATE
        //inhibit_symmetric_swapping                , //  87 0x206A INHIBIT SYMMETRIC SWAPPING
        //activate_symmetric_swapping               , //  88 0x206B ACTIVATE SYMMETRIC SWAPPING
        //inhibit_arabic_form_shaping               , //  89 0x206C INHIBIT ARABIC FORM SHAPING
        //activate_arabic_form_shaping              , //  90 0x206D ACTIVATE ARABIC FORM SHAPING
        //national_digit_shapes                     , //  91 0x206E NATIONAL DIGIT SHAPES
        //nominal_digit_shapes                      , //  92 0x206F NOMINAL DIGIT SHAPES
        || (code > 0x206Fu && code != 0xFEFFu
        //zwnbsp                                    , //  93 0xFEFF ZERO WIDTH NO-BREAK SPACE
        && code < 0xFFF9u)
        //interlinear_annotation_anchor             , //  94 0xFFF9 INTERLINEAR ANNOTATION ANCHOR
        //interlinear_annotation_separator          , //  95 0xFFFA INTERLINEAR ANNOTATION SEPARATOR
        //interlinear_annotation_terminator         , //  96 0xFFFB INTERLINEAR ANNOTATION TERMINATOR
        || (code > 0xFFFBu && code < 0x13430u)
        //egyptian_hieroglyph_vertical_joiner       , //  97 0x13430 EGYPTIAN HIEROGLYPH VERTICAL JOINER
        //egyptian_hieroglyph_horizontal_joiner     , //  98 0x13431 EGYPTIAN HIEROGLYPH HORIZONTAL JOINER
        //egyptian_hieroglyph_insert_at_top_start   , //  99 0x13432 EGYPTIAN HIEROGLYPH INSERT AT TOP START
        //egyptian_hieroglyph_insert_at_bottom_start, // 100 0x13433 EGYPTIAN HIEROGLYPH INSERT AT BOTTOM START
        //egyptian_hieroglyph_insert_at_top_end     , // 101 0x13434 EGYPTIAN HIEROGLYPH INSERT AT TOP END
        //egyptian_hieroglyph_insert_at_bottom_end  , // 102 0x13435 EGYPTIAN HIEROGLYPH INSERT AT BOTTOM END
        //egyptian_hieroglyph_overlay_middle        , // 103 0x13436 EGYPTIAN HIEROGLYPH OVERLAY MIDDLE
        //egyptian_hieroglyph_begin_segment         , // 104 0x13437 EGYPTIAN HIEROGLYPH BEGIN SEGMENT
        //egyptian_hieroglyph_end_segment           , // 105 0x13438 EGYPTIAN HIEROGLYPH END SEGMENT
        //egyptian_hieroglyph_insert_at_middle      , // 106 0x13439 EGYPTIAN HIEROGLYPH INSERT AT MIDDLE
        //egyptian_hieroglyph_insert_at_top         , // 107 0x1343A EGYPTIAN HIEROGLYPH INSERT AT TOP
        //egyptian_hieroglyph_insert_at_bottom      , // 108 0x1343B EGYPTIAN HIEROGLYPH INSERT AT BOTTOM
        //egyptian_hieroglyph_begin_enclosure       , // 109 0x1343C EGYPTIAN HIEROGLYPH BEGIN ENCLOSURE
        //egyptian_hieroglyph_end_enclosure         , // 110 0x1343D EGYPTIAN HIEROGLYPH END ENCLOSURE
        //egyptian_hieroglyph_begin_walled_enclosure, // 111 0x1343E EGYPTIAN HIEROGLYPH BEGIN WALLED ENCLOSURE
        //egyptian_hieroglyph_end_walled_enclosure  , // 112 0x1343F EGYPTIAN HIEROGLYPH END WALLED ENCLOSURE
        || (code > 0x1343Fu && code < 0x1BCA0u)
        //shorthand_format_letter_overlap           , // 113 0x1BCA0 SHORTHAND FORMAT LETTER OVERLAP
        //shorthand_format_continuing_overlap       , // 114 0x1BCA1 SHORTHAND FORMAT CONTINUING OVERLAP
        //shorthand_format_down_step                , // 115 0x1BCA2 SHORTHAND FORMAT DOWN STEP
        //shorthand_format_up_step                  , // 116 0x1BCA3 SHORTHAND FORMAT UP STEP
        || (code > 0x1BCA3u && code < 0x1D173u)
        //musical_symbol_begin_beam                 , // 117 0x1D173 MUSICAL SYMBOL BEGIN BEAM
        //musical_symbol_end_beam                   , // 118 0x1D174 MUSICAL SYMBOL END BEAM
        //musical_symbol_begin_tie                  , // 119 0x1D175 MUSICAL SYMBOL BEGIN TIE
        //musical_symbol_end_tie                    , // 120 0x1D176 MUSICAL SYMBOL END TIE
        //musical_symbol_begin_slur                 , // 121 0x1D177 MUSICAL SYMBOL BEGIN SLUR
        //musical_symbol_end_slur                   , // 122 0x1D178 MUSICAL SYMBOL END SLUR
        //musical_symbol_begin_phrase               , // 123 0x1D179 MUSICAL SYMBOL BEGIN PHRASE
        //musical_symbol_end_phrase                 , // 124 0x1D17A MUSICAL SYMBOL END PHRASE
        || code > 0x1D17Au;
        return is_not_control;
    }
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

        // prop: Check if the next codepooint could be attached to the cluster. Return zero to continue attaching. Return non-zero if cluster is closed.
        auto combine(prop const& next)
        {
            if (next.cdpoint && next.utf8len) // The codepoint '\0' cannot be a cluster fragment.
            {
                if (unidata::allied(next))
                {
                    if (next.cdpoint >= matrix::min_vs_code && next.cdpoint <= matrix::max_vs_code) // Set matrix size.
                    {
                        cmatrix = (si32)(next.cdpoint - matrix::vs_block);
                        // Drop the next.cdpoint by returning 0_sz.
                        //todo no more codepoints should be added (matrix modifier has the gbreak::ext property).
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
                else if (unidata::ucwidth == 0 && cdpoint && !next.is_cmd()) // Append any non-control code point if the current cluster has no width.
                {
                    if (next.ucwidth > unidata::ucwidth)
                    {
                        unidata::ucwidth = next.ucwidth;
                        cmatrix = mtx[unidata::ucwidth];
                    }
                    utf8len += next.utf8len;
                    cpcount += 1;
                    return 0_sz;
                }
            }
            return utf8len;
        }
    };

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
                // First byte based UTF-8 codepoint lengths.
                if (cp < 0x80) // : len = 1
                {
                    utf8len = 1U;
                }
                else if (cp < 0xC2) // : len = 0
                {
                    return prop(utf8len = 1);
                }
                else if (cp < 0xE0) // : len = 2
                {
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
                }
                else if (cp < 0xF0) // : len = 3
                {
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
                }
                else if (cp < 0xF5) // : len = 4
                {
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
                }
                else //if (cp <= 0xFF) : len = 0
                {
                    return prop(utf8len = 1);
                }
            }
            else
            {
                return prop(utf8len = 0);
            }
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
                        if (next.cdpoint == matrix::stx)
                        {
                            auto head = code.textptr;
                            auto utf8len = next.utf8len;
                            auto cpcount = 1; // STX
                            code.step();
                            next = code.take();
                            auto left = next; // Base char in cluster.
                            while (next.correct && (next.cdpoint < matrix::min_vs_code || next.cdpoint > matrix::max_vs_code)) // Eat all until VS.
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

    // utf: Filter cluster's for non-control codepoints and place it to the block.
    void filter_non_control(view cluster, text& block)
    {
        auto code_iter = utf::cpit{ cluster };
        while (code_iter)
        {
            auto codepoint = code_iter.take();
            if (utf::non_control(codepoint.cdpoint))
            {
                block += view{ code_iter.textptr, code_iter.utf8len };
            }
            code_iter.step();
        }
    }
    // utf: Return the first grapheme cluster and its Unicode attributes.
    template<bool AllowControls = faux>
    auto cluster(view utf8)
    {
        return frag::take_cluster<AllowControls>(utf8);
    }
    // utf: Break text into grapheme clusters filtered from codepoints.
    void decode_clusters(view utf8, auto yield)
    {
        if (auto code = cpit{ utf8 })
        {
            auto next = code.take();
            do
            {
                if (!utf::non_control(next.cdpoint))
                {
                    code.step();
                    next = code.take();
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
                            if (!code)
                            {
                                auto crop = view(head, left.utf8len);
                                if (!yield(crop)) return;
                                break;
                            }
                            next = code.take();
                            if (!utf::non_control(next.cdpoint)) // Skip controls.
                            {
                                code.step();
                                next = code.take();
                                break;
                            }
                            if (left.combine(next))
                            {
                                auto crop = view(head, left.utf8len);
                                if (!yield(crop)) return;
                                break;
                            }
                        }
                        else
                        {
                            auto crop = replacement;
                            if (!yield(crop)) return;
                            next = code.take();
                            break;
                        }
                    }
                    while (true);
                }
            }
            while (code);
        }
    }
    // utf: Break the text into the grapheme clusters.
    //      Forward the result using the callable "serve" and "yield".
    //      serve: Processes controls and returns the rest of the utf8.
    //             auto s = [&](utf::prop const& traits, view utf8) -> view;
    //      yield: Processes clusters.
    //             auto y = [&](frag const& cluster){};
    //      ascii: Processes ASCII chars (fast forward).
    //             auto a = [&](frag const& cluster){};
    //      Clusterize: Decode cluster-by-cluster (if true) or codepoint-by-codepoint (if faux).
    template<bool Clusterize = true>
    void decode(auto serve, auto yield, auto ascii, view utf8, si32& decsg)
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
                    auto custom_cluster_initiator = Clusterize && next.cdpoint == matrix::stx && code.balance > 1;
                    if (custom_cluster_initiator)
                    {
                        auto rest = code.rest();
                        auto utf8len = next.utf8len;
                        auto cpcount = 1; // STX
                        code.step();
                        next = code.take();
                        auto left = next; // Base char in cluster.
                        while (next.correct && (next.cdpoint < matrix::min_vs_code || next.cdpoint > matrix::max_vs_code)) // Eat all until VS.
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
                            code.step();
                            next = code.take();
                        }
                        else // Broken cluster. Silently ignore STX.
                        {
                            code.redo(rest.substr(1));
                            next = left;
                        }
                    }
                    else // Proceed general control.
                    {
                        code.step();
                        auto rest = code.rest();
                        auto chars = serve(next, rest);
                        code.redo(chars);
                        next = code.take();
                    }
                }
                else
                {
                    auto is_plain = [](byte c){ return c >= 0x20 && c < 0x7f; };
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
    auto codepoint_count(auto&& cluster)
    {
        auto count = 0;
        auto head = std::begin(cluster);
        auto tail = std::end(cluster);
        while (head != tail)
        {
            auto c = *head++;
            if ((c & 0xC0) != 0x80) ++count;
        }
        return count;
    }
    // utf: Decode clusters from utf8 and filter non-controls to the dest in reverse order (required for RTL rendering).
    void reverse_clusters(view utf8, auto& dest)
    {
        auto rest = (si32)utf8.size();
        dest.resize(dest.size() + rest);
        auto a = dest.end();
        utf::decode_clusters(utf8, [&](auto cluster)
        {
            rest -= (si32)cluster.size();
            if (rest < 0) return faux;
            auto b = std::exchange(a, a - cluster.size());
            dest.replace(a, b, cluster);
            return true;
        });
    }

    struct qiew : public view
    {
        using view::view;
        using equal = std::equal_to<>;

        struct hash
        {
            using is_transparent = void;
            using hash_type = std::hash<view>;
            auto operator () (text const& s) const { return hash_type{}(s); }
            auto operator () (char const* s) const { return hash_type{}(s); }
            auto operator () (view s)        const { return hash_type{}(s); }
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
        // qiew: Pop back.
        auto pop_back()
        {
            auto c = view::back();
            view::remove_suffix(1);
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

    template<class Key = text, class Val = text>
    using unordered_map = std::unordered_map<Key, Val, qiew::hash, qiew::equal>;

    template<class A = si32, si32 Base = 10, class View, class = std::enable_if_t<std::is_base_of_v<view, View>>>
    std::optional<A> to_int(View& ascii)
    {
        auto num = A{};
        if constexpr (Base == 16)
        {
            if (ascii.starts_with("0x") || ascii.starts_with("0X")) ascii.remove_prefix(2);
        }
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
                push((char)code);
            }
            else if (code <= 0x07ff)
            {
                push((char)(0xc0 | ((code >> 0x06) & 0x1f)));
                push((char)(0x80 | ( code          & 0x3f)));
            }
            else if (code <= 0xffff)
            {
                push((char)(0xe0 | ((code >> 0x0c) & 0x0f)));
                push((char)(0x80 | ((code >> 0x06) & 0x3f)));
                push((char)(0x80 | ( code          & 0x3f)));
            }
            else
            {
                push((char)(0xf0 | ((code >> 0x12) & 0x07)));
                push((char)(0x80 | ((code >> 0x0c) & 0x3f)));
                push((char)(0x80 | ((code >> 0x06) & 0x3f)));
                push((char)(0x80 | ( code          & 0x3f)));
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
    void replace_all(view utf8, view what, view to, text& dest)
    {
        auto last = 0_sz;
        if (!what.empty() && utf8.length() >= what.length())
        {
            auto spot = 0_sz;
            while ((spot = utf8.find(what, last)) != text::npos)
            {
                dest += utf8.substr(last, spot - last);
                dest += to;
                last = spot + what.size();
            }
        }
        dest += utf8.substr(last);
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
    template<class T, si32 L = std::numeric_limits<T>::digits>
    auto to_bin(T n)
    {
        return std::bitset<L>(n).to_string();
    }
    template<si32 Size>
    auto to_oct(si32 n)
    {
        static_assert(Size > 0);
        auto crop = text{};
        auto i = Size;
        n = std::abs(n);
        crop.resize(Size);
        while (i--)
        {
            crop[i] = netxs::onlydigits[n & 7];
            n >>= 3;
        }
        return crop;
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
    template<bool Uppercase = faux>
    auto to_hex_0x(auto const& n)
    {
        auto h = [](auto const& n){ return (flux{} << std::showbase << (Uppercase ? std::uppercase : std::nouppercase) << std::hex << n).str(); };
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
    auto split2(view utf8, char delimiter, auto proc)
    {
        auto cur = 0_sz;
        auto pos = 0_sz;
        while ((pos = utf8.find(delimiter, cur)) != text::npos)
        {
            auto frag = view{ utf8.data() + cur, pos - cur };
            if (!proc(frag, faux)) return faux;
            cur = pos + 1;
        }
        auto end = view{ utf8.data() + cur, utf8.size() - cur };
        return proc(end, true);
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
                    for (auto& a : buff) a = (byte)look.find(a);
                    data.push_back(( buff[0]         << 2) + ((buff[1] & 0x30) >> 4));
                    data.push_back(((buff[1] & 0x0F) << 4) + ((buff[2] & 0x3C) >> 2));
                    data.push_back(((buff[2] & 0x03) << 6) +   buff[3]);
                }
            }

            if (step != 0)
            {
                auto temp = step;
                while (temp < 4) buff[temp++] = 0;
                for (auto& a : buff) a = (byte)look.find(a);
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
        return (si32)(buff.length() - init);
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
                     if (c < 0x20)  buff += utf::c0_view[c];
                else if (c == 0x7F) buff += utf::c0_view[0x20];
                else                buff += view(code.textptr, code.utf8len);
                code.step();
                next = code.take();
            }
            while (code);
        }
    }
    // utf: Replace all bytes with "cp437" glyphs.
    auto debase437bytes(qiew utf8, text& buff)
    {
        for (byte c : utf8)
        {
                 if (c < 0x20)  buff += utf::c0_view[c];
            else if (c == 0x7F) buff += utf::c0_view[0x20];
            else if (c < 0x7F)  buff += c;
            else                buff += utf::ext_437[c - 0x80];
        }
    }
    // utf: Replace all bytes with "cp437" glyphs.
    auto debase437bytes(qiew utf8)
    {
        auto buff = text{};
        debase437bytes(utf8, buff);
        return buff;
    }
    // utf: Replace all bytes with colored blocks.
    auto bytes2shades(qiew utf8)
    {
        assert(utf8.size() % 2 == 0);
        auto buff = text{};
        auto bstr = text{};
        auto gstr = text{};
        buff += "\x1b[#{";
        auto head = utf8.begin();
        auto tail = utf8.end();
        while (head != tail)
        {
            auto g = *head++;
            auto b = *head++;
            gstr = std::to_string(g);
            bstr = std::to_string(b);
            buff += "\x1b[38:2:32:" + gstr + ":128m";
            buff += "\x1b[48:2:32:128:" + bstr + "m▐";
        }
        buff += "\x1b[#}";
        return buff;
    }
    // utf: Return a string without control chars (replace all ctrls with "cp437" glyphs).
    auto debase437(qiew utf8)
    {
        auto buff = text{};
        debase437(utf8, buff);
        return buff;
    }
    // utf: Find char position iterator.
    auto _find_char(auto head, auto tail, auto hittest)
    {
        while (head != tail && !hittest(head))
        {
            ++head;
        }
        return head;
    }
    // utf: Find char position iterator ignoring backslashed.
    auto _find_char_except_escaped(auto head, auto tail, auto hittest)
    {
        while (head != tail)
        {
            if (hittest(head) || (*head == '\\' && ++head == tail)) break;
            ++head;
        }
        return head;
    }
    // utf: Find char position iterator ignoring backslashed.
    template<class Iter>
    auto find_char(Iter head, Iter tail, view delims)
    {
        return _find_char_except_escaped(head, tail, [&](auto iter){ return delims.find(*iter) != view::npos; });
    }
    // utf: Check if utf8 start with test string.
    bool _starts_with(auto& iter, view utf8, view test)
    {
        auto found = utf8.starts_with(test);
        if (found)
        {
            iter += test.size();
        }
        return found;
    }
    // utf: Check if utf8 starts with something using test proc (the test proc must do increment of iter if skips).
    bool _starts_with(auto& iter, view utf8, auto test)
    {
        return test(iter, utf8);
    }
    // utf: Find char position iterator ignoring skips.
    template<class ...Args>
    auto find_char_except_skips(view utf8, char c, Args&&... skips)
    {
        auto head = utf8.begin();
        auto tail = utf8.end();
        auto found_iter = _find_char(head, tail, [&](auto& iter)
        {
            if (*iter == c)
            {
                auto substr = view{ iter, tail };
                auto found = (utf::_starts_with(iter, substr, skips) || ...);
                if (found)
                {
                    return iter == tail; // faux if not end
                }
                else
                {
                    return true;
                }
            }
            return faux;
        });
        return found_iter;
    }
    // utf: Find substring position iterator ignoring backslashed.
    auto find_substring(view& utf8, auto... delims)
    {
        auto head = utf8.begin();
        auto tail = utf8.end();
        return _find_char_except_escaped(head, tail, [&](auto iter){ return (view{ iter, tail }.starts_with(delims) || ...); });
    }
    // utf: Find char position iterator ignoring backslashed.
    template<class Iter>
    auto find_char(Iter head, Iter tail, char delim)
    {
        return _find_char_except_escaped(head, tail, [&](auto iter){ return *iter == delim; });
    }
    auto check_any(view shadow, view delims)
    {
        auto p = utf::find_char(shadow.begin(), shadow.end(), delims);
        return p != shadow.end();
    }
    // utf: Trim the utf8 front if not pred.
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
    // utf: Trim the utf8 back if not pred.
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
    // utf: Trim the utf8 front.
    void trim_front(view& utf8, char c = ' ')
    {
        auto head = utf8.begin();
        auto tail = utf8.end();
        while (head != tail && *head == c)
        {
            ++head;
        }
        utf8.remove_prefix(std::distance(utf8.begin(), head));
    }
    // utf: Trim the utf8 back.
    void trim_back(view& utf8, char c = ' ')
    {
        auto head = utf8.rbegin();
        auto tail = utf8.rend();
        while (head != tail && *head == c)
        {
            ++head;
        }
        utf8.remove_suffix(std::distance(utf8.rbegin(), head));
    }
    // utf: Trim the utf8 front.
    void trim_front(view& utf8, view delims)
    {
        utf::trim_front_if(utf8, [&](char c){ return delims.find(c) == text::npos; });
    }
    // utf: Trim the utf8 back.
    void trim_back(view& utf8, view delims)
    {
        utf::trim_back_if(utf8, [&](char c){ return delims.find(c) == text::npos; });
    }
    // utf: Trim the utf8 while any of delims front and return trims.
    auto pop_front_chars(view& utf8, view while_any_of)
    {
        auto temp = utf8;
        utf::trim_front(utf8, while_any_of);
        return temp.substr(0, temp.size() - utf8.size());
    }
    // utf: Trim the utf8 front until any of delims is found.
    template<bool Lazy = true>
    void pop_front_until(view& utf8, auto until_any_of)
    {
        auto head = utf8.begin();
        auto tail = utf8.end();
        auto stop = utf::find_char(head, tail, until_any_of);
        auto prefix_len = stop - head;
        utf8.remove_prefix(prefix_len);
    }
    // utf: Trim the utf8 back while any of delims and return trims.
    auto pop_back_chars(view& utf8, view while_any_of)
    {
        auto temp = utf8;
        utf::trim_back(utf8, while_any_of);
        return temp.substr(utf8.size());
    }

    // utf: Return front-trimmed copy of utf8.
    auto get_trimmed_front(view utf8, char c = ' ')
    {
        utf::trim_front(utf8, c);
        return utf8;
    }
    // utf: Return back-trimmed copy of utf8.
    auto get_trimmed_back(view utf8, char c = ' ')
    {
        utf::trim_back(utf8, c);
        return utf8;
    }
    // utf: Trim utf8.
    void trim(view& utf8, char c = ' ')
    {
        if (!utf8.empty())
        {
            utf::trim_front(utf8, c);
            if (!utf8.empty())
            {
                utf::trim_back(utf8, c);
            }
        }
    }
    // utf: Trim utf8.
    void trim(view& utf8, view delims)
    {
        if (!utf8.empty())
        {
            utf::trim_front(utf8, delims);
            if (!utf8.empty())
            {
                utf::trim_back(utf8, delims);
            }
        }
    }
    // utf: Return trimmed copy of utf8.
    auto get_trimmed(view utf8, char c = ' ')
    {
        utf::trim(utf8, c);
        return utf8;
    }
    // utf: Return trimmed copy of utf8.
    auto get_trimmed(view utf8, view delims)
    {
        utf::trim(utf8, delims);
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
        auto start = dest.size();
        dest.resize(start + line.size() * 2);
        auto iter = dest.begin() + start;
        _escape(line, iter, x...);
        dest.resize(iter - dest.begin());
    }
    auto filter_alphanumeric(qiew line, text& dest)
    {
        dest.reserve(dest.size() + line.size());
        while (line)
        {
            auto c = line.pop_front();
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_') // The environment variable must be alphanumeric, and may contain an underscore.
            {
                dest.push_back(c);
            }
        }
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
    auto dequote(view utf8)
    {
        if (utf8.size() > 1)
        {
            auto c = utf8.front();
            if ((c == '\'' || c == '\"') && c == utf8.back())
            {
                utf8 = utf8.substr(1, utf8.size() - 2);
                return unescape(utf8);
            }
        }
        return text{ utf8 };
    }
    // utf: Trim utf8 up to and including stopstr, and return the trims.
    template<bool Lazy = true>
    auto take_front_including(view& utf8, view stopstr)
    {
        auto iter = utf::find_substring(utf8, stopstr);
        if (iter == utf8.end())
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
        auto str = qiew{ utf8.begin(), iter + stopstr.size() };
        utf8.remove_prefix(str.size());
        return str;
    }
    // utf: Trim utf8 up to and including stopstr, and return the trims.
    template<bool Lazy = true>
    auto split_back(qiew utf8, char delim)
    {
        auto pos = utf8.rfind(delim, utf8.size());
        if (pos != text::npos)
        {
            return std::pair{ utf8.substr(0, pos), utf8.substr(pos + sizeof(delim)) };
        }
        else
        {
            return std::pair{ qiew{}, utf8 };
        }
    }
    // utf: Trim utf8 until any of delims is found, and return trims.
    template<bool Lazy = true>
    auto take_front(view& utf8, view delims)
    {
        auto temp = qiew{ utf8 };
        utf::pop_front_until(utf8, delims);
        if (utf8.empty()) // If not found.
        {
            if constexpr (Lazy)
            {
                return qiew{};
            }
            else
            {
                return temp;
            }
        }
        else
        {
            auto crop = temp.substr(0, temp.size() - utf8.size());
            return crop;
        }
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
    auto remove_quotes(view utf8)
    {
        if (utf8.size() > 2)
        {
            auto c = utf8.front();
            if ((c == '\'' || c == '\"') && c == utf8.back())
            {
                utf8 = utf8.substr(1, utf8.size() - 2);
            }
        }
        return utf8;
    }
    // utf: Split text line into quoted tokens.
    auto tokenize(view utf8, auto&& args)
    {
        utf::trim(utf8, ' ');
        while (utf8.size())
        {
            auto c = utf8.front();
            if (c == '\'' || c == '"') args.emplace_back(utf::unescape(utf::take_quote(utf8, c)));
            else                       args.emplace_back(utf::get_word(utf8));
            utf::trim_front(utf8);
        }
        return args;
    }
    // utf: Remove utf8 tail until any of delims (including delim).
    auto eat_tail(view& utf8, view delims)
    {
        auto head = utf8.begin();
        auto tail = utf8.end();
        auto stop = utf::find_char(head, tail, delims);
        if (stop == tail) utf8 = view{};
        else              utf8.remove_prefix(std::distance(head, stop));
    }
    // utf: Remove utf8 tail including delim.
    auto eat_tail(view& utf8, char delim)
    {
        auto stop = utf8.rfind(delim);
        if (stop == text::npos) utf8 = {};
        else                    utf8 = utf8.substr(0, stop);
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
    auto to_lower(char c)
    {
        return c >= 'A' && c <= 'Z' ? (char)(c + ('a' - 'A')) : c;
    }
    auto to_upper(char c)
    {
        return c >= 'a' && c <= 'z' ? (char)(c - ('a' - 'A')) : c;
    }
    auto& to_lower(text& utf8, size_t size = text::npos)
    {
        auto head = utf8.begin();
        auto tail = head + std::min(utf8.size(), size);
        std::transform(head, tail, head, [](auto c){ return to_lower(c); });
        return utf8;
    }
    auto to_lower(text&& utf8)
    {
        return to_lower(utf8);
    }
    auto& to_upper(text& utf8, size_t size = text::npos)
    {
        auto head = utf8.begin();
        auto tail = head + std::min(utf8.size(), size);
        std::transform(head, tail, head, [](char c){ return to_upper(c); });
        return utf8;
    }
    auto to_upper(text&& utf8)
    {
        return to_upper(utf8);
    }
    auto name2token(view utf8)
    {
        auto name_token = text{};
        name_token.reserve(utf8.size());
        for (auto c : utf8)
        {
            if (c != ' ' && c != '-')
            {
                name_token += utf::to_lower(c);
            }
        }
        return name_token;
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
    auto& operator << (std::ostream& s, time const& o)
    {
        auto [hours, mins, secs, milli, micro] = datetime::breakdown(o);
        return s << utf::adjust(std::to_string(hours), 2, '0', true) << ':'
                 << utf::adjust(std::to_string(mins ), 2, '0', true) << ':'
                 << utf::adjust(std::to_string(secs ), 2, '0', true) << '.'
                 << utf::adjust(std::to_string(milli), 3, '0', true)
                 << utf::adjust(std::to_string(micro), 3, '0', true);
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