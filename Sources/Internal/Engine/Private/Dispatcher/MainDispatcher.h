#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)

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
