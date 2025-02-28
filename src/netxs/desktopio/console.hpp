// Copyright (c) Dmitry Sapozhnikov
// Licensed under the MIT license.

#pragma once

#include "controls.hpp"

namespace netxs::ui
{
    namespace console
    {
        static auto id = std::pair<ui32, time>{};
        static constexpr auto _counter = __COUNTER__ + 1;
        static constexpr auto mouse   = 1 << (__COUNTER__ - _counter);
        static constexpr auto nt      = 1 << (__COUNTER__ - _counter); // Use win32 console api for input.
        static constexpr auto redirio = 1 << (__COUNTER__ - _counter);
        static constexpr auto gui     = 1 << (__COUNTER__ - _counter);
        static constexpr auto tui     = 1 << (__COUNTER__ - _counter); // Output is in TUI mode.
        //todo make 3-bit field for color mode
        static constexpr auto nt16    = 1 << (__COUNTER__ - _counter);
        static constexpr auto vt16    = 1 << (__COUNTER__ - _counter);
        static constexpr auto vt256   = 1 << (__COUNTER__ - _counter);
        static constexpr auto direct  = 1 << (__COUNTER__ - _counter);
        static constexpr auto vtrgb   = 1 << (__COUNTER__ - _counter);

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
                if (mode & vtrgb  ) result += "vtrgb ";
                if (mode & direct ) result += "direct ";
                if (result.size()) result.pop_back();
            }
            else result = "unknown";
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
            gate& owner; // link: Link owner.
            wptr  owner_wptr; // link: .

            link(pipe& canal, gate& owner)
                : s11n{ *this },
                 canal{ canal },
                 owner{ owner }
            {
                auto& oneshot = owner.base::template field<hook>(); //todo Apple clang requires template keyword
                owner.LISTEN(tier::anycast, e2::form::upon::started, root, oneshot)
                {
                    owner_wptr = owner.This();
                    owner.base::unfield(oneshot);
                };
            }

