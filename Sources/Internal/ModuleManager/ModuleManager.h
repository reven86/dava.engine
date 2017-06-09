#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class IModule;
class Engine;

class ModuleManager final
{
public:
    ModuleManager(Engine* engine);
    ~ModuleManager();

    template <typename T>
    T* GetModule() const;

    IModule* GetModule(const String& permanentName);

    void InitModules();
    void ShutdownModules();

private:
    struct PointersToModules;

    Vector<IModule*> modules;
    std::unique_ptr<PointersToModules> pointersToModules;
};
}
