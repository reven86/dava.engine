#pragma once

#include "Classes/Qt/Main/RecentMenuItems.h"
#include "TArc/Core/ClientModule.h"
#include "TArc/Utils/QtConnections.h"

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
    void LoadMaterialsSettings(ProjectManagerData* data);
    ProjectManagerData* GetData();

private:
    std::unique_ptr<RecentMenuItems> recentProject;
    DAVA::TArc::QtConnections connections;
    QtDelayedExecutor delayedExecutor;

    DAVA_VIRTUAL_REFLECTION(ProjectManagerModule, DAVA::TArc::ClientModule)
    {
        DAVA::ReflectionRegistrator<ProjectManagerModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
};