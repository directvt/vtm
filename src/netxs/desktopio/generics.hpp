// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#include "intmath.hpp"

#include <type_traits>
#include <list>
#include <vector>
#include <map>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <thread>
#include <functional>

namespace netxs::generics
{
    // generics: .
    template<class Item>
    class fifo
    {
        Item * peak;
        Item * tail;
        Item * item;
        size_t size;
        Item   zero;

        // The second to last bit in the arg to mark the arg as a subparameter
        // In section 4.3.3.2 of EK-VT520-RM:
        //    “any parameter greater than 9 999 (decimal) is set to 9 999 (decimal)”.
        // In the DECSR (Secure Reset) - from 0 to 16 383 (decimal).
        // Our maximum for Item=int32_t is +/- 1 073 741 823 (wo two last bits)
        static constexpr auto subbit = unsigned{ 1 << (std::numeric_limits<Item>::digits - 2) };
        static constexpr auto sigbit = unsigned{ 1 << (std::numeric_limits<Item>::digits - 1) };

    public:
        static constexpr auto skip = unsigned{ 0x3fff'ffff };
        static inline bool issub(Item const& value) { return (value & subbit) != (value & sigbit) >> 1; }
        static inline auto desub(Item const& value) { return static_cast<Item>((value & ~subbit) | (value & sigbit) >> 1); }
        static inline auto insub(Item const& value) { return static_cast<Item>((value & ~subbit) | ((value & sigbit) ^ sigbit) >> 1); }
        static inline auto isdef(Item const& value) { return (value & fifo::skip) == fifo::skip; }

        static auto& fake() { static fifo empty; return empty; }

        constexpr
        fifo()
            : peak {0},
              tail {0},
              item {0},
              size {0},
              zero { }
        { }

        constexpr
        fifo( Item* data,  size_t size)
            : peak{ data + size},
              tail{ data },
              item{ data },
              size{ 0    },
              zero{      }
        { }

        template<bool IsSub = !true>
        constexpr
        void push(Item value)
        {
            if (tail != peak)
            {
                size++;
                *tail++ = IsSub ? insub(value)
                                : value;
            }
        }
        constexpr
        void remove_prefix(size_t n)
        {
            n = std::min(n, size);
            size -= n;
            item += n;
        }
        constexpr
        void pop_front()
        {
            if (size)
            {
                size--;
                item++;
            }
        }
        constexpr operator bool () const { return size; }
        constexpr auto length() const { return size; }
        constexpr Item front(Item const& dflt = {})
        {
            if (size)
            {
                auto value = *item;
                return isdef(value) ? issub(value) ? insub(dflt) : dflt
                                    : value;
            }
            else return dflt;
        }
        constexpr
        Item operator () (Item const& dflt = {})
        {
            if (size)
            {
                size--;
                auto result = *item++;
                return isdef(result) ? dflt :
                       issub(result) ? desub(result)
                                     : result;
            }
            else return dflt;
        }
        constexpr
        void settop(Item value)
        {
            if (size) *item = value;
        }
        constexpr
        Item rawarg(Item const& dflt = {})
        {
            if (size)
            {
                size--;
                auto result = *item++;
                return result;
            }
            else return dflt;
        }
        constexpr
        Item subarg(Item const& dflt = {})
        {
            if (size)
            {
                auto result = *item;
                if (issub(result))
                {
                    size--;
                    item++;
                    return isdef(result) ? dflt : desub(result);
                }
                else return dflt;
            }
            else return dflt;
        }
    };

    // generics: .
    template<class Item, size_t Size>
    class bank
        : public fifo<Item>
    {
        using fifo = netxs::generics::fifo<Item>;

        Item data[Size];

    public:
        constexpr
        bank()
            : fifo(data, Size)
        { }

        constexpr
        bank(Item value)
            : fifo(data, Size)
        {
            fifo::push(value);
        }
    };

    // generics: .
    struct null_deleter
    {
        void operator () (void const*) const
        { }
    };

    // generics: .
    template<class T>
    class mt_queue
    {
        std::mutex              d_mutex;
        std::condition_variable d_condition;
        std::deque<T>           d_queue;

