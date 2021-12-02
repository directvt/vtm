// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#define MONOTTY_VER "Monotty Desktopio v0.5.9999b"

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
    utf::text config;
    {
        std::ifstream conf;
        conf.open("vtm.conf");
        if (conf.is_open()) std::getline(conf, config);
        if (config.empty()) config = "empty config";

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

    world->SUBMIT(tier::release, e2::form::proceed::createat, what)
    {
        auto& config = app::shared::objs_config[what.menu_item_id];
        auto  window = ui::cake::ctor()
            ->plugin<pro::title>(config.title)
            ->plugin<pro::limit>(dot_11, twod{ 400,200 }) //todo unify, set via config
            ->plugin<pro::sizer>()
            ->plugin<pro::frame>()
            ->plugin<pro::light>()
            ->plugin<pro::align>()
            ->invoke([&](auto& boss)
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
            });

        window->extend(what.location);
        auto& creator = app::shared::creator(config.type);
        window->attach(creator(config.data));
        log(" world create type=", config.type, " menu_item_id=", what.menu_item_id);
        world->branch(what.menu_item_id, window);
        window->broadcast->SIGNAL(tier::release, e2::form::upon::started, world);

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
            
                    client->attach(deskmenu);
                    client->color(app::shared::background_color.fgc(), app::shared::background_color.bgc());
                    text header = username;
                    text footer = ansi::mgr(1).mgl(1).add(MONOTTY_VER);
                    client->SIGNAL(tier::release, e2::form::prop::name, header);
                    client->SIGNAL(tier::preview, e2::form::prop::header, header);
                    client->SIGNAL(tier::preview, e2::form::prop::footer, footer);
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