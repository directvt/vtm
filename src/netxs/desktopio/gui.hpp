// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#include <memory_resource>

namespace netxs::gui
{
    using namespace input;

    struct manager_base
    {
        struct bttn
        {
            static constexpr auto left   = 1 << 0;
            static constexpr auto right  = 1 << 1;
            static constexpr auto middle = 1 << 2;
        };
        struct state
        {
            static constexpr auto _counter  = __COUNTER__ + 1;
            static constexpr auto undefined = __COUNTER__ - _counter;
            static constexpr auto normal    = __COUNTER__ - _counter;
            static constexpr auto minimized = __COUNTER__ - _counter;
            static constexpr auto maximized = __COUNTER__ - _counter;
        };
        struct timers
        {
            static constexpr auto _counter = __COUNTER__ + 1;
            static constexpr auto none     = __COUNTER__ - _counter;
            static constexpr auto blink    = __COUNTER__ - _counter;
        };
        struct syscmd
        {
            static constexpr auto _counter     = __COUNTER__ + 1;
            static constexpr auto minimize     = __COUNTER__ - _counter;
            static constexpr auto maximize     = __COUNTER__ - _counter;
            static constexpr auto restore      = __COUNTER__ - _counter;
            static constexpr auto move         = __COUNTER__ - _counter;
            static constexpr auto monitorpower = __COUNTER__ - _counter;
            static constexpr auto close        = __COUNTER__ - _counter;
        };
        
        bool isfine = true; // manager: All is ok.
        explicit operator bool () const { return isfine; }
    };
    struct surface_base
    {
        using bits = netxs::raster<std::span<argb>, rect>;
        using regs = std::vector<rect>;
        using tset = std::list<ui32>;

        rect prev; // surface: Last presented layer area.
        rect area; // surface: Current layer area.
        bits data; // surface: Layer bitmap.
        regs sync; // surface: Dirty region list.
        bool live; // surface: Should the layer be presented.
        tset klok; // surface: Active timer list.

        surface_base()
          : prev{ .coor = dot_mx },
            area{ dot_00, dot_00 },
            live{ faux }
        { }
        void hide() { live = faux; }
        void show() { live = true; }
        void strike(rect r)
        {
            if (sync.empty()) sync.push_back(r);
            else
            {
                auto& back = sync.back();
                if (back.nearby(r)) back.unitewith(r);
                else                sync.push_back(r);
            }
        }
    };

    //test strings
    template<auto ...Args>
    constexpr auto vss = utf::matrix::vss<Args...>;
    auto intro = ansi::add("").wrp(wrap::on).fgc(purecyan)
        .add("\2Hello", utf::vs10, vss<11>, "\n")
        .add("        LeftDrag: Move window.\n"
             "       RightDrag: Panoramic scrolling.\n"
             "           Wheel: Vertical scrolling.\n"
             "         HzWheel: Horizontal scrolling.\n"
             "      Ctrl+Wheel: Change cell height.\n"
             "F11/DblLeftClick: Toggle fullsceen mode.\n"
             "               A: Toggle antialiasing mode.\n"
             "               0: Roll font fallback list.\n"
             "             1-9: Reorder font fallback list.\n"
             "             ESC: Close window.\n\n");
    auto canvas_text = ansi::add("\n").blk(true).bld(faux).wrp(wrap::on).fgc(purecyan)
        .add(">←→< >↑< ⌠ ⎲ ⎛⎧ ...   ⎝ ⎞ ⎟ ⎠ ⎡ ⎢ ⎣ ⎤ ⎥ ⎦ ⎧ ⎨ ⎩ ⎪ ⎫ ⎬ ⎭ ⎮ ⎯ ⎰ ⎱ \n")
        .add("     >↓< ⌡ ⎳ ⎜⎨⇀⇁\n")
        .add("             ⎝⎩\n")
        .fgc(whitelt).bgc(bluelt).add("\n gggjjj INSERT  ").fgc(bluelt).bgc(blacklt).add("\uE0B0").fgc(whitelt).add(" \uE0A0 master ").fgc(blacklt).bgc(argb{}).add("\uE0B0   ")
            .add("Powerline test   \uE0B2").fgc(whitelt).bgc(blacklt).add(" [dos] ").fgc(bluelt).add("\uE0B2").fgc(whitelt).bgc(bluelt).add(" 100% \uE0A1    2:  1 \n").bgc(argb{})
        .fgc(tint::purecyan).add(
R"==(
CJK文字是對中文、日文文字和韓文的統稱，這些語言全部含有汉字及其變體，某些會與其他文字混合使用。因為越南文曾經使用漢字，所以它有時候與CJK文字結合，組成CJKV文字（英語：Chinese-Japanese-Korean-Vietnamese）。概括來說，CJKV文字通常包括中文的漢字、日文文字的日本汉字及日語假名、韓文的朝鮮漢字及諺文和越南文的儒字和喃字。
)==")
        .add("\nThai sentence: สวัสดี ครับ\n")
        .fgc(tint::purecyan)
        .add(
R"==(
Box drawing alignment tests:                                          █
                                                                      ▉
  ╔══╦══╗  ┌──┬──┐  ╭──┬──╮  ╭──┬──╮  ┏━━┳━━┓  ┎┒┏┑   ╷  ╻ ┏┯┓ ┌┰┐    ▊ ╱╲╱╲╳╳╳
  ║┌─╨─┐║  │╔═╧═╗│  │╒═╪═╕│  │╓─╁─╖│  ┃┌─╂─┐┃  ┗╃╄┙  ╶┼╴╺╋╸┠┼┨ ┝╋┥    ▋ ╲╱╲╱╳╳╳
  ║│╲ ╱│║  │║   ║│  ││ │ ││  │║ ┃ ║│  ┃│ ╿ │┃  ┍╅╆┓   ╵  ╹ ┗┷┛ └┸┘    ▌ ╱╲╱╲╳╳╳
  ╠╡ ╳ ╞╣  ├╢   ╟┤  ├┼─┼─┼┤  ├╫─╂─╫┤  ┣┿╾┼╼┿┫  ┕┛┖┚     ┌┄┄┐ ╎ ┏┅┅┓ ┋ ▍ ╲╱╲╱╳╳╳
  ║│╱ ╲│║  │║   ║│  ││ │ ││  │║ ┃ ║│  ┃│ ╽ │┃  ░░▒▒▓▓██ ┊  ┆ ╎ ╏  ┇ ┋ ▎
  ║└─╥─┘║  │╚═╤═╝│  │╘═╪═╛│  │╙─╀─╜│  ┃└─╂─┘┃  ░░▒▒▓▓██ ┊  ┆ ╎ ╏  ┇ ┋ ▏
  ╚══╩══╝  └──┴──┘  ╰──┴──╯  ╰──┴──╯  ┗━━┻━━┛           └╌╌┘ ╎ ┗╍╍┛ ┋  ▁▂▃▄▅▆▇█
)==")
        .add(
R"==(
Using large type pieces:
𜸜 𜸜𜸚𜸟𜸤𜸜𜸝𜸢𜸜𜸚𜸟𜸤  𜸜  𜸚𜸟𜸤𜸛𜸟𜸤𜸚𜸟𜸤𜸛𜸟𜸥  𜸞𜸠𜸥𜸜 𜸜𜸛𜸟𜸤𜸛𜸟𜸥  𜸛𜸟𜸤𜸜𜸛𜸟𜸥𜸚𜸟𜸤𜸛𜸟𜸥𜸚𜸟𜸤
𜸩 𜸩𜸾𜸟𜸤𜸩𜸩𜸫𜸹𜸩 𜸧  𜸩  𜸨𜸟𜸶𜸨𜸟𜸷𜸩 𜸧𜸨𜸟    𜸩 𜸫𜸳𜸻𜸨𜸟𜹃𜸨𜸟   𜸨𜸟𜹃𜸩𜸨𜸟 𜸩  𜸨𜸟 𜸾𜸟𜸤
𜸾𜸟𜹃𜸾𜸟𜹃𜸼𜸼 𜸼𜸾𜸟𜹃  𜸽𜸟𜸥𜸼 𜸼𜸼 𜸼𜸾𜸟𜹃𜸽𜸟𜸥   𜸼  𜸼 𜸼  𜸽𜸟𜸥  𜸼  𜸼𜸽𜸟𜸥𜸾𜸟𜹃𜸽𜸟𜸥𜸾𜸟𜹃

𜸜𜸝𜸢𜸜𜸞𜸠𜸥𜸛𜸟𜸤𜸚𜸟𜸤𜸛𜸟𜸤𜸜 𜸜𜸚𜸟𜸤𜸞𜸠𜸥𜸜𜸚𜸟𜸤𜸝𜸢𜸜
𜸩𜸩𜸫𜸹 𜸩 𜸨𜸟𜸷𜸩 𜸩𜸩 𜸩𜸩 𜸩𜸩   𜸩 𜸩𜸩 𜸩𜸩𜸫𜸹
𜸼𜸼 𜸼 𜸼 𜸼 𜸼𜸾𜸟𜹃𜸽𜸟𜹃𜸾𜸟𜹃𜸾𜸟𜹃 𜸼 𜸼𜸾𜸟𜹃𜸼 𜸼
)==")
        .add("\n")
        .add("\2aaaa", utf::vs07, vss<21>, "<VS22_00  >👩‍👩‍👧‍👧", vss<31>, "<VS31_00  >👩‍👩‍👧‍👧", vss<41>, "<VS41_00\n")
        .add("❤", vss<11>, "<VS11_00 ", "👩‍👩‍👧‍👧", vss<21>, "<VS21_00  >👩‍👩‍👧‍👧", vss<31>, "<VS31_00  >👩‍👩‍👧‍👧", vss<41>, "<VS41_00\n")
        //todo multiline graphemes
        //.add("\2line1\nline2", vss<52,01>, "\n")
        //.add("\2line1\nline2", vss<52,02>, "\n")
        .bld(true).itc(true).add("vtm GUI frontend WVMQWERTYUIOPASDFGHJKLZXCVBNM韓M😎M\n")
        .bld(true).itc(faux).add("vtm GUI frontend WVMQWERTYUIOPASDFGHJKLZXCVBNM韓M😎M\n")
        .bld(faux).itc(true).add("vtm GUI frontend WVMQWERTYUIOPASDFGHJKLZXCVBNM韓M😎M\n")
        .bld(faux).itc(faux).add("vtm GUI frontend WVMQWERTYUIOPASDFGHJKLZXCVBNM韓M😎M").itc(faux).fgc(tint::purered).bld(true).add(" is currently under development.").nil()
        .fgc(tint::purecyan).add(" You can try it on any versions/editions of Windows platforms starting from Windows 8.1"
                               " (with colored emoji!), including Windows Server Core. 🥵🥵", vss<11>, "🦚😀⛷🏂😁😂😃😄😅😆👌🐞😎👪.\n")
        .add("\n")
        .fgc(tint::purecyan).bld(faux).add("Devanagari script:\n")
        .add("\2अनुच्छेद", vss<51>, " १.\n"     // अनुच्छेद १.
             "\2सभी", vss<31>, " \2मनुष्यों", vss<41>, " को", vss<21>, " \2गौरव", vss<31>, " \2और", vss<31>, " \2अधिकारों", vss<61>, " के", vss<21>, " \2मामले", vss<41>, " में "  // सभी मनुष्यों को गौरव और अधिकारों के मामले में
             "\2जन्मजात", vss<51>, " \2स्वतन्त्रता", vss<51>, " \2और", vss<31>, " \2समानता", vss<51>, " \2प्राप्त", vss<31>, " \2है।", vss<21>, "\n" // जन्मजात स्वतन्त्रता और समानता प्राप्त है।
             "\2उन्हें", vss<31>, " \2बुद्धि", vss<31>, " \2और", vss<31>, " \2अन्तरात्मा", vss<61>, " की", vss<21>, " \2देन", vss<21>, " \2प्राप्त", vss<31>, " है \2और", vss<31>, " " // उन्हें बुद्धि और अन्तरात्मा की देन प्राप्त है और
             "\2परस्पर", vss<41>, " \2उन्हें", vss<31>, " \2भाईचारे", vss<51>, " के", vss<21>, " \2भाव", vss<31>, " से \2बर्ताव ", vss<41>, " \2करना", vss<31>, " \2चाहिए।", vss<41>, "\n") // परस्पर उन्हें भाईचारे के भाव से बर्ताव करना चाहिए।
                        .add("\n")
        //.add("❤", vss<21>, "<VS21_00 😎", vss<11>, "<VS11_00 👩‍👩‍👧‍👧", vss<31>, "<VS31_00\n")
        .add("👩🏾‍👨🏾‍👧🏾‍👧🏾", vss<21>, "<VS21_00 😎", vss<11>, "<VS11_00 😎", vss<21>, "<VS21_00 ❤", vss<11>, "<VS11_00 ❤", vss<21>, "<VS21_00\n")
        .add("😎", vss<21,11>, " 😃", vss<21,21>, "<VS21_11/VS21_21\n")
        .add("\n")
        .add("G", vss<21>,              "<VS21_00:WideG   ").add("\2G", utf::vs13, vss<21>,            "<VS13:      HzFlip           ").add("\2G", utf::vs14, vss<21>,            "<VS14:      VtFlip\n")
        .add("\2G", utf::vs10, vss<21>, "<VS10:  90°CCW   ").add("\2G", utf::vs13, utf::vs10, vss<21>, "<VS13+VS10: HzFlip+90°CCW    ").add("\2G", utf::vs14, utf::vs10, vss<21>, "<VS14+VS10: VtFlip+90°CCW\n")
        .add("\2G", utf::vs11, vss<21>, "<VS11: 180°CCW   ").add("\2G", utf::vs13, utf::vs11, vss<21>, "<VS13+VS11: HzFlip+180°CCW   ").add("\2G", utf::vs14, utf::vs11, vss<21>, "<VS14+VS11: VtFlip+180°CCW\n")
        .add("😎",  utf::vs12, vss<21>, "<VS12: 270°CCW   ").add("\2G", utf::vs13, utf::vs12, vss<21>, "<VS13+VS12: HzFlip+270°CCW   ").add("\2G", utf::vs14, utf::vs12, vss<21>, "<VS14+VS12: VtFlip+270°CCW\n")
        .add("\n")
        .add("\2G", utf::vs10, utf::vs13, vss<21>, "<VS10+VS13: 90°CCW+HzFlip\n")
        .add("\2G", utf::vs13, utf::vs10, vss<21>, "<VS13+VS10: HzFlip+90°CCW\n")
        .add("\n")
        .add("  \2Mirror", utf::vs13, vss<81>, "<VS13\n")
        .add("  \2Mirror", utf::vs14, vss<81>, "<VS14\n")
        .fgc(blacklt).bgc(whitedk).add("\2Height", utf::vs05, utf::vs10, vss<24,11>).fgc(whitelt).bgc(blackdk).add("\2Height", utf::vs05, utf::vs10, vss<24,21>).bgc(argb{}).add("😎", vss<84,01>).fgc(purecyan).bgc(argb{}).add("\2Height", utf::vs05, utf::vs12, vss<24,01>).fgc(purecyan).add(" <VS84_00\n")
        .fgc(whitelt).bgc(blackdk).add("\2Height", utf::vs05, utf::vs10, vss<24,12>).fgc(blacklt).bgc(whitedk).add("\2Height", utf::vs05, utf::vs10, vss<24,22>).bgc(argb{}).add("😎", vss<84,02>).fgc(purecyan).bgc(argb{}).add("\2Height", utf::vs05, utf::vs12, vss<24,02>).add("\n")
        .fgc(blacklt).bgc(whitedk).add("\2Height", utf::vs05, utf::vs10, vss<24,13>).fgc(whitelt).bgc(blackdk).add("\2Height", utf::vs05, utf::vs10, vss<24,23>).bgc(argb{}).add("😎", vss<84,03>).fgc(purecyan).bgc(argb{}).add("\2Height", utf::vs05, utf::vs12, vss<24,03>).add("\n")
        .fgc(whitelt).bgc(blackdk).add("\2Height", utf::vs05, utf::vs10, vss<24,14>).fgc(blacklt).bgc(whitedk).add("\2Height", utf::vs05, utf::vs10, vss<24,24>).bgc(argb{}).add("😎", vss<84,04>).fgc(purecyan).bgc(argb{}).add("\2Height", utf::vs05, utf::vs12, vss<24,04>).add("\n")
        .add("  ").fgc(blacklt).bgc(whitedk).add("\2Width", utf::vs05, utf::vs11, vss<81,11>).fgc(whitelt).bgc(blackdk).add("\2Width", utf::vs05, utf::vs11, vss<81,21>).fgc(blacklt).bgc(whitedk).add("\2Width", utf::vs05, utf::vs11, vss<81,31>).fgc(whitelt).bgc(blackdk).add("\2Width", utf::vs05, utf::vs11, vss<81,41>)
                  .fgc(blacklt).bgc(whitedk).add("\2Width", utf::vs05, utf::vs11, vss<81,51>).fgc(whitelt).bgc(blackdk).add("\2Width", utf::vs05, utf::vs11, vss<81,61>).fgc(blacklt).bgc(whitedk).add("\2Width", utf::vs05, utf::vs11, vss<81,71>).fgc(whitelt).bgc(blackdk).add("\2Width", utf::vs05, utf::vs11, vss<81,81>)
                  .fgc(purecyan).bgc(argb{}).add("<VS11\n")
        .add("Advanced ").add("T", vss<22,01>, "e", vss<22,01>, "r", vss<22,01>, "m", vss<22,01>, "i", vss<22,01>, "n", vss<22,01>, "a", vss<22,01>, "l", vss<22,01>, "\n")
        .add("Terminal ").add("T", vss<22,02>, "e", vss<22,02>, "r", vss<22,02>, "m", vss<22,02>, "i", vss<22,02>, "n", vss<22,02>, "a", vss<22,02>, "l", vss<22,02>, "\n")
        .add("Emulator ").fgc(tint::pureyellow).add("★", vss<21>, "★", vss<21>, "★", vss<21>, "★", vss<21>, "★", vss<21>, "★", vss<21>, "★", vss<21>).fgc(purecyan).add("☆", vss<21>, "\n")
                        .add("\n")
        .add("😎", vss<42,01>, " <VS42_00\n")
        .add("😎", vss<42,02>, "\n")
                        .add("\n")
        .add("❤❤❤👩‍👩‍👧‍👧🥵🦚🧞‍♀️🧞‍♂️>🏴‍☠< Raw>❤< VS15>❤︎< VS16>❤️< >👩🏾‍👨🏾‍👧🏾‍👧🏾< >👩‍👩‍👧‍👧<\n")
        .fgc(purered).bgc(pureblue).add(" test \n")
        .fgc(puregreen).bgc(pureblue).add(" test \n")
        .fgc(purecyan).bgc(purered).add(" test \n")
        //.fgc(purewhite).bgc(pureblack).add(" test \n")
        .bgc(argb{})
        .fgc(tint::purered).add("test").fgc(tint::purecyan).add("test 1234567890 !@#$%^&*()_+=[]\\\n\n");

