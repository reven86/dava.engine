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
bool AnyCompare<String>::IsEqual(const Any& v1, const Any& v2);
extern template struct AnyCompare<String>;

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

template <typename V, typename Eq>
struct AnyCompare<Set<V, Eq>>
{
    static bool IsEqual(const Any& v1, const Any& v2)
    {
        using SetType = Set<V, Eq>;
        const SetType& s1 = v1.Get<SetType>();
        const SetType& s2 = v2.Get<SetType>();
        return s1 == s2;
    }
};

template <typename V>
struct AnyCompare<Vector<V>>
{
    static bool IsEqual(const Any& v1, const Any& v2)
    {
        using VectorType = Vector<V>;
        const VectorType& vv1 = v1.Get<VectorType>();
        const VectorType& vv2 = v2.Get<VectorType>();
        return vv1 == vv2;
    }
};

} // namespace DAVA
