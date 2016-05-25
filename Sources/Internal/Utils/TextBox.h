#pragma once

#include "Base/BaseTypes.h"
#include "Utils/BiDiHelper.h"

namespace DAVA
{
class TextBoxImpl;

class TextBox
{
public:
    enum class DirectionMode : uint8
    {
        AUTO = 0,
        WEAK_LTR,
        WEAK_RTL,
        STRONG_LTR,
        STRONG_RTL
    };

    enum class Direction : uint8
    {
        LTR = 0,
        RTL,
        MIXED
    };

    enum class WrapMode : int8
    {
        NO_WRAP = 0,
        WORD_WRAP,
        SYMBOLS_WRAP
    };

    struct Line
    {
        int32 index = 0;
        int32 start = 0;
        int32 length = 0;
        float32 xadvance = 0.f;
        float32 visiblexadvance = 0.f;
        float32 yadvance = 0.f;
        float32 xoffset = 0.f;
        float32 yoffset = 0.f;
        WideString visualString;
        bool skip = false;

        static Line EMPTY;
    };

    struct Character
    {
        int32 logicIndex = -1;
        int32 visualIndex = -1;
        int32 lineIndex = -1;
        float32 xadvance = 0.f;
        float32 xoffset = 0.f;
        bool rtl = false;
        bool skip = false;
        bool hiden = false;

        static Character EMPTY;
    };

    TextBox();
    TextBox(const TextBox& box);
    virtual ~TextBox();

    void SetText(const WideString& str, const DirectionMode mode = DirectionMode::AUTO);
    void ChangeDirectionMode(const DirectionMode mode);
    void Shape();
    void Split(const WrapMode mode = WrapMode::NO_WRAP, const Vector<uint8>* _breaks = nullptr, const Vector<float32>* _widths = nullptr, const float32 _maxWidth = 0.f);
    void Reorder();
    void Measure(const Vector<float32>& characterSizes, float32 lineHeight, int32 fromLine, int32 toLine);
    void SmartCleanUp();

    const WideString& GetText() const;
    const WideString GetProcessedText() const;
    const Direction GetBaseDirection() const;
    const Direction GetDirection() const;
    const Vector<Line>& GetLines() const;
    Vector<Line>& GetLines();
    const Line& GetLine(const int32 index) const;
    Line& GetLine(const int32 index);
    const uint32 GetLinesCount() const;
    const Vector<Character>& GetCharacters() const;
    Vector<Character>& GetCharacters();
    const Character& GetCharacter(const int32 index) const;
    Character& GetCharacter(const int32 index);
    const uint32 GetCharactersCount() const;

private:
    void ClearLines();
    void AddLineRange(int32 start, int32 length);

    WideString logicalText;
    WideString processedText;
    Vector<Line> lines;
    Vector<Character> characters;
    std::unique_ptr<TextBoxImpl> pImpl;
};

inline const DAVA::WideString& TextBox::GetText() const
{
    return logicalText;
}

inline const WideString TextBox::GetProcessedText() const
{
    return processedText;
}

inline const Vector<TextBox::Line>& TextBox::GetLines() const
{
    return lines;
}

inline Vector<TextBox::Line>& TextBox::GetLines()
{
    return lines;
}

inline const TextBox::Line& TextBox::GetLine(const int32 index) const
{
    return lines.at(index);
}

inline TextBox::Line& TextBox::GetLine(const int32 index)
{
    return lines.at(index);
}

inline const uint32 TextBox::GetLinesCount() const
{
    return uint32(lines.size());
}

inline const Vector<TextBox::Character>& TextBox::GetCharacters() const
{
    return characters;
}

inline Vector<TextBox::Character>& TextBox::GetCharacters()
{
    return characters;
}

inline const TextBox::Character& TextBox::GetCharacter(int32 index) const
{
    return characters.at(index);
}

inline TextBox::Character& TextBox::GetCharacter(const int32 index)
{
    return characters.at(index);
}

inline const uint32 TextBox::GetCharactersCount() const
{
    return uint32(characters.size());
}
}