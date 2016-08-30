#include "UI/Private/Android/TextFieldControlAndroid.h"

#if defined(__DAVAENGINE_ANDROID__)
#if defined(__DAVAENGINE_COREV2__)

#include "UI/Private/Android/TextFieldControlAndroidImpl.h"

namespace DAVA
{
TextFieldPlatformImpl::TextFieldPlatformImpl(Window& w, UITextField& uiTextField)
    : impl(std::make_shared<TextFieldControlImpl>(w, uiTextField))
{
    impl->Initialize();
}

TextFieldPlatformImpl::~TextFieldPlatformImpl()
{
    // Tell private implementation that owner is sentenced to death
    impl->OwnerIsDying();
}

void TextFieldPlatformImpl::SetVisible(bool isVisible)
{
    impl->SetVisible(isVisible);
}

void TextFieldPlatformImpl::SetIsPassword(bool isPassword)
{
    impl->SetIsPassword(isPassword);
}

void TextFieldPlatformImpl::SetMaxLength(int32 value)
{
    impl->SetMaxLength(value);
}

void TextFieldPlatformImpl::OpenKeyboard()
{
    impl->OpenKeyboard();
}

void TextFieldPlatformImpl::CloseKeyboard()
{
    impl->CloseKeyboard();
}

void TextFieldPlatformImpl::UpdateRect(const Rect& rect)
{
    impl->UpdateRect(rect);
}

void TextFieldPlatformImpl::SetText(const WideString& text)
{
    impl->SetText(text);
}

void TextFieldPlatformImpl::GetText(WideString& text) const
{
    impl->GetText(text);
}

void TextFieldPlatformImpl::SetTextColor(const Color& color)
{
    impl->SetTextColor(color);
}

void TextFieldPlatformImpl::SetTextAlign(int32 align)
{
    impl->SetTextAlign(align);
}

int32 TextFieldPlatformImpl::GetTextAlign() const
{
    return impl->GetTextAlign();
}

void TextFieldPlatformImpl::SetTextUseRtlAlign(bool useRtlAlign)
{
    impl->SetTextUseRtlAlign(useRtlAlign);
}

bool TextFieldPlatformImpl::GetTextUseRtlAlign() const
{
    return impl->GetTextUseRtlAlign();
}

void TextFieldPlatformImpl::SetFontSize(float32 size)
{
    impl->SetFontSize(size);
}

void TextFieldPlatformImpl::SetDelegate(UITextFieldDelegate* textFieldDelegate)
{
    impl->SetDelegate(textFieldDelegate);
}

void TextFieldPlatformImpl::SetMultiline(bool value)
{
    impl->SetMultiline(value);
}

void TextFieldPlatformImpl::SetInputEnabled(bool value)
{
    impl->SetInputEnabled(value);
}

void TextFieldPlatformImpl::SetRenderToTexture(bool value)
{
    impl->SetRenderToTexture(value);
}

bool TextFieldPlatformImpl::IsRenderToTexture() const
{
    return impl->IsRenderToTexture();
}

void TextFieldPlatformImpl::SetAutoCapitalizationType(int32 value)
{
    impl->SetAutoCapitalizationType(value);
}

void TextFieldPlatformImpl::SetAutoCorrectionType(int32 value)
{
    impl->SetAutoCorrectionType(value);
}

void TextFieldPlatformImpl::SetSpellCheckingType(int32 value)
{
    impl->SetSpellCheckingType(value);
}

void TextFieldPlatformImpl::SetKeyboardAppearanceType(int32 value)
{
    impl->SetKeyboardAppearanceType(value);
}

void TextFieldPlatformImpl::SetKeyboardType(int32 value)
{
    impl->SetKeyboardType(value);
}

void TextFieldPlatformImpl::SetReturnKeyType(int32 value)
{
    impl->SetReturnKeyType(value);
}

void TextFieldPlatformImpl::SetEnableReturnKeyAutomatically(bool value)
{
    impl->SetEnableReturnKeyAutomatically(value);
}

uint32 TextFieldPlatformImpl::GetCursorPos() const
{
    return impl->GetCursorPos();
}

void TextFieldPlatformImpl::SetCursorPos(uint32 pos)
{
    impl->SetCursorPos(pos);
}

void TextFieldPlatformImpl::SystemDraw(const UIGeometricData&)
{
}

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
#endif // __DAVAENGINE_ANDROID__
