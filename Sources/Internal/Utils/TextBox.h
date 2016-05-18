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
        FORCE_LTR,
        FORCT_RTL
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

    struct ShapedLine
    {
        int32 offset = 0;
        int32 length = 0;
    };

    struct VisualLine
    {
        WideString visualString;
        Vector<int32> shaped2visMap;
        float32 xadvance = 0.f;
        float32 trimxadvance = 0.f;
        float32 yadvance = 0.f;
        float32 trimyadvance = 0.f;
        float32 xoffset = 0.f;
        float32 yoffset = 0.f;
    };

    struct VisualCharacter
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
    TextBox(const WideString& str);
    TextBox(const TextBox& box);
    virtual ~TextBox();

    void SetText(const WideString& str);
    void SetFont(Font* font);
    void DetectDirection();
    void Shape();
    void Wrap(float32 maxWidth, const WrapMode mode = WrapMode::WORD_WRAP);
    void Measure();
    void Reorder(const DirectionMode mode = DirectionMode::AUTO);
    void CleanUp();

    const Direction GetDirection() const;
    const WideString& GetText() const;
    const Vector<ShapedLine>& GetShapedLines() const;
    const Vector<VisualLine>& GetVisualLines() const;
    const Vector<VisualCharacter>& GetCharacters() const;
    const VisualCharacter& GetCharacter(const int32 globalIndex) const;
    const VisualCharacter& GetCharacter(const int32 lineIndex, const int32 charIndex) const;
    const ShapedLine& GetShapedLine(const int32 lineIndex) const;

private:
    const void ShapedIndexToLineIndex(const int32 globalIndex, int32& lineIndex, int32& charIndex) const;

    Font* font = nullptr;
    WideString logicalText;
    WideString shapedText; // it can be different with logicalText
    Vector<int32> log2shapedMap; // mapping logical characters with shaping characters
    Vector<ShapedLine> shapedLines;
    Vector<VisualLine> visualLines;
    Direction direction = Direction::LTR;
    std::unique_ptr<TextBoxImpl> pImpl;
};
}