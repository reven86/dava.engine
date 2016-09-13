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

enum class eCaptureMode : int32
{
    OFF = 0, // Disable any capturing (send absolute xy)
    FRAME, // Capture system cursor into window rect (send absolute xy)
    PINING // Capture system cursor on current position (send xy move delta)
};

#endif

} // namespace DAVA
