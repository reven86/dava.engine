#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
namespace Private
{
Vector<String> GetCommandArgs(int argc, char* argv[]);

#if defined(__DAVAENGINE_WINDOWS__)
Vector<String> GetCommandArgs();
#endif

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
