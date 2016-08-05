#pragma once

namespace DAVA
{
enum class eEngineRunMode : int32
{
    GUI_STANDALONE = 0, // Run engine as standalone GUI application
    GUI_EMBEDDED, // Run engine inside other framework, e.g. Qt
    CONSOLE, // Run engine as standalone console application
};

} // namespace DAVA
