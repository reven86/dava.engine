#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Models/RecentMenuItems.h>
#include <TArc/Utils/QtConnections.h>
#include <TArc/Core/OperationRegistrator.h>

#include <QtTools/Utils/QtDelayedExecutor.h>

namespace DAVA
{
class ResultList;
}
class ProjectData;

class ProjectModule : public DAVA::TArc::ClientModule
{
public:
    ProjectModule();
    ~ProjectModule() override;

private:
    void PostInit() override;
    void OnWindowClosed(const DAVA::TArc::WindowKey& key) override;

    void CreateActions();

    void OnOpenProject();
    void OnNewProject();
    void CreateProject(const QString& projectDir);

    bool CloseProject();
    void OpenProject(const DAVA::String& path);
    void OpenLastProject();
    void ShowResultList(const QString& title, const DAVA::ResultList& resultList);
    void LoadPlugins();
    void RegisterFolders();
    void UnregisterFolders();

    DAVA::TArc::QtConnections connections;
    std::unique_ptr<RecentMenuItems> recentProjects;
    QtDelayedExecutor delayedExecutor;

    DAVA_VIRTUAL_REFLECTION(ProjectModule, DAVA::TArc::ClientModule);
};

namespace ProjectModuleTesting
{
DECLARE_OPERATION_ID(CreateProjectOperation);
}
