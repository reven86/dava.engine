#include "ImportedPackagesNode.h"

#include "PackageControlsNode.h"
#include "PackageNode.h"
#include "../PackageSerializer.h"

using namespace DAVA;

ImportedPackagesNode::ImportedPackagesNode(PackageBaseNode *parent) : PackageBaseNode(parent)
{
}

ImportedPackagesNode::~ImportedPackagesNode()
{
    for (PackageNode *package : packages)
        package->Release();
    packages.clear();
}

void ImportedPackagesNode::Add(PackageNode *node)
{
    DVASSERT(node->GetParent() == NULL);
    node->SetParent(this);
    packages.push_back(SafeRetain(node));
}

void ImportedPackagesNode::InsertAtIndex(DAVA::int32 index, PackageNode *node)
{
    DVASSERT(node->GetParent() == NULL);
    node->SetParent(this);
    
    packages.insert(packages.begin() + index, SafeRetain(node));
}

void ImportedPackagesNode::Remove(PackageNode *node)
{
    auto it = find(packages.begin(), packages.end(), node);
    if (it != packages.end())
    {
        DVASSERT(node->GetParent() == this);
        node->SetParent(NULL);
        
        packages.erase(it);
        SafeRelease(node);
    }
    else
    {
        DVASSERT(false);
    }
}

PackageNode *ImportedPackagesNode::GetImportedPackage(DAVA::int32 index) const
{
    return packages[index];
}

int ImportedPackagesNode::GetCount() const
{
    return (int) packages.size();
}

PackageBaseNode *ImportedPackagesNode::Get(int index) const
{
    return packages[index];
}

String ImportedPackagesNode::GetName() const
{
    return "Imported Packages";
}

bool ImportedPackagesNode::CanInsertImportedPackage() const
{
    return true;
}

PackageNode *ImportedPackagesNode::FindPackageByName(const DAVA::String &name) const
{
    for (PackageNode *node : packages)
    {
        if (node->GetName() == name)
            return node;
    }
    return nullptr;
}

void ImportedPackagesNode::Serialize(PackageSerializer *serializer) const
{
    serializer->BeginArray("ImportedPackages");
    
    for (PackageNode *package : packages)
        serializer->PutValue(package->GetPath().GetFrameworkPath());
    
    serializer->EndArray();
}

void ImportedPackagesNode::Serialize(PackageSerializer *serializer, const DAVA::Set<PackageNode*> &serializationPackages) const
{
    serializer->BeginArray("ImportedPackages");
    
    for (PackageNode *package : serializationPackages)
    {
        DVASSERT(package->GetParent() == this);
        serializer->PutValue(package->GetPath().GetFrameworkPath());
    }
    
    serializer->EndArray();
}

bool ImportedPackagesNode::IsReadOnly() const
{
    return true;
}
