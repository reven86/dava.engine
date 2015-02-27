/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "Utils/UTF8Utils.h"
#include "Utils/Utils.h"
#include "FileSystem/Logger.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "iconv/iconv.h"
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"

namespace DAVA 
{

void UTF8Utils::EncodeToWideString(const uint8 * string, size_t size, WideString & resultString)
{
	resultString = L"";

	iconv_t cd = iconv_open("UTF-32LE", "UTF-8");
	if (cd == (iconv_t)(-1))
	{
		Logger::Debug("Error converting");
		return;
	}

	size_t inSize = (size_t)size;
	wchar_t* outBuf = new wchar_t[inSize + 1];
	size_t outSize = (inSize + 1) * sizeof(wchar_t);
	memset(outBuf, 0, outSize);

	char* buf = (char*)outBuf;
	errno = 0;
	size_t k = iconv(cd, (char**)&string, &inSize, (char**)&buf, &outSize);
	int err = errno;
	resultString = outBuf;

	delete [] outBuf;
	//Logger::Debug("Converted: %u,error=%d\n", (unsigned) k, err );

	iconv_close(cd);
};

String UTF8Utils::EncodeToUTF8(const WideString& wstring)
{
	String resultString = "";

	iconv_t cd = iconv_open("UTF-8", "UTF-32LE");
	if (cd == (iconv_t)(-1))
	{
		Logger::Debug("Error converting");
		return resultString;
	}

	WideString inString = wstring;
	char* inBuf = (char*)inString.c_str();
	size_t inSize = inString.length() * sizeof(wchar_t);
	size_t outSize = (inString.length() + 1) * sizeof(wchar_t);
	char* outString = new char[outSize];
	char* outBuf = outString;
	memset(outBuf, 0, outSize);
	iconv (cd, (char **) &inBuf, &inSize, (char**)&outBuf, &outSize);

	resultString = outString;
	delete [] outString;
	iconv_close(cd);

	return resultString;
};

};

#endif
