#include "Base/Exception.h"
#include "Debug/DVAssert.h"
#include "Logger/Logger.h"
#include "Utils/UTF8Utils.h"

#include <utf8.h>

namespace DAVA
{

#ifdef __DAVAENGINE_WINDOWS__
static_assert(sizeof(char16) == 2, "check size of wchar_t on current platform");
#else
static_assert(sizeof(char16) == 4, "check size of wchar_t on current platform");
#endif

void UTF8Utils::EncodeToWideString(const uint8* string, size_t size, WideString& result)
{
    DVASSERT(nullptr != string);
    result.clear();
    result.reserve(size); // minimum they will be same

    try
    {
#ifdef __DAVAENGINE_WINDOWS__
        utf8::utf8to16(string, string + size, std::back_inserter(result));
#else
        utf8::utf8to32(string, string + size, std::back_inserter(result));
#endif
    }
    catch (const utf8::exception& exception)
    {
        String msg = "UTF8->WideString Conversion error: " + String(exception.what());
        Logger::Error(msg.c_str());
        DAVA_THROW(DAVA::Exception, msg);
    }
};

String UTF8Utils::EncodeToUTF8(const WideString& wstring)
{
    String result;
    result.reserve(wstring.size()); // minimum they will be same

    try
    {
#ifdef __DAVAENGINE_WINDOWS__
        utf8::utf16to8(wstring.begin(), wstring.end(), std::back_inserter(result));
#else
        utf8::utf32to8(wstring.begin(), wstring.end(), std::back_inserter(result));
#endif
    }
    catch (const utf8::exception& exception)
    {
        String msg = "WideString->UTF8 Conversion error: " + String(exception.what());
        DAVA::Logger::Error(msg.c_str());
        DAVA_THROW(DAVA::Exception, msg);
    }

    return result;
};

} // namespace DAVA
