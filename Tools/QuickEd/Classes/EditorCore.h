#ifndef QUICKED_BASECONTROLLER_H
#define QUICKED_BASECONTROLLER_H

#include <QObject>
#include "UI/mainwindow.h"
#include "Project/Project.h"
#include "Base/Singleton.h"

class QAction;
class Document;
class DocumentGroup;
class Project;
class MainWindow;
class PackageNode;

class EditorCore : public QObject, public DAVA::Singleton<EditorCore>
{
    Q_OBJECT
public:
    explicit EditorCore(QObject *parent = nullptr);
    ~EditorCore();
    void Start();

    MainWindow *GetMainWindow() const;
    Project *GetProject() const;

protected slots:
    void OnCleanChanged(bool clean);
    void OnOpenPackageFile(const QString &path);

    bool CloseOneDocument(int index);
    void SaveDocument(int index);
    void SaveAllDocuments();

    void Exit();
    void RecentMenu(QAction *);
    void OnCurrentTabChanged(int index);
    
    void UpdateLanguage();

protected:
    void OpenProject(const QString &path);
    bool CloseProject();
    int CreateDocument(PackageNode *package);
    void SaveDocument(Document *document);

private:
    bool eventFilter( QObject *obj, QEvent *event ) override;
    void CloseDocument(int index);
    int GetIndexByPackagePath(const QString &fileName) const;
    ///Return: pointer to currentDocument if exists, nullptr if not
    Project *project;
    QList<Document*> documents;
    DocumentGroup *documentGroup;
    MainWindow *mainWindow;
};

inline MainWindow* EditorCore::GetMainWindow() const
{
    return const_cast<MainWindow*>(mainWindow);
}

inline Project* EditorCore::GetProject() const
{
    return project;
}

inline EditorLocalizationSystem *GetEditorLocalizationSystem()
{
    return EditorCore::Instance()->GetProject()->GetEditorLocalizationSystem();
}

inline EditorFontSystem *GetEditorFontSystem()
{
    return EditorCore::Instance()->GetProject()->GetEditorFontSystem();
}

#endif // QUICKED_BASECONTROLLER_H
