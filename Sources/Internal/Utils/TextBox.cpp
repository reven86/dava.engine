#include "TextBox.h"
#include "Utils/StringUtils.h"

#define U_COMMON_IMPLEMENTATION
#define U_STATIC_IMPLEMENTATION
#if __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wheader-hygiene"
#pragma clang diagnostic ignored "-Wold-style-cast"
#endif
#include <unicode/unistr.h>
#include <unicode/ustring.h>
#include <unicode/ubidi.h>
#include <unicode/ushape.h>
#if __clang__
#pragma clang diagnostic pop
#endif

namespace DAVA
{
static bool isAlefChar(UChar ch)
{
    return (ch == 0x0622) || (ch == 0x0623) || (ch == 0x0625) || (ch == 0x0627);
}

static bool isLamChar(UChar ch)
{
    return ch == 0x0644;
}

static bool isLamAlefChar(UChar ch)
{
    return (ch >= 0xFEF5) && (ch <= 0xFEFC);
}

static bool isTashkeelCharFE(UChar ch)
{
    return (ch >= 0xFE70) && (ch <= 0xFE7F);
}

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
    u_strToWCS(&dst[0], dst.capacity(), 0, src, length, &errorCode);
    if (errorCode != U_ZERO_ERROR && errorCode != U_STRING_NOT_TERMINATED_WARNING)
    {
        Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
        return WideString();
    }

    return dst;
}

static UnicodeString ConvertW2U(const WideString& src)
{
    int32 dstLen = 0;
    UErrorCode errorCode = U_ZERO_ERROR;
    u_strFromWCS(nullptr, 0, &dstLen, src.data(), src.length(), &errorCode);
    if (errorCode != U_ZERO_ERROR && errorCode != U_BUFFER_OVERFLOW_ERROR)
    {
        Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
        return UnicodeString();
    }

    UnicodeString dst;
    errorCode = U_ZERO_ERROR;
    u_strFromWCS(dst.getBuffer(dstLen), dstLen, 0, src.data(), src.length(), &errorCode);
    dst.releaseBuffer(dstLen);
    if (errorCode != U_ZERO_ERROR && errorCode != U_STRING_NOT_TERMINATED_WARNING)
    {
        Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
        return UnicodeString();
    }

    return dst;
}

class TextBoxImpl
{
public:
    struct Line
    {
        Line()
        {
            para = ubidi_open();
            DVASSERT_MSG(para != nullptr, "Can't alocate new line");
            ubidi_setReorderingOptions(para, UBIDI_OPTION_REMOVE_CONTROLS);
        }

        Line(const Line& copy) = delete;

        Line(Line&& l)
        {
            index = l.index;
            offset = l.offset;
            length = l.length;
            para = l.para;
            reorderedText.fastCopyFrom(l.reorderedText);

            l.index = 0;
            l.offset = 0;
            l.length = 0;
            l.para = nullptr;
        }

        ~Line()
        {
            if (para != nullptr)
            {
                ubidi_close(para);
            }
        }

        Line& operator=(const Line& l) = delete;

        Line& operator==(Line&& l)
        {
            if (this != &l)
            {
                if (para != nullptr)
                {
                    ubidi_close(para);
                }

                index = l.index;
                offset = l.offset;
                length = l.length;
                para = l.para;
                reorderedText.fastCopyFrom(l.reorderedText);

                l.index = 0;
                l.offset = 0;
                l.length = 0;
                l.para = nullptr;
            }
        }

        int32 index = 0;
        int32 offset = 0;
        int32 length = 0;
        UBiDi* para = nullptr;
        UnicodeString reorderedText;
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

    void SetUString(const UnicodeString& str, const TextBox::DirectionMode mode)
    {
        paraText.fastCopyFrom(str);

        switch (mode)
        {
        case TextBox::DirectionMode::AUTO:
        case TextBox::DirectionMode::WEAK_LTR:
            baseLevel = UBIDI_DEFAULT_LTR;
            break;
        case TextBox::DirectionMode::WEAK_RTL:
            baseLevel = UBIDI_DEFAULT_RTL;
            break;
        case TextBox::DirectionMode::STRONG_LTR:
            baseLevel = UBIDI_LTR;
            break;
        case TextBox::DirectionMode::STRONG_RTL:
            baseLevel = UBIDI_RTL;
            break;
        }

        UpdatePara();
    }

