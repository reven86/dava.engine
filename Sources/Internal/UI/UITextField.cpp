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


#include "UI/UITextField.h"
#include "Input/KeyboardDevice.h"
#include "UI/UIYamlLoader.h"
#include "UI/UIControlSystem.h"
#include "Render/2D/FontManager.h"
#include "FileSystem/YamlNode.h"

#if defined(__DAVAENGINE_ANDROID__)
#   include "UITextFieldAndroid.h"
#   include "Utils/UTF8Utils.h"

extern void CreateTextField(DAVA::UITextField *);
extern void ReleaseTextField();
extern void OpenKeyboard();
extern void CloseKeyboard();

#elif defined(__DAVAENGINE_IPHONE__)
#   include "UI/UITextFieldiPhone.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
#   include "UI/UITextFieldWinUAP.h"
#endif

#if defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_WIN_UAP__)
#   define DAVA_TEXTFIELD_USE_NATIVE
#endif

namespace DAVA
{

UITextField::UITextField(const Rect &rect, bool rectInAbsoluteCoordinates/*= false*/)
    : UIControl(rect, rectInAbsoluteCoordinates)
{
#if defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid = new UITextFieldAndroid(this);
    textFieldAndroid->SetVisible(false);
#elif defined(__DAVAENGINE_IPHONE__)
    textFieldiPhone = new UITextFieldiPhone(*this);
    textFieldiPhone->SetVisible(false);
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP = new UITextFieldWinUAP(this);
    textFieldWinUAP->SetVisible(false);
#else
    staticText = new UIStaticText(Rect(0,0,GetRect().dx, GetRect().dy));
    staticText->SetVisible(false);
    AddControl(staticText);
    staticText->SetSpriteAlign(ALIGN_LEFT | ALIGN_BOTTOM);
#endif
    
    SetupDefaults();
}
    
void UITextField::SetupDefaults()
{
    SetInputEnabled(true, false);
    
    SetAutoCapitalizationType(AUTO_CAPITALIZATION_TYPE_SENTENCES);
    SetAutoCorrectionType(AUTO_CORRECTION_TYPE_DEFAULT);
    SetSpellCheckingType(SPELL_CHECKING_TYPE_DEFAULT);
    SetKeyboardAppearanceType(KEYBOARD_APPEARANCE_DEFAULT);
    SetKeyboardType(KEYBOARD_TYPE_DEFAULT);
    SetReturnKeyType(RETURN_KEY_DEFAULT);
    SetEnableReturnKeyAutomatically(false);
    SetTextUseRtlAlign(TextBlock::RTL_DONT_USE);
    
    SetMaxLength(-1);
    
    SetIsPassword(false);
    SetTextColor(GetTextColor());
    SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
    
    SetFontSize(26); //12 is default size for IOS
    
    SetText(L"");
    SetRenderToTexture(true);
}

UITextField::~UITextField()
{
#if defined (__DAVAENGINE_ANDROID__)
    SafeDelete(textFieldAndroid);
#elif defined (__DAVAENGINE_IPHONE__)
    SafeDelete(textFieldiPhone);
#elif defined(__DAVAENGINE_WIN_UAP__)
    SafeDelete(textFieldWinUAP);
#else
    SafeRelease(textFont);

    RemoveAllControls();
    SafeRelease(staticText);
#endif
}

void UITextField::OpenKeyboard()
{
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->OpenKeyboard();
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->OpenKeyboard();
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->OpenKeyboard();
#endif
}

void UITextField::CloseKeyboard()
{
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->CloseKeyboard();
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->CloseKeyboard();
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->CloseKeyboard();
#endif
}

void UITextField::Update(float32 timeElapsed)
{
#ifdef __DAVAENGINE_IPHONE__
    // Calling UpdateRect with allowNativeControlMove set to true
    textFieldiPhone->UpdateRect(GetGeometricData().GetUnrotatedRect());
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->UpdateRect(GetGeometricData().GetUnrotatedRect());
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->UpdateRect(GetGeometricData().GetUnrotatedRect());
#else
    if(this == UIControlSystem::Instance()->GetFocusedControl())
    {
        cursorTime += timeElapsed;

        if (cursorTime >= 0.5f)
        {
            cursorTime = 0;
            showCursor = !showCursor;
            needRedraw = true;
        }
    }
    
    if (!needRedraw)
        return;

    // Use NO_REQUIRED_SIZE to notify staticText->SetText that we don't want
    // to enable of any kind of static text fitting
    static const Vector2 NO_REQUIRED_SIZE = Vector2(-1, -1);

    if(this == UIControlSystem::Instance()->GetFocusedControl())
    {
        WideString txt = GetVisibleText();
        txt += showCursor ? L"_" : L" ";
        staticText->SetText(txt, NO_REQUIRED_SIZE);
    }
    else
    {
        staticText->SetText(GetVisibleText(), NO_REQUIRED_SIZE);
    }
    needRedraw = false;
#endif
}

void UITextField::WillAppear()
{
    if (delegate != nullptr && delegate->IsTextFieldShouldSetFocusedOnAppear(this)) 
    {
        UIControlSystem::Instance()->SetFocusedControl(this, false);
    }
}

void UITextField::DidAppear()
{
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->ShowField();
    textFieldiPhone->SetVisible(IsOnScreen());
#endif
}

void UITextField::WillDisappear()
{
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->HideField();
#endif
}
    
void UITextField::OnFocused()
{
    SetRenderToTexture(false);
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->OpenKeyboard();
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->OpenKeyboard();
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->OpenKeyboard();
#endif
}
    
void UITextField::OnFocusLost(UIControl *newFocus)
{
    SetRenderToTexture(true);
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->CloseKeyboard();
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->CloseKeyboard();
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->CloseKeyboard();
#endif
    if (delegate != nullptr)
    {
        delegate->TextFieldLostFocus(this);
    }
}

bool UITextField::IsLostFocusAllowed(UIControl *newFocus)
{
    if (delegate != nullptr)
    {
        return delegate->IsTextFieldCanLostFocus(this);
    }
    return true;
}

void UITextField::ReleaseFocus()
{
    if(this == UIControlSystem::Instance()->GetFocusedControl())
    {
        UIControlSystem::Instance()->SetFocusedControl(nullptr, true);
    }
}
    
void UITextField::SetFont(Font * font)
{
#if !defined(DAVA_TEXTFIELD_USE_NATIVE)
    if (font == textFont)
    {
        return;
    }

    SafeRelease(textFont);
    textFont = SafeRetain(font);
    staticText->SetFont(textFont);
#endif  // !defined(DAVA_TEXTFIELD_USE_NATIVE)
}

void UITextField::SetTextColor(const Color& fontColor)
{
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->SetTextColor(fontColor);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetTextColor(fontColor);
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->SetTextColor(fontColor);
#else
    staticText->SetTextColor(fontColor);
#endif
}

void UITextField::SetShadowOffset(const DAVA::Vector2 &offset)
{
#if !defined(DAVA_TEXTFIELD_USE_NATIVE)
    staticText->SetShadowOffset(offset);
#endif
}

void UITextField::SetShadowColor(const Color& color)
{
#if !defined(DAVA_TEXTFIELD_USE_NATIVE)
    staticText->SetShadowColor(color);
#endif
}

void UITextField::SetTextAlign(int32 align)
{
#if defined(__DAVAENGINE_IPHONE__)
    textFieldiPhone->SetTextAlign(align);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetTextAlign(align);
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->SetTextAlign(align);
#else
    staticText->SetTextAlign(align);
#endif
}

TextBlock::eUseRtlAlign UITextField::GetTextUseRtlAlign() const
{
#ifdef __DAVAENGINE_IPHONE__
    return (textFieldiPhone && textFieldiPhone->GetTextUseRtlAlign()) ? TextBlock::RTL_USE_BY_CONTENT : TextBlock::RTL_DONT_USE;
#elif defined(__DAVAENGINE_ANDROID__)
    return (textFieldAndroid && textFieldAndroid->GetTextUseRtlAlign()) ? TextBlock::RTL_USE_BY_CONTENT : TextBlock::RTL_DONT_USE;
#elif defined(__DAVAENGINE_WIN_UAP__)
    return textFieldWinUAP->GetTextUseRtlAlign() ? TextBlock::RTL_USE_BY_CONTENT : TextBlock::RTL_DONT_USE;
#else
    return staticText ? staticText->GetTextUseRtlAlign() : TextBlock::RTL_DONT_USE;
#endif
}

void UITextField::SetTextUseRtlAlign(TextBlock::eUseRtlAlign useRtlAlign)
{
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->SetTextUseRtlAlign(useRtlAlign == TextBlock::RTL_USE_BY_CONTENT);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetTextUseRtlAlign(useRtlAlign == TextBlock::RTL_USE_BY_CONTENT);
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->SetTextUseRtlAlign(useRtlAlign == TextBlock::RTL_USE_BY_CONTENT);
#else
    staticText->SetTextUseRtlAlign(useRtlAlign);
#endif
}

void UITextField::SetTextUseRtlAlignFromInt(int32 value)
{
    SetTextUseRtlAlign(static_cast<TextBlock::eUseRtlAlign>(value));
}
    
int32 UITextField::GetTextUseRtlAlignAsInt() const
{
    return static_cast<TextBlock::eUseRtlAlign>(GetTextUseRtlAlign());
}

void UITextField::SetFontSize(float32 size)
{
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->SetFontSize(size);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetFontSize(size);
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->SetFontSize(size);
#endif
}

void UITextField::SetDelegate(UITextFieldDelegate * _delegate)
{
    delegate = _delegate;
#if defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->SetDelegate(_delegate);
#endif
}

UITextFieldDelegate * UITextField::GetDelegate()
{
    return delegate;
}

void UITextField::SetSpriteAlign(int32 align)
{
    UIControl::SetSpriteAlign(align);
}

void UITextField::SetSize(const DAVA::Vector2 &newSize)
{
    UIControl::SetSize(newSize);
#if !defined(DAVA_TEXTFIELD_USE_NATIVE)
    staticText->SetSize(newSize);
#endif
}
    
void UITextField::SetPosition(const DAVA::Vector2 &position)
{
    UIControl::SetPosition(position);
}

void UITextField::SetMultiline(bool value)
{
    if (value != isMultiline_)
    {
        isMultiline_ = value;
        
#ifdef __DAVAENGINE_IPHONE__
        textFieldiPhone->SetMultiline(isMultiline_);
#elif defined(__DAVAENGINE_ANDROID__)
        textFieldAndroid->SetMultiline(isMultiline_);
#elif defined(__DAVAENGINE_WIN_UAP__)
        textFieldWinUAP->SetMultiline(isMultiline_);
#else 
        staticText->SetMultiline(isMultiline_);
#endif
    }
}

bool UITextField::IsMultiline() const
{
    return isMultiline_;
}
    
void UITextField::SetText(const WideString& text_)
{
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->SetText(text_);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetText(text_);
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->SetText(text_);
#else
    if (delegate && text != text_)
    {
        delegate->TextFieldOnTextChanged(this, text_, text);
    }
#endif
    text = text_;

    needRedraw = true;
}

const WideString & UITextField::GetText()
{
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->GetText(text);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->GetText(text);
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->GetText(text);
#endif
    return text;
}
    
Font* UITextField::GetFont() const
{
#if defined(DAVA_TEXTFIELD_USE_NATIVE)
    return nullptr;
#else
    return textFont;
#endif
}

const Color &UITextField::GetTextColor() const
{
#if defined(DAVA_TEXTFIELD_USE_NATIVE)
    return Color::White;
#else
    return staticText ? staticText->GetTextColor() : Color::White;
#endif
}

Vector2 UITextField::GetShadowOffset() const
{
#if defined(DAVA_TEXTFIELD_USE_NATIVE)
    return Vector2(0, 0);
#else
    return staticText ? staticText->GetShadowOffset() : Vector2(0,0);
#endif
}

const Color &UITextField::GetShadowColor() const
{
#if defined(DAVA_TEXTFIELD_USE_NATIVE)
    return Color::White;
#else
    return staticText ? staticText->GetShadowColor() : Color::White;
#endif
}

int32 UITextField::GetTextAlign() const
{
#ifdef __DAVAENGINE_IPHONE__
    return textFieldiPhone->GetTextAlign();
#elif defined(__DAVAENGINE_ANDROID__)
    return textFieldAndroid->GetTextAlign();
#elif defined(__DAVAENGINE_WIN_UAP__)
    return textFieldWinUAP->GetTextAlign();
#else
    return staticText->GetTextAlign();
#endif
}

void UITextField::Input(UIEvent *currentInput)
{
#if !defined(DAVA_TEXTFIELD_USE_NATIVE)
    if (NULL == delegate)
    {
        return;
    }

    if(this != UIControlSystem::Instance()->GetFocusedControl())
        return;


    if (currentInput->phase == UIEvent::PHASE_KEYCHAR)
    {
        if (currentInput->tid == 0 && currentInput->keyChar == '\b')
        {
            WideString str = L"";
            if(delegate->TextFieldKeyPressed(this, (int32)GetText().length() - 1, 1, str))
            {
                SetText(GetAppliedChanges((int32)GetText().length() - 1,  1, str));
            }
        }
        else if (currentInput->tid == DVKEY_ENTER)
        {
            delegate->TextFieldShouldReturn(this);
        }
        else if (currentInput->tid == DVKEY_ESCAPE)
        {
            delegate->TextFieldShouldCancel(this);
        }
        else if(currentInput->keyChar != 0)
        {
            WideString str;
            str += currentInput->keyChar;
            if(delegate->TextFieldKeyPressed(this, (int32)GetText().length(), 0, str))
            {
                SetText(GetAppliedChanges((int32)GetText().length(),  0, str));
            }
        }
    }

    currentInput->SetInputHandledType(UIEvent::INPUT_HANDLED_SOFT); // Drag is not handled - see please DF-2508.
#endif
}
    
WideString UITextField::GetAppliedChanges(int32 replacementLocation, int32 replacementLength, const WideString & replacementString)
{//TODO: fix this for copy/paste
    WideString txt = GetText();
    
    if(replacementLocation >= 0)
    {
        if(replacementLocation <= (int32)txt.length())
        {
            txt.replace(replacementLocation, replacementLength, replacementString);
        }
        else
        {
            Logger::Error("[UITextField::GetAppliedChanges] - index out of bounds.");
        }
    }
    
    return txt;
}

void UITextField::RenderText()
{
#ifdef __DAVAENGINE_IPHONE__
    // nothing to do
#else

#if 0
    SafeRelease(textSprite);

    int32 w = GetRect().dx;
    int32 h = GetRect().dy;

    EnsurePowerOf2(w);
    EnsurePowerOf2(h);

    int16 * buf;
    buf = new int16[w * h];
    memset(buf, 0, w * h * sizeof(int16));

    Size2i ds = textFont->DrawString(text, buf, w, h, 0, 0);
    String addInfo = WStringToString(Format(L"Text texture: %S", text.c_str()));
    Texture *tex = Texture::CreateTextFromData(FORMAT_RGBA4444, (uint8*)buf, w, h, addInfo.c_str());
    delete [] buf;

    textSprite = Sprite::CreateFromTexture(tex, 0, 0, GetRect().dx, GetRect().dy);
    SafeRelease(tex);

    textSprite->SetDefaultPivotPoint(0.0f, (float32)(textFont->GetBaseline() - h));
    
    SetSprite(textSprite, 0);

#endif

#endif
}

void UITextField::LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader)
{
    UIControl::LoadFromYamlNode(node, loader);

    const YamlNode * textNode = node->Get("text");
    if (textNode)
    {
        SetText(textNode->AsWString());
    }

    const YamlNode * fontNode = node->Get("font");
    if (fontNode)
    {
        Font * font = loader->GetFontByName(fontNode->AsString());
        if (font)
        {
            SetFont(font);
            SetFontSize((float32)font->GetFontHeight());
        }
    }
    
    const YamlNode * passwordNode = node->Get("isPassword");
    if (passwordNode)
    {
        SetIsPassword(passwordNode->AsBool());
    }

    // Keyboard customization params.
    const YamlNode* autoCapitalizationTypeNode = node->Get("autoCapitalizationType");
    if (autoCapitalizationTypeNode)
    {
        SetAutoCapitalizationType((eAutoCapitalizationType)autoCapitalizationTypeNode->AsInt32());
    }

    const YamlNode* autoCorrectionTypeNode = node->Get("autoCorrectionType");
    if (autoCorrectionTypeNode)
    {
        SetAutoCorrectionType((eAutoCorrectionType)autoCorrectionTypeNode->AsInt32());
    }

    const YamlNode* spellCheckingTypeNode = node->Get("spellCheckingType");
    if (spellCheckingTypeNode)
    {
        SetSpellCheckingType((eSpellCheckingType)spellCheckingTypeNode->AsInt32());
    }

    const YamlNode* keyboardAppearanceTypeNode = node->Get("keyboardAppearanceType");
    if (keyboardAppearanceTypeNode)
    {
        SetKeyboardAppearanceType((eKeyboardAppearanceType)keyboardAppearanceTypeNode->AsInt32());
    }

    const YamlNode* keyboardTypeNode = node->Get("keyboardType");
    if (keyboardTypeNode)
    {
        SetKeyboardType((eKeyboardType)keyboardTypeNode->AsInt32());
    }

    const YamlNode* returnKeyTypeNode = node->Get("returnKeyType");
    if (returnKeyTypeNode)
    {
        SetReturnKeyType((eReturnKeyType)returnKeyTypeNode->AsInt32());
    }

    const YamlNode* enableReturnKeyAutomaticallyNode = node->Get("enableReturnKeyAutomatically");
    if (enableReturnKeyAutomaticallyNode)
    {
        SetEnableReturnKeyAutomatically(enableReturnKeyAutomaticallyNode->AsBool());
    }

#if !defined(DAVA_TEXTFIELD_USE_NATIVE)
    if (staticText != nullptr)
    {
        staticText->SetRect(Rect(0,0,GetRect().dx, GetRect().dy));

        const YamlNode * shadowColorNode = node->Get("shadowcolor");
        const YamlNode * shadowOffsetNode = node->Get("shadowoffset");
        if(shadowColorNode)
        {
            SetShadowColor(shadowColorNode->AsColor());
        }
        if(shadowOffsetNode)
        {
            SetShadowOffset(shadowOffsetNode->AsVector2());
        }
    }
#endif

    const YamlNode * textColorNode = node->Get("textcolor");
    const YamlNode * textAlignNode = node->Get("textalign");

    if(textColorNode)
    {
        SetTextColor(textColorNode->AsColor());
    }

    if(textAlignNode)
    {
        SetTextAlign(loader->GetAlignFromYamlNode(textAlignNode));
    }

    const YamlNode * textUseRtlAlign = node->Get("textUseRtlAlign");
    if(textUseRtlAlign)
    {
        SetTextUseRtlAlign(static_cast<TextBlock::eUseRtlAlign>(textUseRtlAlign->AsInt32()));
    }

    const YamlNode* maxLengthNode = node->Get("maxLength");
    if (maxLengthNode)
    {
        SetMaxLength(maxLengthNode->AsInt32());
    }

#if 0
    const YamlNode * orientNode = node->Get("orientation");
    if (orientNode)
    {
        if (orientNode->AsString() == "Vertical")
            orientation = ORIENTATION_VERTICAL;
        else if (orientNode->AsString() == "Horizontal")
            orientation = ORIENTATION_HORIZONTAL;
    }



        // TODO
    InitAfterYaml();
#endif
}

