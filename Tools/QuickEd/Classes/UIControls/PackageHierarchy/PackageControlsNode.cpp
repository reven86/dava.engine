#include "PackageControlsNode.h"

#include "ControlNode.h"

#include "../PackageSerializer.h"

using namespace DAVA;

PackageControlsNode::PackageControlsNode(PackageBaseNode *parent, UIPackage *aPackage)
    : PackageBaseNode(parent)
    , name("Controls")
    , package(SafeRetain(aPackage))
    , readOnly(false)
{
}

PackageControlsNode::~PackageControlsNode()
{
    for (auto it = nodes.begin(); it != nodes.end(); ++it)
        (*it)->Release();
    nodes.clear();
    SafeRelease(package);
}

void PackageControlsNode::Add(ControlNode *node)
{
    DVASSERT(node->GetParent() == NULL);
    node->SetParent(this);
    nodes.push_back(SafeRetain(node));
    package->AddControl(node->GetControl());
}

void PackageControlsNode::InsertBelow(ControlNode *node, const ControlNode *belowThis)
{
    DVASSERT(node->GetParent() == NULL);
    node->SetParent(this);
    auto it = find(nodes.begin(), nodes.end(), belowThis);
    if (it != nodes.end())
    {
        package->InsertControlBelow(node->GetControl(), (*it)->GetControl());
        nodes.insert(it, SafeRetain(node));
    }
    else
    {
        nodes.push_back(SafeRetain(node));
        package->AddControl(node->GetControl());
    }
}

void PackageControlsNode::Remove(ControlNode *node)
{
    auto it = find(nodes.begin(), nodes.end(), node);
    if (it != nodes.end())
    {
        DVASSERT(node->GetParent() == this);
        node->SetParent(NULL);

        package->RemoveControl(node->GetControl());
        nodes.erase(it);
        SafeRelease(node);
    }
    else
    {
        DVASSERT(false);
    }
}

int PackageControlsNode::GetCount() const
{
    return (int) nodes.size();
}

ControlNode *PackageControlsNode::Get(int index) const
{
    return nodes[index];
}

String PackageControlsNode::GetName() const
{
    return name;
}

void PackageControlsNode::SetName(const DAVA::String &name)
{
    this->name = name;
}

UIPackage *PackageControlsNode::GetPackage() const
{
    return package;
}

const FilePath &PackageControlsNode::GetPackagePath() const
{
    return package->GetFilePath();
}

int PackageControlsNode::GetFlags() const
{
    return readOnly ? FLAG_READ_ONLY : 0;
}

void PackageControlsNode::SetReadOnly()
{
    readOnly = true;
    for (auto it = nodes.begin(); it != nodes.end(); ++it)
    {
        (*it)->SetReadOnly();
    }
}

ControlNode *PackageControlsNode::FindControlNodeByName(const DAVA::String &name) const
{
    for (auto it = nodes.begin(); it != nodes.end(); ++it)
    {
        if ((*it)->GetControl()->GetName() == name)
            return *it;
    }
    return NULL;
}

void PackageControlsNode::Serialize(PackageSerializer *serializer) const
{
    serializer->BeginArray("Controls");
    
    for (auto it = nodes.begin(); it != nodes.end(); ++it)
        (*it)->Serialize(serializer);
    
    serializer->EndArray();
}
