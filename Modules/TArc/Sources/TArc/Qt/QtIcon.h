#pragma once

#include <Base/Any.h>
#include <QIcon>
#include <QList>
#include <QSize>

namespace DAVA
{
template <>
struct AnyCompare<QIcon>
{
    static bool IsEqual(const Any& v1, const Any& v2)
    {
        QIcon i1 = v1.Get<QIcon>();
        QIcon i2 = v2.Get<QIcon>();

        QList<QSize> s1 = i1.availableSizes();
        QList<QSize> s2 = i2.availableSizes();

        if (s1 != s2)
            return false;

        if (s1.size() == 0)
            return true;

        QPixmap p1 = i1.pixmap(s1.at(0));
        QPixmap p2 = i2.pixmap(s2.at(0));

        return (p1.toImage() == p2.toImage());
    }
};
}
