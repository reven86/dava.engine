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

#ifndef __DAVA_UITEXTFIELDSTB_H__
#define __DAVA_UITEXTFIELDSTB_H__

#include "Base/BaseTypes.h"
#include "Render/2D/TextBlock.h"
#include "UI/Private/StbTextEditBridge.h"

namespace DAVA
{
class UITextField;
class UIStaticText;
class UIGeometricData;
class Font;
class Color;
class UIEvent;
class Vector2;
struct Rect;

// This implementation simulate iOS/Android native controls,
// so no hierarchy for internal UIStaticText, and call UpdateRect
// every frame, and render directly in SyctemDraw. This helps
// to find similar bugs in all implementations
class TextFieldStbImpl : public StbTextEditBridge
{
public:
    friend class UITextField;
    TextFieldStbImpl(UITextField* control);
    ~TextFieldStbImpl();
    void CopyDataFrom(TextFieldStbImpl* t);
    void OpenKeyboard();
    void CloseKeyboard();
    void SetRenderToTexture(bool);
    void SetIsPassword(bool);
    void SetFontSize(float32);
    void SetText(const WideString& text);
    void UpdateRect(const Rect&);
    void SetAutoCapitalizationType(int32);
    void SetAutoCorrectionType(int32);
    void SetSpellCheckingType(int32);
    void SetKeyboardAppearanceType(int32);
    void SetKeyboardType(int32);
    void SetReturnKeyType(int32);
    void SetEnableReturnKeyAutomatically(int32);
    bool IsRenderToTexture() const;
    uint32 GetCursorPos() const;
    void SetCursorPos(int32);
    void SetMaxLength(int32);
    void GetText(WideString&);
    void SetInputEnabled(bool, bool hierarchic = true);
    void SetVisible(bool v);
    void SetFont(Font* f);
    Font* GetFont() const;
    void SetTextColor(const Color& c);
    void SetShadowOffset(const Vector2& v);
    void SetShadowColor(const Color& c);
    void SetTextAlign(int32 align);
    TextBlock::eUseRtlAlign GetTextUseRtlAlign();
    void SetTextUseRtlAlign(TextBlock::eUseRtlAlign align);
    void SetSize(const Vector2 vector2);
    void SetMultiline(bool is_multiline);
    Color GetTextColor();
    Vector2 GetShadowOffset();
    Color GetShadowColor();
    int32 GetTextAlign();
    void SetRect(const Rect& rect);
    void SystemDraw(const UIGeometricData& d);

    void Input(UIEvent* currentInput);
    void DrawSelection(const UIGeometricData& geometricData);
    void DrawCursor(const UIGeometricData& geometricData);

    // StbTextEditBridge
    void InsertText(uint32 position, const WideString::value_type* str, uint32 length) override;
    void DeleteText(uint32 position, uint32 length) override;
    const Vector<TextBlock::Line>& GetMultilineInfo() override;
    const Vector<float32>& GetCharactersSizes() override;
    uint32 GetLength() override;
    WideString::value_type GetChar(uint32 i) override;
    void SendKey(uint32 codePoint) override;

private:
    void UpdateSelection(uint32 start, uint32 end);
    void UpdateCursor(uint32 cursorPos, bool insertMode);

    UIStaticText* staticText = nullptr;
    UITextField* control = nullptr;
    float32 cursorTime = 0.0f;
    bool needRedraw = true;
    bool showCursor = true;
    Color selectionColor = Color(0.f, 0.f, 0.7f, 0.7f);
    Color cursorColor = Color::White;
    Vector<Rect> selectionRects;
    Rect cursorRect;
};

} // end namespace DAVA

#endif //__DAVA_UITEXTFIELDSTB_H__