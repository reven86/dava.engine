#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class IModule;
struct PointersToModules;

class ModuleManager
{
public:
    ModuleManager();
    ~ModuleManager();

    template <typename T>
    T* GetModule();

private:
    Vector<IModule*> listModules;
    PointersToModules* pointersToModules;
};
}