            // link: Send an event message to the link owner.
            template<class E, class T>
            void notify(E, T&& data, si32 Tier = tier::release)
            {
                owner.bell::enqueue(owner_wptr, [Tier, d = data](auto& boss) mutable
                {
                    boss.base::signal(Tier, E::id, d);
                });
            }
            void handle(s11n::xs::req_input_fields lock)
            {
                owner.bell::enqueue(owner_wptr, [&, item = lock.thing](auto& /*boss*/) mutable
                {
                    auto ext_gear_id = item.gear_id;
                    auto int_gear_id = owner.get_int_gear_id(ext_gear_id);
                    auto inputfield_request = owner.base::signal(tier::general, ui::e2::command::request::inputfields, { .gear_id = int_gear_id, .acpStart = item.acpStart, .acpEnd = item.acpEnd }); // pro::focus retransmits as a tier::release for focused objects.
                    auto field_list = inputfield_request.wait_for();
                    for (auto& f : field_list) f.coor -= owner.coor();
                    s11n::ack_input_fields.send(canal, ext_gear_id, field_list);
                });
            }
            void handle(s11n::xs::command     lock)
            {
                auto cmd = eccc{ .cmd = lock.thing.utf8 };
                notify(e2::command::run, cmd);
                auto msg = utf::concat(prompt::repl, ansi::clr(yellowlt, utf::trim(cmd.cmd, "\r\n")));
                s11n::logs.send(canal, ui32{}, datetime::now(), msg);
            }
            void handle(s11n::xs::syswinsz    lock)
            {
                auto& item = lock.thing;
                notify(e2::conio::winsz, item.winsize);
            }
            //todo use s11n::xs::screenmode:  normal/fullscreen/maximized/minimized
            void handle(s11n::xs::fullscrn  /*lock*/)
            {
                owner.fullscreen = true;
            }
            void handle(s11n::xs::restored  /*lock*/)
            {
                owner.fullscreen = faux;
            }
            void handle(s11n::xs::sysboard    lock)
            {
                auto& item = lock.thing;
                notify(e2::conio::board, item);
            }
            void handle(s11n::xs::logs        lock)
            {
                auto& item = lock.thing;
                if (ui::console::id.first == item.id)
                {
                    notify(e2::conio::logs, item.data, tier::general);
                }
                else
                {
                    if (item.data.size() && item.data.back() == '\n') item.data.pop_back();
                    if (item.data.size())
                    {
                        auto data = escx{};
                        utf::split(item.data, '\n', [&](auto line)
                        {
                            data.add(netxs::prompt::pads, item.id, ": ", line, '\n');
                        });
                        notify(e2::conio::logs, data, tier::general);
                    }
                }
            }
            void handle(s11n::xs::syskeybd    lock)
            {
                auto& item = lock.thing;
                notify(e2::conio::keybd, item);
            }
            void handle(s11n::xs::sysfocus    lock)
            {
                auto& item = lock.thing;
                notify(e2::conio::focus::post, item);
            }
            void handle(s11n::xs::sysmouse    lock)
            {
                auto& item = lock.thing;
                notify(e2::conio::mouse, item);
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
                {
                    auto jumbos = cell::glyf::jumbos();
                    for (auto& gc : items)
                    {
                        auto& cluster = jumbos.get(gc.token);
                        if (cluster.length())
                        {
                            list.thing.push(gc.token, cluster);
                        }
                    }
                }
                list.thing.sendby(canal);
            }
            void handle(s11n::xs::fps         lock)
            {
                auto& item = lock.thing;
                notify(e2::config::fps, item.frame_rate);
            }
            void handle(s11n::xs::cwd         lock)
            {
                auto& path = lock.thing.path;
                notify(e2::form::prop::cwd, path, tier::anycast);
            }
            void handle(s11n::xs::sysclose    lock)
            {
                // Immediately reply (w/o queueing) on sysclose request to avoid deadlock.
                // In case of recursive connection via terminal, ui::term schedules self-closing and waiting for the vtty to be released inside the task broker.
                // vtm client waits for disconnect acknowledge which is scheduled (if scheduled) right after the vtty cleanup task.
                lock.unlock();
                owner.disconnect();
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
                span watch{}; // diff::stat: Rendering duration.
                sz_t delta{}; // diff::stat: Last rendered frame size.
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

            // diff: Render current buffer.
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
                        guard.unlock(); // Allow to abort.
                        canal.isbusy = true; // It's okay if someone resets the busy flag before sending.
                        image.sendby(canal);
                        canal.isbusy.wait(true); // Successive frames must be discarded until the current frame is delivered (to prevent unlimited buffer growth).
                        guard.lock();
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
            // diff: Try to add the touched canvas image to the queue for analysis and sending detected differences.
            auto send(core const& canvas)
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
                paint = std::thread{ [&, vtmode]
                {
                         if (vtmode == svga::dtvt ) render<binary::bitmap_dtvt_t >();
                    else if (vtmode == svga::vtrgb) render<binary::bitmap_vtrgb_t>();
                    else if (vtmode == svga::vt256) render<binary::bitmap_vt256_t>();
                    else if (vtmode == svga::vt16 ) render<binary::bitmap_vt16_t >();
                    else if (vtmode == svga::nt16 ) render<binary::bitmap_dtvt_t >();
                }};
            }
            void stop()
            {
                if (!alive.exchange(faux)) return;
                auto thread_id = paint.get_id();
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
                canal.isbusy = faux;
                canal.isbusy.notify_all();
                paint.join();
                if constexpr (debugmode) log(prompt::diff, "Rendering thread joined", ' ', utf::to_hex_0x(thread_id));
            }
        };

        // gate: Application properties.
        struct props_t
        {
            //todo revise
            text os_user_id;
            text title;
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
            bool debug_overlay; // conf: Enable to show debug overlay.
            bool show_regions; // conf: Highlight region ownership.
            bool simple; // conf: .
            svga vtmode; // conf: .
            si32 clip_prtscrn_mime; // conf: Print-screen copy encoding format.

            void read(xmls& config)
            {
                clip_preview_clrs = config.take("/config/clipboard/preview/color"  , cell{}.bgc(bluedk).fgc(whitelt));
                clip_preview_time = config.take("/config/clipboard/preview/timeout", span{ 3s });
                clip_preview_alfa = config.take("/config/clipboard/preview/alpha"  , byte{ 0xFF });
                clip_preview_glow = config.take("/config/clipboard/preview/shadow" , 3);
                clip_preview_show = config.take("/config/clipboard/preview/enabled", true);
                clip_preview_size = config.take("/config/clipboard/preview/size"   , twod{ 80,25 });
                clip_prtscrn_mime = config.take("/config/clipboard/format"         , mime::htmltext, xml::options::format);
                dblclick_timeout  = config.take("/config/timings/dblclick"         , span{ 500ms });
                tooltip_colors    = config.take("/config/tooltips/color"           , cell{}.bgc(0xFFffffff).fgc(0xFF000000));
                tooltip_timeout   = config.take("/config/tooltips/timeout"         , span{ 2000ms });
                tooltip_enabled   = config.take("/config/tooltips/enabled"         , true);
                debug_overlay     = config.take("/config/debug/overlay"            , faux);
                show_regions      = config.take("/config/debug/regions"            , faux);
                clip_preview_glow = std::clamp(clip_preview_glow, 0, 5);
            }

            props_t(pipe& /*canal*/, view userid, si32 mode, bool isvtm, si32 session_id, xmls& config)
            {
                read(config);
                legacy_mode = mode;
                if (isvtm)
                {
                    this->session_id  = session_id;
                    os_user_id        = utf::concat("[", userid, ":", session_id, "]");
                    title             = os_user_id;
                    background_color  = config.take("/config/desktop/background/color", cell{}.fgc(whitedk).bgc(0xFF000000));
                    auto utf8_tile    = config.take("/config/desktop/background/tile", ""s);
                    if (utf8_tile.size())
                    {
                        auto block = page{ utf8_tile };
                        background_image.size(block.limits());
                        background_image.output(block);
                    }
                    simple            = faux;
                }
                else
                {
                    simple            = !(legacy_mode & ui::console::direct);
                    title             = "";
                }
                vtmode = legacy_mode & ui::console::nt16   ? svga::nt16
                       : legacy_mode & ui::console::vt16   ? svga::vt16
                       : legacy_mode & ui::console::vt256  ? svga::vt256
                       : legacy_mode & ui::console::gui    ? svga::dtvt
                       : legacy_mode & ui::console::direct ? svga::dtvt
                       : legacy_mode & ui::console::vtrgb  ? svga::vtrgb
                                                           : svga::vtrgb;
            }

            friend auto& operator << (std::ostream& s, props_t const& c)
            {
                return s << "\n\tuser: " << c.os_user_id
                         << "\n\tmode: " << ui::console::str(c.legacy_mode);
            }
        };

    public:
        pipe&      canal; // gate: Channel to outside.
        props_t    props; // gate: Input gate properties.
        diff       paint; // gate: Renderer.
        link       conio; // gate: Input data parser.
        flag       alive; // gate: sysclose isn't sent.
        bool       direct; // gate: .
        bool       yield; // gate: Indicator that the current frame has been successfully sent.
        bool       fullscreen; // gate: .
        face       canvas; // gate: .
        std::unordered_map<id_t, netxs::sptr<hids>> gears; // gate: .
        pro::debug& debug;
        input::multihome_t& multihome;

        void forward(auto& device)
        {
            auto gear_it = gears.find(device.gear_id);
            if (gear_it == gears.end())
            {
                gear_it = gears.emplace(device.gear_id, bell::create<hids>(*this, canvas)).first;
                auto& gear = *(gear_it->second);
                gear.tooltip_timeout = props.tooltip_timeout;
                gear.board::ghost = props.clip_preview_glow;
                gear.board::brush = props.clip_preview_clrs;
                gear.board::alpha = props.clip_preview_alfa;
                gear.mouse::delay = props.dblclick_timeout;
                auto& luafx = gear.base::template plugin<pro::luafx>(); //todo apple clang requires template keyword
                luafx.activate("gear.proc_map",
                {
                    { "IsKeyRepeated",  [&]
                                        {
                                            auto repeated = gear.keystat == input::key::repeated;
                                            luafx.set_return(repeated);
                                        }},
                    { "SetHandled",     [&]
                                        {
                                            gear.set_handled();
                                            gear.interrupt_key_proc = true;
                                            luafx.set_return();
                                        }},
                });
            }
            auto& [ext_gear_id, gear_ptr] = *gear_it;
            gear_ptr->set_multihome();
            gear_ptr->hids::take(device);
            base::strike();
        }
        void fire(hint event_id)
        {
            for (auto& [ext_gear_id, gear_ptr] : gears)
            {
                if (ext_gear_id)
                {
                    auto& gear = *gear_ptr;
                    if (gear.m_sys.timecod != time{}) // Don't send mouse events if the mouse has not been used yet.
                    {
                        gear.fire_fast();
                        gear.fire(event_id);
                    }
                }
            }
        }
        auto get_ext_gear_id(id_t gear_id)
        {
            for (auto& [ext_gear_id, gear_ptr] : gears)
            {
                if (gear_ptr->id == gear_id) return std::pair{ ext_gear_id, gear_ptr };
            }
            return std::pair{ id_t{}, netxs::sptr<hids>{} };
        }
        id_t get_int_gear_id(id_t ext_gear_id)
        {
            auto int_gear_id = id_t{};
            auto gear_it = gears.find(ext_gear_id);
            if (gear_it != gears.end()) int_gear_id = gear_it->second->id;
            return int_gear_id;
        }
        void fill_pointer(hids& gear, face& parent_canvas)
        {
            static const auto idle = cell{}.txt("\xE2\x96\x88"/*\u2588 █ */).bgc(0x00).fgc(0xFF00ff00);
            static const auto busy = cell{}.bgc(reddk).fgc(0xFFffffff);
            auto brush = gear.m_sys.buttons ? cell{ busy }.txt(64 + (char)gear.m_sys.buttons/*A-Z*/)
                                            : idle;
            auto area = rect{ gear.owner.coor() + gear.coord, dot_11 };
            parent_canvas.fill(area, cell::shaders::fuse(brush));
        }
        void draw_mouse_pointer(face& parent_canvas)
        {
            for (auto& [ext_gear_id, gear_ptr] : gears)
            {
                auto& gear = *gear_ptr;
                if (gear.mouse_disabled) continue;
                fill_pointer(gear, parent_canvas);
            }
        }
        void draw_clipboard_preview(time const& stamp)
        {
            for (auto& [ext_gear_id, gear_ptr] : gears)
            {
                auto& gear = *gear_ptr;
                gear.board::shown = !gear.mouse_disabled &&
                                    (props.clip_preview_time == span::zero() ||
                                     props.clip_preview_time > stamp - gear.delta.stamp());
                if (gear.board::shown)
                {
                    auto coor = twod{ gear.coord } + dot_21 * 2;
                    auto full = gear.board::image.full();
                    gear.board::image.move(coor - full.coor);
                    canvas.plot(gear.board::image, cell::shaders::mix);
                }
            }
        }
        void draw_tooltips(time const& stamp)
        {
            auto full = canvas.full();
            auto area = canvas.area();
            auto zero = rect{ dot_00, area.size };
            canvas.area(zero);
            for (auto& [ext_gear_id, gear_ptr] : gears)
            {
                auto& gear = *gear_ptr;
                if (gear.mouse_disabled) continue;
                if (gear.tooltip_enabled(stamp))
                {
                    auto [tooltip_data, tooltip_update] = gear.get_tooltip();
                    if (tooltip_data)
                    {
                        //todo optimize - cache tooltip_page
                        auto tooltip_page = page{ tooltip_data };
                        auto full_area = full;
                        full_area.coor = std::max(dot_00, twod{ gear.coord } - twod{ 4, tooltip_page.size() + 1 });
                        full_area.size.x = dot_mx.x; // Prevent line wrapping.
                        canvas.full(full_area);
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
            for (auto& [ext_gear_id, gear_ptr] : gears /* use filter gear.is_tooltip_changed()*/)
            {
                auto& gear = *gear_ptr;
                if (gear.mouse_disabled) continue;
                if (gear.is_tooltip_changed())
                {
                    auto [tooltip_data, tooltip_update] = gear.get_tooltip();
                    list.thing.push(ext_gear_id, tooltip_data, tooltip_update);
                }
            }
            list.thing.sendby<true>(canal);
        }
        auto check_tooltips(time now)
        {
            auto result = faux;
            for (auto& [ext_gear_id, gear_ptr] : gears)
            {
                auto& gear = *gear_ptr;
                if (gear.mouse_disabled) continue;
                result |= gear.tooltip_check(now);
            }
            return result;
        }

        // gate: .
        void rebuild_scene(time stamp)
        {
            auto damaged = base::ruined();
            if (props.tooltip_enabled)
            {
                damaged |= check_tooltips(stamp);
            }
            if (damaged)
            {
                if (auto context = canvas.change_basis(base::area()))
                {
                    canvas.wipe(props.background_color);
                    if (base::subset.size() == 1 && props.background_image.size()) // Taskbar only (no full screen app on top).
                    {
                        //todo cache background
                        canvas.tile(props.background_image, cell::shaders::fuse);
                    }
                    if (base::subset.size())
                    {
                        base::subset.back()->render(canvas);
                    }
                    if (!direct && props.clip_preview_show)
                    {
                        draw_clipboard_preview(stamp);
                    }
                    if (props.tooltip_enabled)
                    {
                        if (direct) send_tooltips();
                        else        draw_tooltips(stamp);
                    }
                    if (props.debug_overlay)
                    {
                        debug.output(canvas);
                    }
                    if (props.legacy_mode & ui::console::mouse) // Render our mouse pointer.
                    {
                        draw_mouse_pointer(canvas);
                    }
                    if (props.show_regions)
                    {
                        canvas.each([](cell& c)
                        {
                            auto mark = argb{ argb::vt256[c.link() % 256] };
                            auto bgc = c.bgc();
                            mark.alpha(64);
                            bgc.mix(mark);
                            c.bgc(bgc);
                        });
                    }
                }
            }
            else
            {
                if (props.clip_preview_time != span::zero()) // Check clipboard preview timeout.
                {
                    for (auto& [ext_gear_id, gear_ptr] : gears)
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
            yield = paint.send(canvas); // Try to output updated canvas if paint is not busy.

            if (props.debug_overlay) // Get rendering stats.
            {
                if (yield)
                {
                    auto d = paint.status();
                    debug.update(d.watch, d.delta);
                }
                debug.update(stamp);
            }
            // Note: We have to fire a mouse move event every frame,
            //       because in the global frame the mouse can stand still,
            //       but any form can move under the cursor, so for the form itself,
            //       the mouse cursor moves inside the form.
            base::ruined(faux);
            fire(input::events::mouse::move.id);
        }
        // gate: Rx loop.
        void launch()
        {
            base::signal(tier::anycast, e2::form::upon::started, This()); // Make all stuff ready to receive input.
            directvt::binary::stream::reading_loop(canal, [&](view data){ conio.s11n::sync(data); });
            conio.s11n::stop(); // Wake up waiting dtvt objects, if any.
            if constexpr (debugmode) log(prompt::gate, "DirectVT session closed");
            base::signal(tier::release, e2::form::upon::stopped, true);
        }

        //todo revise
        gate(xipc uplink, si32 vtmode, xmls& config, view userid = {}, si32 session_id = 0, bool isvtm = faux)
            : canal{ *uplink },
              props{ canal, userid, vtmode, isvtm, session_id, config },
              paint{ canal, props.vtmode },
              conio{ canal, *this  },
              alive{ true },
              direct{ !!(vtmode & (ui::console::direct | ui::console::gui)) },
              yield{ faux },
              fullscreen{ faux },
              debug{ base::plugin<pro::debug>() },
              multihome{ base::property<input::multihome_t>("multihome") }
        {
            base::plugin<pro::focus>();
            auto& keybd = base::plugin<pro::keybd>("gate");
            auto& mouse = base::plugin<pro::mouse>();
            auto& luafx = base::plugin<pro::luafx>();
            auto bindings = pro::keybd::load(config, "gate");
            keybd.bind(bindings);
            luafx.activate("gate.proc_map",
            {
                { "Disconnect",             [&]
                                            {
                                                auto gear_ptr = luafx.template get_object<hids>("gear");
                                                auto ok = !!gear_ptr;
                                                if (ok)
                                                {
                                                    gear_ptr->set_handled();
                                                }
                                                base::signal(tier::preview, e2::conio::quit);
                                                luafx.set_return();
                                            }},
                { "DebugOverlay",           [&]
                                            {
                                                auto gear_ptr = luafx.template get_object<hids>("gear");
                                                auto ok = !!gear_ptr;
                                                if (ok)
                                                {
                                                    gear_ptr->set_handled();
                                                }
                                                props.debug_overlay ? debug.stop() : debug.start();
                                                props.debug_overlay = !props.debug_overlay;
                                                base::deface();
                                                luafx.set_return();
                                            }},
                { "IncreasecCellHeight",    [&]
                                            {
                                                auto gui_cmd = e2::command::gui.param();
                                                auto gear_ptr = luafx.template get_object<hids>("gear");
                                                auto ok = !!gear_ptr;
                                                if (ok)
                                                {
                                                    gui_cmd.gear_id = gear_ptr->id;
                                                    gear_ptr->set_handled();
                                                }
                                                gui_cmd.cmd_id = syscmd::tunecellheight;
                                                gui_cmd.args.emplace_back(luafx.get_args_or(1, fp32{ 1.f }));
                                                base::signal(tier::preview, e2::command::gui, gui_cmd);
                                                luafx.set_return();
                                            }},
                { "RollFonts",              [&]
                                            {
                                                auto gui_cmd = e2::command::gui.param();
                                                auto gear_ptr = luafx.template get_object<hids>("gear");
                                                auto ok = !!gear_ptr;
                                                if (ok)
                                                {
                                                    gui_cmd.gear_id = gear_ptr->id;
                                                    gear_ptr->set_handled();
                                                }
                                                gui_cmd.cmd_id = syscmd::rollfontlist;
                                                gui_cmd.args.emplace_back(luafx.get_args_or(1, si32{ 1 }));
                                                base::signal(tier::preview, e2::command::gui, gui_cmd);
                                                luafx.set_return();
                                            }},
                { "WheelAccumReset",        [&]
                                            {
                                                auto gui_cmd = e2::command::gui.param();
                                                auto gear_ptr = luafx.template get_object<hids>("gear");
                                                auto ok = !!gear_ptr;
                                                if (ok)
                                                {
                                                    gui_cmd.gear_id = gear_ptr->id;
                                                }
                                                gui_cmd.cmd_id = syscmd::resetwheelaccum;
                                                base::signal(tier::preview, e2::command::gui, gui_cmd);
                                                luafx.set_return();
                                            }},
                { "CellHeightReset",        [&]
                                            {
                                                auto gui_cmd = e2::command::gui.param();
                                                auto gear_ptr = luafx.template get_object<hids>("gear");
                                                auto ok = !!gear_ptr;
                                                if (ok)
                                                {
                                                    gui_cmd.gear_id = gear_ptr->id;
                                                    gear_ptr->set_handled();
                                                }
                                                gui_cmd.cmd_id = syscmd::resetcellheight;
                                                base::signal(tier::preview, e2::command::gui, gui_cmd);
                                                luafx.set_return();
                                            }},
                { "AntialiasingMode",       [&]
                                            {
                                                auto gui_cmd = e2::command::gui.param();
                                                auto gear_ptr = luafx.template get_object<hids>("gear");
                                                auto ok = !!gear_ptr;
                                                if (ok)
                                                {
                                                    gui_cmd.gear_id = gear_ptr->id;
                                                    gear_ptr->set_handled();
                                                }
                                                //todo args
                                                gui_cmd.cmd_id = syscmd::toggleaamode;
                                                base::signal(tier::preview, e2::command::gui, gui_cmd);
                                                luafx.set_return();
                                            }},
            });

            base::root(true);
            base::limits(dot_11);
            props.background_color.txt(whitespace).link(bell::id);
            canvas.link(bell::id);
            canvas.cmode = props.vtmode;
            canvas.face::area(base::area());
            LISTEN(tier::release, e2::form::proceed::multihome, world_ptr)
            {
                multihome = { world_ptr, world_ptr->base::father };
            };
            LISTEN(tier::release, e2::command::printscreen, gear)
            {
                auto data = escx{};
                props.clip_prtscrn_mime == mime::textonly ? data.s11n<faux>(canvas, gear.slot)
                                                          : data.s11n<true>(canvas, gear.slot);
                if (data.length())
                {
                    if (props.clip_prtscrn_mime != mime::disabled)
                    {
                        gear.set_clipboard(gear.slot.size, data, props.clip_prtscrn_mime);
                    }
                }
            };
            LISTEN(tier::release, e2::area, new_area)
            {
                canvas.face::area(new_area);
            };
            LISTEN(tier::preview, e2::command::gui, gui_cmd)
            {
                if (gui_cmd.cmd_id == syscmd::restore && base::subset.size() > 1)
                {
                    bell::enqueue(This(), [](auto& boss) // Keep the focus tree intact while processing events.
                    {
                        boss.base::signal(tier::release, e2::form::size::restore);
                    });
                }
                else
                {
                    if (gui_cmd.gear_id)
                    {
                        auto [ext_gear_id, gear_ptr] = get_ext_gear_id(gui_cmd.gear_id);
                        if (gear_ptr)
                        {
                            gui_cmd.gear_id = ext_gear_id;
                        }
                    }
                    conio.gui_command.send(canal, gui_cmd);
                }
            };
            LISTEN(tier::release, e2::command::run, script)
            {
                luafx.set_object(This(), "gate");
                luafx.run_script(script);
            };
            LISTEN(tier::preview, e2::runscript, gear)
            {
                if (!gear.script_ptr) return;
                if (!gear.scripting_context_ptr) return;
                auto& script_body = *gear.script_ptr;
                auto& scripting_context = *gear.scripting_context_ptr;
                luafx.set_object(This(), "gate");
                luafx.set_object(gear.This(), "gear");
                luafx.run_script(script_body, scripting_context);
            };
            LISTEN(tier::release, e2::conio::mouse, m)
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
                    base::strike();
                }
                else forward(m);
            };
            LISTEN(tier::release, e2::conio::keybd, k)
            {
                forward(k);
            };
            LISTEN(tier::release, e2::conio::focus::any, f)
            {
                forward(f);
            };
            LISTEN(tier::release, e2::conio::board, c)
            {
                forward(c);
            };
            LISTEN(tier::preview, input::events::focus::set::any, seed)
            {
                if (seed.gear_id)
                {
                    auto [ext_gear_id, gear_ptr] = get_ext_gear_id(seed.gear_id);
                    if (gear_ptr)
                    {
                        auto deed = bell::protos(tier::preview);
                        auto state = deed == input::events::focus::set::on.id;
                        conio.sysfocus.send(canal, ext_gear_id, state, seed.focus_type, ui64{}, ui64{});
                    }
                }
            };
            LISTEN(tier::release, input::events::keybd::any, gear) // Forward unhandled events to the outside. Return back unhandled keybd events.
            {
                if (!gear.handled)
                {
                    auto [ext_gear_id, gear_ptr] = get_ext_gear_id(gear.id);
                    if (gear_ptr)
                    {
                        gear.gear_id = ext_gear_id;
                        conio.syskeybd.send(canal, gear);
                    }
                }
            };
            LISTEN(tier::release, e2::form::proceed::quit::any, fast)
            {
                if constexpr (debugmode) log(prompt::gate, "Quit ", fast ? "fast" : "normal");
                disconnect();
            };
            LISTEN(tier::request, e2::form::prop::viewport, viewport)
            {
                viewport = base::area();
            };
            //todo unify creation (delete simple create wo gear)
            LISTEN(tier::preview, e2::form::proceed::create, dest_region)
            {
                dest_region.coor += base::coor();
                this->base::riseup(tier::release, e2::form::proceed::create, dest_region);
            };
            LISTEN(tier::preview, input::events::mouse::button::click::leftright, gear)
            {
                if (gear.clear_clipboard())
                {
                    this->bell::expire(tier::release);
                    gear.dismiss();
                }
            };
            LISTEN(tier::release, e2::conio::pointer, pointer)
            {
                props.legacy_mode |= pointer ? ui::console::mouse : 0;
            };
            LISTEN(tier::release, e2::form::upon::stopped, fast) // Reading loop ends.
            {
                this->base::signal(tier::anycast, e2::form::proceed::quit::one, fast);
                disconnect();
                paint.stop();
                mouse.reset(); // Reset active mouse clients to avoid hanging pointers.
                bell::sensors.clear();
            };
            LISTEN(tier::preview, e2::conio::quit, deal) // Disconnect.
            {
                disconnect();
            };
            LISTEN(tier::general, e2::conio::quit, deal) // Shutdown.
            {
                disconnect();
            };
            LISTEN(tier::anycast, e2::form::upon::started, item_ptr)
            {
                if (props.debug_overlay) debug.start();
                this->base::signal(tier::release, e2::form::prop::name, props.title);
                //todo revise
                if (props.title.length())
                {
                    this->base::riseup(tier::preview, e2::form::prop::ui::header, props.title);
                }
            };
            LISTEN(tier::request, e2::form::prop::ui::footer, f)
            {
                auto window_id = id_t{};
                auto footer = conio.footer.freeze();
                conio.footer_request.send(canal, window_id);
                footer.wait();
                f = footer.thing.utf8;
            };
            LISTEN(tier::request, e2::form::prop::ui::header, h)
            {
                auto window_id = id_t{};
                auto header = conio.header.freeze();
                conio.header_request.send(canal, window_id);
                header.wait();
                h = header.thing.utf8;
            };
            LISTEN(tier::preview, e2::form::prop::ui::footer, newfooter)
            {
                auto window_id = id_t{};
                conio.footer.send(canal, window_id, newfooter);
            };
            LISTEN(tier::preview, e2::form::prop::ui::header, newheader)
            {
                auto window_id = id_t{};
                conio.header.send(canal, window_id, newheader);
            };
            LISTEN(tier::release, input::events::clipboard, from_gear)
            {
                auto myid = from_gear.id;
                auto [ext_gear_id, gear_ptr] = get_ext_gear_id(myid);
                if (!gear_ptr) return;
                auto& gear =*gear_ptr;
                auto& data = gear.board::cargo;
                conio.clipdata.send(canal, ext_gear_id, data.hash, data.size, data.utf8, data.form, data.meta);
            };
            LISTEN(tier::request, input::events::clipboard, from_gear)
            {
                auto clipdata = conio.clipdata.freeze();
                auto myid = from_gear.id;
                auto [ext_gear_id, gear_ptr] = get_ext_gear_id(myid);
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
            LISTEN(tier::preview, input::events::mouse::button::tplclick::leftright, gear)
            {
                if (props.debug_overlay)
                {
                    props.show_regions = true;
                    props.debug_overlay = faux;
                    debug.stop();
                }
                else
                {
                    if (props.show_regions)
                    {
                        props.show_regions = faux;
                    }
                    else
                    {
                        props.debug_overlay = true;
                        debug.start();
                    }
                }
                gear.dismiss();
            };
            if (direct) // Forward unhandled events outside.
            {
                LISTEN(tier::preview, e2::form::size::minimize, gear)
                {
                    auto [ext_gear_id, gear_ptr] = get_ext_gear_id(gear.id);
                    if (gear_ptr) conio.minimize.send(canal, ext_gear_id);
                };
                LISTEN(tier::release, input::events::mouse::scroll::any, gear)
                {
                    auto [ext_gear_id, gear_ptr] = get_ext_gear_id(gear.id);
                    if (gear_ptr) conio.mouse_event.send(canal, ext_gear_id, gear.ctlstat, gear.mouse::cause, gear.coord, gear.delta.get(), gear.take_button_state(), gear.whlfp, gear.whlsi, gear.hzwhl, gear.click);
                    gear.dismiss();
                };
                LISTEN(tier::release, input::events::mouse::button::any, gear, -, (isvtm))
                {
                    namespace button = input::events::mouse::button;
                    auto forward = faux;
                    auto cause = gear.mouse::cause;
                    if (isvtm && (gear.index == hids::leftright || // Reserved for dragging nested vtm.
                                  gear.index == hids::right)       // Reserved for creation inside nested vtm.
                              && netxs::events::subevent(cause, button::drag::any.id))
                    {
                        return; // Pass event to the hall.
                    }
                    if (fullscreen && netxs::events::subevent(cause, button::drag::any.id)) // Enable left drag in GUI fullscreen mode.
                    {
                        return; // Pass event to the hall.
                    }
                    if (netxs::events::subevent(cause, button::click     ::any.id)
                     || netxs::events::subevent(cause, button::dblclick  ::any.id)
                     || netxs::events::subevent(cause, button::tplclick  ::any.id)
                     || netxs::events::subevent(cause, button::drag::pull::any.id))
                    {
                        gear.setfree();
                        forward = true;
                    }
                    else if (netxs::events::subevent(cause, button::drag::start::any.id))
                    {
                        gear.capture(bell::id); // To avoid unhandled mouse pull processing.
                        forward = true;
                    }
                    else if (netxs::events::subevent(cause, button::drag::cancel::any.id)
                          || netxs::events::subevent(cause, button::drag::stop  ::any.id))
                    {
                        gear.setfree();
                    }
                    if (forward)
                    {
                        auto [ext_gear_id, gear_ptr] = get_ext_gear_id(gear.id);
                        if (gear_ptr) conio.mouse_event.send(canal, ext_gear_id, gear.ctlstat, cause, gear.coord, gear.delta.get(), gear.take_button_state(), gear.whlfp, gear.whlsi, gear.hzwhl, gear.click);
                        gear.dismiss();
                    }
                };
                LISTEN(tier::release, e2::config::fps, fps)
                {
                    if (fps > 0) this->base::signal(tier::general, e2::config::fps, fps);
                };
                LISTEN(tier::preview, e2::form::prop::cwd, path)
                {
                    conio.cwd.send(canal, path);
                };
                LISTEN(tier::preview, input::events::mouse::button::click::any, gear)
                {
                    conio.expose.send(canal);
                };
                LISTEN(tier::preview, e2::form::layout::expose, item)
                {
                    conio.expose.send(canal);
                };
                LISTEN(tier::preview, e2::form::layout::swarp, warp)
                {
                    conio.warping.send(canal, 0, warp);
                };
                LISTEN(tier::preview, e2::form::size::enlarge::fullscreen, gear)
                {
                    auto [ext_gear_id, gear_ptr] = get_ext_gear_id(gear.id);
                    if (gear_ptr) conio.fullscrn.send(canal, ext_gear_id);
                };
                LISTEN(tier::preview, e2::form::size::enlarge::maximize, gear)
                {
                    auto [ext_gear_id, gear_ptr] = get_ext_gear_id(gear.id);
                    if (gear_ptr) conio.maximize.send(canal, ext_gear_id);
                };
            }
            LISTEN(tier::release, e2::conio::winsz, new_size)
            {
                auto delta = base::sizeby(new_size - base::size());
                if (delta && direct)
                {
                    base::ruined(true);
                    paint.cancel();
                }
                auto timestamp = datetime::now(); // Do not wait next timer tick.
                rebuild_scene(timestamp);
            };
            LISTEN(tier::general, e2::timer::any, timestamp)
            {
                rebuild_scene(timestamp);
            };
            conio.sysstart.send(canal);
        }
        // gate: Notify environment to disconnect.
        void disconnect()
        {
            if (alive.exchange(faux))
            {
                conio.s11n::sysclose.send(canal, true);
                canal.wake();
            }
        }
        // gate: .
        void inform(rect new_area) override
        {
            if (base::subset.size())
            if (auto object = base::subset.back())
            {
                object->base::resize(new_area.size);
            }
        }
    };
}