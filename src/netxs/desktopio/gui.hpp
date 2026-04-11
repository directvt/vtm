// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

#if defined(_WIN32)
    #undef GetGlyphIndices
    #include <msctf.h>      // TSF
    #include <wrl\client.h> // ComPtr
    using Microsoft::WRL::ComPtr;
    #pragma comment(lib, "Gdi32")
    #pragma comment(lib, "ole32.lib")
    #pragma comment(lib, "oleaut32.lib")
#endif

#define ok2(...) [&](){ auto hr = __VA_ARGS__; if (hr != S_OK) log(utf::to_hex(hr), " ", #__VA_ARGS__); return hr == S_OK; }()

namespace netxs::gui
{
    namespace e2 = netxs::events::userland::e2;

    using namespace ui;
    using bits = netxs::raster<std::span<argb>>;
    using byts = std::vector<byte>;
    using gray = netxs::raster<byts>;
    using shad = netxs::misc::shadow<gray>;

    static constexpr auto debug_foci = faux;

    struct font_style
    {
        static constexpr auto regular     = 0;
        static constexpr auto italic      = 1;
        static constexpr auto bold        = 2;
        static constexpr auto bold_italic = bold | italic;
        static constexpr auto count       = 4;
    };

    struct cfg_t
    {
        using axis_vals_t = std::array<std::map<si32/*Axis tag*/, fp32>, font_style::count>;

        static constexpr auto ft_tag(auto&& tag)
        {
            return (tag[0] << 24) | (tag[1] << 16) | (tag[2] << 8) | (tag[3] << 0);
        }
        static auto ft_untag(si32 tag)
        {
            auto host_endian_tag = netxs::betoh(tag);
            auto tag_str = text{ (char*)&host_endian_tag, 4 };
            return tag_str;
        }

        si32            win_state{};    // cfg_t: .
        bool            antialiasing{}; // cfg_t: .
        span            blink_rate{};   // cfg_t: .
        twod            wincoord{};     // cfg_t: .
        twod            gridsize{};     // cfg_t: .
        si32            cell_height{};  // cfg_t: .
        std::list<text> font_names;     // cfg_t: Font family list.
        axis_vals_t     font_axes;      // cfg_t: Array of maps<axis_4byte_tag, fp32_value>.
    };

    struct layer
    {
        static constexpr auto hidden = twod{ -32000, -32000 };

        using tset = std::list<ui32>;

        arch  hdc; // layer: Layer bitmap handle.
        arch hWnd; // layer: Hosting OS window handle.
        rect prev; // layer: Last presented layer area.
        rect area; // layer: Current layer area.
        bits data; // layer: Layer bitmap.
        regs sync; // layer: Dirty region list.
        bool live; // layer: Should the layer be presented.
        tset klok; // layer: Active timer list.

        layer()
            :  hdc{},
              hWnd{},
              prev{ .coor = dot_mx },
              area{ .size = dot_11 },
              live{ faux }
        { }
        void hide() { live = faux; }
        void show() { live = true; }
        void wipe() { std::memset((void*)data.data(), 0, (sz_t)area.size.x * area.size.y * sizeof(argb)); }
        auto resized() { return area.size != prev.size; }
        template<bool Forced = faux>
        void strike(rect r)
        {
            if constexpr (Forced)
            {
                sync.clear();
                sync.push_back(r);
            }
            else
            {
                // Unoptimal rendering.
                //sync.push_back(r);
                //return;
                if (sync.empty()) sync.push_back(r);
                else
                {
                    auto& back = sync.back();
                    if (back.nearby(r) // Connected
                     || back.trim({{ -dot_mx.x / 2, r.coor.y }, { dot_mx.x, r.size.y }})) // or on the same line.
                    {
                        back.unitewith(r);
                    }
                    else sync.push_back(r);
                }
            }
        }
    };

    struct fonts
    {
        template<class T>
        using sptr = netxs::sptr<T>;
        using ft_library_sptr = sptr<std::remove_pointer_t<FT_Library>>;
        using irgb = netxs::irgb<fp32>;

        struct color_type
        {
            static constexpr auto _counter = 1 + __COUNTER__;
            static constexpr auto mono = __COUNTER__ - _counter;
            static constexpr auto sbix = __COUNTER__ - _counter;
            static constexpr auto cbdt = __COUNTER__ - _counter;
            static constexpr auto cblc = __COUNTER__ - _counter;
            static constexpr auto colr = __COUNTER__ - _counter;
            static constexpr auto svg  = __COUNTER__ - _counter;
        };
        struct axis_rec_t
        {
            text name;
            fp32 def;
            fp32 min;
            fp32 max;
        };
        struct bare_face_t
        {
            os::fs::path           file_path;
            os::fs::file_time_type file_stamp;
            si32                   face_index{};
            si32                   face_flags{};
            si32                   style_flags{};
            si32                   num_glyphs{};
            text                   family_name;
            text                   style_name;
            limits<twod>           bbox;
            ui16                   units_per_EM{};
            si16                   ascender{};
            si16                   descender{};
            si16                   height{};
            si16                   max_advance_width{};
            si16                   max_advance_height{};
            si16                   underline_position{};
            si16                   underline_thickness{};
            si16                   strikethroughPosition{};
            si16                   strikethroughThickness{};
            si16                   xHeight{};
            si16                   capHeight{};
            si16                   lineGap{};
            si32                   is_color{ fonts::color_type::mono };
            bool                   is_monospaced{};
            bool                   is_italic{};
            si16                   weight_value{};
            si32                   stretch_value{ 5 };
            std::unordered_map<si32, std::bitset<256>> unicode_ranges; // Supported characters map split by 256 codepoint blocks (from 0 up to 10FFFF/256 + 1).
            std::map<si32/*tag*/, axis_rec_t> axes_map;
            bool                   is_variable_font{};
            bool                   valid{};
            bool                   mssymbol_codepage{}; // Non Unicode code page.
            axis_rec_t             width_target{};
            axis_rec_t             weight_target{};
            axis_rec_t             italic_target{};
            std::vector<irgb>      palette; // CPAL cached palette in rgb-linear space.
            std::unordered_map<si32, imagens::docs> svg_cache; // Face specific SVG-document cache. We storing several documents for the currentColor workaround.

            auto get_weight_str() const
            {
                if (weight_target.min != weight_target.max)
                {
                    return utf::fprint("%%-%%-%%", weight_target.min, weight_target.def, weight_target.max);
                }
                else
                {
                    return utf::fprint("%%", weight_target.def);
                }
            }
            auto get_style_str() const
            {
                auto style_str = text{};
                if (is_italic)
                {
                    style_str += style_name;
                    for (auto& [tag, name_limits] : axes_map)
                    {
                        if (name_limits.min != name_limits.max)
                        {
                            style_str += utf::fprint(" %tag%[%min%-%def%-%max%]", cfg_t::ft_untag(tag), name_limits.min, name_limits.def, name_limits.max);
                        }
                    }
                }
                else
                {
                    for (auto& [tag, name_limits] : axes_map)
                    {
                        if (name_limits.min != name_limits.max)
                        {
                            style_str += utf::fprint("%tag%[%min%-%def%-%max%] ", cfg_t::ft_untag(tag), name_limits.min, name_limits.def, name_limits.max);
                        }
                    }
                    style_str += style_name;
                }
                return style_str;
            }
            auto get_width_class_id() const
            {
                auto width_class_id = ((si32)width_target.min << 16) + ((si32)width_target.def << 8) + (si32)width_target.max;
                return width_class_id;
            }
            static auto get_width_class_str(si32 width_class_id)
            {
                auto min = (width_class_id >> 16) & 0xFF;
                auto def = (width_class_id >> 8) & 0xFF;
                auto max = width_class_id & 0xFF;
                if (min != max)
                {
                    return utf::fprint("%%-%%-%%%%", min, def, max, '%');
                }
                else
                {
                    return utf::fprint("%%%%", def, '%');
                }
            }
        };
        struct loaded_font_file_t
        {
            std::vector<byte> buffer;

            explicit operator bool () { return !buffer.empty(); }

            loaded_font_file_t(os::fs::path const& path)
            {
                if (!os::io::load_file(path, buffer))
                {
                    buffer = {};
                }
            }
        };
        struct fthb_pair_t
        {
            FT_Face    ft_face;
            hb_font_t* hb_font;

            fthb_pair_t(FT_Face f, hb_font_t* h)
                : ft_face{ f },
                  hb_font{ h }
            {
                ::hb_ft_font_set_load_flags(hb_font, FT_LOAD_DEFAULT | FT_LOAD_TARGET_LIGHT);
            }
            fthb_pair_t(FT_Face f)
                : ft_face{ f },
                  hb_font{ ::hb_ft_font_create(f, nullptr) }
            {
                ::hb_ft_font_set_load_flags(hb_font, FT_LOAD_DEFAULT | FT_LOAD_TARGET_LIGHT);
            }
            ~fthb_pair_t()
            {
                ::hb_font_destroy(hb_font);
                ::FT_Done_Face(ft_face);
            }
        };
        struct face_inst_t
        {
            static constexpr auto cache_size = 4;

            struct fthb_cache_t
            {
                sptr<fthb_pair_t> fthb_pair;
                fp32              em_height{};
            };

            sptr<bare_face_t>                    bare_face_ptr;
            std::array<fthb_cache_t, cache_size> fthb_pair_cache;
            sptr<loaded_font_file_t>             loaded_file;
            fp32                                 transform{};
            fp32                                 em_height{};
            fp32                                 transform_letters{};
            fp32                                 em_height_letters{};
            fp32                                 penalty{};
            fp2d                                 actual_sz{};
            fp2d                                 base_line{};
            rect                                 underline{}; // face_inst_t: Underline rectangle block within the cell.
            rect                                 doubline1{}; // face_inst_t: The first line of the double underline: at the top of the rect.
            rect                                 doubline2{}; // face_inst_t: The second line of the double underline: at the bottom.
            rect                                 strikeout{}; // face_inst_t: Strikethrough rectangle block within the cell.
            rect                                 overline{};  // face_inst_t: Overline rectangle block within the cell.
            rect                                 dashline{};  // face_inst_t: Dashed underline rectangle block within the cell.
            rect                                 wavyline{};  // face_inst_t: Wavy underline outer rectangle block within the cell.

            void load_palette()
            {
                auto ft_face = fthb_pair_cache[0].fthb_pair->ft_face;
                auto data = FT_Palette_Data{};
                auto ft_palette = (FT_Color*)nullptr;
                if (FT_Err_Ok == ::FT_Palette_Data_Get(ft_face, &data))
                if (FT_Err_Ok == ::FT_Palette_Select(ft_face, 0, &ft_palette)) // Take the fisrt existing palette (index 0).
                {
                    auto& palette = bare_face_ptr->palette;
                    palette.resize(data.num_palette_entries);
                    for (auto i = 0u; i < data.num_palette_entries; ++i)
                    {
                        auto& c = ft_palette[i];
                        palette[i] = irgb::nonpma_srgb_to_pma_linear(argb{ c.red, c.green, c.blue, c.alpha });
                    }
                }
            }
            auto select_face_by_em_height(fp32 custom_height, si32 /*cache_index*/)
            {
                //todo ?implement caching
                auto& slot = fthb_pair_cache[0];
                auto& inst = *(slot.fthb_pair);
                if (slot.em_height != custom_height)
                {
                    slot.em_height = custom_height;
                    ::FT_Set_Char_Size(inst.ft_face, 0, (si32)(custom_height * 64.0f + 0.5f), 0, 0); // Too expensive.
                    ::hb_ft_font_changed(inst.hb_font);                                              //
                }
                return std::pair{ inst.hb_font, inst.ft_face };
            }
            auto load_from_file(fonts& fcache, auto& family_ref, si32 style_id)
            {
                auto& file_path = bare_face_ptr->file_path;
                auto  utf8_path = file_path.generic_string();
                auto& loaded_file_ptr = fcache.loaded_files[utf8_path];
                if (!loaded_file_ptr)
                {
                    loaded_file_ptr = ptr::shared(loaded_font_file_t{ file_path });
                }
                loaded_file = loaded_file_ptr;
                auto& file_buff = *(loaded_file);
                auto raw_face = (FT_Face)nullptr;
                if ((!file_buff && !os::io::load_file(file_path, file_buff.buffer))
                    || FT_Err_Ok != ::FT_New_Memory_Face(fcache.ft_library.get(), file_buff.buffer.data(), (FT_Long)file_buff.buffer.size(), bare_face_ptr->face_index, &raw_face)) // Read a whole font/collection file (relatively slow, e.g. with 30Mb font files).
                {
                    log("%%Failed to load font family file '%family_name%' (style_id=%%): %filename%", prompt::gui, family_ref.family_name, style_id, utf8_path);
                    bare_face_ptr->valid = faux;
                }
                else
                {
                    log("%%Using font family '%family_name%': %iscolor%, index %index%, style=%%", prompt::gui, family_ref.family_name, family_ref.is_color != fonts::color_type::mono ? "color" : "monochromatic", fcache.font_fallback.size(), style_id);
                    // Apply axes.
                    if (bare_face_ptr->is_variable_font)
                    {
                        auto amaster = (FT_MM_Var*)nullptr;
                        if (FT_Err_Ok == ::FT_Get_MM_Var(raw_face, &amaster))
                        {
                            auto coords = std::vector<FT_Fixed>(amaster->num_axis);
                            auto& target_axes = fcache.primary_axes[style_id];
                            for (auto i = 0u; i < amaster->num_axis; i++)
                            {
                                auto& axis = amaster->axis[i];
                                auto final_value = axis.def / 65536.0f;
                                if (auto it = target_axes.find(axis.tag); it != target_axes.end()) // Use if it is.
                                {
                                    final_value = std::clamp(it->second, axis.minimum / 65536.0f, axis.maximum / 65536.0f);
                                }
                                coords[i] = (FT_Fixed)(final_value * 65536.0f);
                            }
                            ::FT_Set_Var_Design_Coordinates(raw_face, amaster->num_axis, coords.data());
                            ::FT_Set_Char_Size(raw_face, 0, raw_face->units_per_EM * 64, 72, 72); // Trigger to apply axes. "Dummy" scale to force metrics refresh (1 char unit = 1 font unit)
                            ::FT_Done_MM_Var(fcache.ft_library.get(), amaster);
                        }
                    }
                    fthb_pair_cache[0].fthb_pair = ptr::shared<fthb_pair_t>(raw_face);
                    if (bare_face_ptr->is_color != fonts::color_type::mono) // Cache palette.
                    {
                        load_palette();
                    }
                    return true;
                }
                return faux;
            }
        };
        struct font_family_t
        {
            view                                       family_name;     // font_family_t: .
            os::fs::file_time_type                     file_stamp{};    // font_family_t: .
            bool                                       is_monospaced{}; // font_family_t: .
            si32                                       is_color{};      // font_family_t: .
            bool                                       is_sorted{};     // font_family_t: .
            bool                                       is_fixed{};      // font_family_t: The font family is specified in configuration.
            bool                                       is_invalid{};    // font_family_t: The font family failed to load (e.g., the font file became inaccessible).
            bool                                       non_square{};    // font_family_t: width/height != 1.
            std::unordered_map<si32, std::bitset<256>> unicode_ranges;  // font_family_t: Supported characters map split by 256 codepoint blocks (from 0 up to 10FFFF/256 + 1).
            std::vector<face_inst_t>                   bare_face_list;  // font_family_t: A full font face list for the specific family (sorted by asc weight then file_stamp).

            auto first_class() const
            {
                return is_fixed && (is_monospaced || is_color);
            }
        };
        struct font_face_t
        {
            fonts&                                                  fcache;
            font_family_t&                                          family_ref;
            std::array<std::vector<face_inst_t>, font_style::count> sorted_face_list; // Sorted list of faces for selecting regular/italic/bold/bold_italic faces.

            fp32 base_descent{ 1.f };
            fp32 base_ascent{ 1.f };
            fp2d base_underline{};
            fp2d base_strikeout{};
            fp2d base_overline{};
            si32 base_emheight{};
            si32 base_x_height{ 1 };
            fp2d facesize{ 1.f, 1.f }; // Typeface cell size.
            fp32 ratio{};
            si32 color{ fonts::color_type::mono };
            bool fixed{ faux }; // Preserve specified font order.
            bool monospaced{ faux };
            text font_name;

            font_face_t() = default;
            font_face_t(font_face_t&&) = default;
            font_face_t(fonts& fcache, font_family_t& family_rec)
                : fcache{ fcache },
                  family_ref{ family_rec },
                  fixed{ true }
            {
                load_face();
            }
            font_face_t(fonts& fcache, font_family_t& family_rec, twod cellsz, bool isbase)
                : fcache{ fcache },
                  family_ref{ family_rec },
                  fixed{ isbase }
            {
                load_face();
                if (!isbase) recalc_metrics(cellsz, isbase);
            }

            void sort_faces()
            {
                for (auto style_id : { font_style::regular, font_style::italic, font_style::bold, font_style::bold_italic })
                {
                    auto& sorted_list = sorted_face_list[style_id];
                    auto& target_axes = fcache.primary_axes[style_id];
                    auto target_ital = target_axes[cfg_t::ft_tag("ital")];
                    auto target_wdth = target_axes[cfg_t::ft_tag("wdth")];
                    auto target_wght = target_axes[cfg_t::ft_tag("wght")];
                    sorted_list = family_ref.bare_face_list;
                    auto calc_penalty = [&](auto& bare_face_rec) // CSS Level 4 matching rules
                    {
                        auto& rec = *(bare_face_rec.bare_face_ptr);
                        auto& p = bare_face_rec.penalty;
                        p = {};
                        // WIDTH. Match width first.
                        if (target_wdth < rec.width_target.min || target_wdth > rec.width_target.max)
                        {
                            auto closest_wdth = std::clamp(target_wdth, rec.width_target.min, rec.width_target.max);
                            p += std::abs(target_wdth - closest_wdth) * 100.0f;
                            p += 10000.0;
                        }
                        // ITALIC. If 'italic' requested, 'italic' faces are preferred over 'normal'.
                        auto req_it = target_ital > 0.5f;
                        auto has_it = rec.italic_target.max > 0.5f;
                        if (req_it != has_it)
                        {
                            p += 2000.0f; // Significant jump to separate styles.
                        }
                        //todo OBLIQUE.
                        // WEIGHT.
                        auto min_w = rec.weight_target.min;
                        auto max_w = rec.weight_target.max;
                        if (target_wght < min_w || target_wght > max_w)
                        {
                            auto diff = 0.0f;
                            if (target_wght >= 400.0f && target_wght <= 500.0f) // Weight matching rules:
                            {
                                diff = min_w > target_wght && min_w <= 500.0f ? min_w - target_wght            // 1. Look at [target...500]
                                     : min_w > 500.0f                         ? min_w - target_wght + 500.0f   // 2. Look at (500...1000] (Heavier)
                                                                              : target_wght - max_w + 1000.0f; // 3. Look at [1...target) (Lighter)
                            }
                            else if (target_wght < 400.0f)
                            {
                                diff = max_w < target_wght ? target_wght - max_w           // 1. Look at [1...target] (Lighter)
                                                           : min_w - target_wght + 500.0f; // 2. Look at (target...1000] (Heavier)
                            }
                            else // target_wght > 500.0f
                            {
                                diff = min_w > target_wght ? min_w - target_wght           // 1. Look at [target...1000] (Heavier)
                                                           : target_wght - max_w + 500.0f; // 2. Look at [1...target) (Lighter)
                            }
                            p += diff;
                        }
                        if (!rec.is_variable_font) // 4. VARIABLE VS STATIC. CSS implicitly prefers variable fonts (exact match).
                        {
                            p += 0.01f;
                        }
                    };
                    for (auto& bare_face_rec : sorted_list)
                    {
                        calc_penalty(bare_face_rec);
                    }
                    std::ranges::sort(sorted_list, std::ranges::less{}, &face_inst_t::penalty);
                }
                //log("Sort family '%%'", family_ref.family_name);
                //for (auto style_id : { font_style::regular, font_style::bold, font_style::italic, font_style::bold_italic })
                //{
                //    log("style_id=", style_id);
                //    auto& sorted_list = sorted_face_list[style_id];
                //    for (auto& bare_face_rec : sorted_list)
                //    {
                //        log("  face: weight=%% stretch=%% italic=%%", bare_face_rec.bare_face_ptr->weight_value, bare_face_rec.bare_face_ptr->stretch_value, bare_face_rec.bare_face_ptr->is_italic);
                //    }
                //}
            }
            void load_face()
            {
                sort_faces();
                auto get = [&](si32 style_id) // Load first face from the sorted list.
                {
                    auto has_broken_fonts = faux;
                    auto& sorted_list = sorted_face_list[style_id];
                    for (auto& bare_face_rec : sorted_list)
                    {
                        if (bare_face_rec.bare_face_ptr->valid
                         && bare_face_rec.load_from_file(fcache, family_ref, style_id))
                        {
                            break; // Load only the first available face (lazy).
                        }
                        else
                        {
                            has_broken_fonts = true;
                        }
                    }
                    if (has_broken_fonts) // Remove broken records from the sorted list.
                    {
                        std::erase_if(sorted_list, [](auto& bare_face_rec){ return !bare_face_rec.bare_face_ptr->valid; });
                    }
                };
                get(font_style::regular    );
                get(font_style::italic     );
                get(font_style::bold       );
                get(font_style::bold_italic);
                //todo ?Get all localized family names for the font family, indexed by locale name
                font_name = family_ref.family_name;
                auto& sorted_list = sorted_face_list[font_style::regular];
                if (sorted_list.size() && sorted_list.front().fthb_pair_cache[0].fthb_pair)
                {
                    auto& regular_face_rec = sorted_list.front();
                    auto& regular_faceinst = regular_face_rec.fthb_pair_cache[0].fthb_pair->ft_face;
                    auto ascender               = (fp32)regular_faceinst->ascender;
                    auto descender              = (fp32)-regular_faceinst->descender; // DWrite has positive descender, ft - negative.
                    auto lineGap                = (fp32)regular_face_rec.bare_face_ptr->lineGap;
                    auto underline_thickness    = (fp32)regular_faceinst->underline_thickness;
                    auto underline_position     = (fp32)regular_faceinst->underline_position;
                    auto strikethroughPosition  = (fp32)regular_face_rec.bare_face_ptr->strikethroughPosition;
                    auto strikethroughThickness = (fp32)regular_face_rec.bare_face_ptr->strikethroughThickness;
                    auto capHeight              = (fp32)regular_face_rec.bare_face_ptr->capHeight;
                    auto xHeight                = (si32)regular_face_rec.bare_face_ptr->xHeight;
                    base_underline = { underline_position, underline_thickness };
                    base_strikeout = { strikethroughPosition, strikethroughThickness };
                    base_overline  = { std::min(ascender, capHeight - underline_position), underline_thickness };
                    base_emheight  = regular_face_rec.bare_face_ptr->units_per_EM;
                    base_x_height  = std::max(1, xHeight);
                    base_ascent    = std::max(1.f, ascender  + lineGap / 2.0f);
                    base_descent   = std::max(1.f, descender + lineGap / 2.0f);
                    color          = regular_face_rec.bare_face_ptr->is_color;
                    monospaced     = regular_face_rec.bare_face_ptr->is_monospaced;
                    facesize.y     = std::max(2.f, ascender + descender + lineGap);
                    facesize.x     = facesize.y / 2.f;
                    // Take metrics for "x" or ".notdef" in case of missing 'x'. Note: ".notdef" is double sized ("x" is narrow) in CJK fonts.
                    //auto code_point = ui32{ 'x' };
                    // Test W for monospaced and test U for proportional font.
                    auto code_point = monospaced ? 'W' : 'U'; // U is approximately half an emoji square in the Segoe Emoji font. W and M may be cut in size. This is a compromise for proportional fonts. Otherwise, emoji become too small.
                    if (fonts::load_char_metrics(regular_faceinst, code_point))
                    {
                        auto& glyph_metrics = regular_faceinst->glyph->metrics;
                        if (glyph_metrics.horiAdvance)
                        {
                            facesize.x = (fp32)glyph_metrics.horiAdvance;
                        }
                    }
                    ratio = facesize.x / facesize.y;
                }
            }
            void get_common_widths(bool& proportional, ui32& normal_width, ui32& italic_width)
            {
                auto advanceWidths = std::array<ui32, font_style::count>{};
                auto widths = std::array<ui32, font_style::count>{};
                for (auto style_id : { font_style::regular, font_style::italic })
                {
                    auto& sorted_list  = sorted_face_list[style_id];
                    if (sorted_list.size() && sorted_list.front().fthb_pair_cache[0].fthb_pair)
                    {
                        auto& faceinst = sorted_list.front().fthb_pair_cache[0].fthb_pair->ft_face;
                        if (fonts::load_char_metrics(faceinst, utfx{'M'}))
                        {
                            auto& glyph_metrics = faceinst->glyph->metrics;
                            if (glyph_metrics.horiAdvance)
                            {
                                advanceWidths[style_id] = (ui32)glyph_metrics.horiAdvance;
                                widths[style_id] = (ui32)(glyph_metrics.horiBearingX + glyph_metrics.width);
                            }
                        }
                    }
                }
                proportional = advanceWidths[font_style::regular] != (ui32)facesize.x;
                normal_width = widths[font_style::regular];
                italic_width = widths[font_style::italic];
            }
            void recalc_metrics(twod& cellsize, bool isbase)
            {
                //log("ft: font_name=", font_name, "\tcellsize=", cellsize, "\tbase_ascent=", base_ascent,
                //    "\tbase_descent=", base_descent, "\tbase_x_height=", base_x_height,
                //    "\tbase_emheight=", base_emheight, "\tfacesize=", facesize);
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
                //log("  ft: font_name=", font_name, "\tasc=", base_ascent, "\tdes=", base_descent, "\tem=", base_emheight, "\tbasline=", b2, "\tdy=", transform, "\tk0=", k0, "\tm1=", m1, "\tm2=", m2);
                for (auto& sorted_list : sorted_face_list) if (sorted_list.size())
                {
                    auto& f = sorted_list.front();
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
                for (auto s : { font_style::regular, font_style::bold }) if (sorted_face_list[s].size())
                {
                    auto& f = sorted_face_list[s].front();
                    f.transform = transform;
                    f.em_height = em_height;
                    f.actual_sz = actual_sz;
                    f.transform_letters = transform_letters;
                    f.em_height_letters = em_height_letters;
                }
                // Detect right bearing delta for italics.
                auto proportional = bool{};
                auto normal_width = ui32{};
                auto italic_width = ui32{};
                get_common_widths(proportional, normal_width, italic_width);
                auto w = proportional && normal_width ? (fp32)normal_width : facesize.x;
                auto italic_w = w + (italic_width - normal_width);
                auto k = italic_w ? w / italic_w : 1.f;
                transform *= k;
                em_height *= k;
                transform_letters = std::floor(base_x_height * transform) / base_x_height; // Respect x-height.
                em_height_letters = base_emheight * transform_letters;
                actual_sz *= k;
                for (auto s : { font_style::italic, font_style::bold_italic }) if (sorted_face_list[s].size())
                {
                    auto& f = sorted_face_list[s].front();
                    f.transform = transform;
                    f.em_height = em_height;
                    f.actual_sz = actual_sz;
                    f.transform_letters = transform_letters;
                    f.em_height_letters = em_height_letters;
                }
            }
            auto& select_font_face(std::span<const utfx> codepoints, si32 style_id)
            {
                auto has_broken_fonts = faux;
                auto& sorted_list = sorted_face_list[style_id];
                auto inst_face = std::reference_wrapper{ sorted_list.front() };
                for (auto& bare_face_rec : sorted_list)
                {
                    if (fonts::hittest(bare_face_rec.bare_face_ptr->unicode_ranges, codepoints))
                    {
                        if (!bare_face_rec.fthb_pair_cache[0].fthb_pair && !bare_face_rec.load_from_file(fcache, family_ref, style_id))
                        {
                            has_broken_fonts = true;
                        }
                        else
                        {
                            inst_face = bare_face_rec;
                            break;
                        }
                    }
                }
                if (has_broken_fonts) // Remove broken records from the sorted list.
                {
                    std::erase_if(sorted_list, [](auto& bare_face_rec){ return !bare_face_rec.bare_face_ptr->valid; });
                }
                return inst_face.get();
            }
        };
        struct shaper
        {
            struct step_t
            {
                ui32 index;
                fp32 width;
                fp2d align;
                si32 color;
                rect b_box; // Bounding box in pixels.
            };

            fonts&              fcache;
            hb_font_t*          hb_font{};
            FT_Face             ft_face{};
            hb_buffer_t*        hb_buffer;
            ui32                glyf_count{};
            sptr<bare_face_t>   bare_face_ptr;
            std::vector<step_t> glyphs;
            si32                colored{};

            shaper(fonts& fcache)
                : fcache{ fcache },
                  hb_buffer{ ::hb_buffer_create() }
            {
                ::hb_buffer_set_language(hb_buffer, fcache.os_locale);
            }
            ~shaper()
            {
                ::hb_buffer_destroy(hb_buffer);
            }

            auto generate_glyph_run(std::vector<utfx>& codepoints, auto script, bool is_rtl, fp32 transform, bool is_monospaced, fp32 grid_step)
            {
                ::hb_buffer_add_codepoints(hb_buffer, codepoints.data(), (si32)codepoints.size(), 0, (si32)codepoints.size());
                ::hb_buffer_set_direction(hb_buffer, is_rtl ? HB_DIRECTION_RTL : HB_DIRECTION_LTR);
                ::hb_buffer_set_script(hb_buffer, script);
                ::hb_shape(hb_font, hb_buffer, nullptr, 0);
                auto info = ::hb_buffer_get_glyph_infos(hb_buffer, &glyf_count);
                auto pos  = ::hb_buffer_get_glyph_positions(hb_buffer, &glyf_count);
                glyphs.resize(glyf_count);
                for (auto i = 0u; i < glyf_count; i++)
                {
                    auto& glyph = glyphs[i];
                    glyph.index = info[i].codepoint;
                    glyph.width = pos[i].x_advance / 64.0f;                             // 26.6 format.
                    glyph.align = { pos[i].x_offset / 64.0f, pos[i].y_offset / 64.0f }; //
                }
                ::hb_buffer_clear_contents(hb_buffer);
                auto length = 0.0f;
                if (is_monospaced)
                {
                    for (auto& glyph : glyphs)
                    {
                        auto& w = glyph.width;
                        if (w) w = grid_step; // Fit to our grid.
                        length += w;
                    }
                }
                else // Render glyphs as is.
                {
                    auto revpad = 0.0f;
                    auto penpos = 0.0f;
                    for (auto& glyph : glyphs)
                    {
                        if (FT_Err_Ok == ::FT_Load_Glyph(ft_face, glyph.index, FT_LOAD_NO_SCALE))
                        {
                            auto& m = ft_face->glyph->metrics;
                            auto w = (fp32)m.horiAdvance;
                            auto r = (fp32)m.horiBearingX;
                            auto f = w - (r + (fp32)m.width);
                            if (is_rtl) std::swap(f, r);
                            auto fwd_bearing = (w - f) * transform;
                            auto rev_bearing = -r * transform;
                            auto glyphpos = penpos + glyph.align.x; // advanceOffset
                            revpad = std::min(revpad, glyphpos - rev_bearing);
                            length = std::max(length, glyphpos + fwd_bearing);
                        }
                        penpos += glyph.width;
                    }
                    length = std::max({ length, penpos, penpos - revpad });
                }
                return length;
            }
        };

