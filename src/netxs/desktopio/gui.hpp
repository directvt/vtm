// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

//dx3d specific
//#include <d3d11_2.h>
//#include <d2d1_2.h>
//#include <dcomp.h>
//#include <wrl.h> // ComPtr
//using namespace Microsoft::WRL;
//#define GDI_ONLY 1

#include <dwrite_2.h>
#pragma comment(lib, "Gdi32")
#pragma comment(lib, "Dwrite.lib")

#define ok2(...) [&](){ auto hr = __VA_ARGS__; if (hr != S_OK) log(utf::to_hex(hr), " ", #__VA_ARGS__); return hr == S_OK; }()

namespace netxs::gui
{
    using namespace input;

    auto fx_trans = [](auto& c){ c = 0x01'00'00'00; };
    auto fx_white = [](auto& c){ c = 0x5F'23'23'23; };
    auto fx_black = [](auto& c){ c = 0x3F'00'00'00; };

    namespace style
    {
        static constexpr auto normal = 0;
        static constexpr auto italic = 1;
        static constexpr auto bold = 2;
        static constexpr auto bold_italic = bold | italic;
    };

    struct w32surface : IDWriteTextRenderer
    {
        using bits = netxs::raster<std::span<argb>, rect>;
        using dwrt = IDWriteBitmapRenderTarget*;

        struct gcfg
        {
            IDWriteFactory2*        pDWriteFactory{};
            IDWriteGdiInterop*      pGdiInterop{};
            IDWriteRenderingParams* pNaturalRendering{};
            IDWriteRenderingParams* pAliasedRendering{};
            IDWriteTextFormat*      pTextFormat[4]{};
            void reset()
            {
                pNaturalRendering->Release();
                pAliasedRendering->Release();
                pTextFormat[style::normal]->Release();
                pTextFormat[style::italic]->Release();
                pTextFormat[style::bold]->Release();
                pTextFormat[style::bold_italic]->Release();
                pGdiInterop->Release();
                pDWriteFactory->Release();
            }
        };

        HDC  hdc;
        argb fgc;
        HWND hWnd;
        bool sync;
        rect prev;
        rect area;
        twod size;
        ui32 refs;
        gcfg conf;
        dwrt surf;

        w32surface(w32surface const&) = default;
        w32surface(w32surface&&) = default;
        w32surface(gcfg context, HWND hWnd)
            :  hdc{},
              hWnd{ hWnd },
              sync{ faux },
              prev{ .coor = dot_mx },
              area{ dot_00, dot_00 },
              size{ dot_00 },
              refs{ 0 },
              conf{ context },
              surf{ nullptr }
        { }
        void set_dpi(auto /*dpi*/)
        {
            //auto pixelsPerDip = dpi / 96.f;
            //surf->SetPixelsPerDip(pixelsPerDip);
        }
        void reset() // We are not using custom copy/move ctors.
        {
            if (surf) surf->Release();
        }
        auto canvas(bool wipe = faux)
        {
            if (area)
            {
                if (area.size != size)
                {
                    auto undo = surf;
                    auto hr = conf.pGdiInterop->CreateBitmapRenderTarget(NULL, area.size.x, area.size.y, &surf); // ET(auto hr = surf->Resize(area.size.x, area.size.y)); // Unnecessary copying.
                    if (hr == S_OK)
                    {
                        hdc = surf->GetMemoryDC();
                        wipe = faux;
                        size = area.size;
                        if (undo) undo->Release();
                        surf->SetPixelsPerDip(1.f); // Ignore DPI to be pixel perfect.
                    }
                    else log("bitmap resizing error: ", utf::to_hex(hr));
                }
                if (hdc)
                {
                    sync = faux;
                    auto pv = BITMAP{};
                    if (sizeof(BITMAP) == ::GetObjectW(::GetCurrentObject(hdc, OBJ_BITMAP), sizeof(BITMAP), &pv))
                    {
                        auto sz = (sz_t)size.x * size.y;
                        if (wipe) std::memset(pv.bmBits, 0, sz * sizeof(argb));
                        return bits{ std::span<argb>{ (argb*)pv.bmBits, sz }, rect{ area.coor, size }};
                    }
                }
            }
            return bits{};
        }
        auto textout(rect clip, twod step, argb color, si32 format, wiew txt)
        {
            auto dest = rect{ clip.coor + step, clip.size - step };
            if (!dest || !hdc) return;
            fgc = color;
            auto pTextLayout = (IDWriteTextLayout*)nullptr;
            auto l = (ui32)txt.size();
            auto w = (fp32)std::abs(dest.size.x);
            auto h = (fp32)std::abs(dest.size.y);
            ok2(conf.pDWriteFactory->CreateTextLayout(txt.data(), l, conf.pTextFormat[format], w, h, &pTextLayout));
            if (dest.size.y < 0)
            {
                auto dtm = DWRITE_TEXT_METRICS{};
                pTextLayout->GetMetrics(&dtm);
                auto minHeight = dtm.height;
                dest.coor.y -= (si32)minHeight;
                dest.size.y = -dest.size.y;
            }
            if (dest.size.x < 0)
            {
                auto min_width = fp32{};
                pTextLayout->DetermineMinWidth(&min_width);
                dest.coor.x -= (si32)min_width;
                dest.size.x = -dest.size.x;
            }
            //pTextLayout->SetUnderline(true, DWRITE_TEXT_RANGE{ 0, l });
            //pTextLayout->SetStrikethrough(true, DWRITE_TEXT_RANGE{ 0, l });
            auto left = clip.coor.x;
            auto top = clip.coor.y;
            auto right = left + clip.size.x;
            auto bottom = top + clip.size.y;
            ::IntersectClipRect(hdc, left, top, right, bottom);
            ok2(pTextLayout->Draw(hdc, this, (fp32)dest.coor.x, (fp32)dest.coor.y));
            ::ExcludeClipRect(hdc, left, top, right, bottom);
            pTextLayout->Release();
        }
        void present()
        {
            if (sync || !hdc) return;
            //netxs::misc::fill(canvas(), [](argb& c){ c.pma(); });
            static auto blend_props = BLENDFUNCTION{ .BlendOp = AC_SRC_OVER, .SourceConstantAlpha = 255, .AlphaFormat = AC_SRC_ALPHA };
            auto scr_coor = POINT{};
            auto old_size =  SIZE{ prev.size.x, prev.size.y };
            auto old_coor = POINT{ prev.coor.x, prev.coor.y };
            auto win_size =  SIZE{      size.x,      size.y };
            auto win_coor = POINT{ area.coor.x, area.coor.y };
            //auto sized = prev.size(     size);
            auto moved = prev.coor(area.coor);
            auto rc = ::UpdateLayeredWindow(hWnd,   // 1.5 ms (copy bitmap to hardware)
                                            HDC{},                       // No color palette matching.  HDC hdcDst,
                                            moved ? &win_coor : nullptr, // POINT         *pptDst,
                                            &win_size,                   // SIZE          *psize,
                                            hdc,                         // HDC           hdcSrc,
                                            &scr_coor,                   // POINT         *pptSrc,
                                            {},                          // COLORREF      crKey,
                                            &blend_props,                // BLENDFUNCTION *pblend,
                                            ULW_ALPHA);                  // DWORD         dwFlags
            if (!rc) log("UpdateLayeredWindow returns unexpected result ", rc);
            sync = true;
        }
        void close()
        {
            ::SendMessageW(hWnd, WM_CLOSE, NULL, NULL);
        }
        IFACEMETHOD(DrawGlyphRun)(void* /*clientDrawingContext*/, fp32 baselineOriginX, fp32 baselineOriginY, DWRITE_MEASURING_MODE measuringMode, DWRITE_GLYPH_RUN const* glyphRun, DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription, IUnknown* /*clientDrawingEffect*/)
        {
            auto dirtyRect = RECT{};
            auto para_layers = (IDWriteColorGlyphRunEnumerator*)nullptr;
            auto hr = conf.pDWriteFactory->TranslateColorGlyphRun(baselineOriginX, baselineOriginY, glyphRun, glyphRunDescription, measuringMode, nullptr, 0, &para_layers);
            if (para_layers) //todo cache color glyphs
            {
                auto subrun = (DWRITE_COLOR_GLYPH_RUN const*)nullptr;
                auto next = BOOL{ true };
                do
                {
                    hr = para_layers->GetCurrentRun(&subrun);
                    auto& s = *subrun;
                    auto color = argb{ s.runColor.r, s.runColor.g, s.runColor.b, s.runColor.a }; // s.runColor.a could be nan != 0.
                    if (hr == S_OK && color.chan.a)
                    {
                        hr = surf->DrawGlyphRun(s.baselineOriginX, s.baselineOriginY,
                                                measuringMode,
                                                &s.glyphRun,
                                                conf.pNaturalRendering, // Emojis are broken without AA (antialiasing).
                                                argb::swap_rb(s.paletteIndex == -1 ? fgc.token : color.token),
                                                &dirtyRect);
                    }
                }
                while (para_layers->MoveNext(&next), next);
            }
            else if (hr == DWRITE_E_NOCOLOR)
            {
                hr = surf->DrawGlyphRun(baselineOriginX,
                                        baselineOriginY,
                                        measuringMode,
                                        glyphRun,
                                        //conf.pNaturalRendering,
                                        conf.pAliasedRendering,
                                        argb::swap_rb(fgc.token),
                                        &dirtyRect);
            }
            return hr;
        }
        IFACEMETHOD(IsPixelSnappingDisabled)(void* /*clientDrawingContext*/, BOOL* isDisabled)
        {
            *isDisabled = FALSE;
            return S_OK;
        }
        IFACEMETHOD(GetCurrentTransform)(void* /*clientDrawingContext*/ , DWRITE_MATRIX* transform)
        {
            surf->GetCurrentTransform(transform);
            return S_OK;
        }
        IFACEMETHOD(GetPixelsPerDip)(void* /*clientDrawingContext*/, fp32* pixelsPerDip)
        {
            *pixelsPerDip = surf->GetPixelsPerDip();
            return S_OK;
        }
        IFACEMETHOD(DrawUnderline)(void* /*clientDrawingContext*/, fp32 /*baselineOriginX*/, fp32 /*baselineOriginY*/, DWRITE_UNDERLINE const* /*underline*/, IUnknown* /*clientDrawingEffect*/)
        {
            return E_NOTIMPL;
        }
        IFACEMETHOD(DrawStrikethrough)(void* /*clientDrawingContext*/, fp32 /*baselineOriginX*/, fp32 /*baselineOriginY*/, DWRITE_STRIKETHROUGH const* /*strikethrough*/, IUnknown* /*clientDrawingEffect*/)
        {
            return E_NOTIMPL;
        }
        IFACEMETHOD(DrawInlineObject)(void* /*clientDrawingContext*/, fp32 /*originX*/, fp32 /*originY*/, IDWriteInlineObject* /*inlineObject*/, BOOL /*isSideways*/, BOOL /*isRightToLeft*/, IUnknown* /*clientDrawingEffect*/)
        {
            return E_NOTIMPL;
        }
        IFACEMETHOD_(unsigned long, AddRef)()
        {
            return InterlockedIncrement(&refs);
        }
        IFACEMETHOD_(unsigned long, Release) ()
        {
            auto new_refs = InterlockedDecrement(&refs);
            if (new_refs == 0) delete this;
            return new_refs;
        }
        IFACEMETHOD(QueryInterface)(IID const& riid, void** ppvObject)
        {
                 if (__uuidof(IDWriteTextRenderer)  == riid) *ppvObject = this;
            else if (__uuidof(IDWritePixelSnapping) == riid) *ppvObject = this;
            else if (__uuidof(IUnknown)             == riid) *ppvObject = this;
            else                                           { *ppvObject = NULL; return E_FAIL; }
            return S_OK;
        }
    };

    template<class window>
    struct w32renderer
    {
        using gcfg = w32surface::gcfg;
        using wins = std::vector<w32surface>;

        gcfg conf;
        bool initialized;

        w32renderer(text font_name_utf8, twod cell_size)
            : initialized{ faux }
        {
            set_dpi_awareness();
            ok2(::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&conf.pDWriteFactory)));
            ok2(conf.pDWriteFactory->GetGdiInterop(&conf.pGdiInterop));
            auto font_name = utf::to_utf(font_name_utf8);
            auto font_size = (fp32)cell_size.y;
            auto locale = L"en-en"s;
            auto font_collection = nullptr;
            ok2(conf.pDWriteFactory->CreateTextFormat(font_name.data(), font_collection, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, font_size, locale.data(), &conf.pTextFormat[style::normal]));
            ok2(conf.pDWriteFactory->CreateTextFormat(font_name.data(), font_collection, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_ITALIC, DWRITE_FONT_STRETCH_NORMAL, font_size, locale.data(), &conf.pTextFormat[style::italic]));
            ok2(conf.pDWriteFactory->CreateTextFormat(font_name.data(), font_collection, DWRITE_FONT_WEIGHT_BOLD,   DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, font_size, locale.data(), &conf.pTextFormat[style::bold]));
            ok2(conf.pDWriteFactory->CreateTextFormat(font_name.data(), font_collection, DWRITE_FONT_WEIGHT_BOLD,   DWRITE_FONT_STYLE_ITALIC, DWRITE_FONT_STRETCH_NORMAL, font_size, locale.data(), &conf.pTextFormat[style::bold_italic]));
            //ok2(conf.pDWriteFactory->CreateRenderingParams(&conf.pAliasedRendering));
            ok2(conf.pDWriteFactory->CreateCustomRenderingParams(1.f/*no gamma*/, 0.f/*nocontrast*/, 0.f/*grayscale*/, DWRITE_PIXEL_GEOMETRY_FLAT, DWRITE_RENDERING_MODE_ALIASED, &conf.pAliasedRendering));
            ok2(conf.pDWriteFactory->CreateCustomRenderingParams(2.2f/*sRGB gamma*/, 0.f/*nocontrast*/, 0.5f/*cleartype*/, DWRITE_PIXEL_GEOMETRY_FLAT, DWRITE_RENDERING_MODE_NATURAL_SYMMETRIC, &conf.pNaturalRendering));
            initialized = true;
        }
        ~w32renderer()
        {
            conf.reset();
        }
        void set_dpi_awareness()
        {
            auto proc = (LONG(_stdcall *)(si32))::GetProcAddress(::GetModuleHandleA("user32.dll"), "SetProcessDpiAwarenessInternal");
            if (proc)
            {
                auto hr = proc(2/*PROCESS_PER_MONITOR_DPI_AWARE*/);
                if (hr != S_OK) log("Set DPI awareness failed ", utf::to_hex(hr));
            }
        }
        constexpr explicit operator bool () const { return initialized; }
        auto add(wins& layers, bool transparent, si32 owner_index = -1, window* host_ptr = nullptr)
        {
            auto window_proc = [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
            {
                //log("\tmsW=", utf::to_hex(msg), " wP=", utf::to_hex(wParam), " lP=", utf::to_hex(lParam), " hwnd=", utf::to_hex(hWnd));
                auto w = (window*)::GetWindowLongPtrW(hWnd, GWLP_USERDATA);
                if (!w) return ::DefWindowProcW(hWnd, msg, wParam, lParam);
                auto stat = LRESULT{};
                auto hi = [](auto n){ return (si32)(si16)((n >> 16) & 0xffff); };
                auto lo = [](auto n){ return (si32)(si16)((n >> 0 ) & 0xffff); };
                static auto i = 0;
                if (w->tasks) log("unsync ", i++);
                switch (msg)
                {
                    case WM_MOUSEMOVE:   w->mouse_shift({ lo(lParam), hi(lParam) }); break; // Client-based coord.
                    case WM_LBUTTONDOWN: w->mouse_press(window::bttn::left,   true); break;
                    case WM_MBUTTONDOWN: w->mouse_press(window::bttn::middle, true); break;
                    case WM_RBUTTONDOWN: w->mouse_press(window::bttn::right,  true); break;
                    case WM_LBUTTONUP:   w->mouse_press(window::bttn::left,   faux); break;
                    case WM_MBUTTONUP:   w->mouse_press(window::bttn::middle, faux); break;
                    case WM_RBUTTONUP:   w->mouse_press(window::bttn::right,  faux); break;
                    case WM_MOUSEWHEEL:  w->mouse_wheel(hi(wParam), faux);           break;
                    case WM_MOUSEHWHEEL: w->mouse_wheel(hi(wParam), true);           break;
                    case WM_MOUSELEAVE:  w->mouse_hover(faux);                       break;
                    case WM_ACTIVATEAPP: w->focus_event(!!wParam);                   break; // Focus between apps.
                    case WM_ACTIVATE:    w->state_event(!!lo(wParam), !!hi(wParam)); break; // Window focus within the app.
                    //case WM_MOUSEACTIVATE: stat = MA_NOACTIVATE;                     break; // Suppress window activation with a mouse click.
                    case WM_SHOWWINDOW:  w->shown_event(!!wParam, lParam);           break; //todo revise
                    //case WM_GETMINMAXINFO: w->maximize(wParam, lParam);              break; // The system is about to maximize the window.
                    //case WM_SYSCOMMAND:  w->sys_command(wParam, lParam);             break; //todo taskbar ctx menu to change the size and position
                    case WM_KEYDOWN:
                    case WM_KEYUP:
                    case WM_SYSKEYDOWN:  // WM_CHAR/WM_SYSCHAR and WM_DEADCHAR/WM_SYSDEADCHAR are derived messages after translation.
                    case WM_SYSKEYUP:    w->keybd_press(wParam, lParam);             break;
                    case WM_DPICHANGED:  w->set_dpi(lo(wParam));                     break;
                    case WM_DESTROY:     ::PostQuitMessage(0);                       break;
                    //dx3d specific
                    case WM_PAINT:   /*w->check_dx3d_state();*/ stat = ::DefWindowProcW(hWnd, msg, wParam, lParam); break;
                    default:                                    stat = ::DefWindowProcW(hWnd, msg, wParam, lParam); break;
                }
                if (w->tasks) w->redraw();
                return stat;
            };
            static auto wc_defwin = WNDCLASSW{ .lpfnWndProc = ::DefWindowProcW, .lpszClassName = L"vtm" };
            static auto wc_window = WNDCLASSW{ .lpfnWndProc = window_proc, .hCursor = ::LoadCursorW(NULL, (LPCWSTR)IDC_ARROW), .lpszClassName = L"vtm_internal" };
            static auto reg = ::RegisterClassW(&wc_defwin) && ::RegisterClassW(&wc_window);
            if (!reg)
            {
                initialized = faux;
                log("window class registration error: ", ::GetLastError());
            }
            auto& wc = transparent ? wc_window : wc_defwin;
            auto owner = owner_index == -1 ? HWND{} : layers[owner_index].hWnd;
            auto hWnd = ::CreateWindowExW(WS_EX_NOREDIRECTIONBITMAP | WS_EX_LAYERED | (wc.hCursor ? 0 : WS_EX_TRANSPARENT),
                                          wc.lpszClassName, owner ? nullptr : wc.lpszClassName, // Title.
                                          WS_POPUP /*todo | owner ? WS_SYSMENU : 0  taskbar ctx menu*/, 0, 0, 0, 0, owner, 0, 0, 0);
            if (!hWnd)
            {
                initialized = faux;
                log("window creation error: ", ::GetLastError());
            }
            else if (host_ptr) ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)host_ptr);
            auto win_index = (si32)layers.size();
            layers.emplace_back(conf, hWnd);
            return win_index;
        }
    };

