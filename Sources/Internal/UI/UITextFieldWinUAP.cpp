#include "UI/UITextFieldWinUAP.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Platform/TemplateWin32/PrivateTextFieldWinUAP.h"

namespace DAVA
{
TextFieldPlatformImpl::TextFieldPlatformImpl(UITextField* uiTextField)
    : privateImpl(std::make_shared<PrivateTextFieldWinUAP>(uiTextField))
{
}

TextFieldPlatformImpl::~TextFieldPlatformImpl()
{
    // Tell private implementation that owner is sentenced to death
    privateImpl->OwnerAtPremortem();
}

void TextFieldPlatformImpl::SetVisible(bool isVisible)
{
    privateImpl->SetVisible(isVisible);
}

void TextFieldPlatformImpl::SetIsPassword(bool isPassword)
{
    privateImpl->SetIsPassword(isPassword);
}

void TextFieldPlatformImpl::SetMaxLength(int32 value)
{
    privateImpl->SetMaxLength(value);
}

void TextFieldPlatformImpl::OpenKeyboard()
{
    privateImpl->OpenKeyboard();
}

void TextFieldPlatformImpl::CloseKeyboard()
{
    privateImpl->CloseKeyboard();
}

void TextFieldPlatformImpl::UpdateRect(const Rect& rect)
{
    privateImpl->UpdateRect(rect);
}

void TextFieldPlatformImpl::SetText(const WideString& text)
{
    privateImpl->SetText(text);
}

void TextFieldPlatformImpl::GetText(WideString& text) const
{
    privateImpl->GetText(text);
}

void TextFieldPlatformImpl::SetTextColor(const Color& color)
{
    privateImpl->SetTextColor(color);
}

void TextFieldPlatformImpl::SetTextAlign(int32 align)
{
    privateImpl->SetTextAlign(align);
}

int32 TextFieldPlatformImpl::GetTextAlign() const
{
    return privateImpl->GetTextAlign();
}

void TextFieldPlatformImpl::SetTextUseRtlAlign(bool useRtlAlign)
{
    privateImpl->SetTextUseRtlAlign(useRtlAlign);
}

bool TextFieldPlatformImpl::GetTextUseRtlAlign() const
{
    return privateImpl->GetTextUseRtlAlign();
}

void TextFieldPlatformImpl::SetFontSize(float32 size)
{
    privateImpl->SetFontSize(size);
}

void TextFieldPlatformImpl::SetDelegate(UITextFieldDelegate* textFieldDelegate)
{
    privateImpl->SetDelegate(textFieldDelegate);
}

void TextFieldPlatformImpl::SetMultiline(bool value)
{
    privateImpl->SetMultiline(value);
}

void TextFieldPlatformImpl::SetInputEnabled(bool value)
{
    privateImpl->SetInputEnabled(value);
}

void TextFieldPlatformImpl::SetRenderToTexture(bool value)
{
    privateImpl->SetRenderToTexture(value);
}

bool TextFieldPlatformImpl::IsRenderToTexture() const
{
    return privateImpl->IsRenderToTexture();
}

void TextFieldPlatformImpl::SetAutoCapitalizationType(int32 value)
{
    privateImpl->SetAutoCapitalizationType(value);
}

void TextFieldPlatformImpl::SetAutoCorrectionType(int32 value)
{
    privateImpl->SetAutoCorrectionType(value);
}

void TextFieldPlatformImpl::SetSpellCheckingType(int32 value)
{
    privateImpl->SetSpellCheckingType(value);
}

void TextFieldPlatformImpl::SetKeyboardAppearanceType(int32 value)
{
    privateImpl->SetKeyboardAppearanceType(value);
}

void TextFieldPlatformImpl::SetKeyboardType(int32 value)
{
    privateImpl->SetKeyboardType(value);
}

void TextFieldPlatformImpl::SetReturnKeyType(int32 value)
{
    privateImpl->SetReturnKeyType(value);
}

void TextFieldPlatformImpl::SetEnableReturnKeyAutomatically(bool value)
{
    privateImpl->SetEnableReturnKeyAutomatically(value);
}

uint32 TextFieldPlatformImpl::GetCursorPos() const
{
    return privateImpl->GetCursorPos();
}

void TextFieldPlatformImpl::SetCursorPos(uint32 pos)
{
    privateImpl->SetCursorPos(pos);
}

void TextFieldPlatformImpl::SystemDraw(const UIGeometricData&)
{
}

} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
