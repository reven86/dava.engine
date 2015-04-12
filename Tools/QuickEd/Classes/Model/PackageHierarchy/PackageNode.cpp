#include "PackageNode.h"

#include "PackageControlsNode.h"
#include "ImportedPackagesNode.h"
#include "ControlPrototype.h"
#include "PackageListener.h"
#include "PackageRef.h"
#include "../PackageSerializer.h"

using namespace DAVA;

PackageNode::PackageNode(PackageRef *_packageRef)
    : PackageBaseNode(nullptr)
    , packageRef(SafeRetain(_packageRef))
    , importedPackagesNode(nullptr)
    , packageControlsNode(nullptr)
{
    importedPackagesNode = new ImportedPackagesNode(this);
    packageControlsNode = new PackageControlsNode(this, packageRef);
}

PackageNode::~PackageNode()
{
    SafeRelease(packageRef);
    importedPackagesNode->SetParent(nullptr);
    SafeRelease(importedPackagesNode);
    packageControlsNode->SetParent(nullptr);
    SafeRelease(packageControlsNode);
}

int PackageNode::GetCount() const
{
    return 2;
}

PackageBaseNode *PackageNode::Get(int index) const
{
    if (index == 0)
        return importedPackagesNode;
    else if (index == 1)
        return packageControlsNode;
    else
        return nullptr;
}

String PackageNode::GetName() const
{
    return packageRef->GetName();
}

int PackageNode::GetFlags() const 
{
    return 0;
}

PackageRef *PackageNode::GetPackageRef() const
{
    return packageRef;
}

ImportedPackagesNode *PackageNode::GetImportedPackagesNode() const
{
    return importedPackagesNode;
}

PackageControlsNode *PackageNode::GetPackageControlsNode() const
{
    return packageControlsNode;
}

PackageControlsNode *PackageNode::FindImportedPackage(const DAVA::FilePath &path)
{
    for (int32 index = 0; index < importedPackagesNode->GetCount(); index++)
    {
        if (importedPackagesNode->Get(index)->GetPackageRef()->GetPath() == path)
            return importedPackagesNode->Get(index);
    }
    return nullptr;
}

void PackageNode::AddListener(PackageListener *listener)
{
    listeners.push_back(listener);
}

void PackageNode::RemoveListener(PackageListener *listener)
{
    auto it = std::find(listeners.begin(), listeners.end(), listener);
    if (it != listeners.end())
    {
        listeners.erase(it);
    }
    else
    {
        DVASSERT(false);
    }
}

void PackageNode::SetControlProperty(ControlNode *node, BaseProperty *property, const DAVA::VariantType &newValue)
{
    node->GetPropertiesRoot()->SetProperty(property, newValue);

    for (PackageListener *listener : listeners)
        listener->ControlPropertyWasChanged(node, property);
}

void PackageNode::SetControlDefaultProperty(ControlNode *node, BaseProperty *property, const DAVA::VariantType &newValue)
{
    node->GetPropertiesRoot()->SetDefaultProperty(property, newValue);

    for (PackageListener *listener : listeners)
        listener->ControlPropertyWasChanged(node, property);
}

void PackageNode::ResetControlProperty(ControlNode *node, BaseProperty *property)
{
    node->GetPropertiesRoot()->ResetProperty(property);

    for (PackageListener *listener : listeners)
        listener->ControlPropertyWasChanged(node, property);
}

void PackageNode::AddComponent(ControlNode *node, ComponentPropertiesSection *section)
{
    node->GetPropertiesRoot()->AddComponentPropertiesSection(section);
}

void PackageNode::RemoveComponent(ControlNode *node, ComponentPropertiesSection *section)
{
    node->GetPropertiesRoot()->RemoveComponentPropertiesSection(section);
}

void PackageNode::InsertControl(ControlNode *node, ControlsContainerNode *dest, DAVA::int32 index)
{
    for (PackageListener *listener : listeners)
        listener->ControlWillBeAdded(node, dest, index);
    
    node->MarkAsAlive();
    dest->InsertAtIndex(index, node);
    
    for (PackageListener *listener : listeners)
        listener->ControlWasAdded(node, dest, index);
}

void PackageNode::RemoveControl(ControlNode *node, ControlsContainerNode *from)
{
    for (PackageListener *listener : listeners)
        listener->ControlWillBeRemoved(node, from);
    
    node->MarkAsRemoved();
    from->Remove(node);
    
    for (PackageListener *listener : listeners)
        listener->ControlWasRemoved(node, from);
}

void PackageNode::InsertImportedPackage(PackageControlsNode *node, DAVA::int32 index)
{
    for (PackageListener *listener : listeners)
        listener->ImportedPackageWillBeAdded(node, this, index);
    
    importedPackagesNode->InsertAtIndex(index, node);
    
    for (PackageListener *listener : listeners)
        listener->ImportedPackageWasAdded(node, this, index);
}

void PackageNode::RemoveImportedPackage(PackageControlsNode *node)
{
    for (PackageListener *listener : listeners)
        listener->ImportedPackageWillBeRemoved(node, this);
    
    importedPackagesNode->Remove(node);
    
    for (PackageListener *listener : listeners)
        listener->ImportedPackageWasRemoved(node, this);
}

void PackageNode::Serialize(PackageSerializer *serializer) const
{
    serializer->BeginMap("Header");
    serializer->PutValue("version", String("0"));
    serializer->EndMap();
    
    importedPackagesNode->Serialize(serializer);
    packageControlsNode->Serialize(serializer);
}

void PackageNode::Serialize(PackageSerializer *serializer, const DAVA::Vector<ControlNode*> &nodes) const
{
    serializer->BeginMap("Header");
    serializer->PutValue("version", String("0"));
    serializer->EndMap();
    
    Set<PackageRef*> usedImportedPackages;
    for (ControlNode *node : nodes)
        CollectPackages(usedImportedPackages, node);

    importedPackagesNode->Serialize(serializer, usedImportedPackages);
    packageControlsNode->Serialize(serializer, nodes);
}

void PackageNode::CollectPackages(Set<PackageRef*> &packageRefs, ControlNode *node) const
{
    if (node->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE)
    {
        ControlPrototype *prototype = node->GetPrototype();
        if (prototype)
        {
            if (packageRefs.find(prototype->GetPackageRef()) == packageRefs.end())
                packageRefs.insert(prototype->GetPackageRef());
        }
    }
    
    for (int32 index = 0; index < node->GetCount(); index++)
        CollectPackages(packageRefs, node->Get(index));
}
