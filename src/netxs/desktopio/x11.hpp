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
            ui32 serial      = 0; // Serial number to sync replay.
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
        struct query_keymap // Opcode 44 (query keyboard state).
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
        struct get_keyboard_control // Opcode 101 (get keybd control).
        {
            struct reply
            {
                byte type;
                byte global_auto_repeat; // 1 = on, 0 = off.
                ui16 sequence;
                ui32 length;
                ui32 led_mask;           // LED bit fileld.
                byte key_click_percent;
                byte bell_percent;
                ui16 bell_pitch;
                ui16 bell_duration;
                ui16 pad0;
                byte auto_repeats[32];
            };
            byte opcode = 101;
            byte pad0   = 0;
            ui16 length = 1;
        };
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
                static constexpr auto Shift    = 1u << 0; // Shift.
                static constexpr auto CapsLock = 1u << 1; // CapsLock.
                static constexpr auto Ctrl     = 1u << 2; // Control.
                static constexpr auto mod1     = 1u << 3; // Alt.
                static constexpr auto mod2     = 1u << 4; // NumLock.
                static constexpr auto mod3     = 1u << 5; // AltGr.
                static constexpr auto mod4     = 1u << 6; // Win/Super.
                static constexpr auto mod5     = 1u << 7; // ScrollLock.
                
                static constexpr auto Alt        = mod1;
                static constexpr auto NumLock    = mod2;
                static constexpr auto AltGr      = mod3;
                static constexpr auto Win        = mod4;
                static constexpr auto ScrollLock = mod5;
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
            struct query_version
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
                ui16 client_major_version = 2;
                ui16 client_minor_version = 2;
            };
            struct get_map
            {
                struct reply
                {
                    byte type;
                    byte device_id;      // Device ID
                    ui16 sequence;
                    ui32 length;         // Payload len in quads.
                    ui16 device_spec;
                    ui16 present;        // Payload bitmask.
                    ui16 first_type;
                    ui16 num_types;
                    byte first_key;      // Start key.
                    byte num_keys;       // Key count.
                    byte first_key_act;
                    byte num_acts;
                    byte first_key_behavior;
                    byte num_behaviors;
                    byte first_key_explicit;
                    byte num_explicits;
                    byte first_mod_map;
                    byte num_mod_maps;
                    byte first_vmod_map;
                    byte num_vmod_maps;
                    ui32 pad;
                    // payload...
                    struct key_behavior_info
                    {
                        byte behavior;
                        byte num_groups;
                        byte width;
                        byte pad;
                    };
                };

                byte major_opcode;    // xkb_major_opcode
                byte minor_opcode = 8; // 8: XkbGetMap.
                ui16 length       = 7;
                ui16 deviceSpec   = 0x0100; // XkbUseCoreKbd (system keybd).
                ui16 full         = 0x0002; // XkbKeySymsMask (only KeySym).
                ui16 partial      = 0;
                byte first_type   = 0;
                byte num_types    = 0;
                byte first_key    = 8;   // s.min_keycode.
                byte num_keys     = 255; // s.max_keycode - s.min_keycode + 1 (all keys).
                ui32 pad[6]       = {};

                //byte first_key_act      = 0;
                //byte num_acts           = 0;
                //byte first_key_behavior = 0;
                //byte num_behaviors      = 0;
                //byte first_key_explicit = 0;
                //byte num_explicit       = 0;
                //byte first_mod_map      = 0;
                //byte num_mod_maps       = 0;
                //byte first_vmod_map     = 0;
                //byte num_vmod_maps      = 0;
                //ui16 pad1               = 0;
            };
            struct per_client_flags
            {
                static constexpr auto DetectableAutoRepeat = 1u;
                static constexpr auto DetectableAutoRepeatMask = 1u << (DetectableAutoRepeat - 1);

                static constexpr auto UseCoreKbd = 0x0100;

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
        //ui32                                  atom_wm_protocols = 0;
        //ui32                                  atom_net_wm_bypass_compositor = 0; //_XWAYLAND_ALLOW_FRACTIONAL_SCALE
        ui32                                  atom_atom = 0;
        ui32                                  atom_cardinal = 0;
        ui32                                  atom_utf8_string = 0;
        ui32                                  atom_window = 0;
        ui32                                  atom_net_active_window = 0;
        ui32                                  atom_net_number_of_desktops = 0;
        ui32                                  atom_net_current_desktop = 0;
        ui32                                  atom_net_workarea = 0; // workarea = desktop_area if is not set (atom_net_workarea=0).

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
                x11connection->recv(read_buffer.data() + 32, extra_data_size); // Blocking call.
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
            return e == shm_completion_event ? "ShmCompletionEvent" : x11::event::str(e);
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
            if (x11connection->recv((char*)&reply, sizeof(reply)).size() == sizeof(reply))
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
                if (x11connection->recv((char*)&v_reply, sizeof(v_reply)).size() == sizeof(v_reply))
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
            //todo it doesn't work // Set WM_PROTOCOLS.
            //sendrq<x11::req::change_property>({ .window_id = (ui32)new_window_id,
            //                                    .property  = atom_wm_protocols,
            //                                    .type      = atom_atom },
            //                                std::to_array({ atom_net_wm_sync_request, atom_net_wm_ping }));
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
                if (x11connection->recv((char*)&reply, sizeof(reply)).size() == 32 && reply.type == x11::event::Reply)
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
            //atom_wm_protocols           = get_atom_id("WM_PROTOCOLS",     faux);
            atom_net_workarea           = get_atom_id("_NET_WORKAREA",    faux);
            atom_net_active_window      = get_atom_id("_NET_ACTIVE_WINDOW",      true);
            atom_net_number_of_desktops = get_atom_id("_NET_NUMBER_OF_DESKTOPS", true);
            atom_net_current_desktop    = get_atom_id("_NET_CURRENT_DESKTOP",    true);
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
                if (x11connection->recv((char*)&reply, sizeof(reply)).size() == 32 && reply.type == x11::event::Reply && reply.prop_type == atom_cardinal)
                {
                    //todo unify
                    auto payload_size = reply.length * 4;
                    auto buffer = text(payload_size, '\0');
                    if (x11connection->recv(buffer.data(), buffer.size()).size() != buffer.size()) return faux;
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
        auto enable_detectable_autorepeat()
        {
            sendrq<x11::req::xkb::per_client_flags>({ .major_opcode = xkb_major_opcode,
                                                      .device_spec  = x11::req::xkb::per_client_flags::UseCoreKbd,
                                                      .change_mask  = x11::req::xkb::per_client_flags::DetectableAutoRepeatMask,
                                                      .value        = x11::req::xkb::per_client_flags::DetectableAutoRepeat });
            auto reply = x11::req::xkb::per_client_flags::reply{};
            if (x11connection->recv((char*)&reply, sizeof(reply)).size() == sizeof(reply) && reply.type == x11::event::Reply)
            {
                if constexpr (debugmode)
                {
                    auto detectable_auto_repeat_state = (reply.value & x11::req::xkb::per_client_flags::DetectableAutoRepeatMask) != 0;
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
            //while (x11connection->recv(buffer.data(), buffer.size()).size() == 32) // Wait for ConfigureNotify.
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
            //while (x11connection->recv(buffer.data(), buffer.size()).size() == 32) // Cleanup.
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
        auto load_keyboard_map()
        {
            sendrq(x11::req::xkb::get_map{ .major_opcode = xkb_major_opcode,
                                           .first_key    = s.min_keycode,
                                           .num_keys     = (byte)(s.max_keycode - s.min_keycode + 1) });
            auto reply = x11::req::xkb::get_map::reply{};
            if (x11connection->recv((char*)&reply, sizeof(reply)).size() == sizeof(reply))
            {
                auto payload_size = reply.length * 4;
                auto buffer = text(payload_size, '\0');
                if (x11connection->recv(buffer.data(), buffer.size()).size() != buffer.size()) return faux;
                auto q = qiew{ buffer };
                auto offset = reply.num_keys * 4;
                auto ptr = q.data();
                for (auto i = 0u; i < reply.num_keys; ++i)
                {
                    auto keycode = (byte)(reply.first_key + i);
                    auto& map_entry = key_map[keycode];
                    auto info = netxs::start_lifetime_as<x11::req::xkb::get_map::reply::key_behavior_info>(q.data());
                    map_entry.num_groups = info.num_groups;
                    map_entry.width      = info.width;
                    auto total_syms_for_key = info.num_groups * info.width;
                    if (total_syms_for_key)
                    {
                        auto total_syms_for_key_bytes = total_syms_for_key * 4;
                        map_entry.syms.resize(total_syms_for_key);
                        if (q.size() < (size_t)total_syms_for_key_bytes) break;
                        std::memcpy(map_entry.syms.data(), ptr + offset, total_syms_for_key_bytes);
                        offset += total_syms_for_key_bytes;
                    }
                    q.remove_prefix(sizeof(info));
                }
                return true;
            }
            return faux;
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
                if (link->recv((char*)&reply, sizeof(reply)).size() == sizeof(reply))
                {
                    auto buffer = text(reply.additional_length * 4, '\0');
                    link->recv(buffer.data(), buffer.size());
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
        if (auto l1 = x11connection->recv((char*)&reply, sizeof(reply)); l1.size() == sizeof(reply))
        {
            auto remaining_bytes = (size_t)reply.additional_length * 4;
            auto buffer = text(remaining_bytes, '\0');
            if (reply.status == x11::event::Error)
            {
                log("%%Connection rejected: '%%'", prompt::x11, utf::debase<faux, faux>(x11connection->recv(buffer.data(), buffer.size())));
            }
            else if (reply.status != x11::event::Reply)
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
}