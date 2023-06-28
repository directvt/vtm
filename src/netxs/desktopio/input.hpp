// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#include "baseui.hpp"

namespace netxs::events::userland
{
    //using proc_fx = std::function<void(ui::base&)>;

    struct hids
    {
        EVENTPACK( hids, netxs::events::userland::root::hids )
        {
            EVENT_XS( die    , input::hids ), // release::global: Notify about the mouse controller is gone. Signal to delete gears inside dtvt-objects.
            EVENT_XS( halt   , input::hids ), // release::global: Notify about the mouse controller is outside.
            EVENT_XS( spawn  , input::hids ), // release::global: Notify about the mouse controller is appear.
            GROUP_XS( clipbrd, input::hids ), // release/request: Set/get clipboard data.
            GROUP_XS( keybd  , input::hids ),
            GROUP_XS( mouse  , input::hids ),
            GROUP_XS( focus  , input::hids ), // release::global: Notify about the focus got/lost.
            GROUP_XS( notify , input::hids ), // Form events that should be propagated down to the visual branch.
            GROUP_XS( device , input::hids ), // Primary device event group for forwarding purposes.

            SUBSET_XS( clipbrd )
            {
                EVENT_XS( get, input::hids ), // release: Get clipboard data.
                EVENT_XS( set, input::hids ), // release: Set clipboard data.
            };
            SUBSET_XS( notify )
            {
                GROUP_XS( mouse, input::hids ),
                //GROUP_XS( keybd, input::hids ),
                //GROUP_XS( focus, input::hids ),

                SUBSET_XS( mouse )
                {
                    EVENT_XS( enter, input::hids ), // inform the form about the mouse hover.
                    EVENT_XS( leave, input::hids ), // inform the form about leaving the mouse.
                };
                //SUBSET_XS( keybd )
                //{
                //    EVENT_XS( got , input::hids ),
                //    EVENT_XS( lost, input::hids ),
                //};
                //SUBSET_XS( focus )
                //{
                //    EVENT_XS( got , input::hids ),
                //    EVENT_XS( lost, input::hids ),
                //};
            };
            SUBSET_XS( keybd )
            {
                EVENT_XS( down   , input::hids ),
                EVENT_XS( up     , input::hids ),
                GROUP_XS( data   , input::hids ),
                GROUP_XS( control, input::hids ),
                GROUP_XS( state  , input::hids ),
                GROUP_XS( focus  , input::hids ),

                SUBSET_XS( data )
                {
                    EVENT_XS( post, input::hids ),
                };
                SUBSET_XS( control )
                {
                    GROUP_XS( up  , input::hids ),
                    GROUP_XS( down, input::hids ),

                    SUBSET_XS( up )
                    {
                        EVENT_XS( alt_right  , input::hids ),
                        EVENT_XS( alt_left   , input::hids ),
                        EVENT_XS( ctrl_right , input::hids ),
                        EVENT_XS( ctrl_left  , input::hids ),
                        EVENT_XS( shift_right, input::hids ),
                        EVENT_XS( shift_left , input::hids ),
                    };
                    SUBSET_XS( down )
                    {
                        EVENT_XS( alt_right  , input::hids ),
                        EVENT_XS( alt_left   , input::hids ),
                        EVENT_XS( ctrl_right , input::hids ),
                        EVENT_XS( ctrl_left  , input::hids ),
                        EVENT_XS( shift_right, input::hids ),
                        EVENT_XS( shift_left , input::hids ),
                    };
                };
                SUBSET_XS( state )
                {
                    GROUP_XS( on , input::hids ),
                    GROUP_XS( off, input::hids ),

                    SUBSET_XS( on )
                    {
                        EVENT_XS( numlock   , input::hids ),
                        EVENT_XS( capslock  , input::hids ),
                        EVENT_XS( scrolllock, input::hids ),
                        EVENT_XS( insert    , input::hids ),
                    };
                    SUBSET_XS( off )
                    {
                        EVENT_XS( numlock   , input::hids ),
                        EVENT_XS( capslock  , input::hids ),
                        EVENT_XS( scrolllock, input::hids ),
                        EVENT_XS( insert    , input::hids ),
                    };
                };
                SUBSET_XS( focus )
                {
                    //EVENT_XS( tie , proc_fx ),
                    //EVENT_XS( die , input::foci ),
                    EVENT_XS( set, input::foci ),
                    EVENT_XS( get, input::foci ),
                    EVENT_XS( off, input::foci ),
                    EVENT_XS( cut, input::foci ), // Cut mono focus branch.
                    EVENT_XS( dry, input::foci ), // Remove the reference to the specified applet.
                    GROUP_XS( bus, input::foci ),

                    SUBSET_XS( bus )
                    {
                        EVENT_XS( on  , input::foci ),
                        EVENT_XS( off , input::foci ),
                        EVENT_XS( copy, input::foci ), // Copy default focus branch.
                    };
                };
            };
            SUBSET_XS( mouse )
            {
                EVENT_XS( move   , input::hids ),
                EVENT_XS( focus  , input::hids ),
                GROUP_XS( button , input::hids ),
                GROUP_XS( scroll , input::hids ),

                SUBSET_XS( scroll )
                {
                    EVENT_XS( up  , input::hids ),
                    EVENT_XS( down, input::hids ),
                };
                SUBSET_XS( button )
                {
                    GROUP_XS( up      , input::hids ),
                    GROUP_XS( down    , input::hids ),
                    GROUP_XS( click   , input::hids ),
                    GROUP_XS( dblclick, input::hids ),
                    GROUP_XS( tplclick, input::hids ),
                    GROUP_XS( drag    , input::hids ),

                    SUBSET_XS( up )
                    {
                        EVENT_XS( left     , input::hids ),
                        EVENT_XS( right    , input::hids ),
                        EVENT_XS( middle   , input::hids ),
                        EVENT_XS( wheel    , input::hids ),
                        EVENT_XS( win      , input::hids ),
                        EVENT_XS( leftright, input::hids ),
                    };
                    SUBSET_XS( down )
                    {
                        EVENT_XS( left     , input::hids ),
                        EVENT_XS( right    , input::hids ),
                        EVENT_XS( middle   , input::hids ),
                        EVENT_XS( wheel    , input::hids ),
                        EVENT_XS( win      , input::hids ),
                        EVENT_XS( leftright, input::hids ),
                    };
                    SUBSET_XS( click )
                    {
                        EVENT_XS( left     , input::hids ),
                        EVENT_XS( right    , input::hids ),
                        EVENT_XS( middle   , input::hids ),
                        EVENT_XS( wheel    , input::hids ),
                        EVENT_XS( win      , input::hids ),
                        EVENT_XS( leftright, input::hids ),

                        INDEX_XS( left, right, middle, wheel, win, leftright ),
                    };
                    SUBSET_XS( dblclick )
                    {
                        EVENT_XS( left     , input::hids ),
                        EVENT_XS( right    , input::hids ),
                        EVENT_XS( middle   , input::hids ),
                        EVENT_XS( wheel    , input::hids ),
                        EVENT_XS( win      , input::hids ),
                        EVENT_XS( leftright, input::hids ),
                    };
                    SUBSET_XS( tplclick )
                    {
                        EVENT_XS( left     , input::hids ),
                        EVENT_XS( right    , input::hids ),
                        EVENT_XS( middle   , input::hids ),
                        EVENT_XS( wheel    , input::hids ),
                        EVENT_XS( win      , input::hids ),
                        EVENT_XS( leftright, input::hids ),
                    };
                    SUBSET_XS( drag )
                    {
                        GROUP_XS( start , input::hids ),
                        GROUP_XS( pull  , input::hids ),
                        GROUP_XS( cancel, input::hids ),
                        GROUP_XS( stop  , input::hids ),

                        SUBSET_XS( start )
                        {
                            EVENT_XS( left     , input::hids ),
                            EVENT_XS( right    , input::hids ),
                            EVENT_XS( middle   , input::hids ),
                            EVENT_XS( wheel    , input::hids ),
                            EVENT_XS( win      , input::hids ),
                            EVENT_XS( leftright, input::hids ),

                            INDEX_XS( left, right, middle, wheel, win, leftright ),
                        };
                        SUBSET_XS( pull )
                        {
                            EVENT_XS( left     , input::hids ),
                            EVENT_XS( right    , input::hids ),
                            EVENT_XS( middle   , input::hids ),
                            EVENT_XS( wheel    , input::hids ),
                            EVENT_XS( win      , input::hids ),
                            EVENT_XS( leftright, input::hids ),

                            INDEX_XS( left, right,  middle, wheel, win, leftright ),
                        };
                        SUBSET_XS( cancel )
                        {
                            EVENT_XS( left     , input::hids ),
                            EVENT_XS( right    , input::hids ),
                            EVENT_XS( middle   , input::hids ),
                            EVENT_XS( wheel    , input::hids ),
                            EVENT_XS( win      , input::hids ),
                            EVENT_XS( leftright, input::hids ),

                            INDEX_XS( left, right, middle, wheel, win, leftright ),
                        };
                        SUBSET_XS( stop )
                        {
                            EVENT_XS( left     , input::hids ),
                            EVENT_XS( right    , input::hids ),
                            EVENT_XS( middle   , input::hids ),
                            EVENT_XS( wheel    , input::hids ),
                            EVENT_XS( win      , input::hids ),
                            EVENT_XS( leftright, input::hids ),

                            INDEX_XS( left, right, middle, wheel, win, leftright ),
                        };
                    };
                };
            };
            SUBSET_XS( focus )
            {
                EVENT_XS( set, input::hids ), // release: Set keybd focus.
                EVENT_XS( off, input::hids ), // release: Off keybd focus.
            };
            SUBSET_XS( device )
            {
                //EVENT_XS( keybd, input::hids ), // release: Primary keybd event for forwarding purposes.
                GROUP_XS( mouse, input::hids ), // release: Primary mouse event for forwarding purposes.
                GROUP_XS( user , id_t        ), // Device properties.

                SUBSET_XS( mouse )
                {
                    EVENT_XS( on, input::hids ),
                };
                SUBSET_XS( user )
                {
                    EVENT_XS( login , id_t ),
                    EVENT_XS( logout, id_t ),
                };
            };
        };
    };
}