    auto header_text = ansi::fgc(tint::purewhite).add("Windows Command Prompt - 😎 - C:\\Windows\\System32\\").nop().pushsgr().chx(0).jet(bias::right).fgc(argb::vt256[4]).add("\0▀"sv).nop().popsgr();
    auto footer_text = ansi::wrp(wrap::on).jet(bias::right).fgc(tint::purewhite).add("4/40000 80:25");
    auto canvas_page = ui::page{};
    auto header_page = ui::page{ header_text };
    auto footer_page = ui::page{ footer_text };
    static constexpr auto tttest1 = utf::matrix::vs_code<11>;
    static constexpr auto tttest2 = vss<11>;
    static constexpr auto tttest3 = utf::utf8bytes<tttest1>;
    static constexpr auto tttest4 = utf::utf8bytes<utf::vs12_code>;
}

#if defined(_WIN32)

#undef GetGlyphIndices
#include <DWrite_2.h>
#pragma comment(lib, "Gdi32")
#pragma comment(lib, "dwrite")

#define ok2(...) [&](){ auto hr = __VA_ARGS__; if (hr != S_OK) log(utf::to_hex(hr), " ", #__VA_ARGS__); return hr == S_OK; }()

namespace netxs::gui
{
    struct font
    {
        struct style
        {
            static constexpr auto normal      = 0;
            static constexpr auto italic      = 1;
            static constexpr auto bold        = 2;
            static constexpr auto bold_italic = bold | italic;
        };
        struct fontcat
        {
            static constexpr auto loaded     = 1ull << 60;
            static constexpr auto valid      = 1ull << 61;
            static constexpr auto monospaced = 1ull << 62;
            static constexpr auto color      = 1ull << 63;
        };
        struct typeface
        {
            struct face_rec
            {
                IDWriteFontFace2* face_inst{};
                fp32              transform{};
                fp32              em_height{};
                fp32              transform_letters{};
                fp32              em_height_letters{};
                fp2d              actual_sz{};
                fp2d              base_line{};
                rect              underline{}; // face_rec: Underline rectangle block within the cell.
                rect              doubline1{}; // font: The first line of the double underline: at the top of the rect.
                rect              doubline2{}; // font: The second line of the double underline: at the bottom.
                rect              strikeout{}; // face_rec: Strikethrough rectangle block within the cell.
                rect              overline{};  // face_rec: Overline rectangle block within the cell.
                rect              dashline{};  // face_rec: Dashed underline rectangle block within the cell.
                rect              wavyline{};  // face_rec: Wavy underline outer rectangle block within the cell.
            };
            std::vector<face_rec>             fontface;
            fp32                              base_descent{};
            fp32                              base_ascent{};
            fp2d                              base_underline{};
            fp2d                              base_strikeout{};
            fp2d                              base_overline{};
            si32                              base_emheight{};
            si32                              base_x_height{};
            fp2d                              facesize; // Typeface cell size.
            fp32                              ratio{};
            ui32                              index{ ~0u };
            bool                              color{ faux };
            bool                              fixed{ faux }; // Preserve specified font order.
            text                              font_name;

            static auto iscolor(auto face_inst)
            {
                auto tableSize = ui32{};
                auto tableData = (void const*)nullptr;
                auto tableContext = (void*)nullptr;
                auto exists = BOOL{};
                face_inst->TryGetFontTable(DWRITE_MAKE_OPENTYPE_TAG('C', 'O', 'L', 'R'), //_In_ UINT32 openTypeTableTag,
                                           &tableData,    //_Outptr_result_bytebuffer_(*tableSize) const void** tableData,
                                           &tableSize,    //_Out_ UINT32* tableSize,
                                           &tableContext, //_Out_ void** tableContext,
                                           &exists);      //_Out_ BOOL* exists
                if (exists) face_inst->ReleaseFontTable(tableContext);
                return exists;
            }
            void load(IDWriteFontFamily* barefont)
            {
                auto get = [&](auto& face_inst, auto weight, auto stretch, auto style)
                {
                    auto fontfile = (IDWriteFont2*)nullptr;
                    barefont->GetFirstMatchingFont(weight, stretch, style, (IDWriteFont**)&fontfile);
                    if (!fontfile) return;
                    fontfile->CreateFontFace((IDWriteFontFace**)&face_inst);
                    fontfile->Release();
                };
                fontface.resize(4);
                get(fontface[style::normal     ].face_inst, DWRITE_FONT_WEIGHT_NORMAL,    DWRITE_FONT_STRETCH_NORMAL, DWRITE_FONT_STYLE_NORMAL);
                get(fontface[style::italic     ].face_inst, DWRITE_FONT_WEIGHT_NORMAL,    DWRITE_FONT_STRETCH_NORMAL, DWRITE_FONT_STYLE_ITALIC);
                get(fontface[style::bold       ].face_inst, DWRITE_FONT_WEIGHT_DEMI_BOLD, DWRITE_FONT_STRETCH_NORMAL, DWRITE_FONT_STYLE_NORMAL);
                get(fontface[style::bold_italic].face_inst, DWRITE_FONT_WEIGHT_DEMI_BOLD, DWRITE_FONT_STRETCH_NORMAL, DWRITE_FONT_STYLE_ITALIC);
                auto names = (IDWriteLocalizedStrings*)nullptr;
                barefont->GetFamilyNames(&names);
                auto buff = wide(100, 0);
                names->GetString(0, buff.data(), (ui32)buff.size());
                font_name = utf::to_utf(buff.data());
                names->Release();

                auto& face_inst = fontface[style::normal].face_inst;
                if (face_inst)
                {
                    auto m = DWRITE_FONT_METRICS1{};
                    face_inst->GetMetrics(&m);
                    base_underline = { (fp32)m.underlinePosition, (fp32)m.underlineThickness };
                    base_strikeout = { (fp32)m.strikethroughPosition, (fp32)m.strikethroughThickness };
                    base_overline = { std::min((fp32)m.ascent, (fp32)(m.capHeight - m.underlinePosition)), (fp32)m.underlineThickness };
                    base_emheight = m.designUnitsPerEm;
                    base_x_height = m.xHeight;
                    base_ascent = m.ascent + m.lineGap / 2.0f;
                    base_descent = m.descent + m.lineGap / 2.0f;
                    auto glyph_metrics = DWRITE_GLYPH_METRICS{};
                    // Take metrics for "x" or ".notdef" in case of missing 'x'. Note: ".notdef" is double sized ("x" is narrow) in CJK fonts.
                    //auto code_points = ui32{ 'x' };
                    auto code_points = ui32{ 'U' }; // U is approximately half an emoji square in the Segoe Emoji font.
                    auto glyph_index = ui16{ 0 };
                    face_inst->GetGlyphIndices(&code_points, 1, &glyph_index);
                    face_inst->GetDesignGlyphMetrics(&glyph_index, 1, &glyph_metrics, faux);
                    facesize.y = (fp32)std::max(2, m.ascent + m.descent + m.lineGap);
                    facesize.x = glyph_metrics.advanceWidth ? (fp32)glyph_metrics.advanceWidth : facesize.y / 2;
                    ratio = facesize.x / facesize.y;
                    color = iscolor(face_inst);
                }
            }
            void recalc_metrics(twod& cellsize, bool isbase)
            {
                auto k0 = cellsize.y / facesize.y;
                auto b0 = base_ascent * k0;
                auto b_f = std::floor(b0);
                auto b_c = std::ceil(b0);
                auto asc_f = b_f;
                auto asc_c = b_c;
                auto des_f = cellsize.y - b_f;
                auto des_c = cellsize.y - b_c;
                auto k1_f = asc_f / base_ascent;
                auto k2_f = des_f / base_descent;
                auto k1_c = asc_c / base_ascent;
                auto k2_c = des_c / base_descent;
                auto m1 = std::max(k1_f, k2_f);
                auto m2 = std::max(k1_c, k2_c);
                auto b2 = fp32{};
                auto transform = fp32{};
                auto transform_letters = fp32{};
                if (m1 < m2)
                {
                    transform = m1;
                    b2 = b_f;
                }
                else
                {
                    transform = m2;
                    b2 = b_c;
                }
                auto base_line = fp2d{ 0.f, b2 };
                if (isbase)
                {
                    auto mx = facesize.x * transform;
                    auto dx = std::ceil(mx) - 1.f; // Grid fitting can move the glyph back more than 1px.
                    cellsize.x = std::max(1, (si32)dx);
                    transform_letters = std::min(transform, cellsize.x / facesize.x); // Respect letter width.
                }
                else
                {
                    transform = std::min(transform, cellsize.x  / facesize.x);
                    transform_letters = transform;
                }
                transform_letters = std::floor(base_x_height * transform_letters) / base_x_height; // Respect x-height.
                auto em_height = base_emheight * transform;
                auto em_height_letters = base_emheight * transform_letters;
                auto actual_sz = facesize * transform;
                //todo revise/optimize
                auto baseline_y = (si32)base_line.y;
                auto underline2 = twod{ std::clamp(baseline_y - (si32)std::round(base_underline.x * transform), 0, cellsize.y - 1), std::max(1, (si32)std::round(base_underline.y * transform)) };
                auto strikeout2 = twod{ std::clamp(baseline_y - (si32)std::round(base_strikeout.x * transform), 0, cellsize.y - 1), std::max(1, (si32)std::round(base_strikeout.y * transform)) };
                auto overline2 =  twod{ std::clamp(baseline_y - (si32)std::round(base_overline.x  * transform), 0, cellsize.y - 1), std::max(1, (si32)std::round(base_overline.y  * transform)) };
                auto vertpos = underline2.x;
                auto bheight = underline2.y;
                auto between = std::max(1, (bheight + 1) / 2);
                auto vtcoor2 = vertpos + bheight + between;
                auto oversize = vtcoor2 + bheight - cellsize.y;
                if (oversize > 0)
                {
                    vertpos -= oversize;
                    auto overpos = vertpos - (baseline_y + 1);
                    if (overpos < between)
                    {
                        auto half = overpos / 2;
                        if (half > 0) // Set equal distance between baseline/underline and line1/line2.
                        {
                            vertpos = baseline_y + 1 + half;
                            between = half;
                        }
                        else
                        {
                            vertpos = baseline_y + 2;
                            between = 1;
                            bheight = cellsize.y - vertpos - between;
                                 if (bheight >= 3) bheight /= 2;
                            else if (bheight == 2) bheight--;
                            else if (bheight == 1) vertpos--;
                            else
                            {
                                between = 0;
                                bheight = cellsize.y - vertpos;
                                if (bheight == 1) vertpos--;
                                else
                                {
                                    vertpos = std::min(vertpos - 1, underline2.x);
                                    bheight = 0;
                                }
                            }
                        }
                    }
                }
                auto doubline3 = rect{{ 0, vertpos }, { cellsize.x, std::max(1, between + bheight * 2) }};
                auto underline3 = rect{{ 0, underline2.x }, { cellsize.x, std::max(1, bheight) }};
                auto strikeout3 = rect{{ 0, strikeout2.x }, { cellsize.x, strikeout2.y }};
                auto od = overline2.y - underline3.size.y;
                auto overline3 = rect{{ 0, overline2.x + od }, underline3.size };
                auto dashpad_l = underline3.size.y;
                auto dashpad_r = underline3.size.y;
                auto dashpad_s = cellsize.x - dashpad_l * 2;
                if (dashpad_s < 1)
                {
                    dashpad_l = 1;
                    dashpad_s = std::max(1, cellsize.x - dashpad_l);
                    dashpad_r = std::max(0, cellsize.x - dashpad_l - dashpad_s);
                    dashpad_l = std::max(0, cellsize.x - dashpad_r - dashpad_s);
                }
                auto dashline3 = rect{{ dashpad_l, underline2.x }, { dashpad_s, underline3.size.y }};
                //log("font_name=", font_name, "\tasc=", base_ascent, "\tdes=", base_descent, "\tem=", base_emheight, "\tbasline=", b2, "\tdy=", transform, "\tk0=", k0, "\tm1=", m1, "\tm2=", m2);
                for (auto& f : fontface)
                {
                    f.base_line = base_line;
                    f.underline = underline3;
                    f.strikeout = strikeout3;
                    f.overline = overline3;
                    f.dashline = dashline3;
                    auto r1 = doubline3;
                    r1.size.y = underline3.size.y;
                    auto r2 = r1;
                    r2.coor.y += doubline3.size.y - r2.size.y;
                    f.doubline1 = r1;
                    f.doubline2 = r2;
                    f.wavyline = doubline3;
                }
                for (auto s : { style::normal, style::bold })
                {
                    fontface[s].transform = transform;
                    fontface[s].em_height = em_height;
                    fontface[s].actual_sz = actual_sz;
                    fontface[s].transform_letters = transform_letters;
                    fontface[s].em_height_letters = em_height_letters;
                }
                // Detect right bearing delta for italics.
                auto italic_glyph_metrics = DWRITE_GLYPH_METRICS{};
                auto normal_glyph_metrics = DWRITE_GLYPH_METRICS{};
                auto code_points = ui32{ 'M' };
                auto glyph_index = ui16{ 0 };
                fontface[style::normal].face_inst->GetGlyphIndices(&code_points, 1, &glyph_index);
                fontface[style::normal].face_inst->GetDesignGlyphMetrics(&glyph_index, 1, &normal_glyph_metrics, faux);
                fontface[style::italic].face_inst->GetGlyphIndices(&code_points, 1, &glyph_index);
                fontface[style::italic].face_inst->GetDesignGlyphMetrics(&glyph_index, 1, &italic_glyph_metrics, faux);
                auto proportional = normal_glyph_metrics.advanceWidth != (ui32)facesize.x;
                auto normal_width = normal_glyph_metrics.advanceWidth - normal_glyph_metrics.rightSideBearing;
                auto italic_width = italic_glyph_metrics.advanceWidth - italic_glyph_metrics.rightSideBearing;
                auto w = proportional && normal_width ? (fp32)normal_width : facesize.x;
                auto k = w / (w + (italic_width - normal_width));
                transform *= k;
                em_height *= k;
                transform_letters = std::floor(base_x_height * transform) / base_x_height; // Respect x-height.
                em_height_letters = base_emheight * transform_letters;
                actual_sz *= k;
                for (auto s : { style::italic, style::bold_italic })
                {
                    fontface[s].transform = transform;
                    fontface[s].em_height = em_height;
                    fontface[s].actual_sz = actual_sz;
                    fontface[s].transform_letters = transform_letters;
                    fontface[s].em_height_letters = em_height_letters;
                }
            }

