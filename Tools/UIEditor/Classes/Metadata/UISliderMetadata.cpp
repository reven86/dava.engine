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


#include "UISliderMetadata.h"
#include "StringUtils.h"
#include "StringConstants.h"
#include "ColorHelper.h"

using namespace DAVA;

UISliderMetadata::UISliderMetadata(QObject* parent) :
    UIControlMetadata(parent)
{
}

UISlider* UISliderMetadata::GetActiveUISlider() const
{
    return static_cast<UISlider*>(GetActiveUIControl());
}

// Initialize the control(s) attached.
void UISliderMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
    UIControlMetadata::InitializeControl(controlName, position);

    int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
		// Initialize UISlider with default parameters
        UISlider* slider = static_cast<UISlider*>(this->treeNodeParams[i].GetUIControl());
        slider->SetMinMaxValue(0.0f, 100.0f);
    }
}

float UISliderMetadata::GetSliderValue() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
    
	return GetActiveUISlider()->GetValue();
}

void UISliderMetadata::SetSliderValue(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	GetActiveUISlider()->SetValue(value);
}

float UISliderMetadata::GetSliderMinValue() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }

	return GetActiveUISlider()->GetMinValue();
}

void UISliderMetadata::SetSliderMinValue(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	GetActiveUISlider()->SetMinValue(value);
}

float UISliderMetadata::GetSliderMaxValue() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
    
	return GetActiveUISlider()->GetMaxValue();
}

void UISliderMetadata::SetSliderMaxValue(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	GetActiveUISlider()->SetMaxValue(value);
}

void UISliderMetadata::ApplyResize(const Rect& /*originalRect*/, const Rect& newRect)
{
    if (!VerifyActiveParamID())
    {
        return;
    }	
	
    GetActiveUISlider()->SetRect(newRect);
}

QColor UISliderMetadata::GetMinColor() const
{
    if (!VerifyActiveParamIDAndMinBackground())
    {
        return UIControlBackground::DRAW_ALIGNED;
    }
    
    return ColorHelper::DAVAColorToQTColor(GetActiveUISlider()->GetBgMin()->GetColor());
}

void UISliderMetadata::SetMinColor(const QColor& value)
{
    if (!VerifyActiveParamIDAndMinBackground())
    {
        return;
    }

    GetActiveUISlider()->GetBgMin()->SetColor(ColorHelper::QTColorToDAVAColor(value));
}

int UISliderMetadata::GetMinDrawType() const
{
    if (!VerifyActiveParamIDAndMinBackground())
    {
        return UIControlBackground::DRAW_ALIGNED;
    }
    
    return GetActiveUISlider()->GetBgMin()->GetDrawType();
}

void UISliderMetadata::SetMinDrawType(int value)
{
    if (!VerifyActiveParamIDAndMinBackground())
    {
        return;
    }
    
    GetActiveUISlider()->GetBgMin()->SetDrawType((UIControlBackground::eDrawType)value);
}

int UISliderMetadata::GetMinColorInheritType() const
{
    if (!VerifyActiveParamIDAndMinBackground())
    {
        return UIControlBackground::COLOR_MULTIPLY_ON_PARENT;
    }
    
    return (int)GetActiveUISlider()->GetBgMin()->GetColorInheritType();
}

void UISliderMetadata::SetMinColorInheritType(int value)
{
    if (!VerifyActiveParamIDAndMinBackground())
    {
        return;
    }
    
    GetActiveUISlider()->GetBgMin()->SetColorInheritType((UIControlBackground::eColorInheritType)value);
}

int UISliderMetadata::GetMinPerPixelAccuracyType() const
{
    if (!VerifyActiveParamIDAndMinBackground())
    {
        return UIControlBackground::PER_PIXEL_ACCURACY_DISABLED;
    }
    
    return (int)GetActiveUISlider()->GetBgMin()->GetPerPixelAccuracyType();
}

void UISliderMetadata::SetMinPerPixelAccuracyType(int value)
{
    if (!VerifyActiveParamIDAndMinBackground())
    {
        return;
    }
    
    GetActiveUISlider()->GetBgMin()->SetPerPixelAccuracyType((UIControlBackground::ePerPixelAccuracyType)value);
}

