#ifndef QUICKED__PROJECT_H__
#define QUICKED__PROJECT_H__

#include <QObject>
#include "Project/EditorFontSystem.h"
#include "Project/EditorLocalizationSystem.h"
#include "Base/Result.h"
#include "Preferences/PreferencesRegistrator.h"

class Project : public QObject
{
    Q_OBJECT
    //     Q_PROPERTY(QString projectPath READ GetProjectPath NOTIFY ProjectPathChanged)
    //     Q_PROPERTY(QString projectName READ GetProjectName NOTIFY ProjectNameChanged)

public:
    struct Settings
    {
        const DAVA::FilePath projectPath;
        DAVA::FilePath fontsPath;
        DAVA::FilePath stringLocalizationsPath;
        DAVA::String currentLocale;
        DAVA::Vector<DAVA::FilePath> libraryPackages;
        DAVA::Vector<std::pair<DAVA::String, DAVA::FilePath>> dataFolders;
    };

    Project(const Settings& aSettings);
    ~Project();

public:
    QString GetProjectPath() const;
    QString GetProjectName() const;

    //bool Open(const QString& path);
    //void Close();
    //bool CanOpenProject(const QString& path) const;

    EditorFontSystem* GetEditorFontSystem() const;
    EditorLocalizationSystem* GetEditorLocalizationSystem() const;
    const DAVA::Vector<DAVA::FilePath>& GetLibraryPackages() const;
    static const QString& GetScreensRelativePath();
    static const QString& GetProjectFileName();
    //QString CreateNewProject(DAVA::Result* result = nullptr);

private:
    //bool OpenInternal(const QString& path);

signals:
    //void IsOpenChanged(bool arg);
    //void ProjectPathChanged(QString arg);
    //void ProjectNameChanged(QString arg);

private:
    //void SetProjectPath(QString arg);
    //void SetProjectName(QString arg);

    Settings settings;
    const DAVA::FilePath projectDirectory;
    const DAVA::String projectName;

    std::unique_ptr<EditorFontSystem> editorFontSystem;
    std::unique_ptr<EditorLocalizationSystem> editorLocalizationSystem;
};

#endif // QUICKED__PROJECT_H__
