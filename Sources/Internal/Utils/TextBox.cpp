#include "TextBox.h"
#include "Utils/StringUtils.h"

#define U_COMMON_IMPLEMENTATION
#define U_STATIC_IMPLEMENTATION
#if __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wheader-hygiene"
#pragma clang diagnostic ignored "-Wold-style-cast"
#endif
#include <unicode/ustring.h>
#include <unicode/ubidi.h>
#include <unicode/ushape.h>
#if __clang__
#pragma clang diagnostic pop
#endif

namespace DAVA
{
static bool isAlefChar(int32 ch)
{
    return (ch == 0x0622) || (ch == 0x0623) || (ch == 0x0625) || (ch == 0x0627);
}

static bool isLamChar(int32 ch)
{
    return ch == 0x0644;
}

static bool isLamAlefChar(int32 ch)
{
    return (ch >= 0xFEF5) && (ch <= 0xFEFC);
}

static bool isTashkeelCharFE(int32 ch)
{
    return (ch >= 0xFE70) && (ch <= 0xFE7F);
}

using UCharString = BasicString<UChar>;

static WideString ConvertU2W(const UChar* src, const int32 length)
{
    int32 dstLen = 0;
    UErrorCode errorCode = U_ZERO_ERROR;
    u_strToWCS(nullptr, 0, &dstLen, src, length, &errorCode);
    if (errorCode != U_ZERO_ERROR && errorCode != U_BUFFER_OVERFLOW_ERROR)
    {
        Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
        return WideString();
    }

    WideString dst(dstLen, 0);
    errorCode = U_ZERO_ERROR;
    u_strToWCS(const_cast<WideString::value_type*>(dst.data()), dst.capacity(), 0, src, length, &errorCode);
    if (errorCode != U_ZERO_ERROR && errorCode != U_STRING_NOT_TERMINATED_WARNING)
    {
        Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
        return WideString();
    }

    return dst;
}

static UCharString ConvertW2U(const WideString& src)
{
    int32 dstLen = 0;
    UErrorCode errorCode = U_ZERO_ERROR;
    u_strFromWCS(nullptr, 0, &dstLen, src.data(), src.length(), &errorCode);
    if (errorCode != U_ZERO_ERROR && errorCode != U_BUFFER_OVERFLOW_ERROR)
    {
        Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
        return UCharString();
    }

    UCharString dst(dstLen, 0);
    errorCode = U_ZERO_ERROR;
    u_strFromWCS(const_cast<UCharString::value_type*>(dst.data()), dst.capacity(), 0, src.data(), src.length(), &errorCode);
    if (errorCode != U_ZERO_ERROR && errorCode != U_STRING_NOT_TERMINATED_WARNING)
    {
        Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
        return UCharString();
    }

    return dst;
}

class TextBoxImpl
{
public:
    enum class Status
    {
        RESETED = 0,
        SHAPED,
        WRAPED,
        REORDERER
    };

    TextBoxImpl()
    {
        para = ubidi_open();
        DVASSERT_MSG(para != nullptr, "Can't alocate new paragraph");
        //ubidi_setReorderingOptions(para, UBIDI_OPTION_REMOVE_CONTROLS);
    }

    virtual ~TextBoxImpl()
    {
        ClearLines();
        if (para != nullptr)
        {
            ubidi_close(para);
        }
    }

    void SetWString(const WideString& str, const TextBox::DirectionMode mode)
    {
        SetUString(ConvertW2U(str), mode);
    }

    void SetUString(const UCharString& str, const TextBox::DirectionMode mode)
    {
        paraText = str;
        baseLevel = DirectionModeToBiDiLevel(mode);
        UpdatePara();

        uint32 length = uint32(paraText.length());
        characters.resize(length);
        for (uint32 i = 0; i < length; ++i)
        {
            TextBox::Character& c = characters[i];
            c.logicIndex = i;
            c.visualIndex = i;
        }

        ClearLines();
        AddLineRange(0, length);
    }

    void ChangeDirectionMode(const TextBox::DirectionMode mode)
    {
        baseLevel = DirectionModeToBiDiLevel(mode);
        UpdatePara();
    }

    void UpdatePara()
    {
        UErrorCode errorCode = U_ZERO_ERROR;
        ubidi_setPara(para, paraText.data(), paraText.length(), baseLevel, nullptr, &errorCode);
        if (errorCode != U_ZERO_ERROR)
        {
            Logger::Error("[TextBox::SetText] errorCode = %d", errorCode);
        }
    }

    void ClearLines()
    {
        lines.clear();
    }

