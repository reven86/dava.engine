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
        return v1.Get<QString>() == v2.Get<QString>();
    }
};
}
