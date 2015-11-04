/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "UI/UITextFieldWinUAP.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Platform/TemplateWin32/PrivateTextFieldWinUAP.h"

namespace DAVA
{
TextFieldPlatformImpl::TextFieldPlatformImpl(UITextField* uiTextField)
    : privateImpl(std::make_shared<PrivateTextFieldWinUAP>(uiTextField))
{}

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

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
