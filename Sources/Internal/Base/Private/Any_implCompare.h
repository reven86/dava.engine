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

template <typename K, typename V, typename Eq>
struct AnyCompare<Map<K, V, Eq>>
{
    static bool IsEqual(const Any& v1, const Any& v2)
    {
        using MapType = Map<K, V, Eq>;
        const MapType& m1 = v1.Get<MapType>();
        const MapType& m2 = v2.Get<MapType>();
        return m1 == m2;
    }
};

template <typename K, typename V, typename Hash, typename Eq>
struct AnyCompare<UnorderedMap<K, V, Hash, Eq>>
{
    static bool IsEqual(const Any& v1, const Any& v2)
    {
        using MapType = UnorderedMap<K, V, Hash, Eq>;
        const MapType& m1 = v1.Get<MapType>();
        const MapType& m2 = v2.Get<MapType>();
        return m1 == m2;
    }
};

} // namespace DAVA
