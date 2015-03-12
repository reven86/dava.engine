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




#include "UIStaticTextMetadata.h"
#include "EditorFontManager.h"

#include "StringUtils.h"
#include "ColorHelper.h"

using namespace DAVA;

UIStaticTextMetadata::UIStaticTextMetadata(QObject* parent) :
    UITextControlMetadata(parent)
{
}

UIStaticText* UIStaticTextMetadata::GetActiveStaticText() const
{
    return static_cast<UIStaticText*>(GetActiveUIControl());
}

Font * UIStaticTextMetadata::GetFont() const
{
    if (VerifyActiveParamID())
    {
        return GetActiveStaticText()->GetFont();
    }
    return EditorFontManager::Instance()->GetDefaultFont();
}

void UIStaticTextMetadata::SetFont(Font * font)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    if (font)
    {
        //DF-3435 correct font size is set in font preset, not in control
        // font size is defined in font preset and can be changed only by modifying font preset
        //font->SetSize(GetFontSize());
        
        //TODO: font should be set correctly, remove this workaround
        Font* localizedFont = EditorFontManager::Instance()->GetLocalizedFont(font);
        if(font != localizedFont)
        {
            Logger::Warning("UIStaticTextMetadata::SetFont font=%p, but localizedFont=%p for locale=%s", font, localizedFont, LocalizationSystem::Instance()->GetCurrentLocale().c_str());
        }
        
        GetActiveStaticText()->SetFont(localizedFont);
    }
}

void UIStaticTextMetadata::SetLocalizedTextKey(const QString& value)
{
    UITextControlMetadata::SetLocalizedTextKey(value);

    // Update the control with the value.
    WideString localizationValue = LocalizationSystem::Instance()->GetLocalizedString(QStrint2WideString(value));
    GetActiveStaticText()->SetText(localizationValue);
}

float UIStaticTextMetadata::GetFontSize() const
{
    if (VerifyActiveParamID())
    {
        Font *font = GetActiveStaticText()->GetFont();
        if (font)
        {
            //TODO: font should be set correctly, remove this workaround
            Font* localizedFont = EditorFontManager::Instance()->GetLocalizedFont(font);
            if(localizedFont)
            {
                return localizedFont->GetSize();
            }
            
            return font->GetSize();
        }
    }
    return -1.0f;
}

//DF-3435 font size is defined in font preset and can be changed only by modifying font preset
//void UIStaticTextMetadata::SetFontSize(float fontSize)
//{
//    if (!VerifyActiveParamID())
//    {
//        return;
//    }
//
//    Font *font = GetActiveStaticText()->GetFont();
//    if (font)
//    {
//        Font* newFont = font->Clone();
//        newFont->SetSize(fontSize);
//        GetActiveStaticText()->SetFont(newFont);
//        newFont->Release();
//    }
//}

// Initialize the control(s) attached.
void UIStaticTextMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
    UIControlMetadata::InitializeControl(controlName, position);
    
    int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        UIStaticText* staticText = static_cast<UIStaticText*>(this->treeNodeParams[i].GetUIControl());

        //TODO: remove default font, or make it a default font preset
        staticText->SetFont(EditorFontManager::Instance()->GetDefaultFont());
        staticText->GetBackground()->SetDrawType(UIControlBackground::DRAW_ALIGNED);
        staticText->SetTextAlign(ALIGN_HCENTER | ALIGN_VCENTER);

        // Initialize both control text and localization key.
        WideString controlText = StringToWString(staticText->GetName());

        HierarchyTreeNode* activeNode = GetTreeNode(i);
        staticText->SetText(controlText);

        // Static text is not state-aware.
        activeNode->GetExtraData().SetLocalizationKey(controlText, this->GetReferenceState());
    }
}

void UIStaticTextMetadata::UpdateExtraData(HierarchyTreeNodeExtraData& extraData, eExtraDataUpdateStyle updateStyle)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    // Static Text has one and only state.
    UIControl::eControlState state = UIControl::STATE_NORMAL;
    UpdateStaticTextExtraData(GetActiveStaticText(), state, extraData, updateStyle);
}

