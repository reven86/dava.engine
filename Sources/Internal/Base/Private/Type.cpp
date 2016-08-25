#pragma once

#include "Base/Type.h"

namespace DAVA
{
bool TypeCast::CanDownCast(const Type* from, const Type* to)
{
    auto it = from->BaseTypes().find(to);
    if (it != from->BaseTypes().end())
    {
        return true;
    }
    else
    {
        for (auto& base : from->BaseTypes())
        {
            if (CanDownCast(base.first, to))
            {
                return true;
            }
        }
    }

    return false;
}

bool TypeCast::CanUpCast(const Type* from, const Type* to)
{
    auto it = from->DerivedTypes().find(to);
    if (it != from->DerivedTypes().end())
    {
        return true;
    }
    else
    {
        for (auto& derived : from->DerivedTypes())
        {
            if (CanUpCast(derived.first, to))
            {
                return true;
            }
        }
    }

    return false;
}

bool TypeCast::CanCast(const Type* from, const Type* to)
{
    return CanDownCast(from, to) || CanUpCast(from, to);
}

bool TypeCast::DownCast(const Type* from, void* inPtr, const Type* to, void** outPtr)
{
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
            if (DownCast(base.first, baseInPtr, to, outPtr))
            {
                return true;
            }
        }
    }

    return false;
}

bool TypeCast::UpCast(const Type* from, void* inPtr, const Type* to, void** outPtr)
{
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
            if (UpCast(derived.first, derivedInPtr, to, outPtr))
            {
                return true;
            }
        }
    }

    return false;
}

bool TypeCast::Cast(const Type* from, void* inPtr, const Type* to, void** outPtr)
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
