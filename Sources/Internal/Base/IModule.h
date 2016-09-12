#ifndef __DAVAENGINE_IMODULE_H__
#define __DAVAENGINE_IMODULE_H__

#include "Base/BaseTypes.h"

namespace DAVA
{
class IModule
{
public:
    virtual ~IModule()
    {
    }
    virtual void Init()
    {
    }
    virtual void PostInit()
    {
    }
    virtual void Shutdown()
    {
    }

    static void ReleaseModules();
    static void InitModules();

private:
    static Vector<IModule*> listModules;
};
}

#endif
