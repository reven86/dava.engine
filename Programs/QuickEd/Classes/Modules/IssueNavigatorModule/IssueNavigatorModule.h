#pragma once

#include <Base/BaseTypes.h>
#include <TArc/Core/ClientModule.h>

class IssueNavigatorWidget;
class LayoutIssuesHandler;

class IssueNavigatorModule : public DAVA::TArc::ClientModule
{
    DAVA_VIRTUAL_REFLECTION(IssueNavigatorModule, DAVA::TArc::ClientModule);

private:
    void PostInit() override;
    void InitUI();
    void RegisterOperations();

    IssueNavigatorWidget* widget = nullptr;

private:
    std::unique_ptr<LayoutIssuesHandler> layoutIssuesHandler;
};