        static bool hittest_codepoint(std::unordered_map<si32, std::bitset<256>> const& unicode_ranges, utfx codepoint)
        {
            auto block = codepoint / 256;
            auto point = codepoint % 256;
            auto exist = faux;
            if (auto iter = unicode_ranges.find(block); iter != unicode_ranges.end())
            {
                exist = iter->second.test(point);
            }
            return exist;
        }
        static bool hittest(std::unordered_map<si32, std::bitset<256>> const& unicode_ranges, std::span<const utfx> codepoints)
        {
            for (auto codepoint : codepoints)
            {
                if (codepoint != 0x200B) // Exclude ZWSP.
                if (!fonts::hittest_codepoint(unicode_ranges, codepoint))
                {
                    return faux;
                }
            }
            return true;
        }
        static auto make_ft_library()
        {
            auto library = (FT_Library)nullptr;
            if (FT_Err_Ok != ::FT_Init_FreeType(&library))
            {
                log("%%Could not initialize FreeType library", prompt::gui);
                std::terminate();
            }
            return ft_library_sptr(library, [](auto l){ if (l) ::FT_Done_FreeType(l); });
        }
        static bool load_char_metrics(FT_Face face, utfx cp)
        {
            auto idx = ::FT_Get_Char_Index(face, cp);
            auto ok = idx > 0 && FT_Err_Ok == ::FT_Load_Glyph(face, idx, FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING); // FT_LOAD_NO_SCALE: use raw Font Units, FT_LOAD_NO_HINTING: eliminate the influence of hinting on geometry.
            return ok;
        }
        static auto has_sfnt_table(FT_Face face, ui32 tag)
        {
            auto l = 0ul;
            return FT_Err_Ok == ::FT_Load_Sfnt_Table(face, tag, 0, nullptr, &l);
        };
        static auto get_char_height(FT_Face face, utfx cp, auto& char_height, auto default_height)
        {
            if (fonts::load_char_metrics(face, cp))
            {
                auto tmp_value = face->glyph->metrics.horiBearingY;
                char_height = (std::remove_reference_t<decltype(char_height)>)tmp_value;
            }
            else
            {
                auto tmp_value = default_height;
                char_height = (std::remove_reference_t<decltype(char_height)>)tmp_value;
            }
        }

        void set_cellsz(si32 cell_height)
        {
            cellsize = { 1, std::clamp(cell_height, 2, 256) };
            auto base_font = true;
            for (auto& f : font_fallback)
            {
                f.recalc_metrics(cellsize, std::exchange(base_font, faux));
            }
            if (font_fallback.size()) // Keep the same *line positions for all fonts.
            if (auto& sorted_list = font_fallback.front().sorted_face_list[font_style::regular]; sorted_list.size())
            {
                auto& f = sorted_list.front();
                underline = f.underline;
                strikeout = f.strikeout;
                doubline1 = f.doubline1;
                doubline2 = f.doubline2;
                overline  = f.overline;
                dashline  = f.dashline;
                wavyline  = f.wavyline;
                //todo implement it via realtime request (for remotes)
                //os::dtvt::fontnm = fallback.front().font_name;
                //os::dtvt::fontsz = cellsize;
            }
            //log("%%Set cell size: ", prompt::gui, cellsize);
        }
        void set_fonts(std::list<text>& family_names, cfg_t::axis_vals_t& font_axes, bool fresh = true)
        {
            primary_axes = font_axes;
            set_fonts(family_names, fresh);
        }
        void set_fonts(std::list<text>& family_names, bool fresh = true)
        {
            families = family_names;
            font_fallback.clear();
            if (!fresh) // Clear the "loaded" and "fixed" flags.
            {
                for (font_family_t& family_rec : font_index)
                {
                    family_rec.is_sorted = faux;
                    family_rec.is_fixed = faux;
                }
            }
            for (auto& family_utf8 : family_names) // Sort faces in families.
            {
                auto iter = font_map.find(family_utf8);
                if (iter != font_map.end())
                {
                    auto& family_rec = iter->second;
                    if (!family_rec.is_sorted) // Skip duplicates.
                    {
                        font_fallback.emplace_back(*this, family_rec);
                        family_rec.is_sorted = true;
                        family_rec.is_fixed = true;
                    }
                }
                else
                {
                    log("%%Font '%fontname%' is not found in the system", prompt::gui, family_utf8);
                }
            }
        }
        auto& find_family(std::span<const utfx> codepoints, bool force_mono = faux)
        {
            for (auto& f : font_fallback) if ( f.family_ref.first_class() && fonts::hittest(f.family_ref.unicode_ranges, codepoints)) return f;
            for (auto& f : font_fallback) if (!f.family_ref.first_class() && fonts::hittest(f.family_ref.unicode_ranges, codepoints)) return f;
            auto try_font = [&](font_family_t& family_rec, bool test)
            {
                auto hit = !test || fonts::hittest(family_rec.unicode_ranges, codepoints);
                if (hit)
                {
                    auto is_primary = font_fallback.empty();
                    font_fallback.emplace_back(*this, family_rec, cellsize, is_primary);
                    family_rec.is_sorted = true;
                }
                return hit;
            };
            if (force_mono) for (font_family_t& family_rec : font_index)
            {
                if (family_rec.is_monospaced && !family_rec.is_invalid && !family_rec.is_sorted && try_font(family_rec, true))
                {
                    return font_fallback.back();
                }
            }
            else for (font_family_t& family_rec : font_index)
            {
                if (!family_rec.is_invalid && !family_rec.is_sorted && try_font(family_rec, true))
                {
                    return font_fallback.back();
                }
            }
            if (font_fallback.size())
            {
                return font_fallback.back();
            }
            for (font_family_t& family_rec : font_index) // Take the first font found in the system if no fonts loaded yet.
            {
                if (!family_rec.is_invalid && try_font(family_rec, faux))
                {
                    return font_fallback.back();
                }
            }
            log("%%No fonts found in the system", prompt::gui);
            std::terminate();
        }
        void log_fonts(bool show_ranges = faux)
        {
            // map:                         FAMILY_NAME1                           |          FAMILY_NAME2
            // map:        WIDTH_CLASS 75%       |           WIDTH_CLASS 100%      |          WIDTH_CLASS 100%
            // [2]:  NORMAL     |     ITALIC     |     NORMAL     |     ITALIC     |    NORMAL     |     ITALIC
            // vec:  fnt400     |     fnt400     |     fnt400     |     fnt400     |    fnt400     |     fnt400
            //       fnt700     |     fnt700     |     fnt700     |     fnt700     |    fnt700     |     fnt700
            //
            // font_map:
            //   family_name : .file_stamp
            //                 .is_monospaced
            //                 .is_color
            //                 .unicode_ranges[block]
            //                 .fonts[width_class] : .normal : vec<bare_face_t>
            //                                       .italic : vec<bare_face_t>
            struct width_group_t
            {
                std::vector<sptr<bare_face_t>> normal;
                std::vector<sptr<bare_face_t>> italic;
            };

            log<faux>("\x1b_lua: align_mode=terminal.LineAlignMode(); terminal.LineAlignMode(2)\x1b\\");
            log<faux>("\x1b_lua: wrap_mode=terminal.LineWrapMode(); terminal.LineWrapMode(0)\x1b\\");
            log<faux>("\x1b_lua: code_page=terminal.CodePage(); terminal.CodePage(65001)\x1b\\");
            auto chars = text(4096, '\0');
            auto width_groups = std::map<si32, width_group_t>{}; // Ordered map of <width_class_id, width_group_t<vec<normal>, vec<italic>>>
            for (auto& family_rec_ref : font_index)
            {
                auto& family_rec = family_rec_ref.get();
                auto& family_name = family_rec.family_name;
                log("\n%is_monospaced% \x1b[7m %name% \x1b[m%is_color% ", family_rec.is_monospaced ? " \x1b[92mmonospaced\x1b[m" : "           ", family_name,
                                                                          family_rec.is_color      ? " \x1b[92mcolor\x1b[m     " : "           ");
                for (auto& bare_face_rec : family_rec.bare_face_list) // Split by width class and then by normal/italic.
                {
                    auto& rec = *(bare_face_rec.bare_face_ptr);
                    auto& width_group = width_groups[rec.get_width_class_id()];
                    auto& dest_bucket = rec.is_italic ? width_group.italic : width_group.normal;
                    dest_bucket.push_back(bare_face_rec.bare_face_ptr);
                }
                for (auto& [width_class_id, width_group] : width_groups)
                {
                    auto max_stylename = std::array<size_t, 2>{};
                    auto max_file_name = std::array<size_t, 2>{};
                    auto max_file_path = std::array<size_t, 2>{};
                    auto max_weightstr = std::array<size_t, 2>{};
                    for (auto type : { faux, true })
                    {
                        auto& rec_list = type ? width_group.italic : width_group.normal;
                        for (auto& rec_ptr : rec_list)
                        {
                            auto& rec = *rec_ptr;
                            max_weightstr[type] = std::max(max_weightstr[type], rec.get_weight_str().size());
                            max_stylename[type] = std::max(max_stylename[type], rec.get_style_str().size());
                            max_file_path[type] = std::max(max_file_path[type], rec.file_path.parent_path().generic_string().size());
                            max_file_name[type] = std::max(max_file_name[type], rec.file_path.filename().generic_string().size());
                        }
                    }
                    auto stamp_len = 28;
                    //              "stamp_len [max_file_path] max_file_name max_stylename  max_weightstr "|" max_weightstr  max_stylename max_file_name [max_file_path] stamp_len"
                    auto left_side = stamp_len + 2 + max_file_path[0] + 2 + max_file_name[0] + 1 + max_stylename[0] + 2 + max_weightstr[0] + 1;
                    auto rght_side = stamp_len + 2 + max_file_path[1] + 2 + max_file_name[1] + 1 + max_stylename[1] + 2 + max_weightstr[1] + 1;
                    auto max_side = std::max(left_side, rght_side);
                    auto mount = text(2 * max_side + 1, ' ') + "\r";
                    auto pad = text(max_side - left_side, ' ');
                    auto ratio = [&]
                    {
                        auto& rec = *(width_group.normal.size() ? width_group.normal.front() : width_group.italic.front());
                        auto r = rec.max_advance_width / fp32(rec.ascender - rec.descender + rec.lineGap);
                        return r;
                    };
                    log("\x1b[33mFont Width: %stretch_value%\x1b[m", bare_face_t::get_width_class_str(width_class_id));
                    if (family_rec.is_monospaced)
                    {
                        log("cell ratio (w/h): %ratio%", ratio());
                    }
                    auto iter_normal = width_group.normal.begin();
                    auto iter_italic = width_group.italic.begin();
                    auto weight_word = "Weight "sv;
                    auto letterindex = 0_sz;
                    auto get_weight_next_char = [&]{ return utf::fprint("\x1b[7m%W%\x1b[27m", weight_word[std::min(weight_word.size() - 1, letterindex++)]); };
                    log("\x1b[4m Normal \x1b[24m%W%\x1b[4m Italic \x1b[m", get_weight_next_char());
                    while (iter_normal != width_group.normal.end() || iter_italic != width_group.italic.end())
                    {
                        auto n0 = iter_normal != width_group.normal.end();
                        auto i0 = iter_italic != width_group.italic.end();
                        auto n = n0 && (!i0 || (*iter_italic)->weight_value >= (*iter_normal)->weight_value || (*iter_normal)->axes_map.find(cfg_t::ft_tag("wght")) != (*iter_normal)->axes_map.end());
                        auto i = i0 && (!n0 || (*iter_italic)->weight_value <= (*iter_normal)->weight_value || (*iter_italic)->axes_map.find(cfg_t::ft_tag("wght")) != (*iter_italic)->axes_map.end());
                        auto fn = utf::adjust(n ? utf::fprint("%stamp% [%dir%] %file%", (*iter_normal)->file_stamp, (*iter_normal)->file_path.parent_path().generic_string(), utf::adjust((*iter_normal)->file_path.filename().string(), max_file_name[0], ' ', true)) : "", stamp_len + 2 + max_file_path[0] + 2 + max_file_name[0], ' ', true);
                        auto fi = utf::adjust(i ? utf::fprint("%file% [%dir%] %stamp%", utf::adjust((*iter_italic)->file_path.filename().string(), max_file_name[1], ' '), (*iter_italic)->file_path.parent_path().generic_string(), (*iter_italic)->file_stamp)       : "", stamp_len + 2 + max_file_path[1] + 2 + max_file_name[1], ' ', faux);
                        auto sn = utf::adjust(n ? (*iter_normal)->get_style_str() : "", max_stylename[0], ' ', true);
                        auto si = utf::adjust(i ? (*iter_italic)->get_style_str() : "", max_stylename[1], ' ', faux);
                        log("%%%pad%%fn% %sn%  %normal_weight% %W% %italic_weight%  %si% %fi%", mount, pad,
                            fn, sn, utf::adjust(n ? (*iter_normal++)->get_weight_str() : "", max_weightstr[0], ' ', true),
                                    get_weight_next_char(),
                                    utf::adjust(i ? (*iter_italic++)->get_weight_str() : "", max_weightstr[1], ' ', faux), si, fi);
                    }
                    log("%%", get_weight_next_char());
                }
                width_groups.clear();
                if (!show_ranges) continue;
                log<faux>("\x1b_lua: terminal.LineAlignMode(0)\x1b\\"); // Set left alignment.
                log(" Unicode ranges:");
                auto& unicode_ranges = family_rec.unicode_ranges;
                //todo sort unicode_ranges
                for (auto& [block, bit_set]: unicode_ranges)
                {
                    auto first = block * 256;
                    auto last = first + 255;
                    chars.clear();
                    for (auto codepoint = first; codepoint <= last; codepoint++)
                    {
                        auto& char_props = netxs::unidata::select(codepoint);
                        if (bit_set.test(codepoint - first) && !char_props.is_cmd())
                        {
                            if (char_props.ucwidth != 2) chars += utf::matrix::stx;
                            auto dotted_circle = netxs::unidata::select(0x25CC);
                            if (dotted_circle.allied(char_props)) // Use dotted circle '◌' (U+25CC) as a placeholder for diacritics-like.
                            {
                                chars += "\u25CC";
                                utf::to_utf_from_code(codepoint, chars);
                            }
                            else // Show the characters as is.
                            {
                                utf::to_utf_from_code(codepoint, chars);
                            }
                            if (char_props.ucwidth != 2) chars += utf::matrix::vss<21>;
                        }
                        else
                        {
                            chars += "\x1b[31m・\x1b[m";
                        }
                    }
                    log("  %%-%%: %chars%", utf::to_hex(first).substr(2), utf::to_hex(last).substr(2), chars);
                }
                log<faux>("\x1b_lua: terminal.LineAlignMode(2)\x1b\\"); // Back to centered alignment.
            }
            log<faux>("\x1b_lua: terminal.LineAlignMode(align_mode)\x1b\\");
            log<faux>("\x1b_lua: terminal.LineWrapMode(wrap_mode)\x1b\\");
            log<faux>("\x1b_lua: terminal.CodePage(code_page)\x1b\\\n");
        }

        ft_library_sptr                                    ft_library;    // fonts: FreeType library instance.
        hb_language_t                                      os_locale;     // fonts: User locale.
        shaper                                             font_shaper;   // fonts: Font shaper.
        std::map<text, font_family_t>                      font_map;      // fonts: Map of available font families sorted by family name.
        std::vector<std::reference_wrapper<font_family_t>> font_index;    // fonts: Index of available font families ordered by filestamp.
        std::vector<font_face_t>                           font_fallback; // fonts: Fallback font list.
        utf::unordered_map<text, sptr<loaded_font_file_t>> loaded_files;  // fonts: Map of loaded font files by file paths (path <-> blob).
        cfg_t::axis_vals_t                                 primary_axes;  // fonts: Map of the primary font axes (4byte_axis_tag <-> values).
        std::list<text>                                    families;      // fonts: List of primary families.

        twod cellsize;  // fonts: Terminal cell size in pixels.
        rect underline; // fonts: Single underline rectangle block within the cell.
        rect doubline1; // fonts: The first line of the double underline: at the top of the rect.
        rect doubline2; // fonts: The second line of the double underline: at the bottom.
        rect strikeout; // fonts: Strikethrough rectangle block within the cell.
        rect overline;  // fonts: Overline rectangle block within the cell.
        rect dashline;  // fonts: Dashed underline rectangle block within the cell.
        rect wavyline;  // fonts: Wavy underline outer rectangle block within the cell.

        fonts(std::list<text>& family_names, cfg_t::axis_vals_t& font_axes, si32 cell_height)
            : ft_library{ make_ft_library() },
               os_locale{ ::hb_language_from_string(os::env::get_locale().c_str(), -1) },
             font_shaper{ *this }
        {
            #if defined(_WIN32)
                auto registered_fonts = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts"sv;
                auto filter = [](auto& item)
                {
                    static auto user_font_dir   = os::env::get("LocalAppData") + "\\Microsoft\\Windows\\Fonts\\";
                    static auto system_font_dir = os::env::get("WINDIR") + "\\Fonts\\";
                    auto ok = item.type == REG_SZ && item.data.size();
                    if (ok)
                    {
                        if (item.data.front() != '\\' && !utf::check_any(item.data, ":")) // Convert to absolute path.
                        {
                            item.data = (item.hive == HKEY_CURRENT_USER ? user_font_dir : system_font_dir) + item.data;
                        }
                    }
                    return ok;
                };
                auto system_font_flow = os::nt::walk_registry(HKEY_CURRENT_USER,  registered_fonts, filter)
                                      | os::nt::walk_registry(HKEY_LOCAL_MACHINE, registered_fonts, filter);
            #else
                //todo build a native system_font_flow
                struct fontfile_item_t
                {
                    text path;
                    text name;
                    text data;
                };
                auto system_font_flow = std::vector<fontfile_item_t>{};
            #endif
            auto font_list = std::vector<sptr<bare_face_t>>{};
            for (auto& item : system_font_flow)
            {
                auto ec = std::error_code{};
                auto file_stamp = os::fs::last_write_time(item.data, ec);
                if (ec)
                {
                    log("%%Failed to get font file '%path%' modification time (error %ec%): %msg%", prompt::gui, item.data, ec.value(), os::get_system_error_message(ec.value()));
                }
                else
                {
                    auto face = FT_Face{};
                    auto index = 0;
                    while (FT_Err_Ok == ::FT_New_Face(ft_library.get(), item.data.c_str(), index++, &face)) // Read headers only (fast enough).
                    {
                        if (auto os2 = (TT_OS2*)::FT_Get_Sfnt_Table(face, FT_SFNT_OS2)) // We need OS/2 metadata.
                        {
                            auto family = qiew{ face->family_name };
                            auto style  = qiew{ face->style_name };
                            auto rec_ptr = font_list.emplace_back(ptr::shared<bare_face_t>());
                            auto& rec = *rec_ptr;
                            rec.file_stamp             = file_stamp;
                            rec.file_path              = std::move(item.data);
                            rec.family_name            = family.str();
                            rec.style_name             = style.str();
                            rec.face_index             = face->face_index;
                            rec.face_flags             = face->face_flags;
                            rec.style_flags            = face->style_flags;
                            rec.num_glyphs             = face->num_glyphs;
                            rec.bbox                   = {{ face->bbox.xMin, face->bbox.yMin }, { face->bbox.xMax, face->bbox.yMax }};
                            rec.units_per_EM           = face->units_per_EM;
                            rec.ascender               = face->ascender;
                            rec.descender              = -face->descender;
                            rec.height                 = face->height;
                            rec.max_advance_width      = face->max_advance_width;
                            rec.max_advance_height     = face->max_advance_height;
                            rec.underline_position     = face->underline_position;
                            rec.underline_thickness    = face->underline_thickness;
                            rec.strikethroughPosition  = os2->yStrikeoutPosition;
                            rec.strikethroughThickness = os2->yStrikeoutSize;
                            rec.lineGap                = os2->sTypoLineGap;
                            auto has_colr = has_sfnt_table(face, FT_MAKE_TAG('C', 'O', 'L', 'R'));
                            auto has_cpal = has_sfnt_table(face, FT_MAKE_TAG('C', 'P', 'A', 'L'));
                            auto has_svg  = has_sfnt_table(face, FT_MAKE_TAG('S', 'V', 'G', ' '));
                            rec.is_color = has_svg ? fonts::color_type::svg : (has_colr && has_cpal) ? fonts::color_type::colr : fonts::color_type::mono;
                            rec.valid = rec.num_glyphs && rec.units_per_EM && (rec.is_color || FT_IS_SCALABLE(face));
                            rec.is_monospaced          = FT_IS_FIXED_WIDTH(face);
                            rec.weight_value           = os2->usWeightClass;
                            rec.stretch_value          = os2->usWidthClass;
                            rec.is_italic              = face->style_flags & FT_STYLE_FLAG_ITALIC;
                            auto amaster = (FT_MM_Var*)nullptr;
                            auto has_wdth = faux;
                            auto has_wght = faux;
                            auto has_ital = faux;
                            if (FT_Err_Ok == ::FT_Get_MM_Var(face, &amaster))
                            {
                                rec.is_variable_font = true;
                                for (auto i = 0u; i < amaster->num_axis; i++) // Axes count (weight, width, ital...)
                                {
                                    auto& axis = amaster->axis[i];
                                    auto axis_values = axis_rec_t{ axis.name, axis.def / 65536.0f, axis.minimum / 65536.0f, axis.maximum / 65536.0f };
                                    rec.axes_map.emplace(axis.tag, axis_values);
                                    switch (axis.tag)
                                    {
                                        case cfg_t::ft_tag("wdth"): has_wdth = true; rec.width_target  = axis_values; break;
                                        case cfg_t::ft_tag("wght"): has_wght = true; rec.weight_target = axis_values; break;
                                        case cfg_t::ft_tag("ital"): has_ital = true; rec.italic_target = axis_values; break;
                                    }
                                }
                                ::FT_Done_MM_Var(ft_library.get(), amaster);
                            }
                            // Bring basic parameters to a unified form - axes.
                            if (!has_wdth)
                            {
                                static constexpr auto stretch_lut = std::to_array<fp32>(
                                {
                                    50.0f,   // 1 Ultra-condensed 50%
                                    62.5f,   // 2 Extra-condensed 62.5%
                                    75.0f,   // 3 Condensed       75%
                                    87.5f,   // 4 Semi-condensed  87.5%
                                    100.0f,  // 5 Medium (Normal) 100%
                                    112.5f,  // 6 Semi-expanded   112.5%
                                    125.0f,  // 7 Expanded        125%
                                    150.0f,  // 8 Extra-expanded  150%
                                    200.0f,  // 9 Ultra-expanded  200%
                                });
                                auto wdth = stretch_lut[std::clamp(rec.stretch_value, 1, (si32)stretch_lut.size()) - 1];
                                auto axis_values = axis_rec_t{ "Width", wdth, wdth, wdth };
                                rec.axes_map.emplace(cfg_t::ft_tag("wdth"), axis_values);
                                rec.width_target = axis_values;
                            }
                            if (!has_wght)
                            {
                                auto wght = (fp32)rec.weight_value;
                                auto axis_values = axis_rec_t{ "Weight", wght, wght, wght };
                                rec.axes_map.emplace(cfg_t::ft_tag("wght"), axis_values);
                                rec.weight_target = axis_values;
                            }
                            if (!has_ital)
                            {
                                auto ital = (fp32)rec.is_italic;
                                auto axis_values = axis_rec_t{ "Italic", ital, ital, ital };
                                rec.axes_map.emplace(cfg_t::ft_tag("ital"), axis_values);
                                rec.italic_target = axis_values;
                            }
                            if (!(os2->fsSelection & 0x80) || os2->version == 0xFFFF) // fsSelection bit 7 (0x80): USE_TYPO_METRICS.
                            {
                                if (auto hhea = (TT_HoriHeader*)::FT_Get_Sfnt_Table(face, FT_SFNT_HHEA)) // Old Mac fonts that do not include an 'OS/2' table. In this case, the `version` field is always set to 0xFFFF.
                                {
                                    rec.lineGap = hhea->Line_Gap;
                                }
                            }
                            if (os2->version != 0xFFFF && os2->version >= 2)
                            {
                                rec.xHeight   = os2->sxHeight;
                                rec.capHeight = os2->sCapHeight;
                            }
                            if (rec.capHeight == 0) fonts::get_char_height(face, 'H', rec.capHeight, face->ascender);         // Arabic, emoji, web/windings have no capitals
                            if (rec.xHeight   == 0) fonts::get_char_height(face, 'x', rec.xHeight,   face->ascender * 2 / 3); // or latin letters.
                            // Fill supported unicode ranges.
                            auto glyph_index = FT_UInt{};
                            auto char_code = ::FT_Get_First_Char(face, &glyph_index);
                            if (!glyph_index) // There is no FT_ENCODING_UNICODE code page. Try to select ms symbol code page.
                            {
                                ::FT_Select_Charmap(face, FT_ENCODING_MS_SYMBOL);
                                char_code = ::FT_Get_First_Char(face, &glyph_index);
                                log("%%Try to select 'MS Symbol' code page for '%%' font.", prompt::gui, rec.file_path.generic_string());
                                if (glyph_index) rec.mssymbol_codepage = true;
                            }
                            if (glyph_index)
                            {
                                auto current_block = char_code / 256;
                                auto bit_set = rec.unicode_ranges.try_emplace(current_block).first;
                                bit_set->second.set(char_code % 256);
                                while ((char_code = ::FT_Get_Next_Char(face, char_code, &glyph_index)))
                                {
                                    auto next_block = char_code / 256;
                                    if (next_block != current_block)
                                    {
                                        current_block = next_block;
                                        bit_set = rec.unicode_ranges.try_emplace(current_block).first;
                                    }
                                    bit_set->second.set(char_code % 256);
                                }
                            }
                            else
                            {
                                log("%%Font '%%' has no FT_ENCODING_UNICODE code page.", prompt::gui, rec.file_path.generic_string());
                            }
                        }
                        ::FT_Done_Face(face);
                    }
                }
            }
            //std::ranges::sort(font_list, std::ranges::greater{}, &bare_face_t::stamp);
            //std::ranges::sort(font_list, [](auto& a, auto& b){ return std::tie(a.stamp, a.name) < std::tie(b.stamp, b.name); }); // With std::tie, sorting all fields is only possible in one direction.
            std::sort(font_list.begin(), font_list.end(), [](auto a_ptr, auto b_ptr)
            {
                //if (a.stamp == b.stamp) return a.family_name < b.family_name; // Sort desc by time then asc by family_name.
                //else                    return a.stamp > b.stamp;
                //if (a.family_name == b.family_name) return a.file_stamp > b.file_stamp; // Sort asc by family_name then desc by time.
                //else                                return a.family_name < b.family_name;
                auto& a = *a_ptr;
                auto& b = *b_ptr;
                if (a.family_name == b.family_name)
                {
                    if (a.weight_value == b.weight_value)
                    {
                        return a.file_stamp > b.file_stamp; // Sort desc by file_stamp.
                    }
                    else
                    {
                        return a.weight_value < b.weight_value; // Sort asc by family_name then asc by weight.
                    }
                }
                else
                {
                    return a.family_name < b.family_name;
                }
            });
            // first: is_color || is_monospaced.
            // then: file_stamp (descending).
            for (auto& rec_ptr : font_list)
            {
                auto& rec = *rec_ptr;
                auto& family_rec = font_map[rec.family_name];
                if (family_rec.file_stamp < rec.file_stamp) family_rec.file_stamp = rec.file_stamp; // Update time stamp.
                family_rec.is_monospaced |= rec.is_monospaced;
                if (rec.is_color > family_rec.is_color) family_rec.is_color = rec.is_color;
                for (auto& [block, bit_set] : rec.unicode_ranges)
                {
                    family_rec.unicode_ranges[block] |= bit_set;
                }
                //log("%stamp% \x1b[7m%name%\x1b[m \x1b[7m%style%\x1b[m", rec.file_stamp, rec.family_name, rec.style_name,
                //    //    "");
                //    "\n\trec.file_path              = ", rec.file_path.generic_string(),
                //    "\n\trec.face_index             = ", rec.face_index,
                //    "\n\trec.face_flags             = ", utf::to_bin(rec.face_flags),
                //    "\n\trec.style_flags            = ", rec.style_flags,
                //    "\n\trec.num_glyphs             = ", rec.num_glyphs,
                //    "\n\trec.bbox                   = ", rec.bbox,
                //    "\n\trec.units_per_EM           = ", rec.units_per_EM,
                //    "\n\trec.ascender               = ", rec.ascender,
                //    "\n\trec.descender              = ", rec.descender,
                //    "\n\trec.height                 = ", rec.height,
                //    "\n\trec.max_advance_width      = ", rec.max_advance_width,
                //    "\n\trec.max_advance_height     = ", rec.max_advance_height,
                //    "\n\trec.underline_position     = ", rec.underline_position,
                //    "\n\trec.underline_thickness    = ", rec.underline_thickness,
                //    "\n\trec.strikethroughPosition  = ", rec.strikethroughPosition,
                //    "\n\trec.strikethroughThickness = ", rec.strikethroughThickness,
                //    "\n\trec.xHeight                = ", rec.xHeight,
                //    "\n\trec.capHeight              = ", rec.capHeight,
                //    "\n\trec.lineGap                = ", rec.lineGap,
                //    "\n\trec.is_color               = ", rec.is_color,
                //    "\n\trec.weight_value           = ", rec.weight_value,
                //    "\n\trec.is_italic              = ", rec.is_italic,
                //    "\n\trec.is_monospaced          = ", rec.is_monospaced,
                //    "\n\trec.stretch_value          = ", rec.stretch_value,
                //        "");
                //    if (rec.is_variable_font)
                //    {
                //        log("\tVariable font with %% ax%es%:", rec.axes_map.size(), rec.axes_map.size() == 1 ? "is" : "es");
                //        for (auto& [tag, axis_rec] : rec.axes_map)
                //        {
                //            log("\t\t%name% %tag% %%-%%-%%", axis_rec.name, ft_untag(tag), axis_rec.min, axis_rec.def, axis_rec.max);
                //        }
                //    }
                //    //log("\x1b[7m%name%\x1b[m \x1b[7m%style%\x1b[m %pad% %glyphs% %file%", rec.family_name, rec.style_name, text(max_name - rec.family_name.size() - rec.style_name.size(), ' '),
                //    //    utf::adjust(utf::fprint("glyphs:%%", rec.num_glyphs), 15, ' '), rec.file_path.filename());
                family_rec.bare_face_list.push_back({ .bare_face_ptr = rec_ptr });
            }
            // Add square/nonsquare property.
            font_index.reserve(font_map.size());
            for (auto& [family_name, family_rec] : font_map)
            {
                family_rec.family_name = family_name;
                font_index.push_back(family_rec);
                if (auto& bare_face_ptr = family_rec.bare_face_list.front().bare_face_ptr)
                {
                    auto& rec = *(bare_face_ptr);
                    auto ratio = rec.max_advance_width / (fp32)(rec.ascender - rec.descender + rec.lineGap);
                    family_rec.non_square = ratio != 1.f;
                }
            }
            // Build ordered vec<ref<family>> by filestamp.
            //std::ranges::sort(font_index, std::ranges::greater{}, &font_family_t::file_stamp);
            std::sort(font_index.begin(), font_index.end(), [](font_family_t& a, font_family_t& b)
            {
                // Sort fonts by iscolor, monospaced, non_square then by file_date.
                auto a_class = (a.is_monospaced + a.is_color) * 2 + a.non_square;
                auto b_class = (b.is_monospaced + b.is_color) * 2 + b.non_square;
                if (a_class == b_class) return a.file_stamp > b.file_stamp;
                else                    return a_class > b_class;
            });
            //log_fonts(faux);
            set_fonts(family_names, font_axes);
            if (font_fallback.empty())
            {
                auto single_codepoint = std::to_array<utfx>({ 'A' });
                log(prompt::gui, "No fonts provided, fallback to the first available font");
                find_family(single_codepoint, true); // Take the first available font.
            }
            set_cellsz(cell_height);
        }
    };

