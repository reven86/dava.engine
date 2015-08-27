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


#include "PackageSerializer.h"

#include "PackageHierarchy/PackageNode.h"
#include "PackageHierarchy/ImportedPackagesNode.h"
#include "PackageHierarchy/PackageControlsNode.h"
#include "PackageHierarchy/ControlNode.h"
#include "PackageHierarchy/StyleSheetsNode.h"
#include "PackageHierarchy/StyleSheetNode.h"

#include "ControlProperties/RootProperty.h"
#include "ControlProperties/BackgroundPropertiesSection.h"
#include "ControlProperties/ClassProperty.h"
#include "ControlProperties/ComponentPropertiesSection.h"
#include "ControlProperties/ControlPropertiesSection.h"
#include "ControlProperties/CustomClassProperty.h"
#include "ControlProperties/FontValueProperty.h"
#include "ControlProperties/InternalControlPropertiesSection.h"
#include "ControlProperties/IntrospectionProperty.h"
#include "ControlProperties/LocalizedTextValueProperty.h"
#include "ControlProperties/NameProperty.h"
#include "ControlProperties/PrototypeNameProperty.h"
#include "ControlProperties/StyleSheetRootProperty.h"
#include "ControlProperties/StyleSheetSelectorProperty.h"
#include "ControlProperties/StyleSheetProperty.h"

using namespace DAVA;

PackageSerializer::PackageSerializer()
{
    
}

PackageSerializer::~PackageSerializer()
{
 
    
}

void PackageSerializer::SerializePackage(PackageNode *package)
{
    for (int32 i = 0; i < package->GetImportedPackagesNode()->GetCount(); i++)
    {
        importedPackages.push_back(package->GetImportedPackagesNode()->GetImportedPackage(i));
    }
    
    for (int32 i = 0; i < package->GetStyleSheets()->GetCount(); i++)
    {
        styles.push_back(package->GetStyleSheets()->Get(i));
    }
    
    for (int32 i = 0; i < package->GetPackageControlsNode()->GetCount(); i++)
    {
        controls.push_back(package->GetPackageControlsNode()->Get(i));
    }
    
    package->Accept(this);
    importedPackages.clear();
    controls.clear();
    styles.clear();
}

void PackageSerializer::SerializePackageNodes(PackageNode *package, const DAVA::Vector<ControlNode*> &serializationControls, const DAVA::Vector<StyleSheetNode*> &serializationStyles)
{
    for (ControlNode *control : serializationControls)
    {
        if (control->CanCopy())
        {
            controls.push_back(control);
            CollectPackages(importedPackages, control);
        }
    }

    for (StyleSheetNode *style : serializationStyles)
    {
        if (style->CanCopy())
            styles.push_back(style);
    }

    package->Accept(this);

    importedPackages.clear();
    controls.clear();
    styles.clear();
}

void PackageSerializer::VisitPackage(PackageNode *node)
{
    BeginMap("Header");
    PutValue("version", Format("%d", CURRENT_VERSION));
    EndMap();

    if (!importedPackages.empty())
    {
        BeginArray("ImportedPackages");
        for (const PackageNode *package : importedPackages)
            PutValue(package->GetPath().GetFrameworkPath());
        EndArray();
    }
    
    if (!styles.empty())
    {
        BeginArray("StyleSheets");
        for (StyleSheetNode *style : styles)
            style->Accept(this);
        EndMap();
    }

    if (!controls.empty())
    {
        BeginArray("Controls");
        for (ControlNode *control : controls)
            control->Accept(this);
        EndArray();
    }
}

void PackageSerializer::VisitImportedPackages(ImportedPackagesNode *node)
{
    // do nothing
}

void PackageSerializer::VisitControls(PackageControlsNode *node)
{
    // do nothing
}

void PackageSerializer::VisitControl(ControlNode *node)
{
    BeginMap();
    
    node->GetRootProperty()->Accept(this);
    
    if (node->GetCount() > 0)
    {
        bool shouldProcessChildren = true;
        Vector<ControlNode*> prototypeChildrenWithChanges;
        
        if (node->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE)
        {
            CollectPrototypeChildrenWithChanges(node, prototypeChildrenWithChanges);
            shouldProcessChildren = !prototypeChildrenWithChanges.empty() || HasNonPrototypeChildren(node);
        }
        
        if (shouldProcessChildren)
        {
            BeginArray("children");
            
            for (const auto &child : prototypeChildrenWithChanges)
                child->Accept(this);
            
            for (int32 i = 0; i < node->GetCount(); i++)
            {
                if (node->Get(i)->GetCreationType() != ControlNode::CREATED_FROM_PROTOTYPE_CHILD)
                    node->Get(i)->Accept(this);
            }
            
            EndArray();
        }
    }
    
    EndMap();
}

