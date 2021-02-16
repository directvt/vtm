// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_FIFO_H
#define NETXS_FIFO_H

#include <limits>

namespace netxs::generics
{
	template<class ITEM>
	class fifo
	{
		ITEM *	peak;
		ITEM *	tail;
		ITEM *	item;
		size_t	size;
		ITEM	zero;

		/// The second to last bit in the arg to mark the arg as a subparameter
		/// In section 4.3.3.2 of EK-VT520-RM:
		///		“any parameter greater than 9 999 (decimal) is set to 9 999 (decimal)”. 
		/// In the DECSR (Secure Reset) - from 0 to 16 383 (decimal).
		/// Our maximum for ITEM=int32_t is +/- 1 073 741 823 (wo two last bits)
		constexpr static unsigned subbit = 1 << (std::numeric_limits<ITEM>::digits - 2);
		
	public:
		static inline bool issub(ITEM const& value) { return value & fifo::subbit; }
		static inline auto desub(ITEM const& value) { return static_cast<ITEM>(value & ~fifo::subbit); }

		static auto& fake() { static fifo empty; return empty; }
		
		constexpr
		fifo()
			:	peak {0},
				tail {0},
				item {0},
				size {0},
				zero { }
		{ }

		constexpr
		fifo(ITEM* data, size_t SIZE)
			:	peak {data + SIZE},
				tail {data},
				item {data},
				size {0},
				zero { }
		{ }
		
		//constexpr
		//void clear()
		//{
		//	if (size)
		//	{
		//		size = 0;
		//		//item += size;
		//	}
		//}
		template<bool ISSUB = false>
		constexpr
		void push(ITEM value)
		{
			if (tail != peak)
			{
				size++;
				*tail++ = ISSUB ? value | subbit
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
		constexpr size_t   length ()    const             { return size;                }
		constexpr operator bool   ()    const             { return size;                }
		constexpr ITEM     front  (ITEM const& dflt = {}) { return size ? *item : dflt; }
		constexpr 
		ITEM operator() (ITEM const& dflt = {})
		{
			if (size)
			{
				size--;
				auto result = *item++;
				return issub(result) ? desub(result)
				                     : result;
			}
			else return dflt;
		}
		constexpr
		void settop(ITEM value)
		{
			if (size) *item = value;
		}
		constexpr
		ITEM subarg(ITEM const& dflt = {})
		{
			if (size)
			{
				auto result = *item;
				if (issub(result))
				{
					size--;
					item++;
					return desub(result);
				}
				else return dflt;
			}
			else return dflt;
		}
	};

	template<class ITEM, size_t SIZE>
	class bank
		: public fifo<ITEM>
	{
		using fifo = netxs::generics::fifo<ITEM>;
		
		ITEM data[SIZE];

	public:
		constexpr
		bank()
			: fifo(data, SIZE)
		{ }

		constexpr
		bank(ITEM value)
			: fifo(data, SIZE)
		{
			fifo::push(value);
		}
	};
}

#endif // NETXS_FIFO_H