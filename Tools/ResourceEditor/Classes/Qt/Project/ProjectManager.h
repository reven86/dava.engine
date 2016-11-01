#pragma once

#include "Base/Singleton.h"
#include "FileSystem/FilePath.h"

#include <QObject>
#include <QVector>

#include <memory>

class SpritesPackerModule;
class ProjectStructure;
class ProjectManager : public QObject, public DAVA::Singleton<ProjectManager>
{
    Q_OBJECT

public:
    struct AvailableMaterialTemplate
    {
        QString name;
        QString path;
        QVector<QString> qualities;
    };

    ProjectManager();
    ~ProjectManager();

    bool IsOpened() const;

    const DAVA::FilePath& GetProjectPath() const;
    const DAVA::FilePath& GetDataSourcePath() const;
    const DAVA::FilePath& GetParticlesConfigPath() const;
    const DAVA::FilePath& GetParticlesDataPath() const;

    const DAVA::FilePath& GetWorkspacePath() const;

    const QVector<ProjectManager::AvailableMaterialTemplate>* GetAvailableMaterialTemplates() const;

    static DAVA::FilePath CreateProjectPathFromPath(const DAVA::FilePath& pathname);

    void SetSpritesPacker(SpritesPackerModule* spritesPacker);

    DAVA::FilePath ProjectOpenDialog() const;
    void OpenProject(const QString& path);
    void OpenProject(const DAVA::FilePath& path);
    void OpenLastProject();
    void CloseProject();

    ProjectStructure* GetDataSourceSceneFiles() const;

signals:
    void ProjectOpened(const QString& path);
    void ProjectClosed();

public slots:

    void OnSpritesReloaded();

private:
    void LoadProjectSettings();
    void LoadMaterialsSettings();

    void UpdateInternalValues();

    std::unique_ptr<ProjectStructure> dataSourceSceneFiles;

    DAVA::FilePath projectPath;
    DAVA::FilePath dataSourcePath;
    DAVA::FilePath particlesConfigPath;
    DAVA::FilePath particlesDataPath;
    DAVA::FilePath workspacePath;

    QVector<AvailableMaterialTemplate> templates;

    SpritesPackerModule* spritesPacker = nullptr;
};
