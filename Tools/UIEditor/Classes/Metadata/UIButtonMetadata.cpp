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




#include "UIButtonMetadata.h"
#include "EditorFontManager.h"
#include "UIControlStateHelper.h"
#include "ColorHelper.h"

#include "PropertyNames.h"
#include "StringUtils.h"
#include "StringConstants.h"

using namespace DAVA;

UIButtonMetadata::UIButtonMetadata(QObject* parent) :
    UITextControlMetadata(parent)
{
}

UIButton* UIButtonMetadata::GetActiveUIButton() const
{
    return static_cast<UIButton*>(GetActiveUIControl());
}

void UIButtonMetadata::SetLocalizedTextKey(const QString& value)
{
    if (!VerifyActiveParamID() || !this->GetActiveTreeNode())
    {
        return;
    }

    // Update the control with the value.
	WideString localizationValue = LocalizationSystem::Instance()->GetLocalizedString(QStrint2WideString(value));
    HierarchyTreeNode* node = this->GetActiveTreeNode();

	for(uint32 i = 0; i < GetStatesCount(); ++i)
	{
		// Update both key and value for all the states requested.
		node->GetExtraData().SetLocalizationKey(QStrint2WideString(value), this->uiControlStates[i]);
		GetActiveUIButton()->SetStateText(this->uiControlStates[i], localizationValue);
	}

    UpdatePropertyDirtyFlagForLocalizedText();
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForLocalizedText()
{
    // Compare all the states with reference one.
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);

        bool curStateDirty = (GetLocalizedTextKeyForState(curState) !=
                              GetLocalizedTextKeyForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::LOCALIZED_TEXT_KEY_PROPERTY_NAME, curStateDirty);
    }
}

QString DAVA::UIButtonMetadata::GetLocalizedTextKeyForState( UIControl::eControlState controlState ) const
{
    // Return the localization key from the Hierarchy Tree node.
    HierarchyTreeNode *node = this->GetActiveTreeNode();
    if (node)
    {
        controlState = UIButton::DrawStateToControlState(GetActiveUIButton()->GetActualTextBlockState(UIButton::ControlStateToDrawState(controlState)));
        return WideString2QString(node->GetExtraData().GetLocalizationKey(controlState));
    }
    return QString();
}

Font * UIButtonMetadata::GetFont() const
{
    if (VerifyActiveParamID())
    {
        return GetFontForState(this->uiControlStates[GetActiveStateIndex()]);
    }
    return EditorFontManager::Instance()->GetDefaultFont();
}

void UIButtonMetadata::SetFont(Font * font)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    if (font)
    {
        //TODO: remove this workaround
        Font* localizedFont = EditorFontManager::Instance()->GetLocalizedFont(font);
        
        //localizedFont->SetSize(GetFontSize());
        
		for (uint32 i = 0; i < this->GetStatesCount(); ++i)
		{
			GetActiveUIButton()->SetStateFont(this->uiControlStates[i], localizedFont);
		}

        UpdateExtraDataLocalizationKey();
        UpdatePropertyDirtyFlagForFont();
    }
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForFont()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);

        bool curStateDirty = (GetFontForState(curState) !=
                              GetFontForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::FONT_PROPERTY_NAME, curStateDirty);
    }
}

Font * UIButtonMetadata::GetFontForState(UIControl::eControlState state) const
{
    UIStaticText *buttonText = GetActiveUIButton()->GetStateTextControl(state);
    if (buttonText)
    {
        //return buttonText->GetFont();
        
        //TODO: remove this workaround
        return EditorFontManager::Instance()->GetLocalizedFont(buttonText->GetFont());
    }
    return EditorFontManager::Instance()->GetDefaultFont();
}

float UIButtonMetadata::GetFontSize() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
    
    return GetFontSizeForState(this->uiControlStates[GetActiveStateIndex()]);
}


float UIButtonMetadata::GetFontSizeForState(UIControl::eControlState state) const
{
   UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(state);
   if (referenceButtonText)
    {
        Font* referenceFont = referenceButtonText->GetFont();
        
        //TODO: remove this workaround
        Font* localizedReferenceFont = EditorFontManager::Instance()->GetLocalizedFont(referenceFont);
        
        if (localizedReferenceFont)
        {
            return localizedReferenceFont->GetSize();
        }
    }
    
    return -1.0f;
}

QColor UIButtonMetadata::GetFontColor() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
    
    return GetFontColorForState(this->uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::SetFontColor(const QColor& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		GetActiveUIButton()->SetStateFontColor(this->uiControlStates[i], ColorHelper::QTColorToDAVAColor(value));
	}

    UpdateExtraDataLocalizationKey();
    UpdatePropertyDirtyFlagForFontColor();
}

float UIButtonMetadata::GetShadowOffsetX() const
{
    if (!VerifyActiveParamID())
    {
        return 0.0f;
    }
    
    return GetShadowOffsetXYForState(this->uiControlStates[GetActiveStateIndex()]).x;
}

void UIButtonMetadata::SetShadowOffsetX(float offset)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
        UIControl::eControlState state = uiControlStates[i];
		UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(state);
		if (referenceButtonText)
		{
			Vector2 shadowOffset = GetOffsetX(referenceButtonText->GetShadowOffset(), offset);
            GetActiveUIButton()->SetStateShadowOffset(state, shadowOffset);
		}
	}

    UpdateExtraDataLocalizationKey();
    UpdatePropertyDirtyFlagForShadowOffsetXY();
}
	
