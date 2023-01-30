// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#include "../desktopio/terminal.hpp"
#include "../desktopio/application.hpp"

namespace netxs::events::userland
{
    struct term
    {
        using mime = ansi::clip::mime;

        EVENTPACK( term, netxs::events::userland::root::custom )
        {
            EVENT_XS( cmd    , si32 ),
            GROUP_XS( preview, si32 ),
            GROUP_XS( release, si32 ),
            GROUP_XS( data   , si32 ),
            GROUP_XS( search , input::hids ),

            SUBSET_XS( preview )
            {
                EVENT_XS( align    , si32 ),
                EVENT_XS( wrapln   , si32 ),
                GROUP_XS( selection, si32 ),
                GROUP_XS( colors   , rgba ),

                SUBSET_XS( selection )
                {
                    EVENT_XS( mode, si32 ),
                    EVENT_XS( box , si32 ),
                };
                SUBSET_XS( colors )
                {
                    EVENT_XS( bg, rgba ),
                    EVENT_XS( fg, rgba ),
                };
            };
            SUBSET_XS( release )
            {
                EVENT_XS( align    , si32 ),
                EVENT_XS( wrapln   , si32 ),
                GROUP_XS( selection, si32 ),
                GROUP_XS( colors   , rgba ),

                SUBSET_XS( selection )
                {
                    EVENT_XS( mode, si32 ),
                    EVENT_XS( box , si32 ),
                };
                SUBSET_XS( colors )
                {
                    EVENT_XS( bg, rgba ),
                    EVENT_XS( fg, rgba ),
                };
            };
            SUBSET_XS( data )
            {
                EVENT_XS( in     , view        ),
                EVENT_XS( out    , view        ),
                EVENT_XS( paste  , input::hids ),
                EVENT_XS( copy   , input::hids ),
                EVENT_XS( prnscrn, input::hids ),
            };
            SUBSET_XS( search )
            {
                EVENT_XS( forward, input::hids ),
                EVENT_XS( reverse, input::hids ),
                EVENT_XS( status , si32        ),
            };
        };
    };
}

