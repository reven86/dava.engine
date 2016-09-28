#pragma once

#include "ModuleManager/IModule.h"
#include "ModuleManager/ModuleManager.h"

#include "Base/Singleton.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
    
class ModuleManager;

namespace Test
{
    
class SampleModule : public IModule
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
    
private:
    SampleModule();
    friend ModuleManager;
    
    void Init() override;
    void Shutdown() override;
    
    Vector<eStatus> statusList;
};
    
};
};