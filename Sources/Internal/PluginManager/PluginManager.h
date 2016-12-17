#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/FileSystem.h"

//

namespace DAVA
{
class IModule;
class Engine;

struct PluginDescriptor;

class PluginManager final
{
public:
    enum eFindPlugunMode
    {
        EFP_Auto,
        EFT_Release,
        EFT_Debug
    };

    Vector<FilePath> GetPlugins(const FilePath& folder, eFindPlugunMode mode) const;

    const PluginDescriptor* InitPlugin(const FilePath& pluginPatch);
    bool ShutdownPlugin(const PluginDescriptor* desc);
    void ShutdownPlugins();

    PluginManager(Engine* engine);
    ~PluginManager();

private:
    List<PluginDescriptor> pluginDescriptors;
    Engine* rootEngine;
};
}
