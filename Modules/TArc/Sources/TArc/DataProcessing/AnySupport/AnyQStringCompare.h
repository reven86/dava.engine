#pragma once

#include <Base/Any.h>

#include <QString>

namespace DAVA
{
template <>
struct AnyCompare<QString>
{
    static bool IsEqual(const Any& v1, const Any& v2)
    {
        const QString& s1 = v1.Get<QString>();
        const QString& s2 = v2.Get<QString>();
        return s1 == s2;
    }
};
}
