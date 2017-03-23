#include "Logger/Logger.h"
#include "Base/Platform.h"

#if defined(__DAVAENGINE_WINDOWS__)
namespace DAVA
{
void Logger::PlatformLog(eLogLevel ll, const char8* text)
{
    ::OutputDebugStringA(text);

#if defined(__DAVAENGINE_WIN_UAP__)
    {
        static Windows::Foundation::Diagnostics::LoggingChannel ^ lc = ref new Windows::Foundation::Diagnostics::LoggingChannel("DAVALogProvider", nullptr);
        Windows::Foundation::Diagnostics::LoggingLevel lv;

        switch (ll)
        {
        case DAVA::Logger::LEVEL_FRAMEWORK:
            lv = Windows::Foundation::Diagnostics::LoggingLevel::Verbose;
            break;
        case DAVA::Logger::LEVEL_DEBUG:
            lv = Windows::Foundation::Diagnostics::LoggingLevel::Verbose;
            break;
        case DAVA::Logger::LEVEL_INFO:
            lv = Windows::Foundation::Diagnostics::LoggingLevel::Information;
            break;
        case DAVA::Logger::LEVEL_WARNING:
            lv = Windows::Foundation::Diagnostics::LoggingLevel::Warning;
            break;
        case DAVA::Logger::LEVEL_ERROR:
            lv = Windows::Foundation::Diagnostics::LoggingLevel::Error;
            break;

        case DAVA::Logger::LEVEL__DISABLE:
        default:
            return;
        }

        WideString wtext = UTF8Utils::EncodeToWideString(text);
        lc->LogMessage(ref new Platform::String(wtext.c_str()), lv);
    }
#endif
}
}
#endif
