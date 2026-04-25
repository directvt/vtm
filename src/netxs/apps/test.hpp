// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

namespace netxs::events::userland
{
    namespace test
    {
        EVENTPACK( app::test::events, netxs::events::userland::seed::custom )
        {
            GROUP_XS( ui, input::hids ),

            SUBSET_XS( ui )
            {
                EVENT_XS( create  , input::hids ),
                GROUP_XS( split   , input::hids ),

                SUBSET_XS( split )
                {
                    EVENT_XS( hz, input::hids ),
                };
            };
        };
    }
}

// test: Test window.
namespace netxs::app::test
{
    static constexpr auto id = "test";
    static constexpr auto name = "Text Layout Test (DEMO)";

    namespace events = netxs::events::userland::test;

    namespace
    {
        enum test_topic_vars
        {
            object1,
            object2,
            object3,

            canvas1,
            canvas2,

            dynamix1,
            dynamix2,
            dynamix3,

            image1,
            image2,
        };

        template<auto ...Args>
        constexpr auto vss = utf::matrix::vss<Args...>;

        auto test_page = [](auto hdrclr, auto txtclr)
        {
            auto header = [&](auto caption)
            {
                return ansi::mgl(1).wrp(wrap::off).fgc(hdrclr).unc(whitedk).cap(caption).erl().und(unln::none).eol().fgc(txtclr).mgl(3).unc(0).wrp(wrap::on);
            };
            auto header2 = [&](auto caption)
            {
                    return ansi::escx{}.cap(caption, 2, 2, faux).erl().eol();
            };
            auto crop = ansi::mgl(1).mgr(2).jet(bias::center)
                .add("\n")
                .wrp(wrap::off).fgc(hdrclr).cap(skin::globals().NsInfoSF, 3, 3, faux).eol()
                .jet(bias::left)
                .add("\n");
                auto o = 11; // Offset.
                crop.add(header("AnyPlex Protocol, APP"))
                    .add("\n\n\n\n")
                    .jet(bias::left).wrp(wrap::off)
                    .jet(bias::center)
                    .mgr(4 + o).mgl(3 - o).fgc(txtclr).add("any                   \n")
                    .mgl(1 - o).mgr(4 + o).cuu(1).fgc(hdrclr).add(header2("AnyPlex"))
                    .mgl(12 - o).mgr(-3 + o).cuu(1).fgc(txtclr).add("plex\n")
                    .nil().nop().cuu(5)
                    .jet(bias::left).mgl(16 - o)
                    .idx(test_topic_vars::image1).add("<").fgc(reddk).add("placeholder").fgc().add(">").nop()
                    .jet(bias::right).mgr(-3 + o)
                    .idx(test_topic_vars::image2).add("<").fgc(reddk).add("placeholder").fgc().add(">").nop()
                    .jet(bias::left).wrp(wrap::off).mgl(1).mgr(2)
                    .add("\n")
                .add(header(skin::globals().NsInfoSubcellSize))
                .add("\n")
                .add("😉", vss<11>) // Color emoji font load trigger.
                .add("\2Cell", utf::vs10, utf::vs08, vss<11>)
                .add("        ")
                .add("\2Word", utf::vs10, vss<11>)
                .add("        ")
                .add("\2Cell", utf::vs07, utf::vs05, vss<21>)
                .add("        ")
                .add("\2Word", utf::vs07, vss<21>)
                .add("        ")
                .eol()
                .bgc(greendk)
                .add("\2Cell", utf::vs10, utf::vs08, vss<11>)
                .nil().add("        ")
                .bgc(magentadk)
                .add("\2Word", utf::vs10, vss<11>)
                .nil().add("        ")
                .bgc(reddk)
                .add("\2Cell", utf::vs07, utf::vs05, utf::vs08, vss<21>)
                .nil().add("        ")
                .bgc(yellowdk).fgc(blackdk)
                .add("\2Word", utf::vs07, utf::vs08, vss<21>)
                .nil().add("        ")
                .eol()
                .add("\2Cell", utf::vs10, utf::vs08, vss<11>)
                .add("\2Word", utf::vs10, vss<11>)
                .add("\2Cell", utf::vs07, utf::vs05, vss<21>)
                .add("\2Word", utf::vs12, vss<11>)
                .eol()
                .add("\n")
                .add(header("Powerline"))
                .add("\n")
                .jet(bias::left).wrp(wrap::off).fgc(whitelt).bgc(bluelt).add("  INSERT  ").fgc(bluelt).bgc(blacklt).add("\uE0B0").fgc(whitelt).add(" \uE0A0 master ").fgc(blacklt).bgc(argb{}).add("\uE0B0   ")
                .fgc(whitelt).add("Powerline test   ").chx(0).jet(bias::right).fgc(blacklt).add("\uE0B2").fgc(whitelt).bgc(blacklt).add(" [dos] ").fgc(bluelt).add("\uE0B2").fgc(whitelt).bgc(bluelt).add(" 100% \uE0A1    2:  1 \n").bgc(argb{})
                .add("\n").nop().nil().jet(bias::left).wrp(wrap::on)
                .add(header(skin::globals().NsInfoLatin))
                .add("\n")
                .add("ANSI sequences were introduced in the 1970s to replace vendor-specific sequences and became "
                    "widespread in the computer equipment market by the early 1980s. They were used in development, "
                    "scientific and commercial applications and later by the nascent bulletin board systems "
                    "to offer improved displays compared to earlier systems lacking cursor movement, "
                    "a primary reason they became a standard adopted by all manufacturers.\n")
                .add("\n")
                .add(header(skin::globals().NsInfoCJK))
                .add("\n")
                .add("CJK文字是對中文、日文文字和韓文的統稱，這些語言全部含有汉字及其變體，"
                     "某些會與其他文字混合使用。因為越南文曾經使用漢字，所以它有時候與CJK文字結合，"
                     "組成CJKV文字（英語：Chinese-Japanese-Korean-Vietnamese）。概括來說，"
                     "CJKV文字通常包括中文的漢字、日文文字的日本汉字及日語假名、"
                     "韓文的朝鮮漢字及諺文和越南文的儒字和喃字。\n")
                .add("\n")
                .add(header(skin::globals().NsInfoThai))
                .add("\n")
                .add("มวยไทย​เป็น​กีฬา​ประจำ​ชาติ​ไทย​ นัก​มวยไทย​มัก​จะ​เป็น​แช​ม​เปีย​นระ​ดับ​ไลท์เวท​ของ​สมาคม​มวย​โลก​เสมอ ​"
                     "ปลาย​คริสต์​ศตวรรษ​ที่​ 19​ ประเทศไทย​รับ​เอา​กีฬา​จาก​ชาติ​ตะวัน​ตก​เข้า​มา​หลาย​ชนิด​ "
                     "โดย​เริ่ม​มี​การ​แข่งขัน​ใน​โรงเรียน​ใน​ต้น​คริสต์​ศตวรรษ​ที่​ 20​ ตาม​มา​ด้วย​ใน​ระบบ​การ​ศึกษา​สมัย​ใหม่\n")
                .add("\n")
                .add(header(skin::globals().NsInfoGeorgian))
                .add("\n")
                .add("ქართული ენა — ქართველურ ენათა ოჯახის ენა. ქართველების მშობლიური ენა, საქართველოს სახელმწიფო ენა. რამდენიმე ავტორი ძველი კოლხეთის ენას, როგორც უძველეს ქართულ ენას, გენეტიკურად უკავშირებდა ეგვიპტურ ენას.\n")
                .add("\n")
                .add(header(skin::globals().NsInfoDevanagari))
                .add("\n")
                .add("\2अनुच्छेद", vss<51>, " १.\n"     // अनुच्छेद १.
                    "\2सभी", vss<31>, " \2मनुष्यों", vss<41>, " को", vss<21>, " \2गौरव", vss<31>, " \2और", vss<31>, " \2अधिकारों", vss<61>, " के", vss<21>, " \2मामले", vss<41>, " में "  // सभी मनुष्यों को गौरव और अधिकारों के मामले में
                    "\2जन्मजात", vss<51>, " \2स्वतन्त्रता", vss<51>, " \2और", vss<31>, " \2समानता", vss<51>, " \2प्राप्त", vss<31>, " \2है।", vss<21>, "\n" // जन्मजात स्वतन्त्रता और समानता प्राप्त है।
                    "\2उन्हें", vss<31>, " \2बुद्धि", vss<31>, " \2और", vss<31>, " \2अन्तरात्मा", vss<61>, " की", vss<21>, " \2देन", vss<21>, " \2प्राप्त", vss<31>, " है \2और", vss<31>, " " // उन्हें बुद्धि और अन्तरात्मा की देन प्राप्त है और
                    "\2परस्पर", vss<41>, " \2उन्हें", vss<31>, " \2भाईचारे", vss<51>, " के", vss<21>, " \2भाव", vss<31>, " से \2बर्ताव ", vss<41>, " \2करना", vss<31>, " \2चाहिए।", vss<41>, "\n") // परस्पर उन्हें भाईचारे के भाव से बर्ताव करना चाहिए।
                .add("\n").jet(bias::right)
                .add(header(skin::globals().NsInfoArabic))
                .add("\n").rtl(rtol::rtl)
                .add("\n")
                .arabic("يولد جميع الناس أحرارًا متساوين في الكرامة والحقوق. وقد وهبوا عقلاً وضميرًا وعليهم أن يعامل بعضهم بعضًا بروح الإخاء.")
                .add("\n")
                .add("\n").rtl(rtol::ltr)
                .add(header(skin::globals().NsInfoHebrew))
                .add("\n").rtl(rtol::rtl)
                .add("\n")
                .add("עִבְרִית היא שפה שמית, ממשפחת השפות האפרו-אסייתיות, הידועה כשפתם של היהודים ושל השומרונים. היא שייכת למשפחת השפות הכנעניות והשפה הכנענית היחידה המדוברת כיום.\n")
                .add("\n").rtl(rtol::ltr).jet(bias::left)
                .add(header(skin::globals().NsInfoEmoji))
                .add("\n")
                .add("😀😃😄😁😆😅😂🤣😊😇🙂🙃😉😌😍😺"
                     "😏😒😞😔😟😕😣😖😫😩🥺😢😭😤😸😹"
                     "🥰😘😗😙😚😋😛😝😜🤪🤨🧐🤓😎🤩🥳"
                     "😡🤬🤯😳🥵🥶😱😨😰😥😓🤗🤔🤭🤫🤥"
                     "😶😐😑😬🙄😯😦😧😮😲🥱😴🤤😪😵🤐"
                     "🥴🤢🤮🤧😷🤒🤕🤑🤠😈👿👹👺🤡💩👻"
                     "💀👽👾🤖🎃😺😸😹😻😼😽🙀😿😾😠😍\n")
                .add("\n")
                .add("😀😃😄😁😆😅😂🤣😊😇🙂🙃😉😌😍😺\n"
                     "😏😒😞😔😟😕😣😖😫😩🥺😢😭😤😸😹\n"
                     "🥰😘😗😙😚😋😛😝😜🤪🤨🧐🤓😎🤩🥳\n"
                     "😡🤬🤯😳🥵🥶😱😨😰😥😓🤗🤔🤭🤫🤥\n"
                     "😶😐😑😬🙄😯😦😧😮😲🥱😴🤤😪😵🤐\n"
                     "🥴🤢🤮🤧😷🤒🤕🤑🤠😈👿👹👺🤡💩👻\n"
                     "💀👽👾🤖🎃😺😸😹😻😼😽🙀😿😾😠😍\n")
                .add("\n")
                .add(header(skin::globals().NsInfoBoxDrawing))
                .add("                                                                             \n"
                     "╔══╦══╗  ┌──┬──┐  ╭──┬──╮  ╭──┬──╮  ┏━━┳━━┓  ┎┒┏┑   ╷  ╻ ┏┯┓ ┌┰┐    █ ╱╲╱╲╳╳╳ \n"
                     "║┌─╨─┐║  │╔═╧═╗│  │╒═╪═╕│  │╓─╁─╖│  ┃┌─╂─┐┃  ┗╃╄┙  ╶┼╴╺╋╸┠┼┨ ┝╋┥    ▉ ╲╱╲╱╳╳╳ \n"
                     "║│╲ ╱│║  │║   ║│  ││ │ ││  │║ ┃ ║│  ┃│ ╿ │┃  ┍╅╆┓   ╵  ╹ ┗┷┛ └┸┘    ▊ ╱╲╱╲╳╳╳ \n"
                     "╠╡ ╳ ╞╣  ├╢   ╟┤  ├┼─┼─┼┤  ├╫─╂─╫┤  ┣┿╾┼╼┿┫  ┕┛┖┚     ┌┄┄┐ ╎ ┏┅┅┓ ┋ ▋ ╲╱╲╱╳╳╳ \n"
                     "║│╱ ╲│║  │║   ║│  ││ │ ││  │║ ┃ ║│  ┃│ ╽ │┃  ░░▒▒▓▓██ ┊  ┆ ╎ ╏  ┇ ┋ ▌         \n"
                     "║└─╥─┘║  │╚═╤═╝│  │╘═╪═╛│  │╙─╀─╜│  ┃└─╂─┘┃  ░░▒▒▓▓██ ┊  ┆ ╎ ╏  ┇ ┋ ▍         \n"
                     "╚══╩══╝  └──┴──┘  ╰──┴──╯  ╰──┴──╯  ┗━━┻━━┛           └╌╌┘ ╎ ┗╍╍┛ ┋ ▎▁▂▃▄▅▆▇█ \n"
                     "                                                                    ▏          \n")
                .add(header(skin::globals().NsInfoLargeTypePieces))
                .add("\n")
                .add("𜸜 𜸜𜸚𜸟𜸤𜸜𜸝𜸢𜸜𜸚𜸟𜸤  𜸜  𜸚𜸟𜸤𜸛𜸟𜸤𜸚𜸟𜸤𜸛𜸟𜸥  𜸞𜸠𜸥𜸜 𜸜𜸛𜸟𜸤𜸛𜸟𜸥  𜸛𜸟𜸤𜸜𜸛𜸟𜸥𜸚𜸟𜸤𜸛𜸟𜸥𜸚𜸟𜸤\n"
                     "𜸩 𜸩𜸾𜸟𜸤𜸩𜸩𜸫𜸹𜸩 𜸧  𜸩  𜸨𜸟𜸶𜸨𜸟𜸷𜸩 𜸧𜸨𜸟    𜸩 𜸫𜸳𜸻𜸨𜸟𜹃𜸨𜸟   𜸨𜸟𜹃𜸩𜸨𜸟 𜸩  𜸨𜸟 𜸾𜸟𜸤\n"
                     "𜸾𜸟𜹃𜸾𜸟𜹃𜸼𜸼 𜸼𜸾𜸟𜹃  𜸽𜸟𜸥𜸼 𜸼𜸼 𜸼𜸾𜸟𜹃𜸽𜸟𜸥   𜸼  𜸼 𜸼  𜸽𜸟𜸥  𜸼  𜸼𜸽𜸟𜸥𜸾𜸟𜹃𜸽𜸟𜸥𜸾𜸟𜹃\n")
                .add("\n")
                .add(header(skin::globals().NsInfoStyledUnderline))
                .add("\n")
                .add(" ").ovr(true)        .add(skin::globals().NsInfoSingleOverline).ovr(faux).eol()
                .add(" ").und(unln::biline).add(skin::globals().NsInfoDoubleUnderline).und(unln::none).eol()
                .add(" ").und(unln::line  ).add(skin::globals().NsInfoSingleUnderline).und(unln::none).eol()
                .add(" ").und(unln::dashed).add(skin::globals().NsInfoDashedUnderline).und(unln::none).eol()
                .add(" ").und(unln::dotted).add(skin::globals().NsInfoDottedUnderline).und(unln::none).eol()
                .add(" ").und(unln::wavy  ).add(skin::globals().NsInfoWavyUnderline).und(unln::none).eol()
                //.add(" ").und(unln::wavy  ).unc(argb{ puregreen }).add("Green Wavy Underline").und(unln::none).eol()
                //.add(" ").und(unln::line  ).unc(argb{ puregreen }).add("Green Single Underline").und(unln::none).eol()
                .add(" ").und(unln::line  ).unc(argb{ purewhite }).add(skin::globals().NsInfoWhiteSingleUnderline).und(unln::none).eol()
                .add(" ").und(unln::wavy  ).unc(argb{ purewhite }).add(skin::globals().NsInfoWhiteWavyUnderline).und(unln::none).eol()
                .add(" ").und(unln::line  ).unc(argb{ purered   }).add(skin::globals().NsInfoRedSingleUnderline).und(unln::none).eol()
                .add(" ").und(unln::wavy  ).unc(argb{ purered   }).add(skin::globals().NsInfoRedWavyUnderline).und(unln::none).eol()
                //.add(" ").und(unln::line  ).unc(argb{ pureblack }).add("Black Single Underline").und(unln::none).eol()
                //.add(" ").und(unln::wavy  ).unc(argb{ pureblack }).add("Black Wavy Underline").und(unln::none).eol()
                .nil()
                .add("\n")
                .add(header(skin::globals().NsInfoFontStyle))
                .add("\n")
                .bld(faux).itc(faux).add(skin::globals().NsInfoNormal).add("        ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890M韓M😎M 🥵🥵", vss<11>, "🦚😀⛷🏂😁😂😃😄😅😆👌🐞😎👪\n")
                                    .add("              abcdefghijklmnopqrstuvwxyz\n")
                .blk(true)          .add(skin::globals().NsInfoBlinking).add("      ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890M韓M😎M 🥵🥵", vss<11>, "🦚😀⛷🏂😁😂😃😄😅😆👌🐞😎👪\n")
                                    .add("              abcdefghijklmnopqrstuvwxyz\n")
                .bld(true).blk(faux).add(skin::globals().NsInfoBold).add("          ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890M韓M😎M 🥵🥵", vss<11>, "🦚😀⛷🏂😁😂😃😄😅😆👌🐞😎👪\n")
                                    .add("              abcdefghijklmnopqrstuvwxyz\n")
                .bld(true).itc(true).add(skin::globals().NsInfoBold).add(" + ").add(skin::globals().NsInfoItalic).add(" ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890M韓M😎M 🥵🥵", vss<11>, "🦚😀⛷🏂😁😂😃😄😅😆👌🐞😎👪\n")
                                    .add("              abcdefghijklmnopqrstuvwxyz\n")
                .bld(faux).itc(true).add("       ").add(skin::globals().NsInfoItalic).add(" ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890M韓M😎M 🥵🥵", vss<11>, "🦚😀⛷🏂😁😂😃😄😅😆👌🐞😎👪\n")
                                    .add("              abcdefghijklmnopqrstuvwxyz\n")
                .nil()
                .add("\n")
                .add(header(skin::globals().NsInfoCharacterWidth))
                .add("\n")
                .add(">👩‍👩‍👧‍👧", vss<11>, "<VS11_00  >👩‍👩‍👧‍👧", vss<21>, "<VS21_00  >👩‍👩‍👧‍👧", vss<31>, "<VS31_00  >👩‍👩‍👧‍👧", vss<41>, "<VS41_00", "  >\2अनुच्छेद", vss<51>, "<VS51_00\n")
                .add(">❤"  , vss<11>, "<VS11_00  >❤" , vss<21>, "<VS21_00\n")
                .add(">😎" , vss<11>, "<VS11_00  >😎" , vss<21>, "<VS21_00\n")
                .add("\n")
                .add(skin::globals().NsInfoLongestWord).add(": >\2विश्वविज्ञानकोशनिर्माणसमिति", vss<161>, "<VSG1_00 (16x1)\n")
                .add("\n")
                .add(header(skin::globals().NsInfoVariationSelectors + " VS15/16"))
                .add("\n")
                .add("Plain>❤<   VS15>❤︎<   VS16>❤️<\n")
                .add("Plain>🏴‍☠<  VS15>🏴‍☠︎<  VS16>🏴‍☠️<\n")
                .add("Plain>👩‍👩‍👧‍👧<  VS15>👩‍👩‍👧‍👧︎<  VS16>👩‍👩‍👧‍👧️<\n")
                .add("\n")
                //todo multiline graphemes
                //.add("\2line1\nline2", vss<52,01>, "\n")
                //.add("\2line1\nline2", vss<52,02>, "\n")
                //.add("\n")
                .add(header(skin::globals().NsInfoRotationFlipandMirror))
                .add("\n")
                .add("G", vss<21>,              "<Plain           ").add("\2G", utf::vs13, vss<21>,            "<VS13:      HzFlip           ").add("\2G", utf::vs14, vss<21>,            "<VS14:      VtFlip\n")
                .add("\2G", utf::vs10, vss<21>, "<VS10:  90°CCW   ").add("\2G", utf::vs13, utf::vs10, vss<21>, "<VS13+VS10: HzFlip+90°CCW    ").add("\2G", utf::vs14, utf::vs10, vss<21>, "<VS14+VS10: VtFlip+90°CCW\n")
                .add("\2G", utf::vs11, vss<21>, "<VS11: 180°CCW   ").add("\2G", utf::vs13, utf::vs11, vss<21>, "<VS13+VS11: HzFlip+180°CCW   ").add("\2G", utf::vs14, utf::vs11, vss<21>, "<VS14+VS11: VtFlip+180°CCW\n")
                .add("😎",  utf::vs12, vss<21>, "<VS12: 270°CCW   ").add("\2G", utf::vs13, utf::vs12, vss<21>, "<VS13+VS12: HzFlip+270°CCW   ").add("\2G", utf::vs14, utf::vs12, vss<21>, "<VS14+VS12: VtFlip+270°CCW\n")
                .add("\n")
                .add("\2G", utf::vs10, utf::vs13, vss<21>, "<VS10+VS13: 90°CCW+HzFlip\n")
                .add("\2G", utf::vs13, utf::vs10, vss<21>, "<VS13+VS10: HzFlip+90°CCW\n")
                .add("\n")
                .add("  \2Mirror", utf::vs13, vss<81>, "<VS13\n")
                .add("  \2Mirror", utf::vs14, vss<81>, "<VS14\n")
                .add("\n")
                .add(header(skin::globals().NsInfoCharacterMatrix))
                .add("\n")
                .fgc(blacklt).bgc(whitedk).add("\2Height", utf::vs05, utf::vs10, vss<24,11>).fgc(whitelt).bgc(blackdk).add("\2Height", utf::vs05, utf::vs10, vss<24,21>).bgc(argb{}).add("😎", vss<84,01>).fgc(txtclr).bgc(argb{}).add("\2Height", utf::vs05, utf::vs12, vss<24,01>).fgc(txtclr).add(" <VS84_00\n")
                .fgc(whitelt).bgc(blackdk).add("\2Height", utf::vs05, utf::vs10, vss<24,12>).fgc(blacklt).bgc(whitedk).add("\2Height", utf::vs05, utf::vs10, vss<24,22>).bgc(argb{}).add("😎", vss<84,02>).fgc(txtclr).bgc(argb{}).add("\2Height", utf::vs05, utf::vs12, vss<24,02>).add("\n")
                .fgc(blacklt).bgc(whitedk).add("\2Height", utf::vs05, utf::vs10, vss<24,13>).fgc(whitelt).bgc(blackdk).add("\2Height", utf::vs05, utf::vs10, vss<24,23>).bgc(argb{}).add("😎", vss<84,03>).fgc(txtclr).bgc(argb{}).add("\2Height", utf::vs05, utf::vs12, vss<24,03>).add("\n")
                .fgc(whitelt).bgc(blackdk).add("\2Height", utf::vs05, utf::vs10, vss<24,14>).fgc(blacklt).bgc(whitedk).add("\2Height", utf::vs05, utf::vs10, vss<24,24>).bgc(argb{}).add("😎", vss<84,04>).fgc(txtclr).bgc(argb{}).add("\2Height", utf::vs05, utf::vs12, vss<24,04>).add("\n")
                .add("  ").fgc(blacklt).bgc(whitedk).add("\2Width", utf::vs05, utf::vs11, vss<81,11>).fgc(whitelt).bgc(blackdk).add("\2Width", utf::vs05, utf::vs11, vss<81,21>).fgc(blacklt).bgc(whitedk).add("\2Width", utf::vs05, utf::vs11, vss<81,31>).fgc(whitelt).bgc(blackdk).add("\2Width", utf::vs05, utf::vs11, vss<81,41>)
                          .fgc(blacklt).bgc(whitedk).add("\2Width", utf::vs05, utf::vs11, vss<81,51>).fgc(whitelt).bgc(blackdk).add("\2Width", utf::vs05, utf::vs11, vss<81,61>).fgc(blacklt).bgc(whitedk).add("\2Width", utf::vs05, utf::vs11, vss<81,71>).fgc(whitelt).bgc(blackdk).add("\2Width", utf::vs05, utf::vs11, vss<81,81>)
                          .fgc(txtclr).bgc(argb{}).add("<VS11\n")
                .add("\n")
                .add("Advanced ").add("T", vss<22,01>, "e", vss<22,01>, "r", vss<22,01>, "m", vss<22,01>, "i", vss<22,01>, "n", vss<22,01>, "a", vss<22,01>, "l", vss<22,01>, "\n")
                .add("Terminal ").add("T", vss<22,02>, "e", vss<22,02>, "r", vss<22,02>, "m", vss<22,02>, "i", vss<22,02>, "n", vss<22,02>, "a", vss<22,02>, "l", vss<22,02>, "\n")
                .add("Emulator ").fgc(pureyellow).add("★", vss<21>, "★", vss<21>, "★", vss<21>, "★", vss<21>, "★", vss<21>, "★", vss<21>, "★", vss<21>).fgc(txtclr).add("☆", vss<21>, "\n")
                .add("\n")
                .add("😎", vss<42,01>, " <VS42_00\n")
                .add("😎", vss<42,02>, "\n")
                .add("\n")
                .add(header(skin::globals().NsInfoCharacterHalves))
                .add("\n")
                .add("😎", vss<21,11>, " 😃", vss<21,21>, "<VS21_11/VS21_21\n")
                .add("\n")
                .add(header(skin::globals().NsInfoTuiShadows))
                .add("\n")
                .add("\033[107;30m")
                .add(" \033[2:1m \033[2:3m \033[2:7m                \033[2:6m \033[2:4m \033[2:0m \n")
                .add(" \033[2:8m \033[2:0m                  \033[2:16m \033[2:0m ").add("\033[19D").add(skin::globals().NsInfoTuiShadowsOuter).add("\n")
                .add(" \033[2:32m \033[2:96m \033[2:224m                \033[2:192m \033[2:128m \033[2:0m \n")
                .add("  \033[2:247m \033[2:231m                \033[2:239m \033[2:0m  ").add("\033[19D\033[2:231m").add(skin::globals().NsInfoTuiShadowsInner).add("\n")
                .add("\033[2:0m                      \n")
                .add("\n");
                auto test = "     abde    abde     \n"
                          //"     f  g    f  g     \n"
                            "  abcpccoccccp  ocde  \n"
                            "  j          f  g  k  \n"
                            "  lmninnhnnnni  hnqr  \n"
                            "     f  g    f  g     \n"
                            "  abcp  occccpccocde  \n"
                            "  j  f  g          k  \n"
                            "  lmni  hnnnninnhnqr  \n"
                          //"     f  g    f  g     \n"
                            "     lmqr    lmqr     \n"s;
                utf::replace_all(test, "m", "\033[2:96m \033[2:0m");  // 𜺍𜹤w2
                utf::replace_all(test, "a", "\033[2:1m \033[2:0m");   // 𜺏𜹕q0
                utf::replace_all(test, "b", "\033[2:3m \033[2:0m");   // 𜹯𜹕q1
                utf::replace_all(test, "c", "\033[2:7m \033[2:0m");   // 𜹟𜹟q2
                utf::replace_all(test, "d", "\033[2:6m \033[2:0m");   // 𜹟𜹥q3
                utf::replace_all(test, "e", "\033[2:4m \033[2:0m");   // 𜹿𜹥q4
                utf::replace_all(test, "f", "\033[2:41m \033[2:0m");  // 𜺏𜹥q5
                utf::replace_all(test, "g", "\033[2:148m \033[2:0m"); // 𜹺𜺏q6
                utf::replace_all(test, "h", "\033[2:244m \033[2:0m"); // 𜹸𜺌w5
                utf::replace_all(test, "i", "\033[2:233m \033[2:0m"); // 𜺌𜹤w4
                utf::replace_all(test, "j", "\033[2:8m \033[2:0m");   // 𜺏𜹡q9
                utf::replace_all(test, "k", "\033[2:16m \033[2:0m");  // 𜺋𜹥w0
                utf::replace_all(test, "l", "\033[2:32m \033[2:0m");  // 𜺏𜹤w1
                utf::replace_all(test, "n", "\033[2:224m \033[2:0m"); // 𜺌𜺌w3
                utf::replace_all(test, "o", "\033[2:151m \033[2:0m"); // 𜹚𜹟q8
                utf::replace_all(test, "p", "\033[2:47m \033[2:0m");  // 𜹟𜹕q7
                utf::replace_all(test, "q", "\033[2:192m \033[2:0m"); // 𜺌𜹥w6
                utf::replace_all(test, "r", "\033[2:128m \033[2:0m"); // 𜺎𜹥w7
                crop.add(test, "\033[m\n");
            if constexpr (debugmode)
            {
                crop.add(header(skin::globals().NsInfosRGBBlending))
                .add("\n")
                .add(skin::globals().NsInfoPressCtrlCaps)
                .add("\n")
                .bgc(pureblue)
                .fgc(purered).add(" test \n")
                .fgc(puregreen).add(" test \n")
                .fgc(purecyan).bgc(purered).add(" test \n")
                .bgc(purewhite)
                .fgc(purered).add(" test \n")
                .fgc(purecyan).add(" test ")
                .bgc(argb{})
                .fgc(purered).add(" test \n")
                .fgc(purecyan).add(" test ");
            }
            return crop;
        };

