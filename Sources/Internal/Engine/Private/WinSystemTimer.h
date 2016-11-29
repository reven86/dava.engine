#pragma once

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WINDOWS__)
namespace DAVA
{
namespace Private
{
void EnableHighResolutionTimer(bool enable);
}
}
#endif
