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
        float32 trimxadvance = 0.f;
        float32 yadvance = 0.f;
        float32 trimyadvance = 0.f;
        float32 xoffset = 0.f;
        float32 yoffset = 0.f;
        WideString visualString;
    };

    struct Character
    {
        int32 codepoint = -1;
        int32 textIndex = -1;
        int32 lineIndex = -1;
        float32 xadvance = 0.f;
        float32 yadvance = 0.f;
        float32 xoffset = 0.f;
        float32 yoffset = 0.f;
    };

    TextBox();
    TextBox(const TextBox& box);
    virtual ~TextBox();

    void SetText(const WideString& str, const DirectionMode mode = DirectionMode::AUTO);
    void SetFont(Font* font);
    void Shape();
    void Wrap(float32 maxWidth, const WrapMode mode = WrapMode::WORD_WRAP);
    void Reorder();

    void Measure();
    void CleanUp();

    const WideString& GetText() const;
    const Direction GetDirection() const;
    const Vector<Line>& GetLines() const;
    const Line& GetLine(const int32 index) const;
    const uint32 GetLinesCount() const;
    const Vector<Character>& GetCharacters() const;
    const Character& GetCharacter(const int32 index) const;
    const uint32 GetCharactersCount() const;

private:
    Font* font = nullptr;
    WideString logicalText;
    Vector<Line> lines;
    Vector<Character> characters;
    Direction direction = Direction::LTR;
    std::unique_ptr<TextBoxImpl> pImpl;
};
}