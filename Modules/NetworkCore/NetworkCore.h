#pragma once

#include <ModuleManager/IModule.h>
#include <ModuleManager/ModuleManager.h>

#include <Reflection/Reflection.h>
#include <Base/BaseTypes.h>

namespace DAVA
{
class ModuleManager;

class NetworkCore : public IModule
{
public:
    enum eStatus
    {
        ES_UNKNOWN,
        ES_INIT,
        ES_SHUTDOWN
    };

    const Vector<eStatus>& StatusList() const
    {
        return statusList;
    }

    NetworkCore(Engine* engine);

    void Init() override;
    void Shutdown() override;

private:
    Vector<eStatus> statusList;

    DAVA_VIRTUAL_REFLECTION(NetworkCore, IModule);
};
};
