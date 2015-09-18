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
#else
#include "UI/UIStaticText.h"
namespace DAVA
{
class TextFieldPlatformImpl : public UIStaticText
{
protected:
    ~TextFieldPlatformImpl() override = default;

public:
    TextFieldPlatformImpl(const Rect& rect = Rect(), bool rectInAbsoluteCoordinates = false)
        : UIStaticText(rect, rectInAbsoluteCoordinates)
    {
    }

    TextFieldPlatformImpl* Clone() override
    {
        TextFieldPlatformImpl* t = new TextFieldPlatformImpl(GetRect());
        t->CopyDataFrom(this);
        return t;
    }
};
} // end namespace DAVA
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
    edit = new TextFieldPlatformImpl(this);
    edit->SetVisible(false);
#elif defined(__DAVAENGINE_IPHONE__)
    edit = new TextFieldPlatformImpl(this);
    edit->SetVisible(false);
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit = new TextFieldPlatformImpl(this);
    edit->SetVisible(false);
#else
    edit = new TextFieldPlatformImpl(Rect(0, 0, GetRect().dx, GetRect().dy));
    edit->SetVisible(false);
    AddControl(edit);
    edit->SetSpriteAlign(ALIGN_LEFT | ALIGN_BOTTOM);
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
    SafeDelete(edit);
#elif defined (__DAVAENGINE_IPHONE__)
    SafeDelete(edit);
#elif defined(__DAVAENGINE_WIN_UAP__)
    SafeDelete(edit);
#else
    SafeRelease(textFont);

    RemoveAllControls();
    SafeRelease(edit);
#endif
}

void UITextField::OpenKeyboard()
{
#ifdef __DAVAENGINE_IPHONE__
    edit->OpenKeyboard();
#elif defined(__DAVAENGINE_ANDROID__)
    edit->OpenKeyboard();
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit->OpenKeyboard();
#endif
}

void UITextField::CloseKeyboard()
{
#ifdef __DAVAENGINE_IPHONE__
    edit->CloseKeyboard();
#elif defined(__DAVAENGINE_ANDROID__)
    edit->CloseKeyboard();
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit->CloseKeyboard();
#endif
}

