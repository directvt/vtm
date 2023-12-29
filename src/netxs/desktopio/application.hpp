// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#if defined(__clang__) || defined(__APPLE__)
    #pragma clang diagnostic ignored "-Wunused-variable"
    #pragma clang diagnostic ignored "-Wunused-function"
#endif

#include "console.hpp"
#include "system.hpp"
#include "scripting.hpp"

#include <fstream>

namespace netxs::app
{
    namespace fs = std::filesystem;
    using namespace std::placeholders;
    using namespace netxs::ui;
}

namespace netxs::app::shared
{
    static const auto version = "v0.9.44";
    static const auto repository = "https://github.com/directvt/vtm";
    static const auto ipc_prefix = "vtm";
    static const auto log_suffix = "_log";
    static const auto usr_config = "~/.config/vtm/settings.xml"s;
    static const auto sys_config = "/etc/vtm/settings.xml"s;

    const auto closing_on_quit = [](auto& boss)
    {
        boss.LISTEN(tier::anycast, e2::form::proceed::quit::any, fast)
        {
            boss.RISEUP(tier::release, e2::form::proceed::quit::one, fast);
        };
    };
    const auto closing_by_gesture = [](auto& boss)
    {
        boss.LISTEN(tier::release, hids::events::mouse::button::click::leftright, gear)
        {
            auto backup = boss.This();
            boss.RISEUP(tier::release, e2::form::proceed::quit::one, true);
            gear.dismiss();
        };
        boss.LISTEN(tier::release, hids::events::mouse::button::click::middle, gear)
        {
            auto backup = boss.This();
            boss.RISEUP(tier::release, e2::form::proceed::quit::one, true);
            gear.dismiss();
        };
    };
    const auto scroll_bars = [](auto master)
    {
        auto scroll_bars = ui::fork::ctor();
        auto scroll_bttm = scroll_bars->attach(slot::_1, ui::fork::ctor(axis::Y));
        auto hz = scroll_bttm->attach(slot::_2, ui::grip<axis::X>::ctor(master));
        auto vt = scroll_bars->attach(slot::_2, ui::grip<axis::Y>::ctor(master));
        return scroll_bars;
    };
    const auto underlined_hz_scrollbar = [](auto scrlrail)
    {
        auto grip = ui::gripfx<axis::X, ui::drawfx::underline>::ctor(scrlrail)
            ->alignment({ snap::both, snap::tail })
            ->invoke([&](auto& boss)
            {
                boss.base::hidden = true;
                scrlrail->LISTEN(tier::release, e2::form::state::mouse, active, -, (grip_shadow = ptr::shadow(boss.This())))
                {
                    if (auto grip_ptr = grip_shadow.lock())
                    {
                        grip_ptr->base::hidden = !active;
                        grip_ptr->base::reflow();
                    }
                };
            });
        return grip;
    };
    const auto scroll_bars_term = [](auto master)
    {
        auto scroll_bars = ui::fork::ctor();
        auto scroll_head = scroll_bars->attach(slot::_1, ui::fork::ctor(axis::Y));
        auto hz = scroll_head->attach(slot::_1, ui::grip<axis::X>::ctor(master));
        auto vt = scroll_bars->attach(slot::_2, ui::grip<axis::Y>::ctor(master));
        return scroll_bars;
    };
    const auto set_title = [](base& boss, input::hids& gear, bias alignment = bias::left)
    {
        boss.RISEUP(tier::request, e2::form::prop::ui::header, old_title, ());
        gear.owner.RISEUP(tier::request, hids::events::clipbrd, gear);
        auto& data = gear.board::cargo;
        if (data.utf8.empty())
        {
            log("%%Clipboard is empty or contains non-text data", prompt::desk);
        }
        else
        {
            if (utf::is_plain(data.utf8) || alignment != bias::left) // Reset aligning to the center if text is plain.
            {
                auto align = ansi::jet(alignment);
                boss.RISEUP(tier::preview, e2::form::prop::ui::header, align);
            }
            // Copy clipboard data to title.
            boss.RISEUP(tier::preview, e2::form::prop::ui::header, title, (data.utf8));
            if (old_title.size()) // Copy old title to clipboard.
            {
                gear.set_clipboard(dot_00, old_title, mime::ansitext);
            }
        }
        gear.dismiss(true);
    };

    using builder_t = std::function<ui::sptr(text, text, text, xmls&, text)>;

    namespace winform
    {
        namespace type
        {
            static const auto undefined = "undefined"s;
            static const auto minimized = "minimized"s;
            static const auto maximized = "maximized"s;
        }

        enum form
        {
            undefined,
            minimized,
            maximized,
        };

