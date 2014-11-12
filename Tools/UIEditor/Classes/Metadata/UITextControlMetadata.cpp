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


#include "UITextControlMetadata.h"
#include "StringUtils.h"

#include "EditorFontManager.h"

using namespace DAVA;

UITextControlMetadata::UITextControlMetadata(QObject* parent) :
    UIControlMetadata(parent)
{
}

QString UITextControlMetadata::GetLocalizedTextKeyForState(UIControl::eControlState controlState) const
{
    // Return the localization key from the Hierarchy Tree node.
    HierarchyTreeNode* node = this->GetActiveTreeNode();
    if (node)
    {
        return WideString2QString(node->GetExtraData().GetLocalizationKey(controlState));
    }
    
    return QString();
}

QString UITextControlMetadata::GetLocalizedTextKey() const
{
    return GetLocalizedTextKeyForState(GetCurrentStateForLocalizedText());
}

void UITextControlMetadata::SetLocalizedTextKey(const QString& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    WideString localizationValue = LocalizationSystem::Instance()->GetLocalizedString(QStrint2WideString(value));
    
    // Update the Tree Node Extradata level with the key.
    HierarchyTreeNode* node = this->GetActiveTreeNode();
    if (node)
    {
        node->GetExtraData().SetLocalizationKey(QStrint2WideString(value), GetCurrentStateForLocalizedText());
    }
    
    // Note - the actual update of the control's text should be performed in the derived classes!
}

void UITextControlMetadata::UpdateStaticTextExtraData(UIStaticText* staticText, UIControl::eControlState state,
                                                      HierarchyTreeNodeExtraData& extraData,
                                                      eExtraDataUpdateStyle updateStyle)
{
    if (!staticText)
    {
        DVASSERT(false);
        return;
    }

    switch (updateStyle)
    {
        case BaseMetadata::UPDATE_EXTRADATA_FROM_CONTROL:
        {
            extraData.SetLocalizationKey(staticText->GetText(), state);
            break;
        }
            
        case BaseMetadata::UPDATE_CONTROL_FROM_EXTRADATA_RAW:
        {
            staticText->SetText(extraData.GetLocalizationKey(state));
            break;
        }
            
        case BaseMetadata::UPDATE_CONTROL_FROM_EXTRADATA_LOCALIZED:
        {
            //TODO: remove this workaround
            String fontPresetName = EditorFontManager::Instance()->GetLocalizedFontName(staticText->GetFont());
            Font* font = EditorFontManager::Instance()->GetLocalizedFont(fontPresetName, LocalizationSystem::Instance()->GetCurrentLocale());
            staticText->SetFont(font);
            
            staticText->SetText(LocalizationSystem::Instance()->GetLocalizedString(extraData.GetLocalizationKey(state)));
            break;
        }
            
        default:
        {
            Logger::Error("Unknown Update Style %i!", updateStyle);
            break;
        }
    }
}

Vector2 UITextControlMetadata::GetOffsetX(const Vector2& currentOffset, float offsetX) const
{
	Vector2 offset(currentOffset);
	offset.x = offsetX;
	return offset;
}

Vector2 UITextControlMetadata::GetOffsetY(const Vector2& currentOffset, float offsetY) const
{
	Vector2 offset(currentOffset);
	offset.y = offsetY;
	return offset;
}

int UITextControlMetadata::GetTextColorInheritType() const
{
    return UIControlBackground::COLOR_IGNORE_PARENT;
}

void UITextControlMetadata::SetTextColorInheritType(int /*value*/)
{
}

int UITextControlMetadata::GetTextPerPixelAccuracyType() const
{
    return UIControlBackground::PER_PIXEL_ACCURACY_DISABLED;
}

void UITextControlMetadata::SetTextPerPixelAccuracyType(int /*value*/)
{
}

int UITextControlMetadata::GetFittingType() const
{
    return TextBlock::FITTING_DISABLED;
}

void UITextControlMetadata::SetFittingType(int /*value*/)
{
}

QRectF UITextControlMetadata::GetTextMargins() const
{
    return QRectF();
}

void UITextControlMetadata::SetTextMargins(const QRectF& /* value */)
{
}

float UITextControlMetadata::GetTextLeftMargin() const
{
    return 0.0f;
}

void UITextControlMetadata::SetTextLeftMargin(float /* value */)
{
}

float UITextControlMetadata::GetTextTopMargin() const
{
    return 0.0f;
}

void UITextControlMetadata::SetTextTopMargin(float /* value */)
{
}

float UITextControlMetadata::GetTextRightMargin() const
{
    return 0.0f;
}

void UITextControlMetadata::SetTextRightMargin(float /* value */)
{
}

float UITextControlMetadata::GetTextBottomMargin() const
{
    return 0.0f;
}

void UITextControlMetadata::SetTextBottomMargin(float /* value */)
{
}