    void Shape()
    {
        const UChar* text = ubidi_getText(para);
        const int32 length = ubidi_getLength(para);

        UErrorCode errorCode = U_ZERO_ERROR;
        UCharString output(length, 0);
        u_shapeArabic(text, length, const_cast<UChar*>(output.data()), length, U_SHAPE_LENGTH_FIXED_SPACES_NEAR | U_SHAPE_LETTERS_SHAPE, &errorCode);
        if (errorCode != U_ZERO_ERROR && errorCode != U_STRING_NOT_TERMINATED_WARNING)
        {
            Logger::Error("[TextBox::Shape] shapeArabic2 errorCode = %d", errorCode);
            DVASSERT_MSG(false, "[TextBox::Shape] shapeArabic2 errorCode");
        }

        // Check new shaped length
        errorCode = U_ZERO_ERROR;
        int32 outputLength = u_shapeArabic(text, length, nullptr, 0, U_SHAPE_LETTERS_SHAPE, &errorCode);
        if (errorCode != U_ZERO_ERROR && errorCode != U_BUFFER_OVERFLOW_ERROR)
        {
            Logger::Error("[TextBox::Shape] shapeArabic1 errorCode = %d", errorCode);
            DVASSERT_MSG(false, "[TextBox::Shape] shapeArabic1 errorCode");
        }
        if (length > outputLength)
        {
            // Fix indeces mapping if length of result string reduced
            for (int32 i = 0; i < length; ++i)
            {
                TextBox::Character& c = characters[i];
                UChar outputChar = output[i];
                UChar inputChar = text[i];
                if (inputChar != ' ' && outputChar == ' ') // According shaping options all merged symbols replace with spaces
                {
                    // Merging!
                    c.skip = true;
                }
            }
        }
        else if (length < outputLength)
        {
            DVASSERT_MSG(false, "[TextBox::Shape] Unexpected behaviour");
        }

        // Store shaped text
        paraText = output;
        UpdatePara();
    }

    const WideString AsWideString() const
    {
        return ConvertU2W(ubidi_getText(para), ubidi_getLength(para));
    }

    const UCharString& AsUCharString() const
    {
        return paraText;
    }

    const int32 GetLength() const
    {
        return int32(ubidi_getLength(para));
    }

    const void AddLineRange(int32 start, int32 length)
    {
        TextBox::Line l;
        l.index = lines.size();
        l.start = start;
        l.length = length;
        l.visualString = ConvertU2W(paraText.data() + start, length);
        lines.push_back(l);

        // Store line index in character
        int32 limit = start + length;
        for (int32 i = start; i < limit; ++i)
        {
            characters[i].lineIndex = l.index;
        }
    }

    void ReorderLines()
    {
        if (lines.empty())
        {
            return;
        }

        Vector<int32> l2v;
        UErrorCode errorCode = U_ZERO_ERROR;
        UBiDi* lpara = ubidi_open();
        for (TextBox::Line& line : lines)
        {
            if (line.length == 0)
            {
                continue;
            }

            errorCode = U_ZERO_ERROR;
            ubidi_setPara(lpara, paraText.data() + line.start, line.length, baseLevel, nullptr, &errorCode);
            if (errorCode != U_ZERO_ERROR)
            {
                Logger::Error("[TextBox::ReorderLines] setPara errorCode = %d", errorCode);
                DVASSERT_MSG(false, "[TextBox::Shape] setPara errorCode");
            }

            // Write reordered string
            //             errorCode = U_ZERO_ERROR;
            //             int32 visualLength = ubidi_writeReordered(lpara, nullptr, 0, UBIDI_DO_MIRRORING, &errorCode);
            //             if (errorCode != U_ZERO_ERROR && errorCode != U_BUFFER_OVERFLOW_ERROR)
            //             {
            //                 Logger::Error("[TextBox::ReorderLines] writeReordered errorCode = %d", errorCode);
            //                 DVASSERT_MSG(false, "[TextBox::ReorderLines] writeReordered errorCode");
            //             }
            errorCode = U_ZERO_ERROR;
            UCharString visString(line.length, 0);
            ubidi_writeReordered(lpara, const_cast<UChar*>(visString.data()), line.length, UBIDI_DO_MIRRORING, &errorCode);
            if (errorCode != U_ZERO_ERROR && errorCode != U_STRING_NOT_TERMINATED_WARNING)
            {
                Logger::Error("[TextBox::ReorderLines] writeReordered errorCode = %d", errorCode);
                DVASSERT_MSG(false, "[TextBox::ReorderLines] writeReordered errorCode");
            }

            line.visualString = ConvertU2W(visString.data(), line.length);

            // Get local reordered characters map
            int32 length = ubidi_getLength(lpara);
            l2v.resize(length);
            errorCode = U_ZERO_ERROR;
            ubidi_getLogicalMap(lpara, l2v.data(), &errorCode);
            if (errorCode != U_ZERO_ERROR)
            {
                Logger::Error("[TextBox::ReorderLines] getLogicalMap errorCode = %d", errorCode);
                DVASSERT_MSG(false, "[TextBox::ReorderLines] getLogicalMap errorCode");
            }

            // Correct global reordered characters map according local map
            int32 li = line.start;
            for (int32 i = 0; i < length; ++i)
            {
                TextBox::Character& c = characters[li++];
                c.visualIndex = l2v[i] + line.start; // Make local index global (text)
                c.rtl = (ubidi_getLevelAt(lpara, i) & UBIDI_RTL) == UBIDI_RTL;
            }
        }
        ubidi_close(lpara);
    }