void PackageSerializer::VisitStyleSheets(StyleSheetsNode *node)
{
    // do nothing
}

void PackageSerializer::VisitStyleSheet(StyleSheetNode *node)
{
    BeginMap();
    node->GetRootProperty()->Accept(this);
    EndMap();
}

void PackageSerializer::AcceptChildren(PackageBaseNode *node)
{
    for (int32 i = 0; i < node->GetCount(); i++)
        node->Get(i)->Accept(this);
}

void PackageSerializer::CollectPackages(Vector<PackageNode*> &packages, ControlNode *node) const
{
    if (node->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE)
    {
        ControlNode *prototype = node->GetPrototype();
        if (prototype && std::find(packages.begin(), packages.end(), prototype->GetPackage()) == packages.end())
        {
            packages.push_back(prototype->GetPackage());
        }
    }
    
    for (int32 index = 0; index < node->GetCount(); index++)
        CollectPackages(packages, node->Get(index));
}

bool PackageSerializer::IsControlInSerializationList(ControlNode *control) const
{
    return std::find(controls.begin(), controls.end(), control) != controls.end();
}

void PackageSerializer::CollectPrototypeChildrenWithChanges(ControlNode *node, Vector<ControlNode*> &out) const
{
    for (int32 i = 0; i < node->GetCount(); i++)
    {
        ControlNode *child = node->Get(i);
        if (child->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE_CHILD)
        {
            if (HasNonPrototypeChildren(child) || child->GetRootProperty()->HasChanges())
                out.push_back(child);
            
            CollectPrototypeChildrenWithChanges(child, out);
        }
    }
}

bool PackageSerializer::HasNonPrototypeChildren(ControlNode *node) const
{
    for (int32 i = 0; i < node->GetCount(); i++)
    {
        if (node->Get(i)->GetCreationType() != ControlNode::CREATED_FROM_PROTOTYPE_CHILD)
            return true;
    }
    return false;
}

// --- 

void PackageSerializer::VisitRootProperty(RootProperty *property)
{
    property->GetPrototypeProperty()->Accept(this);
    property->GetClassProperty()->Accept(this);
    property->GetCustomClassProperty()->Accept(this);
    property->GetNameProperty()->Accept(this);
    
    for (int32 i = 0; i < property->GetControlPropertiesSectionsCount(); i++)
        property->GetControlPropertiesSection(i)->Accept(this);
    
    bool hasChanges = false;
    
    for (const ComponentPropertiesSection *section : property->GetComponents())
    {
        if (section->HasChanges() || (section->GetFlags() & AbstractProperty::EF_INHERITED) == 0)
        {
            hasChanges = true;
            break;
        }
    }
    
    if (!hasChanges)
    {
        for (const auto section : property->GetBackgroundProperties())
        {
            if (section->HasChanges())
            {
                hasChanges = true;
                break;
            }
        }
    }
    
    if (!hasChanges)
    {
        for (const auto section : property->GetInternalControlProperties())
        {
            if (section->HasChanges())
            {
                hasChanges = true;
                break;
            }
        }
    }
    
    
    if (hasChanges)
    {
        BeginMap("components");
        
        for (const auto section : property->GetComponents())
            section->Accept(this);
        
        for (const auto section : property->GetBackgroundProperties())
            section->Accept(this);
        
        for (const auto section : property->GetInternalControlProperties())
            section->Accept(this);
        
        EndMap();
    }

}

void PackageSerializer::VisitControlSection(ControlPropertiesSection *property)
{
    AcceptChildren(property);
}

void PackageSerializer::VisitComponentSection(ComponentPropertiesSection *property)
{
    if (property->HasChanges() || (property->GetFlags() & AbstractProperty::EF_INHERITED) == 0)
    {
        String name = property->GetComponentName();
        if (UIComponent::IsMultiple(property->GetComponentType()))
            name += Format("%d", property->GetComponentIndex());
        
        BeginMap(name);
        AcceptChildren(property);
        EndMap();
    }
}

