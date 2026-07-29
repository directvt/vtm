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
        byte status;            // 0: Failed, 1: Success.
        byte pad1;
        ui16 major_version;
        ui16 minor_version;
        ui16 additional_length; // Payload length in 4-byte chunks.
        // payload ...
    };
    namespace req
    {
        struct create_window // Opcode 1 (create window). This request generates a CreateNotify event.
        {
            byte opcode = 1;
            byte depth = 32;          // Color depth.
            ui16 length = 8;          // 8+n Packet size in 4-byte words.
            ui32 window_id;           // New Window ID.
            ui32 parent_id;           // root_window_id.
            si16 x;                   // Initial coords in px.
            si16 y;                   //
            ui16 width;               // Initial size in px (not including the border).
            ui16 height;              //
            ui16 border_width = 0;
            ui16 window_class = 1;    // 0: CopyFromParent, 1: InputOutput, 2: InputOnly (input events and painting).
            ui32 visual_id = 0;       // 0: CopyFromParent.
            ui32 value_mask = 0x00002800; // event-mask | colormap.
            // payload ...  0x0001u  background-pixmap      4 0: none, 1: ParentRelative or PIXMAP
            //              0x0002u  background-pixel       4 argb
            //              0x0004u  border-pixmap          4 0: CopyFromParent or PIXMAP
            //              0x0008u  border-pixel           4 argb
            //              0x0010u  bit-gravity            1 BITGRAVITY (Forget) defines which region of the window should be retained if the window is resized
            //              0x0020u  win-gravity            1 WINGRAVITY (NorthWest) defines how the window should be repositioned if the parent is resized
            //              0x0040u  backing-store          1 0: NotUseful, 1: WhenMapped, 2: Always
            //              0x0080u  backing-planes         4 (default: all ones)
            //              0x0100u  backing-pixel          4 argb (0x00000000)
            //              0x0200u  override-redirect      1 bool  specifies whether map and configure requests on this window should override a SubstructureRedirect on the parent, typically to inform a window manager not to tamper with the window
            //              0x0400u  save-under             1 bool  If true, the server is advised that when this window is mapped, saving the contents of windows it obscures would be beneficial
            //              0x0800u  event-mask             4 SETofEVENT
            //              0x1000u  do-not-propagate-mask  4 SETofDEVICEEVENT
            //              0x2000u  colormap               4 0: CopyFromParent or COLORMAP
            //              0x4000u  cursor                 4 0: None or CURSOR
        };
        struct destroy_window // Opcode 4 (destroy window).
        {
            byte opcode = 4;
            byte pad    = 0;
            ui16 length = 2;
            ui32 window_id;
        };
        struct map_window // Opcode 8 (show window).
        {
            byte opcode = 8;
            byte pad    = 0;
            ui16 length = 2;
            ui32 window_id;
        };
        struct unmap_window // Opcode 10 (hide window).
        {
            byte opcode = 10;
            byte pad    = 0;
            ui16 length = 2;
            ui32 window_id;
        };
        struct configure_window // Opcode 12 (configure window).
        {
            byte opcode = 10;
            byte pad1   = 0;
            ui16 length;
            ui32 window_id;
            ui16 value_mask; // value bit list
            ui16 pad2   = 0;
            // payload ... value-list
            //             2 si16 x
            //             2 si16 y
            //             2 ui16 width
            //             2 ui16 height
            //             2 ui16 border-width
            //             4 ui32 sibling (window id)
            //             1 byte stack-mode
            //                    0 Above
            //                    1 Below
            //                    2 TopIf
            //                    3 BottomIf
            //                    4 Opposite
        };
        struct create_gc // Opcode 55 (create graphical context).
        {
            byte opcode = 55;
            byte pad    = 0;;
            ui16 length = 4;     // 16 bytes / 4 = 4 words
            ui32 gc_id;          // Generating id
            ui32 drawable;       // Our window ID (or root window)
            ui32 value_mask = 0; // No additional attributes
        };
        struct change_property // Opcode 18 (change property).
        {
            byte opcode    = 18;
            byte mode      = 0;  // 0: Replace
            ui16 length;         // Request length
            ui32 window_id;
            ui32 property  = 39; // Atom WM_NAME (predefined id = 39)
            ui32 type      = 31; // Atom STRING (predefined id = 31)
            byte format    = 8;  // 8-bit chars (string)
            byte pad[3]    = {};
            ui32 data_len;       // Char count
        };
        struct query_extension // Opcode 98 (query extension, query MIT-SHM).
        {
            struct reply
            {
                byte status; // 1
                byte pad;
                ui16 sequence;
                ui32 length;
                byte present; // 1 if supported.
                byte major_opcode; // opcode for extension.
                byte first_event;
                byte first_error;
                byte pad2[20];
            };
            byte opcode = 98;
            byte pad1   = 0;
            ui16 length;
            ui16 name_len;
            ui16 pad2   = 0;
            // payload..., e.g. "MIT-SHM"
        };
        struct shm_query_version // ShmQueryVersion (Minor Opcode 0)
        {
            struct reply // Always 32 bytes.
            {
                byte status;         // 1: Success.
                byte pad1;
                ui16 sequence;       // X11 request sequence number.
                ui32 length;         // Attached payload length (0)
                ui16 major_version;  // (required 1)
                ui16 minor_version;  // (required >= 2)
                ui16 uid;
                ui16 gid;
                byte pixmap_format;
                byte pad2[15];
            };
            byte major_opcode;   // MIT-SHM major_opcode из query_extension
            byte minor_opcode = 0; // X_ShmQueryVersion
            ui16 length = 1;       // 4 bytes / 4 = 1 word
        };
        struct shm_attach // ShmAttach (Minor Opcode 1) - bind SHM segment with x-serveer.
        {
            byte req_opcode;   // MIT-SHM major_opcode from query_extension.
            byte minor_opcode = 1; // X_ShmAttach.
            ui16 length = 4;   // 16 bytes / 4 = 4 words.
            ui32 shm_seg_id;   // Our side generated unique resource ID (XID).
            ui32 shmid;        // Linux kernel's segment id (from shmget).
            byte read_only = 0;
            byte pad[3] = {};
        };
        struct shm_attach_fd // ShmAttachFd (Minor Opcode 6) - bind SHM descriptor with x-serveer.
        {
            byte major_opcode;     // MIT-SHM major_opcode from query_extension.
            byte minor_opcode = 6; // X_ShmAttachFd.
            ui16 length = 4;
            ui32 shm_seg_id;   // Our side generated unique resource ID (XID).
            byte read_only = 0;
            byte pad[3] = {};
        };
        struct shm_detach // ShmDetach (Minor Opcode 2)
        {
            byte major_opcode;     // MIT-SHM major_opcode.
            byte minor_opcode = 2; // X_ShmDetach.
            ui16 length = 2;
            ui32 shm_seg_id;       // Detached segment ID (XID).
        };
        struct shm_put_image // ShmPutImage (Minor Opcode 3) - immediately output from SHM to screen.
        {
            byte req_opcode;   // Dynamic major_opcode.
            byte minor_opcode = 3; // X_ShmPutImage.
            ui16 length = 10;  // 40 bytes / 4 = 10 words.
            ui32 drawable;     // Our window ID (fg_w).
            ui32 gc_id;        // Graphical context.
            ui16 total_width;  // Buffer width/height in SHM
            ui16 total_height; //
            ui16 src_x = 0;    // Clip coor.
            ui16 src_y = 0;    //
            ui16 src_width;    // Clip size.
            ui16 src_height;   //
            si16 dst_x = 0;    // Dest coor.
            si16 dst_y = 0;    //
            byte depth = 32;   // 32-bit ARGB
            byte format = 2;   // 2: ZPixmap
            byte send_event = 0; // Don't notify on output end.
            byte pad = 0;
            ui32 shm_seg_id;   // Linked segment ID.
            ui32 offset = 0;   // Segment offset.
        };
        struct noop // Opcode 127 (NoOperation).
        {
            byte opcode = 127;
            byte pad    = 0;
            ui16 length = 1; // + n for payload
            // arbitrary payload ...
        };
    }
    namespace event
    {
        struct any
        {
            byte type; // Bit 7 may be set if the event is artificially generated (SendEvent).
            byte detail;
            ui16 sequence;
            ui32 pad[7];
        };
        struct mouse_click // Type 4 (button press) and 5 (button release).
        {
            byte type;
            byte button; // 1: Left, 2: Middle, 3: Right, 4: WheelUp, 5: WheelDown, 6: WheelLeft, 7: WheelRight
            ui16 sequence;
            ui32 time;
            ui32 root_window;
            ui32 event_window;
            ui32 child_window;
            si16 root_x;
            si16 root_y;
            si16 event_x; // Relative X
            si16 event_y; // Relative Y
            ui16 state;   // Keybd modifiers (Shift, Ctrl, Alt...)
            byte same_screen;
            byte pad;
        };
        struct motion // Type 6 (motion notify).
        {
            byte type;
            byte detail; // 0: Normal, 1: Hint
            ui16 sequence;
            ui32 time;
            ui32 root_window;
            ui32 event_window;
            ui32 child_window;
            si16 root_x;
            si16 root_y;
            si16 event_x; // Relative (to window) mouse Х.
            si16 event_y; // Relative (to window) mouse Y.
            ui16 state;   // Mouse buttons bitfield + keybd mods (Ctrl, Shift, etc.)
            byte same_screen;
            byte pad;
        };
        struct crossing // Type 7 (enter notify) and 8 (leave notify), a-la TrackMouseEvent/WM_MOUSELEAVE.
        {
            byte type;
            byte detail;
            ui16 sequence;
            ui32 time;
            ui32 root_window;
            ui32 event_window;
            ui32 child_window;
            si16 root_x;
            si16 root_y;
            si16 event_x;
            si16 event_y;
            ui16 state;
            byte mode;
            byte flags; // Bits: same_screen, focus
        };
        struct focus // Type 9 (focus on) and 10 (focus off).
        {
            byte type;
            byte mode; // 0: Normal, 1: Grab, 2: Ungrab
            ui16 sequence;
            ui32 window;
            byte detail;
            byte pad[23];
        };
        struct configure // Type 22 (configure notify) a-la WM_SIZE/WM_MOVE.
        {
            byte type;
            byte pad;
            ui16 sequence;
            ui32 event_window;
            ui32 window;
            ui32 above_sibling;
            si16 x;
            si16 y;
            ui16 width;  // New width.
            ui16 height; // New heigth.
            ui16 border_width;
            byte override_redirect;
            byte pad2;
        };
        struct client_message // Type 33 (client message).
        {
            byte type;
            byte format; // ? 32 bits
            ui16 sequence;
            ui32 window;
            ui32 message_type; // Procotol atom (e.g., WM_PROTOCOLS)
            ui32 data32[5];    // Payload. If data32[0]==atom_WM_DELETE_WINDOW -> Close window
        };
    }
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
        struct shm_alloc_t
        {
            si32 shmid;
            ui32 shm_seg_id;
            void* addr;
        };
        text                                  vendor_str;     // buffer[32..32+vendor_length] = vendor_str
        std::vector<format>                   pixmap_formats; // format * number_of_formats = pixmap_formats
        si32                                  pixmap_format_index = -1;
        std::vector<screen>                   roots;          // screen * number_of_screens = roots (always a multiple of 4)
        byte                                  shm_major_opcode = 0;
        byte                                  shm_first_event = 0;
        fd_t                                  shm_buffer_fd = os::invalid_fd;
        byte*                                 shm_buffer_ptr = {};
        size_t                                shm_buffer_size = {};
        ui32                                  shm_segmen_xid = {};

        sptr<os::ipc::stdcon>                 x11connection;  // Active X11 socket connection.
        std::unordered_map<ui32, shm_alloc_t> active_shm_segments;

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
        auto new_resource_id()
        {
            static auto resource_index = std::atomic<ui32>{ 1 }; //todo use some generic::indexer
            auto current_idx = resource_index.fetch_add(1);
            auto resource_id = s.resource_id_base | (current_idx & s.resource_id_mask);
            return resource_id;
        }
        auto free_resource_id(ui32& resource_id)
        {
            //todo implement
            resource_id = {};
        }
        auto detect_argb_32bit()
        {
            auto i = 0;
            for (auto& format : pixmap_formats)
            {
                auto& pf = format.s;
                if (pf.depth == 32 && pf.bits_per_pixel == 32)
                {
                    pixmap_format_index = i;
                    return true;
                }
                i++;
            }
            auto errmsg = utf::fprint("%%32-bit ARGB pixel format not found on X11 server\n", prompt::x11);
            errmsg += pixmap_formats.size() ? utf::fprint("    Available pixmap formats(%%):\n", pixmap_formats.size()) : "    There are no pixmap formats\n";
            for (auto& format : pixmap_formats)
            {
                auto& pf = format.s;
                errmsg += utf::fprint("\tdepth=%% bpp=%% scanline_pad=%%\n", (si32)pf.depth, (si32)pf.bits_per_pixel, (si32)pf.scanline_pad);
            }
            log<faux>(errmsg);
            return faux;
        }
        auto detect_mit_shm()
        {
            auto ext_name = "MIT-SHM"s;
            auto errdetails = text{};
            auto req = x11::req::query_extension{};
            req.name_len = ext_name.size();
            auto padded_len = (ext_name.size() + 3) & ~3;
            req.length = (sizeof(req) + padded_len) / 4;
            auto packet = text(sizeof(req) + padded_len, '\0');
            std::memcpy(packet.data(), &req, sizeof(req));
            std::memcpy(packet.data() + sizeof(req), ext_name.data(), ext_name.size());
            x11connection->send(packet);
            auto reply = x11::req::query_extension::reply{};
            if (x11connection->recv((char*)&reply, sizeof(reply)).size() == sizeof(reply))
            if (reply.present)
            {
                shm_major_opcode = reply.major_opcode;
                shm_first_event  = reply.first_event;
                auto v_req = x11::req::shm_query_version{ .major_opcode = shm_major_opcode };
                x11connection->send(view{ (char*)&v_req, sizeof(v_req) });
                auto v_reply = x11::req::shm_query_version::reply{};
                if (x11connection->recv((char*)&v_reply, sizeof(v_reply)).size() == sizeof(v_reply))
                if (v_reply.status == 1)
                if (v_reply.major_version > 1 || (v_reply.major_version == 1 && v_reply.minor_version >= 2)) // Check min version 1.2.
                {
                    if constexpr (debugmode) log("%%MIT-SHM version %%.%% detected", prompt::x11, (si32)v_reply.major_version, (si32)v_reply.minor_version);
                    return true;
                }
                if (v_reply.status == 1)
                {
                    errdetails = utf::fprint("\n\tMIT-SHM version %%.%% detected", (si32)v_reply.major_version, (si32)v_reply.minor_version);
                }
                else
                {
                    errdetails = utf::fprint("\n\tFailed to receive MIT-SHM version details");
                }
            }
            auto errmsg = utf::fprint("%%The required MIT-SHM extension (or required min version 1.2) is missing", prompt::x11);
            log(errmsg + errdetails);
            return faux;
        }
        void send_shm_attach_fd(ui32 client_shmseg_xid)
        {
            auto request = x11::req::shm_attach_fd{};
            request.major_opcode = shm_major_opcode;
            request.shm_seg_id   = client_shmseg_xid;
            // Fill iovec.
            auto iov = ::iovec{ .iov_base = &request,
                                .iov_len  = sizeof(request) };
            union // Ancillary Data
            {
                ::cmsghdr cm;
                char control[CMSG_SPACE(sizeof(int))];
            }
            control_buffer{};
            auto msg = ::msghdr{ .msg_name       = nullptr,
                                 .msg_namelen    = 0,
                                 .msg_iov        = &iov,
                                 .msg_iovlen     = 1,
                                 .msg_control    = control_buffer.control,
                                 .msg_controllen = sizeof(control_buffer.control) };
            auto cmsg = CMSG_FIRSTHDR(&msg);
            cmsg->cmsg_len   = CMSG_LEN(sizeof(int));
            cmsg->cmsg_level = SOL_SOCKET;
            cmsg->cmsg_type  = SCM_RIGHTS;
            *reinterpret_cast<fd_t*>(CMSG_DATA(cmsg)) = shm_buffer_fd;
            auto x11_socket_fd = x11connection->handle.w;
            auto bytes_sent = ::sendmsg(x11_socket_fd, &msg, 0);
            if (bytes_sent == -1)
            {
                log("%%sendmsg failed during shm_attach_fd invocation", prompt::x11);
            }
        }
        void send_shm_detach_fd(ui32 client_shmseg_xid)
        {
            auto req = x11::req::shm_detach{ .major_opcode = shm_major_opcode,
                                             .shm_seg_id   = client_shmseg_xid };
            x11connection->send(view{ (char*)&req, sizeof(req) });
            if constexpr (debugmode) log("%%Shared buffer segment XID %% is detached", prompt::x11, client_shmseg_xid);
        }
        void sync_pixmap(ui32 window_id, ui32 gc_id, twod size)
        {
            auto it = active_shm_segments.find(window_id);
            if (it != active_shm_segments.end())
            {
                auto& shm = it->second;
                auto req = x11::req::shm_put_image{};
                req.req_opcode   = shm_major_opcode;
                req.drawable     = window_id;
                req.gc_id        = gc_id;
                req.total_width  = size.x;
                req.total_height = size.y;
                req.src_width    = size.x;
                req.src_height   = size.y;
                req.shm_seg_id   = shm.shm_seg_id;
                auto packet = text(sizeof(req), '\0');
                std::memcpy(packet.data(), &req, sizeof(req));
                x11connection->send(packet);
            }
        }
        auto create_window_flow(ui32 new_window_id, twod coor, twod size)
        {
            auto& screen = roots.front();
            auto req = x11::req::create_window{};
            req.window_id = new_window_id;
            req.parent_id = screen.s.root_window_id;
            req.depth     = 32;//screen.s.root_depth;
            req.visual_id = screen.s.root_visual;
            req.x         = (ui16)coor.x;
            req.y         = (ui16)coor.y;
            req.width     = (ui16)size.x;
            req.height    = (ui16)size.y;
            auto event_mask = 0x0012804Fu; // Bitfield (Focus, Resize, Paint, Mouse, Keyboard).
            auto colormap = screen.s.default_colormap;
            // value_mask specifies arguments are stored in the payload block in bit order: CWColormap (0x00002000), CWEventMask (0x00000800).
            auto create_packet = text(sizeof(req) + sizeof(colormap) + sizeof(event_mask), '\0');
            std::memcpy(create_packet.data(), &req, sizeof(req));
            std::memcpy(create_packet.data() + sizeof(req), &colormap, sizeof(colormap));
            std::memcpy(create_packet.data() + sizeof(req) + sizeof(colormap), &event_mask, sizeof(event_mask));
            auto map = x11::req::map_window{ .window_id = new_window_id }; // Immediately append the map_window request.
            auto map_packet = text(sizeof(map), '\0');
            std::memcpy(map_packet.data(), &map, sizeof(map));
            return create_packet + map_packet;
        }
        void window_set_title(ui32 window_id, view title)
        {
            auto req = x11::req::change_property{};
            req.window_id = window_id;
            req.data_len  = title.size();
            auto padded_str_len = (title.size() + 3) & ~3;
            req.length = (sizeof(req) + padded_str_len) / 4;
            auto packet = text(sizeof(req) + padded_str_len, '\0');
            std::memcpy(packet.data(), &req, sizeof(req));
            std::memcpy(packet.data() + sizeof(req), title.data(), title.size());
            x11connection->send(packet);
        }
        bool resize_shared_buffer(size_t size)
        {
            if (shm_buffer_size)
            {
                //todo implement delayed detach
                send_shm_detach_fd(shm_segmen_xid);
                reset_shared_buffer();
                free_resource_id(shm_segmen_xid);
            }
            shm_buffer_fd =
            #if defined(__linux__)
                ::memfd_create("x11_shm_double_buf", MFD_CLOEXEC);
            #else
                ::shm_open(SHM_ANON, O_RDWR | O_CREAT | O_EXCL, 0600); // SHM_ANON - native anonymous descriptor in BSD.
            #endif
            if (shm_buffer_fd == os::invalid_fd)
            {
                log("%%Failed to create anonymous shared memory fd", prompt::gui);
            }
            else
            {
                if (::ftruncate(shm_buffer_fd, size) == -1) // Set shm size.
                {
                    ::close(shm_buffer_fd);
                    shm_buffer_fd = os::invalid_fd;
                    log("%%Failed to truncate shared memory file to required size", prompt::gui);
                }
                else
                {
                    auto mapped_ptr = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_buffer_fd, 0);
                    if (mapped_ptr == MAP_FAILED)
                    {
                        ::close(shm_buffer_fd);
                        shm_buffer_fd = os::invalid_fd;
                        log("%%Failed to mmap shared memory descriptor", prompt::gui);
                    }
                    else
                    {
                        shm_buffer_ptr = (byte*)mapped_ptr;
                        shm_buffer_size = size;
                        shm_segmen_xid = new_resource_id();
                        send_shm_attach_fd(shm_segmen_xid);
                        if constexpr (debugmode) log("%%Shared buffer successfuly created at 0x%%, %% bytes", prompt::x11, utf::to_hex(shm_buffer_ptr), shm_buffer_size);
                    }
                }
            }
            return shm_buffer_size > 0;
        }
        void reset_shared_buffer()
        {
            if (shm_buffer_ptr && shm_buffer_ptr != MAP_FAILED)
            {
                ::munmap(shm_buffer_ptr, shm_buffer_size);
                shm_buffer_size = {};
                shm_buffer_ptr = {};
            }
            if (shm_buffer_fd != os::invalid_fd)
            {
                ::close(shm_buffer_fd);
                shm_buffer_fd = os::invalid_fd;
            }
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
                if (session.detect_argb_32bit())
                if (session.detect_mit_shm())
                {
                    if constexpr (debugmode) log(session.str());
                    auto& x11screen = session.roots.front().s;
                    auto max_grid_size = x11screen.width_in_pixels * x11screen.height_in_pixels;
                    auto required_buffer_size = 2 * 3 * max_grid_size * sizeof(argb); // 2: Double buffer, 3: master+blinks+header/footer/tooltip.
                    if (session.resize_shared_buffer(required_buffer_size))
                    {
                        x11::session = ptr::shared(std::move(session));
                        return true;
                    }
                }
            }
        }
        return faux;
    }
}