            typeface() = default;
            typeface(typeface&&) = default;
            typeface(IDWriteFontFamily* barefont, ui32 index)
                : index{ index },
                  fixed{ true }
            {
                load(barefont);
            }
            typeface(IDWriteFontFamily* barefont, ui32 index, twod cellsz, bool isbase)
                : index{ index },
                  fixed{ faux }
            {
                load(barefont);
                recalc_metrics(cellsz, isbase);
            }
            ~typeface()
            {
                for (auto& f : fontface) if (f.face_inst) f.face_inst->Release();
            }
            explicit operator bool () { return index != ~0u; }
        };
        struct stat
        {
            ui64 s{};
            si32 i{};
            text n{};
        };
        IDWriteFactory2*               factory2; // font: DWrite factory.
        IDWriteFontCollection*         fontlist; // font: System font collection.
        IDWriteTextAnalyzer2*          analyzer; // font: Glyph indicies reader.
        std::vector<stat>              fontstat; // font: System font collection status list.
        std::vector<typeface>          fallback; // font: Fallback font list.
        wide                           oslocale; // font: User locale.
        flag                           complete; // font: Fallback index is ready.
        std::thread                    bgworker; // font: Background thread.
        twod                           cellsize; // font: Terminal cell size in pixels.
        std::list<text>                families; // font: Primary font name list.
        rect                           underline; // font: Single underline rectangle block within the cell.
        rect                           doubline1; // font: The first line of the double underline: at the top of the rect.
        rect                           doubline2; // font: The second line of the double underline: at the bottom.
        rect                           strikeout; // font: Strikethrough rectangle block within the cell.
        rect                           overline; // font: Overline rectangle block within the cell.
        rect                           dashline; // font: Dashed underline rectangle block within the cell.
        rect                           wavyline; // font: Wavy underline outer rectangle block within the cell.

        static auto msscript(ui32 code) // font: ISO<->MS script map.
        {
            static auto lut = []
            {
                auto map = std::vector<ui16>(1000, 999);
                if (auto f = (IDWriteFactory2*)nullptr; ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&f), f)
                {
                    if (auto a = (IDWriteTextAnalyzer1*)nullptr; f->CreateTextAnalyzer((IDWriteTextAnalyzer**)&a), a)
                    {
                        for (auto i = ui16{}; i < map.size(); i++)
                        {
                            auto prop = DWRITE_SCRIPT_PROPERTIES{};
                            a->GetScriptProperties(DWRITE_SCRIPT_ANALYSIS{ .script = i }, &prop);
                            if (i && prop.isoScriptNumber == 999) break;
                            map[prop.isoScriptNumber] = i;
                            auto code = view{ (char*)&prop.isoScriptCode, 4 };
                        }
                        a->Release();
                    }
                    f->Release();
                }
                return map;
            }();
            return lut[code];
        }

