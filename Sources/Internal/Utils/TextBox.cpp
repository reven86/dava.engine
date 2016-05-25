#include "TextBox.h"
#include "Utils/StringUtils.h"

#define U_COMMON_IMPLEMENTATION
#define U_STATIC_IMPLEMENTATION
#if __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wheader-hygiene"
#pragma clang diagnostic ignored "-Wold-style-cast"
#endif
#include <unicode/ubidi.h>
#include <unicode/ushape.h>
#if __clang__
#pragma clang diagnostic pop
#endif

#if defined(__DAVAENGINE_WIN32__)
static_assert(sizeof(DAVA::WideString::value_type) == sizeof(UChar), "Check size of wchar_t and UChar");
#else
static_assert(sizeof(DAVA::WideString::value_type) == 4, "Check size of wchar_t equal 4");
static_assert(sizeof(UChar) == 2, "Check size of UChar equal 2");
#include <utf8.h>
#endif

namespace DAVA
{
using UCharString = BasicString<UChar>;

class TextBoxImpl
{
public:
    TextBoxImpl(TextBox* _tb);
    TextBoxImpl(TextBox* _tb, const TextBoxImpl& src);
    TextBoxImpl(const TextBoxImpl& src) = delete;
    virtual ~TextBoxImpl();

    void SetWString(const WideString& str, const TextBox::DirectionMode mode);
    void ChangeDirectionMode(const TextBox::DirectionMode mode);
    void UpdatePara();
    void Shape();
    void ReorderLines();
    const WideString AsWString() const;
    const TextBox::Direction GetDirection() const;
    const TextBox::Direction GetBaseDirection() const;

private:
    static UBiDiLevel DirectionModeToBiDiLevel(const TextBox::DirectionMode mode);
    static TextBox::Direction BiDiDirectionToDirection(const UBiDiDirection dir);
    static WideString ConvertU2W(const UCharString& src);
    static UCharString ConvertW2U(const WideString& src);

    UCharString paraText;
    UBiDi* para = nullptr;
    TextBox* tb = nullptr;
    UBiDiLevel baseLevel = UBIDI_DEFAULT_LTR;
};

TextBoxImpl::TextBoxImpl(TextBox* _tb)
    : tb(_tb)
{
    para = ubidi_open();
    DVASSERT_MSG(para != nullptr, "Can't alocate new paragraph");
}

TextBoxImpl::TextBoxImpl(TextBox* _tb, const TextBoxImpl& src)
    : tb(_tb)
{
    para = ubidi_open();
    DVASSERT_MSG(para != nullptr, "Can't alocate new paragraph");
    paraText = src.paraText;
    baseLevel = src.baseLevel;
    UpdatePara();
}

TextBoxImpl::~TextBoxImpl()
{
    if (para != nullptr)
    {
        ubidi_close(para);
    }
}

void TextBoxImpl::SetWString(const WideString& str, const TextBox::DirectionMode mode)
{
    paraText = ConvertW2U(str);
    baseLevel = DirectionModeToBiDiLevel(mode);
    UpdatePara();
}

void TextBoxImpl::ChangeDirectionMode(const TextBox::DirectionMode mode)
{
    baseLevel = DirectionModeToBiDiLevel(mode);
    UpdatePara();
}

void TextBoxImpl::UpdatePara()
{
    UErrorCode errorCode = U_ZERO_ERROR;
    ubidi_setPara(para, paraText.data(), paraText.length(), baseLevel, nullptr, &errorCode);
    if (errorCode != U_ZERO_ERROR)
    {
        Logger::Error("[TextBox::SetText] errorCode = %d", errorCode);
    }
}

void TextBoxImpl::Shape()
{
    // Commented options are extended options from ICU 3.6 and 4.2
    // TODO: uncomment for more complex shaping
    uint32 commonOptions = U_SHAPE_LETTERS_SHAPE /*| U_SHAPE_AGGREGATE_TASHKEEL | U_SHAPE_PRESERVE_PRESENTATION*/;
    uint32 detectLengthOptions = commonOptions | U_SHAPE_LAMALEF_RESIZE;
    uint32 shapeOptions = commonOptions | U_SHAPE_LAMALEF_NEAR /*| U_SHAPE_SEEN_TWOCELL_NEAR | U_SHAPE_YEHHAMZA_TWOCELL_NEAR | U_SHAPE_TASHKEEL_REPLACE_BY_TATWEEL*/;

    const UChar* text = ubidi_getText(para);
    const int32 length = ubidi_getLength(para);

    UErrorCode errorCode = U_ZERO_ERROR;
    UCharString output(length, 0);
    u_shapeArabic(text, length, const_cast<UChar*>(output.data()), length, shapeOptions, &errorCode);
    if (errorCode != U_ZERO_ERROR && errorCode != U_STRING_NOT_TERMINATED_WARNING)
    {
        Logger::Error("[TextBox::Shape] shapeArabic2 errorCode = %d", errorCode);
        DVASSERT_MSG(false, "[TextBox::Shape] shapeArabic2 errorCode");
    }

    // Check new shaped length
    errorCode = U_ZERO_ERROR;
    int32 outputLength = u_shapeArabic(text, length, nullptr, 0, detectLengthOptions, &errorCode);
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
            TextBox::Character& c = tb->GetCharacter(i);
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

const DAVA::WideString TextBoxImpl::AsWString() const
{
    return ConvertU2W(paraText);
}

void TextBoxImpl::ReorderLines()
{
    Vector<int32> l2v;
    UErrorCode errorCode = U_ZERO_ERROR;
    UBiDi* lpara = ubidi_open();
    for (TextBox::Line& line : tb->GetLines())
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
        errorCode = U_ZERO_ERROR;
        UCharString visString(line.length, 0);
        ubidi_writeReordered(lpara, const_cast<UChar*>(visString.data()), line.length, UBIDI_DO_MIRRORING | UBIDI_REMOVE_BIDI_CONTROLS, &errorCode);
        if (errorCode != U_ZERO_ERROR && errorCode != U_STRING_NOT_TERMINATED_WARNING)
        {
            Logger::Error("[TextBox::ReorderLines] writeReordered errorCode = %d", errorCode);
            DVASSERT_MSG(false, "[TextBox::ReorderLines] writeReordered errorCode");
        }

        line.visualString = ConvertU2W(visString);

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
            TextBox::Character& c = tb->GetCharacter(li++);
            c.visualIndex = l2v[i] + line.start; // Make local index global (text)
            c.rtl = (ubidi_getLevelAt(lpara, i) & UBIDI_RTL) == UBIDI_RTL;
        }
    }
    ubidi_close(lpara);
}

