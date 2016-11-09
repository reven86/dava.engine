#include "Base/RttiInheritance.h"

namespace DAVA
{
const RttiInheritance::Info* RttiInheritance::SearchInfo(const RttiType* from, const RttiType* to, RttiInheritance::Direction direction)
{
    if (from->IsPointer() && to->IsPointer())
    {
        const RttiType* to_ = to->Deref();
        const RttiType* from_ = from->Deref();
        const RttiInheritance* inheritance = from_->GetInheritance();
        if (nullptr != inheritance)
        {
            Vector<Info>* typesInfo = &inheritance->baseTypesInfo;

            if (direction == Direction::Up)
            {
                typesInfo = &inheritance->derivedTypesInfo;
            }

            for (Info& info : *typesInfo)
            {
                if (info.type == to_)
                {
                    return &info;
                }
            }

            for (Info& info : *typesInfo)
            {
                const RttiInheritance::Info* ret = SearchInfo(info.type->Pointer(), to, direction);
                if (nullptr != ret)
                {
                    return ret;
                }
            }
        }
    }

    return nullptr;
}

bool RttiInheritance::CanUpCast(const RttiType* from, const RttiType* to)
{
    return (nullptr != RttiInheritance::SearchInfo(from, to, Direction::Down));
}

bool RttiInheritance::CanDownCast(const RttiType* from, const RttiType* to)
{
    return (nullptr != RttiInheritance::SearchInfo(from, to, Direction::Up));
}

bool RttiInheritance::CanCast(const RttiType* from, const RttiType* to)
{
    return (CanUpCast(from, to) || CanDownCast(from, to));
}

bool RttiInheritance::DownCast(const RttiType* from, void* inPtr, const RttiType* to, void** outPtr)
{
    bool ret = false;

    const Info* info = RttiInheritance::SearchInfo(from, to, Direction::Down);
    if (nullptr != info)
    {
        *outPtr = info->castOP(inPtr);
        ret = true;
    }

    return ret;
}

bool RttiInheritance::UpCast(const RttiType* from, void* inPtr, const RttiType* to, void** outPtr)
{
    bool ret = false;

    const Info* info = RttiInheritance::SearchInfo(from, to, Direction::Up);
    if (nullptr != info)
    {
        *outPtr = info->castOP(inPtr);
        ret = true;
    }

    return ret;
}

bool RttiInheritance::Cast(const RttiType* from, void* inPtr, const RttiType* to, void** outPtr)
{
    if (DownCast(from, inPtr, to, outPtr))
    {
        return true;
    }

    if (UpCast(from, inPtr, to, outPtr))
    {
        return true;
    }

    return false;
}

} // namespace DAVA
