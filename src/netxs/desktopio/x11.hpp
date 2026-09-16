// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

namespace netxs::x11
{
    static constexpr auto recv_packet_size = 32;

    using fd_t = os::fd_t;

    auto serialize_list(text& yield, auto& packet, auto const& payload)
    {
        using field_t = std::optional<ui32>;
        auto vmask = 0u;
        auto count = sizeof(payload) / sizeof(field_t);
        auto ptr = (char const*)&payload;
        for (auto i = 0u; i < count; ++i)
        {
            auto field = netxs::start_lifetime_as<field_t>(ptr);
            if (field.has_value())
            {
                vmask |= 1u << i;
            }
            ptr += sizeof(field_t);
        }
        packet.value_mask = vmask;
        packet.length     = sizeof(packet) / 4 + std::popcount(vmask);
        yield += view{ (char*)&packet, sizeof(packet) };
        if (vmask)
        {
            ptr = (char const*)&payload;
            for (auto i = 0u; i < count; ++i)
            {
                auto field = netxs::start_lifetime_as<field_t>(ptr);
                if (field.has_value())
                {
                    auto v = field.value();
                    yield += view{ (char*)&v, sizeof(v) };
                }
                ptr += sizeof(field_t);
            }
        }
    }
    auto serialize_str(text& yield, auto& packet, auto const& data)
    {
        static constexpr auto is_string = requires{ data.substr(0); };
        auto data_bytes = 0ul;
        if constexpr (is_string) // String.
        {
            data_bytes = data.size();
            packet.data_len = data.size();
        }
        else // POD.
        {
            data_bytes = sizeof(data);
            packet.data_len = (ui32)(sizeof(data) * 8 / packet.format);
        }
        packet.length = (ui16)(sizeof(packet) / 4 + (data_bytes + 3) / 4);
        yield += view{ (char*)&packet, sizeof(packet) };
        if constexpr (is_string)
        {
            yield += view{ data.data(), data_bytes };
            yield.append(-data_bytes & 3, '\0');
        }
        else
        {
            yield += view{ (char*)&data, data_bytes };
        }
    }

    #pragma pack(push, 1)
    namespace icccm
    {
        //todo it doesn't work for input/non-input
        static constexpr auto InputHint = 1 << 0;
        struct wm_hints
        {
            ui32 flags         = InputHint;
            ui32 input         = 1;  // 1: Available for input focus.
            ui32 initial_state = 1;  // 1: NormalState.
            ui32 icon_pixmap   = 0;
            ui32 icon_window   = 0;
            ui32 icon_x        = 0;
            ui32 icon_y        = 0;
            ui32 icon_mask     = 0;
            ui32 window_group  = 0;
        };
        namespace win_gravity
        {
            static constexpr auto NorthWest = 1;
            static constexpr auto North     = 2;
            static constexpr auto NorthEast = 3;
            static constexpr auto West      = 4;
            static constexpr auto Center    = 5;
            static constexpr auto East      = 6;
            static constexpr auto SouthWest = 7;
            static constexpr auto South     = 8;
            static constexpr auto SouthEast = 9;
        }
        struct wm_size_hints
        {
            static constexpr auto USPosition  = 1u << 0; // User specified coor.
            static constexpr auto USSize      = 1u << 1; // User specified size.
            static constexpr auto PPosition   = 1u << 2; // Program specified coor.
            static constexpr auto PSize       = 1u << 3; // Program specified size.
            static constexpr auto PMinSize    = 1u << 4; // Program specified minimum size.
            static constexpr auto PMaxSize    = 1u << 5; // Program specified maximum size.
            static constexpr auto PResizeInc  = 1u << 6; // Program specified resize increments.
            static constexpr auto PAspect     = 1u << 7; // Program specified min/max aspect ratios.
            static constexpr auto PBaseSize   = 1u << 8; // Base size.
            static constexpr auto PWinGravity = 1u << 9; // Window gravity.

            ui32 flags;                          // Hint set.
            si32 x, y;                           // Coor (unused).
            si32 width, height;                  // Size (unused).
            si32 min_width, min_height;          // Min size.
            si32 max_width, max_height;          // Max size.
            si32 width_inc, height_inc;          // Resize step.
            si32 min_aspect_num, min_aspect_den; // Min aspect ratio.
            si32 max_aspect_num, max_aspect_den; // Max aspect ratio.
            si32 base_width, base_height;        // Initial size (default: Min size).
            si32 win_gravity;                    // Window gravity (default: NorthWest).
        };
    }
    namespace motif
    {
        static constexpr auto Decorations = 1 << 1;
        struct hints // Motif WM Hints to disable decorations.
        {
            ui32 flags       = 2; // MWM_HINTS_DECORATIONS
            ui32 functions   = 0;
            ui32 decorations = 0; // 0: No decors.
            ui32 input_mode  = 0;
            ui32 status      = 0;
        };
    }
    namespace req
    {
        struct create_window // Opcode 1 (create window). This request generates a CreateNotify event.
        {
            static constexpr auto CopyFromParent = 0;
            static constexpr auto InputOutput    = 1;
            static constexpr auto InputOnly      = 2;

            //static constexpr auto BitGravityForget    = 0;
            //static constexpr auto BitGravityNorthWest = 1;
            //static constexpr auto BitGravityNorth     = 2;
            //static constexpr auto BitGravityNorthEast = 3;
            //static constexpr auto BitGravityWest      = 4;
            //static constexpr auto BitGravityCenter    = 5;
            //static constexpr auto BitGravityEast      = 6;
            //static constexpr auto BitGravitySouthWest = 7;
            //static constexpr auto BitGravitySouth     = 8;
            //static constexpr auto BitGravitySouthEast = 9;
            //static constexpr auto BitGravityStatic    = 10;

            byte opcode = 1;
            byte depth = 0;           // Color depth. Must be zero for InputOnly window.
            ui16 length = 8;          // Packet size in 4-byte words.
            ui32 window_id;           // New Window ID.
            ui32 parent_id;           // root_window_id.
            si16 x = 0;               // Initial coords in px.
            si16 y = 0;               //
            ui16 width = 1;           // Initial size in px (not including the border).
            ui16 height = 1;          //
            ui16 border_width = 0;
            ui16 window_class = InputOutput; // 0: CopyFromParent, 1: InputOutput (input events and painting), 2: InputOnly (input events).
            ui32 visual_id = 0;       // ID: 0: CopyFromParent.
            ui32 value_mask;          // Payload bits.

            struct payload
            {
                static constexpr auto NotUseful  = 0;
                static constexpr auto WhenMapped = 1;
                static constexpr auto Always     = 2;

                std::optional<ui32> background_pixmap;     // 4 0: none, 1: ParentRelative or PIXMAP
                std::optional<ui32> background_pixel;      // 4 argb
                std::optional<ui32> border_pixmap;         // 4 0: CopyFromParent or PIXMAP
                std::optional<ui32> border_pixel;          // 4 argb
                std::optional<ui32> bit_gravity;           // 1 BITGRAVITY (Forget) defines which region of the window should be retained if the window is resized
                std::optional<ui32> win_gravity;           // 1 WINGRAVITY (NorthWest) defines how the window should be repositioned if the parent is resized
                std::optional<ui32> backing_store;         // 1 0: NotUseful, 1: WhenMapped, 2: Always
                std::optional<ui32> backing_planes;        // 4 (default: all ones)
                std::optional<ui32> backing_pixel;         // 4 argb (0x00000000)
                std::optional<ui32> override_redirect;     // 1 bool  specifies whether map and configure requests on this window should override a SubstructureRedirect on the parent, typically to inform a window manager not to tamper with the window
                std::optional<ui32> save_under;            // 1 bool  If true, the server is advised that when this window is mapped, saving the contents of windows it obscures would be beneficial
                std::optional<ui32> event_mask;            // 4 SETofEVENT
                std::optional<ui32> do_not_propagate_mask; // 4 SETofDEVICEEVENT
                std::optional<ui32> colormap_id;           // 4 0: CopyFromParent or COLORMAP
                std::optional<ui32> cursor_id;             // 4 0: None or CURSOR
            };
            auto serialize(text& yield, payload p) { x11::serialize_list(yield, *this, p); }
        };
        struct change_window_attrs // Opcode 2 (change window attributes).
        {
            byte opcode = 2;
            byte pad    = 0;
            ui16 length;
            ui32 window_id;
            ui32 value_mask; // Bitfield specifies attr list.

            using payload = create_window::payload;
            auto serialize(text& yield, payload p) { x11::serialize_list(yield, *this, p); }
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
            static constexpr auto Above    = 0;
            static constexpr auto Below    = 1;
            static constexpr auto TopIf    = 2;
            static constexpr auto BottomIf = 3;
            static constexpr auto Opposite = 4;

            byte opcode = 12;
            byte pad1   = 0;
            ui16 length;
            ui32 window_id;
            ui16 value_mask; // value bit list
            ui16 pad2   = 0;

            struct payload
            {
                std::optional<ui32> x;
                std::optional<ui32> y;
                std::optional<ui32> width;
                std::optional<ui32> height;
                std::optional<ui32> border_width;
                std::optional<ui32> sibling;
                std::optional<ui32> stack_mode; // 0: Above, 1: Below, 2: TopIf, 3: BottomIf, 4: Opposite.
            };
            auto serialize(text& yield, payload p) { x11::serialize_list(yield, *this, p); }
        };
        struct intern_atom // Opcode 16 (intern atom).
        {
            struct reply
            {
                byte type;    // Always 1 (Reply).
                byte pad0;
                ui16 sequence;
                ui32 length;  // Always 0.
                ui32 atom_id; // Requested Atom ID.
                ui32 pad2[5];
            };
            byte opcode = 16;
            byte only_if_exists; // 0: Create if absent, 1: Only if exists.
            ui16 length;         // (sizeof(intern_atom) + name_len + padding) / 4
            ui16 data_len;       // Name string length in bytes.
            ui16 pad = 0;

            auto serialize(text& yield, view name) { x11::serialize_str(yield, *this, name); }
        };
        struct get_atom_name // Opcode 17 (get atom name).
        {
            struct reply
            {
                byte type;             // x11::event::Reply (1)
                byte pad0;
                ui16 sequence;
                ui32 length;
                ui16 name_len;         // Atom name length.
                ui16 pad1[9];
                // Payload: "atom_name".
            };
            byte opcode = 17; // 17: GetAtomName
            byte pad0   = 0;
            ui16 length = 2;
            ui32 atom;
        };
        struct change_property // Opcode 18 (change property).
        {
            byte opcode    = 18;
            byte mode      = 0;  // 0: Replace
            ui16 length;
            ui32 window_id;
            ui32 property;       // Atom (e.g., WM_NAME)
            ui32 type;           // Atom (e.g., STRING)
            byte format    = 32; // Payload unit format (e.g., 8: 8-bit chars (string), 32: 32-bit words)
            byte pad[3]    = {};
            ui32 data_len;       // Payload size in format units.

            auto serialize(text& yield, auto const& data) { x11::serialize_str(yield, *this, data); }
        };
        struct get_property // Opcode 20 (get property).
        {
            struct reply
            {
                byte type;        // Always 1 (Reply).
                byte format;      // Bits in word.
                ui16 sequence;
                ui32 length;      // Payload length in quads.
                ui32 prop_type;   // Property type.
                ui32 bytes_after; // Bytes remains.
                ui32 value_len;   // Item size (array step).
                ui32 pad[3];
                // payload...
            };
            byte opcode = 20;
            byte remove = 0;      // 1: Delete property after read, 0: Do nothing.
            ui16 length = 6;
            ui32 window_id;
            ui32 property;        // Atom for property.
            ui32 prop_type;       // Atom for property type, 0: for any. (e.g., XA_WINDOW=33)
            ui32 long_offset = 0; // Requested payload offset from beginning in quads.
            ui32 long_length = 1; // Requested payload length in quads.
        };
        struct send_event // Opcode 25 (send event).
        {
            struct reply // client_message
            {
                byte type;          // 33: ClientMessage. (or 35: GenericEvent?)
                byte format;        // Data word format: 8, 16, 32 bits.
                ui16 sequence;
                // Payload header:
                ui32 originator_id; // Originator window id.
                ui32 message_type;  // Atom message id (a-la WIN32_WM_USER).
                ui32 serial;        // Serial number to sync replay.
                ui32 command;       //
                ui32 lParam;        //
                // User data start:
                ui32 data32[2];     //
            };
            byte opcode     = 25; // 25: SendEvent.
            byte propagate  = 0;  // Send to the window tree.
            ui16 length     = 11;
            ui32 destination_id;  // Destination window id.
            ui32 event_mask = 0;  // 0 for ClientMessage.
            byte type       = 33; // 33: ClientMessage.
            byte format     = 32; // Data format: 8, 16, 32 bits.
            ui16 sequence   = {};
            // Payload header:
            ui32 originator_id;   // Originator window id.
            ui32 message_type;    // Atom message id (a-la WIN32_WM_USER).
            ui32 serial      = 0; // Serial number to sync replay. =Protocols atom for WM_PROTOCOLS.
            ui32 command     = 0; // User data.
            ui32 lParam      = 0; // User data: =data_length if command==cmd_w_data.
            // User data start:
            ui32 data32[2] = {};  // User data.
            struct chunk // Subsequent chunk. Chunk count = data_length <= 4*2 ? 0 : (data_length-4*2 + 6*4-1) / 6*4.
            {
                byte type;      // 33: ClientMessage. (or 35: GenericEvent?)
                byte format;    // Data word format: 8, 16, 32 bits.
                ui16 sequence;
                // User data start:
                ui32 originator_id; // Originator id.
                ui32 data32[6];     // User data.
            };
        };
        //struct grab_server // Opcode 36 (grab server).
        //{
        //    byte opcode = 36;
        //    byte pad    = 0;
        //    ui16 length = 1;
        //};
        //struct ungrab_server // Opcode 37 (ungrab server)
        //{
        //    byte opcode = 37;
        //    byte pad    = 0;
        //    ui16 length = 1;
        //};
        struct query_pointer // Opcode 38 (query pointer).
        {
            struct reply
            {
                byte status;
                byte same_screen;
                ui16 sequence_number;
                ui32 length;
                ui32 root_window_id;
                ui32 child_window_id;
                si16 root_x;
                si16 root_y;
                si16 win_x;
                si16 win_y;
                ui16 mask;
                ui16 pad2;
            };
            byte opcode = 38; // 38: QueryPointer.
            byte pad    = 0;
            ui16 length = 2;
            ui32 window_id;
        };
        struct set_input_focus // Opcode 42 (set input focus).
        {
            static constexpr auto RevertToNone   = 0;
            static constexpr auto RevertToRoot   = 1;
            static constexpr auto RevertToParent = 2;

            byte opcode    = 42;
            byte revert_to = RevertToRoot;
            ui16 length    = 3;
            ui32 window_id;
            ui32 time      = 0; // 0: CurrentTime.
        };
        struct get_input_focus // Opcode 43 (get_input focus).
        {
            struct reply
            {
                byte type;
                byte revertTo; // Focus destination after focused window deletion: 0: ToNone, 1: ToRoot, 2: ToParent.
                ui16 sequence;
                ui32 length;
                ui32 focused_window_id;
                ui32 pad[5];
            };
            byte opcode = 43;
            byte pad    = 0;
            ui16 length = 1;
        };
        struct query_keymap // Opcode 44 (query pressed key state).
        {
            struct reply
            {
                byte type;
                byte pad0;
                ui16 sequence;
                ui32 length;   // =2.
                byte keys[32]; // Key bit field (256 bits).
            };
            byte opcode = 44;
            byte pad0   = 0;
            ui16 length = 1;
        };
        //struct create_pixmap // Opcode 53 (create pixmap).
        //{
        //    byte opcode = 53; // CreatePixmap.
        //    byte depth  = 32;
        //    ui16 length = 4;
        //    ui32 pixmap_id;   // Our side generated id.
        //    ui32 drawable_id; // root_window_id
        //    ui16 width;       // Pixmap size
        //    ui16 height;      //
        //};
        struct create_gc // Opcode 55 (create graphical context).
        {
            byte opcode = 55;
            byte pad    = 0;
            ui16 length = 4;     // 16 bytes / 4 = 4 words
            ui32 gc_id;          // Generating id
            ui32 drawable;       // Our window ID (or root window)
            ui32 value_mask = 0; // No additional attributes
        };
        struct poly_point // Opcode 64 (PolyPoint).
        {
            byte opcode = 64;
            byte coordinate_mode = 0; // 0: CoordModeOrigin (absolute coords).
            ui16 length = 4;
            ui32 drawable_id;
            ui32 gc_id;
            // Payload:
            si16 x = 0;
            si16 y = 0;
        };
        //struct get_image // Opcode 73 (get image).
        //{
        //    byte reqType = 73;     // X_GetImage.
        //    byte format  = 2;      // 1: XYPixmap, 2: ZPixmap.
        //    ui16 length  = 5;
        //    ui32 drawable_id;      // session.root_window_id
        //    si16 x       = 0;
        //    si16 y       = 0;
        //    ui16 width   = 1;
        //    ui16 height  = 1;
        //    ui32 plane_mask = 0xFFFFFFFF;
        //};
        struct create_colormap // Opcode 78 (create colormap).
        {
            byte opcode = 78;
            byte alloc  = 0;  // 0: None, 1: All
            ui16 length = 4;
            ui32 colormap_id; // Our side XID.
            ui32 window_id;   // Window ID (root_window_id).
            ui32 visual_id;   // Some argb Visual ID.
        };
        struct query_extension // Opcode 98 (query extension).
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
            ui16 data_len; // Extension name length in bytes.
            ui16 pad2   = 0;

            auto serialize(text& yield, view name) { x11::serialize_str(yield, *this, name); }
        };
        struct get_keyboard_mapping // Opcode 101 (get keybd keysym mapping)
        {
            struct reply
            {
                byte type;
                byte key_syms_per_key_code;
                ui16 sequence;
                ui32 length;
                ui32 pad[6];
                // Payload...
            };
            byte opcode = 101;
            byte pad0   = 0;
            ui16 length = 2;
            byte first_keycode;
            byte count;
            ui16 pad1   = 0;
        };
        //struct get_keyboard_control // Opcode 103 (get keybd control, LED state).
        //{
        //    static constexpr auto CapsLock = 1 << 0; // It is unspecified.
        //    static constexpr auto NumLock  = 1 << 1; //
        //    struct reply
        //    {
        //        byte type;
        //        byte global_auto_repeat; // 1 = on, 0 = off.
        //        ui16 sequence;
        //        ui32 length;
        //        ui32 led_mask;           // LED bit fileld.
        //        byte key_click_percent;
        //        byte bell_percent;
        //        ui16 bell_pitch;
        //        ui16 bell_duration;
        //        ui16 pad0;
        //        byte auto_repeats[32];
        //    };
        //    byte opcode = 103;
        //    byte pad0   = 0;
        //    ui16 length = 1;
        //    //ui32 mask   = 0;
        //};
        namespace shm // SHM Minor Opcodes: 0:QueryVersion, 1:Attach, 2:Detach, 3:PutImage, 4:GetImage, 5:CreatePixmap, 6:AttachFd, 7:CreateSegment
        {
            struct query_version // ShmQueryVersion (Minor Opcode 0)
            {
                struct reply // Always 32 bytes.
                {
                    byte status;         // 1: Success.
                    byte pad1;
                    ui16 sequence;       // X11 request sequence number.
                    ui32 length;         // Attached payload length (0)
                    ui16 server_major_version; // (required 1)
                    ui16 server_minor_version; // (required >= 2)
                    ui16 uid;
                    ui16 gid;
                    byte pixmap_format;
                    byte pad2[15];
                };
                byte major_opcode;     // MIT-SHM major_opcode.
                byte minor_opcode = 0; // 0: QueryVersion.
                ui16 length       = 1; // 4 bytes / 4 = 1 word.
            };
            struct attach_fd // ShmAttachFd (Minor Opcode 6) - bind SHM file descriptor with x-serveer (via ::sendmsg()).
            {
                byte major_opcode;     // MIT-SHM major_opcode.
                byte minor_opcode = 6; // 6: AttachFd.
                ui16 length       = 3;
                ui32 shm_seg_id;       // Our side generated unique resource ID (XID).
                byte read_only    = 0;
                byte pad[3]       = {};
            };
            struct detach // ShmDetach (Minor Opcode 2)
            {
                byte major_opcode;     // MIT-SHM major_opcode.
                byte minor_opcode = 2; // 2: Detach.
                ui16 length       = 2;
                ui32 shm_seg_id;       // Detached segment ID (XID).
            };
            struct put_image // ShmPutImage (Minor Opcode 3) - immediately output from SHM to screen.
            {
                struct reply // ShmCompletionEvent
                {
                    byte type;          // shm_completion_event
                    byte pad0;
                    ui16 sequence;
                    ui32 drawable;      // Dest Window ID
                    ui16 minor_opcode;  // 3 (ShmPutImage).
                    byte major_opcode;  // shm_major_opcode.
                    byte pad1;
                    ui32 shmseg;        // Linked segment ID.
                    ui32 offset;        // Segment offset.
                    ui32 pad2;
                    ui32 pad3;
                    ui32 pad4;
                };
                byte major_opcode;      // shm_major_opcode.
                byte minor_opcode = 3;  // 3: PutImage.
                ui16 length       = 10; // 40 bytes / 4 = 10 words.
                ui32 drawable;          // Our window ID (fg_w).
                ui32 gc_id;             // Graphical context.
                ui16 total_width;       // Buffer width/height in SHM
                ui16 total_height;      //
                ui16 src_x;             // Clip coor.
                ui16 src_y;             //
                ui16 src_width;         // Clip size.
                ui16 src_height;        //
                si16 dst_x;             // Dest coor.
                si16 dst_y;             //
                byte depth      = 32;   // 32-bit ARGB
                byte format     = 2;    // 2: ZPixmap
                byte send_event = 1;    // 0: Don't notify on output end; 1: Send event on output end (reply).
                byte pad        = 0;
                ui32 shm_seg_id;        // Linked segment ID.
                ui32 offset;            // Segment offset.
            };
        }
        namespace xfixes
        {
            struct query_version
            {
                struct reply
                {
                    byte status;         // 1: Reply
                    byte pad1;
                    ui16 sequence;
                    ui32 length;
                    ui32 server_major_version;
                    ui32 server_minor_version;
                    ui32 pad2[4];
                };
                byte major_opcode;       // xfixes_major_opcode.
                byte minor_opcode = 0;   // 0: XFixesQueryVersion.
                ui16 length = 3;
                ui32 client_major_version = 2; // Required 2.0+ (Window Shape(Input) Region).
                ui32 client_minor_version = 0;
            };
            struct create_region // XFixesCreateRegion (Minor opcode 5).
            {
                byte major_opcode;      // xfixes_major_opcode.
                byte minor_opcode = 5;
                ui16 length       = 4;
                ui32 region_id;
                // Payload (rectangle list)...
                ui16 rect0[4] = {}; // Empty rect. ui16 x=0,y=0,w=300,h=200
            };
            struct destroy_region // XFixesDestroyRegion (Minor opcode 10).
            {
                byte major_opcode;      // xfixes_major_opcode.
                byte minor_opcode = 10;
                ui16 length       = 2;
                ui32 region_id;
            };
            struct set_window_shape_region // XFixesSetWindowShapeRegion (Minor opcode 21).
            {
                static constexpr auto ShapeBounding = 0; // Visible boundary (painting geometry).
                static constexpr auto ShapeClip     = 1; // Visible clip.
                static constexpr auto ShapeInput    = 2; // Input hit-test.

                byte major_opcode;      // xfixes_major_opcode.
                byte minor_opcode = 21; // 21: XFixesSetWindowShapeRegion
                ui16 length       = 5;
                ui32 window_id;         // Dest window ID.
                byte shape_kind   = ShapeInput;  // 2: ShapeInput (input region).
                byte pad[3]       = {};
                si16 x_offset     = 0;  // Region offset.
                si16 y_offset     = 0;  //
                ui32 region_id;         // 0: None - Reset any filtering.
            };
        }
        namespace xi2
        {
            static constexpr auto MasterPointer  = 1;
            static constexpr auto MasterKeyboard = 2;
            static constexpr auto SlavePointer   = 3;
            static constexpr auto SlaveKeyboard  = 4;
            static constexpr auto FloatingSlave  = 5;

            struct query_version
            {
                struct reply
                {
                    byte status;         // 1: Reply
                    byte xi_opcode;      // minor_opcode (47)
                    ui16 sequence;
                    ui32 length;         // Always 0.
                    ui16 server_major_version;
                    ui16 server_minor_version;
                    ui32 pad[5];
                };
                byte major_opcode;       // xi2_major_opcode.
                byte minor_opcode = 47;  // 47: QueryVersion.
                ui16 length = 2;
                ui16 client_major_version = 2; // Required version 2.2 of the input stack for smooth scroll and touchpad.
                ui16 client_minor_version = 2; //
            };
            struct kbmods // Keybd modifiers (bitfields).
            {
                ui32 pressed;   // Pressed modifiers (Shift, Ctrl, ...).
                ui32 latched;   // Sticky keys.
                ui32 locked;    // Locks (CapsLock, NumLock).
                ui32 effective; // All mods.
            };
            struct kblayout // Keybd layout state (XKB Groups).
            {
                byte base_group;
                byte latched;
                byte locked;
                byte effective; // 0: EN, 1: RU etc.
            };
            namespace mods
            {
                static constexpr auto Shift    = 1u << 0; // 01 Shift.
                static constexpr auto CapsLock = 1u << 1; // 02 CapsLock.
                static constexpr auto Ctrl     = 1u << 2; // 04 Control.
                static constexpr auto mod1     = 1u << 3; // 08 Alt.
                static constexpr auto mod2     = 1u << 4; // 10 NumLock.
                static constexpr auto mod3     = 1u << 5; // 20 Level5Shift.
                static constexpr auto mod4     = 1u << 6; // 40 Win.
                static constexpr auto mod5     = 1u << 7; // 80 Level3Shift/AltGr.

                static constexpr auto Alt         = mod1;
                static constexpr auto NumLock     = mod2;
                static constexpr auto Level5Shift = mod3;
                static constexpr auto Win         = mod4;
                static constexpr auto AltGr       = mod5;
            }
            namespace leds
            {
                static constexpr auto CapsLock   = 1u << 1;
                static constexpr auto NumLock    = 1u << 4;
                static constexpr auto ScrollLock = 1u << 6;
            }
            namespace event
            {
                #define eventlist        \
                    X(undef             )\
                    X(DeviceChanged     )\
                    X(KeyPress          )\
                    X(KeyRelease        )\
                    X(ButtonPress       )\
                    X(ButtonRelease     )\
                    X(Motion            )\
                    X(Enter             )\
                    X(Leave             )\
                    X(FocusIn           )\
                    X(FocusOut          )\
                    X(HierarchyChanged  )\
                    X(PropertyEvent     )\
                    X(RawKeyPress       )\
                    X(RawKeyRelease     )\
                    X(RawButtonPress    )\
                    X(RawButtonRelease  )\
                    X(RawMotion         )\
                    X(TouchBegin        ) /*v2.2*/\
                    X(TouchUpdate       )\
                    X(TouchEnd          )\
                    X(TouchOwnership    )\
                    X(RawTouchBegin     )\
                    X(RawTouchUpdate    )\
                    X(RawTouchEnd       )\
                    X(BarrierHit        ) /*v2.3*/\
                    X(BarrierLeave      )\
                    X(GesturePinchBegin ) /*v2.4*/\
                    X(GesturePinchUpdate)\
                    X(GesturePinchEnd   )\
                    X(GestureSwipeBegin )\
                    X(GestureSwipeUpdate)\
                    X(GestureSwipeEnd   )
                static constexpr auto _counter = __COUNTER__ + 1;
                #define X(a) static constexpr auto a = __COUNTER__ - _counter;
                    eventlist
                #undef X
                #define X(a) #a##sv,
                    static constexpr auto names = std::to_array({ eventlist });
                #undef X
                #undef eventlist

                struct base
                {
                    byte type;      // Always 35 (GenericEvent).
                    byte extension; // xi_major_opcode.
                    ui16 sequence;
                    ui32 length;    // Payload length in quads.
                    ui16 evtype;    // Event type (e.g., 2: KeyPress, 3: KeyRelease).
                    ui16 deviceid;  // Physical or virtual device id.
                    ui32 time;      // Time stamp.
                };
                struct device_changed // 1.
                {
                    static constexpr auto KeyClass      = 0;
                    static constexpr auto ButtonClass   = 1;
                    static constexpr auto ValuatorClass = 2;
                    static constexpr auto ScrollClass   = 3;
                    static constexpr auto TouchClass    = 8;

                    static constexpr auto SlaveSwitch  = 1;
                    static constexpr auto DeviceChange = 2;

                    static constexpr auto Absolute = 0;
                    static constexpr auto Relative = 1;

                    base header;
                    ui16 num_classes; // any_class count in payload.
                    ui16 sourceid;    // Source of the new prop classes.
                    byte reason;      // 1: SlaveSwitch, 2: DeviceChange.
                    byte pad[11];
                    // payload...

                    struct any_class // Header of device properties.
                    {
                        ui16 type;     // 0: Key, 1: Button, 2: Valuator (axis), 3: Scroll, 8: Touch.
                        ui16 length;   // Length in quads including this header.
                        ui16 sourceid; // Device id.
                        ui16 pad;
                    };
                    struct key_class // 0. KeyClass (keybd scancode range).
                    {
                        ui16 type;          // 0: KeyClass.
                        ui16 length;        // KeyClass length with payload.
                        ui16 sourceid;      // Device ID.
                        ui16 num_keys;      // Key count.

                        static auto keys_ptr(char const* class_ptr)
                        {
                            return (ui32 const*)(class_ptr + sizeof(key_class));
                        }
                    };
                    struct button_class // 1. ButtonClass (Mouse button count + buuton names (atoms)).
                    {
                        ui16 type;          // 1: ButtonClass.
                        ui16 length;        // Pack length.
                        ui16 sourceid;
                        ui16 num_buttons;   // Button count.

                        static auto state_mask_ptr(char const* class_ptr) // Button state array.
                        {
                            return (ui32 const*)(class_ptr + sizeof(button_class));
                        }
                        auto labels_ptr(char const* class_ptr) const // Button name array (atom list).
                        {
                            auto mask_words = (size_t)(num_buttons + 31) / 32;
                            return state_mask_ptr(class_ptr) + mask_words;
                        }
                    };
                    struct valuator_class // 2. ValuatorClass (absolute/relative axis bounds for mouse/touchpad).
                    {
                        ui16 type;          // 2: ValuatorClass.
                        ui16 length;
                        ui16 sourceid;
                        ui16 number;        // Axis number (e.g., 0: X, 1: Y).
                        ui32 label;         // Atom: axis name (e.g., "Rel X").
                        fx32 min;           // Min value.
                        fx32 max;           // Max value.
                        fx32 value;         // Current value.
                        ui32 resolution;    // Resolution in unit/meter.
                        byte mode;          // 0: Absolute, 1: Relative
                        byte pad[3];
                    };
                    struct scroll_class // 3. Scroll direction + step length.
                    {
                        static constexpr auto Vertical   = 1;
                        static constexpr auto Horizontal = 2;

                        static constexpr auto NoEmulation = 1;
                        static constexpr auto Preferred   = 2;

                        ui16 type;          // 3: ScrollClass.
                        ui16 length;
                        ui16 sourceid;      // Device ID.
                        ui16 number;        // Wheel axis (valuator number).
                        ui16 scroll_type;   // 1: Vertical, 2: Horizontal.
                        ui16 pad;
                        ui32 flags;         // 1: NoEmulation, 2: Preferred.
                        fx32 inc_step;      // Scroll step.
                    };
                    struct touch_class // 8. TouchClass (multitouch panel).
                    {
                        ui16 type;          // 8: TouchClass.
                        ui16 length;        // Pack length.
                        ui16 sourceid;
                        byte mode;          // 0: Direct (touch-screen), 1: Dependent (touchpad).
                        byte pad;
                        ui16 num_touches;   // Max touches supported.
                    };
                };
                struct km // Keybd/Mouse
                {
                    static constexpr auto PointerEmulated       = 1u << 16;//1u << 4;?
                    static constexpr auto KeyRepeated           = 1u << 16;
                    static constexpr auto TouchPendingEnd       = 1u << 16;
                    static constexpr auto TouchEmulatingPointer = 1u << 17;

                    base header;
                    ui32 detail;         // Keybd: Keycode. Mouse: 0: Motion, 1: Left, 2: Middle, 3: Right, 4/5: Scroll.
                    ui32 root;           // Root window id.
                    ui32 event;          // Event window id.
                    ui32 child;          // Event child window id.
                    fx16 root_x;         // Global fixed point 16.16 coords.
                    fx16 root_y;         //
                    fx16 event_x;        // Relative fixed point 16.16 coords.
                    fx16 event_y;        //
                    ui16 buttons_len;    // Button mask array length in quads.
                    ui16 valuators_len;  // Valuators axis mask array length in quads.
                    ui16 sourceid;       // Event source device id.

                    ui16 pad;
                    ui32 flags;          // KeyRepeated for keybd. PointerEmulated for mouse scroll.

                    kbmods   mods;
                    kblayout group;

                    static auto buttons_mask_ptr(char const* packet_ptr)
                    {
                        return (ui32 const*)(packet_ptr + sizeof(km));
                    }
                    auto valuators_mask_ptr(char const* packet_ptr) const
                    {
                        return buttons_mask_ptr(packet_ptr) + buttons_len;
                    }
                    auto valuators_data_ptr(char const* packet_ptr) const
                    {
                        return (fx32 const*)(valuators_mask_ptr(packet_ptr) + valuators_len);
                    }
                };
                struct focus
                {
                    base header;
                    ui16 sourceid;
                    byte mode;   // Normal, Grab, Ungrab.
                    byte detail; // Ancestor, Virtual, Inferior, Nonlinear, NonlinearVirtual, Pointer, PointerRoot, None.
                    ui32 root;
                    ui32 event;
                    ui32 child;
                    fx16 root_x;
                    fx16 root_y;
                    fx16 event_x;
                    fx16 event_y;
                    byte same_screen;
                    byte focus;       // Unspecified for FocusIn/Out events.
                    ui16 buttons_len; // Length of button mask in payload (in quads).

                    kbmods   mods;
                    kblayout group;
                };
                struct hierarchy_changed
                {
                    static constexpr auto MasterAdded    = 1 << 0;
                    static constexpr auto MasterDeleted  = 1 << 1;
                    static constexpr auto SlaveAdded     = 1 << 2;
                    static constexpr auto SlaveRemoved   = 1 << 3;
                    static constexpr auto SlaveAttached  = 1 << 4;
                    static constexpr auto SlaveDetached  = 1 << 5;
                    static constexpr auto DeviceEnabled  = 1 << 6;
                    static constexpr auto DeviceDisabled = 1 << 7;

                    base header;
                    ui32 flags; // MasterAdded|MasterDeleted|SlaveAttached...
                    ui16 num_info;
                    ui16 pad0;
                    ui32 pad1;
                    ui32 pad2;