int UISliderMetadata::GetMinAlign() const
{
    if (!VerifyActiveParamIDAndMinBackground())
    {
        return ALIGN_LEFT;
    }
    
    return (int)GetActiveUISlider()->GetBgMin()->GetAlign();
}

void UISliderMetadata::SetMinAlign(int value)
{
    if (!VerifyActiveParamIDAndMinBackground())
    {
        return;
    }

    GetActiveUISlider()->GetBgMin()->SetAlign(value);
}

float UISliderMetadata::GetMinLeftRightStretchCap() const
{
    if (!VerifyActiveParamIDAndMinBackground())
    {
        return -1.0;
    }

    return GetActiveUISlider()->GetBgMin()->GetLeftRightStretchCap();
}

void UISliderMetadata::SetMinLeftRightStretchCap(float value)
{
    if (!VerifyActiveParamIDAndMinBackground())
    {
        return;
    }
    
    GetActiveUISlider()->GetBgMin()->SetLeftRightStretchCap(value);
}

float UISliderMetadata::GetMinTopBottomStretchCap() const
{
    if (!VerifyActiveParamIDAndMinBackground())
    {
        return -1.0;
    }

    return GetActiveUISlider()->GetBgMin()->GetTopBottomStretchCap();
}

void UISliderMetadata::SetMinTopBottomStretchCap(float value)
{
    if (!VerifyActiveParamIDAndMinBackground())
    {
        return;
    }
    
    GetActiveUISlider()->GetBgMin()->SetTopBottomStretchCap(value);
}

void UISliderMetadata::SetMinSprite(const QString& value)
{
    if (!VerifyActiveParamIDAndMinBackground())
    {
        return;
    }

    //If empty string value is used - remove sprite
    if (value.isEmpty())
    {
        GetActiveUISlider()->GetBgMin()->SetSprite(NULL, 0);
    }
    else
    {
        Sprite* sprite = Sprite::Create(value.toStdString());
        if (sprite)
        {
            GetActiveUISlider()->GetBgMin()->SetSprite(sprite, 0);
            SafeRelease(sprite);
        }
    }
}

QString UISliderMetadata::GetMinSprite() const
{
    if (!VerifyActiveParamIDAndMinBackground())
    {
        return QString();
    }
    
    Sprite* sprite = GetActiveUISlider()->GetBgMin()->GetSprite();
    if (sprite)
    {
        return QString::fromStdString(sprite->GetRelativePathname().GetAbsolutePathname());
    }
    
    return StringConstants::NO_SPRITE_IS_SET;
}

void UISliderMetadata::SetMinSpriteFrame(int value)
{
    if (!VerifyActiveParamIDAndMinBackground())
    {
        return;
    }
    
    Sprite* sprite = GetActiveUISlider()->GetBgMin()->GetSprite();
    if (sprite && sprite->GetFrameCount() > value)
    {
        GetActiveUISlider()->GetBgMin()->SetFrame(value);
    }
}

int UISliderMetadata::GetMinSpriteFrame() const
{
    if (!VerifyActiveParamIDAndMinBackground())
    {
        return -1;
    }
    
    if (GetActiveUISlider()->GetBgMin()->GetSprite())
    {
        return GetActiveUISlider()->GetBgMin()->GetFrame();
    }
    
    return 0;
}

void UISliderMetadata::SetMinSpriteModification(int value)
{
    if (!VerifyActiveParamIDAndMinBackground())
    {
        return;
    }

    Sprite* sprite = GetActiveUISlider()->GetBgMin()->GetSprite();
    if (sprite)
    {
        GetActiveUISlider()->GetBgMin()->SetModification(value);
    }
}

int UISliderMetadata::GetMinSpriteModification() const
{
    if (!VerifyActiveParamIDAndMinBackground())
    {
        return -1;
    }

    Sprite* sprite = GetActiveUISlider()->GetBgMin()->GetSprite();
    if (sprite)
    {
        return GetActiveUISlider()->GetBgMin()->GetModification();
    }

    return 0;
}

// Max Background.

QColor UISliderMetadata::GetMaxColor() const
{
    if (!VerifyActiveParamIDAndMaxBackground())
    {
        return UIControlBackground::DRAW_ALIGNED;
    }

    return ColorHelper::DAVAColorToQTColor(GetActiveUISlider()->GetBgMax()->GetColor());
}

