#ifndef __QUICKED_CONTROLS_CONTAINER_NODE_H__
#define __QUICKED_CONTROLS_CONTAINER_NODE_H__

#include "PackageBaseNode.h"

class ControlNode;

class ControlsContainerNode : public PackageBaseNode
{
public:
    ControlsContainerNode(PackageBaseNode* parent);

    virtual DAVA::Vector<ControlNode*>::const_iterator begin() const = 0;
    virtual DAVA::Vector<ControlNode*>::const_iterator end() const = 0;

    virtual DAVA::Vector<ControlNode*>::iterator begin() = 0;
    virtual DAVA::Vector<ControlNode*>::iterator end() = 0;

protected:
    virtual ~ControlsContainerNode();

public:
    virtual void Add(ControlNode* node) = 0;
    virtual void InsertAtIndex(int index, ControlNode* node) = 0;
    virtual void Remove(ControlNode* node) = 0;
};

#endif // __QUICKED_CONTROLS_CONTAINER_NODE_H__