void UITextField::Update(float32 timeElapsed)
{
#ifdef __DAVAENGINE_IPHONE__
    // Calling UpdateRect with allowNativeControlMove set to true
    edit->UpdateRect(GetGeometricData().GetUnrotatedRect());
#elif defined(__DAVAENGINE_ANDROID__)
    edit->UpdateRect(GetGeometricData().GetUnrotatedRect());
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit->UpdateRect(GetGeometricData().GetUnrotatedRect());
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

    // Use NO_REQUIRED_SIZE to notify edit->SetText that we don't want
    // to enable of any kind of static text fitting
    static const Vector2 NO_REQUIRED_SIZE = Vector2(-1, -1);

    if(this == UIControlSystem::Instance()->GetFocusedControl())
    {
        WideString txt = GetVisibleText();
        txt += showCursor ? L"_" : L" ";
        edit->SetText(txt, NO_REQUIRED_SIZE);
    }
    else
    {
        edit->SetText(GetVisibleText(), NO_REQUIRED_SIZE);
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
    edit->ShowField();
    edit->SetVisible(IsOnScreen());
#endif
}

void UITextField::WillDisappear()
{
#ifdef __DAVAENGINE_IPHONE__
    edit->HideField();
#endif
}
    
void UITextField::OnFocused()
{
    SetRenderToTexture(false);
#ifdef __DAVAENGINE_IPHONE__
    edit->OpenKeyboard();
#elif defined(__DAVAENGINE_ANDROID__)
    edit->OpenKeyboard();
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit->OpenKeyboard();
#endif
}

void UITextField::SetFocused()
{
    UIControlSystem::Instance()->SetFocusedControl(this, true);
}

void UITextField::OnFocusLost(UIControl *newFocus)
{
    SetRenderToTexture(true);
#ifdef __DAVAENGINE_IPHONE__
    edit->CloseKeyboard();
#elif defined(__DAVAENGINE_ANDROID__)
    edit->CloseKeyboard();
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit->CloseKeyboard();
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
    edit->SetFont(textFont);
#endif  // !defined(DAVA_TEXTFIELD_USE_NATIVE)
}

void UITextField::SetTextColor(const Color& fontColor)
{
#ifdef __DAVAENGINE_IPHONE__
    edit->SetTextColor(fontColor);
#elif defined(__DAVAENGINE_ANDROID__)
    edit->SetTextColor(fontColor);
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit->SetTextColor(fontColor);
#else
    edit->SetTextColor(fontColor);
#endif
}

void UITextField::SetShadowOffset(const DAVA::Vector2 &offset)
{
#if !defined(DAVA_TEXTFIELD_USE_NATIVE)
    edit->SetShadowOffset(offset);
#endif
}

void UITextField::SetShadowColor(const Color& color)
{
#if !defined(DAVA_TEXTFIELD_USE_NATIVE)
    edit->SetShadowColor(color);
#endif
}

void UITextField::SetTextAlign(int32 align)
{
#if defined(__DAVAENGINE_IPHONE__)
    edit->SetTextAlign(align);
#elif defined(__DAVAENGINE_ANDROID__)
    edit->SetTextAlign(align);
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit->SetTextAlign(align);
#else
    edit->SetTextAlign(align);
#endif
}

TextBlock::eUseRtlAlign UITextField::GetTextUseRtlAlign() const
{
#ifdef __DAVAENGINE_IPHONE__
    return (edit && edit->GetTextUseRtlAlign()) ? TextBlock::RTL_USE_BY_CONTENT : TextBlock::RTL_DONT_USE;
#elif defined(__DAVAENGINE_ANDROID__)
    return (edit && edit->GetTextUseRtlAlign()) ? TextBlock::RTL_USE_BY_CONTENT : TextBlock::RTL_DONT_USE;
#elif defined(__DAVAENGINE_WIN_UAP__)
    return edit->GetTextUseRtlAlign() ? TextBlock::RTL_USE_BY_CONTENT : TextBlock::RTL_DONT_USE;
#else
    return edit ? edit->GetTextUseRtlAlign() : TextBlock::RTL_DONT_USE;
#endif
}

void UITextField::SetTextUseRtlAlign(TextBlock::eUseRtlAlign useRtlAlign)
{
#ifdef __DAVAENGINE_IPHONE__
    edit->SetTextUseRtlAlign(useRtlAlign == TextBlock::RTL_USE_BY_CONTENT);
#elif defined(__DAVAENGINE_ANDROID__)
    edit->SetTextUseRtlAlign(useRtlAlign == TextBlock::RTL_USE_BY_CONTENT);
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit->SetTextUseRtlAlign(useRtlAlign == TextBlock::RTL_USE_BY_CONTENT);
#else
    edit->SetTextUseRtlAlign(useRtlAlign);
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
    edit->SetFontSize(size);
#elif defined(__DAVAENGINE_ANDROID__)
    edit->SetFontSize(size);
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit->SetFontSize(size);
#endif
}

void UITextField::SetDelegate(UITextFieldDelegate * _delegate)
{
    delegate = _delegate;
#if defined(__DAVAENGINE_WIN_UAP__)
    edit->SetDelegate(_delegate);
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
    edit->SetSize(newSize);
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
        edit->SetMultiline(isMultiline_);
#elif defined(__DAVAENGINE_ANDROID__)
        edit->SetMultiline(isMultiline_);
#elif defined(__DAVAENGINE_WIN_UAP__)
        edit->SetMultiline(isMultiline_);
#else
        edit->SetMultiline(isMultiline_);
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
    edit->SetText(text_);
#elif defined(__DAVAENGINE_ANDROID__)
    edit->SetText(text_);
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit->SetText(text_);
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
    edit->GetText(text);
#elif defined(__DAVAENGINE_ANDROID__)
    edit->GetText(text);
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit->GetText(text);
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
    return edit ? edit->GetTextColor() : Color::White;
#endif
}

Vector2 UITextField::GetShadowOffset() const
{
#if defined(DAVA_TEXTFIELD_USE_NATIVE)
    return Vector2(0, 0);
#else
    return edit ? edit->GetShadowOffset() : Vector2(0, 0);
#endif
}

const Color &UITextField::GetShadowColor() const
{
#if defined(DAVA_TEXTFIELD_USE_NATIVE)
    return Color::White;
#else
    return edit ? edit->GetShadowColor() : Color::White;
#endif
}

int32 UITextField::GetTextAlign() const
{
#ifdef __DAVAENGINE_IPHONE__
    return edit->GetTextAlign();
#elif defined(__DAVAENGINE_ANDROID__)
    return edit->GetTextAlign();
#elif defined(__DAVAENGINE_WIN_UAP__)
    return edit->GetTextAlign();
#else
    return edit->GetTextAlign();
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
        /// macos
        
        if (currentInput->tid == DVKEY_BACKSPACE)
        {
            //TODO: act the same way on iPhone
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
    if (edit != nullptr)
    {
        edit->SetRect(Rect(0, 0, GetRect().dx, GetRect().dy));

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
    realChildren.remove(edit);
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
    UITextField* t = static_cast<UITextField*>(srcControl);

    cursorTime = t->cursorTime;
    showCursor = t->showCursor;
    isPassword = t->isPassword;
    SetText(t->text);
    SetRect(t->GetRect());
    
    cursorBlinkingTime = t->cursorBlinkingTime;
#if !defined(DAVA_TEXTFIELD_USE_NATIVE)
    SafeRelease(edit);
    if (t->edit != nullptr)
    {
        edit = t->edit->Clone();
        AddControl(edit);
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
    edit->SetIsPassword(isPassword_);
#elif defined(__DAVAENGINE_ANDROID__)
    edit->SetIsPassword(isPassword_);
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit->SetIsPassword(isPassword_);
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
    edit->SetAutoCapitalizationType(value);
#elif defined(__DAVAENGINE_ANDROID__)
    edit->SetAutoCapitalizationType(value);
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit->SetAutoCapitalizationType(value);
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
    edit->SetAutoCorrectionType(value);
#elif defined(__DAVAENGINE_ANDROID__)
    edit->SetAutoCorrectionType(value);
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit->SetAutoCorrectionType(value);
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
    edit->SetSpellCheckingType(value);
#elif defined(__DAVAENGINE_ANDROID__)
    edit->SetSpellCheckingType(value);
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit->SetSpellCheckingType(value);
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
    edit->SetKeyboardAppearanceType(value);
#elif defined(__DAVAENGINE_ANDROID__)
    edit->SetKeyboardAppearanceType(value);
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit->SetKeyboardAppearanceType(value);
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
    edit->SetKeyboardType(value);
#elif defined(__DAVAENGINE_ANDROID__)
    edit->SetKeyboardType(value);
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit->SetKeyboardType(value);
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
    edit->SetReturnKeyType(value);
#elif defined(__DAVAENGINE_ANDROID__)
    edit->SetReturnKeyType(value);
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit->SetReturnKeyType(value);
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
    edit->SetEnableReturnKeyAutomatically(value);
#elif defined(__DAVAENGINE_ANDROID__)
    edit->SetEnableReturnKeyAutomatically(value);
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit->SetEnableReturnKeyAutomatically(value);
#endif
}

void UITextField::SetInputEnabled(bool isEnabled, bool hierarchic)
{
    UIControl::SetInputEnabled(isEnabled, hierarchic);
#ifdef __DAVAENGINE_IPHONE__
    edit->SetInputEnabled(isEnabled);
#elif defined(__DAVAENGINE_ANDROID__)
    edit->SetInputEnabled(isEnabled);
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit->SetInputEnabled(isEnabled);
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
    edit->SetRenderToTexture(value);
#elif defined(__DAVAENGINE_IPHONE__)
    edit->SetRenderToTexture(value);
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit->SetRenderToTexture(value);
#endif
}

bool UITextField::IsRenderToTexture() const
{
#if defined(__DAVAENGINE_ANDROID__)
    return edit->IsRenderToTexture();
#elif defined(__DAVAENGINE_IPHONE__)
    return edit->IsRenderToTexture();
#elif defined(__DAVAENGINE_WIN_UAP__)
    return edit->IsRenderToTexture();
#else
    return false;
#endif
}

uint32 UITextField::GetCursorPos()
{
#ifdef __DAVAENGINE_IPHONE__
    return edit->GetCursorPos();
#elif defined(__DAVAENGINE_ANDROID__)
    return edit->GetCursorPos();
#elif defined(__DAVAENGINE_WIN_UAP__)
    return edit->GetCursorPos();
#endif
    return 0;
}

void UITextField::SetCursorPos(uint32 pos)
{
#ifdef __DAVAENGINE_IPHONE__
    edit->SetCursorPos(pos);
#elif defined(__DAVAENGINE_ANDROID__)
    edit->SetCursorPos(pos);
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit->SetCursorPos(pos);
#endif
}

void UITextField::SetMaxLength(int32 newMaxLength)
{
    maxLength = Max(-1, newMaxLength); //-1 valid value
#ifdef __DAVAENGINE_IPHONE__
    edit->SetMaxLength(maxLength);
#elif defined(__DAVAENGINE_ANDROID__)
    edit->SetMaxLength(maxLength);
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit->SetMaxLength(maxLength);
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
    edit->SetVisible(visible);
#elif defined(__DAVAENGINE_ANDROID__)
    edit->SetVisible(visible);
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit->SetVisible(visible);
#else
    edit->SetVisible(visible);
#endif
}

void UITextField::WillBecomeInvisible()
{
    UIControl::WillBecomeInvisible();

#ifdef __DAVAENGINE_IPHONE__
    edit->SetVisible(false);
#elif defined(__DAVAENGINE_ANDROID__)
    edit->SetVisible(false);
#elif defined(__DAVAENGINE_WIN_UAP__)
    edit->SetVisible(false);
#else
    edit->SetVisible(false);
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
