// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_CONTROLS_HPP
#define NETXS_CONTROLS_HPP

#include "../console/console.hpp"

#include <vector>
#include <mutex>
#include <map>

namespace netxs::ui
{
    using namespace netxs::ui::atoms;
    using namespace netxs::console;

    enum sort
    {
         forward,
         reverse,
    };
    enum slot { _1, _2, _I };
    enum axis { X, Y };
    enum axes
    {
        NONE   = 0,
        X_ONLY = 1 << 0,
        Y_ONLY = 1 << 1,
        ALL    = (X_ONLY | Y_ONLY),
    };
    enum snap
    {
        none,
        head,
        tail,
        stretch,
        center,
    };
    // controls: base UI element.
    template<class T>
    class form
        : public base
    {
        std::map<std::type_index, uptr<pro::skill>> depo;
        std::map<id_t, subs> memomap; // form: Token set for depend submissions.

        //todo use C++20 requires expressions
        template <class A>
        struct has
        {
            template <class B> static int16_t _(decltype(&B::remove));
            template <class B> static uint8_t _(...);
            static constexpr bool remove = sizeof(_<A>(nullptr)) - 1;
        };

    public:
        using sptr = netxs::sptr<base>;

        pro::mouse mouse{ *this }; // form: Mouse controller.
        pro::keybd keybd{ *this }; // form: Keybd controller.

        auto This() { return base::This<T>(); }
        form()
        {
            if constexpr (has<T>::remove)
            {
                SUBMIT(tier::preview, e2::form::proceed::detach, shadow)
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
        auto isroot(bool state, iota kind = 0)
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
            master.SUBMIT_T(tier::release, e2::dtor, memomap[master.id], id)
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
        template<class PROPERTY, class SPTR, class P>
        auto attach_element(PROPERTY, SPTR data_src_sptr, P item_template)
        {
            using prop_t = typename PROPERTY::type;
            auto backup = This();
            prop_t arg_value;
            data_src_sptr->SIGNAL(tier::request, PROPERTY{}, arg_value);
            auto new_item = item_template(data_src_sptr, arg_value)
                                 ->depend(data_src_sptr);
            auto item_shadow = ptr::shadow(new_item);
            auto data_shadow = ptr::shadow(data_src_sptr);
            auto boss_shadow = ptr::shadow(backup);
            data_src_sptr->SUBMIT_T_BYVAL(tier::release, PROPERTY{}, memomap[data_src_sptr->id], arg_new_value)
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
        template<class PROPERTY, class S, class P>
        auto attach_collection(PROPERTY, S& data_collection_src, P item_template)
        {
            auto backup = This();
            for (auto& data_src_sptr : data_collection_src)
            {
                attach_element(PROPERTY{}, data_src_sptr, item_template);
            }
            return backup;
        }
    };

    // controls: Splitter.
    class fork
        : public form<fork>
    {
        enum action { seize, drag, release };

        static constexpr iota MAX_RATIO = 0xFFFF;
        static constexpr iota HALF_RATIO = 0xFFFF >> 1;

        sptr client_1; // fork: 1st object.
        sptr client_2; // fork: 2nd object.
        sptr splitter; // fork: Resizing grip.

        twod size1;
        twod size2;
        twod coor2;
        twod size3;
        twod coor3;
        iota split = 0;

        tint clr;
        //twod size;
        rect stem;
        iota start;
        iota width;
        iota ratio;
        iota maxpos;
        bool updown;
        bool movable;
        bool fixed;

        twod xpose(twod const& pt) { return updown ? twod{ pt.y, pt.x } : pt; }
        iota get_x(twod const& pt) { return updown ? pt.y : pt.x; }
        iota get_y(twod const& pt) { return updown ? pt.x : pt.y; }

        void _config(axis alignment, iota thickness, iota s1 = 1, iota s2 = 1)
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
        void _config_ratio(iota s1, iota s2)
        {
            if (s1 < 0) s1 = 0;
            if (s2 < 0) s2 = 0;
            auto sum = s1 + s2;
            ratio = sum ? netxs::divround(s1 * MAX_RATIO, sum)
                        : MAX_RATIO >> 1;
        }

    public:
        auto get_ratio()
        {
            return ratio;
        }
        void config(iota s1, iota s2 = 1)
        {
            _config_ratio(s1, s2);
            base::reflow();
        }
        auto config(axis alignment, iota thickness, iota s1, iota s2)
        {
            _config(alignment, thickness, s1, s2);
            return This();
        }

        ~fork()
        {
            events::sync lock;
            auto empty = decltype(e2::form::upon::vtree::detached)::type{};
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
        fork(axis alignment = axis::X, iota thickness = 0, iota s1 = 1, iota s2 = 1)
            : maxpos{ 0 },
              start{ 0 },
              width{ 0 },
              movable{ true },
              updown{ faux },
              ratio{ 0xFFFF >> 1 },
              fixed{ faux }
        {
            SUBMIT(tier::release, e2::form::prop::fixedsize, is_fixed) //todo unify -- See terminal window self resize
            {
                fixed = is_fixed;
            };
            SUBMIT(tier::preview, e2::size::set, new_size)
            {
                fork::size_preview(new_size);
            };
            SUBMIT(tier::release, e2::size::set, new_size)
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
            SUBMIT(tier::release, e2::render::any, parent_canvas)
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
                split = netxs::divround(maxpos * ratio, MAX_RATIO);

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

        void slider(action act, twod const& delta)
        {
            if (movable)
            {
                switch (act)
                {
                    case action::seize:
                        //control_w32::mouse(true);
                        start = get_x(stem.coor) - get_x(delta);
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
                        //control_w32::mouse(false);
                        break;
                    default:
                        return;
                }

                //todo move slider

                //fork::deploy(start + xpose(delta).x, true);
                //iota newpos = start + xpose(delta).x;
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

                ratio = std::clamp(netxs::divround(get_x(stem.coor) * MAX_RATIO, maxpos), 0, MAX_RATIO);

                base::deface();
            }
        }
        template<class T>
        auto attach(slot SLOT, T item_ptr)
        {
            if (SLOT == slot::_1)
            {
                if (client_1) remove(client_1);
                client_1 = item_ptr;
            }
            else if (SLOT == slot::_2)
            {
                if (client_2) remove(client_2);
                client_2 = item_ptr;
            }
            else
            {
                if (splitter) remove(splitter);
                splitter = item_ptr;
                splitter->SUBMIT(tier::preview, e2::form::upon::changed, delta)
                {
                    split += get_x(delta);
                    ratio = netxs::divround(MAX_RATIO * split, maxpos);
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
            events::sync lock;
            auto empty = decltype(e2::form::upon::vtree::detached)::type{};
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
            SUBMIT(tier::preview, e2::size::set, new_sz)
            {
                iota  height;
                auto& y_size = updown ? new_sz.y : new_sz.x;
                auto& x_size = updown ? new_sz.x : new_sz.y;
                auto  x_temp = x_size;
                auto  y_temp = y_size;

                auto meter = [&]()
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
            SUBMIT(tier::release, e2::size::set, new_sz)
            {
                auto& y_size = updown ? new_sz.y : new_sz.x;
                auto& x_size = updown ? new_sz.x : new_sz.y;
                twod  new_xy;
                auto& y_coor = updown ? new_xy.y : new_xy.x;
                auto& x_coor = updown ? new_xy.x : new_xy.y;

                auto  found = faux;
                for (auto& client : subset)
                {
                    y_size = client.second.y;
                    if (client.first)
                    {
                        auto& entry = *client.first;
                        if (!found)
                        {
                            // todo optimize: use the only one axis to hittest
                            // todo detect client during preview, use wptr
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
            SUBMIT(tier::release, e2::render::any, parent_canvas)
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
            events::sync lock;
            auto empty = decltype(e2::form::upon::vtree::detached)::type{};
            while (subset.size())
            {
                auto item_ptr = subset.back();
                subset.pop_back();
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, empty);
            }
        }
        cake()
        {
            SUBMIT(tier::preview, e2::size::set, newsz)
            {
                for (auto& client : subset)
                {
                    if (client)
                        client->SIGNAL(tier::preview, e2::size::set, newsz);
                }
            };
            SUBMIT(tier::release, e2::size::set, newsz)
            {
                for (auto& client : subset)
                {
                    if (client)
                        client->SIGNAL(tier::release, e2::size::set, newsz);
                }
            };
            SUBMIT(tier::release, e2::render::any, parent_canvas)
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

        void xform(snap align, iota& coor, iota& size, iota width)
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
            events::sync lock;
            auto empty = decltype(e2::form::upon::vtree::detached)::type{};
            while (subset.size())
            {
                auto item_ptr = subset.back().ptr;
                subset.pop_back();
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, empty);
            }
        }
        park()
        {
            SUBMIT(tier::release, e2::size::set, newsz)
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
            SUBMIT(tier::release, e2::render::any, parent_canvas)
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
        auto attach(snap hz, snap vt, T item_ptr)
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
            events::sync lock;
            auto empty = decltype(e2::form::upon::vtree::detached)::type{};
            while (subset.size())
            {
                auto item_ptr = subset.back();
                subset.pop_back();
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, empty);
            }
        }
        veer()
        {
            SUBMIT(tier::preview, e2::size::set, newsz)
            {
                if (subset.size())
                if (auto active = subset.back())
                    active->SIGNAL(tier::preview, e2::size::set, newsz);
            };
            SUBMIT(tier::release, e2::size::set, newsz)
            {
                if (subset.size())
                if (auto active = subset.back())
                    active->SIGNAL(tier::release, e2::size::set, newsz);
            };
            SUBMIT(tier::release, e2::render::any, parent_canvas)
            {
                if (subset.size())
                if (auto active = subset.back())
                {
                    auto& basis = base::coor();
                    parent_canvas.render(active, basis);
                }
            };
        }
        // veer: Return the last object refrence or empty sptr.
        auto back()
        {
            return subset.size() ? subset.back()
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

        static const iota init_layout_len = 10000;
        std::vector<item> layout;
        page_layout()
        {
            layout.reserve(init_layout_len);
        }

        auto get_entry(twod const& anchor)
        {
            auto& anker = anchor.y;
            item pred = { 0, twod{ 0, std::numeric_limits<iota>::max() } };
            item minp = { 0, twod{ 0, std::numeric_limits<iota>::max() } };
            iota mindist = std::numeric_limits<iota>::max();

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

    // controls: Rigid text page.
    class post
        : public flow, public form<post>
    {
        twod width; // post: Page dimensions.
        text source; // post: Raw content.
        page_layout layout;
        bool beyond; // post: Allow vertical scrolling beyond last line.

    public:
        //using post = post_fx;

        page topic; // post: Text content.

        template<class T>
        auto& lyric(T paraid) { return *topic[paraid].lyric; }
        template<class T>
        auto& content(T paraid) { return topic[paraid]; }

        // post: Set content.
        template<class TEXT>
        auto upload(TEXT utf8, iota initial_width = 0)
        {
            source = utf8;
            topic = utf8;
            base::resize(twod{ initial_width, 0 });
            base::reflow();
            return This();
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
                flow::print(combo, canvas);
            };
            topic.stream(publish);
        }
        auto get_size() const
        {
            return width;
        }
        void recalc()
        {
            if (topic.size() > layout.capacity())
                layout.reserve(topic.size() * 2);

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
            width.y = height + (beyond ? width.y - 1 : 0); //todo unify (text editor)
        }
        void recalc(twod const& size)
        {
            width = size;
            recalc();
        }

        post(bool scroll_beyond = faux)
            : flow{ width },
              beyond{ scroll_beyond }
        {
            SUBMIT(tier::preview, e2::size::set, size)
            {
                recalc(size);
                size.y = width.y;
            };
            SUBMIT(tier::release, e2::size::set, size)
            {
                //if (width != size)
                //{
                //	recalc(size);
                //	//width.y = size.y;
                //}
                width = size;
            };
            SUBMIT(tier::release, e2::render::any, parent_canvas)
            {
                output(parent_canvas);

                //auto mark = rect{ base::anchor + base::coor(), {10,5} };
                //mark.coor += parent_canvas.view().coor; // Set client's basis
                //parent_canvas.fill(mark, [](cell& c) { c.alpha(0x80).bgc().chan.r = 0xff; });
            };
        }
    };

    // controls: Rigid text page.
    //template<auto printfx = noop{}> //todo apple clang doesn't get it
    //class post_fx
    //    : public flow, public form<post_fx<printfx>>
    class post_fx
        : public flow, public form<post_fx>
    {
        twod width; // post: Page dimensions.
        text source; // post: Raw content.
        page_layout layout;
        bool beyond; // post: Allow vertical scrolling beyond last line.

    public:
        using post = post_fx;

        page topic; // post: Text content.

        template<class T>
        auto& lyric(T paraid) { return *topic[paraid].lyric; }
        template<class T>
        auto& content(T paraid) { return topic[paraid]; }

        // post: Set content.
        template<class TEXT>
        auto upload(TEXT utf8, iota initial_width = 0)
        {
            source = utf8;
            topic = utf8;
            base::resize(twod{ initial_width, 0 });
            base::reflow();
            return This();
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
                flow::print(combo, canvas, cell::shaders::contrast);
            };
            topic.stream(publish);
        }
        auto get_size() const
        {
            return width;
        }
        void recalc()
        {
            if (topic.size() > layout.capacity())
                layout.reserve(topic.size() * 2);

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

        post_fx(bool scroll_beyond = faux)
            : flow{ width },
              beyond{ scroll_beyond }
        {
            SUBMIT(tier::preview, e2::size::set, size)
            {
                recalc(size);
                size.y = width.y;
            };
            SUBMIT(tier::release, e2::size::set, size)
            {
                //if (width != size)
                //{
                //	recalc(size);
                //	//width.y = size.y;
                //}
                width = size;
            };
            SUBMIT(tier::release, e2::render::any, parent_canvas)
            {
                output(parent_canvas);

                //auto mark = rect{ base::anchor + base::coor(), {10,5} };
                //mark.coor += parent_canvas.view().coor; // Set client's basis
                //parent_canvas.fill(mark, [](cell& c) { c.alpha(0x80).bgc().chan.r = 0xff; });
            };
        }
    };

    //using post = post_fx<>;  //todo apple clang doesn't get it
    
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

        iota speed{ SPD  }; // rail: Text auto-scroll initial speed component ΔR.
        iota pulse{ PLS  }; // rail: Text auto-scroll initial speed component ΔT.
        iota cycle{ CCL  }; // rail: Text auto-scroll duration in ms.
        bool steer{ faux }; // rail: Text scroll vertical direction.

        static constexpr auto xy(axes AXES)
        {
            return twod{ !!(AXES & axes::X_ONLY), !!(AXES & axes::Y_ONLY) };
        }

    public:
        template<axis AXIS>
        auto follow(sptr master = {})
        {
            if (master)
            {
                master->SUBMIT_T(tier::release, upon::scroll::bycoor::any, fasten, master_scinfo)
                {
                    this->SIGNAL(tier::preview, e2::form::upon::scroll::bycoor::_<AXIS>, master_scinfo);
                };
            }
            else fasten.clear();

            return This();
        }
        ~rail()
        {
            if (client)
            {
                auto empty = decltype(e2::form::upon::vtree::detached)::type{};
                auto item_ptr = client;
                client.reset();
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, empty);
            }
        }
        rail(axes allow_to_scroll = axes::ALL, axes allow_to_capture = axes::ALL, axes allow_overscroll = axes::ALL)
            : permit{ xy(allow_to_scroll)  },
              siezed{ xy(allow_to_capture) },
              oversc{ xy(allow_overscroll) },
              strict{ xy(axes::ALL) },
              manual{ xy(axes::ALL) }
        {
            SUBMIT(tier::preview, e2::form::upon::scroll::any, info) // Receive scroll parameters from external sources.
            {
                if (client)
                {
                    switch (this->bell::protos<tier::preview>())
                    {
                        case upon::scroll::bycoor::x.id: move<X>(scinfo.window.coor.x - info.window.coor.x); break;
                        case upon::scroll::bycoor::y.id: move<Y>(scinfo.window.coor.y - info.window.coor.y); break;
                        case upon::scroll::bystep::x.id: move<X>(info.vector); break;
                        case upon::scroll::bystep::y.id: move<Y>(info.vector); break;
                        case upon::scroll::bypage::x.id: move<X>(info.vector * scinfo.window.size.x); break;
                        case upon::scroll::bypage::y.id: move<Y>(info.vector * scinfo.window.size.y); break;
                        case upon::scroll::cancel::x.id: cancel<X, true>(); break;
                        case upon::scroll::cancel::y.id: cancel<Y, true>(); break;
                    }
                }
            };
            SUBMIT(tier::request, e2::form::upon::scroll::any, req_scinfo)
            {
                req_scinfo = scinfo;
            };

            SUBMIT(tier::release, e2::size::set, new_size)
            {
                if (client)
                    client->base::resize(new_size, base::anchor);
            };

            using bttn = hids::events::mouse::button;
            SUBMIT(tier::release, hids::events::mouse::scroll::any, gear)
            {
                auto dt = gear.whldt > 0;
                auto hz = permit == xy(axes::X_ONLY)
                       || permit == xy(axes::ALL) && gear.meta(hids::ANYCTRL | hids::SHIFT );
                if (hz) wheels<X>(dt);
                else    wheels<Y>(dt);
                gear.dismiss();
            };
            SUBMIT(tier::release, bttn::drag::start::right, gear)
            {
                auto ds = gear.delta.get();
                auto dx = ds.x;
                auto dy = ds.y * 2;
                auto vt = std::abs(dx) < std::abs(dy);

                if ((siezed[X] && !vt) ||
                    (siezed[Y] &&  vt))
                {
                    if (gear.capture(bell::id))
                    {
                        manual = xy(axes::ALL);
                        strict = xy(axes::ALL) - oversc; // !oversc = dot_11 - oversc
                        gear.dismiss();
                    }
                }
            };
            SUBMIT(tier::release, bttn::drag::pull::right, gear)
            {
                if (gear.captured(bell::id))
                {
                    auto delta = gear.mouse::delta.get();
                    auto value = permit * delta;
                    if (value) movexy(value);
                    gear.dismiss();
                }
            };
            SUBMIT(tier::release, bttn::drag::cancel::right, gear)
            {
                if (gear.captured(bell::id))
                {
                    giveup(gear);
                }
            };
            SUBMIT(tier::general, hids::events::die, gear)
            {
                if (gear.captured(bell::id))
                {
                    giveup(gear);
                }
            };
            SUBMIT(tier::release, bttn::drag::stop::right, gear)
            {
                if (gear.captured(bell::id))
                {
                    auto  v0 = gear.delta.avg();
                    auto& speed = v0.dS;
                    auto  start = datetime::round<iota>(v0.t0);
                    auto  cycle = datetime::round<iota>(v0.dT);
                    auto  limit = datetime::round<iota>(STOPPING_TIME);

                    if (permit[X]) actify<X>(quadratic{ speed.x, cycle, limit, start });
                    if (permit[Y]) actify<Y>(quadratic{ speed.y, cycle, limit, start });
                    //todo if (permit == xy(axes::ALL)) actify(quadratic{ speed, cycle, limit, start });

                    base::deface();
                    gear.release();
                    gear.dismiss();
                }
            };
            SUBMIT(tier::release, bttn::click::right, gear)
            {
                if (!gear.captured(bell::id))
                {
                    if (manual[X]) cancel<X, true>();
                    if (manual[Y]) cancel<Y, true>();
                }
            };
            SUBMIT(tier::release, bttn::down::any, gear)
            {
                cutoff();
            };
            SUBMIT(tier::release, e2::form::animate::reset, id)
            {
                cutoff();
            };
            SUBMIT(tier::release, e2::form::animate::stop, id)
            {
                switch (id)
                {
                    case Y: manual[Y] = true; /*scroll<Y>();*/ break;
                    case X: manual[X] = true; /*scroll<X>();*/ break;
                    default: break;
                }
                base::deface();
            };
            SUBMIT(tier::release, e2::render::any, parent_canvas)
            {
                if (client)
                    parent_canvas.render<faux>(client, base::coor());
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
            gear.release();
            gear.dismiss();
        }
        template<axis AXIS>
        void wheels(bool dir)
        {
            if (robot.active(AXIS) && (steer == dir))
            {
                speed += SPD_ACCEL;
                cycle += CCL_ACCEL;
                speed = std::min(speed, SPD_MAX);
                cycle = std::min(cycle, CCL_MAX);
            }
            else
            {
                steer = dir;
                speed = SPD;
                cycle = CCL;
                //todo at least one line should be
                //move<AXIS>(dir ? 1 : -1);
            }
            keepon<AXIS>(quadratic<iota>(dir ? speed : -speed, pulse, cycle, now<iota>()));
        }
        template<axis AXIS, class FX>
        void keepon(FX&& func)
        {
            strict[AXIS] = true;
            robot.actify(AXIS, std::forward<FX>(func), [&](auto& p)
                {
                    move<AXIS>(p);
                });
        }
        template<axis AXIS>
        auto inside()
        {
            if (client && manual[AXIS]) // Check overscroll if no auto correction.
            {
                auto& item = *client;
                auto frame = base::size()[AXIS];
                auto coord = item.base::coor()[AXIS] - item.oversz.topleft()[AXIS]; // coor - scroll origin basis.
                auto block = item.base::size()[AXIS] + item.oversz.summ()[AXIS];
                auto bound = std::min(frame - block, 0);
                auto clamp = std::clamp(coord, bound, 0);
                return clamp == coord;
            }
            return true;
        }
        template<axis AXIS, class FX>
        void actify(FX&& func)
        {
            if (inside<AXIS>()) keepon<AXIS>(std::forward<FX>(func));
            else                lineup<AXIS>();
        }
        template<axis AXIS, bool FORCED = faux>
        void cancel()
        {
            if (FORCED || !inside<AXIS>()) lineup<AXIS>();
        }
        template<axis AXIS>
        void lineup()
        {
            if (client)
            {
                manual[AXIS] = faux;
                auto block = client->base::area();
                auto coord = block.coor[AXIS];
                auto bound = std::min(base::size()[AXIS] - block.size[AXIS], 0);
                auto newxy = std::clamp(coord, bound, 0);
                auto route = newxy - coord;
                iota tempo = SWITCHING_TIME;
                auto fader = constlinearAtoB<iota>(route, tempo, now<iota>());
                keepon<AXIS>(fader);
            }
        }
        auto scroll(twod& coord)
        {
            twod delta;
            if (client)
            {
                auto& item = *client;
                auto frame = base::size();
                auto block = item.base::size() + item.oversz.summ();
                auto basis = item.oversz.topleft();
                coord -= basis; // Scroll origin basis.
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
                scinfo.beyond = item.oversz;
                scinfo.region = block;
                scinfo.window.coor =-coord; // Viewport.
                scinfo.window.size = frame; //
                SIGNAL(tier::release, upon::scroll::bycoor::any, scinfo);
                coord += basis; // Client origin basis.
                base::deface(); // Main menu redraw trigger.
            }
            return delta;
        }
        void movexy(twod const& delta)
        {
            if (client)
                client->base::moveby(delta);
        }
        template<axis AXIS>
        void move(iota p)
        {
            if (p)
            {
                if constexpr (AXIS == X) movexy({ p, 0 });
                if constexpr (AXIS == Y) movexy({ 0, p });
            }
        }
        // rail: Attach specified item.
        template<class T>
        auto attach(T item_ptr)
        {
            if (client) remove(client);
            client = item_ptr;
            tokens.clear();
            item_ptr->SUBMIT_T(tier::preview, e2::coor::set, tokens.extra(), coor)
            {
                scroll(coor);
            };
            item_ptr->SUBMIT_T(tier::release, e2::size::set, tokens.extra(), size)
            {
                if (client)
                {
                    auto coor = client->base::coor();
                    if (auto delta = scroll(coor))
                    {
                        //todo sync
                    }
                }
            };
            item_ptr->SUBMIT_T(tier::release, e2::form::upon::vtree::detached, tokens.extra(), p)
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
            if (client != old_item_ptr) log(" rail: WARNING! Wrong DOM structure. rail.id=", id);
            attach(new_item_ptr);
        }
    };

    // controls: Scrollbar.
    template<axis AXIS>
    class grip
        : public form<grip<AXIS>>
    {
        pro::timer timer{*this }; // grip: Minimize by timeout.
        pro::limit limit{*this }; // grip: Size limits.

        using sptr = netxs::sptr<base>; //todo gcc (ubuntu 20.04) doesn't get it (see form::sptr)
        using wptr = netxs::wptr<base>;
        using form = ui::form<grip<AXIS>>;
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
            iota& master_len = master_inf.region     [AXIS]; // math: Master len.
            iota& master_pos = master_inf.window.coor[AXIS]; // math: Master viewport pos.
            iota& master_box = master_inf.window.size[AXIS]; // math: Master viewport len.
            iota& master_dir = master_inf.vector;            // math: Master scroll direction.
            iota  scroll_len = 0; // math: Scrollbar len.
            iota  scroll_pos = 0; // math: Scrollbar grip pos.
            iota  scroll_box = 0; // math: Scrollbar grip len.
            iota   m         = 0; // math: Master max pos.
            iota   s         = 0; // math: Scroll max pos.
            double r         = 1; // math: Scroll/master len ratio.

            iota  cursor_pos = 0; // math: Mouse cursor position.

            // math: Calc scroll to master metrics.
            void s_to_m()
            {
                auto scroll_center = scroll_pos + scroll_box / 2.0;
                auto master_center = scroll_len ? scroll_center / r
                                                : 0;
                master_pos = (iota)std::round(master_center - master_box / 2.0);

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
                scroll_box = std::max(1, (iota)(master_box * r));
                scroll_pos = (iota)std::round(scroll_middle - scroll_box / 2.0);

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
                scroll_len = new_size[AXIS];
                m_to_s();
            }
            void stepby(iota delta)
            {
                scroll_pos = std::clamp(scroll_pos + delta, 0, s);
                s_to_m();
            }
            void commit(rect& handle)
            {
                handle.coor[AXIS]+= scroll_pos;
                handle.size[AXIS] = scroll_box;
            }
            auto inside(iota coor)
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
            void setdir(iota dir)
            {
                master_dir = -dir;
            }
        };

        wptr boss;
        hook memo;
        iota thin; // grip: Scrollbar thickness.
        iota init; // grip: Handle base width.
        math calc; // grip: Scrollbar calculator.
        bool wide; // grip: Is the scrollbar active.
        iota mult; // grip: Vertical bar width multiplier.

        bool on_pager = faux;

        template<class EVENT>
        void send()
        {
            if (auto master = this->boss.lock())
            {
                master->SIGNAL(tier::preview, EVENT::template _<AXIS>, calc.master_inf);
            }
        }
        void config(iota width)
        {
            thin = width;
            auto lims = AXIS == axis::X ? twod{ -1,width }
                                        : twod{ width,-1 };
            limit.set(lims, lims);
        }
        void giveup(hids& gear)
        {
            if (on_pager) gear.dismiss();
            else
            {
                if (gear.captured(bell::id))
                {
                    if (this->form::template protos<tier::release>(hids::events::mouse::button::drag::cancel::right))
                    {
                        send<upon::scroll::cancel>();
                    }
                    base::deface();
                    gear.release();
                    gear.dismiss();
                }
            }
        }
        void pager(iota dir)
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
        grip(sptr boss, iota thickness = 1, iota multiplier = 2)
            : boss{ boss       },
              thin{ thickness  },
              wide{ faux       },
              init{ thickness  },
              mult{ multiplier }
        {
            config(thin);

            boss->SUBMIT_T(tier::release, upon::scroll::bycoor::any, memo, scinfo)
            {
                calc.update(scinfo);
                base::deface();
            };

            SUBMIT(tier::release, e2::size::set, new_size)
            {
                calc.resize(new_size);
            };

            using bttn = hids::events::mouse::button;
            SUBMIT(tier::release, hids::events::mouse::scroll::any, gear)
            {
                if (gear.whldt)
                {
                    auto dir = gear.whldt < 0 ? 1 : -1;
                    pager(dir);
                    gear.dismiss();
                }
            };
            SUBMIT(tier::release, hids::events::mouse::move, gear)
            {
                calc.cursor_pos = gear.mouse::coord[AXIS];
            };
            SUBMIT(tier::release, hids::events::mouse::button::dblclick::left, gear)
            {
                gear.dismiss(); // Do not pass double clicks outside.
            };
            SUBMIT(tier::release, hids::events::mouse::button::down::any, gear)
            {
                if (!on_pager)
                if (this->form::template protos<tier::release>(bttn::down::left) ||
                    this->form::template protos<tier::release>(bttn::down::right))
                if (auto dir = calc.inside(gear.mouse::coord[AXIS]))
                {
                    if (gear.capture(bell::id))
                    {
                        on_pager = true;
                        pager_repeat();
                        gear.dismiss();

                        timer.actify(activity::pager_first, REPEAT_DELAY, [&](auto p)
                        {
                            if (pager_repeat())
                            {
                                timer.actify(activity::pager_next, REPEAT_RATE, [&](auto d)
                                {
                                    return pager_repeat(); // Repeat until on_pager.
                                });
                            }
                            return faux; // One shot call (first).
                        });
                    }
                }
            };
            SUBMIT(tier::release, hids::events::mouse::button::up::any, gear)
            {
                if (on_pager && gear.captured(bell::id))
                {
                    if (this->form::template protos<tier::release>(bttn::up::left) ||
                        this->form::template protos<tier::release>(bttn::up::right))
                    {
                        gear.release();
                        gear.dismiss();
                        on_pager = faux;
                        timer.pacify(activity::pager_first);
                        timer.pacify(activity::pager_next);
                    }
                }
            };
            SUBMIT(tier::release, hids::events::mouse::button::up::right, gear)
            {
                //if (!gear.captured(bell::id)) //todo why?
                {
                    send<upon::scroll::cancel>();
                    gear.dismiss();
                }
            };

            SUBMIT(tier::release, hids::events::mouse::button::drag::start::any, gear)
            {
                if (on_pager) gear.dismiss();
                else
                {
                    if (gear.capture(bell::id))
                    {
                        gear.dismiss();
                    }
                }
            };
            SUBMIT(tier::release, hids::events::mouse::button::drag::pull::any, gear)
            {
                if (on_pager) gear.dismiss();
                else
                {
                    if (gear.captured(bell::id))
                    {
                        if (auto delta = gear.mouse::delta.get()[AXIS])
                        {
                            calc.stepby(delta);
                            send<upon::scroll::bycoor>();
                            gear.dismiss();
                        }
                    }
                }
            };
            SUBMIT(tier::release, hids::events::mouse::button::drag::cancel::any, gear)
            {
                giveup(gear);
            };
            SUBMIT(tier::general, hids::events::die, gear)
            {
                giveup(gear);
            };
            SUBMIT(tier::release, hids::events::mouse::button::drag::stop::any, gear)
            {
                if (on_pager) gear.dismiss();
                else
                {
                    if (gear.captured(bell::id))
                    {
                        if (this->form::template protos<tier::release>(bttn::drag::stop::right))
                        {
                            send<upon::scroll::cancel>();
                        }
                        base::deface();
                        gear.release();
                        gear.dismiss();
                    }
                }
            };
            SUBMIT(tier::release, e2::form::state::mouse, active)
            {
                auto apply = [&](auto active)
                {
                    wide = active;
                    if (AXIS == axis::Y && mult) config(active ? init * mult // Make vertical scrollbar
                                                               : init);      // wider on hover.
                    base::reflow();
                    return faux; // One shot call.
                };

                timer.pacify(activity::mouse_leave);

                if (active) apply(activity::mouse_hover);
                else timer.actify(activity::mouse_leave, ACTIVE_TIMEOUT, apply);
            };
            //SUBMIT(tier::release, hids::events::mouse::move, gear)
            //{
            //	auto apply = [&](auto active)
            //	{
            //		wide = active;
            //		if (AXIS == axis::Y) config(active ? init * 2 // Make vertical scrollbar
            //		                                   : init);   //  wider on hover
            //		base::reflow();
            //		return faux; // One shot call
            //	};
            //
            //	timer.pacify(activity::mouse_leave);
            //	apply(activity::mouse_hover);
            //	timer.template actify<activity::mouse_leave>(ACTIVE_TIMEOUT, apply);
            //};
            SUBMIT(tier::release, e2::render::any, parent_canvas)
            {
                auto region = parent_canvas.view();
                auto handle = region;

                calc.commit(handle);

                auto& handle_len = handle.size[AXIS];
                auto& region_len = region.size[AXIS];

                handle = region.clip(handle);
                handle_len = std::max(1, handle_len);

                if (handle_len != region_len) // Show only if it is oversized.
                {
                    // Brightener isn't suitable for white backgrounds.
                    //auto bright = skin::color(tone::brighter);
                    //bright.bga(bright.bga() / 2).fga(0);
                    //bright.link(bell::id);

                    if (wide) // Draw full scrollbar on mouse hover
                    {
                        parent_canvas.fill([&](cell& c) { c.link(bell::id).xlight(); });
                    }
                    //parent_canvas.fill(handle, [&](cell& c) { c.fusefull(bright); });
                    parent_canvas.fill(handle, [&](cell& c) { c.link(bell::id).xlight(); });
                }
            };
        }
    };

    // controls: Scroll bar.
    //template<axis AXIS, auto drawfx = noop{}> //todo apple clang doesn't get it
    //class grip_fx
    //    : public flow, public form<grip_fx<AXIS, drawfx>>
    template<axis AXIS>
    class grip_fx
        : public form<grip_fx<AXIS>>
    {
        pro::timer timer{*this }; // grip: Minimize by timeout.
        pro::limit limit{*this }; // grip: Size limits.

        using sptr = netxs::sptr<base>; //todo gcc (ubuntu 20.04) doesn't get it (see form::sptr)
        using wptr = netxs::wptr<base>;
        using form = ui::form<grip_fx<AXIS>>;
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
            iota& master_len = master_inf.region     [AXIS]; // math: Master len.
            iota& master_pos = master_inf.window.coor[AXIS]; // math: Master viewport pos.
            iota& master_box = master_inf.window.size[AXIS]; // math: Master viewport len.
            iota& master_dir = master_inf.vector;            // math: Master scroll direction.
            iota  scroll_len = 0; // math: Scrollbar len.
            iota  scroll_pos = 0; // math: Scrollbar grip pos.
            iota  scroll_box = 0; // math: Scrollbar grip len.
            iota   m         = 0; // math: Master max pos.
            iota   s         = 0; // math: Scroll max pos.
            double r         = 1; // math: Scroll/master len ratio.

            iota  cursor_pos = 0; // math: Mouse cursor position.

            // math: Calc scroll to master metrics.
            void s_to_m()
            {
                auto scroll_center = scroll_pos + scroll_box / 2.0;
                auto master_center = scroll_len ? scroll_center / r
                                                : 0;
                master_pos = (iota)std::round(master_center - master_box / 2.0);

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
                scroll_box = std::max(1, (iota)(master_box * r));
                scroll_pos = (iota)std::round(scroll_middle - scroll_box / 2.0);

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
                scroll_len = new_size[AXIS];
                m_to_s();
            }
            void stepby(iota delta)
            {
                scroll_pos = std::clamp(scroll_pos + delta, 0, s);
                s_to_m();
            }
            void commit(rect& handle)
            {
                handle.coor[AXIS]+= scroll_pos;
                handle.size[AXIS] = scroll_box;
            }
            auto inside(iota coor)
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
            void setdir(iota dir)
            {
                master_dir = -dir;
            }
        };

        wptr boss;
        hook memo;
        iota thin; // grip: Scrollbar thickness.
        iota init; // grip: Handle base width.
        math calc; // grip: Scrollbar calculator.
        bool wide; // grip: Is the scrollbar active.
        iota mult; // grip: Vertical bar width multiplier.

        bool on_pager = faux;

        template<class EVENT>
        void send()
        {
            if (auto master = this->boss.lock())
            {
                master->SIGNAL(tier::preview, EVENT::template _<AXIS>, calc.master_inf);
            }
        }
        void config(iota width)
        {
            thin = width;
            auto lims = AXIS == axis::X ? twod{ -1,width }
                                        : twod{ width,-1 };
            limit.set(lims, lims);
        }
        void giveup(hids& gear)
        {
            if (on_pager) gear.dismiss();
            else
            {
                if (gear.captured(bell::id))
                {
                    if (this->form::template protos<tier::release>(hids::events::mouse::button::drag::cancel::right))
                    {
                        send<upon::scroll::cancel>();
                    }
                    base::deface();
                    gear.release();
                    gear.dismiss();
                }
            }
        }
        void pager(iota dir)
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
        grip_fx(sptr boss, iota thickness = 1, iota multiplier = 2)
            : boss{ boss       },
              thin{ thickness  },
              wide{ faux       },
              init{ thickness  },
              mult{ multiplier }
        {
            config(thin);

            boss->SUBMIT_T(tier::release, upon::scroll::bycoor::any, memo, scinfo)
            {
                calc.update(scinfo);
                base::deface();
            };

            SUBMIT(tier::release, e2::size::set, new_size)
            {
                calc.resize(new_size);
            };

            using bttn = hids::events::mouse::button;
            SUBMIT(tier::release, hids::events::mouse::scroll::any, gear)
            {
                if (gear.whldt)
                {
                    auto dir = gear.whldt < 0 ? 1 : -1;
                    pager(dir);
                    gear.dismiss();
                }
            };
            SUBMIT(tier::release, hids::events::mouse::move, gear)
            {
                calc.cursor_pos = gear.mouse::coord[AXIS];
            };
            SUBMIT(tier::release, hids::events::mouse::button::dblclick::left, gear)
            {
                gear.dismiss(); // Do not pass double clicks outside.
            };
            SUBMIT(tier::release, hids::events::mouse::button::down::any, gear)
            {
                if (!on_pager)
                if (this->form::template protos<tier::release>(bttn::down::left) ||
                    this->form::template protos<tier::release>(bttn::down::right))
                if (auto dir = calc.inside(gear.mouse::coord[AXIS]))
                {
                    if (gear.capture(bell::id))
                    {
                        on_pager = true;
                        pager_repeat();
                        gear.dismiss();

                        timer.actify(activity::pager_first, REPEAT_DELAY, [&](auto p)
                        {
                            if (pager_repeat())
                            {
                                timer.actify(activity::pager_next, REPEAT_RATE, [&](auto d)
                                {
                                    return pager_repeat(); // Repeat until on_pager.
                                });
                            }
                            return faux; // One shot call (first).
                        });
                    }
                }
            };
            SUBMIT(tier::release, hids::events::mouse::button::up::any, gear)
            {
                if (on_pager && gear.captured(bell::id))
                {
                    if (this->form::template protos<tier::release>(bttn::up::left) ||
                        this->form::template protos<tier::release>(bttn::up::right))
                    {
                        gear.release();
                        gear.dismiss();
                        on_pager = faux;
                        timer.pacify(activity::pager_first);
                        timer.pacify(activity::pager_next);
                    }
                }
            };
            SUBMIT(tier::release, hids::events::mouse::button::up::right, gear)
            {
                //if (!gear.captured(bell::id)) //todo why?
                {
                    send<upon::scroll::cancel>();
                    gear.dismiss();
                }
            };

            SUBMIT(tier::release, hids::events::mouse::button::drag::start::any, gear)
            {
                if (on_pager) gear.dismiss();
                else
                {
                    if (gear.capture(bell::id))
                    {
                        gear.dismiss();
                    }
                }
            };
            SUBMIT(tier::release, hids::events::mouse::button::drag::pull::any, gear)
            {
                if (on_pager) gear.dismiss();
                else
                {
                    if (gear.captured(bell::id))
                    {
                        if (auto delta = gear.mouse::delta.get()[AXIS])
                        {
                            calc.stepby(delta);
                            send<upon::scroll::bycoor>();
                            gear.dismiss();
                        }
                    }
                }
            };
            SUBMIT(tier::release, hids::events::mouse::button::drag::cancel::any, gear)
            {
                giveup(gear);
            };
            SUBMIT(tier::general, hids::events::die, gear)
            {
                giveup(gear);
            };
            SUBMIT(tier::release, hids::events::mouse::button::drag::stop::any, gear)
            {
                if (on_pager) gear.dismiss();
                else
                {
                    if (gear.captured(bell::id))
                    {
                        if (this->form::template protos<tier::release>(bttn::drag::stop::right))
                        {
                            send<upon::scroll::cancel>();
                        }
                        base::deface();
                        gear.release();
                        gear.dismiss();
                    }
                }
            };
            SUBMIT(tier::release, e2::form::state::mouse, active)
            {
                auto apply = [&](auto active)
                {
                    wide = active;
                    if (AXIS == axis::Y && mult) config(active ? init * mult // Make vertical scrollbar
                                                               : init);      // wider on hover.
                    base::reflow();
                    return faux; // One shot call.
                };

                timer.pacify(activity::mouse_leave);

                if (active) apply(activity::mouse_hover);
                else timer.actify(activity::mouse_leave, ACTIVE_TIMEOUT, apply);
            };
            //SUBMIT(tier::release, hids::events::mouse::move, gear)
            //{
            //	auto apply = [&](auto active)
            //	{
            //		wide = active;
            //		if (AXIS == axis::Y) config(active ? init * 2 // Make vertical scrollbar
            //		                                   : init);   //  wider on hover
            //		base::reflow();
            //		return faux; // One shot call
            //	};
            //
            //	timer.pacify(activity::mouse_leave);
            //	apply(activity::mouse_hover);
            //	timer.template actify<activity::mouse_leave>(ACTIVE_TIMEOUT, apply);
            //};
            SUBMIT(tier::release, e2::render::any, parent_canvas)
            {
                auto region = parent_canvas.view();
                auto object = parent_canvas.full();
                auto handle = region;

                calc.commit(handle);

                auto& handle_len = handle.size[AXIS];
                auto& region_len = region.size[AXIS];
                auto& object_len = object.size[AXIS];

                handle = region.clip(handle);
                handle_len = std::max(1, handle_len);

                if (object_len && handle_len != region_len) // Show only if it is oversized.
                {
                    //parent_canvas.fill(handle, [](cell& c) { c.und(!c.und()); });
                    parent_canvas.fill(handle, [](cell& c) { c.und(true); });
                }
            };
        }
    };

    // controls: Container with margins (outer space) and padding (inner space).
    class pads
        : public form<pads>
    {
        dent padding; // pads: Space around an element's content, outside of any defined borders. It does not affect the size, only affects the fill. Used in base::renderproc only.
        dent margins; // pads: Space around an element's content, inside of any defined borders. Containers take this parameter into account when calculating sizes. Used in all conainers.
        sptr client;

    public:

        ~pads()
        {
            if (client)
            {
                auto empty = decltype(e2::form::upon::vtree::detached)::type{};
                auto item_ptr = client;
                client.reset();
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, empty);
            }
        }
        pads(dent const& padding_value = {}, dent const& margins_value = {})
            : padding{ padding_value },
              margins{ margins_value }
        {
            SUBMIT(tier::preview, e2::size::set, new_size)
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
            SUBMIT(tier::release, e2::size::set, new_size)
            {
                if (client)
                {
                    auto client_size = new_size - padding;
                    auto client_coor = padding.corner();
                    client->SIGNAL(tier::release, e2::size::set, client_size);
                    client->SIGNAL(tier::release, e2::coor::set, client_coor);
                }
            };
            SUBMIT(tier::release, e2::render::prerender, parent_canvas)
            {
                auto view = parent_canvas.view();
                parent_canvas.view(view + margins);
                this->SIGNAL(tier::release, e2::render::any, parent_canvas);
                parent_canvas.view(view);
                if (client)
                    parent_canvas.render(client, base::coor());
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

        static constexpr view dots = "…";
        para name;
        bool flex; // item: Violate or not the label size, default is faux.
        bool test; // item: Place or not(default) the Two Dot Leader when there is not enough space.

        void recalc()
        {
            auto size = name.size();
            auto lims = flex ? twod{ -1,size.y } : size;
            limit.set(lims, lims);
            base::resize(size);
        }
    public:
        item(para const& label_para, bool flexible = faux, bool check_size = faux)
            : name{ label_para },
              flex{ flexible   },
              test{ check_size }
        {
            recalc();
            SUBMIT(tier::release, e2::render::any, parent_canvas)
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
            };
        }
        item(text const& label_text, bool flexible = faux, bool check_size = faux)
            : item(para{ label_text }, flexible, check_size)
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
        iota sfx_len;
        text pin_str;
        iota cur_val;
        twod box_len;

        enum
        {
            txt_id,
            pin_id,
        };

        void set_pen(uint8_t hilight)
        {
            auto& pen = canvas.mark().bga(hilight);
        }
        void recalc()
        {
            text cur_str = std::to_string(cur_val);
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
        auto set_val(iota new_val, view pin_chr)
        {
            cur_val = new_val;
            pin_str = pin_chr;
            recalc();
            return box_len;
        }

        stem_rate_grip(view sfx_string)
            : sfx_str{ sfx_string }, canvas{*(coreface = std::make_shared<face>())}
        {
            //todo cache specific
            canvas.link(bell::id);
            SUBMIT(tier::release, e2::size::set, new_sz) { canvas.size(new_sz); };
            SUBMIT(tier::release, e2::coor::set, new_xy) { canvas.move(new_xy); };
            SUBMIT(tier::request, e2::form::canvas, canvas) { canvas = coreface; };

            sfx_len = utf::length(sfx_str);

            canvas.mark().bgc(whitelt);
            topic = ansi::idx(txt_id).nop().eol()
                         .idx(pin_id).nop();

            set_pen(0);

            SUBMIT(tier::preview, e2::size::set, size)
            {
                size = box_len; // Suppress resize.
            };
            SUBMIT(tier::release, e2::form::state::mouse, active)
            {
                set_pen(active ? 80 : 0);
                recalc();
            };
            SUBMIT(tier::release, e2::render::any, parent_canvas)
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

    template<tier TIER, class EVENT>
    class stem_rate
        : public form<stem_rate<TIER, EVENT>>
    {
        pro::robot robot{*this }; // stem_rate: Animation controller.
        pro::limit limit{*this }; // stem_rate: Size limits.

        //todo cache specific
        sptr<face> coreface;
        face& canvas;

        using tail = netxs::datetime::tail<iota>;

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
        iota pin_pos = 0;
        iota bar_len = 0;
        iota cur_val = 0;
        iota min_val = 0;
        iota max_val = 0;
        iota origin = 0;
        iota deltas = 0;

        //todo unify mint = 1/fps60 = 16ms
        //seems that the 4ms is enough, no need to bind with fps (opened question)
        tail bygone{ 75ms, 4ms };

        text grip_suffix;
        text label_text;
        iota pad = 5;
        iota bgclr = 4;

        void recalc()
        {
            bar_len = std::max(0, base::size().x - (pad + 1) * 2);
            auto pin_abs = netxs::divround((bar_len + 1) * (cur_val - min_val),
                (max_val - min_val));
            text pin_str;
            if (pin_abs == 0)           pin_str = "├";
            else if (pin_abs == bar_len + 1) pin_str = "┤";
            else                             pin_str = "┼";

            pin_len = grip_ctl->set_val(cur_val, pin_str);
            pin_pos = pad + pin_abs - pin_len.x / 2;
            topic[bar_id] = "└" + utf::repeat("─", bar_len) + "┘";
            topic[bar_id].locus.kill().chx(pad);
        }

        iota next_val(iota delta)
        {
            auto dm = max_val - min_val;
            auto p = divround((bar_len + 1) * (origin - min_val), dm);
            auto c = divround((p + delta) * dm, (bar_len + 1)) + min_val;
            bygone.set(c - cur_val);
            return c;
        }

        bool _move_grip(iota new_val)
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
        void move_grip(iota new_val)
        {
            if (_move_grip(new_val))
            {
                SIGNAL(TIER, EVENT{}, cur_val);
            }
        }
        void giveup(hids& gear)
        {
            if (gear.captured(grip_ctl->id))
            {
                deltas = 0;
                move_grip(origin);
                gear.release();
                gear.dismiss();
            }
        }

        stem_rate(text const& caption, iota min_value, iota max_value, view suffix)
            : min_val{ min_value },
              max_val{ max_value },
              grip_suffix{ suffix },
              canvas{*(coreface = std::make_shared<face>())}
        {
            //todo cache specific
            canvas.link(bell::id);
            SUBMIT(tier::release, e2::size::set, new_sz) { canvas.size(new_sz); };
            SUBMIT(tier::release, e2::coor::set, new_xy) { canvas.move(new_xy); };
            SUBMIT(tier::request, e2::form::canvas, canvas) { canvas = coreface; };

            cur_val = -1;
            SIGNAL(TIER, EVENT{}, cur_val);

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

            SUBMIT(tier::general, e2::form::global::lucidity, alpha)
            {
                if (alpha >= 0 && alpha < 256)
                {
                    canvas.mark().alpha(alpha);
                    base::deface();
                }
            };
            SUBMIT(TIER, EVENT{}, cur_val)
            {
                if (cur_val >= min_val)
                {
                    _move_grip(cur_val);
                }
            };
            SUBMIT(tier::release, e2::form::upon::vtree::attached, parent)
            {
                grip_ctl = base::create<stem_rate_grip>(grip_suffix);
                grip_ctl->SIGNAL(tier::release, e2::form::upon::vtree::attached, base::This());

                grip_ctl->SUBMIT(tier::release, hids::events::mouse::button::drag::start::left, gear)
                {
                    if (gear.capture(grip_ctl->id))
                    {
                        origin = cur_val;
                        gear.dismiss();
                    }
                };
                grip_ctl->SUBMIT(tier::release, hids::events::mouse::button::drag::pull::left, gear)
                {
                    if (gear.captured(grip_ctl->id))
                    {
                        deltas += gear.mouse::delta.get().x;
                        move_grip(next_val(deltas));
                        gear.dismiss();
                    }
                };
                grip_ctl->SUBMIT(tier::release, hids::events::mouse::button::drag::cancel::left, gear)
                {
                    giveup(gear);
                };
                grip_ctl->SUBMIT(tier::general, hids::events::die, gear)
                {
                    giveup(gear);
                };
                grip_ctl->SUBMIT(tier::release, hids::events::mouse::button::drag::stop::left, gear)
                {
                    if (gear.captured(grip_ctl->id))
                    {
                        deltas = 0;
                        gear.release();
                        base::deface();
                        robot.actify(bygone.fader<quadratic<iota>>(750ms), [&](auto& delta)
                            {
                                move_grip(cur_val + delta);
                            });
                        gear.dismiss();
                    }
                };
                grip_ctl->SUBMIT(tier::release, hids::events::mouse::scroll::up, gear)
                {
                    move_grip(cur_val - 1);
                    gear.dismiss();
                };
                grip_ctl->SUBMIT(tier::release, hids::events::mouse::scroll::down, gear)
                {
                    move_grip(cur_val + 1);
                    gear.dismiss();
                };
                recalc();
            };
            SUBMIT(tier::release, e2::size::set, size)
            {
                recalc();
            };
            SUBMIT(tier::release, hids::events::mouse::button::click::right, gear)
            {
                base::color(canvas.mark().fgc(), (tint)((++bgclr) % 16));
                base::deface();
                gear.dismiss();
            };
            SUBMIT(tier::release, hids::events::mouse::scroll::up, gear)
            {
                move_grip(cur_val - 10);
                gear.dismiss();
            };
            SUBMIT(tier::release, hids::events::mouse::scroll::down, gear)
            {
                move_grip(cur_val + 10);
                gear.dismiss();
            };
            SUBMIT(tier::release, e2::render::any, parent_canvas)
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

#endif // NETXS_CONTROLS_HPP