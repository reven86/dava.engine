#include "TArc/Utils/ReflectionHelpers.h"
#include <Reflection/Reflection.h>
#include <Reflection/ReflectedTypeDB.h>

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

const DAVA::ReflectedType* GetValueReflectedType(const Reflection& r)
{
    const ReflectedType* type = GetValueReflectedType(r.GetValue());
    if (type != nullptr)
    {
        return type;
    }

    return r.GetValueObject().GetReflectedType();
}

const DAVA::ReflectedType* GetValueReflectedType(const Any& value)
{
    if (value.IsEmpty() == true)
    {
        return nullptr;
    }

    const Type* type = value.GetType();
    if (type->IsPointer())
    {
        void* pointerValue = value.Get<void*>();
        if (pointerValue != nullptr)
        {
            return ReflectedTypeDB::GetByPointer(pointerValue, type->Deref());
        }
    }

    return ReflectedTypeDB::GetByType(type);
}

} // namespace TArc
} // namespace DAVA