        void sort()
        {
            std::sort(fontstat.begin(), fontstat.end(), [](auto& a, auto& b){ return a.s > b.s; });
        }
        void set_fonts(auto family_names, bool fresh = true)
        {
            if (family_names.empty()) family_names.push_back("Courier New"); //todo unify
            families = family_names;
            fallback.clear();
            if (!fresh) // Restore the original font index order and clear the "loaded" flag.
            {
                auto tempstat = fontstat;
                auto src = fontstat.begin();
                auto end = fontstat.end();
                while (src != end)
                {
                    auto& s = *src++;
                    auto& d = tempstat[s.i];
                    d = s;
                    d.s &= ~fontcat::loaded;
                }
                std::swap(fontstat, tempstat);
            }
            for (auto& family_utf8 : families)
            {
                auto found = BOOL{};   
                auto index = ui32{};
                auto family_utf16 = utf::to_utf(family_utf8);
                fontlist->FindFamilyName(family_utf16.data(), &index, &found);
                if (found)
                {
                    if (fontstat[index].s & fontcat::loaded) continue; // Skip duplicates.
                    auto barefont = (IDWriteFontFamily*)nullptr;
                    fontlist->GetFontFamily(index, &barefont);
                    fontstat[index].s |= fontcat::loaded;
                    auto& f = fallback.emplace_back(barefont, index);
                    log("%%Using font '%fontname%' (%iscolor%). Order %index%.", prompt::gui, f.font_name, f.color ? "color" : "monochromatic", fallback.size() - 1);
                    barefont->Release();

                    //auto sa = DWRITE_SCRIPT_ANALYSIS{ .script = 24 };
                    //auto maxTagCount = ui32{100};
                    //auto tags = std::vector<DWRITE_FONT_FEATURE_TAG>(maxTagCount);
                    //analyzer->GetTypographicFeatures(fallback.back().fontface[0], sa, oslocale.data(), maxTagCount, &maxTagCount, tags.data());
                    //tags.resize(maxTagCount);
                    //log("\tfeat count: ", maxTagCount);
                    //for (auto t : tags) log("\t feat: ", view{ (char*)&t, 4 });
                }
                else log("%%Font '%fontname%' is not found in the system.", prompt::gui, family_utf8);
            }
            sort();
        }
        font(std::list<text>& family_names, si32 cell_height)
            : factory2{ (IDWriteFactory2*)[]{ auto f = (IUnknown*)nullptr; ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &f); return f; }() },
              fontlist{ [&]{ auto c = (IDWriteFontCollection*)nullptr; factory2->GetSystemFontCollection(&c, TRUE); return c; }() },
              analyzer{ [&]{ auto a = (IDWriteTextAnalyzer2*)nullptr; factory2->CreateTextAnalyzer((IDWriteTextAnalyzer**)&a); return a; }() },
              fontstat(fontlist ? fontlist->GetFontFamilyCount() : 0),
              oslocale(LOCALE_NAME_MAX_LENGTH, '\0'),
              complete{ faux }
        {
            if (!fontlist || !analyzer)
            {
                log("%%No fonts found in the system.", prompt::gui);
                return;
            }
            set_fonts(family_names);
            set_cellsz(cell_height);
            if (auto len = ::GetUserDefaultLocaleName(oslocale.data(), (si32)oslocale.size())) oslocale.resize(len);
            else
            {
                oslocale = L"en-US";
                log("%%Using default locale 'en-US'.", prompt::gui);
            }
            oslocale.shrink_to_fit();
            bgworker = std::thread{ [&]
            {
                for (auto i = 0u; i < fontstat.size(); i++)
                {
                    fontstat[i].i = i;
                    if (auto barefont = (IDWriteFontFamily*)nullptr; fontlist->GetFontFamily(i, &barefont), barefont)
                    {
                        if (auto fontfile = (IDWriteFont2*)nullptr; barefont->GetFirstMatchingFont(DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STRETCH_NORMAL, DWRITE_FONT_STYLE_NORMAL, (IDWriteFont**)&fontfile), fontfile)
                        {
                            fontstat[i].s |= fontcat::valid;
                            if (fontfile->IsMonospacedFont()) fontstat[i].s |= fontcat::monospaced;
                            if (auto face_inst = (IDWriteFontFace2*)nullptr; fontfile->CreateFontFace((IDWriteFontFace**)&face_inst), face_inst)
                            {
                                if (typeface::iscolor(face_inst)) fontstat[i].s |= fontcat::color;
                                auto numberOfFiles = ui32{};
                                face_inst->GetFiles(&numberOfFiles, nullptr);
                                auto fontFiles = std::vector<IDWriteFontFile*>(numberOfFiles);
                                if (S_OK == face_inst->GetFiles(&numberOfFiles, fontFiles.data()))
                                {
                                    if (numberOfFiles)
                                    if (auto f = fontFiles.front())
                                    {
                                        auto fontFileReferenceKey = (void const*)nullptr;
                                        auto fontFileReferenceKeySize = ui32{};
                                        f->GetReferenceKey(&fontFileReferenceKey, &fontFileReferenceKeySize);
                                        auto fontFileLoader = (IDWriteFontFileLoader*)nullptr;
                                        if (fontFileReferenceKeySize)
                                        if (f->GetLoader(&fontFileLoader); fontFileLoader)
                                        {
                                            auto fontFileStream = (IDWriteFontFileStream*)nullptr;
                                            if (fontFileLoader->CreateStreamFromKey(fontFileReferenceKey, fontFileReferenceKeySize, &fontFileStream); fontFileStream)
                                            {
                                                auto lastWriteTime = ui64{};
                                                fontFileStream->GetLastWriteTime(&lastWriteTime);
                                                fontstat[i].n = utf::to_utf((wchr*)fontFileReferenceKey);
                                                fontstat[i].s |= ~((ui64)0xFF << 60) & (lastWriteTime >> 4); // Sort fonts by iscolor, monospaced then by file_date.
                                                fontFileStream->Release();
                                            }
                                            fontFileLoader->Release();
                                        }
                                        f->Release();
                                    }
                                }
                                face_inst->Release();
                            }
                            fontfile->Release();
                        }
                        barefont->Release();
                    }
                }
                sort();
                //for (auto f : fontstat) log("id=", utf::to_hex(f.s), " i= ", f.i, " n=", f.n);
                complete.exchange(true);
                complete.notify_all();
                log("%%Font fallback index initialized.", prompt::gui);
            }};
        }
        ~font()
        {
            if (bgworker.joinable()) bgworker.join();
            if (analyzer) analyzer->Release();
            if (fontlist) fontlist->Release();
            if (factory2) factory2->Release();
        }
        void set_cellsz(si32 cell_height)
        {
            cellsize = { 1, std::clamp(cell_height, 2, 256) };
            auto base_font = true;
            for (auto& f : fallback) f.recalc_metrics(cellsize, std::exchange(base_font, faux));
            if (fallback.size()) // Keep the same *line positions for all fonts.
            {
                auto& f = fallback.front().fontface.front();
                underline = f.underline;
                strikeout = f.strikeout;
                doubline1 = f.doubline1;
                doubline2 = f.doubline2;
                overline  = f.overline;
                dashline  = f.dashline;
                wavyline  = f.wavyline;
            }
            log("%%Set cell size: ", prompt::gui, cellsize);
        }
        auto& take_font(utfx codepoint)
        {
            //return fallback.front();
            auto hittest = [&](auto& fontface)
            {
                if (!fontface) return faux;
                auto glyphindex = ui16{};
                fontface->GetGlyphIndices(&codepoint, 1, &glyphindex);
                return !!glyphindex;
            };
            for (auto& f : fallback) if ((f.color || f.fixed) && hittest(f.fontface[0].face_inst)) return f;
            for (auto& f : fallback) if ((!f.color && !f.fixed) && hittest(f.fontface[0].face_inst)) return f;
            complete.wait(faux);
            auto try_font = [&](auto i, bool test)
            {
                auto hit = faux;
                if (auto barefont = (IDWriteFontFamily*)nullptr; fontlist->GetFontFamily(i, &barefont), barefont)
                {
                    if (auto fontfile = (IDWriteFont2*)nullptr; barefont->GetFirstMatchingFont(DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STRETCH_NORMAL, DWRITE_FONT_STYLE_NORMAL, (IDWriteFont**)&fontfile), fontfile)
                    {
                        if (auto fontface = (IDWriteFontFace*)nullptr; fontfile->CreateFontFace(&fontface), fontface)
                        {
                            if (hittest(fontface) || !test)
                            {
                                hit = true;
                                fontstat[i].s |= fontcat::loaded;
                                auto& f = fallback.emplace_back(barefont, i, cellsize, faux);
                                log("%%Using font '%fontname%' (%iscolor%). Order %index%.", prompt::gui, f.font_name, f.color ? "color" : "monochromatic", fallback.size() - 1);
                            }
                            fontface->Release();
                        }
                        fontfile->Release();
                    }
                    barefont->Release();
                }
                return hit;
            };
            for (auto i = 0u; i < fontstat.size(); i++)
            {
                if (((fontstat[i].s & fontcat::valid) && !(fontstat[i].s & fontcat::loaded)) && try_font(fontstat[i].i, true)) return fallback.back();
            }
            if (fallback.size()) return fallback.front();
            for (auto i = 0u; i < fontstat.size(); i++) // Take the first font found in the system.
            {
                if ((fontstat[i].s & fontcat::valid) && try_font(fontstat[i].i, faux)) return fallback.back();
            }
            log("%%No fonts found in the system.", prompt::gui);
            return fallback.emplace_back(); // Should never happen.
        }
    };

    struct glyf
    {
        using irgb = netxs::irgb<fp32>;
        using vect = std::pmr::vector<byte>;
        struct sprite
        {
            static constexpr auto undef = 0;
            static constexpr auto alpha = 1; // Grayscale AA glyph alphamix. byte-based. fx: pixel = blend(pixel, fgc, byte).
            static constexpr auto color = 2; // irgb-colored glyph colormix. irgb-based. fx: pixel = blend(blend(pixel, irgb.alpha(irgb.chan.a - (si32)irgb.chan.a)), fgc, (si32)irgb.chan.a - 256).

            vect bits; // sprite: Contains: type=alpha: bytes [0-255]; type=color: irgb<fp32>.
            rect area; // sprite: Glyph mask black-box.
            si32 type; // sprite: Glyph mask type.
            sprite(auto& pool)
                : bits{ &pool },
                  type{ undef }
            { }
            template<class Elem>
            auto raster()
            {
                return netxs::raster{ std::span{ (Elem*)bits.data(), bits.size() / sizeof(Elem) }, area };
            }
        };
        struct color_layer
        {
            vect bits; // color_layer: Layer pixels (8-bit grayscale).
            rect area; // color_layer: Layer black-box.
            irgb fill; // color_layer: Layer's sRGB color.
            color_layer(auto& pool)
                : bits{ &pool },
                  fill{       }
            { }
        };

        using gmap = std::unordered_map<ui64, sprite>;

        std::pmr::unsynchronized_pool_resource buffer_pool; // glyf: Pool for temp buffers.
        std::pmr::monotonic_buffer_resource    mono_buffer; // glyf: Memory block for sprites.
        font& fcache; // glyf: Font cache.
        twod& cellsz; // glyf: Terminal cell size in pixels.
        bool  aamode; // glyf: Enable AA.
        gmap  glyphs; // glyf: Glyph map.
        wide                                         text_utf16; // glyf: UTF-16 buffer.
        std::vector<utf::prop>                       codepoints; // glyf: .
        std::vector<ui16>                            clustermap; // glyf: .
        std::vector<ui16>                            glyf_index; // glyf: .
        std::vector<FLOAT>                           glyf_steps; // glyf: .
        std::vector<DWRITE_GLYPH_OFFSET>             glyf_align; // glyf: .
        std::vector<DWRITE_GLYPH_METRICS>            glyf_sizes; // glyf: .
        std::vector<DWRITE_SHAPING_GLYPH_PROPERTIES> glyf_props; // glyf: .
        std::vector<DWRITE_SHAPING_TEXT_PROPERTIES>  text_props; // glyf: .
        std::vector<color_layer>                     glyf_masks; // glyf: .

        glyf(font& fcache, bool aamode)
            : fcache{ fcache },
              cellsz{ fcache.cellsize },
              aamode{ aamode }
        { }
        void reset()
        {
            glyphs.clear();
            mono_buffer.release();
        }
        void rasterize(sprite& glyph_mask, cell const& c)
        {
            glyph_mask.type = sprite::alpha;
            if (c.xy() == 0) return;
            auto code_iter = utf::cpit{ c.txt() };
            codepoints.clear();
            auto flipandrotate = 0;
            auto monochromatic = faux;
            auto glyfalignment = bind{ snap::none, snap::none };
            while (code_iter)
            {
                auto codepoint = code_iter.next();
                if (codepoint.cdpoint >= utf::vs04_code && codepoint.cdpoint <= utf::vs16_code)
                {
                         if (codepoint.cdpoint == utf::vs15_code) monochromatic = true;
                    else if (codepoint.cdpoint == utf::vs16_code) monochromatic = faux;
                    else if (codepoint.cdpoint == utf::vs10_code) flipandrotate = (flipandrotate & 0b100) | ((flipandrotate + 0b001) & 0b011); // +90°  CCW
                    else if (codepoint.cdpoint == utf::vs11_code) flipandrotate = (flipandrotate & 0b100) | ((flipandrotate + 0b010) & 0b011); // +180° CCW
                    else if (codepoint.cdpoint == utf::vs12_code) flipandrotate = (flipandrotate & 0b100) | ((flipandrotate + 0b011) & 0b011); // +270° CCW
                    else if (codepoint.cdpoint == utf::vs13_code) flipandrotate = (flipandrotate ^ 0b100) | ((flipandrotate + (flipandrotate & 1 ? 0b010 : 0)) & 0b011); // Hz flip
                    else if (codepoint.cdpoint == utf::vs14_code) flipandrotate = (flipandrotate ^ 0b100) | ((flipandrotate + (flipandrotate & 1 ? 0 : 0b010)) & 0b011); // Vt flip
                    else if (codepoint.cdpoint == utf::vs04_code) glyfalignment.x = snap::head;
                    else if (codepoint.cdpoint == utf::vs05_code) glyfalignment.x = snap::center;
                    else if (codepoint.cdpoint == utf::vs06_code) glyfalignment.x = snap::tail;
                    else if (codepoint.cdpoint == utf::vs07_code) glyfalignment.y = snap::head;
                    else if (codepoint.cdpoint == utf::vs08_code) glyfalignment.y = snap::center;
                    else if (codepoint.cdpoint == utf::vs09_code) glyfalignment.y = snap::tail;
                }
                else codepoints.push_back(codepoint);
            }
            if (codepoints.empty()) return;

            auto format = font::style::normal;
            if (c.itc()) format |= font::style::italic;
            if (c.bld()) format |= font::style::bold;
            auto base_char = codepoints.front().cdpoint;
            auto& f = fcache.take_font(base_char);
            auto face_inst = f.fontface[format].face_inst;
            if (!face_inst) return;
            auto is_box_drawing = (base_char >= 0x2320  && base_char <= 0x23D0)   // ⌠ ⌡ ... ⎛ ⎜ ⎝ ⎞ ⎟ ⎠ ⎡ ⎢ ⎣ ⎤ ⎥ ⎦ ⎧ ⎨ ⎩ ⎪ ⎫ ⎬ ⎭ ⎮ ⎯ ⎰ ⎱ ⎲ ⎳ ⎴ ⎵ ⎶ ⎷ ⎸ ⎹ ... ⏐
                               || (base_char >= 0x2500  && base_char <= 0x25FF)   // Box Elements
                               || (base_char >= 0xE0B0  && base_char <= 0xE0B3)   // Powerline Arrows
                               || (base_char >= 0x1CC00 && base_char <= 0x1CEBF)  // Legacy Computing Supplement. inc Large Type Pieces: U+1CE1A-1CE50
                               || (base_char >= 0x1F67C && base_char <= 0x1F67F)  // Ornamental Dingbats: U+1F67C-1F67F 🙼 🙽 🙾 🙿
                               || (base_char >= 0x1FB00 && base_char <= 0x1FBFF); // Symbols for Legacy Computing
            auto transform = is_box_drawing ? f.fontface[format].transform : f.fontface[format].transform_letters;
            auto em_height = is_box_drawing ? f.fontface[format].em_height : f.fontface[format].em_height_letters;
            auto base_line = f.fontface[format].base_line;
            auto actual_sz = f.fontface[format].actual_sz;

            //todo use otf tables directly: GSUB etc
            //gindex.resize(codepoints.size());
            //hr = face_inst->GetGlyphIndices(codepoints.data(), (ui32)codepoints.size(), gindex.data());
            //auto glyph_run = DWRITE_GLYPH_RUN{ .fontFace     = face_inst,
            //                                   .fontEmSize   = em_height,
            //                                   .glyphCount   = (ui32)gindex.size(),
            //                                   .glyphIndices = gindex.data() };
            text_utf16.clear();
            utf::to_utf(codepoints, text_utf16);
            auto text_count = (ui32)text_utf16.size();
            auto glyf_count = 3 * text_count / 2 + 16;
            glyf_index.resize(glyf_count);
            glyf_props.resize(glyf_count);
            text_props.resize(text_count);
            clustermap.resize(text_count);

            //todo make it configurable (and face_inst based)
            //auto fs = std::to_array<std::pair<ui32, ui32>>({ { DWRITE_MAKE_OPENTYPE_TAG('s', 'a', 'l', 't'), 1 }, });
            //auto const features = std::to_array({ DWRITE_TYPOGRAPHIC_FEATURES{ (DWRITE_FONT_FEATURE*)fs.data(), (ui32)fs.size() }});
            //auto feat_table = features.data();

            auto script_opt = DWRITE_SCRIPT_ANALYSIS{ .script = font::msscript(unidata::script(codepoints.front().cdpoint)) };
            auto hr = fcache.analyzer->GetGlyphs(text_utf16.data(),       //_In_reads_(textLength) WCHAR const* textString,
                                                 text_count,              //UINT32 textLength,
                                                 face_inst,               //_In_ IDWriteFontFace* fontFace,
                                                 faux,                    //BOOL isSideways,
                                                 faux,                    //BOOL isRightToLeft,
                                                 &script_opt,             //_In_ DWRITE_SCRIPT_ANALYSIS const* scriptAnalysis,
                                                 fcache.oslocale.data(),  //_In_opt_z_ WCHAR const* localeName,
                                                 nullptr,                 //_In_opt_ IDWriteNumberSubstitution* numberSubstitution,
                                                 nullptr,//&f.feat_table, //_In_reads_opt_(featureRanges) DWRITE_TYPOGRAPHIC_FEATURES const** features,
                                                 &text_count,             //_In_reads_opt_(featureRanges) UINT32 const* featureRangeLengths,
                                                 0,//f.features.size(),   //UINT32 featureRanges,
                                                 glyf_count,              //UINT32 maxGlyphCount,
                                                 clustermap.data(),       //_Out_writes_(textLength) UINT16* clusterMap,
                                                 text_props.data(),       //_Out_writes_(textLength) DWRITE_SHAPING_TEXT_PROPERTIES* textProps,
                                                 glyf_index.data(),       //_Out_writes_(maxGlyphCount) UINT16* glyphIndices,
                                                 glyf_props.data(),       //_Out_writes_(maxGlyphCount) DWRITE_SHAPING_GLYPH_PROPERTIES* glyphProps,
                                                 &glyf_count);            //_Out_ UINT32* actualGlyphCount
            if (hr != S_OK) return;

            glyf_steps.resize(glyf_count);
            glyf_align.resize(glyf_count);
            glyf_sizes.resize(glyf_count);
            auto actual_height = (fp32)cellsz.y;
            auto mtx = c.mtx();
            auto matrix = fp2d{ mtx * cellsz };
            auto swapxy = flipandrotate & 1;
            if (swapxy)
            {
                std::swap(matrix.x, matrix.y);
                transform *= f.ratio;
                em_height *= f.ratio;
                base_line *= f.ratio;
                actual_height *= f.ratio;
            }
            hr = fcache.analyzer->GetGlyphPlacements(text_utf16.data(),       // _In_reads_(textLength) WCHAR const* textString,
                                                     clustermap.data(),       // _In_reads_(textLength) UINT16 const* clusterMap,
                                                     text_props.data(),       // _Inout_updates_(textLength) DWRITE_SHAPING_TEXT_PROPERTIES* textProps,
                                                     text_count,              // UINT32 textLength,
                                                     glyf_index.data(),       // _In_reads_(glyphCount) UINT16 const* glyphIndices,
                                                     glyf_props.data(),       // _In_reads_(glyphCount) DWRITE_SHAPING_GLYPH_PROPERTIES const* glyphProps,
                                                     glyf_count,              // UINT32 glyphCount,
                                                     face_inst,               // _In_ IDWriteFontFace* fontFace,
                                                     em_height,               // FLOAT fontEmSize,
                                                     faux,                    // BOOL isSideways,
                                                     faux,                    // BOOL isRightToLeft,
                                                     &script_opt,             // _In_ DWRITE_SCRIPT_ANALYSIS const* scriptAnalysis,
                                                     fcache.oslocale.data(),  // _In_opt_z_ WCHAR const* localeName,
                                                     nullptr,//&f.feat_table, // _In_reads_opt_(featureRanges) DWRITE_TYPOGRAPHIC_FEATURES const** features,
                                                     &text_count,             // _In_reads_opt_(featureRanges) UINT32 const* featureRangeLengths,
                                                     0,//f.features.size(),   // UINT32 featureRanges,
                                                     glyf_steps.data(),       // _Out_writes_(glyphCount) FLOAT* glyphAdvances,
                                                     glyf_align.data());      // _Out_writes_(glyphCount) DWRITE_GLYPH_OFFSET* glyphOffsets
            if (hr != S_OK) return;

            hr = face_inst->GetDesignGlyphMetrics(glyf_index.data(), glyf_count, glyf_sizes.data(), faux);
            if (hr != S_OK) return;
            auto length = fp32{};
            auto penpos = fp32{};
            for (auto i = 0u; i < glyf_count; ++i)
            {
                auto w = glyf_sizes[i].advanceWidth;
                auto r = glyf_sizes[i].rightSideBearing;
                auto bearing = ((si32)w - r) * transform;
                auto right_most = penpos + glyf_align[i].advanceOffset + bearing;
                length = std::max(length, right_most);
                penpos += glyf_steps[i];
            }
            auto actual_width = swapxy ? length :
                                is_box_drawing ? std::max(1.f, std::floor((length / cellsz.x))) * cellsz.x
                                               : std::max(1.f, std::ceil(((length - 0.1f * cellsz.x) / cellsz.x))) * cellsz.x;
            auto k = 1.f;
            if (actual_width > matrix.x) // Check if the glyph exceeds the matrix width. (scale down)
            {
                k = matrix.x / length;
                actual_width = matrix.x;
                actual_height *= k;
                em_height *= k;
                for (auto& w : glyf_steps) w *= k;
                for (auto& [h, v] : glyf_align) h *= k;
            }
            else if (actual_height < matrix.y || actual_width < matrix.x) // Check if the glyph is too small for the matrix. (scale up)
            {
                k = std::min(matrix.x / actual_width, matrix.y / actual_height);
                actual_width *= k;
                actual_height *= k;
                base_line *= k;
                em_height *= k;
                for (auto& w : glyf_steps) w *= k;
                for (auto& [h, v] : glyf_align) h *= k;
                k = 1.f;
            }
            if (glyfalignment.x != snap::none && actual_width < matrix.x)
            {
                     if (glyfalignment.x == snap::center) base_line.x += (matrix.x - actual_width) / 2.f;
                else if (glyfalignment.x == snap::tail  ) base_line.x += matrix.x - actual_width;
                //else if (glyfalignment.x == snap::head  ) base_line.x = 0;
            }
            if (glyfalignment.y != snap::none && actual_height < matrix.y)
            {
                base_line.y *= k;
                     if (glyfalignment.y == snap::center) base_line.y += (matrix.y - actual_height) / 2.f;
                else if (glyfalignment.y == snap::tail  ) base_line.y += matrix.y - actual_height;
                //else if (glyfalignment.y == snap::head  ) base_line.y *= k;
            }
            auto glyph_run = DWRITE_GLYPH_RUN{ .fontFace      = face_inst,
                                               .fontEmSize    = em_height,
                                               .glyphCount    = glyf_count,
                                               .glyphIndices  = glyf_index.data(),
                                               .glyphAdvances = glyf_steps.data(),
                                               .glyphOffsets  = glyf_align.data() };
            auto colored_glyphs = (IDWriteColorGlyphRunEnumerator*)nullptr;
            auto measuring_mode = DWRITE_MEASURING_MODE_NATURAL;
            hr = monochromatic ? DWRITE_E_NOCOLOR
                               : fcache.factory2->TranslateColorGlyphRun(base_line.x, base_line.y, &glyph_run, nullptr, measuring_mode, nullptr, 0, &colored_glyphs);
            auto rendering_mode = aamode || colored_glyphs ? DWRITE_RENDERING_MODE_NATURAL : DWRITE_RENDERING_MODE_ALIASED;
            auto pixel_fit_mode = is_box_drawing && cellsz.y > 20 ? DWRITE_GRID_FIT_MODE_DISABLED // Grid-fitting breaks box-drawing linkage.
                                                                  : DWRITE_GRID_FIT_MODE_ENABLED;
            auto aaliasing_mode = DWRITE_TEXT_ANTIALIAS_MODE_GRAYSCALE;
            auto create_texture = [&](auto& run, auto& mask, auto base_line_x, auto base_line_y)
            {
                auto rasterizer = (IDWriteGlyphRunAnalysis*)nullptr;
                if (S_OK == fcache.factory2->CreateGlyphRunAnalysis(&run, nullptr, rendering_mode, measuring_mode, pixel_fit_mode, aaliasing_mode, base_line_x, base_line_y, &rasterizer))
                {
                    auto r = RECT{};
                    if (S_OK == rasterizer->GetAlphaTextureBounds(DWRITE_TEXTURE_ALIASED_1x1, &r))
                    {
                        mask.area = {{ r.left, r.top }, { r.right - r.left, r.bottom - r.top }};
                        if (mask.area.size)
                        {
                            mask.bits.resize(mask.area.size.x * mask.area.size.y);
                            hr = rasterizer->CreateAlphaTexture(DWRITE_TEXTURE_ALIASED_1x1, &r, mask.bits.data(), (ui32)mask.bits.size());
                        }
                    }
                    rasterizer->Release();
                }
            };
            if (colored_glyphs)
            {
                glyph_mask.bits.clear();
                glyph_mask.type = sprite::color;
                auto exist = BOOL{ true };
                auto layer = (DWRITE_COLOR_GLYPH_RUN const*)nullptr;
                glyf_masks.clear();
                while (colored_glyphs->MoveNext(&exist), exist && S_OK == colored_glyphs->GetCurrentRun(&layer))
                {
                    auto& m = glyf_masks.emplace_back(buffer_pool);
                    create_texture(layer->glyphRun, m, layer->baselineOriginX, layer->baselineOriginY);
                    if (m.area)
                    {
                        auto u = layer->runColor;
                        m.fill = layer->paletteIndex != -1 ? irgb{ std::isnormal(u.r) ? u.r : 0.f,
                                                                   std::isnormal(u.g) ? u.g : 0.f,
                                                                   std::isnormal(u.b) ? u.b : 0.f,
                                                                   std::isnormal(u.a) ? u.a : 0.f }.sRGB2Linear() : irgb{}; // runColor.bgra could be nan != 0.
                        //test fgc
                        //if (m.fill.r == 0 && m.fill.g == 0 && m.fill.b == 0) m.fill = {};
                    }
                    else glyf_masks.pop_back();
                }
                glyph_mask.area = {};
                for (auto& m : glyf_masks) glyph_mask.area |= m.area;
                auto l = glyph_mask.area.size.x * glyph_mask.area.size.y;
                glyph_mask.bits.resize(l * sizeof(irgb));
                auto raster = glyph_mask.raster<irgb>();
                for (auto& m : glyf_masks)
                {
                    auto alpha_mask = netxs::raster{ m.bits, m.area };
                    if (m.fill.a != 0.f) // Predefined sRGB color.
                    {
                        netxs::onbody(raster, alpha_mask, [fill = m.fill](irgb& dst, byte& alpha)
                        {
                            if (dst.a >= 256.f) // Update the fgc layer if it exists. dst.a consists of two parts: an integer that represents the fgc alpha in 8-bit format, and a floating point normalized [0.0-1.0] value that represents the alpha for the color glyph sprite.
                            {
                                auto fgc_alpha = (si32)dst.a;
                                dst.a -= fgc_alpha;
                                dst.blend_nonpma(fill, alpha);
                                if (alpha != 255 && fgc_alpha > 256) dst.a += 256 + (si32)netxs::saturate_cast<byte>(fgc_alpha - 256) * (255 - alpha) / 255;
                            }
                            else dst.blend_nonpma(fill, alpha);
                        });
                    }
                    else // Foreground color unknown in advance. Side-effect: fully transparent glyph layers will be colored with the fgc color.
                    {
                        netxs::onbody(raster, alpha_mask, [](irgb& dst, byte& alpha)
                        {
                                 if (alpha == 255) dst = { 0.f, 0.f, 0.f, 256.f + 255.f };
                            else if (alpha != 0)
                            {
                                static constexpr auto kk = (si32)netxs::saturate_cast<byte>(- 256.f);
                                auto fgc_alpha = (si32)netxs::saturate_cast<byte>(dst.a - 256.f);
                                dst.a = dst.a + (-(si32)dst.a + 256 + alpha + (255 - alpha) * fgc_alpha / 255);
                            }
                        });
                    }
                }
                colored_glyphs->Release();
            }
            else if (hr == DWRITE_E_NOCOLOR) create_texture(glyph_run, glyph_mask, base_line.x, base_line.y);
            //auto src_bitmap = glyph_mask.raster<byte>();
            //auto bline = rect{base_line, { cellsz.x, 1 } };
            //netxs::onrect(src_bitmap, bline, [](auto& c){ c = std::min(255, c + 64); });
            if (glyph_mask.area && flipandrotate)
            {
                //todo optimize
                static auto buffer = std::vector<byte>{};
                static constexpr auto l0 = std::to_array({ 1, -1, -1,  1, -1, 1,  1, -1 });
                static constexpr auto l1 = std::to_array({ 1,  1, -1, -1,  1, 1, -1, -1 });
                buffer.assign(glyph_mask.bits.begin(), glyph_mask.bits.end());
                auto xform = [&](auto elem)
                {
                    using type = decltype(elem);
                    auto count = buffer.size() / sizeof(type);
                    auto src = netxs::raster{ std::span{ (type*)buffer.data(), count }, glyph_mask.area };
                    auto mx = glyph_mask.area.size.x;
                    if (swapxy)
                    {
                        std::swap(glyph_mask.area.size.x, glyph_mask.area.size.y);
                        std::swap(glyph_mask.area.coor.x, glyph_mask.area.coor.y);
                    }
                    auto dst = glyph_mask.raster<type>();
                    auto s__dx = 1;
                    auto s__dy = mx;
                    auto dmx = glyph_mask.area.size.x;
                    auto dmy = glyph_mask.area.size.y;
                    auto sx = l0[flipandrotate];
                    auto sy = l1[flipandrotate];
                    auto d__dx = sx * ((flipandrotate & 0b1) ? dmx :   1);
                    auto d__dy = sy * ((flipandrotate & 0b1) ? 1   : dmx);
                    if (flipandrotate & 0b100) std::swap(sx, sy);
                    auto d__px = (sy > 0 ? 0 : dmx - 1);
                    auto d__py = (sx > 0 ? 0 : dmy - 1);
                    auto s_beg = src.begin();
                    auto s_eol = s_beg + mx - 1;
                    auto s_end = s_beg + count - 1;
                    auto d_beg = dst.begin() + (d__px + d__py * dmx);
                    auto d_eol = d_beg + d__dx * (mx - 1);
                    auto s_ptr = s_beg;
                    auto d_ptr = d_beg;
                    while (true)
                    {
                        *d_ptr = *s_ptr;
                        if (s_ptr != s_eol) s_ptr += s__dx;
                        else
                        {
                            if (s_ptr == s_end) break;
                            s_beg += s__dy;
                            s_ptr = s_beg;
                            s_eol += s__dy;
                        }
                        if (d_ptr != d_eol) d_ptr += d__dx;
                        else
                        {
                            d_beg += d__dy;
                            d_ptr = d_beg;
                            d_eol += d__dy;
                        }
                    }
                };
                glyph_mask.type == sprite::color ? xform(irgb{}) : xform(byte{});
            }
        }
        void draw_cell(auto& canvas, auto& blinks, twod coor, cell const& c)
        {
            auto placeholder = canvas.area().trim(rect{ coor, cellsz });
            if (!placeholder) return;
            auto fgc = c.fgc();
            auto bgc = c.bgc();
            if (c.inv()) std::swap(fgc, bgc);
            canvas.clip(placeholder);
            if (c.blk())
            {
                placeholder.coor -= blinks.coor();
                blinks.clip(placeholder);
                if (bgc.alpha()) // Fill the blinking layer's background to fix DWM that doesn't take gamma into account during layered window blending.
                {
                    netxs::onclip(canvas, blinks, [&](auto& dst, auto& src){ dst = bgc; src = bgc; });
                }
            }
            else if (bgc.alpha()) netxs::onrect(canvas, placeholder, cell::shaders::full(bgc));
            auto& target = c.blk() ? blinks : canvas;
            if (auto u = c.und())
            {
                auto index = c.unc();
                auto color = index ? argb{ argb::vt256[index] }.alpha(c.fga()) : c.fgc();
                if (u == unln::line)
                {
                    auto block = fcache.underline;
                    block.coor += placeholder.coor + target.coor();
                    netxs::onrect(target, block, cell::shaders::full(color));
                }
                else if (u == unln::dotted)
                {
                    auto block = fcache.underline;
                    block.coor += placeholder.coor + target.coor();
                    auto limit = block.coor.x + block.size.x;
                    block.size.x = std::max(2, block.size.y);
                    auto stepx = 3 * block.size.x;
                    block.coor.x -= placeholder.coor.x % stepx;
                    while (block.coor.x < limit)
                    {
                        netxs::onrect(target, block.trim(placeholder), cell::shaders::full(color));
                        block.coor.x += stepx;
                    }
                }
                else if (u == unln::dashed)
                {
                    auto block = fcache.dashline;
                    block.coor += placeholder.coor + target.coor();
                    netxs::onrect(target, block, cell::shaders::full(color));
                }
                else if (u == unln::biline)
                {
                    auto b1 = fcache.doubline1;
                    auto b2 = fcache.doubline2;
                    auto offset = placeholder.coor + target.coor();
                    b1.coor += offset;
                    b2.coor += offset;
                    netxs::onrect(target, b1, cell::shaders::full(color));
                    netxs::onrect(target, b2, cell::shaders::full(color));
                }
                else if (u == unln::wavy)
                {
                    //todo optimize
                    auto block = fcache.wavyline;
                    block.coor += placeholder.coor + target.coor();
                    auto width = fcache.underline.size.y;
                    auto vsize = std::max(1, block.size.y - width + 1) / 2.f;
                    auto y0 = block.coor.y + vsize;
                    vsize *= 0.99f;
                    auto start = block.coor.x;
                    auto limit = start + block.size.x;
                    auto fract = width * 3 + width % 2;
                    auto k = 3.14159265359f / 2.f / fract;
                    block.size.x = 1;
                    block.size.y = width;
                    block.coor.x -= block.coor.x % (fract * 4);
                    while (block.coor.x < limit)
                    {
                        for (auto x = 0; x < fract; x++)
                        {
                            auto p = block;
                            p.coor.y = (si32)(y0 - std::sin(k * (x)) * vsize - 0.00001f);
                            netxs::onrect(target, p.trim(placeholder), cell::shaders::full(color));
                            p.coor.x += fract;
                            p.coor.y = (si32)(y0 - std::sin(k * (fract - x)) * vsize - 0.00001f);
                            netxs::onrect(target, p.trim(placeholder), cell::shaders::full(color));
                            p.coor.x += fract;
                            p.coor.y = (si32)(y0 + std::sin(k * (x)) * vsize + 0.00001f);
                            netxs::onrect(target, p.trim(placeholder), cell::shaders::full(color));
                            p.coor.x += fract;
                            p.coor.y = (si32)(y0 + std::sin(k * (fract - x)) * vsize + 0.00001f);
                            netxs::onrect(target, p.trim(placeholder), cell::shaders::full(color));
                            block.coor.x++;
                        }
                        block.coor.x += fract * 3;
                    }
                }
                else
                {
                    auto block = fcache.underline;
                    block.coor += placeholder.coor + target.coor();
                    netxs::onrect(target, block, cell::shaders::full(color));
                }
            }
            if (c.stk())
            {
                auto color = c.fgc();
                auto block = fcache.strikeout;
                block.coor += placeholder.coor + target.coor();
                netxs::onrect(target, block, cell::shaders::full(color));
            }
            if (c.ovr())
            {
                auto color = c.fgc();
                auto block = fcache.overline;
                block.coor += placeholder.coor + target.coor();
                netxs::onrect(target, block, cell::shaders::full(color));
            }
            if (c.xy() == 0) return;
            auto token = c.tkn() & ~3;
            if (c.itc()) token |= font::style::italic;
            if (c.bld()) token |= font::style::bold;
            auto iter = glyphs.find(token);
            if (iter == glyphs.end())
            {
                iter = glyphs.emplace(token, mono_buffer).first;
                rasterize(iter->second, c);
            }
            auto& glyph_mask = iter->second;
            if (!glyph_mask.area) return;

            auto [w, h, x, y] = c.whxy();
            if (x == 0 || y == 0) return;
            auto box = glyph_mask.area.shift(placeholder.coor - twod{ cellsz.x * (x - 1), cellsz.y * (y - 1) });

            auto f_fgc = irgb{ fgc }.sRGB2Linear();
            if (glyph_mask.type == sprite::color)
            {
                auto fx = [fgc, f_fgc](argb& dst, irgb src)
                {
                         if (src.a == 0.f) return;
                    else if (src.a == 1.f) dst = src.linear2sRGB();
                    else if (src.a < 256.f + 255.f)
                    {
                        auto f_dst = irgb{ dst }.sRGB2Linear();
                        if (src.a > 256.f) // Alpha contains non-zero integer for fgc's aplha.
                        {
                            auto fgc_alpha = netxs::saturate_cast<byte>(src.a - 256.f);
                            src.a -= (si32)src.a;
                            f_dst.blend_nonpma(f_fgc, fgc_alpha);
                        }
                        dst = f_dst.blend_pma(src).linear2sRGB();
                    }
                    else dst = fgc; // src.a >= 256 + 255.f
                };
                auto raster = netxs::raster{ std::span{ (irgb*)glyph_mask.bits.data(), (size_t)glyph_mask.area.length() }, box };
                netxs::onclip(target, raster, fx);
            }
            else
            {
                auto fx = [fgc, f_fgc](argb& dst, byte src)
                {
                         if (src == 0) return;
                    else if (src == 255) dst = fgc;
                    else
                    {
                        auto f_dst = irgb{ dst }.sRGB2Linear();;
                        dst = f_dst.blend_nonpma(f_fgc, src).linear2sRGB();
                    }
                };
                auto raster = netxs::raster{ glyph_mask.bits, box };
                netxs::onclip(target, raster, fx);
            }
        }
        void fill_grid(auto& canvas, auto& blinks, si32& blink_count, twod origin, auto& grid_cells)
        {
            auto coor = origin;
            auto maxc = coor + grid_cells.size() * cellsz;
            auto base = canvas.coor();
            auto base_blinks = blinks.coor();
            blinks.move(origin);
            canvas.step(-base);
            for (auto& c : grid_cells)
            {
                if (c.blk()) blink_count++;//todo sync with blink_synch
                draw_cell(canvas, blinks, coor, c);
                coor.x += cellsz.x;
                if (coor.x >= maxc.x)
                {
                    coor.x = origin.x;
                    coor.y += cellsz.y;
                    if (coor.y >= maxc.y) break;
                }
            }
            canvas.step(base);
            blinks.move(base_blinks);
        }
    };

    struct surface : surface_base
    {
        static constexpr auto hidden = twod{ -32000, -32000 };

        HDC   hdc; // surface: .
        HWND hWnd; // surface: .

        surface(surface const&) = default;
        surface(surface&&) = default;
        surface(HWND hWnd)
            :  hdc{ ::CreateCompatibleDC(NULL)}, // Only current thread owns hdc.
              hWnd{ hWnd }
        { }
        void reset() // We don't use custom copy/move ctors.
        {
            if (hdc) ::DeleteDC(hdc);
            for (auto eventid : klok) ::KillTimer(hWnd, eventid);
        }
        auto canvas(bool wipe = faux)
        {
            if (hdc && area)
            {
                if (area.size != prev.size)
                {
                    auto ptr = (void*)nullptr;
                    auto bmi = BITMAPINFO{ .bmiHeader = { .biSize        = sizeof(BITMAPINFOHEADER),
                                                          .biWidth       = area.size.x,
                                                          .biHeight      = -area.size.y,
                                                          .biPlanes      = 1,
                                                          .biBitCount    = 32,
                                                          .biCompression = BI_RGB }};
                    if (auto hbm = ::CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &ptr, 0, 0)) // 0.050 ms
                    {
                        ::DeleteObject(::SelectObject(hdc, hbm));
                        wipe = faux;
                        prev.size = area.size;
                        data = bits{ std::span<argb>{ (argb*)ptr, (sz_t)area.size.x * area.size.y }, area };
                    }
                    else log("%%Compatible bitmap creation error: %ec%", prompt::gui, ::GetLastError());
                }
                if (wipe) std::memset(data.data(), 0, (sz_t)area.size.x * area.size.y * sizeof(argb));
            }
            data.move(area.coor);
            return data;
        }
        void present()
        {
            if (!hdc) return;
            auto windowmoved = prev.coor(live ? area.coor : hidden);
            if (!windowmoved && (sync.empty() || !live)) return;
            if (!live) // Hide window. Windows Server Core doesn't hide windows by ShowWindow(). Details: https://devblogs.microsoft.com/oldnewthing/20041028-00/?p=37453.
            {
                ::SetWindowPos(hWnd, 0, hidden.x, hidden.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOSENDCHANGING | SWP_NOACTIVATE);
                return;
            }
            auto blend_props = BLENDFUNCTION{ .BlendOp = AC_SRC_OVER, .SourceConstantAlpha = 255, .AlphaFormat = AC_SRC_ALPHA };
            auto bitmap_coor = POINT{};
            auto window_coor = POINT{ area.coor.x, area.coor.y };
            auto bitmap_size = SIZE{ area.size.x, area.size.y };
            auto update_area = RECT{};
            auto update_info = UPDATELAYEREDWINDOWINFO{ .cbSize   = sizeof(UPDATELAYEREDWINDOWINFO),
                                                        .pptDst   = windowmoved ? &window_coor : nullptr,
                                                        .psize    = &bitmap_size,
                                                        .hdcSrc   = hdc,
                                                        .pptSrc   = &bitmap_coor,
                                                        .pblend   = &blend_props,
                                                        .dwFlags  = ULW_ALPHA,
                                                        .prcDirty = &update_area };
            //log("hWnd=", hWnd);
            auto update_proc = [&]
            {
                //log("\t", rect{{ update_area.left, update_area.top }, { update_area.right - update_area. left, update_area.bottom - update_area.top }});
                auto ok = ::UpdateLayeredWindowIndirect(hWnd, &update_info);
                if (!ok) log("%%UpdateLayeredWindowIndirect call failed", prompt::gui);
            };
            //todo revise/optimize
            //if (sync.size() > 100) // Full redraw if too much changes.
            //{
            //    update_area = RECT{ .right = size.x, .bottom = size.y };
            //    update_proc();
            //}
            //else
            {
                for (auto r : sync)
                {
                    update_area = { r.coor.x, r.coor.y, r.coor.x + r.size.x, r.coor.y + r.size.y };
                    update_proc();
                    update_info.pptDst = {};
                }
                if (update_info.pptDst) // Just move window.
                {
                    update_area = {};
                    update_proc();
                }
            }
            sync.clear();
        }
        void start_timer(span elapse, ui32 eventid)
        {
            if (eventid)
            {
                if (std::find(klok.begin(), klok.end(), eventid) == klok.end()) klok.push_back(eventid);
                ::SetCoalescableTimer(hWnd, eventid, datetime::round<ui32>(elapse), nullptr, TIMERV_DEFAULT_COALESCING);
            }
        }
        void stop_timer(ui32 eventid)
        {
            auto iter = std::find(klok.begin(), klok.end(), eventid);
            if (iter != klok.end())
            {
                ::KillTimer(hWnd, eventid);
                klok.erase(iter);
            }
        }
    };

    struct manager : manager_base
    {
        using wins = std::vector<surface>;

        wins layers; // manager: ARGB layers.

        manager()
        {
            set_dpi_awareness();
        }
        ~manager()
        {
            for (auto& w : layers) w.reset();
        }

        void set_dpi_awareness()
        {
            auto proc = (LONG(_stdcall *)(si32))::GetProcAddress(::GetModuleHandleA("user32.dll"), "SetProcessDpiAwarenessInternal");
            if (proc)
            {
                auto hr = proc(2/*PROCESS_PER_MONITOR_DPI_AWARE*/);
                if (hr != S_OK || hr != E_ACCESSDENIED) log("%%Set DPI awareness failed %hr% %ec%", prompt::gui, utf::to_hex(hr), ::GetLastError());
            }
        }
        template<bool JustMove = faux>
        void present()
        {
            if constexpr (JustMove)
            {
                auto lock = ::BeginDeferWindowPos((si32)layers.size());
                for (auto& w : layers) if (w.prev.coor(w.live ? w.area.coor : w.hidden))
                {
                    lock = ::DeferWindowPos(lock, w.hWnd, 0, w.area.coor.x, w.area.coor.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
                    if (!lock) { log("%%DeferWindowPos returns unexpected result: %ec%", prompt::gui, ::GetLastError()); }
                }
                ::EndDeferWindowPos(lock);
            }
            else for (auto& w : layers) w.present();
        }
        auto get_fs_area(rect window_area)
        {
            auto enum_proc = [](HMONITOR /*unnamedParam1*/, HDC /*unnamedParam2*/, LPRECT monitor_rect_ptr, LPARAM pair_ptr)
            {
                auto& r = *monitor_rect_ptr;
                auto& [fs_area, wn_area] = *(std::pair<rect, rect>*)pair_ptr;
                auto hw_rect = rect{{ r.left, r.top }, { r.right - r.left, r.bottom - r.top }};
                if (wn_area.trim(hw_rect)) fs_area |= hw_rect;
                return TRUE;
            };
            auto area_pair = std::pair<rect, rect>{{}, window_area };
            ::EnumDisplayMonitors(NULL, nullptr, enum_proc, (LPARAM)&area_pair);
            return area_pair.first;
        }
        void dispatch()
        {
            auto msg = MSG{};
            while (::GetMessageW(&msg, 0, 0, 0) > 0)
            {
                ::DispatchMessageW(&msg);
            }
        }
        //void activate()
        //{
        //    if (!layers.empty()) ::SetActiveWindow(layers.front().hWnd);
        //    log("activated");
        //}
        //void shown_event(bool shown, arch reason)
        //{
        //    log(shown ? "shown" : "hidden", " ", reason == SW_OTHERUNZOOM   ? "The window is being uncovered because a maximize window was restored or minimized."s
        //                                       : reason == SW_OTHERZOOM     ? "The window is being covered by another window that has been maximized."s
        //                                       : reason == SW_PARENTCLOSING ? "The window's owner window is being minimized."s
        //                                       : reason == SW_PARENTOPENING ? "The window's owner window is being restored."s
        //                                                                    : utf::concat("Unknown reason. (", reason, ")"));
        //    activate();
        //}
        void mouse_capture()
        {
            if (!layers.empty()) ::SetCapture(layers.front().hWnd);
        }
        void mouse_release()
        {
            ::ReleaseCapture();
        }
        void close()
        {
            if (!layers.empty()) ::SendMessageW(layers.front().hWnd, WM_CLOSE, NULL, NULL);
        }
        auto client_animation()
        {
            auto a = TRUE;
            ::SystemParametersInfoA(SPI_GETCLIENTAREAANIMATION, 0, &a, 0);
            return a;
        }
        void sync_taskbar(si32 new_state)
        {
            if (layers.empty()) return;
            if (new_state == state::minimized) // In order to be in sync with winNT taskbar. Other ways don't work because explorer.exe tracks our window state on their side.
            {
                ::ShowWindow(layers.front().hWnd, SW_MINIMIZE);
            }
            else if (new_state == state::maximized) // "ShowWindow(SW_MAXIMIZE)" makes the window transparent to the mouse when maximized to multiple monitors.
            {
                //todo It doesn't work that way. Sync with system ctx menu.
                //auto ctxmenu = ::GetSystemMenu(layers.front().hWnd, FALSE);
                //::EnableMenuItem(ctxmenu, SC_RESTORE, MF_CHANGE | MF_ENABLED);
                //::EnableMenuItem(ctxmenu, SC_MAXIMIZE, MF_CHANGE | MF_GRAYED);
            }
            else ::ShowWindow(layers.front().hWnd, SW_RESTORE);
        }
        void run()
        {
            // Customize system ctx menu.
            auto closecmd = wide(100, '\0');
            auto ctxmenu = ::GetSystemMenu(layers.front().hWnd, FALSE);
            auto datalen = ::GetMenuStringW(ctxmenu, SC_CLOSE, closecmd.data(), (si32)closecmd.size(), MF_BYCOMMAND);
            closecmd.resize(datalen);
            auto temp = utf::to_utf(closecmd);
            utf::replace_all(temp, "Alt+F4", "Esc");
            closecmd = utf::to_utf(temp);
            ::ModifyMenuW(ctxmenu, SC_CLOSE, MF_BYCOMMAND | MF_ENABLED | MF_STRING, SC_CLOSE, closecmd.data());
            //todo implement
            ::RemoveMenu(ctxmenu, SC_MOVE, MF_BYCOMMAND);
            ::RemoveMenu(ctxmenu, SC_SIZE, MF_BYCOMMAND);
            // The first ShowWindow() call ignores SW_SHOW.
            auto mode = SW_SHOW;
            for (auto& w : layers) ::ShowWindow(w.hWnd, std::exchange(mode, SW_SHOWNA));
        }

        virtual void update() = 0;
        virtual void mouse_leave() = 0;
        virtual void mouse_moved(twod coord) = 0;
        virtual void focus_event(bool state) = 0;
        virtual void timer_event(arch eventid) = 0;
        //virtual void state_event(bool activated, bool minimized) = 0;
        virtual void sys_command(si32 menucmd) = 0;
        virtual void mouse_press(si32 index, bool pressed) = 0;
        virtual void mouse_wheel(si32 delta, si32 cntrl, bool hzwheel) = 0;
        virtual void keybd_press(arch vkey, arch lParam) = 0;
        virtual void check_fsmode(arch hWnd) = 0;

        auto add(manager* host_ptr = nullptr)
        {
            auto window_proc = [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
            {
                //log("\tmsW=", utf::to_hex(msg), " wP=", utf::to_hex(wParam), " lP=", utf::to_hex(lParam), " hwnd=", utf::to_hex(hWnd));
                auto w = (manager*)::GetWindowLongPtrW(hWnd, GWLP_USERDATA);
                if (!w) return ::DefWindowProcW(hWnd, msg, wParam, lParam);
                auto stat = LRESULT{};
                static auto hi = [](auto n){ return (si32)(si16)((n >> 16) & 0xffff); };
                static auto lo = [](auto n){ return (si32)(si16)((n >> 0 ) & 0xffff); };
                static auto hover_win = testy<HWND>{};
                static auto hover_rec = TRACKMOUSEEVENT{ .cbSize = sizeof(TRACKMOUSEEVENT), .dwFlags = TME_LEAVE, .dwHoverTime = HOVER_DEFAULT };
                switch (msg)
                {
                    case WM_MOUSEMOVE: if (hover_win(hWnd)) ::TrackMouseEvent((hover_rec.hwndTrack = hWnd, &hover_rec));
                                       if (auto r = RECT{}; ::GetWindowRect(hWnd, &r)) w->mouse_moved({ r.left + lo(lParam), r.top + hi(lParam) });
                                       break;
                    case WM_TIMER:         w->timer_event(wParam);                     break;
                    case WM_MOUSELEAVE:    w->mouse_leave(); hover_win = {};           break;
                    case WM_LBUTTONDOWN:   w->mouse_press(bttn::left,   true);         break;
                    case WM_MBUTTONDOWN:   w->mouse_press(bttn::middle, true);         break;
                    case WM_RBUTTONDOWN:   w->mouse_press(bttn::right,  true);         break;
                    case WM_LBUTTONUP:     w->mouse_press(bttn::left,   faux);         break;
                    case WM_MBUTTONUP:     w->mouse_press(bttn::middle, faux);         break;
                    case WM_RBUTTONUP:     w->mouse_press(bttn::right,  faux);         break;
                    case WM_MOUSEWHEEL:    w->mouse_wheel(hi(wParam), lo(wParam), 0);  break;
                    case WM_MOUSEHWHEEL:   w->mouse_wheel(hi(wParam), lo(wParam), 1);  break;
                    case WM_SETFOCUS:      w->focus_event(true);                       break;
                    case WM_KILLFOCUS:     w->focus_event(faux);                       break;
                    // These should be processed on the system side.
                    //case WM_SHOWWINDOW:    w->shown_event(!!wParam, lParam);           break; //todo revise
                    //case WM_MOUSEACTIVATE: w->activate(); stat = MA_NOACTIVATE;        break; // Suppress window activation with a mouse click.
                    //case WM_NCHITTEST:
                    //case WM_ACTIVATE: // Window focus within the app.
                    //case WM_ACTIVATEAPP:
                    //case WM_NCACTIVATE:
                    //case WM_SETCURSOR:
                    //case WM_GETMINMAXINFO:
                    case WM_SYSCOMMAND: switch (wParam & 0xFFF0)
                                        {
                                            case SC_MINIMIZE:     w->sys_command(syscmd::minimize);     break;
                                            case SC_MAXIMIZE:     w->sys_command(syscmd::maximize);     break;
                                            case SC_RESTORE:      w->sys_command(syscmd::restore);      break;
                                            case SC_CLOSE:        w->sys_command(syscmd::close);        break;
                                            default: stat = TRUE; // An application should return zero only if it processes this message.
                                            //todo implement
                                            //case SC_MOVE:         w->sys_command(syscmd::move);         break;
                                            //case SC_MONITORPOWER: w->sys_command(syscmd::monitorpower); break;
                                        }
                                        break; // Taskbar ctx menu to change the size and position.
                    //case WM_INITMENU: // The application can perform its own checking or graying by responding to the WM_INITMENU message that is sent before any menu is displayed.
                    case WM_KEYDOWN:
                    case WM_KEYUP:
                    case WM_SYSKEYDOWN:  // WM_CHAR/WM_SYSCHAR and WM_DEADCHAR/WM_SYSDEADCHAR are derived messages after translation.
                    case WM_SYSKEYUP:      w->keybd_press(wParam, lParam);             break;
                    case WM_WINDOWPOSCHANGED:
                    case WM_DPICHANGED:
                    case WM_DISPLAYCHANGE:
                    case WM_DEVICECHANGE:  w->check_fsmode((arch)hWnd);                break;
                    case WM_DESTROY:       ::PostQuitMessage(0);                       break;
                    //dx3d specific
                    //case WM_PAINT:   /*w->check_dx3d_state();*/ stat = ::DefWindowProcW(hWnd, msg, wParam, lParam); break;
                    default:
                    //log("\tmsW=", utf::to_hex(msg), " wP=", utf::to_hex(wParam), " lP=", utf::to_hex(lParam), " hwnd=", utf::to_hex(hWnd));
                    stat = ::DefWindowProcW(hWnd, msg, wParam, lParam); break;
                }
                w->update();
                return stat;
            };
            static auto wc_defwin = WNDCLASSW{ .lpfnWndProc = ::DefWindowProcW, .lpszClassName = L"vtm_decor" };
            static auto wc_window = WNDCLASSW{ .lpfnWndProc = window_proc, /*.cbWndExtra = 2 * sizeof(LONG_PTR),*/ .hCursor = ::LoadCursorW(NULL, (LPCWSTR)IDC_ARROW), .lpszClassName = L"vtm" };
            static auto reg = ::RegisterClassW(&wc_defwin) && ::RegisterClassW(&wc_window);
            if (!reg)
            {
                isfine = faux;
                log("%%window class registration error: %ec%", prompt::gui, ::GetLastError());
            }
            auto& wc = host_ptr ? wc_window : wc_defwin;
            auto owner = layers.empty() ? HWND{} : layers.front().hWnd;
            auto hWnd = ::CreateWindowExW(WS_EX_NOREDIRECTIONBITMAP | WS_EX_LAYERED | (wc.hCursor ? 0 : WS_EX_TRANSPARENT),
                                          wc.lpszClassName, owner ? nullptr : wc.lpszClassName, // Title.
                                          /*WS_VISIBLE: it is invisible to suppress messages until initialized | */
                                          WS_POPUP | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU, 0, 0, 0, 0, owner, 0, 0, 0);
            auto layer = (si32)layers.size();
            if (!hWnd)
            {
                isfine = faux;
                log("%%Window creation error: %ec%", prompt::gui, ::GetLastError());
            }
            else if (host_ptr)
            {
                ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)host_ptr);
            }
            layers.emplace_back(hWnd);
            return layer;
        }
    };
}

