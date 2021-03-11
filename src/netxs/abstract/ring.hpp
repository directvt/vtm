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
        //addr = (addr + 1) % size;
        // dec()
        //addr = (addr + size - 1) % size;

        // ring: count()/size()/length()
        auto count() const
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
        // ring: rbegin()
        auto rbegin()
        {

        }
        // ring: rbegin() const
        auto rbegin() const
        {

        }
        // ring: rend()
        auto rend()
        {

        }
        // ring: rend() const
        auto rend() const
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
        // ring: push_back()
        void push_back()
        {

        }
        // ring: push_front()
        void push_front()
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
        auto& emplace_back()
        {

        }
        // ring: emplace_front()
        auto& emplace_front()
        {
            
        }
        // ring: clear()
        void clear()
        {

        }
        // ring: resize()
        void resize()
        {
            
        }
        // ring: erase()
        auto erase()
        {

        }
    };
}

#endif // NETXS_RING_HPP