//
//  ImportedPackagesNode.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 9.10.14.
//
//

#include "ImportedPackagesNode.h"

#include "PackageControlsNode.h"

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

YamlNode *ImportedPackagesNode::Serialize() const
{
    YamlNode *arrayNode = YamlNode::CreateArrayNode(YamlNode::AR_BLOCK_REPRESENTATION);
    for (auto it = packageControlsNode.begin(); it != packageControlsNode.end(); ++it)
        arrayNode->Add((*it)->GetPackagePath().GetFrameworkPath());
    return arrayNode;
}