const DAVA::TextBox::Direction TextBoxImpl::GetDirection() const
{
    return BiDiDirectionToDirection(ubidi_getDirection(para));
}

const DAVA::TextBox::Direction TextBoxImpl::GetBaseDirection() const
{
    return BiDiDirectionToDirection(ubidi_getBaseDirection(ubidi_getText(para), ubidi_getLength(para)));
}

UBiDiLevel TextBoxImpl::DirectionModeToBiDiLevel(const TextBox::DirectionMode mode)
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

DAVA::TextBox::Direction TextBoxImpl::BiDiDirectionToDirection(const UBiDiDirection dir)
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

WideString TextBoxImpl::ConvertU2W(const UCharString& src)
{
    if (src.empty())
    {
        return WideString();
    }
#if defined(__DAVAENGINE_WIN32__)
    return WideString(src);
#else
    String tmp;
    tmp.reserve(src.length());
    utf8::utf16to8(src.begin(), src.end(), std::back_inserter(tmp));
    WideString result;
    return.reserve(tmp.length());
    utf8::utf8to32(tmp.begin(), tmp.end(), std::back_inserter(result));
    return result;
#endif
}

UCharString TextBoxImpl::ConvertW2U(const WideString& src)
{
    if (src.empty())
    {
        return UCharString();
    }
#if defined(__DAVAENGINE_WIN32__)
    return UCharString(src);
#else
    String tmp;
    tmp.reserve(src.length());
    utf8::utf32to8(src.begin(), src.end(), std::back_inserter(tmp));
    UCharString result;
    return.reserve(tmp.length());
    utf8::utf8to16(tmp.begin(), tmp.end(), std::back_inserter(result));
    return result;
#endif
}

/******************************************************************************/
/******************************************************************************/

TextBox::Line TextBox::Line::EMPTY = TextBox::Line();
TextBox::Character TextBox::Character::EMPTY = TextBox::Character();

TextBox::TextBox()
{
    pImpl.reset(new TextBoxImpl(this));
}

TextBox::TextBox(const TextBox& box)
{
    logicalText = box.logicalText;
    processedText = box.processedText;
    lines = box.lines;
    characters = box.characters;
    pImpl.reset(new TextBoxImpl(this, *box.pImpl));
}

TextBox::~TextBox()
{
}

void TextBox::ClearLines()
{
    lines.clear();
}

void TextBox::AddLineRange(int32 start, int32 length)
{
    TextBox::Line l;
    l.index = lines.size();
    l.start = start;
    l.length = length;
    l.visualString = processedText.substr(start, length);
    lines.push_back(l);

    // Store line index in character
    int32 limit = start + length;
    for (int32 i = start; i < limit; ++i)
    {
        characters[i].lineIndex = l.index;
    }
}

void TextBox::SetText(const WideString& str, const DirectionMode mode)
{
    logicalText = str;
    processedText = str;

    uint32 length = uint32(str.length());
    characters.resize(length);
    for (uint32 i = 0; i < length; ++i)
    {
        Character& c = characters[i];
        c = Character::EMPTY;
        c.logicIndex = i;
        c.visualIndex = i;
    }

    ClearLines();
    AddLineRange(0, length);

    pImpl->SetWString(logicalText, mode);
}

void TextBox::ChangeDirectionMode(const DirectionMode mode)
{
    pImpl->ChangeDirectionMode(mode);
}

void TextBox::Shape()
{
    pImpl->Shape();
    processedText = pImpl->AsWString();
}

