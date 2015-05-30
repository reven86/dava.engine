#include "PackageNode.h"

#include "PackageVisitor.h"
#include "PackageControlsNode.h"
#include "ImportedPackagesNode.h"
#include "PackageListener.h"
#include "../PackageSerializer.h"
#include "../ControlProperties/RootProperty.h"

using namespace DAVA;

PackageNode::PackageNode(const FilePath &aPath)
    : PackageBaseNode(nullptr)
    , path(aPath)
    , importedPackagesNode(nullptr)
    , packageControlsNode(nullptr)
{
    importedPackagesNode = new ImportedPackagesNode(this);
    packageControlsNode = new PackageControlsNode(this);
    name = path.GetBasename();
}

PackageNode::~PackageNode()
{
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

void PackageNode::Accept(PackageVisitor *visitor)
{
    visitor->VisitPackage(this);
}

String PackageNode::GetName() const
{
    return name;
}

PackageNode *PackageNode::GetPackage()
{
    return this;
}

const FilePath &PackageNode::GetPath() const
{
    return path;
}

const PackageNode *PackageNode::GetPackage() const
{
    return this;
}

bool PackageNode::IsImported() const
{
    return GetParent() != nullptr;
}

bool PackageNode::IsReadOnly() const
{
    return GetParent() != nullptr ? GetParent()->IsReadOnly() : false;
}

ImportedPackagesNode *PackageNode::GetImportedPackagesNode() const
{
    return importedPackagesNode;
}

PackageControlsNode *PackageNode::GetPackageControlsNode() const
{
    return packageControlsNode;
}

PackageNode *PackageNode::FindImportedPackage(const DAVA::FilePath &path)
{
    for (int32 index = 0; index < importedPackagesNode->GetCount(); index++)
    {
        if (importedPackagesNode->GetImportedPackage(index)->GetPath() == path)
            return importedPackagesNode->GetImportedPackage(index);
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

void PackageNode::SetControlProperty(ControlNode *node, AbstractProperty *property, const DAVA::VariantType &newValue)
{
    node->GetRootProperty()->SetProperty(property, newValue);
    
    for (PackageListener *listener : listeners)
        listener->ControlPropertyWasChanged(node, property);

    RefreshPropertiesInInstances(node, property);
}

void PackageNode::SetControlDefaultProperty(ControlNode *node, AbstractProperty *property, const DAVA::VariantType &newValue)
{
    node->GetRootProperty()->SetDefaultProperty(property, newValue);

    for (PackageListener *listener : listeners)
        listener->ControlPropertyWasChanged(node, property);

    RefreshPropertiesInInstances(node, property);
}

void PackageNode::ResetControlProperty(ControlNode *node, AbstractProperty *property)
{
    node->GetRootProperty()->ResetProperty(property);
    
    for (PackageListener *listener : listeners)
        listener->ControlPropertyWasChanged(node, property);
    
    RefreshPropertiesInInstances(node, property);
}

void PackageNode::RefreshProperty(ControlNode *node, AbstractProperty *property)
{
    node->GetRootProperty()->RefreshProperty(property);
    
    for (PackageListener *listener : listeners)
        listener->ControlPropertyWasChanged(node, property);

    RefreshPropertiesInInstances(node, property);
}

void PackageNode::AddComponent(ControlNode *node, ComponentPropertiesSection *section)
{
    node->GetRootProperty()->AddComponentPropertiesSection(section);
}

void PackageNode::RemoveComponent(ControlNode *node, ComponentPropertiesSection *section)
{
    node->GetRootProperty()->RemoveComponentPropertiesSection(section);
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

void PackageNode::InsertImportedPackage(PackageNode *node, DAVA::int32 index)
{
    for (PackageListener *listener : listeners)
        listener->ImportedPackageWillBeAdded(node, importedPackagesNode, index);
    
    importedPackagesNode->InsertAtIndex(index, node);
    
    for (PackageListener *listener : listeners)
        listener->ImportedPackageWasAdded(node, importedPackagesNode, index);
}

void PackageNode::RemoveImportedPackage(PackageNode *node)
{
    for (PackageListener *listener : listeners)
        listener->ImportedPackageWillBeRemoved(node, importedPackagesNode);
    
    importedPackagesNode->Remove(node);
    
    for (PackageListener *listener : listeners)
        listener->ImportedPackageWasRemoved(node, importedPackagesNode);
}

void PackageNode::CollectPackages(Vector<PackageNode*> &packages, ControlNode *node) const
{
    if (node->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE)
    {
        ControlNode *prototype = node->GetPrototype();
        if (prototype && std::find(packages.begin(), packages.end(), prototype->GetPackage()) == packages.end())
            packages.push_back(prototype->GetPackage());
    }
    
    for (int32 index = 0; index < node->GetCount(); index++)
        CollectPackages(packages, node->Get(index));
}

void PackageNode::RefreshPropertiesInInstances(ControlNode *node, AbstractProperty *property)
{
    for (ControlNode *instance : node->GetInstances())
    {
        AbstractProperty *instanceProperty = instance->GetRootProperty()->FindPropertyByPrototype(property);

        if (instanceProperty)
        {
            instance->GetRootProperty()->RefreshProperty(instanceProperty);
            RefreshProperty(instance, instanceProperty);
        }
    }
}
