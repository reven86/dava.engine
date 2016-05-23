#ifndef QUICKED__PROJECT_H__
#define QUICKED__PROJECT_H__

#include <QObject>
#include "Project/EditorFontSystem.h"
#include "Project/EditorLocalizationSystem.h"
#include "Base/Result.h"

class PackageNode;
class QFileInfo;

class Project : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isOpen READ IsOpen NOTIFY IsOpenChanged)
    Q_PROPERTY(QString projectPath READ GetProjectPath NOTIFY ProjectPathChanged)
    Q_PROPERTY(QString projectName READ GetProjectName NOTIFY ProjectNameChanged)

public:
    explicit Project(QObject* parent = nullptr);
    virtual ~Project();
    bool Open(const QString& path);
    void Close();
    bool CanOpenProject(const QString& path) const;

    EditorFontSystem* GetEditorFontSystem() const;
    EditorLocalizationSystem* GetEditorLocalizationSystem() const;
    static const QString& GetScreensRelativePath();
    static const QString& GetProjectFileName();
    QString CreateNewProject(DAVA::Result* result = nullptr);

private:
    bool OpenInternal(const QString& path);

    EditorFontSystem* editorFontSystem;
    EditorLocalizationSystem* editorLocalizationSystem;

    //properties
public:
    bool IsOpen() const;
    QString GetProjectPath() const;
    QString GetProjectName() const;

signals:
    void IsOpenChanged(bool arg);
    void ProjectPathChanged(QString arg);
    void ProjectNameChanged(QString arg);

private:
    void SetProjectPath(QString arg);
    void SetProjectName(QString arg);
    void SetIsOpen(bool arg);

    bool isOpen = false;
    DAVA::FilePath projectPath;
    QString projectName;
};

#endif // QUICKED__PROJECT_H__