    public:
        void push(T const& value)
        {
            auto guard = std::lock_guard{ d_mutex };
            d_queue.push_front(value);
            d_condition.notify_one();
        }
        T pop()
        {
            auto lock = std::unique_lock{ d_mutex };
            d_condition.wait(lock, [this] { return !d_queue.empty(); });
            T rc(std::move(d_queue.back()));
            d_queue.pop_back();
            return rc;
        }
        bool try_pop(T& v, std::chrono::milliseconds timeout)
        {
            auto lock = std::unique_lock{ d_mutex };
            if (!d_condition.wait_for(lock, timeout, [this] { return !d_queue.empty(); }))
            {
                return !true;
            }
            v = d_queue.back();
            d_queue.pop_back();
            return true;
        }
        size_t size()
        {
            auto guard = std::lock_guard{ d_mutex };
            return d_queue.size();
        }
        void clear()
        {
            auto guard = std::lock_guard{ d_mutex };
            d_queue.clear();
        }

        mt_queue() { }
        mt_queue(mt_queue<T>&& x)
        {
            auto guard = std::lock_guard{ d_mutex };
            d_queue = std::move(x.d_queue);
        }
        mt_queue<T>& operator = (mt_queue<T>&& x)
        {
            //todo *this is not a MT safe (only x.d_mutex is locked)
            auto guard = std::lock_guard{ d_mutex };
            d_queue = std::move(x.d_queue);
            return *this;
        }
    };

    // generics: Separate thread for executing deferred tasks.
    template<class T>
    struct jobs
    {
        using token = T;
        using func = std::function<void(token&)>;
        using item = std::pair<token, func>;

        std::mutex              mutex;
        std::condition_variable synch;
        std::list<item>         queue;
        bool                    alive;
        std::thread             agent;

        template<class P>
        void cancel(P&& deactivate)
        {
            auto guard = std::unique_lock{ mutex };
            for (auto& job : queue)
            {
                auto& token = job.first;
                deactivate(token);
            }
        }
        void worker()
        {
            auto guard = std::unique_lock{ mutex };
            while (alive)
            {
                if (queue.empty() /* Not empty at startup */) synch.wait(guard);
                while (queue.size())
                {
                    auto& [token, proc] = queue.front();
                    guard.unlock();
                    proc(token);
                    guard.lock();
                    queue.pop_front();
                }
            }
        }

        jobs()
            : alive{ true },
              agent{ &jobs::worker, this }
        { }
       ~jobs()
        {
            stop();
        }
        template<class TT, class P>
        void add(TT&& token, P&& proc)
        {
            auto guard = std::lock_guard{ mutex };
            if (alive)
            {
                if constexpr (std::is_copy_constructible_v<P>)
                {
                    queue.emplace_back(std::forward<TT>(token), std::forward<P>(proc));
                }
                else
                {
                    //todo issue with MSVC: Generalized lambda capture does't work.
                    auto proxy = std::make_shared<std::decay_t<P>>(std::forward<P>(proc));
                    queue.emplace_back(std::forward<TT>(token), [proxy](auto&&... args)->decltype(auto)
                    {
                        return (*proxy)(decltype(args)(args)...);
                    });
                }
            }
            synch.notify_one();
        }
        void stop()
        {
            auto guard = std::unique_lock{ mutex };
            if (alive)
            {
                alive = faux;
                synch.notify_one();
                guard.unlock();
                agent.join();
            }
        }
    };

    // generics: .
    template<class vect, bool UseUndock = faux>
    struct ring
    {
        using type = typename vect::value_type;

        si32 step; // ring: Unlimited buffer increment step (zero for fixed size buffer).
        si32 head; // ring: Front index.
        si32 tail; // ring: Back index.
        si32 peak; // ring: Limit of the ring buffer.
        vect buff; // ring: Inner container.
        si32 size; // ring: Elements count.
        si32 cart; // ring: Active item position.
        si32 mxsz; // ring: Max unlimited buffer size.

        void inc(si32& a) const {  if (++a == peak) a = 0;        }
        void dec(si32& a) const {  if (--a < 0    ) a = peak - 1; }
        auto mod(si32  a) const { return a < 0  ? ++a % peak - 1 + peak
                                                :   a % peak;     }
        auto dst(si32  a, si32 b) const
                                { return b < a ? b - a + peak
                                               : b - a;           }
        template<class Ring>
        struct iter
        {
            Ring& buff;
            si32  addr;
            iter(Ring& buff, si32 addr)
              : buff{ buff },
                addr{ addr }
            { }
            auto  operator -  (si32 n)        const {      return iter<Ring>{ buff, buff.mod(addr - n) };                 }
            auto  operator +  (si32 n)        const {      return iter<Ring>{ buff, buff.mod(addr + n) };                 }
            auto  operator ++ (int)                 { auto temp = iter<Ring>{ buff, addr }; buff.inc(addr); return temp;  }
            auto  operator -- (int)                 { auto temp = iter<Ring>{ buff, addr }; buff.dec(addr); return temp;  }
            auto& operator ++ ()                    {                                       buff.inc(addr); return *this; }
            auto& operator -- ()                    {                                       buff.dec(addr); return *this; }
            auto& operator *  ()                    { return buff.buff[addr];                                             }
            auto  operator -> ()                    { return buff.buff.begin() + addr;                                    }
            auto  operator != (iter const& m) const { return addr != m.addr;                                              }
            auto  operator == (iter const& m) const { return addr == m.addr;                                              }
        };