YamlNode * UITextField::SaveToYamlNode(UIYamlLoader * loader)
{
    ScopedPtr<UITextField> baseTextField(new UITextField());

    YamlNode *node = UIControl::SaveToYamlNode(loader);

    //Text
    if (baseTextField->GetText() != this->GetText())
    {
        node->Set("text", GetText());
    }

    //Font
    //Get font name and put it here
    node->Set("font", FontManager::Instance()->GetFontName(this->GetFont()));
    
    //TextColor
    const Color &textColor = GetTextColor();
    if (baseTextField->GetTextColor() != textColor)
    {
        node->Set("textcolor", VariantType(textColor));
    }

    // ShadowColor
    const Color &shadowColor = GetShadowColor();
    if (baseTextField->GetShadowColor() != GetShadowColor())
    {
        node->Set("shadowcolor", VariantType(shadowColor));
    }

    // ShadowOffset
    if (baseTextField->GetShadowOffset() != GetShadowOffset())
    {
        node->Set("shadowoffset", GetShadowOffset());
    }

    // Text align
    if (baseTextField->GetTextAlign() != GetTextAlign())
    {
        node->SetNodeToMap("textalign", loader->GetAlignNodeValue(GetTextAlign()));
    }
    
    // Text use rtl align
    if (baseTextField->GetTextUseRtlAlign() != GetTextUseRtlAlign())
    {
        node->Set("textUseRtlAlign", this->GetTextUseRtlAlign());
    }
    

    // Draw Type must be overwritten fot UITextField.
    UIControlBackground::eDrawType drawType = GetBackground()->GetDrawType();
    
    if (baseTextField->GetBackground()->GetDrawType() != drawType)
    {
        node->Set("drawType", loader->GetDrawTypeNodeValue(drawType));
    }

    // Is password
    if (baseTextField->IsPassword() != IsPassword())
    {
        node->Set("isPassword", IsPassword());
    }

    // Keyboard customization params.
    if (baseTextField->GetAutoCapitalizationType() != GetAutoCapitalizationType())
    {
        node->Set("autoCapitalizationType", GetAutoCapitalizationType());
    }
    if (baseTextField->GetAutoCorrectionType() != GetAutoCorrectionType())
    {
        node->Set("autoCorrectionType", GetAutoCorrectionType());
    }
    if (baseTextField->GetSpellCheckingType() != GetSpellCheckingType())
    {
        node->Set("spellCheckingType", GetSpellCheckingType());
    }
    if (baseTextField->GetKeyboardAppearanceType() != GetKeyboardAppearanceType())
    {
        node->Set("keyboardAppearanceType", GetKeyboardAppearanceType());
    }
    if (baseTextField->GetKeyboardType() != GetKeyboardType())
    {
        node->Set("keyboardType", GetKeyboardType());
    }
    if (baseTextField->GetReturnKeyType() != GetReturnKeyType())
    {
        node->Set("returnKeyType", GetReturnKeyType());
    }
    if (baseTextField->IsEnableReturnKeyAutomatically() != IsEnableReturnKeyAutomatically())
    {
        node->Set("enableReturnKeyAutomatically", IsEnableReturnKeyAutomatically());
    }

    // Max length.
    if (baseTextField->GetMaxLength() != GetMaxLength())
    {
        node->Set("maxLength", GetMaxLength());
    }

    return node;
}

