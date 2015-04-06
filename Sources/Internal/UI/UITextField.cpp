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
#include "Base/ObjectFactory.h"
#include "Utils/StringFormat.h"
#include "Utils/Utils.h"
#include "Input/KeyboardDevice.h"
#include "UI/UIYamlLoader.h"
#include "UI/UIControlSystem.h"
#include "Render/2D/FontManager.h"
#include "FileSystem/YamlNode.h"
#ifdef __DAVAENGINE_ANDROID__
#include "UITextFieldAndroid.h"
#include "Utils/UTF8Utils.h"
#endif

extern void CreateTextField(DAVA::UITextField *);
extern void ReleaseTextField();
extern void OpenKeyboard();
extern void CloseKeyboard();

namespace DAVA 
{

void UITextFieldDelegate::TextFieldShouldReturn(UITextField * /*textField*/)
{
}

void UITextFieldDelegate::TextFieldShouldCancel(UITextField * /*textField*/)
{
};
    
void UITextFieldDelegate::TextFieldLostFocus(UITextField * /*textField*/)
{
};
    
void UITextFieldDelegate::TextFieldOnTextChanged(UITextField * /*textField*/, const WideString& /*newText*/, const WideString& /*oldText*/)
{
};

bool UITextFieldDelegate::TextFieldKeyPressed(UITextField * /*textField*/, int32 /*replacementLocation*/, int32 /*replacementLength*/, WideString & /*replacementString*/)
{
	return true;
}
    
bool UITextFieldDelegate::IsTextFieldShouldSetFocusedOnAppear(UITextField * /*textField*/)
{
	return false;
}
	
bool UITextFieldDelegate::IsTextFieldCanLostFocus(UITextField * textField)
{
	return true;
}
	
void UITextFieldDelegate::OnKeyboardShown(const Rect& /*keyboardRect*/)
{
}

void UITextFieldDelegate::OnKeyboardHidden()
{
}
    
UITextField::UITextField(const Rect &rect, bool rectInAbsoluteCoordinates/*= false*/)
:	UIControl(rect, rectInAbsoluteCoordinates)
,	text()
,	delegate(0)
,	cursorBlinkingTime{0.0f}
,   isRenderToTexture{false}
#if !defined (__DAVAENGINE_ANDROID__) && !defined (__DAVAENGINE_IPHONE__)
,   staticText(NULL)
,   textFont(NULL)
#endif
{
#if defined(__DAVAENGINE_ANDROID__)
	textFieldAndroid = new UITextFieldAndroid(this);
    textFieldAndroid->SetVisible(false);
#elif defined(__DAVAENGINE_IPHONE__)
	textFieldiPhone = new UITextFieldiPhone(*this);
    textFieldiPhone->SetVisible(false);
#else
    staticText = new UIStaticText(Rect(0,0,GetRect().dx, GetRect().dy));
    staticText->SetVisible(false);
    AddControl(staticText);
    staticText->SetSpriteAlign(ALIGN_LEFT | ALIGN_BOTTOM);
#endif
    
    cursorTime = 0;
    showCursor = true;

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
    SetTextUseRtlAlign(false);
    
    SetMaxLength(-1);
    
    
    SetIsPassword(false);
    SetTextColor(GetTextColor());
    SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
    
    SetFontSize(26); //12 is default size for IOS
    
    SetText(L"");
    SetRenderToTexture(true);
}

//void UITextField::InitAfterYaml()
//{
//#ifdef __DAVAENGINE_IPHONE__
//	textFieldiPhone = new UITextFieldiPhone(this);
//#else
//    
//    staticText = new UIStaticText(Rect(0,0,GetRect().dx, GetRect().dy));
//    staticText->SetFont(textFont);
//    AddControl(staticText);
//
//    staticText->GetBackground()->SetAlign(ALIGN_LEFT | ALIGN_BOTTOM);
//#endif
//
//    cursorTime = 0;
//    showCursor = true;
//}
	
UITextField::~UITextField()
{
#if defined (__DAVAENGINE_ANDROID__)
	SafeDelete(textFieldAndroid);
#elif defined (__DAVAENGINE_IPHONE__)
	SafeDelete(textFieldiPhone);
#else
    SafeRelease(textFont);

    RemoveAllControls();
    SafeRelease(staticText);
#endif
}

void UITextField::OpenKeyboard()
{
    // automatically disable render to texture on open virtual keyboard
    SetRenderToTexture(false);
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->OpenKeyboard();
#elif defined(__DAVAENGINE_ANDROID__)
	textFieldAndroid->OpenKeyboard();
#endif
}

void UITextField::CloseKeyboard()
{
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->CloseKeyboard();
#elif defined(__DAVAENGINE_ANDROID__)
	textFieldAndroid->CloseKeyboard();
#endif
}
	
void UITextField::Update(float32 timeElapsed)
{
#ifdef __DAVAENGINE_IPHONE__
    // Calling UpdateRect with allowNativeControlMove set to true
	textFieldiPhone->UpdateRect(GetGeometricData().GetUnrotatedRect());
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->UpdateRect(GetGeometricData().GetUnrotatedRect());
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

	if(this == UIControlSystem::Instance()->GetFocusedControl())
	{
        WideString txt = GetVisibleText();
        txt += showCursor ? L"_" : L" ";
        staticText->SetText(txt);
	}
	else
    {
        staticText->SetText(GetVisibleText());
    }
    needRedraw = false;
#endif
}


void UITextField::WillAppear()
{
    if (delegate && delegate->IsTextFieldShouldSetFocusedOnAppear(this)) 
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
#endif
}
    
void UITextField::OnFocusLost(UIControl *newFocus)
{
    SetRenderToTexture(true);
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->CloseKeyboard();
#elif defined(__DAVAENGINE_ANDROID__)
	textFieldAndroid->CloseKeyboard();
#endif
    if (delegate) 
    {
        delegate->TextFieldLostFocus(this);
    }
}

bool UITextField::IsLostFocusAllowed(UIControl *newFocus)
{
    if (delegate)
    {
        return delegate->IsTextFieldCanLostFocus(this);
    }
    return true;
}

void UITextField::ReleaseFocus()
{
	if(this == UIControlSystem::Instance()->GetFocusedControl())
	{
		UIControlSystem::Instance()->SetFocusedControl(NULL, true);
	}
}
    
void UITextField::SetFont(Font * font)
{
#if !defined (__DAVAENGINE_IPHONE__) && !defined (__DAVAENGINE_ANDROID__)
    if (font == textFont)
    {
        return;
    }

    SafeRelease(textFont);
    textFont = SafeRetain(font);
    staticText->SetFont(textFont);
#endif
}

void UITextField::SetTextColor(const Color& fontColor)
{
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->SetTextColor(fontColor);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetTextColor(fontColor);
#else
    staticText->SetTextColor(fontColor);
#endif
}


void UITextField::SetShadowOffset(const DAVA::Vector2 &offset)
{
#if !defined (__DAVAENGINE_ANDROID__) && !defined (__DAVAENGINE_IPHONE__)
	staticText->SetShadowOffset(offset);
#endif
}
	
void UITextField::SetShadowColor(const Color& color)
{
#if !defined (__DAVAENGINE_ANDROID__) && !defined (__DAVAENGINE_IPHONE__)
	staticText->SetShadowColor(color);
#endif
}

void UITextField::SetTextAlign(int32 align)
{
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->SetTextAlign(align);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetTextAlign(align);
#else
    staticText->SetTextAlign(align);
#endif	
}

void UITextField::SetTextUseRtlAlign(bool useRtlAlign)
{
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->SetTextUseRtlAlign(useRtlAlign);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetTextUseRtlAlign(useRtlAlign);
#else
    staticText->SetTextUseRtlAlign(useRtlAlign);
#endif
}

void UITextField::SetFontSize(float32 size)
{
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->SetFontSize(size);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetFontSize(size);
#endif
}

void UITextField::SetDelegate(UITextFieldDelegate * _delegate)
{
	delegate = _delegate;
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
#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)
    staticText->SetSize(newSize);
#elif defined(__DAVAENGINE_IPHONE__)
    textFieldiPhone->OnSetSize(newSize);
#endif
}
    
void UITextField::SetPosition(const DAVA::Vector2 &position)
{
    UIControl::SetPosition(position);
#if defined(__DAVAENGINE_IPHONE__)
    textFieldiPhone->OnSetPosition(position);
#endif
}
    
void UITextField::SetText(const WideString & _text)
{
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->SetText(_text);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetText(_text);
#else
    if (delegate && text != _text)
    {
        delegate->TextFieldOnTextChanged(this, _text, text);
    }
#endif
    text = _text;

    needRedraw = true;
}

const WideString & UITextField::GetText()
{
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->GetText(text);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->GetText(text);
#endif
	
	return text;
}
    
Font* UITextField::GetFont() const
{
#if defined (__DAVAENGINE_ANDROID__) || defined (__DAVAENGINE_IPHONE__)
    return NULL;
#else
    return textFont;
#endif
    
}

const Color &UITextField::GetTextColor() const
{
#if defined (__DAVAENGINE_ANDROID__) || defined (__DAVAENGINE_IPHONE__)
    return Color::White;
#else
    return staticText ? staticText->GetTextColor() : Color::White;
#endif
}

Vector2 UITextField::GetShadowOffset() const
{
#if defined (__DAVAENGINE_ANDROID__) || defined (__DAVAENGINE_IPHONE__)
    return Vector2(0, 0);
#else
    return staticText ? staticText->GetShadowOffset() : Vector2(0,0);
#endif
}

const Color &UITextField::GetShadowColor() const
{
#if defined (__DAVAENGINE_ANDROID__) || defined (__DAVAENGINE_IPHONE__)
    return Color::White;
#else
    return staticText ? staticText->GetShadowColor() : Color::White;
#endif
}

int32 UITextField::GetTextAlign() const
{
#ifdef __DAVAENGINE_IPHONE__
    return textFieldiPhone ? textFieldiPhone->GetTextAlign() : ALIGN_HCENTER|ALIGN_VCENTER;
#elif defined(__DAVAENGINE_ANDROID__)
    return textFieldAndroid ? textFieldAndroid->GetTextAlign() : ALIGN_HCENTER|ALIGN_VCENTER;
#else
    return staticText ? staticText->GetTextAlign() : ALIGN_HCENTER|ALIGN_VCENTER;
#endif
}

bool UITextField::GetTextUseRtlAlign() const
{
#ifdef __DAVAENGINE_IPHONE__
    return textFieldiPhone ? textFieldiPhone->GetTextUseRtlAlign() : false;
#elif defined(__DAVAENGINE_ANDROID__)
    return textFieldAndroid ? textFieldAndroid->GetTextUseRtlAlign() : false;
#else
    return staticText ? staticText->GetTextUseRtlAlign() : false;
#endif
}

void UITextField::Input(UIEvent *currentInput)
{
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    // nothing to do
#else

//	if(currentInput->phase == UIEvent::PHASE_BEGAN)
//	{
//        UIControlSystem::Instance()->SetFocusedControl(this, true);
//	}

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

#if !defined (__DAVAENGINE_ANDROID__) && !defined (__DAVAENGINE_IPHONE__)
	if(staticText)
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
		SetTextUseRtlAlign(textUseRtlAlign->AsBool());
	}
    //InitAfterYaml();

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

List<UIControl* >& UITextField::GetRealChildren()
{
	List<UIControl* >& realChildren = UIControl::GetRealChildren();
#if !defined (__DAVAENGINE_ANDROID__) && !defined (__DAVAENGINE_IPHONE__)
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
#if !defined (__DAVAENGINE_ANDROID__) && !defined (__DAVAENGINE_IPHONE__)
	if (t->staticText)
	{
		staticText = (UIStaticText*)t->staticText->Clone();
		AddControl(staticText);
	}
	if (t->textFont)
		SetFont(t->textFont);
#endif

	SetAutoCapitalizationType(t->GetAutoCapitalizationType());
	SetAutoCorrectionType(t->GetAutoCorrectionType());
	SetSpellCheckingType(t->GetSpellCheckingType());
	SetKeyboardAppearanceType(t->GetKeyboardAppearanceType());
	SetKeyboardType(t->GetKeyboardType());
	SetReturnKeyType(t->GetReturnKeyType());
	SetEnableReturnKeyAutomatically(t->IsEnableReturnKeyAutomatically());
	SetTextUseRtlAlign(t->GetTextUseRtlAlign());
}
    
void UITextField::SetIsPassword(bool isPassword)
{
    this->isPassword = isPassword;
    needRedraw = true;
	
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->SetIsPassword(isPassword);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetIsPassword(isPassword);
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
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->SetAutoCapitalizationType(value);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetAutoCapitalizationType(value);
#endif
}

int32 UITextField::GetAutoCorrectionType() const
{
	return autoCorrectionType;
}

void UITextField::SetAutoCorrectionType(int32 value)
{
	autoCorrectionType = (eAutoCorrectionType)value;
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->SetAutoCorrectionType(value);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetAutoCorrectionType(value);
#endif
}

int32 UITextField::GetSpellCheckingType() const
{
	return spellCheckingType;
}

void UITextField::SetSpellCheckingType(int32 value)
{
	spellCheckingType = (eSpellCheckingType)value;
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->SetSpellCheckingType(value);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetSpellCheckingType(value);
#endif
}

int32 UITextField::GetKeyboardAppearanceType() const
{
	return keyboardAppearanceType;
}

void UITextField::SetKeyboardAppearanceType(int32 value)
{
	keyboardAppearanceType = (eKeyboardAppearanceType)value;
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->SetKeyboardAppearanceType(value);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetKeyboardAppearanceType(value);
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
#endif
}

void UITextField::SetInputEnabled(bool isEnabled, bool hierarchic)
{
	UIControl::SetInputEnabled(isEnabled, hierarchic);
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->SetInputEnabled(isEnabled);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetInputEnabled(isEnabled);
#endif
}

void UITextField::SetRenderToTexture(bool value)
{
    // disable this functionality
    value = false;
#ifdef __DAVAENGINE_WIN32__
    // do nothing
#elif defined(__DAVAENGINE_MACOS__)
    // do nothing
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetRenderToTexture(value);
#elif defined(__DAVAENGINE_IPHONE__)
    textFieldiPhone->SetRenderToTexture(value);
#else
#error "implement new platform"
#endif
}
    
bool UITextField::IsRenderToTexture() const
{
#ifdef __DAVAENGINE_WIN32__
    return false;
#elif defined(__DAVAENGINE_MACOS__)
    return false;
#elif defined(__DAVAENGINE_ANDROID__)
    return textFieldAndroid->IsRenderToTexture();
#elif defined(__DAVAENGINE_IPHONE__)
    return textFieldiPhone->IsRenderToTexture();
#else
    static_assert(false, "implement new platform");
#endif
}

uint32 UITextField::GetCursorPos()
{
#ifdef __DAVAENGINE_IPHONE__
	return textFieldiPhone->GetCursorPos();
#elif defined(__DAVAENGINE_ANDROID__)
	return textFieldAndroid->GetCursorPos();
#endif
    // TODO! implement for other OS!
    return 0;
}

void UITextField::SetCursorPos(uint32 pos)
{
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->SetCursorPos(pos);
#elif defined(__DAVAENGINE_ANDROID__)
	textFieldAndroid->SetCursorPos(pos);
#endif
    // TODO! implement for other OS!
}

void UITextField::SetMaxLength(int32 maxLength)
{
    this->maxLength = maxLength;
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->SetMaxLength(maxLength);
#elif defined(__DAVAENGINE_ANDROID__)
	textFieldAndroid->SetMaxLength(maxLength);
#endif
	// TODO! implement for other OS!
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
#else
    staticText->SetVisible(false);
#endif
}
    

String UITextField::GetFontPresetName() const
{
    Font *font = GetFont();
    if (!font)
        return "";
    return FontManager::Instance()->GetFontName(font);
}

void UITextField::SetFontByPresetName( const String &presetName )
{
    Font *font = NULL;

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

}; // namespace