namespace netxs::input
{
    using netxs::ansi::clip;
    using netxs::ui::base;
    using netxs::ui::face;
    using netxs::ui::page;

    namespace key
    {
        namespace vk
        {
            static constexpr ui16 Backspace  = 0x08;
            static constexpr ui16 Tab        = 0x09;
            static constexpr ui16 Clear      = 0x0C;
            static constexpr ui16 Enter      = 0x0D;
            static constexpr ui16 Shift      = 0x10;
            static constexpr ui16 Control    = 0x11;
            static constexpr ui16 Alt        = 0x12;
            static constexpr ui16 Pause      = 0x13;
            static constexpr ui16 Escape     = 0x1B;
            static constexpr ui16 PageUp     = 0x21;
            static constexpr ui16 PageDown   = 0x22;
            static constexpr ui16 End        = 0x23;
            static constexpr ui16 Home       = 0x24;
            static constexpr ui16 LeftArrow  = 0x25;
            static constexpr ui16 UpArrow    = 0x26;
            static constexpr ui16 RightArrow = 0x27;
            static constexpr ui16 DownArrow  = 0x28;
            static constexpr ui16 Insert     = 0x2D;
            static constexpr ui16 Delete     = 0x2E;
            static constexpr ui16 F1         = 0x70;
            static constexpr ui16 F2         = 0x71;
            static constexpr ui16 F3         = 0x72;
            static constexpr ui16 F4         = 0x73;
            static constexpr ui16 F5         = 0x74;
            static constexpr ui16 F6         = 0x75;
            static constexpr ui16 F7         = 0x76;
            static constexpr ui16 F8         = 0x77;
            static constexpr ui16 F9         = 0x78;
            static constexpr ui16 F10        = 0x79;
            static constexpr ui16 F11        = 0x7A;
            static constexpr ui16 F12        = 0x7B;
            static constexpr ui16 F13        = 0x7C;
            static constexpr ui16 F14        = 0x7D;
            static constexpr ui16 F15        = 0x7E;
            static constexpr ui16 F16        = 0x7F;
            static constexpr ui16 F17        = 0x80;
            static constexpr ui16 F18        = 0x81;
            static constexpr ui16 F19        = 0x82;
            static constexpr ui16 F20        = 0x83;
            static constexpr ui16 F21        = 0x84;
            static constexpr ui16 F22        = 0x85;
            static constexpr ui16 F23        = 0x86;
            static constexpr ui16 F24        = 0x87;
            static constexpr ui16 Undo       = 0xFFCB;
            static constexpr ui16 Redo       = 0xFFC9;
        }

