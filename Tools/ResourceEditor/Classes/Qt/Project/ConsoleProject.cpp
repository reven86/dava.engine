#include "Qt/Project/ConsoleProject.h"
#include "QtTools/ProjectInformation/MaterialTemplatesInfo.h"

#include "FileSystem/FileSystem.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Sound/SoundSystem.h"

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
