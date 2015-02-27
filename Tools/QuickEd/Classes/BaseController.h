#ifndef BASECONTROLLER_H
#define BASECONTROLLER_H

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
    explicit BaseController(QObject *parent = 0);
    ~BaseController();
    void start();
protected:
    void CloseProject();
    void OpenProject(const QString &path);

    int CreateScene(PackageNode *package);
signals:

public slots:
protected slots:
    void OnSelectionRootControlChanged(const QList<ControlNode *> &activatedRootControls, const QList<ControlNode *> &deactivatedRootControls);
    void OnSelectionControlChanged(const QList<ControlNode *> &activatedControls, const QList<ControlNode *> &deactivatedControls);
    void OnControlSelectedInEditor(ControlNode *activatedControls);
    void OnAllControlDeselectedInEditor();
    void OnOpenPackageFile(QString path);

    void CloseScene(int index);
    void SaveDocument(int index);
    void SaveAllDocuments();

    void Exit();
    void RecentMenu(QAction *);
private:
    int GetIndexByPackagePath(const QString &fileName) const;
    QList<std::shared_ptr<Document> > documents;
    MainWindow mainWindow;
    QUndoGroup undoGroup;
    Project project;

//properties
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

#endif // BASECONTROLLER_H
