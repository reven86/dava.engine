#include "BacktraceCorkscrewImpl.h"
#include "Platform/TemplateAndroid/ExternC/AndroidLayer.h"
namespace DAVA
{
MemoryMapCorkscrewInterface::MemoryMapCorkscrewInterface()
    : map_info(nullptr)
    , iterator(map_info)
{
    map_info = acquire_my_map_info_list();

    iterator = MemoryMapCorkscrewIterator(map_info);
}
MemoryMapCorkscrewInterface::~MemoryMapCorkscrewInterface()
{
    release_my_map_info_list(map_info);
}
MemoryMapIterator& MemoryMapCorkscrewInterface::GetIterator() const
{
    return iterator;
}
MemoryMapCorkscrewIterator::MemoryMapCorkscrewIterator(map_info_t* map_info)
{
    this->map_info = map_info;
    now = nullptr;
}
bool MemoryMapCorkscrewIterator::Next()
{
    if (now == nullptr)
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
const char* MemoryMapCorkscrewIterator::GetLib() const
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
                                          const char** libName, pointer_size* pc) const
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
    if (processMap != nullptr)
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
        BacktraceCorkscrewImpl* face = new BacktraceCorkscrewImpl();
        face->loaded = true;
        return face;
    }
    return nullptr;
}
void BacktraceCorkscrewImpl::BuildMemoryMap()
{
    if (loaded && processMap == nullptr)
    {
        processMap = new MemoryMapCorkscrewInterface();
    }
}
const MemoryMapInterface* BacktraceCorkscrewImpl::GetMemoryMap() const
{
    return processMap;
}
//handler safe function
void BacktraceCorkscrewImpl::Backtrace(Function<void(pointer_size)> onFrame,
                                       void* context, void* siginfo)
{
    if (processMap == nullptr)
    {
        BuildMemoryMap();
    }

    std::array<backtrace_frame_t, 256> frames =
    { 0 };
    if (context == nullptr && unwind_backtrace == nullptr)
    {
        //if context is null then we are NOT inside signal handler and so print a message
        LOGE("Backtracing with libcorkscrew outside signalhandler is not implimented");
        return;
    }
    ssize_t size = 0;
    if (context == nullptr)
    {
        size = unwind_backtrace_signal_arch((siginfo_t*)siginfo, context,
                                            processMap->GetMapInfo(), frames.data(), 0, 255);
    }
    else
    {
        size = unwind_backtrace(frames.data(), 0, 255);
    }

    for (auto& frame : frames)
    {
        onFrame(static_cast<pointer_size>(frame.absolute_pc));
    }
}
void BacktraceCorkscrewImpl::PrintableBacktrace(Function<void(pointer_size, const char* str)> onFrame,
                                                void* context, void* siginfo)
{
    LOGE("libCorkscrew not supported for printing any usefull stack trace");
}
} /* namespace DAVA */
