// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

#include "baseui.hpp"

namespace netxs::events::userland
{
    namespace hids
    {
        EVENTPACK( input::events, netxs::events::userland::seed::input )
        {
            EVENT_XS( die      , input::hids ), // release::global: Notify about the mouse controller is gone. Signal to delete gears inside dtvt-objects.
            EVENT_XS( halt     , input::hids ), // release::global: Notify about the mouse controller is outside.
            EVENT_XS( clipboard, input::hids ), // release/request: Set/get clipboard data.
            EVENT_XS( invite   , input::hids ), // release: Notify about the mouse controller is registered.
            GROUP_XS( keybd    , input::hids ), // Keybd related events.
            GROUP_XS( focus    , input::foci ), // Focus related events.
            GROUP_XS( device   , input::hids ), // Primary device event group for fast forwarding.

            SUBSET_XS( keybd )
            {
                EVENT_XS( post, input::hids ),
            };
            SUBSET_XS( focus )
            {
                EVENT_XS( hop, input::foci ), // request: Switch focus branch to seed.item.
                EVENT_XS( cut, input::foci ), // request: Unfocus and delete downstream (to inside) focus route.
                EVENT_XS( add, input::foci ), // request: Initiate focus setting toward outside (used by gui and dtvt).
                EVENT_XS( rem, input::foci ), // request: Initiate focus unsetting toward outside (used by gui and dtvt).
                EVENT_XS( dup, input::foci ), // request: Make a focus tree copy from default.
                GROUP_XS( set, input::foci ), // Release: Set on/off focus toward inside; preview: Set on/off focus toward outside.

                SUBSET_XS( set )
                {
                    EVENT_XS( on , input::foci ), // release: Set focus toward inside; preview: Set focus toward outside.
                    EVENT_XS( off, input::foci ), // release: Unset focus toward inside; preview: Unset focus toward outside.
                };
            };
            SUBSET_XS( device )
            {
                GROUP_XS( mouse, input::hids ), // release: Primary mouse event for fast forwarding.

                SUBSET_XS( mouse )
                {
                    EVENT_XS( on, input::hids ),
                };
            };
        };
    }
}

namespace netxs::input
{
    using ui::sptr;
    using ui::wptr;
    using ui::base;
    using ui::face;
    using ui::page;

    namespace events = netxs::events::userland::hids;

    namespace key
    {
        static constexpr auto ExtendedKey = 0x0100; // ENHANCED_KEY
        static constexpr auto NumLockMode = 0x0020; // NUMLOCK_ON

        static constexpr auto _counter    = __COUNTER__ + 1;
        static constexpr auto released    = __COUNTER__ - _counter;
        static constexpr auto pressed     = __COUNTER__ - _counter;
        static constexpr auto repeated    = __COUNTER__ - _counter;
        static constexpr auto interrupted = __COUNTER__ - _counter;

        static constexpr auto generic_sign   = 0xF0; // generic event: 0b1111'xxxx  xxxx=4-bit tier
        static constexpr auto scancode_sign  = 0x80; // virtcode: 0x00   scancode: 0x80
        static constexpr auto unpressed_sign = 0x40; //  Pressed: 0x00   Released: 0x40
        static constexpr auto cluster_sign   = 0x20;
        static constexpr auto mouse_sign     = 0x10;

        struct layout
        {
            static constexpr auto _counter = __COUNTER__ + 1;
            static constexpr auto undef    = __COUNTER__ - _counter;
            static constexpr auto qwerty   = __COUNTER__ - _counter;
            static constexpr auto qwertz   = __COUNTER__ - _counter;
            static constexpr auto azerty   = __COUNTER__ - _counter;
            static constexpr auto dvorak   = __COUNTER__ - _counter;
            static constexpr auto colemak  = __COUNTER__ - _counter;
            static constexpr auto bepo     = __COUNTER__ - _counter;
        };

        struct klid // Latin-based Keyboard Layouts (20 klids).
        {
            static constexpr auto k00000409 = 0x00000409; // English (US)        | United States Standard            | `qwerty`
            static constexpr auto k00000407 = 0x00000407; // German (Germany)    | Germany Standard                  | `qwertz`
            static constexpr auto k00010407 = 0x00010407; // German (Germany)    | Germany (IBM)                     | `qwertz`
            static constexpr auto k00000C07 = 0x00000C07; // German (Austria)    | Austrian Standard                 | `qwertz`
            static constexpr auto k0000040C = 0x0000040C; // French (France)     | France Standard                   | `azerty`
            static constexpr auto k0000080C = 0x0000080C; // French (Belgium)    | Belgian Standard                  | `azerty`
            static constexpr auto k0005040C = 0x0005040C; // French (France)     | French BÉPO Layout                | `bepo`
            static constexpr auto k00010409 = 0x00010409; // English (US)        | United States-Dvorak              | `dvorak`
            static constexpr auto k00020409 = 0x00020409; // English (US)        | United States-Dvorak (Left hand)  | `dvorak`
            static constexpr auto k00030409 = 0x00030409; // English (US)        | United States-Dvorak (Right hand) | `dvorak`
            static constexpr auto k00060409 = 0x00060409; // English (US)        | Colemak NATIVE (Win 11 24H2+)     | `colemak`
            static constexpr auto k00000405 = 0x00000405; // Czech (Czechia)     | Czech QWERTZ                      | `qwertz`
            static constexpr auto k00010405 = 0x00010405; // Czech (Czechia)     | Czech QWERTY                      | `qwerty`
            static constexpr auto k0000041B = 0x0000041B; // Slovak (Slovakia)   | Slovak QWERTZ                     | `qwertz`
            static constexpr auto k0001041B = 0x0001041B; // Slovak (Slovakia)   | Slovak QWERTY                     | `qwerty`
            static constexpr auto k0000040E = 0x0000040E; // Hungarian (Hungary) | Hungarian Standard                | `qwertz`
            static constexpr auto k0000041A = 0x0000041A; // Croatian            | Croatian Standard (South Slavic)  | `qwertz`
            static constexpr auto k00000424 = 0x00000424; // Slovenian           | Slovenian Standard (South Slavic) | `qwertz`
            static constexpr auto k00000415 = 0x00000415; // Polish (Poland)     | Polish (Programmers)              | `qwerty`
            static constexpr auto k00000418 = 0x00000418; // Romanian            | Romanian Standard                 | `qwertz`
        };
        auto get_layout_type(si32 klid)
        {
            if (klid == 0x00000409 || klid == 0) return layout::qwerty;
            if (klid == 0x00010409) return layout::dvorak; // US Dvorak Standard
            if (klid == 0x00020409) return layout::dvorak; // US Dvorak Left-Hand
            if (klid == 0x00030409) return layout::dvorak; // US Dvorak Right-Hand
            if (klid == 0x00000419) return layout::undef;
            if (klid == 0x00000405 || klid == 0x00010405) return layout::qwertz; // Czech
            if (klid == 0x00000407 || klid == 0x00010407) return layout::qwertz; // German (Austria)
            if (klid == 0x0000040c || klid == 0x0000080c) return layout::azerty; // French (Belgium)
            if (klid == 0x0005040c)                       return layout::bepo;   // French BEPO
            if (klid == 0x0000040e)                       return layout::qwertz; // Hungarian
            if (klid == 0x0000041a)                       return layout::qwertz; // Croatian
            if (klid == 0x0000041b)                       return layout::qwertz; // Slovak
            return layout::qwerty;
        }

