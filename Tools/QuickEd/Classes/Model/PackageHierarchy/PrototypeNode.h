#pragma once

#include "Model/PackageHierarchy/ControlNode.h"

class PrototypeNode : public ControlNode
{
public:
    PrototypeNode(DAVA::UIControl* control, bool recursively);
    PrototypeNode(ControlNode* node, eCreationType creationType);

    ~PrototypeNode();

private:
    DAVA::Vector<ControlNode*> instances; // weak
};
