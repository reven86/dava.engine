#ifndef __QUICKED_PACKAGE_DOCUMENT_H__
#define __QUICKED_PACKAGE_DOCUMENT_H__

#include <QWidget>
#include <QVariant>
#include <QPointer>
#include <QUndoStack>
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
class QItemSelection;

class UIPackageModel;
class PackageNode;
class DavaGLWidget;
class QtModelPackageCommandExecutor;

struct PackageContext;
class PropertiesContext;
class PreviewContext;

class PackageModel;
class PropertiesModel;

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
    PackageNode *GetPackage() const;
    Project *GetProject() const;
    
    const QList<ControlNode*> &GetSelectedControls() const;
    const QList<ControlNode*> &GetActiveRootControls() const;
    PackageModel* GetPackageModel() const;
    PackageContext* GetPackageContext() const;

    PropertiesModel* GetPropertiesModel() const;
    QAbstractItemModel* GetLibraryModel() const;
    PreviewContext* GetPreviewContext() const;
  
    QtModelPackageCommandExecutor *GetCommandExecutor() const;
    QUndoStack *GetUndoStack() const;

signals:
    void activeRootControlsChanged(const QList<ControlNode *> &activatedRootControls, const QList<ControlNode *> &deactivatedRootControls);

    void controlSelectedInEditor(ControlNode *activatedControls);
    void allControlsDeselectedInEditor();

public slots:
    void OnSelectionRootControlChanged(const QList<ControlNode *> &activatedRootControls, const QList<ControlNode *> &deactivatedRootControls);
    void OnSelectionControlChanged(const QList<ControlNode *> &activatedControls, const QList<ControlNode *> &deactivatedControls);

protected slots:
    void OnControlSelectedInEditor(ControlNode *activatedControls);
    void OnAllControlDeselectedInEditor();

private:
    void UpdateControlCanvas();
    
private:
    QPointer<Project> project;
    PackageNode *package;
    QList<ControlNode *> selectedControls;
    QList<ControlNode *> activeRootControls;

    PackageContext *packageContext;
    PropertiesContext *propertiesContext;
    PreviewContext *previewContext;
    QAbstractItemModel *libraryModel;
    
    QtModelPackageCommandExecutor *commandExecutor;
    QScopedPointer<QUndoStack> undoStack;
};

inline QUndoStack *Document::GetUndoStack() const
{
    return undoStack.data();
}

inline PackageNode *Document::GetPackage() const 
{
    return package; 
}

inline Project *Document::GetProject() const
{ 
    return project; 
}

inline const QList<ControlNode*> &Document::GetSelectedControls() const
{ 
    return selectedControls; 
}

inline const QList<ControlNode*> &Document::GetActiveRootControls() const
{ 
    return activeRootControls; 
}

inline QtModelPackageCommandExecutor *Document::GetCommandExecutor() const
{
    return commandExecutor;
}

#endif // __QUICKED_PACKAGE_DOCUMENT_H__