    void SmartCleanUp()
    {
        for (TextBox::Line& line : lines)
        {
            int32 limit = line.start + line.length;
            for (int32 li = line.start; li < limit; ++li)
            {
                if (characters[li].skip)
                {
                    int32 vi = characters[li].visualIndex - line.start; // Make global index local (line)
                    line.visualString.erase(vi, 1);
                }
            }
        }
    }

    const TextBox::Direction GetDirection() const
    {
        return BiDiDirectionToDirection(ubidi_getDirection(para));
    }

    const TextBox::Direction GetBaseDirection() const
    {
        return BiDiDirectionToDirection(ubidi_getBaseDirection(ubidi_getText(para), ubidi_getLength(para)));
    }

    UCharString paraText;
    UBiDi* para = nullptr;
    Vector<TextBox::Line> lines;
    Vector<TextBox::Character> characters;
    UBiDiLevel baseLevel = UBIDI_DEFAULT_LTR;

    static UBiDiLevel DirectionModeToBiDiLevel(const TextBox::DirectionMode mode)
    {
        switch (mode)
        {
        case TextBox::DirectionMode::AUTO:
        case TextBox::DirectionMode::WEAK_LTR:
            return UBIDI_DEFAULT_LTR;
        case TextBox::DirectionMode::WEAK_RTL:
            return UBIDI_DEFAULT_RTL;
        case TextBox::DirectionMode::STRONG_LTR:
            return UBIDI_LTR;
        case TextBox::DirectionMode::STRONG_RTL:
            return UBIDI_RTL;
        }
        DVASSERT_MSG(false, "Undefined direction mode");
        return UBIDI_DEFAULT_LTR;
    }

