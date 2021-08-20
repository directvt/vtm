// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_RING_HPP
#define NETXS_RING_HPP

#ifndef faux
    #define faux (false)
#endif

namespace netxs::generics
{
    template<class vect, class iota = int32_t>
    struct ring
    {
        using type = typename vect::value_type;

        struct iter
        {
            ring& buff;
            iota  addr;
            iter(ring& buff, iota addr)
              : buff{ buff },
                addr{ addr }
            { }
            auto  operator =  (iter const& m)       { addr = m.addr;                 }
            auto  operator -  (iter const& r)       { return addr - r.addr;          }
            auto  operator -  (iota n)              { return iter{ buff, addr - n }; }
            auto  operator +  (iota n)              { return iter{ buff, addr + n }; }
            auto  operator ++ (iota)                { return iter{ buff, addr++   }; }
            auto  operator -- (iota)                { return iter{ buff, addr--   }; }
            auto& operator ++ ()                    { ++addr; return *this;          }
            auto& operator -- ()                    { --addr; return *this;          }
            auto& operator *  ()                    { return   buff[addr];           }
            auto  operator -> ()                    { return &(buff[addr]);          }
            auto  operator != (iter const& m) const { return addr != m.addr;         }
            auto  operator == (iter const& m) const { return addr == m.addr;         }
            friend auto operator - (iter const& n, iter const& m) { return n.addr - m.addr; }
        };

        iota step; // ring: Unlimited buffer increment step (zero for fixed size buffer).
        iota peak; // ring: Limit of the ring buffer.
        vect buff; // ring: Inner container.
        iota size; // ring: Elements count.
        iota cart; // ring: Active item position.
        iota head; // ring: Front index.
        iota tail; // ring: Back index.
        iota mxsz; // ring: Max unlimited buffer size.

        ring(iota ring_size, iota grow_by)
            : step{ grow_by                                 },
              peak{ !ring_size ? step : ring_size           },
              buff( peak                                    ), // Rounded brackets! Not curly! In oreder to call T::ctor().
              size{ 0                                       },
              cart{ 0                                       },
              head{ 0                                       },
              tail{ peak - 1                                },
              mxsz{ std::numeric_limits<iota>::max() - step }
        { }

        virtual void undock(type&) = 0;

        void inc(iota& a) const   { if  (++a == peak) a = 0;        }
        void dec(iota& a) const   { if  (--a < 0    ) a = peak - 1; }
        auto mod(iota  a) const   { return a < 0  ? ++a % peak - 1 + peak
                                                  :   a % peak;     }
        auto dst(iota  a, iota b) const
                                  { return b < a ? b - a + peak
                                                 : b - a;           }
        auto  begin()             { return iter{ *this, 0    };     }
        auto  end()               { return iter{ *this, size };     }
        auto  current_it()        { return iter{ *this, cart };     }
        auto& operator[] (iota i) { return buff[mod(head + i)];     }
        auto& back()              { return buff[tail];              }
        auto& front()             { return buff[head];              }
        auto& length() const      { return size;                    }
        auto  full()
        {
            if (size == peak)
            {
                if (step && peak < mxsz) resize(peak + step, step);
                else                     return true;
            }
            return faux;
        }
        template<class ...Args>
        auto& push_back(Args&&... args)
        {
            if (full())
            {
                undock(front());
                if (cart == head) inc(head), cart = head;
                else              inc(head);
            }
            else ++size;
            inc(tail);
            auto& item = back();
            item = type(std::forward<Args>(args)...);
            return item;
        }
        template<class ...Args>
        auto& push_front(Args&&... args)
        {
            if (full())
            {
                undock(back());
                if (cart == tail) dec(tail), cart = tail;
                else              dec(tail);
            }
            else ++size;
            dec(head);
            auto& item = front();
            item = type(std::forward<Args>(args)...);
            return item;
        }
        void pop()
        {
            auto& item = back();
            undock(item);
            item = type{};
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
        template<bool BOTTOM_ANCHORED = true>
        void resize(iota new_size, iota grow_by = 0)
        {
            if (new_size > 0)
            {
                vect temp;
                temp.reserve(new_size);
                if constexpr (BOTTOM_ANCHORED)
                {
                    if (size > new_size)
                    {
                        do
                        {
                            undock(front());
                            inc(head);
                        }
                        while(--size != new_size);
                    }
                    cart = std::max(0, size - 1 - dst(cart, tail));
                }
                else // TOP_ANCHORED
                {
                    if (size > new_size)
                    {
                        do
                        {
                            undock(back());
                            dec(tail);
                        }
                        while(--size != new_size);
                    }
                    cart = std::min(size - 1, dst(head, cart));
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
                tail = size - 1;
                step = grow_by;
            }
            else step = grow_by;
        }
        auto& current     () { return buff[cart];           }
        auto& operator  * () { return buff[cart];           }
        auto  operator -> () { return buff.begin() + cart;  }
        auto  index (iota i) { return cart = mod(head + i); }
        auto  index () const { return dst(head, cart);      }
    };
}

#endif // NETXS_RING_HPP