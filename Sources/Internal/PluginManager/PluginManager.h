#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/FileSystem.h"

//
#if defined(__DAVAENGINE_COREV2__)

namespace DAVA
{
class IModule;
class Engine;

struct PluginDescriptor;

/**
 \defgroup plugin manager system
 */
class PluginManager final
{
public:
    enum eFindPluginMode
    {
        Auto, //!< auto mode
        Release, //!< only release plugins
        Debug //!< only plugins
    };

    /**
     Returns a list of plugins in the specified mode.
     */
    Vector<FilePath> GetPlugins(const FilePath& folder, eFindPluginMode mode) const;

    /**
     Load plugin located on the path pluginPath and returns descriptor to it
     */
    const PluginDescriptor* LoadPlugin(const FilePath& pluginPath);

    /**
    Unload plugin on a descriptor and returns the operation result
     */
    bool UnloadPlugin(const PluginDescriptor* desc);

    /**
     Unload all loaded plugins
     */
    void UnloadPlugins();

    PluginManager(Engine* engine);
    ~PluginManager();

private:
    List<PluginDescriptor> pluginDescriptors;
    Engine* rootEngine;
};
}
#endif // __DAVAENGINE_COREV2__