    auto canvas_text = ansi::itc(true).add("vtm GUI frontend").itc(faux).fgc(tint::redlt).bld(true).add(" is currently under development.").nil()
        .add(" You can try it on any versions/editions of Windows platforms starting from Windows 8.1 (with colored emoji!), including Windows Server Core. 😀😬😁😂😃😄😅😆 👌🐞😎👪.").fgc(tint::greenlt).add(" Press Esc or Right click to close.");
    auto header_text = L"Windows Command Prompt - 😎 - C:\\Windows\\System32\\"s;
    auto footer_text = L"4/4000 80:25"s;
    auto canvas_para = ui::para{ canvas_text };
    auto header_para = ui::para{ utf::to_utf(header_text) };
    auto footer_para = ui::para{ utf::to_utf(footer_text) };

    template<template<class Window> class Renderer>
    struct window
    {
        using reng = Renderer<window>;
        using wins = reng::wins;
        using gray = netxs::raster<std::vector<byte>, rect>;
        using shad = netxs::misc::shadow<gray, cell::shaders::alpha>;

        //dx3d specific
        //static HRESULT __stdcall D2D1CreateFactory(D2D1_FACTORY_TYPE, IID const&, D2D1_FACTORY_OPTIONS*, void**) {}
        //#define import_dx3d \
        //    X(D3D11CreateDevice, D3D11) \
        //    X(CreateDXGIFactory2, Dxgi) \
        //    X(D2D1CreateFactory, D2d1) \
        //    X(DCompositionCreateDevice, Dcomp)
        //#define X(func, dll) std::decay<decltype(##func)>::type func##_ptr{}; \
        //                    HMODULE dll##_dll{};
        //    import_dx3d
        //#undef X
        //ComPtr<ID3D11Device>          d3d_Device;
        //ComPtr<IDXGIDevice>           dxgi_Device;
        //ComPtr<IDXGIFactory2>         dxgi_Factory;
        //ComPtr<IDXGISwapChain1>       dxgi_SwapChain;
        //ComPtr<IDXGISurface2>         dxgi_Surface0;
        //ComPtr<ID2D1Factory2>         d2d_Factory;
        //ComPtr<ID2D1Device1>          d2d_Device;
        //ComPtr<ID2D1DeviceContext>    d2d_DC;
        //ComPtr<ID2D1SolidColorBrush>  d2d_Brush;
        //ComPtr<ID2D1Bitmap1>          d2d_Bitmap;
        //ComPtr<IDCompositionDevice>   dcomp_Device;
        //ComPtr<IDCompositionTarget>   dcomp_Target;
        //ComPtr<IDCompositionVisual>   dcomp_Visual;

