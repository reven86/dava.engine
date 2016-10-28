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

/**
    \ingroup engine
    Windows fullscreen switching
*/
enum class Fullscreen
{
    On = 0, //<! True full screen mode
    Off, //<! Windowed mode
};

} // namespace DAVA
