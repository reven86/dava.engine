#pragma once

#include "ModuleManager/IModule.h"
#include "ModuleManager/ModuleManager.h"

#include "Base/Singleton.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
    
class ModuleManager;

class SampleDynamicModule : public IModule
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
    
    SampleDynamicModule(Engine* engine);
    
    void Init() override;
    void Shutdown() override;
    
private:
    Vector<eStatus> statusList;
};

};