#ifndef __DAVAENGINE_UTF8UTILS_H__
#define __DAVAENGINE_UTF8UTILS_H__

#include <string>

class UTF8Utils
{
public:
	static void EncodeToWideString(const char* str, int size, std::wstring& resultString);
	static std::string EncodeToUTF8(const std::wstring& wstr);
};

#endif // __DAVAENGINE_UTF8UTILS_H__