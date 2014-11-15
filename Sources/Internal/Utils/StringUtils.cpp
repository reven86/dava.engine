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

bool StringUtils::BiDiPrepare(const WideString& logicalStr, WideString& preparedStr, bool* isRTL)
{
    static const uint32 MAX_LINE_LENGTH = 65535;
    static FriBidiFlags flags = FRIBIDI_FLAGS_DEFAULT | FRIBIDI_FLAGS_ARABIC;

    String inputUTF = UTF8Utils::EncodeToUTF8(logicalStr);
    uint32 utf_len = (uint32)inputUTF.length();
    
    FriBidiChar* logical = new FriBidiChar[utf_len + 1];
    uint32 fribidi_len = fribidi_charset_to_unicode(FRIBIDI_CHAR_SET_UTF8, inputUTF.c_str(), utf_len, logical);
    if (fribidi_len == 0)
    {
        SAFE_DELETE_ARRAY(logical);
        return false;
    }
    
    FriBidiCharType* bidi_types = new FriBidiCharType[fribidi_len];
    if (!bidi_types)
    {
        SAFE_DELETE_ARRAY(logical);
        return false;
    }

    fribidi_get_bidi_types(logical, fribidi_len, bidi_types);

    FriBidiLevel* embedding_levels = new FriBidiLevel[fribidi_len];
    if (!embedding_levels)
    {
        SAFE_DELETE_ARRAY(logical);
        return false;
    }

    FriBidiParType base_dir = FRIBIDI_PAR_ON;
    FriBidiLevel max_level = fribidi_get_par_embedding_levels(bidi_types, fribidi_len, &base_dir, embedding_levels) - 1;
    if (max_level < 0)
    {
        SAFE_DELETE_ARRAY(logical);
        return false;
    }

    FriBidiChar* visual = new FriBidiChar[fribidi_len + 1];
    Memcpy(visual, logical, fribidi_len * sizeof(*visual));

    /* Arabic joining */
    FriBidiArabicProp * ar_props = new FriBidiArabicProp[fribidi_len];
    fribidi_get_joining_types(logical, fribidi_len, ar_props);
    fribidi_join_arabic(bidi_types, fribidi_len, embedding_levels, ar_props);
    fribidi_shape(flags, embedding_levels, fribidi_len, ar_props, visual);
    SAFE_DELETE_ARRAY(ar_props);
    
    char outstring[MAX_LINE_LENGTH];
    utf_len = fribidi_unicode_to_charset(FRIBIDI_CHAR_SET_UTF8, visual, fribidi_len, outstring);
    UTF8Utils::EncodeToWideString(reinterpret_cast<unsigned char*>(outstring), utf_len, preparedStr);
    if (isRTL)
    {
        *isRTL = FRIBIDI_IS_RTL(base_dir);
    }

    SAFE_DELETE_ARRAY(logical);
    SAFE_DELETE_ARRAY(visual);
    return true;
}

bool StringUtils::BiDiReorder(WideString& string, const bool forceRtl)
{
    static const uint32 MAX_LINE_LENGTH = 65535;
    static FriBidiFlags flags = FRIBIDI_FLAGS_DEFAULT | FRIBIDI_FLAGS_ARABIC;

    String inputUTF = UTF8Utils::EncodeToUTF8(string);
    uint32 utf_len = (uint32)inputUTF.length();

    FriBidiChar* logical = new FriBidiChar[utf_len + 1];
    uint32 fribidi_len = fribidi_charset_to_unicode(FRIBIDI_CHAR_SET_UTF8, inputUTF.c_str(), utf_len, logical);
    if (fribidi_len == 0)
    {
        SAFE_DELETE_ARRAY(logical);
        return false;
    }

    FriBidiCharType* bidi_types = new FriBidiCharType[fribidi_len];
    if (!bidi_types)
    {
        SAFE_DELETE_ARRAY(logical);
        return false;
    }

    fribidi_get_bidi_types(logical, fribidi_len, bidi_types);

    FriBidiLevel* embedding_levels = new FriBidiLevel[fribidi_len];
    if (!embedding_levels)
    {
        SAFE_DELETE_ARRAY(logical);
        return false;
    }

    FriBidiParType base_dir = forceRtl ? (FRIBIDI_PAR_RTL) : (FRIBIDI_PAR_ON);
    FriBidiLevel max_level = fribidi_get_par_embedding_levels(bidi_types, fribidi_len, &base_dir, embedding_levels) - 1;
    if (max_level < 0)
    {
        SAFE_DELETE_ARRAY(logical);
        return false;
    }

    FriBidiChar* visual = new FriBidiChar[fribidi_len + 1];
    Memcpy(visual, logical, fribidi_len * sizeof(*visual));

    fribidi_boolean status = fribidi_reorder_line(flags, bidi_types, fribidi_len, 0, base_dir, embedding_levels, visual, NULL);

    char outstring[MAX_LINE_LENGTH];
    fribidi_len = fribidi_remove_bidi_marks(visual, fribidi_len, NULL, NULL, NULL);
    utf_len = fribidi_unicode_to_charset(FRIBIDI_CHAR_SET_UTF8, visual, fribidi_len, outstring);
    UTF8Utils::EncodeToWideString(reinterpret_cast<unsigned char*>(outstring), utf_len, string);

    SAFE_DELETE_ARRAY(logical);
    SAFE_DELETE_ARRAY(visual);
    return status != 0;
}

}