        static constexpr auto undef            = 0;
        static constexpr auto Esc              = 2;
        static constexpr auto Space            = 4;
        static constexpr auto Backspace        = 6;
        static constexpr auto Tab              = 8;
        static constexpr auto LeftShift        = 10;
        static constexpr auto RightShift       = 11;
        static constexpr auto LeftCtrl         = 12;
        static constexpr auto RightCtrl        = 13;
        static constexpr auto LeftAlt          = 14;
        static constexpr auto RightAlt         = 15;
        static constexpr auto LeftWin          = 16;
        static constexpr auto RightWin         = 17;
        static constexpr auto CapsLock         = 18;
        static constexpr auto NumLock          = 20;
        static constexpr auto ScrollLock       = 22;
        static constexpr auto Apps             = 24;
        static constexpr auto Break            = 26;
        static constexpr auto Pause            = 28;
        static constexpr auto Select           = 30;
        static constexpr auto SysRq            = 32;
        static constexpr auto PrintScreen      = 34;
        static constexpr auto Enter            = 36;
        static constexpr auto NumpadEnter      = 37;
        static constexpr auto PageUp           = 38;
        static constexpr auto NumpadPageUp     = 39;
        static constexpr auto PageDown         = 40;
        static constexpr auto NumpadPageDown   = 41;
        static constexpr auto End              = 42;
        static constexpr auto NumpadEnd        = 43;
        static constexpr auto Home             = 44;
        static constexpr auto NumpadHome       = 45;
        static constexpr auto LeftArrow        = 46;
        static constexpr auto NumpadLeftArrow  = 47;
        static constexpr auto UpArrow          = 48;
        static constexpr auto NumpadUpArrow    = 49;
        static constexpr auto RightArrow       = 50;
        static constexpr auto NumpadRightArrow = 51;
        static constexpr auto DownArrow        = 52;
        static constexpr auto NumpadDownArrow  = 53;
        static constexpr auto Key0             = 54;
        static constexpr auto Numpad0          = 55;
        static constexpr auto Key1             = 56;
        static constexpr auto Numpad1          = 57;
        static constexpr auto Key2             = 58;
        static constexpr auto Numpad2          = 59;
        static constexpr auto Key3             = 60;
        static constexpr auto Numpad3          = 61;
        static constexpr auto Key4             = 62;
        static constexpr auto Numpad4          = 63;
        static constexpr auto Key5             = 64;
        static constexpr auto Numpad5          = 65;
        static constexpr auto Key6             = 66;
        static constexpr auto Numpad6          = 67;
        static constexpr auto Key7             = 68;
        static constexpr auto Numpad7          = 69;
        static constexpr auto Key8             = 70;
        static constexpr auto Numpad8          = 71;
        static constexpr auto Key9             = 72;
        static constexpr auto Numpad9          = 73;
        static constexpr auto Insert           = 74;
        static constexpr auto NumpadInsert     = 75;
        static constexpr auto Delete           = 76;
        static constexpr auto NumpadDelete     = 77;
        static constexpr auto Clear            = 78;
        static constexpr auto NumpadClear      = 79;
        static constexpr auto Multiply         = 80;
        static constexpr auto NumpadMultiply   = 81;
        static constexpr auto Plus             = 82;
        static constexpr auto NumpadPlus       = 83;
        static constexpr auto Separator        = 84;
        static constexpr auto NumpadSeparator  = 85;
        static constexpr auto Minus            = 86;
        static constexpr auto NumpadMinus      = 87;
        static constexpr auto Period           = 88;
        static constexpr auto NumpadDecimal    = 89;
        static constexpr auto Slash            = 90;
        static constexpr auto NumpadSlash      = 91;
        static constexpr auto BackSlash        = 92;
        static constexpr auto OpenBracket      = 94;
        static constexpr auto ClosedBracket    = 96;
        static constexpr auto Equal            = 98;
        static constexpr auto BackQuote        = 100;
        static constexpr auto SingleQuote      = 102;
        static constexpr auto Comma            = 104;
        static constexpr auto Semicolon        = 106;
        static constexpr auto F1               = 108;
        static constexpr auto F2               = 110;
        static constexpr auto F3               = 112;
        static constexpr auto F4               = 114;
        static constexpr auto F5               = 116;
        static constexpr auto F6               = 118;
        static constexpr auto F7               = 120;
        static constexpr auto F8               = 122;
        static constexpr auto F9               = 124;
        static constexpr auto F10              = 126;
        static constexpr auto F11              = 128;
        static constexpr auto F12              = 130;
        static constexpr auto F13              = 132;
        static constexpr auto F14              = 134;
        static constexpr auto F15              = 136;
        static constexpr auto F16              = 138;
        static constexpr auto F17              = 140;
        static constexpr auto F18              = 142;
        static constexpr auto F19              = 144;
        static constexpr auto F20              = 146;
        static constexpr auto F21              = 148;
        static constexpr auto F22              = 150;
        static constexpr auto F23              = 152;
        static constexpr auto F24              = 154;
        static constexpr auto KeyA             = 156;
        static constexpr auto KeyB             = 158;
        static constexpr auto KeyC             = 160;
        static constexpr auto KeyD             = 162;
        static constexpr auto KeyE             = 164;
        static constexpr auto KeyF             = 166;
        static constexpr auto KeyG             = 168;
        static constexpr auto KeyH             = 170;
        static constexpr auto KeyI             = 172;
        static constexpr auto KeyJ             = 174;
        static constexpr auto KeyK             = 176;
        static constexpr auto KeyL             = 178;
        static constexpr auto KeyM             = 180;
        static constexpr auto KeyN             = 182;
        static constexpr auto KeyO             = 184;
        static constexpr auto KeyP             = 186;
        static constexpr auto KeyQ             = 188;
        static constexpr auto KeyR             = 190;
        static constexpr auto KeyS             = 192;
        static constexpr auto KeyT             = 194;
        static constexpr auto KeyU             = 196;
        static constexpr auto KeyV             = 198;
        static constexpr auto KeyW             = 200;
        static constexpr auto KeyX             = 202;
        static constexpr auto KeyY             = 204;
        static constexpr auto KeyZ             = 206;
        static constexpr auto Sleep            = 208;
        static constexpr auto WWW              = 210;
        static constexpr auto Calculator       = 212;
        static constexpr auto Mail             = 214;
        static constexpr auto MediaVolMute     = 216;
        static constexpr auto MediaVolDown     = 218;
        static constexpr auto MediaVolUp       = 220;
        static constexpr auto MediaNext        = 222;
        static constexpr auto MediaPrev        = 224;
        static constexpr auto MediaStop        = 226;
        static constexpr auto MediaPlayPause   = 228;
        static constexpr auto MediaSelect      = 230;
        static constexpr auto BrowserBack      = 232;
        static constexpr auto BrowserForward   = 234;
        static constexpr auto BrowserRefresh   = 236;
        static constexpr auto BrowserStop      = 238;
        static constexpr auto BrowserSearch    = 240;
        static constexpr auto BrowserFavorites = 242;
        static constexpr auto BrowserHome      = 244;

        static constexpr auto ExtendedKey = 0x0100;
        static constexpr auto NumLockMode = 0x0020;

        struct mapkey
        {
            si32 vk; // mapkey: Virtual code.
            si32 sc; // mapkey: Scan code.
            si32 cs; // mapkey: Control state.

            mapkey(si32 vk, si32 sc, si32 cs)
                : vk{ vk }, sc{ sc }, cs{ cs }
            { }

            bool operator == (mapkey const&) const = default;
            struct hash
            {
                auto operator()(mapkey const& k) const
                {
                    return std::size_t{ (ui64)k.vk | ((ui64)k.sc << 16) | ((ui64)k.sc << 32) };
                }
            };
        };

