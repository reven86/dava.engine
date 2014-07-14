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
#ifdef __DAVAENGINE_ANDROID__
#include "UITextFieldAndroid.h"
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

bool UITextFieldDelegate::TextFieldKeyPressed(UITextField * /*textField*/, int32 /*replacementLocation*/, int32 /*replacementLength*/, const WideString & /*replacementString*/)
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
,	cursorBlinkingTime(0.0f)
#if !defined (__DAVAENGINE_ANDROID__) && !defined (__DAVAENGINE_IPHONE__)
,   textFont(NULL)
,   staticText(NULL)
#endif
,   isPassword(false)
,	autoCapitalizationType(AUTO_CAPITALIZATION_TYPE_SENTENCES)
,	autoCorrectionType(AUTO_CORRECTION_TYPE_DEFAULT)
,	spellCheckingType(SPELL_CHECKING_TYPE_DEFAULT)
,	keyboardAppearanceType(KEYBOARD_APPEARANCE_DEFAULT)
,	keyboardType(KEYBOARD_TYPE_DEFAULT)
,	returnKeyType(RETURN_KEY_DEFAULT)
,	enableReturnKeyAutomatically(false)
,   showNativeControl(false)
{
#if defined(__DAVAENGINE_ANDROID__)
	textFieldAndroid = new UITextFieldAndroid(this);
#elif defined(__DAVAENGINE_IPHONE__)
	textFieldiPhone = new UITextFieldiPhone(this);
#else
    staticText = new UIStaticText(Rect(0,0,GetRect().dx, GetRect().dy));
    AddControl(staticText);
    
    staticText->SetSpriteAlign(ALIGN_LEFT | ALIGN_BOTTOM);
#endif
    
    cursorTime = 0;
    showCursor = true;    
}

UITextField::UITextField()
:   delegate(NULL)
,   cursorBlinkingTime(0.f)
#if !defined (__DAVAENGINE_ANDROID__) && !defined (__DAVAENGINE_IPHONE__)
,   textFont(NULL)
,   staticText(NULL)
#endif
,   isPassword(false)
,	autoCapitalizationType(AUTO_CAPITALIZATION_TYPE_SENTENCES)
,	autoCorrectionType(AUTO_CORRECTION_TYPE_DEFAULT)
,	spellCheckingType(SPELL_CHECKING_TYPE_DEFAULT)
,	keyboardAppearanceType(KEYBOARD_APPEARANCE_DEFAULT)
,	keyboardType(KEYBOARD_TYPE_DEFAULT)
,	returnKeyType(RETURN_KEY_DEFAULT)
,	enableReturnKeyAutomatically(false)
{
#if defined (__DAVAENGINE_ANDROID__)
	textFieldAndroid = new UITextFieldAndroid(this);
#elif defined(__DAVAENGINE_IPHONE__)
	textFieldiPhone = new UITextFieldiPhone(this);
#else
    staticText = new UIStaticText(Rect(0,0,GetRect().dx, GetRect().dy));
    AddControl(staticText);
    
    staticText->SetSpriteAlign(ALIGN_LEFT | ALIGN_BOTTOM);
#endif
    
    cursorTime = 0;
    showCursor = true;
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
    if (showNativeControl)
    {
#ifdef __DAVAENGINE_IPHONE__
        textFieldiPhone->ShowField();
#elif defined(__DAVAENGINE_ANDROID__)
        textFieldAndroid->ShowField();
#endif
        showNativeControl = false;
    }
    
#ifdef __DAVAENGINE_IPHONE__
	Rect rect = GetGeometricData().GetUnrotatedRect();//GetRect(true);
	textFieldiPhone->UpdateRect(rect);
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
    showNativeControl = true;
}

void UITextField::WillDisappear()
{
#ifdef __DAVAENGINE_IPHONE__
    textFieldiPhone->HideField();
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->HideField();
#endif
    showNativeControl = false;
}
    
void UITextField::OnFocused()
{
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->OpenKeyboard();
#elif defined(__DAVAENGINE_ANDROID__)
	textFieldAndroid->OpenKeyboard();
#endif
}
    
void UITextField::OnFocusLost(UIControl *newFocus)
{
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

void UITextField::SetFontSize(float size)
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
#endif
}
    
