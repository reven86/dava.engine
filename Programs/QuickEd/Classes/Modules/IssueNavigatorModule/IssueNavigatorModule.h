#pragma once

#include <Base/BaseTypes.h>
#include <TArc/Core/ClientModule.h>

class IssueNavigatorWidget;

class IssueNavigatorModule : public DAVA::TArc::ClientModule
{
private:
    void PostInit() override;
    void InitUI();
    void RegisterOperations();

    void OnSomething();

    IssueNavigatorWidget* widget = nullptr;

    DAVA_VIRTUAL_REFLECTION(IssueNavigatorModule, DAVA::TArc::ClientModule);
};
