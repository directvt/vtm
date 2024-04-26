// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

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
    auto canvas_text = ansi::itc(true).add("vtm GUI frontend").itc(faux).fgc(tint::redlt).bld(true).add(" is currently under development.").nil()
        .add(" You can try it on any versions/editions of Windows platforms starting from Windows 8.1 (with colored emoji!), including Windows Server Core. 😀😬😁😂😃😄😅😆 👌🐞😎👪.").fgc(tint::greenlt).add(" Press Esc or Right click to close.");
    auto header_text = L"Windows Command Prompt - 😎 - C:\\Windows\\System32\\"s;
    auto footer_text = L"4/4000 80:25"s;
    auto canvas_para = ui::para{ canvas_text };
    auto header_para = ui::para{ utf::to_utf(header_text) };
    auto footer_para = ui::para{ utf::to_utf(footer_text) };

    namespace style
    {
        static constexpr auto normal = 0;
        static constexpr auto italic = 1;
        static constexpr auto bold = 2;
        static constexpr auto bold_italic = bold | italic;
    };

    struct surface : IDWriteTextRenderer
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

        HDC   hdc;
        argb  fgc;
        HWND hWnd;
        bool sync;
        rect prev;
        rect area;
        twod size;
        ui32 refs;
        gcfg conf;
        dwrt surf;

        surface(surface const&) = default;
        surface(surface&&) = default;
        surface(gcfg context, HWND hWnd)
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
        void set_dpi(auto /*dpi*/) // We are do not rely on dpi. Users should configure all metrics in pixels.
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
            ok2(pTextLayout->Draw(hdc, this, (fp32)dest.coor.x, (fp32)dest.coor.y));
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

    struct render
    {
        using gcfg = surface::gcfg;
        using wins = std::vector<surface>;

        enum bttn
        {
            left   = 1 << 0,
            right  = 1 << 1,
            middle = 1 << 2,
        };

        gcfg conf;
        bool initialized;
        wins layers;

        render(text font_name_utf8, twod cellsz)
            : initialized{ faux }
        {
            set_dpi_awareness();
            ok2(::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&conf.pDWriteFactory)));
            ok2(conf.pDWriteFactory->GetGdiInterop(&conf.pGdiInterop));
            auto font_name = utf::to_utf(font_name_utf8);
            auto font_size = (fp32)cellsz.y;
            auto locale = wide(LOCALE_NAME_MAX_LENGTH, '\0');
            if (!::GetUserDefaultLocaleName(locale.data(), (si32)locale.size())) // Return locale length or 0.
            {
                locale = L"en-US";
                log("%%Using default locale 'en-US'.", prompt::gui);
            }
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
        ~render()
        {
            for (auto& w : layers) w.reset();
            conf.reset();
        }
        auto& operator [] (si32 layer) { return layers[layer]; }
        explicit operator bool () const { return initialized; }

        void set_dpi_awareness()
        {
            auto proc = (LONG(_stdcall *)(si32))::GetProcAddress(::GetModuleHandleA("user32.dll"), "SetProcessDpiAwarenessInternal");
            if (proc)
            {
                auto hr = proc(2/*PROCESS_PER_MONITOR_DPI_AWARE*/);
                if (hr != S_OK || hr != E_ACCESSDENIED) log("Set DPI awareness failed ", utf::to_hex(hr), " ", ::GetLastError());
            }
        }
        auto set_dpi(auto new_dpi)
        {
            //for (auto& w : layers) w.set_dpi(new_dpi);
            log("%%DPI changed to %dpi%", prompt::gui, new_dpi);
        }
        auto moveby(twod coor_delta)
        {
            for (auto& w : layers) w.area.coor += coor_delta;
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

        auto add(render* host_ptr = nullptr)
        {
            auto window_proc = [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
            {
                //log("\tmsW=", utf::to_hex(msg), " wP=", utf::to_hex(wParam), " lP=", utf::to_hex(lParam), " hwnd=", utf::to_hex(hWnd));
                //auto layer = (si32)::GetWindowLongPtrW(hWnd, sizeof(LONG_PTR));
                auto w = (render*)::GetWindowLongPtrW(hWnd, GWLP_USERDATA);
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
                    case WM_MOUSELEAVE:  if (!--h) w->mouse_shift(dot_mx), hover_win = {}; break;
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
                initialized = faux;
                log("window class registration error: ", ::GetLastError());
            }
            auto& wc = host_ptr ? wc_window : wc_defwin;
            auto owner = layers.empty() ? HWND{} : layers.front().hWnd;
            auto hWnd = ::CreateWindowExW(WS_EX_NOREDIRECTIONBITMAP | WS_EX_LAYERED | (wc.hCursor ? 0 : WS_EX_TRANSPARENT),
                                          wc.lpszClassName, owner ? nullptr : wc.lpszClassName, // Title.
                                          WS_POPUP /*todo | owner ? WS_SYSMENU : 0  taskbar ctx menu*/, 0, 0, 0, 0, owner, 0, 0, 0);
            auto layer = (si32)layers.size();
            if (!hWnd)
            {
                initialized = faux;
                log("window creation error: ", ::GetLastError());
            }
            else if (host_ptr)
            {
                ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)host_ptr);
                //::SetWindowLongPtrW(hWnd, 0, (LONG_PTR)host_ptr);
                //::SetWindowLongPtrW(hWnd, sizeof(LONG_PTR), (LONG_PTR)layer);
            }
            layers.emplace_back(conf, hWnd);
            return layer;
        }
    };

    struct window : render
    {
        using gray = netxs::raster<std::vector<byte>, rect>;
        using shad = netxs::misc::shadow<gray>;
        using grip = netxs::misc::szgrips;

        struct task
        {
            enum
            {
                moved = 1 << 0,
                sized = 1 << 1,
                grips = 1 << 2,
                hover = 1 << 3,
                inner = 1 << 4,
                header = 1 << 5,
                footer = 1 << 6,
                all = -1,
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

        twod gridsz;
        twod cellsz;
        twod gripsz;
        shad shadow;
        dent border;
        grip szgrip;
        twod mcoord;
        si32 mbttns;
        task reload;
        si32 client;
        si32 grip_l;
        si32 grip_r;
        si32 grip_t;
        si32 grip_b;
        si32 header;
        si32 footer;
        bool drop_shadow{ true };

        static constexpr auto shadow_dent = dent{ 1,1,1,1 } * 3;

        window(rect win_coor_px_size_cell, text font, twod cell_size = { 10, 20 }, si32 win_mode = 0, twod grip_cell = { 2, 1 })
            : render{ font, cell_size },
              gridsz{ std::max(dot_11, win_coor_px_size_cell.size) },
              cellsz{ cell_size },
              gripsz{ grip_cell * cell_size },
              shadow{ 0.44f/*bias*/, 116.5f/*alfa*/, gripsz.x, dot_00, dot_11, [](auto& c, auto a){ c = a; } },
              border{ gripsz.x, gripsz.x, gripsz.y, gripsz.y },
              mbttns{},
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
            update();
            render::show(win_mode);
        }
        void recalc_layout()
        {
            auto base_rect = layers[client].area;
            layers[grip_l].area = base_rect + dent{ gripsz.x, -base_rect.size.x, gripsz.y, gripsz.y };
            layers[grip_r].area = base_rect + dent{ -base_rect.size.x, gripsz.x, gripsz.y, gripsz.y };
            layers[grip_t].area = base_rect + dent{ 0, 0, gripsz.y, -base_rect.size.y };
            layers[grip_b].area = base_rect + dent{ 0, 0, -base_rect.size.y, gripsz.y };
            auto header_height = cellsz.y * ((header_para.size().x + gridsz.x - 1) / gridsz.x);
            auto footer_height = cellsz.y * ((footer_para.size().x + gridsz.x - 1) / gridsz.x);
            layers[header].area = base_rect + dent{ 0, 0, header_height, -base_rect.size.y } + shadow_dent;
            layers[footer].area = base_rect + dent{ 0, 0, -base_rect.size.y, footer_height } + shadow_dent;
            layers[header].area.coor.y -= shadow_dent.b;
            //layers[footer].area.coor.y += shadow_dent.t;
        }
        auto move_window(twod coor_delta)
        {
            render::moveby(coor_delta);
            reload += task::moved;
        }
        auto size_window(twod size_delta)
        {
            layers[client].area.size += size_delta;
            recalc_layout();
            reload += task::sized;
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
        void draw_grid()
        {
            auto canvas = layers[client].canvas();
            auto region = canvas.area();
            auto r  = rect{{}, cellsz };
            auto lt = dent{ 1, 0, 1, 0 };
            auto rb = dent{ 0, 1, 0, 1 };
            auto fx_pure_wt = [](auto& c){ c = 0xFF'ff'ff'ff; };
            auto fx_white2  = [](auto& c){ c = 0xAf'7f'7f'7f; };
            auto fx_black2  = [](auto& c){ c = 0xAF'00'00'00; };
            auto fx_blue    = [](auto& c){ c = 0xAF'00'00'7f; };
            //auto fx_white = [](auto& c){ c = 0xFF'7f'7f'7f; };
            //auto fx_black = [](auto& c){ c = 0xFF'00'00'00; };
            //auto fx_blue  = [](auto& c){ c = 0xFF'00'00'7f; };
            canvas.step(-region.coor);
            auto rtc = argb{ tint::pureblue  }.alpha(0.5f);
            auto ltc = argb{ tint::pureblack };
            auto rbc = argb{ tint::pureblack };
            auto lbc = argb{ tint::pureblue  }.alpha(0.5f);
            for (r.coor.y = 0; r.coor.y < region.size.y; r.coor.y += cellsz.y)
            {
                auto y = (fp32)r.coor.y / (region.size.y - 1);
                auto lc = argb::transit(ltc, lbc, y);
                auto rc = argb::transit(rtc, rbc, y);
                for (r.coor.x = 0; r.coor.x < region.size.x; r.coor.x += cellsz.x)
                {
                    auto x = (fp32)r.coor.x / (region.size.x - 1);
                    auto p = argb::transit(lc, rc, x);
                    netxs::onrect(canvas, r, [&](auto& c){ c = p; });
                    //netxs::misc::cage(canvas, r, lt, fx_white2);
                    //netxs::misc::cage(canvas, r, rb, fx_black2);
                }
            }

            //auto c1 = rect{ dot_00, region.size - dent{ 0, region.size.x / 2, 0, 0 }};
            //auto c2 = c1;
            //auto c3 =  rect{ dot_00, region.size };
            //c2.coor.x += region.size.x / 2;
            //netxs::onrect(canvas, c1, fx_black);
            //netxs::onrect(canvas, c2, fx_pure_wt);
            //netxs::misc::cage(canvas, c2, 1, fx_black);
            //netxs::onrect(canvas, c3, fx_pure_wt);

            canvas.step(region.coor);
            auto r_init = rect{ .coor = gripsz + cellsz * dot_01, .size = region.size };
            r = r_init;
            r.coor.y += cellsz.y;
            auto& content = canvas_para.content();
            auto& layer = layers[client];
            auto m = gripsz + region.size;
            auto right_part = twod{ -cellsz.x, 0 };

            auto hdc = layer.hdc;
            auto left = r.coor.x;
            auto top = r.coor.y;
            auto right = left + r.size.x;
            auto bottom = top + r.size.y;
            auto off = r.coor;
            ::IntersectClipRect(hdc, left, top, right, bottom);
            for (auto& c : content)
            {
                auto format = style::normal;
                if (c.itc()) format |= style::italic;
                if (c.bld()) format |= style::bold;
                auto fgc = c.fgc();
                if (!fgc) fgc = argb{ tint::cyanlt };
                auto w = c.wdt() == 3 ? right_part : dot_00;
                layer.textout(r, w, fgc, format, utf::to_utf(c.txt()));
                r.coor.x += cellsz.x;
                if (r.coor.x >= m.x)
                {
                    r.coor.x = r_init.coor.x;
                    r.coor.y += cellsz.y;
                    if (r.coor.y >= m.y) break;
                }
                off = r.coor - off;
                ::OffsetClipRgn(hdc, off.x, off.y);
            }
            ::ExcludeClipRect(hdc, 0, 0, layer.size.x, layer.size.y);
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
            auto inner_rect = layers[client].area;
            auto outer_rect = layers[client].area + border;
            static auto fx_trans2 = [](auto& c){ c = 0x01'00'00'00; };
            static auto fx_shade2 = [](auto& c){ c = 0x5F'3f'3f'3f; };
            static auto fx_black2 = [](auto& c){ c = 0x3F'00'00'00; };
            fill_grips(outer_rect, [](auto& canvas, auto r){ netxs::misc::fill(canvas, r, fx_trans2); });
            if (hit_grips())
            {
                auto s = szgrip.sector;
                auto [side_x, side_y] = szgrip.layout(outer_rect);
                auto dent_x = dent{ s.x < 0, s.x > 0, s.y > 0, s.y < 0 };
                auto dent_y = dent{ s.x > 0, s.x < 0, 1, 1 };
                fill_grips(side_x, [&](auto& canvas, auto r)
                {
                    netxs::misc::fill(canvas, r, fx_shade2);
                    netxs::misc::cage(canvas, side_x, dent_x, fx_black2); // 1-px dark contour around.
                });
                fill_grips(side_y, [&](auto& canvas, auto r)
                {
                    netxs::misc::fill(canvas, r, fx_shade2);
                    netxs::misc::cage(canvas, side_y, dent_y, fx_black2); // 1-px dark contour around.
                });
            }
            if (drop_shadow) fill_grips(outer_rect, [&](auto& canvas, auto r)
            {
                shadow.render(canvas, r, inner_rect, cell::shaders::alpha);
            });
        }
        void draw_title(si32 index, twod align, wiew utf8) //todo just render ui::core
        {
            auto& layer = layers[index];
            auto canvas = layer.canvas(true);
            auto r = canvas.area().moveto(dot_00).rotate(align) - shadow_dent;
            layer.textout(r, dot_00, 0xFF'ff'ff'ff, style::normal, utf8);
            netxs::misc::contour(canvas); // 1ms
        }
        void draw_header() { draw_title(header, { 1, -1 }, header_text); }
        void draw_footer() { draw_title(footer, { -1, 1 }, footer_text); }
        void update()
        {
            if (!reload) return;
            auto mods = reload;
            reload.reset();
                 if (mods.list == task::moved) render::present<true>();
            else if (mods)
            {
                if (mods(task::sized | task::inner ))   draw_grid();
                if (mods(task::sized | task::hover | task::grips)) draw_grips(); // 0.150 ms
                if (mods(task::sized | task::header)) draw_header();
                if (mods(task::sized | task::footer)) draw_footer();
                //if (layers[client].area.hittest(mcoord))
                //{
                //    auto fx_green = [](auto& c){ c = 0x7F'00'3f'00; };
                //    auto cursor = rect{ mcoord - (mcoord - layers[client].area.coor) % cellsz, cellsz };
                //    netxs::onrect(layers[client].canvas(), cursor, fx_green);
                //}
                render::present();
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
        void mouse_wheel(si32 delta, bool /*hz*/)
        {
            auto wheeldt = delta / 120;
            auto kb = kbs();
            //     if (kb & (hids::LCtrl | hids::LAlt)) netxs::_k2 += wheeldt > 0 ? 1 : -1; // LCtrl + Alt t +Wheel.
            //else if (kb & hids::LCtrl)                netxs::_k0 += wheeldt > 0 ? 1 : -1; // LCtrl+Wheel.
            //else if (kb & hids::anyAlt)               netxs::_k1 += wheeldt > 0 ? 1 : -1; // Alt+Wheel.
            //else if (kb & hids::RCtrl)                netxs::_k3 += wheeldt > 0 ? 1 : -1; // RCtrl+Wheel.
            //shadow = build_shadow_corner(cellsz.x);
            //reload += task::sized;
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
            else if (szgrip.zoomon)
            {
                szgrip.zoomon = faux;
                mouse_release();
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
            if (hit_grips() || szgrip.seized)
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
                    reload += task::grips;
                }
            }
            if (szgrip.zoomon && !(kb & hids::anyCtrl))
            {
                szgrip.zoomon = faux;
                mouse_release();
            }
            if (szgrip.calc(inner_rect, coord, border, dent{}, cellsz))
            {
                reload += task::grips;
            }
            if (!szgrip.seized && mbttns & bttn::left)
            {
                if (auto dxdy = coord - mcoord)
                {
                    render::moveby(dxdy);
                    reload += task::moved;
                }
            }
            mcoord = coord;
            if (!mbttns)
            {
                static auto s = testy{ faux };
                reload += s(hit_grips()) ? task::grips | task::inner
                                     : s ? task::grips : task::inner;
            }
        }
        void mouse_press(si32 button, bool pressed)
        {
            if (pressed && !mbttns) mouse_capture();
            pressed ? mbttns |= button
                    : mbttns &= ~button;
            if (!mbttns) mouse_release();
            if (!pressed & (button == bttn::right)) render::close();
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
            log("vkey: ", utf::to_hex(vkey),
                " scode: ", utf::to_hex(param.v.scancode),
                " state: ", param.v.state == 0 ? "pressed"
                          : param.v.state == 1 ? "rep"
                          : param.v.state == 3 ? "released" : "unknown");
            if (vkey == 0x1b) render::close();
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