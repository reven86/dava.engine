#ifndef __QUICKED_PACKAGE_DOCUMENT_H__
#define __QUICKED_PACKAGE_DOCUMENT_H__

#include <QWidget>
#include <QVariant>
#include <QPointer>
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Project.h"

namespace Ui {
    class PackageDocument;
}

namespace DAVA {
    class UIPackage;
    class FilePath;
    class UIControl;
    class UIScreen;
}

class QAbstractItemModel;
class QSortFilterProxyModel;
class QUndoStack;
class QItemSelection;

class UIPackageModel;
class PackageNode;
class DavaGLWidget;
class QtModelPackageCommandExecutor;

class TreeViewContext
{
public:
    QPoint scrollPosition;
    //QModelIndexList expandedItems;
    UIPackageModel *model;
    QSortFilterProxyModel *proxyModel;
    QItemSelection *currentSelection;
    QString filterString;
};

class LibraryViewContext
{
public:
    QPoint scrollPosition;
    //QModelIndexList expandedItems;
    QAbstractItemModel *model;
};

class GraphicsViewContext;
class PropertiesViewContext;
class PackageNode;
class ControlNode;

class Document : public QObject
{
    Q_OBJECT
public:
    Document(Project * project, PackageNode *package, QObject *parent = NULL);
    virtual ~Document();
    
    bool IsModified() const;
    void ClearModified();
    const DAVA::FilePath &PackageFilePath() const;
    PackageNode *GetPackage() const {return package;}
    Project *GetProject() const { return project; }
    
    const QList<ControlNode*> &GetSelectedControls() const { return selectedControls; }
    const QList<ControlNode*> &GetActiveRootControls() const { return activeRootControls; }
    
    const TreeViewContext *GetTreeContext() const { return &treeContext; };
    const GraphicsViewContext *GetGraphicsContext() const {return graphicsContext; };
    const PropertiesViewContext *GetPropertiesContext() const {return propertiesContext; };
    const LibraryViewContext *GetLibraryContext() const {return &libraryContext; };

    TreeViewContext *GetTreeContext() { return &treeContext; };
    GraphicsViewContext *GetGraphicsContext() {return graphicsContext; };
    PropertiesViewContext *GetPropertiesContext() {return propertiesContext; };
    LibraryViewContext *GetLibraryContext() {return &libraryContext; };

    QUndoStack *UndoStack() const { return undoStack; }
    
    QtModelPackageCommandExecutor *GetCommandExecutor() const;

signals:
    void controlsSelectionChanged(const QList<ControlNode *> &activatedControls, const QList<ControlNode *> &deactivatedControls);
    void activeRootControlsChanged(const QList<ControlNode *> &activatedRootControls, const QList<ControlNode *> &deactivatedRootControls);

    void controlSelectedInEditor(ControlNode *activatedControls);
    void allControlsDeselectedInEditor();

public slots:
    void OnSelectionRootControlChanged(const QList<ControlNode *> &activatedRootControls, const QList<ControlNode *> &deactivatedRootControls);
    void OnSelectionControlChanged(const QList<ControlNode *> &activatedControls, const QList<ControlNode *> &deactivatedControls);

    void OnControlSelectedInEditor(ControlNode *activatedControls);
    void OnAllControlDeselectedInEditor();

private:
    void UpdateControlCanvas();
    
private:
    QPointer<Project> project;
    PackageNode *package;
    QList<ControlNode *> selectedControls;
    QList<ControlNode *> activeRootControls;
    TreeViewContext treeContext;
    GraphicsViewContext *graphicsContext;
    PropertiesViewContext *propertiesContext;
    LibraryViewContext libraryContext;
    QtModelPackageCommandExecutor *commandExecutor;

    QUndoStack *undoStack;
};

Q_DECLARE_METATYPE(Document *);

#endif // __QUICKED_PACKAGE_DOCUMENT_H__
