#include "PackageBaseNode.h"

using namespace DAVA;

PackageBaseNode::PackageBaseNode(PackageBaseNode *parent) : parent(parent)
{
    
}

PackageBaseNode::~PackageBaseNode()
{
    parent = NULL;
}

int PackageBaseNode::GetIndex(PackageBaseNode *node) const
{
    for (int i = 0; i < GetCount(); i++)
    {
        if (Get(i) == node)
            return i;
    }
    return -1;
}

PackageBaseNode *PackageBaseNode::GetParent() const
{
    return parent;
}

void PackageBaseNode::SetParent(PackageBaseNode *parent)
{
    this->parent = parent;
}

String PackageBaseNode::GetName() const
{
    return "Unknown";
}

UIControl *PackageBaseNode::GetControl() const
{
    return NULL;
}

void PackageBaseNode::debugDump(int depth)
{
    String str;
    for (int i = 0; i < depth; i++)
        str += ' ';
    Logger::Debug("%sNode %s (%s), %d", str.c_str(), GetName().c_str(), typeid(this).name(), this->GetRetainCount());
    for (int i = 0; i < GetCount(); i++)
        Get(i)->debugDump(depth + 2);
}
