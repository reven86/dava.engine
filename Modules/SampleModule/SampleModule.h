#pragma once

#include "ModuleManager/IModule.h"
#include "Base/Singleton.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
    
class SampleModule : public IModule, public Singleton<SampleModule>
{
public:
    enum eStatus
    {
        ES_UNKNOWN,
        ES_INIT,
        ES_POST_INIT,
        ES_SHUTDOWN
    };

    Vector<eStatus> StatusList() const
    {
        return statusList;
    }
    
    SampleModule();
    
private:
    void Init() override;
    void PostInit() override;
    void Shutdown() override;
    
    Vector<eStatus> statusList;
};
    
};