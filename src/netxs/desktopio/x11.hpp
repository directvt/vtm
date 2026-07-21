// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

namespace x11
{
    #pragma pack(push, 1)
    template<class T>
    struct data_n_size
    {
        auto data() { return (void*)this; }
        auto size() { return sizeof(T::s); }
    };
    struct reply_header
    {
        byte status;            // 0 = Fail, 1 = Success.
        byte pad1;
        ui16 major_version;
        ui16 minor_version;
        ui16 additional_length; // Payload length in 4-byte chunks.
        // payload ...
    };
    struct session_t : data_n_size<session_t>
    {
        struct format : data_n_size<format>
        {
            struct
            {
                byte depth;          // 1 byte depth
                byte bits_per_pixel; // 1 byte bits_per_pixel
                byte scanline_pad;   // 1 byte scanline_pad
                byte pad[5];         // 5 pad  unused
            } s;
        };
        struct screen : data_n_size<screen>
        {
            struct depth : data_n_size<depth>
            {
                struct visual_type : data_n_size<visual_type>
                {
                    struct vclass
                    {
                        static constexpr auto StaticGray  = (byte)1;
                        static constexpr auto GrayScale   = (byte)2;
                        static constexpr auto StaticColor = (byte)3;
                        static constexpr auto PseudoColor = (byte)4;
                        static constexpr auto TrueColor   = (byte)5;
                        static constexpr auto DirectColor = (byte)6;
                    };
                    struct
                    {
                        ui32 visual_id;          // 4 ui32 visual_id
                        byte visual_class;       // 1 byte vclass
                        byte bits_per_rgb_value; // 1 byte bits_per_rgb_value
                        ui16 colormap_entries;   // 2 ui16 colormap_entries
                        ui32 red_mask;           // 4 ui32 red_mask
                        ui32 green_mask;         // 4 ui32 green_mask
                        ui32 blue_mask;          // 4 ui32 blue_mask
                        ui32 pad;                // 4 pad  unused
                    } s;
                };
                struct
                {
                    byte depth;               // 1 byte depth
                    byte pad1;                // 1 pad  unused
                    ui16 num_of_visual_types; // 2 n    number of visual_types in visuals
                    ui32 pad2;                // 4 pad  unused
                } s;
                std::vector<visual_type> list_of_visual_types; // 24*n  list_of_visual_types  visuals
            };
            struct
            {
                ui32 root_window_id;        // 4 ui32 WINDOW      root_window_id
                ui32 default_colormap;      // 4 ui32 COLORMAP    default_colormap
                ui32 white_pixel;           // 4 ui32             white_pixel
                ui32 black_pixel;           // 4 ui32             black_pixel
                ui32 current_input_masks;   // 4 ui32 SETofEVENT  current_input_masks
                ui16 width_in_pixels;       // 2 ui16             width_in_pixels
                ui16 height_in_pixels;      // 2 ui16             height_in_pixels
                ui16 width_in_millimeters;  // 2 ui16             width_in_millimeters
                ui16 height_in_millimeters; // 2 ui16             height_in_millimeters
                ui16 min_installed_maps;    // 2 ui16             min_installed_maps
                ui16 max_installed_maps;    // 2 ui16             max_installed_maps
                ui32 root_visual;           // 4 ui32 VisualId    root_visual
                byte backing_stores;        // 1 byte             backing_stores 0: Never, 1: WhenMapped, 2: Always
                byte save_unders;           // 1 byte BOOL        save_unders 0/1
                byte root_depth;            // 1 byte             root_depth
                byte number_of_depths;      // 1 byte             number of depths (list_of_depths) in allowed_depths
            } s;
            std::vector<depth> list_of_depths; // List of allowed_depths (n is always a multiple of 4)
        };
        struct
        {
            ui32 release_number;              // 4 ui32 buffer[0..3]   = release_number
            ui32 resource_id_base;            // 4 ui32 buffer[4..7]   = resource_id_base
            ui32 resource_id_mask;            // 4 ui32 buffer[8..11]  = resource_id_mask
            ui32 motion_buffer_size;          // 4 ui32 buffer[12..15] = motion_buffer_size
            ui16 vendor_length;               // 2 ui16 buffer[16..17] = vendor_length
            ui16 maximum_request_length;      // 2 ui16 buffer[18..19] = maximum_request_length
            byte number_of_screens;           // 1 byte buffer[20]     = number_of_screens in roots
            byte number_of_formats;           // 1 byte buffer[21]     = number_of_formats in pixmap_formats
            byte image_byte_order;            // 1 byte buffer[22]     = 0: LSBFirst, 1: MSBFirst
            byte bitmap_format_bit_order;     // 1 byte buffer[23]     = 0: LeastSignificant, 1: MostSignificant
            byte bitmap_format_scanline_unit; // 1 byte buffer[24]     = bitmap_format_scanline_unit
            byte bitmap_format_scanline_pad;  // 1 byte buffer[25]     = bitmap_format_scanline_pad
            byte min_keycode;                 // 1 byte buffer[26]     = min_keycode
            byte max_keycode;                 // 1 byte buffer[27]     = max_keycode
            byte pad[4];                      // 4 ui32 buffer[28..31] = unused
        } s;
        text                vendor_str;     // buffer[32..32+vendor_length] = vendor_str
        std::vector<format> pixmap_formats; // format * number_of_formats = pixmap_formats
        std::vector<screen> roots;          // screen * number_of_screens = roots (always a multiple of 4)
        sptr<os::ipc::stdcon>   x11connection;  // Active X11 socket connection.
        template<bool B = true>
        auto str() const
        {
            if (roots.empty()) return "no screen roots"s;
            auto str = utf::fprint("%%Connected: id_base/mask=%%/%% root_window_id=%% screens=%% vendor='%%'\n", prompt::x11,
                utf::to_hex(s.resource_id_base),
                utf::to_hex(s.resource_id_mask),
                utf::to_hex(roots.front().s.root_window_id),
                (si32)s.number_of_screens,
                utf::debase<faux, faux>(vendor_str));
            str += pixmap_formats.size() ? utf::fprint("    pixmap_formats(%%):\n", pixmap_formats.size()) : "    no pixmap_formats\n";
            for (auto& format : pixmap_formats)
            {
                auto& pf = format.s;
                str += utf::fprint("\tdepth=%% bpp=%% scanline_pad=%%\n", (si32)pf.depth, (si32)pf.bits_per_pixel, (si32)pf.scanline_pad);
            }
            str += roots.size() ? utf::fprint("    root screens(%%):\n", roots.size()) : "    no screen roots\n";
            for (auto& root : roots)
            {
                auto& sc = root.s;
                str += utf::fprint("     root_window_id="        , utf::to_hex(sc.root_window_id),
                                    "\n\t default_colormap="     , utf::to_hex(sc.default_colormap),
                                    "\n\t white_pixel="          , utf::to_hex(sc.white_pixel),
                                    "\n\t black_pixel="          , utf::to_hex(sc.black_pixel),
                                    "\n\t current_input_masks="  , utf::to_hex(sc.current_input_masks),
                                    "\n\t width_in_pixels="      , sc.width_in_pixels,
                                    "\n\t height_in_pixels="     , sc.height_in_pixels,
                                    "\n\t width_in_millimeters=" , sc.width_in_millimeters,
                                    "\n\t height_in_millimeters=", sc.height_in_millimeters,
                                    "\n\t min_installed_maps="   , sc.min_installed_maps,
                                    "\n\t max_installed_maps="   , sc.max_installed_maps,
                                    "\n\t root_visual="          , utf::to_hex(sc.root_visual),
                                    "\n\t backing_stores="       , (si32)sc.backing_stores,
                                    "\n\t save_unders="          , (si32)sc.save_unders,
                                    "\n\t root_depth="           , (si32)sc.root_depth,
                                    "\n\t number_of_depths="     , (si32)sc.number_of_depths,
                                    "\n");
                str += root.list_of_depths.size() ? utf::fprint("\t   depths(%%):\n", root.list_of_depths.size()) : "        no depths\n";
                for (auto& depth : root.list_of_depths)
                {
                    auto& d = depth.s;
                    str += utf::fprint("\t\t depth=%% num_of_visual_types=%%\n", (si32)d.depth, d.num_of_visual_types);
                    //str += depth.list_of_visual_types.size() ? utf::fprint("          visual_types(%%):\n", depth.list_of_visual_types.size()) : "          no visual_types\n";
                    //for (auto& vt : depth.list_of_visual_types)
                    //{
                    //    auto& v = vt.s;
                    //    str += utf::fprint("\tvisual_id=",              utf::to_hex(v.visual_id),
                    //                        "\n\t\t visual_class=",       (si32)v.visual_class,
                    //                        "\n\t\t bits_per_rgb_value=", (si32)v.bits_per_rgb_value,
                    //                        "\n\t\t colormap_entries=",   v.colormap_entries,
                    //                        "\n\t\t red_mask=",           utf::to_hex(v.red_mask),
                    //                        "\n\t\t green_mask=",         utf::to_hex(v.green_mask),
                    //                        "\n\t\t blue_mask=",          utf::to_hex(v.blue_mask),
                    //                        "\n");
                    //}
                }
            }
            if (str.back() == '\n') str.pop_back();
            return str;
        }
        auto reset()
        {
            roots.clear();
        }
        constexpr explicit operator bool () const
        {
            return !roots.empty();
        }
    };
    #pragma pack(pop)

