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

#include "Math/Rect.h"
#include "Math/Color.h"

namespace DAVA
{

UITextFieldWinUAP::UITextFieldWinUAP(UITextField* uiTextField_)
    : uiTextField(uiTextField_)
{}

UITextFieldWinUAP::~UITextFieldWinUAP()
{}

void UITextFieldWinUAP::SetVisible(bool isVisible)
{
}

void UITextFieldWinUAP::SetIsPassword(bool isPassword)
{
}

void UITextFieldWinUAP::SetMaxLength(int32 value)
{}

void UITextFieldWinUAP::OpenKeyboard()
{
}

void UITextFieldWinUAP::CloseKeyboard()
{
}

void UITextFieldWinUAP::UpdateRect(const Rect& rect)
{
}

void UITextFieldWinUAP::SetText(const WideString& text)
{
}

void UITextFieldWinUAP::GetText(WideString& text) const
{
}

void UITextFieldWinUAP::SetTextColor(const Color& color)
{
}

void UITextFieldWinUAP::SetTextAlign(int32 align)
{
}

int32 UITextFieldWinUAP::GetTextAlign() const
{
    return 0;
}

void UITextFieldWinUAP::SetTextUseRtlAlign(bool useRtlAlign)
{
}

bool UITextFieldWinUAP::GetTextUseRtlAlign() const
{
    return false;
}

void UITextFieldWinUAP::SetFontSize(float32 size)
{
}

void UITextFieldWinUAP::SetMultiline(bool value)
{
}

void UITextFieldWinUAP::SetInputEnabled(bool value)
{}

void UITextFieldWinUAP::SetRenderToTexture(bool value)
{}

bool UITextFieldWinUAP::IsRenderToTexture() const
{
    return false;
}

void UITextFieldWinUAP::SetAutoCapitalizationType(int32 value)
{}

void UITextFieldWinUAP::SetAutoCorrectionType(int32 value)
{}

void UITextFieldWinUAP::SetSpellCheckingType(int32 value)
{}

void UITextFieldWinUAP::SetKeyboardAppearanceType(int32 value)
{}

void UITextFieldWinUAP::SetKeyboardType(int32 value)
{}

void UITextFieldWinUAP::SetReturnKeyType(int32 value)
{}

void UITextFieldWinUAP::SetEnableReturnKeyAutomatically(bool value)
{}

uint32 UITextFieldWinUAP::GetCursorPos() const
{
    return 0;
}

void UITextFieldWinUAP::SetCursorPos(uint32 pos)
{
}

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