float UIButtonMetadata::GetShadowOffsetY() const
{
    if (!VerifyActiveParamID())
    {
        return 0.0f;
    }
    
    return GetShadowOffsetXYForState(this->uiControlStates[GetActiveStateIndex()]).y;
}

void UIButtonMetadata::SetShadowOffsetY(float offset)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
        UIControl::eControlState state = uiControlStates[i];
		UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(state);
		if (referenceButtonText)
		{
			Vector2 shadowOffset = GetOffsetY(referenceButtonText->GetShadowOffset(), offset);
            GetActiveUIButton()->SetStateShadowOffset(state, shadowOffset);
		}
	}

    UpdateExtraDataLocalizationKey();
    UpdatePropertyDirtyFlagForShadowOffsetXY();
}
	
QColor UIButtonMetadata::GetShadowColor() const
{
    if (!VerifyActiveParamID())
    {
        return QColor();
    }
    
    return GetShadowColorForState(this->uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::SetShadowColor(const QColor& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		GetActiveUIButton()->SetStateShadowColor(this->uiControlStates[i], ColorHelper::QTColorToDAVAColor(value));
	}

    UpdateExtraDataLocalizationKey();
    UpdatePropertyDirtyFlagForShadowColor();
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForFontColor()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        
        bool curStateDirty = (GetFontColorForState(curState) !=
                              GetFontColorForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::FONT_COLOR_PROPERTY_NAME, curStateDirty);
    }
}

QColor UIButtonMetadata::GetFontColorForState(UIControl::eControlState state) const
{
    UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(state);
    if (referenceButtonText)
    {
		return ColorHelper::DAVAColorToQTColor(referenceButtonText->GetTextColor());
    }
    
    return QColor();
}

int UIButtonMetadata::GetTextAlignForState(UIControl::eControlState state) const
{
	UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(state);
    if (referenceButtonText)
    {
		return referenceButtonText->GetTextAlign();
    }
    
    return ALIGN_HCENTER|ALIGN_VCENTER;
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForTextAlign()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        
        bool curStateDirty = (GetTextAlignForState(curState) !=
                              GetTextAlignForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::TEXT_ALIGN_PROPERTY_NAME, curStateDirty);
    }
}

bool UIButtonMetadata::GetTextUseRtlAlignForState(UIControl::eControlState state) const
{
	UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(state);
    if (referenceButtonText)
    {
		return referenceButtonText->GetTextUseRtlAlign();
    }
    
    return false;
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForTextUseRtlAlign()
{
	int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        
        bool curStateDirty = (GetTextUseRtlAlignForState(curState) !=
                              GetTextUseRtlAlignForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::TEXT_USE_RTL_ALIGN_PROPERTY_NAME, curStateDirty);
    }

}

bool UIButtonMetadata::GetTextMultilineForState(UIControl::eControlState state) const
{
    UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(state);
    if (referenceButtonText)
    {
        return referenceButtonText->GetMultiline();
    }

    return false;
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForTextMultiline()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);

        bool curStateDirty = (GetTextMultilineForState(curState) != GetTextMultilineForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::TEXT_PROPERTY_MULTILINE, curStateDirty);
    }
}

bool UIButtonMetadata::GetTextMultilineBySymbolForState(UIControl::eControlState state) const
{
    UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(state);
    if (referenceButtonText)
    {
        return referenceButtonText->GetMultilineBySymbol();
    }

    return false;
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForTextMultilineBySymbol()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);

        bool curStateDirty = (GetTextMultilineBySymbolForState(curState) != GetTextMultilineBySymbolForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::TEXT_PROPERTY_MULTILINE_BY_SYMBOL, curStateDirty);
    }
}

void UIButtonMetadata::SetSprite(const QString& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		//If empty string value is used - remove sprite
		if (value.isEmpty())
		{
			GetActiveUIButton()->SetStateSprite(this->uiControlStates[i], NULL);
		}
		else
		{
			GetActiveUIButton()->SetStateSprite(this->uiControlStates[i], value.toStdString());
		}
	}

    UpdatePropertyDirtyFlagForSpriteName();
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForSpriteName()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
            
        bool curStateDirty = (GetSpriteNameForState(curState) !=
                              GetSpriteNameForState(this->GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::SPRITE_PROPERTY_NAME, curStateDirty);
    }
}

QString UIButtonMetadata::GetSpriteNameForState(UIControl::eControlState state) const
{
    Sprite* sprite = GetActiveUIButton()->GetStateSprite(state);
    if (sprite == NULL)
    {
        return StringConstants::NO_SPRITE_IS_SET;
    }

	return QString::fromStdString(sprite->GetRelativePathname().GetFrameworkPath());
}

QString UIButtonMetadata::GetSprite() const
{
    if (!VerifyActiveParamID())
    {
        return QString();
    }
    
    return GetSpriteNameForState(this->uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::SetSpriteFrame(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		Sprite* sprite = GetActiveUIButton()->GetStateSprite(this->uiControlStates[i]);
		if (sprite == NULL)
		{
			continue;
		}
		
		if (sprite->GetFrameCount() <= value)
		{
			// No way to set this frame.
			continue;
		}
		
		GetActiveUIButton()->SetStateFrame(this->uiControlStates[i], value);
	}

    UpdatePropertyDirtyFlagForSpriteFrame();
}

int UIButtonMetadata::GetSpriteFrame() const
{
    if (!VerifyActiveParamID())
    {
        return -1;
    }

    return GetSpriteFrameForState(this->uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForSpriteFrame()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);

        bool curStateDirty = (GetSpriteFrameForState(curState) !=
                              GetSpriteFrameForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::SPRITE_FRAME_PROPERTY_NAME, curStateDirty);
    }
}

