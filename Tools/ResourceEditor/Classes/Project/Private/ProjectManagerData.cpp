#include "Classes/Project/ProjectManagerData.h"

#include "Classes/Deprecated/EditorConfig.h"
#include "QtTools/ProjectInformation/FileSystemCache.h"
#include "SpritesPacker/SpritesPackerModule.h"

namespace ProjectManagerDataDetails
{
const char* DATA_PATH = "Data/";
const char* DATASOURCE_3D_PATH = "DataSource/3d/";
const char* PARTICLE_CONFIG_PATH = "DataSource/Configs/Particles/";
const char* PARTICLE_GFX_PATH = "DataSource/Gfx/Particles/";
const char* WORKSPACE_PATH = "~doc:/ResourceEditor/";
}

const DAVA::String ProjectManagerData::ProjectPathProperty = DAVA::String("ProjectPath");

ProjectManagerData::ProjectManagerData()
{
}

ProjectManagerData::~ProjectManagerData() = default;

bool ProjectManagerData::IsOpened() const
{
    return (!projectPath.IsEmpty());
}

const DAVA::FilePath& ProjectManagerData::GetProjectPath() const
{
    return projectPath;
}

DAVA::FilePath ProjectManagerData::GetDataPath() const
{
    return projectPath + ProjectManagerDataDetails::DATA_PATH;
}

DAVA::FilePath ProjectManagerData::GetDataSource3DPath() const
{
    return projectPath + ProjectManagerDataDetails::DATASOURCE_3D_PATH;
}

DAVA::FilePath ProjectManagerData::GetParticlesConfigPath() const
{
    return projectPath + ProjectManagerDataDetails::PARTICLE_CONFIG_PATH;
}

DAVA::FilePath ProjectManagerData::GetParticlesGfxPath() const
{
    return projectPath + ProjectManagerDataDetails::PARTICLE_GFX_PATH;
}

DAVA::FilePath ProjectManagerData::GetWorkspacePath() const
{
    return DAVA::FilePath(ProjectManagerDataDetails::WORKSPACE_PATH + projectPath.GetLastDirectoryName() + "/");
}

const QVector<ProjectManagerData::AvailableMaterialTemplate>& ProjectManagerData::GetAvailableMaterialTemplates() const
{
    return templates;
}

const QVector<ProjectManagerData::AvailableMaterialQuality>& ProjectManagerData::GetAvailableMaterialQualities() const
{
    return qualities;
}

DAVA::FilePath ProjectManagerData::CreateProjectPathFromPath(const DAVA::FilePath& pathname)
{
    DAVA::String fullPath = pathname.GetAbsolutePathname();
    DAVA::String::size_type pos = fullPath.find("/Data");
    if (pos != DAVA::String::npos)
    {
        return fullPath.substr(0, pos + 1);
    }

    return DAVA::FilePath();
}

const FileSystemCache* ProjectManagerData::GetDataSourceSceneFiles() const
{
    return dataSourceSceneFiles.get();
}

const EditorConfig* ProjectManagerData::GetEditorConfig() const
{
    return editorConfig.get();
}

const SpritesPackerModule* ProjectManagerData::GetSpritesModules() const
{
    return spritesPacker.get();
}
