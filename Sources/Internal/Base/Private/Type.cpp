#pragma once

#include "Base/Type.h"

namespace DAVA
{
bool TypePtrCast::CanDownCast(const Type* from, const Type* to)
{
    if (from->IsPointer() && to->IsPointer())
    {
        to = to->Deref();
        from = from->Deref();

        auto it = from->BaseTypes().find(to);
        if (it != from->BaseTypes().end())
        {
            return true;
        }
        else
        {
            for (auto& base : from->BaseTypes())
            {
                if (CanDownCast(base.first->Pointer(), to->Pointer()))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool TypePtrCast::CanUpCast(const Type* from, const Type* to)
{
    if (from->IsPointer() && to->IsPointer())
    {
        to = to->Deref();
        from = from->Deref();

        auto it = from->DerivedTypes().find(to);
        if (it != from->DerivedTypes().end())
        {
            return true;
        }
        else
        {
            for (auto& derived : from->DerivedTypes())
            {
                if (CanUpCast(derived.first->Pointer(), to->Pointer()))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool TypePtrCast::CanCast(const Type* from, const Type* to)
{
    return CanDownCast(from, to) || CanUpCast(from, to);
}

bool TypePtrCast::DownCast(const Type* from, void* inPtr, const Type* to, void** outPtr)
{
    if (from->IsPointer() && to->IsPointer())
    {
        to = to->Deref();
        from = from->Deref();

        auto it = from->BaseTypes().find(to);
        if (it != from->BaseTypes().end())
        {
            *outPtr = it->second(inPtr);
            return true;
        }
        else
        {
            for (auto& base : from->BaseTypes())
            {
                void* baseInPtr = base.second(inPtr);
                if (DownCast(base.first->Pointer(), baseInPtr, to->Pointer(), outPtr))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool TypePtrCast::UpCast(const Type* from, void* inPtr, const Type* to, void** outPtr)
{
    if (from->IsPointer() && to->IsPointer())
    {
        to = to->Deref();
        from = from->Deref();

        auto it = from->DerivedTypes().find(to);
        if (it != from->DerivedTypes().end())
        {
            *outPtr = it->second(inPtr);
            return true;
        }
        else
        {
            for (auto& derived : from->DerivedTypes())
            {
                void* derivedInPtr = derived.second(inPtr);
                if (UpCast(derived.first->Pointer(), derivedInPtr, to->Pointer(), outPtr))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool TypePtrCast::Cast(const Type* from, void* inPtr, const Type* to, void** outPtr)
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