        ring(si32 ring_size, si32 grow_by = 0)
            : step{ grow_by                      },
              head{ 0                            },
              tail{ ring_size ? ring_size : step },
              peak{ tail + 1                     },
              buff( peak                         ), // Rounded brackets! Not curly! In oreder to call T::ctor().
              size{ 0                            },
              cart{ 0                            },
              mxsz{ maxsi32 - step               }
        { }

        virtual void undock_base_front(type&) { };
        virtual void undock_base_back (type&) { };

        auto  current_it()         { return iter<      ring>{ *this, cart };          }
        auto  begin()              { return iter<      ring>{ *this, head };          }
        auto    end()              { return iter<      ring>{ *this, mod(tail + 1) }; }
        auto  begin() const        { return iter<const ring>{ *this, head };          }
        auto    end() const        { return iter<const ring>{ *this, mod(tail + 1) }; }
        auto& length() const       { return size;                }
        auto&  back()              { return buff[tail];          }
        auto&  back() const        { return buff[tail];          }
        auto& front()              { return buff[head];          }
        auto& front() const        { return buff[head];          }
        auto& current     ()       { return buff[cart];          }
        auto& operator  * ()       { return buff[cart];          }
        auto  operator -> ()       { return buff.begin() + cart; }
        auto&          at (si32 i) { assert(i >= 0 && i < size); return buff[mod(head + i)]; }
        auto& operator [] (si32 i) { return at(i);               }
        auto  index() const        { return dst(head, cart);     }
        void  index(si32 i)        { assert((i > 0 && i < size) || i == 0); cart = mod(head + i); }
        void  prev()               { dec(cart); test();          }
        void  next()               { inc(cart); test();          }

    private:
        void  test()
        {
            assert((head == tail &&  cart == head)
                || (head <  tail &&  cart >= head && cart <= tail)
                || (head >  tail && (cart >= head && cart >= tail
                                  || cart <= head && cart <= tail)));
        }
        auto  full()
        {
            if (size == peak - 1)
            {
                if (step && peak < mxsz) resize(size + step, step);
                else                     return true;
            }
            return faux;
        }
        template<bool UseBack = faux>
        inline void undock_front()
        {
            auto& item = front();
            if constexpr (UseUndock)
            {
                if constexpr (UseBack) undock_base_back (item);
                else                   undock_base_front(item);
            }
            item = type{};
            if (cart == head) inc(head), cart = head;
            else              inc(head);
        }
        inline void undock_back()
        {
            auto& item = back();
            if constexpr (UseUndock) undock_base_back(item);
            item = type{};
            if (cart == tail) dec(tail), cart = tail;
            else              dec(tail);
        }
        // ring: Insert an item before the specified position. Pop front when full. Return an iterator pointing to the new item.
        template<class ...Args>
        auto insert_impl(si32 at, Args&&... args) // Pop front always if ring is full.
        {
            if (at == 0)
            {
                push_front(std::forward<Args>(args)...);
                return begin();
            }
            else
            if (at == size)
            {
                push_back(std::forward<Args>(args)...);
                return begin() + size - 1;
            }

            auto d = at;
            if (size >> 1 > d)
            {
                auto it_1 = begin();
                auto it_2 = it_1 + d;
                if (full())
                {
                    auto& item = front();
                    if constexpr (UseUndock) undock_base_front(item);
                    item = type(std::forward<Args>(args)...);
                    ++it_1;
                }
                else
                {
                    ++size;
                    dec(head);
                    auto& item = front();
                    item = type(std::forward<Args>(args)...);
                }
                swap_block<true>(it_1, it_2, begin());
                --it_2;
                return it_2;
            }
            else
            {
                auto dest = end();
                auto it_1 = dest - 1;
                d = size - d;
                auto it_2 = it_1 - d;
                if (full())
                {
                    auto& item = front();
                    if constexpr (UseUndock) undock_base_front(item);
                    item = type{};
                    if (cart == head) inc(head), cart = head;
                    else              inc(head);
                }
                else ++size;
                inc(tail);
                back() = type(std::forward<Args>(args)...);
                swap_block<faux>(it_1, it_2, end() - 1);
                ++it_2;
                return it_2;
            }
        }

