#include "Base/RttiInheritance.h"

namespace DAVA
{
bool RttiInheritance::CanDownCast(const RttiType* from, const RttiType* to)
{
    if (from->IsPointer() && to->IsPointer())
    {
        to = to->Deref();
        from = from->Deref();

        const RttiInheritance* fti = from->inheritance.get();
        if (nullptr != fti)
        {
            auto it = fti->GetBaseTypes().find(to);
            if (it != fti->GetBaseTypes().end())
            {
                return true;
            }
            else
            {
                for (auto& base : fti->GetBaseTypes())
                {
                    if (CanDownCast(base.first->Pointer(), to->Pointer()))
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool RttiInheritance::CanUpCast(const RttiType* from, const RttiType* to)
{
    if (from->IsPointer() && to->IsPointer())
    {
        to = to->Deref();
        from = from->Deref();

        const RttiInheritance* fti = from->inheritance.get();
        if (nullptr != fti)
        {
            auto it = fti->GetDerivedTypes().find(to);
            if (it != fti->GetDerivedTypes().end())
            {
                return true;
            }
            else
            {
                for (auto& derived : fti->GetDerivedTypes())
                {
                    if (CanUpCast(derived.first->Pointer(), to->Pointer()))
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool RttiInheritance::CanCast(const RttiType* from, const RttiType* to)
{
    return CanDownCast(from, to) || CanUpCast(from, to);
}

bool RttiInheritance::DownCast(const RttiType* from, void* inPtr, const RttiType* to, void** outPtr)
{
    if (from->IsPointer() && to->IsPointer())
    {
        to = to->Deref();
        from = from->Deref();

        const RttiInheritance* fti = from->inheritance.get();
        if (nullptr != fti)
        {
            auto it = fti->GetBaseTypes().find(to);
            if (it != fti->GetBaseTypes().end())
            {
                *outPtr = it->second(inPtr);
                return true;
            }
            else
            {
                for (auto& base : fti->GetBaseTypes())
                {
                    void* baseInPtr = base.second(inPtr);
                    if (DownCast(base.first->Pointer(), baseInPtr, to->Pointer(), outPtr))
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool RttiInheritance::UpCast(const RttiType* from, void* inPtr, const RttiType* to, void** outPtr)
{
    if (from->IsPointer() && to->IsPointer())
    {
        to = to->Deref();
        from = from->Deref();

        const RttiInheritance* fti = from->inheritance.get();
        if (nullptr != fti)
        {
            auto it = fti->GetDerivedTypes().find(to);
            if (it != fti->GetDerivedTypes().end())
            {
                *outPtr = it->second(inPtr);
                return true;
            }
            else
            {
                for (auto& derived : fti->GetDerivedTypes())
                {
                    void* derivedInPtr = derived.second(inPtr);
                    if (UpCast(derived.first->Pointer(), derivedInPtr, to->Pointer(), outPtr))
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
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
