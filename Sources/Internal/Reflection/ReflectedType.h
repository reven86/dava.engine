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
    const RttiType* rttiType;
    String permanentName;

    std::unique_ptr<ReflectedStructure, void (*)(ReflectedStructure*)> structure;
    std::unique_ptr<StructureWrapper, void (*)(StructureWrapper*)> structureWrapper;

protected:
    ReflectedType(const RttiType* rttiType_);
};

} // namespace DAVA
