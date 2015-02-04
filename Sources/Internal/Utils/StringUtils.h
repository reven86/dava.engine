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

#ifndef __DAVAENGINE_STRING_UTILS__
#define __DAVAENGINE_STRING_UTILS__

#include "Base/BaseTypes.h"

namespace DAVA
{
/**
 * \namespace StringUtils
 *
 * \brief Namespace with string helper functions.
 */
namespace StringUtils
{
/**
* \enum eLineBreakType
*
* \brief Values that represent line break types.
*/
enum eLineBreakType
{
    LB_MUSTBREAK = 0,   /**< Break is mandatory */
    LB_ALLOWBREAK,      /**< Break is allowed */
    LB_NOBREAK,         /**< No break is possible */
    LB_INSIDECHAR       /**< A UTF-8/16 sequence is unfinished */
};

/**
* \brief Gets information about line breaks using libunibreak library.
* \param string The input string.
* \param [out] breaks The output vector of breaks.
* \param locale (Optional) The locale code.
*/
void GetLineBreaks(const WideString& string, Vector<uint8>& breaks, const char8* locale = 0);

/**
* \brief Trims the given string.
* \param [in,out] string The string.
*/
WideString Trim(const WideString& string);

/**
* \brief Trim left.
* \param [in,out] string The string.
*/
WideString TrimLeft(const WideString& string);

/**
* \brief Trim right.
* \param [in,out] string The string.
*/
WideString TrimRight(const WideString& string);

/**
 * \brief Query if 't' is space.
 * \param t The char16 to process.
 * \return true if space, false if not.
 */
inline bool IsWhitespace(char16 t)
{
    switch (t)
    {
        case 0x0009: // Tabulation
        case 0x000A: // Line feed
        case 0x000B: // Line tab
        case 0x000C: // Form feed
        case 0x000D: // Carriage return
        // Unicode characters in 'Separator, Space' category (Zs)
        case 0x0020: // Space
        case 0x00A0: // No-break space
        case 0x1680: // Ogham space mark
        case 0x2000: // En quad
        case 0x2001: // Em quad
        case 0x2002: // En space
        case 0x2003: // Em space
        case 0x2004: // Three-per-em space
        case 0x2005: // Four-per-em space
        case 0x2006: // Siz-per-em sapce
        case 0x2007: // Figure space
        case 0x2008: // Punctuation space
        case 0x2009: // Thin space
        case 0x200A: // Hair space
        case 0x202F: // Narrow No-break space
        case 0x205F: // Medium mathematical space
        case 0x3000: // Ideographic space
        // Unicode characters in 'Separator, Line' category (Zl)
        case 0x2028:
        // Unicode characters in 'Separator, Paragraph' category (Zp)
        case 0x2029:
        // Additional characters are treated as spaces
        case 0x200B: // Zero-width space
        case 0x200E: // Left-to-right zero-width character
        case 0x200F: // Right-to-left zero-width non-Arabic character
        case 0x061C: // Right-to-left zero-width Arabic character
            return true;
        default:
            return false;
    }
}

/**
 * \brief Prepare string for BiDi transformation (shape arabic string). Need for correct splitting.
 * \param [in] logicalStr The logical string.
 * \param [out] preparedStr The prepared string.
 * \param [out] isRTL If non-null, store in isRTL true if line contains Right-to-left text.
 * \return true if it succeeds, false if it fails.
 */
bool BiDiPrepare(const WideString& logicalStr, WideString& preparedStr, bool* isRTL);

/**
 * \brief Reorder characters in string based.
 * \param [in,out] string The string.
 * \param forceRtl (Optional) true if input text is mixed and must be processed as RTL.
 * \return true if it succeeds, false if it fails.
 */
bool BiDiReorder(WideString& string, const bool forceRtl = false);

}
}

#endif 
