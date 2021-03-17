// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_RING_HPP
#define NETXS_RING_HPP

#include <cassert> // ::assert(msg, 0)

#ifndef faux
    #define faux (false)
#endif

namespace netxs::generics
{
    template<class T>
    struct ring
    {
        using heap = T;
        using type = typename T::value_type;
        using iota = int32_t;

        // []
        // addr = addr < 0 ? addr % peak + peak
        //                 : addr % peak;
        // inc()
        // addr = (addr + 1) % peak;
        // dec()
        // addr = (addr + peak - 1) % peak;

        class iter
        {
            friend struct ring;
        protected:
            ring& buff;
            iota  addr;
            iter(ring& buff, iota addr)
              : buff{ buff },
                addr{ addr }
            { }

        public:
            auto  operator -  (iter const& r) { return addr - r.addr;          }
            auto  operator -  (int n)         { return iter{ buff, addr - n }; }
            auto  operator +  (int n)         { return iter{ buff, addr + n }; }
            auto  operator ++ (int)           { return iter{ buff, addr++   }; }
            auto  operator -- (int)           { return iter{ buff, addr--   }; }
            auto& operator ++ ()              { ++addr; return *this;          }
            auto& operator -- ()              { --addr; return *this;          }
            auto& operator *  ()              { return   buff[addr];           }
            auto  operator -> ()              { return &(buff[addr]);          }
            auto  operator != (iter const& m) const
            {
                assert(&buff == &m.buff);
                return addr != m.addr;
            }
            auto operator == (iter const& m) const
            {
                assert(&buff == &m.buff);
                return addr == m.addr;
            }
        };

        heap buff; // ring: Inner container
        iota peak; // ring: Limit of the ring buffer (-1: unlimited)
        iota size; // ring: Elements count
        iota head; // ring: head
        iota tail; // ring: back

        //ring(iota limit = -1)
        ring(iota peak = 30)
            : peak{ peak     },
              size{ 0        },
              head{ 0        },
              tail{ peak - 1 }
        {
            buff.resize(peak);
        }

        void inc(iota& a)         { if (++a == peak) a = 0;        }
        void dec(iota& a)         { if (--a < 0)     a = peak - 1; }
        auto mod(iota  a) const   { return a < 0 ? a % peak + peak
                                                 : a % peak;       }
        auto  begin()             { return iter{ *this, 0    };    }
        auto  end()               { return iter{ *this, size };    }
        auto& operator[] (iota i) { return buff[mod(head + i)];    }
        //auto& back()              { return buff[mod(tail - 1)];    }
        auto& back()              { return buff[tail];             }
        auto& front()             { return buff[head];             }
        auto& length() const      { return size;                   }
        template<class ...Args>
        auto& push(Args&&... args)
        {
            inc(tail);
            if (size != peak) ++size;
            else              inc(head);
            auto& item = back();
            item = type(std::forward<Args>(args)...);
            return item;
        }
        void pop()
        {
            assert(size > 0);
            back() = type{};
            dec(tail);
            --size;
        }
        void clear()
        {
            while(size) pop();
            head = 0;
            tail = peak - 1;
        }
        void resize(iota newsize)
        {
            heap temp;
            temp.reserve(newsize);
            if (size > newsize)
            {
                auto diff = size - newsize;
                head = mod(head + diff);
                size = newsize;
            }
            auto i = size;
            while(i--)
            {
                temp.emplace_back(std::move(front()));
                inc(head);
            }
            temp.resize(newsize);
            std::swap(buff, temp);
            peak = newsize;
            head = 0;
            tail = size ? size - 1 : peak - 1;
        }
    };
}

#endif // NETXS_RING_HPP