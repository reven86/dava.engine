#pragma once

#include "CommandLine/CommandLineManager.h"

#include "NgtTools/Application/NGTApplication.h"
#include "NGTPropertyEditor/ComponentProvider.h"

class QtMainWindow;
class WGTCommand;
namespace NGTLayer
{
class NGTCmdLineParser;
}

namespace wgt
{
class ICommandManager;
}

class REApplication : public NGTLayer::BaseApplication
{
public:
    REApplication(int argc, char** argv);
    ~REApplication();

    int Run();

protected:
    void GetPluginsForLoad(DAVA::Vector<DAVA::WideString>& names) const override;
    void OnPostLoadPlugins() override;
    void OnPreUnloadPlugins() override;
    bool OnRequestCloseApp() override;
    void ConfigureLineCommand(NGTLayer::NGTCmdLineParser& lineParser) override;

private:
    void RunWindow();
    void RunConsole();

private:
    wgt::ICommandManager* commandManager = nullptr;
    std::unique_ptr<WGTCommand> wgtCommand;
    std::unique_ptr<wgt::IComponentProvider> componentProvider;
    QtMainWindow* mainWindow = nullptr;
    std::unique_ptr<CommandLineManager> cmdLineManager;
};