#else

namespace netxs::gui
{
    struct font
    {
        twod cellsize;
        std::list<text> families;
        font(std::list<text>& /*family_names*/, si32 /*cell_height*/)
        { }
        void set_fonts(std::list<text>&, bool)
        {
            //...
        }
        void set_cellsz(si32 /*height*/)
        {
            //...
        }
    };
    struct glyf
    {
        si32 aamode{};
        glyf(font& /*fcache*/ , bool /*aamode*/)
        { }
        void reset()
        {
            //...
        }
        void fill_grid(auto& /*canvas*/, auto& /*blinks*/, si32& /*blink_count*/, twod /*origin*/, auto& /*grid_cells*/)
        {
            //...
        }
    };
    struct surface : surface_base
    {
        arch hWnd{};
        auto canvas(bool wipe = faux)
        {
            if (wipe)
            {
                //...
            }
            //...
            return data;
        }
        void start_timer(span /*elapse*/, ui32 /*eventid*/)
        {
            //...
        }
        void stop_timer(ui32 /*eventid*/)
        {
            //...
        }
    };
    struct manager : manager_base
    {
        using wins = std::vector<surface>;

        wins layers; // manager: ARGB layers.

        auto add(auto ...)
        {
            //...
            return 0;
        }
        void run()
        {
            //...
        }
        bool client_animation()
        {
            //...
            return true;
        }
        void sync_taskbar(si32 /*new_state*/)
        {
            //...
        }
        rect get_fs_area(rect area)
        {
            //...
            return area;
        }
        template<bool JustMove = faux>
        void present()
        {
            //...
        }
        void close()
        {
            //...
        }
        void mouse_capture()
        {
            //...
        }
        void mouse_release()
        {
            //...
        }
        void dispatch()
        {
            //...
        }
    };
}

