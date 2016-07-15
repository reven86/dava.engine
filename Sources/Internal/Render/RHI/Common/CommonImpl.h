#pragma once
#include "../rhi_Type.h"

namespace rhi
{
struct RenderPassBase
{
    DAVA::Vector<Handle> cmdBuf;
    int32 priority;
    uint32 perfQueryIndex0;
    uint32 perfQueryIndex1;
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
    std::vector<Handle> pass;
    bool readyToExecute;
};

namespace CommonDetail
{
static bool renderContextReady = false;
static bool resetPending = false;
}

namespace DispatchPlatform
{
void (*InitContext)() = nullptr;
void (*AcquireContext)() = nullptr;
void (*ReleaseContext)() = nullptr;
void (*CheckSurface)() = nullptr;
void (*Suspend)() = nullptr;

void (*ProcessImmediateCommands)() = nullptr;
void (*UpdateSyncObjects)(uint32 frame_n) = nullptr; //platform as metal uses api callbacks and dx/gl on desktop can use queries in future

void (*ExecuteCommandBuffer)(Handle cb) = nullptr; //should also handle command buffer sync here
void (*FreeCommandBuffer)(Handle cb) = nullptr;
void (*RejectCommandBuffer)(Handle cb) = nullptr; //should also handle command buffer sync here

bool (*PresntBuffer)() = nullptr;
void (*ResetBlock)() = nullptr;

//TODO - think may be we really can store them without platform dispatch
RenderPassBase* (*GetRenderPass)(Handle passHandle) = nullptr;
void (*FreeRenderPass)(Handle cb) = nullptr;
SyncObjectBase* (*GetSyncObject)(Handle syncHandle) = nullptr;
}
}