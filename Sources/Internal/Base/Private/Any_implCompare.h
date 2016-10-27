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

} // namespace DAVA
