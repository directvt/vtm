// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#define MONOTTY_VER "Monotty Desktopio v0.5.9999h"

// Enable demo apps and assign Esc key to log off.
//#define DEMO

// Enable keyboard input and unassign Esc key.
#define PROD

// Tiling limits.
#ifndef PROD
    #define INHERITANCE_LIMIT 12
    #define APPS_MAX_COUNT    20
    #define TILE_MAX_COUNT    2
#else
    #define INHERITANCE_LIMIT 30
#endif
#define APPS_DEL_TIMEOUT  1s

// Enable to show debug overlay.
//#define DEBUG_OVERLAY

// Enable to show all terminal input (keyboard/mouse etc).
//#define KEYLOG

// Highlight region ownership.
//#define REGIONS

#include "netxs/apps.hpp"

#include <fstream> // Get current config from vtm.conf.

using namespace netxs::console;
using namespace netxs;

int main(int argc, char* argv[])
{
    netxs::logger logger([&](auto&& data) { os::syslog(data); });

    auto banner = [&]() { log(MONOTTY_VER); };
    auto client = true;
    auto daemon = faux;
    {
        auto getopt = os::args{ argc, argv };
        while (getopt)
        {
            switch (getopt.next())
            {
                case 's': client = faux; break;
                case 'd': daemon = true; break;
                default:
                    #ifndef PROD

                        if (os::get_env("SHELL").ends_with("vtm"))
                        {
                            auto error = utf::text{ "main: interactive server is not allowed in demo mode" };
                            if (argc > 1)
                            {
                                auto host = os::get_env("SSH_CLIENT");
                                auto name = os::get_env("USER");
                                error += "\nblock explicit shell command invocation {" + name + ", " + host + "}";
                                for (auto i = 1; i < argc; i++)
                                {
                                    error += '\n';
                                    error += utf::text(argv[i]);
                                }
                            }
                            os::start_log("vtm");
                            os::exit(1, error);
                        }

                    #endif

                    banner();
                    log("Usage:\n\n ", argv[0], " [OPTION...]\n\n"s
                                    + " No arguments\tRun client, auto start server if is not started.\n"s
                                             + "\t-d\tRun server in background.\n"s
                                             + "\t-s\tRun server in interactive mode.\n"s);
                    os::exit(1);
            }
        }

        if (daemon)
        {
            if (!os::daemonize(argv[0]))
            {
                banner();
                os::exit(1, "main: failed to daemonize");
            }
            else client = faux;
        }
    }

    banner();

    if (client)
    {
        os::start_log("vtm");

        auto host = os::get_env("SSH_CLIENT");
        auto name = os::get_env("USER");

        // Demo: Get current region from "~/.config/vtm/settings.ini".
        utf::text spot;
        {
            std::ifstream config;
            config.open(os::homepath() + "/.config/vtm/settings.ini");

            if (config.is_open())
                std::getline(config, spot);

            if (spot.empty())
                spot = "unknown region";
        }

        auto user = os::user();
        auto path = utf::concat("monotty_", user); //todo unify, use vtm.conf
        auto link = os::ipc::open<os::client>(path, 10s, [&]()
                    {
                        log("main: new desktop environment for user ", user);
                        auto binary = view{ argv[0] };
                        utf::trim_front(binary, "-"); // Sometimes "-" appears before executable.
                        return os::exec(text{ binary }, "-d");
                    });
        if (!link) os::exit(-1, "main: desktop server connection error");

        auto mode = os::legacy_mode();
        link->send(utf::concat(spot, ";",
                               host, ";",
                               name, ";",
                               user, ";",
                               mode, ";"));
        auto gate = os::tty::proxy(link);
        gate.ignite();
        gate.output(ansi::esc{}.save_title()
                               .altbuf(true)
                               .vmouse(true)
                               .cursor(faux)
                               .bpmode(true)
                               .setutf(true));
        gate.splice(mode);
        gate.output(ansi::esc{}.scrn_reset()
                               .vmouse(faux)
                               .cursor(true)
                               .altbuf(faux)
                               .bpmode(faux)
                               .load_title());

        std::this_thread::sleep_for(200ms); // Pause to complete consuming/receiving buffered input (e.g. mouse tracking) that has just been canceled.
        os::exit(0);
    }
    
    netxs::logger srv_logger( [=](auto const& utf8)
    {
        static text buff;
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

    //todo Get current config from "~/.config/vtm/settings.ini".
    utf::text config_data;
    {
        std::ifstream config;
        config.open(os::homepath() + "/.config/vtm/settings.ini");
        if (config.is_open()) std::getline(config, config_data);
        if (config_data.empty()) config_data = "empty config";

        //todo unify
        //skin::setup(tone::lucidity, 192);
        //skin::setup(tone::shadower, 0);
        skin::setup(tone::kb_focus, 60);
        skin::setup(tone::brighter, 60);//120);
        skin::setup(tone::shadower, 180);//60);//40);// 20);
        skin::setup(tone::shadow  , 180);//5);
        skin::setup(tone::lucidity, 255);
        skin::setup(tone::selector, 48);
        skin::setup(tone::bordersz, dot_11);
    }

    auto world = base::create<host>([&](auto reason) { os::exit(0, reason); });

    log("host: created");

    world->SUBMIT(tier::general, e2::cleanup, counter)
    {
        world->router<tier::general>().cleanup(counter.ref_count, counter.del_count);
    };

    auto base_window = [](auto header, auto footer, auto menu_item_id)
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
                    auto& area = boss.base::area();
                    if (!area.size.inside(gear.coord))
                    {
                        auto center = area.coor + (area.size / 2);
                        bell::getref(gear.id)->SIGNAL(tier::release, e2::form::layout::shift, center);
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
                boss.SUBMIT(tier::release, e2::dtor, p)
                {
                    auto start = tempus::now();
                    auto counter = decltype(e2::cleanup)::type{};
                    SIGNAL_GLOBAL(e2::cleanup, counter);
                    auto stop = tempus::now() - start;
                    log("world: Garbage collection",
                    "\n\ttime ", utf::format(stop.count()), "ns",
                    "\n\tobjs ", counter.obj_count,
                    "\n\trefs ", counter.ref_count,
                    "\n\tdels ", counter.del_count);
                };
            });
    };

    world->SUBMIT(tier::release, e2::form::proceed::createat, what)
    {
        auto& config = app::shared::objs_config[what.menuid];
        auto window = base_window(config.title, "", what.menuid);

        window->extend(what.square);
        auto& creator = app::shared::creator(config.group);
        window->attach(creator(config.param));
        log("world: create type=", config.group, " menu_item_id=", what.menuid);
        world->branch(what.menuid, window, config.fixed);
        window->SIGNAL(tier::anycast, e2::form::upon::started, world);

        what.object = window;
    };
    world->SUBMIT(tier::release, e2::form::proceed::createfrom, what)
    {
        auto& config = app::shared::objs_config[what.menuid];
        auto window = base_window(what.header, what.footer, what.menuid);

        window->extend(what.square);
        window->attach(what.object);
        log("world: attach type=", config.group, " menu_item_id=", what.menuid);
        world->branch(what.menuid, window, config.fixed);
        window->SIGNAL(tier::anycast, e2::form::upon::started, world);

        what.object = window;
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

    app::shared::init_menu(world);

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

            std::thread{ [=]    // _name
                                // _mode
                                // _region
                                // peer
                                // c_ip
                                // c_port
                                // world
                                // user_coor
                                // 
            {
                log("user: session name ", peer);

                #ifndef PROD
                    auto username = "[User." + utf::remain(c_ip) + ":" + c_port + "]";
                #else
                    auto username = _name;
                #endif

                auto lock = std::make_unique<events::sync>();
                    auto legacy_mode = os::legacy::clean;
                    if (auto mode = utf::to_int(view(_mode)))
                    {
                        legacy_mode = mode.value();
                    }
                    auto client = world->invite<ui::gate>(username, legacy_mode);

                    auto& menu_builder = app::shared::creator("Desk");
                    auto deskmenu = menu_builder(utf::concat(client->id, ";", user, ";", path));
                    auto& fone_builder = app::shared::creator("Fone");
                    auto bkground = fone_builder(
                    #ifndef PROD
                        "Shop;Demo;"
                    #else
                        "HeadlessTerm;Demo;ssh demo@netxs.online"
                    #endif
                    );
            
                    client->attach(deskmenu);
                    client->ground(bkground);
                    client->color(app::shared::background_color.fgc(), app::shared::background_color.bgc());
                    text header = username;
                    client->SIGNAL(tier::release, e2::form::prop::name, header);
                    client->SIGNAL(tier::preview, e2::form::prop::header, header);
                    client->base::moveby(user_coor);
                lock.reset();
                log("user: new gate for ", peer);

                client->proceed(peer,
                    #ifndef PROD
                        _region
                    #else
                        username
                    #endif
                );

                lock = std::make_unique<events::sync>();
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

    os::exit(0, "bye!");
}