int UIButtonMetadata::GetSpriteFrameForState(UIControl::eControlState state) const
{
    return GetActiveUIButton()->GetStateFrame(state);
}

UIControl::eControlState UIButtonMetadata::GetCurrentStateForLocalizedText() const
{
    // UIButton is state-aware, so return current state.
    return this->uiControlStates[GetActiveStateIndex()];
}

QColor UIButtonMetadata::GetColor() const
{
    if (!VerifyActiveParamID())
    {
        return -1;
    }
    
    return GetColorForState(this->uiControlStates[GetActiveStateIndex()]);
}


void UIButtonMetadata::SetColor(const QColor& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		GetActiveUIButton()->SetStateColor(this->uiControlStates[i], ColorHelper::QTColorToDAVAColor(value));
	}
    
    UpdatePropertyDirtyFlagForColor();
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForColor()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        
        bool curStateDirty = (GetColorForState(curState) !=
                              GetColorForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::BACKGROUND_COLOR_PROPERTY_NAME, curStateDirty);
    }
}

QColor UIButtonMetadata::GetColorForState(UIControl::eControlState state) const
{
    UIControlBackground* background = GetActiveUIButton()->GetStateBackground(state);
    if (background)
    {
        return ColorHelper::DAVAColorToQTColor(background->color);
    }
    
    return QColor();
}

int UIButtonMetadata::GetDrawType() const
{
    if (!VerifyActiveParamID())
    {
        return UIControlBackground::DRAW_ALIGNED;
    }
    
    return GetActiveUIButton()->GetStateDrawType(this->uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::SetDrawType(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		GetActiveUIButton()->SetStateDrawType(this->uiControlStates[i], (UIControlBackground::eDrawType)value);
	}
    UpdatePropertyDirtyFlagForDrawType();
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForDrawType()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);

        bool curStateDirty = (GetActiveUIButton()->GetStateDrawType(curState) !=
                              GetActiveUIButton()->GetStateDrawType(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::DRAW_TYPE_PROPERTY_NAME, curStateDirty);
    }
}

int UIButtonMetadata::GetColorInheritType() const
{
    if (!VerifyActiveParamID())
    {
        return UIControlBackground::COLOR_MULTIPLY_ON_PARENT;
    }

    return GetColorInheritTypeForState(this->uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::SetColorInheritType(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		GetActiveUIButton()->SetStateColorInheritType(this->uiControlStates[i],(UIControlBackground::eColorInheritType)value);
	}
    UpdatePropertyDirtyFlagForColorInheritType();
}

int UIButtonMetadata::GetColorInheritTypeForState(UIControl::eControlState state) const
{
    UIControlBackground* background = GetActiveUIButton()->GetStateBackground(state);
    if (!background)
    {
        return UIControlBackground::COLOR_MULTIPLY_ON_PARENT;
    }
    
    return background->GetColorInheritType();
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForColorInheritType()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        
        bool curStateDirty = (GetColorInheritTypeForState(curState) !=
                              GetColorInheritTypeForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::COLOR_INHERIT_TYPE_PROPERTY_NAME, curStateDirty);
    }
}

int UIButtonMetadata::GetPerPixelAccuracyType() const
{
    if (!VerifyActiveParamID())
    {
        return UIControlBackground::PER_PIXEL_ACCURACY_DISABLED;
    }
    
    return GetPerPixelAccuracyTypeForState(this->uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::SetPerPixelAccuracyType(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		GetActiveUIButton()->SetStatePerPixelAccuracyType(this->uiControlStates[i],(UIControlBackground::ePerPixelAccuracyType)value);
	}
    UpdatePropertyDirtyFlagForPerPixelAccuracyType();
}

int UIButtonMetadata::GetPerPixelAccuracyTypeForState(UIControl::eControlState state) const
{
    UIControlBackground* background = GetActiveUIButton()->GetStateBackground(state);
    if (!background)
    {
        return UIControlBackground::PER_PIXEL_ACCURACY_DISABLED;
    }
    
    return background->GetPerPixelAccuracyType();
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForPerPixelAccuracyType()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        
        bool curStateDirty = (GetPerPixelAccuracyTypeForState(curState) !=
                              GetPerPixelAccuracyTypeForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::PER_PIXEL_ACCURACY_TYPE_PROPERTY_NAME, curStateDirty);
    }
}

int UIButtonMetadata::GetAlign() const
{
    if (!VerifyActiveParamID())
    {
        return ALIGN_TOP;
    }
    
    return GetAlignForState(this->uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::SetAlign(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		GetActiveUIButton()->SetStateAlign(this->uiControlStates[i], value);
	}

    UpdatePropertyDirtyFlagForAlign();
}

int UIButtonMetadata::GetAlignForState(UIControl::eControlState state) const
{
	return GetActiveUIButton()->GetStateAlign(state);
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForAlign()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        
        bool curStateDirty = (GetAlignForState(curState) !=
                              GetAlignForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::ALIGN_PROPERTY_NAME, curStateDirty);
    }
}


