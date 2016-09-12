#pragma once
#include "rhi_Pool.h"
#include "rhi_CommonImpl.h"

namespace rhi
{
namespace FrameLoop
{
void ProcessFrame();
bool FinishFrame(Handle sync); //return false if frame was empty
bool FrameReady();
uint32 FramesCount();
void AddPass(Handle pass);
void RejectFrames();
}
}
