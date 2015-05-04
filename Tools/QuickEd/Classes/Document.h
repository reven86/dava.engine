#ifndef __QUICKED_DOCUMENT_H__
#define __QUICKED_DOCUMENT_H__

#include <QUndoStack>
#include "Base/BaseTypes.h"

namespace DAVA {
    class FilePath;
    class VariantType;
}
class ValueProperty;

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
    Document(PackageNode *package, QObject *parent = nullptr);

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
    void BeginUpdatePreset();
    void UpdateFontPreset();
    void BeginChangePreset(const DAVA::String &oldPresetName);
    void ChangePreset(const DAVA::String &newPresetName);

    void BeginSetProperty(const DAVA::String &property, const DAVA::VariantType &findValue);
    void SetProperty(const DAVA::String &property, const DAVA::VariantType &newValue);
    void BeginUpdateProperty(const DAVA::String &property);
    void UpdateProperty(const DAVA::String &property);
private:

    static void BeginUpdatePropertyRecuresively(ControlNode* node, const DAVA::String &property);
    static void UpdatePropertyRecursively(ControlNode* node, const DAVA::String &property);
    void BeginSetPropertyRecursively(ControlNode *node, const DAVA::String &property, const DAVA::VariantType &findValue);
    void InitSharedData();

private:
    PackageNode *package;

    SharedData *sharedData;

    QtModelPackageCommandExecutor *commandExecutor;
    QUndoStack *undoStack;
    DAVA::Map<DAVA::String, DAVA::Vector<ValueProperty*> >savedProperties;
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
