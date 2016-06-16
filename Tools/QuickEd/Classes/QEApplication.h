#pragma once

#include "NgtTools/Application/NGTApplication.h"

class WGTCommand;
class ICommandManager;

class QEApplication : public NGTLayer::BaseApplication
{
public:
    QEApplication(int argc, char** argv);
    ~QEApplication();

    int Run();

protected:
    void GetPluginsForLoad(DAVA::Vector<DAVA::WideString>& names) const override;
    void OnPostLoadPugins() override;
    void OnPreUnloadPlugins() override;

private:
    ICommandManager* commandManager = nullptr;
    std::unique_ptr<WGTCommand> ngtCommand;
};
