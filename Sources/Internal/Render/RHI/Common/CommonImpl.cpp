#include "CommonImpl.h"
namespace rhi
{
namespace DispatchPlatform
{
void (*InitContext)() = nullptr;
void (*AcquireContext)() = nullptr;
void (*ReleaseContext)() = nullptr;
void (*CheckSurface)() = nullptr;
void (*Suspend)() = nullptr;

void (*ProcessImmediateCommand)(CommonImpl::ImmediateCommand* command) = nullptr; //called from render thread

void (*InvalidateFrameCache)() = nullptr;
void (*ExecuteFrame)(CommonImpl::Frame&&) = nullptr; //should also handle command buffer sync here
void (*RejectFrame)(CommonImpl::Frame&&) = nullptr; //should also handle command buffer sync here

bool (*PresntBuffer)() = nullptr;
void (*ResetBlock)() = nullptr;
}
}