        enum bttn
        {
            left   = 1 << 0,
            right  = 1 << 1,
            middle = 1 << 2,
        };
        struct task
        {
            enum
            {
                moved = 1 << 0,
                sized = 1 << 1,
                grips = 1 << 2,
                inner = 1 << 3,
                title = 1 << 4,
                all = moved | sized | grips | inner | title,
            };
            si32 list{ all };
            auto& operator += (si32 t) { list |= t; return *this; }
            //auto& operator -= (si32 t) { list &=~t; return *this; }
            auto operator () (si32 t) { return list & t; }
            void reset() { list = 0; }
            constexpr explicit operator bool () const
            {
                return !!list;
            }
        };

        //bool dx3d; //dx3d specific
        reng engine;
        wins layers;
        twod mouse_coord;
        twod grid_size;
        twod cell_size;
        twod grip_cell;
        twod grip_size;
        shad shadow_rastr;
        dent inner_dent;
        dent outer_dent;
        rect inner_rect;
        netxs::misc::szgrips grip;
        bool hovered{};
        bool grips_drawn;
        si32 buttons;
        task tasks;
        bool initialized{};
        si32 shadow;
        si32 header;
        si32 footer;
        si32 client;
        static constexpr auto shadow_dent = dent{ 1,1,1,1 } * 3;
        constexpr explicit operator bool () const { return initialized; }

