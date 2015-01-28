#include "ImportedPackagesNode.h"

#include "PackageControlsNode.h"
#include "../PackageSerializer.h"

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

int ImportedPackagesNode::GetCount() const
{
    return (int) packageControlsNode.size();
}

PackageBaseNode *ImportedPackagesNode::Get(int index) const
{
    return packageControlsNode[index];
}

String ImportedPackagesNode::GetName() const
{
    return "Imported Packages";
}

PackageControlsNode *ImportedPackagesNode::FindPackageControlsNodeByName(const DAVA::String &name) const
{
    for (auto it = packageControlsNode.begin(); it != packageControlsNode.end(); ++it)
    {
        if ((*it)->GetName() == name)
            return *it;
    }
    return NULL;
}

int ImportedPackagesNode::GetFlags() const
{
    return FLAG_READ_ONLY;
}

void ImportedPackagesNode::Serialize(PackageSerializer *serializer) const
{
    serializer->BeginArray("ImportedPackages");
    
    for (auto it = packageControlsNode.begin(); it != packageControlsNode.end(); ++it)
        serializer->PutValue((*it)->GetPackagePath().GetFrameworkPath());
    
    serializer->EndArray();
}
