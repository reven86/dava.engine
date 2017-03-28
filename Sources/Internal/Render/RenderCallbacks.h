#ifndef __DAVAENGINE_RENDER_CALLBACKS_H__
#define __DAVAENGINE_RENDER_CALLBACKS_H__

#include "Functional/Function.h"
#include "Base/Token.h"
#include "Render/RHI/rhi_Public.h"

namespace DAVA
{
namespace RenderCallbacks
{
//can register same callback for multiple objects, callback is removed after sync callback
Token RegisterSyncCallback(rhi::HSyncObject syncObject, Function<void(rhi::HSyncObject)> callback);

//unregister for all sync objects
void UnRegisterSyncCallback(Token token);

void ProcessFrame();
}
}

#endif
