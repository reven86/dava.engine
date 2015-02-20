#ifndef __QUICKED_PACKAGE_DOCUMENT_H__
#define __QUICKED_PACKAGE_DOCUMENT_H__

#include <QPointer>
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Project.h"

namespace DAVA {
    class UIPackage;
    class FilePath;
    class UIControl;
    class UIScreen;
}

class QUndoStack;

class QtModelPackageCommandExecutor;
class PackageContext;
class PropertiesContext;
class LibraryContext;
class PreviewContext;

class PackageNode;
class ControlNode;

class Document : public QObject
{
    Q_OBJECT
public:
    Document(Project * project, PackageNode *package, QObject *parent = nullptr);
    virtual ~Document();
       
    bool IsModified() const;
    void ClearModified();
    void SetActive(bool active);
    const DAVA::FilePath &PackageFilePath() const;
    inline PackageNode *GetPackage() const;
    inline Project *GetProject() const;
    
    inline const QList<ControlNode*> &GetSelectedControls() const;
    inline const QList<ControlNode*> &GetActiveRootControls() const;
    
    inline PackageContext *GetPackageContext() const;
    inline PropertiesContext *GetPropertiesContext() const;
    inline LibraryContext *GetLibraryContext() const;
    inline PreviewContext *GetPreviewContext() const;

    inline QUndoStack *UndoStack() const;
    
    inline QtModelPackageCommandExecutor *GetCommandExecutor() const;

signals:
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

    PackageContext *packageContext;
    PropertiesContext *propertiesContext;
    PreviewContext *previewContext;
    LibraryContext *libraryContext;
    
    QtModelPackageCommandExecutor *commandExecutor;

    QUndoStack *undoStack;
};

Q_DECLARE_METATYPE(Document *);

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

inline PackageContext *Document::GetPackageContext() const 
{ 
    return packageContext; 
}

inline PropertiesContext *Document::GetPropertiesContext() const 
{ 
    return propertiesContext; 
}

inline LibraryContext *Document::GetLibraryContext() const 
{ 
    return libraryContext; 
}

inline PreviewContext *Document::GetPreviewContext() const 
{ 
    return previewContext; 
}

inline QUndoStack *Document::UndoStack() const 
{ 
    return undoStack; 
}

inline QtModelPackageCommandExecutor *Document::GetCommandExecutor() const
{
    return commandExecutor;
}


#endif // __QUICKED_PACKAGE_DOCUMENT_H__
