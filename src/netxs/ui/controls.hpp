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
    using namespace netxs;
    using namespace netxs::events;
    using namespace netxs::console;
    using namespace netxs::ui::atoms;

    enum slot : id_t { _1, _2 };
    enum axis : id_t { X, Y };
    enum axes
    {
        NONE   = 0,
        ONLY_X = 1 << 0,
        ONLY_Y = 1 << 1,
        ALL    = (ONLY_X | ONLY_Y),
    };

    // controls: base UI control.
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
        pro::mouse mouse{ *this }; // form: Mouse controller.
        pro::keybd keybd{ *this }; // form: Keybd controller.

        form()
        {
            if constexpr (has<T>::remove)
            {
                SUBMIT(e2::preview, e2::form::proceed::detach, shadow)
                {
                    This<T>()->T::remove(shadow);
                };
            }
        }
        // form: Attach feature and return itself.
        template<class S, class ...Args>
        auto plugin(Args&&... args)
        {
            auto backup = This<T>();
            depo[std::type_index(typeid(S))] = std::make_unique<S>(*this, std::forward<Args>(args)...);
            base::reflow();
            return backup;
        }
        // form: Set colors and return itself.
        template<class ...Args>
        auto colors(Args&&... args)
        {
            base::color(std::forward<Args>(args)...);
            return This<T>();
        }
        // form: Set the form visible for mouse.
        auto active(bool visible = true)
        {
            auto brush = base::color();
            if (!brush.wdt()) base::color(brush.txt(whitespace));
            return This<T>();
        }
        // form: Return plugin reference of specified type.
        template<class S>
        auto& plugins()
        {
            auto ptr = static_cast<S*>(depo[std::type_index(typeid(S))].get());
            return *ptr;
        }
        // form: Invoke arbitrary functor(itself/*This/boss).
        template<class P>
        auto invoke(P functor)
        {
            auto backup = This<T>();
            functor(*backup);
            return backup;
        }
        // form: Attach homeless branch and return itself.
        template<class C, class ...Args>
        auto branch(C child, Args&&... args)
        {
            auto backup = This<T>();
            if (child) backup->T::attach(child, std::forward<Args>(args)...);
            return backup;
        }
        // form: UI-control will be detached when the master is detached.
        auto depend(sptr<base> master_ptr)
        {
            auto& master = *master_ptr;
            master.SUBMIT_T(e2::release, e2::form::upon::vtree::detached, memomap[master.id], parent_ptr)
            {
                auto backup = This<T>();
                memomap.erase(master.id);
                if (memomap.empty()) base::detach();
            };
            return This<T>();
        }
        // form: UI-control will be detached when the last item of collection is detached.
        template<class S>
        auto depend_on_collection(S data_collection_src)
        {
            auto backup = This<T>();
            for(auto& data_src : data_collection_src)
            {
                depend(data_src);
            }
            return backup;
        }
        // form: Create and attach a new item using a template and dynamic datasource.
        template<e2::type PROPERTY, class C, class P>
        auto attach_element(C& data_src_sptr, P item_template)
        {
            auto backup = This<T>();
            ARGTYPE(PROPERTY) arg_value;
            data_src_sptr->SIGNAL(e2::request, PROPERTY, arg_value);
            auto new_item = item_template(data_src_sptr, arg_value)
                                 ->depend(data_src_sptr);
            auto item_shadow = ptr::shadow(new_item);
            auto data_shadow = ptr::shadow(data_src_sptr);
            auto boss_shadow = ptr::shadow(backup);
            data_src_sptr->SUBMIT_BYVAL_T(e2::release, PROPERTY, memomap[data_src_sptr->id], arg_new_value)
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
        template<e2::type PROPERTY, class S, class P>
        auto attach_collection(S& data_collection_src, P item_template)
        {
            auto backup = This<T>();
            for(auto& data_src_sptr : data_collection_src)
            {
                attach_element<PROPERTY>(data_src_sptr, item_template);
            }
            return backup;
        }
    };

    // controls: Splitter control.
    class fork
        : public form<fork>
    {
        enum action { seize, drag, release };

        static constexpr iota MAX_RATIO = 0xFFFF;
        static constexpr iota HALF_RATIO = 0xFFFF >> 1;

        sptr<base> client_1; // fork: 1st object.
        sptr<base> client_2; // fork: 2nd object.

        twod size1;
        twod size2;
        twod coor2;

        tint clr;
        //twod size;
        rect stem;
        iota start;
        iota width;
        iota ratio;
        iota maxpos;
        bool updown;
        bool movable;

        twod xpose(twod const& pt) { return updown ? twod{ pt.y, pt.x } : pt; }
        iota get_x(twod const& pt) { return updown ? pt.y : pt.x; }
        iota get_y(twod const& pt) { return updown ? pt.x : pt.y; }

        void _config(axis alignment, iota thickness, iota scale)
        {
            switch (alignment)
            {
                case axis::X:
                    updown = faux;
                    break;
                case axis::Y:
                    updown = true;
                    break;
                default:
                    updown = faux;
                    break;
            }
            width = std::max(thickness, 0);
            ratio = MAX_RATIO * std::clamp(scale, 0, 100) / 100;
            base::reflow();
        }

    public:
        auto get_ratio()
        {
            return ratio;
        }
        auto config(iota scale)
        {
            ratio = MAX_RATIO * std::clamp(scale, 0, 100) / 100;
            base::reflow();
        }
        auto config(axis alignment, iota thickness, iota scale)
        {
            _config(alignment, thickness, scale);
            return This<fork>();
        }

        ~fork()
        {
            e2::sync lock;
            if (client_1) client_1->base::detach();
            if (client_2) client_2->base::detach();
        }
        fork(axis alignment = axis::X, iota thickness = 0, iota scale = 50)
        :   maxpos{ 0 },
            start{ 0 },
            width{ 0 },
            movable{ true },
            updown{ faux },
            ratio{ 0xFFFF >> 1 }
        {
            SUBMIT(e2::preview, e2::size::set, new_size)
            {
                fork::size_preview(new_size);
            };
            SUBMIT(e2::release, e2::size::set, new_size)
            {
                //size = new_size;
                if (client_1)
                {
                    client_1->SIGNAL(e2::release, e2::size::set, size1);
                }
                if (client_2)
                {
                    client_2->SIGNAL(e2::release, e2::coor::set, coor2);
                    client_2->SIGNAL(e2::release, e2::size::set, size2);
                }
            };
            SUBMIT(e2::release, e2::render::any, parent_canvas)
            {
                auto& basis = base::coor();
                if (client_1) parent_canvas.render(client_1, basis);
                if (client_2) parent_canvas.render(client_2, basis);

                if (width)
                {
                    //todo draw grips
                }
            };

            _config(alignment, thickness, scale);
            //  case WM_SIZE:
            //  	entity->resize({ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
            //  	return 0;
            //  case WM_PAINT:
            //  	entity->draw();
            //  	return 0;
            //  case WM_SETCURSOR:
            //  	if (LOWORD(lParam) == HTCLIENT)
            //  	{
            //  		return entity->showcursor();
            //  	}
            //  	return 0;
            //  case WM_APP_LIMITSCHANGED:
            //  	entity->calclimits();
            //  	return 0;
            //  case WM_APP_CTXUPDATE:
            //  	entity->forward(WM_APP_CTXUPDATE);
            //  	return 0;
            //  case WM_APP_CONFIG:
            //  	entity->config((gui::orientation)wParam, LOWORD(lParam), HIWORD(lParam));
            //  	return 0;
            //  case WM_LBUTTONDBLCLK:
            //  	entity->toggle();
            //  	return 0;
            //  case WM_RBUTTONUP:
            //  	entity->rotate();
            //  	return 0;
            //  case WM_LBUTTONDOWN:
            //  	entity->slider(action::seize, { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
            //  	return 0;
            //  case WM_LBUTTONUP:
            //  	entity->slider(action::release, { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
            //  	return 0;
            //  case WM_MOUSEMOVE:
            //  	if (wParam & MK_LBUTTON)
            //  	{
            //  		entity->slider(action::drag, { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) });
            //  	}
            //  	return 0;
        }
        void rotate()
        {
            updown = !updown;
            //base::resize(size);
            base::reflow();

            //fork::resize(size);
            //takecursor();
        }
        void toggle()
        {
            if (movable)
            {
                switch (ratio)
                {
                    case 0:
                        ratio = HALF_RATIO;
                        break;
                    case MAX_RATIO:
                        ratio = HALF_RATIO + 1;
                        break;
                    default:
                        ratio = ratio > HALF_RATIO ? MAX_RATIO : 0;
                        break;
                }
                base::reflow();
            }
        }
        // fork: .
        void size_preview(twod& new_size)
        {
            //todo revise/unify
            auto new_size0 = xpose(new_size);
            {
                maxpos = std::max(new_size0.x - width, 0);
                auto split = netxs::divround(maxpos * ratio, MAX_RATIO);

                size1 = xpose({ split, new_size0.y });
                if (client_1)
                {
                    auto& item = *client_1;
                    item.SIGNAL(e2::preview, e2::size::set, size1);

                    split = get_x(size1);
                    new_size0.y = get_y(size1);
                }

                size2 = xpose({ maxpos - split, new_size0.y });
                auto test_size2 = size2;
                if (client_2)
                {
                    auto& item = *client_2;
                    item.SIGNAL(e2::preview, e2::size::set, size2);
                    split = new_size0.x - width - get_x(size2);

                    if (test_size2 != size2) // If size2 doesn't fit.
                    {
                        new_size0.y = get_y(size2);
                        size1 = xpose({ split, new_size0.y });
                        if (client_1)
                        {
                            auto& item = *client_1;
                            item.SIGNAL(e2::preview, e2::size::set, size1);

                            split = get_x(size1);
                            new_size0.y = get_y(size1);
                        }
                        size2 = xpose({ maxpos - split, new_size0.y });
                        if (client_2)
                        {
                            auto& item = *client_2;
                            item.SIGNAL(e2::preview, e2::size::set, size2);
                            new_size0.y = get_y(size2);
                        }
                    }

                    coor2 = xpose({ split + width, 0 });
                }

                new_size = xpose({ split + width + get_x(size2), new_size0.y });
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
        template<slot SLOT, class T>
        auto attach(sptr<T> item)
        {
            if (SLOT == slot::_1) client_1 = item;
            else                  client_2 = item;
            item->SIGNAL(e2::release, e2::form::upon::vtree::attached, This());
            return item;
        }
        template<slot SLOT, class T, class ...Args>
        // fork: Create a new item of the specified subtype and attach it to a specified slot.
        auto attach(Args&&... args)
        {
            return attach<SLOT>(create<T>(std::forward<Args>(args)...));
        }
        // fork: Remove nested object by it's ptr.
        void remove(sptr<base> item_ptr)
        {
            if (client_1 == item_ptr ? (client_1.reset(), true) :
                client_2 == item_ptr ? (client_2.reset(), true) : faux)
            {
                auto backup = This();
                item_ptr->SIGNAL(e2::release, e2::form::upon::vtree::detached, backup);
            }
        }
    };

    // controls: Vertical/horizontal list control.
    class list
        : public form<list>
    {
        using roll = std::list<std::pair<sptr<base>, iota>>;
        roll subset;
        bool updown; // list: List orientation, true: vertical(default), faux: horizontal.

    public:
        ~list()
        {
            e2::sync lock;
            while (subset.size())
            {
                subset.back().first->base::detach();
                subset.pop_back();
            }
        }
        list(axis orientation = axis::Y)
            : updown{ orientation == axis::Y }
        {
            SUBMIT(e2::preview, e2::size::set, new_sz)
            {
                iota  height;
                auto& y_size = updown ? new_sz.y : new_sz.x;
                auto& x_size = updown ? new_sz.x : new_sz.y;
                auto  x_temp = x_size;
                auto  y_temp = y_size;

                auto meter = [&]() {
                    height = 0;
                    for (auto& client : subset)
                    {
                        y_size = 0;
                        client.first->SIGNAL(e2::preview, e2::size::set, new_sz);
                        client.second = y_size;
                        height += y_size;
                    }
                };
                meter(); if (x_temp != x_size) meter();
                y_size = height;
            };
            SUBMIT(e2::release, e2::size::set, new_sz)
            {
                //todo optimize avoid SIGNAL if size/coor is unchanged
                auto& y_size = updown ? new_sz.y : new_sz.x;
                auto& x_size = updown ? new_sz.x : new_sz.y;
                twod  new_xy;
                auto& y_coor = updown ? new_xy.y : new_xy.x;
                auto& x_coor = updown ? new_xy.x : new_xy.y;

                auto  found = faux;
                for (auto& client : subset)
                {
                    y_size = client.second;
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

                        entry.SIGNAL(e2::release, e2::coor::set, new_xy);
                        entry.SIGNAL(e2::release, e2::size::set, new_sz);
                    }
                    y_coor+= client.second;
                }
            };
            SUBMIT(e2::release, e2::render::any, parent_canvas)
            {
                auto& basis = base::coor();
                for (auto& client : subset)
                {
                    parent_canvas.render(client.first, basis);
                }
            };
        }
        // list: Attach specified item.
        template<class T>
        auto attach(sptr<T> item)
        {
            subset.push_back({ item, 0 });
            item->SIGNAL(e2::release, e2::form::upon::vtree::attached, This());
            return item;
        }
        // list: Create a new item of the specified subtype and attach it.
        template<class T, class ...Args>
        auto attach(Args&&... args)
        {
            return attach(create<T>(std::forward<Args>(args)...));
        }
        // list: Remove nested object.
        void remove(sptr<base> item_ptr)
        {
            auto head = subset.begin();
            auto tail = subset.end();
            auto item = std::find_if(head, tail, [&](auto& c){ return c.first == item_ptr; });
            if (item != tail)
            {
                auto backup = This();
                subset.erase(item);
                item_ptr->SIGNAL(e2::release, e2::form::upon::vtree::detached, backup);
            }
        }
        // list: Update nested object.
        template<class T, class S>
        void update(T old_item_ptr, S new_item_ptr)
        {
            auto head = subset.begin();
            auto tail = subset.end();
            auto item = std::find_if(head, tail, [&](auto& c){ return c.first == old_item_ptr; });
            if (item != tail)
            {
                auto backup = This();
                auto pos = subset.erase(item);
                old_item_ptr->SIGNAL(e2::release, e2::form::upon::vtree::detached, backup);
                subset.insert(pos, std::pair{ new_item_ptr, 0 });
                new_item_ptr->SIGNAL(e2::release, e2::form::upon::vtree::attached, backup);
            }
        }
    };

    // controls: (puff) Layered cake of forms on top of each other.
    class cake
        : public form<cake>
    {
        std::list<sptr<base>> subset;

    public:
        ~cake()
        {
            e2::sync lock;
            while (subset.size())
            {
                subset.back()->base::detach();
                subset.pop_back();
            }
        }
        cake()
        {
            SUBMIT(e2::preview, e2::size::set, newsz)
            {
                for (auto& client : subset)
                {
                    if (client)
                        client->SIGNAL(e2::preview, e2::size::set, newsz);
                }
            };
            SUBMIT(e2::release, e2::size::set, newsz)
            {
                for (auto& client : subset)
                {
                    if (client)
                        client->SIGNAL(e2::release, e2::size::set, newsz);
                }
            };
            SUBMIT(e2::release, e2::render::any, parent_canvas)
            {
                auto& basis = base::coor();
                for (auto& client : subset)
                {
                    parent_canvas.render(client, basis);
                }
            };
        }
        // cake: Create a new item of the specified subtype and attach it.
        template<class T>
        auto attach(sptr<T> item)
        {
            subset.push_back(item);
            item->SIGNAL(e2::release, e2::form::upon::vtree::attached, This());
            return item;
        }
        // cake: Create a new item of the specified subtype and attach it.
        template<class T, class ...Args>
        auto attach(Args&&... args)
        {
            return attach(create<T>(std::forward<Args>(args)...));
        }
        // cake: Remove nested object.
        void remove(sptr<base> item_ptr)
        {
            auto head = subset.begin();
            auto tail = subset.end();
            auto item = std::find_if(head, tail, [&](auto& c){ return c == item_ptr; });
            if (item != tail)
            {
                auto backup = This();
                subset.erase(item);
                item_ptr->SIGNAL(e2::release, e2::form::upon::vtree::detached, backup);
            }
        }
    };

    struct page_layout
    {
        struct item
        {
            iota id;
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
        page_layout layout;
        bool beyond; // post: Allow vertical scrolling beyond last line.

    public:
        page topic; // post: Text content.

        template<class T>
        auto& lyric(T paraid) { return *topic[paraid].lyric; }
        template<class T>
        auto& content(T paraid) { return topic[paraid]; }

        // post: Set content.
        template<class TEXT>
        auto upload(TEXT utf8)
        {
            topic = utf8;
            base::reflow();
            return This<post>();
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
            base::oversize.set(-std::min(0, cover.l),
                                std::max(0, cover.r - width.x + 1),
                               -std::min(0, cover.t),
                                0);
            width.y = cover.height() + (beyond ? width.y : 1); //todo unify (text editor)
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
            SUBMIT(e2::preview, e2::size::set, size)
            {
                recalc(size);
                size.y = width.y;
            };
            SUBMIT(e2::release, e2::size::set, size)
            {
                //if (width != size)
                //{
                //	recalc(size);
                //	//width.y = size.y;
                //}
                width = size;
            };
            SUBMIT(e2::release, e2::render::any, parent_canvas)
            {
                output(parent_canvas);

                //auto mark = rect{ base::anchor + base::coor(), {10,5} };
                //mark.coor += parent_canvas.view().coor; // Set client's basis
                //parent_canvas.fill(mark, [](cell& c) { c.alpha(0x80).bgc().chan.r = 0xff; });
            };
        }
    };

    // controls: Scroller.
    class rail
        : public form<rail>
    {
        pro::robot robot{*this }; // rail: Animation controller.

        static constexpr hint events[] = { e2::form::upon::scroll::x,
                                           e2::form::upon::scroll::y,
                                           e2::form::upon::scroll::resetx,
                                           e2::form::upon::scroll::resety };
        bool strict[2] = { true, true }; // rail: Don't allow overscroll.
        bool manual[2] = { true, true }; // rail: Manaul scrolling (no auto align).
        bool locked{}; // rail: Client is under resizing.
        subs tokens{}; // rail: Subscriptions on client moveto and resize.
        subs fasten{}; // rail: Subscriptions on masters to follow they state.
        rack scinfo{}; // rail: Scroll info.
        axes permit{}; // rail: Allowed axes to scroll.
        axes siezed{}; // rail: Allowed axes to capture.

        iota speed{ SPD  }; // rail: Text auto-scroll initial speed component ΔR.
        iota pulse{ PLS  }; // rail: Text auto-scroll initial speed component ΔT.
        iota cycle{ CCL  }; // rail: Text auto-scroll duration in ms.
        bool steer{ faux }; // rail: Text scroll vertical direction.

        sptr<base> client; // rail: Client instance.

    public:
        bool overscroll[2] = { true, true }; // rail: Allow overscroll with auto correct.
        auto config(bool allow_x_overscroll = true, bool allow_y_overscroll = true)
        {
            overscroll[axis::X] = allow_x_overscroll;
            overscroll[axis::Y] = allow_y_overscroll;
            return This<rail>();
        }
        template<axis AXIS>
        auto moveby(iota coor)
        {
            AXIS == axis::X ? scroll<X>(coor)
                            : scroll<Y>(coor);
            return This<rail>();
        }
        template<axis AXIS>
        auto follow(sptr<base> master = {})
        {
            if (master) master->SUBMIT_T(e2::release, events[AXIS], fasten, master_scinfo)
            {
                AXIS == axis::X ? scroll<X>(scinfo.window.coor.x - master_scinfo.window.coor.x)
                                : scroll<Y>(scinfo.window.coor.y - master_scinfo.window.coor.y);
            };
            else fasten.clear();

            return This<rail>();
        }
        //todo should we detach client in dtor?
        //~rail...

        rail(axes allow_to_scroll = axes::ALL, axes allow_to_capture = axes::ALL)
            : permit{ allow_to_scroll  },
              siezed{ allow_to_capture }
        {
            // Receive scroll parameters from external source.
            SUBMIT(e2::preview, e2::form::upon::scroll::any, info)
            {
                if (client)
                {
                    auto& item = *client;
                    switch (this->bell::protos<e2::preview>())
                    {
                        case events[X]:
                            scroll<X>(scinfo.window.coor.x - info.window.coor.x);
                            break;
                        case events[Y]:
                            scroll<Y>(scinfo.window.coor.y - info.window.coor.y);
                            break;
                        case events[X + 2]:
                            cancel<X, true>();
                            break;
                        case events[Y + 2]:
                            cancel<Y, true>();
                            break;
                    }
                }
            };
            SUBMIT(e2::request, e2::form::upon::scroll::any, req_scinfo)
            {
                req_scinfo = scinfo;
            };

            SUBMIT(e2::release, e2::size::set, new_size)
            {
                if (client)
                {
                    locked = true; // See the details in subscription at the attach().
                    auto delta = client->base::resize(new_size, base::anchor);
                    locked = faux;
                    scroll<X>(delta.x);
                    scroll<Y>(delta.y);
                }
            };

            using bttn = e2::hids::mouse::button;
            SUBMIT(e2::release, e2::hids::mouse::scroll::any, gear)
            {
                auto dir = gear.whldt > 0;
                if (permit == axes::ONLY_X || gear.meta(hids::ANYCTRL |
                                                        hids::SHIFT ))
                     wheels<X>(dir);
                else wheels<Y>(dir);

                gear.dismiss();
            };
            SUBMIT(e2::release, bttn::drag::start::right, gear)
            {
                auto ds = gear.delta.get();
                auto dx = ds.x;
                auto dy = ds.y * 2;
                auto vt = std::abs(dx) < std::abs(dy);

                if (((siezed & axes::ONLY_X) && !vt) ||
                    ((siezed & axes::ONLY_Y) &&  vt))
                {
                    if (gear.capture(bell::id))
                    {
                        manual[X] = true;
                        manual[Y] = true;
                        strict[X] = !overscroll[X];
                        strict[Y] = !overscroll[Y];
                        gear.dismiss();
                    }
                }
            };
            SUBMIT(e2::release, bttn::drag::pull::right, gear)
            {
                if (gear.captured(bell::id))
                {
                    auto delta = gear.mouse::delta.get();
                    if (permit & axes::ONLY_X) scroll<X>(delta.x);
                    if (permit & axes::ONLY_Y) scroll<Y>(delta.y);
                    gear.dismiss();
                }
            };
            SUBMIT(e2::release, bttn::drag::cancel::right, gear)
            {
                if (gear.captured(bell::id))
                {
                    giveup(gear);
                }
            };
            SUBMIT(e2::general, e2::hids::mouse::gone, gear)
            {
                if (gear.captured(bell::id))
                {
                    giveup(gear);
                }
            };
            SUBMIT(e2::release, bttn::drag::stop::right, gear)
            {
                if (gear.captured(bell::id))
                {
                    auto  v0 = gear.delta.avg();
                    auto& speed = v0.dS;
                    auto  start = datetime::round<iota>(v0.t0);
                    auto  cycle = datetime::round<iota>(v0.dT);
                    auto  limit = datetime::round<iota>(STOPPING_TIME);

                    if (permit & axes::ONLY_X) actify<X>(quadratic{ speed.x, cycle, limit, start });
                    if (permit & axes::ONLY_Y) actify<Y>(quadratic{ speed.y, cycle, limit, start });

                    base::deface();
                    gear.release();
                    gear.dismiss();
                }
            };
            SUBMIT(e2::release, bttn::down::any, gear)
            {
                if (manual[X]) robot.pacify(X);
                if (manual[Y]) robot.pacify(Y);
            };
            SUBMIT(e2::release, bttn::click::right, gear)
            {
                if (!gear.captured(bell::id))
                {
                    if (manual[X]) cancel<X, true>();
                    if (manual[Y]) cancel<Y, true>();
                    gear.dismiss();
                }
            };
            SUBMIT(e2::release, e2::form::animate::stop, id)
            {
                switch (id)
                {
                    case X:
                        manual[X] = true;
                        scroll<X>();
                        break;
                    case Y:
                        manual[Y] = true;
                        scroll<Y>();
                        break;
                    default:
                        break;
                }
                deface();
            };
            SUBMIT(e2::release, e2::render::any, parent_canvas)
            {
                if (client)
                    parent_canvas.render<faux>(client, base::coor());
            };
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
            }
            scroll<AXIS>(dir ? 1 : -1);
            keepon<AXIS>(quadratic<iota>(dir ? speed : -speed, pulse, cycle, now<iota>()));
        }
        template<axis AXIS, class FX>
        void keepon(FX&& func)
        {
            strict[AXIS] = true;
            robot.actify(AXIS, func, [&](auto& p)
                {
                    scroll<AXIS>(p);
                });
        }
        template<axis AXIS, class FX>
        void actify(FX&& func)
        {
            auto inside = scroll<AXIS>();
            if  (inside)  keepon<AXIS>(func);
            else          lineup<AXIS>();
        }
        template<axis AXIS, bool FORCED = faux>
        void cancel()
        {
            if constexpr (FORCED) lineup<AXIS>();
            else
            {
                auto inside = scroll<AXIS>();
                if (!inside)  lineup<AXIS>();
            }
        }
        template<axis AXIS>
        void lineup()
        {
            if (client)
            {
                manual[AXIS] = faux;
                auto& block = client->base::area();
                auto  level = AXIS == X;
                auto  coord = level ? block.coor.x
                                    : block.coor.y;
                auto  bound = level ? std::min(base::size().x - block.size.x, 0)
                                    : std::min(base::size().y - block.size.y, 0);
                auto  newxy = std::clamp(coord, bound, 0);
                auto  route = newxy - coord;
                iota  tempo = SWITCHING_TIME;
                auto  fader = constlinearAtoB<iota>(route, tempo, now<iota>());
                keepon<AXIS>(fader);
            }
        }
        template<axis AXIS>
        bool scroll(iota delta = {})
        {
            bool inside = true;

            if (client)
            {
                auto& thing = *client;
                auto  block = thing.base::area();
                auto  basis = thing.oversize.topleft();
                block.coor -= basis; // Scroll origin basis.
                block.size += thing.oversize.summ();
                auto& frame = base::size();
                auto  level = AXIS == X;
                auto  bound = level ? std::min(frame.x - block.size.x, 0)
                                    : std::min(frame.y - block.size.y, 0);
                auto& coord = level ? block.coor.x
                                    : block.coor.y;
                coord += delta;

                if (manual[AXIS]) // Check overscroll if no auto correction.
                {
                    auto clamp = std::clamp(coord, bound, 0);
                    inside = clamp == coord;
                    if (!inside && strict[AXIS]) // If outside the scroll limits
                    {                            // and overscroll is not allowed.
                            coord = clamp;
                    }
                }

                scinfo.beyond = thing.oversize;  // Oversize value.
                scinfo.region = block.size;
                scinfo.window.coor =-block.coor; // Viewport.
                scinfo.window.size = frame;      //
                SIGNAL(e2::release, events[AXIS], scinfo);

                block.coor += basis; // Client origin basis.
                locked = true;
                thing.base::moveto(block.coor);
                locked = faux;
                deface();
            }

            return inside;
        }
        // rail: Attach specified item.
        template<class T>
        auto attach(sptr<T> item)
        {
            client = item;
            tokens.clear();
            item->SUBMIT_T(e2::release, e2::size::set, tokens.extra(), size)
            {
                if (!locked)
                {
                    scroll<X>();
                    scroll<Y>();
                }
            };
            item->SUBMIT_T(e2::release, e2::form::upon::vtree::detached, tokens.extra(), p)
            {
                scinfo.region = {};
                scinfo.window.coor = {};
                this->SIGNAL(e2::release, events[axis::X], scinfo);
                this->SIGNAL(e2::release, events[axis::Y], scinfo);
                tokens.clear();
                fasten.clear();
            };
            item->SIGNAL(e2::release, e2::form::upon::vtree::attached, This());
            return item;
        }
        // rail: Create a new item of the specified subtype and attach it.
        template<class T, class ...Args>
        auto attach(Args&&... args)
        {
            return attach(create<T>(std::forward<Args>(args)...));
        }
        //template<class T>
        void remove(sptr<base> item_ptr)
        {
            if (client == item_ptr)
            {
                auto backup = This();
                client.reset();
                item_ptr->SIGNAL(e2::release, e2::form::upon::vtree::detached, backup);
            }
        }
        // rail: Update nested object.
        template<class T, class S>
        void update(T old_item_ptr, S new_item_ptr)
        {
            auto backup = This();
            client = new_item_ptr;
            old_item_ptr->SIGNAL(e2::release, e2::form::upon::vtree::detached, backup);
            new_item_ptr->SIGNAL(e2::release, e2::form::upon::vtree::attached, backup);
        }
    };

    // controls: Scroll bar.
    template<axis AXIS>
    class grip // rename to roll?
        : public base
    {
        pro::mouse mouse{*this }; // grip: Mouse events controller.
        pro::timer timer{*this }; // grip: Minimize by timeout.
        pro::limit limit{*this }; // grip: Size limits.

        using wptr = netxs::wptr<bell>;
        using sptr = netxs::sptr<bell>;

        enum activity
        {
            mouse_leave = 0, // faux
            mouse_hover = 1, // true
            pager_first = 10,
            pager_next  = 11,
        };
        static constexpr hint events[] = { e2::form::upon::scroll::x,
                                           e2::form::upon::scroll::y,
                                           e2::form::upon::scroll::resetx,
                                           e2::form::upon::scroll::resety };
        static inline auto  xy(twod const& p) { return AXIS == axis::X ? p.x : p.y; }
        static inline auto  yx(twod const& p) { return AXIS == axis::Y ? p.x : p.y; }
        static inline auto& xy(twod&       p) { return AXIS == axis::X ? p.x : p.y; }
        static inline auto& yx(twod&       p) { return AXIS == axis::Y ? p.x : p.y; }

        struct math
        {
            rack  master_inf = {};                         // math: Master scroll info.
            iota& master_len = xy(master_inf.region);      // math: Master len.
            iota& master_pos = xy(master_inf.window.coor); // math: Master viewport pos.
            iota& master_box = xy(master_inf.window.size); // math: Master viewport len.
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

                // Reset to extreme positions
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

                // Don't place the grip behind the scrollbar
                if (scroll_pos >= scroll_len) scroll_pos = scroll_len - 1;

                // Extreme positions are always closed last
                s = scroll_len - scroll_box;
                m = master_len - master_box;

                if (scroll_len > 2) // Two-row hight is not suitable for this type of aligning
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
                scroll_len = xy(new_size);
                m_to_s();
            }
            void stepby(iota delta)
            {
                scroll_pos = std::clamp(scroll_pos + delta, 0, s);
                s_to_m();
            }
            void commit(rect& handle)
            {
                xy(handle.coor)+= scroll_pos;
                xy(handle.size) = scroll_box;
            }
            auto inside(iota coor)
            {
                if (coor >= scroll_pos + scroll_box) return 1; // Below the grip.
                if (coor >= scroll_pos)              return 0; // Inside the grip.
                                                     return-1; // Above the grip.
            }
            void pager(iota dir)
            {
                master_pos += master_box * dir;
                m_to_s();
            }
            auto follow()
            {
                auto dir = scroll_len > 2 ? inside(cursor_pos)
                                          : cursor_pos > 0 ? 1 // Don't stop to follow over
                                                           :-1;//    box on small scrollbar.
                if (dir)
                {
                    pager(dir);
                    return true;
                }
                return faux;
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

        template<hint EVENT = events[AXIS]>
        void send()
        {
            if (auto master = this->boss.lock())
            {
                master->SIGNAL(e2::preview, EVENT, calc.master_inf);
            }
        }
        void gohome()
        {
            send<events[AXIS + 2]>();
        }
        void config(iota width)
        {
            thin = width;
            auto lims = twod{ xy({ -1,thin }), yx({ -1,thin }) };
            limit.set(lims, lims);
        }
        void giveup(hids& gear)
        {
            if (on_pager) gear.dismiss();
            else
            {
                if (gear.captured(bell::id))
                {
                    if (this->bell::protos<e2::release>(e2::hids::mouse::button::drag::cancel::right))
                    {
                        gohome();
                    }
                    base::deface();
                    gear.release();
                    gear.dismiss();
                }
            }
        }
        auto pager_repeat()
        {
            if (on_pager && calc.follow())
            {
                send<events[AXIS]>();
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

            boss->SUBMIT_T(e2::release, events[AXIS], memo, scinfo)
            {
                calc.update(scinfo);
                base::deface();
            };

            SUBMIT(e2::release, e2::size::set, new_size)
            {
                calc.resize(new_size);
            };

            using bttn = e2::hids::mouse::button;
            SUBMIT(e2::release, e2::hids::mouse::scroll::any, gear)
            {
                if (gear.whldt)
                {
                    auto dir = gear.whldt < 0 ? 1 : -1;
                    calc.pager(dir);
                    send<events[AXIS]>();
                    gear.dismiss();
                }
            };
            SUBMIT(e2::release, e2::hids::mouse::move, gear)
            {
                calc.cursor_pos = xy(gear.mouse::coord);
            };
            SUBMIT(e2::release, e2::hids::mouse::button::dblclick::left, gear)
            {
                gear.dismiss(); // Do not pass double clicks outside.
            };
            SUBMIT(e2::release, e2::hids::mouse::button::down::any, gear)
            {
                if (!on_pager)
                if (this->bell::protos<e2::release>(bttn::down::left ) ||
                    this->bell::protos<e2::release>(bttn::down::right))
                if (auto dir = calc.inside(xy(gear.mouse::coord)))
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
            SUBMIT(e2::release, e2::hids::mouse::button::up::any, gear)
            {
                if (on_pager && gear.captured(bell::id))
                {
                    if (this->bell::protos<e2::release>(bttn::up::left) ||
                        this->bell::protos<e2::release>(bttn::up::right))
                    {
                        gear.release();
                        gear.dismiss();
                        on_pager = faux;
                        timer.pacify(activity::pager_first);
                        timer.pacify(activity::pager_next);
                    }
                }
            };
            SUBMIT(e2::release, e2::hids::mouse::button::up::right, gear)
            {
                //if (!gear.captured(bell::id)) //todo why?
                {
                    gohome();
                    gear.dismiss();
                }
            };

            SUBMIT(e2::release, e2::hids::mouse::button::drag::start::any, gear)
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
            SUBMIT(e2::release, e2::hids::mouse::button::drag::pull::any, gear)
            {
                if (on_pager) gear.dismiss();
                else
                {
                    if (gear.captured(bell::id))
                    {
                        if (auto delta = xy(gear.mouse::delta.get()))
                        {
                            calc.stepby(delta);
                            send<events[AXIS]>();
                            gear.dismiss();
                        }
                    }
                }
            };
            SUBMIT(e2::release, e2::hids::mouse::button::drag::cancel::any, gear)
            {
                giveup(gear);
            };
            SUBMIT(e2::general, e2::hids::mouse::gone, gear)
            {
                giveup(gear);
            };
            SUBMIT(e2::release, e2::hids::mouse::button::drag::stop::any, gear)
            {
                if (on_pager) gear.dismiss();
                else
                {
                    if (gear.captured(bell::id))
                    {
                        if (this->bell::protos<e2::release>(bttn::drag::stop::right))
                        {
                            gohome();
                        }
                        base::deface();
                        gear.release();
                        gear.dismiss();
                    }
                }
            };
            SUBMIT(e2::release, e2::form::state::mouse, active)
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
            //SUBMIT(e2::release, e2::hids::mouse::move, gear)
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
            SUBMIT(e2::release, e2::render::any, parent_canvas)
            {
                auto region = parent_canvas.view();
                auto handle = region;

                calc.commit(handle);

                auto& handle_len = xy(handle.size);
                auto& region_len = xy(region.size);

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

    // controls: Container with margins (outer space) and padding (inner space).
    class pads
        : public form<pads>
    {
        dent padding; // pads: Space around an element's content, outside of any defined borders. It does not affect the size, only affects the fill. Used in base::renderproc only.
        dent margins; // pads: Space around an element's content, inside of any defined borders. Containers take this parameter into account when calculating sizes. Used in all conainers.

    public:
        sptr<base> client;

        pads(dent const& padding_value = {}, dent const& margins_value = {})
            : padding{ padding_value },
              margins{ margins_value }
        {
            SUBMIT(e2::preview, e2::size::set, new_size)
            {
                if (client)
                {
                    auto client_size = new_size - padding;
                    client->SIGNAL(e2::preview, e2::size::set, client_size);
                    new_size = client_size + padding;
                    //todo unify
                    //auto lims = base::limits();
                    //new_size = std::clamp(new_size, lims.min, lims.max);
                }
            };
            SUBMIT(e2::release, e2::size::set, new_size)
            {
                if (client)
                {
                    auto client_size = new_size - padding;
                    auto client_coor = padding.corner();
                    client->SIGNAL(e2::release, e2::size::set, client_size);
                    client->SIGNAL(e2::release, e2::coor::set, client_coor);
                }
            };
            SUBMIT(e2::release, e2::render::prerender, parent_canvas)
            {
                auto view = parent_canvas.view();
                parent_canvas.view(view + margins);
                this->SIGNAL(e2::release, e2::render::any, parent_canvas);
                parent_canvas.view(view);
                if (client)
                    parent_canvas.render(client, base::coor());
                bell::expire(e2::release);
            };
        }
        // pads: Attach specified item.
        template<class T>
        auto attach(sptr<T> item)
        {
            client = item;
            item->SIGNAL(e2::release, e2::form::upon::vtree::attached, This());
            return item;
        }
        // pads: Create a new item of the specified subtype and attach it.
        template<class T, class ...Args>
        auto attach(Args&&... args)
        {
            return attach(create<T>(std::forward<Args>(args)...));
        }
        // pads: Remove item.
        void remove(sptr<base> item_ptr)
        {
            if (client == item_ptr)
            {
                auto backup = This();
                client.reset();
                item_ptr->SIGNAL(e2::release, e2::form::upon::vtree::detached, backup);
            }
        }
        // pads: Update nested object.
        template<class T, class S>
        void update(T old_item_ptr, S new_item_ptr)
        {
            auto backup = This();
            client = new_item_ptr;
            old_item_ptr->SIGNAL(e2::release, e2::form::upon::vtree::detached, backup);
            new_item_ptr->SIGNAL(e2::release, e2::form::upon::vtree::attached, backup);
        }
    };

    // controls: Pluggable dummy object.
    class mock
        : public form<mock>
    { };

    // controls: Menu label.
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
            SUBMIT(e2::release, e2::render::any, parent_canvas)
            {
                parent_canvas.cup(dot_00);
                parent_canvas.output(name);
                if (test)
                {
                    auto area = parent_canvas.view();
                    auto size = name.size();
                    if (area.size.x > 0 && area.size.x < size.x)
                    {
                        auto coor = area.coor + area.size - dot_11;
                        parent_canvas.core::data(coor)->txt(dots);
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

    // controls: Edit control.
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
            SUBMIT(e2::release, e2::size::set, new_sz) { canvas.size(new_sz); };
            SUBMIT(e2::release, e2::coor::set, new_xy) { canvas.move(new_xy); };
            SUBMIT(e2::request, e2::form::canvas, canvas) { canvas = coreface; };

            sfx_len = utf::length(sfx_str);

            canvas.mark().bgc(whitelt);
            topic = ansi::idx(txt_id).nop().eol()
                + ansi::idx(pin_id).nop();

            set_pen(0);

            SUBMIT(e2::preview, e2::size::set, size)
            {
                size = box_len; // Suppress resize.
            };
            SUBMIT(e2::release, e2::form::state::mouse, active)
            {
                set_pen(active ? 80 : 0);
                recalc();
            };
            SUBMIT(e2::release, e2::render::any, parent_canvas)
            {
                if (base::ruined())
                {
                    canvas.wipe();
                    canvas.output(topic);
                    base::ruined(faux);
                }
                parent_canvas.plot(canvas);
            };
        }
    };

    template<e2::tier TIER, e2::type EVENT>
    class stem_rate
        : public base
    {
        pro::mouse mouse{*this }; // stem_rate: Mouse controller.
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
                deface();

                return true;
            }
            return faux;
        }
        void move_grip(iota new_val)
        {
            if (_move_grip(new_val))
            {
                SIGNAL(TIER, EVENT, cur_val);
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
            SUBMIT(e2::release, e2::size::set, new_sz) { canvas.size(new_sz); };
            SUBMIT(e2::release, e2::coor::set, new_xy) { canvas.move(new_xy); };
            SUBMIT(e2::request, e2::form::canvas, canvas) { canvas = coreface; };

            cur_val = -1;
            SIGNAL(TIER, EVENT, cur_val);

            limit.set(twod{ utf::length(caption) + (pad + 2) * 2,
                           10 });

            topic = ansi::wrp(wrap::off).jet(bias::left)
                .cpy(50).chx(pad + 2).cuu(3) + caption + ansi::cud(3)
                + ansi::idx(bar_id).nop().eol()
                + ansi::idx(min_id).nop() + ansi::idx(max_id).nop();

            topic[min_id] = std::to_string(min_val);
            topic[max_id] = std::to_string(max_val);
            topic[max_id].style.jet(bias::right);
            topic[max_id].locus.chx(pad);
            topic[min_id].locus.chx(pad);

            SUBMIT(e2::general, e2::form::global::lucidity, alpha)
            {
                if (alpha >= 0 && alpha < 256)
                {
                    canvas.mark().alpha(alpha);
                    base::deface();
                }
            };
            SUBMIT(TIER, EVENT, cur_val)
            {
                if (cur_val >= min_val)
                {
                    _move_grip(cur_val);
                }
            };
            SUBMIT(e2::release, e2::form::upon::vtree::attached, parent)
            {
                grip_ctl = create<stem_rate_grip>(grip_suffix);
                grip_ctl->SIGNAL(e2::release, e2::form::upon::vtree::attached, This());

                grip_ctl->SUBMIT(e2::release, e2::hids::mouse::button::drag::start::left, gear)
                {
                    if (gear.capture(grip_ctl->id))
                    {
                        origin = cur_val;
                        gear.dismiss();
                    }
                };
                grip_ctl->SUBMIT(e2::release, e2::hids::mouse::button::drag::pull::left, gear)
                {
                    if (gear.captured(grip_ctl->id))
                    {
                        deltas += gear.mouse::delta.get().x;
                        move_grip(next_val(deltas));
                        gear.dismiss();
                    }
                };
                grip_ctl->SUBMIT(e2::release, e2::hids::mouse::button::drag::cancel::left, gear)
                {
                    giveup(gear);
                };
                grip_ctl->SUBMIT(e2::general, e2::hids::mouse::gone, gear)
                {
                    giveup(gear);
                };
                grip_ctl->SUBMIT(e2::release, e2::hids::mouse::button::drag::stop::left, gear)
                {
                    if (gear.captured(grip_ctl->id))
                    {
                        deltas = 0;
                        gear.release();
                        deface();
                        robot.actify(bygone.fader<quadratic<iota>>(750ms), [&](auto& delta)
                            {
                                move_grip(cur_val + delta);
                            });
                        gear.dismiss();
                    }
                };
                grip_ctl->SUBMIT(e2::release, e2::hids::mouse::scroll::up, gear)
                {
                    move_grip(cur_val - 1);
                    gear.dismiss();
                };
                grip_ctl->SUBMIT(e2::release, e2::hids::mouse::scroll::down, gear)
                {
                    move_grip(cur_val + 1);
                    gear.dismiss();
                };
                this->SUBMIT(e2::release, e2::size::set, size)
                {
                    recalc();
                };
                recalc();
            };
            SUBMIT(e2::release, e2::hids::mouse::button::click::right, gear)
            {
                color(canvas.mark().fgc(), (tint)((++bgclr) % 16));
                deface();
                gear.dismiss();
            };
            SUBMIT(e2::release, e2::hids::mouse::scroll::up, gear)
            {
                move_grip(cur_val - 10);
                gear.dismiss();
            };
            SUBMIT(e2::release, e2::hids::mouse::scroll::down, gear)
            {
                move_grip(cur_val + 10);
                gear.dismiss();
            };
            SUBMIT(e2::release, e2::render::any, parent_canvas)
            {
                if (base::ruined())
                {
                    canvas.wipe(base::color());
                    canvas.output(topic);
                    auto cp = canvas.cp();
                    cp.x = pin_pos;
                    cp.y -= 3;
                    grip_ctl->base::moveto(cp);
                    canvas.render(grip_ctl, base::coor());
                    base::ruined(faux);
                }
                parent_canvas.plot(canvas);
            };
        }
    };
}

#endif // NETXS_CONTROLS_HPP