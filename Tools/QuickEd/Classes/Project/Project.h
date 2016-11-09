#ifndef QUICKED__PROJECT_H__
#define QUICKED__PROJECT_H__

#include "ProjectProperties.h"

#include "Base/Result.h"
#include "Preferences/PreferencesRegistrator.h"

#include <QObject>
#include <QVector>
#include <QPair>

#include "UI/mainwindow.h"

class EditorFontSystem;
class EditorLocalizationSystem;
class DocumentGroup;
class MainWindow;
class SpritesPacker;
class FileSystemCache;

namespace DAVA
{
class AssetCacheClient;
class YamlNode;
}

class Project : public QObject
{
    Q_OBJECT
public:
    static std::tuple<DAVA::ResultList, ProjectProperties> ParseProjectPropertiesFromFile(const QString& projectFile);
    static bool EmitProjectPropertiesToFile(const ProjectProperties& settings);

    static const QStringList& GetFontsFileExtensionFilter();
    static const QString& GetGraphicsFileExtension();
    static const QString& Get3dFileExtension();
    static const QString& GetUiFileExtension();

    Project(MainWindow::ProjectView* view, const ProjectProperties& properties);
    ~Project();

    void SetAssetCacheClient(DAVA::AssetCacheClient* newCacheClient);

    QString GetProjectPath() const;
    const QString& GetProjectDirectory() const;
    const QString& GetProjectName() const;
    QString GetResourceDirectory() const;

    QStringList GetAvailableLanguages() const;
    QString GetCurrentLanguage() const;
    void SetCurrentLanguage(const QString& newLanguageCode);

    const QStringList& GetDefaultPresetNames() const;

    EditorFontSystem* GetEditorFontSystem() const;

    void SetRtl(bool isRtl);
    void SetBiDiSupport(bool support);
    void SetGlobalStyleClasses(const QString& classesStr);

    DAVA::Vector<ProjectProperties::ResDir> GetLibraryPackages() const;

    bool TryCloseAllDocuments();

signals:
    void CurrentLanguageChanged(const QString& newLanguageCode);

private:
    void OnReloadSprites();
    void OnReloadSpritesFinished();
    void OnFindFileInProject();

    ProjectProperties properties;
    const QString projectDirectory;
    const QString projectName;
    QString uiResourcesPath;

    MainWindow::ProjectView* view = nullptr;
    std::unique_ptr<EditorFontSystem> editorFontSystem;
    std::unique_ptr<EditorLocalizationSystem> editorLocalizationSystem;
    std::unique_ptr<DocumentGroup> documentGroup;
    std::unique_ptr<SpritesPacker> spritesPacker;
    std::unique_ptr<FileSystemCache> projectStructure;
};

#endif // QUICKED__PROJECT_H__