void UITextField::SetText(const WideString & _text)
{
	this->text = _text;
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->SetText(text);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetText(text);
#endif

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
    
Font* UITextField::GetFont()
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
        txt.replace(replacementLocation, replacementLength, replacementString);
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
    //InitAfterYaml();

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
    YamlNode *node = UIControl::SaveToYamlNode(loader);

    //Temp variable
    VariantType *nodeValue = new VariantType();

    //Text
    nodeValue->SetWideString(this->GetText());
    node->Set("text", nodeValue);

    //Font
    //Get font name and put it here
    nodeValue->SetString(FontManager::Instance()->GetFontName(this->GetFont()));
    node->Set("font", nodeValue);
	
	//TextColor
	const Color &textColor = GetTextColor();
	nodeValue->SetColor(textColor);
	node->Set("textcolor", nodeValue);

	// ShadowColor
	const Color &shadowColor = GetShadowColor();
	nodeValue->SetColor(shadowColor);
	node->Set("shadowcolor", nodeValue);

	// ShadowOffset
	nodeValue->SetVector2(GetShadowOffset());
	node->Set("shadowoffset", nodeValue);

	// Text align
	node->SetNodeToMap("textalign", loader->GetAlignNodeValue(this->GetTextAlign()));

	// Draw Type must be overwritten fot UITextField.
	UIControlBackground::eDrawType drawType =  this->GetBackground()->GetDrawType();
	node->Set("drawType", loader->GetDrawTypeNodeValue(drawType));
    
    // Is password
    node->Set("isPassword", isPassword);

	// Keyboard customization params.
	node->Set("autoCapitalizationType", autoCapitalizationType);
	node->Set("autoCorrectionType", autoCorrectionType);
	node->Set("spellCheckingType", spellCheckingType);
	node->Set("keyboardAppearanceType", keyboardAppearanceType);
	node->Set("keyboardType", keyboardType);
	node->Set("returnKeyType", returnKeyType);
	node->Set("enableReturnKeyAutomatically", enableReturnKeyAutomatically);

    SafeDelete(nodeValue);
    
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

UIControl* UITextField::Clone()
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
    
    WideString text = this->text;
    text.replace(0, text.length(), text.length(), L'*');
    return text;
}
	
UITextField::eAutoCapitalizationType UITextField::GetAutoCapitalizationType() const
{
	return autoCapitalizationType;
}

void UITextField::SetAutoCapitalizationType(eAutoCapitalizationType value)
{
	autoCapitalizationType = value;
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->SetAutoCapitalizationType(value);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetAutoCapitalizationType(value);
#endif
}

UITextField::eAutoCorrectionType UITextField::GetAutoCorrectionType() const
{
	return autoCorrectionType;
}

void UITextField::SetAutoCorrectionType(eAutoCorrectionType value)
{
	autoCorrectionType = value;
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->SetAutoCorrectionType(value);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetAutoCorrectionType(value);
#endif
}

UITextField::eSpellCheckingType UITextField::GetSpellCheckingType() const
{
	return spellCheckingType;
}

void UITextField::SetSpellCheckingType(eSpellCheckingType value)
{
	spellCheckingType = value;
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->SetSpellCheckingType(value);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetSpellCheckingType(value);
#endif
}

UITextField::eKeyboardAppearanceType UITextField::GetKeyboardAppearanceType() const
{
	return keyboardAppearanceType;
}

void UITextField::SetKeyboardAppearanceType(eKeyboardAppearanceType value)
{
	keyboardAppearanceType = value;
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->SetKeyboardAppearanceType(value);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetKeyboardAppearanceType(value);
#endif
}

UITextField::eKeyboardType UITextField::GetKeyboardType() const
{
	return keyboardType;
}

void UITextField::SetKeyboardType(eKeyboardType value)
{
	keyboardType = value;
#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->SetKeyboardType(value);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldAndroid->SetKeyboardType(value);
#endif
}

UITextField::eReturnKeyType UITextField::GetReturnKeyType() const
{
	return returnKeyType;
}

void UITextField::SetReturnKeyType(eReturnKeyType value)
{
	returnKeyType = value;
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

void UITextField::SetVisible(bool isVisible, bool hierarchic)
{
    UIControl::SetVisible(isVisible, hierarchic);

#ifdef __DAVAENGINE_IPHONE__
	textFieldiPhone->SetVisible(isVisible);
#elif defined(__DAVAENGINE_ANDROID__)
	textFieldAndroid->SetVisible(isVisible);
#endif
}
}; // namespace


