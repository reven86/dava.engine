#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
enum class eEngineRunMode : int32
{
    GUI_STANDALONE = 0, // Run engine as standalone GUI application
    GUI_EMBEDDED, // Run engine inside other framework, e.g. Qt
    CONSOLE_MODE, // Run engine as standalone console application
};

#if defined(__DAVAENGINE_COREV2__)

enum class eMouseMode : int32
{
    OFF = 0, // Disable any capturing, show cursor (send absolute xy)
    FRAME, // Capture system cursor into window rect, show cursor (send absolute xy)
    PINING, // Capture system cursor on current position, and hide cursor (send xy move delta)
    HIDE // Disable any capturing, and hide cursor (send absolute xy)
};

#endif

} // namespace DAVA
