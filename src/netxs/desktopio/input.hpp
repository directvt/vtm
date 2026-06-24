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
        static constexpr auto ctrl     = 0x11; // VK_CONTROL
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

        //todo drop
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
        //todo drop
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
        //todo drop
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
            /*00 500*/ 0x00000409, // US
            /*01 360*/ 0x00000411, // Japanese
            /*02 360*/ 0x00000415, // Polish (Programmers)
            /*03 360*/ 0x00020405, // Czech Programmers
            /*04 360*/ 0x00020418, // Romanian (Programmers)
            /*05 360*/ 0x00010426, // Latvian (QWERTY)
            /*06 360*/ 0x00020426, // Latvian (Standard)
            /*07 360*/ 0x00050408, // Greek Latin
            /*08 360*/ 0x00004009, // English (India)
            /*09 360*/ 0x0000046a, // Yoruba
            /*0A 360*/ 0x00000468, // Hausa
            /*0B 360*/ 0x00000470, // Igbo
            /*0C 360*/ 0x00000481, // Maori
            /*0D 360*/ 0x0000046c, // Sesotho sa Leboa
            /*0E 353*/ 0x00020409, // United States-International
            /*0F 353*/ 0x00010427, // Lithuanian
            /*10 353*/ 0x00000475, // Hawaiian
            /*11 348*/ 0x00000412, // Korean
            /*12 346*/ 0x00050409, // US English Table for IBM Arabic 238_L
            /*13 344*/ 0x0001045d, // Inuktitut - Naqittaut
            /*14 344*/ 0x0000085d, // Inuktitut - Latin
            /*15 343*/ 0x00000452, // United Kingdom Extended
            /*16 336*/ 0x00000809, // United Kingdom
            /*17 336*/ 0x00011809, // Scottish Gaelic
            /*18 336*/ 0x00001809, // Irish
            /*19 332*/ 0x0000042a, // Vietnamese
            /*1A 332*/ 0x0000043a, // Maltese 47-Key
            /*1B 332*/ 0x0001043a, // Maltese 48-Key
            /*1C 318*/ 0x00010418, // Romanian (Standard)
            /*1D 313*/ 0x00001009, // Canadian French
            /*1E 311*/ 0x00000416, // Portuguese (Brazil ABNT)
            /*1F 311*/ 0x00000c0c, // Canadian French (Legacy)
            /*20 308*/ 0x0000041c, // Albanian
            /*21 305*/ 0x00011009, // Canadian Multilingual Standard
            /*22 299*/ 0x0001083b, // Finnish with Sami
            /*23 299*/ 0x00030408, // Greek (220) Latin
            /*24 299*/ 0x00040408, // Greek (319) Latin
            /*25 299*/ 0x0000043b, // Norwegian with Sami
            /*26 299*/ 0x0000041d, // Swedish
            /*27 299*/ 0x0000046f, // Greenlandic
            /*28 299*/ 0x00000474, // Guarani
            /*29 297*/ 0x00010405, // Czech (QWERTY)
            /*2A 297*/ 0x0001040a, // Spanish Variation
            /*2B 297*/ 0x00000425, // Estonian
            /*2C 297*/ 0x0001041b, // Slovak (QWERTY)
            /*2D 294*/ 0x00000410, // Italian
            /*2E 294*/ 0x00010410, // Italian (142)
            /*2F 294*/ 0x00000816, // Portuguese
            /*30 292*/ 0x00000406, // Danish
            /*31 292*/ 0x0000040b, // Finnish
            /*32 292*/ 0x00000413, // Dutch
            /*33 292*/ 0x00000414, // Norwegian
            /*34 292*/ 0x0000040a, // Spanish
            /*35 292*/ 0x0000080a, // Latin American
            /*36 292*/ 0x00000438, // Faeroese
            /*37 291*/ 0x00000405, // Czech
            /*38 290*/ 0x0000040f, // Icelandic
            /*39 289*/ 0x00010415, // Polish (214)
            /*3A 289*/ 0x0000041b, // Slovak
            /*3B 289*/ 0x0001042e, // Sorbian Extended
            /*3C 289*/ 0x0002042e, // Sorbian Standard
            /*3D 289*/ 0x0000042e, // Sorbian Standard (Legacy)
            /*3E 289*/ 0x0000081a, // Serbian (Latin)
            /*3F 285*/ 0x0000040e, // Hungarian
            /*40 282*/ 0x00000407, // German
            /*41 282*/ 0x00020407, // German Extended (E1)
            /*42 282*/ 0x00030407, // German Extended (E2)
            /*43 282*/ 0x00010407, // German (IBM)
            /*44 282*/ 0x0000046e, // Luxembourgish
            /*45 282*/ 0x00000807, // Swiss German
            /*46 282*/ 0x0000041f, // Turkish Q
            /*47 277*/ 0x0001040e, // Hungarian 101-key
            /*48 275*/ 0x00000424, // Slovenian
            /*49 275*/ 0x00060409, // Colemak
            /*4A 273*/ 0x00000418, // Romanian (Legacy)
            /*4B 267*/ 0x0000085f, // Central Atlas Tamazight
            /*4C 267*/ 0x00000488, // Wolof
            /*4D 266*/ 0x0001040c, // French (Standard, AZERTY)
            /*4E 260*/ 0x0000040c, // French (Legacy, AZERTY)
            /*4F 257*/ 0x00000813, // Belgian (Period)
            /*50 257*/ 0x0001080c, // Belgian (Comma)
            /*51 256*/ 0x00020427, // Lithuanian Standard
            /*52 188*/ 0x00010409, // United States-Dvorak
            /*53 172*/ 0x0001041f, // Turkish F
            /*54 165*/ 0x00040409, // United States-Dvorak for right hand
            /*55 160*/ 0x00030409, // United States-Dvorak for left hand
            /*56 155*/ 0x0002040c, // French (Standard, BÉPO)
        });
        static constexpr auto klid_hash(si32 klid)
        {
            auto iter = std::ranges::find(latin_klids, klid);
            if (iter != latin_klids.end())
            {
                return (si32)std::distance(latin_klids.begin(), iter);
            }
            else
            return 0;
        }
        static constexpr auto supported_klids = [] // This won't compile if there are collisions.
        {
            struct layout
            {
                si32 klid;
                si32 index;
            };
            auto index = 0;
            auto klids = std::array<layout, latin_klids.size()>{};
            for (auto klid : input::key::latin_klids) // Check hash collisions.
            {
                klids[index] = { (si32)klid, index };
                index++;
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
            auto hash = (ui16)((scan & 0xFF) | ((si32)extflag << 8) | (input::key::klid_hash(klid) << 9));
            return hash;
        }

        // Notes:
        //  IsoLevel5Shift: 5th-level of kb layout (mathematical signs, Greek letters). Physical keyboards don't have this key; in Linux, it's usually remapped to Caps Lock or the right Ctrl key.
        //  Hyper:          Users specifically create Hyper (for example, by remapping Caps Lock) to bind hotkeys, which are guaranteed to not interact with anything.
        //  Shift:          We normalize the left and right shifts: right shift = VK_SHIFT+ExtFlag.
        #define key_list \
          /* ID  Input Ex_Vk  Name                 Generic              Literal  Uc  KKPdef KKPsuffix KKPascii wCtl  PhysicalCode */\
            X(0   , 1, 0    , undef              , "undef"             , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(1   , 0, 0    , invalid            , "invalid"           , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(2   , 0, 0x011, LeftCtrl           , "Ctrl"              , ""    , 0     , 57442, 'u', -1    , -1    , "001d11061d11101d110e1d11141d11161d11021d110a1d110c1d11181d11041d11081d111a1d11121d11201d111e1d111c1d11221d11241d11281d11261d112a1d11301d112e1d112c1d11341d11361d11321d11381d113a1d113e1d113c1d11401d11421d11441d11461d11481d114e1d11501d114a1d114c1d11521d11561d11581d11541d115a1d115c1d115e1d11601d11641d116c1d11621d116a1d11661d11681d116e1d11701d11721d117c1d11741d11761d11781d117a1d117e1d11801d11861d11821d11841d11881d118a1d118c1d118e1d11921d11901d11941d11961d11981d119a1d119c1d11a01d119e1d11a21d11a41d11a61d11a81d11aa1d11ac1d11")\
            X( 3  , 0, 0x111, RightCtrl          , "Ctrl"              , ""    , 0     , 57448, 'u', -1    , -1    , "011d11071d11111d110f1d11151d11171d11031d110b1d110d1d11191d11051d11091d111b1d11131d11211d111f1d111d1d11251d11291d11271d112b1d11311d112f1d112d1d11351d11371d11331d11391d113b1d113f1d113d1d11411d11451d11471d11491d114f1d11511d114b1d114d1d11531d11571d11591d11551d115b1d115d1d115f1d11611d11651d116d1d11631d116b1d11671d11691d116f1d11711d11731d117d1d11751d11771d11791d117b1d117f1d11811d11871d11831d11851d11891d118b1d118d1d118f1d11931d11911d11951d11971d11991d119b1d119d1d11a11d119f1d11a31d11a51d11a71d11a91d11ab1d11ad1d11")\
            X(4   , 0, 0x012, LeftAlt            , "Alt"               , ""    , 0     , 57443, 'u', -1    , -1    , "0038120638121038120e38121438121638120238120a38120c38121838120438120838121a38121238122038121e38121c38122238122438122838122638122a38123038122e38122c38123438123638123238123838123a38123e38123c38124038124238124438124638124838124e38125038124a38124c38125238125638125838125438125a38125c38125e38126038126438126c38126238126a38126638126838126e38127038127238127c38127438127638127838127a38127e38128038128638128238128438128838128a38128c38128e38129238129038129438129638129838129a38129c3812a038129e3812a23812a43812a63812a83812aa3812ac3812")\
            X( 5  , 0, 0x112, RightAlt           , "Alt"               , ""    , 0     , 57449, 'u', -1    , -1    , "0138120738121138120f38121538121738120338120b38120d38121938120538120938121b38121338122138121f38121d38122538122938122738122b38123138122f38122d38123538123738123338123938123b38123f38123d38124138124338124538124738124938124f38125138124b38124d38125338125738125938125538125b38125d38125f38126138126538126d38126338126b38126738126938126f38127138127338127d38127538127738127938127b38127f38128138128738128338128538128938128b38128d38128f38129338129138129538129738129938129b38129d3812a138129f3812a33812a53812a73812a93812ab3812ad3812")\
            X(6   , 0, 0x010, LeftShift          , "Shift"             , ""    , 0     , 57441, 'u', -1    , -1    , "002a10062a10102a100e2a10142a10162a10022a100a2a100c2a10182a10042a10082a101a2a10122a10202a101e2a101c2a10222a10242a10282a10262a102a2a10302a102e2a102c2a10342a10362a10322a10382a103a2a103e2a103c2a10402a10422a10442a10462a10482a104e2a10502a104a2a104c2a10522a10562a10582a10542a105a2a105c2a105e2a10602a10642a106c2a10622a106a2a10662a10682a106e2a10702a10722a107c2a10742a10762a10782a107a2a107e2a10802a10862a10822a10842a10882a108a2a108c2a108e2a10922a10902a10942a10962a10982a109a2a109c2a10a02a109e2a10a22a10a42a10a62a10a82a10aa2a10ac2a10")\
            X( 7  , 0, 0x110, RightShift         , "Shift"             , ""    , 0     , 57447, 'u', -1    , -1    , "0036100636101036100e36101436101636100236100a36100c36101836100436100836101a36101236102036101e36101c36102236102436102836102636102a36103036102e36102c36103436103636103236103836103a36103e36103c36104036104236104436104636104836104e36105036104a36104c36105236105636105836105436105a36105c36105e36106036106436106c36106236106a36106636106836106e36107036107236107c36107436107636107836107a36107e36108036108636108236108436108836108a36108c36108e36109236109036109436109636109836109a36109c3610a036109e3610a23610a43610a63610a83610aa3610ac3610")\
            X(8   , 0, 0x15B, LeftSuper          , "Super"             , ""    , 0     , 57444, 'u', -1    , -1    , "015b5b075b5b115b5b0f5b5b155b5b175b5b035b5b0b5b5b0d5b5b195b5b055b5b095b5b1b5b5b135b5b215b5b1f5b5b1d5b5b235b5b255b5b295b5b275b5b2b5b5b315b5b2f5b5b2d5b5b355b5b375b5b335b5b395b5b3b5b5b3f5b5b3d5b5b415b5b435b5b455b5b475b5b4f5b5b515b5b4b5b5b4d5b5b535b5b575b5b595b5b555b5b5b5b5b5d5b5b5f5b5b615b5b655b5b6d5b5b635b5b6b5b5b675b5b695b5b6f5b5b715b5b735b5b7d5b5b755b5b775b5b795b5b7b5b5b7f5b5b815b5b875b5b835b5b855b5b895b5b8b5b5b8d5b5b935b5b975b5b995b5b9b5b5b9d5b5ba15b5b9f5b5ba35b5ba55b5ba75b5ba95b5bab5b5bad5b5b")\
            X( 9  , 0, 0x15C, RightSuper         , "Super"             , ""    , 0     , 57450, 'u', -1    , -1    , "015c5c075c5c115c5c0f5c5c155c5c175c5c035c5c0b5c5c0d5c5c195c5c055c5c095c5c1b5c5c135c5c215c5c1f5c5c1d5c5c235c5c255c5c295c5c275c5c2b5c5c315c5c2f5c5c2d5c5c355c5c375c5c335c5c395c5c3b5c5c3f5c5c3d5c5c415c5c435c5c455c5c475c5c495c5c4f5c5c515c5c4b5c5c4d5c5c535c5c575c5c595c5c555c5c5b5c5c5d5c5c5f5c5c615c5c655c5c6d5c5c635c5c6b5c5c675c5c695c5c6f5c5c715c5c735c5c7d5c5c755c5c775c5c795c5c7b5c5c7f5c5c815c5c875c5c835c5c855c5c895c5c8b5c5c8d5c5c8f5c5c935c5c915c5c955c5c975c5c995c5c9b5c5c9d5c5ca15c5c9f5c5ca35c5ca55c5ca75c5ca95c5cab5c5cad5c5c")\
            X(10  , 0, 0    , LeftHyper          , "Hyper"             , ""    , 0     , 57445, 'u', -1    , -1    , "")\
            X( 11 , 0, 0    , RightHyper         , "Hyper"             , ""    , 0     , 57451, 'u', -1    , -1    , "")\
            X(12  , 0, 0x190, NumLock            , "NumLock"           , ""    , 0     , 57360, 'u', -1    , -1    , "0145900745901145900f45901545901745900345900b45900d45901945900545900945901b45901345902145901f45901d45902345902545902945902745902b45903145902f45902d45903545903745903345903945903b45903f45903d45904145904345904545904745904945904f45905145904b45904d45905345905745905945905545905b45905d45905f45906145906545906d45906345906b45906745906945906f45907145907345907d45907545907745907945907b45907f45908145908745908345908545908945908b45908d45908f45909345909145909545909745909945909b45909d4590a145909f4590a34590a54590a74590a94590ab4590ad4590")\
            X(14  , 0, 0x014, CapsLock           , "CapsLock"          , ""    , 0     , 57358, 'u', -1    , -1    , "003a14063a14103a140e3a14143a14163a14023a140a3a140c3a14183a14043a14083a141a3a14123a14203a141e3a141c3a14223a14243a14283a14263a142a3a14303a142e3a142c3a14343a14363a14323a14383a143a3a143e3a143c3a14403a14423a14443a14463a14483a144e3a14503a144a3a144c3a14523a14563a14583a14543a145a3a145c3a145e3a14603a14643a146c3a14623a146a3a14663a14683a146e3a14703a14723a147c3a14743a14763a14783a147a3a147e3a14803a14863a14823a14843a14883a148a3a148c3a148e3a14923a14903a14943a14963a14983a149a3a149c3a14a03a149e3a14a23a14a43a14a63a14a83a14aa3a14ac3a14")\
            X(16  , 0, 0x091, ScrollLock         , "ScrollLock"        , ""    , 0     , 57359, 'u', -1    , -1    , "0046910646911046910e46911446911646910246910a46910c46911846910446910846911a46911246912046911e46911c46912246912446912846912646912a46913046912e46912c46913446913646913246913846913a46913e46913c46914046914246914446914646914846914e46915046914a46914c46915246915646915846915446915a46915c46915e46916046916446916c46916246916a46916646916846916e46917046917246917c46917446917646917846917a46917e46918046918646918246918446918846918a46918c46918e46919246919046919446919646919846919a46919c4691a046919e4691a24691a44691a64691a84691aa4691ac4691")\
            X(18  , 0, 0x05E, AltGr              , "AltGr"             , ""    , 0     , 57453, 'u', -1    , -1    , "")\
            X(20  , 0, 0    , IsoLevel5Shift     , "IsoLevel5Shift"    , ""    , 0     , 57454, 'u', -1    , -1    , "431ddf")\
            X(22  , 0, 0    , Kana               , "Kana"              , ""    , 0     , 0    , 'u', -1    , -1    , "0270f2")\
            X(24  , 0, 0    , Henkan             , "Henkan"            , ""    , 0     , 0    , 'u', -1    , -1    , "02791c")\
            X(26  , 0, 0    , Muhenkan           , "Muhenkan"          , ""    , 0     , 0    , 'u', -1    , -1    , "027b1d")\
            X(28  , 0, 0x019, Hanja              , "Hanja"             , ""    , 0     , 0    , 'u', -1    , -1    , "22f119231d19")\
            X(30  , 0, 0    , Hanguel            , "Hanguel"           , ""    , 0     , 0    , 'u', -1    , -1    , "22f215233815")\
            X(32  , 0, 0x15D, Apps               , "Apps"              , ""    , 0     , 57363, 'u', -1    , -1    , "015d5d075d5d115d5d0f5d5d155d5d175d5d035d5d0b5d5d0d5d5d195d5d055d5d095d5d1b5d5d135d5d215d5d1f5d5d1d5d5d235d5d255d5d295d5d275d5d2b5d5d315d5d2f5d5d2d5d5d355d5d375d5d335d5d395d5d3b5d5d3f5d5d3d5d5d415d5d435d5d455d5d475d5d4f5d5d515d5d4b5d5d4d5d5d535d5d575d5d595d5d555d5d5b5d5d5d5d5d5f5d5d615d5d655d5d6d5d5d635d5d6b5d5d675d5d695d5d6f5d5d715d5d735d5d7d5d5d755d5d775d5d795d5d7b5d5d7f5d5d815d5d875d5d835d5d855d5d895d5d8b5d5d8d5d5d935d5d975d5d995d5d9b5d5d9d5d5da15d5d9f5d5da35d5da55d5da75d5da95d5dab5d5dad5d5d")\
            X(34  , 0, 0x029, Select             , "Select"            , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(36  , 0, 0    , Fn                 , "Fn"                , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(38  , 0, 0x070, F1                 , "F1"                , ""    , 0     , 11   , '~', -1    , -1    , "003b70063b70103b700e3b70143b70163b70023b700a3b700c3b70183b70043b70083b701a3b70123b70203b701e3b701c3b70223b70243b70283b70263b702a3b70303b702e3b702c3b70343b70363b70323b70383b703a3b703e3b703c3b70403b70423b70443b70463b70483b704e3b70503b704a3b704c3b70523b70563b70583b70543b705a3b705c3b705e3b70603b70643b706c3b70623b706a3b70663b70683b706e3b70703b70723b707c3b70743b70763b70783b707a3b707e3b70803b70863b70823b70843b70883b708a3b708c3b708e3b70923b70903b70943b70963b70983b709a3b709c3b70a03b709e3b70a23b70a43b70a63b70a83b70aa3b70ac3b70")\
            X(40  , 0, 0x071, F2                 , "F2"                , ""    , 0     , 12   , '~', -1    , -1    , "003c71063c71103c710e3c71143c71163c71023c710a3c710c3c71183c71043c71083c711a3c71123c71203c711e3c711c3c71223c71243c71283c71263c712a3c71303c712e3c712c3c71343c71363c71323c71383c713a3c713e3c713c3c71403c71423c71443c71463c71483c714e3c71503c714a3c714c3c71523c71563c71583c71543c715a3c715c3c715e3c71603c71643c716c3c71623c716a3c71663c71683c716e3c71703c71723c717c3c71743c71763c71783c717a3c717e3c71803c71863c71823c71843c71883c718a3c718c3c718e3c71923c71903c71943c71963c71983c719a3c719c3c71a03c719e3c71a23c71a43c71a63c71a83c71aa3c71ac3c71")\
            X(42  , 0, 0x072, F3                 , "F3"                , ""    , 0     , 13   , '~', -1    , -1    , "003d72063d72103d720e3d72143d72163d72023d720a3d720c3d72183d72043d72083d721a3d72123d72203d721e3d721c3d72223d72243d72283d72263d722a3d72303d722e3d722c3d72343d72363d72323d72383d723a3d723e3d723c3d72403d72423d72443d72463d72483d724e3d72503d724a3d724c3d72523d72563d72583d72543d725a3d725c3d725e3d72603d72643d726c3d72623d726a3d72663d72683d726e3d72703d72723d727c3d72743d72763d72783d727a3d727e3d72803d72863d72823d72843d72883d728a3d728c3d728e3d72923d72903d72943d72963d72983d729a3d729c3d72a03d729e3d72a23d72a43d72a63d72a83d72aa3d72ac3d72")\
            X(44  , 0, 0x073, F4                 , "F4"                , ""    , 0     , 14   , '~', -1    , -1    , "003e73063e73103e730e3e73143e73163e73023e730a3e730c3e73183e73043e73083e731a3e73123e73203e731e3e731c3e73223e73243e73283e73263e732a3e73303e732e3e732c3e73343e73363e73323e73383e733a3e733e3e733c3e73403e73423e73443e73463e73483e734e3e73503e734a3e734c3e73523e73563e73583e73543e735a3e735c3e735e3e73603e73643e736c3e73623e736a3e73663e73683e736e3e73703e73723e737c3e73743e73763e73783e737a3e737e3e73803e73863e73823e73843e73883e738a3e738c3e738e3e73923e73903e73943e73963e73983e739a3e739c3e73a03e739e3e73a23e73a43e73a63e73a83e73aa3e73ac3e73")\
            X(46  , 0, 0x074, F5                 , "F5"                , ""    , 0     , 15   , '~', -1    , -1    , "003f74063f74103f740e3f74143f74163f74023f740a3f740c3f74183f74043f74083f741a3f74123f74203f741e3f741c3f74223f74243f74283f74263f742a3f74303f742e3f742c3f74343f74363f74323f74383f743a3f743e3f743c3f74403f74423f74443f74463f74483f744e3f74503f744a3f744c3f74523f74563f74583f74543f745a3f745c3f745e3f74603f74643f746c3f74623f746a3f74663f74683f746e3f74703f74723f747c3f74743f74763f74783f747a3f747e3f74803f74863f74823f74843f74883f748a3f748c3f748e3f74923f74903f74943f74963f74983f749a3f749c3f74a03f749e3f74a23f74a43f74a63f74a83f74aa3f74ac3f74")\
            X(48  , 0, 0x075, F6                 , "F6"                , ""    , 0     , 17   , '~', -1    , -1    , "0040750640751040750e40751440751640750240750a40750c40751840750440750840751a40751240752040751e40751c40752240752440752840752640752a40753040752e40752c40753440753640753240753840753a40753e40753c40754040754240754440754640754840754e40755040754a40754c40755240755640755840755440755a40755c40755e40756040756440756c40756240756a40756640756840756e40757040757240757c40757440757640757840757a40757e40758040758640758240758440758840758a40758c40758e40759240759040759440759640759840759a40759c4075a040759e4075a24075a44075a64075a84075aa4075ac4075")\
            X(50  , 0, 0x076, F7                 , "F7"                , ""    , 0     , 18   , '~', -1    , -1    , "0041760641761041760e41761441761641760241760a41760c41761841760441760841761a41761241762041761e41761c41762241762441762841762641762a41763041762e41762c41763441763641763241763841763a41763e41763c41764041764241764441764641764841764e41765041764a41764c41765241765641765841765441765a41765c41765e41766041766441766c41766241766a41766641766841766e41767041767241767c41767441767641767841767a41767e41768041768641768241768441768841768a41768c41768e41769241769041769441769641769841769a41769c4176a041769e4176a24176a44176a64176a84176aa4176ac4176")\
            X(52  , 0, 0x077, F8                 , "F8"                , ""    , 0     , 19   , '~', -1    , -1    , "0042770642771042770e42771442771642770242770a42770c42771842770442770842771a42771242772042771e42771c42772242772442772842772642772a42773042772e42772c42773442773642773242773842773a42773e42773c42774042774242774442774642774842774e42775042774a42774c42775242775642775842775442775a42775c42775e42776042776442776c42776242776a42776642776842776e42777042777242777c42777442777642777842777a42777e42778042778642778242778442778842778a42778c42778e42779242779042779442779642779842779a42779c4277a042779e4277a24277a44277a64277a84277aa4277ac4277")\
            X(54  , 0, 0x078, F9                 , "F9"                , ""    , 0     , 20   , '~', -1    , -1    , "0043780643781043780e43781443781643780243780a43780c43781843780443780843781a43781243782043781e43781c43782243782443782843782643782a43783043782e43782c43783443783643783243783843783a43783e43783c43784043784243784443784643784843784e43785043784a43784c43785243785643785843785443785a43785c43785e43786043786443786c43786243786a43786643786843786e43787043787243787c43787443787643787843787a43787e43788043788643788243788443788843788a43788c43788e43789243789043789443789643789843789a43789c4378a043789e4378a24378a44378a64378a84378aa4378ac4378")\
            X(56  , 0, 0x079, F10                , "F10"               , ""    , 0     , 21   , '~', -1    , -1    , "0044790644791044790e44791444791644790244790a44790c44791844790444790844791a44791244792044791e44791c44792244792444792844792644792a44793044792e44792c44793444793644793244793844793a44793e44793c44794044794244794444794644794844794e44795044794a44794c44795244795644795844795444795a44795c44795e44796044796444796c44796244796a44796644796844796e44797044797244797c44797444797644797844797a44797e44798044798644798244798444798844798a44798c44798e44799244799044799444799644799844799a44799c4479a044799e4479a24479a44479a64479a84479aa4479ac4479")\
            X(58  , 0, 0x07A, F11                , "F11"               , ""    , 0     , 23   , '~', -1    , -1    , "00577a06577a10577a0e577a14577a16577a02577a0a577a0c577a18577a04577a08577a1a577a12577a20577a1e577a1c577a22577a24577a28577a26577a2a577a30577a2e577a2c577a34577a36577a32577a38577a3a577a3e577a3c577a40577a42577a44577a46577a48577a4e577a50577a4a577a4c577a52577a56577a58577a54577a5a577a5c577a5e577a60577a64577a6c577a62577a6a577a66577a68577a6e577a70577a72577a7c577a74577a76577a78577a7a577a7e577a80577a86577a82577a84577a88577a8a577a8c577a8e577a92577a90577a94577a96577a98577a9a577a9c577aa0577a9e577aa2577aa4577aa6577aa8577aaa577aac577a")\
            X(60  , 0, 0x07B, F12                , "F12"               , ""    , 0     , 24   , '~', -1    , -1    , "00587b06587b10587b0e587b14587b16587b02587b0a587b0c587b18587b04587b08587b1a587b12587b20587b1e587b1c587b22587b24587b28587b26587b2a587b30587b2e587b2c587b34587b36587b32587b38587b3a587b3e587b3c587b40587b42587b44587b46587b48587b4e587b50587b4a587b4c587b52587b56587b58587b54587b5a587b5c587b5e587b60587b64587b6c587b62587b6a587b66587b68587b6e587b70587b72587b7c587b74587b76587b78587b7a587b7e587b80587b86587b82587b84587b88587b8a587b8c587b8e587b92587b90587b94587b96587b98587b9a587b9c587ba0587b9e587ba2587ba4587ba6587ba8587baa587bac587b")\
            X(62  , 0, 0x07C, F13                , "F13"               , ""    , 0     , 57376, 'u', -1    , -1    , "00647c06647c10647c0e647c14647c16647c02647c0a647c0c647c18647c04647c08647c1a647c12647c20647c1e647c1c647c22647c24647c28647c26647c2a647c30647c2e647c2c647c34647c36647c32647c38647c3a647c3e647c3c647c40647c42647c44647c46647c48647c4e647c50647c4a647c4c647c52647c56647c58647c54647c5a647c5c647c5e647c60647c64647c6c647c62647c6a647c66647c68647c6e647c70647c72647c7c647c74647c76647c78647c7a647c7e647c80647c86647c82647c84647c88647c8a647c8c647c8e647c92647c90647c94647c96647c98647c9a647c9c647ca0647c9e647ca2647ca4647ca6647ca8647caa647cac647c")\
            X(64  , 0, 0x07D, F14                , "F14"               , ""    , 0     , 57377, 'u', -1    , -1    , "00657d06657d10657d0e657d14657d16657d02657d0a657d0c657d18657d04657d08657d1a657d12657d20657d1e657d1c657d22657d24657d28657d26657d2a657d30657d2e657d2c657d34657d36657d32657d38657d3a657d3e657d3c657d40657d42657d44657d46657d48657d4e657d50657d4a657d4c657d52657d56657d58657d54657d5a657d5c657d5e657d60657d64657d6c657d62657d6a657d66657d68657d6e657d70657d72657d7c657d74657d76657d78657d7a657d7e657d80657d86657d82657d84657d88657d8a657d8c657d8e657d92657d90657d94657d96657d98657d9a657d9c657da0657d9e657da2657da4657da6657da8657daa657dac657d")\
            X(66  , 0, 0x07E, F15                , "F15"               , ""    , 0     , 57378, 'u', -1    , -1    , "00667e06667e10667e0e667e14667e16667e02667e0a667e0c667e18667e04667e08667e1a667e12667e20667e1e667e1c667e22667e24667e28667e26667e2a667e30667e2e667e2c667e34667e36667e32667e38667e3a667e3e667e3c667e40667e42667e44667e46667e48667e4e667e50667e4a667e4c667e52667e56667e58667e54667e5a667e5c667e5e667e60667e64667e6c667e62667e6a667e66667e68667e6e667e70667e72667e7c667e74667e76667e78667e7a667e7e667e80667e86667e82667e84667e88667e8a667e8c667e8e667e92667e90667e94667e96667e98667e9a667e9c667ea0667e9e667ea2667ea4667ea6667ea8667eaa667eac667e")\
            X(68  , 0, 0x07F, F16                , "F16"               , ""    , 0     , 57379, 'u', -1    , -1    , "00677f06677f10677f0e677f14677f16677f02677f0a677f0c677f18677f04677f08677f1a677f12677f20677f1e677f1c677f22677f24677f28677f26677f2a677f30677f2e677f2c677f34677f36677f32677f38677f3a677f3e677f3c677f40677f42677f44677f46677f48677f4e677f50677f4a677f4c677f52677f56677f58677f54677f5a677f5c677f5e677f60677f64677f6c677f62677f6a677f66677f68677f6e677f70677f72677f7c677f74677f76677f78677f7a677f7e677f80677f86677f82677f84677f88677f8a677f8c677f8e677f92677f90677f94677f96677f98677f9a677f9c677fa0677f9e677fa2677fa4677fa6677fa8677faa677fac677f")\
            X(70  , 0, 0x080, F17                , "F17"               , ""    , 0     , 57380, 'u', -1    , -1    , "0068800668801068800e68801468801668800268800a68800c68801868800468800868801a68801268802068801e68801c68802268802468802868802668802a68803068802e68802c68803468803668803268803868803a68803e68803c68804068804268804468804668804868804e68805068804a68804c68805268805668805868805468805a68805c68805e68806068806468806c68806268806a68806668806868806e68807068807268807c68807468807668807868807a68807e68808068808668808268808468808868808a68808c68808e68809268809068809468809668809868809a68809c6880a068809e6880a26880a46880a66880a86880aa6880ac6880")\
            X(72  , 0, 0x081, F18                , "F18"               , ""    , 0     , 57381, 'u', -1    , -1    , "0069810669811069810e69811469811669810269810a69810c69811869810469810869811a69811269812069811e69811c69812269812469812869812669812a69813069812e69812c69813469813669813269813869813a69813e69813c69814069814269814469814669814869814e69815069814a69814c69815269815669815869815469815a69815c69815e69816069816469816c69816269816a69816669816869816e69817069817269817c69817469817669817869817a69817e69818069818669818269818469818869818a69818c69818e69819269819069819469819669819869819a69819c6981a069819e6981a26981a46981a66981a86981aa6981ac6981")\
            X(74  , 0, 0x082, F19                , "F19"               , ""    , 0     , 57382, 'u', -1    , -1    , "006a82066a82106a820e6a82146a82166a82026a820a6a820c6a82186a82046a82086a821a6a82126a82206a821e6a821c6a82226a82246a82286a82266a822a6a82306a822e6a822c6a82346a82366a82326a82386a823a6a823e6a823c6a82406a82426a82446a82466a82486a824e6a82506a824a6a824c6a82526a82566a82586a82546a825a6a825c6a825e6a82606a82646a826c6a82626a826a6a82666a82686a826e6a82706a82726a827c6a82746a82766a82786a827a6a827e6a82806a82866a82826a82846a82886a828a6a828c6a828e6a82926a82906a82946a82966a82986a829a6a829c6a82a06a829e6a82a26a82a46a82a66a82a86a82aa6a82ac6a82")\
            X(76  , 0, 0x083, F20                , "F20"               , ""    , 0     , 57383, 'u', -1    , -1    , "006b83066b83106b830e6b83146b83166b83026b830a6b830c6b83186b83046b83086b831a6b83126b83206b831e6b831c6b83226b83246b83286b83266b832a6b83306b832e6b832c6b83346b83366b83326b83386b833a6b833e6b833c6b83406b83426b83446b83466b83486b834e6b83506b834a6b834c6b83526b83566b83586b83546b835a6b835c6b835e6b83606b83646b836c6b83626b836a6b83666b83686b836e6b83706b83726b837c6b83746b83766b83786b837a6b837e6b83806b83866b83826b83846b83886b838a6b838c6b838e6b83926b83906b83946b83966b83986b839a6b839c6b83a06b839e6b83a26b83a46b83a66b83a86b83aa6b83ac6b83")\
            X(78  , 0, 0x084, F21                , "F21"               , ""    , 0     , 57384, 'u', -1    , -1    , "006c84066c84106c840e6c84146c84166c84026c840a6c840c6c84186c84046c84086c841a6c84126c84206c841e6c841c6c84226c84246c84286c84266c842a6c84306c842e6c842c6c84346c84366c84326c84386c843a6c843e6c843c6c84406c84426c84446c84466c84486c844e6c84506c844a6c844c6c84526c84566c84586c84546c845a6c845c6c845e6c84606c84646c846c6c84626c846a6c84666c84686c846e6c84706c84726c847c6c84746c84766c84786c847a6c847e6c84806c84866c84826c84846c84886c848a6c848c6c848e6c84926c84906c84946c84966c84986c849a6c849c6c84a06c849e6c84a26c84a46c84a66c84a86c84aa6c84ac6c84")\
            X(80  , 0, 0x085, F22                , "F22"               , ""    , 0     , 57385, 'u', -1    , -1    , "006d85066d85106d850e6d85146d85166d85026d850a6d850c6d85186d85046d85086d851a6d85126d85206d851e6d851c6d85226d85246d85286d85266d852a6d85306d852e6d852c6d85346d85366d85326d85386d853a6d853e6d853c6d85406d85426d85446d85466d85486d854e6d85506d854a6d854c6d85526d85566d85586d85546d855a6d855c6d855e6d85606d85646d856c6d85626d856a6d85666d85686d856e6d85706d85726d857c6d85746d85766d85786d857a6d857e6d85806d85866d85826d85846d85886d858a6d858c6d858e6d85926d85906d85946d85966d85986d859a6d859c6d85a06d859e6d85a26d85a46d85a66d85a86d85aa6d85ac6d85")\
            X(82  , 0, 0x086, F23                , "F23"               , ""    , 0     , 57386, 'u', -1    , -1    , "006e86066e86106e860e6e86146e86166e86026e860a6e860c6e86186e86046e86086e861a6e86126e86206e861e6e861c6e86226e86246e86286e86266e862a6e86306e862e6e862c6e86346e86366e86326e86386e863a6e863e6e863c6e86406e86426e86446e86466e86486e864e6e86506e864a6e864c6e86526e86566e86586e86546e865a6e865c6e865e6e86606e86646e866c6e86626e866a6e86666e86686e866e6e86706e86726e867c6e86746e86766e86786e867a6e867e6e86806e86866e86826e86846e86886e868a6e868c6e868e6e86926e86906e86946e86966e86986e869a6e869c6e86a06e869e6e86a26e86a46e86a66e86a86e86aa6e86ac6e86")\
            X(84  , 0, 0x087, F24                , "F24"               , ""    , 0     , 57387, 'u', -1    , -1    , "006f87066f87106f870e6f87146f87166f87026f870a6f870c6f87186f87046f87086f871a6f87126f87206f871e6f871c6f87226f87246f87286f87266f872a6f87306f872e6f872c6f87346f87366f87326f87386f873a6f873e6f873c6f87406f87426f87446f87466f87486f874e6f87506f874a6f874c6f87526f87566f87586f87546f875a6f875c6f875e6f87606f87646f876c6f87626f876a6f87666f87686f876e6f87706f87726f877c6f87746f87766f87786f877a6f877e6f87806f87866f87826f87846f87886f878a6f878c6f878e6f87926f87906f87946f87966f87986f879a6f879c6f87a06f879e6f87a26f87a46f87a66f87a86f87aa6f87ac6f87")\
            X(86  , 0, 0    , F25                , "F25"               , ""    , 0     , 57388, 'u', -1    , -1    , "")\
            X(88  , 0, 0    , F26                , "F26"               , ""    , 0     , 57389, 'u', -1    , -1    , "")\
            X(90  , 0, 0    , F27                , "F27"               , ""    , 0     , 57390, 'u', -1    , -1    , "")\
            X(92  , 0, 0    , F28                , "F28"               , ""    , 0     , 57391, 'u', -1    , -1    , "")\
            X(94  , 0, 0    , F29                , "F29"               , ""    , 0     , 57392, 'u', -1    , -1    , "")\
            X(96  , 0, 0    , F30                , "F30"               , ""    , 0     , 57393, 'u', -1    , -1    , "")\
            X(98  , 0, 0    , F31                , "F31"               , ""    , 0     , 57394, 'u', -1    , -1    , "")\
            X(100 , 0, 0    , F32                , "F32"               , ""    , 0     , 57395, 'u', -1    , -1    , "")\
            X(102 , 0, 0    , F33                , "F33"               , ""    , 0     , 57396, 'u', -1    , -1    , "")\
            X(104 , 0, 0    , F34                , "F34"               , ""    , 0     , 57397, 'u', -1    , -1    , "")\
            X(106 , 0, 0    , F35                , "F35"               , ""    , 0     , 57398, 'u', -1    , -1    , "")\
            X(108 , 0, 0    , PrintScreen        , "PrintScreen"       , ""    , 0     , 57361, 'u', -1    , -1    , "01372c07372c11372c0f372c15372c17372c03372c0b372c0d372c19372c05372c09372c1b372c13372c21372c1f372c1d372c23372c25372c29372c27372c2b372c31372c2f372c2d372c35372c37372c33372c39372c3b372c3f372c3d372c41372c43372c45372c47372c49372c4f372c51372c4b372c4d372c53372c57372c59372c55372c5b372c5d372c5f372c61372c65372c6d372c63372c6b372c67372c69372c6f372c71372c73372c7d372c75372c77372c79372c7b372c7f372c81372c87372c83372c85372c89372c8b372c8d372c8f372c93372c91372c95372c97372c99372c9b372c9d372ca1372c9f372ca3372ca5372ca7372ca9372cab372cad372c")\
            X(110 , 0, 0x013, Pause              , "Pause"             , ""    , 0     , 57362, 'u', '\x1a', '\x03', "0045900645901045900e45901445901645900245900a45900c45901845900445900845901a45901245902045901e45901c45902245902445902845902645902a45903045902e45902c45903445903645903245903845903a45903e45903c45904045904245904445904645904845904e45905045904a45904c45905245905645905845905445905a45905c45905e45906045906445906c45906245906a45906645906845906e45907045907245907c45907445907645907845907a45907e45908045908645908245908445908845908a45908c45908e45909245909045909445909645909845909a45909c4590a045909e4590a24590a44590a64590a84590aa4590ac4590")\
            X(112 , 1, 0    , Break              , "Break"             , "\x03", 0x03  , 3    , 'u', '\x03', '\x03', "0146030746031146030f46031546031746030346030b46030d46031946030546030946031b46031346032146031f46031d46032346032546032946032746032b46033146032f46032d46033546033746033346033946033b46033f46033d46034146034346034546034746034946034f46035146034b46034d46035346035746035946035546035b46035d46035f46036146036546036d46036346036b46036746036946036f46037146037346037d46037546037746037946037b46037f46038146038746038346038546038946038b46038d46038f46039346039146039546039746039946039b46039d4603a146039f4603a34603a54603a74603a94603ab4603ad4603")\
            X(114 , 1, 0    , SysReq             , "SysReq"            , ""    , 0     , 0    , 'u', -1    , '\x03', "00542c06542c10542c0e542c14542c16542c02542c0a542c0c542c18542c04542c08542c1a542c12542c20542c1e542c22542c24542c28542c26542c2a542c34542c36542c38542c3c542c40542c44542c46542c48542c4e542c50542c4a542c4c542c52542c56542c58542c6e542c72542c7c542c74542c76542c78542c7a542c7e542c8c542c8e542c92542c90542c94542c96542c98542ca2542ca6542ca8542caa542c")\
            X(116 , 1, 0x01B, Esc                , "Esc"               , "\x1B", 0x1b  , 27   , 'u', '\x1b', '\x1b', "00011b06011b10011b0e011b14011b16011b02011b0a011b0c011b18011b04011b08011b1a011b12011b20011b1e011b1c011b22011b24011b28011b26011b2a011b30011b2e011b2c011b34011b36011b32011b38011b3a011b3e011b3c011b40011b42011b44011b46011b48011b4e011b50011b4a011b4c011b52011b56011b58011b54011b5a011b5c011b5e011b60011b64011b6c011b62011b6a011b66011b68011b6e011b70011b72011b7c011b74011b76011b78011b7a011b7e011b80011b86011b82011b84011b88011b8a011b8c011b8e011b92011b90011b94011b96011b98011b9a011b9c011ba0011b9e011ba2011ba4011ba6011ba8011baa011bac011b")\
            X(118 , 1, 0x009, Tab                , "Tab"               , "\x09", 0x09  , 9    , 'u', '\x09', '\x09', "000f09060f09100f090e0f09140f09160f09020f090a0f090c0f09180f09040f09080f091a0f09120f09200f091e0f091c0f09220f09240f09280f09260f092a0f09300f092e0f092c0f09340f09360f09320f09380f093a0f093e0f093c0f09400f09420f09440f09460f09480f094e0f09500f094a0f094c0f09520f09560f09580f09540f095a0f095c0f095e0f09600f09640f096c0f09620f096a0f09660f09680f096e0f09700f09720f097c0f09740f09760f09780f097a0f097e0f09800f09860f09820f09840f09880f098a0f098c0f098e0f09920f09900f09940f09960f09980f099a0f099c0f09a00f099e0f09a20f09a40f09a60f09a80f09aa0f09ac0f09")\
            X(120 , 1, 0x008, Backspace          , "Backspace"         , "\x08", 0x08  , 127  , 'u', '\x7f', '\x08', "000e08060e08100e080e0e08140e08160e08020e080a0e080c0e08180e08040e08080e081a0e08120e08200e081e0e081c0e08220e08240e08280e08260e082a0e08300e082e0e082c0e08340e08360e08320e08380e083a0e083e0e083c0e08400e08420e08440e08460e08480e084e0e08500e084a0e084c0e08520e08560e08580e08540e085a0e085c0e085e0e08600e08640e086c0e08620e086a0e08660e08680e086e0e08700e08720e087c0e08740e08760e08780e087a0e087e0e08800e08860e08820e08840e08880e088a0e088c0e088e0e08920e08900e08940e08960e08980e089a0e089c0e08a00e089e0e08a20e08a40e08a60e08a80e08aa0e08ac0e08")\
            X(122 , 1, 0x020, Space              , "Space"             , "\x20", 0x20  , 32   , 'u', '\x20', '\0'  , "0039200639201039200e39201439201639200239200a39200c39201839200439200839201a39201239202039201e39201c39202239202439202839202639202a39203039202e39202c39203439203639203239203839203a39203e39203c39204039204239204439204639204839204e39205039204a39204c39205239205639205839205439205a39205c39205e39206039206439206c39206239206a39206639206839206e39207039207239207c39207439207639207839207a39207e39208039208639208239208439208839208a39208c39208e39209239209039209439209639209839209a39209c3920a039209e3920a23920a43920a63920a83920aa3920ac3920")\
            X(124 , 1, 0x00D, KeyEnter           , "Enter"             , "\x0D", 0x0d  , 13   , 'u', '\x0d', '\x0a', "001c0d061c0d101c0d0e1c0d141c0d161c0d021c0d0a1c0d0c1c0d181c0d041c0d081c0d1a1c0d121c0d201c0d1e1c0d1c1c0d221c0d241c0d281c0d261c0d2a1c0d301c0d2e1c0d2c1c0d341c0d361c0d321c0d381c0d3a1c0d3e1c0d3c1c0d401c0d421c0d441c0d461c0d481c0d4e1c0d501c0d4a1c0d4c1c0d521c0d561c0d581c0d541c0d5a1c0d5c1c0d5e1c0d601c0d641c0d6c1c0d621c0d6a1c0d661c0d681c0d6e1c0d701c0d721c0d7c1c0d741c0d761c0d781c0d7a1c0d7e1c0d801c0d861c0d821c0d841c0d881c0d8a1c0d8c1c0d8e1c0d921c0d901c0d941c0d961c0d981c0d9a1c0d9c1c0da01c0d9e1c0da21c0da41c0da61c0da81c0daa1c0dac1c0d")\
            X( 125, 1, 0x10D, NumpadEnter        , "Enter"             , "\x0D", 0x0d  , 57414, 'u', '\x0d', '\x0a', "011c0d071c0d111c0d0f1c0d151c0d171c0d031c0d0b1c0d0d1c0d191c0d051c0d091c0d1b1c0d131c0d211c0d1f1c0d1d1c0d231c0d251c0d291c0d271c0d2b1c0d311c0d2f1c0d2d1c0d351c0d371c0d331c0d391c0d3b1c0d3f1c0d3d1c0d411c0d431c0d451c0d471c0d491c0d4f1c0d511c0d4b1c0d4d1c0d531c0d571c0d591c0d551c0d5b1c0d5d1c0d5f1c0d611c0d651c0d6d1c0d631c0d6b1c0d671c0d691c0d6f1c0d711c0d731c0d7d1c0d751c0d771c0d791c0d7b1c0d7f1c0d811c0d871c0d831c0d851c0d891c0d8b1c0d8d1c0d8f1c0d931c0d911c0d951c0d971c0d991c0d9b1c0d9d1c0da11c0d9f1c0da31c0da51c0da71c0da91c0dab1c0dad1c0d")\
            X(126 , 1, 0x12D, KeyInsert          , "Insert"            , ""    , 0     , 2    , '~', -1    , -1    , "01522d07522d11522d0f522d15522d17522d03522d0b522d0d522d19522d05522d09522d1b522d13522d21522d1f522d1d522d23522d25522d29522d27522d2b522d31522d2f522d2d522d35522d37522d33522d39522d3b522d3f522d3d522d41522d43522d45522d47522d49522d4f522d51522d4b522d4d522d53522d57522d59522d55522d5b522d5d522d5f522d61522d65522d6d522d63522d6b522d67522d69522d6f522d71522d73522d7d522d75522d77522d79522d7b522d7f522d81522d87522d83522d85522d89522d8b522d8d522d8f522d93522d91522d95522d97522d99522d9b522d9d522da1522d9f522da3522da5522da7522da9522dab522dad522d")\
            X( 127, 1, 0x02D, NumpadInsert       , "Insert"            , ""    , 0     , 57425, 'u', -1    , -1    , "00822d06822d10822d0e822d14822d16822d02822d0a822d0c822d18822d04822d08822d1a822d12822d20822d1e822d1c822d22822d24822d28822d26822d2a822d30822d2e822d2c822d34822d36822d32822d38822d3a822d3e822d3c822d40822d42822d44822d46822d48822d4e822d50822d4a822d4c822d52822d56822d58822d54822d5a822d5c822d5e822d60822d64822d6c822d62822d6a822d66822d68822d6e822d70822d72822d7c822d74822d76822d78822d7a822d7e822d80822d86822d82822d84822d88822d8a822d8c822d8e822d92822d90822d94822d96822d98822d9a822d9c822da0822d9e822da2822da4822da6822da8822daa822dac822d")\
            X(128 , 1, 0x12E, KeyDelete          , "Delete"            , ""    , 0     , 3    , '~', -1    , -1    , "01532e07532e11532e0f532e15532e17532e03532e0b532e0d532e19532e05532e09532e1b532e13532e21532e1f532e1d532e23532e25532e29532e27532e2b532e31532e2f532e2d532e35532e37532e33532e39532e3b532e3f532e3d532e41532e43532e45532e47532e49532e4f532e51532e4b532e4d532e53532e57532e59532e55532e5b532e5d532e5f532e61532e65532e6d532e63532e6b532e67532e69532e6f532e71532e73532e7d532e75532e77532e79532e7b532e7f532e81532e87532e83532e85532e89532e8b532e8d532e8f532e93532e91532e95532e97532e99532e9b532e9d532ea1532e9f532ea3532ea5532ea7532ea9532eab532ead532e")\
            X( 129, 1, 0x02E, NumpadDelete       , "Delete"            , ""    , 0     , 57426, 'u', -1    , -1    , "00832e06832e10832e0e832e14832e16832e02832e0a832e0c832e18832e04832e08832e1a832e12832e20832e1e832e1c832e22832e24832e28832e26832e2a832e30832e2e832e2c832e34832e36832e32832e38832e3a832e3e832e3c832e40832e42832e44832e46832e48832e4e832e50832e4a832e4c832e52832e56832e58832e54832e5a832e5c832e5e832e60832e64832e6c832e62832e6a832e66832e68832e6e832e70832e72832e7c832e74832e76832e78832e7a832e7e832e80832e86832e82832e84832e88832e8a832e8c832e8e832e92832e90832e94832e96832e98832e9a832e9c832ea0832e9e832ea2832ea4832ea6832ea8832eaa832eac832e")\
            X(130 , 1, 0x10C, KeyClear           , "Clear"             , ""    , 0     , 1    , 'E', -1    , -1    , "")\
            X( 131, 1, 0x00C, NumpadClear        , "Clear"             , ""    , 0     , 57427, '~', -1    , -1    , "008c0c068c0c108c0c0e8c0c148c0c168c0c028c0c0a8c0c0c8c0c188c0c048c0c088c0c1a8c0c128c0c208c0c1e8c0c1c8c0c228c0c248c0c288c0c268c0c2a8c0c308c0c2e8c0c2c8c0c348c0c368c0c328c0c388c0c3a8c0c3e8c0c3c8c0c408c0c428c0c448c0c468c0c488c0c4e8c0c508c0c4a8c0c4c8c0c528c0c568c0c588c0c548c0c5a8c0c5c8c0c5e8c0c608c0c648c0c6c8c0c628c0c6a8c0c668c0c688c0c6e8c0c708c0c728c0c7c8c0c748c0c768c0c788c0c7a8c0c7e8c0c808c0c868c0c828c0c848c0c888c0c8a8c0c8c8c0c8e8c0c928c0c908c0c948c0c968c0c988c0c9a8c0c9c8c0ca08c0c9e8c0ca28c0ca48c0ca68c0ca88c0caa8c0cac8c0c")\
            X(132 , 1, 0x121, KeyPageUp          , "PageUp"            , ""    , 0     , 5    , '~', -1    , -1    , "0149210749211149210f49211549211749210349210b49210d49211949210549210949211b49211349212149211f49211d49212349212549212949212749212b49213149212f49212d49213549213749213349213949213b49213f49213d49214149214349214549214749214949214f49215149214b49214d49215349215749215949215549215b49215d49215f49216149216549216d49216349216b49216749216949216f49217149217349217d49217549217749217949217b49217f49218149218749218349218549218949218b49218d49218f49219349219149219549219749219949219b49219d4921a149219f4921a34921a54921a74921a94921ab4921ad4921")\
            X( 133, 1, 0x021, NumpadPageUp       , "PageUp"            , ""    , 0     , 57421, 'u', -1    , -1    , "0089210689211089210e89211489211689210289210a89210c89211889210489210889211a89211289212089211e89211c89212289212489212889212689212a89213089212e89212c89213489213689213289213889213a89213e89213c89214089214289214489214689214889214e89215089214a89214c89215289215689215889215489215a89215c89215e89216089216489216c89216289216a89216689216889216e89217089217289217c89217489217689217889217a89217e89218089218689218289218489218889218a89218c89218e89219289219089219489219689219889219a89219c8921a089219e8921a28921a48921a68921a88921aa8921ac8921")\
            X(134 , 1, 0x122, KeyPageDown        , "PageDown"          , ""    , 0     , 6    , '~', -1    , -1    , "0151220751221151220f51221551221751220351220b51220d51221951220551220951221b51221351222151221f51221d51222351222551222951222751222b51223151222f51222d51223551223751223351223951223b51223f51223d51224151224351224551224751224951224f51225151224b51224d51225351225751225951225551225b51225d51225f51226151226551226d51226351226b51226751226951226f51227151227351227d51227551227751227951227b51227f51228151228751228351228551228951228b51228d51228f51229351229151229551229751229951229b51229d5122a151229f5122a35122a55122a75122a95122ab5122ad5122")\
            X( 135, 1, 0x022, NumpadPageDown     , "PageDown"          , ""    , 0     , 57422, 'u', -1    , -1    , "0081220681221081220e81221481221681220281220a81220c81221881220481220881221a81221281222081221e81221c81222281222481222881222681222a81223081222e81222c81223481223681223281223881223a81223e81223c81224081224281224481224681224881224e81225081224a81224c81225281225681225881225481225a81225c81225e81226081226481226c81226281226a81226681226881226e81227081227281227c81227481227681227881227a81227e81228081228681228281228481228881228a81228c81228e81229281229081229481229681229881229a81229c8122a081229e8122a28122a48122a68122a88122aa8122ac8122")\
            X(136 , 1, 0x124, KeyHome            , "Home"              , ""    , 0     , 7    , '~', -1    , -1    , "0147240747241147240f47241547241747240347240b47240d47241947240547240947241b47241347242147241f47241d47242347242547242947242747242b47243147242f47242d47243547243747243347243947243b47243f47243d47244147244347244547244747244947244f47245147244b47244d47245347245747245947245547245b47245d47245f47246147246547246d47246347246b47246747246947246f47247147247347247d47247547247747247947247b47247f47248147248747248347248547248947248b47248d47248f47249347249147249547249747249947249b47249d4724a147249f4724a34724a54724a74724a94724ab4724ad4724")\
            X( 137, 1, 0x024, NumpadHome         , "Home"              , ""    , 0     , 57423, 'u', -1    , -1    , "0087240687241087240e87241487241687240287240a87240c87241887240487240887241a87241287242087241e87241c87242287242487242887242687242a87243087242e87242c87243487243687243287243887243a87243e87243c87244087244287244487244687244887244e87245087244a87244c87245287245687245887245487245a87245c87245e87246087246487246c87246287246a87246687246887246e87247087247287247c87247487247687247887247a87247e87248087248687248287248487248887248a87248c87248e87249287249087249487249687249887249a87249c8724a087249e8724a28724a48724a68724a88724aa8724ac8724")\
            X(138 , 1, 0x123, KeyEnd             , "End"               , ""    , 0     , 8    , '~', -1    , -1    , "014f23074f23114f230f4f23154f23174f23034f230b4f230d4f23194f23054f23094f231b4f23134f23214f231f4f231d4f23234f23254f23294f23274f232b4f23314f232f4f232d4f23354f23374f23334f23394f233b4f233f4f233d4f23414f23434f23454f23474f23494f234f4f23514f234b4f234d4f23534f23574f23594f23554f235b4f235d4f235f4f23614f23654f236d4f23634f236b4f23674f23694f236f4f23714f23734f237d4f23754f23774f23794f237b4f237f4f23814f23874f23834f23854f23894f238b4f238d4f238f4f23934f23914f23954f23974f23994f239b4f239d4f23a14f239f4f23a34f23a54f23a74f23a94f23ab4f23ad4f23")\
            X( 139, 1, 0x023, NumpadEnd          , "End"               , ""    , 0     , 57424, 'u', -1    , -1    , "008f23068f23108f230e8f23148f23168f23028f230a8f230c8f23188f23048f23088f231a8f23128f23208f231e8f231c8f23228f23248f23288f23268f232a8f23308f232e8f232c8f23348f23368f23328f23388f233a8f233e8f233c8f23408f23428f23448f23468f23488f234e8f23508f234a8f234c8f23528f23568f23588f23548f235a8f235c8f235e8f23608f23648f236c8f23628f236a8f23668f23688f236e8f23708f23728f237c8f23748f23768f23788f237a8f237e8f23808f23868f23828f23848f23888f238a8f238c8f238e8f23928f23908f23948f23968f23988f239a8f239c8f23a08f239e8f23a28f23a48f23a68f23a88f23aa8f23ac8f23")\
            X(140 , 1, 0x125, KeyLeftArrow       , "LeftArrow"         , ""    , 0     , 1    , 'D', -1    , -1    , "014b25074b25114b250f4b25154b25174b25034b250b4b250d4b25194b25054b25094b251b4b25134b25214b251f4b251d4b25234b25254b25294b25274b252b4b25314b252f4b252d4b25354b25374b25334b25394b253b4b253f4b253d4b25414b25434b25454b25474b25494b254f4b25514b254b4b254d4b25534b25574b25594b25554b255b4b255d4b255f4b25614b25654b256d4b25634b256b4b25674b25694b256f4b25714b25734b257d4b25754b25774b25794b257b4b257f4b25814b25874b25834b25854b25894b258b4b258d4b258f4b25934b25914b25954b25974b25994b259b4b259d4b25a14b259f4b25a34b25a54b25a74b25a94b25ab4b25ad4b25")\
            X( 141, 1, 0x025, NumpadLeftArrow    , "LeftArrow"         , ""    , 0     , 57417, 'u', -1    , -1    , "008b25068b25108b250e8b25148b25168b25028b250a8b250c8b25188b25048b25088b251a8b25128b25208b251e8b251c8b25228b25248b25288b25268b252a8b25308b252e8b252c8b25348b25368b25328b25388b253a8b253e8b253c8b25408b25428b25448b25468b25488b254e8b25508b254a8b254c8b25528b25568b25588b25548b255a8b255c8b255e8b25608b25648b256c8b25628b256a8b25668b25688b256e8b25708b25728b257c8b25748b25768b25788b257a8b257e8b25808b25868b25828b25848b25888b258a8b258c8b258e8b25928b25908b25948b25968b25988b259a8b259c8b25a08b259e8b25a28b25a48b25a68b25a88b25aa8b25ac8b25")\
            X(142 , 1, 0x127, KeyRightArrow      , "RightArrow"        , ""    , 0     , 1    , 'C', -1    , -1    , "014d27074d27114d270f4d27154d27174d27034d270b4d270d4d27194d27054d27094d271b4d27134d27214d271f4d271d4d27234d27254d27294d27274d272b4d27314d272f4d272d4d27354d27374d27334d27394d273b4d273f4d273d4d27414d27434d27454d27474d27494d274f4d27514d274b4d274d4d27534d27574d27594d27554d275b4d275d4d275f4d27614d27654d276d4d27634d276b4d27674d27694d276f4d27714d27734d277d4d27754d27774d27794d277b4d277f4d27814d27874d27834d27854d27894d278b4d278d4d278f4d27934d27914d27954d27974d27994d279b4d279d4d27a14d279f4d27a34d27a54d27a74d27a94d27ab4d27ad4d27")\
            X( 143, 1, 0x027, NumpadRightArrow   , "RightArrow"        , ""    , 0     , 57418, 'u', -1    , -1    , "008d27068d27108d270e8d27148d27168d27028d270a8d270c8d27188d27048d27088d271a8d27128d27208d271e8d271c8d27228d27248d27288d27268d272a8d27308d272e8d272c8d27348d27368d27328d27388d273a8d273e8d273c8d27408d27428d27448d27468d27488d274e8d27508d274a8d274c8d27528d27568d27588d27548d275a8d275c8d275e8d27608d27648d276c8d27628d276a8d27668d27688d276e8d27708d27728d277c8d27748d27768d27788d277a8d277e8d27808d27868d27828d27848d27888d278a8d278c8d278e8d27928d27908d27948d27968d27988d279a8d279c8d27a08d279e8d27a28d27a48d27a68d27a88d27aa8d27ac8d27")\
            X(144 , 1, 0x126, KeyUpArrow         , "UpArrow"           , ""    , 0     , 1    , 'A', -1    , -1    , "0148260748261148260f48261548261748260348260b48260d48261948260548260948261b48261348262148261f48261d48262348262548262948262748262b48263148262f48262d48263548263748263348263948263b48263f48263d48264148264348264548264748264948264f48265148264b48264d48265348265748265948265548265b48265d48265f48266148266548266d48266348266b48266748266948266f48267148267348267d48267548267748267948267b48267f48268148268748268348268548268948268b48268d48268f48269348269148269548269748269948269b48269d4826a148269f4826a34826a54826a74826a94826ab4826ad4826")\
            X( 145, 1, 0x026, NumpadUpArrow      , "UpArrow"           , ""    , 0     , 57419, 'u', -1    , -1    , "0088260688261088260e88261488261688260288260a88260c88261888260488260888261a88261288262088261e88261c88262288262488262888262688262a88263088262e88262c88263488263688263288263888263a88263e88263c88264088264288264488264688264888264e88265088264a88264c88265288265688265888265488265a88265c88265e88266088266488266c88266288266a88266688266888266e88267088267288267c88267488267688267888267a88267e88268088268688268288268488268888268a88268c88268e88269288269088269488269688269888269a88269c8826a088269e8826a28826a48826a68826a88826aa8826ac8826")\
            X(146 , 1, 0x128, KeyDownArrow       , "DownArrow"         , ""    , 0     , 1    , 'B', -1    , -1    , "0150280750281150280f50281550281750280350280b50280d50281950280550280950281b50281350282150281f50281d50282350282550282950282750282b50283150282f50282d50283550283750283350283950283b50283f50283d50284150284350284550284750284950284f50285150284b50284d50285350285750285950285550285b50285d50285f50286150286550286d50286350286b50286750286950286f50287150287350287d50287550287750287950287b50287f50288150288750288350288550288950288b50288d50288f50289350289150289550289750289950289b50289d5028a150289f5028a35028a55028a75028a95028ab5028ad5028")\
            X( 147, 1, 0x028, NumpadDownArrow    , "DownArrow"         , ""    , 0     , 57420, 'u', -1    , -1    , "0080280680281080280e80281480281680280280280a80280c80281880280480280880281a80281280282080281e80281c80282280282480282880282680282a80283080282e80282c80283480283680283280283880283a80283e80283c80284080284280284480284680284880284e80285080284a80284c80285280285680285880285480285a80285c80285e80286080286480286c80286280286a80286680286880286e80287080287280287c80287480287680287880287a80287e80288080288680288280288480288880288a80288c80288e80289280289080289480289680289880289a80289c8028a080289e8028a28028a48028a68028a88028aa8028ac8028")\
            X(148 , 1, 0x030, Key0               , "0"                 , "0"   , 0x30  , 48   , 'u', '0'   , '0'   , "000b30060b30100b300e0b30140b30160b30020b300a0b300c0b30180b30040b30080b301a0b30120b30200b301e0b301c0b30220b30240b30280b30260b302a0b30300b302e0b302c0b30340b30360b30320b30380b303a0b303e0b303c0b30400b30420b30440b30460b30480b304e0b30500b304a0b304c0b30520b30560b30580b30540b305a0b305c0b305e0b30600b30640b306c0b30620b306a0b30660b30680b306e0b30700b30720b307c0b30740b30760b30780b307a0b307e29307e0bc0800b30860b30820b30840b30880b308a0b308c0b308e0bc0920b30900b30940b30960b30980b309a0b309c0b30a00b309e0b30a20b30a40b30a60b30a82d30aa3430ac0b30")\
            X( 149, 1, 0x060, Numpad0            , "0"                 , "0"   , 0x30  , 57399, 'u', '0'   , '0'   , "0052600652601052600e52601452601652600252600a52600c52601852600452600852601a52601252602052601e52601c52602252602452602852602652602a52603052602e52602c52603452603652603252603852603a52603e52603c52604052604252604452604652604852604e52605052604a52604c52605252605652605852605452605a52605c52605e52606052606452606c52606252606a52606652606852606e52607052607252607c52607452607652607852607a52607e52608052608652608252608452608852608a52608c52608e52609252609052609452609652609852609a52609c5260a052609e5260a25260a45260a65260a85260aa5260ac5260")\
            X(150 , 1, 0x031, Key1               , "1"                 , "1"   , 0x31  , 49   , 'u', '1'   , '1'   , "0002310602311002310e02311402311602310202310a02310c02311802310402310802311a02311202312002311e02311c02312202312402312802312602312a02313002312e02312c02313402313602313202313802313a02313e02313c02314002314202314402314602314802314e02315002314a02314c02315202315602315802315402315a02315c02315e02316002316402316c02316202316a02316602316802316e02317002317202317c02317402317602317802317a02317e02318002318602318202318402318802318a02318c02318e02319202319002319402319602319802319a02319c0231a002319e0231a20231a40231a60231a80231aa0d31ac0231")\
            X( 151, 1, 0x061, Numpad1            , "1"                 , "1"   , 0x31  , 57400, 'u', '1'   , '1'   , "004f61064f61104f610e4f61144f61164f61024f610a4f610c4f61184f61044f61084f611a4f61124f61204f611e4f611c4f61224f61244f61284f61264f612a4f61304f612e4f612c4f61344f61364f61324f61384f613a4f613e4f613c4f61404f61424f61444f61464f61484f614e4f61504f614a4f614c4f61524f61564f61584f61544f615a4f615c4f615e4f61604f61644f616c4f61624f616a4f61664f61684f616e4f61704f61724f617c4f61744f61764f61784f617a4f617e4f61804f61864f61824f61844f61884f618a4f618c4f618e4f61924f61904f61944f61964f61984f619a4f619c4f61a04f619e4f61a24f61a44f61a64f61a84f61aa4f61ac4f61")\
            X(152 , 1, 0x032, Key2               , "2"                 , "2"   , 0x32  , 50   , 'u', '2'   , '\0'  , "0003320603321003320e03321403321603320203320a03320c03321803320403320803321a03321203322003321e03321c03322203322403322803322603322a03323003322e03322c03323403323603323203323803323a03323e03323c03324003324203324403324603324803324e03325003324a03324c03325203325603325803325403325a03325c03325e03326003326403326c03326203326a03326603326803326e03327003327203327c03327403327603327803327a03327e03328003328603328203328403328803328a03328c03328e03329203329003329403329603329803329a03329c0332a003329e0332a20332a40332a60332a80332aa0c32ac0332")\
            X( 153, 1, 0x062, Numpad2            , "2"                 , "2"   , 0x32  , 57401, 'u', '2'   , '\0'  , "0050620650621050620e50621450621650620250620a50620c50621850620450620850621a50621250622050621e50621c50622250622450622850622650622a50623050622e50622c50623450623650623250623850623a50623e50623c50624050624250624450624650624850624e50625050624a50624c50625250625650625850625450625a50625c50625e50626050626450626c50626250626a50626650626850626e50627050627250627c50627450627650627850627a50627e50628050628650628250628450628850628a50628c50628e50629250629050629450629650629850629a50629c5062a050629e5062a25062a45062a65062a85062aa5062ac5062")\
            X(154 , 1, 0x033, Key3               , "3"                 , "3"   , 0x33  , 51   , 'u', '3'   , '\x1b', "0004330604331004330e04331404331604330204330a04330c04331804330404330804331a04331204332004331e04331c04332204332404332804332604332a04333004332e04332c04333404333604333204333804333a04333e04333c04334004334204334404334604334804334e04335004334a04334c04335204335604335804335404335a04335c04335e04336004336404336c04336204336a04336604336804336e04337004337204337c04337404337604337804337a04337e04338004338604338204338404338804338a04338c04338e04339204339004339404339604339804339a04339c0433a004339e0433a20433a40433a60433a80433aa0b33ac0433")\
            X( 155, 1, 0x063, Numpad3            , "3"                 , "3"   , 0x33  , 57402, 'u', '3'   , '\x1b', "0051630651631051630e51631451631651630251630a51630c51631851630451630851631a51631251632051631e51631c51632251632451632851632651632a51633051632e51632c51633451633651633251633851633a51633e51633c51634051634251634451634651634851634e51635051634a51634c51635251635651635851635451635a51635c51635e51636051636451636c51636251636a51636651636851636e51637051637251637c51637451637651637851637a51637e51638051638651638251638451638851638a51638c51638e51639251639051639451639651639851639a51639c5163a051639e5163a25163a45163a65163a85163aa5163ac5163")\
            X(156 , 1, 0x034, Key4               , "4"                 , "4"   , 0x34  , 52   , 'u', '4'   , '\x1c', "0005340605341005340e05341405341605340205340a05340c05341805340405340805341a05341205342005341e05341c05342205342405342805342605342a05343005342e05342c05343405343605343205343805343a05343e05343c05344005344205344405344605344805344e05345005344a05344c05345205345605345805345405345a05345c05345e05346005346405346c05346205346a05346605346805346e05347005347205347c05347405347605347805347a05347e05348005348605348205348405348805348a05348c05348e05349205349005349405349605349805349a05349c0534a005349e0534a20534a40534a60534a80534aa0a34ac0534")\
            X( 157, 1, 0x064, Numpad4            , "4"                 , "4"   , 0x34  , 57403, 'u', '4'   , '\x1c', "004b64064b64104b640e4b64144b64164b64024b640a4b640c4b64184b64044b64084b641a4b64124b64204b641e4b641c4b64224b64244b64284b64264b642a4b64304b642e4b642c4b64344b64364b64324b64384b643a4b643e4b643c4b64404b64424b64444b64464b64484b644e4b64504b644a4b644c4b64524b64564b64584b64544b645a4b645c4b645e4b64604b64644b646c4b64624b646a4b64664b64684b646e4b64704b64724b647c4b64744b64764b64784b647a4b647e4b64804b64864b64824b64844b64884b648a4b648c4b648e4b64924b64904b64944b64964b64984b649a4b649c4b64a04b649e4b64a24b64a44b64a64b64a84b64aa4b64ac4b64")\
            X(158 , 1, 0x035, Key5               , "5"                 , "5"   , 0x35  , 53   , 'u', '5'   , '\x1d', "0006350606351006350e06351406351606350206350a06350c06351806350406350806351a06351206352006351e06351c06352206352406352806352606352a06353006352e06352c06353406353606353206353806353a06353e06353c06354006354206354406354606354806354e06355006354a06354c06355206355606355806355406355a06355c06355e06356006356406356c06356206356a06356606356806356e06357006357206357c06357406357606357806357a06357e06358006358606358206358406358806358a06358c06358e06359206359006359406359606359806359a06359c0635a006359e0635a20635a40635a60635a81035aa1a35ac0635")\
            X( 159, 1, 0x065, Numpad5            , "5"                 , "5"   , 0x35  , 57404, 'u', '5'   , '\x1d', "004c65064c65104c650e4c65144c65164c65024c650a4c650c4c65184c65044c65084c651a4c65124c65204c651e4c651c4c65224c65244c65284c65264c652a4c65304c652e4c652c4c65344c65364c65324c65384c653a4c653e4c653c4c65404c65424c65444c65464c65484c654e4c65504c654a4c654c4c65524c65564c65584c65544c655a4c655c4c655e4c65604c65644c656c4c65624c656a4c65664c65684c656e4c65704c65724c657c4c65744c65764c65784c657a4c657e4c65804c65864c65824c65844c65884c658a4c658c4c658e4c65924c65904c65944c65964c65984c659a4c659c4c65a04c659e4c65a24c65a44c65a64c65a84c65aa4c65ac4c65")\
            X(160 , 1, 0x036, Key6               , "6"                 , "6"   , 0x36  , 54   , 'u', '6'   , '\x1e', "0007360607361007360e07361407361607360207360a07360c07361807360407360807361a07361207362007361e07361c07362207362407362807362607362a07363007362e07362c07363407363607363207363807363a07363e07363c07364007364207364407364607364807364e07365007364a07364c07365207365607365807365407365a07365c07365e07366007366407366c07366207366a07366607366807366e07367007367207367c07367407367607367807367a07367e07368007368607368207368407368807368a07368c07368e07369207369007369407369607369807369a07369c0736a007369e0736a20736a40736a60736a81136aa1936ac0736")\
            X( 161, 1, 0x066, Numpad6            , "6"                 , "6"   , 0x36  , 57405, 'u', '6'   , '\x1e', "004d66064d66104d660e4d66144d66164d66024d660a4d660c4d66184d66044d66084d661a4d66124d66204d661e4d661c4d66224d66244d66284d66264d662a4d66304d662e4d662c4d66344d66364d66324d66384d663a4d663e4d663c4d66404d66424d66444d66464d66484d664e4d66504d664a4d664c4d66524d66564d66584d66544d665a4d665c4d665e4d66604d66644d666c4d66624d666a4d66664d66684d666e4d66704d66724d667c4d66744d66764d66784d667a4d667e4d66804d66864d66824d66844d66884d668a4d668c4d668e4d66924d66904d66944d66964d66984d669a4d669c4d66a04d669e4d66a24d66a44d66a64d66a84d66aa4d66ac4d66")\
            X(162 , 1, 0x037, Key7               , "7"                 , "7"   , 0x37  , 55   , 'u', '7'   , '\x1f', "0008370608371008370e08371408371608370208370a08370c08371808370408370808371a08371208372008371e08371c08372208372408372808372608372a08373008372e08372c08373408373608373208373808373a08373e08373c08374008374208374408374608374808374e08375008374a08374c08375208375608375808375408375a08375c08375e08376008376408376c08376208376a08376608376808376e08377008377208377c08377408377608377808377a08377e08378008378608378208378408378808378a08378c08378e08379208379008379408379608379808379a08379c0837a008379e0837a20837a40837a60837a81e37aa2837ac0837")\
            X( 163, 1, 0x067, Numpad7            , "7"                 , "7"   , 0x37  , 57406, 'u', '7'   , '\x1f', "0047670647671047670e47671447671647670247670a47670c47671847670447670847671a47671247672047671e47671c47672247672447672847672647672a47673047672e47672c47673447673647673247673847673a47673e47673c47674047674247674447674647674847674e47675047674a47674c47675247675647675847675447675a47675c47675e47676047676447676c47676247676a47676647676847676e47677047677247677c47677447677647677847677a47677e47678047678647678247678447678847678a47678c47678e47679247679047679447679647679847679a47679c4767a047679e4767a24767a44767a64767a84767aa4767ac4767")\
            X(164 , 1, 0x038, Key8               , "8"                 , "8"   , 0x38  , 56   , 'u', '8'   , '\x7f', "0009380609381009380e09381409381609380209380a09380c09381809380409380809381a09381209382009381e09381c09382209382409382809382609382a09383009382e09382c09383409383609383209383809383a09383e09383c09384009384209384409384609384809384e09385009384a09384c09385209385609385809385409385a09385c09385e09386009386409386c09386209386a09386609386809386e09387009387209387c09387409387609387809387a09387e09388009388609388209388409388809388a09388c09388e09389209389009389409389609389809389a09389c0938a009389e0938a20938a40938a60938a81f38aa2738ac0938")\
            X( 165, 1, 0x068, Numpad8            , "8"                 , "8"   , 0x38  , 57407, 'u', '8'   , '\x7f', "0048680648681048680e48681448681648680248680a48680c48681848680448680848681a48681248682048681e48681c48682248682448682848682648682a48683048682e48682c48683448683648683248683848683a48683e48683c48684048684248684448684648684848684e48685048684a48684c48685248685648685848685448685a48685c48685e48686048686448686c48686248686a48686648686848686e48687048687248687c48687448687648687848687a48687e48688048688648688248688448688848688a48688c48688e48689248689048689448689648689848689a48689c4868a048689e4868a24868a44868a64868a84868aa4868ac4868")\
            X(166 , 1, 0x039, Key9               , "9"                 , "9"   , 0x39  , 57   , 'u', '9'   , '9'   , "000a39060a39100a390e0a39140a39160a39020a390a0a390c0a39180a39040a39080a391a0a39120a39200a391e0a391c0a39220a39240a39280a39260a392a0a39300a392e0a392c0a39340a39360a39320a39380a393a0a393e0a393c0a39400a39420a39440a39460a39480a394e0a39500a394a0a394c0a39520a39560a39580a39540a395a0a395c0a395e0a39600a39640a396c0a39620a396a0a39660a39680a396e0a39700a39720a397c0a39740a39760a39780a397a0a397e0a39800a39860a39820a39840a39880a398a0a398c0a398e0a39920a39900a39940a39960a39980a399a0a399c0a39a00a399e0a39a20a39a40a39a60a39a82c39aa3539ac0a39")\
            X( 167, 1, 0x069, Numpad9            , "9"                 , "9"   , 0x39  , 57408, 'u', '9'   , '9'   , "0049690649691049690e49691449691649690249690a49690c49691849690449690849691a49691249692049691e49691c49692249692449692849692649692a49693049692e49692c49693449693649693249693849693a49693e49693c49694049694249694449694649694849694e49695049694a49694c49695249695649695849695449695a49695c49695e49696049696449696c49696249696a49696649696849696e49697049697249697c49697449697649697849697a49697e49698049698649698249698449698849698a49698c49698e49699249699049699449699649699849699a49699c4969a049699e4969a24969a44969a64969a84969aa4969ac4969")\
            X(168 , 1, 0    , KeyMultiply        , "*"                 , "*"   , 0x2A  , 42   , 'u', '*'   , '*'   , "641bba8c0cdf962bdc982bdc9a2bdc9c2bdc")\
            X( 169, 1, 0x06A, NumpadMultiply     , "*"                 , "*"   , 0x2A  , 57411, 'u', '*'   , '*'   , "00376a06376a10376a0e376a14376a16376a02376a0a376a0c376a18376a04376a08376a1a376a12376a20376a1e376a1c376a22376a24376a28376a26376a2a376a30376a2e376a2c376a34376a36376a32376a38376a3a376a3e376a3c376a40376a42376a44376a46376a48376a4e376a50376a4a376a4c376a52376a56376a58376a54376a5a376a5c376a5e376a60376a64376a6c376a62376a6a376a66376a68376a6e376a70376a72376a7c376a74376a76376a78376a7a376a7e376a80376a86376a82376a84376a88376a8a376a8c376a8e376a92376a90376a94376a96376a98376a9a376a9c376aa0376a9e376aa2376aa4376aa6376aa8376aaa376aac376a")\
            X(170 , 1, 0    , KeySlash           , "/"                 , "/"   , 0x2F  , 47   , 'u', '/'   , '\x1f', "0035bf0635bf1035bf0e35bf1435bf1635bf0235bf0a35bf0c35bf1835bf0435bf0835bf1a35bf1235bf2035bf1e35bf1c35bf2235bf2435bf2835bf2635bf2a35bf3035bf2e35bf2c35bf3435bf3635bf3235bf3835bf3c73c14035bf4229de640cdb9235bf9a28dba41abfa60cbba80bbfaa04bf")\
            X( 171, 1, 0x16F, NumpadDivide       , "/"                 , "/"   , 0x2F  , 57410, 'u', '/'   , '\x1f', "01356f07356f11356f0f356f15356f17356f03356f0b356f0d356f19356f05356f09356f1b356f13356f21356f1f356f1d356f23356f25356f29356f27356f2b356f31356f2f356f2d356f35356f37356f33356f39356f3b356f3f356f3d356f41356f43356f45356f47356f49356f4f356f51356f4b356f4d356f53356f57356f59356f55356f5b356f5d356f5f356f61356f65356f6d356f63356f6b356f67356f69356f6f356f71356f73356f7d356f75356f77356f79356f7b356f7f356f81356f87356f83356f85356f89356f8b356f8d356f8f356f93356f91356f95356f97356f99356f9b356f9d356fa1356f9f356fa3356fa5356fa7356fa9356fab356fad356f")\
            X(172 , 1, 0    , KeyPlus            , "Plus"              , "+"   , 0x2B  , 43   , 'u', '+'   , '+'   , "440cbb461abb480dbb4e0cbb501bbb4a0cbb4c0cbb560cbd5a1bbb5c1bbb5e1abb600cbb6427bb6c0cbb620cbb6a1bbb660cbb681bbb702bbf720cbb7c0dbb761bbb781bbb7a1bbb801bbb861bbb821bbb841bbb900dbb940cbd9a1bbba629c0")\
            X( 173, 1, 0x06B, NumpadPlus         , "Plus"              , "+"   , 0x2B  , 57413, 'u', '+'   , '+'   , "004e6b064e6b104e6b0e4e6b144e6b164e6b024e6b0a4e6b0c4e6b184e6b044e6b084e6b1a4e6b124e6b204e6b1e4e6b1c4e6b224e6b244e6b284e6b264e6b2a4e6b304e6b2e4e6b2c4e6b344e6b364e6b324e6b384e6b3a4e6b3e4e6b3c4e6b404e6b424e6b444e6b464e6b484e6b4e4e6b504e6b4a4e6b4c4e6b524e6b564e6b584e6b544e6b5a4e6b5c4e6b5e4e6b604e6b644e6b6c4e6b624e6b6a4e6b664e6b684e6b6e4e6b704e6b724e6b7c4e6b744e6b764e6b784e6b7a4e6b7e4e6b804e6b864e6b824e6b844e6b884e6b8a4e6b8c4e6b8e4e6b924e6b904e6b944e6b964e6b984e6b9a4e6b9c4e6ba04e6b9e4e6ba24e6ba44e6ba64e6ba84e6baa4e6bac4e6b")\
            X(174 , 1, 0    , KeyMinus           , "Minus"             , "-"   , 0x2D  , 45   , 'u', '-'   , '-'   , "000cbd060cbd100cbd0e0cbd140cbd160cbd020cbd0a0cbd0c0cbd180cbd040cbd080cbd1a0cbd120cbd200cbd1e0cbd1c0cbd220cbd240cbd280cbd260cbd2a0cbd300cbd2e0cbd2c0cbd340cbd360cbd320cbd380cbd3a0cbd3e0cbd3c0cbd400cbd420cbd4435bd4635bd4835bd4e35bd5035bd4a35bd4c35bd5235bf5635dd5835bf540cbd5a35bd5c35bd5e35bd6035bd6435bd6c35bd6235bd6a35bd6635bd6835bd6e35bd700ddb7235bd7c35bd7435bd7635bd7835bd7a35bd7e35bd8035bd8635bd8235bd8435bd8835bd8a35bd8c0dbd8e35bf9256e2920cbd9035bd9435bf9a1abda00dbd9e0dbda428bda60dbda828bdaa1ebd")\
            X( 175, 1, 0x06D, NumpadMinus        , "Minus"             , "-"   , 0x2D  , 57412, 'u', '-'   , '-'   , "004a6d064a6d104a6d0e4a6d144a6d164a6d024a6d0a4a6d0c4a6d184a6d044a6d084a6d1a4a6d124a6d204a6d1e4a6d1c4a6d224a6d244a6d284a6d264a6d2a4a6d304a6d2e4a6d2c4a6d344a6d364a6d324a6d384a6d3a4a6d3e4a6d3c4a6d404a6d424a6d444a6d464a6d484a6d4e4a6d504a6d4a4a6d4c4a6d524a6d564a6d584a6d544a6d5a4a6d5c4a6d5e4a6d604a6d644a6d6c4a6d624a6d6a4a6d664a6d684a6d6e4a6d704a6d724a6d7c4a6d744a6d764a6d784a6d7a4a6d7e4a6d804a6d864a6d824a6d844a6d884a6d8a4a6d8c4a6d8e4a6d924a6d904a6d944a6d964a6d984a6d9a4a6d9c4a6da04a6d9e4a6da24a6da44a6da64a6da84a6daa4a6dac4a6d")\
            X(176 , 1, 0    , KeyEqual           , "="                 , "="   , 0x3D  , 61   , 'u', '='   , '='   , "000dbb060dbb100dbb0e0dbb140dbb160dbb020dbb0a0dbb0c0dbb180dbb040dbb080dbb1a0dbb120dbb200dbb1c0dbb220dbb240dbb280dbb260dbb2a0dbb300dbb2e0dbb2c0dbb340dbb360dbb380dbb3a0dbb3e0dbb3c0dbb400dbb420dbb520cbd580cbd5435df6e0cbb740cbf920dbb960dbb980dbb9c0dbba035bb9e35bba41bbba81bbbaa1bbbac0cbd")\
            X( 177, 1, 0x092, NumpadEqual        , "="                 , "="   , 0x3D  , 57415, 'u', '='   , '='   , "")\
            X(178 , 1, 0    , KeyPeriod          , "."                 , "."   , 0x2E  , 46   , 'u', '.'   , '.'   , "0034be0634be1034be0e34be1434be1634be0234be0a34be0c34be1834be0434be0834be1a34be1234be2034be1e34be1c34be2234be2434be2834be2634be2a34be3034be2e34be2c34be3434be3634be3234be3834be3a34be3e34be3c34be4034be4234be4434be4634be4834be4e34be5034be4a34be4c34be5234be5634be5834be5434be5a34be5c34be5e34be6034be6434be6c34be6234be6a34be6634be6834be6e34be7034be7234be7c34be7434be7634be7834be7a34be7e34be8034be8634be8234be8434be8834be8a34be8c35be8e34be9234be9034be9434be9a32bea412bea634bea813beaa18beac2fbe")\
            X( 179, 1, 0x06E, NumpadDecimal      , "."                 , "."   , 0x2E  , 57409, 'u', '.'   , '.'   , "00536e06536e10536e0e536e14536e16536e02536e0a536e0c536e18536e04536e08536e1a536e12536e20536e1e536e1c536e22536e24536e28536e26536e2a536e30536e2e536e2c536e34536e36536e32536e38536e3a536e3e536e3c536e40536e42536e44536e46536e48536e4e536e50536e4a536e4c536e52536e56536e58536e54536e5a536e5c536e5e536e60536e64536e6c536e62536e6a536e66536e68536e6e536e70536e72536e7c536e74536e76536e78536e7a536e7e536e80536e86536e82536e84536e88536e8a536e8c536e8e536e92536e90536e94536e96536e98536e9a536e9c536ea0536e9e536ea2536ea4536ea6536ea8536eaa536eac536e")\
            X(180 , 1, 0    , KeyComma           , ","                 , ","   , 0x2C  , 44   , 'u', ','   , ','   , "0033bc0633bc1033bc0e33bc1433bc1633bc0233bc0a33bc0c33bc1833bc0433bc0833bc1a33bc1233bc2033bc1e33bc1c33bc2233bc2433bc2833bc2633bc2a33bc3033bc2e33bc2c33bc3433bc3633bc3233bc3833bc3a33bc3e33bc3c33bc4033bc4233bc4433bc4633bc4833bc4e33bc5033bc4a33bc4c33bc5233bc5633bc5833bc5433bc5a33bc5c33bc5e33bc6033bc6433bc6c33bc6233bc6a33bc6633bc6833bc6e33bc7033bc7233bc7c33bc7433bc7633bc7833bc7a33bc7e33bc8033bc8633bc8233bc8433bc8833bc8a33bc8c2bbc8e33bc9233bc9033bc9433bc9632bc9832bc9a33bc9c32bca032bc9e32bca411bca635bca82fbcaa33bcac22bc")\
            X( 181, 1, 0x0C2, NumpadPoint        , ","                 , ","   , 0x2C  , 57416, 'u', ','   , ','   , "007ec2067ec2107ec20e7ec2147ec2167ec2027ec20a7ec20c7ec2187ec2047ec2087ec21a7ec2127ec2207ec21e7ec21c7ec2227ec2247ec2287ec2267ec22a7ec2307ec22e7ec22c7ec2347ec2367ec2327ec2387ec23a7ec23e7ec23c7ec2407ec2427ec2447ec2467ec2487ec24e7ec2507ec24a7ec24c7ec2527ec2567ec2587ec2547ec25a7ec25c7ec25e7ec2607ec2647ec26c7ec2627ec26a7ec2667ec2687ec26e7ec2707ec2727ec27c7ec2747ec2767ec2787ec27a7ec27e7ec2807ec2867ec2827ec2847ec2887ec28a7ec28c7ec28e7ec2927ec2907ec2947ec2967ec2987ec29a7ec29c7ec2a07ec29e7ec2a27ec2a47ec2a67ec2a87ec2aa7ec2ac7ec2")\
            X(182 , 1, 0    , Colon              , ":"                 , ":"   , 0x3A  , 58   , 'u', ':'   , ':'   , "9634bf9834bf9a34bf9c34bfa034bf9e34bf")\
            X(184 , 1, 0    , Semicolon          , ";"                 , ";"   , 0x3B  , 59   , 'u', ';'   , ';'   , "0027ba0627ba1027ba0e27ba1427ba1627ba0227ba0a27ba0c27ba1827ba0427ba0827ba1a27ba1227ba2027ba1e27ba1c27ba2227ba2427ba2827ba2627ba2a27ba3027ba2e27ba2c27ba3427ba3627ba3227ba3a27ba3e27ba3c35bf4227ba5229c05829c06e29c07429c09219ba9633be9833be9a35ba9c33bea033be9e33bea42cbaa81abaaa10ba")\
            X(186 , 1, 0    , TurnedComma        , "ʻ"                 , "ʻ"   , 0x02BB, 699  , 'u', -1    , -1    , "2028de")\
            X(188 , 1, 0    , OpenSquareBracket  , "["                 , "["   , 0x5B  , 91   , 'u', '['   , '\x1b', "001adb061adb101adb0e1adb141adb161adb021adb0a1adb0c1adb181adb041adb081adb1a1adb121adb201adb1e1adb1c1adb221adb241adb281adb261adb2a1adb301adb2e1adb2c1adb3c1bdd4028de481adb921adba40cdba80cdbaa02db")\
            X(190 , 1, 0    , CloseSquareBracket , "]"                 , "]"   , 0x5D  , 93   , 'u', ']'   , '\x1d', "001bdd061bdd101bdd0e1bdd141bdd161bdd021bdd0a1bdd0c1bdd181bdd041bdd081bdd1a1bdd121bdd201bdd1e1bdd1c1bdd221bdd241bdd281bdd261bdd2a1bdd301bdd2e1bdd2c1bdd3c2bdc402bdc460ddb481bdd6456e2921bdd9429c0a40ddda80dddaa03dd")\
            X(192 , 1, 0    , OpenCurlyBracket   , "{"                 , "{"   , 0x7B  , 123  , 'u', '{'   , 27    , "5028de6a28de")\
            X(194 , 1, 0    , CloseCurlyBracket  , "}"                 , "}"   , 0x7D  , 125  , 'u', '}'   , 29    , "461bdd502bbf6a2bbf")\
            X(196 , 1, 0    , CloseRoundBracket  , ")"                 , ")"   , 0x29  , 41   , 'u', ')'   , ')'   , "521bdd6e1bdd960cdb980cdb9c0cdba00cdb9e0cdb")\
            X(198 , 1, 0    , LessThan           , "<"                 , "<"   , 0x3C  , 60   , 'u', '<'   , '<'   , "2429c03a2bdc4056e24456e24656e24856e24e56e25056e24a56e24c56e25656e25456e25a56e25c56e25e56e26056e2642bdc6c56e26256e26a56e26656e26856e27056e27256e27c56e27656e27856e27a56e28056e28656e28256e28856e28a56e28c56e29056e29456e29656e29856e29a56e29c56e2a056e29e56e2a256e2a656e2")\
            X(200 , 1, 0    , BackSlash          , "\\"                , "\\"  , 0x5C  , 92   , 'u', '\\'  , '\x1c', "002bdc0056e2062bdc0656e2102bdc1056e20e56e20e2bdc1456e2142bdc162bdc1656e2022bdc0256e20a2bdc0a56e20c2bdc0c56e21856e2182bdc042bdc0456e20856e2082bdc1a2bdc1a56e2122bdc1256e2202bdc2056e21e56e21e2bdc1c2bdc1c56e2222bdc2256e22456e2282bde2856dc262bde2656dc2a56dc3056dc2e56dc2c56dc3256e2322bdc3856e23c56e24029c04629c04829c04a0ddb5256e25a29dc5c29dc5e29dc660ddb6e56e2922bdca42bdca456e2a82bdca856e2aa56e2aa2bdc")\
            X(202 , 1, 0    , Underscore         , "_"                 , "_"   , 0x5F  , 95   , 'u', '_'   , '\x1f', "0273e2")\
            X(204 , 1, 0    , VerticalBar        , "|"                 , "|"   , 0x7C  , 124  , 'u', '|'   , 28    , "5029dc4a29dc6a29dc6629dc")\
            X(206 , 1, 0    , DivisionSign       , "÷"                 , "÷"   , 0xF7  , 247  , 'u', -1    , -1    , "541add")\
            X(208 , 1, 0    , OneHalf            , "½"                 , "½"   , 0xBD  , 189  , 'u', -1    , -1    , "4e29dc6029dc6c29dc")\
            X(210 , 1, 0    , SuperscriptTwo     , "²"                 , "²"   , 0xB2  , 178  , 'u', -1    , -1    , "9629de9c29dea029de9e29de")\
            X(212 , 1, 0    , DegreeSign         , "°"                 , "°"   , 0xB0  , 176  , 'u', -1    , -1    , "3e29de640dbf7029dc")\
            X(214 , 1, 0    , NumeroSign         , "º"                 , "º"   , 0xBA  , 186  , 'u', -1    , -1    , "5e28de6829dc")\
            X(216 , 1, 0    , Acute              , "´"                 , "´"   , 0xB4  , 180  , 'u', -1    , -1    , "3c1adb440ddb4827ba4e0ddb501aba4c0ddb520dbb560dbb580dbb542bbf5e1bba600ddb6428c06c0ddb620ddb6a1aba6828de6e0dbf7028de740ddf760ddd780ddd7a0ddd800ddd860ddd820ddd840ddd")\
            X(218 , 1, 0    , Caron              , "ˇ"                 , "ˇ"   , 0x02C7, 780  , 'u', -1    , -1    , "5629de")\
            X(220 , 1, 0    , Cedilla            , "¸"                 , "¸"   , 0xB8  , 184  , 'u', -1    , -1    , "3a1bdd9029c0")\
            X(222 , 1, 0    , Circumflex         , "^"                 , "^"   , 0x5E  , 94   , 'u', '^'   , 30    , "3a1adb3e1adb421adb4828de7629dc7829dc7a29dc8029dc8629dc8229dc8429dc880ddd8a0ddd961add981add9a0ddd9c1adda01add9e1addac15db")\
            X(224 , 1, 0    , Ogonek             , "˛"                 , "˛"   , 0x02DB, 731  , 'u', -1    , -1    , "7229c0")\
            X(226 , 1, 0    , Cross              , "˟"                 , "˟"   , 0x02DF, 735  , 'u', -1    , -1    , "8456e2")\
            X(228 , 1, 0    , Tilde              , "~"                 , "~"   , 0x7E  , 771  , 'u', '~'   , 30    , "3c28de5e2bbf")\
            X(230 , 1, 0    , Tonos              , "΄"                 , "΄"   , 0x0384, 900  , 'u', -1    , -1    , "4627ba")\
            X(232 , 1, 0    , Umlaut             , "¨"                 , "¨"   , 0xA8  , 168  , 'u', -1    , -1    , "441bba4628de4e1bba4a1bba4c1bba522bdc540ddb601bba641add621bba661bba6e2bdc881bc08a1bc0")\
            X(234 , 1, 0    , BackQuote          , "`"                 , "`"   , 0x60  , 96   , 'u', '`'   , '`'   , "0029c00629c01029c00e29c01429c01629c00229c00a29c00c29c01829c00429c00829c01a29c01229c02029c01e29c01c29c02229c0242bdc2829df2629df2a29df3029df2e29df2c29df3229c03a28c0482bdc541bba681aba9229c0a229dca429c0a829c0aa29c0")\
            X(236 , 1, 0    , SingleQuote        , "'"                 , "'"   , 0x27  , 39   , 'u', '\''  , '\''  , "0028de0628de1028de0e28de1428de1628de0228de0a28de0c28de1828de0428de0828de1a28de1228de1e28de1c28de2228de2428de2828c02628c02a28c03028c02e28c02c28c03428de3628de3228de3c29c0442bbf460cbf480cbf4e2bbf500cdb4a2bbf4c2bbf562bdc5429dc5a0cdb5c0cdb5e0cdb602bbf6c2bbf622bbf6a0cdb662bbf680cdb701bba720dbf7c0cbf880cdb8a0cdb9228de900cbf940dbb9a0cdea410dea835deaa2cde")\
            X(238 , 1, 0    , DoubleQuote        , "\""                , "\""  , 0x22  , 34   , 'u', '"'   , '"'   , "8c29c0")\
            X(240 , 1, 0    , SingleRightQuote   , "’"                 , "’"   , 0x2019, 8217 , 'u', -1    , -1    , "ac31c0")\
            X(242 , 1, 0    , SingleLowQuote     , "‚"                 , "‚"   , 0x201A, 8218 , 'u', -1    , -1    , "7c29c0")\
            X(244 , 1, 0    , DoubleLowQuote     , "„"                 , "„"   , 0x201E, 8222 , 'u', -1    , -1    , "3829c0")\
            X(246 , 1, 0    , LeftGuillemet      , "«"                 , "«"   , 0xAB  , 171  , 'u', -1    , -1    , "3a56e25e0ddd")\
            X(248 , 1, 0    , Hash               , "#"                 , "#"   , 0x23  , 35   , 'u', '#'   , '#'   , "2a2bde302bde2e2bde2c2bde362bdc3a29de462bdc782bbf802bbf862bbf822bbf842bbf")\
            X(250 , 1, 0    , AtSign             , "@"                 , "@"   , 0x40  , 64   , 'u', '@'   , '\0'  , "401bdd6429de9a29c0")\
            X(252 , 1, 0    , Exclamation        , "!"                 , "!"   , 0x21  , 33   , 'u', '!'   , '!'   , "9635df9835df9c35df")\
            X(254 , 1, 0    , InvertedExclamation, "¡"                 , "¡"   , 0xA1  , 161  , 'u', -1    , -1    , "680ddd")\
            X(256 , 1, 0    , QuestionMark       , "?"                 , "?"   , 0x3F  , 63   , 'u', '?'   , 127   , "a20cbb")\
            X(258 , 1, 0    , InvertedQuestion   , "¿"                 , "¿"   , 0xBF  , 191  , 'u', -1    , -1    , "500ddd6a0ddd")\
            X(260 , 1, 0    , Paragraph          , "§"                 , "§"   , 0xA7  , 167  , 'u', -1    , -1    , "4429dc4c29dc5228de5828de6229dc6e28de7428de8829bf8a29bf")\
            X(262 , 1, 0    , Ampersand          , "&"                 , "&"   , 0x26  , 38   , 'u', '&'   , '&'   , "5856e27456e2")\
            X(264 , 1, 0    , Dollar             , "$"                 , "$"   , 0x24  , 36   , 'u', '$'   , '$'   , "882bdf8a2bdf961bba9c1bbaa01bba9e1bbaac29de")\
            X(266 , 1, 0    , Percent            , "%"                 , "%"   , 0x25  , 37   , 'u', '%'   , '%'   , "ac0dbb")\
            X(268 , 1, 0    , Dong               , "₫"                 , "₫"   , 0x20AB, 8363 , 'u', -1    , -1    , "320dbb")\
            X(270 , 1, 0    , Yen                , "¥"                 , "¥"   , 0xA5  , 165  , 'u', -1    , -1    , "027ddc227ddc")\
            X(272 , 1, 0    , DotlessI           , "ı"                 , "ı"   , 0x0131, 305  , 'u', -1    , -1    , "8c1749a61349")\
            X(274 , 1, 0    , MicroSign          , "µ"                 , "µ"   , 0xB5  , 181  , 'u', -1    , -1    , "a02bdc9e2bdc")\
            X(276 , 1, 0    , Eth                , "ð"                 , "ð"   , 0xF0  , 240  , 'u', -1    , -1    , "6c1bba701add")\
            X(278 , 1, 0    , Thorn              , "þ"                 , "þ"   , 0xFE  , 254  , 'u', -1    , -1    , "7035bd")\
            X(280 , 1, 0    , Eszett             , "ẞ"                 , "ß"   , 0xDF  , 223  , 'u', -1    , -1    , "760cdb780cdb7a0cdb800cdb860cdb820cdb840cdb")\
            X(282 , 1, 0x041, KeyA               , "A"                 , "a"   , 0x61  , 97   , 'u', 'a'   , '\x01', "001e41061e41101e410e1e41141e41161e41021e410a1e410c1e41181e41041e41081e411a1e41121e41201e411e1e411c1e41221e41241e41281e41261e412a1e41301e412e1e412c1e41341e41361e41321e41381e413a1e413e1e413c1e41401e41421e41441e41461e41481e414e1e41501e414a1e414c1e41521e41561e41581e41541e415a1e415c1e415e1e41601e41641e416c1e41621e416a1e41661e41681e416e1e41701e41721e417c1e41741e41761e41781e417a1e417e1e41801e41861e41821e41841e41881e418a1e418c1e418e1e41921e41901e41941e419610419810419a10419c1041a010419e1041a21e41a41e41a62141a82141aa2541ac1e41")\
            X(284 , 1, 0x042, KeyB               , "B"                 , "b"   , 0x62  , 98   , 'u', 'b'   , '\x02', "0030420630421030420e30421430421630420230420a30420c30421830420430420830421a30421230422030421e30421c30422230422430422830422630422a30423030422e30422c30423430423630423230423830423a30423e30423c30424030424230424430424630424830424e30425030424a30424c30425230425630425830425430425a30425c30425e30426030426430426c30426230426a30426630426830426e30427030427230427c30427430427630427830427a30427e30428030428630428230428430428830428a30428c30428e30429230429030429430429630429830429a30429c3042a030429e3042a23042a43142a63342a81942aa1242ac1042")\
            X(286 , 1, 0x043, KeyC               , "C"                 , "c"   , 0x63  , 99   , 'u', 'c'   , '\x03', "002e43062e43102e430e2e43142e43162e43022e430a2e430c2e43182e43042e43082e431a2e43122e43202e431e2e431c2e43222e43242e43282e43262e432a2e43302e432e2e432c2e43342e43362e43322e43382e433a2e433e2e433c2e43402e43422e43442e43462e43482e434e2e43502e434a2e434c2e43522e43562e43582e43542e435a2e435c2e435e2e43602e43642e436c2e43622e436a2e43662e43682e436e2e43702e43722e437c2e43742e43762e43782e437a2e437e2e43802e43862e43822e43842e43882e438a2e438c2e438e2e43922e43902e43942e43962e43982e439a2e439c2e43a02e439e2e43a22e43a41743a62f43a82643aa2043ac2343")\
            X(288 , 1, 0x044, KeyD               , "D"                 , "d"   , 0x64  , 100  , 'u', 'd'   , '\x04', "0020440620441020440e20441420441620440220440a20440c20441820440420440820441a20441220442020441e20441c20442220442420442820442620442a20443020442e20442c20443420443620443220443820443a20443e20443c20444020444220444420444620444820444e20445020444a20444c20445220445620445820445420445a20445c20445e20446020446420446c20446220446a20446620446820446e20447020447220447c20447420447620447820447a20447e20448020448620448220448420448820448a20448c20448e20449222449020449420449620449820449a20449c2044a020449e2044a22044a42344a61544a82544aa2144ac1744")\
            X(290 , 1, 0x045, KeyE               , "E"                 , "e"   , 0x65  , 101  , 'u', 'e'   , '\x05', "0012450612451012450e12451412451612450212450a12450c12451812450412450812451a12451212452012451e12451c12452212452412452812452612452a12453012452e12452c12453412453612453212453812453a12453e12453c12454012454212454412454612454812454e12455012454a12454c12455212455612455812455412455a12455c12455e12456012456412456c12456212456a12456612456812456e12457012457212457c12457412457612457812457a12457e12458012458612458212458412458812458a12458c12458e12459225459012459412459612459812459a12459c1245a012459e1245a21245a42045a62045a82245aa2445ac2145")\
            X(292 , 1, 0x046, KeyF               , "F"                 , "f"   , 0x66  , 102  , 'u', 'f'   , '\x06', "0021460621461021460e21461421461621460221460a21460c21461821460421460821461a21461221462021461e21461c21462221462421462821462621462a21463021462e21462c21463421463621463221463821463a21463e21463c21464021464221464421464621464821464e21465021464a21464c21465221465621465821465421465a21465c21465e21466021466421466c21466221466a21466621466821466e21467021467221467c21467421467621467821467a21467e21468021468621468221468421468821468a21468c21468e21469212469021469421469621469821469a21469c2146a021469e2146a234bea41546a61046a80946aa0646ac3546")\
            X(294 , 1, 0x047, KeyG               , "G"                 , "g"   , 0x67  , 103  , 'u', 'g'   , '\x07', "0022470622471022470e22471422471622470222470a22470c22471822470422470822471a22471222472022471e22471c22472222472422472822472622472a22473022472e22472c22473422473622473222473822473a22473e22473c22474022474222474422474622474822474e22475022474a22474c22475222475622475822475422475a22475c22475e22476022476422476c22476222476a22476622476822476e22477022477222477c22477422477622477822477a22477e22478022478622478222478422478822478a22478c22478e22479214479022479422479622479822479a22479c2247a022479e2247a22247a41647a61147a83447aa2e47ac3347")\
            X(296 , 1, 0x048, KeyH               , "H"                 , "h"   , 0x68  , 104  , 'u', 'h'   , '\x08', "0023480623481023480e23481423481623480223480a23480c23481823480423480823481a23481223482023481e23481c23482223482423482823482623482a23483023482e23482c23483423483623483223483823483a23483e23483c23484023484223484423484623484823484e23485023484a23484c23485223485623485823485423485a23485c23485e23486023486423486c23486223486a23486623486823486e23487023487223487c23487423487623487823487a23487e23488023488623488223488423488823488a23488c23488e23489223489023489423489623489823489a23489c2348a023489e2348a22348a42448a61848a82348aa2348ac3448")\
            X(298 , 1, 0x049, KeyI               , "I"                 , "i"   , 0x69  , 105  , 'u', 'i'   , '\x09', "0017490617491017490e17491417491617490217490a17490c17491817490417490817491a17491217492017491e17491c17492217492417492817492617492a17493017492e17492c17493417493617493217493817493a17493e17493c17494017494217494417494617494817494e17495017494a17494c17495217495617495817495417495a17495c17495e17496017496417496c17496217496a17496617496817496e17497017497217497c17497417497617497817497a17497e17498017498617498217498417498817498a17498c28de8e17499226499017499417499617499817499a17499c1749a017499e1749a21749a42249a61fdba83049aa3249ac2049")\
            X(300 , 1, 0x04A, KeyJ               , "J"                 , "j"   , 0x6A  , 106  , 'u', 'j'   , '\x0a', "00244a06244a10244a0e244a14244a16244a02244a0a244a0c244a18244a04244a08244a1a244a12244a20244a1e244a1c244a22244a24244a28244a26244a2a244a30244a2e244a2c244a34244a36244a32244a38244a3a244a3e244a3c244a40244a42244a44244a46244a48244a4e244a50244a4a244a4c244a52244a56244a58244a54244a5a244a5c244a5e244a60244a64244a6c244a62244a6a244a66244a68244a6e244a70244a72244a7c244a74244a76244a78244a7a244a7e244a80244a86244a82244a84244a88244a8a244a8c244a8e244a92154a90244a94244a96244a98244a9a244a9c244aa0244a9e244aa2244aa42e4aa62c4aa8064aaa094aac194a")\
            X(302 , 1, 0x04B, KeyK               , "K"                 , "k"   , 0x6B  , 107  , 'u', 'k'   , '\x0b', "00254b06254b10254b0e254b14254b16254b02254b0a254b0c254b18254b04254b08254b1a254b12254b20254b1e254b1c254b22254b24254b28254b26254b2a254b30254b2e254b2c254b34254b36254b32254b38254b3a254b3e254b3c254b40254b42254b44254b46254b48254b4e254b50254b4a254b4c254b52254b56254b58254b54254b5a254b5c254b5e254b60254b64254b6c254b62254b6a254b66254b68254b6e254b70254b72254b7c254b74254b76254b78254b7a254b7e254b80254b86254b82254b84254b88254b8a254b8c254b8e254b92314b90254b94254b96254b98254b9a254b9c254ba0254b9e254ba2254ba42f4ba6244ba8274baa1f4bac304b")\
            X(304 , 1, 0x04C, KeyL               , "L"                 , "l"   , 0x6C  , 108  , 'u', 'l'   , '\x0c', "00264c06264c10264c0e264c14264c16264c02264c0a264c0c264c18264c04264c08264c1a264c12264c20264c1e264c1c264c22264c24264c28264c26264c2a264c30264c2e264c2c264c34264c36264c32264c38264c3a264c3e264c3c264c40264c42264c44264c46264c48264c4e264c50264c4a264c4c264c52264c56264c58264c54264c5a264c5c264c5e264c60264c64264c6c264c62264c6a264c66264c68264c6e264c70264c72264c7c264c74264c76264c78264c7a264c7e264c80264c86264c82264c84264c88264c8a264c8c264c8e264c92164c90264c94264c96264c98264c9a264c9c264ca0264c9e264ca2264ca4194ca6264ca8074caa084cac184c")\
            X(306 , 1, 0x04D, KeyM               , "M"                 , "m"   , 0x6D  , 109  , 'u', 'm'   , '\x0d', "00324d06324d10324d0e324d14324d16324d02324d0a324d0c324d18324d04324d08324d1a324d12324d20324d1e324d1c324d22324d24324d28324d26324d2a324d30324d2e324d2c324d34324d36324d32324d38324d3a324d3e324d3c324d40324d42324d44324d46324d48324d4e324d50324d4a324d4c324d52324d56324d58324d54324d5a324d5c324d5e324d60324d64324d6c324d62324d6a324d66324d68324d6e324d70324d72324d7c324d74324d76324d78324d7a324d7e324d80324d86324d82324d84324d88324d8a324d8c324d8e324d92324d90324d94324d96274d98274d9a274d9c274da0274d9e274da2324da4324da6254da8084daa074dac284d")\
            X(308 , 1, 0x04E, KeyN               , "N"                 , "n"   , 0x6E  , 110  , 'u', 'n'   , '\x0e', "00314e06314e10314e0e314e14314e16314e02314e0a314e0c314e18314e04314e08314e1a314e12314e20314e1e314e1c314e22314e24314e28314e26314e2a314e30314e2e314e2c314e34314e36314e32314e38314e3a314e3e314e3c314e40314e42314e44314e46314e48314e4e314e50314e4a314e4c314e52314e56314e58314e54314e5a314e5c314e5e314e60314e64314e6c314e62314e6a314e66314e68314e6e314e70314e72314e7c314e74314e76314e78314e7a314e7e314e80314e86314e82314e84314e88314e8a314e8c314e8e314e92244e90314e94314e96314e98314e9a314e9c314ea0314e9e314ea2314ea4264ea6174ea8314eaa314eac274e")\
            X(310 , 1, 0x04F, KeyO               , "O"                 , "o"   , 0x6F  , 111  , 'u', 'o'   , '\x0f', "00184f06184f10184f0e184f14184f16184f02184f0a184f0c184f18184f04184f08184f1a184f12184f20184f1e184f1c184f22184f24184f28184f26184f2a184f30184f2e184f2c184f34184f36184f32184f38184f3a184f3e184f3c184f40184f42184f44184f46184f48184f4e184f50184f4a184f4c184f52184f56184f58184f54184f5a184f5c184f5e184f60184f64184f6c184f62184f6a184f66184f68184f6e184f70184f72184f7c184f74184f76184f78184f7a184f7e184f80184f86184f82184f84184f88184f8a184f8c184f8e184f92274f90184f94184f96184f98184f9a184f9c184fa0184f9e184fa2184fa41f4fa6144fa8144faa174fac134f")\
            X(312 , 1, 0x050, KeyP               , "P"                 , "p"   , 0x70  , 112  , 'u', 'p'   , '\x10', "0019500619501019500e19501419501619500219500a19500c19501819500419500819501a19501219502019501e19501c19502219502419502819502619502a19503019502e19502c19503419503619503219503819503a19503e19503c19504019504219504419504619504819504e19505019504a19504c19505219505619505819505419505a19505c19505e19506019506419506c19506219506a19506619506819506e19507019507219507c19507419507619507819507a19507e19508019508619508219508419508819508a19508c19508e19509213509019509419509619509819509a19509c1950a019509e1950a21950a41350a61950a80a50aa0550ac1250")\
            X(314 , 1, 0x051, KeyQ               , "Q"                 , "q"   , 0x71  , 113  , 'u', 'q'   , '\x11', "0010510610511010510e10511410511610510210510a10510c10511810510410510810511a10511210512010511e10511c10512210512410512810512610512a10513010512e10512c10513410513610513210513810513a10513e10513c10514010514210514410514610514810514e10515010514a10514c10515210515610515810515410515a10515c10515e10516010516410516c10516210516a10516610516810516e10517010517210517c10517410517610517810517a10517e10518010518610518210518410518810518a10518c10518e1051921051901051941051961e51981e519a1e519c1e51a01e519e1e51a22bbfa42d51a61a51a81251aa1151ac3251")\
            X(316 , 1, 0x052, KeyR               , "R"                 , "r"   , 0x72  , 114  , 'u', 'r'   , '\x12', "0013520613521013520e13521413521613520213520a13520c13521813520413520813521a13521213522013521e13521c13522213522413522813522613522a13523013522e13522c13523413523613523213523813523a13523e13523c13524013524213524413524613524813524e13525013524a13524c13525213525613525813525413525a13525c13525e13526013526413526c13526213526a13526613526813526e13527013527213527c13527413527613527813527a13527e13528013528613528213528413528813528a13528c13528e1352921f529013529413529613529813529a13529c1352a013529e1352a21352a41852a61652a81552aa1552ac2652")\
            X(318 , 1, 0x053, KeyS               , "S"                 , "s"   , 0x73  , 115  , 'u', 's'   , '\x13', "001f53061f53101f530e1f53141f53161f53021f530a1f530c1f53181f53041f53081f531a1f53121f53201f531e1f531c1f53221f53241f53281f53261f532a1f53301f532e1f532c1f53341f53361f53321f53381f533a1f533e1f533c1f53401f53421f53441f53461f53481f534e1f53501f534a1f534c1f53521f53561f53581f53541f535a1f535c1f535e1f53601f53641f536c1f53621f536a1f53661f53681f536e1f53701f53721f537c1f53741f53761f53781f537a1f537e1f53801f53861f53821f53841f53881f538a1f538c1f538e1f53922053901f53941f53961f53981f539a1f539c1f53a01f539e1f53a21f53a42753a63253a81653aa1653ac2553")\
            X(320 , 1, 0x054, KeyT               , "T"                 , "t"   , 0x74  , 116  , 'u', 't'   , '\x14', "0014540614541014540e14541414541614540214540a14540c14541814540414540814541a14541214542014541e14541c14542214542414542814542614542a14543014542e14542c14543414543614543214543814543a14543e14543c14544014544214544414544614544814544e14545014544a14544c14545214545614545814545414545a14545c14545e14546014546414546c14546214546a14546614546814546e14547014547214547c14547414547614547814547a14547e14548014548614548214548414548814548a14548c14548e14549221549014549414549614549814549a14549c1454a014549e1454a21454a42554a62354a82454aa2254ac2454")\
            X(322 , 1, 0x055, KeyU               , "U"                 , "u"   , 0x75  , 117  , 'u', 'u'   , '\x15', "0016550616551016550e16551416551616550216550a16550c16551816550416550816551a16551216552016551e16551c16552216552416552816552616552a16553016552e16552c16553416553616553216553816553a16553e16553c16554016554216554416554616554816554e16555016554a16554c16555216555616555816555416555a16555c16555e16556016556416556c16556216556a16556616556816556e16557016557216557c16557416557616557816557a16557e16558016558616558216558416558816558a16558c16558e16559217559016559416559616559816559a16559c1655a016559e1655a21655a42155a61e55a81755aa1455ac1f55")\
            X(324 , 1, 0x056, KeyV               , "V"                 , "v"   , 0x76  , 118  , 'u', 'v'   , '\x16', "002f56062f56102f560e2f56142f56162f56022f560a2f560c2f56182f56042f56082f561a2f56122f56202f561e2f561c2f56222f56242f56282f56262f562a2f56302f562e2f562c2f56342f56362f56322f56382f563a2f563e2f563c2f56402f56422f56442f56462f56482f564e2f56502f564a2f564c2f56522f56562f56582f56542f565a2f565c2f565e2f56602f56642f566c2f56622f566a2f56662f56682f566e2f56702f56722f567c2f56742f56762f56782f567a2f567e2f56802f56862f56822f56842f56882f568a2f568c2f568e2f56922f56902f56942f56962f56982f569a2f569c2f56a02f569e2f56a22f56a43456a62e56a83356aa2f56ac1656")\
            X(326 , 1, 0x057, KeyW               , "W"                 , "w"   , 0x77  , 119  , 'u', 'w'   , '\x17', "0011570611571011570e11571411571611570211570a11570c11571811570411570811571a11571211572011571e11571c11572211572411572811572611572a11573011572e11572c11573411573611573211573811573a11573e11573c11574011574211574411574611574811574e11575011574a11574c11575211575611575811575411575a11575c11575e11576011576411576c11576211576a11576611576811576e11577011577211577c11577411577611577811577a11577e11578011578611578211578411578811578a11578c11578e1157921157901157941157962c57982c579a2c579c2c57a02c579e2c57a21bbaa43357a61b57a83257aa3057ac1b57")\
            X(328 , 1, 0x058, KeyX               , "X"                 , "x"   , 0x78  , 120  , 'u', 'x'   , '\x18', "002d58062d58102d580e2d58142d58162d58022d580a2d580c2d58182d58042d58082d581a2d58122d58202d581e2d581c2d58222d58242d58282d58262d582a2d58302d582e2d582c2d58342d58362d58322d58382d583a2d583e2d583c2d58402d58422d58442d58462d58482d584e2d58502d584a2d584c2d58522d58562d58582d58542d585a2d585c2d585e2d58602d58642d586c2d58622d586a2d58662d58682d586e2d58702d58722d587c2d58742d58762d58782d587a2d587e2d58802d58862d58822d58842d58882d588a2d588c2d588e2d58922d58902d58942d58962d58982d589a2d589c2d58a02d589e2d58a20ddba43058a62b58a82e58aa2d58ac2e58")\
            X(330 , 1, 0x059, KeyY               , "Y"                 , "y"   , 0x79  , 121  , 'u', 'y'   , '\x19', "0015590615591015590e15591415591615590215590a15590c15591815590415590815591a15591215592015591e15591c15592215592415592815592615592a15593015592e15592c15593415593615593215593815593a15593e15593c1559402c594215594415594615594815594e15595015594a15594c15595215595615595815595415595a15595c15595e15596015596415596c15596215596a15596615596815596e2c59701559722c597c2c59742c59762c59782c597a2c597e2c59802c59862c59822c59842c59882c598a2c598c15598e1559921859902c59942c599615599815599a15599c1559a015599e1559a21559a41459a62759a81859aa1359ac2d59")\
            X(332 , 1, 0x05A, KeyZ               , "Z"                 , "z"   , 0x7A  , 122  , 'u', 'z'   , '\x1a', "002c5a062c5a102c5a0e2c5a142c5a162c5a022c5a0a2c5a0c2c5a182c5a042c5a082c5a1a2c5a122c5a202c5a1e2c5a1c2c5a222c5a242c5a282c5a262c5a2a2c5a302c5a2e2c5a2c2c5a342c5a362c5a322c5a382c5a3a2c5a3e2c5a3c2c5a40155a422c5a442c5a462c5a482c5a4e2c5a502c5a4a2c5a4c2c5a522c5a562c5a582c5a542c5a5a2c5a5c2c5a5e2c5a602c5a642c5a6c2c5a622c5a6a2c5a662c5a682c5a6e155a702c5a72155a7c155a74155a76155a78155a7a155a7e155a80155a86155a82155a84155a88155a8a155a8c2c5a8e2c5a922c5a90155a94155a96115a98115a9a115a9c115aa0115a9e115aa22c5aa4355aa6315aa8205aaa265aac1a5a")\
            X(334 , 1, 0    , AeLigature         , "Æ"                 , "æ"   , 0xE6  , 230  , 'u', -1    , -1    , "4e27c04a28de6027c06c27c06628de7027c0")\
            X(336 , 1, 0    , AcuteA             , "Á"                 , "á"   , 0xE1  , 225  , 'u', -1    , -1    , "7e28de8e28de")\
            X(338 , 1, 0    , BreveA             , "Ă"                 , "ă"   , 0x0103, 259  , 'u', -1    , -1    , "381adb941adb")\
            X(340 , 1, 0    , CircumflexA        , "Â"                 , "â"   , 0xE2  , 226  , 'u', -1    , -1    , "382bdc942bdc")\
            X(342 , 1, 0    , GraveA             , "À"                 , "à"   , 0xE0  , 224  , 'u', -1    , -1    , "3e2bdc422bdc5a28de5c28de8828dcac2cdd")\
            X(344 , 1, 0    , OgonekA            , "Ą"                 , "ą"   , 0x0105, 261  , 'u', -1    , -1    , "7228dea21051")\
            X(346 , 1, 0    , RingA              , "Å"                 , "å"   , 0xE5  , 229  , 'u', -1    , -1    , "441add4e1add4a1add4c1add601add6c1add621add661add")\
            X(348 , 1, 0    , TildeA             , "Ã"                 , "ã"   , 0xE3  , 227  , 'u', -1    , -1    , "9829de")\
            X(350 , 1, 0    , UmlautA            , "Ä"                 , "ä"   , 0xE4  , 228  , 'u', -1    , -1    , "4428de4c28de5628bf581bdd6228de741bdd7628de7828de7a28de8028de8628de8228de8428de8a28dc")\
            X(352 , 1, 0    , AcuteC             , "Ć"                 , "ć"   , 0x0107, 263  , 'u', -1    , -1    , "7c28de9028de")\
            X(354 , 1, 0    , CaronC             , "Č"                 , "č"   , 0x010D, 269  , 'u', -1    , -1    , "7c27ba9027baa233bc")\
            X(356 , 1, 0    , CedillaC           , "Ç"                 , "ç"   , 0xE7  , 231  , 'u', -1    , -1    , "3e1bdd3c27ba401adb421bdd5428de5e27c0682bbf8c34dca630bfac2bdc")\
            X(358 , 1, 0    , DotAboveC          , "Ċ"                 , "ċ"   , 0x010B, 267  , 'u', -1    , -1    , "3429c03629c0")\
            X(360 , 1, 0    , AcuteE             , "É"                 , "é"   , 0xE9  , 233  , 'u', -1    , -1    , "3a35bf3e35bf4235bf7e27ba8827de8e27baac11ba")\
            X(362 , 1, 0    , CircumflexE        , "Ê"                 , "ê"   , 0xEA  , 234  , 'u', -1    , -1    , "ac56e2")\
            X(364 , 1, 0    , GraveE             , "È"                 , "è"   , 0xE8  , 232  , 'u', -1    , -1    , "3e28c04228c05a1aba5c1aba881abaac14bf")\
            X(366 , 1, 0    , DotAboveE          , "Ė"                 , "ė"   , 0x0117, 279  , 'u', -1    , -1    , "a228de")\
            X(368 , 1, 0    , OgonekE            , "Ę"                 , "ę"   , 0x0119, 281  , 'u', -1    , -1    , "a235bd")\
            X(370 , 1, 0    , UmlautE            , "Ë"                 , "ë"   , 0xEB  , 235  , 'u', -1    , -1    , "4027ba")\
            X(372 , 1, 0    , CrossedD           , "Đ"                 , "đ"   , 0x0111, 273  , 'u', -1    , -1    , "7c1bdd901bdd")\
            X(374 , 1, 0    , BreveG             , "Ğ"                 , "ğ"   , 0x011F, 287  , 'u', -1    , -1    , "8c1adba612ba")\
            X(376 , 1, 0    , DotAboveG          , "Ġ"                 , "ġ"   , 0x0121, 289  , 'u', -1    , -1    , "341adb361adb")\
            X(378 , 1, 0    , CrossedH           , "Ħ"                 , "ħ"   , 0x0127, 295  , 'u', -1    , -1    , "341bdd361bdd")\
            X(380 , 1, 0    , AcuteI             , "Í"                 , "í"   , 0xED  , 237  , 'u', -1    , -1    , "7e56e28e29308e56e2")\
            X(382 , 1, 0    , CircumflexI        , "Î"                 , "î"   , 0xEE  , 238  , 'u', -1    , -1    , "381bdd941bdd")\
            X(384 , 1, 0    , GraveI             , "Ì"                 , "ì"   , 0xEC  , 236  , 'u', -1    , -1    , "5a0ddd5c0ddd")\
            X(386 , 1, 0    , OgonekI            , "Į"                 , "į"   , 0x012F, 303  , 'u', -1    , -1    , "a21add")\
            X(388 , 1, 0    , CrossedL           , "Ł"                 , "ł"   , 0x0142, 322  , 'u', -1    , -1    , "7227ba762bbf7a2bbf")\
            X(390 , 1, 0    , CaronN             , "Ň"                 , "ň"   , 0x0148, 328  , 'u', -1    , -1    , "582bdc742bdc")\
            X(392 , 1, 0    , TildeN             , "Ñ"                 , "ñ"   , 0xF1  , 241  , 'u', -1    , -1    , "5027c05427c06a27c06827c0")\
            X(394 , 1, 0    , AcuteO             , "Ó"                 , "ó"   , 0xF3  , 243  , 'u', -1    , -1    , "722bdc7e0dbb8e0dbb981bba")\
            X(396 , 1, 0    , CircumflexO        , "Ô"                 , "ô"   , 0xF4  , 244  , 'u', -1    , -1    , "5827ba7427ba")\
            X(398 , 1, 0    , DoubleAcuteO       , "Ő"                 , "ő"   , 0x0151, 337  , 'u', -1    , -1    , "7e1adb8e1adb")\
            X(400 , 1, 0    , GraveO             , "Ò"                 , "ò"   , 0xF2  , 242  , 'u', -1    , -1    , "5a27c05c27c0")\
            X(402 , 1, 0    , HornO              , "Ơ"                 , "ơ"   , 0x01A1, 417  , 'u', -1    , -1    , "321bdd")\
            X(404 , 1, 0    , SlashedO           , "Ø"                 , "ø"   , 0xF8  , 248  , 'u', -1    , -1    , "4e28de4a27c06028de6c28de6627c0")\
            X(406 , 1, 0    , TildeO             , "Õ"                 , "õ"   , 0xF5  , 245  , 'u', -1    , -1    , "561bdb")\
            X(408 , 1, 0    , UmlautO            , "Ö"                 , "ö"   , 0xF6  , 246  , 'u', -1    , -1    , "4427c04c27c05627ba6227c0700cbb7627c07827c07a27c08027c08627c08227c08427c08a27de8c33bfa62ddc")\
            X(410 , 1, 0    , AcuteS             , "Ś"                 , "ś"   , 0x015B, 347  , 'u', -1    , -1    , "721bdd")\
            X(412 , 1, 0    , CaronS             , "Š"                 , "š"   , 0x0161, 353  , 'u', -1    , -1    , "7c1adb901adba22146")\
            X(414 , 1, 0    , CedillaS           , "Ş"                 , "ş"   , 0x015F, 351  , 'u', -1    , -1    , "8c27ba9427baa628de")\
            X(416 , 1, 0    , CommaS             , "Ș"                 , "ș"   , 0x0219, 537  , 'u', -1    , -1    , "3827ba")\
            X(418 , 1, 0    , CedillaT           , "Ţ"                 , "ţ"   , 0x0163, 355  , 'u', -1    , -1    , "9428de")\
            X(420 , 1, 0    , CommaT             , "Ț"                 , "ț"   , 0x021B, 539  , 'u', -1    , -1    , "3828de")\
            X(422 , 1, 0    , AcuteU             , "Ú"                 , "ú"   , 0xFA  , 250  , 'u', -1    , -1    , "521adb581adb6e1adb741adb7e1bdd8e1bdd")\
            X(424 , 1, 0    , DoubleAcuteU       , "Ű"                 , "ű"   , 0x0171, 369  , 'u', -1    , -1    , "7e2bdc8e2bdc")\
            X(426 , 1, 0    , GraveU             , "Ù"                 , "ù"   , 0xF9  , 249  , 'u', -1    , -1    , "3e56e24256e25a2bbf5c2bbf9628c09828c09c28c0a028c09e28c0")\
            X(428 , 1, 0    , HornU              , "Ư"                 , "ư"   , 0x01B0, 432  , 'u', -1    , -1    , "321adb")\
            X(430 , 1, 0    , MacronU            , "Ū"                 , "ū"   , 0x016B, 363  , 'u', -1    , -1    , "a22d58")\
            X(432 , 1, 0    , OgonekU            , "Ų"                 , "ų"   , 0x0173, 371  , 'u', -1    , -1    , "a227c0")\
            X(434 , 1, 0    , RingU              , "Ů"                 , "ů"   , 0x016F, 367  , 'u', -1    , -1    , "5227ba6e27ba")\
            X(436 , 1, 0    , UmlautU            , "Ü"                 , "ü"   , 0xFC  , 252  , 'u', -1    , -1    , "561ac0761aba781aba7a1aba7e0cbf801aba861aba821aba841aba8a1aba8c1bdd8e0cbda622dd")\
            X(438 , 1, 0    , CaronZ             , "Ž"                 , "ž"   , 0x017E, 382  , 'u', -1    , -1    , "1e0dbb7c2bdc902bdca21157")\
            X(440 , 1, 0    , DotAboveZ          , "Ż"                 , "ż"   , 0x017C, 380  , 'u', -1    , -1    , "342bdc3656e2721adb")\
            X(442 , 0, 0x15F, Sleep              , "Sleep"             , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(444 , 0, 0x1B6, AppStart1          , "AppStart1"         , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(446 , 0, 0x1B7, AppStart2          , "AppStart2"         , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(448 , 0, 0    , AppNewWindow       , "AppNewWindow"      , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(450 , 0, 0    , AppOpenWindow      , "AppOpenWindow"     , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(452 , 0, 0x02F, AppHelp            , "AppHelp"           , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(454 , 0, 0    , AppSave            , "AppSave"           , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(456 , 0, 0    , AppFind            , "AppFind"           , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(458 , 0, 0    , AppPrint           , "AppPrint"          , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(460 , 0, 0    , AppClose           , "AppClose"          , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(462 , 1, 0    , AppCut             , "AppCut"            , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(464 , 0, 0    , AppCopy            , "AppCopy"           , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(466 , 1, 0    , AppPaste           , "AppPaste"          , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(468 , 1, 0    , AppUndo            , "AppUndo"           , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(470 , 1, 0    , AppRedo            , "AppRedo"           , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(472 , 0, 0    , AppSpeechMode      , "AppSpeechMode"     , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(474 , 0, 0    , AppSpeechCorrection, "AppSpeechCorrect"  , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(476 , 0, 0    , AppSpellCheck      , "AppSpellCheck"     , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(478 , 0, 0    , Calculator         , "Calculator"        , ""    , 0     , 0    , 'u', -1    , -1    , "0121b70721b71121b70f21b71521b71721b70321b70b21b70d21b71921b70521b70921b71b21b71321b72121b71f21b71d21b72321b72521b72921b72721b72b21b73121b72f21b72d21b73521b73721b73321b73921b73b21b73f21b73d21b74121b74321b74521b74721b74921b74f21b75121b74b21b74d21b75321b75721b75921b75521b75b21b75d21b75f21b76121b76521b76d21b76321b76b21b76721b76921b76f21b77121b77321b77d21b77521b77721b77921b77b21b77f21b78121b78721b78321b78521b78921b78b21b78d21b78f21b79321b79121b79521b79721b79921b79b21b79d21b7a121b79f21b7a321b7a521b7a721b7a921b7ab21b7ad21b7")\
            X(480 , 0, 0x1B4, Mail               , "Mail"              , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(482 , 0, 0    , MailSend           , "MailSend"          , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(484 , 0, 0    , MailForward        , "MailForward"       , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(486 , 0, 0    , MailReply          , "MailReply"         , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(488 , 0, 0    , MediaBassBoost     , "MediaBassBoost"    , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(490 , 0, 0    , MediaBassDown      , "MediaBassDown"     , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(492 , 0, 0    , MediaBassUp        , "MediaBassUp"       , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(494 , 0, 0    , MediaChanDown      , "MediaChanDown"     , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(496 , 0, 0    , MediaChanUp        , "MediaChanUp"       , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(498 , 0, 0    , MediaTrebleDown    , "MediaTrebleDown"   , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(500 , 0, 0    , MediaTrebleUp      , "MediaTrebleUp"     , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(502 , 0, 0x1AD, MediaVolMute       , "MediaVolMute"      , ""    , 0     , 57440, 'u', -1    , -1    , "0120ad0720ad1120ad0f20ad1520ad1720ad0320ad0b20ad0d20ad1920ad0520ad0920ad1b20ad1320ad2120ad1f20ad1d20ad2320ad2520ad2920ad2720ad2b20ad3120ad2f20ad2d20ad3520ad3720ad3320ad3920ad3b20ad3f20ad3d20ad4120ad4320ad4520ad4720ad4920ad4f20ad5120ad4b20ad4d20ad5320ad5720ad5920ad5520ad5b20ad5d20ad5f20ad6120ad6520ad6d20ad6320ad6b20ad6720ad6920ad6f20ad7120ad7320ad7d20ad7520ad7720ad7920ad7b20ad7f20ad8120ad8720ad8320ad8520ad8920ad8b20ad8d20ad8f20ad9320ad9120ad9520ad9720ad9920ad9b20ad9d20ada120ad9f20ada320ada520ada720ada920adab20adad20ad")\
            X(504 , 0, 0x1AE, MediaVolDown       , "MediaVolDown"      , ""    , 0     , 57438, 'u', -1    , -1    , "012eae072eae112eae0f2eae152eae172eae032eae0b2eae0d2eae192eae052eae092eae1b2eae132eae212eae1f2eae1d2eae232eae252eae292eae272eae2b2eae312eae2f2eae2d2eae352eae372eae332eae392eae3b2eae3f2eae3d2eae412eae432eae452eae472eae492eae4f2eae512eae4b2eae4d2eae532eae572eae592eae552eae5b2eae5d2eae5f2eae612eae652eae6d2eae632eae6b2eae672eae692eae6f2eae712eae732eae7d2eae752eae772eae792eae7b2eae7f2eae812eae872eae832eae852eae892eae8b2eae8d2eae8f2eae932eae912eae952eae972eae992eae9b2eae9d2eaea12eae9f2eaea32eaea52eaea72eaea92eaeab2eaead2eae")\
            X(506 , 0, 0x1AF, MediaVolUp         , "MediaVolUp"        , ""    , 0     , 57439, 'u', -1    , -1    , "0130af0730af1130af0f30af1530af1730af0330af0b30af0d30af1930af0530af0930af1b30af1330af2130af1f30af1d30af2330af2530af2930af2730af2b30af3130af2f30af2d30af3530af3730af3330af3930af3b30af3f30af3d30af4130af4330af4530af4730af4930af4f30af5130af4b30af4d30af5330af5730af5930af5530af5b30af5d30af5f30af6130af6530af6d30af6330af6b30af6730af6930af6f30af7130af7330af7d30af7530af7730af7930af7b30af7f30af8130af8730af8330af8530af8930af8b30af8d30af8f30af9330af9130af9530af9730af9930af9b30af9d30afa130af9f30afa330afa530afa730afa930afab30afad30af")\
            X(508 , 0, 0x1B0, MediaNext          , "MediaNext"         , ""    , 0     , 57435, 'u', -1    , -1    , "0119b00719b01119b00f19b01519b01719b00319b00b19b00d19b01919b00519b00919b01b19b01319b02119b01f19b01d19b02319b02519b02919b02719b02b19b03119b02f19b02d19b03519b03719b03319b03919b03b19b03f19b03d19b04119b04319b04519b04719b04919b04f19b05119b04b19b04d19b05319b05719b05919b05519b05b19b05d19b05f19b06119b06519b06d19b06319b06b19b06719b06919b06f19b07119b07319b07d19b07519b07719b07919b07b19b07f19b08119b08719b08319b08519b08919b08b19b08d19b08f19b09319b09119b09519b09719b09919b09b19b09d19b0a119b09f19b0a319b0a519b0a719b0a919b0ab19b0ad19b0")\
            X(510 , 0, 0x1B1, MediaPrev          , "MediaPrev"         , ""    , 0     , 57436, 'u', -1    , -1    , "0110b10710b11110b10f10b11510b11710b10310b10b10b10d10b11910b10510b10910b11b10b11310b12110b11f10b11d10b12310b12510b12910b12710b12b10b13110b12f10b12d10b13510b13710b13310b13910b13b10b13f10b13d10b14110b14310b14510b14710b14910b14f10b15110b14b10b14d10b15310b15710b15910b15510b15b10b15d10b15f10b16110b16510b16d10b16310b16b10b16710b16910b16f10b17110b17310b17d10b17510b17710b17910b17b10b17f10b18110b18710b18310b18510b18910b18b10b18d10b18f10b19310b19110b19510b19710b19910b19b10b19d10b1a110b19f10b1a310b1a510b1a710b1a910b1ab10b1ad10b1")\
            X(512 , 0, 0x1B2, MediaStop          , "MediaStop"         , ""    , 0     , 57432, 'u', -1    , -1    , "0124b20724b21124b20f24b21524b21724b20324b20b24b20d24b21924b20524b20924b21b24b21324b22124b21f24b21d24b22324b22524b22924b22724b22b24b23124b22f24b22d24b23524b23724b23324b23924b23b24b23f24b23d24b24124b24324b24524b24724b24924b24f24b25124b24b24b24d24b25324b25724b25924b25524b25b24b25d24b25f24b26124b26524b26d24b26324b26b24b26724b26924b26f24b27124b27324b27d24b27524b27724b27924b27b24b27f24b28124b28724b28324b28524b28924b28b24b28d24b28f24b29324b29124b29524b29724b29924b29b24b29d24b2a124b29f24b2a324b2a524b2a724b2a924b2ab24b2ad24b2")\
            X(514 , 0, 0    , MediaPause         , "MediaPause"        , ""    , 0     , 57429, 'u', -1    , -1    , "")\
            X(516 , 0, 0x1B3, MediaPlayPause     , "MediaPlayPause"    , ""    , 0     , 57430, 'u', -1    , -1    , "0122b30722b31122b30f22b31522b31722b30322b30b22b30d22b31922b30522b30922b31b22b31322b32122b31f22b31d22b32322b32522b32922b32722b32b22b33122b32f22b32d22b33522b33722b33322b33922b33b22b33f22b33d22b34122b34322b34522b34722b34922b34f22b35122b34b22b34d22b35322b35722b35922b35522b35b22b35d22b35f22b36122b36522b36d22b36322b36b22b36722b36922b36f22b37122b37322b37d22b37522b37722b37922b37b22b37f22b38122b38722b38322b38522b38922b38b22b38d22b38f22b39322b39122b39522b39722b39922b39b22b39d22b3a122b39f22b3a322b3a522b3a722b3a922b3ab22b3ad22b3")\
            X(518 , 0, 0    , MediaPlay          , "MediaPlay"         , ""    , 0     , 57428, 'u', -1    , -1    , "")\
            X(520 , 0, 0x1B5, MediaSelectMode    , "MediaSelectMode"   , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(522 , 0, 0    , MediaReverse       , "MediaReverse"      , ""    , 0     , 57431, 'u', -1    , -1    , "")\
            X(524 , 0, 0    , MediaRecord        , "MediaRecord"       , ""    , 0     , 57437, 'u', -1    , -1    , "")\
            X(526 , 0, 0    , MediaFastForward   , "MediaFastForward"  , ""    , 0     , 57433, 'u', -1    , -1    , "")\
            X(528 , 0, 0    , MediaRewind        , "MediaRewind"       , ""    , 0     , 57434, 'u', -1    , -1    , "")\
            X(530 , 0, 0    , MicAirToggle       , "MicAirToggle"      , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(532 , 0, 0    , MicMute            , "MicMute"           , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(534 , 0, 0    , MicVolUp           , "MicVolUp"          , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(536 , 0, 0    , MicVolDown         , "MicVolDown"        , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(538 , 0, 0x1A6, BrowserBackward    , "BrowserBackward"   , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(540 , 0, 0x1A7, BrowserForward     , "BrowserForward"    , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(542 , 0, 0x1A8, BrowserRefresh     , "BrowserRefresh"    , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(544 , 0, 0x1A9, BrowserStop        , "BrowserStop"       , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(546 , 0, 0x1AA, BrowserSearch      , "BrowserSearch"     , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(548 , 0, 0x1AB, BrowserFavorites   , "BrowserFavorites"  , ""    , 0     , 0    , 'u', -1    , -1    , "")\
            X(550 , 0, 0x1AC, BrowserHome        , "BrowserHome"       , ""    , 0     , 0    , 'u', -1    , -1    , "0132ac0732ac1132ac0f32ac1532ac1732ac0332ac0b32ac0d32ac1932ac0532ac0932ac1b32ac1332ac2132ac1f32ac1d32ac2332ac2532ac2932ac2732ac2b32ac3132ac2f32ac2d32ac3532ac3732ac3332ac3932ac3b32ac3f32ac3d32ac4132ac4332ac4532ac4732ac4932ac4f32ac5132ac4b32ac4d32ac5332ac5732ac5932ac5532ac5b32ac5d32ac5f32ac6132ac6532ac6d32ac6332ac6b32ac6732ac6932ac6f32ac7132ac7332ac7d32ac7532ac7732ac7932ac7b32ac7f32ac8132ac8732ac8332ac8532ac8932ac8b32ac8d32ac8f32ac9332ac9132ac9532ac9732ac9932ac9b32ac9d32aca132ac9f32aca332aca532aca732aca932acab32acad32ac")\
            X(552 , 0, 0    , lastKey            , "lastKey"           , ""    , 0     , 0    , 0  , -1    , -1    , "")

            // Max 12 bits for KeyId.
            static constexpr auto idbits = 12;

        #define X(KeyId, Input, Vk, Name, Generic, Literal, Uc, KKPdef, KKPsuffix, KKPascii, wCtl, PhysicalCode) \
            static constexpr auto Name = KeyId;
            key_list
        #undef X

        static constexpr auto fx_map = []
        {
            auto m = std::array<si16, 512>{};
            auto fill = [&](si32 KeyId, si32 Vk)
            {
                if (Vk > (si32)m.size() || m[Vk]) log("The Vk value is duplicated or incorrect (vk=%%)", Vk); /* It won't compile if broken. */\
                if (Vk) m[Vk] = (si16)KeyId;
            };
            #define X(KeyId, Input, Vk, Name, Generic, Literal, Uc, KKPdef, KKPsuffix, KKPascii, wCtl, PhysicalCode) \
                fill(KeyId, Vk);
                key_list
            #undef X
            return m;
        }();
        static constexpr auto key_map = []
        {
            auto m = std::array<si16, 65536>{};
            auto fill = [&](si32 KeyId, qiew codes)
            {
                while (codes)
                {
                    auto hash = codes.pop_front(6);
                    auto key_hash = utf::to_int_from_hex_str(hash) >> 8;
                    if (m[key_hash]) log("Key %KeyId% is duplicated (hash=%hash%)", KeyId, hash); // It won't compile if collide.
                    m[key_hash] = (si16)KeyId;
                }
            };
            #define X(KeyId, Input, Vk, Name, Generic, Literal, Uc, KKPdef, KKPsuffix, KKPascii, wCtl, PhysicalCode) \
                fill(KeyId, qiew{ PhysicalCode });
                key_list
            #undef X
            return m;
        }();
        static constexpr auto vk_map = []
        {
            struct keyrec
            {
                si16 code;
                si16 scan;
                utfx unic;
                si16 klid; // Sorted klid index.
                si16 vkey;
                struct cmp
                {
                    auto operator()(keyrec const& r, si32 vkey) const { return r.vkey < vkey; }
                    auto operator()(si32 vkey, keyrec const& r) const { return vkey < r.vkey; }
                };
            };
            constexpr auto total_hash_count = []
            {
                auto total_hash_count = 0;
                #define X(KeyId, Input, Vk, Name, Generic, Literal, Uc, KKPdef, KKPsuffix, KKPascii, wCtl, PhysicalCode) \
                    total_hash_count += (si32)qiew{ PhysicalCode }.size() / 6;
                    key_list
                #undef X
                return total_hash_count;
            }();
            auto m = std::array<keyrec, total_hash_count>{};
            auto i = 0;
            auto fill = [&](si16 KeyId, utfx unic, qiew codes)
            {
                while (codes)
                {
                    auto key_hash = utf::to_int_from_hex_str(codes.pop_front(6));
                    auto vkey = (si16)(key_hash & 0xFF);
                    auto scan = (si16)((key_hash >> 8) & 0x1FF);
                    auto klid = (si16)(key_hash >> 17);
                    m[i++] = { .code = KeyId, .scan = scan, .unic = unic, .klid = klid, .vkey = vkey };
                }
            };
            #define X(KeyId, Input, Vk, Name, Generic, Literal, Uc, KKPdef, KKPsuffix, KKPascii, wCtl, PhysicalCode) \
                fill(KeyId, Uc, qiew{ PhysicalCode });
                key_list
            #undef X
            std::sort(m.begin(), m.end(), [](auto& a, auto& b)
            {
                if (a.vkey != b.vkey) return a.vkey < b.vkey;
                else                  return a.klid < b.klid;
            });
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
            #define X(KeyId, Input, Vk, Name, Generic, Literal, Uc, KKPdef, KKPsuffix, KKPascii, wCtl, PhysicalCode) \
                { map{ 0,0,0,0 }, KeyId },
                key_list
            #undef X
        };

        static const auto _init0 = []
        {
            //todo move it to std::array
            #define X(KeyId, Input, Vk, Name, Generic, Literal, Uc, KKPdef, KKPsuffix, KKPascii, wCtl, PhysicalCode) \
                map::set(KeyId, Input, #Name, Generic, KKPdef, KKPsuffix, KKPascii, wCtl, PhysicalCode);
                key_list
            #undef X
            return true;
        }();
        static const auto kkpmap = std::unordered_map<si32, si32>
        {
            #define X(KeyId, Input, Vk, Name, Generic, Literal, Uc, KKPdef, KKPsuffix, KKPascii, wCtl, PhysicalCode) \
                { KKPdef | (KKPsuffix << 16), KeyId },
                key_list
            #undef X
        };
        static const auto specific_names = utf::unordered_map<text, si32>
        {
            #define X(KeyId, Input, Vk, Name, Generic, Literal, Uc, KKPdef, KKPsuffix, KKPascii, wCtl, PhysicalCode) \
                { utf::to_lower(#Name), KeyId },
                key_list
            #undef X
        };
        static const auto generic_names = utf::unordered_map<text, si32>
        {
            #define X(KeyId, Input, Vk, Name, Generic, Literal, Uc, KKPdef, KKPsuffix, KKPascii, wCtl, PhysicalCode) \
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

    // input: Tooltip tracker.
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
            RAlt         = 1 <<  3, // Right ⎇ Alt, Right ⌥ Option
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
            //           = 1 << 15,
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
                if (s & hids::LCtrl && s & hids::RAlt) // Reset AltGr emulation on win32.
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
                        auto& ctls = *++(it_shift->second.rbegin());
                        ctls = '1';
                        if (shift) ctls += 1;
                        if (alt  ) ctls += 2;
                        if (ctrl ) ctls += 4;
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

            void reset(auto& k, bool full = true)
            {
                k.vkchord.clear();
                k.scchord.clear();
                k.chchord.clear();
                k.shifted.clear();
                k.unshift.clear();
                if (full)
                {
                    pushed.clear();
                    keyout = {};
                }
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
            static auto _vkey_str(si32 keyid, bool ispressed)
            {
                keyid &= 0x0FFF; // 12 bit max.
                auto keyid_str = text(2, '\0');
                keyid_str[0] = (byte)((keyid >> 8) | (ispressed ? 0x00 : input::key::unpressed_sign));
                keyid_str[1] = (byte)(keyid & 0xFF);
                return keyid_str;
            }
            static void push_keyid(bool ispressed, text& vkchord, si32 keyid)
            {
                vkchord += input::key::kmap::_vkey_str(keyid, ispressed);
            }
            static auto pressed(auto& k, si32 keyid)
            {
                auto pressed_keyid = input::key::kmap::_vkey_str(keyid, true);
                return k.vkchord.find(pressed_keyid) != text::npos;
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
            void build(auto& k, P test_if_key_released = {})
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
                            auto is_released = test_if_key_released(val.vcode); // Check if it is still pressed.
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
                        auto has_cluster = k.cluster.size() && k.cluster.front();
                        auto has_unshift = k.unshift.size() && valid_codepoint(k.unshift) && !shift_state;
                        auto has_shifted = k.shifted.size() && valid_codepoint(k.shifted) && shift_state;
                        if (has_cluster || has_unshift || has_shifted) // Try to keep national key names.
                        {
                            k.chchord = k.vkchord; // The main part of the chchord is the same as in vkchord.
                                 if (has_cluster) push_cluster(sign, k.chchord, k.cluster);
                            else if (has_unshift) push_cluster(sign, k.chchord, k.unshift);
                            else                  push_cluster(sign, k.chchord, k.shifted);
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

        void fix_altgr_and_right_shift(si32& vk, si32 sc, bool& extflag, bool fake_ralt) // Set extflag for right shift.
        {
            if (fake_ralt && sc == input::key::map::data(input::key::RightAlt).scan)
            {
                vk = 0x5E; // Use unassigned VK_ for AltGr.
                extflag = faux;
            }
            else if (vk == input::vkey::shift && sc == input::key::map::data(input::key::RightShift).scan)
            {
                extflag = true;
            }
        }
        auto xlat(si32 vk, si32 sc, bool extflag, si32 xlayout, si32 layout_fallback, si32& layout_hint)
        {
            auto keyid = key::undef;
            fix_altgr_and_right_shift(vk, sc, extflag, faux);
            auto vk_ex = (vk & 0xFF) | (extflag << 8);
            if (auto keycode = input::key::fx_map[vk_ex]) // Fast detection of function keys.
            {
                keyid = keycode;
                if constexpr (debugmode) log("Fast detection of function keys: ", keyid);
            }
            else if (xlayout)
            {
                auto klid = input::key::is_layout_supported(xlayout) ? xlayout
                                                   : layout_fallback ? layout_fallback
                                                                     : input::key::latin_klids[0];
                auto hash = input::key::key_hash(klid, sc, extflag);
                keyid = (si32)input::key::key_map[hash];
            }
            else
            {
                sc |= extflag << 8;
                auto new_layout_hint = layout_hint;
                using keyrec = std::decay_t<decltype(input::key::vk_map[0])>;
                auto [head, tail] = std::equal_range(input::key::vk_map.begin(), input::key::vk_map.end(), vk, keyrec::cmp{});
                while (head != tail)
                {
                    auto& r = *head++;
                    if (r.scan == sc)
                    {
                        if (new_layout_hint == layout_hint && r.klid != layout_hint) // Best effort.
                        {
                            keyid = r.code;
                            new_layout_hint = r.klid;
                            if (layout_hint == -1) break; // If there are no hints, then a match with the scancode is enough.
                        }
                        else if (r.klid == layout_hint) // Exact match.
                        {
                            keyid = r.code;
                            new_layout_hint = r.klid;
                            break;
                        }
                    }
                }
                layout_hint = new_layout_hint;
            }
            return keyid;
        }
        auto xlat_direct(si32 vk, si32 sc, bool extflag, bool fake_ralt, si32 xlayout, si32 layout_fallback)
        {
            auto keyid = key::undef;
            fix_altgr_and_right_shift(vk, sc, extflag, fake_ralt);
            auto vk_ex = (vk & 0xFF) | (extflag << 8);
            if (auto keycode = input::key::fx_map[vk_ex]) // Function keys fast detection.
            {
                keyid = keycode;
                if constexpr (debugmode) log("Function keys fast detection: ", keyid);
            }
            else if (xlayout)
            {
                auto klid = input::key::is_layout_supported(xlayout) ? xlayout
                                                   : layout_fallback ? layout_fallback
                                                                     : input::key::latin_klids[0];
                auto hash = input::key::key_hash(klid, sc, extflag);
                keyid = (si32)input::key::key_map[hash];
            }
            return keyid;
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