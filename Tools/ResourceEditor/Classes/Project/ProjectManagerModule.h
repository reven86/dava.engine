#pragma once

#include "Classes/Qt/Main/RecentMenuItems.h"
#include "TArc/Core/ClientModule.h"
#include "TArc/Utils/QtConnections.h"
#include "Project/ProjectResources.h"

#include "QtTools/Utils/QtDelayedExecutor.h"

class ProjectManagerData;
class ProjectManagerModule : public DAVA::TArc::ClientModule
{
public:
    ~ProjectManagerModule();

protected:
    void PostInit() override;

private:
    void CreateActions();
    void RegisterOperations();

    void OpenProject();
    void OpenProjectByPath(const DAVA::FilePath& incomePath);
    void OpenProjectImpl(const DAVA::FilePath& incomePath);
    void OpenLastProject();
    bool CloseProject();
    void ReloadSprites();

private:
    ProjectManagerData* GetData();

private:
    std::unique_ptr<RecentMenuItems> recentProject;
    DAVA::TArc::QtConnections connections;
    QtDelayedExecutor delayedExecutor;
    std::unique_ptr<ProjectResources> projectResources;
};