#endif

namespace netxs::gui
{
    struct window : manager
    {
        using gray = netxs::raster<std::vector<byte>, rect>;
        using shad = netxs::misc::shadow<gray>;
        using grip = netxs::misc::szgrips;

        struct task
        {
            static constexpr auto _counter = 1 + __COUNTER__;
            static constexpr auto blink  = 1 << (__COUNTER__ - _counter);
            static constexpr auto moved  = 1 << (__COUNTER__ - _counter);
            static constexpr auto sized  = 1 << (__COUNTER__ - _counter);
            static constexpr auto grips  = 1 << (__COUNTER__ - _counter);
            static constexpr auto hover  = 1 << (__COUNTER__ - _counter);
            static constexpr auto inner  = 1 << (__COUNTER__ - _counter);
            static constexpr auto header = 1 << (__COUNTER__ - _counter);
            static constexpr auto footer = 1 << (__COUNTER__ - _counter);
            static constexpr auto all = -1;
        };

        std::vector<byte> blink_synch;
        si32              blink_count{};
        ui::face main_grid;
        ui::face head_grid;
        ui::face foot_grid;

        font fcache; // window: Font cache.
        glyf gcache; // window: Glyph cache.
        fp32 height; // window: Cell height in fp32 pixels.
        twod gripsz; // window: Resizing grips size in pixels.
        dent border; // window: Border around window for resizing grips (dent in pixels).
        shad shadow; // window: Shadow generator.
        grip szgrip; // window: Resizing grips UI-control.
        twod mcoord; // window: Mouse cursor coord.
        si32 mbttns; // window: Mouse button state.
        bool mhover; // window: Mouse hover.
        bool active; // window: Window is focused.
        si32 fsmode; // window: Window size state.
        rect normsz; // window: Non-fullscreen window area backup.
        si32 reload; // window: Changelog for update.
        si32 client; // window: Surface index for Client.
        si32 blinky; // window: Surface index for blinking characters.
        si32 header; // window: Surface index for Header.
        si32 footer; // window: Surface index for Footer.
        rect grip_l; // window: .
        rect grip_r; // window: .
        rect grip_t; // window: .
        rect grip_b; // window: .
        bool drop_shadow{ true }; // window: .
        twod& cellsz{ fcache.cellsize }; // window: Cell size in pixels.
        span blinkrate; // window: .
        bool blinking; // window: .

