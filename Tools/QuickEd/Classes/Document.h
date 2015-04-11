#ifndef __QUICKED_DOCUMENT_H__
#define __QUICKED_DOCUMENT_H__

#include <QUndoStack>

namespace DAVA {
    class FilePath;
}

class PackageNode;
class QtModelPackageCommandExecutor;

class SharedData;
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
    
    SharedData *GetContext() const;
    QUndoStack *GetUndoStack() const;
    QtModelPackageCommandExecutor *GetCommandExecutor() const;

signals:
    void SharedDataChanged(const QByteArray &role);
public slots:
    void UpdateLanguage();

private:
    void UpdateLanguageRecursively(ControlNode *node);
    void UpdateControlCanvas();
    void InitSharedData();

private:
    PackageNode *package;

    SharedData *sharedData;

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

inline SharedData *Document::GetContext() const
{
    return sharedData;
}

#endif // __QUICKED_DOCUMENT_H__