    void UpdatePara()
    {
        UErrorCode errorCode = U_ZERO_ERROR;
        ubidi_setPara(para, paraText.getBuffer(), paraText.length(), baseLevel, nullptr, &errorCode);
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
        static const int32 shapeOptions = /*U_SHAPE_LENGTH_FIXED_SPACES_NEAR |*/ U_SHAPE_LETTERS_SHAPE;

        const UChar* text = ubidi_getText(para);
        const int32 length = ubidi_getLength(para);
        UErrorCode errorCode = U_ZERO_ERROR;
        int32 outputLength = u_shapeArabic(text, length, nullptr, 0, shapeOptions, &errorCode);
        if (errorCode != U_ZERO_ERROR && errorCode != U_BUFFER_OVERFLOW_ERROR)
        {
            Logger::Error("[TextBox::Shape] errorCode = %d", errorCode);
        }

        UnicodeString output;
        errorCode = U_ZERO_ERROR;
        u_shapeArabic(text, length, output.getBuffer(outputLength), outputLength, shapeOptions, &errorCode);
        output.releaseBuffer(outputLength);
        if (errorCode != U_ZERO_ERROR && errorCode != U_STRING_NOT_TERMINATED_WARNING)
        {
            Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
        }

        log2vis.resize(length);
        if (length == outputLength)
        {
            // Fill log2vis map from 0 to length-1
            std::iota(std::begin(log2vis), std::end(log2vis), 0);
        }
        else if (length > outputLength)
        {
            // Fix indeces mapping if length of result string reduced
            int32 vi = 0;
            for (int32 li = 0; li < length; ++li, ++vi)
            {
                log2vis[li] = vi;
                if ((isLamChar(text[li]) && (li + 1 < length) && isAlefChar(text[li + 1])) || isTashkeelCharFE(text[li]))
                {
                    log2vis[++li] = -1;
                }
            }
            DVASSERT_MSG(vi == outputLength, "Incorrect mapping logical to shaped characters");
        }
        else
        {
            DVASSERT_MSG(false, "Unexpected behaviour");
        }

        paraText.fastCopyFrom(output);
        UpdatePara();
    }

    const WideString ToWideString() const
    {
        return ConvertU2W(ubidi_getText(para), ubidi_getLength(para));
    }

    const int32 GetLength() const
    {
        return int32(ubidi_getLength(para));
    }

    const void AddLineRange(int32 start, int32 length)
    {
        Line l;
        l.index = lines.size();
        l.offset = start;
        l.length = length;

        UErrorCode errorCode = U_ZERO_ERROR;
        ubidi_setLine(para, start, start + length, l.para, &errorCode);
        if (errorCode != U_ZERO_ERROR)
        {
            Logger::Error("[TextBox::SetText] errorCode = %d", errorCode);
        }

        lines.push_back(std::move(l));
    }

    void ReorderLines()
    {
        const static int32 reorderingOptions = UBIDI_DO_MIRRORING | UBIDI_REMOVE_BIDI_CONTROLS;
        Vector<int32> l2v;
        UErrorCode errorCode = U_ZERO_ERROR;
        for (Line& line : lines)
        {
            // Write reordered string
            errorCode = U_ZERO_ERROR;
            int32 visualLength = ubidi_writeReordered(line.para, nullptr, 0, reorderingOptions, &errorCode);
            if (errorCode != U_ZERO_ERROR && errorCode != U_BUFFER_OVERFLOW_ERROR)
            {
                Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
            }
            errorCode = U_ZERO_ERROR;
            ubidi_writeReordered(line.para, line.reorderedText.getBuffer(visualLength), visualLength, reorderingOptions, &errorCode);
            line.reorderedText.releaseBuffer(visualLength);
            if (errorCode != U_ZERO_ERROR && errorCode != U_STRING_NOT_TERMINATED_WARNING)
            {
                Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
            }

            // Get local reordered characters map
            int32 length = ubidi_getLength(line.para);
            l2v.resize(length);
            errorCode = U_ZERO_ERROR;
            ubidi_getLogicalMap(line.para, &l2v[0], &errorCode);
            if (errorCode != U_ZERO_ERROR)
            {
                Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
            }

            // Correct global reordered characters map according local map
            int32 li = line.offset;
            for (int32 i = 0; i < length; ++i)
            {
                if (log2vis[li] < 0)
                {
                    li++;
                }
                int32 vi = line.offset + l2v[i];
                log2vis[li++] = vi;
            }
        }
    }

