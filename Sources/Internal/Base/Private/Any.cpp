#include "Base/Any.h"

namespace DAVA
{
void Any::LoadValue(const Type* type_, void* data)
{
    type = type_;

    if (type_->IsPointer())
    {
        void** src = reinterpret_cast<void**>(data);
        anyStorage.SetAuto(*src);
    }
    else if (type_->IsTriviallyCopyable())
    {
        anyStorage.SetData(data, type_->GetSize());
    }
    else
    {
        DAVA_THROW(Exception, "Any:: can't be loaded from not trivial type");
    }
}

void Any::StoreValue(void* data, size_t size) const
{
    if (nullptr != type)
    {
        if (type->GetSize() > size)
        {
            DAVA_THROW(Exception, "Any:: can't be stored, size mismatch");
        }

        if (type->IsPointer())
        {
            void** dst = reinterpret_cast<void**>(data);
            *dst = anyStorage.GetAuto<void*>();
        }
        else if (type->IsTriviallyCopyable())
        {
            std::memcpy(data, anyStorage.GetData(), size);
        }
        else
        {
            DAVA_THROW(Exception, "Any:: can't be stored into not trivial type");
        }
    }
}

bool Any::operator==(const Any& any) const
{
    if (type == nullptr && any.type == nullptr)
    {
        return true;
    }
    else if (type == nullptr || any.type == nullptr)
    {
        return false;
    }

    if (any.type->IsPointer())
    {
        if (type->IsPointer())
        {
            return anyStorage.GetSimple<void*>() == any.anyStorage.GetSimple<void*>();
        }
        else
        {
            return false;
        }
    }

    if (type != any.type)
    {
        return false;
    }

    if (type->IsTriviallyCopyable())
    {
        return (0 == std::memcmp(anyStorage.GetData(), any.anyStorage.GetData(), type->GetSize()));
    }

    return (*compareFn)(*this, any);
}

} // namespace DAVA
