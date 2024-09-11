// Copyright (c) Dmitry Sapozhnikov
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
                GROUP_XS( key    , input::hids ),
                GROUP_XS( control, input::hids ),
                GROUP_XS( state  , input::hids ),
                GROUP_XS( focus  , input::hids ),

                SUBSET_XS( key )
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
                    EVENT_XS( hop, input::foci ), // Change next hop destination. args: pair<what, with>.
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
                    EVENT_XS( act, input::hids ),
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
                        EVENT_XS( xbutton1 , input::hids ),
                        EVENT_XS( xbutton2 , input::hids ),
                        EVENT_XS( leftright, input::hids ),
                    };
                    SUBSET_XS( down )
                    {
                        EVENT_XS( left     , input::hids ),
                        EVENT_XS( right    , input::hids ),
                        EVENT_XS( middle   , input::hids ),
                        EVENT_XS( xbutton1 , input::hids ),
                        EVENT_XS( xbutton2 , input::hids ),
                        EVENT_XS( leftright, input::hids ),
                    };
                    SUBSET_XS( click )
                    {
                        EVENT_XS( left     , input::hids ),
                        EVENT_XS( right    , input::hids ),
                        EVENT_XS( middle   , input::hids ),
                        EVENT_XS( xbutton1 , input::hids ),
                        EVENT_XS( xbutton2 , input::hids ),
                        EVENT_XS( leftright, input::hids ),

                        INDEX_XS( left, right, middle, xbutton1, xbutton2, leftright ),
                    };
                    SUBSET_XS( dblclick )
                    {
                        EVENT_XS( left     , input::hids ),
                        EVENT_XS( right    , input::hids ),
                        EVENT_XS( middle   , input::hids ),
                        EVENT_XS( xbutton1 , input::hids ),
                        EVENT_XS( xbutton2 , input::hids ),
                        EVENT_XS( leftright, input::hids ),
                    };
                    SUBSET_XS( tplclick )
                    {
                        EVENT_XS( left     , input::hids ),
                        EVENT_XS( right    , input::hids ),
                        EVENT_XS( middle   , input::hids ),
                        EVENT_XS( xbutton1 , input::hids ),
                        EVENT_XS( xbutton2 , input::hids ),
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
                            EVENT_XS( xbutton1 , input::hids ),
                            EVENT_XS( xbutton2 , input::hids ),
                            EVENT_XS( leftright, input::hids ),

                            INDEX_XS( left, right, middle, xbutton1, xbutton2, leftright ),
                        };
                        SUBSET_XS( pull )
                        {
                            EVENT_XS( left     , input::hids ),
                            EVENT_XS( right    , input::hids ),
                            EVENT_XS( middle   , input::hids ),
                            EVENT_XS( xbutton1 , input::hids ),
                            EVENT_XS( xbutton2 , input::hids ),
                            EVENT_XS( leftright, input::hids ),

                            INDEX_XS( left, right,  middle, xbutton1, xbutton2, leftright ),
                        };
                        SUBSET_XS( cancel )
                        {
                            EVENT_XS( left     , input::hids ),
                            EVENT_XS( right    , input::hids ),
                            EVENT_XS( middle   , input::hids ),
                            EVENT_XS( xbutton1 , input::hids ),
                            EVENT_XS( xbutton2 , input::hids ),
                            EVENT_XS( leftright, input::hids ),

                            INDEX_XS( left, right, middle, xbutton1, xbutton2, leftright ),
                        };
                        SUBSET_XS( stop )
                        {
                            EVENT_XS( left     , input::hids ),
                            EVENT_XS( right    , input::hids ),
                            EVENT_XS( middle   , input::hids ),
                            EVENT_XS( xbutton1 , input::hids ),
                            EVENT_XS( xbutton2 , input::hids ),
                            EVENT_XS( leftright, input::hids ),

                            INDEX_XS( left, right, middle, xbutton1, xbutton2, leftright ),
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
    using ui::sptr;
    using ui::wptr;
    using ui::base;
    using ui::face;
    using ui::page;

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
                return mask()[std::clamp(vk, 0, 255)];
            }
            static auto& data(si32 keycode)
            {
                struct key
                {
                    view name;
                    si32 vkey;
                    si32 scan;
                    si32 edit;
                };
                static auto data = std::vector<key>(256);
                return data[std::clamp(keycode, 0, 255)];
            }

            map(si32 vk, si32 sc, si32 cs)
                : hash{ static_cast<sz_t>(mask(vk) & (vk | (sc << 8) | (cs << 16))) }
            { }
            map(si32 vk, si32 sc, si32 cs, si32 keymask, view keyname, si32 doinput, si32 id)
            {
                mask(vk) = keymask;
                data(id) = { .name = keyname, .vkey = vk, .scan = sc, .edit = doinput };
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

        //todo check non-us kb layouts with key::Slash
        #define key_list \
            /*Id   Vkey  Scan    CtrlState          Mask  I  Name            */\
            X(0,      0,    0,           0, 0x0000'00'FF, 1, undef            )\
            X(1,   0xFF, 0xFF,           0, 0x0000'FF'FF, 0, config           )\
            X(2,   0x11, 0x1D,           0, 0x0100'00'FF, 0, LeftCtrl         )\
            X(4,   0x11, 0x1D, ExtendedKey, 0x0100'00'FF, 0, RightCtrl        )\
            X(6,   0x12, 0x38,           0, 0x0100'00'FF, 0, LeftAlt          )\
            X(8,   0x12, 0x38, ExtendedKey, 0x0100'00'FF, 0, RightAlt         )\
            X(10,  0x10, 0x2A,           0, 0x0000'FF'FF, 0, LeftShift        )\
            X(11,  0x10, 0x36,           0, 0x0000'FF'FF, 0, RightShift       )\
            X(12,  0x5B, 0x5B, ExtendedKey, 0x0000'00'FF, 0, LeftWin          )\
            X(13,  0x5D, 0x5D, ExtendedKey, 0x0000'00'FF, 0, Apps             )\
            X(14,  0x5C, 0x5C, ExtendedKey, 0x0000'00'FF, 0, RightWin         )\
            X(15,  0x90, 0x45,           0, 0x0000'00'FF, 0, NumLock          )\
            X(16,  0x14, 0x3A,           0, 0x0000'00'FF, 0, CapsLock         )\
            X(17,  0x91, 0x45,           0, 0x0000'00'FF, 0, ScrollLock       )\
            X(18,  0x1B, 0x01,           0, 0x0000'00'FF, 1, Esc              )\
            X(20,  0x20, 0x39,           0, 0x0000'00'FF, 1, Space            )\
            X(22,  0x08, 0x0E,           0, 0x0000'00'FF, 1, Backspace        )\
            X(24,  0x09, 0x0F,           0, 0x0000'00'FF, 1, Tab              )\
            X(26,  0x03, 0x46,           0, 0x0000'FF'FF, 1, Break            )\
            X(28,  0x13, 0x45,           0, 0x0000'FF'FF, 0, Pause            )\
            X(30,  0x29,    0,           0, 0x0000'00'FF, 0, Select           )\
            X(32,  0x2C, 0x54,           0, 0x0000'FF'FF, 1, SysRq            )\
            X(34,  0x2C, 0x37, ExtendedKey, 0x0100'FF'FF, 0, PrintScreen      )\
            X(36,  0x0D, 0x1C,           0, 0x0100'00'FF, 1, Enter            )\
            X(37,  0x0D, 0x1C, ExtendedKey, 0x0100'00'FF, 1, NumpadEnter      )\
            X(38,  0x21, 0x49, ExtendedKey, 0x0100'00'FF, 1, PageUp           )\
            X(39,  0x21, 0x49,           0, 0x0100'00'FF, 1, NumpadPageUp     )\
            X(40,  0x22, 0x51, ExtendedKey, 0x0100'00'FF, 1, PageDown         )\
            X(41,  0x22, 0x51,           0, 0x0100'00'FF, 1, NumpadPageDown   )\
            X(42,  0x23, 0x4F, ExtendedKey, 0x0100'00'FF, 1, End              )\
            X(43,  0x23, 0x4F,           0, 0x0100'00'FF, 1, NumpadEnd        )\
            X(44,  0x24, 0x47, ExtendedKey, 0x0100'00'FF, 1, Home             )\
            X(45,  0x24, 0x47,           0, 0x0100'00'FF, 1, NumpadHome       )\
            X(46,  0x25, 0x4B, ExtendedKey, 0x0100'00'FF, 1, LeftArrow        )\
            X(47,  0x25, 0x4B,           0, 0x0100'00'FF, 1, NumpadLeftArrow  )\
            X(48,  0x26, 0x48, ExtendedKey, 0x0100'00'FF, 1, UpArrow          )\
            X(49,  0x26, 0x48,           0, 0x0100'00'FF, 1, NumpadUpArrow    )\
            X(50,  0x27, 0x4D, ExtendedKey, 0x0100'00'FF, 1, RightArrow       )\
            X(51,  0x27, 0x4D,           0, 0x0100'00'FF, 1, NumpadRightArrow )\
            X(52,  0x28, 0x50, ExtendedKey, 0x0100'00'FF, 1, DownArrow        )\
            X(53,  0x28, 0x50,           0, 0x0100'00'FF, 1, NumpadDownArrow  )\
            X(54,  0x30, 0x0B,           0, 0x0000'FF'FF, 1, Key0             )\
            X(55,  0x60, 0x52, NumLockMode, 0x0000'FF'FF, 1, Numpad0          )\
            X(56,  0x31, 0x02,           0, 0x0000'FF'FF, 1, Key1             )\
            X(57,  0x61, 0x4F, NumLockMode, 0x0000'FF'FF, 1, Numpad1          )\
            X(58,  0x32, 0x03,           0, 0x0000'FF'FF, 1, Key2             )\
            X(59,  0x62, 0x50, NumLockMode, 0x0000'FF'FF, 1, Numpad2          )\
            X(60,  0x33, 0x04,           0, 0x0000'FF'FF, 1, Key3             )\
            X(61,  0x63, 0x51, NumLockMode, 0x0000'FF'FF, 1, Numpad3          )\
            X(62,  0x34, 0x05,           0, 0x0000'FF'FF, 1, Key4             )\
            X(63,  0x64, 0x4B, NumLockMode, 0x0000'FF'FF, 1, Numpad4          )\
            X(64,  0x35, 0x06,           0, 0x0000'FF'FF, 1, Key5             )\
            X(65,  0x65, 0x4C, NumLockMode, 0x0000'FF'FF, 1, Numpad5          )\
            X(66,  0x36, 0x07,           0, 0x0000'FF'FF, 1, Key6             )\
            X(67,  0x66, 0x4D, NumLockMode, 0x0000'FF'FF, 1, Numpad6          )\
            X(68,  0x37, 0x08,           0, 0x0000'FF'FF, 1, Key7             )\
            X(69,  0x67, 0x47, NumLockMode, 0x0000'FF'FF, 1, Numpad7          )\
            X(70,  0x38, 0x09,           0, 0x0000'FF'FF, 1, Key8             )\
            X(71,  0x68, 0x48, NumLockMode, 0x0000'FF'FF, 1, Numpad8          )\
            X(72,  0x39, 0x0A,           0, 0x0000'FF'FF, 1, Key9             )\
            X(73,  0x69, 0x49, NumLockMode, 0x0000'FF'FF, 1, Numpad9          )\
            X(74,  0x2D, 0x52, ExtendedKey, 0x0100'00'FF, 1, Insert           )\
            X(75,  0x2D, 0x52,           0, 0x0100'00'FF, 1, NumpadInsert     )\
            X(76,  0x2E, 0x53, ExtendedKey, 0x0100'00'FF, 1, Delete           )\
            X(77,  0x2E, 0x55,           0, 0x0100'00'FF, 1, NumpadDelete     )\
            X(78,  0x0C, 0x4C, ExtendedKey, 0x0100'00'FF, 1, Clear            )\
            X(79,  0x0C, 0x4C,           0, 0x0100'00'FF, 1, NumpadClear      )\
            X(80,  0x6A, 0x09,           0, 0x0000'FF'FF, 1, Multiply         )\
            X(81,  0x6A, 0x37,           0, 0x0000'FF'FF, 1, NumpadMultiply   )\
            X(82,  0x6B, 0x0D,           0, 0x0000'FF'FF, 1, Plus             )\
            X(83,  0x6B, 0x4E,           0, 0x0000'FF'FF, 1, NumpadPlus       )\
            X(84,  0x6C,    0,           0, 0x0020'00'FF, 1, Separator        )\
            X(85,  0x6C,    0, NumLockMode, 0x0020'00'FF, 1, NumpadSeparator  )\
            X(86,  0xBD, 0x0C,           0, 0x0000'00'FF, 1, Minus            )\
            X(87,  0x6D, 0x4A,           0, 0x0000'00'FF, 1, NumpadMinus      )\
            X(88,  0xBE, 0x34,           0, 0x0000'00'FF, 1, Period           )\
            X(89,  0x6E, 0x53, NumLockMode, 0x0000'00'FF, 1, NumpadDecimal    )\
            X(90,  0xBF, 0x35,           0, 0x0000'00'FF, 1, Slash            )\
            X(91,  0x6F, 0x35, ExtendedKey, 0x0000'00'FF, 1, NumpadSlash      )\
            X(92,  0xDC, 0x2B,           0, 0x0000'00'FF, 1, BackSlash        )\
            X(94,  0xDB, 0x1A,           0, 0x0000'00'FF, 1, OpenBracket      )\
            X(96,  0xDD, 0x1B,           0, 0x0000'00'FF, 1, ClosedBracket    )\
            X(98,  0xBB, 0x0D,           0, 0x0000'00'FF, 1, Equal            )\
            X(100, 0xC0, 0x29,           0, 0x0000'00'FF, 1, BackQuote        )\
            X(102, 0xDE, 0x28,           0, 0x0000'00'FF, 1, SingleQuote      )\
            X(104, 0xBC, 0x33,           0, 0x0000'00'FF, 1, Comma            )\
            X(106, 0xBA, 0x27,           0, 0x0000'00'FF, 1, Semicolon        )\
            X(108, 0x70, 0x3B,           0, 0x0000'00'FF, 1, F1               )\
            X(110, 0x71, 0x3C,           0, 0x0000'00'FF, 1, F2               )\
            X(112, 0x72, 0x3D,           0, 0x0000'00'FF, 1, F3               )\
            X(114, 0x73, 0x3E,           0, 0x0000'00'FF, 1, F4               )\
            X(116, 0x74, 0x3F,           0, 0x0000'00'FF, 1, F5               )\
            X(118, 0x75, 0x40,           0, 0x0000'00'FF, 1, F6               )\
            X(120, 0x76, 0x41,           0, 0x0000'00'FF, 1, F7               )\
            X(122, 0x77, 0x42,           0, 0x0000'00'FF, 1, F8               )\
            X(124, 0x78, 0x43,           0, 0x0000'00'FF, 1, F9               )\
            X(126, 0x79, 0x44,           0, 0x0000'00'FF, 1, F10              )\
            X(128, 0x7A, 0x57,           0, 0x0000'00'FF, 1, F11              )\
            X(130, 0x7B, 0x5B,           0, 0x0000'00'FF, 1, F12              )\
            X(132, 0x7C,    0,           0, 0x0000'00'FF, 1, F13              )\
            X(134, 0x7D,    0,           0, 0x0000'00'FF, 1, F14              )\
            X(136, 0x7E,    0,           0, 0x0000'00'FF, 1, F15              )\
            X(138, 0x7F,    0,           0, 0x0000'00'FF, 1, F16              )\
            X(140, 0x80,    0,           0, 0x0000'00'FF, 1, F17              )\
            X(142, 0x81,    0,           0, 0x0000'00'FF, 1, F18              )\
            X(144, 0x82,    0,           0, 0x0000'00'FF, 1, F19              )\
            X(146, 0x83,    0,           0, 0x0000'00'FF, 1, F20              )\
            X(148, 0x84,    0,           0, 0x0000'00'FF, 1, F21              )\
            X(150, 0x85,    0,           0, 0x0000'00'FF, 1, F22              )\
            X(152, 0x86,    0,           0, 0x0000'00'FF, 1, F23              )\
            X(154, 0x87,    0,           0, 0x0000'00'FF, 1, F24              )\
            X(156, 0x41,    0,           0, 0x0100'00'FF, 1, KeyA             )\
            X(158, 0x42,    0,           0, 0x0100'00'FF, 1, KeyB             )\
            X(160, 0x43,    0,           0, 0x0100'00'FF, 1, KeyC             )\
            X(162, 0x44,    0,           0, 0x0100'00'FF, 1, KeyD             )\
            X(164, 0x45,    0,           0, 0x0100'00'FF, 1, KeyE             )\
            X(166, 0x46,    0,           0, 0x0100'00'FF, 1, KeyF             )\
            X(168, 0x47,    0,           0, 0x0100'00'FF, 1, KeyG             )\
            X(170, 0x48,    0,           0, 0x0100'00'FF, 1, KeyH             )\
            X(172, 0x49,    0,           0, 0x0100'00'FF, 1, KeyI             )\
            X(174, 0x4A,    0,           0, 0x0100'00'FF, 1, KeyJ             )\
            X(176, 0x4B,    0,           0, 0x0100'00'FF, 1, KeyK             )\
            X(178, 0x4C,    0,           0, 0x0100'00'FF, 1, KeyL             )\
            X(180, 0x4D,    0,           0, 0x0100'00'FF, 1, KeyM             )\
            X(182, 0x4E,    0,           0, 0x0100'00'FF, 1, KeyN             )\
            X(184, 0x4F,    0,           0, 0x0100'00'FF, 1, KeyO             )\
            X(186, 0x50,    0,           0, 0x0100'00'FF, 1, KeyP             )\
            X(188, 0x51,    0,           0, 0x0100'00'FF, 1, KeyQ             )\
            X(190, 0x52,    0,           0, 0x0100'00'FF, 1, KeyR             )\
            X(192, 0x53,    0,           0, 0x0100'00'FF, 1, KeyS             )\
            X(194, 0x54,    0,           0, 0x0100'00'FF, 1, KeyT             )\
            X(196, 0x55,    0,           0, 0x0100'00'FF, 1, KeyU             )\
            X(198, 0x56,    0,           0, 0x0100'00'FF, 1, KeyV             )\
            X(200, 0x57,    0,           0, 0x0100'00'FF, 1, KeyW             )\
            X(202, 0x58,    0,           0, 0x0100'00'FF, 1, KeyX             )\
            X(204, 0x59,    0,           0, 0x0100'00'FF, 1, KeyY             )\
            X(206, 0x5A,    0,           0, 0x0100'00'FF, 1, KeyZ             )\
            X(208, 0x5F,    0, ExtendedKey, 0x0100'00'FF, 0, Sleep            )\
            X(210, 0xB7,    0, ExtendedKey, 0x0100'00'FF, 0, Calculator       )\
            X(212, 0x48,    0, ExtendedKey, 0x0100'00'FF, 0, Mail             )\
            X(214, 0xAD,    0, ExtendedKey, 0x0100'00'FF, 0, MediaVolMute     )\
            X(216, 0xAE,    0, ExtendedKey, 0x0100'00'FF, 0, MediaVolDown     )\
            X(218, 0xAF,    0, ExtendedKey, 0x0100'00'FF, 0, MediaVolUp       )\
            X(220, 0xB0,    0, ExtendedKey, 0x0100'00'FF, 0, MediaNext        )\
            X(222, 0xB1,    0, ExtendedKey, 0x0100'00'FF, 0, MediaPrev        )\
            X(224, 0xB2,    0, ExtendedKey, 0x0100'00'FF, 0, MediaStop        )\
            X(226, 0xB3,    0, ExtendedKey, 0x0100'00'FF, 0, MediaPlayPause   )\
            X(228, 0xB5,    0, ExtendedKey, 0x0100'00'FF, 0, MediaSelect      )\
            X(230, 0xA6,    0, ExtendedKey, 0x0100'00'FF, 0, BrowserBack      )\
            X(232, 0xA7,    0, ExtendedKey, 0x0100'00'FF, 0, BrowserForward   )\
            X(234, 0xA8,    0, ExtendedKey, 0x0100'00'FF, 0, BrowserRefresh   )\
            X(236, 0xA9,    0, ExtendedKey, 0x0100'00'FF, 0, BrowserStop      )\
            X(238, 0xAA,    0, ExtendedKey, 0x0100'00'FF, 0, BrowserSearch    )\
            X(240, 0xAB,    0, ExtendedKey, 0x0100'00'FF, 0, BrowserFavorites )\
            X(242, 0xAC,    0, ExtendedKey, 0x0100'00'FF, 0, BrowserHome      )

        #define X(KeyId, Vkey, Scan, CtrlState, Mask, Input, Name) \
            static constexpr auto Name = KeyId;
            key_list
        #undef X

        static const auto keymap = std::unordered_map<map, si32, map::hashproc>
        {
            #define X(KeyId, Vkey, Scan, CtrlState, Mask, Input, Name) \
                { map{ Vkey, Scan, CtrlState, Mask, #Name, Input, Name }, Name, },
                key_list
            #undef X
        };

        #undef key_list

        template<class ...Args>
        auto xlat(Args&&... args)
        {
            auto iter = keymap.find(map{ args... });
            return iter != keymap.end() ? iter->second : key::undef;
        }
        auto find(si32 vkey, si32 fallback)
        {
            auto k = fallback;
            for (auto& [key, val] : keymap)
            {
                if ((si32)(key.hash & 0xff) == vkey)
                {
                    k = val & -2; // Generic keys only.
                    break;
                }
            }
            return k;
        }
    }

    struct foci
    {
        id_t   id{}; // foci: Gear id.
        si32 solo{}; // foci: Exclusive focus request.
        bool flip{}; // foci: Toggle focus request.
        bool skip{}; // foci: Ignore focusable object, just activate it.
        sptr what{}; // foci: Replacement item.
        sptr item{}; // foci: Next focused item.
        ui32 deep{}; // foci: Counter for debug.
        time guid{}; // foci: Originating environment ID.
    };

    // input: Mouse tracker.
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
            xbutton1  = click::xbutton1 .index(),
            xbutton2  = click::xbutton2 .index(),
            leftright = click::leftright.index(),
            numofbuttons,
        };

        struct stat
        {
            enum
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
        using tail = netxs::datetime::tail<fp2d>;

        static constexpr auto dragstrt = mouse_event::button::drag::start:: any.group<numofbuttons>();
        static constexpr auto dragpull = mouse_event::button::drag::pull::  any.group<numofbuttons>();
        static constexpr auto dragcncl = mouse_event::button::drag::cancel::any.group<numofbuttons>();
        static constexpr auto dragstop = mouse_event::button::drag::stop::  any.group<numofbuttons>();
        static constexpr auto released = mouse_event::button::up::          any.group<numofbuttons>();
        static constexpr auto pushdown = mouse_event::button::down::        any.group<numofbuttons>();
        static constexpr auto sglclick = mouse_event::button::click::       any.group<numofbuttons>();
        static constexpr auto dblclick = mouse_event::button::dblclick::    any.group<numofbuttons>();
        static constexpr auto tplclick = mouse_event::button::tplclick::    any.group<numofbuttons>();
        static constexpr auto wheeling = mouse_event::scroll::act.id;
        static constexpr auto movement = mouse_event::move.id;
        static constexpr auto noactive = si32{ -1 };

        fp2d prime{}; // mouse: System mouse cursor coordinates.
        fp2d coord{}; // mouse: Relative mouse cursor coordinates.
        fp32 accum{}; // mouse: Mouse motion accumulator to delay mouse drag.
        tail delta{}; // mouse: History of mouse movements for a specified period of time.
        bool reach{}; // mouse: Has the event tree relay reached the mouse event target.
        bool nodbl{}; // mouse: Whether single click event processed (to prevent double clicks).
        bool hzwhl{}; // mouse: If true: Horizontal scrolling. If faux: Vertical scrolling.
        fp32 whlfp{}; // mouse: Scroll delta in float units (lines).
        si32 whlsi{}; // mouse: Scroll delta in integer units (lines).
        si32 locks{}; // mouse: State of the captured buttons (bit field).
        si32 index{}; // mouse: Index of the active button. -1 if the buttons are not involed.
        id_t swift{}; // mouse: Delegate's ID of the current mouse owner.
        id_t hover{}; // mouse: Hover control ID.
        id_t start{}; // mouse: Initiator control ID.
        hint cause{}; // mouse: Current event id.
        hist stamp{}; // mouse: Recorded intervals between successive button presses to track double-clicks.
        span delay{}; // mouse: Double-click threshold.
        knob bttns{}; // mouse: Extended state of mouse buttons.
        sysmouse m_sys{}; // mouse: Device state.
        sysmouse m_sav{}; // mouse: Previous device state.

        static constexpr auto drag_threshold = 0.5f; // mouse: Threshold for mouse drag.

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
            auto bitstat = si32{};
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
        auto load_button_state(si32 bitstat)
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
        // mouse: Sync the button state with a bitset.
        auto sync_button_state(si32 bitstat)
        {
            for (auto& b : bttns)
            {
                b.pressed = bitstat & 0x1;
                bitstat >>= 1;
            }
        }
        // mouse: Return a kinetic animator.
        template<class Law>
        auto fader(span spell)
        {
            //todo use current item's type: Law<twod>
            return delta.fader<Law>(spell);
        }
        // mouse: Generate mouse event.
        void update(sysmouse& m, core const& idmap)
        {
            auto m_buttons = std::bitset<8>(m.buttons);
            // Interpret button combinations.
            //todo possible bug in Apple's Terminal - it does not return the second release
            //                                        in case when two buttons are pressed.
            m_buttons[leftright] = (bttns[leftright].pressed && (m_buttons[left] || m_buttons[right]))
                                                             || (m_buttons[left] && m_buttons[right]);
            m.buttons = (si32)m_buttons.to_ulong();
            auto modschanged = m_sys.ctlstat != m.ctlstat;
            m_sys.set(m);
            auto busy = captured();
            if (busy && fire_fast())
            {
                delta.set(m_sys.coordxy - prime);
                coord = m_sys.coordxy;
                prime = m_sys.coordxy;
                fire(movement); // Update mouse enter/leave state.
                sync_button_state(m_sys.buttons);
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
                m_sys.buttons = static_cast<si32>(m_buttons.to_ulong());
            }

            // Suppress left and right to avoid single button tracking (click, pull, etc)
            bttns[left ].blocked = m_buttons[leftright] || bttns[leftright].pressed;
            bttns[right].blocked = bttns[left].blocked;

            if (m_sys.coordxy != prime || modschanged)
            {
                auto step = m_sys.coordxy - prime;
                if (m.buttons) accum += std::abs(step.x) + std::abs(step.y);
                else           accum = {};
                auto new_target = idmap.link(m_sys.coordxy) != idmap.link(prime);
                auto allow_drag = accum > drag_threshold || new_target;
                delta.set(step);
                auto active = si32{};
                auto genptr = std::begin(bttns);
                for (auto i = 0; i < numofbuttons; i++)
                {
                    auto& genbtn = *genptr++;
                    if (genbtn.pressed && !genbtn.blocked)
                    {
                        if (allow_drag && !genbtn.dragged)
                        {
                            fire(dragstrt, i);
                            genbtn.dragged = true;
                        }
                        active |= 1 << i;
                    }
                }
                coord = m_sys.coordxy;
                prime = m_sys.coordxy;
                if (allow_drag) for (auto i = 0; active; ++i)
                {
                    if (active & 0x1)
                    {
                        fire(dragpull, i);
                    }
                    active >>= 1;
                }
                fire(movement);
            }

            if (!busy && fire_fast())
            {
                sync_button_state(m_sys.buttons);
                return;
            }

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
                                auto fired = m_sys.timecod;
                                if (fired - s.fired < delay && s.coord == coord)
                                {
                                    if (!genbtn.blocked)
                                    {
                                        if (s.count == 1)
                                        {
                                            fire(dblclick, i);
                                            s.fired = fired;
                                            s.count++;
                                        }
                                        else if (s.count >= 2)
                                        {
                                            fire(tplclick, i);
                                            if (s.count == 4) // Limit to quintuple click.
                                            {
                                                s.fired = {};
                                                s.count = {};
                                            }
                                            else
                                            {
                                                s.fired = fired;
                                                s.count++;
                                            }
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

            coord = m_sys.coordxy;
            if (m_sys.wheelfp)
            {
                hzwhl = m_sys.hzwheel;
                whlfp = m_sys.wheelfp;
                whlsi = m_sys.wheelsi;
                fire(wheeling);
                m_sys.hzwheel = {}; // Clear one-shot events.
                m_sys.wheelfp = {};
                m_sys.wheelsi = {};
            }
        }
        // mouse: Return the number of clicks for the specified button.
        auto clicks(si32 button)
        {
            button = std::clamp(button, 0, buttons::numofbuttons - 1);
            return stamp[button].count + 1;
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

    // input: Keybd tracker.
    struct keybd
    {
        enum prot
        {
            vt,
            w32,
        };
        struct type
        {
            static constexpr byte keypress = 0;
            static constexpr byte keypaste = 1;
            static constexpr byte imeinput = 2;
            static constexpr byte imeanons = 3;
            static constexpr byte kblayout = 4;
        };

        si32 nullkey = key::Key2;

        text cluster{};
        byte payload{}; // keybd: Payload type.
        bool extflag{};
        bool pressed{};
        bool handled{};
        si32 virtcod{};
        si32 scancod{};
        si32 keycode{};

        auto doinput()
        {
            return pressed && key::map::data(keycode).edit;
        }
        auto generic()
        {
            return keycode & -2;
        }
        void update(syskeybd& k)
        {
            extflag = k.extflag;
            payload = k.payload;
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

    // input: Focus tracker.
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

    // input: Clipboard tracker.
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

        static void normalize(clipdata& c, id_t gear_id, time hash, twod winsz, qiew utf8, si32 form, qiew meta = {})
        {
            auto size = dot_00;
            if (form == mime::disabled && meta) // Try to parse meta: mime/size_x/size_y;data
            {
                     if (meta.starts_with(mime::tag::ansi)) { meta.remove_prefix(mime::tag::ansi.length()); form = mime::ansitext; }
                else if (meta.starts_with(mime::tag::text)) { meta.remove_prefix(mime::tag::text.length()); form = mime::textonly; }
                else if (meta.starts_with(mime::tag::rich)) { meta.remove_prefix(mime::tag::rich.length()); form = mime::richtext; }
                else if (meta.starts_with(mime::tag::html)) { meta.remove_prefix(mime::tag::html.length()); form = mime::htmltext; }
                else if (meta.starts_with(mime::tag::safe)) { meta.remove_prefix(mime::tag::safe.length()); form = mime::safetext; }

                if (form != mime::disabled)
                {
                    if (meta && meta.front() == '/') // Proceed preview size if present.
                    {
                        meta.remove_prefix(1);
                        if (auto v = utf::to_int(meta))
                        {
                            static constexpr auto max_value = twod{ 1000, 500 }; //todo unify
                            size.x = v.value();
                            if (meta)
                            {
                                meta.remove_prefix(1);
                                v = utf::to_int(meta);
                                if (v) size.y = v.value();
                            }
                            if (!size.x || !size.y) size = dot_00;
                            else                    size = std::clamp(size, dot_00, max_value);
                        }
                    }
                    meta = {};
                }
            }
            size = utf8.empty() ? dot_00
                 : size         ? size
                 : winsz        ? winsz
                                : twod{ 80,25 }; //todo make it configurable
            c.set(gear_id, hash, size, utf8.str(), form, meta.str());
        }
        static void normalize(clipdata& c)
        {
            normalize(c, id_t{}, c.hash, c.size, c.utf8, c.form, c.meta);
        }
        auto clear_clipboard()
        {
            auto not_empty = !!board::cargo.utf8.size();
            board::cargo.set(id_t{}, datetime::now(), dot_00, text{}, mime::ansitext, text{});
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
            c.set(id_t{}, datetime::now(), size, utf8.str(), form, text{});
            set_clipboard(c);
        }
        void update(sysboard& b) // Update clipboard preview.
        {
            board::image.move(dot_00); // Reset basis.
            auto draw_shadow = [](auto& canvas, auto& block, auto shadow_radius, twod trim_to)
            {
                canvas.mark(cell{});
                canvas.wipe();
                canvas.size(dot_21 * shadow_radius * 2 + trim_to);
                auto full = rect{ dot_21 * shadow_radius + dot_21, trim_to };
                auto temp = vrgb{};
                while (shadow_radius--)
                {
                    canvas.reset();
                    canvas.full(full);
                    canvas.template output<true>(block, cell::shaders::color(cell{}.bgc(0).fgc(0).alpha(0x60)));
                    canvas.template blur<true>(1, temp, [&](cell& c){ c.fgc(c.bgc()).txt(""); });
                }
                full.coor -= dot_21;
                canvas.reset();
                canvas.full(full);
            };
            if (b.form == mime::safetext)
            {
                auto blank = ansi::bgc(0x7Fffffff).fgc(0xFF000000).add(" Protected Data "); //todo unify (i18n)
                auto block = page{ blank };
                if (ghost) draw_shadow(board::image, block, ghost, block.current().size());
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
                if (ghost) draw_shadow(board::image, block, ghost, b.size);
                else
                {
                    board::image.size(b.size);
                    board::image.wipe();
                }
                board::image.mark(cell{});
                auto plain = b.form == mime::textonly || b.form == mime::disabled;
                if (plain) board::image.output<true>(block, cell::shaders::color(  brush));
                else       board::image.output<true>(block, cell::shaders::xlucent(alpha));
            }
        }
    };

    // input: Human interface device controller.
    struct hids
        : public mouse,
          public keybd,
          public focus,
          public board,
          public bell
    {
        using events = netxs::events::userland::hids;
        using list = std::list<wptr>;
        using kmap = std::unordered_map<si32, text>;

        enum modifiers
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
            AltGr    = LAlt   | LCtrl,
            anyCtrl  = LCtrl  | RCtrl,
            anyAlt   = LAlt   | RAlt,
            anyShift = LShift | RShift,
            anyAltGr = anyAlt | anyCtrl,
            anyWin   = LWin   | RWin,
            anyMod   = anyAlt | anyCtrl | anyShift | anyWin,
        };

        static auto build_alone_key()
        {
            return std::unordered_map<si32, text>
            {
                { key::Backspace,  "\x7f"     },
                { key::Tab,        "\x09"     },
                { key::Pause,      "\x1a"     },
                { key::Esc,        "\033"     },
                { key::PageUp,     "\033[5~"  },
                { key::PageDown,   "\033[6~"  },
                { key::End,        "\033[F"   },
                { key::Home,       "\033[H"   },
                { key::LeftArrow,  "\033[D"   },
                { key::UpArrow,    "\033[A"   },
                { key::RightArrow, "\033[C"   },
                { key::DownArrow,  "\033[B"   },
                { key::Insert,     "\033[2~"  },
                { key::Delete,     "\033[3~"  },
                { key::F1,         "\033OP"   },
                { key::F2,         "\033OQ"   },
                { key::F3,         "\033OR"   },
                { key::F4,         "\033OS"   },
                { key::F5,         "\033[15~" },
                { key::F6,         "\033[17~" },
                { key::F7,         "\033[18~" },
                { key::F8,         "\033[19~" },
                { key::F9,         "\033[20~" },
                { key::F10,        "\033[21~" },
                { key::F11,        "\033[23~" },
                { key::F12,        "\033[24~" },
            };
        }
        static auto build_shift_key()
        {
            return std::unordered_map<si32, text>
            {
                { key::PageUp,     "\033[5; ~"  },
                { key::PageDown,   "\033[6; ~"  },
                { key::End,        "\033[1; F"  },
                { key::Home,       "\033[1; H"  },
                { key::LeftArrow,  "\033[1; D"  },
                { key::UpArrow,    "\033[1; A"  },
                { key::RightArrow, "\033[1; C"  },
                { key::DownArrow,  "\033[1; B"  },
                { key::Insert,     "\033[2; ~"  },
                { key::Delete,     "\033[3; ~"  },
                { key::F1,         "\033[1; P"  },
                { key::F2,         "\033[1; Q"  },
                { key::F3,         "\033[1; R"  },
                { key::F4,         "\033[1; S"  },
                { key::F5,         "\033[15; ~" },
                { key::F6,         "\033[17; ~" },
                { key::F7,         "\033[18; ~" },
                { key::F8,         "\033[19; ~" },
                { key::F9,         "\033[20; ~" },
                { key::F10,        "\033[21; ~" },
                { key::F11,        "\033[23; ~" },
                { key::F12,        "\033[24; ~" },
            };

        }
        static auto build_other_key(si32 slash, si32 quest)
        {
            return std::unordered_map<si32, text>
            {
                { key::Enter     | hids::anyCtrl  << 8, { "\x0a"      }},
                { key::Backspace | hids::anyCtrl  << 8, { "\x08"      }},
                { key::Backspace | hids::anyAlt   << 8, { "\033\x7f"  }},
                { key::Backspace | hids::anyAltGr << 8, { "\033\x08"  }},
                { key::Tab       | hids::anyCtrl  << 8, { "\t"        }},
                { key::Tab       | hids::anyShift << 8, { "\033[Z"    }},
                { key::Tab       | hids::anyAlt   << 8, { "\033[1;3I" }},
                { key::Esc       | hids::anyAlt   << 8, { "\033\033"  }},
                { key::Key1      | hids::anyCtrl  << 8, { "1"         }},
                { key::Key3      | hids::anyCtrl  << 8, { "\x1b"      }},
                { key::Key4      | hids::anyCtrl  << 8, { "\x1c"      }},
                { key::Key5      | hids::anyCtrl  << 8, { "\x1d"      }},
                { key::Key6      | hids::anyCtrl  << 8, { "\x1e"      }},
                { key::Key7      | hids::anyCtrl  << 8, { "\x1f"      }},
                { key::Key8      | hids::anyCtrl  << 8, { "\x7f"      }},
                { key::Key9      | hids::anyCtrl  << 8, { "9"         }},
                { key::Slash     | hids::anyCtrl  << 8, { "\x1f"      }},
                { slash          | hids::anyAltGr << 8, { "\033\x1f"  }},
                { slash          | hids::anyCtrl  << 8, { "\x1f"      }},
                { quest          | hids::anyAltGr << 8, { "\033\x7f"  }},
                { quest          | hids::anyCtrl  << 8, { "\x7f"      }},
            };
        }

        id_t        relay; // hids: Mouse routing call stack initiator.
        base&       owner;
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
        bool        tooltip_set  = faux; // hids: Tooltip has been set.
        testy<twod> tooltip_coor = {}; // hids: .

        si32 ctlstate = {};

        //todo unify
        rect slot; // slot for pro::maker and e2::createby.
        bool slot_forced = faux; // .

        //todo unify
        bool disabled = faux;
        si32 countdown = 0;

        id_t user_index; // hids: User/Device image/icon index.
        kmap other_key; // hids: Dynamic key-vt mapping.

        template<class T>
        hids(auth& indexer, T& props, base& owner, core const& idmap)
            : bell{ indexer },
              relay{ 0 },
              owner{ owner },
              idmap{ idmap },
              alive{ faux },
              tooltip_timeout{   props.tooltip_timeout },
              other_key{ build_other_key(key::Slash, key::Slash | (hids::anyShift << 8)) } // Defaults for US layout.
        {
            board::ghost = props.clip_preview_glow;
            board::brush = props.clip_preview_clrs;
            board::alpha = props.clip_preview_alfa;
            mouse::delay = props.dblclick_timeout;
            mouse::prime = dot_mx;
            mouse::coord = dot_mx;
            SIGNAL(tier::general, events::device::user::login, user_index);
        }
        virtual ~hids()
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
            return !mouse::m_sys.buttons
                && !disabled
                && !tooltip_stop
                && tooltip_show
                && tooltip_data.size()
                && tooltip_time < now
                && !captured();
        }
        void set_tooltip(view data, bool update = faux)
        {
            tooltip_set = true;
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
            else if (deed == hids::events::mouse::scroll::act.id) // Drop tooltip away.
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

        void replay(hint new_cause, fp2d new_coord, fp2d new_delta, si32 new_button_state, si32 new_ctlstate, fp32 new_whlfp, si32 new_whlsi, bool new_hzwhl)
        {
            static constexpr auto mask = netxs::events::level_mask(hids::events::mouse::button::any.id);
            static constexpr auto base = mask & hids::events::mouse::button::any.id;
            alive = true;
            ctlstate = new_ctlstate;
            mouse::coord = new_coord;
            mouse::whlfp = new_whlfp;
            mouse::whlsi = new_whlsi;
            mouse::hzwhl = new_hzwhl;
            mouse::cause = (new_cause & ~mask) | base; // Remove the dependency on the event tree root.
            mouse::delta.set(new_delta);
            mouse::load_button_state(new_button_state);
        }

        auto meta(si32 ctl_key = -1) { return ctlstate & ctl_key; }
        template<class ...Args>
        auto chord(si32 k, Args&&... mods)
        {
            if constexpr (sizeof...(mods)) return k == keybd::keycode && (meta(mods) && ...);
            else                           return k == keybd::keycode && !meta(hids::anyMod);
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
            #if defined(DEBUG)
            if (m.wheelsi)
            {
                auto s = m.ctlstat;
                auto alt     = s & hids::anyAlt ? 1 : 0;
                auto l_ctrl  = s & hids::LCtrl  ? 1 : 0;
                auto r_ctrl  = s & hids::RCtrl  ? 1 : 0;
                     if (l_ctrl && alt) netxs::_k2 += m.wheelsi > 0 ? 1 : -1; // LCtrl + Alt t +Wheel.
                else if (l_ctrl)        netxs::_k0 += m.wheelsi > 0 ? 1 : -1; // LCtrl+Wheel.
                else if (alt)           netxs::_k1 += m.wheelsi > 0 ? 1 : -1; // Alt+Wheel.
                else if (r_ctrl)        netxs::_k3 += m.wheelsi > 0 ? 1 : -1; // RCtrl+Wheel.
            }
            #endif
            disabled = faux;
            ctlstate = m.ctlstat;
            mouse::update(m, idmap);
        }
        void take(syskeybd& k)
        {
            if (k.keycode == key::config) // Receive three layout related values coded as codepoints: nullkey keycode, '/' keycode+mods, '?' keycode+mods.
            {
                auto i = utf::cpit{ k.cluster };
                keybd::nullkey = i.next().cdpoint;
                auto slash     = i.next().cdpoint;
                auto quest     = i.next().cdpoint;
                other_key = build_other_key(slash, quest);
            }
            else
            {
                tooltip_stop = true;
                ctlstate = k.ctlstat;
                keybd::update(k);
            }
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
        void pass(sptr object, fp2d offset, bool relative = faux)
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
                    auto saved_start = mouse::start;
                    mouse::start = start_id;
                    last->SIGNAL(tier::release, events::notify::mouse::leave, *this);
                    mouse::start = saved_start;
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
                tooltip_set = faux;
                boss.SIGNAL(tier::release, events::notify::mouse::enter, *this);
                mouse_leave(mouse::hover, start_l);
                mouse::hover = boss.id;
            }
        }
        void deactivate()
        {
            mouse::load_button_state(0);
            mouse::m_sys.buttons = {};
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
        void fire(hint new_cause, si32 new_index = mouse::noactive)
        {
            if (disabled) return;

            alive = true;
            mouse::index = new_index;
            mouse::cause = new_cause;
            mouse::coord = mouse::prime;
            mouse::nodbl = faux;

            auto offset = idmap.coor();
            if (mouse::swift)
            {
                auto next = bell::getref<base>(mouse::swift);
                if (next)
                {
                    redirect_mouse_focus(*next);
                    pass<tier::release>(next, offset, true);

                    if (alive && !captured()) // Pass unhandled event to the gate.
                    {
                        owner.bell::template signal<tier::release>(new_cause, *this);
                    }
                }
                else mouse::setfree();
            }
            else
            {
                if (!tooltip_stop) tooltip_recalc(new_cause);
                owner.bell::template signal<tier::preview>(new_cause, *this);

                if (!alive) return;

                auto next = idmap.link(mouse::coord);
                if (next != owner.id)
                {
                    relay = next;
                    pass<tier::preview>(bell::getref<base>(next), offset, true);
                    relay = 0;

                    if (!alive) return;
                }

                owner.bell::template signal<tier::release>(new_cause, *this); // Pass unhandled event to the gate.
            }
        }
        bool fire_fast()
        {
            if (disabled) return true;
            alive = true;
            auto next_id = mouse::swift ? mouse::swift
                                        : idmap.link(m_sys.coordxy);
            if (next_id != owner.id)
            {
                if (auto next_ptr = bell::getref<base>(next_id))
                {
                    auto& next = *next_ptr;
                    auto  temp = m_sys.coordxy;
                    m_sys.coordxy += idmap.coor();
                    next.global(m_sys.coordxy);
                    next.SIGNAL(tier::release, events::device::mouse::on, *this);
                    m_sys.coordxy = temp;
                    if (!alive) // Clear one-shot events on success.
                    {
                        m_sys.wheelfp = {};
                        m_sys.wheelsi = {};
                        m_sys.hzwheel = {};
                    }
                }
                else if (mouse::swift == next_id) mouse::setfree();
            }
            if (m_sav.changed != m_sys.changed) m_sav = m_sys;
            return !alive;
        }
        void fire_keybd()
        {
            alive = true;
            owner.SIGNAL(tier::preview, hids::events::keybd::key::post, *this);
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
        text interpret(bool decckm)
        {
            static auto alone_key = build_alone_key();
            static auto shift_key = build_shift_key();

            if (keybd::pressed)
            {
                auto s = hids::ctlstate;
                auto v = keybd::keycode & -2; // Generic keys only
                auto c = keybd::cluster.empty() ? 0 : keybd::cluster.front();

                if (s & hids::LCtrl && s & hids::RAlt) // This combination is already translated.
                {
                    s &= ~(hids::LCtrl | hids::RAlt);
                }

                auto shift = s & hids::anyShift ? hids::anyShift : 0;
                auto alt   = s & hids::anyAlt   ? hids::anyAlt   : 0;
                auto ctrl  = s & hids::anyCtrl  ? hids::anyCtrl  : 0;
                if (shift || alt || ctrl)
                {
                    if (ctrl && alt) // c == 0 for ctrl+alt+key combinationsons on windows.
                    {
                        if (c == 0) // Chars and vkeys for ' '(0x20),'A'-'Z'(0x41-5a) are the same on windows.
                        {
                                 if (v >= key::KeyA  && v <= key::KeyZ)      return "\033"s + (char)((0x41 + (v - key::KeyA) / 2) & 0b00011111);//generate('\033', (wchr)( a  & 0b00011111)); // Alt causes to prepend '\033'. Ctrl trims by 0b00011111.
                            else if (v == key::Space || v == keybd::nullkey) return "\033\0"s;  //'\033' + (wchr)('@' & 0b00011111)); // Map ctrl+alt+@ to ^[^@;
                        }
                        else if (c == 0x20 || (c >= 'A' && c <= 'Z')) return "\033"s + (char)(c & 0b00011111);//generate('\033', (wchr)( a  & 0b00011111)); // Alt causes to prepend '\033'. Ctrl trims by 0b00011111.
                    }

                    if (auto it_shift = shift_key.find(v); it_shift != shift_key.end())
                    {
                        auto& mods = *++(it_shift->second.rbegin());
                        mods = '1';
                        if (shift) mods += 1;
                        if (alt  ) mods += 2;
                        if (ctrl ) mods += 4;
                        return it_shift->second;
                    }
                    else if (auto it_other = other_key.find(v | (shift | alt | ctrl) << 8); it_other != other_key.end())
                    {
                        return it_other->second;
                    }
                    else if (!ctrl &&  alt && c) return text{ '\033' + keybd::cluster };
                    else if ( ctrl && !alt)
                    {
                             if (c == 0x20 || (c == 0x00 && v == keybd::nullkey)) return text(1, '@' & 0b00011111); // Detect ctrl+@ and ctrl+space.
                        else if (c == 0x00 && (v >= key::KeyA && v <= key::KeyZ)) return text(1, (0x41 + (v - key::KeyA) / 2) & 0b00011111); // Emulate ctrl+key mapping to C0 if current kb layout does not contain it.
                    }
                }

                if (auto it_alone = alone_key.find(v); it_alone != alone_key.end())
                {
                    if (v >= key::End && v <= key::DownArrow) it_alone->second[1] = decckm ? 'O' : '[';
                    return it_alone->second;
                }
                else if (c) return keybd::cluster;
            }
            return text{};
        }
    };
}