#pragma once

#include "Functional/Signal.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
namespace TArc
{
class DataNode : public ReflectionBase
{
    DAVA_VIRTUAL_REFLECTION(DataNode)
    {
    }

public:
    virtual ~DataNode() = default;
};
} // namespace TArc
} // namespace DAVA
