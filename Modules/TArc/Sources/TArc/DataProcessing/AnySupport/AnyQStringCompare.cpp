#include "TArc/DataProcessing/AnySupport/AnyQStringCompare.h"

namespace DAVA
{
template <>
bool AnyCompare<QString>::IsEqual(const Any& v1, const Any& v2)
{
    return v1.Get<QString>() == v2.Get<QString>();
}
}