    public:
        template<class ...Args>
        type& push_back(Args&&... args)
        {
            if (full()) undock_front();
            else        ++size;
            inc(tail);
            auto& item = back();
            item = type(std::forward<Args>(args)...);
            return item;
        }
        template<class ...Args>
        type& push_front(Args&&... args)
        {
            if (full()) undock_back();
            else        ++size;
            dec(head);
            auto& item = front();
            item = type(std::forward<Args>(args)...);
            return item;
        }
        template<bool UseBack = faux>
        void pop_front() { undock_front<UseBack>(); --size; }
        void pop_back () { undock_back();           --size; }
        // ring: Insert an item before the specified position. Pop front when full. Return an iterator pointing to the new item.
        template<class ...Args>
        auto insert(si32 at, Args&&... args) // Pop front always if ring is full.
        {
            assert(at >= 0 && at <= size);
            auto temp = index();
            if (full())
            {
                if (temp > 0 && temp < at) temp--;
            }
            else
            {
                if (temp >= at) temp++;
            }
            auto iter = insert_impl(at, std::forward<Args>(args)...);
            index(temp);
            return iter;
        }
        auto remove(si32 at, si32 n)
        {
            assert(at >= 0 && at < size);

            auto max = size - at;
            if (n > max) n = max;

            auto vol = n;
            auto tmp = index();
                 if (tmp >= at + n) tmp -= n;
            else if (tmp >= at    ) tmp = std::max(at - 1, 0);

            auto top_block = at;
            auto btm_block = max - n;
            if (btm_block > top_block)
            {
                auto tail = begin() - 1;
                auto head = tail + top_block;
                netxs::swap_block<faux>(head, tail, head + n);
                static constexpr auto UseBack = true;
                while (n-- > 0) pop_front<UseBack>();
            }
            else
            {
                auto tail = end();
                auto head = tail - btm_block;
                netxs::swap_block<true>(head, tail, head - n);
                while (n-- > 0) pop_back();
            }
            index(tmp); // Restore current item selector.
            return vol;
        }
        void clear()
        {
            if constexpr (UseUndock) while (size) pop_back(); //todo undock?
            else                     size = 0;
            cart = 0;
            head = 0;
            tail = peak - 1;
        }
        template<bool BottomAnchored = true>
        void resize(si32 new_size, si32 grow_by = 0)
        {
            if (new_size > 0)
            {
                if constexpr (BottomAnchored)
                {
                    if (size > new_size)
                    {
                        //todo optimize for !UseUndock
                        do
                        {
                            if constexpr (UseUndock) undock_base_front(front());
                            inc(head);
                        }
                        while (--size != new_size);
                    }
                    cart = std::max(0, size - 1 - dst(cart, tail));
                }
                else // TOP_ANCHORED
                {
                    if (size > new_size)
                    {
                        //todo optimize for !UseUndock
                        do
                        {
                            if constexpr (UseUndock) undock_base_back(back());
                            dec(tail);
                        }
                        while (--size != new_size);
                    }
                    cart = std::min(size - 1, dst(head, cart));
                }
                vect temp;
                temp.reserve(++new_size);
                auto i = size;
                while (i--)
                {
                    temp.emplace_back(std::move(front()));
                    inc(head);
                }
                temp.resize(new_size);
                std::swap(buff, temp);
                peak = new_size;
                head = 0;
                tail = size ? size - 1 : peak - 1;
                step = grow_by;
            }
            else step = grow_by;
        }
        template<class P>
        void for_each(si32 from, si32 upto, P proc)
        {
            auto head = begin() + from;
            auto tail = begin() + upto;
            if constexpr (std::is_same_v< decltype(proc(*head)), bool >)
            {
                     if (from < upto) while (proc(*head) && ++head != tail);
                else if (from > upto) while (proc(*head) && --head != tail);
            }
            else
            {
                     if (from < upto) do { proc(*head); } while (++head != tail);
                else if (from > upto) do { proc(*head); } while (--head != tail);
            }
        }
    };

