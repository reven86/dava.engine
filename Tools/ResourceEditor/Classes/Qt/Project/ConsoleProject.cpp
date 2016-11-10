#include "Qt/Project/ConsoleProject.h"

ConsoleProject::~ConsoleProject()
{
    CloseProject();
}

void ConsoleProject::OpenProject(const DAVA::FilePath& incomePath)
{
    if (incomePath.IsDirectoryPathname() && incomePath != projectPath)
    {
        CloseProject();

        projectPath = incomePath;

        if (DAVA::FileSystem::Instance()->Exists(incomePath))
        {
            DAVA::FilePath::AddTopResourcesFolder(projectPath + "Data/");

            EditorConfig::Instance()->ParseConfig(projectPath + "EditorConfig.yaml");
            MaterialTemplatesInfo::Instance()->Load("~res:/Materials/assignable.yaml");
            DAVA::QualitySettingsSystem::Instance()->Load("~res:/quality.yaml");
            DAVA::SoundSystem::Instance()->InitFromQualitySettings();
        }
    }
}

void ConsoleProject::CloseProject()
{
    if (!projectPath.IsEmpty())
    {
        DAVA::FilePath::RemoveResourcesFolder(projectPath + "Data/");
        projectPath = "";
    }
}
