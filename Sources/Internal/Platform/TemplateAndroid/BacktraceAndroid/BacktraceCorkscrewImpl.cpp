/*
 * BacktraceCorkscrewImpl.cpp
 *
 *  Created on: Dec 20, 2014
 *      Author: l_sinyakov
 */

#include "BacktraceCorkscrewImpl.h"
#include "Platform/TemplateAndroid/ExternC/AndroidLayer.h"
namespace DAVA
{
MemoryMapCorkscrewInterface::MemoryMapCorkscrewInterface():map_info(NULL),iterator(map_info)
{
	
	map_info = acquire_my_map_info_list();
	
	iterator = MemoryMapCorkscrewIterator(map_info);
	
}
MemoryMapCorkscrewInterface::~MemoryMapCorkscrewInterface()
{
	
	release_my_map_info_list(map_info);
	
}
MemoryMapIterator & MemoryMapCorkscrewInterface::GetIterator() const
{
	return iterator;
}
MemoryMapCorkscrewIterator::MemoryMapCorkscrewIterator(map_info_t *map_info)
{
	this->map_info = map_info;
	now = NULL;
}
bool MemoryMapCorkscrewIterator::Next()
{
	if(now == NULL)
	{
		now = map_info;
		return now != NULL;
	}
	else
		now = now->next;
	return now != NULL;
}
void MemoryMapCorkscrewIterator::ToBegin()
{
	now = NULL;
}
const char * MemoryMapCorkscrewIterator::GetLib() const
{
	if (now != NULL)
		return now->name;
	return NULL;
}
pointer_size MemoryMapCorkscrewIterator::GetAddrStart() const
{
	if (now != NULL)
		return now->start;
	return 0;
}
pointer_size MemoryMapCorkscrewIterator::GetAddrEnd() const
{
	if (now != NULL)
		return now->end;
	return 0;
}
bool MemoryMapCorkscrewInterface::Resolve(pointer_size addr,
		const char ** libName, pointer_size * pc) const
{
	const map_info_t* mi = map_info;
	while (mi && !(addr >= mi->start && addr < mi->end))
	{
		mi = mi->next;
	}
	if (mi == NULL)
	{
		*libName = NULL;
		*pc = 0;
		return false;
	}

	*pc = addr - mi->start;
	*libName = mi->name;
	return false;
}
BacktraceCorkscrewImpl::BacktraceCorkscrewImpl()
{
	loaded = false;
	processMap = NULL;
}

BacktraceCorkscrewImpl::~BacktraceCorkscrewImpl()
{
	if(processMap != NULL)
	{
		delete processMap;
	}
	processMap = NULL;
	loaded = false;
}
BacktraceCorkscrewImpl* BacktraceCorkscrewImpl::Load()
{
	//initialize library
	if (DynLoadLibcorkscrew())
	{
		BacktraceCorkscrewImpl * face= new BacktraceCorkscrewImpl();
		face->loaded = true;
		return face;
	}

	return NULL;
}
void BacktraceCorkscrewImpl::BuildMemoryMap()
{
	if(loaded && processMap == NULL)
	{
		
		processMap = new MemoryMapCorkscrewInterface();
		
	}
}
const MemoryMapInterface * BacktraceCorkscrewImpl::GetMemoryMap() const
{
	return processMap;
}
//handler safe function
void BacktraceCorkscrewImpl::Backtrace(Function<void(pointer_size)> onFrame,
		void * context, void * siginfo)
{
	
	if(processMap == NULL)
		BuildMemoryMap();
	backtrace_frame_t frames[256] =
	{ 0 };
	if(context == NULL)
	{
		//if context is null then we are NOT inside signal handler and so print a message
		LOGE("Backtracing with libcorkscrew outside signalhandler is not implimented");
		return;
	}
	const ssize_t size = unwind_backtrace_signal_arch((siginfo_t*)siginfo, context,
			processMap->GetMapInfo(), frames, 0, 255);

	for (int i = 0; i < size; ++i)
	{

		onFrame(static_cast<pointer_size>(frames[i].absolute_pc));

	}
}

} /* namespace DAVA */
