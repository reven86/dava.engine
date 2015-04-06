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
    
    WidgetContext *GetContext() const;
    PropertiesModel *GetPropertiesModel() const; //TODO: this is deprecated
    PackageModel* GetPackageModel() const; //TODO: this is deprecated
    QUndoStack *GetUndoStack() const;
    QtModelPackageCommandExecutor *GetCommandExecutor() const;

signals:
    void ContextDataChanged(const QByteArray &role);
public slots:
    void UpdateLanguage();

private:
    void UpdateLanguageRecursively(ControlNode *node);
    void UpdateControlCanvas();
    void InitWidgetContexts();

private:
    PackageNode *package;

    WidgetContext *dataContext;

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

inline WidgetContext *Document::GetContext() const
{
    return dataContext;
}

#endif // __QUICKED_PACKAGE_DOCUMENT_H__
