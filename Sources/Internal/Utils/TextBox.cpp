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
    u_strFromWCS(const_cast<WideString::value_type*>(dst.data()), dst.capacity(), 0, src.data(), src.length(), &errorCode);
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
        ubidi_setReorderingOptions(para, UBIDI_OPTION_REMOVE_CONTROLS);
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

        uint32 length = uint32(paraText.length());
        characters.resize(length);
        for (uint32 i = 0; i < length; ++i)
        {
            TextBox::Character& c = characters[i];
            c.codepoint = paraText.at(i);
            c.logicIndex = i;
            c.shapedIndex = i;
            c.visualIndex = i;
        }

        baseLevel = DirectionModeToBiDiLevel(mode);
        UpdatePara();
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
        static const int32 shapeOptions = U_SHAPE_LENGTH_FIXED_SPACES_NEAR | U_SHAPE_LETTERS_SHAPE;

        const UChar* text = ubidi_getText(para);
        const int32 length = ubidi_getLength(para);
        UErrorCode errorCode = U_ZERO_ERROR;
        int32 outputLength = u_shapeArabic(text, length, nullptr, 0, U_SHAPE_LETTERS_SHAPE, &errorCode);
        if (errorCode != U_ZERO_ERROR && errorCode != U_BUFFER_OVERFLOW_ERROR)
        {
            Logger::Error("[TextBox::Shape] errorCode = %d", errorCode);
        }

        UCharString output(length, 0);
        errorCode = U_ZERO_ERROR;
        u_shapeArabic(text, length, const_cast<UChar*>(output.data()), length, U_SHAPE_LENGTH_FIXED_SPACES_NEAR | U_SHAPE_LETTERS_SHAPE, &errorCode);
        if (errorCode != U_ZERO_ERROR && errorCode != U_STRING_NOT_TERMINATED_WARNING)
        {
            Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
        }

        if (length > outputLength)
        {
            // Fix indeces mapping if length of result string reduced
#if 1
            for (int32 i = 0; i < length; ++i)
            {
                UChar outputChar = output[i];
                TextBox::Character& c = characters[i];
                if (c.codepoint != uint32(' ') && outputChar == ' ')
                {
                    // Merging!
                    c.shapedIndex = -1;
                    c.visualIndex = -1;
                }
                else
                {
                    c.shapedIndex = i;
                    c.visualIndex = i;
                }
            }
#elif 0 // Direct mapping
            int32 vi = 0;
            for (int32 li = 0; li < length; ++vi)
            {
                TextBox::Character& c = characters[li++];
                c.shapedIndex = vi;
                c.visualIndex = vi;

                bool isLam = isLamChar(c.codepoint);
                bool isTashkeelFE = isTashkeelCharFE(c.codepoint);
                if ((isLam || isTashkeelFE) && (li < length))
                {
                    TextBox::Character& next = characters[li++];
                    if ((isLam && isAlefChar(next.codepoint)) || isTashkeelFE)
                    {
                        next.shapedIndex = -1;
                        next.visualIndex = -1;
                    }
                }
            }
            DVASSERT_MSG(vi == outputLength, "Incorrect mapping logical to shaped characters");
#else // Invert mapping
            int32 li = 0;
            for (int32 si = 0; si < outputLength; ++si)
            {
                TextBox::Character& c = characters[li++];
                c.shapedIndex = si;
                c.visualIndex = si;

                if (isLamAlefChar(output.at(si)))
                {
                    TextBox::Character& next = characters[li++];
                    next.shapedIndex = -1;
                    next.visualIndex = -1;
                }
            }
            DVASSERT_MSG(li == length, "Incorrect mapping logical to shaped characters");
#endif
        }
        else if (length < outputLength)
        {
            DVASSERT_MSG(false, "Unexpected behaviour");
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
        l.shapedOffset = start;
        l.shapedLength = length;
        lines.push_back(l);
    }

    void ReorderLines()
    {
        const static int32 reorderingOptions = UBIDI_DO_MIRRORING;
        Vector<int32> l2v;
        UErrorCode errorCode = U_ZERO_ERROR;
        UBiDi* lpara = ubidi_open();
        for (TextBox::Line& line : lines)
        {
            UErrorCode errorCode = U_ZERO_ERROR;
            ubidi_setLine(para, line.shapedOffset, line.shapedOffset + line.shapedLength, lpara, &errorCode);
            if (errorCode != U_ZERO_ERROR)
            {
                Logger::Error("[TextBox::SetText] errorCode = %d", errorCode);
                return;
            }

            // Write reordered string
            errorCode = U_ZERO_ERROR;
            int32 visualLength = ubidi_writeReordered(lpara, nullptr, 0, reorderingOptions, &errorCode);
            if (errorCode != U_ZERO_ERROR && errorCode != U_BUFFER_OVERFLOW_ERROR)
            {
                Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
                return;
            }
            errorCode = U_ZERO_ERROR;
            UCharString visString(visualLength, 0);
            ubidi_writeReordered(lpara, const_cast<UChar*>(visString.data()), visualLength, reorderingOptions, &errorCode);
            if (errorCode != U_ZERO_ERROR && errorCode != U_STRING_NOT_TERMINATED_WARNING)
            {
                Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
                return;
            }

            line.visualString = ConvertU2W(visString.data(), visualLength);

#if 1 // Logic 2 visual
            // Get local reordered characters map
            int32 length = ubidi_getLength(lpara);
            l2v.resize(length);
            errorCode = U_ZERO_ERROR;
            ubidi_getLogicalMap(lpara, l2v.data(), &errorCode);
            if (errorCode != U_ZERO_ERROR)
            {
                Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
                return;
            }

#if 1 // Algorithm 1
            // Correct global reordered characters map according local map
            int32 li = line.shapedOffset;
            for (int32 i = 0; i < length; ++i)
            {
                TextBox::Character& c = characters[li++];
                if (c.shapedIndex < 0)
                {
                    //continue;
                }
                c.visualIndex = l2v[i] + line.shapedOffset;
                c.rtl = (ubidi_getLevelAt(lpara, i) & UBIDI_RTL) == UBIDI_RTL;
            }
#else // Algorithm 2
            // Correct global reordered characters map according local map
            int32 limit = line.shapedOffset + line.shapedLength;
            for (int32 li = line.shapedOffset; li < limit; ++li)
            {
                TextBox::Character* c = &characters[li];
                if (c->shapedIndex >= 0)
                {
                    int32 si = c->shapedIndex - line.shapedOffset; // Make shaped index local (line)
                    int32 vi = l2v[si] + line.shapedOffset; // Make visual index global (text)
                    c->visualIndex = vi;
                }
            }
#endif
#else // Visual 2 logic
            // Get local reordered characters map
            int32 length = ubidi_getLength(lpara);
            line.visualOrder.resize(length);
            errorCode = U_ZERO_ERROR;
            ubidi_getVisualMap(lpara, line.visualOrder.data(), &errorCode);
            if (errorCode != U_ZERO_ERROR)
            {
                Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
            }

            // Correct global reordered characters map according local map
            int32 limit = line.shapedOffset + line.shapedLength;
            for (int32 li = line.shapedOffset; li < limit; ++li)
            {
                TextBox::Character* c = &characters[li];
                int32 si = c->shapedIndex;
                if (si >= 0)
                {
                    si -= line.shapedOffset; // Make shaped index local (line)
                    int32& vi = line.visualOrder[si]; // Get visual
                    vi += line.shapedOffset; // Make visual index global (text)
                    c->visualIndex = vi;
                }
            }
#endif
        }
        ubidi_close(lpara);
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
        pImpl->ClearLines();
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

void TextBox::Wrap(const WrapMode mode, float32 maxWidth, const Vector<float32>* characterSizes, const Vector<uint8>* breaks)
{
    pImpl->ClearLines();

    // Wrap
    if (mode != WrapMode::NO_WRAP)
    {
        const UCharString& shapedText = pImpl->AsUCharString();
        DVASSERT_MSG(breaks != nullptr, "Empty breaks pointer");
        DVASSERT_MSG(breaks->size() == shapedText.length(), "Incorrect breaks information");
        DVASSERT_MSG(characterSizes != nullptr, "Empty character sizes pointer");
        DVASSERT_MSG(characterSizes->size() == shapedText.length(), "Incorrect character sizes information");

        int32 lineOffset = 0;
        int32 lineLength = 0;
        float32 currentWidth = 0;
        uint32 lastPossibleBreak = 0;
        uint32 textLength = uint32(shapedText.length());
        for (uint32 pos = 0; pos < textLength; ++pos)
        {
            UChar ch = shapedText.at(pos);
            uint8 canBreak = breaks->at(pos);
            currentWidth += characterSizes->at(pos);

            // Check that targetWidth defined and currentWidth less than targetWidth.
            // If symbol is whitespace skip it and go to next (add all whitespace to current line)
            if (currentWidth <= maxWidth || StringUtils::IsWhitespace(char16(ch)))
            {
                if (canBreak == StringUtils::LB_MUSTBREAK) // If symbol is line breaker then split string
                {
                    lineLength = pos - lineOffset + 1;
                    currentWidth = 0.f;
                    lastPossibleBreak = 0;
                    pImpl->AddLineRange(lineOffset, lineLength);
                    lineOffset += lineLength;
                }
                else if (canBreak == StringUtils::LB_ALLOWBREAK || mode == WrapMode::SYMBOLS_WRAP) // Store breakable symbol position
                {
                    lastPossibleBreak = pos;
                }
                continue;
            }

            if (lastPossibleBreak > 0) // If we have any breakable symbol in current substring then split by it
            {
                pos = lastPossibleBreak;
                lineLength = pos - lineOffset + 1;
                currentWidth = 0.f;
                lastPossibleBreak = 0;
                pImpl->AddLineRange(lineOffset, lineLength);
                lineOffset += lineLength;
                continue;
            }
        }
        DVASSERT_MSG(lineOffset == textLength, "Incorrect line split");
    }
    else
    {
        pImpl->AddLineRange(0, pImpl->characters.size());
    }
}

void TextBox::Reorder()
{
    pImpl->ReorderLines();
}

void TextBox::CleanUp()
{
}

void TextBox::Measure(const Vector<float32>& characterSizes, float32 lineHeight, int32 fromLine, int32 linesCount)
{
    //     int32 limit = start + length;
    //     for (int32 i = start; i < limit; ++i)
    //     {
    //         characters[i].line = l.index;
    //     }

    DVASSERT_MSG(characterSizes.size() == pImpl->GetLength(), "Incorrect character sizes information");

    int32 lineLimit = fromLine + linesCount;
    for (int32 lineIndex = fromLine; lineIndex < lineLimit; ++lineIndex)
    {
        Line& line = pImpl->lines.at(lineIndex);

        float32 lineyoffset = (lineIndex - fromLine) * lineHeight;
        float32 lineyadvance = lineHeight;
        float32 linexadvance = 0.f;
        float32 linexoffset = 0.f;
#if 1 // Generate visual order
        Vector<int32> vo(line.shapedLength, -1);
        int32 limit = line.shapedOffset + line.shapedLength;
        auto beginIt = std::find_if(std::begin(pImpl->characters), std::end(pImpl->characters), [&line](const Character& c)
                                    {
                                        return c.shapedIndex == line.shapedOffset;
                                    });
        auto endIt = std::find_if(std::begin(pImpl->characters), std::end(pImpl->characters), [&limit](const Character& c)
                                  {
                                      return c.shapedIndex == limit;
                                  });
        for (auto it = beginIt; it != endIt; ++it)
        {
            const Character& c = *it;
            //if (c.visualIndex >= 0)
            {
                int32 vi = c.visualIndex - line.shapedOffset; // Make visual index local (line)
                vo[vi] = c.logicIndex;
            }
        }
#else // Take from line
        const Vector<int32>& vo = line.visualOrder;
#endif
        for (int32 logInd : vo)
        {
            if (logInd >= 0)
            {
                Character& c = pImpl->characters[logInd];
                c.xoffset = linexadvance;
                c.xadvance = characterSizes[logInd];
                c.yoffset = lineyoffset;
                c.yadvance = lineyadvance;
                linexoffset = std::min(linexoffset, c.xoffset);
                linexadvance += c.xadvance;
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

}