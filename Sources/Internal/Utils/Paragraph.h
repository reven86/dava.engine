namespace DAVA
{
struct Character
{
    int32 index = -1;
    int32 codepoint = -1;
    Rect glyphRect;
};

struct Run
{
    int32 offset = 0;
    int32 length = 0;
    int32 direction = -1;
    int32 script = -1;
    Rect runRect;
};

struct Line
{
    int32 offset = 0;
    int32 length = 0;
    Rect lineRect;
};

class Paragraph
{
public:
    Paragraph();
    Paragraph(const WideString& str);
    Paragraph(const Paragraph& para);
    virtual ~Paragraph();

    bool isRtl() const;

    void SetText(const WideString& str);
    void Shape();
    void Wrap(const Font* font, float32 maxWidth);
    void Reorder(bool forceRtl = false);
    void CleanUp();

    const WideString& GetRawText() const;
    const WideString& GetText() const;
    const Vector<Line>& GetLines() const;
    const Vector<WideString> GetLinesAsWideStrings() const;
    const int32 GetLogic2Visual(const int32 logicIndex) const;
    const int32 GetVisual2Logic(const int32 visualIndex) const;
    const WideString GetSubstring(const int32 offset, const int32 length) const;
    const WideString GetSubstring(const int32 from, const int32 to) const;
};
}