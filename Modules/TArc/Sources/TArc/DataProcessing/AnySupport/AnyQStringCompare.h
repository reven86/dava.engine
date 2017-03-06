#pragma once

#include <Base/Any.h>
#include <QString>

namespace DAVA
{
template <>
bool AnyCompare<QString>::IsEqual(const Any& v1, const Any& v2);
extern template struct AnyCompare<QString>;
}
