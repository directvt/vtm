// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_APPS_HPP
#define NETXS_APPS_HPP


#define APPS_DEL_TIMEOUT 1s

#include "ui/controls.hpp"
#include "text/xml.hpp"

#include <fstream>

namespace netxs::app
{
    namespace fs = std::filesystem;

    using namespace std::placeholders;
    using namespace netxs::console;
    using namespace netxs;

    using slot = ui::slot;
    using axis = ui::axis;
    using axes = ui::axes;
    using snap = ui::snap;
    using id_t = netxs::input::id_t;
}

namespace netxs::app::shared
{
    static constexpr auto default_config = R"==(
<config>
    <menu>
        <selected=Term /> <!-- Set selected using menu item id. -->
        <item*/>  <!-- Use asterisk at the end of the element name to set defaults.
                       Using an asterisk with the parameter name of the first element in the list without any other nested arguments
                       indicates the beginning of the list, i.e. the list will replace the existing one when the configuration is merged. -->
        <item splitter label="apps">
            <notes>
                " Default applications group                         \n"
                " It can be configured in ~/.config/vtm/settings.xml "
            </notes>
        </item>
        <item* hidden=no slimmenu=false type=SHELL fgc=whitedk bgc=0x00000000 winsize=0,0 wincoor=0,0 />
        <item id=Test label="Test" type=DirectVT title="Terminal Emulator" notes=" menu item for testing configuration options (e.g., window style) " param="$0 -r term">
            <config>   <!-- The following config partially overrides the base configuration. It is valid for DirectVT apps only. -->
                <term>
                    <scrollback>
                        <size=15000 />   <!-- Scrollback buffer length. -->
                        <wrap="off" />   <!-- Lines wrapping mode. -->
                    </scrollback>
                    <menu>
                        <autohide = off />  <!--  If true, show menu only on hover. -->
                        <slim = false   />
                    </menu>
                    <selection>
                        <mode = none /> <!-- text | ansi | rich | html | protected | none -->
                    </selection>
                </term>
            </config>
        </item>
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
            <hotkeys>    <!-- not implemented -->
                <key*/>
                <key="Ctrl+'t'" action=start />
                <key="Ctrl+'z'" action=close />
            </hotkeys>
            <config>   <!-- The following config partially overrides the base configuration. It is valid for DirectVT apps only. -->
                <term>
                    <scrollback>
                        <size=40000    />   <!-- Scrollback buffer length. -->
                        <wrap="on"     />   <!-- Lines wrapping mode. -->
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
                    <hotkeys>    <!-- not implemented -->
                        <key*/>
                        <key="Alt+RightArrow" action=findNext />
                        <key="Alt+LeftArrow"  action=findPrev />
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
        <item id=Tile       label="Tile"       type=Group    title="Tiling Window Manager" param="h1:1(Term, Term)" notes=" run Tiling Window Manager with two terminals attached "/>
        <item id=View       label=View         type=Region   title="\e[11:3pView: Region"                           notes=" set desktop region "/>
        <item id=Settings   label=Settings     type=DirectVT title="Settings"              param="$0 -r settings"   notes=" run Settings " winsize=50,15 />
        <item id=Logs       label=Logs         type=DirectVT title="Logs Title"            param="$0 -r logs"       notes=" run Logs "/>
        <autorun>    <!-- not implemented -->
            <item*/>
            <item*=Term winsize=48%,48% /> <!-- item*=_item_id_ - assign the same _item_id_ to each item by default. -->
            <item wincoor=0,0 />
            <item wincoor=52%,0 />
            <item wincoor=0,52% />
            <item=mc wincoor=52%,52% />
        </autorun>
        <width>    <!-- not implemented -->
            <folded=4/>
            <expanded=31/>
        </width>
    </menu>
    <hotkeys>    <!-- not implemented -->
        <key*/>
        <key="Ctrl+PgUp" action=prevWindow />
        <key="Ctrl+PgDn" action=nextWindow />
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
            <fader duration=0ms fast=0ms />  <!-- Fader animation config. -->
        </defaults>
        <runapp>    <!-- Override defaults. -->
            <brighter=0 />
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
        <background fgc=whitedk bgc=0xFF000000 />  <!-- Desktop background color. -->
        <clipboard>
            <preview enabled=true size=80x25 bgc=bluedk fgc=whitelt>
                <alpha=0xFF />  <!-- Preview alpha is applied only to the ansi/rich/html text type -->
                <timeout=3s />  <!-- Preview hiding timeout. Set it to zero to disable hiding. -->
                <shadow=7   />  <!-- Preview shadow strength (0-10). -->
            </preview>
        </clipboard>
        <viewport coor=0,0 />
        <mouse dblclick=500ms />
        <tooltip timeout=500ms enabled=true fgc=pureblack bgc=purewhite />
        <glowfx=true />                      <!-- Show glow effect around selected item. -->
        <debug overlay=faux toggle="🐞" />  <!-- Display console debug info. -->
        <regions enabled=faux />             <!-- Highlight UI objects boundaries. -->
    </client>
    <term>      <!-- Base configuration for the Term app. It can be partially overridden by the menu item's config subarg. -->
        <scrollback>
            <size=20000    />   <!-- Scrollback buffer length. -->
            <growstep=0    />   <!-- Scrollback buffer grow step. The buffer behaves like a ring in case of zero. -->
            <maxline=65535 />   <!-- Max line length. Line splits if it exceeds the limit. -->
            <wrap="on"     />   <!-- Lines wrapping mode. -->
            <reset onkey="on" onoutput="off" />   <!-- Scrollback viewport reset triggers. -->
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
            <match fx=selection bgc="0xFF007F00" fgc=whitelt />  <!-- Color of the selected text occurrences. Set fx to use cell::shaders: xlight | selection | contrast | invert | reverse -->
            <selection>
                <text fx=selection bgc=bluelt fgc=whitelt />  <!-- Highlighting of the selected text in plaintext mode. -->
                <protected fx=selection bgc=bluelt fgc=whitelt />
                <ansi fx=xlight/>
                <rich fx=xlight/>
                <html fx=xlight/>
                <none fx=selection bgc=blacklt fgc=whitedk />  <!-- Inactive selection color. -->
            </selection>
        </color>
        <fields>
            <lucent=0xC0 /> <!-- Fields transparency level. -->
            <size=0      /> <!-- Left/right field size (for hz scrolling UX). -->
        </fields>
        <tablen=8 />   <!-- Tab length. -->
        <cursor>
            <style="underline"/> <!-- block | underline -->
            <blink=400ms/>       <!-- blink period -->
            <show=true/>
        </cursor>
        <menu>
            <autohide=true/>  <!--  If true/on, show menu only on hover. -->
            <enabled="on"/>
            <slim=true />
        </menu>
        <selection>
            <mode="text"/> <!-- text | ansi | rich | html | protected | none -->
        </selection>
        <atexit = auto /> <!-- auto:    Stay open and ask if exit code != 0. (default)
                               ask:     Stay open and ask.
                               close:   Always close.
                               restart: Restart session.
                               retry:   Restart session if exit code != 0. -->
        <hotkeys>    <!-- not implemented -->
            <key*/>
            <key="Alt+RightArrow" action=findNext />
            <key="Alt+LeftArrow"  action=findPrev />
        </hotkeys>
    </term>
    <defapp>
        <menu>
            <autohide=faux />  <!--  If true, show menu only on hover. -->
            <enabled="on"/>
            <slim=faux />
        </menu>
    </defapp>
    <tile>
        <menu>
            <autohide=true />  <!--  If true, show menu only on hover. -->
            <enabled="on"/>
            <slim=1 />
        </menu>
    </tile>
    <text>      <!-- Base configuration for the Text app. It can be overridden by param's subargs. -->
        <!-- not implemented -->
    </text>
    <calc>      <!-- Base configuration for the Calc app. It can be overridden by param's subargs. -->
        <!-- not implemented -->
    </calc>
    <logs>      <!-- Base configuration for the Logs app. It can be overridden by param's subargs. -->
        <!-- not implemented -->
    </logs>
    <settings>      <!-- Base configuration for the Settings app. It can be overridden by param's subargs. -->
        <!-- not implemented -->
    </settings>
</config>
)==";

    static constexpr auto usr_config = "~/.config/vtm/settings.xml";
    static constexpr auto env_config = "$VTM_CONFIG"sv;

    static constexpr auto path_autorun  = "config/menu/autorun";
    static constexpr auto path_hotkeys  = "config/hotkeys";

    using menu_item_type = std::tuple<bool, text, text, std::function<void(ui::pads&)>>;
    using menu_list_type = std::list<menu_item_type>;

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
        boss.SUBMIT(tier::anycast, e2::form::quit, item)
        {
            boss.base::template riseup<tier::release>(e2::form::quit, item);
        };
    };
    const auto closing_by_gesture = [](auto& boss)
    {
        boss.SUBMIT(tier::release, hids::events::mouse::button::click::leftright, gear)
        {
            auto backup = boss.This();
            boss.base::template riseup<tier::release>(e2::form::quit, backup);
            gear.dismiss();
        };
        boss.SUBMIT(tier::release, hids::events::mouse::button::click::middle, gear)
        {
            auto backup = boss.This();
            boss.base::template riseup<tier::release>(e2::form::quit, backup);
            gear.dismiss();
        };
    };
    const auto app_limit = [](auto boss, text title)
    {
        log("app_limit: max count reached");
        auto timeout = tempus::now() + APPS_DEL_TIMEOUT;
        auto shadow = ptr::shadow(boss);
        boss->SUBMIT_BYVAL(tier::general, e2::timer::any, timestamp)
        {
            if (timestamp > timeout)
            {
                if (auto boss = shadow.lock())
                {
                    log("app_limit: detached");
                    boss->base::template riseup<tier::release>(e2::form::quit, boss);
                }
            }
        };
        boss->SUBMIT_BYVAL(tier::release, e2::form::upon::vtree::attached, parent)
        {
            parent->base::riseup<tier::preview>(e2::form::prop::ui::header, title);
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
                master->SUBMIT_BYVAL(tier::release, e2::form::state::mouse, active)
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

    // Menu bar (shrinkable on right-click).
    const auto custom_menu = [](xml::settings& config, app::shared::menu_list_type menu_items)
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

        auto menu_area = ui::fork::ctor()
                        ->active();
            auto inner_pads = dent{ 1,2,1,1 };
            auto menu_list = menu_area->attach(slot::_1, ui::fork::ctor());

                menu_list->attach(slot::_1, ui::pads::ctor(inner_pads, dent{ 0 }))
                         ->plugin<pro::fader>(x3, c3, skin::timeout(tone::fader))
                         ->plugin<pro::notes>(" Maximize/restore window ")
                         ->invoke([&](ui::pads& boss)
                         {
                             boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                             {
                                 boss.base::template riseup<tier::release>(e2::form::maximize, gear);
                                 gear.dismiss();
                             };
                         })
                         ->attach(ui::item::ctor(" ≡", faux, true));

                auto scrl_area = menu_list->attach(slot::_2, ui::cake::ctor());
                auto scrl_rail = scrl_area->attach(ui::rail::ctor(axes::X_ONLY, axes::X_ONLY));
                auto scrl_list = scrl_rail->attach(ui::list::ctor(axis::X));

                auto scroll_hint = ui::park::ctor();
                auto hints = scroll_hint->attach(snap::stretch, menusize ? snap::center : snap::tail, ui::grip_fx<axis::X>::ctor(scrl_rail));

                auto scrl_grip = scrl_area->attach(scroll_hint);

            auto fader = skin::timeout(tone::fader);
            for (auto& body : menu_items)
            {
                auto& hover = std::get<0>(body);
                auto& label = std::get<1>(body);
                auto& notes = std::get<2>(body);
                auto& setup = std::get<3>(body);
                if (hover)
                {
                    scrl_list->attach(ui::pads::ctor(inner_pads, dent{ 1 }))
                             ->plugin<pro::fader>(x3, c3, fader)
                             ->plugin<pro::notes>(notes)
                             ->invoke(setup)
                             ->attach(ui::item::ctor(label, faux, true));
                }
                else
                {
                    scrl_list->attach(ui::pads::ctor(inner_pads, dent{ 1 }))
                             ->colors(0,0) //todo for mouse tracking
                             ->plugin<pro::notes>(notes)
                             ->invoke(setup)
                             ->attach(ui::item::ctor(label, faux, true));
                }
            }
            menu_area->attach(slot::_2, ui::pads::ctor(dent{ 2,2,1,1 }, dent{}))
                     ->plugin<pro::fader>(x1, c1, fader)
                     ->plugin<pro::notes>(" Close window ")
                     ->invoke([&](auto& boss)
                     {
                         boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                         {
                             boss.base::template riseup<tier::release>(e2::form::quit, boss.This());
                             gear.dismiss();
                         };
                     })
                     ->attach(ui::item::ctor("×"));

        auto menu_block = ui::park::ctor()
            ->plugin<pro::limit>(twod{ -1, menusize ? 1 : 3 }, twod{ -1, menusize ? 1 : 3 })
            ->invoke([&](ui::park& boss)
            {
                scroll_hint->visible(hints, faux);
                auto boss_shadow = ptr::shadow(boss.This());
                auto park_shadow = ptr::shadow(scroll_hint);
                auto grip_shadow = ptr::shadow(hints);
                boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::click::right, gear)
                {
                    if (auto park_ptr = park_shadow.lock())
                    if (auto grip_ptr = grip_shadow.lock())
                    if (auto boss_ptr = boss_shadow.lock())
                    {
                        auto& boss = *boss_ptr;
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
                boss.SUBMIT_BYVAL(tier::anycast, e2::form::prop::ui::slimmenu, slim)
                {
                    auto size = slim ? 1 : 3;
                    if (auto park_ptr = park_shadow.lock())
                    if (auto grip_ptr = grip_shadow.lock())
                    if (auto boss_ptr = boss_shadow.lock())
                    {
                        auto& boss = *boss_ptr;
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
                    boss.SUBMIT_BYVAL(tier::release, e2::form::state::mouse, active)
                    {
                        if (auto park_ptr = park_shadow.lock())
                        if (auto grip_ptr = grip_shadow.lock())
                        if (auto boss_ptr = boss_shadow.lock())
                        {
                            park_ptr->visible(grip_ptr, active);
                            boss_ptr->base::deface();
                        }
                    };
                }
            });
        menu_block->attach(snap::stretch, snap::center, menu_area);

        auto menu = slot1->attach(menu_block);
                auto border = slot1->attach(ui::mock::ctor())
                                   ->plugin<pro::limit>(twod{ -1,1 }, twod{ -1,1 });
                     if (menushow == faux) autohide = faux;
                else if (autohide == faux) slot1->roll();
                slot1->invoke([&](auto& boss)
                {
                    auto menu_shadow = ptr::shadow(menu_block);
                    auto boss_shadow = ptr::shadow(boss.This());
                    auto hide_shadow = ptr::shared(autohide);
                    boss.SUBMIT_BYVAL(tier::release, e2::form::state::mouse, hits)
                    {
                        if (*hide_shadow)
                        if (auto menu_ptr = menu_shadow.lock())
                        if (auto boss_ptr = boss_shadow.lock())
                        {
                            auto& boss = *boss_ptr;
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
    const auto main_menu = [](xml::settings& config)
    {
        auto items = app::shared::menu_list_type
        {
            { true, ansi::und(true).add("F").nil().add("ile"), " File menu item ", [&](auto& boss){ } },
            { true, ansi::und(true).add("E").nil().add("dit"), " Edit menu item ", [&](auto& boss){ } },
            { true, ansi::und(true).add("V").nil().add("iew"), " View menu item ", [&](auto& boss){ } },
            { true, ansi::und(true).add("D").nil().add("ata"), " Data menu item ", [&](auto& boss){ } },
            { true, ansi::und(true).add("H").nil().add("elp"), " Help menu item ", [&](auto& boss){ } },
        };
        config.cd("/config/defapp/");
        auto [menu, cover, menu_data] = custom_menu(config, items);
        return menu;
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
                auto shadow = ptr::shadow(boss.This());
                boss.SUBMIT_BYVAL(tier::preview, e2::form::proceed::d_n_d::drop, what)
                {
                    if (auto boss_ptr = shadow.lock())
                    if (auto object = boss_ptr->pop_back())
                    {
                        auto& boss = *boss_ptr;
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
                boss.SUBMIT(tier::release, hids::events::mouse::button::dblclick::left, gear)
                {
                    boss.base::template riseup<tier::release>(e2::form::maximize, gear);
                    gear.dismiss();
                };
                boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
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
                boss.SUBMIT(tier::release, e2::form::proceed::detach, backup)
                {
                    boss.mouse.reset();
                    boss.base::detach(); // The object kills itself.
                };
                boss.SUBMIT(tier::release, e2::form::quit, nested_item)
                {
                    boss.mouse.reset();
                    if (nested_item) boss.base::detach(); // The object kills itself.
                };
                boss.SUBMIT(tier::release, e2::dtor, p)
                {
                    auto start = tempus::now();
                    auto counter = e2::cleanup.param();
                    SIGNAL_GLOBAL(e2::cleanup, counter);
                    auto stop = tempus::now() - start;
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
                        boss.SUBMIT(tier::release, e2::form::upon::vtree::attached, parent)
                        {
                            auto title = "error"s;
                            boss.base::template riseup<tier::preview>(e2::form::prop::ui::header, title);
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
                    frame->SUBMIT(tier::release, e2::form::upon::vtree::detached, master)
                    {
                        insts_count--;
                        log("hall: detached: ", insts_count);
                    };

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
    auto activate = [](auto world_ptr)
    {
        auto& world = *world_ptr;
        world.SUBMIT(tier::release, e2::form::proceed::createby, gear)
        {
            create::by(world, gear);
        };
        world.SUBMIT(tier::release, e2::form::proceed::createat, what)
        {
            create::at(world, what);
        };
        world.SUBMIT(tier::release, e2::form::proceed::createfrom, what)
        {
            create::from(world, what);
        };
    };

    namespace load
    {
        auto settings(view cli_config_path, view patch)
        {
            auto conf = xml::settings{ default_config };
            auto load = [&](view shadow)
            {
                if (shadow.empty()) return faux;
                auto path = text{ shadow };
                log("apps: loading configuration from ", path, "...");
                if (path.starts_with("$"))
                {
                    auto temp = path.substr(1);
                    path = os::get_env(temp);
                    if (path.empty()) return faux;
                    log('\t', temp, " = ", path);
                }
                auto config_path = path.starts_with("~/") ? os::homepath() / path.substr(2)
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
                        log("\tfailed\n\tunable to get configuration file size, skip it: ", config_path_str);
                        return faux;
                    }
                    else
                    {
                        log("\treading configuration: ", config_path_str);
                        auto size = file.tellg();
                        auto buff = text(size, '\0');
                        file.seekg(0, std::ios::beg);
                        file.read(buff.data(), size);
                        conf.fuse(buff, config_path.string());
                        return true;
                    }
                }
                log("\tno configuration found, try another source");
                return faux;
            };
            if (!load(cli_config_path)
             && !load(app::shared::env_config)
             && !load(app::shared::usr_config))
            {
                log("apps: fallback to hardcoded configuration");
            }

            os::set_env(app::shared::env_config.substr(1)/*remove $*/, conf.document->page.file);

            conf.fuse(patch);
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
        auto direct = !!(vtmode & os::legacy::direct);
        if (!direct) os::start_log(log_title);

        //std::this_thread::sleep_for(15s);

        auto shadow = app_name;
        utf::to_low(shadow);
        if (!config.cd("/config/" + shadow)) config.cd("/config/appearance/");

        auto tunnel = os::ipc::local(vtmode);
        auto cons = os::tty::proxy(tunnel.second);
        auto size = cons.ignite(vtmode);
        if (!size.last) return faux;

        config.cd("/config/appearance/runapp/", "/config/appearance/defaults/");
        auto ground = base::create<host>(tunnel.first, config);
        auto runapp = [&]
        {
            auto patch = ""s;
            auto aclass = utf::cutoff(app_name, ' ');
            utf::to_low(aclass);
            auto params = utf::remain(app_name, ' ');
            auto applet = app::shared::create::builder(aclass)("", (direct ? "" : "!") + params, config, patch); // ! - means simple (w/o plugins)
            auto window = ground->invite<gate>(vtmode, config);
            window->resize(size);
            window->launch(tunnel.first, applet);
            window.reset();
            applet.reset();
            ground->shutdown();
        };

        if (direct) runapp();
        else
        {
            auto thread = std::thread{ [&]{ os::ipc::splice(cons, vtmode); }};
            runapp();
            thread.join();
        }
        return true;
    }
}

#include "apps/term.hpp"
#include "apps/tile.hpp"
#include "apps/calc.hpp"
#include "apps/text.hpp"
#include "apps/shop.hpp"
#include "apps/logs.hpp"
#include "apps/test.hpp"
#include "apps/desk.hpp"

namespace netxs::app::shared
{
    namespace
    {
        auto build_Strobe        = [](text cwd, text v,     xml::settings& config, text patch)
        {
            auto window = ui::cake::ctor();
            auto strob = window->plugin<pro::focus>()
                               ->plugin<pro::notes>(" Left+Right click to close ")
                               ->invoke([](auto& boss)
                                {
                                    boss.keybd.accept(true);
                                    closing_by_gesture(boss);
                                    closing_on_quit(boss);
                                })
                               ->attach(ui::mock::ctor());
            auto strob_shadow = ptr::shadow(strob);
            auto stobe_state = true;
            strob->SUBMIT_BYVAL(tier::general, e2::timer::any, now)
            {
                stobe_state = !stobe_state;
                if (auto strob = strob_shadow.lock())
                {
                    strob->color(0x00, stobe_state ? 0xFF000000 : 0xFFFFFFFF);
                    strob->deface();
                }
            };
            return window;
        };
        auto build_Settings      = [](text cwd, text v,     xml::settings& config, text patch)
        {
            auto window = ui::cake::ctor();
            window->plugin<pro::focus>()
                  ->plugin<pro::cache>()
                  ->plugin<pro::notes>(" Left+Right click to close ")
                  ->attach(ui::stem_rate<tier::preview, decltype(e2::config::fps)>::ctor("Set frame rate limit", 1, 200, "fps"))
                  ->colors(0xFFFFFFFF, bluedk)
                  ->invoke([&](auto& boss)
                  {
                        boss.keybd.accept(true);
                        closing_by_gesture(boss);
                        closing_on_quit(boss);
                        boss.SUBMIT(tier::anycast, e2::form::prop::colors::any, clr)
                        {
                            auto deed = boss.bell::template protos<tier::anycast>();
                                 if (deed == e2::form::prop::colors::bg.id) boss.base::color(boss.base::color().fgc(), clr);
                            else if (deed == e2::form::prop::colors::fg.id) boss.base::color(clr, boss.base::color().bgc());
                        };
                  });
            return window;
        };
        auto build_Empty         = [](text cwd, text v,     xml::settings& config, text patch)
        {
            auto window = ui::cake::ctor();
            window->plugin<pro::focus>()
                  ->plugin<pro::track>()
                  ->plugin<pro::acryl>()
                  ->plugin<pro::notes>(" Left+Right click to close ")
                  ->invoke([&](auto& boss)
                  {
                      boss.keybd.accept(true);
                      closing_by_gesture(boss);
                      closing_on_quit(boss);
                      boss.SUBMIT(tier::release, e2::form::upon::vtree::attached, parent)
                      {
                          auto title = ansi::add("Empty Instance \nid: ", parent->id);
                          boss.base::template riseup<tier::preview>(e2::form::prop::ui::header, title);
                      };
                  });
            auto object = window->attach(ui::mock::ctor())
                                ->colors(0,0); //todo mouse tracking
            return window;
        };
        auto build_Region        = [](text cwd, text v,     xml::settings& config, text patch)
        {
            auto window = ui::cake::ctor();
            window->invoke([&](auto& boss)
                    {
                        //todo reimplement (tiling/window)
                        //boss.SUBMIT(tier::release, hids::events::mouse::button::dblclick::left, gear)
                        //{
                        //    auto outer = e2::config::plugins::sizer::outer.param();
                        //    boss.base::template riseup<tier::request>(e2::config::plugins::sizer::outer, outer);
                        //    auto actual_rect = rect{ dot_00, boss.base::size() } + outer;
                        //    if (actual_rect.hittest(gear.coord))
                        //    {
                        //        rect viewport;
                        //        gate.owner.SIGNAL(tier::request, e2::form::prop::viewport, viewport);
                        //        boss.base::extend(viewport);
                        //        gear.dismiss();
                        //    }
                        //};
                        closing_on_quit(boss);
                        boss.SUBMIT(tier::release, e2::render::prerender, parent_canvas)
                        {
                            auto title_fg_color = rgba{ 0xFFffffff };
                            auto area = parent_canvas.full();
                            auto mark = skin::color(tone::shadower);
                            mark.fgc(title_fg_color).link(boss.bell::id);
                            auto fill = [&](cell& c) { c.fusefull(mark); };
                            parent_canvas.cage(area, dot_21, fill);
                        };
                        boss.SUBMIT(tier::release, e2::form::upon::vtree::attached, parent_ptr)
                        {
                            auto& parent = *parent_ptr;
                            closing_by_gesture(parent);

                            //todo too hacky
                            if (auto form_ptr = std::dynamic_pointer_cast<ui::cake>(parent_ptr))
                            {
                                form_ptr->plugin<pro::notes>(" Right click to set title from clipboard. Left+Right to close. ");
                            }

                            static auto i = 0; i++;
                            auto title = ansi::add("View\nRegion ", i);
                            boss.base::template riseup<tier::preview>(e2::form::prop::ui::header, title);

                            auto outer = dent{ 2,2,1,1 };
                            auto inner = dent{ -4,-4,-2,-2 };
                            boss.base::template riseup<tier::release>(e2::config::plugins::sizer::outer, outer);
                            boss.base::template riseup<tier::release>(e2::config::plugins::sizer::inner, inner);
                            boss.base::template riseup<tier::release>(e2::config::plugins::align, faux);
                            boss.base::template riseup<tier::preview>(e2::form::prop::zorder, Z_order::backmost);
                            parent.SUBMIT(tier::release, hids::events::mouse::button::click::right, gear)
                            {
                                auto old_title = e2::form::prop::ui::header.param();
                                boss.base::template riseup<tier::request>(e2::form::prop::ui::header, old_title);

                                auto data = gear.get_clip_data();

                                if (utf::is_plain(data.utf8)) // Reset aligning to the center if text is plain.
                                {
                                    auto align = ansi::jet(bias::center);
                                    boss.base::template riseup<tier::preview>(e2::form::prop::ui::header, align);
                                }
                                // Copy clipboard data to title.
                                auto title = e2::form::prop::ui::header.param(data.utf8);
                                boss.base::template riseup<tier::preview>(e2::form::prop::ui::header, title);
                                gear.dismiss();

                                if (old_title.size()) // Copy old title to clipboard.
                                {
                                    gear.set_clip_data(clip{ dot_00, old_title, clip::ansitext });
                                }
                            };
                        };
                    });
            return window;
        };
        auto build_Truecolor     = [](text cwd, text v,     xml::settings& config, text patch)
        {
            #pragma region samples
                //todo put all ansi art into external files
                auto r_grut00 = ansi::wrp(wrap::off).rlf(feed::fwd).jet(bias::center).add(
                    "\033[0m\033[s"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;1;1;1m▄\033[48;2;0;0;0m\033[38;2;9;8;8m▄\033[48;2;0;0;0m\033[38;2;20;17;15m▄\033[48;2;0;0;0m\033[38;2;19;16;13m▄\033[48;2;0;0;0m\033[38;2;7;6;5m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;1;1;1m▄\033[48;2;0;0;0m\033[38;2;7;7;6m▄\033[48;2;0;0;0m\033[38;2;6;6;5m▄\033[48;2;0;0;0m\033[38;2;1;1;1m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;2;2;2m▄\033[48;2;3;3;3m\033[38;2;25;23;21m▄\033[48;2;21;18;17m\033[38;2;80;60;45m▄\033[48;2;64;49;38m\033[38;2;124;88;64m▄\033[48;2;89;64;48m\033[38;2;136;94;68m▄\033[48;2;84;61;46m\033[38;2;134;95;69m▄\033[48;2;45;35;28m\033[38;2;99;72;53m▄\033[48;2;8;7;7m\033[38;2;32;26;23m▄\033[48;2;0;0;0m\033[38;2;2;2;1m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;1;1;1m▄\033[48;2;4;4;4m\033[38;2;26;24;21m▄\033[48;2;34;31;27m\033[38;2;104;79;61m▄\033[48;2;77;62;49m\033[38;2;136;100;74m▄\033[48;2;73;58;47m\033[38;2;137;101;75m▄\033[48;2;36;32;27m\033[38;2;107;80;61m▄\033[48;2;9;9;8m\033[38;2;58;47;40m▄\033[48;2;0;0;0m\033[38;2;14;13;12m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;8;8;8m▄\033[48;2;17;16;15m\033[38;2;54;43;37m▄\033[48;2;67;50;41m\033[38;2;104;78;60m▄\033[48;2;121;89;67m\033[38;2;134;103;82m▄\033[48;2;141;105;78m\033[38;2;144;115;90m▄\033[48;2;152;113;84m\033[38;2;153;122;95m▄\033[48;2;151;111;83m\033[38;2;148;117;92m▄\033[48;2;126;91;67m\033[38;2;134;104;80m▄\033[48;2;65;52;42m\033[38;2;92;70;54m▄\033[48;2;9;9;8m\033[38;2;22;20;18m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;6;6;5m\033[38;2;14;14;13m▄\033[48;2;61;50;40m\033[38;2;88;68;52m▄\033[48;2;134;98;72m\033[38;2;144;108;83m▄\033[48;2;145;109;82m\033[38;2;149;120;95m▄\033[48;2;153;117;89m\033[38;2;159;132;106m▄\033[48;2;140;105;79m\033[38;2;157;129;103m▄\033[48;2;112;83;63m\033[38;2;144;111;86m▄\033[48;2;58;48;39m\033[38;2;110;82;62m▄\033[48;2;9;9;8m\033[38;2;43;37;33m▄\033[48;2;0;0;0m\033[38;2;5;5;4m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;1;1;1m▄\033[48;2;3;3;3m\033[38;2;17;15;13m▄\033[48;2;33;28;24m\033[38;2;73;57;45m▄\033[48;2;94;72;55m\033[38;2;125;100;79m▄\033[48;2;126;101;81m\033[38;2;127;109;92m▄\033[48;2;129;107;89m\033[38;2;113;99;87m▄\033[48;2;140;117;97m\033[38;2;123;106;93m▄\033[48;2;143;120;100m\033[38;2;132;112;97m▄\033[48;2;136;113;94m\033[38;2;132;114;97m▄\033[48;2;132;108;87m\033[38;2;139;116;97m▄\033[48;2;116;89;69m\033[38;2;133;103;81m▄\033[48;2;38;33;28m\033[38;2;57;47;41m▄\033[48;2;1;1;1m\033[38;2;4;4;4m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;1;0m\033[38;2;1;1;1m▄\033[48;2;25;23;20m\033[38;2;41;35;30m▄\033[48;2;106;80;61m\033[38;2;119;90;67m▄\033[48;2;144;114;90m\033[38;2;135;109;89m▄\033[48;2;148;124;103m\033[38;2;144;123;104m▄\033[48;2;150;129;110m\033[38;2;140;122;107m▄\033[48;2;153;133;113m\033[38;2;139;122;107m▄\033[48;2;151;126;103m\033[38;2;142;123;106m▄\033[48;2;141;109;84m\033[38;2;146;122;99m▄\033[48;2;86;64;50m\033[38;2;125;94;71m▄\033[48;2;29;27;25m\033[38;2;70;55;45m▄\033[48;2;2;2;2m\033[38;2;16;15;15m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;5;5;5m\033[38;2;19;16;15m▄\033[48;2;43;36;30m\033[38;2;82;64;50m▄\033[48;2;112;87;67m\033[38;2;130;105;82m▄\033[48;2;138;114;95m\033[38;2;128;109;92m▄\033[48;2;111;94;84m\033[38;2;89;77;70m▄\033[48;2;99;87;79m\033[38;2;88;76;70m▄\033[48;2;104;91;81m\033[38;2;98;87;79m▄\033[48;2;129;111;98m\033[38;2;122;107;96m▄\033[48;2;137;119;104m\033[38;2;124;108;97m▄\033[48;2;148;127;108m\033[38;2;142;122;106m▄\033[48;2;143;113;89m\033[38;2;151;123;97m▄\033[48;2;74;60;50m\033[38;2;91;70;54m▄\033[48;2;8;8;8m\033[38;2;14;13;13m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;3;3;3m\033[38;2;5;5;5m▄\033[48;2;54;44;36m\033[38;2;61;49;39m▄\033[48;2;123;95;71m\033[38;2;118;93;72m▄\033[48;2;125;103;86m\033[38;2;118;99;84m▄\033[48;2;134;116;101m\033[38;2;113;99;91m▄\033[48;2;130;114;101m\033[38;2;117;102;94m▄\033[48;2;128;111;99m\033[38;2;122;106;96m▄\033[48;2;134;117;102m\033[38;2;125;109;98m▄\033[48;2;145;125;107m\033[38;2;134;117;104m▄\033[48;2;145;122;99m\033[38;2;140;121;104m▄\033[48;2;104;78;61m\033[38;2;130;109;90m▄\033[48;2;47;39;35m\033[38;2;87;67;53m▄\033[48;2;5;5;5m\033[38;2;29;25;22m▄\033[48;2;0;0;0m\033[38;2;2;2;2m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;3;3;3m\033[38;2;17;16;16m▄\033[48;2;48;41;36m\033[38;2;83;64;51m▄\033[48;2;116;88;67m\033[38;2;127;102;80m▄\033[48;2;134;111;89m\033[38;2;117;100;82m▄\033[48;2;105;91;78m\033[38;2;93;80;69m▄\033[48;2;75;65;59m\033[38;2;78;69;64m▄\033[48;2;85;76;70m\033[38;2;91;82;74m▄\033[48;2;91;81;74m\033[38;2;85;75;69m▄\033[48;2;107;94;85m\033[38;2;97;86;78m▄\033[48;2;122;108;98m\033[38;2;115;101;92m▄\033[48;2;134;116;104m\033[38;2;135;118;105m▄\033[48;2;152;124;101m\033[38;2;151;126;103m▄\033[48;2;108;82;63m\033[38;2;125;98;75m▄\033[48;2;21;20;18m\033[38;2;33;28;26m▄\033[48;2;0;0;0m\033[38;2;1;1;1m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;8;8;8m\033[38;2;11;10;9m▄\033[48;2;68;55;45m\033[38;2;74;61;50m▄\033[48;2;106;87;72m\033[38;2;104;87;73m▄\033[48;2;108;93;81m\033[38;2;92;81;72m▄\033[48;2;100;87;79m\033[38;2;93;81;73m▄\033[48;2;104;91;84m\033[38;2;97;85;79m▄\033[48;2;116;102;93m\033[38;2;110;96;89m▄\033[48;2;118;103;94m\033[38;2;111;98;90m▄\033[48;2;129;111;100m\033[38;2;120;103;94m▄\033[48;2;129;112;98m\033[38;2;120;103;92m▄\033[48;2;131;113;97m\033[38;2;124;108;96m▄\033[48;2;118;96;76m\033[38;2;125;105;88m▄\033[48;2;59;46;37m\033[38;2;92;71;56m▄\033[48;2;12;12;11m\033[38;2;35;30;26m▄\033[48;2;0;0;0m\033[38;2;3;3;3m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;5;5;5m\033[38;2;19;18;18m▄\033[48;2;52;44;39m\033[38;2;89;67;52m▄\033[48;2;110;85;66m\033[38;2;119;95;74m▄\033[48;2;110;93;77m\033[38;2;88;77;66m▄\033[48;2;96;84;73m\033[38;2;81;72;65m▄\033[48;2;86;75;67m\033[38;2;78;69;63m▄\033[48;2;78;69;65m\033[38;2;79;71;67m▄\033[48;2;80;72;67m \033[48;2;84;75;69m\033[38;2;88;79;73m▄\033[48;2;96;85;78m\033[38;2;106;93;86m▄\033[48;2;107;94;86m\033[38;2;113;99;91m▄\033[48;2;133;115;104m\033[38;2;130;112;102m▄\033[48;2;148;126;107m\033[38;2;146;125;108m▄\033[48;2;134;106;82m\033[38;2;144;115;91m▄\033[48;2;50;42;35m\033[38;2;67;53;42m▄\033[48;2;3;3;3m\033[38;2;7;7;7m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;14;13;12m\033[38;2;19;17;15m▄\033[48;2;82;67;55m\033[38;2;94;74;58m▄\033[48;2;104;88;74m\033[38;2;115;94;76m▄\033[48;2;88;78;68m\033[38;2;95;82;71m▄\033[48;2;81;69;63m\033[38;2;87;74;66m▄\033[48;2;91;79;73m\033[38;2;90;78;72m▄\033[48;2;108;94;86m\033[38;2;105;91;84m▄\033[48;2;112;97;90m\033[38;2;115;100;92m▄\033[48;2;110;96;88m\033[38;2;114;100;92m▄\033[48;2;109;94;85m\033[38;2;107;93;86m▄\033[48;2;115;101;91m\033[38;2;108;94;86m▄\033[48;2;120;103;89m\033[38;2;116;101;90m▄\033[48;2;116;93;72m\033[38;2;127;107;87m▄\033[48;2;70;56;45m\033[38;2;103;79;61m▄\033[48;2;14;13;12m\033[38;2;38;33;29m▄\033[48;2;0;0;0m\033[38;2;2;2;2m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;4;4;4m\033[38;2;12;12;12m▄\033[48;2;48;40;34m\033[38;2;81;62;47m▄\033[48;2;118;87;64m\033[38;2;131;100;74m▄\033[48;2;106;87;70m\033[38;2;93;79;67m▄\033[48;2;77;68;60m\033[38;2;69;61;56m▄\033[48;2;70;62;57m\033[38;2;65;58;53m▄\033[48;2;67;59;56m\033[38;2;62;55;49m▄\033[48;2;75;68;63m\033[38;2;75;68;62m▄\033[48;2;90;82;77m\033[38;2;93;84;78m▄\033[48;2;91;81;76m\033[38;2;98;84;79m▄\033[48;2;113;99;91m\033[38;2;112;98;91m▄\033[48;2;118;103;95m\033[38;2;118;102;95m▄\033[48;2;128;110;100m\033[38;2;123;106;99m▄\033[48;2;146;124;108m\033[38;2;149;127;110m▄\033[48;2;154;124;100m\033[38;2;153;124;100m▄\033[48;2;86;66;50m\033[38;2;108;83;63m▄\033[48;2;13;13;12m\033[38;2;21;19;17m▄\033[48;2;0;0;0m\033[38;2;1;1;1m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;1;1;1m \033[48;2;25;22;19m\033[38;2;29;26;23m▄\033[48;2;104;83;66m\033[38;2;107;85;67m▄\033[48;2;121;98;78m\033[38;2;124;99;79m▄\033[48;2;110;94;79m\033[38;2;102;86;73m▄\033[48;2;86;73;65m\033[38;2;92;78;69m▄\033[48;2;90;77;71m\033[38;2;93;80;72m▄\033[48;2;101;87;80m\033[38;2;94;80;73m▄\033[48;2;119;103;95m\033[38;2;113;96;88m▄\033[48;2;121;106;99m\033[38;2;122;107;99m▄\033[48;2;109;94;87m\033[38;2;116;101;94m▄\033[48;2;107;94;86m\033[38;2;107;93;85m▄\033[48;2;102;87;79m\033[38;2;110;94;86m▄\033[48;2;123;105;90m\033[38;2;127;107;93m▄\033[48;2;116;90;69m\033[38;2;134;106;83m▄\033[48;2;69;56;47m\033[38;2;97;72;53m▄\033[48;2;10;9;9m\033[38;2;33;30;27m▄\033[48;2;0;0;0m\033[38;2;1;1;0m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;2;2;2m\033[38;2;8;8;8m▄\033[48;2;36;31;27m\033[38;2;67;52;42m▄\033[48;2;107;81;61m\033[38;2;123;93;70m▄\033[48;2;136;107;82m\033[38;2;138;111;88m▄\033[48;2;86;73;64m\033[38;2;91;78;65m▄\033[48;2;60;53;49m\033[38;2;62;55;50m▄\033[48;2;61;54;49m\033[38;2;64;55;50m▄\033[48;2;68;60;54m\033[38;2;65;55;51m▄\033[48;2;78;69;63m\033[38;2;81;70;64m▄\033[48;2;87;76;70m\033[38;2;84;71;64m▄\033[48;2;104;91;84m\033[38;2;95;81;74m▄\033[48;2;102;85;79m\033[38;2;106;87;81m▄\033[48;2;121;102;96m\033[38;2;128;108;101m▄\033[48;2;129;111;104m\033[38;2;135;117;109m▄\033[48;2;142;124;109m\033[38;2;133;116;103m▄\033[48;2;146;121;97m\033[38;2;136;114;95m▄\033[48;2;122;95;74m\033[38;2;129;103;82m▄\033[48;2;32;27;24m\033[38;2;43;36;31m▄\033[48;2;1;1;1m\033[38;2;3;3;3m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "");
                auto r_grut01 = text{} + ""\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;1;1;1m\033[38;2;2;2;2m▄\033[48;2;35;30;26m\033[38;2;38;33;28m▄\033[48;2;114;90;71m\033[38;2;116;94;73m▄\033[48;2;127;101;80m\033[38;2;128;103;81m▄\033[48;2;103;85;73m\033[38;2;116;95;79m▄\033[48;2;93;79;69m\033[38;2;94;80;69m▄\033[48;2;91;78;71m\033[38;2;87;73;66m▄\033[48;2;94;80;73m\033[38;2;92;76;69m▄\033[48;2;111;93;84m\033[38;2;105;87;79m▄\033[48;2;128;108;97m\033[38;2;130;109;98m▄\033[48;2;124;106;98m\033[38;2;135;113;104m▄\033[48;2;127;109;102m\033[38;2;139;118;109m▄\033[48;2;111;92;85m\033[38;2;119;99;88m▄\033[48;2;127;105;93m\033[38;2;128;106;91m▄\033[48;2;148;120;97m\033[38;2;147;120;99m▄\033[48;2;123;93;69m\033[38;2;130;100;77m▄\033[48;2;76;64;54m\033[38;2;98;73;56m▄\033[48;2;12;12;11m\033[38;2;50;46;42m▄\033[48;2;0;0;0m\033[38;2;5;5;4m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;1;1;1m▄\033[48;2;3;3;3m\033[38;2;19;18;17m▄\033[48;2;34;30;27m\033[38;2;73;60;49m▄\033[48;2;102;78;60m\033[38;2;119;89;65m▄\033[48;2;136;105;80m\033[38;2;135;106;80m▄\033[48;2;141;116;92m\033[38;2;142;118;95m▄\033[48;2;99;85;72m\033[38;2;110;95;81m▄\033[48;2;69;60;55m\033[38;2;77;67;59m▄\033[48;2;63;54;50m\033[38;2;67;57;51m▄\033[48;2;66;56;51m\033[38;2;67;56;51m▄\033[48;2;77;64;58m\033[38;2;75;62;56m▄\033[48;2;89;75;68m\033[38;2;90;75;68m▄\033[48;2;95;79;71m\033[38;2;94;76;69m▄\033[48;2;116;96;89m\033[38;2;120;99;92m▄\033[48;2;130;108;100m\033[38;2;134;111;103m▄\033[48;2;135;116;108m\033[38;2;132;115;106m▄\033[48;2;135;118;104m\033[38;2;133;116;104m▄\033[48;2;127;108;92m\033[38;2;133;113;96m▄\033[48;2;134;109;88m\033[38;2;143;117;93m▄\033[48;2;55;44;36m\033[38;2;63;50;41m▄\033[48;2;5;5;5m\033[38;2;6;6;6m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;2;2;2m\033[38;2;1;1;1m▄\033[48;2;38;33;28m\033[38;2;36;30;26m▄\033[48;2;118;95;74m\033[38;2;118;93;72m▄\033[48;2;124;100;80m\033[38;2;127;103;81m▄\033[48;2;120;99;82m\033[38;2;109;89;74m▄\033[48;2;102;86;73m\033[38;2;102;87;74m▄\033[48;2;88;74;66m\033[38;2;90;77;67m▄\033[48;2;87;73;65m\033[38;2;80;66;59m▄\033[48;2;100;82;74m\033[38;2;92;75;68m▄\033[48;2;116;94;86m\033[38;2;109;86;79m▄\033[48;2;141;116;105m\033[38;2;127;100;92m▄\033[48;2;144;120;110m\033[38;2;141;114;104m▄\033[48;2;128;106;92m\033[38;2;141;117;100m▄\033[48;2;129;105;86m\033[38;2;141;117;95m▄\033[48;2;137;111;89m\033[38;2;129;104;81m▄\033[48;2;121;93;71m\033[38;2;110;84;64m▄\033[48;2;103;75;55m\033[38;2;99;73;54m▄\033[48;2;94;76;62m\033[38;2;111;82;63m▄\033[48;2;40;38;35m\033[38;2;95;79;68m▄\033[48;2;3;3;3m\033[38;2;29;28;26m▄\033[48;2;0;0;0m\033[38;2;2;2;1m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;9;9;8m▄\033[48;2;14;13;12m\033[38;2;59;50;43m▄\033[48;2;65;57;50m\033[38;2;107;83;67m▄\033[48;2;105;82;64m\033[38;2;106;78;59m▄\033[48;2;116;86;62m\033[38;2;105;80;60m▄\033[48;2;123;98;75m\033[38;2;113;90;69m▄\033[48;2;137;116;93m\033[38;2;107;89;72m▄\033[48;2;119;102;86m\033[38;2;126;109;95m▄\033[48;2;93;80;70m\033[38;2;115;100;86m▄\033[48;2;81;69;60m\033[38;2;86;70;60m▄\033[48;2;67;56;50m\033[38;2;62;48;44m▄\033[48;2;70;56;51m\033[38;2;59;45;42m▄\033[48;2;75;60;55m\033[38;2;69;54;50m▄\033[48;2;94;75;69m\033[38;2;89;70;65m▄\033[48;2;117;94;88m\033[38;2;114;92;86m▄\033[48;2;131;110;102m\033[38;2;126;106;97m▄\033[48;2;131;115;105m\033[38;2;136;119;108m▄\033[48;2;138;121;107m\033[38;2;153;134;118m▄\033[48;2;150;128;105m\033[38;2;153;129;106m▄\033[48;2;150;123;96m\033[38;2;146;119;92m▄\033[48;2;65;52;42m\033[38;2;60;48;39m▄\033[48;2;7;7;7m\033[38;2;6;6;6m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;1;1;1m\033[38;2;1;1;0m▄\033[48;2;32;27;24m\033[38;2;25;22;19m▄\033[48;2;118;92;70m\033[38;2;112;88;67m▄\033[48;2;134;106;82m\033[38;2;140;109;83m▄\033[48;2;118;93;76m\033[38;2;127;100;80m▄\033[48;2;99;83;71m\033[38;2;101;85;71m▄\033[48;2;92;78;69m\033[38;2;90;78;68m▄\033[48;2;74;60;54m\033[38;2;72;59;52m▄\033[48;2;80;63;57m\033[38;2;69;52;48m▄\033[48;2;103;81;74m\033[38;2;89;69;63m▄\033[48;2;121;94;87m\033[38;2;117;90;83m▄\033[48;2;135;106;96m\033[38;2;137;108;97m▄\033[48;2;147;121;105m\033[38;2;142;115;101m▄\033[48;2;148;127;105m\033[38;2;139;116;96m▄\033[48;2;127;104;83m\033[38;2;127;103;82m▄\033[48;2;105;80;61m\033[38;2;131;100;74m▄\033[48;2;99;72;55m\033[38;2;144;105;76m▄\033[48;2;125;90;68m\033[38;2;173;130;94m▄\033[48;2;130;97;75m\033[38;2;167;123;90m▄\033[48;2;76;63;53m\033[38;2;127;94;71m▄\033[48;2;12;11;11m\033[38;2;60;51;44m▄\033[48;2;0;0;0m\033[38;2;16;16;15m▄\033[48;2;0;0;0m\033[38;2;4;4;4m▄\033[48;2;0;0;0m\033[38;2;3;3;3m▄\033[48;2;0;0;0m\033[38;2;3;3;2m▄\033[48;2;0;0;0m\033[38;2;3;2;2m▄\033[48;2;0;0;0m\033[38;2;3;2;2m▄\033[48;2;0;0;0m\033[38;2;3;3;3m▄\033[48;2;0;0;0m\033[38;2;3;3;3m▄\033[48;2;0;0;0m\033[38;2;4;4;4m▄\033[48;2;0;0;0m\033[38;2;6;6;6m▄\033[48;2;0;0;0m\033[38;2;9;8;7m▄\033[48;2;0;0;0m\033[38;2;13;12;11m▄\033[48;2;0;0;0m\033[38;2;24;22;20m▄\033[48;2;4;4;4m\033[38;2;54;49;42m▄\033[48;2;43;40;37m\033[38;2;134;106;82m▄\033[48;2;128;96;73m\033[38;2;182;136;98m▄\033[48;2;143;107;82m\033[38;2;187;140;101m▄\033[48;2;121;90;70m\033[38;2;165;125;91m▄\033[48;2;98;74;58m\033[38;2;119;91;71m▄\033[48;2;98;78;61m \033[48;2;110;93;77m\033[38;2;96;79;63m▄\033[48;2;139;124;109m\033[38;2;128;111;93m▄\033[48;2;126;110;95m\033[38;2;132;113;94m▄\033[48;2;85;68;59m\033[38;2;95;77;66m▄\033[48;2;58;44;40m\033[38;2;61;46;43m▄\033[48;2;57;43;40m\033[38;2;58;45;42m▄\033[48;2;66;50;47m\033[38;2;68;50;49m▄\033[48;2;85;64;61m\033[38;2;86;65;63m▄\033[48;2;111;88;83m\033[38;2;105;82;78m▄\033[48;2;121;101;93m\033[38;2;112;95;86m▄\033[48;2;136;120;108m\033[38;2;131;114;99m▄\033[48;2;152;132;114m\033[38;2;159;133;109m▄\033[48;2;157;130;104m\033[38;2;161;128;99m▄\033[48;2;144;118;92m\033[38;2;131;105;80m▄\033[48;2;52;44;37m\033[38;2;43;39;34m▄\033[48;2;4;5;4m\033[38;2;3;3;2m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;17;15;14m\033[38;2;9;9;8m▄\033[48;2;100;80;62m\033[38;2;82;68;56m▄\033[48;2;145;114;86m\033[38;2;149;119;91m▄\033[48;2;131;104;82m\033[38;2;144;116;91m▄\033[48;2;103;86;71m\033[38;2;118;98;81m▄\033[48;2;86;74;63m\033[38;2;85;72;62m▄\033[48;2;75;62;54m\033[38;2;77;64;55m▄\033[48;2;66;51;46m\033[38;2;70;55;49m▄\033[48;2;84;65;58m\033[38;2;81;62;56m▄\033[48;2;110;85;75m\033[38;2;102;78;67m▄\033[48;2;140;113;99m\033[38;2;132;104;87m▄\033[48;2;143;116;100m\033[38;2;143;115;94m▄\033[48;2;136;110;90m\033[38;2;144;112;87m▄\033[48;2;140;108;81m\033[38;2;176;136;101m▄\033[48;2;172;129;94m\033[38;2;201;156;114m▄\033[48;2;193;145;103m\033[38;2;203;155;113m▄\033[48;2;199;153;113m\033[38;2;197;150;109m▄\033[48;2;190;145;105m\033[38;2;192;143;102m▄\033[48;2;167;125;90m\033[38;2;187;140;99m▄\033[48;2;132;104;83m\033[38;2;174;129;91m▄\033[48;2;85;71;60m\033[38;2;159;117;84m▄\033[48;2;56;45;37m\033[38;2;143;103;72m▄\033[48;2;48;38;31m\033[38;2;135;97;68m▄\033[48;2;45;37;29m\033[38;2;123;87;62m▄\033[48;2;42;33;26m\033[38;2;113;79;56m▄\033[48;2;40;32;26m\033[38;2;121;87;63m▄\033[48;2;44;36;31m\033[38;2;133;98;70m▄\033[48;2;49;38;29m\033[38;2;142;103;71m▄\033[48;2;57;45;34m\033[38;2;152;110;76m▄\033[48;2;64;51;40m\033[38;2;154;112;78m▄\033[48;2;72;58;45m\033[38;2;155;113;78m▄\033[48;2;85;69;54m\033[38;2;153;112;77m▄\033[48;2;105;85;66m\033[38;2;155;113;79m▄\033[48;2;140;107;79m\033[38;2;160;117;81m▄\033[48;2;178;133;95m\033[38;2;168;123;85m▄\033[48;2;187;139;98m\033[38;2;164;119;82m▄\033[48;2;188;141;100m\033[38;2;167;123;85m▄\033[48;2;181;138;100m\033[38;2;170;126;91m▄\033[48;2;156;118;88m\033[38;2;161;121;87m▄\033[48;2;104;80;61m\033[38;2;123;94;70m▄\033[48;2;83;66;51m\033[38;2;86;67;52m▄\033[48;2;108;89;71m\033[38;2;90;71;55m▄\033[48;2;123;103;82m\033[38;2;109;87;69m▄\033[48;2;92;76;63m\033[38;2;83;67;55m▄\033[48;2;63;49;45m\033[38;2;59;47;42m▄\033[48;2;56;42;41m\033[38;2;54;41;39m▄\033[48;2;60;45;44m\033[38;2;58;44;43m▄\033[48;2;79;60;57m\033[38;2;70;54;52m▄\033[48;2;96;75;71m\033[38;2;85;69;65m▄\033[48;2;108;92;83m\033[38;2;109;91;79m▄\033[48;2;134;113;96m\033[38;2;136;110;89m▄\033[48;2;158;129;102m\033[38;2;157;125;97m▄\033[48;2;164;129;97m\033[38;2;130;98;73m▄\033[48;2;109;85;65m\033[38;2;91;69;54m▄\033[48;2;42;39;36m\033[38;2;48;41;34m▄\033[48;2;3;3;3m\033[38;2;4;4;4m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;5;5;4m\033[38;2;4;4;4m▄\033[48;2;68;61;54m\033[38;2;62;54;47m▄\033[48;2;140;110;85m\033[38;2;151;126;102m▄\033[48;2;152;123;96m\033[38;2;140;107;83m▄\033[48;2;134;110;89m\033[38;2;141;112;88m▄\033[48;2;97;82;69m\033[38;2;118;98;79m▄\033[48;2;68;56;49m\033[38;2;76;63;54m▄\033[48;2;69;56;49m\033[38;2;63;51;46m▄\033[48;2;72;55;50m\033[38;2;68;54;49m▄\033[48;2;91;70;60m\033[38;2;81;63;56m▄\033[48;2;119;91;75m\033[38;2;112;86;71m▄\033[48;2;136;104;81m\033[38;2;145;112;87m▄\033[48;2;165;128;97m\033[38;2;183;143;107m▄\033[48;2;198;156;117m\033[38;2;198;150;110m▄\033[48;2;204;158;114m\033[38;2;198;148;106m▄\033[48;2;199;149;106m\033[38;2;202;153;110m▄\033[48;2;195;146;104m\033[38;2;200;150;109m▄\033[48;2;195;146;104m\033[38;2;201;153;111m▄\033[48;2;192;143;101m\033[38;2;196;148;106m▄\033[48;2;186;136;94m\033[38;2;189;140;99m▄\033[48;2;175;126;85m\033[38;2;176;126;87m▄\033[48;2;162;113;75m\033[38;2;163;114;77m▄\033[48;2;152;105;70m\033[38;2;152;102;68m▄\033[48;2;139;93;62m\033[38;2;144;96;63m▄\033[48;2;137;93;64m\033[38;2;161;110;75m▄\033[48;2;164;115;78m\033[38;2;181;127;85m▄\033[48;2;176;125;83m\033[38;2;187;136;91m▄\033[48;2;181;130;89m\033[38;2;193;142;99m▄\033[48;2;182;132;90m\033[38;2;188;138;95m▄\033[48;2;180;132;92m\033[38;2;187;139;97m▄\033[48;2;175;127;87m\033[38;2;185;136;96m▄\033[48;2;170;123;85m\033[38;2;181;133;93m▄\033[48;2;164;119;82m\033[38;2;168;122;83m▄\033[48;2;160;116;81m\033[38;2;159;115;81m▄\033[48;2;154;110;76m\033[38;2;155;113;81m▄\033[48;2;152;109;75m\033[38;2;151;108;76m▄\033[48;2;147;104;71m\033[38;2;143;103;71m▄\033[48;2;149;107;74m\033[38;2;135;95;65m▄\033[48;2;147;110;76m\033[38;2;131;93;65m▄\033[48;2;132;100;72m\033[38;2;134;99;71m▄\033[48;2;107;84;65m\033[38;2;118;90;67m▄\033[48;2;85;66;51m\033[38;2;97;75;57m▄\033[48;2;87;68;54m\033[38;2;78;61;48m▄\033[48;2;68;53;45m\033[38;2;61;47;41m▄\033[48;2;53;41;38m\033[38;2;51;40;37m▄\033[48;2;51;40;38m\033[38;2;48;37;35m▄\033[48;2;55;44;42m\033[38;2;50;39;37m▄\033[48;2;66;52;50m\033[38;2;60;48;46m▄\033[48;2;79;65;60m\033[38;2;86;73;65m▄\033[48;2;112;93;78m\033[38;2;130;110;91m▄\033[48;2;135;108;85m\033[38;2;141;114;89m▄\033[48;2;136;107;82m\033[38;2;114;87;69m▄\033[48;2;82;60;50m\033[38;2;101;79;66m▄\033[48;2;122;100;81m\033[38;2;154;130;104m▄\033[48;2;59;49;40m\033[38;2;57;47;39m▄\033[48;2;6;6;5m\033[38;2;5;5;5m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;4;4;4m\033[38;2;3;3;3m▄\033[48;2;60;49;41m\033[38;2;56;49;42m▄\033[48;2;149;125;100m\033[38;2;142;117;90m▄\033[48;2;135;109;91m\033[38;2;134;109;88m▄\033[48;2;147;116;91m\033[38;2;147;118;95m▄\033[48;2;138;111;88m\033[38;2;158;132;107m▄\033[48;2;106;89;74m\033[38;2;126;104;86m▄\033[48;2;72;59;54m\033[38;2;80;65;57m▄\033[48;2;65;51;47m\033[38;2;70;56;50m▄\033[48;2;77;61;55m\033[38;2;78;61;54m▄\033[48;2;111;85;73m\033[38;2;123;94;77m▄\033[48;2;165;130;100m\033[38;2;174;134;100m▄\033[48;2;193;147;108m\033[38;2;195;148;108m▄\033[48;2;197;148;107m\033[38;2;203;155;115m▄\033[48;2;207;159;118m\033[38;2;207;159;119m▄\033[48;2;207;159;117m\033[38;2;205;159;117m▄\033[48;2;205;156;115m\033[38;2;209;163;121m▄\033[48;2;206;157;117m\033[38;2;211;166;124m▄\033[48;2;203;155;113m\033[38;2;199;151;108m▄\033[48;2;200;151;110m\033[38;2;198;149;107m▄\033[48;2;182;131;93m\033[38;2;190;141;102m▄\033[48;2;169;119;82m\033[38;2;178;128;91m▄\033[48;2;149;99;66m\033[38;2;157;107;74m▄\033[48;2;153;104;70m\033[38;2;172;118;81m▄\033[48;2;180;125;84m\033[38;2;190;136;93m▄\033[48;2;191;137;93m\033[38;2;200;148;103m▄\033[48;2;195;143;99m\033[38;2;197;145;100m▄\033[48;2;195;144;100m\033[38;2;198;145;101m▄\033[48;2;189;140;98m\033[38;2;197;150;108m▄\033[48;2;186;138;98m\033[38;2;184;138;98m▄\033[48;2;182;134;94m\033[38;2;173;127;89m▄\033[48;2;172;124;86m\033[38;2;170;125;90m▄\033[48;2;164;119;84m\033[38;2;165;121;87m▄\033[48;2;158;116;84m\033[38;2;152;110;77m▄\033[48;2;148;107;75m\033[38;2;153;112;80m▄\033[48;2;152;111;81m\033[38;2;152;111;80m▄\033[48;2;147;107;77m\033[38;2;144;105;74m▄\033[48;2;138;99;71m\033[38;2;136;97;68m▄\033[48;2;126;88;61m\033[38;2;131;93;66m▄\033[48;2;123;86;60m\033[38;2;125;89;63m▄\033[48;2;126;95;69m\033[38;2;122;87;61m▄\033[48;2;107;82;60m\033[38;2;118;88;65m▄\033[48;2;86;67;53m\033[38;2;112;87;66m▄\033[48;2;66;51;44m\033[38;2;88;69;56m▄\033[48;2;50;39;36m\033[38;2;59;46;41m▄\033[48;2;45;35;33m\033[38;2;60;50;45m▄\033[48;2;59;49;45m\033[38;2;66;54;48m▄\033[48;2;64;53;49m\033[38;2;73;60;53m▄\033[48;2;96;81;72m\033[38;2;101;85;73m▄\033[48;2;140;118;98m\033[38;2;142;117;95m▄\033[48;2;151;123;97m\033[38;2;158;126;97m▄\033[48;2;145;115;91m\033[38;2;175;142;111m▄\033[48;2;145;115;93m\033[38;2;164;126;95m▄\033[48;2;157;128;102m\033[38;2;133;103;78m▄\033[48;2;54;46;38m\033[38;2;50;41;34m▄\033[48;2;3;4;3m\033[38;2;3;3;3m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;3;3;3m\033[38;2;3;3;2m▄\033[48;2;53;47;39m\033[38;2;51;44;36m▄\033[48;2;137;105;78m\033[38;2;135;101;72m▄\033[48;2;146;112;85m\033[38;2;163;123;89m▄\033[48;2;155;126;99m\033[38;2;163;127;96m▄\033[48;2;157;132;108m\033[38;2;149;120;96m▄\033[48;2;138;115;97m\033[38;2;136;112;94m▄\033[48;2;102;85;73m\033[38;2;107;88;75m▄\033[48;2;97;81;70m\033[38;2;110;90;76m▄\033[48;2;115;93;76m\033[38;2;165;133;105m▄\033[48;2;165;127;95m\033[38;2;190;146;108m▄\033[48;2;192;146;106m\033[38;2;193;146;106m▄\033[48;2;199;151;111m\033[38;2;193;146;106m▄\033[48;2;200;152;112m\033[38;2;183;139;103m▄\033[48;2;191;146;109m\033[38;2;167;129;99m▄\033[48;2;189;144;104m\033[38;2;178;140;102m▄\033[48;2;199;154;112m\033[38;2;189;141;99m▄\033[48;2;204;157;116m\033[38;2;195;147;105m▄\033[48;2;190;141;99m\033[38;2;181;134;93m▄\033[48;2;190;142;100m\033[38;2;187;139;98m▄\033[48;2;190;141;101m\033[38;2;191;143;102m▄\033[48;2;178;130;91m\033[38;2;179;131;91m▄\033[48;2;168;117;82m\033[38;2;180;129;89m▄\033[48;2;189;136;95m\033[38;2;199;147;103m▄\033[48;2;199;146;102m\033[38;2;203;152;107m▄\033[48;2;200;149;103m\033[38;2;202;150;106m▄\033[48;2;199;147;102m\033[38;2;199;146;102m▄\033[48;2;199;149;102m\033[38;2;208;157;112m▄\033[48;2;198;153;110m\033[38;2;204;160;119m▄\033[48;2;184;140;101m\033[38;2;178;138;101m▄\033[48;2;168;125;90m\033[38;2;160;124;93m▄\033[48;2;160;119;87m\033[38;2;144;111;85m▄\033[48;2;153;115;85m\033[38;2;141;107;80m▄\033[48;2;152;114;82m\033[38;2;141;104;76m▄\033[48;2;150;109;78m\033[38;2;140;99;70m▄\033[48;2;143;101;69m\033[38;2;137;97;67m▄\033[48;2;138;97;66m\033[38;2;137;97;67m▄\033[48;2;138;98;68m\033[38;2;140;98;68m▄\033[48;2;137;97;68m\033[38;2;139;98;68m▄\033[48;2;133;95;67m\033[38;2;138;98;68m▄\033[48;2;129;93;66m\033[38;2;131;95;67m▄\033[48;2;126;93;67m\033[38;2;126;91;63m▄\033[48;2;126;97;72m\033[38;2;125;91;65m▄\033[48;2;109;85;66m\033[38;2;127;98;73m▄\033[48;2;82;65;54m\033[38;2;114;91;71m▄\033[48;2;80;66;56m\033[38;2;96;79;64m▄\033[48;2;84;72;62m\033[38;2;79;67;57m▄\033[48;2;66;54;46m\033[38;2;65;54;46m▄\033[48;2;112;93;78m\033[38;2;114;95;78m▄\033[48;2;160;128;100m\033[38;2;159;127;98m▄\033[48;2;170;131;98m\033[38;2;170;128;94m▄\033[48;2;170;130;96m\033[38;2;166;122;87m▄\033[48;2;159;117;84m\033[38;2;140;101;71m▄\033[48;2;116;87;64m\033[38;2;82;59;45m▄\033[48;2;40;32;26m\033[38;2;19;16;14m▄\033[48;2;2;2;2m\033[38;2;0;0;0m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;1;1;1m \033[48;2;42;35;30m\033[38;2;25;22;20m▄\033[48;2;132;99;69m\033[38;2;116;87;63m▄\033[48;2;170;127;89m\033[38;2;171;127;89m▄\033[48;2;166;126;90m\033[38;2;162;122;85m▄\033[48;2;149;116;88m\033[38;2;144;111;82m▄\033[48;2;127;102;82m\033[38;2;121;94;73m▄\033[48;2;118;95;77m\033[38;2;129;101;80m▄\033[48;2;150;120;96m\033[38;2;180;142;110m▄\033[48;2;192;151;115m\033[38;2;196;149;111m▄\033[48;2;195;148;109m\033[38;2;187;138;101m▄\033[48;2;193;145;106m\033[38;2;175;129;94m▄\033[48;2;171;128;94m\033[38;2;159;119;89m▄\033[48;2;140;106;81m\033[38;2;119;91;73m▄\033[48;2;146;118;93m\033[38;2;126;103;85m▄\033[48;2;160;125;94m\033[38;2;144;110;83m▄\033[48;2;183;133;92m\033[38;2;191;142;101m▄\033[48;2;192;142;100m\033[38;2;191;140;98m▄\033[48;2;178;131;91m\033[38;2;175;125;87m▄\033[48;2;188;140;100m\033[38;2;183;136;96m▄\033[48;2;196;150;109m\033[38;2;189;144;104m▄\033[48;2;183;135;95m\033[38;2;182;134;94m▄\033[48;2;183;134;93m\033[38;2;193;143;104m▄\033[48;2;206;155;110m\033[38;2;201;152;109m▄\033[48;2;204;153;109m\033[38;2;200;151;108m▄\033[48;2;201;149;105m\033[38;2;195;142;101m▄\033[48;2;200;147;103m\033[38;2;199;146;102m▄\033[48;2;210;157;112m\033[38;2;197;144;100m▄\033[48;2;210;166;126m\033[38;2;198;153;113m▄\033[48;2;168;130;95m\033[38;2;156;120;88m▄\033[48;2;156;127;99m\033[38;2;120;97;77m▄\033[48;2;135;108;85m\033[38;2;100;82;67m▄\033[48;2;133;103;79m\033[38;2;94;74;59m▄\033[48;2;132;100;77m\033[38;2;118;91;72m▄\033[48;2;133;95;68m\033[38;2;138;107;80m▄\033[48;2;130;91;63m\033[38;2;141;105;78m▄\033[48;2;130;91;65m\033[38;2;128;92;65m▄\033[48;2;135;95;67m\033[38;2;123;86;60m▄\033[48;2;139;97;68m\033[38;2;130;91;64m▄\033[48;2;139;97;68m\033[38;2;142;101;73m▄\033[48;2;131;91;63m\033[38;2;137;96;67m▄\033[48;2;129;93;65m\033[38;2;129;93;64m▄\033[48;2;122;86;61m\033[38;2;123;88;61m▄\033[48;2;121;90;64m\033[38;2;119;85;61m▄\033[48;2;120;93;70m\033[38;2;117;88;65m▄\033[48;2;101;81;63m\033[38;2;116;91;70m▄\033[48;2;75;61;52m\033[38;2;83;66;53m▄\033[48;2;72;59;50m\033[38;2;73;60;50m▄\033[48;2;94;78;63m\033[38;2;77;62;51m▄\033[48;2;130;101;77m\033[38;2;98;73;56m▄\033[48;2;152;112;81m\033[38;2;122;86;61m▄\033[48;2;142;101;71m\033[38;2;120;86;62m▄\033[48;2;103;78;62m\033[38;2;70;61;55m▄\033[48;2;38;32;28m\033[38;2;10;9;9m▄\033[48;2;4;4;4m\033[38;2;0;0;0m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;9;9;8m\033[38;2;2;2;2m▄\033[48;2;82;66;54m\033[38;2;43;41;38m▄\033[48;2;157;115;79m\033[38;2;140;109;83m▄\033[48;2;158;119;84m\033[38;2;133;96;66m▄\033[48;2;129;97;72m\033[38;2;114;85;63m▄\033[48;2;104;77;60m\033[38;2;111;83;62m▄\033[48;2;143;109;84m\033[38;2;158;118;87m▄\033[48;2;188;144;107m\033[38;2;189;141;104m▄\033[48;2;188;140;101m\033[38;2;166;120;86m▄\033[48;2;171;122;89m\033[38;2;151;116;94m▄\033[48;2;172;131;100m\033[38;2;154;131;110m▄\033[48;2;165;132;105m\033[38;2;139;121;104m▄\033[48;2;150;124;102m\033[38;2;122;107;94m▄\033[48;2;145;121;100m\033[38;2;147;127;109m▄\033[48;2;156;122;95m\033[38;2;156;124;97m▄\033[48;2;190;143;104m\033[38;2;177;134;100m▄\033[48;2;183;133;92m\033[38;2;179;129;92m▄\033[48;2;177;127;88m\033[38;2;168;117;80m▄\033[48;2;180;134;95m\033[38;2;170;122;85m▄\033[48;2;182;137;99m\033[38;2;176;130;93m▄\033[48;2;181;135;96m\033[38;2;175;129;93m▄\033[48;2;184;135;94m\033[38;2;176;127;88m▄\033[48;2;183;134;94m\033[38;2;177;128;90m▄\033[48;2;186;137;98m\033[38;2;167;118;82m▄\033[48;2;185;133;94m\033[38;2;173;123;87m▄\033[48;2;195;142;99m\033[38;2;181;132;93m▄\033[48;2;185;136;95m\033[38;2;179;134;98m▄\033[48;2;180;137;99m\033[38;2;171;134;99m▄\033[48;2;142;107;78m\033[38;2;140;109;82m▄\033[48;2;91;71;56m\033[38;2;83;66;51m▄\033[48;2;65;52;43m\033[38;2;60;50;42m▄\033[48;2;74;58;48m\033[38;2;65;56;49m▄\033[48;2;109;84;68m\033[38;2;67;58;51m▄\033[48;2;121;94;72m\033[38;2;67;57;50m▄\033[48;2;109;81;63m\033[38;2;76;63;52m▄\033[48;2;119;88;63m\033[38;2;88;67;51m▄\033[48;2;117;83;58m\033[38;2;96;68;50m▄\033[48;2;118;82;58m\033[38;2;105;72;53m▄\033[48;2;132;95;69m\033[38;2;119;83;59m▄\033[48;2;147;106;77m\033[38;2;129;89;63m▄\033[48;2;138;98;70m\033[38;2;133;94;68m▄\033[48;2;128;91;64m\033[38;2;126;90;64m▄\033[48;2;119;85;59m\033[38;2;121;86;61m▄\033[48;2;108;77;55m\033[38;2;107;75;54m▄\033[48;2;126;97;76m\033[38;2;102;73;53m▄\033[48;2;115;91;71m\033[38;2;117;89;68m▄\033[48;2;70;56;46m\033[38;2;85;65;51m▄\033[48;2;69;54;44m\033[38;2;66;52;42m▄\033[48;2;85;63;49m\033[38;2;72;55;44m▄\033[48;2;104;71;51m\033[38;2;86;61;46m▄\033[48;2;97;70;50m\033[38;2;85;67;55m▄\033[48;2;34;30;26m\033[38;2;23;21;20m▄\033[48;2;1;1;1m\033[38;2;0;0;0m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;18;18;17m\033[38;2;8;8;8m▄\033[48;2;115;95;79m\033[38;2;100;92;83m▄\033[48;2;119;84;60m\033[38;2;134;103;79m▄\033[48;2;107;79;58m\033[38;2;123;90;66m▄\033[48;2;130;97;71m\033[38;2;154;112;80m▄\033[48;2;175;130;95m\033[38;2;184;136;98m▄\033[48;2;186;137;99m\033[38;2;171;127;91m▄\033[48;2;128;96;74m\033[38;2;68;56;50m▄\033[48;2;75;66;58m\033[38;2;42;33;31m▄\033[48;2;44;37;34m\033[38;2;101;79;69m▄\033[48;2;54;46;44m\033[38;2;86;80;80m▄\033[48;2;67;61;58m\033[38;2;52;48;47m▄\033[48;2;116;103;92m\033[38;2;78;70;63m▄\033[48;2;141;114;92m\033[38;2;107;88;72m▄\033[48;2;160;122;94m\033[38;2;139;105;79m▄\033[48;2;166;117;83m\033[38;2;158;113;80m▄\033[48;2;165;117;81m\033[38;2;161;116;83m▄\033[48;2;163;117;82m\033[38;2;154;112;81m▄\033[48;2;161;116;81m\033[38;2;148;107;75m▄\033[48;2;159;114;80m\033[38;2;150;107;74m▄\033[48;2;164;118;83m\033[38;2;160;117;85m▄\033[48;2;158;113;81m\033[38;2;138;97;69m▄\033[48;2;151;108;77m\033[38;2;132;94;67m▄\033[48;2;152;109;78m\033[38;2;127;90;65m▄\033[48;2;147;106;75m\033[38;2;113;82;59m▄\033[48;2;152;116;86m\033[38;2;111;84;64m▄\033[48;2;136;107;82m\033[38;2;87;68;53m▄\033[48;2;115;92;72m\033[38;2;74;61;51m▄\033[48;2;79;67;54m\033[38;2;48;43;38m▄\033[48;2;40;36;33m\033[38;2;24;24;23m▄\033[48;2;39;39;41m\033[38;2;60;58;61m▄\033[48;2;27;27;30m\033[38;2;37;38;42m▄\033[48;2;26;24;25m\033[38;2;38;34;33m▄\033[48;2;33;28;26m\033[38;2;66;48;40m▄\033[48;2;40;35;30m\033[38;2;33;29;27m▄\033[48;2;69;53;43m\033[38;2;40;35;33m▄\033[48;2;90;63;47m\033[38;2;72;53;43m▄\033[48;2;112;79;56m\033[38;2;105;75;55m▄\033[48;2;121;86;61m\033[38;2;117;84;59m▄\033[48;2;129;92;65m\033[38;2;120;84;59m▄\033[48;2;126;90;64m\033[38;2;119;84;59m▄\033[48;2;120;86;61m\033[38;2;117;84;59m▄\033[48;2;112;79;57m\033[38;2;113;81;59m▄\033[48;2;98;69;48m\033[38;2;101;71;50m▄\033[48;2;99;70;51m\033[38;2;90;61;44m▄\033[48;2;91;68;51m\033[38;2;92;66;49m▄\033[48;2;67;51;40m\033[38;2;78;58;45m▄\033[48;2;65;50;40m\033[38;2;65;49;38m▄\033[48;2;79;58;45m\033[38;2;71;52;40m▄\033[48;2;85;71;62m\033[38;2;79;65;55m▄\033[48;2;19;18;17m\033[38;2;21;20;18m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;6;6;5m\033[38;2;8;7;7m▄\033[48;2;89;84;79m\033[38;2;90;85;78m▄\033[48;2;132;105;84m\033[38;2;114;83;64m▄\033[48;2;127;91;66m\033[38;2;129;90;64m▄\033[48;2;164;117;83m\033[38;2;171;122;85m▄\033[48;2;187;137;98m\033[38;2;184;134;94m▄\033[48;2;165;123;89m\033[38;2;161;123;94m▄\033[48;2;77;66;60m\033[38;2;105;91;82m▄\033[48;2;79;62;55m\033[38;2;63;50;46m▄\033[48;2;130;97;77m\033[38;2;118;79;57m▄\033[48;2;73;69;70m\033[38;2;101;71;54m▄\033[48;2;59;49;44m\033[38;2;90;66;53m▄\033[48;2;61;52;48m\033[38;2;69;56;49m▄\033[48;2;83;67;57m\033[38;2;87;67;55m▄\033[48;2;122;90;67m\033[38;2;152;112;83m▄\033[48;2;155;113;84m\033[38;2;152;114;86m▄\033[48;2;150;112;83m\033[38;2;135;102;77m▄\033[48;2;142;105;77m\033[38;2;129;96;72m▄\033[48;2;138;99;71m\033[38;2;133;98;73m▄\033[48;2;144;105;75m\033[38;2;130;97;70m▄\033[48;2;139;102;75m\033[38;2;116;84;62m▄\033[48;2;127;91;67m\033[38;2;108;78;58m▄\033[48;2;115;80;58m\033[38;2;111;80;59m▄\033[48;2;112;78;56m\033[38;2;111;78;57m▄\033[48;2;83;60;46m\033[38;2;92;65;48m▄\033[48;2;72;55;43m\033[38;2;56;42;34m▄\033[48;2;57;45;37m\033[38;2;48;38;33m▄\033[48;2;40;34;32m\033[38;2;37;31;29m▄\033[48;2;27;25;25m\033[38;2;27;26;25m▄\033[48;2;21;21;21m\033[38;2;21;21;22m▄\033[48;2;45;36;35m\033[38;2;27;24;25m▄\033[48;2;44;35;34m\033[38;2;40;31;28m▄\033[48;2;53;40;33m\033[38;2;47;35;30m▄\033[48;2;85;57;45m\033[38;2;45;36;34m▄\033[48;2;34;30;29m\033[38;2;48;43;41m▄\033[48;2;46;42;40m\033[38;2;75;64;54m▄\033[48;2;60;48;41m\033[38;2;80;63;49m▄\033[48;2;89;66;50m\033[38;2;68;50;40m▄\033[48;2;107;77;56m\033[38;2;77;56;42m▄\033[48;2;113;80;58m\033[38;2;99;69;52m▄\033[48;2;112;79;58m\033[38;2;104;73;54m▄\033[48;2;113;80;57m\033[38;2;105;73;53m▄\033[48;2;112;80;58m\033[38;2;108;76;55m▄\033[48;2;103;72;52m\033[38;2;104;72;52m▄\033[48;2;95;66;48m\033[38;2;97;66;48m▄\033[48;2;83;57;41m\033[38;2;89;60;45m▄\033[48;2;84;62;46m\033[38;2;78;54;41m▄\033[48;2;69;50;37m\033[38;2;77;55;41m▄\033[48;2;72;52;40m\033[38;2;80;56;41m▄\033[48;2;77;60;49m\033[38;2;81;61;49m▄\033[48;2;25;22;20m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;11;11;10m\033[38;2;10;9;9m▄\033[48;2;91;80;71m\033[38;2;86;72;62m▄\033[48;2;107;70;51m\033[38;2;126;86;62m▄\033[48;2;139;94;66m\033[38;2;141;95;67m▄\033[48;2;174;124;85m\033[38;2;170;118;82m▄\033[48;2;183;133;92m\033[38;2;175;124;85m▄\033[48;2;144;105;78m\033[38;2;145;101;73m▄\033[48;2;139;118;101m\033[38;2;179;142;112m▄\033[48;2;52;50;49m\033[38;2;152;127;106m▄\033[48;2;49;43;41m\033[38;2;108;90;76m▄\033[48;2;55;45;41m\033[38;2;97;81;70m▄\033[48;2;61;51;47m\033[38;2;126;100;80m▄\033[48;2;104;79;64m\033[38;2;141;108;83m▄\033[48;2;152;113;87m\033[38;2;138;105;83m▄\033[48;2;149;112;85m\033[38;2;132;101;78m▄\033[48;2;133;99;77m\033[38;2;131;100;77m▄\033[48;2;128;97;74m\033[38;2;123;93;70m▄\033[48;2;126;96;73m\033[38;2;114;84;62m▄\033[48;2;120;90;67m\033[38;2;108;81;62m▄\033[48;2;112;83;62m\033[38;2;91;69;54m▄\033[48;2;102;75;56m\033[38;2;91;69;55m▄\033[48;2;98;71;54m\033[38;2;94;70;55m▄\033[48;2;100;73;54m\033[38;2;95;69;53m▄\033[48;2;105;76;55m\033[38;2;97;70;51m▄\033[48;2;107;78;57m\033[38;2;96;69;50m▄\033[48;2;76;56;44m\033[38;2;98;71;52m▄\033[48;2;54;42;35m\033[38;2;79;58;44m▄\033[48;2;53;41;33m\033[38;2;68;51;40m▄\033[48;2;52;42;36m\033[38;2;75;57;46m▄\033[48;2;46;39;37m\033[38;2;81;65;54m▄\033[48;2;45;41;39m\033[38;2;92;76;63m▄\033[48;2;53;48;44m\033[38;2;102;85;71m▄\033[48;2;60;52;49m\033[38;2;138;116;97m▄\033[48;2;82;72;64m\033[38;2;136;111;90m▄\033[48;2;94;76;61m\033[38;2;135;108;84m▄\033[48;2;103;82;62m\033[38;2;114;87;64m▄\033[48;2;101;76;57m\033[38;2;100;74;55m▄\033[48;2;80;57;42m\033[38;2;91;65;50m▄\033[48;2;62;44;34m\033[38;2;88;61;45m▄\033[48;2;83;58;45m\033[38;2;87;60;44m▄\033[48;2;97;68;51m\033[38;2;94;64;47m▄\033[48;2;105;74;55m\033[38;2;102;71;52m▄\033[48;2;107;75;54m\033[38;2;106;74;54m▄\033[48;2;102;70;50m\033[38;2;103;71;51m▄\033[48;2;99;68;50m\033[38;2;92;62;44m▄\033[48;2;86;60;43m\033[38;2;84;58;41m▄\033[48;2;73;50;38m\033[38;2;72;49;36m▄\033[48;2;86;62;47m\033[38;2;82;58;44m▄\033[48;2;97;70;51m\033[38;2;97;71;54m▄\033[48;2;91;74;60m\033[38;2;83;68;58m▄\033[48;2;24;22;21m\033[38;2;18;16;15m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;3;2;2m\033[38;2;0;0;0m▄\033[48;2;50;45;42m\033[38;2;9;9;9m▄\033[48;2;131;103;82m\033[38;2;80;69;61m▄\033[48;2;152;108;78m\033[38;2;141;107;84m▄\033[48;2;166;115;79m\033[38;2;158;109;75m▄\033[48;2;171;118;80m\033[38;2;176;123;84m▄\033[48;2;166;116;82m\033[38;2;188;137;97m▄\033[48;2;180;138;105m\033[38;2;178;131;96m▄\033[48;2;169;135;106m\033[38;2;181;146;116m▄\033[48;2;146;117;94m\033[38;2;176;142;114m▄\033[48;2;145;116;92m\033[38;2;154;123;100m▄\033[48;2;137;105;82m\033[38;2;121;96;78m▄\033[48;2;123;96;75m\033[38;2;111;88;71m▄\033[48;2;120;92;72m\033[38;2;110;85;65m▄\033[48;2;124;97;74m\033[38;2;111;86;66m▄\033[48;2;116;89;68m\033[38;2;105;82;64m▄\033[48;2;107;80;61m\033[38;2;94;72;57m▄\033[48;2;100;75;59m\033[38;2;86;68;55m▄\033[48;2;93;71;57m\033[38;2;79;63;52m▄\033[48;2;90;71;56m\033[38;2;80;63;52m▄\033[48;2;85;67;53m\033[38;2;79;64;52m▄\033[48;2;86;66;54m\033[38;2;79;62;51m▄\033[48;2;92;69;54m\033[38;2;84;64;51m▄\033[48;2;93;68;50m\033[38;2;87;64;49m▄\033[48;2;87;63;47m\033[38;2;83;60;46m▄\033[48;2;87;62;46m\033[38;2;79;56;42m▄\033[48;2;90;65;48m\033[38;2;83;59;45m▄\033[48;2;80;58;43m\033[38;2;91;66;50m▄\033[48;2;89;66;49m\033[38;2;93;67;50m▄\033[48;2;100;78;61m\033[38;2;101;76;58m▄\033[48;2;117;95;77m\033[38;2;120;95;76m▄\033[48;2;114;92;76m\033[38;2;109;86;67m▄\033[48;2;126;102;82m\033[38;2;100;76;60m▄\033[48;2;118;94;75m\033[38;2;99;76;59m▄\033[48;2;113;87;66m\033[38;2;93;69;51m▄\033[48;2;101;74;55m\033[38;2;91;65;48m▄\033[48;2;92;65;49m\033[38;2;87;61;45m▄\033[48;2;87;60;46m\033[38;2;90;63;46m▄\033[48;2;98;68;49m\033[38;2;113;80;58m▄\033[48;2;103;73;52m\033[38;2;114;81;56m▄\033[48;2;102;71;50m\033[38;2;108;75;53m▄\033[48;2;101;70;50m\033[38;2;106;74;53m▄\033[48;2;109;76;57m\033[38;2;108;76;56m▄\033[48;2;105;73;53m\033[38;2;108;76;55m▄\033[48;2;95;64;45m\033[38;2;101;70;51m▄\033[48;2;84;57;41m\033[38;2;87;58;43m▄\033[48;2;73;49;37m\033[38;2;78;53;39m▄\033[48;2;78;54;40m\033[38;2;85;60;45m▄\033[48;2;98;75;60m\033[38;2;100;77;61m▄\033[48;2;74;66;60m\033[38;2;67;59;53m▄\033[48;2;13;13;12m\033[38;2;11;10;10m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "";
                auto r_grut02 = text{} + ""\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;26;23;22m\033[38;2;4;4;4m▄\033[48;2;94;77;65m\033[38;2;48;42;38m▄\033[48;2;153;111;81m\033[38;2;140;108;84m▄\033[48;2;188;138;97m\033[38;2;192;144;104m▄\033[48;2;202;151;110m\033[38;2;205;156;115m▄\033[48;2;175;123;86m\033[38;2;188;138;99m▄\033[48;2;179;138;106m\033[38;2;165;127;101m▄\033[48;2;163;131;106m\033[38;2;136;125;117m▄\033[48;2;127;107;91m\033[38;2;96;90;86m▄\033[48;2;109;93;81m\033[38;2;81;75;71m▄\033[48;2;104;87;75m\033[38;2;76;71;67m▄\033[48;2;104;87;73m\033[38;2;75;70;67m▄\033[48;2;99;83;69m\033[38;2;84;81;77m▄\033[48;2;87;71;60m\033[38;2;85;80;77m▄\033[48;2;78;63;52m\033[38;2;82;75;71m▄\033[48;2;77;62;52m\033[38;2;69;59;53m▄\033[48;2;75;61;51m\033[38;2;68;58;50m▄\033[48;2;71;57;48m\033[38;2;62;50;44m▄\033[48;2;74;60;50m\033[38;2;65;53;44m▄\033[48;2;78;63;52m\033[38;2;73;59;49m▄\033[48;2;78;61;49m\033[38;2;76;60;48m▄\033[48;2;80;60;46m\033[38;2;75;57;45m▄\033[48;2;78;57;43m\033[38;2;77;56;43m▄\033[48;2;75;55;41m\033[38;2;70;51;37m▄\033[48;2;76;54;42m\033[38;2;72;52;41m▄\033[48;2;87;63;48m\033[38;2;75;55;41m▄\033[48;2;95;69;52m\033[38;2;89;65;49m▄\033[48;2;96;71;53m\033[38;2;97;71;52m▄\033[48;2;103;78;60m\033[38;2;98;71;52m▄\033[48;2;100;76;57m\033[38;2;95;69;50m▄\033[48;2;90;67;51m\033[38;2;86;63;49m▄\033[48;2;89;66;50m\033[38;2;86;63;47m▄\033[48;2;88;64;47m\033[38;2;87;63;46m▄\033[48;2;88;63;47m\033[38;2;92;66;48m▄\033[48;2;90;63;47m\033[38;2;99;70;51m▄\033[48;2;100;72;51m\033[38;2;108;78;55m▄\033[48;2;121;87;62m\033[38;2;123;89;63m▄\033[48;2;123;87;61m\033[38;2;127;92;65m▄\033[48;2;117;83;57m\033[38;2;120;86;60m▄\033[48;2;111;77;55m\033[38;2;115;81;58m▄\033[48;2;113;80;59m\033[38;2;115;83;62m▄\033[48;2;113;81;59m\033[38;2;117;85;63m▄\033[48;2;108;77;55m\033[38;2;118;86;63m▄\033[48;2;94;66;49m\033[38;2;107;77;57m▄\033[48;2;86;59;44m\033[38;2;94;65;48m▄\033[48;2;94;68;51m\033[38;2;104;75;56m▄\033[48;2;103;81;67m\033[38;2;106;84;70m▄\033[48;2;75;69;64m\033[38;2;83;75;70m▄\033[48;2;14;13;13m\033[38;2;22;20;18m▄\033[48;2;0;0;0m\033[38;2;1;1;1m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;1;1;1m \033[48;2;29;25;23m\033[38;2;28;24;21m▄\033[48;2;123;96;74m\033[38;2;125;97;74m▄\033[48;2;194;148;109m\033[38;2;196;152;113m▄\033[48;2;213;167;125m\033[38;2;208;167;127m▄\033[48;2;180;143;112m\033[38;2;104;93;86m▄\033[48;2;93;87;80m\033[38;2;53;54;55m▄\033[48;2;59;59;58m\033[38;2;48;49;49m▄\033[48;2;52;52;51m\033[38;2;46;46;48m▄\033[48;2;49;49;49m\033[38;2;45;45;47m▄\033[48;2;47;46;48m\033[38;2;41;41;43m▄\033[48;2;46;46;46m\033[38;2;37;37;39m▄\033[48;2;47;47;47m\033[38;2;35;36;37m▄\033[48;2;51;50;50m\033[38;2;36;36;36m▄\033[48;2;65;64;62m\033[38;2;43;43;43m▄\033[48;2;77;75;72m\033[38;2;53;53;52m▄\033[48;2;63;59;55m\033[38;2;54;53;52m▄\033[48;2;60;52;46m\033[38;2;52;46;42m▄\033[48;2;58;47;40m\033[38;2;50;41;35m▄\033[48;2;64;51;43m\033[38;2;54;44;37m▄\033[48;2;71;56;45m\033[38;2;68;54;45m▄\033[48;2;74;57;45m\033[38;2;72;56;45m▄\033[48;2;75;57;43m\033[38;2;72;55;44m▄\033[48;2;68;48;36m\033[38;2;70;52;40m▄\033[48;2;69;51;39m \033[48;2;72;53;41m\033[38;2;67;49;38m▄\033[48;2;84;62;47m\033[38;2;76;56;43m▄\033[48;2;91;64;47m\033[38;2;88;63;47m▄\033[48;2;96;69;49m\033[38;2;95;67;47m▄\033[48;2;92;66;48m\033[38;2;93;65;47m▄\033[48;2;84;59;44m\033[38;2;90;63;46m▄\033[48;2;87;61;46m\033[38;2;88;61;46m▄\033[48;2;87;60;44m\033[38;2;89;62;45m▄\033[48;2;93;65;46m\033[38;2;97;69;50m▄\033[48;2;100;70;50m\033[38;2;107;76;55m▄\033[48;2;113;81;58m\033[38;2;116;84;60m▄\033[48;2;128;93;67m\033[38;2;132;98;70m▄\033[48;2;135;100;72m\033[38;2;138;102;74m▄\033[48;2;127;91;65m\033[38;2;130;95;68m▄\033[48;2;116;82;58m\033[38;2;119;86;62m▄\033[48;2;109;77;56m\033[38;2;111;79;58m▄\033[48;2;114;84;61m\033[38;2;112;81;60m▄\033[48;2;118;86;63m\033[38;2;113;81;58m▄\033[48;2;119;87;66m\033[38;2;120;88;67m▄\033[48;2;109;78;58m\033[38;2;123;88;67m▄\033[48;2;106;76;57m\033[38;2;116;83;62m▄\033[48;2;101;74;59m\033[38;2;101;71;53m▄\033[48;2;86;72;63m\033[38;2;91;71;60m▄\033[48;2;41;38;36m\033[38;2;58;51;48m▄\033[48;2;4;3;3m\033[38;2;10;10;9m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;2;2;2m\033[38;2;6;5;5m▄\033[48;2;38;33;27m\033[38;2;60;50;42m▄\033[48;2;138;107;80m\033[38;2;157;121;90m▄\033[48;2;198;153;113m\033[38;2;185;144;107m▄\033[48;2;161;133;107m\033[38;2;92;81;72m▄\033[48;2;56;55;56m\033[38;2;55;55;57m▄\033[48;2;54;55;57m\033[38;2;67;67;69m▄\033[48;2;54;55;57m\033[38;2;60;61;64m▄\033[48;2;53;54;57m\033[38;2;54;54;58m▄\033[48;2;60;61;64m\033[38;2;66;66;71m▄\033[48;2;58;58;61m\033[38;2;59;60;64m▄\033[48;2;37;37;40m\033[38;2;38;40;42m▄\033[48;2;32;32;34m\033[38;2;32;32;35m▄\033[48;2;32;32;32m\033[38;2;31;32;33m▄\033[48;2;34;35;35m\033[38;2;31;32;31m▄\033[48;2;43;43;43m\033[38;2;37;37;37m▄\033[48;2;45;45;45m\033[38;2;48;47;48m▄\033[48;2;43;41;39m\033[38;2;42;41;40m▄\033[48;2;45;39;35m\033[38;2;39;35;33m▄\033[48;2;53;43;37m\033[38;2;48;40;34m▄\033[48;2;63;50;41m\033[38;2;56;46;38m▄\033[48;2;69;55;44m\033[38;2;64;52;43m▄\033[48;2;71;57;46m\033[38;2;70;57;47m▄\033[48;2;72;56;43m\033[38;2;72;56;45m▄\033[48;2;70;52;42m\033[38;2;67;50;40m▄\033[48;2;67;49;39m\033[38;2;67;50;40m▄\033[48;2;69;50;39m\033[38;2;69;50;40m▄\033[48;2;80;57;43m\033[38;2;73;53;40m▄\033[48;2;91;64;47m\033[38;2;87;63;48m▄\033[48;2;98;70;52m\033[38;2;91;65;48m▄\033[48;2;93;65;46m\033[38;2;94;67;49m▄\033[48;2;91;63;45m\033[38;2;96;67;48m▄\033[48;2;96;67;49m\033[38;2;98;67;48m▄\033[48;2;100;71;51m\033[38;2;100;70;50m▄\033[48;2;107;76;55m\033[38;2;109;79;56m▄\033[48;2;123;91;66m\033[38;2;125;92;68m▄\033[48;2;132;98;70m\033[38;2;125;92;67m▄\033[48;2;134;99;72m\033[38;2;127;93;67m▄\033[48;2;126;91;65m\033[38;2;121;88;62m▄\033[48;2;118;85;60m\033[38;2;115;83;59m▄\033[48;2;109;77;57m\033[38;2;108;78;55m▄\033[48;2;117;86;64m\033[38;2;116;87;65m▄\033[48;2;122;90;67m\033[38;2;126;94;70m▄\033[48;2;123;88;66m\033[38;2;126;90;67m▄\033[48;2;126;91;68m\033[38;2;126;91;66m▄\033[48;2;125;91;68m\033[38;2;126;92;65m▄\033[48;2;114;80;60m\033[38;2;120;85;62m▄\033[48;2;102;76;60m\033[38;2;107;76;56m▄\033[48;2;79;68;60m\033[38;2;95;75;62m▄\033[48;2;33;30;28m\033[38;2;63;54;48m▄\033[48;2;4;4;3m\033[38;2;18;17;15m▄\033[48;2;0;0;0m\033[38;2;1;1;1m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;16;15;14m\033[38;2;35;31;27m▄\033[48;2;95;78;64m\033[38;2;133;107;85m▄\033[48;2;174;133;100m\033[38;2;190;148;111m▄\033[48;2;142;114;90m\033[38;2;125;102;84m▄\033[48;2;64;61;58m\033[38;2;42;38;37m▄\033[48;2;46;45;47m\033[38;2;22;21;22m▄\033[48;2;55;56;57m\033[38;2;24;24;26m▄\033[48;2;52;53;56m\033[38;2;37;37;40m▄\033[48;2;45;46;50m\033[38;2;39;39;42m▄\033[48;2;50;51;55m\033[38;2;42;42;45m▄\033[48;2;44;44;48m\033[38;2;29;29;32m▄\033[48;2;27;28;30m\033[38;2;16;17;18m▄\033[48;2;22;23;25m\033[38;2;12;13;15m▄\033[48;2;22;23;24m\033[38;2;14;15;17m▄\033[48;2;24;24;25m\033[38;2;22;22;23m▄\033[48;2;27;27;27m\033[38;2;25;24;24m▄\033[48;2;43;44;44m\033[38;2;35;36;36m▄\033[48;2;34;35;34m\033[38;2;34;34;34m▄\033[48;2;40;37;36m\033[38;2;45;44;42m▄\033[48;2;41;36;32m\033[38;2;42;36;33m▄\033[48;2;51;43;37m\033[38;2;50;42;37m▄\033[48;2;59;48;41m\033[38;2;56;47;41m▄\033[48;2;69;56;47m\033[38;2;62;51;43m▄\033[48;2;73;59;48m\033[38;2;67;55;46m▄\033[48;2;67;51;42m\033[38;2;64;50;42m▄\033[48;2;64;48;39m\033[38;2;62;48;39m▄\033[48;2;69;51;41m\033[38;2;66;51;41m▄\033[48;2;71;53;41m\033[38;2;71;55;44m▄\033[48;2;80;59;46m\033[38;2;81;61;48m▄\033[48;2;89;66;49m\033[38;2;94;72;56m▄\033[48;2;95;69;52m\033[38;2;102;77;59m▄\033[48;2;102;74;55m\033[38;2;107;81;61m▄\033[48;2;103;73;52m\033[38;2;110;80;60m▄\033[48;2;101;71;51m\033[38;2;106;76;56m▄\033[48;2;108;77;56m\033[38;2;106;75;56m▄\033[48;2;116;85;63m\033[38;2;111;80;60m▄\033[48;2;118;87;66m\033[38;2;109;78;57m▄\033[48;2;118;86;62m\033[38;2;109;77;57m▄\033[48;2;117;84;60m\033[38;2;110;79;57m▄\033[48;2;114;82;58m\033[38;2;112;80;58m▄\033[48;2;111;80;57m\033[38;2;114;82;59m▄\033[48;2;115;85;64m\033[38;2;118;87;64m▄\033[48;2;127;95;71m\033[38;2;133;101;77m▄\033[48;2;124;90;65m\033[38;2;125;90;65m▄\033[48;2;124;89;63m\033[38;2;125;90;66m▄\033[48;2;122;87;61m\033[38;2;123;88;63m▄\033[48;2;120;84;60m\033[38;2;123;86;62m▄\033[48;2;118;82;59m\033[38;2;129;91;67m▄\033[48;2;108;75;53m\033[38;2;130;93;66m▄\033[48;2;89;69;55m\033[38;2;115;82;61m▄\033[48;2;43;38;34m\033[38;2;73;60;50m▄\033[48;2;9;8;8m\033[38;2;28;25;23m▄\033[48;2;0;0;0m\033[38;2;2;2;2m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;1;1;1m\033[38;2;7;7;7m▄\033[48;2;61;53;46m\033[38;2;98;86;75m▄\033[48;2;165;129;100m\033[38;2;189;152;121m▄\033[48;2;192;151;115m\033[38;2;190;155;125m▄\033[48;2;127;107;92m\033[38;2;145;126;111m▄\033[48;2;46;43;43m\033[38;2;54;50;49m▄\033[48;2;24;24;25m\033[38;2;22;22;23m▄\033[48;2;21;21;23m\033[38;2;16;17;19m▄\033[48;2;27;27;29m\033[38;2;24;25;26m▄\033[48;2;38;38;40m\033[38;2;36;36;37m▄\033[48;2;37;37;39m\033[38;2;32;32;34m▄\033[48;2;26;26;28m \033[48;2;15;15;17m\033[38;2;18;18;20m▄\033[48;2;12;13;15m\033[38;2;13;14;16m▄\033[48;2;14;15;17m\033[38;2;13;14;16m▄\033[48;2;24;24;25m\033[38;2;17;17;19m▄\033[48;2;23;23;23m\033[38;2;20;20;20m▄\033[48;2;28;28;28m\033[38;2;22;22;22m▄\033[48;2;44;45;44m\033[38;2;32;32;32m▄\033[48;2;47;44;42m\033[38;2;52;49;47m▄\033[48;2;42;38;35m\033[38;2;45;41;39m▄\033[48;2;48;41;38m\033[38;2;42;38;36m▄\033[48;2;46;38;35m\033[38;2;43;38;33m▄\033[48;2;53;44;39m\033[38;2;49;42;36m▄\033[48;2;56;46;38m\033[38;2;56;46;40m▄\033[48;2;64;52;44m\033[38;2;54;44;38m▄\033[48;2;62;49;41m\033[38;2;63;50;43m▄\033[48;2;68;54;45m\033[38;2;67;53;45m▄\033[48;2;71;55;45m\033[38;2;72;58;49m▄\033[48;2;75;58;47m\033[38;2;72;58;48m▄\033[48;2;88;69;55m\033[38;2;76;60;49m▄\033[48;2;97;75;57m\033[38;2;79;62;51m▄\033[48;2;105;79;60m\033[38;2;99;78;60m▄\033[48;2;110;83;62m\033[38;2;109;82;63m▄\033[48;2;112;82;62m\033[38;2;113;84;64m▄\033[48;2;106;77;57m\033[38;2;110;82;61m▄\033[48;2;107;76;56m\033[38;2;103;75;54m▄\033[48;2;105;75;55m\033[38;2;98;71;51m▄\033[48;2;103;74;54m\033[38;2;96;69;50m▄\033[48;2;108;78;58m\033[38;2;105;75;55m▄\033[48;2;116;84;62m\033[38;2;119;87;64m▄\033[48;2;120;87;63m\033[38;2;129;95;70m▄\033[48;2;125;92;67m\033[38;2;129;96;71m▄\033[48;2;138;106;80m\033[38;2;129;98;70m▄\033[48;2;130;95;69m\033[38;2;126;91;65m▄\033[48;2;121;85;59m\033[38;2;120;84;57m▄\033[48;2;119;85;59m\033[38;2;122;86;60m▄\033[48;2;122;87;61m\033[38;2;132;96;70m▄\033[48;2;131;94;67m\033[38;2;145;106;79m▄\033[48;2;140;103;75m\033[38;2;150;111;82m▄\033[48;2;133;94;65m\033[38;2;146;105;74m▄\033[48;2;102;76;58m\033[38;2;125;88;62m▄\033[48;2;57;49;42m\033[38;2;89;68;55m▄\033[48;2;12;11;10m\033[38;2;32;27;24m▄\033[48;2;0;0;0m\033[38;2;3;3;3m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;2;1;2m▄\033[48;2;19;19;18m\033[38;2;44;41;39m▄\033[48;2;136;115;99m\033[38;2;165;138;115m▄\033[48;2;198;159;127m\033[38;2;199;159;127m▄\033[48;2;186;155;129m\033[38;2;178;149;126m▄\033[48;2;148;132;121m\033[38;2;145;131;124m▄\033[48;2;82;78;79m\033[38;2;141;136;137m▄\033[48;2;38;38;39m\033[38;2;64;62;63m▄\033[48;2;41;41;44m\033[38;2;61;61;63m▄\033[48;2;39;39;40m\033[38;2;42;42;43m▄\033[48;2;32;32;33m\033[38;2;29;28;29m▄\033[48;2;22;22;24m\033[38;2;20;19;20m▄\033[48;2;24;24;26m\033[38;2;24;23;24m▄\033[48;2;22;22;23m\033[38;2;21;20;21m▄\033[48;2;22;22;24m\033[38;2;20;20;21m▄\033[48;2;21;21;22m\033[38;2;24;24;25m▄\033[48;2;24;24;24m\033[38;2;29;29;29m▄\033[48;2;22;22;22m\033[38;2;26;26;27m▄\033[48;2;29;28;29m\033[38;2;41;40;40m▄\033[48;2;48;47;46m\033[38;2;59;58;56m▄\033[48;2;62;58;55m\033[38;2;64;60;57m▄\033[48;2;50;46;45m\033[38;2;54;50;49m▄\033[48;2;44;40;38m\033[38;2;46;42;40m▄\033[48;2;41;36;33m\033[38;2;41;37;35m▄\033[48;2;45;39;35m\033[38;2;43;38;35m▄\033[48;2;50;41;36m\033[38;2;48;41;36m▄\033[48;2;55;45;38m\033[38;2;52;44;37m▄\033[48;2;58;48;42m \033[48;2;67;57;48m\033[38;2;64;55;47m▄\033[48;2;68;57;48m\033[38;2;66;56;48m▄\033[48;2;72;60;50m\033[38;2;65;56;48m▄\033[48;2;65;53;45m\033[38;2;54;45;41m▄\033[48;2;60;49;41m\033[38;2;50;42;36m▄\033[48;2;67;55;44m\033[38;2;48;39;34m▄\033[48;2;99;77;60m\033[38;2;75;59;48m▄\033[48;2;109;82;63m\033[38;2;102;78;61m▄\033[48;2;107;80;60m\033[38;2;104;77;58m▄\033[48;2;100;73;54m\033[38;2;98;71;52m▄\033[48;2;91;65;48m\033[38;2;90;63;46m▄\033[48;2;96;68;51m\033[38;2;95;68;50m▄\033[48;2;108;79;59m\033[38;2;104;77;57m▄\033[48;2;119;88;65m\033[38;2;116;86;64m▄\033[48;2;127;94;69m\033[38;2;123;91;68m▄\033[48;2;130;96;71m\033[38;2;126;92;67m▄\033[48;2;124;89;64m\033[38;2;122;88;63m▄\033[48;2;128;94;69m\033[38;2;121;87;64m▄\033[48;2;126;92;67m\033[38;2;119;86;62m▄\033[48;2;127;91;66m\033[38;2;121;87;63m▄\033[48;2;135;100;73m\033[38;2;130;94;68m▄\033[48;2;144;106;77m\033[38;2;140;102;73m▄\033[48;2;151;112;81m\033[38;2;153;113;83m▄\033[48;2;152;111;79m\033[38;2;153;112;79m▄\033[48;2;137;95;65m\033[38;2;139;96;65m▄\033[48;2;109;79;59m\033[38;2;115;82;60m▄\033[48;2;54;44;38m\033[38;2;64;52;43m▄\033[48;2;10;9;8m\033[38;2;14;12;10m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;5;5;5m\033[38;2;13;13;12m▄\033[48;2;78;71;65m\033[38;2;116;102;91m▄\033[48;2;176;139;112m\033[38;2;185;142;110m▄\033[48;2;195;154;121m\033[38;2;192;149;117m▄\033[48;2;172;146;127m\033[38;2;177;152;137m▄\033[48;2;147;136;133m\033[38;2;152;143;143m▄\033[48;2;175;171;172m\033[38;2;173;168;169m▄\033[48;2;126;125;126m\033[38;2;174;174;176m▄\033[48;2;46;43;45m\033[38;2;97;91;94m▄\033[48;2;31;30;31m\033[38;2;39;34;36m▄\033[48;2;26;25;25m\033[38;2;29;26;27m▄\033[48;2;22;20;21m\033[38;2;27;25;26m▄\033[48;2;23;21;21m\033[38;2;23;21;22m▄\033[48;2;25;22;23m\033[38;2;26;24;25m▄\033[48;2;21;21;22m\033[38;2;29;26;27m▄\033[48;2;27;27;28m\033[38;2;27;26;27m▄\033[48;2;25;24;25m\033[38;2;30;30;30m▄\033[48;2;29;29;30m\033[38;2;39;39;39m▄\033[48;2;48;47;47m\033[38;2;52;52;51m▄\033[48;2;58;56;55m\033[38;2;61;60;58m▄\033[48;2;67;63;60m\033[38;2;62;59;58m▄\033[48;2;62;59;56m\033[38;2;60;58;55m▄\033[48;2;49;46;44m\033[38;2;54;50;48m▄\033[48;2;43;39;37m\033[38;2;51;46;44m▄\033[48;2;44;38;36m\033[38;2;48;42;40m▄\033[48;2;48;41;37m\033[38;2;42;36;33m▄\033[48;2;49;41;36m\033[38;2;56;49;42m▄\033[48;2;64;52;46m\033[38;2;64;53;46m▄\033[48;2;65;55;50m\033[38;2;60;52;47m▄\033[48;2;59;52;46m\033[38;2;52;45;43m▄\033[48;2;56;49;44m\033[38;2;46;41;39m▄\033[48;2;50;44;40m\033[38;2;40;36;34m▄\033[48;2;38;34;31m\033[38;2;30;29;28m▄\033[48;2;32;28;26m\033[38;2;27;24;24m▄\033[48;2;51;42;35m\033[38;2;39;32;30m▄\033[48;2;87;67;54m\033[38;2;77;60;49m▄\033[48;2;104;78;60m\033[38;2;104;79;60m▄\033[48;2;102;75;56m\033[38;2;101;75;56m▄\033[48;2;92;65;48m\033[38;2;92;65;47m▄\033[48;2;92;65;48m\033[38;2;89;62;45m▄\033[48;2;99;72;53m\033[38;2;91;65;46m▄\033[48;2;112;82;62m\033[38;2;103;74;55m▄\033[48;2;116;85;63m\033[38;2;106;75;55m▄\033[48;2;114;81;58m\033[38;2;103;72;51m▄\033[48;2;111;80;58m\033[38;2;97;69;49m▄\033[48;2;109;76;56m\033[38;2;105;75;56m▄\033[48;2;113;79;58m\033[38;2;108;74;55m▄\033[48;2;117;83;60m\033[38;2;118;83;61m▄\033[48;2;131;95;69m\033[38;2;139;102;75m▄\033[48;2;141;102;74m\033[38;2;145;105;76m▄\033[48;2;151;111;81m\033[38;2;146;103;74m▄\033[48;2;152;109;77m\033[38;2;147;103;72m▄\033[48;2;141;97;66m\033[38;2;138;94;64m▄\033[48;2;117;82;59m\033[38;2;118;81;58m▄\033[48;2;70;56;47m\033[38;2;78;63;53m▄\033[48;2;17;16;14m\033[38;2;25;22;20m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;1;1;1m▄\033[48;2;24;23;22m\033[38;2;38;35;33m▄\033[48;2;143;122;106m\033[38;2;156;125;105m▄\033[48;2;187;141;105m\033[38;2;185;132;96m▄\033[48;2;195;146;111m\033[38;2;203;135;107m▄\033[48;2;201;156;137m\033[38;2;206;99;110m▄\033[48;2;181;157;159m\033[38;2;201;104;129m▄\033[48;2;183;176;178m\033[38;2;186;154;164m▄\033[48;2;178;178;180m\033[38;2;182;175;179m▄\033[48;2;138;133;137m\033[38;2;144;136;140m▄\033[48;2;75;70;71m\033[38;2;81;73;76m▄\033[48;2;42;40;41m\033[38;2;56;49;52m▄\033[48;2;35;34;35m\033[38;2;42;39;40m▄\033[48;2;29;27;28m\033[38;2;40;35;36m▄\033[48;2;30;27;28m\033[38;2;53;44;44m▄\033[48;2;36;34;35m\033[38;2;45;42;43m▄\033[48;2;34;32;33m \033[48;2;36;34;35m\033[38;2;39;37;38m▄\033[48;2;50;48;49m\033[38;2;49;49;49m▄\033[48;2;65;65;63m\033[38;2;58;57;57m▄\033[48;2;71;71;67m\033[38;2;71;70;70m▄\033[48;2;66;65;63m\033[38;2;73;72;68m▄\033[48;2;67;65;62m\033[38;2;62;58;56m▄\033[48;2;60;57;53m\033[38;2;65;60;56m▄\033[48;2;52;48;45m\033[38;2;56;52;49m▄\033[48;2;38;33;32m\033[38;2;46;42;40m▄\033[48;2;39;34;31m\033[38;2;44;39;37m▄\033[48;2;50;43;39m\033[38;2;46;41;37m▄\033[48;2;57;48;43m\033[38;2;47;42;39m▄\033[48;2;56;48;45m\033[38;2;48;43;41m▄\033[48;2;47;42;40m\033[38;2;42;39;37m▄\033[48;2;40;38;36m\033[38;2;37;36;34m▄\033[48;2;33;31;31m\033[38;2;33;32;32m▄\033[48;2;30;28;29m\033[38;2;32;31;31m▄\033[48;2;22;21;22m\033[38;2;21;21;22m▄\033[48;2;32;28;26m\033[38;2;24;22;21m▄\033[48;2;67;55;45m\033[38;2;53;44;37m▄\033[48;2;101;78;61m\033[38;2;91;71;55m▄\033[48;2;102;77;57m\033[38;2;97;74;56m▄\033[48;2;96;71;51m\033[38;2;89;68;51m▄\033[48;2;87;61;44m\033[38;2;75;54;41m▄\033[48;2;85;59;43m\033[38;2;78;55;41m▄\033[48;2;90;63;46m\033[38;2;80;57;41m▄\033[48;2;93;65;48m\033[38;2;81;56;41m▄\033[48;2;89;62;44m\033[38;2;80;54;40m▄\033[48;2;81;54;39m\033[38;2;84;57;42m▄\033[48;2;88;59;42m\033[38;2;97;67;49m▄\033[48;2;107;74;54m\033[38;2;118;83;61m▄\033[48;2;128;93;68m\033[38;2;132;95;70m▄\033[48;2;141;103;75m\033[38;2;139;99;71m▄\033[48;2;141;99;70m\033[38;2;137;94;66m▄\033[48;2;140;96;67m\033[38;2;136;92;64m▄\033[48;2;136;92;63m\033[38;2;129;85;58m▄\033[48;2;128;83;56m\033[38;2;117;74;50m▄\033[48;2;112;74;52m\033[38;2;107;73;52m▄\033[48;2;83;63;52m\033[38;2;83;67;57m▄\033[48;2;34;29;26m\033[38;2;37;33;30m▄\033[48;2;2;2;2m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;2;2;1m\033[38;2;2;2;2m▄\033[48;2;49;44;41m\033[38;2;52;47;43m▄\033[48;2;161;124;100m\033[38;2;160;119;95m▄\033[48;2;192;126;99m\033[38;2;199;105;103m▄\033[48;2;211;100;106m\033[38;2;220;67;105m▄\033[48;2;218;63;105m\033[38;2;222;58;111m▄\033[48;2;214;65;112m\033[38;2;220;60;119m▄\033[48;2;198;91;126m\033[38;2;216;61;119m▄\033[48;2;188;137;153m\033[38;2;204;65;114m▄\033[48;2;159;130;140m\033[38;2;185;78;112m▄\033[48;2;108;85;91m\033[38;2;161;71;98m▄\033[48;2;79;59;65m\033[38;2;136;57;77m▄\033[48;2;65;43;49m\033[38;2;128;46;67m▄\033[48;2;79;51;57m\033[38;2;129;42;63m▄\033[48;2;104;61;68m\033[38;2;137;42;64m▄\033[48;2;64;43;48m\033[38;2;140;50;71m▄\033[48;2;53;41;46m\033[38;2;118;50;68m▄\033[48;2;45;40;41m\033[38;2;98;45;60m▄\033[48;2;50;48;49m\033[38;2;93;51;62m▄\033[48;2;60;58;58m\033[38;2;85;55;61m▄\033[48;2;75;73;74m\033[38;2;78;61;64m▄\033[48;2;70;67;63m\033[38;2;68;57;56m▄\033[48;2;60;55;52m\033[38;2;68;58;56m▄\033[48;2;64;59;56m\033[38;2;66;57;55m▄\033[48;2;58;53;50m\033[38;2;69;60;57m▄\033[48;2;51;45;43m\033[38;2;68;59;56m▄\033[48;2;39;35;34m\033[38;2;63;55;53m▄\033[48;2;53;49;45m\033[38;2;58;50;49m▄\033[48;2;49;45;42m\033[38;2;65;59;56m▄\033[48;2;47;43;42m\033[38;2;53;46;44m▄\033[48;2;43;41;40m\033[38;2;48;42;40m▄\033[48;2;40;39;38m\033[38;2;38;35;33m▄\033[48;2;38;37;37m\033[38;2;32;32;31m▄\033[48;2;35;33;34m\033[38;2;33;31;31m▄\033[48;2;32;32;31m\033[38;2;35;34;33m▄\033[48;2;24;23;23m\033[38;2;32;30;29m▄\033[48;2;44;36;32m\033[38;2;43;36;32m▄\033[48;2;79;61;48m\033[38;2;66;50;39m▄\033[48;2;89;69;52m\033[38;2;68;51;39m▄\033[48;2;70;54;43m\033[38;2;60;45;35m▄\033[48;2;71;52;40m\033[38;2;70;50;38m▄\033[48;2;81;58;45m\033[38;2;75;52;39m▄\033[48;2;80;57;42m\033[38;2;76;54;40m▄\033[48;2;78;54;40m \033[48;2;83;57;43m\033[38;2;88;61;45m▄\033[48;2;96;67;49m\033[38;2;102;72;52m▄\033[48;2;108;76;55m\033[38;2;111;78;56m▄\033[48;2;121;85;61m\033[38;2;119;83;60m▄\033[48;2;130;93;66m\033[38;2;120;83;58m▄\033[48;2;133;93;65m\033[38;2;119;80;54m▄\033[48;2;127;86;58m\033[38;2;116;76;49m▄\033[48;2;123;80;54m\033[38;2;109;68;43m▄\033[48;2;114;71;47m\033[38;2;99;60;39m▄\033[48;2;105;66;44m\033[38;2;97;60;40m▄\033[48;2;104;73;54m\033[38;2;96;62;43m▄\033[48;2;84;69;60m\033[38;2;88;64;51m▄\033[48;2;41;37;34m\033[38;2;56;47;42m▄\033[48;2;3;3;3m\033[38;2;8;6;6m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;1;1;0m\033[38;2;0;0;0m▄\033[48;2;37;34;31m\033[38;2;20;18;17m▄\033[48;2;154;121;106m\033[38;2;138;97;96m▄\033[48;2;208;85;104m\033[38;2;213;53;87m▄\033[48;2;223;58;102m\033[38;2;223;53;98m▄\033[48;2;226;61;114m\033[38;2;228;56;108m▄\033[48;2;224;57;118m\033[38;2;229;55;115m▄\033[48;2;216;53;114m\033[38;2;223;83;138m▄\033[48;2;196;53;97m\033[38;2;195;67;109m▄\033[48;2;207;58;107m\033[38;2;211;66;113m▄\033[48;2;200;49;103m\033[38;2;215;55;115m▄\033[48;2;169;41;84m\033[38;2;199;49;106m▄\033[48;2;161;38;75m\033[38;2;189;43;91m▄\033[48;2;150;34;71m\033[38;2;183;40;88m▄\033[48;2;147;33;65m\033[38;2;177;37;82m▄\033[48;2;160;35;68m\033[38;2;173;34;75m▄\033[48;2;158;31;66m\033[38;2;170;30;70m▄\033[48;2;154;33;68m\033[38;2;167;28;68m▄\033[48;2;147;35;66m\033[38;2;156;29;66m▄\033[48;2;142;40;66m\033[38;2;155;32;65m▄\033[48;2;126;43;62m\033[38;2;161;35;67m▄\033[48;2;115;43;60m\033[38;2;162;25;62m▄\033[48;2;72;48;51m\033[38;2;79;32;43m▄\033[48;2;70;52;53m\033[38;2;53;30;34m▄\033[48;2;75;57;56m\033[38;2;44;28;31m▄\033[48;2;84;65;62m\033[38;2;58;37;40m▄\033[48;2;91;73;70m\033[38;2;66;42;45m▄\033[48;2;77;62;61m\033[38;2;58;40;41m▄\033[48;2;63;51;50m\033[38;2;49;34;34m▄\033[48;2;54;39;39m\033[38;2;56;37;37m▄\033[48;2;47;34;33m\033[38;2;58;42;39m▄\033[48;2;25;22;20m\033[38;2;36;30;26m▄\033[48;2;14;15;16m\033[38;2;13;14;14m▄\033[48;2;22;21;23m\033[38;2;18;18;20m▄\033[48;2;22;22;23m\033[38;2;18;17;19m▄\033[48;2;33;31;29m\033[38;2;33;30;29m▄\033[48;2;47;40;36m\033[38;2;55;45;39m▄\033[48;2;66;49;39m\033[38;2;75;56;43m▄\033[48;2;73;54;40m\033[38;2;84;61;46m▄\033[48;2;75;54;40m\033[38;2;82;57;42m▄\033[48;2;73;51;38m\033[38;2;79;55;41m▄\033[48;2;74;51;38m\033[38;2;79;57;43m▄\033[48;2;77;55;40m\033[38;2;85;61;46m▄\033[48;2;81;56;41m\033[38;2;87;61;45m▄\033[48;2;89;62;45m\033[38;2;88;61;44m▄\033[48;2;99;68;48m\033[38;2;98;66;47m▄\033[48;2;107;73;51m\033[38;2;109;74;53m▄\033[48;2;111;75;51m\033[38;2;110;73;49m▄\033[48;2;111;74;49m\033[38;2;110;74;49m▄\033[48;2;114;77;52m \033[48;2;114;74;49m\033[38;2;107;69;46m▄\033[48;2;104;64;40m\033[38;2;95;58;38m▄\033[48;2;94;57;36m\033[38;2;90;55;35m▄\033[48;2;91;56;37m\033[38;2;89;55;35m▄\033[48;2;93;58;40m\033[38;2;94;60;41m▄\033[48;2;101;75;61m\033[38;2;107;81;65m▄\033[48;2;68;59;53m\033[38;2;74;64;58m▄\033[48;2;8;7;6m\033[38;2;9;8;7m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;10;10;10m\033[38;2;12;12;12m▄\033[48;2;119;62;74m\033[38;2;126;43;65m▄\033[48;2;209;47;83m\033[38;2;213;68;109m▄\033[48;2;220;59;105m\033[38;2;222;72;119m▄\033[48;2;228;57;110m\033[38;2;232;80;134m▄\033[48;2;235;66;134m\033[38;2;238;79;146m▄\033[48;2;231;91;153m\033[38;2;227;73;142m▄\033[48;2;217;86;137m\033[38;2;229;100;152m▄\033[48;2;219;76;125m\033[38;2;229;90;142m▄\033[48;2;218;55;115m\033[38;2;216;60;118m▄\033[48;2;210;45;106m\033[38;2;208;54;108m▄\033[48;2;203;40;96m\033[38;2;206;48;100m▄\033[48;2;201;41;96m\033[38;2;211;48;108m▄\033[48;2;181;35;84m\033[38;2;194;41;99m▄\033[48;2;163;29;68m\033[38;2;177;38;87m▄\033[48;2;172;32;71m\033[38;2;174;34;75m▄\033[48;2;179;35;74m\033[38;2;187;38;80m▄\033[48;2;172;33;75m\033[38;2;191;40;86m▄\033[48;2;164;38;76m\033[38;2;188;44;92m▄\033[48;2;173;45;84m\033[38;2;184;40;86m▄\033[48;2;183;24;70m\033[38;2;189;23;71m▄\033[48;2;136;31;60m\033[38;2;160;28;64m▄\033[48;2;59;26;33m\033[38;2;75;27;39m▄\033[48;2;36;19;23m\033[38;2;63;25;34m▄\033[48;2;45;24;29m\033[38;2;66;31;38m▄\033[48;2;53;28;33m\033[38;2;72;41;44m▄\033[48;2;61;40;43m\033[38;2;80;54;57m▄\033[48;2;64;44;44m\033[38;2;89;61;60m▄\033[48;2;76;53;52m\033[38;2;95;66;65m▄\033[48;2;74;57;51m\033[38;2;55;40;40m▄\033[48;2;45;38;33m\033[38;2;21;19;20m▄\033[48;2;15;15;17m\033[38;2;14;15;17m▄\033[48;2;17;17;19m\033[38;2;18;18;20m▄\033[48;2;20;19;21m\033[38;2;22;22;22m▄\033[48;2;37;34;32m\033[38;2;38;35;34m▄\033[48;2;62;50;42m \033[48;2;83;62;47m\033[38;2;80;59;44m▄\033[48;2;87;62;47m\033[38;2;80;57;41m▄\033[48;2;82;57;42m\033[38;2;76;54;40m▄\033[48;2;80;57;42m\033[38;2;82;60;46m▄\033[48;2;83;59;44m\033[38;2;91;67;51m▄\033[48;2;90;65;49m\033[38;2;93;67;50m▄\033[48;2;88;62;45m\033[38;2;95;67;49m▄\033[48;2;89;62;45m\033[38;2;101;71;51m▄\033[48;2;104;72;52m\033[38;2;101;70;49m▄\033[48;2;109;75;52m\033[38;2;101;66;44m▄\033[48;2;111;75;51m\033[38;2;107;71;48m▄\033[48;2;111;74;48m\033[38;2;108;72;48m▄\033[48;2;105;68;45m\033[38;2;98;62;40m▄\033[48;2;96;60;40m\033[38;2;89;54;34m▄\033[48;2;90;56;37m\033[38;2;90;57;38m▄\033[48;2;90;56;37m\033[38;2;95;59;40m▄\033[48;2;89;55;35m\033[38;2;94;58;38m▄\033[48;2;92;57;38m\033[38;2;97;63;44m▄\033[48;2;103;72;55m\033[38;2;110;85;71m▄\033[48;2;76;61;53m\033[38;2;71;62;56m▄\033[48;2;9;9;7m\033[38;2;6;6;5m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;17;12;13m\033[38;2;18;10;11m▄\033[48;2;147;30;55m\033[38;2;149;35;59m▄\033[48;2;216;50;91m\033[38;2;218;50;90m▄\033[48;2;223;63;111m\033[38;2;226;59;107m▄\033[48;2;237;101;151m\033[38;2;236;88;140m▄\033[48;2;244;118;173m\033[38;2;239;69;130m▄\033[48;2;238;88;160m\033[38;2;234;52;131m▄\033[48;2;232;79;149m\033[38;2;227;78;142m▄\033[48;2;224;70;137m\033[38;2;222;80;137m▄\033[48;2;210;59;118m\033[38;2;213;59;117m▄\033[48;2;194;48;98m\033[38;2;209;52;106m▄\033[48;2;196;45;91m\033[38;2;208;54;102m▄\033[48;2;207;43;96m\033[38;2;208;42;94m▄\033[48;2;203;38;98m\033[38;2;202;35;91m▄\033[48;2;191;37;93m\033[38;2;198;36;91m▄\033[48;2;193;39;91m\033[38;2;196;34;88m▄\033[48;2;204;47;98m\033[38;2;197;34;89m▄\033[48;2;199;42;92m\033[38;2;195;34;90m▄\033[48;2;194;39;91m\033[38;2;195;36;89m▄\033[48;2;190;32;82m\033[38;2;196;46;93m▄\033[48;2;189;31;77m\033[38;2;189;27;76m▄\033[48;2;161;27;64m\033[38;2;165;27;68m▄\033[48;2;96;29;48m\033[38;2;108;29;51m▄\033[48;2;88;30;44m\033[38;2;100;35;50m▄\033[48;2;90;42;51m\033[38;2;106;53;62m▄\033[48;2;95;56;60m\033[38;2;119;69;75m▄\033[48;2;99;62;65m\033[38;2;124;75;79m▄\033[48;2;108;71;72m\033[38;2;119;78;79m▄\033[48;2;104;72;70m\033[38;2;98;71;67m▄\033[48;2;65;49;45m\033[38;2;49;40;36m▄\033[48;2;22;20;21m\033[38;2;20;19;19m▄\033[48;2;17;18;18m\033[38;2;19;20;21m▄\033[48;2;19;20;19m \033[48;2;23;23;23m\033[38;2;25;24;23m▄\033[48;2;33;30;28m\033[38;2;41;35;31m▄\033[48;2;55;43;35m\033[38;2;62;46;36m▄\033[48;2;76;56;42m\033[38;2;74;55;41m▄\033[48;2;78;56;41m\033[38;2;73;51;38m▄\033[48;2;75;53;39m\033[38;2;77;55;41m▄\033[48;2;82;59;45m\033[38;2;83;61;45m▄\033[48;2;88;63;46m \033[48;2;92;65;47m\033[38;2;88;61;43m▄\033[48;2;96;67;47m\033[38;2;96;66;46m▄\033[48;2;98;66;46m\033[38;2;100;67;46m▄\033[48;2;95;62;42m\033[38;2;96;62;42m▄\033[48;2;96;61;41m\033[38;2;99;64;44m▄\033[48;2;97;62;42m\033[38;2;94;59;39m▄\033[48;2;92;56;37m\033[38;2;87;51;33m▄\033[48;2;87;52;33m\033[38;2;89;54;35m▄\033[48;2;89;54;34m\033[38;2;92;57;37m▄\033[48;2;92;57;37m\033[38;2;96;60;40m▄\033[48;2;97;60;41m\033[38;2;99;62;42m▄\033[48;2;96;59;40m \033[48;2;97;62;43m\033[38;2;96;61;42m▄\033[48;2;101;75;61m\033[38;2;97;70;55m▄\033[48;2;53;44;39m\033[38;2;49;39;34m▄\033[48;2;3;2;2m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;9;7;8m\033[38;2;2;2;2m▄\033[48;2;105;31;47m\033[38;2;47;23;29m▄\033[48;2;211;47;86m\033[38;2;181;43;75m▄\033[48;2;225;63;111m\033[38;2;222;56;99m▄\033[48;2;227;62;118m\033[38;2;223;56;106m▄\033[48;2;230;57;114m\033[38;2;224;53;106m▄\033[48;2;230;51;112m\033[38;2;226;56;111m▄\033[48;2;223;54;113m\033[38;2;225;55;113m▄\033[48;2;217;58;114m\033[38;2;222;55;114m▄\033[48;2;216;55;110m\033[38;2;222;52;115m▄\033[48;2;215;50;106m\033[38;2;221;48;118m▄\033[48;2;212;49;102m\033[38;2;208;39;103m▄\033[48;2;208;40;92m\033[38;2;202;35;93m▄\033[48;2;204;34;88m\033[38;2;207;36;91m▄\033[48;2;200;36;93m\033[38;2;208;42;102m▄\033[48;2;191;35;94m\033[38;2;191;36;99m▄\033[48;2;191;35;93m\033[38;2;191;37;97m▄\033[48;2;193;35;94m\033[38;2;197;42;101m▄\033[48;2;196;32;86m\033[38;2;204;45;95m▄\033[48;2;201;45;95m\033[38;2;201;41;92m▄\033[48;2;201;30;91m\033[38;2;200;26;83m▄\033[48;2;168;34;79m\033[38;2;144;31;66m▄\033[48;2;120;33;57m\033[38;2;124;38;60m▄\033[48;2;112;43;58m\033[38;2;124;54;69m▄\033[48;2;122;64;74m\033[38;2;136;74;84m▄\033[48;2;140;81;90m\033[38;2;140;83;90m▄\033[48;2;132;81;85m\033[38;2;125;75;78m▄\033[48;2;111;73;73m\033[38;2;87;61;58m▄\033[48;2;60;48;47m\033[38;2;28;26;27m▄\033[48;2;28;27;27m\033[38;2;21;21;23m▄\033[48;2;19;19;20m\033[38;2;21;21;22m▄\033[48;2;21;21;23m\033[38;2;21;21;21m▄\033[48;2;22;22;22m\033[38;2;22;21;22m▄\033[48;2;29;27;26m\033[38;2;30;28;27m▄\033[48;2;44;35;30m\033[38;2;49;38;31m▄\033[48;2;65;48;36m\033[38;2;65;47;35m▄\033[48;2;70;51;38m\033[38;2;65;46;34m▄\033[48;2;70;50;36m\033[38;2;67;48;35m▄\033[48;2;78;56;42m\033[38;2;80;58;43m▄\033[48;2;85;62;46m\033[38;2;90;65;48m▄\033[48;2;87;62;44m\033[38;2;91;63;45m▄\033[48;2;87;60;42m\033[38;2;90;60;42m▄\033[48;2;94;64;45m\033[38;2;92;61;43m▄\033[48;2;98;66;45m\033[38;2;94;63;42m▄\033[48;2;93;60;39m\033[38;2;92;60;39m▄\033[48;2;94;59;39m\033[38;2;93;58;38m▄\033[48;2;91;56;36m\033[38;2;90;56;37m▄\033[48;2;89;54;35m\033[38;2;92;57;37m▄\033[48;2;94;58;39m\033[38;2;95;60;40m▄\033[48;2;100;64;42m\033[38;2;105;67;45m▄\033[48;2;100;63;41m\033[38;2;103;64;40m▄\033[48;2;96;59;38m\033[38;2;88;53;33m▄\033[48;2;88;53;34m\033[38;2;80;47;31m▄\033[48;2;90;57;39m\033[38;2;82;50;34m▄\033[48;2;100;75;61m\033[38;2;91;64;49m▄\033[48;2;56;48;43m\033[38;2;68;55;49m▄\033[48;2;4;3;3m\033[38;2;9;7;7m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "";
                auto r_grut03 = ansi::esc(
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;15;13;13m\033[38;2;4;4;4m▄\033[48;2;134;38;60m\033[38;2;92;25;39m▄\033[48;2;215;47;85m\033[38;2;199;39;76m▄\033[48;2;223;45;93m\033[38;2;224;43;92m▄\033[48;2;223;48;102m\033[38;2;225;43;102m▄\033[48;2;226;51;114m\033[38;2;223;47;115m▄\033[48;2;223;48;116m\033[38;2;222;53;123m▄\033[48;2;224;50;115m\033[38;2;222;53;120m▄\033[48;2;226;46;119m\033[38;2;232;51;125m▄\033[48;2;222;45;125m\033[38;2;236;52;137m▄\033[48;2;213;43;117m\033[38;2;223;52;131m▄\033[48;2;203;42;103m\033[38;2;215;58;119m▄\033[48;2;210;43;99m\033[38;2;214;49;107m▄\033[48;2;209;40;100m\033[38;2;208;38;98m▄\033[48;2;196;37;99m\033[38;2;201;36;96m▄\033[48;2;196;36;95m\033[38;2;196;33;89m▄\033[48;2;199;34;92m\033[38;2;199;36;90m▄\033[48;2;201;35;85m\033[38;2;198;36;86m▄\033[48;2;194;31;79m\033[38;2;192;31;77m▄\033[48;2;174;23;67m\033[38;2;153;27;63m▄\033[48;2;133;33;61m\033[38;2;122;39;59m▄\033[48;2;126;47;66m\033[38;2;132;61;75m▄\033[48;2;132;67;80m\033[38;2;137;75;85m▄\033[48;2;138;79;87m\033[38;2;129;74;79m▄\033[48;2;126;73;79m\033[38;2;114;67;70m▄\033[48;2;102;65;66m\033[38;2;62;45;46m▄\033[48;2;33;29;30m\033[38;2;16;16;18m▄\033[48;2;20;20;22m\033[38;2;21;21;23m▄\033[48;2;19;19;21m\033[38;2;22;22;23m▄\033[48;2;22;22;24m\033[38;2;23;23;24m▄\033[48;2;23;23;23m\033[38;2;27;25;26m▄\033[48;2;28;27;26m\033[38;2;33;31;30m▄\033[48;2;32;28;27m\033[38;2;36;32;28m▄\033[48;2;54;41;32m\033[38;2;53;39;30m▄\033[48;2;59;43;32m\033[38;2;56;40;30m▄\033[48;2;61;43;31m\033[38;2;58;41;29m▄\033[48;2;68;49;36m\033[38;2;71;51;38m▄\033[48;2;82;59;45m\033[38;2;84;60;45m▄\033[48;2;95;68;50m\033[38;2;92;65;46m▄\033[48;2;98;69;49m\033[38;2;97;66;46m▄\033[48;2;100;68;47m\033[38;2;101;68;47m▄\033[48;2;98;65;45m\033[38;2;97;63;42m▄\033[48;2;96;63;41m\033[38;2;99;63;41m▄\033[48;2;92;58;37m\033[38;2;95;58;37m▄\033[48;2;92;57;37m\033[38;2;90;54;34m▄\033[48;2;93;59;40m\033[38;2;95;59;40m▄\033[48;2;94;59;39m\033[38;2;106;69;46m▄\033[48;2;100;64;42m\033[38;2;113;74;48m▄\033[48;2;107;70;46m\033[38;2;109;68;44m▄\033[48;2;105;66;42m\033[38;2;95;59;38m▄\033[48;2;82;48;31m\033[38;2;77;46;30m▄\033[48;2;75;44;30m\033[38;2;70;41;27m▄\033[48;2;79;47;32m\033[38;2;75;45;30m▄\033[48;2;86;55;40m\033[38;2;84;52;38m▄\033[48;2;80;61;52m\033[38;2;92;71;61m▄\033[48;2;15;13;12m\033[38;2;29;24;22m▄\033[48;2;0;0;0m\033[38;2;1;1;1m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;48;19;25m\033[38;2;24;20;20m▄\033[48;2;164;40;71m\033[38;2;121;71;79m▄\033[48;2;218;37;90m\033[38;2;201;65;100m▄\033[48;2;223;38;98m\033[38;2;222;47;100m▄\033[48;2;224;50;114m\033[38;2;227;55;110m▄\033[48;2;225;58;124m\033[38;2;226;55;113m▄\033[48;2;225;58;126m\033[38;2;229;57;121m▄\033[48;2;232;54;130m\033[38;2;236;60;136m▄\033[48;2;239;53;139m\033[38;2;237;56;140m▄\033[48;2;232;52;134m\033[38;2;227;50;127m▄\033[48;2;220;45;115m\033[38;2;217;41;108m▄\033[48;2;211;37;100m\033[38;2;204;32;93m▄\033[48;2;206;34;94m\033[38;2;207;35;95m▄\033[48;2;195;35;87m\033[38;2;191;30;80m▄\033[48;2;193;35;86m\033[38;2;190;32;80m▄\033[48;2;195;34;85m\033[38;2;193;32;85m▄\033[48;2;194;31;82m\033[38;2;189;33;87m▄\033[48;2;184;29;80m\033[38;2;148;30;71m▄\033[48;2;136;32;61m\033[38;2;144;50;75m▄\033[48;2;130;52;69m\033[38;2;149;75;90m▄\033[48;2;145;77;88m\033[38;2;148;81;89m▄\033[48;2;137;77;84m\033[38;2;147;94;98m▄\033[48;2;117;68;72m\033[38;2;109;76;75m▄\033[48;2;93;57;59m\033[38;2;50;39;39m▄\033[48;2;25;24;25m\033[38;2;29;28;28m▄\033[48;2;22;22;23m\033[38;2;30;29;30m▄\033[48;2;25;25;26m\033[38;2;30;29;29m▄\033[48;2;25;25;26m\033[38;2;27;27;27m▄\033[48;2;25;25;25m\033[38;2;29;28;28m▄\033[48;2;27;26;25m\033[38;2;32;29;27m▄\033[48;2;25;22;22m\033[38;2;35;28;25m▄\033[48;2;41;31;26m\033[38;2;44;32;25m▄\033[48;2;50;36;27m\033[38;2;51;36;27m▄\033[48;2;54;39;30m\033[38;2;57;41;31m▄\033[48;2;63;45;34m\033[38;2;72;52;38m▄\033[48;2;74;54;40m\033[38;2;79;57;41m▄\033[48;2;85;59;44m\033[38;2;83;57;40m▄\033[48;2;96;67;46m\033[38;2;95;64;44m▄\033[48;2;99;67;46m\033[38;2;99;67;45m▄\033[48;2;98;64;42m \033[48;2;100;65;43m\033[38;2;96;61;41m▄\033[48;2;98;62;40m\033[38;2;97;61;40m▄\033[48;2;95;58;36m\033[38;2;99;62;39m▄\033[48;2;97;62;40m\033[38;2;100;64;40m▄\033[48;2;102;64;42m\033[38;2;104;67;43m▄\033[48;2;105;67;43m\033[38;2;106;68;44m▄\033[48;2;111;72;47m\033[38;2;103;65;43m▄\033[48;2;109;69;46m\033[38;2;98;62;41m▄\033[48;2;90;56;35m\033[38;2;82;50;33m▄\033[48;2;72;43;30m\033[38;2;67;40;27m▄\033[48;2;69;41;28m\033[38;2;67;40;27m▄\033[48;2;74;44;30m\033[38;2;71;42;28m▄\033[48;2;79;47;32m\033[38;2;75;45;30m▄\033[48;2;83;54;39m\033[38;2;83;51;34m▄\033[48;2;58;43;36m\033[38;2;84;56;41m▄\033[48;2;10;8;7m\033[38;2;41;31;26m▄\033[48;2;0;0;0m\033[38;2;5;5;4m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;16;14;13m\033[38;2;11;10;9m▄\033[48;2;98;77;68m\033[38;2;80;66;57m▄\033[48;2;180;99;94m\033[38;2;162;114;86m▄\033[48;2;214;64;96m\033[38;2;189;102;84m▄\033[48;2;224;49;97m\033[38;2;207;65;86m▄\033[48;2;224;47;100m\033[38;2;215;44;88m▄\033[48;2;227;50;111m\033[38;2;217;45;98m▄\033[48;2;229;52;122m\033[38;2;220;46;106m▄\033[48;2;226;49;122m\033[38;2;221;54;116m▄\033[48;2;215;44;110m\033[38;2;215;52;109m▄\033[48;2;214;40;104m\033[38;2;222;48;116m▄\033[48;2;207;36;96m\033[38;2;212;40;101m▄\033[48;2;212;41;104m\033[38;2;207;47;103m▄\033[48;2;201;36;90m\033[38;2;170;41;72m▄\033[48;2;178;28;71m\033[38;2;136;30;56m▄\033[48;2;169;31;76m\033[38;2;122;34;57m▄\033[48;2;148;34;73m\033[38;2;122;44;64m▄\033[48;2;128;41;66m\033[38;2;144;67;86m▄\033[48;2;161;78;98m\033[38;2;179;106;122m▄\033[48;2;164;90;105m\033[38;2;179;103;118m▄\033[48;2;165;92;100m\033[38;2;201;128;136m▄\033[48;2;195;151;150m\033[38;2;224;183;173m▄\033[48;2;129;111;105m\033[38;2;136;113;103m▄\033[48;2;41;36;36m\033[38;2;45;40;40m▄\033[48;2;37;35;34m\033[38;2;39;35;35m▄\033[48;2;33;31;32m\033[38;2;31;29;29m▄\033[48;2;29;28;29m\033[38;2;28;26;26m▄\033[48;2;30;29;28m\033[38;2;31;28;27m▄\033[48;2;34;31;30m\033[38;2;34;28;25m▄\033[48;2;34;27;25m\033[38;2;36;26;23m▄\033[48;2;39;31;25m\033[38;2;40;31;25m▄\033[48;2;47;33;25m\033[38;2;50;37;28m▄\033[48;2;54;39;30m\033[38;2;57;42;31m▄\033[48;2;61;44;33m\033[38;2;65;46;34m▄\033[48;2;75;54;39m\033[38;2;79;57;39m▄\033[48;2;84;59;42m\033[38;2;92;65;46m▄\033[48;2;94;67;46m\033[38;2;107;77;55m▄\033[48;2;92;61;41m\033[38;2;96;64;43m▄\033[48;2;98;65;44m\033[38;2;95;61;40m▄\033[48;2;102;67;43m\033[38;2;104;68;43m▄\033[48;2;97;63;41m\033[38;2;104;68;44m▄\033[48;2;99;64;43m\033[38;2;105;69;47m▄\033[48;2;111;73;50m\033[38;2;115;77;52m▄\033[48;2;115;76;50m\033[38;2;120;79;51m▄\033[48;2;114;75;49m\033[38;2;114;73;47m▄\033[48;2;109;71;46m\033[38;2;108;69;44m▄\033[48;2;92;57;37m\033[38;2;90;57;37m▄\033[48;2;76;46;30m\033[38;2;68;42;28m▄\033[48;2;68;42;28m\033[38;2;65;42;29m▄\033[48;2;66;40;27m\033[38;2;68;43;29m▄\033[48;2;69;42;29m\033[38;2;74;46;31m▄\033[48;2;74;44;30m\033[38;2;82;51;34m▄\033[48;2;76;46;30m\033[38;2;86;52;34m▄\033[48;2;83;50;33m\033[38;2;89;54;34m▄\033[48;2;92;58;40m\033[38;2;95;58;38m▄\033[48;2;76;53;40m\033[38;2;93;59;40m▄\033[48;2;31;24;20m\033[38;2;72;48;35m▄\033[48;2;4;3;3m\033[38;2;30;22;18m▄\033[48;2;0;0;0m\033[38;2;4;3;2m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;4;4;4m\033[38;2;1;1;1m▄\033[48;2;57;49;45m\033[38;2;36;32;29m▄\033[48;2;150;116;93m\033[38;2;128;99;81m▄\033[48;2;176;122;84m\033[38;2;173;123;86m▄\033[48;2;186;121;84m\033[38;2;184;126;84m▄\033[48;2;191;115;86m\033[38;2;187;128;85m▄\033[48;2;205;93;99m\033[38;2;193;131;89m▄\033[48;2;216;65;108m\033[38;2;202;137;101m▄\033[48;2;218;71;117m\033[38;2;206;146;111m▄\033[48;2;212;85;110m\033[38;2;211;160;124m▄\033[48;2;222;116;127m\033[38;2;215;172;134m▄\033[48;2;208;125;122m\033[38;2;211;170;137m▄\033[48;2;175;96;97m\033[38;2;130;92;86m▄\033[48;2;143;68;74m\033[38;2;101;65;66m▄\033[48;2;130;53;71m\033[38;2;128;75;86m▄\033[48;2;139;61;78m\033[38;2;178;101;119m▄\033[48;2;156;78;96m\033[38;2;188;109;127m▄\033[48;2;170;93;110m\033[38;2;192;116;133m▄\033[48;2;186;112;126m\033[38;2;188;116;130m▄\033[48;2;188;114;127m\033[38;2;171;106;113m▄\033[48;2;212;158;160m\033[38;2;146;107;105m▄\033[48;2;214;186;165m\033[38;2;136;117;101m▄\033[48;2;90;72;64m\033[38;2;54;46;40m▄\033[48;2;38;32;32m\033[38;2;35;33;32m▄\033[48;2;32;30;29m\033[38;2;26;25;25m▄\033[48;2;30;28;28m\033[38;2;41;37;36m▄\033[48;2;37;33;32m\033[38;2;49;43;39m▄\033[48;2;37;31;28m\033[38;2;35;29;25m▄\033[48;2;32;24;21m\033[38;2;31;25;21m▄\033[48;2;37;29;25m\033[38;2;37;30;24m▄\033[48;2;38;28;23m\033[38;2;40;31;25m▄\033[48;2;51;37;29m\033[38;2;50;36;27m▄\033[48;2;60;44;33m\033[38;2;60;43;31m▄\033[48;2;73;52;37m\033[38;2;73;52;36m▄\033[48;2;89;63;44m\033[38;2;90;62;42m▄\033[48;2;103;72;51m\033[38;2;98;66;45m▄\033[48;2;102;70;48m\033[38;2;102;66;44m▄\033[48;2;102;68;45m\033[38;2;103;67;44m▄\033[48;2;97;62;41m\033[38;2;98;62;40m▄\033[48;2;105;68;44m \033[48;2;105;69;43m\033[38;2;111;74;49m▄\033[48;2;98;63;41m\033[38;2;89;57;36m▄\033[48;2;121;83;55m\033[38;2;96;64;43m▄\033[48;2;108;70;44m\033[38;2;101;66;43m▄\033[48;2;104;66;42m\033[38;2;97;62;40m▄\033[48;2;105;67;43m\033[38;2;95;60;38m▄\033[48;2;88;54;35m\033[38;2;77;47;30m▄\033[48;2;66;41;27m\033[38;2;64;40;26m▄\033[48;2;60;38;25m\033[38;2;63;39;27m▄\033[48;2;67;42;29m\033[38;2;66;41;28m▄\033[48;2;77;48;32m\033[38;2;72;45;30m▄\033[48;2;87;54;35m\033[38;2;81;51;34m▄\033[48;2;91;56;37m\033[38;2;91;57;38m▄\033[48;2;94;57;36m\033[38;2;97;60;38m▄\033[48;2;97;58;37m\033[38;2;100;61;39m▄\033[48;2;103;64;43m\033[38;2;106;68;44m▄\033[48;2;99;64;43m\033[38;2;106;69;44m▄\033[48;2;74;51;38m\033[38;2;97;64;43m▄\033[48;2;28;21;17m\033[38;2;62;43;32m▄\033[48;2;2;1;1m\033[38;2;12;9;7m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;17;15;14m\033[38;2;7;7;6m▄\033[48;2;101;84;72m\033[38;2;77;68;61m▄\033[48;2;170;127;96m\033[38;2;159;122;95m▄\033[48;2;185;129;87m\033[38;2;181;126;84m▄\033[48;2;189;130;87m\033[38;2;195;139;94m▄\033[48;2;196;138;94m\033[38;2;199;144;100m▄\033[48;2;200;144;101m\033[38;2;203;151;108m▄\033[48;2;208;157;117m\033[38;2;206;156;115m▄\033[48;2;210;163;124m\033[38;2;204;156;117m▄\033[48;2;207;164;128m\033[38;2;199;154;119m▄\033[48;2;196;156;121m\033[38;2;178;138;107m▄\033[48;2;106;83;85m\033[38;2;88;74;75m▄\033[48;2;171;135;128m\033[38;2;117;96;95m▄\033[48;2;150;109;107m\033[38;2;138;114;111m▄\033[48;2;165;118;125m\033[38;2;147;123;120m▄\033[48;2;180;127;139m\033[38;2;141;117;119m▄\033[48;2;176;123;134m\033[38;2;113;88;90m▄\033[48;2;181;126;130m\033[38;2;153;129;123m▄\033[48;2;155;117;114m\033[38;2;98;83;80m▄\033[48;2;130;111;104m\033[38;2;77;70;67m▄\033[48;2;53;46;45m\033[38;2;41;39;38m▄\033[48;2;42;39;39m\033[38;2;44;43;42m▄\033[48;2;37;35;34m\033[38;2;41;39;39m▄\033[48;2;37;35;34m\033[38;2;61;55;51m▄\033[48;2;65;58;53m\033[38;2;89;78;68m▄\033[48;2;58;51;45m\033[38;2;55;46;40m▄\033[48;2;35;28;24m\033[38;2;46;38;31m▄\033[48;2;36;28;23m\033[38;2;45;35;27m▄\033[48;2;33;26;20m\033[38;2;41;30;23m▄\033[48;2;40;31;24m\033[38;2;42;30;23m▄\033[48;2;48;34;25m\033[38;2;51;35;25m▄\033[48;2;60;41;29m\033[38;2;63;42;29m▄\033[48;2;73;50;34m\033[38;2;80;54;38m▄\033[48;2;91;62;43m\033[38;2;92;63;43m▄\033[48;2;93;62;41m\033[38;2;94;63;41m▄\033[48;2;98;64;43m\033[38;2;97;63;42m▄\033[48;2;105;68;45m\033[38;2;107;70;47m▄\033[48;2;95;59;38m\033[38;2;97;60;38m▄\033[48;2;101;65;42m\033[38;2;103;67;45m▄\033[48;2;111;71;46m\033[38;2;114;74;49m▄\033[48;2;99;64;41m\033[38;2;99;62;41m▄\033[48;2;91;59;39m\033[38;2;91;56;37m▄\033[48;2;96;62;42m\033[38;2;93;59;39m▄\033[48;2;84;52;33m\033[38;2;82;51;33m▄\033[48;2;82;52;33m\033[38;2;78;50;32m▄\033[48;2;79;50;33m\033[38;2;85;56;38m▄\033[48;2;76;49;34m\033[38;2;89;59;40m▄\033[48;2;72;47;32m\033[38;2;85;57;38m▄\033[48;2;70;45;31m\033[38;2;90;59;40m▄\033[48;2;79;51;34m\033[38;2;93;61;41m▄\033[48;2;91;60;40m\033[38;2;103;69;46m▄\033[48;2;101;66;45m\033[38;2;109;74;49m▄\033[48;2;102;66;43m\033[38;2;109;72;47m▄\033[48;2;103;65;42m\033[38;2;108;70;45m▄\033[48;2;102;64;41m\033[38;2;105;68;43m▄\033[48;2;105;68;43m\033[38;2;109;72;46m▄\033[48;2;105;68;44m\033[38;2;110;72;46m▄\033[48;2;89;59;41m\033[38;2;103;67;45m▄\033[48;2;30;21;15m\033[38;2;56;38;27m▄\033[48;2;1;0;0m\033[38;2;5;3;2m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;3;3;3m\033[38;2;1;1;1m▄\033[48;2;60;53;48m\033[38;2;39;35;32m▄\033[48;2;148;116;92m\033[38;2;131;106;87m▄\033[48;2;181;127;86m\033[38;2;180;129;92m▄\033[48;2;196;140;96m\033[38;2;196;141;97m▄\033[48;2;202;148;105m\033[38;2;201;148;104m▄\033[48;2;204;153;111m\033[38;2;203;151;109m▄\033[48;2;204;155;113m\033[38;2;204;156;116m▄\033[48;2;200;153;114m\033[38;2;194;147;108m▄\033[48;2;185;141;106m\033[38;2;173;129;95m▄\033[48;2;165;129;102m\033[38;2;156;118;93m▄\033[48;2;68;59;59m\033[38;2;120;104;93m▄\033[48;2;64;54;57m\033[38;2;52;46;47m▄\033[48;2;84;74;77m\033[38;2;55;51;52m▄\033[48;2;115;105;107m\033[38;2;59;55;57m▄\033[48;2;108;99;102m\033[38;2;51;49;50m▄\033[48;2;73;64;65m\033[38;2;40;37;38m▄\033[48;2;73;63;62m\033[38;2;34;31;29m▄\033[48;2;45;40;40m\033[38;2;31;28;27m▄\033[48;2;34;31;30m\033[38;2;33;30;28m▄\033[48;2;36;35;33m\033[38;2;54;49;45m▄\033[48;2;43;40;39m\033[38;2;95;85;78m▄\033[48;2;70;61;58m\033[38;2;136;120;107m▄\033[48;2;112;98;86m\033[38;2;144;125;109m▄\033[48;2;108;92;78m\033[38;2;136;118;101m▄\033[48;2;72;58;48m\033[38;2;120;102;87m▄\033[48;2;70;58;46m\033[38;2;103;87;72m▄\033[48;2;63;49;37m\033[38;2;87;68;52m▄\033[48;2;56;39;28m\033[38;2;72;51;37m▄\033[48;2;45;30;22m\033[38;2;56;37;27m▄\033[48;2;54;36;25m\033[38;2;60;39;27m▄\033[48;2;66;43;30m\033[38;2;71;45;31m▄\033[48;2;83;55;37m\033[38;2;84;54;36m▄\033[48;2;94;64;43m\033[38;2;95;62;42m▄\033[48;2;99;66;43m\033[38;2;99;64;42m▄\033[48;2;94;59;37m\033[38;2;95;59;37m▄\033[48;2;102;66;43m\033[38;2;98;62;39m▄\033[48;2;99;62;39m\033[38;2;96;61;39m▄\033[48;2;102;66;44m\033[38;2;94;60;39m▄\033[48;2;104;66;42m\033[38;2;90;56;36m▄\033[48;2;89;55;35m\033[38;2;82;50;32m▄\033[48;2;78;48;31m\033[38;2;77;48;32m▄\033[48;2;86;55;36m\033[38;2;80;50;33m▄\033[48;2;87;55;36m\033[38;2;93;61;42m▄\033[48;2;85;55;36m\033[38;2;94;62;42m▄\033[48;2;93;63;43m\033[38;2;103;70;48m▄\033[48;2;101;69;47m\033[38;2;109;75;50m▄\033[48;2;96;65;43m\033[38;2;111;76;50m▄\033[48;2;104;70;48m\033[38;2;115;80;54m▄\033[48;2;106;71;47m\033[38;2;123;87;59m▄\033[48;2;111;76;51m\033[38;2;123;87;59m▄\033[48;2;118;82;56m\033[38;2;127;91;62m▄\033[48;2;121;83;56m\033[38;2;132;95;65m▄\033[48;2;119;80;52m\033[38;2;129;88;58m▄\033[48;2;114;77;49m\033[38;2;123;84;53m▄\033[48;2;112;76;49m\033[38;2;115;78;51m▄\033[48;2;112;74;48m\033[38;2;110;74;48m▄\033[48;2;109;72;47m\033[38;2;108;72;46m▄\033[48;2;83;56;37m\033[38;2;100;68;44m▄\033[48;2;19;12;8m\033[38;2;49;33;21m▄\033[48;2;0;0;0m\033[38;2;2;0;0m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;18;16;15m\033[38;2;9;8;7m▄\033[48;2;106;91;80m\033[38;2;87;77;69m▄\033[48;2;177;133;99m\033[38;2;172;134;104m▄\033[48;2;196;143;101m\033[38;2;195;143;100m▄\033[48;2;204;151;107m\033[38;2;203;153;110m▄\033[48;2;202;150;109m\033[38;2;202;153;113m▄\033[48;2;199;150;110m\033[38;2;192;145;105m▄\033[48;2;181;135;98m\033[38;2;179;131;94m▄\033[48;2;162;118;85m\033[38;2;175;127;92m▄\033[48;2;149;108;79m\033[38;2;160;115;85m▄\033[48;2;159;130;108m\033[38;2;161;125;98m▄\033[48;2;128;113;101m\033[38;2;165;134;111m▄\033[48;2;69;60;57m\033[38;2;163;135;115m▄\033[48;2;53;46;43m\033[38;2;172;149;133m▄\033[48;2;45;39;37m\033[38;2;172;153;140m▄\033[48;2;43;38;36m\033[38;2;173;155;142m▄\033[48;2;44;39;36m\033[38;2;177;158;143m▄\033[48;2;60;54;49m\033[38;2;181;159;141m▄\033[48;2;106;94;86m\033[38;2;187;163;143m▄\033[48;2;144;127;115m\033[38;2;190;167;149m▄\033[48;2;168;148;132m\033[38;2;193;174;154m▄\033[48;2;174;155;138m\033[38;2;191;173;155m▄\033[48;2;158;140;123m\033[38;2;178;161;143m▄\033[48;2;157;139;120m\033[38;2;174;159;141m▄\033[48;2;157;140;124m\033[38;2;174;160;144m▄\033[48;2;136;118;102m\033[38;2;158;141;125m▄\033[48;2;112;91;71m\033[38;2;137;113;92m▄\033[48;2;89;65;45m\033[38;2;107;79;58m▄\033[48;2;74;49;32m\033[38;2;93;63;42m▄\033[48;2;71;46;31m\033[38;2;87;56;35m▄\033[48;2;82;53;35m\033[38;2;91;58;37m▄\033[48;2;88;57;37m\033[38;2;95;61;39m▄\033[48;2;94;60;40m\033[38;2;94;59;38m▄\033[48;2;101;65;43m\033[38;2;103;67;46m▄\033[48;2;100;63;42m\033[38;2;104;66;44m▄\033[48;2;100;64;41m\033[38;2;102;66;43m▄\033[48;2;94;59;37m\033[38;2;86;53;33m▄\033[48;2;85;53;33m\033[38;2;81;50;32m▄\033[48;2;80;48;31m\033[38;2;80;49;33m▄\033[48;2;86;54;36m\033[38;2;85;52;34m▄\033[48;2;82;51;33m\033[38;2;89;56;37m▄\033[48;2;84;55;36m\033[38;2;99;66;45m▄\033[48;2;101;68;46m\033[38;2;107;73;49m▄\033[48;2;109;76;52m\033[38;2;123;87;59m▄\033[48;2;118;83;56m\033[38;2;126;88;60m▄\033[48;2;119;82;56m\033[38;2;131;94;67m▄\033[48;2;118;83;55m\033[38;2;127;89;62m▄\033[48;2;126;91;63m\033[38;2;131;94;65m▄\033[48;2;139;101;69m\033[38;2;132;95;63m▄\033[48;2;143;104;71m\033[38;2;139;101;70m▄\033[48;2;136;97;66m\033[38;2;140;102;72m▄\033[48;2;135;97;67m\033[38;2;136;98;69m▄\033[48;2;137;98;67m\033[38;2;134;97;66m▄\033[48;2;125;86;55m\033[38;2;130;92;62m▄\033[48;2;116;79;51m\033[38;2;121;84;55m▄\033[48;2;113;78;51m\033[38;2;116;80;52m▄\033[48;2;115;81;54m\033[38;2;117;83;56m▄\033[48;2;112;79;53m\033[38;2;117;85;60m▄\033[48;2;68;48;32m\033[38;2;79;58;42m▄\033[48;2;4;2;0m\033[38;2;7;4;2m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;7;6;6m\033[38;2;7;7;6m▄\033[48;2;77;68;62m\033[38;2;80;71;64m▄\033[48;2;168;130;101m\033[38;2;169;131;100m▄\033[48;2;194;144;102m\033[38;2;193;144;103m▄\033[48;2;203;154;111m\033[38;2;204;155;114m▄\033[48;2;203;156;115m\033[38;2;204;157;115m▄\033[48;2;196;148;107m\033[38;2;198;150;109m▄\033[48;2;192;143;102m\033[38;2;197;149;109m▄\033[48;2;187;139;101m\033[38;2;196;150;112m▄\033[48;2;182;138;105m\033[38;2;201;162;130m▄\033[48;2;186;150;120m\033[38;2;199;165;136m▄\033[48;2;173;137;110m\033[38;2;187;151;121m▄\033[48;2;170;136;110m\033[38;2;191;158;130m▄\033[48;2;202;175;152m\033[38;2;216;190;169m▄\033[48;2;215;192;171m\033[38;2;223;200;179m▄\033[48;2;210;188;169m\033[38;2;214;189;167m▄\033[48;2;209;184;164m\033[38;2;207;177;154m▄\033[48;2;205;177;153m\033[38;2;208;178;152m▄\033[48;2;204;179;156m\033[38;2;206;178;153m▄\033[48;2;199;174;152m\033[38;2;202;173;146m▄\033[48;2;199;178;156m\033[38;2;206;183;160m▄\033[48;2;205;191;174m\033[38;2;212;197;179m▄\033[48;2;195;182;161m\033[38;2;206;191;171m▄\033[48;2;187;173;154m\033[38;2;201;187;170m▄\033[48;2;189;175;160m\033[38;2;199;186;169m▄\033[48;2;173;156;139m\033[38;2;183;162;142m▄\033[48;2;152;128;107m\033[38;2;169;144;122m▄\033[48;2;127;97;72m\033[38;2;148;117;90m▄\033[48;2;113;79;53m\033[38;2;134;100;72m▄\033[48;2;103;67;43m\033[38;2;120;83;56m▄\033[48;2;100;64;39m\033[38;2;114;75;47m▄\033[48;2;101;64;40m\033[38;2;112;73;46m▄\033[48;2;101;63;40m\033[38;2;106;68;42m▄\033[48;2;105;68;46m\033[38;2;101;64;42m▄\033[48;2;104;67;45m\033[38;2;98;62;40m▄\033[48;2;93;57;35m\033[38;2;87;53;34m▄\033[48;2;81;50;31m\033[38;2;76;46;28m▄\033[48;2;74;44;29m\033[38;2;70;42;28m▄\033[48;2;79;50;33m\033[38;2;82;53;35m▄\033[48;2;92;59;39m\033[38;2;94;63;41m▄\033[48;2;97;64;43m\033[38;2;107;71;49m▄\033[48;2;106;71;47m\033[38;2;117;80;54m▄\033[48;2;120;84;57m\033[38;2;125;87;58m▄\033[48;2;138;100;69m\033[38;2;142;103;71m▄\033[48;2;140;102;70m\033[38;2;151;112;80m▄\033[48;2;149;109;77m\033[38;2;157;118;84m▄\033[48;2;151;110;77m\033[38;2;157;117;83m▄\033[48;2;140;101;70m\033[38;2;158;119;86m▄\033[48;2;139;101;70m\033[38;2;151;113;80m▄\033[48;2;147;110;78m\033[38;2;150;111;79m▄\033[48;2;151;113;81m\033[38;2;156;116;83m▄\033[48;2;148;109;76m\033[38;2;152;110;76m▄\033[48;2;139;100;68m\033[38;2;136;95;62m▄\033[48;2;130;91;61m\033[38;2;128;88;58m▄\033[48;2;124;86;57m\033[38;2;122;84;55m▄\033[48;2;120;84;56m\033[38;2;119;83;56m▄\033[48;2;120;85;58m\033[38;2;119;84;57m▄\033[48;2;116;83;56m \033[48;2;87;62;43m\033[38;2;96;68;47m▄\033[48;2;16;10;7m\033[38;2;30;19;12m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;9;9;8m \033[48;2;90;78;69m\033[38;2;91;81;73m▄\033[48;2;173;133;101m\033[38;2;175;140;111m▄\033[48;2;193;144;102m\033[38;2;194;147;107m▄\033[48;2;202;155;114m\033[38;2;203;157;117m▄\033[48;2;202;155;114m\033[38;2;204;160;120m▄\033[48;2;201;153;113m\033[38;2;205;163;124m▄\033[48;2;202;157;118m\033[38;2;206;166;131m▄\033[48;2;206;165;130m\033[38;2;213;182;154m▄\033[48;2;212;180;153m\033[38;2;218;195;172m▄\033[48;2;207;176;149m\033[38;2;214;191;167m▄\033[48;2;202;169;139m\033[38;2;211;184;159m▄\033[48;2;209;180;155m\033[38;2;219;197;176m▄\033[48;2;225;203;185m\033[38;2;229;211;195m▄\033[48;2;225;201;181m\033[38;2;225;203;183m▄\033[48;2;216;188;163m\033[38;2;217;190;164m▄\033[48;2;212;182;157m\033[38;2;213;183;158m▄\033[48;2;212;182;156m\033[38;2;213;184;160m▄\033[48;2;208;181;154m\033[38;2;211;189;165m▄\033[48;2;209;183;157m\033[38;2;212;191;169m▄\033[48;2;212;189;166m\033[38;2;211;190;169m▄\033[48;2;207;187;166m\033[38;2;209;190;168m▄\033[48;2;218;203;184m\033[38;2;229;216;199m▄\033[48;2;212;197;179m\033[38;2;220;205;186m▄\033[48;2;206;189;169m\033[38;2;215;197;174m▄\033[48;2;194;170;144m\033[38;2;207;184;159m▄\033[48;2;184;158;131m\033[38;2;198;176;152m▄\033[48;2;169;140;114m\033[38;2;187;164;140m▄\033[48;2;151;118;90m\033[38;2;165;138;112m▄\033[48;2;139;102;72m\033[38;2;160;127;100m▄\033[48;2;131;91;60m\033[38;2;150;111;81m▄\033[48;2;120;79;51m\033[38;2;132;93;62m▄\033[48;2;112;72;45m\033[38;2;121;80;52m▄\033[48;2;106;67;44m\033[38;2;117;77;52m▄\033[48;2;94;58;36m\033[38;2;100;62;39m▄\033[48;2;83;51;32m\033[38;2;85;52;33m▄\033[48;2;72;42;27m\033[38;2;71;41;26m▄\033[48;2;70;42;28m\033[38;2;76;46;30m▄\033[48;2;91;60;41m\033[38;2;101;67;45m▄\033[48;2;97;64;43m\033[38;2;103;68;44m▄\033[48;2;118;80;55m\033[38;2;132;92;63m▄\033[48;2;125;86;58m\033[38;2;141;100;69m▄\033[48;2;124;85;57m\033[38;2;137;97;67m▄\033[48;2;147;108;76m\033[38;2;155;115;81m▄\033[48;2;161;122;89m\033[38;2;162;123;89m▄\033[48;2;166;128;93m\033[38;2;162;126;93m▄\033[48;2;162;123;89m\033[38;2;163;126;93m▄\033[48;2;162;123;88m\033[38;2;167;131;96m▄\033[48;2;153;114;80m\033[38;2;165;126;90m▄\033[48;2;144;106;74m\033[38;2;152;113;79m▄\033[48;2;150;112;80m\033[38;2;149;112;79m▄\033[48;2;153;114;79m\033[38;2;150;112;78m▄\033[48;2;141;100;67m\033[38;2;142;102;71m▄\033[48;2;128;87;57m\033[38;2;130;90;60m▄\033[48;2;118;80;51m\033[38;2;114;78;49m▄\033[48;2;114;79;52m\033[38;2;109;74;48m▄\033[48;2;113;80;54m\033[38;2;102;69;46m▄\033[48;2;107;76;51m\033[38;2;90;61;41m▄\033[48;2;87;61;42m\033[38;2;60;41;28m▄\033[48;2;27;17;11m\033[38;2;8;6;3m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;8;8;8m\033[38;2;14;13;12m▄\033[48;2;88;81;74m\033[38;2;101;91;83m▄\033[48;2;179;148;124m\033[38;2;183;153;125m▄\033[48;2;195;153;113m\033[38;2;198;156;118m▄\033[48;2;203;160;118m\033[38;2;204;161;121m▄\033[48;2;207;165;127m\033[38;2;212;176;141m▄\033[48;2;210;174;139m\033[38;2;217;191;165m▄\033[48;2;213;184;158m\033[38;2;220;202;184m▄\033[48;2;217;197;178m\033[38;2;224;208;193m▄\033[48;2;220;203;184m\033[38;2;220;202;185m▄\033[48;2;217;196;176m\033[38;2;219;200;180m▄\033[48;2;218;195;173m\033[38;2;218;197;176m▄\033[48;2;225;207;190m\033[38;2;228;212;197m▄\033[48;2;235;221;207m\033[38;2;235;224;212m▄\033[48;2;227;206;186m\033[38;2;228;209;190m▄\033[48;2;218;190;164m\033[38;2;219;189;162m▄\033[48;2;213;182;155m\033[38;2;213;177;146m▄\033[48;2;214;185;162m\033[38;2;214;185;161m▄\033[48;2;214;192;171m\033[38;2;213;191;169m▄\033[48;2;214;194;173m\033[38;2;208;186;165m▄\033[48;2;203;182;159m\033[38;2;205;185;163m▄\033[48;2;228;213;196m\033[38;2;233;218;201m▄\033[48;2;232;216;198m\033[38;2;234;222;205m▄\033[48;2;229;218;201m\033[38;2;231;222;206m▄\033[48;2;222;207;186m\033[38;2;224;210;192m▄\033[48;2;215;197;177m\033[38;2;218;200;180m▄\033[48;2;205;187;167m\033[38;2;211;192;172m▄\033[48;2;195;175;155m\033[38;2;203;186;168m▄\033[48;2;181;161;140m\033[38;2;197;181;163m▄\033[48;2;175;151;128m\033[38;2;193;176;159m▄\033[48;2;165;136;109m\033[38;2;181;160;139m▄\033[48;2;148;113;83m\033[38;2;165;133;107m▄\033[48;2;130;91;63m\033[38;2;140;104;78m▄\033[48;2;118;80;55m\033[38;2;102;66;43m▄\033[48;2;102;65;42m\033[38;2;93;59;39m▄\033[48;2;78;46;28m\033[38;2;72;41;26m▄\033[48;2;71;40;25m\033[38;2;79;48;30m▄\033[48;2;83;51;33m\033[38;2;102;67;44m▄\033[48;2;107;71;46m\033[38;2;123;82;53m▄\033[48;2;113;75;49m\033[38;2;127;86;56m▄\033[48;2;141;101;68m\033[38;2;150;110;76m▄\033[48;2;154;111;76m\033[38;2;161;119;84m▄\033[48;2;149;107;73m\033[38;2;163;119;84m▄\033[48;2;153;112;77m\033[38;2;157;114;79m▄\033[48;2;163;125;90m\033[38;2;165;126;91m▄\033[48;2;162;125;91m\033[38;2;169;132;97m▄\033[48;2;167;132;99m\033[38;2;168;132;98m▄\033[48;2;169;133;99m\033[38;2;163;127;94m▄\033[48;2;169;131;94m\033[38;2;168;132;97m▄\033[48;2;159;119;83m\033[38;2;163;124;89m▄\033[48;2;154;116;81m\033[38;2;155;117;81m▄\033[48;2;152;113;78m\033[38;2;150;111;76m▄\033[48;2;144;104;73m\033[38;2;138;97;66m▄\033[48;2;126;87;57m\033[38;2;121;83;55m▄\033[48;2;112;75;48m\033[38;2;105;70;45m▄\033[48;2;103;69;44m\033[38;2;91;59;38m▄\033[48;2;87;57;36m\033[38;2;81;52;34m▄\033[48;2;67;42;28m\033[38;2;63;40;27m▄\033[48;2;33;21;15m\033[38;2;21;12;9m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;1;0;0m▄\033[48;2;27;25;24m\033[38;2;33;30;28m▄\033[48;2;122;107;94m\033[38;2;131;115;101m▄\033[48;2;190;158;128m\033[38;2;192;161;134m▄\033[48;2;202;162;127m\033[38;2;202;168;135m▄\033[48;2;208;172;138m\033[38;2;210;183;158m▄\033[48;2;216;190;165m\033[38;2;218;202;186m▄\033[48;2;221;204;187m\033[38;2;221;209;197m▄\033[48;2;222;208;193m\033[38;2;222;209;196m▄\033[48;2;222;206;191m\033[38;2;220;204;190m▄\033[48;2;217;199;181m\033[38;2;219;203;186m▄\033[48;2;220;199;179m\033[38;2;219;200;180m▄\033[48;2;219;197;179m\033[38;2;223;205;189m▄\033[48;2;231;219;206m\033[38;2;230;218;206m▄\033[48;2;233;221;209m\033[38;2;229;216;205m▄\033[48;2;223;204;186m\033[38;2;222;201;184m▄\033[48;2;213;179;150m\033[38;2;216;188;163m▄\033[48;2;212;176;145m\033[38;2;213;185;160m▄\033[48;2;213;185;159m\033[38;2;214;189;165m▄\033[48;2;212;190;167m\033[38;2;210;186;163m▄\033[48;2;204;179;155m\033[38;2;212;188;164m▄\033[48;2;220;202;184m\033[38;2;227;212;196m▄\033[48;2;234;223;207m\033[38;2;234;224;211m▄\033[48;2;234;224;208m\033[38;2;233;224;208m▄\033[48;2;230;219;204m\033[38;2;230;218;201m▄\033[48;2;223;208;187m\033[38;2;227;212;192m▄\033[48;2;220;202;179m\033[38;2;225;210;191m▄\033[48;2;215;197;175m\033[38;2;216;201;183m▄\033[48;2;206;191;173m\033[38;2;210;198;181m▄\033[48;2;200;186;169m\033[38;2;203;191;174m▄\033[48;2;197;184;167m\033[38;2;200;187;170m▄\033[48;2;182;160;140m\033[38;2;184;162;143m▄\033[48;2;162;128;101m\033[38;2;164;133;109m▄\033[48;2;129;92;66m\033[38;2;130;93;67m▄\033[48;2;99;62;40m\033[38;2;103;65;40m▄\033[48;2;77;45;29m\033[38;2;81;49;31m▄\033[48;2;97;61;39m\033[38;2;112;73;48m▄\033[48;2;92;58;37m\033[38;2;105;68;43m▄\033[48;2;117;81;56m\033[38;2;126;89;63m▄\033[48;2;144;100;68m\033[38;2;159;117;83m▄\033[48;2;139;95;62m\033[38;2;153;109;75m▄\033[48;2;149;108;75m\033[38;2;152;111;78m▄\033[48;2;166;124;90m\033[38;2;174;135;100m▄\033[48;2;173;131;94m\033[38;2;178;137;100m▄\033[48;2;161;118;83m\033[38;2;165;123;86m▄\033[48;2;167;128;92m\033[38;2;169;129;93m▄\033[48;2;178;143;107m\033[38;2;179;142;107m▄\033[48;2;181;146;111m\033[38;2;180;145;109m▄\033[48;2;174;138;103m\033[38;2;167;129;94m▄\033[48;2;166;130;95m\033[38;2;162;125;91m▄\033[48;2;163;125;90m\033[38;2;160;121;87m▄\033[48;2;153;113;78m\033[38;2;150;110;75m▄\033[48;2;143;103;69m\033[38;2;133;95;64m▄\033[48;2;128;88;57m\033[38;2;113;75;50m▄\033[48;2;111;74;48m\033[38;2;99;64;42m▄\033[48;2;97;63;41m\033[38;2;88;57;38m▄\033[48;2;85;55;35m\033[38;2;88;60;40m▄\033[48;2;81;53;35m\033[38;2;86;58;39m▄\033[48;2;70;46;32m\033[38;2;73;49;34m▄\033[48;2;25;15;10m\033[38;2;32;21;14m▄\033[48;2;0;0;0m\033[38;2;1;0;0m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;1;0;0m▄\033[48;2;30;28;27m\033[38;2;31;30;29m▄\033[48;2;131;121;110m\033[38;2;130;119;108m▄\033[48;2;194;168;144m\033[38;2;195;168;144m▄\033[48;2;202;170;142m\033[38;2;199;164;134m▄\033[48;2;208;184;165m\033[38;2;205;183;165m▄\033[48;2;220;209;199m\033[38;2;219;210;201m▄\033[48;2;222;212;201m\033[38;2;223;213;204m▄\033[48;2;222;211;199m\033[38;2;224;214;205m▄\033[48;2;221;208;195m\033[38;2;222;211;200m▄\033[48;2;220;205;190m\033[38;2;218;204;190m▄\033[48;2;220;201;182m\033[38;2;219;203;184m▄\033[48;2;226;209;193m\033[38;2;225;207;189m▄\033[48;2;227;213;199m\033[38;2;225;208;191m▄\033[48;2;227;211;196m\033[38;2;223;205;186m▄\033[48;2;224;205;188m\033[38;2;221;198;177m▄\033[48;2;218;194;173m\033[38;2;217;193;172m▄\033[48;2;218;197;177m\033[38;2;220;202;185m▄\033[48;2;216;196;175m\033[38;2;217;200;182m▄\033[48;2;214;192;170m\033[38;2;216;197;176m▄\033[48;2;218;198;177m\033[38;2;217;200;182m▄\033[48;2;231;218;205m\033[38;2;229;217;203m▄\033[48;2;235;225;211m\033[38;2;235;225;210m▄\033[48;2;232;223;208m\033[38;2;234;225;208m▄\033[48;2;230;217;199m\033[38;2;229;214;195m▄\033[48;2;229;215;195m\033[38;2;231;218;201m▄\033[48;2;231;220;205m\033[38;2;230;221;207m▄\033[48;2;221;211;196m\033[38;2;225;217;202m▄\033[48;2;215;205;190m\033[38;2;217;208;194m▄\033[48;2;210;200;184m\033[38;2;213;204;188m▄\033[48;2;202;190;174m\033[38;2;204;193;177m▄\033[48;2;189;173;155m\033[38;2;189;173;156m▄\033[48;2;171;149;128m\033[38;2;170;149;129m▄\033[48;2;143;109;84m\033[38;2;153;118;93m▄\033[48;2;112;71;46m\033[38;2;120;77;49m▄\033[48;2;101;65;42m\033[38;2;127;85;55m▄\033[48;2;116;77;51m\033[38;2;136;96;65m▄\033[48;2;134;92;62m\033[38;2;149;107;75m▄\033[48;2;146;108;78m\033[38;2;159;121;89m▄\033[48;2;175;134;99m\033[38;2;181;141;105m▄\033[48;2;173;129;91m\033[38;2;185;140;101m▄\033[48;2;171;129;91m\033[38;2;180;138;97m▄\033[48;2;173;133;97m\033[38;2;176;136;100m▄\033[48;2;181;140;104m\033[38;2;175;133;98m▄\033[48;2;175;134;97m\033[38;2;175;134;98m▄\033[48;2;167;126;92m\033[38;2;165;125;90m▄\033[48;2;174;137;103m\033[38;2;172;135;101m▄\033[48;2;175;140;106m\033[38;2;180;144;109m▄\033[48;2;167;128;94m\033[38;2;163;123;89m▄\033[48;2;160;122;89m\033[38;2;156;119;87m▄\033[48;2;156;118;85m\033[38;2;156;119;87m▄\033[48;2;142;103;71m\033[38;2;140;102;71m▄\033[48;2;124;86;58m\033[38;2;115;78;52m▄\033[48;2;98;64;42m\033[38;2;93;60;40m▄\033[48;2;88;57;37m\033[38;2;89;59;39m▄\033[48;2;88;59;40m\033[38;2;92;63;43m▄\033[48;2;91;62;42m\033[38;2;94;64;44m▄\033[48;2;90;62;42m\033[38;2;96;67;46m▄\033[48;2;79;53;37m\033[38;2;86;59;41m▄\033[48;2;48;32;21m\033[38;2;63;42;30m▄\033[48;2;8;5;3m\033[38;2;35;22;16m▄\033[48;2;0;0;0m\033[38;2;11;6;4m▄\033[48;2;0;0;0m\033[38;2;1;0;0m▄\033[48;2;0;0;0m \033[48;2;0;0;0m \033[48;2;0;0;0m\033[38;2;2;1;1m▄\033[0m"\
                    "\033[0m\033[u\033[B\033[s\033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;0;0;0m \033[0m\033[48;2;2;2;2m \033[0m\033[48;2;49;47;44m \033[0m\033[48;2;147;133;120m \033[0m\033[48;2;192;164;138m \033[0m\033[48;2;197;158;122m \033[0m\033[48;2;203;174;150m \033[0m\033[48;2;213;200;190m \033[0m\033[48;2;223;214;206m \033[0m\033[48;2;226;217;209m \033[0m\033[48;2;220;207;197m \033[0m\033[48;2;217;203;189m \033[0m\033[48;2;222;207;192m \033[0m\033[48;2;225;207;190m \033[0m\033[48;2;222;203;185m \033[0m\033[48;2;220;199;179m \033[0m\033[48;2;217;190;167m \033[0m\033[48;2;213;189;168m \033[0m\033[48;2;217;198;180m \033[0m\033[48;2;216;198;180m \033[0m\033[48;2;216;198;181m \033[0m\033[48;2;217;202;186m \033[0m\033[48;2;230;219;204m \033[0m\033[48;2;233;222;205m \033[0m\033[48;2;234;219;199m \033[0m\033[48;2;231;214;194m \033[0m\033[48;2;233;222;205m \033[0m\033[48;2;229;220;206m \033[0m\033[48;2;224;215;201m \033[0m\033[48;2;217;208;194m \033[0m\033[48;2;210;200;184m \033[0m\033[48;2;202;189;172m \033[0m\033[48;2;182;162;141m \033[0m\033[48;2;163;134;108m \033[0m\033[48;2;152;111;81m \033[0m\033[48;2;147;98;63m \033[0m\033[48;2;156;109;72m \033[0m\033[48;2;164;120;83m \033[0m\033[48;2;167;127;94m \033[0m\033[48;2;175;139;108m \033[0m\033[48;2;182;145;113m \033[0m\033[48;2;194;156;119m \033[0m\033[48;2;184;142;103m \033[0m\033[48;2;176;136;100m \033[0m\033[48;2;171;130;95m \033[0m\033[48;2;174;134;98m \033[0m\033[48;2;172;131;96m \033[0m\033[48;2;164;124;89m \033[0m\033[48;2;172;134;99m \033[0m\033[48;2;160;118;85m \033[0m\033[48;2;146;109;78m \033[0m\033[48;2;145;108;76m \033[0m\033[48;2;128;90;62m \033[0m\033[48;2;105;70;48m \033[0m\033[48;2;96;64;44m \033[0m\033[48;2;96;65;45m \033[0m\033[48;2;96;65;44m \033[0m\033[48;2;101;70;48m \033[0m\033[48;2;103;72;51m \033[0m\033[48;2;92;63;44m \033[0m\033[48;2;68;45;34m \033[0m\033[48;2;55;38;28m \033[0m\033[48;2;41;27;19m \033[0m\033[48;2;26;16;11m \033[0m\033[48;2;13;8;6m \033[0m\033[48;2;12;10;8m \033[0m\033[48;2;32;28;24m \033[0m"\
                    "").nop();

                auto clr = 0xFFFFFFFF;
                auto wiki00 = ansi::wrp(wrap::on).jet(bias::left).fgc(clr).add("ANSI escape code\n\n")

                    .nil().add("From Wikipedia, the free encyclopedia\n"
                            "  (Redirected from ANSI CSI)\n\n")

                    .jet(bias::center).itc(true).add("\"ANSI code\" redirects here.\n"
                                                    "For other uses, see ANSI (disambiguation).\n\n")

                    .jet(bias::left).itc(faux).fgc(clr).add("ANSI escape sequences").nil()
                    .add(" are a standard for ").fgc(clr).add("in-band signaling").nil()
                    .add(" to control the cursor location, color, and other options on video ")
                    .fgc(clr).add("text terminals").nil().add(" and ")
                    .fgc(clr).add("terminal emulators").nil().add(". Certain sequences of ")
                    .fgc(clr).add("bytes").nil().add(", most starting with ")
                    .fgc(clr).add("Esc").nil().add(" and '[', are embedded into the text, "
                    "which the terminal looks for and interprets as commands, not as ")
                    .fgc(clr).add("character codes").nil().add(".\n");

                auto wiki01 = ansi::wrp(wrap::on).jet(bias::left).add("\n\n\n"
                    "ANSI sequences were introduced in the 1970s to replace vendor-specific sequences "
                    "and became widespread in the computer equipment market by the early 1980s. "
                    "They were used in development, scientific and commercial applications and later by "
                    "the nascent ").fgc(clr).add("bulletin board systems").nil()
                    .add(" to offer improved displays compared to earlier systems lacking cursor movement, "
                    "a primary reason they became a standard adopted by all manufacturers.\n\n"

                    "Although hardware text terminals have become increasingly rare in the 21st century, "
                    "the relevance of the ANSI standard persists because most terminal emulators interpret "
                    "at least some of the ANSI escape sequences in output text. A notable exception was ")
                    .fgc(clr).add("DOS").nil().add(" and older versions of the ")
                    .fgc(clr).add("Win32 console").nil().add(" of ")
                    .fgc(clr).add("Microsoft Windows").nil().add(".\n");

            #pragma endregion

            auto truecolor = text{};
            truecolor += wiki00;
            truecolor += r_grut00;
            truecolor += r_grut01;
            truecolor += r_grut02;
            truecolor += r_grut03;
            truecolor += wiki01;

            auto window = ui::cake::ctor();
            window->plugin<pro::focus>()
                  ->plugin<pro::track>()
                  ->plugin<pro::acryl>()
                  ->plugin<pro::cache>()
                  ->invoke([](auto& boss)
                    {
                        boss.keybd.accept(true);
                        closing_on_quit(boss);
                    });
            auto object = window->attach(ui::fork::ctor(axis::Y))
                                ->colors(whitelt, 0xA01f0fc4);
                config.cd("/config/defapp/");
                auto [menu_block, cover, menu_data] = app::shared::custom_menu(config, {});
                auto menu = object->attach(slot::_1, menu_block);
                auto test_stat_area = object->attach(slot::_2, ui::fork::ctor(axis::Y));
                    auto layers = test_stat_area->attach(slot::_1, ui::cake::ctor());
                        auto scroll = layers->attach(ui::rail::ctor())
                                            ->colors(whitelt, reddk);
                                    scroll->attach(ui::post::ctor())
                                          ->upload(truecolor);
                        auto scroll_bars = layers->attach(ui::fork::ctor());
                            auto vt = scroll_bars->attach(slot::_2, ui::grip<axis::Y>::ctor(scroll));
                            auto hz = test_stat_area->attach(slot::_2, ui::grip<axis::X>::ctor(scroll));
            return window;
        };
        auto build_Headless      = [](text cwd, text param, xml::settings& config, text patch)
        {
            auto menu_white = skin::color(tone::menu_white);
            auto cB = menu_white;

            auto window = ui::cake::ctor()
                  ->invoke([&](auto& boss)
                    {
                        closing_on_quit(boss);
                    });
            window->plugin<pro::focus>()
                  ->plugin<pro::track>()
                  ->plugin<pro::acryl>()
                  ->plugin<pro::cache>();
            //auto object = window->attach(ui::fork::ctor(axis::Y))
            //                    ->colors(cB.fgc(), cB.bgc());
            //    auto menu = object->attach(slot::_1, app::shared::custom_menu(faux, {}));
            //    auto layers = object->attach(slot::_2, ui::cake::ctor())
            //                        ->plugin<pro::limit>(dot_11, twod{ 400,200 });
            auto layers = window->attach(ui::cake::ctor())
                                ->colors(cB.fgc(), cB.bgc())
                                ->plugin<pro::limit>(dot_11, twod{ 400,200 });
                    auto scroll = layers->attach(ui::rail::ctor())
                                        ->plugin<pro::limit>(twod{ 10,1 }); // mc crashes when window is too small
                    auto data = param.empty() ? os::get_shell() + " -i"
                                              : param;
                    auto inst = scroll->attach(ui::term::ctor(cwd, data, config))
                                      ->colors(whitelt, blackdk) //todo apply settings
                                      ->invoke([&](auto& boss)
                                      {
                                            //todo unify: Same as in app::term (term.hpp).
                                            boss.SUBMIT(tier::anycast, app::term::events::cmd, cmd)
                                            {
                                                boss.exec_cmd(static_cast<ui::term::commands::ui::commands>(cmd));
                                            };
                                            boss.SUBMIT(tier::anycast, app::term::events::data::in, data)
                                            {
                                                boss.data_in(data);
                                            };
                                            boss.SUBMIT(tier::anycast, app::term::events::data::out, data)
                                            {
                                                boss.data_out(data);
                                            };
                                            //todo add color picker to the menu
                                            boss.SUBMIT(tier::anycast, app::term::events::colors::bg, bg)
                                            {
                                                boss.set_bg_color(bg);
                                            };
                                            boss.SUBMIT(tier::anycast, app::term::events::colors::fg, fg)
                                            {
                                                boss.set_fg_color(fg);
                                            };
                                            boss.SUBMIT(tier::anycast, e2::form::prop::colors::any, clr)
                                            {
                                                auto deed = boss.bell::template protos<tier::anycast>();
                                                     if (deed == e2::form::prop::colors::bg.id) boss.SIGNAL(tier::anycast, app::term::events::colors::bg, clr);
                                                else if (deed == e2::form::prop::colors::fg.id) boss.SIGNAL(tier::anycast, app::term::events::colors::fg, clr);
                                            };
                                            boss.SUBMIT(tier::anycast, e2::form::upon::started, root)
                                            {
                                                boss.start();
                                            };
                                            boss.SUBMIT(tier::anycast, app::term::events::search::forward, gear)
                                            {
                                                boss.search(gear, feed::fwd);
                                            };
                                            boss.SUBMIT(tier::anycast, app::term::events::search::reverse, gear)
                                            {
                                                boss.search(gear, feed::rev);
                                            };
                                      });
                layers->attach(app::shared::scroll_bars(scroll));
            return window;
        };
        auto build_Fone          = [](text cwd, text param, xml::settings& config, text patch)
        {
            auto highlight_color = skin::color(tone::highlight);
            auto c8 = cell{}.bgc(0x00).fgc(highlight_color.bgc());
            auto x8 = cell{ c8 }.alpha(0x00);
            return ui::park::ctor()
                ->branch(ui::snap::tail, ui::snap::tail, ui::item::ctor(DESKTOPIO_MYNAME)
                ->template plugin<pro::fader>(x8, c8, 0ms))
                ->template plugin<pro::notes>(" About Environment ")
                ->invoke([&](auto& boss)
                {
                    auto shadow = ptr::shadow(boss.This());
                    auto data = utf::divide(param, ";");
                    auto type = text{ data.size() > 0 ? data[0] : view{} };
                    auto name = text{ data.size() > 1 ? data[1] : view{} };
                    auto args = text{ data.size() > 2 ? data[2] : view{} };
                    boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::click::left, gear)
                    {
                        //todo revise/unify
                        auto world_ptr = e2::config::whereami.param();
                        SIGNAL_GLOBAL(e2::config::whereami, world_ptr);
                        if (auto boss = shadow.lock())
                        if (world_ptr)
                        {
                            static auto random = 0;
                            random = (random + 2) % 10;
                            auto offset = twod{ random * 2, random };
                            auto viewport = gear.area();
                            gear.slot.coor = viewport.coor + viewport.size / 8 + offset;
                            gear.slot.size = viewport.size * 3 / 4;
                            gear.slot_forced = faux;

                            auto menu_list_ptr = e2::bindings::list::apps.param();
                            auto conf_list_ptr = e2::bindings::list::links.param();
                            world_ptr->SIGNAL(tier::request, e2::bindings::list::apps, menu_list_ptr);
                            world_ptr->SIGNAL(tier::request, e2::bindings::list::links, conf_list_ptr);
                            auto& menu_list = *menu_list_ptr;
                            auto& conf_list = *conf_list_ptr;

                            if (conf_list.contains(name) && !conf_list[name].hidden) // Check for id availability.
                            {
                                auto i = 1;
                                auto test = text{};
                                do   test = name + " (" + std::to_string(++i) + ")";
                                while (conf_list.contains(test) && !conf_list[name].hidden);
                                std::swap(test, name);
                            }
                            auto& m = conf_list[name];
                            m.type = type;
                            m.label = name;
                            m.title = name; // Use the same title as the menu label.
                            m.param = args;
                            m.hidden = true;
                            m.settings = config; //todo it is dangerous
                            menu_list[name];

                            auto current_default = e2::data::changed.param();
                            boss->template riseup<tier::request>(e2::data::changed, current_default); //todo "template" required by gcc (ubuntu 18.04)

                            if (auto gate = boss->parent())
                            {
                                gate->SIGNAL(tier::release, e2::data::changed, name);
                                world_ptr->SIGNAL(tier::release, e2::form::proceed::createby, gear);
                                gate->SIGNAL(tier::release, e2::data::changed, current_default);
                            }
                            gear.dismiss();
                        }
                    };
                });
        };
        auto build_DirectVT      = [](text cwd, text param, xml::settings& config, text patch)
        {
            return ui::dtvt::ctor(cwd, param, patch)
                ->plugin<pro::limit>(dot_11)
                ->plugin<pro::focus>()
                ->invoke([](auto& boss)
                {
                    boss.SUBMIT(tier::anycast, e2::form::upon::started, root)
                    {
                        boss.start();
                    };
                });
        };
        auto build_ANSIVT        = [](text cwd, text param, xml::settings& config, text patch)
        {
            if (param.empty()) log("apps: nothing to run, use 'type=SHELL' to run instance without arguments");

            auto args = os::current_module_file();
            if (args.find(' ') != text::npos) args = "\"" + args + "\"";

            args += " -r term ";
            args += param;

            return build_DirectVT(cwd, args, config, patch);
        };
        auto build_SHELL         = [](text cwd, text param, xml::settings& config, text patch)
        {
            auto args = os::current_module_file();
            if (args.find(' ') != text::npos) args = "\"" + args + "\"";

            args += " -r term ";
            if (param.empty())
            {
                #if defined(_WIN32)
                    args += "cmd";
                #else
                    args += os::get_shell();
                #endif
            }
            else
            {
                #if defined(_WIN32)
                    args += "cmd /c ";
                #else
                    args += os::get_shell() + " -c ";
                #endif
                args += param;
            }

            return build_DirectVT(cwd, args, config, patch);
        };

        app::shared::initialize builder_Strobe       { "strobe"                 , build_Strobe     };
        app::shared::initialize builder_Settings     { "settings"               , build_Settings   };
        app::shared::initialize builder_Empty        { "empty"                  , build_Empty      };
        app::shared::initialize builder_Truecolor    { "truecolor"              , build_Truecolor  };
        app::shared::initialize builder_Headless     { menuitem_t::type_Headless, build_Headless   };
        app::shared::initialize builder_Fone         { menuitem_t::type_Fone    , build_Fone       };
        app::shared::initialize builder_Region       { menuitem_t::type_Region  , build_Region     };
        app::shared::initialize builder_DirectVT     { menuitem_t::type_DirectVT, build_DirectVT   };
        app::shared::initialize builder_ANSIVT       { menuitem_t::type_ANSIVT  , build_ANSIVT     };
        app::shared::initialize builder_SHELL        { menuitem_t::type_SHELL   , build_SHELL      };
    }
}

#endif // NETXS_APPS_HPP