void TextBox::Split(const WrapMode mode, const Vector<uint8>* breaks, const Vector<float32>* widths, const float32 maxWidth)
{
    ClearLines();
    if (mode == WrapMode::NO_WRAP)
    {
        AddLineRange(0, processedText.length());
    }
    else
    {
        DVASSERT_MSG(breaks != nullptr, "Breaks information is nullptr");
        DVASSERT_MSG(widths != nullptr, "Characters widths information is nullptr");
        DVASSERT_MSG(breaks->size() == processedText.length(), "Incorrect breaks information");
        DVASSERT_MSG(widths->size() == processedText.length(), "Incorrect character sizes information");

        float32 currentWidth = 0;
        uint32 lastPossibleBreak = 0;
        uint32 lineStart = 0;
        uint32 lineLength = 0;
        uint32 textLength = uint32(processedText.length());

        auto addLine = [&](const uint32 pos) {
            currentWidth = 0.f;
            lastPossibleBreak = 0;
            lineLength = pos - lineStart + 1;
            AddLineRange(lineStart, lineLength);
            lineStart += lineLength;
        };

        for (uint32 pos = lineStart; pos < textLength; ++pos)
        {
            uint8 canBreak = breaks->at(pos);
            currentWidth += widths->at(pos);

            // Check that targetWidth defined and currentWidth less than targetWidth.
            // If symbol is whitespace skip it and go to next (add all whitespace to current line)
            if (currentWidth <= maxWidth || StringUtils::IsWhitespace(char16(processedText.at(pos))))
            {
                if (canBreak == StringUtils::LB_MUSTBREAK) // If symbol is line breaker then split string
                {
                    addLine(pos);
                    continue;
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
            }
            else if (pos > lineStart) // Too big single word in line, split word by symbol
            {
                pos--;
            }
            addLine(pos);
        }
    }
}

void TextBox::Reorder()
{
    if (lines.empty())
    {
        return;
    }

    pImpl->ReorderLines();
}

void TextBox::SmartCleanUp()
{
    Set<int32> removeLiterals;
    for (TextBox::Line& line : lines)
    {
        int32 limit = line.start + line.length;

        // Detect trimmed whitespace and hide it
        for (int32 li = limit - 1; li >= line.start; --li)
        {
            Character& ch = GetCharacter(li);
            if (StringUtils::IsWhitespace(processedText.at(li)))
            {
                ch.hiden = true;
                continue;
            }
            break;
        }

        // Store all visual indices for erasing
        removeLiterals.clear();
        for (int32 li = line.start; li < limit; ++li)
        {
            Character& ch = GetCharacter(li);
            if (ch.skip || ch.hiden)
            {
                int32 vi = characters[li].visualIndex - line.start; // Make global index local (line)
                removeLiterals.insert(vi);
                //line.visualString.erase(vi, 1);
            }
        }

        // Erase set of literal indeces from visual string
        auto endReverseIt = removeLiterals.crend();
        for (auto reverseIt = removeLiterals.crbegin(); reverseIt != endReverseIt; ++reverseIt)
        {
            line.visualString.erase(*reverseIt, 1);
        }
    }
}

void TextBox::Measure(const Vector<float32>& characterSizes, float32 lineHeight, int32 fromLine, int32 linesCount)
{
    DVASSERT_MSG(characterSizes.size() == processedText.size(), "Incorrect character sizes information");

    int32 lineLimit = fromLine + linesCount;
    int32 linesSize = int32(lines.size());
    for (int32 lineIndex = 0; lineIndex < linesSize; ++lineIndex)
    {
        Line& line = lines.at(lineIndex);
        if (lineIndex < fromLine || lineIndex >= lineLimit)
        {
            line.skip = true;
            continue;
        }

        line.skip = false;
        line.yoffset = (lineIndex - fromLine) * lineHeight;
        line.yadvance = lineHeight;
        line.xoffset = 0.f;
        line.xadvance = 0.f;
        line.visiblexadvance = 0.f;

        // Generate visual order
        Vector<int32> vo(line.length, -1);
        int32 limit = line.start + line.length;
        for (int32 li = line.start; li < limit; ++li)
        {
            const Character& c = characters.at(li);
            int32 vi = c.visualIndex - line.start; // Make global index local (line)
            vo[vi] = c.logicIndex;
        }

        // Layout
        for (int32 logInd : vo)
        {
            Character& c = characters.at(logInd);
            if (c.skip)
            {
                continue;
            }

            c.xoffset = line.xadvance;
            c.xadvance = characterSizes[logInd];

            if (line.xoffset > c.xoffset)
            {
                line.xoffset = c.xoffset;
            }
            line.xadvance += c.xadvance;
            if (!c.hiden)
            {
                line.visiblexadvance += c.xadvance;
            }
        }
    }
}

const DAVA::TextBox::Direction TextBox::GetDirection() const
{
    return pImpl->GetDirection();
}

const TextBox::Direction TextBox::GetBaseDirection() const
{
    return pImpl->GetBaseDirection();
}
}