List<UIControl*>& UITextField::GetRealChildren()
{
    List<UIControl*>& realChildren = UIControl::GetRealChildren();
#if !defined(DAVA_TEXTFIELD_USE_NATIVE)
    realChildren.remove(staticText);
#endif
    return realChildren;
}

UITextField* UITextField::Clone()
{
    UITextField *t = new UITextField();
    t->CopyDataFrom(this);
    return t;
}

void UITextField::CopyDataFrom(UIControl *srcControl)
{
    UIControl::CopyDataFrom(srcControl);
    UITextField* t = (UITextField*) srcControl;
        
    cursorTime = t->cursorTime;
    showCursor = t->showCursor;
    isPassword = t->isPassword;
    SetText(t->text);
    SetRect(t->GetRect());
    
    cursorBlinkingTime = t->cursorBlinkingTime;
#if !defined(DAVA_TEXTFIELD_USE_NATIVE)
    SafeRelease(staticText);
    if (t->staticText != nullptr)
    {
        staticText = (UIStaticText*)t->staticText->Clone();
        AddControl(staticText);
    }
    if (t->textFont != nullptr)
    {
        SetFont(t->textFont);
    }
#endif

    SetAutoCapitalizationType(t->GetAutoCapitalizationType());
    SetAutoCorrectionType(t->GetAutoCorrectionType());
    SetSpellCheckingType(t->GetSpellCheckingType());
    SetKeyboardAppearanceType(t->GetKeyboardAppearanceType());
    SetKeyboardType(t->GetKeyboardType());
    SetReturnKeyType(t->GetReturnKeyType());
    SetEnableReturnKeyAutomatically(t->IsEnableReturnKeyAutomatically());
    SetTextUseRtlAlign(t->GetTextUseRtlAlign());
    SetMaxLength(t->GetMaxLength());
}
    
