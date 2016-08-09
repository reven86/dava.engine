#pragma once
#include "../rhi_Type.h"

namespace rhi
{
namespace CommonImpl
{
struct Frame
{
    Handle sync = InvalidHandle;
    std::vector<Handle> pass;
    bool readyToExecute = false;
    uint32 frameNumber;
};

struct ImmediateCommand
{
    void* cmdData = nullptr;
    uint32 cmdCount;
    bool forceImmediate = false;
};
}

namespace DispatchPlatform
{
extern void (*InitContext)();
extern void (*AcquireContext)();
extern void (*ReleaseContext)();
extern void (*CheckSurface)();
extern void (*Suspend)();

extern void (*ProcessImmediateCommand)(CommonImpl::ImmediateCommand* command); //called from render thread

extern void (*InvalidateFrameCache)();
extern void (*ExecuteFrame)(CommonImpl::Frame&&); //should also handle command buffer sync here
extern void (*RejectFrame)(CommonImpl::Frame&&); //should also handle command buffer sync here

extern bool (*PresntBuffer)();
extern void (*ResetBlock)();
extern int test;
}
}