#pragma once
#include "Reflection/ReflectionWrappers.h"

namespace DAVA
{
template <typename C>
class DtorWrapperDefault : public DtorWrapper
{
public:
    void Destroy(ReflectedObject&& object) const override
    {
        if (object.IsValid())
        {
            C* c = object.GetPtr<C>();
            delete c;

            object = ReflectedObject();
        }
    }

    void Destroy(Any&& object) const override
    {
        if (!object.IsEmpty())
        {
            C* c = object.Get<C*>();
            delete c;

            object.Clear();
        }
    }
};

} // namespace DAVA
