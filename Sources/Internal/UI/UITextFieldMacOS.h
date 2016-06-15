#ifndef __DAVAENGINE_UI_TEXT_FIELD_MAC_OS_H__
#define __DAVAENGINE_UI_TEXT_FIELD_MAC_OS_H__

#include "UI/UITextField.h"
#include <memory>

namespace DAVA
{
class ObjCWrapper;

class TextFieldPlatformImpl
{
public:
    explicit TextFieldPlatformImpl(UITextField* tf);
    ~TextFieldPlatformImpl();

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
    void SystemDraw(const UIGeometricData&)
    {
    }

private:
    ObjCWrapper& objcWrapper;
};
} // end namespace DAVA

#endif // __DAVAENGINE_UI_TEXT_FIELD_MAC_OS_H__