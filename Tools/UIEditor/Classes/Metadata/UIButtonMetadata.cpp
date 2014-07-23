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

Font * UIButtonMetadata::GetFont()
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

//void UIButtonMetadata::SetFontSize(float fontSize)
//{
//    if (!VerifyActiveParamID())
//    {
//        return;
//    }
//
//	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
//	{
//		UIStaticText *buttonText = GetActiveUIButton()->GetStateTextControl(this->uiControlStates[i]);
//		if (!buttonText)
//		{
//			return;
//		}
//    
//		Font *font = buttonText->GetFont();
//		if (!font)
//		{
//			return;
//		}
//
//		Font* newFont = font->Clone();
//		newFont->SetSize(fontSize);
//		buttonText->SetFont(newFont);
//		newFont->Release();
//	}
//
//    UpdatePropertyDirtyFlagForFontSize();
//}

//void UIButtonMetadata::UpdatePropertyDirtyFlagForFontSize()
//{
//    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
//    for (int i = 0; i < statesCount; i ++)
//    {
//        UIControl::eControlState curState = UIControlStateHelper::GetUIControlState(i);
//
//        bool curStateDirty = (GetFontSizeForState(curState) !=
//                              GetFontSizeForState(GetReferenceState()));
//        SetStateDirtyForProperty(curState, PropertyNames::FONT_SIZE_PROPERTY_NAME, curStateDirty);
//    }
//}

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

    UpdatePropertyDirtyFlagForFontColor();
}

float UIButtonMetadata::GetShadowOffsetX() const
{
    if (VerifyActiveParamID())
    {
		UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(this->uiControlStates[GetActiveStateIndex()]);
    	if (referenceButtonText)
    	{
			return referenceButtonText->GetShadowOffset().x;
    	}
	}
    
	return -1.0f;	
}

void UIButtonMetadata::SetShadowOffsetX(float offset)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(this->uiControlStates[i]);
		if (referenceButtonText)
		{
			Vector2 shadowOffset = GetOffsetX(referenceButtonText->GetShadowOffset(), offset);
			referenceButtonText->SetShadowOffset(shadowOffset);
		}
	}
}
	
float UIButtonMetadata::GetShadowOffsetY() const
{
    if (VerifyActiveParamID())
    {
		UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(this->uiControlStates[GetActiveStateIndex()]);
    	if (referenceButtonText)
    	{
			return referenceButtonText->GetShadowOffset().y;
    	}
	}
    
	return -1.0f;	
}

void UIButtonMetadata::SetShadowOffsetY(float offset)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(this->uiControlStates[i]);
		if (referenceButtonText)
		{
			Vector2 shadowOffset = GetOffsetY(referenceButtonText->GetShadowOffset(), offset);
			referenceButtonText->SetShadowOffset(shadowOffset);
		}
	}
}
	
QColor UIButtonMetadata::GetShadowColor() const
{
    if (VerifyActiveParamID())
    {
		UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(this->uiControlStates[GetActiveStateIndex()]);
    	if (referenceButtonText)
    	{
			return ColorHelper::DAVAColorToQTColor(referenceButtonText->GetShadowColor());
    	}
	}
    
	return QColor();
}

void UIButtonMetadata::SetShadowColor(const QColor& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	for (uint32 i = 0; i < this->GetStatesCount(); ++i)
	{
		UIStaticText* referenceButtonText = GetActiveUIButton()->GetStateTextControl(this->uiControlStates[i]);
		if (referenceButtonText)
		{
			referenceButtonText->SetShadowColor(ColorHelper::QTColorToDAVAColor(value));
		}
	}
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

QString UIButtonMetadata::GetSprite()
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

int UIButtonMetadata::GetSpriteFrame()
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

QColor UIButtonMetadata::GetColor()
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

int UIButtonMetadata::GetDrawType()
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

int UIButtonMetadata::GetColorInheritType()
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

int UIButtonMetadata::GetAlign()
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


int UIButtonMetadata::GetSpriteModification()
{
	if (!VerifyActiveParamID())
	{
		return UIControlBackground::DRAW_ALIGNED;
	}

	return GetSpriteModificationForState(uiControlStates[GetActiveStateIndex()]);
}

int UIButtonMetadata::GetTextAlign()
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

	UpdatePropertyDirtyFlagForTextAlign();
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
            // Changing Fitting Option affects the font which might be reused
            // by other controls, so clone the existing one.
            CloneFont(buttonText);
            buttonText->SetFittingOption(value);
        }
    }

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

float UIButtonMetadata::GetLeftRightStretchCap()
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

float UIButtonMetadata::GetTopBottomStretchCap()
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

void UIButtonMetadata::RecoverPropertyDirtyFlags()
{
    UpdatePropertyDirtyFlagForLocalizedText();
    UpdatePropertyDirtyFlagForFont();
    //UpdatePropertyDirtyFlagForFontSize();
    UpdatePropertyDirtyFlagForColor();

    UpdatePropertyDirtyFlagForSpriteName();
    UpdatePropertyDirtyFlagForSpriteFrame();
    
    UpdatePropertyDirtyFlagForDrawType();
    UpdatePropertyDirtyFlagForColorInheritType();
    UpdatePropertyDirtyFlagForAlign();
    
    UpdatePropertyDirtyFlagForFittingType();
    
    UpdatePropertyDirtyFlagForLeftRightStretchCap();
    UpdatePropertyDirtyFlagForTopBottomStretchCap();
}
