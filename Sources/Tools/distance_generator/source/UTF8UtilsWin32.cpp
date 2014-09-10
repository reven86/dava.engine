#include "UTF8Utils.h"
#include <Windows.h>

using namespace std;

void  UTF8Utils::EncodeToWideString(const char* str, int size, std::wstring& resultString)
{
	resultString = L"";

	int wstringLen = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)str, size, NULL, NULL);
	if (!wstringLen)
	{
		return;
	}

	wchar_t* buf = new wchar_t[wstringLen];
	int convertRes = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)str, size, buf, wstringLen);
	if (convertRes)
	{
		resultString = wstring(buf, wstringLen);
	}

	delete[] buf;
};

string UTF8Utils::EncodeToUTF8(const std::wstring& wstr)
{
	int bufSize = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, 0, 0, NULL, NULL);
	if (!bufSize)
	{
		return "";
	}

	string resStr = "";

	char* buf = new char[bufSize];
	int res = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, buf, bufSize, NULL, NULL);
	if (res)
	{
		resStr = string(buf);
	}

	delete[] buf;
	return resStr;
};
