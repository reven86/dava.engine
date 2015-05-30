#include "PackageSerializer.h"

#include "PackageHierarchy/PackageNode.h"
#include "PackageHierarchy/ImportedPackagesNode.h"
#include "PackageHierarchy/PackageControlsNode.h"
#include "PackageHierarchy/ControlNode.h"
#include "ControlProperties/RootProperty.h"

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
    
    node->GetRootProperty()->Serialize(this);
    
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

