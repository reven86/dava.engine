#ifndef __DAVA_UITEXTFIELDSTB_H__
#define __DAVA_UITEXTFIELDSTB_H__

#include "Base/BaseTypes.h"
#include "Render/2D/TextBlock.h"
#include "UI/Private/StbTextEditBridge.h"

namespace DAVA
{
class UITextField;
class UIStaticText;
class UIGeometricData;
class Font;
class Color;
class UIEvent;
class Vector2;
struct Rect;

// This implementation simulate iOS/Android native controls,
// so no hierarchy for internal UIStaticText, and call UpdateRect
// every frame, and render directly in SyctemDraw. This helps
// to find similar bugs in all implementations
class TextFieldStbImpl : public StbTextEditBridge::StbTextDelegate
{
public:
    friend class UITextField;
    TextFieldStbImpl(UITextField* control);
    ~TextFieldStbImpl();
    void CopyDataFrom(TextFieldStbImpl* t);
    void OpenKeyboard();
    void CloseKeyboard();
    void SetRenderToTexture(bool);
    void SetIsPassword(bool);
    void SetFontSize(float32);
    void SetText(const WideString& text);
    void UpdateRect(const Rect&);
    void SetAutoCapitalizationType(int32);
    void SetAutoCorrectionType(int32);
    void SetSpellCheckingType(int32);
    void SetKeyboardAppearanceType(int32);
    void SetKeyboardType(int32);
    void SetReturnKeyType(int32);
    void SetEnableReturnKeyAutomatically(int32);
    bool IsRenderToTexture() const;
    uint32 GetCursorPos() const;
    void SetCursorPos(int32);
    void SetMaxLength(int32);
    void GetText(WideString&);
    void SetInputEnabled(bool, bool hierarchic = true);
    void SetVisible(bool v);
    void SetFont(Font* f);
    Font* GetFont() const;
    void SetTextColor(const Color& c);
    void SetShadowOffset(const Vector2& v);
    void SetShadowColor(const Color& c);
    void SetTextAlign(int32 align);
    TextBlock::eUseRtlAlign GetTextUseRtlAlign();
    void SetTextUseRtlAlign(TextBlock::eUseRtlAlign align);
    void SetSize(const Vector2 vector2);
    void SetMultiline(bool is_multiline);
    Color GetTextColor();
    Vector2 GetShadowOffset();
    Color GetShadowColor();
    int32 GetTextAlign();
    void SetRect(const Rect& rect);
    void SystemDraw(const UIGeometricData& d);
    void SetSelectionColor(const Color& selectionColor);
    const Color& GetSelectionColor() const;
    void Input(UIEvent* currentInput);

    void SelectAll();
    bool CutToClipboard();
    bool CopyToClipboard();
    bool PasteFromClipboard();

    // StbTextEditBridge::StbTextDelegate
    uint32 InsertText(uint32 position, const WideString::value_type* str, uint32 length) override;
    uint32 DeleteText(uint32 position, uint32 length) override;
    const Vector<TextBlock::Line>& GetMultilineInfo() override;
    const Vector<float32>& GetCharactersSizes() override;
    uint32 GetTextLength() override;
    WideString::value_type GetCharAt(uint32 i) override;

private:
    void UpdateSelection(uint32 start, uint32 end);
    void UpdateCursor(uint32 cursorPos, bool insertMode);

    UIStaticText* staticText = nullptr; // Control for displaying text
    UITextField* control = nullptr; // Weak link to parent text field
    StbTextEditBridge* stb = nullptr;
    WideString text;
    float32 cursorTime = 0.0f;
    int32 maxLength;
    bool needRedraw = true;
    bool showCursor = true;
    bool isEditing = false;
    bool ignoreKeyPressedDelegate = false;
    Color selectionColor = Color(0.f, 0.f, 0.7f, 0.7f);
    Vector<Rect> selectionRects;
    Rect cursorRect;
    Vector2 staticTextOffset;
    void UpdateOffset(const Rect& visibleRect);
};

} // end namespace DAVA

#endif //__DAVA_UITEXTFIELDSTB_H__