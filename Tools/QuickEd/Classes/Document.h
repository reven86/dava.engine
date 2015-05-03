#ifndef __QUICKED_DOCUMENT_H__
#define __QUICKED_DOCUMENT_H__

#include <QUndoStack>
#include "Base/BaseTypes.h"

namespace DAVA {
    class FilePath;
    class VariantType;
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
    PropertiesModel *GetPropertiesModel() const; //TODO: this is deprecated
    PackageModel* GetPackageModel() const; //TODO: this is deprecated
    QUndoStack *GetUndoStack() const;
    QtModelPackageCommandExecutor *GetCommandExecutor() const;

signals:
    void SharedDataChanged(const QByteArray &role);
public slots:
    void UpdateLanguage();
    void UpdateFontPreset();
    void NewFontPreset(const QString &oldPresetName, const QString &newPresetName);

private:
    void UpdateProperty(const DAVA::String &property);
    static void UpdatePropertyRecursively(ControlNode* node, const DAVA::String &property);
    static void SetPropertyValueRecursively(ControlNode *node, const DAVA::String &property, const DAVA::VariantType &findValue, const DAVA::VariantType &newValue);
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
