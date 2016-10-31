#pragma once
#include "Reflection/ReflectedObject.h"

namespace DAVA
{
class ReflectedType;
class ReflectedMeta;
class ReflectedObject;
class PropertieWrapper;

class ReflectionWrapper
{
public:
    const ReflectedType* type;
    const ReflectedMeta* meta;
    const PropertieWrapper* propertieWrapper;
};

} // namespace DAVA
