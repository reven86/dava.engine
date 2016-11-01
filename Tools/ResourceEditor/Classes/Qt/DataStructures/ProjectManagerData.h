#pragma once

#include "TArc/DataProcessing/DataNode.h"
#include "TArc/DataProcessing/PropertiesHolder.h"

class ProjectManagerData : public DAVA::TArc::DataNode
{
public:
    struct AvailableMaterialTemplate
    {
        QString name;
        QString path;
    };

    struct AvailableMaterialQuality
    {
        QString name;
        QString prefix;
        QVector<QString> values;
    };

    ProjectManagerData();

    bool IsOpened() const;

    const DAVA::FilePath& GetProjectPath() const;
    DAVA::FilePath GetDataPath() const;
    DAVA::FilePath GetDataSourcePath() const;
    DAVA::FilePath GetParticlesConfigPath() const;
    DAVA::FilePath GetParticlesDataPath() const;
    DAVA::FilePath GetWorkspacePath() const;

    const QVector<AvailableMaterialTemplate>& GetAvailableMaterialTemplates() const;
    const QVector<AvailableMaterialQuality>& GetAvailableMaterialQualities() const;

    DAVA::FilePath CreateProjectPathFromPath(const DAVA::FilePath& pathname);
    const ProjectStructure* GetDataSourceSceneFiles() const;

private:
    friend class ProjectManagerModule;
    DAVA::TArc::PropertiesItem properties;

    std::unique_ptr<ProjectStructure> dataSourceSceneFiles;
    std::unique_ptr<SpritesPackerModule> spritesPacker;

    DAVA::FilePath projectPath;
    QVector<AvailableMaterialTemplate> templates;
    QVector<AvailableMaterialQuality> qualities;

    DAVA_VIRTUAL_REFLECTION(ProjectManagerData)
    {
        DAVA::ReflectionRegistrator<ProjectManagerData>::Begin()
        .Field("ProjectPath", &ProjectManagerData::GetProjectPath, nullptr)
        .End();
    }
};