// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_RING_HPP
#define NETXS_RING_HPP

#include "../math/intmath.hpp"

namespace netxs::generics
{
    template<class vect, bool USE_UNDOCK = faux>
    struct ring
    {
        using type = typename vect::value_type;

        si32 step; // ring: Unlimited buffer increment step (zero for fixed size buffer).
        si32 head; // ring: Front index.
        si32 tail; // ring: Back index.
        si32 peak; // ring: Limit of the ring buffer.
        vect buff; // ring: Inner container.
        si32 size; // ring: Elements count.
        si32 cart; // ring: Active item position.
        si32 mxsz; // ring: Max unlimited buffer size.

        void inc(si32& a) const { if  (++a == peak) a = 0;        }
        void dec(si32& a) const { if  (--a < 0    ) a = peak - 1; }
        auto mod(si32  a) const { return a < 0  ? ++a % peak - 1 + peak
                                                :   a % peak;     }
        auto dst(si32  a, si32 b) const
                                { return b < a ? b - a + peak
                                               : b - a;           }
        template<class RING>
        struct iter
        {
            RING& buff;
            si32  addr;
            iter(RING& buff, si32 addr)
              : buff{ buff },
                addr{ addr }
            { }
            auto  operator -  (si32 n)        const {      return iter<RING>{ buff, buff.mod(addr - n) };                 }
            auto  operator +  (si32 n)        const {      return iter<RING>{ buff, buff.mod(addr + n) };                 }
            auto  operator ++ (int)                 { auto temp = iter<RING>{ buff, addr }; buff.inc(addr); return temp;  }
            auto  operator -- (int)                 { auto temp = iter<RING>{ buff, addr }; buff.dec(addr); return temp;  }
            auto& operator ++ ()                    {                                       buff.inc(addr); return *this; }
            auto& operator -- ()                    {                                       buff.dec(addr); return *this; }
            auto& operator *  ()                    { return buff.buff[addr];                                             }
            auto  operator -> ()                    { return buff.buff.begin() + addr;                                    }
            auto  operator != (iter const& m) const { return addr != m.addr;                                              }
            auto  operator == (iter const& m) const { return addr == m.addr;                                              }
        };

        ring(si32 ring_size, si32 grow_by = 0)
            : step{ grow_by                      },
              head{ 0                            },
              tail{ ring_size ? ring_size : step },
              peak{ tail + 1                     },
              buff( peak                         ), // Rounded brackets! Not curly! In oreder to call T::ctor().
              size{ 0                            },
              cart{ 0                            },
              mxsz{ maxsi32 - step               }
        { }

        virtual void undock_base_front(type&) { };
        virtual void undock_base_back (type&) { };

        auto  current_it()        { return iter<      ring>{ *this, cart };          }
        auto  begin()             { return iter<      ring>{ *this, head };          }
        auto    end()             { return iter<      ring>{ *this, mod(tail + 1) }; }
        auto  begin() const       { return iter<const ring>{ *this, head };          }
        auto    end() const       { return iter<const ring>{ *this, mod(tail + 1) }; }
        auto& length() const      { return size;                }
        auto&  back()             { return buff[tail];          }
        auto&  back() const       { return buff[tail];          }
        auto& front()             { return buff[head];          }
        auto& front() const       { return buff[head];          }
        auto& current     ()      { return buff[cart];          }
        auto& operator  * ()      { return buff[cart];          }
        auto  operator -> ()      { return buff.begin() + cart; }
        auto&         at (si32 i) { assert(i >= 0 && i < size); return buff[mod(head + i)]; }
        auto& operator[] (si32 i) { return at(i);               }
        auto  index() const       { return dst(head, cart);     }
        void  index(si32 i)       { assert(i >= 0 && i < size); cart = mod(head + i); }
        void  prev()              { dec(cart); test();          }
        void  next()              { inc(cart); test();          }

    private:
        void  test()
        {
            assert((head == tail &&  cart == head)
                || (head <  tail &&  cart >= head && cart <= tail)
                || (head >  tail && (cart >= head && cart >= tail
                                  || cart <= head && cart <= tail)));
        }
        auto  full()
        {
            if (size == peak - 1)
            {
                if (step && peak < mxsz) resize(size + step, step);
                else                     return true;
            }
            return faux;
        }
        template<bool USE_BACK = faux>
        inline void undock_front()
        {
            auto& item = front();
            if constexpr (USE_UNDOCK)
            {
                if constexpr (USE_BACK) undock_base_back (item);
                else                    undock_base_front(item);
            } 
            item = type{};
            if (cart == head) inc(head), cart = head;
            else              inc(head);
        }
        inline void undock_back()
        {
            auto& item = back();
            if constexpr (USE_UNDOCK) undock_base_back(item);
            item = type{};
            if (cart == tail) dec(tail), cart = tail;
            else              dec(tail);
        }
        // ring: Insert an item before the specified position. Pop front when full. Return an iterator pointing to the new item.
        template<class ...Args>
        auto insert_impl(si32 at, Args&&... args) // Pop front always if ring is full.
        {
            if (at == 0)
            {
                push_front(std::forward<Args>(args)...);
                return begin();
            }
            else
            if (at == size)
            {
                push_back(std::forward<Args>(args)...);
                return begin() + size - 1;
            }

            auto d = at;
            if (size >> 1 > d)
            {
                auto it_1 = begin();
                auto it_2 = it_1 + d;
                if (full())
                {
                    auto& item = front();
                    if constexpr (USE_UNDOCK) undock_base_front(item);
                    item = type(std::forward<Args>(args)...);
                    ++it_1;
                }
                else
                {
                    ++size;
                    dec(head);
                    auto& item = front();
                    item = type(std::forward<Args>(args)...);
                }
                swap_block<true>(it_1, it_2, begin());
                --it_2;
                return it_2;
            }
            else
            {
                auto dest = end();
                auto it_1 = dest - 1;
                d = size - d;
                auto it_2 = it_1 - d;
                if (full())
                {
                    auto& item = front();
                    if constexpr (USE_UNDOCK) undock_base_front(item);
                    item = type{};
                    if (cart == head) inc(head), cart = head;
                    else              inc(head);
                }
                else ++size;
                inc(tail);
                back() = type(std::forward<Args>(args)...);
                swap_block<faux>(it_1, it_2, end() - 1);
                ++it_2;
                return it_2;
            }
        }

