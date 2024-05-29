// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#undef GetGlyphIndices
#include <DWrite_2.h>
#include <memory_resource>
#pragma comment(lib, "Gdi32")
#pragma comment(lib, "dwrite")

#define ok2(...) [&](){ auto hr = __VA_ARGS__; if (hr != S_OK) log(utf::to_hex(hr), " ", #__VA_ARGS__); return hr == S_OK; }()

namespace netxs::gui
{
    using namespace input;

    //test strings
    template<auto ...Args>
    auto vss = utf::matrix::vss<Args...>;
    auto canvas_text = ansi::add("")
        .wrp(wrap::on).fgc(tint::purecyan)
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
        .add("\n")
        .add("\2aaa", vss<21>, "<VS22_00  >👩‍👩‍👧‍👧", vss<31>, "<VS31_00  >👩‍👩‍👧‍👧", vss<41>, "<VS41_00\n")
        .add("❤", vss<11>, "<VS11_00 ", "👩‍👩‍👧‍👧", vss<21>, "<VS21_00  >👩‍👩‍👧‍👧", vss<31>, "<VS31_00  >👩‍👩‍👧‍👧", vss<41>, "<VS41_00\n")
        //todo multiline graphemes
        //.add("\2line1\nline2", vss<52,01>, "\n")
        //.add("\2line1\nline2", vss<52,02>, "\n")
        .bld(faux).itc(true).add("vtm GUI frontend").itc(faux).fgc(tint::purered).bld(true).add(" is currently under development.").nil()
        .fgc(tint::cyanlt).add(" You can try it on any versions/editions of Windows platforms starting from Windows 8.1"
                               " (with colored emoji!), including Windows Server Core. 🥵🥵", vss<11>, "🦚😀⛷🏂😁😂😃😄😅😆👌🐞😎👪.\n")
        .fgc(tint::greenlt).add("Press Esc to close.\n")
        .add("\n")
        .fgc(tint::purecyan).bld(faux).add("Devanagari script:\n")
        .add("\2अनुच्छेद", vss<51>, " १.\n"     // अनुच्छेद १.
             "\2सभी", vss<31>, " \2मनुष्यों", vss<41>, " को", vss<21>, " \2गौरव", vss<31>, " \2और", vss<31>, " \2अधिकारों", vss<61>, " के", vss<21>, " \2मामले", vss<41>, " में "  // सभी मनुष्यों को गौरव और अधिकारों के मामले में
             "\2जन्मजात", vss<51>, " \2स्वतन्त्रता", vss<51>, " \2और", vss<31>, " \2समानता", vss<51>, " \2प्राप्त", vss<31>, " \2है।", vss<21>, "\n" // जन्मजात स्वतन्त्रता और समानता प्राप्त है।
             "\2उन्हें", vss<21>, " \2बुद्धि", vss<31>, " \2और", vss<31>, " \2अन्तरात्मा", vss<61>, " की", vss<21>, " \2देन", vss<21>, " \2प्राप्त", vss<31>, " है \2और", vss<31>, " " // उन्हें बुद्धि और अन्तरात्मा की देन प्राप्त है और
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
        .fgc(blacklt).bgc(whitedk).add("\2Height", utf::vs10, vss<24,11>).fgc(whitelt).bgc(blackdk).add("\2Height", utf::vs10, vss<24,21>).bgc(argb{}).add("😎", vss<84,01>).fgc(purecyan).bgc(argb{}).add("\2Height", utf::vs12, vss<24,01>).fgc(purecyan).add(" <VS84_00\n")
        .fgc(whitelt).bgc(blackdk).add("\2Height", utf::vs10, vss<24,12>).fgc(blacklt).bgc(whitedk).add("\2Height", utf::vs10, vss<24,22>).bgc(argb{}).add("😎", vss<84,02>).fgc(purecyan).bgc(argb{}).add("\2Height", utf::vs12, vss<24,02>).add("\n")
        .fgc(blacklt).bgc(whitedk).add("\2Height", utf::vs10, vss<24,13>).fgc(whitelt).bgc(blackdk).add("\2Height", utf::vs10, vss<24,23>).bgc(argb{}).add("😎", vss<84,03>).fgc(purecyan).bgc(argb{}).add("\2Height", utf::vs12, vss<24,03>).add("\n")
        .fgc(whitelt).bgc(blackdk).add("\2Height", utf::vs10, vss<24,14>).fgc(blacklt).bgc(whitedk).add("\2Height", utf::vs10, vss<24,24>).bgc(argb{}).add("😎", vss<84,04>).fgc(purecyan).bgc(argb{}).add("\2Height", utf::vs12, vss<24,04>).add("\n")
        .add("  ").fgc(blacklt).bgc(whitedk).add("\2Width", utf::vs11, vss<81,11>).fgc(whitelt).bgc(blackdk).add("\2Width", utf::vs11, vss<81,21>).fgc(blacklt).bgc(whitedk).add("\2Width", utf::vs11, vss<81,31>).fgc(whitelt).bgc(blackdk).add("\2Width", utf::vs11, vss<81,41>)
                  .fgc(blacklt).bgc(whitedk).add("\2Width", utf::vs11, vss<81,51>).fgc(whitelt).bgc(blackdk).add("\2Width", utf::vs11, vss<81,61>).fgc(blacklt).bgc(whitedk).add("\2Width", utf::vs11, vss<81,71>).fgc(whitelt).bgc(blackdk).add("\2Width", utf::vs11, vss<81,81>)
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
    auto canvas_page = ui::page{ canvas_text + canvas_text + canvas_text + canvas_text + canvas_text};
    auto header_page = ui::page{ header_text };
    auto footer_page = ui::page{ footer_text };

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
            std::vector<IDWriteFontFace2*>    fontface;
            DWRITE_FONT_METRICS               metrics{};
            fp32                              base_baseline{};
            fp2d                              face_baseline{};
            fp32                              transform{};
            si32                              base_emheight{};
            fp32                              face_emheight{};
            twod                              facesize; // Typeface cell size.
            ui32                              index{ ~0u };
            bool                              color{ faux };
            bool                              fixed{ faux }; // Preserve specified font order.

            static auto iscolor(auto faceinst)
            {
                auto tableSize = ui32{};
                auto tableData = (void const*)nullptr;
                auto tableContext = (void*)nullptr;
                auto exists = BOOL{};
                faceinst->TryGetFontTable(DWRITE_MAKE_OPENTYPE_TAG('C', 'O', 'L', 'R'), //_In_ UINT32 openTypeTableTag,
                                          &tableData,    //_Outptr_result_bytebuffer_(*tableSize) const void** tableData,
                                          &tableSize,    //_Out_ UINT32* tableSize,
                                          &tableContext, //_Out_ void** tableContext,
                                          &exists);      //_Out_ BOOL* exists
                if (exists) faceinst->ReleaseFontTable(tableContext);
                return exists;
            }
            auto load(auto& faceinst, auto barefont, auto weight, auto stretch, auto style)
            {
                auto fontfile = (IDWriteFont2*)nullptr;
                barefont->GetFirstMatchingFont(weight, stretch, style, (IDWriteFont**)&fontfile);
                if (!fontfile) return;
                fontfile->CreateFontFace((IDWriteFontFace**)&faceinst);
                if (faceinst && !metrics.designUnitsPerEm)
                {
                    faceinst->GetMetrics(&metrics);
                    base_emheight = metrics.designUnitsPerEm;
                    auto glyph_metrics = DWRITE_GLYPH_METRICS{};
                    auto glyph_index = ui16{ 0 };
                    faceinst->GetDesignGlyphMetrics(&glyph_index, 1, &glyph_metrics, faux);
                    facesize.x = glyph_metrics.advanceWidth;
                    facesize.y = metrics.ascent + metrics.descent + metrics.lineGap;
                    base_baseline = metrics.ascent + metrics.lineGap / 2.0f;
                    color = iscolor(faceinst);
                    //log("glyph_metrics.advanceWidth=", glyph_metrics.advanceWidth,
                    //    " metrics.designUnitsPerEm=", metrics.designUnitsPerEm,
                    //    " metrics.ascent=", metrics.ascent,
                    //    " metrics.descent=", metrics.descent,
                    //    " metrics.lineGap=", metrics.lineGap);

                    // formats 8, 10 and 12 with 32-bit encoding
                    // format 13 - last-resort
                    // format 14 for Unicode variation sequences
                    struct cmap_table
                    {
                        enum platforms
                        {
                            Unicode   = 0, // Various
                            Macintosh = 1, // Script manager code
                            ISO       = 2, // ISO encoding
                            Windows   = 3, // Windows encoding
                            Custom    = 4, // Custom encoding
                        };
                        enum encodings
                        {
                            Symbol            = 0, // Symbols
                            Windows_1_0       = 1, // Unicode BMP
                            Unicode_2_0       = 3, // Unicode 2.0 and onwards semantics, Unicode BMP only
                            Unicode_2_0f      = 4, // Unicode 2.0 and onwards semantics, Unicode full repertoire
                            Unicode_Variation = 5, // Unicode Variation Sequences—for use with subtable format 14
                            Unicode_full      = 6, // Unicode full repertoire—for use with subtable format 13
                            Windows_2_0       = 10,// Unicode full repertoire
                        };
                        struct rect
                        {
                            ui16 platformID;
                            ui16 encodingID;
                            ui32 subtableOffset; // Byte offset from beginning of table to the subtable for this encoding.
                        };
                        ui16 version;
                        ui16 numTables;
                        //rect records[];
                    };
                    struct colr_table
                    {
                        struct baseGlyphRecord
                        {
                            ui16 glyphID;         // Glyph ID of the base glyph.
                            ui16 firstLayerIndex; // Index (base 0) into the layerRecords array.
                            ui16 numLayers;       // Number of color layers associated with this glyph.
                        };
                        struct layerRecord
                        {
                            ui16 glyphID;      // Glyph ID of the glyph used for a given layer.
                            ui16 paletteIndex; // Index (base 0) for a palette entry in the CPAL table.
                        };
                        ui16 version;                   // Table version number—set to 0.
                        ui16 numBaseGlyphRecords;       // Number of base glyphs.
                        ui32 baseGlyphRecordsOffset;    // Offset to baseGlyphRecords array.
                        ui32 layerRecordsOffset;        // Offset to layerRecords array.
                        ui16 numLayerRecords;           // Number of Layer records.
                        //baseGlyphRecord baseGlyphRecs[];
                        //layerRecord     layersRecs[];
                    };

                    //auto cmap_data = (void const*)nullptr;
                    //auto cmap_size = ui32{};
                    //auto cmap_ctx = (void*)nullptr;
                    //auto exists = BOOL{};
                    //fontface->TryGetFontTable(DWRITE_MAKE_OPENTYPE_TAG('c','m','a','p'), &cmap_data, &cmap_size, &cmap_ctx, &exists);
                    // 1. cmap: codepoints -> indices
                    // 2. GSUB: indices -> indices
                    // 3. BASE: take font-wise metrics
                    // 4. GPOS: glyph positions
                    // 5. COLR+CPAL: multicolored glyphs (version 0)
                }
                fontfile->Release();
            }
            void load(IDWriteFontFamily* barefont)
            {
                fontface.resize(4);
                load(fontface[style::normal     ], barefont, DWRITE_FONT_WEIGHT_NORMAL,    DWRITE_FONT_STRETCH_NORMAL, DWRITE_FONT_STYLE_NORMAL);
                load(fontface[style::italic     ], barefont, DWRITE_FONT_WEIGHT_NORMAL,    DWRITE_FONT_STRETCH_NORMAL, DWRITE_FONT_STYLE_ITALIC);
                load(fontface[style::bold       ], barefont, DWRITE_FONT_WEIGHT_DEMI_BOLD, DWRITE_FONT_STRETCH_NORMAL, DWRITE_FONT_STYLE_NORMAL);
                load(fontface[style::bold_italic], barefont, DWRITE_FONT_WEIGHT_DEMI_BOLD, DWRITE_FONT_STRETCH_NORMAL, DWRITE_FONT_STYLE_ITALIC);
                auto names = (IDWriteLocalizedStrings*)nullptr;
                barefont->GetFamilyNames(&names);
                auto buff = wide(100, 0);
                names->GetString(0, buff.data(), (ui32)buff.size());
                log("%%Using font '%fontname%' (%iscolor%).", prompt::gui, utf::to_utf(buff.data()), color ? "color" : "monochromatic");
                names->Release();
            }

            //todo make software font
            typeface() = default;
            typeface(typeface&&) = default;
            typeface(IDWriteFontFamily* barefont, ui32 index, bool fixed = faux)
                : index{ index },
                  fixed{ fixed }
            {
                load(barefont);
            }
            ~typeface()
            {
                for (auto f : fontface) if (f) f->Release();
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

        font(std::list<text>& family_names)
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
            for (auto& family_utf8 : family_names)
            {
                auto found = BOOL{};   
                auto index = ui32{};
                auto family_utf16 = utf::to_utf(family_utf8);
                fontlist->FindFamilyName(family_utf16.data(), &index, &found);
                if (found)
                {
                    auto barefont = (IDWriteFontFamily*)nullptr;
                    fontlist->GetFontFamily(index, &barefont);
                    fontstat[index].s |= fontcat::loaded;
                    fallback.emplace_back(barefont, index, true);
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
                            if (auto faceinst = (IDWriteFontFace2*)nullptr; fontfile->CreateFontFace((IDWriteFontFace**)&faceinst), faceinst)
                            {
                                if (typeface::iscolor(faceinst)) fontstat[i].s |= fontcat::color;
                                auto numberOfFiles = ui32{};
                                faceinst->GetFiles(&numberOfFiles, nullptr);
                                auto fontFiles = std::vector<IDWriteFontFile*>(numberOfFiles);
                                if (S_OK == faceinst->GetFiles(&numberOfFiles, fontFiles.data()))
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
                                faceinst->Release();
                            }
                            fontfile->Release();
                        }
                        barefont->Release();
                    }
                }
                std::sort(fontstat.begin(), fontstat.end(), [](auto& a, auto& b){ return a.s > b.s; });
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
            for (auto& f : fallback) if ((f.color || f.fixed) && hittest(f.fontface[0])) return f;
            for (auto& f : fallback) if ((!f.color && !f.fixed) && hittest(f.fontface[0])) return f;
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
                                fallback.emplace_back(barefont, i);
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
                if ((fontstat[i].s & fontcat::valid && !(fontstat[i].s & fontcat::loaded)) && try_font(fontstat[i].i, true)) return fallback.back();
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
        twod  cellsz; // glyf: Narrow glyph black-box size in pixels.
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

        glyf(font& fcache, twod& cellsz, bool aamode)
            : fcache{ fcache },
              aamode{ aamode }
        {
            set_cellsz(cellsz);
        }
        void set_cellsz(twod& cellsz)
        {
            if (!fcache.fallback.empty()) // Keep fontface cell proportions until we implement box-drawing glyphs on our side.
            {
                auto facesize = fcache.fallback.front().facesize;
                cellsz.x = facesize.x * (10 * cellsz.y + 15) / (facesize.y * 10); // +1.5
                // Lucida Console 1.5ok
                // Courier New    1.5ok 17-2.0bad
                // Consolas       1.5ok
                // Iosevka Term   1.5ok
                // Cascadia Mono  1.5ok
            }
            if (cellsz.x < 1) cellsz.x = 1;
            if (cellsz.y < 2) cellsz.y = 2;
            this->cellsz = cellsz;
            log("%%Set cell size: ", prompt::gui, cellsz);
        }
        void rasterize(sprite& glyph_mask, cell const& c)
        {
            glyph_mask.type = sprite::alpha;
            if (c.xy() == 0) return;
            auto code_iter = utf::cpit{ c.txt() };
            codepoints.clear();
            auto flipandrotate = 0;
            auto monochromatic = faux;
            auto glyfalignment = bind{ snap::center, snap::center };
            while (code_iter)
            {
                auto codepoint = code_iter.next();
                if (codepoint.cdpoint >= utf::vs06_code && codepoint.cdpoint <= utf::vs16_code)
                {
                         if (codepoint.cdpoint == utf::vs15_code) monochromatic = true;
                    else if (codepoint.cdpoint == utf::vs16_code) monochromatic = faux;
                    else if (codepoint.cdpoint == utf::vs10_code) flipandrotate = (flipandrotate & 0b100) | ((flipandrotate + 0b001) & 0b011);
                    else if (codepoint.cdpoint == utf::vs11_code) flipandrotate = (flipandrotate & 0b100) | ((flipandrotate + 0b010) & 0b011);
                    else if (codepoint.cdpoint == utf::vs12_code) flipandrotate = (flipandrotate & 0b100) | ((flipandrotate + 0b011) & 0b011);
                    else if (codepoint.cdpoint == utf::vs13_code) flipandrotate = (flipandrotate ^ 0b100) | ((flipandrotate + (flipandrotate & 1 ? 0b010 : 0)) & 0b011);
                    else if (codepoint.cdpoint == utf::vs14_code) flipandrotate = (flipandrotate ^ 0b100) | ((flipandrotate + (flipandrotate & 1 ? 0 : 0b010)) & 0b011);
                    else if (codepoint.cdpoint == utf::vs06_code) glyfalignment.x = snap::head;
                    else if (codepoint.cdpoint == utf::vs07_code) glyfalignment.x = snap::tail;
                    else if (codepoint.cdpoint == utf::vs08_code) glyfalignment.y = snap::head;
                    else if (codepoint.cdpoint == utf::vs09_code) glyfalignment.y = snap::tail;
                }
                else codepoints.push_back(codepoint);
            }
            if (codepoints.empty()) return;

            auto format = font::style::normal;
            if (c.itc()) format |= font::style::italic;
            if (c.bld()) format |= font::style::bold;
            auto& f = fcache.take_font(codepoints.front().cdpoint);
            auto font_face = f.fontface[format];
            if (!font_face) return;

            auto fs = fp2d{ f.facesize };
            f.transform = (cellsz.y + 1.5f) / fs.y;
            //f.transform = std::min((cellsz.x + 1.0f) / fs.x, (cellsz.y + 1.5f) / fs.y);
            auto half_pixel = fs.y / cellsz.y * 0.80f;
            // consolas (top 13, 1.5-0.80ok) (top 13, 1.5-0.79bad) (top 13, 1.5-0.78bad) (top 13, 1.5-0.77bad) (top 13, 1.5-0.76bad) (top 13, 1.5-0.75bad) (top 12, 1.5-0.80ok) (top 12, 1.5-0.85ok) (top 12, 1.5-0.96ok) (top 12, 1.5-1.0ok) (top 12, 1.5-0.90ok) (top 12, 1.4-1.0ok) (top 12, 1.4-0.90bad) (top 12, 1.4-0.89bad) (top 12, 1.3-0.85bad) (top 12, 1.3-0.82bad) (top 12, 1.3-0.89ok) (top 12, 1.3-0.8bad) (top 1.3-0.9ok, 1.2-1.0ok)
            // courier  (btm 21, 1.5-0.75ok) (btm 21, 1.5-0.80ok) (btm 21, 1.5-0.96ok) (btm 21, 1.5-0.97bad) (btm 21, 1.5-0.98bad) (btm 21, 1.5-0.95ok) (btm 21, 1.5-1.0bad) (btm 21, 1.5-0.90ok) (btm 21, 1.4-0.90ok) (btm 21, 1.4-0.91bad) (btm 21, 1.4-0.93bad) (btm 17, 1.4-0.95bad) (btm 17, 1.4-1.0bad) (btm 21, 1.3-0.82ok) (btm 21, 1.3-0.85bad) (btm 17, 1.3-0.89bad) (btm 22, 1.3-0.8ok) (btm 22, 1.2-0.5ok) (btm 17, 1.3-0.9bad)
            // Iosevka Term   1.5-0.80ok  1.5-0.75ok
            // Cascadia Mono  1.5-0.80ok  1.5-0.75ok
            // Lucida Console 1.5-0.80ok  1.5-0.75ok
            f.face_baseline.y = (f.base_baseline - half_pixel) * f.transform;
            if (format & font::style::italic) f.transform *= 0.85f;// + _k0*0.05f; //todo revise, or make it configurable via settings
            f.face_emheight = f.base_emheight * f.transform;

            auto transform = f.transform;
            auto base_line = f.face_baseline;
            auto em_height = f.face_emheight;
            //todo use otf tables directly: GSUB etc
            //gindex.resize(codepoints.size());
            //hr = font_face->GetGlyphIndices(codepoints.data(), (ui32)codepoints.size(), gindex.data());
            //auto glyph_run = DWRITE_GLYPH_RUN{ .fontFace     = font_face,
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

            //todo make it configurable (and font_face based)
            //auto fs = std::to_array<std::pair<ui32, ui32>>({ { DWRITE_MAKE_OPENTYPE_TAG('s', 'a', 'l', 't'), 1 }, });
            //auto const features = std::to_array({ DWRITE_TYPOGRAPHIC_FEATURES{ (DWRITE_FONT_FEATURE*)fs.data(), (ui32)fs.size() }});
            //auto feat_table = features.data();

            auto script_opt = DWRITE_SCRIPT_ANALYSIS{ .script = font::msscript(unidata::script(codepoints.front().cdpoint)) };
            auto hr = fcache.analyzer->GetGlyphs(
                text_utf16.data(),       //_In_reads_(textLength) WCHAR const* textString,
                text_count,              //UINT32 textLength,
                font_face,               //_In_ IDWriteFontFace* fontFace,
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

            hr = fcache.analyzer->GetGlyphPlacements(
                text_utf16.data(),                 // _In_reads_(textLength) WCHAR const* textString,
                clustermap.data(),                 // _In_reads_(textLength) UINT16 const* clusterMap,
                text_props.data(),                 // _Inout_updates_(textLength) DWRITE_SHAPING_TEXT_PROPERTIES* textProps,
                text_count,                        // UINT32 textLength,
                glyf_index.data(),                 // _In_reads_(glyphCount) UINT16 const* glyphIndices,
                glyf_props.data(),                 // _In_reads_(glyphCount) DWRITE_SHAPING_GLYPH_PROPERTIES const* glyphProps,
                glyf_count,                        // UINT32 glyphCount,
                font_face,                         // _In_ IDWriteFontFace* fontFace,
                em_height,                         // FLOAT fontEmSize,
                faux,                              // BOOL isSideways,
                faux,                              // BOOL isRightToLeft,
                &script_opt,                       // _In_ DWRITE_SCRIPT_ANALYSIS const* scriptAnalysis,
                fcache.oslocale.data(),            // _In_opt_z_ WCHAR const* localeName,
                nullptr,//&f.feat_table,           // _In_reads_opt_(featureRanges) DWRITE_TYPOGRAPHIC_FEATURES const** features,
                &text_count,                       // _In_reads_opt_(featureRanges) UINT32 const* featureRangeLengths,
                0,//f.features.size(),             // UINT32 featureRanges,
                glyf_steps.data(),                 // _Out_writes_(glyphCount) FLOAT* glyphAdvances,
                glyf_align.data());                // _Out_writes_(glyphCount) DWRITE_GLYPH_OFFSET* glyphOffsets
            if (hr != S_OK) return;

            hr = font_face->GetDesignGlyphMetrics(glyf_index.data(), glyf_count, glyf_sizes.data(), faux);
            if (hr != S_OK) return;
            auto length = fp32{};
            auto penpos = fp32{};
            auto matrix = fp2d{ c.mtx() * cellsz };
            auto swapxy = flipandrotate & 1;
            if (swapxy) std::swap(matrix.x, matrix.y);
            for (auto i = 0u; i < glyf_count; ++i)
            {
                auto w = glyf_sizes[i].advanceWidth;
                auto r = glyf_sizes[i].rightSideBearing;
                auto bearing = ((si32)w - r) * transform;
                auto right_most = penpos + glyf_align[i].advanceOffset + bearing;
                length = std::max(length, right_most);
                penpos += glyf_steps[i];
            }
            auto actual_width = std::max(1.f, std::floor((length + cellsz.x * 0.40f) / cellsz.x)) * cellsz.x;
            auto actual_height = (fp32)cellsz.y;
            if (actual_width > matrix.x) // Check if the glyph exceeds the matrix width. (scale down)
            {
                auto k = matrix.x / actual_width;
                actual_width = matrix.x;
                em_height *= k;
                for (auto& w : glyf_steps) w *= k;
                for (auto& [h, v] : glyf_align) h *= k;
            }
            else if (actual_height < matrix.y && actual_width <= matrix.x - cellsz.x) // Check if the glyph is too small for the matrix. (scale up)
            {
                auto k = std::min(matrix.x / actual_width, matrix.y / actual_height);
                actual_width *= k;
                actual_height *= k;
                base_line.y *= k;
                em_height *= k;
                for (auto& w : glyf_steps) w *= k;
                for (auto& [h, v] : glyf_align) h *= k;
            }
            if (actual_width <= matrix.x - cellsz.x / 2.f) // Hz alignment.
            {
                     if (glyfalignment.x == snap::center) base_line.x += (matrix.x - actual_width) / 2.f;
                else if (glyfalignment.x == snap::tail  ) base_line.x += matrix.x - actual_width;
            }
            if (actual_height < matrix.y - cellsz.y / 2.f) // Vt alignment.
            {
                     if (glyfalignment.y == snap::center) base_line.y += (matrix.y - actual_height) / 2.f;
                else if (glyfalignment.y == snap::tail  ) base_line.y += matrix.y - actual_height;
            }
            auto glyph_run = DWRITE_GLYPH_RUN{ .fontFace      = font_face,
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
            auto pixel_fit_mode = DWRITE_GRID_FIT_MODE_ENABLED;
            auto aaliasing_mode = DWRITE_TEXT_ANTIALIAS_MODE_GRAYSCALE; //DWRITE_TEXT_ANTIALIAS_MODE_CLEARTYPE
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
        void draw_cell(auto& canvas, twod coor, cell const& c)
        {
            auto placeholder = canvas.area().trim(rect{ coor, cellsz });
            if (!placeholder) return;
            if (c.bga()) { netxs::misc::fill(canvas, placeholder, cell::shaders::full(c.bgc())); }
            if (c.und()) { }
            if (c.stk()) { }
            if (c.ovr()) { }
            if (c.inv()) { }
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
            auto box = glyph_mask.area.shift(coor - twod{ cellsz.x * (x - 1), cellsz.y * (y - 1) });
            canvas.clip(placeholder);
            //canvas.clip(canvas.area());

            auto fgc = c.fgc();
            auto f_fgc = irgb{ c.fgc() }.sRGB2Linear();
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
                netxs::onclip(canvas, raster, fx);
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
                netxs::onclip(canvas, raster, fx);
            }
        }
        void fill_grid(auto& canvas, twod origin, auto& grid_cells)
        {
            auto coor = origin;
            auto size = grid_cells.size() * cellsz;
            auto maxc = coor + size;
            auto base = canvas.coor();
            canvas.step(-base);
            for (auto& c : grid_cells)
            {
                draw_cell(canvas, coor, c);
                coor.x += cellsz.x;
                if (coor.x >= maxc.x)
                {
                    coor.x = origin.x;
                    coor.y += cellsz.y;
                    if (coor.y >= maxc.y) break;
                }
            }
            canvas.step(base);
        }
    };

    struct surface
    {
        using bits = netxs::raster<std::span<argb>, rect>;

        HDC   hdc;
        HWND hWnd;
        bool sync;
        rect prev;
        rect area;
        twod size;
        bits data;

        surface(surface const&) = default;
        surface(surface&&) = default;
        surface(HWND hWnd)
            :  hdc{ ::CreateCompatibleDC(NULL)}, // Only current thread owns hdc.
              hWnd{ hWnd },
              sync{ faux },
              prev{ .coor = dot_mx },
              area{ dot_00, dot_00 },
              size{ dot_00 }
        { }
        void reset() // We don't use custom copy/move ctors.
        {
            if (hdc) ::DeleteDC(hdc);
        }
        void set_dpi(auto /*dpi*/) // We are do not rely on dpi. Users should configure all metrics in pixels.
        { }
        auto canvas(bool wipe = faux)
        {
            if (area)
            {
                if (area.size != size)
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
                        size = area.size;
                        data = bits{ std::span<argb>{ (argb*)ptr, (sz_t)size.x * size.y }, rect{ area.coor, size }};
                    }
                    else log("%%Compatible bitmap creation error: %ec%", prompt::gui, ::GetLastError());
                }
                if (wipe) std::memset(data.data(), 0, (sz_t)size.x * size.y * sizeof(argb));
                sync = faux;
            }
            data.move(area.coor);
            return data;
        }
        void present()
        {
            if (sync || !hdc) return;
            static auto blend_props = BLENDFUNCTION{ .BlendOp = AC_SRC_OVER, .SourceConstantAlpha = 255, .AlphaFormat = AC_SRC_ALPHA };
            auto scr_coor = POINT{};
            auto old_size =  SIZE{ prev.size.x, prev.size.y };
            auto old_coor = POINT{ prev.coor.x, prev.coor.y };
            auto win_size =  SIZE{      size.x,      size.y };
            auto win_coor = POINT{ area.coor.x, area.coor.y };
            //auto sized = prev.size(     size);
            auto moved = prev.coor(area.coor);
            auto rc = ::UpdateLayeredWindow(hWnd,   // 1.5 ms (syscall, copy bitmap to hardware)
                                            HDC{},                       // No color palette matching.  HDC hdcDst,
                                            moved ? &win_coor : nullptr, // POINT         *pptDst,
                                            &win_size,                   // SIZE          *psize,
                                            hdc,                         // HDC           hdcSrc,
                                            &scr_coor,                   // POINT         *pptSrc,
                                            {},                          // COLORREF      crKey,
                                            &blend_props,                // BLENDFUNCTION *pblend,
                                            ULW_ALPHA);                  // DWORD         dwFlags
            if (!rc) log("%%UpdateLayeredWindow returns unexpected result ", prompt::gui, rc);
            sync = true;
        }
    };

