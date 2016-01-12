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

#include "UI/UITextFieldMacOs.h"

#ifdef __DAVAENGINE_MACOS__

#include <AppKit/NSTextField.h>

namespace DAVA
{
TextFieldPlatformImpl::TextFieldPlatformImpl(UITextField* tf)
{
}
TextFieldPlatformImpl::~TextFieldPlatformImpl()
{
}

void TextFieldPlatformImpl::OpenKeyboard()
{
}
void TextFieldPlatformImpl::CloseKeyboard()
{
}
void TextFieldPlatformImpl::GetText(WideString& string) const
{
}
void TextFieldPlatformImpl::SetText(const WideString& string)
{
}
void TextFieldPlatformImpl::UpdateRect(const Rect& rect)
{
}

void TextFieldPlatformImpl::SetTextColor(const DAVA::Color& color)
{
}
void TextFieldPlatformImpl::SetFontSize(float size)
{
}

void TextFieldPlatformImpl::SetTextAlign(DAVA::int32 align)
{
}
DAVA::int32 TextFieldPlatformImpl::GetTextAlign()
{
    return 0;
}
void TextFieldPlatformImpl::SetTextUseRtlAlign(bool useRtlAlign)
{
}
bool TextFieldPlatformImpl::GetTextUseRtlAlign() const
{
    return false;
}

void TextFieldPlatformImpl::SetVisible(bool value)
{
}
void TextFieldPlatformImpl::TextFieldPlatformImpl::ShowField()
{
}
void TextFieldPlatformImpl::HideField()
{
}

void TextFieldPlatformImpl::SetIsPassword(bool isPassword)
{
}

void TextFieldPlatformImpl::SetInputEnabled(bool value)
{
}

// Keyboard traits.
void TextFieldPlatformImpl::SetAutoCapitalizationType(DAVA::int32 value)
{
}
void TextFieldPlatformImpl::SetAutoCorrectionType(DAVA::int32 value)
{
}
void TextFieldPlatformImpl::SetSpellCheckingType(DAVA::int32 value)
{
}
void TextFieldPlatformImpl::SetKeyboardAppearanceType(DAVA::int32 value)
{
}
void TextFieldPlatformImpl::SetKeyboardType(DAVA::int32 value)
{
}
void TextFieldPlatformImpl::SetReturnKeyType(DAVA::int32 value)
{
}
void TextFieldPlatformImpl::SetEnableReturnKeyAutomatically(bool value)
{
}

// Cursor pos.
uint32 TextFieldPlatformImpl::GetCursorPos()
{
    return 0;
}
void TextFieldPlatformImpl::SetCursorPos(uint32 pos)
{
}

// Max text length.
void TextFieldPlatformImpl::SetMaxLength(int maxLength)
{
}

void TextFieldPlatformImpl::SetMultiline(bool multiline)
{
}

void TextFieldPlatformImpl::SetRenderToTexture(bool value)
{
}
bool TextFieldPlatformImpl::IsRenderToTexture() const
{
    return false;
}
void TextFieldPlatformImpl::SystemDraw(const UIGeometricData& geometricData)
{
}

} // end namespace DAVA

#endif //__DAVAENGINE_MACOS__