void UISliderMetadata::SetMaxColor(const QColor& value)
{
    if (!VerifyActiveParamIDAndMaxBackground())
    {
        return;
    }

    GetActiveUISlider()->GetBgMax()->SetColor(ColorHelper::QTColorToDAVAColor(value));
}

int UISliderMetadata::GetMaxDrawType() const
{
    if (!VerifyActiveParamIDAndMaxBackground())
    {
        return UIControlBackground::DRAW_ALIGNED;
    }

    return GetActiveUISlider()->GetBgMax()->GetDrawType();
}

void UISliderMetadata::SetMaxDrawType(int value)
{
    if (!VerifyActiveParamIDAndMaxBackground())
    {
        return;
    }
    
    GetActiveUISlider()->GetBgMax()->SetDrawType((UIControlBackground::eDrawType)value);
}

int UISliderMetadata::GetMaxColorInheritType() const
{
    if (!VerifyActiveParamIDAndMaxBackground())
    {
        return UIControlBackground::COLOR_MULTIPLY_ON_PARENT;
    }
    
    return (int)GetActiveUISlider()->GetBgMax()->GetColorInheritType();
}

void UISliderMetadata::SetMaxColorInheritType(int value)
{
    if (!VerifyActiveParamIDAndMaxBackground())
    {
        return;
    }
    
    GetActiveUISlider()->GetBgMax()->SetColorInheritType((UIControlBackground::eColorInheritType)value);
}

int UISliderMetadata::GetMaxPerPixelAccuracyType() const
{
    if (!VerifyActiveParamIDAndMaxBackground())
    {
        return UIControlBackground::PER_PIXEL_ACCURACY_DISABLED;
    }
    
    return (int)GetActiveUISlider()->GetBgMax()->GetPerPixelAccuracyType();
}

void UISliderMetadata::SetMaxPerPixelAccuracyType(int value)
{
    if (!VerifyActiveParamIDAndMaxBackground())
    {
        return;
    }
    
    GetActiveUISlider()->GetBgMax()->SetPerPixelAccuracyType((UIControlBackground::ePerPixelAccuracyType)value);
}

int UISliderMetadata::GetMaxAlign() const
{
    if (!VerifyActiveParamIDAndMaxBackground())
    {
        return ALIGN_LEFT;
    }
    
    return (int)GetActiveUISlider()->GetBgMax()->GetAlign();
}

void UISliderMetadata::SetMaxAlign(int value)
{
    if (!VerifyActiveParamIDAndMaxBackground())
    {
        return;
    }
    
    GetActiveUISlider()->GetBgMax()->SetAlign(value);
}

float UISliderMetadata::GetMaxLeftRightStretchCap() const
{
    if (!VerifyActiveParamIDAndMaxBackground())
    {
        return -1.0;
    }

    return GetActiveUISlider()->GetBgMax()->GetLeftRightStretchCap();
}

void UISliderMetadata::SetMaxLeftRightStretchCap(float value)
{
    if (!VerifyActiveParamIDAndMaxBackground())
    {
        return;
    }
    
    GetActiveUISlider()->GetBgMax()->SetLeftRightStretchCap(value);
}

float UISliderMetadata::GetMaxTopBottomStretchCap() const
{
    if (!VerifyActiveParamIDAndMaxBackground())
    {
        return -1.0;
    }
    
    return GetActiveUISlider()->GetBgMax()->GetTopBottomStretchCap();
}

void UISliderMetadata::SetMaxTopBottomStretchCap(float value)
{
    if (!VerifyActiveParamIDAndMaxBackground())
    {
        return;
    }

    GetActiveUISlider()->GetBgMax()->SetTopBottomStretchCap(value);
}

void UISliderMetadata::SetMaxSprite(const QString& value)
{
    if (!VerifyActiveParamIDAndMaxBackground())
    {
        return;
    }
    
    //If empty string value is used - remove sprite
    if (value.isEmpty())
    {
        GetActiveUISlider()->GetBgMax()->SetSprite(NULL, 0);
    }
    else
    {
        Sprite* sprite = Sprite::Create(value.toStdString());
        if (sprite)
        {
            GetActiveUISlider()->GetBgMax()->SetSprite(sprite, 0);
            SafeRelease(sprite);
        }
    }
}

