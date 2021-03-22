// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_RING_HPP
#define NETXS_RING_HPP

#include <cassert>

#ifndef faux
    #define faux (false)
#endif

namespace netxs::generics
{
    template<class T>
    struct ring
    {
        using type = typename T::value_type;
        using iota = int32_t;

        struct iter
        {
            ring& buff;
            iota  addr;
            iter(ring& buff, iota addr)
              : buff{ buff },
                addr{ addr }
            { }
            auto  operator -  (iter const& r)       { return addr - r.addr;          }
            auto  operator -  (int n)               { return iter{ buff, addr - n }; }
            auto  operator +  (int n)               { return iter{ buff, addr + n }; }
            auto  operator ++ (int)                 { return iter{ buff, addr++   }; }
            auto  operator -- (int)                 { return iter{ buff, addr--   }; }
            auto& operator ++ ()                    { ++addr; return *this;          }
            auto& operator -- ()                    { --addr; return *this;          }
            auto& operator *  ()                    { return   buff[addr];           }
            auto  operator -> ()                    { return &(buff[addr]);          }
            auto  operator != (iter const& m) const { return addr != m.addr;         }
            auto  operator == (iter const& m) const { return addr == m.addr;         }
        };

        T    buff; // ring: Inner container
        bool flex; // ring: True if unlimited
        iota peak; // ring: Limit of the ring buffer
        iota size; // ring: Elements count
        iota cart; // ring: Active item position
        iota head; // ring: head
        iota tail; // ring: back
        iota step; // ring: Unlimited buffer increment step
        iota mxsz; // ring: Max unlimited buffer size

        void init(iota ring_size = 0, iota grow_by = 2)
        {
            assert(ring_size >= 0 && grow_by >= 0);
            flex = !ring_size;
            step = grow_by;
            peak = flex ? step : ring_size;
            size = 0;
            cart = 0;
            head = 0;
            tail = peak - 1;
            buff.resize(peak);
            mxsz = std::numeric_limits<iota>::max() - step;
        }
        void inc(iota& a) const   { if  (++a == peak) a = 0;        }
        void dec(iota& a) const   { if  (--a < 0    ) a = peak - 1; }
        auto mod(iota  a) const   { return a < 0  ? ++a % peak - 1 + peak
                                                  :   a % peak;     }
        auto dst(iota  a, iota b) const
                                  { return b < a ? b - a + peak
                                                 : b - a;           }
        auto  begin()             { return iter{ *this, 0    };     }
        auto  end()               { return iter{ *this, size };     }
        auto& operator[] (iota i) { return buff[mod(head + i)];     }
        auto& back()              { return buff[tail];              }
        auto& front()             { return buff[head];              }
        auto& length() const      { return size;                    }
        auto  full()
        {
            if (size == peak)
            {
                if (flex && peak < mxsz) resize(peak + step, true);
                else                     return true;
            }
            return faux;
        }
        template<class ...Args>
        auto& push(Args&&... args)
        {
            if (full())
            {
                if (cart == head) inc(head), cart = head;
                else              inc(head);
            }
            else ++size;
            inc(tail);
            auto& item = back();
            item = type(std::forward<Args>(args)...);
            return item;
        }
        void pop()
        {
            assert(size > 0);
            back() = type{};
            if (cart == tail) dec(tail), cart = tail;
            else              dec(tail);
            --size;
        }
        void clear()
        {
            while(size) pop();
            cart = 0;
            head = 0;
            tail = peak - 1;
        }
        void resize(iota new_size, bool is_unlimited = faux)
        {
            if (new_size > 0)
            {
                T temp;
                temp.reserve(new_size);
                auto dist = dst(cart, tail);
                if (size > new_size)
                {
                    auto diff = size - new_size;
                    head = mod(head + diff);
                    size = new_size;
                }
                auto i = size;
                while(i--)
                {
                    temp.emplace_back(std::move(front()));
                    inc(head);
                }
                temp.resize(new_size);
                std::swap(buff, temp);
                peak = new_size;
                head = 0;
                flex = is_unlimited;
                tail = size ? size - 1 : peak - 1;
                cart = size ? std::max(0, tail - dist) : 0;
            }
            else flex = true;
        }
        auto& operator  * () { return buff[cart];           }
        auto  operator -> () { return buff.begin() + cart;  }
        auto  set(iota i)    { return cart = mod(head + i); }
        auto  get() const    { return dst(head, cart);      }
    };
}

#endif // NETXS_RING_HPP