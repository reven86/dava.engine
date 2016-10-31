#pragma once

#include <memory>
#include "Base/BaseTypes.h"

namespace DAVA
{
class RttiType;
class StructureWrapper;
class ReflectedStructure;

class ReflectedType final
{
    friend class ReflectedTypeDB;

public:
    String permanentName;
    const RttiType* rttiType;

    ReflectedStructure* structure;
    StructureWrapper* structureWrapper;

protected:
    ReflectedType() = default;
    ~ReflectedType();
};

} // namespace DAVA