int UIButtonMetadata::GetSpriteModification() const
{
	if (!VerifyActiveParamID())
	{
		return UIControlBackground::DRAW_ALIGNED;
	}

	return GetSpriteModificationForState(uiControlStates[GetActiveStateIndex()]);
}

int UIButtonMetadata::GetTextAlign() const
{
	if (!VerifyActiveParamID())
	{
		return ALIGN_HCENTER|ALIGN_VCENTER;
	}

	return GetTextAlignForState(this->uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::SetTextAlign(int align)
{
	if (!VerifyActiveParamID())
    {
        return;
    }
	
    for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		GetActiveUIButton()->SetStateTextAlign(this->uiControlStates[i], align);
	}

    UpdateExtraDataLocalizationKey();
	UpdatePropertyDirtyFlagForTextAlign();
}

bool UIButtonMetadata::GetTextUseRtlAlign()
{
	if (!VerifyActiveParamID())
	{
		return false;
	}
	
	return GetTextUseRtlAlignForState(this->uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::SetTextUseRtlAlign(bool value)
{
	if (!VerifyActiveParamID())
    {
        return;
    }
	
    for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		GetActiveUIButton()->SetStateTextUseRtlAlign(this->uiControlStates[i], value);
	}
	
    UpdateExtraDataLocalizationKey();
	UpdatePropertyDirtyFlagForTextUseRtlAlign();
	
}

bool UIButtonMetadata::GetTextMultiline() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }

    return GetTextMultilineForState(this->uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::SetTextMultiline(bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    for (uint32 i = 0; i < this->GetStatesCount(); ++i)
    {
        GetActiveUIButton()->SetStateTextMultiline(this->uiControlStates[i], value);
    }

    UpdateExtraDataLocalizationKey();
    UpdatePropertyDirtyFlagForTextMultiline();
}

bool UIButtonMetadata::GetTextMultilineBySymbol() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }

    return GetTextMultilineBySymbolForState(this->uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::SetTextMultilineBySymbol(bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    for (uint32 i = 0; i < this->GetStatesCount(); ++i)
    {
        GetActiveUIButton()->SetStateTextMultilineBySymbol(this->uiControlStates[i], value);
    }

    UpdateExtraDataLocalizationKey();
    UpdatePropertyDirtyFlagForTextMultilineBySymbol();
}

void UIButtonMetadata::SetSpriteModification(int value)
{
	if (!VerifyActiveParamID())
	{
		return;
	}

	for (uint32 i = 0; i < GetStatesCount(); ++i)
	{
		GetActiveUIButton()->SetStateModification(this->uiControlStates[i],(UIControlBackground::eColorInheritType)value);
	}

	UpdatePropertyDirtyFlagForSpriteModification();
}

int UIButtonMetadata::GetSpriteModificationForState(UIControl::eControlState state) const
{
	UIControlBackground* background = GetActiveUIButton()->GetStateBackground(state);

	if (!background)
	{
		return UIControlBackground::DRAW_ALIGNED;
	}

	return background->GetModification();
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForSpriteModification()
{
	int statesCount = UIControlStateHelper::GetUIControlStatesCount();
	for (int i = 0; i < statesCount; i ++)
	{
		UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);

		bool curStateDirty = (GetSpriteModificationForState(curState) !=
							  GetSpriteModificationForState(GetReferenceState()));
		SetStateDirtyForProperty(curState, PropertyNames::SPRITE_MODIFICATION_PROPERTY_NAME, curStateDirty);
	}
}


// Initialize the control(s) attached.
void UIButtonMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
    UIControlMetadata::InitializeControl(controlName, position);

    int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        UIButton* button = static_cast<UIButton*>(this->treeNodeParams[i].GetUIControl());
        WideString controlText = StringToWString(button->GetName());
        HierarchyTreeNode* activeNode = GetTreeNode(i);
    
        // Define some properties for the reference state.
        UIControl::eControlState refState = GetReferenceState();
        button->SetStateFont(refState, EditorFontManager::Instance()->GetDefaultFont());
        button->SetStateText(refState, controlText);
        button->SetStateTextAlign(refState, ALIGN_HCENTER | ALIGN_VCENTER);
        button->SetStateDrawType(refState, UIControlBackground::DRAW_SCALE_TO_RECT);

        // Button is state-aware.
        activeNode->GetExtraData().SetLocalizationKey(controlText, refState);
    }
}

void UIButtonMetadata::UpdateExtraData(HierarchyTreeNodeExtraData& extraData, eExtraDataUpdateStyle updateStyle)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    UIButton* button = GetActiveUIButton();

    // Button is state-aware.
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int stateID = 0; stateID < statesCount; stateID ++)
    {
        UIControl::eControlState state = UIControlStateHelper::GetUIControlState(stateID);
        UIStaticText* textControl = button->GetTextBlock(UIButton::ControlStateToDrawState(state));
        if (!textControl)
        {
            continue;
        }
        
        UpdateStaticTextExtraData(textControl, state, extraData, updateStyle);
    }
    
    if (updateStyle == UPDATE_EXTRADATA_FROM_CONTROL)
    {
        // Also need to recover Dirty Flags in this case.
        RecoverPropertyDirtyFlags();
    }
}

