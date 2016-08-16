#pragma once

#include "Functional/Signal.h"
#include "Reflection/Public/ReflectedBase.h"
#include "Reflection/Registrator.h"

namespace tarc
{

class DataNode : public DAVA::ReflectedBase
{
    DAVA_VIRTUAL_REFLECTION(DataNode) {}

public:
    virtual ~DataNode() = default;
};

}