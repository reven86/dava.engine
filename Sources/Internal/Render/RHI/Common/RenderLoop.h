#pragma once
#include "../rhi_Type.h"
#include "CommonImpl.h"
#include "Concurrency/Thread.h"

namespace rhi
{
namespace RenderLoop
{
void Present(Handle syncHandle); // called from main thread

void InitializeRenderLoop(uint32 frameCount, DAVA::Thread::eThreadPriority priority, int32 bindToProcessor);
void UninitializeRenderLoop();

void SuspendRender();
void ResumeRender();

void IssueImmediateCommand(CommonImpl::ImmediateCommand* command); //blocking until complete
void CheckImmediateCommand(); //called from render thread only

void SetResetPending();
}
}