        auto get_text = []
        {
            static auto topic = text{};

            if (topic.empty())
            {
                auto clr = 0xFFFFFFFF;
                auto msg = text{ "The quick brown fox jumps over the lazy dog." };
                auto msg_rtl = ansi::rtl(rtol::rtl).add("RTL: ", msg).nop().rtl(rtol::ltr);
                auto msg_ltr = text{ "LTR: " } + msg;
                auto testline = ansi::jet(bias::center).rtl(rtol::rtl)
                    .add("RTL: centered text.\n\n")
                    .rtl(rtol::ltr)
                    .add("centered text\n\n")
                    .add("another ").nop().fgc(redlt).add("inlined").nop().nil().add(" segment")
                    .jet(bias::left).bgc(blackdk)
                    .add(" affix \n\n")
                    .nil()
                    .add(msg_ltr, "\n\n",
                         msg_rtl, "\n\n")
                    .nop().jet(bias::right).rtl(rtol::ltr)
                    .add(msg_ltr, "\n\n", msg_rtl);

                auto margin = si32{ 4 };
                auto l1 = ansi::mgl(margin * 1).mgr(1).fgc(whitelt).und(true);
                auto l2 = ansi::mgl(margin * 2).mgr(1);
                auto l3 = ansi::mgl(margin * 3).mgr(1);
                auto c1 = bluelt;// 0xffff00;
                auto c2 = whitedk;//0xffffff;
                auto intro = ansi::mgl(0).mgr(0)
                    .add(" ")
                    //+ ansi::jet(bias::right).mgl(1).mgr(1).wrp(true)
                    //+ "https://github.com/directvt/vtm\n\n"
                    .jet(bias::center).wrp(wrap::off).fgc(whitelt).mgl(0).mgr(0).eol()
                    .fgc(c1).bgc(c2).add("▄")
                    .fgc(c2).bgc(c1).add("▄")
                    .fgc(clr).bgc().add("  Text-based Desktop Environment\n")
                    .fgc().bgc().add("Test Page    \n\n")

                    .nil().jet(bias::left).mgl(4).mgr(4).wrp(wrap::off)
                    .fgc(0xff000000).bgc(0xff00FF00).add(" ! ", "\n")
                    .cuu(1).chx(0).mgl(9).fgc().bgc().wrp(wrap::on)
                    .add("Test page for testing text rendering. \n"
                    "The following text doesn't make much sense, "
                    "it's just a bunch of text samples.\n"
                    "\n")
                    .jet(bias::center).wrp(wrap::off).fgc(whitelt).mgl(1).mgr(0)
                    .add(test_page(purewhite, purecyan))
                    .add("\n\n")
                    .mgl(1).fgc(purewhite).cap("User Interface Commands (outdated)").erl().und(unln::none).eol()
                    .wrp(wrap::off).fgc(whitelt).mgr(0)
                    .add("\n")
                    .fgc(whitelt).bld(true)
                    .add("Mouse:").nil().eol()
                    .add(l1).wrp(wrap::off)
                    .add("left").nil().eol()
                        .add(l2).fgc(blackdk).bgc(clr)
                        .add("click").nil().eol()
                        .add(l2).wrp(wrap::on)
                        .add("on the label in the upper left list of objects:\n"
                            "on the connecting line of an object:\n")
                            .add(l3, "- while holding down the Ctrl key, set/unset keyboard focus.\n")
                            .add(l3, "- move the viewport to the center of the object.\n")
                        .add(l2).fgc(blackdk).bgc(clr).wrp(wrap::off)
                        .add("drag ").nil().eol()
                        .add(l2).wrp(wrap::on)
                        .add("outside of any objects:\n")
                            .add(l3, "- panoramic navigation.\n")
                    .add(l1).wrp(wrap::off)
                    .add("right").nil().eol()
                        .add(l2).fgc(blackdk).bgc(clr).wrp(wrap::off)
                        .add("click").nil().eol()
                        .add(l2).wrp(wrap::on)
                        .add("on the connecting line of an object:\n")
                            .add(l3, "- move object to mouse cursor.\n")
                        .add(l2)
                        .add("outside of any objects:\n")
                            .add(l3, "- move the menu window to mouse cursor.\n")
                        .add(l2).fgc(blackdk).bgc(clr).wrp(wrap::off)
                        .add("drag ").nil().eol()
                        .add(l2).wrp(wrap::on)
                        .add("outside of any objects:\n")
                            .add(l3, "- while holding down the Ctrl key, copies the selected area to the clipboard.\n")
                            .add(l3, "- create a new object of the type selected in the menu (20 max).\n")
                        .add(l2)
                        .add("the scrollable content:\n")
                            .add(l3, "- scrolling and kinetic scrolling on release, like a mouse wheel, but faster and more convenient.\n")
                    .add(l1).wrp(wrap::off)
                    .add("middle").nil().eol()
                        .add(l2).fgc(blackdk).bgc(clr)
                        .add("click").nil().eol()
                        .add(l2).wrp(wrap::on)
                        .add("on the label in the upper left list of objects:\n")
                        .add("on the connecting line of an object:\n")
                        .add("on the object itself:\n")
                            .add(l3, "- destroy the object (except menu window).\n")
                        .add(l2).fgc(blackdk).bgc(clr).wrp(wrap::off)
                        .add("drag ").nil().eol()
                        .add(l2).wrp(wrap::on)
                        .add("inside the viewport:\n")
                            .add(l3, "- while holding down the Ctrl key, copies the selected area to the clipboard.\n")
                            .add(l3, "- create a new object of the type selected in the menu (20 max).\n")
                    .add(l1).wrp(wrap::off)
                    .add("left + right").nil().wrp(wrap::off).eol()
                        .add(l2).fgc(blackdk).bgc(clr).wrp(wrap::off)
                        .add("drag ").nil().wrp(wrap::on).eol()
                        .add("inside the viewport:\n")
                            .add(l3, "- panoramic navigation.\n")
                        .add(l2).fgc(blackdk).bgc(clr)
                        .add("click").nil().eol()
                        .add(l2).wrp(wrap::on)
                        .add("inside the object:\n")
                            .add(l3, "- destroy the object (except menu window).\n")
                    .add(l1).wrp(wrap::off)
                    .add("wheel").nil().wrp(wrap::off).eol()
                        .add(l2).fgc(blackdk).bgc(clr).wrp(wrap::off)
                        .add("scroll ").nil().wrp(wrap::on).eol()
                            .add(l3, "- vertical scrolling.\n")
                        .add(l2).wrp(wrap::on)
                        .add("scroll + ctrl\n")
                            .add(l3, "- horizontal scrolling.\n")
                        .add(l2)

                    .mgl(1).mgr(0)
                    .fgc(whitelt).bld(true).add("Keyboard:").nil().wrp(wrap::off).eol()
                    .add("    ").fgc(whitelt).und(true).add("Esc")             .nil().wrp(wrap::on).add(" - Quit/disconnect.\n")
                    .add("    ").fgc(whitelt).und(true).add("Ctrl")            .nil().wrp(wrap::on).add(" - Combine with the left mouse button to set/unset keyboard focus; combining with dragging right/middle mouse buttons copies the selected area to the clipboard.\n")
                    .add("    ").fgc(whitelt).und(true).add("Ctrl + PgUp/PgDn").nil().wrp(wrap::on).add(" - Navigation between windows.\n")
                    .eol()
                    .fgc(whitelt).bld(true).add("Taskbar menu:").nil().wrp(wrap::off).add(" (outdated)\n")
                    .add("    "            ).fgc(whitelt).und(true).add("Midnight Commander"  ).nil().wrp(wrap::off).add(" - live instance of Midnight Commander.\n")
                    .add("       "         ).fgc(whitelt).und(true).add("Truecolor image"     ).nil().wrp(wrap::off).add(" - true color ANSI/ASCII image.\n")
                    .add("          "      ).fgc(whitelt).und(true).add("Refresh Rate"        ).nil().wrp(wrap::off).add(" - terminal screen refresh rate selector (all users affected).\n")
                    .add("                ").fgc(whitelt).und(true).add("Strobe"              ).nil().wrp(wrap::off).add(" - an empty resizable window that changes background color every frame.\n")
                    .add("  "              ).fgc(whitelt).und(true).add("Recursive connection").nil().wrp(wrap::off).add(" - is limited to 3 connections in Demo mode.\n")
                    .eol()
                    .add("    ").fgc(whitelt).und(true).add("Disconnect").nil().wrp(wrap::off).add(" - Disconnect current user.\n")
                    .add("    ").fgc(whitelt).und(true).add("Shutdown"  ).nil().wrp(wrap::off).add("   - Disconnect all connected users and shutdown.\n\n")
                    .eol()
                    .wrp(wrap::on).mgl(0).mgr(0).eol()
                    .eol();

                auto data = ansi::nil()
                    .jet(bias::center).wrp(wrap::off).fgc(whitelt).add("Test Layouts\n\n")
                    .wrp(wrap::off).jet(bias::left).und(true)
                    .add("Text wrapping is OFF:\n\n").nil().wrp(wrap::off)
                    .add(testline)
                    .add("\n\n")

                    .wrp(wrap::off).jet(bias::left).und(true).fgc(whitelt)
                    .add("Text wrapping is ON:\n\n").nil().wrp(wrap::on)
                    .add(testline)
                    .add("\n\n")

                    .jet(bias::left)
                    .add("text: ").idx(test_topic_vars::object1).sav().fgc(whitelt).bgc(reddk)
                    .add("some_text").nop().nil().add(" some text\n\n") // inline text object test
                    .jet(bias::center)
                    .add("text: ").idx(test_topic_vars::object2).sav().fgc(whitelt).bgc(reddk)
                    .add("some_text").nop().nil().add(" some text\n\n") // inline text object test
                    .jet(bias::right)
                    .add("text: ").idx(test_topic_vars::object3).sav().fgc(whitelt).bgc(reddk)
                    .add("some_text").nop().nil().add(" some text\n\n") // inline text object test

                    .jet(bias::left).wrp(wrap::on)
                    .add("text text text ").idx(test_topic_vars::canvas1).wrp(wrap::on)
                    .nop().nil().add(" text text text")
                    .add("\n\n\n")

                    .jet(bias::center).wrp(wrap::off).fgc(clr)
                    .add("Variable-width/-height Characters\n\n")
                    .jet(bias::left).wrp(wrap::on).fgc()
                    .add("left aligned\n").ref(test_topic_vars::canvas2).nop()
                                          .ref(test_topic_vars::canvas2).nop()
                                          .ref(test_topic_vars::canvas2).nop()
                                          .ref(test_topic_vars::canvas2).nop()
                                          .ref(test_topic_vars::canvas2).nop()
                    .nop().nil().eol().eol()

                    .jet(bias::center).wrp(wrap::on)
                    .add("centered\n").ref(test_topic_vars::canvas2).nop()
                                      .ref(test_topic_vars::canvas2).nop()
                                      .ref(test_topic_vars::canvas2).nop()
                                      .ref(test_topic_vars::canvas2).nop()
                                      .ref(test_topic_vars::canvas2).nop()
                    .nop().nil().eol().eol()

                    .jet(bias::right).wrp(wrap::on)
                    .add("right aligned\n").ref(test_topic_vars::canvas2).nop()
                                           .ref(test_topic_vars::canvas2).nop()
                                           .ref(test_topic_vars::canvas2).nop()
                                           .ref(test_topic_vars::canvas2).nop()
                                           .ref(test_topic_vars::canvas2).nop()
                    .nop().nil().eol().eol()

                    .jet(bias::center).wrp(wrap::off).fgc(clr)
                    .add("Embedded Content\n\n")
                    .jet(bias::left).wrp(wrap::on)
                    .idx(test_topic_vars::dynamix1).add("<").fgc(reddk).add("placeholder").fgc().add(">")//.nop()
                    .idx(test_topic_vars::dynamix2).add("<").fgc(reddk).add("placeholder").fgc().add(">")//.nop()
                    .idx(test_topic_vars::dynamix3).add("<").fgc(reddk).add("placeholder").fgc().add(">")//.nop()
                    .nil().eol();


                // Wikipedia run
                auto wiki = escx("\n\n")
                    .jet(bias::center).wrp(wrap::off).fgc(clr)
                    .add("Sample Article\n\n")

                    .jet(bias::left).fgc(clr).wrp(wrap::on).add("ANSI escape code\n\n")

                    .nil().add("From Wikipedia, the free encyclopedia\n")
                    .add("  (Redirected from ANSI CSI)\n\n")

                    .jet(bias::center).itc(true)
                    .add("\"ANSI code\" redirects here.\n")
                    .add("For other uses, see ANSI (disambiguation).\n\n")

                    .jet(bias::left).itc(faux).fgc(clr).add("ANSI escape sequences").nil()
                    .add(" are a standard for ").fgc(clr).add("in-band signaling").nil()
                    .add(" to control the cursor location, color, and other options on video ")
                    .fgc(clr).add("text terminals").nil().add(" and ")
                    .fgc(clr).add("terminal emulators").nil().add(". Certain sequences of ")
                    .fgc(clr).add("bytes").nil().add(", most starting with ")
                    .fgc(clr).add("Esc").nil().add(" and '[', are embedded into the text, ")
                    .add("which the terminal looks for and interprets as commands, not as ")
                    .fgc(clr).add("character codes").nil().add(".\n\n")

                    .add("ANSI sequences were introduced in the 1970s to replace vendor-specific sequences "
                        "and became widespread in the computer equipment market by the early 1980s. "
                        "They were used in development, scientific and commercial applications and later by "
                        "the nascent ").fgc(clr).add("bulletin board systems").nil()
                    .add(" to offer improved displays ").ref(test_topic_vars::canvas2).nop()
                    .add("compared to earlier systems lacking cursor movement, "
                        "a primary reason they became a standard adopted by all manufacturers.\n\n")

                    .add("Although hardware text terminals have become increasingly rare in the 21st century, "
                        "the relevance of the ANSI standard persists because most terminal emulators interpret "
                        "at least some of the ANSI escape sequences in output text. A notable exception was ")
                    .fgc(clr).add("DOS").nil().add(" and older versions of the ")
                    .fgc(clr).add("Win32 console").nil().add(" of ")
                    .fgc(clr).add("Microsoft Windows").nil().add(".");

                auto wiki_ru = ansi::eol().eol()
                    .fgc(clr).add("Управляющие символы ANSI").nil()
                    .add(" (англ. ANSI escape code) — символы, встраиваемые в текст, для управления форматом,")
                    .add(" цветом и другими опциями вывода в ")
                    .fgc(clr).add("текстовом терминале").nil().add(".")
                    .add(" Почти все ").fgc(clr).add("эмуляторы терминалов").nil()
                    .add(", предназначенные для отображения текстового вывода с удалённого компьютера и (за исключением ")
                    .fgc(clr).add("Microsoft Windows").nil().add(") для отображения текстового вывода")
                    .add(" локального программного обеспечения, способны интерпретировать по крайней мере некоторые")
                    .add(" управляющие последовательности ANSI.");

                auto wiki_emoji = ansi::fgc(clr).add("\n\n")
                    .add("Emoji\n").nil()
                    .add("Emoji (Japanese: 絵文字, English: /ɪˈmoʊdʒiː/; Japanese: [emodʑi];"
                        " singular emoji, plural emoji or emojis) are ideograms and "
                        "smileys used in electronic messages and web pages. "
                        "Some examples of emoji are 😃, 😭, and 😈. Emoji exist "
                        "in various genres, including facial expressions, common objects, "
                        "places and types of weather, and animals. They are much like "
                        "emoticons, but emoji are pictures rather than typographic "
                        "approximations; the term \"emoji\" in the strict sense refers "
                        "to such pictures which can be represented as encoded characters, "
                        "but it is sometimes applied to messaging stickers by extension."
                        " Originally meaning pictograph, the word emoji comes from "
                        "Japanese e (絵, \"picture\") + moji (文字, \"character\"); "
                        "the resemblance to the English words emotion and emoticon is "
                        "purely coincidental. The ISO 15924 script code for emoji is Zsye."
                        "\n")
                    .fgc(clr).wrp(wrap::off).add("\nEmoji (wrap off)\n").nil()
                        .add("😀😃😄😁😆😅😂🤣😊😇🙂🙃😉😌😍😺\n"
                             "😏😒😞😔😟😕😣😖😫😩🥺😢😭😤😸😹\n"
                             "🥰😘😗😙😚😋😛😝😜🤪🤨🧐🤓😎🤩🥳\n"
                             "😡🤬🤯😳🥵🥶😱😨😰😥😓🤗🤔🤭🤫🤥\n"
                             "😶😐😑😬🙄😯😦😧😮😲🥱😴🤤😪😵🤐\n"
                             "🥴🤢🤮🤧😷🤒🤕🤑🤠😈👿👹👺🤡💩👻\n"
                             "💀👽👾🤖🎃😺😸😹😻😼😽🙀😿😾😠😍\n")
                    .fgc(clr).wrp(wrap::on).add("\nEmoji (wrap on)\n").nil()
                        .add("😀😃😄😁😆😅😂🤣😊😇🙂🙃😉😌😍😺"
                             "😏😒😞😔😟😕😣😖😫😩🥺😢😭😤😸😹"
                             "🥰😘😗😙😚😋😛😝😜🤪🤨🧐🤓😎🤩🥳"
                             "😡🤬🤯😳🥵🥶😱😨😰😥😓🤗🤔🤭🤫🤥"
                             "😶😐😑😬🙄😯😦😧😮😲🥱😴🤤😪😵🤐"
                             "🥴🤢🤮🤧😷🤒🤕🤑🤠😈👿👹👺🤡💩👻"
                             "💀👽👾🤖🎃😺😸😹😻😼😽🙀😿😾😠😍\n");

                auto wiki_cjk = ansi::wrp(wrap::off).fgc(clr).eol().eol()
                    .add("端末エミュレータ\n").nil()
                    .add("出典: フリー百科事典『ウィキペディア（Wikipedia）』\n\n")
                    .wrp(wrap::on).fgc(clr).add("端末エミュレータ").nil()
                    .add("（たんまつエミュレータ）")
                    .fgc(clr).add("・端末模倣プログラム").nil()
                    .add("とは、端末として動作するソフトウェアである。端末エミュレータといった場合は、"
                        "DEC VT100のエミュレーションをするソフトウェアをさすことが多い。別称と"
                        "して")
                    .fgc(clr).add("ターミナルエミュレータ").nil()
                    .add("、また特にグラフィカルユーザインタフェース "
                        "(GUI) 環境で用いるものを")
                    .fgc(clr).add("端末ウィンドウ").nil()
                    .add("と呼ぶことがある。キャラクタユーザインタフェースを提供する。\n\n")
                    .fgc(clr).add("概要\n").nil()
                    .add("端末エミュレータは、専用端末の機能を、パーソナルコンピュータ (PC) "
                        "やUnixワークステーションなどで代替するためのソフトウェア。通常はキャ"
                        "ラクタベースのビデオ端末をエミュレートするが、グラフィック端末（xterm"
                        "がTektronix 4014をエミュレートする）やプリンタのエミュレーションを行"
                        "うものもある。\n\n")
                    .add("端末エミュレータを動作させるコンピュータがウィンドウシステムを搭載して"
                        "いる場合、これを利用して一つのコンピュータ上で複数の端末エミュレータを"
                        "同時に稼働させることができることが多い。これは殆どの専用端末では実現で"
                        "きない機能である。\n\n")
                    .add("TCP / IPを介した端末エミュレータの接続にはSSH、Telnet、rlogin等の機"
                        "能を用いる。rloginとTelnetは、パスワードなども含めて、すべての通信内容"
                        "を平文（暗号化されていない状態）で送受信する。極めて限定された用途であれ"
                        "ば、それが必ずしも悪いわけではないが、インターネットを介した接続ではあま"
                        "りに危険な行為である。したがって、近年は、SSHによる接続が一般的である。\n\n")
                    .add("2015年ごろまで、Windows用のSSHクライアントは公式に提供されていなかっ"
                        "たため、端末エミュレータはSSHクライアントを統合したものが多かった。現在"
                        "ではOpenSSH in Windowsが提供されており、PowerShellやコマンドプロン"
                        "プトなどのコマンドラインツールから利用することができる。\n\n")
                    .fgc(clr).add("エミュレートする端末\n").nil()
                    .add("実際の端末における、画面制御やキーボード制御、プリンタ制御など、入出力処"
                        "理には統一された規格が存在しない。現在、端末エミュレータを使用する接続先"
                        "はUnixが多いため、Unixで事実上の標準となっているDEC社のVT100やその上位"
                        "機種のエミュレータが多い。VT100の端末エミュレータやその機能を「VT100互換"
                        "」と呼称する。\n\n"
                        "接続先がメインフレームであれば、IBM 3270、富士通、日立製作所の端末を、"
                        "接続先がIBM AS / 400であればIBM 5250を、エミュレートすることになる。"
                        "それぞれのメーカーから純正のエミュレータが発売されているが、サードパーティ"
                        "製もある。メインフレームの端末の多くは、RS - 232のような単純なシリアルイ"
                        "ンターフェースではなく、インテリジェントなものだったが、その後、シリアル接"
                        "続やイーサネット接続も可能となっている。"
                        "\n\n"
                        "多くの端末はキャラクタしか扱えないが、グラフィックを扱うことができるグラ"
                        "フィック端末もある。例えばxtermがエミュレートするTektronix 4014がその一"
                        "例で、キャラクタとグラフィックのどちらも扱うことができる。日本では、ヤマハ"
                        "のYIS(YGT - 100)もよく知られている。また、コンピュータグラフィックスの黎"
                        "明期には、多くのメインフレームにオプションとして専用のグラフィック端末が用意"
                        "されていた。\n");

                topic += intro;
                topic += data;
                topic += wiki;
                topic += wiki_ru;
                topic += wiki_emoji;
                topic += wiki_cjk;
            }

            return topic;
        };
        auto get_svg = []
        {
            return R"==(
<?xml version="1.0" encoding="UTF-8" standalone="no"?><!-- Created with Inkscape (http://www.inkscape.org/) --><svg xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://www.w3.org/2000/svg" xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd" xmlns:cc="http://web.resource.org/cc/" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:svg="http://www.w3.org/2000/svg" xmlns:ns1="http://sozi.baierouge.fr" id="svg2" sodipodi:docname="Nowy dokument.svg" sodipodi:modified="true" viewBox="0 0 256 256" sodipodi:version="0.32" version="1.0" inkscape:output_extension="org.inkscape.output.svg.inkscape" inkscape:version="0.44+devel" sodipodi:docbase="/home/michal"><defs id="defs18"><linearGradient id="linearGradient5839" inkscape:collect="always"><stop id="stop5841" style="stop-color:#eeeeee" offset="0"/><stop id="stop5843" style="stop-color:#eeeeee;stop-opacity:0" offset="1"/></linearGradient><linearGradient id="linearGradient6653"><stop id="stop6655" style="stop-color:#ffffff;stop-opacity:0" offset="0"/><stop id="stop6663" style="stop-color:#ffffff;stop-opacity:.49804" offset=".77863"/><stop id="stop6657" style="stop-color:#ffffff" offset="1"/></linearGradient><linearGradient id="linearGradient5738"><stop id="stop5740" style="stop-color:#fe0505" offset="0"/><stop id="stop5742" style="stop-color:#8b0000" offset="1"/></linearGradient><filter id="filter6757" inkscape:collect="always"><feGaussianBlur id="feGaussianBlur6759" stdDeviation="1.5101933" inkscape:collect="always"/></filter><filter id="filter6909" height="2.0016" width="1.4196" y="-.50078" x="-.20981" inkscape:collect="always"><feGaussianBlur id="feGaussianBlur6911" stdDeviation="3.1879868" inkscape:collect="always"/></filter><filter id="filter6933" height="1.238" width="1.1434" y="-.11898" x="-.071719" inkscape:collect="always"><feGaussianBlur id="feGaussianBlur6935" stdDeviation="0.74906281" inkscape:collect="always"/></filter>
)==" // MSVC2022: C2026 String too big, trailing characters truncated.
R"==(
<filter id="filter6020" inkscape:collect="always"><feGaussianBlur id="feGaussianBlur6022" stdDeviation="3.2891356" inkscape:collect="always"/></filter><filter id="filter5785" height="1.4503" width="1.394" y="-.22513" x="-.19698" inkscape:collect="always"><feGaussianBlur id="feGaussianBlur5787" stdDeviation="1.1143122" inkscape:collect="always"/></filter><filter id="filter5835" inkscape:collect="always"><feGaussianBlur id="feGaussianBlur5837" stdDeviation="1.7846046" inkscape:collect="always"/></filter><filter id="filter6021" height="1.2345" width="1.1447" y="-.11724" x="-.072367" inkscape:collect="always"><feGaussianBlur id="feGaussianBlur6023" stdDeviation="2.5861883" inkscape:collect="always"/></filter><filter id="filter6938" height="1.3991" width="1.2551" y="-.19955" x="-.12757" inkscape:collect="always"><feGaussianBlur id="feGaussianBlur6940" stdDeviation="3.222517" inkscape:collect="always"/></filter><filter id="filter7333" height="1.2395" width="1.2278" y="-.11973" x="-.11388" inkscape:collect="always"><feGaussianBlur id="feGaussianBlur7335" stdDeviation="1.3442502" inkscape:collect="always"/></filter><filter id="filter7450" inkscape:collect="always"><feGaussianBlur id="feGaussianBlur7452" stdDeviation="0.65095441" inkscape:collect="always"/></filter><filter id="filter7454" inkscape:collect="always"><feGaussianBlur id="feGaussianBlur7456" stdDeviation="0.42331528" inkscape:collect="always"/></filter><filter id="filter7576" height="1.5667" width="1.3339" y="-.28333" x="-.16697" inkscape:collect="always"><feGaussianBlur id="feGaussianBlur7578" stdDeviation="1.7783362" inkscape:collect="always"/></filter><filter id="filter7934" height="1.5445" width="1.6111" y="-.27223" x="-.30557" inkscape:collect="always"><feGaussianBlur id="feGaussianBlur7936" stdDeviation="26.49476" inkscape:collect="always"/></filter><filter id="filter7982" height="1.4267" width="1.479" y="-.21337" x="-.23950" inkscape:collect="always"><feGaussianBlur id="feGaussianBlur7984" stdDeviation="20.766163" inkscape:collect="always"/></filter>
)==" // MSVC2022: C2026 String too big, trailing characters truncated.
R"==(
<filter id="filter8192" inkscape:collect="always"><feGaussianBlur id="feGaussianBlur8194" stdDeviation="2.312615" inkscape:collect="always"/></filter><filter id="filter9241" inkscape:collect="always"><feGaussianBlur id="feGaussianBlur9243" stdDeviation="1.4137332" inkscape:collect="always"/></filter><linearGradient id="linearGradient9430" y2="283.8" gradientUnits="userSpaceOnUse" x2="-477.66" y1="206.34" x1="-426.19" inkscape:collect="always"><stop id="stop8154" style="stop-color:#000000" offset="0"/><stop id="stop8156" style="stop-color:#000000;stop-opacity:0" offset="1"/></linearGradient><linearGradient id="linearGradient9432" y2="424.72" xlink:href="#linearGradient5839" gradientUnits="userSpaceOnUse" x2="139.36" gradientTransform="translate(1.0607 .70711)" y1="427.55" x1="164.27" inkscape:collect="always"/><linearGradient id="linearGradient9434" y2="412.7" xlink:href="#linearGradient5839" gradientUnits="userSpaceOnUse" x2="245.62" gradientTransform="translate(1.7678 .70711)" y1="390.78" x1="268.22" inkscape:collect="always"/><radialGradient id="radialGradient9436" xlink:href="#linearGradient5738" gradientUnits="userSpaceOnUse" cy="520.01" cx="-113.91" gradientTransform="matrix(.91070 .36281 -.24003 .60251 132.1 251.08)" r="96.482" inkscape:collect="always"/><radialGradient id="radialGradient9438" xlink:href="#linearGradient6653" gradientUnits="userSpaceOnUse" cy="531.5" cx="-109.74" gradientTransform="matrix(1 0 0 .95148 0 25.787)" r="96.481" inkscape:collect="always"/><radialGradient id="radialGradient9440" fx="537.55" fy="461.21" xlink:href="#linearGradient5738" gradientUnits="userSpaceOnUse" cy="496.97" cx="529.47" gradientTransform="matrix(.86412 -.59392 .46267 .67316 -167.72 488.17)" r="86.418" inkscape:collect="always"/><linearGradient id="linearGradient9442" y2="513.33" gradientUnits="userSpaceOnUse" x2="501.59" y1="545.83" x1="439.81" inkscape:collect="always"><stop id="stop5982" style="stop-color:#220000" offset="0"/><stop id="stop5984" style="stop-color:#570000;stop-opacity:0" offset="1"/>
)==" // MSVC2022: C2026 String too big, trailing characters truncated.
R"==(
</linearGradient><linearGradient id="linearGradient9444" y2="486.4" xlink:href="#linearGradient5839" gradientUnits="userSpaceOnUse" x2="528" y1="513.27" x1="605.97" inkscape:collect="always"/><radialGradient id="radialGradient9446" xlink:href="#linearGradient6653" gradientUnits="userSpaceOnUse" cy="527.74" cx="342.74" gradientTransform="matrix(1 0 0 1.0314 177 -24.075)" r="86.43" inkscape:collect="always"/></defs><sodipodi:namedview id="base" bordercolor="#666666" inkscape:pageshadow="2" inkscape:window-y="25" pagecolor="#ffffff" inkscape:window-height="696" width="256px" inkscape:zoom="1" inkscape:window-x="0" height="256px" borderopacity="1.0" inkscape:current-layer="layer1" inkscape:cx="128" inkscape:cy="128" inkscape:window-width="1274" inkscape:pageopacity="0.0" inkscape:document-units="px"/><g id="layer1" inkscape:label="Warstwa 1" inkscape:groupmode="layer"><g id="g9400" transform="matrix(.52563 0 0 .52563 325.92 -65.929)"><path id="path7912" d="m-483.29 248.89c109.07-73.88 78.88-40.57 28.28 16.82-33.84 22.94-38.75 3.18-28.28-16.82z" sodipodi:nodetypes="ccc" style="opacity:.27626;color:#000000;fill-rule:evenodd;filter:url(#filter8192);fill:url(#linearGradient9430)" transform="matrix(.75926 0 0 .77032 -98.528 47.393)"/><path id="path7910" sodipodi:nodetypes="czzzz" style="opacity:.58755;color:#000000;fill-rule:evenodd;filter:url(#filter7934);fill:#000000" d="m-208 450.36c0 28.58-28.82 43.24-83 88-54.18 44.77-111.07 5.98-83-88s37.18-88 83-88 83 55.43 83 88z"/><path id="path7587" sodipodi:nodetypes="czzzz" style="opacity:.53307;color:#000000;fill-rule:evenodd;filter:url(#filter7982);fill:#000000" d="m-410 466.36c0 28.58-28.82 43.24-83 88-54.18 44.77-111.07 5.98-83-88s37.18-88 83-88 83 55.43 83 88z"/><g id="g7580" transform="translate(-602,-138)">
)==" // MSVC2022: C2026 String too big, trailing characters truncated.
R"==(
<path id="path6952" sodipodi:nodetypes="cccccssccccssc" style="fill-rule:evenodd;fill:#616f3c" d="m161.22 321.22c2.15 7.87-10.9 16.81-0.34 20.5-13.33 48.99-12.24 100.06-12.07 150.66l6.38-0.04c-0.17-50.68-1.17-101.16 11.84-149 4.05 0.93 7.94 1.64 9.03 1.91 9.73 2.43-4.84 6.16 11.32 2.13 0.05-0.02 0.1-0.02 0.15-0.04 39.26 32.24 78.39 67.5 117.53 118.66l5.06-3.91c-38.36-50.14-76.76-85.2-115.03-116.81 3.32-1.22 4.37-2.73 2.91-6.37-2.28-5.7-7.87-8.89-14.84-10.63-8.28-2.07-12.93-5.56-21.94-7.06z"/><path id="path6964" style="opacity:.70039;filter:url(#filter7454);fill-rule:evenodd;fill:url(#linearGradient9432)" d="m165.19 338.11c-14.44 49.36-13.57 101.8-13.32 152.97 5.29-0.91 0.95-11.28 2.32-16.03-0.17-45.82 0.39-92.49 13.46-136.72-0.61 0.2-2.45-1.47-2.46-0.22z"/><path id="path6993" style="filter:url(#filter7333);fill-rule:evenodd;fill:#373f22" d="m165.78 329.22c-0.04 3.34-3.32 8.55 1.6 10.53 3.52 1.16 7.22 1.17 9.87 4.16 3.25 3.04 9.27 1.46 8.91-3.5 0.82-4.71-2.06-9.52-7.17-9.53-6.09-0.99-11.08-4.83-16.3-7.88 1.03 2.07 2.06 4.15 3.09 6.22z"/><path id="path7175" style="color:#000000;fill-rule:evenodd;filter:url(#filter7450);fill:url(#linearGradient9434)" d="m184.38 331.12c-3.03 0 0.92 6.1-1.22 9-3.19 4.32-4.81 8.04 5.46 6.54 0.52-0.08 1.08-0.22 1.66-0.41 42.99 34.98 83.27 73.48 116.94 117.66 4.04-2.82-4.08-7.2-5.34-10.63-31.97-40.36-69.34-75.61-109.16-108.16 1.91-1.15 3.5-2.88 3.5-5 0-3.61-8.82-9-11.84-9z"/><path id="path7306" style="opacity:.87549;color:#000000;fill-rule:evenodd;filter:url(#filter7576);fill:#633918" d="m163.22 323.42c-0.45 0.6 0.52 1.58-0.81 1.91-1.59 2.42-2.95 5.26-3.19 8.15 0.32 2.68 4.44 3.48 5.64 1.03 0.83-2.49-0.79-5.1-0.38-7.73-0.03-1.18-1-2.9 0.78-1.64 4.2 1.84 8.35 3.8 12.48 5.79 1.52 0.92 4.05-0.61 2.04-2.1-3.11-2.88-7.52-3.36-11.42-4.54-1.69-0.41-3.41-0.74-5.14-0.87z"/></g>
)==" // MSVC2022: C2026 String too big, trailing characters truncated.
R"==(
<g id="g4851" transform="translate(-357.35 -110.7)"><path id="path5750" sodipodi:nodetypes="csssc" style="fill:url(#radialGradient9436)" d="m-13.277 526.51c1.01 32.14-40.155 105.31-108.08 95.96-44.22-6.08-83.85-34.53-84.86-103.03-0.68-46.28 49-114.81 111.12-58.59 10.353 9.37 71.722-3.85 81.823 65.66z"/><path id="path5752" style="opacity:.43969;fill:url(#radialGradient9438)" d="m-141.25 439.7c-38.8-0.23-65.47 45.74-64.96 79.72 1.01 68.5 40.66 96.98 84.87 103.06 67.932 9.35 109.07-63.83 108.06-95.97-10.098-69.51-71.457-56.28-81.809-65.65-16.501-14.94-32.121-21.07-46.161-21.16zm15.04 10.22c0.17 0.09-0.19 0.98-1.29 2.94 10.81 0.47 21.26 20.04 34.192 33.56 1.499-8.96-13.502-30.33-3.281-18.13 11.985-7.13 21.268 7.05 27.531 10.07 5.553 2.51 10.842 4.25 15.937 11.03-3.619 10.3 17.389 21.21 7.344 31.69 5.006 15.24 3.685-10.38 3.688-12.16l1.406 8.78c0.901 6.77 4.57 7.89-0.656 30.72-11.037 7.44-14.104 19.04-20.5 27.06-21.136 1.22-25.038 8.96-22.938 12.06-13.283-4.85-25.713 27.57-34.313 11.69-9.81 1.23-12.32-8.25-19.34-11.59-11.13-1.44-11.83-20.54-21.38-19.19-10.58 6.47-7.22-16.69-12.69-23.84 9.41-13.52-12.3-26.69-8.96-28.6-12.34-18.21 2.02-34.47 11.25-30.9 4.92-4.32 11.94-1.77 20.84-8.1 5.23-4.77 22.19-27.6 23.16-27.09zm94.808 46.94c7.492 0 14.313 13.63 14.313 27.62 2.095 40-28.372 65.37-30.844 64.28-3.061-1.34 31.532-38.65 18.031-38.65-7.491 0-7.25-12.63-7.25-26.63 0-13.99-1.741-26.62 5.75-26.62z"/><path id="path5754" style="opacity:.72374;filter:url(#filter6021);fill:#ffffff" d="m-116.06 465.04c-6.44-0.16-8.24 8.66-3.69 12.35 9.88 8.61 23.26 12.8 32.411 22.31 8.885 7.17 22.261 12.87 33.218 7.06 6.516-4.58 2.151-14-2.218-18.53-10.212-9.22-24.707-10.76-36.726-16.97-7.325-2.8-14.965-6.38-22.995-6.22z"/><path id="path5756" style="opacity:.78599;fill:#ffffff" d="m-115.84 471.06c-1.37 2.16 2.96 4.38 4.53 5.25 10.96 6.37 21.827 13 31.31 21.31 6.504 4.06 16.259 8.39 23.062 3.26 0.164-8.46-9.487-12.78-16.24-15.53-12.373-3.75-23.828-9.92-36.292-13.54-2.09-0.44-4.23-0.79-6.37-0.75z"/>
)==" // MSVC2022: C2026 String too big, trailing characters truncated.
R"==(
<path id="path5758" style="opacity:.91440;filter:url(#filter6757);fill:#ffffff" d="m-118.38 445.05c-0.36 1.62-0.11 3.79 1.06 6.72 3.77 9.44 19.11 4.91 23.217 13.12 0.421 0.85 1.283 1.31 2.407 1.57 1.571-0.25 3.099-0.17 4.562 0.15h0.031c3.666-0.24 8.054-0.95 11.625-0.69-9.174-1.5-16.602-2.32-19.625-5.06-7.997-7.23-15.777-12.39-23.277-15.81zm45.434 21.31c2.448 0.71 4.125 2.29 4.125 5.6 0 4.34 2.554 7.08 5.875 9.21 0.934 0.48 1.861 1 2.781 1.6 0.033 0.02 0.06 0.04 0.094 0.06 0.039 0.03 0.086 0.04 0.125 0.06 4.51 2.31 9.421 4.1 11.312 7.25 4.386 7.31 14.946 5.39 21.344 0.1-12.747-16.56-31.056-21.33-45.656-23.88z"/><path id="path5760" sodipodi:rx="6.0609155" sodipodi:ry="7.0710678" style="filter:url(#filter6933);fill:#eeeeee" sodipodi:type="arc" d="m124.25 478.6a6.0609 7.0711 0 1 1 -12.12 0 6.0609 7.0711 0 1 1 12.12 0z" inkscape:transform-center-y="4.8498591" transform="matrix(1.2299 .56234 0 .41415 -269.27 183.64)" sodipodi:cy="478.59555" sodipodi:cx="118.18785"/><path id="path5762" sodipodi:nodetypes="ccc" style="opacity:.68872;filter:url(#filter6909);fill:#ffffff" d="m-41.214 522.61c-4.28 9.98-22.925 3.01-30.357-4.64 7.286-4.7 31.395-4.87 30.357 4.64z"/></g><g id="g9388"><g id="g6942" transform="translate(-783.65 -129.07)"><path id="path6290" sodipodi:nodetypes="csssc" style="fill:url(#radialGradient9440)" d="m604.8 509.94c9.82 54.42-35.57 88.03-70.36 97.93-34.8 9.9-95.2-30.51-100.05-75.31-8.62-79.47 36.84-61.82 70.35-89.45 43.6-35.93 100.06 15.35 100.06 66.83z"/><path id="path5967" style="opacity:.79377;filter:url(#filter6020);fill:url(#linearGradient9442)" d="m467.06 466.12c-13.4 4.9-26.03 15.47-27.91 30.39-3.29 17.5-2.72 36.43 4.13 53.05 12.34 26.64 37.8 46.1 65.66 54.06 13.7 3.76 28.24 0.95 40.55-5.71 8.98-4.22 18.75-10.35 25.39-16.75-22.59 6.67-46.03-3.14-64.72-15.5-23.53-15.97-42.85-41.52-43.66-71.03-0.41-9.62-0.18-19.36 1.5-28.85l-0.94 0.34z"/>
)==" // MSVC2022: C2026 String too big, trailing characters truncated.
R"==(
<path id="path5789" sodipodi:nodetypes="csss" style="opacity:.56420;filter:url(#filter5835);fill-rule:evenodd;fill:url(#linearGradient9444)" d="m581.04 520.62c7.92-12.18-12.23-82.73-54.18-67.87-32.01 11.34-7.37 42.48 5.32 49.12 20.4 10.67 44.46 25.53 48.86 18.75z"/><path id="path5761" style="filter:url(#filter6938);fill:#eeeeee" d="m541.78 453.38c-6.53-0.35-16.08-0.94-18.5 6.81-1.88 5.21 1.22 12.26 7.44 12 6.07-1.14 13.62-4.6 18.59 0.97 6.4 3.74 10.98 12.18 18.88 12.34 7.05-2.41 6.02-12.92 1.34-17.22-6.66-8.41-16.39-16-27.75-14.9z"/><path id="path6293" style="opacity:.43969;fill:url(#radialGradient9446)" d="m536.62 431.09c-10.73-0.03-21.65 3.61-31.87 12.03-33.51 27.63-78.99 9.97-70.37 89.44 4.85 44.79 65.26 85.22 100.06 75.32 34.79-9.9 80.19-43.52 70.37-97.94 0-39.42-33.1-78.72-68.19-78.85zm-2.06 15.82c3.61 0.7 7.09 1.83 10.63 2.81 8.58 1.46 18.01 2.64 21.78 11.9 7.27 4.73 7.28 13.23 10.75 20.22 8.83 3.38 7.94 11.88 8.28 18.6 5.96 5.5 0.12 13.79 0.34 21 3.5 3.7 6.24 8.77 1.44 13.31-3.02 4.71-1.93 12.33-8.84 14.13-0.97 6.21-5.57 17.35-10.53 10.62-7.13-3.33-9.56 4.51-12.38 8.53-7.45 2.27-17.87 6.22-25.65 2.59-6.44-4-13.96 2.94-20.69-1.03-6.27-4.99-14.68-7.9-22.35-10.68-3.74-8.17-9.62-12.9-14.59-19.69-2.15-2.05-6.46-14.36-5.44-5.34 0.4 6.16 1.97 12.82-3.53 17.09 2.54 3-0.58 7.06 2.84 10.03 8.99 4.98-0.14 5.86-5.84 3.56-6.73-1.33-12.08-10.69-9.81-17.34-2.38-5.7-8.72-12.77-10.38-20.16-4.38-7.13-2.87-11.67-1.25-17.75-0.84-5.52 7.94-3.33 10.75-9.37 9.34 0.85 5.86-10.25 10.41-14.35 7.19-3.02 2.63-11.56 9.53-14.93 7.88-1.94 13.7-6.31 19.56-11.82 9-1.36 18.3-2.98 26.91-6.72 5.09-5.18 12.9-0.71 18.06-5.21z"/><path id="path5757" sodipodi:nodetypes="csssc" style="opacity:.82879;fill:#ffffff" d="m569 474.11c1 16.87-7.01 1.36-22.25-7.25-6.96-3.93-19.42 6.05-19.75-2.25-0.31-7.86 3.64-7.3 16.75-7.25 12.83 0.06 19.6 10.02 25.25 16.75z"/>
)==" // MSVC2022: C2026 String too big, trailing characters truncated.
R"==(
<path id="path4866" sodipodi:rx="5.6568542" sodipodi:ry="4.9497476" style="opacity:.45136;filter:url(#filter5785);fill:#ffffff" sodipodi:type="arc" d="m317.49 477.48a5.6569 4.9497 0 1 1 -11.31 0 5.6569 4.9497 0 1 1 11.31 0z" transform="translate(179.61 -7.7782)" sodipodi:cy="477.48438" sodipodi:cx="311.83408"/><path id="path5847" sodipodi:rx="1.9445436" sodipodi:ry="1.5909903" style="opacity:.56420;fill:#ffffff" sodipodi:type="arc" d="m491.09 470.94a1.9445 1.591 0 1 1 -3.89 0 1.9445 1.591 0 1 1 3.89 0z" transform="translate(1.3679 1.3833)" sodipodi:cy="470.94363" sodipodi:cx="489.14111"/></g><path id="path8205" sodipodi:nodetypes="czcccc" style="filter:url(#filter9241);fill-rule:evenodd;fill:#ffffff" d="m-259.97 306.69c17.86 4.31 23.82 0.46 41.4 12.97 16.79 11.94 20.78 26.04 28.47 37.7 0.26 1.87 2.52 6.44 3.03 2.12 1.46-4.54 0.95-5.94-2.37-9.71 2.94 0.8-27.35-58.85-70.53-43.08z"/></g></g></g><metadata><rdf:RDF><cc:Work><dc:format>image/svg+xml</dc:format><dc:type rdf:resource="http://purl.org/dc/dcmitype/StillImage"/><cc:license rdf:resource="http://creativecommons.org/licenses/publicdomain/"/><dc:publisher><cc:Agent rdf:about="http://openclipart.org/"><dc:title>Openclipart</dc:title></cc:Agent></dc:publisher><dc:title>cherries</dc:title><dc:date>2006-12-03T07:23:45</dc:date><dc:description>Inkscape 0.44+devel</dc:description><dc:source>https://openclipart.org/detail/1742/cherries-by-thestructorr</dc:source><dc:creator><cc:Agent><dc:title>TheStructorr</dc:title></cc:Agent></dc:creator><dc:subject><rdf:Bag><rdf:li>cherry</rdf:li><rdf:li>food</rdf:li><rdf:li>fruit</rdf:li><rdf:li>photorealistic</rdf:li><rdf:li>plant</rdf:li><rdf:li>red</rdf:li></rdf:Bag></dc:subject></cc:Work><cc:License rdf:about="http://creativecommons.org/licenses/publicdomain/"><cc:permits rdf:resource="http://creativecommons.org/ns#Reproduction"/><cc:permits rdf:resource="http://creativecommons.org/ns#Distribution"/><cc:permits rdf:resource="http://creativecommons.org/ns#DerivativeWorks"/></cc:License></rdf:RDF></metadata></svg>
)=="sv;
        };
        auto register_images = [](auto& boss)
        {
            auto& canvas_l = *boss.content(test_topic_vars::image1).lyric;
            auto& canvas_r = *boss.content(test_topic_vars::image2).lyric;
            auto images = cell::images(); // Lock.
            auto image_0_ptr = ptr::shared(imagens::image{});
            auto image_l_ptr = ptr::shared(imagens::image{});
            auto image_r_ptr = ptr::shared(imagens::image{});
            auto image_0_index = images.set(image_0_ptr);
            auto image_l_index = images.set(image_l_ptr);
            auto image_r_index = images.set(image_r_ptr);
            auto size = twod{ 20, 10 };
            auto& image_0 = *image_0_ptr;
            auto& image_l = *image_l_ptr;
            auto& image_r = *image_r_ptr;
            image_0.index = image_0_index;
            image_l.index = image_l_index;
            image_r.index = image_r_index;
            image_0.document = get_svg();
            image_0.gb_attrs[imagens::gb::w] = (fp32)size.x;
            image_0.gb_attrs[imagens::gb::h] = (fp32)size.y;
            image_0.gb_attrs[imagens::gb::uw] = 1.f;
            image_0.gb_attrs[imagens::gb::vh] = 1.f;
            auto& r = image_r.layers.emplace_back();
            auto& l = image_l.layers.emplace_back();
            r.index = image_0_index;
            l.index = image_0_index;
            r.image_wptr = image_0_ptr;
            l.image_wptr = image_0_ptr;
            r.opt_attrs[imagens::gb::tr] = 0.f;
            imagens::mirror_fx(r.opt_attrs[imagens::gb::tr].value(), imagens::flips::hz);
            auto brush_r = cell{};
            auto brush_l = cell{};
            brush_r.set_image_index(image_r.index);
            brush_l.set_image_index(image_l.index);
            brush_r.set_image_WH(size.x, size.y).set_image_ontop(1);
            brush_l.set_image_WH(size.x, size.y);
            canvas_r.core::template size<true>(size, brush_r); // clang requires template
            canvas_l.core::template size<true>(size, brush_l); //
            auto head_r = canvas_r.begin();
            auto head_l = canvas_l.begin();
            for (auto row = 1; row <= size.y; row++)
            {
                for (auto column = 1; column <= size.x; column++)
                {
                    (*head_r++).set_image_cr(column, row);
                    (*head_l++).set_image_cr(column, row);
                }
            }
            boss.topic.reindex();
            auto& indexer = boss.bell::indexer;
            boss.base::atexit([&, image_0_index, image_l_index, image_r_index]
            {
                log("Cleanup: image_0=%% image_l=%% image_r=%%", image_0_index, image_l_index, image_r_index);
                auto images = cell::images(); // Lock.
                images.remove(image_r_index);
                images.remove(image_l_index);
                images.remove(image_0_index);
                auto removed_indexes = e2::data::image::remove.param({ image_r_index, image_l_index, image_0_index });
                indexer.notify_general(e2::data::image::remove.id, removed_indexes);
            });
        };

        auto build = [](eccc /*appcfg*/, settings& config)
        {
            auto topic = get_text();
            auto window = ui::cake::ctor()
                ->plugin<pro::focus>(pro::focus::mode::focused)
                ->plugin<pro::keybd>()
                //->plugin<pro::acryl>()
                ->plugin<pro::cache>()
                ->invoke([](auto& boss)
                {
                    boss.LISTEN(tier::anycast, e2::form::proceed::quit::any, fast)
                    {
                        boss.base::riseup(tier::release, e2::form::proceed::quit::one, fast);
                    };
                });
            auto object0 = window->attach(ui::fork::ctor(axis::Y))
                                 ->colors(whitelt, 0xA0'00'37'db);
            auto defapp_context = config.settings::push_context("/config/defapp/");
                auto [menu_block, cover, menu_data] = app::shared::menu::create(config, {});
                auto menu = object0->attach(slot::_1, menu_block);
                auto test_stat_area = object0->attach(slot::_2, ui::fork::ctor(axis::Y));
                    auto layers = test_stat_area->attach(slot::_1, ui::cake::ctor());
                        auto scroll = layers->attach(ui::rail::ctor())
                                            ->active(cyanlt, bluedk);
                            auto object = scroll->attach(ui::post::ctor())
                                                ->upload(topic)
                                                ->invoke([&](auto& boss)
                                                {
                                                    register_images(boss);
                                                    boss.LISTEN(tier::release, e2::postrender, canvas)
                                                    {
                                                        static auto counter = 0; counter++;
                                                        static auto textclr =  ansi::bgc(reddk).fgc(whitelt);
                                                        boss.content(test_topic_vars::object1) = textclr + " inlined #1: " + std::to_string(counter) + " hits ";
                                                        boss.content(test_topic_vars::object2) = textclr + " inlined #2: " + canvas.face::area().size.str() + " ";
                                                        boss.content(test_topic_vars::object3) = textclr + " inlined #3: " + canvas.flow::full().coor.str() + " ";
                                                    };
                                                    //todo
                                                    //boss.LISTEN(tier::general, e2::form::canvas, canvas_ptr)
                                                    //{
                                                    //    boss.content(test_topic_vars::dynamix1).lyric = boss.content(test_topic_vars::dynamix2).lyric;
                                                    //    boss.content(test_topic_vars::dynamix2).lyric = boss.content(test_topic_vars::dynamix3).lyric;
                                                    //    boss.content(test_topic_vars::dynamix3).lyric = canvas_ptr;
                                                    //};
                                                });
                        auto scroll_bars = layers->attach(ui::fork::ctor());
                            auto vt = scroll_bars->attach(slot::_2, ui::grip<axis::Y>::ctor(scroll));
                            auto hz = test_stat_area->attach(slot::_2, ui::grip<axis::X>::ctor(scroll));
                auto& a = object->lyric(test_topic_vars::canvas1);
                    a.mark().fgc(0xFF000000);
                    a.size({ 40, 9 });
                    a.grad(argb{ 0xFF00FFFF }, argb{ 0x40FFFFFF });
                    auto t = para{ "ARBITRARY SIZE BLOCK" };
                    a.text((a.size() - twod{ t.length(), 0 }) / 2, t.shadow());
                auto& b = object->lyric(test_topic_vars::canvas2);
                    b.mark().fgc(0xFF000000);
                    b.size({ 6, 2 });
                    b.grad(argb{ 0xFF00FFFF }, argb{ 0x40FFFFFF });
                    b[{5, 0}].alpha(0);
                    b[{5, 1}].alpha(0);
            window->invoke([&](auto& boss)
            {
                app::shared::base_kb_navigation(config, scroll, boss);
            });
            return window;
        };
    }

    app::shared::initialize builder{ app::test::id, build };
}