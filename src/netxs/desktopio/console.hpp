// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#include "controls.hpp"

namespace netxs::ui
{
    namespace console
    {
        static auto id = std::pair<ui32, time>{};
        static constexpr auto mouse   = 1 << 0;
        static constexpr auto onlylog = 1 << 5;
        //todo make 3-bit field for color mode
        static constexpr auto vtrgb   = 0;
        static constexpr auto nt16    = 1 << 1;
        static constexpr auto vt16    = 1 << 2;
        static constexpr auto vt256   = 1 << 3;
        static constexpr auto direct  = 1 << 4;

        template<class T>
        auto str(T mode)
        {
            auto result = text{};
            if (mode)
            {
                if (mode & mouse  ) result += "mouse ";
                if (mode & nt16   ) result += "nt16 ";
                if (mode & vt16   ) result += "vt16 ";
                if (mode & vt256  ) result += "vt256 ";
                if (mode & direct ) result += "direct ";
                if (mode & onlylog) result += "onlylog ";
                if (result.size()) result.pop_back();
            }
            else result = "vtrgb";
            return result;
        }
    }

    struct pipe;
    using xipc = netxs::sptr<pipe>;

    // console: Fullduplex channel base.
    struct pipe
    {
        flag active; // pipe: Is connected.
        flag isbusy; // pipe: Buffer is still busy.

        pipe(bool active)
            : active{ active },
              isbusy{ faux   }
        { }
        virtual ~pipe()
        { }

        operator bool () const { return active; }

        void start()
        {
            active.exchange(true);
            isbusy.exchange(faux);
        }
        virtual bool send(view buff) = 0;
        virtual qiew recv(char* buff, size_t size) = 0;
        virtual qiew recv() = 0;
        virtual bool shut()
        {
            return active.exchange(faux);
        }
        virtual bool stop()
        {
            return pipe::shut();
        }
        virtual void wake()
        {
            shut();
        }
        virtual std::ostream& show(std::ostream& s) const = 0;
        void output(view data)
        {
            send(data);
        }
        friend auto& operator << (std::ostream& s, pipe const& sock)
        {
            return sock.show(s << "{ " << prompt::xipc) << " }";
        }
        friend auto& operator << (std::ostream& s, xipc const& sock)
        {
            return s << *sock;
        }
        void cleanup()
        {
            active.exchange(faux);
            isbusy.exchange(faux);
        }
    };

