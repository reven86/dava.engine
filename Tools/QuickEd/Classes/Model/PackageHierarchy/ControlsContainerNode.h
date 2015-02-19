#ifndef __QUICKED_CONTROLS_CONTAINER_NODE_H__
#define __QUICKED_CONTROLS_CONTAINER_NODE_H__

#include "PackageBaseNode.h"

class ControlNode;

class ControlsContainerNode : public PackageBaseNode
{
public:
    ControlsContainerNode(PackageBaseNode *parent);
    
protected:
    virtual ~ControlsContainerNode();
    
public:
    virtual void Add(ControlNode *node) = 0;
    virtual void InsertAtIndex(int index, ControlNode *node) = 0;
    virtual void Remove(ControlNode *node) = 0;

};

#endif // __QUICKED_CONTROLS_CONTAINER_NODE_H__
