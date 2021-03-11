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
        iota start; // ring: Ring start
        iota finis; // ring: Ring tail
        
        ring(iota limit = -1)
            : limit{ limit }
        { }

        // count()/size()/length()
        // begin() (+const)
        // end() (+const)
        // rbegin() (+const)
        // end() (+const)
        // operator[] (+const)
        // back() (+const)
        // front() (+const)
        // push_back()
        // push_front()
        // pop_back()
        // pop_front()
        // emplace_back()
        // emplace_front()
        // clear()
        // resize()
        // erase()
        // 

    };
}

#endif // NETXS_RING_HPP