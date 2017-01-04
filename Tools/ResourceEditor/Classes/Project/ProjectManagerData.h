#pragma once

#include "TArc/DataProcessing/DataNode.h"
#include "TArc/DataProcessing/PropertiesHolder.h"

#include "FileSystem/FilePath.h"

#include <QString>
#include <QVector>

class FileSystemCache;
class SpritesPackerModule;
class EditorConfig;
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
    ProjectManagerData(const ProjectManagerData& other) = delete;
    ~ProjectManagerData();

    bool IsOpened() const;

    const DAVA::FilePath& GetProjectPath() const;
    DAVA::FilePath GetDataPath() const;
    DAVA::FilePath GetDataSourcePath() const;
    DAVA::FilePath GetDataSource3DPath() const;
    DAVA::FilePath GetParticlesConfigPath() const;
    DAVA::FilePath GetParticlesGfxPath() const;
    DAVA::FilePath GetWorkspacePath() const;

    const QVector<AvailableMaterialTemplate>& GetAvailableMaterialTemplates() const;
    const QVector<AvailableMaterialQuality>& GetAvailableMaterialQualities() const;

    static DAVA::FilePath CreateProjectPathFromPath(const DAVA::FilePath& pathname);
    const FileSystemCache* GetDataSourceSceneFiles() const;
    const EditorConfig* GetEditorConfig() const;
    DAVA_DEPRECATED(const SpritesPackerModule* GetSpritesModules() const);

public:
    static const DAVA::String ProjectPathProperty;

private:
    friend class ProjectManagerModule;
    std::unique_ptr<FileSystemCache> dataSourceSceneFiles;
    std::unique_ptr<SpritesPackerModule> spritesPacker;
    std::unique_ptr<EditorConfig> editorConfig;

    DAVA::FilePath projectPath;
    QVector<AvailableMaterialTemplate> templates;
    QVector<AvailableMaterialQuality> qualities;

    DAVA_VIRTUAL_REFLECTION(ProjectManagerData, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<ProjectManagerData>::Begin()
        .Field(ProjectPathProperty.c_str(), &ProjectManagerData::GetProjectPath, nullptr)
        .End();
    }
};
