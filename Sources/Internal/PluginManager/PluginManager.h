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

//

#define EXPORT_PLUGIN( PLUGIN ) \
extern "C" { \
    PLUGIN_FUNCTION_EXPORT \
    DAVA::IModule* CreatePlugin(DAVA::Engine* engine)\
    {\
        return new PLUGIN(engine);\
    }\
    PLUGIN_FUNCTION_EXPORT\
    void DestroyPlugin(DAVA::IModule* plugin)\
    {\
        delete plugin;\
    }\
}

//

namespace DAVA
{
class IModule;
class Engine;

class PluginManager final
{
public:

    struct PluginDescriptor;

    enum eFrustumPlane
    {
        EFP_LEFT = 0,
        EFP_RIGHT
    };

    enum eFindPlugunMode
    {
        EFP_Auto,
        EFT_Release,
        EFT_Debug
    };

    Vector<FilePath> GetPlugins(const FilePath& folder, eFindPlugunMode mode) const;

    PluginDescriptor* InitPlugin(const FilePath& pluginPatch);
    bool ShutdownPlugin( PluginDescriptor* desc );
    void ShutdownPlugins();

    PluginManager(Engine* engine);
    ~PluginManager();

private:
    Vector<PluginDescriptor> pluginDescriptors;
    Engine* rootEngine;
};
}