    // console: Client gate.
    class gate
        : public form<gate>
    {
        // gate: Data decoder.
        struct link
            : public s11n
        {
            pipe& canal; // link: Data highway.
            base& owner; // link: Link owner.
            flag  alive; // link: sysclose isn't sent.

            // link: Send data outside.
            void run()
            {
                directvt::binary::stream::reading_loop(canal, [&](view data){ s11n::sync(data); });
                s11n::stop(); // Wake up waiting objects, if any.
                if constexpr (debugmode) log(prompt::gate, "DirectVT session complete");
            }
            // link: Notify environment to disconnect.
            void disconnect()
            {
                if (alive.exchange(faux))
                {
                    s11n::sysclose.send(canal, true);
                    canal.wake();
                }
            }

            link(pipe& canal, base& owner)
                : s11n{ *this },
                 canal{ canal },
                 owner{ owner },
                 alive{ true  }
            { }

            // link: Send an event message to the link owner.
            template<tier Tier = tier::release, class E, class T>
            void notify(E, T&& data)
            {
                netxs::events::enqueue(owner.This(), [d = data](auto& boss) mutable
                {
                    //boss.SIGNAL(Tier, E{}, d); // VS2022 17.4.1 doesn't get it for some reason (nested lambdas + static_cast + decltype(...)::type).
                    boss.bell::template signal<Tier>(E::id, static_cast<typename E::type &&>(d));
                });
            }
            void handle(s11n::xs::focusbus    lock)
            {
                auto& focus = lock.thing;
                auto deed = netxs::events::makeid(hids::events::keybd::focus::bus::any.id, focus.cause);
                if (focus.guid != ui::console::id.second || deed != hids::events::keybd::focus::bus::copy.id) // To avoid focus tree infinite looping.
                netxs::events::enqueue(owner.This(), [d = focus, deed](auto& boss) mutable
                {
                    auto seed = hids::events::keybd::focus::bus::on.param({ .id = d.gear_id });
                    boss.bell::template signal<tier::release>(deed, seed);
                });
            }
            void handle(s11n::xs::sysfocus    lock)
            {
                auto& focus = lock.thing;
                notify(e2::conio::focus, focus);
            }
            void handle(s11n::xs::syswinsz    lock)
            {
                auto& item = lock.thing;
                notify(e2::conio::winsz, item.winsize);
            }
            void handle(s11n::xs::sysboard    lock)
            {
                auto& item = lock.thing;
                notify(e2::conio::board, item);
            }
            void handle(s11n::xs::logs        lock)
            {
                auto& logs = lock.thing;
                if (ui::console::id.first == lock.thing.id)
                {
                    notify<tier::general>(e2::conio::logs, logs.data);
                }
                else
                {
                    if (logs.data.size() && logs.data.back() == '\n') logs.data.pop_back();
                    if (logs.data.size())
                    {
                        auto data = escx{};
                        utf::divide(logs.data, '\n', [&](auto line)
                        {
                            data.add(netxs::prompt::pads, logs.id, ": ", line, '\n');
                        });
                        notify<tier::general>(e2::conio::logs, data);
                    }
                }
            }
            void handle(s11n::xs::syskeybd    lock)
            {
                auto& keybd = lock.thing;
                notify(e2::conio::keybd, keybd);
            }
            //void handle(s11n::xs::syspaste    lock)
            //{
            //    auto& paste = lock.thing;
            //    notify(e2::conio::paste, paste);
            //}
            void handle(s11n::xs::sysmouse    lock)
            {
                auto& mouse = lock.thing;
                notify(e2::conio::mouse, mouse);
            }
            void handle(s11n::xs::mousebar    lock)
            {
                auto& item = lock.thing;
                notify(e2::conio::pointer, item.mode);
            }
            void handle(s11n::xs::request_gc  lock)
            {
                auto& items = lock.thing;
                auto list = s11n::jgc_list.freeze();
                for (auto& gc : items)
                {
                    auto cluster = cell::gc_get_data(gc.token);
                    list.thing.push(gc.token, cluster);
                }
                list.thing.sendby(canal);
            }
            void handle(s11n::xs::fps         lock)
            {
                auto& item = lock.thing;
                notify(e2::config::fps, item.frame_rate);
            }
            void handle(s11n::xs::bgc         lock)
            {
                auto& item = lock.thing;
                notify<tier::anycast>(e2::form::prop::colors::bg, item.color);
            }
            void handle(s11n::xs::fgc         lock)
            {
                auto& item = lock.thing;
                notify<tier::anycast>(e2::form::prop::colors::fg, item.color);
            }
            void handle(s11n::xs::slimmenu    lock)
            {
                auto& item = lock.thing;
                notify<tier::anycast>(e2::form::prop::ui::slimmenu, item.menusize);
            }
            void handle(s11n::xs::sysclose    lock)
            {
                auto& item = lock.thing;
                auto backup = owner.This();
                notify<tier::anycast>(e2::form::proceed::quit::one, item.fast);
            }
        };

        // gate: Bitmap forwarder.
        struct diff
        {
            using work = std::thread;
            using lock = std::mutex;
            using cond = std::condition_variable_any;

            struct stat
            {
                span watch{}; // diff::stat: Duration of the STDOUT rendering.
                sz_t delta{}; // diff::stat: Last ansi-rendered frame size.
            };

            pipe& canal; // diff: Channel to outside.
            lock  mutex; // diff: Mutex between renderer and committer threads.
            cond  synch; // diff: Synchronization between renderer and committer.
            core  cache; // diff: The current content buffer which going to be checked and processed.
            flag  alive; // diff: Working loop state.
            flag  ready; // diff: Conditional variable to avoid spurious wakeup.
            flag  abort; // diff: Abort building current frame.
            work  paint; // diff: Rendering thread.
            stat  debug; // diff: Debug info.

            // diff: Render current buffer to the screen.
            template<class Bitmap>
            void render()
            {
                if constexpr (debugmode) log(prompt::diff, "Rendering thread started", ' ', utf::to_hex_0x(std::this_thread::get_id()));
                auto start = time{};
                auto image = Bitmap{};
                auto guard = std::unique_lock{ mutex };
                while ((void)synch.wait(guard, [&]{ return !!ready; }), alive)
                {
                    start = datetime::now();
                    ready = faux;
                    abort = faux;
                    auto winid = id_t{ 0xddccbbaa };
                    auto coord = dot_00;
                    image.set(winid, coord, cache, abort, debug.delta);
                    if (debug.delta)
                    {
                        canal.isbusy = true; // It's okay if someone resets the busy flag before sending.
                        image.sendby(canal);
                        canal.isbusy.wait(true); // Successive frames must be discarded until the current frame is delivered (to prevent unlimited buffer growth).
                    }
                    debug.watch = datetime::now() - start;
                }
                if constexpr (debugmode) log(prompt::diff, "Rendering thread ended", ' ', utf::to_hex_0x(std::this_thread::get_id()));
            }
            // diff: Get rendering statistics.
            auto status()
            {
                return debug;
            }
            // diff: Discard current frame.
            void cancel()
            {
                abort = true;
            }
            // diff: Obtain new content to render.
            auto commit(core const& canvas)
            {
                if (abort)
                {
                    while (alive) // Try to send a new frame as soon as possible (e.g. after resize).
                    {
                        auto lock = std::unique_lock{ mutex, std::try_to_lock };
                        if (lock.owns_lock())
                        {
                            cache = canvas;
                            ready = true;
                            synch.notify_one();
                            return true;
                        }
                        else std::this_thread::yield();
                    }
                }
                else
                {
                    auto lock = std::unique_lock{ mutex, std::try_to_lock };
                    if (lock.owns_lock())
                    {
                        cache = canvas;
                        ready = true;
                        synch.notify_one();
                        return true;
                    }
                }
                return faux;
            }

            diff(pipe& dest, svga vtmode)
                : canal{ dest },
                  alive{ true },
                  ready{ faux },
                  abort{ faux }
            {
                using namespace netxs::directvt;
                paint = work([&, vtmode]
                {
                         if (vtmode == svga::dtvt ) render<binary::bitmap_dtvt_t >();
                    else if (vtmode == svga::vtrgb) render<binary::bitmap_vtrgb_t>();
                    else if (vtmode == svga::vt256) render<binary::bitmap_vt256_t>();
                    else if (vtmode == svga::vt16 ) render<binary::bitmap_vt16_t >();
                    else if (vtmode == svga::nt16 ) render<binary::bitmap_dtvt_t >();
                });
            }
            void stop()
            {
                if (!alive.exchange(faux)) return;
                auto id = paint.get_id();
                while (true)
                {
                    auto guard = std::unique_lock{ mutex, std::try_to_lock };
                    if (guard.owns_lock())
                    {
                        ready = true;
                        synch.notify_all();
                        break;
                    }
                    canal.isbusy = faux;
                    canal.isbusy.notify_all();
                    std::this_thread::yield();
                }
                paint.join();
                if constexpr (debugmode) log(prompt::diff, "Rendering thread joined", ' ', utf::to_hex_0x(id));
            }
        };

        // gate: Application properties.
        struct props_t
        {
            //todo revise
            text os_user_id;
            text title;
            text selected;
            span clip_preview_time;
            cell clip_preview_clrs;
            byte clip_preview_alfa;
            bool clip_preview_show;
            twod clip_preview_size;
            si32 clip_preview_glow;
            cell background_color;
            face background_image;
            si32 legacy_mode;
            si32 session_id;
            span dblclick_timeout; // conf: Double click timeout.
            span tooltip_timeout; // conf: Timeout for tooltip.
            cell tooltip_colors; // conf: Tooltip rendering colors.
            bool tooltip_enabled; // conf: Enable tooltips.
            bool glow_fx; // conf: Enable glow effect in main menu.
            bool debug_overlay; // conf: Enable to show debug overlay.
            text debug_toggle; // conf: Debug toggle shortcut.
            bool show_regions; // conf: Highlight region ownership.
            bool simple; // conf: .
            svga vtmode; // conf: .

            void read(xmls& config)
            {
                config.cd("/config/client/");
                clip_preview_clrs = config.take("clipboard/preview"        , cell{}.bgc(bluedk).fgc(whitelt));
                clip_preview_time = config.take("clipboard/preview/timeout", span{ 3s });
                clip_preview_alfa = config.take("clipboard/preview/alpha"  , 0xFF);
                clip_preview_glow = config.take("clipboard/preview/shadow" , 7);
                clip_preview_show = config.take("clipboard/preview/enabled", true);
                clip_preview_size = config.take("clipboard/preview/size"   , twod{ 80,25 });
                dblclick_timeout  = config.take("mouse/dblclick"           , span{ 500ms });
                tooltip_colors    = config.take("tooltips"                 , cell{}.bgc(0xFFffffff).fgc(0xFF000000));
                tooltip_timeout   = config.take("tooltips/timeout"         , span{ 2000ms });
                tooltip_enabled   = config.take("tooltips/enabled"         , true);
                debug_overlay     = config.take("debug/overlay"            , faux);
                debug_toggle      = config.take("debug/toggle"             , "🐞"s);
                show_regions      = config.take("regions/enabled"          , faux);
                clip_preview_glow = std::clamp(clip_preview_glow, 0, 10);
            }

            props_t(pipe& canal, view userid, si32 mode, bool isvtm, si32 session_id, xmls& config)
            {
                read(config);
                legacy_mode = mode;
                if (isvtm)
                {
                    this->session_id  = session_id;
                    os_user_id        = utf::concat("[", userid, ":", session_id, "]");
                    title             = os_user_id;
                    selected          = config.take("/config/menu/selected", ""s);
                    background_color  = cell{}.fgc(config.take("background/fgc", rgba{ whitedk }))
                                              .bgc(config.take("background/bgc", rgba{ 0xFF000000 }));
                    auto utf8_tile = config.take("background/tile", ""s);
                    if (utf8_tile.size())
                    {
                        auto block = page{ utf8_tile };
                        background_image.size(block.limits());
                        background_image.output(block);
                    }
                    glow_fx           = config.take("glowfx", true);
                    simple            = faux;
                }
                else
                {
                    simple            = !(legacy_mode & ui::console::direct);
                    glow_fx           = faux;
                    title             = "";
                }
                vtmode = legacy_mode & ui::console::nt16   ? svga::nt16
                       : legacy_mode & ui::console::vt16   ? svga::vt16
                       : legacy_mode & ui::console::vt256  ? svga::vt256
                       : legacy_mode & ui::console::direct ? svga::dtvt
                                                           : svga::vtrgb;
            }

            friend auto& operator << (std::ostream& s, props_t const& c)
            {
                return s << "\n\tuser: " << c.os_user_id
                         << "\n\tmode: " << ui::console::str(c.legacy_mode);
            }
        };

        // gate: Input forwarder.
        struct input_t
        {
            using depo = std::unordered_map<id_t, netxs::sptr<hids>>;
            using lock = std::recursive_mutex;

            template<class T>
            void forward(T& device)
            {
                auto gear_it = gears.find(device.gear_id);
                if (gear_it == gears.end())
                {
                    gear_it = gears.emplace(device.gear_id, bell::create<hids>(boss.props, boss, xmap)).first;
                }
                auto& [_id, gear_ptr] = *gear_it;
                gear_ptr->hids::take(device);
                boss.strike();
            }

            gate& boss;
            subs  memo;
            face  xmap;
            lock  sync;
            depo  gears;

            input_t(props_t& props, gate& boss)
                : boss{ boss }
            {
                xmap.cmode = props.vtmode;
                xmap.mark(props.background_color.txt(whitespace).link(boss.bell::id));
                xmap.face::area(boss.base::area());
                boss.LISTEN(tier::release, e2::command::printscreen, gear, memo)
                {
                    auto data = escx{};
                    data.s11n(xmap, gear.slot);
                    if (data.length())
                    {
                        gear.set_clipboard(gear.slot.size, data, mime::ansitext);
                    }
                };
                boss.LISTEN(tier::release, e2::form::prop::filler, filler, memo)
                {
                    auto guard = std::lock_guard{ sync }; // Syncing with diff::render thread.
                    xmap.mark(filler);
                };
                boss.LISTEN(tier::release, e2::area, new_area, memo)
                {
                    auto guard = std::lock_guard{ sync }; // Syncing with diff::render thread.
                    xmap.face::area(new_area);
                };
                boss.LISTEN(tier::release, e2::conio::mouse, m, memo)
                {
                    if (m.enabled != hids::stat::ok)
                    {
                        auto gear_it = gears.find(m.gear_id);
                        if (gear_it != gears.end())
                        {
                            switch (m.enabled)
                            {
                                case hids::stat::ok:   break;
                                case hids::stat::halt: gear_it->second->deactivate(); break;
                                case hids::stat::die:  gears.erase(gear_it);          break;
                            }
                        }
                        boss.strike();
                    }
                    else forward(m);
                };
                boss.LISTEN(tier::release, e2::conio::keybd, k, memo)
                {
                    forward(k);
                };
                boss.LISTEN(tier::release, e2::conio::focus, f, memo)
                {
                    forward(f);
                };
                boss.LISTEN(tier::release, e2::conio::board, c, memo)
                {
                    forward(c);
                };
            }
            void fire(hint event_id)
            {
                for (auto& [id, gear_ptr] : gears)
                {
                    auto& gear = *gear_ptr;
                    gear.fire_fast();
                    gear.fire(event_id);
                }
            }
            auto get_foreign_gear_id(id_t gear_id)
            {
                for (auto& [foreign_id, gear_ptr] : gears)
                {
                    if (gear_ptr->id == gear_id) return std::pair{ foreign_id, gear_ptr };
                }
                return std::pair{ id_t{}, netxs::sptr<hids>{} };
            }
        };

        // gate: Realtime statistics.
        struct debug_t
        {
            #define prop_list                     \
            X(total_size   , "total sent"       ) \
            X(proceed_ns   , "rendering time"   ) \
            X(render_ns    , "stdout time"      ) \
            X(frame_size   , "frame size"       ) \
            X(frame_rate   , "frame rate"       ) \
            X(focused      , "focus"            ) \
            X(win_size     , "win size"         ) \
            X(key_code     , "key virt"         ) \
            X(key_scancode , "key scan"         ) \
            X(key_character, "key char"         ) \
            X(key_pressed  , "key push"         ) \
            X(ctrl_state   , "controls"         ) \
            X(mouse_pos    , "mouse coord"      ) \
            X(mouse_wheeldt, "wheel delta"      ) \
            X(mouse_hzwheel, "H wheel"          ) \
            X(mouse_vtwheel, "V wheel"          ) \
            X(mouse_btn_1  , "left button"      ) \
            X(mouse_btn_2  , "right button"     ) \
            X(mouse_btn_3  , "middle button"    ) \
            X(mouse_btn_4  , "4th button"       ) \
            X(mouse_btn_5  , "5th button"       ) \
            X(mouse_btn_6  , "left+right combo" ) \
            X(last_event   , "event"            )

            #define X(a, b) a,
            enum prop { prop_list count };
            #undef X

            #define X(a, b) b,
            text description[prop::count] = { prop_list };
            #undef X
            #undef prop_list

            base& boss;
            subs  tokens;
            cell  alerts;
            cell  stress;
            page  status;
            escx  coder;
            bool  bypass = faux;

            struct
            {
                span render = span::zero();
                span output = span::zero();
                si32 frsize = 0;
                si64 totals = 0;
                si32 number = 0;    // info: Current frame number
                //bool   onhold = faux; // info: Indicator that the current frame has been successfully STDOUT
            }
            track; // debug: Textify the telemetry data for debugging purpose.

            void shadow()
            {
                for (auto i = 0; i < prop::count; i++)
                {
                    status[i].ease();
                }
            }

            debug_t(base& boss)
                : boss{ boss }
            { }

            operator bool () const { return tokens.count(); }

            void update(bool focus_state)
            {
                shadow();
                status[prop::last_event].set(stress) = "focus";
                status[prop::focused].set(stress) = focus_state ? "active" : "lost";
            }
            void update(twod new_size)
            {
                shadow();
                status[prop::last_event].set(stress) = "size";

                status[prop::win_size].set(stress) =
                    std::to_string(new_size.x) + " x " +
                    std::to_string(new_size.y);
            }
            void update(span watch, si32 delta)
            {
                track.output = watch;
                track.frsize = delta;
                track.totals+= delta;
            }
            void update(time timestamp)
            {
                track.render = datetime::now() - timestamp;
            }
            void output(face& canvas)
            {
                status[prop::render_ns].set(track.output > 12ms ? alerts : stress) =
                    utf::adjust(utf::format(track.output.count()), 11, " ", true) + "ns";

                status[prop::proceed_ns].set(track.render > 12ms ? alerts : stress) =
                    utf::adjust(utf::format (track.render.count()), 11, " ", true) + "ns";

                status[prop::frame_size].set(stress) =
                    utf::adjust(utf::format(track.frsize), 7, " ", true) + " bytes";

                status[prop::total_size].set(stress) =
                    utf::format(track.totals) + " bytes";

                track.number++;
                canvas.output(status);
            }
            void stop()
            {
                track = {};
                tokens.clear();
            }
            void start()
            {
                //todo use skin
                stress = cell{}.fgc(whitelt);
                alerts = cell{}.fgc(rgba{ 0xFFd0d0FFu });

                status.style.wrp(wrap::on).jet(bias::left).rlf(feed::rev).mgl(4);
                status.current().locus.cup(dot_00).cnl(2);

                auto maxlen = 0_sz;
                for (auto& desc : description)
                {
                    maxlen = std::max(maxlen, desc.size());
                }
                auto attr = si32{ 0 };
                for (auto& desc : description)
                {
                    status += coder.add(" ", utf::adjust(desc, maxlen, " ", true), " ").idx(attr++).nop().nil().eol();
                    coder.clear();
                }

                boss.LISTEN(tier::general, e2::config::fps, fps, tokens)
                {
                    status[prop::frame_rate].set(stress) = std::to_string(fps);
                    boss.base::strike();
                };
                boss.SIGNAL(tier::general, e2::config::fps, e2::config::fps.param(-1));
                boss.LISTEN(tier::release, e2::conio::focus, f, tokens)
                {
                    update(f.state);
                    boss.base::strike();
                };
                boss.LISTEN(tier::release, e2::area, new_area, tokens)
                {
                    update(new_area.size);
                };
                boss.LISTEN(tier::release, e2::conio::mouse, m, tokens)
                {
                    if (bypass) return;
                    shadow();
                    status[prop::last_event].set(stress) = "mouse";
                    status[prop::mouse_pos ].set(stress) =
                        (m.coordxy.x < 10000 ? std::to_string(m.coordxy.x) : "-") + " : " +
                        (m.coordxy.y < 10000 ? std::to_string(m.coordxy.y) : "-") ;

                    auto m_buttons = std::bitset<8>(m.buttons);
                    for (auto i = 0; i < hids::numofbuttons; i++)
                    {
                        auto& state = status[prop::mouse_btn_1 + i].set(stress);
                        state = m_buttons[i] ? "pressed" : "idle   ";
                    }

                    status[prop::mouse_wheeldt].set(stress) = m.wheeldt ? std::to_string(m.wheeldt) :  " -- "s;
                    status[prop::mouse_hzwheel].set(stress) = m.hzwheel ? "active" : "idle  ";
                    status[prop::mouse_vtwheel].set(stress) = m.wheeled ? "active" : "idle  ";
                    status[prop::ctrl_state   ].set(stress) = "0x" + utf::to_hex(m.ctlstat);
                };
                boss.LISTEN(tier::release, e2::conio::keybd, k, tokens)
                {
                    shadow();
                    status[prop::last_event   ].set(stress) = "keybd";
                    status[prop::key_pressed  ].set(stress) = k.pressed ? "pressed" : "idle";
                    status[prop::ctrl_state   ].set(stress) = "0x" + utf::to_hex(k.ctlstat );
                    status[prop::key_code     ].set(stress) = "0x" + utf::to_hex(k.virtcod );
                    status[prop::key_scancode ].set(stress) = "0x" + utf::to_hex(k.scancod );

                    if (k.cluster.length())
                    {
                        auto t = k.cluster;
                        for (auto i = 0; i < 0x20; i++)
                        {
                            utf::change(t, text{ (char)i }, "^" + utf::to_utf_from_code(i + 0x40));
                        }
                        utf::change(t, text{ (char)0x7f }, "\\x7F");
                        utf::change(t, text{ (char)0x20 }, "\\x20");
                        status[prop::key_character].set(stress) = t;
                    }
                };
                boss.LISTEN(tier::release, e2::conio::error, e, tokens)
                {
                    shadow();
                    status[prop::last_event].set(stress) = "error";
                    throw;
                };
            }
        };

    public:
        pipe&      canal; // gate: Channel to outside.
        bool       yield; // gate: Indicator that the current frame has been successfully STDOUT'd.
        para       uname; // gate: Client name.
        text       uname_txt; // gate: Client name (original).
        props_t    props; // gate: Application properties.
        input_t    input; // gate: Input event handler.
        debug_t    debug; // gate: Statistics monitor.
        sptr       applet; // gate: Standalone application.
        diff       paint; // gate: Render.
        link       conio; // gate: Input data parser.
        subs       tokens; // gate: Subscription tokens.
        bool       direct; // gate: .
        bool       local; // gate: .
        wptr       nexthop; // gate: .
        hook       oneoff_focus; // gate: .

        void draw_foreign_names(face& parent_canvas)
        {
            auto& header = *uname.lyric;
            auto  half_x = header.size().x / 2;
            for (auto& [id, gear_ptr] : input.gears)
            {
                auto& gear = *gear_ptr;
                if (gear.disabled) continue;
                auto coor = gear.coord;
                coor.y -= 1;
                coor.x -= half_x;
                header.move(coor);
                parent_canvas.fill(header, cell::shaders::fuse);
            }
        }
        void draw_mouse_pointer(face& canvas)
        {
            static const auto idle = cell{}.txt("\xE2\x96\x88"/*\u2588 █ */).bgc(0x00).fgc(0xFF00ff00);
            static const auto busy = cell{}.bgc(reddk).fgc(0xFFffffff);
            auto area = rect_11;
            for (auto& [id, gear_ptr] : input.gears)
            {
                auto& gear = *gear_ptr;
                if (gear.disabled) continue;
                area.coor = gear.coord;
                auto brush = gear.m.buttons ? cell{ busy }.txt(64 + gear.m.buttons/*A-Z*/)
                                            : idle;
                canvas.fill(area, cell::shaders::fuse(brush));
            }
        }
        void draw_clipboard_preview(face& canvas, time const& stamp)
        {
            for (auto& [id, gear_ptr] : input.gears)
            {
                auto& gear = *gear_ptr;
                gear.board::shown = !gear.disabled &&
                                    (props.clip_preview_time == span::zero() ||
                                     props.clip_preview_time > stamp - gear.delta.stamp());
                if (gear.board::shown)
                {
                    auto coor = gear.coord + dot_21 * 2;
                    auto full = gear.board::image.full();
                    gear.board::image.move(coor - full.coor);
                    canvas.plot(gear.board::image, cell::shaders::mix);
                }
            }
        }
        void draw_tooltips(face& canvas, time const& stamp)
        {
            auto full = canvas.full();
            auto area = canvas.area();
            auto zero = rect{ dot_00, area.size };
            canvas.area(zero);
            for (auto& [id, gear_ptr] : input.gears)
            {
                auto& gear = *gear_ptr;
                if (gear.disabled) continue;
                if (gear.tooltip_enabled(stamp))
                {
                    auto [tooltip_data, tooltip_update] = gear.get_tooltip();
                    if (tooltip_data)
                    {
                        //todo optimize - cache tooltip_page
                        auto tooltip_page = page{ tooltip_data };
                        auto area = full;
                        area.coor = std::max(dot_00, gear.coord - twod{ 4, tooltip_page.size() + 1 });
                        area.size.x = dot_mx.x; // Prevent line wrapping.
                        canvas.full(area);
                        canvas.cup(dot_00);
                        canvas.output(tooltip_page, cell::shaders::color(props.tooltip_colors));
                    }
                }
            }
            canvas.area(area);
            canvas.full(full);
        }
        void send_tooltips()
        {
            auto list = conio.tooltips.freeze();
            for (auto& [gear_id, gear_ptr] : input.gears /* use filter gear.is_tooltip_changed()*/)
            {
                auto& gear = *gear_ptr;
                if (gear.disabled) continue;
                if (gear.is_tooltip_changed())
                {
                    auto [tooltip_data, tooltip_update] = gear.get_tooltip();
                    list.thing.push(gear_id, tooltip_data, tooltip_update);
                }
            }
            list.thing.sendby<true>(canal);
        }
        void check_tooltips(time now)
        {
            auto result = faux;
            for (auto& [gear_id, gear_ptr] : input.gears)
            {
                auto& gear = *gear_ptr;
                if (gear.disabled) continue;
                result |= gear.tooltip_check(now);
            }
            if (result) base::strike();
        }

        // gate: Attach a new item.
        auto attach(sptr& item)
        {
            std::swap(applet, item);
            if (local) nexthop = applet;
            applet->SIGNAL(tier::release, e2::form::upon::vtree::attached, This());
        }
        // gate: .
        void _rebuild_scene(bool damaged)
        {
            auto stamp = datetime::now();
            auto& canvas = input.xmap;
            if (damaged)
            {
                if (props.legacy_mode & ui::console::mouse) // Render our mouse pointer.
                {
                    draw_mouse_pointer(canvas);
                }
                if (!direct && props.clip_preview_show)
                {
                    draw_clipboard_preview(canvas, stamp);
                }
                if (props.tooltip_enabled)
                {
                    if (direct) send_tooltips();
                    else        draw_tooltips(canvas, stamp);
                }
                if (debug)
                {
                    debug.output(canvas);
                }
                if (props.show_regions)
                {
                    canvas.each([](cell& c)
                    {
                        auto mark = rgba{ rgba::vt256[c.link() % 256] };
                        auto bgc = c.bgc();
                        mark.alpha(64);
                        bgc.mix(mark);
                        c.bgc(bgc);
                    });
                }
            }
            else
            {
                if (props.clip_preview_time != span::zero()) // Check clipboard preview timeout.
                {
                    for (auto& [id, gear_ptr] : input.gears)
                    {
                        auto& gear = *gear_ptr;
                        if (gear.board::shown && props.clip_preview_time < stamp - gear.delta.stamp())
                        {
                            base::deface();
                            return;
                        }
                    }
                }
                if (yield) return;
            }

            // Note: We have to fire a mouse move event every frame,
            //       because in the global frame the mouse can stand still,
            //       but any form can move under the cursor, so for the form itself,
            //       the mouse cursor moves inside the form.
            if (debug)
            {
                debug.bypass = true;
                input.fire(hids::events::mouse::move.id);
                debug.bypass = faux;
                yield = paint.commit(canvas);
                if (yield)
                {
                    auto d = paint.status();
                    debug.update(d.watch, d.delta);
                }
                debug.update(stamp);
            }
            else
            {
                input.fire(hids::events::mouse::move.id);
                yield = paint.commit(canvas); // Try output my canvas to the my console.
            }
        }
        // gate: .
        void rebuild_scene(id_t world_id, bool damaged)
        {
            if (damaged)
            {
                auto& canvas = input.xmap;
                canvas.wipe(world_id);
                if (applet)
                if (auto context = canvas.change_basis(base::area()))
                {
                    applet->render(canvas);
                }
            }
            _rebuild_scene(damaged);
        }
        // gate: Main loop.
        void launch()
        {
            SIGNAL(tier::anycast, e2::form::upon::started, This()); // Make all stuff ready to receive input.
            conio.run();
            SIGNAL(tier::release, e2::form::upon::stopped, true);
        }

    protected:
        //todo revise
        gate(xipc uplink, si32 vtmode, xmls& config, view userid = {}, si32 session_id = 0, bool isvtm = faux)
            : canal{ *uplink },
              props{ canal, userid, vtmode, isvtm, session_id, config },
              input{ props, *this },
              paint{ canal, props.vtmode },
              conio{ canal, *this  },
              debug{*this },
              direct{ props.vtmode == svga::dtvt },
              local{ true }
        {
            auto isolated = config.take("/config/isolated", faux); // DTVT proxy console case.
            config.set("/config/isolated", faux);

            base::root(true);
            base::limits(dot_11);

            LISTEN(tier::release, hids::events::focus::set, gear, oneoff_focus) // Restore all foci for the first user.
            {
                //if (auto target = local ? applet : base::parent())
                if (auto target = nexthop.lock())
                {
                    pro::focus::set(target, gear.id, pro::focus::solo::off, pro::focus::flip::off, true);
                }
                oneoff_focus.reset();
            };
            LISTEN(tier::preview, hids::events::keybd::data::post, gear, tokens) // Start of kb event propagation.
            {
                if (gear)
                //if (auto target = local ? applet : base::parent())
                if (auto target = nexthop.lock())
                {
                    target->SIGNAL(tier::preview, hids::events::keybd::data::post, gear);
                }
            };
            if (!direct)
            {
                LISTEN(tier::release, hids::events::focus::set, gear) // Conio focus tracking.
                {
                    //if (auto target = local ? applet : base::parent())
                    if (auto target = nexthop.lock())
                    {
                        target->SIGNAL(tier::release, hids::events::keybd::focus::bus::on, seed, ({ .id = gear.id }));
                    }
                };
                LISTEN(tier::release, hids::events::focus::off, gear)
                {
                    //if (auto target = local ? applet : base::parent())
                    if (auto target = nexthop.lock())
                    {
                        target->SIGNAL(tier::release, hids::events::keybd::focus::bus::off, seed, ({ .id = gear.id }));
                    }
                };
            }

            LISTEN(tier::release, hids::events::keybd::focus::bus::any, seed, tokens)
            {
                //todo use input::forward<focus>
                if (seed.id != id_t{}) // Translate only the real foreign gear id.
                {
                    auto gear_it = input.gears.find(seed.id);
                    if (gear_it == input.gears.end())
                    {
                        gear_it = input.gears.emplace(seed.id, bell::create<hids>(props, *this, input.xmap)).first;
                    }
                    auto& [_id, gear_ptr] = *gear_it;
                    seed.id = gear_ptr->id;
                }

                auto deed = this->bell::template protos<tier::release>();
                //if constexpr (debugmode) log(prompt::foci, text(seed.deep++ * 4, ' '), "---gate bus::any gear:", seed.id, " hub:", this->id);
                //if (auto target = local ? applet : base::parent())
                if (auto target = nexthop.lock())
                {
                    target->bell::template signal<tier::release>(deed, seed);
                }
                //if constexpr (debugmode) log(prompt::foci, text(--seed.deep * 4, ' '), "----------------gate");
            };
            LISTEN(tier::preview, hids::events::keybd::focus::cut, seed, tokens)
            {
                if (direct)
                {
                    auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(seed.id);
                    if (!gear_ptr) return;
                    conio.focus_cut.send(canal, ext_gear_id);
                }
                else
                {
                    //todo revise see preview::focus::set
                    ////if (auto target = local ? applet : base::parent())
                    if (auto target = base::parent())
                    //if (auto target = nexthop.lock())
                    {
                        target->SIGNAL(tier::release, hids::events::keybd::focus::bus::off, seed);
                    }
                }
            };
            LISTEN(tier::preview, hids::events::keybd::focus::set, seed, tokens)
            {
                if (direct)
                {
                    auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(seed.id);
                    if (!gear_ptr) return;
                    conio.focus_set.send(canal, ext_gear_id, seed.solo);
                }
                else
                {
                    if (seed.item)
                    {
                        seed.item->SIGNAL(tier::release, hids::events::keybd::focus::bus::on, seed);
                    }
                }
            };
            if (direct) // Forward unhandled events outside.
            {
                LISTEN(tier::release, hids::events::keybd::data::any, gear) // Return back unhandled keybd events.
                {
                    if (gear)
                    {
                        auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(gear.id);
                        if (gear_ptr)
                        {
                            conio.keybd_event.send(canal, ext_gear_id,
                                                          gear.ctlstate,
                                                          gear.extflag,
                                                          gear.virtcod,
                                                          gear.scancod,
                                                          gear.pressed,
                                                          gear.cluster,
                                                          gear.handled);
                        }
                    }
                };
            }

            LISTEN(tier::release, e2::form::proceed::quit::any, fast, tokens)
            {
                if constexpr (debugmode) log(prompt::gate, "Quit ", fast ? "fast" : "normal");
                conio.disconnect();
            };
            LISTEN(tier::release, e2::form::prop::name, user_name, tokens)
            {
                uname = uname_txt = user_name;
            };
            LISTEN(tier::request, e2::form::prop::name, user_name, tokens)
            {
                user_name = uname_txt;
            };
            LISTEN(tier::request, e2::form::prop::viewport, viewport, tokens)
            {
                this->SIGNAL(tier::anycast, e2::form::prop::viewport, viewport);
                viewport.coor += base::coor();
            };
            //todo unify creation (delete simple create wo gear)
            LISTEN(tier::preview, e2::form::proceed::create, region, tokens)
            {
                region.coor += base::coor();
                this->RISEUP(tier::release, e2::form::proceed::create, region);
            };
            LISTEN(tier::release, e2::form::proceed::onbehalf, proc, tokens)
            {
                //todo hids
                //proc(input.gear);
            };
            LISTEN(tier::preview, hids::events::keybd::data::any, gear, tokens)
            {
                //todo unify
                if (gear.keystrokes == props.debug_toggle)
                {
                    debug ? debug.stop()
                          : debug.start();
                }
            };
            LISTEN(tier::preview, hids::events::mouse::button::click::leftright, gear, tokens)
            {
                if (gear.clear_clipboard())
                {
                    this->bell::template expire<tier::release>();
                    gear.dismiss();
                }
            };
            LISTEN(tier::release, e2::conio::winsz, new_size, tokens)
            {
                auto new_area = rect{ dot_00, new_size };
                if (applet) applet->SIGNAL(tier::anycast, e2::form::upon::resized, new_area);
                auto old_size = base::size();
                auto delta = base::resize(new_size).size - old_size;
                if (delta && direct) paint.cancel();
            };
            LISTEN(tier::release, e2::conio::pointer, pointer, tokens)
            {
                props.legacy_mode |= pointer ? ui::console::mouse : 0;
            };
            LISTEN(tier::release, e2::conio::error, errcode, tokens)
            {
                log(prompt::gate, "Console error: ", errcode);
                conio.disconnect();
            };
            LISTEN(tier::release, e2::form::upon::stopped, fast, tokens) // Reading loop ends.
            {
                this->SIGNAL(tier::anycast, e2::form::proceed::quit::one, fast);
                conio.disconnect();
                paint.stop();
                mouse.reset(); // Reset active mouse clients to avoid hanging pointers.
                base::detach();
                tokens.reset();
            };
            LISTEN(tier::preview, e2::conio::quit, deal, tokens) // Disconnect.
            {
                conio.disconnect();
            };
            LISTEN(tier::general, e2::conio::quit, deal, tokens) // Shutdown.
            {
                conio.disconnect();
            };
            //LISTEN(tier::general, e2::conio::logs, utf8, tokens)
            //{
            //    //todo application internal log output
            //};
            LISTEN(tier::anycast, e2::form::upon::started, item_ptr, tokens)
            {
                if (props.debug_overlay) debug.start();
                this->SIGNAL(tier::release, e2::form::prop::name, props.title);
                //todo revise
                if (props.title.length())
                {
                    this->RISEUP(tier::preview, e2::form::prop::ui::header, props.title);
                }
            };
            LISTEN(tier::request, e2::form::prop::ui::footer, f, tokens)
            {
                auto window_id = id_t{};
                auto footer = conio.footer.freeze();
                conio.footer_request.send(canal, window_id);
                footer.wait();
                f = footer.thing.utf8;
            };
            LISTEN(tier::request, e2::form::prop::ui::header, h, tokens)
            {
                auto window_id = id_t{};
                auto header = conio.header.freeze();
                conio.header_request.send(canal, window_id);
                header.wait();
                h = header.thing.utf8;
            };
            LISTEN(tier::preview, e2::form::prop::ui::footer, newfooter, tokens)
            {
                auto window_id = id_t{};
                conio.footer.send(canal, window_id, newfooter);
            };
            LISTEN(tier::preview, e2::form::prop::ui::header, newheader, tokens)
            {
                auto window_id = id_t{};
                conio.header.send(canal, window_id, newheader);
            };
            LISTEN(tier::release, hids::events::clipbrd, from_gear, tokens)
            {
                auto myid = from_gear.id;
                auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(myid);
                if (!gear_ptr) return;
                auto& gear =*gear_ptr;
                auto& data = gear.board::cargo;
                conio.clipdata.send(canal, ext_gear_id, data.hash, data.size, data.utf8, data.form);
            };
            LISTEN(tier::request, hids::events::clipbrd, from_gear, tokens)
            {
                auto clipdata = conio.clipdata.freeze();
                auto myid = from_gear.id;
                auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(myid);
                if (gear_ptr)
                {
                    conio.clipdata_request.send(canal, ext_gear_id, from_gear.board::cargo.hash);
                    clipdata.wait();
                    if (clipdata.thing.hash != from_gear.board::cargo.hash)
                    {
                        from_gear.board::cargo.set(clipdata.thing);
                    }
                }
            };
            LISTEN(tier::preview, hids::events::mouse::button::tplclick::leftright, gear, tokens)
            {
                if (debug)
                {
                    props.show_regions = true;
                    debug.stop();
                }
                else
                {
                    if (props.show_regions) props.show_regions = faux;
                    else                    debug.start();
                }
                gear.dismiss();
            };
            if (props.tooltip_enabled)
            {
                LISTEN(tier::general, e2::timer::any, now, tokens)
                {
                    check_tooltips(now);
                };
            }
            if (direct && !isolated) // Forward unhandled events outside.
            {
                LISTEN(tier::release, e2::form::layout::minimize, gear, tokens)
                {
                    auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(gear.id);
                    if (gear_ptr) conio.minimize.send(canal, ext_gear_id);
                };
                LISTEN(tier::release, hids::events::mouse::scroll::any, gear, tokens, (isvtm))
                {
                    auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(gear.id);
                    if (gear_ptr) conio.mouse_event.send(canal, ext_gear_id, gear.ctlstate, gear.mouse::cause, gear.coord, gear.delta.get(), gear.take_button_state());
                    gear.dismiss();
                };
                LISTEN(tier::release, hids::events::mouse::button::any, gear, tokens, (isvtm))
                {
                    using button = hids::events::mouse::button;
                    auto forward = faux;
                    auto cause = gear.mouse::cause;
                    if (isvtm && (gear.index == hids::leftright // Reserved for dragging nested vtm.
                              ||  gear.index == hids::right)    // Reserved for creation inside nested vtm.
                     && events::subevent(cause, button::drag::any.id)) return;
                    if (events::subevent(cause, button::click     ::any.id)
                     || events::subevent(cause, button::dblclick  ::any.id)
                     || events::subevent(cause, button::tplclick  ::any.id)
                     || events::subevent(cause, button::drag::pull::any.id))
                    {
                        gear.setfree();
                        forward = true;
                    }
                    else if (events::subevent(cause, button::drag::start::any.id))
                    {
                        gear.capture(bell::id); // To avoid unhandled mouse pull processing.
                        forward = true;
                    }
                    else if (events::subevent(cause, button::drag::cancel::any.id)
                          || events::subevent(cause, button::drag::stop  ::any.id))
                    {
                        gear.setfree();
                    }
                    if (forward)
                    {
                        auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(gear.id);
                        if (gear_ptr) conio.mouse_event.send(canal, ext_gear_id, gear.ctlstate, cause, gear.coord, gear.delta.get(), gear.take_button_state());
                        gear.dismiss();
                    }
                };
                LISTEN(tier::release, e2::config::fps, fps, tokens)
                {
                    if (fps > 0) this->SIGNAL(tier::general, e2::config::fps, fps);
                };
                LISTEN(tier::preview, e2::config::fps, fps, tokens)
                {
                    conio.fps.send(canal, fps);
                };
                LISTEN(tier::preview, hids::events::mouse::button::click::any, gear, tokens)
                {
                    conio.expose.send(canal);
                };
                LISTEN(tier::preview, e2::form::layout::expose, item, tokens)
                {
                    conio.expose.send(canal);
                };
                LISTEN(tier::preview, e2::form::layout::swarp, warp, tokens)
                {
                    conio.warping.send(canal, 0, warp);
                };
                LISTEN(tier::release, e2::form::layout::fullscreen, gear, tokens)
                {
                    auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(gear.id);
                    if (gear_ptr) conio.fullscreen.send(canal, ext_gear_id);
                };
            }
        }
        // gate: .
        void inform(rect new_area) override
        {
            if (applet)
            {
                applet->base::resize(new_area.size);
            }
        }
    };

