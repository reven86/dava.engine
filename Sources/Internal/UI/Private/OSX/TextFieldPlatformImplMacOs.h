#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_MACOS__)
#if !defined(DISABLE_NATIVE_TEXTFIELD)

namespace DAVA
{
struct Rect;
class Color;
class UIGeometricData;
class ObjCWrapper;
class UITextField;
class UITextFieldDelegate;
class Window;

class TextFieldPlatformImpl
{
public:
#if defined(__DAVAENGINE_COREV2__)
    TextFieldPlatformImpl(Window* w, UITextField* uiTextField);
#else
    TextFieldPlatformImpl(UITextField* tf);
#endif
    ~TextFieldPlatformImpl();

    void Initialize()
    {
    }
    void OwnerIsDying()
    {
    }
    void SetDelegate(UITextFieldDelegate*)
    {
    }

    void OpenKeyboard();
    void CloseKeyboard();
    void GetText(WideString& string) const;
    void SetText(const WideString& string);
    void UpdateRect(const Rect& rect);

    void SetTextColor(const Color& color);
    void SetFontSize(float size);

    void SetTextAlign(int32 align);
    int32 GetTextAlign();
    void SetTextUseRtlAlign(bool useRtlAlign);
    bool GetTextUseRtlAlign() const;

    void SetVisible(bool value);
    void ShowField();
    void HideField();

    void SetIsPassword(bool isPassword);

    void SetInputEnabled(bool value);

    // Keyboard traits.
    void SetAutoCapitalizationType(int32 value);
    void SetAutoCorrectionType(int32 value);
    void SetSpellCheckingType(int32 value);
    void SetKeyboardAppearanceType(int32 value);
    void SetKeyboardType(int32 value);
    void SetReturnKeyType(int32 value);
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

#endif //!DISABLE_NATIVE_TEXTFIELD
#endif // __DAVAENGINE_MACOS__
