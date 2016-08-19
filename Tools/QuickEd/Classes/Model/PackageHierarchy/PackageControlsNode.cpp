#include "PackageControlsNode.h"

#include "ControlNode.h"
#include "PackageVisitor.h"

#include "PackageNode.h"
#include "UI/UIPackage.h"
#include "UI/UIControl.h"

using namespace DAVA;

PackageControlsNode::PackageControlsNode(PackageNode* _parent)
    : ControlsContainerNode(_parent)
{
}

PackageControlsNode::~PackageControlsNode()
{
    for (ControlNode* node : nodes)
        node->Release();
    nodes.clear();
}

void PackageControlsNode::Add(ControlNode* node)
{
    DVASSERT(node->GetParent() == nullptr);
    DVASSERT(node->GetPackageContext() == nullptr);
    node->SetParent(this);
    node->SetPackageContext(GetPackage()->GetContext());
    nodes.push_back(SafeRetain(node));
}

void PackageControlsNode::InsertAtIndex(int index, ControlNode* node)
{
    DVASSERT(node->GetParent() == nullptr);
    DVASSERT(node->GetPackageContext() == nullptr);
    node->SetParent(this);
    node->SetPackageContext(GetPackage()->GetContext());
    nodes.insert(nodes.begin() + index, SafeRetain(node));
}

void PackageControlsNode::Remove(ControlNode* node)
{
    auto it = find(nodes.begin(), nodes.end(), node);
    if (it != nodes.end())
    {
        DVASSERT(node->GetParent() == this);
        node->SetParent(nullptr);

        DVASSERT(node->GetPackageContext() == GetPackage()->GetContext());
        node->SetPackageContext(nullptr);

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
    return static_cast<int>(nodes.size());
}

ControlNode* PackageControlsNode::Get(int index) const
{
    return nodes[index];
}

void PackageControlsNode::Accept(PackageVisitor* visitor)
{
    visitor->VisitControls(this);
}

String PackageControlsNode::GetName() const
{
    return "Controls";
}

bool PackageControlsNode::IsEditingSupported() const
{
    return false;
}

bool PackageControlsNode::IsInsertingControlsSupported() const
{
    return !IsReadOnly();
}

bool PackageControlsNode::CanInsertControl(const ControlNode* node, DAVA::int32 pos) const
{
    return !IsReadOnly();
}

bool PackageControlsNode::CanRemove() const
{
    return false;
}

bool PackageControlsNode::CanCopy() const
{
    return false;
}

void PackageControlsNode::RefreshControlProperties()
{
    for (ControlNode* node : nodes)
        node->RefreshProperties();
}

ControlNode* PackageControlsNode::FindControlNodeByName(const DAVA::String& name) const
{
    FastName fName(name);
    for (auto it = nodes.begin(); it != nodes.end(); ++it)
    {
        if ((*it)->GetControl()->GetName() == fName)
            return *it;
    }
    return NULL;
}

DAVA::Vector<ControlNode*>::const_iterator PackageControlsNode::begin() const
{
    return nodes.begin();
}

DAVA::Vector<ControlNode*>::const_iterator PackageControlsNode::end() const
{
    return nodes.end();
}

DAVA::Vector<ControlNode*>::iterator PackageControlsNode::begin()
{
    return nodes.begin();
}

DAVA::Vector<ControlNode*>::iterator PackageControlsNode::end()
{
    return nodes.end();
}
