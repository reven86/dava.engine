#ifndef __DAVAENGINE_IMODULE_H__
#define __DAVAENGINE_IMODULE_H__

#include "Base/BaseTypes.h"

namespace DAVA
{
    
class IModule
{
    virtual void Init(Engine* engine) {};
    virtual void PostInit() {};
    virtual void Shutdown() {};
}
    
}

#endif
