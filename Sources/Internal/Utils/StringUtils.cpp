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

#include "DAVAEngine.h"
#include "StringUtils.h"
#include "Utils/UTF8Utils.h"

#include "fribidi/fribidi.h"
#include "fribidi/fribidi-bidi-types.h"
#include "unibreak/linebreak.h"

namespace DAVA
{

void StringUtils::GetLineBreaks(const WideString& string, Vector<uint8>& breaks, const char8* locale)
{
    breaks.resize(string.length(), LB_NOBREAK); // By default all characters not breakable
#if defined(__DAVAENGINE_WIN32__) // sizeof(wchar_t) == 2
    set_linebreaks_utf16(reinterpret_cast<const utf16_t*>(string.c_str()), string.length(), locale, reinterpret_cast<char*>(&breaks.front()));
#else
    set_linebreaks_utf32(reinterpret_cast<const utf32_t*>(string.c_str()), string.length(), locale, reinterpret_cast<char*>(&breaks.front()));
#endif
}

WideString StringUtils::Trim(const WideString& string)
{
    WideString::const_iterator it = string.begin();
    WideString::const_iterator end = string.end();
    WideString::const_reverse_iterator rit = string.rbegin();
    while (it != end && iswspace(*it)) ++it;
    while (rit.base() != it && iswspace(*rit)) ++rit;
    return WideString(it, rit.base());
}

WideString StringUtils::TrimLeft(const WideString& string)
{
    WideString::const_iterator it = string.begin();
    WideString::const_iterator end = string.end();
    while (it != end && iswspace(*it)) ++it;
    return WideString(it, end);
}

WideString StringUtils::TrimRight(const WideString& string)
{
    WideString::const_reverse_iterator rit = string.rbegin();
    WideString::const_reverse_iterator rend = string.rend();
    while (rit != rend && iswspace(*rit)) ++rit;
    return WideString(rend.base(), rit.base());
}

}