    // generics: .
    template<class In, class Out, class Func = void (*)(In&, Out&)>
    struct tree
        : public std::vector<tree<In, Out, Func>>
    {
        using bulk = std::vector<tree>;
        using hndl = Func;

        hndl proc = nullptr;
        bool sure = faux;
        bool stop = true;

        template<class F>
        void operator = (F func)
        {
            if constexpr (std::is_same_v<F, std::nullptr_t>) sure = faux;
            else                                             sure = true;
            proc = func;
        }
        auto& resize(size_t newsize)
        {
            proc = nullptr;
            sure = true;
            bulk::resize(newsize);
            return *this;
        }
        template<bool NoMultiArg = faux>
        void enable_multi_arg()
        {
            for (auto& rec : *this) rec.stop = NoMultiArg;
        }
        operator bool () const
        {
            return sure;
        }

        void execute(In& queue, Out& story) const
        {
            auto last = this;
            while (queue)
            {
                auto task = queue.front();
                if (task >= 0 && task < last->size())
                {
                    if (auto const& next = last->at(task))
                    {
                        queue.pop_front();
                        if (next.proc)
                        {
                            next.proc(queue, story);
                            if (next.stop) break;
                        }
                        else last = &next;
                    }
                    else
                    {
                        if (last->stop) break;
                        else queue.pop_front();
                    }
                }
                else
                {
                    if (last->stop) break;
                    else queue.pop_front();
                }
            }
        }

        void execute(size_t firstcmd, In& queue, Out& story) const
        {
            auto last = this;
            if (auto const& next = last->at(firstcmd))
            {
                if (next.proc)
                {
                    next.proc(queue, story);
                }
                else
                {
                    last = &next;
                    while (queue)
                    {
                        auto task = queue.front();
                        if (task >= 0 && task < last->size())
                        {
                            if (auto const& next = last->at(task))
                            {
                                queue.pop_front();
                                if (next.proc)
                                {
                                    next.proc(queue, story);
                                    break;
                                }
                                else last = &next;
                            }
                            else
                            {
                                if (last->stop) break;
                                else queue.pop_front();
                            }
                        }
                        else
                        {
                            if (last->stop) break;
                            else queue.pop_front();
                        }
                    }
                }
            }
        }
        // Exec without parameters.
        void execute(size_t alonecmd, Out& story) const
        {
            auto& queue = In::fake();
            if (alonecmd >= 0 && alonecmd < this->size())
            {
                if (auto const& next = this->at(alonecmd))
                {
                    if (next.proc)
                    {
                        next.proc(queue, story);
                    }
                    else
                    {
                        if (auto const& last = next[0])
                        {
                            if (last.proc)
                            {
                                last.proc(queue, story);
                            }
                        }
                    }
                }
            }
        }
    };

    // generics: Map that keeps the insertion order.
    template<class Key, class Val>
    class imap
    {
        std::unordered_map<Key, Val> storage{};
        std::unordered_map<Key, int> reverse{};
        std::map          <int, Key> forward{};
        int                          counter{};

        template<class IMAP>
        struct iter
        {
            using it_t = decltype(IMAP{}.forward.begin());
            using type = typename std::iterator_traits<it_t>::difference_type; //todo "typename" keyword is required by clang 13.0.0

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
        void   clear()
        {
            counter = {};
            storage.clear();
            reverse.clear();
            forward.clear();
        }
        template<class K>
        auto erase(K&& key )
        {
            auto storage_it = storage.find(std::forward<K>(key));
            if (storage_it != storage.end())
            {
                auto& key_value = storage_it->first;
                auto reverse_it = reverse.find(key_value);
                auto forward_it = reverse_it->second;
                reverse.erase(reverse_it);
                forward.erase(forward_it);
                storage.erase(storage_it);
                return true;
            }
            return faux;
        }
        template<class K>
        auto contains(K&& key )
        {
            return storage.contains(std::forward<K>(key));
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
        imap(std::initializer_list<std::pair<Key, Val>> list)
        {
            for (auto& [key, val] : list) at(key) = val;
        }
    };
}

// generics: Map helpers.
namespace netxs
{
    template<class M, class K>
    bool on_key(M const& map, K const& key)
    {
        const auto it = map.find(key);
        return it != map.end();
    }

    // do it in place
    //template<class M, class K>
    //auto on_key_get(const M& map, const K& key)
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
    auto& map_or(Map& map, Key const& key, FBKey const& fallback) // Note: The map must contain a fallback.
    {
        auto const it = map.find(key);
        return it == map.end() ? map.at(fallback)
                               : it->second;
    }
}