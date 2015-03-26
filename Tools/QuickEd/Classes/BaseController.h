#ifndef QUICKED_BASECONTROLLER_H
#define QUICKED_BASECONTROLLER_H

#include <QObject>
#include <QUndoGroup>
#include "Project.h"
#include "UI/mainwindow.h"
#include "DocumentGroup.h"

class QAction;
class Document;
class BaseController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ CurrentIndex WRITE SetCurrentIndex NOTIFY CurrentIndexChanged)
public:
    explicit BaseController(QObject *parent = nullptr);
    ~BaseController();
    void Start();

    MainWindow *GetMainWindow() const;

protected slots:
    void OnCleanChanged(bool clean);
    void OnOpenPackageFile(const QString &path);

    bool CloseOneDocument(int index);
    void SaveDocument(int index);
    void SaveAllDocuments();

    void Exit();
    void RecentMenu(QAction *);

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
    QList<Document*> documents;
    DocumentGroup *documentGroup;
    Project *project;
    MainWindow *mainWindow;

    //properties. must be at the end of file. Properties interface declared at the begin of the class
public:
    int CurrentIndex() const;

public slots:
    void SetCurrentIndex(int arg);

signals:
    void CurrentIndexChanged(int arg);

private:
    int currentIndex;
};

#endif // QUICKED_BASECONTROLLER_H
