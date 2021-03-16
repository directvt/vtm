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
        auto mod(iota  addr) const
        { 
            return addr < 0 ? addr % peak + peak
                            : addr % peak;
        }
        void inc(iota& addr)       { if (++addr == peak) addr = 0;        }
        void dec(iota& addr)       { if (--addr < 0)     addr = peak - 1; }
        auto get(iota  addr) const { return mod(head + addr); }

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
            auto operator + (int n)
            {
                return iter{ buff, addr + n };
            }
            auto operator - (int n)
            {
                return iter{ buff, addr - n };
            }
            auto operator - (iter const& r)
            {
                return addr - r.addr;
            }
            auto operator ++ (int) // Postfix++
            {
                return iter{ buff, addr++ };
            }
            auto& operator ++ () // ++Prefix
            {
                ++addr;
                return *this;
            }
            auto operator -- (int) // Postfix--
            {
                return iter{ buff, addr-- };
            }
            auto& operator -- () // --Prefix
            {
                --addr;
                return *this;
            }
            auto& operator * ()
            {
                return buff[addr];
            }
            auto operator -> ()
            {
                return &(buff[addr]);
            }
            //auto get_addr() const
            //{
            //    return buff.get(addr);
            //}
            auto operator != (iter const& m) const
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
        iota head; // ring: Ring head
        iota tail; // ring: Ring tail

        //ring(iota limit = -1)
        ring(iota limit = 30)
            : peak{ limit },
              size{ 0     },
              head{ 0     },
              tail{ 0     }
        {
            buff.resize(limit);
        }

        // ring: count()/size()/length()
        auto length() const
        {
            return size;
        }
        // ring: begin()
        auto begin()
        {
            return iter{ *this, 0 };
        }
        // ring: begin() const
            //auto begin() const
            //{
            //
            //}
        // ring: end()
        auto end()
        {
            return iter{ *this, size };
        }
        // ring: end() const
            //auto end() const
            //{
            //
            //}
        // ring: operator[]
        auto& operator[] (iota addr)
        {
            return buff[mod(head + addr)];
        }
        // ring: operator[] const
            //auto& operator[] (iota n) const
            //{
            //
            //}
        // ring: back()
        auto& back()
        {
            return buff[mod(tail - 1)];
        }
        // ring: back() const
            //auto& back() const
            //{
            //
            //}
        // ring: front()
        auto& front()
        {
            return buff[head];
        }
        // ring: front() const
            //auto& front() const
            //{
            //
            //}
        // ring: push_back() move
        void push_back(type&& a)
        {
            inc(tail);
            if (size != peak) ++size;
            else              inc(head);
            back() = std::move(a);
        }
        // ring: push_back() copy
            //void push_back(type const& a)
            //{
            //
            //}
        // ring: push_front() move
        void push_front(type&& a)
        {
            dec(head);
            if (size != peak) ++size;
            else              dec(tail);
            front() = std::move(a);
        }
        // ring: push_front() copy
            //void push_front(type const& a)
            //{
            //
            //}
        // ring: pop_back()
        void pop_back()
        {
            assert(size > 0);
            back() = type{};
            dec(tail);
            --size;
        }
        // ring: pop_front()
        void pop_front()
        {
            assert(size > 0);
            front() = type{};
            inc(head);
            --size;
        }
        // ring: emplace_back()
        template<class... Args>
        void emplace_back(Args&&... args)
        {
            push_back(type(std::forward<Args>(args)...));
        }
        // ring: emplace_front()
        template<class... Args>
        void emplace_front(Args&&... args)
        {
            push_front(type(std::forward<Args>(args)...));
        }
        // ring: clear()
        void clear()
        {
            while(size) pop_back();
            head = 0;
            tail = 0;
        }
        // ring: resize()
            //void resize(size_t newsize)
            //{
            //    buff.resize(newsize);
            //    peak = newsize;
            //    //...
            //    //head = std::clamp(head, ...);
            //    //tail = std::clamp(tail, ...);
            //}
        // ring: erase()
            //auto erase(iter const& pos)
            //{
            //
            //}
        // ring: erase()
            //auto erase(iter const& first, iter const& last) // [first, last) -> last/end()
            //{
            //
            //}
    };
}

#endif // NETXS_RING_HPP