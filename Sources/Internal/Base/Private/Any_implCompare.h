#pragma once
#ifndef __Dava_Any__
#include "Base/Any.h"
#endif

namespace DAVA
{
template <typename T>
bool AnyCompare<T>::IsEqual(const Any&, const Any&)
{
    DAVA_THROW(Exception, "Any:: can't be equal compared");
    return false;
}

template <>
struct AnyCompare<String>
{
    static bool IsEqual(const Any& v1, const Any& v2)
    {
        const String& s1 = v1.Get<String>();
        const String& s2 = v2.Get<String>();
        return s1 == s2;
    }
};

} // namespace DAVA
