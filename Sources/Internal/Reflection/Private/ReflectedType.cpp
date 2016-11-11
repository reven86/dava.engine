#include "Reflection/Reflection.h"
#include "Reflection/ReflectedType.h"
#include "Reflection/ReflectedStructure.h"

namespace DAVA
{
ReflectedType::~ReflectedType() = default;

ReflectedType::ReflectedType(const RttiType* rttiType_)
    : rttiType(rttiType_)
{
}

Vector<const CtorWrapper*> ReflectedType::GetCtors() const
{
    Vector<const CtorWrapper*> ret;

    ret.reserve(structure->ctors.size());
    for (auto& it : structure->ctors)
    {
        ret.push_back(it.get());
    }

    return ret;
}

const DtorWrapper* ReflectedType::GetDtor() const
{
    return structure->dtor.get();
}

bool ReflectedType::HasDtor() const
{
    // TODO:
    // ...
    return false;
}

bool ReflectedType::Destroy(Any&& any) const
{
    // TODO:
    // ...
    return false;
}
} // namespace DAVA