    auto read_ui16be(std::ifstream& fs)
    {
        auto uword = ui16{};
        auto bytes = text(2, '\0');
        if (fs.read(bytes.data(), bytes.size()))
        {
            uword = ((ui16)(byte)bytes[0] << 8) | (byte)bytes[1];
        }
        return uword;
    }
    auto read_string(std::ifstream& fs, ui16 length)
    {
        auto buffer = text(length, '\0');
        if (length > 0)
        {
            fs.read(buffer.data(), length);
        }
        return buffer;
    }
    auto get_cookie(view target_display_num)
    {
        struct x11cookie_t
        {
            text auth_name;
            text auth_data;
        };
        auto x11cookie = x11cookie_t{};
        auto auth_path = text{}; // Path to .Xauthority file.
        if (auto xauth_env = os::env::get("XAUTHORITY"); xauth_env.size())
        {
            auth_path = xauth_env;
        }
        else if (auto home_env = os::env::get("HOME"); home_env.size())
        {
            //todo expand home_env for win32
            auth_path = home_env + "/.Xauthority";
        }
        if (auth_path.size())
        if (auto fs = std::ifstream{ auth_path, std::ios::binary }; fs.is_open())
        {
            while (fs.peek() != EOF)
            {
[[maybe_unused]]auto family   = read_ui16be(fs); // family = 256 (FamilyLocal).
                auto addr_len = read_ui16be(fs);
[[maybe_unused]]auto addr_str = read_string(fs, addr_len);
                auto disp_len = read_ui16be(fs);
                auto disp_str = read_string(fs, disp_len);
                auto name_len = read_ui16be(fs);
                auto name_str = read_string(fs, name_len);
                auto data_len = read_ui16be(fs);
                auto data_str = read_string(fs, data_len);
                if (!fs) break; // Unexpected errors.
                if constexpr (debugmode) log("XAuth entry: family=%%, disp='%%', proto='%%', data_size=%%", family, disp_str, name_str, data_len);
                if (name_str == "MIT-MAGIC-COOKIE-1" && (disp_str == target_display_num || disp_str.empty()))
                {
                    if constexpr (debugmode) log("Cookie found. Name: %%, Data size: %%", name_str, data_len);
                    x11cookie.auth_name = std::move(name_str);
                    x11cookie.auth_data = std::move(data_str);
                    break;
                }
            }
        }
        return x11cookie;
    }
    auto build_connect_packet(auto& cookie_data)
    {
        #pragma pack(push, 1)
        struct x11_connect_request
        {
            byte byte_order;       // 0x6c ('l') or 0x42 ('B')
            byte pad1;             //
            ui16 major_version;    // X_PROTOCOL
            ui16 minor_version;    // X_PROTOCOL_REVISION
            ui16 auth_proto_len;   //
            ui16 auth_data_len;    //
            ui16 pad2;             //
        };
        #pragma pack(pop)
        auto header = x11_connect_request{};
        header.byte_order = netxs::endian_LE ? 'l' : 'B';
        header.major_version = 11;
        header.minor_version = 0;
        header.auth_proto_len = (ui16)cookie_data.auth_name.size();
        header.auth_data_len  = (ui16)cookie_data.auth_data.size();
        auto auth_name_padded_len = (header.auth_proto_len + 3) & ~3; // Rounding up to a multiple of 4.
        auto auth_data_padded_len = (header.auth_data_len  + 3) & ~3; //
        auto packet = text(sizeof(header) + auth_name_padded_len + auth_data_padded_len, '\0');
        std::memcpy(packet.data(), &header, sizeof(header));
        std::memcpy(packet.data() + sizeof(header), cookie_data.auth_name.data(), cookie_data.auth_name.size());
        std::memcpy(packet.data() + sizeof(header) + auth_name_padded_len, cookie_data.auth_data.data(), cookie_data.auth_data.size());
        return packet;
    }
    auto parse_connection_reply(auto x11connection)
    {
        auto header = x11::reply_header{};
        auto session = x11::session_t{};
        if (auto l1 = x11connection->recv((char*)&header, sizeof(header)); l1.size() == sizeof(header))
        {
            auto remaining_bytes = (size_t)header.additional_length * 4;
            auto buffer = text(remaining_bytes, '\0');
            if (header.status == 0) // Failed.
            {
                log("%%Connection rejected: '%%'", prompt::x11, utf::debase<faux, faux>(x11connection->recv(buffer.data(), buffer.size())));
            }
            else if (header.status != 1)
            {
                log("%%Unknown response status", prompt::x11);
            }
            else if (auto l3 = x11connection->recv(buffer.data(), buffer.size()); l3.size() != buffer.size())
            {
                log("%%Error reading response payload", prompt::x11);
            }
            else
            {
                auto q = qiew{ buffer };
                auto failed = faux;
                auto load = [&](auto& object)
                {
                    auto ptr = object.data();
                    auto len = object.size();
                    auto len_padded = (size_t)((len + 3) & ~3);
                    if (!failed && q.size() >= len_padded)
                    {
                        std::memcpy(ptr, q.data(), len);
                        q.remove_prefix(len_padded);
                    }
                    else failed = true;
                };
                load(session);
                session.vendor_str.resize(session.s.vendor_length);
                load(session.vendor_str);
                session.pixmap_formats.resize(session.s.number_of_formats);
                for (auto& pixmap_format : session.pixmap_formats)
                {
                    load(pixmap_format);
                }
                session.roots.resize(session.s.number_of_screens);
                for (auto& screen : session.roots)
                {
                    load(screen);
                    screen.list_of_depths.resize(screen.s.number_of_depths);
                    for (auto& depth : screen.list_of_depths)
                    {
                        load(depth);
                        depth.list_of_visual_types.resize(depth.s.num_of_visual_types);
                        for (auto& visual_type : depth.list_of_visual_types)
                        {
                            load(visual_type);
                        }
                    }
                }
                if (failed) session.reset();
            }
        }
        else
        {
            log("%%Error reading response header", prompt::x11);
        }
        return session;
    }
    static auto session = sptr<session_t>{}; // x11: Active X11 session.
    auto connect()
    {
        if (auto display_env = os::env::get("DISPLAY"); display_env.size())
        if (auto colon_start = display_env.find(':'); colon_start != text::npos)
        if (auto display_num = utf::to_int(display_env.substr(colon_start + 1)))
        if (auto x11unixpath = utf::concat("/tmp/.X11-unix/X", display_num.value()); os::fs::exists(x11unixpath))
        if (auto socket_link = os::ipc::socket::connect(x11unixpath))
        {
            auto display_str = std::to_string(display_num.value());
            auto cookie_data = x11::get_cookie(display_str);
            auto init_packet = x11::build_connect_packet(cookie_data);
            socket_link->send(init_packet);
            if (auto session = x11::parse_connection_reply(socket_link))
            {
                session.x11connection = socket_link;
                x11::session = ptr::shared(std::move(session));
                return true;
            }
        }
        return faux;
    }
}