// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#include "netxs/desktopio/application.hpp"

namespace netxs::app::vtm
{
    static constexpr auto id = "vtm";
    static constexpr auto desc = " vtm:";

    struct menuitem_t
    {
        text       id{};
        text    alias{};
        bool   hidden{};
        text    label{};
        text    notes{};
        text    title{};
        text   footer{};
        rgba      bgc{};
        rgba      fgc{};
        twod  winsize{};
        twod  wincoor{};
        bool slimmenu{};
        bool splitter{};
        text   hotkey{};
        text      cwd{};
        text     type{};
        text    param{};
        text    patch{};
        xml::settings settings;
    };

    using links_t = std::unordered_map<text, menuitem_t>;
    using registry_t = generics::imap<text, std::pair<bool, std::list<sptr<base>>>>;

    static constexpr auto attr_id       = "id";
    static constexpr auto attr_alias    = "alias";
    static constexpr auto attr_hidden   = "hidden";
    static constexpr auto attr_label    = "label";
    static constexpr auto attr_notes    = "notes";
    static constexpr auto attr_title    = "title";
    static constexpr auto attr_footer   = "footer";
    static constexpr auto attr_bgc      = "bgc";
    static constexpr auto attr_fgc      = "fgc";
    static constexpr auto attr_winsize  = "winsize";
    static constexpr auto attr_wincoor  = "wincoor";
    static constexpr auto attr_focused  = "focused";
    static constexpr auto attr_slimmenu = "slimmenu";
    static constexpr auto attr_hotkey   = "hotkey";
    static constexpr auto attr_type     = "type";
    static constexpr auto attr_cwd      = "cwd";
    static constexpr auto attr_param    = "param";
    static constexpr auto attr_splitter = "splitter";
    static constexpr auto attr_config   = "config";

    static constexpr auto path_item     = "/config/menu/item";
    static constexpr auto path_autorun  = "/config/menu/autorun/item";

