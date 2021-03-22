// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_ITERATOR_HPP
#define NETXS_ITERATOR_HPP

#include <memory>

namespace netxs
{
    // iterator.h: Search an item in the container for which the predicate(item) is true.
    template <class T, class P>
    auto search(T&& iterable, P predicate)
    {
        using std::begin;
        using std::end;
        auto it = begin(iterable);
        auto end_it = end(iterable);
        while (it != end_it)
        {
            if (predicate(*it))
            {
                break;
            }
            ++it;
        }
        return it;
    }

    // iterator.h: Execute the func(item) for each item in the iterable container if the predicate(item) is true.
    template <class T, class P, class F>
    void foreach(T&& iterable, P predicate, F func)
    {
        using std::begin;
        using std::end;
        auto it = begin(iterable);
        auto end_it = end(iterable);
        while (it != end_it)
        {
            auto& item = *it++;
            if (predicate(item))
            {
                func(item);
            }
        }
    }

    // iterator.h: Reversed iterable helper wrapper.
    template <class T>
    struct reversion_wrapper { T& iterable; };

    // iterator.h: Reversed iterable helper begin().
    template <class T>
    auto begin (reversion_wrapper<T> w) { return std::rbegin(w.iterable); }

    // iterator.h: Reversed iterable helper end().
    template <class T>
    auto end (reversion_wrapper<T> w) { return std::rend(w.iterable); }

    // iterator.h: Reverse iterable.
    template <class T>
    reversion_wrapper<T> reverse (T&& iterable) { return { iterable }; }

    //todo check traits of the T: T must implement operator bool() - the end sequence indicator
    template <class T>
    class iterator
    {
    protected:
        std::unique_ptr<T> current;
        iterator<T> *      instance;

        virtual T get() = 0;

        bool next() { return *(current = std::make_unique<T>(std::move(get()))); }

        class it
        {
            iterator<T> * source;

            void next() { if (source && !source->next()) source = nullptr; }

        public:
            using iterator_category = std::input_iterator_tag;
            using value_type = T;
            using pointer = T * ;
            using reference = T& ;
            using difference_type = void;

            it(iterator<T> * source) : source(source) { next(); }

            reference operator* () const { return *source->current; };
            pointer   operator->() const { return  source->current; };

            it&	operator++() { next(); return *this; }
            friend bool operator==(it const& lhs, it const& rhs) { return lhs.source == rhs.source; }
            friend bool operator!=(it const& lhs, it const& rhs) { return !(lhs == rhs); }
        };

    public:
        it begin() const { return it(instance); }
        it end()   const { return it(nullptr); }

        iterator() { instance = this; }

        iterator(iterator<T>&& x) : current(std::move(x.current))
        {
            instance = this;
            x.instance = nullptr;
        }
        iterator<T>& operator=(iterator<T>&& x)
        {
            current = std::move(x.current);
            return *(instance = this);
        }
    };
}

#endif // NETXS_ITERATOR_HPP