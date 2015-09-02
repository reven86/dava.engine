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
#include "Debug/DVAssert.h"

#include <utf8.h>

namespace DAVA
{

#ifdef __DAVAENGINE_WINDOWS__

static_assert(sizeof(wchar_t) == 2, "check size of wchar_t on current platform");

void UTF8Utils::EncodeToWideString(const uint8 * string, size_t size, WideString & result)
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

static_assert(sizeof(wchar_t) == 4, "check size of wchar_t on current platform");

void UTF8Utils::EncodeToWideString(const uint8 * string, size_t size, WideString & result)
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