    struct glyph
    {
        using irgb = netxs::irgb<fp32>;

        struct synthetic
        {
            static constexpr auto _counter = 1 + __COUNTER__;
            static constexpr auto wavyunderline = __COUNTER__ - _counter;
        };

        using gmap = std::unordered_map<ui64, sprite>;

        std::pmr::unsynchronized_pool_resource buffer_pool; // glyph: Pool for temp buffers.
        std::pmr::monotonic_buffer_resource    mono_buffer; // glyph: Memory block for sprites.
        fonts&                                 fcache;      // glyph: Font cache.
        twod&                                  cellsz;      // glyph: Terminal cell size in pixels.
        bool                                   aamode;      // glyph: Enable AA.
        gmap                                   glyphs;      // glyph: Glyph map.
        std::vector<sprite>                    cgi_glyphs;  // glyph: Synthetic glyphs.
        std::vector<sprite>                    cgi_shadow;  // glyph: Synthetic shadow.
        std::vector<utfx>                      codepoints;  // glyph: Codepoint list for shaping.

        glyph(fonts& fcache, bool aamode)
            : fcache{ fcache },
              cellsz{ fcache.cellsize },
              aamode{ aamode }
        {
            generate_glyphs();
            generate_shadow();
        }

        // Generate shadow sprites.
        void generate_shadow()
        {
            using namespace ui::pro;
            auto block = rect{ dot_00, cellsz };
            cgi_shadow.reserve(256);
            auto width = cellsz.x;
            auto height = cellsz.y;
            auto shadow = shad{ 0.44f/*bias*/, 116.5f/*alfa*/, width, dot_00, dot_11, cell::shaders::full };
            auto l_area_1x1 = rect{{ -width, -height }, dot_00};
            auto r_area_1x1 = rect{{  width, -height }, dot_00};
            //todo unify/optimize
            for (auto i = 0; i < 256; i++)
            {
                auto& s = cgi_shadow.emplace_back(buffer_pool);
                s.area = block;
                s.type = sprite::alpha;
                s.bits.resize(netxs::udivupper(s.area.length(), sizeof(ui32)));
                auto raster = s.raster<byte>();
                auto matrix = i;
                // Left verticals.
                auto l = l_area_1x1;
                auto mid_bits = (matrix & ghost::x2y1) | (matrix & ghost::x2y3);
                if ((matrix & ghost::x1y1_x1y2_x1y3) == ghost::x1y1_x1y2_x1y3)
                {
                    l.size = { width, height * 3 };
                    matrix = (matrix & ~ghost::x1y1_x1y2_x1y3) | ((mid_bits & ghost::x1y1_x1y2_x1y3) << 1);
                }
                else if ((matrix & ghost::x1y1_x1y2) == ghost::x1y1_x1y2)
                {
                    l.size = { width, height * 2 };
                    matrix = (matrix & ~ghost::x1y1_x1y2) | ((mid_bits & ghost::x1y1_x1y2) << 1);
                }
                else if ((matrix & ghost::x1y2_x1y3) == ghost::x1y2_x1y3)
                {
                    l.coor.y += height;
                    l.size = { width, height * 2 };
                    matrix = (matrix & ~ghost::x1y2_x1y3) | ((mid_bits & ghost::x1y2_x1y3) << 1);
                }
                if (l)
                {
                    shadow.render(raster, raster.area(), l, cell::shaders::alphamix);
                }
                // Right verticals.
                auto r = r_area_1x1;
                if ((matrix & ghost::x3y1_x3y2_x3y3) == ghost::x3y1_x3y2_x3y3)
                {
                    r.size = { width, height * 3 };
                    matrix = (matrix & ~ghost::x3y1_x3y2_x3y3) | ((mid_bits & ghost::x3y1_x3y2_x3y3) >> 1);
                }
                else if ((matrix & ghost::x3y1_x3y2) == ghost::x3y1_x3y2)
                {
                    r.size = { width, height * 2 };
                    matrix = (matrix & ~ghost::x3y1_x3y2) | ((mid_bits & ghost::x3y1_x3y2) >> 1);
                }
                else if ((matrix & ghost::x3y2_x3y3) == ghost::x3y2_x3y3)
                {
                    r.coor.y += height;
                    r.size = { width, height * 2 };
                    matrix = (matrix & ~ghost::x3y2_x3y3) | ((mid_bits & ghost::x3y2_x3y3) >> 1);
                }
                if (r)
                {
                    shadow.render(raster, raster.area(), r, cell::shaders::alphamix);
                }
                // Top horizontals.
                auto t = l_area_1x1;
                if ((matrix & ghost::x1y1_x2y1_x3y1) == ghost::x1y1_x2y1_x3y1)
                {
                    t.size = { width * 3, height };
                }
                else if ((matrix & ghost::x1y1_x2y1) == ghost::x1y1_x2y1)
                {
                    t.size = { width * 2, height };
                }
                else if ((matrix & ghost::x2y1_x3y1) == ghost::x2y1_x3y1)
                {
                    t.size = { width * 2, height };
                    t.coor.x += width;
                }
                else if ((matrix & ghost::x1y1_x3y1) == ghost::x1y1_x3y1)
                {
                    t.size = { width, height };
                    auto t2 = t;
                    t.coor.x += width * 2;
                    shadow.render(raster, raster.area(), t2, cell::shaders::alphamix);
                }
                else if ((matrix & ghost::x1y1) == ghost::x1y1)
                {
                    t.size = { width, height };
                }
                else if ((matrix & ghost::x2y1) == ghost::x2y1)
                {
                    t.size = { width, height };
                    t.coor.x += width;
                }
                else if ((matrix & ghost::x3y1) == ghost::x3y1)
                {
                    t.size = { width, height };
                    t.coor.x += width * 2;
                }
                if (t)
                {
                    shadow.render(raster, raster.area(), t, cell::shaders::alphamix);
                }
                // Mid horizontals.
                auto m = l_area_1x1;
                m.coor.y += height;
                if ((matrix & ghost::x1y2) == ghost::x1y2)
                {
                    m.size = { width, height };
                    shadow.render(raster, raster.area(), m, cell::shaders::alphamix);
                }
                else if ((matrix & ghost::x3y2) == ghost::x3y2)
                {
                    m.size = { width, height };
                    m.coor.x += width * 2;
                    shadow.render(raster, raster.area(), m, cell::shaders::alphamix);
                }
                // Bottom horizontals.
                auto b = l_area_1x1;
                b.coor.y += height * 2;
                if ((matrix & ghost::x1y3_x2y3_x3y3) == ghost::x1y3_x2y3_x3y3)
                {
                    b.size = { width * 3, height };
                }
                else if ((matrix & ghost::x1y3_x2y3) == ghost::x1y3_x2y3)
                {
                    b.size = { width * 2, height };
                }
                else if ((matrix & ghost::x2y3_x3y3) == ghost::x2y3_x3y3)
                {
                    b.size = { width * 2, height };
                    b.coor.x += width;
                }
                else if ((matrix & ghost::x1y3_x3y3) == ghost::x1y3_x3y3)
                {
                    b.size = { width, height };
                    auto b2 = b;
                    b.coor.x += width * 2;
                    shadow.render(raster, raster.area(), b2, cell::shaders::alphamix);
                }
                else if ((matrix & ghost::x1y3) == ghost::x1y3)
                {
                    b.size = { width, height };
                }
                else if ((matrix & ghost::x2y3) == ghost::x2y3)
                {
                    b.size = { width, height };
                    b.coor.x += width;
                }
                else if ((matrix & ghost::x3y3) == ghost::x3y3)
                {
                    b.size = { width, height };
                    b.coor.x += width * 2;
                }
                if (b)
                {
                    shadow.render(raster, raster.area(), b, cell::shaders::alphamix);
                }
            }
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
            auto k = 3.14f / 2.f / fract; // Align the fract with the sine period.
            block.size.x = 1;
            block.size.y = thick;
            auto c = byte{ 255 }; // Opaque alpha texture.
            auto& m = cgi_glyphs.emplace_back(buffer_pool);
            m.area = {{ 0, block.coor.y }, { width, height }};
            m.type = sprite::alpha;
            m.bits.resize(netxs::udivupper(m.area.length(), sizeof(ui32)));
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
            cgi_shadow.clear();
            mono_buffer.release();
            generate_glyphs();
            generate_shadow();
            reset_cached_rasters();
        }
        void reset_cached_rasters()
        {
            auto images = cell::images(); // Lock.
            for (auto image_ptr : images) // Iterate over 65536 ptrs.
            {
                if (image_ptr)
                {
                    image_ptr->reset_raster();
                }
            }
        }
        void draw_layer_to_canvas(auto& canvas, FT_GlyphSlot slot, fp2d pen, fp2d hb_align, irgb fill = {})
        {
            auto& bitmap = slot->bitmap;
            if (bitmap.width == 0 || bitmap.rows == 0) return;
            auto layer_area = rect{};
            layer_area.size = { (si32)bitmap.width, (si32)bitmap.rows };
            layer_area.coor = { (si32)(pen.x + hb_align.x + slot->bitmap_left),
                                (si32)(pen.y + hb_align.y - slot->bitmap_top) };
            auto canvas_area = canvas.area();
            if (auto intersect = canvas_area.trim(layer_area))
            {
                auto dst_base = intersect.coor - canvas_area.coor;
                auto src_base = intersect.coor - layer_area.coor;
                auto src_data = bitmap.buffer;
                auto src_pitch = bitmap.pitch;
                auto draw_pixel = [&](si32 x, si32 y, byte alpha)
                {
                    auto dst_xy = twod{ dst_base.x + x, dst_base.y + y };
                    auto& dst_px = canvas[dst_xy];
                    if constexpr (std::is_same_v<std::decay_t<decltype(dst_px)>, irgb>)
                    {
                        if (fill.a == 0.f)
                        {
                            if (alpha == 255)
                            {
                                dst_px.r = 0.f; dst_px.g = 0.f; dst_px.b = 0.f;
                                dst_px.pack_alpha(255);
                            }
                            else
                            {
                                auto fgc_a = dst_px.has_extra_alpha() ? dst_px.get_extra_alpha() : byte{};
                                dst_px.pack_alpha(alpha + (255 - alpha) * fgc_a / 255);
                            }
                        }
                        else dst_px.blend_nonpma(fill, alpha);
                    }
                    else dst_px = (byte)std::min(255, dst_px + alpha);
                };
                if (bitmap.pixel_mode == FT_PIXEL_MODE_MONO) // 1-bit per pixel.
                {
                    for (auto row = 0; row < intersect.size.y; ++row)
                    {
                        auto src_row = src_data + (src_base.y + row) * src_pitch;
                        for (auto col = 0; col < intersect.size.x; ++col)
                        {
                            auto sx = src_base.x + col;
                            if (src_row[sx >> 3] & (0x80 >> (sx & 7)))
                            {
                                draw_pixel(col, row, 255);
                            }
                        }
                    }
                }
                else // 8-bit FT_PIXEL_MODE_GRAY.
                {
                    for (auto row = 0; row < intersect.size.y; ++row)
                    {
                        auto src_row = src_data + (src_base.y + row) * src_pitch + src_base.x;
                        for (auto col = 0; col < intersect.size.x; ++col)
                        {
                            if (auto alpha = src_row[col])
                            {
                                draw_pixel(col, row, alpha);
                            }
                        }
                    }
                }
            }
        }
        void draw_svg_to_canvas(auto& canvas, lunasvg::Bitmap const& svg_btm, rect layer_area)
        {
            if (!svg_btm.valid()) return;
            auto canvas_area = canvas.area();
            if (auto intersect = canvas_area.trim(layer_area))
            {
                auto dst_base = intersect.coor - canvas_area.coor;
                auto src_base = intersect.coor - layer_area.coor;
                auto* src_data = (ui32*)svg_btm.data();
                auto src_stride = svg_btm.stride() / sizeof(argb);
                for (auto y = 0; y < intersect.size.y; ++y)
                {
                    auto* src_row = src_data + (src_base.y + y) * src_stride + src_base.x;
                    for (auto x = 0; x < intersect.size.x; ++x)
                    {
                        auto src_px = argb{ src_row[x] }; // PMA sRGB.
                        if (src_px.chan.a > 0)
                        {
                            auto dst_xy = twod{ dst_base.x + x, dst_base.y + y };
                            auto& dst_px = canvas[dst_xy]; // PMA Linear.
                            if constexpr (std::is_same_v<std::decay_t<decltype(dst_px)>, irgb>)
                            {
                                auto pixel = irgb::pma_srgb_to_pma_linear(src_px); // PMA Linear.
                                if (dst_px.has_extra_alpha())
                                {
                                    auto fgc_a = dst_px.unpack_alpha();
                                    dst_px.blend_pma(pixel);
                                    dst_px.pack_alpha(fgc_a);
                                }
                                else
                                {
                                    dst_px.blend_pma(pixel);
                                }
                            }
                            else // Alpha-only canvas fallback
                            {
                                dst_px = (byte)std::min(255, dst_px + src_px.chan.a);
                            }
                        }
                    }
                }
            }
        }
        void draw_svg_to_canvas(auto& canvas,
                                lunasvg::Bitmap const& bitmap_black, // currentColor = #000000
                                lunasvg::Bitmap const& bitmap_trans, // currentColor = 0x00000000
                                lunasvg::Bitmap const& bitmap_white, // currentColor = #FFFFFF
                                rect layer_area)
        {
            if (!bitmap_black.valid() || !bitmap_trans.valid() || !bitmap_white.valid()) return;
            auto canvas_area = canvas.area();
            auto intersect = canvas_area.trim(layer_area);
            if (!intersect) return;
            auto dst_base = intersect.coor - canvas_area.coor;
            auto src_base = intersect.coor - layer_area.coor;
            auto src_stride_trans = bitmap_trans.stride() / sizeof(ui32);
            auto src_stride_black = bitmap_black.stride() / sizeof(ui32);
            auto src_stride_white = bitmap_white.stride() / sizeof(ui32);
            auto src_data_trans = (ui32 const*)bitmap_trans.data() + src_base.y * src_stride_trans;
            auto src_data_black = (ui32 const*)bitmap_black.data() + src_base.y * src_stride_black;
            auto src_data_white = (ui32 const*)bitmap_white.data() + src_base.y * src_stride_white;
            for (auto y = 0; y < intersect.size.y; ++y)
            {
                auto r_trans = src_data_trans + src_base.x;
                auto r_black = src_data_black + src_base.x;
                auto r_white = src_data_white + src_base.x;
                for (auto x = 0; x < intersect.size.x; ++x)
                {
                    auto white_px = argb{ r_white[x] };
                    if (white_px.chan.a > 0)
                    {
                        auto dst_xy = twod{ dst_base.x + x, dst_base.y + y };
                        auto& dst_px = canvas[dst_xy];
                        if constexpr (std::is_same_v<std::decay_t<decltype(dst_px)>, irgb>)
                        {
                            auto trans_px = argb{ r_trans[x] };
                            auto black_px = argb{ r_black[x] };
                            auto alpha = (byte)std::max(0, (si32)white_px.chan.r - black_px.chan.r);
                            auto fgc_a = dst_px.has_extra_alpha() ? dst_px.unpack_alpha() : byte{};
                            alpha = alpha + (255 - alpha) * fgc_a / 255;
                            dst_px.blend_pma(trans_px);
                            dst_px.pack_alpha(alpha);
                        }
                        else // Alpha-only canvas
                        {
                            dst_px = (byte)std::min(255, dst_px + white_px.chan.a);
                        }
                    }
                }
                src_data_trans += src_stride_trans;
                src_data_black += src_stride_black;
                src_data_white += src_stride_white;
            }
        }
        void resolveOTSVGVariables(std::span<char> data)
        {
            auto crop = view{ data.data(), data.size() };
            auto pos = crop.rfind("var("); // var(--name) or var(--name, defvalue) or var(--name, var(--name, defvalue)) ...
            while (pos != view::npos)
            {
                auto end = crop.find(')', pos);
                if (end != view::npos)
                {
                    auto comma = crop.find(',', pos);
                    if (comma != view::npos && comma < end)
                    {
                        data[pos] = data[pos + 1] = data[pos + 2] = data[pos + 3] = ' '; // Wipe "var(".
                        for (auto i = pos + 4; i <= comma; ++i) // "--name," -> "      ".
                        {
                            data[i] = ' ';
                        }
                        data[end] = ' '; // ')'->' '
                    }
                    else
                    {
                        for (auto i = pos; i <= end; ++i) // '(var(--name))'->' ... '.
                        {
                            data[i] = ' ';
                        }
                    }
                }
                if (pos == 0) break;
                pos = crop.rfind("var(", pos - 1);
            }
        }
        auto generate_DOM(std::span<char> svg_data)
        {
            resolveOTSVGVariables(svg_data);
            // currentColor test
            //auto test_view = view{ svg_data.data(), svg_data.size() };
            ////auto test_cc = utf::replace_all(test_view, "gold", "currentColor");
            //auto test_cc = utf::replace_all(test_view, "#C90900", "currentColor");
            //svg_data = test_cc;
            auto change_currentColor = [](std::span<char> data, view colorString) -> auto& // Workaround for currentColor.
            {
                assert(colorString.size() <= "currentColor"sv.size());
                static thread_local auto matches = std::vector<arch>{}; // List of currentColors positions within the document.
                matches.clear();
                auto pos = arch{};
                auto crop = view{ data.data(), data.size() };
                while((pos = crop.find("currentColor", pos)) != text::npos)
                {
                    matches.push_back(pos);
                    std::copy(colorString.begin(), colorString.end(), data.begin() + pos); // "#556677"
                    auto end = pos + "currentColor"sv.size();
                    pos += colorString.size();
                    std::fill(data.begin() + pos, data.begin() + end, ' '); // Trailing spaces.
                    pos = end;
                }
                return matches;
            };
            auto& matches = change_currentColor(svg_data, "#000000"); // Set black (default) for the currentColor if it is.
            auto document_black = lunasvg::Document::loadFromData(svg_data.data(), svg_data.size());
            //auto document_black = uptr<lunasvg::Document>{};
            auto document_trans = uptr<lunasvg::Document>{};
            auto document_white = uptr<lunasvg::Document>{};
            if (!document_black) // Fallback to \uFFFD "�".
            {
                static constexpr auto badsvg = R"==(<svg viewBox="0 0 1000 1000">
                                                        <path d="M 500,50 950,500 500,950 50,500 Z m -46.53619,594.80924 h 93.07238 v 93.07237 H 453.46381 Z M 430,381.07084 c 0,-50 30,-80 70,-80 40,0 70,30 70,80 0,50 -30,70 -55,95 -35,35 -45,65 -45,115 h 60 c 0,-30 10,-50 35,-75 25,-25 65,-60 65,-135 0,-80 -55,-140 -130,-140 -75,0 -130,60 -130,140 z" fill="currentColor" fill-rule="evenodd" />
                                                    </svg>)=="sv;
                auto black = utf::replace_all(badsvg, "currentColor", "#000000");
                document_black = lunasvg::Document::loadFromData(black.data(), black.size());
                auto trans = utf::replace_all(badsvg, "currentColor", "transparent");
                document_trans = lunasvg::Document::loadFromData(trans.data(), trans.size());
                auto white = utf::replace_all(badsvg, "currentColor", "#FFFFFF");
                document_white = lunasvg::Document::loadFromData(white.data(), white.size());
            }
            else if (document_black && matches.size()) // We have currentColors. Generate addidtional document layers.
            {
                                                // "currentColor"
                static constexpr auto pure_trans = "transparent "sv;
                static constexpr auto pure_white = "#FFFFFF     "sv;
                for (auto pos : matches)
                {
                    std::copy(pure_trans.begin(), pure_trans.end(), svg_data.begin() + pos);
                }
                document_trans = lunasvg::Document::loadFromData(svg_data.data(), svg_data.size());
                for (auto pos : matches)
                {
                    std::copy(pure_white.begin(), pure_white.end(), svg_data.begin() + pos);
                }
                document_white = lunasvg::Document::loadFromData(svg_data.data(), svg_data.size());
            }
            return std::to_array({ std::move(document_black),
                                   std::move(document_trans),
                                   std::move(document_white) });
        }
        void rasterize_glyph_DOM(auto& canvas, imagens::docs& svg_DOM, qiew sub_id, rect area)
        {
            static thread_local auto bitmaps = std::array<lunasvg::Bitmap, 3>();

            auto& [document_black_ptr, document_trans_ptr, document_white_ptr] = svg_DOM;
            if (!document_black_ptr) return;
            auto w = area.size.x;
            auto h = area.size.y;
            auto render_bitmap = [&](auto& document_ptr, auto& bitmap)
            {
                auto& document = *document_ptr;
                auto element = [&]
                {
                    if (sub_id.empty()) return document.documentElement();
                    auto e = document.getElementById(sub_id);
                    if (!e) // Render a whole document if sub_id not found.
                    {
                        e = document.documentElement();
                    }
                    return e;
                }();
                auto bounds = element.getBoundingBox().transform(element.getLocalMatrix());
                auto scale = std::min(w / bounds.w, h / bounds.h);
                auto tx = (w - bounds.w * scale) / 2.f - bounds.x * scale;
                auto ty = (h - bounds.h * scale) / 2.f - bounds.y * scale;
                auto matrix = lunasvg::Matrix{ scale, 0, 0, scale, tx, ty };
                if (bitmap.height() < h || bitmap.width() < w)
                {
                    bitmap = { w, h };
                }
                else
                {
                    bitmap.clear(0);
                }
                element.render(bitmap, matrix); // Premultiplied ARGB32 pixel data.
            };
            render_bitmap(document_black_ptr, bitmaps[0]);
            if (document_trans_ptr)
            {
                render_bitmap(document_trans_ptr, bitmaps[1]);
                render_bitmap(document_white_ptr, bitmaps[2]);
                draw_svg_to_canvas(canvas, bitmaps[0], bitmaps[1], bitmaps[2], area);
            }
            else
            {
                draw_svg_to_canvas(canvas, bitmaps[0], area);
            }
        }
        void rasterize_svg_DOM(auto& canvas, imagens::docs& svg_DOM, fp2d original_doc_size_fpx, qiew sub_id, bool keep_ratio)
        {
            static thread_local auto bitmaps = std::array<lunasvg::Bitmap, 3>();

            auto& [document_black_ptr, document_trans_ptr, document_white_ptr] = svg_DOM;
            if (!document_black_ptr) return;
            auto area = canvas.area();
            auto w = area.size.x;
            auto h = area.size.y;
            auto render_bitmap = [&](auto& document_ptr, auto& bitmap)
            {
                if (bitmap.height() < h || bitmap.width() < w)
                {
                    bitmap = { w, h };
                }
                else
                {
                    bitmap.clear(0);
                }
                auto& document = *document_ptr;
                auto element = sub_id.empty() ? document.documentElement()
                                              : document.getElementById(sub_id);
                if (element) // Draw nothing if sub_id is not found.
                {
                    auto bounds = original_doc_size_fpx;
                    auto scale = fp2d{ area.size.x / bounds.x, area.size.y / bounds.y };
                    if (keep_ratio)
                    {
                        auto r = std::min(scale.x, scale.y);
                        scale = { r, r };
                    }
                    auto matrix = lunasvg::Matrix{ scale.x, 0, 0, scale.y, 0, 0 };
                    if (sub_id && element != document.documentElement())
                    {
                        matrix *= element.getGlobalMatrix();
                    }
                    element.render(bitmap, matrix); // Premultiplied ARGB32 pixel data.
                }
            };
            render_bitmap(document_black_ptr, bitmaps[0]);
            if (document_trans_ptr)
            {
                render_bitmap(document_trans_ptr, bitmaps[1]);
                render_bitmap(document_white_ptr, bitmaps[2]);
                draw_svg_to_canvas(canvas, bitmaps[0], bitmaps[1], bitmaps[2], area);
            }
            else
            {
                draw_svg_to_canvas(canvas, bitmaps[0], area);
            }
        }
        void rasterize_single_run(sprite& glyph_mask, fp2d base_line, fonts::shaper& fs, bool monochromatic, bool antialiasing)
        {
            auto run_area = rect{};
            auto pen = base_line;
            auto is_colored = faux;
            auto colored_glyph_mode = FT_LOAD_NO_HINTING;
            auto monochromatic_mode = antialiasing ? FT_LOAD_FORCE_AUTOHINT : FT_LOAD_TARGET_MONO;
            for (auto& glyph : fs.glyphs) // Calc bounding box and check colors.
            {
                glyph.color = fonts::color_type::mono;
                if (!monochromatic)
                {
                    if (fs.colored == fonts::color_type::svg)
                    {
                        if (FT_Err_Ok == ::FT_Load_Glyph(fs.ft_face, glyph.index, FT_LOAD_COLOR))
                        {
                            if (fs.ft_face->glyph->format == FT_GLYPH_FORMAT_SVG)
                            {
                                glyph.color = fonts::color_type::svg;
                                is_colored = true;
                            }
                        }
                    }
                    else if (fs.colored == fonts::color_type::colr)
                    {
                        auto it = FT_LayerIterator{};
                        auto l_glyph = FT_UInt{};
                        auto l_color = FT_UInt{};
                        if (::FT_Get_Color_Glyph_Layer(fs.ft_face, glyph.index, &l_glyph, &l_color, &it))
                        {
                            glyph.color = fonts::color_type::colr;
                            is_colored = true; // Has a colored glyph within the run.
                        }
                    }
                }
                auto metrics_load_flags = glyph.color != fonts::color_type::mono ? colored_glyph_mode : monochromatic_mode; // Force AA in case of colored glyph.
                if (FT_Err_Ok == ::FT_Load_Glyph(fs.ft_face, glyph.index, metrics_load_flags))
                {
                    auto& metrics = fs.ft_face->glyph->metrics;
                    if (metrics.width && metrics.height)
                    {
                        auto horiBearingX = metrics.horiBearingX / 64.f;
                        auto horiBearingY = metrics.horiBearingY / 64.f;
                        auto glyph_area_coor = fp2d{ pen.x + glyph.align.x + horiBearingX, pen.y - (glyph.align.y + horiBearingY) };
                        auto glyph_area_size = fp2d{ metrics.width / 64.f, metrics.height / 64.f };
                        glyph.b_box.coor = std::floor(glyph_area_coor);
                        glyph.b_box.size = std::ceil(glyph_area_coor + glyph_area_size) - glyph.b_box.coor;
                        if (run_area)
                        {
                            run_area |= glyph.b_box;
                        }
                        else
                        {
                            run_area = glyph.b_box;
                        }
                    }
                    else
                    {
                        glyph.b_box = {};
                    }
                    pen.x += glyph.width;
                }
            }
            if (run_area)
            {
                pen = base_line;
                glyph_mask.set_area<irgb>(run_area, is_colored);
                auto render_fx = [&](auto canvas)
                {
                    for (auto& glyph : fs.glyphs) // Rasterize.
                    {
                        if (glyph.color == fonts::color_type::colr) // Colored glyph (COLR).
                        {
                            auto it = FT_LayerIterator{};
                            auto l_glyph = FT_UInt{};
                            auto l_color = FT_UInt{};
                            while (::FT_Get_Color_Glyph_Layer(fs.ft_face, glyph.index, &l_glyph, &l_color, &it))
                            {
                                if (FT_Err_Ok == ::FT_Load_Glyph(fs.ft_face, l_glyph, colored_glyph_mode | FT_LOAD_RENDER))
                                {
                                    auto* slot = fs.ft_face->glyph;
                                    auto fill = irgb{};
                                    if (l_color != 0xFFFF) // Use fgc if 0xFFFF or linear color from palette otherwise.
                                    {
                                        auto& cached_palette = fs.bare_face_ptr->palette;
                                        if (l_color < cached_palette.size())
                                        {
                                            fill = cached_palette[l_color];
                                        }
                                        else
                                        {
                                            fill.a = 1.f; // Fallback to black on errors.
                                        }
                                    }
                                    draw_layer_to_canvas(canvas, slot, pen, glyph.align, fill);
                                }
                            }
                        }
                        else if (glyph.color == fonts::color_type::svg) // Colored glyph (SVG).
                        {
                            if (FT_Err_Ok == ::FT_Load_Glyph(fs.ft_face, glyph.index, FT_LOAD_COLOR))
                            {
                                auto slot = fs.ft_face->glyph;
                                auto doc = (FT_SVG_Document)slot->other;
                                if (doc->svg_document_length)
                                {
                                    //log(text{ (char const*)doc->svg_document, doc->svg_document_length });
                                    //todo suggest that the lunasvg project do var(...) resolving on their side
                                    //todo suggest that the lunasvg project use custom allocators, this will solve memory allocation issues in one fell swoop

                                    // Looking for the cached lunasvg::Documents.
                                    auto svg_doc_id = (doc->start_glyph_id << 16) | doc->end_glyph_id;
                                    auto& svg_cache = fs.bare_face_ptr->svg_cache;
                                    auto svg_doc_iter = svg_cache.find(svg_doc_id);
                                    if (svg_doc_iter == svg_cache.end())
                                    {
                                        auto svg_data = std::span{ (char*)doc->svg_document, doc->svg_document_length };
                                        auto svg_DOM = generate_DOM(svg_data);
                                        svg_doc_iter = svg_cache.emplace(svg_doc_id, std::move(svg_DOM)).first;
                                    }
                                    static thread_local auto glyph_id = text{};
                                    glyph_id = "glyph" + std::to_string(glyph.index); // According to the OT-SVG standard, each glyph within a document must be contained within an element with id="glyph<index>".
                                    rasterize_glyph_DOM(canvas, svg_doc_iter->second, glyph_id, glyph.b_box);
                                }
                            }
                        }
                        else // Grayscale anti-aliasing or aliased B/W.
                        {
                            if (FT_Err_Ok == ::FT_Load_Glyph(fs.ft_face, glyph.index, monochromatic_mode | FT_LOAD_RENDER))
                            {
                                auto slot = fs.ft_face->glyph;
                                draw_layer_to_canvas(canvas, slot, pen, glyph.align);
                            }
                        }
                        pen.x += glyph.width;
                    }
                };
                is_colored ? render_fx(glyph_mask.raster<irgb>())
                           : render_fx(glyph_mask.raster<byte>());
            }
        }
        void rasterize(sprite& glyph_mask, cell const& c)
        {
            glyph_mask.type = sprite::alpha;
            if (c.xy() == 0) return;
            auto code_iter = utf::cpit{ c.txt() };
            codepoints.clear();
            auto flip_swap = 0;
            auto monochromatic = faux;
            auto img_alignment = bind{ snap::none, snap::none };
            while (code_iter)
            {
                auto codepoint = code_iter.next();
                if (codepoint.cdpoint >= utf::vs04_code && codepoint.cdpoint <= utf::vs16_code)
                {
                         if (codepoint.cdpoint == utf::vs15_code) monochromatic = true;
                    else if (codepoint.cdpoint == utf::vs16_code) monochromatic = faux;
                    else if (codepoint.cdpoint == utf::vs10_code) imagens::rotate_fx(flip_swap, imagens::ccw::r90 ); // Rotate 90° CCW
                    else if (codepoint.cdpoint == utf::vs11_code) imagens::rotate_fx(flip_swap, imagens::ccw::r180); // Rotate 180° (=FlipX+FlipY)
                    else if (codepoint.cdpoint == utf::vs12_code) imagens::rotate_fx(flip_swap, imagens::ccw::r270); // Rotate 270° CCW
                    else if (codepoint.cdpoint == utf::vs13_code) imagens::mirror_fx(flip_swap, imagens::flips::hz); // Horizontal flip (FlipX)
                    else if (codepoint.cdpoint == utf::vs14_code) imagens::mirror_fx(flip_swap, imagens::flips::vt); // Vertical flip (FlipY)
                    else if (codepoint.cdpoint == utf::vs04_code) img_alignment.x = snap::head;
                    else if (codepoint.cdpoint == utf::vs05_code) img_alignment.x = snap::center;
                    else if (codepoint.cdpoint == utf::vs06_code) img_alignment.x = snap::tail;
                    else if (codepoint.cdpoint == utf::vs07_code) img_alignment.y = snap::head;
                    else if (codepoint.cdpoint == utf::vs08_code) img_alignment.y = snap::center;
                    else if (codepoint.cdpoint == utf::vs09_code) img_alignment.y = snap::tail;
                }
                else if (utf::non_control(codepoint.cdpoint))
                {
                    codepoints.push_back(codepoint.cdpoint);
                }
            }
            if (codepoints.empty()) return;

            auto format = font_style::regular;
            if (c.itc()) format |= font_style::italic;
            if (c.bld()) format |= font_style::bold;

            auto& f2 = fcache.find_family(codepoints);
            auto& f = f2.select_font_face(codepoints, format);
            if (!f.fthb_pair_cache[0].fthb_pair) return;
            monochromatic |= !f.bare_face_ptr->is_color;

            auto base_char = codepoints.front();
            auto is_box_drawing = base_char >= 0x2320  && (base_char <= 0x23D0   // ⌠ ⌡ ... ⎛ ⎜ ⎝ ⎞ ⎟ ⎠ ⎡ ⎢ ⎣ ⎤ ⎥ ⎦ ⎧ ⎨ ⎩ ⎪ ⎫ ⎬ ⎭ ⎮ ⎯ ⎰ ⎱ ⎲ ⎳ ⎴ ⎵ ⎶ ⎷ ⎸ ⎹ ... ⏐
                              || (base_char >= 0x2500  && (base_char <  0x259F   // Box Elements
                              //|| (base_char >= 0x25A0  && (base_char <= 0x25FF   // Geometric Shapes
                              || (base_char >= 0xE0B0  && (base_char <= 0xE0B3   // Powerline Arrows
                              || (base_char >= 0x1CC00 && (base_char <= 0x1CEBF  // Legacy Computing Supplement. inc Large Type Pieces: U+1CE1A-1CE50
                              || (base_char >= 0x1F67C && (base_char <= 0x1F67F  // Ornamental Dingbats: U+1F67C-1F67F 🙼 🙽 🙾 🙿
                              || (base_char >= 0x1FB00 && (base_char <= 0x1FBFF))))))))))); // Symbols for Legacy Computing
            auto transform = is_box_drawing ? f.transform : f.transform_letters;
            auto em_height = is_box_drawing ? f.em_height : f.em_height_letters;
            auto base_line = f.base_line;
            auto actual_height = (fp32)cellsz.y;
            auto grid_step = (fp32)cellsz.x;
            auto mtx = c.mtx();
            auto matrix = fp2d{ mtx * cellsz };
            flip_swap = imagens::xlate[flip_swap & 7]; // Convert between the "pull-based" and the "push-based" renderer (we accumulate tranformations in "pull_based").
            auto swapxy = flip_swap & 1;
            if (swapxy)
            {
                std::swap(matrix.x, matrix.y);
                std::swap(mtx.x, mtx.y);
                auto ratio = grid_step / actual_height;
                transform     *= ratio;
                em_height     *= ratio;
                base_line     *= ratio;
                actual_height *= ratio;
                grid_step     *= ratio;
            }

            //todo revise
            auto hb_ufuncs = ::hb_unicode_funcs_get_default();
            auto hb_script = ::hb_unicode_script(hb_ufuncs, base_char); // Map from 0-999 numbers to 4-letter ISO tags (like 215 -> "Latn").
            auto is_rtl = ::hb_script_get_horizontal_direction(hb_script) == HB_DIRECTION_RTL;
            auto cache_index = mtx.y == 1 && !swapxy ? (si32)is_box_drawing
                                                     : (si32)is_box_drawing + 2;
            auto [hb_font, ft_face] = f.select_face_by_em_height(em_height, cache_index);
            fcache.font_shaper.hb_font = hb_font;
            fcache.font_shaper.ft_face = ft_face;
            fcache.font_shaper.colored = f2.color;
            fcache.font_shaper.bare_face_ptr = f.bare_face_ptr;
            auto length = fcache.font_shaper.generate_glyph_run(codepoints, hb_script, is_rtl, transform, f2.monospaced, grid_step); // Generate glyph run and get its length in pixels (fp32).
            auto actual_width = swapxy ? std::max(1.f, length) :
                        is_box_drawing ? std::max(1.f, std::floor((length / cellsz.x))) * cellsz.x
                                       : std::max(1.f, std::ceil(((length - (mtx.y/*glyph scale*/ * 0.114f/*min=0.113: Bold+Italic Courier V*/) * cellsz.x) / cellsz.x))) * cellsz.x;
            auto k = 1.f;
            if (actual_width > matrix.x) // Check if the glyph exceeds the matrix width. (scale down)
            {
                k = matrix.x / length;
                length       = matrix.x;
                actual_width = matrix.x;
                actual_height *= k;
                em_height     *= k;
                for (auto& glyph : fcache.font_shaper.glyphs)
                {
                    glyph.width *= k;
                    glyph.align *= k;
                }
            }
            else if (actual_height < matrix.y || actual_width < matrix.x) // Check if the glyph is too small for the matrix. (scale up)
            {
                k = std::min(matrix.x / actual_width, matrix.y / actual_height);
                length        *= k;
                actual_width  *= k;
                actual_height *= k;
                base_line     *= k;
                em_height     *= k;
                for (auto& glyph : fcache.font_shaper.glyphs)
                {
                    glyph.width *= k;
                    glyph.align *= k;
                }
                k = 1.f;
            }
            if (!f2.monospaced && !is_box_drawing)
            {
                auto offset = matrix.x - length; // This (using length) will allow us to seamlessly connect the two fragments.
                if (img_alignment.x == snap::none || img_alignment.x == snap::center)
                {
                    if (is_rtl) base_line.x -= offset / 2.f; // Centrify actual proportional glyph as is.
                    else        base_line.x += offset / 2.f; //
                }
                else if (img_alignment.x == snap::tail)
                {
                    if (is_rtl) base_line.x -= offset;
                    else        base_line.x += offset;
                }
            }
            else if (img_alignment.x != snap::none && actual_width < matrix.x)
            {
                auto offset = matrix.x - actual_width;
                if (img_alignment.x == snap::center)
                {
                    if (is_rtl) base_line.x -= offset / 2.f; // Center the cell containing the glyph, not the glyph outline itself.
                    else        base_line.x += offset / 2.f; //
                }
                else if (img_alignment.x == snap::tail)
                {
                    if (is_rtl) base_line.x -= offset;
                    else        base_line.x += offset;
                }
                //else if (img_alignment.x == snap::head) base_line.x = 0;
            }
            if (img_alignment.y != snap::none && actual_height < matrix.y)
            {
                base_line.y *= k;
                     if (img_alignment.y == snap::center) base_line.y += (matrix.y - actual_height) / 2.f;
                else if (img_alignment.y == snap::tail  ) base_line.y += matrix.y - actual_height;
                //else if (img_alignment.y == snap::head  ) base_line.y *= k;
            }

            //todo optimize
            auto [hb_font2, ft_face2] = f.select_face_by_em_height(em_height, cache_index);
            fcache.font_shaper.hb_font = hb_font2;
            fcache.font_shaper.ft_face = ft_face2;
            rasterize_single_run(glyph_mask, base_line, fcache.font_shaper, monochromatic, aamode);

            //auto src_bitmap = glyph_mask.raster<byte>();
            //auto bline = rect{base_line, { cellsz.x, 1 } };
            //netxs::onrect(src_bitmap, bline, [](auto& c){ c = std::min(255, c + 64); });
            if (glyph_mask.area && flip_swap)
            {
                glyph_mask.transform<irgb>(flip_swap, matrix);
            }
        }
        void rasterize_svg_document(imagens::image& image)
        {
            if (!image.dom[0])
            {
                image.dom = generate_DOM(image.document);
            }
            if (image.dom[0])
            {
                auto& image_dom = *image.dom[0];
                auto original_doc_size_fpx = fp2d{ image_dom.width(), image_dom.height() }; // Original doc size (float).
                if (!std::isnormal(original_doc_size_fpx.x) || !std::isnormal(original_doc_size_fpx.y))
                {
                    image.fragment.set_area<irgb>(rect{});
                    return;
                }
                auto final_doc_size_fpx = original_doc_size_fpx; // Rendered doc size (float).

                auto attrs = std::array<fp32, imagens::attr_count>{};
                for (auto i = 0; i < imagens::attr_count; i++)
                {
                    if (auto v = image.attrs[i]) attrs[i] = v.value();
                }
                auto width     = attrs[imagens::width    ];
                auto height    = attrs[imagens::height   ];
                auto scale     = attrs[imagens::scale    ];
                auto transform = attrs[imagens::transform];
                auto wh_fp = fp2d{ width, height };
                auto bounding_rect_pixels = std::round(wh_fp * cellsz); // Document bounding box size in pixels.
                if ((si32)transform & 1)
                {
                    std::swap(bounding_rect_pixels.x, bounding_rect_pixels.y);
                }
                auto ratio = bounding_rect_pixels / original_doc_size_fpx;
                auto keep_ratio = true;
                switch ((si32)scale)
                {
                    case scale_mode::none:    /*final_doc_size_fpx = final_doc_size_fpx;*/ break;
                    case scale_mode::inside:  final_doc_size_fpx *= std::min(ratio.x, ratio.y); break;
                    case scale_mode::outside: final_doc_size_fpx *= std::max(ratio.x, ratio.y); break;
                    case scale_mode::stretch: final_doc_size_fpx = bounding_rect_pixels; keep_ratio = faux; /* matrix_pixels = matrix_pixels */ break;
                }
                // Apply system limits.
                auto px_limits = std::max(dot_11, skin::globals().max_value * cellsz);
                final_doc_size_fpx = std::clamp(final_doc_size_fpx, fp2d{ dot_11 }, fp2d{ px_limits });

                auto matrix_pixels = twod{ std::ceil(final_doc_size_fpx) };
                image.document_area = rect{ dot_00, matrix_pixels };
                static thread_local auto full_doc_tmp_buffer = netxs::sprite{ *std::pmr::new_delete_resource() };
                full_doc_tmp_buffer.set_area<irgb>(image.document_area);
                auto tmp_document_block = full_doc_tmp_buffer.raster<irgb>();
                tmp_document_block.zeroize();
                rasterize_svg_DOM(tmp_document_block, image.dom, original_doc_size_fpx, image.sub_id, keep_ratio);
                // Trim all transparent pixels.
                auto nested_fragment_area = full_doc_tmp_buffer.get_minimal_non_transparent_area_for_pma<irgb>();
                image.fragment.set_area<irgb>(nested_fragment_area);
                if (nested_fragment_area)
                {
                    auto dst_fragment_block = image.fragment.raster<irgb>();
                    netxs::onbody(tmp_document_block, dst_fragment_block, [](auto& src, auto& dst){ dst = src; });
                }
            }
        }
        void draw_glyph(auto& canvas, sprite& glyph_mask, twod offset, argb fgc, bool semi_transparent = faux)
        {
            auto box = glyph_mask.area.shift(offset);
            auto f_fgc = irgb::nonpma_srgb_to_pma_linear(fgc);
            if (glyph_mask.type == sprite::color)
            {
                auto global_alpha = semi_transparent ? 0.5f : 1.0f; // Triggers on SGR 7 (reverse video). Makes emoji semi-transparent on selection.
                auto fx = [f_fgc, global_alpha](argb& dst, irgb src) //todo unify: same code in draw_image()
                {
                    if (src.a == 0.f) return;
                    auto f_dst = irgb::pma_srgb_to_pma_linear(dst);
                    if (src.has_extra_alpha())
                    {
                        auto fgc_alpha = src.unpack_alpha();
                        if (fgc_alpha > 0)
                        {
                            f_dst.blend_pma(f_fgc, fgc_alpha);
                        }
                    }
                    dst = irgb::pma_linear_to_pma_srgb(f_dst.blend_pma(src * global_alpha));
                };
                auto raster = netxs::raster{ std::span{ (irgb*)glyph_mask.bits.data(), (size_t)glyph_mask.area.length() }, box };
                netxs::onclip(canvas, raster, fx);
            }
            else
            {
                auto fx = [fgc, f_fgc](argb& dst, byte src)
                {
                    if (src == 0) return;
                    if (src == 255)
                    {
                        dst = fgc;
                    }
                    else
                    {
                        auto f_dst = irgb::pma_srgb_to_pma_linear(dst);
                        dst = irgb::pma_linear_to_pma_srgb(f_dst.blend_pma(f_fgc, src));
                    }
                };
                auto raster = netxs::raster{ std::span{ (byte*)glyph_mask.bits.data(), (size_t)glyph_mask.area.length() }, box };
                netxs::onclip(canvas, raster, fx);
            }
        }
        void draw_image(auto& canvas, imagens::image& image, twod offset, argb fgc, bool semi_transparent, si32 xform)
        {
            auto& image_mask = image.fragment;
            auto f_fgc = irgb::nonpma_srgb_to_pma_linear(fgc);
            auto global_alpha = semi_transparent ? 0.5f : 1.0f; // Triggers on SGR 7 (reverse video).
            assert(image_mask.type == sprite::color);
            auto raster = netxs::raster{ std::span{ (irgb*)image_mask.bits.data(), (size_t)image_mask.area.length() }, image_mask.area };
            auto fx = [f_fgc, global_alpha](argb& dst, irgb src) //todo unify: same code in draw_glyph()
            {
                if (src.a == 0.f) return;
                auto f_dst = irgb::pma_srgb_to_pma_linear(dst);
                if (src.has_extra_alpha())
                {
                    auto fgc_alpha = src.unpack_alpha();
                    if (fgc_alpha > 0)
                    {
                        f_dst.blend_pma(f_fgc, fgc_alpha);
                    }
                }
                dst = irgb::pma_linear_to_pma_srgb(f_dst.blend_pma(src * global_alpha));
            };
            // xform on write
            auto canvas_clip = canvas.clip(); // Cell placeholder.
            auto document_area = image.document_area.shift(offset); // Raster inside the document.
            netxs::xform_render(canvas, canvas_clip, raster, document_area, xform, fx);
        }
        auto render_image(auto& canvas, rect placeholder, argb fgc, cell const& c)
        {
            if (auto image_index = c.get_image_index())
            if (auto image_xy = c.get_image_xy(); image_xy.x != 0 && image_xy.y != 0)
            {
                auto image_align = c.get_image_align();
                auto image_xform = c.get_image_xform();
                auto images = cell::images(); // Lock.
                if (auto image_ptr = images.exists(image_index)) // We form all image requests on dtvt recv stage.
                {
                    auto& image = *image_ptr;
                    //todo diff by sizes
                    if (image.fragment.type == sprite::undef && image.document.size())
                    {
                        rasterize_svg_document(image);
                    }
                    if (image.fragment.area)
                    {
                        auto& _dx = image.attrs[imagens::dx];
                        auto& _dy = image.attrs[imagens::dy];
                        auto dxy = twod{ _dx ? std::round(cellsz.x * _dx.value()) : 0,
                                         _dy ? std::round(cellsz.y * _dy.value()) : 0 };
                        // Alignment.
                        auto width  = image.attrs[imagens::width ].value();
                        auto height = image.attrs[imagens::height].value();
                        auto wh_fp = fp2d{ width, height };
                        auto cellcanvas_size = twod{ std::ceil(wh_fp) * cellsz }; // Cellrect in pixels (outer rect).

                        //auto get_factor = [](auto align)
                        //{
                        //    if (align == (si32)bias::center || !align) return 0.5f;
                        //    if (align == (si32)bias::right)            return 1.0f;
                        //    return 0.0f;
                        //};
                        //auto factors = fp2d{ get_factor(image_align & 0b0011), get_factor(image_align >> 2) };
                        //auto fragment_area_coor = image.fragment.area.coor + (cellcanvas_size - image.document_area.size) * factors;

                        auto get_off = [](auto align, auto diff)
                        {
                            return (align == (si32)bias::right)            ? diff
                                 : (align == (si32)bias::center || !align) ? diff / 2
                                                                           : 0;
                        };
                        auto fragment_area_coor = image.fragment.area.coor + twod{ get_off(image_align & 0b0011, cellcanvas_size.x - image.document_area.size.x),
                                                                                   get_off(image_align >> 2,     cellcanvas_size.y - image.document_area.size.y) };

                        // Rendering.
                        image_xy = (image_xy - dot_11) * cellsz;
                        auto offset = placeholder.coor - image_xy + dxy + fragment_area_coor;
                        draw_image(canvas, image, offset, fgc, c.inv(), image_xform);
                    }
                }
            }
        }
        void draw_shadows(auto& canvas, rect placeholder, cell const& c)
        {
            if (auto shadow = c.dim()) // Render shadow if it is.
            {
                auto& shadow_raster = cgi_shadow[shadow];
                auto offset = placeholder.coor;
                draw_glyph(canvas, shadow_raster, offset, argb{ tint::pureblack });
            }
        }
        template<class T = noop>
        void draw_cell(auto& canvas, rect placeholder, cell const& c, T&& blink_canvas = {})
        {
            static constexpr auto blink_canvas_specified = !std::is_same_v<std::decay_t<T>, noop>;
            placeholder.trimby(canvas.area());
            if (!placeholder) return;
            auto image_ontop = c.get_image_ontop();
            auto blinking = blink_canvas_specified && c.blk() && !c.hid();
            auto fgc = c.fgc();
            auto bgc = c.bgc();
            if (c.inv()) std::swap(fgc, bgc);

            // Build background.
            canvas.clip(placeholder);
            auto target_ptr = &canvas;
            netxs::onrect(canvas, placeholder, cell::shaders::full(bgc));
            if (!image_ontop)
            {
                render_image(canvas, placeholder, fgc, c);
            }
            if constexpr (blink_canvas_specified)
            if (blinking) // Copy background to the the blinking layer to fix DWM that doesn't take gamma into account during layered window blending.
            {
                target_ptr = &blink_canvas;
                blink_canvas.clip(placeholder);
                netxs::onclip(canvas, blink_canvas, [&](auto& dst, auto& src){ src = dst; });
            }

            auto& target = *target_ptr;
            while (!c.hid()) // Render visible glyph.
            {
                if (auto u = c.und())
                {
                    auto index = c.unc();
                    auto color = index ? argb{ argb::vt256[index] }.alpha(fgc.alpha()) : fgc;
                    if (u == unln::line)
                    {
                        auto block = fcache.underline;
                        block.coor += placeholder.coor;
                        netxs::onrect(target, block, cell::shaders::full(color));
                    }
                    else if (u == unln::dotted)
                    {
                        auto block = fcache.underline;
                        block.coor += placeholder.coor;
                        auto limit = block.coor.x + block.size.x;
                        block.size.x = std::max(2, block.size.y);
                        auto stepx = 3 * block.size.x;
                        block.coor.x -= netxs::grid_mod(placeholder.coor.x, stepx);
                        while (block.coor.x < limit)
                        {
                            netxs::onrect(target, block.trim(placeholder), cell::shaders::full(color));
                            block.coor.x += stepx;
                        }
                    }
                    else if (u == unln::dashed)
                    {
                        auto block = fcache.dashline;
                        block.coor += placeholder.coor;
                        netxs::onrect(target, block, cell::shaders::full(color));
                    }
                    else if (u == unln::biline)
                    {
                        auto b1 = fcache.doubline1;
                        auto b2 = fcache.doubline2;
                        auto offset = placeholder.coor;
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
                        offset.x -= netxs::grid_mod(offset.x, fract4);
                        draw_glyph(target, wavy_raster, offset, color);
                    }
                    else
                    {
                        auto block = fcache.underline;
                        block.coor += placeholder.coor;
                        netxs::onrect(target, block, cell::shaders::full(color));
                    }
                }
                if (c.stk())
                {
                    auto block = fcache.strikeout;
                    block.coor += placeholder.coor;
                    netxs::onrect(target, block, cell::shaders::full(fgc));
                }
                if (c.ovr())
                {
                    auto block = fcache.overline;
                    block.coor += placeholder.coor;
                    netxs::onrect(target, block, cell::shaders::full(fgc));
                }
                if (c.xy() == 0)
                {
                    break;
                }
                auto token = c.tkn();
                if (c.itc()) token ^= 0xAAAA'AAAA'AAAA'AA00; // Randomize token to differentiate italics (0xb101010...0000'0000 excluding matrix metadata).
                if (c.bld()) token ^= 0x5555'5555'5555'5500; // Randomize token to differentiate bolds (0xb010101...0000'0000 excluding matrix metadata).
                auto iter = glyphs.find(token);
                if (iter == glyphs.end())
                {
                    if (c.jgc())
                    {
                        iter = glyphs.emplace(token, mono_buffer).first;
                    }
                    else break;
                }
                auto& glyph_mask = iter->second;
                if (glyph_mask.type == sprite::undef)
                {
                    if (c.jgc())
                    {
                        rasterize(glyph_mask, c);
                    }
                    else break;
                }
                if (glyph_mask.area)
                {
                    auto [w, h, x, y] = c.whxy();
                    if (x != 0 && y != 0)
                    {
                        auto offset = placeholder.coor - twod{ cellsz.x * (x - 1), cellsz.y * (y - 1) };
                        //todo implement a contour or a shadow layer
                        //if (bgc.alpha() < 2 && fgc == argb{ purewhite })
                        //{
                        //    auto blk = argb{ pureblack };
                        //    draw_glyph(target, glyph_mask, offset - dot_10, blk);
                        //    draw_glyph(target, glyph_mask, offset + dot_10, blk);
                        //    draw_glyph(target, glyph_mask, offset - dot_01, blk);
                        //    draw_glyph(target, glyph_mask, offset + dot_01, blk);
                        //    draw_glyph(target, glyph_mask, offset - dot_11, blk);
                        //    draw_glyph(target, glyph_mask, offset + dot_11, blk);
                        //    draw_glyph(target, glyph_mask, offset - dot_01 + dot_10, blk);
                        //    draw_glyph(target, glyph_mask, offset + dot_01 - dot_10, blk);
                        //}
                        draw_glyph(target, glyph_mask, offset, fgc, c.inv());
                    }
                }
                //if (bgc.alpha()< 2 && fgc == argb{ purewhite })
                //{
                //    // hilight glyph edges
                //}
                break;
            }

