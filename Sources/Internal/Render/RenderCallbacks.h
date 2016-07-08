#ifndef __DAVAENGINE_RENDER_CALLBACKS_H__
#define __DAVAENGINE_RENDER_CALLBACKS_H__

#include "Functional/Function.h"
#include "Render/RHI/rhi_Public.h"

namespace DAVA
{
namespace RenderCallbacks
{
void RegisterResourceRestoreCallback(Function<void()> callback);
void UnRegisterResourceRestoreCallback(Function<void()> callback);

//post restore callbacks are used to restore complex derived objects - EG render targets that contain render of other resources
void RegisterPostRestoreCallback(Function<void()> callback);
void UnRegisterPostRestoreCallback(Function<void()> callback);

//can register same callback for multiple objects, callback is removed after sync callback
void RegisterSyncCallback(rhi::HSyncObject syncObject, Function<void(rhi::HSyncObject)> callback);

//unregister for all sync objects
void UnRegisterSyncCallback(Function<void(rhi::HSyncObject)> callback);

void ProcessFrame();
}
}

#endif
