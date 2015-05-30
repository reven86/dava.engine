#include "PackageSerializer.h"

#include "PackageHierarchy/PackageNode.h"
#include "PackageHierarchy/ImportedPackagesNode.h"
#include "PackageHierarchy/PackageControlsNode.h"
#include "PackageHierarchy/ControlNode.h"

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
    
    for (int32 i = 0; i < package->GetPackageControlsNode()->GetCount(); i++)
    {
        controls.push_back(package->GetPackageControlsNode()->Get(i));
    }
    
    package->Accept(this);
    importedPackages.clear();
    controls.clear();
}

void PackageSerializer::SerializePackageNodes(PackageNode *package, const DAVA::Vector<ControlNode*> &serializationControls)
{
    for (ControlNode *control : serializationControls)
    {
        if (control->CanCopy())
            controls.push_back(control);
    }

    for (ControlNode *control : controls)
    {
        CollectPackages(importedPackages, control);
    }

    package->Accept(this);

    importedPackages.clear();
    controls.clear();
}

void PackageSerializer::VisitPackage(PackageNode *node)
{
    BeginMap("Header");
    PutValue("version", String("0"));
    EndMap();
    
    BeginArray("ImportedPackages");
    for (PackageNode *package : importedPackages)
        PutValue(package->GetPath().GetFrameworkPath());
    EndArray();

    BeginArray("Controls");
    for (ControlNode *control : controls)
        control->Accept(this);
    EndArray();
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
        if (!IsControlInSerializationList(prototype))
        {
            if (prototype && std::find(packages.begin(), packages.end(), prototype->GetPackage()) == packages.end())
            {
                packages.push_back(prototype->GetPackage());
            }
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
        
        EndArray();
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
            name += Format("%d", index);
        
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
        if (!IsControlInSerializationList(prototype))
        {
            if (prototypePackage != nullptr)
            {
                name = prototypePackage->GetName() + "/";
            }
            else
            {
                DVASSERT(false);
            }
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
    if (property->IsReplaced())
    {
        PutValue("customClass", property->GetCustomClassName());
    }
}

void PackageSerializer::VisitIntrospectionProperty(IntrospectionProperty *property)
{
    if (property->IsReplaced())
    {
        VariantType value = property->GetValue();
        String key = property->GetMember()->Name();
        
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
            PutValue(key, values);
        }
        else if (value.GetType() == VariantType::TYPE_INT32 && property->GetType() == AbstractProperty::TYPE_ENUM)
        {
            const EnumMap *enumMap = property->GetEnumMap();
            PutValue(key, enumMap->ToString(value.AsInt32()));
        }
        else
        {
            PutValue(key, value);
        }
    }
}

void PackageSerializer::AcceptChildren(AbstractProperty *property)
{
    for (int32 i = 0; i < property->GetCount(); i++)
        property->GetProperty(i)->Accept(this);
}
