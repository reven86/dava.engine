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
    enum eFindPlugunMode
    {
        EFP_Auto,
        EFT_Release,
        EFT_Debug
    };

    /**
     \brief get list of plug-ins
     \param[in] folder - folder where find plug-ins
     \param[in] mode - plugin search mode, EFP_Auto - automatic, EFT_Release - only release version, EFT_Debug - only debug
     \returns returns a list of plug-ins
     */
    Vector<FilePath> GetPlugins(const FilePath& folder, eFindPlugunMode mode) const;

    /**
     \brief initializes and load plugin
     \param[in] pluginPatch - path to plugin
     \returns pointer descriptor to plugin
     */
    const PluginDescriptor* InitPlugin(const FilePath& pluginPatch);

    /**
     \brief delete downloaded plugin
     \param[in] desc - pointer to plugin descriptor
     \returns returns true - success, false - error
     */
    bool ShutdownPlugin(const PluginDescriptor* desc);

    /**
     \brief removes all loaded plugs
     */
    void ShutdownPlugins();

    PluginManager(Engine* engine);
    ~PluginManager();

private:
    List<PluginDescriptor> pluginDescriptors;
    Engine* rootEngine;
};
}
#endif __DAVAENGINE_COREV2__