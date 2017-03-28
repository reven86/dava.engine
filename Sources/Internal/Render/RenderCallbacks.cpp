#include "RenderCallbacks.h"
#include "Concurrency/LockGuard.h"
#include "Utils/Utils.h"

namespace DAVA
{
namespace RenderCallbacksDetails
{
struct SyncCallback
{
    rhi::HSyncObject syncObject;
    Token callbackToken;
    Function<void(rhi::HSyncObject)> callback;
};

Vector<SyncCallback> syncCallbacks;
TokenProvider<rhi::HSyncObject> syncCallbackTokenProvider;
}

namespace RenderCallbacks
{

void ProcessFrame()
{
    using namespace RenderCallbacksDetails;

    for (size_t i = 0, sz = syncCallbacks.size(); i < sz;)
    {
        if (rhi::SyncObjectSignaled(syncCallbacks[i].syncObject))
        {
            syncCallbacks[i].callback(syncCallbacks[i].syncObject);
            RemoveExchangingWithLast(syncCallbacks, i);
            --sz;
        }
        else
        {
            ++i;
        }
    }
}

Token RegisterSyncCallback(rhi::HSyncObject syncObject, Function<void(rhi::HSyncObject)> callback)
{
    using namespace RenderCallbacksDetails;

    Token token = syncCallbackTokenProvider.Generate();
    syncCallbacks.push_back({ syncObject, token, callback });

    return token;
}

void UnRegisterSyncCallback(Token token)
{
    using namespace RenderCallbacksDetails;

    for (size_t i = 0, sz = syncCallbacks.size(); i < sz;)
    {
        if (syncCallbacks[i].callbackToken == token)
        {
            RemoveExchangingWithLast(syncCallbacks, i);
            --sz;
        }
        else
        {
            ++i;
        }
    }
}
}
}