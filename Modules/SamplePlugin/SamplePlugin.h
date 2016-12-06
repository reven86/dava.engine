#pragma once

#include "ModuleManager/IModule.h"
#include "Base/BaseTypes.h"

class SamplePlugin : public DAVA::IModule
{
public:

    SamplePlugin(DAVA::Engine* engine);
    
    void Init() override;
    void Shutdown() override;
    
private:
    
};
    
