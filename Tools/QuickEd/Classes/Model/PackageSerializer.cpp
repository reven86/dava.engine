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
    : forceQualifiedName(false)
{
    
}

PackageSerializer::~PackageSerializer()
{
 
    
}

void PackageSerializer::SerializePackage(PackageNode *node)
{
    node->Accept(this);
}

void PackageSerializer::SerializePackageNodes(PackageNode *node, const DAVA::Vector<ControlNode*> &nodes)
{
    node->Accept(this);
}

bool PackageSerializer::IsForceQualifiedName() const
{
    return forceQualifiedName;
}

void PackageSerializer::SetForceQualifiedName(bool qualifiedName)
{
    forceQualifiedName = qualifiedName;
}

void PackageSerializer::VisitPackage(PackageNode *node)
{
    BeginMap("Header");
    PutValue("version", String("0"));
    EndMap();
    
    AcceptChildren(node);
}

void PackageSerializer::VisitImportedPackages(ImportedPackagesNode *node)
{
    BeginArray("ImportedPackages");
    
    for (int32 i = 0; i < node->GetCount(); i++)
        PutValue(node->GetImportedPackage(i)->GetPath().GetFrameworkPath());
    
    EndArray();
}

void PackageSerializer::VisitControls(PackageControlsNode *node)
{
    BeginArray("Controls");
    AcceptChildren(node);
    EndArray();

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
            PutValue("path", property->GetControlNode()->GetPathToPrototypeChild(false));
            break;
            
        default:
            DVASSERT(false);
    }

}

void PackageSerializer::VisitPrototypeNameProperty(PrototypeNameProperty *property)
{
    if (property->GetControl()->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE)
    {
        PutValue("prototype", property->GetControl()->GetPrototype()->GetQualifiedName(forceQualifiedName));
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
