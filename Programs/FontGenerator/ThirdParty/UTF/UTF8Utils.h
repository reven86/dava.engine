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


#ifndef __DAVAENGINE_UTF8UTILS_H__
#define __DAVAENGINE_UTF8UTILS_H__

#include <Base/BaseTypes.h>

/**
	 \ingroup utils
	 \brief Class to work with UTF8 strings
 */
class UTF8Utils
{
public:
    /**
		\brief convert UTF8 string to WideString
		\param[in] string string in UTF8 format
		\param[in] size size of buffer allocated for this string
		\param[out] resultString result unicode string
	 */
    static void EncodeToWideString(const DAVA::uint8* string, size_t size, DAVA::WideString& resultString);

    /**
        \brief convert UTF8 string to WideString
        \param[in] utf8String string in UTF8 format
        \return string in unicode
     */
    inline static DAVA::WideString EncodeToWideString(const DAVA::String& utf8String);

    /**
	 \brief convert WideString string to UTF8
	 \param[in] wstring string in WideString format
	 \returns string in UTF8 format, contained in DAVA::String
	 */
    static DAVA::String EncodeToUTF8(const DAVA::WideString& wstring);
};

//////////////////////////////////////////////////////////////////////////

inline DAVA::WideString UTF8Utils::EncodeToWideString(const DAVA::String& utf8String)
{
    DAVA::WideString str;
    EncodeToWideString((const DAVA::uint8*)utf8String.c_str(), utf8String.length(), str);
    return str;
}

#endif // __DAVAENGINE_UTF8UTILS_H__
