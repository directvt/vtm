// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#include "console.hpp"

#include <vector>
#include <mutex>
#include <map>

namespace netxs::ui
{
    enum axis { X, Y };

    enum class sort
    {
        forward,
        reverse,
    };

    enum class snap
    {
        none,
        head,
        tail,
        stretch,
        center,
    };

    enum class slot { _1, _2, _I };

    enum class axes
    {
        none   = 0,
        X_only = 1 << 0,
        Y_only = 1 << 1,
        all    = (X_only | Y_only),
    };
    constexpr auto operator & (axes l, axes r) { return static_cast<si32>(l) & static_cast<si32>(r); }

    // controls: base UI element.
    template<class T>
    class form
        : public base
    {
        std::map<std::type_index, uptr<pro::skill>> depo;
        std::map<id_t, subs> memomap; // form: Token set for depend submissions.

    public:
        using sptr = netxs::sptr<base>;

        pro::mouse mouse{ *this }; // form: Mouse controller.
        //pro::keybd keybd{ *this }; // form: Keybd controller.

        auto This() { return base::This<T>(); }
        form()
        {
            if constexpr (requires(decltype(e2::form::proceed::detach)::type shadow) { This()->T::remove(shadow); })
            {
                LISTEN(tier::preview, e2::form::proceed::detach, shadow)
                {
                    This()->T::remove(shadow);
                };
            }
        }
        template<class ...Args>
        static auto ctor(Args&&... args)
        {
            auto item = base::create<T>(std::forward<Args>(args)...);
            return item;
        }
        // form: Attach feature and return itself.
        template<class S, class ...Args>
        auto plugin(Args&&... args)
        {
            auto backup = This();
            depo[std::type_index(typeid(S))] = std::make_unique<S>(*this, std::forward<Args>(args)...);
            base::reflow();
            return backup;
        }
        // form: Attach feature and return itself.
        template<class S>
        auto unplug()
        {
            auto backup = This();
            depo.erase(std::type_index(typeid(S)));
            base::reflow();
            return backup;
        }
        // form: Set colors and return itself.
        template<class ...Args>
        auto colors(Args&&... args)
        {
            base::color(std::forward<Args>(args)...);
            return This();
        }
        // form: Set control as root.
        auto isroot(bool state, si32 kind = 0)
        {
            base::root(state);
            base::kind(kind);
            return This();
        }
        // form: Set the form visible for mouse.
        auto active(bool visible = true)
        {
            auto brush = base::color();
            if (!brush.wdt()) base::color(brush.txt(whitespace));
            return This();
        }
        // form: Return plugin reference of specified type. Add the specified plugin (using specified args) if it is missing.
        template<class S, class ...Args>
        auto& plugins(Args&&... args)
        {
            const auto it = depo.find(std::type_index(typeid(S)));
            if (it == depo.end())
            {
                plugin<S>(std::forward<Args>(args)...);
            }
            auto ptr = static_cast<S*>(depo[std::type_index(typeid(S))].get());
            return *ptr;
        }
        // form: Invoke arbitrary functor(itself/*This/boss).
        template<class P>
        auto invoke(P functor)
        {
            auto backup = This();
            functor(*backup);
            return backup;
        }
        // form: Attach homeless branch and return itself.
        template<class ...Args>
        auto branch(Args&&... args)
        {
            auto backup = This();
            backup->T::attach(std::forward<Args>(args)...);
            return backup;
        }
        // form: UI-control will be detached when the master is detached.
        auto depend(sptr master_ptr)
        {
            auto& master = *master_ptr;
            master.LISTEN(tier::release, e2::dtor, id, memomap[master.id])
            {
                auto backup = This();
                memomap.erase(master.id);
                if (memomap.empty()) base::detach();
            };
            return This();
        }
        // form: UI-control will be detached when the last item of collection is detached.
        template<class S>
        auto depend_on_collection(S data_collection_src)
        {
            auto backup = This();
            for (auto& data_src : data_collection_src)
            {
                depend(data_src);
            }
            return backup;
        }
        // form: Create and attach a new item using a template and dynamic datasource.
        template<class Property, class Sptr, class P>
        auto attach_element(Property, Sptr data_src_sptr, P item_template)
        {
            auto arg_value = typename Property::type{};

            auto backup = This();
            data_src_sptr->SIGNAL(tier::request, Property{}, arg_value);
            auto new_item = item_template(data_src_sptr, arg_value)
                                 ->depend(data_src_sptr);
            auto item_shadow = ptr::shadow(new_item);
            auto data_shadow = ptr::shadow(data_src_sptr);
            auto boss_shadow = ptr::shadow(backup);
            data_src_sptr->LISTEN(tier::release, Property{}, arg_new_value, memomap[data_src_sptr->id], (boss_shadow, data_shadow, item_shadow, item_template))
            {
                if (auto boss_ptr = boss_shadow.lock())
                if (auto data_src = data_shadow.lock())
                if (auto old_item = item_shadow.lock())
                {
                    auto new_item = item_template(data_src, arg_new_value)
                                         ->depend(data_src);
                    item_shadow = ptr::shadow(new_item); // Update current item shadow.
                    boss_ptr->update(old_item, new_item);
                }
            };
            branch(new_item);
            return new_item;
        }
        // form: Create and attach a new item using a template and dynamic datasource.
        template<class Property, class S, class P>
        auto attach_collection(Property, S& data_collection_src, P item_template)
        {
            auto backup = This();
            for (auto& data_src_sptr : data_collection_src)
            {
                attach_element(Property{}, data_src_sptr, item_template);
            }
            return backup;
        }
        template<class BackendProp, class P>
        void publish_property(BackendProp, P setter)
        {
            LISTEN(tier::request, BackendProp{}, property_value, -, (setter))
            {
                setter(property_value);
            };
        }
        template<class BackendProp, class FrontendProp>
        auto attach_property(BackendProp, FrontendProp)
        {
            auto property_value = typename BackendProp::type{};

            auto backup = This();
            SIGNAL(tier::request, BackendProp{},  property_value);
            SIGNAL(tier::anycast, FrontendProp{}, property_value);

            LISTEN(tier::release, BackendProp{}, property_value)
            {
                this->SIGNAL(tier::anycast, FrontendProp{}, property_value);
            };
            return backup;
        }
    };

