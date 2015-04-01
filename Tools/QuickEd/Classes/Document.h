#ifndef __QUICKED_PACKAGE_DOCUMENT_H__
#define __QUICKED_PACKAGE_DOCUMENT_H__

#include <QUndoStack>

namespace DAVA {
    class FilePath;
}

class PackageNode;
class QtModelPackageCommandExecutor;

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
    
    const DAVA::FilePath &GetPackageFilePath() const;
    PackageNode *GetPackage() const;
    
    WidgetContext *GetLibraryContext() const;
    WidgetContext *GetPropertiesContext() const;
    WidgetContext *GetPreviewContext() const;
    WidgetContext* GetPackageContext() const;
    PropertiesModel *GetPropertiesModel() const; //TODO: this is deprecated
    PackageModel* GetPackageModel() const; //TODO: this is deprecated
    QUndoStack *GetUndoStack() const;
    QtModelPackageCommandExecutor *GetCommandExecutor() const;

signals:
    void LibraryDataChanged(const QByteArray &role);
    void PropertiesDataChanged(const QByteArray &role);
    void PackageDataChanged(const QByteArray &role);
    void PreviewDataChanged(const QByteArray &role);
public slots:
    void UpdateLanguage();

private slots:
    void OnContextDataChanged(const QByteArray &role);
private:
    void UpdateLanguageRecursively(ControlNode *node);
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
    QUndoStack *undoStack;
};

inline QUndoStack *Document::GetUndoStack() const
{
    return undoStack;
}

inline PackageNode *Document::GetPackage() const 
{
    return package; 
}

inline QtModelPackageCommandExecutor *Document::GetCommandExecutor() const
{
    return commandExecutor;
}

inline WidgetContext *Document::GetLibraryContext() const
{
    return libraryContext;
}

inline WidgetContext* Document::GetPackageContext() const
{
    return packageContext;
}

inline WidgetContext *Document::GetPreviewContext() const
{
    return previewContext;
}

inline WidgetContext *Document::GetPropertiesContext() const
{
    return propertiesContext;
}



#endif // __QUICKED_PACKAGE_DOCUMENT_H__