QColor UIStaticTextMetadata::GetFontColor() const
{
    if (!VerifyActiveParamID())
    {
        return QColor();
    }

	return ColorHelper::DAVAColorToQTColor(GetActiveStaticText()->GetTextColor());
}

void UIStaticTextMetadata::SetFontColor(const QColor& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	GetActiveStaticText()->SetTextColor(ColorHelper::QTColorToDAVAColor(value));
}

float UIStaticTextMetadata::GetShadowOffsetX() const
{
	if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
	
	return GetActiveStaticText()->GetShadowOffset().x;
}

void UIStaticTextMetadata::SetShadowOffsetX(float offset)
{
	if (!VerifyActiveParamID())
    {
        return;
    }
	Vector2 shadowOffset = GetOffsetX(GetActiveStaticText()->GetShadowOffset(), offset);
	GetActiveStaticText()->SetShadowOffset(shadowOffset);
}

float UIStaticTextMetadata::GetShadowOffsetY() const
{
	if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
	
	return GetActiveStaticText()->GetShadowOffset().y;
}

void UIStaticTextMetadata::SetShadowOffsetY(float offset)
{
	if (!VerifyActiveParamID())
    {
        return;
    }	
	Vector2 shadowOffset = GetOffsetY(GetActiveStaticText()->GetShadowOffset(), offset);
	GetActiveStaticText()->SetShadowOffset(shadowOffset);
}

QColor UIStaticTextMetadata::GetShadowColor() const
{
    if (!VerifyActiveParamID())
    {
    	return QColor();
    }
	
	return ColorHelper::DAVAColorToQTColor(GetActiveStaticText()->GetShadowColor());
}

void UIStaticTextMetadata::SetShadowColor(const QColor& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveStaticText()->SetShadowColor(ColorHelper::QTColorToDAVAColor(value));
}

int UIStaticTextMetadata::GetAlign() const
{
    if (!VerifyActiveParamID())
    {
        return ALIGN_LEFT;
    }
    
    return (int)GetActiveStaticText()->GetAlign();
}

void UIStaticTextMetadata::SetAlign(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveStaticText()->SetAlign((eAlign)value);
}

int UIStaticTextMetadata::GetTextAlign() const
{
    if (!VerifyActiveParamID())
    {
        return ALIGN_LEFT;
    }
    
    return (int)GetActiveStaticText()->GetTextAlign();
}

void UIStaticTextMetadata::SetTextAlign(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveStaticText()->SetTextAlign((eAlign)value);
}

bool UIStaticTextMetadata::GetTextUseRtlAlign()
{
	if (!VerifyActiveParamID())
	{
		return false;
	}
	
	return GetActiveStaticText()->GetTextUseRtlAlign();
}

void UIStaticTextMetadata::SetTextUseRtlAlign(bool value)
{
	if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveStaticText()->SetTextUseRtlAlign(value);
}

bool UIStaticTextMetadata::GetTextMultiline() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }
	
	return GetActiveStaticText()->GetMultiline();
}

void UIStaticTextMetadata::SetTextMultiline(bool value)
{
	if (!VerifyActiveParamID())
    {
        return;
    }

    // Have to keep current MultilineBySymbol value.
    bool curMultilineBySymbolValue = GetActiveStaticText()->GetMultilineBySymbol();
    GetActiveStaticText()->SetMultiline(value, curMultilineBySymbolValue);
}

bool UIStaticTextMetadata::GetTextMultilineBySymbol() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }

    return GetActiveStaticText()->GetMultilineBySymbol();
}

void UIStaticTextMetadata::SetTextMultilineBySymbol(bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    // Have to keep current Multiline value.
    bool curMultilineValue = GetActiveStaticText()->GetMultiline();
    GetActiveStaticText()->SetMultiline(curMultilineValue, value);
}

void UIStaticTextMetadata::SetFittingType(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    UIStaticText* staticText = GetActiveStaticText();
    staticText->SetFittingOption(value);
}

int UIStaticTextMetadata::GetFittingType() const
{
    if (!VerifyActiveParamID())
    {
        return TextBlock::FITTING_DISABLED;
    }
    
    return GetActiveStaticText()->GetFittingOption();
}