void UITextField::SetIsPassword(bool isPassword_)
{
    isPassword = isPassword_;
    needRedraw = true;
    
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->SetIsPassword(isPassword_);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetIsPassword(isPassword_);
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->SetIsPassword(isPassword_);
#endif
}
    
bool UITextField::IsPassword() const
{
    return isPassword;
}
    
WideString UITextField::GetVisibleText() const
{
    if (!isPassword)
        return text;
    
    return WideString(text.length(), L'*');
}
    
int32 UITextField::GetAutoCapitalizationType() const
{
    return autoCapitalizationType;
}

void UITextField::SetAutoCapitalizationType(int32 value)
{
    autoCapitalizationType = (eAutoCapitalizationType)value;
#if defined(__DAVAENGINE_IPHONE__)
    textFieldiPhone->SetAutoCapitalizationType(value);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetAutoCapitalizationType(value);
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->SetAutoCapitalizationType(value);
#endif
}

int32 UITextField::GetAutoCorrectionType() const
{
    return autoCorrectionType;
}

void UITextField::SetAutoCorrectionType(int32 value)
{
    autoCorrectionType = (eAutoCorrectionType)value;
#if defined(__DAVAENGINE_IPHONE__)
    textFieldiPhone->SetAutoCorrectionType(value);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetAutoCorrectionType(value);
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->SetAutoCorrectionType(value);
#endif
}