        // Notes:
        //  IsoLevel5Shift: 5th-level of kb layout (mathematical signs, Greek letters). Physical keyboards don't have this key; in Linux, it's usually remapped to Caps Lock or the right Ctrl key.
        //  Hyper:          Users specifically create Hyper (for example, by remapping Caps Lock) to bind hotkeys, which are guaranteed to not interact with anything.
        #define key_list \
            /*Id   Index Vkey  Scan  KLID     CS          Mask  CS KLID VK SC  I  Name                   GenericName      KKP base,suffix,ascii,w\ctrl*/\
            X(0,      0,    0,    0, 0x00000,           0, 0x0000'00000'00'FF, 1, undef                , "undef"           , 0    , 'u', -1    , -1    )\
            X(1,      0, 0xFF, 0xFF, 0x00409,           0, 0x0100'00000'FF'FF, 0, config               , "config"          , 0    , 'u', -1    , -1    )\
            X(2,   0xA2, 0x11, 0x1D, 0x00409,           0, 0x0100'00000'00'FF, 0, LeftCtrl             , "Ctrl"            , 57442, 'u', -1    , -1    )\
            X( 3,  0xA3, 0x11, 0x1D, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, RightCtrl            , "Ctrl"            , 57448, 'u', -1    , -1    )\
            X(4,   0xA4, 0x12, 0x38, 0x00409,           0, 0x0100'00000'00'FF, 0, LeftAlt              , "Alt"             , 57443, 'u', -1    , -1    )\
            X( 5,  0xA5, 0x12, 0x38, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, RightAlt             , "Alt"             , 57449, 'u', -1    , -1    )\
            X(6,   0xA0, 0x10, 0x2A, 0x00409,           0, 0x0000'00000'FF'FF, 0, LeftShift            , "Shift"           , 57441, 'u', -1    , -1    )\
            X( 7,  0xA1, 0x10, 0x36, 0x00409,           0, 0x0000'00000'FF'FF, 0, RightShift           , "Shift"           , 57447, 'u', -1    , -1    )\
            X(8,   0x5B, 0x5B, 0x5B, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, LeftSuper            , "Super"           , 57444, 'u', -1    , -1    )\
            X( 9,  0x5C, 0x5C, 0x5C, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, RightSuper           , "Super"           , 57450, 'u', -1    , -1    )\
            X(10,  0x5B, 0x5B, 0x5B, 0x00409,           0, 0x0100'00000'00'FF, 0, LeftHyper            , "Hyper"           , 57445, 'u', -1    , -1    )\
            X( 11, 0x5C, 0x5C, 0x5C, 0x00409,           0, 0x0100'00000'00'FF, 0, RightHyper           , "Hyper"           , 57451, 'u', -1    , -1    )\
            X(12,  0x5D, 0x5D, 0x5D, 0x00409, ExtendedKey, 0x0000'00000'00'FF, 0, Apps                 , "Apps"            , 57363, 'u', -1    , -1    )\
            X(14,  0x90, 0x90, 0x45, 0x00409,           0, 0x0000'00000'00'FF, 0, NumLock              , "NumLock"         , 57360, 'u', -1    , -1    )\
            X(16,  0x14, 0x14, 0x3A, 0x00409,           0, 0x0100'00000'00'FF, 0, CapsLock             , "CapsLock"        , 57358, 'u', -1    , -1    )\
            X(18,  0x91, 0x91, 0x45, 0x00409,           0, 0x0100'00000'00'FF, 0, ScrollLock           , "ScrollLock"      , 57359, 'u', -1    , -1    )\
            X(20,  0x14, 0x14, 0x3A, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, AltGR                , "AltGr"           , 57453, 'u', -1    , -1    )/*IsoLevel3Shift*/\
            X(22,  0x91, 0x91, 0x45, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, IsoLevel5Shift       , "IsoLevel5Shift"  , 57454, 'u', -1    , -1    )\
            X(24,  0x1B, 0x1B, 0x01, 0x00409,           0, 0x0000'00000'00'FF, 1, Esc                  , "Esc"             , 27   , 'u', '\x1b', '\x1b')\
            X(26,  0x20, 0x20, 0x39, 0x00409,           0, 0x0000'00000'00'FF, 1, Space                , "Space"           , 32   , 'u', '\x20', '\0'  )\
            X(28,  0x08, 0x08, 0x0E, 0x00409,           0, 0x0000'00000'00'FF, 1, Backspace            , "Backspace"       , 127  , 'u', '\x7f', '\x08')\
            X(30,  0x09, 0x09, 0x0F, 0x00409,           0, 0x0000'00000'00'FF, 1, Tab                  , "Tab"             , 9    , 'u', '\x09', '\x09')\
            X(32,  0x03, 0x03, 0x46, 0x00409,           0, 0x0000'00000'FF'FF, 1, Break                , "Break"           , 0    , 'u', '\x1a', '\x03')\
            X(34,  0x13, 0x13, 0x45, 0x00409,           0, 0x0000'00000'FF'FF, 0, Pause                , "Pause"           , 57362, 'u', '\x1a', '\x03')\
            X(36,  0x29, 0x29,    0, 0x00409,           0, 0x0000'00000'00'FF, 0, Select               , "Select"          , 0    , 'u', -1    , -1    )\
            X(38,  0x2C, 0x2C, 0x54, 0x00409,           0, 0x0000'00000'FF'FF, 1, SysRq                , "SysRq"           , 0    , 'u', -1    , '\x03')\
            X(40,  0x2C, 0x2C, 0x37, 0x00409, ExtendedKey, 0x0100'00000'FF'FF, 0, PrintScreen          , "PrintScreen"     , 57361, 'u', -1    , -1    )\
            X(42,  0x0D, 0x0D, 0x1C, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyEnter             , "Enter"           , 13   , 'u', '\x0d', '\x0a')\
            X( 43, 0x0D, 0x0D, 0x1C, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 1, NumpadEnter          , "Enter"           , 57414, 'u', '\x0d', '\x0a')\
            X(44,  0x21, 0x21, 0x49, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 1, KeyPageUp            , "PageUp"          , 5    , '~', -1    , -1    )\
            X( 45, 0x21, 0x21, 0x49, 0x00409,           0, 0x0100'00000'00'FF, 1, NumpadPageUp         , "PageUp"          , 57421, 'u', -1    , -1    )\
            X(46,  0x22, 0x22, 0x51, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 1, KeyPageDown          , "PageDown"        , 6    , '~', -1    , -1    )\
            X( 47, 0x22, 0x22, 0x51, 0x00409,           0, 0x0100'00000'00'FF, 1, NumpadPageDown       , "PageDown"        , 57422, 'u', -1    , -1    )\
            X(48,  0x23, 0x23, 0x4F, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 1, KeyEnd               , "End"             , 8    , '~', -1    , -1    )/*don't reorder*/\
            X( 49, 0x23, 0x23, 0x4F, 0x00409,           0, 0x0100'00000'00'FF, 1, NumpadEnd            , "End"             , 57424, 'u', -1    , -1    )/*don't reorder*/\
            X(50,  0x24, 0x24, 0x47, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 1, KeyHome              , "Home"            , 7    , '~', -1    , -1    )/*don't reorder*/\
            X( 51, 0x24, 0x24, 0x47, 0x00409,           0, 0x0100'00000'00'FF, 1, NumpadHome           , "Home"            , 57423, 'u', -1    , -1    )/*don't reorder*/\
            X(52,  0x25, 0x25, 0x4B, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 1, KeyLeftArrow         , "LeftArrow"       , 1    , 'D', -1    , -1    )/*don't reorder*/\
            X( 53, 0x25, 0x25, 0x4B, 0x00409,           0, 0x0100'00000'00'FF, 1, NumpadLeftArrow      , "LeftArrow"       , 57417, 'u', -1    , -1    )/*don't reorder*/\
            X(54,  0x26, 0x26, 0x48, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 1, KeyUpArrow           , "UpArrow"         , 1    , 'A', -1    , -1    )/*don't reorder*/\
            X( 55, 0x26, 0x26, 0x48, 0x00409,           0, 0x0100'00000'00'FF, 1, NumpadUpArrow        , "UpArrow"         , 57419, 'u', -1    , -1    )/*don't reorder*/\
            X(56,  0x27, 0x27, 0x4D, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 1, KeyRightArrow        , "RightArrow"      , 1    , 'C', -1    , -1    )/*don't reorder*/\
            X( 57, 0x27, 0x27, 0x4D, 0x00409,           0, 0x0100'00000'00'FF, 1, NumpadRightArrow     , "RightArrow"      , 57418, 'u', -1    , -1    )/*don't reorder*/\
            X(58,  0x28, 0x28, 0x50, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 1, KeyDownArrow         , "DownArrow"       , 1    , 'B', -1    , -1    )/*don't reorder*/\
            X( 59, 0x28, 0x28, 0x50, 0x00409,           0, 0x0100'00000'00'FF, 1, NumpadDownArrow      , "DownArrow"       , 57420, 'u', -1    , -1    )/*don't reorder*/\
            X(60,  0x30, 0x30, 0x0B, 0x00409,           0, 0x0000'00000'FF'FF, 1, Key0                 , "0"               , 48   , 'u', '0'   , '0'   )\
            X( 61, 0x60, 0x60, 0x52, 0x00409, NumLockMode, 0x0000'00000'FF'FF, 1, Numpad0              , "0"               , 57399, 'u', '0'   , '0'   )\
            X(62,  0x31, 0x31, 0x02, 0x00409,           0, 0x0000'00000'FF'FF, 1, Key1                 , "1"               , 49   , 'u', '1'   , '1'   )\
            X( 63, 0x61, 0x61, 0x4F, 0x00409, NumLockMode, 0x0000'00000'FF'FF, 1, Numpad1              , "1"               , 57400, 'u', '1'   , '1'   )\
            X(64,  0x32, 0x32, 0x03, 0x00409,           0, 0x0000'00000'FF'FF, 1, Key2                 , "2"               , 50   , 'u', '2'   , '\0'  )\
            X( 65, 0x62, 0x62, 0x50, 0x00409, NumLockMode, 0x0000'00000'FF'FF, 1, Numpad2              , "2"               , 57401, 'u', '2'   , '\0'  )\
            X(66,  0x33, 0x33, 0x04, 0x00409,           0, 0x0000'00000'FF'FF, 1, Key3                 , "3"               , 51   , 'u', '3'   , '\x1b')\
            X( 67, 0x63, 0x63, 0x51, 0x00409, NumLockMode, 0x0000'00000'FF'FF, 1, Numpad3              , "3"               , 57402, 'u', '3'   , '\x1b')\
            X(68,  0x34, 0x34, 0x05, 0x00409,           0, 0x0000'00000'FF'FF, 1, Key4                 , "4"               , 52   , 'u', '4'   , '\x1c')\
            X( 69, 0x64, 0x64, 0x4B, 0x00409, NumLockMode, 0x0000'00000'FF'FF, 1, Numpad4              , "4"               , 57403, 'u', '4'   , '\x1c')\
            X(70,  0x35, 0x35, 0x06, 0x00409,           0, 0x0000'00000'FF'FF, 1, Key5                 , "5"               , 53   , 'u', '5'   , '\x1d')\
            X( 71, 0x65, 0x65, 0x4C, 0x00409, NumLockMode, 0x0000'00000'FF'FF, 1, Numpad5              , "5"               , 57404, 'u', '5'   , '\x1d')\
            X(72,  0x36, 0x36, 0x07, 0x00409,           0, 0x0000'00000'FF'FF, 1, Key6                 , "6"               , 54   , 'u', '6'   , '\x1e')\
            X( 73, 0x66, 0x66, 0x4D, 0x00409, NumLockMode, 0x0000'00000'FF'FF, 1, Numpad6              , "6"               , 57405, 'u', '6'   , '\x1e')\
            X(74,  0x37, 0x37, 0x08, 0x00409,           0, 0x0000'00000'FF'FF, 1, Key7                 , "7"               , 55   , 'u', '7'   , '\x1f')\
            X( 75, 0x67, 0x67, 0x47, 0x00409, NumLockMode, 0x0000'00000'FF'FF, 1, Numpad7              , "7"               , 57406, 'u', '7'   , '\x1f')\
            X(76,  0x38, 0x38, 0x09, 0x00409,           0, 0x0000'00000'FF'FF, 1, Key8                 , "8"               , 56   , 'u', '8'   , '\x7f')\
            X( 77, 0x68, 0x68, 0x48, 0x00409, NumLockMode, 0x0000'00000'FF'FF, 1, Numpad8              , "8"               , 57407, 'u', '8'   , '\x7f')\
            X(78,  0x39, 0x39, 0x0A, 0x00409,           0, 0x0000'00000'FF'FF, 1, Key9                 , "9"               , 57   , 'u', '9'   , '9'   )\
            X( 79, 0x69, 0x69, 0x49, 0x00409, NumLockMode, 0x0000'00000'FF'FF, 1, Numpad9              , "9"               , 57408, 'u', '9'   , '9'   )\
            X(80,  0x2D, 0x2D, 0x52, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 1, KeyInsert            , "Insert"          , 2    , '~', -1    , -1    )\
            X( 81, 0x2D, 0x2D, 0x52, 0x00409,           0, 0x0100'00000'00'FF, 1, NumpadInsert         , "Insert"          , 57425, 'u', -1    , -1    )\
            X(82,  0x2E, 0x2E, 0x53, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 1, KeyDelete            , "Delete"          , 3    , '~', -1    , -1    )\
            X( 83, 0x2E, 0x2E, 0x55, 0x00409,           0, 0x0100'00000'00'FF, 1, NumpadDelete         , "Delete"          , 57426, 'u', -1    , -1    )\
            X(84,  0x0C, 0x0C, 0x4C, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 1, KeyClear             , "Clear"           , 1    , 'E', -1    , -1    )\
            X( 85, 0x0C, 0x0C, 0x4C, 0x00409,           0, 0x0100'00000'00'FF, 1, NumpadClear          , "Clear"           , 57427, '~', -1    , -1    )\
            X(86,  0x6A, 0x6A, 0x09, 0x00409,           0, 0x0000'00000'FF'FF, 1, KeyMultiply          , "*"               , 0    , 'u', '*'   , '*'   )\
            X( 87, 0x6A, 0x6A, 0x37, 0x00409,           0, 0x0000'00000'FF'FF, 1, NumpadMultiply       , "*"               , 57411, 'u', '*'   , '*'   )\
            X(88,  0x6B, 0x6B, 0x0D, 0x00409,           0, 0x0000'00000'FF'FF, 1, KeyPlus              , "Plus"            , 43   , 'u', '+'   , '+'   )\
            X( 89, 0x6B, 0x6B, 0x4E, 0x00409,           0, 0x0000'00000'FF'FF, 1, NumpadPlus           , "Plus"            , 57413, 'u', '+'   , '+'   )\
            X(90,  0x6C, 0x6C,    0, 0x00409,           0, 0x0020'00000'00'FF, 1, KeySeparator         , "Separator"       , 0    , 'u', ','   , ','   )\
            X( 91, 0x6C, 0x6C,    0, 0x00409, NumLockMode, 0x0020'00000'00'FF, 1, NumpadSeparator      , "Separator"       , 57416, 'u', ','   , ','   )\
            X(92,  0xBD, 0xBD, 0x0C, 0x00409,           0, 0x0000'00000'00'FF, 1, KeyMinus             , "Minus"           , 45   , 'u', '-'   , '-'   )\
            X( 93, 0x6D, 0x6D, 0x4A, 0x00409,           0, 0x0000'00000'00'FF, 1, NumpadMinus          , "Minus"           , 57412, 'u', '-'   , '-'   )\
            X(94,  0xBE, 0xBE, 0x34, 0x00409,           0, 0x0000'00000'00'FF, 1, KeyPeriod            , "."               , 46   , 'u', '.'   , '.'   )\
            X( 95, 0x6E, 0x6E, 0x53, 0x00409, NumLockMode, 0x0000'00000'00'FF, 1, NumpadDecimal        , "."               , 57409, 'u', '.'   , '.'   )\
            X(96,  0xBF, 0xBF, 0x35, 0x00409,           0, 0x0000'00000'00'FF, 1, KeySlash             , "/"               , 47   , 'u', '/'   , '\x1f')\
            X( 97, 0x6F, 0x6F, 0x35, 0x00409, ExtendedKey, 0x0000'00000'00'FF, 1, NumpadSlash          , "/"               , 57410, 'u', '/'   , '\x1f')\
            X(98,  0xBB, 0xBB, 0x0D, 0x00409,           0, 0x0100'00000'00'FF, 1, Equal                , "="               , 61   , 'u', '='   , '='   )\
            X( 99, 0xBB, 0xBB, 0x0D, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 1, NumpadEqual          , "="               , 57415, 'u', '='   , '='   )\
            X(100, 0xDC, 0xDC, 0x2B, 0x00409,           0, 0x0000'00000'00'FF, 1, BackSlash            , "BackSlash"       , 92   , 'u', '\\'  , '\x1c')\
            X(102, 0xDB, 0xDB, 0x1A, 0x00409,           0, 0x0000'00000'00'FF, 1, OpenBracket          , "["               , 91   , 'u', '['   , '\x1b')\
            X(104, 0xDD, 0xDD, 0x1B, 0x00409,           0, 0x0000'00000'00'FF, 1, CloseBracket         , "]"               , 93   , 'u', ']'   , '\x1d')\
            X(106, 0xC0, 0xC0, 0x29, 0x00409,           0, 0x0000'00000'00'FF, 1, BackQuote            , "`"               , 96   , 'u', '`'   , '`'   )\
            X(108, 0xDE, 0xDE, 0x28, 0x00409,           0, 0x0000'00000'00'FF, 1, SingleQuote          , "'"               , 39   , 'u', '\''  , '\''  )\
            X(110, 0xBC, 0xBC, 0x33, 0x00409,           0, 0x0000'00000'00'FF, 1, Comma                , ","               , 44   , 'u', ','   , ','   )\
            X(112, 0xBA, 0xBA, 0x27, 0x00409,           0, 0x0000'00000'00'FF, 1, Semicolon            , ";"               , 59   , 'u', ';'   , ';'   )\
            X(114, 0xBC, 0xBC, 0x33, 0x00415,           0, 0x0000'FFFFF'00'FF, 1, LessThan             , "<"               , 60   , 'u', ';'   , ';'   )/*bepo  */\
            X(116, 0xBE, 0xBE, 0x34, 0x00418,           0, 0x0000'FFFFF'00'FF, 1, GreaterThan          , ">"               , 62   , 'u', ';'   , ';'   )/*bepo  */\
            X(118, 0xC0, 0xC0, 0x29, 0x00405,           0, 0x0000'FFFFF'00'FF, 1, DeadRing             , "°"               , 176  , 'u', '"'   , '"'   )/*qwertz Hungarian*/\
            X(120, 0x33, 0x33, 0x04, 0x5040C,           0, 0x0000'FFFFF'00'FF, 1, DoubleQuote          , "\""              , 34   , 'u', '"'   , '"'   )/*azerty*/\
            X(122, 0x33, 0x29, 0xC0, 0x0081A,           0, 0x0000'FFFFF'00'FF, 1, LowQuote             , "‚"               , 8218 , 'u', -1    , -1    )\
            X(124, 0x33, 0x29, 0xC0, 0x10418,           0, 0x0000'FFFFF'00'FF, 1, DoubleLowQuote       , "„"               , 8222 , 'u', -1    , -1    )\
            X(126, 0x35, 0x35, 0x06, 0x0040C,           0, 0x0000'FFFFF'00'FF, 1, OpenRoundBracket     , "("               , 40   , 'u', '('   , '('   )/*azerty*/\
            X(128, 0x38, 0x38, 0x2D, 0x5040C,           0, 0x0000'FFFFF'00'FF, 1, CloseRoundBracket    , ")"               , 41   , 'u', ')'   , ')'   )/*azerty*/\
            X(130, 0x36, 0x36, 0x07, 0x00407,           0, 0x0000'FFFFF'00'FF, 1, AtSign               , "@"               , 64   , 'u', '@'   , '\0'  )/*bepo  */\
            X(132, 0xDD, 0xDD, 0x0D, 0x00407,           0, 0x0000'FFFFF'00'FF, 1, Percent              , "%"               , 37   , 'u', '%'   , '%'   )/*bepo  */\
            X(134, 0xDF, 0xDF, 0x35, 0x00405,           0, 0x0000'FFFFF'00'FF, 1, Exclamation          , "!"               , 33   , 'u', '!'   , '!'   )/*azerty*/\
            X(136, 0xBF, 0xBF, 0x2B, 0x00407,           0, 0x0000'FFFFF'00'FF, 1, Hash                 , "#"               , 35   , 'u', '#'   , '#'   )/*qwertz*/\
            X(138, 0x31, 0x31, 0x02, 0x0040E,           0, 0x0000'FFFFF'00'FF, 1, Ampersand            , "&"               , 38   , 'u', '&'   , '&'   )/*azerty*/\
            X(140, 0x38, 0x38, 0x09, 0x0040E,           0, 0x0000'FFFFF'00'FF, 1, Underscore           , "_"               , 95   , 'u', '_'   , '\x1f')/*azerty*/\
            X(142, 0x32, 0x32, 0x03, 0x00407,           0, 0x0000'FFFFF'00'FF, 1, LeftGuillemet        , "«"               , 171  , 'u', -1    , -1    )/*bepo  */\
            X(144, 0x33, 0x33, 0x04, 0x00407,           0, 0x0000'FFFFF'00'FF, 1, RightGuillemet       , "»"               , 187  , 'u', -1    , -1    )/*bepo  */\
            X(146, 0x33, 0x33, 0x04, 0x00407,           0, 0x0000'FFFFF'00'FF, 1, Dollar               , "$"               , '$'  , 'u', '$'   , '$'   )\
            X(148, 0x33, 0x33, 0x04, 0x00407,           0, 0x0000'FFFFF'00'FF, 1, PoundSign            , "£"               , 163  , 'u', -1    , -1    )\
            X(150, 0x33, 0x33, 0x04, 0x00407,           0, 0x0000'FFFFF'00'FF, 1, Paragraph            , "§"               , 167  , 'u', -1    , -1    )\
            X(152, 0x33, 0x33, 0x04, 0x00407,           0, 0x0000'FFFFF'00'FF, 1, MicroSign            , "µ"               , 181  , 'u', -1    , -1    )\
            X(154, 0x33, 0x33, 0x04, 0x00407,           0, 0x0000'FFFFF'00'FF, 1, AeLigature           , "æ"               , 230  , 'u', -1    , -1    )\
            X(156, 0x33, 0x33, 0x04, 0x00407,           0, 0x0000'FFFFF'00'FF, 1, TildeO               , "õ"               , 245  , 'u', -1    , -1    )\
            X(158, 0x33, 0x33, 0x04, 0x00407,           0, 0x0000'FFFFF'00'FF, 1, StrokeO              , "ø"               , 248  , 'u', -1    , -1    )\
            X(160, 0x33, 0x33, 0x04, 0x00407,           0, 0x0000'FFFFF'00'FF, 1, CrossedD             , "đ"               , 273  , 'u', -1    , -1    )\
            X(162, 0x33, 0x33, 0x04, 0x00407,           0, 0x0000'FFFFF'00'FF, 1, RingA                , "å"               , 229  , 'u', -1    , -1    )\
            X(164, 0x33, 0x33, 0x04, 0x00407,           0, 0x0000'FFFFF'00'FF, 1, GraveO               , "ò"               , 242  , 'u', -1    , -1    )\
            X(166, 0x33, 0x33, 0x04, 0x00407,           0, 0x0000'FFFFF'00'FF, 1, AcuteC               , "ć"               , 263  , 'u', -1    , -1    )\
            X(168, 0x33, 0x33, 0x04, 0x00407,           0, 0x0000'FFFFF'00'FF, 1, GraveU               , "ù"               , 249  , 'u', -1    , -1    )\
            X(170, 0xDB, 0xDB, 0x2D, 0x00407,           0, 0x0000'FFFFF'00'FF, 1, Eszett               , "ß"               , 223  , 'u', -1    , -1    )/*qwertz*/\
            X(172, 0x38, 0x38, 0x09, 0x0040C,           0, 0x0000'FFFFF'00'FF, 1, AcuteA               , "á"               , 225  , 'u', -1    , -1    )/*qwertz Czech*/\
            X(174, 0x32, 0x32, 0x03, 0x0040C,           0, 0x0000'FFFFF'00'FF, 1, AcuteE               , "é"               , 233  , 'u', -1    , -1    )/*azerty*/\
            X(176, 0x39, 0x39, 0x0A, 0x0041B,           0, 0x0000'FFFFF'00'FF, 1, AcuteI               , "í"               , 237  , 'u', -1    , -1    )/*qwertz Czech*/\
            X(178, 0x38, 0x38, 0x09, 0x0041B,           0, 0x0000'FFFFF'00'FF, 1, AcuteO               , "ó"               , 243  , 'u', -1    , -1    )/*qwertz Hungarian*/\
            X(180, 0x35, 0x35, 0x06, 0x00405,           0, 0x0000'FFFFF'00'FF, 1, AcuteU               , "ú"               , 250  , 'u', -1    , -1    )/*qwertz Hungarian*/\
            X(182, 0x37, 0x37, 0x08, 0x00405,           0, 0x0000'FFFFF'00'FF, 1, AcuteY               , "ý"               , 253  , 'u', -1    , -1    )/*qwertz Czech*/\
            X(184, 0x34, 0x34, 0x05, 0x00405,           0, 0x0000'FFFFF'00'FF, 1, DoubleAcuteO         , "ő"               , 337  , 'u', -1    , -1    )/*qwertz Hungarian*/\
            X(186, 0x39, 0x39, 0x0A, 0x00405,           0, 0x0000'FFFFF'00'FF, 1, CedillaC             , "ç"               , 231  , 'u', -1    , -1    )/*azerty*/\
            X(188, 0x5A, 0x5A, 0x2C, 0x00405,           0, 0x0000'FFFFF'00'FF, 1, CircumflexE          , "ê"               , 234  , 'u', -1    , -1    )/*bepo  */\
            X(190, 0x32, 0x32, 0x03, 0x5040C,           0, 0x0000'FFFFF'00'FF, 1, CaronE               , "ě"               , 283  , 'u', -1    , -1    )/*qwertz Czech*/\
            X(192, 0x33, 0x33, 0x04, 0x0040C,           0, 0x0000'FFFFF'00'FF, 1, CaronS               , "š"               , 353  , 'u', -1    , -1    )/*qwertz Czech*/\
            X(194, 0x34, 0x34, 0x05, 0x0040E,           0, 0x0000'FFFFF'00'FF, 1, CaronC               , "č"               , 269  , 'u', -1    , -1    )/*qwertz Czech*/\
            X(196, 0x35, 0x35, 0x06, 0x00405,           0, 0x0000'FFFFF'00'FF, 1, CaronR               , "ř"               , 345  , 'u', -1    , -1    )/*qwertz Czech*/\
            X(198, 0x36, 0x36, 0x07, 0x0040E,           0, 0x0000'FFFFF'00'FF, 1, CaronZ               , "ž"               , 282  , 'u', -1    , -1    )/*qwertz Czech*/\
            X(200, 0x32, 0x32, 0x03, 0x0040E,           0, 0x0000'FFFFF'00'FF, 1, CaronL               , "ľ"               , 318  , 'u', -1    , -1    )/*qwertz Slovak*/\
            X(202, 0x35, 0x35, 0x06, 0x00405,           0, 0x0000'FFFFF'00'FF, 1, CaronT               , "ť"               , 357  , 'u', -1    , -1    )/*qwertz Slovak*/\
            X(204, 0x30, 0x30, 0x0B, 0x0040C,           0, 0x0000'FFFFF'00'FF, 1, GraveA               , "à"               , 224  , 'u', -1    , -1    )/*azerty*/\
            X(206, 0x37, 0x37, 0x08, 0x00405,           0, 0x0000'FFFFF'00'FF, 1, GraveE               , "è"               , 232  , 'u', -1    , -1    )/*azerty*/\
            X(208, 0xDE, 0xDE, 0x28, 0x00407,           0, 0x0000'FFFFF'00'FF, 1, UmlautA              , "ä"               , 228  , 'u', -1    , -1    )/*qwertz*/\
            X(210, 0xBF, 0xBF, 0x27, 0x5040C,           0, 0x0000'0FFFF'00'FF, 1, UmlautO              , "ö"               , 246  , 'u', -1    , -1    )/*qwertz*/\
            X(212, 0xBA, 0xBA, 0x1A, 0x5040C,           0, 0x0000'0FFFF'00'FF, 1, UmlautU              , "ü"               , 252  , 'u', -1    , -1    )/*qwertz*/\
            X(214, 0xBA, 0xBA, 0x27, 0x0040C,           0, 0x0000'FFFFF'00'FF, 1, RingU                , "ů"               , 367  , 'u', -1    , -1    )/*qwertz Czech*/\
            X(216, 0xDC, 0xDC, 0x29, 0x5040C,           0, 0x0000'FFFFF'00'FF, 0, DeadCircumflex       , "^"               , 94   , 'u', -1    , -1    )/*qwertz*/\
            X(218, 0xDD, 0xDD, 0x0D, 0x5040C,           0, 0x0000'FFFFF'00'FF, 0, DeadAcute            , "´"               , 180  , 'u', -1    , -1    )/*qwertz*/\
            X(220, 0xC0, 0xC0, 0x29, 0x0040C,           0, 0x0000'FFFFF'00'FF, 0, DeadGrave            , "`"               , 96   , 'u', -1    , -1    )/*bepo  */\
            X(222, 0xDD, 0xDD, 0x1A, 0x0040C,           0, 0x0000'FFFFF'00'FF, 0, DeadUmlaut           , "¨"               , 168  , 'u', -1    , -1    )/*azerty*/\
            X(224, 0x4E, 0x4E, 0x31, 0x0040C,           0, 0x0000'FFFFF'00'FF, 0, DeadTilde            , "~"               , 771  , 'u', -1    , -1    )/*bepo  */\
            X(226, 0xDC, 0xDC, 0x29, 0x0040E,           0, 0x0000'FFFFF'00'FF, 0, DeadCaron            , "ˇ"               , 780  , 'u', -1    , -1    )/*qwertz Hungarian*/\
            X(228, 0xDC, 0xDC, 0x29, 0x5040C,           0, 0x0000'FFFFF'00'FF, 0, DeadBreve            , "˘"               , 774  , 'u', -1    , -1    )/*bepo French*/\
            X(230, 0xDE, 0xDE, 0x28, 0x5040C,           0, 0x0000'FFFFF'00'FF, 0, DeadOgonek           , "˛"               , 731  , 'u', -1    , -1    )/*bepo French*/\
            X(232, 0x70, 0x70, 0x3B, 0x00409,           0, 0x0000'00000'00'FF, 1, F1                   , "F1"              , 11   , '~', -1    , -1    )\
            X(234, 0x71, 0x71, 0x3C, 0x00409,           0, 0x0000'00000'00'FF, 1, F2                   , "F2"              , 12   , '~', -1    , -1    )\
            X(236, 0x72, 0x72, 0x3D, 0x00409,           0, 0x0000'00000'00'FF, 1, F3                   , "F3"              , 13   , '~', -1    , -1    )\
            X(238, 0x73, 0x73, 0x3E, 0x00409,           0, 0x0000'00000'00'FF, 1, F4                   , "F4"              , 14   , '~', -1    , -1    )\
            X(240, 0x74, 0x74, 0x3F, 0x00409,           0, 0x0000'00000'00'FF, 1, F5                   , "F5"              , 15   , '~', -1    , -1    )\
            X(242, 0x75, 0x75, 0x40, 0x00409,           0, 0x0000'00000'00'FF, 1, F6                   , "F6"              , 17   , '~', -1    , -1    )\
            X(244, 0x76, 0x76, 0x41, 0x00409,           0, 0x0000'00000'00'FF, 1, F7                   , "F7"              , 18   , '~', -1    , -1    )\
            X(246, 0x77, 0x77, 0x42, 0x00409,           0, 0x0000'00000'00'FF, 1, F8                   , "F8"              , 19   , '~', -1    , -1    )\
            X(248, 0x78, 0x78, 0x43, 0x00409,           0, 0x0000'00000'00'FF, 1, F9                   , "F9"              , 20   , '~', -1    , -1    )\
            X(250, 0x79, 0x79, 0x44, 0x00409,           0, 0x0000'00000'00'FF, 1, F10                  , "F10"             , 21   , '~', -1    , -1    )\
            X(252, 0x7A, 0x7A, 0x57, 0x00409,           0, 0x0000'00000'00'FF, 1, F11                  , "F11"             , 23   , '~', -1    , -1    )\
            X(254, 0x7B, 0x7B, 0x5B, 0x00409,           0, 0x0000'00000'00'FF, 1, F12                  , "F12"             , 24   , '~', -1    , -1    )\
            X(256, 0x7C, 0x7C,    0, 0x00409,           0, 0x0000'00000'00'FF, 1, F13                  , "F13"             , 57376, 'u', -1    , -1    )\
            X(258, 0x7D, 0x7D,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, F14                  , "F14"             , 57377, 'u', -1    , -1    )\
            X(260, 0x7E, 0x7E,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, F15                  , "F15"             , 57378, 'u', -1    , -1    )\
            X(262, 0x7F, 0x7F,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, F16                  , "F16"             , 57379, 'u', -1    , -1    )\
            X(264, 0x80, 0x80,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, F17                  , "F17"             , 57380, 'u', -1    , -1    )\
            X(266, 0x81, 0x81,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, F18                  , "F18"             , 57381, 'u', -1    , -1    )\
            X(268, 0x82, 0x82,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, F19                  , "F19"             , 57382, 'u', -1    , -1    )\
            X(270, 0x83, 0x83,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, F20                  , "F20"             , 57383, 'u', -1    , -1    )\
            X(272, 0x84, 0x84,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, F21                  , "F21"             , 57384, 'u', -1    , -1    )\
            X(274, 0x85, 0x85,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, F22                  , "F22"             , 57385, 'u', -1    , -1    )\
            X(276, 0x86, 0x86,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, F23                  , "F23"             , 57386, 'u', -1    , -1    )\
            X(278, 0x87, 0x87,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, F24                  , "F24"             , 57387, 'u', -1    , -1    )\
            X(280, 0x7D, 0x7D,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 1, F25                  , "F25"             , 57388, 'u', -1    , -1    )\
            X(282, 0x7E, 0x7E,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 1, F26                  , "F26"             , 57389, 'u', -1    , -1    )\
            X(284, 0x7F, 0x7F,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 1, F27                  , "F27"             , 57390, 'u', -1    , -1    )\
            X(286, 0x80, 0x80,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 1, F28                  , "F28"             , 57391, 'u', -1    , -1    )\
            X(288, 0x81, 0x81,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 1, F29                  , "F29"             , 57392, 'u', -1    , -1    )\
            X(290, 0x82, 0x82,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 1, F30                  , "F30"             , 57393, 'u', -1    , -1    )\
            X(292, 0x83, 0x83,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 1, F31                  , "F31"             , 57394, 'u', -1    , -1    )\
            X(294, 0x84, 0x84,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 1, F32                  , "F32"             , 57395, 'u', -1    , -1    )\
            X(296, 0x85, 0x85,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 1, F33                  , "F33"             , 57396, 'u', -1    , -1    )\
            X(298, 0x86, 0x86,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 1, F34                  , "F34"             , 57397, 'u', -1    , -1    )\
            X(300, 0x87, 0x87,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 1, F35                  , "F35"             , 57398, 'u', -1    , -1    )\
            X(302, 0x41, 0x41,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyA                 , "A"               , 97   , 'u', 'a'   , '\x01')\
            X(304, 0x42, 0x42,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyB                 , "B"               , 98   , 'u', 'b'   , '\x02')\
            X(306, 0x43, 0x43,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyC                 , "C"               , 99   , 'u', 'c'   , '\x03')\
            X(308, 0x44, 0x44,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyD                 , "D"               , 100  , 'u', 'd'   , '\x04')\
            X(310, 0x45, 0x45,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyE                 , "E"               , 101  , 'u', 'e'   , '\x05')\
            X(312, 0x46, 0x46,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyF                 , "F"               , 102  , 'u', 'f'   , '\x06')\
            X(314, 0x47, 0x47,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyG                 , "G"               , 103  , 'u', 'g'   , '\x07')\
            X(316, 0x48, 0x48,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyH                 , "H"               , 104  , 'u', 'h'   , '\x08')\
            X(318, 0x49, 0x49,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyI                 , "I"               , 105  , 'u', 'i'   , '\x09')\
            X(320, 0x4A, 0x4A,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyJ                 , "J"               , 106  , 'u', 'j'   , '\x0a')\
            X(322, 0x4B, 0x4B,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyK                 , "K"               , 107  , 'u', 'k'   , '\x0b')\
            X(324, 0x4C, 0x4C,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyL                 , "L"               , 108  , 'u', 'l'   , '\x0c')\
            X(326, 0x4D, 0x4D,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyM                 , "M"               , 109  , 'u', 'm'   , '\x0d')\
            X(328, 0x4E, 0x4E,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyN                 , "N"               , 110  , 'u', 'n'   , '\x0e')\
            X(330, 0x4F, 0x4F,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyO                 , "O"               , 111  , 'u', 'o'   , '\x0f')\
            X(332, 0x50, 0x50,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyP                 , "P"               , 112  , 'u', 'p'   , '\x10')\
            X(334, 0x51, 0x51,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyQ                 , "Q"               , 113  , 'u', 'q'   , '\x11')\
            X(336, 0x52, 0x52,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyR                 , "R"               , 114  , 'u', 'r'   , '\x12')\
            X(338, 0x53, 0x53,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyS                 , "S"               , 115  , 'u', 's'   , '\x13')\
            X(340, 0x54, 0x54,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyT                 , "T"               , 116  , 'u', 't'   , '\x14')\
            X(342, 0x55, 0x55,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyU                 , "U"               , 117  , 'u', 'u'   , '\x15')\
            X(344, 0x56, 0x56,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyV                 , "V"               , 118  , 'u', 'v'   , '\x16')\
            X(346, 0x57, 0x57,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyW                 , "W"               , 119  , 'u', 'w'   , '\x17')\
            X(348, 0x58, 0x58,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyX                 , "X"               , 120  , 'u', 'x'   , '\x18')\
            X(350, 0x59, 0x59,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyY                 , "Y"               , 121  , 'u', 'y'   , '\x19')\
            X(352, 0x5A, 0x5A,    0, 0x00409,           0, 0x0100'00000'00'FF, 1, KeyZ                 , "Z"               , 122  , 'u', 'z'   , '\x1a')\
            X(354, 0x5F, 0x5F,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, Sleep                , "Sleep"           , 0    , 'u', -1    , -1    )\
            X(356, 0xB7, 0xB7,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, Calculator           , "Calculator"      , 0    , 'u', -1    , -1    )\
            X(368, 0x48, 0x48,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, Mail                 , "Mail"            , 0    , 'u', -1    , -1    )\
            X(360, 0xAD, 0xAD,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, MediaVolMute         , "MediaVolMute"    , 57440, 'u', -1    , -1    )\
            X(362, 0xAE, 0xAE,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, MediaVolDown         , "MediaVolDown"    , 57438, 'u', -1    , -1    )\
            X(364, 0xAF, 0xAF,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, MediaVolUp           , "MediaVolUp"      , 57439, 'u', -1    , -1    )\
            X(366, 0xB0, 0xB0,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, MediaNext            , "MediaNext"       , 57435, 'u', -1    , -1    )\
            X(368, 0xB1, 0xB1,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, MediaPrev            , "MediaPrev"       , 57436, 'u', -1    , -1    )\
            X(370, 0xB2, 0xB2,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, MediaStop            , "MediaStop"       , 57432, 'u', -1    , -1    )\
            X(372, 0xB2, 0xB2,    0, 0x00409,           0, 0x0100'00000'00'FF, 0, MediaPause           , "MediaPause"      , 57429, 'u', -1    , -1    )\
            X(374, 0xB3, 0xB3,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, MediaPlayPause       , "MediaPlayPause"  , 57430, 'u', -1    , -1    )\
            X(376, 0xB3, 0xB3,    0, 0x00409,           0, 0x0100'00000'00'FF, 0, MediaPlay            , "MediaPlay"       , 57428, 'u', -1    , -1    )\
            X(378, 0xB5, 0xB5,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, MediaSelect          , "MediaSelect"     , 0    , 'u', -1    , -1    )\
            X(380, 0xB8, 0xB8,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, MediaReverse         , "MediaReverse"    , 57431, 'u', -1    , -1    )\
            X(382, 0xB8, 0xB8,    0, 0x00409,           0, 0x0100'00000'00'FF, 0, MediaRecord          , "MediaRecord"     , 57437, 'u', -1    , -1    )\
            X(384, 0xB9, 0xB9,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, MediaFastForward     , "MediaFastForward", 57433, 'u', -1    , -1    )\
            X(386, 0xB9, 0xB9,    0, 0x00409,           0, 0x0100'00000'00'FF, 0, MediaRewind          , "MediaRewind"     , 57434, 'u', -1    , -1    )\
            X(388, 0xA6, 0xA6,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, BrowserBack          , "BrowserBack"     , 0    , 'u', -1    , -1    )\
            X(390, 0xA7, 0xA7,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, BrowserForward       , "BrowserForward"  , 0    , 'u', -1    , -1    )\
            X(392, 0xA8, 0xA8,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, BrowserRefresh       , "BrowserRefresh"  , 0    , 'u', -1    , -1    )\
            X(394, 0xA9, 0xA9,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, BrowserStop          , "BrowserStop"     , 0    , 'u', -1    , -1    )\
            X(396, 0xAA, 0xAA,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, BrowserSearch        , "BrowserSearch"   , 0    , 'u', -1    , -1    )\
            X(398, 0xAB, 0xAB,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, BrowserFavorites     , "BrowserFavorites", 0    , 'u', -1    , -1    )\
            X(400, 0xAC, 0xAC,    0, 0x00409, ExtendedKey, 0x0100'00000'00'FF, 0, BrowserHome          , "BrowserHome"     , 0    , 'u', -1    , -1    )\
            X(402, 0xFF, 0xFF, 0xFF, 0x00000, ExtendedKey, 0x0100'FFFFF'FF'FF, 0, lastKey              , "lastKey"         , 0    , 0  , -1    , -1    )
            // Max 12 bits for KeyId.
            static constexpr auto idbits = 12;

        #define X(KeyId, Index, Vkey, Scan, Klid, CtrlState, Mask, Input, Name, GenericName, KKPDef, KKPSuffix, KKPAscii, KKPCtl) \
            static constexpr auto Name = KeyId;
            key_list
        #undef X

        struct map
        {
            si64 hash; // map: Key hash.

            static auto& mask()
            {
                static auto m = std::vector<decltype(map::hash)>(input::key::lastKey);
                return m;
            }
            static auto& mask(si32 vk)
            {
                return mask()[std::clamp(vk, 0, input::key::lastKey - 1)];
            }
            static auto& data(si32 keycode)
            {
                struct key
                {
                    view name;
                    view generic;
                    si32 vkey;
                    si32 scan;
                    si32 klid;
                    si32 edit;
                    si32 KKPDef;
                    si32 KKPSuffix;
                    si32 KKPAscii;
                    si32 KKPCtl;
                };
                static auto data = std::vector<key>(input::key::lastKey);
                return data[std::clamp(keycode, 0, input::key::lastKey - 1)];
            }

            map(si32 vk, si32 sc, si32 cs, si32 klid)
                : hash{ (si64)(mask(vk) & (vk | (sc << 8) | (klid << 16) | ((si64)cs << 36))) }
            { }
            map(si32 vk, si32 sc, si32 klid, si32 cs, si64 keymask, view keyname, view generic_keyname, si32 doinput, si32 id,
                si32 KKPDef, si32 KKPSuffix, si32 KKPAscii, si32 KKPCtl)
            {
                mask(vk) = keymask;
                data(id) = { .name      = keyname,
                             .generic   = generic_keyname,
                             .vkey      = vk,
                             .scan      = sc,
                             .klid      = klid,
                             .edit      = doinput,
                             .KKPDef    = KKPDef,
                             .KKPSuffix = KKPSuffix,
                             .KKPAscii  = KKPAscii,
                             .KKPCtl    = KKPCtl };
                hash = (si64)(keymask & (vk | (sc << 8) | (klid << 16) | ((si64)cs << 36)));
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

        static const auto keymap = std::unordered_map<map, si32, map::hashproc>
        {
            #define X(KeyId, Index, Vkey, Scan, Klid, CtrlState, Mask, Input, Name, GenericName, KKPDef, KKPSuffix, KKPAscii, KKPCtl) \
                { map{ Vkey, Scan, Klid, CtrlState, Mask, #Name, GenericName, Input, KeyId, KKPDef, KKPSuffix, KKPAscii, KKPCtl }, Name },
                key_list
            #undef X
        };
        static const auto kkpmap = std::unordered_map<si32, si32>
        {
            #define X(KeyId, Index, Vkey, Scan, Klid, CtrlState, Mask, Input, Name, GenericName, KKPDef, KKPSuffix, KKPAscii, KKPCtl) \
                { KKPDef | (KKPSuffix << 16), KeyId },
                key_list
            #undef X
        };
        static const auto specific_names = utf::unordered_map<text, si32>
        {
            #define X(KeyId, Index, Vkey, Scan, Klid, CtrlState, Mask, Input, Name, GenericName, KKPDef, KKPSuffix, KKPAscii, KKPCtl) \
                { utf::to_lower(#Name), KeyId },
                key_list
            #undef X
        };
        static const auto generic_names = utf::unordered_map<text, si32>
        {
            #define X(KeyId, Index, Vkey, Scan, Klid, CtrlState, Mask, Input, Name, GenericName, KKPDef, KKPSuffix, KKPAscii, KKPCtl) \
                { utf::to_lower(GenericName), KeyId & -2 },
                key_list
            #undef X
        };
        static constexpr auto ctrl_lut = [] // Ctrl+Key lookup table.
        {
            auto lut = std::array<si32, 128>{ -1 };
            auto pairs = std::to_array<std::pair<byte, byte>>(
            {
                { 127, 8 }, { 8, 127 },
                { 9, 9 }, { 13, 10 }, { 27, 27 },
                { '`', '`' }, { '!', '!' }, { '"', '"' }, { '#', '#' }, { '$', '$' }, { '%', '%' }, { '&', '&' },
                { '\'', '\'' },
                { '(', '(' }, { ')', ')' }, { '*', '*' }, { '+', '+' }, { '=', '=' }, { ',', ',' }, { '.', '.' },
                { ':', ':' }, { ';', ';' }, { '<', '<' }, { '>', '>' },
                { '@', 0   }, { ' ', 0   },
                { 'a', 1   }, { 'f', 6   }, { 'k', 11  }, { 'p', 16  }, { 'u', 21  },
                { 'b', 2   }, { 'g', 7   }, { 'l', 12  }, { 'q', 17  }, { 'v', 22  },
                { 'c', 3   }, { 'h', 8   }, { 'm', 13  }, { 'r', 18  }, { 'w', 23  },
                { 'd', 4   }, { 'i', 9   }, { 'n', 14  }, { 's', 19  }, { 'x', 24  },
                { 'e', 5   }, { 'j', 10  }, { 'o', 15  }, { 't', 20  }, { 'y', 25  }, { 'z', 26  },
                { 'A', 1   }, { 'F', 6   }, { 'K', 11  }, { 'P', 16  }, { 'U', 21  },
                { 'B', 2   }, { 'G', 7   }, { 'L', 12  }, { 'Q', 17  }, { 'V', 22  },
                { 'C', 3   }, { 'H', 8   }, { 'M', 13  }, { 'R', 18  }, { 'W', 23  },
                { 'D', 4   }, { 'I', 9   }, { 'N', 14  }, { 'S', 19  }, { 'X', 24  },
                { 'E', 5   }, { 'J', 10  }, { 'O', 15  }, { 'T', 20  }, { 'Y', 25  }, { 'Z', 26  },
                { '[', 27  }, { '{', 27  }, // { '{', '{' }, ???
                { '\\',28  }, { '|', 28  }, // { '|', '|' }, ???
                { ']', 29  }, { '}', 29  }, // { '}', '}' }, ???
                { '~', 30  }, { '^', 30  },
                { '/', 31  }, { '_', 31  },
                { '-', 31 }, //  { '-', '-' }, ???
                { '?', 127 },
                { '0', '0' }, { '2', 0   }, { '4', 28  }, { '6', 30  }, { '8', 127 },
                { '1', '1' }, { '3', 27  }, { '5', 29  }, { '7', 31  }, { '9', '9' },
            });
            for (auto [asc, c] : pairs) lut[asc] = c;
            return lut;
        }();
        text interpret_ctrl(auto& k, bool ctrl, bool shift)
        {
            auto crop = text{};
            auto asc = utf::to_code(shift ? k.shifted : k.unshift);
            if (ctrl && asc > 0 && asc < 128)
            {
                auto converted_char = input::key::ctrl_lut[asc];
                if (converted_char != -1)
                {
                    crop += (char)converted_char;
                }
            }
            return crop;
        }

        #define mouse_list \
            X(MouseAny           , 0x00, 0)\
            X(MouseDown          , 0x01, 0)X(LeftDown          , 0x1, 0b001)X(RightDown          , 0x1, 0b010)X(LeftRightDown          , 0x1, 0b011)X(MiddleDown          , 0x1, 0b100)\
            X(MouseUp            , 0x02, 0)X(LeftUp            , 0x2, 0b001)X(RightUp            , 0x2, 0b010)X(LeftRightUp            , 0x2, 0b011)X(MiddleUp            , 0x2, 0b100)\
            X(MouseClick         , 0x03, 0)X(LeftClick         , 0x3, 0b001)X(RightClick         , 0x3, 0b010)X(LeftRightClick         , 0x3, 0b011)X(MiddleClick         , 0x3, 0b100)\
            X(MouseDoubleClick   , 0x04, 0)X(LeftDoubleClick   , 0x4, 0b001)X(RightDoubleClick   , 0x4, 0b010)X(LeftRightDoubleClick   , 0x4, 0b011)X(MiddleDoubleClick   , 0x4, 0b100)\
            X(MouseDoublePress   , 0x05, 0)X(LeftDoublePress   , 0x5, 0b001)X(RightDoublePress   , 0x5, 0b010)X(LeftRightDoublePress   , 0x5, 0b011)X(MiddleDoublePress   , 0x5, 0b100)\
            X(MouseMultiClick    , 0x06, 0)X(LeftMultiClick    , 0x6, 0b001)X(RightMultiClick    , 0x6, 0b010)X(LeftRightMultiClick    , 0x6, 0b011)X(MiddleMultiClick    , 0x6, 0b100)\
            X(MouseMultiPress    , 0x07, 0)X(LeftMultiPress    , 0x7, 0b001)X(RightMultiPress    , 0x7, 0b010)X(LeftRightMultiPress    , 0x7, 0b011)X(MiddleMultiPress    , 0x7, 0b100)\
            X(MouseDragStart     , 0x08, 0)X(LeftDragStart     , 0x8, 0b001)X(RightDragStart     , 0x8, 0b010)X(LeftRightDragStart     , 0x8, 0b011)X(MiddleDragStart     , 0x8, 0b100)\
            X(MouseDragPull      , 0x09, 0)X(LeftDragPull      , 0x9, 0b001)X(RightDragPull      , 0x9, 0b010)X(LeftRightDragPull      , 0x9, 0b011)X(MiddleDragPull      , 0x9, 0b100)\
            X(MouseDragStop      , 0x0A, 0)X(LeftDragStop      , 0xA, 0b001)X(RightDragStop      , 0xA, 0b010)X(LeftRightDragStop      , 0xA, 0b011)X(MiddleDragStop      , 0xA, 0b100)\
            X(MouseDragCancel    , 0x0B, 0)X(LeftDragCancel    , 0xB, 0b001)X(RightDragCancel    , 0xB, 0b010)X(LeftRightDragCancel    , 0xB, 0b011)X(MiddleDragCancel    , 0xB, 0b100)\
            X(MouseHover         , 0x0C, 0)\
            X(MouseEnter         , 0x0D, 0)\
            X(MouseLeave         , 0x0E, 0)\
            X(MouseMove          , 0x0F, 0)\
            X(MouseWheel         , 0x10, 0)
        static const auto mouse_names = utf::unordered_map<text, std::pair<si32, si32>>
        {
            #define X(name, action_index, button_bits) \
                { utf::to_lower(#name), { action_index, button_bits }},
                mouse_list
            #undef X
        };

        #define X(name, action_index, button_bits) \
            static constexpr auto name = (action_index << 8) | button_bits;
            mouse_list
        #undef X

        static constexpr auto MouseAnyButtonMask = 0xFF00;

        #undef mouse_list
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
        id_t gear_id{}; // foci: Gear id.
        si32 focus_type{}; // foci: Exclusive focus request.
        bool just_activate_only{}; // foci: Ignore focusable object, just activate it.
        sptr item{}; // foci: Next focused item.
        sptr next{}; // foci: Next focused item.
        si64 treeid{}; // foci: Focus tree id.
        ui64 digest{}; // foci: Incrementing event number to avoid refocusing when connecting recursively.
    };

    struct multihome_t
    {
        wptr                      world_wptr;  // multihome_t: World reference.
        wptr                      parent_wptr; // multihome_t: Current world's parent.
        std::list<sptr>::iterator holder;      // multihome_t: Iterator on parent's subset list.
    };

    // input: Mouse tracker.
    struct mouse
    {
        enum mode
        {
            none = 0,
            bttn = 1 << 0,
            drag = 1 << 1,
            move = 1 << 2,
            over = 1 << 3,
            utf8 = 1 << 4,
            vtim = 1 << 5, // vt-input-mode
            buttons_press = bttn,
            buttons_drags = bttn | drag,
            all_movements = bttn | drag | move,
            negative_args = bttn | drag | move | over,
            vt_input_mode = bttn | drag | move | over | vtim,
        };
        enum prot
        {
            x11,
            sgr,
            w32,
            //vtm, // vt-input-mode
        };
        struct buttons
        {
            static constexpr auto _counter  = __COUNTER__ + 1;
            static constexpr auto left      = __COUNTER__ - _counter;
            static constexpr auto right     = __COUNTER__ - _counter;
            static constexpr auto middle    = __COUNTER__ - _counter;
            static constexpr auto xbutton1  = __COUNTER__ - _counter;
            static constexpr auto xbutton2  = __COUNTER__ - _counter;
            static constexpr auto leftright = __COUNTER__ - _counter;
            static constexpr auto count     = __COUNTER__ - _counter;
            static constexpr auto bttn_id = std::to_array({ 0b00001,     // left
                                                            0b00010,     // right
                                                            0b00100,     // middle
                                                            0b01000,     // xbutton1
                                                            0b10000,     // xbutton2
                                                            0b00011, }); // leftright
        };

        struct stat
        {
            static constexpr auto _counter = __COUNTER__ + 1;
            static constexpr auto ok       = __COUNTER__ - _counter;
            static constexpr auto halt     = __COUNTER__ - _counter;
            static constexpr auto die      = __COUNTER__ - _counter;
        };

        struct hist_t // Timer for successive double-clicks, e.g. triple-clicks.
        {
            time fired; // hist: .
            twod coord; // hist: .
            si32 count; // hist: .
        };

        using hist = std::unordered_map<si32, hist_t>;
        using tail = netxs::datetime::tail<fp2d>;

        static constexpr auto drag_threshold = 0.3f; // mouse: Mouse drag threshold (to support jittery clicks).

        fp2d prime{}; // mouse: System mouse cursor coordinates.
        fp2d coord{}; // mouse: Relative mouse cursor coordinates.
        fp2d click{}; // mouse: Click position on drag start. Should be used in area selection (not dragging).
        fp32 accum{}; // mouse: Mouse motion accumulator to deffer the mouse drag a bit.
        tail delta{}; // mouse: Mouse movements history.
        bool nodbl{}; // mouse: Whether single click event processed (to prevent double clicks).
        bool hzwhl{}; // mouse: True: Horizontal scrolling. Faux: Vertical scrolling.
        fp32 whlfp{}; // mouse: Scroll delta in float units (lines).
        si32 whlsi{}; // mouse: Scroll delta in integer units (lines).
        id_t swift{}; // mouse: Mouse capturer id.
        id_t hover{}; // mouse: Hovered object id.
        hint cause{}; // mouse: Current event.
        hist stamp{}; // mouse: Recorded intervals between successive button presses to track multi-clicks.
        span delay{}; // mouse: Double-click threshold.
        sysmouse m_sys{}; // mouse: Device state.
        sysmouse m_sav{}; // mouse: Previous device state.

        si32 pressed_count{}; // mouse: The number of pressed physical buttons.

        si32 dtvt_digest{}; // mouse: Synchronized digest with ui::dtvt (per mouse device).
        fp2d dtvt_coords{}; // mouse: Synchronized coords with ui::dtvt (per mouse device).
        ui32 dtvt_serial{}; // mouse: Synchronized serial with ui::dtvt (per mouse device).

        // mouse: Forward the extended mouse event.
        virtual void fire(hint cause) = 0; //, si32 index = mouse::noactive) = 0;
        // mouse: Try to forward the mouse event intact.
        virtual bool fire_fast() = 0;

        // mouse: Return a kinetic animator.
        template<class Law>
        auto fader(span spell)
        {
            //todo use current item's type: Law<twod>
            return delta.fader<Law>(spell);
        }

        si32 pressed = {}; // mouse: Physical button state.
        si32 bttn_id = {}; // mouse: Logical button id.
        si32 clicked = {}; // mouse: Multiclick count.
        bool dragged = {}; // mouse: The button is dragged.
        fp2d pressxy = {}; // mouse: Press coordinates.
        void m2_move()             { fire(input::key::MouseMove                 );               } //log("move        bttn=%% \tcoor=%% \tpressxy=%%", utf::to_bin((si16)bttn_id), coord, pressxy); }
        void m2_wheel()            { fire(input::key::MouseWheel                );               } //log("wheel       bttn=%% \tcoor=%% \tpressxy=%%", utf::to_bin((si16)bttn_id), coord, pressxy, " \thzwhl=", hzwhl, " whlfp=", whlfp, " whlsi=", whlsi); }
        void m2_sglclick()         { fire(input::key::MouseClick       | bttn_id);               } //log("sgl click   bttn=%% \tcoor=%% \tpressxy=%%", utf::to_bin((si16)bttn_id), coord, pressxy); }
        void m2_dblclick()         { fire(input::key::MouseDoubleClick | bttn_id);               } //log("dbl click   bttn=%% \tcoor=%% \tpressxy=%%", utf::to_bin((si16)bttn_id), coord, pressxy); }
        void m2_dblpress()         { fire(input::key::MouseDoublePress | bttn_id);               } //log("dbl press   bttn=%% \tcoor=%% \tpressxy=%%", utf::to_bin((si16)bttn_id), coord, pressxy); }
        void m2_multiclick()       { fire(input::key::MouseMultiClick  | bttn_id);               } //log("multi_click bttn=%% \tcoor=%% \tpressxy=%%", utf::to_bin((si16)bttn_id), coord, pressxy, " \tclicks: ", clicked); }
        void m2_multipress()       { fire(input::key::MouseMultiPress  | bttn_id);               } //log("multi_press bttn=%% \tcoor=%% \tpressxy=%%", utf::to_bin((si16)bttn_id), coord, pressxy, " \tclicks: ", clicked); }
        void m2_drag_start()       { fire(input::key::MouseDragStart   | bttn_id);               } //log("drag_start  bttn=%% \tcoor=%% \tpressxy=%%", utf::to_bin((si16)bttn_id), coord, pressxy); }
        void m2_drag_pull()        { fire(input::key::MouseDragPull    | bttn_id);               } //log("drag_pull   bttn=%% \tcoor=%% \tpressxy=%%", utf::to_bin((si16)bttn_id), coord, pressxy); }
        void m2_drag_cancel()      { fire(input::key::MouseDragCancel  | bttn_id);               } //log("drag_cancel bttn=%% \tcoor=%% \tpressxy=%%", utf::to_bin((si16)bttn_id), coord, pressxy); }
        void m2_drag_stop()        { fire(input::key::MouseDragStop    | bttn_id);               } //log("drag_stop   bttn=%% \tcoor=%% \tpressxy=%%", utf::to_bin((si16)bttn_id), coord, pressxy); }
        void m2_up()               { fire(input::key::MouseUp          | bttn_id);               } //log("up          bttn=%% \tcoor=%% \tpressxy=%%", utf::to_bin((si16)bttn_id), coord, pressxy, " pressed count: ", pressed_count); }
        void m2_push()             { fire(input::key::MouseDown        | bttn_id); m2_pressed(); } //log("push        bttn=%% \tcoor=%% \tpressxy=%%", utf::to_bin((si16)bttn_id), coord, pressxy, " pressed count: ", pressed_count); }
        void m2_pressed()
        {
            if (bttn_id && !dragged && !nodbl) // Fire a double/multi-press if delay is not expired and the mouse is at the same position.
            {
                auto& s = stamp[bttn_id];
                auto fired = m_sys.timecod;
                if (fired - s.fired < delay && s.coord == coord)
                {
                    clicked = s.count + 1;
                    if (s.count == 1)
                    {
                        m2_dblpress();
                        s.fired = fired;
                        s.count++;
                    }
                    else if (s.count > 1)
                    {
                        m2_multipress();
                        s.fired = fired;
                        s.count++;
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
        void m2_click()
        {
            clicked = 1;
            m2_sglclick();
            if (!nodbl) // Fire a double/multi-click if delay is not expired and the mouse is at the same position.
            {
                auto& s = stamp[bttn_id];
                clicked = s.count;
                if (s.count == 2)
                {
                    m2_dblclick();
                }
                else if (s.count > 2)
                {
                    m2_multiclick();
                    if (s.count == 5) // Limit to quintuple click. 0-based.
                    {
                        s.fired = {};
                        s.count = {};
                    }
                }
            }
        }
        // mouse: Generate mouse event.
        void update(sysmouse& m, core const& idmap)
        {
            auto modschanged = m_sys.ctlstat != m.ctlstat;
            m_sys.set(m);
            if (auto prev_count = std::exchange(pressed_count, std::popcount((ui32)m.buttons)); prev_count != pressed_count)
            {
                if (prev_count < pressed_count) fire(input::key::MouseDown); // Signal down/up with bttn_id=0 in order to update the pressed count for all objects under the event tree.
                else                            fire(input::key::MouseUp);   //   See hids::dispatch() for details.
            }
            auto busy = captured();
            if (busy && fire_fast())
            {
                delta.set(m_sys.coordxy - prime);
                coord = m_sys.coordxy;
                prime = m_sys.coordxy;
                m2_move(); // Update mouse enter/leave state. Don't care about buttons.
                pressed = m_sys.buttons;
                bttn_id = m_sys.buttons;
                dragged = {};
                return;
            }

            if (m.buttons != pressed)
            {
                auto next_state = m.buttons;
                auto prev_state = pressed;
                if (bttn_id & next_state)
                {
                    auto next_bttn_id = bttn_id | next_state;
                    auto prev_bttn_id = bttn_id;
                    if (prev_bttn_id != next_bttn_id) // Additional button pressed. E.g., 100 -> 101. //todo possible bug in Apple's Terminal - it doesn't return the second release in case when two buttons are pressed.
                    {
                        if (dragged)
                        {
                            bttn_id = prev_bttn_id;
                            m2_drag_cancel(); // drag_cancel(100);
                            dragged = faux;
                        }
                        // Press new and then release old to keep the mouse capture continuous.
                        bttn_id = next_bttn_id;
                        pressxy = m_sys.coordxy;
                        m2_push(); // push(101);
                        bttn_id = prev_bttn_id;
                        m2_up(); // up(100);
                        bttn_id = next_bttn_id;
                    }
                    else // Some button released. E.g., 101 -> 001.
                    {
                        // Do nothing.
                    }
                }
                else if (!prev_state && next_state) // Initial button(s) pressed. E.g., 000 -> 100.
                {
                    assert(bttn_id == 0);
                    bttn_id = next_state;
                    pressxy = m_sys.coordxy;
                    m2_push(); // push(100);
                }
                else if (prev_state && !next_state) // The last button(s) released. E.g., 100 -> 000 (bttn_id=101).
                {
                    assert(bttn_id != 0);
                    if (dragged)
                    {
                        m2_drag_stop(); // drag_stop(101);
                        dragged = faux;
                    }
                    else
                    {
                        m2_click();
                    }
                    m2_up(); // up(101);
                    bttn_id = {};
                }
                pressed = next_state;
            }

            if (m_sys.coordxy != prime || modschanged)
            {
                auto step = m_sys.coordxy - prime;
                if (pressed) accum += std::abs(step.x) + std::abs(step.y);
                else         accum = {};
                auto new_target = idmap.link(m_sys.coordxy) != idmap.link(prime);
                auto allow_drag = accum > drag_threshold || new_target;
                delta.set(step);
                if (pressed && allow_drag && !dragged)
                {
                    click = pressxy;
                    dragged = true;
                    m2_drag_start();
                }
                coord = m_sys.coordxy;
                prime = m_sys.coordxy;
                if (allow_drag && dragged)
                {
                    m2_drag_pull();
                }
                m2_move();
            }

            if (!busy && fire_fast())
            {
                pressed = m_sys.buttons;
                bttn_id = m_sys.buttons;
                dragged = {};
                return;
            }

            coord = m_sys.coordxy;
            if (m_sys.wheelfp)
            {
                hzwhl = m_sys.hzwheel;
                whlfp = m_sys.wheelfp;
                whlsi = m_sys.wheelsi;
                m2_wheel();
                m_sys.hzwheel = {}; // Clear one-shot events.
                m_sys.wheelfp = {};
                m_sys.wheelsi = {};
                hzwhl = {};
                whlfp = {};
                whlsi = {};
            }
        }
        // mouse: Is the mouse captured by asker?
        bool captured(id_t asker) const
        {
            return swift == asker;
        }
        // mouse: Is the mouse captured?
        bool captured() const
        {
            return swift;
        }
        // mouse: Capture mouse.
        bool capture(id_t asker)
        {
            if (!swift || swift == asker)
            {
                swift = asker;
                return true;
            }
            return faux;
        }
        // mouse: Release mouse.
        void setfree()
        {
            swift = {};
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

        id_t gear_id{};
        si32 ctlstat{};
        time timecod{};
        si32 keystat{};
        si32 virtcod{};
        si32 scancod{};
        si32 keycode{};
        si32 xlayout{};
        bool extflag{};
        bool handled{};
        si64 touched{};
        text cluster{};
        text vkchord{};
        text scchord{};
        text chchord{};
        text shifted{};
        text unshift{};
        byte payload{}; // keybd: Payload type.

        hint vkevent{}; // In-process keybd virtcod chord identifier.
        hint scevent{}; // In-process keybd scancod chord identifier.
        hint chevent{}; // In-process keybd cluster chord identifier.

        auto doinput()
        {
            return keystat && key::map::data(keycode).edit;
        }
        auto generic()
        {
            return keycode & -2;
        }
        void update(syskeybd& k)
        {
            k.syncto(*this);
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

        bool state{};
        si64 treeid{};
        ui64 digest{};

        void update(sysfocus& f)
        {
            state = f.state;
            treeid = f.treeid;
            digest = f.digest;
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

    struct tooltip_t
    {
        text string{};
        ui32 digest{};
        netxs::sptr<page> page_sptr; // Tooltip render cache for gate.

        tooltip_t(qiew string = {})
            : string{ string }
        { }
        auto get() const
        {
            return qiew{ string };
        }
        void set(view utf8)
        {
            digest++;
            string = utf8;
            page_sptr.reset();
        }
        auto get_render_sptr(cell const& tooltip_colors)
        {
            if (!page_sptr)
            {
                page_sptr = ptr::shared(page{ string, tooltip_colors });
            }
            return page_sptr;
        }
    };

    // input: Human interface device controller.
    struct hids
        : public mouse,
          public keybd,
          public focus,
          public board,
          public base
    {
        using list = std::list<wptr>;
        using kmap = std::unordered_map<si32, text>;

        static constexpr auto classname = basename::gear;

        enum modifiers
        {
            LCtrl        = 1 <<  0, // Left  ⌃ Ctrl
            RCtrl        = 1 <<  1, // Right ⌃ Ctrl
            LAlt         = 1 <<  2, // Left  ⎇ Alt, Left  ⌥ Option
            RAlt         = 1 <<  3, // Right ⎇ Alt, Right ⌥ Option, ⇮ AltGr
            LShift       = 1 <<  4, // Left  ⇧ Shift
            RShift       = 1 <<  5, // Right ⇧ Shift
            LSuper       = 1 <<  6, // Left  ⊞ Win, ◆ Meta, ⌘ Cmd (Apple key), ❖ Super
            RSuper       = 1 <<  7, // Right ⊞ Win, ◆ Meta, ⌘ Cmd (Apple key), ❖ Super
            LHyper       = 1 <<  8, // Left  Hyper
            RHyper       = 1 <<  9, // Right Hyper
            //           = 1 << 10,
            //           = 1 << 11,
            NumLock      = 1 << 12, // ⇭ Num Lock
            CapsLock     = 1 << 13, // ⇪ Caps Lock
            ScrlLock     = 1 << 14, // ⇳ Scroll Lock (⤓)
            AltGr        = 1 << 15, // AltGr on non-US keyboards
            LCtrlAlt     = LAlt   | LCtrl,
            anyCtrl      = LCtrl  | RCtrl,
            anyAlt       = LAlt   | RAlt,
            anyShift     = LShift | RShift,
            anyCtrlAlt   = anyAlt | anyCtrl,
            anySuper     = LSuper | RSuper,
            anyHyper     = LHyper | RHyper,
            anyMod       = anyAlt | anyCtrl | anyShift | anySuper | anyHyper,
        };
        struct kkp
        {
            static constexpr auto shift     = 0b1;        // 1
            static constexpr auto alt       = 0b10;       // 2
            static constexpr auto ctrl      = 0b100;      // 4
            static constexpr auto super     = 0b1000;     // 8
            static constexpr auto hyper     = 0b10000;    // 16
            static constexpr auto meta      = 0b100000;   // 32
            static constexpr auto caps_lock = 0b1000000;  // 64
            static constexpr auto num_lock  = 0b10000000; // 128
        };

        static auto build_alone_key()
        {
            return std::unordered_map<si32, text>
            {
                { key::Backspace,     "\x7f"     },
                { key::Tab,           "\x09"     },
                { key::Pause,         "\x1a"     },
                { key::Esc,           "\033"     },
                { key::KeyPageUp,     "\033[5~"  },
                { key::KeyPageDown,   "\033[6~"  },
                { key::KeyEnd,        "\033[F"   },
                { key::KeyHome,       "\033[H"   },
                { key::KeyLeftArrow,  "\033[D"   },
                { key::KeyUpArrow,    "\033[A"   },
                { key::KeyRightArrow, "\033[C"   },
                { key::KeyDownArrow,  "\033[B"   },
                { key::KeyInsert,     "\033[2~"  },
                { key::KeyDelete,     "\033[3~"  },
                { key::F1,            "\033OP"   },
                { key::F2,            "\033OQ"   },
                { key::F3,            "\033OR"   },
                { key::F4,            "\033OS"   },
                { key::F5,            "\033[15~" },
                { key::F6,            "\033[17~" },
                { key::F7,            "\033[18~" },
                { key::F8,            "\033[19~" },
                { key::F9,            "\033[20~" },
                { key::F10,           "\033[21~" },
                { key::F11,           "\033[23~" },
                { key::F12,           "\033[24~" },
            };
        }
        static auto build_shift_key()
        {
            return std::unordered_map<si32, text>
            {
                { key::KeyPageUp,     "\033[5; ~"  },
                { key::KeyPageDown,   "\033[6; ~"  },
                { key::KeyEnd,        "\033[1; F"  },
                { key::KeyHome,       "\033[1; H"  },
                { key::KeyLeftArrow,  "\033[1; D"  },
                { key::KeyUpArrow,    "\033[1; A"  },
                { key::KeyRightArrow, "\033[1; C"  },
                { key::KeyDownArrow,  "\033[1; B"  },
                { key::KeyInsert,     "\033[2; ~"  },
                { key::KeyDelete,     "\033[3; ~"  },
                { key::F1,            "\033[1; P"  },
                { key::F2,            "\033[1; Q"  },
                { key::F3,            "\033[1; R"  },
                { key::F4,            "\033[1; S"  },
                { key::F5,            "\033[15; ~" },
                { key::F6,            "\033[17; ~" },
                { key::F7,            "\033[18; ~" },
                { key::F8,            "\033[19; ~" },
                { key::F9,            "\033[20; ~" },
                { key::F10,           "\033[21; ~" },
                { key::F11,           "\033[23; ~" },
                { key::F12,           "\033[24; ~" },
            };

        }
        static auto build_other_key(si32 slash, si32 quest)
        {
            return std::unordered_map<si32, text>
            {
                { key::KeyEnter  | (hids::anyCtrl    << key::idbits), { "\x0a"      }},
                { key::Backspace | (hids::anyCtrl    << key::idbits), { "\x08"      }},
                { key::Backspace | (hids::anyAlt     << key::idbits), { "\033\x7f"  }},
                { key::Backspace | (hids::anyCtrlAlt << key::idbits), { "\033\x08"  }},
                { key::Tab       | (hids::anyCtrl    << key::idbits), { "\t"        }},
                { key::Tab       | (hids::anyShift   << key::idbits), { "\033[Z"    }},
                { key::Tab       | (hids::anyAlt     << key::idbits), { "\033\t"    }},
                { key::Esc       | (hids::anyAlt     << key::idbits), { "\033\033"  }},
                { key::Key1      | (hids::anyCtrl    << key::idbits), { "1"         }},
                { key::Key2      | (hids::anyCtrl    << key::idbits), { "\x00"      }},
                { key::Key3      | (hids::anyCtrl    << key::idbits), { "\x1b"      }},
                { key::Key4      | (hids::anyCtrl    << key::idbits), { "\x1c"      }},
                { key::Key5      | (hids::anyCtrl    << key::idbits), { "\x1d"      }},
                { key::Key6      | (hids::anyCtrl    << key::idbits), { "\x1e"      }},
                { key::Key7      | (hids::anyCtrl    << key::idbits), { "\x1f"      }},
                { key::Key8      | (hids::anyCtrl    << key::idbits), { "\x7f"      }},
                { key::Key9      | (hids::anyCtrl    << key::idbits), { "9"         }},
                { key::KeySlash  | (hids::anyCtrl    << key::idbits), { "\x1f"      }},
                { slash          | (hids::anyCtrlAlt << key::idbits), { "\033\x1f"  }},
                { slash          | (hids::anyCtrl    << key::idbits), { "\x1f"      }},
                { quest          | (hids::anyCtrlAlt << key::idbits), { "\033\x7f"  }},
                { quest          | (hids::anyCtrl    << key::idbits), { "\x7f"      }},
            };
        }

        struct
        {
            hids&                   gear;
            netxs::sptr<tooltip_t>  current_sptr;            // tooltip: Last visible tooltip.
            span                    timeout = {};            // tooltip: Time to wait for the tooltip to be shown.
            time                    time_to_run = {};        // tooltip: The moment to show tooltip.
            bool                    visible = {};            // tooltip: Tooltip could be rendered.
            bool                    fresh = {};              // tooltip: Tooltip is visible and just updated.
            bool                    canceled = {};           // tooltip: Tooltip canceled.
            bool                    changed_visibility = {}; // tooltip: Tooltip changes its visibility.
            ui32                    digest = {};             // tooltip: Digest for tracking current tooltip updates.
            twod                    coor = {};               // tooltip: Mouse position when tooltip shown.
            argb                    default_fgc = {};        // tooltip: Default fgc color.
            argb                    default_bgc = {};        // tooltip: Default bgc color.

            void set_text(qiew utf8, argb fgc, argb bgc)
            {
                if (!current_sptr)
                {
                    current_sptr = ptr::shared<input::tooltip_t>(utf8);
                }
                else
                {
                    current_sptr->set(utf8);
                }
                default_fgc = fgc;
                default_bgc = bgc;
            }
            void set(netxs::sptr<tooltip_t> new_current_sptr = {})
            {
                current_sptr = new_current_sptr;
            }
            void request(base& boss)
            {
                auto prev_sptr = std::exchange(current_sptr, netxs::sptr<tooltip_t>{});
                boss.base::raw_riseup(tier::mouserelease, input::key::MouseHover, gear);
                if (!ptr::is_equal(prev_sptr, current_sptr))
                {
                    canceled = !current_sptr || current_sptr->get().empty();
                    if (!canceled)
                    {
                        time_to_run = datetime::now() + timeout;
                        digest = current_sptr->digest;
                    }
                    if (current_sptr)
                    {
                        digest = current_sptr->digest;
                    }
                    fresh = true;
                    visible = faux;
                    changed_visibility = true;
                }
            }
            auto get_render_sptr_and_offset(cell const& tooltip_colors = {})
            {
                if (visible && current_sptr)
                {
                    auto render_sptr = current_sptr->get_render_sptr(tooltip_colors);
                    auto page_offset = -twod{ 4, render_sptr->size() + 1 };
                    return std::pair{ render_sptr, page_offset };
                }
                else
                {
                    return std::pair{ netxs::sptr<page>{}, dot_00 };
                }
            }
            auto get_fresh_qiew()
            {
                if (fresh)
                {
                    fresh = faux;
                    return std::optional{ visible && current_sptr ? current_sptr->get() : qiew{} };
                }
                else
                {
                    return std::optional<qiew>{};
                }
            }
            void hide()
            {
                fresh = true;
                visible = faux;
                canceled = true;
                changed_visibility = true;
            }
            void recalc(hint deed)
            {
                if (canceled) return;
                if (deed == input::key::MouseMove)
                {
                    if (coor(gear.mouse::coord)) // Hide tooltip on mouse move.
                    {
                        if (visible)
                        {
                            hide();
                        }
                        else // Reset timer to begin.
                        {
                            time_to_run = datetime::now() + timeout;
                        }
                    }
                }
                else if (deed == input::key::MouseWheel             // Hide tooltip on wheeling.
                     ||  deed == input::key::MouseLeave             // Hide tooltip on mouse leave.
                     || (deed >> 8 == input::key::MouseDown >> 8))  // Hide tooltip on any press.
                {
                    hide();
                }
            }
            auto check(time now) // Called every timer tick.
            {
                if (changed_visibility)
                {
                    changed_visibility = faux;
                    return true;
                }
                else if (current_sptr && digest != current_sptr->digest) // Show tooltip immediately if it is recently updated.
                {
                    digest = current_sptr->digest;
                    fresh = true;
                    visible = true;
                    canceled = current_sptr->get().empty();
                    coor(gear.mouse::coord);
                    return true;
                }
                else if (!canceled && !visible && current_sptr && time_to_run < now) // Show tooltip on idle timeout.
                {
                    fresh = true;
                    visible = true;
                    return true;
                }
                else
                {
                    return faux;
                }
            }
        }
        tooltip{ *this }; // hids: Tooltips.

        base&           owner;
        core const&     idmap; // hids: Area of the main form. Primary or relative region of the mouse coverage.
        bool            alive; // hids: Whether event processing is complete.
        ui::pro::timer& timer; // hids: .

        //todo unify
        rect slot; // slot for pro::maker and e2::createby.
        bool slot_forced = faux; // slot is preferred over cfg.winsize.

        //todo unify
        bool mouse_disabled = true; // Hide mouse cursor.
        bool keybd_disabled = true; // Inactive gear.
        si32 countdown = 0;

        si32 gear_index; // hids: Gear visual index.
        argb gear_color; // hids: Gear's focus color.
        kmap other_key; // hids: Dynamic key-vt mapping.

        bool shared_event = faux; // hids: The key event was touched by another procees/handler. See pro::keybd(release, key::post) for detailts.

        multihome_t& multihome; // hids: .

        hids(auth& indexer, base& owner, core const& idmap, bool use_index)
            : base{ indexer },
              owner{ owner },
              idmap{ idmap },
              alive{ faux },
              timer{ base::plugin<ui::pro::timer>() },
              gear_index{ indexer.take_gear_available_index(use_index) },
              other_key{ build_other_key(key::KeySlash, key::KeySlash | (hids::anyShift << key::idbits)) }, // Defaults for US layout.
              multihome{ owner.base::property<multihome_t>("multihome") }
        {
            mouse::prime = dot_mx;
            mouse::coord = dot_mx;
            keybd::gear_id = bell::id;
        }
        // Null gear.
        hids(auth& indexer)
            : hids{ indexer, *this, indexer._null_idmap, faux }
        { }
        virtual ~hids()
        {
            mouse_leave(owner);
            release_if_captured();
            bell::indexer.release_gear_index(gear_index);
            base::signal(tier::release, input::events::halt, *this);
            base::signal(tier::general, input::events::halt, *this);
            base::signal(tier::release, input::events::die, *this);
            base::signal(tier::general, input::events::die, *this);
        }

        // hids: Whether event processing is complete.
        operator bool () const
        {
            return alive;
        }

        // hids: The gear has visual index.
        bool use_index()
        {
            return gear_index != auth::non_index;
        }
        static argb get_color(si32 gear_index)
        {
            auto color = argb::vt256[4 + gear_index % (256 - 4)];
            return color;
        }
        bool is_real()
        {
            return id != 0;
        }
        void set_multihome()
        {
            if (auto world_ptr = multihome.world_wptr.lock())
            {
                world_ptr->base::father = multihome.parent_wptr;
                world_ptr->base::holder = multihome.holder;
            }
            bell::indexer.luafx.set_gear(*this);
        }

        auto meta(si32 ctl_key = -1) { return keybd::ctlstat & ctl_key; }

        // hids: Stop handeling this event.
        void dismiss(bool set_nodbl = faux)
        {
            alive = faux;
            if (set_nodbl)
            {
                mouse::nodbl = true;
            }
        }
        void dismiss_dblclick()
        {
            nodbl = true;
        }
        void set_handled(bool do_dismiss = true)
        {
            keybd::handled = true;
            if (do_dismiss)
            {
                dismiss(true);
            }
        }

        si32 repeat_bttn_id = {}; // hids: .
        void repeat_while_pressed(id_t asker_id)
        {
            if (timer || !asker_id || !mouse::bttn_id) return;
            repeat_bttn_id = mouse::bttn_id;
            mouse::capture(asker_id);
            timer.actify(0, ui::skin::globals().repeat_delay, [&, asker_id](auto)
            {
                if (!mouse::captured(asker_id)) return faux;
                auto temp = std::exchange(keybd::keystat, input::key::repeated);
                mouse::m2_push();
                keybd::keystat = temp;
                timer.actify(0, ui::skin::globals().repeat_rate, [&, asker_id](auto)
                {
                    if (!mouse::captured(asker_id)) return faux;
                    auto temp = std::exchange(keybd::keystat, input::key::repeated);
                    mouse::m2_push();
                    keybd::keystat = temp;
                    return true; // Repeat forever.
                });
                return true; // One shot call: timer is reinitialized.
            });
        }

        void take(sysfocus& f)
        {
            if (f.state) keybd_disabled = faux;
            focus::update(f);
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
                     if (l_ctrl && alt) { netxs::_k2 += m.wheelsi > 0 ? 1 : -1; log("_k2=", _k2); } // LCtrl+Alt+Wheel.
                else if (l_ctrl)        { netxs::_k0 += m.wheelsi > 0 ? 1 : -1; log("_k0=", _k0); } // LCtrl+Wheel.
                else if (alt)           { netxs::_k1 += m.wheelsi > 0 ? 1 : -1; log("_k1=", _k1); } // Alt+Wheel.
                else if (r_ctrl)        { netxs::_k3 += m.wheelsi > 0 ? 1 : -1; log("_k3=", _k3); } // RCtrl+Wheel.
            }
            #endif
            mouse_disabled = faux;
            keybd_disabled = faux;
            keybd::ctlstat = m.ctlstat;
            mouse::update(m, idmap);
            if (repeat_bttn_id && repeat_bttn_id != mouse::bttn_id)
            {
                if (timer)
                {
                    mouse::setfree();
                    timer.pacify();
                }
                repeat_bttn_id = {};
            }
            //if (bttn_id)
            //{
            //    repeat_while_pressed(id);
            //}
        }
        void take(syskeybd& k)
        {
            keybd_disabled = faux;
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
                if (tooltip.visible)
                {
                    tooltip.hide();
                }
                keybd::vkevent = indexer.get_kbchord_hint(k.vkchord);
                keybd::scevent = indexer.get_kbchord_hint(k.scchord);
                keybd::chevent = indexer.get_kbchord_hint(k.chchord);
                keybd::update(k);
            }
        }
        auto take(sysboard& b)
        {
            board::update(b);
        }

        auto& area() const { return idmap.area(); }

        void dispatch(si32 tier_id, base& boss)
        {
            if (tier_id == tier::mouserelease && (mouse::cause == input::key::MouseUp     // Just amplify mouse hover on any button press.
                                               || mouse::cause == input::key::MouseDown)) //   See mouse::update() for details.
            {
                notify_form_state(boss);
                return;
            }
            auto saved_cause = mouse::cause;
            boss.base::signal(tier_id, mouse::cause, *this);
            mouse::cause = saved_cause;
            auto any_bttn_event = mouse::cause & input::key::MouseAnyButtonMask; // Set button_bits = 0.
            if (alive && mouse::cause != any_bttn_event)
            {
                boss.base::signal(tier_id, any_bttn_event, *this);
                mouse::cause = saved_cause;
            }
            if (alive && mouse::cause != input::key::MouseAny)
            {
                boss.base::signal(tier_id, input::key::MouseAny, *this);
                mouse::cause = saved_cause;
            }
        }
        void forward(si32 Tier, base& boss)
        {
            auto offset = boss.base::coor() + boss.base::intpad.corner();
            if (Tier == tier::mousepreview)
            {
                if (auto parent_ptr = boss.base::parent())
                {
                    auto& parent = *parent_ptr;
                    pass(Tier, parent, offset);
                }
                dispatch(Tier, boss);
            }
            else
            {
                dispatch(Tier, boss);
                if ((alive && !captured()) || mouse::cause == input::key::MouseEnter || mouse::cause == input::key::MouseLeave)
                {
                    if (auto parent_ptr = boss.base::parent())
                    {
                        auto& parent = *parent_ptr;
                        pass(Tier, parent, offset);
                    }
                }
            }
        }
        void pass(si32 Tier, base& boss, fp2d offset, bool relative = faux)
        {
            auto saved_coord = mouse::coord;
            auto saved_click = mouse::click;
            if (relative)
            {
                boss.global(coord);
                click += coord - saved_coord;
            }
            mouse::coord += offset;
            mouse::click += offset;
            forward(Tier, boss);
            mouse::coord = saved_coord;
            mouse::click = saved_click;
        }
        void replay(base& boss, hint new_cause, fp2d new_coord, fp2d new_click, fp2d new_delta, si32 new_button_state, si32 new_bttn_id, bool new_dragged, si32 new_ctlstate, fp32 new_whlfp, si32 new_whlsi, bool new_hzwhl)
        {
            alive = true;
            keybd::ctlstat = new_ctlstate;
            mouse::coord = new_coord;
            mouse::click = new_click;
            mouse::whlfp = new_whlfp;
            mouse::whlsi = new_whlsi;
            mouse::hzwhl = new_hzwhl;
            mouse::cause = new_cause;
            mouse::delta.set(new_delta);
            mouse::dragged = new_dragged;
            mouse::pressxy = new_click;
            auto saved_pressed = std::exchange(mouse::pressed, new_button_state); // Save/restore state to avoid double click leaks.
            auto saved_bttn_id = std::exchange(mouse::bttn_id, new_bttn_id);      //
            pass(tier::mouserelease, boss, owner.base::coor(), true);
            mouse::pressed = saved_pressed;
            mouse::bttn_id = saved_bttn_id;
        }
        // hids: Notify about the number of mouse hovers.
        void notify_form_state(base& boss, feed enter_or_leave = feed::none)
        {
            if (enter_or_leave == feed::fwd)
            {
                //log("on enter:");
                if (boss.mouse_focus.size() == 1) // The first mouse cursor came.
                {
                    boss.base::signal(tier::release, ui::e2::form::state::mouse, true);
                }
            }
            else if (enter_or_leave == feed::rev)
            {
                //log("on leave:");
                if (boss.mouse_focus.empty()) // The last mouse cursor is gone.
                {
                    boss.base::signal(tier::release, ui::e2::form::state::mouse, faux);
                }
            }
            //else
            //{
            //    log("on update:");
            //}
            //    for (auto& [g, n] : boss.mouse_focus)
            //    {
            //        log("  gear id: ", g.lock()?g.lock()->id:0);
            //        log("  next id: ", n.lock()?n.lock()->id:0);
            //    }
            auto hover_count = boss.mouse_focus.size() ? (si32)boss.mouse_focus.size() + mouse::pressed_count : 0;
            //log("boss id=%% hover_count=%% mouse::pressed_count=%%", boss.id, hover_count, mouse::pressed_count);
            boss.base::signal(tier::release, ui::e2::form::state::hover, hover_count);
        }
        void mouse_leave(base& boss)
        {
            auto this_wptr = weak_from_this();
            auto saved_cause = std::exchange(mouse::cause, input::key::MouseLeave);
            for (auto& rec : boss.mouse_focus)
            {
                if (ptr::is_equal(rec.gear_wptr, this_wptr)) // We found our gear. Boss is already focused.
                {
                    dispatch(tier::mouserelease, boss); // Signal MouseLeave.
                    if (auto next_ptr = rec.next_wptr.lock())
                    {
                        auto& next = *next_ptr;
                        mouse_leave(next);
                    }
                    if (boss.mouse_focus.size() > 1) rec = boss.mouse_focus.back(); // Remove an item without allocations.
                    boss.mouse_focus.pop_back();
                    notify_form_state(boss, feed::rev);
                    break;
                }
            }
            mouse::cause = saved_cause;
        }
        void _mouse_enter(base& boss, wptr next_wptr = {})
        {
            auto this_wptr = weak_from_this();
            for (auto& rec : boss.mouse_focus)
            {
                if (ptr::is_equal(rec.gear_wptr, this_wptr)) // We found our gear. Boss is already focused.
                {
                    if (!ptr::is_empty(rec.next_wptr)) // Unfocus the next branch if next_wptr is not empty.
                    {
                        if (auto next_ptr = rec.next_wptr.lock())
                        {
                            auto& next = *next_ptr;
                            mouse_leave(next);
                        }
                    }
                    rec.next_wptr = next_wptr;
                    return;
                }
            }
            // Gear not found.
            boss.mouse_focus.push_back({ weak_from_this(), next_wptr });
            dispatch(tier::mouserelease, boss); // Signal MouseEnter.
            notify_form_state(boss, feed::fwd);
            if (auto parent_ptr = boss.base::parent())
            {
                auto& parent = *parent_ptr;
                _mouse_enter(parent, boss.weak_from_this());
            }
        }
        void mouse_enter(base& boss)
        {
            auto saved_cause = std::exchange(mouse::cause, input::key::MouseEnter);
            _mouse_enter(boss);
            mouse::cause = saved_cause;
        }
        void redirect_mouse_focus(base& boss)
        {
            if (mouse::hover != boss.id) // The mouse cursor is over the new object.
            {
                mouse_enter(boss);
                mouse::hover = boss.id;
                tooltip.request(boss);
            }
        }
        void release_if_captured()
        {
            if (captured())
            {
                setfree();
                dismiss();
            }
        }
        void deactivate()
        {
            mouse::pressed = {};
            mouse::bttn_id = {};
            mouse::dragged = {};
            mouse::m_sys.buttons = {};
            redirect_mouse_focus(owner);
            release_if_captured();
            base::signal(tier::general, input::events::halt, *this);
            mouse_disabled = true;
            keybd_disabled = true;
        }
        void fire(hint new_cause)//, si32 new_index = mouse::noactive)
        {
            if (mouse_disabled) return;
            alive = true;
            mouse::cause = new_cause;
            mouse::coord = mouse::prime;
            mouse::nodbl = faux;
            auto gate_coor = idmap.coor();
            if (mouse::swift)
            {
                if (auto next_ptr = base::getref(mouse::swift))
                {
                    auto& next = *next_ptr;
                    redirect_mouse_focus(next);
                    pass(tier::mouserelease, next, gate_coor, true);
                    if (alive && !captured()) // Pass unhandled event to the gate.
                    {
                        forward(tier::mouserelease, owner);
                    }
                }
                else
                {
                    mouse::setfree();
                    redirect_mouse_focus(owner);
                }
            }
            else
            {
                auto next_id = idmap.link(mouse::coord);
                if (auto next_ptr = next_id != owner.id ? base::getref(next_id) : sptr{})
                {
                    auto& next = *next_ptr;
                    pass(tier::mousepreview, next, gate_coor, true);
                    if (alive)
                    {
                        redirect_mouse_focus(next);
                        pass(tier::mouserelease, next, gate_coor, true);
                    }
                    else // Pass mouse events through accesslocked objects.
                    {
                        alive = true;
                        if (auto world_ptr = multihome.world_wptr.lock())
                        {
                            forward(tier::mouserelease, *world_ptr); // Pass event to the ui::hall.
                            tooltip.recalc(new_cause);
                            return;
                        }
                    }
                }
                else
                {
                    forward(tier::mousepreview, owner);
                    if (alive)
                    {
                        redirect_mouse_focus(owner);
                    }
                }
                if (alive)
                {
                    forward(tier::mouserelease, owner); // Pass unhandled event to the gate.
                }
                tooltip.recalc(new_cause);
            }
        }
        bool fire_fast()
        {
            if (mouse_disabled) return true;
            alive = true;
            auto next_id = mouse::swift ? mouse::swift
                                        : idmap.link(m_sys.coordxy);
            if (next_id != owner.id)
            {
                if (auto next_ptr = base::getref(next_id))
                {
                    auto& next = *next_ptr;
                    auto  temp = m_sys.coordxy;
                    m_sys.coordxy += idmap.coor();
                    next.global(m_sys.coordxy, [&](auto& parent) // Ask parents.
                    {
                        parent.base::signal(tier::preview, input::events::device::mouse::on, *this);
                        return alive;
                    });
                    if (alive)
                    {
                        next.base::signal(tier::release, input::events::device::mouse::on, *this);
                    }
                    else // Redirect mouse focus when rejected objects are detected.
                    {
                        redirect_mouse_focus(owner);
                    }
                    m_sys.coordxy = temp;
                    if (!alive) // Clear one-shot events on success.
                    {
                        m_sys.wheelfp = {};
                        m_sys.wheelsi = {};
                        m_sys.hzwheel = {};
                    }
                }
                else if (mouse::swift == next_id) mouse::setfree(); // Captureer is dead.
            }
            if (m_sav.changed != m_sys.changed) m_sav = m_sys;
            return !alive;
        }
        void fire_keybd()
        {
            owner.base::signal(tier::preview, input::events::keybd::post, *this);
        }
        void fire_board()
        {
            owner.base::signal(tier::release, input::events::clipboard, *this);
            mouse::delta.set(); // Update time stamp.
        }
        void fire_focus()
        {
            focus::state ? owner.base::signal(tier::release, input::events::focus::set::on,  { .gear_id = id, .just_activate_only = true, .treeid = focus::treeid, .digest = focus::digest })
                         : owner.base::signal(tier::release, input::events::focus::set::off, { .gear_id = id,                             .treeid = focus::treeid, .digest = focus::digest });
        }
        text interpret(bool decckm) const
        {
            static auto alone_key = build_alone_key();
            static auto shift_key = build_shift_key();

            if (keystat != input::key::released)
            {
                auto s = keybd::ctlstat;
                auto v = keybd::keycode & -2; // Generic keys only
                auto c = keybd::cluster.empty() ? 0 : (byte)keybd::cluster.front();

                if (v < 0 || v >= input::key::lastKey) v = 0;
                if (s & hids::AltGr || (s & hids::LCtrl && s & hids::RAlt)) // This combination is already translated.
                {
                    s &= ~(hids::LCtrl | hids::RAlt);
                }

                auto shift = s & hids::anyShift ? hids::anyShift : 0;
                auto alt   = s & hids::anyAlt   ? hids::anyAlt   : 0;
                auto ctrl  = s & hids::anyCtrl  ? hids::anyCtrl  : 0;
                if (shift || alt || ctrl)
                {
                    if (auto it_shift = shift_key.find(v); it_shift != shift_key.end())
                    {
                        auto& mods = *++(it_shift->second.rbegin());
                        mods = '1';
                        if (shift) mods += 1;
                        if (alt  ) mods += 2;
                        if (ctrl ) mods += 4;
                        return it_shift->second;
                    }
                    if (auto crop = input::key::interpret_ctrl(*this, ctrl, shift); crop.size())
                    {
                        if (alt) crop = '\x1b' + crop;
                        return crop;
                    }
                    if (auto it_other = other_key.find(v | (shift | alt | ctrl) << key::idbits); it_other != other_key.end())
                    {
                        return it_other->second;
                    }
                    auto& rec = input::key::map::data(v);
                    if (!shift)
                    {
                        if (ctrl && alt)
                        {
                                 if (rec.KKPCtl != -1 ) return "\x1b"s + (char)rec.KKPCtl;
                            else if (c > 0 && c <= 127) return "\x1b"s + (char)(c & 31);  // ^[^@;
                        }
                        else if (ctrl)
                        {
                                 if (rec.KKPCtl != -1 ) return ""s + (char)rec.KKPCtl;
                            else if (c > 0 && c <= 127) return ""s + (char)(c & 31);  // ^@;
                        }
                        else if (alt)
                        {
                            if (rec.KKPAscii != -1) return "\x1b"s + (char)rec.KKPAscii; // Ignore kb layout.
                        }
                    }
                    else
                    {
                        if (ctrl && alt)
                        {
                                 if (rec.KKPCtl != -1 ) return "\x1b"s + (char)rec.KKPCtl;
                            else if (c > 0 && c <= 127) return "\x1b"s + (char)(c & 31);
                        }
                        else if (ctrl)
                        {
                                 if (rec.KKPCtl != -1 ) return ""s + (char)rec.KKPCtl;
                            else if (c > 0 && c <= 127) return ""s + (char)(c & 31);
                        }
                    }
                    if (alt && c)
                    {
                        return text{ '\x1b' + keybd::cluster };
                    }
                }

                if (auto it_alone = alone_key.find(v); it_alone != alone_key.end())
                {
                    if (v >= key::KeyEnd && v <= key::KeyDownArrow)
                    {
                        it_alone->second[1] = decckm ? 'O' : '[';
                    }
                    return it_alone->second;
                }
                else if (c)
                {
                    return keybd::cluster;
                }
            }
            return text{};
        }
    };

    namespace key
    {
        // The bind record is a set of 16-bit words: 0x000a 0x000b ... 0xffff 0xa 0xfe0e
        //  15 bit: 0 - virt code, 1 - scan code                     (0x80'00).
        //  14 bit: 0 - pressed, 1 - released                        (0x40'00).
        //  13 bit: 1 - all subsequent bytes form a grapheme cluster (0x20'00).
        //  12 bit: 1 - mouse code                                   (0x10'00).
        //  0-11 bits: virt (aka KeyId, keycode), scan or mouse code. For clusters it is set to '\x20FF'('\x60FF').
        //  Generic events: (12-15 bits on)  (0xFn ' 00 00 00 00) -- (0xF0 & 4-bit tier ' 32-bit generic event_id).
        auto is_generic( byte sign) { return  (sign & input::key::generic_sign) == input::key::generic_sign; }
        auto is_scancode(byte sign) { return   sign & input::key::scancode_sign; }
        auto is_pressed( byte sign) { return !(sign & input::key::unpressed_sign); }
        auto is_cluster( byte sign) { return   sign & input::key::cluster_sign; }
        auto is_mouse(   byte sign) { return  (sign & input::key::generic_sign) == input::key::mouse_sign; }

        struct kmap
        {
            struct chord_item_t
            {
                si32 index;
                si32 scode;
                time stamp;
            };

            using cmap = std::map<si32, chord_item_t>; // std::unordered_map doesn't sort items.

            cmap pushed{}; // kmap: Pushed key map.
            bool keyout{}; // kmap: Some key has left the key chord.

            void reset(auto& k)
            {
                k.vkchord.clear();
                k.scchord.clear();
                k.chchord.clear();
                k.shifted.clear();
                k.unshift.clear();
                pushed.clear();
                keyout = {};
            }
            auto exist(si32 keyid)
            {
                auto iter = pushed.find(keyid);
                return iter != pushed.end();
            }
            static void push_generic(si32 sign, si32 event_id, text& g_chord)
            {
                g_chord += (byte)sign;
                g_chord += view{ (char*)&event_id, sizeof(event_id) };
            }
            static void push_keyid(bool ispressed, text& vkchord, si32 keyid)
            {
                keyid &= 0x0FFF; // 12 bit max.
                auto hi_12bit_keyid = (byte)(keyid >> 8);
                auto lo_12bit_keyid = (byte)(keyid & 0xFF);
                vkchord.push_back(hi_12bit_keyid | (byte)(ispressed ? 0x00 : input::key::unpressed_sign));
                vkchord.push_back(lo_12bit_keyid);
            }
            static void push_scode(bool ispressed, text& scchord, si32 scode)
            {
                scode &= 0x0FFF; // 12 bit max.
                auto hi_12bit_scode = (byte)((scode >> 8) & 0x01);
                auto lo_12bit_scode = (byte)(scode & 0xFF);
                scchord.push_back(hi_12bit_scode | (byte)((ispressed ? 0x00 : input::key::unpressed_sign) | input::key::scancode_sign));
                scchord.push_back(lo_12bit_scode);
            }
            static void push_cluster(bool ispressed, text& chchord, view cluster)
            {
                chchord += (byte)((ispressed ? 0x00 : input::key::unpressed_sign) | input::key::cluster_sign);
                chchord += '\xFF';
                chchord += cluster;
            }
            static void push_mouse(si32 sign, si32 action_id, si32 button_id, text& m_chord)
            {
                m_chord += (byte)sign;
                m_chord += (byte)action_id;
                m_chord += (byte)button_id;
            }
            template<class P = noop>
            void build(auto& k, P test_key_released = {})
            {
                if (k.keystat != input::key::repeated)
                {
                    //log("key=%% pressed=%%", input::key::map::data(k.keycode).name, k.keystat);
                    if (k.keystat == input::key::released)
                    {
                        pushed.erase(k.keycode);
                    }
                    k.vkchord.clear();
                    k.scchord.clear();
                    k.chchord.clear();
                    auto vk_valid = k.keycode > input::key::config;
                    auto sc_valid = k.scancod > 0;
                    if (!keyout || k.keystat != input::key::released)
                    {
                        keyout = k.keystat == input::key::released;
                        //log(" erasing %%", k.keystat == input::key::released ? "key::released" : k.keystat == input::key::pressed ? "key::pressed" : "key::repeated");
                        std::erase_if(pushed, [&](auto& rec)
                        {
                            auto& [keyid, val] = rec;
                            //log("\tcheck keyid=%%", input::key::map::data(keyid).name);
                            auto is_released = test_key_released(val.index); // Check if it is still pressed.
                            if (!is_released && keyid != k.keycode/*exclude repeated key*/)
                            {
                                if (keyid <= input::key::config) vk_valid = faux;
                                if (val.scode == 0) sc_valid = faux;
                                if (vk_valid) push_keyid(true, k.vkchord, keyid);
                                if (sc_valid) push_scode(true, k.scchord, val.scode);
                            }
                            //else if (is_released) log("\tkeyid=%% released", input::key::map::data(keyid).name);
                            return is_released;
                        });
                        auto valid_codepoint = [](view utf8)
                        {
                            auto codepoint = utf::to_code(utf8);
                            return codepoint > 0 && (codepoint < 57358 || codepoint > 57454);
                        };
                        auto sign = !!k.keystat;
                        auto shift_state = k.ctlstat & hids::anyShift;
                        auto altgr_state = k.ctlstat & hids::AltGr;
                        auto has_cluster = k.cluster.size() && k.cluster.front();
                        auto has_unshift = k.unshift.size() && valid_codepoint(k.unshift) && !altgr_state && !shift_state;
                        auto has_shifted = k.shifted.size() && valid_codepoint(k.shifted) && !altgr_state && shift_state;
                        if (has_cluster || has_unshift || has_shifted) // Try to keep national key names.
                        {
                            k.chchord = k.vkchord; // The main part of the chchord is the same as in vkchord.
                                 if (has_unshift) push_cluster(sign, k.chchord, k.unshift);
                            else if (has_shifted) push_cluster(sign, k.chchord, k.shifted);
                            else                  push_cluster(sign, k.chchord, k.cluster);
                        }
                        push_keyid(sign, k.vkchord, k.keycode);
                        push_scode(sign, k.scchord, k.scancod | (k.extflag ? 0x100 : 0));
                        if (!vk_valid) k.vkchord.clear();
                        if (!sc_valid) k.scchord.clear();
                    }
                    if (k.keystat == input::key::pressed && (vk_valid || sc_valid)) // Filter IME/pasted clusters in chords (sc=0 vk=0).
                    {
                        auto& key = pushed[k.keycode];
                        key.scode = k.scancod | (k.extflag ? 0x100 : 0); // Store the scan code of a pressed key.
                        key.index = k.virtcod; // Store the virtual code to check later that it is still pressed.
                        key.stamp = datetime::now();
                    }
                }
            }
            static auto to_string(qiew chord, bool generic)
            {
                auto crop = text{};
                while (chord.size() > 1)
                {
                    auto s = (si32)(byte)chord.pop_front();
                    auto v = (si32)(byte)chord.pop_front();
                    if (crop.size() || s & input::key::unpressed_sign) crop += s & input::key::unpressed_sign ? '-' : '+';
                    if (s & input::key::scancode_sign) // Scancodes.
                    {
                        auto scancode = v | (s & 0x01 ? 0x100 : 0);
                        auto length = scancode & 0xF00 ? 3 : 2;
                        crop += "0x" + utf::to_hex<true>(scancode, length);
                    }
                    else if (s & input::key::cluster_sign) // Cluster.
                    {
                        auto plain = utf::debase<faux, faux>(chord);
                        utf::replace_all(plain, "'", "\\'");
                        crop += '\'' + plain + '\'';
                        chord.clear();
                    }
                    else // 12-bit Keyid
                    {
                        auto keyid = v | ((s & 0x0F) << 8);
                        crop += generic ? input::key::map::data(keyid).generic : input::key::map::data(keyid).name;
                    }
                }
                return crop;
            }
            static constexpr auto any_key = qiew{ "\0"sv };
            static auto chord_list(qiew chord)
            {
                struct key_t
                {
                    byte sign;
                    si32 code1; // Left (or specific) key code.
                    si32 code2; // Right (if chord is generic) key code
                    text utf8;
                };
                auto keys = std::vector<key_t>{};
                auto crop = std::vector<text>{};
                //todo reimplement chord_list
                if (auto anytest = utf::to_lower(chord);
                   (anytest.starts_with("any") && !anytest.starts_with(tier::str[tier::anycast])) ||
                   (anytest.starts_with(tier::str[tier::preview])
                       && utf::get_trimmed((view{ anytest }.substr(tier::str[tier::preview].size())), ": ").starts_with("any")))
                {
                    crop.push_back(any_key);
                    return crop;
                }
                auto take = [](qiew& chord)
                {
                    auto k = key_t{};
                    utf::trim(chord, netxs::whitespaces);
                    if (chord.empty()) return k;
                    if (auto pos = chord.find("::"); pos != text::npos) // Environment event.
                    {
                        auto event_tier = chord.starts_with(tier::str[tier::preview]) ? tier::preview
                                        : chord.starts_with(tier::str[tier::release]) ? tier::release
                                        : chord.starts_with(tier::str[tier::general]) ? tier::general
                                        : chord.starts_with(tier::str[tier::anycast]) ? tier::anycast
                                        : chord.starts_with(tier::str[tier::request]) ? tier::request
                                                                                      : tier::unknown;
                        if (event_tier != tier::unknown)
                        {
                            auto event_str = chord;
                            event_str.remove_prefix(tier::str[event_tier].size());
                            utf::trim(event_str, netxs::whitespaces_and<':'>);
                            auto& rtti = netxs::events::rtti();
                            auto iter = rtti.find(event_str);
                            if (iter != rtti.end())
                            {
                                auto metadata = iter->second;
                                k.sign = (byte)(input::key::generic_sign | event_tier);
                                k.code1 = metadata.event_id;
                                if constexpr (debugmode) log("generic event: event_str=%% event_id=%% param_typename=%% tier=%%", event_str, metadata.event_id, metadata.param_typename, tier::str[event_tier]);
                            }
                            else
                            {
                                log("generic event: unknown event '%%'", chord);
                            }
                        }
                        chord = {};
                        return k;
                    }
                    if (chord.starts_with(tier::str[tier::preview])) // Drop the "preview:" prefix (it is not used here).
                    {
                        chord.remove_prefix(tier::str[tier::preview].size());
                        utf::trim_front(chord, netxs::whitespaces_and<':'>);
                    }
                    auto c = chord.front();
                    if (c != '-') // Is pressed.
                    {
                        if (c == '+')
                        {
                            chord.pop_front(); // Pop '+'.
                            utf::trim(chord, netxs::whitespaces);
                            if (chord.empty()) return k;
                            c = chord.front();
                        }
                    }
                    else if (chord.size() > 1)
                    {
                        k.sign |= input::key::unpressed_sign;
                        chord.pop_front(); // Pop '-'.
                        utf::trim(chord, netxs::whitespaces);
                        if (chord.empty()) return k;
                        c = chord.front();
                    }
                    utf::trim(chord, netxs::whitespaces);
                    if (chord.empty()) return k;
                    if (auto isscancode = chord.starts_with("0x") || chord.starts_with("0X"); isscancode)
                    {
                        chord.remove_prefix(2);
                        if (auto v = utf::to_int<si32, 16>(chord))
                        {
                            k.sign |= input::key::scancode_sign;
                            k.code1 = v.value();
                        }
                    }
                    else if (chord.size() > 2 && chord.front() == chord.back() && (chord.front() == '\'' || chord.front() == '\"')) // The literal key must be the last key in a sequence.
                    {
                        k.sign |= input::key::cluster_sign;
                        k.utf8 = utf::unescape(chord.substr(1, chord.size() - 2));
                        k.code1 = 0xFF;
                        chord.clear();
                    }
                    else if (auto key_name = qiew{ utf::get_word(chord, "+- ") })
                    {
                        auto name = utf::to_lower(key_name);
                        auto name_shadow = qiew{ name };
                        auto digits = utf::pop_back_chars(name_shadow, netxs::onlydigits);
                        if (auto iter_m = input::key::mouse_names.find(name_shadow); iter_m != input::key::mouse_names.end()) // Mouse events.
                        {
                            auto [action_index, button_index] = iter_m->second;
                            if (digits.size())
                            {
                                auto d = digits.front();
                                if (d == '0' || d == '1') // MouseClick001  binary format.
                                {
                                    auto str = text{ digits };
                                    std::reverse(str.begin(), str.end());
                                    if (auto v = utf::to_int<si32, 2>(str))
                                    {
                                        button_index = v.value();
                                    }
                                }
                                else // MouseClick3  decimal format. B1 to B8 mouse buttons.
                                {
                                    button_index = 1 << std::min(7, d - '0' - 1);
                                }
                            }
                            k.sign = (byte)(input::key::mouse_sign);
                            k.code1 = action_index;
                            k.code2 = button_index;
                            //log("mouse event=%%", ansi::hi(utf::to_hex_0x((k.sign<<8)|k.code1)));
                        }
                        else if (auto iter = input::key::generic_names.find(name); iter == input::key::generic_names.end()) // Is specific.
                        {
                            auto iter2 = input::key::specific_names.find(name);
                            if (iter2 != input::key::specific_names.end())
                            {
                                k.code1 = iter2->second;
                            }
                        }
                        else // Is generic.
                        {
                            auto code = iter->second & -2;
                            auto n1 = input::key::map::data(code).name.size();
                            auto n2 = input::key::map::data(code + 1).name.size();
                            k.code1 = n1 ? code : 0;
                            k.code2 = n2 ? code + 1 : 0;
                        }
                    }
                    utf::trim(chord, netxs::whitespaces);
                    return k;
                };
                // Split.
                while (chord)
                {
                    auto k = take(chord); // Unfold.
                    if (!input::key::is_mouse(k.sign) && !k.code1) return crop; // Unknown key or generic event.
                    keys.push_back(k);
                }
                if (keys.empty() || keys.size() > 8)
                {
                    if (keys.size()) log("%%A maximum of eight keys are allowed per chord", prompt::hids);
                    return crop;
                }
                if (auto& k = keys.front(); input::key::is_generic(k.sign)) // It is generic event.
                {
                    auto& g_chord = crop.emplace_back();
                    push_generic(k.sign, k.code1, g_chord);
                    return crop;
                }
                if (auto& k = keys.front(); input::key::is_mouse(k.sign)) // It is mouse event.
                {
                    auto& m_chord = crop.emplace_back();
                    push_mouse(k.sign, k.code1, k.code2, m_chord);
                    return crop;
                }
                // Sort all but last.
                std::sort(keys.begin(), std::prev(keys.end()), [](auto& l, auto& r){ return l.code1 < r.code1; });
                // Generate.
                auto count = 1 << keys.size();
                auto temp = text{};
                while (count--)
                {
                    auto bits = count;
                    for (auto& k : keys)
                    {
                        if (auto code = bits & 1 ? k.code1 : k.code2)
                        {
                            auto sign = input::key::is_pressed(k.sign);
                            if (input::key::is_scancode(k.sign))
                            {
                                push_scode(sign, temp, code);
                            }
                            else if (input::key::is_cluster(k.sign))
                            {
                                push_cluster(sign, temp, k.utf8);
                                break;
                            }
                            else
                            {
                                push_keyid(sign, temp, code);
                            }
                        }
                        else
                        {
                            temp.clear();
                            break;
                        }
                        bits >>= 1;
                    }
                    if (temp.size()) crop.push_back(temp);
                    temp.clear();
                }
                return crop;
            }
        };
        //todo use lut
        auto detect_layout(si32 unshift, si32 base)
        {
            // QWERTY:
            if (unshift == base)
            {
                if ((unshift >= 'a' && unshift <= 'z') || unshift == ',' || unshift == '.' || unshift == '-'
                 || (unshift >= '1' && unshift <= '9'))
                {
                    if (unshift != 'a' && unshift != 'm' && unshift != 'u'
                     && unshift != 'i' && unshift != 'o' && unshift != 'p'
                     && unshift != 'h' && unshift != 'j' && unshift != 'k' && unshift != 'l') 
                    {
                        return layout::qwerty;
                    }
                }
            }
            // QWERTZ:
            if (unshift == 'z' && base == 'y') return layout::qwertz;
            if (unshift == 'y' && base == 'z') return layout::qwertz;
            if (unshift == '+' && base == ']') return layout::qwertz;
            if (unshift == '-' && base == '/') return layout::qwertz;
            if (unshift == '^' && base == '`') return layout::qwertz;
            if (unshift == '#' && base =='\\') return layout::qwertz; // # <- \   //
            if (unshift == 180 && base == '=') return layout::qwertz; // ´ <- =  Deadkey
            if (unshift == 228 && base =='\'') return layout::qwertz; // ä <- '
            if (unshift == 223 && base == '-') return layout::qwertz; // ß <- -
            if (unshift == 252 && base == '[') return layout::qwertz; // ü <- [
            if (unshift == 367 && base == ';') return layout::qwertz; // ů <- ;  Czech QWERTZ
            if (unshift == 246 && base == ';') return layout::qwertz; // ö <- ;
            if (unshift == 337 && base == '4') return layout::qwertz; // ő <- 4  Hungarian
            // AZERTY:
            if (unshift == 'a' && base == 'q') return layout::azerty;
            if (unshift == 'q' && base == 'a') return layout::azerty;
            if (unshift == 'w' && base == 'z') return layout::azerty;
            if (unshift == 'z' && base == 'w') return layout::azerty;
            if (unshift == 'm' && base == ';') return layout::azerty;
            if (unshift == '&' && base == '1') return layout::azerty;
            if (unshift == 233 && base == '2') return layout::azerty; // é
            if (unshift == '"' && base == '3') return layout::azerty;
            if (unshift =='\'' && base == '4') return layout::azerty;
            if (unshift == '(' && base == '5') return layout::azerty;
            if (unshift == ')' && base == '-') return layout::azerty;
            if (unshift == '-' && base == '6') return layout::azerty;
            if (unshift == 232 && base == '7') return layout::azerty; // è
            if (unshift == '_' && base == '8') return layout::azerty;
            if (unshift == 231 && base == '9') return layout::azerty; // ç
            if (unshift == 224 && base == '0') return layout::azerty; // à
            // DVORAK:
            if (unshift == ',' && base == 'w') return layout::dvorak;
            if (unshift == '.' && base == 'e') return layout::dvorak;
            if (unshift == 'p' && base == 'r') return layout::dvorak;
            if (unshift == 'o' && base == 's') return layout::dvorak;
            if (unshift == 'e' && base == 'd') return layout::dvorak;
            if (unshift == 'u' && base == 'f') return layout::dvorak;
            if (unshift == 'i' && base == 'g') return layout::dvorak;
            if (unshift == 'd' && base == 'h') return layout::dvorak;
            if (unshift =='\'' && base == 'q') return layout::dvorak;
            // BEPO:
            if (unshift == '"' && base == '1') return layout::bepo;
            if (unshift == 171 && base == '2') return layout::bepo; // «
            if (unshift == 187 && base == '3') return layout::bepo; // »
            if (unshift == '(' && base == '4') return layout::bepo;
            if (unshift == ')' && base == '5') return layout::bepo;
            if (unshift == '@' && base == '6') return layout::bepo;
            if (unshift == '+' && base == '7') return layout::bepo;
            if (unshift == '-' && base == '8') return layout::bepo;
            if (unshift == '/' && base == '9') return layout::bepo;
            if (unshift == '*' && base == '0') return layout::bepo;
            if (unshift == '=' && base == '-') return layout::bepo;
            if (unshift == '%' && base == '=') return layout::bepo;
            if (unshift == 'w' && base == ']') return layout::bepo; // ']' -> 'w'
            if (unshift == 'b' && base == 'q') return layout::bepo; // 'Q' -> 'b'
            if (unshift == 233 && base == 'w') return layout::bepo; // 'W' -> 'é'
            if (unshift == 'p' && base == 'e') return layout::bepo; // 'E' -> 'p'
            if (unshift == 'o' && base == 'r') return layout::bepo; // 'R' -> 'o'
            if (unshift == 232 && base == 't') return layout::bepo; // 'T' -> 'è'
            if (unshift == 'v' && base == 'u') return layout::bepo; // 'U' -> 'v'
            if (unshift == 'u' && base == 'i') return layout::bepo; // 'I' -> 'u'
            if (unshift == 'i' && base == 'd') return layout::bepo; // 'D' -> 'i'
            if (unshift == 234 && base == 'z') return layout::bepo; // 'Z' -> 'ê'
            if (unshift == 224 && base == 'x') return layout::bepo; // 'X' -> 'à'
            if (unshift == 'y' && base == 'c') return layout::bepo; // 'C' -> 'y'
            if (unshift == 'x' && base == 'v') return layout::bepo; // 'V' -> 'x'
            if (unshift == 'k' && base == 'b') return layout::bepo; // 'B' -> 'k'
            return layout::undef;
        }
        void remap_bepo(si32& base)
        {
            switch (base)
            {
                // Digit row (punctuation.)
                case '1':  base = '"';  break;
                case '2':  base = 171;  break; // «
                case '3':  base = 187;  break; // »
                case '4':  base = '(';  break;
                case '5':  base = ')';  break;
                case '6':  base = '@';  break;
                case '7':  base = '+';  break;
                case '8':  base = '-';  break;
                case '9':  base = '/';  break;
                case '0':  base = '*';  break;
                case '-':  base = '=';  break;
                case '=':  base = '%';  break;
                // Top row
                case 'q':  base = 'b';  break;
                case 'w':  base = 233;  break; // é
                case 'e':  base = 'p';  break;
                case 'r':  base = 'o';  break;
                case 't':  base = 232;  break; // è
                case 'y':  base = '!';  break;
                case 'u':  base = 'v';  break;
                case 'i':  base = 'd';  break;
                case 'o':  base = 'l';  break;
                case 'p':  base = 'j';  break;
                case '[':  base = 'z';  break;
                case ']':  base = 'w';  break;
                // Home Row
                //case 'a':  base = 'a'; break; // same
                case 's':  base = 'u';  break;
                case 'd':  base = 'i';  break;
                case 'f':  base = 'e';  break;
                case 'g':  base = ',';  break;
                case 'h':  base = 't';  break;
                case 'j':  base = 's';  break;
                case 'k':  base = 'r';  break;
                case 'l':  base = 'n';  break;
                case ';':  base = 'm';  break;
                case '\'': base = 231;  break; // ç (ANSI)
                case '\\': base = 231;  break; // ç (ISO)
                // Bottom row
                case 'z':  base = 234;  break; // ê
                case 'x':  base = 224;  break; // à
                case 'c':  base = 'y';  break;
                case 'v':  base = 'x';  break;
                case 'b':  base = 'k';  break;
                case 'n':  base = '\''; break;
                case 'm':  base = 'q';  break;
                case ',':  base = 'g';  break;
                case '.':  base = 'h';  break;
                case '/':  base = 'f';  break;
                default: break;
            }
        }
        void remap_dvorak(si32& base)
        {
            switch (base)
            {
                // Digits
                case '-':  base = '[';  break;
                case '=':  base = ']';  break;
                // Top
                case 'q':  base = '\''; break;
                case 'w':  base = ',';  break;
                case 'e':  base = '.';  break;
                case 'r':  base = 'p';  break;
                case 't':  base = 'y';  break;
                case 'y':  base = 'f';  break;
                case 'u':  base = 'g';  break;
                case 'i':  base = 'c';  break;
                case 'o':  base = 'r';  break;
                case 'p':  base = 'l';  break;
                case '[':  base = '/';  break;
                case ']':  base = '=';  break;
                // Home Row
                //case 'a':  base = 'a';  break; // same
                case 's':  base = 'o';  break;
                case 'd':  base = 'e';  break;
                case 'f':  base = 'u';  break;
                case 'g':  base = 'i';  break;
                case 'h':  base = 'd';  break;
                case 'j':  base = 'h';  break;
                case 'k':  base = 't';  break;
                case 'l':  base = 'n';  break;
                case ';':  base = 's';  break;
                case '\'': base = '-';  break;
                // Bottom row
                case 'z':  base = ';';  break;
                case 'x':  base = 'q';  break;
                case 'c':  base = 'j';  break;
                case 'v':  base = 'k';  break;
                case 'b':  base = 'x';  break;
                case 'n':  base = 'b';  break;
                //case 'm':  base = 'm';  break; // same
                case ',':  base = 'w';  break;
                case '.':  base = 'v';  break;
                case '/':  base = 'z';  break;
                default: break;
            }
        }
    }

    namespace bindings
    {
        struct binding_t
        {
            text                               chord;
            txts                               sources; // Event source list.
            netxs::sptr<std::pair<ui64, text>> script_ptr;
            netxs::sptr<std::pair<ui64, text>> prerun_ptr;
        };
        using vector = std::vector<binding_t>;

        auto _get_chord_list(qiew chord_str = {})
        {
            auto binary_chord_list = input::key::kmap::chord_list(chord_str);
            if (binary_chord_list.empty())
            {
                if (chord_str) log("%%Unknown key chord or generic event: '%chord%'", prompt::hids, chord_str);
            }
            return binary_chord_list;
        }
        auto get_chords(qiew chord_list_str)
        {
            auto chord_qiew_list = utf::split<true>(chord_list_str, " | ");
            if (chord_qiew_list.size())
            {
                auto head = chord_qiew_list.begin();
                auto tail = chord_qiew_list.end();
                auto fragment = utf::get_trimmed(*head++, ' ');
                auto is_preview = fragment.starts_with(tier::str[tier::preview]);
                auto binary_chord_list = _get_chord_list(fragment);
                if (binary_chord_list.size())
                {
                    while (head != tail)
                    {
                        auto chord_qiew = *head++;
                        auto next_chord_list = _get_chord_list(chord_qiew);
                        auto& c = next_chord_list;
                        binary_chord_list.insert(binary_chord_list.end(), c.begin(), c.end());
                    }
                    return std::pair{ binary_chord_list, is_preview };
                }
            }
            return std::pair{ _get_chord_list(), faux };
        }
        void set_handler(auto reset_handler, base& boss, si32 tier_id, hint event_id, txts const& sources, netxs::sptr<script_ref> script_ptr)
        {
            if (reset_handler) // Reset all script bindings for event_id.
            {
                //log("Erase handlers for event_id:%%", event_id);
                boss.bell::erase_script_handlers(tier_id, event_id);
            }
            else // Set new handler.
            {
                if (sources.empty())
                {
                    //log("Set handler for event_id:%% script: %%", event_id, ansi::hi(script_ptr->script_body_ptr->second));
                    boss.bell::submit_generic(tier_id, event_id, script_ptr);
                }
                else //todo revise: too hacky
                {
                    //log("Deferred setting handler on '%target%' for script: ", sources.front(), ansi::hi(script_ptr->script_body_ptr->second));
                    auto& indexer = boss.indexer;
                    indexer._null_gear_sptr->ui::base::enqueue([&, id = boss.id, tier_id, event_id, sources, script_ptr](auto& /*gear_0*/) // Subscribe on sources (with boss.sensors).
                    {
                        if (auto boss_ptr = indexer._null_gear_sptr->getref(id)) // The boss may already be deleted.
                        {
                            auto& scripting_context = boss_ptr->get_scripting_context();
                            for (auto& src_name : sources)
                            {
                                //log("Set handler on '%target%' for script: ", src_name, ansi::hi(script_ptr->script_body_ptr->second));
                                if (auto target_ptr = indexer.get_target(scripting_context, src_name))
                                {
                                    target_ptr->bell::submit_generic(tier_id, event_id, boss_ptr->sensors, script_ptr);
                                }
                                else
                                {
                                    log("%%Event source '%src_name%' not found", prompt::lua, src_name);
                                }
                            }
                        }
                    });
                }
            }
        }
        auto keybind(base& boss, qiew chord_str, auto&& script_body, netxs::sptr<std::pair<ui64, text>> prerun_body = {}, txts const& sources = {})
        {
            if (!chord_str) return;
            auto [chords, is_preview] = input::bindings::get_chords(chord_str);
            if (chords.size())
            {
                auto script_ptr = ptr::shared<script_ref>(boss.indexer, boss, script_body);
                auto prerun_ptr = prerun_body && prerun_body->second.size() ? ptr::shared<script_ref>(boss.indexer, boss, prerun_body) : netxs::sptr<script_ref>{};
                auto reset_handler = !(script_ptr->script_body_ptr && script_ptr->script_body_ptr->second.size());
                for (auto& binary_chord : chords) if (binary_chord.size()) // Scripts always store their sensors at the boss side, since the lifetime of base::scripting_context depends on the boss.
                {
                    auto k = (byte)binary_chord.front();
                    if (input::key::is_generic(k))
                    {
                        if (binary_chord.size() == sizeof(hint) + 1)
                        {
                            auto tier_id = k & 0x0F;
                            auto event_id = netxs::aligned<hint>(binary_chord.data() + 1);
                            set_handler(reset_handler, boss, tier_id, event_id, sources, script_ptr);
                        }
                        else
                        {
                            log(ansi::err("Broken generic event: ", ansi::hi(utf::debase437(binary_chord))));
                        }
                    }
                    else if (input::key::is_mouse(k) && binary_chord.size() == 3)
                    {
                        auto event_id = (binary_chord[1] << 8) | binary_chord[2];
                        auto tier_id = is_preview ? tier::mousepreview : tier::mouserelease;
                        set_handler(reset_handler, boss, tier_id, event_id, sources, script_ptr);
                    }
                    else // Keybd events.
                    {
                        auto event_id = boss.indexer.get_kbchord_hint(binary_chord);
                        auto tier_id = is_preview ? tier::keybdpreview : tier::keybdrelease;
                        set_handler(reset_handler, boss, tier_id, event_id, sources, script_ptr);
                        if (prerun_ptr)
                        {
                            set_handler(reset_handler, boss, tier::keybd_prerun, event_id, sources, prerun_ptr);
                        }
                    }
                }
            }
        }
        auto keybind(base& boss, auto& bindings)
        {
            for (auto& r : bindings)
            {
                keybind(boss, r.chord, r.script_ptr, r.prerun_ptr, r.sources);
            }
        }
        void dispatch(auto& boss, auto& instance_id, hids& gear, si32 tier_id, hint event_id)
        {
            boss.base::signal(tier_id, event_id, gear);
            if (tier_id == tier::keybdpreview && !boss.bell::accomplished()// && !gear.handled
                && boss.bell::has_handlers(tier::keybdrelease, event_id))
            {
                gear.touched = instance_id;
                boss.base::signal(tier::keybd_prerun, event_id, gear);
            }
        }
        auto load(settings& config, auto& script_list)
        {
            auto bindings = input::bindings::vector{};
            for (auto script_ptr : script_list)
            {
                //todo revise
                //auto script_context = config.settings::push_context(script_ptr);
                auto script_body_ptr = ptr::shared(std::pair<ui64, text>{ 0, config.settings::take_value(script_ptr) });
                auto prerun_body_ptr = ptr::shared(std::pair<ui64, text>{ 0, config.settings::take_value_from(script_ptr, "prerun", ""s) });
                auto on_ptr_list = config.settings::take_ptr_list_of(script_ptr, "on");
                for (auto event_ptr : on_ptr_list)
                {
                    //auto on_context = config.settings::push_context(event_ptr); //todo revise
                    auto on_rec     = config.settings::take_value(event_ptr); // ... on="MouseDown01" ... on="preview:Enter"... .
                    auto sources    = config.settings::take_value_list_of(event_ptr, "source");
                    //if constexpr (debugmode)
                    //{
                    //    for (auto& sourse : sources)
                    //    {
                    //         log("chord='%%' \tpreview=%% source='%%' script=%%", on_rec, (si32)preview, source, ansi::hi(script_body_ptr->second));
                    //    }
                    //}
                    bindings.push_back({ .chord = std::move(on_rec), .sources = std::move(sources), .script_ptr = script_body_ptr, .prerun_ptr = prerun_body_ptr });
                }
            }
            return bindings;
        }
    }
}