                    struct info
                    {
                        ui16 deviceid;
                        ui16 attachment; // Paired or master device id.
                        byte use;        // MasterKeyboard, MasterPointer, ...
                        byte enabled;
                        ui16 pad;
                        ui32 flags;      // MasterAdded|MasterDeleted|SlaveAttached...
                    };
                };
            }
            namespace dev_type
            {
                static constexpr auto all_devices        = 0; // All system devices.
                static constexpr auto all_master_devices = 1; // Virtual generic master devices (keybd/mouse).
            }
            struct query_pointer
            {
                struct reply
                {
                    byte     type;      // xi2_major_opcode.
                    byte     extension; // query_pointer.
                    ui16     sequence;
                    ui32     length;
                    ui32     root_id;
                    ui32     child_id;
                    fx16     root_x;
                    fx16     root_y;
                    fx16     win_x;
                    fx16     win_y;
                    byte     same_screen;
                    byte     pad0;
                    ui16     buttons_len;
                    kbmods   mods;
                    kblayout group;
                };
                byte major_opcode;
                byte minor_opcode = 40; // 40: QueryPointer.
                ui16 length       = 3;
                ui32 window_id;
                ui16 device_id;
                ui16 pad1 = {};
            };
            struct select_events // Minor opcode 46.
            {
                struct payload // device_mask
                {
                    ui16 deviceid;     // xi_device_id (e.g., all_master_devices).
                    ui16 mask_len = 2; // Mask length in quads.
                    ui32 mask1 = 0;    // 1..31 Mask of required events (event_type).
                    ui32 mask2 = 0;    // 32...
                };
                byte major_opcode;      // xi2_major_opcode
                byte minor_opcode = 46; // 46: SelectEvents
                ui16 length;
                ui32 window_id;
                ui16 num_masks = 1;     // Number of device_mask in payload.
                ui16 pad = 0;

                auto serialize(text& yield, payload mask_data)
                {
                    length = (ui16)(sizeof(*this) / 4 + sizeof(mask_data) / 4);
                    yield += view{ (char*)this, sizeof(*this) };
                    yield += view{ (char*)&mask_data, sizeof(mask_data) };
                    yield.append(-yield.size() & 3, '\0');
                }
            };
            struct query_device // Minor opcode 48.
            {
                struct reply
                {
                    byte type;        // Always 1 (Reply).
                    byte pad0;
                    ui16 sequence;
                    ui32 length;      // Payload length in quads.
                    ui16 num_devices; // Device count.
                    ui16 pad[11];

                    struct device_info
                    {
                        ui16 deviceid;
                        ui16 use;         // 1: MasterPointer, 2: MasterKeyboard, 3: SlavePointer, 4: SlaveKeyboard, 5: FloatingSlave.
                        ui16 attachment;  // Attached (paired) to device id.
                        ui16 num_classes; // Number of classes in payload.
                        ui16 name_len;    // Name length in bytes.
                        byte enabled;     // Device is enabled.
                        byte pad;
                        // payload:
                        //    - name (utf8)
                        //    - class list
                    };
                };
                byte major_opcode;      // xi2_major_opcode
                byte minor_opcode = 48; // 48: QueryDevice
                ui16 length = 2;
                ui16 device_id;         // 0: all_devices, 1: all_master_devices, or device_id.
                ui16 pad = 0;
            };
            //struct set_focus // Minor opcode 49.
            //{
            //    byte major_opcode;      // xi2_major_opcode
            //    byte minor_opcode = 49; // 49: SetFocus
            //    ui16 length = 4;        // Length in quads.
            //    ui32 window_id;
            //    ui32 time = 0;          // 0: CurrentTime.
            //    ui16 device_id;         // Virtual master keyboard id.
            //    ui16 pad = {};
            //};
            struct grab_device // Minor opcode 51.
            {
                static constexpr auto GrabModeSync  = 0;
                static constexpr auto GrabModeAsync = 1;
                static constexpr auto GrabModeTouch = 2;

                static constexpr auto StatusGrabSuccess     = 0;
                static constexpr auto StatusAlreadyGrabbed  = 1;
                static constexpr auto StatusGrabInvalidTime = 2;
                static constexpr auto StatusGrabNotViewable = 3;
                static constexpr auto StatusGrabFrozen      = 4;

                struct reply
                {
                    byte type;
                    byte minor_opcode;
                    ui16 sequence;
                    ui32 length;
                    byte status;
                    byte pad[23];
                };
                byte major_opcode;      // xi2_major_opcode
                byte minor_opcode = 51; // 51: GrabDevice
                ui16 length = 7;        // Length in quads.
                ui32 window_id;
                ui32 time      = 0;     // 0: CurrentTime.
                ui32 cursor_id = 0;
                ui16 device_id;         // Virtual master pointer id.
                byte grab_mode          = grab_device::GrabModeAsync;
                byte paired_device_mode = grab_device::GrabModeAsync;;
                byte owner_events = 0;  // 0: window_id is the only owner.
                byte pad = {};
                ui16 mask_len = 1;
                // Payload:
                ui32 mask = (1u << x11::req::xi2::event::Motion)
                          | (1u << x11::req::xi2::event::ButtonPress)
                          | (1u << x11::req::xi2::event::ButtonRelease)
                          | (1u << x11::req::xi2::event::DeviceChanged);
            };
            struct ungrab_device // Minor opcode 52.
            {
                byte major_opcode;      // xi2_major_opcode
                byte minor_opcode = 52; // 52: UngrabDevice
                ui16 length       = 3;
                ui32 time         = 0;
                ui16 device_id;         // Virtual master pointer id.
                ui16 pad          = {};
            };
        }
        namespace xkb
        {
            //namespace event
            //{
            //    static constexpr auto StateNotify = 0;
            //    static constexpr auto MapNotify   = 1;
            //    struct any
            //    {
            //        byte type;
            //        byte xkb_type;
            //        ui16 sequence;
            //        ui32 time;
            //        byte device;
            //    };
            //    struct state_notify
            //    {
            //        byte type;
            //        byte xkb_type;
            //        ui16 sequence;
            //        ui32 time;
            //        byte device;
            //        byte mods;
            //        byte base_mods;
            //        byte latched_mods;
            //        byte locked_mods;
            //        byte group;
            //        byte base_group;
            //        ui16 latched_group;
            //        ui16 locked_group;
            //        //...
            //    };
            //}

            // Shift Levels.
            static constexpr auto Base_Char          = 0; // map_entry.syms[0]  Level 1 ('q')
            static constexpr auto Shift_Char         = 1; // map_entry.syms[1]  Level 2 ('Q')
            static constexpr auto AltGr_Char         = 2; // map_entry.syms[2]  Level 3 (symbols/diaritics)
            static constexpr auto AltGr_Shift        = 3; // map_entry.syms[3]  Level 4 (symbols/diaritics)
            static constexpr auto Level5_Char        = 4; // map_entry.syms[4]  Level 5 (symbols/diaritics/national)
            static constexpr auto Level5_Shift       = 5; // map_entry.syms[5]  Level 6 (symbols/diaritics/national upper case)
            static constexpr auto Level5_AltGr       = 6; // map_entry.syms[6]  Level 7 (symbols/diaritics)
            static constexpr auto Level5_AltGr_Shift = 7; // map_entry.syms[7]  Level 8 (symbols/diaritics)

            static constexpr auto UseCoreKbd = 0x0100;

            static constexpr auto DetectableAutoRepeat = 1u;
            static constexpr auto DetectableAutoRepeatMask = 1u << (DetectableAutoRepeat - 1);

            static constexpr auto KeyTypesMask           = 1 << 0; // Return list of key_type_desc. Type like OneLevel/TwoLevel/Alphabetic.
            static constexpr auto KeySymsMask            = 1 << 1; // Return list of key_sym_map_desc. Unicode codepoints for [layout_index][shift_level0..7].
            static constexpr auto ModifierMapMask        = 1 << 2; // Return "pair<keycode,bytemask> modifier_map[total_mod_map_keys]". What modifier KeyCodes (list) are mapped to modifier bitfield (8 bit).

            static constexpr auto ExplicitComponentsMask = 1 << 3; // List of user specified key properties (server can't change/update these properties).
            static constexpr auto KeyActionsMask         = 1 << 4; // CapsLock to lock caps or Ctrl+Alt+Backspace to XkbSA_Terminate (kill x-server). Or multimedia key actions.
            static constexpr auto KeyBehaviorsMask       = 1 << 5; // XkbKB_Lock/XkbKB_RadioGroup/XkbKB_Overlay1(a-la Fn)
            static constexpr auto VirtualModsMask        = 1 << 6; // 16 modifiers name list like (16 bit): NumLock, AltGr, ScrollLock, Meta, Hyper, Super...
            static constexpr auto VirtualModMapMask      = 1 << 7; // How to map 16 bit modifiers to 8 bit modifier bitfiled.

            static constexpr auto AllClientInfoMask    = KeyTypesMask | KeySymsMask | ModifierMapMask;
            static constexpr auto AllServerInfoMask    = ExplicitComponentsMask | KeyActionsMask | KeyBehaviorsMask | VirtualModsMask | VirtualModMapMask;
            static constexpr auto AllMapComponentsMask = AllClientInfoMask | AllServerInfoMask;

            struct query_version // 0: QueryVersion (XkbUseExtension = 0).
            {
                struct reply
                {
                    byte status;         // 1: Reply
                    byte xkb_opcode;     // 0: QueryVersion (XkbUseExtension = 0).
                    ui16 sequence;
                    ui32 length;         // Always 0.
                    ui16 server_major_version;
                    ui16 server_minor_version;
                    ui32 pad[5];
                };
                byte major_opcode;      // xkb_major_opcode.
                byte minor_opcode = 0;  // 0: QueryVersion (XkbUseExtension = 0).
                ui16 length = 2;
                ui16 client_major_version = 1;
                ui16 client_minor_version = 0;
            };
            //struct select_events // 1: XkbSelectEvents
            //{
            //    static constexpr auto StateNotify = 1 << 0;
            //    static constexpr auto MapNotify   = 1 << 1;
            //    byte major_opcode;         // session.xkb_major_opcode
            //    byte minor_opcode = 1;     // 1: XkbSelectEvents
            //    ui16 length       = 4;
            //    ui16 device_spec  = UseCoreKbd; // XkbUseCoreKbd.
            //    ui16 affect_which = StateNotify | MapNotify;
            //    ui16 clear        = 0;
            //    ui16 select_any   = StateNotify | MapNotify;
            //    ui16 affect_map   = 0;
            //    ui16 map          = 0;
            //};
            struct get_state // 4: XkbGetState (get modifier state).
            {
                struct reply
                {
                    byte type;
                    byte device_id;
                    ui16 sequence;
                    ui32 length;
                    byte mods;
                    byte base_mods;
                    byte latched_mods;
                    byte locked_mods;
                    byte group;
                    byte locked_group;
                    si16 base_group;
                    si16 latched_group;
                    byte compat_state;
                    byte grab_mods;
                    byte compat_grab_mods;
                    byte lookup_mods;
                    byte compat_lookup_mods;
                    byte pad1;
                    ui16 pointer_btn_state;
                    ui16 pad2[3];
                };
                byte major_opcode;     // xkb_major_opcode
                byte minor_opcode = 4; // 4: XKBGetState.
                ui16 length       = 2;
                ui16 device_spec  = UseCoreKbd;
                ui16 pad          = {};
            };
            struct get_map // 8: XkbGetMap (get keyboard layout).
            {
                struct reply
                {
                    byte type;
                    byte device_id;
                    ui16 sequence;
                    ui32 length;
                    ui16 pad0;
                    byte min_key_code;
                    byte max_key_code;
                    ui16 present;      // Payload bitmask.
                    byte first_type;
                    byte num_types;
                    byte total_types;
                    byte first_key_sym;
                    ui16 total_syms;
                    byte num_key_syms;
                    byte first_key_act;
                    ui16 total_acts;
                    byte num_key_acts;
                    byte first_key_behavior;
                    byte num_key_behaviors;
                    byte total_key_behaviors;
                    byte first_key_explicit;
                    byte num_key_explicit;
                    byte total_key_explicit;
                    byte first_mod_map_key;
                    byte num_mod_map_keys;
                    byte total_mod_map_keys;
                    byte first_vmod_map_key;
                    byte num_vmod_map_keys;
                    byte total_vmod_map_keys;
                    byte pad1;
                    ui16 virtual_mods;
                    // payload...
                    struct key_type_desc // 1. Key's Type Description Block. Returned by KeyTypesMask. xkbKeyTypeWireDesc.
                    {
                        byte mask;           // Битовая маска модификаторов, на которые реагирует тип (напр. для Shift будет 0x00000001)
                        byte real_mods;      // Какие из них реальные.
                        ui16 virtual_mods;   // Какие из них виртуальные (AltGr, NumLock и т.д.)
                        byte num_levels;     // Сколько уровней сдвига генерирует тип (width)
                        byte num_map_entries;// Количество правил перевода модификаторов в уровни.
                        byte preserve;       // Флаг наличия таблицы сохранения модификаторов (сервер не убирает модификатор из mods.effective) (пока не знаю зачем мне это может понадобиться)
                        byte pad;
                        // Сразу за этим дескриптором идут два массива:
                        // a) xkb_kt_map_entry map_entries[num_map_entries];
                        // b) if (preserve!=0) mods_desc preserve_real_mods[num_map_entries];
                        struct xkb_kt_map_entry // xkbKTMapEntryWireDesc Структура правила (маппинга) внутри типа клавиши
                        {
                            byte active;         // 1: enabled, 0: not active.
                            byte mods_mask;      // Affected modifiers for the rule.
                            byte level;          // НА КАКОЙ УРОВЕНЬ ПЕРЕКЛЮЧИТЬ (0..num_levels-1)
                            byte real_mods;      // Физические модификаторы правила.
                            ui16 virtual_mods;   // Виртуальные модификаторы правила
                            ui16 pad;
                        };
                        struct mods_desc // ModsWireDesc - это для if (key_type_desc::preserve!=0)
                        {
                            byte mask;
                            byte realMods;
                            ui16 virtualMods;
                        };
                    };
                    struct key_sym_map_desc // 2. xkbSymMapWireDesc (XkbSymMapRec).
                    {
                        static constexpr auto GroupCountMask = (byte)0b00'00'1111; // g_count= 0 или 1..4
                        static constexpr auto GroupsWrapMask = (byte)0b11'00'0000;

                        static constexpr auto Wrap_WrapIntoRange     = 0b00'00'0000; // g = g % g_count.
                        static constexpr auto Wrap_ClampIntoRange    = 0b01'00'0000; // g = clamp(g, 0, g_count - 1).
                        static constexpr auto Wrap_RedirectIntoRange = 0b10'00'0000; // g = 0. Unconditional redirect to Group0.

                        byte kt_index[4];    // Key type for every group (0..num_key_types).
                        byte group_info;     // Bits 0-3: layout_count (1..4). Bits 4-5: GroupsWrap flags. Bits 6-7: Reserved.
                        byte width;          // Shift-level count for the key: 1..8.
                        ui16 num_syms;       // KeySym count for the key.
                        // ui32 syms[num_syms]
                    };
                    // 3. List KeyCodes for modifiers:
                    // ui32 modifier_codes[total_mod_map_keys];

                    // Когда пользователь нажимает LeftCtrl (KeyCode 50) и букву Q (KeyCode 24):
                    // - Из Блока 3 (modifier_map) смотрим, что KeyCode 50 взводит бит 0x04 (Control).
                    // - Из Блока 2 (key_sym_map_desc) для KeyCode 24 берем kt_index (например KeyType=2).
                    // - Идем в Блок 1 (xkb_key_type_desc) под индексом 2 и проверяем, есть ли там правило для маски 0x04 (Control).
                    //   - Если правила нет, уровень сдвига остается дефолтным (level = 0).
                    // - Возвращаемся в Блок 2 и забираем из all_syms латинский код символа 'q'.
                    // Жесть какая.
                };

                byte major_opcode;     // xkb_major_opcode
                byte minor_opcode = 8; // 8: XkbGetMap.
                ui16 length       = 7;
                ui16 device_spec  = UseCoreKbd; // XkbUseCoreKbd (system keybd).
                ui16 full         = AllClientInfoMask; // Request complete (full) client tables. Bit field.
                ui16 partial      = 0; // Request tables for key range (if non zero).
                byte first_type         = 0; // types like OneLevel/TwoLevel/Alphabetic.
                byte num_types          = 0; //
                byte first_key_sym;   // s.min_keycode.
                byte num_key_syms;    // s.max_keycode - s.min_keycode + 1 (all keys).
                byte first_key_act      = 0; // Request key actions: like CapsLock or mouse pointer mover.
                byte num_acts           = 0;
                byte first_key_behavior = 0; // like radio button, autorepeat, lock toggle.
                byte num_behaviors      = 0;
                ui16 virtual_mods       = 0; // like NumLock, AltGr...
                byte first_key_explicit = 0; // Explicit properties (user specified vs set by x-server). List of user specified key properties (strong fixed by user).
                byte num_explicit       = 0;
                byte first_mod_map_key  = 0; // Modifiers binding to bitfield: Shift, Ctrl, Lock, Mod1–Mod5.
                byte num_mod_map_keys   = 0;
                byte first_vmod_map_key = 0; // Virtual modifier mapping: like AltGr to Mod5 bit.
                byte num_vmod_map_keys  = 0;
                ui16 pad1               = 0;
            };
            struct per_client_flags // 21: XkbXPerClientFlags (switch autorepeat mode).
            {
                struct reply
                {
                    byte type;
                    byte xkb_type;
                    ui16 sequence;
                    ui32 length;
                    ui32 supported;
                    ui32 value;
                    ui32 auto_ctrls;
                    ui32 auto_ctrl_values;
                    ui32 pad[2];
                };
                byte major_opcode;          // xkb_major_opcode
                byte minor_opcode     = 21; // 21: XkbXPerClientFlags.
                ui16 length           = 7;
                ui16 device_spec      = UseCoreKbd;
                ui16 pad0             = 0;
                ui32 change_mask      = DetectableAutoRepeatMask; // Bit mask.
                ui32 value            = DetectableAutoRepeat;     // New value: Enable DetectableAutoRepeat.
                ui32 ctrls_to_change  = 0;
                ui32 auto_ctrls       = 0;
                ui32 auto_ctrl_values = 0;
            };
        }
        namespace xpresent
        {
            static constexpr auto ConfigureNotifyMask = 1 << 0;
            static constexpr auto CompleteNotifyMask  = 1 << 1;
            static constexpr auto IdleNotifyMask      = 1 << 2;
            //static constexpr auto RedirectNotifyMask  = 1 << 3;

            static constexpr auto ConfigureNotify = 0;
            static constexpr auto CompleteNotify  = 1;
            static constexpr auto IdleNotify      = 2;
            //static constexpr auto RedirectNotify  = 3;

            struct query_version
            {
                struct reply
                {
                    byte status;
                    byte pad1;
                    ui16 sequence;
                    ui32 length;
                    ui32 server_major_version;
                    ui32 server_minor_version;
                    ui32 pad2[4];
                };
                byte major_opcode;     // xpresent_major_opcode.
                byte minor_opcode = 0; // 0: PresentQueryVersion.
                ui16 length = 3;
                ui32 client_major_version;
                ui32 client_minor_version;
            };
            struct present_pixmap
            {
                static constexpr auto PresentOptionNone         = 0;
                static constexpr auto PresentOptionAsync        = 1 << 0; // Don't wait for VBlank.
                static constexpr auto PresentOptionCopy         = 1 << 1;
                static constexpr auto PresentOptionUST          = 1 << 2;
                static constexpr auto PresentOptionSuboptimal   = 1 << 3;
                static constexpr auto PresentOptionAsyncMayTear = 1 << 4;

                byte major_opcode;       // xpresent_major_opcode
                byte minor_opcode  = 1;  // 1: PresentPixmap.
                ui16 length        = 18;
                ui32 window_id;          // s.back_hWnd
                ui32 pixmap_id;          // Virtual pixmap id (1x1).
                ui32 serial;             // Our frame id (seq_num_any).
                ui32 valid_region  = 0;  // 0: Update a whole window.
                ui32 update_region = 0;  //

                si16 x_offset      = 0;
                si16 y_offset      = 0;
                ui32 target_crtc   = 0;

                ui32 wait_fence    = 0;
                ui32 idle_fence    = 0;

                ui32 options       = 0;//PresentOptionCopy;//PresentOptionAsync;
                ui32 pad           = {};
                ui64 target_msc    = 0;  // 0: Make it fast as possible (don't wait for VBlank).
                ui64 divisor       = 0;
                ui64 remainder     = 0;
                // Payload: LISTofPRESENTNOTIFY
            };
            struct notify_msc
            {
                byte major_opcode;       // session.xpresent_major_opcode
                byte minor_opcode = 2;   // 2: PresentNotifyMSC
                ui16 length       = 10;
                ui32 window_id;          // master.hWnd
                ui32 serial;
                ui32 pad          = 0;
                ui64 target_msc   = 0;   // (current_msc + 1 or +2)
                ui64 divisor      = 0;
                ui64 remainder    = 0;
            };
            struct select_input // Subscribe window on xpresent events.
            {

                byte major_opcode;     // xpresent_major_opcode.
                byte minor_opcode = 3; // 3: PresentSelectInput.
                ui16 length       = 4;
                ui32 event_id;         // Our subscription ID (=session.new_resource_id()).
                ui32 window_id;        // Target window id.
                ui32 event_mask = CompleteNotifyMask;
            };
            struct base // notify_event header.
            {
                byte type;       // =GenericEvent (35).
                byte extension;  // =xpresent_major_opcode.
                ui16 sequence;
                ui32 length;
                ui16 evtype;
            };
            struct complete_notify // xpresent_complete_notify_event
            {
                static constexpr auto CompleteNotify = 0;