int32 UITextField::GetSpellCheckingType() const
{
    return spellCheckingType;
}

void UITextField::SetSpellCheckingType(int32 value)
{
    spellCheckingType = (eSpellCheckingType)value;
#if defined(__DAVAENGINE_IPHONE__)
    textFieldiPhone->SetSpellCheckingType(value);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetSpellCheckingType(value);
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->SetSpellCheckingType(value);
#endif
}

int32 UITextField::GetKeyboardAppearanceType() const
{
    return keyboardAppearanceType;
}

void UITextField::SetKeyboardAppearanceType(int32 value)
{
    keyboardAppearanceType = (eKeyboardAppearanceType)value;
#if defined(__DAVAENGINE_IPHONE__)
    textFieldiPhone->SetKeyboardAppearanceType(value);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetKeyboardAppearanceType(value);
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->SetKeyboardAppearanceType(value);
#endif
}

int32 UITextField::GetKeyboardType() const
{
    return keyboardType;
}

void UITextField::SetKeyboardType(int32 value)
{
    keyboardType = (eKeyboardType)value;
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->SetKeyboardType(value);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetKeyboardType(value);
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->SetKeyboardType(value);
#endif
}

int32 UITextField::GetReturnKeyType() const
{
    return returnKeyType;
}

void UITextField::SetReturnKeyType(int32 value)
{
    returnKeyType = (eReturnKeyType)value;
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->SetReturnKeyType(value);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetReturnKeyType(value);
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->SetReturnKeyType(value);
#endif
}

