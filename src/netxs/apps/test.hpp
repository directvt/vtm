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
        };

        template<auto ...Args>
        constexpr auto vss = utf::matrix::vss<Args...>;

        auto test_page = [](auto hdrclr, auto txtclr)
        {
            auto header = [&](auto caption)
            {
                return ansi::mgl(1).wrp(wrap::off).fgc(hdrclr).unc(whitedk).cap(caption).erl().und(unln::none).eol().fgc(txtclr).mgl(3).unc(0).wrp(wrap::on);
            };
            auto crop = ansi::mgl(1).mgr(2).jet(bias::center)
                .add("\n")
                .wrp(wrap::off).fgc(hdrclr).cap(skin::globals().NsInfoSF, 3, 3, faux).eol()
                .jet(bias::left)
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
                                                ->invoke([&](auto& self)
                                                {
                                                    self.LISTEN(tier::release, e2::postrender, canvas)
                                                    {
                                                        static auto counter = 0; counter++;
                                                        static auto textclr =  ansi::bgc(reddk).fgc(whitelt);
                                                        self.content(test_topic_vars::object1) = textclr + " inlined #1: " + std::to_string(counter) + " hits ";
                                                        self.content(test_topic_vars::object2) = textclr + " inlined #2: " + canvas.face::area().size.str() + " ";
                                                        self.content(test_topic_vars::object3) = textclr + " inlined #3: " + canvas.flow::full().coor.str() + " ";
                                                    };
                                                    //todo
                                                    //self.LISTEN(tier::general, e2::form::canvas, canvas_ptr)
                                                    //{
                                                    //    self.content(test_topic_vars::dynamix1).lyric = self.content(test_topic_vars::dynamix2).lyric;
                                                    //    self.content(test_topic_vars::dynamix2).lyric = self.content(test_topic_vars::dynamix3).lyric;
                                                    //    self.content(test_topic_vars::dynamix3).lyric = canvas_ptr;
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