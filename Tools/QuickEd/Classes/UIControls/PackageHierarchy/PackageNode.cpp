#include "PackageNode.h"

#include "PackageControlsNode.h"
#include "ImportedPackagesNode.h"
#include "../PackageSerializer.h"
#include "ControlPrototype.h"
#include "PackageRef.h"

using namespace DAVA;

PackageNode::PackageNode(PackageRef *_packageRef)
    : PackageBaseNode(nullptr)
    , packageRef(SafeRetain(_packageRef))
    , importedPackagesNode(nullptr)
    , packageControlsNode(nullptr)
{
    importedPackagesNode = new ImportedPackagesNode(this);
    packageControlsNode = new PackageControlsNode(this, packageRef);
}

PackageNode::~PackageNode()
{
    SafeRelease(packageRef);
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
    return packageRef->GetName();
}

int PackageNode::GetFlags() const 
{
    return 0;
}

PackageRef *PackageNode::GetPackageRef() const
{
    return packageRef;
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
    serializer->PutValue("version", String("0"));
    serializer->EndMap();

    importedPackagesNode->Serialize(serializer);
    packageControlsNode->Serialize(serializer);
}

void PackageNode::AddControlWithResolvingDependencies(ControlNode *sourceControl)
{
    Set<FilePath> newPackages;
    CollectPackages(newPackages, sourceControl);
    
    for (const FilePath &path : newPackages)
    {
        bool found = false;
        for (int32 index = 0; index < importedPackagesNode->GetCount(); index++)
        {
            PackageControlsNode *controlNode = importedPackagesNode->Get(index);
            if (controlNode->GetPackageRef()->GetPath() == path)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            ScopedPtr<PackageRef> ref(new PackageRef(path, nullptr));
            ScopedPtr<PackageControlsNode> newPackageControlsNode(new PackageControlsNode(nullptr, ref));
            importedPackagesNode->Add(newPackageControlsNode);
        }
    }
    
    ScopedPtr<ControlNode> dest(sourceControl->Clone());
    packageControlsNode->Add(dest);
}


void PackageNode::CollectPackages(Set<FilePath> &packages, ControlNode *node) const
{
    if (node->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE)
    {
        ControlPrototype *prototype = node->GetPrototype();
        if (prototype)
        {
            
            if (packages.find(prototype->GetPackageRef()->GetPath()) == packages.end())
                packages.insert(prototype->GetPackageRef()->GetPath());
        }
    }
    
    for (int32 index = 0; index < node->GetCount(); index++)
        CollectPackages(packages, node->Get(index));
}