            if (image_ontop)
            {
                if (blinking)
                {
                    if constexpr (blink_canvas_specified)
                    {
                        render_image(canvas, placeholder, bgc, c);
                        draw_shadows(canvas, placeholder, c);

                        render_image(target, placeholder, fgc, c);
                        draw_shadows(target, placeholder, c);
                    }
                }
                else
                {
                    render_image(canvas, placeholder, fgc, c);
                    draw_shadows(canvas, placeholder, c);
                }
            }
            else
            {
                draw_shadows(canvas, placeholder, c);
                if constexpr (blink_canvas_specified)
                if (blinking)
                {
                    draw_shadows(target, placeholder, c);
                }
            }
        }
    };

    struct winbase : base
    {
        using grip = netxs::misc::szgrips;
        using s11n = netxs::directvt::binary::s11n;
        using b256 = std::array<byte, 256>;
        using title = ui::pro::title;
        using focus = ui::pro::focus;
        using kmap = input::key::kmap;

        static constexpr auto classname = basename::gui_window;
        static constexpr auto shadow_dent = dent{ dot_11 } * 3;
        static constexpr auto wheel_delta_base = 120; // WHEEL_DELTA

        struct blink
        {
            span init{}; // blink: Specified blinking interval.
            span rate{}; // blink: Current blinking interval.
            bool show{}; // blink: Blinking layer is active.
            byts mask{}; // blink: Blinking cells map.
            si32 poll{}; // blink: Blinking cells count.
        };
        struct bttn
        {
            static constexpr auto left     = 1 << 0;
            static constexpr auto right    = 1 << 1;
            static constexpr auto middle   = 1 << 2;
            static constexpr auto xbutton1 = 1 << 3;
            static constexpr auto xbutton2 = 1 << 4;
        };
        struct by
        {
            static constexpr auto mouse = 1 << 0;
            static constexpr auto keybd = 1 << 1;
        };
        struct timers
        {
            static constexpr auto _counter   = __COUNTER__ + 1;
            static constexpr auto none       = __COUNTER__ - _counter;
            static constexpr auto blink      = __COUNTER__ - _counter;
            static constexpr auto clipboard  = __COUNTER__ - _counter;
            static constexpr auto rightshift = __COUNTER__ - _counter;
        };
        struct ipc
        {
            #define ipc_t \
            X(make_offer) /* Make group focus offer on Ctrl+Click. */ \
            X(drop_focus) /* Order to drop any focus.              */ \
            X(take_focus) /* Order to take OS focus.               */ \
            X(main_focus) /* Advertise OS focus owner.             */ \
            X(solo_focus) /* Set solo focus.                       */ \
            X(sync_state) /* Sync keybd modifiers state with OS.   */ \
            X(pass_state) /* Pass keybd modifiers state.           */ \
            X(pass_input) /* Pass keybd input.                     */ \
            X(expose_win) /* Order to expose window.               */ \
            X(make_ontop) /* Order to make window topmost.         */ \
            X(set_normal) /* Order to make window notopmost.       */ \
            X(no_command) /* Noop. Just to update.                 */ \
            X(cmd_w_data) /* Command with payload.                 */
            static constexpr auto _base = 99900;
            static constexpr auto _counter = __COUNTER__ + 1 - _base;
            #define X(cmd) static constexpr auto cmd = __COUNTER__ - _counter;
            ipc_t
            #undef X
            static constexpr auto _str = std::to_array({
                #define X(cmd) #cmd,
                ipc_t
                #undef X
            });
            #undef ipc_t
            static auto str(auto cmd) { return _str[cmd - _base]; }
        };
        struct task
        {
            static constexpr auto _counter = 1 + __COUNTER__;
            static constexpr auto blink    = 1 << (__COUNTER__ - _counter);
            static constexpr auto moved    = 1 << (__COUNTER__ - _counter);
            static constexpr auto sized    = 1 << (__COUNTER__ - _counter);
            static constexpr auto grips    = 1 << (__COUNTER__ - _counter);
            static constexpr auto hover    = 1 << (__COUNTER__ - _counter);
            static constexpr auto inner    = 1 << (__COUNTER__ - _counter);
            static constexpr auto header   = 1 << (__COUNTER__ - _counter);
            static constexpr auto footer   = 1 << (__COUNTER__ - _counter);
            static constexpr auto tooltip  = 1 << (__COUNTER__ - _counter);
            static constexpr auto all = -1;
        };
        struct vkey
        {
            static constexpr auto lbutton  = 0x01; // VK_LBUTTON;
            static constexpr auto rbutton  = 0x02; // VK_RBUTTON;
            static constexpr auto mbutton  = 0x04; // VK_MBUTTON;
            static constexpr auto xbutton1 = 0x05; // VK_XBUTTON1;
            static constexpr auto xbutton2 = 0x06; // VK_XBUTTON2;

            static constexpr auto shift    = 0x10; // VK_SHIFT;
            static constexpr auto control  = 0x11; // VK_CONTROL;
            static constexpr auto alt      = 0x12; // VK_MENU;
            static constexpr auto lshift   = 0xA0; // VK_LSHIFT;
            static constexpr auto rshift   = 0xA1; // VK_RSHIFT;
            static constexpr auto lcontrol = 0xA2; // VK_LCONTROL;
            static constexpr auto rcontrol = 0xA3; // VK_RCONTROL;
            static constexpr auto lalt     = 0xA4; // VK_LMENU;
            static constexpr auto ralt     = 0xA5; // VK_RMENU;
            static constexpr auto lwin     = 0x5B; // VK_LWIN;
            static constexpr auto rwin     = 0x5C; // VK_RWIN;

            static constexpr auto enter    = 0x0D; // VK_RETURN;
            static constexpr auto left     = 0x25; // VK_LEFT;
            static constexpr auto up       = 0x26; // VK_UP;
            static constexpr auto right    = 0x27; // VK_RIGHT;
            static constexpr auto down     = 0x28; // VK_DOWN;
            static constexpr auto end      = 0x23; // VK_END;
            static constexpr auto home     = 0x24; // VK_HOME;

            static constexpr auto f11      = 0x7A; // VK_F11;
            static constexpr auto f12      = 0x7B; // VK_F12;

            static constexpr auto key_0    = '0'; // VK_0;

            static constexpr auto numlock  = 0x90; // VK_NUMLOCK;
            static constexpr auto capslock = 0x14; // VK_CAPITAL;
            static constexpr auto scrllock = 0x91; // VK_SCROLL;
            static constexpr auto kana     = 0x15; // VK_KANA;
            static constexpr auto oem_loya = 0x95; // VK_OEM_FJ_LOYA;
            static constexpr auto oem_roya = 0x96; // VK_OEM_FJ_ROYA;

            static constexpr auto oem_copy = 0xF2; // VK_OEM_COPY;
            static constexpr auto oem_auto = 0xF3; // VK_OEM_AUTO;
            static constexpr auto oem_enlw = 0xF4; // VK_OEM_ENLW;

            static constexpr auto packet   = 0xE7; // VK_PACKET;
        };
        struct cont
        {
            si32  cmd;
            void* ptr;
            ui32  len;
        };
        struct foci
        {
            using uset = std::unordered_set<ui32>;
            using sync = std::mutex;
            sync mutex{}; // foci: Mutex.
            uset group{}; // foci: Members.
            ui32 owner{}; // foci: Leader id.
            bool buson{}; // foci: Focus indication (bus) is on.
            bool angel{}; // foci: Window is a member of the multi-focus group.
            bool wheel{}; // foci: Window is a leader of the multi-focus group.
            bool offer{}; // foci: Window just received a multi-focus offer.

            void  insert(ui32 target) { auto lock = std::lock_guard{ mutex }; group.insert(target); }
            void   erase(ui32 target) { auto lock = std::lock_guard{ mutex }; group.erase(target); }
            bool   clear()            { auto lock = std::lock_guard{ mutex }; group.clear(); owner = {}; angel = faux; return std::exchange(buson, faux); }
            auto    size()            { auto lock = std::lock_guard{ mutex }; return group.size(); }
            auto    copy()            { auto lock = std::lock_guard{ mutex }; return std::vector<ui32>(group.begin(), group.end()); }
            auto  active()            { auto lock = std::lock_guard{ mutex }; return wheel && group.size() > 1; }
            bool focused()            { auto lock = std::lock_guard{ mutex }; return wheel || angel; }
            auto is_idle()
            {
                auto lock = std::lock_guard{ mutex };
                auto target_list = std::optional<std::vector<ui32>>{};
                if (!wheel && buson && owner) target_list = std::vector<ui32>(group.begin(), group.end());
                return target_list;
            }
            auto set_solo(ui32 local_target)
            {
                auto lock = std::lock_guard{ mutex };
                auto copy = std::vector<ui32>(group.size());
                std::copy_if(group.begin(), group.end(), copy.begin(), [&](auto t){ return t != local_target; });
                group = { local_target };
                angel = faux;
                return copy;
            }
            bool update(auto target_list, auto local_target)
            {
                auto lock = std::lock_guard{ mutex };
                auto ok = !std::exchange(angel, true);
                if (ok)
                {
                    group = { target_list.begin(), target_list.end() };
                    group.insert((ui32)local_target);
                }
                return ok;
            }
            bool set_owner(auto lParam)
            {
                auto lock = std::lock_guard{ mutex };
                owner = (ui32)lParam;
                group.insert(owner);
                return std::exchange(buson, true);
            }
            auto set_focus(auto local_target, auto new_focus_state)
            {
                auto lock = std::lock_guard{ mutex };
                auto target_list = std::optional<std::vector<ui32>>{};
                auto changed = std::exchange(wheel, new_focus_state) != wheel;
                if (changed && (wheel || owner == local_target))
                {
                    if (wheel) group.insert(local_target);
                    target_list = std::vector<ui32>(group.begin(), group.end());
                }
                return std::pair{ changed, target_list };
            }
        };
        struct link : s11n, ui::input_fields_handler
        {
            using input_fields_handler::handle;

            winbase&        owner; // link: .
            ui::pipe&       intio; // link: .
            flag            alive; // link: .

            //todo use gear.m_sys
            input::sysmouse m = {}; // link: .
            input::syswinsz w = {}; // link: .
            input::sysclose c = {}; // link: .
            netxs::sptr<input::hids> gears; // link: .

            auto keybd(hids& gear, auto proc)
            {
                if (alive)
                {
                    auto lock = s11n::syskeybd.freeze();
                    lock.thing.set(gear);
                    lock.thing.sendfx([&](auto block)
                    {
                        proc(block);
                        intio.send(block);
                    });
                }
            };
            auto keybd(auto&& data) { if (alive)                s11n::syskeybd.send(intio, data); }
            auto mouse(auto&& data) { if (alive)                s11n::sysmouse.send(intio, data); }
            auto winsz(auto&& data) { if (alive)                s11n::syswinsz.send(intio, data); }
            auto close(auto&& data) { if (alive.exchange(faux)) s11n::sysclose.send(intio, data); }
            auto fsmod(auto&& data) { if (alive)         data ? s11n::fullscrn.send(intio, gears->id)
                                                              : s11n::restored.send(intio, gears->id); }
            void direct(s11n::xs::bitmap_dtvt lock, view& data)
            {
                auto& bitmap = lock.thing;
                if (owner.reload == task::all || owner.fsmode == winstate::minimized) // We need full repaint.
                {
                    if (owner.fsmode == winstate::minimized) owner.redraw = true;
                    bitmap.get(data, s11n::nat);
                    if (owner.waitsz == bitmap.image.size()) owner.waitsz = dot_00;
                }
                else
                {
                    auto update = [&](auto head, auto iter, auto tail)
                    {
                        if (owner.waitsz == bitmap.image.size()) owner.waitsz = dot_00;
                        if (owner.waitsz || owner.gridsz.x == 0 || owner.cellsz.x == 0) return;
                        auto offset = (si32)(iter - head);
                        auto length = (si32)(tail - iter);
                        auto origin = twod{ offset % owner.gridsz.x, offset / owner.gridsz.x } * owner.cellsz;
                        owner.fill_stripe(iter, tail, origin, offset);
                        // Calc dirty regions.
                        auto len_x = length * owner.cellsz.x;
                        auto width = owner.gridsz.x * owner.cellsz.x;
                        auto max_x = origin.x + len_x - 1;
                        auto end_x = max_x % width + 1;
                        auto end_y = origin.y + (max_x / width + 1) * owner.cellsz.y;
                        if (len_x > width)
                        {
                            auto dirty = rect{{ 0, origin.y }, { width, end_y - origin.y }};
                            dirty.coor += owner.blinky.area.coor;
                            owner.master.strike(dirty);
                        }
                        else
                        {
                            auto dirty = rect{ origin, { origin.x < end_x ? len_x : width - origin.x, owner.cellsz.y }};
                            dirty.coor += owner.blinky.area.coor;
                            owner.master.strike(dirty);
                            if (origin.x >= end_x)
                            {
                                auto remain = rect{{ 0, end_y - owner.cellsz.y }, { end_x, owner.cellsz.y }};
                                remain.coor += owner.blinky.area.coor;
                                owner.master.strike(remain);
                            }
                        }
                    };
                    bitmap.get(data, s11n::nat, update);
                    netxs::set_flag<task::inner>(owner.reload);
                    owner.check_blinky();
                }
            }
            void handle(s11n::xs::img_list         lock)
            {
                s11n::receive_img(lock);
                netxs::set_flag<task::all>(owner.reload); // Trigger to redraw all to update unknown images.
            }
            void handle(s11n::xs::jgc_list         lock)
            {
                s11n::receive_jgc(lock);
                netxs::set_flag<task::all>(owner.reload); // Trigger to redraw all to update jumbo clusters.
            }
            void handle(s11n::xs::header_request   lock)
            {
                auto& item = lock.thing;
                auto header_utf8 = owner.base::riseup(tier::request, e2::form::prop::ui::header);
                s11n::header.send(intio, item.window_id, header_utf8);
            }
            void handle(s11n::xs::footer_request   lock)
            {
                auto& item = lock.thing;
                auto footer_utf8 = owner.base::riseup(tier::request, e2::form::prop::ui::footer);
                s11n::footer.send(intio, item.window_id, footer_utf8);
            }
            void handle(s11n::xs::header           lock)
            {
                auto& item = lock.thing;
                owner.base::riseup(tier::preview, e2::form::prop::ui::header, item.utf8);
            }
            void handle(s11n::xs::footer           lock)
            {
                auto& item = lock.thing;
                owner.base::riseup(tier::preview, e2::form::prop::ui::footer, item.utf8);
            }
            void handle(s11n::xs::tooltips         lock)
            {
                for (auto& tooltip : lock.thing)
                {
                    gears->tooltip.visible = tooltip.utf8.size();
                    if (gears->tooltip.visible)
                    {
                        gears->tooltip.set_text(tooltip.utf8, tooltip.fgc, tooltip.bgc);
                    }
                }
                owner.update_tooltip();
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
            //todo use xs::screenmode
            void handle(s11n::xs::fullscrn       /*lock*/)
            {
                if (owner.fsmode == winstate::maximized) owner.set_state(winstate::normal);
                else                                     owner.set_state(winstate::maximized);
            }
            void handle(s11n::xs::maximize       /*lock*/)
            {
                //todo diff fullscreen and maximized
                if (owner.fsmode == winstate::maximized) owner.set_state(winstate::normal);
                else                                     owner.set_state(winstate::maximized);
            }
            void handle(s11n::xs::minimize       /*lock*/)
            {
                if (owner.fsmode == winstate::minimized) owner.set_state(winstate::normal);
                else                                     owner.set_state(winstate::minimized);
            }
            void handle(s11n::xs::expose         /*lock*/)
            {
                owner.window_post_command(ipc::expose_win);
                //owner.base::enqueue([&](auto& /*boss*/)
                //{
                //    owner.base::riseup(tier::preview, e2::form::layout::expose);
                //});
            }
            void handle(s11n::xs::sysfocus         lock)
            {
                auto f = lock.thing;
                lock.unlock();
                auto guard = owner.sync(); // Guard the owner.This() call.
                auto owner_ptr = owner.This();
                if (f.state)
                {
                    if (owner.mfocus.focused()) // We are the focus tree endpoint.
                    {
                        owner.base::signal(tier::request, input::events::focus::add, { .gear_id = f.gear_id, .focus_type = f.focus_type });
                    }
                    else owner.window_post_command(ipc::take_focus);
                    if (f.focus_type == solo::on) // Set solo focus.
                    {
                        owner.window_post_command(ipc::solo_focus); // Request to drop all parallel foci.
                    }
                }
                else
                {
                    owner.base::signal(tier::request, input::events::focus::rem, { .gear_id = f.gear_id });
                }
            }
            void handle(s11n::xs::syskeybd         lock)
            {
                auto keybd = lock.thing;
                lock.unlock();
                auto guard = owner.sync();
                auto& gear = *gears;
                gear.keybd::vkevent = owner.indexer.get_kbchord_hint(keybd.vkchord);
                gear.keybd::scevent = owner.indexer.get_kbchord_hint(keybd.scchord);
                gear.keybd::chevent = owner.indexer.get_kbchord_hint(keybd.chchord);
                keybd.syncto(gear);
                owner.base::signal(tier::release, input::events::keybd::post, gear);
            };
            void handle(s11n::xs::mouse_event      lock)
            {
                auto mouse = lock.thing;
                lock.unlock();
                auto guard = owner.sync();
                auto& gear = *gears;
                gear.set_multihome();
                auto basis = gear.owner.base::coor();
                owner.global(basis);
                gear.replay(owner, mouse.cause, mouse.coord - basis, mouse.click - basis, mouse.delta, mouse.buttons, mouse.bttn_id, mouse.dragged, mouse.ctlstat, mouse.whlfp, mouse.whlsi, mouse.hzwhl);
            }
            void handle(s11n::xs::warping          lock)
            {
                auto warp = lock.thing;
                lock.unlock();
                auto guard = owner.sync();
                owner.warp_window(warp.warpdata * owner.cellsz);
            }
            void handle(s11n::xs::logs             lock)
            {
                s11n::recycle_log(lock, os::process::id.second);
            }
            void handle(s11n::xs::sysclose       /*lock*/)
            {
                //todo revise
                owner.window_shutdown();
                //owner.active.exchange(faux);
                //owner.stop(true);
            }
            void handle(s11n::xs::sysstart       /*lock*/)
            {
                //todo revise
                //owner.base::enqueue([&](auto& /*boss*/)
                //{
                //    owner.base::riseup(tier::release, e2::form::global::sysstart, 1);
                //});
            }
            void handle(s11n::xs::cwd            /*lock*/)
            {
                //todo revise
                //owner.base::enqueue([&, path = lock.thing.path](auto& /*boss*/)
                //{
                //    owner.base::riseup(tier::preview, e2::form::prop::cwd, path);
                //});
            }
            void handle(s11n::xs::gui_command      lock)
            {
                auto& gui_cmd = lock.thing;
                if (gui_cmd.cmd_id == syscmd::accesslock)
                {
                    if (gui_cmd.args.size())
                    {
                        gui_cmd.args[0] = 0;
                        s11n::gui_command.send(intio, gui_cmd);
                    }
                }
                else
                {
                    owner.sys_command(gui_cmd.cmd_id, gui_cmd.args);
                }
            }

            link(winbase& owner, ui::pipe& intio)
                : s11n{ *this },
                 input_fields_handler{ owner },
                 owner{ owner },
                 intio{ intio },
                 alive{ true },
                 gears{ owner.bell::create<hids>(owner, s11n::bitmap_dtvt.freeze().thing.image, true) }
            {
                auto& gear = *gears;
                m.gear_id = gear.id;
                w.gear_id = gear.id;
                m.enabled = input::hids::stat::ok;
                m.coordxy = { si16min, si16min };
                c.fast = true;
            }
        };

        cfg_t config; // winbase: User specified settings.
        title titles; // winbase: UI header/footer.
        focus wfocus; // winbase: UI focus.
        layer master; // winbase: Layer for Client.
        layer blinky; // winbase: Layer for blinking characters.
        layer header; // winbase: Layer for Header.
        layer footer; // winbase: Layer for Footer.
        layer tooltip_layer; // winbase: Layer for Tooltip.
        std::array<std::reference_wrapper<layer>, 5> layers = // gcc requires double braces on x32 platforms.
        {{
            master,
            blinky,
            footer,
            header,
            tooltip_layer,
        }};
        fonts fcache; // winbase: Font cache.
        glyph gcache; // winbase: Glyph cache.
        blink blinks; // winbase: Blinking layer state.
        twod& cellsz; // winbase: Cell size in pixels.
        si32  origsz; // winbase: Original cell size in pixels.
        fp32  height; // winbase: Cell height in fp32 pixels.
        twod  gripsz; // winbase: Resizing grips size in pixels.
        twod  gridsz; // winbase: Window grid size in cells.
        dent  border; // winbase: Border around window for resizing grips (dent in pixels).
        shad  shadow; // winbase: Shadow generator.
        grip  szgrip; // winbase: Resizing grips UI-control.
        twod  waitsz; // winbase: Window is waiting resize acknowledge.
        twod  mcoord; // winbase: Mouse cursor coord.
        bool  inside; // winbase: Mouse is inside the client area.
        bool  seized; // winbase: Mouse is locked inside the client area.
        bool  mhover; // winbase: Mouse hover.
        bool  moving; // winbase: Mouse is in dragging window state.
        bool  redraw; // winbase: Window canvas is out of sync during minimization.
        flag  isbusy; // winbase: Window is awaiting update.
        si32  reload; // winbase: Window update bitfield.
        si32  fsmode; // winbase: Window mode.
        rect  normsz; // winbase: Normal mode window area backup.
        twod  fullcs; // winbase: Cell size for fullscreen mode.
        twod  normcs; // winbase: Cell size for normal mode.
        face  h_grid; // winbase: Header layer cell grid.
        face  f_grid; // winbase: Footer layer cell grid.
        face  tooltip_grid; // winbase: Tooltip layer cell grid.
        rect  grip_l; // winbase: Resizing grips left segment area.
        rect  grip_r; // winbase: Resizing grips right segment area.
        rect  grip_t; // winbase: Resizing grips top segment area.
        rect  grip_b; // winbase: Resizing grips bottom segment area.
        b256  vkstat; // winbase: Keyboard virtual keys state.
        si32  keymod; // winbase: Keyboard modifiers state.
        si32  heldby; // winbase: Mouse capture owners bitfield.
        fp32  whlacc; // winbase: Mouse wheel accumulator.
        fp32  wdelta; // winbase: Mouse wheel OS-wise setting.
        foci  mfocus; // winbase: GUI multi-focus control.
        regs  fields; // winbase: Text input field list.
        link  stream; // winbase: DirectVT event proxy.
        kmap  chords; // winbase: Pressed key table (key chord).
        bool  fake_ctrl; // winbase: Fake ctrl key event on AltGr press/release (non-US kb layouts).
        bool  wait_ralt; // winbase: Wait RightAlt right after the fake LeftCtrl.

        winbase(auth& indexer, cfg_t& config, twod grip_cell)
            : base{ indexer },
              config{ config },
              titles{ *this, "", "", faux },
              wfocus{ *this, ui::pro::focus::mode::relay },
              fcache{ config.font_names, config.font_axes, config.cell_height },//, [&]{ netxs::set_flag<task::all>(reload); window_post_command(ipc::no_command); } },
              gcache{ fcache, config.antialiasing },
              blinks{ .init = config.blink_rate },
              cellsz{ fcache.cellsize },
              origsz{ fcache.cellsize.y },
              height{ (fp32)cellsz.y },
              gripsz{ grip_cell * cellsz },
              border{ gripsz.x, gripsz.x, gripsz.y, gripsz.y },
              shadow{ 0.44f/*bias*/, 116.5f/*alfa*/, gripsz.x, dot_00, dot_11, cell::shaders::full },
              inside{ faux },
              seized{ faux },
              mhover{ faux },
              moving{ faux },
              redraw{ faux },
              isbusy{ faux },
              reload{ task::all },
              fsmode{ winstate::undefined },
              fullcs{ cellsz },
              normcs{ cellsz },
              vkstat{},
              keymod{ 0x0 },
              heldby{ 0x0 },
              whlacc{ 0.f },
              wdelta{ 24.f },
              stream{ *this, *os::dtvt::client },
              fake_ctrl{ faux },
              wait_ralt{ faux }
        { }

        virtual bool layer_create(layer& s, winbase* host_ptr = nullptr, twod win_coord = {}, twod grid_size = {}, dent border_dent = {}, twod cell_size = {}) = 0;
        //virtual void layer_delete(layer& s) = 0;
        virtual void layer_move_all() = 0;
        virtual void layer_present(layer& s) = 0;
        virtual bits layer_get_bits(layer& s, bool zeroize = faux) = 0;
        virtual void layer_timer_start(layer& s, span elapse, ui32 eventid) = 0;
        virtual void layer_timer_stop(layer& s, ui32 eventid) = 0;

        virtual void keybd_sync_state(si32 virtcod = 0) = 0;
        virtual void keybd_sync_layout() = 0;
        virtual void keybd_read_vkstat() = 0;
        virtual void keybd_wipe_vkstat() = 0;
        virtual bool keybd_read_input() = 0;
        virtual void keybd_send_block(view block) = 0;
        virtual bool keybd_read_toggled(si32 virtcod) = 0;
        virtual bool keybd_test_toggled(si32 virtcod) = 0;
        virtual bool keybd_read_pressed(si32 virtcod) = 0;
        virtual bool keybd_test_pressed(si32 virtcod) = 0;

        virtual void mouse_capture(si32 captured_by) = 0;
        virtual void mouse_release(si32 released_by) = 0;
        virtual void mouse_catch_outside() = 0;
        virtual twod mouse_get_pos() = 0;

        virtual void window_set_title(view utf8) = 0;
        virtual void window_sync_taskbar(si32 new_state) = 0;
        virtual rect window_get_fs_area(rect window_area) = 0;
        virtual void window_send_command(arch target, si32 command, arch lParam = {}) = 0;
        virtual void window_post_command(arch target, si32 command, arch lParam = {}) = 0;
        virtual cont window_recv_command(arch lParam) = 0;
        virtual void window_message_pump() = 0;
        virtual void window_initilize() = 0;
        virtual void window_shutdown() = 0;
        virtual void window_cleanup() = 0;
        virtual void window_make_foreground() = 0;
        virtual void window_make_focused() = 0;
        virtual void window_make_exposed() = 0;
        virtual void window_make_topmost(bool) = 0;

        virtual void sync_os_settings() = 0;

        void mouse_normalize_wheeldt(fp32& wheelfp)
        {
            wheelfp /= wheel_delta_base / wdelta; // Disable system-wide acceleration.
        }
        void window_post_command(si32 command)
        {
            if (master.hWnd) window_post_command(master.hWnd, command);
        }
        auto ctrl_pressed()
        {
            return mfocus.focused() ? keybd_test_pressed(vkey::control)
                                    : keybd_read_pressed(vkey::control);
        }
        auto lbutton_pressed()
        {
            return keybd_read_pressed(vkey::lbutton);
        }
        void print_vkstat(text s)
        {
            s += "\n    x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xA xB xC xD xE xF"s;
            auto i = 0;
            for (auto k : vkstat)
            {
                if (i % 16 == 0)
                {
                    s += "\n ";
                    utf::to_hex<true>(i, s, 2);
                    s += ' ';
                }
                     if (k == 0x80) s += ansi::fgc(tint::greenlt);
                else if (k == 0x01) s += ansi::fgc(tint::yellowlt);
                else if (k == 0x81) s += ansi::fgc(tint::cyanlt);
                else if (k)         s += ansi::fgc(tint::magentalt);
                else                s += ansi::nil();
                s += utf::to_hex(k) + ' ';
                i++;
            }
            log(s);
        }
        void output(view data)
        {
            stream.intio.send(data);
        }
        void sync_pixel_layout()
        {
            grip_l = rect{{ 0                            , gripsz.y }, { gripsz.x, master.area.size.y - gripsz.y * 2}};
            grip_r = rect{{ master.area.size.x - gripsz.x, gripsz.y }, grip_l.size };
            grip_t = rect{{ 0, 0                                    }, { master.area.size.x, gripsz.y }};
            grip_b = rect{{ 0, master.area.size.y - gripsz.y        }, grip_t.size };
            auto header_height = h_grid.size().y * cellsz.y;
            auto footer_height = f_grid.size().y * cellsz.y;
            header.area = blinky.area + dent{ 0, 0, header_height, -blinky.area.size.y } + shadow_dent;
            header.area.coor.y -= shadow_dent.b;
            footer.area = blinky.area + dent{ 0, 0, -blinky.area.size.y, footer_height } + shadow_dent;
            footer.area.coor.y += shadow_dent.t;
        }
        void reset_blinky()
        {
            if (mfocus.focused() && blinky.live) // Hide blinking layer to avoid visual desync.
            {
                blinky.hide();
                layer_move_all();
                blinky.show();
            }
        }
        void sync_cellsz()
        {
            fullcs = cellsz;
            if (fsmode != winstate::maximized) normcs = cellsz;
        }
        void change_cell_size(bool forced = true, fp32 dy = {}, twod resize_center = {})
        {
            if (std::exchange(height, std::clamp(height + dy, 2.f, 256.f)) == height && !forced) return;
            reset_blinky();
            auto grip_cell = gripsz / cellsz;
            fcache.set_cellsz((si32)height);
            gcache.reset();
            gripsz = grip_cell * cellsz; // cellsz was updated in fcache.
            shadow.generate(0.44f/*bias*/, 116.5f/*alfa*/, gripsz.x, dot_00, dot_11, cell::shaders::full);
            if (fsmode == winstate::maximized)
            {
                auto over_sz = master.area.size % cellsz;
                auto half_sz = over_sz / 2;
                border = { half_sz.x, over_sz.x - half_sz.x, half_sz.y, over_sz.y - half_sz.y };
                size_window();
            }
            else
            {
                border = { gripsz.x, gripsz.x, gripsz.y, gripsz.y };
                auto new_size = gridsz * cellsz + border;
                auto old_size = master.area.size;
                auto xy_delta = resize_center - resize_center * new_size / std::max(dot_11, old_size);
                master.area.coor += xy_delta;
                master.area.size = new_size;
                blinky.area = master.area - border;
                sync_pixel_layout();
            }
            netxs::set_flag<task::all>(reload);
        }
        void set_aa_mode(bool mode)
        {
            log("%%AA mode %state%", prompt::gui, mode ? "enabled" : "disabled");
            gcache.aamode = mode;
            gcache.reset();
            netxs::set_flag<task::all>(reload);
        }
        void update_header()
        {
            page_to_grid(faux, h_grid, titles.head_page, cell::shaders::contrast, { gridsz.x, dot_mx.y });
            sync_pixel_layout();
            netxs::set_flag<task::header>(reload);
        }
        void update_footer()
        {
            page_to_grid(faux, f_grid, titles.foot_page, cell::shaders::contrast, { gridsz.x, dot_mx.y });
            sync_pixel_layout();
            netxs::set_flag<task::footer>(reload);
        }
        void update_tooltip()
        {
            auto& tooltip = stream.gears->tooltip;
            auto  tooltip_clrs = cell{}.bgc(tooltip.default_bgc).fgc(tooltip.default_fgc);
            auto [render_sptr, tooltip_offset] = tooltip.get_render_sptr_and_offset(tooltip_clrs);
            if (render_sptr)
            {
                auto& tooltip_page = *render_sptr;
                auto margins = dent{ dot_11 }; // Shadow around tooltip.
                page_to_grid(true, tooltip_grid, tooltip_page, cell::shaders::fuse, dot_mx, margins);
                tooltip_layer.area.coor = mcoord + (tooltip_offset - margins.corner()) * cellsz;
                tooltip_layer.area.size = tooltip_grid.size() * cellsz;
                tooltip_layer.show();
            }
            else
            {
                tooltip_layer.hide();
            }
            netxs::set_flag<task::tooltip>(reload);
        }
        void set_font_list(auto& flist)
        {
            log("%%Font list changed: ", prompt::gui, flist);
            fcache.set_fonts(flist, faux);
            change_cell_size(true);
        }
        auto move_window(twod delta)
        {
            for (auto& l : layers)
            {
                auto& p = l.get();
                p.area.coor += delta;
            }
            netxs::set_flag<task::moved>(reload);
        }
        void drop_grips()
        {
            if (szgrip.seized) // drag stop
            {
                szgrip.drop();
                netxs::set_flag<task::grips>(reload);
            }
        }
        void set_state(si32 new_state)
        {
            if (fsmode == new_state && fsmode != winstate::normal) return; // Restore to normal if it was silently hidden by the system.
            log("%%Set window to ", prompt::gui, new_state == winstate::maximized ? "maximized" : new_state == winstate::normal ? "normal" : "minimized", " state");
            auto old_state = std::exchange(fsmode, winstate::undefined);
            if (new_state != winstate::minimized) reset_blinky(); // To avoid visual desync.
            window_sync_taskbar(new_state);
            fsmode = new_state;
            if (old_state == winstate::normal) normsz = master.area;
            if (fsmode == winstate::normal)
            {
                for (auto p : { &master, &header, &footer }) p->show();
                if (blinks.poll) blinky.show();
                master.area = normsz;
                if (auto celldt = (fp32)(normcs.y - cellsz.y))
                {
                    auto grip_cell = gripsz / cellsz;
                    auto prev_gripsz = grip_cell * normcs;
                    gridsz = (normsz.size - dent{ prev_gripsz.x, prev_gripsz.x, prev_gripsz.y, prev_gripsz.y }) / normcs; // Restore normal mode gridsz.
                    change_cell_size(faux, celldt);
                }
                else border = { gripsz.x, gripsz.x, gripsz.y, gripsz.y };
                size_window();
            }
            else if (fsmode == winstate::minimized)
            {
                for (auto& l : layers)
                {
                    auto& p = l.get();
                    p.hide();
                }
            }
            else if (fsmode == winstate::maximized)
            {
                drop_grips();
                master.area = window_get_fs_area(master.area - border);
                header.hide();
                footer.hide();
                master.show();
                if (blinks.poll) blinky.show();
                if (auto celldt = (fp32)(fullcs.y - cellsz.y))
                {
                    change_cell_size(faux, celldt);
                }
                else
                {
                    auto over_sz = master.area.size % cellsz;
                    auto half_sz = over_sz / 2;
                    border = { half_sz.x, over_sz.x - half_sz.x, half_sz.y, over_sz.y - half_sz.y };
                    size_window();
                }
            }
            if (old_state != fsmode)
            {
                stream.fsmod(fsmode == winstate::maximized);
                if (redraw && old_state == winstate::minimized) // Redraw all to restore after minimization.
                {
                    redraw = faux;
                    netxs::set_flag<task::all>(reload);
                }
            }
        }
        si32 prev_window_state = winstate::normal;
        bool ontop_state = faux;
        void restore_if_minimized() // Restore focused window.
        {
            if (fsmode == winstate::minimized) // We are restoring from an implicit minimized state.
            {
                auto restored_state = std::exchange(prev_window_state, winstate::normal);
                set_state(restored_state);
                window_make_topmost(ontop_state); // Restore AlwaysOnTop state.
            }
        }
        void check_window(twod coor)
        {
            if (coor != layer::hidden) restore_if_minimized(); // Restore unfocused window.
            if (fsmode != winstate::normal) return;
            if (coor == layer::hidden) // We are going to an implicit hidden state caused by Win+D or so.
            {
                log("%%Set window to minimized state (implicit)", prompt::gui);
                prev_window_state = std::exchange(fsmode, winstate::minimized);
                if (prev_window_state == winstate::normal) normsz = master.area;
                for (auto& l : layers)
                {
                    auto& p = l.get();
                    p.hide();
                }
            }
            else if (auto delta = coor - master.area.coor)
            {
                base::enqueue([&, delta](auto& /*boss*/) // Perform corrections.
                {
                    move_window(delta);
                });
            }
        }
        void fit_to_displays(rect& layer_area, dent contour = {})
        {
            auto fs_area = window_get_fs_area(master.area) + contour;
            fs_area.size = std::max(dot_00, fs_area.size - layer_area.size);
            layer_area.coor = fs_area.clamp(layer_area.coor);
        }
        void check_fsmode()
        {
            if (fsmode == winstate::undefined) return;
            auto unsync = true;
            if (auto lock = bell::try_sync()) // Try to sync with ui thread for fast checking.
            {
                if (fsmode == winstate::maximized)
                {
                    auto fs_area = window_get_fs_area(master.area);
                    unsync = fs_area != master.area;
                }
                else if (fsmode == winstate::normal)
                {
                    auto avail_area = window_get_fs_area(rect{ -dot_mx / 2, dot_mx });
                    unsync = !avail_area.trim(master.area);
                }
                else unsync = faux;
            }
            if (unsync) base::enqueue([&](auto& /*boss*/) // Perform corrections.
            {
                if (fsmode == winstate::maximized)
                {
                    auto fs_area = window_get_fs_area(master.area);
                    if (fs_area != master.area)
                    {
                        auto avail_area = window_get_fs_area(rect{ -dot_mx / 2, dot_mx });
                        avail_area.size -= std::min(avail_area.size, normsz.size);
                        normsz.coor = avail_area.clamp(normsz.coor);
                        set_state(winstate::normal);
                    }
                }
                else if (fsmode == winstate::normal)
                {
                    auto avail_area = window_get_fs_area(rect{ -dot_mx / 2, dot_mx });
                    if (!avail_area.trim(master.area))
                    {
                        auto area = master.area;
                        avail_area.size -= std::min(avail_area.size, area.size);
                        auto delta = avail_area.clamp(area.coor) - area.coor;
                        move_window(delta);
                        sync_pixel_layout(); // Align grips and shadow.
                    }
                }
                if (fsmode != winstate::minimized)
                {
                    for (auto& l : layers)
                    {
                        auto& p = l.get();
                        p.prev.coor = dot_mx; // Windows moves our windows the way it wants, breaking the layout.
                    }
                    netxs::set_flag<task::moved>(reload);
                }
                update_gui();
            });
        }
        void page_to_grid(bool update_all, ui::face& target_grid, ui::page& source_page, auto fuse, twod grid_size, dent margins = {})
        {
            target_grid.get_page_size(source_page, grid_size, update_all);
            target_grid.size(grid_size + margins);
            target_grid.wipe();
            target_grid.mgn(margins);
            target_grid.cup(dot_00);
            target_grid.output(source_page, fuse);
        }
        void size_window(twod size_delta = {})
        {
            //todo revise
            master.area.size += size_delta;
            blinky.area = master.area - border;
            gridsz = std::max(dot_11, blinky.area.size / cellsz);
            auto sizechanged = stream.w.winsize != gridsz;
            blinky.area.size = gridsz * cellsz;
            master.area = blinky.area + border;
            if (fsmode != winstate::maximized)
            {
                page_to_grid(faux, h_grid, titles.head_page, cell::shaders::contrast, { gridsz.x, dot_mx.y });
                page_to_grid(faux, f_grid, titles.foot_page, cell::shaders::contrast, { gridsz.x, dot_mx.y });
                sync_pixel_layout();
            }
            if (sizechanged)
            {
                netxs::set_flag<task::all>(reload);
                waitsz = gridsz;
                stream.w.winsize = gridsz;
                stream.winsz(stream.w); // And wait for reply to resize and redraw.
            }
            else netxs::set_flag<task::sized>(reload);
        }
        auto resize_window(twod size_delta)
        {
            auto old_client = blinky.area;
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
            auto old_client = blinky.area;
            auto new_client = old_client + warp_delta;
            auto new_gridsz = std::max(dot_11, new_client.size / cellsz);
            auto size_delta = new_gridsz * cellsz - old_client.size;
            auto coor_delta = new_client.coor - old_client.coor;
            if (size_delta || coor_delta)
            {
                size_window(size_delta);
                move_window(coor_delta);
            }
            return master.area - old_client;
        }
        bool hit_grips()
        {
            if (fsmode == winstate::maximized || szgrip.zoomon) return faux;
            auto inner_rect = blinky.area;
            auto outer_rect = master.area;
            auto hit = szgrip.seized || (mhover && outer_rect.hittest(mcoord) && !inner_rect.hittest(mcoord));
            return hit;
        }
        void draw_grips()
        {
            if (fsmode == winstate::maximized) return;
            static auto trans = argb::active_transparent;
            static auto shade = 0x5F'3f'3f'3f;
            static auto black = 0x3F'00'00'00;
            auto canvas = layer_get_bits(master);
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
                auto shade2 = shade + 0x00'09'09'09 * std::popcount((ui32)stream.m.buttons);
                fill_grips(side_x, [&, &side_x = side_x](auto& canvas, auto r) //todo &side_x: Apple clang still disallows capturing structured bindings
                {
                    netxs::onrect(canvas, r, cell::shaders::full(shade2));
                    netxs::misc::cage(canvas, side_x, dent_x, cell::shaders::full(black)); // 1-px dark contour around.
                });
                fill_grips(side_y, [&, &side_y = side_y](auto& canvas, auto r) //todo &side_y: Apple clang still disallows capturing structured bindings
                {
                    netxs::onrect(canvas, r, cell::shaders::full(shade2));
                    netxs::misc::cage(canvas, side_y, dent_y, cell::shaders::full(black)); // 1-px dark contour around.
                });
            }
            if (!shadow.hide) fill_grips(outer_rect, [&](auto& canvas, auto r)
            {
                shadow.render(canvas, r, inner_rect, cell::shaders::alpha);
            });
            if (reload != task::all)
            {
                auto coor = master.area.coor;
                for (auto g_area : { grip_l, grip_r, grip_t, grip_b })
                {
                    //todo push diffs only
                    g_area.coor += coor;
                    master.sync.push_back(g_area);
                }
            }
        }
        template<class T = noop>
        void draw_cell_with_cursor(auto& canvas, rect placeholder, cell c, T&& blink_canvas = {})
        {
            //todo hilight grapheme cluster.
            auto style = c.cur();
            auto [bgcolor, fgcolor] = c.cursor_color();
            auto width = std::max(1, (si32)std::round(cellsz.x / 8.f));
            if (!bgcolor || bgcolor == argb::default_color) bgcolor = { c.bgc().luma() < 192 ? tint::purewhite : tint::pureblack };
            if (!fgcolor || fgcolor == argb::default_color) fgcolor = { bgcolor.luma() < 192 ? tint::purewhite : tint::pureblack };
            if (style == text_cursor::block)
            {
                c.fgc(fgcolor).bgc(bgcolor);
                gcache.draw_cell(canvas, placeholder, c, blink_canvas);
            }
            else if (style == text_cursor::I_bar)
            {
                gcache.draw_cell(canvas, placeholder, c, blink_canvas);
                placeholder.size.x = width;
                //todo draw glyph inside the cursor (respect blinking glyphs)
                //c.fgc(fgcolor).bgc(bgcolor);
                //gcache.draw_cell(canvas, placeholder, c, blink_canvas);
                netxs::onrect(canvas, placeholder, cell::shaders::full(bgcolor));
            }
            else if (style == text_cursor::underline)
            {
                gcache.draw_cell(canvas, placeholder, c, blink_canvas);
                placeholder.coor.y += cellsz.y - width;
                placeholder.size.y = width;
                c.fgc(fgcolor).bgc(bgcolor);
                //todo draw glyph inside the cursor (respect blinking glyphs)
                //gcache.draw_cell(canvas, placeholder, c, blink_canvas);
                netxs::onrect(canvas, placeholder, cell::shaders::full(bgcolor));
            }
        }
        void fill_grid(auto& canvas, auto& cellgrid, twod origin = {})
        {
            origin += canvas.coor();
            auto p = rect{ origin, cellsz };
            auto m = origin + cellgrid.size() * cellsz;
            for (auto& c : cellgrid)
            {
                if (c.cur()) draw_cell_with_cursor(canvas, p, c);
                else         gcache.draw_cell(canvas, p, c);
                p.coor.x += cellsz.x;
                if (p.coor.x >= m.x)
                {
                    p.coor.x = origin.x;
                    p.coor.y += cellsz.y;
                    if (p.coor.y >= m.y) break;
                }
            }
        }
        void fill_stripe(auto head, auto tail, twod start = {}, si32 offset = {})
        {
            auto prime_canvas = layer_get_bits(master);
            auto blink_canvas = layer_get_bits(blinky);
            auto origin = blink_canvas.coor();
            //todo blinks.mask size is out of sync on intensive resize
            auto iter = blinks.mask.begin() + offset;
            auto p = rect{ origin + start, cellsz };
            auto m = origin + blink_canvas.size();
            while (head != tail)
            {
                auto& c = *head++;
                auto& b = *iter++;
                if (std::exchange(b, c.blk()) != b)
                {
                    if (b) blinks.poll++;
                    else
                    {
                        blinks.poll--;
                        netxs::onrect(blink_canvas, p, cell::shaders::wipe);
                        blinky.strike(p);
                    }
                }
                if (b) blinky.strike(p);
                if (c.cur()) draw_cell_with_cursor(prime_canvas, p, c, blink_canvas);
                else         gcache.draw_cell(prime_canvas, p, c, blink_canvas);
                p.coor.x += cellsz.x;
                if (p.coor.x >= m.x)
                {
                    p.coor.x = origin.x;
                    p.coor.y += cellsz.y;
                    if (p.coor.y >= m.y) break;
                }
            }
        }
        void draw_grid(layer& s, auto& facedata, bool apply_contour = true) //todo just output ui::core
        {
            auto canvas = layer_get_bits(s, true);
            if (apply_contour)
            {
                fill_grid(canvas, facedata, shadow_dent.corner());
                netxs::misc::contour(canvas); // 1ms
            }
            else
            {
                auto area = facedata.area() - dent{ dot_11 };
                ui::pro::ghost::draw_shadow(area, facedata);
                fill_grid(canvas, facedata, dot_00);
            }
            s.strike<true>(canvas.area());
        }
        void draw_header()  { draw_grid(header, h_grid); }
        void draw_footer()  { draw_grid(footer, f_grid); }
        void draw_tooltip() { draw_grid(tooltip_layer, tooltip_grid, faux); }
        void check_blinky()
        {
            auto changed = std::exchange(blinks.show, !!blinks.poll) != blinks.show;
            if (changed)
            {
                if (blinks.poll)
                {
                    if (blinks.rate != span::zero()) layer_timer_start(master, blinks.rate, timers::blink);
                    else                             blinky.show();
                }
                else
                {
                    if (mfocus.focused() && blinky.live) blinky.hide();
                    layer_timer_stop(master, timers::blink);
                }
            }
        }
        void update_gui()
        {
            if (!reload || waitsz) return;
            auto what = reload;
            reload = {};
                 if (what == task::moved) layer_move_all();
            else if (what)
            {
                if (what == task::all)
                {
                    if (blinks.poll)
                    {
                        blinks.poll = 0;
                        blinks.mask.assign(gridsz.x * gridsz.y, 0);
                        if(!blinky.resized()) // Manually zeroize blinking canvas if its size has not changed.
                        {
                            blinky.wipe();
                        }
                        blinky.strike<true>(blinky.area);
                    }
                    else // Keep blink mask size in sync.
                    {
                        blinks.mask.resize(gridsz.x * gridsz.y);
                    }
                    auto bitmap_lock = stream.bitmap_dtvt.freeze();
                    auto& grid = bitmap_lock.thing.image;
                    fill_stripe(grid.begin(), grid.end());
                    if (fsmode == winstate::maximized)
                    {
                        auto canvas = layer_get_bits(master);
                        netxs::misc::cage(canvas, canvas.area(), border, cell::shaders::full(argb{ tint::pureblack }));
                    }
                    master.strike<true>(master.area);
                    check_blinky();
                }
                if (fsmode == winstate::normal)
                {
                    if (what & (task::sized | task::hover | task::grips)) draw_grips(); // 0.150 ms
                    if (what & (task::sized | task::header)) draw_header();
                    if (what & (task::sized | task::footer)) draw_footer();
                }
                if (what & task::tooltip && tooltip_layer.live)
                {
                    auto contour = twod{ 0, cellsz.y / 2 };
                    fit_to_displays(tooltip_layer.area, dent{ contour });
                    draw_tooltip();
                }
                for (auto& l : layers)
                {
                    layer_present(l);
                }
            }
            isbusy.exchange(faux);
        }
        auto get_mods_state()
        {
            if (mfocus.focused()) return keymod;
            else
            {
                auto state = 0;
                if (keybd_read_pressed(vkey::lshift  )) state |= input::hids::LShift;
                if (keybd_read_pressed(vkey::rshift  )) state |= input::hids::RShift;
                if (keybd_read_pressed(vkey::lcontrol)) state |= input::hids::LCtrl;
                if (keybd_read_pressed(vkey::rcontrol)) state |= input::hids::RCtrl;
                if (keybd_read_pressed(vkey::lalt    )) state |= input::hids::LAlt;
                if (keybd_read_pressed(vkey::ralt    )) state |= input::hids::RAlt;
                if (keybd_read_pressed(vkey::lwin    )) state |= input::hids::LWin;
                if (keybd_read_pressed(vkey::rwin    )) state |= input::hids::RWin;
                if (keybd_read_toggled(vkey::capslock)) state |= input::hids::CapsLock;
                if (keybd_read_toggled(vkey::scrllock)) state |= input::hids::ScrlLock;
                if (keybd_read_toggled(vkey::numlock )) state |= input::hids::NumLock;
                return state;
            }
        }
        void zoom_by_wheel(fp32 wheelfp, bool enqueue)
        {
            if (ctrl_pressed())
            {
                mouse_normalize_wheeldt(wheelfp); // Disable system-wide acceleration.
                if (whlacc * wheelfp < 0.f) whlacc = 0.f; // Reset accumulator if the wheeling direction has changed.
                whlacc += wheelfp;
                if (!isbusy.exchange(true))
                {
                    wheelfp = std::exchange(whlacc, 0.f);
                    auto zoom = [&, wheelfp, center = mcoord - master.area.coor]
                    {
                        change_cell_size(faux, wheelfp, center);
                        sync_cellsz();
                        update_gui();
                    };
                    if (enqueue) base::enqueue([zoom](auto& /*boss*/){ zoom(); });
                    else         zoom();
                }
            }
        }
        void resize_by_grips(twod coord)
        {
            auto inner_rect = blinky.area;
            auto zoom = ctrl_pressed();
            auto [preview_area, size_delta] = szgrip.drag(inner_rect, coord, border, zoom, cellsz);
            auto old_client = blinky.area;
            auto new_gridsz = std::max(dot_11, (old_client.size + size_delta) / cellsz);
            size_delta = new_gridsz * cellsz - old_client.size;
            if (size_delta)
            {
                //todo sync ui
                if (auto move_delta = szgrip.move(size_delta, zoom))
                {
                    move_window(move_delta);
                    sync_pixel_layout(); // Align grips and shadow.
                }
                resize_window(size_delta);
            }
        }
        void mouse_wheel(si32 delta, bool hz)
        {
            if (delta == 0) return;
            if (hz) delta = -delta;
            auto wheelfp = delta / wdelta; // Same code in system.hpp.
            if (whlacc * wheelfp < 0) whlacc = {}; // Reset accum if direction has changed.
            whlacc += wheelfp;
            auto wheelsi = (si32)whlacc;
            if (wheelsi) whlacc -= (fp32)wheelsi;

            if (inside)
            {
                stream.m.changed++;
                stream.m.timecod = datetime::now();
                stream.m.ctlstat = get_mods_state();
                stream.m.hzwheel = hz;
                stream.m.wheelfp = wheelfp;
                stream.m.wheelsi = wheelsi;
                stream.m.enabled = hids::stat::ok;
                stream.mouse(stream.m);
                stream.m.hzwheel = {};
                stream.m.wheelfp = {};
                stream.m.wheelsi = {};
            }
            else zoom_by_wheel(wheelfp, true);
        }
        void mouse_send_halt()
        {
            stream.m.changed++;
            stream.m.timecod = datetime::now();
            stream.m.enabled = hids::stat::halt;
            if (!mfocus.focused()) stream.m.ctlstat &= input::hids::NumLock | input::hids::CapsLock | input::hids::ScrlLock;
            if (std::exchange(stream.gears->tooltip.visible, faux)) // Hide all active tooltips on mouse leave.
            {
                update_tooltip();
            }
            stream.mouse(stream.m);
        }
        void mouse_leave()
        {
            mhover = faux;
            if (szgrip.leave()) netxs::set_flag<task::grips>(reload);
            mouse_send_halt();
        }
        void mouse_moved()
        {
            auto coord = mouse_get_pos();
            auto& mbttns = stream.m.buttons;
            mhover = true;
            auto inner_rect = blinky.area;
            auto ingrip = hit_grips();
            if (moving && !mbttns) // Don't allow to move GUI window without mouse button pressed (race condition, left mouse button sticks randomly when dragging GUI window).
            {
                moving = faux;
            }
            if (auto target_list = mfocus.is_idle()) // Seize OS focus if group focus is active but window is idle.
            {
                auto local_target = master.hWnd;
                for (auto target : target_list.value()) window_send_command(target, ipc::main_focus, local_target);
                window_make_foreground();
            }
            if ((!seized && ingrip) || szgrip.seized)
            {
                if (mbttns == bttn::right && fsmode == winstate::normal) // Move window.
                {
                    moving = true;
                }
                else if (mbttns == bttn::left)
                {
                    if (!szgrip.seized) // drag start
                    {
                        szgrip.grab(inner_rect, mcoord, border, cellsz);
                    }
                    base::enqueue([&, coord](auto& /*boss*/)
                    {
                        resize_by_grips(coord);
                    });
                }
                else drop_grips();
            }
            if (!moving && !seized && szgrip.calc(inner_rect, coord, border, dent{}, cellsz))
            {
                netxs::set_flag<task::grips>(reload);
            }
            if (moving && fsmode == winstate::normal)
            {
                if (auto dxdy = coord - mcoord)
                {
                    mcoord = coord;
                    base::enqueue([&, dxdy](auto& /*boss*/)
                    {
                        //todo revise
                        //if (fsmode == winstate::maximized) set_state(winstate::normal);
                        move_window(dxdy);
                        sync_pixel_layout(); // Align grips and shadow.
                        update_gui();
                    });
                }
                return;
            }
            mcoord = coord;
            auto new_state = !szgrip.seized && (seized || (border.t ? inner_rect : inner_rect - dent{ 0,0,1,0 }).hittest(mcoord)); // Allow 1px border at the top of the maximized window.
            auto leave = std::exchange(inside, new_state) != inside;
            auto coordxy = fp2d{ mcoord - inner_rect.coor } / cellsz;
            auto changed = stream.m.coordxy(coordxy);
            if (inside)
            {
                if (changed)
                {
                    stream.m.changed++;
                    stream.m.timecod = datetime::now();
                    stream.m.ctlstat = get_mods_state();
                    stream.m.enabled = hids::stat::ok;
                    stream.mouse(stream.m);
                }
            }
            else if (leave) // Mouse leaves viewport.
            {
                mouse_send_halt();
            }
            if (!mbttns && (std::exchange(ingrip, hit_grips()) != ingrip || ingrip)) // Redraw grips when hover state changes.
            {
                netxs::set_flag<task::grips>(reload);
            }
        }
        void mouse_press(si32 button, bool pressed)
        {
            if constexpr (debug_foci) log("--- mouse ", pressed?"1":"0");
            auto inner_rect = blinky.area;
            auto coord = mouse_get_pos();
            mcoord = coord;
            stream.m.coordxy = fp2d{ mcoord - inner_rect.coor } / cellsz;
            auto& mbttns = stream.m.buttons;
            if (std::exchange(mfocus.offer, faux)) // Ignore any first Ctrl+AnyClick inside the just focused window.
            {
                if constexpr (debug_foci) log("group focus pressed");
                return;
            }
            if (!mbttns && !master.area.hittest(coord)) // Drop AnyClick outside the yet focused window. To avoid clicking on an invisible desktop object.
            {
                if constexpr (debug_foci) log(ansi::clr(yellowlt, "drop click"));
                return;
            }
            auto prev_state = std::exchange(mbttns, pressed ? mbttns | button : mbttns & ~button);
            auto changed = prev_state != mbttns;
            if (pressed)
            {
                if (!prev_state)
                {
                    mouse_capture(by::mouse);
                    if (inside) seized = true;
                }
            }
            else if (!mbttns)
            {
                mouse_release(by::mouse);
                seized = faux;
            }
            if (pressed)
            {
                //if (moving && prev_state) // Stop GUI window dragging if any addition mouse button pressed.
                //{
                //    moving = faux;
                //}
            }
            else
            {
                if (button == bttn::left) drop_grips();
                //moving = faux; // Stop GUI window dragging if any button released.
            }
            if (changed && hit_grips()) // Amplify grips on any mouse press.
            {
                netxs::set_flag<task::grips>(reload);
            }
            if (moving) // Don't report mouse clicks while dragging window.
            {
                if (!mbttns) // Update mouse cursor position after stop dragging window.
                {
                    moving = faux;
                    stream.m.changed++;
                    stream.m.timecod = datetime::now();
                    stream.m.ctlstat = get_mods_state();
                    stream.m.enabled = hids::stat::ok;
                    stream.mouse(stream.m);
                }
                return;
            }

            static auto dblclick = datetime::now() - 1s;
            if (changed && (seized || inside))
            {
                stream.m.changed++;
                stream.m.timecod = datetime::now();
                stream.m.ctlstat = get_mods_state();
                stream.m.enabled = hids::stat::ok;
                stream.mouse(stream.m);
            }
            else
            {
                if (!pressed && button == bttn::left) // Maximize window by dbl click on resizing grips.
                {
                    if (datetime::now() - dblclick < 500ms)
                    {
                        if (fsmode != winstate::minimized) set_state(fsmode == winstate::maximized ? winstate::normal : winstate::maximized);
                        dblclick -= 1s;
                    }
                    else
                    {
                        dblclick = datetime::now();
                    }
                }
            }
        }
        void stream_keybd(hids& gear)
        {
            gear.handled = {};
            gear.touched = {};
            gear.timecod = datetime::now();
            stream.keybd(gear, [&](view block)
            {
                if (mfocus.active())
                {
                    keybd_send_block(block); // Send multifocus events.
                }
            });
        }
        void keybd_send_state(si32 virtcod = {}, si32 keystat = {}, si32 scancod = {}, bool extflag = {}, view cluster = {}, bool synth = faux)
        {
            auto state = 0;
            auto cs = 0;
            if (extflag) cs |= input::key::ExtendedKey;
            if (synth)
            {
                state = keymod;
            }
            else
            {
                if (fake_ctrl && wait_ralt) // RAlt is expected right after the fake LCtrl when AltGr is pressed.
                {
                    wait_ralt = faux;
                    auto is_ralt = scancod == input::key::map::data(input::key::RightAlt).scan/*0x38*/ && extflag; // RAlt.
                    if (!is_ralt) // If something else comes instead of RAlt, it means that the LCtrl key was actually pressed.
                    {
                        fake_ctrl = faux;
                        keybd_send_state(vkey::control, input::key::pressed, input::key::map::data(input::key::LeftCtrl).scan/*0x1d*/); // Send LCtrl actually pressed.
                    }
                }
                if (fake_ctrl) state |= input::hids::AltGr; // Keep AltGr flag even if RightAlt released.
                if (fake_ctrl && keystat == input::key::released && scancod == input::key::map::data(input::key::RightAlt).scan) // Clear the AltGr state.
                {
                    fake_ctrl = faux;
                }
                if (keybd_test_toggled(vkey::numlock )) { state |= input::hids::NumLock; cs |= input::key::NumLockMode; }
                if (keybd_test_toggled(vkey::capslock)) state |= input::hids::CapsLock;
                if (keybd_test_toggled(vkey::scrllock)) state |= input::hids::ScrlLock;
                if (keybd_test_pressed(vkey::lcontrol) && !fake_ctrl) state |= input::hids::LCtrl;
                if (keybd_test_pressed(vkey::rcontrol)) state |= input::hids::RCtrl;
                if (keybd_test_pressed(vkey::lalt    )) state |= input::hids::LAlt;
                if (keybd_test_pressed(vkey::ralt    )) state |= input::hids::RAlt;
                if (keybd_test_pressed(vkey::lwin    )) state |= input::hids::LWin;
                if (keybd_test_pressed(vkey::rwin    )) state |= input::hids::RWin;
                if (keybd_test_pressed(vkey::control )) mouse_capture(by::keybd); // Capture mouse if Ctrl modifier is pressed (to catch Ctrl+AnyClick outside the window).
                else                                    mouse_release(by::keybd);
                auto old_ls = keymod & input::hids::LShift;
                auto old_rs = keymod & input::hids::RShift;
                auto new_ls = keybd_test_pressed(vkey::lshift);
                auto new_rs = keybd_test_pressed(vkey::rshift);
                //log("old_ls=%% old_rs=%%  new_ls=%% new_rs=%% keymod=%%", (si32)old_ls, (si32)old_rs, (si32)new_ls, (si32)new_rs, utf::to_hex(keymod));
                state |= old_ls | old_rs;
                if (new_ls != !!old_ls || new_rs != !!old_rs) // MS Windows Shift+Shift bug workaround.
                {
                    keymod = state;
                    //todo unify
                    if (!new_ls && !new_rs && old_ls && old_rs && chords.pushed[input::key::LeftShift].stamp < chords.pushed[input::key::RightShift].stamp) // Respect release order.
                    {
                        //if (old_rs && !new_rs) // RightShift released.
                        {
                            layer_timer_stop(master, timers::rightshift); // Stop catching RightShift release.
                            keymod &= ~input::hids::RShift;
                            keybd_send_state(vkey::shift, input::key::released, input::key::map::data(input::key::RightShift).scan, {}, {}, true);
                        }
                        //if (old_ls && !new_ls) // LeftShift released.
                        {
                            keymod &= ~input::hids::LShift;
                            keybd_send_state(vkey::shift, input::key::released, input::key::map::data(input::key::LeftShift).scan, {}, {}, true);
                        }
                    }
                    else
                    {
                        if (old_ls && !new_ls) // LeftShift released.
                        {
                            keymod &= ~input::hids::LShift;
                            keybd_send_state(vkey::shift, input::key::released, input::key::map::data(input::key::LeftShift).scan, {}, {}, true);
                        }
                        if (old_rs && !new_rs) // RightShift released.
                        {
                            layer_timer_stop(master, timers::rightshift); // Stop catching RightShift release.
                            keymod &= ~input::hids::RShift;
                            keybd_send_state(vkey::shift, input::key::released, input::key::map::data(input::key::RightShift).scan, {}, {}, true);
                        }
                    }
                    if (!old_ls && new_ls) // LeftShift pressed.
                    {
                        keymod |= input::hids::LShift;
                        keybd_send_state(vkey::shift, input::key::pressed, input::key::map::data(input::key::LeftShift).scan, {}, {}, true);
                    }
                    if (!old_rs && new_rs) // RightShift pressed.
                    {
                        keymod |= input::hids::RShift;
                        keybd_send_state(vkey::shift, input::key::pressed, input::key::map::data(input::key::RightShift).scan, {}, {}, true);
                    }
                    if (new_ls && new_rs) // Two Shifts pressed.
                    {
                        layer_timer_start(master, 33ms, timers::rightshift); // Try to catch RightShift release.
                    }
                    //log(" keymod=%%", utf::to_hex(keymod));
                    if (virtcod == vkey::shift) return;
                    state = keymod;
                }
                else if (scancod == input::key::map::data(input::key::LeftCtrl).scan/*0x1d*/ && !extflag) // Filter fake LeftCtrl messages when AltGr pressed/repeated/released (non-US kb layouts).
                {
                    if (keystat == input::key::pressed)
                    {
                        //if constexpr (debugmode) log("Fake LeftCtrl pressed");
                        fake_ctrl = !(state & input::hids::RAlt) && keybd_read_pressed(vkey::ralt); // Actually AltGr is pressed.
                    }
                    else if (keystat == input::key::released)
                    {
                        //if constexpr (debugmode) log("Fake LeftCtrl released");
                        fake_ctrl = (state & input::hids::RAlt) && !keybd_read_pressed(vkey::ralt); // Actually AltGr is released.
                    }
                    else // Actually AltGr is repeated if fake_ctrl==true.
                    {
                        //if constexpr (debugmode) log("Fake LeftCtrl repeated");
                    }
                    if (fake_ctrl) // Filter input::key::repeated events as well.
                    {
                        if constexpr (debugmode) log("Fake left ctrl key '%%' event filtered", keystat == input::key::pressed ? "pressed" : keystat == input::key::released ? "released" : "repeated");
                        wait_ralt = keystat != input::key::released; // Zeroize flag on release.
                        return;
                    }
                }
            }
            auto changed = std::exchange(keymod, state) != keymod || synth;
            auto& gear = *stream.gears;
            if ((changed || gear.ctlstat != keymod))
            {
                gear.ctlstat = keymod;
                if (stream.m.enabled == hids::stat::ok)
                {
                    stream.m.ctlstat = keymod;
                    stream.m.timecod = datetime::now();
                    stream.m.changed++;
                    stream.mouse(stream.m); // Fire mouse event to update kb modifiers.
                }
            }
            gear.payload = input::keybd::type::keypress;
            gear.extflag = extflag;
            gear.virtcod = virtcod;
            gear.scancod = scancod;
            auto keycode = input::key::xlat(virtcod, scancod, cs);
            if ((gear.keystat == input::key::released || keycode != gear.keycode) && keystat == input::key::repeated) keystat = input::key::pressed; // LeftMod+RightMod press is treated by the OS as a repeated LeftMod.
            gear.keystat = keystat;
            gear.keycode = keycode;
            gear.cluster = cluster;
            auto repeat_ctrl = keystat == input::key::repeated && (virtcod == vkey::shift    || virtcod == vkey::control || virtcod == vkey::alt
                                                                || virtcod == vkey::capslock || virtcod == vkey::numlock || virtcod == vkey::scrllock
                                                                || virtcod == vkey::lwin     || virtcod == vkey::rwin);
            //print_vkstat("keybd_send_state");
            if (changed || (!repeat_ctrl && (scancod != 0 || !cluster.empty()))) // We don't send repeated modifiers.
            {
                synth ? chords.build(gear)
                      : chords.build(gear, [&](auto index){ return !keybd_test_pressed(index); });
                stream_keybd(gear);
            }
        }
        void keybd_send_input(view utf8, byte payload_type)
        {
            auto& gear = *stream.gears;
            gear.payload = payload_type;
            gear.cluster = utf8;
            chords.reset(gear);
            stream_keybd(gear);
        }

        void ResetWheelAccumulator()
        {
            whlacc = {};
        }
        void IncreaseCellHeight(many const& args)
        {
            auto dir = args.size() ? netxs::any_get_or(args.front(), 0.f) : 0.f;
            change_cell_size(faux, dir);
            sync_cellsz();
            update_gui();
        }
        void ResetCellHeight()
        {
            auto dy = origsz - cellsz.y;
            change_cell_size(faux, (fp32)dy, master.area.size / 2);
            sync_cellsz();
            update_gui();
        }
        void ToggleFullscreenMode()
        {
            if (fsmode != winstate::minimized)
            {
                set_state(fsmode == winstate::maximized ? winstate::normal : winstate::maximized);
            }
        }
        void ToggleAntialiasingMode()
        {
            set_aa_mode(!gcache.aamode);
        }
        void RollFontList(many const& args)
        {
            if (fcache.families.empty()) return;
            auto dir = args.size() ? netxs::any_get_or<si32>(args.front()) : 0;
            auto& families = fcache.families;
            if (dir >= 0)
            {
                families.push_back(std::move(families.front()));
                families.pop_front();
            }
            else
            {
                families.push_front(std::move(families.back()));
                families.pop_back();
            }
            set_font_list(families);
        }
        void MoveWindow(many const& args)
        {
            if (args.size() != 2) return;
            if (auto delta = twod{ netxs::any_get_or(args[0]), netxs::any_get_or(args[1]) })
            {
                move_window(delta);
            }
        }
        void WarpWindow(many const& args)
        {
            if (args.size() != 4) return;
            auto warp = dent{ netxs::any_get_or(args[0]),
                              netxs::any_get_or(args[1]),
                              netxs::any_get_or(args[2]),
                              netxs::any_get_or(args[3]) };
            warp_window(warp * cellsz);
        }
        void FocusNextWindow(many const& args)
        {
            auto dir = args.size() ? netxs::any_get_or<si32>(args.front()) : 0;
            if (dir >= 0)
            {
                //todo implement
            }
            else
            {
                //todo implement
            }
        }
        void ZOrder(many const& args)
        {
            auto state = args.size() ? netxs::any_get_or(args.front(), zpos::plain) : zpos::plain;
            window_send_command(master.hWnd, state ? ipc::make_ontop : ipc::set_normal);
        }

        arch run_command(arch command, arch lParam)
        {
            if constexpr (debug_foci) log("command: ", ipc::str(command));
            if (command == ipc::cmd_w_data)
            {
                if (!lParam) return 0;
                auto data = window_recv_command(lParam);
                command = data.cmd;
                if constexpr (debug_foci) log("\tsubcommand: ", ipc::str(command));
                if (command == ipc::make_offer) // Group focus offer.
                {
                    auto ctrl_click = ctrl_pressed() && lbutton_pressed();
                    auto local_target = master.hWnd;
                    auto target_list = std::span<ui32>{ (ui32*)data.ptr, data.len / sizeof(ui32) };
                    if (ctrl_click && mfocus.update(target_list, local_target)) // Block foreign offers.
                    {
                        if constexpr (debug_foci) log("\tGot group focus offer");
                        for (auto target : target_list)
                        {
                            if constexpr (debug_foci) log("\thwnd=", utf::to_hex(target));
                            window_send_command(target, ipc::main_focus, local_target);
                        }
                    }
                    else
                    {
                        if constexpr (debug_foci)
                        {
                            log(ansi::err("Unexpected focus offer:"));
                            for (auto target : target_list) log(ansi::err("\thwnd=", utf::to_hex(target)));
                        }
                    }
                }
                else if (command == ipc::pass_state) // Keybd state.
                {
                    if (data.len == sizeof(vkstat))
                    {
                        auto state_data = std::span<byte>{ (byte*)data.ptr, data.len };
                        std::copy(state_data.begin(), state_data.end(), vkstat.begin());
                    }
                }
                else if (command == ipc::pass_input) // Keybd input.
                {
                    auto input_data = qiew{ (char*)data.ptr, data.len };
                    auto keybd = input::syskeybd{};
                    if (stream.gears)
                    if (keybd.load(input_data))
                    {
                        auto& gear = *stream.gears;
                        keymod = keybd.ctlstat;
                        stream.m.ctlstat = keymod;
                        keybd.syncto(gear);
                        gear.gear_id = gear.bell::id; // Restore gear id.
                        gear.touched = {};
                        stream.keybd(gear);
                    }
                }
                else command = 0;
            }
            else if (command == ipc::main_focus)
            {
                auto focus_bus_on = mfocus.set_owner(lParam);
                if (!focus_bus_on)
                {
                    base::enqueue([&](auto& /*boss*/)
                    {
                        base::signal(tier::release, input::events::focus::set::on, { .gear_id = stream.gears->id, .focus_type = solo::on });
                        if (mfocus.wheel) window_post_command(ipc::sync_state);
                    });
                }
            }
            else if (command == ipc::drop_focus)
            {
                auto focus_bus_on = mfocus.clear();
                keybd_wipe_vkstat();
                keybd_send_state();
                if (focus_bus_on)
                {
                    base::enqueue([&](auto& /*boss*/)
                    {
                        auto seed = base::signal(tier::release, input::events::focus::set::off, { .gear_id = stream.gears->id });
                    });
                }
            }
            else if (command == ipc::solo_focus)
            {
                auto local_target = (ui32)master.hWnd;
                auto target_list = mfocus.set_solo(local_target);
                for (auto target : target_list) window_send_command(target, ipc::drop_focus);
            }
            else if (command == ipc::sync_state)
            {
                keybd_send_state();
            }
            else if (command == ipc::take_focus)
            {
                window_make_focused();
            }
            else if (command == ipc::expose_win)
            {
                window_make_exposed();
            }
            else if (command == ipc::make_ontop)
            {
                window_make_topmost(true);
            }
            else if (command == ipc::set_normal)
            {
                window_make_topmost(faux);
            }
            else command = 0;
            return command;
        }
        void focus_event(bool new_focus_state)
        {
            auto local_target = (ui32)master.hWnd;
            if (auto [changed, target_list] = mfocus.set_focus(local_target, new_focus_state); changed)
            {
                if (new_focus_state)
                {
                    keybd_read_vkstat(); // It must be called in current thread.
                    for (auto target : target_list.value()) window_send_command(target, ipc::main_focus, local_target);
                }
                else if (target_list) // Send to all that the focus is going to lost.
                {
                    for (auto target : target_list.value()) window_send_command(target, ipc::drop_focus);
                }
            }
        }
        void timer_event(arch eventid)
        {
            if (eventid == timers::clipboard)
            {
                layer_timer_stop(master, timers::clipboard);
                sync_clipboard();
            }
            else if (eventid == timers::rightshift)
            {
                auto new_rs = keybd_read_pressed(vkey::rshift);
                if (!new_rs)
                {
                    layer_timer_stop(master, timers::rightshift);
                    keybd_sync_state();
                    //::GetKeyboardState(vkstat.data()); // Sync with thread kb state.
                    //if (keymod & input::hids::RShift)
                    //{
                    //    keymod &= ~input::hids::RShift;
                    //    keybd_send_state(vkey::shift, input::key::released, input::key::map::data(input::key::RightShift).scan, {}, {}, true);
                    //}
                }
            }
            else if (eventid == timers::blink) base::enqueue([&](auto& /*boss*/)
            {
                if (fsmode == winstate::minimized) return;
                auto visible = blinky.live;
                if (mfocus.focused() && visible)
                {
                    blinky.hide();
                    netxs::set_flag<task::blink>(reload);
                }
                else if (!visible) // Do not blink without focus.
                {
                    blinky.show();
                    netxs::set_flag<task::blink>(reload);
                }
                update_gui();
            });
        }
        void sys_command(si32 menucmd, many args = {})
        {
            if (menucmd == syscmd::update && !reload) return;
            if (menucmd == syscmd::tunecellheight)
            {
                if (isbusy.exchange(true) || args.empty() || netxs::any_get_or(args.front(), 0.f) == 0.f)
                {
                    return;
                }
            }
            base::enqueue([&, menucmd, args](auto& /*boss*/)
            {
                //log("sys_command: menucmd=", utf::to_hex_0x(menucmd));
                switch (menucmd)
                {
                    case syscmd::maximize: set_state(fsmode == winstate::maximized ? winstate::normal : winstate::maximized); break;
                    case syscmd::minimize: set_state(winstate::minimized); break;
                    case syscmd::restore:  set_state(winstate::normal);    break;
                    //todo implement
                    //case syscmd::move:          break;
                    //case syscmd::monitorpower:  break;
                    case syscmd::close:  window_shutdown(); break;
                    //case syscmd::update: update_gui(); break;
                    //
                    case syscmd::resetwheelaccum: ResetWheelAccumulator();  break;
                    case syscmd::tunecellheight:  IncreaseCellHeight(args); break;
                    case syscmd::resetcellheight: ResetCellHeight();        break;
                    case syscmd::fullscreen:      ToggleFullscreenMode();   break;
                    case syscmd::toggleaamode:    ToggleAntialiasingMode(); break;
                    case syscmd::rollfontlist:    RollFontList(args);       break;
                    case syscmd::warpwindow:      WarpWindow(args);         break;
                    case syscmd::move:            MoveWindow(args);         break;
                    case syscmd::focusnextwindow: FocusNextWindow(args);    break;
                    case syscmd::zorder:          ZOrder(args);             break;
                }
                update_gui();
            });
        }
        void book_clipboard()
        {
            auto random_delay = 150ms + datetime::milliseconds(os::process::id.second) / 2; // Delay in random range from 150ms upto 650ms.
            layer_timer_start(master, random_delay, timers::clipboard);
        }
        void sync_clipboard()
        {
            os::clipboard::sync(master.hWnd, stream, stream.intio, gridsz);
        }
        void update_input_field_list(si32 acpStart, si32 acpEnd)
        {
            fields.clear();
            auto inputfield_request = ui::e2::command::request::inputfields.param({ .gear_id = stream.gears->id, .acpStart = acpStart, .acpEnd = acpEnd });
            stream.send_input_fields_request(*this, inputfield_request);
            // We can't sync with the ui here. This causes a deadlock.
            //auto inputfield_request = base::signal(tier::general, ui::e2::command::request::inputfields, { .gear_id = stream.gears->id, .acpStart = acpStart, .acpEnd = acpEnd }); // pro::focus retransmits as a tier::release for focused objects.
            fields = inputfield_request.wait_for();
            auto win_area = blinky.area;
            if (fields.empty()) fields.push_back(win_area);
            else for (auto& f : fields)
            {
                f.size *= cellsz;
                f.coor *= cellsz;
                f.coor += win_area.coor;
            }
        }
        void connect()
        {
            sync_os_settings();
            if (!(layer_create(master, this, config.wincoord, config.gridsize, border, cellsz)
               && layer_create(blinky)
               && layer_create(header)
               && layer_create(footer)
               && layer_create(tooltip_layer)))
            {
                os::dtvt::flagsz = true;
                os::dtvt::flagsz.notify_all();
                return;
            }
            else
            {
                os::dtvt::gridsz = (master.area.size - border) / std::max(cellsz, dot_11);
                os::dtvt::flagsz = true; // Notify app::shared::splice.
                os::dtvt::flagsz.notify_all();
                auto lock = bell::sync();
                normsz = master.area;
                size_window();
                set_state(config.win_state);
                update_gui();
                window_initilize();

                on(tier::mouserelease, input::key::MouseDragStart, [&](hids& gear)
                {
                    if (fsmode != winstate::normal) return;
                    moving = true;
                    mouse_send_halt();
                    auto dxdy = twod{ std::round(gear.delta.get() * cellsz) };
                    move_window(dxdy);
                    sync_pixel_layout(); // Align grips and shadow.
                });
                on(tier::mouserelease, input::key::LeftDoubleClick, [&](hids& /*gear*/)
                {
                         if (fsmode == winstate::maximized) set_state(winstate::normal);
                    else if (fsmode == winstate::normal)    set_state(winstate::maximized);
                });
                on(tier::mouserelease, input::key::MouseWheel, [&](hids& gear)
                {
                    zoom_by_wheel(gear.whlfp, faux);
                });
                LISTEN(tier::release, input::events::focus::set::any, seed, -, (treeid = datetime::uniqueid(), digest = ui64{}))
                {
                    auto deed = bell::protos();
                    auto state = deed == input::events::focus::set::on.id;
                    stream.sysfocus.send(stream.intio, seed.gear_id, state, seed.focus_type, treeid, ++digest);
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
                        window_set_title(filtered);
                    }
                    auto window_id = id_t{};
                    stream.header.send(stream.intio, window_id, utf8);
                };
                LISTEN(tier::release, e2::form::prop::ui::footer, utf8)
                {
                    update_footer();
                    auto window_id = id_t{};
                    stream.footer.send(stream.intio, window_id, utf8);
                };
                base::broadcast(tier::anycast, e2::form::upon::started, This());
            }
            auto winio = std::thread{ [&]
            {
                auto sync = [&](view data)
                {
                    auto lock = bell::sync();
                    stream.sync(data);
                    update_gui();
                    //todo combine these two?
                    stream.request_jgc(stream.intio);
                    stream.request_images(stream.intio);
                };
                directvt::binary::stream::reading_loop(stream.intio, sync);
                stream.stop(); // Wake up waiting objects, if any.
                window_shutdown(); // Interrupt dispatching.
            }};
            window_message_pump();
            //for (auto& l : layers) layer_delete(l);
            stream.intio.shut(); // Close link to server. Interrupt binary reading loop.
            base::dequeue(); // Clear task queue.
            winio.join();
        }
    };
}

