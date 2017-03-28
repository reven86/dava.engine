#pragma once

#include "Base/BaseTypes.h"
#include "Base/Any.h"

namespace DAVA
{
class AnyConverter
{
public:
    static String ToString(const Any& any);
};
}
