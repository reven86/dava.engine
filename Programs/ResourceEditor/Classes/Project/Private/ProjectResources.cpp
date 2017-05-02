#include "Classes/Project/ProjectResources.h"
#include "Classes/Project/ProjectManagerData.h"

#include <TArc/Core/ContextAccessor.h>

#include <Engine/Engine.h>
#include <FileSystem/FileSystem.h>
#include <FileSystem/YamlParser.h>
#include <FileSystem/YamlNode.h>
#include <FileSystem/FilePath.h>
#include <Scene3D/Systems/QualitySettingsSystem.h>
#include <Sound/SoundSystem.h>

#include <QStringList>

namespace ProjectResourcesDetails
{
using namespace DAVA;

Vector<String> LoadMaterialQualities(FilePath fxPath)
{
    Vector<String> qualities;
    ScopedPtr<YamlParser> parser(YamlParser::Create(fxPath));
    if (parser)
    {
        YamlNode* rootNode = parser->GetRootNode();
        if (rootNode)
        {
            const YamlNode* materialTemplateNode = rootNode->Get("MaterialTemplate");
            if (materialTemplateNode)
            {
                static const char* QUALITIES[] = { "LOW", "MEDIUM", "HIGH", "ULTRA_HIGH" };
                for (const char* quality : QUALITIES)
                {
                    const YamlNode* qualityNode = materialTemplateNode->Get(quality);
                    if (qualityNode)
                    {
                        qualities.push_back(quality);
                    }
                }
            }
        }
    }

    return qualities;
}

static const FilePath PATH_TO_ASSIGNABLE_YAML = "~res:/Materials/assignable.yaml"; // file that contains info about available materials

void LoadMaterialTemplatesInfo(Vector<MaterialTemplateInfo>& templates)
{
    templates.clear();

    if (FileSystem::Instance()->Exists(PATH_TO_ASSIGNABLE_YAML))
    {
        ScopedPtr<YamlParser> parser(YamlParser::Create(PATH_TO_ASSIGNABLE_YAML));
        YamlNode* rootNode = parser->GetRootNode();

        if (nullptr != rootNode)
        {
            FilePath materialsListDir = PATH_TO_ASSIGNABLE_YAML.GetDirectory();

            for (uint32 i = 0; i < rootNode->GetCount(); ++i)
            {
                const YamlNode* templateNode = rootNode->Get(i);
                if (nullptr != templateNode)
                {
                    const YamlNode* name = templateNode->Get("name");
                    const YamlNode* path = templateNode->Get("path");

                    if (nullptr != name && nullptr != path &&
                        name->GetType() == YamlNode::TYPE_STRING &&
                        path->GetType() == YamlNode::TYPE_STRING)
                    {
                        const FilePath templatePath = materialsListDir + path->AsString();
                        if (FileSystem::Instance()->Exists(templatePath))
                        {
                            MaterialTemplateInfo info;

                            info.name = name->AsString().c_str();
                            info.path = templatePath.GetFrameworkPath().c_str();
                            info.qualities = ProjectResourcesDetails::LoadMaterialQualities(templatePath);

                            templates.push_back(info);
                        }
                    }
                }
            }
        }
    }
}
}

ProjectResources::ProjectResources(DAVA::TArc::ContextAccessor* accessor)
    : accessor(accessor)
{
    DAVA::TArc::DataContext* globalContext = accessor->GetGlobalContext();
    std::unique_ptr<ProjectManagerData> data(new ProjectManagerData);

    QStringList extensions = { "sc2" };
    globalContext->CreateData(std::move(data));
}

ProjectResources::~ProjectResources()
{
    UnloadProject();
    DAVA::TArc::DataContext* globalContext = accessor->GetGlobalContext();
    globalContext->DeleteData<ProjectManagerData>();
}

ProjectManagerData* ProjectResources::GetProjectManagerData()
{
    DAVA::TArc::DataContext* ctx = accessor->GetGlobalContext();
    return ctx->GetData<ProjectManagerData>();
}

void ProjectResources::LoadProject(const DAVA::FilePath& incomePath)
{
    ProjectManagerData* data = GetProjectManagerData();

    if (incomePath.IsDirectoryPathname() && incomePath != data->projectPath)
    {
        UnloadProject();

        data->projectPath = incomePath;
        DAVA::FilePath::AddResourcesFolder(data->GetDataSourcePath());
        DAVA::FilePath::AddTopResourcesFolder(data->GetDataPath());

        ProjectResourcesDetails::LoadMaterialTemplatesInfo(data->materialTemplatesInfo);

        DAVA::QualitySettingsSystem::Instance()->Load(data->projectPath + "DataSource/quality.yaml");

        const DAVA::EngineContext* engineCtx = accessor->GetEngineContext();
        engineCtx->soundSystem->InitFromQualitySettings();
    }
}

void ProjectResources::UnloadProject()
{
    ProjectManagerData* data = GetProjectManagerData();

    if (!data->projectPath.IsEmpty())
    {
        const DAVA::EngineContext* engineCtx = accessor->GetEngineContext();
        engineCtx->soundSystem->UnloadFMODProjects();

        DAVA::FilePath::RemoveResourcesFolder(data->GetDataPath());
        DAVA::FilePath::RemoveResourcesFolder(data->GetDataSourcePath());
        data->projectPath = "";
    }
}
