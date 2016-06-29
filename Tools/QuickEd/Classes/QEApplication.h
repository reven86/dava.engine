#pragma once

#include "NgtTools/Application/NGTApplication.h"

class WGTCommand;
class EditorCore;
namespace wgt
{
class ICommandManager;
}
class QEApplication : public NGTLayer::BaseApplication
{
public:
    QEApplication(int argc, char** argv);
    ~QEApplication();

    int Run();

protected:
    void GetPluginsForLoad(DAVA::Vector<DAVA::WideString>& names) const override;
    void OnPostLoadPlugins() override;
    void OnPreUnloadPlugins() override;
    void ConfigureLineCommand(NGTLayer::NGTCmdLineParser& lineParser) override;
    bool OnRequestCloseApp() override;

private:
    wgt::ICommandManager* commandManager = nullptr;
    std::unique_ptr<WGTCommand> ngtCommand;
    EditorCore* editorCore = nullptr;
};
