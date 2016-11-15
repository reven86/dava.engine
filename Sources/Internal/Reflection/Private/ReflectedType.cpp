#include "Reflection/Reflection.h"

namespace DAVA
{
ReflectedType::~ReflectedType() = default;

ReflectedType::ReflectedType(const RtType* rttiType_)
    : rtType(rttiType_)
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
    return (nullptr != structure->dtor);
}

void ReflectedType::Destroy(Any&& any) const
{
    if (!any.GetRtType()->IsPointer())
    {
        any.Clear();
    }
    else if (nullptr != structure->dtor)
    {
        structure->dtor->Destroy(std::move(any));
    }
    else
    {
        DAVA_THROW(Exception, "There is no appropriate dtor to destroy such object");
    }
}
} // namespace DAVA
