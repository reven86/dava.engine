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
}

namespace DispatchPlatform
{
static void (*InitContext)() = nullptr;
static void (*AcquireContext)() = nullptr;
static void (*ReleaseContext)() = nullptr;
static void (*CheckSurface)() = nullptr;
static void (*Suspend)() = nullptr;

static void (*ProcessImmediateCommands)() = nullptr;

static void (*InvalidateFrameCache)() = nullptr;
static void (*ExecuteFrame)(CommonImpl::Frame&&) = nullptr; //should also handle command buffer sync here
static void (*RejectFrame)(CommonImpl::Frame&&) = nullptr; //should also handle command buffer sync here

static bool (*PresntBuffer)() = nullptr;
static void (*ResetBlock)() = nullptr;
}
}