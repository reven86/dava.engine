#pragma once
#ifndef __Dava_Any__
#include "Base/Any.h"
#endif

#include "Math/Color.h"
#include "Math/Vector.h"

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

template <>
struct AnyCompare<Vector2>
{
    static bool IsEqual(const Any& v1, const Any& v2)
    {
        const Vector2& vec1 = v1.Get<Vector2>();
        const Vector2& vec2 = v2.Get<Vector2>();
        return vec1 == vec2;
    }
};

template <>
struct AnyCompare<Color>
{
    static bool IsEqual(const Any& v1, const Any& v2)
    {
        const Color& c1 = v1.Get<Color>();
        const Color& c2 = v2.Get<Color>();
        return c1 == c2;
    }
};

} // namespace DAVA
