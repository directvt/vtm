// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#define APPS_DEL_TIMEOUT 1s

#include "controls.hpp"
#include "xml.hpp"

#include <fstream>

namespace netxs::app
{
    namespace fs = std::filesystem;

    using namespace std::placeholders;
    using namespace netxs::ui;
}

namespace netxs::app::shared
{
    static constexpr auto default_config = R"==(
<config>
    <menu selected=Term>  <!-- Set selected using menu item id. -->
        <item*/>  <!-- Use asterisk at the end of the element name to set defaults.
                       Using an asterisk with the parameter name of the first element in the list without any other nested attributes
                       indicates the beginning of the list, i.e. the list will replace the existing one when the configuration is merged. -->
        <item splitter label="apps">
            <notes>
                " Default applications group                         \n"
                " It can be configured in ~/.config/vtm/settings.xml "
            </notes>
        </item>
        <item* hidden=no fgc=whitedk bgc=0x00000000 winsize=0,0 wincoor=0,0/>
)=="
#if defined(_WIN32)
R"==(
        <item id=Term label="cmd" type=DirectVT title="Command Prompt" notes=" run Windows Command Prompt " param="$0 -r term">
)=="
#else
R"==(
        <item id=Term label="Term" type=DirectVT title="Terminal Emulator" notes=" run built-in Terminal " param="$0 -r term">
)=="
#endif
R"==(
            <hotkeys key*>    <!-- not implemented -->
                <key="Ctrl+'t'" action=Start/>
            </hotkeys>
            <config>   <!-- The following config partially overrides the base configuration. It is valid for DirectVT apps only. -->
                <term>
                    <scrollback>
                        <size=40000/>   <!-- Scrollback buffer length. -->
                        <wrap="on"/>    <!-- Lines wrapping mode. -->
                    </scrollback>
                    <cursor>
                        <style="underline"/> <!-- block | underline  -->
                    </cursor>
                    <menu>
                        <autohide = on/>  <!--  If true, show menu only on hover. -->
                        <slim = true/>
                    </menu>
                    <selection>
                        <mode = text/> <!-- text | ansi | rich | html | protected | none -->
                    </selection>
                    <hotkeys key*>    <!-- not implemented -->
                        <key="Alt+RightArrow" action=TerminalFindNext/>
                        <key="Alt+LeftArrow"  action=TerminalFindPrev/>
                        <key="Ctrl+'z'"       action=TerminalQuit/>
                    </hotkeys>
                </term>
            </config>
        </item>
)=="
#if defined(_WIN32)
R"==(
        <item id=PowerShell label="PowerShell" type=DirectVT title="PowerShell"                  param="$0 -r term powershell" fgc=15 bgc=0xFF562401 notes=" run PowerShell "/>
        <item id=WSL        label="WSL"        type=DirectVT title="Windows Subsystem for Linux" param="$0 -r term wsl"                              notes=" run default WSL profile "/>
)=="
#endif
R"==(
        <item id=Tile       label="Tile"       type=Group    title="Tiling Window Manager" param="h1:1(Term, Term)"    notes=" run Tiling Window Manager with two terminals attached "/>
        <item id=View       label=View         type=Region   title="\e[11:3pView: Region"                              notes=" set desktop region "/>
        <item id=Settings   label=Settings     type=DirectVT title="Settings"              param="$0 -r settings"      notes=" run Settings " winsize=50,15/>
        <item id=Logs       label=Logs         type=DirectVT title="Logs"                  param="$0 -q -r term $0 -m" notes=" run Logs ">
            <config>
                <term>
                    <scrollback>
                        <size=5000/>
                        <wrap="off"/>
                    </scrollback>
                    <menu>
                        <autohide = on/>
                        <slim = true/>
                    </menu>
                    <selection>
                        <mode = text/> <!-- text | ansi | rich | html | protected | none -->
                    </selection>
                </term>
            </config>
        </item>
        <autorun item*>  <!-- Autorun of specified menu items -->
            <!--  <item* id=Term winsize=80,25/> --> <!-- Set defaults for the list -->
            <!--  <item focused wincoor=8,3/> -->
            <!--  <item wincoor=92,30/> -->
            <!--  <item wincoor=8,30 focused/> -->
        </autorun>
        <width>    <!-- Taskbar menu width. -->
            <folded=4/>
            <expanded=31/>
        </width>
    </menu>
    <hotkeys key*>    <!-- not implemented -->
        <key="Ctrl+PgUp" action=PrevWindow/>
        <key="Ctrl+PgDn" action=NextWindow/>
    </hotkeys>
    <appearance>
        <defaults>
            <fps      = 60   />
            <bordersz = 1,1  />
            <brighter = 60   />
            <kb_focus = 60   />
            <shadower = 180  />
            <shadow   = 180  />
            <lucidity = 0xff /> <!-- not implemented -->
            <selector = 48   />
            <highlight  fgc=purewhite bgc=bluelt      />
            <warning    fgc=whitelt   bgc=yellowdk    />
            <danger     fgc=whitelt   bgc=redlt       />
            <action     fgc=whitelt   bgc=greenlt     />
            <label      fgc=blackdk   bgc=whitedk     />
            <inactive   fgc=blacklt   bgc=transparent />
            <menu_white fgc=whitelt   bgc=0x80404040  />
            <menu_black fgc=blackdk   bgc=0x80404040  />
            <fader duration=0ms fast=0ms/>  <!-- Fader animation config. -->
        </defaults>
        <runapp>    <!-- Override defaults. -->
            <brighter=0/>
        </runapp>
    </appearance>
    <set>         <!-- Global namespace - Unresolved literals will be taken from here. -->
        <blackdk   = 0xFF101010 /> <!-- Color reference literals. -->
        <reddk     = 0xFF1f0fc4 />
        <greendk   = 0xFF0ea112 />
        <yellowdk  = 0xFF009cc0 />
        <bluedk    = 0xFFdb3700 />
        <magentadk = 0xFF981787 />
        <cyandk    = 0xFFdd963b />
        <whitedk   = 0xFFbbbbbb />
        <blacklt   = 0xFF757575 />
        <redlt     = 0xFF5648e6 />
        <greenlt   = 0xFF0cc615 />
        <yellowlt  = 0xFFa5f1f8 />
        <bluelt    = 0xFFff783a />
        <magentalt = 0xFF9e00b3 />
        <cyanlt    = 0xFFd6d660 />
        <whitelt   = 0xFFf3f3f3 />
        <pureblack = 0xFF000000 />
        <purewhite = 0xFFffffff />
        <nocolor   = 0x00000000 />
        <transparent = nocolor  />
    </set>
    <client>
        <background fgc=whitedk bgc=0xFF000000>  <!-- Desktop background color. -->
            <tile=""/> <!-- True color ANSI-art with gradients can be used here. -->
        </background>
        <clipboard>
            <preview enabled=true size=80x25 bgc=bluedk fgc=whitelt>
                <alpha=0xFF/>  <!-- Preview alpha is applied only to the ansi/rich/html text type -->
                <timeout=3s/>  <!-- Preview hiding timeout. Set it to zero to disable hiding. -->
                <shadow=7  />  <!-- Preview shadow strength (0-10). -->
            </preview>
        </clipboard>
        <viewport coor=0,0/>
        <mouse dblclick=500ms/>
        <tooltip timeout=500ms enabled=true fgc=pureblack bgc=purewhite/>
        <glowfx=true/>                      <!-- Show glow effect around selected item. -->
        <debug overlay=faux toggle="🐞"/>  <!-- Display console debug info. -->
        <regions enabled=faux/>             <!-- Highlight UI objects boundaries. -->
    </client>
    <term>      <!-- Base configuration for the Term app. It can be partially overridden by the menu item's config subarg. -->
        <scrollback>
            <size=20000    />   <!-- Scrollback buffer length. -->
            <growstep=0    />   <!-- Scrollback buffer grow step. The buffer behaves like a ring in case of zero. -->
            <maxline=65535 />   <!-- Max line length. Line splits if it exceeds the limit. -->
            <wrap="on"     />   <!-- Lines wrapping mode. -->
            <reset onkey="on" onoutput="off"/>   <!-- Scrollback viewport reset triggers. -->
        </scrollback>
        <color>
            <color0  = blackdk    /> <!-- See /config/set/* for the color name reference. -->
            <color1  = reddk      />
            <color2  = greendk    />
            <color3  = yellowdk   />
            <color4  = bluedk     />
            <color5  = magentadk  />
            <color6  = cyandk     />
            <color7  = whitedk    />
            <color8  = blacklt    />
            <color9  = redlt      />
            <color10 = greenlt    />
            <color11 = yellowlt   />
            <color12 = bluelt     />
            <color13 = magentalt  />
            <color14 = cyanlt     />
            <color15 = whitelt    />
            <default bgc=0 fgc=15 />  <!-- Initial colors. -->
            <match fx=color bgc="0xFF007F00" fgc=whitelt/>  <!-- Color of the selected text occurrences. Set fx to use cell::shaders: xlight | color | invert | reverse -->
            <selection>
                <text fx=color bgc=bluelt fgc=whitelt/>  <!-- Highlighting of the selected text in plaintext mode. -->
                <protected fx=color bgc=bluelt fgc=whitelt/>  <!-- Note: The bgc and fgc attributes only apply to the fx=color shader. -->
                <ansi fx=xlight bgc=bluelt fgc=whitelt/>
                <rich fx=xlight bgc=bluelt fgc=whitelt/>
                <html fx=xlight bgc=bluelt fgc=whitelt/>
                <none fx=color bgc=blacklt fgc=whitedk/>  <!-- Inactive selection color. -->
            </selection>
        </color>
        <fields>
            <lucent=0xC0/> <!-- Fields transparency level. -->
            <size=0/>      <!-- Left/right field size (for hz scrolling UX). -->
        </fields>
        <tablen=8/>   <!-- Tab length. -->
        <cursor>
            <style="underline"/> <!-- block | underline -->
            <blink=400ms/>       <!-- blink period -->
            <show=true/>
        </cursor>
        <menu item*>  <!-- Use asterisk to drop previous/existing item list. -->
            <autohide=true/>  <!-- If true, show menu only on hover. -->
            <enabled=1/>
            <slim=1/>
            <item label="Wrap" type=Option action=TerminalWrapMode data="off">
                <label="\e[38:2:0:255:0mWrap\e[m" data="on"/>
                <notes>
                    " Wrapping text lines on/off      \n"
                    " - applied to selection if it is "
                </notes>
            </item>
            <item label="Selection" notes=" Text selection mode " type=Option action=TerminalSelectionMode data="none">  <!-- type=Option means that the тext label will be selected when clicked.  -->
                <label="\e[38:2:0:255:0mPlaintext\e[m" data="text"/>
                <label="\e[38:2:255:255:0mANSI-text\e[m" data="ansi"/>
                <label data="rich">
                    "\e[38:2:109:231:237m""R"
                    "\e[38:2:109:237:186m""T"
                    "\e[38:2:60:255:60m"  "F"
                    "\e[38:2:189:255:53m" "-"
                    "\e[38:2:255:255:49m" "s"
                    "\e[38:2:255:189:79m" "t"
                    "\e[38:2:255:114:94m" "y"
                    "\e[38:2:255:60:157m" "l"
                    "\e[38:2:255:49:214m" "e" "\e[m"
                </label>
                <label="\e[38:2:0:255:255mHTML-code\e[m" data="html"/>
                <label="\e[38:2:0:255:255mProtected\e[m" data="protected"/>
            </item>
            <item label="<" action=TerminalFindPrev>  <!-- type=Command is a default item's attribute. -->
                <label="\e[38:2:0:255:0m<\e[m"/>
                <notes>
                    " Previous match                    \n"
                    " - using clipboard if no selection \n"
                    " - page up if no clipboard data    "
                </notes>
            </item>
            <item label=">" action=TerminalFindNext>
                <label="\e[38:2:0:255:0m>\e[m"/>
                <notes>
                    " Next match                        \n"
                    " - using clipboard if no selection \n"
                    " - page up if no clipboard data    "
                </notes>
            </item>
            <item label="  "    notes=" ...empty menu block/splitter for safety "/>
            <item label="Clear" notes=" Clear TTY viewport "                  action=TerminalOutput data="\e[2J"/>
            <item label="Reset" notes=" Clear scrollback and SGR-attributes " action=TerminalOutput data="\e[!p"/>
            <!-- <item label="Hello, World!" notes=" Simulating keypresses "       action=TerminalSendKey data="Hello World!"/> -->
        </menu>
        <selection>
            <mode="text"/> <!-- text | ansi | rich | html | protected | none -->
            <rect=faux/>  <!-- Preferred selection form: Rectangular: true, Linear false. -->
        </selection>
        <atexit = auto/>  <!-- auto:    Stay open and ask if exit code != 0. (default)
                               ask:     Stay open and ask.
                               close:   Always close.
                               restart: Restart session.
                               retry:   Restart session if exit code != 0. -->
        <hotkeys key*>    <!-- not implemented -->
            <key="Alt+RightArrow" action=FindNext/>
            <key="Alt+LeftArrow"  action=FindPrev/>
        </hotkeys>
    </term>
    <defapp>
        <menu>
            <autohide=faux/>
            <enabled="on"/>
            <slim=true/>
        </menu>
    </defapp>
    <tile>
        <menu>
            <autohide=true/>
            <enabled="on"/>
            <slim=1/>
        </menu>
    </tile>
    <text>      <!-- Base configuration for the Text app. It can be overridden by param's subargs. -->
        <!-- not implemented -->
    </text>
    <calc>      <!-- Base configuration for the Calc app. It can be overridden by param's subargs. -->
        <!-- not implemented -->
    </calc>
    <settings>      <!-- Base configuration for the Settings app. It can be overridden by param's subargs. -->
        <!-- not implemented -->
    </settings>
</config>
)==";

    static const auto usr_config = "~/.config/vtm/settings.xml";
    static const auto env_config = "$VTM_CONFIG"s;

    static constexpr auto path_autorun  = "config/menu/autorun";
    static constexpr auto path_hotkeys  = "config/hotkeys";

    namespace menu
    {
        namespace attr
        {
            static constexpr auto brand = "type";
            static constexpr auto label = "label";
            static constexpr auto notes = "notes";
            static constexpr auto route = "action";
            static constexpr auto param = "data";
            static constexpr auto onkey = "hotkey";
        }
        namespace type
        {
            static const auto Command  = "Command"s;
            static const auto Splitter = "Splitter"s;
            static const auto Option   = "Option"s;
            static const auto Repeat   = "Repeat"s;
        }

        struct item
        {
            enum type
            {
                Splitter,
                Command,
                Option,
                Repeat,
            };
            struct look
            {
                text label{};
                text notes{};
                text param{};
                text onkey{};
                si32 value{};
            };

            using imap = std::unordered_map<si32, si32>;
            using list = std::vector<look>;

            type brand{};
            bool alive{};
            si32 taken{};
            list views{};
            imap index{};

            void select(si32 i)
            {
                auto iter = index.find(i);
                taken = iter == index.end() ? 0 : iter->second;
            }
            template<class P>
            void reindex(P take)
            {
                for (auto i = 0; i < views.size(); i++)
                {
                    auto& l = views[i];
                    l.value = static_cast<si32>(take(l.param));
                    index[l.value] = i;
                }
            }
        };

        using link = std::tuple<netxs::sptr<item>, std::function<void(ui::pads&, item&)>>;
        using list = std::list<link>;

        const auto create = [](xml::settings& config, list menu_items) // Menu bar (shrinkable on right-click).
        {
            auto highlight_color = skin::color(tone::highlight);
            auto danger_color    = skin::color(tone::danger);
            auto c3 = highlight_color;
            auto x3 = cell{ c3 }.alpha(0x00);
            auto c1 = danger_color;
            auto x1 = cell{ c1 }.alpha(0x00);

            auto slot1 = ui::veer::ctor();
            auto autohide = config.take("menu/autohide", faux);
            auto menushow = config.take("menu/enabled" , true);
            auto menusize = config.take("menu/slim"    , faux);

            auto menuarea = ui::fork::ctor()
                            ->active();
                auto inner_pads = dent{ 1,2,1,1 };
                auto menulist = menuarea->attach(slot::_1, ui::fork::ctor());

                    menulist->attach(slot::_1, ui::pads::ctor(inner_pads, dent{ 0 }))
                            ->plugin<pro::fader>(x3, c3, skin::timeout(tone::fader))
                            ->plugin<pro::notes>(" Maximize/restore window ")
                            ->invoke([&](ui::pads& boss)
                            {
                                boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                                {
                                    boss.RISEUP(tier::release, e2::form::maximize, gear);
                                    gear.dismiss();
                                };
                            })
                            ->attach(ui::item::ctor(" ≡", faux, true));

                    auto scrlarea = menulist->attach(slot::_2, ui::cake::ctor());
                    auto scrlrail = scrlarea->attach(ui::rail::ctor(axes::X_ONLY, axes::X_ONLY));
                    auto scrllist = scrlrail->attach(ui::list::ctor(axis::X));

                    auto scroll_hint = ui::park::ctor();
                    auto hints = scroll_hint->attach(snap::stretch, menusize ? snap::center : snap::tail, ui::grip_fx<axis::X>::ctor(scrlrail));

                    auto scrl_grip = scrlarea->attach(scroll_hint);

                auto fader = skin::timeout(tone::fader);
                for (auto& body : menu_items)
                {
                    auto& item_ptr = std::get<0>(body);
                    auto& setup = std::get<1>(body);
                    auto& item = *item_ptr;
                    auto& hover = item.alive;
                    auto& label = item.views.front().label;
                    auto& notes = item.views.front().notes;
                    if (hover)
                    {
                        scrllist->attach(ui::pads::ctor(inner_pads, dent{ 1 }))
                                ->plugin<pro::fader>(x3, c3, fader)
                                ->plugin<pro::notes>(notes)
                                ->invoke([&](ui::pads& boss){ setup(boss, item); })
                                ->attach(ui::item::ctor(label, faux, true));
                    }
                    else
                    {
                        scrllist->attach(ui::pads::ctor(inner_pads, dent{ 1 }))
                                ->colors(0,0) //todo for mouse tracking
                                ->plugin<pro::notes>(notes)
                                ->invoke([&](ui::pads& boss){ setup(boss, item); })
                                ->attach(ui::item::ctor(label, faux, true));
                    }
                    scrllist->invoke([&](auto& boss) // Store shared ptr to the menu item config.
                    {
                        boss.LISTEN(tier::release, e2::dtor, v, 0, (item_ptr))
                        {
                            item_ptr.reset();
                        };
                    });
                }
                menuarea->attach(slot::_2, ui::pads::ctor(dent{ 2,2,1,1 }, dent{}))
                        ->plugin<pro::fader>(x1, c1, fader)
                        ->plugin<pro::notes>(" Close window ")
                        ->invoke([&](auto& boss)
                        {
                            boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                            {
                                boss.RISEUP(tier::release, e2::form::quit, boss.This());
                                gear.dismiss();
                            };
                        })
                        ->attach(ui::item::ctor("×"));

            auto menu_block = ui::park::ctor()
                ->plugin<pro::limit>(twod{ -1, menusize ? 1 : 3 }, twod{ -1, menusize ? 1 : 3 })
                ->invoke([&](ui::park& boss)
                {
                    scroll_hint->visible(hints, faux);
                    auto park_shadow = ptr::shadow(scroll_hint);
                    auto grip_shadow = ptr::shadow(hints);
                    boss.LISTEN(tier::release, hids::events::mouse::button::click::right, gear, 0, (park_shadow, grip_shadow))
                    {
                        if (auto park_ptr = park_shadow.lock())
                        if (auto grip_ptr = grip_shadow.lock())
                        {
                            auto& limit = boss.plugins<pro::limit>();
                            auto limits = limit.get();
                            if (limits.min.y == 1)
                            {
                                park_ptr->config(grip_ptr, snap::stretch, snap::tail);
                                limits.min.y = limits.max.y = 3;
                            }
                            else
                            {
                                park_ptr->config(grip_ptr, snap::stretch, snap::center);
                                limits.min.y = limits.max.y = 1;
                            }
                            limit.set(limits);
                            boss.reflow();
                            gear.dismiss();
                        }
                    };
                    boss.LISTEN(tier::anycast, e2::form::prop::ui::slimmenu, slim, 0, (park_shadow, grip_shadow))
                    {
                        auto size = slim ? 1 : 3;
                        if (auto park_ptr = park_shadow.lock())
                        if (auto grip_ptr = grip_shadow.lock())
                        {
                            auto& limit = boss.plugins<pro::limit>();
                            auto limits = limit.get();
                            limits.min.y = limits.max.y = std::max(0, size);
                            //todo too hacky
                            if (limits.min.y == 3)
                            {
                                park_ptr->config(grip_ptr, snap::stretch, snap::tail);
                            }
                            else
                            {
                                park_ptr->config(grip_ptr, snap::stretch, snap::center);
                            }
                            limit.set(limits);
                            boss.reflow();
                        }
                    };
                    //todo revise
                    if (menu_items.size()) // Show scrolling hint only if elements exist.
                    {
                        boss.LISTEN(tier::release, e2::form::state::mouse, active, 0, (park_shadow, grip_shadow))
                        {
                            if (auto park_ptr = park_shadow.lock())
                            if (auto grip_ptr = grip_shadow.lock())
                            {
                                park_ptr->visible(grip_ptr, active);
                                boss.base::deface();
                            }
                        };
                    }
                });
            menu_block->attach(snap::stretch, snap::center, menuarea);

            auto menu = slot1->attach(menu_block);
                    auto border = slot1->attach(ui::mock::ctor())
                                       ->plugin<pro::limit>(twod{ -1,1 }, twod{ -1,1 });
                         if (menushow == faux) autohide = faux;
                    else if (autohide == faux) slot1->roll();
                    slot1->invoke([&](auto& boss)
                    {
                        auto menu_shadow = ptr::shadow(menu_block);
                        auto hide_shadow = ptr::shared(autohide);
                        boss.LISTEN(tier::release, e2::form::state::mouse, hits, 0, (menu_shadow, hide_shadow))
                        {
                            if (*hide_shadow)
                            if (auto menu_ptr = menu_shadow.lock())
                            {
                                if (!!hits != (boss.back() == menu_ptr))
                                {
                                    boss.roll();
                                    boss.reflow();
                                }
                            }
                        };
                    });

            return std::tuple{ slot1, border, menu_block };
        };
        const auto demo = [](xml::settings& config)
        {
            auto items = list
            {
                { std::make_shared<item>(item{ item::type::Command, true, 0, std::vector<item::look>{{ .label = ansi::und(true).add("F").nil().add("ile"), .notes = " File menu item " } }}), [&](auto& boss, auto& item){ } },
                { std::make_shared<item>(item{ item::type::Command, true, 0, std::vector<item::look>{{ .label = ansi::und(true).add("E").nil().add("dit"), .notes = " Edit menu item " } }}), [&](auto& boss, auto& item){ } },
                { std::make_shared<item>(item{ item::type::Command, true, 0, std::vector<item::look>{{ .label = ansi::und(true).add("V").nil().add("iew"), .notes = " View menu item " } }}), [&](auto& boss, auto& item){ } },
                { std::make_shared<item>(item{ item::type::Command, true, 0, std::vector<item::look>{{ .label = ansi::und(true).add("D").nil().add("ata"), .notes = " Data menu item " } }}), [&](auto& boss, auto& item){ } },
                { std::make_shared<item>(item{ item::type::Command, true, 0, std::vector<item::look>{{ .label = ansi::und(true).add("H").nil().add("elp"), .notes = " Help menu item " } }}), [&](auto& boss, auto& item){ } },
            };
            config.cd("/config/defapp/");
            auto [menu, cover, menu_data] = create(config, items);
            return menu;
        };
    }

    //static si32 max_count = 20;// 50;
    static si32 max_vtm = 3;
    static si32 vtm_count = 0;
    static si32 tile_count = 0;
    //constexpr auto del_timeout = 1s;

    enum class app_type
    {
        simple,
        normal,
    };

    const auto app_class = [](view& v)
    {
        auto type = app_type::normal;
        if (!v.empty() && v.front() == '!')
        {
            type = app_type::simple;
            v.remove_prefix(1);
            v = utf::trim(v);
        }
        return type;
    };
    const auto closing_on_quit = [](auto& boss)
    {
        boss.LISTEN(tier::anycast, e2::form::quit, item)
        {
            boss.RISEUP(tier::release, e2::form::quit, item);
        };
    };
    const auto closing_by_gesture = [](auto& boss)
    {
        boss.LISTEN(tier::release, hids::events::mouse::button::click::leftright, gear)
        {
            auto backup = boss.This();
            boss.RISEUP(tier::release, e2::form::quit, backup);
            gear.dismiss();
        };
        boss.LISTEN(tier::release, hids::events::mouse::button::click::middle, gear)
        {
            auto backup = boss.This();
            boss.RISEUP(tier::release, e2::form::quit, backup);
            gear.dismiss();
        };
    };
    const auto app_limit = [](auto boss, text title)
    {
        log("app_limit: max count reached");
        auto timeout = datetime::now() + APPS_DEL_TIMEOUT;
        auto shadow = ptr::shadow(boss);
        boss->LISTEN(tier::general, e2::timer::any, timestamp, 0, (shadow))
        {
            if (timestamp > timeout)
            {
                if (auto boss = shadow.lock())
                {
                    log("app_limit: detached");
                    boss->RISEUP(tier::release, e2::form::quit, boss);
                }
            }
        };
        boss->LISTEN(tier::release, e2::form::upon::vtree::attached, parent, 0, (title))
        {
            parent->RISEUP(tier::preview, e2::form::prop::ui::header, title);
        };
    };
    const auto scroll_bars = [](auto master)
    {
        auto scroll_bars = ui::fork::ctor();
            auto scroll_down = scroll_bars->attach(slot::_1, ui::fork::ctor(axis::Y));
                auto hz = scroll_down->attach(slot::_2, ui::grip<axis::X>::ctor(master));
                auto vt = scroll_bars->attach(slot::_2, ui::grip<axis::Y>::ctor(master));
        return scroll_bars;
    };
    const auto underlined_hz_scrollbars = [](auto master)
    {
        auto area = ui::park::ctor();
        auto grip = ui::grip_fx<axis::X>::ctor(master);
        area->branch(snap::stretch, snap::tail, grip)
            ->invoke([&](auto& boss)
            {
                area->visible(grip, faux);
                auto boss_shadow = ptr::shadow(boss.This());
                auto park_shadow = ptr::shadow(area);
                auto grip_shadow = ptr::shadow(grip);
                master->LISTEN(tier::release, e2::form::state::mouse, active, 0, (boss_shadow, park_shadow, grip_shadow))
                {
                    if (auto park_ptr = park_shadow.lock())
                    if (auto grip_ptr = grip_shadow.lock())
                    if (auto boss_ptr = boss_shadow.lock())
                    {
                        auto& boss = *boss_ptr;
                        park_ptr->visible(grip_ptr, active);
                        boss_ptr->base::deface();
                    }
                };
            });
        return area;
    };
    const auto scroll_bars_term = [](auto master)
    {
        auto scroll_bars = ui::fork::ctor();
            auto scroll_head = scroll_bars->attach(slot::_1, ui::fork::ctor(axis::Y));
                auto hz = scroll_head->attach(slot::_1, ui::grip<axis::X>::ctor(master));
                auto vt = scroll_bars->attach(slot::_2, ui::grip<axis::Y>::ctor(master));
        return scroll_bars;
    };
    const auto base_window = [](auto header, auto footer, auto menu_item_id)
    {
        return ui::cake::ctor()
            ->template plugin<pro::d_n_d>()
            ->template plugin<pro::title>(header, footer) //todo "template": gcc complains on ubuntu 18.04
            ->template plugin<pro::limit>(dot_11, twod{ 400,200 }) //todo unify, set via config
            ->template plugin<pro::sizer>()
            ->template plugin<pro::frame>()
            ->template plugin<pro::light>()
            ->template plugin<pro::align>()
            ->invoke([&](auto& boss)
            {
                boss.keybd.active();
                boss.base::kind(base::reflow_root); //todo unify -- See base::reflow()
                boss.LISTEN(tier::preview, e2::form::proceed::d_n_d::drop, what, 0, (menu_item_id))
                {
                    if (auto object = boss.pop_back())
                    {
                        auto target = what.object;
                        what.menuid = menu_item_id;
                        what.object = object;
                        auto& title = boss.template plugins<pro::title>();
                        what.header = title.header();
                        what.footer = title.footer();
                        target->SIGNAL(tier::release, e2::form::proceed::d_n_d::drop, what);
                        boss.base::detach(); // The object kills itself.
                    }
                };
                boss.LISTEN(tier::release, hids::events::mouse::button::dblclick::left, gear)
                {
                    boss.RISEUP(tier::release, e2::form::maximize, gear);
                    gear.dismiss();
                };
                boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                {
                    auto area = boss.base::area();
                    auto home = rect{ -dot_21, area.size + dot_21 * 2}; // Including resizer grips.
                    if (!home.hittest(gear.coord))
                    {
                        auto center = area.coor + (area.size / 2);
                        gear.owner.SIGNAL(tier::release, e2::form::layout::shift, center);
                        boss.base::deface();
                    }
                };
                boss.LISTEN(tier::release, e2::form::proceed::detach, backup)
                {
                    boss.mouse.reset();
                    boss.base::detach(); // The object kills itself.
                };
                boss.LISTEN(tier::release, e2::form::quit, nested_item)
                {
                    boss.mouse.reset();
                    if (nested_item) boss.base::detach(); // The object kills itself.
                };
                boss.LISTEN(tier::release, e2::dtor, p)
                {
                    auto start = datetime::now();
                    auto counter = e2::cleanup.param();
                    SIGNAL_GLOBAL(e2::cleanup, counter);
                    auto stop = datetime::now() - start;
                    log("host: garbage collection",
                    "\n\ttime ", utf::format(stop.count()), "ns",
                    "\n\tobjs ", counter.obj_count,
                    "\n\trefs ", counter.ref_count,
                    "\n\tdels ", counter.del_count);
                };
            });
    };

    using builder_t = std::function<sptr<base>(text, text, xml::settings&, text)>;

    namespace get
    {
        auto& creator()
        {
            static auto creator = std::map<text, builder_t>{};
            return creator;
        }
        auto& configs()
        {
            auto world_ptr = e2::config::whereami.param();
            SIGNAL_GLOBAL(e2::config::whereami, world_ptr);
            auto conf_list_ptr = e2::bindings::list::links.param();
            world_ptr->SIGNAL(tier::request, e2::bindings::list::links, conf_list_ptr);
            auto& conf_list = *conf_list_ptr;
            return conf_list;
        }
    }
    namespace create
    {
        auto& builder(text app_typename)
        {
            static builder_t empty =
            [&](text, text, xml::settings&, text) -> sptr<base>
            {
                auto window = ui::cake::ctor()
                    ->plugin<pro::focus>()
                    ->invoke([&](auto& boss)
                    {
                        boss.keybd.accept(true);
                        closing_on_quit(boss);
                        closing_by_gesture(boss);
                        boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent)
                        {
                            auto title = "error"s;
                            boss.RISEUP(tier::preview, e2::form::prop::ui::header, title);
                        };
                    });
                auto msg = ui::post::ctor()
                    ->colors(whitelt, rgba{ 0x7F404040 })
                    ->upload(ansi::fgc(yellowlt).mgl(4).mgr(4).wrp(wrap::off)
                    + "\n\nUnsupported application type\n\n"
                    + ansi::nil().wrp(wrap::on)
                    + "Only the following application types are supported\n\n"
                    + ansi::nil().wrp(wrap::off).fgc(whitedk)
                    + "   type = DirectVT \n"
                      "   type = ANSIVT   \n"
                      "   type = SHELL    \n"
                      "   type = Group    \n"
                      "   type = Region   \n\n"
                    + ansi::nil().wrp(wrap::on).fgc(whitelt)
                    + "apps: See logs for details."
                    );
                auto placeholder = ui::park::ctor()
                    ->colors(whitelt, rgba{ 0x7F404040 })
                    ->attach(snap::stretch, snap::stretch, msg);
                window->attach(ui::rail::ctor())
                      ->attach(placeholder);
                return window;
            };
            auto& map = get::creator();
            const auto it = map.find(app_typename);
            if (it == map.end())
            {
                log("apps: unknown app type - '", app_typename, "'");
                return empty;
            }
            else return it->second;
        };
        auto by = [](auto& world, auto& gear)
        {
            static auto insts_count = si32{ 0 };
            auto& gate = gear.owner;
            auto location = gear.slot;
            if (gear.meta(hids::anyCtrl))
            {
                log("hall: area copied to clipboard ", location);
                gate.SIGNAL(tier::release, e2::command::printscreen, gear);
            }
            else
            {
                auto what = e2::form::proceed::createat.param();
                what.square = gear.slot;
                what.forced = gear.slot_forced;
                gate.SIGNAL(tier::request, e2::data::changed, what.menuid);
                world.SIGNAL(tier::release, e2::form::proceed::createat, what);
                if (auto& frame = what.object)
                {
                    insts_count++;
                    frame->LISTEN(tier::release, e2::form::upon::vtree::detached, master)
                    {
                        insts_count--;
                        log("hall: detached: ", insts_count);
                    };

                    gear.clear_kb_focus(); // DirectVT app could have a group of focused.
                    gear.kb_focus_changed = faux;
                    frame->SIGNAL(tier::release, hids::events::upevent::kboffer, gear);
                    frame->SIGNAL(tier::anycast, e2::form::upon::created, gear); // Tile should change the menu item.
                }
            }
        };
        auto go = [](auto& menuid)
        {
            auto& conf_list = app::shared::get::configs();
            auto& config = conf_list[menuid];
            auto& creator = app::shared::create::builder(config.type);
            auto object = creator(config.cwd, config.param, config.settings, config.patch);
            if (config.bgc     ) object->SIGNAL(tier::anycast, e2::form::prop::colors::bg,   config.bgc);
            if (config.fgc     ) object->SIGNAL(tier::anycast, e2::form::prop::colors::fg,   config.fgc);
            if (config.slimmenu) object->SIGNAL(tier::anycast, e2::form::prop::ui::slimmenu, config.slimmenu);
            return std::pair{ object, config };
        };
        auto at = [](auto& world, auto& what)
        {
            auto [object, config] = app::shared::create::go(what.menuid);

            auto window = app::shared::base_window(config.title, config.footer, what.menuid);
            if (config.winsize && !what.forced) window->extend({what.square.coor, config.winsize });
            else                                window->extend(what.square);
            window->attach(object);
            log("apps: app type: ", utf::debase(config.type), ", menu item id: ", utf::debase(what.menuid));
            world.branch(what.menuid, window, !config.hidden);
            window->SIGNAL(tier::anycast, e2::form::upon::started, world.This());

            what.object = window;
        };
        auto from = [](auto& world, auto& what)
        {
            auto& conf_list = app::shared::get::configs();
            auto& config = conf_list[what.menuid];
            auto  window = app::shared::base_window(what.header, what.footer, what.menuid);

            window->extend(what.square);
            window->attach(what.object);
            log("apps: attach type=", utf::debase(config.type), " menu_item_id=", utf::debase(what.menuid));
            world.branch(what.menuid, window, !config.hidden);
            window->SIGNAL(tier::anycast, e2::form::upon::started, world.This());

            what.object = window;
        };
    }
    auto activate = [](auto world_ptr, xml::settings& config)
    {
        auto& world = *world_ptr;
        world.LISTEN(tier::release, e2::form::proceed::createby, gear)
        {
            create::by(world, gear);
        };
        world.LISTEN(tier::release, e2::form::proceed::createat, what)
        {
            create::at(world, what);
        };
        world.LISTEN(tier::release, e2::form::proceed::createfrom, what)
        {
            create::from(world, what);
        };
        world.autorun(config);
    };

    namespace load
    {
        template<bool Print = faux>
        auto settings(view cli_config_path, view patch)
        {
            auto conf = xml::settings{ default_config };
            auto pads = "      ";
            auto load = [&](view shadow)
            {
                if (shadow.empty()) return faux;
                if (shadow.starts_with(":"))
                {
                    shadow.remove_prefix(1);
                    auto utf8 = os::ipc::memory::get(shadow);
                    if (utf8.size())
                    {
                        conf.fuse<Print>(utf8);
                        return true;
                    }
                    else
                    {
                        log("apps: failed to get configuration from :", shadow);
                        return faux;
                    }
                }
                auto path = text{ shadow };
                log("apps: loading configuration from ", path, "...");
                if (path.starts_with("$"))
                {
                    auto temp = path.substr(1);
                    path = os::env::get(temp);
                    if (path.empty()) return faux;
                    log(pads, temp, " = ", path);
                }
                auto config_path = path.starts_with("~/") ? os::env::homepath() / path.substr(2)
                                                          : fs::path{ path };
                auto ec = std::error_code{};
                auto config_file = fs::directory_entry(config_path, ec);
                if (!ec && (config_file.is_regular_file() || config_file.is_symlink()))
                {
                    auto config_path_str = "'" + config_path.string() + "'";
                    utf::change(config_path_str, "\\", "/");
                    auto file = std::ifstream(config_file.path(), std::ios::binary | std::ios::in);
                    if (file.seekg(0, std::ios::end).fail())
                    {
                        log(pads, "failed\n\tunable to get configuration file size, skip it: ", config_path_str);
                        return faux;
                    }
                    else
                    {
                        log(pads, "reading configuration: ", config_path_str);
                        auto size = file.tellg();
                        auto buff = text(size, '\0');
                        file.seekg(0, std::ios::beg);
                        file.read(buff.data(), size);
                        conf.fuse<Print>(buff, config_path.string());
                        return true;
                    }
                }
                log(pads, "no configuration found, try another source");
                return faux;
            };
            if (!load(cli_config_path)
             && !load(app::shared::env_config)
             && !load(app::shared::usr_config))
            {
                log(pads, "fallback to hardcoded configuration");
            }

            os::env::set(app::shared::env_config.substr(1)/*remove $*/, conf.document->page.file);

            conf.fuse<Print>(patch);
            return conf;
        }
    }

    struct initialize
    {
        initialize(view app_typename, builder_t builder)
        {
            app::shared::get::creator()[text{ app_typename }] = builder;
        }
    };

    auto start(text app_name, text log_title, si32 vtmode, xml::settings& config)
    {
        auto direct = !!(vtmode & os::vt::direct);
        if (!direct) os::logging::start(log_title);

        //std::this_thread::sleep_for(15s);

        auto shadow = app_name;
        utf::to_low(shadow);
        //if (!config.cd("/config/" + shadow)) config.cd("/config/appearance/");
        config.cd("/config/appearance/runapp/", "/config/appearance/defaults/");
        auto runapp = [&](auto uplink)
        {
            auto patch = ""s;
            auto ground = base::create<host>(uplink, config);
            auto aclass = utf::to_low(utf::cutoff(app_name, ' '));
            auto params = utf::remain(app_name, ' ');
            auto applet = app::shared::create::builder(aclass)("", (direct ? "" : "!") + params, config, patch); // ! - means simple (w/o plugins)
            auto window = ground->template invite<gate>(vtmode, config);
            window->launch(uplink, applet);
            window.reset();
            applet.reset();
            ground->shutdown();
        };

        if (direct)
        {
            auto server = os::ipc::stdio();
            runapp(server);
        }
        else
        {
            //todo Clang 11.0.1 doesn't get it
            //auto [client, server] = os::ipc::xlink();
            auto xlinks = os::ipc::xlink();
            auto client = xlinks.first;
            auto server = xlinks.second;
            auto thread = std::thread{ [&]{ os::tty::splice(client, vtmode); }};
            runapp(server);
            thread.join();
        }
        return true;
    }
}
