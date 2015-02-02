#include "ControlPrototype.h"

#include "ControlNode.h"

using namespace DAVA;

ControlPrototype::ControlPrototype(ControlNode *_controlNode, const FilePath &_packagePath)
    : controlNode(SafeRetain(_controlNode))
    , packagePath(_packagePath)
{
    
}

ControlPrototype::~ControlPrototype()
{
    SafeRelease(controlNode);
}

ControlNode *ControlPrototype::GetControlNode() const
{
    return controlNode;
}

String ControlPrototype::GetName() const
{
    if (!packagePath.IsEmpty())
        return packagePath.GetBasename() + "/" + controlNode->GetName();
    
    return controlNode->GetName();
}