    // controls: Splitter.
    class fork
        : public form<fork>
    {
        enum class action { seize, drag, release };

        sptr client_1; // fork: 1st object.
        sptr client_2; // fork: 2nd object.
        sptr splitter; // fork: Resizing grip.

        twod size1;
        twod size2;
        twod coor2;
        twod size3;
        twod coor3;
        si32 split = 0;

        tint clr;
        //twod size;
        rect stem;
        //si32 start;
        si32 width;
        si32 ratio;
        si32 maxpos;
        bool updown;
        bool movable;
        bool fixed;

        twod xpose(twod const& pt) { return updown ? twod{ pt.y, pt.x } : pt; }
        si32 get_x(twod const& pt) { return updown ? pt.y : pt.x; }
        si32 get_y(twod const& pt) { return updown ? pt.x : pt.y; }

        void _config(axis alignment, si32 thickness, si32 s1 = 1, si32 s2 = 1)
        {
            switch (alignment)
            {
                case axis::X: updown = faux; break;
                case axis::Y: updown = true; break;
                default:      updown = faux; break;
            }
            width = std::max(thickness, 0);
            config(s1, s2);
        }
        void _config_ratio(si32 s1, si32 s2)
        {
            if (s1 < 0) s1 = 0;
            if (s2 < 0) s2 = 0;
            auto sum = s1 + s2;
            ratio = sum ? netxs::divround(s1 * max_ratio, sum)
                        : max_ratio >> 1;
        }

    public:
        static constexpr auto min_ratio = si32{ 0           };
        static constexpr auto max_ratio = si32{ 0xFFFF      };
        static constexpr auto mid_ratio = si32{ 0xFFFF >> 1 };

        auto get_ratio()
        {
            return ratio;
        }
        auto set_ratio(si32 new_ratio = max_ratio)
        {
            ratio = new_ratio;
        }
        void config(si32 s1, si32 s2 = 1)
        {
            _config_ratio(s1, s2);
            base::reflow();
        }
        auto config(axis alignment, si32 thickness, si32 s1, si32 s2)
        {
            _config(alignment, thickness, s1, s2);
            return This();
        }

       ~fork()
        {
            auto empty = e2::form::upon::vtree::detached.param();
            if (client_1)
            {
                auto item_ptr = client_1;
                client_1.reset();
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, empty);
            }
            if (client_2)
            {
                auto item_ptr = client_2;
                client_2.reset();
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, empty);
            }
            if (splitter)
            {
                auto item_ptr = splitter;
                splitter.reset();
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, empty);
            }
        }
        fork(axis alignment = axis::X, si32 thickness = 0, si32 s1 = 1, si32 s2 = 1)
            : maxpos{ 0 },
              //start{ 0 },
              width{ 0 },
              movable{ true },
              updown{ faux },
              ratio{ 0xFFFF >> 1 },
              fixed{ faux }
        {
            LISTEN(tier::release, e2::form::prop::fixedsize, is_fixed) //todo unify -- See terminal window self resize
            {
                fixed = is_fixed;
            };
            LISTEN(tier::preview, e2::size::set, new_size)
            {
                fork::size_preview(new_size);
            };
            LISTEN(tier::release, e2::size::any, new_size)
            {
                //size = new_size;
                if (client_1)
                {
                    client_1->SIGNAL(tier::release, e2::size::set, size1);
                }
                if (client_2)
                {
                    client_2->SIGNAL(tier::release, e2::coor::set, coor2);
                    client_2->SIGNAL(tier::release, e2::size::set, size2);
                }
                if (splitter)
                {
                    splitter->SIGNAL(tier::release, e2::coor::set, coor3);
                    splitter->SIGNAL(tier::release, e2::size::set, size3);
                }
            };
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                auto& basis = base::coor();
                if (splitter) parent_canvas.render(splitter, basis);
                if (client_1) parent_canvas.render(client_1, basis);
                if (client_2) parent_canvas.render(client_2, basis);
            };

            _config(alignment, thickness, s1, s2);
        }
        void rotate()
        {
            updown = !updown;
            //todo revise/unify
                 if (updown  && width == 2) width = 1;
            else if (!updown && width == 1) width = 2;
            base::reflow();
        }
        void swap()
        {
            std::swap(client_1, client_2);
            if (client_1)
            {
                auto coor1 = dot_00;
                client_1->SIGNAL(tier::release, e2::coor::set, coor1);
            }
            base::reflow();
        }
        // fork: .
        void size_preview(twod& new_size)
        {
            //todo revise/unify
            //todo client_2 doesn't respect ui::pads
            auto new_size0 = xpose(new_size);
            {
                maxpos = std::max(new_size0.x - width, 0);
                split = netxs::divround(maxpos * ratio, max_ratio);

                size1 = xpose({ split, new_size0.y });
                if (client_1)
                {
                    auto& item = *client_1;
                    item.SIGNAL(tier::preview, e2::size::set, size1);

                    split = get_x(size1);
                    new_size0.y = get_y(size1);
                }

                size2 = xpose({ maxpos - split, new_size0.y });
                auto test_size2 = size2;
                if (client_2)
                {
                    auto& item = *client_2;
                    item.SIGNAL(tier::preview, e2::size::set, size2);
                    split = new_size0.x - width - get_x(size2);

                    if (test_size2 != size2) // If size2 doesn't fit.
                    {
                        new_size0.y = get_y(size2);
                        size1 = xpose({ split, new_size0.y });
                        if (client_1)
                        {
                            auto& item = *client_1;
                            item.SIGNAL(tier::preview, e2::size::set, size1);

                            split = get_x(size1);
                            new_size0.y = get_y(size1);
                        }
                        size2 = xpose({ maxpos - split, new_size0.y });
                        if (client_2)
                        {
                            auto& item = *client_2;
                            item.SIGNAL(tier::preview, e2::size::set, size2);
                            new_size0.y = get_y(size2);
                        }
                    }

                    coor2 = xpose({ split + width, 0 });
                }
                if (splitter)
                {
                    coor3 = xpose({ split, 0 });
                    size3 = xpose({ width, new_size0.y });
                }

                new_size = xpose({ split + width + get_x(size2), new_size0.y });

                if (fixed) _config_ratio(split, get_x(size2));
            }
        }
        void move_slider(si32 const& step)
        {
            if (splitter)
            {
                auto delta = xpose({ step * width, 0 });
                splitter->SIGNAL(tier::preview, e2::form::upon::changed, delta);
            }
        }
        void slider(action act, twod const& delta)
        {
            if (movable)
            {
                switch (act)
                {
                    case action::seize:
                        //control_w32::mouse(true);
                        //start = get_x(stem.coor) - get_x(delta);
                        break;
                    case action::drag:
                        //if (!control_w32::mouse())
                        //{
                        //	return;
                        //}
                        break;
                    case action::release:
                        //if (!control_w32::mouse())
                        //{
                        //	return;
                        //}
                        //control_w32::mouse(faux);
                        break;
                    default:
                        return;
                }

                //todo move slider

                //fork::deploy(start + xpose(delta).x, true);
                //si32 newpos = start + xpose(delta).x;
                //
                //auto c1 = base::limit(ctrl::_1);
                //auto c2 = base::limit(ctrl::_2);
                //
                //auto minsplit = std::max(get_x(c1.min), maxpos - get_x(c2.max));
                //auto maxsplit = std::min(get_x(c1.max), maxpos - get_x(c2.min));
                //auto split = std::clamp(newpos, minsplit, std::max(minsplit, maxsplit));
                //
                //stem = { xpose({ split, 0 }), xpose({ width, get_y(size) }) };
                //if (client_1)
                //	client_1->extend(
                //		rect{ dot_00,                      xpose({ split,          get_y(size) }) });
                //if (client_2)
                //	client_2->extend(
                //		rect{ xpose({ split + width, 0 }), xpose({ maxpos - split, get_y(size) }) });

                ratio = std::clamp(netxs::divround(get_x(stem.coor) * max_ratio, maxpos), 0, max_ratio);

                base::deface();
            }
        }
        template<class T>
        auto attach(slot Slot, T item_ptr)
        {
            if (Slot == slot::_1)
            {
                if (client_1) remove(client_1);
                client_1 = item_ptr;
            }
            else if (Slot == slot::_2)
            {
                if (client_2) remove(client_2);
                client_2 = item_ptr;
            }
            else
            {
                if (splitter) remove(splitter);
                splitter = item_ptr;
                splitter->LISTEN(tier::preview, e2::form::upon::changed, delta)
                {
                    split += get_x(delta);
                    ratio = netxs::divround(max_ratio * split, maxpos);
                    this->base::reflow();
                };
            }

            item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::attached, This());
            return item_ptr;
        }
        // fork: Remove nested object by it's ptr.
        void remove(sptr item_ptr)
        {
            if (client_1 == item_ptr ? ((void)client_1.reset(), true) :
                client_2 == item_ptr ? ((void)client_2.reset(), true) :
                splitter == item_ptr ? ((void)splitter.reset(), true) : faux)
            {
                auto backup = This();
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, backup);
            }
        }
    };

    // controls: Vertical/horizontal list.
    class list
        : public form<list>
    {
        using book = std::list<std::pair<sptr, twod>>;

        book subset;
        bool updown; // list: List orientation, true: vertical(default), faux: horizontal.
        sort lineup; // list: Attachment order.

    public:
        void clear()
        {
            auto backup = This();
            while (subset.size())
            {
                auto item_ptr = subset.back().first;
                subset.pop_back();
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, backup);
            }
        }
       ~list()
        {
            auto empty = e2::form::upon::vtree::detached.param();
            while (subset.size())
            {
                auto item_ptr = subset.back().first;
                subset.pop_back();
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, empty);
            }
        }
        list(axis orientation = axis::Y, sort attach_order = sort::forward)
            : updown{ orientation == axis::Y },
              lineup{ attach_order }
        {
            LISTEN(tier::preview, e2::size::set, new_sz)
            {
                auto  height = si32{};
                auto& y_size = updown ? new_sz.y : new_sz.x;
                auto& x_size = updown ? new_sz.x : new_sz.y;
                auto  x_temp = x_size;
                auto  y_temp = y_size;

                auto meter = [&]
                {
                    height = 0;
                    for (auto& client : subset)
                    {
                        y_size = 0;
                        client.first->SIGNAL(tier::preview, e2::size::set, new_sz);
                        client.second = { x_size, y_size };
                        if (x_size > x_temp) x_temp = x_size;
                        x_size = x_temp;
                        height += y_size;
                    }
                };
                meter(); if (x_temp != x_size) meter();
                y_size = height;
            };
            LISTEN(tier::release, e2::size::any, new_sz)
            {
                auto& y_size = updown ? new_sz.y : new_sz.x;
                auto& x_size = updown ? new_sz.x : new_sz.y;
                auto  new_xy = twod{};
                auto& y_coor = updown ? new_xy.y : new_xy.x;
                auto& x_coor = updown ? new_xy.x : new_xy.y;

                auto found = faux;
                for (auto& client : subset)
                {
                    y_size = client.second.y;
                    if (client.first)
                    {
                        auto& entry = *client.first;
                        if (!found)
                        {
                            //todo optimize: use the only one axis to hittest
                            //todo detect client during preview, use wptr
                            auto& anker = entry.base::area();
                            if (anker.hittest(base::anchor))
                            {
                                found = true;
                                base::anchor += new_xy - anker.coor;
                            }
                        }
                        entry.SIGNAL(tier::release, e2::coor::set, new_xy);
                        auto& sz_y = updown ? client.second.y : client.second.x;
                        auto& sz_x = updown ? client.second.x : client.second.y;
                        auto& size = entry.resize(sz_x, sz_y);
                        sz_x = size.x;
                        sz_y = size.y;
                        y_coor += client.second.y;
                    }
                }
            };
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                auto& basis = base::coor();
                for (auto& client : subset)
                {
                    parent_canvas.render(client.first, basis);
                }
            };
        }
        // list: Remove the last nested object. Return the object refrence.
        auto pop_back()
        {
            if (subset.size())
            {
                auto iter = std::prev(subset.end());
                auto item_ptr = iter->first;
                auto backup = This();
                subset.erase(iter);
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, backup);
                return item_ptr;
            }
            return sptr{};
        }
        // list: Attach specified item.
        template<class T>
        auto attach(T item_ptr)
        {
            if (lineup == sort::forward) subset.push_back ({ item_ptr, dot_00 });
            else                         subset.push_front({ item_ptr, dot_00 });
            item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::attached, This());
            return item_ptr;
        }
        // list: Remove nested object.
        void remove(sptr item_ptr)
        {
            auto head = subset.begin();
            auto tail = subset.end();
            auto iter = std::find_if(head, tail, [&](auto& c){ return c.first == item_ptr; });
            if (iter != tail)
            {
                auto backup = This();
                subset.erase(iter);
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, backup);
            }
        }
        // list: Update nested object.
        void update(sptr old_item_ptr, sptr new_item_ptr)
        {
            auto head = subset.begin();
            auto tail = subset.end();
            auto iter = std::find_if(head, tail, [&](auto& c){ return c.first == old_item_ptr; });
            if (iter != tail)
            {
                auto backup = This();
                auto pos = subset.erase(iter);
                old_item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, backup);
                subset.insert(pos, std::pair{ new_item_ptr, dot_00 });
                new_item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::attached, backup);
            }
        }
    };

    // controls: (puff) Layered cake of forms on top of each other.
    class cake
        : public form<cake>
    {
        std::list<sptr> subset;

    public:
       ~cake()
        {
            auto empty = e2::form::upon::vtree::detached.param();
            while (subset.size())
            {
                auto item_ptr = subset.back();
                subset.pop_back();
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, empty);
            }
        }
        cake()
        {
            LISTEN(tier::preview, e2::size::set, newsz)
            {
                for (auto& client : subset)
                {
                    if (client)
                    {
                        client->SIGNAL(tier::preview, e2::size::set, newsz);
                    }
                }
            };
            LISTEN(tier::release, e2::size::any, newsz)
            {
                for (auto& client : subset)
                {
                    if (client)
                    {
                        client->SIGNAL(tier::release, e2::size::set, newsz);
                    }
                }
            };
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                auto& basis = base::coor();
                for (auto& client : subset)
                {
                    parent_canvas.render(client, basis);
                }
            };
        }
        // cake: Remove the last nested object. Return the object refrence.
        auto pop_back()
        {
            if (subset.size())
            {
                auto iter = std::prev(subset.end());
                auto item_ptr = *iter;
                auto backup = This();
                subset.erase(iter);
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, backup);
                return item_ptr;
            }
            return sptr{};
        }
        // cake: Create a new item of the specified subtype and attach it.
        template<class T>
        auto attach(T item_ptr)
        {
            subset.push_back(item_ptr);
            item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::attached, This());
            return item_ptr;
        }
        // cake: Remove nested object.
        void remove(sptr item_ptr)
        {
            auto head = subset.begin();
            auto tail = subset.end();
            auto iter = std::find_if(head, tail, [&](auto& c){ return c == item_ptr; });
            if (iter != tail)
            {
                auto backup = This();
                subset.erase(iter);
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, backup);
            }
        }
    };

    // controls: Form aligner.
    class park
        : public form<park>
    {
        struct type
        {
            sptr ptr;
            snap  hz;
            snap  vt;
            bool  on;
        };
        std::list<type> subset;

        void xform(snap align, si32& coor, si32& size, si32 width)
        {
            switch (align)
            {
                case snap::head:
                    coor = 0;
                    break;
                case snap::tail:
                    coor = width - size;
                    break;
                case snap::center:
                    coor = (width - size) / 2;
                    break;
                case snap::stretch:
                    coor = 0;
                    size = width;
                    break;
                default:
                    break;
            }
        }

    public:
       ~park()
        {
            auto empty = e2::form::upon::vtree::detached.param();
            while (subset.size())
            {
                auto item_ptr = subset.back().ptr;
                subset.pop_back();
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, empty);
            }
        }
        park()
        {
            LISTEN(tier::release, e2::size::any, newsz)
            {
                for (auto& client : subset)
                {
                    if (client.ptr)
                    {
                        auto& item = *client.ptr;
                        auto  area = item.area();
                        xform(client.hz, area.coor.x, area.size.x, newsz.x);
                        xform(client.vt, area.coor.y, area.size.y, newsz.y);
                        item.extend(area);
                    }
                }
            };
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                auto& basis = base::coor();
                for (auto& client : subset)
                {
                    if (client.on) parent_canvas.render(client.ptr, basis);
                }
            };
        }
        // park: Remove the last nested object. Return the object refrence.
        auto pop_back()
        {
            if (subset.size())
            {
                auto iter = std::prev(subset.end());
                auto item_ptr = iter->ptr;
                auto backup = This();
                subset.erase(iter);
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, backup);
                return item_ptr;
            }
            return sptr{};
        }
        // park: Configure specified object.
        void config(sptr item_ptr, snap new_hz, snap new_vt)
        {
            if (!item_ptr) return;
            auto head = subset.begin();
            auto tail = subset.end();
            auto iter = std::find_if(head, tail, [&](auto& c){ return c.ptr == item_ptr; });
            if (iter != tail)
            {
                iter->hz = new_hz;
                iter->vt = new_vt;
            }
        }
        // park: Make specified object visible or not.
        void visible(sptr item_ptr, bool is_visible)
        {
            if (!item_ptr) return;
            auto head = subset.begin();
            auto tail = subset.end();
            auto iter = std::find_if(head, tail, [&](auto& c){ return c.ptr == item_ptr; });
            if (iter != tail) iter->on = is_visible;
        }
        // park: Create a new item of the specified subtype and attach it.
        template<class T>
        auto attach(T item_ptr, snap hz, snap vt)
        {
            subset.push_back({ item_ptr, hz, vt, true });
            item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::attached, This());
            return item_ptr;
        }
        // park: Remove nested object.
        void remove(sptr item_ptr)
        {
            auto head = subset.begin();
            auto tail = subset.end();
            auto iter = std::find_if(head, tail, [&](auto& c){ return c.ptr == item_ptr; });
            if (iter != tail)
            {
                auto backup = This();
                subset.erase(iter);
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, backup);
            }
        }
    };

    // controls: Container for many controls but shows the last one.
    class veer
        : public form<veer>
    {
        std::list<sptr> subset;

    public:
       ~veer()
        {
            auto empty = e2::form::upon::vtree::detached.param();
            while (subset.size())
            {
                auto item_ptr = subset.back();
                subset.pop_back();
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, empty);
            }
        }
        veer()
        {
            LISTEN(tier::preview, e2::size::set, newsz)
            {
                if (subset.size())
                if (auto active = subset.back())
                {
                    active->SIGNAL(tier::preview, e2::size::set, newsz);
                }
            };
            LISTEN(tier::release, e2::size::any, newsz)
            {
                if (subset.size())
                if (auto active = subset.back())
                {
                    active->SIGNAL(tier::release, e2::size::set, newsz);
                }
            };
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                if (subset.size())
                if (auto active = subset.back())
                {
                    auto& basis = base::coor();
                    parent_canvas.render(active, basis);
                }
            };
        }
        // veer: Return the last object or empty sptr.
        auto back()
        {
            return subset.size() ? subset.back()
                                 : sptr{};
        }
        // veer: Return the first object or empty sptr.
        auto front()
        {
            return subset.size() ? subset.front()
                                 : sptr{};
        }
        // veer: Return nested objects count.
        auto count()
        {
            return subset.size();
        }
        // veer: Return true if empty.
        auto empty()
        {
            return subset.empty();
        }
        // veer: Remove the last object. Return the object refrence.
        auto pop_back()
        {
            if (subset.size())
            {
                auto iter = std::prev(subset.end());
                auto item_ptr = *iter;
                auto backup = This();
                subset.erase(iter);
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, backup);
                return item_ptr;
            }
            return sptr{};
        }
        // veer: Roll objects.
        void roll(si32 dt = 1)
        {
            if (dt && subset.size() > 1)
            {
                if (dt > 0) while (dt--)
                            {
                                subset.push_front(subset.back());
                                subset.pop_back();
                            }
                else        while (dt++)
                            {
                                subset.push_back(subset.front());
                                subset.pop_front();
                            }
            }
        }
        // veer: Create a new item of the specified subtype and attach it.
        template<class T>
        auto attach(T item_ptr)
        {
            subset.push_back(item_ptr);
            item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::attached, This());
            return item_ptr;
        }
        // veer: Remove nested object.
        void remove(sptr item_ptr)
        {
            auto head = subset.begin();
            auto tail = subset.end();
            auto iter = std::find_if(head, tail, [&](auto& c){ return c == item_ptr; });
            if (iter != tail)
            {
                auto backup = This();
                subset.erase(iter);
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, backup);
            }
        }
    };

    struct page_layout
    {
        struct item
        {
            ui32 id;
            twod coor;
        };

        size_t len = 0;

        static const auto init_layout_len = si32{ 10000 };
        std::vector<item> layout;
        page_layout()
        {
            layout.reserve(init_layout_len);
        }

        auto get_entry(twod const& anchor)
        {
            auto& anker = anchor.y;
            auto pred = item{ 0, twod{ 0, si32max } };
            auto minp = item{ 0, twod{ 0, si32max } };
            auto mindist = si32max;

            //todo optimize, use binary search
            //start from the end
            for (auto& p : layout)
            {
                auto& post = p.coor.y;
                if (pred.coor.y <= anker && post > anker) // inside the entry
                {
                    return pred;
                }
                else
                {
                    auto dist = std::abs(anker - post);
                    if (dist < mindist)
                    {
                        minp = p;
                        mindist = dist;
                    }
                }
                pred = p;
            }
            return minp;
        }
        auto begin() { return layout.begin(); }
        auto capacity() { return layout.capacity(); }
        void reserve(size_t newsize) { layout.reserve(newsize); }
        void push_back(item const& p) { layout.push_back(p); }
        void clear() { layout.clear(); }
    };

    // controls: Static text page.
    template<auto fx>
    class postfx
        : public flow, public form<postfx<fx>>
    {
        twod width; // post: Page dimensions.
        text source; // post: Raw content.
        page_layout layout; // post: .
        bool beyond; // post: Allow vertical scrolling beyond last line.

    public:
        using post = postfx;

        page topic; // post: Text content.

        auto& lyric(si32 paraid) { return *topic[paraid].lyric; }
        auto& content(si32 paraid) { return topic[paraid]; }
        auto upload(view utf8, si32 initial_width = 0) // Don't use cell link id here. Apply it to the parent (with a whole rect coverage).
        {
            source = utf8;
            topic = utf8;
            base::resize(twod{ initial_width, 0 });
            base::reflow();
            return this->This();
        }
        auto& get_source() const
        {
            return source;
        }
        void output(face& canvas)
        {
            flow::reset(canvas);
            auto publish = [&](auto const& combo)
            {
                flow::print(combo, canvas, fx);
            };
            topic.stream(publish);
        }
        auto get_size() const
        {
            return width;
        }
        void recalc()
        {
            auto s = (size_t)topic.size();
            if (s > layout.capacity())
            {
                layout.reserve(s * 2);
            }

            auto entry = layout.get_entry(base::anchor); // Take the object under central point
            layout.clear();

            flow::reset();
            auto publish = [&](auto const& combo)
            {
                auto cp = flow::print(combo);

                auto id = combo.id();
                if (id == entry.id) entry.coor.y -= cp.y;
                layout.push_back({ id,cp });
            };
            topic.stream(publish);

            // Apply only vertical anchoring for this type of control.
            base::anchor.y -= entry.coor.y; // Move the central point accordingly to the anchored object

            auto& cover = flow::minmax();
            base::oversz.set(-std::min(0, cover.l),
                              std::max(0, cover.r - width.x + 1),
                             -std::min(0, cover.t),
                              0);
            auto height = cover.width() ? cover.height() + 1
                                        : 0;
            width.y = height + (beyond ? width.y : 0); //todo unify (text editor)
        }
        void recalc(twod const& size)
        {
            width = size;
            recalc();
        }

        postfx(bool scroll_beyond = faux)
            : flow{ width },
              beyond{ scroll_beyond }
        {
            LISTEN(tier::preview, e2::size::set, size)
            {
                recalc(size);
                size.y = width.y;
            };
            LISTEN(tier::release, e2::size::any, size)
            {
                //if (width != size)
                //{
                //	recalc(size);
                //	//width.y = size.y;
                //}
                width = size;
            };
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                output(parent_canvas);

                //auto mark = rect{ base::anchor + base::coor(), {10,5} };
                //mark.coor += parent_canvas.view().coor; // Set client's basis
                //parent_canvas.fill(mark, [](cell& c) { c.alpha(0x80).bgc().chan.r = 0xff; });
            };
        }
    };

    using post = postfx<noop{}>;

    // controls: Scroller.
    class rail
        : public form<rail>
    {
        pro::robot robot{*this }; // rail: Animation controller.

        using upon = e2::form::upon;

        twod strict; // rail: Don't allow overscroll.
        twod manual; // rail: Manual scrolling (no auto align).
        twod permit; // rail: Allowed axes to scroll.
        twod siezed; // rail: Allowed axes to capture.
        twod oversc; // rail: Allow overscroll with auto correct.
        subs tokens; // rail: Subscriptions on client moveto and resize.
        subs fasten; // rail: Subscriptions on masters to follow they state.
        rack scinfo; // rail: Scroll info.
        sptr client; // rail: Client instance.

        si32 spd       = skin::globals().spd;
        si32 pls       = skin::globals().pls;
        si32 ccl       = skin::globals().ccl;
        si32 spd_accel = skin::globals().spd_accel;
        si32 ccl_accel = skin::globals().ccl_accel;
        si32 spd_max   = skin::globals().spd_max;
        si32 ccl_max   = skin::globals().ccl_max;
        si32 switching = skin::globals().switching;

        si32 speed{ spd  }; // rail: Text auto-scroll initial speed component ΔR.
        si32 pulse{ pls  }; // rail: Text auto-scroll initial speed component ΔT.
        si32 cycle{ ccl  }; // rail: Text auto-scroll duration in ms.
        bool steer{ faux }; // rail: Text scroll vertical direction.

        static constexpr auto xy(axes Axes)
        {
            return twod{ !!(Axes & axes::X_only), !!(Axes & axes::Y_only) };
        }

    public:
        template<axis Axis>
        auto follow(sptr master = {})
        {
            if (master)
            {
                master->LISTEN(tier::release, upon::scroll::bycoor::any, master_scinfo, fasten)
                {
                    this->SIGNAL(tier::preview, e2::form::upon::scroll::bycoor::_<Axis>, master_scinfo);
                };
            }
            else fasten.clear();

            return This();
        }
       ~rail()
        {
            if (client)
            {
                auto empty = e2::form::upon::vtree::detached.param();
                auto item_ptr = client;
                client.reset();
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, empty);
            }
        }
        rail(axes allow_to_scroll = axes::all, axes allow_to_capture = axes::all, axes allow_overscroll = axes::all)
            : permit{ xy(allow_to_scroll)  },
              siezed{ xy(allow_to_capture) },
              oversc{ xy(allow_overscroll) },
              strict{ xy(axes::all) },
              manual{ xy(axes::all) }
        {
            LISTEN(tier::preview, e2::form::upon::scroll::any, info) // Receive scroll parameters from external sources.
            {
                if (client)
                {
                    switch (this->bell::protos<tier::preview>())
                    {
                        case upon::scroll::bycoor::x.id: move<X>(scinfo.window.coor.x - info.window.coor.x); break;
                        case upon::scroll::bycoor::y.id: move<Y>(scinfo.window.coor.y - info.window.coor.y); break;
                        case upon::scroll::to_top::x.id: move<X>(dot_mx.x); break;
                        case upon::scroll::to_top::y.id: move<Y>(dot_mx.y); break;
                        case upon::scroll::to_end::x.id: move<X>(-dot_mx.x); break;
                        case upon::scroll::to_end::y.id: move<Y>(-dot_mx.y); break;
                        case upon::scroll::bystep::x.id: move<X>(info.vector); break;
                        case upon::scroll::bystep::y.id: move<Y>(info.vector); break;
                        case upon::scroll::bypage::x.id: move<X>(info.vector * scinfo.window.size.x); break;
                        case upon::scroll::bypage::y.id: move<Y>(info.vector * scinfo.window.size.y); break;
                        case upon::scroll::cancel::x.id: cancel<X, true>(); break;
                        case upon::scroll::cancel::y.id: cancel<Y, true>(); break;
                    }
                }
            };
            LISTEN(tier::request, e2::form::upon::scroll::any, req_scinfo)
            {
                req_scinfo = scinfo;
            };

            LISTEN(tier::release, e2::size::any, new_size)
            {
                if (client)
                {
                    client->base::resize(new_size, base::anchor);
                }
            };

            using button = hids::events::mouse::button;
            LISTEN(tier::release, hids::events::mouse::scroll::any, gear)
            {
                if (gear.meta(hids::anyCtrl)) return; // Ctrl+Wheel is reserved for zooming.
                auto dt = gear.whldt > 0;
                auto hz = permit == xy(axes::X_only)
                      || (permit == xy(axes::all) && gear.meta(hids::anyAlt | hids::anyShift));
                if (hz) wheels<X>(dt);
                else    wheels<Y>(dt);
                gear.dismiss();
            };
            LISTEN(tier::release, button::drag::start::right, gear)
            {
                auto ds = gear.delta.get();
                auto dx = ds.x;
                auto dy = ds.y * 2;
                auto vt = std::abs(dx) < std::abs(dy);

                if ((siezed[X] && !vt)
                 || (siezed[Y] &&  vt))
                {
                    if (gear.capture(bell::id))
                    {
                        manual = xy(axes::all);
                        strict = xy(axes::all) - oversc; // !oversc = dot_11 - oversc
                        gear.dismiss();
                    }
                }
            };
            LISTEN(tier::release, button::drag::pull::right, gear)
            {
                if (gear.captured(bell::id))
                {
                    auto delta = gear.mouse::delta.get();
                    auto value = permit * delta;
                    if (value) movexy(value);
                    gear.dismiss();
                }
            };
            LISTEN(tier::release, button::drag::cancel::right, gear)
            {
                if (gear.captured(bell::id))
                {
                    giveup(gear);
                }
            };
            LISTEN(tier::general, hids::events::halt, gear)
            {
                if (gear.captured(bell::id))
                {
                    giveup(gear);
                }
            };
            LISTEN(tier::release, button::drag::stop::right, gear)
            {
                if (gear.captured(bell::id))
                {
                    auto  v0 = gear.delta.avg();
                    auto& speed = v0.dS;
                    auto  cycle = datetime::round<si32>(v0.dT);
                    auto  limit = datetime::round<si32>(skin::globals().deceleration);
                    auto  start = 0;

                    if (permit[X]) actify<X>(quadratic{ speed.x, cycle, limit, start });
                    if (permit[Y]) actify<Y>(quadratic{ speed.y, cycle, limit, start });
                    //todo if (permit == xy(axes::all)) actify(quadratic{ speed, cycle, limit, start });

                    base::deface();
                    gear.setfree();
                    gear.dismiss();
                }
            };
            LISTEN(tier::release, button::click::right, gear)
            {
                if (!gear.captured(bell::id))
                {
                    if (manual[X]) cancel<X, true>();
                    if (manual[Y]) cancel<Y, true>();
                }
            };
            LISTEN(tier::release, button::down::any, gear)
            {
                cutoff();
            };
            LISTEN(tier::release, e2::form::animate::reset, id)
            {
                cutoff();
            };
            LISTEN(tier::release, e2::form::animate::stop, id)
            {
                switch (id)
                {
                    case Y: manual[Y] = true; /*scroll<Y>();*/ break;
                    case X: manual[X] = true; /*scroll<X>();*/ break;
                    default: break;
                }
                base::deface();
            };
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                if (client)
                {
                    parent_canvas.render<faux>(client, base::coor());
                }
            };
        }
        void cutoff()
        {
            if (manual[X]) robot.pacify(X);
            if (manual[Y]) robot.pacify(Y);
        }
        void giveup(hids& gear)
        {
            cancel<X>();
            cancel<Y>();
            base::deface();
            gear.setfree();
            gear.dismiss();
        }
        template<axis Axis>
        void wheels(bool dir)
        {
            if (robot.active(Axis) && (steer == dir))
            {
                speed += spd_accel;
                cycle += ccl_accel;
                speed = std::min(speed, spd_max);
                cycle = std::min(cycle, ccl_max);
            }
            else
            {
                steer = dir;
                speed = spd;
                cycle = ccl;
                //todo at least one line should be
                //move<Axis>(dir ? 1 : -1);
            }
            auto start = 0;
            auto boost = dir ? speed : -speed;
            if constexpr (Axis == X) boost *= 2;
            keepon<Axis>(quadratic<si32>(boost, pulse, cycle, start));
        }
        template<axis Axis, class Fx>
        void keepon(Fx&& func)
        {
            strict[Axis] = true;
            robot.actify(Axis, std::forward<Fx>(func), [&](auto& p)
                {
                    move<Axis>(p);
                });
        }
        template<axis Axis>
        auto inside()
        {
            if (client && manual[Axis]) // Check overscroll if no auto correction.
            {
                auto& item = *client;
                auto frame = base::size()[Axis];
                auto coord = item.base::coor()[Axis] - item.oversz.topleft()[Axis]; // coor - scroll origin basis.
                auto block = item.base::size()[Axis] + item.oversz.summ()[Axis];
                auto bound = std::min(frame - block, 0);
                auto clamp = std::clamp(coord, bound, 0);
                return clamp == coord;
            }
            return true;
        }
        template<axis Axis, class Fx>
        void actify(Fx&& func)
        {
            if (inside<Axis>()) keepon<Axis>(std::forward<Fx>(func));
            else                lineup<Axis>();
        }
        template<axis Axis, bool Forced = faux>
        void cancel()
        {
            if (Forced || !inside<Axis>()) lineup<Axis>();
        }
        template<axis Axis>
        void lineup()
        {
            if (client)
            {
                manual[Axis] = faux;
                auto block = client->base::area();
                auto coord = block.coor[Axis];
                auto bound = std::min(base::size()[Axis] - block.size[Axis], 0);
                auto newxy = std::clamp(coord, bound, 0);
                auto route = newxy - coord;
                auto tempo = switching;
                auto start = 0;
                auto fader = constlinearAtoB<si32>(route, tempo, start);
                keepon<Axis>(fader);
            }
        }
        template<bool Preview>
        auto scroll(twod& coord)
        {
            auto delta = dot_00;
            if (client)
            {
                auto& item = *client;
                auto frame = base::size();
                auto block = item.base::size() + item.oversz.summ();
                auto basis = item.oversz.topleft();
                coord -= basis; // Scroll origin basis.
                if constexpr (Preview)
                {
                    auto bound = std::min(frame - block, dot_00);
                    auto clamp = std::clamp(coord, bound, dot_00);
                    for (auto xy : { axis::X, axis::Y }) // Check overscroll if no auto correction.
                    {
                        if (coord[xy] != clamp[xy] && manual[xy] && strict[xy]) // Clamp if it is outside the scroll limits and no overscroll.
                        {
                            delta[xy] = clamp[xy] - coord[xy];
                            coord[xy] = clamp[xy];
                        }
                    }
                }
                else
                {
                    scinfo.beyond = item.oversz;
                    scinfo.region = block;
                    scinfo.window.coor =-coord; // Viewport.
                    scinfo.window.size = frame; //
                    SIGNAL(tier::release, upon::scroll::bycoor::any, scinfo);
                    base::deface(); // Main menu redraw trigger.
                }
                coord += basis; // Client origin basis.
            }
            return delta;
        }
        void movexy(twod const& delta)
        {
            if (client)
            {
                client->base::moveby(delta);
            }
        }
        template<axis Axis>
        void move(si32 p)
        {
            if (p)
            {
                if constexpr (Axis == X) movexy({ p, 0 });
                if constexpr (Axis == Y) movexy({ 0, p });
            }
        }
        // rail: Attach specified item.
        template<class T>
        auto attach(T item_ptr)
        {
            if (client) remove(client);
            client = item_ptr;
            tokens.clear();
            item_ptr->LISTEN(tier::preview, e2::coor::any, coor, tokens) // any - To check coor first of all.
            {
                scroll<true>(coor);
            };
            item_ptr->LISTEN(tier::release, e2::coor::any, coor, tokens)
            {
                scroll<faux>(coor);
            };
            item_ptr->LISTEN(tier::release, e2::size::any, size, tokens)
            {
                if (client)
                {
                    auto coor = client->base::coor();
                    auto delta = scroll<true>(coor);
                                 scroll<faux>(coor);
                    if (delta)
                    {
                        //todo sync
                    }
                }
            };
            item_ptr->LISTEN(tier::release, e2::form::upon::vtree::detached, p, tokens)
            {
                scinfo.region = {};
                scinfo.window.coor = {};
                this->SIGNAL(tier::release, upon::scroll::bycoor::any, scinfo); // Reset dependent scrollbars.
                fasten.clear();
                tokens.clear();
            };
            item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::attached, This());
            return item_ptr;
        }
        // rail: Detach specified item.
        void remove(sptr item_ptr)
        {
            if (client == item_ptr)
            {
                auto backup = This();
                client.reset();
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, backup);
            }
        }
        // rail: Update nested object.
        void update(sptr old_item_ptr, sptr new_item_ptr)
        {
            if constexpr (debugmode)
            {
                if (client != old_item_ptr) log(prompt::rail, "Wrong DOM structure. rail.id=", id);
            }
            if (client)
            {
                auto current_position = client->base::coor();
                attach(new_item_ptr);
                if (new_item_ptr) new_item_ptr->base::moveto(current_position);
            }
            else attach(new_item_ptr);
        }
    };

    // controls: Scrollbar.
    template<axis Axis, auto drawfx>
    class gripfx
        : public flow, public form<gripfx<Axis, drawfx>>
    {
        pro::timer timer{*this }; // grip: Minimize by timeout.
        pro::limit limit{*this }; // grip: Size limits.

        using sptr = netxs::sptr<base>;
        using wptr = netxs::wptr<base>;
        using form = ui::form<gripfx<Axis, drawfx>>;
        using upon = e2::form::upon;

        enum activity
        {
            mouse_leave = 0, // faux
            mouse_hover = 1, // true
            pager_first = 10,
            pager_next  = 11,
        };

        struct math
        {
            rack  master_inf = {};                           // math: Master scroll info.
            si32& master_len = master_inf.region     [Axis]; // math: Master len.
            si32& master_pos = master_inf.window.coor[Axis]; // math: Master viewport pos.
            si32& master_box = master_inf.window.size[Axis]; // math: Master viewport len.
            si32& master_dir = master_inf.vector;            // math: Master scroll direction.
            si32  scroll_len = 0; // math: Scrollbar len.
            si32  scroll_pos = 0; // math: Scrollbar grip pos.
            si32  scroll_box = 0; // math: Scrollbar grip len.
            si32   m         = 0; // math: Master max pos.
            si32   s         = 0; // math: Scroll max pos.
            double r         = 1; // math: Scroll/master len ratio.

            si32  cursor_pos = 0; // math: Mouse cursor position.

            // math: Calc scroll to master metrics.
            void s_to_m()
            {
                auto scroll_center = scroll_pos + scroll_box / 2.0;
                auto master_center = scroll_len ? scroll_center / r
                                                : 0;
                master_pos = (si32)std::round(master_center - master_box / 2.0);

                // Reset to extreme positions.
                if (scroll_pos == 0 && master_pos > 0) master_pos = 0;
                if (scroll_pos == s && master_pos < m) master_pos = m;
            }
            // math: Calc master to scroll metrics.
            void m_to_s()
            {
                r = (double)scroll_len / master_len;
                auto master_middle = master_pos + master_box / 2.0;
                auto scroll_middle = master_middle * r;
                scroll_box = std::max(1, (si32)(master_box * r));
                scroll_pos = (si32)std::round(scroll_middle - scroll_box / 2.0);

                // Don't place the grip behind the scrollbar.
                if (scroll_pos >= scroll_len) scroll_pos = scroll_len - 1;

                // Extreme positions are always closed last.
                s = scroll_len - scroll_box;
                m = master_len - master_box;

                if (scroll_len > 2) // Two-row hight is not suitable for this type of aligning.
                {
                    if (scroll_pos == 0 && master_pos > 0) scroll_pos = 1;
                    if (scroll_pos == s && master_pos < m) scroll_pos = s - 1;
                }
            }
            void update(rack const& scinfo)
            {
                master_inf = scinfo;
                m_to_s();
            }
            void resize(twod const& new_size)
            {
                scroll_len = new_size[Axis];
                m_to_s();
            }
            void stepby(si32 delta)
            {
                scroll_pos = std::clamp(scroll_pos + delta, 0, s);
                s_to_m();
            }
            void commit(rect& handle)
            {
                handle.coor[Axis]+= scroll_pos;
                handle.size[Axis] = scroll_box;
            }
            auto inside(si32 coor)
            {
                if (coor >= scroll_pos + scroll_box) return 1; // Below the grip.
                if (coor >= scroll_pos)              return 0; // Inside the grip.
                                                     return-1; // Above the grip.
            }
            auto follow()
            {
                auto dir = scroll_len > 2 ? inside(cursor_pos)
                                          : cursor_pos > 0 ? 1 // Don't stop to follow over
                                                           :-1;//    box on small scrollbar.
                return dir;
            }
            void setdir(si32 dir)
            {
                master_dir = -dir;
            }
        };

        wptr boss; // grip: .
        hook memo; // grip: .
        si32 thin; // grip: Scrollbar thickness.
        si32 init; // grip: Handle base width.
        math calc; // grip: Scrollbar calculator.
        bool wide; // grip: Is the scrollbar active.
        si32 mult; // grip: Vertical bar width multiplier.
        bool on_pager = faux; // grip: .

        template<class Event>
        void send()
        {
            if (auto master = this->boss.lock())
            {
                master->SIGNAL(tier::preview, Event::template _<Axis>, calc.master_inf);
            }
        }
        void config(si32 width)
        {
            thin = width;
            auto lims = Axis == axis::X ? twod{ -1,width }
                                        : twod{ width,-1 };
            limit.set(lims, lims);
        }
        void giveup(hids& gear)
        {
            if (on_pager)
            {
                gear.dismiss();
            }
            else
            {
                if (gear.captured(bell::id))
                {
                    if (this->form::template protos<tier::release>(hids::events::mouse::button::drag::cancel::right))
                    {
                        send<upon::scroll::cancel>();
                    }
                    base::deface();
                    gear.setfree();
                    gear.dismiss();
                }
            }
        }
        void pager(si32 dir)
        {
            calc.setdir(dir);
            send<upon::scroll::bypage>();
        }
        auto pager_repeat()
        {
            if (on_pager)
            {
                auto dir = calc.follow();
                pager(dir);
            }
            return on_pager;
        }

    public:
        gripfx(sptr boss, si32 thickness = 1, si32 multiplier = 2)
            : boss{ boss       },
              thin{ thickness  },
              wide{ faux       },
              init{ thickness  },
              mult{ multiplier }
        {
            config(thin);

            boss->LISTEN(tier::release, upon::scroll::bycoor::any, scinfo, memo)
            {
                calc.update(scinfo);
                base::deface();
            };

            LISTEN(tier::release, e2::size::any, new_size)
            {
                calc.resize(new_size);
            };

            using bttn = hids::events::mouse::button;
            LISTEN(tier::release, hids::events::mouse::scroll::any, gear)
            {
                if (gear.meta(hids::anyCtrl)) return; // Ctrl+Wheel is reserved for zooming.
                if (gear.whldt)
                {
                    auto dir = gear.whldt < 0 ? 1 : -1;
                    pager(dir);
                    gear.dismiss();
                }
            };
            LISTEN(tier::release, hids::events::mouse::move, gear)
            {
                calc.cursor_pos = gear.mouse::coord[Axis];
            };
            LISTEN(tier::release, hids::events::mouse::button::dblclick::left, gear)
            {
                gear.dismiss(); // Do not pass double clicks outside.
            };
            LISTEN(tier::release, hids::events::mouse::button::down::any, gear)
            {
                if (!on_pager)
                if (this->form::template protos<tier::release>(bttn::down::left)
                 || this->form::template protos<tier::release>(bttn::down::right))
                if (auto dir = calc.inside(gear.mouse::coord[Axis]))
                {
                    if (gear.capture(bell::id))
                    {
                        on_pager = true;
                        pager_repeat();
                        gear.dismiss();

                        timer.actify(activity::pager_first, skin::globals().repeat_delay, [&](auto p)
                        {
                            if (pager_repeat())
                            {
                                timer.actify(activity::pager_next, skin::globals().repeat_rate, [&](auto d)
                                {
                                    return pager_repeat(); // Repeat until on_pager.
                                });
                            }
                            return faux; // One shot call (first).
                        });
                    }
                }
            };
            LISTEN(tier::release, hids::events::mouse::button::up::any, gear)
            {
                if (on_pager && gear.captured(bell::id))
                {
                    if (this->form::template protos<tier::release>(bttn::up::left)
                     || this->form::template protos<tier::release>(bttn::up::right))
                    {
                        gear.setfree();
                        gear.dismiss();
                        on_pager = faux;
                        timer.pacify(activity::pager_first);
                        timer.pacify(activity::pager_next);
                    }
                }
            };
            LISTEN(tier::release, hids::events::mouse::button::up::right, gear)
            {
                //if (!gear.captured(bell::id)) //todo why?
                {
                    send<upon::scroll::cancel>();
                    gear.dismiss();
                }
            };

            LISTEN(tier::release, hids::events::mouse::button::drag::start::any, gear)
            {
                if (on_pager)
                {
                    gear.dismiss();
                }
                else
                {
                    if (gear.capture(bell::id))
                    {
                        gear.dismiss();
                    }
                }
            };
            LISTEN(tier::release, hids::events::mouse::button::drag::pull::any, gear)
            {
                if (on_pager)
                {
                    gear.dismiss();
                }
                else
                {
                    if (gear.captured(bell::id))
                    {
                        if (auto delta = gear.mouse::delta.get()[Axis])
                        {
                            calc.stepby(delta);
                            send<upon::scroll::bycoor>();
                            gear.dismiss();
                        }
                    }
                }
            };
            LISTEN(tier::release, hids::events::mouse::button::drag::cancel::any, gear)
            {
                giveup(gear);
            };
            LISTEN(tier::general, hids::events::halt, gear)
            {
                giveup(gear);
            };
            LISTEN(tier::release, hids::events::mouse::button::drag::stop::any, gear)
            {
                if (on_pager)
                {
                    gear.dismiss();
                }
                else
                {
                    if (gear.captured(bell::id))
                    {
                        if (this->form::template protos<tier::release>(bttn::drag::stop::right))
                        {
                            send<upon::scroll::cancel>();
                        }
                        base::deface();
                        gear.setfree();
                        gear.dismiss();
                    }
                }
            };
            LISTEN(tier::release, e2::form::state::mouse, active)
            {
                auto apply = [&](auto active)
                {
                    wide = active;
                    if (Axis == axis::Y && mult) config(active ? init * mult // Make vertical scrollbar
                                                               : init);      // wider on hover.
                    base::reflow();
                    return faux; // One shot call.
                };

                timer.pacify(activity::mouse_leave);

                if (active) apply(activity::mouse_hover);
                else timer.actify(activity::mouse_leave, skin::globals().active_timeout, apply);
            };
            //LISTEN(tier::release, hids::events::mouse::move, gear)
            //{
            //	auto apply = [&](auto active)
            //	{
            //		wide = active;
            //		if (Axis == axis::Y) config(active ? init * 2 // Make vertical scrollbar
            //		                                   : init);   //  wider on hover
            //		base::reflow();
            //		return faux; // One shot call
            //	};
            //
            //	timer.pacify(activity::mouse_leave);
            //	apply(activity::mouse_hover);
            //	timer.template actify<activity::mouse_leave>(skin::globals().active_timeout, apply);
            //};
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                auto region = parent_canvas.view();
                auto object = parent_canvas.full();
                auto handle = region;

                calc.commit(handle);

                auto& handle_len = handle.size[Axis];
                auto& region_len = region.size[Axis];
                auto& object_len = object.size[Axis];

                handle = region.clip(handle);
                handle_len = std::max(1, handle_len);

                drawfx(*this, parent_canvas, handle, object_len, handle_len, region_len, wide);
            };
        }
    };

    static constexpr auto drawfx = [](auto& boss, auto& canvas, auto handle, auto object_len, auto handle_len, auto region_len, auto wide)
    {
        if (object_len && handle_len != region_len) // Show only if it is oversized.
        {
            // Brightener isn't suitable for white backgrounds.
            //auto bright = skin::color(tone::brighter);
            //bright.bga(bright.bga() / 2).fga(0);
            //bright.link(bell::id);

            if (wide) // Draw full scrollbar on mouse hover
            {
                canvas.fill([&](cell& c) { c.link(boss.bell::id).xlight(); });
            }
            //canvas.fill(handle, [&](cell& c) { c.fusefull(bright); });
            canvas.fill(handle, [&](cell& c) { c.link(boss.bell::id).xlight(); });
        }
    };

    template<axis Axis>
    using grip = gripfx<Axis, drawfx>;

    // controls: Container with margins (outer space) and padding (inner space).
    class pads
        : public form<pads>
    {
        dent padding; // pads: Space around an element's content, outside of any defined borders. It does not affect the size, only affects the fill. Used in base::renderproc only.
        dent margins; // pads: Space around an element's content, inside of any defined borders. Containers take this parameter into account when calculating sizes. Used in all conainers.

    public:
        sptr client;

       ~pads()
        {
            if (client)
            {
                auto empty = e2::form::upon::vtree::detached.param();
                auto item_ptr = client;
                client.reset();
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, empty);
            }
        }
        pads(dent const& padding_value = {}, dent const& margins_value = {})
            : padding{ padding_value },
              margins{ margins_value }
        {
            LISTEN(tier::preview, e2::size::set, new_size)
            {
                if (client)
                {
                    auto client_size = new_size - padding;
                    client->SIGNAL(tier::preview, e2::size::set, client_size);
                    new_size = client_size + padding;
                    //todo unify
                    //auto lims = base::limits();
                    //new_size = std::clamp(new_size, lims.min, lims.max);
                }
            };
            LISTEN(tier::release, e2::size::any, new_size)
            {
                if (client)
                {
                    auto client_size = new_size - padding;
                    auto client_coor = padding.corner();
                    client->SIGNAL(tier::release, e2::size::set, client_size);
                    client->SIGNAL(tier::release, e2::coor::set, client_coor);
                }
            };
            LISTEN(tier::release, e2::render::prerender, parent_canvas)
            {
                auto view = parent_canvas.view();
                parent_canvas.view(view + margins);
                this->SIGNAL(tier::release, e2::render::any, parent_canvas);
                parent_canvas.view(view);
                if (client)
                {
                    parent_canvas.render(client, base::coor());
                }
                this->bell::expire<tier::release>();
            };
        }
        // pads: Attach specified item.
        template<class T>
        auto attach(T item_ptr)
        {
            if (client) remove(client);
            client = item_ptr;
            item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::attached, This());
            return item_ptr;
        }
        // pads: Remove item.
        void remove(sptr item_ptr)
        {
            if (client == item_ptr)
            {
                auto backup = This();
                client.reset();
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, backup);
            }
        }
        // pads: Update nested object.
        void update(sptr old_item_ptr, sptr new_item_ptr)
        {
            auto backup = This();
            client = new_item_ptr;
            old_item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, backup);
            new_item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::attached, backup);
        }
    };

    // controls: Pluggable dummy object.
    class mock
        : public form<mock>
    { };

    // controls: Menu item.
    class item
        : public form<item>
    {
        pro::limit limit{ *this }; // item: Size limits.

        static constexpr auto dots = "…"sv;
        para name;
        bool flex; // item: Violate or not the label size, default is faux.
        bool test; // item: Place or not(default) the Two Dot Leader when there is not enough space.
        bool unln; // item: Full width underline.

        void recalc()
        {
            auto size = name.size();
            auto lims = flex ? twod{ -1,size.y } : size;
            limit.set(lims, lims);
            base::resize(size);
        }

    public:
        item(para const& label_para, bool flexible = faux, bool check_size = faux, bool underline = faux)
            : name{ label_para },
              flex{ flexible   },
              test{ check_size },
              unln{ underline  }
        {
            recalc();
            LISTEN(tier::release, e2::data::utf8, label_text)
            {
                set(label_text);
            };
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                parent_canvas.cup(dot_00);
                parent_canvas.output(name);
                if (test)
                {
                    auto area = parent_canvas.view();
                    auto size = name.size();
                    if (area.size > 0 && size.x > 0)
                    {
                        auto full = parent_canvas.full();
                        if (full.coor.x < area.coor.x)
                        {
                            auto coor = area.coor;
                            parent_canvas.core::data(coor)->txt(dots);
                        }
                        if (full.coor.x + size.x > area.coor.x + area.size.x)
                        {
                            auto coor = area.coor + area.size - dot_11;
                            parent_canvas.core::data(coor)->txt(dots);
                        }
                    }
                }
                if (unln)
                {
                    auto area = parent_canvas.view();
                    parent_canvas.fill(area, [](cell& c)
                    {
                        auto u = c.und();
                        if (u == 1) c.und(2);
                        else        c.und(1);
                    });
                }
            };
        }
        item(text const& label_text, bool flexible = faux, bool check_size = faux, bool underline = faux)
            : item(para{ label_text }, flexible, check_size, underline)
        { }
        void set(text const& label_text)
        {
            name = label_text;
            recalc();
        }
    };

    // controls: Textedit box.
    class edit
        : public form<edit>
    {
        page data;

    public:
        edit()
        {
        }
    };

    // DEPRECATED STUFF

    class stem_rate_grip
        : public base
    {
        pro::mouse mouse{*this }; // stem_rate_grip: Mouse controller.

        //todo cache specific
        sptr<face> coreface;
        face& canvas;

    public:
        page topic; // stem_rate_grip: Text content.

        bool enabled;
        text sfx_str;
        si32 sfx_len;
        text pin_str;
        si32 cur_val;
        twod box_len;

        enum
        {
            txt_id,
            pin_id,
        };

        void set_pen(byte hilight)
        {
            auto& pen = canvas.mark().bga(hilight);
        }
        void recalc()
        {
            auto cur_str = std::to_string(cur_val);
            auto cur_len = utf::length(cur_str);
            auto pin_pos = std::max(cur_len, sfx_len) + 1;
            box_len.x = 1 + 2 * pin_pos;
            box_len.y = 4;

            topic[txt_id] = cur_str + " " + sfx_str;
            topic[pin_id] = pin_str;
            topic[txt_id].locus.kill().chx(pin_pos - cur_len).cud(1);
            topic[pin_id].locus.kill().chx(pin_pos);

            base::resize(box_len);
            deface();
        }
        auto set_val(si32 new_val, view pin_chr)
        {
            cur_val = new_val;
            pin_str = pin_chr;
            recalc();
            return box_len;
        }

        stem_rate_grip(view sfx_string)
            : sfx_str{ sfx_string }, canvas{*(coreface = ptr::shared<face>())}
        {
            //todo cache specific
            canvas.link(bell::id);
            LISTEN(tier::release, e2::size::any, new_sz) { canvas.size(new_sz); };
            LISTEN(tier::release, e2::coor::any, new_xy) { canvas.move(new_xy); };
            LISTEN(tier::request, e2::form::canvas, canvas) { canvas = coreface; };

            sfx_len = utf::length(sfx_str);

            canvas.mark().bgc(whitelt);
            topic = ansi::idx(txt_id).nop().eol()
                         .idx(pin_id).nop();

            set_pen(0);

            LISTEN(tier::preview, e2::size::set, size)
            {
                size = box_len; // Suppress resize.
            };
            LISTEN(tier::release, e2::form::state::mouse, active)
            {
                set_pen(active ? 80 : 0);
                recalc();
            };
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                if (base::ruined())
                {
                    canvas.wipe();
                    canvas.output(topic);
                    base::ruined(faux);
                }
                parent_canvas.fill(canvas, cell::shaders::fusefull);
            };
        }
    };

    template<tier Tier, class Event>
    class stem_rate
        : public form<stem_rate<Tier, Event>>
    {
        pro::robot robot{*this }; // stem_rate: Animation controller.
        pro::limit limit{*this }; // stem_rate: Size limits.

        //todo cache specific
        sptr<face> coreface;
        face& canvas;

        using tail = netxs::datetime::tail<si32>;

    public:
        page topic; // stem_rate: Text content.

        sptr<stem_rate_grip> grip_ctl;

        enum
        {
            //   cur_id + "sfx_str"
            //  ────█─────────bar_id──
            // min_id            max_id

            bar_id,
            min_id,
            max_id,
        };

        twod pin_len;
        si32 pin_pos = 0;
        si32 bar_len = 0;
        si32 cur_val = 0;
        si32 min_val = 0;
        si32 max_val = 0;
        si32 origin = 0;
        si32 deltas = 0;

        //todo unify mint = 1/fps60 = 16ms
        //seems that the 4ms is enough, no need to bind with fps (opened question)
        tail bygone{ 75ms, 4ms };

        text grip_suffix;
        text label_text;
        si32 pad = 5;
        si32 bgclr = 4;

        void recalc()
        {
            bar_len = std::max(0, base::size().x - (pad + 1) * 2);
            auto pin_abs = netxs::divround((bar_len + 1) * (cur_val - min_val),
                (max_val - min_val));
            auto pin_str = text{};
                 if (pin_abs == 0)           pin_str = "├";
            else if (pin_abs == bar_len + 1) pin_str = "┤";
            else                             pin_str = "┼";

            pin_len = grip_ctl->set_val(cur_val, pin_str);
            pin_pos = pad + pin_abs - pin_len.x / 2;
            topic[bar_id] = "└" + utf::repeat("─", bar_len) + "┘";
            topic[bar_id].locus.kill().chx(pad);
        }

        si32 next_val(si32 delta)
        {
            auto dm = max_val - min_val;
            auto p = divround((bar_len + 1) * (origin - min_val), dm);
            auto c = divround((p + delta) * dm, (bar_len + 1)) + min_val;
            bygone.set(c - cur_val);
            return c;
        }

        bool _move_grip(si32 new_val)
        {
            new_val = std::clamp(new_val, min_val, max_val);
            if (new_val != cur_val)
            {
                cur_val = new_val;
                recalc();
                base::deface();

                return true;
            }
            return faux;
        }
        void move_grip(si32 new_val)
        {
            if (_move_grip(new_val))
            {
                RISEUP(Tier, Event{}, cur_val);
            }
        }
        void giveup(hids& gear)
        {
            if (gear.captured(grip_ctl->id))
            {
                deltas = 0;
                move_grip(origin);
                gear.setfree();
                gear.dismiss();
            }
        }

        stem_rate(text const& caption, si32 min_value, si32 max_value, view suffix)
            : min_val{ min_value },
              max_val{ max_value },
              grip_suffix{ suffix },
              canvas{*(coreface = ptr::shared<face>())}
        {
            //todo cache specific
            canvas.link(bell::id);
            LISTEN(tier::release, e2::size::any, new_sz) { canvas.size(new_sz); };
            LISTEN(tier::release, e2::coor::any, new_xy) { canvas.move(new_xy); };
            LISTEN(tier::request, e2::form::canvas, canvas) { canvas = coreface; };

            cur_val = -1;
            RISEUP(Tier, Event{}, cur_val);

            limit.set(twod{ utf::length(caption) + (pad + 2) * 2,
                           10 });

            topic = ansi::wrp(wrap::off).jet(bias::left)
                .cpy(50).chx(pad + 2).cuu(3).add(caption).cud(3)
                .idx(bar_id).nop().eol()
                .idx(min_id).nop().idx(max_id).nop();

            topic[min_id] = std::to_string(min_val);
            topic[max_id] = std::to_string(max_val);
            topic[max_id].style.jet(bias::right);
            topic[max_id].locus.chx(pad);
            topic[min_id].locus.chx(pad);

            LISTEN(tier::general, e2::form::global::lucidity, alpha)
            {
                if (alpha >= 0 && alpha < 256)
                {
                    canvas.mark().alpha(alpha);
                    base::deface();
                }
            };
            LISTEN(tier::general, Event{}, cur_val)
            {
                if (cur_val >= min_val)
                {
                    _move_grip(cur_val);
                }
            };
            LISTEN(tier::release, e2::form::upon::vtree::attached, parent)
            {
                grip_ctl = base::create<stem_rate_grip>(grip_suffix);
                grip_ctl->SIGNAL(tier::release, e2::form::upon::vtree::attached, base::This());

                grip_ctl->LISTEN(tier::release, hids::events::mouse::button::drag::start::left, gear)
                {
                    if (gear.capture(grip_ctl->id))
                    {
                        origin = cur_val;
                        gear.dismiss();
                    }
                };
                grip_ctl->LISTEN(tier::release, hids::events::mouse::button::drag::pull::left, gear)
                {
                    if (gear.captured(grip_ctl->id))
                    {
                        deltas += gear.mouse::delta.get().x;
                        move_grip(next_val(deltas));
                        gear.dismiss();
                    }
                };
                grip_ctl->LISTEN(tier::release, hids::events::mouse::button::drag::cancel::left, gear)
                {
                    giveup(gear);
                };
                grip_ctl->LISTEN(tier::general, hids::events::halt, gear)
                {
                    giveup(gear);
                };
                grip_ctl->LISTEN(tier::release, hids::events::mouse::button::drag::stop::left, gear)
                {
                    if (gear.captured(grip_ctl->id))
                    {
                        deltas = 0;
                        gear.setfree();
                        base::deface();
                        robot.actify(bygone.fader<quadratic<si32>>(750ms), [&](auto& delta)
                            {
                                move_grip(cur_val + delta);
                            });
                        gear.dismiss();
                    }
                };
                grip_ctl->LISTEN(tier::release, hids::events::mouse::scroll::up, gear)
                {
                    if (gear.meta(hids::anyCtrl)) return; // Ctrl+Wheel is reserved for zooming.
                    move_grip(cur_val - 1);
                    gear.dismiss();
                };
                grip_ctl->LISTEN(tier::release, hids::events::mouse::scroll::down, gear)
                {
                    if (gear.meta(hids::anyCtrl)) return; // Ctrl+Wheel is reserved for zooming.
                    move_grip(cur_val + 1);
                    gear.dismiss();
                };
                recalc();
            };
            LISTEN(tier::release, e2::size::any, size)
            {
                recalc();
            };
            LISTEN(tier::release, hids::events::mouse::button::click::right, gear)
            {
                base::color(canvas.mark().fgc(), (tint)((++bgclr) % 16));
                base::deface();
                gear.dismiss();
            };
            LISTEN(tier::release, hids::events::mouse::scroll::up, gear)
            {
                if (gear.meta(hids::anyCtrl)) return; // Ctrl+Wheel is reserved for zooming.
                move_grip(cur_val - 10);
                gear.dismiss();
            };
            LISTEN(tier::release, hids::events::mouse::scroll::down, gear)
            {
                if (gear.meta(hids::anyCtrl)) return; // Ctrl+Wheel is reserved for zooming.
                move_grip(cur_val + 10);
                gear.dismiss();
            };
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                if (base::ruined())
                {
                    base::ruined(faux); // The object may be invalidated again during rendering. (see pro::d_n_d)
                    canvas.wipe(base::color());
                    canvas.output(topic);
                    auto cp = canvas.cp();
                    cp.x = pin_pos;
                    cp.y -= 3;
                    grip_ctl->base::moveto(cp);
                    canvas.render(grip_ctl, base::coor());
                }
                parent_canvas.fill(canvas, cell::shaders::fusefull);
            };
        }
    };
}