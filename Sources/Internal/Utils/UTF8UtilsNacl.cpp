#include "Utils/UTF8Utils.h"
#include "FileSystem/Logger.h"

#if defined(__DAVAENGINE_NACL__)
#include <stdlib.h>
namespace DAVA
{
void UTF8Utils::EncodeToWideString(const uint8 * string, int32 size, WideString & resultString)
{
	resultString = L"";
	
	size_t inSize = (size_t)size;
	wchar_t* outBuf = new wchar_t[inSize + 1];
	size_t outSize = (inSize + 1) * sizeof(wchar_t);
	memset(outBuf, 0, outSize);
	
	mbstowcs(outBuf, (char*)string, inSize + 1);
	
	resultString = outBuf;
	delete [] outBuf;
}

String UTF8Utils::EncodeToUTF8(const WideString& wstring)
{
	String resultString = "";
	
	WideString inString = wstring;
	char* inBuf = (char*)inString.c_str();
	size_t inSize = inString.length() * sizeof(wchar_t);
	size_t outSize = (inString.length() + 1) * sizeof(wchar_t);
	char* outString = new char[outSize];
	char* outBuf = outString;
	memset(outBuf, 0, outSize);
	
	wcstombs(outBuf, inString.c_str(), inString.length()+1);
	
	resultString = outString;
	delete[] outString;
	
	return resultString;
}

};
#endif