    struct manager
    {
        using wins = std::vector<surface>;

        enum bttn
        {
            left   = 1 << 0,
            right  = 1 << 1,
            middle = 1 << 2,
        };

        font fcache; // manager: Font cache.
        glyf gcache; // manager: Glyph cache.
        bool isfine; // manager: All is ok.
        wins layers; // manager: ARGB layers.

        manager(std::list<text>& font_names_utf8, twod& cellsz, bool antialiasing)
            : fcache{ font_names_utf8 },
              gcache{ fcache, cellsz, antialiasing },
              isfine{ true }
        {
            set_dpi_awareness();
        }
        ~manager()
        {
            for (auto& w : layers) w.reset();
        }
        auto& operator [] (si32 layer) { return layers[layer]; }
        explicit operator bool () const { return isfine; }

        void set_dpi_awareness()
        {
            auto proc = (LONG(_stdcall *)(si32))::GetProcAddress(::GetModuleHandleA("user32.dll"), "SetProcessDpiAwarenessInternal");
            if (proc)
            {
                auto hr = proc(2/*PROCESS_PER_MONITOR_DPI_AWARE*/);
                if (hr != S_OK || hr != E_ACCESSDENIED) log("%%Set DPI awareness failed %hr% %ec%", prompt::gui, utf::to_hex(hr), ::GetLastError());
            }
        }
        auto set_dpi(auto new_dpi)
        {
            //for (auto& w : layers) w.set_dpi(new_dpi);
            log("%%DPI changed to %dpi%", prompt::gui, new_dpi);
        }
        auto moveby(twod delta)
        {
            for (auto& w : layers) w.area.coor += delta;
        }
        template<bool JustMove = faux>
        void present()
        {
            if constexpr (JustMove)
            {
                auto lock = ::BeginDeferWindowPos((si32)layers.size());
                for (auto& w : layers)
                {
                    lock = ::DeferWindowPos(lock, w.hWnd, 0, w.area.coor.x, w.area.coor.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
                    if (!lock) { log("%%DeferWindowPos returns unexpected result: %ec%", prompt::gui, ::GetLastError()); }
                    w.prev.coor = w.area.coor;
                }
                ::EndDeferWindowPos(lock);
            }
            else for (auto& w : layers) w.present(); // 3.000 ms
        }
        void dispatch()
        {
            auto msg = MSG{};
            while (::GetMessageW(&msg, 0, 0, 0) > 0)
            {
                ::DispatchMessageW(&msg);
            }
        }
        void shown_event(bool shown, arch reason)
        {
            log(shown ? "shown" : "hidden", " ", reason == SW_OTHERUNZOOM   ? "The window is being uncovered because a maximize window was restored or minimized."
                                               : reason == SW_OTHERZOOM     ? "The window is being covered by another window that has been maximized."
                                               : reason == SW_PARENTCLOSING ? "The window's owner window is being minimized."
                                               : reason == SW_PARENTOPENING ? "The window's owner window is being restored."
                                                                            : "unknown reason");
        }
        void show(si32 win_state)
        {
            if (win_state == 0 || win_state == 2) //todo fullscreen mode (=2). 0 - normal, 1 - minimized, 2 - fullscreen
            {
                auto mode = SW_SHOWNORMAL;
                for (auto& w : layers) { ::ShowWindow(w.hWnd, mode); }
            }
        }
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
        void activate()
        {
            log("activated");
            if (!layers.empty()) ::SetActiveWindow(layers.front().hWnd);
        }
        void state_event(bool activated, bool minimized)
        {
            log(activated ? "activated" : "deactivated", " ", minimized ? "minimized" : "restored");
        }

        virtual void update() = 0;
        virtual void mouse_shift(twod coord) = 0;
        virtual void focus_event(bool state) = 0;
        virtual void mouse_press(si32 index, bool pressed) = 0;
        virtual void mouse_wheel(si32 delta, bool hzwheel) = 0;
        virtual void keybd_press(arch vkey, arch lParam) = 0;

        auto add(manager* host_ptr = nullptr)
        {
            auto window_proc = [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
            {
                //log("\tmsW=", utf::to_hex(msg), " wP=", utf::to_hex(wParam), " lP=", utf::to_hex(lParam), " hwnd=", utf::to_hex(hWnd));
                //auto layer = (si32)::GetWindowLongPtrW(hWnd, sizeof(LONG_PTR));
                auto w = (manager*)::GetWindowLongPtrW(hWnd, GWLP_USERDATA);
                if (!w) return ::DefWindowProcW(hWnd, msg, wParam, lParam);
                auto stat = LRESULT{};
                static auto hi = [](auto n){ return (si32)(si16)((n >> 16) & 0xffff); };
                static auto lo = [](auto n){ return (si32)(si16)((n >> 0 ) & 0xffff); };
                static auto hover_win = testy<HWND>{};
                static auto hover_rec = TRACKMOUSEEVENT{ .cbSize = sizeof(TRACKMOUSEEVENT), .dwFlags = TME_LEAVE, .dwHoverTime = HOVER_DEFAULT };
                static auto h = 0;
                static auto f = 0;
                switch (msg)
                {
                    case WM_MOUSEMOVE:
                        if (hover_win(hWnd)) ::TrackMouseEvent((++h, hover_rec.hwndTrack = hWnd, &hover_rec));
                        if (auto r = RECT{}; ::GetWindowRect(hWnd, &r)) w->mouse_shift({ r.left + lo(lParam), r.top + hi(lParam) });
                        break;
                    case WM_MOUSELEAVE:  if (!--h) w->mouse_shift(dot_mx), hover_win = {}; break; //todo reimplement mouse leaving
                    case WM_ACTIVATEAPP: if (!(wParam ? f++ : --f)) w->focus_event(f); break; // Focus between apps.
                    case WM_ACTIVATE:      w->state_event(!!lo(wParam), !!hi(wParam)); break; // Window focus within the app.
                    case WM_MOUSEACTIVATE: w->activate(); stat = MA_NOACTIVATE;        break; // Suppress window activation with a mouse click.
                    case WM_LBUTTONDOWN:   w->mouse_press(bttn::left,   true);         break;
                    case WM_MBUTTONDOWN:   w->mouse_press(bttn::middle, true);         break;
                    case WM_RBUTTONDOWN:   w->mouse_press(bttn::right,  true);         break;
                    case WM_LBUTTONUP:     w->mouse_press(bttn::left,   faux);         break;
                    case WM_MBUTTONUP:     w->mouse_press(bttn::middle, faux);         break;
                    case WM_RBUTTONUP:     w->mouse_press(bttn::right,  faux);         break;
                    case WM_MOUSEWHEEL:    w->mouse_wheel(hi(wParam), faux);           break;
                    case WM_MOUSEHWHEEL:   w->mouse_wheel(hi(wParam), true);           break;
                    case WM_SHOWWINDOW:    w->shown_event(!!wParam, lParam);           break; //todo revise
                    //case WM_GETMINMAXINFO: w->maximize(wParam, lParam);              break; // The system is about to maximize the window.
                    //case WM_SYSCOMMAND:  w->sys_command(wParam, lParam);             break; //todo taskbar ctx menu to change the size and position
                    case WM_KEYDOWN:
                    case WM_KEYUP:
                    case WM_SYSKEYDOWN:  // WM_CHAR/WM_SYSCHAR and WM_DEADCHAR/WM_SYSDEADCHAR are derived messages after translation.
                    case WM_SYSKEYUP:      w->keybd_press(wParam, lParam);             break;
                    case WM_DPICHANGED:    w->set_dpi(lo(wParam));                     break;
                    case WM_DESTROY:       ::PostQuitMessage(0);                       break;
                    //dx3d specific
                    //case WM_PAINT:   /*w->check_dx3d_state();*/ stat = ::DefWindowProcW(hWnd, msg, wParam, lParam); break;
                    default:                                    stat = ::DefWindowProcW(hWnd, msg, wParam, lParam); break;
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
                                          WS_POPUP /*todo | owner ? WS_SYSMENU : 0  taskbar ctx menu*/, 0, 0, 0, 0, owner, 0, 0, 0);
            auto layer = (si32)layers.size();
            if (!hWnd)
            {
                isfine = faux;
                log("%%Window creation error: %ec%", prompt::gui, ::GetLastError());
            }
            else if (host_ptr)
            {
                ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)host_ptr);
                //::SetWindowLongPtrW(hWnd, 0, (LONG_PTR)host_ptr);
                //::SetWindowLongPtrW(hWnd, sizeof(LONG_PTR), (LONG_PTR)layer);
            }
            layers.emplace_back(hWnd);
            return layer;
        }
    };