#if defined(_WIN32)

namespace netxs::gui
{
    struct window : winbase
    {
        struct tsfl : ITfContextOwnerCompositionSink, // To declare we are composition owner.
                      ITfContextOwner,
                      ITfTextEditSink, // To catch composition updates.
                      ITfEditSession
        {
            window&                        owner;
            ComPtr<ITfThreadMgrEx>         tsf_thread_manager;
            ComPtr<ITfDocumentMgr>         tsf_document_manager;
            ComPtr<ITfContext>             tsf_context;
            ComPtr<ITfSource>              tsf_source;
            ComPtr<ITfCategoryMgr>         tsf_category_manager;
            ComPtr<ITfDisplayAttributeMgr> tsf_attribute_manager;
            TfClientId                     tsf_registration_id = {};
            DWORD                          dwCookieContextOwner = TF_INVALID_COOKIE;
            DWORD                          dwCookieTextEditSink = TF_INVALID_COOKIE;

            tsfl(window& owner) // start() should be run under UI lock to be able to query input fields.
                : owner{ owner }
            { }
            #define log(...)

            // IUnknown
            ULONG refs = 1;
            STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj)
            {
                if (!ppvObj) return E_POINTER;
                *ppvObj = nullptr;
                log("call: QueryInterface ", os::guid(riid));
                     if (::IsEqualGUID(riid, IID_ITfContextOwner))                { *ppvObj = (ITfContextOwner*)this;                           log("    ask: IID_ITfContextOwner"); }
                else if (::IsEqualGUID(riid, IID_ITfTextEditSink))                { *ppvObj = (ITfTextEditSink*)this;                           log("    ask: IID_ITfTextEditSink"); }
                else if (::IsEqualGUID(riid, IID_ITfContextOwnerCompositionSink)) { *ppvObj = (ITfContextOwnerCompositionSink*)this;            log("    ask: IID_ITfContextOwnerCompositionSink"); }
                else if (::IsEqualGUID(riid, IID_IUnknown))                       { *ppvObj = (IUnknown*)(ITfContextOwnerCompositionSink*)this; log("    ask: IID_IUnknown"); }
                if (*ppvObj)
                {
                    AddRef();
                    return S_OK;
                }
                else return E_NOINTERFACE;
            }
            ULONG STDMETHODCALLTYPE AddRef()  { log("call: AddRef ", refs, " +1"); return InterlockedIncrement(&refs); }
            ULONG STDMETHODCALLTYPE Release() { log("call: DecRef ", refs, " -1"); auto r = InterlockedDecrement(&refs); if (r == 0) delete this; return r; }