        //test
        fp2d scroll_pos;
        twod scroll_origin;
        twod scroll_delta;
        text font_list_str;
        text testtext;
        bool panoramic_scroll{};

        static constexpr auto shadow_dent = dent{ 1,1,1,1 } * 3;

        window(rect win_coor_px_size_cell, std::list<text>& font_names, si32 cell_height, si32 win_state, bool antialiasing, span blinkrate, text testtext = {},  twod grip_cell = dot_21)
            : fcache{ font_names, cell_height },
              gcache{ fcache, antialiasing },
              height{ (fp32)fcache.cellsize.y },
              gripsz{ grip_cell * fcache.cellsize },
              border{ gripsz.x, gripsz.x, gripsz.y, gripsz.y },
              shadow{ 0.44f/*bias*/, 116.5f/*alfa*/, gripsz.x, dot_00, dot_11, cell::shaders::full },
              mbttns{},
              mhover{},
              active{},
              fsmode{ state::undefined },
              reload{ task::all },
              client{ manager::add(this) },
              blinky{ manager::add() },
              header{ manager::add() },
              footer{ manager::add() },
              blinkrate{ manager::client_animation() ? blinkrate : span::zero() },
              blinking{ faux }
        {
            if (!*this) return;
            normsz = rect{ win_coor_px_size_cell.coor, std::max(dot_11, win_coor_px_size_cell.size) * cellsz } + border;
            layers[client].area = normsz;
            size_window();
            set_state(win_state);

            //test
            this->testtext = testtext;
            print_font_list();
            refillgrid();

            update();
            manager::run();
        }
        void sync_titles_pixel_layout()
        {
            auto base_rect = layers[client].area;
            grip_l = rect{{ 0                          , gripsz.y }, { gripsz.x, base_rect.size.y - gripsz.y * 2}};
            grip_r = rect{{ base_rect.size.x - gripsz.x, gripsz.y }, grip_l.size };
            grip_t = rect{{ 0, 0                                  }, { base_rect.size.x, gripsz.y }};
            grip_b = rect{{ 0, base_rect.size.y - gripsz.y        }, grip_t.size };
            base_rect -= border;
            auto header_height = head_grid.size().y * cellsz.y;
            auto footer_height = foot_grid.size().y * cellsz.y;
            layers[header].area = base_rect + dent{ 0, 0, header_height, -base_rect.size.y } + shadow_dent;
            layers[footer].area = base_rect + dent{ 0, 0, -base_rect.size.y, footer_height } + shadow_dent;
            layers[header].area.coor.y -= shadow_dent.b;
            layers[footer].area.coor.y += shadow_dent.t;
        }
        void reset_blinky()
        {
            if (active && layers[blinky].live) // Hide blinking layer to avoid visual desync.
            {
                layers[blinky].hide();
                manager::present<true>();
                layers[blinky].show();
            }
        }
        void change_cell_size(fp32 dy = {})
        {
            reset_blinky();
            gcache.reset();
            auto grip_cell = gripsz / cellsz;
            height += dy;
            auto prev_cellsz = cellsz;
            fcache.set_cellsz((si32)height);
            gripsz = grip_cell * cellsz;
            shadow.generate(0.44f/*bias*/, 116.5f/*alfa*/, gripsz.x, dot_00, dot_11, cell::shaders::full);
            if (fsmode == state::maximized)
            {
                normsz.size = normsz.size / prev_cellsz * cellsz;
                auto over_sz = layers[client].area.size % cellsz;
                auto half_sz = over_sz / 2;
                border = { half_sz.x, over_sz.x - half_sz.x, half_sz.y, over_sz.y - half_sz.y };
                size_window();
            }
            else
            {
                border = { gripsz.x, gripsz.x, gripsz.y, gripsz.y };
                auto new_size = main_grid.size() * cellsz;
                layers[client].area.size = new_size + border;
                layers[blinky].area = layers[client].area - border;
                sync_titles_pixel_layout();
            }
        }
        void set_aa_mode(bool mode)
        {
            log("%%AA mode %state%", prompt::gui, mode ? "enabled" : "disabled");
            gcache.aamode = mode;
            gcache.reset();
            reload |= task::all;
        }

        //test
        void print_font_list(bool refill = faux)
        {
            auto i = 0;
            font_list_str = utf::concat("Test text: \033[10m", testtext, "\033[m\n\n Antialiasing ", gcache.aamode ? "On" : "Off",
                                      "\n Cell Size ", cellsz.x, "x", cellsz.y,
                                      "\n Font Fallback\n");
            for (auto& f : fcache.families)
            {
                font_list_str += utf::concat(ansi::bld(i == 0), utf::adjust(std::to_string(i), 4, ' ', true), ": ", f, '\n');
                i++;
            }
            canvas_page = intro + font_list_str + canvas_text;
            if (refill) refillgrid();
        }
        void refillgrid()
        {
            auto area = layers[blinky].area;
            main_grid.wipe();
            main_grid.zz(scroll_pos);
            main_grid.vsize(std::min(0, (si32)-scroll_pos.y) + area.size.y);
            main_grid.output<true>(canvas_page);
            if (fsmode != state::maximized)
            {
                head_grid.wipe();
                foot_grid.wipe();
                head_grid.cup(dot_00);
                head_grid.output(header_page);
                foot_grid.cup(dot_00);
                foot_grid.output(footer_page);
            }
        }

