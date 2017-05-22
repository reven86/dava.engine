#include "DAVAEngine.h"
#include "StringUtils.h"
#include "Utils/UTF8Utils.h"

#include "unibreak/linebreak.h"

namespace DAVA
{
namespace StringUtils
{
void GetLineBreaks(const WideString& string, Vector<uint8>& breaks, const char8* locale)
{
    breaks.resize(string.length(), LB_NOBREAK); // By default all characters not breakable
#if defined(__DAVAENGINE_WINDOWS__) // sizeof(wchar_t) == 2
    set_linebreaks_utf16(reinterpret_cast<const utf16_t*>(string.c_str()), string.length(), locale, reinterpret_cast<char*>(&breaks.front()));
#elif defined(__DAVAENGINE_LINUX__)
// TODO: linux: build unibreak lib
#else
    set_linebreaks_utf32(reinterpret_cast<const utf32_t*>(string.c_str()), string.length(), locale, reinterpret_cast<char*>(&breaks.front()));
#endif
}

WideString RemoveNonPrintable(const WideString& string, const int8 tabRule /*= -1*/)
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
    static Vector<std::pair<int32, int32>> ranges = { { 0x2190, 0x21FF }, { 0x2300, 0x243F }, { 0x2600, 0x26FF }, { 0x2700, 0x27BF }, { 0x3000, 0x303F }, /*{ 0x1F1E6, 0x1F1FF },*/ { 0x1F300, 0x1F6FF }, { 0x1F900, 0x1F9FF } };
    for (auto range : ranges)
    {
        if (sym >= range.first && sym <= range.second)
        {
            return true;
        }
    }
    return false;
}

bool RemoveEmoji(WideString& string)
{
    WideString ret;
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
            while (i + 1 < length)
            {
                i++;
                Memcpy(&sym, data + i, sizeof(int32));
                if (sym != 0x200D && sym != 0xFE0F)
                {
                    i--;
                    break;
                }
            }
            isChanged = true;
        }
    }

    string = ret;

    // true means "we removed some emojies".
    return isChanged;
}

void ReplaceAll(WideString& string, const WideString& search, const WideString& replacement)
{
    size_t pos = 0;
    size_t oldSubStringLength = search.length();
    while ((pos = string.find(search, pos)) != WideString::npos)
    {
        string.replace(pos, oldSubStringLength, replacement);
        pos += 1;
    }
}
}
}