QString UISliderMetadata::GetMaxSprite() const
{
    if (!VerifyActiveParamIDAndMaxBackground())
    {
        return QString();
    }

    Sprite* sprite = GetActiveUISlider()->GetBgMax()->GetSprite();
    if (sprite)
    {
        return QString::fromStdString(sprite->GetRelativePathname().GetAbsolutePathname());
    }

    return StringConstants::NO_SPRITE_IS_SET;
}

void UISliderMetadata::SetMaxSpriteFrame(int value)
{
    if (!VerifyActiveParamIDAndMaxBackground())
    {
        return;
    }
    
    Sprite* sprite = GetActiveUISlider()->GetBgMax()->GetSprite();
    if (sprite && sprite->GetFrameCount() > value)
    {
        GetActiveUISlider()->GetBgMax()->SetFrame(value);
    }
}

int UISliderMetadata::GetMaxSpriteFrame() const
{
    if (!VerifyActiveParamIDAndMaxBackground())
    {
        return -1;
    }
    
    if (GetActiveUISlider()->GetBgMax()->GetSprite())
    {
        return GetActiveUISlider()->GetBgMax()->GetFrame();
    }
    
    return 0;
}

void UISliderMetadata::SetMaxSpriteModification(int value)
{
    if (!VerifyActiveParamIDAndMaxBackground())
    {
        return;
    }
    
    Sprite* sprite = GetActiveUISlider()->GetBgMax()->GetSprite();
    if (sprite)
    {
        GetActiveUISlider()->GetBgMax()->SetModification(value);
    }
}

int UISliderMetadata::GetMaxSpriteModification() const
{
    if (!VerifyActiveParamIDAndMaxBackground())
    {
        return -1;
    }

    Sprite* sprite = GetActiveUISlider()->GetBgMax()->GetSprite();
    if (sprite)
    {
        return GetActiveUISlider()->GetBgMax()->GetModification();
    }

    return 0;
}

bool UISliderMetadata::VerifyActiveParamIDAndMinBackground() const
{
    return VerifyActiveParamID() && GetActiveUISlider()->GetBgMin();
}

bool UISliderMetadata::VerifyActiveParamIDAndMaxBackground() const
{
    return VerifyActiveParamID() && GetActiveUISlider()->GetBgMax();
}

QRectF UISliderMetadata::GetMinMargins() const
{
    return GetMarginsForBackground(GetActiveUISlider()->GetBgMin());
}

void UISliderMetadata::SetMinMargins(const QRectF& value)
{
    SetMarginsForBackground(GetActiveUISlider()->GetBgMin(), value);
}

float UISliderMetadata::GetMinLeftMargin() const
{
    return GetMinMargins().left();
}

void UISliderMetadata::SetMinLeftMargin(float value)
{
    if (!VerifyActiveParamID() || !GetActiveUISlider()->GetBgMin())
    {
        return;
    }
    
    UIControlBackground::UIMargins margins = GetMinMarginsToUpdate();
    margins.left = value;
    GetActiveUISlider()->GetBgMin()->SetMargins(&margins);
}

float UISliderMetadata::GetMinTopMargin() const
{
    return GetMinMargins().top();
}

void UISliderMetadata::SetMinTopMargin(float value)
{
    if (!VerifyActiveParamID() || !GetActiveUISlider()->GetBgMin())
    {
        return;
    }
    
    UIControlBackground::UIMargins margins = GetMinMarginsToUpdate();
    margins.top = value;
    GetActiveUISlider()->GetBgMin()->SetMargins(&margins);
}

float UISliderMetadata::GetMinRightMargin() const
{
    return GetMinMargins().width();
}

void UISliderMetadata::SetMinRightMargin(float value)
{
    if (!VerifyActiveParamID() || !GetActiveUISlider()->GetBgMin())
    {
        return;
    }
    
    UIControlBackground::UIMargins margins = GetMinMarginsToUpdate();
    margins.right = value;
    GetActiveUISlider()->GetBgMin()->SetMargins(&margins);
}

float UISliderMetadata::GetMinBottomMargin() const
{
    return GetMinMargins().height();
}

