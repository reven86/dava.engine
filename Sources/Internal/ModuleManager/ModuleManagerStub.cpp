#include "ModuleManager/ModuleManager.h"
#include "ModuleManager/IModule.h"

namespace DAVA
{
    
    struct ModuleManager::PointersToModules
    {
    };
    

    
    ModuleManager::ModuleManager()
    {
    }
    
    ModuleManager::~ModuleManager()
    {
        ResetModules();
    }
    
    void ModuleManager::InitModules()
    {
        
    }
    
    void  ModuleManager::ResetModules()
    {
        for (const auto& module : modules)
        {
            module->Shutdown();
            delete module;
        }
        modules.clear();
    }
    
    
}
