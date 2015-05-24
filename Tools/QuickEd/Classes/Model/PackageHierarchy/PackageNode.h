#ifndef __UI_EDITOR_PACKAGE_NODE_H__
#define __UI_EDITOR_PACKAGE_NODE_H__

#include "PackageBaseNode.h"

#include "FileSystem/VariantType.h"

class ImportedPackagesNode;
class PackageControlsNode;
class ControlsContainerNode;
class PackageSerializer;
class ControlNode;
class PackageRef;
class PackageListener;
class AbstractProperty;
class ComponentPropertiesSection;

class PackageNode : public PackageBaseNode
{
public:
    PackageNode(PackageRef *_packageRef);
private:
    virtual ~PackageNode();
    
public:
    int GetCount() const override;
    PackageBaseNode *Get(int index) const override;

    DAVA::String GetName() const override;
    
    PackageRef *GetPackageRef() const override;
    PackageNode *GetPackage() override;
    const PackageNode *GetPackage() const override;
    bool IsImported() const;

    virtual bool IsReadOnly() const override;
    
    ImportedPackagesNode *GetImportedPackagesNode() const;
    PackageControlsNode *GetPackageControlsNode() const;

    PackageNode *FindImportedPackage(const DAVA::FilePath &path);

    void AddListener(PackageListener *listener);
    void RemoveListener(PackageListener *listener);
    
    void SetControlProperty(ControlNode *node, AbstractProperty *property, const DAVA::VariantType &newValue);
    void SetControlDefaultProperty(ControlNode *node, AbstractProperty *property, const DAVA::VariantType &newValue);
    void ResetControlProperty(ControlNode *node, AbstractProperty *property);
    void RefreshProperty(ControlNode *node, AbstractProperty *property);

    void AddComponent(ControlNode *node, ComponentPropertiesSection *section);
    void RemoveComponent(ControlNode *node, ComponentPropertiesSection *section);
    
    void InsertControl(ControlNode *node, ControlsContainerNode *dest, DAVA::int32 index);
    void RemoveControl(ControlNode *node, ControlsContainerNode *from);

    void InsertImportedPackage(PackageNode *node, DAVA::int32 index);
    void RemoveImportedPackage(PackageNode *node);

    void Serialize(PackageSerializer *serializer) const;
    void Serialize(PackageSerializer *serializer, const DAVA::Vector<ControlNode*> &nodes) const;
    
    
private:
    void CollectPackages(DAVA::Set<PackageRef*> &packageRefs, ControlNode *node) const;
    void RefreshPropertiesInInstances(ControlNode *node, AbstractProperty *property);
    
private:
    PackageRef *packageRef;
    
    ImportedPackagesNode *importedPackagesNode;
    PackageControlsNode *packageControlsNode;
    DAVA::Vector<PackageListener*> listeners;
};

#endif // __UI_EDITOR_PACKAGE_NODE_H__