    TextBox::Direction GetDirection() const
    {
        UBiDiDirection dir = ubidi_getDirection(para);
        switch (dir)
        {
        case UBIDI_RTL:
            return TextBox::Direction::RTL;
        case UBIDI_MIXED:
            return TextBox::Direction::MIXED;
        case UBIDI_LTR:
        case UBIDI_NEUTRAL:
        default:
            return TextBox::Direction::LTR;
        }
    }

    UnicodeString paraText;
    UBiDi* para = nullptr;
    Vector<Line> lines;
    Vector<int32> log2vis;
    UBiDiLevel baseLevel = UBIDI_DEFAULT_LTR;
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

void TextBox::SetText(const WideString& str, const DirectionMode mode = DirectionMode::AUTO)
{
    if (logicalText != str)
    {
        logicalText = str;
        pImpl->ClearLines();
        pImpl->SetWString(logicalText, mode);
        direction = pImpl->GetDirection();
    }
}

void TextBox::SetFont(Font* _font)
{
    font = _font;
}

void TextBox::Shape()
{
    pImpl->Shape();
}

void TextBox::Wrap(float32 maxWidth, const WrapMode mode)
{
    uint32 textLength = uint32(pImpl->GetLength());
    WideString shapedText = pImpl->ToWideString();

    // Detect breaks
    Vector<uint8> breaks;
    StringUtils::GetLineBreaks(shapedText, breaks);
    DVASSERT_MSG(breaks.size() == textLength, "Incorrect breaks information");

    // Get characters metrics
    Vector<float32> characterSizes;
    font->GetStringMetrics(shapedText, &characterSizes);
    DVASSERT_MSG(characterSizes.size() == textLength, "Incorrect character sizes information");

    // Wrap
    if (mode != WrapMode::NO_WRAP)
    {
        int32 lineOffset = 0;
        int32 lineLength = 0;
        float32 currentWidth = 0;
        uint32 lastPossibleBreak = 0;

        for (uint32 pos = 0; pos < textLength; ++pos)
        {
            char16 ch = logicalText[pos];
            uint8 canBreak = breaks[pos];

            currentWidth += characterSizes[pos];

            // Check that targetWidth defined and currentWidth less than targetWidth.
            // If symbol is whitespace skip it and go to next (add all whitespace to current line)
            if (currentWidth <= maxWidth || StringUtils::IsWhitespace(ch))
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
        pImpl->AddLineRange(0, textLength);
    }
}

void TextBox::Reorder()
{
    pImpl->ReorderLines();
}

void TextBox::CleanUp()
{
}

void TextBox::Measure()
{
}

const DAVA::TextBox::Direction TextBox::GetDirection() const
{
    return direction;
}

const DAVA::WideString& TextBox::GetText() const
{
    return logicalText;
}

const Vector<TextBox::Line>& TextBox::GetLines() const
{
    return lines;
}

const TextBox::Line& TextBox::GetLine(const int32 index) const
{
    return lines.at(index);
}

const uint32 TextBox::GetLinesCount() const
{
    return uint32(lines.size());
}

const Vector<TextBox::Character>& TextBox::GetCharacters() const
{
    return characters;
}

const TextBox::Character& TextBox::GetCharacter(int32 index) const
{
    static const Character NONE;
    int32 visualIndex = pImpl->log2vis.at(index);
    if (visualIndex == -1)
    {
        visualIndex = pImpl->log2vis.at(index - 1);
    }
    return characters.at(visualIndex);
}

const uint32 TextBox::GetCharactersCount() const
{
    return uint32(characters.size());
}

}