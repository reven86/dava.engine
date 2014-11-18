//
//  PackageNode.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 9.10.14.
//
//

#include "PackageNode.h"

#include "PackageControlsNode.h"
#include "ImportedPackagesNode.h"

using namespace DAVA;

PackageNode::PackageNode(DAVA::UIPackage *package)
    : PackageBaseNode(NULL)
    , package(SafeRetain(package))
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

YamlNode *PackageNode::Serialize() const
{
    YamlNode *node = YamlNode::CreateMapNode(false, YamlNode::MR_BLOCK_REPRESENTATION, YamlNode::SR_PLAIN_REPRESENTATION);
    
    YamlNode *headerNode = YamlNode::CreateMapNode(false, YamlNode::MR_BLOCK_REPRESENTATION, YamlNode::SR_PLAIN_REPRESENTATION);
    headerNode->Add("version", "0");
    
    node->Add("Header", headerNode);
    node->Add("ImportedPackages", importedPackagesNode->Serialize());
    node->Add("Controls", packageControlsNode->Serialize());
    
    return node;
}
