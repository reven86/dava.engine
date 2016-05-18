#include "TextBox.h"

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
#include <unicode/ubrk.h>
#if __clang__
#pragma clang diagnostic pop
#endif

#include "Utils/BiDiHelper.h"
#include "Utils/StringUtils.h"

namespace DAVA
{
using UCharString = BasicString<UChar>;

void ConvertU2W(const UCharString& src, WideString& dst)
{
    int32 dstLen = 0;
    UErrorCode errorCode = U_ZERO_ERROR;
    u_strToWCS(nullptr, 0, &dstLen, src.data(), src.length(), &errorCode);
    if (errorCode != U_ZERO_ERROR && errorCode != U_BUFFER_OVERFLOW_ERROR)
    {
        Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
    }

    dst.resize(dstLen);
    errorCode = U_ZERO_ERROR;
    u_strToWCS(&dst[0], dst.capacity(), 0, src.data(), src.length(), &errorCode);
    if (errorCode != U_ZERO_ERROR)
    {
        Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
    }
}

void ConvertW2U(const WideString& src, UCharString& dst)
{
    int32 dstLen = 0;
    UErrorCode errorCode = U_ZERO_ERROR;
    u_strFromWCS(nullptr, 0, &dstLen, src.data(), src.length(), &errorCode);
    if (errorCode != U_ZERO_ERROR && errorCode != U_BUFFER_OVERFLOW_ERROR)
    {
        Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
    }

    dst.resize(dstLen);
    errorCode = U_ZERO_ERROR;
    u_strFromWCS(&dst[0], dst.capacity(), 0, src.data(), src.length(), &errorCode);
    if (errorCode != U_ZERO_ERROR)
    {
        Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
    }
}

struct UReorderedLine
{
    UCharString reorderedText;
    Vector<int32> logicalMap;
};

class TextBoxImpl
{
public:
    UCharString utext;
    UBiDi* para = nullptr;
    Vector<UBiDi*> lines;
    Vector<UReorderedLine> reorderedLines;
};

TextBox::TextBox()
{
    pImpl.reset(new TextBoxImpl());
    UErrorCode errorCode = U_ZERO_ERROR;
    pImpl->para = ubidi_openSized(0, 0, &errorCode);
    if (pImpl->para == NULL)
    {
        Logger::Error("[TextBox::TextBox] para == nullptr", errorCode);
    }
    if (errorCode != U_ZERO_ERROR)
    {
        Logger::Error("[TextBox::TextBox] errorCode = %d", errorCode);
    }
    DVASSERT(pImpl->para);
    ubidi_setReorderingOptions(pImpl->para, UBIDI_OPTION_REMOVE_CONTROLS);
}

TextBox::TextBox(const WideString& str)
{
}

TextBox::TextBox(const TextBox& box)
{
}

TextBox::~TextBox()
{
    for (UBiDi* l : pImpl->lines)
    {
        ubidi_close(l);
    }
    if (pImpl->para)
    {
        ubidi_close(pImpl->para);
        pImpl->para = nullptr;
    }
}

void TextBox::SetText(const WideString& str)
{
    if (text != str)
    {
        text = str;

        ConvertW2U(text, pImpl->utext);
        UErrorCode errorCode = U_ZERO_ERROR;
        ubidi_setPara(pImpl->para, pImpl->utext.data(), pImpl->utext.size(), UBIDI_DEFAULT_LTR, nullptr, &errorCode);
        if (errorCode != U_ZERO_ERROR)
        {
            Logger::Error("[TextBox::SetText] errorCode = %d", errorCode);
        }
    }
}

void TextBox::SetFont(Font* _font)
{
    font = _font;
}

void TextBox::DetectDirection()
{
}

void TextBox::Shape()
{
    static const int32 shapeOptions = U_SHAPE_LENGTH_FIXED_SPACES_NEAR | U_SHAPE_LETTERS_SHAPE;

    UErrorCode errorCode = U_ZERO_ERROR;
    int32 outputLength = u_shapeArabic(pImpl->utext.data(), pImpl->utext.size(), nullptr, 0, shapeOptions, &errorCode);
    if (errorCode != U_ZERO_ERROR && errorCode != U_BUFFER_OVERFLOW_ERROR)
    {
        Logger::Error("[TextBox::Shape] errorCode = %d", errorCode);
    }

    UCharString output(outputLength, 0);
    errorCode = U_ZERO_ERROR;
    u_shapeArabic(pImpl->utext.data(), pImpl->utext.size(), &output[0], outputLength, shapeOptions, &errorCode);
    if (errorCode != U_ZERO_ERROR && errorCode != U_STRING_NOT_TERMINATED_WARNING)
    {
        Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
    }

    // RESET TEXT AND LINES
    pImpl->utext = output;
    ConvertU2W(pImpl->utext, text);
    errorCode = U_ZERO_ERROR;
    ubidi_setPara(pImpl->para, pImpl->utext.data(), pImpl->utext.size(), UBIDI_DEFAULT_LTR, nullptr, &errorCode);
    if (errorCode != U_ZERO_ERROR)
    {
        Logger::Error("[TextBox::SetText] errorCode = %d", errorCode);
    }
    pImpl->lines.clear();
}

void TextBox::Wrap(float32 maxWidth, const WrapMode mode)
{
    int32 textLength = int32(pImpl->utext.length());

    Vector<uint8> breaks;
    StringUtils::GetLineBreaks(text, breaks);
    DVASSERT_MSG(breaks.size() == textLength, "Incorrect breaks information");

    Vector<float32> characterSizes;
    font->GetStringMetrics(text, &characterSizes);
    DVASSERT_MSG(characterSizes.size() == textLength, "Incorrect character sizes information");

    if (mode != WrapMode::NO_WRAP)
    {
        ShapedLine line;
        float32 currentWidth = 0;
        uint32 lastPossibleBreak = 0;

        for (uint32 pos = 0; pos < textLength; ++pos)
        {
            char16 ch = text[pos];
            uint8 canBreak = breaks[pos];

            currentWidth += characterSizes[pos];

            // Check that targetWidth defined and currentWidth less than targetWidth.
            // If symbol is whitespace skip it and go to next (add all whitespace to current line)
            if (currentWidth <= maxWidth || StringUtils::IsWhitespace(ch))
            {
                if (canBreak == StringUtils::LB_MUSTBREAK) // If symbol is line breaker then split string
                {
                    line.length = pos - line.offset + 1;
                    currentWidth = 0.f;
                    lastPossibleBreak = 0;
                    lines.push_back(line);
                    line.offset = line.offset + line.length;
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
                line.length = pos - line.offset + 1;
                currentWidth = 0.f;
                lastPossibleBreak = 0;
                lines.push_back(line);
                line.offset = line.offset + line.length;
                continue;
            }
        }
        DVASSERT_MSG(line.offset == textLength, "Incorrect line split");
    }
    else
    {
        ShapedLine line;
        line.length = textLength;
        lines.push_back(line);
    }

    UErrorCode errorCode = U_ZERO_ERROR;
    for (ShapedLine& line : lines)
    {
        UBiDi* uline = ubidi_openSized(0, 0, &errorCode);
        if (errorCode != U_ZERO_ERROR)
        {
            Logger::Error("[TextBox::SetText] errorCode = %d", errorCode);
        }
        errorCode = U_ZERO_ERROR;
        ubidi_setLine(pImpl->para, line.offset, line.offset + line.length, uline, &errorCode);
        if (errorCode != U_ZERO_ERROR)
        {
            Logger::Error("[TextBox::SetText] errorCode = %d", errorCode);
        }
        pImpl->lines.push_back(uline);
    }
}

void TextBox::Measure()
{
}

void TextBox::Reorder(const DirectionMode mode)
{
    UErrorCode errorCode = U_ZERO_ERROR;
    pImpl->reorderedLines.clear();
    for (UBiDi* line : pImpl->lines)
    {
        errorCode = U_ZERO_ERROR;
        int32 visualLength = ubidi_writeReordered(line, nullptr, 0, UBIDI_DO_MIRRORING | UBIDI_REMOVE_BIDI_CONTROLS, &errorCode);
        if (errorCode != U_ZERO_ERROR && errorCode != U_BUFFER_OVERFLOW_ERROR)
        {
            Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
        }

        UReorderedLine rline;

        rline.reorderedText.resize(visualLength);
        errorCode = U_ZERO_ERROR;
        ubidi_writeReordered(line, &rline.reorderedText[0], rline.reorderedText.size(), UBIDI_DO_MIRRORING | UBIDI_REMOVE_BIDI_CONTROLS, &errorCode);
        if (errorCode != U_ZERO_ERROR && errorCode != U_STRING_NOT_TERMINATED_WARNING)
        {
            Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
        }
        rline.logicalMap.resize(ubidi_getLength(line));
        errorCode = U_ZERO_ERROR;
        ubidi_getLogicalMap(line, &rline.logicalMap[0], &errorCode);
        if (errorCode != U_ZERO_ERROR)
        {
            Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
        }
        pImpl->reorderedLines.push_back(rline);
    }
}

void TextBox::CleanUp()
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

const DAVA::Vector<DAVA::TextBox::ShapedLine>& TextBox::GetShapedLines() const
{
    return shapedLines;
}

const Vector<TextBox::VisualLine>& TextBox::GetVisualLines() const
{
    return visualLines;
}

const DAVA::Vector<DAVA::TextBox::VisualCharacter>& TextBox::GetCharacters() const
{
    return DAVA::Vector<DAVA::TextBox::VisualCharacter>();
}

const TextBox::VisualCharacter& TextBox::GetCharacter(int32 globalIndex) const
{
    static const VisualCharacter NONE;

    if (globalIndex < 0 || globalIndex >= logicalText.length())
    {
        return NONE;
    }

    int32 shapedIndex = log2shapedMap[globalIndex];
    int32 lineIndex, charIndex;
    ShapedIndexToLineIndex(shapedIndex, lineIndex, charIndex);

    if (lineIndex < 0 || charIndex < 0)
    {
        return NONE;
    }

    const VisualLine& vline = visualLines[lineIndex];
    int32 visIndex = vline.shaped2visMap[charIndex];
    // TODO chars;

    return NONE;
}

const TextBox::VisualCharacter& TextBox::GetCharacter(int32 lineIndex, int32 charIndex) const
{
    static const VisualCharacter NONE;
    return NONE;
}

const TextBox::ShapedLine& TextBox::GetShapedLine(int32 lineIndex) const
{
    return shapedLines[lineIndex];
}

const void TextBox::ShapedIndexToLineIndex(const int32 globalIndex, int32& lineIndex, int32& charIndex) const
{
    if (shapedLines.empty() || charIndex < 0 || charIndex > shapedText.length())
    {
        lineIndex = -1;
        charIndex = -1;
    }

    int32 linesCount = int32(shapedLines.size());
    for (int32 index = 0; index < linesCount; ++index)
    {
        const ShapedLine& sline = shapedLines[index];
        int32 limit = sline.offset + sline.length;
        if (sline.offset <= charIndex && charIndex < limit)
        {
            lineIndex = index;
            charIndex = globalIndex - sline.offset;
            return;
        }
    }
    DVASSERT(false);
}
}