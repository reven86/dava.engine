#pragma once

#include "Functional/Signal.h"
#include "Reflection/Reflection.h"

#define IMPLEMENT_TYPE(className) \
    const DAVA::Type* GetType() const override { return DAVA::Type::Instance<className>(); }

namespace tarc
{

class DataNode : public DAVA::VirtualReflection
{
    DAVA_DECLARE_TYPE_VIRTUAL_REFLECTION
public:
    virtual const DAVA::Type* GetType() const
    {
        return DAVA::Type::Instance<DataNode>();
    }
};

}