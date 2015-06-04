/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include <sys/types.h>
#include <unistd.h>
#include "BacktraceUnwindImpl.h"
#include "AndroidCrashUtility.h"
#if defined(__arm__) && defined(CRASH_HANDLER_CUSTOMSIGNALS)
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
    *libName = nullptr;
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

    return now != end;
}
void MemoryMapUnwindIterator::ToBegin()
{
    now = end;
}
const char * MemoryMapUnwindIterator::GetLib() const
{
    if(now == end)
    {
        return nullptr;
    }
    return now->path;
}
pointer_size MemoryMapUnwindIterator::GetAddrStart() const
{
    if(now == end)
    {
        return 0;
    }
    return now->start;
}
pointer_size MemoryMapUnwindIterator::GetAddrEnd() const
{
    if(now == end)
    {
        return 0;
    }
    return now->end;
}
BacktraceUnwindImpl::BacktraceUnwindImpl()
    :processMap{nullptr},loaded(false)
{
}

BacktraceUnwindImpl::~BacktraceUnwindImpl()
{
    if (processMap != nullptr)
    {
        delete processMap;
    }
    processMap = nullptr;
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

    return nullptr;
}
void BacktraceUnwindImpl::BuildMemoryMap()
{
    if(!loaded || processMap != nullptr) 
    {
        return;
    }
    processMap = new MemoryMapUnwind();
}
const MemoryMapInterface * BacktraceUnwindImpl::GetMemoryMap() const
{
    if(!loaded) 
    {
        return nullptr;
    }
    return processMap;
}

//handler safe function
void BacktraceUnwindImpl::Backtrace(Function<void(pointer_size)> onFrame,  void * context , void * siginfo )
{
     BacktraceInternal(onFrame,NULL,context,siginfo);
}
void BacktraceUnwindImpl::PrintableBacktrace(Function<void (pointer_size,const char * str)> onFrame,  void * context , void * siginfo)
{
    BacktraceInternal(NULL,onFrame,context,siginfo);
}
void BacktraceUnwindImpl::BacktraceInternal(Function<void(pointer_size)> onFrame,
            Function<void (pointer_size,const char * str)> onFrameName, 
            void * context  , void * siginfo )
{
    PreBacktrace();
    unw_context_t uctx;
    if (context != nullptr)
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
        if(onFrame != NULL)
        {
            onFrame(static_cast<pointer_size>(ip));
        }
        if(onFrameName != NULL)
        {
            std::array<char,256> procName;
            unw_word_t frameOffset;
            unw_get_proc_name(&cursor,procName.data(),procName.size(),&frameOffset);
            onFrameName(static_cast<pointer_size>(ip),procName.data());
            
        }
        counter++;

    } while (unw_step(&cursor) > 0);
}
void BacktraceUnwindImpl::PreBacktrace()
{
    if(!loaded)
    {
        Load();
    }
    if(!loaded)
    {
        return;
    }
    if(processMap == nullptr)
    {
        BuildMemoryMap();
    }
}
} /* namespace DAVA */
#endif /* __arm__*/
