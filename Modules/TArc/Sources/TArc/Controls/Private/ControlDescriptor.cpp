#include "TArc/Controls/ControlDescriptor.h"

namespace DAVA
{
namespace TArc
{
DescriptorNode::DescriptorNode()
    : fieldName("")
{
}

DescriptorNode& DescriptorNode::operator=(const char* name)
{
    fieldName = FastName(name);
    return *this;
}

DescriptorNode& DescriptorNode::operator=(const String& name)
{
    fieldName = FastName(name);
    return *this;
}

DescriptorNode& DescriptorNode::operator=(const FastName& name)
{
    fieldName = name;
    return *this;
}

} // namespace TArc
} // namespace DAVA