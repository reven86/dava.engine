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

    class Line
    {
    public:
        virtual ~Line()
        {
        }

        int32 index = 0;
        int32 offset = 0;
        int32 length = 0;
        float32 xadvance = 0.f;
        float32 trimxadvance = 0.f;
        float32 yadvance = 0.f;
        float32 trimyadvance = 0.f;
        float32 xoffset = 0.f;
        float32 yoffset = 0.f;
        WideString visualString;
    };

    class Character
    {
    public:
        int32 codepoint = -1;
        int32 logicIndex = -1;
        int32 shapedIndex = -1;
        int32 visualIndex = -1;
        float32 xadvance = 0.f;
        float32 yadvance = 0.f;
        float32 xoffset = 0.f;
        float32 yoffset = 0.f;
    };

    TextBox();
    TextBox(const TextBox& box);
    virtual ~TextBox();

    void SetText(const WideString& str, const DirectionMode mode = DirectionMode::AUTO);
    void ChangeDirectionMode(const DirectionMode mode);
    void Shape();
    void Wrap(const WrapMode mode = WrapMode::NO_WRAP, float32 maxWidth = 0.f, const Vector<float32>* characterSizes = nullptr, const Vector<uint8>* breaks = nullptr);
    void Reorder();

    void Measure();
    void CleanUp();

    const WideString& GetText() const;
    const WideString GetShapedText() const;
    const Direction GetBaseDirection() const;
    const Direction GetDirection() const;
    const Vector<Line>& GetLines() const;
    const Line& GetLine(const int32 index) const;
    const uint32 GetLinesCount() const;
    const Vector<Character>& GetCharacters() const;
    const Character& GetCharacter(const int32 index) const;
    const uint32 GetCharactersCount() const;

private:
    WideString logicalText;
    Direction direction = Direction::LTR;
    std::unique_ptr<TextBoxImpl> pImpl;
};
}