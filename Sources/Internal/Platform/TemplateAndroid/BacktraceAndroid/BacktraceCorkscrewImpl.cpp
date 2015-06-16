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


#include "BacktraceCorkscrewImpl.h"
#include "Platform/TemplateAndroid/ExternC/AndroidLayer.h"
namespace DAVA
{
MemoryMapCorkscrewInterface::MemoryMapCorkscrewInterface():map_info(nullptr),iterator(map_info)
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
    now = nullptr;
}
bool MemoryMapCorkscrewIterator::Next()
{
    if(now == nullptr)
    {
        now = map_info;
        return now != nullptr;
    }
    else
	{
        now = now->next;
	}
    return now != nullptr;
}
void MemoryMapCorkscrewIterator::ToBegin()
{
    now = nullptr;
}
const char * MemoryMapCorkscrewIterator::GetLib() const
{
    if (now != nullptr)
    {
        return now->name;
    }
    return nullptr;
}
pointer_size MemoryMapCorkscrewIterator::GetAddrStart() const
{
    if (now != nullptr)
    {
        return now->start;
    }
    return 0;
}
pointer_size MemoryMapCorkscrewIterator::GetAddrEnd() const
{
    if (now != nullptr)
    {
        return now->end;
    }
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
    if (mi == nullptr)
    {
        *libName = nullptr;
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
    processMap = nullptr;
}

BacktraceCorkscrewImpl::~BacktraceCorkscrewImpl()
{
    if(processMap != nullptr)
    {
        delete processMap;
    }
    processMap = nullptr;
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
    return nullptr;
}
void BacktraceCorkscrewImpl::BuildMemoryMap()
{
    if(loaded && processMap == nullptr)
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
    
    if(processMap == nullptr)
    {
        BuildMemoryMap();
    }
    
    std::array<backtrace_frame_t,256> frames =
    { 0 };
    if(context == nullptr && unwind_backtrace == nullptr)
    {
        //if context is null then we are NOT inside signal handler and so print a message
        LOGE("Backtracing with libcorkscrew outside signalhandler is not implimented");
        return;
    }
    ssize_t size = 0;
    if(context == nullptr)
    {
        size = unwind_backtrace_signal_arch((siginfo_t*)siginfo, context,
                processMap->GetMapInfo(), frames.data(), 0, 255);
    }
    else
    {
        size = unwind_backtrace(frames.data(), 0, 255);
    }

    for (auto & frame : frames)
    {
        onFrame(static_cast<pointer_size>(frame.absolute_pc));
    }
}
void BacktraceCorkscrewImpl::PrintableBacktrace(Function<void (pointer_size,const char * str)> onFrame, 
        void * context  , void * siginfo )
{
    LOGE("libCorkscrew not supported for printing any usefull stack trace");
}
} /* namespace DAVA */
