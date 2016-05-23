#ifndef __LOGGER_ERROR_HANDLER_H__
#define __LOGGER_ERROR_HANDLER_H__

#include "Base/BaseTypes.h"
#include "Logger/Logger.h"

class LoggerErrorHandler : public DAVA::LoggerOutput
{
public:
    ~LoggerErrorHandler() override;

    void Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text) override;

    bool HasErrors() const;
    const DAVA::Set<DAVA::String>& GetErrors() const;

protected:
    DAVA::Set<DAVA::String> errors;
};

inline bool LoggerErrorHandler::HasErrors() const
{
    return !errors.empty();
}

inline const DAVA::Set<DAVA::String>& LoggerErrorHandler::GetErrors() const
{
    return errors;
}


#endif // __LOGGER_ERROR_HANDLER_H__
