#include "PrototypeNode.h"

PrototypeNode::PrototypeNode(DAVA::UIControl* control, bool recursively)
    : ControlNode(control, recursively)
{
}

PrototypeNode::PrototypeNode(ControlNode* node, eCreationType creationType)
    : ControlNode(node, creationType)
{
}

PrototypeNode::~PrototypeNode()
{
}