// term: Terminal Emulator.
namespace netxs::app::term
{
    static constexpr auto defaults = R"==(
<config>
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
</config>
)==";

    using events = netxs::events::userland::term;
    using mime = clip::mime;

    namespace
    {
        using namespace app::shared;
        static auto _update(ui::pads& boss, menu::item& item)
        {
            auto& look = item.views[item.taken];
            if (boss.client)
            {
                auto& item = *boss.client;
                item.SIGNAL(tier::release, e2::data::text,              look.label);
                boss.SIGNAL(tier::preview, e2::form::prop::ui::tooltip, look.notes);
                item.reflow();
            }
        }
        static auto _update_to(ui::pads& boss, menu::item& item, si32 i)
        {
            item.select(i);
            _update(boss, item);
        }
        template<bool AutoUpdate = faux, class P>
        static auto _submit(ui::pads& boss, menu::item& item, P proc)
        {
            if (item.brand == menu::item::Repeat)
            {
                auto& tick = boss.plugins<pro::timer>();
                boss.LISTEN(tier::release, hids::events::mouse::button::down::left, gear)
                {
                    if (item.views.size())
                    {
                        item.taken = (item.taken + 1) % item.views.size();
                        _update(boss, item);
                    }
                    if (gear.capture(boss.id))
                    {
                        proc(boss, item, gear);
                        tick.actify(0, REPEAT_DELAY, [&, proc](auto p)
                        {
                            proc(boss, item, gear);
                            tick.actify(1, REPEAT_RATE, [&, proc](auto d)
                            {
                                proc(boss, item, gear);
                                return true; // Repeat forever.
                            });
                            return faux; // One shot call (first).
                        });
                        gear.dismiss(true);
                    }
                };
                boss.LISTEN(tier::release, hids::events::mouse::button::up::left, gear)
                {
                    tick.pacify();
                    gear.setfree();
                    gear.dismiss(true);
                    if (item.views.size() && item.taken)
                    {
                        item.taken = 0;
                        _update(boss, item);
                    }
                };
                boss.LISTEN(tier::release, e2::form::state::mouse, active)
                {
                    if (!active && tick)
                    {
                        tick.pacify();
                        if (item.views.size() && item.taken)
                        {
                            item.taken = 0;
                            _update(boss, item);
                        }
                    }
                };
            }
            else
            {
                boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                {
                    proc(boss, item, gear);
                    if constexpr (AutoUpdate)
                    {
                        if (item.brand == menu::item::Option) _update(boss, item);
                    }
                    gear.dismiss(true);
                };
            }
        };
    }

    const auto terminal_menu = [](xml::settings& config)
    {
        auto highlight_color = skin::color(tone::highlight);
        auto c3 = highlight_color;
        auto x3 = cell{ c3 }.alpha(0x00);

        config.cd("/config/term/", "/config/defapp/");
        auto menudata = config.list("menu/item");

        using namespace app::shared;

        static auto brand_options = std::unordered_map<text, menu::item::type>
           {{ menu::type::Splitter, menu::item::Splitter },
            { menu::type::Command,  menu::item::Command  },
            { menu::type::Option,   menu::item::Option   },
            { menu::type::Repeat,   menu::item::Repeat   }};

        #define PROC_LIST \
            X(Noop                      ) /* */ \
            X(TerminalQuit              ) /* */ \
            X(TerminalMaximize          ) /* */ \
            X(TerminalRestart           ) /* */ \
            X(TerminalSendKey           ) /* */ \
            X(TerminalWrapMode          ) /* */ \
            X(TerminalAlignMode         ) /* */ \
            X(TerminalOutput            ) /* */ \
            X(TerminalFindNext          ) /* */ \
            X(TerminalFindPrev          ) /* */ \
            X(TerminalUndo              ) /* Undo/Redo for cooked read under win32 */ \
            X(TerminalRedo              ) /* */ \
            X(TerminalPaste             ) /* */ \
            X(TerminalSelectionCopy     ) /* */ \
            X(TerminalSelectionMode     ) /* */ \
            X(TerminalSelectionRect     ) /* Linear/Rectangular */ \
            X(TerminalSelectionClear    ) /* */ \
            X(TerminalViewportPageUp    ) /* */ \
            X(TerminalViewportPageDown  ) /* */ \
            X(TerminalViewportLineUp    ) /* */ \
            X(TerminalViewportLineDown  ) /* */ \
            X(TerminalViewportPageLeft  ) /* */ \
            X(TerminalViewportPageRight ) /* */ \
            X(TerminalViewportCharLeft  ) /* */ \
            X(TerminalViewportCharRight ) /* */ \
            X(TerminalViewportTop       ) /* */ \
            X(TerminalViewportEnd       ) /* */ \
            X(TerminalViewportCopy      ) /* */ \
            X(TerminalLogStart          ) /* */ \
            X(TerminalLogPause          ) /* */ \
            X(TerminalLogStop           ) /* */ \
            X(TerminalLogAbort          ) /* */ \
            X(TerminalLogRestart        ) /* */ \
            X(TerminalVideoRecStart     ) /* */ \
            X(TerminalVideoRecStop      ) /* */ \
            X(TerminalVideoRecPause     ) /* */ \
            X(TerminalVideoRecAbort     ) /* */ \
            X(TerminalVideoRecRestart   ) /* */ \
            X(TerminalVideoPlay         ) /* */ \
            X(TerminalVideoPause        ) /* */ \
            X(TerminalVideoStop         ) /* */ \
            X(TerminalVideoForward      ) /* */ \
            X(TerminalVideoBackward     ) /* */ \
            X(TerminalVideoHome         ) /* */ \
            X(TerminalVideoEnd          ) /* */

        enum func
        {
            #define X(_proc) _proc,
            PROC_LIST
            #undef X
        };

        static const auto route_options = std::unordered_map<text, func>
        {
            #define X(_proc) { #_proc, func::_proc },
            PROC_LIST
            #undef X
        };

        struct disp
        {
            using preview = app::term::events::preview;
            using release = app::term::events::release;

            static void Noop(ui::pads& boss, menu::item& item) { }
            static void TerminalWrapMode(ui::pads& boss, menu::item& item)
            {
                item.reindex([](auto& utf8){ return xml::take<bool>(utf8).value() ? wrap::on : wrap::off; });
                _submit(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, preview::wrapln, item.views[item.taken].value);
                });
                boss.LISTEN(tier::anycast, release::wrapln, wrapln)
                {
                    _update_to(boss, item, wrapln);
                };
            }
            static void TerminalAlignMode(ui::pads& boss, menu::item& item)
            {
                item.reindex([](auto& utf8){ return netxs::get_or(xml::options::align, utf8, bias::left); });
                _submit(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, preview::align, item.views[item.taken].value);
                });
                boss.LISTEN(tier::anycast, release::align, align)
                {
                    _update_to(boss, item, align);
                };
            }
            static void TerminalFindPrev(ui::pads& boss, menu::item& item)
            {
                item.reindex([](auto& utf8){ return xml::take<bool>(utf8).value(); });
                _submit(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::search::reverse, gear);
                });
                boss.LISTEN(tier::anycast, app::term::events::search::status, status)
                {
                    _update_to(boss, item, (status & 2) ? 1 : 0);
                };
            }
            static void TerminalFindNext(ui::pads& boss, menu::item& item)
            {
                item.reindex([](auto& utf8){ return xml::take<bool>(utf8).value(); });
                _submit(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::search::forward, gear);
                });
                boss.LISTEN(tier::anycast, app::term::events::search::status, status)
                {
                    _update_to(boss, item, (status & 1) ? 1 : 0);
                };
            }
            static void TerminalOutput(ui::pads& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::data::in, view{ item.views[item.taken].param });
                });
            }
            static void TerminalSendKey(ui::pads& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::data::out, view{ item.views[item.taken].param });
                });
            }
            static void TerminalQuit(ui::pads& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.RISEUP(tier::release, e2::form::quit, boss.This());
                });
            }
            static void TerminalMaximize(ui::pads& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.RISEUP(tier::release, e2::form::maximize, gear);
                });
            }
            static void TerminalRestart(ui::pads& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::cmd, ui::term::commands::ui::commands::restart);
                });
            }
            static void TerminalUndo(ui::pads& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::cmd, ui::term::commands::ui::commands::undo);
                });
            }
            static void TerminalRedo(ui::pads& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::cmd, ui::term::commands::ui::commands::redo);
                });
            }
            static void TerminalPaste(ui::pads& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::data::paste, gear);
                });
            }
            static void TerminalSelectionCopy(ui::pads& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::data::copy, gear);
                });
            }
            static void TerminalSelectionMode(ui::pads& boss, menu::item& item)
            {
                item.reindex([](auto& utf8){ return netxs::get_or(xml::options::selmod, utf8, mime::disabled); });
                _submit(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, preview::selection::mode, item.views[item.taken].value);
                });
                boss.LISTEN(tier::anycast, release::selection::mode, mode)
                {
                    _update_to(boss, item, mode);
                };
            }
            static void TerminalSelectionRect(ui::pads& boss, menu::item& item)
            {
                item.reindex([](auto& utf8){ return xml::take<bool>(utf8).value(); });
                _submit(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, preview::selection::box, item.views[item.taken].value);
                });
                boss.LISTEN(tier::anycast, release::selection::box, selbox)
                {
                    _update_to(boss, item, selbox);
                };
            }
            static void TerminalSelectionClear(ui::pads& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::cmd, ui::term::commands::ui::commands::deselect);
                });
            }
            static void TerminalViewportCopy(ui::pads& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    boss.SIGNAL(tier::anycast, app::term::events::data::prnscrn, gear);
                });
            }
            static void TerminalViewportPageUp(ui::pads& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    auto info = e2::form::upon::scroll::bypage::y.param();
                    info.vector = 1;
                    boss.SIGNAL(tier::anycast, e2::form::upon::scroll::bypage::y, info);
                });
            }
            static void TerminalViewportPageDown(ui::pads& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    auto info = e2::form::upon::scroll::bypage::y.param();
                    info.vector = -1;
                    boss.SIGNAL(tier::anycast, e2::form::upon::scroll::bypage::y, info);
                });
            }
            static void TerminalViewportLineUp(ui::pads& boss, menu::item& item)
            {
                item.reindex([](auto& utf8){ auto v = xml::take<si32>(utf8); return v ? v.value() : 1; });
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    auto info = e2::form::upon::scroll::bystep::y.param();
                    info.vector = std::abs(item.views[item.taken].value);
                    boss.SIGNAL(tier::anycast, e2::form::upon::scroll::bystep::y, info);
                });
            }
            static void TerminalViewportLineDown(ui::pads& boss, menu::item& item)
            {
                item.reindex([](auto& utf8){ auto v = xml::take<si32>(utf8); return v ? v.value() : 1; });
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    auto info = e2::form::upon::scroll::bystep::y.param();
                    info.vector = -std::abs(item.views[item.taken].value);
                    boss.SIGNAL(tier::anycast, e2::form::upon::scroll::bystep::y, info);
                });
            }
            static void TerminalViewportTop(ui::pads& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    auto info = e2::form::upon::scroll::to_top::y.param();
                    boss.SIGNAL(tier::anycast, e2::form::upon::scroll::to_top::y, info);
                });
            }
            static void TerminalViewportEnd(ui::pads& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    auto info = e2::form::upon::scroll::to_end::y.param();
                    boss.SIGNAL(tier::anycast, e2::form::upon::scroll::to_end::y, info);
                });
            }
            static void TerminalViewportPageLeft(ui::pads& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    auto info = e2::form::upon::scroll::bypage::x.param();
                    info.vector = 1;
                    boss.SIGNAL(tier::anycast, e2::form::upon::scroll::bypage::x, info);
                });
            }
            static void TerminalViewportPageRight(ui::pads& boss, menu::item& item)
            {
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    auto info = e2::form::upon::scroll::bypage::x.param();
                    info.vector = -1;
                    boss.SIGNAL(tier::anycast, e2::form::upon::scroll::bypage::x, info);
                });
            }
            static void TerminalViewportCharLeft(ui::pads& boss, menu::item& item)
            {
                item.reindex([](auto& utf8){ auto v = xml::take<si32>(utf8); return v ? v.value() : 1; });
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    auto info = e2::form::upon::scroll::bystep::x.param();
                    info.vector = std::abs(item.views[item.taken].value);
                    boss.SIGNAL(tier::anycast, e2::form::upon::scroll::bystep::x, info);
                });
            }
            static void TerminalViewportCharRight(ui::pads& boss, menu::item& item)
            {
                item.reindex([](auto& utf8){ auto v = xml::take<si32>(utf8); return v ? v.value() : 1; });
                _submit<true>(boss, item, [](auto& boss, auto& item, auto& gear)
                {
                    auto info = e2::form::upon::scroll::bystep::x.param();
                    info.vector = -std::abs(item.views[item.taken].value);
                    boss.SIGNAL(tier::anycast, e2::form::upon::scroll::bystep::x, info);
                });
            }
            static void TerminalLogStart(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalLogPause(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalLogStop(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalLogAbort(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalLogRestart(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalVideoRecStart(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalVideoRecStop(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalVideoRecPause(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalVideoRecAbort(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalVideoRecRestart(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalVideoPlay(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalVideoPause(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalVideoStop(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalVideoForward(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalVideoBackward(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalVideoHome(ui::pads& boss, menu::item& item)
            {

            }
            static void TerminalVideoEnd(ui::pads& boss, menu::item& item)
            {

            }
        };
        using submit_proc = std::function<void(ui::pads&, menu::item&)>;
        static const auto proc_map = std::unordered_map<func, submit_proc>
        {
            #define X(_proc) { func::_proc, &disp::_proc },
            PROC_LIST
            #undef X
        };
        #undef PROC_LIST

        auto list = menu::list{};
        auto defs = menu::item::look{};
        for (auto data_ptr : menudata)
        {
            auto item_ptr = std::make_shared<menu::item>();
            auto& data = *data_ptr;
            auto& item = *item_ptr;
            auto route = data.take(menu::attr::route, func::Noop,          route_options);
            item.brand = data.take(menu::attr::brand, menu::item::Command, brand_options);
            defs.notes = data.take(menu::attr::notes, ""s);
            defs.param = data.take(menu::attr::param, ""s);
            defs.onkey = data.take(menu::attr::onkey, ""s);
            item.alive = route != func::Noop && item.brand != menu::item::Splitter;
            for (auto label : data.list(menu::attr::label))
            {
                item.views.push_back(
                {
                    .label = label->value(),
                    .notes = label->take(menu::attr::notes, defs.notes),
                    .param = label->take(menu::attr::param, defs.param),
                    .onkey = label->take(menu::attr::onkey, defs.onkey),
                });
            }
            if (item.views.empty())
            {
                log("term: drop menu item without label");
                continue;
            }
            auto setup = [route](ui::pads& boss, menu::item& item)
            {
                if (item.brand == menu::item::Option)
                {
                    boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                    {
                        item.taken = (item.taken + 1) % item.views.size();
                    };
                }
                auto& initproc = proc_map.find(route)->second;
                initproc(boss, item);
            };
            list.push_back({ item_ptr, setup });
        }
        return menu::create(config, list);
    };

    namespace
    {
        auto build = [](text cwd, text arg, xml::settings& config, text patch)
        {
            auto menu_white = skin::color(tone::menu_white);
            auto cB = menu_white;

            auto window = ui::cake::ctor();
            auto arg_shadow = view{ arg };
            auto term_type = shared::app_class(arg_shadow);
            arg = arg_shadow;
            if (term_type == shared::app_type::normal)
            {
                window->plugin<pro::focus>()
                      ->plugin<pro::track>()
                      ->plugin<pro::acryl>()
                      ->plugin<pro::cache>();
            }
            else window->plugin<pro::focus>(faux);

            auto object = window->attach(ui::fork::ctor(axis::Y))
                                ->colors(cB.fgc(), cB.bgc());
            auto term_stat_area = object->attach(slot::_2, ui::fork::ctor(axis::Y));
                    auto layers = term_stat_area->attach(slot::_1, ui::cake::ctor())
                                                ->plugin<pro::limit>(dot_11, twod{ 400,200 });
                        auto scroll = layers->attach(ui::rail::ctor());
                            auto min_size = twod{ 12,1 }; // mc crashes when window is too small
                            auto max_size = -dot_11;
                            auto forced_clamp = faux;
                            auto forced_resize = true;
                            scroll->plugin<pro::limit>(min_size, max_size, forced_clamp, forced_resize)
                                  ->invoke([](auto& boss)
                                  {
                                    boss.LISTEN(tier::preview, e2::form::prop::window::size, new_size)
                                    {
                                        // Axis x/y (see XTWINOPS):
                                        //   -1 -- preserve
                                        //    0 -- maximize (toggle)
                                        if (new_size == dot_00) // Toggle maximize/restore terminal window (only if it is focused by someone).
                                        {
                                            auto gates = e2::form::state::keybd::enlist.param();
                                            boss.SIGNAL(tier::anycast, e2::form::state::keybd::enlist, gates);
                                            if (gates.size())
                                            if (auto gate_ptr = bell::getref(gates.back()))
                                            {
                                                gate_ptr->SIGNAL(tier::release, e2::form::proceed::onbehalf, [&](auto& gear)
                                                {
                                                    boss.RISEUP(tier::release, e2::form::maximize, gear);
                                                });
                                            }
                                        }
                                        else
                                        {
                                            auto size = boss.size();
                                            new_size = new_size.less(dot_11, size, std::max(dot_11, new_size));
                                            boss.SIGNAL(tier::release, e2::form::prop::window::size, new_size);
                                        }
                                    };
                                  });

                            auto shell = os::env::shell() + " -i";
                            auto inst = scroll->attach(ui::term::ctor(cwd, arg.empty() ? shell : arg, config));

                            auto scroll_bars = layers->attach(ui::fork::ctor());
                                auto vt = scroll_bars->attach(slot::_2, ui::grip<axis::Y>::ctor(scroll));
                                auto hz = term_stat_area->attach(slot::_2, ui::grip_fx2<axis::X>::ctor(scroll))
                                                        ->plugin<pro::limit>(twod{ -1,1 }, twod{ -1,1 })
                                                        ->invoke([&](auto& boss)
                                                        {
                                                            boss.LISTEN(tier::anycast, app::term::events::release::colors::bg, bg)
                                                            {
                                                                boss.color(boss.color().bgc(bg).txt(""));
                                                            };
                                                        });

            auto [slot1, cover, menu_data] = terminal_menu(config);
            auto menu = object->attach(slot::_1, slot1);
            auto menu_id = slot1->id;
            cover->invoke([&](auto& boss)
            {
                boss.colors(cell{ cB }.inv(true).txt("▀"sv).link(menu_id));
                boss.LISTEN(tier::anycast, app::term::events::release::colors::bg, bg)
                {
                    boss.color(boss.color().fgc(bg));
                };
            });

            inst->attach_property(ui::term::events::colors::bg,      app::term::events::release::colors::bg)
                ->attach_property(ui::term::events::colors::fg,      app::term::events::release::colors::fg)
                ->attach_property(ui::term::events::selmod,          app::term::events::release::selection::mode)
                ->attach_property(ui::term::events::selalt,          app::term::events::release::selection::box)
                ->attach_property(ui::term::events::layout::wrapln,  app::term::events::release::wrapln)
                ->attach_property(ui::term::events::layout::align,   app::term::events::release::align)
                ->attach_property(ui::term::events::search::status,  app::term::events::search::status)
                ->invoke([](auto& boss)
                {
                    boss.LISTEN(tier::anycast, app::term::events::cmd, cmd)
                    {
                        boss.exec_cmd(static_cast<ui::term::commands::ui::commands>(cmd));
                    };
                    boss.LISTEN(tier::anycast, app::term::events::data::in, data)
                    {
                        boss.data_in(data);
                    };
                    boss.LISTEN(tier::anycast, app::term::events::data::out, data)
                    {
                        boss.data_out(data);
                    };
                    //todo add color picker to the menu
                    boss.LISTEN(tier::anycast, app::term::events::preview::colors::bg, bg)
                    {
                        boss.set_bg_color(bg);
                    };
                    boss.LISTEN(tier::anycast, app::term::events::preview::colors::fg, fg)
                    {
                        boss.set_fg_color(fg);
                    };
                    boss.LISTEN(tier::anycast, e2::form::prop::colors::any, clr)
                    {
                        auto deed = boss.bell::template protos<tier::anycast>();
                             if (deed == e2::form::prop::colors::bg.id) boss.SIGNAL(tier::anycast, app::term::events::preview::colors::bg, clr);
                        else if (deed == e2::form::prop::colors::fg.id) boss.SIGNAL(tier::anycast, app::term::events::preview::colors::fg, clr);
                    };
                    boss.LISTEN(tier::anycast, app::term::events::preview::selection::mode, selmod)
                    {
                        boss.set_selmod(selmod);
                    };
                    boss.LISTEN(tier::anycast, app::term::events::preview::selection::box, selbox)
                    {
                        boss.set_selalt(selbox);
                    };
                    boss.LISTEN(tier::anycast, app::term::events::preview::wrapln, wrapln)
                    {
                        boss.set_wrapln(wrapln);
                    };
                    boss.LISTEN(tier::anycast, app::term::events::preview::align, align)
                    {
                        boss.set_align(align);
                    };
                    boss.LISTEN(tier::anycast, e2::form::upon::started, root)
                    {
                        boss.start();
                    };
                    boss.LISTEN(tier::anycast, app::term::events::search::forward, gear)
                    {
                        boss.search(gear, feed::fwd);
                    };
                    boss.LISTEN(tier::anycast, app::term::events::search::reverse, gear)
                    {
                        boss.search(gear, feed::rev);
                    };
                    boss.LISTEN(tier::anycast, app::term::events::data::paste, gear)
                    {
                        boss.paste(gear);
                    };
                    boss.LISTEN(tier::anycast, app::term::events::data::copy, gear)
                    {
                        boss.copy(gear);
                    };
                    boss.LISTEN(tier::anycast, app::term::events::data::prnscrn, gear)
                    {
                        boss.prnscrn(gear);
                    };
                    boss.LISTEN(tier::anycast, e2::form::upon::scroll::any, i)
                    {
                        auto info = e2::form::upon::scroll::bypage::y.param();
                        auto deed = boss.bell::template protos<tier::anycast>();
                        boss.base::template raw_riseup<tier::request>(e2::form::upon::scroll::any.id, info);
                        info.vector = i.vector;
                        boss.base::template raw_riseup<tier::preview>(deed, info);
                    };
                });
            return window;
        };
    }

    app::shared::initialize builder{ "term", build };
}