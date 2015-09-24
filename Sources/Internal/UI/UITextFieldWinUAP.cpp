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

UITextFieldWinUAP::UITextFieldWinUAP(UITextField* uiTextField)
    : privateImpl(std::make_shared<PrivateTextFieldWinUAP>(uiTextField))
{}

UITextFieldWinUAP::~UITextFieldWinUAP()
{
    // Tell private implementation that owner is sentenced to death
    privateImpl->OwnerAtPremortem();
}

void UITextFieldWinUAP::SetVisible(bool isVisible)
{
    privateImpl->SetVisible(isVisible);
}

void UITextFieldWinUAP::SetIsPassword(bool isPassword)
{
    privateImpl->SetIsPassword(isPassword);
}

void UITextFieldWinUAP::SetMaxLength(int32 value)
{
    privateImpl->SetMaxLength(value);
}

void UITextFieldWinUAP::OpenKeyboard()
{
    privateImpl->OpenKeyboard();
}

void UITextFieldWinUAP::CloseKeyboard()
{
    privateImpl->CloseKeyboard();
}

void UITextFieldWinUAP::UpdateRect(const Rect& rect)
{
    privateImpl->UpdateRect(rect);
}

void UITextFieldWinUAP::SetText(const WideString& text)
{
    privateImpl->SetText(text);
}

void UITextFieldWinUAP::GetText(WideString& text) const
{
    privateImpl->GetText(text);
}

void UITextFieldWinUAP::SetTextColor(const Color& color)
{
    privateImpl->SetTextColor(color);
}

void UITextFieldWinUAP::SetTextAlign(int32 align)
{
    privateImpl->SetTextAlign(align);
}

int32 UITextFieldWinUAP::GetTextAlign() const
{
    return privateImpl->GetTextAlign();
}

void UITextFieldWinUAP::SetTextUseRtlAlign(bool useRtlAlign)
{
    privateImpl->SetTextUseRtlAlign(useRtlAlign);
}

bool UITextFieldWinUAP::GetTextUseRtlAlign() const
{
    return privateImpl->GetTextUseRtlAlign();
}

void UITextFieldWinUAP::SetFontSize(float32 size)
{
    privateImpl->SetFontSize(size);
}

void UITextFieldWinUAP::SetDelegate(UITextFieldDelegate* textFieldDelegate)
{
    privateImpl->SetDelegate(textFieldDelegate);
}

void UITextFieldWinUAP::SetMultiline(bool value)
{
    privateImpl->SetMultiline(value);
}

void UITextFieldWinUAP::SetInputEnabled(bool value)
{
    privateImpl->SetInputEnabled(value);
}

void UITextFieldWinUAP::SetRenderToTexture(bool value)
{
    privateImpl->SetRenderToTexture(value);
}

bool UITextFieldWinUAP::IsRenderToTexture() const
{
    return privateImpl->IsRenderToTexture();
}

void UITextFieldWinUAP::SetAutoCapitalizationType(int32 value)
{
    privateImpl->SetAutoCapitalizationType(value);
}

void UITextFieldWinUAP::SetAutoCorrectionType(int32 value)
{
    privateImpl->SetAutoCorrectionType(value);
}

void UITextFieldWinUAP::SetSpellCheckingType(int32 value)
{
    privateImpl->SetSpellCheckingType(value);
}

void UITextFieldWinUAP::SetKeyboardAppearanceType(int32 value)
{
    privateImpl->SetKeyboardAppearanceType(value);
}

void UITextFieldWinUAP::SetKeyboardType(int32 value)
{
    privateImpl->SetKeyboardType(value);
}

void UITextFieldWinUAP::SetReturnKeyType(int32 value)
{
    privateImpl->SetReturnKeyType(value);
}

void UITextFieldWinUAP::SetEnableReturnKeyAutomatically(bool value)
{
    privateImpl->SetEnableReturnKeyAutomatically(value);
}

uint32 UITextFieldWinUAP::GetCursorPos() const
{
    return privateImpl->GetCursorPos();
}

void UITextFieldWinUAP::SetCursorPos(uint32 pos)
{
    privateImpl->SetCursorPos(pos);
}

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