            void fill_attr(cell& mark, TfGuidAtom atom)
            {
                if (atom == TF_INVALID_GUIDATOM) mark.und(unln::dashed);
                else
                {
                    auto guid = GUID{};
                    auto attr = TF_DISPLAYATTRIBUTE{};
                    auto info = ComPtr<ITfDisplayAttributeInfo>{};
                    if (SUCCEEDED(tsf_category_manager->GetGUID(atom, &guid))
                     && SUCCEEDED(tsf_attribute_manager->GetDisplayAttributeInfo(guid, info.GetAddressOf(), nullptr))
                     && SUCCEEDED(info->GetAttributeInfo(&attr)))
                    {
                        auto color = [](auto c){ return argb{ (ui32)(c.type == TF_CT_COLORREF ? c.cr : ::GetSysColor(c.nIndex)) }; };
                        if (attr.crText.type != TF_CT_NONE    ) mark.fgc(color(attr.crText));
                        if (attr.crBk.type   != TF_CT_NONE    ) mark.bgc(color(attr.crBk));
                        if (attr.crLine.type != TF_CT_NONE    ) mark.unc(color(attr.crLine));
                        if (attr.lsStyle     == TF_LS_SOLID   ) mark.und(unln::line);
                        if (attr.fBoldLine   == TRUE          ) mark.und(unln::biline);
                        if (attr.lsStyle     == TF_LS_DOT     ) mark.und(unln::dotted);
                        if (attr.lsStyle     == TF_LS_DASH    ) mark.und(unln::dashed);
                        if (attr.lsStyle     == TF_LS_SQUIGGLE) mark.und(unln::wavy);
                    }
                }
            }