        static const auto xlat = std::unordered_map<mapkey, si32, mapkey::hash>
        {
            {{    0,    0,           0 }, undef            },
            {{ 0x1B, 0x01,           0 }, Esc              },
            {{ 0x20, 0x39,           0 }, Space            },
            {{ 0x08, 0x0E,           0 }, Backspace        },
            {{ 0x09, 0x0F,           0 }, Tab              },
            {{ 0x10, 0x2A,           0 }, LeftShift        },
            {{ 0x10, 0x36,           0 }, RightShift       },
            {{ 0x11, 0x1D,           0 }, LeftCtrl         },
            {{ 0x11, 0x1D, ExtendedKey }, RightCtrl        },
            {{ 0x12, 0x38,           0 }, LeftAlt          },
            {{ 0x12, 0x38, ExtendedKey }, RightAlt         },
            {{ 0x5B, 0x5B, ExtendedKey }, LeftWin          },
            {{ 0x5C, 0x5C, ExtendedKey }, RightWin         },
            {{ 0x14, 0x3A,           0 }, CapsLock         },
            {{ 0x90, 0x45,           0 }, NumLock          },
            {{ 0x91, 0x45,           0 }, ScrollLock       },
            {{ 0x5D, 0x5D, ExtendedKey }, Apps             },
            {{ 0x03, 0x45,           0 }, Break            },
            {{ 0x13, 0x45,           0 }, Pause            },
            {{ 0x29,    0,           0 }, Select           },
            {{ 0x2C, 0x54,           0 }, SysRq            },
            {{ 0x2C, 0x37, ExtendedKey }, PrintScreen      },
            {{ 0x0D, 0x1C,           0 }, Enter            },
            {{ 0x0D, 0x1C, ExtendedKey }, NumpadEnter      },
            {{ 0x21, 0x49, ExtendedKey }, PageUp           },
            {{ 0x21, 0x49,           0 }, NumpadPageUp     },
            {{ 0x22, 0x51, ExtendedKey }, PageDown         },
            {{ 0x22, 0x51,           0 }, NumpadPageDown   },
            {{ 0x23, 0x4F, ExtendedKey }, End              },
            {{ 0x23, 0x4F,           0 }, NumpadEnd        },
            {{ 0x24, 0x47, ExtendedKey }, Home             },
            {{ 0x24, 0x47,           0 }, NumpadHome       },
            {{ 0x25, 0x4B, ExtendedKey }, LeftArrow        },
            {{ 0x25, 0x4B,           0 }, NumpadLeftArrow  },
            {{ 0x26, 0x48, ExtendedKey }, UpArrow          },
            {{ 0x26, 0x48,           0 }, NumpadUpArrow    },
            {{ 0x27, 0x4D, ExtendedKey }, RightArrow       },
            {{ 0x27, 0x4D,           0 }, NumpadRightArrow },
            {{ 0x28, 0x50, ExtendedKey }, DownArrow        },
            {{ 0x28, 0x50,           0 }, NumpadDownArrow  },
            {{ 0x30, 0x0B,           0 }, Key0             },
            {{ 0x60, 0x52, NumLockMode }, Numpad0          },
            {{ 0x31, 0x02,           0 }, Key1             },
            {{ 0x61, 0x4F, NumLockMode }, Numpad1          },
            {{ 0x32, 0x03,           0 }, Key2             },
            {{ 0x62, 0x50, NumLockMode }, Numpad2          },
            {{ 0x33, 0x04,           0 }, Key3             },
            {{ 0x63, 0x51, NumLockMode }, Numpad3          },
            {{ 0x34, 0x05,           0 }, Key4             },
            {{ 0x64, 0x4B, NumLockMode }, Numpad4          },
            {{ 0x35, 0x06,           0 }, Key5             },
            {{ 0x65, 0x4C, NumLockMode }, Numpad5          },
            {{ 0x36, 0x07,           0 }, Key6             },
            {{ 0x66, 0x4D, NumLockMode }, Numpad6          },
            {{ 0x37, 0x08,           0 }, Key7             },
            {{ 0x67, 0x47, NumLockMode }, Numpad7          },
            {{ 0x38, 0x09,           0 }, Key8             },
            {{ 0x68, 0x48, NumLockMode }, Numpad8          },
            {{ 0x39, 0x0A,           0 }, Key9             },
            {{ 0x69, 0x49, NumLockMode }, Numpad9          },
            {{ 0x2D, 0x52, ExtendedKey }, Insert           },
            {{ 0x2D, 0x52,           0 }, NumpadInsert     },
            {{ 0x2E, 0x53, ExtendedKey }, Delete           },
            {{ 0x2E, 0x55,           0 }, NumpadDelete     },
            {{ 0x0C, 0x4C, ExtendedKey }, Clear            },
            {{ 0x0C, 0x4C,           0 }, NumpadClear      },
            {{ 0x6A,    0,           0 }, Multiply         },
            {{ 0x6A, 0x37,           0 }, NumpadMultiply   },
            {{ 0x6B,    0,           0 }, Plus             },
            {{ 0x6B, 0x4E,           0 }, NumpadPlus       },
            {{ 0x6C,    0,           0 }, Separator        }, //todo revise
            {{ 0x6C,    0, NumLockMode }, NumpadSeparator  }, //todo revise
            {{ 0xBD, 0x0C,           0 }, Minus            },
            {{ 0x6D, 0x4A,           0 }, NumpadMinus      },
            {{ 0xBE, 0x34,           0 }, Period           },
            {{ 0x6E, 0x53, NumLockMode }, NumpadDecimal    },
            {{ 0xBF, 0x35,           0 }, Slash            },
            {{ 0x6F, 0x35, ExtendedKey }, NumpadSlash      },
            {{ 0xDC, 0x2B,           0 }, BackSlash        },
            {{ 0xDB, 0x1A,           0 }, OpenBracket      },
            {{ 0xDD, 0x1B,           0 }, ClosedBracket    },
            {{ 0xBB, 0x0D,           0 }, Equal            },
            {{ 0xC0, 0x29,           0 }, BackQuote        },
            {{ 0xDE, 0x28,           0 }, SingleQuote      },
            {{ 0xBC, 0x33,           0 }, Comma            },
            {{ 0xBA, 0x27,           0 }, Semicolon        },
            {{ 0x70, 0x3B,           0 }, F1               },
            {{ 0x71, 0x3C,           0 }, F2               },
            {{ 0x72, 0x3D,           0 }, F3               },
            {{ 0x73, 0x3E,           0 }, F4               },
            {{ 0x74, 0x3F,           0 }, F5               },
            {{ 0x75, 0x40,           0 }, F6               },
            {{ 0x76, 0x41,           0 }, F7               },
            {{ 0x77, 0x42,           0 }, F8               },
            {{ 0x78, 0x43,           0 }, F9               },
            {{ 0x79, 0x44,           0 }, F10              },
            {{ 0x7A, 0x57,           0 }, F11              },
            {{ 0x7B, 0x5B,           0 }, F12              },
            {{ 0x7C,    0,           0 }, F13              },
            {{ 0x7D,    0,           0 }, F14              },
            {{ 0x7E,    0,           0 }, F15              },
            {{ 0x7F,    0,           0 }, F16              },
            {{ 0x80,    0,           0 }, F17              },
            {{ 0x81,    0,           0 }, F18              },
            {{ 0x82,    0,           0 }, F19              },
            {{ 0x83,    0,           0 }, F20              },
            {{ 0x84,    0,           0 }, F21              },
            {{ 0x85,    0,           0 }, F22              },
            {{ 0x86,    0,           0 }, F23              },
            {{ 0x87,    0,           0 }, F24              },
            {{ 0x41,    0,           0 }, KeyA             },
            {{ 0x42,    0,           0 }, KeyB             },
            {{ 0x43,    0,           0 }, KeyC             },
            {{ 0x44,    0,           0 }, KeyD             },
            {{ 0x45,    0,           0 }, KeyE             },
            {{ 0x46,    0,           0 }, KeyF             },
            {{ 0x47,    0,           0 }, KeyG             },
            {{ 0x48,    0,           0 }, KeyH             },
            {{ 0x49,    0,           0 }, KeyI             },
            {{ 0x4A,    0,           0 }, KeyJ             },
            {{ 0x4B,    0,           0 }, KeyK             },
            {{ 0x4C,    0,           0 }, KeyL             },
            {{ 0x4D,    0,           0 }, KeyM             },
            {{ 0x4E,    0,           0 }, KeyN             },
            {{ 0x4F,    0,           0 }, KeyO             },
            {{ 0x50,    0,           0 }, KeyP             },
            {{ 0x51,    0,           0 }, KeyQ             },
            {{ 0x52,    0,           0 }, KeyR             },
            {{ 0x53,    0,           0 }, KeyS             },
            {{ 0x54,    0,           0 }, KeyT             },
            {{ 0x55,    0,           0 }, KeyU             },
            {{ 0x56,    0,           0 }, KeyV             },
            {{ 0x57,    0,           0 }, KeyW             },
            {{ 0x58,    0,           0 }, KeyX             },
            {{ 0x59,    0,           0 }, KeyY             },
            {{ 0x5A,    0,           0 }, KeyZ             },
            {{ 0x5F,    0, ExtendedKey }, Sleep            },
            {{ 0xAC,    0, ExtendedKey }, WWW              },
            {{ 0xB7,    0, ExtendedKey }, Calculator       },
            {{ 0x48,    0, ExtendedKey }, Mail             },
            {{ 0xAD,    0, ExtendedKey }, MediaVolMute     },
            {{ 0xAE,    0, ExtendedKey }, MediaVolDown     },
            {{ 0xAF,    0, ExtendedKey }, MediaVolUp       },
            {{ 0xB0,    0, ExtendedKey }, MediaNext        },
            {{ 0xB1,    0, ExtendedKey }, MediaPrev        },
            {{ 0xB2,    0, ExtendedKey }, MediaStop        },
            {{ 0xB3,    0, ExtendedKey }, MediaPlayPause   },
            {{ 0xB5,    0, ExtendedKey }, MediaSelect      },
            {{ 0xA6,    0, ExtendedKey }, BrowserBack      },
            {{ 0xA7,    0, ExtendedKey }, BrowserForward   },
            {{ 0xA8,    0, ExtendedKey }, BrowserRefresh   },
            {{ 0xA9,    0, ExtendedKey }, BrowserStop      },
            {{ 0xAA,    0, ExtendedKey }, BrowserSearch    },
            {{ 0xAB,    0, ExtendedKey }, BrowserFavorites },
            {{ 0xAC,    0, ExtendedKey }, BrowserHome      },
        };
    }