int UIButtonMetadata::GetFittingType() const
{
    if (!VerifyActiveParamID())
    {
        return TextBlock::FITTING_DISABLED;
    }

    return GetFittingTypeForState(uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::SetFittingType(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
        UIStaticText* buttonText = GetActiveUIButton()->GetStateTextControl(this->uiControlStates[i]);
        if (buttonText)
        {
            buttonText->SetFittingOption(value);
        }
    }

    UpdateExtraDataLocalizationKey();
    UpdatePropertyDirtyFlagForFittingType();
}

int UIButtonMetadata::GetFittingTypeForState(UIControl::eControlState state) const
{
    UIStaticText* buttonText = GetActiveUIButton()->GetStateTextControl(state);
    if (buttonText)
    {
        return buttonText->GetFittingOption();
    }
    
    return TextBlock::FITTING_DISABLED;
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForFittingType()
{
	int statesCount = UIControlStateHelper::GetUIControlStatesCount();
	for (int i = 0; i < statesCount; i ++)
	{
		UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        
		bool curStateDirty = (GetFittingTypeForState(curState) !=
							  GetFittingTypeForState(GetReferenceState()));
		SetStateDirtyForProperty(curState, PropertyNames::TEXT_FITTING_TYPE_PROPERTY_NAME, curStateDirty);
	}
}

float UIButtonMetadata::GetLeftRightStretchCap() const
{
    if (!VerifyActiveParamID())
    {
        return 0.0f;
    }

    return GetLeftRightStretchCapForState(uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::SetLeftRightStretchCap(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
        UIControlBackground* background = GetActiveUIButton()->GetStateBackground(uiControlStates[i]);
        if (background)
        {
            background->SetLeftRightStretchCap(value);
        }
	}

    UpdatePropertyDirtyFlagForLeftRightStretchCap();
}

float UIButtonMetadata::GetTopBottomStretchCap() const
{
    if (!VerifyActiveParamID())
    {
        return 0.0f;
    }
    
    return GetTopBottomStretchCapForState(uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::SetTopBottomStretchCap(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
        UIControlBackground* background = GetActiveUIButton()->GetStateBackground(uiControlStates[i]);
        if (background)
        {
            background->SetTopBottomStretchCap(value);
        }
	}
    
    UpdatePropertyDirtyFlagForTopBottomStretchCap();
}

float UIButtonMetadata::GetLeftRightStretchCapForState(UIControl::eControlState state) const
{
	UIControlBackground* background = GetActiveUIButton()->GetStateBackground(state);
	if (!background)
	{
		return 0.0f;
	}
    
	return background->GetLeftRightStretchCap();
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForLeftRightStretchCap()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        
        bool curStateDirty = (GetLeftRightStretchCapForState(curState) !=
                              GetLeftRightStretchCapForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::STRETCH_HORIZONTAL_PROPERTY_NAME, curStateDirty);
    }
}

float UIButtonMetadata::GetTopBottomStretchCapForState(UIControl::eControlState state) const
{
	UIControlBackground* background = GetActiveUIButton()->GetStateBackground(state);
	if (!background)
	{
		return 0.0f;
	}
    
	return background->GetTopBottomStretchCap();
}

Vector2 UIButtonMetadata::GetShadowOffsetXYForState(UIControl::eControlState state) const
{
	UIStaticText* staticText = GetActiveUIButton()->GetStateTextControl(state);
	if (!staticText)
	{
		return Vector2();
	}

	return staticText->GetShadowOffset();
}

QColor UIButtonMetadata::GetShadowColorForState(UIControl::eControlState state) const
{
    UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(state);
    if (referenceButtonText)
    {
		return ColorHelper::DAVAColorToQTColor(referenceButtonText->GetShadowColor());
    }
    
    return QColor();
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForTopBottomStretchCap()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        
        bool curStateDirty = (GetTopBottomStretchCapForState(curState) !=
                              GetTopBottomStretchCapForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::STRETCH_VERTICAL_PROPERTY_NAME, curStateDirty);
    }
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForShadowOffsetXY()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    const Vector2& refShadowOffset = GetShadowOffsetXYForState(GetReferenceState());

    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        const Vector2& curShadowOffset = GetShadowOffsetXYForState(curState);
        
        SetStateDirtyForProperty(curState, PropertyNames::SHADOW_OFFSET_X, refShadowOffset.x != curShadowOffset.x);
        SetStateDirtyForProperty(curState, PropertyNames::SHADOW_OFFSET_Y, refShadowOffset.y != curShadowOffset.y);
    }
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForShadowColor()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        
        bool curStateDirty = (GetShadowColorForState(curState) !=
                              GetShadowColorForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::SHADOW_COLOR, curStateDirty);
    }
}