    public:
        template<class ...Args>
        type& push_back(Args&&... args)
        {
            if (full()) undock_front();
            else        ++size;
            inc(tail);
            auto& item = back();
            item = type(std::forward<Args>(args)...);
            return item;
        }
        template<class ...Args>
        type& push_front(Args&&... args)
        {
            if (full()) undock_back();
            else        ++size;
            dec(head);
            auto& item = front();
            item = type(std::forward<Args>(args)...);
            return item;
        }
        template<bool USE_BACK = faux>
        void pop_front() { undock_front<USE_BACK>(); --size; }
        void pop_back () { undock_back();            --size; }
        // ring: Insert an item before the specified position. Pop front when full. Return an iterator pointing to the new item.
        template<class ...Args>
        auto insert(si32 at, Args&&... args) // Pop front always if ring is full.
        {
            assert(at >= 0 && at <= size);
            auto temp = index();
            if (full())
            {
                if (temp > 0 && temp < at) temp--;
            }
            else
            {
                if (temp >= at) temp++;
            }
            auto iter = insert_impl(at, std::forward<Args>(args)...);
            index(temp);
            return iter;
        }
        auto remove(si32 at, si32 n)
        {
            assert(at >= 0 && at < size);

            auto max = size - at;
            if (n > max) n = max;

            auto vol = n;
            auto tmp = index();
                 if (tmp >= at + n) tmp -= n;
            else if (tmp >= at    ) tmp = std::max(at - 1, 0);

            auto top_block = at;
            auto btm_block = max - n;
            if (btm_block > top_block)
            {
                auto tail = begin() - 1;
                auto head = tail + top_block;
                netxs::swap_block<faux>(head, tail, head + n);
                static constexpr auto USE_BACK = true;
                while (n-- > 0) pop_front<USE_BACK>();
            }
            else
            {
                auto tail = end();
                auto head = tail - btm_block;
                netxs::swap_block<true>(head, tail, head - n);
                while (n-- > 0) pop_back();
            }
            index(tmp); // Restore current item selector.
            return vol;
        }
        void clear()
        {
            if constexpr (USE_UNDOCK) while (size) pop_back(); //todo undock?
            else                      size = 0;
            cart = 0;
            head = 0;
            tail = peak - 1;
        }
        template<bool BOTTOM_ANCHORED = true>
        void resize(si32 new_size, si32 grow_by = 0)
        {
            if (new_size > 0)
            {
                if constexpr (BOTTOM_ANCHORED)
                {
                    if (size > new_size)
                    {
                        //todo optimize for !USE_UNDOCK
                        do
                        {
                            if constexpr (USE_UNDOCK) undock_base_front(front());
                            inc(head);
                        }
                        while (--size != new_size);
                    }
                    cart = std::max(0, size - 1 - dst(cart, tail));
                }
                else // TOP_ANCHORED
                {
                    if (size > new_size)
                    {
                        //todo optimize for !USE_UNDOCK
                        do
                        {
                            if constexpr (USE_UNDOCK) undock_base_back(back());
                            dec(tail);
                        }
                        while (--size != new_size);
                    }
                    cart = std::min(size - 1, dst(head, cart));
                }
                vect temp;
                temp.reserve(++new_size);
                auto i = size;
                while (i--)
                {
                    temp.emplace_back(std::move(front()));
                    inc(head);
                }
                temp.resize(new_size);
                std::swap(buff, temp);
                peak = new_size;
                head = 0;
                tail = size ? size - 1 : peak - 1;
                step = grow_by;
            }
            else step = grow_by;
        }
        template<class P>
        void for_each(si32 from, si32 upto, P proc)
        {
            auto head = begin() + from;
            auto tail = begin() + upto;
            if constexpr (std::is_same_v< decltype(proc(*head)), bool >)
            {
                     if (from < upto) while (proc(*head) && ++head != tail);
                else if (from > upto) while (proc(*head) && --head != tail);
            }
            else
            {
                     if (from < upto) do { proc(*head); } while (++head != tail);
                else if (from > upto) do { proc(*head); } while (--head != tail);
            }
        }
    };
}

#endif // NETXS_RING_HPP