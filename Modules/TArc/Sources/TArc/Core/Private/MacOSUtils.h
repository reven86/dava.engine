#pragma once

#include "Base/Platform.h"

namespace DAVA
{
namespace TArc
{
#if defined(__DAVAENGINE_MACOS__)
void MakeAppForeground();
void FixOSXFonts();
void RestoreMenuBar();
#endif
} // namespace TArc
} // namespace DAVA