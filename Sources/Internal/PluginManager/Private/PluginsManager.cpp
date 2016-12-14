
#include "PluginManager/PluginManager.h"
#include "PluginManager/Private/PluginHelper.h"
#include "ModuleManager/IModule.h"

struct PluginManager::PointersToPluginFuctions
{
    CreatPluginFuncPtr creatPluginFunc;
    DestroyPluginFuncPtr destroyPluginFunc;
    IModule* ptrPlugin;
    String namePlugin;

    PluginHandle handle;
};

Vector<FilePath> PluginManager::PluginList(const FilePath& folder, EFindPlugunMode mode) const
{
#ifdef __DAVAENGINE_DEBUG__
    bool debugMode = true;
#else
    bool debugMode = false;
#endif

#if defined(__DAVAENGINE_MACOS__) 
    String dlibExtension = ".dylib";
#elif defined(__DAVAENGINE_WIN32__)
    String dlibExtension = ".dll";
#else
    String dlibExtension = ".so";
#endif

    String debugSuffix = "Debug";

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

void PluginManager::InitPlugin(const FilePath& pluginPatch)
{
    PointersToPluginFuctions plugin;

    // Open the library.
    String pluginPath = pluginPatch.GetAbsolutePathname();
    plugin.handle = OpenPlugin(pluginPath.c_str());
    if (nullptr == plugin.handle)
    {
        Logger::Error("[%s] Unable to open library: %s\n", __FILE__, pluginPath.c_str() );
    }

    plugin.namePlugin = pluginPatch.GetFilename();
    plugin.creatPluginFunc = LoadFunction<CreatPluginFuncPtr>( plugin.handle, "CreatPlugin" );
    plugin.destroyPluginFunc = LoadFunction<DestroyPluginFuncPtr>( plugin.handle, "DestroyPlugin" );

    if ( nullptr == plugin.creatPluginFunc )
    {
        Logger::Error("[%s] Unable to get symbol: %s\n", __FILE__, "CreatPlugin" );
        DVASSERT(nullptr != plugin.creatPluginFunc);
    }

    if ( nullptr == plugin.destroyPluginFunc )
    {
        Logger::Error("[%s] Unable to get symbol: %s\n", __FILE__, "DestroyPlugin" );
        DVASSERT(nullptr != plugin.destroyPluginFunc);
    }

    plugin.ptrPlugin = plugin.creatPluginFunc(rootEngine);

    DVASSERT(nullptr != plugin.ptrPlugin);

    plugin.ptrPlugin->Init();

    plugins.emplace_back(plugin);

    Logger::Debug("Plugin loaded - %s", plugin.namePlugin.c_str());

}

void PluginManager::ShutdownPlugins()
{
    for (auto it = rbegin(plugins); it != rend(plugins); ++it)
    {
        it->ptrPlugin->Shutdown();
    }

    for (auto it = rbegin(plugins); it != rend(plugins); ++it)
    {
        it->destroyPluginFunc(it->ptrPlugin);
        ClosePlugin( it->handle );
        Logger::Debug("Plugin unloaded - %s", it->namePlugin.c_str());
    }

    plugins.clear();
}

PluginManager::PluginManager(Engine* engine)
  : rootEngine(engine )
{
}

PluginManager::~PluginManager()
{
    DVASSERT( !plugins.size() );
}