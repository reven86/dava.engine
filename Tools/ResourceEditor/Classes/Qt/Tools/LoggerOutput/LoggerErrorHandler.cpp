#include "Tools/LoggerOutput/LoggerErrorHandler.h"

LoggerErrorHandler::~LoggerErrorHandler() = default;

void LoggerErrorHandler::Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text)
{
    if (ll < DAVA::Logger::LEVEL_ERROR)
        return;

    const size_t strLength = strlen(text);
    if (strLength > 0)
    {
        errors.emplace(text, strLength - 1); //remove extra '/n' that Logger was placed into end of text
    }
}
