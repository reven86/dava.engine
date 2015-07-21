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
#include "StyleSheetsNode.h"
#include "PackageListener.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/StyleSheetRootProperty.h"
#include "Model/ControlProperties/StyleSheetSelectorProperty.h"
#include "Model/ControlProperties/DependedOnLayoutProperty.h"
#include "Model/ControlProperties/ControlPropertiesSection.h"
#include "Model/ControlProperties/ComponentPropertiesSection.h"

#include "UI/Layouts/UILayoutSystem.h"
#include "UI/Styles/UIStyleSheetSystem.h"
#include "UI/UIControlSystem.h"
#include "UI/UIControlPackageContext.h"
#include "UI/Styles/UIStyleSheet.h"

using namespace DAVA;

PackageNode::PackageNode(const FilePath &aPath)
    : PackageBaseNode(nullptr)
    , path(aPath)
{
    name = path.GetBasename();
    importedPackagesNode = new ImportedPackagesNode(this);
    packageControlsNode = new PackageControlsNode(this);
    styleSheets = new StyleSheetsNode(this);
    packageContext = new UIControlPackageContext();
}

PackageNode::~PackageNode()
{
    importedPackagesNode->SetParent(nullptr);
    SafeRelease(importedPackagesNode);
    
    packageControlsNode->SetParent(nullptr);
    SafeRelease(packageControlsNode);

    styleSheets->SetParent(nullptr);
    SafeRelease(styleSheets);
    
    SafeRelease(packageContext);
}

int PackageNode::GetCount() const
{
    return 3;
}

