#include "Analytics/LoggingBackend.h"
#include "Logger/Logger.h"

namespace DAVA
{
namespace Analytics
{
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