bool UITextField::IsEnableReturnKeyAutomatically() const
{
    return enableReturnKeyAutomatically;
}

void UITextField::SetEnableReturnKeyAutomatically(bool value)
{
    enableReturnKeyAutomatically = value;
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->SetEnableReturnKeyAutomatically(value);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetEnableReturnKeyAutomatically(value);
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->SetEnableReturnKeyAutomatically(value);
#endif
}

void UITextField::SetInputEnabled(bool isEnabled, bool hierarchic)
{
    UIControl::SetInputEnabled(isEnabled, hierarchic);
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->SetInputEnabled(isEnabled);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetInputEnabled(isEnabled);
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->SetInputEnabled(isEnabled);
#endif
}

void UITextField::SetRenderToTexture(bool value)
{
    // Workaround! Users need scrolling of large texts in
    // multiline mode so we have to disable render into texture
    if (isMultiline_)
    {
        value = false;
    }
#if defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetRenderToTexture(value);
#elif defined(__DAVAENGINE_IPHONE__)
    textFieldiPhone->SetRenderToTexture(value);
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->SetRenderToTexture(value);
#endif
}

bool UITextField::IsRenderToTexture() const
{
#if defined(__DAVAENGINE_ANDROID__)
    return textFieldAndroid->IsRenderToTexture();
#elif defined(__DAVAENGINE_IPHONE__)
    return textFieldiPhone->IsRenderToTexture();
#elif defined(__DAVAENGINE_WIN_UAP__)
    return textFieldWinUAP->IsRenderToTexture();
#else
    return false;
#endif
}

