#pragma once

#include <Base/Any.h>
#include <QSize>

namespace DAVA
{
template <>
struct AnyCompare<QSize>
{
    static bool IsEqual(const Any& v1, const Any& v2)
    {
        return v1.Get<QSize>() == v2.Get<QSize>();
    }
};
}
