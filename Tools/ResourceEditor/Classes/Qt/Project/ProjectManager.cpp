#include "Project/ProjectManager.h"
#include "Main/QtUtils.h"
#include "Settings/SettingsManager.h"
#include "Deprecated/SceneValidator.h"
#include "Deprecated/EditorConfig.h"

#include "Utils/TextureDescriptor/TextureDescriptorUtils.h"

#include "FileSystem/FileSystem.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"

#include "QtTools/FileDialogs/FileDialog.h"
#include "QtTools/ProjectInformation/ProjectStructure.h"
#include "QtTools/ProjectInformation/MaterialTemplatesInfo.h"

#include "SpritesPacker/SpritesPackerModule.h"

ProjectManager::ProjectManager()
{
    DAVA::Vector<DAVA::String> sceneFileExtensions = { "sc2" };
    dataSourceSceneFiles.reset(new ProjectStructure(sceneFileExtensions));
}

ProjectManager::~ProjectManager() = default;

bool ProjectManager::IsOpened() const
{
    return (!projectPath.IsEmpty());
}

const DAVA::FilePath& ProjectManager::GetProjectPath() const
{
    return projectPath;
}

const DAVA::FilePath& ProjectManager::GetDataSourcePath() const
{
    return dataSourcePath;
}

const DAVA::FilePath& ProjectManager::GetParticlesConfigPath() const
{
    return particlesConfigPath;
}

const DAVA::FilePath& ProjectManager::GetParticlesDataPath() const
{
    return particlesDataPath;
}

const DAVA::FilePath& ProjectManager::GetWorkspacePath() const
{
    return workspacePath;
}

DAVA::FilePath ProjectManager::ProjectOpenDialog() const
{
    QString newPathStr = FileDialog::getExistingDirectory(NULL, QString("Open Project Folder"), QString("/"));
    if (!newPathStr.isEmpty())
    {
        DAVA::FilePath path = DAVA::FilePath(PathnameToDAVAStyle(newPathStr));
        path.MakeDirectoryPathname();
        return path;
    }

    return DAVA::FilePath();
}

void ProjectManager::OpenProject(const QString& path)
{
    OpenProject(PathnameToDAVAStyle(path));
}

void ProjectManager::OpenProject(const DAVA::FilePath& incomePath)
{
    if (incomePath.IsDirectoryPathname() && incomePath != projectPath)
    {
        CloseProject();

        projectPath = incomePath;

        if (DAVA::FileSystem::Instance()->Exists(incomePath))
        {
            DAVA::FilePath::AddTopResourcesFolder(projectPath + "Data/");

            SettingsManager::SetValue(Settings::Internal_LastProjectPath, DAVA::VariantType(projectPath));

            UpdateInternalValues();
            LoadEditorConfig();

            DAVA::FileSystem::Instance()->CreateDirectory(workspacePath, true);

            SceneValidator::Instance()->SetPathForChecking(projectPath);

            MaterialTemplatesInfo::Instance()->Load("~res:/Materials/assignable.yaml");
            DAVA::QualitySettingsSystem::Instance()->Load("~res:/quality.yaml");
            DAVA::SoundSystem::Instance()->InitFromQualitySettings();

            if (DAVA::FileSystem::Instance()->Exists(dataSourcePath))
            {
                dataSourceSceneFiles->SetProjectDirectory(dataSourcePath);
            }

            bool reloadParticles = SettingsManager::GetValue(Settings::General_ReloadParticlesOnPojectOpening).AsBool();
            if (spritesPacker != nullptr && reloadParticles)
            {
                //emit ProjectOpened will be done later
                spritesPacker->RepackImmediately(projectPath, static_cast<DAVA::eGPUFamily>(SettingsManager::GetValue(Settings::Internal_SpriteViewGPU).AsUInt32()));
            }
            else
            {
                emit ProjectOpened(projectPath.GetAbsolutePathname().c_str());
            }
        }
    }
}

void ProjectManager::SetSpritesPacker(SpritesPackerModule* spritesPacker_)
{
    if (spritesPacker != nullptr)
    {
        disconnect(spritesPacker, &SpritesPackerModule::SpritesReloaded, this, &ProjectManager::OnSpritesReloaded);
    }

    spritesPacker = spritesPacker_;

    if (spritesPacker != nullptr)
    {
        connect(spritesPacker, &SpritesPackerModule::SpritesReloaded, this, &ProjectManager::OnSpritesReloaded);
    }
}

void ProjectManager::OpenLastProject()
{
    DAVA::FilePath path = SettingsManager::GetValue(Settings::Internal_LastProjectPath).AsFilePath();
    if (!path.IsEmpty())
    {
        OpenProject(path);
    }
}

void ProjectManager::CloseProject()
{
    if (!projectPath.IsEmpty())
    {
        DAVA::FilePath::RemoveResourcesFolder(projectPath + "Data/");

        projectPath = "";
        UpdateInternalValues();

        SettingsManager::ResetPerProjectSettings();
        SettingsManager::SetValue(Settings::Internal_LastProjectPath, DAVA::VariantType(DAVA::FilePath())); // reset last project path

        emit ProjectClosed();
    }
}

void ProjectManager::OnSpritesReloaded()
{
    emit ProjectOpened(projectPath.GetAbsolutePathname().c_str());
}

void ProjectManager::LoadEditorConfig()
{
    EditorConfig::Instance()->ParseConfig(projectPath + "EditorConfig.yaml");
}

void ProjectManager::UpdateInternalValues()
{
    if (projectPath.IsEmpty())
    {
        dataSourcePath = "";
        particlesConfigPath = "";
        particlesDataPath = "";
        workspacePath = "";
    }
    else
    {
        dataSourcePath = projectPath + "DataSource/3d/";
        particlesConfigPath = projectPath + "Data/Configs/Particles/";
        particlesDataPath = projectPath + "Data/Gfx/Particles/";
        workspacePath = "~doc:/ResourceEditor/" + projectPath.GetLastDirectoryName() + "/";
    }
}

DAVA::FilePath ProjectManager::CreateProjectPathFromPath(const DAVA::FilePath& pathname)
{
    DAVA::String fullPath = pathname.GetAbsolutePathname();
    DAVA::String::size_type pos = fullPath.find("/Data");
    if (pos != DAVA::String::npos)
    {
        return fullPath.substr(0, pos + 1);
    }

    return DAVA::FilePath();
}

const ProjectStructure* ProjectManager::GetDataSourceSceneFiles() const
{
    return dataSourceSceneFiles.get();
}
