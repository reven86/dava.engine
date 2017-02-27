#pragma once

#include "Engine/Private/Dispatcher/Dispatcher.h"

namespace DAVA
{
namespace Net
{
using NetEventsDispatcher = Dispatcher<Function<void()>>;
}
}
