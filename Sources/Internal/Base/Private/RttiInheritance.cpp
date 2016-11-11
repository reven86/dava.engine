#include "Base/RttiInheritance.h"

namespace DAVA
{
bool RttiInheritance::TryCast(const RttiType* from, const RttiType* to, CastType castType, void* inPtr, void** outPtr)
{
    if (to == from)
    {
        *outPtr = inPtr;
        return true;
    }

    to = to->IsPointer() ? to->Deref()->Decay() : to->Decay();
    from = from->IsPointer() ? from->Deref()->Decay() : from->Decay();

    if (to == from)
    {
        *outPtr = inPtr;
        return true;
    }

    const RttiInheritance* inheritance = from->GetInheritance();
    if (nullptr != inheritance)
    {
        Vector<Info>* typesInfo = &inheritance->baseTypesInfo;

        if (castType == CastType::UpCast)
        {
            typesInfo = &inheritance->derivedTypesInfo;
        }

        for (Info& info : *typesInfo)
        {
            if (info.type == to)
            {
                *outPtr = OffsetPointer<void*>(inPtr, info.ptrDiff);
                return true;
            }
        }

        for (Info& info : *typesInfo)
        {
            if (TryCast(info.type, to, castType, OffsetPointer<void*>(inPtr, info.ptrDiff), outPtr))
            {
                return true;
            }
        }
    }

    return false;
}

bool RttiInheritance::CanCast(const RttiType* from, const RttiType* to)
{
    void* out = nullptr;
    return (TryCast(from, to, CastType::DownCast, nullptr, &out) || TryCast(from, to, CastType::UpCast, nullptr, &out));
}

bool RttiInheritance::Cast(const RttiType* from, const RttiType* to, void* inPtr, void** outPtr)
{
    if (TryCast(from, to, CastType::DownCast, inPtr, outPtr))
    {
        return true;
    }
    else if (TryCast(from, to, CastType::UpCast, inPtr, outPtr))
    {
        return true;
    }

    return false;
}

} // namespace DAVA