    struct foci
    {
        using sptr = netxs::sptr<base>;

        id_t   id{}; // foci: Gear id.
        si32 solo{}; // foci: Exclusive focus request.
        bool flip{}; // foci: Toggle focus request.
        bool skip{}; // foci: Ignore focusable object, just activate it.
        sptr item{}; // foci: Next focused item.
        ui32 deep{}; // foci: Counter for debug.
        time guid{}; // foci: Originating environment ID.
    };

    // console: Mouse tracker.
    struct mouse
    {
        using mouse_event = netxs::events::userland::hids::mouse;
        using click = mouse_event::button::click;

        enum buttons
        {
            left      = click::left     .index(),
            right     = click::right    .index(),
            middle    = click::middle   .index(),
            wheel     = click::wheel    .index(),
            win       = click::win      .index(),
            leftright = click::leftright.index(),
            numofbuttons,
        };

        struct stat
        {
            enum : ui32
            {
                ok,
                halt,
                die,
            };
        };

        struct knob_t
        {
            bool pressed; // knod: Button pressed state.
            bool dragged; // knod: The button is in a dragging state.
            bool blocked; // knod: The button is blocked (leftright disables left and right).
        };
        struct hist_t // Timer for successive double-clicks, e.g. triple-clicks.
        {
            time fired; // hist: .
            twod coord; // hist: .
            si32 count; // hist: .
        };

        using hist = std::array<hist_t, numofbuttons>;
        using knob = std::array<knob_t, numofbuttons>;
        using tail = netxs::datetime::tail<twod>;

        static constexpr auto dragstrt = mouse_event::button::drag::start:: any.group<numofbuttons>();
        static constexpr auto dragpull = mouse_event::button::drag::pull::  any.group<numofbuttons>();
        static constexpr auto dragcncl = mouse_event::button::drag::cancel::any.group<numofbuttons>();
        static constexpr auto dragstop = mouse_event::button::drag::stop::  any.group<numofbuttons>();
        static constexpr auto released = mouse_event::button::up::          any.group<numofbuttons>();
        static constexpr auto pushdown = mouse_event::button::down::        any.group<numofbuttons>();
        static constexpr auto sglclick = mouse_event::button::click::       any.group<numofbuttons>();
        static constexpr auto dblclick = mouse_event::button::dblclick::    any.group<numofbuttons>();
        static constexpr auto tplclick = mouse_event::button::tplclick::    any.group<numofbuttons>();
        static constexpr auto scrollup = mouse_event::scroll::up.id;
        static constexpr auto scrolldn = mouse_event::scroll::down.id;
        static constexpr auto movement = mouse_event::move.id;
        static constexpr auto noactive = si32{ -1 };

        twod prime = {}; // mouse: System mouse cursor coordinates.
        twod coord = {}; // mouse: Relative mouse cursor coordinates.
        tail delta = {}; // mouse: History of mouse movements for a specified period of time.
        bool reach = {}; // mouse: Has the event tree relay reached the mouse event target.
        bool nodbl = {}; // mouse: Whether single click event processed (to prevent double clicks).
        bool scrll = {}; // mouse: Vertical scrolling.
        bool hzwhl = {}; // mouse: Horizontal scrolling.
        si32 whldt = {}; // mouse: Scroll delta.
        si32 locks = {}; // mouse: State of the captured buttons (bit field).
        si32 index = {}; // mouse: Index of the active button. -1 if the buttons are not involed.
        id_t swift = {}; // mouse: Delegate's ID of the current mouse owner.
        id_t hover = {}; // mouse: Hover control ID.
        id_t start = {}; // mouse: Initiator control ID.
        hint cause = {}; // mouse: Current event id.
        hist stamp = {}; // mouse: Recorded intervals between successive button presses to track double-clicks.
        span delay = {}; // mouse: Double-click threshold.
        knob bttns = {}; // mouse: Extended state of mouse buttons.
        sysmouse m = {}; // mouse: Device state.
        sysmouse s = {}; // mouse: Previous device state.

        // mouse: Forward the extended mouse event.
        virtual void fire(hint cause, si32 index = mouse::noactive) = 0;
        // mouse: Try to forward the mouse event intact.
        virtual bool fire_fast() = 0;

