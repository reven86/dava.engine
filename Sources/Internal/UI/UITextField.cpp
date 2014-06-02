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
#include "Utils/StringFormat.h"
#include "Utils/Utils.h"
#include "UI/UIYamlLoader.h"
#include "UI/UIControlSystem.h"
#include "Render/2D/FontManager.h"
#include "UI/UISystemKeyboard.h"

#include "Input/KeyboardDevice.h"
#if defined(__DAVAENGINE_IPHONE__)
//#include "Platform/TemplateiOS/"
#elif defined(__DAVAENGINE_ANDROID__)
#include "Platform/TemplateAndroid/UITextFieldImplAndroid.h"
#else
#include "Platform/UITextFieldImpl_Custom.h"
#endif

namespace DAVA 
{

void UITextFieldDelegate::TextFieldShouldReturn(UITextField * /*textField*/)
{
}

void UITextFieldDelegate::TextFieldShouldCancel(UITextField * /*textField*/)
{
}

void UITextFieldDelegate::TextFieldLostFocus(UITextField * /*textField*/)
{
}

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
,	delegate(NULL)
,	autoCapitalizationType(AUTO_CAPITALIZATION_TYPE_SENTENCES)
,	autoCorrectionType(AUTO_CORRECTION_TYPE_DEFAULT)
,	spellCheckingType(SPELL_CHECKING_TYPE_DEFAULT)
,	keyboardAppearanceType(KEYBOARD_APPEARANCE_DEFAULT)
,	keyboardType(KEYBOARD_TYPE_DEFAULT)
,	returnKeyType(RETURN_KEY_DEFAULT)
,   isPassword(false)
,	enableReturnKeyAutomatically(false)
,	align(ALIGN_LEFT|ALIGN_VCENTER)
{
#if defined(__DAVAENGINE_IPHONE__)
    textFieldImpl = new UITextFieldImpl_iOS(this);
#elif defined(__DAVAENGINE_ANDROID__)
    textFieldImpl = new UITextFieldImpl_Android(this);
#else
    textFieldImpl = new UITextFieldImpl_Custom(this);
#endif
	
}

UITextField::~UITextField()
{
	SafeDelete(textFieldImpl);
}

void UITextField::OpenKeyboard()
{
    textFieldImpl->OpenKeyboard();
}

void UITextField::CloseKeyboard()
{
    textFieldImpl->CloseKeyboard();
}
	
void UITextField::Update(float32 timeElapsed)
{
	Rect rect = GetGeometricData().GetUnrotatedRect();//GetRect(true);
	textFieldImpl->UpdateRect(rect, timeElapsed);
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
    UIControlSystem::Instance()->GetUISystemKeyboard()->AddListener( this );
	textFieldImpl->ShowField();
}

void UITextField::WillDisappear()
{
	textFieldImpl->HideField();
    UIControlSystem::Instance()->GetUISystemKeyboard()->RemoveListener( this );
}
    
void UITextField::OnFocused()
{
	textFieldImpl->OpenKeyboard();
}

void UITextField::OnFocusLost(UIControl *newFocus)
{
	textFieldImpl->CloseKeyboard();

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
    textFieldImpl->SetFont( font );
}

Color UITextField::GetTextColor() const
{
    Color textColor;
    textFieldImpl->GetTextColor( textColor );
    return textColor;
}

void UITextField::SetTextColor(const Color& fontColor)
{
    textFieldImpl->SetTextColor(fontColor);
}

void UITextField::SetTextAlign(int32 align)
{
    textFieldImpl->SetTextAlign(align);
}

void UITextField::SetFontSize(float32 size)
{
    textFieldImpl->SetFontSize(size);
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

void UITextField::SetText(const WideString & text)
{
	textFieldImpl->SetText(text);
}

WideString UITextField::GetText() const
{
    WideString text;
	textFieldImpl->GetText(text);
	return text;
}
    
Font* UITextField::GetFont()
{
    return textFieldImpl->GetFont();
}

int32 UITextField::GetTextAlign() const
{
    return align;
}

void UITextField::Input(UIEvent *currentInput)
{
    textFieldImpl->Input(currentInput);
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
}

YamlNode * UITextField::SaveToYamlNode(UIYamlLoader * loader)
{
    YamlNode *node = UIControl::SaveToYamlNode(loader);

    //Temp variable
    VariantType *nodeValue = new VariantType();

    //Text
    nodeValue->SetWideString(GetText());
    node->Set("text", nodeValue);

    //Font
    //Get font name and put it here
    nodeValue->SetString(FontManager::Instance()->GetFontName(this->GetFont()));
    node->Set("font", nodeValue);
	
	//TextColor
	const Color &textColor = GetTextColor();
	nodeValue->SetColor(textColor);
	node->Set("textcolor", nodeValue);

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
// #if !defined (__DAVAENGINE_ANDROID__) && !defined (__DAVAENGINE_IPHONE__)
// 	realChildren.remove(staticText);
// #endif
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
	UITextField* t = static_cast<UITextField*>(srcControl);

    SetText(t->GetText());
    SetRect(t->GetRect());
    SetIsPassword(t->IsPassword());
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
	textFieldImpl->SetIsPassword(isPassword);
}
    
bool UITextField::IsPassword() const
{
    return isPassword;
}
	
UITextField::eAutoCapitalizationType UITextField::GetAutoCapitalizationType() const
{
	return autoCapitalizationType;
}

void UITextField::SetAutoCapitalizationType(eAutoCapitalizationType value)
{
	autoCapitalizationType = value;
	textFieldImpl->SetAutoCapitalizationType(value);
}

UITextField::eAutoCorrectionType UITextField::GetAutoCorrectionType() const
{
	return autoCorrectionType;
}

void UITextField::SetAutoCorrectionType(eAutoCorrectionType value)
{
	autoCorrectionType = value;
	textFieldImpl->SetAutoCorrectionType(value);
}

UITextField::eSpellCheckingType UITextField::GetSpellCheckingType() const
{
	return spellCheckingType;
}

void UITextField::SetSpellCheckingType(eSpellCheckingType value)
{
	spellCheckingType = value;
	textFieldImpl->SetSpellCheckingType(value);
}

UITextField::eKeyboardAppearanceType UITextField::GetKeyboardAppearanceType() const
{
	return keyboardAppearanceType;
}

void UITextField::SetKeyboardAppearanceType(eKeyboardAppearanceType value)
{
	keyboardAppearanceType = value;
	textFieldImpl->SetKeyboardAppearanceType(value);
}

UITextField::eKeyboardType UITextField::GetKeyboardType() const
{
	return keyboardType;
}

void UITextField::SetKeyboardType(eKeyboardType value)
{
	keyboardType = value;
	textFieldImpl->SetKeyboardType(value);
}

UITextField::eReturnKeyType UITextField::GetReturnKeyType() const
{
	return returnKeyType;
}

void UITextField::SetReturnKeyType(eReturnKeyType value)
{
	returnKeyType = value;
	textFieldImpl->SetReturnKeyType(value);
}

bool UITextField::IsEnableReturnKeyAutomatically() const
{
	return enableReturnKeyAutomatically;
}

void UITextField::SetEnableReturnKeyAutomatically(bool value)
{
	enableReturnKeyAutomatically = value;
	textFieldImpl->SetEnableReturnKeyAutomatically(value);
}

void UITextField::SetInputEnabled(bool isEnabled, bool hierarchic)
{
	UIControl::SetInputEnabled(isEnabled, hierarchic);
	textFieldImpl->SetInputEnabled(isEnabled);
}

uint32 UITextField::GetCursorPos()
{
	return textFieldImpl->GetCursorPos();
}

void UITextField::SetCursorPos(uint32 pos)
{
	textFieldImpl->SetCursorPos(pos);
}

void UITextField::SetVisible(bool isVisible, bool hierarchic)
{
    UIControl::SetVisible(isVisible, hierarchic);

    if (isVisible)
        textFieldImpl->ShowField();
    else
        textFieldImpl->HideField();
}

void UITextField::Draw( const UIGeometricData &geometricData )
{
    UIControl::Draw( geometricData );
    textFieldImpl->Draw();
}

}; // namespace


