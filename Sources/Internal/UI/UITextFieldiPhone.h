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


#ifndef __DAVAENGINE_UI_TEXT_FIELD_IPHONE_H__
#define __DAVAENGINE_UI_TEXT_FIELD_IPHONE_H__

#include "UI/UITextField.h"

namespace DAVA 
{
class UITextField;

class TextFieldPlatformImpl
{
public:
    TextFieldPlatformImpl(UITextField* tf);
    virtual ~TextFieldPlatformImpl();

    void OpenKeyboard();
    void CloseKeyboard();
    void GetText(WideString & string) const;
	void SetText(const WideString & string);
	void UpdateRect(const Rect & rect);

	void SetTextColor(const DAVA::Color &color);
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
    void SystemDraw(const UIGeometricData& geometricData);

private:
    // Truncate the text to maxLength characters.
    void* TruncateText(void* text, int maxLength);
    void UpdateStaticTexture();
    void UpdateNativeRect(const Rect & virtualRect, int xOffset);

    Rect nextRect;
    Rect prevRect;
    UITextField& davaTextField;
	void * objcClassPtr;
    bool renderToTexture;
    bool isSingleLine = true;
    int deltaMoveControl = 0;
    bool isNeedToUpdateTexture = false;
};
};

#endif // __DAVAENGINE_UI_TEXT_FIELD_IPHONE_H__
