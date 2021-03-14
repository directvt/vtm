// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_HASH_HPP
#define NETXS_HASH_HPP

#include <type_traits>
#include <optional>

namespace netxs
{
    template <typename M, typename K>
    bool on_key(M const& map, K const& key)
    {
        const auto it = map.find(key);
        if (it == map.end())
            return false;
        else
            return true;
    }

    // do it in place
    //template <typename M, typename K>
    //auto	on_key_get(const M& map, const K& key)
    //{
    //	const auto it = map.find(key);
    //	return it == map.end() ? std::nullopt
    //	                       : std::optional{ it };
    //}

    template<typename M>
    struct addref
    {
        using type = typename std::conditional<std::is_class<typename M::mapped_type>::value, typename M::mapped_type &, typename M::mapped_type>::type;
    };

    template<typename M, typename K>
    typename addref<M>::type get_or(M& map, K const& key, typename addref<M>::type default_value)
    {
        const auto it = map.find(key);
        if (it == map.end())
            return default_value;
        else
            return it->second;
    }
}
#endif // NETXS_HASH_HPP