void UISliderMetadata::SetMinBottomMargin(float value)
{
    if (!VerifyActiveParamID() || !GetActiveUISlider()->GetBgMin())
    {
        return;
    }
    
    UIControlBackground::UIMargins margins = GetMinMarginsToUpdate();
    margins.bottom = value;
    GetActiveUISlider()->GetBgMin()->SetMargins(&margins);
}

QRectF UISliderMetadata::GetMaxMargins() const
{
    return GetMarginsForBackground(GetActiveUISlider()->GetBgMax());
}

void UISliderMetadata::SetMaxMargins(const QRectF& value)
{
    SetMarginsForBackground(GetActiveUISlider()->GetBgMax(), value);
}

float UISliderMetadata::GetMaxLeftMargin() const
{
    return GetMaxMargins().left();
}

void UISliderMetadata::SetMaxLeftMargin(float value)
{
    if (!VerifyActiveParamID() || !GetActiveUISlider()->GetBgMax())
    {
        return;
    }
    
    UIControlBackground::UIMargins margins = GetMaxMarginsToUpdate();
    margins.left = value;
    GetActiveUISlider()->GetBgMax()->SetMargins(&margins);
}

float UISliderMetadata::GetMaxTopMargin() const
{
    return GetMaxMargins().top();
}

void UISliderMetadata::SetMaxTopMargin(float value)
{
    if (!VerifyActiveParamID() || !GetActiveUISlider()->GetBgMax())
    {
        return;
    }
    
    UIControlBackground::UIMargins margins = GetMaxMarginsToUpdate();
    margins.top = value;
    GetActiveUISlider()->GetBgMax()->SetMargins(&margins);
}

float UISliderMetadata::GetMaxRightMargin() const
{
    return GetMaxMargins().width();
}

void UISliderMetadata::SetMaxRightMargin(float value)
{
    if (!VerifyActiveParamID() || !GetActiveUISlider()->GetBgMax())
    {
        return;
    }
    
    UIControlBackground::UIMargins margins = GetMaxMarginsToUpdate();
    margins.right = value;
    GetActiveUISlider()->GetBgMax()->SetMargins(&margins);
}

float UISliderMetadata::GetMaxBottomMargin() const
{
    return GetMaxMargins().height();
}

void UISliderMetadata::SetMaxBottomMargin(float value)
{
    if (!VerifyActiveParamID() || !GetActiveUISlider()->GetBgMax())
    {
        return;
    }
    
    UIControlBackground::UIMargins margins = GetMaxMarginsToUpdate();
    margins.bottom = value;
    GetActiveUISlider()->GetBgMax()->SetMargins(&margins);
}

QRectF UISliderMetadata::GetMarginsForBackground(UIControlBackground* background) const
{
    if (!VerifyActiveParamID() || !background)
    {
        return QRectF();
    }
    
    const UIControlBackground::UIMargins* margins = background->GetMargins();
    if (!margins)
    {
        return QRectF();
    }
    
    return UIMarginsToQRectF(margins);
}

void UISliderMetadata::SetMarginsForBackground(UIControlBackground* background, const QRectF& value)
{
    if (!VerifyActiveParamID() || !background)
    {
        return;
    }
    
    UIControlBackground::UIMargins margins = QRectFToUIMargins(value);
    background->SetMargins(&margins);
}

UIControlBackground::UIMargins UISliderMetadata::GetMinMarginsToUpdate()
{
    if (!VerifyActiveParamID() || !GetActiveUISlider()->GetBgMin())
    {
        return UIControlBackground::UIMargins();
    }
    
    const UIControlBackground::UIMargins* margins = GetActiveUISlider()->GetBgMin()->GetMargins();
    if (!margins)
    {
        return UIControlBackground::UIMargins();
    }
    
    return *margins;
}

UIControlBackground::UIMargins UISliderMetadata::GetMaxMarginsToUpdate()
{
    if (!VerifyActiveParamID() || !GetActiveUISlider()->GetBgMax())
    {
        return UIControlBackground::UIMargins();
    }
    
    const UIControlBackground::UIMargins* margins = GetActiveUISlider()->GetBgMax()->GetMargins();
    if (!margins)
    {
        return UIControlBackground::UIMargins();
    }
    
    return *margins;
}



