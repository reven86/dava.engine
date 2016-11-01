#ifndef QUICKED__PROJECT_H__
#define QUICKED__PROJECT_H__

#include <QObject>

#include "Base/Result.h"
#include "Preferences/PreferencesRegistrator.h"

#include <QVector>
#include <QPair>

#include "UI/mainwindow.h"

class EditorFontSystem;
class EditorLocalizationSystem;
class DocumentGroup;
class MainWindow;
class SpritesPacker;
class ProjectStructure;

namespace DAVA
{
class AssetCacheClient;
class YamlNode;
}

class Project : public QObject
{
    Q_OBJECT
public:
    struct Settings
    {
        QString projectFile;
        QString resourceDirectory;
        QString additionalResourceDirectory;
        QString intermediateResourceDirectory;

        DAVA::FilePath fontsDirectory;
        DAVA::FilePath textsDirectory;
        DAVA::FilePath fontsConfigsDirectory;
        DAVA::String defaultLanguage;
        DAVA::Vector<std::pair<QString, QSize>> gfxDirectories;
        DAVA::Vector<DAVA::FilePath> libraryPackages;
    };

    static const int CURRENT_PROJECT_FILE_VERSION = 1;

    static std::tuple<Settings, DAVA::ResultList> ParseProjectSettings(const QString& projectFile);
    static const QString& GetUIRelativePath();
    static const QString& GetProjectFileName();

    static const QStringList& GetFontsFileExtensionFilter();
    static const QString& GetGraphicsFileExtensionFilter();
    static const QString& Get3dFileExtensionFilter();
    static const QString& GetUIFileExtensionFilter();
    static const QString& GetUIFileExtension();

    Project(MainWindow::ProjectView* aView, const Settings& aSettings);
    ~Project();

    void SetAssetCacheClient(DAVA::AssetCacheClient* newCacheClient);

    QString GetProjectPath() const;
    QString GetProjectDirectory() const;
    QString GetProjectName() const;

    QStringList GetAvailableLanguages() const;
    QString GetCurrentLanguage() const;
    void SetCurrentLanguage(const QString& newLanguageCode);

    const QStringList& GetDefaultPresetNames() const;

    EditorFontSystem* GetEditorFontSystem() const;

    void SetRtl(bool isRtl);
    void SetBiDiSupport(bool support);
    void SetGlobalStyleClasses(const QString& classesStr);

    const DAVA::Vector<DAVA::FilePath>& GetLibraryPackages() const;

    const QString& SourceResourceDirectory() const;

    void OnReloadSprites();

    void OnReloadSpritesFinished();

    bool TryCloseAllDocuments();

signals:
    void CurrentLanguageChanged(const QString& newLanguageCode);

private:
    static std::tuple<Settings, DAVA::ResultList> ParseActualProjectSettings(const QString& projectFile, const DAVA::YamlNode* root);
    static std::tuple<Settings, DAVA::ResultList> ParseLegacyProjectSettings(const QString& projectFile, const DAVA::YamlNode* root, int version);
    void FindFileInProject();

    Settings settings;
    const QString projectDirectory;
    const QString projectName;
    QString uiResourcesPath;

    MainWindow::ProjectView* view = nullptr;
    std::unique_ptr<EditorFontSystem> editorFontSystem;
    std::unique_ptr<EditorLocalizationSystem> editorLocalizationSystem;
    std::unique_ptr<DocumentGroup> documentGroup;
    std::unique_ptr<SpritesPacker> spritesPacker;
    std::unique_ptr<ProjectStructure> projectStructure;
};

#endif // QUICKED__PROJECT_H__
