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
static void (*InitContext)() = nullptr;
static void (*AcquireContext)() = nullptr;
static void (*ReleaseContext)() = nullptr;
static void (*CheckSurface)() = nullptr;
static void (*Suspend)() = nullptr;

static void (*ProcessImmediateCommand)(CommonImpl::ImmediateCommand* command) = nullptr; //called from render thread

static void (*InvalidateFrameCache)() = nullptr;
static void (*ExecuteFrame)(CommonImpl::Frame&&) = nullptr; //should also handle command buffer sync here
static void (*RejectFrame)(CommonImpl::Frame&&) = nullptr; //should also handle command buffer sync here

static bool (*PresntBuffer)() = nullptr;
static void (*ResetBlock)() = nullptr;
}
}