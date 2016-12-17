
#include "Engine/Engine.h"
#include "Utils/StringUtils.h"
#include "PluginManager/PluginManager.h"
#include "PluginManager/Plugin.h"
#include "ModuleManager/IModule.h"

namespace DAVA
{
using CreatePluginFuncPtr = IModule* (*)(Engine*);
using DestroyPluginFuncPtr = void (*)(IModule*);

struct PluginDescriptor
{
    CreatePluginFuncPtr createPluginFunc;
    DestroyPluginFuncPtr destroyPluginFunc;
    IModule* plugin;
    String pluginName;

    PluginHandle handle;
};

Vector<FilePath> PluginManager::GetPlugins(const FilePath& folder, eFindPlugunMode mode) const
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

    FileSystem* fs = rootEngine->GetContext()->fileSystem;
    Vector<FilePath> cacheDirContent = fs->EnumerateFilesInDirectory(folder, false);

    for (auto& path : cacheDirContent)
    {
        String fileName = path.GetBasename();
        String fileExtension = path.GetExtension();

        bool debugLib = StringUtils::EndsWith(fileName, debugSuffix);

        if (fileExtension == dlibExtension)
        {
            bool isModeMatched = false;

            switch (mode)
            {
            case EFP_Auto:
                isModeMatched = debugMode == debugLib;
                break;

            case EFT_Release:
                isModeMatched = !debugLib;

                break;

            case EFT_Debug:
                isModeMatched = debugLib;
                break;
            }

            if (isModeMatched)
            {
                pluginsList.push_back(path);
            }
        }
    }

    return pluginsList;
}

const PluginDescriptor* PluginManager::InitPlugin(const FilePath& pluginPatch)
{
    PluginDescriptor desc;

    bool success = true;

    // Open the library.
    String pluginPath = pluginPatch.GetAbsolutePathname();
    desc.handle = OpenPlugin(pluginPath.c_str());
    if (nullptr == desc.handle)
    {
        Logger::Warning("[%s] Unable to open library: %s\n", __FILE__, pluginPath.c_str());
        return nullptr;
    }

    desc.pluginName = pluginPatch.GetFilename();
    desc.createPluginFunc = LoadFunction<CreatePluginFuncPtr>(desc.handle, "CreatePlugin");
    desc.destroyPluginFunc = LoadFunction<DestroyPluginFuncPtr>(desc.handle, "DestroyPlugin");

    if (nullptr == desc.createPluginFunc)
    {
        Logger::Warning("[%s] Unable to get symbol: %s\n", __FILE__, "CreatePlugin");
        success = false;
    }

    if (nullptr == desc.destroyPluginFunc)
    {
        Logger::Warning("[%s] Unable to get symbol: %s\n", __FILE__, "DestroyPlugin");
        success = false;
    }

    if (success)
    {
        desc.plugin = desc.createPluginFunc(rootEngine);

        if (nullptr == desc.plugin)
        {
            Logger::Warning("[%s] Can not create plugin: %s\n", __FILE__, pluginPath.c_str() );
            success = false;
        }
    }

    if (!success )
    {
        ClosePlugin(desc.handle);
        return nullptr;
    }

    desc.plugin->Init();

    pluginDescriptors.push_back(desc);

    Logger::Debug("Plugin loaded - %s", desc.pluginName.c_str());

    return &pluginDescriptors.back();
}

bool PluginManager::ShutdownPlugin(const PluginDescriptor* desc)
{
    DVASSERT(desc != nullptr);

    for (auto it = begin(pluginDescriptors); it != end(pluginDescriptors); ++it)
    {
        if (&(*it) == desc)
        {
            desc->plugin->Shutdown();
            desc->destroyPluginFunc( desc->plugin );
            ClosePlugin( desc->handle );
            pluginDescriptors.erase(it);
            return true;
        }
    }

    return false;
}

void PluginManager::ShutdownPlugins()
{
    for (auto& desc : pluginDescriptors)
    {
        desc.plugin->Shutdown();
    }

    for (auto& desc : pluginDescriptors)
    {
        desc.destroyPluginFunc(desc.plugin);
        ClosePlugin(desc.handle);
        Logger::Debug("Plugin unloaded - %s", desc.pluginName.c_str());
    }

    pluginDescriptors.clear();
}

PluginManager::PluginManager(Engine* engine)
    : rootEngine(engine)
{
}

PluginManager::~PluginManager()
{
    DVASSERT(pluginDescriptors.empty());
}
}
