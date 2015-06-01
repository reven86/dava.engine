#include "ControlPrototype.h"

#include "ControlNode.h"
#include "PackageRef.h"

using namespace DAVA;

ControlPrototype::ControlPrototype(ControlNode *_controlNode, PackageRef *_packageRef)
    : controlNode(SafeRetain(_controlNode))
    , packageRef(SafeRetain(_packageRef))
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

String ControlPrototype::GetName(bool withPackage) const
{
    String controlName = controlNode->GetName();
    
    if (withPackage)
        return packageRef->GetName() + "/" + controlName;
    else
        return  controlName;
}

PackageRef *ControlPrototype::GetPackageRef() const
{
    return packageRef;
}