    // console: World aether.
    class host
        : public form<host>
    {
    public:
        using tick = datetime::quartz<events::reactor<>, hint>;
        using list = std::vector<rect>;

        pro::focus focus; // host: Focus controller. Must be the first of all focus subscriptions.

        tick quartz; // host: Frame rate synchronizator.
        si32 maxfps; // host: Frame rate.
        list debris; // host: Wrecked regions.
        xmls config; // host: Running configuration.
        subs tokens; // host: Subscription tokens.
        flag active; // host: Host is available for connections.

        std::vector<bool> user_numbering; // host: .

    protected:
        host(xipc server, xmls config, pro::focus::mode m = pro::focus::mode::hub)
            :  focus{ *this, m, faux },
              quartz{ bell::router<tier::general>(), e2::timer::tick.id },
              config{ config },
              active{ true }
        {
            using namespace std::chrono;
            auto& canal = *server;

            auto& g = skin::globals();
            g.wheel_dt       = config.take("wheel_dt"              , 3     );
            g.brighter       = config.take("brighter"              , cell{});//120);
            g.kb_focus       = config.take("kb_focus"              , cell{});//60
            g.shadower       = config.take("shadower"              , cell{});//180);//60);//40);// 20);
            g.shadow         = config.take("shadow"                , cell{});//180);//5);
            g.selector       = config.take("selector"              , cell{});//48);
            g.highlight      = config.take("highlight"             , cell{});
            g.selected       = config.take("selected"              , cell{});
            g.warning        = config.take("warning"               , cell{});
            g.danger         = config.take("danger"                , cell{});
            g.action         = config.take("action"                , cell{});
            g.label          = config.take("label"                 , cell{});
            g.inactive       = config.take("inactive"              , cell{});
            g.menu_white     = config.take("menu_white"            , cell{});
            g.menu_black     = config.take("menu_black"            , cell{});
            g.lucidity       = config.take("lucidity");
            g.tracking       = config.take("tracking"              , faux);
            g.bordersz       = config.take("bordersz"              , dot_11);
            g.macstyle       = config.take("macstyle"              , faux);
            g.spd            = config.take("timings/spd"           , 10  );
            g.pls            = config.take("timings/pls"           , 167 );
            g.spd_accel      = config.take("timings/spd_accel"     , 1   );
            g.spd_max        = config.take("timings/spd_max"       , 100 );
            g.ccl            = config.take("timings/ccl"           , 120 );
            g.ccl_accel      = config.take("timings/ccl_accel"     , 30  );
            g.ccl_max        = config.take("timings/ccl_max"       , 1   );
            g.switching      = config.take("timings/switching"     , 200 );
            g.deceleration   = config.take("timings/deceleration"  , span{ 2s    });
            g.blink_period   = config.take("timings/blink_period"  , span{ 400ms });
            g.menu_timeout   = config.take("timings/menu_timeout"  , span{ 250ms });
            g.active_timeout = config.take("timings/active_timeout", span{ 1s    });
            g.repeat_delay   = config.take("timings/repeat_delay"  , span{ 500ms });
            g.repeat_rate    = config.take("timings/repeat_rate"   , span{ 30ms  });
            g.fader_time     = config.take("timings/fader/duration", span{ 150ms });
            g.fader_fast     = config.take("timings/fader/fast"    , span{ 0ms   });
            g.max_value      = config.take("limits/window/size"    , twod{ 2000, 1000  });
            g.menuwide       = config.take("/config/menu/wide"     , faux);

            maxfps = config.take("fps");
            if (maxfps <= 0) maxfps = 60;

            LISTEN(tier::request, e2::config::creator, world_ptr, tokens)
            {
                world_ptr = base::This();
            };
            LISTEN(tier::general, e2::config::fps, fps, tokens)
            {
                if (fps > 0)
                {
                    maxfps = fps;
                    quartz.ignite(maxfps);
                }
                else if (fps == -1)
                {
                    fps = maxfps;
                }
                else
                {
                    quartz.stop();
                }
            };
            LISTEN(tier::general, e2::cleanup, counter, tokens)
            {
                this->template router<tier::general>().cleanup(counter.ref_count, counter.del_count);
            };
            LISTEN(tier::general, hids::events::halt, gear, tokens)
            {
                if (gear.captured(bell::id))
                {
                    gear.setfree();
                    gear.dismiss();
                }
            };
            LISTEN(tier::general, e2::shutdown, msg, tokens)
            {
                if constexpr (debugmode) log(prompt::host, msg);
                active.exchange(faux); // To prevent new applications from launching.
                canal.stop();
            };
            LISTEN(tier::general, hids::events::device::user::login, props, tokens)
            {
                props = 0;
                while (props < user_numbering.size() && user_numbering[props]) { props++; }
                if (props == user_numbering.size()) user_numbering.push_back(true);
                else                                user_numbering[props] = true;
            };
            LISTEN(tier::general, hids::events::device::user::logout, props, tokens)
            {
                if (props < user_numbering.size()) user_numbering[props] = faux;
                else
                {
                    if constexpr (debugmode) log(prompt::host, ansi::err("User accounting error: ring size:", user_numbering.size(), " user_number:", props));
                }
            };

            quartz.ignite(maxfps);
            log(prompt::host, "Rendering refresh rate: ", maxfps, " fps");
        }

    public:
        // host: Mark dirty region.
        void denote(rect updateregion)
        {
            if (updateregion)
            {
                debris.push_back(updateregion);
            }
        }
        void deface(rect region) override
        {
            base::deface(region);
            denote(region);
        }
        // host: Create a new root of the specified subtype and attach it.
        auto invite(xipc uplink, sptr& applet, si32 vtmode, twod winsz)
        {
            auto lock = events::unique_lock();
            auto portal = ui::gate::ctor(uplink, vtmode, host::config);
            portal->SIGNAL(tier::release, e2::form::upon::vtree::attached, base::This());
            portal->attach(applet);
            portal->base::resize(winsz);
            auto& screen = *portal;
            LISTEN(tier::general, e2::timer::any, timestamp)
            {
                auto damaged = !debris.empty();
                debris.clear();
                screen.rebuild_scene(bell::id, damaged);
            };
            screen.LISTEN(tier::release, e2::conio::winsz, new_size, -)
            {
                screen.rebuild_scene(bell::id, true);
            };
            lock.unlock();
            portal->launch();
            netxs::events::dequeue();
            quartz.stop();
        }
        // host: Shutdown.
        void stop()
        {
            auto lock = events::sync{};
            mouse.reset();
            tokens.reset();
        }
    };
}