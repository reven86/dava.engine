/*
 * BacktraceUnwindImpl.cpp
 *
 *  Created on: Dec 19, 2014
 *      Author: l_sinyakov
 */

#include <sys/types.h>
#include <unistd.h>
#include "BacktraceUnwindImpl.h"
#include "AndroidCrashUtility.h"
#if defined(__arm__)
namespace DAVA
{
MemoryMapUnwind::MemoryMapUnwind():
		iterator(memoryMap.begin(),memoryMap.end())
{

	//loop through all info and ad them to the list

	unw_map_cursor_create(&mapCursor, getpid());

	// must be called even after create
	unw_map_cursor_reset(&mapCursor);
	unw_map_t map;
	//int log = unw_map_local_cursor_valid(&mapCursor);
	while (unw_map_cursor_get_next(&mapCursor, &map) > 0)
	{
		memoryMap.push_back(map);

	}
	iterator = MemoryMapUnwindIterator(memoryMap.begin(),memoryMap.end());

}
MemoryMapUnwind::~MemoryMapUnwind()
{
	unw_map_cursor_destroy(&mapCursor);

}
MemoryMapIterator & MemoryMapUnwind::GetIterator() const
{
	return iterator;
}
bool MemoryMapUnwind::Resolve(pointer_size addr, const char **libName,
		pointer_size *addresInLib) const
{
	for (Vector<unw_map_t>::const_iterator map = memoryMap.begin();
			map != memoryMap.end(); ++map)
	{
		if (map->start < addr && map->end > addr)
		{
			*libName = map->path;
			*addresInLib = addr - map->start;
			return true;
		}
	}
	*addresInLib = addr;
	*libName = NULL;
	return false;
}
MemoryMapUnwindIterator::MemoryMapUnwindIterator(Vector<unw_map_t>::const_iterator begin,Vector<unw_map_t>::const_iterator end):
		begin(begin),end(end),now(begin)
{

}
bool MemoryMapUnwindIterator::Next()
{
	if(now == end)
	{
		now = begin;
		return true;
	}
	++now;

	if(now != end)
		return true;
	return false;
}
void MemoryMapUnwindIterator::ToBegin()
{
	now = end;
}
const char * MemoryMapUnwindIterator::GetLib() const
{
	if(now == end)
		return NULL;
	return now->path;
}
pointer_size MemoryMapUnwindIterator::GetAddrStart() const
{
	if(now == end)
		return 0;
	return now->start;
}
pointer_size MemoryMapUnwindIterator::GetAddrEnd() const
{
	if(now == end)
		return 0;
	return now->end;
}
BacktraceUnwindImpl::BacktraceUnwindImpl()
{
	// TODO Auto-generated constructor stub
	processMap = NULL;
	loaded = false;
}

BacktraceUnwindImpl::~BacktraceUnwindImpl()
{
	if (processMap != NULL)
		delete processMap;
	processMap = NULL;
	loaded = false;
}
BacktraceUnwindImpl* BacktraceUnwindImpl::Load()
{
	//initialize library
	if (DynLoadLibunwind())
	{
		BacktraceUnwindImpl * face = new BacktraceUnwindImpl();
		face->loaded = true;
		return face;
	}

	return NULL;
}
void BacktraceUnwindImpl::BuildMemoryMap()
{
	if(!loaded) return;
	processMap = new MemoryMapUnwind();
}
const MemoryMapInterface * BacktraceUnwindImpl::GetMemoryMap() const
{
	if(!loaded) return NULL;
	return processMap;
}

//handler safe function
void BacktraceUnwindImpl::Backtrace(Function<void(pointer_size)> onFrame,  void * context , void * siginfo )
{
	if(!loaded)
		Load();
	if(!loaded)
		return;
	if(processMap == NULL)
		BuildMemoryMap();
	//custom format context for libunwind
	unw_context_t uctx;
	if (context != NULL)
	{
		ConvertContextARM((ucontext_t*) context, &uctx);

	}
	else
	{
		unw_tdep_getcontext(&uctx);
	}
	unw_cursor_t cursor;
	unw_word_t ip, sp;

	int counter = 0;
	unw_init_local(&cursor, &uctx);
	do
	{
		if (unw_is_signal_frame(&cursor) > 0)
		{
			unw_handle_signal_frame(&cursor);
		}

		unw_get_reg(&cursor, UNW_REG_IP, &ip);

		onFrame(static_cast<pointer_size>(ip));
		counter++;

	} while (unw_step(&cursor) > 0);

}

} /* namespace DAVA */
#endif /* __arm__*/