PackageBaseNode *PackageNode::Get(int index) const
{
    switch (index)
    {
        case 0:
            return importedPackagesNode;
            
        case 1:
            return styleSheets;

        case 2:
            return packageControlsNode;
            
    }
    DVASSERT(false);
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

UIControlPackageContext *PackageNode::GetContext() const
{
    return packageContext;
}

const PackageNode *PackageNode::GetPackage() const
{
    return this;
}

bool PackageNode::IsImported() const
{
    return GetParent() != nullptr;
}

bool PackageNode::CanRemove() const
{
    return GetParent() != nullptr && !GetParent()->IsReadOnly();
}

bool PackageNode::IsReadOnly() const
{
    return GetParent() != nullptr;
}

ImportedPackagesNode *PackageNode::GetImportedPackagesNode() const
{
    return importedPackagesNode;
}

PackageControlsNode *PackageNode::GetPackageControlsNode() const
{
    return packageControlsNode;
}

StyleSheetsNode *PackageNode::GetStyleSheets() const
{
    return styleSheets;
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
    if (property->GetStylePropertyIndex() != -1)
        node->GetControl()->SetPropertyLocalFlag(property->GetStylePropertyIndex(), true);

    node->GetRootProperty()->SetProperty(property, newValue);
    
    for (PackageListener *listener : listeners)
        listener->ControlPropertyWasChanged(node, property);

    RefreshPropertiesInInstances(node, property);
    RefreshLayout();
}

void PackageNode::ResetControlProperty(ControlNode *node, AbstractProperty *property)
{
    node->GetRootProperty()->ResetProperty(property);
    
    if (property->GetStylePropertyIndex() != -1)
        node->GetControl()->SetPropertyLocalFlag(property->GetStylePropertyIndex(), property->IsReplaced());

    for (PackageListener *listener : listeners)
        listener->ControlPropertyWasChanged(node, property);
    
    RefreshPropertiesInInstances(node, property);
    RefreshLayout();
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
    RefreshLayout();
}

void PackageNode::RemoveComponent(ControlNode *node, ComponentPropertiesSection *section)
{
    node->GetRootProperty()->RemoveComponentPropertiesSection(section);
    RefreshLayout();
}

void PackageNode::AttachPrototypeComponent(ControlNode *node, ComponentPropertiesSection *section, ComponentPropertiesSection *prototypeSection)
{
    node->GetRootProperty()->AttachPrototypeComponent(section, prototypeSection);
    
    RefreshProperty(node, section);
    for (int32 i = 0; i < section->GetCount(); i++)
        RefreshProperty(node, section->GetProperty(i));
}

void PackageNode::DetachPrototypeComponent(ControlNode *node, ComponentPropertiesSection *section, ComponentPropertiesSection *prototypeSection)
{
    node->GetRootProperty()->DetachPrototypeComponent(section, prototypeSection);
    
    RefreshProperty(node, section);
    for (int32 i = 0; i < section->GetCount(); i++)
        RefreshProperty(node, section->GetProperty(i));
}

void PackageNode::SetStyleProperty(StyleSheetNode *node, AbstractProperty *property, const DAVA::VariantType &newValue)
{
    node->GetRootProperty()->SetProperty(property, newValue);
    node->UpdateName();
    
    RebuildStyleSheets();
    RefreshLayout();

    for (PackageListener *listener : listeners)
        listener->StylePropertyWasChanged(node, property);
}

void PackageNode::AddStyleProperty(StyleSheetNode *node, StyleSheetProperty *property)
{
    node->GetRootProperty()->AddProperty(property);
    RefreshLayout();
}

void PackageNode::RemoveStyleProperty(StyleSheetNode *node, StyleSheetProperty *property)
{
    node->GetRootProperty()->RemoveProperty(property);

    RefreshLayout();
}

void PackageNode::InsertSelector(StyleSheetNode *node, StyleSheetSelectorProperty *property, DAVA::int32 index)
{
    node->GetRootProperty()->InsertSelector(property, index);
    node->UpdateName();

    RebuildStyleSheets();
    RefreshLayout();

    for (PackageListener *listener : listeners)
        listener->StylePropertyWasChanged(node, property);
}

void PackageNode::RemoveSelector(StyleSheetNode *node, StyleSheetSelectorProperty *property)
{
    node->GetRootProperty()->RemoveSelector(property);
    node->UpdateName();
    
    RebuildStyleSheets();
    RefreshLayout();

    for (PackageListener *listener : listeners)
        listener->StylePropertyWasChanged(node, property);
}

void PackageNode::InsertControl(ControlNode *node, ControlsContainerNode *dest, DAVA::int32 index)
{
    for (PackageListener *listener : listeners)
        listener->ControlWillBeAdded(node, dest, index);
    
    node->MarkAsAlive();
    dest->InsertAtIndex(index, node);
    
    for (PackageListener *listener : listeners)
        listener->ControlWasAdded(node, dest, index);
    
    RefreshLayout();
}

void PackageNode::RemoveControl(ControlNode *node, ControlsContainerNode *from)
{
    for (PackageListener *listener : listeners)
        listener->ControlWillBeRemoved(node, from);
    
    node->MarkAsRemoved();
    from->Remove(node);
    
    for (PackageListener *listener : listeners)
        listener->ControlWasRemoved(node, from);
    
    RefreshLayout();
}

void PackageNode::InsertStyle(StyleSheetNode *node, StyleSheetsNode *dest, DAVA::int32 index)
{
    for (PackageListener *listener : listeners)
        listener->StyleWillBeAdded(node, dest, index);
    
    dest->InsertAtIndex(index, node);
    
    for (PackageListener *listener : listeners)
        listener->StyleWasAdded(node, dest, index);
    
    RefreshLayout();
}

void PackageNode::RemoveStyle(StyleSheetNode *node, StyleSheetsNode *from)
{
    for (PackageListener *listener : listeners)
        listener->StyleWillBeRemoved(node, from);
    
    from->Remove(node);
    
    for (PackageListener *listener : listeners)
        listener->StyleWasRemoved(node, from);
    
    RefreshLayout();
}

void PackageNode::InsertImportedPackage(PackageNode *node, DAVA::int32 index)
{
    for (PackageListener *listener : listeners)
        listener->ImportedPackageWillBeAdded(node, importedPackagesNode, index);
    
    importedPackagesNode->InsertAtIndex(index, node);
    
    for (PackageListener *listener : listeners)
        listener->ImportedPackageWasAdded(node, importedPackagesNode, index);

    RebuildStyleSheets();
    RefreshLayout();
}

void PackageNode::RemoveImportedPackage(PackageNode *node)
{
    for (PackageListener *listener : listeners)
        listener->ImportedPackageWillBeRemoved(node, importedPackagesNode);
    
    importedPackagesNode->Remove(node);
    
    for (PackageListener *listener : listeners)
        listener->ImportedPackageWasRemoved(node, importedPackagesNode);

    RebuildStyleSheets();
    RefreshLayout();
}

void PackageNode::RefreshLayout()
{
    for (int32 i = 0; i < packageControlsNode->GetCount(); i++)
    {
        RefreshStyles(packageControlsNode->Get(i));
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

void PackageNode::RebuildStyleSheets()
{
    Vector<UIStyleSheet*> importedStyleSheets;
    for (int32 i = 0; i < importedPackagesNode->GetCount(); i++)
    {
        PackageNode *node = importedPackagesNode->GetImportedPackage(i);
        const Vector<UIStyleSheet*> &styleSheets = node->GetContext()->GetSortedStyleSheets();
        importedStyleSheets.insert(importedStyleSheets.end(), styleSheets.begin(), styleSheets.end());
    }
    
    std::sort(importedStyleSheets.begin(), importedStyleSheets.end());
    auto last = std::unique(importedStyleSheets.begin(), importedStyleSheets.end());
    importedStyleSheets.erase(last, importedStyleSheets.end());
    
    packageContext->RemoveAllStyleSheets();
    for (UIStyleSheet *styleSheet : importedStyleSheets)
    {
        packageContext->AddStyleSheet(styleSheet);
    }
    
    for (int32 i = 0; i < styleSheets->GetCount(); i++)
    {
        StyleSheetNode *node = styleSheets->Get(i);
        Vector<UIStyleSheet*> styleSheets = node->GetRootProperty()->CollectStyleSheets();
        for (UIStyleSheet *styleSheet : styleSheets)
            packageContext->AddStyleSheet(styleSheet);
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

void PackageNode::RefreshStyles(ControlNode *node)
{
    UIControlSystem::Instance()->GetStyleSheetSystem()->ProcessControl(node->GetControl());
    for (int32 i = 0; i < node->GetCount(); i++)
    {
        RefreshStyles(node->Get(i));
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
    RootProperty *rootProperty = container->GetRootProperty();
    for (int i = 0; i < rootProperty->GetControlPropertiesSection(0)->GetCount(); i++)
    {
        DependedOnLayoutProperty *prop = dynamic_cast<DependedOnLayoutProperty*>(rootProperty->GetControlPropertiesSection(0)->GetProperty(i));
        if (prop)
            prop->RestoreSourceValue();
    }

    for (int i = 0; i < container->GetCount(); i++)
        RestoreProperties(container->Get(i));
}

void PackageNode::NotifyPropertyChanged(ControlNode *control)
{
    RootProperty *rootProperty = control->GetRootProperty();
    for (int i = 0; i < rootProperty->GetControlPropertiesSection(0)->GetCount(); i++)
    {
        DependedOnLayoutProperty *prop = dynamic_cast<DependedOnLayoutProperty*>(rootProperty->GetControlPropertiesSection(0)->GetProperty(i));
        
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
