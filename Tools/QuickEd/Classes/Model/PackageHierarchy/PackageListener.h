#ifndef __QUICKED_PACKAGE_LISTENER_H__
#define __QUICKED_PACKAGE_LISTENER_H__

class PackageNode;
class ControlNode;
class ControlsContainerNode;
class PackageControlsNode;
class AbstractProperty;

class PackageListener
{
public:
    virtual void ControlPropertyWasChanged(ControlNode *node, AbstractProperty *property) = 0;
    
    virtual void ControlWillBeAdded(ControlNode *node, ControlsContainerNode *destination, int index) = 0;
    virtual void ControlWasAdded(ControlNode *node, ControlsContainerNode *destination, int index) = 0;

    virtual void ControlWillBeRemoved(ControlNode *node, ControlsContainerNode *from) = 0;
    virtual void ControlWasRemoved(ControlNode *node, ControlsContainerNode *from) = 0;
    
    virtual void ImportedPackageWillBeAdded(PackageNode *node, PackageNode *to, int index) = 0;
    virtual void ImportedPackageWasAdded(PackageNode *node, PackageNode *to, int index) = 0;

    virtual void ImportedPackageWillBeRemoved(PackageNode *node, PackageNode *from) = 0;
    virtual void ImportedPackageWasRemoved(PackageNode *node, PackageNode *from) = 0;
    
};

#endif // __QUICKED_PACKAGE_LISTENER_H__
