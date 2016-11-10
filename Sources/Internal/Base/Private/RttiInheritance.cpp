#include "Base/RttiInheritance.h"

namespace DAVA
{
bool RttiInheritance::TryCast(const RttiType* from, const RttiType* to, CastType castType, void* inPtr, void** outPtr)
{
    if (from->IsPointer() && to->IsPointer())
    {
        const RttiType* to_ = to->Deref()->Decay();
        const RttiType* from_ = from->Deref()->Decay();

        if (to_ == from_)
        {
            *outPtr = inPtr;
            return true;
        }

        const RttiInheritance* inheritance = from_->GetInheritance();
        if (nullptr != inheritance)
        {
            Vector<Info>* typesInfo = &inheritance->baseTypesInfo;

            if (castType == CastType::UpCast)
            {
                typesInfo = &inheritance->derivedTypesInfo;
            }

            for (Info& info : *typesInfo)
            {
                if (info.type == to_)
                {
                    *outPtr = (*info.castOP)(inPtr);
                    return true;
                }
            }

            for (Info& info : *typesInfo)
            {
                return TryCast(info.type->Pointer(), to, castType, (*info.castOP)(inPtr), outPtr);
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
