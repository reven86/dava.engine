#ifndef QUICKED__PROJECT_H__
#define QUICKED__PROJECT_H__

#include <QObject>
//#include "Project/EditorFontSystem.h"
//#include "Project/EditorLocalizationSystem.h"
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
}

class Project : public QObject
{
    Q_OBJECT
public:
    struct Settings
    {
        QString projectFile;
        QString sourceResourceDirectory;
        QString intermediateResourceDirectory;

        DAVA::FilePath fontsDirectory;
        DAVA::FilePath textsDirectory;
        DAVA::FilePath fontsConfigsDirectory;
        DAVA::String defaultLanguage;
        DAVA::Vector<DAVA::FilePath> libraryPackages;
    };

    static std::tuple<Settings, DAVA::ResultList> ParseProjectSettings(const QString& projectFile);
    static const QString& GetUIRelativePath();
    static const QString& GetProjectFileName();

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
    void FindFileInProject();

    Settings settings;
    const QString projectDirectory;
    const QString projectName;

    MainWindow::ProjectView* view = nullptr;
    std::unique_ptr<EditorFontSystem> editorFontSystem;
    std::unique_ptr<EditorLocalizationSystem> editorLocalizationSystem;
    std::unique_ptr<DocumentGroup> documentGroup;
    std::unique_ptr<SpritesPacker> spritesPacker;
    std::unique_ptr<ProjectStructure> projectStructure;
};

#endif // QUICKED__PROJECT_H__
