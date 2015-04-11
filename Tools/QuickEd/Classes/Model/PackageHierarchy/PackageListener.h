#ifndef __QUICKED_PACKAGE_LISTENER_H__
#define __QUICKED_PACKAGE_LISTENER_H__

class ControlNode;
class ControlsContainerNode;

class PackageListener {
public:
    virtual void ControlWillBeAdded(ControlNode *node, ControlsContainerNode *destination, int index) = 0;
    virtual void ControlWasAdded(ControlNode *node, ControlsContainerNode *destination, int index) = 0;

    virtual void ControlWillBeRemoved(ControlNode *node, ControlsContainerNode *from) = 0;
    virtual void ControlWasRemoved(ControlNode *node, ControlsContainerNode *from) = 0;
    
};

#endif // __QUICKED_PACKAGE_LISTENER_H__
