//
//  UIPackageDocument.h
//  UIEditor
//
//  Created by Alexey Strokachuk on 9/17/14.
//
//

#ifndef __UIEditor__UIPackageDocument__
#define __UIEditor__UIPackageDocument__

#include <QWidget>
#include <QVariant>
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"

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

class UIPackageModel;
class PackageNode;
class DavaGLWidget;

class TreeViewContext
{
public:
    QPoint scrollPosition;
    //QModelIndexList expandedItems;
    UIPackageModel *model;
    QSortFilterProxyModel *proxyModel;
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

class PackageDocument: public QObject
{
    Q_OBJECT
public:
    PackageDocument(PackageNode *package, QObject *parent = NULL);
    ~PackageDocument();
    
    bool IsModified() const;
    void ClearModified();
    const DAVA::FilePath &PackageFilePath() const;
    PackageNode *Package() const {return package;}
    
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

signals:
    void controlsSelectionChanged(const QList<ControlNode *> &activatedControls, const QList<ControlNode *> &deactivatedControls);
    void activeRootControlsChanged(const QList<ControlNode *> &activatedRootControls, const QList<ControlNode *> &deactivatedRootControls);
    
public slots:
    void OnSelectionRootControlChanged(const QList<ControlNode *> &activatedRootControls, const QList<ControlNode *> &deactivatedRootControls);
    void OnSelectionControlChanged(const QList<ControlNode *> &activatedControls, const QList<ControlNode *> &deactivatedControls);

private:
    void UpdateControlCanvas();
    
private:
    PackageNode *package;
    QList<ControlNode *> selectedControls;
    QList<ControlNode *> activeRootControls;
    TreeViewContext treeContext;
    GraphicsViewContext *graphicsContext;
    PropertiesViewContext *propertiesContext;
    LibraryViewContext libraryContext;

    QUndoStack *undoStack;
};

Q_DECLARE_METATYPE(PackageDocument *);

#endif /* defined(__UIEditor__UIPackageDocument__) */
