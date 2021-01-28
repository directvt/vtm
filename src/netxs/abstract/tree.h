// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_TREE_H
#define NETXS_TREE_H

#include <vector>

namespace netxs::generics
{
	template<class IN, class OUT, class FUNC = void (*)(IN&, OUT&)>
	struct tree 
		: public std::vector<tree<IN, OUT, FUNC>>
	{
		using bulk = std::vector<tree>;
		using hndl = FUNC;

		hndl proc;
		bool sure;
		bool rise;

		tree(bool reset_level_after_exec = true)
			:	proc (nullptr),
				sure (false),
				rise (reset_level_after_exec)
		{}

		tree(hndl func )
			:	proc (func), 
				sure (true),
				rise (false)
		{}

		auto& resize(size_t newsize)
		{
			proc = nullptr;
			sure = true;
			bulk::resize(newsize);
			return *this;
		}
		template<class F>
		void operator = (F func)
		{
			sure = true;
			proc = func;
		}
		operator bool () const
		{
			return sure;
		}

		void execute(IN& queue, OUT& story) const
		{
			auto base = this;
			auto last = base;

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
							if (rise) last = base;
						}
						else
						{
							last = &next;
						}
					}
					else break;
				}
				else break;
			}
		}

		void execute(size_t firstcmd, IN& queue, OUT& story) const
		{
			auto base = this;
			auto last = base;

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
									if (rise) break;//todo revise ansi::parser::proceed(x,y,z)
									//if (rise) last = base;
								}
								else
								{
									last = &next;
								}
							}
							else break;
						}
						else break;
					}
				}
			}
		}

		void execute(size_t alonecmd, OUT& story) const
		{
			auto& queue = IN::fake();
			
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
}

#endif // NETXS_TREE_H