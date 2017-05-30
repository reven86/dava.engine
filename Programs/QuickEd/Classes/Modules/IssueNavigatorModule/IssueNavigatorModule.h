#pragma once

#include <Base/BaseTypes.h>
#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

class IssueNavigatorWidget;
class LayoutIssuesHandler;

class IssueNavigatorModule : public DAVA::TArc::ClientModule
{
    DAVA_VIRTUAL_REFLECTION(IssueNavigatorModule, DAVA::TArc::ClientModule);

private:
    void PostInit() override;

    void JumpToControl(const DAVA::FilePath& packagePath, const DAVA::String& controlName);
    void JumpToPackage(const DAVA::FilePath& packagePath);

    DAVA::TArc::QtConnections connections;
    IssueNavigatorWidget* widget = nullptr;

    std::unique_ptr<LayoutIssuesHandler> layoutIssuesHandler;
};
