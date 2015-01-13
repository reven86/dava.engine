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



#include "UITextFieldMetadata.h"
#include "EditorFontManager.h"

#include "StringUtils.h"
#include "ColorHelper.h"

using namespace DAVA;

UITextFieldMetadata::UITextFieldMetadata(QObject* parent) :
    UITextControlMetadata(parent)
{
}

UITextField* UITextFieldMetadata::GetActiveUITextField() const
{
    return static_cast<UITextField*>(GetActiveUIControl());
}

QString UITextFieldMetadata::GetText() const
{
    if (!VerifyActiveParamID())
    {
        return QString();
    }
    return WideString2QString(GetActiveUITextField()->GetText());
}


void UITextFieldMetadata::SetText(const QString& text)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUITextField()->SetText(QStrint2WideString(text));
}

Font * UITextFieldMetadata::GetFont() const
{
    if (VerifyActiveParamID())
    {
        return GetActiveUITextField()->GetFont();
    }
    return EditorFontManager::Instance()->GetDefaultFont();
}

void UITextFieldMetadata::SetFont(Font * font)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    if (font)
    {
        GetActiveUITextField()->SetFont(font);
    }
}

float UITextFieldMetadata::GetFontSize() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }    
   
    Font* font = GetActiveUITextField()->GetFont();
    if (!font)
    {
        return 0.0f;
    }
    
    //TODO: font should be set correctly, remove this workaround
    Font* localizedFont = EditorFontManager::Instance()->GetLocalizedFont(font);
    if(localizedFont)
    {
        return localizedFont->GetSize();
    }
    
    return font->GetSize();
}

//DF-3435 font size is defined in font preset and can be changed only by modifying font preset
//void UITextFieldMetadata::SetFontSize(float fontSize)
//{
//    if (!VerifyActiveParamID())
//    {
//        return;
//    }
//    
//    Font* font = GetActiveUITextField()->GetFont();
//    if (font)
//    {
//        Font* newFont = font->Clone();
//        newFont->SetSize(fontSize);
//        GetActiveUITextField()->SetFont(newFont);
//        newFont->Release();
//    }
//}

QColor UITextFieldMetadata::GetTextColor() const
{
    if (!VerifyActiveParamID())
    {
        return QColor();
    }
	
	return ColorHelper::DAVAColorToQTColor(GetActiveUITextField()->GetTextColor());
}

void UITextFieldMetadata::SetTextColor(const QColor &color)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
    GetActiveUITextField()->SetTextColor(ColorHelper::QTColorToDAVAColor(color));
}

QColor UITextFieldMetadata::GetFontColor() const
{
	return GetTextColor();
}

void UITextFieldMetadata::SetFontColor(const QColor& value)
{
	SetTextColor(value);
}

float UITextFieldMetadata::GetShadowOffsetX() const
{
	if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
	
	return GetActiveUITextField()->GetShadowOffset().x;	
}

void UITextFieldMetadata::SetShadowOffsetX(float offset)
{
	if (!VerifyActiveParamID())
    {
        return;
    }
	
	Vector2 shadowOffset = GetOffsetX(GetActiveUITextField()->GetShadowOffset(), offset);
	GetActiveUITextField()->SetShadowOffset(shadowOffset);
}

float UITextFieldMetadata::GetShadowOffsetY() const
{
	if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
	
	return GetActiveUITextField()->GetShadowOffset().y;
}

void UITextFieldMetadata::SetShadowOffsetY(float offset)
{
	if (!VerifyActiveParamID())
    {
        return;
    }
	
	Vector2 shadowOffset = GetOffsetY(GetActiveUITextField()->GetShadowOffset(), offset);
	GetActiveUITextField()->SetShadowOffset(shadowOffset);
}

QColor UITextFieldMetadata::GetShadowColor() const
{
    if (!VerifyActiveParamID())
    {
        return QColor();
    }
	
	return ColorHelper::DAVAColorToQTColor(GetActiveUITextField()->GetShadowColor());
}

void UITextFieldMetadata::SetShadowColor(const QColor& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUITextField()->SetShadowColor(ColorHelper::QTColorToDAVAColor(value));
}

int UITextFieldMetadata::GetTextAlign() const
{
	if (!VerifyActiveParamID())
	{
		return ALIGN_HCENTER|ALIGN_VCENTER;
	}
	
	return GetActiveUITextField()->GetTextAlign();
}

void UITextFieldMetadata::SetTextAlign(int32 align)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUITextField()->SetTextAlign(align);
}

bool UITextFieldMetadata::GetTextUseRtlAlign()
{
	if (!VerifyActiveParamID())
	{
		return false;
	}
	
	return GetActiveUITextField()->GetTextUseRtlAlign();
}

void UITextFieldMetadata::SetTextUseRtlAlign(bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUITextField()->SetTextUseRtlAlign(value);
}

bool UITextFieldMetadata::GetTextMultiline() const
{
    return false; // Multiline for textfield not implemented
}

