// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#define MONOTTY_VER "Monotty Desktopio v0.5.9999a"

// Enable demo apps and assign Esc key to log off.
#define DEMO

// Enable keyboard input and unassign Esc key.
#define PROD

// Tiling nesting max level.
#ifndef PROD
    #define INHERITANCE_LIMIT 12
    #define APPS_MAX_COUNT 20
    #define APPS_DEL_TIMEOUT 1s
#else
    #define INHERITANCE_LIMIT 30
#endif

// Enable to show debug overlay.
//#define DEBUG_OVERLAY

// Enable to show all terminal input (keyboard/mouse etc).
//#define KEYLOG

// Highlight region ownership.
//#define REGIONS

#include "netxs/console/terminal.hpp"
#include "netxs/apps.hpp"

#include <fstream>

//todo remove
using namespace std::placeholders;
using namespace netxs::console;
using namespace netxs;

int main(int argc, char* argv[])
{
    // Initialize global logger.
    netxs::logger::logger logger(
        [](auto const& utf8)
        {
            static text buff;
            os::syslog(utf8);
            if (auto sync = events::try_sync{})
            {
                if (buff.size())
                {
                    SIGNAL_GLOBAL(e2::debug::logs, view{ buff });
                    buff.clear();
                }
                SIGNAL_GLOBAL(e2::debug::logs, view{ utf8 });
            }
            else buff += utf8;
        });

    {
        auto banner = [&]() { log(MONOTTY_VER"\nDesktop Environment Server"); };
        bool daemon = faux;
        auto getopt = os::args{ argc, argv };
        while (getopt)
        {
            switch (getopt.next())
            {
                case 'd':
                    daemon = true;
                    break;
                default:
                    banner();
                    log("Usage:\n\t", argv[0], " [ -d ]\n\n\t-d\tRun as a daemon.");
                    os::exit(1);
            }
        }

        if (daemon && !os::daemonize(argv[0]))
        {
            banner();
            os::exit(1, "main: failed to daemonize");
        }

        banner();
    }

    //todo Get current config from vtm.conf.
    text config;
    {
        std::ifstream conf;
        conf.open("vtm.conf");
        if (conf.is_open()) std::getline(conf, config);
        if (config.empty()) config = "empty config";
    }

    // vtm: Get user defined tiling layouts.
    auto tiling_profiles = os::get_envars("VTM_PROFILE");
    if (auto size = tiling_profiles.size())
    {
        iota i = 0;
        log("main: tiling profile", size > 1 ? "s":"", " found");
        for (auto& p : tiling_profiles)
        {
            log(" ", i++, ". profile: ", utf::debase(p));
        }
    }

    {
        skin::setup(tone::kb_focus, 60);
        skin::setup(tone::brighter, 60);//120);
        //skin::setup(tone::shadower, 0);
        skin::setup(tone::shadower, 180);//60);//40);// 20);
        skin::setup(tone::shadow, 180);//5);
        //skin::setup(tone::lucidity, 192);
        skin::setup(tone::lucidity, 255);
        skin::setup(tone::selector, 48);
        skin::setup(tone::bordersz, dot_11);

        auto world = base::create<host>([&](auto reason) { os::exit(0, reason); });

        log("host: created");

        using slot = ui::slot;
        using axis = ui::axis;
        using axes = ui::axes;
        using snap = ui::snap;
        using id_t = netxs::input::id_t;

        world->SUBMIT(tier::release, e2::form::proceed::createat, what)
        {
            auto menu_item_id = what.menu_item_id;
            auto location = what.location;

            auto config = app::shared::objs_config[menu_item_id];
            sptr<ui::cake> window = ui::cake::ctor()
                ->plugin<pro::title>(config.title)
                ->plugin<pro::limit>(dot_11, twod{ 400,200 }) //todo unify, set via config
                ->plugin<pro::sizer>()
                ->plugin<pro::frame>()
                ->plugin<pro::light>()
                ->plugin<pro::align>()
                ->invoke([&](ui::cake& boss)
                {
                    boss.SUBMIT(tier::release, hids::events::mouse::button::dblclick::left, gear)
                    {
                        boss.base::template riseup<tier::release>(e2::form::maximize, gear);
                        gear.dismiss();
                    };
                    boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                    {
                        auto& area = boss.base::area();
                        if (!area.size.inside(gear.coord))
                        {
                            auto center = area.coor + (area.size / 2);
                            bell::getref(gear.id)->
                                SIGNAL(tier::release, e2::form::layout::shift, center);
                        }
                        boss.base::deface();
                    };
                    boss.SUBMIT(tier::release, hids::events::mouse::button::click::leftright, gear)
                    {
                        auto backup = boss.This();
                        boss.base::detach();
                        gear.dismiss();
                    };
                    boss.SUBMIT(tier::release, hids::events::mouse::button::click::middle, gear)
                    {
                        auto backup = boss.This();
                        boss.base::detach();
                        gear.dismiss();
                    };
                    boss.SUBMIT(tier::release, e2::form::proceed::detach, backup)
                    {
                        boss.base::detach(); // The object kills itself.
                    };
                    boss.SUBMIT(tier::release, e2::form::quit, nested_item)
                    {
                        if (nested_item) boss.base::detach(); // The object kills itself.
                    };
                });

            window->extend(location);
            auto& creator = app::shared::creator(config.type);
            window->attach(creator(config.data));
            log(" world create type: ", config.type);
            world->branch(config.type, window);

            what.frame = window;
        };
        world->SUBMIT(tier::general, e2::form::global::lucidity, alpha)
        {
            if (alpha == -1)
            {
                alpha = skin::shady();
            }
            else
            {
                alpha = std::clamp(alpha, 0, 255);
                skin::setup(tone::lucidity, alpha);
                world->SIGNAL(tier::preview, e2::form::global::lucidity, alpha);
            }
        };

        // Init registry/menu list.
        {
            sptr<registry_t> menu_list_ptr;
            world->SIGNAL(tier::request, e2::bindings::list::apps, menu_list_ptr);
            auto& menu_list = *menu_list_ptr;
            auto b = app::shared::objs_config.begin();
            auto e = app::shared::objs_config.end();

            #ifdef DEMO
                #ifdef PROD
                    //app::shared::objs_config["Tile"].data = "VTM_PROFILE_1=\"Tile\", \"Tiling Window Manager\", h1:1(v1:1(\"bash -c 'LC_ALL=en_US.UTF-8 mc -c -x; bash'\", h1:1(\"bash -c 'ls /bin | nl | ccze -A; bash'\", a(\"RefreshRate\",\"\",\"\"))), a(\"Calc\",\"app title\",\"app data\"))";
                    //app::shared::objs_config["Tile"].data = "VTM_PROFILE_1=\"Tile\", \"Tiling Window Manager\", h1:1(v1:1(\"bash -c 'LC_ALL=en_US.UTF-8 mc -c -x; bash'\", h1:1(\"bash -c 'ls /bin | nl | ccze -A; bash'\", a(\"Text\",\"app title\",\"app data\"))), a(\"Calc\",\"app title\",\"app data\"))";
                    //app::shared::objs_config["Tile"].data = "VTM_PROFILE_1=\"Tile\", \"Tiling Window Manager\", h1:1:1(v1:1:2(\"bash -c 'LC_ALL=en_US.UTF-8 mc -c -x -d; cat'\", h1:1:0(\"bash -c 'ls /bin | nl | ccze -A; bash'\", a(\"RefreshRate\",\"\",\"\"))), a(\"Calc\",\"\",\"\"))";
                    app::shared::objs_config["Tile"].data = "VTM_PROFILE_1=\"Tile\", \"Tiling Window Manager\", h(v(\"bash -c 'LC_ALL=en_US.UTF-8 mc -c -x -d; cat'\", h(\"bash -c 'ls /bin | nl | ccze -A; bash'\", a(\"RefreshRate\",\"\",\"\"))), a(\"Calc\",\"\",\"\"))";
                #else
                    app::shared::objs_config["Tile"].data = "VTM_PROFILE_1=\"Tile\", \"Tiling Window Manager\", h1:1(v1:1(\"bash -c 'LC_ALL=en_US.UTF-8 mc -c -x -d; cat'\", h1:1(\"bash -c 'ls /bin | nl | ccze -A; bash'\", a(\"RefreshRate\",\"\",\"\"))), a(\"Calc\",\"\",\"\"))";
                #endif

                for (auto& [app_name, app_data] : app::shared::objs_config)
                    menu_list[app_name];
            #else
                #ifdef _WIN32
                    menu_list["CommandPrompt"];
                    menu_list["PowerShell"];
                    menu_list["Tile"];
                    menu_list["Logs"];
                    menu_list["View"];
                    menu_list["RefreshRate"];
                #else
                    menu_list["Term"];
                    menu_list["Tile"];
                    menu_list["Logs"];
                    menu_list["View"];
                    menu_list["RefreshRate"];
                    menu_list["vtm"];
                #endif
                // Add custom commands to the menu.
                for (auto& p : tiling_profiles)
                {
                    //todo rewrite
                    auto v = view{ p };
                    auto name = utf::get_quote(v, '\"');
                    if (!name.empty())
                    {
                        auto& m = app::shared::objs_config[name];
                        m.type = "Tile";
                        m.name = name;
                        m.title = name; // Use the same title as the menu label.
                        m.data = text{ p };
                    }
                }
            #endif

            #ifdef DEMO
                auto creator = [&](text const& menu_item_id, rect area)
                {
                    auto what = decltype(e2::form::proceed::createat)::type{};
                    what.menu_item_id = menu_item_id;
                    what.location = area;
                    world->SIGNAL(tier::release, e2::form::proceed::createat, what);
                };
                auto sub_pos = twod{ 12+17, 0 };
                creator("Test", { twod{ 22     , 1  } + sub_pos, { 70, 21 } });
                creator("Shop", { twod{ 4      , 6  } + sub_pos, { 82, 38 } });
                creator("Calc", { twod{ 15     , 15 } + sub_pos, { 65, 23 } });
                creator("Text", { twod{ 30     , 22 } + sub_pos, { 59, 26 } });
                creator("MC",   { twod{ 49     , 28 } + sub_pos, { 63, 22 } });
                creator("Term", { twod{ 34     , 38 } + sub_pos, { 64, 16 } });
                creator("Term", { twod{ 44 + 85, 35 } + sub_pos, { 64, 15 } });
                creator("Term", { twod{ 40 + 85, 42 } + sub_pos, { 64, 15 } });
                creator("Tile", { twod{ 40 + 85,-10 } + sub_pos, {160, 42 } });

                creator("View", { twod{ 0, 7 } + twod{ -120, 60 }, { 120, 52 } });
                creator("View", { twod{ 0,-1 } + sub_pos, { 120, 52 } });

                sub_pos = twod{-120, 60};
                creator("Truecolor",   { twod{ 20, 15 } + sub_pos, { 70, 30 } });
                creator("Logs",        { twod{ 52, 33 } + sub_pos, { 45, 12 } });
                creator("RefreshRate", { twod{ 60, 41 } + sub_pos, { 35, 10 } });
            #endif

        }

        world->SIGNAL(tier::general, e2::config::fps, 60);

        iota usr_count = 0;
        auto user = os::user();
        auto path = utf::concat("monotty_", user);
        log("user: ", user);
        log("pipe: ", path);

        if (auto link = os::ipc::open<os::server>(path))
        {
            log("sock: listening socket ", link);

            while (auto peer = link->meet())
            {
                if (!peer->cred(user))
                {
                    log("sock: other users are not allowed to the session, abort");
                    continue;
                }

                auto _region = peer->line(';');
                auto _ip     = peer->line(';');
                auto _user   = peer->line(';');
                auto _name   = peer->line(';');
                auto _mode   = peer->line(';');
                log("peer: region= ", _region,
                        ", ip= "    , _ip,
                        ", user= "  , _user,
                        ", name= "  , _name,
                        ", mode= "  , _mode);
                text c_ip;
                text c_port;
                auto c_info = utf::divide(_ip, " ");
                if (c_info.size() > 0) c_ip   = c_info[0];
                if (c_info.size() > 1) c_port = c_info[1];

                utf::change(_ip, " ", ":");

                //todo Move user's viewport to the last saved position
                auto user_coor = twod{};

                //todo distinguish users by config, enumerate if no config
                _name = "[" + _name + ":" + std::to_string(usr_count++) + "]";
                log("main: creating a new thread for user ", _name);

                std::thread{ [=]
                {
                    iota uibar_min_size = 4;
                    iota uibar_full_size = 32;
                    log("user: session name ", peer);

                    #ifndef PROD
                        auto username = "[User." + utf::remain(c_ip) + ":" + c_port + "]";
                    #else
                        auto username = _name;
                    #endif

                    auto lock = std::make_unique<events::sync>();
                    iota legacy_mode = os::legacy::clean;
                    if (auto mode = utf::to_int(view(_mode)))
                    {
                        legacy_mode = mode.value();
                    }
                    auto client = world->invite<ui::gate>(username, legacy_mode);
                    auto client_shadow = ptr::shadow(client);
                    auto world_shadow = ptr::shadow(world);
                    auto my_id = client->id;

                    // Taskbar Layout (PoC)

                    #ifdef _WIN32
                        auto current_default = "CommandPrompt"s;
                        //auto current_default = "PowerShell"s;
                    #else
                        auto current_default = "Term"s;
                    #endif
                    auto previous_default = current_default;

                    //todo unify
                    client->SUBMIT(tier::request, e2::data::changed, data)
                    {
                        data = current_default;
                    };
                    client->SUBMIT(tier::preview, e2::data::changed, data)
                    {
                        data = previous_default;
                    };
                    client->SUBMIT(tier::release, e2::data::changed, data)
                    {
                        auto new_default = data;
                        if (current_default != new_default)
                        {
                            previous_default = current_default;
                            current_default = new_default;
                        }
                    };

                    auto app_template = [&](auto& data_src, auto const& utf8)
                    {
                        const static auto c4 = app::shared::c4;
                        const static auto x4 = app::shared::x4;
                        const static auto c5 = app::shared::c5;
                        const static auto x5 = app::shared::x5;

                        auto item_area = ui::pads::ctor(dent{ 1,0,1,0 }, dent{ 0,0,0,1 })
                                ->plugin<pro::fader>(x4, c4, 0ms)//150ms)
                                ->invoke([&](auto& boss)
                                {
                                    auto data_src_shadow = ptr::shadow(data_src);
                                    boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::click::left, gear)
                                    {
                                        if (auto data_src = data_src_shadow.lock())
                                        {
                                            auto& inst = *data_src;
                                            inst.SIGNAL(tier::preview, e2::form::layout::expose, inst);
                                            auto& area = inst.base::area();
                                            auto center = area.coor + (area.size / 2);
                                            bell::getref(gear.id)->
                                                SIGNAL(tier::release, e2::form::layout::shift, center);  // Goto to the window.
                                            gear.pass_kb_focus(inst);
                                            gear.dismiss();
                                        }
                                    };
                                    boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::click::right, gear)
                                    {
                                        if (auto data_src = data_src_shadow.lock())
                                        {
                                            auto& inst = *data_src;
                                            inst.SIGNAL(tier::preview, e2::form::layout::expose, inst);
                                            auto& area = gear.area();
                                            auto center = area.coor + (area.size / 2);
                                            inst.SIGNAL(tier::preview, e2::form::layout::appear, center); // Pull window.
                                            gear.pass_kb_focus(inst);
                                            gear.dismiss();
                                        }
                                    };
                                    boss.SUBMIT_BYVAL(tier::release, e2::form::state::mouse, hits)
                                    {
                                        if (auto data_src = data_src_shadow.lock())
                                        {
                                            data_src->SIGNAL(tier::release, e2::form::highlight::any, !!hits);
                                        }
                                    };
                                });
                            auto label_area = item_area->attach(ui::fork::ctor());
                                auto mark_app = label_area->attach(slot::_1, ui::fork::ctor());
                                    auto mark = mark_app->attach(slot::_1, ui::pads::ctor(dent{ 2,1,0,0 }, dent{ 0,0,0,0 }))
                                                        ->attach(ui::item::ctor(ansi::fgc4(0xFF00ff00).add("‣"), faux));
                                    auto app_label = mark_app->attach(slot::_2,
                                                ui::item::ctor(ansi::fgc(whitelt).add(utf8).mgl(0).wrp(wrap::off).jet(bias::left), true, true));
                                auto app_close_area = label_area->attach(slot::_2, ui::pads::ctor(dent{ 0,0,0,0 }, dent{ 0,0,1,1 }))
                                                                ->template plugin<pro::fader>(x5, c5, 150ms)
                                                                ->invoke([&](auto& boss)
                                                                {
                                                                    auto data_src_shadow = ptr::shadow(data_src);
                                                                    boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::click::left, gear)
                                                                    {
                                                                        if (auto data_src = data_src_shadow.lock())
                                                                        {
                                                                            data_src->SIGNAL(tier::release, e2::form::proceed::detach, data_src);
                                                                            gear.dismiss();
                                                                        }
                                                                    };
                                                                });
                                    auto app_close = app_close_area->attach(ui::item::ctor("  ×  ", faux));
                        return item_area;
                    };
                    auto apps_template = [&](auto& data_src, auto& apps_map)
                    {
                        const static auto c3 = app::shared::c3;
                        const static auto x3 = app::shared::x3;

                        auto apps = ui::list::ctor();
                        //todo loops are not compatible with Declarative UI
                        for (auto const& [class_id, inst_ptr_list] : *apps_map)
                        {
                            auto inst_id  = class_id;
                            auto obj_desc = app::shared::objs_config[class_id].name;
                            if (inst_ptr_list.size())
                            {
                                auto selected = class_id == current_default;
                                auto item_area = apps->attach(ui::pads::ctor(dent{ 0,0,0,1 }, dent{ 0,0,1,0 }))
                                                    ->template plugin<pro::fader>(x3, c3, 0ms)
                                                    ->depend_on_collection(inst_ptr_list)
                                                    ->invoke([&](auto& boss)
                                                    {
                                                        boss.mouse.take_all_events(faux);
                                                        auto data_src_shadow = ptr::shadow(data_src);
                                                        boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::click::left, gear)
                                                        {
                                                            if (auto data_src = data_src_shadow.lock())
                                                            {
                                                                sptr<registry_t> registry_ptr;
                                                                data_src->SIGNAL(tier::request, e2::bindings::list::apps, registry_ptr);
                                                                auto& app_list = (*registry_ptr)[inst_id];
                                                                // Rotate list forward.
                                                                app_list.push_back(app_list.front());
                                                                app_list.pop_front();
                                                                // Expose window.
                                                                auto& inst = *app_list.back();
                                                                inst.SIGNAL(tier::preview, e2::form::layout::expose, inst);
                                                                auto& area = inst.base::area();
                                                                auto center = area.coor + (area.size / 2);
                                                                bell::getref(gear.id)->
                                                                SIGNAL(tier::release, e2::form::layout::shift, center);  // Goto to the window.
                                                                gear.pass_kb_focus(inst);
                                                                gear.dismiss();
                                                            }
                                                        };
                                                        boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::click::right, gear)
                                                        {
                                                            if (auto data_src = data_src_shadow.lock())
                                                            {
                                                                sptr<registry_t> registry_ptr;
                                                                data_src->SIGNAL(tier::request, e2::bindings::list::apps, registry_ptr);
                                                                auto& app_list = (*registry_ptr)[inst_id];
                                                                // Rotate list forward.
                                                                app_list.push_front(app_list.back());
                                                                app_list.pop_back();
                                                                // Expose window.
                                                                auto& inst = *app_list.back();
                                                                inst.SIGNAL(tier::preview, e2::form::layout::expose, inst);
                                                                auto& area = inst.base::area();
                                                                auto center = area.coor + (area.size / 2);
                                                                bell::getref(gear.id)->
                                                                SIGNAL(tier::release, e2::form::layout::shift, center);  // Goto to the window.
                                                                gear.pass_kb_focus(inst);
                                                                gear.dismiss();
                                                            }
                                                        };
                                                    });
                                    auto block = item_area->attach(ui::fork::ctor(axis::Y));
                                        auto head_area = block->attach(slot::_1, ui::pads::ctor(dent{ 0,0,0,0 }, dent{ 0,0,1,1 }));
                                            auto head = head_area->attach(ui::item::ctor(obj_desc, true));
                                        auto list_pads = block->attach(slot::_2, ui::pads::ctor(dent{ 0,0,0,0 }, dent{ 0,0,0,0 }));
                                auto insts = list_pads->attach(ui::list::ctor())
                                                      ->attach_collection(e2::form::prop::header, inst_ptr_list, app_template);
                            }
                        }
                        return apps;
                    };
                    auto menuitems_template = [&](auto& data_src, auto& apps_map)
                    {
                        const static auto c3 = app::shared::c3;
                        const static auto x3 = app::shared::x3;

                        auto menuitems = ui::list::ctor();
                        //todo loops are not compatible with Declarative UI
                        for (auto const& [class_id, inst_ptr_list] : *apps_map)
                        {
                            auto id = class_id;
                            auto obj_desc = app::shared::objs_config[class_id].name;

                            auto selected = class_id == current_default;
                            auto item_area = menuitems->attach(ui::pads::ctor(dent{ 0,0,0,1 }, dent{ 0,0,1,0 }))
                                                    ->plugin<pro::fader>(x3, c3, 0ms)
                                                    ->invoke([&](auto& boss)
                                                    {
                                                        boss.mouse.take_all_events(faux);
                                                        boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::click::left, gear)
                                                        {
                                                            if (auto client = client_shadow.lock())
                                                            {
                                                                client->SIGNAL(tier::release, e2::data::changed, id);
                                                                gear.dismiss();
                                                            }
                                                        };
                                                        boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::dblclick::left, gear)
                                                        {
                                                            if (auto world = world_shadow.lock())
                                                            {
                                                                static iota random = 0;
                                                                random = (random + 2) % 10;
                                                                auto offset = twod{ random * 2, random };
                                                                auto viewport = gear.area();
                                                                gear.slot.coor = viewport.coor + viewport.size / 4 + offset;
                                                                gear.slot.size = viewport.size / 2;
                                                                world->SIGNAL(tier::release, e2::form::proceed::createby, gear);
                                                            }
                                                        };
                                                    });
                                auto block = item_area->attach(ui::fork::ctor(axis::X));
                                    auto mark_area = block->attach(slot::_1, ui::pads::ctor(dent{ 1,1,0,0 }, dent{ 0,0,0,0 }));
                                        auto mark = mark_area->attach(ui::item::ctor(ansi::fgc4(selected ? 0xFF00ff00
                                                                                                         : 0xFF000000).add("██"), faux))
                                                    ->invoke([&](auto& boss)
                                                    {
                                                        if (auto client = client_shadow.lock())
                                                        {
                                                            auto mark_shadow = ptr::shadow(boss.This());
                                                            client->SUBMIT_T_BYVAL(tier::release, e2::data::changed, boss.tracker, data)
                                                            {
                                                                auto selected = id == data;
                                                                if (auto mark = mark_shadow.lock())
                                                                {
                                                                    mark->set(ansi::fgc4(selected ? 0xFF00ff00 : 0xFF000000).add("██"));
                                                                    mark->deface();
                                                                }
                                                            };
                                                        }
                                                    });
                                    auto label_area = block->attach(slot::_2, ui::pads::ctor(dent{ 1,1,0,0 }, dent{ 0,0,0,0 }));
                                        auto label = label_area->attach(ui::item::ctor(ansi::fgc4(0xFFffffff).add(obj_desc), true, true));
                        }
                        return menuitems;
                    };
                    auto user_template = [&](auto& data_src, auto const& utf8)
                    {
                        const static auto c3 = app::shared::c3;
                        const static auto x3 = app::shared::x3;

                        auto item_area = ui::pads::ctor(dent{ 1,0,0,1 }, dent{ 0,0,1,0 })
                                             ->plugin<pro::fader>(x3, c3, 150ms);
                            auto user = item_area->attach(
                                ui::item::ctor(ansi::esc(" &").nil().add(" ")
                                    .fgc4(data_src->id == my_id ? rgba::color256[whitelt] : 0x00).add(utf8), true));
                        return item_area;
                    };
                    auto branch_template = [&](auto& data_src, auto& usr_list)
                    {
                        auto users = ui::list::ctor()
                            ->attach_collection(e2::form::prop::name, *usr_list, user_template);
                        return users;
                    };
                    {
                    auto window = client->attach(ui::cake::ctor());
                        auto taskbar_viewport = window->attach(ui::fork::ctor(axis::X))
                                                ->invoke([&](auto& boss)
                                                {
                                                    boss.broadcast->SUBMIT(tier::request, e2::form::prop::viewport, viewport)
                                                    {
                                                        viewport = boss.base::area();
                                                    };
                                                });
                        auto taskbar = taskbar_viewport->attach(slot::_1, ui::fork::ctor(axis::Y))
                                            ->colors(whitedk, 0x60202020)
                                            ->plugin<pro::limit>(twod{ uibar_min_size,-1 }, twod{ uibar_min_size,-1 })
                                            ->plugin<pro::timer>()
                                            ->plugin<pro::acryl>()
                                            ->plugin<pro::cache>()
                                            ->invoke([&](auto& boss)
                                            {
                                                boss.mouse.template draggable<sysmouse::left>();
                                                boss.SUBMIT(tier::release, e2::form::drag::pull::_<sysmouse::left>, gear)
                                                {
                                                    auto& limits = boss.template plugins<pro::limit>();
                                                    auto lims = limits.get();
                                                    lims.min.x += gear.delta.get().x;
                                                    lims.max.x = uibar_full_size = lims.min.x;
                                                    limits.set(lims.min, lims.max);
                                                    boss.base::reflow();
                                                };
                                                boss.SUBMIT(tier::release, e2::form::state::mouse, active)
                                                {
                                                    auto apply = [&](auto active)
                                                    {
                                                        auto& limits = boss.template plugins<pro::limit>();
                                                        auto size = active ? uibar_full_size : std::min(uibar_full_size, uibar_min_size);
                                                        auto lims = twod{ size,-1 };
                                                        limits.set(lims, lims);
                                                        boss.base::reflow();
                                                        return faux; // One shot call.
                                                    };
                                                    auto& timer = boss.template plugins<pro::timer>();
                                                    timer.pacify(faux);
                                                    if (active) apply(true);
                                                    else timer.actify(faux, MENU_TIMEOUT, apply);
                                                };
                                                boss.broadcast->SUBMIT(tier::request, e2::form::prop::viewport, viewport)
                                                {
                                                    viewport.coor.x += uibar_min_size;
                                                    viewport.size.x -= uibar_min_size;
                                                };
                                            });
                            auto apps_users = taskbar->attach(slot::_1, ui::fork::ctor(axis::Y, 0, 100));
                            {
                                const static auto c3 = app::shared::c3;
                                const static auto x3 = app::shared::x3;
                                const static auto c6 = app::shared::c6;
                                const static auto x6 = app::shared::x6;

                                auto apps_area = apps_users->attach(slot::_1, ui::fork::ctor(axis::Y));
                                {
                                    auto label_pads = apps_area->attach(slot::_1, ui::pads::ctor(dent{ 0,0,1,1 }, dent{ 0,0,0,0 }))
                                                               ->plugin<pro::fader>(x3, c3, 150ms);
                                        auto label_bttn = label_pads->attach(ui::fork::ctor());
                                            auto label = label_bttn->attach(slot::_1,
                                                ui::item::ctor(ansi::fgc(whitelt).add("  ≡ "), faux, faux));
                                            auto bttn_area = label_bttn->attach(slot::_2, ui::fork::ctor());
                                                auto bttn_pads = bttn_area->attach(slot::_2, ui::pads::ctor(dent{ 2,2,0,0 }, dent{ 0,0,1,1 }))
                                                                          ->plugin<pro::fader>(x6, c6, 150ms);
                                                    auto bttn = bttn_pads->attach(ui::item::ctor(">", faux));
                                    auto applist_area = apps_area->attach(slot::_2, ui::pads::ctor(dent{ 0,0,1,0 }, dent{}))
                                                                 ->attach(ui::cake::ctor());
                                        auto task_menu_area = applist_area->attach(ui::fork::ctor(axis::Y, 0, 0));
                                            auto menu_scrl = task_menu_area->attach(slot::_1, ui::rail::ctor(axes::ONLY_Y))
                                                                           ->colors(0x00, 0x00); //todo mouse events passthrough
                                                auto menuitems = menu_scrl->attach_element(e2::bindings::list::apps, world, menuitems_template);
                                            auto tasks_scrl = task_menu_area->attach(slot::_2, ui::rail::ctor(axes::ONLY_Y))
                                                                            ->colors(0x00, 0x00); //todo mouse events passthrough
                                                auto apps = tasks_scrl->attach_element(e2::bindings::list::apps, world, apps_template);
                                    label_pads->invoke([&](auto& boss)
                                                {
                                                    auto task_menu_area_shadow = ptr::shadow(task_menu_area);
                                                    auto bttn_shadow = ptr::shadow(bttn);
                                                    boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::click::left, gear)
                                                    {
                                                        if (auto bttn = bttn_shadow.lock())
                                                        if (auto task_menu_area = task_menu_area_shadow.lock())
                                                        {
                                                            auto state = task_menu_area->get_ratio();
                                                            bttn->set(state ? ">" : "<");
                                                            if (state) task_menu_area->config(0, 1);
                                                            else       task_menu_area->config(1, 0);
                                                            gear.dismiss();
                                                        }
                                                    };
                                                });
                                    apps_area->invoke([&](auto& boss)
                                                {
                                                    auto task_menu_area_shadow = ptr::shadow(task_menu_area);
                                                    auto bttn_shadow = ptr::shadow(bttn);
                                                    boss.SUBMIT_BYVAL(tier::release, e2::form::state::mouse, active)
                                                    {
                                                        if (!active)
                                                        if (auto bttn = bttn_shadow.lock())
                                                        if (auto task_menu_area = task_menu_area_shadow.lock())
                                                        {
                                                            if (auto state = task_menu_area->get_ratio())
                                                            {
                                                                bttn->set(">");
                                                                task_menu_area->config(0);
                                                            }
                                                        }
                                                    };
                                                });
                                    //todo make some sort of highlighting at the bottom and top
                                    //scroll_bars_left(items_area, items_scrl);
                                }
                                auto users_area = apps_users->attach(slot::_2, ui::fork::ctor(axis::Y));
                                {
                                    auto label_pads = users_area->attach(slot::_1, ui::pads::ctor(dent{ 0,0,1,1 }, dent{ 0,0,0,0 }))
                                                                ->plugin<pro::fader>(x3, c3, 150ms);
                                        auto label_bttn = label_pads->attach(ui::fork::ctor());
                                            auto label = label_bttn->attach(slot::_1,
                                                ui::item::ctor(ansi::fgc(whitelt).add("Users"), faux, faux));
                                            auto bttn_area = label_bttn->attach(slot::_2, ui::fork::ctor());
                                                auto bttn_pads = bttn_area->attach(slot::_2, ui::pads::ctor(dent{ 2,2,0,0 }, dent{ 0,0,1,1 }))
                                                                          ->plugin<pro::fader>(x6, c6, 150ms);
                                                    auto bttn = bttn_pads->attach(ui::item::ctor("<", faux));
                                    auto userlist_area = users_area->attach(slot::_2, ui::pads::ctor())
                                                                   ->plugin<pro::limit>();
                                        auto users = userlist_area->attach_element(e2::bindings::list::users, world, branch_template);
                                        //auto users_rail = userlist_area->attach(ui::rail::ctor());
                                        //auto users = users_rail->attach_element(e2::bindings::list::users, world, branch_template);
                                    //todo unify
                                    bttn_pads->invoke([&](auto& boss)
                                                {
                                                    auto userlist_area_shadow = ptr::shadow(userlist_area);
                                                    auto bttn_shadow = ptr::shadow(bttn);
                                                    boss.SUBMIT_BYVAL(tier::release, hids::events::mouse::button::click::left, gear)
                                                    {
                                                        static bool state = faux;
                                                        if (auto bttn = bttn_shadow.lock())
                                                        if (auto userlist = userlist_area_shadow.lock())
                                                        {
                                                            state = !state;
                                                            bttn->set(state ? ">" : "<");
                                                            auto& limits = userlist->plugins<pro::limit>();
                                                            auto lims = limits.get();
                                                            lims.min.y = lims.max.y = state ? 0 : -1;
                                                            limits.set(lims, true);
                                                            userlist->base::reflow();
                                                        }
                                                    };
                                                });
                                }
                            }
                            auto bttns_area = taskbar->attach(slot::_2, ui::fork::ctor(axis::X));
                            {
                                const static auto c2 = app::shared::c2;
                                const static auto x2 = app::shared::x2;
                                const static auto c1 = app::shared::c1;
                                const static auto x1 = app::shared::x1;

                                auto bttns = bttns_area->attach(slot::_1, ui::fork::ctor(axis::X));
                                    auto disconnect_area = bttns->attach(slot::_1, ui::pads::ctor(dent{ 2,3,1,1 }))
                                                                ->plugin<pro::fader>(x2, c2, 150ms)
                                                                ->invoke([&](auto& boss)
                                                                {
                                                                    boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                                                                    {
                                                                        if (auto owner = base::getref(gear.id))
                                                                        {
                                                                            owner->SIGNAL(tier::release, e2::conio::quit, "taskbar: logout by button");
                                                                            gear.dismiss();
                                                                        }
                                                                    };
                                                                });
                                        auto disconnect = disconnect_area->attach(ui::item::ctor("× Disconnect"));
                                    auto shutdown_area = bttns->attach(slot::_2, ui::pads::ctor(dent{ 2,3,1,1 }))
                                                              ->plugin<pro::fader>(x1, c1, 150ms)
                                                              ->invoke([&](auto& boss)
                                                              {
                                                                  boss.SUBMIT(tier::release, hids::events::mouse::button::click::left, gear)
                                                                  {
                                                                      //todo unify, see system.h:1614
                                                                      #if defined(__APPLE__) || defined(__FreeBSD__)
                                                                      auto path2 = "/tmp/" + path + ".sock";
                                                                      ::unlink(path2.c_str());
                                                                      #endif
                                                                      os::exit(0, "taskbar: shutdown by button");
                                                                  };
                                                              });
                                        auto shutdown = shutdown_area->attach(ui::item::ctor("× Shutdown"));
                            }
                    }
                    client->color(app::shared::background_color.fgc(), app::shared::background_color.bgc());
                    text header = username;
                    text footer = ansi::mgr(1).mgl(1).add(MONOTTY_VER);
                    client->SIGNAL(tier::release, e2::form::prop::name, header);
                    client->SIGNAL(tier::preview, e2::form::prop::header, header);
                    client->SIGNAL(tier::preview, e2::form::prop::footer, footer);
                    client->base::moveby(user_coor);
                    lock.reset();
                    log("user: new gate for ", peer);

                    #ifndef PROD
                    client->proceed(peer, _region);
                    #else
                    client->proceed(peer, username);
                    #endif

                    lock = std::unique_ptr<events::sync>();
                    log("user: ", peer, " has logged out");
                    client->detach();
                    log("user: ", peer, " is diconnected");
                    log("user: client.use_count() ", client.use_count());
                    client.reset();
                    lock.reset();
                } }.detach();

                log("main: new thread constructed for ", peer);
            }

            world->SIGNAL(tier::general, e2::config::fps, 0);
        }
    }
    os::exit(0, "bye!");
}