                base header;
                byte kind;       // =PresentCompleteKindPixmap (0).
                byte mode;       // =PresentCompleteModeCopy (0) or Flip (1).
                ui32 event_id;
                ui32 window_id;  // Our window id.
                ui32 serial;     // Our cookies.
                ui64 ust;        // Unadjusted System Time.
                ui64 msc;        // Media Stream Counter.
            };
            struct configure_notify
            {
                base header;
                ui16 pad2;
                ui32 event_id;
                ui32 window_id;
                si16 x;
                si16 y;
                ui16 width;
                ui16 height;
                si16 off_x;
                si16 off_y;
                ui16 pixmap_width;
                ui16 pixmap_height;
                ui32 pixmap_flags;
            };
            struct idle_notify
            {
                base header;
                ui16 pad2;
                ui32 event_id;
                ui32 window_id;
                ui32 serial;
                ui32 pixmap_id;
                ui32 idle_fence;
            };
        }
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
        namespace mask
        {
            #define eventmasks         \
                X(KeyPress            )\
                X(KeyRelease          )\
                X(ButtonPress         )\
                X(ButtonRelease       )\
                X(EnterWindow         )\
                X(LeaveWindow         )\
                X(PointerMotion       )\
                X(PointerMotionHint   )\
                X(Button1Motion       )\
                X(Button2Motion       )\
                X(Button3Motion       )\
                X(Button4Motion       )\
                X(Button5Motion       )\
                X(ButtonMotion        )\
                X(KeymapState         )\
                X(Exposure            )\
                X(VisibilityChange    )\
                X(StructureNotify     )\
                X(ResizeRedirect      )\
                X(SubstructureNotify  )\
                X(SubstructureRedirect)\
                X(FocusChange         )\
                X(PropertyChange      )\
                X(ColormapChange      )\
                X(OwnerGrabButton     )
            static constexpr auto _counter = __COUNTER__ + 1;
            #define X(a) static constexpr auto a = 1 << (__COUNTER__ - _counter);
                eventmasks
            #undef X
            #undef eventmasks
        }
        #define eventlist      \
            X(Error           )\
            X(Reply           )\
            X(KeyPress        )\
            X(KeyRelease      )\
            X(ButtonPress     )\
            X(ButtonRelease   )\
            X(MotionNotify    )\
            X(EnterNotify     )\
            X(LeaveNotify     )\
            X(FocusIn         )\
            X(FocusOut        )\
            X(KeymapNotify    )\
            X(Expose          )\
            X(GraphicsExpose  )\
            X(NoExpose        )\
            X(VisibilityNotify)\
            X(CreateNotify    )\
            X(DestroyNotify   )\
            X(UnmapNotify     )\
            X(MapNotify       )\
            X(MapRequest      )\
            X(ReparentNotify  )\
            X(ConfigureNotify )\
            X(ConfigureRequest)\
            X(GravityNotify   )\
            X(ResizeRequest   )\
            X(CirculateNotify )\
            X(CirculateRequest)\
            X(PropertyNotify  )\
            X(SelectionClear  )\
            X(SelectionRequest)\
            X(SelectionNotify )\
            X(ColormapNotify  )\
            X(ClientMessage   )\
            X(MappingNotify   )\
            X(GenericEvent    )\
            X(_last           )
        static constexpr auto _counter = __COUNTER__ + 1;
        #define X(a) static constexpr auto a = __COUNTER__ - _counter;
            eventlist
        #undef X
        auto str(si32 e)
        {
            static constexpr auto el = std::to_array(
            {
                #define X(a) #a##sv, // "EventName"sv
                    eventlist
                #undef X
            });
            return e >= 0 && e < _last ? el[e] : "undef"sv;
        }
        #undef eventlist
        struct any
        {
            byte type; // Bit 7 may be set if the event is artificially generated (SendEvent).
            byte detail;
            ui16 sequence;
            ui32 length; // Reply's payload length in quads.
            ui32 pad[6];
        };
        struct error // Type 0
        {
            byte type;
            byte error_code;   // Error code (1: BadRequest, 2: BadValue, 3: BadWindow, 128+: Extensions...).
            ui16 sequence;     // Request stamp.
            ui32 bad_value;    // Invalid XID.
            ui16 minor_opcode; // Request's minor opcode.
            byte major_opcode; // Request's major opcode.
            byte pad[21];
        };
        struct expose_event // Type: 12 (expose event).
        {
            byte type;
            byte pad0;
            ui16 sequence;
            ui32 window_id;
            ui16 x;         // Dirty region (relative to window).
            ui16 y;         //
            ui16 width;     //
            ui16 height;    //
            ui16 count;     // Left Expose events.
            byte pad1[14];
        };
        //struct create_notify // Type 16 (create window notify)
        //{
        //    byte type;
        //    byte pad0;
        //    ui16 sequence;
        //    ui32 parent_id; // Parent window id.
        //    ui32 window_id; // Created window id.
        //    si32 x, y;      // Window coor.
        //    si32 width, height; // Window size.
        //    si32 border_width;
        //    byte override_redirect;
        //};
        //struct map_notify // Type 19 (window map notify)
        //{
        //    byte type;
        //    byte pad0;
        //    ui16 sequence;
        //    ui32 event_window_id;   // Window ID sent event (could be parent).
        //    ui32 window_id;         // Mapped Window ID.
        //    byte override_redirect; // 0:..., 1:...
        //    byte pad2[19];
        //};
        struct configure_notify // Type 22 (configure notify) a-la WM_SIZE/WM_MOVE.
        {
            byte type;
            byte pad;
            ui16 sequence;
            ui32 event_window_id;
            ui32 window_id;
            ui32 above_sibling;
            si16 x;
            si16 y;
            ui16 width;  // New width.
            ui16 height; // New heigth.
            ui16 border_width;
            byte override_redirect;
            byte pad2;
        };
        struct property_notify // Type 28 (property notify)
        {
            byte type;
            byte pad0;
            ui16 sequence;
            ui32 window_id; // Source window ID.
            ui32 atom;      // Changed property atom.
            ui32 time;      // Timestamp in ms.
            byte state;     // 0: PropertyNewValue, 1: PropertyDelete.
            byte pad1[15];
        };
        using client_message = x11::req::send_event::reply;
    }
    template<class T>
    struct data_n_size
    {
        auto data() { return (void*)this; }
        auto size() { return sizeof(T::s); }
    };
    struct session_t : data_n_size<session_t>
    {
        struct auth
        {
            struct reply
            {
                byte status;            // 0: Failed, 1: Success.
                byte pad1;
                ui16 major_version;
                ui16 minor_version;
                ui16 additional_length; // Payload length in 4-byte chunks.
                // payload ...
            };
            byte byte_order;       // 0x6c ('l') or 0x42 ('B')
            byte pad1;             //
            ui16 major_version;    // X_PROTOCOL
            ui16 minor_version;    // X_PROTOCOL_REVISION
            ui16 auth_proto_len;   //
            ui16 auth_data_len;    //
            ui16 pad2;             //
        };
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
        struct session_init
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
    #pragma pack(pop)

        struct device_t
        {
            struct axis_t
            {
                fp64 last_val{};
                fp64 inc_step{};
                bool vertical{};
                limits<fp64> min_max{};
                ui32 dpi{};
                bool is_abs{};
                bool is_scroll{};
            };
            std::vector<axis_t> axes;
            text name;
            ui32 min_keycode{};
            ui32 max_keycode{};
            std::vector<ui32> pressed_buttons;
            si32 touch_mode{};
            ui16 num_touches{};
            bool is_master{};
            bool enabled{};
        };

        text                                  vendor_str;     // buffer[32..32+vendor_length] = vendor_str
        std::vector<format>                   pixmap_formats; // format * number_of_formats = pixmap_formats
        std::vector<screen>                   roots;          // screen * number_of_screens = roots (always a multiple of 4)
        ui32                                  root_window_id = 0;

        ui32                                  argb_visual32_id = 0;
        ui32                                  argb_colormap_id = 0;
        ui32                                  empty_region_id = 0; // XFixes empty region.
        //ui32                                  xsync_counter_id = 0; // XSync counter.
        //ui32                                  xsync_fence_id = 0; // XSync fence.
        //ui32                                  virtual_pixmap_id = 0; // Fake Pixmap for XPresent triggering.

        //ui32                                  atom_my_ping = 0; // _MY_PING
        ui32                                  atom_vtmx = 0; // VTMX  WIN32: WM_USER

        ui32                                  atom_wm_hints = 35; // WM_HINTS
        ui32                                  atom_wm_transient_for = 68; // WM_TRANSIENT_FOR
        ui32                                  atom_wm_normal_hints = 0;
        ui32                                  atom_wm_size_hints = 0;

        ui32                                  atom_motif_wm_hints = 0; // Disable decoractions.
        ui32                                  atom_net_wm_name = 0;
        ui32                                  atom_net_wm_state_skip_taskbar = 0; // Hide from the taskbar.
        ui32                                  atom_net_wm_state = 0;              //
        //ui32                                  atom_net_wm_window_opacity = 0; // _NET_WM_WINDOW_OPACITY (doesn't work in wslg)
        ui32                                  atom_net_wm_window_type = 0;
        //ui32                                  atom_net_wm_window_type_normal = 0;
        ui32                                  atom_net_wm_window_type_utility = 0;
        //ui32                                  atom_net_wm_window_type_combo = 0;
        //ui32                                  atom_compton_shadow = 0;
        //ui32                                  atom_net_wm_ping = 0;
        //ui32                                  atom_net_wm_sync_request = 0;
        //ui32                                  atom_net_wm_sync_request_counter = 0;
        //ui32                                  atom_net_wm_bypass_compositor = 0; //_XWAYLAND_ALLOW_FRACTIONAL_SCALE
        ui32                                  atom_wm_protocols = 0;
        ui32                                  atom_wm_delete_window = 0;
        ui32                                  atom_atom = 0;
        ui32                                  atom_cardinal = 0;
        ui32                                  atom_utf8_string = 0;
        ui32                                  atom_window = 0;
        ui32                                  atom_net_active_window = 0;
        ui32                                  atom_net_number_of_desktops = 0;
        ui32                                  atom_net_current_desktop = 0;
        ui32                                  atom_net_workarea = 0; // workarea = desktop_area if is not set (atom_net_workarea=0).
        ui32                                  atom_xkb_rules_names = 0; // Triggered on root_window when the list of keyboard layouts changes.

        byte                                  xfixes_major_opcode = 0;
        byte                                  xfixes_first_event = 0;

        byte                                  shm_major_opcode = 0;
        byte                                  shm_completion_event = 0;
        fd_t                                  shm_buffer_fd = os::invalid_fd;
        byte*                                 shm_buffer_ptr = {};
        ui32                                  shm_buffer_len = {};
        ui32                                  shm_segment_xid = {};

        byte                                  xi2_major_opcode = 0;

        byte                                  xkb_major_opcode = 0;
        byte                                  xkb_first_event = 0;

        byte                                  xpresent_major_opcode = 0;
        //byte                                  xpresent_first_event = 0;

        //byte                                  xsync_major_opcode = 0;
        //ui64                                  xsync_current_value = 1;

        size_t                                current_frame_index = {};
        bool shm_ready_flag[2]   = { true, true }; // Buffer ready flags.

        generics::indexer_growing<ui32, 256>  resource_indexer; // Use growing indexer to avoid reusing indexes.

        struct seq_handler
        {
            using fx_t = std::function<void(x11::event::any const& ev, qiew payload)>;
            ui16 sequence;
            fx_t callback;
        };
        std::deque<seq_handler> reply_callbacks;

        std::mutex              mutex;
        sptr<os::ipc::stdcon>   x11connection;        // Main X11 socket connection.
        ui16                    sequence_counter = 0; // Async sent request counter.

        std::mutex              sync_mutex;
        sptr<os::ipc::stdcon>   sync_x11connection;        // Parallel sync X11 socket connection.
        ui16                    sync_sequence_counter = 0; // Sync sent request counter.
        ui32                    sync_msg_window_id = 0;    // Window for receiving sync messages (=sync_base_id).
        text                    sync_buffer;

        std::unordered_map<ui16, device_t> input_devices;

        struct key_sym_map_t
        {
            byte num_groups = {};   // Layout count.
            byte width = {};        // Modifiers count.
            std::vector<ui32> syms; // size = num_groups * width.
        };
        std::array<key_sym_map_t, 256> key_map = {};

        //todo ?multiple displays: std::vector<rect> workareas;
        rect workarea; // Actual _NET_WORKAREA value.
        rect default_window_area; // Window area (? provided by the window manager).
        twod x11_display_size;
        si32 x11_diagonal{};
        bool wl_present{};

        std::array<flag, 65536> received_replies;

        session_t() = default;
        ~session_t()
        {
            reset_shared_buffer();
        }

        void sync_reply(ui16 sequence_number) const
        {
            while (received_replies[sequence_number].load(std::memory_order_acquire))
            {
                //if constexpr (debugmode) log(ansi::clr(tint::yellowlt, "%% wait sync_reply %%..."), datetime::now(), sequence_number);
                std::this_thread::yield();
            }
        }
        void sync_reply(ui16 sequence_number, span timeout, span wait_step = span{}) const
        {
            auto current_time = datetime::now();
            while (received_replies[sequence_number].load(std::memory_order_acquire))
            {
                //if constexpr (debugmode) log(ansi::clr(tint::yellowlt, "wait sync_reply %%..."), sequence_number);
                if (datetime::now() - current_time >= timeout) break;
                std::this_thread::sleep_for(wait_step);
            }
        }
        // Callback usage example.
        //    session.sendrq<x11::req::map_window>({ .window_id = 0 }, {},
        //    [&](auto& ev, view payload) // ev stored in payload.
        //    {
        //        if (ev.type == x11::event::Error)
        //        {
        //            if constexpr (debugmode) log("message");
        //        }
        //        else
        //        {
        //            if constexpr (debugmode) log("Recieved reply...");
        //            auto reply = netxs::start_lifetime_as<x11::req::...::reply>(payload.data());
        //            ...
        //        }
        //    });
        template<class R, class V = qiew, class P = noop>
        auto accumrq(text& batch_buffer, R request, V payload = {}, P callback = {}) // Note: callbacks must check reply errors on their side: ev.type == x11::event::Error.
        {
            //auto lock = std::lock_guard{ mutex };
            sequence_counter++;
            if constexpr (requires(text packet){ request.serialize(packet, payload); })
            {
                request.serialize(batch_buffer, payload);
            }
            else
            {
                assert(!payload);
                batch_buffer += view{ (char*)&request, sizeof(request) };
            }
            if constexpr (!std::is_same_v<P, noop>)
            {
                reply_callbacks.push_back({ sequence_counter, std::move(callback) });
            }
            return sequence_counter;
        }
        template<class R, class V = qiew, class P = noop>
        auto sendrq(R request = {}, V payload = {}, P callback = {}) // Note: callbacks must check reply errors on their side: ev.type == x11::event::Error.
        {
            auto lock = std::lock_guard{ mutex };
            sequence_counter++;
            if constexpr (requires(text packet){ request.serialize(packet, payload); })
            {
                auto packet = text{};
                request.serialize(packet, payload);
                x11connection->send(packet);
            }
            else
            {
                assert(!payload);
                x11connection->send(view{ (char*)&request, sizeof(request) });
            }
            if constexpr (!std::is_same_v<P, noop>)
            {
                reply_callbacks.push_back({ sequence_counter, std::move(callback) });
            }
            return sequence_counter;
        }
        template<class R, class V = qiew, class P = noop>
        auto syncrq(R request = {}, V payload = {})
        {
            auto lock = std::lock_guard{ sync_mutex };
            sync_sequence_counter++;
            if constexpr (requires(text packet){ request.serialize(packet, payload); })
            {
                auto packet = text{};
                request.serialize(packet, payload);
                sync_x11connection->send(packet);
            }
            else
            {
                assert(!payload);
                sync_x11connection->send(view{ (char*)&request, sizeof(request) });
            }
            return sync_sequence_counter;
        }
        template<class R, class V = qiew, class P = noop>
        auto syncrq(text& buffer, R request = {}, V payload = {})
        {
            //auto lock = std::lock_guard{ sync_mutex };
            sync_sequence_counter++;
            if constexpr (requires(text packet){ request.serialize(packet, payload); })
            {
                request.serialize(buffer, payload);
            }
            else
            {
                assert(!payload);
                buffer += view{ (char*)&request, sizeof(request) };
            }
            return sync_sequence_counter;
        }
        auto parse_reply(x11::event::any& ev, text& read_buffer)
        {
            auto r = std::decay_t<decltype(reply_callbacks.front())>{};
            {
                auto lock = std::lock_guard{ mutex };
                if (reply_callbacks.size())
                {
                    r = std::move(reply_callbacks.front());
                    reply_callbacks.pop_front();
                }
            }
            if (ev.type == x11::event::Reply && ev.length > 0)
            {
                auto extra_data_size = ev.length * sizeof(ui32);
                read_buffer.resize(32 + extra_data_size);
                x11connection->recv_all(read_buffer.data() + 32, extra_data_size); // Blocking call.
            }
            if constexpr (debugmode) log("%%seq=%%", prompt::x11, r.sequence);
            if (r.callback)
            {
                r.callback(ev, read_buffer);
            }
            else
            {
                if constexpr (debugmode) log("      Unexpected reply with an empty callback queue");
            }
        }
        text get_error(x11::event::any const& ev)
        {
            auto err = netxs::start_lifetime_as<x11::event::error>(ev);
            auto err_str = text{};
            switch (err.error_code)
            {
                case  1: err_str = "Bad Request";        break;
                case  2: err_str = "Bad Value";          break;
                case  3: err_str = "Bad Window";         break;
                case  4: err_str = "Bad Pixmap";         break;
                case  5: err_str = "Bad Atom";           break;
                case  6: err_str = "Bad Cursor";         break;
                case  7: err_str = "Bad Font";           break;
                case  8: err_str = "Bad Match";          break;
                case  9: err_str = "Bad Drawable";       break;
                case 10: err_str = "Bad Access";         break;
                case 11: err_str = "Bad Alloc";          break;
                case 12: err_str = "Bad Color";          break;
                case 13: err_str = "Bad GC";             break;
                case 14: err_str = "Bad IDChoice";       break;
                case 15: err_str = "Bad Name";           break;
                case 16: err_str = "Bad Length";         break;
                case 17: err_str = "Bad Implementation"; break;
            }
                 if (err.major_opcode == shm_major_opcode     ) err_str += " (MIT-SHM Extension Error)";
            else if (err.major_opcode == xfixes_major_opcode  ) err_str += " (XFIXES Extension Error)";
            else if (err.major_opcode == xi2_major_opcode     ) err_str += " (XInput2 Extension Error)";
            else if (err.major_opcode == xkb_major_opcode     ) err_str += " (XKB Extension Error)";
            //else if (err.major_opcode == xpresent_major_opcode) err_str += " (XPresent Extension Error)";
            //else if (err.major_opcode == xsync_major_opcode ) err_str += " ('SYNC' Extension Error)";
            return utf::fprint(ansi::err("%%Error: code=%%, seq=%%, bad_resource_id=0x%%, major=%%, minor=%% desc: %%"), prompt::x11,
                            (ui32)err.error_code, (ui32)err.sequence, utf::to_hex(err.bad_value),
                            (ui32)err.major_opcode, (ui32)err.minor_opcode, err_str);
        }
        auto parse_error(x11::event::any& ev, text& read_buffer)
        {
            log(get_error(ev));
            auto is_reply = faux;
            {
                auto lock = std::lock_guard{ mutex };
                is_reply = reply_callbacks.size() && reply_callbacks.front().sequence == ev.sequence;
            }
            if (is_reply) // Forward broken request reply to handler.
            {
                parse_reply(ev, read_buffer);
            }
        }
        auto event_str(si32 e)
        {
            return e == shm_completion_event ? "ShmCompletionEvent"
                 : e == xkb_first_event      ? "XkbEvent"
                                             : x11::event::str(e);
        }
        template<bool B = true>
        auto str() const
        {
            if (roots.empty()) return "no screen roots"s;
            auto str = utf::fprint(prompt::x11, "Connected:"
                "\n                id_base/mask: ",   utf::to_hex(s.resource_id_base), '/', utf::to_hex(s.resource_id_mask),
                "\n              root_window_id: 0x", utf::to_hex(roots.front().s.root_window_id),
                "\n          motion_buffer_size: ",   s.motion_buffer_size,
                "\n               vendor_length: ",   s.vendor_length,
                "\n                      vendor: '",  utf::debase<faux, faux>(vendor_str), '\'',
                "\n      maximum_request_length: ",   s.maximum_request_length,
                "\n           number_of_screens: ",   (si32)s.number_of_screens,
                "\n           number_of_formats: ",   (si32)s.number_of_formats,
                "\n            image_byte_order: ",   (si32)s.image_byte_order,
                "\n     bitmap_format_bit_order: ",   (si32)s.bitmap_format_bit_order,
                "\n bitmap_format_scanline_unit: ",   (si32)s.bitmap_format_scanline_unit,
                "\n  bitmap_format_scanline_pad: ",   (si32)s.bitmap_format_scanline_pad,
                "\n                 min_keycode: ",   (si32)s.min_keycode,
                "\n                 max_keycode: ",   (si32)s.max_keycode,
                "\n");
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
                str += utf::fprint("            root_window_id: 0x", utf::to_hex(sc.root_window_id),
                                 "\n          default_colormap: "  , sc.default_colormap,
                                 "\n               white_pixel: 0x", utf::to_hex(sc.white_pixel),
                                 "\n               black_pixel: 0x", utf::to_hex(sc.black_pixel),
                                 "\n       current_input_masks: 0x", utf::to_hex(sc.current_input_masks),
                                 "\n           width_in_pixels: "  , sc.width_in_pixels,
                                 "\n          height_in_pixels: "  , sc.height_in_pixels,
                                 "\n      width_in_millimeters: "  , sc.width_in_millimeters,
                                 "\n     height_in_millimeters: "  , sc.height_in_millimeters,
                                 "\n        min_installed_maps: "  , sc.min_installed_maps,
                                 "\n        max_installed_maps: "  , sc.max_installed_maps,
                                 "\n               root_visual: 0x", utf::to_hex(sc.root_visual),
                                 "\n            backing_stores: "  , (si32)sc.backing_stores,
                                 "\n               save_unders: "  , (si32)sc.save_unders,
                                 "\n                root_depth: "  , (si32)sc.root_depth,
                                 "\n          number_of_depths: "  , (si32)sc.number_of_depths,
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
                    //    str += utf::fprint("\tvisual_id=0x",              utf::to_hex(v.visual_id),
                    //                        "\n\t\t visual_class=",       (si32)v.visual_class,
                    //                        "\n\t\t bits_per_rgb_value=", (si32)v.bits_per_rgb_value,
                    //                        "\n\t\t colormap_entries=",   v.colormap_entries,
                    //                        "\n\t\t red_mask=0x",         utf::to_hex(v.red_mask),
                    //                        "\n\t\t green_mask=0x",       utf::to_hex(v.green_mask),
                    //                        "\n\t\t blue_mask=0x",        utf::to_hex(v.blue_mask),
                    //                        "\n");
                    //}
                }
            }
            if (str.back() == '\n') str.pop_back();
            return str;
        }
        auto new_resource_id()
        {
            auto current_idx = resource_indexer.get_new();
            auto resource_id = s.resource_id_base | (current_idx & s.resource_id_mask);
            return resource_id;
        }
        auto free_resource_id(ui32& resource_id)
        {
            resource_indexer.release(resource_id & s.resource_id_mask);
            resource_id = {};
        }
        auto detect_argb_32bit()
        {
            auto argb_supported = faux;
            for (auto& format : pixmap_formats) // Check if argb supported.
            {
                auto& pf = format.s;
                if (pf.depth == 32 && pf.bits_per_pixel == 32)
                {
                    argb_supported = true;
                    break;
                }
            }
            if (argb_supported && roots.size()) // Find visual_id with depth=32.
            for (auto& depth : roots.front().list_of_depths)
            {
                if (depth.s.depth == 32)
                if (depth.list_of_visual_types.size()) // Take first available visual_type.
                {
                    auto& v = depth.list_of_visual_types.front().s;
                    argb_visual32_id = v.visual_id;
                    argb_colormap_id = new_resource_id();
                    sendrq<x11::req::create_colormap>({ .colormap_id = argb_colormap_id,
                                                        .window_id   = root_window_id,
                                                        .visual_id   = argb_visual32_id });
                    if constexpr (debugmode) log("%%ARGB visual id found: argb_visual32_id=0x%%", prompt::x11, utf::to_hex(argb_visual32_id));
                    return true;
                }
            }
            auto errmsg = utf::fprint("%%32-bit ARGB pixel format is not supported on X11 server\n", prompt::x11);
            errmsg += pixmap_formats.size() ? utf::fprint("    Supported pixmap formats(%%):\n", pixmap_formats.size()) : "    There are no pixmap formats\n";
            for (auto& format : pixmap_formats)
            {
                auto& pf = format.s;
                errmsg += utf::fprint("\tdepth=%% bpp=%% scanline_pad=%%\n", (si32)pf.depth, (si32)pf.bits_per_pixel, (si32)pf.scanline_pad);
            }
            log<faux>(errmsg);
            return faux;
        }
        template<class ExtensionQueryVersion>
        auto detect_extension(qiew extension_name, byte& major_opcode, auto&& first_event, ui16 required_major_version, ui16 required_minor_version)
        {
            auto errdetails = text{};
            sendrq<x11::req::query_extension>({}, extension_name);
            auto reply = x11::req::query_extension::reply{};
            if (x11connection->recv_all((char*)&reply, sizeof(reply)).size() == sizeof(reply))
            if (reply.present)
            {
                major_opcode = reply.major_opcode;
                first_event  = reply.first_event;
                if constexpr (requires{ ExtensionQueryVersion::client_major_version; })
                {
                    sendrq<ExtensionQueryVersion>({ .major_opcode         = reply.major_opcode,
                                                    .client_major_version = (decltype(ExtensionQueryVersion::client_major_version))required_major_version,
                                                    .client_minor_version = (decltype(ExtensionQueryVersion::client_minor_version))required_minor_version, });
                }
                else
                {
                    sendrq<ExtensionQueryVersion>({ .major_opcode = reply.major_opcode });
                }
                auto v_reply = typename ExtensionQueryVersion::reply{};
                if (x11connection->recv_all((char*)&v_reply, sizeof(v_reply)).size() == sizeof(v_reply))
                if (v_reply.status == x11::event::Reply)
                if (v_reply.server_major_version > required_major_version
                || (v_reply.server_major_version == required_major_version && v_reply.server_minor_version >= required_minor_version)) // Check min version major.minor.
                {
                    if constexpr (debugmode) log("%%%% version %%.%% detected (ext_major_opcode=%% ext_completion_event=%%)", prompt::x11, extension_name, (si32)v_reply.server_major_version, (si32)v_reply.server_minor_version, (si32)major_opcode, (si32)first_event);
                    return true;
                }
                if (v_reply.status == x11::event::Reply)
                {
                    errdetails = utf::fprint("\n\t%% version %%.%% detected", extension_name, (si32)v_reply.server_major_version, (si32)v_reply.server_minor_version);
                }
                else
                {
                    auto ev = netxs::start_lifetime_as<x11::event::any>(&v_reply);
                    errdetails = utf::fprint("\n\tFailed to receive %% version details (ext_major_opcode=%% ext_completion_event=%%)\n\t%%", extension_name, (si32)major_opcode, (si32)first_event, get_error(ev));
                }
            }
            auto errmsg = utf::fprint("%%The required %% extension (or required min version %%.%%) is missing", prompt::x11, extension_name, required_major_version, required_minor_version);
            log(errmsg + errdetails);
            return faux;
        }
        void send_shm_attach_fd(ui32 client_shmseg_xid)
        {
            auto request = x11::req::shm::attach_fd{ .major_opcode = shm_major_opcode,
                                                     .shm_seg_id   = client_shmseg_xid };
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
            std::memcpy(&(CMSG_DATA(cmsg)), &shm_buffer_fd, sizeof(shm_buffer_fd));
            auto lock = std::lock_guard{ mutex };
            sequence_counter++;
            auto bytes_sent = ::sendmsg(x11connection->handle.w, &msg, 0);
            if (bytes_sent == -1)
            {
                log("%%sendmsg failed during shm_attach_fd invocation", prompt::x11);
            }
        }
        void send_shm_detach_fd(ui32 client_shmseg_xid)
        {
            sendrq<x11::req::shm::detach>({ .major_opcode = shm_major_opcode,
                                            .shm_seg_id   = client_shmseg_xid });
            if constexpr (debugmode) log("%%Shared buffer segment XID %% is detached", prompt::x11, client_shmseg_xid);
        }
        void set_mouse_input(text& batch_buffer, arch window_id, bool state) // Make sub-layer transparent for mouse.
        {
            accumrq(batch_buffer, x11::req::xfixes::set_window_shape_region{ .major_opcode = xfixes_major_opcode,
                                                                             .window_id    = (ui32)window_id,
                                                                             .region_id    = state ? 0 : empty_region_id }); // 0: enable mouse input, empty_region_id: disable mouse input.
        }
        void set_mouse_input(arch window_id, bool state) // Make sub-layer transparent for mouse.
        {
            sendrq<x11::req::xfixes::set_window_shape_region>({ .major_opcode = xfixes_major_opcode,
                                                                .window_id    = (ui32)window_id,
                                                                .region_id    = state ? 0 : empty_region_id }); // 0: enable mouse input, empty_region_id: disable mouse input.
        }
        auto create_window(arch master_window_id, ui32 new_window_id, ui32 new_gc_id, bool is_master, bool override_redirect)
        {
            sendrq<x11::req::create_window>({ .depth     = 32,
                                              .window_id = new_window_id,
                                              .parent_id = root_window_id,
                                              .visual_id = argb_visual32_id },
                                            x11::req::create_window::payload{ //.background_pixel = 0x00'000000u,
                                                                              .border_pixel = 0x00'000000u, // Mandatory: Own 32-bit ARGB border pixel value.
                                                                              .backing_store = x11::req::create_window::payload::Always,
                                                                              //.bit_gravity = x11::req::create_window::BitGravityStatic,//BitGravityForget,//BitGravityNorthWest,
                                                                              //.win_gravity = StaticGravity,
                                                                              //todo WSLg does not show any windows with override_redirect=1 if none with override_redirect=0 were shown previously after wsl boot (bug)
                                                                              .override_redirect = override_redirect, // 1: On.
                                                                              //.save_under = override_redirect ? 0 : 1,
                                                                              .event_mask = 0u
                                                                                          | (override_redirect ? 0 : x11::event::mask::Exposure) // KDE doesn't redraw our layers in background.
                                                                                          | x11::event::mask::StructureNotify // For ConfigureNotify events.
                                                                                          ,
                                                                              .colormap_id = argb_colormap_id }); // Mandatory: own colormap.
            // Mark our windows.
            sendrq<x11::req::change_property>({ .window_id = (ui32)new_window_id,
                                                .property  = atom_vtmx,
                                                .type      = atom_cardinal }, 1);
            // Disable decorations.
            if (!override_redirect)
            {
                sendrq<x11::req::change_property>({ .window_id = new_window_id,
                                                    .property  = atom_motif_wm_hints,
                                                    .type      = atom_motif_wm_hints },
                                                x11::motif::hints{ .flags = x11::motif::Decorations, .decorations = 0 });
            }
            // Set WM_PROTOCOLS.
            sendrq<x11::req::change_property>({ .window_id = (ui32)new_window_id,
                                                .property  = atom_wm_protocols,
                                                .type      = atom_atom },
                                            std::to_array({ atom_wm_delete_window })); //std::to_array({ atom_net_wm_sync_request, atom_net_wm_ping }));
            // Set XSync counter.
            //sendrq<x11::req::change_property>({ .window_id = (ui32)new_window_id,
            //                                    .property  = atom_net_wm_sync_request_counter,
            //                                    .type      = atom_cardinal },
            //                                xsync_counter_id);
            // Try to make it DPI-aware.
            //sendrq<x11::req::change_property>({ .window_id = (ui32)new_window_id,
            //                                    .property  = atom_net_wm_bypass_compositor, //_XWAYLAND_ALLOW_FRACTIONAL_SCALE
            //                                    .type      = atom_cardinal },
            //                                1);
            if (is_master)
            {
                //todo it doesn't work // Set XSync fence.
                //auto seq = sendrq<x11::req::xsync::create_fence>({ .major_opcode = xsync_major_opcode,
                //                                        .window_id    = (ui32)new_window_id,
                //                                        .fence_id     = xsync_fence_id });
                //if constexpr (debugmode) log("Attached fence_id: 0x%% to window_id=0x%% seq=%%", utf::to_hex(xsync_fence_id), utf::to_hex(new_window_id), seq);
                //todo it doesn't work // Make the window available for input focus.
                //sendrq<x11::req::change_property>({ .window_id = new_window_id,
                //                                    .property  = atom_wm_hints,
                //                                    .type      = atom_wm_hints },
                //                                x11::icccm::wm_hints{ .flags = x11::icccm::InputHint, .input = 1 });
                // Init XPresent event subscription.
                auto new_event_id = new_resource_id();
                sendrq<x11::req::xpresent::select_input>({ .major_opcode = xpresent_major_opcode,
                                                           .event_id     = new_event_id,
                                                           .window_id    = new_window_id });
            }
            else
            {
                //todo it doesn't work // Make the window output only.
                //sendrq<x11::req::change_property>({ .window_id = new_window_id,
                //                                    .property  = atom_wm_hints,
                //                                    .type      = atom_wm_hints },
                //                                x11::icccm::wm_hints{ .flags = x11::icccm::InputHint, .input = 0 });

                // Make the window output only.
                sendrq<x11::req::change_property>({ .window_id = new_window_id,
                                                    .property  = atom_net_wm_window_type,
                                                    .type      = atom_atom },
                                                atom_net_wm_window_type_utility);
                if (override_redirect == 0) // Wm layers only.
                {
                    //todo wslg (sometimes) places all transient windows inside the master if override_redirect=0
                    // Group sub-layers with master.
                    sendrq<x11::req::change_property>({ .window_id = new_window_id,
                                                        .property  = atom_wm_transient_for,
                                                        .type      = atom_window },
                                                    (ui32)master_window_id);
                }
                // Remove sub-layers from taskbar.
                if (!override_redirect)
                {
                    sendrq<x11::req::change_property>({ .window_id = new_window_id,
                                                        .property  = atom_net_wm_state,
                                                        .type      = atom_atom },
                                                    atom_net_wm_state_skip_taskbar);
                }
                // Make sub-layer transparent for mouse.
                set_mouse_input(new_window_id, faux);
            }

            // Disable shadows.
            //sendrq<x11::req::change_property>({ .window_id = new_window_id,
            //                                    .property  = atom_net_wm_window_type, // Atom "_NET_WM_WINDOW_TYPE".
            //                                    .type      = atom_atom },             // Atom XA_ATOM=4.
            //                                atom_net_wm_window_type_combo);
            //sendrq<x11::req::change_property>({ .window_id = new_window_id,
            //                                    .property  = atom_compton_shadow,      // Atom "_COMPTON_SHADOW".
            //                                    .type      = atom_cardinal },          // Atom XA_CARDINAL=6.
            //                                0u); // 0: off, 1: on.

            sendrq<x11::req::create_gc>({ .gc_id = new_gc_id, .drawable = new_window_id });

            if constexpr (debugmode) log("create window: window_id=%% parent_id=%% depth=%% visual_id=0x%% colormap_id=0x%%",
                utf::to_hex(new_window_id), utf::to_hex(root_window_id), 32, utf::to_hex(argb_visual32_id), utf::to_hex(argb_colormap_id));
        }
        void window_set_title(arch window_id, qiew title)
        {
            sendrq<x11::req::change_property>({ .window_id = (ui32)window_id,
                                                .property  = atom_net_wm_name,
                                                .type      = atom_utf8_string,
                                                .format    = sizeof(byte) * 8 },  // Format (8: 8-bit chars (string)).
                                            title);
        }
        bool resize_shared_buffer(size_t size)
        {
            if (shm_buffer_len)
            {
                //todo implement delayed detach+copy
                send_shm_detach_fd(shm_segment_xid);
                reset_shared_buffer();
                free_resource_id(shm_segment_xid);
            }
            shm_buffer_fd =
                #if defined(__linux__)
                    ::memfd_create("x11_shm_buffer", MFD_CLOEXEC);
                #else
                    ::shm_open(SHM_ANON, O_RDWR | O_CREAT | O_EXCL, 0600); // SHM_ANON - native anonymous descriptor in BSD.
                #endif
            if (shm_buffer_fd == os::invalid_fd)
            {
                log("%%Failed to create anonymous shared memory fd (errno=%%)", prompt::gui, os::error());
            }
            else
            {
                if (::ftruncate(shm_buffer_fd, size) == -1) // Set shm size.
                {
                    ::close(shm_buffer_fd);
                    shm_buffer_fd = os::invalid_fd;
                    log("%%Failed to truncate shared memory file to required size (errno=%%)", prompt::gui, os::error());
                }
                else
                {
                    auto mapped_ptr = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_buffer_fd, 0);
                    if (mapped_ptr == MAP_FAILED)
                    {
                        ::close(shm_buffer_fd);
                        shm_buffer_fd = os::invalid_fd;
                        log("%%Failed to mmap shared memory descriptor (errno=%%)", prompt::gui, os::error());
                    }
                    else
                    {
                        shm_buffer_ptr = (byte*)mapped_ptr;
                        shm_buffer_len = size;
                        shm_segment_xid = new_resource_id();
                        send_shm_attach_fd(shm_segment_xid);
                        if constexpr (debugmode) log("%%Shared buffer successfuly created at 0x%%, %% bytes", prompt::x11, utf::to_hex(shm_buffer_ptr), shm_buffer_len);
                    }
                }
            }
            return shm_buffer_len > 0;
        }
        void reset_shared_buffer()
        {
            if (shm_buffer_ptr && shm_buffer_ptr != MAP_FAILED)
            {
                ::munmap(shm_buffer_ptr, shm_buffer_len);
                shm_buffer_len = {};
                shm_buffer_ptr = {};
            }
            if (shm_buffer_fd != os::invalid_fd)
            {
                ::close(shm_buffer_fd);
                shm_buffer_fd = os::invalid_fd;
            }
        }
        auto get_atoms()
        {
            auto get_atom_id = [&](qiew name, bool create)
            {
                sendrq<x11::req::intern_atom>({ .only_if_exists = !create }, name); // 0: Create if absent. 1: Don't create.
                auto reply = x11::req::intern_atom::reply{};
                if (x11connection->recv_all((char*)&reply, sizeof(reply)).size() == 32 && reply.type == x11::event::Reply)
                {
                    if constexpr (debugmode) log("%%Received atom for '%%'=0x%%", prompt::x11, name, reply.atom_id);
                    return reply.atom_id;
                }
                log("%%Failed to intern atom: %%", prompt::x11, name);
                return 0u;
            };
            // Window related.
            //atom_my_ping                     = get_atom_id("_MY_PING", true);
            atom_motif_wm_hints              = get_atom_id("_MOTIF_WM_HINTS", true);
            atom_net_wm_name                 = get_atom_id("_NET_WM_NAME", true);
            atom_net_wm_state                = get_atom_id("_NET_WM_STATE", true);
            atom_net_wm_state_skip_taskbar   = get_atom_id("_NET_WM_STATE_SKIP_TASKBAR", true);
            atom_net_wm_window_type          = get_atom_id("_NET_WM_WINDOW_TYPE", true);
            atom_net_wm_window_type_utility  = get_atom_id("_NET_WM_WINDOW_TYPE_UTILITY", true);
            //atom_net_wm_window_type_normal   = get_atom_id("_NET_WM_WINDOW_TYPE_NORMAL", true);
            //atom_net_wm_window_type_combo    = get_atom_id("_NET_WM_WINDOW_TYPE_COMBO", true);
            //atom_net_wm_window_opacity       = get_atom_id("_NET_WM_WINDOW_OPACITY", true);
            //atom_compton_shadow              = get_atom_id("_COMPTON_SHADOW", true); // Picom/Compton
            //atom_net_wm_ping                 = get_atom_id("_NET_WM_PING", true);
            //atom_net_wm_sync_request         = get_atom_id("_NET_WM_SYNC_REQUEST", true);
            //atom_net_wm_sync_request_counter = get_atom_id("_NET_WM_SYNC_REQUEST_COUNTER", true);
            //atom_net_wm_bypass_compositor    = get_atom_id("_NET_WM_BYPASS_COMPOSITOR", true);
            atom_utf8_string                 = get_atom_id("UTF8_STRING", true);
            atom_vtmx                        = get_atom_id("VTMX", true);

            // Server related.
            atom_atom                   = get_atom_id("ATOM",             faux);
            atom_window                 = get_atom_id("WINDOW",           faux);
            atom_cardinal               = get_atom_id("CARDINAL",         faux);
            atom_wm_transient_for       = get_atom_id("WM_TRANSIENT_FOR", faux);
            atom_wm_hints               = get_atom_id("WM_HINTS",         faux);
            atom_wm_normal_hints        = get_atom_id("WM_NORMAL_HINTS",  faux);
            atom_wm_size_hints          = get_atom_id("WM_SIZE_HINTS",    faux);
            atom_net_workarea           = get_atom_id("_NET_WORKAREA",    faux);
            atom_wm_protocols           = get_atom_id("WM_PROTOCOLS",            true);
            atom_wm_delete_window       = get_atom_id("WM_DELETE_WINDOW",        true);
            atom_net_active_window      = get_atom_id("_NET_ACTIVE_WINDOW",      true);
            atom_net_number_of_desktops = get_atom_id("_NET_NUMBER_OF_DESKTOPS", true);
            atom_net_current_desktop    = get_atom_id("_NET_CURRENT_DESKTOP",    true);
            atom_xkb_rules_names        = get_atom_id("_XKB_RULES_NAMES",        true);
            return true;
        }
        auto get_props()
        {
            auto ok = true;
            if (atom_net_workarea)
            {
                sendrq<x11::req::get_property>({ .window_id   = root_window_id,
                                                 .property    = atom_net_workarea,
                                                 .prop_type   = atom_cardinal,
                                                 .long_length = 4 });
                auto reply = x11::req::get_property::reply{};
                if (x11connection->recv_all((char*)&reply, sizeof(reply)).size() == 32 && reply.type == x11::event::Reply && reply.prop_type == atom_cardinal)
                {
                    //todo unify
                    auto payload_size = reply.length * 4;
                    auto buffer = text(payload_size, '\0');
                    if (x11connection->recv_all(buffer.data(), buffer.size()).size() != buffer.size()) return faux;
                    auto ptr = (si32*)buffer.data();
                    auto x = netxs::start_lifetime_as<si32>(ptr + 0);
                    auto y = netxs::start_lifetime_as<si32>(ptr + 1);
                    auto w = netxs::start_lifetime_as<si32>(ptr + 2);
                    auto h = netxs::start_lifetime_as<si32>(ptr + 3);
                    workarea = rect{{ x, y }, { w, h }};
                    if constexpr (debugmode) log("%%Received property for atom='_NET_WORKAREA' value=", prompt::x11, workarea);
                }
                else ok = faux;
            }
            return ok;
        }
        auto get_atom_name(ui32 atom)
        {
            auto atom_name = text{};
            auto lock = std::lock_guard{ sync_mutex };
            auto seq_num = syncrq(sync_buffer, x11::req::get_atom_name{ .atom = atom });
            if constexpr (debugmode) log("get_atom_name: atom=0x%% seq=%%", utf::to_hex(atom), seq_num);
            sync_x11connection->send(sync_buffer);
            auto ev = x11::event::any{};
            while (sync_x11connection->recv_all((char*)&ev, sizeof(ev)).size() == sizeof(ev))
            {
                auto type = ev.type & 0x7f;
                if (type == x11::event::Error)
                {
                    if constexpr (debugmode) log(ansi::err("get_atom_name: seq=%% error: %%", ev.sequence, get_error(ev)));
                }
                else if ((type == x11::event::Reply || type == x11::event::GenericEvent) && ev.length)
                {
                    sync_buffer.assign(ev.length * 4, '\0');
                    if (sync_x11connection->recv_all(sync_buffer.data(), sync_buffer.size()).size() == sync_buffer.size())
                    if (ev.sequence == seq_num)
                    {
                        auto reply = netxs::start_lifetime_as<x11::req::get_atom_name::reply>(ev);
                        atom_name = text{ sync_buffer.data(), reply.name_len };
                    }
                }
                if (ev.sequence == seq_num) break;
            }
            sync_buffer.clear();
            return atom_name;
        }
        auto enable_detectable_autorepeat()
        {
            sendrq<x11::req::xkb::per_client_flags>({ .major_opcode = xkb_major_opcode,
                                                      .device_spec  = x11::req::xkb::UseCoreKbd,
                                                      .change_mask  = x11::req::xkb::DetectableAutoRepeatMask,
                                                      .value        = x11::req::xkb::DetectableAutoRepeat });
            auto reply = x11::req::xkb::per_client_flags::reply{};
            if (x11connection->recv_all((char*)&reply, sizeof(reply)).size() == sizeof(reply) && reply.type == x11::event::Reply)
            {
                if constexpr (debugmode)
                {
                    auto detectable_auto_repeat_state = (reply.value & x11::req::xkb::DetectableAutoRepeatMask) != 0;
                    log("detectable_auto_repeat_state = ", detectable_auto_repeat_state);
                }
            }
            return true;
        }
        auto get_default_window_area()
        {
            auto default_area = workarea ? workarea : rect{ dot_00, { roots.front().s.width_in_pixels, roots.front().s.height_in_pixels }};
            default_window_area.coor = default_area.coor + default_area.size * 1 / 8;
            default_window_area.size = default_area.size * 3 / 4;
            //todo it doesn't work
            // Create default window.
            //auto new_window_id = new_resource_id();
            //default_window_area = rect{ dot_00, (workarea ? workarea.size : twod{ roots.front().s.width_in_pixels, roots.front().s.height_in_pixels }) * 3 / 4 };
            //if constexpr (debugmode) log("Requested win area: %%", default_window_area);
            //sendrq<x11::req::create_window>({ .window_id = new_window_id,
            //                                  .parent_id = root_window_id,
            //                                  .x         = (si16)default_window_area.coor.x,
            //                                  .y         = (si16)default_window_area.coor.y,
            //                                  .width     = (ui16)default_window_area.size.x,
            //                                  .height    = (ui16)default_window_area.size.y,
            //                                  .visual_id = argb_visual32_id },
            //                                x11::req::create_window::payload{ .border_pixel = 0x00'000000u, // Own 32-bit ARGB border pixel value.
            //                                                                  .override_redirect = 0,       // 1: On.
            //                                                                  .event_mask        = x11::event::mask::StructureNotify, // To force CreateNotify reply.
            //                                                                  .colormap_id       = argb_colormap_id });
            //// Disable decorations.
            //sendrq<x11::req::change_property>({ .window_id = new_window_id,
            //                                    .property  = atom_motif_wm_hints,   // Atom "_MOTIF_WM_HINTS".
            //                                    .type      = atom_motif_wm_hints }, // Atom "_MOTIF_WM_HINTS".
            //                                x11::motif::hints{ .flags = x11::motif::Decorations, .decorations = 0 });
            //// Set WM hints.
            ////sendrq<x11::req::change_property>({ .window_id = new_window_id,
            ////                                    .property  = atom_wm_normal_hints, // WM_NORMAL_HINTS
            ////                                    .type      = atom_wm_size_hints }, // WM_HINTS.
            ////                                x11::icccm::wm_size_hints{}); // Empty hints: request window position and size from WM.???
            ////                                //x11::icccm::wm_size_hints{ .flags = x11::icccm::wm_size_hints::USPosition
            ////                                //                                  | x11::icccm::wm_size_hints::USSize
            ////                                //                                  | x11::icccm::wm_size_hints::PMaxSize
            ////                                //                                  | x11::icccm::wm_size_hints::PBaseSize });
            //sendrq(x11::req::map_window{ .window_id = new_window_id });
            //auto buffer = std::array<char, 32>{};
            //while (x11connection->recv_all(buffer.data(), buffer.size()).size() == 32) // Wait for ConfigureNotify.
            //{
            //    auto ev = netxs::start_lifetime_as<x11::event::any>(buffer.data());
            //    if constexpr (debugmode) log("Next event: %%", event_str(ev.type));
            //    if (ev.type == x11::event::Error)
            //    {
            //        if constexpr (debugmode) log("Request error: %%", get_error(ev));
            //        break;
            //    }
            //    else if (ev.type == x11::event::ConfigureNotify)
            //    {
            //        auto cn = netxs::start_lifetime_as<x11::event::configure_notify>(buffer.data());
            //        default_window_area = rect{{ cn.x, cn.y }, { cn.width, cn.height }};
            //        if constexpr (debugmode) log("Default window area=%%", default_window_area);
            //        break;
            //    }
            //    else if (ev.type == x11::event::MapNotify)
            //    {
            //        if constexpr (debugmode) log("Window mapped: area=%%", default_window_area);
            //        break;
            //    }
            //}
            //sendrq(x11::req::unmap_window{ .window_id = new_window_id });
            //sendrq(x11::req::destroy_window{ .window_id = new_window_id });
            //while (x11connection->recv_all(buffer.data(), buffer.size()).size() == 32) // Cleanup.
            //{
            //    auto ev = netxs::start_lifetime_as<x11::event::any>(buffer.data());
            //    if constexpr (debugmode) log("Next event (cleanup stage): %%", event_str(ev.type));
            //    if (ev.type == x11::event::Error || ev.type == x11::event::DestroyNotify) break;
            //}
            //free_resource_id(new_window_id);
            return true;
        }
        bool create_shared_objects()
        {
            empty_region_id = new_resource_id();
            sendrq<x11::req::xfixes::create_region>({ .major_opcode = xfixes_major_opcode,
                                                      .region_id    = empty_region_id });
            //sendrq<x11::req::xfixes::destroy_region>({ .major_opcode = xfixes_major_opcode,
            //                                           .region_id    = empty_region_id });
            //free_resource_id(empty_region_id);

            //xsync_counter_id = new_resource_id();
            //sendrq<x11::req::xsync::create_counter>({ .major_opcode = xsync_major_opcode,
            //                                          .counter_id   = xsync_counter_id });
            //xsync_fence_id = new_resource_id();
            //if constexpr (debugmode) log("Allocated fence_id: 0x%%", utf::to_hex(xsync_fence_id));

            //virtual_pixmap_id = new_resource_id();
            //sendrq<x11::req::create_pixmap>({ .pixmap_id   = virtual_pixmap_id,
            //                                  .drawable_id = root_window_id,
            //                                  .width       = 1,
            //                                  .height      = 1 });
            return true;
        }
        auto listen_root_events() // Subscribe on root's property change (to track some desktop window has received focus).
        {
            //sendrq(x11::req::xkb::select_events{ .major_opcode = xkb_major_opcode }); // Subscribe on keyboard device events.
            sendrq<x11::req::change_window_attrs>({ .window_id = root_window_id },
                x11::req::change_window_attrs::payload{ .event_mask = x11::event::mask::PropertyChange      // Track global input focus changes.
                                                                    | x11::event::mask::StructureNotify }); // Track root window size changes.
        }
        auto sync_device_classes(ui16 num_classes, view& q)
        {
            for (auto c = 0u; c < num_classes; ++c)
            {
                if (q.size() >= sizeof(x11::req::xi2::event::device_changed::any_class))
                if (auto any_cls = netxs::start_lifetime_as<x11::req::xi2::event::device_changed::any_class>(q.data()); q.size() >= any_cls.length * 4)
                {
                    if (any_cls.type == x11::req::xi2::event::device_changed::KeyClass)
                    {
                        auto key_cls = netxs::start_lifetime_as<x11::req::xi2::event::device_changed::key_class>(q.data());
                        auto& dev = input_devices[key_cls.sourceid];
                        auto min_keycode = 0xFFFFFFFFu;
                        auto max_keycode = 0u;
                        auto keys_data_ptr = q.data() + sizeof(x11::req::xi2::event::device_changed::key_class);
                        if (q.size() >= sizeof(x11::req::xi2::event::device_changed::key_class) + (key_cls.num_keys * sizeof(ui32)))
                        {
                            for (auto k = 0u; k < key_cls.num_keys; ++k)
                            {
                                auto keycode = 0u;
                                std::memcpy(&keycode, keys_data_ptr + (k * sizeof(ui32)), sizeof(ui32));
                                //if constexpr (debugmode) log<faux>("key%%=%% ", k, keycode);
                                if (keycode)
                                {
                                    if (keycode < min_keycode) min_keycode = keycode;
                                    if (keycode > max_keycode) max_keycode = keycode;
                                }
                            }
                            dev.min_keycode = min_keycode;
                            dev.max_keycode = max_keycode;
                        }
                        //if constexpr (debugmode) log("\t  Key Class: dev_id=%% '%%' num_keys: %% (Range: %%-%%)", key_cls.sourceid, dev.name,
                        //        key_cls.num_keys, min_keycode == 0xFFFFFFFF ? 0 : min_keycode, max_keycode);
                    }
                    else if (any_cls.type == x11::req::xi2::event::device_changed::ButtonClass)
                    {
                        auto btn_cls = netxs::start_lifetime_as<x11::req::xi2::event::device_changed::button_class>(q.data());
                        auto& dev = input_devices[btn_cls.sourceid];
                        //if constexpr (debugmode) log("\t  Button Class: dev_id=%% '%%' num_buttons: %%", btn_cls.sourceid, dev.name, btn_cls.num_buttons);
                        auto mask_words = ((size_t)btn_cls.num_buttons + 31) / 32;
                        auto dynamic_payload_bytes = (mask_words * sizeof(ui32)) + (btn_cls.num_buttons * sizeof(ui32));
                        if (q.size() >= sizeof(x11::req::xi2::event::device_changed::button_class) + dynamic_payload_bytes)
                        {
                            auto state_ptr = btn_cls.state_mask_ptr(q.data());
                            dev.pressed_buttons.reserve(mask_words);
                            for (auto word_idx = 0u; word_idx < mask_words; ++word_idx)
                            {
                                auto mask_word = netxs::start_lifetime_as<ui32>(state_ptr + word_idx);
                                dev.pressed_buttons.push_back(mask_word);
                            }
                            //if constexpr (debugmode)
                            //{
                            //    auto labels_ptr = btn_cls.labels_ptr(q.data());
                            //    for (auto b = 0u; b < btn_cls.num_buttons; ++b)
                            //    {
                            //        auto button_atom = netxs::start_lifetime_as<ui32>(labels_ptr + b);
                            //        auto word_idx = b / 32;
                            //        auto bit_idx = b % 32;
                            //        auto mask_word = netxs::start_lifetime_as<ui32>(state_ptr + word_idx);
                            //        auto is_pressed = (mask_word & (1u << bit_idx)) != 0;
                            //        log("\t    Button #%% Atom ID: %% [%%]",
                            //                b + 1, button_atom, is_pressed ? "Pressed" : "Released");
                            //    }
                            //}
                        }
                    }
                    else if (any_cls.type == x11::req::xi2::event::device_changed::ValuatorClass)
                    {
                        auto val_cls = netxs::start_lifetime_as<x11::req::xi2::event::device_changed::valuator_class>(q.data());
                        auto& dev = input_devices[val_cls.sourceid];
                        if (dev.axes.size() <= val_cls.number) dev.axes.resize(val_cls.number + 1);
                        auto& axis = dev.axes[val_cls.number];
                        axis.last_val = val_cls.value.to_fp64();
                        axis.min_max  = { val_cls.min.to_fp64(), val_cls.max.to_fp64() };
                        axis.is_abs   = !val_cls.mode;
                        axis.dpi      = val_cls.resolution;
                        //if constexpr (debugmode) log("\t  Valuator Axis: dev_id=%% '%%' axis: %% min_max: %% last_val: %% dpi: %% mode: %%", val_cls.sourceid, dev.name, val_cls.number, axis.min_max, axis.last_val, val_cls.resolution, val_cls.mode ? "Relative":"Absolute");
                    }
                    else if (any_cls.type == x11::req::xi2::event::device_changed::ScrollClass)
                    {
                        auto scr_cls = netxs::start_lifetime_as<x11::req::xi2::event::device_changed::scroll_class>(q.data());
                        auto& dev = input_devices[scr_cls.sourceid];
                        if (dev.axes.size() <= scr_cls.number) dev.axes.resize(scr_cls.number + 1);
                        auto& axis = dev.axes[scr_cls.number];
                        axis.is_scroll = true;
                        axis.vertical  = scr_cls.scroll_type == x11::req::xi2::event::device_changed::scroll_class::Vertical;
                        axis.inc_step  = scr_cls.inc_step.to_fp64();
                        //if constexpr (debugmode) log("\t  Scroll Axis: dev_id=%% '%%' axis: %% type: %% step: %%", scr_cls.sourceid, dev.name, scr_cls.number, axis.vertical ? "Vertical" : "Horizontal", axis.inc_step);
                    }
                    else if (any_cls.type == x11::req::xi2::event::device_changed::TouchClass)
                    {
                        auto tch_cls = netxs::start_lifetime_as<x11::req::xi2::event::device_changed::touch_class>(q.data());
                        auto& dev = input_devices[tch_cls.sourceid];
                        dev.touch_mode  = tch_cls.mode;
                        dev.num_touches = tch_cls.num_touches;
                        //if constexpr (debugmode) log("\t  Touch Class: dev_id=%% '%%' mode: '%%' num_touches: %%", tch_cls.sourceid, dev.name, tch_cls.mode ? "touchpad":"touchscreen", tch_cls.num_touches);
                    }
                    q.remove_prefix(any_cls.length * 4);
                    continue;
                }
                log("%%Error: Unexpected buffer end", prompt::x11);
                q = {};
                break;
            }
        }
        auto query_device(ui16 device_id)
        {
            sendrq(x11::req::xi2::query_device{ .major_opcode = xi2_major_opcode,
                                                .device_id    = device_id }, {},
                [&](auto& ev, qiew q)
                {
                    if (ev.type == x11::event::Error)
                    {
                        log("%%Failed to query devices: %%", prompt::x11, get_error(ev));
                    }
                    else
                    {
                        auto reply = netxs::start_lifetime_as<x11::req::xi2::query_device::reply>(q.data());
                        q.remove_prefix(sizeof(ev));
                        if constexpr (debugmode) log("Connected input devices: %%", reply.num_devices);
                        if (reply.num_devices)
                        while (q)
                        {
                            if (q.size() < sizeof(x11::req::xi2::query_device::reply::device_info))
                            {
                                log("%%Error: Broken device list", prompt::x11);
                                break;
                            }
                            auto dev = netxs::start_lifetime_as<x11::req::xi2::query_device::reply::device_info>(q.data());
                            auto name_padded_len = ((size_t)dev.name_len + 3) & ~3;
                            auto total_dev_header_len = sizeof(x11::req::xi2::query_device::reply::device_info) + name_padded_len;
                            if (q.size() < total_dev_header_len)
                            {
                                log("%%Error: Wrong device name length (buffer_size=%% name_length=%% padded_length=%%)", prompt::x11, q.size(), dev.name_len, name_padded_len);
                                break;
                            }
                            q.remove_prefix(sizeof(x11::req::xi2::query_device::reply::device_info));
                            auto name = q.substr(0, dev.name_len);
                            q.remove_prefix(name_padded_len);
                            auto& target_dev = input_devices[dev.deviceid];
                            target_dev.enabled   = dev.enabled;
                            target_dev.name      = name;
                            target_dev.is_master = dev.use == x11::req::xi2::MasterPointer
                                                || dev.use == x11::req::xi2::MasterKeyboard;
                            if constexpr (debugmode) log("\tDeviceID=%% '%%' Classes=%% is_master=%%", dev.deviceid, name, dev.num_classes, target_dev.is_master);
                            sync_device_classes(dev.num_classes, q);
                        }
                    }
                });
        }
        void activate_xinput2(arch master_window_id)
        {
            auto event_mask_bits = 0u;
            event_mask_bits |= 1u << x11::req::xi2::event::KeyPress;
            event_mask_bits |= 1u << x11::req::xi2::event::KeyRelease;
            event_mask_bits |= 1u << x11::req::xi2::event::ButtonPress;
            event_mask_bits |= 1u << x11::req::xi2::event::ButtonRelease;
            event_mask_bits |= 1u << x11::req::xi2::event::Motion;
            event_mask_bits |= 1u << x11::req::xi2::event::Enter;
            event_mask_bits |= 1u << x11::req::xi2::event::Leave;
            event_mask_bits |= 1u << x11::req::xi2::event::FocusIn;
            event_mask_bits |= 1u << x11::req::xi2::event::FocusOut;
            event_mask_bits |= 1u << x11::req::xi2::event::DeviceChanged;
            //event_mask_bits |= 1u << x11::req::xi2::event::PropertyEvent;
            event_mask_bits |= 1u << x11::req::xi2::event::HierarchyChanged;
            sendrq(x11::req::xi2::select_events{ .major_opcode = xi2_major_opcode,
                                                 .window_id    = (ui32)master_window_id, },
                                            x11::req::xi2::select_events::payload
                                            {
                                                .deviceid = x11::req::xi2::dev_type::all_devices,
                                                .mask_len = 2,
                                                .mask1    = event_mask_bits,
                                            });
        }
        void set_x11_display_size(twod size)
        {
            x11_display_size = size;
            x11_diagonal = size.x * size.x + size.y * size.y;
        }
        auto open_sync_connection(view x11unixpath, view auth_packet)
        {
            if (auto link = os::ipc::socket::connect(x11unixpath))
            {
                link->send(auth_packet);
                auto reply = x11::session_t::auth::reply{};
                if (link->recv_all((char*)&reply, sizeof(reply)).size() == sizeof(reply))
                {
                    auto buffer = text(reply.additional_length * 4, '\0');
                    link->recv_all(buffer.data(), buffer.size());
                    if (reply.status == x11::event::Reply)
                    {
                        auto sync_session_state = netxs::start_lifetime_as<x11::session_t::session_init>(buffer.data());
                        sync_msg_window_id = sync_session_state.resource_id_base;
                        sync_x11connection = link;
                        // Create invisible window for sync receiving messages.
                        if constexpr (debugmode) log("Create invisible window: id=0x%% seq=%%", utf::to_hex(sync_msg_window_id), sync_sequence_counter + (ui16)1);
                        syncrq(x11::req::create_window{ .window_id = sync_msg_window_id,
                                                        .parent_id = root_window_id });
                        // Mark our windows.
                        //syncrq<x11::req::change_property>({ .window_id = sync_msg_window_id,
                        //                                    .property  = atom_vtmx,
                        //                                    .type      = atom_cardinal }, 1);
                        // Register as a parallel XKB client.
                        syncrq(x11::req::xkb::query_version{ .major_opcode = xkb_major_opcode });
                        buffer.resize(x11::recv_packet_size);
                        if (sync_x11connection->recv_all(buffer.data(), x11::recv_packet_size).size() != x11::recv_packet_size)
                        {
                            if constexpr (debugmode) log(ansi::err("Unexpected error while XKB activation"));
                        }
                    }
                }
            }
            if constexpr (debugmode) log("Sync X11 connection: ", sync_x11connection.get());
            return !!sync_x11connection;
        }
    };

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
            auth_path = home_env + "/.Xauthority";
        }
        if (auth_path.size())
        if (auto fs = std::ifstream{ auth_path, std::ios::binary }; fs.is_open())
        {
            auto read_ui16be = [&]
            {
                auto bytes = text(2, '\0');
                return fs.read(bytes.data(), bytes.size()) ? ((ui16)(byte)bytes[0] << 8) | (byte)bytes[1] : ui16{};
            };
            auto read_string = [&](ui16 length)
            {
                auto string = text(length, '\0');
                if (length > 0) fs.read(string.data(), length);
                return string;
            };
            while (fs.peek() != EOF)
            {
[[maybe_unused]]auto family   = read_ui16be(); // family = 256 (FamilyLocal).
                auto addr_len = read_ui16be();
[[maybe_unused]]auto addr_str = read_string(addr_len);
                auto disp_len = read_ui16be();
                auto disp_str = read_string(disp_len);
                auto name_len = read_ui16be();
                auto name_str = read_string(name_len);
                auto data_len = read_ui16be();
                auto data_str = read_string(data_len);
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
    auto build_auth_packet(auto& cookie_data)
    {
        auto req = x11::session_t::auth{ .byte_order     = netxs::endian_LE ? 'l' : 'B',
                                         .major_version  = 11,
                                         .minor_version  = 0,
                                         .auth_proto_len = (ui16)cookie_data.auth_name.size(),
                                         .auth_data_len  = (ui16)cookie_data.auth_data.size() };
        auto auth_name_padded_len = (req.auth_proto_len + 3) & ~3; // Rounding up to a multiple of 4.
        auto auth_data_padded_len = (req.auth_data_len  + 3) & ~3; //
        auto packet = text(sizeof(req) + auth_name_padded_len + auth_data_padded_len, '\0');
        std::memcpy(packet.data(), &req, sizeof(req));
        std::memcpy(packet.data() + sizeof(req), cookie_data.auth_name.data(), cookie_data.auth_name.size());
        std::memcpy(packet.data() + sizeof(req) + auth_name_padded_len, cookie_data.auth_data.data(), cookie_data.auth_data.size());
        return packet;
    }
    auto parse_auth_reply(auto x11connection)
    {
        auto session_ptr = ptr::shared<x11::session_t>();
        auto& session = *session_ptr;
        auto reply = x11::session_t::auth::reply{};
        if (auto l1 = x11connection->recv_all((char*)&reply, sizeof(reply)); l1.size() == sizeof(reply))
        {
            auto remaining_bytes = (size_t)reply.additional_length * 4;
            auto buffer = text(remaining_bytes, '\0');
            if (reply.status == x11::event::Error)
            {
                log("%%Connection rejected: '%%'", prompt::x11, utf::debase<faux, faux>(x11connection->recv_all(buffer.data(), buffer.size())));
            }
            else if (reply.status != x11::event::Reply)
            {
                log("%%Unknown response status", prompt::x11);
            }
            else if (auto l3 = x11connection->recv_all(buffer.data(), buffer.size()); l3.size() != buffer.size())
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
                if (failed) session_ptr.reset();
                else
                {
                    session.x11connection = x11connection;
                    if (session.roots.size())
                    {
                        session.root_window_id = session.roots.front().s.root_window_id;
                    }
                }
            }
        }
        else
        {
            log("%%Error reading connection reply", prompt::x11);
        }
        return session_ptr;
    }
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
            auto auth_packet = x11::build_auth_packet(cookie_data);
            socket_link->send(auth_packet);
            if (auto session_ptr = x11::parse_auth_reply(socket_link))
            {
                auto& session = *session_ptr;
                if (session.detect_argb_32bit())
                if (session.detect_extension<x11::req::shm     ::query_version>("MIT-SHM"        , session.shm_major_opcode, session.shm_completion_event, 1, 2)) // MIT-SHM ver >= 1.2
                if (session.detect_extension<x11::req::xfixes  ::query_version>("XFIXES"         , session.xfixes_major_opcode, session.xfixes_first_event, 2, 0)) // XFIXES ver >= 2.0
                if (session.detect_extension<x11::req::xi2     ::query_version>("XInputExtension", session.xi2_major_opcode, byte{}, 2, 4)) // XInputExtension (XInput2) ver >= 2.4
                if (session.detect_extension<x11::req::xkb     ::query_version>("XKEYBOARD"      , session.xkb_major_opcode, session.xkb_first_event, 1, 0))
                //if (session.detect_extension<x11::req::xsync   ::query_version>("SYNC"           , session.xsync_major_opcode, byte{}, 3, 1))
                if (session.detect_extension<x11::req::xpresent::query_version>("Present"        , session.xpresent_major_opcode, byte{}, 1, 0))
                if (session.get_atoms())
                if (session.get_props())
                if (session.enable_detectable_autorepeat())
                if (session.get_default_window_area())
                if (session.create_shared_objects())
                if (session.open_sync_connection(x11unixpath, auth_packet))
                {
                    if constexpr (debugmode) log(session.str());
                    session.wl_present = !!os::env::get("WAYLAND_DISPLAY").size();
                    auto& x11screen = session.roots.front().s;
                    session.set_x11_display_size(twod{ x11screen.width_in_pixels, x11screen.height_in_pixels });
                    auto max_grid_size = x11screen.width_in_pixels * x11screen.height_in_pixels;
                    auto required_buffer_size = 2 * 3 * max_grid_size * sizeof(argb); // 2: Double buffer, 3: master+blinks+header/footer/tooltip.
                    if (session.resize_shared_buffer(required_buffer_size))
                    {
                        x11::session_ptr = session_ptr;
                    }
                }
            }
        }
        return !!x11::session_ptr;
    }

    namespace key
    {
        #define key_list \
           /* VKeyName  XKeyName          KeySym */\
            X(lbutton , Pointer_Button1 , 0xfee9) /* VK_LBUTTON */\
            X(altgr   , ISO_Level3_Shift, 0xfe03) \
            X(grselect, ISO_Level5_Shift, 0xfe11) /* VK_OEM_8 GroupSelect (Level5Shift) on Canadian layout */\
            X(numlock , Num_Lock        , 0xff7f) /* VK_NUMLOCK  */\
            X(capslock, Caps_Lock       , 0xffe5) /* VK_CAPITAL  */\
            X(scrllock, Scroll_Lock     , 0xff14) /* VK_SCROLL   */\
            X(lshift  , Shift_L         , 0xffe1) /* VK_LSHIFT   */\
            X(rshift  , Shift_R         , 0xffe2) /* VK_RSHIFT   */\
            X(lctrl   , Control_L       , 0xffe3) /* VK_LCONTROL */\
            X(rctrl   , Control_R       , 0xffe4) /* VK_RCONTROL */\
            X(lalt    , Alt_L           , 0xffe9) /* VK_LMENU    */\
            X(ralt    , Alt_R           , 0xffea) /* VK_RMENU    */\
            X(lsuper  , Super_L         , 0xffeb) /* VK_LWIN     */\
            X(rsuper  , Super_R         , 0xffec) /* VK_RWIN     */\
            X(undef   , Hyper_L         , 0xffed) /* VK_LWIN     */\
            X(undef   , Hyper_R         , 0xffee) /* VK_RWIN     */\
            X(prntscrn, Print           , 0xff61) /* VK_SNAPSHOT (SysReq, Alt+PrntScrn) */ \
            X(cancel  , Cancel          , 0xff69) /* VK_CANCEL (Break, Ctrl+Pause) */ \
            X(clear   , Clear           , 0xff0b) /* VK_CLEAR    */ \
            X(enter   , Return          , 0xff0d) /* VK_RETURN   */ \
            X(pgup    , Prior           , 0xff55) /* VK_PRIOR    */ \
            X(pgdn    , Next            , 0xff56) /* VK_NEXT     */ \
            X(end     , End             , 0xff57) /* VK_END      */ \
            X(home    , Home            , 0xff50) /* VK_HOME     */ \
            X(left    , Left            , 0xff51) /* VK_LEFT     */ \
            X(up      , Up              , 0xff52) /* VK_UP       */ \
            X(right   , Right           , 0xff53) /* VK_RIGHT    */ \
            X(down    , Down            , 0xff54) /* VK_DOWN     */ \
            X(insert  , Insert          , 0xff63) /* VK_INSERT   */ \
            X(del     , Delete          , 0xffff) /* VK_DELETE   */ \
            X(f11     , F11             , 0xffc8) /* VK_F11      */ \
            X(f12     , F12             , 0xffc9) /* VK_F12      */ \
            X(key_0   , 0               , 0x0030) /* VK_0        */ \
            X(numpad0 , KP_0            , 0xffb0) /* VK_NUMPAD0  */ \
            X(numpad1 , KP_1            , 0xffb1) /* VK_NUMPAD1  */ \
            X(numpad2 , KP_2            , 0xffb2) /* VK_NUMPAD2  */ \
            X(numpad3 , KP_3            , 0xffb3) /* VK_NUMPAD3  */ \
            X(numpad4 , KP_4            , 0xffb4) /* VK_NUMPAD4  */ \
            X(numpad5 , KP_5            , 0xffb5) /* VK_NUMPAD5  */ \
            X(numpad6 , KP_6            , 0xffb6) /* VK_NUMPAD6  */ \
            X(numpad7 , KP_7            , 0xffb7) /* VK_NUMPAD7  */ \
            X(numpad8 , KP_8            , 0xffb8) /* VK_NUMPAD8  */ \
            X(numpad9 , KP_9            , 0xffb9) /* VK_NUMPAD9  */ \
            X(numpadD , KP_Decimal      , 0xffae) /* VK_DECIMAL  */
        // Sync with input::vkey::* from input.hpp.
        constexpr byte keysym_to_vkey(ui32 keysym)
        {
            auto vk = (byte)0;
            switch (keysym)
            {
                #define X(VKeyName, XKeyName, KeySym) case KeySym: vk = input::vkey::VKeyName; break;
                    key_list
                #undef X
                #undef key_list
            }
            return vk;
        }
        static constexpr auto _symdef = []
        {
            struct key_t
            {
                view name;
                ui32 keysym;
                ui16 uc;
            };
            auto m = std::to_array<key_t>(
            {
                // Auto generated 15.09.2026 00:55 from https://gitlab.freedesktop.org/xorg/proto/xorgproto/-/blob/master/include/X11/keysymdef.h
                // awk '/^[ \t]*#[ \t]*define[ \t]+XK_/ && $3 ~ /^0x[0-9a-fA-F]+/ { name=$2; sub(/^XK_/, "", name); keysym=tolower($3); codepoint="0"; for(i=4; i<=NF; i++) { if($i ~ /U\+[0-9A-Fa-f]+/) { codepoint=$i; gsub(/[^0-9A-Fa-f]/, "", codepoint); codepoint="0x" tolower(codepoint); break } } qname = "\"" name "\""; rec = sprintf("{ %-29s, %-9s, %-6s }", qname, keysym, codepoint); if (line == "") line = "            " rec; else line = line ", " rec; count++; if (count % 30 == 0) { print line ","; line = "" } } END { if (line != "") print line }' keysymdef.h
                //1                                                     2                                                     3                                                     4                                                     5                                                     6                                                     7                                                     8                                                     9                                                     10                                                    11                                                    12                                                    13                                                    14                                                    15                                                    16                                                    17                                                    18                                                    19                                                    20                                                    21                                                    22                                                    23                                                    24                                                    25                                                    26                                                    27                                                    28                                                    29                                                    30
                { "VoidSymbol"                 , 0xffffff , 0      }, { "BackSpace"                  , 0xff08   , 0x0008 }, { "Tab"                        , 0xff09   , 0x0009 }, { "Linefeed"                   , 0xff0a   , 0x000a }, { "Clear"                      , 0xff0b   , 0x000b }, { "Return"                     , 0xff0d   , 0x000d }, { "Pause"                      , 0xff13   , 0      }, { "Scroll_Lock"                , 0xff14   , 0      }, { "Sys_Req"                    , 0xff15   , 0      }, { "Escape"                     , 0xff1b   , 0x001b }, { "Delete"                     , 0xffff   , 0x007f }, { "Multi_key"                  , 0xff20   , 0      }, { "Codeinput"                  , 0xff37   , 0      }, { "SingleCandidate"            , 0xff3c   , 0      }, { "MultipleCandidate"          , 0xff3d   , 0      }, { "PreviousCandidate"          , 0xff3e   , 0      }, { "Kanji"                      , 0xff21   , 0      }, { "Muhenkan"                   , 0xff22   , 0      }, { "Henkan_Mode"                , 0xff23   , 0      }, { "Henkan"                     , 0xff23   , 0      }, { "Romaji"                     , 0xff24   , 0      }, { "Hiragana"                   , 0xff25   , 0      }, { "Katakana"                   , 0xff26   , 0      }, { "Hiragana_Katakana"          , 0xff27   , 0      }, { "Zenkaku"                    , 0xff28   , 0      }, { "Hankaku"                    , 0xff29   , 0      }, { "Zenkaku_Hankaku"            , 0xff2a   , 0      }, { "Touroku"                    , 0xff2b   , 0      }, { "Massyo"                     , 0xff2c   , 0      }, { "Kana_Lock"                  , 0xff2d   , 0      },
                { "Kana_Shift"                 , 0xff2e   , 0      }, { "Eisu_Shift"                 , 0xff2f   , 0      }, { "Eisu_toggle"                , 0xff30   , 0      }, { "Kanji_Bangou"               , 0xff37   , 0      }, { "Zen_Koho"                   , 0xff3d   , 0      }, { "Mae_Koho"                   , 0xff3e   , 0      }, { "Home"                       , 0xff50   , 0      }, { "Left"                       , 0xff51   , 0      }, { "Up"                         , 0xff52   , 0      }, { "Right"                      , 0xff53   , 0      }, { "Down"                       , 0xff54   , 0      }, { "Prior"                      , 0xff55   , 0      }, { "Page_Up"                    , 0xff55   , 0      }, { "Next"                       , 0xff56   , 0      }, { "Page_Down"                  , 0xff56   , 0      }, { "End"                        , 0xff57   , 0      }, { "Begin"                      , 0xff58   , 0      }, { "Select"                     , 0xff60   , 0      }, { "Print"                      , 0xff61   , 0      }, { "Execute"                    , 0xff62   , 0      }, { "Insert"                     , 0xff63   , 0      }, { "Undo"                       , 0xff65   , 0      }, { "Redo"                       , 0xff66   , 0      }, { "Menu"                       , 0xff67   , 0      }, { "Find"                       , 0xff68   , 0      }, { "Cancel"                     , 0xff69   , 0      }, { "Help"                       , 0xff6a   , 0      }, { "Break"                      , 0xff6b   , 0      }, { "ISO_Group_Shift"            , 0xff7e   , 0      }, { "Mode_switch"                , 0xff7e   , 0      },
                { "script_switch"              , 0xff7e   , 0      }, { "Num_Lock"                   , 0xff7f   , 0      }, { "KP_Space"                   , 0xff80   , 0x0020 }, { "KP_Tab"                     , 0xff89   , 0x0009 }, { "KP_Enter"                   , 0xff8d   , 0x000d }, { "KP_F1"                      , 0xff91   , 0      }, { "KP_F2"                      , 0xff92   , 0      }, { "KP_F3"                      , 0xff93   , 0      }, { "KP_F4"                      , 0xff94   , 0      }, { "KP_Home"                    , 0xff95   , 0      }, { "KP_Left"                    , 0xff96   , 0      }, { "KP_Up"                      , 0xff97   , 0      }, { "KP_Right"                   , 0xff98   , 0      }, { "KP_Down"                    , 0xff99   , 0      }, { "KP_Prior"                   , 0xff9a   , 0      }, { "KP_Page_Up"                 , 0xff9a   , 0      }, { "KP_Next"                    , 0xff9b   , 0      }, { "KP_Page_Down"               , 0xff9b   , 0      }, { "KP_End"                     , 0xff9c   , 0      }, { "KP_Begin"                   , 0xff9d   , 0      }, { "KP_Insert"                  , 0xff9e   , 0      }, { "KP_Delete"                  , 0xff9f   , 0      }, { "KP_Equal"                   , 0xffbd   , 0x003d }, { "KP_Multiply"                , 0xffaa   , 0x002a }, { "KP_Add"                     , 0xffab   , 0x002b }, { "KP_Separator"               , 0xffac   , 0x002c }, { "KP_Subtract"                , 0xffad   , 0x002d }, { "KP_Decimal"                 , 0xffae   , 0x002e }, { "KP_Divide"                  , 0xffaf   , 0x002f }, { "KP_0"                       , 0xffb0   , 0x0030 },
                { "KP_1"                       , 0xffb1   , 0x0031 }, { "KP_2"                       , 0xffb2   , 0x0032 }, { "KP_3"                       , 0xffb3   , 0x0033 }, { "KP_4"                       , 0xffb4   , 0x0034 }, { "KP_5"                       , 0xffb5   , 0x0035 }, { "KP_6"                       , 0xffb6   , 0x0036 }, { "KP_7"                       , 0xffb7   , 0x0037 }, { "KP_8"                       , 0xffb8   , 0x0038 }, { "KP_9"                       , 0xffb9   , 0x0039 }, { "F1"                         , 0xffbe   , 0      }, { "F2"                         , 0xffbf   , 0      }, { "F3"                         , 0xffc0   , 0      }, { "F4"                         , 0xffc1   , 0      }, { "F5"                         , 0xffc2   , 0      }, { "F6"                         , 0xffc3   , 0      }, { "F7"                         , 0xffc4   , 0      }, { "F8"                         , 0xffc5   , 0      }, { "F9"                         , 0xffc6   , 0      }, { "F10"                        , 0xffc7   , 0      }, { "F11"                        , 0xffc8   , 0      }, { "L1"                         , 0xffc8   , 0      }, { "F12"                        , 0xffc9   , 0      }, { "L2"                         , 0xffc9   , 0      }, { "F13"                        , 0xffca   , 0      }, { "L3"                         , 0xffca   , 0      }, { "F14"                        , 0xffcb   , 0      }, { "L4"                         , 0xffcb   , 0      }, { "F15"                        , 0xffcc   , 0      }, { "L5"                         , 0xffcc   , 0      }, { "F16"                        , 0xffcd   , 0      },
                { "L6"                         , 0xffcd   , 0      }, { "F17"                        , 0xffce   , 0      }, { "L7"                         , 0xffce   , 0      }, { "F18"                        , 0xffcf   , 0      }, { "L8"                         , 0xffcf   , 0      }, { "F19"                        , 0xffd0   , 0      }, { "L9"                         , 0xffd0   , 0      }, { "F20"                        , 0xffd1   , 0      }, { "L10"                        , 0xffd1   , 0      }, { "F21"                        , 0xffd2   , 0      }, { "R1"                         , 0xffd2   , 0      }, { "F22"                        , 0xffd3   , 0      }, { "R2"                         , 0xffd3   , 0      }, { "F23"                        , 0xffd4   , 0      }, { "R3"                         , 0xffd4   , 0      }, { "F24"                        , 0xffd5   , 0      }, { "R4"                         , 0xffd5   , 0      }, { "F25"                        , 0xffd6   , 0      }, { "R5"                         , 0xffd6   , 0      }, { "F26"                        , 0xffd7   , 0      }, { "R6"                         , 0xffd7   , 0      }, { "F27"                        , 0xffd8   , 0      }, { "R7"                         , 0xffd8   , 0      }, { "F28"                        , 0xffd9   , 0      }, { "R8"                         , 0xffd9   , 0      }, { "F29"                        , 0xffda   , 0      }, { "R9"                         , 0xffda   , 0      }, { "F30"                        , 0xffdb   , 0      }, { "R10"                        , 0xffdb   , 0      }, { "F31"                        , 0xffdc   , 0      },
                { "R11"                        , 0xffdc   , 0      }, { "F32"                        , 0xffdd   , 0      }, { "R12"                        , 0xffdd   , 0      }, { "F33"                        , 0xffde   , 0      }, { "R13"                        , 0xffde   , 0      }, { "F34"                        , 0xffdf   , 0      }, { "R14"                        , 0xffdf   , 0      }, { "F35"                        , 0xffe0   , 0      }, { "R15"                        , 0xffe0   , 0      }, { "Shift_L"                    , 0xffe1   , 0      }, { "Shift_R"                    , 0xffe2   , 0      }, { "Control_L"                  , 0xffe3   , 0      }, { "Control_R"                  , 0xffe4   , 0      }, { "Caps_Lock"                  , 0xffe5   , 0      }, { "Shift_Lock"                 , 0xffe6   , 0      }, { "Meta_L"                     , 0xffe7   , 0      }, { "Meta_R"                     , 0xffe8   , 0      }, { "Alt_L"                      , 0xffe9   , 0      }, { "Alt_R"                      , 0xffea   , 0      }, { "Super_L"                    , 0xffeb   , 0      }, { "Super_R"                    , 0xffec   , 0      }, { "Hyper_L"                    , 0xffed   , 0      }, { "Hyper_R"                    , 0xffee   , 0      }, { "ISO_Lock"                   , 0xfe01   , 0      }, { "ISO_Level2_Latch"           , 0xfe02   , 0      }, { "ISO_Level3_Shift"           , 0xfe03   , 0      }, { "ISO_Level3_Latch"           , 0xfe04   , 0      }, { "ISO_Level3_Lock"            , 0xfe05   , 0      }, { "ISO_Level5_Shift"           , 0xfe11   , 0      }, { "ISO_Level5_Latch"           , 0xfe12   , 0      },
                { "ISO_Level5_Lock"            , 0xfe13   , 0      }, { "ISO_Group_Latch"            , 0xfe06   , 0      }, { "ISO_Group_Lock"             , 0xfe07   , 0      }, { "ISO_Next_Group"             , 0xfe08   , 0      }, { "ISO_Next_Group_Lock"        , 0xfe09   , 0      }, { "ISO_Prev_Group"             , 0xfe0a   , 0      }, { "ISO_Prev_Group_Lock"        , 0xfe0b   , 0      }, { "ISO_First_Group"            , 0xfe0c   , 0      }, { "ISO_First_Group_Lock"       , 0xfe0d   , 0      }, { "ISO_Last_Group"             , 0xfe0e   , 0      }, { "ISO_Last_Group_Lock"        , 0xfe0f   , 0      }, { "ISO_Left_Tab"               , 0xfe20   , 0      }, { "ISO_Move_Line_Up"           , 0xfe21   , 0      }, { "ISO_Move_Line_Down"         , 0xfe22   , 0      }, { "ISO_Partial_Line_Up"        , 0xfe23   , 0      }, { "ISO_Partial_Line_Down"      , 0xfe24   , 0      }, { "ISO_Partial_Space_Left"     , 0xfe25   , 0      }, { "ISO_Partial_Space_Right"    , 0xfe26   , 0      }, { "ISO_Set_Margin_Left"        , 0xfe27   , 0      }, { "ISO_Set_Margin_Right"       , 0xfe28   , 0      }, { "ISO_Release_Margin_Left"    , 0xfe29   , 0      }, { "ISO_Release_Margin_Right"   , 0xfe2a   , 0      }, { "ISO_Release_Both_Margins"   , 0xfe2b   , 0      }, { "ISO_Fast_Cursor_Left"       , 0xfe2c   , 0      }, { "ISO_Fast_Cursor_Right"      , 0xfe2d   , 0      }, { "ISO_Fast_Cursor_Up"         , 0xfe2e   , 0      }, { "ISO_Fast_Cursor_Down"       , 0xfe2f   , 0      }, { "ISO_Continuous_Underline"   , 0xfe30   , 0      }, { "ISO_Discontinuous_Underline", 0xfe31   , 0      }, { "ISO_Emphasize"              , 0xfe32   , 0      },
                { "ISO_Center_Object"          , 0xfe33   , 0      }, { "ISO_Enter"                  , 0xfe34   , 0      }, { "dead_grave"                 , 0xfe50   , 0      }, { "dead_acute"                 , 0xfe51   , 0      }, { "dead_circumflex"            , 0xfe52   , 0      }, { "dead_tilde"                 , 0xfe53   , 0      }, { "dead_perispomeni"           , 0xfe53   , 0      }, { "dead_macron"                , 0xfe54   , 0      }, { "dead_breve"                 , 0xfe55   , 0      }, { "dead_abovedot"              , 0xfe56   , 0      }, { "dead_diaeresis"             , 0xfe57   , 0      }, { "dead_abovering"             , 0xfe58   , 0      }, { "dead_doubleacute"           , 0xfe59   , 0      }, { "dead_caron"                 , 0xfe5a   , 0      }, { "dead_cedilla"               , 0xfe5b   , 0      }, { "dead_ogonek"                , 0xfe5c   , 0      }, { "dead_iota"                  , 0xfe5d   , 0      }, { "dead_voiced_sound"          , 0xfe5e   , 0      }, { "dead_semivoiced_sound"      , 0xfe5f   , 0      }, { "dead_belowdot"              , 0xfe60   , 0      }, { "dead_hook"                  , 0xfe61   , 0      }, { "dead_horn"                  , 0xfe62   , 0      }, { "dead_stroke"                , 0xfe63   , 0      }, { "dead_abovecomma"            , 0xfe64   , 0      }, { "dead_psili"                 , 0xfe64   , 0      }, { "dead_abovereversedcomma"    , 0xfe65   , 0      }, { "dead_dasia"                 , 0xfe65   , 0      }, { "dead_doublegrave"           , 0xfe66   , 0      }, { "dead_belowring"             , 0xfe67   , 0      }, { "dead_belowmacron"           , 0xfe68   , 0      },
                { "dead_belowcircumflex"       , 0xfe69   , 0      }, { "dead_belowtilde"            , 0xfe6a   , 0      }, { "dead_belowbreve"            , 0xfe6b   , 0      }, { "dead_belowdiaeresis"        , 0xfe6c   , 0      }, { "dead_invertedbreve"         , 0xfe6d   , 0      }, { "dead_belowcomma"            , 0xfe6e   , 0      }, { "dead_currency"              , 0xfe6f   , 0      }, { "dead_lowline"               , 0xfe90   , 0      }, { "dead_aboveverticalline"     , 0xfe91   , 0      }, { "dead_belowverticalline"     , 0xfe92   , 0      }, { "dead_longsolidusoverlay"    , 0xfe93   , 0      }, { "dead_a"                     , 0xfe80   , 0      }, { "dead_A"                     , 0xfe81   , 0      }, { "dead_e"                     , 0xfe82   , 0      }, { "dead_E"                     , 0xfe83   , 0      }, { "dead_i"                     , 0xfe84   , 0      }, { "dead_I"                     , 0xfe85   , 0      }, { "dead_o"                     , 0xfe86   , 0      }, { "dead_O"                     , 0xfe87   , 0      }, { "dead_u"                     , 0xfe88   , 0      }, { "dead_U"                     , 0xfe89   , 0      }, { "dead_small_schwa"           , 0xfe8a   , 0      }, { "dead_schwa"                 , 0xfe8a   , 0      }, { "dead_capital_schwa"         , 0xfe8b   , 0      }, { "dead_SCHWA"                 , 0xfe8b   , 0      }, { "dead_greek"                 , 0xfe8c   , 0      }, { "dead_hamza"                 , 0xfe8d   , 0      }, { "dead_apostrophe"            , 0xfe8e   , 0      }, { "First_Virtual_Screen"       , 0xfed0   , 0      }, { "Prev_Virtual_Screen"        , 0xfed1   , 0      },
                { "Next_Virtual_Screen"        , 0xfed2   , 0      }, { "Last_Virtual_Screen"        , 0xfed4   , 0      }, { "Terminate_Server"           , 0xfed5   , 0      }, { "AccessX_Enable"             , 0xfe70   , 0      }, { "AccessX_Feedback_Enable"    , 0xfe71   , 0      }, { "RepeatKeys_Enable"          , 0xfe72   , 0      }, { "SlowKeys_Enable"            , 0xfe73   , 0      }, { "BounceKeys_Enable"          , 0xfe74   , 0      }, { "StickyKeys_Enable"          , 0xfe75   , 0      }, { "MouseKeys_Enable"           , 0xfe76   , 0      }, { "MouseKeys_Accel_Enable"     , 0xfe77   , 0      }, { "Overlay1_Enable"            , 0xfe78   , 0      }, { "Overlay2_Enable"            , 0xfe79   , 0      }, { "AudibleBell_Enable"         , 0xfe7a   , 0      }, { "Pointer_Left"               , 0xfee0   , 0      }, { "Pointer_Right"              , 0xfee1   , 0      }, { "Pointer_Up"                 , 0xfee2   , 0      }, { "Pointer_Down"               , 0xfee3   , 0      }, { "Pointer_UpLeft"             , 0xfee4   , 0      }, { "Pointer_UpRight"            , 0xfee5   , 0      }, { "Pointer_DownLeft"           , 0xfee6   , 0      }, { "Pointer_DownRight"          , 0xfee7   , 0      }, { "Pointer_Button_Dflt"        , 0xfee8   , 0      }, { "Pointer_Button1"            , 0xfee9   , 0      }, { "Pointer_Button2"            , 0xfeea   , 0      }, { "Pointer_Button3"            , 0xfeeb   , 0      }, { "Pointer_Button4"            , 0xfeec   , 0      }, { "Pointer_Button5"            , 0xfeed   , 0      }, { "Pointer_DblClick_Dflt"      , 0xfeee   , 0      }, { "Pointer_DblClick1"          , 0xfeef   , 0      },
                { "Pointer_DblClick2"          , 0xfef0   , 0      }, { "Pointer_DblClick3"          , 0xfef1   , 0      }, { "Pointer_DblClick4"          , 0xfef2   , 0      }, { "Pointer_DblClick5"          , 0xfef3   , 0      }, { "Pointer_Drag_Dflt"          , 0xfef4   , 0      }, { "Pointer_Drag1"              , 0xfef5   , 0      }, { "Pointer_Drag2"              , 0xfef6   , 0      }, { "Pointer_Drag3"              , 0xfef7   , 0      }, { "Pointer_Drag4"              , 0xfef8   , 0      }, { "Pointer_Drag5"              , 0xfefd   , 0      }, { "Pointer_EnableKeys"         , 0xfef9   , 0      }, { "Pointer_Accelerate"         , 0xfefa   , 0      }, { "Pointer_DfltBtnNext"        , 0xfefb   , 0      }, { "Pointer_DfltBtnPrev"        , 0xfefc   , 0      }, { "ch"                         , 0xfea0   , 0      }, { "Ch"                         , 0xfea1   , 0      }, { "CH"                         , 0xfea2   , 0      }, { "c_h"                        , 0xfea3   , 0      }, { "C_h"                        , 0xfea4   , 0      }, { "C_H"                        , 0xfea5   , 0      }, { "3270_Duplicate"             , 0xfd01   , 0      }, { "3270_FieldMark"             , 0xfd02   , 0      }, { "3270_Right2"                , 0xfd03   , 0      }, { "3270_Left2"                 , 0xfd04   , 0      }, { "3270_BackTab"               , 0xfd05   , 0      }, { "3270_EraseEOF"              , 0xfd06   , 0      }, { "3270_EraseInput"            , 0xfd07   , 0      }, { "3270_Reset"                 , 0xfd08   , 0      }, { "3270_Quit"                  , 0xfd09   , 0      }, { "3270_PA1"                   , 0xfd0a   , 0      },
                { "3270_PA2"                   , 0xfd0b   , 0      }, { "3270_PA3"                   , 0xfd0c   , 0      }, { "3270_Test"                  , 0xfd0d   , 0      }, { "3270_Attn"                  , 0xfd0e   , 0      }, { "3270_CursorBlink"           , 0xfd0f   , 0      }, { "3270_AltCursor"             , 0xfd10   , 0      }, { "3270_KeyClick"              , 0xfd11   , 0      }, { "3270_Jump"                  , 0xfd12   , 0      }, { "3270_Ident"                 , 0xfd13   , 0      }, { "3270_Rule"                  , 0xfd14   , 0      }, { "3270_Copy"                  , 0xfd15   , 0      }, { "3270_Play"                  , 0xfd16   , 0      }, { "3270_Setup"                 , 0xfd17   , 0      }, { "3270_Record"                , 0xfd18   , 0      }, { "3270_ChangeScreen"          , 0xfd19   , 0      }, { "3270_DeleteWord"            , 0xfd1a   , 0      }, { "3270_ExSelect"              , 0xfd1b   , 0      }, { "3270_CursorSelect"          , 0xfd1c   , 0      }, { "3270_PrintScreen"           , 0xfd1d   , 0      }, { "3270_Enter"                 , 0xfd1e   , 0      }, { "space"                      , 0x0020   , 0x0020 }, { "exclam"                     , 0x0021   , 0x0021 }, { "quotedbl"                   , 0x0022   , 0x0022 }, { "numbersign"                 , 0x0023   , 0x0023 }, { "dollar"                     , 0x0024   , 0x0024 }, { "percent"                    , 0x0025   , 0x0025 }, { "ampersand"                  , 0x0026   , 0x0026 }, { "apostrophe"                 , 0x0027   , 0x0027 }, { "quoteright"                 , 0x0027   , 0      }, { "parenleft"                  , 0x0028   , 0x0028 },
                { "parenright"                 , 0x0029   , 0x0029 }, { "asterisk"                   , 0x002a   , 0x002a }, { "plus"                       , 0x002b   , 0x002b }, { "comma"                      , 0x002c   , 0x002c }, { "minus"                      , 0x002d   , 0x002d }, { "period"                     , 0x002e   , 0x002e }, { "slash"                      , 0x002f   , 0x002f }, { "0"                          , 0x0030   , 0x0030 }, { "1"                          , 0x0031   , 0x0031 }, { "2"                          , 0x0032   , 0x0032 }, { "3"                          , 0x0033   , 0x0033 }, { "4"                          , 0x0034   , 0x0034 }, { "5"                          , 0x0035   , 0x0035 }, { "6"                          , 0x0036   , 0x0036 }, { "7"                          , 0x0037   , 0x0037 }, { "8"                          , 0x0038   , 0x0038 }, { "9"                          , 0x0039   , 0x0039 }, { "colon"                      , 0x003a   , 0x003a }, { "semicolon"                  , 0x003b   , 0x003b }, { "less"                       , 0x003c   , 0x003c }, { "equal"                      , 0x003d   , 0x003d }, { "greater"                    , 0x003e   , 0x003e }, { "question"                   , 0x003f   , 0x003f }, { "at"                         , 0x0040   , 0x0040 }, { "A"                          , 0x0041   , 0x0041 }, { "B"                          , 0x0042   , 0x0042 }, { "C"                          , 0x0043   , 0x0043 }, { "D"                          , 0x0044   , 0x0044 }, { "E"                          , 0x0045   , 0x0045 }, { "F"                          , 0x0046   , 0x0046 },
                { "G"                          , 0x0047   , 0x0047 }, { "H"                          , 0x0048   , 0x0048 }, { "I"                          , 0x0049   , 0x0049 }, { "J"                          , 0x004a   , 0x004a }, { "K"                          , 0x004b   , 0x004b }, { "L"                          , 0x004c   , 0x004c }, { "M"                          , 0x004d   , 0x004d }, { "N"                          , 0x004e   , 0x004e }, { "O"                          , 0x004f   , 0x004f }, { "P"                          , 0x0050   , 0x0050 }, { "Q"                          , 0x0051   , 0x0051 }, { "R"                          , 0x0052   , 0x0052 }, { "S"                          , 0x0053   , 0x0053 }, { "T"                          , 0x0054   , 0x0054 }, { "U"                          , 0x0055   , 0x0055 }, { "V"                          , 0x0056   , 0x0056 }, { "W"                          , 0x0057   , 0x0057 }, { "X"                          , 0x0058   , 0x0058 }, { "Y"                          , 0x0059   , 0x0059 }, { "Z"                          , 0x005a   , 0x005a }, { "bracketleft"                , 0x005b   , 0x005b }, { "backslash"                  , 0x005c   , 0x005c }, { "bracketright"               , 0x005d   , 0x005d }, { "asciicircum"                , 0x005e   , 0x005e }, { "underscore"                 , 0x005f   , 0x005f }, { "grave"                      , 0x0060   , 0x0060 }, { "quoteleft"                  , 0x0060   , 0      }, { "a"                          , 0x0061   , 0x0061 }, { "b"                          , 0x0062   , 0x0062 }, { "c"                          , 0x0063   , 0x0063 },
                { "d"                          , 0x0064   , 0x0064 }, { "e"                          , 0x0065   , 0x0065 }, { "f"                          , 0x0066   , 0x0066 }, { "g"                          , 0x0067   , 0x0067 }, { "h"                          , 0x0068   , 0x0068 }, { "i"                          , 0x0069   , 0x0069 }, { "j"                          , 0x006a   , 0x006a }, { "k"                          , 0x006b   , 0x006b }, { "l"                          , 0x006c   , 0x006c }, { "m"                          , 0x006d   , 0x006d }, { "n"                          , 0x006e   , 0x006e }, { "o"                          , 0x006f   , 0x006f }, { "p"                          , 0x0070   , 0x0070 }, { "q"                          , 0x0071   , 0x0071 }, { "r"                          , 0x0072   , 0x0072 }, { "s"                          , 0x0073   , 0x0073 }, { "t"                          , 0x0074   , 0x0074 }, { "u"                          , 0x0075   , 0x0075 }, { "v"                          , 0x0076   , 0x0076 }, { "w"                          , 0x0077   , 0x0077 }, { "x"                          , 0x0078   , 0x0078 }, { "y"                          , 0x0079   , 0x0079 }, { "z"                          , 0x007a   , 0x007a }, { "braceleft"                  , 0x007b   , 0x007b }, { "bar"                        , 0x007c   , 0x007c }, { "braceright"                 , 0x007d   , 0x007d }, { "asciitilde"                 , 0x007e   , 0x007e }, { "nobreakspace"               , 0x00a0   , 0x00a0 }, { "exclamdown"                 , 0x00a1   , 0x00a1 }, { "cent"                       , 0x00a2   , 0x00a2 },
                { "sterling"                   , 0x00a3   , 0x00a3 }, { "currency"                   , 0x00a4   , 0x00a4 }, { "yen"                        , 0x00a5   , 0x00a5 }, { "brokenbar"                  , 0x00a6   , 0x00a6 }, { "section"                    , 0x00a7   , 0x00a7 }, { "diaeresis"                  , 0x00a8   , 0x00a8 }, { "copyright"                  , 0x00a9   , 0x00a9 }, { "ordfeminine"                , 0x00aa   , 0x00aa }, { "guillemotleft"              , 0x00ab   , 0      }, { "guillemetleft"              , 0x00ab   , 0x00ab }, { "notsign"                    , 0x00ac   , 0x00ac }, { "hyphen"                     , 0x00ad   , 0x00ad }, { "registered"                 , 0x00ae   , 0x00ae }, { "macron"                     , 0x00af   , 0x00af }, { "degree"                     , 0x00b0   , 0x00b0 }, { "plusminus"                  , 0x00b1   , 0x00b1 }, { "twosuperior"                , 0x00b2   , 0x00b2 }, { "threesuperior"              , 0x00b3   , 0x00b3 }, { "acute"                      , 0x00b4   , 0x00b4 }, { "mu"                         , 0x00b5   , 0x00b5 }, { "paragraph"                  , 0x00b6   , 0x00b6 }, { "periodcentered"             , 0x00b7   , 0x00b7 }, { "cedilla"                    , 0x00b8   , 0x00b8 }, { "onesuperior"                , 0x00b9   , 0x00b9 }, { "masculine"                  , 0x00ba   , 0      }, { "ordmasculine"               , 0x00ba   , 0x00ba }, { "guillemotright"             , 0x00bb   , 0      }, { "guillemetright"             , 0x00bb   , 0x00bb }, { "onequarter"                 , 0x00bc   , 0x00bc }, { "onehalf"                    , 0x00bd   , 0x00bd },
                { "threequarters"              , 0x00be   , 0x00be }, { "questiondown"               , 0x00bf   , 0x00bf }, { "Agrave"                     , 0x00c0   , 0x00c0 }, { "Aacute"                     , 0x00c1   , 0x00c1 }, { "Acircumflex"                , 0x00c2   , 0x00c2 }, { "Atilde"                     , 0x00c3   , 0x00c3 }, { "Adiaeresis"                 , 0x00c4   , 0x00c4 }, { "Aring"                      , 0x00c5   , 0x00c5 }, { "AE"                         , 0x00c6   , 0x00c6 }, { "Ccedilla"                   , 0x00c7   , 0x00c7 }, { "Egrave"                     , 0x00c8   , 0x00c8 }, { "Eacute"                     , 0x00c9   , 0x00c9 }, { "Ecircumflex"                , 0x00ca   , 0x00ca }, { "Ediaeresis"                 , 0x00cb   , 0x00cb }, { "Igrave"                     , 0x00cc   , 0x00cc }, { "Iacute"                     , 0x00cd   , 0x00cd }, { "Icircumflex"                , 0x00ce   , 0x00ce }, { "Idiaeresis"                 , 0x00cf   , 0x00cf }, { "ETH"                        , 0x00d0   , 0x00d0 }, { "Eth"                        , 0x00d0   , 0      }, { "Ntilde"                     , 0x00d1   , 0x00d1 }, { "Ograve"                     , 0x00d2   , 0x00d2 }, { "Oacute"                     , 0x00d3   , 0x00d3 }, { "Ocircumflex"                , 0x00d4   , 0x00d4 }, { "Otilde"                     , 0x00d5   , 0x00d5 }, { "Odiaeresis"                 , 0x00d6   , 0x00d6 }, { "multiply"                   , 0x00d7   , 0x00d7 }, { "Oslash"                     , 0x00d8   , 0x00d8 }, { "Ooblique"                   , 0x00d8   , 0      }, { "Ugrave"                     , 0x00d9   , 0x00d9 },
                { "Uacute"                     , 0x00da   , 0x00da }, { "Ucircumflex"                , 0x00db   , 0x00db }, { "Udiaeresis"                 , 0x00dc   , 0x00dc }, { "Yacute"                     , 0x00dd   , 0x00dd }, { "THORN"                      , 0x00de   , 0x00de }, { "Thorn"                      , 0x00de   , 0      }, { "ssharp"                     , 0x00df   , 0x00df }, { "agrave"                     , 0x00e0   , 0x00e0 }, { "aacute"                     , 0x00e1   , 0x00e1 }, { "acircumflex"                , 0x00e2   , 0x00e2 }, { "atilde"                     , 0x00e3   , 0x00e3 }, { "adiaeresis"                 , 0x00e4   , 0x00e4 }, { "aring"                      , 0x00e5   , 0x00e5 }, { "ae"                         , 0x00e6   , 0x00e6 }, { "ccedilla"                   , 0x00e7   , 0x00e7 }, { "egrave"                     , 0x00e8   , 0x00e8 }, { "eacute"                     , 0x00e9   , 0x00e9 }, { "ecircumflex"                , 0x00ea   , 0x00ea }, { "ediaeresis"                 , 0x00eb   , 0x00eb }, { "igrave"                     , 0x00ec   , 0x00ec }, { "iacute"                     , 0x00ed   , 0x00ed }, { "icircumflex"                , 0x00ee   , 0x00ee }, { "idiaeresis"                 , 0x00ef   , 0x00ef }, { "eth"                        , 0x00f0   , 0x00f0 }, { "ntilde"                     , 0x00f1   , 0x00f1 }, { "ograve"                     , 0x00f2   , 0x00f2 }, { "oacute"                     , 0x00f3   , 0x00f3 }, { "ocircumflex"                , 0x00f4   , 0x00f4 }, { "otilde"                     , 0x00f5   , 0x00f5 }, { "odiaeresis"                 , 0x00f6   , 0x00f6 },
                { "division"                   , 0x00f7   , 0x00f7 }, { "oslash"                     , 0x00f8   , 0x00f8 }, { "ooblique"                   , 0x00f8   , 0      }, { "ugrave"                     , 0x00f9   , 0x00f9 }, { "uacute"                     , 0x00fa   , 0x00fa }, { "ucircumflex"                , 0x00fb   , 0x00fb }, { "udiaeresis"                 , 0x00fc   , 0x00fc }, { "yacute"                     , 0x00fd   , 0x00fd }, { "thorn"                      , 0x00fe   , 0x00fe }, { "ydiaeresis"                 , 0x00ff   , 0x00ff }, { "Aogonek"                    , 0x01a1   , 0x0104 }, { "breve"                      , 0x01a2   , 0x02d8 }, { "Lstroke"                    , 0x01a3   , 0x0141 }, { "Lcaron"                     , 0x01a5   , 0x013d }, { "Sacute"                     , 0x01a6   , 0x015a }, { "Scaron"                     , 0x01a9   , 0x0160 }, { "Scedilla"                   , 0x01aa   , 0x015e }, { "Tcaron"                     , 0x01ab   , 0x0164 }, { "Zacute"                     , 0x01ac   , 0x0179 }, { "Zcaron"                     , 0x01ae   , 0x017d }, { "Zabovedot"                  , 0x01af   , 0x017b }, { "aogonek"                    , 0x01b1   , 0x0105 }, { "ogonek"                     , 0x01b2   , 0x02db }, { "lstroke"                    , 0x01b3   , 0x0142 }, { "lcaron"                     , 0x01b5   , 0x013e }, { "sacute"                     , 0x01b6   , 0x015b }, { "caron"                      , 0x01b7   , 0x02c7 }, { "scaron"                     , 0x01b9   , 0x0161 }, { "scedilla"                   , 0x01ba   , 0x015f }, { "tcaron"                     , 0x01bb   , 0x0165 },
                { "zacute"                     , 0x01bc   , 0x017a }, { "doubleacute"                , 0x01bd   , 0x02dd }, { "zcaron"                     , 0x01be   , 0x017e }, { "zabovedot"                  , 0x01bf   , 0x017c }, { "Racute"                     , 0x01c0   , 0x0154 }, { "Abreve"                     , 0x01c3   , 0x0102 }, { "Lacute"                     , 0x01c5   , 0x0139 }, { "Cacute"                     , 0x01c6   , 0x0106 }, { "Ccaron"                     , 0x01c8   , 0x010c }, { "Eogonek"                    , 0x01ca   , 0x0118 }, { "Ecaron"                     , 0x01cc   , 0x011a }, { "Dcaron"                     , 0x01cf   , 0x010e }, { "Dstroke"                    , 0x01d0   , 0x0110 }, { "Nacute"                     , 0x01d1   , 0x0143 }, { "Ncaron"                     , 0x01d2   , 0x0147 }, { "Odoubleacute"               , 0x01d5   , 0x0150 }, { "Rcaron"                     , 0x01d8   , 0x0158 }, { "Uring"                      , 0x01d9   , 0x016e }, { "Udoubleacute"               , 0x01db   , 0x0170 }, { "Tcedilla"                   , 0x01de   , 0x0162 }, { "racute"                     , 0x01e0   , 0x0155 }, { "abreve"                     , 0x01e3   , 0x0103 }, { "lacute"                     , 0x01e5   , 0x013a }, { "cacute"                     , 0x01e6   , 0x0107 }, { "ccaron"                     , 0x01e8   , 0x010d }, { "eogonek"                    , 0x01ea   , 0x0119 }, { "ecaron"                     , 0x01ec   , 0x011b }, { "dcaron"                     , 0x01ef   , 0x010f }, { "dstroke"                    , 0x01f0   , 0x0111 }, { "nacute"                     , 0x01f1   , 0x0144 },
                { "ncaron"                     , 0x01f2   , 0x0148 }, { "odoubleacute"               , 0x01f5   , 0x0151 }, { "rcaron"                     , 0x01f8   , 0x0159 }, { "uring"                      , 0x01f9   , 0x016f }, { "udoubleacute"               , 0x01fb   , 0x0171 }, { "tcedilla"                   , 0x01fe   , 0x0163 }, { "abovedot"                   , 0x01ff   , 0x02d9 }, { "Hstroke"                    , 0x02a1   , 0x0126 }, { "Hcircumflex"                , 0x02a6   , 0x0124 }, { "Iabovedot"                  , 0x02a9   , 0x0130 }, { "Gbreve"                     , 0x02ab   , 0x011e }, { "Jcircumflex"                , 0x02ac   , 0x0134 }, { "hstroke"                    , 0x02b1   , 0x0127 }, { "hcircumflex"                , 0x02b6   , 0x0125 }, { "idotless"                   , 0x02b9   , 0x0131 }, { "gbreve"                     , 0x02bb   , 0x011f }, { "jcircumflex"                , 0x02bc   , 0x0135 }, { "Cabovedot"                  , 0x02c5   , 0x010a }, { "Ccircumflex"                , 0x02c6   , 0x0108 }, { "Gabovedot"                  , 0x02d5   , 0x0120 }, { "Gcircumflex"                , 0x02d8   , 0x011c }, { "Ubreve"                     , 0x02dd   , 0x016c }, { "Scircumflex"                , 0x02de   , 0x015c }, { "cabovedot"                  , 0x02e5   , 0x010b }, { "ccircumflex"                , 0x02e6   , 0x0109 }, { "gabovedot"                  , 0x02f5   , 0x0121 }, { "gcircumflex"                , 0x02f8   , 0x011d }, { "ubreve"                     , 0x02fd   , 0x016d }, { "scircumflex"                , 0x02fe   , 0x015d }, { "kra"                        , 0x03a2   , 0x0138 },
                { "kappa"                      , 0x03a2   , 0      }, { "Rcedilla"                   , 0x03a3   , 0x0156 }, { "Itilde"                     , 0x03a5   , 0x0128 }, { "Lcedilla"                   , 0x03a6   , 0x013b }, { "Emacron"                    , 0x03aa   , 0x0112 }, { "Gcedilla"                   , 0x03ab   , 0x0122 }, { "Tslash"                     , 0x03ac   , 0x0166 }, { "rcedilla"                   , 0x03b3   , 0x0157 }, { "itilde"                     , 0x03b5   , 0x0129 }, { "lcedilla"                   , 0x03b6   , 0x013c }, { "emacron"                    , 0x03ba   , 0x0113 }, { "gcedilla"                   , 0x03bb   , 0x0123 }, { "tslash"                     , 0x03bc   , 0x0167 }, { "ENG"                        , 0x03bd   , 0x014a }, { "eng"                        , 0x03bf   , 0x014b }, { "Amacron"                    , 0x03c0   , 0x0100 }, { "Iogonek"                    , 0x03c7   , 0x012e }, { "Eabovedot"                  , 0x03cc   , 0x0116 }, { "Imacron"                    , 0x03cf   , 0x012a }, { "Ncedilla"                   , 0x03d1   , 0x0145 }, { "Omacron"                    , 0x03d2   , 0x014c }, { "Kcedilla"                   , 0x03d3   , 0x0136 }, { "Uogonek"                    , 0x03d9   , 0x0172 }, { "Utilde"                     , 0x03dd   , 0x0168 }, { "Umacron"                    , 0x03de   , 0x016a }, { "amacron"                    , 0x03e0   , 0x0101 }, { "iogonek"                    , 0x03e7   , 0x012f }, { "eabovedot"                  , 0x03ec   , 0x0117 }, { "imacron"                    , 0x03ef   , 0x012b }, { "ncedilla"                   , 0x03f1   , 0x0146 },
                { "omacron"                    , 0x03f2   , 0x014d }, { "kcedilla"                   , 0x03f3   , 0x0137 }, { "uogonek"                    , 0x03f9   , 0x0173 }, { "utilde"                     , 0x03fd   , 0x0169 }, { "umacron"                    , 0x03fe   , 0x016b }, { "Wcircumflex"                , 0x1000174, 0x0174 }, { "wcircumflex"                , 0x1000175, 0x0175 }, { "Ycircumflex"                , 0x1000176, 0x0176 }, { "ycircumflex"                , 0x1000177, 0x0177 }, { "Babovedot"                  , 0x1001e02, 0x1e02 }, { "babovedot"                  , 0x1001e03, 0x1e03 }, { "Dabovedot"                  , 0x1001e0a, 0x1e0a }, { "dabovedot"                  , 0x1001e0b, 0x1e0b }, { "Fabovedot"                  , 0x1001e1e, 0x1e1e }, { "fabovedot"                  , 0x1001e1f, 0x1e1f }, { "Mabovedot"                  , 0x1001e40, 0x1e40 }, { "mabovedot"                  , 0x1001e41, 0x1e41 }, { "Pabovedot"                  , 0x1001e56, 0x1e56 }, { "pabovedot"                  , 0x1001e57, 0x1e57 }, { "Sabovedot"                  , 0x1001e60, 0x1e60 }, { "sabovedot"                  , 0x1001e61, 0x1e61 }, { "Tabovedot"                  , 0x1001e6a, 0x1e6a }, { "tabovedot"                  , 0x1001e6b, 0x1e6b }, { "Wgrave"                     , 0x1001e80, 0x1e80 }, { "wgrave"                     , 0x1001e81, 0x1e81 }, { "Wacute"                     , 0x1001e82, 0x1e82 }, { "wacute"                     , 0x1001e83, 0x1e83 }, { "Wdiaeresis"                 , 0x1001e84, 0x1e84 }, { "wdiaeresis"                 , 0x1001e85, 0x1e85 }, { "Ygrave"                     , 0x1001ef2, 0x1ef2 },
                { "ygrave"                     , 0x1001ef3, 0x1ef3 }, { "OE"                         , 0x13bc   , 0x0152 }, { "oe"                         , 0x13bd   , 0x0153 }, { "Ydiaeresis"                 , 0x13be   , 0x0178 }, { "overline"                   , 0x047e   , 0x203e }, { "kana_fullstop"              , 0x04a1   , 0x3002 }, { "kana_openingbracket"        , 0x04a2   , 0x300c }, { "kana_closingbracket"        , 0x04a3   , 0x300d }, { "kana_comma"                 , 0x04a4   , 0x3001 }, { "kana_conjunctive"           , 0x04a5   , 0x30fb }, { "kana_middledot"             , 0x04a5   , 0      }, { "kana_WO"                    , 0x04a6   , 0x30f2 }, { "kana_a"                     , 0x04a7   , 0x30a1 }, { "kana_i"                     , 0x04a8   , 0x30a3 }, { "kana_u"                     , 0x04a9   , 0x30a5 }, { "kana_e"                     , 0x04aa   , 0x30a7 }, { "kana_o"                     , 0x04ab   , 0x30a9 }, { "kana_ya"                    , 0x04ac   , 0x30e3 }, { "kana_yu"                    , 0x04ad   , 0x30e5 }, { "kana_yo"                    , 0x04ae   , 0x30e7 }, { "kana_tsu"                   , 0x04af   , 0x30c3 }, { "kana_tu"                    , 0x04af   , 0      }, { "prolongedsound"             , 0x04b0   , 0x30fc }, { "kana_A"                     , 0x04b1   , 0x30a2 }, { "kana_I"                     , 0x04b2   , 0x30a4 }, { "kana_U"                     , 0x04b3   , 0x30a6 }, { "kana_E"                     , 0x04b4   , 0x30a8 }, { "kana_O"                     , 0x04b5   , 0x30aa }, { "kana_KA"                    , 0x04b6   , 0x30ab }, { "kana_KI"                    , 0x04b7   , 0x30ad },
                { "kana_KU"                    , 0x04b8   , 0x30af }, { "kana_KE"                    , 0x04b9   , 0x30b1 }, { "kana_KO"                    , 0x04ba   , 0x30b3 }, { "kana_SA"                    , 0x04bb   , 0x30b5 }, { "kana_SHI"                   , 0x04bc   , 0x30b7 }, { "kana_SU"                    , 0x04bd   , 0x30b9 }, { "kana_SE"                    , 0x04be   , 0x30bb }, { "kana_SO"                    , 0x04bf   , 0x30bd }, { "kana_TA"                    , 0x04c0   , 0x30bf }, { "kana_CHI"                   , 0x04c1   , 0x30c1 }, { "kana_TI"                    , 0x04c1   , 0      }, { "kana_TSU"                   , 0x04c2   , 0x30c4 }, { "kana_TU"                    , 0x04c2   , 0      }, { "kana_TE"                    , 0x04c3   , 0x30c6 }, { "kana_TO"                    , 0x04c4   , 0x30c8 }, { "kana_NA"                    , 0x04c5   , 0x30ca }, { "kana_NI"                    , 0x04c6   , 0x30cb }, { "kana_NU"                    , 0x04c7   , 0x30cc }, { "kana_NE"                    , 0x04c8   , 0x30cd }, { "kana_NO"                    , 0x04c9   , 0x30ce }, { "kana_HA"                    , 0x04ca   , 0x30cf }, { "kana_HI"                    , 0x04cb   , 0x30d2 }, { "kana_FU"                    , 0x04cc   , 0x30d5 }, { "kana_HU"                    , 0x04cc   , 0      }, { "kana_HE"                    , 0x04cd   , 0x30d8 }, { "kana_HO"                    , 0x04ce   , 0x30db }, { "kana_MA"                    , 0x04cf   , 0x30de }, { "kana_MI"                    , 0x04d0   , 0x30df }, { "kana_MU"                    , 0x04d1   , 0x30e0 }, { "kana_ME"                    , 0x04d2   , 0x30e1 },
                { "kana_MO"                    , 0x04d3   , 0x30e2 }, { "kana_YA"                    , 0x04d4   , 0x30e4 }, { "kana_YU"                    , 0x04d5   , 0x30e6 }, { "kana_YO"                    , 0x04d6   , 0x30e8 }, { "kana_RA"                    , 0x04d7   , 0x30e9 }, { "kana_RI"                    , 0x04d8   , 0x30ea }, { "kana_RU"                    , 0x04d9   , 0x30eb }, { "kana_RE"                    , 0x04da   , 0x30ec }, { "kana_RO"                    , 0x04db   , 0x30ed }, { "kana_WA"                    , 0x04dc   , 0x30ef }, { "kana_N"                     , 0x04dd   , 0x30f3 }, { "voicedsound"                , 0x04de   , 0x309b }, { "semivoicedsound"            , 0x04df   , 0x309c }, { "kana_switch"                , 0xff7e   , 0      }, { "Farsi_0"                    , 0x10006f0, 0x06f0 }, { "Farsi_1"                    , 0x10006f1, 0x06f1 }, { "Farsi_2"                    , 0x10006f2, 0x06f2 }, { "Farsi_3"                    , 0x10006f3, 0x06f3 }, { "Farsi_4"                    , 0x10006f4, 0x06f4 }, { "Farsi_5"                    , 0x10006f5, 0x06f5 }, { "Farsi_6"                    , 0x10006f6, 0x06f6 }, { "Farsi_7"                    , 0x10006f7, 0x06f7 }, { "Farsi_8"                    , 0x10006f8, 0x06f8 }, { "Farsi_9"                    , 0x10006f9, 0x06f9 }, { "Arabic_percent"             , 0x100066a, 0x066a }, { "Arabic_superscript_alef"    , 0x1000670, 0x0670 }, { "Arabic_tteh"                , 0x1000679, 0x0679 }, { "Arabic_peh"                 , 0x100067e, 0x067e }, { "Arabic_tcheh"               , 0x1000686, 0x0686 }, { "Arabic_ddal"                , 0x1000688, 0x0688 },
                { "Arabic_rreh"                , 0x1000691, 0x0691 }, { "Arabic_comma"               , 0x05ac   , 0x060c }, { "Arabic_fullstop"            , 0x10006d4, 0x06d4 }, { "Arabic_0"                   , 0x1000660, 0x0660 }, { "Arabic_1"                   , 0x1000661, 0x0661 }, { "Arabic_2"                   , 0x1000662, 0x0662 }, { "Arabic_3"                   , 0x1000663, 0x0663 }, { "Arabic_4"                   , 0x1000664, 0x0664 }, { "Arabic_5"                   , 0x1000665, 0x0665 }, { "Arabic_6"                   , 0x1000666, 0x0666 }, { "Arabic_7"                   , 0x1000667, 0x0667 }, { "Arabic_8"                   , 0x1000668, 0x0668 }, { "Arabic_9"                   , 0x1000669, 0x0669 }, { "Arabic_semicolon"           , 0x05bb   , 0x061b }, { "Arabic_question_mark"       , 0x05bf   , 0x061f }, { "Arabic_hamza"               , 0x05c1   , 0x0621 }, { "Arabic_maddaonalef"         , 0x05c2   , 0x0622 }, { "Arabic_hamzaonalef"         , 0x05c3   , 0x0623 }, { "Arabic_hamzaonwaw"          , 0x05c4   , 0x0624 }, { "Arabic_hamzaunderalef"      , 0x05c5   , 0x0625 }, { "Arabic_hamzaonyeh"          , 0x05c6   , 0x0626 }, { "Arabic_alef"                , 0x05c7   , 0x0627 }, { "Arabic_beh"                 , 0x05c8   , 0x0628 }, { "Arabic_tehmarbuta"          , 0x05c9   , 0x0629 }, { "Arabic_teh"                 , 0x05ca   , 0x062a }, { "Arabic_theh"                , 0x05cb   , 0x062b }, { "Arabic_jeem"                , 0x05cc   , 0x062c }, { "Arabic_hah"                 , 0x05cd   , 0x062d }, { "Arabic_khah"                , 0x05ce   , 0x062e }, { "Arabic_dal"                 , 0x05cf   , 0x062f },
                { "Arabic_thal"                , 0x05d0   , 0x0630 }, { "Arabic_ra"                  , 0x05d1   , 0x0631 }, { "Arabic_zain"                , 0x05d2   , 0x0632 }, { "Arabic_seen"                , 0x05d3   , 0x0633 }, { "Arabic_sheen"               , 0x05d4   , 0x0634 }, { "Arabic_sad"                 , 0x05d5   , 0x0635 }, { "Arabic_dad"                 , 0x05d6   , 0x0636 }, { "Arabic_tah"                 , 0x05d7   , 0x0637 }, { "Arabic_zah"                 , 0x05d8   , 0x0638 }, { "Arabic_ain"                 , 0x05d9   , 0x0639 }, { "Arabic_ghain"               , 0x05da   , 0x063a }, { "Arabic_tatweel"             , 0x05e0   , 0x0640 }, { "Arabic_feh"                 , 0x05e1   , 0x0641 }, { "Arabic_qaf"                 , 0x05e2   , 0x0642 }, { "Arabic_kaf"                 , 0x05e3   , 0x0643 }, { "Arabic_lam"                 , 0x05e4   , 0x0644 }, { "Arabic_meem"                , 0x05e5   , 0x0645 }, { "Arabic_noon"                , 0x05e6   , 0x0646 }, { "Arabic_ha"                  , 0x05e7   , 0x0647 }, { "Arabic_heh"                 , 0x05e7   , 0      }, { "Arabic_waw"                 , 0x05e8   , 0x0648 }, { "Arabic_alefmaksura"         , 0x05e9   , 0x0649 }, { "Arabic_yeh"                 , 0x05ea   , 0x064a }, { "Arabic_fathatan"            , 0x05eb   , 0x064b }, { "Arabic_dammatan"            , 0x05ec   , 0x064c }, { "Arabic_kasratan"            , 0x05ed   , 0x064d }, { "Arabic_fatha"               , 0x05ee   , 0x064e }, { "Arabic_damma"               , 0x05ef   , 0x064f }, { "Arabic_kasra"               , 0x05f0   , 0x0650 }, { "Arabic_shadda"              , 0x05f1   , 0x0651 },
                { "Arabic_sukun"               , 0x05f2   , 0x0652 }, { "Arabic_madda_above"         , 0x1000653, 0x0653 }, { "Arabic_hamza_above"         , 0x1000654, 0x0654 }, { "Arabic_hamza_below"         , 0x1000655, 0x0655 }, { "Arabic_jeh"                 , 0x1000698, 0x0698 }, { "Arabic_veh"                 , 0x10006a4, 0x06a4 }, { "Arabic_keheh"               , 0x10006a9, 0x06a9 }, { "Arabic_gaf"                 , 0x10006af, 0x06af }, { "Arabic_noon_ghunna"         , 0x10006ba, 0x06ba }, { "Arabic_heh_doachashmee"     , 0x10006be, 0x06be }, { "Farsi_yeh"                  , 0x10006cc, 0x06cc }, { "Arabic_farsi_yeh"           , 0x10006cc, 0      }, { "Arabic_yeh_baree"           , 0x10006d2, 0x06d2 }, { "Arabic_heh_goal"            , 0x10006c1, 0x06c1 }, { "Arabic_switch"              , 0xff7e   , 0      }, { "Cyrillic_GHE_bar"           , 0x1000492, 0x0492 }, { "Cyrillic_ghe_bar"           , 0x1000493, 0x0493 }, { "Cyrillic_ZHE_descender"     , 0x1000496, 0x0496 }, { "Cyrillic_zhe_descender"     , 0x1000497, 0x0497 }, { "Cyrillic_KA_descender"      , 0x100049a, 0x049a }, { "Cyrillic_ka_descender"      , 0x100049b, 0x049b }, { "Cyrillic_KA_vertstroke"     , 0x100049c, 0x049c }, { "Cyrillic_ka_vertstroke"     , 0x100049d, 0x049d }, { "Cyrillic_EN_descender"      , 0x10004a2, 0x04a2 }, { "Cyrillic_en_descender"      , 0x10004a3, 0x04a3 }, { "Cyrillic_U_straight"        , 0x10004ae, 0x04ae }, { "Cyrillic_u_straight"        , 0x10004af, 0x04af }, { "Cyrillic_U_straight_bar"    , 0x10004b0, 0x04b0 }, { "Cyrillic_u_straight_bar"    , 0x10004b1, 0x04b1 }, { "Cyrillic_HA_descender"      , 0x10004b2, 0x04b2 },
                { "Cyrillic_ha_descender"      , 0x10004b3, 0x04b3 }, { "Cyrillic_CHE_descender"     , 0x10004b6, 0x04b6 }, { "Cyrillic_che_descender"     , 0x10004b7, 0x04b7 }, { "Cyrillic_CHE_vertstroke"    , 0x10004b8, 0x04b8 }, { "Cyrillic_che_vertstroke"    , 0x10004b9, 0x04b9 }, { "Cyrillic_SHHA"              , 0x10004ba, 0x04ba }, { "Cyrillic_shha"              , 0x10004bb, 0x04bb }, { "Cyrillic_SCHWA"             , 0x10004d8, 0x04d8 }, { "Cyrillic_schwa"             , 0x10004d9, 0x04d9 }, { "Cyrillic_I_macron"          , 0x10004e2, 0x04e2 }, { "Cyrillic_i_macron"          , 0x10004e3, 0x04e3 }, { "Cyrillic_O_bar"             , 0x10004e8, 0x04e8 }, { "Cyrillic_o_bar"             , 0x10004e9, 0x04e9 }, { "Cyrillic_U_macron"          , 0x10004ee, 0x04ee }, { "Cyrillic_u_macron"          , 0x10004ef, 0x04ef }, { "Serbian_dje"                , 0x06a1   , 0x0452 }, { "Macedonia_gje"              , 0x06a2   , 0x0453 }, { "Cyrillic_io"                , 0x06a3   , 0x0451 }, { "Ukrainian_ie"               , 0x06a4   , 0x0454 }, { "Ukranian_je"                , 0x06a4   , 0      }, { "Macedonia_dse"              , 0x06a5   , 0x0455 }, { "Ukrainian_i"                , 0x06a6   , 0x0456 }, { "Ukranian_i"                 , 0x06a6   , 0      }, { "Ukrainian_yi"               , 0x06a7   , 0x0457 }, { "Ukranian_yi"                , 0x06a7   , 0      }, { "Cyrillic_je"                , 0x06a8   , 0x0458 }, { "Serbian_je"                 , 0x06a8   , 0      }, { "Cyrillic_lje"               , 0x06a9   , 0x0459 }, { "Serbian_lje"                , 0x06a9   , 0      }, { "Cyrillic_nje"               , 0x06aa   , 0x045a },
                { "Serbian_nje"                , 0x06aa   , 0      }, { "Serbian_tshe"               , 0x06ab   , 0x045b }, { "Macedonia_kje"              , 0x06ac   , 0x045c }, { "Ukrainian_ghe_with_upturn"  , 0x06ad   , 0x0491 }, { "Byelorussian_shortu"        , 0x06ae   , 0x045e }, { "Cyrillic_dzhe"              , 0x06af   , 0x045f }, { "Serbian_dze"                , 0x06af   , 0      }, { "numerosign"                 , 0x06b0   , 0x2116 }, { "Serbian_DJE"                , 0x06b1   , 0x0402 }, { "Macedonia_GJE"              , 0x06b2   , 0x0403 }, { "Cyrillic_IO"                , 0x06b3   , 0x0401 }, { "Ukrainian_IE"               , 0x06b4   , 0x0404 }, { "Ukranian_JE"                , 0x06b4   , 0      }, { "Macedonia_DSE"              , 0x06b5   , 0x0405 }, { "Ukrainian_I"                , 0x06b6   , 0x0406 }, { "Ukranian_I"                 , 0x06b6   , 0      }, { "Ukrainian_YI"               , 0x06b7   , 0x0407 }, { "Ukranian_YI"                , 0x06b7   , 0      }, { "Cyrillic_JE"                , 0x06b8   , 0x0408 }, { "Serbian_JE"                 , 0x06b8   , 0      }, { "Cyrillic_LJE"               , 0x06b9   , 0x0409 }, { "Serbian_LJE"                , 0x06b9   , 0      }, { "Cyrillic_NJE"               , 0x06ba   , 0x040a }, { "Serbian_NJE"                , 0x06ba   , 0      }, { "Serbian_TSHE"               , 0x06bb   , 0x040b }, { "Macedonia_KJE"              , 0x06bc   , 0x040c }, { "Ukrainian_GHE_WITH_UPTURN"  , 0x06bd   , 0x0490 }, { "Byelorussian_SHORTU"        , 0x06be   , 0x040e }, { "Cyrillic_DZHE"              , 0x06bf   , 0x040f }, { "Serbian_DZE"                , 0x06bf   , 0      },
                { "Cyrillic_yu"                , 0x06c0   , 0x044e }, { "Cyrillic_a"                 , 0x06c1   , 0x0430 }, { "Cyrillic_be"                , 0x06c2   , 0x0431 }, { "Cyrillic_tse"               , 0x06c3   , 0x0446 }, { "Cyrillic_de"                , 0x06c4   , 0x0434 }, { "Cyrillic_ie"                , 0x06c5   , 0x0435 }, { "Cyrillic_ef"                , 0x06c6   , 0x0444 }, { "Cyrillic_ghe"               , 0x06c7   , 0x0433 }, { "Cyrillic_ha"                , 0x06c8   , 0x0445 }, { "Cyrillic_i"                 , 0x06c9   , 0x0438 }, { "Cyrillic_shorti"            , 0x06ca   , 0x0439 }, { "Cyrillic_ka"                , 0x06cb   , 0x043a }, { "Cyrillic_el"                , 0x06cc   , 0x043b }, { "Cyrillic_em"                , 0x06cd   , 0x043c }, { "Cyrillic_en"                , 0x06ce   , 0x043d }, { "Cyrillic_o"                 , 0x06cf   , 0x043e }, { "Cyrillic_pe"                , 0x06d0   , 0x043f }, { "Cyrillic_ya"                , 0x06d1   , 0x044f }, { "Cyrillic_er"                , 0x06d2   , 0x0440 }, { "Cyrillic_es"                , 0x06d3   , 0x0441 }, { "Cyrillic_te"                , 0x06d4   , 0x0442 }, { "Cyrillic_u"                 , 0x06d5   , 0x0443 }, { "Cyrillic_zhe"               , 0x06d6   , 0x0436 }, { "Cyrillic_ve"                , 0x06d7   , 0x0432 }, { "Cyrillic_softsign"          , 0x06d8   , 0x044c }, { "Cyrillic_yeru"              , 0x06d9   , 0x044b }, { "Cyrillic_ze"                , 0x06da   , 0x0437 }, { "Cyrillic_sha"               , 0x06db   , 0x0448 }, { "Cyrillic_e"                 , 0x06dc   , 0x044d }, { "Cyrillic_shcha"             , 0x06dd   , 0x0449 },
                { "Cyrillic_che"               , 0x06de   , 0x0447 }, { "Cyrillic_hardsign"          , 0x06df   , 0x044a }, { "Cyrillic_YU"                , 0x06e0   , 0x042e }, { "Cyrillic_A"                 , 0x06e1   , 0x0410 }, { "Cyrillic_BE"                , 0x06e2   , 0x0411 }, { "Cyrillic_TSE"               , 0x06e3   , 0x0426 }, { "Cyrillic_DE"                , 0x06e4   , 0x0414 }, { "Cyrillic_IE"                , 0x06e5   , 0x0415 }, { "Cyrillic_EF"                , 0x06e6   , 0x0424 }, { "Cyrillic_GHE"               , 0x06e7   , 0x0413 }, { "Cyrillic_HA"                , 0x06e8   , 0x0425 }, { "Cyrillic_I"                 , 0x06e9   , 0x0418 }, { "Cyrillic_SHORTI"            , 0x06ea   , 0x0419 }, { "Cyrillic_KA"                , 0x06eb   , 0x041a }, { "Cyrillic_EL"                , 0x06ec   , 0x041b }, { "Cyrillic_EM"                , 0x06ed   , 0x041c }, { "Cyrillic_EN"                , 0x06ee   , 0x041d }, { "Cyrillic_O"                 , 0x06ef   , 0x041e }, { "Cyrillic_PE"                , 0x06f0   , 0x041f }, { "Cyrillic_YA"                , 0x06f1   , 0x042f }, { "Cyrillic_ER"                , 0x06f2   , 0x0420 }, { "Cyrillic_ES"                , 0x06f3   , 0x0421 }, { "Cyrillic_TE"                , 0x06f4   , 0x0422 }, { "Cyrillic_U"                 , 0x06f5   , 0x0423 }, { "Cyrillic_ZHE"               , 0x06f6   , 0x0416 }, { "Cyrillic_VE"                , 0x06f7   , 0x0412 }, { "Cyrillic_SOFTSIGN"          , 0x06f8   , 0x042c }, { "Cyrillic_YERU"              , 0x06f9   , 0x042b }, { "Cyrillic_ZE"                , 0x06fa   , 0x0417 }, { "Cyrillic_SHA"               , 0x06fb   , 0x0428 },
                { "Cyrillic_E"                 , 0x06fc   , 0x042d }, { "Cyrillic_SHCHA"             , 0x06fd   , 0x0429 }, { "Cyrillic_CHE"               , 0x06fe   , 0x0427 }, { "Cyrillic_HARDSIGN"          , 0x06ff   , 0x042a }, { "Greek_ALPHAaccent"          , 0x07a1   , 0x0386 }, { "Greek_EPSILONaccent"        , 0x07a2   , 0x0388 }, { "Greek_ETAaccent"            , 0x07a3   , 0x0389 }, { "Greek_IOTAaccent"           , 0x07a4   , 0x038a }, { "Greek_IOTAdieresis"         , 0x07a5   , 0x03aa }, { "Greek_IOTAdiaeresis"        , 0x07a5   , 0      }, { "Greek_OMICRONaccent"        , 0x07a7   , 0x038c }, { "Greek_UPSILONaccent"        , 0x07a8   , 0x038e }, { "Greek_UPSILONdieresis"      , 0x07a9   , 0x03ab }, { "Greek_OMEGAaccent"          , 0x07ab   , 0x038f }, { "Greek_accentdieresis"       , 0x07ae   , 0x0385 }, { "Greek_horizbar"             , 0x07af   , 0x2015 }, { "Greek_alphaaccent"          , 0x07b1   , 0x03ac }, { "Greek_epsilonaccent"        , 0x07b2   , 0x03ad }, { "Greek_etaaccent"            , 0x07b3   , 0x03ae }, { "Greek_iotaaccent"           , 0x07b4   , 0x03af }, { "Greek_iotadieresis"         , 0x07b5   , 0x03ca }, { "Greek_iotaaccentdieresis"   , 0x07b6   , 0x0390 }, { "Greek_omicronaccent"        , 0x07b7   , 0x03cc }, { "Greek_upsilonaccent"        , 0x07b8   , 0x03cd }, { "Greek_upsilondieresis"      , 0x07b9   , 0x03cb }, { "Greek_upsilonaccentdieresis", 0x07ba   , 0x03b0 }, { "Greek_omegaaccent"          , 0x07bb   , 0x03ce }, { "Greek_ALPHA"                , 0x07c1   , 0x0391 }, { "Greek_BETA"                 , 0x07c2   , 0x0392 }, { "Greek_GAMMA"                , 0x07c3   , 0x0393 },
                { "Greek_DELTA"                , 0x07c4   , 0x0394 }, { "Greek_EPSILON"              , 0x07c5   , 0x0395 }, { "Greek_ZETA"                 , 0x07c6   , 0x0396 }, { "Greek_ETA"                  , 0x07c7   , 0x0397 }, { "Greek_THETA"                , 0x07c8   , 0x0398 }, { "Greek_IOTA"                 , 0x07c9   , 0x0399 }, { "Greek_KAPPA"                , 0x07ca   , 0x039a }, { "Greek_LAMDA"                , 0x07cb   , 0x039b }, { "Greek_LAMBDA"               , 0x07cb   , 0      }, { "Greek_MU"                   , 0x07cc   , 0x039c }, { "Greek_NU"                   , 0x07cd   , 0x039d }, { "Greek_XI"                   , 0x07ce   , 0x039e }, { "Greek_OMICRON"              , 0x07cf   , 0x039f }, { "Greek_PI"                   , 0x07d0   , 0x03a0 }, { "Greek_RHO"                  , 0x07d1   , 0x03a1 }, { "Greek_SIGMA"                , 0x07d2   , 0x03a3 }, { "Greek_TAU"                  , 0x07d4   , 0x03a4 }, { "Greek_UPSILON"              , 0x07d5   , 0x03a5 }, { "Greek_PHI"                  , 0x07d6   , 0x03a6 }, { "Greek_CHI"                  , 0x07d7   , 0x03a7 }, { "Greek_PSI"                  , 0x07d8   , 0x03a8 }, { "Greek_OMEGA"                , 0x07d9   , 0x03a9 }, { "Greek_alpha"                , 0x07e1   , 0x03b1 }, { "Greek_beta"                 , 0x07e2   , 0x03b2 }, { "Greek_gamma"                , 0x07e3   , 0x03b3 }, { "Greek_delta"                , 0x07e4   , 0x03b4 }, { "Greek_epsilon"              , 0x07e5   , 0x03b5 }, { "Greek_zeta"                 , 0x07e6   , 0x03b6 }, { "Greek_eta"                  , 0x07e7   , 0x03b7 }, { "Greek_theta"                , 0x07e8   , 0x03b8 },
                { "Greek_iota"                 , 0x07e9   , 0x03b9 }, { "Greek_kappa"                , 0x07ea   , 0x03ba }, { "Greek_lamda"                , 0x07eb   , 0x03bb }, { "Greek_lambda"               , 0x07eb   , 0      }, { "Greek_mu"                   , 0x07ec   , 0x03bc }, { "Greek_nu"                   , 0x07ed   , 0x03bd }, { "Greek_xi"                   , 0x07ee   , 0x03be }, { "Greek_omicron"              , 0x07ef   , 0x03bf }, { "Greek_pi"                   , 0x07f0   , 0x03c0 }, { "Greek_rho"                  , 0x07f1   , 0x03c1 }, { "Greek_sigma"                , 0x07f2   , 0x03c3 }, { "Greek_finalsmallsigma"      , 0x07f3   , 0x03c2 }, { "Greek_tau"                  , 0x07f4   , 0x03c4 }, { "Greek_upsilon"              , 0x07f5   , 0x03c5 }, { "Greek_phi"                  , 0x07f6   , 0x03c6 }, { "Greek_chi"                  , 0x07f7   , 0x03c7 }, { "Greek_psi"                  , 0x07f8   , 0x03c8 }, { "Greek_omega"                , 0x07f9   , 0x03c9 }, { "Greek_switch"               , 0xff7e   , 0      }, { "leftradical"                , 0x08a1   , 0x23b7 }, { "topleftradical"             , 0x08a2   , 0x250c }, { "horizconnector"             , 0x08a3   , 0x2500 }, { "topintegral"                , 0x08a4   , 0x2320 }, { "botintegral"                , 0x08a5   , 0x2321 }, { "vertconnector"              , 0x08a6   , 0x2502 }, { "topleftsqbracket"           , 0x08a7   , 0x23a1 }, { "botleftsqbracket"           , 0x08a8   , 0x23a3 }, { "toprightsqbracket"          , 0x08a9   , 0x23a4 }, { "botrightsqbracket"          , 0x08aa   , 0x23a6 }, { "topleftparens"              , 0x08ab   , 0x239b },
                { "botleftparens"              , 0x08ac   , 0x239d }, { "toprightparens"             , 0x08ad   , 0x239e }, { "botrightparens"             , 0x08ae   , 0x23a0 }, { "leftmiddlecurlybrace"       , 0x08af   , 0x23a8 }, { "rightmiddlecurlybrace"      , 0x08b0   , 0x23ac }, { "topleftsummation"           , 0x08b1   , 0      }, { "botleftsummation"           , 0x08b2   , 0      }, { "topvertsummationconnector"  , 0x08b3   , 0      }, { "botvertsummationconnector"  , 0x08b4   , 0      }, { "toprightsummation"          , 0x08b5   , 0      }, { "botrightsummation"          , 0x08b6   , 0      }, { "rightmiddlesummation"       , 0x08b7   , 0      }, { "lessthanequal"              , 0x08bc   , 0x2264 }, { "notequal"                   , 0x08bd   , 0x2260 }, { "greaterthanequal"           , 0x08be   , 0x2265 }, { "integral"                   , 0x08bf   , 0x222b }, { "therefore"                  , 0x08c0   , 0x2234 }, { "variation"                  , 0x08c1   , 0x221d }, { "infinity"                   , 0x08c2   , 0x221e }, { "nabla"                      , 0x08c5   , 0x2207 }, { "approximate"                , 0x08c8   , 0x223c }, { "similarequal"               , 0x08c9   , 0x2243 }, { "ifonlyif"                   , 0x08cd   , 0x21d4 }, { "implies"                    , 0x08ce   , 0x21d2 }, { "identical"                  , 0x08cf   , 0x2261 }, { "radical"                    , 0x08d6   , 0x221a }, { "includedin"                 , 0x08da   , 0x2282 }, { "includes"                   , 0x08db   , 0x2283 }, { "intersection"               , 0x08dc   , 0x2229 }, { "union"                      , 0x08dd   , 0x222a },
                { "logicaland"                 , 0x08de   , 0x2227 }, { "logicalor"                  , 0x08df   , 0x2228 }, { "partialderivative"          , 0x08ef   , 0x2202 }, { "function"                   , 0x08f6   , 0x0192 }, { "leftarrow"                  , 0x08fb   , 0x2190 }, { "uparrow"                    , 0x08fc   , 0x2191 }, { "rightarrow"                 , 0x08fd   , 0x2192 }, { "downarrow"                  , 0x08fe   , 0x2193 }, { "blank"                      , 0x09df   , 0      }, { "soliddiamond"               , 0x09e0   , 0x25c6 }, { "checkerboard"               , 0x09e1   , 0x2592 }, { "ht"                         , 0x09e2   , 0x2409 }, { "ff"                         , 0x09e3   , 0x240c }, { "cr"                         , 0x09e4   , 0x240d }, { "lf"                         , 0x09e5   , 0x240a }, { "nl"                         , 0x09e8   , 0x2424 }, { "vt"                         , 0x09e9   , 0x240b }, { "lowrightcorner"             , 0x09ea   , 0x2518 }, { "uprightcorner"              , 0x09eb   , 0x2510 }, { "upleftcorner"               , 0x09ec   , 0x250c }, { "lowleftcorner"              , 0x09ed   , 0x2514 }, { "crossinglines"              , 0x09ee   , 0x253c }, { "horizlinescan1"             , 0x09ef   , 0x23ba }, { "horizlinescan3"             , 0x09f0   , 0x23bb }, { "horizlinescan5"             , 0x09f1   , 0x2500 }, { "horizlinescan7"             , 0x09f2   , 0x23bc }, { "horizlinescan9"             , 0x09f3   , 0x23bd }, { "leftt"                      , 0x09f4   , 0x251c }, { "rightt"                     , 0x09f5   , 0x2524 }, { "bott"                       , 0x09f6   , 0x2534 },
                { "topt"                       , 0x09f7   , 0x252c }, { "vertbar"                    , 0x09f8   , 0x2502 }, { "emspace"                    , 0x0aa1   , 0x2003 }, { "enspace"                    , 0x0aa2   , 0x2002 }, { "em3space"                   , 0x0aa3   , 0x2004 }, { "em4space"                   , 0x0aa4   , 0x2005 }, { "digitspace"                 , 0x0aa5   , 0x2007 }, { "punctspace"                 , 0x0aa6   , 0x2008 }, { "thinspace"                  , 0x0aa7   , 0x2009 }, { "hairspace"                  , 0x0aa8   , 0x200a }, { "emdash"                     , 0x0aa9   , 0x2014 }, { "endash"                     , 0x0aaa   , 0x2013 }, { "signifblank"                , 0x0aac   , 0x2423 }, { "ellipsis"                   , 0x0aae   , 0x2026 }, { "doubbaselinedot"            , 0x0aaf   , 0x2025 }, { "onethird"                   , 0x0ab0   , 0x2153 }, { "twothirds"                  , 0x0ab1   , 0x2154 }, { "onefifth"                   , 0x0ab2   , 0x2155 }, { "twofifths"                  , 0x0ab3   , 0x2156 }, { "threefifths"                , 0x0ab4   , 0x2157 }, { "fourfifths"                 , 0x0ab5   , 0x2158 }, { "onesixth"                   , 0x0ab6   , 0x2159 }, { "fivesixths"                 , 0x0ab7   , 0x215a }, { "careof"                     , 0x0ab8   , 0x2105 }, { "figdash"                    , 0x0abb   , 0x2012 }, { "leftanglebracket"           , 0x0abc   , 0x2329 }, { "decimalpoint"               , 0x0abd   , 0x002e }, { "rightanglebracket"          , 0x0abe   , 0x232a }, { "marker"                     , 0x0abf   , 0      }, { "oneeighth"                  , 0x0ac3   , 0x215b },
                { "threeeighths"               , 0x0ac4   , 0x215c }, { "fiveeighths"                , 0x0ac5   , 0x215d }, { "seveneighths"               , 0x0ac6   , 0x215e }, { "trademark"                  , 0x0ac9   , 0x2122 }, { "signaturemark"              , 0x0aca   , 0x2613 }, { "trademarkincircle"          , 0x0acb   , 0      }, { "leftopentriangle"           , 0x0acc   , 0x25c1 }, { "rightopentriangle"          , 0x0acd   , 0x25b7 }, { "emopencircle"               , 0x0ace   , 0x25cb }, { "emopenrectangle"            , 0x0acf   , 0x25af }, { "leftsinglequotemark"        , 0x0ad0   , 0x2018 }, { "rightsinglequotemark"       , 0x0ad1   , 0x2019 }, { "leftdoublequotemark"        , 0x0ad2   , 0x201c }, { "rightdoublequotemark"       , 0x0ad3   , 0x201d }, { "prescription"               , 0x0ad4   , 0x211e }, { "permille"                   , 0x0ad5   , 0x2030 }, { "minutes"                    , 0x0ad6   , 0x2032 }, { "seconds"                    , 0x0ad7   , 0x2033 }, { "latincross"                 , 0x0ad9   , 0x271d }, { "hexagram"                   , 0x0ada   , 0      }, { "filledrectbullet"           , 0x0adb   , 0x25ac }, { "filledlefttribullet"        , 0x0adc   , 0x25c0 }, { "filledrighttribullet"       , 0x0add   , 0x25b6 }, { "emfilledcircle"             , 0x0ade   , 0x25cf }, { "emfilledrect"               , 0x0adf   , 0x25ae }, { "enopencircbullet"           , 0x0ae0   , 0x25e6 }, { "enopensquarebullet"         , 0x0ae1   , 0x25ab }, { "openrectbullet"             , 0x0ae2   , 0x25ad }, { "opentribulletup"            , 0x0ae3   , 0x25b3 }, { "opentribulletdown"          , 0x0ae4   , 0x25bd },
                { "openstar"                   , 0x0ae5   , 0x2606 }, { "enfilledcircbullet"         , 0x0ae6   , 0x2022 }, { "enfilledsqbullet"           , 0x0ae7   , 0x25aa }, { "filledtribulletup"          , 0x0ae8   , 0x25b2 }, { "filledtribulletdown"        , 0x0ae9   , 0x25bc }, { "leftpointer"                , 0x0aea   , 0x261c }, { "rightpointer"               , 0x0aeb   , 0x261e }, { "club"                       , 0x0aec   , 0x2663 }, { "diamond"                    , 0x0aed   , 0x2666 }, { "heart"                      , 0x0aee   , 0x2665 }, { "maltesecross"               , 0x0af0   , 0x2720 }, { "dagger"                     , 0x0af1   , 0x2020 }, { "doubledagger"               , 0x0af2   , 0x2021 }, { "checkmark"                  , 0x0af3   , 0x2713 }, { "ballotcross"                , 0x0af4   , 0x2717 }, { "musicalsharp"               , 0x0af5   , 0x266f }, { "musicalflat"                , 0x0af6   , 0x266d }, { "malesymbol"                 , 0x0af7   , 0x2642 }, { "femalesymbol"               , 0x0af8   , 0x2640 }, { "telephone"                  , 0x0af9   , 0x260e }, { "telephonerecorder"          , 0x0afa   , 0x2315 }, { "phonographcopyright"        , 0x0afb   , 0x2117 }, { "caret"                      , 0x0afc   , 0x2038 }, { "singlelowquotemark"         , 0x0afd   , 0x201a }, { "doublelowquotemark"         , 0x0afe   , 0x201e }, { "cursor"                     , 0x0aff   , 0      }, { "leftcaret"                  , 0x0ba3   , 0x003c }, { "rightcaret"                 , 0x0ba6   , 0x003e }, { "downcaret"                  , 0x0ba8   , 0x2228 }, { "upcaret"                    , 0x0ba9   , 0x2227 },
                { "overbar"                    , 0x0bc0   , 0x00af }, { "downtack"                   , 0x0bc2   , 0x22a4 }, { "upshoe"                     , 0x0bc3   , 0x2229 }, { "downstile"                  , 0x0bc4   , 0x230a }, { "underbar"                   , 0x0bc6   , 0x005f }, { "jot"                        , 0x0bca   , 0x2218 }, { "quad"                       , 0x0bcc   , 0x2395 }, { "uptack"                     , 0x0bce   , 0x22a5 }, { "circle"                     , 0x0bcf   , 0x25cb }, { "upstile"                    , 0x0bd3   , 0x2308 }, { "downshoe"                   , 0x0bd6   , 0x222a }, { "rightshoe"                  , 0x0bd8   , 0x2283 }, { "leftshoe"                   , 0x0bda   , 0x2282 }, { "lefttack"                   , 0x0bdc   , 0x22a3 }, { "righttack"                  , 0x0bfc   , 0x22a2 }, { "hebrew_doublelowline"       , 0x0cdf   , 0x2017 }, { "hebrew_aleph"               , 0x0ce0   , 0x05d0 }, { "hebrew_bet"                 , 0x0ce1   , 0x05d1 }, { "hebrew_beth"                , 0x0ce1   , 0      }, { "hebrew_gimel"               , 0x0ce2   , 0x05d2 }, { "hebrew_gimmel"              , 0x0ce2   , 0      }, { "hebrew_dalet"               , 0x0ce3   , 0x05d3 }, { "hebrew_daleth"              , 0x0ce3   , 0      }, { "hebrew_he"                  , 0x0ce4   , 0x05d4 }, { "hebrew_waw"                 , 0x0ce5   , 0x05d5 }, { "hebrew_zain"                , 0x0ce6   , 0x05d6 }, { "hebrew_zayin"               , 0x0ce6   , 0      }, { "hebrew_chet"                , 0x0ce7   , 0x05d7 }, { "hebrew_het"                 , 0x0ce7   , 0      }, { "hebrew_tet"                 , 0x0ce8   , 0x05d8 },
                { "hebrew_teth"                , 0x0ce8   , 0      }, { "hebrew_yod"                 , 0x0ce9   , 0x05d9 }, { "hebrew_finalkaph"           , 0x0cea   , 0x05da }, { "hebrew_kaph"                , 0x0ceb   , 0x05db }, { "hebrew_lamed"               , 0x0cec   , 0x05dc }, { "hebrew_finalmem"            , 0x0ced   , 0x05dd }, { "hebrew_mem"                 , 0x0cee   , 0x05de }, { "hebrew_finalnun"            , 0x0cef   , 0x05df }, { "hebrew_nun"                 , 0x0cf0   , 0x05e0 }, { "hebrew_samech"              , 0x0cf1   , 0x05e1 }, { "hebrew_samekh"              , 0x0cf1   , 0      }, { "hebrew_ayin"                , 0x0cf2   , 0x05e2 }, { "hebrew_finalpe"             , 0x0cf3   , 0x05e3 }, { "hebrew_pe"                  , 0x0cf4   , 0x05e4 }, { "hebrew_finalzade"           , 0x0cf5   , 0x05e5 }, { "hebrew_finalzadi"           , 0x0cf5   , 0      }, { "hebrew_zade"                , 0x0cf6   , 0x05e6 }, { "hebrew_zadi"                , 0x0cf6   , 0      }, { "hebrew_qoph"                , 0x0cf7   , 0x05e7 }, { "hebrew_kuf"                 , 0x0cf7   , 0      }, { "hebrew_resh"                , 0x0cf8   , 0x05e8 }, { "hebrew_shin"                , 0x0cf9   , 0x05e9 }, { "hebrew_taw"                 , 0x0cfa   , 0x05ea }, { "hebrew_taf"                 , 0x0cfa   , 0      }, { "Hebrew_switch"              , 0xff7e   , 0      }, { "Thai_kokai"                 , 0x0da1   , 0x0e01 }, { "Thai_khokhai"               , 0x0da2   , 0x0e02 }, { "Thai_khokhuat"              , 0x0da3   , 0x0e03 }, { "Thai_khokhwai"              , 0x0da4   , 0x0e04 }, { "Thai_khokhon"               , 0x0da5   , 0x0e05 },
                { "Thai_khorakhang"            , 0x0da6   , 0x0e06 }, { "Thai_ngongu"                , 0x0da7   , 0x0e07 }, { "Thai_chochan"               , 0x0da8   , 0x0e08 }, { "Thai_choching"              , 0x0da9   , 0x0e09 }, { "Thai_chochang"              , 0x0daa   , 0x0e0a }, { "Thai_soso"                  , 0x0dab   , 0x0e0b }, { "Thai_chochoe"               , 0x0dac   , 0x0e0c }, { "Thai_yoying"                , 0x0dad   , 0x0e0d }, { "Thai_dochada"               , 0x0dae   , 0x0e0e }, { "Thai_topatak"               , 0x0daf   , 0x0e0f }, { "Thai_thothan"               , 0x0db0   , 0x0e10 }, { "Thai_thonangmontho"         , 0x0db1   , 0x0e11 }, { "Thai_thophuthao"            , 0x0db2   , 0x0e12 }, { "Thai_nonen"                 , 0x0db3   , 0x0e13 }, { "Thai_dodek"                 , 0x0db4   , 0x0e14 }, { "Thai_totao"                 , 0x0db5   , 0x0e15 }, { "Thai_thothung"              , 0x0db6   , 0x0e16 }, { "Thai_thothahan"             , 0x0db7   , 0x0e17 }, { "Thai_thothong"              , 0x0db8   , 0x0e18 }, { "Thai_nonu"                  , 0x0db9   , 0x0e19 }, { "Thai_bobaimai"              , 0x0dba   , 0x0e1a }, { "Thai_popla"                 , 0x0dbb   , 0x0e1b }, { "Thai_phophung"              , 0x0dbc   , 0x0e1c }, { "Thai_fofa"                  , 0x0dbd   , 0x0e1d }, { "Thai_phophan"               , 0x0dbe   , 0x0e1e }, { "Thai_fofan"                 , 0x0dbf   , 0x0e1f }, { "Thai_phosamphao"            , 0x0dc0   , 0x0e20 }, { "Thai_moma"                  , 0x0dc1   , 0x0e21 }, { "Thai_yoyak"                 , 0x0dc2   , 0x0e22 }, { "Thai_rorua"                 , 0x0dc3   , 0x0e23 },
                { "Thai_ru"                    , 0x0dc4   , 0x0e24 }, { "Thai_loling"                , 0x0dc5   , 0x0e25 }, { "Thai_lu"                    , 0x0dc6   , 0x0e26 }, { "Thai_wowaen"                , 0x0dc7   , 0x0e27 }, { "Thai_sosala"                , 0x0dc8   , 0x0e28 }, { "Thai_sorusi"                , 0x0dc9   , 0x0e29 }, { "Thai_sosua"                 , 0x0dca   , 0x0e2a }, { "Thai_hohip"                 , 0x0dcb   , 0x0e2b }, { "Thai_lochula"               , 0x0dcc   , 0x0e2c }, { "Thai_oang"                  , 0x0dcd   , 0x0e2d }, { "Thai_honokhuk"              , 0x0dce   , 0x0e2e }, { "Thai_paiyannoi"             , 0x0dcf   , 0x0e2f }, { "Thai_saraa"                 , 0x0dd0   , 0x0e30 }, { "Thai_maihanakat"            , 0x0dd1   , 0x0e31 }, { "Thai_saraaa"                , 0x0dd2   , 0x0e32 }, { "Thai_saraam"                , 0x0dd3   , 0x0e33 }, { "Thai_sarai"                 , 0x0dd4   , 0x0e34 }, { "Thai_saraii"                , 0x0dd5   , 0x0e35 }, { "Thai_saraue"                , 0x0dd6   , 0x0e36 }, { "Thai_sarauee"               , 0x0dd7   , 0x0e37 }, { "Thai_sarau"                 , 0x0dd8   , 0x0e38 }, { "Thai_sarauu"                , 0x0dd9   , 0x0e39 }, { "Thai_phinthu"               , 0x0dda   , 0x0e3a }, { "Thai_maihanakat_maitho"     , 0x0dde   , 0x0e3e }, { "Thai_baht"                  , 0x0ddf   , 0x0e3f }, { "Thai_sarae"                 , 0x0de0   , 0x0e40 }, { "Thai_saraae"                , 0x0de1   , 0x0e41 }, { "Thai_sarao"                 , 0x0de2   , 0x0e42 }, { "Thai_saraaimaimuan"         , 0x0de3   , 0x0e43 }, { "Thai_saraaimaimalai"        , 0x0de4   , 0x0e44 },
                { "Thai_lakkhangyao"           , 0x0de5   , 0x0e45 }, { "Thai_maiyamok"              , 0x0de6   , 0x0e46 }, { "Thai_maitaikhu"             , 0x0de7   , 0x0e47 }, { "Thai_maiek"                 , 0x0de8   , 0x0e48 }, { "Thai_maitho"                , 0x0de9   , 0x0e49 }, { "Thai_maitri"                , 0x0dea   , 0x0e4a }, { "Thai_maichattawa"           , 0x0deb   , 0x0e4b }, { "Thai_thanthakhat"           , 0x0dec   , 0x0e4c }, { "Thai_nikhahit"              , 0x0ded   , 0x0e4d }, { "Thai_leksun"                , 0x0df0   , 0x0e50 }, { "Thai_leknung"               , 0x0df1   , 0x0e51 }, { "Thai_leksong"               , 0x0df2   , 0x0e52 }, { "Thai_leksam"                , 0x0df3   , 0x0e53 }, { "Thai_leksi"                 , 0x0df4   , 0x0e54 }, { "Thai_lekha"                 , 0x0df5   , 0x0e55 }, { "Thai_lekhok"                , 0x0df6   , 0x0e56 }, { "Thai_lekchet"               , 0x0df7   , 0x0e57 }, { "Thai_lekpaet"               , 0x0df8   , 0x0e58 }, { "Thai_lekkao"                , 0x0df9   , 0x0e59 }, { "Hangul"                     , 0xff31   , 0      }, { "Hangul_Start"               , 0xff32   , 0      }, { "Hangul_End"                 , 0xff33   , 0      }, { "Hangul_Hanja"               , 0xff34   , 0      }, { "Hangul_Jamo"                , 0xff35   , 0      }, { "Hangul_Romaja"              , 0xff36   , 0      }, { "Hangul_Codeinput"           , 0xff37   , 0      }, { "Hangul_Jeonja"              , 0xff38   , 0      }, { "Hangul_Banja"               , 0xff39   , 0      }, { "Hangul_PreHanja"            , 0xff3a   , 0      }, { "Hangul_PostHanja"           , 0xff3b   , 0      },
                { "Hangul_SingleCandidate"     , 0xff3c   , 0      }, { "Hangul_MultipleCandidate"   , 0xff3d   , 0      }, { "Hangul_PreviousCandidate"   , 0xff3e   , 0      }, { "Hangul_Special"             , 0xff3f   , 0      }, { "Hangul_switch"              , 0xff7e   , 0      }, { "Hangul_Kiyeog"              , 0x0ea1   , 0x3131 }, { "Hangul_SsangKiyeog"         , 0x0ea2   , 0x3132 }, { "Hangul_KiyeogSios"          , 0x0ea3   , 0x3133 }, { "Hangul_Nieun"               , 0x0ea4   , 0x3134 }, { "Hangul_NieunJieuj"          , 0x0ea5   , 0x3135 }, { "Hangul_NieunHieuh"          , 0x0ea6   , 0x3136 }, { "Hangul_Dikeud"              , 0x0ea7   , 0x3137 }, { "Hangul_SsangDikeud"         , 0x0ea8   , 0x3138 }, { "Hangul_Rieul"               , 0x0ea9   , 0x3139 }, { "Hangul_RieulKiyeog"         , 0x0eaa   , 0x313a }, { "Hangul_RieulMieum"          , 0x0eab   , 0x313b }, { "Hangul_RieulPieub"          , 0x0eac   , 0x313c }, { "Hangul_RieulSios"           , 0x0ead   , 0x313d }, { "Hangul_RieulTieut"          , 0x0eae   , 0x313e }, { "Hangul_RieulPhieuf"         , 0x0eaf   , 0x313f }, { "Hangul_RieulHieuh"          , 0x0eb0   , 0x3140 }, { "Hangul_Mieum"               , 0x0eb1   , 0x3141 }, { "Hangul_Pieub"               , 0x0eb2   , 0x3142 }, { "Hangul_SsangPieub"          , 0x0eb3   , 0x3143 }, { "Hangul_PieubSios"           , 0x0eb4   , 0x3144 }, { "Hangul_Sios"                , 0x0eb5   , 0x3145 }, { "Hangul_SsangSios"           , 0x0eb6   , 0x3146 }, { "Hangul_Ieung"               , 0x0eb7   , 0x3147 }, { "Hangul_Jieuj"               , 0x0eb8   , 0x3148 }, { "Hangul_SsangJieuj"          , 0x0eb9   , 0x3149 },
                { "Hangul_Cieuc"               , 0x0eba   , 0x314a }, { "Hangul_Khieuq"              , 0x0ebb   , 0x314b }, { "Hangul_Tieut"               , 0x0ebc   , 0x314c }, { "Hangul_Phieuf"              , 0x0ebd   , 0x314d }, { "Hangul_Hieuh"               , 0x0ebe   , 0x314e }, { "Hangul_A"                   , 0x0ebf   , 0x314f }, { "Hangul_AE"                  , 0x0ec0   , 0x3150 }, { "Hangul_YA"                  , 0x0ec1   , 0x3151 }, { "Hangul_YAE"                 , 0x0ec2   , 0x3152 }, { "Hangul_EO"                  , 0x0ec3   , 0x3153 }, { "Hangul_E"                   , 0x0ec4   , 0x3154 }, { "Hangul_YEO"                 , 0x0ec5   , 0x3155 }, { "Hangul_YE"                  , 0x0ec6   , 0x3156 }, { "Hangul_O"                   , 0x0ec7   , 0x3157 }, { "Hangul_WA"                  , 0x0ec8   , 0x3158 }, { "Hangul_WAE"                 , 0x0ec9   , 0x3159 }, { "Hangul_OE"                  , 0x0eca   , 0x315a }, { "Hangul_YO"                  , 0x0ecb   , 0x315b }, { "Hangul_U"                   , 0x0ecc   , 0x315c }, { "Hangul_WEO"                 , 0x0ecd   , 0x315d }, { "Hangul_WE"                  , 0x0ece   , 0x315e }, { "Hangul_WI"                  , 0x0ecf   , 0x315f }, { "Hangul_YU"                  , 0x0ed0   , 0x3160 }, { "Hangul_EU"                  , 0x0ed1   , 0x3161 }, { "Hangul_YI"                  , 0x0ed2   , 0x3162 }, { "Hangul_I"                   , 0x0ed3   , 0x3163 }, { "Hangul_J_Kiyeog"            , 0x0ed4   , 0x11a8 }, { "Hangul_J_SsangKiyeog"       , 0x0ed5   , 0x11a9 }, { "Hangul_J_KiyeogSios"        , 0x0ed6   , 0x11aa }, { "Hangul_J_Nieun"             , 0x0ed7   , 0x11ab },
                { "Hangul_J_NieunJieuj"        , 0x0ed8   , 0x11ac }, { "Hangul_J_NieunHieuh"        , 0x0ed9   , 0x11ad }, { "Hangul_J_Dikeud"            , 0x0eda   , 0x11ae }, { "Hangul_J_Rieul"             , 0x0edb   , 0x11af }, { "Hangul_J_RieulKiyeog"       , 0x0edc   , 0x11b0 }, { "Hangul_J_RieulMieum"        , 0x0edd   , 0x11b1 }, { "Hangul_J_RieulPieub"        , 0x0ede   , 0x11b2 }, { "Hangul_J_RieulSios"         , 0x0edf   , 0x11b3 }, { "Hangul_J_RieulTieut"        , 0x0ee0   , 0x11b4 }, { "Hangul_J_RieulPhieuf"       , 0x0ee1   , 0x11b5 }, { "Hangul_J_RieulHieuh"        , 0x0ee2   , 0x11b6 }, { "Hangul_J_Mieum"             , 0x0ee3   , 0x11b7 }, { "Hangul_J_Pieub"             , 0x0ee4   , 0x11b8 }, { "Hangul_J_PieubSios"         , 0x0ee5   , 0x11b9 }, { "Hangul_J_Sios"              , 0x0ee6   , 0x11ba }, { "Hangul_J_SsangSios"         , 0x0ee7   , 0x11bb }, { "Hangul_J_Ieung"             , 0x0ee8   , 0x11bc }, { "Hangul_J_Jieuj"             , 0x0ee9   , 0x11bd }, { "Hangul_J_Cieuc"             , 0x0eea   , 0x11be }, { "Hangul_J_Khieuq"            , 0x0eeb   , 0x11bf }, { "Hangul_J_Tieut"             , 0x0eec   , 0x11c0 }, { "Hangul_J_Phieuf"            , 0x0eed   , 0x11c1 }, { "Hangul_J_Hieuh"             , 0x0eee   , 0x11c2 }, { "Hangul_RieulYeorinHieuh"    , 0x0eef   , 0x316d }, { "Hangul_SunkyeongeumMieum"   , 0x0ef0   , 0x3171 }, { "Hangul_SunkyeongeumPieub"   , 0x0ef1   , 0x3178 }, { "Hangul_PanSios"             , 0x0ef2   , 0x317f }, { "Hangul_KkogjiDalrinIeung"   , 0x0ef3   , 0x3181 }, { "Hangul_SunkyeongeumPhieuf"  , 0x0ef4   , 0x3184 }, { "Hangul_YeorinHieuh"         , 0x0ef5   , 0x3186 },
                { "Hangul_AraeA"               , 0x0ef6   , 0x318d }, { "Hangul_AraeAE"              , 0x0ef7   , 0x318e }, { "Hangul_J_PanSios"           , 0x0ef8   , 0x11eb }, { "Hangul_J_KkogjiDalrinIeung" , 0x0ef9   , 0x11f0 }, { "Hangul_J_YeorinHieuh"       , 0x0efa   , 0x11f9 }, { "Korean_Won"                 , 0x0eff   , 0x20a9 }, { "Armenian_ligature_ew"       , 0x1000587, 0x0587 }, { "Armenian_full_stop"         , 0x1000589, 0x0589 }, { "Armenian_verjaket"          , 0x1000589, 0      }, { "Armenian_separation_mark"   , 0x100055d, 0x055d }, { "Armenian_but"               , 0x100055d, 0      }, { "Armenian_hyphen"            , 0x100058a, 0x058a }, { "Armenian_yentamna"          , 0x100058a, 0      }, { "Armenian_exclam"            , 0x100055c, 0x055c }, { "Armenian_amanak"            , 0x100055c, 0      }, { "Armenian_accent"            , 0x100055b, 0x055b }, { "Armenian_shesht"            , 0x100055b, 0      }, { "Armenian_question"          , 0x100055e, 0x055e }, { "Armenian_paruyk"            , 0x100055e, 0      }, { "Armenian_AYB"               , 0x1000531, 0x0531 }, { "Armenian_ayb"               , 0x1000561, 0x0561 }, { "Armenian_BEN"               , 0x1000532, 0x0532 }, { "Armenian_ben"               , 0x1000562, 0x0562 }, { "Armenian_GIM"               , 0x1000533, 0x0533 }, { "Armenian_gim"               , 0x1000563, 0x0563 }, { "Armenian_DA"                , 0x1000534, 0x0534 }, { "Armenian_da"                , 0x1000564, 0x0564 }, { "Armenian_YECH"              , 0x1000535, 0x0535 }, { "Armenian_yech"              , 0x1000565, 0x0565 }, { "Armenian_ZA"                , 0x1000536, 0x0536 },
                { "Armenian_za"                , 0x1000566, 0x0566 }, { "Armenian_E"                 , 0x1000537, 0x0537 }, { "Armenian_e"                 , 0x1000567, 0x0567 }, { "Armenian_AT"                , 0x1000538, 0x0538 }, { "Armenian_at"                , 0x1000568, 0x0568 }, { "Armenian_TO"                , 0x1000539, 0x0539 }, { "Armenian_to"                , 0x1000569, 0x0569 }, { "Armenian_ZHE"               , 0x100053a, 0x053a }, { "Armenian_zhe"               , 0x100056a, 0x056a }, { "Armenian_INI"               , 0x100053b, 0x053b }, { "Armenian_ini"               , 0x100056b, 0x056b }, { "Armenian_LYUN"              , 0x100053c, 0x053c }, { "Armenian_lyun"              , 0x100056c, 0x056c }, { "Armenian_KHE"               , 0x100053d, 0x053d }, { "Armenian_khe"               , 0x100056d, 0x056d }, { "Armenian_TSA"               , 0x100053e, 0x053e }, { "Armenian_tsa"               , 0x100056e, 0x056e }, { "Armenian_KEN"               , 0x100053f, 0x053f }, { "Armenian_ken"               , 0x100056f, 0x056f }, { "Armenian_HO"                , 0x1000540, 0x0540 }, { "Armenian_ho"                , 0x1000570, 0x0570 }, { "Armenian_DZA"               , 0x1000541, 0x0541 }, { "Armenian_dza"               , 0x1000571, 0x0571 }, { "Armenian_GHAT"              , 0x1000542, 0x0542 }, { "Armenian_ghat"              , 0x1000572, 0x0572 }, { "Armenian_TCHE"              , 0x1000543, 0x0543 }, { "Armenian_tche"              , 0x1000573, 0x0573 }, { "Armenian_MEN"               , 0x1000544, 0x0544 }, { "Armenian_men"               , 0x1000574, 0x0574 }, { "Armenian_HI"                , 0x1000545, 0x0545 },
                { "Armenian_hi"                , 0x1000575, 0x0575 }, { "Armenian_NU"                , 0x1000546, 0x0546 }, { "Armenian_nu"                , 0x1000576, 0x0576 }, { "Armenian_SHA"               , 0x1000547, 0x0547 }, { "Armenian_sha"               , 0x1000577, 0x0577 }, { "Armenian_VO"                , 0x1000548, 0x0548 }, { "Armenian_vo"                , 0x1000578, 0x0578 }, { "Armenian_CHA"               , 0x1000549, 0x0549 }, { "Armenian_cha"               , 0x1000579, 0x0579 }, { "Armenian_PE"                , 0x100054a, 0x054a }, { "Armenian_pe"                , 0x100057a, 0x057a }, { "Armenian_JE"                , 0x100054b, 0x054b }, { "Armenian_je"                , 0x100057b, 0x057b }, { "Armenian_RA"                , 0x100054c, 0x054c }, { "Armenian_ra"                , 0x100057c, 0x057c }, { "Armenian_SE"                , 0x100054d, 0x054d }, { "Armenian_se"                , 0x100057d, 0x057d }, { "Armenian_VEV"               , 0x100054e, 0x054e }, { "Armenian_vev"               , 0x100057e, 0x057e }, { "Armenian_TYUN"              , 0x100054f, 0x054f }, { "Armenian_tyun"              , 0x100057f, 0x057f }, { "Armenian_RE"                , 0x1000550, 0x0550 }, { "Armenian_re"                , 0x1000580, 0x0580 }, { "Armenian_TSO"               , 0x1000551, 0x0551 }, { "Armenian_tso"               , 0x1000581, 0x0581 }, { "Armenian_VYUN"              , 0x1000552, 0x0552 }, { "Armenian_vyun"              , 0x1000582, 0x0582 }, { "Armenian_PYUR"              , 0x1000553, 0x0553 }, { "Armenian_pyur"              , 0x1000583, 0x0583 }, { "Armenian_KE"                , 0x1000554, 0x0554 },
                { "Armenian_ke"                , 0x1000584, 0x0584 }, { "Armenian_O"                 , 0x1000555, 0x0555 }, { "Armenian_o"                 , 0x1000585, 0x0585 }, { "Armenian_FE"                , 0x1000556, 0x0556 }, { "Armenian_fe"                , 0x1000586, 0x0586 }, { "Armenian_apostrophe"        , 0x100055a, 0x055a }, { "Georgian_an"                , 0x10010d0, 0x10d0 }, { "Georgian_ban"               , 0x10010d1, 0x10d1 }, { "Georgian_gan"               , 0x10010d2, 0x10d2 }, { "Georgian_don"               , 0x10010d3, 0x10d3 }, { "Georgian_en"                , 0x10010d4, 0x10d4 }, { "Georgian_vin"               , 0x10010d5, 0x10d5 }, { "Georgian_zen"               , 0x10010d6, 0x10d6 }, { "Georgian_tan"               , 0x10010d7, 0x10d7 }, { "Georgian_in"                , 0x10010d8, 0x10d8 }, { "Georgian_kan"               , 0x10010d9, 0x10d9 }, { "Georgian_las"               , 0x10010da, 0x10da }, { "Georgian_man"               , 0x10010db, 0x10db }, { "Georgian_nar"               , 0x10010dc, 0x10dc }, { "Georgian_on"                , 0x10010dd, 0x10dd }, { "Georgian_par"               , 0x10010de, 0x10de }, { "Georgian_zhar"              , 0x10010df, 0x10df }, { "Georgian_rae"               , 0x10010e0, 0x10e0 }, { "Georgian_san"               , 0x10010e1, 0x10e1 }, { "Georgian_tar"               , 0x10010e2, 0x10e2 }, { "Georgian_un"                , 0x10010e3, 0x10e3 }, { "Georgian_phar"              , 0x10010e4, 0x10e4 }, { "Georgian_khar"              , 0x10010e5, 0x10e5 }, { "Georgian_ghan"              , 0x10010e6, 0x10e6 }, { "Georgian_qar"               , 0x10010e7, 0x10e7 },
                { "Georgian_shin"              , 0x10010e8, 0x10e8 }, { "Georgian_chin"              , 0x10010e9, 0x10e9 }, { "Georgian_can"               , 0x10010ea, 0x10ea }, { "Georgian_jil"               , 0x10010eb, 0x10eb }, { "Georgian_cil"               , 0x10010ec, 0x10ec }, { "Georgian_char"              , 0x10010ed, 0x10ed }, { "Georgian_xan"               , 0x10010ee, 0x10ee }, { "Georgian_jhan"              , 0x10010ef, 0x10ef }, { "Georgian_hae"               , 0x10010f0, 0x10f0 }, { "Georgian_he"                , 0x10010f1, 0x10f1 }, { "Georgian_hie"               , 0x10010f2, 0x10f2 }, { "Georgian_we"                , 0x10010f3, 0x10f3 }, { "Georgian_har"               , 0x10010f4, 0x10f4 }, { "Georgian_hoe"               , 0x10010f5, 0x10f5 }, { "Georgian_fi"                , 0x10010f6, 0x10f6 }, { "Xabovedot"                  , 0x1001e8a, 0x1e8a }, { "Ibreve"                     , 0x100012c, 0x012c }, { "Zstroke"                    , 0x10001b5, 0x01b5 }, { "Gcaron"                     , 0x10001e6, 0x01e6 }, { "Ocaron"                     , 0x10001d1, 0x01d1 }, { "Obarred"                    , 0x100019f, 0x019f }, { "xabovedot"                  , 0x1001e8b, 0x1e8b }, { "ibreve"                     , 0x100012d, 0x012d }, { "zstroke"                    , 0x10001b6, 0x01b6 }, { "gcaron"                     , 0x10001e7, 0x01e7 }, { "ocaron"                     , 0x10001d2, 0x01d2 }, { "obarred"                    , 0x1000275, 0x0275 }, { "SCHWA"                      , 0x100018f, 0x018f }, { "schwa"                      , 0x1000259, 0x0259 }, { "EZH"                        , 0x10001b7, 0x01b7 },
                { "ezh"                        , 0x1000292, 0x0292 }, { "Lbelowdot"                  , 0x1001e36, 0x1e36 }, { "lbelowdot"                  , 0x1001e37, 0x1e37 }, { "Abelowdot"                  , 0x1001ea0, 0x1ea0 }, { "abelowdot"                  , 0x1001ea1, 0x1ea1 }, { "Ahook"                      , 0x1001ea2, 0x1ea2 }, { "ahook"                      , 0x1001ea3, 0x1ea3 }, { "Acircumflexacute"           , 0x1001ea4, 0x1ea4 }, { "acircumflexacute"           , 0x1001ea5, 0x1ea5 }, { "Acircumflexgrave"           , 0x1001ea6, 0x1ea6 }, { "acircumflexgrave"           , 0x1001ea7, 0x1ea7 }, { "Acircumflexhook"            , 0x1001ea8, 0x1ea8 }, { "acircumflexhook"            , 0x1001ea9, 0x1ea9 }, { "Acircumflextilde"           , 0x1001eaa, 0x1eaa }, { "acircumflextilde"           , 0x1001eab, 0x1eab }, { "Acircumflexbelowdot"        , 0x1001eac, 0x1eac }, { "acircumflexbelowdot"        , 0x1001ead, 0x1ead }, { "Abreveacute"                , 0x1001eae, 0x1eae }, { "abreveacute"                , 0x1001eaf, 0x1eaf }, { "Abrevegrave"                , 0x1001eb0, 0x1eb0 }, { "abrevegrave"                , 0x1001eb1, 0x1eb1 }, { "Abrevehook"                 , 0x1001eb2, 0x1eb2 }, { "abrevehook"                 , 0x1001eb3, 0x1eb3 }, { "Abrevetilde"                , 0x1001eb4, 0x1eb4 }, { "abrevetilde"                , 0x1001eb5, 0x1eb5 }, { "Abrevebelowdot"             , 0x1001eb6, 0x1eb6 }, { "abrevebelowdot"             , 0x1001eb7, 0x1eb7 }, { "Ebelowdot"                  , 0x1001eb8, 0x1eb8 }, { "ebelowdot"                  , 0x1001eb9, 0x1eb9 }, { "Ehook"                      , 0x1001eba, 0x1eba },
                { "ehook"                      , 0x1001ebb, 0x1ebb }, { "Etilde"                     , 0x1001ebc, 0x1ebc }, { "etilde"                     , 0x1001ebd, 0x1ebd }, { "Ecircumflexacute"           , 0x1001ebe, 0x1ebe }, { "ecircumflexacute"           , 0x1001ebf, 0x1ebf }, { "Ecircumflexgrave"           , 0x1001ec0, 0x1ec0 }, { "ecircumflexgrave"           , 0x1001ec1, 0x1ec1 }, { "Ecircumflexhook"            , 0x1001ec2, 0x1ec2 }, { "ecircumflexhook"            , 0x1001ec3, 0x1ec3 }, { "Ecircumflextilde"           , 0x1001ec4, 0x1ec4 }, { "ecircumflextilde"           , 0x1001ec5, 0x1ec5 }, { "Ecircumflexbelowdot"        , 0x1001ec6, 0x1ec6 }, { "ecircumflexbelowdot"        , 0x1001ec7, 0x1ec7 }, { "Ihook"                      , 0x1001ec8, 0x1ec8 }, { "ihook"                      , 0x1001ec9, 0x1ec9 }, { "Ibelowdot"                  , 0x1001eca, 0x1eca }, { "ibelowdot"                  , 0x1001ecb, 0x1ecb }, { "Obelowdot"                  , 0x1001ecc, 0x1ecc }, { "obelowdot"                  , 0x1001ecd, 0x1ecd }, { "Ohook"                      , 0x1001ece, 0x1ece }, { "ohook"                      , 0x1001ecf, 0x1ecf }, { "Ocircumflexacute"           , 0x1001ed0, 0x1ed0 }, { "ocircumflexacute"           , 0x1001ed1, 0x1ed1 }, { "Ocircumflexgrave"           , 0x1001ed2, 0x1ed2 }, { "ocircumflexgrave"           , 0x1001ed3, 0x1ed3 }, { "Ocircumflexhook"            , 0x1001ed4, 0x1ed4 }, { "ocircumflexhook"            , 0x1001ed5, 0x1ed5 }, { "Ocircumflextilde"           , 0x1001ed6, 0x1ed6 }, { "ocircumflextilde"           , 0x1001ed7, 0x1ed7 }, { "Ocircumflexbelowdot"        , 0x1001ed8, 0x1ed8 },
                { "ocircumflexbelowdot"        , 0x1001ed9, 0x1ed9 }, { "Ohornacute"                 , 0x1001eda, 0x1eda }, { "ohornacute"                 , 0x1001edb, 0x1edb }, { "Ohorngrave"                 , 0x1001edc, 0x1edc }, { "ohorngrave"                 , 0x1001edd, 0x1edd }, { "Ohornhook"                  , 0x1001ede, 0x1ede }, { "ohornhook"                  , 0x1001edf, 0x1edf }, { "Ohorntilde"                 , 0x1001ee0, 0x1ee0 }, { "ohorntilde"                 , 0x1001ee1, 0x1ee1 }, { "Ohornbelowdot"              , 0x1001ee2, 0x1ee2 }, { "ohornbelowdot"              , 0x1001ee3, 0x1ee3 }, { "Ubelowdot"                  , 0x1001ee4, 0x1ee4 }, { "ubelowdot"                  , 0x1001ee5, 0x1ee5 }, { "Uhook"                      , 0x1001ee6, 0x1ee6 }, { "uhook"                      , 0x1001ee7, 0x1ee7 }, { "Uhornacute"                 , 0x1001ee8, 0x1ee8 }, { "uhornacute"                 , 0x1001ee9, 0x1ee9 }, { "Uhorngrave"                 , 0x1001eea, 0x1eea }, { "uhorngrave"                 , 0x1001eeb, 0x1eeb }, { "Uhornhook"                  , 0x1001eec, 0x1eec }, { "uhornhook"                  , 0x1001eed, 0x1eed }, { "Uhorntilde"                 , 0x1001eee, 0x1eee }, { "uhorntilde"                 , 0x1001eef, 0x1eef }, { "Uhornbelowdot"              , 0x1001ef0, 0x1ef0 }, { "uhornbelowdot"              , 0x1001ef1, 0x1ef1 }, { "Ybelowdot"                  , 0x1001ef4, 0x1ef4 }, { "ybelowdot"                  , 0x1001ef5, 0x1ef5 }, { "Yhook"                      , 0x1001ef6, 0x1ef6 }, { "yhook"                      , 0x1001ef7, 0x1ef7 }, { "Ytilde"                     , 0x1001ef8, 0x1ef8 },
                { "ytilde"                     , 0x1001ef9, 0x1ef9 }, { "Ohorn"                      , 0x10001a0, 0x01a0 }, { "ohorn"                      , 0x10001a1, 0x01a1 }, { "Uhorn"                      , 0x10001af, 0x01af }, { "uhorn"                      , 0x10001b0, 0x01b0 }, { "combining_tilde"            , 0x1000303, 0x0303 }, { "combining_grave"            , 0x1000300, 0x0300 }, { "combining_acute"            , 0x1000301, 0x0301 }, { "combining_hook"             , 0x1000309, 0x0309 }, { "combining_belowdot"         , 0x1000323, 0x0323 }, { "EcuSign"                    , 0x10020a0, 0x20a0 }, { "ColonSign"                  , 0x10020a1, 0x20a1 }, { "CruzeiroSign"               , 0x10020a2, 0x20a2 }, { "FFrancSign"                 , 0x10020a3, 0x20a3 }, { "LiraSign"                   , 0x10020a4, 0x20a4 }, { "MillSign"                   , 0x10020a5, 0x20a5 }, { "NairaSign"                  , 0x10020a6, 0x20a6 }, { "PesetaSign"                 , 0x10020a7, 0x20a7 }, { "RupeeSign"                  , 0x10020a8, 0x20a8 }, { "WonSign"                    , 0x10020a9, 0x20a9 }, { "NewSheqelSign"              , 0x10020aa, 0x20aa }, { "DongSign"                   , 0x10020ab, 0x20ab }, { "EuroSign"                   , 0x20ac   , 0x20ac }, { "zerosuperior"               , 0x1002070, 0x2070 }, { "foursuperior"               , 0x1002074, 0x2074 }, { "fivesuperior"               , 0x1002075, 0x2075 }, { "sixsuperior"                , 0x1002076, 0x2076 }, { "sevensuperior"              , 0x1002077, 0x2077 }, { "eightsuperior"              , 0x1002078, 0x2078 }, { "ninesuperior"               , 0x1002079, 0x2079 },
                { "zerosubscript"              , 0x1002080, 0x2080 }, { "onesubscript"               , 0x1002081, 0x2081 }, { "twosubscript"               , 0x1002082, 0x2082 }, { "threesubscript"             , 0x1002083, 0x2083 }, { "foursubscript"              , 0x1002084, 0x2084 }, { "fivesubscript"              , 0x1002085, 0x2085 }, { "sixsubscript"               , 0x1002086, 0x2086 }, { "sevensubscript"             , 0x1002087, 0x2087 }, { "eightsubscript"             , 0x1002088, 0x2088 }, { "ninesubscript"              , 0x1002089, 0x2089 }, { "partdifferential"           , 0x1002202, 0x2202 }, { "emptyset"                   , 0x1002205, 0x2205 }, { "elementof"                  , 0x1002208, 0x2208 }, { "notelementof"               , 0x1002209, 0x2209 }, { "containsas"                 , 0x100220b, 0x220b }, { "squareroot"                 , 0x100221a, 0x221a }, { "cuberoot"                   , 0x100221b, 0x221b }, { "fourthroot"                 , 0x100221c, 0x221c }, { "dintegral"                  , 0x100222c, 0x222c }, { "tintegral"                  , 0x100222d, 0x222d }, { "because"                    , 0x1002235, 0x2235 }, { "approxeq"                   , 0x1002248, 0x2248 }, { "notapproxeq"                , 0x1002247, 0x2247 }, { "notidentical"               , 0x1002262, 0x2262 }, { "stricteq"                   , 0x1002263, 0x2263 }, { "braille_dot_1"              , 0xfff1   , 0      }, { "braille_dot_2"              , 0xfff2   , 0      }, { "braille_dot_3"              , 0xfff3   , 0      }, { "braille_dot_4"              , 0xfff4   , 0      }, { "braille_dot_5"              , 0xfff5   , 0      },
                { "braille_dot_6"              , 0xfff6   , 0      }, { "braille_dot_7"              , 0xfff7   , 0      }, { "braille_dot_8"              , 0xfff8   , 0      }, { "braille_dot_9"              , 0xfff9   , 0      }, { "braille_dot_10"             , 0xfffa   , 0      }, { "braille_blank"              , 0x1002800, 0x2800 }, { "braille_dots_1"             , 0x1002801, 0x2801 }, { "braille_dots_2"             , 0x1002802, 0x2802 }, { "braille_dots_12"            , 0x1002803, 0x2803 }, { "braille_dots_3"             , 0x1002804, 0x2804 }, { "braille_dots_13"            , 0x1002805, 0x2805 }, { "braille_dots_23"            , 0x1002806, 0x2806 }, { "braille_dots_123"           , 0x1002807, 0x2807 }, { "braille_dots_4"             , 0x1002808, 0x2808 }, { "braille_dots_14"            , 0x1002809, 0x2809 }, { "braille_dots_24"            , 0x100280a, 0x280a }, { "braille_dots_124"           , 0x100280b, 0x280b }, { "braille_dots_34"            , 0x100280c, 0x280c }, { "braille_dots_134"           , 0x100280d, 0x280d }, { "braille_dots_234"           , 0x100280e, 0x280e }, { "braille_dots_1234"          , 0x100280f, 0x280f }, { "braille_dots_5"             , 0x1002810, 0x2810 }, { "braille_dots_15"            , 0x1002811, 0x2811 }, { "braille_dots_25"            , 0x1002812, 0x2812 }, { "braille_dots_125"           , 0x1002813, 0x2813 }, { "braille_dots_35"            , 0x1002814, 0x2814 }, { "braille_dots_135"           , 0x1002815, 0x2815 }, { "braille_dots_235"           , 0x1002816, 0x2816 }, { "braille_dots_1235"          , 0x1002817, 0x2817 }, { "braille_dots_45"            , 0x1002818, 0x2818 },
                { "braille_dots_145"           , 0x1002819, 0x2819 }, { "braille_dots_245"           , 0x100281a, 0x281a }, { "braille_dots_1245"          , 0x100281b, 0x281b }, { "braille_dots_345"           , 0x100281c, 0x281c }, { "braille_dots_1345"          , 0x100281d, 0x281d }, { "braille_dots_2345"          , 0x100281e, 0x281e }, { "braille_dots_12345"         , 0x100281f, 0x281f }, { "braille_dots_6"             , 0x1002820, 0x2820 }, { "braille_dots_16"            , 0x1002821, 0x2821 }, { "braille_dots_26"            , 0x1002822, 0x2822 }, { "braille_dots_126"           , 0x1002823, 0x2823 }, { "braille_dots_36"            , 0x1002824, 0x2824 }, { "braille_dots_136"           , 0x1002825, 0x2825 }, { "braille_dots_236"           , 0x1002826, 0x2826 }, { "braille_dots_1236"          , 0x1002827, 0x2827 }, { "braille_dots_46"            , 0x1002828, 0x2828 }, { "braille_dots_146"           , 0x1002829, 0x2829 }, { "braille_dots_246"           , 0x100282a, 0x282a }, { "braille_dots_1246"          , 0x100282b, 0x282b }, { "braille_dots_346"           , 0x100282c, 0x282c }, { "braille_dots_1346"          , 0x100282d, 0x282d }, { "braille_dots_2346"          , 0x100282e, 0x282e }, { "braille_dots_12346"         , 0x100282f, 0x282f }, { "braille_dots_56"            , 0x1002830, 0x2830 }, { "braille_dots_156"           , 0x1002831, 0x2831 }, { "braille_dots_256"           , 0x1002832, 0x2832 }, { "braille_dots_1256"          , 0x1002833, 0x2833 }, { "braille_dots_356"           , 0x1002834, 0x2834 }, { "braille_dots_1356"          , 0x1002835, 0x2835 }, { "braille_dots_2356"          , 0x1002836, 0x2836 },
                { "braille_dots_12356"         , 0x1002837, 0x2837 }, { "braille_dots_456"           , 0x1002838, 0x2838 }, { "braille_dots_1456"          , 0x1002839, 0x2839 }, { "braille_dots_2456"          , 0x100283a, 0x283a }, { "braille_dots_12456"         , 0x100283b, 0x283b }, { "braille_dots_3456"          , 0x100283c, 0x283c }, { "braille_dots_13456"         , 0x100283d, 0x283d }, { "braille_dots_23456"         , 0x100283e, 0x283e }, { "braille_dots_123456"        , 0x100283f, 0x283f }, { "braille_dots_7"             , 0x1002840, 0x2840 }, { "braille_dots_17"            , 0x1002841, 0x2841 }, { "braille_dots_27"            , 0x1002842, 0x2842 }, { "braille_dots_127"           , 0x1002843, 0x2843 }, { "braille_dots_37"            , 0x1002844, 0x2844 }, { "braille_dots_137"           , 0x1002845, 0x2845 }, { "braille_dots_237"           , 0x1002846, 0x2846 }, { "braille_dots_1237"          , 0x1002847, 0x2847 }, { "braille_dots_47"            , 0x1002848, 0x2848 }, { "braille_dots_147"           , 0x1002849, 0x2849 }, { "braille_dots_247"           , 0x100284a, 0x284a }, { "braille_dots_1247"          , 0x100284b, 0x284b }, { "braille_dots_347"           , 0x100284c, 0x284c }, { "braille_dots_1347"          , 0x100284d, 0x284d }, { "braille_dots_2347"          , 0x100284e, 0x284e }, { "braille_dots_12347"         , 0x100284f, 0x284f }, { "braille_dots_57"            , 0x1002850, 0x2850 }, { "braille_dots_157"           , 0x1002851, 0x2851 }, { "braille_dots_257"           , 0x1002852, 0x2852 }, { "braille_dots_1257"          , 0x1002853, 0x2853 }, { "braille_dots_357"           , 0x1002854, 0x2854 },
                { "braille_dots_1357"          , 0x1002855, 0x2855 }, { "braille_dots_2357"          , 0x1002856, 0x2856 }, { "braille_dots_12357"         , 0x1002857, 0x2857 }, { "braille_dots_457"           , 0x1002858, 0x2858 }, { "braille_dots_1457"          , 0x1002859, 0x2859 }, { "braille_dots_2457"          , 0x100285a, 0x285a }, { "braille_dots_12457"         , 0x100285b, 0x285b }, { "braille_dots_3457"          , 0x100285c, 0x285c }, { "braille_dots_13457"         , 0x100285d, 0x285d }, { "braille_dots_23457"         , 0x100285e, 0x285e }, { "braille_dots_123457"        , 0x100285f, 0x285f }, { "braille_dots_67"            , 0x1002860, 0x2860 }, { "braille_dots_167"           , 0x1002861, 0x2861 }, { "braille_dots_267"           , 0x1002862, 0x2862 }, { "braille_dots_1267"          , 0x1002863, 0x2863 }, { "braille_dots_367"           , 0x1002864, 0x2864 }, { "braille_dots_1367"          , 0x1002865, 0x2865 }, { "braille_dots_2367"          , 0x1002866, 0x2866 }, { "braille_dots_12367"         , 0x1002867, 0x2867 }, { "braille_dots_467"           , 0x1002868, 0x2868 }, { "braille_dots_1467"          , 0x1002869, 0x2869 }, { "braille_dots_2467"          , 0x100286a, 0x286a }, { "braille_dots_12467"         , 0x100286b, 0x286b }, { "braille_dots_3467"          , 0x100286c, 0x286c }, { "braille_dots_13467"         , 0x100286d, 0x286d }, { "braille_dots_23467"         , 0x100286e, 0x286e }, { "braille_dots_123467"        , 0x100286f, 0x286f }, { "braille_dots_567"           , 0x1002870, 0x2870 }, { "braille_dots_1567"          , 0x1002871, 0x2871 }, { "braille_dots_2567"          , 0x1002872, 0x2872 },
                { "braille_dots_12567"         , 0x1002873, 0x2873 }, { "braille_dots_3567"          , 0x1002874, 0x2874 }, { "braille_dots_13567"         , 0x1002875, 0x2875 }, { "braille_dots_23567"         , 0x1002876, 0x2876 }, { "braille_dots_123567"        , 0x1002877, 0x2877 }, { "braille_dots_4567"          , 0x1002878, 0x2878 }, { "braille_dots_14567"         , 0x1002879, 0x2879 }, { "braille_dots_24567"         , 0x100287a, 0x287a }, { "braille_dots_124567"        , 0x100287b, 0x287b }, { "braille_dots_34567"         , 0x100287c, 0x287c }, { "braille_dots_134567"        , 0x100287d, 0x287d }, { "braille_dots_234567"        , 0x100287e, 0x287e }, { "braille_dots_1234567"       , 0x100287f, 0x287f }, { "braille_dots_8"             , 0x1002880, 0x2880 }, { "braille_dots_18"            , 0x1002881, 0x2881 }, { "braille_dots_28"            , 0x1002882, 0x2882 }, { "braille_dots_128"           , 0x1002883, 0x2883 }, { "braille_dots_38"            , 0x1002884, 0x2884 }, { "braille_dots_138"           , 0x1002885, 0x2885 }, { "braille_dots_238"           , 0x1002886, 0x2886 }, { "braille_dots_1238"          , 0x1002887, 0x2887 }, { "braille_dots_48"            , 0x1002888, 0x2888 }, { "braille_dots_148"           , 0x1002889, 0x2889 }, { "braille_dots_248"           , 0x100288a, 0x288a }, { "braille_dots_1248"          , 0x100288b, 0x288b }, { "braille_dots_348"           , 0x100288c, 0x288c }, { "braille_dots_1348"          , 0x100288d, 0x288d }, { "braille_dots_2348"          , 0x100288e, 0x288e }, { "braille_dots_12348"         , 0x100288f, 0x288f }, { "braille_dots_58"            , 0x1002890, 0x2890 },
                { "braille_dots_158"           , 0x1002891, 0x2891 }, { "braille_dots_258"           , 0x1002892, 0x2892 }, { "braille_dots_1258"          , 0x1002893, 0x2893 }, { "braille_dots_358"           , 0x1002894, 0x2894 }, { "braille_dots_1358"          , 0x1002895, 0x2895 }, { "braille_dots_2358"          , 0x1002896, 0x2896 }, { "braille_dots_12358"         , 0x1002897, 0x2897 }, { "braille_dots_458"           , 0x1002898, 0x2898 }, { "braille_dots_1458"          , 0x1002899, 0x2899 }, { "braille_dots_2458"          , 0x100289a, 0x289a }, { "braille_dots_12458"         , 0x100289b, 0x289b }, { "braille_dots_3458"          , 0x100289c, 0x289c }, { "braille_dots_13458"         , 0x100289d, 0x289d }, { "braille_dots_23458"         , 0x100289e, 0x289e }, { "braille_dots_123458"        , 0x100289f, 0x289f }, { "braille_dots_68"            , 0x10028a0, 0x28a0 }, { "braille_dots_168"           , 0x10028a1, 0x28a1 }, { "braille_dots_268"           , 0x10028a2, 0x28a2 }, { "braille_dots_1268"          , 0x10028a3, 0x28a3 }, { "braille_dots_368"           , 0x10028a4, 0x28a4 }, { "braille_dots_1368"          , 0x10028a5, 0x28a5 }, { "braille_dots_2368"          , 0x10028a6, 0x28a6 }, { "braille_dots_12368"         , 0x10028a7, 0x28a7 }, { "braille_dots_468"           , 0x10028a8, 0x28a8 }, { "braille_dots_1468"          , 0x10028a9, 0x28a9 }, { "braille_dots_2468"          , 0x10028aa, 0x28aa }, { "braille_dots_12468"         , 0x10028ab, 0x28ab }, { "braille_dots_3468"          , 0x10028ac, 0x28ac }, { "braille_dots_13468"         , 0x10028ad, 0x28ad }, { "braille_dots_23468"         , 0x10028ae, 0x28ae },
                { "braille_dots_123468"        , 0x10028af, 0x28af }, { "braille_dots_568"           , 0x10028b0, 0x28b0 }, { "braille_dots_1568"          , 0x10028b1, 0x28b1 }, { "braille_dots_2568"          , 0x10028b2, 0x28b2 }, { "braille_dots_12568"         , 0x10028b3, 0x28b3 }, { "braille_dots_3568"          , 0x10028b4, 0x28b4 }, { "braille_dots_13568"         , 0x10028b5, 0x28b5 }, { "braille_dots_23568"         , 0x10028b6, 0x28b6 }, { "braille_dots_123568"        , 0x10028b7, 0x28b7 }, { "braille_dots_4568"          , 0x10028b8, 0x28b8 }, { "braille_dots_14568"         , 0x10028b9, 0x28b9 }, { "braille_dots_24568"         , 0x10028ba, 0x28ba }, { "braille_dots_124568"        , 0x10028bb, 0x28bb }, { "braille_dots_34568"         , 0x10028bc, 0x28bc }, { "braille_dots_134568"        , 0x10028bd, 0x28bd }, { "braille_dots_234568"        , 0x10028be, 0x28be }, { "braille_dots_1234568"       , 0x10028bf, 0x28bf }, { "braille_dots_78"            , 0x10028c0, 0x28c0 }, { "braille_dots_178"           , 0x10028c1, 0x28c1 }, { "braille_dots_278"           , 0x10028c2, 0x28c2 }, { "braille_dots_1278"          , 0x10028c3, 0x28c3 }, { "braille_dots_378"           , 0x10028c4, 0x28c4 }, { "braille_dots_1378"          , 0x10028c5, 0x28c5 }, { "braille_dots_2378"          , 0x10028c6, 0x28c6 }, { "braille_dots_12378"         , 0x10028c7, 0x28c7 }, { "braille_dots_478"           , 0x10028c8, 0x28c8 }, { "braille_dots_1478"          , 0x10028c9, 0x28c9 }, { "braille_dots_2478"          , 0x10028ca, 0x28ca }, { "braille_dots_12478"         , 0x10028cb, 0x28cb }, { "braille_dots_3478"          , 0x10028cc, 0x28cc },
                { "braille_dots_13478"         , 0x10028cd, 0x28cd }, { "braille_dots_23478"         , 0x10028ce, 0x28ce }, { "braille_dots_123478"        , 0x10028cf, 0x28cf }, { "braille_dots_578"           , 0x10028d0, 0x28d0 }, { "braille_dots_1578"          , 0x10028d1, 0x28d1 }, { "braille_dots_2578"          , 0x10028d2, 0x28d2 }, { "braille_dots_12578"         , 0x10028d3, 0x28d3 }, { "braille_dots_3578"          , 0x10028d4, 0x28d4 }, { "braille_dots_13578"         , 0x10028d5, 0x28d5 }, { "braille_dots_23578"         , 0x10028d6, 0x28d6 }, { "braille_dots_123578"        , 0x10028d7, 0x28d7 }, { "braille_dots_4578"          , 0x10028d8, 0x28d8 }, { "braille_dots_14578"         , 0x10028d9, 0x28d9 }, { "braille_dots_24578"         , 0x10028da, 0x28da }, { "braille_dots_124578"        , 0x10028db, 0x28db }, { "braille_dots_34578"         , 0x10028dc, 0x28dc }, { "braille_dots_134578"        , 0x10028dd, 0x28dd }, { "braille_dots_234578"        , 0x10028de, 0x28de }, { "braille_dots_1234578"       , 0x10028df, 0x28df }, { "braille_dots_678"           , 0x10028e0, 0x28e0 }, { "braille_dots_1678"          , 0x10028e1, 0x28e1 }, { "braille_dots_2678"          , 0x10028e2, 0x28e2 }, { "braille_dots_12678"         , 0x10028e3, 0x28e3 }, { "braille_dots_3678"          , 0x10028e4, 0x28e4 }, { "braille_dots_13678"         , 0x10028e5, 0x28e5 }, { "braille_dots_23678"         , 0x10028e6, 0x28e6 }, { "braille_dots_123678"        , 0x10028e7, 0x28e7 }, { "braille_dots_4678"          , 0x10028e8, 0x28e8 }, { "braille_dots_14678"         , 0x10028e9, 0x28e9 }, { "braille_dots_24678"         , 0x10028ea, 0x28ea },
                { "braille_dots_124678"        , 0x10028eb, 0x28eb }, { "braille_dots_34678"         , 0x10028ec, 0x28ec }, { "braille_dots_134678"        , 0x10028ed, 0x28ed }, { "braille_dots_234678"        , 0x10028ee, 0x28ee }, { "braille_dots_1234678"       , 0x10028ef, 0x28ef }, { "braille_dots_5678"          , 0x10028f0, 0x28f0 }, { "braille_dots_15678"         , 0x10028f1, 0x28f1 }, { "braille_dots_25678"         , 0x10028f2, 0x28f2 }, { "braille_dots_125678"        , 0x10028f3, 0x28f3 }, { "braille_dots_35678"         , 0x10028f4, 0x28f4 }, { "braille_dots_135678"        , 0x10028f5, 0x28f5 }, { "braille_dots_235678"        , 0x10028f6, 0x28f6 }, { "braille_dots_1235678"       , 0x10028f7, 0x28f7 }, { "braille_dots_45678"         , 0x10028f8, 0x28f8 }, { "braille_dots_145678"        , 0x10028f9, 0x28f9 }, { "braille_dots_245678"        , 0x10028fa, 0x28fa }, { "braille_dots_1245678"       , 0x10028fb, 0x28fb }, { "braille_dots_345678"        , 0x10028fc, 0x28fc }, { "braille_dots_1345678"       , 0x10028fd, 0x28fd }, { "braille_dots_2345678"       , 0x10028fe, 0x28fe }, { "braille_dots_12345678"      , 0x10028ff, 0x28ff }, { "Sinh_ng"                    , 0x1000d82, 0x0d82 }, { "Sinh_h2"                    , 0x1000d83, 0x0d83 }, { "Sinh_a"                     , 0x1000d85, 0x0d85 }, { "Sinh_aa"                    , 0x1000d86, 0x0d86 }, { "Sinh_ae"                    , 0x1000d87, 0x0d87 }, { "Sinh_aee"                   , 0x1000d88, 0x0d88 }, { "Sinh_i"                     , 0x1000d89, 0x0d89 }, { "Sinh_ii"                    , 0x1000d8a, 0x0d8a }, { "Sinh_u"                     , 0x1000d8b, 0x0d8b },
                { "Sinh_uu"                    , 0x1000d8c, 0x0d8c }, { "Sinh_ri"                    , 0x1000d8d, 0x0d8d }, { "Sinh_rii"                   , 0x1000d8e, 0x0d8e }, { "Sinh_lu"                    , 0x1000d8f, 0x0d8f }, { "Sinh_luu"                   , 0x1000d90, 0x0d90 }, { "Sinh_e"                     , 0x1000d91, 0x0d91 }, { "Sinh_ee"                    , 0x1000d92, 0x0d92 }, { "Sinh_ai"                    , 0x1000d93, 0x0d93 }, { "Sinh_o"                     , 0x1000d94, 0x0d94 }, { "Sinh_oo"                    , 0x1000d95, 0x0d95 }, { "Sinh_au"                    , 0x1000d96, 0x0d96 }, { "Sinh_ka"                    , 0x1000d9a, 0x0d9a }, { "Sinh_kha"                   , 0x1000d9b, 0x0d9b }, { "Sinh_ga"                    , 0x1000d9c, 0x0d9c }, { "Sinh_gha"                   , 0x1000d9d, 0x0d9d }, { "Sinh_ng2"                   , 0x1000d9e, 0x0d9e }, { "Sinh_nga"                   , 0x1000d9f, 0x0d9f }, { "Sinh_ca"                    , 0x1000da0, 0x0da0 }, { "Sinh_cha"                   , 0x1000da1, 0x0da1 }, { "Sinh_ja"                    , 0x1000da2, 0x0da2 }, { "Sinh_jha"                   , 0x1000da3, 0x0da3 }, { "Sinh_nya"                   , 0x1000da4, 0x0da4 }, { "Sinh_jnya"                  , 0x1000da5, 0x0da5 }, { "Sinh_nja"                   , 0x1000da6, 0x0da6 }, { "Sinh_tta"                   , 0x1000da7, 0x0da7 }, { "Sinh_ttha"                  , 0x1000da8, 0x0da8 }, { "Sinh_dda"                   , 0x1000da9, 0x0da9 }, { "Sinh_ddha"                  , 0x1000daa, 0x0daa }, { "Sinh_nna"                   , 0x1000dab, 0x0dab }, { "Sinh_ndda"                  , 0x1000dac, 0x0dac },
                { "Sinh_tha"                   , 0x1000dad, 0x0dad }, { "Sinh_thha"                  , 0x1000dae, 0x0dae }, { "Sinh_dha"                   , 0x1000daf, 0x0daf }, { "Sinh_dhha"                  , 0x1000db0, 0x0db0 }, { "Sinh_na"                    , 0x1000db1, 0x0db1 }, { "Sinh_ndha"                  , 0x1000db3, 0x0db3 }, { "Sinh_pa"                    , 0x1000db4, 0x0db4 }, { "Sinh_pha"                   , 0x1000db5, 0x0db5 }, { "Sinh_ba"                    , 0x1000db6, 0x0db6 }, { "Sinh_bha"                   , 0x1000db7, 0x0db7 }, { "Sinh_ma"                    , 0x1000db8, 0x0db8 }, { "Sinh_mba"                   , 0x1000db9, 0x0db9 }, { "Sinh_ya"                    , 0x1000dba, 0x0dba }, { "Sinh_ra"                    , 0x1000dbb, 0x0dbb }, { "Sinh_la"                    , 0x1000dbd, 0x0dbd }, { "Sinh_va"                    , 0x1000dc0, 0x0dc0 }, { "Sinh_sha"                   , 0x1000dc1, 0x0dc1 }, { "Sinh_ssha"                  , 0x1000dc2, 0x0dc2 }, { "Sinh_sa"                    , 0x1000dc3, 0x0dc3 }, { "Sinh_ha"                    , 0x1000dc4, 0x0dc4 }, { "Sinh_lla"                   , 0x1000dc5, 0x0dc5 }, { "Sinh_fa"                    , 0x1000dc6, 0x0dc6 }, { "Sinh_al"                    , 0x1000dca, 0x0dca }, { "Sinh_aa2"                   , 0x1000dcf, 0x0dcf }, { "Sinh_ae2"                   , 0x1000dd0, 0x0dd0 }, { "Sinh_aee2"                  , 0x1000dd1, 0x0dd1 }, { "Sinh_i2"                    , 0x1000dd2, 0x0dd2 }, { "Sinh_ii2"                   , 0x1000dd3, 0x0dd3 }, { "Sinh_u2"                    , 0x1000dd4, 0x0dd4 }, { "Sinh_uu2"                   , 0x1000dd6, 0x0dd6 },
                { "Sinh_ru2"                   , 0x1000dd8, 0x0dd8 }, { "Sinh_e2"                    , 0x1000dd9, 0x0dd9 }, { "Sinh_ee2"                   , 0x1000dda, 0x0dda }, { "Sinh_ai2"                   , 0x1000ddb, 0x0ddb }, { "Sinh_o2"                    , 0x1000ddc, 0x0ddc }, { "Sinh_oo2"                   , 0x1000ddd, 0x0ddd }, { "Sinh_au2"                   , 0x1000dde, 0x0dde }, { "Sinh_lu2"                   , 0x1000ddf, 0x0ddf }, { "Sinh_ruu2"                  , 0x1000df2, 0x0df2 }, { "Sinh_luu2"                  , 0x1000df3, 0x0df3 }, { "Sinh_kunddaliya"            , 0x1000df4, 0x0df4 }, { "SSHARP"                     , 0x1001e9e, 0x1e9e }, { "leftsingleanglequotemark"   , 0x1002039, 0x2039 }, { "rightsingleanglequotemark"  , 0x100203a, 0x203a }
            });
            std::sort(m.begin(), m.end(), [](auto& a, auto& b){ return a.keysym < b.keysym; });
            auto i = 0u;
            auto j = 0u;
            while (i < m.size()) // Duplicate the Unicode code point for identical keysyms.
            {
                auto found_uc = 0u;
                for (i = j; j < m.size() && m[j].keysym == m[i].keysym; j++)
                {
                    if (m[j].uc != 0) found_uc = m[j].uc;
                }
                if (found_uc)
                {
                    for (auto k = i; k < j; ++k) m[k].uc = found_uc;
                }
            }
            return m;
        }();
        static constexpr auto _sym_to_unicode = []
        {
            auto m = std::array<ui16, 65536>{};
            for (auto rec : _symdef)
            {
                if (rec.keysym < 65536) // Filter 0x10xxxxx records.
                {
                    m[rec.keysym] = rec.uc;
                }
            }
            return m;
        }();
        static constexpr auto _index_name_to_symdef = []
        {
            auto idx = std::array<ui16, _symdef.size()>{};
            for (ui16 i = 0; i < _symdef.size(); ++i) idx[i] = i;
            std::sort(idx.begin(), idx.end(), [](auto a, auto b) { return _symdef[a].name < _symdef[b].name; });
            return idx;
        }();
        constexpr auto sym_to_unicode(ui32 keysym)
        {
            return keysym <= 0xFFFF                   ? _sym_to_unicode[keysym]
                : (keysym & 0xFF000000) == 0x01000000 ? keysym & 0x00FFFFFF
                                                      : 0u;
        }
        constexpr auto sym_to_name(ui32 keysym)
        {
            auto it = std::lower_bound(_symdef.begin(), _symdef.end(), keysym, [](auto& rec, ui32 val){return rec.keysym < val; });
            if (it != _symdef.end() && it->keysym == keysym)
            {
                return it->name;
            }
            return "undef"sv;
        }
        constexpr auto name_to_sym(view name)
        {
            auto it = std::lower_bound(_index_name_to_symdef.begin(), _index_name_to_symdef.end(), name, [](ui16 idx, view val){ return _symdef[idx].name < val; });
            if (it != _index_name_to_symdef.end())
            {
                if (auto rec = _symdef[*it]; rec.name == name)
                {
                    return rec.keysym;
                }
            }
            if (name.size() > 1)
            if (auto u = name.front(); u == 'U' || u == 'u') // Uxxxx uxxxx U+xxxx u+xxxx
            {
                name.remove_prefix(1);
                if (name.front() == '+')
                {
                    name.remove_prefix(1);
                }
                if (name.size())
                {
                    auto cp = 0u;
                    auto valid = true;
                    for (auto c : name)
                    {
                        cp <<= 4;
                             if (c >= '0' && c <= '9') cp |= c - '0';
                        else if (c >= 'a' && c <= 'f') cp |= c - 'a' + 10;
                        else if (c >= 'A' && c <= 'F') cp |= c - 'A' + 10;
                        else
                        {
                            valid = faux;
                            break;
                        }
                    }
                    if (valid)
                    {
                        return 0x01000000 | cp;
                    }
                }
            }
            return 0u;
        }
    }

    struct compose
    {
        struct rule_t
        {
            std::vector<ui32> keysyms;
            text              utf8;
        };
        struct node_t
        {
            using list = std::vector<netxs::sptr<node_t>>;

            ui32 keysym = 0; // Triggerred keysym.
            text utf8;       // Final UTF-8 string if next is empty.
            list next;       // Next hop list sorted (by keysym).

            auto find_next(ui32 keysym)
            {
                auto it = std::lower_bound(next.begin(), next.end(), keysym, [](auto& node, ui32 val){ return node->keysym < val; });
                if (it != next.end() && (*it)->keysym == keysym)
                {
                    return *it;
                }
                return netxs::sptr<node_t>{};
            }
            text to_string(si32 level) const
            {
                auto s = text{};
                if (next.size())
                {
                    for (auto& n : next)
                    {
                        s += utf::fprint("<%%> %%", x11::key::sym_to_name(keysym), n->to_string(level + 1));
                    }
                }
                else
                {
                    s += utf::fprint("<%%> : '%%'\n%%", x11::key::sym_to_name(keysym), utf::debase437(utf8), text(level, '\t'));
                }
                return s;
            }
        };
        enum class status
        {
            inactive,    // Plain input.
            matching,    // Waiting the next keysym.
            completed,   // Got utf8 string.
            invalidated, // Aborted.
        };
        struct input_result
        {
            status stat;
            text   utf8;
        };

        netxs::sptr<node_t> current_node = {};
        netxs::sptr<node_t> root = ptr::shared(node_t{});
        text                locale;
        std::vector<ui32>   input_backup;

        compose()
        {
            if (auto raw_locale = std::setlocale(LC_ALL, "")) // "ru_RU.UTF-8" or "sr_RS@latin"
            {
                locale = text{ raw_locale };
                if constexpr (debugmode) log("Current locale: '%%'", locale);
                //todo filter locale by /usr/share/X11/locale/locale.alias (simplified locale name -> full locale name)
            }
            else
            {
                locale = "en_US.UTF-8";
                if constexpr (debugmode) log("Fallback to locale: '%%'", locale);
            }
            auto compose_file = os::fs::path{};
            auto include_stack = std::vector<os::fs::path>{};
            // 1. Check for the presence of the XCOMPOSEFILE file.
            if (auto xcomposefile = os::env::get("XCOMPOSEFILE"); xcomposefile.size())
            {
                if (auto xcomposefile_path = os::fs::path{ xcomposefile }; os::fs::exists(xcomposefile_path))
                {
                    compose_file = std::move(xcomposefile_path);
                }
            }
            // 2. Check for the presence of the '~/.XCompose' file.
            if (compose_file.empty())
            if (auto home = os::env::get("HOME"); home.size())
            {
                if (auto user_xcompose = os::fs::path{ home } / ".XCompose"; os::fs::exists(user_xcompose))
                {
                    compose_file = std::move(user_xcompose);
                }
            }
            // 3. Check the system locale path.
            if (compose_file.empty())
            if (auto system_path = _get_system_compose_path() / locale / "Compose"; os::fs::exists(system_path))
            {
                compose_file = std::move(system_path);
            }
            if constexpr (debugmode) log(" Compose file: '%%'", compose_file.string());
            _load_compose_file(compose_file, include_stack);
            if constexpr (debugmode)
            {
                log("   root->next.size=", root->next.size());
                for (auto& next : root->next)
                {
                    auto s = next->to_string(1);
                    log("%%", s);
                }
            }
        }

        os::fs::path _get_system_compose_path()
        {
            if (auto xlocaledir = os::env::get("XLOCALEDIR"); xlocaledir.size())
            if (auto xlocaledir_path = os::fs::path{ xlocaledir }; os::fs::exists(xlocaledir_path))
            {
                return xlocaledir_path;
            }
            return os::fs::path{ "/usr/share/X11/locale" };
        }
        auto _parse_line(qiew line) -> std::variant<std::monostate, rule_t, text> // A line can be a rule, an include, or nothing (a comment/error).
        {
            utf::trim_front(line, "\t ");
            if (line.empty() || line.front() == '#')
            {
                return std::monostate{};
            }
            if (line.starts_with("include"))
            {
                line.remove_prefix(sizeof("include") - 1/*trailing null*/); // Remove 'include' keyword.
                utf::trim_front(line, "\t ");
                if (line && line.front() == '"')
                if (auto raw_path = utf::take_quote(line, '"'); raw_path.size())
                {
                    auto include_path = text{};
                    include_path = raw_path;
                    return include_path;
                }
                return std::monostate{};
            }
            auto keysyms = std::vector<ui32>{};
            while (line && line.front() == '<') // Parse rule line: <key_name> ... <key_name>.
            {
                if (auto keyname = utf::take_quote(line, '>'); keyname.size())
                if (auto keysym = x11::key::name_to_sym(keyname))
                {
                    keysyms.push_back(keysym);
                    utf::trim_front(line, "\t ");
                    continue;
                }
                return std::monostate{}; // Unknown key name.
            }
            if (keysyms.size() && line.size() && line.front() == ':')
            {
                utf::trim_front(line, "\t :"); // Pop ':' with spaces.
                if (line && line.front() == '"')
                {
                    auto rule = rule_t{ .keysyms = std::move(keysyms) };
                    rule.utf8 = utf::take_quote(line, '"');
                    return rule;
                }
            }
            return std::monostate{};
        }
        auto _resolve_include_path(text raw_path)
        {
            if (raw_path.find("%L") != text::npos) // Expand %L marco with the current locale.
            {
                utf::replace_all(raw_path, "%L", locale);
            }
            if (raw_path.find("%H") != text::npos) // Expand %H marco with the home path.
            {
                auto home = os::env::get("HOME");
                utf::replace_all(raw_path, "%H", home);
            }
            if (raw_path.find("%S") != text::npos) // Expand %S marco with the system compose path.
            {
                utf::replace_all(raw_path, "%S", _get_system_compose_path().string());
            }
            auto path = os::fs::path{ raw_path };
            return path.is_absolute() ? path
                                      : _get_system_compose_path() / path;
        }
        void _inject_into_trie(std::vector<ui32> const& keysyms, view utf8)
        {
            if (keysyms.size())
            {
                auto node = root;
                for (auto& keysym : keysyms)
                {
                    auto it = std::lower_bound(node->next.begin(), node->next.end(), keysym, [](auto& node, ui32 val){ return node->keysym < val; });
                    if (it == node->next.end() || (*it)->keysym != keysym)
                    {
                        auto new_node = ptr::shared(node_t{ .keysym = keysym });
                        it = node->next.insert(it, new_node);
                    }
                    node = *it;
                }
                node->utf8 = utf8;
                node->next.clear(); // Cut current node branch.
            }
        }
        void _load_compose_file(os::fs::path const& file_path, std::vector<os::fs::path>& include_stack)
        {
            if (std::find(include_stack.begin(), include_stack.end(), file_path) == include_stack.end())
            {
                auto file = std::ifstream{ file_path };
                if (file.is_open())
                {
                    include_stack.push_back(file_path);
                    auto line = text{};
                    while (std::getline(file, line))
                    {
                        auto res = _parse_line(line);
                        std::visit([&](auto&& arg)
                        {
                            using T = std::decay_t<decltype(arg)>;
                            if constexpr (std::is_same_v<T, rule_t>) // Add rule.
                            {
                                _inject_into_trie(arg.keysyms, arg.utf8);
                            }
                            else if constexpr (std::is_same_v<T, text>) // Recursively expand the include directive.
                            {
                                auto next_file = _resolve_include_path(arg);
                                _load_compose_file(next_file, include_stack);
                            }
                        }, res);
                    }
                    include_stack.pop_back();
                }
            }
        }
        auto process_keysym(ui32 keysym)
        {
            if (!current_node) // 1. Check activation by the first key.
            {
                if (auto next_node = root->find_next(keysym))
                {
                    if (next_node->next.empty())
                    {
                        return input_result{ status::completed, next_node->utf8 }; // Single key chord.
                    }
                    input_backup.push_back(keysym);
                    current_node = next_node;
                    return input_result{ status::matching };
                }
                return input_result{ status::inactive }; // Plain input.
            }
            if (auto next_node = current_node->find_next(keysym)) // 2. Try to next step.
            {
                if (next_node->next.empty()) // Got utf8.
                {
                    reset();
                    return input_result{ .stat = status::completed, .utf8 = next_node->utf8 };
                }
                input_backup.push_back(keysym);
                current_node = next_node;
                return input_result{ status::matching };
            }
            if (keysym >= 0xffe1 && keysym <= 0xffee) // 3. Filter modifiers (e.g., Shift, Ctrl, Alt).
            {
                input_backup.push_back(keysym);
                return input_result{ status::matching }; // Ignore modifiers, wait letters.
            }
            reset(); // 4. Broken input.
            return input_result{ status::invalidated };
        }
        // Explicit reset (e.g., on lost focus).
        void reset()
        {
            current_node.reset();
            input_backup.clear();
        }
    };
}