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
extern void (*CheckSurface)(); //TODO - may be this should be part of opengl only?
extern void (*Suspend)(); //perform finalization before going to suspend

extern void (*ProcessImmediateCommand)(CommonImpl::ImmediateCommand* command); //called from render thread

extern void (*FinishFrame)(); //this functions is called from main thread
extern void (*ExecuteFrame)(CommonImpl::Frame&&); //should also handle command buffer sync here
extern void (*RejectFrame)(CommonImpl::Frame&&); //should also handle command buffer sync here

extern bool (*PresntBuffer)();
extern void (*ResetBlock)();
}
}