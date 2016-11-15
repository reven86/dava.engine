#include "Base/Any.h"

namespace DAVA
{
bool Any::LoadValue(void* data, const RtType* type_)
{
    rtType = type_;

    if (rtType->IsPointer())
    {
        void** src = reinterpret_cast<void**>(data);
        anyStorage.SetAuto(*src);
        return true;
    }
    else if (rtType->IsTrivial())
    {
        anyStorage.SetData(data, rtType->GetSize());
        return true;
    }

    return false;
}

bool Any::StoreValue(void* data, size_t size) const
{
    if (nullptr != rtType && size >= rtType->GetSize())
    {
        if (rtType->IsPointer())
        {
            void** dst = reinterpret_cast<void**>(data);
            *dst = anyStorage.GetAuto<void*>();
            return true;
        }
        else if (rtType->IsTrivial())
        {
            std::memcpy(data, anyStorage.GetData(), size);
            return true;
        }
    }

    return false;
}

bool Any::operator==(const Any& any) const
{
    if (rtType == nullptr && any.rtType == nullptr)
    {
        return true;
    }
    else if (rtType == nullptr || any.rtType == nullptr)
    {
        return false;
    }

    if (any.rtType->IsPointer())
    {
        if (rtType->IsPointer())
        {
            return anyStorage.GetSimple<void*>() == any.anyStorage.GetSimple<void*>();
        }
        else
        {
            return false;
        }
    }

    if (rtType != any.rtType)
    {
        return false;
    }

    if (rtType->IsTrivial())
    {
        return (0 == std::memcmp(anyStorage.GetData(), any.anyStorage.GetData(), rtType->GetSize()));
    }

    return (*compareFn)(*this, any);
}
} // namespace DAVA
