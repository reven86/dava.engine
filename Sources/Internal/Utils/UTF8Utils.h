#ifndef __DAVAENGINE_UTF8UTILS_H__
#define __DAVAENGINE_UTF8UTILS_H__

#include "Base/BaseTypes.h"

namespace DAVA
{
/**
	 \ingroup utils
	 \brief Class to work with UTF8 strings
 */
namespace UTF8Utils
{
/**
		\brief convert UTF8 string to WideString
		\param[in] string string in UTF8 format
		\param[in] size size of buffer allocated for this string
		\param[out] resultString result unicode string
	 */
void EncodeToWideString(const uint8* string, size_type size, WideString& resultString);

/**
        \brief convert UTF8 string to WideString
        \param[in] utf8String string in UTF8 format
        \return string in unicode
     */
inline WideString EncodeToWideString(const String& utf8String);

/**
	 \brief convert WideString string to UTF8
	 \param[in] wstring string in WideString format
	 \returns string in UTF8 format, contained in DAVA::String
	 */
String EncodeToUTF8(const WideString& wstring);

template <typename CHARTYPE>
inline String MakeUTF8String(const CHARTYPE* value);
};

//////////////////////////////////////////////////////////////////////////

inline WideString UTF8Utils::EncodeToWideString(const String& utf8String)
{
    WideString str;
    EncodeToWideString(reinterpret_cast<const uint8*>(utf8String.c_str()), utf8String.length(), str);
    return str;
}
template <>
inline String UTF8Utils::MakeUTF8String<char8>(const char8* value)
{
    return String(value);
}

template <>
inline String UTF8Utils::MakeUTF8String<char16>(const char16* value)
{
    return EncodeToUTF8(WideString(value));
}
};

#endif // __DAVAENGINE_UTF8UTILS_H__