int UIStaticTextMetadata::GetTextColorInheritType() const
{
    if (!VerifyActiveParamID() || !GetActiveStaticText()->GetTextBackground())
    {
        return UIControlBackground::COLOR_MULTIPLY_ON_PARENT;
    }

    // Text Color is the base one.
    return GetActiveStaticText()->GetTextBackground()->GetColorInheritType();
}

void UIStaticTextMetadata::SetTextColorInheritType(int value)
{
    if (!VerifyActiveParamID() || !GetActiveStaticText()->GetTextBackground())
    {
        return;
    }

    GetActiveStaticText()->GetTextBackground()->SetColorInheritType((UIControlBackground::eColorInheritType)value);
    GetActiveStaticText()->GetShadowBackground()->SetColorInheritType((UIControlBackground::eColorInheritType)value);
}

int UIStaticTextMetadata::GetTextPerPixelAccuracyType() const
{
    if (!VerifyActiveParamID() || !GetActiveStaticText()->GetTextBackground())
    {
        return UIControlBackground::PER_PIXEL_ACCURACY_DISABLED;
    }
    
    return GetActiveStaticText()->GetTextBackground()->GetPerPixelAccuracyType();
}

void UIStaticTextMetadata::SetTextPerPixelAccuracyType(int value)
{
    if (!VerifyActiveParamID() || !GetActiveStaticText()->GetTextBackground())
    {
        return;
    }
    
    GetActiveStaticText()->GetTextBackground()->SetPerPixelAccuracyType((UIControlBackground::ePerPixelAccuracyType)value);
    GetActiveStaticText()->GetShadowBackground()->SetPerPixelAccuracyType((UIControlBackground::ePerPixelAccuracyType)value);
}

QRectF UIStaticTextMetadata::GetTextMargins() const
{
    if (!VerifyActiveParamID() || !GetActiveStaticText()->GetTextBackground())
    {
        return QRectF();
    }
    
    const UIControlBackground::UIMargins* margins = GetActiveStaticText()->GetTextBackground()->GetMargins();
    if (!margins)
    {
        return QRectF();
    }

    return UIMarginsToQRectF(margins);
}

void UIStaticTextMetadata::SetTextMargins(const QRectF& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    UIControlBackground::UIMargins margins = QRectFToUIMargins(value);
    GetActiveStaticText()->SetMargins(&margins);
}

float UIStaticTextMetadata::GetTextLeftMargin() const
{
    return GetTextMargins().left();
}

void UIStaticTextMetadata::SetTextLeftMargin(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    UIControlBackground::UIMargins margins = GetTextMarginsToUpdate();
    margins.left = value;
    GetActiveStaticText()->SetMargins(&margins);
}

float UIStaticTextMetadata::GetTextTopMargin() const
{
    return GetTextMargins().top();
}

void UIStaticTextMetadata::SetTextTopMargin(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    UIControlBackground::UIMargins margins = GetTextMarginsToUpdate();
    margins.top = value;
    GetActiveStaticText()->SetMargins(&margins);
}

float UIStaticTextMetadata::GetTextRightMargin() const
{
    return GetTextMargins().width();
}

void UIStaticTextMetadata::SetTextRightMargin(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    UIControlBackground::UIMargins margins = GetTextMarginsToUpdate();
    margins.right = value;
    GetActiveStaticText()->SetMargins(&margins);
}

float UIStaticTextMetadata::GetTextBottomMargin() const
{
    return GetTextMargins().height();
}

void UIStaticTextMetadata::SetTextBottomMargin(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    UIControlBackground::UIMargins margins = GetTextMarginsToUpdate();
    margins.bottom = value;
    GetActiveStaticText()->SetMargins(&margins);
}

UIControlBackground::UIMargins UIStaticTextMetadata::GetTextMarginsToUpdate(UIControl::eControlState /* state */) const
{
    if (!VerifyActiveParamID() || !GetActiveStaticText()->GetTextBackground())
    {
        return UIControlBackground::UIMargins();
    }
    
    const UIControlBackground::UIMargins* textMargins = GetActiveStaticText()->GetTextBackground()->GetMargins();
    if (!textMargins)
    {
        return UIControlBackground::UIMargins();
    }
    
    return *textMargins;
}
