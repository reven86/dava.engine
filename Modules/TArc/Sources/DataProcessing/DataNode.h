#pragma once

#include "Functional/Signal.h"
#include "Reflection/Public/ReflectedBase.h"
#include "Reflection/Registrator.h"

namespace DAVA
{
namespace TArc
{
class DataNode : public ReflectedBase
{
    DAVA_VIRTUAL_REFLECTION(DataNode) {}

public:
    virtual ~DataNode() = default;
};
} // namespace TArc
} // namespace DAVA
