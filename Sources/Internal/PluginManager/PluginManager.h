#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/FileSystem.h"

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
    bool ShutdownPlugin(PluginDescriptor* desc);
    void ShutdownPlugins();

    PluginManager(Engine* engine);
    ~PluginManager();

private:
    Vector<PluginDescriptor> pluginDescriptors;
    Engine* rootEngine;
};
}
