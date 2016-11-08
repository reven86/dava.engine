#pragma once

#include "Base/BaseTypes.h"
#include "Math/Rect.h"

namespace DAVA
{
struct DisplayInfo
{
    uintptr_t systemId = 0;
    Rect rect;
    float32 dpiX = 0.f;
    float32 dpiY = 0.f;
    String name;
};

} // namespace DAVA
