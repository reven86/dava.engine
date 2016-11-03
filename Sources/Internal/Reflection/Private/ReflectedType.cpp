#include "Reflection/ReflectedType.h"
#include "Reflection/ReflectedStructure.h"
#include "Reflection/WrappersRuntime.h"

namespace DAVA
{
ReflectedType::ReflectedType(const RttiType* rttiType_)
    : rttiType(rttiType_)
    , structure(nullptr, [](ReflectedStructure* p) { if (nullptr != p) delete p; })
    , structureWrapper(nullptr, [](StructureWrapper* p) { if (nullptr != p) delete p; })
{
}
} // namespace DAVA
