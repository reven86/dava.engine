#include "Utils/UTF8Utils.h"
#include "Debug/DVAssert.h"

#include <utf8.h>

namespace DAVA
{

#ifdef __DAVAENGINE_WINDOWS__

static_assert(sizeof(char16) == 2, "check size of wchar_t on current platform");

void UTF8Utils::EncodeToWideString(const uint8* string, size_t size, WideString& result)
{
    DVASSERT(nullptr != string);
    result.clear();
    result.reserve(size); // minimum they will be same
    utf8::utf8to16(string, string + size, std::back_inserter(result));
};

String UTF8Utils::EncodeToUTF8(const WideString& wstring)
{
    String result;
    result.reserve(wstring.size()); // minimum they will be same
    utf8::utf16to8(wstring.begin(), wstring.end(), std::back_inserter(result));
    return result;
};

#else

static_assert(sizeof(char16) == 4, "check size of wchar_t on current platform");

void UTF8Utils::EncodeToWideString(const uint8* string, size_type size, WideString& result)
{
    DVASSERT(nullptr != string);
    result.clear();
    result.reserve(size); // minimum they will be same
    utf8::utf8to32(string, string + size, std::back_inserter(result));
};

String UTF8Utils::EncodeToUTF8(const WideString& wstring)
{
    String result;
    result.reserve(wstring.size()); // minimum they will be same
    utf8::utf32to8(wstring.begin(), wstring.end(), std::back_inserter(result));
    return result;
};
#endif

} // namespace DAVA
