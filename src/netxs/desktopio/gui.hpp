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
            static constexpr auto update       = __COUNTER__ - _counter;
            static constexpr auto close        = __COUNTER__ - _counter;
        };
        
        bool isfine = true; // manager_base: All is ok.
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
                    base_x_height = std::max(1, (si32)m.xHeight);
                    base_ascent = std::max(1.f, m.ascent + m.lineGap / 2.0f);
                    base_descent = std::max(1.f, m.descent + m.lineGap / 2.0f);
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
                    transform = std::min(transform, cellsize.x / facesize.x);
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
                    log("%%Using font '%fontname%' (%iscolor%). Index %index%.", prompt::gui, f.font_name, f.color ? "color" : "monochromatic", fallback.size() - 1);
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
        struct synthetic
        {
            static constexpr auto _counter = 1 + __COUNTER__;
            static constexpr auto wavyunderline = __COUNTER__ - _counter;
        };

        using gmap = std::unordered_map<ui64, sprite>;

        std::pmr::unsynchronized_pool_resource buffer_pool; // glyf: Pool for temp buffers.
        std::pmr::monotonic_buffer_resource    mono_buffer; // glyf: Memory block for sprites.
        font& fcache; // glyf: Font cache.
        twod& cellsz; // glyf: Terminal cell size in pixels.
        bool  aamode; // glyf: Enable AA.
        gmap  glyphs; // glyf: Glyph map.
        std::vector<sprite>                          cgi_glyphs; // glyf: Synthetic glyphs.
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
        {
            generate_glyphs();
        }
        void generate_glyphs()
        {
            // Generate wavy underline.
            auto block = fcache.wavyline;
            auto height = block.size.y;
            auto thick = fcache.underline.size.y;
            auto vsize = std::max(1, block.size.y - thick + 1) / 2.f; // Vertical space/amp for wave.
            auto y0 = block.coor.y + vsize;
            vsize *= 0.99f; // Make the wave amp a little smaller to aviod pixels get outside.
            auto fract = (thick * 3) & ~1; // &~1: To make it look better for small sizes.
            auto width = block.size.x + fract * 4; // Bump for texture sliding.
            auto k = 3.14f / 2.f / fract; // Aling fract with the sine period.
            block.size.x = 1;
            block.size.y = thick;
            auto c = byte{ 255 }; // Opaque alpha texture.
            auto& m = cgi_glyphs.emplace_back(buffer_pool);
            m.area = {{ 0, block.coor.y }, { width, height }};
            m.type = sprite::alpha;
            m.bits.resize(m.area.length());
            auto raster = m.raster<byte>();
            while (block.coor.x < width)
            {
                for (auto x = 0; x < fract; x++) // Split the sine wave on four parts in order to keep absolute pixel symmetry.
                {
                    auto p = block;
                    p.coor.y = (si32)(y0 - std::sin(k * (x)) * vsize - 0.00001f); // -0.00001f: To move first pixel up (x=0).
                    netxs::onrect(raster, p, cell::shaders::full(c));
                    p.coor.x += fract;
                    p.coor.y = (si32)(y0 - std::sin(k * (fract - x)) * vsize - 0.00001f);
                    netxs::onrect(raster, p, cell::shaders::full(c));
                    p.coor.x += fract;
                    p.coor.y = (si32)(y0 + std::sin(k * (x)) * vsize + 0.00001f); // +0.00001f: To move first pixel down.
                    netxs::onrect(raster, p, cell::shaders::full(c));
                    p.coor.x += fract;
                    p.coor.y = (si32)(y0 + std::sin(k * (fract - x)) * vsize + 0.00001f);
                    netxs::onrect(raster, p, cell::shaders::full(c));
                    block.coor.x++;
                }
                block.coor.x += fract * 3;
            }
        }
        void reset()
        {
            glyphs.clear();
            cgi_glyphs.clear();
            mono_buffer.release();
            generate_glyphs();
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
            auto is_box_drawing = base_char >= 0x2320  && (base_char <= 0x23D0   // ⌠ ⌡ ... ⎛ ⎜ ⎝ ⎞ ⎟ ⎠ ⎡ ⎢ ⎣ ⎤ ⎥ ⎦ ⎧ ⎨ ⎩ ⎪ ⎫ ⎬ ⎭ ⎮ ⎯ ⎰ ⎱ ⎲ ⎳ ⎴ ⎵ ⎶ ⎷ ⎸ ⎹ ... ⏐
                              || (base_char >= 0x2500  && (base_char <  0x259F   // Box Elements
                              //|| (base_char >= 0x25A0  && (base_char <= 0x25FF   // Geometric Shapes
                              || (base_char >= 0xE0B0  && (base_char <= 0xE0B3   // Powerline Arrows
                              || (base_char >= 0x1CC00 && (base_char <= 0x1CEBF  // Legacy Computing Supplement. inc Large Type Pieces: U+1CE1A-1CE50
                              || (base_char >= 0x1F67C && (base_char <= 0x1F67F  // Ornamental Dingbats: U+1F67C-1F67F 🙼 🙽 🙾 🙿
                              || (base_char >= 0x1FB00 && (base_char <= 0x1FBFF))))))))))); // Symbols for Legacy Computing
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
            auto actual_width = swapxy ? std::max(1.f, length) :
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
        void draw_glyf(auto& canvas, sprite& glyph_mask, twod offset, argb fgc)
        {
            auto box = glyph_mask.area.shift(offset);
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
        void draw_cell(auto& canvas, auto& blinks, rect placeholder, cell const& c)
        {
            placeholder.trimby(canvas.area());
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
                    auto& wavy_raster = cgi_glyphs[synthetic::wavyunderline];
                    auto offset = placeholder.coor;
                    auto fract4 = wavy_raster.area.size.x - cellsz.x; // synthetic::wavyunderline has a bump at the beginning to synchronize the texture offset.
                    offset.x -= offset.x % fract4;
                    draw_glyf(target, wavy_raster, offset, color);
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
                if (c.jgc())
                {
                    iter = glyphs.emplace(token, mono_buffer).first;
                    rasterize(iter->second, c);
                }
                else return;
            }
            auto& glyph_mask = iter->second;
            if (!glyph_mask.area) return;

            auto [w, h, x, y] = c.whxy();
            if (x == 0 || y == 0) return;
            auto offset = placeholder.coor - twod{ cellsz.x * (x - 1), cellsz.y * (y - 1) };
            draw_glyf(target, glyph_mask, offset, fgc);
        }
        void draw_cell_with_cursor(auto& canvas, auto& blinks, rect placeholder, cell c)
        {
            //todo hilight grapheme cluster.
            //todo hint for ime
            auto style = c.cur();
            auto [bgcolor, fgcolor] = c.cursor_color();
            auto width = std::max(1, (si32)std::round(cellsz.x / 8.f));
            if (!bgcolor || bgcolor == argb::default_color) bgcolor = { c.bgc().luma() < 192 ? tint::purewhite : tint::pureblack };
            if (!fgcolor || fgcolor == argb::default_color) fgcolor = { bgcolor.luma() < 192 ? tint::purewhite : tint::pureblack };
            if (style == text_cursor::block)
            {
                c.fgc(fgcolor).bgc(bgcolor);
                draw_cell(canvas, blinks, placeholder, c);
            }
            else if (style == text_cursor::I_bar)
            {
                draw_cell(canvas, blinks, placeholder, c);
                placeholder.size.x = width;
                //todo draw glyph inside the cursor (respect blinking glyphs)
                //c.fgc(fgcolor).bgc(bgcolor);
                //draw_cell(canvas, blinks, placeholder, c);
                netxs::onrect(canvas, placeholder, cell::shaders::full(bgcolor));
            }
            else if (style == text_cursor::underline)
            {
                draw_cell(canvas, blinks, placeholder, c);
                placeholder.coor.y += cellsz.y - width;
                placeholder.size.y = width;
                c.fgc(fgcolor).bgc(bgcolor);
                //todo draw glyph inside the cursor (respect blinking glyphs)
                //draw_cell(canvas, blinks, placeholder, c);
                netxs::onrect(canvas, placeholder, cell::shaders::full(bgcolor));
            }
        }
        void fill_grid(auto& canvas, auto& blinks, si32& blink_count, twod origin, auto& grid_cells)
        {
            auto area = rect{ origin, cellsz };
            auto maxc = area.coor + grid_cells.size() * cellsz;
            auto base = canvas.coor();
            auto base_blinks = blinks.coor();
            blinks.move(origin);
            canvas.step(-base);
            for (cell& c : grid_cells)
            {
                if (c.blk()) blink_count++;//todo sync with blink_synch
                if (c.cur()) draw_cell_with_cursor(canvas, blinks, area, c);
                else         draw_cell(canvas, blinks, area, c);
                area.coor.x += cellsz.x;
                if (area.coor.x >= maxc.x)
                {
                    area.coor.x = origin.x;
                    area.coor.y += cellsz.y;
                    if (area.coor.y >= maxc.y) break;
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
        std::array<byte, 256> kbstate = {}; // manager: Global keyboard state.

        manager()
        {
            set_dpi_awareness();
        }
        ~manager()
        {
            for (auto& w : layers) w.reset();
        }

        auto get_window_title()
        {
            auto hWnd = layers.front().hWnd;
            auto size = ::GetWindowTextLengthW(hWnd);
            auto crop = wide(size, '\0');
            ::GetWindowTextW(hWnd, crop.data(), (si32)crop.size() + 1);
            return utf::to_utf(crop);
        }
        void set_window_title(view utf8)
        {
            ::SetWindowTextW(layers.front().hWnd, utf::to_utf(utf8).data());
        }
        void set_dpi_awareness()
        {
            auto proc = (LONG(_stdcall *)(si32))::GetProcAddress(::GetModuleHandleA("user32.dll"), "SetProcessDpiAwarenessInternal");
            if (proc)
            {
                proc(2/*PROCESS_PER_MONITOR_DPI_AWARE*/);
                //auto hr = proc(2/*PROCESS_PER_MONITOR_DPI_AWARE*/);
                //if (hr != S_OK || hr != E_ACCESSDENIED) log("%%Set DPI awareness failed %hr% %ec%", prompt::gui, utf::to_hex(hr), ::GetLastError());
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
                    lock = ::DeferWindowPos(lock, w.hWnd, 0, w.prev.coor.x, w.prev.coor.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
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
        void dispatch()//os::fire& alarm)
        {
            auto msg = MSG{};
            while (::GetMessageW(&msg, 0, 0, 0) > 0)
            {
                ::DispatchMessageW(&msg);
            }
            //auto stop = os::fd_t{ alarm };
            //auto next = MSG{};
            //while (next.message != WM_QUIT)
            //{
            //    if (auto yield = ::MsgWaitForMultipleObjects(1, &stop, FALSE, INFINITE, QS_ALLINPUT); yield == WAIT_OBJECT_0)
            //    {
            //        //manager::close();
            //        ::DestroyWindow(layers[client].hWnd);
            //        break;
            //    }
            //    while (::PeekMessageW(&next, NULL, 0, 0, PM_REMOVE) && next.message != WM_QUIT)
            //    {
            //        ::DispatchMessageW(&next);
            //    }
            //}
        }
        void print_kbstate(text s)
        {
            s += "\n"s;
            auto i = 0;
            for (auto k : kbstate)
            {
                     if (k == 0x80) s += ansi::fgc(tint::greenlt);
                else if (k == 0x01) s += ansi::fgc(tint::yellowlt);
                else if (k == 0x81) s += ansi::fgc(tint::cyanlt);
                else if (k)         s += ansi::fgc(tint::magentalt);
                else                s += ansi::nil();
                s += utf::to_hex(k) + ' ';
                i++;
                if (i % 16 == 0)s += '\n';
            }
            os::logstd(s);
        }
        void activate()
        {
            if (!layers.empty()) ::SetActiveWindow(layers.front().hWnd);
            ::GetKeyboardState(kbstate.data()); // Get some state (modifiers and locks are outdated).
            for (auto vkey : { VK_MENU,  VK_SHIFT,  VK_CONTROL,            // Get current modifiers state.
                               VK_LMENU, VK_LSHIFT, VK_LCONTROL, VK_LWIN,
                               VK_RMENU, VK_RSHIFT, VK_RCONTROL, VK_RWIN })
            {
                auto s = ::GetAsyncKeyState(vkey);
                kbstate[vkey] = (s >> 8) | (s & 0x1);
            }
            for (auto vkey : { VK_NUMLOCK, VK_CAPITAL, VK_SCROLL }) // Get current locks state.
            {
                auto s = ::GetKeyState(vkey);
                kbstate[vkey] = (s >> 8) | (s & 0x1);
            }
            //print_kbstate("::GetKeyboardState");
        }
        void deactivate()
        {
            //kbstate = {};
            for (auto vkey : { VK_MENU,  VK_SHIFT,  VK_CONTROL,           // Clear current mods state.
                               VK_LMENU, VK_LSHIFT, VK_LCONTROL, VK_LWIN,
                               VK_RMENU, VK_RSHIFT, VK_RCONTROL, VK_RWIN })
            {
                kbstate[vkey] = 0;
            }
            //print_kbstate("deactivate");
        }
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
        virtual void mouse_wheel(si32 delta, bool hzwheel) = 0;
        virtual void keybd_press(arch vkey, arch lParam) = 0;
        virtual void keybd_paste(arch wide_char) = 0;
        virtual void check_fsmode(arch hWnd) = 0;
        virtual void sync_clipboard() = 0;

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
                    case WM_MOUSEWHEEL:    w->mouse_wheel(hi(wParam), 0);              break;
                    case WM_MOUSEHWHEEL:   w->mouse_wheel(hi(wParam), 1);              break;
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
                    case WM_INPUTLANGCHANGE:
                    {
                        //todo sync kb layout
                        //auto hkl = ::GetKeyboardLayout(0);
                        auto kblayout = wide(KL_NAMELENGTH, '\0');
                        ::GetKeyboardLayoutNameW(kblayout.data());
                        log("%%Keyboard layout has changed to ", prompt::gui, utf::to_utf(kblayout));//, " lo(hkl),langid=", lo((arch)hkl), " hi(hkl),handle=", hi((arch)hkl));
                        break;
                    }
                    case WM_KEYDOWN:
                    case WM_KEYUP:
                    case WM_SYSKEYDOWN:  // WM_CHAR/WM_SYSCHAR and WM_DEADCHAR/WM_SYSDEADCHAR are derived messages after translation.
                    case WM_SYSKEYUP:      w->keybd_press(wParam, lParam);             break;
                    //case WM_UNICHAR: log("WM_UNICHAR"); w->keybd_paste(wParam, lParam); break;
                    //case WM_CHAR: log("WM_CHAR"); w->keybd_paste(wParam, lParam); break;
                    //case WM_SYSCHAR: log("WM_SYSCHAR");       w->keybd_paste(wParam, lParam);             break;
                    case WM_IME_CHAR:      w->keybd_paste(wParam);                     break;
                    case WM_WINDOWPOSCHANGED:
                    case WM_DPICHANGED:
                    case WM_DISPLAYCHANGE:
                    case WM_DEVICECHANGE:  w->check_fsmode((arch)hWnd);                break;
                    //dx3d specific
                    //case WM_PAINT:   /*w->check_dx3d_state();*/ stat = ::DefWindowProcW(hWnd, msg, wParam, lParam); break;
                    case WM_CREATE: ::AddClipboardFormatListener(hWnd); break;
                    case WM_CLIPBOARDUPDATE: w->sync_clipboard(); break;
                    case WM_DESTROY: ::RemoveClipboardFormatListener(hWnd);
                                     ::PostQuitMessage(0);
                                     //if (alive.exchange(faux))
                                     //{
                                     //    os::signals::place(os::signals::close); // taskkill /pid nnn
                                     //}
                                     break;
                    //case WM_ENDSESSION:
                    //    if (wParam && alive.exchange(faux))
                    //    {
                    //             if (lParam & ENDSESSION_CLOSEAPP) os::signals::place(os::signals::close);
                    //        else if (lParam & ENDSESSION_LOGOFF)   os::signals::place(os::signals::logoff);
                    //        else                                   os::signals::place(os::signals::shutdown);
                    //    }
                    //    break;
                    default:
                    //log("\tmsW=", utf::to_hex(msg), " wP=", utf::to_hex(wParam), " lP=", utf::to_hex(lParam), " hwnd=", utf::to_hex(hWnd));
                    stat = ::DefWindowProcW(hWnd, msg, wParam, lParam); break;
                }
                w->sys_command(syscmd::update);
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

        auto get_window_title()
        {
            //...
            return ""s;
        }
        void set_window_title(view /*utf8*/)
        {
            //...
        }
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
        void dispatch()//os::fire& /*alarm*/)
        {
            //...
        }
        void activate()
        {
            //...
        }
        void deactivate()
        {
            //...
        }
    };
}

#endif

namespace netxs::gui
{
    struct window : manager, ui::base
    {
        using e2 = netxs::events::userland::e2;
        using gray = netxs::raster<std::vector<byte>, rect>;
        using shad = netxs::misc::shadow<gray>;
        using grip = netxs::misc::szgrips;
        using s11n = netxs::directvt::binary::s11n;

        ui::pro::title titles; // window: .
        ui::pro::focus wfocus; // window: .

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
        struct evnt : s11n
        {
            window&         owner; // evnt: .
            ui::pipe&       intio; // evnt: .
            flag            alive; // evnt: .

            //todo use gear.m_sys
            input::sysmouse m = {}; // evnt: .
            input::syskeybd k = {}; // evnt: .
            input::sysfocus f = {}; // evnt: .
            input::syspaste p = {}; // evnt: .
            input::syswinsz w = {}; // evnt: .
            input::sysclose c = {}; // evnt: .
            netxs::sptr<input::hids> gears; // evnt: .

            auto keybd(auto& data) { if (alive)                s11n::syskeybd.send(intio, data); }
            auto mouse(auto& data) { if (alive)                s11n::sysmouse.send(intio, data); }
            auto winsz(auto& data) { if (alive)                s11n::syswinsz.send(intio, data); }
            auto paste(auto& data) { if (alive)                s11n::syspaste.send(intio, data); }
            auto close(auto& data) { if (alive.exchange(faux)) s11n::sysclose.send(intio, data); }
            void direct(s11n::xs::bitmap_dtvt lock, view& data)
            {
                auto& bitmap = lock.thing;
                auto resize = [&](auto new_gridsz)
                {
                    if (owner.waitsz == new_gridsz) owner.waitsz = dot_00;
                };
                auto update = [&](auto size, auto head, auto iter, [[maybe_unused]] auto tail)
                {
                    auto offset = (si32)(iter - head);
                    auto mx = std::max(1, size.x);
                    auto coor = twod{ offset % mx, offset / mx };
                    //todo render in pixels
                    //see gcache.fill_grid()
                    //owner.print(size, coor, iter, tail);
                    //auto dist = tail - head;
                    //auto dest = main_grid.begin() + coor.x + coor.y * area.x;
                    //todo render in pixels
                    //todo now
                    //while (head != tail)
                    //{
                    //    auto src = *head++;
                    //    auto& dst = *dest++;
                    //    dst = src;
                    //}
                    //auto& client_layer = layers[client];
                    //auto& blinky_layer = layers[blinky];
                    //client_layers.strike(dirty_area);
                    //blinky_layers.strike({ .size = layers[blinky].area.size });
                };
                bitmap.get(data, update, resize);
                s11n::request_jgc(intio, lock);
                owner.reload |= task::inner;
            }
            void handle(s11n::xs::jgc_list         lock)
            {
                s11n::receive_jgc(lock);
                //todo repaint jgc cells
                owner.reload |= task::inner; // Trigger to redraw viewport to update jumbo clusters.
            }
            void handle(s11n::xs::header_request   lock)
            {
                auto& item = lock.thing;
                owner.RISEUP(tier::request, e2::form::prop::ui::header, header_utf8, ());
                s11n::header.send(intio, item.window_id, header_utf8);
            }
            void handle(s11n::xs::footer_request   lock)
            {
                auto& item = lock.thing;
                owner.RISEUP(tier::request, e2::form::prop::ui::footer, footer_utf8, ());
                s11n::footer.send(intio, item.window_id, footer_utf8);
            }
            void handle(s11n::xs::header           lock)
            {
                auto& item = lock.thing;
                owner.RISEUP(tier::preview, e2::form::prop::ui::header, item.utf8);
            }
            void handle(s11n::xs::footer           lock)
            {
                auto& item = lock.thing;
                owner.RISEUP(tier::preview, e2::form::prop::ui::footer, item.utf8);
            }
            void handle(s11n::xs::clipdata         lock)
            {
                auto& item = lock.thing;
                if (item.form == mime::disabled) input::board::normalize(item);
                else                             item.set();
                os::clipboard::set(item);
                auto crop = utf::trunc(item.utf8, owner.gridsz.y / 2); // Trim preview before sending.
                s11n::sysboard.send(intio, id_t{}, item.size, crop.str(), item.form);
            }
            void handle(s11n::xs::clipdata_request lock)
            {
                s11n::recycle_cliprequest(intio, lock);
            }
            void handle(s11n::xs::tooltips         lock)
            {
                auto copy = lock.thing;
                //todo implement
                //owner.bell::enqueue(owner.This(), [tooltips = std::move(copy)](auto& boss) mutable
                //{
                //    for (auto& tooltip : tooltips)
                //    {
                //        if (auto gear_ptr = boss.bell::getref<hids>(tooltip.gear_id))
                //        {
                //            gear_ptr->set_tooltip(tooltip.tip_text, tooltip.update);
                //        }
                //    }
                //});
            }
            void handle(s11n::xs::fullscreen       lock)
            {
                if (owner.fsmode == state::maximized) owner.set_state(state::normal);
                else                                  owner.set_state(state::maximized);
            }
            void handle(s11n::xs::maximize         lock)
            {
                //todo diff fullscreen and maximized
                if (owner.fsmode == state::maximized) owner.set_state(state::normal);
                else                                  owner.set_state(state::maximized);
            }
            void handle(s11n::xs::minimize         lock)
            {
                if (owner.fsmode == state::minimized) owner.set_state(state::normal);
                else                                  owner.set_state(state::minimized);
            }
            void handle(s11n::xs::expose         /*lock*/)
            {
                owner.bell::enqueue(owner.This(), [&](auto& /*boss*/)
                {
                    owner.RISEUP(tier::preview, e2::form::layout::expose, area, ());
                });
            }
            void handle(s11n::xs::focus_cut        lock)
            {
                auto& item = lock.thing;
                // We are the focus tree endpoint. Signal back the focus set up.
                owner.SIGNAL(tier::release, hids::events::keybd::focus::bus::off, seed, ({ .id = item.gear_id }));

            }
            void handle(s11n::xs::focus_set        lock)
            {
                auto& item = lock.thing;
                // We are the focus tree endpoint. Signal back the focus set up.
                owner.SIGNAL(tier::release, hids::events::keybd::focus::bus::on, seed, ({ .id = item.gear_id, .solo = item.solo, .item = owner.This() }));
            }
            void handle(s11n::xs::keybd_event      lock)
            {
                auto& gear = *gears;
                auto& keybd = lock.thing;
                gear.alive    = true;
                gear.ctlstate = keybd.ctlstat;
                gear.extflag  = keybd.extflag;
                gear.virtcod  = keybd.virtcod;
                gear.scancod  = keybd.scancod;
                gear.pressed  = keybd.pressed;
                gear.cluster  = keybd.cluster;
                gear.handled  = keybd.handled;
                owner.SIGNAL(tier::release, hids::events::keybd::key::post, gear);
            };
            void handle(s11n::xs::mouse_event      lock)
            {
                auto& gear = *gears;
                auto& mouse = lock.thing;
                auto basis = gear.owner.base::coor();
                owner.global(basis);
                gear.replay(mouse.cause, mouse.coord - basis, mouse.delta, mouse.buttons, mouse.ctlstat, mouse.whldt, mouse.hzwhl);
                gear.pass<tier::release>(owner.This(), gear.owner.base::coor(), true);
            }
            void handle(s11n::xs::warping          lock)
            {
                auto& warp = lock.thing;
                owner.warp_window(warp.warpdata);
            }
            void handle(s11n::xs::fps              lock)
            {
                //todo revise
                //owner.bell::enqueue(owner.This(), [&, fps = lock.thing.frame_rate](auto& /*boss*/) mutable
                //{
                //    owner.SIGNAL(tier::general, e2::config::fps, fps);
                //});
            }
            void handle(s11n::xs::logs             lock)
            {
                s11n::recycle_log(lock, os::process::id.second);
            }
            void handle(s11n::xs::fatal            lock)
            {
                //todo revise
                //owner.bell::enqueue(owner.This(), [&, utf8 = lock.thing.err_msg](auto& /*boss*/)
                //{
                //    owner.errmsg = owner.genmsg(utf8);
                //    owner.deface();
                //});
            }
            void handle(s11n::xs::sysclose       /*lock*/)
            {
                //todo revise
                owner.manager::close();
                //owner.active.exchange(faux);
                //owner.stop(true);
            }
            void handle(s11n::xs::sysstart       /*lock*/)
            {
                //todo revise
                //owner.bell::enqueue(owner.This(), [&](auto& /*boss*/)
                //{
                //    owner.RISEUP(tier::release, e2::form::global::sysstart, 1);
                //});
            }
            void handle(s11n::xs::cwd              lock)
            {
                //todo revise
                //owner.bell::enqueue(owner.This(), [&, path = lock.thing.path](auto& /*boss*/)
                //{
                //    owner.RISEUP(tier::preview, e2::form::prop::cwd, path);
                //});
            }
            struct
            {
                span tooltip_timeout;
                si32 clip_preview_glow;
                cell clip_preview_clrs;
                byte clip_preview_alfa;
                span dblclick_timeout;
            }
            props{};

            evnt(window& owner, ui::pipe& intio)
                : s11n{ *this },
                 owner{ owner },
                 intio{ intio },
                 alive{ true },
                 gears{ owner.bell::create<hids>(props, owner, s11n::bitmap_dtvt.freeze().thing.image) }
            {
                auto& gear = *gears;
                m.gear_id = gear.id;
                k.gear_id = gear.id;
                f.gear_id = gear.id;
                p.gear_id = gear.id;
                w.gear_id = gear.id;
                m.enabled = input::hids::stat::ok;
                m.coordxy = { si16min, si16min };
                c.fast = true;
                w.winsize = owner.gridsz;
            }
        };

        std::vector<byte> blink_synch;
        si32              blink_count{};
        ui::face head_grid;
        ui::face foot_grid;

        font fcache; // window: Font cache.
        glyf gcache; // window: Glyph cache.
        fp32 height; // window: Cell height in fp32 pixels.
        twod gripsz; // window: Resizing grips size in pixels.
        twod gridsz; // window: Window grid size in cells.
        dent border; // window: Border around window for resizing grips (dent in pixels).
        shad shadow; // window: Shadow generator.
        grip szgrip; // window: Resizing grips UI-control.
        twod mcoord; // window: Mouse cursor coord.
        twod waitsz; // window: Window is waiting resize acknowledge.
        bool inside; // window: Mouse is inside the client area.
        bool seized; // window: Mouse is locked inside the client area.
        bool mhover; // window: Mouse hover.
        bool active; // window: Window is focused.
        bool moving; // window: Window is in d_n_d state.
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
        evnt proxy; // window: .
        text toUTF8;
        utfx point = {}; // window: Surrogate pair buffer.
        si32 kbmod = {};

        static constexpr auto shadow_dent = dent{ 1,1,1,1 } * 3;

        window(auth& indexer, rect win_coor_px_size_cell, std::list<text>& font_names, si32 cell_height, bool antialiasing, span blinkrate, twod grip_cell = dot_21)
            : base{ indexer },
              fcache{ font_names, cell_height },
              gcache{ fcache, antialiasing },
              height{ (fp32)fcache.cellsize.y },
              gripsz{ grip_cell * fcache.cellsize },
              gridsz{ std::max(dot_11, win_coor_px_size_cell.size) },
              border{ gripsz.x, gripsz.x, gripsz.y, gripsz.y },
              shadow{ 0.44f/*bias*/, 116.5f/*alfa*/, gripsz.x, dot_00, dot_11, cell::shaders::full },
              inside{},
              seized{},
              mhover{},
              active{},
              moving{},
              fsmode{ state::undefined },
              reload{ task::all },
              client{ manager::add(this) },
              blinky{ manager::add() },
              header{ manager::add() },
              footer{ manager::add() },
              blinkrate{ manager::client_animation() ? blinkrate : span::zero() },
              blinking{ faux },
              proxy{ *this, *os::dtvt::client },
              titles{ *this, "", "", faux },
              wfocus{ *this }
        {
            if (!*this) return;
            normsz = rect{ win_coor_px_size_cell.coor, gridsz * cellsz } + border;
            layers[client].area = normsz;
            size_window();
        }
        void sync_header_pixel_layout()
        {
            auto base_rect = layers[blinky].area;
            auto header_height = head_grid.size().y * cellsz.y;
            layers[header].area = base_rect + dent{ 0, 0, header_height, -base_rect.size.y } + shadow_dent;
            layers[header].area.coor.y -= shadow_dent.b;
        }
        void sync_footer_pixel_layout()
        {
            auto base_rect = layers[blinky].area;
            auto footer_height = foot_grid.size().y * cellsz.y;
            layers[footer].area = base_rect + dent{ 0, 0, -base_rect.size.y, footer_height } + shadow_dent;
            layers[footer].area.coor.y += shadow_dent.t;
        }
        void sync_titles_pixel_layout()
        {
            auto base_rect = layers[client].area;
            grip_l = rect{{ 0                          , gripsz.y }, { gripsz.x, base_rect.size.y - gripsz.y * 2}};
            grip_r = rect{{ base_rect.size.x - gripsz.x, gripsz.y }, grip_l.size };
            grip_t = rect{{ 0, 0                                  }, { base_rect.size.x, gripsz.y }};
            grip_b = rect{{ 0, base_rect.size.y - gripsz.y        }, grip_t.size };
            sync_header_pixel_layout();
            sync_footer_pixel_layout();
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
        void change_cell_size(fp32 dy = {}, twod resize_center = {})
        {
            reset_blinky();
            auto grip_cell = gripsz / cellsz;
            height = std::clamp(height + dy, 2.f, 256.f);
            auto prev_cellsz = cellsz;
            fcache.set_cellsz((si32)height);
            gcache.reset();
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
                auto new_size = gridsz * cellsz + border;
                auto old_size = layers[client].area.size;
                auto xy_delta = resize_center - resize_center * new_size / std::max(dot_11, old_size);
                layers[client].area.coor += xy_delta;
                layers[client].area.size = new_size;
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
        void update_header()
        {
            size_title(head_grid, titles.head_page);
            sync_titles_pixel_layout();
            reload |= task::header;
        }
        void update_footer()
        {
            size_title(foot_grid, titles.foot_page);
            sync_titles_pixel_layout();
            reload |= task::footer;
        }
        void set_font_list(auto& flist)
        {
            log("%%Font list changed: ", prompt::gui, flist);
            fcache.set_fonts(flist, faux);
            change_cell_size();
            reload |= task::all;
        }
        auto move_window(twod delta)
        {
            for (auto& w : layers) w.area.coor += delta;
            reload |= task::moved;
        }
        void drop_grips()
        {
            if (szgrip.seized) // drag stop
            {
                szgrip.drop();
                reload |= task::grips;
            }
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
                border = { gripsz.x, gripsz.x, gripsz.y, gripsz.y };
                layers[client].area = normsz;
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
                drop_grips();
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
        }
        void check_fsmode(arch hWnd)
        {
            if ((arch)(layers[client].hWnd) != hWnd) return;
            if (fsmode == state::undefined || layers.empty()) return;
            auto unsync = true;
            if (auto lock = bell::try_sync()) // Try to sync with ui thread for fast checking.
            {
                if (fsmode == state::maximized)
                {
                    auto fs_area = manager::get_fs_area(layers[client].area);
                    unsync = fs_area != layers[client].area;
                }
                else if (fsmode == state::normal)
                {
                    auto avail_area = manager::get_fs_area(rect{ -dot_mx / 2, dot_mx });
                    unsync = !avail_area.trim(layers[client].area);
                }
                else unsync = faux;
            }
            if (unsync) bell::enqueue(This(), [&](auto& /*boss*/) // Perform corrections.
            {
                if (fsmode == state::maximized)
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
                        move_window(delta);
                        sync_titles_pixel_layout(); // Align grips and shadow.
                    }
                }
                if (fsmode != state::minimized)
                {
                    for (auto& w : layers) w.prev.coor = dot_mx; // Windows moves our windows the way it wants, breaking the layout.
                    reload |= task::moved;
                }
                update();
            });
        }
        void size_title(ui::face& title_grid, ui::page& title_page)
        {
            auto grid_size = gridsz;
            title_grid.calc_page_height(title_page, grid_size);
            title_grid.size(grid_size);
            title_grid.wipe();
            title_grid.cup(dot_00);
            title_grid.output(title_page, cell::shaders::contrast);
        }
        void size_window(twod size_delta = {})
        {
            layers[client].area.size += size_delta;
            layers[blinky].area = layers[client].area - border;
            gridsz = layers[blinky].area.size / cellsz;
            blink_synch.assign(gridsz.x * gridsz.y, 0);
            if (fsmode != state::maximized)
            {
                size_title(head_grid, titles.head_page);
                size_title(foot_grid, titles.foot_page);
                sync_titles_pixel_layout();
            }
            reload |= task::sized;
            if (proxy.w.winsize != gridsz)
            {
                waitsz = gridsz;
                proxy.w.winsize = gridsz;
                proxy.winsz(proxy.w); // And wait for reply to resize and redraw.
            }
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
        dent warp_window(dent warp_delta)
        {
            auto old_client = layers[blinky].area;
            auto new_client = old_client + warp_delta;
            auto new_gridsz = std::max(dot_11, new_client.size / cellsz);
            if (gridsz != new_gridsz)
            {
                auto size_delta = new_gridsz * cellsz - old_client.size;
                auto coor_delta = new_client.coor - old_client.coor;
                size_window(size_delta);
                move_window(coor_delta);
            }
            return layers[client].area - old_client;
        }
        bool hit_grips()
        {
            if (fsmode == state::maximized || szgrip.zoomon) return faux;
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
                //todo push diffs only
                layer.sync.push_back(g_area);
            }
        }
        void draw_title(si32 index, auto& facedata) //todo just output ui::core
        {
            auto canvas = layers[index].canvas(true);
            auto blinks = layers[blinky].canvas(); //todo unify blinks in titles
            auto title_blink_count = 0;
            gcache.fill_grid(canvas, blinks, title_blink_count, shadow_dent.corner(), facedata);
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
            if (!reload || waitsz) return;
            auto what = reload;
            reload = {};
                 if (what == task::moved) manager::present<true>();
            else if (what)
            {
                if (what & (task::sized | task::inner))
                {
                    auto canvas = layers[client].canvas();
                    auto blinks = layers[blinky].canvas(true);
                    canvas.move(dot_00);
                    auto dirty_area = canvas.area() - border;
                    blink_count = 0;
                    {
                        //todo update while sync
                        auto bitmap_lock = proxy.bitmap_dtvt.freeze();
                        auto& main_grid = bitmap_lock.thing.image;
                        gcache.fill_grid(canvas, blinks, blink_count, dirty_area.coor, main_grid);
                    }
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
        void mouse_wheel(si32 delta, bool hz)
        {
            if (delta == 0) return;
            auto wheeldt = delta / 120.f;
            if (inside)
            {
                proxy.m.changed++;
                proxy.m.timecod = datetime::now();
                proxy.m.ctlstat = proxy.k.ctlstat;
                proxy.m.hzwheel = hz;
                proxy.m.wheeldt = wheeldt;
                proxy.mouse(proxy.m);
                proxy.m.hzwheel = {};
                proxy.m.wheeldt = {};
            }
            else
            {
                bell::enqueue(This(), [&, wheeldt](auto& /*boss*/)
                {
                    //if (gear.meta(hids::anyCtrl))
                    {
                        change_cell_size(wheeldt, mcoord - layers[client].area.coor);
                        reload |= task::all;
                        update();
                    }
                });
            }
        }
        void send_mouse_halt()
        {
            proxy.m.changed++;
            proxy.m.timecod = datetime::now();
            proxy.m.enabled = hids::stat::halt;
            proxy.mouse(proxy.m);
            proxy.m.enabled = hids::stat::ok;
        }
        void mouse_leave()
        {
            mhover = faux;
            if (szgrip.leave()) reload |= task::grips;
            send_mouse_halt();
        }
        void resize_by_grips(twod coord)
        {
            auto inner_rect = layers[blinky].area;
            auto zoom = proxy.k.ctlstat & hids::anyCtrl;
            auto [preview_area, size_delta] = szgrip.drag(inner_rect, coord, border, zoom, cellsz);
            auto old_client = layers[blinky].area;
            auto new_gridsz = std::max(dot_11, (old_client.size + size_delta) / cellsz);
            size_delta = new_gridsz * cellsz - old_client.size;
            if (size_delta)
            {
                //todo sync ui
                if (auto move_delta = szgrip.move(size_delta, zoom))
                {
                    move_window(move_delta);
                    sync_titles_pixel_layout(); // Align grips and shadow.
                }
                resize_window(size_delta);
            }
        }
        void mouse_moved(twod coord)
        {
            auto& mbttns = proxy.m.buttons;
            mhover = true;
            auto inner_rect = layers[blinky].area;
            auto ingrip = hit_grips();
            if (moving && mbttns != bttn::left && mbttns != bttn::right) // Do not allow to move window with multiple buttons pressed.
            {
                moving = faux;
            }
            if ((!seized && ingrip) || szgrip.seized)
            {
                if (mbttns == bttn::right && fsmode == state::normal) // Move window.
                {
                    moving = true;
                }
                else if (mbttns == bttn::left)
                {
                    if (!szgrip.seized) // drag start
                    {
                        szgrip.grab(inner_rect, mcoord, border, cellsz);
                    }
                    bell::enqueue(This(), [&, coord](auto& /*boss*/)
                    {
                        resize_by_grips(coord);
                    });
                }
                else drop_grips();
            }
            if (!moving && !seized && szgrip.calc(inner_rect, coord, border, dent{}, cellsz))
            {
                reload |= task::grips;
            }
            if (moving && fsmode == state::normal)
            {
                if (auto dxdy = coord - mcoord)
                {
                    mcoord = coord;
                    bell::enqueue(This(), [&, dxdy](auto& /*boss*/)
                    {
                        //todo revise
                        //if (fsmode == state::maximized) set_state(state::normal);
                        move_window(dxdy);
                        sync_titles_pixel_layout(); // Align grips and shadow.
                        update();
                    });
                }
                return;
            }
            mcoord = coord;
            auto new_state = !szgrip.seized && (seized || (border.t ? inner_rect : inner_rect - dent{ 0,0,1,0 }).hittest(mcoord)); // Allow 1px border at the top of the maximized window.
            auto leave = std::exchange(inside, new_state) != inside;
            auto coordxy = fp2d{ mcoord - inner_rect.coor } / cellsz;
            auto changed = proxy.m.coordxy(coordxy);
            if (inside)
            {
                if (changed)
                {
                    auto timecode = datetime::now();
                    proxy.m.changed++;
                    proxy.m.timecod = timecode;
                    proxy.mouse(proxy.m);
                }
            }
            else if (leave) // Mouse leaves viewport.
            {
                send_mouse_halt();
            }
            if (!mbttns && (std::exchange(ingrip, hit_grips()) != ingrip || ingrip)) // Redraw grips when hover state changes.
            {
                reload |= task::grips;
            }
        }
        void mouse_press(si32 button, bool pressed)
        {
            auto& mbttns = proxy.m.buttons;
            auto prev_state = mbttns;
            auto changed = std::exchange(mbttns, pressed ? mbttns | button : mbttns & ~button) != mbttns;
            if (pressed)
            {
                if (!prev_state)
                {
                    mouse_capture();
                    if (inside) seized = true;
                }
            }
            else if (!mbttns)
            {
                mouse_release();
                seized = faux;
            }
            if (pressed)
            {
                if (moving && prev_state) // Stop GUI window dragging if any addition mouse button pressed.
                {
                    moving = faux;
                }
            }
            else
            {
                if (button == bttn::left) drop_grips();
                moving = faux; // Stop GUI window dragging if any button released.
            }
            static auto dblclick = datetime::now() - 1s;
            if (changed && (seized || inside))
            {
                auto timecode = datetime::now();
                proxy.m.changed++;
                proxy.m.timecod = timecode;
                proxy.mouse(proxy.m);
            }
            else
            {
                if (!pressed && button == bttn::left) // Maximize window by dbl click on resizing grips.
                {
                    if (datetime::now() - dblclick < 500ms)
                    {
                        if (fsmode != state::minimized) set_state(fsmode == state::maximized ? state::normal : state::maximized);
                        dblclick -= 1s;
                    }
                    else
                    {
                        dblclick = datetime::now();
                    }
                }
            }
        }
        void sync_kbstat(view cluster = {}, bool pressed = {}, si32 virtcod = {}, si32 scancod = {}, bool extflag = {})
        {
            auto cs = 0;
            if (extflag) cs |= ENHANCED_KEY;
            if (kbstate[VK_RMENU   ] & 0x80) cs |= RIGHT_ALT_PRESSED;
            if (kbstate[VK_LMENU   ] & 0x80) cs |= LEFT_ALT_PRESSED;
            if (kbstate[VK_RCONTROL] & 0x80) cs |= RIGHT_CTRL_PRESSED;
            if (kbstate[VK_LCONTROL] & 0x80) cs |= LEFT_CTRL_PRESSED;
            if (kbstate[VK_SHIFT   ] & 0x80) cs |= SHIFT_PRESSED;
            if (kbstate[VK_NUMLOCK ] & 0x01) cs |= NUMLOCK_ON;
            if (kbstate[VK_CAPITAL ] & 0x01) cs |= CAPSLOCK_ON;
            if (kbstate[VK_SCROLL  ] & 0x01) cs |= SCROLLLOCK_ON;
            auto modstat = os::nt::modstat(kbmod, cs, scancod, pressed);
            if (modstat.repeats) return; // We don't repeat modifiers.
            else
            {
                if (modstat.changed || proxy.k.ctlstat != kbmod)
                {
                    proxy.k.ctlstat = kbmod;
                    proxy.m.ctlstat = kbmod;
                    proxy.m.timecod = datetime::now();
                    proxy.m.changed++;
                    proxy.mouse(proxy.m); // Fire mouse event to update kb modifiers.
                }
                proxy.k.extflag = extflag;
                proxy.k.virtcod = virtcod;
                proxy.k.scancod = scancod;
                proxy.k.pressed = pressed;
                proxy.k.keycode = input::key::xlat(virtcod, scancod, cs);
                proxy.k.cluster = cluster;
                proxy.keybd(proxy.k);
            }
        }
        void keybd_paste(arch wide_char)
        {
            if (utf::to_code((wchr)wide_char, point))
            {
                auto pressed = true;
                toUTF8.clear();
                utf::to_utf_from_code(point, toUTF8);
                sync_kbstat(toUTF8, pressed);
                point = {};
            }
        }
        void keybd_press(arch vkey, arch lParam)
        {
            auto virtcod = std::clamp((si32)vkey, 0, 255);
            if (virtcod || lParam)
            {
                //...
            }
            #if defined(_WIN32)

            union key_state_t
            {
                ui32 token;
                struct
                {
                    ui32 repeat   : 16;// 0-15
                    si32 scancode : 9; // 16-24 (24 - extended)
                    ui32 reserved : 4; // 25-28 (reserved)
                    ui32 context  : 1; // 29 (29 - context)
                    ui32 state    : 2; // 30-31: 0 - pressed, 1 - repeated, 2 - unknown, 3 - released
                } v;
            };
            auto param = key_state_t{ .token = (ui32)lParam };
            if (param.v.state == 2/*unknown*/) return;
            auto pressed = param.v.state == 0;
            auto repeat = param.v.state == 1;
            auto extflag = !!(param.v.scancode >> 9);
            auto scancod = param.v.scancode;
            if (pressed || repeat)
            {
                if (pressed)
                {
                         if (virtcod == VK_CAPITAL) kbstate[virtcod] ^= 0x01;
                    else if (virtcod == VK_NUMLOCK) kbstate[virtcod] ^= 0x01;
                    else if (virtcod == VK_SCROLL)  kbstate[virtcod] ^= 0x01;
                }
                kbstate[virtcod] |= 0x80;
                     if (virtcod == VK_CONTROL) kbstate[extflag ?         VK_RCONTROL : VK_LCONTROL] |= 0x80;
                else if (virtcod == VK_MENU)    kbstate[extflag ?         VK_RMENU    : VK_LMENU   ] |= 0x80;
                else if (virtcod == VK_SHIFT)   kbstate[scancod != 0x2a ? VK_RSHIFT   : VK_LSHIFT  ] |= 0x80;
            }
            else
            {
                kbstate[virtcod] &= ~0x80;
                     if (virtcod == VK_CONTROL) kbstate[extflag ?         VK_RCONTROL : VK_LCONTROL] &= ~0x80;
                else if (virtcod == VK_MENU)    kbstate[extflag ?         VK_RMENU    : VK_LMENU   ] &= ~0x80;
                else if (virtcod == VK_SHIFT)   kbstate[scancod != 0x2a ? VK_RSHIFT   : VK_LSHIFT  ] &= ~0x80;
            }
            auto to_WIDE = std::array<wchr, 32>{};
            auto len = ::ToUnicodeEx(virtcod,              // UINT wVirtKey,
                                     scancod,              // UINT wScanCode,
                                     kbstate.data(),       // const BYTE *lpKeyState,
                                     to_WIDE.data(),       // [out] LPWSTR pwszBuff,
                                     (si32)to_WIDE.size(), // [in]  int    cchBuff,
                                     1,                    // [in]  UINT   wFlags, - Do not process Alt+Numpad
                                     0);                   // [in, optional] HKL dwhkl
            //print_kbstate("key press:");
            if (len >= 0)
            {
                toUTF8.clear();
                if (len > 0) utf::to_utf(to_WIDE.data(), len, toUTF8);
                sync_kbstat(toUTF8, pressed || repeat, virtcod, scancod, extflag);
            }
            //if (virtcod == 'A' && param.v.state == 3) // Toggle AA mode.
            //{
            //    set_aa_mode(!gcache.aamode);
            //}
            if (pressed)
            {
                if (kbstate[VK_MENU] & 0x80 && kbstate[VK_RETURN] & 0x80) // Toggle maximized mode by Alt+Enter.
                {
                    bell::enqueue(This(), [&](auto& /*boss*/)
                    {
                        if (fsmode != state::minimized) set_state(fsmode == state::maximized ? state::normal : state::maximized);
                    });
                }
                else if (kbstate[VK_HOME] & 0x80 && kbstate[VK_END] & 0x80) // Shutdown by LeftArrow+RightArrow.
                {
                    bell::enqueue(This(), [&](auto& /*boss*/)
                    {
                        manager::close();
                    });
                }
            }
            //else if (param.v.state == 3 && fcache.families.size()) // Renumerate font list.
            //{
            //    auto flen = fcache.families.size();
            //    auto index = virtcod == 0x30 ? fcache.families.size() - 1 : virtcod - 0x30;
            //    if (index > 0 && index < flen)
            //    {
            //        auto& flist = fcache.families;
            //        auto iter = flist.begin();
            //        std::advance(iter, index);
            //        flist.splice(flist.begin(), flist, iter, std::next(iter)); // Move it to the begining of the list.
            //        set_font_list(flist);
            //        print_font_list(true);
            //    }
            //}
            #endif
        }
        void focus_event(bool focused)
        {
            active = focused;
            if (active) activate(); // It must be called in current thread.
            else        deactivate();
            bell::enqueue(This(), [&](auto& /*boss*/)
            {
                if (active)
                {
                    SIGNAL(tier::release, hids::events::keybd::focus::bus::on, seed, ({ .id = proxy.gears->id, .solo = (si32)ui::pro::focus::solo::on, .item = This() }));
                    sync_kbstat();
                }
                else
                {
                    sync_kbstat();
                    SIGNAL(tier::release, hids::events::keybd::focus::bus::off, seed, ({ .id = proxy.gears->id }));
                }
            });
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
            bell::enqueue(This(), [&, eventid](auto& /*boss*/)
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
                update();
            });
        }
        void sys_command(si32 menucmd)
        {
            if (menucmd == syscmd::update && !reload) return;
            bell::enqueue(This(), [&, menucmd](auto& /*boss*/)
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
                    case syscmd::close:  manager::close(); break;
                    case syscmd::update: update(); break;
                }
            });
        }
        void sync_clipboard()
        {
            os::clipboard::sync(layers[client].hWnd, proxy, proxy.intio, gridsz);
        }
        void connect(si32 win_state)
        {
            {
                auto lock = bell::sync();
                set_state(win_state);
                sync_clipboard();
                update();
                manager::run();//todo call run() only after receiving the first frame

                LISTEN(tier::preview, e2::form::layout::expose, area)
                {
                    //todo
                };
                auto accum_ptr = ptr::shared(dot_00);
                LISTEN(tier::release, hids::events::mouse::button::drag::start::any, gear, -, (accum_ptr))
                {
                    *accum_ptr = {};
                };
                LISTEN(tier::release, hids::events::mouse::button::drag::pull::left, gear, -, (accum_ptr)) // Move window only when mouse events get back.
                {
                    if (fsmode == state::normal) //todo revise this restriction
                    if (proxy.m.buttons == bttn::left || proxy.m.buttons == bttn::right) // Allow to move with one button pressed.
                    if (auto dxdy = twod{ std::round(gear.delta.get() * cellsz) }) // Return back to the pixels.
                    {
                        //todo revise
                        //auto& accum = *accum_ptr;
                        //accum += dxdy;
                        //log("accum=", accum);
                        //auto threshold = 2 * cellsz.y;
                        //if (std::abs(accum.x) > threshold || std::abs(accum.y) > threshold)
                        //{
                        //    log("\tgo");
                        //    dxdy = accum;
                        //    accum = {};
                        //}
                        //else return;
                        send_mouse_halt();
                        //todo revise
                        //if (fsmode == state::maximized)
                        //{
                        //    auto cur_cursor_coor = mcoord - layers[blinky].area.coor;
                        //    auto cur_blinky_size = normsz.size - gripsz * 2;
                        //    auto new_blinky_coor = mcoord - cur_cursor_coor.clampby(cur_blinky_size) + dxdy;
                        //    normsz.coor = new_blinky_coor - gripsz;
                        //    set_state(state::normal);
                        //}
                        //else
                        {
                            move_window(dxdy);
                        }
                        sync_titles_pixel_layout(); // Align grips and shadow.
                        moving = true;
                    }
                };
                LISTEN(tier::release, hids::events::mouse::button::dblclick::left, gear)
                {
                         if (fsmode == state::maximized) set_state(state::normal);
                    else if (fsmode == state::normal)    set_state(state::maximized);
                };
                LISTEN(tier::release, hids::events::mouse::scroll::any, gear)
                {
                    //todo uncomment
                    //if (gear.meta(hids::anyCtrl))
                    {
                        change_cell_size(gear.whldt, mcoord - layers[client].area.coor);
                        reload |= task::all;
                    }
                };
                LISTEN(tier::release, hids::events::keybd::focus::bus::any, seed)
                {
                    auto deed = this->bell::template protos<tier::release>();
                    if (seed.guid == decltype(seed.guid){}) // To avoid focus tree infinite looping.
                    {
                        seed.guid = os::process::id.second;
                    }
                    proxy.focusbus.send(proxy.intio, seed.id, seed.guid, netxs::events::subindex(deed));
                };
                LISTEN(tier::release, e2::form::prop::ui::title, head_foci)
                {
                    update_header(); // Update focus indicator.
                };
                LISTEN(tier::release, e2::form::prop::ui::header, utf8)
                {
                    if (utf8.length()) // Update os window title.
                    {
                        auto filtered = ui::para{ utf8 }.lyric->utf8();
                        manager::set_window_title(filtered);
                    }
                };
                LISTEN(tier::release, e2::form::prop::ui::footer, utf8)
                {
                    update_footer();
                };
            }
            auto winio = std::thread{[&]
            {
                auto sync = [&](view data)
                {
                    auto lock = bell::sync();
                    proxy.sync(data);
                    update();
                };
                directvt::binary::stream::reading_loop(proxy.intio, sync);
                proxy.stop(); // Wake up waiting objects, if any.
                manager::close(); // Interrupt dispatching.
            }};
            dispatch();
            {
                auto lock = bell::sync();
                proxy.gears.reset();
            }
            proxy.intio.shut(); // Close link to server. Interrupt binary reading loop.
            winio.join();
        }
    };
}