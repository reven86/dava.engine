
#include "ModuleManager/ModuleManager.h"
#include "ModuleManager/IModule.h"
#include "FileSystem/FileSystem.h"
#include <dlfcn.h>

namespace DAVA
{
Vector<FilePath> ModuleManager::PluginList(const FilePath& folder, EFindPlugunMode mode) const
{
#ifdef __DAVAENGINE_DEBUG__
    bool debugMode = true;
#else
    bool debugMode = false;
#endif

    String debugSuffix = "Debug";
    String dlibExtension = ".dylib";

    Vector<FilePath> pluginsList;

    FileSystem* fs = FileSystem::Instance();
    Vector<FilePath> cacheDirContent = fs->EnumerateFilesInDirectory(folder, false);

    for (auto it = rbegin(cacheDirContent); it != rend(cacheDirContent); ++it)
    {
        String nameFile = it->GetBasename();
        String extensionFile = it->GetExtension();

        bool debugLib = nameFile.find(debugSuffix, nameFile.size() - debugSuffix.size()) != std::string::npos;

        if (extensionFile == dlibExtension)
        {
            bool emplace_back = false;

            switch (mode)
            {
            case EFP_Auto:
                emplace_back = debugMode == debugLib;
                break;

            case EFT_Release:
                emplace_back = !debugLib;

                break;

            case EFT_Debug:
                emplace_back = debugLib;
                break;
            }

            if (emplace_back)
            {
                pluginsList.emplace_back(*it);
            }
        }
    }

    return pluginsList;
}

void ModuleManager::InitPlugin(const FilePath& pluginPatch)
{
    PointersToPluginFuctions plugin;

    // Open the library.
    String pluginPath = pluginPatch.GetAbsolutePathname();
    plugin.libHandle = dlopen(pluginPath.c_str(), RTLD_NOW);
    if (nullptr == plugin.libHandle)
    {
        Logger::Error("[%s] Unable to open library: %s\n", __FILE__, dlerror());
    }

    plugin.namePlugin = pluginPatch.GetFilename();
    plugin.creatPluginFunc = reinterpret_cast<CreatPluginFuncPtr>(dlsym(plugin.libHandle, "CreatPlugin"));
    plugin.destroyPluginFunc = reinterpret_cast<DestroyPluginFuncPtr>(dlsym(plugin.libHandle, "DestroyPlugin"));

    if (nullptr == plugin.creatPluginFunc || nullptr == plugin.destroyPluginFunc)
    {
        Logger::Error("[%s] Unable to get symbol: %s\n", __FILE__, dlerror());
    }

    plugin.ptrPlugin = plugin.creatPluginFunc(rootEngine);

    DVASSERT(nullptr != plugin.ptrPlugin);

    plugin.ptrPlugin->Init();

    plugins.emplace_back(plugin);

    Logger::Debug("Plugin loaded - %s", plugin.namePlugin.c_str());
}

void ModuleManager::ShutdownPlugins()
{
    for (auto it = rbegin(plugins); it != rend(plugins); ++it)
    {
        it->ptrPlugin->Shutdown();
    }

    for (auto it = rbegin(plugins); it != rend(plugins); ++it)
    {
        it->destroyPluginFunc(it->ptrPlugin);
        dlclose(it->libHandle);
        Logger::Debug("Plugin unloaded - %s", it->namePlugin.c_str());
    }

    plugins.clear();
}
}