int UIButtonMetadata::GetTextColorInheritType() const
{
    if (!VerifyActiveParamID())
    {
        return UIControlBackground::COLOR_IGNORE_PARENT;
    }
    
    return GetTextColorInheritTypeForState(uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::SetTextColorInheritType(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
        UIControl::eControlState state = uiControlStates[i];
        GetActiveUIButton()->SetStateTextColorInheritType(state, (UIControlBackground::eColorInheritType)value);
    }

    UpdateExtraDataLocalizationKey();
    UpdatePropertyDirtyFlagForTextColorInheritType();
}

QRectF UIButtonMetadata::GetMargins() const
{
    if (!VerifyActiveParamID())
    {
        return QRectF();
    }
    
    return GetMarginsForState(uiControlStates[GetActiveStateIndex()]);
}

QRectF UIButtonMetadata::GetMarginsForState(UIControl::eControlState state) const
{
    if (!GetActiveUIButton()->GetStateBackground(state))
    {
        return QRectF();
    }

    const UIControlBackground::UIMargins* margins = GetActiveUIButton()->GetStateBackground(state)->GetMargins();
    return UIMarginsToQRectF(margins);
}

void UIButtonMetadata::SetMargins(const QRectF& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
        UIControl::eControlState state = uiControlStates[i];
 
        UIControlBackground::UIMargins margins = QRectFToUIMargins(value);
        GetActiveUIButton()->SetStateMargins(state, &margins);
    }
    
    UpdatePropertyDirtyFlagForMargins();
}

float UIButtonMetadata::GetLeftMargin() const
{
    return GetMargins().left();
}

void UIButtonMetadata::SetLeftMargin(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
 
    for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
        UIControl::eControlState state = uiControlStates[i];
 
        UIControlBackground::UIMargins margins = GetMarginsToUpdate(state);
        margins.left = value;
        GetActiveUIButton()->SetStateMargins(state, &margins);
    }
    
    UpdatePropertyDirtyFlagForMargins();
}

float UIButtonMetadata::GetTopMargin() const
{
    return GetMargins().top();
}

void UIButtonMetadata::SetTopMargin(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
        UIControl::eControlState state = uiControlStates[i];

        UIControlBackground::UIMargins margins = GetMarginsToUpdate(state);
        margins.top = value;
        GetActiveUIButton()->SetStateMargins(state, &margins);
    }
    
    UpdatePropertyDirtyFlagForMargins();
}

float UIButtonMetadata::GetRightMargin() const
{
    return GetMargins().width();
}

void  UIButtonMetadata::SetRightMargin(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
        UIControl::eControlState state = uiControlStates[i];
        
        UIControlBackground::UIMargins margins = GetMarginsToUpdate(state);
        margins.right = value;
        GetActiveUIButton()->SetStateMargins(state, &margins);
    }
    
    UpdatePropertyDirtyFlagForMargins();
}

float UIButtonMetadata::GetBottomMargin() const
{
    return GetMargins().height();
}

void UIButtonMetadata::SetBottomMargin(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
        UIControl::eControlState state = uiControlStates[i];

        UIControlBackground::UIMargins margins = GetMarginsToUpdate(state);
        margins.bottom = value;
        GetActiveUIButton()->SetStateMargins(state, &margins);
    }
    
    UpdatePropertyDirtyFlagForMargins();
}

QRectF UIButtonMetadata::GetTextMargins() const
{
    if (!VerifyActiveParamID())
    {
        return QRectF();
    }

    return GetTextMarginsForState(uiControlStates[GetActiveStateIndex()]);
}
    
QRectF UIButtonMetadata::GetTextMarginsForState(UIControl::eControlState state) const
{
    if (!GetActiveUIButton()->GetStateTextControl(state))
    {
        return QRectF();
    }
 
    const UIControlBackground::UIMargins* margins = GetActiveUIButton()->GetStateTextControl(state)->GetMargins();
    return UIMarginsToQRectF(margins);
}

void UIButtonMetadata::SetTextMargins(const QRectF& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    for (uint32 i = 0; i < this->GetStatesCount(); ++i)
    {
        UIControl::eControlState state = uiControlStates[i];

        UIControlBackground::UIMargins margins = QRectFToUIMargins(value);
        GetActiveUIButton()->SetStateTextMargins(state, &margins);
    }

    UpdateExtraDataLocalizationKey();
    UpdatePropertyDirtyFlagForTextMargins();
}

float UIButtonMetadata::GetTextLeftMargin() const
{
    return GetTextMargins().left();
}

void UIButtonMetadata::SetTextLeftMargin(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
        UIControl::eControlState state = uiControlStates[i];
        
        UIControlBackground::UIMargins margins = GetTextMarginsToUpdate(state);
        margins.left = value;
        GetActiveUIButton()->SetStateTextMargins(state, &margins);
    }
    
    UpdatePropertyDirtyFlagForTextMargins();
    UpdateExtraDataLocalizationKey();
}

float UIButtonMetadata::GetTextTopMargin() const
{
    return GetTextMargins().top();
}

void UIButtonMetadata::SetTextTopMargin(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
        UIControl::eControlState state = uiControlStates[i];
        
        UIControlBackground::UIMargins margins = GetTextMarginsToUpdate(state);
        margins.top = value;
        GetActiveUIButton()->SetStateTextMargins(state, &margins);
    }
    
    UpdatePropertyDirtyFlagForTextMargins();
    UpdateExtraDataLocalizationKey();
}

float UIButtonMetadata::GetTextRightMargin() const
{
    return GetTextMargins().width();
}

void UIButtonMetadata::SetTextRightMargin(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
        UIControl::eControlState state = uiControlStates[i];
        
        UIControlBackground::UIMargins margins = GetTextMarginsToUpdate(state);
        margins.right = value;
        GetActiveUIButton()->SetStateTextMargins(state, &margins);
    }
    
    UpdatePropertyDirtyFlagForTextMargins();
    UpdateExtraDataLocalizationKey();
}

float UIButtonMetadata::GetTextBottomMargin() const
{
    return GetTextMargins().height();
}