void PackageSerializer::VisitBackgroundSection(BackgroundPropertiesSection *property)
{
    if (property->HasChanges())
    {
        BeginMap(property->GetName());
        AcceptChildren(property);
        EndMap();
    }
}

void PackageSerializer::VisitInternalControlSection(InternalControlPropertiesSection *property)
{
    if (property->HasChanges())
    {
        BeginMap(property->GetName());
        AcceptChildren(property);
        EndMap();
    }
}

void PackageSerializer::VisitNameProperty(NameProperty *property)
{
    switch (property->GetControlNode()->GetCreationType())
    {
        case ControlNode::CREATED_FROM_PROTOTYPE:
        case ControlNode::CREATED_FROM_CLASS:
            PutValue("name", property->GetControlNode()->GetName());
            break;
            
        case ControlNode::CREATED_FROM_PROTOTYPE_CHILD:
            PutValue("path", property->GetControlNode()->GetPathToPrototypeChild());
            break;
            
        default:
            DVASSERT(false);
    }

}

void PackageSerializer::VisitPrototypeNameProperty(PrototypeNameProperty *property)
{
    if (property->GetControl()->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE)
    {
        ControlNode *prototype = property->GetControl()->GetPrototype();
        
        String name = "";
        PackageNode *prototypePackage = prototype->GetPackage();
        if (std::find(importedPackages.begin(), importedPackages.end(), prototypePackage) != importedPackages.end())
        {
            name = prototypePackage->GetName() + "/";
        }
        name += prototype->GetName();
        
        PutValue("prototype", name);
    }
}

void PackageSerializer::VisitClassProperty(ClassProperty *property)
{
    if (property->GetControlNode()->GetCreationType() == ControlNode::CREATED_FROM_CLASS)
    {
        PutValue("class", property->GetClassName());
    }
}

void PackageSerializer::VisitCustomClassProperty(CustomClassProperty *property)
{
    if (property->IsOverriddenLocally())
    {
        PutValue("customClass", property->GetCustomClassName());
    }
}

void PackageSerializer::VisitIntrospectionProperty(IntrospectionProperty *property)
{
    if (property->IsOverriddenLocally())
    {
        PutValueProperty(property->GetMember()->Name().c_str(), property);
    }
}

void PackageSerializer::VisitStyleSheetRoot(StyleSheetRootProperty *property)
{
    PutValue("selector", property->GetSelectorsAsString());

    BeginMap("properties", false);
    if (property->GetPropertiesSection()->GetCount() > 0)
    {
        AcceptChildren(property->GetPropertiesSection());
    }
    EndMap();
}

void PackageSerializer::VisitStyleSheetSelectorProperty(StyleSheetSelectorProperty *property)
{
    // do nothing
}

void PackageSerializer::VisitStyleSheetProperty(StyleSheetProperty *property)
{
    if (property->HasTransition())
    {
        BeginMap(property->GetName());
        PutValueProperty("value", property);
        PutValue("transitionTime", VariantType(property->GetTransitionTime()));

        const EnumMap *enumMap = GlobalEnumMap<Interpolation::FuncType>::Instance();
        PutValue("transitionFunction", enumMap->ToString(property->GetTransitionFunction()));
        EndMap();
    }
    else
    {
        PutValueProperty(property->GetName(), property);
    }
}

void PackageSerializer::AcceptChildren(AbstractProperty *property)
{
    for (int32 i = 0; i < property->GetCount(); i++)
        property->GetProperty(i)->Accept(this);
}

void PackageSerializer::PutValueProperty(const DAVA::String &name, ValueProperty *property)
{
    VariantType value = property->GetValue();
    
    if (value.GetType() == VariantType::TYPE_INT32 && property->GetType() == AbstractProperty::TYPE_FLAGS)
    {
        Vector<String> values;
        const EnumMap *enumMap = property->GetEnumMap();
        int val = value.AsInt32();
        int p = 1;
        while (val > 0)
        {
            if ((val & 0x01) != 0)
                values.push_back(enumMap->ToString(p));
            val >>= 1;
            p <<= 1;
        }
        PutValue(name, values);
    }
    else if (value.GetType() == VariantType::TYPE_INT32 && property->GetType() == AbstractProperty::TYPE_ENUM)
    {
        const EnumMap *enumMap = property->GetEnumMap();
        PutValue(name, enumMap->ToString(value.AsInt32()));
    }
    else
    {
        PutValue(name, value);
    }
}