    struct window : manager
    {
        using gray = netxs::raster<std::vector<byte>, rect>;
        using shad = netxs::misc::shadow<gray>;
        using grip = netxs::misc::szgrips;

        struct task
        {
            static constexpr auto _counter = 1 + __COUNTER__;
            static constexpr auto moved  = 1 << (__COUNTER__ - _counter);
            static constexpr auto sized  = 1 << (__COUNTER__ - _counter);
            static constexpr auto grips  = 1 << (__COUNTER__ - _counter);
            static constexpr auto hover  = 1 << (__COUNTER__ - _counter);
            static constexpr auto inner  = 1 << (__COUNTER__ - _counter);
            static constexpr auto header = 1 << (__COUNTER__ - _counter);
            static constexpr auto footer = 1 << (__COUNTER__ - _counter);
            static constexpr auto all = -1;
        };

        ui::face main_grid;
        ui::face head_grid;
        ui::face foot_grid;

        twod gridsz; // window: Grid size in cells.
        twod cellsz; // window: Cell size in pixels.
        twod gripsz; // window: Resizing grips size in pixels.
        dent border; // window: Border around window for resizing grips (dent in pixels).
        shad shadow; // window: Shadow generator.
        grip szgrip; // window: Resizing grips UI-control.
        twod mcoord; // window: Mouse cursor coord.
        si32 mbttns; // window: Mouse button state.
        si32 reload; // window: Changelog for update.
        si32 client; // window: Surface index for Client.
        si32 grip_l; // window: Surface index for Left resizing grip.
        si32 grip_r; // window: Surface index for Right resizing grip.
        si32 grip_t; // window: Surface index for Top resizing grip.
        si32 grip_b; // window: Surface index for Bottom resizing grip.
        si32 header; // window: Surface index for Header.
        si32 footer; // window: Surface index for Footer.
        bool drop_shadow{ true };

