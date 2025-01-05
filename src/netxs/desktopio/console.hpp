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
            flag  alive; // link: sysclose isn't sent.
            wptr  owner_wptr; // link: .

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

            link(pipe& canal, gate& owner)
                : s11n{ *this },
                 canal{ canal },
                 owner{ owner },
                 alive{ true  }
            {
                auto oneshot = ptr::shared(hook{});
                owner.LISTEN(tier::anycast, e2::form::upon::started, root, *oneshot, (oneshot))
                {
                    owner_wptr = owner.This();
                    oneshot->reset();
                };
            }

            // link: Send an event message to the link owner.
            template<class E, class T>
            void notify(E, T&& data, si32 Tier = tier::release)
            {
                owner.bell::enqueue(owner_wptr, [Tier, d = data](auto& boss) mutable
                {
                    boss.bell::signal(Tier, E::id, d);
                });
            }
            void handle(s11n::xs::req_input_fields lock)
            {
                owner.bell::enqueue(owner_wptr, [&, item = lock.thing](auto& /*boss*/) mutable
                {
                    auto ext_gear_id = item.gear_id;
                    auto int_gear_id = owner.get_int_gear_id(ext_gear_id);
                    auto inputfield_request = owner.bell::signal(tier::general, ui::e2::command::request::inputfields, { .gear_id = int_gear_id, .acpStart = item.acpStart, .acpEnd = item.acpEnd }); // pro::focus retransmits as a tier::release for focused objects.
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
            //todo use s11n::xs::screenmode:  normal/fullscreen/maximized/mnimized
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
                notify(e2::conio::focus, item);
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
                disconnect();
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
                    selected          = config.take("/config/desktop/taskbar/selected", ""s);
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

        using depo = std::unordered_map<id_t, netxs::sptr<hids>>;

    public:
        pipe&      canal; // gate: Channel to outside.
        props_t    props; // gate: Input gate properties.
        diff       paint; // gate: Render.
        link       conio; // gate: Input data parser.
        bool       direct; // gate: .
        bool       local; // gate: .
        bool       yield; // gate: Indicator that the current frame has been successfully STDOUT'd.
        bool       fullscreen; // gate: .
        para       uname; // gate: Client name.
        text       uname_txt; // gate: Client name (original).
        sptr       applet; // gate: Standalone application.
        subs       tokens; // gate: Subscription tokens.
        wptr       nexthop; // gate: .
        face       xmap; // gate: .
        depo       gears; // gate: .

        void forward(auto& device)
        {
            auto gear_it = gears.find(device.gear_id);
            if (gear_it == gears.end())
            {
                gear_it = gears.emplace(device.gear_id, bell::create<hids>(*this, xmap)).first;
                auto& gear = *(gear_it->second);
                gear.tooltip_timeout = props.tooltip_timeout;
                gear.board::ghost = props.clip_preview_glow;
                gear.board::brush = props.clip_preview_clrs;
                gear.board::alpha = props.clip_preview_alfa;
                gear.mouse::delay = props.dblclick_timeout;

                static auto proc_map = pro::luafx::fxmap<hids>
                {
                    { "IsKeyRepeated", [](hids& gear, auto lua)
                                       {
                                           auto repeated = gear.keystat == input::key::repeated;
                                           ::lua_settop(lua, 0);
                                           ::lua_pushboolean(lua, repeated);
                                       }},
                };
                gear.base::plugin<pro::luafx>(proc_map);
            }
            auto& [_id, gear_ptr] = *gear_it;
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
        void draw_foreign_names(face& parent_canvas)
        {
            auto& header = *uname.lyric;
            auto  half_x = header.size().x / 2;
            for (auto& [ext_gear_id, gear_ptr] : gears)
            {
                auto& gear = *gear_ptr;
                if (gear.mouse_disabled) continue;
                auto coor = twod{ gear.coord };
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
            for (auto& [ext_gear_id, gear_ptr] : gears)
            {
                auto& gear = *gear_ptr;
                if (gear.mouse_disabled) continue;
                area.coor = gear.coord;
                auto brush = gear.m_sys.buttons ? cell{ busy }.txt(64 + (char)gear.m_sys.buttons/*A-Z*/)
                                                : idle;
                canvas.fill(area, cell::shaders::fuse(brush));
            }
        }
        void draw_clipboard_preview(face& canvas, time const& stamp)
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
        void draw_tooltips(face& canvas, time const& stamp)
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
        void check_tooltips(time now)
        {
            auto result = faux;
            for (auto& [ext_gear_id, gear_ptr] : gears)
            {
                auto& gear = *gear_ptr;
                if (gear.mouse_disabled) continue;
                result |= gear.tooltip_check(now);
            }
            if (result) base::strike();
        }

        // gate: Attach a new item.
        auto attach(sptr& item)
        {
            std::swap(applet, item);
            if (local) nexthop = applet;
            applet->bell::signal(tier::release, e2::form::upon::vtree::attached, This());
        }
        // gate: .
        void _rebuild_scene(bool damaged)
        {
            auto stamp = datetime::now();
            auto& canvas = xmap;
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
                if (props.debug_overlay)
                {
                    auto& debug = plugins<pro::debug>();
                    debug.output(canvas);
                    if constexpr (debugmode) // Red channel histogram.
                    if (gears.size())
                    {
                        auto& [gear_id, gear_ptr] = *gears.begin();
                        if (gear_ptr->meta(hids::ScrlLock)) 
                        {
                            auto hist = page{};
                            hist.brush.bgc(0x80ffffff);
                            auto full = canvas.full();
                            auto area = canvas.area();
                            canvas.area({ dot_00, area.size });
                            auto coor = gear_ptr->coord;
                            for (auto x = 0; x < area.size.y; x++)
                            {
                                auto xy = coor + twod{ x - area.size.y/2, 0 };
                                auto has_value = xy.x > 0 && xy.x < canvas.size().x;
                                if (has_value) utf::repeat(" ", canvas[xy].bgc().chan.r);
                                hist += "\n"s;
                            }
                            auto full_area = full;
                            full_area.coor = {};
                            full_area.size.x = dot_mx.x; // Prevent line wrapping.
                            canvas.full(full_area);
                            canvas.cup(dot_00);
                            canvas.output(hist, cell::shaders::blend);
                            canvas.area(area);
                            canvas.full(full);
                        }
                    }
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

            // Note: We have to fire a mouse move event every frame,
            //       because in the global frame the mouse can stand still,
            //       but any form can move under the cursor, so for the form itself,
            //       the mouse cursor moves inside the form.
            if (props.debug_overlay)
            {
                auto& debug = plugins<pro::debug>();
                debug.bypass = true;
                fire(hids::events::mouse::move.id);
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
                fire(hids::events::mouse::move.id);
                yield = paint.commit(canvas); // Try output my canvas to the my console.
            }
        }
        // gate: .
        void rebuild_scene(id_t world_id, bool damaged)
        {
            if (damaged)
            {
                xmap.wipe(world_id);
                if (applet)
                if (auto context = xmap.change_basis(base::area()))
                {
                    applet->render(xmap);
                }
            }
            _rebuild_scene(damaged);
        }
        // gate: Main loop.
        void launch()
        {
            bell::signal(tier::anycast, e2::form::upon::started, This()); // Make all stuff ready to receive input.
            conio.run();
            bell::signal(tier::release, e2::form::upon::stopped, true);
        }

        //todo revise
        gate(xipc uplink, si32 vtmode, xmls& config, view userid = {}, si32 session_id = 0, bool isvtm = faux)
            : canal{ *uplink },
              props{ canal, userid, vtmode, isvtm, session_id, config },
              paint{ canal, props.vtmode },
              conio{ canal, *this  },
              direct{ !!(vtmode & (ui::console::direct | ui::console::gui)) },
              local{ true },
              yield{ faux },
              fullscreen{ faux }
        {
            //todo
            //auto& focus = plugins<pro::focus>();
            //todo scripting
            //keybd.proc("ToggleDebugOverlay", [&](hids& gear)
            //{
            //    gear.set_handled();
            //    auto& debug = plugins<pro::debug>();
            //    props.debug_overlay ? debug.stop() : debug.start();
            //    props.debug_overlay = !props.debug_overlay;
            //});
            auto& keybd = plugins<pro::keybd>("gate");
            auto& mouse = plugins<pro::mouse>();
            auto& debug = plugins<pro::debug>();
            auto bindings = pro::keybd::load(config, "tui"); //todo -> pro::keybd("gate")
            keybd.bind(bindings);

            base::root(true);
            base::limits(dot_11);

            xmap.cmode = props.vtmode;
            xmap.mark(props.background_color.txt(whitespace).link(bell::id));
            xmap.face::area(base::area());
            LISTEN(tier::release, e2::command::printscreen, gear, tokens)
            {
                auto data = escx{};
                data.s11n(xmap, gear.slot);
                if (data.length())
                {
                    if (props.clip_prtscrn_mime != mime::disabled)
                    {
                        gear.set_clipboard(gear.slot.size, data, props.clip_prtscrn_mime);
                    }
                }
            };
            LISTEN(tier::release, e2::form::prop::filler, new_filler, tokens)
            {
                xmap.mark(new_filler);
            };
            LISTEN(tier::release, e2::area, new_area, tokens)
            {
                xmap.face::area(new_area);
            };
            LISTEN(tier::release, e2::command::run, script)
            {
                if (auto world_ptr = parent())
                {
                    //todo set gear_id
                    world_ptr->bell::signal(tier::release, e2::command::run, script);
                }
            };
            LISTEN(tier::release, e2::conio::mouse, m, tokens)
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
            LISTEN(tier::release, e2::conio::keybd, k, tokens)
            {
                forward(k);
            };
            LISTEN(tier::release, e2::conio::focus, f, tokens)
            {
                forward(f);
            };
            LISTEN(tier::release, e2::conio::board, c, tokens)
            {
                forward(c);
            };
            LISTEN(tier::preview, hids::events::focus::set::any, seed, tokens)
            {
                if (seed.gear_id)
                {
                    auto [ext_gear_id, gear_ptr] = get_ext_gear_id(seed.gear_id);
                    if (gear_ptr)
                    {
                        auto deed = bell::protos(tier::preview);
                        auto state = deed == hids::events::focus::set::on.id;
                        conio.sysfocus.send(canal, ext_gear_id, state, seed.focus_type, ui64{}, ui64{});
                    }
                }
            };
            //todo mimic pro::focus
            LISTEN(tier::release, hids::events::focus::any, seed, tokens)
            {
                if (auto target = nexthop.lock())
                {
                    auto deed = bell::protos(tier::release);
                    target->bell::signal(tier::release, deed, seed);
                }
            };
            //todo mimic pro::focus
            //if (standalone)
            {
                LISTEN(tier::request, e2::config::plugins::focus::owner, owner_ptr, tokens)
                {
                    owner_ptr = This();
                };
            }
            LISTEN(tier::preview, hids::events::keybd::key::post, gear, tokens) // Start of kb event propagation.
            {
                if (auto target = nexthop.lock())
                {
                    target->bell::signal(tier::preview, hids::events::keybd::key::post, gear);
                }
            };
            LISTEN(tier::release, hids::events::keybd::any, gear, tokens) // Forward unhandled events to the outside. Return back unhandled keybd events.
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
                viewport = base::area();
            };
            //todo unify creation (delete simple create wo gear)
            LISTEN(tier::preview, e2::form::proceed::create, dest_region, tokens)
            {
                dest_region.coor += base::coor();
                this->base::riseup(tier::release, e2::form::proceed::create, dest_region);
            };
            LISTEN(tier::release, e2::form::proceed::onbehalf, proc, tokens)
            {
                //todo hids
                //proc(gear);
            };
            LISTEN(tier::preview, hids::events::mouse::button::click::leftright, gear, tokens)
            {
                if (gear.clear_clipboard())
                {
                    this->bell::expire(tier::release);
                    gear.dismiss();
                }
            };
            LISTEN(tier::release, e2::conio::winsz, new_size, tokens)
            {
                auto new_area = rect{ dot_00, new_size };
                if (applet) applet->bell::signal(tier::anycast, e2::form::upon::resized, new_area);
                auto old_size = base::size();
                auto delta = base::resize(new_size).size - old_size;
                if (delta && direct) paint.cancel();
            };
            LISTEN(tier::release, e2::conio::pointer, pointer, tokens)
            {
                props.legacy_mode |= pointer ? ui::console::mouse : 0;
            };
            LISTEN(tier::release, e2::form::upon::stopped, fast, tokens) // Reading loop ends.
            {
                this->bell::signal(tier::anycast, e2::form::proceed::quit::one, fast);
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
            LISTEN(tier::anycast, e2::form::upon::started, item_ptr, tokens)
            {
                if (props.debug_overlay) debug.start();
                this->bell::signal(tier::release, e2::form::prop::name, props.title);
                //todo revise
                if (props.title.length())
                {
                    this->base::riseup(tier::preview, e2::form::prop::ui::header, props.title);
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
            LISTEN(tier::release, hids::events::clipboard, from_gear, tokens)
            {
                auto myid = from_gear.id;
                auto [ext_gear_id, gear_ptr] = get_ext_gear_id(myid);
                if (!gear_ptr) return;
                auto& gear =*gear_ptr;
                auto& data = gear.board::cargo;
                conio.clipdata.send(canal, ext_gear_id, data.hash, data.size, data.utf8, data.form, data.meta);
            };
            LISTEN(tier::request, hids::events::clipboard, from_gear, tokens)
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
            LISTEN(tier::preview, hids::events::mouse::button::tplclick::leftright, gear, tokens)
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
            if (props.tooltip_enabled)
            {
                LISTEN(tier::general, e2::timer::any, now, tokens)
                {
                    check_tooltips(now);
                };
            }
            if (direct) // Forward unhandled events outside.
            {
                LISTEN(tier::release, e2::form::size::minimize, gear, tokens)
                {
                    auto [ext_gear_id, gear_ptr] = get_ext_gear_id(gear.id);
                    if (gear_ptr) conio.minimize.send(canal, ext_gear_id);
                };
                LISTEN(tier::release, hids::events::mouse::scroll::any, gear, tokens)
                {
                    auto [ext_gear_id, gear_ptr] = get_ext_gear_id(gear.id);
                    if (gear_ptr) conio.mouse_event.send(canal, ext_gear_id, gear.ctlstat, gear.mouse::cause, gear.coord, gear.delta.get(), gear.take_button_state(), gear.whlfp, gear.whlsi, gear.hzwhl, gear.click);
                    gear.dismiss();
                };
                LISTEN(tier::release, hids::events::mouse::button::any, gear, tokens, (isvtm))
                {
                    using button = hids::events::mouse::button;
                    auto forward = faux;
                    auto cause = gear.mouse::cause;
                    if (isvtm && (gear.index == hids::leftright || // Reserved for dragging nested vtm.
                                  gear.index == hids::right)       // Reserved for creation inside nested vtm.
                              && events::subevent(cause, button::drag::any.id))
                    {
                        return; // Pass event to the hall.
                    }
                    if (fullscreen && events::subevent(cause, button::drag::any.id)) // Enable left drag in fullscreen mode.
                    {
                        return; // Pass event to the hall.
                    }
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
                        auto [ext_gear_id, gear_ptr] = get_ext_gear_id(gear.id);
                        if (gear_ptr) conio.mouse_event.send(canal, ext_gear_id, gear.ctlstat, cause, gear.coord, gear.delta.get(), gear.take_button_state(), gear.whlfp, gear.whlsi, gear.hzwhl, gear.click);
                        gear.dismiss();
                    }
                };
                LISTEN(tier::release, e2::config::fps, fps, tokens)
                {
                    if (fps > 0) this->bell::signal(tier::general, e2::config::fps, fps);
                };
                LISTEN(tier::preview, e2::config::fps, fps, tokens)
                {
                    conio.fps.send(canal, fps);
                };
                LISTEN(tier::preview, e2::form::prop::cwd, path, tokens)
                {
                    conio.cwd.send(canal, path);
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
                LISTEN(tier::preview, e2::form::size::enlarge::fullscreen, gear, tokens)
                {
                    auto [ext_gear_id, gear_ptr] = get_ext_gear_id(gear.id);
                    if (gear_ptr) conio.fullscrn.send(canal, ext_gear_id);
                };
                LISTEN(tier::preview, e2::form::size::enlarge::maximize, gear, tokens)
                {
                    auto [ext_gear_id, gear_ptr] = get_ext_gear_id(gear.id);
                    if (gear_ptr) conio.maximize.send(canal, ext_gear_id);
                };
            }
            conio.sysstart.send(canal);
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
        struct luna
        {
            std::vector<std::pair<sptr, qiew>> object_list; // luna: .
            lua_State* lua; // luna: .

            auto vtmlua_getval()
            {
                auto n = ::lua_gettop(lua);
                auto crop = ansi::escx{};
                for (auto i = 1; i <= n; i++)
                {
                    auto t = ::lua_type(lua, i);
                    switch (t)
                    {
                        case LUA_TBOOLEAN:
                        case LUA_TNUMBER:
                        case LUA_TSTRING: crop.add(::lua_torawstring(lua, i)); break;
                        default:          crop.add('<', ::lua_typename(lua, t), '>'); break;
                    }
                }
                log("", crop);
                return 0;
            }
            static auto vtmlua_log(lua_State* lua)
            {
                auto n = ::lua_gettop(lua);
                auto crop = ansi::escx{};
                for (auto i = 1; i <= n; i++)
                {
                    auto t = ::lua_type(lua, i);
                    switch (t)
                    {
                        case LUA_TBOOLEAN:
                        case LUA_TNUMBER:
                        case LUA_TSTRING: crop.add(::lua_torawstring(lua, i)); break;
                        default:          crop.add('<', ::lua_typename(lua, t), '>'); break;
                    }
                }
                log("", crop);
                return 0;
            }
            static auto vtmlua_call(lua_State* lua) // UpValue[1]: Object_ptr. UpValue[2]: Function_name.
            {
                // Stack:
                //      lua_upvalueindex(1): Get Object_ptr.
                //      lua_upvalueindex(2): Fx name.
                //      1. args:        ...
                //      2.     :        ...
                if (auto object_ptr = (bell*)::lua_touserdata(lua, lua_upvalueindex(1))) // Get Object_ptr.
                {
                    object_ptr->bell::signal(tier::release, e2::luafx, lua);
                }
                return ::lua_gettop(lua);
            }
            static auto vtmlua_index(lua_State* lua)
            {
                // Stack:
                //      1. userdata (or table).
                //      2. fx name (keyname).
                ::lua_pushcclosure(lua, vtmlua_call, 2);
                return 1;
            }
            static auto vtmlua_tostring(lua_State* lua)
            {
                auto crop = text{};
                if (auto object_ptr = (bell*)::lua_touserdata(lua, -1)) // Get Object_ptr.
                {
                    crop = utf::concat("<object:", object_ptr->id, ">");
                }
                else crop = "<object>";
                ::lua_pushstring(lua, crop.data());
                return 1;
            }
            auto setobject(sptr object_ptr, qiew object_name)
            {
                if (object_ptr)
                {
                    object_list.push_back({ object_ptr, object_name });
                    ::lua_pushlightuserdata(lua, object_ptr.get()); // Object ptr.
                    ::luaL_setmetatable(lua, "vtmmetatable"); // Set the metatable.
                    ::lua_setglobal(lua, object_name.data()); // Set global var.
                }
            }
            auto logcontext()
            {
                log("%%Context:", prompt::lua);
                lua_pushglobaltable(lua);
                ::lua_pushnil(lua);
                while (::lua_next(lua, -2))
                {
                    auto var = ::lua_torawstring(lua, -2);
                    auto val = ::lua_torawstring(lua, -1);
                    if (val.size()) log("%%%name% = %value%", prompt::pads, utf::adjust(var, 11, ' ', true), val);
                    ::lua_pop(lua, 1); // Pop val.
                }
                ::lua_pop(lua, 1); // Pop table.
            }
            auto runscript(auto& script_body, auto& scripting_context)
            {
                //log("  script body: ", ansi::hi(script_body));
                for (auto& [object_name, object_wptr] : scripting_context)
                {
                    if (auto object_ptr = object_wptr.lock())
                    {
                        setobject(object_ptr, object_name);
                        object_list.push_back({ object_ptr, object_name });
                    }
                }
                logcontext();
                ::lua_settop(lua, 0);
                auto error = ::luaL_loadbuffer(lua, script_body.data(), script_body.size(), "script body")
                          || ::lua_pcall(lua, 0, 0, 0);
                auto result = text{};
                if (error)
                {
                    result = ::lua_tostring(lua, -1);
                    log("%%%msg%", prompt::lua, ansi::err(result));
                    ::lua_pop(lua, 1);  // Pop error message from stack.
                }
                else if (::lua_gettop(lua))
                {
                    result = ::lua_tostring(lua, -1);
                }
                for (auto& [object_ptr, object_name] : object_list) // Wipe global context.
                {
                    ::lua_pushnil(lua);
                    ::lua_setglobal(lua, object_name.data());
                }
                object_list.clear();
                return result;
            }

            luna()
                : lua{ ::luaL_newstate() }
            {
                ::luaL_openlibs(lua);
                ::lua_pushcclosure(lua, vtmlua_log, 0);
                ::lua_setglobal(lua, "log");
                static auto metalist = std::to_array<luaL_Reg>({{ "__index", vtmlua_index },
                                                                { "__tostring", vtmlua_tostring },
                                                                { nullptr, nullptr }});
                ::luaL_newmetatable(lua, "vtmmetatable"); // Create a new metatable in registry and push it to the stack.
                ::luaL_setfuncs(lua, metalist.data(), 0); // Assign metamethods for the table which at the top of the stack.
            }
            ~luna()
            {
                if (lua) ::lua_close(lua);
            }
        };

    public:
        using tick = datetime::quartz<bell, tier::general, e2::timer::tick.id>;

        tick quartz; // host: Frame rate synchronizator.
        si32 maxfps; // host: Frame rate.
        regs debris; // host: Wrecked regions.
        xmls config; // host: Resultant settings.
        subs tokens; // host: Subscription tokens.
        flag active; // host: Host is available for connections.
        luna vtmlua; // host: Lua scripting.

        std::vector<bool> user_numbering; // host: .

        host(xipc server, xmls config, si32 focus_type = pro::focus::mode::hub)
            : quartz{ *this },
              config{ config },
              active{ true }
        {
            plugins<pro::focus>(focus_type, faux);
            plugins<pro::keybd>("vtm");

            using namespace std::chrono;
            auto& canal = *server;

            auto& g = skin::globals();
            g.window_clr     = config.take("/config/colors/window"     , cell{ whitespace });
            g.winfocus       = config.take("/config/colors/focus"      , cell{ whitespace });
            g.brighter       = config.take("/config/colors/brighter"   , cell{ whitespace });
            g.shadower       = config.take("/config/colors/shadower"   , cell{ whitespace });
            g.warning        = config.take("/config/colors/warning"    , cell{ whitespace });
            g.danger         = config.take("/config/colors/danger"     , cell{ whitespace });
            g.action         = config.take("/config/colors/action"     , cell{ whitespace });

            g.selected       = config.take("/config/desktop/taskbar/colors/selected"  , cell{ whitespace });
            g.active         = config.take("/config/desktop/taskbar/colors/active"    , cell{ whitespace });
            g.focused        = config.take("/config/desktop/taskbar/colors/focused"   , cell{ whitespace });
            g.inactive       = config.take("/config/desktop/taskbar/colors/inactive"  , cell{ whitespace });

            g.spd            = config.take("/config/timings/kinetic/spd"      , 10  );
            g.pls            = config.take("/config/timings/kinetic/pls"      , 167 );
            g.spd_accel      = config.take("/config/timings/kinetic/spd_accel", 1   );
            g.spd_max        = config.take("/config/timings/kinetic/spd_max"  , 100 );
            g.ccl            = config.take("/config/timings/kinetic/ccl"      , 120 );
            g.ccl_accel      = config.take("/config/timings/kinetic/ccl_accel", 30  );
            g.ccl_max        = config.take("/config/timings/kinetic/ccl_max"  , 1   );
            g.switching      = config.take("/config/timings/switching"        , span{ 200ms });
            g.deceleration   = config.take("/config/timings/deceleration"     , span{ 2s    });
            g.blink_period   = config.take("/config/cursor/blink"             , span{ 400ms });
            g.menu_timeout   = config.take("/config/desktop/taskbar/timeout"  , span{ 250ms });
            g.leave_timeout  = config.take("/config/timings/leave_timeout"    , span{ 1s    });
            g.repeat_delay   = config.take("/config/timings/repeat_delay"     , span{ 500ms });
            g.repeat_rate    = config.take("/config/timings/repeat_rate"      , span{ 30ms  });
            g.max_value      = config.take("/config/desktop/windowmax"        , twod{ 3000, 2000  });
            g.macstyle       = config.take("/config/desktop/macstyle"         , faux);
            g.menuwide       = config.take("/config/desktop/taskbar/wide"     , faux);

            g.shadow_enabled = config.take("/config/desktop/shadow/enabled", true);
            g.shadow_bias    = config.take("/config/desktop/shadow/bias"   , 0.37f);
            g.shadow_blur    = config.take("/config/desktop/shadow/blur"   , 3);
            g.shadow_opacity = config.take("/config/desktop/shadow/opacity", 105.5f);
            g.shadow_offset  = config.take("/config/desktop/shadow/offset" , dot_21);

            maxfps = config.take("/config/timings/fps", 60);
            if (maxfps <= 0) maxfps = 60;

            LISTEN(tier::release, e2::command::run, script)
            {
                auto scripting_context = std::unordered_map<text, wptr>{};
                if (script.cwd.size())
                {
                    //todo os::cwd(script.cwd);
                }
                if (utf::trim(script.cmd, " \r\n\t\f").empty()) return;
                vtmlua.setobject(This(), "vtm");
                if (script.gear_id)
                if (auto gear_ptr = bell::getref<hids>(script.gear_id))
                {
                    vtmlua.setobject(gear_ptr, "gear");
                }
                auto result = vtmlua.runscript(script.cmd, scripting_context);
                if (result.empty()) result = "ok";
                log(ansi::clr(yellowlt, script.cmd, ": "), result);
                script.cmd += ": " + result + '\n';
            };
            LISTEN(tier::preview, e2::runscript, gear)
            {
                if (!gear.action_ptr) return;
                if (!gear.scripting_context_ptr) return;
                auto& script_body = *gear.action_ptr;
                auto& scripting_context = *gear.scripting_context_ptr;
                vtmlua.setobject(gear.This(), "gear");
                vtmlua.runscript(script_body, scripting_context);
            };
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
                this->router(tier::general).cleanup(counter.ref_count, counter.del_count);
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
            LISTEN(tier::request, hids::events::focus::set::any, seed, tokens, (focus_tree_map = std::unordered_map<ui64, ui64>{})) // Filter recursive focus loops.
            {
                auto is_recursive = faux;
                if (seed.treeid)
                {
                    auto& digest = focus_tree_map[seed.treeid];
                    if (digest < seed.digest) // This is the first time this focus event has been received.
                    {
                        digest = seed.digest;
                    }
                    else // We've seen this event before.
                    {
                        is_recursive = true;
                    }
                }
                if (!is_recursive)
                {
                    auto deed = this->bell::protos(tier::request);
                    this->bell::signal(tier::release, deed, seed);
                }
            };

            quartz.ignite(maxfps);
            log(prompt::host, "Rendering refresh rate: ", maxfps, " fps");
        }

        // host: Mark dirty region.
        void denote(rect updateregion)
        {
            if (updateregion)
            {
                debris.push_back(updateregion);
            }
        }
        void deface(rect damaged_region) override
        {
            base::deface(damaged_region);
            denote(damaged_region);
        }
        // host: Create a new root of the specified subtype and attach it.
        auto invite(xipc uplink, sptr& applet, si32 vtmode, twod winsz)
        {
            auto lock = bell::unique_lock();
            auto portal = ui::gate::ctor(uplink, vtmode, host::config);
            portal->bell::signal(tier::release, e2::form::upon::vtree::attached, base::This());
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
            bell::dequeue();
            quartz.stop();
        }
        // host: Shutdown.
        void stop()
        {
            auto lock = bell::sync();
            auto& mouse = plugins<pro::mouse>();
            mouse.reset();
            tokens.reset();
        }
    };
}