    static TextBox::Direction BiDiDirectionToDirection(const UBiDiDirection dir)
    {
        switch (dir)
        {
        case UBIDI_LTR:
        case UBIDI_NEUTRAL:
            return TextBox::Direction::LTR;
        case UBIDI_RTL:
            return TextBox::Direction::RTL;
        case UBIDI_MIXED:
            return TextBox::Direction::MIXED;
        }
        DVASSERT_MSG(false, "Undefined direction");
        return TextBox::Direction::LTR;
    }
};

/******************************************************************************/
/******************************************************************************/

TextBox::TextBox()
{
    pImpl.reset(new TextBoxImpl());
}

TextBox::TextBox(const TextBox& box)
{
}

TextBox::~TextBox()
{
}

void TextBox::SetText(const WideString& str, const DirectionMode mode)
{
    if (logicalText != str)
    {
        logicalText = str;
        pImpl->SetWString(logicalText, mode);
    }
}

void TextBox::ChangeDirectionMode(const DirectionMode mode)
{
    pImpl->ChangeDirectionMode(mode);
}

void TextBox::Shape()
{
    pImpl->Shape();
}

void TextBox::Split(Splitter& splitter)
{
    pImpl->ClearLines();
    int32 lineOffset = 0;
    int32 lineLength = 0;
    const WideString& shapedText = pImpl->AsWideString();
    while ((lineLength = splitter.split(shapedText, lineOffset)) != 0)
    {
        pImpl->AddLineRange(lineOffset, lineLength);
        lineOffset += lineLength;
    }
}

void TextBox::Reorder()
{
    pImpl->ReorderLines();
}

void TextBox::SmartCleanUp()
{
    pImpl->SmartCleanUp();
}

void TextBox::Measure(const Vector<float32>& characterSizes, float32 lineHeight, int32 fromLine, int32 linesCount)
{
    DVASSERT_MSG(characterSizes.size() == pImpl->GetLength(), "Incorrect character sizes information");

    int32 lineLimit = fromLine + linesCount;
    for (int32 lineIndex = fromLine; lineIndex < lineLimit; ++lineIndex)
    {
        Line& line = pImpl->lines.at(lineIndex);

        line.yoffset = (lineIndex - fromLine) * lineHeight;
        line.yadvance = lineHeight;
        line.xoffset = 0.f;
        line.xadvance = 0.f;

        // Generate visual order
        Vector<int32> vo(line.length, -1);
        int32 limit = line.start + line.length;
        for (int32 li = line.start; li < limit; ++li)
        {
            const Character& c = pImpl->characters.at(li);
            if (c.skip)
            {
                continue;
            }

            int32 vi = c.visualIndex - line.start; // Make global index local (line)
            vo[vi] = c.logicIndex;
        }

        // Layout
        for (int32 logInd : vo)
        {
            if (logInd >= 0)
            {
                Character& c = pImpl->characters[logInd];
                c.xoffset = line.xadvance;
                c.xadvance = characterSizes[logInd];
                c.yoffset = line.yoffset;
                c.yadvance = line.yadvance;
                line.xoffset = std::min(line.xoffset, c.xoffset);
                line.xadvance += c.xadvance;
            }
        }
    }
}

const DAVA::TextBox::Direction TextBox::GetDirection() const
{
    return pImpl->GetDirection();
}

const DAVA::WideString& TextBox::GetText() const
{
    return logicalText;
}

const WideString TextBox::GetShapedText() const
{
    return pImpl->AsWideString();
}

const TextBox::Direction TextBox::GetBaseDirection() const
{
    return pImpl->GetBaseDirection();
}

const Vector<TextBox::Line>& TextBox::GetLines() const
{
    return pImpl->lines;
}

const TextBox::Line& TextBox::GetLine(const int32 index) const
{
    return pImpl->lines.at(index);
}

const uint32 TextBox::GetLinesCount() const
{
    return uint32(pImpl->lines.size());
}

const Vector<TextBox::Character>& TextBox::GetCharacters() const
{
    return pImpl->characters;
}

const TextBox::Character& TextBox::GetCharacter(int32 index) const
{
    return pImpl->characters.at(index);
}

const uint32 TextBox::GetCharactersCount() const
{
    return uint32(pImpl->characters.size());
}

/******************************************************************************/
/******************************************************************************/

uint32 TextBox::EmptySplitter::split(const WideString& str, const uint32 from)
{
    return str.length() - from;
}

uint32 TextBox::SimpleSplitter::split(const WideString& str, const uint32 from)
{
    int32 to = str.find_first_of('\n', from);
    return to - from;
}

TextBox::SmartSplitter::SmartSplitter(Vector<uint8>* _breaks, Vector<float32>* _widths, const float32 _maxWidth, const bool _splitBySymbols)
    : breaks(_breaks)
    , widths(_widths)
    , maxWidth(_maxWidth)
    , splitBySymbols(_splitBySymbols)
{
    DVASSERT_MSG(breaks != nullptr, "Breaks information is nullptr");
    DVASSERT_MSG(widths != nullptr, "Characters widths information is nullptr");
}

uint32 TextBox::SmartSplitter::split(const WideString& str, const uint32 from)
{
    DVASSERT_MSG(breaks->size() == str.length(), "Incorrect breaks information");
    DVASSERT_MSG(widths->size() == str.length(), "Incorrect character sizes information");

    float32 currentWidth = 0;
    uint32 lastPossibleBreak = 0;
    uint32 textLength = uint32(str.length());
    for (uint32 pos = from; pos < textLength; ++pos)
    {
        const WideString::value_type& ch = str.at(pos);
        uint8 canBreak = breaks->at(pos);
        currentWidth += widths->at(pos);

        // Check that targetWidth defined and currentWidth less than targetWidth.
        // If symbol is whitespace skip it and go to next (add all whitespace to current line)
        if (currentWidth <= maxWidth || StringUtils::IsWhitespace(char16(ch)))
        {
            if (canBreak == StringUtils::LB_MUSTBREAK) // If symbol is line breaker then split string
            {
                return pos - from + 1;
            }
            else if (canBreak == StringUtils::LB_ALLOWBREAK || splitBySymbols) // Store breakable symbol position
            {
                lastPossibleBreak = pos;
            }
            continue;
        }

        if (lastPossibleBreak > 0) // If we have any breakable symbol in current substring then split by it
        {
            return lastPossibleBreak - from + 1;
        }
        else // Too big single word in line, split word by symbol
        {
            return pos - from;
        }
    }
    return 0;
}
}