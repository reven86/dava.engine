#ifndef __QUICKED_PACKAGE_LISTENER_H__
#define __QUICKED_PACKAGE_LISTENER_H__

class PackageNode;
class ControlNode;
class ControlsContainerNode;
class StyleSheetNode;
class StyleSheetsNode;
class PackageControlsNode;
class AbstractProperty;
class ImportedPackagesNode;

class PackageListener
{
public:
    virtual ~PackageListener() = 0;
    virtual void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property){};
    virtual void StylePropertyWasChanged(StyleSheetNode* node, AbstractProperty* property){};

    virtual void ControlWillBeAdded(ControlNode* node, ControlsContainerNode* destination, int index){};
    virtual void ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int index){};

    virtual void ControlWillBeRemoved(ControlNode* node, ControlsContainerNode* from){};
    virtual void ControlWasRemoved(ControlNode* node, ControlsContainerNode* from){};

    virtual void StyleWillBeAdded(StyleSheetNode* node, StyleSheetsNode* destination, int index){};
    virtual void StyleWasAdded(StyleSheetNode* node, StyleSheetsNode* destination, int index){};

    virtual void StyleWillBeRemoved(StyleSheetNode* node, StyleSheetsNode* from){};
    virtual void StyleWasRemoved(StyleSheetNode* node, StyleSheetsNode* from){};

    virtual void ImportedPackageWillBeAdded(PackageNode* node, ImportedPackagesNode* to, int index){};
    virtual void ImportedPackageWasAdded(PackageNode* node, ImportedPackagesNode* to, int index){};

    virtual void ImportedPackageWillBeRemoved(PackageNode* node, ImportedPackagesNode* from){};
    virtual void ImportedPackageWasRemoved(PackageNode* node, ImportedPackagesNode* from){};
};

inline PackageListener::~PackageListener()
{
}

#endif // __QUICKED_PACKAGE_LISTENER_H__