        //test
        twod scroll_pos;
        twod scroll_origin;
        twod scroll_delta;

        static constexpr auto shadow_dent = dent{ 1,1,1,1 } * 3;

        window(rect win_coor_px_size_cell, std::list<text>& font_names, twod cell_size = { 10, 20 }, si32 win_mode = 0, twod grip_cell = { 2, 1 }, bool antialiasing = faux)
            : manager{ font_names, cell_size, antialiasing },
              gridsz{ std::max(dot_11, win_coor_px_size_cell.size) },
              cellsz{ cell_size },
              gripsz{ grip_cell * cell_size },
              border{ gripsz.x, gripsz.x, gripsz.y, gripsz.y },
              shadow{ 0.44f/*bias*/, 116.5f/*alfa*/, gripsz.x, dot_00, dot_11, cell::shaders::full },
              mbttns{},
              reload{ task::all },
              client{ add(this) },
              grip_l{ add(this) },
              grip_r{ add(this) },
              grip_t{ add(this) },
              grip_b{ add(this) },
              header{ add() },
              footer{ add() }
        {
            if (!*this) return;
            layers[client].area = { win_coor_px_size_cell.coor, gridsz * cellsz };
            recalc_layout();
            //todo temp
            main_grid.size(layers[client].area.size / cellsz);
            main_grid.cup(dot_00);
            main_grid.output<true>(canvas_page);
            head_grid.size(layers[header].area.size / cellsz);
            head_grid.cup(dot_00);
            head_grid.output(header_page);
            foot_grid.size(layers[footer].area.size / cellsz);
            foot_grid.cup(dot_00);
            foot_grid.output(footer_page);
            update();
            manager::show(win_mode);
        }
        void recalc_layout()
        {
            auto base_rect = layers[client].area;
            auto grid_size = base_rect.size / cellsz;
            auto c_size = grid_size;
            auto h_size = grid_size;
            auto f_size = grid_size;
            main_grid.calc_page_height(canvas_page, c_size);
            head_grid.calc_page_height(header_page, h_size);
            head_grid.calc_page_height(footer_page, f_size);
            footer_page = ansi::wrp(wrap::on).jet(bias::right).fgc(tint::purewhite).add(scroll_pos.x, ":", scroll_pos.y, "/", c_size.y, " ",grid_size.x, ":", grid_size.y);
            layers[grip_l].area = base_rect + dent{ gripsz.x, -base_rect.size.x, gripsz.y, gripsz.y };
            layers[grip_r].area = base_rect + dent{ -base_rect.size.x, gripsz.x, gripsz.y, gripsz.y };
            layers[grip_t].area = base_rect + dent{ 0, 0, gripsz.y, -base_rect.size.y };
            layers[grip_b].area = base_rect + dent{ 0, 0, -base_rect.size.y, gripsz.y };
            auto header_height = cellsz.y * h_size.y;
            auto footer_height = cellsz.y * f_size.y;
            layers[header].area = base_rect + dent{ 0, 0, header_height, -base_rect.size.y } + shadow_dent;
            layers[footer].area = base_rect + dent{ 0, 0, -base_rect.size.y, footer_height } + shadow_dent;
            layers[header].area.coor.y -= shadow_dent.b;
            layers[footer].area.coor.y += shadow_dent.t;
        }
        auto move_window(twod coor_delta)
        {
            manager::moveby(coor_delta);
            reload |= task::moved;
        }
        auto size_window(twod size_delta, bool wipe = faux)
        {
            layers[client].area.size += size_delta;
            recalc_layout();
            reload |= task::sized;
            //todo temp
            main_grid.size(layers[client].area.size / cellsz);
            if (wipe)
            {
                main_grid.wipe();
                foot_grid.wipe();
            }
            main_grid.zz(scroll_pos);
            main_grid.vsize(std::min(0, -scroll_pos.y) + layers[client].area.size.y);
            main_grid.output<true>(canvas_page);
            head_grid.size(layers[header].area.size / cellsz);
            head_grid.cup(dot_00);
            head_grid.output(header_page);
            foot_grid.size(layers[footer].area.size / cellsz);
            foot_grid.cup(dot_00);
            foot_grid.output(footer_page);
        }
        auto resize_window(twod size_delta)
        {
            auto old_client = layers[client].area;
            auto new_gridsz = std::max(dot_11, (old_client.size + size_delta) / cellsz);
            size_delta = dot_00;
            if (gridsz != new_gridsz)
            {
                gridsz = new_gridsz;
                size_delta = gridsz * cellsz - old_client.size;
                size_window(size_delta);
            }
            return size_delta;
        }
        auto warp_window(dent warp_delta)
        {
            auto old_client = layers[client].area;
            auto new_client = old_client + warp_delta;
            auto new_gridsz = std::max(dot_11, new_client.size / cellsz);
            if (gridsz != new_gridsz)
            {
                gridsz = new_gridsz;
                auto size_delta = gridsz * cellsz - old_client.size;
                auto coor_delta = new_client.coor - old_client.coor;
                size_window(size_delta);
                move_window(coor_delta);
            }
            return layers[client].area - old_client;
        }
        void fill_back(auto& grid_cells)
        {
            auto rtc = argb{ tint::pureblue  };//.alpha(0.5f);
            auto ltc = argb{ tint::pureblack };
            auto rbc = argb{ tint::purered };
            auto lbc = argb{ tint::puregreen  };//.alpha(0.5f);
            auto white = argb{ tint::whitedk };
            auto black = argb{ tint::blackdk };
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
            auto inner_rect = layers[client].area;
            auto outer_rect = layers[client].area + border;
            auto hit = !szgrip.zoomon && (szgrip.seized || (outer_rect.hittest(mcoord) && !inner_rect.hittest(mcoord)));
            return hit;
        }
        void fill_grips(rect area, auto fx)
        {
            for (auto g : { grip_l, grip_r, grip_t, grip_b })
            {
                auto& layer = layers[g];
                if (auto r = layer.area.trim(area))
                {
                    auto canvas = layer.canvas();
                    fx(canvas, r);
                }
            }
        }
        void draw_grips()
        {
            static auto trans = 0x01'00'00'00;
            static auto shade = 0x5F'3f'3f'3f;
            static auto black = 0x3F'00'00'00;
            auto inner_rect = layers[client].area;
            auto outer_rect = layers[client].area + border;
            fill_grips(outer_rect, [](auto& canvas, auto r){ netxs::misc::fill(canvas, r, cell::shaders::full(trans)); });
            if (hit_grips())
            {
                auto s = szgrip.sector;
                auto [side_x, side_y] = szgrip.layout(outer_rect);
                auto dent_x = dent{ s.x < 0, s.x > 0, s.y > 0, s.y < 0 };
                auto dent_y = dent{ s.x > 0, s.x < 0, 1, 1 };
                fill_grips(side_x, [&](auto& canvas, auto r)
                {
                    netxs::misc::fill(canvas, r, cell::shaders::full(shade));
                    netxs::misc::cage(canvas, side_x, dent_x, cell::shaders::full(black)); // 1-px dark contour around.
                });
                fill_grips(side_y, [&](auto& canvas, auto r)
                {
                    netxs::misc::fill(canvas, r, cell::shaders::full(shade));
                    netxs::misc::cage(canvas, side_y, dent_y, cell::shaders::full(black)); // 1-px dark contour around.
                });
            }
            if (drop_shadow) fill_grips(outer_rect, [&](auto& canvas, auto r)
            {
                shadow.render(canvas, r, inner_rect, cell::shaders::alpha);
            });
        }
        void draw_title(si32 index, auto& facedata) //todo just output ui::core
        {
            auto canvas = layers[index].canvas(true);
            gcache.fill_grid(canvas, shadow_dent.corner(), facedata);
            netxs::misc::contour(canvas); // 1ms
        }
        void draw_header() { draw_title(header, head_grid); }
        void draw_footer() { draw_title(footer, foot_grid); }
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
                    auto canvas = layers[client].canvas();
                    fill_back(main_grid);
                    gcache.fill_grid(canvas, dot_00, main_grid); // 0.500 ms);
                }
                if (what & (task::sized | task::hover | task::grips)) draw_grips(); // 0.150 ms
                if (what & (task::sized | task::header)) draw_header();
                if (what & (task::sized | task::footer)) draw_footer();
                //if (layers[client].area.hittest(mcoord))
                //{
                //    auto cursor = rect{ mcoord - (mcoord - layers[client].area.coor) % cellsz, cellsz };
                //    netxs::onrect(layers[client].canvas(), cursor, cell::shaders::full(0x7F'00'3f'00));
                //}
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
            auto state = hids::LShift   * !!::GetAsyncKeyState(VK_LSHIFT)
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
            return state;
        }
        void mouse_wheel(si32 delta, bool hz)
        {
            auto wheeldt = delta / 120;
            auto kb = kbs();
            if (kb & hids::LCtrl)
            {
                netxs::_k0 += wheeldt > 0 ? 1 : -1; // LCtrl+Wheel.
                log("_k0=", _k0);
                gcache.glyphs.clear();
                static auto orig_sz = cellsz;
                auto grip_cell = gripsz / cellsz;
                cellsz.y = orig_sz.y + _k0;
                gcache.set_cellsz(cellsz);
                gripsz = grip_cell * cellsz;
                border = { gripsz.x, gripsz.x, gripsz.y, gripsz.y };
                shadow.generate(0.44f/*bias*/, 116.5f/*alfa*/, gripsz.x, dot_00, dot_11, cell::shaders::full);
                layers[client].area = rect{ layers[client].area.coor, gridsz * cellsz };
                recalc_layout();
                reload |= task::all;
                return;
            }
            //     if (kb & (hids::LCtrl | hids::LAlt)) netxs::_k2 += wheeldt > 0 ? 1 : -1; // LCtrl + Alt t +Wheel.
            //else if (kb & hids::LCtrl)                netxs::_k0 += wheeldt > 0 ? 1 : -1; // LCtrl+Wheel.
            //else if (kb & hids::anyAlt)               netxs::_k1 += wheeldt > 0 ? 1 : -1; // Alt+Wheel.
            //else if (kb & hids::RCtrl)                netxs::_k3 += wheeldt > 0 ? 1 : -1; // RCtrl+Wheel.
            //shadow = build_shadow_corner(cellsz.x);
            //reload |= task::sized;
            //netxs::_k0 += wheeldt > 0 ? 1 : -1;
            //log("wheel ", wheeldt, " k0= ", _k0, " k1= ", _k1, " k2= ", _k2, " k3= ", _k3, " keybd ", utf::to_bin(kb));

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
                hz ? scroll_pos.x += wheeldt
                   : scroll_pos.y += wheeldt;
                size_window({}, true);
                reload |= task::all;
                reload &= ~task::sized;
            }
            if (szgrip.zoomon)
            {
                auto warp = dent{ gripsz.x, gripsz.x, gripsz.y, gripsz.y };
                auto step = szgrip.zoomdt + warp * wheeldt;
                auto next = szgrip.zoomsz + step;
                next.size = std::max(dot_00, next.size);
                ///auto viewport = ...get max win size (multimon)
                //next.trimby(viewport);
                if (warp_window(next - layers[client].area)) szgrip.zoomdt = step;
            }
        }
        void mouse_shift(twod coord)
        {
            auto kb = kbs();// keybd_state();
            auto inner_rect = layers[client].area;
            if (mbttns & bttn::right)
            {
                scroll_delta += coord - mcoord;
                if (scroll_pos(scroll_origin + scroll_delta / cellsz))
                {
                    size_window({}, true);
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
                    manager::moveby(dxdy);
                    reload |= task::moved;
                }
            }
            mcoord = coord;
            if (!mbttns)
            {
                static auto s = testy{ faux };
                reload |= s(hit_grips()) ? task::grips | task::inner
                                     : s ? task::grips : task::inner;
            }
        }
        void mouse_press(si32 button, bool pressed)
        {
            if (pressed && !mbttns) mouse_capture();
            pressed ? mbttns |= button
                    : mbttns &= ~button;
            if (!mbttns) mouse_release();
            //if (!pressed & (button == bttn::right)) manager::close();
            if (pressed & (button == bttn::right))
            {
                scroll_origin = scroll_pos;
                scroll_delta = {};
            }
        }
        void keybd_press(arch vkey, arch lParam)
        {
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
            if (vkey == 0x1b) manager::close();
            kbs() = keybd_state();
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
        }
    };
}