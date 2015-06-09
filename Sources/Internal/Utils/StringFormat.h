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


#ifndef __DAVAENGINE_STRINGFORMAT_H__
#define __DAVAENGINE_STRINGFORMAT_H__

#include "Base/BaseTypes.h"
#include <cstdarg>

#include "cppformat/format.h"

namespace DAVA
{

//! String formating functions
//! Functions for use together with Global::Log

//! Formatting function (use printf syntax)
//! Function support recursive calls as : 
//! Format("%s", Format("%d: %d: %d", 10, 20, 33));

//String Format(const char8* format, ... );
//String FormatVL(const char8* format, va_list args);

//! Formatting function (use printf syntax)
//WideString Format(const char16* format, ...);
//WideString FormatVL(const char16* format, va_list args);

template<typename... Args>
String Format(const char8* format, const Args&... args)
{
    String result = fmt::sprintf(format, args...);
    return result;
}

template<typename... Args>
WideString Format(const char16* format, const Args&... args)
{
    WideString result = fmt::sprintf(format, args...);
    return result;
}

//! Function to get indent strings for usage in printf and similar functions
inline String GetIndentString(char8 indentChar, int32 level)
{
    return String(level, indentChar);
}

};  // namespace DAVA

#endif // __DAVAENGINE_STRINGFORMAT_H__
