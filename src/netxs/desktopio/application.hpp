// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#if defined(__clang__) || defined(__APPLE__)
    #pragma clang diagnostic ignored "-Wunused-variable"
    #pragma clang diagnostic ignored "-Wunused-function"
#endif

#include "controls.hpp"

#include <fstream>

namespace netxs::app
{
    namespace fs = std::filesystem;
    using namespace std::placeholders;
    using namespace netxs::ui;
}

namespace netxs::app::shared
{
    static const auto version = "v0.9.9j";
    static const auto desktopio = "desktopio";
    static const auto logsuffix = "_log";
    static const auto usr_config = "~/.config/vtm/settings.xml";
    static const auto env_config = "$VTM_CONFIG"s;

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

        static constexpr auto drawfx = [](auto& boss, auto& canvas, auto handle, auto object_len, auto handle_len, auto region_len, auto wide)
        {
            if (object_len && handle_len != region_len) // Show only if it is oversized.
            {
                //canvas.fill(handle, [](cell& c) { c.und(!c.und()); });
                canvas.fill(handle, [](cell& c) { c.und(true); });
            }
        };
        static auto mini(bool autohide, bool menushow, bool menusize, bool custom, bool allow_minimize, list menu_items) // Menu bar (shrinkable on right-click).
        {
            auto highlight_color = skin::color(tone::highlight);
            auto danger_color    = skin::color(tone::danger);
            auto action_color    = skin::color(tone::action);
            auto c6 = action_color;
            auto x6 = cell{ c6 }.alpha(0x00);
            auto c3 = highlight_color;
            auto x3 = cell{ c3 }.alpha(0x00);
            auto c1 = danger_color;
            auto x1 = cell{ c1 }.alpha(0x00);

            auto slot1 = ui::veer::ctor();
            auto menuarea = ui::fork::ctor()->active();
                auto inner_pads = dent{ 1,2,1,1 };
                auto menulist = menuarea->attach(slot::_1, ui::fork::ctor());
                auto fader = skin::globals().fader_time;

                auto make_item = [&](auto& body, auto& dest_ptr, auto& fgc, auto& bgc)
                {
                    auto& item_ptr = std::get<0>(body);
                    auto& setup = std::get<1>(body);
                    auto& item = *item_ptr;
                    auto& hover = item.alive;
                    auto& label = item.views.front().label;
                    auto& notes = item.views.front().notes;
                    if (hover) dest_ptr->template plugin<pro::fader>(fgc, bgc, fader); //todo template: GCC complains
                    else       dest_ptr->colors(0,0); //todo for mouse tracking
                    dest_ptr->template plugin<pro::notes>(notes) //todo template: GCC complains
                            ->branch(ui::item::ctor(label, faux, true))
                            ->invoke([&](auto& boss) // Store shared ptr to the menu item config.
                            {
                                setup(boss, item);
                                boss.LISTEN(tier::release, e2::dtor, v, -, (item_ptr))
                                {
                                    item_ptr.reset();
                                };
                            });
                };
                auto headitem = menulist->attach(slot::_1, ui::pads::ctor(inner_pads, dent{ 0 }));
                auto tailitem = menuarea->attach(slot::_2, ui::pads::ctor(dent{ 2,2,1,1 }, dent{ 0 }));
                if (custom) // Apply a custom front/last menu items.
                {
                    make_item(menu_items.front(), headitem, x6, c6);
                    make_item(menu_items.back(),  tailitem, x1, c1);
                    menu_items.pop_front();
                    menu_items.pop_back();
                }
                else // Apply standard front/last menu items.
                {
                    headitem->plugin<pro::fader>(x3, c3, fader)
                            ->plugin<pro::notes>(" Fullscreen mode ")
                            ->invoke([&](ui::pads& boss)
                            {
                                boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                                {
                                    boss.RISEUP(tier::release, e2::form::layout::fullscreen, gear);
                                    gear.dismiss();
                                };
                            })
                            ->attach(ui::item::ctor(" ≡", faux, true));
                    tailitem->plugin<pro::fader>(x1, c1, fader)
                            ->plugin<pro::notes>(" Close ")
                            ->invoke([&](auto& boss)
                            {
                                boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
                                {
                                    boss.SIGNAL(tier::anycast, e2::form::proceed::quit::one, boss.This());
                                    gear.dismiss();
                                };
                            })
                            ->attach(ui::item::ctor("×"));
                }
                auto scrlarea = menulist->attach(slot::_2, ui::cake::ctor());
                auto scrlrail = scrlarea->attach(ui::rail::ctor(axes::X_only, axes::all));
                auto scrllist = scrlrail->attach(ui::list::ctor(axis::X));

                auto scroll_hint = ui::park::ctor();
                auto hints = scroll_hint->attach(ui::gripfx<axis::X, drawfx>::ctor(scrlrail), snap::stretch, menusize ? snap::center : snap::tail);

                auto scrl_grip = scrlarea->attach(scroll_hint);

                for (auto& body : menu_items)
                {
                    auto mid_item = scrllist->attach(ui::pads::ctor(inner_pads, dent{ 1 }));
                    make_item(body, mid_item, x3, c3);
                }

            auto menu_block = ui::park::ctor()
                ->plugin<pro::limit>(twod{ -1, menusize ? 1 : 3 }, twod{ -1, menusize ? 1 : 3 })
                ->invoke([&](ui::park& boss)
                {
                    scroll_hint->visible(hints, faux);
                    auto slim_status = ptr::shared(menusize);
                    if (allow_minimize)
                    {
                        boss.LISTEN(tier::release, hids::events::mouse::button::click::right, gear)
                        {
                            boss.RISEUP(tier::release, e2::form::layout::minimize, gear);
                            gear.dismiss();
                        };
                    }
                    boss.LISTEN(tier::anycast, e2::form::upon::resize, new_size, -, (slim_status))
                    {
                        if (!*slim_status)
                        {
                            auto& limit = boss.plugins<pro::limit>();
                            auto limits = limit.get();
                            if (new_size.y < 3)
                            {
                                if (limits.min.y != new_size.y)
                                {
                                    limits.min.y = limits.max.y = new_size.y;
                                    limit.set(limits);
                                }
                            }
                            else if (limits.min.y != 3)
                            {
                                limits.min.y = limits.max.y = 3;
                                limit.set(limits);
                            }
                        }
                    };
                    boss.LISTEN(tier::anycast, e2::form::prop::ui::slimmenu, slim, -, (slim_status))
                    {
                        auto& limit = boss.plugins<pro::limit>();
                        auto limits = limit.get();
                        *slim_status = slim;
                        limits.min.y = limits.max.y = slim ? 1 : 3;
                        limit.set(limits);
                        boss.reflow();
                    };
                    //todo revise
                    if (menu_items.size()) // Show scrolling hint only if elements exist.
                    {
                        auto scrl_shadow = ptr::shadow(scroll_hint);
                        auto grip_shadow = ptr::shadow(hints);
                        boss.LISTEN(tier::release, e2::form::state::mouse, active, -, (scrl_shadow, grip_shadow))
                        {
                            if (auto scrl_ptr = scrl_shadow.lock())
                            if (auto grip_ptr = grip_shadow.lock())
                            {
                                scrl_ptr->visible(grip_ptr, active);
                                boss.base::deface();
                            }
                        };
                    }
                });
            menu_block->attach(menuarea, snap::stretch, snap::center);

            auto menu = slot1->attach(menu_block);
                    auto border = slot1->attach(ui::mock::ctor())
                                       ->plugin<pro::limit>(twod{ -1,1 }, twod{ -1,1 });
                         if (menushow == faux) autohide = faux;
                    else if (autohide == faux) slot1->roll();
                    slot1->invoke([&](auto& boss)
                    {
                        auto border_shadow = ptr::shadow(border);
                        auto menu_shadow = ptr::shadow(menu_block);
                        auto hide_shadow = ptr::shared(autohide);
                        boss.LISTEN(tier::release, e2::form::state::mouse, hits, -, (menu_shadow, hide_shadow, border_shadow))
                        {
                            if (*hide_shadow)
                            if (auto menu_ptr = menu_shadow.lock())
                            {
                                auto menu_visible = boss.back() != menu_ptr;
                                if (!!hits == menu_visible)
                                {
                                    boss.roll();
                                    boss.reflow();
                                    if (auto border = border_shadow.lock())
                                    {
                                        border->SIGNAL(tier::release, e2::form::state::visible, menu_visible);
                                    }
                                }
                            }
                        };
                    });

            return std::tuple{ slot1, border, menu_block };
        };
        const auto create = [](xmls& config, list menu_items)
        {
            auto autohide = config.take("menu/autohide", faux);
            auto menushow = config.take("menu/enabled" , true);
            auto menusize = config.take("menu/slim"    , faux);
            return mini(autohide, menushow, menusize, faux, true, menu_items);
        };
        const auto demo = [](xmls& config)
        {
            auto items = list
            {
                { ptr::shared(item{ item::type::Command, true, 0, std::vector<item::look>{{ .label = ansi::und(true).add("F").nil().add("ile"), .notes = " File menu item " } }}), [&](auto& boss, auto& item){ } },
                { ptr::shared(item{ item::type::Command, true, 0, std::vector<item::look>{{ .label = ansi::und(true).add("E").nil().add("dit"), .notes = " Edit menu item " } }}), [&](auto& boss, auto& item){ } },
                { ptr::shared(item{ item::type::Command, true, 0, std::vector<item::look>{{ .label = ansi::und(true).add("V").nil().add("iew"), .notes = " View menu item " } }}), [&](auto& boss, auto& item){ } },
                { ptr::shared(item{ item::type::Command, true, 0, std::vector<item::look>{{ .label = ansi::und(true).add("D").nil().add("ata"), .notes = " Data menu item " } }}), [&](auto& boss, auto& item){ } },
                { ptr::shared(item{ item::type::Command, true, 0, std::vector<item::look>{{ .label = ansi::und(true).add("H").nil().add("elp"), .notes = " Help menu item " } }}), [&](auto& boss, auto& item){ } },
            };
            config.cd("/config/defapp/");
            auto [menu, cover, menu_data] = create(config, items);
            return menu;
        };
    }

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
        boss.LISTEN(tier::anycast, e2::form::proceed::quit::any, item)
        {
            boss.RISEUP(tier::release, e2::form::proceed::quit::one, item);
        };
    };
    const auto closing_by_gesture = [](auto& boss)
    {
        boss.LISTEN(tier::release, hids::events::mouse::button::click::leftright, gear)
        {
            auto backup = boss.This();
            boss.RISEUP(tier::release, e2::form::proceed::quit::one, backup);
            gear.dismiss();
        };
        boss.LISTEN(tier::release, hids::events::mouse::button::click::middle, gear)
        {
            auto backup = boss.This();
            boss.RISEUP(tier::release, e2::form::proceed::quit::one, backup);
            gear.dismiss();
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
        auto grip = ui::gripfx<axis::X, menu::drawfx>::ctor(master);
        area->branch(grip, snap::stretch, snap::tail)
            ->invoke([&](auto& boss)
            {
                area->visible(grip, faux);
                auto boss_shadow = ptr::shadow(boss.This());
                auto park_shadow = ptr::shadow(area);
                auto grip_shadow = ptr::shadow(grip);
                master->LISTEN(tier::release, e2::form::state::mouse, active, -, (boss_shadow, park_shadow, grip_shadow))
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

    using builder_t = std::function<sptr<base>(text, text, xmls&, text)>;

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
        [&](text, text, xmls&, text) -> sptr<base>
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
                ->attach(msg, snap::stretch, snap::stretch);
            window->attach(ui::rail::ctor())
                  ->attach(placeholder);
            return window;
        };
        auto& map = creator();
        const auto it = map.find(app_typename);
        if (it == map.end())
        {
            log("apps: unknown app type - '", app_typename, "'");
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
                auto config_path = path.starts_with("~") ? os::env::homepath() / path.substr(2 /* trim `~/` */)
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
    auto start(text app_name, text log_title, si32 vtmode, xmls& config)
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
            auto domain = base::create<host>(uplink, config);
            auto aclass = utf::to_low(utf::cutoff(app_name, ' '));
            auto params = utf::remain(app_name, ' ');
            auto applet = app::shared::builder(aclass)("", (direct ? "" : "!") + params, config, patch); // ! - means simple (w/o plugins)
            domain->invite(uplink, applet, vtmode);
            events::dequeue();
            domain->shutdown();
        };

        if (direct)
        {
            auto server = os::ipc::stdio();
            runapp(server);
        }
        else
        {
            auto [client, server] = os::ipc::xlink();
            auto thread = std::thread{ [&, &client = client]{ os::tty::splice(client, vtmode); }}; //todo clang 15.0.0 still disallows capturing structured bindings (wait for clang 16.0.0)
            runapp(server);
            thread.join();
        }
        return true;
    }

    struct initialize
    {
        initialize(text app_typename, builder_t builder)
        {
            auto& map = creator();
            map[app_typename] = builder;
        }
    };
}
