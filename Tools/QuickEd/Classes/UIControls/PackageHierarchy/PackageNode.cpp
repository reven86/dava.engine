#include "PackageNode.h"

#include "PackageControlsNode.h"
#include "ImportedPackagesNode.h"
#include "../PackageSerializer.h"

using namespace DAVA;

PackageNode::PackageNode(DAVA::UIPackage *aPackage)
    : PackageBaseNode(NULL)
    , package(SafeRetain(aPackage))
    , importedPackagesNode(NULL)
    , packageControlsNode(NULL)
{
    importedPackagesNode = new ImportedPackagesNode(this);
    packageControlsNode = new PackageControlsNode(this, package);
}

PackageNode::~PackageNode()
{
    SafeRelease(package);
    importedPackagesNode->SetParent(NULL);
    SafeRelease(importedPackagesNode);
    packageControlsNode->SetParent(NULL);
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
        return NULL;
}

String PackageNode::GetName() const
{
    return package->GetName();
}

int PackageNode::GetFlags() const 
{
    return 0;
}

UIPackage *PackageNode::GetPackage() const
{
    return package;
}

ImportedPackagesNode *PackageNode::GetImportedPackagesNode() const
{
    return importedPackagesNode;
}

PackageControlsNode *PackageNode::GetPackageControlsNode() const
{
    return packageControlsNode;
}

void PackageNode::Serialize(PackageSerializer *serializer) const
{
    serializer->BeginMap("Header");
    serializer->PutValue("version", "0");
    serializer->EndMap();

    importedPackagesNode->Serialize(serializer);
    packageControlsNode->Serialize(serializer);
}
