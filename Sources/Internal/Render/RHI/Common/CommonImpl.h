#pragma once
#include "../rhi_Type.h"

namespace rhi
{
namespace CommonImpl
{
struct Frame
{
    Handle sync = InvalidHandle;
    Handle perfQuerySet = InvalidHandle;
    std::vector<Handle> pass;
    bool readyToExecute = false;
    uint32 frameNumber;
};

struct ImmediateCommand
{
    void* cmdData = nullptr; //TODO - should be common immediate command interface like software command ?
    uint32 cmdCount;
    bool forceImmediate = false;
};
}

namespace DispatchPlatform
{
extern void (*InitContext)();
extern void (*AcquireContext)(); //TODO - may be this should be just part of opengl
extern void (*ReleaseContext)(); //TODO - may be this should be just part of opengl
extern void (*CheckSurface)();
extern void (*Suspend)();

extern void (*ProcessImmediateCommand)(CommonImpl::ImmediateCommand* command); //called from render thread

extern void (*BeginFrame)(); //this functions are called from main thread
extern void (*FinishFrame)();
extern void (*ExecuteFrame)(CommonImpl::Frame&&); //should also handle command buffer sync here
extern void (*RejectFrame)(CommonImpl::Frame&&); //should also handle command buffer sync here

extern bool (*PresntBuffer)();
extern void (*ResetBlock)();
}
}