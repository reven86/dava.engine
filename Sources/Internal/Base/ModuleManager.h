#ifndef __DAVAENGINE_MODULE_MANAGER_H__
#define __DAVAENGINE_MODULE_MANAGER_H__

#include "Base/BaseTypes.h"

namespace DAVA
{
class IModule;

class ModuleManager
{
public:
    ModuleManager();
    ~ModuleManager();

private:
    Vector<IModule*> listModules;
};
}

#endif