uint32 UITextField::GetCursorPos()
{
#ifdef __DAVAENGINE_IPHONE__
    return textFieldiPhone->GetCursorPos();
#elif defined(__DAVAENGINE_ANDROID__)
    return textFieldAndroid->GetCursorPos();
#elif defined(__DAVAENGINE_WIN_UAP__)
    return textFieldWinUAP->GetCursorPos();
#endif
    return 0;
}

void UITextField::SetCursorPos(uint32 pos)
{
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->SetCursorPos(pos);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetCursorPos(pos);
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->SetCursorPos(pos);
#endif
}

void UITextField::SetMaxLength(int32 newMaxLength)
{
    maxLength = Max(-1, newMaxLength); //-1 valid value
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->SetMaxLength(maxLength);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetMaxLength(maxLength);
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->SetMaxLength(maxLength);
#endif
}

int32 UITextField::GetMaxLength() const
{
    return maxLength;
}

void UITextField::WillBecomeVisible()
{
    UIControl::WillBecomeVisible();

#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->SetVisible(visible);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetVisible(visible);
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->SetVisible(visible);
#else
    staticText->SetVisible(visible);
#endif
}

void UITextField::WillBecomeInvisible()
{
    UIControl::WillBecomeInvisible();

#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->SetVisible(false);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetVisible(false);
#elif defined(__DAVAENGINE_WIN_UAP__)
    textFieldWinUAP->SetVisible(false);
#else
    staticText->SetVisible(false);
#endif
}

String UITextField::GetFontPresetName() const
{
    String name;
    Font* font = GetFont();
    if (font != nullptr)
    {
        name = FontManager::Instance()->GetFontName(font);
    }
    return name;
}

void UITextField::SetFontByPresetName(const String &presetName)
{
    Font *font = nullptr;
    if (!presetName.empty())
    {
        font = FontManager::Instance()->GetFont(presetName);
    }

    SetFont(font);
    if (font)
    {
        SetFontSize((float32)font->GetFontHeight());
    }
}

}   // namespace DAVA
