#ifndef __QUICKED_PACKAGE_LISTENER_H__
#define __QUICKED_PACKAGE_LISTENER_H__

class PackageNode;
class ControlNode;
class ControlsContainerNode;
class PackageControlsNode;
class BaseProperty;

class PackageListener
{
public:
    virtual void ControlPropertyWasChanged(ControlNode *node, BaseProperty *property) = 0;
    
    virtual void ControlWillBeAdded(ControlNode *node, ControlsContainerNode *destination, int index) = 0;
    virtual void ControlWasAdded(ControlNode *node, ControlsContainerNode *destination, int index) = 0;

    virtual void ControlWillBeRemoved(ControlNode *node, ControlsContainerNode *from) = 0;
    virtual void ControlWasRemoved(ControlNode *node, ControlsContainerNode *from) = 0;
    
    virtual void ImportedPackageWillBeAdded(PackageControlsNode *node, PackageNode *to, int index) = 0;
    virtual void ImportedPackageWasAdded(PackageControlsNode *node, PackageNode *to, int index) = 0;

    virtual void ImportedPackageWillBeRemoved(PackageControlsNode *node, PackageNode *from) = 0;
    virtual void ImportedPackageWasRemoved(PackageControlsNode *node, PackageNode *from) = 0;
    
};

#endif // __QUICKED_PACKAGE_LISTENER_H__
