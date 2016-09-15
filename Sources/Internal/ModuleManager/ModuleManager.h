#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{

class IModule;

class ModuleManager
{
public:
    ModuleManager();
    ~ModuleManager();

    template <typename T>
    T* GetModule() const;

private:
    struct PointersToModules;

    Vector<IModule*> modules;
    std::unique_ptr<PointersToModules> pointersToModules;

};
}
