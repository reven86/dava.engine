#pragma once
#include "rhi_Pool.h"
#include "rhi_CommonImpl.h"
#include "../rhi_Public.h"

namespace rhi
{
namespace FrameLoop
{
static const uint32 FRAME_POOL_SIZE = 16;

void ProcessFrame();
bool FinishFrame(); //return false if frame was empty
bool FrameReady();
uint32 FramesCount();
void AddPass(Handle pass);
void RejectFrames();
void SetFramePerfQueries(Handle startQuery, Handle endQuery);

void ScheduleResourceDeletion(Handle handle, ResourceType resourceType);
HSyncObject GetCurrentFrameSyncObject();
}
}
