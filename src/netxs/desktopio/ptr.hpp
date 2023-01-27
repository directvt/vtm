// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#include <memory>
#include <mutex>

namespace netxs
{
    template<class T> using sptr = std::shared_ptr<T>;
    template<class T> using wptr = std::  weak_ptr<T>;
    template<class T> using uptr = std::unique_ptr<T>;

    // Due to the fact that alias templates are never deduced by template argument deduction (C++20).
    namespace ptr
    {
        template<class T>
        auto shadow(sptr<T> p)
        {
            return std::weak_ptr<T>{ p };
        }
        template<class T>
        auto shared(T&& from)
        {
            return std::make_shared<std::decay_t<T>>(std::forward<T>(from));
        }
    }

    template<class T1, class T2>
    inline bool equals(std::weak_ptr<T1> const& p1, std::weak_ptr<T2> const& p2)
    {
        return !p1.owner_before(p2)
            && !p2.owner_before(p1);
    }

    template<class T1, class T2>
    inline bool equals(std::shared_ptr<T1> const& p1, std::weak_ptr<T2> const& p2)
    {
        return !p1.owner_before(p2)
            && !p2.owner_before(p1);
    }

    template<class T1, class T2>
    inline bool equals(std::weak_ptr<T1> const& p1, std::shared_ptr<T2> const& p2)
    {
        return !p1.owner_before(p2)
            && !p2.owner_before(p1);
    }

    template<class T>
    auto shared_singleton()
    {
        static std::mutex mutex;
        static wptr<T>    count;

        auto guard = std::lock_guard{ mutex };

        auto thing = count.lock();
         if (thing) return thing;

        thing = std::make_shared<T>();
        count = thing;

        return  thing;
    }

    template<class...> struct change_value_type_helper;
    template<template<class...> class C, class... Args>
    struct change_value_type_helper<C<Args...>>
    {
        template<class ...NewArgs>
        using new_type = C<NewArgs...>;
    };
    template<class C, class T>
    using change_value_type = typename change_value_type_helper<C>::template new_type<T>;
}