        ~window()
        {
            for (auto& w : layers) w.reset();
        }
        window(rect win_coor_px_size_cell, text font, twod cell_size = { 10, 20 }, si32 win_mode = 0, twod grip_cell = { 2, 1 })
            : //dx3d{ faux }, //dx3d specific
              engine{ font, cell_size },
              grid_size{ std::max(dot_11, win_coor_px_size_cell.size) },
              cell_size{ cell_size },
              grip_cell{ grip_cell },
              grip_size{ grip_cell * cell_size },
              shadow_rastr{ 0.44f/*bias*/, 116.5f/*alfa*/, cell_size.x, dot_00, dot_11, [](auto& c, auto a){ c = a; } },
              inner_dent{},
              outer_dent{ grip_size.x, grip_size.x, grip_size.y, grip_size.y },
              inner_rect{ win_coor_px_size_cell.coor, grid_size * cell_size },
              grips_drawn{ faux },
              buttons{},
              shadow{ engine.add(layers, 0) },
              header{ engine.add(layers, 0, shadow) },
              footer{ engine.add(layers, 0, shadow) },
              client{ engine.add(layers, 1, shadow, this) }
        {
            if (!engine) return;
            layers[shadow].area = { inner_rect.coor - shadow_rastr.over / 2, inner_rect.size + shadow_rastr.over };
            layers[client].area = inner_rect + outer_dent;

            layers[header].area = rect{ inner_rect.coor, { inner_rect.size.x, -cell_size.y * ((header_para.size().x + grid_size.x - 1)/ grid_size.x) }}.normalize_itself() + shadow_dent;
            layers[footer].area = rect{{ inner_rect.coor.x, inner_rect.coor.y + inner_rect.size.y }, { inner_rect.size.x, cell_size.y * ((footer_para.size().x + grid_size.x - 1)/ grid_size.x) }} + shadow_dent;
            layers[header].area.coor.y -= shadow_dent.b;
            //layers[footer].area.coor.y += shadow_dent.t;

            //dx3d specific
            //dx3d = [&]
            //{
            //    if (GDI_ONLY) return faux;
            //    #define X(func, dll) \
            //        dll##_dll = ::LoadLibraryExA(#dll ".dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32); \
            //        if (!dll##_dll) return faux; \
            //        func##_ptr = reinterpret_cast<std::decay<decltype(##func)>::type>(::GetProcAddress(dll##_dll, #func));\
            //        if (!func##_ptr) return faux;
            //        import_dx3d
            //    #undef import_dx3d
            //    #undef GDI_ONLY
            //    return true;
            //}();
            //if (dx3d) reinit();
            //else log("Direct3D not found.");
            //#undef X

            redraw();
            initialized = true;
            show(win_mode);
        }
        auto set_dpi(auto new_dpi)
        {
            for (auto& w : layers) w.set_dpi(new_dpi);
            log("%%DPI changed to %dpi%", prompt::gui, new_dpi);
            //tasks += task::all;
        }
        auto move_window(twod coor_delta)
        {
            layers[shadow].area.coor += coor_delta;
            layers[client].area.coor += coor_delta;
            inner_rect.coor          += coor_delta;
            //todo unify
            layers[header].area = rect{ inner_rect.coor, { inner_rect.size.x, -cell_size.y * ((header_para.size().x + grid_size.x - 1)/ grid_size.x) }}.normalize_itself() + shadow_dent;
            layers[footer].area = rect{{ inner_rect.coor.x, inner_rect.coor.y + inner_rect.size.y }, { inner_rect.size.x, cell_size.y * ((footer_para.size().x + grid_size.x - 1)/ grid_size.x) }} + shadow_dent;
            layers[header].area.coor.y -= shadow_dent.b;
            //layers[footer].area.coor.y += shadow_dent.t;

            tasks += task::moved;
        }
        auto size_window(twod size_delta)
        {
            layers[client].area.size += size_delta;
            layers[shadow].area.size += size_delta;
            inner_rect.size          += size_delta;
            //todo unify
            layers[header].area = rect{ inner_rect.coor, { inner_rect.size.x, -cell_size.y * ((header_para.size().x + grid_size.x - 1)/ grid_size.x) }}.normalize_itself() + shadow_dent;
            layers[footer].area = rect{{ inner_rect.coor.x, inner_rect.coor.y + inner_rect.size.y }, { inner_rect.size.x, cell_size.y * ((footer_para.size().x + grid_size.x - 1)/ grid_size.x) }} + shadow_dent;
            layers[header].area.coor.y -= shadow_dent.b;
            //layers[footer].area.coor.y += shadow_dent.t;

            tasks += task::sized;
        }
        auto resize_window(twod size_delta)
        {
            auto new_size = inner_rect.size + size_delta;
            auto new_grid_size = std::max(dot_11, new_size / cell_size);
            size_delta = dot_00;
            if (grid_size != new_grid_size)
            {
                grid_size = new_grid_size;
                size_delta = grid_size * cell_size - inner_rect.size;
                size_window(size_delta);
            }
            return size_delta;
        }
        auto warp_window(dent warp_delta)
        {
            auto inner_prev = inner_rect;
            auto new_area = inner_rect + warp_delta;
            auto new_grid_size = std::max(dot_11, new_area.size / cell_size);
            if (grid_size != new_grid_size)
            {
                grid_size = new_grid_size;
                auto size_delta = grid_size * cell_size - inner_rect.size;
                auto coor_delta = new_area.coor - inner_rect.coor;
                size_window(size_delta);
                move_window(coor_delta);
            }
            return inner_rect - inner_prev;
        }
        void mouse_hover(bool hover)
        {
            if (!hover)
            {
                hovered = faux;
                if (grips_drawn) tasks += task::grips;
            }
            else
            {
                if (!hovered)
                {
                    hovered = true;
                    static auto mouse_track = TRACKMOUSEEVENT{ .cbSize = sizeof(TRACKMOUSEEVENT),
                                                               .dwFlags = TME_LEAVE,
                                                               .hwndTrack = layers[client].hWnd,
                                                               .dwHoverTime = HOVER_DEFAULT };
                    auto rc = ::TrackMouseEvent(&mouse_track);
                    if (!rc) log("track = ", rc ? "ok":"failed");
                }
            }
        }
        void draw_grid()
        {
            auto canvas = layers[client].canvas();
            auto r  = rect{{}, cell_size };
            auto lt = dent{ 1, 0, 1, 0 };
            auto rb = dent{ 0, 1, 0, 1 };
            auto fx_pure_wt = [](auto& c){ c = 0xFF'ff'ff'ff; };
            auto fx_white2  = [](auto& c){ c = 0xAf'7f'7f'7f; };
            auto fx_black2  = [](auto& c){ c = 0xAF'00'00'00; };
            auto fx_blue    = [](auto& c){ c = 0xAF'00'00'7f; };
            //auto fx_white = [](auto& c){ c = 0xFF'7f'7f'7f; };
            //auto fx_black = [](auto& c){ c = 0xFF'00'00'00; };
            //auto fx_blue  = [](auto& c){ c = 0xFF'00'00'7f; };
            canvas.step(-inner_rect.coor);
            auto rtc = argb{ tint::pureblue  }.alpha(0.5f);
            auto ltc = argb{ tint::pureblack };
            auto rbc = argb{ tint::pureblack };
            auto lbc = argb{ tint::pureblue  }.alpha(0.5f);
            for (r.coor.y = 0; r.coor.y < inner_rect.size.y; r.coor.y += cell_size.y)
            {
                auto y = (fp32)r.coor.y / (inner_rect.size.y - 1);
                auto lc = argb::transit(ltc, lbc, y);
                auto rc = argb::transit(rtc, rbc, y);
                for (r.coor.x = 0; r.coor.x < inner_rect.size.x; r.coor.x += cell_size.x)
                {
                    auto x = (fp32)r.coor.x / (inner_rect.size.x - 1);
                    auto p = argb::transit(lc, rc, x);
                    netxs::onrect(canvas, r, [&](auto& c){ c = p; });
                    //netxs::misc::cage(canvas, r, lt, fx_white2);
                    //netxs::misc::cage(canvas, r, rb, fx_black2);
                }
            }

            //auto c1 = rect{ dot_00, inner_rect.size - dent{ 0, inner_rect.size.x / 2, 0, 0 }};
            //auto c2 = c1;
            //auto c3 =  rect{ dot_00, inner_rect.size };
            //c2.coor.x += inner_rect.size.x / 2;
            //netxs::onrect(canvas, c1, fx_black);
            //netxs::onrect(canvas, c2, fx_pure_wt);
            //netxs::misc::cage(canvas, c2, 1, fx_black);
            //netxs::onrect(canvas, c3, fx_pure_wt);

            canvas.step(inner_rect.coor);
            auto r_init = rect{ .coor = grip_size + cell_size * dot_01, .size = inner_rect.size };
            r = r_init;
            r.coor.y += cell_size.y;
            auto& content = canvas_para.content();
            auto& layer = layers[client];
            auto m = grip_size + inner_rect.size;
            auto right_part = twod{ -cell_size.x, 0 };
            for (auto& c : content)
            {
                auto format = style::normal;
                if (c.itc()) format |= style::italic;
                if (c.bld()) format |= style::bold;
                auto fgc = c.fgc();
                if (!fgc) fgc = argb{ tint::cyanlt };
                auto w = c.wdt() == 3 ? right_part : dot_00;
                layer.textout(r, w, fgc, format, utf::to_utf(c.txt()));
                r.coor.x += cell_size.x;
                if (r.coor.x >= m.x)
                {
                    r.coor.x = r_init.coor.x;
                    r.coor.y += cell_size.y;
                    if (r.coor.y >= m.y) break;
                }
            }
        }
        bool hit_grips()
        {
            auto hit = !grip.zoomon && (grip.seized || (hovered && !inner_rect.hittest(mouse_coord) && layers[client].area.hittest(mouse_coord)));
            return hit;
        }
        void draw_grips()
        {
            auto canvas = layers[client].canvas();
            auto fx_trans2 = [](auto& c){ c = 0x01'00'00'00; };
            auto fx_white2 = [](auto& c){ c = 0x5F'23'23'23; };
            auto fx_black2 = [](auto& c){ c = 0x3F'00'00'00; };
            netxs::misc::cage(canvas, canvas.area(), outer_dent, fx_trans2); // Transparent grips.
            grips_drawn = hit_grips();
            if (grips_drawn)
            {
                auto [side_x, side_y] = grip.draw(canvas, canvas.area(), fx_white2);
                auto s = grip.sector;
                netxs::misc::cage(canvas, side_x, dent{ s.x < 0, s.x > 0, s.y > 0, s.y < 0 }, fx_black2); // 1-px dark contour around.
                netxs::misc::cage(canvas, side_y, dent{ s.x > 0, s.x < 0, 1, 1 }, fx_black2);             //
                //log("grips ", side_x, " ", side_y);
            }
        }
        void draw_shadow() //todo draw shadow per cell
        {
            auto canvas = layers[shadow].canvas();
            canvas.move(-shadow_rastr.over / 2); //todo unify
            shadow_rastr.render(canvas, inner_rect.size);
        }
        void draw_titles()
        {
            auto header_dest = layers[header].canvas(true);
            auto footer_dest = layers[footer].canvas(true);
            auto h = header_dest.area().moveto(dot_00).rotate({ 1, -1 }) - shadow_dent;
            auto f = footer_dest.area().moveto(dot_00).rotate({ -1, 1 }) - shadow_dent;
            layers[header].textout(h, dot_00, 0xFF'ff'ff'ff, style::normal, header_text);
            layers[footer].textout(f, dot_00, 0xFF'ff'ff'ff, style::normal, footer_text);
            netxs::misc::contour(header_dest);
            netxs::misc::contour(footer_dest);
        }
        void redraw()
        {
            auto mods = tasks;
            tasks.reset();
            if (mods.list == task::moved)
            {
                //todo unify
                auto lock = ::BeginDeferWindowPos((si32)layers.size());
                for (auto& w : layers)
                {
                    lock = ::DeferWindowPos(lock, w.hWnd, 0, w.area.coor.x, w.area.coor.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
                    if (!lock) { log("error: ", ::GetLastError()); throw; }
                    w.prev.coor = w.area.coor;
                }
                ::EndDeferWindowPos(lock);
            }
            else if (mods)
            {
                if (mods(task::sized | task::inner))   draw_grid(); // 0.600 ms
                if (mods(task::sized | task::grips))  draw_grips(); // 0.150 ms
                if (mods(task::sized              )) draw_shadow(); // 0.300 ms
                if (mods(task::sized | task::title)) draw_titles();
                //if (hovered/*mouse test*/)
                //{
                //    auto fx_green = [](auto& c){ c = 0x7F'00'3f'00; };
                //    auto cursor = rect{ mouse_coord - (mouse_coord - layers[client].area.coor) % cell_size, cell_size };
                //    netxs::onrect(layers[client].canvas(), cursor, fx_green);
                //}
                for (auto& w : layers) w.present();
                //log("full update");
            }
        }
        //dx3d specific
        //void reinit()
        //{
        //    //todo hWnd_window
        //    if (!ok2(D3D11CreateDevice_ptr(0, D3D_DRIVER_TYPE_HARDWARE, 0, D3D11_CREATE_DEVICE_BGRA_SUPPORT, 0, 0, D3D11_SDK_VERSION, &d3d_Device, 0, 0)))
        //    {
        //        ok2(D3D11CreateDevice_ptr(0, D3D_DRIVER_TYPE_WARP, 0, D3D11_CREATE_DEVICE_BGRA_SUPPORT, 0, 0, D3D11_SDK_VERSION, &d3d_Device, 0, 0)); // No GPU.
        //    }
        //    ok2(d3d_Device.As(&dxgi_Device));
        //    //ok2(::CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, __uuidof(dxFactory), reinterpret_cast<void**>(dxFactory.GetAddressOf())));
        //    ok2(CreateDXGIFactory2_ptr(0, __uuidof(dxgi_Factory), (void**)dxgi_Factory.GetAddressOf()));
        //        auto dxgi_SwapChain_desc = DXGI_SWAP_CHAIN_DESC1{ .Width       = (ui32)layers[shadow].area.size.x,
        //                                                          .Height      = (ui32)layers[shadow].area.size.y,
        //                                                          .Format      = DXGI_FORMAT_B8G8R8A8_UNORM,
        //                                                          .SampleDesc  = { .Count = 1 },
        //                                                          .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        //                                                          .BufferCount = 2,
        //                                                          .SwapEffect  = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,
        //                                                          .AlphaMode   = DXGI_ALPHA_MODE_PREMULTIPLIED };
        //        ok2(dxgi_Factory->CreateSwapChainForComposition(dxgi_Device.Get(), &dxgi_SwapChain_desc, nullptr, dxgi_SwapChain.GetAddressOf()));
        //            // Get the first swap chain's back buffer (surface[0])
        //            ok2(dxgi_SwapChain->GetBuffer(0, __uuidof(dxgi_Surface0), (void**)dxgi_Surface0.GetAddressOf()));
        //    // Create a Direct2D factory
        //    auto d2d_factory_opts = D2D1_FACTORY_OPTIONS{ .debugLevel = D2D1_DEBUG_LEVEL_INFORMATION };
        //    ok2(D2D1CreateFactory_ptr(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(d2d_Factory), &d2d_factory_opts, (void**)d2d_Factory.GetAddressOf()));
        //        // Create the Direct2D device
        //        ok2(d2d_Factory->CreateDevice(dxgi_Device.Get(), d2d_Device.GetAddressOf()));
        //            // Create the Direct2D context
        //            ok2(d2d_Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, d2d_DC.GetAddressOf()));
        //                // Create a Direct2D bitmap that linked with the swap chain surface[0]
        //                auto d2d_bitmap_props = D2D1_BITMAP_PROPERTIES1{ .pixelFormat = { .format = DXGI_FORMAT_B8G8R8A8_UNORM,
        //                                                                                  .alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED },
        //                                                                 .bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW };
        //                ok2(d2d_DC->CreateBitmapFromDxgiSurface(dxgi_Surface0.Get(), d2d_bitmap_props, d2d_Bitmap.GetAddressOf()));
        //                    // Tie the device context with the bitmap
        //                    d2d_DC->SetTarget(d2d_Bitmap.Get());
        //                ok2(d2d_DC->CreateSolidColorBrush({ 0.f, 1.f, 1.f, 0.25f }, d2d_Brush.GetAddressOf()));
        //                d2d_DC->BeginDraw();
        //                    d2d_DC->Clear();
        //                    d2d_DC->FillEllipse({{ 150.0f, 150.0f }, 100.0f, 100.0f }, d2d_Brush.Get());
        //                ok2(d2d_DC->EndDraw());
        //    // Present
        //    ok2(dxgi_SwapChain->Present(1, 0));
        //    ok2(DCompositionCreateDevice_ptr(dxgi_Device.Get(), __uuidof(dcomp_Device), (void**)dcomp_Device.GetAddressOf()));
        //    ok2(dcomp_Device->CreateTargetForHwnd(layers[shadow].hWnd, true, dcomp_Target.GetAddressOf()));
        //    ok2(dcomp_Device->CreateVisual(dcomp_Visual.GetAddressOf()));
        //        ok2(dcomp_Visual->SetContent(dxgi_SwapChain.Get()));
        //        ok2(dcomp_Target->SetRoot(dcomp_Visual.Get()));
        //    ok2(dcomp_Device->Commit());
        //}
        //void check_dx3d_state()
        //{
        //    log("WM_PAINT");
        //    auto valid = BOOL{};
        //    if (dx3d && (!dcomp_Device || (dcomp_Device->CheckDeviceState(&valid), !valid)))
        //    {
        //        reinit();
        //    }
        //}
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
        void mouse_capture() { ::SetCapture(layers[client].hWnd); }
        void mouse_release() { ::ReleaseCapture(); }
        void mouse_wheel(si32 delta, bool /*hz*/)
        {
            auto wheeldt = delta / 120;
            auto kb = kbs();
            //     if (kb & (hids::LCtrl | hids::LAlt)) netxs::_k2 += wheeldt > 0 ? 1 : -1; // LCtrl + Alt t +Wheel.
            //else if (kb & hids::LCtrl)                netxs::_k0 += wheeldt > 0 ? 1 : -1; // LCtrl+Wheel.
            //else if (kb & hids::anyAlt)               netxs::_k1 += wheeldt > 0 ? 1 : -1; // Alt+Wheel.
            //else if (kb & hids::RCtrl)                netxs::_k3 += wheeldt > 0 ? 1 : -1; // RCtrl+Wheel.
            //shadow_rastr = build_shadow_corner(cell_size.x);
            //tasks += task::sized;
            //netxs::_k0 += wheeldt > 0 ? 1 : -1;
            //log("wheel ", wheeldt, " k0= ", _k0, " k1= ", _k1, " k2= ", _k2, " k3= ", _k3, " keybd ", utf::to_bin(kb));

            if ((kb & hids::anyCtrl) && !(kb & hids::ScrlLock))
            {
                if (!grip.zoomon)
                {
                    grip.zoomdt = {};
                    grip.zoomon = true;
                    grip.zoomsz = inner_rect;
                    grip.zoomat = mouse_coord;
                    mouse_capture();
                }
            }
            else if (grip.zoomon)
            {
                grip.zoomon = faux;
                mouse_release();
            }
            if (grip.zoomon)
            {
                auto warp = dent{ grip_size.x, grip_size.x, grip_size.y, grip_size.y };
                auto step = grip.zoomdt + warp * wheeldt;
                auto next = grip.zoomsz + step;
                next.size = std::max(dot_00, next.size);
                ///auto viewport = ...get max win size (multimon)
                //next.trimby(viewport);
                if (warp_window(next - inner_rect)) grip.zoomdt = step;
            }
        }
        void mouse_shift(twod coord)
        {
            auto kb = kbs();// keybd_state();
            mouse_hover(true);
            coord += layers[client].area.coor;
            if (hit_grips() || grip.seized)
            {
                if (buttons & bttn::left)
                {
                    if (!grip.seized) // drag start
                    {
                        grip.grab(inner_rect, mouse_coord, outer_dent, cell_size);
                    }
                    auto zoom = kb & hids::anyCtrl;
                    auto [preview_area, size_delta] = grip.drag(inner_rect, coord, outer_dent, zoom, cell_size);
                    if (auto dxdy = resize_window(size_delta))
                    {
                        if (auto move_delta = grip.move(dxdy, zoom))
                        {
                            move_window(move_delta);
                        }
                    }
                }
                else if (grip.seized) // drag stop
                {
                    grip.drop();
                    tasks += task::grips;
                }
            }
            if (grip.zoomon && !(kb & hids::anyCtrl))
            {
                grip.zoomon = faux;
                mouse_release();
            }
            if (grip.calc(inner_rect, coord, outer_dent, inner_dent, cell_size))
            {
                tasks += task::grips;
            }
            if (!grip.seized && buttons & bttn::left)
            {
                if (auto dxdy = coord - mouse_coord)
                {
                    //log("moveby ", dxdy);
                    for (auto& w : layers)
                    {
                        w.area.coor += dxdy;
                        //log("   coor: ", w.area.coor);
                    }
                    inner_rect.coor += dxdy;
                    tasks += task::moved;
                }
            }
            else
            {
                //dx3d specific
                //auto& x = coord.x;
                //auto& y = coord.y;
                //if (!dx3d)
                //{
                //}
                //else if (d2d_DC)
                //{
                //    d2d_Brush->SetColor({ 0.f, 1.f, 1.f, 0.250f });
                //    d2d_DC->BeginDraw();
                //    //d2d_DC->Clear();
                //    d2d_DC->FillEllipse({{ (float)x, (float)y }, 7.5f, 7.5f }, d2d_Brush.Get());
                //    ok2(d2d_DC->EndDraw());
                //    ok2(dxgi_SwapChain->Present(1, 0));
                //    //ok2(dxgi_SwapChain->Present(1, 0));
                //}
            }
            mouse_coord = coord;
            if (!buttons)
            {
                static auto s = testy{ faux };
                tasks += s(hit_grips()) ? task::grips | task::inner
                                    : s ? task::grips : task::inner;
            }
        }
        void mouse_press(si32 button, bool pressed)
        {
            if (pressed && !buttons) mouse_capture();
            pressed ? buttons |= button
                    : buttons &= ~button;
            if (!buttons) mouse_release();
            if (!pressed & (button == bttn::right)) quit();
        }
        void keybd_press(arch vkey, arch lParam)
        {
            union key_state
            {
                ui32 token;
                struct k
                {
                    ui32 repeat   : 16;// 0-15
                    ui32 scancode : 9; // 16-24 (24 - extended)
                    ui32 reserved : 5; // 25-29 (29 - context)
                    ui32 state    : 2; // 30-31: 0 - pressed, 1 - repeated, 2 - unknown, 3 - released
                };
            };
            auto param = key_state{ .token = (ui32)lParam };
            //log("vkey: ", utf::to_hex(vkey),
            //    " scode: ", utf::to_hex(param.scancode),
            //    " state: ", param.state == 0 ? "pressed"
            //              : param.state == 1 ? "rep"
            //              : param.state == 3 ? "released" : "unknown");
            if (vkey == 0x1b) quit();
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
        void state_event(bool activated, bool minimized)
        {
            log(activated ? "activated" : "deactivated", " ", minimized ? "minimized" : "restored");
        }
        void shown_event(bool shown, arch reason) //todo revise
        {
            log(shown ? "shown" : "hidden", " ", reason == SW_OTHERUNZOOM   ? "The window is being uncovered because a maximize window was restored or minimized."
                                               : reason == SW_OTHERZOOM     ? "The window is being covered by another window that has been maximized."
                                               : reason == SW_PARENTCLOSING ? "The window's owner window is being minimized."
                                               : reason == SW_PARENTOPENING ? "The window's owner window is being restored."
                                                                            : "unknown reason");
        }
        void quit()
        {
            layers[client].close();
        }
        void dispatch()
        {
            auto msg = MSG{};
            while (::GetMessageW(&msg, 0, 0, 0) > 0)
            {
                ::DispatchMessageW(&msg);
            }
        }
        void show(si32 win_state)
        {
            if (win_state == 0 || win_state == 2) //todo fullscreen mode (=2). 0 - normal, 1 - minimized, 2 - fullscreen
            {
                auto mode = SW_SHOWNORMAL;
                for (auto& w : layers) { ::ShowWindow(w.hWnd, mode); }
            }
        }
    };
}