            // ITfEditSession
            STDMETHODIMP DoEditSession(TfEditCookie ec)
            {
                log(" call: DoEditSession ec=", utf::to_hex(ec));
                auto composition = ComPtr<ITfRange>{};
                auto utf16 = wide{};
                auto width = LONG{};
                auto fixed = LONG{ LONG_MAX };
                auto caret = LONG{ LONG_MAX };
                auto count = ULONG{};
                auto attrs = std::vector<std::pair<si32, cell>>{};
                auto guids = std::to_array({ &GUID_PROP_ATTRIBUTE, &GUID_PROP_COMPOSING });
                auto piece = std::array<wchr, 64>{};
                auto props = ComPtr<ITfReadOnlyProperty>{};
                auto parts = ComPtr<IEnumTfRanges>{};
                if (SUCCEEDED(tsf_context->GetStart(ec, composition.GetAddressOf()))
                 && SUCCEEDED(composition->ShiftEnd(ec, LONG_MAX, &width, nullptr))
                 && SUCCEEDED(tsf_context->TrackProperties(guids.data(), (ULONG)guids.size(), nullptr, 0, props.GetAddressOf()))
                 && SUCCEEDED(props->EnumRanges(ec, parts.GetAddressOf(), composition.Get())))
                {
                    auto ranges = std::array<ITfRange*, 15>{};
                    while (parts->Next((ULONG)ranges.size(), ranges.data(), &count), count)
                    {
                        for (auto range : std::span{ ranges.data(), count })
                        {
                            auto marker = cell{};
                            auto buffer = VARIANT{};
                            auto length = utf16.size();
                            auto values = std::array<TF_PROPERTYVAL, guids.size()>{};
                            auto v_iter = ComPtr<IEnumTfPropertyValue>{};
                            ::VariantInit(&buffer);
                            if (SUCCEEDED(props->GetValue(ec, range, &buffer))
                             && SUCCEEDED(buffer.punkVal->QueryInterface(IID_IEnumTfPropertyValue, (void**)v_iter.GetAddressOf()))
                             && SUCCEEDED(v_iter->Next((ULONG)guids.size(), values.data(), nullptr)))
                            {
                                for (auto& v : values)
                                {
                                    auto is_si32 = V_VT(&v.varValue) == VT_I4;
                                    auto int_val = is_si32 ? V_I4(&v.varValue) : 0;
                                         if (::IsEqualGUID(v.guidId, GUID_PROP_ATTRIBUTE)) fill_attr(marker, int_val);
                                    else if (fixed == LONG_MAX && int_val && ::IsEqualGUID(v.guidId, GUID_PROP_COMPOSING)) fixed = (LONG)length;
                                    ::VariantClear(&v.varValue);
                                }
                            }
                            ::VariantClear(&buffer);
                            while (SUCCEEDED(range->GetText(ec, TF_TF_MOVESTART, piece.data(), (ULONG)piece.size(), &count)) && count)
                            {
                                utf16.append(piece.data(), count);
                                if (count != piece.size()) break;
                            }
                            if (fixed != LONG_MAX) // Store attributes only for unstable segments. Comment it to allow colored input.
                            {
                                auto delta = utf16.size() - length;
                                if (delta) attrs.emplace_back((si32)delta, marker);
                            }
                            range->Release();
                        }
                        count = 0;
                    }
                    auto selection = TF_SELECTION{};
                    if (SUCCEEDED(tsf_context->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &selection, &count)) && count)
                    {
                        auto hcond = TF_HALTCOND{ .pHaltRange = selection.range, .aHaltPos = selection.style.ase == TF_AE_START ? TF_ANCHOR_START : TF_ANCHOR_END };
                        auto start = ComPtr<ITfRange>{};
                        if (SUCCEEDED(tsf_context->GetStart(ec, start.GetAddressOf()))) start->ShiftEnd(ec, LONG_MAX, &caret, &hcond);
                        if (selection.range) selection.range->Release();
                    }
                    if (fixed && utf16.size()) // Drop fixed segment from composition.
                    {
                        auto range = ComPtr<ITfRange>{};
                        auto ok = SUCCEEDED(tsf_context->GetStart(ec, range.GetAddressOf()))
                               && SUCCEEDED(range->ShiftEnd(ec, fixed, &width, nullptr))
                               && SUCCEEDED(range->SetText(ec, 0, nullptr, 0));
                        if (!ok)
                        {
                            log(ansi::err("range->SetText failed"));
                        }
                    }
                }
                auto whole = wiew{ utf16 };
                auto rigid = whole.substr(0, fixed);
                auto fluid = whole.substr(rigid.size());
                auto anons = ansi::escx{};
                caret = std::clamp(caret - (LONG)rigid.size(), LONG{}, (LONG)fluid.size());
                if (fluid.size())
                {
                    auto cache = fluid;
                    auto brush = cell{};
                    auto index = 0;
                    for (auto& [l, c] : attrs)
                    {
                        c.scan_attr(brush, anons);
                        if (caret >= index && caret < index + l)
                        {
                            auto s = caret - index;
                            utf::to_utf(cache.substr(0, s), anons);
                            anons.scp(); // Inline caret.
                            utf::to_utf(cache.substr(s, l - s), anons);
                        }
                        else utf::to_utf(cache.substr(0, l), anons);
                        cache.remove_prefix(l);
                        index += l;
                    }
                    if (caret == index) anons.scp(); // Inline caret.
                }
                auto yield = utf::to_utf(rigid);
                log(" whole=", ansi::hi(utf::to_utf(whole)), " fixed=", ansi::hi(yield),
                  "\n fluid=", ansi::hi(utf::to_utf(fluid)), " anons=", ansi::pushsgr().hi(anons).popsgr(), " attrs=", attrs.size(), " cursor=", caret);
                if (yield.size()) owner.keybd_send_input(yield, input::keybd::type::imeinput);
                owner.keybd_send_input(anons, input::keybd::type::imeanons);
                return S_OK;
            }

            // ITfContextOwnerCompositionSink
            STDMETHODIMP OnStartComposition(ITfCompositionView*, BOOL* pfOk) { if (pfOk) *pfOk = TRUE; return S_OK; }
            STDMETHODIMP OnUpdateComposition(ITfCompositionView*, ITfRange*) { return S_OK; }
            STDMETHODIMP OnEndComposition(ITfCompositionView*)               { return S_OK; }

            // ITfTextEditSink
            STDMETHODIMP OnEndEdit(ITfContext* /*pic*/, TfEditCookie /*ecReadOnly*/, ITfEditRecord* /*pEditRecord*/)
            {
                log("call: OnEndEdit");
                auto hrSession = HRESULT{};
                if (!SUCCEEDED(tsf_context->RequestEditSession(tsf_registration_id, this, TF_ES_READWRITE | TF_ES_ASYNC, &hrSession))) // Enqueue an implicit call to DoEditSession(ec).
                {
                    log(ansi::err("RequestEditSession failed"));
                }
                else if (!SUCCEEDED(hrSession))
                {
                    log(ansi::err("hrSession failed"));
                }
                return S_OK;
            }

            // ITfContextOwner
            STDMETHODIMP GetAttribute(REFGUID /*rguidAttribute*/, VARIANT* /*pvarValue*/) { return E_NOTIMPL; }
            STDMETHODIMP GetACPFromPoint(POINT const* /*ptScreen*/, DWORD /*dwFlags*/, LONG* /*pacp*/) { return E_NOTIMPL; }
            STDMETHODIMP GetWnd(HWND* phwnd)
            {
                *phwnd = (HWND)owner.master.hWnd;
                return S_OK;
            }
            STDMETHODIMP GetStatus(TF_STATUS* pdcs)
            {
                log("call: GetStatus -> ", pdcs);
                if (!pdcs) return E_POINTER;
                pdcs->dwDynamicFlags = TS_SD_UIINTEGRATIONENABLE; // To indicate owr support of IME UI integration.
                pdcs->dwStaticFlags = TS_SS_TRANSITORY; // It is expected to have a short usage cycle.
                return S_OK;
            }
            STDMETHODIMP GetScreenExt(RECT* prc) // Returns the bounding box, in screen coordinates, of the document display.
            {
                if (prc)
                {
                    auto r = owner.master.live ? owner.master.area : rect{}; // Reply an empty rect if window is hidden.
                    //static auto random = true;
                    //if ((random = !random)) r.coor += dot_11; // Randomize coord to trigger IME to update their coords.
                    *prc = RECT{ r.coor.x, r.coor.y, r.coor.x + r.size.x, r.coor.y + r.size.y };
                    log("call: GetScreenExt -> ", r);
                }
                return S_OK;
            }
            STDMETHODIMP GetTextExt(LONG acpStart, LONG acpEnd, RECT* prc, BOOL* pfClipped) // Returns the bounding box, in screen coordinates, of the text at the specified character positions.
            {
                log("call: GetTextExt acpStart=", acpStart, " acpEnd=", acpEnd);
                if (pfClipped) *pfClipped = FALSE;
                if (prc)
                {
                    auto r = rect{};
                    if (owner.master.live) // Reply an empty rect if window is hidden.
                    {
                        //todo throttle by 400ms
                        owner.update_input_field_list(acpStart, acpEnd);
                        auto& field_list = owner.fields;
                        if (field_list.empty()) return GetScreenExt(prc);
                        else
                        {
                            auto head = field_list.begin();
                            auto tail = field_list.end();
                            while (head != tail && !head->trim(owner.master.area)) ++head; // Drop all fields that outside client.
                            if (head != tail)
                            {
                                r = field_list.front();
                                log(" field: ", r);
                                while (head != tail)
                                {
                                    auto f = *head++;
                                    if (f.trim(owner.master.area))
                                    {
                                        log(" field: ", f);
                                        r.unitewith(f);
                                    }
                                }
                            }
                        }
                    }
                    *prc = RECT{ r.coor.x, r.coor.y, r.coor.x + r.size.x, r.coor.y + r.size.y };
                }
                return S_OK;
            }

