#ifndef QUICKED_BASECONTROLLER_H
#define QUICKED_BASECONTROLLER_H

#include <QObject>
#include <QUndoGroup>
#include "Project.h"
#include "UI/mainwindow.h"
#include "Ui/Document.h"

class QAction;

class BaseController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int count READ Count WRITE SetCount NOTIFY CountChanged)
    Q_PROPERTY(int currentIndex READ CurrentIndex WRITE SetCurrentIndex NOTIFY CurrentIndexChanged)
public:
    explicit BaseController(QObject *parent = nullptr);
    ~BaseController();
    void Start();

    MainWindow *GetMainWindow() const;

protected slots:
    void OnCleanChanged(bool clean);
    void OnSelectionRootControlChanged(const QList<ControlNode *> &activatedRootControls, const QList<ControlNode *> &deactivatedRootControls);
    void OnSelectionControlChanged(const QList<ControlNode *> &activatedControls, const QList<ControlNode *> &deactivatedControls);
    void OnControlSelectedInEditor(ControlNode *activatedControls);
    void OnAllControlDeselectedInEditor();
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
    void AttachDocument(Document *document);
    void DetachDocument(Document *document);
    void CloseDocument(int index);
    int GetIndexByPackagePath(const QString &fileName) const;
    ///Return: pointer to currentDocument if exists, nullptr if not
    Document* GetCurrentDocument() const;
    QList<Document*> documents;
    MainWindow mainWindow;
    QUndoGroup undoGroup;
    Project project;

    //properties. must be at the end of file. Properties interface declared at the begin of the class
public:
    int Count() const;
    int CurrentIndex() const;

public slots:
    void SetCount(int arg);
    void SetCurrentIndex(int arg);

signals:
    void CountChanged(int arg);
    void CurrentIndexChanged(int arg);

private:
    int count;
    int currentIndex;
};

#endif // QUICKED_BASECONTROLLER_H
