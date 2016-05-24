#ifndef __QUICKED_QT_MODEL_PACKAGE_COMMAND_EXECUTOR_H__
#define __QUICKED_QT_MODEL_PACKAGE_COMMAND_EXECUTOR_H__

#include "Base/BaseObject.h"
#include "Base/Result.h"
#include "EditorSystems/EditorSystemsManager.h"

#include <QString>

class Document;
class PackageBaseNode;
class QUndoStack;
class QUndoCommand;

class ControlNode;
class StyleSheetNode;
class StyleSheetsNode;
class PackageControlsNode;
class PackageNode;
class AbstractProperty;
class ControlsContainerNode;
class ComponentPropertiesSection;

class QtModelPackageCommandExecutor
{
public:
    QtModelPackageCommandExecutor(Document* _document);
    virtual ~QtModelPackageCommandExecutor();

    void AddImportedPackagesIntoPackage(const DAVA::Vector<DAVA::FilePath> packagePaths, PackageNode* package);
    void RemoveImportedPackagesFromPackage(const DAVA::Vector<PackageNode*>& importedPackage, PackageNode* package);

    void ChangeProperty(const DAVA::Vector<ChangePropertyAction>& propertyActions, size_t hash = 0);
    void ChangeProperty(ControlNode* node, AbstractProperty* property, const DAVA::VariantType& value, size_t hash = 0);
    void ResetProperty(ControlNode* node, AbstractProperty* property);

    void AddComponent(ControlNode* node, DAVA::uint32 componentType);
    void RemoveComponent(ControlNode* node, DAVA::uint32 componentType, DAVA::uint32 componentIndex);

    void ChangeProperty(StyleSheetNode* node, AbstractProperty* property, const DAVA::VariantType& value);

    void AddStyleProperty(StyleSheetNode* node, DAVA::uint32 propertyIndex);
    void RemoveStyleProperty(StyleSheetNode* node, DAVA::uint32 propertyIndex);

    void AddStyleSelector(StyleSheetNode* node);
    void RemoveStyleSelector(StyleSheetNode* node, DAVA::int32 selectorIndex);

    DAVA::ResultList InsertControl(ControlNode* control, ControlsContainerNode* dest, DAVA::int32 destIndex);
    DAVA::Vector<ControlNode*> InsertInstances(const DAVA::Vector<ControlNode*>& controls, ControlsContainerNode* dest, DAVA::int32 destIndex);
    DAVA::Vector<ControlNode*> CopyControls(const DAVA::Vector<ControlNode*>& nodes, ControlsContainerNode* dest, DAVA::int32 destIndex);
    DAVA::Vector<ControlNode*> MoveControls(const DAVA::Vector<ControlNode*>& nodes, ControlsContainerNode* dest, DAVA::int32 destIndex);

    DAVA::ResultList InsertStyle(StyleSheetNode* node, StyleSheetsNode* dest, DAVA::int32 destIndex);
    void CopyStyles(const DAVA::Vector<StyleSheetNode*>& nodes, StyleSheetsNode* dest, DAVA::int32 destIndex);
    void MoveStyles(const DAVA::Vector<StyleSheetNode*>& nodes, StyleSheetsNode* dest, DAVA::int32 destIndex);

    void Remove(const DAVA::Vector<ControlNode*>& controls, const DAVA::Vector<StyleSheetNode*>& styles);
    DAVA::Vector<PackageBaseNode*> Paste(PackageNode* root, PackageBaseNode* dest, DAVA::int32 destIndex, const DAVA::String& data);

private:
    void AddImportedPackageIntoPackageImpl(PackageNode* importedPackage, PackageNode* package);
    void InsertControlImpl(ControlNode* control, ControlsContainerNode* dest, DAVA::int32 destIndex);
    void RemoveControlImpl(ControlNode* node);
    void AddComponentImpl(ControlNode* node, DAVA::int32 type, DAVA::int32 index, ComponentPropertiesSection* prototypeSection);
    void RemoveComponentImpl(ControlNode* node, ComponentPropertiesSection* section);
    bool IsNodeInHierarchy(const PackageBaseNode* node) const;

    QUndoStack* GetUndoStack() const;
    void PushCommand(QUndoCommand* cmd);
    void BeginMacro(const QString& name);
    void EndMacro();

private:
    Document* document = nullptr;
    PackageNode* packageNode = nullptr;
};

#endif // __QUICKED_QT_MODEL_PACKAGE_COMMAND_EXECUTOR_H__
