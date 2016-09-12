#ifndef __DAVAENGINE_IMODULE_H__
#define __DAVAENGINE_IMODULE_H__

namespace DAVA
{
    
class IModule
{
    virtual void Init() {};
    virtual void PostInit() {};
    virtual void Shutdown() {};
    
    static void InitModules();
};
    
}

#endif
