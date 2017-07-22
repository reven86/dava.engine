#include "NetworkCore.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
NetworkCore::NetworkCore(Engine* engine)
    : IModule(engine)
{
    statusList.emplace_back(eStatus::ES_UNKNOWN);
}

void NetworkCore::Init()
{
    statusList.emplace_back(eStatus::ES_INIT);
}

void NetworkCore::Shutdown()
{
    statusList.emplace_back(eStatus::ES_SHUTDOWN);
}

DAVA_VIRTUAL_REFLECTION_IMPL(NetworkCore)
{
    ReflectionRegistrator<NetworkCore>::Begin()
    .End();
}
}
