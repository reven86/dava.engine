#include "ImportedPackagesNode.h"

#include "PackageControlsNode.h"
#include "../PackageSerializer.h"
#include "PackageRef.h"

using namespace DAVA;

ImportedPackagesNode::ImportedPackagesNode(PackageBaseNode *parent) : PackageBaseNode(parent)
{
}

ImportedPackagesNode::~ImportedPackagesNode()
{
    for (auto it = packageControlsNode.begin(); it != packageControlsNode.end(); ++it)
        (*it)->Release();
    packageControlsNode.clear();
}

void ImportedPackagesNode::Add(PackageControlsNode *node)
{
    DVASSERT(node->GetParent() == NULL);
    node->SetParent(this);
    node->SetReadOnly();
    packageControlsNode.push_back(SafeRetain(node));
}

void ImportedPackagesNode::InsertAtIndex(DAVA::int32 index, PackageControlsNode *node)
{
    DVASSERT(node->GetParent() == NULL);
    node->SetParent(this);
    node->SetReadOnly();
    
    packageControlsNode.insert(packageControlsNode.begin() + index, SafeRetain(node));
}

void ImportedPackagesNode::Remove(PackageControlsNode *node)
{
    auto it = find(packageControlsNode.begin(), packageControlsNode.end(), node);
    if (it != packageControlsNode.end())
    {
        DVASSERT(node->GetParent() == this);
        node->SetParent(NULL);
        
        packageControlsNode.erase(it);
        SafeRelease(node);
    }
    else
    {
        DVASSERT(false);
    }
}

int ImportedPackagesNode::GetCount() const
{
    return (int) packageControlsNode.size();
}

PackageControlsNode *ImportedPackagesNode::Get(int index) const
{
    return packageControlsNode[index];
}

String ImportedPackagesNode::GetName() const
{
    return "Imported Packages";
}

PackageControlsNode *ImportedPackagesNode::FindPackageControlsNodeByName(const DAVA::String &name) const
{
    for (PackageControlsNode *node : packageControlsNode)
    {
        if (node->GetPackageRef()->GetName() == name)
            return node;
    }
    return nullptr;
}

int ImportedPackagesNode::GetFlags() const
{
    return FLAG_READ_ONLY;
}

void ImportedPackagesNode::Serialize(PackageSerializer *serializer) const
{
    serializer->BeginArray("ImportedPackages");
    
    for (PackageControlsNode *controlsNode : packageControlsNode)
        serializer->PutValue(controlsNode->GetPackageRef()->GetPath().GetFrameworkPath());
    
    serializer->EndArray();
}

void ImportedPackagesNode::Serialize(PackageSerializer *serializer, const DAVA::Set<PackageRef*> &packageRefs) const
{
    serializer->BeginArray("ImportedPackages");
    
    for (PackageControlsNode *controlsNode : packageControlsNode)
    {
        PackageRef *ref = controlsNode->GetPackageRef();
        if (packageRefs.find(ref) != packageRefs.end())
            serializer->PutValue(ref->GetPath().GetFrameworkPath());
    }
    
    serializer->EndArray();
}
