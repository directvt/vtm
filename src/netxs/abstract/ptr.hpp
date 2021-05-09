// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_PTR_HPP
#define NETXS_PTR_HPP

#include <memory>

namespace netxs
{
    template<class T>
    struct testy
    {
        T prev;
        T last;
        bool operator()(T newvalue)
        {
            prev = last;
            bool result = last != newvalue;
            if (result)
            {
                last = newvalue;
            }
            return result;
        }
        operator T& ()
        {
            return last;
        }
    };

    struct	null_deleter
    {
        void operator()(void const*) const
        { }
    };

    template<class T> using sptr = std::shared_ptr<T>;
    template<class T> using wptr = std::  weak_ptr<T>;
    template<class T> using uptr = std::unique_ptr<T>;

    // Due to the fact that alias templates are never deduced by template argument deduction (C++20).
    namespace ptr
    {
        template <class T>
        auto shadow(sptr<T> p)
        {
            return std::weak_ptr<T>{ p };
        }
    }

    template <class T1, class T2>
    inline bool equals(std::weak_ptr<T1> const& p1, std::weak_ptr<T2> const& p2)
    {
        return !p1.owner_before(p2)
            && !p2.owner_before(p1);
    }

    template <class T1, class T2>
    inline bool equals(std::shared_ptr<T1> const& p1, std::weak_ptr<T2> const& p2)
    {
        return !p1.owner_before(p2)
            && !p2.owner_before(p1);
    }

    template <class T1, class T2>
    inline bool equals(std::weak_ptr<T1> const& p1, std::shared_ptr<T2> const& p2)
    {
        return !p1.owner_before(p2)
            && !p2.owner_before(p1);
    }

    template <class T>
    auto shared_singleton()
    {
        static std::mutex mutex;
        static wptr<T>    count;

        std::lock_guard lock(mutex);

        auto thing = count.lock();
        if  (thing) return thing;

        thing = std::make_shared<T>();
        count = thing;

        return  thing;
    }
}

#endif // NETXS_PTR_HPP