            void set_focus()
            {
                if (!tsf_thread_manager) start();
                if (tsf_thread_manager) tsf_thread_manager->SetFocus(tsf_document_manager.Get());
            }
            void start()
            {
                log("call: start");
                auto ec = TfEditCookie{};
                auto ok = SUCCEEDED(::CoInitialize(NULL)) // TSF supports STA only.
                       && SUCCEEDED(::CoCreateInstance(CLSID_TF_CategoryMgr,         NULL, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr,         (void**)tsf_category_manager.GetAddressOf()))
                       && SUCCEEDED(::CoCreateInstance(CLSID_TF_DisplayAttributeMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfDisplayAttributeMgr, (void**)tsf_attribute_manager.GetAddressOf()))
                       && SUCCEEDED(::CoCreateInstance(CLSID_TF_ThreadMgr,           NULL, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr,           (void**)tsf_thread_manager.GetAddressOf()))
                       && SUCCEEDED(tsf_thread_manager->Activate(&tsf_registration_id))
                       && SUCCEEDED(tsf_thread_manager->CreateDocumentMgr(tsf_document_manager.GetAddressOf()))
                       && SUCCEEDED(tsf_document_manager->CreateContext(tsf_registration_id, 0, (ITfContextOwnerCompositionSink*)this, tsf_context.GetAddressOf(), &ec))
                       && SUCCEEDED(tsf_context->QueryInterface(IID_ITfSource, (void**)tsf_source.GetAddressOf()))
                       && SUCCEEDED(tsf_source->AdviseSink(IID_ITfContextOwner, (ITfContextOwner*)this, &dwCookieContextOwner))
                       && SUCCEEDED(tsf_source->AdviseSink(IID_ITfTextEditSink, (ITfTextEditSink*)this, &dwCookieTextEditSink))
                       && SUCCEEDED(tsf_document_manager->Push(tsf_context.Get()));
                if (ok)
                {
                    log("TSF activated",
                               "\n    tsf_document_manager=", tsf_document_manager.Get(),
                               "\n    tsf_context=", tsf_context.Get(),
                               "\n    tsf_source=", tsf_source.Get(),
                               "\n    tsf_category_manager=", tsf_category_manager.Get(),
                               "\n    tsf_attribute_manager=", tsf_attribute_manager.Get(),
                               "\n    dwCookieTextEditSink=", dwCookieTextEditSink,
                               "\n    dwCookieContextOwner=", dwCookieContextOwner);
                }
                else
                {
                    log("TSF activation failed");
                }
            }
            void stop()
            {
                log("call: stop");
                if (dwCookieTextEditSink != TF_INVALID_COOKIE) tsf_source->UnadviseSink(dwCookieTextEditSink);
                if (dwCookieContextOwner != TF_INVALID_COOKIE) tsf_source->UnadviseSink(dwCookieContextOwner);
                if (tsf_document_manager)                      tsf_document_manager->Pop(TF_POPF_ALL);
                if (tsf_thread_manager)                        tsf_thread_manager->Deactivate();
                ::CoUninitialize();
            }
            #undef log
        };

        tsfl tslink; // window: TSF link.
        MSG  winmsg; // window: Last OS window message.
        text toUTF8; // window: UTF-8 conversion buffer.
        wide toWIDE; // window: UTF-16 conversion buffer.

        window(auto&& ...Args)
            : winbase{ Args... },
              tslink{ *this },
              winmsg{}
        {
            auto proc = (LONG(_stdcall *)(si32))::GetProcAddress(::GetModuleHandleA("user32.dll"), "SetProcessDpiAwarenessInternal");
            if (proc) proc(2/*PROCESS_PER_MONITOR_DPI_AWARE*/);
        }

        bits layer_get_bits(layer& s, bool zeroize = faux)
        {
            if (s.hdc && s.area)
            {
                if (s.resized())
                {
                    auto ptr = (void*)nullptr;
                    auto bmi = BITMAPINFO{ .bmiHeader = { .biSize        = sizeof(BITMAPINFOHEADER),
                                                          .biWidth       = s.area.size.x,
                                                          .biHeight      = -s.area.size.y,
                                                          .biPlanes      = 1,
                                                          .biBitCount    = 32,
                                                          .biCompression = BI_RGB }};
                    if (auto hbm = ::CreateDIBSection((HDC)s.hdc, &bmi, DIB_RGB_COLORS, &ptr, 0, 0)) // 0.050 ms
                    {
                        //auto new_data = bits{ std::span<argb>{ (argb*)ptr, (sz_t)s.area.size.x * s.area.size.y }, s.area };
                        //if (!zeroize) // Crop.
                        //{
                        //    auto d = new_data.data();
                        //    auto s = s.data.data();
                        //    auto w = std::min(s.prev.size.x, s.area.size.x) * sizeof(argb);
                        //    auto h = std::min(s.prev.size.y, s.area.size.y);
                        //    while (h--)
                        //    {
                        //        std::memcpy(d, s, w);
                        //        d += s.area.size.x;
                        //        s += s.prev.size.x;
                        //    }
                        //}
                        //data = new_data;
                        ::DeleteObject(::SelectObject((HDC)s.hdc, hbm));
                        zeroize = faux;
                        s.prev.size = s.area.size;
                        s.data = bits{ std::span<argb>{ (argb*)ptr, (sz_t)s.area.size.x * s.area.size.y }, s.area };
                    }
                    else log("%%Compatible bitmap creation error: %ec%", prompt::gui, ::GetLastError());
                }
                if (zeroize) s.wipe();
            }
            s.data.move(s.area.coor);
            return s.data;
        }
        void layer_delete(layer& s)
        {
            if (s.hdc)
            {
                ::DeleteDC((HDC)s.hdc);
                s.hdc = {};
            }
            for (auto eventid : s.klok) ::KillTimer((HWND)s.hWnd, eventid);
        }
        void layer_timer_start(layer& s, span elapse, ui32 eventid)
        {
            if (eventid)
            {
                if (std::find(s.klok.begin(), s.klok.end(), eventid) == s.klok.end()) s.klok.push_back(eventid);
                ::SetTimer((HWND)s.hWnd, eventid, datetime::round<ui32>(elapse), nullptr);
            }
        }
        void layer_timer_stop(layer& s, ui32 eventid)
        {
            auto iter = std::find(s.klok.begin(), s.klok.end(), eventid);
            if (iter != s.klok.end())
            {
                ::KillTimer((HWND)s.hWnd, eventid);
                s.klok.erase(iter);
            }
        }
        void layer_move_all()
        {
            auto lock = ::BeginDeferWindowPos((si32)layers.size());
            for (auto& l : layers)
            {
                auto& p = l.get();
                if (p.prev.coor(p.live ? p.area.coor : p.hidden))
                {
                    lock = ::DeferWindowPos(lock, (HWND)p.hWnd, 0, p.prev.coor.x, p.prev.coor.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
                    if (!lock) { log("%%DeferWindowPos returns unexpected result: %ec%", prompt::gui, ::GetLastError()); }
                }
            }
            ::EndDeferWindowPos(lock);
        }
        //todo static
        void layer_present(layer& s)
        {
            if (!s.hdc) return;
            auto windowmoved = s.prev.coor(s.live ? s.area.coor : s.hidden);
            if (s.sync.empty())
            {
                if (windowmoved) // Hide window. Windows Server Core doesn't hide windows by ShowWindow(). Details: https://devblogs.microsoft.com/oldnewthing/20041028-00/?p=37453.
                {
                    ::SetWindowPos((HWND)s.hWnd, 0, s.prev.coor.x, s.prev.coor.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOSENDCHANGING | SWP_NOACTIVATE);
                }
                return;
            }
            auto blend_props = BLENDFUNCTION{ .BlendOp = AC_SRC_OVER, .SourceConstantAlpha = 255, .AlphaFormat = AC_SRC_ALPHA };
            auto bitmap_coor = POINT{};
            auto window_coor = POINT{ s.prev.coor.x, s.prev.coor.y };
            auto bitmap_size = SIZE{ s.area.size.x, s.area.size.y };
            auto update_area = RECT{};
            auto update_info = UPDATELAYEREDWINDOWINFO{ .cbSize   = sizeof(UPDATELAYEREDWINDOWINFO),
                                                        .pptDst   = windowmoved ? &window_coor : nullptr,
                                                        .psize    = &bitmap_size,
                                                        .hdcSrc   = (HDC)s.hdc,
                                                        .pptSrc   = &bitmap_coor,
                                                        .pblend   = &blend_props,
                                                        .dwFlags  = ULW_ALPHA,
                                                        .prcDirty = &update_area };
            //log("hWnd=", hWnd);
            auto update_proc = [&]
            {
                //log("\t", rect{{ update_area.left, update_area.top }, { update_area.right - update_area. left, update_area.bottom - update_area.top }});
                auto ok = ::UpdateLayeredWindowIndirect((HWND)s.hWnd, &update_info);
                if constexpr (debugmode) if (!ok)
                {
                    log("%%UpdateLayeredWindowIndirect call failed (%%)", prompt::gui, ::GetLastError());
                }
            };
            //static auto clr = 0; clr++;
            for (auto r : s.sync)
            {
                // Hilight changes
                //auto c = layer_get_bits(s);
                //netxs::misc::cage(c, r, dent{ dot_11 }, cell::shaders::blend(argb{ (tint)((clr - 1) % 8 + 1) }));
                r.coor -= s.area.coor;
                update_area = { r.coor.x, r.coor.y, r.coor.x + r.size.x, r.coor.y + r.size.y };
                update_proc();
                update_info.pptDst = {};
            }
            if (update_info.pptDst) // Just move window.
            {
                update_area = {};
                update_proc();
            }
            s.sync.clear();
        }
        void window_set_title(view utf8) { ::SetWindowTextW((HWND)master.hWnd, utf::to_utf(utf8).data()); }
        bool keybd_test_pressed(si32 virtcod) { return !!(vkstat[virtcod] & 0x80); }
        bool keybd_test_toggled(si32 virtcod) { return !!(vkstat[virtcod] & 0x01); }
        //todo static
        bool keybd_read_pressed(si32 virtcod) { return !!(::GetAsyncKeyState(virtcod) & 0x8000); }
        bool keybd_read_toggled(si32 virtcod) { return !!(::GetAsyncKeyState(virtcod) & 0x0001); }
        bool keybd_read_input()
        {
            union key_state_t
            {
                ui32 token;
                struct
                {
                    ui32 repeat   : 16;// 0-15
                    si32 scancode : 8; // 16-23
                    si32 extended : 1; // 24
                    ui32 reserved : 4; // 25-28 (reserved)
                    ui32 context  : 1; // 29 (29 - context)
                    ui32 state    : 2; // 30-31: 0 - pressed, 1 - repeated, 2 - unknown, 3 - released
                } v;
            };
            auto param = key_state_t{ .token = (ui32)winmsg.lParam };
            if (param.v.state == 2/*unknown*/) return faux;
            auto virtcod = std::clamp((si32)winmsg.wParam, 0, 255);
            // When RightMod is pressed while the LeftMod is pressed it is treated as repeating.
            auto keystat = param.v.state == 0 ? input::key::pressed
                         : param.v.state == 1 ? input::key::repeated
                         /*param.v.state ==3*/: input::key::released;
            auto extflag = param.v.extended;
            auto scancod = param.v.scancode;
            auto keytype = 0;
            //log("Vkey=", utf::to_hex(virtcod), " scancod=", utf::to_hex(scancod), " pressed=", pressed ? "1":"0", " repeat=", repeat ? "1":"0");
            //todo process Alt+Numpads on our side: use TSF message pump.
            //if (auto rc = os::nt::TranslateMessageEx(&winmsg, 1/*It doesn't work as expected: Do not process Alt+Numpad*/)) // ::TranslateMessageEx() do not update IME.
            ::TranslateMessage(&winmsg); // Update kb buffer + update IME. Alt_Numpads are sent via WM_IME_CHAR for IME-aware kb layouts. ! All WM_IME_CHARs are sent before any WM_KEYUP.
                                      // ::ToUnicodeEx() doesn't update IME.
            auto m = MSG{};           // ::TranslateMessage(&winmsg) sequentially decodes a stream of VT_PACKET messages into a sequence of WM_CHAR messages. Return always non-zero for WM_*KEY*.
            auto msgtype = winmsg.message == WM_KEYUP || winmsg.message == WM_KEYDOWN ? WM_CHAR : WM_SYSCHAR;
            while (::PeekMessageW(&m, {}, msgtype, msgtype, PM_REMOVE)) toWIDE.push_back((wchr)m.wParam);
            if (toWIDE.size()) keytype = 1;
            else
            {
                while (::PeekMessageW(&m, {}, msgtype + 1/*Peek WM_DEADCHAR*/, msgtype + 1, PM_REMOVE)) toWIDE.push_back((wchr)m.wParam);
                if (toWIDE.size()) keytype = 2;
            }
            //log("\tvkey=", utf::to_hex(virtcod), " pressed=", pressed ? "1" : "0", " scancod=", scancod);
            //log("\t::TranslateMessage()=", rc, " toWIDE.size=", toWIDE.size(), " toWIDE=", ansi::hi(utf::debase<faux, faux>(utf::to_utf(toWIDE))), " key_type=", keytype);
            if (!mfocus.focused()) // ::PeekMessageW() could call wind_proc() inside for any non queued msgs like wind_proc(WM_KILLFOCUS).
            {
                toWIDE.clear();
                return faux;
            }
            if (virtcod == vkey::packet && toWIDE.size())
            {
                auto c = toWIDE.back();
                if (c >= 0xd800 && c <= 0xdbff)
                {
                    return faux; // Incomplete surrogate pair in VT_PACKET stream.
                }
            }
            ::GetKeyboardState(vkstat.data()); // Sync with thread kb state.
            if (keytype != 2) // Do not notify dead keys.
            {
                toUTF8.clear();
                if (keytype == 1)
                {
                    utf::to_utf(toWIDE, toUTF8);
                    if (keystat == input::key::released) // Only Alt+Numpad fires on release.
                    {
                        keybd_send_state(virtcod, keystat, scancod, extflag); // Release Alt. Send empty string.
                        keybd_send_input(toUTF8, input::keybd::type::imeinput); // Send Alt+Numpads result.
                        toWIDE.clear();
                        //print_vkstat("Alt+Numpad");
                        return true;
                    }
                }
                keybd_send_state(virtcod, keystat, scancod, extflag, toUTF8);
            }
            toWIDE.clear();
            //print_vkstat("keybd_read_input");
            return true;
        }
        void window_message_pump()
        {
            while (::GetMessageW(&winmsg, 0, 0, 0) > 0)
            {
                //log("\twinmsg=", utf::to_hex(winmsg.message), " coor=", twod{ winmsg.pt.x, winmsg.pt.y }, " wP=", utf::to_hex(winmsg.wParam), " lP=", utf::to_hex(winmsg.lParam), " hwnd=", utf::to_hex(winmsg.hwnd));
                //if (winmsg.message == 0xC060) keybd_sync_state(); // Unstick the Win key when switching to the same keyboard layout using Win+Space.
                if (mfocus.wheel && (winmsg.message == WM_KEYDOWN    || winmsg.message == WM_KEYUP || // Ignore all kb events in unfocused state.
                                     winmsg.message == WM_SYSKEYDOWN || winmsg.message == WM_SYSKEYUP))
                {
                    keybd_read_input();
                    sys_command(syscmd::update);
                }
                else
                {
                    if (keybd_test_pressed(vkey::rwin) || keybd_test_pressed(vkey::lwin)) keybd_sync_state(); // Hack: Unstick the Win key when switching to the same keyboard layout using Win+Space.
                    ::DispatchMessageW(&winmsg);
                }
            }
            tslink.stop();
        }
        void keybd_sync_state(si32 virtcod = {})
        {
            ::GetKeyboardState(vkstat.data());
            keybd_send_state(virtcod);
            //print_vkstat("keybd_sync_state");
        }
        void keybd_read_vkstat() // Loading without sending. Will be sent after the focus bus is turned on.
        {
            ::GetKeyboardState(vkstat.data());
            mfocus.offer = !mfocus.buson && ctrl_pressed(); // Check if we are focused by Ctrl+AnyClick to ignore that click.
            //print_vkstat("keybd_read_vkstat");
            tslink.set_focus();
        }
        void keybd_wipe_vkstat()
        {
            auto n = vkstat[vkey::numlock ];
            auto c = vkstat[vkey::capslock];
            auto s = vkstat[vkey::scrllock];
            auto k = vkstat[vkey::kana    ];
            auto r = vkstat[vkey::oem_roya];
            auto l = vkstat[vkey::oem_loya];
            vkstat = {}; // Keep keybd locks only.
            vkstat[vkey::numlock ] = n;
            vkstat[vkey::capslock] = c;
            vkstat[vkey::scrllock] = s;
            vkstat[vkey::kana    ] = k;
            vkstat[vkey::oem_roya] = r;
            vkstat[vkey::oem_loya] = l;
            ::SetKeyboardState(vkstat.data()); // Sync thread kb state.
            //print_vkstat("deactivate");
        }
        void keybd_sync_layout()
        {
            keybd_sync_state();
            //todo sync kb layout
            //auto hkl = ::GetKeyboardLayout(0);
            auto kblayout = wide(KL_NAMELENGTH, '\0');
            ::GetKeyboardLayoutNameW(kblayout.data());
            log("%%Keyboard layout changed to ", prompt::gui, utf::to_utf(kblayout));//, " lo(hkl),langid=", lo((arch)hkl), " hi(hkl),handle=", hi((arch)hkl));
        }
        void window_make_focused()       { restore_if_minimized(); ::SetFocus((HWND)master.hWnd); } // Calls WM_KILLFOCOS(prev) + WM_ACTIVATEAPP(next) + WM_SETFOCUS(next).
        void window_make_exposed()       { ::SetWindowPos((HWND)master.hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOACTIVATE); }
        void window_make_topmost(bool s) { ontop_state = s; ::SetWindowPos((HWND)master.hWnd, s ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE); }
        void window_make_foreground()    { ::SetForegroundWindow((HWND)master.hWnd); } //::AllowSetForegroundWindow(ASFW_ANY); } // Neither ::SetFocus() nor ::SetActiveWindow() can switch focus immediately.
        void window_shutdown()           { ::SendMessageW((HWND)master.hWnd, WM_CLOSE, NULL, NULL); }
        void window_cleanup()            { ::RemoveClipboardFormatListener((HWND)master.hWnd); ::PostQuitMessage(0); }
        twod mouse_get_pos()             { return twod{ winmsg.pt.x, winmsg.pt.y }; }
        void mouse_capture(si32 captured_by)
        {
            if (!std::exchange(heldby, heldby | captured_by))
            {
                ::SetCapture((HWND)master.hWnd);
                if constexpr (debug_foci) log("captured by ", captured_by == by::mouse ? "mouse" : "keybd");
            }
        }
        void mouse_release(si32 captured_by = -1)
        {
            if (std::exchange(heldby, heldby & ~captured_by) && !heldby)
            {
                if constexpr (debug_foci) log("released by ", captured_by == by::mouse ? "mouse" : captured_by == by::keybd ? "keybd" : "system");
                ::ReleaseCapture();
            }
        }
        void mouse_catch_outside()
        {
            auto ctrl_click = ctrl_pressed()                        // Detect Ctrl+LeftClick
                           && !master.area.hittest(mouse_get_pos()) // outside our window.
                           && keybd_read_pressed(vkey::lbutton);    //
                           //&& (keybd_read_pressed(vkey::lbutton) || keybd_read_pressed(vkey::rbutton) || keybd_read_pressed(vkey::mbutton));
            if (ctrl_click) // Try to make group focus offer before we lose focus.
            {
                auto target = ::WindowFromPoint(winmsg.pt);
                auto target_list = mfocus.copy();
                auto data = COPYDATASTRUCT{ .dwData = ipc::make_offer,
                                            .cbData = (DWORD)(target_list.size() * sizeof(ui32)),
                                            .lpData = (void*)target_list.data() };
                auto rc = ::SendMessageW(target, WM_COPYDATA, (WPARAM)master.hWnd, (LPARAM)&data);
                if constexpr (debug_foci)
                {
                    if (rc == ipc::make_offer) log(ansi::clr(greenlt, "Group focus offer accepted by hwnd=", utf::to_hex(target)));
                    else                       log(ansi::err("Failed to offer group focus to hwnd=", utf::to_hex(target)));
                }
            }
            mouse_release();
        }
        void keybd_send_block(view block)
        {
            auto target_list = mfocus.copy();
            auto local_hwnd = (ui32)master.hWnd;
            auto state_data = COPYDATASTRUCT{ .dwData = ipc::pass_state, .cbData = (DWORD)vkstat.size(), .lpData = (void*)vkstat.data() };
            auto input_data = COPYDATASTRUCT{ .dwData = ipc::pass_input, .cbData = (DWORD)block.size(),   .lpData = (void*)block.data() };
            for (auto target : target_list) // Send to group focused targets.
            {
                if (target != local_hwnd)
                if (ipc::pass_state != ::SendMessageW((HWND)(arch)target, WM_COPYDATA, (WPARAM)local_hwnd, (LPARAM)&state_data)
                 || ipc::pass_input != ::SendMessageW((HWND)(arch)target, WM_COPYDATA, (WPARAM)local_hwnd, (LPARAM)&input_data))
                {
                    mfocus.erase(target); // Drop failed targets.
                }
            }
        }
        void window_sync_taskbar(si32 new_state)
        {
            if (new_state == winstate::minimized) // In order to be in sync with winNT taskbar. Other ways don't work because explorer.exe tracks our window state on their side.
            {
                ::ShowWindow((HWND)master.hWnd, SW_MINIMIZE);
            }
            else if (new_state == winstate::maximized) // "ShowWindow(SW_MAXIMIZE)" makes the window transparent to the mouse when maximized to multiple monitors.
            {
                //todo It doesn't work that way. Sync with system ctx menu.
                //auto ctxmenu = ::GetSystemMenu((HWND)master.hWnd, FALSE);
                //::EnableMenuItem(ctxmenu, SC_RESTORE, MF_CHANGE | MF_ENABLED);
                //::EnableMenuItem(ctxmenu, SC_MAXIMIZE, MF_CHANGE | MF_GRAYED);
            }
            else
            {
                ::ShowWindow((HWND)master.hWnd, SW_RESTORE);
            }
        }
        void sync_os_settings()
        {
            auto dt = ULONG{};
            ::SystemParametersInfoW(SPI_GETWHEELSCROLLLINES, 0, &dt, FALSE);
            wdelta = std::max(WHEEL_DELTA / std::max((fp32)dt, 1.f), 1.f);
            auto a = TRUE;
            ::SystemParametersInfoA(SPI_GETCLIENTAREAANIMATION, 0, &a, 0);
            blinks.rate = a ? blinks.init : span::zero();
        }
        void window_initilize()
        {
            // Customize system ctx menu.
            auto closecmd = wide(100, '\0');
            auto ctxmenu = ::GetSystemMenu((HWND)master.hWnd, FALSE);
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
            for (auto& l : layers)
            {
                auto& p = l.get();
                ::ShowWindow((HWND)p.hWnd, std::exchange(mode, SW_SHOWNA));
            }
            ::AddClipboardFormatListener((HWND)master.hWnd); // It posts WM_CLIPBOARDUPDATE to sync clipboard anyway.
            sync_clipboard(); // Clipboard should be in sync at (before) startup.
            window_make_foreground();
        }

        //todo static
        cont window_recv_command(arch lParam) { auto& data = *(COPYDATASTRUCT*)lParam; return cont{ .cmd = (si32)data.dwData, .ptr = data.lpData, .len = data.cbData }; }
        void window_send_command(arch target, si32 command, arch lParam = {}) { ::SendMessageW((HWND)target, WM_USER, command, lParam); }
        void window_post_command(arch target, si32 command, arch lParam = {}) { ::PostMessageW((HWND)target, WM_USER, command, lParam); }
        rect window_get_fs_area(rect window_area)
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
        bool layer_create(layer& s, winbase* host_ptr = nullptr, twod win_coord = {}, twod grid_size = {}, dent border_dent = {}, twod cell_size = {})
        {
            auto window_proc = [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
            {
                //log("\tmsg=", utf::to_hex(msg), " wP=", utf::to_hex(wParam), " lP=", utf::to_hex(lParam), " hwnd=", utf::to_hex(hWnd));
                auto w = (winbase*)::GetWindowLongPtrW(hWnd, GWLP_USERDATA);
                if (!w) return ::DefWindowProcW(hWnd, msg, wParam, lParam);
                auto stat = LRESULT{};
                static auto hi = [](auto n){ return (si32)(si16)((n >> 16) & 0xffff); };
                static auto lo = [](auto n){ return (si32)(si16)((n >> 0 ) & 0xffff); };
                static auto xbttn = [](auto wParam){ return hi(wParam) == XBUTTON1 ? bttn::xbutton1 : bttn::xbutton2; };
                static auto moved = [](auto lParam){ auto& p = *((WINDOWPOS*)lParam); return !(p.flags & SWP_NOMOVE); };
                static auto coord = [](auto lParam){ auto& p = *((WINDOWPOS*)lParam); return twod{ p.x, p.y }; };
                static auto hover_win = HWND{};
                static auto hover_rec = TRACKMOUSEEVENT{ .cbSize = sizeof(TRACKMOUSEEVENT), .dwFlags = TME_LEAVE, .dwHoverTime = HOVER_DEFAULT };
                switch (msg)
                {
                    case WM_MOUSEMOVE:        if (std::exchange(hover_win, hWnd) != hover_win) ::TrackMouseEvent((hover_rec.hwndTrack = hWnd, &hover_rec));
                                              w->mouse_moved();                                  break; //todo mouse events are broken when IME is active (only work on lower rotated monitor half). TSF message pump?
                    case WM_TIMER:            w->timer_event(wParam);                            break;
                    case WM_MOUSELEAVE:       w->mouse_leave(); hover_win = {};                  break;
                    case WM_LBUTTONDOWN:      w->mouse_press(bttn::left,    true);               break;
                    case WM_LBUTTONUP:        w->mouse_press(bttn::left,    faux);               break;
                    case WM_RBUTTONDOWN:      w->mouse_press(bttn::right,   true);               break;
                    case WM_RBUTTONUP:        w->mouse_press(bttn::right,   faux);               break;
                    case WM_MBUTTONDOWN:      w->mouse_press(bttn::middle,  true);               break;
                    case WM_MBUTTONUP:        w->mouse_press(bttn::middle,  faux);               break;
                    case WM_XBUTTONDOWN:      w->mouse_press(xbttn(wParam), true);               break;
                    case WM_XBUTTONUP:        w->mouse_press(xbttn(wParam), faux);               break;
                    case WM_MOUSEWHEEL:       w->mouse_wheel(hi(wParam), 0);                     break;
                    case WM_MOUSEHWHEEL:      w->mouse_wheel(hi(wParam), 1);                     break;
                    case WM_CAPTURECHANGED:   w->mouse_catch_outside();                          break; // Catch outside clicks.
                    case WM_ACTIVATEAPP:      if (wParam == TRUE) w->window_make_focused();      break; // Do focus explicitly: Sometimes WM_SETFOCUS follows WM_ACTIVATEAPP, sometime not. explorer.exe gives us focus (w/o WM_SETFOCUS) when other window minimizing.
                    case WM_SETFOCUS:         if (wParam != (arch)hWnd) w->focus_event(true);    break; // Don't refocus. ::SetFocus calls twice wnd_proc(WM_KILLFOCUS+WM_SETFOCUS).
                    case WM_KILLFOCUS:        if (wParam != (arch)hWnd) w->focus_event(faux);    break; // Don't refocus.
                    case WM_COPYDATA:         stat = w->run_command(ipc::cmd_w_data, lParam);    break; // Receive command with data.
                    case WM_USER:             stat = w->run_command(wParam, lParam);             break; // Receive command.
                    case WM_CLIPBOARDUPDATE:  w->book_clipboard();                               break; // Schedule clipboard update.
                    case WM_INPUTLANGCHANGE:  w->keybd_sync_layout();                            break;
                    case WM_SETTINGCHANGE:    w->sync_os_settings();                             break;
                    case WM_WINDOWPOSCHANGED: if (moved(lParam)) w->check_window(coord(lParam)); break; // Check moving only. Windows moves our layers the way they wants without our control.
                    case WM_DISPLAYCHANGE:
                    case WM_DEVICECHANGE:     w->check_fsmode();                                 break; // Restore from maximized mode if resolution changed.
                    case WM_DESTROY:          w->window_cleanup();                               break;
                    case WM_SYSCOMMAND: switch (wParam & 0xFFF0) { case SC_MINIMIZE: w->sys_command(syscmd::minimize); return stat;
                                                                   case SC_MAXIMIZE: w->sys_command(syscmd::maximize); return stat;
                                                                   case SC_RESTORE:  w->sys_command(syscmd::restore);  return stat;
                                                                   case SC_CLOSE:    w->sys_command(syscmd::close);    return stat;
                                                                   //todo implement
                                                                   //case SC_MOVE:         w->sys_command(syscmd::move);         return stat;
                                                                   //case SC_MONITORPOWER: w->sys_command(syscmd::monitorpower); return stat;
                                                                   default: stat = TRUE; // An application should return zero only if it processes this message.
                                                                 } break; // Taskbar ctx menu to change the size and position.
                    //case WM_INITMENU: //todo The application can perform its own checking or graying by responding to the WM_INITMENU message that is sent before any menu is displayed.
                    //case WM_PAINT:   /*w->check_dx3d_state();*/ stat = ::DefWindowProcW(hWnd, msg, wParam, lParam); break; //dx3d specific
                    //case WM_MOUSEACTIVATE: stat = MA_NOACTIVATE; break; // Suppress window auto focus by mouse. Note: window always loses focus on any click outside.
                    //case WM_ENDSESSION:
                    //    if (wParam && alive.exchange(faux))
                    //    {
                    //             if (lParam & ENDSESSION_CLOSEAPP) os::signals::place(os::signals::close);
                    //        else if (lParam & ENDSESSION_LOGOFF)   os::signals::place(os::signals::logoff);
                    //        else                                   os::signals::place(os::signals::shutdown);
                    //    }
                    //    break;
                    default: stat = ::DefWindowProcW(hWnd, msg, wParam, lParam); break;
                }
                w->sys_command(syscmd::update);
                return stat;
            };
            static auto wc_defwin = WNDCLASSW{ .lpfnWndProc = ::DefWindowProcW, .lpszClassName = L"vtm_decor" };
            static auto wc_window = WNDCLASSW{ .lpfnWndProc = window_proc, /*.cbWndExtra = 2 * sizeof(LONG_PTR),*/ .hCursor = ::LoadCursorW(NULL, (LPCWSTR)IDC_ARROW), .lpszClassName = L"vtm" };
            static auto reg = ::RegisterClassW(&wc_defwin) && ::RegisterClassW(&wc_window);
            if (!reg)
            {
                log("%%window class registration error: %ec%", prompt::gui, ::GetLastError());
                return faux;
            }
            auto& wc = host_ptr ? wc_window : wc_defwin;
            auto owner = (HWND)master.hWnd;
            if (cell_size)
            {
                auto use_default_size = grid_size == dot_mx;
                auto use_default_coor = win_coord == dot_mx;
                if (use_default_size || use_default_coor) // Request size and position by creating a fake window.
                {
                    if (use_default_coor) win_coord = { CW_USEDEFAULT, CW_USEDEFAULT };
                    if (use_default_size) grid_size = { CW_USEDEFAULT, CW_USEDEFAULT };
                    else                  grid_size *= cell_size;
                    auto r = RECT{};
                    auto h = ::CreateWindowExW(0, wc_defwin.lpszClassName, 0, WS_OVERLAPPEDWINDOW, win_coord.x, win_coord.y, grid_size.x, grid_size.y, 0, 0, 0, 0);
                    ::GetWindowRect(h, &r);
                    ::DestroyWindow(h);
                    win_coord = twod{ r.left, r.top };
                    grid_size = twod{ r.right - r.left, r.bottom - r.top };
                    if (!grid_size) grid_size = cell_size * twod{ 80, 25 };
                }
                else grid_size *= cell_size;
            }
            auto hWnd = ::CreateWindowExW(WS_EX_NOREDIRECTIONBITMAP | WS_EX_LAYERED | (wc.hCursor ? 0 : WS_EX_TRANSPARENT),
                                          wc.lpszClassName, owner ? nullptr : wc.lpszClassName, // Title.
                                          /*WS_VISIBLE: it is invisible to suppress messages until initialized | */
                                          WS_POPUP | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU,
                                          win_coord.x, win_coord.y,
                                          grid_size.x, grid_size.y,
                                          owner, 0, 0, 0);
            if (!hWnd)
            {
                log("%%Window creation error: %ec%", prompt::gui, ::GetLastError());
                return faux;
            }
            else if (host_ptr)
            {
                ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)host_ptr);
            }
            s.hWnd = (arch)hWnd;
            s.hdc = (arch)::CreateCompatibleDC(NULL); // Only current thread owns created CompatibleDC.
            if (cell_size)
            {
                grid_size /= cell_size;
                s.area = rect{ win_coord, grid_size * cell_size } + border_dent;
            }
            return true;
        }
    };
}

#else

namespace netxs::gui
{
    struct window : winbase
    {
        window(auto&& ...Args)
            : winbase{ Args... }
        { }
        bool keybd_test_pressed(si32 /*virtcod*/) { return true; /*!!(vkstat[virtcod] & 0x80);*/ }
        bool keybd_test_toggled(si32 /*virtcod*/) { return true; /*!!(vkstat[virtcod] & 0x01);*/ }
        bool keybd_read_pressed(si32 /*virtcod*/) { return true; /*!!(::GetAsyncKeyState(virtcod) & 0x8000);*/ }
        bool keybd_read_toggled(si32 /*virtcod*/) { return true; /*!!(::GetAsyncKeyState(virtcod) & 0x0001);*/ }
        bool keybd_read_input() { return true; }
        void keybd_wipe_vkstat() {}
        void keybd_read_vkstat() {}
        void keybd_send_block(view /*block*/) {}
        void keybd_sync_layout() {}
        void keybd_sync_state(si32 /*virtcod*/) {}
        bool layer_create(layer& /*s*/, winbase* /*host_ptr*/ = nullptr, twod /*win_coord*/ = {}, twod /*grid_size*/ = {}, dent /*border_dent*/ = {}, twod /*cell_size*/ = {}) { return true; }
        void layer_move_all() {}
        void layer_present(layer& /*s*/) {}
        void layer_timer_start(layer& /*s*/, span /*elapse*/, ui32 /*eventid*/) {}
        void layer_timer_stop(layer& /*s*/, ui32 /*eventid*/) {}
        bits layer_get_bits(layer& /*s*/, bool /*zeroize*/ = faux) { return bits{}; }
        void window_sync_taskbar(si32 /*new_state*/) {}
        rect window_get_fs_area(rect window_area) { return window_area; }
        void window_send_command(arch /*target*/, si32 /*command*/, arch /*lParam*/ = {}) {}
        void window_post_command(arch /*target*/, si32 /*command*/, arch /*lParam*/ = {}) {}
        cont window_recv_command(arch /*lParam*/) { return cont{}; }
        void window_make_foreground() {}
        void window_make_focused() {}
        void window_make_exposed() {}
        void window_make_topmost(bool) {}
        void window_message_pump() {}
        void window_initilize() {}
        void window_shutdown() {}
        void window_cleanup() {}
        void window_set_title(view /*utf8*/) {}
        twod mouse_get_pos() { return twod{}; }
        void mouse_capture(si32 /*captured_by*/) {}
        void mouse_release(si32 /*released_by*/) {}
        void mouse_catch_outside() {}
        void sync_os_settings() {}
    };
}

#endif