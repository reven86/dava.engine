#pragma once

#include <QObject>
#include <QVector>

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
    };

    struct AvailableMaterialQuality
    {
        QString name;
        QString prefix;
        QVector<QString> values;
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
    const QVector<ProjectManager::AvailableMaterialQuality>* GetAvailableMaterialQualities() const;

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
    QVector<AvailableMaterialQuality> qualities;

    SpritesPackerModule* spritesPacker = nullptr;
};
