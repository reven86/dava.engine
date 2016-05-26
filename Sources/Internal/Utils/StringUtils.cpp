#include "DAVAEngine.h"
#include "StringUtils.h"
#include "Utils/UTF8Utils.h"

#include "unibreak/linebreak.h"

namespace DAVA
{
void StringUtils::GetLineBreaks(const WideString& string, Vector<uint8>& breaks, const char8* locale)
{
    breaks.resize(string.length(), LB_NOBREAK); // By default all characters not breakable
#if defined(__DAVAENGINE_WINDOWS__) // sizeof(wchar_t) == 2
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
    while (it != end && IsWhitespace(*it)) ++it;
    while (rit.base() != it && IsWhitespace(*rit)) ++rit;
    return WideString(it, rit.base());
}

WideString StringUtils::TrimLeft(const WideString& string)
{
    WideString::const_iterator it = string.begin();
    WideString::const_iterator end = string.end();
    while (it != end && IsWhitespace(*it)) ++it;
    return WideString(it, end);
}

WideString StringUtils::TrimRight(const WideString& string)
{
    WideString::const_reverse_iterator rit = string.rbegin();
    WideString::const_reverse_iterator rend = string.rend();
    while (rit != rend && IsWhitespace(*rit)) ++rit;
    return WideString(rend.base(), rit.base());
}

WideString StringUtils::RemoveNonPrintable(const WideString& string, const int8 tabRule /*= -1*/)
{
    WideString out;
    WideString::const_iterator it = string.begin();
    WideString::const_iterator end = string.end();
    for (; it != end; ++it)
    {
        if (!IsPrintable(*it))
        {
            continue;
        }

        switch (*it)
        {
        case L'\t':
            if (tabRule < 0)
            {
                out.push_back(*it); // Leave tab symbol
            }
            else
            {
                out.append(tabRule, L' '); // Replace tab with (tabRule x spaces)
            }
            break;
        case 0x00A0: // Non-break space
            out.push_back(L' ');
            break;
        default:
            out.push_back(*it);
            break;
        }
    }
    return out;
}

bool IsEmoji(int32 sym)
{
    // ranges of symbol codes with unicode emojies.
    static Vector<std::pair<DAVA::int32, DAVA::int32>> ranges = { { 0x2190, 0x21FF }, { 0x2600, 0x26FF }, { 0x2700, 0x27BF }, { 0x3000, 0x303F }, { 0xF300, 0x1F64F }, { 0x1F680, 0x1F6FF } };
    for (auto range : ranges)
    {
        if (sym >= range.first && sym <= range.second)
        {
            return true;
        }
    }
    return false;
}

bool StringUtils::RemoveEmoji(WideString& string)
{
    DAVA::WideString ret;
    bool isChanged = false;

    auto data = string.data();
    size_t length = string.length();
    for (size_t i = 0; i < length; ++i)
    {
        int32 sym;
        Memcpy(&sym, data + i, sizeof(int32));

        if (!IsEmoji(sym))
        {
            ret += sym;
        }
        else
        {
            isChanged = true;
        }
    }

    string = ret;

    // true means "we removed some emojies".
    return isChanged;
}
}