void UIButtonMetadata::SetTextBottomMargin(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
        UIControl::eControlState state = uiControlStates[i];
        
        UIControlBackground::UIMargins margins = GetTextMarginsToUpdate(state);
        margins.bottom = value;
        GetActiveUIButton()->SetStateTextMargins(state, &margins);
    }
    
    UpdatePropertyDirtyFlagForTextMargins();
    UpdateExtraDataLocalizationKey();
}

int UIButtonMetadata::GetTextColorInheritTypeForState(UIControl::eControlState state) const
{
    UIStaticText* textControl = GetActiveUIButton()->GetStateTextControl(state);
    if (textControl)
    {
        return textControl->GetTextBackground()->GetColorInheritType();
    }
    
    return UIControlBackground::COLOR_IGNORE_PARENT;
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForTextColorInheritType()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        
        bool curStateDirty = (GetTextColorInheritTypeForState(curState) !=
                              GetTextColorInheritTypeForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::TEXT_COLOR_INHERIT_TYPE_PROPERTY_NAME, curStateDirty);
    }
}

int UIButtonMetadata::GetTextPerPixelAccuracyType() const
{
   if (!VerifyActiveParamID())
    {
        return UIControlBackground::PER_PIXEL_ACCURACY_DISABLED;
    }
    
    return GetTextPerPixelAccuracyTypeForState(uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::SetTextPerPixelAccuracyType(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
        UIControl::eControlState state = uiControlStates[i];
        GetActiveUIButton()->SetStateTextPerPixelAccuracyType(state, (UIControlBackground::ePerPixelAccuracyType)value);
    }

    UpdateExtraDataLocalizationKey();
    UpdatePropertyDirtyFlagForTextPerPixelAccuracyType();
}

int UIButtonMetadata::GetTextPerPixelAccuracyTypeForState(UIControl::eControlState state) const
{
    UIStaticText* textControl = GetActiveUIButton()->GetStateTextControl(state);
    if (textControl)
    {
        return textControl->GetTextBackground()->GetPerPixelAccuracyType();
    }
    
    return UIControlBackground::PER_PIXEL_ACCURACY_DISABLED;
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForTextPerPixelAccuracyType()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        
        bool curStateDirty = (GetTextPerPixelAccuracyTypeForState(curState) !=
                              GetTextPerPixelAccuracyTypeForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::TEXT_PER_PIXEL_ACCURACY_TYPE_PROPERTY_NAME, curStateDirty);
    }
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForMargins()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    const QRectF& referenceMargins = GetMarginsForState(GetReferenceState());

    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        const QRectF& curMargins = GetMarginsForState(curState);

        SetStateDirtyForProperty(curState, PropertyNames::LEFT_MARGIN_PROPERTY_NAME,
                                 !FLOAT_EQUAL(referenceMargins.left(), curMargins.left()));
        SetStateDirtyForProperty(curState, PropertyNames::TOP_MARGIN_PROPERTY_NAME,
                                 !FLOAT_EQUAL(referenceMargins.top(), curMargins.top()));
        SetStateDirtyForProperty(curState, PropertyNames::RIGHT_MARGIN_PROPERTY_NAME,
                                 !FLOAT_EQUAL(referenceMargins.width(), curMargins.width()));
        SetStateDirtyForProperty(curState, PropertyNames::BOTTOM_MARGIN_PROPERTY_NAME,
                                 !FLOAT_EQUAL(referenceMargins.height(), curMargins.height()));
    }
}

bool UIButtonMetadata::GetMultiline() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }
    
    return GetMultilineForState(uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::SetMultiline(const bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    // Need  to keep multilineBySymbol value.
    bool multilineBySymbol = GetMultilineBySymbolForState(uiControlStates[0]);
    for (uint32 i = 0; i < this->GetStatesCount(); ++i)
    {
        UIControl::eControlState state = uiControlStates[i];
        UIStaticText* stateTextControl = GetActiveUIButton()->GetStateTextControl(state);
        if (stateTextControl)
        {
            stateTextControl->SetMultiline(value, multilineBySymbol);
        }
    }

    UpdateExtraDataLocalizationKey();
    UpdatePropertyDirtyFlagForMultiline();
}

bool UIButtonMetadata::GetMultilineForState(UIControl::eControlState state) const
{
    UIStaticText* textControl = GetActiveUIButton()->GetStateTextControl(state);
    if (textControl)
    {
        return textControl->GetMultiline();
    }

    return false;
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForMultiline()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        
        bool curStateDirty = (GetMultilineForState(curState) !=
                              GetMultilineForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::TEXT_PROPERTY_MULTILINE, curStateDirty);
    }
}

bool UIButtonMetadata::GetMultilineBySymbol() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }
    
    return GetMultilineBySymbolForState(uiControlStates[GetActiveStateIndex()]);
}

void UIButtonMetadata::SetMultilineBySymbol(const bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    // Need  to keep multiline value.
    bool multiline = GetMultilineForState(uiControlStates[0]);

    for (uint32 i = 0; i < this->GetStatesCount(); ++i)
    {
        UIControl::eControlState state = uiControlStates[i];
        UIStaticText* stateTextControl = GetActiveUIButton()->GetStateTextControl(state);
        if (stateTextControl)
        {
            stateTextControl->SetMultiline(multiline, value);
        }
    }
    
    UpdateExtraDataLocalizationKey();
    UpdatePropertyDirtyFlagForMultilineBySymbol();
}

