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
    
    WidgetContext *GetLibraryContext() const;
    WidgetContext *GetPropertiesContext() const;
    WidgetContext *GetPreviewContext() const;
    PropertiesModel *GetPropertiesModel() const;
    PackageModel* GetPackageModel() const;
    WidgetContext* GetPackageContext() const;
  
    QUndoStack *GetUndoStack() const;

signals:
    void LibraryDataChanged(const QByteArray &role);
    void PropertiesDataChanged(const QByteArray &role);
    void PackageDataChanged(const QByteArray &role);
    void PreviewDataChanged(const QByteArray &role);
public slots:

private slots:
    void OnPreviewContextDataChanged(const QByteArray &role);
    void OnPackageContextDataChanged(const QByteArray &role);
private:
    void UpdateControlCanvas();
    void InitWidgetContexts();
    void ConnectWidgetContexts() const;

private:
    PackageNode *package;

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




#endif // __QUICKED_PACKAGE_DOCUMENT_H__
