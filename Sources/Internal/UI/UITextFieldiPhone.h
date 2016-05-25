#ifndef __DAVAENGINE_UI_TEXT_FIELD_IPHONE_H__
#define __DAVAENGINE_UI_TEXT_FIELD_IPHONE_H__

#include "UI/UITextField.h"

namespace DAVA
{
class UITextField;

class TextFieldPlatformImpl
{
public:
    TextFieldPlatformImpl(UITextField* tf);
    virtual ~TextFieldPlatformImpl();

    void OpenKeyboard();
    void CloseKeyboard();
    void GetText(WideString& string) const;
    void SetText(const WideString& string);
    void UpdateRect(const Rect& rect);

    void SetTextColor(const DAVA::Color& color);
    void SetFontSize(float size);

    void SetTextAlign(DAVA::int32 align);
    DAVA::int32 GetTextAlign();
    void SetTextUseRtlAlign(bool useRtlAlign);
    bool GetTextUseRtlAlign() const;

    void SetVisible(bool value);
    void ShowField();
    void HideField();

    void SetIsPassword(bool isPassword);

    void SetInputEnabled(bool value);

    // Keyboard traits.
    void SetAutoCapitalizationType(DAVA::int32 value);
    void SetAutoCorrectionType(DAVA::int32 value);
    void SetSpellCheckingType(DAVA::int32 value);
    void SetKeyboardAppearanceType(DAVA::int32 value);
    void SetKeyboardType(DAVA::int32 value);
    void SetReturnKeyType(DAVA::int32 value);
    void SetEnableReturnKeyAutomatically(bool value);

    // Cursor pos.
    uint32 GetCursorPos();
    void SetCursorPos(uint32 pos);

    // Max text length.
    void SetMaxLength(int maxLength);

    void SetMultiline(bool multiline);

    void SetRenderToTexture(bool value);
    bool IsRenderToTexture() const;
    void SystemDraw(const UIGeometricData& geometricData);

private:
    // Truncate the text to maxLength characters.
    void* TruncateText(void* text, int maxLength);
    void UpdateStaticTexture();
    void UpdateNativeRect(const Rect& virtualRect, int xOffset);

    Rect nextRect;
    Rect prevRect;
    UITextField& davaTextField;
    void* objcClassPtr;
    bool renderToTexture;
    bool isSingleLine = true;
    int deltaMoveControl = 0;
    bool isNeedToUpdateTexture = false;
};
};

#endif // __DAVAENGINE_UI_TEXT_FIELD_IPHONE_H__
