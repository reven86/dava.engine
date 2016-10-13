#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
/**
    \ingroup engine
    Engine run modes
*/
enum class eEngineRunMode : int32
{
    GUI_STANDALONE = 0, //!< Run engine as standalone GUI application
    GUI_EMBEDDED, //!< Run engine inside other framework, e.g. Qt
    CONSOLE_MODE //!< Run engine as standalone console application
};

/** Enum class representing the management cursor, using SetCursorCapture(mode) */
enum class eCursorCapture : int32
{
    OFF = 0, //!< Disable any capturing(send absolute xy)
    FRAME, //!< Capture system cursor into window rect(send absolute xy) */
    PINNING, //!< Capture system cursor on current position(send xy move delta) */
};

} // namespace DAVA
