#pragma once
#include "Reflection/Wrappers.h"

namespace DAVA
{
template <typename C>
class DtorWrapperByPointer : public DtorWrapper
{
public:
    DtorWrapperByPointer()
    {
        destroyer = [](C* c) { delete c; };
    }

    DtorWrapperByPointer(void (*destroyer_)(C*))
        : destroyer(destroyer_)
    {
    }

    void Destroy(ReflectedObject&& object) const override
    {
        if (object.IsValid())
        {
            C* c = object.GetPtr<C>();
            (*destroyer)(c);

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

protected:
    void (*destroyer)(C*);
};

} // namespace DAVA
