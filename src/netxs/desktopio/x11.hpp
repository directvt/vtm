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
            X(lbutton , Pointer_Button1 , 0XFEE9) /* VK_LBUTTON */\
            X(altgr   , ISO_Level3_Shift, 0XFE03) \
            X(grselect, ISO_Level5_Shift, 0XFE11) /* VK_OEM_8 GroupSelect (Level5Shift) on Canadian layout */\
            X(numlock , Num_Lock        , 0XFF7F) /* VK_NUMLOCK  */\
            X(capslock, Caps_Lock       , 0XFFE5) /* VK_CAPITAL  */\
            X(scrllock, Scroll_Lock     , 0XFF14) /* VK_SCROLL   */\
            X(lshift  , Shift_L         , 0XFFE1) /* VK_LSHIFT   */\
            X(rshift  , Shift_R         , 0XFFE2) /* VK_RSHIFT   */\
            X(lctrl   , Control_L       , 0XFFE3) /* VK_LCONTROL */\
            X(rctrl   , Control_R       , 0XFFE4) /* VK_RCONTROL */\
            X(lalt    , Alt_L           , 0XFFE9) /* VK_LMENU    */\
            X(ralt    , Alt_R           , 0XFFEA) /* VK_RMENU    */\
            X(lsuper  , Super_L         , 0XFFEB) /* VK_LWIN     */\
            X(rsuper  , Super_R         , 0XFFEC) /* VK_RWIN     */\
            X(undef   , Hyper_L         , 0XFFED) /* VK_LWIN     */\
            X(undef   , Hyper_R         , 0XFFEE) /* VK_RWIN     */\
            X(prntscrn, Print           , 0XFF61) /* VK_SNAPSHOT (SysReq, Alt+PrntScrn) */ \
            X(cancel  , Cancel          , 0XFF69) /* VK_CANCEL (Break, Ctrl+Pause) */ \
            X(clear   , Clear           , 0XFF0B) /* VK_CLEAR    */ \
            X(enter   , Return          , 0XFF0D) /* VK_RETURN   */ \
            X(pgup    , Prior           , 0XFF55) /* VK_PRIOR    */ \
            X(pgdn    , Next            , 0XFF56) /* VK_NEXT     */ \
            X(end     , End             , 0XFF57) /* VK_END      */ \
            X(home    , Home            , 0XFF50) /* VK_HOME     */ \
            X(left    , Left            , 0XFF51) /* VK_LEFT     */ \
            X(up      , Up              , 0XFF52) /* VK_UP       */ \
            X(right   , Right           , 0XFF53) /* VK_RIGHT    */ \
            X(down    , Down            , 0XFF54) /* VK_DOWN     */ \
            X(insert  , Insert          , 0XFF63) /* VK_INSERT   */ \
            X(del     , Delete          , 0XFFFF) /* VK_DELETE   */ \
            X(f11     , F11             , 0XFFC8) /* VK_F11      */ \
            X(f12     , F12             , 0XFFC9) /* VK_F12      */ \
            X(key_0   , 0               , 0X0030) /* VK_0        */ \
            X(numpad0 , KP_0            , 0XFFB0) /* VK_NUMPAD0  */ \
            X(numpad1 , KP_1            , 0XFFB1) /* VK_NUMPAD1  */ \
            X(numpad2 , KP_2            , 0XFFB2) /* VK_NUMPAD2  */ \
            X(numpad3 , KP_3            , 0XFFB3) /* VK_NUMPAD3  */ \
            X(numpad4 , KP_4            , 0XFFB4) /* VK_NUMPAD4  */ \
            X(numpad5 , KP_5            , 0XFFB5) /* VK_NUMPAD5  */ \
            X(numpad6 , KP_6            , 0XFFB6) /* VK_NUMPAD6  */ \
            X(numpad7 , KP_7            , 0XFFB7) /* VK_NUMPAD7  */ \
            X(numpad8 , KP_8            , 0XFFB8) /* VK_NUMPAD8  */ \
            X(numpad9 , KP_9            , 0XFFB9) /* VK_NUMPAD9  */ \
            X(numpadD , KP_Decimal      , 0XFFAE) /* VK_DECIMAL  */
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
        static constexpr auto sym_to_unicode = []
        {
            auto pairs = std::to_array<ui16>(
            {
            //  1              2              3              4              5              6              7              8              9              0              1              2              3              4              5              6              7              8              9              0              1              2              3              4              5              6              7              8              9              0
                0xff08,0x0008, 0xff09,0x0009, 0xff0a,0x000a, 0xff0b,0x000b, 0xff0d,0x000d, 0xff1b,0x001b, 0xffff,0x007f, 0x0020,0x0020, 0x0021,0x0021, 0x0022,0x0022, 0x0023,0x0023, 0x0024,0x0024, 0x0025,0x0025, 0x0026,0x0026, 0x0027,0x0027, 0x0028,0x0028, 0x0029,0x0029, 0x002a,0x002a, 0x002b,0x002b, 0x002c,0x002c, 0x002d,0x002d, 0x002e,0x002e, 0x002f,0x002f, 0x0030,0x0030, 0x0031,0x0031, 0x0032,0x0032, 0x0033,0x0033, 0x0034,0x0034, 0x0035,0x0035, 0x0036,0x0036,
                0x0037,0x0037, 0x0038,0x0038, 0x0039,0x0039, 0x003a,0x003a, 0x003b,0x003b, 0x003c,0x003c, 0x003d,0x003d, 0x003e,0x003e, 0x003f,0x003f, 0x0040,0x0040, 0x0041,0x0041, 0x0042,0x0042, 0x0043,0x0043, 0x0044,0x0044, 0x0045,0x0045, 0x0046,0x0046, 0x0047,0x0047, 0x0048,0x0048, 0x0049,0x0049, 0x004a,0x004a, 0x004b,0x004b, 0x004c,0x004c, 0x004d,0x004d, 0x004e,0x004e, 0x004f,0x004f, 0x0050,0x0050, 0x0051,0x0051, 0x0052,0x0052, 0x0053,0x0053, 0x0054,0x0054,
                0x0055,0x0055, 0x0056,0x0056, 0x0057,0x0057, 0x0058,0x0058, 0x0059,0x0059, 0x005a,0x005a, 0x005b,0x005b, 0x005c,0x005c, 0x005d,0x005d, 0x005e,0x005e, 0x005f,0x005f, 0x0060,0x0060, 0x0061,0x0061, 0x0062,0x0062, 0x0063,0x0063, 0x0064,0x0064, 0x0065,0x0065, 0x0066,0x0066, 0x0067,0x0067, 0x0068,0x0068, 0x0069,0x0069, 0x006a,0x006a, 0x006b,0x006b, 0x006c,0x006c, 0x006d,0x006d, 0x006e,0x006e, 0x006f,0x006f, 0x0070,0x0070, 0x0071,0x0071, 0x0072,0x0072,
                0x0073,0x0073, 0x0074,0x0074, 0x0075,0x0075, 0x0076,0x0076, 0x0077,0x0077, 0x0078,0x0078, 0x0079,0x0079, 0x007a,0x007a, 0x007b,0x007b, 0x007c,0x007c, 0x007d,0x007d, 0x007e,0x007e, 0x00a0,0x00a0, 0x00a1,0x00a1, 0x00a2,0x00a2, 0x00a3,0x00a3, 0x00a4,0x00a4, 0x00a5,0x00a5, 0x00a6,0x00a6, 0x00a7,0x00a7, 0x00a8,0x00a8, 0x00a9,0x00a9, 0x00aa,0x00aa, 0x00ab,0x00ab, 0x00ac,0x00ac, 0x00ad,0x00ad, 0x00ae,0x00ae, 0x00af,0x00af, 0x00b0,0x00b0, 0x00b1,0x00b1,
                0x00b2,0x00b2, 0x00b3,0x00b3, 0x00b4,0x00b4, 0x00b5,0x00b5, 0x00b6,0x00b6, 0x00b7,0x00b7, 0x00b8,0x00b8, 0x00b9,0x00b9, 0x00ba,0x00ba, 0x00bb,0x00bb, 0x00bc,0x00bc, 0x00bd,0x00bd, 0x00be,0x00be, 0x00bf,0x00bf, 0x00c0,0x00c0, 0x00c1,0x00c1, 0x00c2,0x00c2, 0x00c3,0x00c3, 0x00c4,0x00c4, 0x00c5,0x00c5, 0x00c6,0x00c6, 0x00c7,0x00c7, 0x00c8,0x00c8, 0x00c9,0x00c9, 0x00ca,0x00ca, 0x00cb,0x00cb, 0x00cc,0x00cc, 0x00cd,0x00cd, 0x00ce,0x00ce, 0x00cf,0x00cf,
                0x00d0,0x00d0, 0x00d1,0x00d1, 0x00d2,0x00d2, 0x00d3,0x00d3, 0x00d4,0x00d4, 0x00d5,0x00d5, 0x00d6,0x00d6, 0x00d7,0x00d7, 0x00d8,0x00d8, 0x00d9,0x00d9, 0x00da,0x00da, 0x00db,0x00db, 0x00dc,0x00dc, 0x00dd,0x00dd, 0x00de,0x00de, 0x00df,0x00df, 0x00e0,0x00e0, 0x00e1,0x00e1, 0x00e2,0x00e2, 0x00e3,0x00e3, 0x00e4,0x00e4, 0x00e5,0x00e5, 0x00e6,0x00e6, 0x00e7,0x00e7, 0x00e8,0x00e8, 0x00e9,0x00e9, 0x00ea,0x00ea, 0x00eb,0x00eb, 0x00ec,0x00ec, 0x00ed,0x00ed,
                0x00ee,0x00ee, 0x00ef,0x00ef, 0x00f0,0x00f0, 0x00f1,0x00f1, 0x00f2,0x00f2, 0x00f3,0x00f3, 0x00f4,0x00f4, 0x00f5,0x00f5, 0x00f6,0x00f6, 0x00f7,0x00f7, 0x00f8,0x00f8, 0x00f9,0x00f9, 0x00fa,0x00fa, 0x00fb,0x00fb, 0x00fc,0x00fc, 0x00fd,0x00fd, 0x00fe,0x00fe, 0x00ff,0x00ff, 0x01a1,0x0104, 0x01a2,0x02d8, 0x01a3,0x0141, 0x01a5,0x013d, 0x01a6,0x015a, 0x01a9,0x0160, 0x01aa,0x015e, 0x01ab,0x0164, 0x01ac,0x0179, 0x01ae,0x017d, 0x01af,0x017b, 0x01b1,0x0105,
                0x01b2,0x02db, 0x01b3,0x0142, 0x01b5,0x013e, 0x01b6,0x015b, 0x01b7,0x02c7, 0x01b9,0x0161, 0x01ba,0x015f, 0x01bb,0x0165, 0x01bc,0x017a, 0x01bd,0x02dd, 0x01be,0x017e, 0x01bf,0x017c, 0x01c0,0x0154, 0x01c3,0x0102, 0x01c5,0x0139, 0x01c6,0x0106, 0x01c8,0x010c, 0x01ca,0x0118, 0x01cc,0x011a, 0x01cf,0x010e, 0x01d0,0x0110, 0x01d1,0x0143, 0x01d2,0x0147, 0x01d5,0x0150, 0x01d8,0x0158, 0x01d9,0x016e, 0x01db,0x0170, 0x01de,0x0162, 0x01e0,0x0155, 0x01e3,0x0103,
                0x01e5,0x013a, 0x01e6,0x0107, 0x01e8,0x010d, 0x01ea,0x0119, 0x01ec,0x011b, 0x01ef,0x010f, 0x01f0,0x0111, 0x01f1,0x0144, 0x01f2,0x0148, 0x01f5,0x0151, 0x01f8,0x0159, 0x01f9,0x016f, 0x01fb,0x0171, 0x01fe,0x0163, 0x01ff,0x02d9, 0x02a1,0x0126, 0x02a6,0x0124, 0x02a9,0x0130, 0x02ab,0x011e, 0x02ac,0x0134, 0x02b1,0x0127, 0x02b6,0x0125, 0x02b9,0x0131, 0x02bb,0x011f, 0x02bc,0x0135, 0x02c5,0x010a, 0x02c6,0x0108, 0x02d5,0x0120, 0x02d8,0x011c, 0x02dd,0x016c,
                0x02de,0x015c, 0x02e5,0x010b, 0x02e6,0x0109, 0x02f5,0x0121, 0x02f8,0x011d, 0x02fd,0x016d, 0x02fe,0x015d, 0x03a2,0x0138, 0x03a3,0x0156, 0x03a5,0x0128, 0x03a6,0x013b, 0x03aa,0x0112, 0x03ab,0x0122, 0x03ac,0x0166, 0x03b3,0x0157, 0x03b5,0x0129, 0x03b6,0x013c, 0x03ba,0x0113, 0x03bb,0x0123, 0x03bc,0x0167, 0x03bd,0x014a, 0x03bf,0x014b, 0x03c0,0x0100, 0x03c7,0x012e, 0x03cc,0x0116, 0x03cf,0x012a, 0x03d1,0x0145, 0x03d2,0x014c, 0x03d3,0x0136, 0x03d9,0x0172,
                0x03dd,0x0168, 0x03de,0x016a, 0x03e0,0x0101, 0x03e7,0x012f, 0x03ec,0x0117, 0x03ef,0x012b, 0x03f1,0x0146, 0x03f2,0x014d, 0x03f3,0x0137, 0x03f9,0x0173, 0x03fd,0x0169, 0x03fe,0x016b, 0x13bc,0x0152, 0x13bd,0x0153, 0x13be,0x0178, 0x047e,0x203e, 0x04a1,0x3002, 0x04a2,0x300c, 0x04a3,0x300d, 0x04a4,0x3001, 0x04a5,0x30fb, 0x04a6,0x30f2, 0x04a7,0x30a1, 0x04a8,0x30a3, 0x04a9,0x30a5, 0x04aa,0x30a7, 0x04ab,0x30a9, 0x04ac,0x30e3, 0x04ad,0x30e5, 0x04ae,0x30e7,
                0x04af,0x30c3, 0x04b0,0x30fc, 0x04b1,0x30a2, 0x04b2,0x30a4, 0x04b3,0x30a6, 0x04b4,0x30a8, 0x04b5,0x30aa, 0x04b6,0x30ab, 0x04b7,0x30ad, 0x04b8,0x30af, 0x04b9,0x30b1, 0x04ba,0x30b3, 0x04bb,0x30b5, 0x04bc,0x30b7, 0x04bd,0x30b9, 0x04be,0x30bb, 0x04bf,0x30bd, 0x04c0,0x30bf, 0x04c1,0x30c1, 0x04c2,0x30c4, 0x04c3,0x30c6, 0x04c4,0x30c8, 0x04c5,0x30ca, 0x04c6,0x30cb, 0x04c7,0x30cc, 0x04c8,0x30cd, 0x04c9,0x30ce, 0x04ca,0x30cf, 0x04cb,0x30d2, 0x04cc,0x30d5,
                0x04cd,0x30d8, 0x04ce,0x30db, 0x04cf,0x30de, 0x04d0,0x30df, 0x04d1,0x30e0, 0x04d2,0x30e1, 0x04d3,0x30e2, 0x04d4,0x30e4, 0x04d5,0x30e6, 0x04d6,0x30e8, 0x04d7,0x30e9, 0x04d8,0x30ea, 0x04d9,0x30eb, 0x04da,0x30ec, 0x04db,0x30ed, 0x04dc,0x30ef, 0x04dd,0x30f3, 0x04de,0x309b, 0x04df,0x309c, 0x05ac,0x060c, 0x05bb,0x061b, 0x05bf,0x061f, 0x05c1,0x0621, 0x05c2,0x0622, 0x05c3,0x0623, 0x05c4,0x0624, 0x05c5,0x0625, 0x05c6,0x0626, 0x05c7,0x0627, 0x05c8,0x0628,
                0x05c9,0x0629, 0x05ca,0x062a, 0x05cb,0x062b, 0x05cc,0x062c, 0x05cd,0x062d, 0x05ce,0x062e, 0x05cf,0x062f, 0x05d0,0x0630, 0x05d1,0x0631, 0x05d2,0x0632, 0x05d3,0x0633, 0x05d4,0x0634, 0x05d5,0x0635, 0x05d6,0x0636, 0x05d7,0x0637, 0x05d8,0x0638, 0x05d9,0x0639, 0x05da,0x063a, 0x05e0,0x0640, 0x05e1,0x0641, 0x05e2,0x0642, 0x05e3,0x0643, 0x05e4,0x0644, 0x05e5,0x0645, 0x05e6,0x0646, 0x05e7,0x0647, 0x05e8,0x0648, 0x05e9,0x0649, 0x05ea,0x064a, 0x05eb,0x064b,
                0x05ec,0x064c, 0x05ed,0x064d, 0x05ee,0x064e, 0x05ef,0x064f, 0x05f0,0x0650, 0x05f1,0x0651, 0x05f2,0x0652, 0x06a1,0x0452, 0x06a2,0x0453, 0x06a3,0x0451, 0x06a4,0x0454, 0x06a5,0x0455, 0x06a6,0x0456, 0x06a7,0x0457, 0x06a8,0x0458, 0x06a9,0x0459, 0x06aa,0x045a, 0x06ab,0x045b, 0x06ac,0x045c, 0x06ad,0x0491, 0x06ae,0x045e, 0x06af,0x045f, 0x06b0,0x2116, 0x06b1,0x0402, 0x06b2,0x0403, 0x06b3,0x0401, 0x06b4,0x0404, 0x06b5,0x0405, 0x06b6,0x0406, 0x06b7,0x0407,
                0x06b8,0x0408, 0x06b9,0x0409, 0x06ba,0x040a, 0x06bb,0x040b, 0x06bc,0x040c, 0x06bd,0x0490, 0x06be,0x040e, 0x06bf,0x040f, 0x06c0,0x044e, 0x06c1,0x0430, 0x06c2,0x0431, 0x06c3,0x0446, 0x06c4,0x0434, 0x06c5,0x0435, 0x06c6,0x0444, 0x06c7,0x0433, 0x06c8,0x0445, 0x06c9,0x0438, 0x06ca,0x0439, 0x06cb,0x043a, 0x06cc,0x043b, 0x06cd,0x043c, 0x06ce,0x043d, 0x06cf,0x043e, 0x06d0,0x043f, 0x06d1,0x044f, 0x06d2,0x0440, 0x06d3,0x0441, 0x06d4,0x0442, 0x06d5,0x0443,
                0x06d6,0x0436, 0x06d7,0x0432, 0x06d8,0x044c, 0x06d9,0x044b, 0x06da,0x0437, 0x06db,0x0448, 0x06dc,0x044d, 0x06dd,0x0449, 0x06de,0x0447, 0x06df,0x044a, 0x06e0,0x042e, 0x06e1,0x0410, 0x06e2,0x0411, 0x06e3,0x0426, 0x06e4,0x0414, 0x06e5,0x0415, 0x06e6,0x0424, 0x06e7,0x0413, 0x06e8,0x0425, 0x06e9,0x0418, 0x06ea,0x0419, 0x06eb,0x041a, 0x06ec,0x041b, 0x06ed,0x041c, 0x06ee,0x041d, 0x06ef,0x041e, 0x06f0,0x041f, 0x06f1,0x042f, 0x06f2,0x0420, 0x06f3,0x0421,
                0x06f4,0x0422, 0x06f5,0x0423, 0x06f6,0x0416, 0x06f7,0x0412, 0x06f8,0x042c, 0x06f9,0x042b, 0x06fa,0x0417, 0x06fb,0x0428, 0x06fc,0x042d, 0x06fd,0x0429, 0x06fe,0x0427, 0x06ff,0x042a, 0x07a1,0x0386, 0x07a2,0x0388, 0x07a3,0x0389, 0x07a4,0x038a, 0x07a5,0x03aa, 0x07a7,0x038c, 0x07a8,0x038e, 0x07a9,0x03ab, 0x07ab,0x038f, 0x07ae,0x0385, 0x07af,0x2015, 0x07b1,0x03ac, 0x07b2,0x03ad, 0x07b3,0x03ae, 0x07b4,0x03af, 0x07b5,0x03ca, 0x07b6,0x0390, 0x07b7,0x03cc,
                0x07b8,0x03cd, 0x07b9,0x03cb, 0x07ba,0x03b0, 0x07bb,0x03ce, 0x07c1,0x0391, 0x07c2,0x0392, 0x07c3,0x0393, 0x07c4,0x0394, 0x07c5,0x0395, 0x07c6,0x0396, 0x07c7,0x0397, 0x07c8,0x0398, 0x07c9,0x0399, 0x07ca,0x039a, 0x07cb,0x039b, 0x07cc,0x039c, 0x07cd,0x039d, 0x07ce,0x039e, 0x07cf,0x039f, 0x07d0,0x03a0, 0x07d1,0x03a1, 0x07d2,0x03a3, 0x07d4,0x03a4, 0x07d5,0x03a5, 0x07d6,0x03a6, 0x07d7,0x03a7, 0x07d8,0x03a8, 0x07d9,0x03a9, 0x07e1,0x03b1, 0x07e2,0x03b2,
                0x07e3,0x03b3, 0x07e4,0x03b4, 0x07e5,0x03b5, 0x07e6,0x03b6, 0x07e7,0x03b7, 0x07e8,0x03b8, 0x07e9,0x03b9, 0x07ea,0x03ba, 0x07eb,0x03bb, 0x07ec,0x03bc, 0x07ed,0x03bd, 0x07ee,0x03be, 0x07ef,0x03bf, 0x07f0,0x03c0, 0x07f1,0x03c1, 0x07f2,0x03c3, 0x07f3,0x03c2, 0x07f4,0x03c4, 0x07f5,0x03c5, 0x07f6,0x03c6, 0x07f7,0x03c7, 0x07f8,0x03c8, 0x07f9,0x03c9, 0x08a1,0x23b7, 0x08a2,0x250c, 0x08a3,0x2500, 0x08a4,0x2320, 0x08a5,0x2321, 0x08a6,0x2502, 0x08a7,0x23a1,
                0x08a8,0x23a3, 0x08a9,0x23a4, 0x08aa,0x23a6, 0x08ab,0x239b, 0x08ac,0x239d, 0x08ad,0x239e, 0x08ae,0x23a0, 0x08af,0x23a8, 0x08b0,0x23ac, 0x08bc,0x2264, 0x08bd,0x2260, 0x08be,0x2265, 0x08bf,0x222b, 0x08c0,0x2234, 0x08c1,0x221d, 0x08c2,0x221e, 0x08c5,0x2207, 0x08c8,0x223c, 0x08c9,0x2243, 0x08cd,0x21d4, 0x08ce,0x21d2, 0x08cf,0x2261, 0x08d6,0x221a, 0x08da,0x2282, 0x08db,0x2283, 0x08dc,0x2229, 0x08dd,0x222a, 0x08de,0x2227, 0x08df,0x2228, 0x08ef,0x2202,
                0x08f6,0x0192, 0x08fb,0x2190, 0x08fc,0x2191, 0x08fd,0x2192, 0x08fe,0x2193, 0x09e0,0x25c6, 0x09e1,0x2592, 0x09e2,0x2409, 0x09e3,0x240c, 0x09e4,0x240d, 0x09e5,0x240a, 0x09e8,0x2424, 0x09e9,0x240b, 0x09ea,0x2518, 0x09eb,0x2510, 0x09ec,0x250c, 0x09ed,0x2514, 0x09ee,0x253c, 0x09ef,0x23ba, 0x09f0,0x23bb, 0x09f1,0x2500, 0x09f2,0x23bc, 0x09f3,0x23bd, 0x09f4,0x251c, 0x09f5,0x2524, 0x09f6,0x2534, 0x09f7,0x252c, 0x09f8,0x2502, 0x0aa1,0x2003, 0x0aa2,0x2002,
                0x0aa3,0x2004, 0x0aa4,0x2005, 0x0aa5,0x2007, 0x0aa6,0x2008, 0x0aa7,0x2009, 0x0aa8,0x200a, 0x0aa9,0x2014, 0x0aaa,0x2013, 0x0aac,0x2423, 0x0aae,0x2026, 0x0aaf,0x2025, 0x0ab0,0x2153, 0x0ab1,0x2154, 0x0ab2,0x2155, 0x0ab3,0x2156, 0x0ab4,0x2157, 0x0ab5,0x2158, 0x0ab6,0x2159, 0x0ab7,0x215a, 0x0ab8,0x2105, 0x0abb,0x2012, 0x0abc,0x2329, 0x0abd,0x002e, 0x0abe,0x232a, 0x0ac3,0x215b, 0x0ac4,0x215c, 0x0ac5,0x215d, 0x0ac6,0x215e, 0x0ac9,0x2122, 0x0aca,0x2613,
                0x0acc,0x25c1, 0x0acd,0x25b7, 0x0ace,0x25cb, 0x0acf,0x25af, 0x0ad0,0x2018, 0x0ad1,0x2019, 0x0ad2,0x201c, 0x0ad3,0x201d, 0x0ad4,0x211e, 0x0ad5,0x2030, 0x0ad6,0x2032, 0x0ad7,0x2033, 0x0ad9,0x271d, 0x0adb,0x25ac, 0x0adc,0x25c0, 0x0add,0x25b6, 0x0ade,0x25cf, 0x0adf,0x25ae, 0x0ae0,0x25e6, 0x0ae1,0x25ab, 0x0ae2,0x25ad, 0x0ae3,0x25b3, 0x0ae4,0x25bd, 0x0ae5,0x2606, 0x0ae6,0x2022, 0x0ae7,0x25aa, 0x0ae8,0x25b2, 0x0ae9,0x25bc, 0x0aea,0x261c, 0x0aeb,0x261e,
                0x0aec,0x2663, 0x0aed,0x2666, 0x0aee,0x2665, 0x0af0,0x2720, 0x0af1,0x2020, 0x0af2,0x2021, 0x0af3,0x2713, 0x0af4,0x2717, 0x0af5,0x266f, 0x0af6,0x266d, 0x0af7,0x2642, 0x0af8,0x2640, 0x0af9,0x260e, 0x0afa,0x2315, 0x0afb,0x2117, 0x0afc,0x2038, 0x0afd,0x201a, 0x0afe,0x201e, 0x0ba3,0x003c, 0x0ba6,0x003e, 0x0ba8,0x2228, 0x0ba9,0x2227, 0x0bc0,0x00af, 0x0bc2,0x22a4, 0x0bc3,0x2229, 0x0bc4,0x230a, 0x0bc6,0x005f, 0x0bca,0x2218, 0x0bcc,0x2395, 0x0bce,0x22a5,
                0x0bcf,0x25cb, 0x0bd3,0x2308, 0x0bd6,0x222a, 0x0bd8,0x2283, 0x0bda,0x2282, 0x0bdc,0x22a3, 0x0bfc,0x22a2, 0x0cdf,0x2017, 0x0ce0,0x05d0, 0x0ce1,0x05d1, 0x0ce2,0x05d2, 0x0ce3,0x05d3, 0x0ce4,0x05d4, 0x0ce5,0x05d5, 0x0ce6,0x05d6, 0x0ce7,0x05d7, 0x0ce8,0x05d8, 0x0ce9,0x05d9, 0x0cea,0x05da, 0x0ceb,0x05db, 0x0cec,0x05dc, 0x0ced,0x05dd, 0x0cee,0x05de, 0x0cef,0x05df, 0x0cf0,0x05e0, 0x0cf1,0x05e1, 0x0cf2,0x05e2, 0x0cf3,0x05e3, 0x0cf4,0x05e4, 0x0cf5,0x05e5,
                0x0cf6,0x05e6, 0x0cf7,0x05e7, 0x0cf8,0x05e8, 0x0cf9,0x05e9, 0x0cfa,0x05ea, 0x0da1,0x0e01, 0x0da2,0x0e02, 0x0da3,0x0e03, 0x0da4,0x0e04, 0x0da5,0x0e05, 0x0da6,0x0e06, 0x0da7,0x0e07, 0x0da8,0x0e08, 0x0da9,0x0e09, 0x0daa,0x0e0a, 0x0dab,0x0e0b, 0x0dac,0x0e0c, 0x0dad,0x0e0d, 0x0dae,0x0e0e, 0x0daf,0x0e0f, 0x0db0,0x0e10, 0x0db1,0x0e11, 0x0db2,0x0e12, 0x0db3,0x0e13, 0x0db4,0x0e14, 0x0db5,0x0e15, 0x0db6,0x0e16, 0x0db7,0x0e17, 0x0db8,0x0e18, 0x0db9,0x0e19,
                0x0dba,0x0e1a, 0x0dbb,0x0e1b, 0x0dbc,0x0e1c, 0x0dbd,0x0e1d, 0x0dbe,0x0e1e, 0x0dbf,0x0e1f, 0x0dc0,0x0e20, 0x0dc1,0x0e21, 0x0dc2,0x0e22, 0x0dc3,0x0e23, 0x0dc4,0x0e24, 0x0dc5,0x0e25, 0x0dc6,0x0e26, 0x0dc7,0x0e27, 0x0dc8,0x0e28, 0x0dc9,0x0e29, 0x0dca,0x0e2a, 0x0dcb,0x0e2b, 0x0dcc,0x0e2c, 0x0dcd,0x0e2d, 0x0dce,0x0e2e, 0x0dcf,0x0e2f, 0x0dd0,0x0e30, 0x0dd1,0x0e31, 0x0dd2,0x0e32, 0x0dd3,0x0e33, 0x0dd4,0x0e34, 0x0dd5,0x0e35, 0x0dd6,0x0e36, 0x0dd7,0x0e37,
                0x0dd8,0x0e38, 0x0dd9,0x0e39, 0x0dda,0x0e3a, 0x0dde,0x0e3e, 0x0ddf,0x0e3f, 0x0de0,0x0e40, 0x0de1,0x0e41, 0x0de2,0x0e42, 0x0de3,0x0e43, 0x0de4,0x0e44, 0x0de5,0x0e45, 0x0de6,0x0e46, 0x0de7,0x0e47, 0x0de8,0x0e48, 0x0de9,0x0e49, 0x0dea,0x0e4a, 0x0deb,0x0e4b, 0x0dec,0x0e4c, 0x0ded,0x0e4d, 0x0df0,0x0e50, 0x0df1,0x0e51, 0x0df2,0x0e52, 0x0df3,0x0e53, 0x0df4,0x0e54, 0x0df5,0x0e55, 0x0df6,0x0e56, 0x0df7,0x0e57, 0x0df8,0x0e58, 0x0df9,0x0e59, 0x0ea1,0x3131,
                0x0ea2,0x3132, 0x0ea3,0x3133, 0x0ea4,0x3134, 0x0ea5,0x3135, 0x0ea6,0x3136, 0x0ea7,0x3137, 0x0ea8,0x3138, 0x0ea9,0x3139, 0x0eaa,0x313a, 0x0eab,0x313b, 0x0eac,0x313c, 0x0ead,0x313d, 0x0eae,0x313e, 0x0eaf,0x313f, 0x0eb0,0x3140, 0x0eb1,0x3141, 0x0eb2,0x3142, 0x0eb3,0x3143, 0x0eb4,0x3144, 0x0eb5,0x3145, 0x0eb6,0x3146, 0x0eb7,0x3147, 0x0eb8,0x3148, 0x0eb9,0x3149, 0x0eba,0x314a, 0x0ebb,0x314b, 0x0ebc,0x314c, 0x0ebd,0x314d, 0x0ebe,0x314e, 0x0ebf,0x314f,
                0x0ec0,0x3150, 0x0ec1,0x3151, 0x0ec2,0x3152, 0x0ec3,0x3153, 0x0ec4,0x3154, 0x0ec5,0x3155, 0x0ec6,0x3156, 0x0ec7,0x3157, 0x0ec8,0x3158, 0x0ec9,0x3159, 0x0eca,0x315a, 0x0ecb,0x315b, 0x0ecc,0x315c, 0x0ecd,0x315d, 0x0ece,0x315e, 0x0ecf,0x315f, 0x0ed0,0x3160, 0x0ed1,0x3161, 0x0ed2,0x3162, 0x0ed3,0x3163, 0x0ed4,0x11a8, 0x0ed5,0x11a9, 0x0ed6,0x11aa, 0x0ed7,0x11ab, 0x0ed8,0x11ac, 0x0ed9,0x11ad, 0x0eda,0x11ae, 0x0edb,0x11af, 0x0edc,0x11b0, 0x0edd,0x11b1,
                0x0ede,0x11b2, 0x0edf,0x11b3, 0x0ee0,0x11b4, 0x0ee1,0x11b5, 0x0ee2,0x11b6, 0x0ee3,0x11b7, 0x0ee4,0x11b8, 0x0ee5,0x11b9, 0x0ee6,0x11ba, 0x0ee7,0x11bb, 0x0ee8,0x11bc, 0x0ee9,0x11bd, 0x0eea,0x11be, 0x0eeb,0x11bf, 0x0eec,0x11c0, 0x0eed,0x11c1, 0x0eee,0x11c2, 0x0eef,0x316d, 0x0ef0,0x3171, 0x0ef1,0x3178, 0x0ef2,0x317f, 0x0ef3,0x3181, 0x0ef4,0x3184, 0x0ef5,0x3186, 0x0ef6,0x318d, 0x0ef7,0x318e, 0x0ef8,0x11eb, 0x0ef9,0x11f0, 0x0efa,0x11f9, 0x0eff,0x20a9,
                0x20ac,0x20ac
            });
            auto m = std::array<ui16, 65536>{};
            for (auto i = 0u; i < pairs.size(); i += 2)
            {
                auto key_sym = pairs[i];
                auto unicode = pairs[i + 1];
                m[key_sym] = unicode;
            }
            return m;
        }();
    }
}