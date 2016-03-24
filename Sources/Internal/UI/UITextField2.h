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

#ifndef __DAVAENGINE_UI_TEXT_FIELD2_H__
#define __DAVAENGINE_UI_TEXT_FIELD2_H__

#include "UI/UIControl.h"
#include "UI/UITextField.h"
#include "Render/2D/TextBlock.h"

struct StbTextStruct;

namespace DAVA
{
class UIStaticText;
class UITextField2;
class IUITextField2Impl;

class UITextField2Delegate
{
public:
    virtual ~UITextField2Delegate() = default;

    virtual void OnSubmit(UITextField2* /*textField*/)
    {
    }
    virtual void OnCancel(UITextField2* /*textField*/)
    {
    }
    virtual void OnLostFocus(UITextField2* /*textField*/)
    {
    }
    virtual bool OnTextChanging(UITextField2* /*textField*/, int32 /*replacementLocation*/, int32 /*replacementLength*/, WideString& /*replacementString*/)
    {
        return true;
    }
    virtual void OnTextChanged(UITextField2* /*textField*/, const WideString& /*newText*/, const WideString& /*oldText*/)
    {
    }
    virtual void OnKeyboardShown(const Rect& keyboardRect)
    {
    }
    virtual void OnKeyboardResized(const Rect& keyboardRect)
    {
    }
    virtual void OnKeyboardHidden()
    {
    }
};

class UITextField2 : public UIControl
{
public:
    UITextField2(const Rect& rect = Rect());
    UITextField2* Clone() override;
    void CopyDataFrom(UIControl* srcControl) override;

    // Text field text modification
    void innerInsertText(uint32 position, const WideString::value_type* str, uint32 length);
    void innerDeleteText(uint32 position, uint32 length);

    void InsertText(uint32 position, const WideString& str);
    void SendChar(uint32 codePoint);

    // Text field properties
    const WideString& GetText() const;
    virtual void SetText(const WideString& text);
    WideString GetVisibleText() const;
    bool GetMultiline() const;
    void SetMultiline(bool value);
    bool GetPassword() const;
    void SetPassword(bool value);
    UITextField2Delegate* GetDelegate();
    void SetDelegate(UITextField2Delegate* delegate);
    uint32 GetCursorPos();
    void SetCursorPos(uint32 pos);
    int32 GetMaxLength() const;
    void SetMaxLength(int32 maxLength);

    // Keyboard control
    void OpenKeyboard();
    void CloseKeyboard();

    // Keyboard customizing properties
    int32 GetAutoCapitalizationType() const;
    void SetAutoCapitalizationType(int32 value);
    int32 GetAutoCorrectionType() const;
    void SetAutoCorrectionType(int32 value);
    int32 GetSpellCheckingType() const;
    void SetSpellCheckingType(int32 value);
    int32 GetKeyboardAppearanceType() const;
    void SetKeyboardAppearanceType(int32 value);
    int32 GetKeyboardType() const;
    void SetKeyboardType(int32 value);
    int32 GetReturnKeyType() const;
    void SetReturnKeyType(int32 value);
    bool IsEnableReturnKeyAutomatically() const;
    void SetEnableReturnKeyAutomatically(bool value);

    // UIStaticText properties proxy
    Font* GetFont() const;
    void SetFont(Font* font);
    String GetFontPresetName() const;
    void SetFontByPresetName(const String& presetName);
    void SetFontSize(float32 size);
    Color GetTextColor() const;
    void SetTextColor(const Color& fontColor);
    Vector2 GetShadowOffset() const;
    void SetShadowOffset(const DAVA::Vector2& offset);
    Color GetShadowColor() const;
    void SetShadowColor(const Color& color);
    int32 GetTextAlign() const;
    void SetTextAlign(int32 align);
    TextBlock::eUseRtlAlign GetTextUseRtlAlign() const;
    void SetTextUseRtlAlign(TextBlock::eUseRtlAlign useRtlAlign);
    int32 GetTextUseRtlAlignAsInt() const;
    void SetTextUseRtlAlignFromInt(int32 value);

