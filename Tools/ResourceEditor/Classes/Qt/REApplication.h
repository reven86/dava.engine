#pragma once

#include "NgtTools/Application/NGTApplication.h"

class NGTCommand;
class ICommandManager;

class REApplication : public NGTLayer::BaseApplication
{
public:
    REApplication(int argc, char** argv);
    ~REApplication();

    int Run();

protected:
    void GetPluginsForLoad(DAVA::Vector<DAVA::WideString>& names) const override;
    void OnPostLoadPugins() override;
    void OnPreUnloadPlugins() override;

private:
    ICommandManager* commandManager = nullptr;
    std::unique_ptr<NGTCommand> ngtCommand;
};
