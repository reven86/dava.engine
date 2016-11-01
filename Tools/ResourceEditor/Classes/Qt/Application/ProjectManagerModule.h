#pragma once

#include "TArc/Core/ClientModule.h"
#include "TArc/Utils/QtConnections.h"

class ProjectManagerData;
class ProjectManagerModule : public DAVA::TArc::ClientModule
{
protected:
    void OnContextCreated(DAVA::TArc::DataContext& context) override;
    void OnContextDeleted(DAVA::TArc::DataContext& context) override;
    void PostInit() override;

private:
    void OpenProject();
    void OpenProject(const DAVA::FilePath& incomePath);
    void OpenProjectImpl(const DAVA::FilePath& incomePath);
    void OpenLastProject();
    void CloseProject();

private:
    void LoadProjectSettings();
    void LoadMaterialsSettings(ProjectManagerData* data);
    ProjectManagerData* GetData();

private:
    DAVA::TArc::QtConnections connections;
};