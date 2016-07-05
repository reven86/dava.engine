#pragma once
#include "../rhi_Type.h"

namespace rhi
{
struct RenderPassBase
{
    int32 priority;
    DAVA::Vector<Handle> cmdBuf;
};

struct SyncObjectBase
{
    uint32 frame;
    uint32 is_signaled : 1;
    uint32 is_used : 1;
};

struct FrameBase
{
    Handle sync;
    bool readyToExecute;
};

namespace DispatchPlatform
{
void (*InitContext)() = nullptr;
void (*AcquireContext)() = nullptr;
void (*ReleaseContext)() = nullptr;
void (*CheckSurface)() = nullptr;

void (*Suspend)() = nullptr;

void (*ProcessImmediateCommands)() = nullptr;

RenderPassBase* (*GetPass)(Handle passHandle) = nullptr;
SyncObjectBase* (*GetSyncObject)(Handle passHandle) = nullptr;
}
}