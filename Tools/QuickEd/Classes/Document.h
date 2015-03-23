#ifndef __QUICKED_PACKAGE_DOCUMENT_H__
#define __QUICKED_PACKAGE_DOCUMENT_H__

#include <QUndoStack>

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
class WidgetContext;
class PropertiesModel;
class PackageModel;

class PackageNode;
class ControlNode;

class Document : public QObject
{
    Q_OBJECT
public:
    Document(PackageNode *package, QObject *parent = NULL);

    virtual ~Document();
    
    bool IsModified() const;
    void ClearModified();
    const DAVA::FilePath &PackageFilePath() const;
    PackageNode *GetPackage() const;
    
    const QList<ControlNode*> &GetSelectedControls() const;
    const QList<ControlNode*> &GetActiveRootControls() const;
    WidgetContext *GetLibraryContext() const;
    WidgetContext *GetPropertiesContext() const;
    WidgetContext *GetPreviewContext() const;
    PropertiesModel *GetPropertiesModel() const;
    PackageModel* GetPackageModel() const;
    WidgetContext* GetPackageContext() const;
  
    QtModelPackageCommandExecutor *GetCommandExecutor() const;
    QUndoStack *GetUndoStack() const;

signals:
    void activeRootControlsChanged(const QList<ControlNode *> &activatedRootControls, const QList<ControlNode *> &deactivatedRootControls);

    void controlSelectedInEditor(ControlNode *activatedControls);
    void allControlsDeselectedInEditor();

    void LibraryDataChanged(const QByteArray &role);
    void PropertiesDataChanged(const QByteArray &role);
    void PackageDataChanged(const QByteArray &role);
    void PreviewDataChanged(const QByteArray &role);
public slots:
    void OnSelectionRootControlChanged(const QList<ControlNode *> &activatedRootControls, const QList<ControlNode *> &deactivatedRootControls);
    void OnSelectionControlChanged(const QList<ControlNode *> &activatedControls, const QList<ControlNode *> &deactivatedControls);

protected slots:
    void OnControlSelectedInEditor(ControlNode *activatedControls);
    void OnAllControlDeselectedInEditor();

private:
    void UpdateControlCanvas();
    void InitWidgetContexts();
    void ConnectWidgetContexts() const;

private:
    PackageNode *package;
    QList<ControlNode *> selectedControls;
    QList<ControlNode *> activeRootControls;

    WidgetContext *libraryContext;
    WidgetContext *propertiesContext;
    WidgetContext *packageContext;
    WidgetContext *previewContext;

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
