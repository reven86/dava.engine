#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_ANDROID__)
#if defined(__DAVAENGINE_COREV2__)

namespace DAVA
{
class Window;
class UITextField;
class UITextFieldDelegate;
class TextFieldControlImpl;

struct Rect;
class Color;
class UIGeometricData;

class TextFieldPlatformImpl
{
public:
    TextFieldPlatformImpl(Window& w, UITextField& uiTextField);
    ~TextFieldPlatformImpl();

    void SetVisible(bool isVisible);
    void SetIsPassword(bool isPassword);
    void SetMaxLength(int32 value);

    void OpenKeyboard();
    void CloseKeyboard();

    void UpdateRect(const Rect& rect);

    void SetText(const WideString& text);
    void GetText(WideString& text) const;

    void SetTextColor(const Color& color);
    void SetTextAlign(int32 align);
    int32 GetTextAlign() const;
    void SetTextUseRtlAlign(bool useRtlAlign);
    bool GetTextUseRtlAlign() const;

    void SetFontSize(float32 size);

    void SetDelegate(UITextFieldDelegate* textFieldDelegate);

    void SetMultiline(bool value);

    void SetInputEnabled(bool value);

    void SetRenderToTexture(bool value);
    bool IsRenderToTexture() const;

    void SetAutoCapitalizationType(int32 value);
    void SetAutoCorrectionType(int32 value);
    void SetSpellCheckingType(int32 value);
    void SetKeyboardAppearanceType(int32 value);
    void SetKeyboardType(int32 value);
    void SetReturnKeyType(int32 value);
    void SetEnableReturnKeyAutomatically(bool value);

    uint32 GetCursorPos() const;
    void SetCursorPos(uint32 pos);

    void SystemDraw(const UIGeometricData&);

private:
    std::shared_ptr<TextFieldControlImpl> impl;
};

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
#endif // __DAVAENGINE_ANDROID__
