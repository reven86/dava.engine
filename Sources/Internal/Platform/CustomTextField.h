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

#ifndef __DAVAENGINE_CUSTOM_TEXT_FIELD_H__
#define __DAVAENGINE_CUSTOM_TEXT_FIELD_H__

#include "Base/BaseTypes.h"

namespace DAVA
{
struct Rect;
class Color;
class Font;
class UIEvent;
class UIStaticText;
class UITextFieldImpl;

class CustomTextField
{
public:
    CustomTextField(UITextFieldImpl* impl);
    ~CustomTextField();

    void GetText(WideString & string) const;
    void SetText(const WideString & string);
    void UpdateRect(const Rect & rect, float32 timeElapsed);
    void GetTextColor(DAVA::Color &color) const;
    void SetTextColor(const DAVA::Color &color);
    Font *GetFont();
    void SetFont(Font * font);
    void SetFontSize(float32 size);
    void SetTextAlign(DAVA::int32 align);
    void SetIsPassword(bool isPassword);

    uint32 GetCursorPos() const;
    void SetCursorPos(uint32 pos);

    void Input(UIEvent *currentInput);
    void Draw();

    WideString GetVisibleText() const;

private:
    UIStaticText *staticText;
    WideString text;

    bool needRedraw:1;
    bool showCursor:1;
    bool isPassword:1;
    float32 cursorTime;
    UITextFieldImpl * textFieldImpl;
};
};
#endif // __DAVAENGINE_CUSTOM_TEXT_FIELD_H__
