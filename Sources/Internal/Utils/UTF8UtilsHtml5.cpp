#include "Utils/UTF8Utils.h"
#include "FileSystem/Logger.h"

#if defined(__DAVAENGINE_HTML5__)
#include <locale>
#include <codecvt>

namespace DAVA
{

void UTF8Utils::EncodeToWideString(uint8 * string, int32 size, WideString & resultString)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv;
    resultString = conv.from_bytes((const char*)string);
}

String UTF8Utils::EncodeToUTF8(const WideString& wstring)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t> > conv;
	return conv.to_bytes(wstring);
}

};
#endif