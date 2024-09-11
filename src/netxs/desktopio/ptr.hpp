// Copyright (c) Dmitry Sapozhnikov
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
        auto test(T a, T b)
        {
            return a ? a : b;
        }
        template<class T>
        auto shadow(sptr<T> p)
        {
            return wptr<T>{ p };
        }
        template<class T = void, class ...Args>
        auto shared(Args&&... args)
        {
            if constexpr (std::is_same_v<T, void>) return std::make_shared<std::decay_t<Args>...>(std::forward<Args>(args)...);
            else                                   return std::make_shared<T>(std::forward<Args>(args)...);
        }
        namespace
        {
            template<class T>
            struct _function : public _function<decltype(&T::operator())> // Functors.
            { };
            template<class R, class ...Args>
            struct _function<R(*)(Args...)> // Static pointers.
            {
                using type = std::function<R(Args...)>;
            };
            template<class R, class A, class ...Args>
            struct _function<R(A::*)(Args...)> // Member functions.
            {
                using type = std::function<R(Args...)>;
            };
            template<class R, class A, class ...Args>
            struct _function<R(A::*)(Args...) const> // Const member functions.
            {
                using type = std::function<R(Args...)>;
            };
        }
        template<class F>
        auto function(F lambda) // Don't use lambdas/functions with auto args here.
        {
            return ptr::shared(typename _function<F>::type{ lambda });
        }
        template<class T>
        auto singleton()
        {
            static auto mutex = std::mutex{};
            static auto anker = wptr<T>{};

            auto guard = std::lock_guard{ mutex };
            auto thing = anker.lock();
            if (!thing)
            {
                thing = std::make_shared<T>();
                anker = thing;
            }
            return thing;
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

        template<class T, class Allocator = std::allocator<T>>
        struct raw_allocator : Allocator
        {
            using Allocator::Allocator;
            using allocator_traits = std::allocator_traits<Allocator>;

            template<class R>
            struct rebind
            {
                using other = raw_allocator<R, typename allocator_traits::template rebind_alloc<R>>;
            };
            template<class R>
            void construct(R* p) noexcept(std::is_nothrow_default_constructible_v<R>)
            {
                ::new((void*)p) R; // Construct an "R" object, placing it directly into pre-allocated storage at memory address p.
            }
            template<class R, class ...Args>
            void construct(R* p, Args&&... args)
            {
                allocator_traits::construct((Allocator&)*this, p, std::forward<Args>(args)...);
            }
        };
    }
    template<class T>
    using raw_vector = std::vector<T, ptr::raw_allocator<T>>;
}