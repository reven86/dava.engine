#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/FileSystem.h"

#if defined(__DAVAENGINE_MACOS__) 

#define PLUGIN_FUNCTION_EXPORT __attribute__((visibility("default")))

#elif defined(__DAVAENGINE_WIN32__) 

#define PLUGIN_FUNCTION_EXPORT __declspec(dllexport)

#else

#define PLUGIN_FUNCTION_EXPORT 

#endif

namespace DAVA
{
class IModule;
class Engine;

typedef IModule* (*CreatPluginFuncPtr)(Engine*);
typedef void(*DestroyPluginFuncPtr)(IModule*);

class PluginManager final
{
public:
    enum EFindPlugunMode
    {
        EFP_Auto,
        EFT_Release,
        EFT_Debug
    };

    Vector<FilePath> PluginList(const FilePath& folder, EFindPlugunMode mode) const;

    void InitPlugin(const FilePath& pluginPatch);
    void ShutdownPlugins();


    PluginManager(Engine* engine);
    ~PluginManager();

private:
    struct PointersToPluginFuctions;
    Vector<PointersToPluginFuctions> plugins;
    Engine* rootEngine;

};
}
