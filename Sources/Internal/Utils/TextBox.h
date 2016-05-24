#pragma once

#include "Base/BaseTypes.h"
#include "Utils/BiDiHelper.h"

namespace DAVA
{
class TextBoxImpl;

class TextBox
{
public:
    class Splitter
    {
    public:
        virtual ~Splitter() = default;
        virtual uint32 split(const WideString& str, const uint32 from) = 0;
    };

    class EmptySplitter : public Splitter
    {
    public:
        uint32 split(const WideString& str, const uint32 from) override;
    };

    class SimpleSplitter : public Splitter
    {
    public:
        uint32 split(const WideString& str, const uint32 from) override;
    };

    class SmartSplitter : public Splitter
    {
    public:
        SmartSplitter(Vector<uint8>* breaks, Vector<float32>* widths, const float32 maxWidth = FLT_MAX, const bool splitBySymbols = false);
        uint32 split(const WideString& str, const uint32 from) override;

    private:
        Vector<uint8>* breaks = nullptr;
        Vector<float32>* widths = nullptr;
        float32 maxWidth = 0.f;
        bool splitBySymbols = false;
    };

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
        int32 logicIndex = -1;
        int32 visualIndex = -1;
        int32 lineIndex = -1;
        float32 xadvance = 0.f;
        float32 yadvance = 0.f;
        float32 xoffset = 0.f;
        float32 yoffset = 0.f;
        bool rtl = false;
        bool skip = false;
    };

    TextBox();
    TextBox(const TextBox& box);
    virtual ~TextBox();

    void SetText(const WideString& str, const DirectionMode mode = DirectionMode::AUTO);
    void ChangeDirectionMode(const DirectionMode mode);
    void Shape();
    void Split(Splitter& splitter);
    void Reorder();
    void Measure(const Vector<float32>& characterSizes, float32 lineHeight, int32 fromLine, int32 toLine);
    void SmartCleanUp();

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
    std::unique_ptr<TextBoxImpl> pImpl;
};
}