        void set_font_list(auto& flist)
        {
            log("%%Font list changed: ", prompt::gui, flist);
            fcache.set_fonts(flist, faux);
            change_cell_size();
            reload |= task::all;
        }
        auto moveby(twod delta)
        {
            for (auto& w : layers) w.area.coor += delta;
        }
        void set_state(si32 new_state)
        {
            if (fsmode == new_state) return;
            log("%%Set window to ", prompt::gui, new_state == state::maximized ? "maximized" : new_state == state::normal ? "normal" : "minimized", " state.");
            auto old_state = std::exchange(fsmode, state::undefined);
            if (new_state != state::minimized) reset_blinky(); // To avoid visual desync.
            manager::sync_taskbar(new_state);
            fsmode = new_state;
            if (old_state == state::normal) normsz = layers[client].area;
            if (fsmode == state::normal)
            {
                layers[client].area = normsz;
                border = { gripsz.x, gripsz.x, gripsz.y, gripsz.y };
                size_window();
                for (auto l : { client, header, footer }) layers[l].show();
                if (blink_count) layers[blinky].show();
            }
            else if (fsmode == state::minimized)
            {
                for (auto& l : layers) l.hide();
            }
            else if (fsmode == state::maximized)
            {
                layers[client].area = manager::get_fs_area(layers[client].area - border);
                auto over_sz = layers[client].area.size % cellsz;
                auto half_sz = over_sz / 2;
                border = { half_sz.x, over_sz.x - half_sz.x, half_sz.y, over_sz.y - half_sz.y };
                size_window();
                layers[header].hide();
                layers[footer].hide();
                layers[client].show();
                if (blink_count) layers[blinky].show();
            }
            reload |= task::all;

            //test
            refillgrid();
        }
        void check_fsmode(arch hWnd)
        {
            if ((arch)(layers[client].hWnd) != hWnd) return;
            if (fsmode == state::undefined || layers.empty()) return;
            else if (fsmode == state::maximized)
            {
                auto fs_area = manager::get_fs_area(layers[client].area);
                if (fs_area != layers[client].area)
                {
                    auto avail_area = manager::get_fs_area(rect{ -dot_mx / 2, dot_mx });
                    avail_area.size -= std::min(avail_area.size, normsz.size);
                    normsz.coor = avail_area.clamp(normsz.coor);
                    set_state(state::normal);
                    reload |= task::all;
                }
            }
            else if (fsmode == state::normal)
            {
                auto avail_area = manager::get_fs_area(rect{ -dot_mx / 2, dot_mx });
                if (!avail_area.trim(layers[client].area))
                {
                    auto area = layers[client].area;
                    avail_area.size -= std::min(avail_area.size, area.size);
                    auto delta = avail_area.clamp(area.coor) - area.coor;
                    moveby(delta);
                }
            }
            if (fsmode != state::minimized)
            {
                for (auto& w : layers) w.prev.coor = dot_mx; // Windows moves our windows the way it wants, breaking the layout.
                reload |= task::moved;
            }
        }
        auto move_window(twod coor_delta)
        {
            moveby(coor_delta);
            reload |= task::moved;
        }
        void size_window(twod size_delta = {})
        {
            layers[client].area.size += size_delta;
            layers[blinky].area = layers[client].area - border;
            auto gridsz = layers[blinky].area.size / cellsz;
            main_grid.size(gridsz);
            blink_synch.assign(main_grid.volume(), 0);
            blink_count = 0;
            if (fsmode != state::maximized)
            {
                //test
                auto c_size = gridsz;
                main_grid.calc_page_height(canvas_page, c_size);
                footer_page = ansi::wrp(wrap::on).jet(bias::right).fgc(tint::purewhite).add((si32)scroll_pos.x, ":", (si32)scroll_pos.y, "/", c_size.y, " ", gridsz.x, ":", gridsz.y);

                auto h_size = gridsz;
                auto f_size = gridsz;
                head_grid.calc_page_height(header_page, h_size);
                foot_grid.calc_page_height(footer_page, f_size);
                head_grid.size(h_size);
                foot_grid.size(f_size);
                sync_titles_pixel_layout();
            }
            reload |= task::sized;

            //test
            refillgrid();
        }
        auto resize_window(twod size_delta)
        {
            auto old_client = layers[blinky].area;
            auto new_gridsz = std::max(dot_11, (old_client.size + size_delta) / cellsz);
            size_delta = new_gridsz * cellsz - old_client.size;
            if (size_delta)
            {
                size_window(size_delta);
            }
            return size_delta;
        }
        auto warp_window(dent warp_delta)
        {
            auto old_client = layers[blinky].area;
            auto new_client = old_client + warp_delta;
            auto new_gridsz = std::max(dot_11, new_client.size / cellsz);
            if (main_grid.size() != new_gridsz)
            {
                auto size_delta = new_gridsz * cellsz - old_client.size;
                auto coor_delta = new_client.coor - old_client.coor;
                size_window(size_delta);
                move_window(coor_delta);
            }
            return layers[client].area - old_client;
        }
        void _test_fill_back(auto& grid_cells)
        {
            auto rtc = argb{ tint::pureblue  };//.alpha(0.5f);
            auto ltc = argb{ tint::pureblack };
            auto rbc = argb{ tint::purered };
            auto lbc = argb{ tint::puregreen  };//.alpha(0.5f);
            auto lc = ltc;
            auto rc = rtc;
            auto x = 0.f;
            auto y = 0.f;
            auto m = std::max(dot_11, grid_cells.size() - dot_11);
            auto eol = [&]
            {
                x = 0.f;
                auto dc = ++y / m.y;
                lc = argb::transit(ltc, lbc, dc);
                rc = argb::transit(rtc, rbc, dc);
            };
            auto fx = [&](cell& c)
            {
                auto dc = x++ / m.x;
                if (!c.bgc()) c.bgc(argb::transit(lc, rc, dc));
            };
            netxs::onrect(grid_cells, grid_cells.area(), fx, eol);
        }
        bool hit_grips()
        {
            if (fsmode == state::maximized || szgrip.zoomon || panoramic_scroll) return faux;
            auto inner_rect = layers[blinky].area;
            auto outer_rect = layers[client].area;
            auto hit = szgrip.seized || (mhover && outer_rect.hittest(mcoord) && !inner_rect.hittest(mcoord));
            return hit;
        }
        void draw_grips()
        {
            if (fsmode == state::maximized) return;
            static auto trans = 0x01'00'00'00;
            static auto shade = 0x5F'3f'3f'3f;
            static auto black = 0x3F'00'00'00;
            auto& layer = layers[client];
            auto canvas = layer.canvas();
            canvas.move(dot_00);
            auto outer_rect = canvas.area();
            auto inner_rect = outer_rect - border;
            auto fill_grips = [&](rect area, auto fx)
            {
                for (auto g_area : { grip_l, grip_r, grip_t, grip_b })
                {
                    if (auto r = g_area.trim(area)) fx(canvas, r);
                }
            };
            fill_grips(outer_rect, [](auto& canvas, auto r){ netxs::onrect(canvas, r, cell::shaders::full(trans)); });
            if (hit_grips())
            {
                auto s = szgrip.sector;
                auto [side_x, side_y] = szgrip.layout(outer_rect);
                auto dent_x = dent{ s.x < 0, s.x > 0, s.y > 0, s.y < 0 };
                auto dent_y = dent{ s.x > 0, s.x < 0, 1, 1 };
                fill_grips(side_x, [&](auto& canvas, auto r)
                {
                    netxs::onrect(canvas, r, cell::shaders::full(shade));
                    netxs::misc::cage(canvas, side_x, dent_x, cell::shaders::full(black)); // 1-px dark contour around.
                });
                fill_grips(side_y, [&](auto& canvas, auto r)
                {
                    netxs::onrect(canvas, r, cell::shaders::full(shade));
                    netxs::misc::cage(canvas, side_y, dent_y, cell::shaders::full(black)); // 1-px dark contour around.
                });
            }
            if (drop_shadow) fill_grips(outer_rect, [&](auto& canvas, auto r)
            {
                shadow.render(canvas, r, inner_rect, cell::shaders::alpha);
            });
            for (auto g_area : { grip_l, grip_r, grip_t, grip_b })
            {
                layer.sync.push_back(g_area);
            }
        }
        void draw_title(si32 index, auto& facedata) //todo just output ui::core
        {
            auto canvas = layers[index].canvas(true);
            auto blinks = layers[blinky].canvas(); //todo unify blinks in titles
            gcache.fill_grid(canvas, blinks, blink_count, shadow_dent.corner(), facedata);
            netxs::misc::contour(canvas); // 1ms
            layers[index].sync.push_back({ .size = canvas.size() });
        }
        void draw_header() { draw_title(header, head_grid); }
        void draw_footer() { draw_title(footer, foot_grid); }
        void check_blinky()
        {
            if (blinking != !!blink_count)
            {
                blinking = !!blink_count;
                if (blink_count)
                {
                    if (blinkrate != span::zero()) layers[client].start_timer(blinkrate, timers::blink);
                    else                           layers[blinky].show();
                }
                else
                {
                    if (active && layers[blinky].live) layers[blinky].hide();
                    layers[client].stop_timer(timers::blink);
                }
            }
        }
        void update()
        {
            if (!reload) return;
            auto what = reload;
            reload = {};
                 if (what == task::moved) manager::present<true>();
            else if (what)
            {
                if (what & (task::sized | task::inner))
                {
                    _test_fill_back(main_grid);
                    auto canvas = layers[client].canvas();
                    auto blinks = layers[blinky].canvas(true);
                    canvas.move(dot_00);
                    auto dirty_area = canvas.area() - border;
                    blink_count = 0;
                    gcache.fill_grid(canvas, blinks, blink_count, dirty_area.coor, main_grid); // 0.500 ms);
                    check_blinky();
                    if (fsmode == state::maximized && (what & task::sized))
                    {
                        netxs::misc::cage(canvas, canvas.area(), border, cell::shaders::full(argb{ tint::pureblack }));
                        dirty_area += border;
                    }
                    layers[client].strike(dirty_area);
                    layers[blinky].strike({ .size = layers[blinky].area.size });
                }
                if (fsmode == state::normal)
                {
                    if (what & (task::sized | task::hover | task::grips)) draw_grips(); // 0.150 ms
                    if (what & (task::sized | task::header)) draw_header();
                    if (what & (task::sized | task::footer)) draw_footer();
                }
                manager::present();
            }
        }
        auto& kbs()
        {
            static auto state_kb = 0;
            return state_kb;
        }
        auto keybd_state()
        {
            //todo unify
            auto state = 0;
            #if defined(_WIN32)
            state = hids::LShift   * !!::GetAsyncKeyState(VK_LSHIFT)
                       | hids::RShift   * !!::GetAsyncKeyState(VK_RSHIFT)
                       | hids::LWin     * !!::GetAsyncKeyState(VK_LWIN)
                       | hids::RWin     * !!::GetAsyncKeyState(VK_RWIN)
                       | hids::LAlt     * !!::GetAsyncKeyState(VK_LMENU)
                       | hids::RAlt     * !!::GetAsyncKeyState(VK_RMENU)
                       | hids::LCtrl    * !!::GetAsyncKeyState(VK_LCONTROL)
                       | hids::RCtrl    * !!::GetAsyncKeyState(VK_RCONTROL)
                       | hids::ScrlLock * !!::GetKeyState(VK_SCROLL)
                       | hids::NumLock  * !!::GetKeyState(VK_NUMLOCK)
                       | hids::CapsLock * !!::GetKeyState(VK_CAPITAL);
            #endif
            return state;
        }
        void mouse_wheel(si32 delta, si32 cntrl, bool hz)
        {
            auto wheeldt = delta / 120.f;
            auto kb = keybd_state();//kbs();
            if (cntrl || hz)
            {
                //...
            }
            #if defined(_WIN32)
            if (cntrl & MK_CONTROL)
            {
                change_cell_size(wheeldt);
                reload |= task::all;
                print_font_list(true);
                return;
            }
            //else if (kb & hids::LAlt)
            else if (cntrl & MK_SHIFT)
            {
                netxs::_k1 += wheeldt > 0 ? 1 : -1; // LCtrl+Wheel.
                log("_k0=", _k0, "_k1=", _k1);
                change_cell_size();
                reload |= task::all;
                print_font_list(true);
                return;
            }
            #endif
            //     if (kb & (hids::LCtrl | hids::LAlt)) netxs::_k2 += wheeldt > 0 ? 1 : -1; // LCtrl + Alt t +Wheel.
            //else if (kb & hids::LCtrl)                netxs::_k0 += wheeldt > 0 ? 1 : -1; // LCtrl+Wheel.
            //else if (kb & hids::anyAlt)               netxs::_k1 += wheeldt > 0 ? 1 : -1; // Alt+Wheel.
            //else if (kb & hids::RCtrl)                netxs::_k3 += wheeldt > 0 ? 1 : -1; // RCtrl+Wheel.
            //shadow = build_shadow_corner(cellsz.x);
            //reload |= task::sized;
            //netxs::_k0 += wheeldt > 0 ? 1 : -1;
            //log("wheel ", wheeldt, " k0= ", _k0, " k1= ", _k1, " k2= ", _k2, " k3= ", _k3, " keybd ", utf::to_bin(kb));

            //todo Activate it in another way.
            if ((kb & hids::anyCtrl) && !(kb & hids::ScrlLock))
            {
                if (!szgrip.zoomon)
                {
                    szgrip.zoomdt = {};
                    szgrip.zoomon = true;
                    szgrip.zoomsz = layers[client].area;
                    szgrip.zoomat = mcoord;
                    mouse_capture();
                }
            }
            else
            {
                if (szgrip.zoomon)
                {
                    szgrip.zoomon = faux;
                    mouse_release();
                }
                hz ? scroll_pos.x -= wheeldt
                   : scroll_pos.y += wheeldt;
                size_window();
                reload |= task::all;
                reload &= ~task::sized;
            }
            if (szgrip.zoomon)
            {
                auto warp = dent{ gripsz.x, gripsz.x, gripsz.y, gripsz.y }; // border. exclude state::maximized
                auto step = szgrip.zoomdt + warp * (si32)wheeldt;
                auto next = szgrip.zoomsz + step;
                next.size = std::max(dot_00, next.size);
                ///auto viewport = ...get max win size (multimon)
                //next.trimby(viewport);
                if (warp_window(next - layers[client].area)) szgrip.zoomdt = step;
            }
        }
        void mouse_leave()
        {
            mhover = faux;
            if (szgrip.leave()) reload |= task::grips;
        }
        void mouse_moved(twod coord)
        {
            mhover = true;
            auto kb = kbs();// keybd_state();
            auto inner_rect = layers[blinky].area;
            if (mbttns & bttn::right)
            {
                scroll_delta += coord - mcoord;
                if (scroll_pos(scroll_origin + scroll_delta / cellsz))
                {
                    size_window();
                    reload |= task::all;
                    reload &= ~task::sized;
                }
            }
            else if (hit_grips() || szgrip.seized)
            {
                if (mbttns & bttn::left)
                {
                    if (!szgrip.seized) // drag start
                    {
                        szgrip.grab(inner_rect, mcoord, border, cellsz);
                    }
                    auto zoom = kb & hids::anyCtrl;
                    auto [preview_area, size_delta] = szgrip.drag(inner_rect, coord, border, zoom, cellsz);
                    if (auto dxdy = resize_window(size_delta))
                    {
                        if (auto move_delta = szgrip.move(dxdy, zoom))
                        {
                            move_window(move_delta);
                        }
                    }
                }
                else if (szgrip.seized) // drag stop
                {
                    szgrip.drop();
                    reload |= task::grips;
                }
            }
            if (szgrip.zoomon && !(kb & hids::anyCtrl))
            {
                szgrip.zoomon = faux;
                mouse_release();
            }
            if (szgrip.calc(inner_rect, coord, border, dent{}, cellsz))
            {
                reload |= task::grips;
            }
            if (!szgrip.seized && (mbttns & bttn::left))
            {
                if (auto dxdy = coord - mcoord)
                {
                    if (fsmode == state::maximized) set_state(state::normal);
                    moveby(dxdy);
                    reload |= task::moved;
                }
            }
            mcoord = coord;
            if (!mbttns)
            {
                static auto s = testy{ faux };
                reload |= s(hit_grips()) ? task::grips// | task::inner
                                     : s ? task::grips : 0;//task::inner;
            }
        }
        void mouse_press(si32 button, bool pressed)
        {
            if (pressed) { if (0 == std::exchange(mbttns, mbttns | button)) mouse_capture(); }
            else           if (0 == (mbttns &= ~button)) mouse_release();

            //test
            //if (!pressed && (button == bttn::right)) manager::close();
            static auto dblclick = datetime::now() - 1s;
            if (pressed)
            {
                if (button == bttn::right)
                {
                    panoramic_scroll = true;
                    scroll_origin = scroll_pos;
                    scroll_delta = {};
                }
            }
            else
            {
                if (button == bttn::left)
                {
                    if (datetime::now() - dblclick < 500ms)
                    {
                        if (fsmode != state::minimized) set_state(fsmode == state::maximized ? state::normal : state::maximized);
                        dblclick -= 1s;
                    }
                    else
                    {
                        dblclick = datetime::now();
                        if (szgrip.seized) // drag stop
                        {
                            szgrip.drop();
                            reload |= task::grips;
                        }
                    }
                }
                else if (button == bttn::right)
                {
                    panoramic_scroll = faux;
                }
            }
        }
        void keybd_press(arch vkey, arch lParam)
        {
            if (vkey || lParam)
            {
                //...
            }
            #if defined(_WIN32)

            union key_state
            {
                ui32 token;
                struct
                {
                    ui32 repeat   : 16;// 0-15
                    ui32 scancode : 9; // 16-24 (24 - extended)
                    ui32 reserved : 5; // 25-29 (29 - context)
                    ui32 state    : 2; // 30-31: 0 - pressed, 1 - repeated, 2 - unknown, 3 - released
                } v;
            };
            auto param = key_state{ .token = (ui32)lParam };
            //log("vkey: ", utf::to_hex(vkey),
            //    " scode: ", utf::to_hex(param.v.scancode),
            //    " state: ", param.v.state == 0 ? "pressed"
            //              : param.v.state == 1 ? "rep"
            //              : param.v.state == 3 ? "released" : "unknown");
            kbs() = keybd_state();
            if (vkey == 0x1b) manager::close();
            else if (vkey == 'A' && param.v.state == 3) // Toggle aa mode.
            {
                set_aa_mode(!gcache.aamode);
                print_font_list(true);
            }
            else if (vkey == VK_F11 && param.v.state == 3) // Toggle maximized mode.
            {
                if (fsmode != state::minimized) set_state(fsmode == state::maximized ? state::normal : state::maximized);
            }
            else if (param.v.state == 3 && fcache.families.size()) // Renumerate font list.
            {
                auto flen = fcache.families.size();
                auto index = vkey == 0x30 ? fcache.families.size() - 1 : vkey - 0x30;
                if (index > 0 && index < flen)
                {
                    auto& flist = fcache.families;
                    auto iter = flist.begin();
                    std::advance(iter, index);
                    flist.splice(flist.begin(), flist, iter, std::next(iter)); // Move it to the begining of the list.
                    set_font_list(flist);
                    print_font_list(true);
                }
            }
            #endif
            //auto s = keybd_state();
            //log("keybd ", utf::to_bin(s));
            //static auto keybd_state = std::array<byte, 256>{};
            //::GetKeyboardState(keybd_state.data());
            //auto l_shift = keybd_state[VK_LSHIFT];
            //auto r_shift = keybd_state[VK_RSHIFT];
            //auto l_win   = keybd_state[VK_LWIN];
            //auto r_win   = keybd_state[VK_RWIN];
            //bool alt     = keybd_state[VK_MENU];
            //bool l_alt   = keybd_state[VK_LMENU];
            //bool r_alt   = keybd_state[VK_RMENU];
            //bool l_ctrl  = keybd_state[VK_LCONTROL];
            //bool r_ctrl  = keybd_state[VK_RCONTROL];
            //log("keybd",
            //    "\n\t l_shift ", utf::to_hex(l_shift ),
            //    "\n\t r_shift ", utf::to_hex(r_shift ),
            //    "\n\t l_win   ", utf::to_hex(l_win   ),
            //    "\n\t r_win   ", utf::to_hex(r_win   ),
            //    "\n\t alt     ", utf::to_hex(alt     ),
            //    "\n\t l_alt   ", utf::to_hex(l_alt   ),
            //    "\n\t r_alt   ", utf::to_hex(r_alt   ),
            //    "\n\t l_ctrl  ", utf::to_hex(l_ctrl  ),
            //    "\n\t r_ctrl  ", utf::to_hex(r_ctrl  ));
        }
        void focus_event(bool focused)
        {
            log(focused ? "focused" : "unfocused");
            active = focused;
        }
        //void state_event(bool activated, bool minimized)
        //{
        //    //todo revise
        //    log(activated ? "activated" : "deactivated", " ", minimized ? "minimized" : "restored");
        //    //if (!activated) set_state(state::minimized);
        //    if (!activated && minimized) set_state(state::minimized);
        //    else                         set_state(state::normal);
        //}
        void timer_event(arch eventid)
        {
            if (fsmode == state::minimized || eventid != timers::blink) return;
            auto visible = layers[blinky].live;
            if (active && visible)
            {
                layers[blinky].hide();
                reload |= task::blink;
            }
            else if (!visible) // Do not blink without focus.
            {
                layers[blinky].show();
                reload |= task::blink;
            }
        }
        void sys_command(si32 menucmd)
        {
            //log("sys_command: menucmd=", utf::to_hex_0x(menucmd));
            switch (menucmd)
            {
                case syscmd::maximize: set_state(fsmode == state::maximized ? state::normal : state::maximized); break;
                case syscmd::minimize: set_state(state::minimized); break;
                case syscmd::restore:  set_state(state::normal);    break;
                //todo implement
                //case syscmd::move:          break;
                //case syscmd::monitorpower:  break;
                case syscmd::close: manager::close(); break;
            }
        }
        auto send(auto ...)
        {
            //...
            return faux;
        }
        auto recv()
        {
            //...
            return ""s;
        }
        auto shut()
        {
            //...
        }
    };
}