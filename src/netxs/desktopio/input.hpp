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

    namespace vkey
    {
        static constexpr auto lbutton  = 0x01; // VK_LBUTTON
        static constexpr auto rbutton  = 0x02; // VK_RBUTTON
        static constexpr auto mbutton  = 0x04; // VK_MBUTTON
        static constexpr auto xbutton1 = 0x05; // VK_XBUTTON1
        static constexpr auto xbutton2 = 0x06; // VK_XBUTTON2

        static constexpr auto shift    = 0x10; // VK_SHIFT
        static constexpr auto control  = 0x11; // VK_CONTROL
        static constexpr auto alt      = 0x12; // VK_MENU
        static constexpr auto lshift   = 0xA0; // VK_LSHIFT
        static constexpr auto rshift   = 0xA1; // VK_RSHIFT
        static constexpr auto lcontrol = 0xA2; // VK_LCONTROL
        static constexpr auto rcontrol = 0xA3; // VK_RCONTROL
        static constexpr auto lalt     = 0xA4; // VK_LMENU
        static constexpr auto ralt     = 0xA5; // VK_RMENU
        static constexpr auto lsuper   = 0x5B; // VK_LWIN
        static constexpr auto rsuper   = 0x5C; // VK_RWIN

        static constexpr auto clear    = 0x0C; // VK_CLEAR
        static constexpr auto enter    = 0x0D; // VK_RETURN
        static constexpr auto pgup     = 0x21; // VK_PRIOR
        static constexpr auto pgdn     = 0x22; // VK_NEXT
        static constexpr auto end      = 0x23; // VK_END
        static constexpr auto home     = 0x24; // VK_HOME
        static constexpr auto left     = 0x25; // VK_LEFT
        static constexpr auto up       = 0x26; // VK_UP
        static constexpr auto right    = 0x27; // VK_RIGHT
        static constexpr auto down     = 0x28; // VK_DOWN
        static constexpr auto insert   = 0x2D; // VK_INSERT
        static constexpr auto del      = 0x2E; // VK_DELETE
        static constexpr auto divide   = 0x6F; // VK_DIVIDE

        static constexpr auto f11      = 0x7A; // VK_F11
        static constexpr auto f12      = 0x7B; // VK_F12

        static constexpr auto key_0    = '0'; // VK_0

        static constexpr auto numlock  = 0x90; // VK_NUMLOCK
        static constexpr auto capslock = 0x14; // VK_CAPITAL
        static constexpr auto scrllock = 0x91; // VK_SCROLL
        static constexpr auto kana     = 0x15; // VK_KANA
        static constexpr auto oem_loya = 0x95; // VK_OEM_FJ_LOYA
        static constexpr auto oem_roya = 0x96; // VK_OEM_FJ_ROYA
        static constexpr auto intl_yen = 0xDC; // VK_OEM_5

        static constexpr auto oem_copy = 0xF2; // VK_OEM_COPY
        static constexpr auto oem_auto = 0xF3; // VK_OEM_AUTO
        static constexpr auto oem_enlw = 0xF4; // VK_OEM_ENLW

        static constexpr auto numpad0  = 0x60; // VK_NUMPAD0
        static constexpr auto numpad1  = 0x61; // VK_NUMPAD1
        static constexpr auto numpad2  = 0x62; // VK_NUMPAD2
        static constexpr auto numpad3  = 0x63; // VK_NUMPAD3
        static constexpr auto numpad4  = 0x64; // VK_NUMPAD4
        static constexpr auto numpad5  = 0x65; // VK_NUMPAD5
        static constexpr auto numpad6  = 0x66; // VK_NUMPAD6
        static constexpr auto numpad7  = 0x67; // VK_NUMPAD7
        static constexpr auto numpad8  = 0x68; // VK_NUMPAD8
        static constexpr auto numpad9  = 0x69; // VK_NUMPAD9
        static constexpr auto numpadD  = 0x6E; // VK_DECIMAL

        static constexpr auto packet   = 0xE7; // VK_PACKET
    }
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

        static constexpr auto latin_klids = std::to_array<ui32>(
        {
            0x00000409, // US
            0x00000412, // Korean
            0x00000468, // Hausa
            0x00000411, // Japanese
            0x00000432, // Setswana
            0x00050408, // Greek Latin
            0x00000415, // Polish (Programmers)
            0x00000404, // Chinese (Traditional) - US
            0x00004009, // English (India)
            0x00020418, // Romanian (Programmers)
            0x00020409, // United States-International
            0x00020405, // Czech Programmers
            0x00000c04, // Chinese (Traditional, Hong Kong S.A.R.) - US
            0x00000804, // Chinese (Simplified) - US
            0x00020426, // Latvian (Standard)
            0x00000481, // Maori
            0x00001404, // Chinese (Traditional, Macao S.A.R.) - US
            0x00010402, // Bulgarian (Latin)
            0x00000470, // Igbo
            0x0000046c, // Sesotho sa Leboa
            0x0000046a, // Yoruba
            0x00001004, // Chinese (Simplified, Singapore) - US
            0x00010426, // Latvian (QWERTY)
            0x00000475, // Hawaiian
            0x00050409, // US English Table for IBM Arabic 238_L
            0x0001045d, // Inuktitut - Naqittaut
            0x0000085d, // Inuktitut - Latin
            0x00001809, // Irish
            0x00011809, // Scottish Gaelic
            0x00000452, // United Kingdom Extended
            0x00000809, // United Kingdom
            0x0000043a, // Maltese 47-Key
            0x0001043a, // Maltese 48-Key
            0x00001009, // Canadian French
            0x00011009, // Canadian Multilingual Standard
            0x00000c0c, // Canadian French (Legacy)
            0x00040408, // Greek (319) Latin
            0x00000416, // Portuguese (Brazil ABNT)
            0x00010418, // Romanian (Standard)
            0x00010416, // Portuguese (Brazil ABNT2)
            0x0001040a, // Spanish Variation
            0x00000816, // Portuguese
            0x0000041c, // Albanian
            0x00000410, // Italian
            0x00010410, // Italian (142)
            0x0000041d, // Swedish
            0x0000083b, // Swedish with Sami
            0x00000413, // Dutch
            0x00000414, // Norwegian
            0x00000474, // Guarani
            0x00000406, // Danish
            0x0000046f, // Greenlandic
            0x0000043b, // Norwegian with Sami
            0x0000080a, // Latin American
            0x0001083b, // Finnish with Sami
            0x0000040a, // Spanish
            0x0000040b, // Finnish
            0x00030408, // Greek (220) Latin
            0x00000438, // Faeroese
            0x00000425, // Estonian
            0x0000040f, // Icelandic
            0x00010427, // Lithuanian
            0x0001040e, // Hungarian 101-key
            0x0000100c, // Swiss French
            0x00000424, // Slovenian
            0x0001042e, // Sorbian Extended
            0x00000407, // German
            0x0000081a, // Serbian (Latin)
            0x00010407, // German (IBM)
            0x00010415, // Polish (214)
            0x0000042e, // Sorbian Standard (Legacy)
            0x0002042e, // Sorbian Standard
            0x00000807, // Swiss German
            0x0000041a, // Standard
            0x0000046e, // Luxembourgish
            0x00000418, // Romanian (Legacy)
            0x0000040e, // Hungarian
            0x0000041f, // Turkish Q
            0x0000042a, // Vietnamese
            0x0001041b, // Slovak (QWERTY)
            0x00010405, // Czech (QWERTY)
            0x00000405, // Czech
            0x0000041b, // Slovak
            0x0000040c, // French
            0x0000085f, // Central Atlas Tamazight
            0x00000488, // Wolof
            0x0001080c, // Belgian (Comma)
            0x0000080c, // Belgian French
            0x00000813, // Belgian (Period)
            0x00010409, // United States-Dvorak
            0x00020427, // Lithuanian Standard
            0x0001041f, // Turkish F
            0x00040409, // United States-Dvorak for right hand
            0x00030409, // United States-Dvorak for left hand
        });
        // modul=161(0x000000a1) | prime=221868(0x000362ac) | shift=374331(0x0005b63b) | MIN=2 | MAX=159 | DELTA=157
        static constexpr auto klid_prime = 221868u;
        static constexpr auto klid_shift = 374331u;
        static constexpr auto klid_modul = 161u;
        static constexpr auto klid_hash(si32 klid)
        {
            auto uniq = (((ui32)klid ^ (ui32)klid_prime) * (ui32)klid_shift) % (ui32)klid_modul;
            return uniq;
        }
        static constexpr auto supported_klids = [] // This won't compile if there are collisions.
        {
            struct layout
            {
                si32 klid;
                si32 index;
            };
            auto index = 0;
            auto klids = std::array<layout, input::key::klid_modul>{};
            for (auto klid : input::key::latin_klids) // Check hash collisions.
            {
                auto uniq = input::key::klid_hash(klid);
                if (klids[uniq].klid != 0)
                {
                    log("Hash collision detected. Try to update prime/shift/modul.");
                }
                klids[uniq] = { (si32)klid, index++ };
            }
            return klids;
        }();
        static constexpr auto is_layout_supported(si32 klid)
        {
            return input::key::supported_klids[input::key::klid_hash(klid)].klid == klid;
        }
        //  15                       9   8   7                           0
        // +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
        // | L   L   L   L   L   L   L | E | S   S   S   S   S   S   S   S |
        // +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
        //  \_________________________/  |  \_____________________________/
        //      7 bit Layout Index       |      8 bit Base Scancode
        //        (0 - 93)               |        (0 - 255)
        //                 Extended Bit (0 or 1)
        auto key_hash(si32 klid, si32 scan, bool extflag)
        {
            assert(input::key::supported_klids[input::key::klid_hash(klid)].klid == klid);
            auto hash = (ui16)((scan & 0xFF) | ((si32)extflag << 8) | (input::key::supported_klids[input::key::klid_hash(klid)].index << 9));
            return hash;
        }

        // Notes:
        //  IsoLevel5Shift: 5th-level of kb layout (mathematical signs, Greek letters). Physical keyboards don't have this key; in Linux, it's usually remapped to Caps Lock or the right Ctrl key.
        //  Hyper:          Users specifically create Hyper (for example, by remapping Caps Lock) to bind hotkeys, which are guaranteed to not interact with anything.
        #define key_list \
          /* ID  Input Name                 Generic              Literal  Uc  KKPdef KKPsuffix KKPascii wCtl  PhysicalCode */\
            X(0   , 1, undef              , "undef"             , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(1   , 0, invalid            , "invalid"           , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(2   , 0, LeftCtrl           , "Ctrl"              , ""    , 0     , 57442, 'u', -1    , -1    , "001da2281da2081da2261da2121da20c1da21e1da21c1da2061da2241da2041da20a1da2101da2161da2201da2181da20e1da22a1da21a1da2221da2141da27a1da22c1da22e1da2021da2301da2321da2341da23a1da23c1da2381da2361da29c1da2401da23e1da24c1da2421da24e1da24a1da2461da2541da2441da25c1da25a1da2681da2621da2661da2481da2721da26c1da2501da29e1da2761da2a01da2521da2581da2561da26e1da2601da26a1da2701da2741da25e1da2641da2a21da2781da28c1da28e1da2821da2a41da2861da28a1da2981da29a1da2901da27e1da2941da2881da2841da27c1da2921da2801da2961da2aa1da2a81da2a61da2ae1da2b01da2ac1da2b41da2b21da2b61da2b81da2ba1da2")\
            X( 3  , 0, RightCtrl          , "Ctrl"              , ""    , 0     , 57448, 'u', -1    , -1    , "011da3291da3091da3271da3131da30d1da31f1da31d1da3071da3251da3051da30b1da3111da3171da3211da3191da30f1da32b1da31b1da3231da3151da37b1da32d1da32f1da3031d19311da3331da3351da33b1da33d1da3391da3371da39d1da3411da33f1da34d1da3431da34f1da34b1da3471da3551da3451ddf5d1da35b1da3691da3631da3671da3491da3731da36d1da3511da39f1da3771da3a11da3531da3591da3571da36f1da3611da36b1da3711da3751da35f1da3651da3a31da3791da38d1da38f1da3831da3a51da3871da38b1da3991da39b1da3911da37f1da3951da3891da3851da37d1da3931da3811da3971da3ab1da3a91da3a71da3af1da3b11da3ad1da3b51da3b31da3b71da3b91da3bb1da3")\
            X(4   , 0, LeftAlt            , "Alt"               , ""    , 0     , 57443, 'u', -1    , -1    , "0038a42838a40838a42638a41238a40c38a41e38a41c38a40638a42438a40438a40a38a41038a41638a42038a41838a40e38a42a38a41a38a42238a41438a47a38a42c38a42e38a40238a43038a43238a43438a43a38a43c38a43838a43638a49c38a44038a43e38a44c38a44238a44e38a44a38a44638a45438a44438a45c38a45a38a46838a46238a46638a44838a47238a46c38a45038a49e38a47638a4a038a45238a45838a45638a46e38a46038a46a38a47038a47438a45e38a46438a4a238a47838a48c38a48e38a48238a4a438a48638a48a38a49838a49a38a49038a47e38a49438a48838a48438a47c38a49238a48038a49638a4aa38a4a838a4a638a4ae38a4b038a4ac38a4b438a4b238a4b638a4b838a4ba38a4")\
            X( 5  , 0, RightAlt           , "Alt"               , ""    , 0     , 57449, 'u', -1    , -1    , "0138a52938a50938a52738a51338a50d38a51f38a51d38a50738a52538a50538a50b38a51138a51738a52138a51938a50f38a52b38a51b38a52338a51538a57b38a52d38a52f38a50338153138a53338a53538a53b38a53d38a53938a53738a59d38a54138a53f38a54d38a54338a54f38a54b38a54738a55538a54538a55d38a55b38a56938a56338a56738a54938a57338a56d38a55138a59f38a57738a5a138a55338a55938a55738a56f38a56138a56b38a57138a57538a55f38a56538a5a338a57938a58d38a58f38a58338a5a538a58738a58b38a59938a59b38a59138a57f38a59538a58938a58538a57d38a59338a58138a59738a5ab38a5a938a5a738a5af38a5b138a5ad38a5b538a5b338a5b738a5b938a5bb38a5")\
            X(6   , 0, LeftShift          , "Shift"             , ""    , 0     , 57441, 'u', -1    , -1    , "002aa0282aa0082aa0262aa0122aa00c2aa01e2aa01c2aa0062aa0242aa0042aa00a2aa0102aa0162aa0202aa0182aa00e2aa02a2aa01a2aa0222aa0142aa07a2aa02c2aa02e2aa0022aa0302aa0322aa0342aa03a2aa03c2aa0382aa0362aa09c2aa0402aa03e2aa04c2aa0422aa04e2aa04a2aa0462aa0542aa0442aa05c2aa05a2aa0682aa0622aa0662aa0482aa0722aa06c2aa0502aa09e2aa0762aa0a02aa0522aa0582aa0562aa06e2aa0602aa06a2aa0702aa0742aa05e2aa0642aa0a22aa0782aa08c2aa08e2aa0822aa0a42aa0862aa08a2aa0982aa09a2aa0902aa07e2aa0942aa0882aa0842aa07c2aa0922aa0802aa0962aa0aa2aa0a82aa0a62aa0ae2aa0b02aa0ac2aa0b42aa0b22aa0b62aa0b82aa0ba2aa0")\
            X( 7  , 0, RightShift         , "Shift"             , ""    , 0     , 57447, 'u', -1    , -1    , "0036a12836a10836a12636a11236a10c36a11e36a11c36a10636a12436a10436a10a36a11036a11636a12036a11836a10e36a12a36a11a36a12236a11436a17a36a12c36a12e36a10236a13036a13236a13436a13a36a13c36a13836a13636a19c36a14036a13e36a14c36a14236a14e36a14a36a14636a15436a14436a15c36a15a36a16836a16236a16636a14836a17236a16c36a15036a19e36a17636a1a036a15236a15836a15636a16e36a16036a16a36a17036a17436a15e36a16436a1a236a17836a18c36a18e36a18236a1a436a18636a18a36a19836a19a36a19036a17e36a19436a18836a18436a17c36a19236a18036a19636a1aa36a1a836a1a636a1ae36a1b036a1ac36a1b436a1b236a1b636a1b836a1ba36a1")\
            X(8   , 0, LeftSuper          , "Super"             , ""    , 0     , 57444, 'u', -1    , -1    , "015b5b295b5b095b5b275b5b135b5b0d5b5b1f5b5b1d5b5b075b5b255b5b055b5b0b5b5b115b5b175b5b215b5b195b5b0f5b5b2b5b5b1b5b5b235b5b155b5b7b5b5b2d5b5b2f5b5b035b5b315b5b335b5b355b5b3b5b5b3d5b5b395b5b375b5b9d5b5b415b5b3f5b5b4d5b5b435b5b4f5b5b4b5b5b475b5b555b5b455b5b5d5b5b5b5b5b695b5b635b5b675b5b735b5b6d5b5b515b5b9f5b5b775b5ba15b5b535b5b595b5b575b5b6f5b5b615b5b6b5b5b715b5b755b5b5f5b5b655b5ba35b5b795b5b8d5b5b8f5b5b835b5ba55b5b875b5b8b5b5b995b5b9b5b5b915b5b7f5b5b955b5b895b5b855b5bab5b5ba95b5ba75b5baf5b5bb15b5bad5b5bb55b5bb35b5bb75b5bb95b5bbb5b5b")\
            X( 9  , 0, RightSuper         , "Super"             , ""    , 0     , 57450, 'u', -1    , -1    , "015c5c295c5c095c5c275c5c135c5c0d5c5c1f5c5c1d5c5c075c5c255c5c055c5c0b5c5c115c5c175c5c215c5c195c5c0f5c5c2b5c5c1b5c5c235c5c155c5c7b5c5c2d5c5c2f5c5c035c5c315c5c335c5c355c5c3b5c5c3d5c5c395c5c375c5c9d5c5c415c5c3f5c5c4d5c5c435c5c4f5c5c4b5c5c475c5c555c5c455c5c5d5c5c5b5c5c695c5c635c5c675c5c495c5c735c5c6d5c5c515c5c9f5c5c775c5ca15c5c535c5c595c5c575c5c6f5c5c615c5c6b5c5c715c5c755c5c5f5c5c655c5ca35c5c795c5c8d5c5c8f5c5c835c5ca55c5c875c5c8b5c5c995c5c9b5c5c915c5c7f5c5c955c5c895c5c855c5c7d5c5c935c5c815c5c975c5cab5c5ca95c5ca75c5caf5c5cb15c5cad5c5cb55c5cb35c5cb75c5cb95c5cbb5c5c")\
            X(10  , 0, LeftHyper          , "Hyper"             , ""    , 0     , 57445, 'u', -1    , -1    , "")\
            X( 11 , 0, RightHyper         , "Hyper"             , ""    , 0     , 57451, 'u', -1    , -1    , "")\
            X(12  , 0, AltGR              , "AltGr"             , ""    , 0     , 57453, 'u', -1    , -1    , "")\
            X(14  , 0, NumLock            , "NumLock"           , ""    , 0     , 57360, 'u', -1    , -1    , "0145902945900945902745901345900d45901f45901d45900745902545900545900b45901145901745902145901945900f45902b45901b45902345901545907b45902d45902f45900345903145903345903545903b45903d45903945903745909d45904145903f45904d45904345904f45904b45904745905545904545905d45905b45906945906345906745904945907345906d45905145909f4590774590a145905345905945905745906f45906145906b45907145907545905f4590654590a345907945908d45908f4590834590a545908745908b45909945909b45909145907f45909545908945908545907d4590934590814590974590ab4590a94590a74590af4590b14590ad4590b54590b34590b74590b94590bb4590")\
            X(16  , 0, CapsLock           , "CapsLock"          , ""    , 0     , 57358, 'u', -1    , -1    , "003a14283a14083a14263a14123a140c3a141e3a141c3a14063a14243a14043a140a3a14103a14163a14203a14183a140e3a142a3a141a3a14223a14143a147a3a142c3a142e3a14023a14303a14323a14343a143a3a143c3a14383a14363a149c3a14403a143e3a144c3a14423a144e3a144a3a14463a14543a14443a145c3a145a3a14683a14623a14663a14483a14723a146c3a14503a149e3a14763a14a03a14523a14583a14563a146e3a14603a146a3a14703a14743a145e3a14643a14a23a14783a148c3a148e3a14823a14a43a14863a148a3a14983a149a3a14903a147e3a14943a14883a14843a147c3a14923a14803a14963a14aa3a14a83a14a63a14ae3a14b03a14ac3a14b43a14b23a14b63a14b83a14ba3a14")\
            X(18  , 0, ScrollLock         , "ScrollLock"        , ""    , 0     , 57359, 'u', -1    , -1    , "0046912846910846912646911246910c46911e46911c46910646912446910446910a46911046911646912046911846910e46912a46911a46912246911446917a46912c46912e46910246913046913246913446913a46913c46913846913646919c46914046913e46914c46914246914e46914a46914646915446914446915c46915a46916846916246916646914846917246916c46915046919e4691764691a046915246915846915646916e46916046916a46917046917446915e4691644691a246917846918c46918e4691824691a446918646918a46919846919a46919046917e46919446918846918446917c4691924691804691964691aa4691a84691a64691ae4691b04691ac4691b44691b24691b64691b84691ba4691")\
            X(20  , 0, Kana               , "Kana"              , ""    , 0     , 0    , 'u', -1    , -1    , "0670f2")\
            X(22  , 0, Henkan             , "Henkan"            , ""    , 0     , 0    , 'u', -1    , -1    , "06791c")\
            X(24  , 0, Muhenkan           , "Muhenkan"          , ""    , 0     , 0    , 'u', -1    , -1    , "067b1d")\
            X(26  , 0, Hanja              , "Hanja"             , ""    , 0     , 0    , 'u', -1    , -1    , "02f119")\
            X(28  , 0, Hanguel            , "Hanguel"           , ""    , 0     , 0    , 'u', -1    , -1    , "02f215")\
            X(30  , 0, IsoLevel5Shift     , "IsoLevel5Shift"    , ""    , 0     , 57454, 'u', -1    , -1    , "")\
            X(32  , 0, Apps               , "Apps"              , ""    , 0     , 57363, 'u', -1    , -1    , "015d5d295d5d095d5d275d5d135d5d0d5d5d1f5d5d1d5d5d075d5d255d5d055d5d0b5d5d115d5d175d5d215d5d195d5d0f5d5d2b5d5d1b5d5d235d5d155d5d7b5d5d2d5d5d2f5d5d035d5d315d5d335d5d355d5d3b5d5d3d5d5d395d5d375d5d9d5d5d415d5d3f5d5d4d5d5d435d5d4f5d5d4b5d5d475d5d555d5d455d5d5d5d5d5b5d5d695d5d635d5d675d5d735d5d6d5d5d515d5d9f5d5d775d5da15d5d535d5d595d5d575d5d6f5d5d615d5d6b5d5d715d5d755d5d5f5d5d655d5da35d5d795d5d8d5d5d8f5d5d835d5da55d5d875d5d8b5d5d995d5d9b5d5d915d5d7f5d5d955d5d895d5d855d5dab5d5da95d5da75d5daf5d5db15d5dad5d5db55d5db35d5db75d5db95d5dbb5d5d")\
            X(34  , 0, Select             , "Select"            , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(36  , 0, Fn                 , "Fn"                , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(38  , 0, F1                 , "F1"                , ""    , 0     , 11   , '~', -1    , -1    , "003b70283b70083b70263b70123b700c3b701e3b701c3b70063b70243b70043b700a3b70103b70163b70203b70183b700e3b702a3b701a3b70223b70143b707a3b702c3b702e3b70023b70303b70323b70343b703a3b703c3b70383b70363b709c3b70403b703e3b704c3b70423b704e3b704a3b70463b70543b70443b705c3b705a3b70683b70623b70663b70483b70723b706c3b70503b709e3b70763b70a03b70523b70583b70563b706e3b70603b706a3b70703b70743b705e3b70643b70a23b70783b708c3b708e3b70823b70a43b70863b708a3b70983b709a3b70903b707e3b70943b70883b70843b707c3b70923b70803b70963b70aa3b70a83b70a63b70ae3b70b03b70ac3b70b43b70b23b70b63b70b83b70ba3b70")\
            X(40  , 0, F2                 , "F2"                , ""    , 0     , 12   , '~', -1    , -1    , "003c71283c71083c71263c71123c710c3c711e3c711c3c71063c71243c71043c710a3c71103c71163c71203c71183c710e3c712a3c711a3c71223c71143c717a3c712c3c712e3c71023c71303c71323c71343c713a3c713c3c71383c71363c719c3c71403c713e3c714c3c71423c714e3c714a3c71463c71543c71443c715c3c715a3c71683c71623c71663c71483c71723c716c3c71503c719e3c71763c71a03c71523c71583c71563c716e3c71603c716a3c71703c71743c715e3c71643c71a23c71783c718c3c718e3c71823c71a43c71863c718a3c71983c719a3c71903c717e3c71943c71883c71843c717c3c71923c71803c71963c71aa3c71a83c71a63c71ae3c71b03c71ac3c71b43c71b23c71b63c71b83c71ba3c71")\
            X(42  , 0, F3                 , "F3"                , ""    , 0     , 13   , '~', -1    , -1    , "003d72283d72083d72263d72123d720c3d721e3d721c3d72063d72243d72043d720a3d72103d72163d72203d72183d720e3d722a3d721a3d72223d72143d727a3d722c3d722e3d72023d72303d72323d72343d723a3d723c3d72383d72363d729c3d72403d723e3d724c3d72423d724e3d724a3d72463d72543d72443d725c3d725a3d72683d72623d72663d72483d72723d726c3d72503d729e3d72763d72a03d72523d72583d72563d726e3d72603d726a3d72703d72743d725e3d72643d72a23d72783d728c3d728e3d72823d72a43d72863d728a3d72983d729a3d72903d727e3d72943d72883d72843d727c3d72923d72803d72963d72aa3d72a83d72a63d72ae3d72b03d72ac3d72b43d72b23d72b63d72b83d72ba3d72")\
            X(44  , 0, F4                 , "F4"                , ""    , 0     , 14   , '~', -1    , -1    , "003e73283e73083e73263e73123e730c3e731e3e731c3e73063e73243e73043e730a3e73103e73163e73203e73183e730e3e732a3e731a3e73223e73143e737a3e732c3e732e3e73023e73303e73323e73343e733a3e733c3e73383e73363e739c3e73403e733e3e734c3e73423e734e3e734a3e73463e73543e73443e735c3e735a3e73683e73623e73663e73483e73723e736c3e73503e739e3e73763e73a03e73523e73583e73563e736e3e73603e736a3e73703e73743e735e3e73643e73a23e73783e738c3e738e3e73823e73a43e73863e738a3e73983e739a3e73903e737e3e73943e73883e73843e737c3e73923e73803e73963e73aa3e73a83e73a63e73ae3e73b03e73ac3e73b43e73b23e73b63e73b83e73ba3e73")\
            X(46  , 0, F5                 , "F5"                , ""    , 0     , 15   , '~', -1    , -1    , "003f74283f74083f74263f74123f740c3f741e3f741c3f74063f74243f74043f740a3f74103f74163f74203f74183f740e3f742a3f741a3f74223f74143f747a3f742c3f742e3f74023f74303f74323f74343f743a3f743c3f74383f74363f749c3f74403f743e3f744c3f74423f744e3f744a3f74463f74543f74443f745c3f745a3f74683f74623f74663f74483f74723f746c3f74503f749e3f74763f74a03f74523f74583f74563f746e3f74603f746a3f74703f74743f745e3f74643f74a23f74783f748c3f748e3f74823f74a43f74863f748a3f74983f749a3f74903f747e3f74943f74883f74843f747c3f74923f74803f74963f74aa3f74a83f74a63f74ae3f74b03f74ac3f74b43f74b23f74b63f74b83f74ba3f74")\
            X(48  , 0, F6                 , "F6"                , ""    , 0     , 17   , '~', -1    , -1    , "0040752840750840752640751240750c40751e40751c40750640752440750440750a40751040751640752040751840750e40752a40751a40752240751440757a40752c40752e40750240753040753240753440753a40753c40753840753640759c40754040753e40754c40754240754e40754a40754640755440754440755c40755a40756840756240756640754840757240756c40755040759e4075764075a040755240755840755640756e40756040756a40757040757440755e4075644075a240757840758c40758e4075824075a440758640758a40759840759a40759040757e40759440758840758440757c4075924075804075964075aa4075a84075a64075ae4075b04075ac4075b44075b24075b64075b84075ba4075")\
            X(50  , 0, F7                 , "F7"                , ""    , 0     , 18   , '~', -1    , -1    , "0041762841760841762641761241760c41761e41761c41760641762441760441760a41761041761641762041761841760e41762a41761a41762241761441767a41762c41762e41760241763041763241763441763a41763c41763841763641769c41764041763e41764c41764241764e41764a41764641765441764441765c41765a41766841766241766641764841767241766c41765041769e4176764176a041765241765841765641766e41766041766a41767041767441765e4176644176a241767841768c41768e4176824176a441768641768a41769841769a41769041767e41769441768841768441767c4176924176804176964176aa4176a84176a64176ae4176b04176ac4176b44176b24176b64176b84176ba4176")\
            X(52  , 0, F8                 , "F8"                , ""    , 0     , 19   , '~', -1    , -1    , "0042772842770842772642771242770c42771e42771c42770642772442770442770a42771042771642772042771842770e42772a42771a42772242771442777a42772c42772e42770242773042773242773442773a42773c42773842773642779c42774042773e42774c42774242774e42774a42774642775442774442775c42775a42776842776242776642774842777242776c42775042779e4277764277a042775242775842775642776e42776042776a42777042777442775e4277644277a242777842778c42778e4277824277a442778642778a42779842779a42779042777e42779442778842778442777c4277924277804277964277aa4277a84277a64277ae4277b04277ac4277b44277b24277b64277b84277ba4277")\
            X(54  , 0, F9                 , "F9"                , ""    , 0     , 20   , '~', -1    , -1    , "0043782843780843782643781243780c43781e43781c43780643782443780443780a43781043781643782043781843780e43782a43781a43782243781443787a43782c43782e43780243783043783243783443783a43783c43783843783643789c43784043783e43784c43784243784e43784a43784643785443784443785c43785a43786843786243786643784843787243786c43785043789e4378764378a043785243785843785643786e43786043786a43787043787443785e4378644378a243787843788c43788e4378824378a443788643788a43789843789a43789043787e43789443788843788443787c4378924378804378964378aa4378a84378a64378ae4378b04378ac4378b44378b24378b64378b84378ba4378")\
            X(56  , 0, F10                , "F10"               , ""    , 0     , 21   , '~', -1    , -1    , "0044792844790844792644791244790c44791e44791c44790644792444790444790a44791044791644792044791844790e44792a44791a44792244791444797a44792c44792e44790244793044793244793444793a44793c44793844793644799c44794044793e44794c44794244794e44794a44794644795444794444795c44795a44796844796244796644794844797244796c44795044799e4479764479a044795244795844795644796e44796044796a44797044797444795e4479644479a244797844798c44798e4479824479a444798644798a44799844799a44799044797e44799444798844798444797c4479924479804479964479aa4479a84479a64479ae4479b04479ac4479b44479b24479b64479b84479ba4479")\
            X(58  , 0, F11                , "F11"               , ""    , 0     , 23   , '~', -1    , -1    , "00577a28577a08577a26577a12577a0c577a1e577a1c577a06577a24577a04577a0a577a10577a16577a20577a18577a0e577a2a577a1a577a22577a14577a7a577a2c577a2e577a02577a30577a32577a34577a3a577a3c577a38577a36577a9c577a40577a3e577a4c577a42577a4e577a4a577a46577a54577a44577a5c577a5a577a68577a62577a66577a48577a72577a6c577a50577a9e577a76577aa0577a52577a58577a56577a6e577a60577a6a577a70577a74577a5e577a64577aa2577a78577a8c577a8e577a82577aa4577a86577a8a577a98577a9a577a90577a7e577a94577a88577a84577a7c577a92577a80577a96577aaa577aa8577aa6577aae577ab0577aac577ab4577ab2577ab6577ab8577aba577a")\
            X(60  , 0, F12                , "F12"               , ""    , 0     , 24   , '~', -1    , -1    , "00587b28587b08587b26587b12587b0c587b1e587b1c587b06587b24587b04587b0a587b10587b16587b20587b18587b0e587b2a587b1a587b22587b14587b7a587b2c587b2e587b02587b30587b32587b34587b3a587b3c587b38587b36587b9c587b40587b3e587b4c587b42587b4e587b4a587b46587b54587b44587b5c587b5a587b68587b62587b66587b48587b72587b6c587b50587b9e587b76587ba0587b52587b58587b56587b6e587b60587b6a587b70587b74587b5e587b64587ba2587b78587b8c587b8e587b82587ba4587b86587b8a587b98587b9a587b90587b7e587b94587b88587b84587b7c587b92587b80587b96587baa587ba8587ba6587bae587bb0587bac587bb4587bb2587bb6587bb8587bba587b")\
            X(62  , 0, F13                , "F13"               , ""    , 0     , 57376, 'u', -1    , -1    , "00647c28647c08647c26647c12647c0c647c1e647c1c647c06647c24647c04647c0a647c10647c16647c20647c18647c0e647c2a647c1a647c22647c14647c7a647c2c647c2e647c02647c30647c32647c34647c3a647c3c647c38647c36647c9c647c40647c3e647c4c647c42647c4e647c4a647c46647c54647c44647c5c647c5a647c68647c62647c66647c48647c72647c6c647c50647c9e647c76647ca0647c52647c58647c56647c6e647c60647c6a647c70647c74647c5e647c64647ca2647c78647c8c647c8e647c82647ca4647c86647c8a647c98647c9a647c90647c7e647c94647c88647c84647c7c647c92647c80647c96647caa647ca8647ca6647cae647cb0647cac647cb4647cb2647cb6647cb8647cba647c")\
            X(64  , 0, F14                , "F14"               , ""    , 0     , 57377, 'u', -1    , -1    , "00657d28657d08657d26657d12657d0c657d1e657d1c657d06657d24657d04657d0a657d10657d16657d20657d18657d0e657d2a657d1a657d22657d14657d7a657d2c657d2e657d02657d30657d32657d34657d3a657d3c657d38657d36657d9c657d40657d3e657d4c657d42657d4e657d4a657d46657d54657d44657d5c657d5a657d68657d62657d66657d48657d72657d6c657d50657d9e657d76657da0657d52657d58657d56657d6e657d60657d6a657d70657d74657d5e657d64657da2657d78657d8c657d8e657d82657da4657d86657d8a657d98657d9a657d90657d7e657d94657d88657d84657d7c657d92657d80657d96657daa657da8657da6657dae657db0657dac657db4657db2657db6657db8657dba657d")\
            X(66  , 0, F15                , "F15"               , ""    , 0     , 57378, 'u', -1    , -1    , "00667e28667e08667e26667e12667e0c667e1e667e1c667e06667e24667e04667e0a667e10667e16667e20667e18667e0e667e2a667e1a667e22667e14667e7a667e2c667e2e667e02667e30667e32667e34667e3a667e3c667e38667e36667e9c667e40667e3e667e4c667e42667e4e667e4a667e46667e54667e44667e5c667e5a667e68667e62667e66667e48667e72667e6c667e50667e9e667e76667ea0667e52667e58667e56667e6e667e60667e6a667e70667e74667e5e667e64667ea2667e78667e8c667e8e667e82667ea4667e86667e8a667e98667e9a667e90667e7e667e94667e88667e84667e7c667e92667e80667e96667eaa667ea8667ea6667eae667eb0667eac667eb4667eb2667eb6667eb8667eba667e")\
            X(68  , 0, F16                , "F16"               , ""    , 0     , 57379, 'u', -1    , -1    , "00677f28677f08677f26677f12677f0c677f1e677f1c677f06677f24677f04677f0a677f10677f16677f20677f18677f0e677f2a677f1a677f22677f14677f7a677f2c677f2e677f02677f30677f32677f34677f3a677f3c677f38677f36677f9c677f40677f3e677f4c677f42677f4e677f4a677f46677f54677f44677f5c677f5a677f68677f62677f66677f48677f72677f6c677f50677f9e677f76677fa0677f52677f58677f56677f6e677f60677f6a677f70677f74677f5e677f64677fa2677f78677f8c677f8e677f82677fa4677f86677f8a677f98677f9a677f90677f7e677f94677f88677f84677f7c677f92677f80677f96677faa677fa8677fa6677fae677fb0677fac677fb4677fb2677fb6677fb8677fba677f")\
            X(70  , 0, F17                , "F17"               , ""    , 0     , 57380, 'u', -1    , -1    , "0068802868800868802668801268800c68801e68801c68800668802468800468800a68801068801668802068801868800e68802a68801a68802268801468807a68802c68802e68800268803068803268803468803a68803c68803868803668809c68804068803e68804c68804268804e68804a68804668805468804468805c68805a68806868806268806668804868807268806c68805068809e6880766880a068805268805868805668806e68806068806a68807068807468805e6880646880a268807868808c68808e6880826880a468808668808a68809868809a68809068807e68809468808868808468807c6880926880806880966880aa6880a86880a66880ae6880b06880ac6880b46880b26880b66880b86880ba6880")\
            X(72  , 0, F18                , "F18"               , ""    , 0     , 57381, 'u', -1    , -1    , "0069812869810869812669811269810c69811e69811c69810669812469810469810a69811069811669812069811869810e69812a69811a69812269811469817a69812c69812e69810269813069813269813469813a69813c69813869813669819c69814069813e69814c69814269814e69814a69814669815469814469815c69815a69816869816269816669814869817269816c69815069819e6981766981a069815269815869815669816e69816069816a69817069817469815e6981646981a269817869818c69818e6981826981a469818669818a69819869819a69819069817e69819469818869818469817c6981926981806981966981aa6981a86981a66981ae6981b06981ac6981b46981b26981b66981b86981ba6981")\
            X(74  , 0, F19                , "F19"               , ""    , 0     , 57382, 'u', -1    , -1    , "006a82286a82086a82266a82126a820c6a821e6a821c6a82066a82246a82046a820a6a82106a82166a82206a82186a820e6a822a6a821a6a82226a82146a827a6a822c6a822e6a82026a82306a82326a82346a823a6a823c6a82386a82366a829c6a82406a823e6a824c6a82426a824e6a824a6a82466a82546a82446a825c6a825a6a82686a82626a82666a82486a82726a826c6a82506a829e6a82766a82a06a82526a82586a82566a826e6a82606a826a6a82706a82746a825e6a82646a82a26a82786a828c6a828e6a82826a82a46a82866a828a6a82986a829a6a82906a827e6a82946a82886a82846a827c6a82926a82806a82966a82aa6a82a86a82a66a82ae6a82b06a82ac6a82b46a82b26a82b66a82b86a82ba6a82")\
            X(76  , 0, F20                , "F20"               , ""    , 0     , 57383, 'u', -1    , -1    , "006b83286b83086b83266b83126b830c6b831e6b831c6b83066b83246b83046b830a6b83106b83166b83206b83186b830e6b832a6b831a6b83226b83146b837a6b832c6b832e6b83026b83306b83326b83346b833a6b833c6b83386b83366b839c6b83406b833e6b834c6b83426b834e6b834a6b83466b83546b83446b835c6b835a6b83686b83626b83666b83486b83726b836c6b83506b839e6b83766b83a06b83526b83586b83566b836e6b83606b836a6b83706b83746b835e6b83646b83a26b83786b838c6b838e6b83826b83a46b83866b838a6b83986b839a6b83906b837e6b83946b83886b83846b837c6b83926b83806b83966b83aa6b83a86b83a66b83ae6b83b06b83ac6b83b46b83b26b83b66b83b86b83ba6b83")\
            X(78  , 0, F21                , "F21"               , ""    , 0     , 57384, 'u', -1    , -1    , "006c84286c84086c84266c84126c840c6c841e6c841c6c84066c84246c84046c840a6c84106c84166c84206c84186c840e6c842a6c841a6c84226c84146c847a6c842c6c842e6c84026c84306c84326c84346c843a6c843c6c84386c84366c849c6c84406c843e6c844c6c84426c844e6c844a6c84466c84546c84446c845c6c845a6c84686c84626c84666c84486c84726c846c6c84506c849e6c84766c84a06c84526c84586c84566c846e6c84606c846a6c84706c84746c845e6c84646c84a26c84786c848c6c848e6c84826c84a46c84866c848a6c84986c849a6c84906c847e6c84946c84886c84846c847c6c84926c84806c84966c84aa6c84a86c84a66c84ae6c84b06c84ac6c84b46c84b26c84b66c84b86c84ba6c84")\
            X(80  , 0, F22                , "F22"               , ""    , 0     , 57385, 'u', -1    , -1    , "006d85286d85086d85266d85126d850c6d851e6d851c6d85066d85246d85046d850a6d85106d85166d85206d85186d850e6d852a6d851a6d85226d85146d857a6d852c6d852e6d85026d85306d85326d85346d853a6d853c6d85386d85366d859c6d85406d853e6d854c6d85426d854e6d854a6d85466d85546d85446d855c6d855a6d85686d85626d85666d85486d85726d856c6d85506d859e6d85766d85a06d85526d85586d85566d856e6d85606d856a6d85706d85746d855e6d85646d85a26d85786d858c6d858e6d85826d85a46d85866d858a6d85986d859a6d85906d857e6d85946d85886d85846d857c6d85926d85806d85966d85aa6d85a86d85a66d85ae6d85b06d85ac6d85b46d85b26d85b66d85b86d85ba6d85")\
            X(82  , 0, F23                , "F23"               , ""    , 0     , 57386, 'u', -1    , -1    , "006e86286e86086e86266e86126e860c6e861e6e861c6e86066e86246e86046e860a6e86106e86166e86206e86186e860e6e862a6e861a6e86226e86146e867a6e862c6e862e6e86026e86306e86326e86346e863a6e863c6e86386e86366e869c6e86406e863e6e864c6e86426e864e6e864a6e86466e86546e86446e865c6e865a6e86686e86626e86666e86486e86726e866c6e86506e869e6e86766e86a06e86526e86586e86566e866e6e86606e866a6e86706e86746e865e6e86646e86a26e86786e868c6e868e6e86826e86a46e86866e868a6e86986e869a6e86906e867e6e86946e86886e86846e867c6e86926e86806e86966e86aa6e86a86e86a66e86ae6e86b06e86ac6e86b46e86b26e86b66e86b86e86ba6e86")\
            X(84  , 0, F24                , "F24"               , ""    , 0     , 57387, 'u', -1    , -1    , "006f87286f87086f87266f87126f870c6f871e6f871c6f87066f87246f87046f870a6f87106f87166f87206f87186f870e6f872a6f871a6f87226f87146f877a6f872c6f872e6f87026f87306f87326f87346f873a6f873c6f87386f87366f879c6f87406f873e6f874c6f87426f874e6f874a6f87466f87546f87446f875c6f875a6f87686f87626f87666f87486f87726f876c6f87506f879e6f87766f87a06f87526f87586f87566f876e6f87606f876a6f87706f87746f875e6f87646f87a26f87786f878c6f878e6f87826f87a46f87866f878a6f87986f879a6f87906f877e6f87946f87886f87846f877c6f87926f87806f87966f87aa6f87a86f87a66f87ae6f87b06f87ac6f87b46f87b26f87b66f87b86f87ba6f87")\
            X(86  , 0, F25                , "F25"               , ""    , 0     , 57388, 'u', -1    , -1    , "")\
            X(88  , 0, F26                , "F26"               , ""    , 0     , 57389, 'u', -1    , -1    , "")\
            X(90  , 0, F27                , "F27"               , ""    , 0     , 57390, 'u', -1    , -1    , "")\
            X(92  , 0, F28                , "F28"               , ""    , 0     , 57391, 'u', -1    , -1    , "")\
            X(94  , 0, F29                , "F29"               , ""    , 0     , 57392, 'u', -1    , -1    , "")\
            X(96  , 0, F30                , "F30"               , ""    , 0     , 57393, 'u', -1    , -1    , "")\
            X(98  , 0, F31                , "F31"               , ""    , 0     , 57394, 'u', -1    , -1    , "")\
            X(100 , 0, F32                , "F32"               , ""    , 0     , 57395, 'u', -1    , -1    , "")\
            X(102 , 0, F33                , "F33"               , ""    , 0     , 57396, 'u', -1    , -1    , "")\
            X(104 , 0, F34                , "F34"               , ""    , 0     , 57397, 'u', -1    , -1    , "")\
            X(106 , 0, F35                , "F35"               , ""    , 0     , 57398, 'u', -1    , -1    , "")\
            X(108 , 0, PrintScreen        , "PrintScreen"       , ""    , 0     , 57361, 'u', -1    , -1    , "01372c29372c09372c27372c13372c0d372c1f372c1d372c07372c25372c05372c0b372c11372c17372c21372c19372c0f372c2b372c1b372c23372c15372c7b372c2d372c2f372c03372c31372c33372c35372c3b372c3d372c39372c37372c9d372c41372c3f372c4d372c43372c4f372c4b372c47372c55372c45372c5d372c5b372c69372c63372c67372c49372c73372c6d372c51372c9f372c77372ca1372c53372c59372c57372c6f372c61372c6b372c71372c75372c5f372c65372ca3372c79372c8d372c8f372c83372ca5372c87372c8b372c99372c9b372c91372c7f372c95372c89372c85372c7d372c93372c81372c97372cab372ca9372ca7372caf372cb1372cad372cb5372cb3372cb7372cb9372cbb372c")\
            X(110 , 0, Pause              , "Pause"             , ""    , 0     , 57362, 'u', '\x1a', '\x03', "0045902845900845902645901245900c45901e45901c45900645902445900445900a45901045901645902045901845900e45902a45901a45902245901445907a45902c45902e45900245903045903245903445903a45903c45903845903645909c45904045903e45904c45904245904e45904a45904645905445904445905c45905a45906845906245906645904845907245906c45905045909e4590764590a045905245905845905645906e45906045906a45907045907445905e4590644590a245907845908c45908e4590824590a445908645908a45909845909a45909045907e45909445908845908445907c4590924590804590964590aa4590a84590a64590ae4590b04590ac4590b44590b24590b64590b84590ba4590")\
            X(112 , 1, Break              , "Break"             , "\x03", 0x03  , 3    , 'u', '\x03', '\x03', "0146032946030946032746031346030d46031f46031d46030746032546030546030b46031146031746032146031946030f46032b46031b46032346031546037b46032d46032f46030346033146033346033546033b46033d46033946033746039d46034146033f46034d46034346034f46034b46034746035546034546035d46035b46036946036346036746034946037346036d46035146039f4603774603a146035346035946035746036f46036146036b46037146037546035f4603654603a346037946038d46038f4603834603a546038746038b46039946039b46039146037f46039546038946038546037d4603934603814603974603ab4603a94603a74603af4603b14603ad4603b54603b34603b74603b94603bb4603")\
            X(114 , 1, SysReq             , "SysReq"            , ""    , 0     , 0    , 'u', -1    , '\x03', "00542c28542c08542c26542c12542c0c542c1e542c1c542c06542c24542c04542c0a542c10542c16542c20542c18542c0e542c2a542c1a542c22542c7a542c2c542c2e542c02542c30542c32542c34542c3a542c40542c3e542c4c542c4e542c4a542c54542c5c542c5a542c68542c62542c66542c48542c72542c6c542c9e542c76542ca0542ca2542c8c542c8e542c82542ca4542c86542c8a542c98542c9a542c7c542c92542c80542c96542caa542ca8542cb4542cb6542cb8542cba542c")\
            X(116 , 1, Esc                , "Esc"               , "\x1B", 0x1b  , 27   , 'u', '\x1b', '\x1b', "00011b28011b08011b26011b12011b0c011b1e011b1c011b06011b24011b04011b0a011b10011b16011b20011b18011b0e011b2a011b1a011b22011b14011b7a011b2c011b2e011b02011b30011b32011b34011b3a011b3c011b38011b36011b9c011b40011b3e011b4c011b42011b4e011b4a011b46011b54011b44011b5c011b5a011b68011b62011b66011b48011b72011b6c011b50011b9e011b76011ba0011b52011b58011b56011b6e011b60011b6a011b70011b74011b5e011b64011ba2011b78011b8c011b8e011b82011ba4011b86011b8a011b98011b9a011b90011b7e011b94011b88011b84011b7c011b92011b80011b96011baa011ba8011ba6011bae011bb0011bac011bb4011bb2011bb6011bb8011bba011b")\
            X(118 , 1, Tab                , "Tab"               , "\x09", 0x09  , 9    , 'u', '\x09', '\x09', "000f09280f09080f09260f09120f090c0f091e0f091c0f09060f09240f09040f090a0f09100f09160f09200f09180f090e0f092a0f091a0f09220f09140f097a0f092c0f092e0f09020f09300f09320f09340f093a0f093c0f09380f09360f099c0f09400f093e0f094c0f09420f094e0f094a0f09460f09540f09440f095c0f095a0f09680f09620f09660f09480f09720f096c0f09500f099e0f09760f09a00f09520f09580f09560f096e0f09600f096a0f09700f09740f095e0f09640f09a20f09780f098c0f098e0f09820f09a40f09860f098a0f09980f099a0f09900f097e0f09940f09880f09840f097c0f09920f09800f09960f09aa0f09a80f09a60f09ae0f09b00f09ac0f09b40f09b20f09b60f09b80f09ba0f09")\
            X(120 , 1, Backspace          , "Backspace"         , "\x08", 0x08  , 127  , 'u', '\x7f', '\x08', "000e08280e08080e08260e08120e080c0e081e0e081c0e08060e08240e08040e080a0e08100e08160e08200e08180e080e0e082a0e081a0e08220e08140e087a0e082c0e082e0e08020e08300e08320e08340e083a0e083c0e08380e08360e089c0e08400e083e0e084c0e08420e084e0e084a0e08460e08540e08440e085c0e085a0e08680e08620e08660e08480e08720e086c0e08500e089e0e08760e08a00e08520e08580e08560e086e0e08600e086a0e08700e08740e085e0e08640e08a20e08780e088c0e088e0e08820e08a40e08860e088a0e08980e089a0e08900e087e0e08940e08880e08840e087c0e08920e08800e08960e08aa0e08a80e08a60e08ae0e08b00e08ac0e08b40e08b20e08b60e08b80e08ba0e08")\
            X(122 , 1, Space              , "Space"             , "\x20", 0x20  , 32   , 'u', '\x20', '\0'  , "0039202839200839202639201239200c39201e39201c39200639202439200439200a39201039201639202039201839200e39202a39201a39202239201439207a39202c39202e39200239203039203239203439203a39203c39203839203639209c39204039203e39204c39204239204e39204a39204639205439204439205c39205a39206839206239206639204839207239206c39205039209e3920763920a039205239205839205639206e39206039206a39207039207439205e3920643920a239207839208c39208e3920823920a439208639208a39209839209a39209039207e39209439208839208439207c3920923920803920963920aa3920a83920a63920ae3920b03920ac3920b43920b23920b63920b83920ba3920")\
            X(124 , 1, KeyEnter           , "Enter"             , "\x0D", 0x0d  , 13   , 'u', '\x0d', '\x0a', "001c0d281c0d081c0d261c0d121c0d0c1c0d1e1c0d1c1c0d061c0d241c0d041c0d0a1c0d101c0d161c0d201c0d181c0d0e1c0d2a1c0d1a1c0d221c0d141c0d7a1c0d2c1c0d2e1c0d021c0d301c0d321c0d341c0d3a1c0d3c1c0d381c0d361c0d9c1c0d401c0d3e1c0d4c1c0d421c0d4e1c0d4a1c0d461c0d541c0d441c0d5c1c0d5a1c0d681c0d621c0d661c0d481c0d721c0d6c1c0d501c0d9e1c0d761c0da01c0d521c0d581c0d561c0d6e1c0d601c0d6a1c0d701c0d741c0d5e1c0d641c0da21c0d781c0d8c1c0d8e1c0d821c0da41c0d861c0d8a1c0d981c0d9a1c0d901c0d7e1c0d941c0d881c0d841c0d7c1c0d921c0d801c0d961c0daa1c0da81c0da61c0dae1c0db01c0dac1c0db41c0db21c0db61c0db81c0dba1c0d")\
            X( 125, 1, NumpadEnter        , "Enter"             , "\x0D", 0x0d  , 57414, 'u', '\x0d', '\x0a', "011c0d291c0d091c0d271c0d131c0d0d1c0d1f1c0d1d1c0d071c0d251c0d051c0d0b1c0d111c0d171c0d211c0d191c0d0f1c0d2b1c0d1b1c0d231c0d151c0d7b1c0d2d1c0d2f1c0d031c0d311c0d331c0d351c0d3b1c0d3d1c0d391c0d371c0d9d1c0d411c0d3f1c0d4d1c0d431c0d4f1c0d4b1c0d471c0d551c0d451c0d5d1c0d5b1c0d691c0d631c0d671c0d491c0d731c0d6d1c0d511c0d9f1c0d771c0da11c0d531c0d591c0d571c0d6f1c0d611c0d6b1c0d711c0d751c0d5f1c0d651c0da31c0d791c0d8d1c0d8f1c0d831c0da51c0d871c0d8b1c0d991c0d9b1c0d911c0d7f1c0d951c0d891c0d851c0d7d1c0d931c0d811c0d971c0dab1c0da91c0da71c0daf1c0db11c0dad1c0db51c0db31c0db71c0db91c0dbb1c0d")\
            X(126 , 1, KeyInsert          , "Insert"            , ""    , 0     , 2    , '~', -1    , -1    , "01522d29522d09522d27522d13522d0d522d1f522d1d522d07522d25522d05522d0b522d11522d17522d21522d19522d0f522d2b522d1b522d23522d15522d7b522d2d522d2f522d03522d31522d33522d35522d3b522d3d522d39522d37522d9d522d41522d3f522d4d522d43522d4f522d4b522d47522d55522d45522d5d522d5b522d69522d63522d67522d49522d73522d6d522d51522d9f522d77522da1522d53522d59522d57522d6f522d61522d6b522d71522d75522d5f522d65522da3522d79522d8d522d8f522d83522da5522d87522d8b522d99522d9b522d91522d7f522d95522d89522d85522d7d522d93522d81522d97522dab522da9522da7522daf522db1522dad522db5522db3522db7522db9522dbb522d")\
            X( 127, 1, NumpadInsert       , "Insert"            , ""    , 0     , 57425, 'u', -1    , -1    , "00822d28822d08822d26822d12822d0c822d1e822d1c822d06822d24822d04822d0a822d10822d16822d20822d18822d0e822d2a822d1a822d22822d14822d7a822d2c822d2e822d02822d30822d32822d34822d3a822d3c822d38822d36822d9c822d40822d3e822d4c822d42822d4e822d4a822d46822d54822d44822d5c822d5a822d68822d62822d66822d48822d72822d6c822d50822d9e822d76822da0822d52822d58822d56822d6e822d60822d6a822d70822d74822d5e822d64822da2822d78822d8c822d8e822d82822da4822d86822d8a822d98822d9a822d90822d7e822d94822d88822d84822d7c822d92822d80822d96822daa822da8822da6822dae822db0822dac822db4822db2822db6822db8822dba822d")\
            X(128 , 1, KeyDelete          , "Delete"            , ""    , 0     , 3    , '~', -1    , -1    , "01532e29532e09532e27532e13532e0d532e1f532e1d532e07532e25532e05532e0b532e11532e17532e21532e19532e0f532e2b532e1b532e23532e15532e7b532e2d532e2f532e03532e31532e33532e35532e3b532e3d532e39532e37532e9d532e41532e3f532e4d532e43532e4f532e4b532e47532e55532e45532e5d532e5b532e69532e63532e67532e49532e73532e6d532e51532e9f532e77532ea1532e53532e59532e57532e6f532e61532e6b532e71532e75532e5f532e65532ea3532e79532e8d532e8f532e83532ea5532e87532e8b532e99532e9b532e91532e7f532e95532e89532e85532e7d532e93532e81532e97532eab532ea9532ea7532eaf532eb1532ead532eb5532eb3532eb7532eb9532ebb532e")\
            X( 129, 1, NumpadDelete       , "Delete"            , ""    , 0     , 57426, 'u', -1    , -1    , "00832e28832e08832e26832e12832e0c832e1e832e1c832e06832e24832e04832e0a832e10832e16832e20832e18832e0e832e2a832e1a832e22832e14832e7a832e2c832e2e832e02832e30832e32832e34832e3a832e3c832e38832e36832e9c832e40832e3e832e4c832e42832e4e832e4a832e46832e54832e44832e5c832e5a832e68832e62832e66832e48832e72832e6c832e50832e9e832e76832ea0832e52832e58832e56832e6e832e60832e6a832e70832e74832e5e832e64832ea2832e78832e8c832e8e832e82832ea4832e86832e8a832e98832e9a832e90832e7e832e94832e88832e84832e7c832e92832e80832e96832eaa832ea8832ea6832eae832eb0832eac832eb4832eb2832eb6832eb8832eba832e")\
            X(130 , 1, KeyClear           , "Clear"             , ""    , 0     , 1    , 'E', -1    , -1    , "")\
            X( 131, 1, NumpadClear        , "Clear"             , ""    , 0     , 57427, '~', -1    , -1    , "008c0c288c0c088c0c268c0c128c0c0c8c0c1e8c0c1c8c0c068c0c248c0c048c0c0a8c0c108c0c168c0c208c0c188c0c0e8c0c2a8c0c1a8c0c228c0c148c0c7a8c0c2c8c0c2e8c0c028c0c308c0c328c0c348c0c3a8c0c3c8c0c388c0c368c0c9c8c0c408c0c3e8c0c4c8c0c428c0c4e8c0c4a8c0c468c0c548c0c448c0c5c8c0c5a8c0c688c0c628c0c668c0c488c0c728c0c6c8c0c508c0c9e8c0c768c0ca08c0c528c0c588c0c568c0c6e8c0c608c0c6a8c0c708c0c748c0c5e8c0c648c0ca28c0c788c0c8c8c0c8e8c0c828c0ca48c0c868c0c8a8c0c988c0c9a8c0c908c0c7e8c0c948c0c888c0c848c0c7c8c0c928c0c808c0c968c0caa8c0ca88c0ca68c0cae8c0cb08c0cac8c0cb48c0cb28c0cb68c0cb88c0cba8c0c")\
            X(132 , 1, KeyPageUp          , "PageUp"            , ""    , 0     , 5    , '~', -1    , -1    , "0149212949210949212749211349210d49211f49211d49210749212549210549210b49211149211749212149211949210f49212b49211b49212349211549217b49212d49212f49210349213149213349213549213b49213d49213949213749219d49214149213f49214d49214349214f49214b49214749215549214549215d49215b49216949216349216749214949217349216d49215149219f4921774921a149215349215949215749216f49216149216b49217149217549215f4921654921a349217949218d49218f4921834921a549218749218b49219949219b49219149217f49219549218949218549217d4921934921814921974921ab4921a94921a74921af4921b14921ad4921b54921b34921b74921b94921bb4921")\
            X( 133, 1, NumpadPageUp       , "PageUp"            , ""    , 0     , 57421, 'u', -1    , -1    , "0089212889210889212689211289210c89211e89211c89210689212489210489210a89211089211689212089211889210e89212a89211a89212289211489217a89212c89212e89210289213089213289213489213a89213c89213889213689219c89214089213e89214c89214289214e89214a89214689215489214489215c89215a89216889216289216689214889217289216c89215089219e8921768921a089215289215889215689216e89216089216a89217089217489215e8921648921a289217889218c89218e8921828921a489218689218a89219889219a89219089217e89219489218889218489217c8921928921808921968921aa8921a88921a68921ae8921b08921ac8921b48921b28921b68921b88921ba8921")\
            X(134 , 1, KeyPageDown        , "PageDown"          , ""    , 0     , 6    , '~', -1    , -1    , "0151222951220951222751221351220d51221f51221d51220751222551220551220b51221151221751222151221951220f51222b51221b51222351221551227b51222d51222f51220351223151223351223551223b51223d51223951223751229d51224151223f51224d51224351224f51224b51224751225551224551225d51225b51226951226351226751224951227351226d51225151229f5122775122a151225351225951225751226f51226151226b51227151227551225f5122655122a351227951228d51228f5122835122a551228751228b51229951229b51229151227f51229551228951228551227d5122935122815122975122ab5122a95122a75122af5122b15122ad5122b55122b35122b75122b95122bb5122")\
            X( 135, 1, NumpadPageDown     , "PageDown"          , ""    , 0     , 57422, 'u', -1    , -1    , "0081222881220881222681221281220c81221e81221c81220681222481220481220a81221081221681222081221881220e81222a81221a81222281221481227a81222c81222e81220281223081223281223481223a81223c81223881223681229c81224081223e81224c81224281224e81224a81224681225481224481225c81225a81226881226281226681224881227281226c81225081229e8122768122a081225281225881225681226e81226081226a81227081227481225e8122648122a281227881228c81228e8122828122a481228681228a81229881229a81229081227e81229481228881228481227c8122928122808122968122aa8122a88122a68122ae8122b08122ac8122b48122b28122b68122b88122ba8122")\
            X(136 , 1, KeyHome            , "Home"              , ""    , 0     , 7    , '~', -1    , -1    , "0147242947240947242747241347240d47241f47241d47240747242547240547240b47241147241747242147241947240f47242b47241b47242347241547247b47242d47242f47240347243147243347243547243b47243d47243947243747249d47244147243f47244d47244347244f47244b47244747245547244547245d47245b47246947246347246747244947247347246d47245147249f4724774724a147245347245947245747246f47246147246b47247147247547245f4724654724a347247947248d47248f4724834724a547248747248b47249947249b47249147247f47249547248947248547247d4724934724814724974724ab4724a94724a74724af4724b14724ad4724b54724b34724b74724b94724bb4724")\
            X( 137, 1, NumpadHome         , "Home"              , ""    , 0     , 57423, 'u', -1    , -1    , "0087242887240887242687241287240c87241e87241c87240687242487240487240a87241087241687242087241887240e87242a87241a87242287241487247a87242c87242e87240287243087243287243487243a87243c87243887243687249c87244087243e87244c87244287244e87244a87244687245487244487245c87245a87246887246287246687244887247287246c87245087249e8724768724a087245287245887245687246e87246087246a87247087247487245e8724648724a287247887248c87248e8724828724a487248687248a87249887249a87249087247e87249487248887248487247c8724928724808724968724aa8724a88724a68724ae8724b08724ac8724b48724b28724b68724b88724ba8724")\
            X(138 , 1, KeyEnd             , "End"               , ""    , 0     , 8    , '~', -1    , -1    , "014f23294f23094f23274f23134f230d4f231f4f231d4f23074f23254f23054f230b4f23114f23174f23214f23194f230f4f232b4f231b4f23234f23154f237b4f232d4f232f4f23034f23314f23334f23354f233b4f233d4f23394f23374f239d4f23414f233f4f234d4f23434f234f4f234b4f23474f23554f23454f235d4f235b4f23694f23634f23674f23494f23734f236d4f23514f239f4f23774f23a14f23534f23594f23574f236f4f23614f236b4f23714f23754f235f4f23654f23a34f23794f238d4f238f4f23834f23a54f23874f238b4f23994f239b4f23914f237f4f23954f23894f23854f237d4f23934f23814f23974f23ab4f23a94f23a74f23af4f23b14f23ad4f23b54f23b34f23b74f23b94f23bb4f23")\
            X( 139, 1, NumpadEnd          , "End"               , ""    , 0     , 57424, 'u', -1    , -1    , "008f23288f23088f23268f23128f230c8f231e8f231c8f23068f23248f23048f230a8f23108f23168f23208f23188f230e8f232a8f231a8f23228f23148f237a8f232c8f232e8f23028f23308f23328f23348f233a8f233c8f23388f23368f239c8f23408f233e8f234c8f23428f234e8f234a8f23468f23548f23448f235c8f235a8f23688f23628f23668f23488f23728f236c8f23508f239e8f23768f23a08f23528f23588f23568f236e8f23608f236a8f23708f23748f235e8f23648f23a28f23788f238c8f238e8f23828f23a48f23868f238a8f23988f239a8f23908f237e8f23948f23888f23848f237c8f23928f23808f23968f23aa8f23a88f23a68f23ae8f23b08f23ac8f23b48f23b28f23b68f23b88f23ba8f23")\
            X(140 , 1, KeyLeftArrow       , "LeftArrow"         , ""    , 0     , 1    , 'D', -1    , -1    , "014b25294b25094b25274b25134b250d4b251f4b251d4b25074b25254b25054b250b4b25114b25174b25214b25194b250f4b252b4b251b4b25234b25154b257b4b252d4b252f4b25034b25314b25334b25354b253b4b253d4b25394b25374b259d4b25414b253f4b254d4b25434b254f4b254b4b25474b25554b25454b255d4b255b4b25694b25634b25674b25494b25734b256d4b25514b259f4b25774b25a14b25534b25594b25574b256f4b25614b256b4b25714b25754b255f4b25654b25a34b25794b258d4b258f4b25834b25a54b25874b258b4b25994b259b4b25914b257f4b25954b25894b25854b257d4b25934b25814b25974b25ab4b25a94b25a74b25af4b25b14b25ad4b25b54b25b34b25b74b25b94b25bb4b25")\
            X( 141, 1, NumpadLeftArrow    , "LeftArrow"         , ""    , 0     , 57417, 'u', -1    , -1    , "008b25288b25088b25268b25128b250c8b251e8b251c8b25068b25248b25048b250a8b25108b25168b25208b25188b250e8b252a8b251a8b25228b25148b257a8b252c8b252e8b25028b25308b25328b25348b253a8b253c8b25388b25368b259c8b25408b253e8b254c8b25428b254e8b254a8b25468b25548b25448b255c8b255a8b25688b25628b25668b25488b25728b256c8b25508b259e8b25768b25a08b25528b25588b25568b256e8b25608b256a8b25708b25748b255e8b25648b25a28b25788b258c8b258e8b25828b25a48b25868b258a8b25988b259a8b25908b257e8b25948b25888b25848b257c8b25928b25808b25968b25aa8b25a88b25a68b25ae8b25b08b25ac8b25b48b25b28b25b68b25b88b25ba8b25")\
            X(142 , 1, KeyRightArrow      , "RightArrow"        , ""    , 0     , 1    , 'C', -1    , -1    , "014d27294d27094d27274d27134d270d4d271f4d271d4d27074d27254d27054d270b4d27114d27174d27214d27194d270f4d272b4d271b4d27234d27154d277b4d272d4d272f4d27034d27314d27334d27354d273b4d273d4d27394d27374d279d4d27414d273f4d274d4d27434d274f4d274b4d27474d27554d27454d275d4d275b4d27694d27634d27674d27494d27734d276d4d27514d279f4d27774d27a14d27534d27594d27574d276f4d27614d276b4d27714d27754d275f4d27654d27a34d27794d278d4d278f4d27834d27a54d27874d278b4d27994d279b4d27914d277f4d27954d27894d27854d277d4d27934d27814d27974d27ab4d27a94d27a74d27af4d27b14d27ad4d27b54d27b34d27b74d27b94d27bb4d27")\
            X( 143, 1, NumpadRightArrow   , "RightArrow"        , ""    , 0     , 57418, 'u', -1    , -1    , "008d27288d27088d27268d27128d270c8d271e8d271c8d27068d27248d27048d270a8d27108d27168d27208d27188d270e8d272a8d271a8d27228d27148d277a8d272c8d272e8d27028d27308d27328d27348d273a8d273c8d27388d27368d279c8d27408d273e8d274c8d27428d274e8d274a8d27468d27548d27448d275c8d275a8d27688d27628d27668d27488d27728d276c8d27508d279e8d27768d27a08d27528d27588d27568d276e8d27608d276a8d27708d27748d275e8d27648d27a28d27788d278c8d278e8d27828d27a48d27868d278a8d27988d279a8d27908d277e8d27948d27888d27848d277c8d27928d27808d27968d27aa8d27a88d27a68d27ae8d27b08d27ac8d27b48d27b28d27b68d27b88d27ba8d27")\
            X(144 , 1, KeyUpArrow         , "UpArrow"           , ""    , 0     , 1    , 'A', -1    , -1    , "0148262948260948262748261348260d48261f48261d48260748262548260548260b48261148261748262148261948260f48262b48261b48262348261548267b48262d48262f48260348263148263348263548263b48263d48263948263748269d48264148263f48264d48264348264f48264b48264748265548264548265d48265b48266948266348266748264948267348266d48265148269f4826774826a148265348265948265748266f48266148266b48267148267548265f4826654826a348267948268d48268f4826834826a548268748268b48269948269b48269148267f48269548268948268548267d4826934826814826974826ab4826a94826a74826af4826b14826ad4826b54826b34826b74826b94826bb4826")\
            X( 145, 1, NumpadUpArrow      , "UpArrow"           , ""    , 0     , 57419, 'u', -1    , -1    , "0088262888260888262688261288260c88261e88261c88260688262488260488260a88261088261688262088261888260e88262a88261a88262288261488267a88262c88262e88260288263088263288263488263a88263c88263888263688269c88264088263e88264c88264288264e88264a88264688265488264488265c88265a88266888266288266688264888267288266c88265088269e8826768826a088265288265888265688266e88266088266a88267088267488265e8826648826a288267888268c88268e8826828826a488268688268a88269888269a88269088267e88269488268888268488267c8826928826808826968826aa8826a88826a68826ae8826b08826ac8826b48826b28826b68826b88826ba8826")\
            X(146 , 1, KeyDownArrow       , "DownArrow"         , ""    , 0     , 1    , 'B', -1    , -1    , "0150282950280950282750281350280d50281f50281d50280750282550280550280b50281150281750282150281950280f50282b50281b50282350281550287b50282d50282f50280350283150283350283550283b50283d50283950283750289d50284150283f50284d50284350284f50284b50284750285550284550285d50285b50286950286350286750284950287350286d50285150289f5028775028a150285350285950285750286f50286150286b50287150287550285f5028655028a350287950288d50288f5028835028a550288750288b50289950289b50289150287f50289550288950288550287d5028935028815028975028ab5028a95028a75028af5028b15028ad5028b55028b35028b75028b95028bb5028")\
            X( 147, 1, NumpadDownArrow    , "DownArrow"         , ""    , 0     , 57420, 'u', -1    , -1    , "0080282880280880282680281280280c80281e80281c80280680282480280480280a80281080281680282080281880280e80282a80281a80282280281480287a80282c80282e80280280283080283280283480283a80283c80283880283680289c80284080283e80284c80284280284e80284a80284680285480284480285c80285a80286880286280286680284880287280286c80285080289e8028768028a080285280285880285680286e80286080286a80287080287480285e8028648028a280287880288c80288e8028828028a480288680288a80289880289a80289080287e80289480288880288480287c8028928028808028968028aa8028a88028a68028ae8028b08028ac8028b48028b28028b68028b88028ba8028")\
            X(148 , 1, Key0               , "0"                 , "0"   , 0x30  , 48   , 'u', '0'   , '0'   , "000b30280b30080b30260b30120b300c0b301e0b301c0b30060b30240b30040b300a0b30100b30160b30200b30180b300e0b302a0b301a0b30220b30140b307a0b302c0b302e0b30020b30300b30320b30340b303a0b303c0b30380b30360b309c0b30400b303e0b304c0b30420b304e0b304a0b30460b30540b30440b305c0b305a0b30680b30620b30660b30480b30720b306c0b30500b309e0b30760b30a00b30520b30580b30560b306e0b30600b306a0b30700b30740b305e0b30640b30a20b30780b308c0b308e0b30820b30a40b30860b308a0b30982930980bc09a0b30900b307e0b30940b30880b30840b307c0bc0920b30800b30960b30aa0b30a80b30a60b30ae0b30b00b30ac0b30b40b30b20b30b60b30b82d30ba3430")\
            X( 149, 1, Numpad0            , "0"                 , "0"   , 0x30  , 57399, 'u', '0'   , '0'   , "0052602852600852602652601252600c52601e52601c52600652602452600452600a52601052601652602052601852600e52602a52601a52602252601452607a52602c52602e52600252603052603252603452603a52603c52603852603652609c52604052603e52604c52604252604e52604a52604652605452604452605c52605a52606852606252606652604852607252606c52605052609e5260765260a052605252605852605652606e52606052606a52607052607452605e5260645260a252607852608c52608e5260825260a452608652608a52609852609a52609052607e52609452608852608452607c5260925260805260965260aa5260a85260a65260ae5260b05260ac5260b45260b25260b65260b85260ba5260")\
            X(150 , 1, Key1               , "1"                 , "1"   , 0x31  , 49   , 'u', '1'   , '1'   , "0002312802310802312602311202310c02311e02311c02310602312402310402310a02311002311602312002311802310e02312a02311a02312202311402317a02312c02312e02310202313002313202313402313a02313c02313802313602319c02314002313e02314c02314202314e02314a02314602315402314402315c02315a02316802316202316602314802317202316c02315002319e0231760231a002315202315802315602316e02316002316a02317002317402315e0231640231a202317802318c02318e0231820231a402318602318a02319802319a02319002317e02319402318802318402317c0231920231800231960231aa0231a80231a60231ae0231b00231ac0231b40231b20231b60231b80231ba0d31")\
            X( 151, 1, Numpad1            , "1"                 , "1"   , 0x31  , 57400, 'u', '1'   , '1'   , "004f61284f61084f61264f61124f610c4f611e4f611c4f61064f61244f61044f610a4f61104f61164f61204f61184f610e4f612a4f611a4f61224f61144f617a4f612c4f612e4f61024f61304f61324f61344f613a4f613c4f61384f61364f619c4f61404f613e4f614c4f61424f614e4f614a4f61464f61544f61444f615c4f615a4f61684f61624f61664f61484f61724f616c4f61504f619e4f61764f61a04f61524f61584f61564f616e4f61604f616a4f61704f61744f615e4f61644f61a24f61784f618c4f618e4f61824f61a44f61864f618a4f61984f619a4f61904f617e4f61944f61884f61844f617c4f61924f61804f61964f61aa4f61a84f61a64f61ae4f61b04f61ac4f61b44f61b24f61b64f61b84f61ba4f61")\
            X(152 , 1, Key2               , "2"                 , "2"   , 0x32  , 50   , 'u', '2'   , '\0'  , "0003322803320803322603321203320c03321e03321c03320603322403320403320a03321003321603322003321803320e03322a03321a03322203321403327a03322c03322e03320203323003323203323403323a03323c03323803323603329c03324003323e03324c03324203324e03324a03324603325403324403325c03325a03326803326203326603324803327203326c03325003329e0332760332a003325203325803325603326e03326003326a03327003327403325e0332640332a203327803328c03328e0332820332a403328603328a03329803329a03329003327e03329403328803328403327c0332920332800332960332aa0332a80332a60332ae0332b00332ac0332b40332b20332b60332b80332ba0c32")\
            X( 153, 1, Numpad2            , "2"                 , "2"   , 0x32  , 57401, 'u', '2'   , '\0'  , "0050622850620850622650621250620c50621e50621c50620650622450620450620a50621050621650622050621850620e50622a50621a50622250621450627a50622c50622e50620250623050623250623450623a50623c50623850623650629c50624050623e50624c50624250624e50624a50624650625450624450625c50625a50626850626250626650624850627250626c50625050629e5062765062a050625250625850625650626e50626050626a50627050627450625e5062645062a250627850628c50628e5062825062a450628650628a50629850629a50629050627e50629450628850628450627c5062925062805062965062aa5062a85062a65062ae5062b05062ac5062b45062b25062b65062b85062ba5062")\
            X(154 , 1, Key3               , "3"                 , "3"   , 0x33  , 51   , 'u', '3'   , '\x1b', "0004332804330804332604331204330c04331e04331c04330604332404330404330a04331004331604332004331804330e04332a04331a04332204331404337a04332c04332e04330204333004333204333404333a04333c04333804333604339c04334004333e04334c04334204334e04334a04334604335404334404335c04335a04336804336204336604334804337204336c04335004339e0433760433a004335204335804335604336e04336004336a04337004337404335e0433640433a204337804338c04338e0433820433a404338604338a04339804339a04339004337e04339404338804338404337c0433920433800433960433aa0433a80433a60433ae0433b00433ac0433b40433b20433b60433b80433ba0b33")\
            X( 155, 1, Numpad3            , "3"                 , "3"   , 0x33  , 57402, 'u', '3'   , '\x1b', "0051632851630851632651631251630c51631e51631c51630651632451630451630a51631051631651632051631851630e51632a51631a51632251631451637a51632c51632e51630251633051633251633451633a51633c51633851633651639c51634051633e51634c51634251634e51634a51634651635451634451635c51635a51636851636251636651634851637251636c51635051639e5163765163a051635251635851635651636e51636051636a51637051637451635e5163645163a251637851638c51638e5163825163a451638651638a51639851639a51639051637e51639451638851638451637c5163925163805163965163aa5163a85163a65163ae5163b05163ac5163b45163b25163b65163b85163ba5163")\
            X(156 , 1, Key4               , "4"                 , "4"   , 0x34  , 52   , 'u', '4'   , '\x1c', "0005342805340805342605341205340c05341e05341c05340605342405340405340a05341005341605342005341805340e05342a05341a05342205341405347a05342c05342e05340205343005343205343405343a05343c05343805343605349c05344005343e05344c05344205344e05344a05344605345405344405345c05345a05346805346205346605344805347205346c05345005349e0534760534a005345205345805345605346e05346005346a05347005347405345e0534640534a205347805348c05348e0534820534a405348605348a05349805349a05349005347e05349405348805348405347c0534920534800534960534aa0534a80534a60534ae0534b00534ac0534b40534b20534b60534b80534ba0a34")\
            X( 157, 1, Numpad4            , "4"                 , "4"   , 0x34  , 57403, 'u', '4'   , '\x1c', "004b64284b64084b64264b64124b640c4b641e4b641c4b64064b64244b64044b640a4b64104b64164b64204b64184b640e4b642a4b641a4b64224b64144b647a4b642c4b642e4b64024b64304b64324b64344b643a4b643c4b64384b64364b649c4b64404b643e4b644c4b64424b644e4b644a4b64464b64544b64444b645c4b645a4b64684b64624b64664b64484b64724b646c4b64504b649e4b64764b64a04b64524b64584b64564b646e4b64604b646a4b64704b64744b645e4b64644b64a24b64784b648c4b648e4b64824b64a44b64864b648a4b64984b649a4b64904b647e4b64944b64884b64844b647c4b64924b64804b64964b64aa4b64a84b64a64b64ae4b64b04b64ac4b64b44b64b24b64b64b64b84b64ba4b64")\
            X(158 , 1, Key5               , "5"                 , "5"   , 0x35  , 53   , 'u', '5'   , '\x1d', "0006352806350806352606351206350c06351e06351c06350606352406350406350a06351006351606352006351806350e06352a06351a06352206351406357a06352c06352e06350206353006353206353406353a06353c06353806353606359c06354006353e06354c06354206354e06354a06354606355406354406355c06355a06356806356206356606354806357206356c06355006359e0635760635a006355206355806355606356e06356006356a06357006357406355e0635640635a206357806358c06358e0635820635a406358606358a06359806359a06359006357e06359406358806358406357c0635920635800635960635aa0635a80635a60635ae0635b00635ac0635b40635b20635b60635b81035ba1a35")\
            X( 159, 1, Numpad5            , "5"                 , "5"   , 0x35  , 57404, 'u', '5'   , '\x1d', "004c65284c65084c65264c65124c650c4c651e4c651c4c65064c65244c65044c650a4c65104c65164c65204c65184c650e4c652a4c651a4c65224c65144c657a4c652c4c652e4c65024c65304c65324c65344c653a4c653c4c65384c65364c659c4c65404c653e4c654c4c65424c654e4c654a4c65464c65544c65444c655c4c655a4c65684c65624c65664c65484c65724c656c4c65504c659e4c65764c65a04c65524c65584c65564c656e4c65604c656a4c65704c65744c655e4c65644c65a24c65784c658c4c658e4c65824c65a44c65864c658a4c65984c659a4c65904c657e4c65944c65884c65844c657c4c65924c65804c65964c65aa4c65a84c65a64c65ae4c65b04c65ac4c65b44c65b24c65b64c65b84c65ba4c65")\
            X(160 , 1, Key6               , "6"                 , "6"   , 0x36  , 54   , 'u', '6'   , '\x1e', "0007362807360807362607361207360c07361e07361c07360607362407360407360a07361007361607362007361807360e07362a07361a07362207361407367a07362c07362e07360207363007363207363407363a07363c07363807363607369c07364007363e07364c07364207364e07364a07364607365407364407365c07365a07366807366207366607364807367207366c07365007369e0736760736a007365207365807365607366e07366007366a07367007367407365e0736640736a207367807368c07368e0736820736a407368607368a07369807369a07369007367e07369407368807368407367c0736920736800736960736aa0736a80736a60736ae0736b00736ac0736b40736b20736b60736b81136ba1936")\
            X( 161, 1, Numpad6            , "6"                 , "6"   , 0x36  , 57405, 'u', '6'   , '\x1e', "004d66284d66084d66264d66124d660c4d661e4d661c4d66064d66244d66044d660a4d66104d66164d66204d66184d660e4d662a4d661a4d66224d66144d667a4d662c4d662e4d66024d66304d66324d66344d663a4d663c4d66384d66364d669c4d66404d663e4d664c4d66424d664e4d664a4d66464d66544d66444d665c4d665a4d66684d66624d66664d66484d66724d666c4d66504d669e4d66764d66a04d66524d66584d66564d666e4d66604d666a4d66704d66744d665e4d66644d66a24d66784d668c4d668e4d66824d66a44d66864d668a4d66984d669a4d66904d667e4d66944d66884d66844d667c4d66924d66804d66964d66aa4d66a84d66a64d66ae4d66b04d66ac4d66b44d66b24d66b64d66b84d66ba4d66")\
            X(162 , 1, Key7               , "7"                 , "7"   , 0x37  , 55   , 'u', '7'   , '\x1f', "0008372808370808372608371208370c08371e08371c08370608372408370408370a08371008371608372008371808370e08372a08371a08372208371408377a08372c08372e08370208373008373208373408373a08373c08373808373608379c08374008373e08374c08374208374e08374a08374608375408374408375c08375a08376808376208376608374808377208376c08375008379e0837760837a008375208375808375608376e08376008376a08377008377408375e0837640837a208377808378c08378e0837820837a408378608378a08379808379a08379008377e08379408378808378408377c0837920837800837960837aa0837a80837a60837ae0837b00837ac0837b40837b20837b60837b81e37ba2837")\
            X( 163, 1, Numpad7            , "7"                 , "7"   , 0x37  , 57406, 'u', '7'   , '\x1f', "0047672847670847672647671247670c47671e47671c47670647672447670447670a47671047671647672047671847670e47672a47671a47672247671447677a47672c47672e47670247673047673247673447673a47673c47673847673647679c47674047673e47674c47674247674e47674a47674647675447674447675c47675a47676847676247676647674847677247676c47675047679e4767764767a047675247675847675647676e47676047676a47677047677447675e4767644767a247677847678c47678e4767824767a447678647678a47679847679a47679047677e47679447678847678447677c4767924767804767964767aa4767a84767a64767ae4767b04767ac4767b44767b24767b64767b84767ba4767")\
            X(164 , 1, Key8               , "8"                 , "8"   , 0x38  , 56   , 'u', '8'   , '\x7f', "0009382809380809382609381209380c09381e09381c09380609382409380409380a09381009381609382009381809380e09382a09381a09382209381409387a09382c09382e09380209383009383209383409383a09383c09383809383609389c09384009383e09384c09384209384e09384a09384609385409384409385c09385a09386809386209386609384809387209386c09385009389e0938760938a009385209385809385609386e09386009386a09387009387409385e0938640938a209387809388c09388e0938820938a409388609388a09389809389a09389009387e09389409388809388409387c0938920938800938960938aa0938a80938a60938ae0938b00938ac0938b40938b20938b60938b81f38ba2738")\
            X( 165, 1, Numpad8            , "8"                 , "8"   , 0x38  , 57407, 'u', '8'   , '\x7f', "0048682848680848682648681248680c48681e48681c48680648682448680448680a48681048681648682048681848680e48682a48681a48682248681448687a48682c48682e48680248683048683248683448683a48683c48683848683648689c48684048683e48684c48684248684e48684a48684648685448684448685c48685a48686848686248686648684848687248686c48685048689e4868764868a048685248685848685648686e48686048686a48687048687448685e4868644868a248687848688c48688e4868824868a448688648688a48689848689a48689048687e48689448688848688448687c4868924868804868964868aa4868a84868a64868ae4868b04868ac4868b44868b24868b64868b84868ba4868")\
            X(166 , 1, Key9               , "9"                 , "9"   , 0x39  , 57   , 'u', '9'   , '9'   , "000a39280a39080a39260a39120a390c0a391e0a391c0a39060a39240a39040a390a0a39100a39160a39200a39180a390e0a392a0a391a0a39220a39140a397a0a392c0a392e0a39020a39300a39320a39340a393a0a393c0a39380a39360a399c0a39400a393e0a394c0a39420a394e0a394a0a39460a39540a39440a395c0a395a0a39680a39620a39660a39480a39720a396c0a39500a399e0a39760a39a00a39520a39580a39560a396e0a39600a396a0a39700a39740a395e0a39640a39a20a39780a398c0a398e0a39820a39a40a39860a398a0a39980a399a0a39900a397e0a39940a39880a39840a397c0a39920a39800a39960a39aa0a39a80a39a60a39ae0a39b00a39ac0a39b40a39b20a39b60a39b82c39ba3539")\
            X( 167, 1, Numpad9            , "9"                 , "9"   , 0x39  , 57408, 'u', '9'   , '9'   , "0049692849690849692649691249690c49691e49691c49690649692449690449690a49691049691649692049691849690e49692a49691a49692249691449697a49692c49692e49690249693049693249693449693a49693c49693849693649699c49694049693e49694c49694249694e49694a49694649695449694449695c49695a49696849696249696649694849697249696c49695049699e4969764969a049695249695849695649696e49696049696a49697049697449695e4969644969a249697849698c49698e4969824969a449698649698a49699849699a49699049697e49699449698849698449697c4969924969804969964969aa4969a84969a64969ae4969b04969ac4969b44969b24969b64969b84969ba4969")\
            X(168 , 1, KeyMultiply        , "*"                 , "*"   , 0x2A  , 42   , 'u', '*'   , '*'   , "5e1bba9a0cdfaa2bdca82bdca62bdc")\
            X( 169, 1, NumpadMultiply     , "*"                 , "*"   , 0x2A  , 57411, 'u', '*'   , '*'   , "00376a28376a08376a26376a12376a0c376a1e376a1c376a06376a24376a04376a0a376a10376a16376a20376a18376a0e376a2a376a1a376a22376a14376a7a376a2c376a2e376a02376a30376a32376a34376a3a376a3c376a38376a36376a9c376a40376a3e376a4c376a42376a4e376a4a376a46376a54376a44376a5c376a5a376a68376a62376a66376a48376a72376a6c376a50376a9e376a76376aa0376a52376a58376a56376a6e376a60376a6a376a70376a74376a5e376a64376aa2376a78376a8c376a8e376a82376aa4376a86376a8a376a98376a9a376a90376a7e376a94376a88376a84376a7c376a92376a80376a96376aaa376aa8376aa6376aae376ab0376aac376ab4376ab2376ab6376ab8376aba376a")\
            X(170 , 1, KeySlash           , "/"                 , "/"   , 0x2F  , 47   , 'u', '/'   , '\x1f', "0035bf2835bf0835bf2635bf1235bf0c35bf1e35bf1c35bf0635bf2435bf0435bf0a35bf1035bf1635bf2035bf1835bf0e35bf2a35bf1a35bf2235bf1435bf7a35bf2c35bf2e35bf0235bf3035bf3235bf3435bf3a35bf3c35bf3835bf3635bf9c35bf4035bf3e35bf4c35bf4e73c14a73c15435bf4429de5e0cdbb21abfb60cbbb80bbfba04bf")\
            X( 171, 1, NumpadDivide       , "/"                 , "/"   , 0x2F  , 57410, 'u', '/'   , '\x1f', "01356f29356f09356f27356f13356f0d356f1f356f1d356f07356f25356f05356f0b356f11356f17356f21356f19356f0f356f2b356f1b356f23356f15356f7b356f2d356f2f356f03356f31356f33356f35356f3b356f3d356f39356f37356f9d356f41356f3f356f4d356f43356f4f356f4b356f47356f55356f45356f5d356f5b356f69356f63356f67356f49356f73356f6d356f51356f9f356f77356fa1356f53356f59356f57356f6f356f61356f6b356f71356f75356f5f356f65356fa3356f79356f8d356f8f356f83356fa5356f87356f8b356f99356f9b356f91356f7f356f95356f89356f85356f7d356f93356f81356f97356fab356fa9356fa7356faf356fb1356fad356fb5356fb3356fb7356fb9356fbb356f")\
            X(172 , 1, KeyPlus            , "Plus"              , "+"   , 0x2B  , 43   , 'u', '+'   , '+'   , "5c0cbb5a0cbb680cbb621bbb660cbb480dbb721abb6c0cbb760cbd521abb581bbb561bbb6e1bbb600cbb6a1bbb700cbb740cbb5e27bb640cbb782bbf8c1bbb8e1bbb821bbb860dbb8a0cbb881bbb841bbb920dbb800dbb960cbdb629c0")\
            X( 173, 1, NumpadPlus         , "Plus"              , "+"   , 0x2B  , 57413, 'u', '+'   , '+'   , "004e6b284e6b084e6b264e6b124e6b0c4e6b1e4e6b1c4e6b064e6b244e6b044e6b0a4e6b104e6b164e6b204e6b184e6b0e4e6b2a4e6b1a4e6b224e6b144e6b7a4e6b2c4e6b2e4e6b024e6b304e6b324e6b344e6b3a4e6b3c4e6b384e6b364e6b9c4e6b404e6b3e4e6b4c4e6b424e6b4e4e6b4a4e6b464e6b544e6b444e6b5c4e6b5a4e6b684e6b624e6b664e6b484e6b724e6b6c4e6b504e6b9e4e6b764e6ba04e6b524e6b584e6b564e6b6e4e6b604e6b6a4e6b704e6b744e6b5e4e6b644e6ba24e6b784e6b8c4e6b8e4e6b824e6ba44e6b864e6b8a4e6b984e6b9a4e6b904e6b7e4e6b944e6b884e6b844e6b7c4e6b924e6b804e6b964e6baa4e6ba84e6ba64e6bae4e6bb04e6bac4e6bb44e6bb24e6bb64e6bb84e6bba4e6b")\
            X(174 , 1, KeyMinus           , "Minus"             , "-"   , 0x2D  , 45   , 'u', '-'   , '-'   , "000cbd280cbd080cbd260cbd120cbd0c0cbd1e0cbd1c0cbd060cbd240cbd040cbd0a0cbd100cbd160cbd200cbd180cbd0e0cbd2a0cbd1a0cbd220cbd140cbd7a0cbd2c0cbd2e0cbd020cbd300cbd320cbd340cbd3a0cbd3c0cbd380cbd360cbd9c0cbd400cbd3e0cbd4c0cbd420cbd4e0cbd4a0cbd460cbd540cbd440cbd5c35bd5a35bd6835bd6235bd6635bd4835bd7235bd6c35bd500cbd9e35bf7635dda035bf5235bd5835bd5635bd6e35bd6035bd6a35bd7035bd7435bd5e35bd6435bda235bd780ddb8c35bd8e35bd8235bda435bd8635bd8a35bd9835bd9a0dbd9035bd7e35bd9435bd8835bd8435bd7c35bf9235bd8035bd9635bfae0dbdb00dbdac0dbdb228bdb60dbdb828bdba1ebd")\
            X( 175, 1, NumpadMinus        , "Minus"             , "-"   , 0x2D  , 57412, 'u', '-'   , '-'   , "004a6d284a6d084a6d264a6d124a6d0c4a6d1e4a6d1c4a6d064a6d244a6d044a6d0a4a6d104a6d164a6d204a6d184a6d0e4a6d2a4a6d1a4a6d224a6d144a6d7a4a6d2c4a6d2e4a6d024a6d304a6d324a6d344a6d3a4a6d3c4a6d384a6d364a6d9c4a6d404a6d3e4a6d4c4a6d424a6d4e4a6d4a4a6d464a6d544a6d444a6d5c4a6d5a4a6d684a6d624a6d664a6d484a6d724a6d6c4a6d504a6d9e4a6d764a6da04a6d524a6d584a6d564a6d6e4a6d604a6d6a4a6d704a6d744a6d5e4a6d644a6da24a6d784a6d8c4a6d8e4a6d824a6da44a6d864a6d8a4a6d984a6d9a4a6d904a6d7e4a6d944a6d884a6d844a6d7c4a6d924a6d804a6d964a6daa4a6da84a6da64a6dae4a6db04a6dac4a6db44a6db24a6db64a6db84a6dba4a6d")\
            X(176 , 1, KeyEqual           , "="                 , "="   , 0x3D  , 61   , 'u', '='   , '='   , "000dbb280dbb080dbb260dbb120dbb0c0dbb1e0dbb1c0dbb060dbb240dbb040dbb0a0dbb100dbb160dbb200dbb180dbb0e0dbb2a0dbb1a0dbb220dbb140dbb2c0dbb2e0dbb020dbb300dbb320dbb340dbb3a0dbb3c0dbb380dbb360dbb400dbb3e0dbb4c0dbb420dbb4e0dbb4a0dbb460dbb540dbb440dbb5035df9e0cbda00cbda20cbba40cbfaa0dbba80dbba60dbbae35bbb035bbac35bbb21bbbb81bbbba1bbb")\
            X( 177, 1, NumpadEqual        , "="                 , "="   , 0x3D  , 57415, 'u', '='   , '='   , "")\
            X(178 , 1, KeyPeriod          , "."                 , "."   , 0x2E  , 46   , 'u', '.'   , '.'   , "0034be2834be0834be2634be1234be0c34be1e34be1c34be0634be2434be0434be0a34be1034be1634be2034be1834be0e34be2a34be1a34be2234be1434be7a34be2c34be2e34be0234be3034be3234be3434be3a34be3c34be3834be3634be9c34be4034be3e34be4c34be4234be4e34be4a34be4634be5434be4434be5c34be5a34be6834be6234be6634be4834be7234be6c34be5034be9e34be7634bea034be5234be5834be5634be6e34be6034be6a34be7034be7434be5e34be6434bea234be7834be8c34be8e34be8234bea434be8634be8a34be9834be9a35be9034be7e34be9434be8834be8434be7c34be9234be8034be9634beb212beb634beb813beba18be")\
            X( 179, 1, NumpadDecimal      , "."                 , "."   , 0x2E  , 57409, 'u', '.'   , '.'   , "00536e28536e08536e26536e12536e0c536e1e536e1c536e06536e24536e04536e0a536e10536e16536e20536e18536e0e536e2a536e1a536e22536e14536e7a536e2c536e2e536e02536e30536e32536e34536e3a536e3c536e38536e36536e9c536e40536e3e536e4c536e42536e4e536e4a536e46536e54536e44536e5c536e5a536e68536e62536e66536e48536e72536e6c536e50536e9e536e76536ea0536e52536e58536e56536e6e536e60536e6a536e70536e74536e5e536e64536ea2536e78536e8c536e8e536e82536ea4536e86536e8a536e98536e9a536e90536e7e536e94536e88536e84536e7c536e92536e80536e96536eaa536ea8536ea6536eae536eb0536eac536eb4536eb2536eb6536eb8536eba536e")\
            X(180 , 1, KeyComma           , ","                 , ","   , 0x2C  , 44   , 'u', ','   , ','   , "0033bc2833bc0833bc2633bc1233bc0c33bc1e33bc1c33bc0633bc2433bc0433bc0a33bc1033bc1633bc2033bc1833bc0e33bc2a33bc1a33bc2233bc1433bc7a33bc2c33bc2e33bc0233bc3033bc3233bc3433bc3a33bc3c33bc3833bc3633bc9c33bc4033bc3e33bc4c33bc4233bc4e33bc4a33bc4633bc5433bc4433bc5c33bc5a33bc6833bc6233bc6633bc4833bc7233bc6c33bc5033bc9e33bc7633bca033bc5233bc5833bc5633bc6e33bc6033bc6a33bc7033bc7433bc5e33bc6433bca233bc7833bc8c33bc8e33bc8233bca433bc8633bc8a33bc9833bc9a2bbc9033bc7e33bc9433bc8833bc8433bc7c33bc9233bc8033bc9633bcaa32bca832bca632bcae32bcb032bcac32bcb211bcb635bcb82fbcba33bc")\
            X( 181, 1, NumpadPoint        , ","                 , ","   , 0x2C  , 57416, 'u', ','   , ','   , "007ec2287ec2087ec2267ec2127ec20c7ec21e7ec21c7ec2067ec2247ec2047ec20a7ec2107ec2167ec2207ec2187ec20e7ec22a7ec21a7ec2227ec2147ec27a7ec22c7ec22e7ec2027ec2307ec2327ec2347ec23a7ec23c7ec2387ec2367ec29c7ec2407ec23e7ec24c7ec2427ec24e7ec24a7ec2467ec2547ec2447ec25c7ec25a7ec2687ec2627ec2667ec2487ec2727ec26c7ec2507ec29e7ec2767ec2a07ec2527ec2587ec2567ec26e7ec2607ec26a7ec2707ec2747ec25e7ec2647ec2a27ec2787ec28c7ec28e7ec2827ec2a47ec2867ec28a7ec2987ec29a7ec2907ec27e7ec2947ec2887ec2847ec27c7ec2927ec2807ec2967ec2aa7ec2a87ec2a67ec2ae7ec2b07ec2ac7ec2b47ec2b27ec2b67ec2b87ec2ba7ec2")\
            X(182 , 1, Colon              , ":"                 , ":"   , 0x3A  , 58   , 'u', ':'   , ':'   , "aa34bfa834bfa634bfae34bfb034bfac34bf")\
            X(184 , 1, Semicolon          , ";"                 , ";"   , 0x3B  , 59   , 'u', ';'   , ';'   , "0027ba2827ba0827ba2627ba1227ba0c27ba1e27ba1c27ba0627ba2427ba0427ba0a27ba1027ba1627ba2027ba1827ba0e27ba2a27ba1a27ba2227ba1427ba7a27ba2c27ba2e27ba0227ba3027ba3227ba3427ba3a27ba3c27ba3827ba3627ba9c27ba4027ba3e27ba4227ba4e35bf4a35bf4627ba4427ba9e29c0a029c0a229c0a429c0aa33bea833bea633beae33beb033beac33beb22cbab81ababa10ba")\
            X(186 , 1, TurnedComma        , "ʻ"                 , "ʻ"   , 0x02BB, 699  , 'u', -1    , -1    , "2e28de")\
            X(188 , 1, OpenSquareBracket  , "["                 , "["   , 0x5B  , 91   , 'u', '['   , '\x1b', "001adb281adb081adb261adb121adb0c1adb1e1adb1c1adb061adb241adb041adb0a1adb101adb161adb201adb181adb0e1adb2a1adb1a1adb221adb141adb7a1adb2c1adb2e1adb021adb301adb321adb341adb3a1adb3c1adb381adb361adb4e1bdd4a1bdd5428de481adbb20cdbb80cdbba02db")\
            X(190 , 1, CloseSquareBracket , "]"                 , "]"   , 0x5D  , 93   , 'u', ']'   , '\x1d', "001bdd281bdd081bdd261bdd121bdd0c1bdd1e1bdd1c1bdd061bdd241bdd041bdd0a1bdd101bdd161bdd201bdd181bdd0e1bdd2a1bdd1a1bdd221bdd141bdd7a1bdd2c1bdd2e1bdd021bdd301bdd321bdd341bdd3a1bdd3c1bdd381bdd361bdd4e2bdc4a2bdc542bdc481bdd720ddb5e56e29629c0b20dddb80dddba03dd")\
            X(192 , 1, OpenCurlyBracket   , "{"                 , "{"   , 0x7B  , 123  , 'u', '{'   , 27    , "6228de6a28de")\
            X(194 , 1, CloseCurlyBracket  , "}"                 , "}"   , 0x7D  , 125  , 'u', '}'   , 29    , "622bbf721bdd6a2bbf")\
            X(196 , 1, CloseRoundBracket  , ")"                 , ")"   , 0x29  , 41   , 'u', ')'   , ')'   , "a01bdda21bddaa0cdba80cdba60cdbae0cdbb00cdbac0cdb")\
            X(198 , 1, LessThan           , "<"                 , "<"   , 0x3C  , 60   , 'u', '<'   , '<'   , "3029c0422bdc5456e25c56e25a56e26856e26256e26656e24856e27256e26c56e25056e27656e25256e25856e25656e26e56e26056e26a56e27056e27456e25e2bdc6456e27856e28c56e28e56e28256e28656e28a56e29a56e29056e27e56e29456e28856e28456e29256e28056e29656e2aa56e2a856e2a656e2ae56e2b056e2ac56e2b456e2b656e2")\
            X(200 , 1, BackSlash          , "\\"                , "\\"  , 0x5C  , 92   , 'u', '\\'  , '\x1c', "002bdc0056e22856e2282bdc0856e2082bdc262bdc2656e21256e2122bdc0c2bdc0c56e21e2bdc1e56e21c2bdc1c56e2062bdc0656e2242bdc2456e20456e2042bdc0a56e20a2bdc102bdc1056e2162bdc1656e22056e2202bdc182bdc1856e20e2bdc0e56e22a56e22a2bdc1a2bdc1a56e22256e2222bdc1456e2142bdc7a56e27a2bdc2c56e22e2bdc2e56e2022bdc0256e23056e2322bde3256dc342bde3456dc3a56dc3c56dc3856dc3656dc9c56e29c2bdc4c56e24e56e24a56e25429c0680ddb4829c07229c0a056e25229dc5829dc5629dc600ddba256e2b256e2b22bdcb82bdcb856e2ba2bdcba56e2")\
            X(202 , 1, Underscore         , "_"                 , "_"   , 0x5F  , 95   , 'u', '_'   , '\x1f', "0673e2")\
            X(204 , 1, VerticalBar        , "|"                 , "|"   , 0x7C  , 124  , 'u', '|'   , 28    , "6829dc6229dc6029dc6a29dc")\
            X(206 , 1, DivisionSign       , "÷"                 , "÷"   , 0xF7  , 247  , 'u', -1    , -1    , "501add")\
            X(208 , 1, OneHalf            , "½"                 , "½"   , 0xBD  , 189  , 'u', -1    , -1    , "6629dc7429dc6429dc")\
            X(210 , 1, SuperscriptTwo     , "²"                 , "²"   , 0xB2  , 178  , 'u', -1    , -1    , "a829dea629deae29deb029deac29de")\
            X(212 , 1, DegreeSign         , "°"                 , "°"   , 0xB0  , 176  , 'u', -1    , -1    , "2c2bdc4629de5e0dbf7829dc")\
            X(214 , 1, NumeroSign         , "º"                 , "º"   , 0xBA  , 186  , 'u', -1    , -1    , "5228de6e29dc")\
            X(216 , 1, Acute              , "´"                 , "´"   , 0xB4  , 180  , 'u', -1    , -1    , "4e1adb4a1adb5c0ddb5a0ddb621aba660ddb4827ba6c0ddb502bbf9e0dbb760dbba00dbb521bba6e28de6a1aba700ddb740ddb5e28c0640ddba20dbf7828de8c0ddd8e0ddd820ddda40ddf880ddd840ddd")\
            X(218 , 1, Caron              , "ˇ"                 , "ˇ"   , 0x02C7, 780  , 'u', -1    , -1    , "7629de")\
            X(220 , 1, Cedilla            , "¸"                 , "¸"   , 0xB8  , 184  , 'u', -1    , -1    , "421bdd9229c08029c0")\
            X(222 , 1, Circumflex         , "^"                 , "^"   , 0x5E  , 94   , 'u', '^'   , 30    , "421adb461adb441adb4828de8c29dc8e29dc8229dc900ddd7e0ddd940ddd8829dc8429dcaa1adda81adda61addae1addb01addac1add")\
            X(224 , 1, Ogonek             , "˛"                 , "˛"   , 0x02DB, 731  , 'u', -1    , -1    , "8a29c0")\
            X(226 , 1, Tilde              , "~"                 , "~"   , 0x7E  , 771  , 'u', '~'   , 30    , "4e28de4a28de522bbf")\
            X(228 , 1, Tonos              , "΄"                 , "΄"   , 0x0384, 900  , 'u', -1    , -1    , "7227ba")\
            X(230 , 1, Umlaut             , "¨"                 , "¨"   , 0xA8  , 168  , 'u', -1    , -1    , "5c1bba5a1bba681bba661bba7228de6c1bba500ddba02bdc601bba701bba5e1add641bbaa22bdc901bc07e1bc0941bc0")\
            X(232 , 1, BackQuote          , "`"                 , "`"   , 0x60  , 96   , 'u', '`'   , '`'   , "0029c02829c00829c02629c01229c00c29c01e29c01c29c00629c02429c00429c00a29c01029c01629c02029c01829c00e29c02a29c01a29c02229c01429c07a29c02c29c02e29c00229c0302bdc3229df3429df3a29df3c29df3829df3629df9c29c04228c0482bdc501bba6e1abab429dcb229c0b829c0ba29c0")\
            X(234 , 1, SingleQuote        , "'"                 , "'"   , 0x27  , 39   , 'u', '\''  , '\''  , "0028de2828de0828de2628de1228de0c28de1e28de1c28de0628de2428de0428de0a28de1028de1628de2028de1828de0e28de2a28de1a28de2228de1428de7a28de2c28de0228de3028de3228c03428c03a28c03c28c03828c03628c09c28de4028de3e28de4e29c04a29c05c2bbf5a2bbf682bbf620cdb662bbf480cbf720cbf6c2bbf5029dc762bdc520cdb580cdb560cdb6e0cdb602bbf6a0cdb702bbf742bbf642bbf781bba860cbf8a0dbf900cdb7e0cdb940cdb920cbf800cbf960dbbb210deb835deba2cde")\
            X(236 , 1, DoubleQuote        , "\""                , "\""  , 0x22  , 34   , 'u', '"'   , '"'   , "9a29c0")\
            X(238 , 1, SingleLowQuote     , "‚"                 , "‚"   , 0x201A, 8218 , 'u', -1    , -1    , "8629c0")\
            X(240 , 1, DoubleLowQuote     , "„"                 , "„"   , 0x201E, 8222 , 'u', -1    , -1    , "4c29c0")\
            X(242 , 1, LeftGuillemet      , "«"                 , "«"   , 0xAB  , 171  , 'u', -1    , -1    , "4256e2520ddd")\
            X(244 , 1, Hash               , "#"                 , "#"   , 0x23  , 35   , 'u', '#'   , '#'   , "3a2bde3c2bde382bde362bde402bdc4229de722bdc8e2bbf882bbf842bbf")\
            X(246 , 1, AtSign             , "@"                 , "@"   , 0x40  , 64   , 'u', '@'   , '\0'  , "541bdd5e29de")\
            X(248 , 1, Exclamation        , "!"                 , "!"   , 0x21  , 33   , 'u', '!'   , '!'   , "aa35dfa835dfa635df")\
            X(250 , 1, InvertedExclamation, "¡"                 , "¡"   , 0xA1  , 161  , 'u', -1    , -1    , "6e0ddd")\
            X(252 , 1, QuestionMark       , "?"                 , "?"   , 0x3F  , 63   , 'u', '?'   , 127   , "b40cbb")\
            X(254 , 1, InvertedQuestion   , "¿"                 , "¿"   , 0xBF  , 191  , 'u', -1    , -1    , "620ddd6a0ddd")\
            X(256 , 1, Paragraph          , "§"                 , "§"   , 0xA7  , 167  , 'u', -1    , -1    , "5c29dc5a29dc6c29dc9e28dea028de7029dca228dea428de9029bf7e29bf9429bf")\
            X(258 , 1, Ampersand          , "&"                 , "&"   , 0x26  , 38   , 'u', '&'   , '&'   , "9e56e2a456e2")\
            X(260 , 1, Dollar             , "$"                 , "$"   , 0x24  , 36   , 'u', '$'   , '$'   , "902bdf7e2bdf942bdfa81bbaa61bbaae1bbab01bbaac1bba")\
            X(262 , 1, Dong               , "₫"                 , "₫"   , 0x20AB, 8363 , 'u', -1    , -1    , "9c0dbb")\
            X(264 , 1, Yen                , "¥"                 , "¥"   , 0xA5  , 165  , 'u', -1    , -1    , "067ddc027ddc")\
            X(266 , 1, DotlessI           , "ı"                 , "ı"   , 0x0131, 305  , 'u', -1    , -1    , "9a1749b61349")\
            X(268 , 1, MicroSign          , "µ"                 , "µ"   , 0xB5  , 181  , 'u', -1    , -1    , "ae2bdcb02bdcac2bdc")\
            X(270 , 1, Eth                , "ð"                 , "ð"   , 0xF0  , 240  , 'u', -1    , -1    , "741bba781add")\
            X(272 , 1, Thorn              , "þ"                 , "þ"   , 0xFE  , 254  , 'u', -1    , -1    , "7835bd")\
            X(274 , 1, Eszett             , "ẞ"                 , "ß"   , 0xDF  , 223  , 'u', -1    , -1    , "8c0cdb8e0cdb820cdb880cdb840cdb")\
            X(276 , 1, KeyA               , "A"                 , "a"   , 0x61  , 97   , 'u', 'a'   , '\x01', "001e41281e41081e41261e41121e410c1e411e1e411c1e41061e41241e41041e410a1e41101e41161e41201e41181e410e1e412a1e411a1e41221e41141e417a1e412c1e412e1e41021e41301e41321e41341e413a1e413c1e41381e41361e419c1e41401e413e1e414c1e41421e414e1e414a1e41461e41541e41441e415c1e415a1e41681e41621e41661e41481e41721e416c1e41501e419e1e41761e41a01e41521e41581e41561e416e1e41601e416a1e41701e41741e415e1e41641e41a21e41781e418c1e418e1e41821e41a41e41861e418a1e41981e419a1e41901e417e1e41941e41881e41841e417c1e41921e41801e41961e41aa1041a81041a61041ae1041b01041ac1041b41e41b21e41b62141b82141ba2541")\
            X(278 , 1, KeyB               , "B"                 , "b"   , 0x62  , 98   , 'u', 'b'   , '\x02', "0030422830420830422630421230420c30421e30421c30420630422430420430420a30421030421630422030421830420e30422a30421a30422230421430427a30422c30422e30420230423030423230423430423a30423c30423830423630429c30424030423e30424c30424230424e30424a30424630425430424430425c30425a30426830426230426630424830427230426c30425030429e3042763042a030425230425830425630426e30426030426a30427030427430425e3042643042a230427830428c30428e3042823042a430428630428a30429830429a30429030427e30429430428830428430427c3042923042803042963042aa3042a83042a63042ae3042b03042ac3042b43042b23142b63342b81942ba1242")\
            X(280 , 1, KeyC               , "C"                 , "c"   , 0x63  , 99   , 'u', 'c'   , '\x03', "002e43282e43082e43262e43122e430c2e431e2e431c2e43062e43242e43042e430a2e43102e43162e43202e43182e430e2e432a2e431a2e43222e43142e437a2e432c2e432e2e43022e43302e43322e43342e433a2e433c2e43382e43362e439c2e43402e433e2e434c2e43422e434e2e434a2e43462e43542e43442e435c2e435a2e43682e43622e43662e43482e43722e436c2e43502e439e2e43762e43a02e43522e43582e43562e436e2e43602e436a2e43702e43742e435e2e43642e43a22e43782e438c2e438e2e43822e43a42e43862e438a2e43982e439a2e43902e437e2e43942e43882e43842e437c2e43922e43802e43962e43aa2e43a82e43a62e43ae2e43b02e43ac2e43b42e43b21743b62f43b82643ba2043")\
            X(282 , 1, KeyD               , "D"                 , "d"   , 0x64  , 100  , 'u', 'd'   , '\x04', "0020442820440820442620441220440c20441e20441c20440620442420440420440a20441020441620442020441820440e20442a20441a20442220441420447a20442c20442e20440220443020443220443420443a20443c20443820443620449c20444020443e20444c20444220444e20444a20444620445420444420445c20445a20446820446220446620444820447220446c20445020449e2044762044a020445220445820445620446e20446020446a20447020447420445e2044642044a220447820448c20448e2044822044a420448620448a20449820449a20449020447e20449420448820448420447c2044922044802044962044aa2044a82044a62044ae2044b02044ac2044b42044b22344b61544b82544ba2144")\
            X(284 , 1, KeyE               , "E"                 , "e"   , 0x65  , 101  , 'u', 'e'   , '\x05', "0012452812450812452612451212450c12451e12451c12450612452412450412450a12451012451612452012451812450e12452a12451a12452212451412457a12452c12452e12450212453012453212453412453a12453c12453812453612459c12454012453e12454c12454212454e12454a12454612455412454412455c12455a12456812456212456612454812457212456c12455012459e1245761245a012455212455812455612456e12456012456a12457012457412455e1245641245a212457812458c12458e1245821245a412458612458a12459812459a12459012457e12459412458812458412457c1245921245801245961245aa1245a81245a61245ae1245b01245ac1245b41245b22045b62045b82245ba2445")\
            X(286 , 1, KeyF               , "F"                 , "f"   , 0x66  , 102  , 'u', 'f'   , '\x06', "0021462821460821462621461221460c21461e21461c21460621462421460421460a21461021461621462021461821460e21462a21461a21462221461421467a21462c21462e21460221463021463221463421463a21463c21463821463621469c21464021463e21464c21464221464e21464a21464621465421464421465c21465a21466821466221466621464821467221466c21465021469e2146762146a021465221465821465621466e21466021466a21467021467421465e2146642146a221467821468c21468e2146822146a421468621468a21469821469a21469021467e21469421468821468421467c2146922146802146962146aa2146a82146a62146ae2146b02146ac2146b434beb21546b61046b80946ba0646")\
            X(288 , 1, KeyG               , "G"                 , "g"   , 0x67  , 103  , 'u', 'g'   , '\x07', "0022472822470822472622471222470c22471e22471c22470622472422470422470a22471022471622472022471822470e22472a22471a22472222471422477a22472c22472e22470222473022473222473422473a22473c22473822473622479c22474022473e22474c22474222474e22474a22474622475422474422475c22475a22476822476222476622474822477222476c22475022479e2247762247a022475222475822475622476e22476022476a22477022477422475e2247642247a222477822478c22478e2247822247a422478622478a22479822479a22479022477e22479422478822478422477c2247922247802247962247aa2247a82247a62247ae2247b02247ac2247b42247b21647b61147b83447ba2e47")\
            X(290 , 1, KeyH               , "H"                 , "h"   , 0x68  , 104  , 'u', 'h'   , '\x08', "0023482823480823482623481223480c23481e23481c23480623482423480423480a23481023481623482023481823480e23482a23481a23482223481423487a23482c23482e23480223483023483223483423483a23483c23483823483623489c23484023483e23484c23484223484e23484a23484623485423484423485c23485a23486823486223486623484823487223486c23485023489e2348762348a023485223485823485623486e23486023486a23487023487423485e2348642348a223487823488c23488e2348822348a423488623488a23489823489a23489023487e23489423488823488423487c2348922348802348962348aa2348a82348a62348ae2348b02348ac2348b42348b22448b61848b82348ba2348")\
            X(292 , 1, KeyI               , "I"                 , "i"   , 0x69  , 105  , 'u', 'i'   , '\x09', "0017492817490817492617491217490c17491e17491c17490617492417490417490a17491017491617492017491817490e17492a17491a17492217491417497a17492c17492e17490217493017493217493417493a17493c17493817493617499c17494017493e17494c17494217494e17494a17494617495417494417495c17495a17496817496217496617494817497217496c17495017499e1749761749a017495217495817495617496e17496017496a17497017497417495e1749641749a217497817498c17498e1749821749a417498617498a17499817499a28de9017497e17499417498817498417497c1749921749801749961749aa1749a81749a61749ae1749b01749ac1749b41749b22249b61fdbb83049ba3249")\
            X(294 , 1, KeyJ               , "J"                 , "j"   , 0x6A  , 106  , 'u', 'j'   , '\x0a', "00244a28244a08244a26244a12244a0c244a1e244a1c244a06244a24244a04244a0a244a10244a16244a20244a18244a0e244a2a244a1a244a22244a14244a7a244a2c244a2e244a02244a30244a32244a34244a3a244a3c244a38244a36244a9c244a40244a3e244a4c244a42244a4e244a4a244a46244a54244a44244a5c244a5a244a68244a62244a66244a48244a72244a6c244a50244a9e244a76244aa0244a52244a58244a56244a6e244a60244a6a244a70244a74244a5e244a64244aa2244a78244a8c244a8e244a82244aa4244a86244a8a244a98244a9a244a90244a7e244a94244a88244a84244a7c244a92244a80244a96244aaa244aa8244aa6244aae244ab0244aac244ab4244ab22e4ab62c4ab8064aba094a")\
            X(296 , 1, KeyK               , "K"                 , "k"   , 0x6B  , 107  , 'u', 'k'   , '\x0b', "00254b28254b08254b26254b12254b0c254b1e254b1c254b06254b24254b04254b0a254b10254b16254b20254b18254b0e254b2a254b1a254b22254b14254b7a254b2c254b2e254b02254b30254b32254b34254b3a254b3c254b38254b36254b9c254b40254b3e254b4c254b42254b4e254b4a254b46254b54254b44254b5c254b5a254b68254b62254b66254b48254b72254b6c254b50254b9e254b76254ba0254b52254b58254b56254b6e254b60254b6a254b70254b74254b5e254b64254ba2254b78254b8c254b8e254b82254ba4254b86254b8a254b98254b9a254b90254b7e254b94254b88254b84254b7c254b92254b80254b96254baa254ba8254ba6254bae254bb0254bac254bb4254bb22f4bb6244bb8274bba1f4b")\
            X(298 , 1, KeyL               , "L"                 , "l"   , 0x6C  , 108  , 'u', 'l'   , '\x0c', "00264c28264c08264c26264c12264c0c264c1e264c1c264c06264c24264c04264c0a264c10264c16264c20264c18264c0e264c2a264c1a264c22264c14264c7a264c2c264c2e264c02264c30264c32264c34264c3a264c3c264c38264c36264c9c264c40264c3e264c4c264c42264c4e264c4a264c46264c54264c44264c5c264c5a264c68264c62264c66264c48264c72264c6c264c50264c9e264c76264ca0264c52264c58264c56264c6e264c60264c6a264c70264c74264c5e264c64264ca2264c78264c8c264c8e264c82264ca4264c86264c8a264c98264c9a264c90264c7e264c94264c88264c84264c7c264c92264c80264c96264caa264ca8264ca6264cae264cb0264cac264cb4264cb2194cb6264cb8074cba084c")\
            X(300 , 1, KeyM               , "M"                 , "m"   , 0x6D  , 109  , 'u', 'm'   , '\x0d', "00324d28324d08324d26324d12324d0c324d1e324d1c324d06324d24324d04324d0a324d10324d16324d20324d18324d0e324d2a324d1a324d22324d14324d7a324d2c324d2e324d02324d30324d32324d34324d3a324d3c324d38324d36324d9c324d40324d3e324d4c324d42324d4e324d4a324d46324d54324d44324d5c324d5a324d68324d62324d66324d48324d72324d6c324d50324d9e324d76324da0324d52324d58324d56324d6e324d60324d6a324d70324d74324d5e324d64324da2324d78324d8c324d8e324d82324da4324d86324d8a324d98324d9a324d90324d7e324d94324d88324d84324d7c324d92324d80324d96324daa274da8274da6274dae274db0274dac274db4324db2324db6254db8084dba074d")\
            X(302 , 1, KeyN               , "N"                 , "n"   , 0x6E  , 110  , 'u', 'n'   , '\x0e', "00314e28314e08314e26314e12314e0c314e1e314e1c314e06314e24314e04314e0a314e10314e16314e20314e18314e0e314e2a314e1a314e22314e14314e7a314e2c314e2e314e02314e30314e32314e34314e3a314e3c314e38314e36314e9c314e40314e3e314e4c314e42314e4e314e4a314e46314e54314e44314e5c314e5a314e68314e62314e66314e48314e72314e6c314e50314e9e314e76314ea0314e52314e58314e56314e6e314e60314e6a314e70314e74314e5e314e64314ea2314e78314e8c314e8e314e82314ea4314e86314e8a314e98314e9a314e90314e7e314e94314e88314e84314e7c314e92314e80314e96314eaa314ea8314ea6314eae314eb0314eac314eb4314eb2264eb6174eb8314eba314e")\
            X(304 , 1, KeyO               , "O"                 , "o"   , 0x6F  , 111  , 'u', 'o'   , '\x0f', "00184f28184f08184f26184f12184f0c184f1e184f1c184f06184f24184f04184f0a184f10184f16184f20184f18184f0e184f2a184f1a184f22184f14184f7a184f2c184f2e184f02184f30184f32184f34184f3a184f3c184f38184f36184f9c184f40184f3e184f4c184f42184f4e184f4a184f46184f54184f44184f5c184f5a184f68184f62184f66184f48184f72184f6c184f50184f9e184f76184fa0184f52184f58184f56184f6e184f60184f6a184f70184f74184f5e184f64184fa2184f78184f8c184f8e184f82184fa4184f86184f8a184f98184f9a184f90184f7e184f94184f88184f84184f7c184f92184f80184f96184faa184fa8184fa6184fae184fb0184fac184fb4184fb21f4fb6144fb8144fba174f")\
            X(306 , 1, KeyP               , "P"                 , "p"   , 0x70  , 112  , 'u', 'p'   , '\x10', "0019502819500819502619501219500c19501e19501c19500619502419500419500a19501019501619502019501819500e19502a19501a19502219501419507a19502c19502e19500219503019503219503419503a19503c19503819503619509c19504019503e19504c19504219504e19504a19504619505419504419505c19505a19506819506219506619504819507219506c19505019509e1950761950a019505219505819505619506e19506019506a19507019507419505e1950641950a219507819508c19508e1950821950a419508619508a19509819509a19509019507e19509419508819508419507c1950921950801950961950aa1950a81950a61950ae1950b01950ac1950b41950b21350b61950b80a50ba0550")\
            X(308 , 1, KeyQ               , "Q"                 , "q"   , 0x71  , 113  , 'u', 'q'   , '\x11', "0010512810510810512610511210510c10511e10511c10510610512410510410510a10511010511610512010511810510e10512a10511a10512210511410517a10512c10512e10510210513010513210513410513a10513c10513810513610519c10514010513e10514c10514210514e10514a10514610515410514410515c10515a10516810516210516610514810517210516c10515010519e1051761051a010515210515810515610516e10516010516a10517010517410515e1051641051a210517810518c10518e1051821051a410518610518a10519810519a10519010517e10519410518810518410517c1051921051801051961051aa1e51a81e51a61e51ae1e51b01e51ac1e51b42bbfb22d51b61a51b81251ba1151")\
            X(310 , 1, KeyR               , "R"                 , "r"   , 0x72  , 114  , 'u', 'r'   , '\x12', "0013522813520813522613521213520c13521e13521c13520613522413520413520a13521013521613522013521813520e13522a13521a13522213521413527a13522c13522e13520213523013523213523413523a13523c13523813523613529c13524013523e13524c13524213524e13524a13524613525413524413525c13525a13526813526213526613524813527213526c13525013529e1352761352a013525213525813525613526e13526013526a13527013527413525e1352641352a213527813528c13528e1352821352a413528613528a13529813529a13529013527e13529413528813528413527c1352921352801352961352aa1352a81352a61352ae1352b01352ac1352b41352b21852b61652b81552ba1552")\
            X(312 , 1, KeyS               , "S"                 , "s"   , 0x73  , 115  , 'u', 's'   , '\x13', "001f53281f53081f53261f53121f530c1f531e1f531c1f53061f53241f53041f530a1f53101f53161f53201f53181f530e1f532a1f531a1f53221f53141f537a1f532c1f532e1f53021f53301f53321f53341f533a1f533c1f53381f53361f539c1f53401f533e1f534c1f53421f534e1f534a1f53461f53541f53441f535c1f535a1f53681f53621f53661f53481f53721f536c1f53501f539e1f53761f53a01f53521f53581f53561f536e1f53601f536a1f53701f53741f535e1f53641f53a21f53781f538c1f538e1f53821f53a41f53861f538a1f53981f539a1f53901f537e1f53941f53881f53841f537c1f53921f53801f53961f53aa1f53a81f53a61f53ae1f53b01f53ac1f53b41f53b22753b63253b81653ba1653")\
            X(314 , 1, KeyT               , "T"                 , "t"   , 0x74  , 116  , 'u', 't'   , '\x14', "0014542814540814542614541214540c14541e14541c14540614542414540414540a14541014541614542014541814540e14542a14541a14542214541414547a14542c14542e14540214543014543214543414543a14543c14543814543614549c14544014543e14544c14544214544e14544a14544614545414544414545c14545a14546814546214546614544814547214546c14545014549e1454761454a014545214545814545614546e14546014546a14547014547414545e1454641454a214547814548c14548e1454821454a414548614548a14549814549a14549014547e14549414548814548414547c1454921454801454961454aa1454a81454a61454ae1454b01454ac1454b41454b22554b62354b82454ba2254")\
            X(316 , 1, KeyU               , "U"                 , "u"   , 0x75  , 117  , 'u', 'u'   , '\x15', "0016552816550816552616551216550c16551e16551c16550616552416550416550a16551016551616552016551816550e16552a16551a16552216551416557a16552c16552e16550216553016553216553416553a16553c16553816553616559c16554016553e16554c16554216554e16554a16554616555416554416555c16555a16556816556216556616554816557216556c16555016559e1655761655a016555216555816555616556e16556016556a16557016557416555e1655641655a216557816558c16558e1655821655a416558616558a16559816559a16559016557e16559416558816558416557c1655921655801655961655aa1655a81655a61655ae1655b01655ac1655b41655b22155b61e55b81755ba1455")\
            X(318 , 1, KeyV               , "V"                 , "v"   , 0x76  , 118  , 'u', 'v'   , '\x16', "002f56282f56082f56262f56122f560c2f561e2f561c2f56062f56242f56042f560a2f56102f56162f56202f56182f560e2f562a2f561a2f56222f56142f567a2f562c2f562e2f56022f56302f56322f56342f563a2f563c2f56382f56362f569c2f56402f563e2f564c2f56422f564e2f564a2f56462f56542f56442f565c2f565a2f56682f56622f56662f56482f56722f566c2f56502f569e2f56762f56a02f56522f56582f56562f566e2f56602f566a2f56702f56742f565e2f56642f56a22f56782f568c2f568e2f56822f56a42f56862f568a2f56982f569a2f56902f567e2f56942f56882f56842f567c2f56922f56802f56962f56aa2f56a82f56a62f56ae2f56b02f56ac2f56b42f56b23456b62e56b83356ba2f56")\
            X(320 , 1, KeyW               , "W"                 , "w"   , 0x77  , 119  , 'u', 'w'   , '\x17', "0011572811570811572611571211570c11571e11571c11570611572411570411570a11571011571611572011571811570e11572a11571a11572211571411577a11572c11572e11570211573011573211573411573a11573c11573811573611579c11574011573e11574c11574211574e11574a11574611575411574411575c11575a11576811576211576611574811577211576c11575011579e1157761157a011575211575811575611576e11576011576a11577011577411575e1157641157a211577811578c11578e1157821157a411578611578a11579811579a11579011577e11579411578811578411577c1157921157801157961157aa2c57a82c57a62c57ae2c57b02c57ac2c57b41bbab23357b61b57b83257ba3057")\
            X(322 , 1, KeyX               , "X"                 , "x"   , 0x78  , 120  , 'u', 'x'   , '\x18', "002d58282d58082d58262d58122d580c2d581e2d581c2d58062d58242d58042d580a2d58102d58162d58202d58182d580e2d582a2d581a2d58222d58142d587a2d582c2d582e2d58022d58302d58322d58342d583a2d583c2d58382d58362d589c2d58402d583e2d584c2d58422d584e2d584a2d58462d58542d58442d585c2d585a2d58682d58622d58662d58482d58722d586c2d58502d589e2d58762d58a02d58522d58582d58562d586e2d58602d586a2d58702d58742d585e2d58642d58a22d58782d588c2d588e2d58822d58a42d58862d588a2d58982d589a2d58902d587e2d58942d58882d58842d587c2d58922d58802d58962d58aa2d58a82d58a62d58ae2d58b02d58ac2d58b40ddbb23058b62b58b82e58ba2d58")\
            X(324 , 1, KeyY               , "Y"                 , "y"   , 0x79  , 121  , 'u', 'y'   , '\x19', "0015592815590815592615591215590c15591e15591c15590615592415590415590a15591015591615592015591815590e15592a15591a15592215591415597a15592c15592e15590215593015593215593415593a15593c15593815593615599c15594015593e15594c15594215594e15594a1559461559542c594415595c15595a15596815596215596615594815597215596c15595015599e1559761559a015595215595815595615596e15596015596a15597015597415595e1559641559a22c597815598c2c598e2c59822c59a42c59862c598a2c59982c599a1559902c597e2c59942c59882c59842c597c1559922c59802c59962c59aa1559a81559a61559ae1559b01559ac1559b41559b21459b62759b81859ba1359")\
            X(326 , 1, KeyZ               , "Z"                 , "z"   , 0x7A  , 122  , 'u', 'z'   , '\x1a', "002c5a282c5a082c5a262c5a122c5a0c2c5a1e2c5a1c2c5a062c5a242c5a042c5a0a2c5a102c5a162c5a202c5a182c5a0e2c5a2a2c5a1a2c5a222c5a142c5a7a2c5a2c2c5a2e2c5a022c5a302c5a322c5a342c5a3a2c5a3c2c5a382c5a362c5a9c2c5a402c5a3e2c5a4c2c5a422c5a4e2c5a4a2c5a462c5a54155a442c5a5c2c5a5a2c5a682c5a622c5a662c5a482c5a722c5a6c2c5a502c5a9e2c5a762c5aa02c5a522c5a582c5a562c5a6e2c5a602c5a6a2c5a702c5a742c5a5e2c5a642c5aa2155a782c5a8c155a8e155a82155aa4155a86155a8a155a98155a9a2c5a90155a7e155a94155a88155a84155a7c2c5a92155a80155a96155aaa115aa8115aa6115aae115ab0115aac115ab42c5ab2355ab6315ab8205aba265a")\
            X(328 , 1, AeLigature         , "Æ"                 , "æ"   , 0xE6  , 230  , 'u', -1    , -1    , "6828de6627c06028de7427c06427c07827c0")\
            X(330 , 1, AcuteA             , "Á"                 , "á"   , 0xE1  , 225  , 'u', -1    , -1    , "9828de7c28de")\
            X(332 , 1, BreveA             , "Ă"                 , "ă"   , 0x0103, 259  , 'u', -1    , -1    , "4c1adb961adb")\
            X(334 , 1, CircumflexA        , "Â"                 , "â"   , 0xE2  , 226  , 'u', -1    , -1    , "4c2bdc962bdc")\
            X(336 , 1, GraveA             , "À"                 , "à"   , 0xE0  , 224  , 'u', -1    , -1    , "462bdc442bdc5828de5628de7e28dc9428dc")\
            X(338 , 1, OgonekA            , "Ą"                 , "ą"   , 0x0105, 261  , 'u', -1    , -1    , "8a28deb41051")\
            X(340 , 1, RingA              , "Å"                 , "å"   , 0xE5  , 229  , 'u', -1    , -1    , "5c1add5a1add681add661add6c1add601add701add741add641add")\
            X(342 , 1, TildeA             , "Ã"                 , "ã"   , 0xE3  , 227  , 'u', -1    , -1    , "aa29de")\
            X(344 , 1, UmlautA            , "Ä"                 , "ä"   , 0xE4  , 228  , 'u', -1    , -1    , "5c28de5a28de6c28de9e1bdd7628bf7028de8c28de8e28de8228dea41bdd9028dc8828de8428de")\
            X(346 , 1, AcuteC             , "Ć"                 , "ć"   , 0x0107, 263  , 'u', -1    , -1    , "8628de9228de8028de")\
            X(348 , 1, CaronC             , "Č"                 , "č"   , 0x010D, 269  , 'u', -1    , -1    , "8627ba9227ba8027bab433bc")\
            X(350 , 1, CedillaC           , "Ç"                 , "ç"   , 0xE7  , 231  , 'u', -1    , -1    , "4e27ba4a27ba461bdd541adb441bdd5028de5227c06e2bbf9a34dcb630bf")\
            X(352 , 1, DotAboveC          , "Ċ"                 , "ċ"   , 0x010B, 267  , 'u', -1    , -1    , "4029c03e29c0")\
            X(354 , 1, AcuteE             , "É"                 , "é"   , 0xE9  , 233  , 'u', -1    , -1    , "4235bf4635bf4435bf9827ba7e27de9427de7c27ba")\
            X(356 , 1, GraveE             , "È"                 , "è"   , 0xE8  , 232  , 'u', -1    , -1    , "4628c04428c0581aba561aba7e1aba941aba")\
            X(358 , 1, DotAboveE          , "Ė"                 , "ė"   , 0x0117, 279  , 'u', -1    , -1    , "b428de")\
            X(360 , 1, OgonekE            , "Ę"                 , "ę"   , 0x0119, 281  , 'u', -1    , -1    , "b435bd")\
            X(362 , 1, UmlautE            , "Ë"                 , "ë"   , 0xEB  , 235  , 'u', -1    , -1    , "5427ba")\
            X(364 , 1, CrossedD           , "Đ"                 , "đ"   , 0x0111, 273  , 'u', -1    , -1    , "861bdd921bdd801bdd")\
            X(366 , 1, BreveG             , "Ğ"                 , "ğ"   , 0x011F, 287  , 'u', -1    , -1    , "9a1adbb612ba")\
            X(368 , 1, DotAboveG          , "Ġ"                 , "ġ"   , 0x0121, 289  , 'u', -1    , -1    , "401adb3e1adb")\
            X(370 , 1, CrossedH           , "Ħ"                 , "ħ"   , 0x0127, 295  , 'u', -1    , -1    , "401bdd3e1bdd")\
            X(372 , 1, AcuteI             , "Í"                 , "í"   , 0xED  , 237  , 'u', -1    , -1    , "9856e27c29307c56e2")\
            X(374 , 1, CircumflexI        , "Î"                 , "î"   , 0xEE  , 238  , 'u', -1    , -1    , "4c1bdd961bdd")\
            X(376 , 1, GraveI             , "Ì"                 , "ì"   , 0xEC  , 236  , 'u', -1    , -1    , "580ddd560ddd")\
            X(378 , 1, OgonekI            , "Į"                 , "į"   , 0x012F, 303  , 'u', -1    , -1    , "b41add")\
            X(380 , 1, CrossedL           , "Ł"                 , "ł"   , 0x0142, 322  , 'u', -1    , -1    , "8c2bbf822bbf8a27ba")\
            X(382 , 1, CaronN             , "Ň"                 , "ň"   , 0x0148, 328  , 'u', -1    , -1    , "9e2bdca42bdc")\
            X(384 , 1, TildeN             , "Ñ"                 , "ñ"   , 0xF1  , 241  , 'u', -1    , -1    , "6227c05027c06e27c06a27c0")\
            X(386 , 1, AcuteO             , "Ó"                 , "ó"   , 0xF3  , 243  , 'u', -1    , -1    , "8a2bdc980dbb7c0dbbaa1bba")\
            X(388 , 1, CircumflexO        , "Ô"                 , "ô"   , 0xF4  , 244  , 'u', -1    , -1    , "9e27baa427ba")\
            X(390 , 1, DoubleAcuteO       , "Ő"                 , "ő"   , 0x0151, 337  , 'u', -1    , -1    , "981adb7c1adb")\
            X(392 , 1, GraveO             , "Ò"                 , "ò"   , 0xF2  , 242  , 'u', -1    , -1    , "5827c05627c0")\
            X(394 , 1, HornO              , "Ơ"                 , "ơ"   , 0x01A1, 417  , 'u', -1    , -1    , "9c1bdd")\
            X(396 , 1, SlashedO           , "Ø"                 , "ø"   , 0xF8  , 248  , 'u', -1    , -1    , "6827c06628de6027c07428de6428de")\
            X(398 , 1, TildeO             , "Õ"                 , "õ"   , 0xF5  , 245  , 'u', -1    , -1    , "761bdb")\
            X(400 , 1, UmlautO            , "Ö"                 , "ö"   , 0xF6  , 246  , 'u', -1    , -1    , "5c27c05a27c06c27c07627ba7027c0780cbb8c27c08e27c08227c09a33bf9027de8827c08427c0b62ddc")\
            X(402 , 1, AcuteS             , "Ś"                 , "ś"   , 0x015B, 347  , 'u', -1    , -1    , "8a1bdd")\
            X(404 , 1, CaronS             , "Š"                 , "š"   , 0x0161, 353  , 'u', -1    , -1    , "861adb921adb801adbb42146")\
            X(406 , 1, CedillaS           , "Ş"                 , "ş"   , 0x015F, 351  , 'u', -1    , -1    , "9a27ba9627bab628de")\
            X(408 , 1, CommaS             , "Ș"                 , "ș"   , 0x0219, 537  , 'u', -1    , -1    , "4c27ba")\
            X(410 , 1, CedillaT           , "Ţ"                 , "ţ"   , 0x0163, 355  , 'u', -1    , -1    , "9628de")\
            X(412 , 1, CommaT             , "Ț"                 , "ț"   , 0x021B, 539  , 'u', -1    , -1    , "4c28de")\
            X(414 , 1, AcuteU             , "Ú"                 , "ú"   , 0xFA  , 250  , 'u', -1    , -1    , "9e1adba01adba21adba41adb981bdd7c1bdd")\
            X(416 , 1, DoubleAcuteU       , "Ű"                 , "ű"   , 0x0171, 369  , 'u', -1    , -1    , "982bdc7c2bdc")\
            X(418 , 1, GraveU             , "Ù"                 , "ù"   , 0xF9  , 249  , 'u', -1    , -1    , "4656e24456e2582bbf562bbfaa28c0a828c0a628c0ae28c0b028c0ac28c0")\
            X(420 , 1, HornU              , "Ư"                 , "ư"   , 0x01B0, 432  , 'u', -1    , -1    , "9c1adb")\
            X(422 , 1, MacronU            , "Ū"                 , "ū"   , 0x016B, 363  , 'u', -1    , -1    , "b42d58")\
            X(424 , 1, OgonekU            , "Ų"                 , "ų"   , 0x0173, 371  , 'u', -1    , -1    , "b427c0")\
            X(426 , 1, RingU              , "Ů"                 , "ů"   , 0x016F, 367  , 'u', -1    , -1    , "a027baa227ba")\
            X(428 , 1, UmlautU            , "Ü"                 , "ü"   , 0xFC  , 252  , 'u', -1    , -1    , "761ac08c1aba8e1aba821aba980cbf9a1bdd901aba881aba841aba7c0cbdb622dd")\
            X(430 , 1, CaronZ             , "Ž"                 , "ž"   , 0x017E, 382  , 'u', -1    , -1    , "7a0dbb862bdc922bdc802bdcb41157")\
            X(432 , 1, DotAboveZ          , "Ż"                 , "ż"   , 0x017C, 380  , 'u', -1    , -1    , "4056e23e2bdc8a1adb")\
            X(434 , 0, Sleep              , "Sleep"             , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(436 , 0, AppStart1          , "AppStart1"         , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(438 , 0, AppStart2          , "AppStart2"         , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(440 , 0, AppNewWindow       , "AppNewWindow"      , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(442 , 0, AppOpenWindow      , "AppOpenWindow"     , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(444 , 0, AppHelp            , "AppHelp"           , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(446 , 0, AppSave            , "AppSave"           , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(448 , 0, AppFind            , "AppFind"           , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(450 , 0, AppPrint           , "AppPrint"          , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(452 , 0, AppClose           , "AppClose"          , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(454 , 1, AppCut             , "AppCut"            , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(456 , 0, AppCopy            , "AppCopy"           , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(458 , 1, AppPaste           , "AppPaste"          , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(460 , 1, AppUndo            , "AppUndo"           , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(462 , 1, AppRedo            , "AppRedo"           , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(464 , 0, AppSpeechMode      , "AppSpeechMode"     , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(466 , 0, AppSpeechCorrection, "AppSpeechCorrect"  , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(468 , 0, AppSpellCheck      , "AppSpellCheck"     , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(470 , 0, Calculator         , "Calculator"        , ""    , 0     , 0    , 'u', -1    , -1    , "0121b72921b70921b72721b71321b70d21b71f21b71d21b70721b72521b70521b70b21b71121b71721b72121b71921b70f21b72b21b71b21b72321b71521b77b21b72d21b72f21b70321b73121b73321b73521b73b21b73d21b73921b73721b79d21b74121b73f21b74d21b74321b74f21b74b21b74721b75521b74521b75d21b75b21b76921b76321b76721b74921b77321b76d21b75121b79f21b77721b7a121b75321b75921b75721b76f21b76121b76b21b77121b77521b75f21b76521b7a321b77921b78d21b78f21b78321b7a521b78721b78b21b79921b79b21b79121b77f21b79521b78921b78521b77d21b79321b78121b79721b7ab21b7a921b7a721b7af21b7b121b7ad21b7b521b7b321b7b721b7b921b7bb21b7")\
            X(472 , 0, Mail               , "Mail"              , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(474 , 0, MailSend           , "MailSend"          , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(476 , 0, MailForward        , "MailForward"       , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(478 , 0, MailReply          , "MailReply"         , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(480 , 0, MediaBassBoost     , "MediaBassBoost"    , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(482 , 0, MediaBassDown      , "MediaBassDown"     , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(484 , 0, MediaBassUp        , "MediaBassUp"       , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(486 , 0, MediaChanDown      , "MediaChanDown"     , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(488 , 0, MediaChanUp        , "MediaChanUp"       , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(490 , 0, MediaTrebleDown    , "MediaTrebleDown"   , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(492 , 0, MediaTrebleUp      , "MediaTrebleUp"     , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(494 , 0, MediaVolMute       , "MediaVolMute"      , ""    , 0     , 57440, 'u', -1    , -1    , "0120ad2920ad0920ad2720ad1320ad0d20ad1f20ad1d20ad0720ad2520ad0520ad0b20ad1120ad1720ad2120ad1920ad0f20ad2b20ad1b20ad2320ad1520ad7b20ad2d20ad2f20ad0320ad3120ad3320ad3520ad3b20ad3d20ad3920ad3720ad9d20ad4120ad3f20ad4d20ad4320ad4f20ad4b20ad4720ad5520ad4520ad5d20ad5b20ad6920ad6320ad6720ad4920ad7320ad6d20ad5120ad9f20ad7720ada120ad5320ad5920ad5720ad6f20ad6120ad6b20ad7120ad7520ad5f20ad6520ada320ad7920ad8d20ad8f20ad8320ada520ad8720ad8b20ad9920ad9b20ad9120ad7f20ad9520ad8920ad8520ad7d20ad9320ad8120ad9720adab20ada920ada720adaf20adb120adad20adb520adb320adb720adb920adbb20ad")\
            X(496 , 0, MediaVolDown       , "MediaVolDown"      , ""    , 0     , 57438, 'u', -1    , -1    , "012eae292eae092eae272eae132eae0d2eae1f2eae1d2eae072eae252eae052eae0b2eae112eae172eae212eae192eae0f2eae2b2eae1b2eae232eae152eae7b2eae2d2eae2f2eae032eae312eae332eae352eae3b2eae3d2eae392eae372eae9d2eae412eae3f2eae4d2eae432eae4f2eae4b2eae472eae552eae452eae5d2eae5b2eae692eae632eae672eae492eae732eae6d2eae512eae9f2eae772eaea12eae532eae592eae572eae6f2eae612eae6b2eae712eae752eae5f2eae652eaea32eae792eae8d2eae8f2eae832eaea52eae872eae8b2eae992eae9b2eae912eae7f2eae952eae892eae852eae7d2eae932eae812eae972eaeab2eaea92eaea72eaeaf2eaeb12eaead2eaeb52eaeb32eaeb72eaeb92eaebb2eae")\
            X(498 , 0, MediaVolUp         , "MediaVolUp"        , ""    , 0     , 57439, 'u', -1    , -1    , "0130af2930af0930af2730af1330af0d30af1f30af1d30af0730af2530af0530af0b30af1130af1730af2130af1930af0f30af2b30af1b30af2330af1530af7b30af2d30af2f30af0330af3130af3330af3530af3b30af3d30af3930af3730af9d30af4130af3f30af4d30af4330af4f30af4b30af4730af5530af4530af5d30af5b30af6930af6330af6730af4930af7330af6d30af5130af9f30af7730afa130af5330af5930af5730af6f30af6130af6b30af7130af7530af5f30af6530afa330af7930af8d30af8f30af8330afa530af8730af8b30af9930af9b30af9130af7f30af9530af8930af8530af7d30af9330af8130af9730afab30afa930afa730afaf30afb130afad30afb530afb330afb730afb930afbb30af")\
            X(500 , 0, MediaNext          , "MediaNext"         , ""    , 0     , 57435, 'u', -1    , -1    , "0119b02919b00919b02719b01319b00d19b01f19b01d19b00719b02519b00519b00b19b01119b01719b02119b01919b00f19b02b19b01b19b02319b01519b07b19b02d19b02f19b00319b03119b03319b03519b03b19b03d19b03919b03719b09d19b04119b03f19b04d19b04319b04f19b04b19b04719b05519b04519b05d19b05b19b06919b06319b06719b04919b07319b06d19b05119b09f19b07719b0a119b05319b05919b05719b06f19b06119b06b19b07119b07519b05f19b06519b0a319b07919b08d19b08f19b08319b0a519b08719b08b19b09919b09b19b09119b07f19b09519b08919b08519b07d19b09319b08119b09719b0ab19b0a919b0a719b0af19b0b119b0ad19b0b519b0b319b0b719b0b919b0bb19b0")\
            X(502 , 0, MediaPrev          , "MediaPrev"         , ""    , 0     , 57436, 'u', -1    , -1    , "0110b12910b10910b12710b11310b10d10b11f10b11d10b10710b12510b10510b10b10b11110b11710b12110b11910b10f10b12b10b11b10b12310b11510b17b10b12d10b12f10b10310b13110b13310b13510b13b10b13d10b13910b13710b19d10b14110b13f10b14d10b14310b14f10b14b10b14710b15510b14510b15d10b15b10b16910b16310b16710b14910b17310b16d10b15110b19f10b17710b1a110b15310b15910b15710b16f10b16110b16b10b17110b17510b15f10b16510b1a310b17910b18d10b18f10b18310b1a510b18710b18b10b19910b19b10b19110b17f10b19510b18910b18510b17d10b19310b18110b19710b1ab10b1a910b1a710b1af10b1b110b1ad10b1b510b1b310b1b710b1b910b1bb10b1")\
            X(504 , 0, MediaStop          , "MediaStop"         , ""    , 0     , 57432, 'u', -1    , -1    , "0124b22924b20924b22724b21324b20d24b21f24b21d24b20724b22524b20524b20b24b21124b21724b22124b21924b20f24b22b24b21b24b22324b21524b27b24b22d24b22f24b20324b23124b23324b23524b23b24b23d24b23924b23724b29d24b24124b23f24b24d24b24324b24f24b24b24b24724b25524b24524b25d24b25b24b26924b26324b26724b24924b27324b26d24b25124b29f24b27724b2a124b25324b25924b25724b26f24b26124b26b24b27124b27524b25f24b26524b2a324b27924b28d24b28f24b28324b2a524b28724b28b24b29924b29b24b29124b27f24b29524b28924b28524b27d24b29324b28124b29724b2ab24b2a924b2a724b2af24b2b124b2ad24b2b524b2b324b2b724b2b924b2bb24b2")\
            X(506 , 0, MediaPause         , "MediaPause"        , ""    , 0     , 57429, 'u', -1    , -1    , "")\
            X(508 , 0, MediaPlayPause     , "MediaPlayPause"    , ""    , 0     , 57430, 'u', -1    , -1    , "0122b32922b30922b32722b31322b30d22b31f22b31d22b30722b32522b30522b30b22b31122b31722b32122b31922b30f22b32b22b31b22b32322b31522b37b22b32d22b32f22b30322b33122b33322b33522b33b22b33d22b33922b33722b39d22b34122b33f22b34d22b34322b34f22b34b22b34722b35522b34522b35d22b35b22b36922b36322b36722b34922b37322b36d22b35122b39f22b37722b3a122b35322b35922b35722b36f22b36122b36b22b37122b37522b35f22b36522b3a322b37922b38d22b38f22b38322b3a522b38722b38b22b39922b39b22b39122b37f22b39522b38922b38522b37d22b39322b38122b39722b3ab22b3a922b3a722b3af22b3b122b3ad22b3b522b3b322b3b722b3b922b3bb22b3")\
            X(510 , 0, MediaPlay          , "MediaPlay"         , ""    , 0     , 57428, 'u', -1    , -1    , "")\
            X(512 , 0, MediaSelectMode    , "MediaSelectMode"   , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(514 , 0, MediaReverse       , "MediaReverse"      , ""    , 0     , 57431, 'u', -1    , -1    , "")\
            X(516 , 0, MediaRecord        , "MediaRecord"       , ""    , 0     , 57437, 'u', -1    , -1    , "")\
            X(518 , 0, MediaFastForward   , "MediaFastForward"  , ""    , 0     , 57433, 'u', -1    , -1    , "")\
            X(520 , 0, MediaRewind        , "MediaRewind"       , ""    , 0     , 57434, 'u', -1    , -1    , "")\
            X(522 , 0, MicAirToggle       , "MicAirToggle"      , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(524 , 0, MicMute            , "MicMute"           , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(526 , 0, MicVolUp           , "MicVolUp"          , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(528 , 0, MicVolDown         , "MicVolDown"        , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(530 , 0, BrowserBackward    , "BrowserBackward"   , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(532 , 0, BrowserForward     , "BrowserForward"    , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(534 , 0, BrowserRefresh     , "BrowserRefresh"    , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(536 , 0, BrowserStop        , "BrowserStop"       , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(538 , 0, BrowserSearch      , "BrowserSearch"     , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(540 , 0, BrowserFavorites   , "BrowserFavorites"  , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(542 , 0, BrowserHome        , "BrowserHome"       , ""    , 0     , 0    , 'u', -1    , -1    , "0132ac2932ac0932ac2732ac1332ac0d32ac1f32ac1d32ac0732ac2532ac0532ac0b32ac1132ac1732ac2132ac1932ac0f32ac2b32ac1b32ac2332ac1532ac7b32ac2d32ac2f32ac0332ac3132ac3332ac3532ac3b32ac3d32ac3932ac3732ac9d32ac4132ac3f32ac4d32ac4332ac4f32ac4b32ac4732ac5532ac4532ac5d32ac5b32ac6932ac6332ac6732ac4932ac7332ac6d32ac5132ac9f32ac7732aca132ac5332ac5932ac5732ac6f32ac6132ac6b32ac7132ac7532ac5f32ac6532aca332ac7932ac8d32ac8f32ac8332aca532ac8732ac8b32ac9932ac9b32ac9132ac7f32ac9532ac8932ac8532ac7d32ac9332ac8132ac9732acab32aca932aca732acaf32acb132acad32acb532acb332acb732acb932acbb32ac")\
            X(544 , 0, lastKey            , "lastKey"           , ""    , 0     , 0    , 0  , -1    , -1    , "")

        #define key_list2 \
            /*Id   Index Vkey  Scan  KLID     CS          Mask  CS KLID VK SC  I  Name                   GenericName      KKP base,suffix,ascii,w\ctrl*/\
            X(0,      0,    0,    0, 0x00000,           0, 0x0000'00000'00'FF, 1, undef                , "undef"           , 0    , 'u', -1    , -1    )\
            X(1,      0, 0xFF, 0xFF, 0x00409,           0, 0x0100'00000'FF'FF, 0, config               , "config"          , 0    , 'u', -1    , -1    )\
            X(402, 0xFF, 0xFF, 0xFF, 0x00000, ExtendedKey, 0x0100'FFFFF'FF'FF, 0, lastKey              , "lastKey"         , 0    , 0  , -1    , -1    )
            // Max 12 bits for KeyId.
            static constexpr auto idbits = 12;

        #define X(KeyId, Input, Name, Generic, Literal, Uc, KKPdef, KKPsuffix, KKPascii, wCtl, PhysicalCode) \
            static constexpr auto Name = KeyId;
            key_list
        #undef X

        static constexpr auto key_map = []
        {
            auto m = std::array<si16, 65536>{};
            auto fill = [&](si32 KeyId, qiew codes)
            {
                while (codes)
                {
                    auto key_hash = utf::to_int_from_hex_str(codes.pop_front(6)) >> 8;
                    if (m[key_hash]) log("key duplicates"); // It won't compile if collide.
                    m[key_hash] = (si16)KeyId;
                }
            };
            #define X(KeyId, Input, Name, Generic, Literal, Uc, KKPdef, KKPsuffix, KKPascii, wCtl, PhysicalCode) \
                fill(KeyId, qiew{ PhysicalCode });
                key_list
            #undef X
            return m;
        }();
        static const auto vk_map = []
        {
            struct keyrec
            {
                si16 code;
                si16 scan;
                si32 klid;
            };
            auto m = std::array<std::vector<keyrec>, 256>{};
            auto fill = [&](si16 KeyId, qiew codes)
            {
                while (codes)
                {
                    auto key_hash = utf::to_int_from_hex_str(codes.pop_front(6));
                    auto vkey = key_hash & 0xFF;
                    auto scan = (si16)((key_hash >> 8) & 0x1FF);
                    auto klid = (si32)(key_hash >> 17);
                    m[vkey].push_back({ .code = KeyId, .scan = scan, .klid = klid });
                }
            };
            #define X(KeyId, Input, Name, Generic, Literal, Uc, KKPdef, KKPsuffix, KKPascii, wCtl, PhysicalCode) \
                fill(KeyId, qiew{ PhysicalCode });
                key_list
            #undef X
            return m;
        }();

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
                static auto data = std::array<key, input::key::lastKey>{};
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
            static void set(si32 id, bool doinput, view specific_keyname, view generic_keyname, si32 KKPdef, si32 KKPsuffix, si32 KKPascii, si32 wCtl, qiew PhysicalCode)
            {
                auto vk = 0;
                auto sc = 0;
                auto klid = 0;
                if (PhysicalCode)
                {
                    auto v = utf::to_int_from_hex_str(qiew{ PhysicalCode }.substr(0, 6));
                    vk = v & 0xFF;
                    sc = (v >> 8) & 0xFF;
                    klid = v >> 17;
                }
                data(id) = { .name      = specific_keyname,
                             .generic   = generic_keyname,
                             .vkey      = vk,
                             .scan      = sc,
                             .klid      = klid,
                             .edit      = doinput,
                             .KKPDef    = KKPdef,
                             .KKPSuffix = KKPsuffix,
                             .KKPAscii  = KKPascii,
                             .KKPCtl    = wCtl };
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
            #define X(KeyId, Input, Name, Generic, Literal, Uc, KKPdef, KKPsuffix, KKPascii, wCtl, PhysicalCode) \
                { map{ 0,0,0,0 }, KeyId },
                key_list
            #undef X
        };

        static const auto _init0 = []
        {
            //todo move it to std::array
            #define X(KeyId, Input, Name, Generic, Literal, Uc, KKPdef, KKPsuffix, KKPascii, wCtl, PhysicalCode) \
                map::set(KeyId, Input, #Name, Generic, KKPdef, KKPsuffix, KKPascii, wCtl, PhysicalCode);
                key_list
            #undef X
            return true;
        }();
        static const auto kkpmap = std::unordered_map<si32, si32>
        {
            #define X(KeyId, Input, Name, Generic, Literal, Uc, KKPdef, KKPsuffix, KKPascii, wCtl, PhysicalCode) \
                { KKPdef | (KKPsuffix << 16), KeyId },
                key_list
            #undef X
        };
        static const auto specific_names = utf::unordered_map<text, si32>
        {
            #define X(KeyId, Input, Name, Generic, Literal, Uc, KKPdef, KKPsuffix, KKPascii, wCtl, PhysicalCode) \
                { utf::to_lower(#Name), KeyId },
                key_list
            #undef X
        };
        static const auto generic_names = utf::unordered_map<text, si32>
        {
            #define X(KeyId, Input, Name, Generic, Literal, Uc, KKPdef, KKPsuffix, KKPascii, wCtl, PhysicalCode) \
                { utf::to_lower(Generic), KeyId & -2 },
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
        #undef key_list2
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
            static constexpr byte deadkey  = 5;
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
            if (tooltip.visible)
            {
                tooltip.hide();
            }
            keybd::vkevent = indexer.get_kbchord_hint(k.vkchord);
            keybd::scevent = indexer.get_kbchord_hint(k.scchord);
            keybd::chevent = indexer.get_kbchord_hint(k.chchord);
            keybd::update(k);
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
                    static constexpr auto min = std::min({ key::KeyHome, key::KeyEnd, key::KeyLeftArrow, key::KeyRightArrow, key::KeyUpArrow, key::KeyDownArrow });
                    static constexpr auto max = std::max({ key::KeyHome, key::KeyEnd, key::KeyLeftArrow, key::KeyRightArrow, key::KeyUpArrow, key::KeyDownArrow });
                    if (v >= min && v <= max)
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
                si32 vcode;
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
                    auto vk_valid = k.keycode > input::key::invalid;
                    auto sc_valid = k.scancod > 0;
                    if (!keyout || k.keystat != input::key::released)
                    {
                        auto broken_state = faux;
                        keyout = k.keystat == input::key::released;
                        //log(" erasing %%", k.keystat == input::key::released ? "key::released" : k.keystat == input::key::pressed ? "key::pressed" : "key::repeated");
                        std::erase_if(pushed, [&](auto& rec)
                        {
                            auto& [keyid, val] = rec;
                            //log("\tcheck keyid=%%", input::key::map::data(keyid).name);
                            auto is_released = test_key_released(val.vcode); // Check if it is still pressed.
                            auto same = k.keycode == keyid
                                     && k.scancod == val.scode
                                     && k.virtcod == val.vcode;
                            if (!is_released)
                            {
                                if (same && k.keystat == input::key::pressed) // The same key was pressed twice without releasing.
                                {
                                    broken_state = true;
                                }
                                else if (!same) // Exclude repeated key.
                                {
                                    if (keyid <= input::key::invalid) vk_valid = faux;
                                    if (val.scode == 0) sc_valid = faux;
                                    if (vk_valid) push_keyid(true, k.vkchord, keyid);
                                    if (sc_valid) push_scode(true, k.scchord, val.scode);
                                }
                            }
                            //else if (is_released) log("\tkeyid=%% released", input::key::map::data(keyid).name);
                            return is_released;
                        });
                        if (broken_state)
                        {
                            reset(k);
                            return;
                        }
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
                        key.vcode = k.virtcod; // Store the virtual code to check later that it is still pressed.
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
        void fix_numpad(auto& sc, bool numlock, bool extflag)
        {
            if (!numlock && !extflag
                && ((sc >= 0x47 && sc <= 0x49)   // 7 8 9 -> Home Up    PgUp
                 || (sc >= 0x4B && sc <= 0x4D)   // 4 5 6 -> Left Clear Right
                 || (sc >= 0x4F && sc <= 0x53))) // 1 2 3 -> End  Down  PgDn   // 0->Ins .->Del
            {
                sc = 0x80 | (sc & 0xF); // ala 0x4* -> 0x8* if no numlock
            }
        }
        void fix_Numpad_Yen_Slash(auto& vk, auto& sc, bool numlock, bool extflag)
        {
                 if (sc == 0x53/*NumpadDecimal*/ && numlock         ) vk = vkey::numpadD;
            else if (sc == 0x7D/*IntlYen*/                          ) vk = vkey::intl_yen;
            else if (numlock && !extflag && sc >= 0x47 && sc <= 0x53)
            {
                switch (vk)
                {
                    case vkey::home:   vk = vkey::numpad7; break; // sc 0x47
                    case vkey::up:     vk = vkey::numpad8; break; // sc 0x48
                    case vkey::pgup:   vk = vkey::numpad9; break; // sc 0x49 (Page Up)
                    case vkey::left:   vk = vkey::numpad4; break; // sc 0x4B
                    case vkey::clear:  vk = vkey::numpad5; break; // sc 0x4C (Num 5)
                    case vkey::right:  vk = vkey::numpad6; break; // sc 0x4D
                    case vkey::end:    vk = vkey::numpad1; break; // sc 0x4F
                    case vkey::down:   vk = vkey::numpad2; break; // sc 0x50
                    case vkey::pgdn:   vk = vkey::numpad3; break; // sc 0x51 (Page Down)
                    case vkey::insert: vk = vkey::numpad0; break; // sc 0x52
                    case vkey::del:    vk = vkey::numpadD; break; // sc 0x53 (Dot/Comma)
                }
            }
            else if (extflag && sc == 0x35 && vk != vkey::divide) // Extended Numpad Slash (scan=0xE035)
            {
                vk = vkey::divide;
            }
            else fix_numpad(sc, numlock, extflag);
        }
        auto xlat(si32 vk, si32 sc, bool extflag, si32 keymod, si32 xlayout, si32 layout_fallback, si32& layout_hint)
        {
            auto keyid = key::undef;
            auto numlock = keymod & input::hids::NumLock;
            if (xlayout)
            {
                auto klid = input::key::is_layout_supported(xlayout) ? xlayout
                                                   : layout_fallback ? layout_fallback
                                                                     : input::key::latin_klids[0];
                fix_numpad(sc, numlock, extflag);
                auto hash = input::key::key_hash(klid, sc, extflag);
                keyid = (si32)input::key::key_map[hash];
            }
            else
            {
                fix_Numpad_Yen_Slash(vk, sc, numlock, extflag);
                sc |= extflag << 8;
                auto& code_scan_klid_list = input::key::vk_map[vk];
                auto new_layout_hint = layout_hint;
                for (auto [code, scan, klid] : code_scan_klid_list)
                {
                    if (scan == sc)
                    {
                        if (new_layout_hint == layout_hint && klid != layout_hint) // Best effort.
                        {
                            keyid = code;
                            new_layout_hint = klid;
                            if (layout_hint == 0) break;
                        }
                        else if (klid == layout_hint) // Exact match.
                        {
                            keyid = code;
                            new_layout_hint = klid;
                            break;
                        }
                    }
                }
                std::swap(new_layout_hint, layout_hint);
            }
            return keyid;
        }
        auto xlat(si32 vk, si32 sc, bool extflag, si32 keymod, si32 xlayout, si32 layout_fallback)
        {
            auto unused_hint = 0;
            return xlat(vk, sc, extflag, keymod, xlayout, layout_fallback, unused_hint);
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