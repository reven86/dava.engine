#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"
#include "Engine/Private/Dispatcher/DispatcherT.h"
#include "Engine/Private/Dispatcher/PlatformEvent.h"

namespace DAVA
{
namespace Private
{
using PlatformDispatcher = DispatcherT<PlatformEvent>;

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
