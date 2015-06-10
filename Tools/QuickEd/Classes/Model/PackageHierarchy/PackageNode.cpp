/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "PackageNode.h"

#include "PackageVisitor.h"
#include "PackageControlsNode.h"
#include "ImportedPackagesNode.h"
#include "PackageListener.h"
#include "../ControlProperties/RootProperty.h"
#include "../ControlProperties/DependedOnLayoutProperty.h"
#include "../ControlProperties/ControlPropertiesSection.h"

#include "UI/Layouts/UILayoutSystem.h"
#include "UI/UIControlSystem.h"

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

PackageNode *PackageNode::FindImportedPackage(const DAVA::FilePath &path) const
{
    for (int32 index = 0; index < importedPackagesNode->GetCount(); index++)
    {
        if (importedPackagesNode->GetImportedPackage(index)->GetPath() == path)
            return importedPackagesNode->GetImportedPackage(index);
    }
    return nullptr;
}

bool PackageNode::FindPackageInImportedPackagesRecursively(const PackageNode *node) const
{
    return FindPackageInImportedPackagesRecursively(node->GetPath());
}

bool PackageNode::FindPackageInImportedPackagesRecursively(const DAVA::FilePath &path) const
{
    for (int32 index = 0; index < importedPackagesNode->GetCount(); index++)
    {
        PackageNode *importedPackage = importedPackagesNode->GetImportedPackage(index);
        if (importedPackage->GetPath().GetFrameworkPath() == path.GetFrameworkPath())
            return true;
        if (importedPackage->FindPackageInImportedPackagesRecursively(path))
            return true;
    }
    return false;
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
    RefreshLayout(node);
}

void PackageNode::SetControlDefaultProperty(ControlNode *node, AbstractProperty *property, const DAVA::VariantType &newValue)
{
    node->GetRootProperty()->SetDefaultProperty(property, newValue);

    for (PackageListener *listener : listeners)
        listener->ControlPropertyWasChanged(node, property);

    RefreshPropertiesInInstances(node, property);
    RefreshLayout(node);
}

void PackageNode::ResetControlProperty(ControlNode *node, AbstractProperty *property)
{
    node->GetRootProperty()->ResetProperty(property);
    
    for (PackageListener *listener : listeners)
        listener->ControlPropertyWasChanged(node, property);
    
    RefreshPropertiesInInstances(node, property);
    RefreshLayout(node);
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
    RefreshLayout(node);
}

void PackageNode::RemoveComponent(ControlNode *node, ComponentPropertiesSection *section)
{
    node->GetRootProperty()->RemoveComponentPropertiesSection(section);
    RefreshLayout(node);
}

void PackageNode::InsertControl(ControlNode *node, ControlsContainerNode *dest, DAVA::int32 index)
{
    for (PackageListener *listener : listeners)
        listener->ControlWillBeAdded(node, dest, index);
    
    node->MarkAsAlive();
    dest->InsertAtIndex(index, node);
    
    for (PackageListener *listener : listeners)
        listener->ControlWasAdded(node, dest, index);
    
    RefreshLayout(node);
}

void PackageNode::RemoveControl(ControlNode *node, ControlsContainerNode *from)
{
    for (PackageListener *listener : listeners)
        listener->ControlWillBeRemoved(node, from);
    
    node->MarkAsRemoved();
    from->Remove(node);
    
    for (PackageListener *listener : listeners)
        listener->ControlWasRemoved(node, from);
    
    ControlNode *parent = dynamic_cast<ControlNode*>(from);
    if (parent)
        RefreshLayout(parent);
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

void PackageNode::RefreshLayout()
{
    for (int32 i = 0; i < importedPackagesNode->GetCount(); i++)
    {
        importedPackagesNode->GetImportedPackage(i)->RefreshLayout();
    }
    
    for (int32 i = 0; i < packageControlsNode->GetCount(); i++)
    {
        RefreshLayout(packageControlsNode->Get(i));
    }
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

void PackageNode::RefreshLayout(ControlNode *node)
{
    Vector<ControlNode*> roots;
    CollectRootControlsToRefreshLayout(node, roots);
    
    for (ControlNode *root : roots)
    {
        RestoreProperties(root);
    }
    
    for (ControlNode *root : roots)
    {
        UIControlSystem::Instance()->GetLayoutSystem()->ApplyLayout(root->GetControl());
    }

    for (ControlNode *root : roots)
    {
        NotifyPropertyChanged(root);
    }
}

void PackageNode::CollectRootControlsToRefreshLayout(ControlNode *node, DAVA::Vector<ControlNode*> &roots)
{
    ControlNode *root = node;
    while (root->GetParent() != nullptr && root->GetParent()->GetControl() != nullptr)
        root = static_cast<ControlNode*>(root->GetParent());
    
    if (std::find(roots.begin(), roots.end(), root) == roots.end())
    {
        roots.push_back(root);
        for (ControlNode *instance : root->GetInstances())
            CollectRootControlsToRefreshLayout(instance, roots);
    }
}

void PackageNode::RestoreProperties(ControlNode *container)
{
    for (int i = 0; i < container->GetCount(); i++)
    {
        ControlNode *control = container->Get(i);
        RootProperty *rootProperty = control->GetRootProperty();
        for (int j = 0; j < rootProperty->GetControlPropertiesSection(0)->GetCount(); j++)
        {
            DependedOnLayoutProperty *prop = dynamic_cast<DependedOnLayoutProperty*>(rootProperty->GetControlPropertiesSection(0)->GetProperty(j));
            if (prop)
            {
                prop->RestoreSourceValue();
            }
        }
        RestoreProperties(control);
    }
}

void PackageNode::NotifyPropertyChanged(ControlNode *control)
{
    RootProperty *rootProperty = control->GetRootProperty();
    for (int j = 0; j < rootProperty->GetControlPropertiesSection(0)->GetCount(); j++)
    {
        DependedOnLayoutProperty *prop = dynamic_cast<DependedOnLayoutProperty*>(rootProperty->GetControlPropertiesSection(0)->GetProperty(j));
        if (prop)
        {
            for (PackageListener *listener : listeners)
                listener->ControlPropertyWasChanged(control, prop);
        }
    }

    for (int i = 0; i < control->GetCount(); i++)
    {
        NotifyPropertyChanged(control->Get(i));
    }
}
