// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

namespace netxs::events::userland
{
    struct test
    {
        EVENTPACK( test, netxs::events::userland::root::custom )
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
    };
}

// test: Test window.
namespace netxs::app::test
{
    static constexpr auto id = "test";
    static constexpr auto desc = "Desktopio App Testing (DEMO)";

    using events = netxs::events::userland::test;

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

        auto get_text = []
        {
            static text topic;

            if (topic.empty())
            {
                auto clr = 0xFFFFFFFF;
                auto msg = text{ "The quick brown fox jumps over the lazy dog." };
                auto msg_rtl = ansi::rtl(rtol::rtl).add("RTL: ", msg).rtl(rtol::ltr);
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
                    .jet(bias::right).rtl(rtol::ltr)
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
                    //+ "https://github.com/netxs-group/vtm\n\n"
                    .jet(bias::center).wrp(wrap::off).fgc(whitelt).mgl(0).mgr(0).eol()
                    .fgc(c1).bgc(c2).add("▄")
                    .fgc(c2).bgc(c1).add("▄")
                    .fgc(clr).bgc().add("  Desktopio Environment\n")
                    .fgc().bgc().add("Test Page    \n\n")

                    .nil().jet(bias::left).mgl(4).mgr(4).wrp(wrap::off)
                    .fgc(0xff000000).bgc(0xff00FF00).add(" ! ", "\n")
                    .cuu(1).chx(0).mgl(9).fgc().bgc().wrp(wrap::on)
                    .add("Test page for testing text formatting functionality. \n"
                    "The following text doesn't make much sense, "
                    "it's just a bunch of text samples.\n"
                    "\n")
                    .nil().jet(bias::left).mgl(4).mgr(4).wrp(wrap::off)
                    .fgc(0xffFFFFFF).bgc(0xff0000FF).add(" ! ", "\n")
                    .cuu(1).chx(0).mgl(9).fgc().bgc().wrp(wrap::on)
                    .add("Make sure your terminal supports mouse reporting.\n"
                    "\n")
                    .nil().jet(bias::left).mgl(4).mgr(4).wrp(wrap::off)
                    .fgc(0xffFFFFFF).bgc(0xff0000FF).add(" ! ", "\n")
                    .cuu(1).chx(0).mgl(9).fgc().bgc().wrp(wrap::on)
                    .add("At the moment terminal "
                    "emulators are not able to display wide characters "
                    "in parts, as well as independently color the left "
                    "and right parts. In every case, when such a wide "
                    "character separation is required, we have to replace "
                    "the corresponding parts with 'REPLACEMENT CHARACTER' "
                    "(U+FFFD).\n"
                    "\n")

                    .jet(bias::center).wrp(wrap::off).fgc(whitelt).mgl(1).mgr(0)
                    .add("Test Samples\n\n")
                    .jet(bias::left).wrp(wrap::off).fgc(whitelt).mgl(1).mgr(0)
                    .add("User Interface Commands (outdated)\n")
                    .jet(bias::left).mgl(1).mgr(0).wrp(wrap::off).eol()
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
                auto wiki = ansi::esc("\n\n")
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
                    .fgc(clr).wrp(wrap::off).add("\nSmileys (wrap OFF)\n").nil()
                        .add("😀😃😄😁😆😅😂🤣😊😇🙂🙃😉😌😍😺\n"
                             "😏😒😞😔😟😕😣😖😫😩🥺😢😭😤😸😹\n"
                             "🥰😘😗😙😚😋😛😝😜🤪🤨🧐🤓😎🤩🥳\n"
                             "😡🤬🤯😳🥵🥶😱😨😰😥😓🤗🤔🤭🤫🤥\n"
                             "😶😐😑😬🙄😯😦😧😮😲🥱😴🤤😪😵🤐\n"
                             "🥴🤢🤮🤧😷🤒🤕🤑🤠😈👿👹👺🤡💩👻\n"
                             "💀👽👾🤖🎃😺😸😹😻😼😽🙀😿😾😠😍\n")
                    .fgc(clr).wrp(wrap::on).add("\nSmileys (wrap ON)\n").nil()
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
        auto build = [](text cwd, text arg, xmls& config, text patch)
        {
            auto topic = get_text();
            auto window = ui::cake::ctor()
                ->plugin<pro::focus>()
                ->plugin<pro::track>()
                ->plugin<pro::acryl>()
                ->plugin<pro::cache>()
                ->invoke([](auto& boss)
                {
                    boss.keybd.accept(true);
                    boss.LISTEN(tier::anycast, e2::form::quit, item)
                    {
                        boss.RISEUP(tier::release, e2::form::quit, item);
                    };
                });
            auto object0 = window->attach(ui::fork::ctor(axis::Y))
                                 ->colors(whitelt, 0xA0db3700);
                config.cd("/config/test/", "/config/defapp/");
                auto [menu_block, cover, menu_data] = app::shared::menu::create(config, {});
                auto menu = object0->attach(slot::_1, menu_block);
                auto test_stat_area = object0->attach(slot::_2, ui::fork::ctor(axis::Y));
                    auto layers = test_stat_area->attach(slot::_1, ui::cake::ctor());
                        auto scroll = layers->attach(ui::rail::ctor())
                                            ->colors(cyanlt, bluedk);
                            auto object = scroll->attach(ui::post::ctor())
                                                ->upload(topic)
                                                ->invoke([&](auto& self)
                                                {
                                                    self.LISTEN(tier::release, e2::postrender, canvas)
                                                    {
                                                        static auto counter = 0; counter++;
                                                        static auto textclr =  ansi::bgc(reddk).fgc(whitelt);
                                                        self.content(test_topic_vars::object1) = textclr + " inlined #1: " + std::to_string(counter) + " hits ";
                                                        self.content(test_topic_vars::object2) = textclr + " inlined #2: " + canvas.area().size.str() + " ";
                                                        self.content(test_topic_vars::object3) = textclr + " inlined #3: " + canvas.full().coor.str() + " ";
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
                    a.grad(rgba{ 0xFFFFFF00 }, rgba{ 0x40FFFFFF });
                    auto t = para{ "ARBITRARY SIZE BLOCK" };
                    a.text((a.size() - twod{ t.length(), 0 }) / 2, t.shadow());
                auto& b = object->lyric(test_topic_vars::canvas2);
                    b.mark().fgc(0xFF000000);
                    b.size({ 6, 2 });
                    b.grad(rgba{ 0xFFFFFF00 }, rgba{ 0x40FFFFFF });
                    b[{5, 0}].alpha(0);
                    b[{5, 1}].alpha(0);

            return window;
        };
    }

    app::shared::initialize builder{ app::test::id, build };
}