#pragma once

#include "ReflectedObject.h"

namespace DAVA
{
class ReflectedType;
class ReflectedMeta;
class ValueWrapper;

struct ReflectionRaw
{
    ReflectedObject object;

    const ReflectedType* type;
    const ReflectedMeta* meta;
    const ValueWrapper* propertieWrapper;
};
} // namespace DAVA
