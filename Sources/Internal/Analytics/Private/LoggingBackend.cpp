#include "Analytics/LoggingBackend.h"
#include "Logger/Logger.h"

namespace DAVA
{
namespace Analytics
{
String FormatDateTime(const DateTime& dt)
{
    int32 year = dt.GetYear();
    int32 month = dt.GetMonth();
    int32 day = dt.GetDay();
    int32 hour = dt.GetHour();
    int32 minute = dt.GetMinute();
    int32 sec = dt.GetSecond();

    return Format("%04d-%02d-%02d %02d:%02d:%02d%", year, month, day, hour, minute, sec);
}

LoggingBackend::LoggingBackend(const FilePath& path)
    : filePath(path)
{
}

void LoggingBackend::ConfigChanged(const KeyedArchive& config)
{
}

void LoggingBackend::ProcessEvent(const EventRecord& event)
{
    String value;
    String msg;

    for (const auto& field : event.fields)
    {
        if (field.second.CanCast<String>())
        {
            value = field.second.Cast<String>();
        }
        else if (field.second.CanGet<const char*>())
        {
            value = field.second.Get<const char*>();
        }
        else if (field.second.CanGet<const DateTime&>())
        {
            const DateTime& dateTime = field.second.Get<const DateTime&>();
            value = FormatDateTime(dateTime);
        }

        if (!msg.empty())
        {
            msg += ", ";
        }
        msg += field.first + ": " + value;
    }

    if (filePath.IsEmpty())
    {
        Logger::Info(msg.c_str());
    }
    else
    {
        Logger::InfoToFile(filePath, msg.c_str());
    }
}

} // namespace Analytics
} // namespace DAVA
