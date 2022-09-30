// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_HASH_HPP
#define NETXS_HASH_HPP

#include <type_traits>
#include <optional>
#include <unordered_map>
#include <map>

namespace netxs
{
    template <class M, class K>
    bool on_key(M const& map, K const& key)
    {
        const auto it = map.find(key);
        return it != map.end();
    }

    // do it in place
    //template <class M, class K>
    //auto	on_key_get(const M& map, const K& key)
    //{
    //	const auto it = map.find(key);
    //	return it == map.end() ? std::nullopt
    //	                       : std::optional{ it };
    //}

    template<class M>
    struct addref
    {
        using type = typename std::conditional<std::is_class<typename M::mapped_type>::value, typename M::mapped_type &, typename M::mapped_type>::type;
    };

    template<class M, class K>
    typename addref<M>::type get_or(M& map, K const& key, typename addref<M>::type default_value)
    {
        auto const it = map.find(key);
        if (it == map.end())
        {
            return default_value;
        }
        else
        {
            return it->second;
        }
    }
    template<class Map, class Key, class FBKey>
    auto& map_or(Map& map, Key const& key, FBKey const& fallback)
    {
        auto const it = map.find(key);
        return it == map.end() ? map[fallback]
                               : it->second;
    }

    // hash: Map that keeps the insertion order.
    template<class KEY, class VAL>
    class imap
    {
        std::unordered_map<KEY, VAL> storage{};
        std::unordered_map<KEY, int> reverse{};
        std::map          <int, KEY> forward{};
        int                          counter{};

        template<class IMAP>
        struct iter
        {
            using it_t = decltype(IMAP{}.forward.begin());
            using type = typename std::iterator_traits<it_t>::difference_type; //todo "typename" keyword is required by FreeBSD clang 11.0.1

            IMAP& buff;
            it_t  addr;

            iter(IMAP& buff, it_t&& addr)
              : buff{ buff },
                addr{ addr }
            { }

            auto  operator -  (type n)        const { return iter<IMAP>{ buff, addr - n };         }
            auto  operator +  (type n)        const { return iter<IMAP>{ buff, addr + n };         }
            auto  operator ++ (int)                 { return iter<IMAP>{ buff, addr++   };         }
            auto  operator -- (int)                 { return iter<IMAP>{ buff, addr--   };         }
            auto& operator ++ ()                    {                        ++addr; return *this; }
            auto& operator -- ()                    {                        --addr; return *this; }
            auto  operator -> ()                    { return buff.storage.find(addr->second);      }
            auto& operator *  ()                    { return *(this->operator->());                }
            auto  operator != (iter const& m) const { return addr != m.addr;                       }
            auto  operator == (iter const& m) const { return addr == m.addr;                       }
        };

    public:
        auto   begin()       { return iter<      imap>{ *this, forward.begin() }; }
        auto     end()       { return iter<      imap>{ *this, forward.end()   }; }
        auto   begin() const { return iter<const imap>{ *this, forward.begin() }; }
        auto     end() const { return iter<const imap>{ *this, forward.end()   }; }
        auto  length() const { return forward.size();                             }
        auto    size() const { return forward.size();                             }
        auto&   back()       { return storage[std::prev(forward.end()) ->second]; }
        auto&  front()       { return storage[          forward.begin()->second]; }
        //todo implement erase and friends
        // ...
        template<class K>
        auto erase(K&& key )
        {
            auto test = si32{ 0 };
            auto iter = storage.find(std::forward<K>(key));
            if (iter != storage.end())
            {
                test++;
                auto& my_key = iter->first;
                auto counter_it = reverse.find(my_key);
                auto number = counter_it->second;
                reverse.erase(counter_it);
                forward.erase(number);
                storage.erase(iter);
            }
            return test;
        }
        template<class K>
        auto contains(K&& key )
        {
            auto iter = storage.find(std::forward<K>(key));
            return iter != storage.end();
        }
        template<class K>
        auto& at(K&& key)
        {
            auto [iter, anew] = storage.try_emplace(std::forward<K>(key));
            if (anew)
            {
                auto& new_key = iter->first;
                forward[counter] = new_key;
                reverse[new_key] = counter;
                counter++;
            }

            return iter->second;
        }
        template<class K>
        auto& operator [] (K&& key) { return at(std::forward<K>(key)); }

        imap()
        { }
        imap(std::initializer_list<std::pair<KEY, VAL>> list)
        {
            for (auto& [key, val] : list) at(key) = val;
        }
    };
}
#endif // NETXS_HASH_HPP