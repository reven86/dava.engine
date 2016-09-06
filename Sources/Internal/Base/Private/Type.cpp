#pragma once

#include "Base/Type.h"

namespace DAVA
{
bool TypeInheritance::CanDownCast(const Type* from, const Type* to)
{
    if (from->IsPointer() && to->IsPointer())
    {
        to = to->Deref();
        from = from->Deref();

        const TypeInheritance* fti = from->inheritance.get();
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

bool TypeInheritance::CanUpCast(const Type* from, const Type* to)
{
    if (from->IsPointer() && to->IsPointer())
    {
        to = to->Deref();
        from = from->Deref();

        const TypeInheritance* fti = from->inheritance.get();
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

bool TypeInheritance::CanCast(const Type* from, const Type* to)
{
    return CanDownCast(from, to) || CanUpCast(from, to);
}

bool TypeInheritance::DownCast(const Type* from, void* inPtr, const Type* to, void** outPtr)
{
    if (from->IsPointer() && to->IsPointer())
    {
        to = to->Deref();
        from = from->Deref();

        const TypeInheritance* fti = from->inheritance.get();
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

bool TypeInheritance::UpCast(const Type* from, void* inPtr, const Type* to, void** outPtr)
{
    if (from->IsPointer() && to->IsPointer())
    {
        to = to->Deref();
        from = from->Deref();

        const TypeInheritance* fti = from->inheritance.get();
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

bool TypeInheritance::Cast(const Type* from, void* inPtr, const Type* to, void** outPtr)
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
