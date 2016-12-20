#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)

namespace DAVA
{
namespace Private
{
Vector<String> GetCommandArgs(int argc, char* argv[]);
Vector<String> GetCommandArgs(const String& cmdline);

#if defined(__DAVAENGINE_WINDOWS__)
Vector<String> GetCommandArgs();
#endif

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