        static auto options = std::unordered_map<text, form>
           {{ type::undefined, form::undefined },
            { type::minimized, form::minimized },
            { type::maximized, form::maximized }};
    }

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
                cell hover{};
                cell focus{};
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
                auto count = static_cast<si32>(views.size());
                for (auto i = 0; i < count; i++)
                {
                    auto& l = views[i];
                    l.value = static_cast<si32>(take(l.param));
                    index[l.value] = i;
                }
            }
        };

        using link = std::tuple<item, std::function<void(ui::item&, item&)>>;
        using list = std::list<link>;

        static auto mini(bool autohide, bool menushow, bool slimsize, si32 custom, list menu_items) // Menu bar (shrinkable on right-click).
        {
            auto highlight_color = skin::color(tone::highlight);
            auto danger_color    = skin::color(tone::danger);
            auto action_color    = skin::color(tone::action);
            auto warning_color   = skin::color(tone::warning);
            auto c6 = action_color;
            auto c3 = highlight_color;
            auto c2 = warning_color;
            auto c1 = danger_color;
            auto macstyle = skin::globals().macstyle;
            auto menuveer = ui::veer::ctor();
            auto menufork = ui::fork::ctor()
                //todo
                //->alignment({ snap::both, snap::both }, { macstyle ? snap::head : snap::tail, snap::both })
                ->active();
            auto makeitem = [&](auto& config)
            {
                auto& props = std::get<0>(config);
                auto& setup = std::get<1>(config);
                auto& alive = props.alive;
                auto& label = props.views.front().label;
                auto& notes = props.views.front().notes;
                auto& hover = props.views.front().hover;
                auto button = ui::item::ctor(label)->drawdots();
                button->active(); // Always active for tooltips.
                if (alive)
                {
                    if (hover.set()) button->shader(hover                , e2::form::state::hover);
                    else             button->shader(cell::shaders::xlight, e2::form::state::hover);
                }
                button->template plugin<pro::notes>(notes)
                    ->setpad({ 2, 2, !slimsize, !slimsize })
                    ->invoke([&](auto& boss) // Store shared ptr to the menu item config.
                    {
                        auto props_shadow = ptr::shared(std::move(props));
                        setup(boss, *props_shadow);
                        boss.LISTEN(tier::release, e2::dtor, v, -, (props_shadow))
                        {
                            props_shadow.reset();
                        };
                    });
                return button;
            };
            auto ctrlslot = macstyle ? slot::_1 : slot::_2;
            auto menuslot = macstyle ? slot::_2 : slot::_1;
            auto ctrllist = menufork->attach(ctrlslot, ui::list::ctor(axis::X));
            if (custom) // Apply a custom menu controls.
            {
                while (custom--)
                {
                    auto button = makeitem(menu_items.back());
                    ctrllist->attach<sort::reverse>(button);
                    menu_items.pop_back();
                }
            }
            else // Add standard menu controls.
            {
                auto control = std::vector<link>
                {
                    { menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{{ .label = "—", .notes = " Minimize " }}},//, .hover = c2 }}}, //toto too funky
                    [](auto& boss, auto& item)
                    {
                        boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                        {
                            boss.RISEUP(tier::release, e2::form::size::minimize, gear);
                            gear.dismiss();
                        };
                    }},
                    { menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{{ .label = "□", .notes = " Maximize " }}},//, .hover = c6 }}},
                    [](auto& boss, auto& item)
                    {
                        boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                        {
                            boss.RISEUP(tier::preview, e2::form::size::enlarge::maximize, gear);
                            gear.dismiss();
                        };
                    }},
                    { menu::item{ menu::item::type::Command, true, 0, std::vector<menu::item::look>{{ .label = "×", .notes = " Close ", .hover = c1 }}},
                    [](auto& boss, auto& item)
                    {
                        boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                        {
                            auto backup = boss.This();
                            boss.SIGNAL(tier::anycast, e2::form::proceed::quit::one, faux); // fast=faux: Show closing process.
                            gear.dismiss();
                        };
                    }},
                };
                if (macstyle)
                {
                    ctrllist->attach(makeitem(control[2]));
                    ctrllist->attach(makeitem(control[0]));
                    ctrllist->attach(makeitem(control[1]));
                }
                else
                {
                    ctrllist->attach(makeitem(control[0]));
                    ctrllist->attach(makeitem(control[1]));
                    ctrllist->attach(makeitem(control[2]));
                }
            }
            auto scrlarea = menufork->attach(menuslot, ui::cake::ctor());
            auto scrlrail = scrlarea->attach(ui::rail::ctor(axes::X_only, axes::all));
            auto scrllist = scrlrail->attach(ui::list::ctor(axis::X));

            auto scrlcake = ui::cake::ctor();
            auto scrlhint = scrlcake->attach(underlined_hz_scrollbar(scrlrail));
            auto scrlgrip = scrlarea->attach(scrlcake);

            for (auto& body : menu_items)
            {
                scrllist->attach(makeitem(body));
            }

            auto menucake = menuveer->attach(ui::cake::ctor()->branch(menufork))
                ->invoke([&](auto& boss)
                {
                    auto slim_status = ptr::shared(slimsize);
                    boss.LISTEN(tier::anycast, e2::form::upon::resized, new_area, -, (slim_status))
                    {
                        if (!*slim_status)
                        {
                            auto height = boss.base::min_sz.y;
                            if (new_area.size.y < 3)
                            {
                                if (height != new_area.size.y)
                                {
                                    boss.base::limits({ -1, new_area.size.y }, { -1, new_area.size.y });
                                }
                            }
                            else if (height != 3)
                            {
                                boss.base::limits({ -1, 3 }, { -1, 3 });
                            }
                        }
                    };
                    boss.LISTEN(tier::anycast, e2::form::prop::ui::slimmenu, slim, -, (slim_status))
                    {
                        *slim_status = slim;
                        auto height = slim ? 1 : 3;
                        boss.base::limits({ -1, height }, { -1, height });
                        boss.reflow();
                    };
                });
            auto menutent = menuveer->attach(ui::mock::ctor()->limits({ -1,1 }, { -1,1 }));
                 if (menushow == faux) autohide = faux;
            else if (autohide == faux) menuveer->roll();
            menuveer->limits({ -1, slimsize ? 1 : 3 }, { -1, slimsize ? 1 : 3 })
                ->invoke([&](auto& boss)
                {
                    auto menutent_shadow = ptr::shadow(menutent);
                    auto menucake_shadow = ptr::shadow(menucake);
                    auto autohide_shadow = ptr::shared(autohide);
                    boss.LISTEN(tier::release, e2::form::state::mouse, hits, -, (menucake_shadow, autohide_shadow, menutent_shadow))
                    {
                        if (*autohide_shadow)
                        if (auto menucake = menucake_shadow.lock())
                        {
                            auto menu_visible = boss.back() != menucake;
                            if (!!hits == menu_visible)
                            {
                                boss.roll();
                                boss.reflow();
                                if (auto menutent = menutent_shadow.lock())
                                {
                                    menutent->SIGNAL(tier::release, e2::form::state::visible, menu_visible);
                                }
                            }
                        }
                    };
                });

            return std::tuple{ menuveer, menutent, menucake };
        };
        const auto create = [](xmls& config, list menu_items)
        {
            auto autohide = config.take("menu/autohide", faux);
            auto menushow = config.take("menu/enabled" , true);
            auto slimsize = config.take("menu/slim"    , faux);
            return mini(autohide, menushow, slimsize, 0, menu_items);
        };
        const auto demo = [](xmls& config)
        {
            auto highlight_color = skin::color(tone::highlight);
            auto c3 = highlight_color;
            auto items = list
            {
                { item{ item::type::Command, true, 0, std::vector<item::look>{{ .label = ansi::und(true).add("F").nil().add("ile"), .notes = " File menu item " }}}, [&](auto& boss, auto& item){ }},
                { item{ item::type::Command, true, 0, std::vector<item::look>{{ .label = ansi::und(true).add("E").nil().add("dit"), .notes = " Edit menu item " }}}, [&](auto& boss, auto& item){ }},
                { item{ item::type::Command, true, 0, std::vector<item::look>{{ .label = ansi::und(true).add("V").nil().add("iew"), .notes = " View menu item " }}}, [&](auto& boss, auto& item){ }},
                { item{ item::type::Command, true, 0, std::vector<item::look>{{ .label = ansi::und(true).add("D").nil().add("ata"), .notes = " Data menu item " }}}, [&](auto& boss, auto& item){ }},
                { item{ item::type::Command, true, 0, std::vector<item::look>{{ .label = ansi::und(true).add("H").nil().add("elp"), .notes = " Help menu item " }}}, [&](auto& boss, auto& item){ }},
            };
            config.cd("/config/defapp/");
            auto [menu, cover, menu_data] = create(config, items);
            return menu;
        };
    }
    namespace
    {
        auto& creator()
        {
            static auto creator = std::map<text, builder_t>{};
            return creator;
        }
    }
    auto& builder(text app_typename)
    {
        static builder_t empty =
        [&](text, text, text, xmls&, text) -> ui::sptr
        {
            auto window = ui::cake::ctor()
                ->plugin<pro::focus>()
                ->plugin<pro::track>()
                ->plugin<pro::acryl>()
                ->invoke([&](auto& boss)
                {
                    //boss.keybd.accept(true);
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
                ->upload(ansi::fgc(yellowlt).mgl(4).mgr(4).wrp(wrap::off) +
                    "\n"
                    "\nUnsupported application type"
                    "\n" + ansi::nil().wrp(wrap::on) +
                    "\nOnly the following application types are supported"
                    "\n" + ansi::nil().wrp(wrap::off).fgc(whitedk) +
                    "\n   type = DirectVT(dtvt)"
                    "\n   type = XLinkVT(xlvt)"
                    "\n   type = ANSIVT"
                    "\n   type = SHELL"
                    "\n   type = Group"
                    "\n   type = Region"
                    "\n"
                    "\n" + ansi::nil().wrp(wrap::on).fgc(whitelt)
                    .add(prompt::apps, "See logs for details."));
            auto placeholder = ui::cake::ctor()
                ->colors(whitelt, rgba{ 0x7F404040 })
                ->attach(msg->alignment({ snap::head, snap::head }));
            window->attach(ui::rail::ctor())
                ->attach(placeholder);
            return window;
        };
        auto& map = creator();
        const auto it = map.find(app_typename);
        if (it == map.end())
        {
            log("%%Unknown app type - '%app_typename%'", prompt::apps, app_typename);
            return empty;
        }
        else return it->second;
    };
    namespace load
    {
        template<bool Print = faux>
        auto settings(view defaults, view cli_config_path, view patch)
        {
            auto conf = xmls{ defaults };
            auto load = [&](qiew shadow)
            {
                if (shadow.empty()) return faux;
                if (shadow.starts_with(":"))
                {
                    shadow.remove_prefix(1);
                    auto utf8 = os::process::memory::get(shadow);
                    if (utf8.size())
                    {
                        conf.fuse<Print>(utf8);
                        return true;
                    }
                    else
                    {
                        log(prompt::apps, "Failed to get settings from :", shadow);
                        return faux;
                    }
                }
                auto [config_path, config_path_str] = os::path::expand(shadow);
                if (config_path.empty()) return faux;
                log("%%Loading settings from %path%...", prompt::apps, config_path_str);
                auto ec = std::error_code{};
                auto config_file = fs::directory_entry(config_path, ec);
                if (!ec && (config_file.is_regular_file() || config_file.is_symlink()))
                {
                    auto file = std::ifstream(config_file.path(), std::ios::binary | std::ios::in);
                    if (file.seekg(0, std::ios::end).fail())
                    {
                        log(prompt::pads, "Failed\n\tUnable to get settings file size, skip ", config_path_str);
                        return faux;
                    }
                    else
                    {
                        log(prompt::pads, "Merging settings from ", config_path_str);
                        auto size = file.tellg();
                        auto buff = text((size_t)size, '\0');
                        file.seekg(0, std::ios::beg);
                        file.read(buff.data(), size);
                        conf.fuse<Print>(buff, config_path_str);
                        return true;
                    }
                }
                log(prompt::pads, "Not found");
                return faux;
            };
            if (!load(cli_config_path)) // Merge explicitly specified settings.
            {
                load(app::shared::sys_config); // Merge system-wide settings.
                load(app::shared::usr_config); // Merge user-wise settings.
            }
            conf.fuse<Print>(patch); // Apply dtvt patch.
            return conf;
        }
    }

    struct initialize
    {
        initialize(text app_typename, builder_t builder)
        {
            auto& map = creator();
            map[app_typename] = builder;
        }
    };

    void start(text params, text aclass, si32 vtmode, twod winsz, xmls& config)
    {
        auto [client, server] = os::ipc::xlink();
        auto thread = std::thread{[&, &client = client] //todo clang 15.0.0 still disallows capturing structured bindings (wait for clang 16.0.0)
        {
            os::tty::splice(client);
        }};
        //if (!config.cd("/config/" + aclass)) config.cd("/config/appearance/");
        config.cd("/config/appearance/runapp/", "/config/appearance/defaults/");
        auto domain = ui::host::ctor(server, config)
            ->plugin<scripting::host>();
        auto direct = os::dtvt::active;
        os::dtvt::isolated = !direct;
        auto applet = app::shared::builder(aclass)("", "", params, config, /*patch*/(direct ? ""s : "<config isolated=1/>"s));
        domain->invite(server, applet, vtmode, winsz);
        domain->stop();
        server->shut();
        thread.join();
    }
}
