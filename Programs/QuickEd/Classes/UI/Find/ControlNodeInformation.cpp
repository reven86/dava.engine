#include "ControlNodeInformation.h"
#include "Model/PackageHierarchy/PackageNode.h"

using namespace DAVA;

ControlNodeInformation::ControlNodeInformation(const ControlNode* controlNode_)
    : controlNode(controlNode_)
{
}

FastName ControlNodeInformation::GetName() const
{
    return FastName(controlNode->GetName());
}

FastName ControlNodeInformation::GetPrototype() const
{
    const ControlNode* prototype = controlNode->GetPrototype();

    if (prototype != nullptr)
    {
        return FastName(prototype->GetName());
    }
    else
    {
        return FastName();
    }
}

String ControlNodeInformation::GetPrototypePackagePath() const
{
    const ControlNode* prototype = controlNode->GetPrototype();

    if (prototype != nullptr)
    {
        return prototype->GetPackage()->GetPath().GetStringValue();
    }
    else
    {
        return String();
    }
}

void ControlNodeInformation::VisitParent(const Function<void(const ControlInformation*)>& visitor) const
{
    if (ControlNode* parentControlNode = dynamic_cast<ControlNode*>(controlNode->GetParent()))
    {
        ControlNodeInformation parentInfo(parentControlNode);

        visitor(&parentInfo);
    }
    else
    {
        visitor(nullptr);
    }
}

void ControlNodeInformation::VisitChildren(const Function<void(const ControlInformation*)>& visitor) const
{
    for (int32 childIndex = 0; childIndex < controlNode->GetCount(); ++childIndex)
    {
        if (ControlNode* childControlNode = dynamic_cast<ControlNode*>(controlNode->Get(childIndex)))
        {
            ControlNodeInformation childInfo(childControlNode);

            visitor(&childInfo);
        }
    }
}