        // mouse: Forward the specified button event.
        template<class T>
        void fire(T const& event_subset, si32 index)
        {
            fire(event_subset[index], index);
        }
        // mouse: Pack the button state into a bitset.
        auto take_button_state()
        {
            auto bitstat = ui32{};
            auto pressed = 1 << 0;
            auto dragged = 1 << 1;
            auto blocked = 1 << 2;
            for (auto& b : bttns)
            {
                if (b.pressed) bitstat |= pressed;
                if (b.dragged) bitstat |= dragged;
                if (b.blocked) bitstat |= blocked;
                pressed <<= 3;
                dragged <<= 3;
                blocked <<= 3;
            }
            return bitstat;
        }
        // mouse: Load the button state from a bitset.
        auto load_button_state(ui32 bitstat)
        {
            auto pressed = 1 << 0;
            auto dragged = 1 << 1;
            auto blocked = 1 << 2;
            for (auto& b : bttns)
            {
                b.pressed = bitstat & pressed;
                b.dragged = bitstat & dragged;
                b.blocked = bitstat & blocked;
                pressed <<= 3;
                dragged <<= 3;
                blocked <<= 3;
            }
        }
        // mouse: Return a kinetic animator.
        template<class Law>
        auto fader(span spell)
        {
            //todo use current item's type: Law<twod>
            return delta.fader<Law>(spell);
        }
        // mouse: Extended mouse event generation.
        void update(sysmouse& m0)
        {
            auto m_buttons = std::bitset<8>(m0.buttons);
            // Interpret button combinations.
            //todo possible bug in Apple's Terminal - it does not return the second release
            //                                        in case when two buttons are pressed.
            m_buttons[leftright] = (bttns[leftright].pressed && (m_buttons[left] || m_buttons[right]))
                                                             || (m_buttons[left] && m_buttons[right]);
            m0.buttons = static_cast<ui32>(m_buttons.to_ulong());
            m.set(m0);
            auto busy = captured();
            if (busy && fire_fast()) //todo fire_fast on mods press
            {
                delta.set(m.coordxy - prime);
                coord = m.coordxy;
                prime = m.coordxy;
                fire(movement); // Update mouse enter/leave state.
                return;
            }
            if (m_buttons[leftright]) // Cancel left and right dragging if it is.
            {
                if (bttns[left].dragged)
                {
                    bttns[left].dragged = faux;
                    fire(dragcncl, left);
                }
                if (bttns[right].dragged)
                {
                    bttns[right].dragged = faux;
                    fire(dragcncl, right);
                }
                m_buttons[left ] = faux;
                m_buttons[right] = faux;
                m.buttons = static_cast<ui32>(m_buttons.to_ulong());
            }

            // Suppress left and right to avoid single button tracking (click, pull, etc)
            bttns[left ].blocked = m_buttons[leftright] || bttns[leftright].pressed;
            bttns[right].blocked = bttns[left].blocked;

            if (m.coordxy != prime)
            {
                delta.set(m.coordxy - prime);
                auto active = si32{};
                auto genptr = std::begin(bttns);
                for (auto i = 0; i < numofbuttons; i++)
                {
                    auto& genbtn = *genptr++;
                    if (genbtn.pressed && !genbtn.blocked)
                    {
                        if (!genbtn.dragged)
                        {
                            fire(dragstrt, i);
                            genbtn.dragged = true;
                        }
                        active |= 1 << i;
                    }
                }
                coord = m.coordxy;
                prime = m.coordxy;
                for (auto i = 0; active; ++i)
                {
                    if (active & 0x1)
                    {
                        fire(dragpull, i);
                    }
                    active >>= 1;
                }
                fire(movement);
            }

            if (!busy && fire_fast()) return;

            auto genptr = std::begin(bttns);
            for (auto i = 0; i < numofbuttons; i++)
            {
                auto& genbtn = *genptr++;
                auto  sysbtn = m_buttons[i];
                if (genbtn.pressed != sysbtn)
                {
                    genbtn.pressed = sysbtn;
                    if (genbtn.pressed)
                    {
                        fire(pushdown, i);
                    }
                    else
                    {
                        if (genbtn.dragged)
                        {
                            genbtn.dragged = faux;
                            fire(dragstop, i);
                        }
                        else
                        {
                            if (!genbtn.blocked)
                            {
                                fire(sglclick, i);
                            }
                            if (!nodbl)
                            {
                                // Fire a double/triple-click if delay is not expired
                                // and the mouse is at the same position.
                                auto& s = stamp[i];
                                auto fired = datetime::now();
                                if (fired - s.fired < delay && s.coord == coord)
                                {
                                    if (!genbtn.blocked)
                                    {
                                        if (s.count == 1)
                                        {
                                            fire(dblclick, i);
                                            s.fired = fired;
                                            s.count = 2;
                                        }
                                        else if (s.count == 2)
                                        {
                                            fire(tplclick, i);
                                            s.fired = {};
                                            s.count = {};
                                        }
                                    }
                                }
                                else
                                {
                                    s.fired = fired;
                                    s.coord = coord;
                                    s.count = 1;
                                }
                            }
                        }
                        fire(released, i);
                    }
                }
            }

            coord = m.coordxy;
            if (m.wheeled || m.hzwheel)
            {
                scrll = m.wheeled;
                hzwhl = m.hzwheel;
                whldt = m.wheeldt;
                fire(m.wheeldt > 0 ? scrollup : scrolldn);
                m.wheeled = {}; // Clear one-shot events.
                m.hzwheel = {};
                m.wheeldt = {};
                m.doubled = {};
            }
        }
        // mouse: Initiator of visual tree informing about mouse enters/leaves.
        template<bool Entered>
        bool direct(id_t asker)
        {
            if constexpr (Entered)
            {
                if (!start) // The first one to track the mouse will assign itslef to be a root.
                {
                    start = asker;
                    return true;
                }
                return start == asker;
            }
            else
            {
                if (start == asker)
                {
                    start = 0;
                    return true;
                }
                return faux;
            }
        }
        // mouse: Is the mouse seized/captured by asker?
        bool captured(id_t asker) const
        {
            return swift == asker;
        }
        // mouse: Is the mouse seized/captured?
        bool captured() const
        {
            return swift;
        }
        // mouse: Seize specified mouse control.
        bool capture(id_t asker)
        {
            if (!swift || swift == asker)
            {
                swift = asker;
                if (index != mouse::noactive) locks |= 1 << index;
                return true;
            }
            return faux;
        }
        // mouse: Release specified mouse control.
        void setfree(bool forced = true)
        {
            forced |= index == mouse::noactive;
            locks = forced ? 0
                           : locks & ~(1 << index);
            if (!locks) swift = {};
        }
    };

    // console: Keybd tracker.
    struct keybd
    {
        text cluster = {};
        bool pressed = {};
        ui16 imitate = {};
        ui16 virtcod = {};
        ui16 scancod = {};
        hint cause = netxs::events::userland::hids::keybd::data::post.id;
        text keystrokes;
        bool handled = {};

        void update(syskeybd& k)
        {
            pressed = k.pressed;
            imitate = k.imitate;
            virtcod = k.virtcod;
            scancod = k.scancod;
            cluster = k.cluster;
            handled = k.handled;
            fire_keybd();
        }

        virtual void fire_keybd() = 0;
    };

