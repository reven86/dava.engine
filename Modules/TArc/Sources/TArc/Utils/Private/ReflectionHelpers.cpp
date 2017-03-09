#include "TArc/Utils/ReflectionHelpers.h"
#include <Reflection/Reflection.h>

#include <algorithm>

namespace DAVA
{
namespace TArc
{
void ForEachField(const Reflection& r, const Function<void(Reflection::Field&& field)>& fn)
{
    Vector<Reflection::Field> fields = r.GetFields();
    for (Reflection::Field& f : fields)
    {
        fn(std::move(f));
    }
}

const DAVA::ReflectedType* GetReflectedType(const Reflection& r)
{
    const ReflectedType* type = GetReflectedType(r.GetValue());
    if (type != nullptr)
    {
        return type;
    }

    return r.GetValueObject().GetReflectedType();
}

const DAVA::ReflectedType* GetReflectedType(const Any& value)
{
    if (value.CanCast<ReflectionBase*>())
    {
        return value.Cast<ReflectionBase*>()->Dava__GetReflectedType();
    }

    return nullptr;
}

} // namespace TArc
} // namespace DAVA