    // UIControl overrides
    void OnFocused() override;
    void OnFocusLost(UIControl* newFocus) override;
    void Update(float32 timeElapsed) override;
    void Input(UIEvent* currentInput) override;
    void SetSize(const DAVA::Vector2& newSize) override;

protected:
    ~UITextField2() override;

private:
    void SetupDefaults();

    WideString text;

    UITextField2Delegate* delegate = nullptr;
    UIStaticText* staticText = nullptr;
    StbTextStruct* stb_struct = nullptr;
    IUITextField2Impl* pImpl = nullptr;

    // Keyboard customization params
    UITextField::eAutoCapitalizationType autoCapitalizationType;
    UITextField::eAutoCorrectionType autoCorrectionType;
    UITextField::eSpellCheckingType spellCheckingType;
    UITextField::eKeyboardAppearanceType keyboardAppearanceType;
    UITextField::eKeyboardType keyboardType;
    UITextField::eReturnKeyType returnKeyType;

    int32 maxLength = -1;

    bool isPassword = false;
    bool enableReturnKeyAutomatically = false;
    bool isMultiline = false;
    bool needRedraw = true;
    bool showCursor = true;

    float32 cursorBlinkingTime = 0.0f;
    float32 cursorTime = 0.0f;

public:
    INTROSPECTION_EXTEND(UITextField2, UIControl,
                         PROPERTY("text", "Text", GetText, SetText, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("font", "Font", GetFontPresetName, SetFontByPresetName, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("textcolor", "Text color", GetTextColor, SetTextColor, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("shadowoffset", "Shadow Offset", GetShadowOffset, SetShadowOffset, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("shadowcolor", "Shadow Color", GetShadowColor, SetShadowColor, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("textalign", InspDesc("Text Align", GlobalEnumMap<eAlign>::Instance(), InspDesc::T_FLAGS), GetTextAlign, SetTextAlign, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("textUseRtlAlign", InspDesc("Use Rtl Align", GlobalEnumMap<TextBlock::eUseRtlAlign>::Instance(), InspDesc::T_ENUM), GetTextUseRtlAlignAsInt, SetTextUseRtlAlignFromInt, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("maxLength", "Max text lenght", GetMaxLength, SetMaxLength, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("isPassword", "Is password", GetPassword, SetPassword, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("isMultiline", "Multi Line", GetMultiline, SetMultiline, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("autoCapitalizationType", InspDesc("Auto capitalization type", GlobalEnumMap<UITextField::eAutoCapitalizationType>::Instance()), GetAutoCapitalizationType, SetAutoCapitalizationType, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("autoCorrectionType", InspDesc("Auto correction type", GlobalEnumMap<UITextField::eAutoCorrectionType>::Instance()), GetAutoCorrectionType, SetAutoCorrectionType, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("spellCheckingType", InspDesc("Spell checking type", GlobalEnumMap<UITextField::eSpellCheckingType>::Instance()), GetSpellCheckingType, SetSpellCheckingType, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("keyboardAppearanceType", InspDesc("Keyboard appearance type", GlobalEnumMap<UITextField::eKeyboardAppearanceType>::Instance()), GetKeyboardAppearanceType, SetKeyboardAppearanceType, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("keyboardType", InspDesc("Keyboard type", GlobalEnumMap<UITextField::eKeyboardType>::Instance()), GetKeyboardType, SetKeyboardType, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("returnKeyType", InspDesc("Return key type", GlobalEnumMap<UITextField::eReturnKeyType>::Instance()), GetReturnKeyType, SetReturnKeyType, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("enableReturnKeyAutomatically", "Automatically enable return key", IsEnableReturnKeyAutomatically, SetEnableReturnKeyAutomatically, I_SAVE | I_VIEW | I_EDIT))
};

} // namespace DAVA

#endif // __DAVAENGINE_UI_TEXT_FIELD2_H__
