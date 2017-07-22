#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Dispatcher.h"
#include "Engine/Private/Dispatcher/UIDispatcherEvent.h"

namespace DAVA
{
namespace Private
{
using UIDispatcher = Dispatcher<UIDispatcherEvent>;

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
