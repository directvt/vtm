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
            EVENT_XS( clipbrd, input::hids ), // release/request: Set/get clipboard data.
            GROUP_XS( keybd  , input::hids ),
            GROUP_XS( mouse  , input::hids ),
            GROUP_XS( focus  , input::hids ), // release::global: Notify about the focus got/lost.
            GROUP_XS( notify , input::hids ), // Form events that should be propagated down to the visual branch.
            GROUP_XS( device , input::hids ), // Primary device event group for forwarding purposes.

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
    using netxs::ui::base;
    using netxs::ui::face;
    using netxs::ui::page;

    namespace key
    {
        static constexpr auto ExtendedKey = 0x0100;
        static constexpr auto NumLockMode = 0x0020;

        struct map
        {
            sz_t hash; // map: Key hash.

            static auto& mask()
            {
                static auto m = std::vector<si32>(256);
                return m;
            }
            static auto& mask(si32 vk)
            {
                return mask()[vk & 0xFF];
            }
            static auto& name()
            {
                static auto n = std::vector<view>(256);
                return n;
            }
            static auto& name(si32 keycode)
            {
                return name()[keycode];
            }

            map(si32 vk, si32 sc, ui32 cs)
                : hash{ static_cast<sz_t>(mask(vk) & (vk | (sc << 8) | (cs << 16))) }
            { }
            map(si32 vk, si32 sc, ui32 cs, si32 keymask, view keyname, si32 id)
            {
                mask(vk) = keymask;
                name(id) = keyname;
                hash = static_cast<sz_t>(keymask & (vk | (sc << 8) | (cs << 16)));
            }

            bool operator == (map const& m) const = default;
            struct hashproc
            {
                auto operator()(map const& m) const
                {
                    return m.hash;
                }
            };
        };

        #define key_list \
            /*Id   Vkey  Scan    CtrlState          Mask  Name            */\
            X(0,      0,    0,           0, 0x0000'00'FF, undef            )\
            X(2,   0x11, 0x1D,           0, 0x0100'00'FF, LeftCtrl         )\
            X(4,   0x11, 0x1D, ExtendedKey, 0x0100'00'FF, RightCtrl        )\
            X(6,   0x12, 0x38,           0, 0x0100'00'FF, LeftAlt          )\
            X(8,   0x12, 0x38, ExtendedKey, 0x0100'00'FF, RightAlt         )\
            X(10,  0x10, 0x2A,           0, 0x0000'FF'FF, LeftShift        )\
            X(11,  0x10, 0x36,           0, 0x0000'FF'FF, RightShift       )\
            X(12,  0x5B, 0x5B, ExtendedKey, 0x0000'00'FF, LeftWin          )\
            X(13,  0x5D, 0x5D, ExtendedKey, 0x0000'00'FF, Apps             )\
            X(14,  0x5C, 0x5C, ExtendedKey, 0x0000'00'FF, RightWin         )\
            X(15,  0x90, 0x45,           0, 0x0000'00'FF, NumLock          )\
            X(16,  0x14, 0x3A,           0, 0x0000'00'FF, CapsLock         )\
            X(17,  0x91, 0x45,           0, 0x0000'00'FF, ScrollLock       )\
            X(18,  0x1B, 0x01,           0, 0x0000'00'FF, Esc              )\
            X(20,  0x20, 0x39,           0, 0x0000'00'FF, Space            )\
            X(22,  0x08, 0x0E,           0, 0x0000'00'FF, Backspace        )\
            X(24,  0x09, 0x0F,           0, 0x0000'00'FF, Tab              )\
            X(26,  0x03, 0x46,           0, 0x0000'FF'FF, Break            )\
            X(28,  0x13, 0x45,           0, 0x0000'FF'FF, Pause            )\
            X(30,  0x29,    0,           0, 0x0000'00'FF, Select           )\
            X(32,  0x2C, 0x54,           0, 0x0000'FF'FF, SysRq            )\
            X(34,  0x2C, 0x37, ExtendedKey, 0x0100'FF'FF, PrintScreen      )\
            X(36,  0x0D, 0x1C,           0, 0x0100'00'FF, Enter            )\
            X(37,  0x0D, 0x1C, ExtendedKey, 0x0100'00'FF, NumpadEnter      )\
            X(38,  0x21, 0x49, ExtendedKey, 0x0100'00'FF, PageUp           )\
            X(39,  0x21, 0x49,           0, 0x0100'00'FF, NumpadPageUp     )\
            X(40,  0x22, 0x51, ExtendedKey, 0x0100'00'FF, PageDown         )\
            X(41,  0x22, 0x51,           0, 0x0100'00'FF, NumpadPageDown   )\
            X(42,  0x23, 0x4F, ExtendedKey, 0x0100'00'FF, End              )\
            X(43,  0x23, 0x4F,           0, 0x0100'00'FF, NumpadEnd        )\
            X(44,  0x24, 0x47, ExtendedKey, 0x0100'00'FF, Home             )\
            X(45,  0x24, 0x47,           0, 0x0100'00'FF, NumpadHome       )\
            X(46,  0x25, 0x4B, ExtendedKey, 0x0100'00'FF, LeftArrow        )\
            X(47,  0x25, 0x4B,           0, 0x0100'00'FF, NumpadLeftArrow  )\
            X(48,  0x26, 0x48, ExtendedKey, 0x0100'00'FF, UpArrow          )\
            X(49,  0x26, 0x48,           0, 0x0100'00'FF, NumpadUpArrow    )\
            X(50,  0x27, 0x4D, ExtendedKey, 0x0100'00'FF, RightArrow       )\
            X(51,  0x27, 0x4D,           0, 0x0100'00'FF, NumpadRightArrow )\
            X(52,  0x28, 0x50, ExtendedKey, 0x0100'00'FF, DownArrow        )\
            X(53,  0x28, 0x50,           0, 0x0100'00'FF, NumpadDownArrow  )\
            X(54,  0x30, 0x0B,           0, 0x0000'FF'FF, Key0             )\
            X(55,  0x60, 0x52, NumLockMode, 0x0000'FF'FF, Numpad0          )\
            X(56,  0x31, 0x02,           0, 0x0000'FF'FF, Key1             )\
            X(57,  0x61, 0x4F, NumLockMode, 0x0000'FF'FF, Numpad1          )\
            X(58,  0x32, 0x03,           0, 0x0000'FF'FF, Key2             )\
            X(59,  0x62, 0x50, NumLockMode, 0x0000'FF'FF, Numpad2          )\
            X(60,  0x33, 0x04,           0, 0x0000'FF'FF, Key3             )\
            X(61,  0x63, 0x51, NumLockMode, 0x0000'FF'FF, Numpad3          )\
            X(62,  0x34, 0x05,           0, 0x0000'FF'FF, Key4             )\
            X(63,  0x64, 0x4B, NumLockMode, 0x0000'FF'FF, Numpad4          )\
            X(64,  0x35, 0x06,           0, 0x0000'FF'FF, Key5             )\
            X(65,  0x65, 0x4C, NumLockMode, 0x0000'FF'FF, Numpad5          )\
            X(66,  0x36, 0x07,           0, 0x0000'FF'FF, Key6             )\
            X(67,  0x66, 0x4D, NumLockMode, 0x0000'FF'FF, Numpad6          )\
            X(68,  0x37, 0x08,           0, 0x0000'FF'FF, Key7             )\
            X(69,  0x67, 0x47, NumLockMode, 0x0000'FF'FF, Numpad7          )\
            X(70,  0x38, 0x09,           0, 0x0000'FF'FF, Key8             )\
            X(71,  0x68, 0x48, NumLockMode, 0x0000'FF'FF, Numpad8          )\
            X(72,  0x39, 0x0A,           0, 0x0000'FF'FF, Key9             )\
            X(73,  0x69, 0x49, NumLockMode, 0x0000'FF'FF, Numpad9          )\
            X(74,  0x2D, 0x52, ExtendedKey, 0x0100'00'FF, Insert           )\
            X(75,  0x2D, 0x52,           0, 0x0100'00'FF, NumpadInsert     )\
            X(76,  0x2E, 0x53, ExtendedKey, 0x0100'00'FF, Delete           )\
            X(77,  0x2E, 0x55,           0, 0x0100'00'FF, NumpadDelete     )\
            X(78,  0x0C, 0x4C, ExtendedKey, 0x0100'00'FF, Clear            )\
            X(79,  0x0C, 0x4C,           0, 0x0100'00'FF, NumpadClear      )\
            X(80,  0x6A, 0x09,           0, 0x0000'FF'FF, Multiply         )\
            X(81,  0x6A, 0x37,           0, 0x0000'FF'FF, NumpadMultiply   )\
            X(82,  0x6B, 0x0D,           0, 0x0000'FF'FF, Plus             )\
            X(83,  0x6B, 0x4E,           0, 0x0000'FF'FF, NumpadPlus       )\
            X(84,  0x6C,    0,           0, 0x0020'00'FF, Separator        )\
            X(85,  0x6C,    0, NumLockMode, 0x0020'00'FF, NumpadSeparator  )\
            X(86,  0xBD, 0x0C,           0, 0x0000'00'FF, Minus            )\
            X(87,  0x6D, 0x4A,           0, 0x0000'00'FF, NumpadMinus      )\
            X(88,  0xBE, 0x34,           0, 0x0000'00'FF, Period           )\
            X(89,  0x6E, 0x53, NumLockMode, 0x0000'00'FF, NumpadDecimal    )\
            X(90,  0xBF, 0x35,           0, 0x0000'00'FF, Slash            )\
            X(91,  0x6F, 0x35, ExtendedKey, 0x0000'00'FF, NumpadSlash      )\
            X(92,  0xDC, 0x2B,           0, 0x0000'00'FF, BackSlash        )\
            X(94,  0xDB, 0x1A,           0, 0x0000'00'FF, OpenBracket      )\
            X(96,  0xDD, 0x1B,           0, 0x0000'00'FF, ClosedBracket    )\
            X(98,  0xBB, 0x0D,           0, 0x0000'00'FF, Equal            )\
            X(100, 0xC0, 0x29,           0, 0x0000'00'FF, BackQuote        )\
            X(102, 0xDE, 0x28,           0, 0x0000'00'FF, SingleQuote      )\
            X(104, 0xBC, 0x33,           0, 0x0000'00'FF, Comma            )\
            X(106, 0xBA, 0x27,           0, 0x0000'00'FF, Semicolon        )\
            X(108, 0x70, 0x3B,           0, 0x0000'00'FF, F1               )\
            X(110, 0x71, 0x3C,           0, 0x0000'00'FF, F2               )\
            X(112, 0x72, 0x3D,           0, 0x0000'00'FF, F3               )\
            X(114, 0x73, 0x3E,           0, 0x0000'00'FF, F4               )\
            X(116, 0x74, 0x3F,           0, 0x0000'00'FF, F5               )\
            X(118, 0x75, 0x40,           0, 0x0000'00'FF, F6               )\
            X(120, 0x76, 0x41,           0, 0x0000'00'FF, F7               )\
            X(122, 0x77, 0x42,           0, 0x0000'00'FF, F8               )\
            X(124, 0x78, 0x43,           0, 0x0000'00'FF, F9               )\
            X(126, 0x79, 0x44,           0, 0x0000'00'FF, F10              )\
            X(128, 0x7A, 0x57,           0, 0x0000'00'FF, F11              )\
            X(130, 0x7B, 0x5B,           0, 0x0000'00'FF, F12              )\
            X(132, 0x7C,    0,           0, 0x0000'00'FF, F13              )\
            X(134, 0x7D,    0,           0, 0x0000'00'FF, F14              )\
            X(136, 0x7E,    0,           0, 0x0000'00'FF, F15              )\
            X(138, 0x7F,    0,           0, 0x0000'00'FF, F16              )\
            X(140, 0x80,    0,           0, 0x0000'00'FF, F17              )\
            X(142, 0x81,    0,           0, 0x0000'00'FF, F18              )\
            X(144, 0x82,    0,           0, 0x0000'00'FF, F19              )\
            X(146, 0x83,    0,           0, 0x0000'00'FF, F20              )\
            X(148, 0x84,    0,           0, 0x0000'00'FF, F21              )\
            X(150, 0x85,    0,           0, 0x0000'00'FF, F22              )\
            X(152, 0x86,    0,           0, 0x0000'00'FF, F23              )\
            X(154, 0x87,    0,           0, 0x0000'00'FF, F24              )\
            X(156, 0x41,    0,           0, 0x0100'00'FF, KeyA             )\
            X(158, 0x42,    0,           0, 0x0100'00'FF, KeyB             )\
            X(160, 0x43,    0,           0, 0x0100'00'FF, KeyC             )\
            X(162, 0x44,    0,           0, 0x0100'00'FF, KeyD             )\
            X(164, 0x45,    0,           0, 0x0100'00'FF, KeyE             )\
            X(166, 0x46,    0,           0, 0x0100'00'FF, KeyF             )\
            X(168, 0x47,    0,           0, 0x0100'00'FF, KeyG             )\
            X(170, 0x48,    0,           0, 0x0100'00'FF, KeyH             )\
            X(172, 0x49,    0,           0, 0x0100'00'FF, KeyI             )\
            X(174, 0x4A,    0,           0, 0x0100'00'FF, KeyJ             )\
            X(176, 0x4B,    0,           0, 0x0100'00'FF, KeyK             )\
            X(178, 0x4C,    0,           0, 0x0100'00'FF, KeyL             )\
            X(180, 0x4D,    0,           0, 0x0100'00'FF, KeyM             )\
            X(182, 0x4E,    0,           0, 0x0100'00'FF, KeyN             )\
            X(184, 0x4F,    0,           0, 0x0100'00'FF, KeyO             )\
            X(186, 0x50,    0,           0, 0x0100'00'FF, KeyP             )\
            X(188, 0x51,    0,           0, 0x0100'00'FF, KeyQ             )\
            X(190, 0x52,    0,           0, 0x0100'00'FF, KeyR             )\
            X(192, 0x53,    0,           0, 0x0100'00'FF, KeyS             )\
            X(194, 0x54,    0,           0, 0x0100'00'FF, KeyT             )\
            X(196, 0x55,    0,           0, 0x0100'00'FF, KeyU             )\
            X(198, 0x56,    0,           0, 0x0100'00'FF, KeyV             )\
            X(200, 0x57,    0,           0, 0x0100'00'FF, KeyW             )\
            X(202, 0x58,    0,           0, 0x0100'00'FF, KeyX             )\
            X(204, 0x59,    0,           0, 0x0100'00'FF, KeyY             )\
            X(206, 0x5A,    0,           0, 0x0100'00'FF, KeyZ             )\
            X(208, 0x5F,    0, ExtendedKey, 0x0100'00'FF, Sleep            )\
            X(210, 0xB7,    0, ExtendedKey, 0x0100'00'FF, Calculator       )\
            X(212, 0x48,    0, ExtendedKey, 0x0100'00'FF, Mail             )\
            X(214, 0xAD,    0, ExtendedKey, 0x0100'00'FF, MediaVolMute     )\
            X(216, 0xAE,    0, ExtendedKey, 0x0100'00'FF, MediaVolDown     )\
            X(218, 0xAF,    0, ExtendedKey, 0x0100'00'FF, MediaVolUp       )\
            X(220, 0xB0,    0, ExtendedKey, 0x0100'00'FF, MediaNext        )\
            X(222, 0xB1,    0, ExtendedKey, 0x0100'00'FF, MediaPrev        )\
            X(224, 0xB2,    0, ExtendedKey, 0x0100'00'FF, MediaStop        )\
            X(226, 0xB3,    0, ExtendedKey, 0x0100'00'FF, MediaPlayPause   )\
            X(228, 0xB5,    0, ExtendedKey, 0x0100'00'FF, MediaSelect      )\
            X(230, 0xA6,    0, ExtendedKey, 0x0100'00'FF, BrowserBack      )\
            X(232, 0xA7,    0, ExtendedKey, 0x0100'00'FF, BrowserForward   )\
            X(234, 0xA8,    0, ExtendedKey, 0x0100'00'FF, BrowserRefresh   )\
            X(236, 0xA9,    0, ExtendedKey, 0x0100'00'FF, BrowserStop      )\
            X(238, 0xAA,    0, ExtendedKey, 0x0100'00'FF, BrowserSearch    )\
            X(240, 0xAB,    0, ExtendedKey, 0x0100'00'FF, BrowserFavorites )\
            X(242, 0xAC,    0, ExtendedKey, 0x0100'00'FF, BrowserHome      )

        #define X(KeyId, Vkey, Scan, CtrlState, Mask, Name) \
            static constexpr auto Name = KeyId;
            key_list
        #undef X

        static const auto keymap = std::unordered_map<map, si32, map::hashproc>
        {
            #define X(KeyId, Vkey, Scan, CtrlState, Mask, Name) \
                { map{ Vkey, Scan, CtrlState, Mask, #Name, Name }, Name, },
                key_list
            #undef X
        };

        #undef key_list

        template<class ...Args>
        auto xlat(Args&&... args)
        {
            auto iter = keymap.find(map{ args... });
            return iter != keymap.end() ? iter->second : input::key::undef;
        }
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

        enum mode
        {
            none = 0,
            bttn = 1 << 0,
            drag = 1 << 1,
            move = 1 << 2,
            over = 1 << 3,
            utf8 = 1 << 4,
            buttons_press = bttn,
            buttons_drags = bttn | drag,
            all_movements = bttn | drag | move,
            negative_args = bttn | drag | move | over,
        };
        enum prot
        {
            x11,
            sgr,
            w32,
        };
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
            auto modschanged = m.ctlstat != m0.ctlstat;
            m.set(m0);
            auto busy = captured();
            if (busy && fire_fast())
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

            if (m.coordxy != prime || modschanged)
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
        enum prot
        {
            vt,
            w32,
        };

        text cluster = {};
        bool extflag = {};
        bool pressed = {};
        ui16 virtcod = {};
        ui16 scancod = {};
        hint cause = netxs::events::userland::hids::keybd::data::post.id;
        text keystrokes;
        bool handled = {};
        si32 keycode = {};

        auto generic()
        {
            return keycode & -2;
        }
        void update(syskeybd& k)
        {
            extflag = k.extflag;
            pressed = k.pressed;
            virtcod = k.virtcod;
            scancod = k.scancod;
            cluster = k.cluster;
            handled = k.handled;
            keycode = k.keycode;
            fire_keybd();
        }

        virtual void fire_keybd() = 0;
    };

    // console: Focus tracker.
    struct focus
    {
        enum prot
        {
            w32,
            dec,
        };

        bool state = {};

        void update(sysfocus& f)
        {
            state = f.state;
            fire_focus();
        }

        virtual void fire_focus() = 0;
    };

    // console: Clipboard tracker.
    struct board
    {
        enum prot
        {
            w32,
            dec,
        };

        using clip = clipdata;

        clip cargo{}; // board: Clipboard data.
        face image{}; // board: Clipboard preview render.
        bool shown{}; // board: Preview output tracker.
        si32 ghost{}; // board: Preview shadow size.
        cell brush{}; // board: Preview default color.
        byte alpha{}; // board: Preview transparency.

        virtual void fire_board() = 0;

        static void set(clipdata& c, id_t gear_id, twod winsz, qiew utf8, si32 form)
        {
            auto size = dot_00;
            if (form == mime::disabled) // Try to parse utf8: mime/size_x/size_y;data
            {
                     if (utf8.starts_with(mime::tag::ansi)) { utf8.remove_prefix(mime::tag::ansi.length()); form = mime::ansitext; }
                else if (utf8.starts_with(mime::tag::text)) { utf8.remove_prefix(mime::tag::text.length()); form = mime::textonly; }
                else if (utf8.starts_with(mime::tag::rich)) { utf8.remove_prefix(mime::tag::rich.length()); form = mime::richtext; }
                else if (utf8.starts_with(mime::tag::html)) { utf8.remove_prefix(mime::tag::html.length()); form = mime::htmltext; }
                else if (utf8.starts_with(mime::tag::safe)) { utf8.remove_prefix(mime::tag::safe.length()); form = mime::safetext; }
                else
                {
                    if (auto pos = utf8.find(';'); pos != text::npos) utf8 = utf8.substr(pos + 1);
                    else                                              utf8 = {};
                }
                if (form == mime::disabled) form = mime::textonly;
                else
                {
                    if (utf8 && utf8.front() == '/') // Proceed preview size if present.
                    {
                        utf8.remove_prefix(1);
                        if (auto v = utf::to_int(utf8))
                        {
                            static constexpr auto max_value = twod{ 2000, 1000 }; //todo unify
                            size.x = v.value();
                            if (utf8)
                            {
                                utf8.remove_prefix(1);
                                if (auto v = utf::to_int(utf8)) size.y = v.value();
                            }
                            if (!size.x || !size.y) size = dot_00;
                            else                    size = std::clamp(size, dot_00, max_value);
                        }
                    }
                    if (utf8 && utf8.front() == ';') utf8.remove_prefix(1);
                    else                             utf8 = {}; // Unknown format.
                }
            }
            size = utf8.empty() ? dot_00
                 : size         ? size
                 : winsz        ? winsz
                                : twod{ 80,25 }; //todo make it configurable
            c.set(gear_id, datetime::now(), size, utf8.str(), form);
        }
        auto clear_clipboard()
        {
            auto not_empty = !!board::cargo.utf8.size();
            board::cargo.set(id_t{}, datetime::now(), dot_00, text{}, mime::ansitext);
            fire_board();
            return not_empty;
        }
        void set_clipboard(clipdata const& data)
        {
            board::cargo.set(data);
            fire_board();
        }
        void set_clipboard(twod size, qiew utf8, si32 form)
        {
            auto c = clip{};
            c.set(id_t{}, datetime::now(), size, utf8.str(), form);
            set_clipboard(c);
        }
        void update(sysboard& b) // Update clipboard preview.
        {
            auto draw_shadow = [&](auto& block, auto size)
            {
                board::image.mark(cell{});
                board::image.wipe();
                board::image.size(dot_21 * size * 2 + b.size);
                auto full = rect{ dot_21 * size + dot_21, b.size };
                while (size--)
                {
                    board::image.reset();
                    board::image.full(full);
                    board::image.output(block, cell::shaders::color(cell{}.bgc(0).fgc(0).alpha(0x60)));
                    board::image.blur(1, [&](cell& c) { c.fgc(c.bgc()).txt(""); });
                }
                full.coor -= dot_21;
                board::image.reset();
                board::image.full(full);
            };
            if (b.form == mime::safetext)
            {
                auto blank = ansi::bgc(0x7Fffffff).fgc(0xFF000000).add(" Protected Data "); //todo unify (i18n)
                auto block = page{ blank };
                if (ghost) draw_shadow(block, ghost);
                else
                {
                    board::image.size(block.current().size());
                    board::image.wipe();
                }
                board::image.output(block);
            }
            else
            {
                auto block = page{ b.utf8 };
                if (ghost) draw_shadow(block, ghost);
                else
                {
                    board::image.size(b.size);
                    board::image.wipe();
                }
                board::image.mark(cell{});
                if (b.form == mime::textonly) board::image.output(block, cell::shaders::color(  brush));
                else                          board::image.output(block, cell::shaders::xlucent(alpha));
            }
        }
    };

    // console: Human interface device controller.
    struct hids
        : public mouse,
          public keybd,
          public focus,
          public board,
          public bell
    {
        using events = netxs::events::userland::hids;
        using list = std::list<wptr<base>>;

        id_t        relay; // hids: Mouse routing call stack initiator.
        core const& idmap; // hids: Area of the main form. Primary or relative region of the mouse coverage.
        bool        alive; // hids: Whether event processing is complete.

        //todo unify
        span&       tooltip_timeout; // hids: .
        text        tooltip_data; // hids: Tooltip data.
        ui32        digest = 0; // hids: Tooltip digest.
        testy<ui32> digest_tracker = 0; // hids: Tooltip changes tracker.
        ui32        tooltip_digest = 0; // hids: Tooltip digest.
        time        tooltip_time = {}; // hids: The moment to show tooltip.
        bool        tooltip_show = faux; // hids: Show tooltip or not.
        bool        tooltip_stop = true; // hids: Disable tooltip.
        testy<twod> tooltip_coor = {}; // hids: .

        base& owner;
        ui32 ctlstate = {};

        //todo unify
        rect slot; // slot for pro::maker and e2::createby.
        bool slot_forced = faux; // .

        //todo unify
        bool disabled = faux;
        si32 countdown = 0;

        id_t user_index; // hids: User/Device image/icon index.

        template<class T>
        hids(T& props, base& owner, core const& idmap)
            : relay{ 0 },
            owner{ owner },
            idmap{ idmap },
            alive{ faux },
            tooltip_timeout{   props.tooltip_timeout }
        {
            board::ghost = props.clip_preview_glow;
            board::brush = props.clip_preview_clrs;
            board::alpha = props.clip_preview_alfa;
            mouse::delay = props.dblclick_timeout;
            mouse::prime = dot_mx;
            mouse::coord = dot_mx;
            SIGNAL(tier::general, events::device::user::login, user_index);
        }
       ~hids()
        {
            mouse_leave(mouse::hover, mouse::start);
            SIGNAL(tier::general, events::halt, *this);
            SIGNAL(tier::general, events::die, *this);
            SIGNAL(tier::general, events::device::user::logout, user_index);
        }

        // hids: Whether event processing is complete.
        operator bool () const
        {
            return alive;
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

        void replay(hint cause, twod const& coord, twod const& delta, ui32 button_state, ui32 ctlstat)
        {
            static constexpr auto mask = netxs::events::level_mask(hids::events::mouse::button::any.id);
            static constexpr auto base = mask & hids::events::mouse::button::any.id;
            alive = true;
            ctlstate = ctlstat;
            mouse::coord = coord;
            mouse::cause = (cause & ~mask) | base; // Remove the dependency on the event tree root.
            mouse::delta.set(delta);
            mouse::load_button_state(button_state);
        }

        enum modifiers : ui32
        {
            LCtrl    = 1 <<  0, // Left  ⌃ Ctrl
            RCtrl    = 1 <<  1, // Right ⌃ Ctrl
            LAlt     = 1 <<  2, // Left  ⎇ Alt, Left  ⌥ Option
            RAlt     = 1 <<  3, // Right ⎇ Alt, Right ⌥ Option, ⇮ AltGr
            LShift   = 1 <<  4, // Left  ⇧ Shift
            RShift   = 1 <<  5, // Right ⇧ Shift
            LWin     = 1 <<  6, // Left  ⊞ Win, ◆ Meta, ⌘ Cmd (Apple key), ❖ Super
            RWin     = 1 <<  7, // Right ⊞ Win, ◆ Meta, ⌘ Cmd (Apple key), ❖ Super
            NumLock  = 1 << 12, // ⇭ Num Lock
            CapsLock = 1 << 13, // ⇪ Caps Lock
            ScrlLock = 1 << 14, // ⇳ Scroll Lock (⤓)
            anyCtrl  = LCtrl  | RCtrl,
            anyAlt   = LAlt   | RAlt,
            anyShift = LShift | RShift,
            anyWin   = LWin   | RWin,
        };

        auto meta(ui32 ctl_key = -1) { return ctlstate & ctl_key; }
        auto kbmod()
        {
            return meta(hids::anyCtrl | hids::anyAlt | hids::anyShift | hids::anyWin);
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
            disabled = faux;
            ctlstate = m.ctlstat;
            mouse::update(m);
        }
        void take(syskeybd& k)
        {
            tooltip_stop = true;
            ctlstate = k.ctlstat;
            keybd::update(k);
        }
        void take(sysfocus& f)
        {
            tooltip_stop = true;
            focus::update(f);
        }
        auto take(sysboard& b)
        {
            board::update(b);
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
                else
                {
                    //todo revise
                    log("%%Error condition: Clients count is broken, dangling %last_id%", prompt::hids, last_id);
                }
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
        void fire_focus()
        {
            alive = true;
            //if constexpr (debugmode) log(prompt::foci, "Take focus hid:", id, " state:", f.state ? "on":"off");
            //todo focus<->seed
            if (focus::state) owner.SIGNAL(tier::release, hids::events::focus::set, *this);
            else              owner.SIGNAL(tier::release, hids::events::focus::off, *this);
        }
        void fire_board()
        {
            owner.SIGNAL(tier::release, hids::events::clipbrd, *this);
            mouse::delta.set(); // Update time stamp.
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
                switch (keycode & -2 /*Generic keys only*/)
                {
                    //todo Ctrl+Space
                    //     Ctrl+Backspace
                    //     Alt+0..9
                    //     Ctrl/Shift+Enter
                    case key::Backspace: textline = "\177"; break;
                    case key::Tab:
                        textline = ctlstate & hids::anyShift ? "\033[Z"
                                                             : "\t";
                        break;
                    case key::PageUp:     ctrl("[5",  "[5",  "~"); break;
                    case key::PageDown:   ctrl("[6",  "[6",  "~"); break;
                    case key::Insert:     ctrl("[2",  "[2",  "~"); break;
                    case key::Delete:     ctrl("[3",  "[3",  "~"); break;
                    case key::End:        ctrl("[",   "[1",  "F"); break;
                    case key::Home:       ctrl("[",   "[1",  "H"); break;
                    case key::UpArrow:    ctrl("[",   "[1",  "A"); break;
                    case key::DownArrow:  ctrl("[",   "[1",  "B"); break;
                    case key::RightArrow: ctrl("[",   "[1",  "C"); break;
                    case key::LeftArrow:  ctrl("[",   "[1",  "D"); break;
                    case key::F1:         ctrl("O",   "[1",  "P"); break;
                    case key::F2:         ctrl("O",   "[1",  "Q"); break;
                    case key::F3:         ctrl("O",   "[1",  "R"); break;
                    case key::F4:         ctrl("O",   "[1",  "S"); break;
                    case key::F5:         ctrl("[15", "[15", "~"); break;
                    case key::F6:         ctrl("[17", "[17", "~"); break;
                    case key::F7:         ctrl("[18", "[18", "~"); break;
                    case key::F8:         ctrl("[19", "[19", "~"); break;
                    case key::F9:         ctrl("[20", "[20", "~"); break;
                    case key::F10:        ctrl("[21", "[21", "~"); break;
                    case key::F11:        ctrl("[23", "[23", "~"); break;
                    case key::F12:        ctrl("[24", "[24", "~"); break;
                    case key::F13:        ctrl("[25", "[25", "~"); break;
                    case key::F14:        ctrl("[26", "[26", "~"); break;
                    case key::F15:        ctrl("[28", "[28", "~"); break;
                    case key::F16:        ctrl("[29", "[29", "~"); break;
                    case key::F17:        ctrl("[31", "[31", "~"); break;
                    case key::F18:        ctrl("[32", "[32", "~"); break;
                    case key::F19:        ctrl("[33", "[33", "~"); break;
                    case key::F20:        ctrl("[34", "[34", "~"); break;
                    case key::F21:        ctrl("[35", "[35", "~"); break;
                    case key::F22:        ctrl("[36", "[36", "~"); break;
                    case key::F23:        ctrl("[37", "[37", "~"); break;
                    case key::F24:        ctrl("[38", "[38", "~"); break;
                    default:
                        textline = cluster;
                        break;
                }
            }
            return textline;
        }
    };
}