    // vtm: Desktopio Workspace.
    struct hall
        : public host
    {
        struct bindings
        {
            EVENTPACK( bindings, ui::e2::extra )
            {
                GROUP_XS( list, si32 ), // UI-tree pre-rendering, used by pro::cache (can interrupt SIGNAL) and any kind of highlighters, release only.

                SUBSET_XS( list )
                {
                    EVENT_XS( users, sptr<std::list<sptr<ui::base>>> ), // list of connected users.
                    EVENT_XS( apps , sptr<registry_t>                ), // list of running apps.
                    EVENT_XS( links, sptr<links_t>                   ), // list of registered apps.
                };
            };
        };

    private:
        class node // hall: Helper-class for the pro::scene. Adapter for the object that going to be attached to the scene.
        {
            struct ward
            {
                enum states
                {
                    unused_hidden, // 00
                    unused_usable, // 01
                    active_hidden, // 10
                    active_usable, // 11
                    count
                };

                para title[states::count];
                cell brush[states::count];
                para basis;
                bool usable = faux;
                bool highlighted = faux;
                si32 active = 0;
                tone color;

                operator bool ()
                {
                    return basis.size() != dot_00;
                }
                void set(para const& caption)
                {
                    basis = caption;
                    basis.decouple();
                    recalc();
                }
                auto& get()
                {
                    return title[(active || highlighted ? 2 : 0) + usable];
                }
                void recalc()
                {
                    brush[active_hidden] = skin::color(color.active);
                    brush[active_usable] = skin::color(color.active);
                    brush[unused_hidden] = skin::color(color.passive);
                    brush[unused_usable] = skin::color(color.passive);

                    brush[unused_usable].bga(brush[unused_usable].bga() << 1);

                    auto i = 0;
                    for (auto& label : title)
                    {
                        auto& c = brush[i++];
                        label = basis;
                        label.decouple();

                        //todo unify clear formatting/aligning in header
                        label.locus.kill();
                        label.style.rst();
                        label.lyric->each([&](auto& a) { a.meta(c); });
                    }
                }
            };

            using sptr = netxs::sptr<base>;

            ward header;

        public:
            rect region;
            sptr object;
            id_t obj_id;
            si32 z_order = Z_order::plain;

            node(sptr item)
                : object{ item }
            {
                auto& inst = *item;
                obj_id = inst.bell::id;

                inst.LISTEN(tier::release, e2::form::prop::zorder, order)
                {
                    z_order = order;
                };
                inst.LISTEN(tier::release, e2::size::any, size)
                {
                    region.size = size;
                };
                inst.LISTEN(tier::release, e2::coor::any, coor)
                {
                    region.coor = coor;
                };
                inst.LISTEN(tier::release, e2::form::state::mouse, state)
                {
                    header.active = state;
                };
                inst.LISTEN(tier::release, e2::form::highlight::any, state)
                {
                    header.highlighted = state;
                };
                inst.LISTEN(tier::release, e2::form::state::header, caption)
                {
                    header.set(caption);
                };
                inst.LISTEN(tier::release, e2::form::state::color, color)
                {
                    header.color = color;
                };

                inst.SIGNAL(tier::request, e2::size::set,  region.size);
                inst.SIGNAL(tier::request, e2::coor::set,  region.coor);
                inst.SIGNAL(tier::request, e2::form::state::mouse,  header.active);
                inst.SIGNAL(tier::request, e2::form::state::header, header.basis);
                inst.SIGNAL(tier::request, e2::form::state::color,  header.color);

                header.recalc();
            }
            // hall::node: Check equality.
            bool equals(id_t id)
            {
                return obj_id == id;
            }
            // hall::node: Draw a navigation string.
            void fasten(face& canvas)
            {
                auto window = canvas.area();
                auto origin = window.size / 2;
                //auto origin = twod{ 6, window.size.y - 3 };
                auto offset = region.coor - window.coor;
                auto center = offset + (region.size / 2);
                header.usable = window.overlap(region);
                auto active = header.active || header.highlighted;
                auto& grade = skin::grade(active ? header.color.active
                                                 : header.color.passive);
                auto pset = [&](twod const& p, uint8_t k)
                {
                    //canvas[p].fuse(grade[k], obj_id, p - offset);
                    //canvas[p].fuse(grade[k], obj_id);
                    canvas[p].link(obj_id).bgc().mix_one(grade[k].bgc());
                };
                window.coor = dot_00;
                netxs::online(window, origin, center, pset);
            }
            // hall::node: Visualize the underlying object.
            template<bool Post = true>
            void render(face& canvas)
            {
                canvas.render<Post>(*object);
            }
            void postrender(face& canvas)
            {
                object->SIGNAL(tier::release, e2::postrender, canvas);
            }
        };

        class list // hall: Helper-class. List of objects that can be reordered, etc.
        {
            std::list<sptr<node>> items;

            template<class D>
            auto search(D head, D tail, id_t id)
            {
                if (items.size())
                {
                    auto test = [id](auto& a) { return a->equals(id); };
                    return std::find_if(head, tail, test);
                }
                return tail;
            }

        public:
            operator bool () { return items.size(); }
            auto size()      { return items.size(); }
            auto back()      { return items.back()->object; }
            void append(sptr<base> item)
            {
                items.push_back(std::make_shared<node>(item));
            }
            //hall::list: Draw backpane for spectators.
            void prerender(face& canvas)
            {
                for (auto& item : items) item->fasten(canvas); // Draw strings.
                for (auto& item : items) item->render<faux>(canvas); // Draw shadows without postrendering.
            }
            //hall::list: Draw windows.
            void render(face& canvas)
            {
                for (auto& item : items) item->fasten(canvas);
                //todo optimize
                for (auto& item : items) if (item->z_order == Z_order::backmost) item->render(canvas);
                for (auto& item : items) if (item->z_order == Z_order::plain   ) item->render(canvas);
                for (auto& item : items) if (item->z_order == Z_order::topmost ) item->render(canvas);
            }
            //hall::list: Draw spectator's mouse pointers.
            void postrender(face& canvas)
            {
                for (auto& item : items) item->postrender(canvas);
            }
            //hall::list: Delete all items.
            void reset()
            {
                items.clear();
            }
            rect remove(id_t id)
            {
                auto area = rect{};
                auto head = items.begin();
                auto tail = items.end();
                auto item = search(head, tail, id);
                if (item != tail)
                {
                    area = (**item).region;
                    items.erase(item);
                }
                return area;
            }
            rect bubble(id_t id)
            {
                auto head = items.rbegin();
                auto tail = items.rend();
                auto item = search(head, tail, id);

                if (item != head && item != tail)
                {
                    auto& area = (**item).region;
                    if (!area.clip((**std::prev(item)).region))
                    {
                        auto shadow = *item;
                        items.erase(std::next(item).base());

                        while (--item != head
                            && !area.clip((**std::prev(item)).region))
                        { }

                        items.insert(item.base(), shadow);
                        return area;
                    }
                }

                return rect_00;
            }
            rect expose(id_t id)
            {
                auto head = items.rbegin();
                auto tail = items.rend();
                auto item = search(head, tail, id);

                if (item != head && item != tail)
                {
                    auto shadow = *item;
                    items.erase(std::next(item).base());
                    items.push_back(shadow);
                    return shadow->region;
                }

                return rect_00;
            }
            auto rotate_next()
            {
                items.push_back(items.front());
                items.pop_front();
                return items.back();
            }
            auto rotate_prev()
            {
                items.push_front(items.back());
                items.pop_back();
                return items.back();
            }
        };

        class depo // hall: Helper-class. Actors registry.
        {
        public:
            sptr<registry_t>            app_ptr = std::make_shared<registry_t>();
            sptr<std::list<sptr<base>>> usr_ptr = std::make_shared<std::list<sptr<base>>>();
            sptr<links_t>               lnk_ptr = std::make_shared<links_t>();
            registry_t&                 app = *app_ptr;
            std::list<sptr<base>>&      usr = *usr_ptr;
            links_t&                    lnk = *lnk_ptr;

            auto remove(sptr<base> item_ptr)
            {
                auto found = faux;
                // Remove from active app registry.
                for (auto& [class_id, fxd_app_list] : app)
                {
                    auto& [fixed, app_list] = fxd_app_list;
                    auto head = app_list.begin();
                    auto tail = app_list.end();
                    auto iter = std::find_if(head, tail, [&](auto& c) { return c == item_ptr; });
                    if (iter != tail)
                    {
                        app_list.erase(iter);
                        if (app_list.empty() && !fixed)
                        {
                            app.erase(class_id);
                        }
                        found = true;
                        break;
                    }
                }
                { // Remove user.
                    auto head = usr.begin();
                    auto tail = usr.end();
                    auto iter = std::find_if(head, tail, [&](auto& c){ return c == item_ptr; });
                    if (iter != tail)
                    {
                        usr.erase(iter);
                        found = true;
                    }
                }
                return found;
            }
            void reset()
            {
                app_ptr.reset();
            }
        };

        using idls = std::unordered_map<id_t, std::list<id_t>>;

        list items; // hall: Child visual tree.
        list users; // hall: Scene spectators.
        depo regis; // hall: Actors registry.
        idls taken; // hall: Focused objects for the last user.

    public:
        //todo use events
        static auto& configs()
        {
            auto world_ptr = e2::config::creator.param();
            SIGNAL_GLOBAL(e2::config::creator, world_ptr);
            auto conf_list_ptr = hall::bindings::list::links.param();
            world_ptr->SIGNAL(tier::request, hall::bindings::list::links, conf_list_ptr);
            auto& conf_list = *conf_list_ptr;
            return conf_list;
        }
        //todo use events
        static auto newapp(auto& menuid)
        {
            auto& conf_list = vtm::hall::configs();
            auto& config = conf_list[menuid];
            auto& creator = app::shared::create::builder(config.type);
            auto object = creator(config.cwd, config.param, config.settings, config.patch);
            if (config.bgc     ) object->SIGNAL(tier::anycast, e2::form::prop::colors::bg,   config.bgc);
            if (config.fgc     ) object->SIGNAL(tier::anycast, e2::form::prop::colors::fg,   config.fgc);
            if (config.slimmenu) object->SIGNAL(tier::anycast, e2::form::prop::ui::slimmenu, config.slimmenu);
            return std::pair{ object, config };
        }
        auto _newapp(auto& menuid)
        {
            auto& config = regis.lnk[menuid];
            auto& creator = app::shared::create::builder(config.type);
            auto object = creator(config.cwd, config.param, config.settings, config.patch);
            if (config.bgc     ) object->SIGNAL(tier::anycast, e2::form::prop::colors::bg,   config.bgc);
            if (config.fgc     ) object->SIGNAL(tier::anycast, e2::form::prop::colors::fg,   config.fgc);
            if (config.slimmenu) object->SIGNAL(tier::anycast, e2::form::prop::ui::slimmenu, config.slimmenu);
            return std::pair{ object, config };
        }

    protected:
        hall(xipc server_pipe, xml::settings& config, text defailt_id)
            : host{ server_pipe, config }
        {
            auto current_module_file = os::process::binary();
            auto& menu_list = *regis.app_ptr;
            auto& conf_list = *regis.lnk_ptr;
            auto  free_list = std::list<std::pair<text, menuitem_t>>{};
            auto  temp_list = free_list;
            auto dflt_rec = menuitem_t
            {
                .hidden   = faux,
                .slimmenu = faux,
                .type     = defailt_id,
            };
            auto base_window = [](auto header, auto footer, auto menuid)
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
                        boss.LISTEN(tier::preview, e2::form::proceed::d_n_d::drop, what, -, (menuid))
                        {
                            if (auto object = boss.pop_back())
                            {
                                auto& title = boss.template plugins<pro::title>();
                                what.header = title.header();
                                what.footer = title.footer();
                                what.object = object;
                                what.menuid = menuid;
                            }
                        };
                        boss.LISTEN(tier::release, hids::events::mouse::button::dblclick::left, gear)
                        {
                            boss.RISEUP(tier::release, e2::form::maximize, gear);
                            gear.dismiss();
                        };
                        boss.LISTEN(tier::release, hids::events::mouse::button::click::left, gear)
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
                        boss.LISTEN(tier::release, e2::form::proceed::detach, backup)
                        {
                            boss.mouse.reset();
                            boss.base::detach(); // The object kills itself.
                        };
                        boss.LISTEN(tier::release, e2::form::quit, nested_item)
                        {
                            boss.mouse.reset();
                            if (nested_item) boss.base::detach(); // The object kills itself.
                        };
                        boss.LISTEN(tier::release, e2::dtor, p)
                        {
                            auto start = datetime::now();
                            auto counter = e2::cleanup.param();
                            SIGNAL_GLOBAL(e2::cleanup, counter);
                            auto stop = datetime::now() - start;
                            log("hall: garbage collection",
                            "\n\ttime ", utf::format(stop.count()), "ns",
                            "\n\tobjs ", counter.obj_count,
                            "\n\trefs ", counter.ref_count,
                            "\n\tdels ", counter.del_count);
                        };
                    });
            };
            auto find = [&](auto const& id) -> auto&
            {
                auto test = [&](auto& p) { return p.first == id; };

                auto iter_free = std::find_if(free_list.begin(), free_list.end(), test);
                if (iter_free != free_list.end()) return iter_free->second;

                auto iter_temp = std::find_if(temp_list.begin(), temp_list.end(), test);
                if (iter_temp != temp_list.end()) return iter_temp->second;

                return dflt_rec;
            };

            auto splitter_count = 0;
            for (auto item_ptr : config.list(path_item))
            {
                auto& item = *item_ptr;
                auto conf_rec = menuitem_t{};
                //todo autogen id if absent
                conf_rec.splitter = item.take(attr_splitter, faux);
                conf_rec.id       = item.take(attr_id,       ""s );
                if (conf_rec.splitter)
                {
                    conf_rec.id = "splitter_" + std::to_string(splitter_count++);
                }
                else if (conf_rec.id.empty())
                {
                    log("hall: attribute '", utf::debase(attr_id), "' is missing, skip item");
                    continue;
                }
                auto label        = item.take(attr_label, ""s);
                conf_rec.label    = label.empty() ? conf_rec.id : label;
                conf_rec.alias    = item.take(attr_alias, ""s);
                auto& fallback = conf_rec.alias.empty() ? dflt_rec
                                                        : find(conf_rec.alias);
                conf_rec.hidden   = item.take(attr_hidden,   fallback.hidden  );
                conf_rec.notes    = item.take(attr_notes,    fallback.notes   );
                conf_rec.title    = item.take(attr_title,    fallback.title   );
                conf_rec.footer   = item.take(attr_footer,   fallback.footer  );
                conf_rec.bgc      = item.take(attr_bgc,      fallback.bgc     );
                conf_rec.fgc      = item.take(attr_fgc,      fallback.fgc     );
                conf_rec.winsize  = item.take(attr_winsize,  fallback.winsize );
                conf_rec.wincoor  = item.take(attr_wincoor,  fallback.wincoor );
                conf_rec.slimmenu = item.take(attr_slimmenu, fallback.slimmenu);
                conf_rec.hotkey   = item.take(attr_hotkey,   fallback.hotkey  ); //todo register hotkey
                conf_rec.cwd      = item.take(attr_cwd,      fallback.cwd     );
                conf_rec.param    = item.take(attr_param,    fallback.param   );
                conf_rec.type     = item.take(attr_type,     fallback.type    );
                conf_rec.settings = config;
                auto patches      = item.list("config");
                if (patches.size()) conf_rec.patch = patches.front()->snapshot();
                if (conf_rec.title.empty()) conf_rec.title = conf_rec.id + (conf_rec.param.empty() ? ""s : ": " + conf_rec.param);

                utf::to_low(conf_rec.type);
                utf::change(conf_rec.title,  "$0", current_module_file);
                utf::change(conf_rec.footer, "$0", current_module_file);
                utf::change(conf_rec.label,  "$0", current_module_file);
                utf::change(conf_rec.notes,  "$0", current_module_file);
                utf::change(conf_rec.param,  "$0", current_module_file);

                if (conf_rec.hidden) temp_list.emplace_back(std::move(conf_rec.id), std::move(conf_rec));
                else                 free_list.emplace_back(std::move(conf_rec.id), std::move(conf_rec));
            }
            for (auto& [id, conf_rec] : free_list)
            {
                menu_list[id];
                conf_list.emplace(std::move(id), std::move(conf_rec));
            }
            for (auto& [id, conf_rec] : temp_list)
            {
                conf_list.emplace(std::move(id), std::move(conf_rec));
            }

            LISTEN(tier::general, e2::form::global::lucidity, alpha)
            {
                if (alpha == -1)
                {
                    alpha = skin::shady();
                }
                else
                {
                    alpha = std::clamp(alpha, 0, 255);
                    skin::setup(tone::lucidity, alpha);
                    this->SIGNAL(tier::preview, e2::form::global::lucidity, alpha);
                }
            };
            LISTEN(tier::preview, e2::form::proceed::detach, item_ptr)
            {
                if (regis.usr.size() == 1) // Save all foci for the last user.
                {
                    auto& active = taken[id_t{}];
                    auto proc = e2::form::proceed::functor.param([&](sptr<base> focused_item_ptr)
                    {
                        active.push_back(focused_item_ptr->id);
                    });
                    this->SIGNAL(tier::general, e2::form::proceed::functor, proc);
                }
                auto& inst = *item_ptr;
                host::denote(items.remove(inst.id));
                host::denote(users.remove(inst.id));
                if (regis.remove(item_ptr))
                {
                    inst.SIGNAL(tier::release, e2::form::upon::vtree::detached, This());
                }

                if (items.size()) // Pass focus to the top most object.
                {
                    auto last_ptr = items.back();
                    auto gear_id_list = e2::form::state::keybd::enlist.param();
                    item_ptr->SIGNAL(tier::anycast, e2::form::state::keybd::enlist, gear_id_list);
                    for (auto gear_id : gear_id_list)
                    {
                        if (auto ptr = bell::getref(gear_id))
                        if (auto gear_ptr = std::dynamic_pointer_cast<hids>(ptr))
                        {
                            auto& gear = *gear_ptr;
                            gear.annul_kb_focus(item_ptr);
                            if (gear.kb_focus_empty())
                            {
                                gear.kb_offer_4(last_ptr);
                            }
                        }
                    }
                }
            };
            LISTEN(tier::release, e2::form::layout::bubble, inst)
            {
                auto region = items.bubble(inst.bell::id);
                host::denote(region);
            };
            LISTEN(tier::release, e2::form::layout::expose, inst)
            {
                auto region = items.expose(inst.bell::id);
                host::denote(region);
            };
            LISTEN(tier::request, hall::bindings::list::users, usr_list_ptr)
            {
                usr_list_ptr = regis.usr_ptr;
            };
            LISTEN(tier::request, hall::bindings::list::apps, app_list_ptr)
            {
                app_list_ptr = regis.app_ptr;
            };
            LISTEN(tier::request, hall::bindings::list::links, list_ptr)
            {
                list_ptr = regis.lnk_ptr;
            };
            //todo unify
            LISTEN(tier::request, e2::form::layout::gonext, next)
            {
                if (items)
                if (auto next_ptr = items.rotate_next())
                {
                    next = next_ptr->object;
                }
            };
            LISTEN(tier::request, e2::form::layout::goprev, prev)
            {
                if (items)
                if (auto prev_ptr = items.rotate_prev())
                {
                    prev = prev_ptr->object;
                }
            };
            LISTEN(tier::release, e2::form::proceed::autofocus::take, gear)
            {
                autofocus(gear);
            };
            LISTEN(tier::release, e2::form::proceed::autofocus::lost, gear)
            {
                taken[gear.id] = gear.clear_kb_focus();
            };
            LISTEN(tier::release, e2::form::proceed::createby, gear)
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
                    this->SIGNAL(tier::release, e2::form::proceed::createat, what);
                    if (auto& frame = what.object)
                    {
                        insts_count++;
                        frame->LISTEN(tier::release, e2::form::upon::vtree::detached, master)
                        {
                            insts_count--;
                            log("hall: detached: ", insts_count);
                        };
                        gear.clear_kb_focus(); // DirectVT app could have a group of focused.
                        gear.kb_offer_10(frame);
                        frame->SIGNAL(tier::anycast, e2::form::upon::created, gear); // Tile should change the menu item.
                    }
                }
            };
            LISTEN(tier::release, e2::form::proceed::createat, what)
            {
                auto [object, config] = _newapp(what.menuid);
                auto window = base_window(config.title, config.footer, what.menuid);
                if (config.winsize && !what.forced) window->extend({what.square.coor, config.winsize });
                else                                window->extend(what.square);
                window->attach(object);
                log("hall: app type: ", utf::debase(config.type), ", menu item id: ", utf::debase(what.menuid));
                this->branch(what.menuid, window, !config.hidden);
                window->SIGNAL(tier::anycast, e2::form::upon::started, this->This());
                what.object = window;
            };
            LISTEN(tier::release, e2::form::proceed::createfrom, what)
            {
                auto& config = regis.lnk[what.menuid];
                auto window = base_window(what.header, what.footer, what.menuid);
                window->extend(what.square);
                window->attach(what.object);
                log("hall: attach type=", utf::debase(config.type), " menuid=", utf::debase(what.menuid));
                this->branch(what.menuid, window, !config.hidden);
                window->SIGNAL(tier::anycast, e2::form::upon::started, this->This());
                what.object = window;
            };
        }

    public:
       ~hall()
        {
            auto lock = events::sync{};
            regis.reset();
            items.reset();
        }

        // hall: Autorun apps from config.
        void autorun(xml::settings& config)
        {
            auto what = e2::form::proceed::createat.param();
            auto& active = taken[id_t{}];
            for (auto app_ptr : config.list(path_autorun))
            {
                auto& app = *app_ptr;
                if (!app.fake)
                {
                    what.menuid =   app.take(attr_id, ""s);
                    what.square = { app.take(attr_wincoor, dot_00),
                                    app.take(attr_winsize, twod{ 80,25 }) };
                    auto focused =  app.take(attr_focused, faux);
                    what.forced = !!what.square.size;
                    if (what.menuid.size())
                    {
                        SIGNAL(tier::release, e2::form::proceed::createat, what);
                        if (focused) active.push_back(what.object->id);
                    }
                    else log("hall: Unexpected empty app id in autorun configuration");
                }
            }
        }
        // hall: Restore all foci for the first user.
        void autofocus(hids& gear)
        {
            auto focus = [&](auto& active)
            {
                if (active.size())
                {
                    for (auto id : active)
                    {
                        if (auto window_ptr = bell::getref(id))
                        {
                            gear.kb_offer_4(window_ptr);
                        }
                    }
                    active.clear();
                }
            };
            focus(taken[id_t{}]);
            if (auto iter = taken.find(gear.id); iter != taken.end())
            {
                focus(iter->second);
            }
        }
        void redraw(face& canvas) override
        {
            if (users.size() > 1) users.prerender (canvas); // Draw backpane for spectators.
                                  items.render    (canvas); // Draw objects of the world.
                                  users.postrender(canvas); // Draw spectator's mouse pointers.
        }
        // hall: Attach a new item to the scene.
        template<class S>
        void branch(text const& class_id, sptr<S> item, bool fixed = true)
        {
            items.append(item);
            item->base::root(true); //todo move it to the window creator (main)
            auto& [stat, list] = regis.app[class_id];
            stat = fixed;
            list.push_back(item);
            item->SIGNAL(tier::release, e2::form::upon::vtree::attached, base::This());
            SIGNAL(tier::release, hall::bindings::list::apps, regis.app_ptr);
        }
        // hall: Create a new user of the specified subtype and invite him to the scene.
        template<class S, class ...Args>
        auto invite(Args&&... args)
        {
            auto lock = events::sync{};
            auto user = host::invite<S>(std::forward<Args>(args)...);
            users.append(user);
            regis.usr.push_back(user);
            SIGNAL(tier::release, hall::bindings::list::users, regis.usr_ptr);
            return user;
        }
    };
}