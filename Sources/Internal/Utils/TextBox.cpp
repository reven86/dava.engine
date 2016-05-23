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
        static const int32 shapeOptions = /*U_SHAPE_LENGTH_FIXED_SPACES_NEAR |*/ U_SHAPE_LETTERS_SHAPE;

        const UChar* text = ubidi_getText(para);
        const int32 length = ubidi_getLength(para);
        UErrorCode errorCode = U_ZERO_ERROR;
        int32 outputLength = u_shapeArabic(text, length, nullptr, 0, shapeOptions, &errorCode);
        if (errorCode != U_ZERO_ERROR && errorCode != U_BUFFER_OVERFLOW_ERROR)
        {
            Logger::Error("[TextBox::Shape] errorCode = %d", errorCode);
        }

        UCharString output(outputLength, 0);
        errorCode = U_ZERO_ERROR;
        u_shapeArabic(text, length, const_cast<UChar*>(output.data()), outputLength, shapeOptions, &errorCode);
        if (errorCode != U_ZERO_ERROR && errorCode != U_STRING_NOT_TERMINATED_WARNING)
        {
            Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
        }

        if (length > outputLength)
        {
            // Fix indeces mapping if length of result string reduced
#if 0 // Direct mapping 
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
            for (int32 vi = 0; vi < outputLength; ++vi)
            {
                TextBox::Character& c = characters[li++];
                c.shapedIndex = vi;
                c.visualIndex = vi;

                if (isLamAlefChar(output.at(vi)))
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
        l.offset = start;
        l.length = length;
        lines.push_back(l);
    }

    void ReorderLines()
    {
        const static int32 reorderingOptions = UBIDI_DO_MIRRORING | UBIDI_REMOVE_BIDI_CONTROLS;
        Vector<int32> l2v;
        UErrorCode errorCode = U_ZERO_ERROR;
        UBiDi* lpara = ubidi_open();
        for (TextBox::Line& line : lines)
        {
            errorCode = U_ZERO_ERROR;
            ubidi_setLine(para, line.offset, line.offset + line.length, lpara, &errorCode);
            if (errorCode != U_ZERO_ERROR)
            {
                Logger::Error("[TextBox::SetText] errorCode = %d", errorCode);
            }

            // Write reordered string
            errorCode = U_ZERO_ERROR;
            int32 visualLength = ubidi_writeReordered(lpara, nullptr, 0, reorderingOptions, &errorCode);
            if (errorCode != U_ZERO_ERROR && errorCode != U_BUFFER_OVERFLOW_ERROR)
            {
                Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
            }
            errorCode = U_ZERO_ERROR;
            UCharString visString(visualLength, 0);
            ubidi_writeReordered(lpara, const_cast<UChar*>(visString.data()), visualLength, reorderingOptions, &errorCode);
            if (errorCode != U_ZERO_ERROR && errorCode != U_STRING_NOT_TERMINATED_WARNING)
            {
                Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
            }

            line.visualString = ConvertU2W(visString.data(), visualLength);

            // Get local reordered characters map
            int32 length = ubidi_getLength(lpara);
            l2v.resize(length);
            errorCode = U_ZERO_ERROR;
            ubidi_getLogicalMap(lpara, l2v.data(), &errorCode);
            if (errorCode != U_ZERO_ERROR)
            {
                Logger::Error("[BiDiImpl::Reorder] errorCode = %d", errorCode);
            }

            // Correct global reordered characters map according local map
            int32 li = line.offset;
            for (int32 i = 0; i < length; ++i)
            {
                TextBox::Character* c = &characters[li++];
                if (c->shapedIndex < 0)
                {
                    c = &characters[li++];
                }
                int32 vi = line.offset + l2v[i];
                c->visualIndex = vi;
            }
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
        pImpl->ClearLines();
        pImpl->SetWString(logicalText, mode);
        direction = pImpl->GetDirection();
    }
}

void TextBox::ChangeDirectionMode(const DirectionMode mode)
{
    pImpl->ChangeDirectionMode(mode);
}

void TextBox::Shape()
{
    pImpl->Shape();
    logicalText = pImpl->AsWideString();
}

void TextBox::Wrap(const WrapMode mode, float32 maxWidth, const Vector<float32>* characterSizes, const Vector<uint8>* breaks)
{
    uint32 textLength = uint32(pImpl->GetLength());
    pImpl->ClearLines();

    // Wrap
    if (mode != WrapMode::NO_WRAP)
    {
        if (characterSizes == nullptr || breaks == nullptr)
        {
            return;
        }

        DVASSERT_MSG(breaks->size() == textLength, "Incorrect breaks information");
        DVASSERT_MSG(characterSizes->size() == textLength, "Incorrect character sizes information");

        const UCharString& shapedText = pImpl->AsUCharString();
        int32 lineOffset = 0;
        int32 lineLength = 0;
        float32 currentWidth = 0;
        uint32 lastPossibleBreak = 0;
        for (uint32 pos = 0; pos < textLength; ++pos)
        {
            UChar ch = shapedText[pos];
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
    int32 visualIndex = pImpl->characters.at(index).visualIndex;
    if (visualIndex == -1)
    {
        visualIndex = pImpl->characters.at(index - 1).visualIndex;
    }
    return pImpl->characters.at(visualIndex);
}

const uint32 TextBox::GetCharactersCount() const
{
    return uint32(pImpl->characters.size());
}

}