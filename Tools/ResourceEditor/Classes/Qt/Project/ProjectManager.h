#ifndef __PROJECT_MANAGER_H__
#define __PROJECT_MANAGER_H__

#include <QObject>
#include <QVector>
#include "DAVAEngine.h"

class SpritesPackerModule;
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

    ProjectManager() = default;
    ~ProjectManager() = default;

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

signals:
    void ProjectOpened(const QString& path);
    void ProjectClosed();

public slots:

    void OnSpritesReloaded();

private:
    void LoadProjectSettings();
    void LoadMaterialsSettings();

    void UpdateInternalValues();

    DAVA::FilePath projectPath;
    DAVA::FilePath dataSourcePath;
    DAVA::FilePath particlesConfigPath;
    DAVA::FilePath particlesDataPath;
    DAVA::FilePath workspacePath;

    QVector<AvailableMaterialTemplate> templates;
    QVector<AvailableMaterialQuality> qualities;

    SpritesPackerModule* spritesPacker = nullptr;
};

#endif // __PROJECT_MANAGER_H__
