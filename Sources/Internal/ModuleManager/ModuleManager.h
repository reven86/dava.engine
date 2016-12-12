#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/FileSystem.h"

namespace DAVA
{
class IModule;
class Engine;

typedef IModule* (*CreatPluginFuncPtr)(Engine*);
typedef void (*DestroyPluginFuncPtr)(IModule*);

class ModuleManager final
{
public:
    enum EFindPlugunMode
    {
        EFP_Auto,
        EFT_Release,
        EFT_Debug
    };

    ModuleManager(Engine* engine);
    ~ModuleManager();

    template <typename T>
    T* GetModule() const;

    void InitModules();
    void ShutdownModules();

    Vector<FilePath> PluginList(const FilePath& folder, EFindPlugunMode mode) const;

    void InitPlugin(const FilePath& pluginPatch);
    void ShutdownPlugins();

private:
    struct PointersToModules;
    struct PointersToPluginFuctions
    {
        CreatPluginFuncPtr creatPluginFunc;
        DestroyPluginFuncPtr destroyPluginFunc;
        void* libHandle;
        IModule* ptrPlugin;
        String namePlugin;
    };

    Vector<IModule*> modules;
    std::unique_ptr<PointersToModules> pointersToModules;

    Vector<PointersToPluginFuctions> plugins;
    Engine* rootEngine;
};
}