    // console: Human interface device controller.
    struct hids
        : public mouse,
          public keybd,
          public bell
    {
        using events = netxs::events::userland::hids;
        using list = std::list<wptr<base>>;

        id_t        relay; // hids: Mouse routing call stack initiator.
        core const& idmap; // hids: Area of the main form. Primary or relative region of the mouse coverage.
        bool        alive; // hids: Whether event processing is complete.
        span&       tooltip_timeout; // hids: .
        bool&       simple_instance; // hids: .

        //todo unify
        text        tooltip_data; // hids: Tooltip data.
        ui32        digest = 0; // hids: Tooltip digest.
        testy<ui32> digest_tracker = 0; // hids: Tooltip changes tracker.
        ui32        tooltip_digest = 0; // hids: Tooltip digest.
        time        tooltip_time = {}; // hids: The moment to show tooltip.
        bool        tooltip_show = faux; // hids: Show tooltip or not.
        bool        tooltip_stop = true; // hids: Disable tooltip.
        testy<twod> tooltip_coor = {}; // hids: .

        base& owner;
        ui32 ctlstate = 0;
        ui32 winctrl = {}; // MS Windows specific.

        //todo unify
        rect slot; // slot for pro::maker and e2::createby.
        bool slot_forced = faux; // .

        //todo unify
        bool disabled = faux;
        si32 countdown = 0;

        clip clip_rawdata{}; // hids: Clipboard data.
        face clip_preview{}; // hids: Clipboard preview render.
        bool not_directvt{}; // hids: Is it the top level gear (not directvt).
        bool clip_printed{}; // hids: Preview output tracker.
        si32& clip_shadow_size;
        cell& clip_preview_clrs;
        byte& clip_preview_alfa;

        id_t user_index; // hids: User/Device image/icon index.

        template<class T>
        hids(T& props, bool not_directvt, base& owner, core const& idmap)
            : relay{ 0 },
            owner{ owner },
            idmap{ idmap },
            alive{ faux },
            tooltip_timeout{   props.tooltip_timeout },
            simple_instance{   props.simple },
            clip_shadow_size{  props.clip_preview_glow },
            clip_preview_clrs{ props.clip_preview_clrs },
            clip_preview_alfa{ props.clip_preview_alfa },
            not_directvt{ not_directvt }
        {
            mouse::prime = dot_mx;
            mouse::coord = dot_mx;
            mouse::delay = props.dblclick_timeout;
            SIGNAL(tier::general, events::device::user::login, user_index);
        }
        ~hids()
        {
            auto lock = netxs::events::sync{};
            mouse_leave(mouse::hover, mouse::start);
            SIGNAL(tier::general, events::halt, *this);
            SIGNAL(tier::general, events::die, *this);
            SIGNAL(tier::general, events::device::user::logout, user_index);
        }

        // hids: Whether event processing is complete.
        operator bool() const
        {
            return alive;
        }

        auto clear_clip_data()
        {
            auto not_empty = !!clip_rawdata.utf8.size();
            clip_rawdata.clear();
            owner.SIGNAL(tier::release, hids::events::clipbrd::set, *this);
            if (not_directvt)
            {
                clip_preview.size(clip_rawdata.size);
            }
            return not_empty;
        }
        void set_clip_data(clip const& data, bool forward = true)
        {
            clip_rawdata.set(data);
            if (not_directvt)
            {
                auto draw_shadow = [&](auto& block, auto size)
                {
                    clip_preview.mark(cell{});
                    clip_preview.wipe();
                    clip_preview.size(dot_21 * size * 2 + clip_rawdata.size);
                    auto full = rect{ dot_21 * size + dot_21, clip_rawdata.size };
                    while (size--)
                    {
                        clip_preview.reset();
                        clip_preview.full(full);
                        clip_preview.output(block, cell::shaders::color(cell{}.bgc(0).fgc(0).alpha(0x60)));
                        clip_preview.blur(1, [&](cell& c) { c.fgc(c.bgc()).txt(""); });
                    }
                    full.coor -= dot_21;
                    clip_preview.reset();
                    clip_preview.full(full);
                };
                if (clip_rawdata.kind == clip::safetext)
                {
                    auto blank = ansi::bgc(0x7Fffffff).fgc(0xFF000000).add(" Protected Data "); //todo unify (i18n)
                    auto block = page{ blank };
                    clip_rawdata.size = block.current().size();
                    if (clip_shadow_size) draw_shadow(block, clip_shadow_size);
                    else
                    {
                        clip_preview.size(clip_rawdata.size);
                        clip_preview.wipe();
                    }
                    clip_preview.output(block);
                }
                else
                {
                    auto block = page{ clip_rawdata.utf8 };
                    if (clip_shadow_size) draw_shadow(block, clip_shadow_size);
                    else
                    {
                        clip_preview.size(clip_rawdata.size);
                        clip_preview.wipe();
                    }
                    clip_preview.mark(cell{});
                    if (clip_rawdata.kind == clip::textonly) clip_preview.output(block, cell::shaders::color(  clip_preview_clrs));
                    else                                     clip_preview.output(block, cell::shaders::xlucent(clip_preview_alfa));
                }
            }
            if (forward) owner.SIGNAL(tier::release, hids::events::clipbrd::set, *this);
            mouse::delta.set(); // Update time stamp.
        }
        auto get_clip_data()
        {
            auto data = clip{};
            owner.SIGNAL(tier::release, hids::events::clipbrd::get, *this);
            if (not_directvt) data.utf8 = clip_rawdata.utf8;
            else              data.utf8 = std::move(clip_rawdata.utf8);
            data.kind = clip_rawdata.kind;
            return data;
        }

        auto tooltip_enabled(time const& now)
        {
            return !mouse::m.buttons
                && !disabled
                && !tooltip_stop
                && tooltip_show
                && tooltip_data.size()
                && tooltip_time < now
                && !captured();
        }
        void set_tooltip(view data, bool update = faux)
        {
            if (!update || data != tooltip_data)
            {
                tooltip_data = data;
                if (update)
                {
                    if (tooltip_digest == digest) // To show tooltip even after clicks.
                    {
                        ++tooltip_digest;
                    }
                    if (!tooltip_stop) tooltip_stop = data.empty();
                }
                else
                {
                    tooltip_show = faux;
                    tooltip_stop = data.empty();
                    tooltip_time = datetime::now() + tooltip_timeout;
                }
                digest++;
            }
        }
        auto is_tooltip_changed()
        {
            return digest_tracker(digest);
        }
        auto get_tooltip()
        {
            return std::pair{ qiew{ tooltip_data }, tooltip_digest == digest };
        }
        void tooltip_recalc(hint deed)
        {
            if (deed == hids::events::mouse::move.id)
            {
                if (tooltip_coor(mouse::coord) || (tooltip_show && tooltip_digest != digest)) // Do nothing on shuffle.
                {
                    if (tooltip_show && tooltip_digest == digest) // Drop tooltip if moved.
                    {
                        tooltip_stop = true;
                    }
                    else
                    {
                        tooltip_time = datetime::now() + tooltip_timeout;
                    }
                }
            }
            else if (deed == hids::events::mouse::scroll::up.id
                  || deed == hids::events::mouse::scroll::down.id) // Drop tooltip away.
            {
                tooltip_stop = true;
            }
            else
            {
                if (tooltip_show == faux) // Reset timer to begin,
                {
                    tooltip_time = datetime::now() + tooltip_timeout;
                }
            }
        }
        auto tooltip_check(time now) // Called every timer tick.
        {
            if (tooltip_stop) return faux;
            if (!tooltip_show
             &&  tooltip_time < now
             && !captured())
            {
                tooltip_show = true;
                if (tooltip_digest != digest)
                {
                    tooltip_digest = digest;
                    return true;
                }
            }
            else if (captured())
            {
                if (!tooltip_show) // If not shown then drop tooltip.
                {
                    tooltip_stop = true;
                }
                //else if (tooltip_time < now) // Give time to update tooltip text after mouse button release.
                //{
                //    tooltip_time = now + 100ms;
                //}
            }
            return faux;
        }

        void replay(hint cause, twod const& coord, twod const& delta, ui32 button_state)
        {
            static constexpr auto mask = netxs::events::level_mask(hids::events::mouse::button::any.id);
            static constexpr auto base = mask & hids::events::mouse::button::any.id;
            alive = true;
            mouse::coord = coord;
            mouse::cause = (cause & ~mask) | base; // Remove the dependency on the event tree root.
            mouse::delta.set(delta);
            mouse::load_button_state(button_state);
        }

        enum modifiers : ui32
        {
            LShift   = 1 <<  0, // ⇧ Shift, Left  Shift
            RShift   = 1 <<  1, //          Right Shift
            LAlt     = 1 <<  2, // ⎇ Alt, ⌥ Option,   Left  Alt
            RAlt     = 1 <<  3, // ⇮ AltGr, Alt Graph, Right Alt
            LCtrl    = 1 <<  4, // ⌃ Ctrl, Left  Ctrl
            RCtrl    = 1 <<  5, //         Right Ctrl
            Meta     = 1 <<  6, // ◆ Meta, ⊞ Win, ⌘ Cmd (Apple key), ❖ Super
            Fn       = 1 <<  7, //
            CapsLock = 1 <<  8, // ⇪ Caps Lock
            NumLock  = 1 <<  9, // ⇭ Num Lock
            ScrlLock = 1 << 10, // ⇳ Scroll Lock (⤓)
            anyCtrl  = LCtrl  | RCtrl,
            anyAlt   = LAlt   | RAlt,
            anyShift = LShift | RShift,
        };

        auto meta(ui32 ctl_key = -1) { return hids::ctlstate & ctl_key; }
        auto kbmod()
        {
            return meta(hids::anyCtrl | hids::anyAlt | hids::anyShift);
        }

        // hids: Stop handeling this event.
        void dismiss(bool set_nodbl = faux)
        {
            alive = faux;
            if (set_nodbl) nodbl = true;
        }
        void set_handled(bool b)
        {
            handled = b;
        }

        void take(sysmouse& m)
        {
            ctlstate = m.ctlstat;
            winctrl  = m.winctrl;
            disabled = faux;
            mouse::update(m);
        }
        void take(syskeybd& k)
        {
            tooltip_stop = true;
            ctlstate = k.ctlstat;
            winctrl  = k.winctrl;
            keybd::update(k);
        }
        void take(sysfocus& f)
        {
            tooltip_stop = true;
            //if constexpr (debugmode) log(prompt::foci, "Take focus hid:", id, " state:", f.state ? "on":"off");
            //todo focus<->seed
            if (f.state) owner.SIGNAL(tier::release, hids::events::focus::set, *this);
            else         owner.SIGNAL(tier::release, hids::events::focus::off, *this);
        }

        auto& area() const { return idmap.area(); }

        template<tier Tier>
        void pass(sptr<base> object, twod const& offset, bool relative = faux)
        {
            if (object)
            {
                auto temp = mouse::coord;
                mouse::coord += offset;
                if (relative)
                {
                    object->global(coord);
                }
                object->bell::template signal<Tier>(mouse::cause, *this);
                mouse::coord = temp;
            }
        }
        void mouse_leave(id_t last_id, id_t start_id)
        {
            if (last_id)
            {
                if (auto last = bell::getref(last_id))
                {
                    auto start = mouse::start;
                    mouse::start = start_id;
                    last->SIGNAL(tier::release, events::notify::mouse::leave, *this);
                    mouse::start = start;
                }
                else log(prompt::hids, "Error condition: Clients count is broken, dangling ", last_id);
            }
        }
        void redirect_mouse_focus(base& boss)
        {
            if (mouse::hover != boss.id) // The mouse cursor is over the new object.
            {
                if (tooltip_data.size())
                {
                    digest++;
                    tooltip_data.clear();
                    tooltip_stop = true;
                }

                // Firing the leave event right after the enter allows us
                // to avoid flickering the parent object state when focus
                // acquired by children.
                auto start_l = mouse::start;
                mouse::start = 0; // The first one to track the mouse will assign itself by calling gear.direct<true>(id).
                boss.SIGNAL(tier::release, events::notify::mouse::enter, *this);
                mouse_leave(mouse::hover, start_l);
                mouse::hover = boss.id;
            }
        }
        void deactivate()
        {
            mouse::load_button_state(0);
            mouse::m.buttons = {};
            redirect_mouse_focus(owner);
            SIGNAL(tier::general, events::halt, *this);
            disabled = true;
        }
        void okay(base& boss)
        {
            if (boss.id == relay)
            {
                redirect_mouse_focus(boss);
                boss.bell::template signal<tier::release>(mouse::cause, *this);
            }
        }
        void fire(hint cause, si32 index = mouse::noactive)
        {
            if (disabled) return;

            alive = true;
            mouse::index = index;
            mouse::cause = cause;
            mouse::coord = mouse::prime;
            mouse::nodbl = faux;

            auto& offset = idmap.coor();
            if (mouse::swift)
            {
                auto next = bell::getref<base>(mouse::swift);
                if (next)
                {
                    redirect_mouse_focus(*next);
                    pass<tier::release>(next, offset, true);

                    if (alive && !captured()) // Pass unhandled event to the gate.
                    {
                        owner.bell::template signal<tier::release>(cause, *this);
                    }
                }
                else mouse::setfree();
            }
            else
            {
                if (!tooltip_stop) tooltip_recalc(cause);
                owner.bell::template signal<tier::preview>(cause, *this);

                if (!alive) return;

                auto next = idmap.link(mouse::coord);
                if (next != owner.id)
                {
                    relay = next;
                    pass<tier::preview>(bell::getref<base>(next), offset, true);
                    relay = 0;

                    if (!alive) return;
                }

                owner.bell::template signal<tier::release>(cause, *this); // Pass unhandled event to the gate.
            }
        }
        bool fire_fast()
        {
            if (disabled) return true;
            alive = true;
            auto next_id = mouse::swift ? mouse::swift
                                        : idmap.link(m.coordxy);
            if (next_id != owner.id)
            {
                if (auto next_ptr = bell::getref<base>(next_id))
                {
                    auto& next = *next_ptr;
                    auto  temp = m.coordxy;
                    m.coordxy += idmap.coor();
                    next.global(m.coordxy);
                    next.SIGNAL(tier::release, events::device::mouse::on, *this);
                    m.coordxy = temp;
                    if (!alive) // Clear one-shot events on success.
                    {
                        m.wheeled = {};
                        m.wheeldt = {};
                        m.hzwheel = {};
                        m.doubled = {};
                    }
                }
                else if (mouse::swift == next_id) mouse::setfree();
            }
            if (s.changed != m.changed) s = m;
            return !alive;
        }
        void fire_keybd()
        {
            alive = true;
            keystrokes = interpret();
            owner.bell::template signal<tier::preview>(keybd::cause, *this);
        }
        text interpret()
        {
            auto textline = text{};
            auto ctrl = [&](auto pure, auto f, auto suffix)
            {
                textline = "\033";
                if (ctlstate & hids::anyShift)
                {
                    textline += f;
                    textline += ";2";
                }
                else if (ctlstate & hids::anyAlt)
                {
                    textline += f;
                    textline += ";3";
                }
                else if (ctlstate & hids::anyCtrl)
                {
                    textline += f;
                    textline += ";5";
                }
                else textline += pure;
                textline += suffix;
            };
            if (pressed)
            {
                switch (virtcod)
                {
                    //todo Ctrl+Space
                    //     Ctrl+Backspace
                    //     Alt+0..9
                    //     Ctrl/Shift+Enter
                    case key::vk::Backspace: textline = "\177"; break;
                    case key::vk::Tab:
                        textline = ctlstate & hids::anyShift ? "\033[Z"
                                                             : "\t";
                        break;
                    case key::vk::PageUp:     ctrl("[5",  "[5",  "~"); break;
                    case key::vk::PageDown:   ctrl("[6",  "[6",  "~"); break;
                    case key::vk::Insert:     ctrl("[2",  "[2",  "~"); break;
                    case key::vk::Delete:     ctrl("[3",  "[3",  "~"); break;
                    case key::vk::End:        ctrl("[",   "[1",  "F"); break;
                    case key::vk::Home:       ctrl("[",   "[1",  "H"); break;
                    case key::vk::UpArrow:    ctrl("[",   "[1",  "A"); break;
                    case key::vk::DownArrow:  ctrl("[",   "[1",  "B"); break;
                    case key::vk::RightArrow: ctrl("[",   "[1",  "C"); break;
                    case key::vk::LeftArrow:  ctrl("[",   "[1",  "D"); break;
                    case key::vk::F1:         ctrl("O",   "[1",  "P"); break;
                    case key::vk::F2:         ctrl("O",   "[1",  "Q"); break;
                    case key::vk::F3:         ctrl("O",   "[1",  "R"); break;
                    case key::vk::F4:         ctrl("O",   "[1",  "S"); break;
                    case key::vk::F5:         ctrl("[15", "[15", "~"); break;
                    case key::vk::F6:         ctrl("[17", "[17", "~"); break;
                    case key::vk::F7:         ctrl("[18", "[18", "~"); break;
                    case key::vk::F8:         ctrl("[19", "[19", "~"); break;
                    case key::vk::F9:         ctrl("[20", "[20", "~"); break;
                    case key::vk::F10:        ctrl("[21", "[21", "~"); break;
                    case key::vk::F11:        ctrl("[23", "[23", "~"); break;
                    case key::vk::F12:        ctrl("[24", "[24", "~"); break;
                    case key::vk::F13:        ctrl("[25", "[25", "~"); break;
                    case key::vk::F14:        ctrl("[26", "[26", "~"); break;
                    case key::vk::F15:        ctrl("[28", "[28", "~"); break;
                    case key::vk::F16:        ctrl("[29", "[29", "~"); break;
                    case key::vk::F17:        ctrl("[31", "[31", "~"); break;
                    case key::vk::F18:        ctrl("[32", "[32", "~"); break;
                    case key::vk::F19:        ctrl("[33", "[33", "~"); break;
                    case key::vk::F20:        ctrl("[34", "[34", "~"); break;
                    case key::vk::F21:        ctrl("[35", "[35", "~"); break;
                    case key::vk::F22:        ctrl("[36", "[36", "~"); break;
                    case key::vk::F23:        ctrl("[37", "[37", "~"); break;
                    case key::vk::F24:        ctrl("[38", "[38", "~"); break;
                    default:
                        textline = cluster;
                        break;
                }
            }
            return textline;
        }
    };
}