bool UIButtonMetadata::GetMultilineBySymbolForState(UIControl::eControlState state) const
{
    UIStaticText* textControl = GetActiveUIButton()->GetStateTextControl(state);
    if (textControl)
    {
        return textControl->GetMultilineBySymbol();
    }
    
    return false;
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForMultilineBySymbol()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        
        bool curStateDirty = (GetMultilineBySymbolForState(curState) !=
                              GetMultilineBySymbolForState(GetReferenceState()));
        SetStateDirtyForProperty(curState, PropertyNames::TEXT_PROPERTY_MULTILINE_BY_SYMBOL, curStateDirty);
    }
}

void UIButtonMetadata::UpdatePropertyDirtyFlagForTextMargins()
{
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    const QRectF& referenceMargins = GetTextMarginsForState(GetReferenceState());

    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
        const QRectF& curMargins = GetTextMarginsForState(curState);
        
        SetStateDirtyForProperty(curState, PropertyNames::TEXT_LEFT_MARGIN_PROPERTY_NAME,
                                 !FLOAT_EQUAL(referenceMargins.left(), curMargins.left()));
        SetStateDirtyForProperty(curState, PropertyNames::TEXT_TOP_MARGIN_PROPERTY_NAME,
                                 !FLOAT_EQUAL(referenceMargins.top(), curMargins.top()));
        SetStateDirtyForProperty(curState, PropertyNames::TEXT_RIGHT_MARGIN_PROPERTY_NAME,
                                 !FLOAT_EQUAL(referenceMargins.width(), curMargins.width()));
        SetStateDirtyForProperty(curState, PropertyNames::TEXT_BOTTOM_MARGIN_PROPERTY_NAME,
                                 !FLOAT_EQUAL(referenceMargins.height(), curMargins.height()));
    }
}

void UIButtonMetadata::RecoverPropertyDirtyFlags()
{
    UpdatePropertyDirtyFlagForLocalizedText();
    UpdatePropertyDirtyFlagForFont();
    UpdatePropertyDirtyFlagForColor();

    UpdatePropertyDirtyFlagForSpriteName();
    UpdatePropertyDirtyFlagForSpriteFrame();
    
    UpdatePropertyDirtyFlagForDrawType();
    UpdatePropertyDirtyFlagForColorInheritType();
    UpdatePropertyDirtyFlagForPerPixelAccuracyType();
    UpdatePropertyDirtyFlagForAlign();
    
    UpdatePropertyDirtyFlagForFittingType();
    UpdatePropertyDirtyFlagForTextColorInheritType();
    UpdatePropertyDirtyFlagForTextPerPixelAccuracyType();
    
    UpdatePropertyDirtyFlagForLeftRightStretchCap();
    UpdatePropertyDirtyFlagForTopBottomStretchCap();

    UpdatePropertyDirtyFlagForShadowColor();
    UpdatePropertyDirtyFlagForShadowOffsetXY();

    UpdatePropertyDirtyFlagForMargins();
    
    UpdatePropertyDirtyFlagForMultiline();
    UpdatePropertyDirtyFlagForMultilineBySymbol();
}

void UIButtonMetadata::UpdateExtraDataLocalizationKey()
{
    UIButton* button = GetActiveUIButton();
    HierarchyTreeNode* node = this->GetActiveTreeNode();
    if (!node || !button)
    {
        return;
    }

    for(uint32 i = 0; i < GetStatesCount(); ++i)
	{
        UIControl::eControlState curState = uiControlStates[i];
        if (node->GetExtraData().IsLocalizationKeyExist(curState))
        {
            // There is already localization key for this string - no need to update it.
            continue;
        }

        UIButton::eButtonDrawState drawState = button->ControlStateToDrawState(curState);

        // Sanity check to verify whether appropriate textblock was created.
        if (!button->GetTextBlock(drawState))
        {
            continue;
        }

        // Get the reference draw state.
        UIButton::eButtonDrawState refDrawState = button->GetActualTextBlockState(button->GetStateReplacer(drawState));
        const WideString& referenceLocalizationKey = node->GetExtraData().GetLocalizationKey(button->DrawStateToControlState(refDrawState));

        // Update the current localization key with the reference one.
        node->GetExtraData().SetLocalizationKey(referenceLocalizationKey, button->DrawStateToControlState(drawState));
    }
}

UIControlBackground::UIMargins UIButtonMetadata::GetMarginsToUpdate(UIControl::eControlState state) const
{
    if (!VerifyActiveParamID() || !GetActiveUIButton()->GetStateBackground(state))
    {
        return UIControlBackground::UIMargins();
    }

    const UIControlBackground::UIMargins* margins = GetActiveUIButton()->GetStateBackground(state)->GetMargins();
    if (!margins)
    {
        return UIControlBackground::UIMargins();
    }

    return *margins;
}

UIControlBackground::UIMargins UIButtonMetadata::GetTextMarginsToUpdate(UIControl::eControlState state) const
{
    if (!VerifyActiveParamID() || !GetActiveUIButton()->GetStateTextControl(state))
    {
        return UIControlBackground::UIMargins();
    }
    
    const UIControlBackground::UIMargins* margins = GetActiveUIButton()->GetStateTextControl(state)->GetMargins();
    if (!margins)
    {
        return UIControlBackground::UIMargins();
    }
    
    return *margins;
}
