// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_RING_HPP
#define NETXS_RING_HPP

#include <vector>

#ifndef faux
    #define faux (false)
#endif

namespace netxs::generics
{
    template<class T>
    struct ring
    {
        using buff = std::vector<T>;
        using iota = int32_t;

        struct iter
        {

        };

        buff batch; // ring: Inner container
        iota limit; // ring: Limit of the ring buffer (-1: unlimited)
        iota count; // ring: Elements count
        iota start; // ring: Ring head
        iota finis; // ring: Ring tail

        ring(iota limit = -1)
            : limit{ limit }
        { }

        // []
        // addr = addr < 0 ? addr % size + size
        //                 : addr % size;
        // inc()
        // addr = (addr + 1) % size;
        // dec()
        // addr = (addr + size - 1) % size;

        // ring: count()/size()/length()
        auto size() const
        {

        }
        // ring: begin()
        auto begin()
        {

        }
        // ring: begin() const
        auto begin() const
        {

        }
        // ring: end()
        auto end()
        {

        }
        // ring: end() const
        auto end() const
        {

        }
        // ring: operator[]
        auto& operator[] (size_t n)
        {

        }
        // ring: operator[] const
        auto& operator[] (size_t n) const
        {

        }
        // ring: back()
        auto& back()
        {
            
        }
        // ring: back() const
        auto& back() const
        {

        }
        // ring: front()
        auto& front()
        {
            
        }
        // ring: front() const
        auto& front() const
        {

        }
        // ring: push_back() move
        void push_back(T&& a)
        {

        }
        // ring: push_back() copy
        void push_back(T const& a)
        {

        }
        // ring: push_front() move
        void push_front(T&& a)
        {
            
        }
        // ring: push_front() copy
        void push_front(T const& a)
        {

        }
        // ring: pop_back()
        void pop_back()
        {

        }
        // ring: pop_front()
        void pop_front()
        {
            
        }
        // ring: emplace_back()
        template<class... Args>
        void emplace_back(Args&&... args)
        {

        }
        // ring: emplace_front()
        template<class... Args>
        void emplace_front(Args&&... args)
        {

        }
        // ring: clear()
        void clear()
        {

        }
        // ring: resize()
        void resize(size_t newsize)
        {
            
        }
        // ring: erase()
        auto erase(iter const& pos)
        {

        }
        // ring: erase()
        auto erase(iter const& first, iter const& last) // [first, last) -> last/end()
        {

        }
    };
}

#endif // NETXS_RING_HPP