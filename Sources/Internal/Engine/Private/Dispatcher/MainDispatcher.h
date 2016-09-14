#if defined(__DAVAENGINE_COREV2__)

#pragma once

#include "Base/BaseTypes.h"
#include "Engine/Dispatcher.h"
#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"

namespace DAVA
{
namespace Private
{
using MainDispatcher = Dispatcher<MainDispatcherEvent>;

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
