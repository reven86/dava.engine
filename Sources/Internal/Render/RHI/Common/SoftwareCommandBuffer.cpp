#include "SoftwareCommandBuffer.h"

namespace rhi
{
void SoftwareCommandBuffer::Begin()
{
    curUsedSize = 0;
}

void SoftwareCommandBuffer::End()
{
}
}