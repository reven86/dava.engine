#ifndef __UI_EDITOR_PACKAGE_NODE_H__
#define __UI_EDITOR_PACKAGE_NODE_H__

#include "PackageBaseNode.h"

#include "FileSystem/VariantType.h"

class ImportedPackagesNode;
class PackageControlsNode;
class ControlsContainerNode;
class ControlNode;
class StyleSheetsNode;
class StyleSheetNode;
class StyleSheetProperty;
class StyleSheetSelectorProperty;
class PackageListener;
class AbstractProperty;
class ComponentPropertiesSection;

namespace DAVA
{
class UIControlPackageContext;
}

class PackageNode : public PackageBaseNode
{
public:
    PackageNode(const DAVA::FilePath& path);

private:
    virtual ~PackageNode();

public:
    int GetCount() const override;
    PackageBaseNode* Get(int index) const override;

    void Accept(PackageVisitor* visitor) override;

    DAVA::String GetName() const override;

    PackageNode* GetPackage() override;
    const PackageNode* GetPackage() const override;
    void SetPath(const DAVA::FilePath& path);
    const DAVA::FilePath& GetPath() const;
    DAVA::UIControlPackageContext* GetContext() const;
    bool IsImported() const;

    bool CanRemove() const override;
    bool IsReadOnly() const override;

    ImportedPackagesNode* GetImportedPackagesNode() const;
    PackageControlsNode* GetPackageControlsNode() const;
    StyleSheetsNode* GetStyleSheets() const;

    PackageNode* FindImportedPackage(const DAVA::FilePath& path) const;
    bool FindPackageInImportedPackagesRecursively(const PackageNode* node) const;
    bool FindPackageInImportedPackagesRecursively(const DAVA::FilePath& path) const;

    void AddListener(PackageListener* listener);
    void RemoveListener(PackageListener* listener);

    void SetControlProperty(ControlNode* node, AbstractProperty* property, const DAVA::VariantType& newValue);
    void ResetControlProperty(ControlNode* node, AbstractProperty* property);
    void RefreshProperty(ControlNode* node, AbstractProperty* property);

    void AddComponent(ControlNode* node, ComponentPropertiesSection* section);
    void RemoveComponent(ControlNode* node, ComponentPropertiesSection* section);
    void AttachPrototypeComponent(ControlNode* node, ComponentPropertiesSection* destSection, ComponentPropertiesSection* prototypeSection);
    void DetachPrototypeComponent(ControlNode* node, ComponentPropertiesSection* destSection, ComponentPropertiesSection* prototypeSection);

    void SetStyleProperty(StyleSheetNode* node, AbstractProperty* property, const DAVA::VariantType& newValue);
    void AddStyleProperty(StyleSheetNode* node, StyleSheetProperty* property);
    void RemoveStyleProperty(StyleSheetNode* node, StyleSheetProperty* property);
    void InsertSelector(StyleSheetNode* node, StyleSheetSelectorProperty* property, DAVA::int32 index);
    void RemoveSelector(StyleSheetNode* node, StyleSheetSelectorProperty* property);

    void InsertControl(ControlNode* node, ControlsContainerNode* dest, DAVA::int32 index);
    void RemoveControl(ControlNode* node, ControlsContainerNode* from);

    void InsertStyle(StyleSheetNode* node, StyleSheetsNode* dest, DAVA::int32 index);
    void RemoveStyle(StyleSheetNode* node, StyleSheetsNode* from);

    void InsertImportedPackage(PackageNode* node, DAVA::int32 index);
    void RemoveImportedPackage(PackageNode* node);

    void RebuildStyleSheets();
    void RefreshPackageStylesAndLayout(bool includeImportedPackages = false);

private:
    void RefreshPropertiesInInstances(ControlNode* node, AbstractProperty* property);

    void RefreshControlStylesAndLayout(ControlNode* node);
    void RefreshStyles(ControlNode* node);
    void CollectRootControlsToRefreshLayout(ControlNode* node, DAVA::Vector<ControlNode*>& roots);
    void RestoreProperties(ControlNode* control);
    void NotifyPropertyChanged(ControlNode* control);

private:
    enum eSection
    {
        SECTION_IMPORTED_PACKAGES = 0,
        SECTION_STYLES = 1,
        SECTION_CONTROLS = 2,
        SECTION_COUNT = 3
    };

private:
    DAVA::FilePath path;
    DAVA::String name;

    ImportedPackagesNode* importedPackagesNode = nullptr;
    PackageControlsNode* packageControlsNode = nullptr;
    StyleSheetsNode* styleSheets = nullptr;
    DAVA::UIControlPackageContext* packageContext = nullptr;
    DAVA::Vector<PackageListener*> listeners;
};

#endif // __UI_EDITOR_PACKAGE_NODE_H__
