#pragma once

#include "NGTCmdLineParser.h"

#include "Base/BaseTypes.h"

#include <core_generic_plugin_manager/generic_plugin_manager.hpp>
#include <core_generic_plugin/interfaces/i_component_context.hpp>

#include <core_ui_framework/i_window.hpp>

class QMainWindow;
namespace NGTLayer
{
class BaseApplication
{
public:
    BaseApplication(int argc, char** argv);
    virtual ~BaseApplication();

    void LoadPlugins();
    IComponentContext& GetComponentContext();
    int StartApplication(QMainWindow* appMainWindow);

protected:
    virtual void GetPluginsForLoad(DAVA::Vector<DAVA::WideString>& names) const = 0;
    virtual void OnPostLoadPugins()
    {
    }
    virtual void OnPreUnloadPlugins()
    {
    }

    virtual bool OnRequestCloseApp()
    {
        return true;
    }

private:
    DAVA::WideString GetPluginsFolder() const;
    void OnMainWindowTryClose(bool& result);
    void OnMainWindowClosed();

private:
    GenericPluginManager pluginManager;
    NGTCmdLineParser commandLineParser;
};
} // namespace NGTLayer