void UITextFieldMetadata::SetTextMultiline(bool value)
{
	Q_UNUSED(value);
    // Multiline for textfield not implemented
}

bool UITextFieldMetadata::GetTextMultilineBySymbol() const
{
    return false; // Multiline for textfield not implemented
}

void UITextFieldMetadata::SetTextMultilineBySymbol(bool value)
{
	Q_UNUSED(value);
    // Multiline for textfield not implemented
}

bool UITextFieldMetadata::GetIsPassword() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }
	
	return GetActiveUITextField()->IsPassword();
}

void UITextFieldMetadata::SetIsPassword(bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUITextField()->SetIsPassword(value);
}

int UITextFieldMetadata::GetAutoCapitalizationType() const
{
	if (!VerifyActiveParamID())
	{
		return 0;
	}
	
	return GetActiveUITextField()->GetAutoCapitalizationType();
}

void UITextFieldMetadata::SetAutoCapitalizationType(int value)
{
	if (!VerifyActiveParamID())
	{
		return;
	}
	
	return GetActiveUITextField()->SetAutoCapitalizationType((DAVA::UITextField::eAutoCapitalizationType) value);
}

int UITextFieldMetadata::GetAutoCorrectionType() const
{
	if (!VerifyActiveParamID())
	{
		return 0;
	}
	
	return GetActiveUITextField()->GetAutoCorrectionType();
}

void UITextFieldMetadata::SetAutoCorrectionType(int value)
{
	if (!VerifyActiveParamID())
	{
		return;
	}
	
	return GetActiveUITextField()->SetAutoCorrectionType((DAVA::UITextField::eAutoCorrectionType) value);
}

int UITextFieldMetadata::GetSpellCheckingType() const
{
	if (!VerifyActiveParamID())
	{
		return 0;
	}
	
	return GetActiveUITextField()->GetSpellCheckingType();
}

void UITextFieldMetadata::SetSpellCheckingType(int value)
{
	if (!VerifyActiveParamID())
	{
		return;
	}
	
	return GetActiveUITextField()->SetSpellCheckingType((DAVA::UITextField::eSpellCheckingType) value);
}

int UITextFieldMetadata::GetKeyboardAppearanceType() const
{
	if (!VerifyActiveParamID())
	{
		return 0;
	}
	
	return GetActiveUITextField()->GetKeyboardAppearanceType();
}

void UITextFieldMetadata::SetKeyboardAppearanceType(int value)
{
	if (!VerifyActiveParamID())
	{
		return;
	}
	
	return GetActiveUITextField()->SetKeyboardAppearanceType((DAVA::UITextField::eKeyboardAppearanceType) value);
}

int UITextFieldMetadata::GetKeyboardType() const
{
	if (!VerifyActiveParamID())
	{
		return 0;
	}
	
	return GetActiveUITextField()->GetKeyboardType();
}

void UITextFieldMetadata::SetKeyboardType(int value)
{
	if (!VerifyActiveParamID())
	{
		return;
	}
	
	return GetActiveUITextField()->SetKeyboardType((DAVA::UITextField::eKeyboardType) value);
}

int UITextFieldMetadata::GetReturnKeyType() const
{
	if (!VerifyActiveParamID())
	{
		return 0;
	}
	
	return GetActiveUITextField()->GetReturnKeyType();
}

void UITextFieldMetadata::SetReturnKeyType(int value)
{
	if (!VerifyActiveParamID())
	{
		return;
	}
	
	return GetActiveUITextField()->SetReturnKeyType((DAVA::UITextField::eReturnKeyType) value);
}

bool UITextFieldMetadata::GetIsReturnKeyAutomatically() const
{
	if (!VerifyActiveParamID())
	{
		return false;
	}
	
	return GetActiveUITextField()->IsEnableReturnKeyAutomatically();
}

void UITextFieldMetadata::SetIsReturnKeyAutomatically(bool value)
{
	if (!VerifyActiveParamID())
	{
		return;
	}
	
	return GetActiveUITextField()->SetEnableReturnKeyAutomatically(value);
}

int UITextFieldMetadata::GetMaxLength() const
{
	if (!VerifyActiveParamID())
	{
		return -1;
	}
	
	return GetActiveUITextField()->GetMaxLength();
}

void UITextFieldMetadata::SetMaxLength(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUITextField()->SetMaxLength(value);
}

// Initialize the control(s) attached.
void UITextFieldMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
    UIControlMetadata::InitializeControl(controlName, position);
    
    int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        UITextField* textField = static_cast<UITextField*>(this->treeNodeParams[i].GetUIControl());
        
        textField->SetFont(EditorFontManager::Instance()->GetDefaultFont());
        textField->GetBackground()->SetDrawType(UIControlBackground::DRAW_ALIGNED);
        
        // Initialize both control text and localization key.
        WideString controlText = StringToWString(textField->GetName());
        
        HierarchyTreeNode* activeNode = GetTreeNode(i);
        textField->SetText(controlText);
        
        activeNode->GetExtraData().SetLocalizationKey(controlText, this->GetReferenceState());
    }
}
