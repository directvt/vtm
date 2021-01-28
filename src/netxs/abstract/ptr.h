// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_PTR_H
#define NETXS_PTR_H

#include <memory>

namespace netxs
{
	// drop it (Windows API)
	//struct intptr
	//{
	//	int * val;
	//
	//	template<typename T>
	//	operator T    () { return reinterpret_cast<T>	(val); }
	//	operator int  () { return reinterpret_cast<int> (val); }
	//	operator bool () { return val; }
	//
	//	template<class T>
	//	intptr(T val) : val((int*)(val)) { }
	//};

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
		{}
	};

	template<class T>
	using sptr = std::shared_ptr<T>;

	template<class T>
	using wptr = std::  weak_ptr<T>;

	template<class T>
	using uptr = std::unique_ptr<T>;

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

#endif // NETXS_PTR_H