#include "Base/Any.h"

namespace DAVA
{
bool Any::LoadValue(void* data, const RttiType* type_)
{
    rttiType = type_;

    if (rttiType->IsPointer())
    {
        void** src = reinterpret_cast<void**>(data);
        anyStorage.SetAuto(*src);
        return true;
    }
    else if (rttiType->IsTrivial())
    {
        anyStorage.SetData(data, rttiType->GetSize());
        return true;
    }

    return false;
}

bool Any::StoreValue(void* data, size_t size) const
{
    if (nullptr != rttiType && size >= rttiType->GetSize())
    {
        if (rttiType->IsPointer())
        {
            void** dst = reinterpret_cast<void**>(data);
            *dst = anyStorage.GetAuto<void*>();
            return true;
        }
        else if (rttiType->IsTrivial())
        {
            std::memcpy(data, anyStorage.GetData(), size);
            return true;
        }
    }

    return false;
}

bool Any::operator==(const Any& any) const
{
    if (rttiType == nullptr && any.rttiType == nullptr)
    {
        return true;
    }
    else if (rttiType == nullptr || any.rttiType == nullptr)
    {
        return false;
    }

    if (any.rttiType->IsPointer())
    {
        if (rttiType->IsPointer())
        {
            return anyStorage.GetSimple<void*>() == any.anyStorage.GetSimple<void*>();
        }
        else
        {
            return false;
        }
    }

    if (rttiType != any.rttiType)
    {
        return false;
    }

    if (rttiType->IsTrivial())
    {
        return (0 == std::memcmp(anyStorage.GetData(), any.anyStorage.GetData(), rttiType->GetSize()));
    }

    return (*compareFn)(*this, any);
}
} // namespace DAVA
