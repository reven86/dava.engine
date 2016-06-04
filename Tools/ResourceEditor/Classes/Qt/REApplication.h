#pragma once

#include "NgtTools/Application/NGTApplication.h"
#include "NGTPropertyEditor/ComponentProvider.h"

class QtMainWindow;
class NGTCommand;
class ICommandManager;

class REApplication : public NGTLayer::BaseApplication
{
public:
    REApplication(int argc, char** argv);
    ~REApplication();

    void Run();

protected:
    void GetPluginsForLoad(DAVA::Vector<DAVA::WideString>& names) const override;
    void OnPostLoadPugins() override;
    void OnPreUnloadPlugins() override;

private:
    ICommandManager* commandManager = nullptr;
    std::unique_ptr<NGTCommand> ngtCommand;
    std::unique_ptr<IComponentProvider> componentProvider;
    QtMainWindow* mainWindow = nullptr;
};
