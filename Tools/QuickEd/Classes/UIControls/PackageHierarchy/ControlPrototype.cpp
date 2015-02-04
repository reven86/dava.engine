#include "ControlPrototype.h"

#include "ControlNode.h"
#include "PackageRef.h"

using namespace DAVA;

ControlPrototype::ControlPrototype(ControlNode *_controlNode, PackageRef *_packageRef, bool _isPackageImported)
    : controlNode(SafeRetain(_controlNode))
    , packageRef(_packageRef)
    , isPackageImported(_isPackageImported)
{
    DVASSERT(packageRef);
}

ControlPrototype::~ControlPrototype()
{
    SafeRelease(controlNode);
    SafeRelease(packageRef);
}

ControlNode *ControlPrototype::GetControlNode() const
{
    return controlNode;
}

String ControlPrototype::GetName() const
{
    if (isPackageImported)
        return packageRef->GetName() + "/" + controlNode->GetName();
    else
        return controlNode->GetName();
}

PackageRef *ControlPrototype::GetPackageRef() const
{
    return packageRef;
}
