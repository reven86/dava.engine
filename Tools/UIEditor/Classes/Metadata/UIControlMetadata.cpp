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


#include "UIControlMetadata.h"
#include "HierarchyTreeController.h"
#include "StringUtils.h"
#include "StringConstants.h"

#include "Helpers/ColorHelper.h"

#include <QtGlobal>

namespace DAVA {

UIControlMetadata::UIControlMetadata(QObject* parent) :
    BaseMetadata(parent)
{
}

void UIControlMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
	BaseMetadata::InitializeControl(controlName, position);
	
    int paramsCount = this->GetParamsCount();
	for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
   		UIControl* control = this->treeNodeParams[i].GetUIControl();
		ResizeScrollViewContent(control);
    }
}

QString UIControlMetadata::GetName() const
{
    if (!VerifyActiveParamID())
    {
        return QString();
    }

    return GetActiveUIControl()->GetName().c_str();
}

void UIControlMetadata::SetName(const QString& name)
{
    if (!VerifyActiveParamID() || !GetActiveTreeNode())
    {
        return;
    }

    // Need to update on the Tree Node level too.
    GetActiveUIControl()->SetName(name.toStdString());
    GetActiveTreeNode()->SetName(name);
}

int UIControlMetadata::GetTag() const
{
    if (!VerifyActiveParamID())
    {
        return -1;
    }

    return GetActiveUIControl()->GetTag();
}

void UIControlMetadata::SetTag(int tag)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    GetActiveUIControl()->SetTag(tag);
}

float UIControlMetadata::GetRelativeX() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }

    return GetActiveUIControl()->GetPosition(false).x;
}

void UIControlMetadata::SetRelativeX(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    Vector2 relativePosition = GetActiveUIControl()->GetPosition(false);
    relativePosition.x = value;
    GetActiveUIControl()->SetPosition(relativePosition, false);
}
    
float UIControlMetadata::GetRelativeY() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
    
    return GetActiveUIControl()->GetPosition(false).y;
}

void UIControlMetadata::SetRelativeY(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    Vector2 relativePosition = GetActiveUIControl()->GetPosition(false);
    relativePosition.y = value;
    GetActiveUIControl()->SetPosition(relativePosition, false);
}

float UIControlMetadata::GetAbsoluteX() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }

    return GetActiveUIControl()->GetPosition(true).x;
}
    
void UIControlMetadata::SetAbsoluteX(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    Vector2 absolutePosition = GetActiveUIControl()->GetPosition(true);
    absolutePosition.x = value;
    GetActiveUIControl()->SetPosition(absolutePosition, true);
}
    
float UIControlMetadata::GetAbsoluteY() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
    
    return GetActiveUIControl()->GetPosition(true).y;
}
    
void UIControlMetadata::SetAbsoluteY(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    Vector2 absolutePosition = GetActiveUIControl()->GetPosition(true);
    absolutePosition.y = value;
    GetActiveUIControl()->SetPosition(absolutePosition, true);
}

float UIControlMetadata::GetSizeX() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
    
    return GetActiveUIControl()->GetSize().x;
}

void UIControlMetadata::SetSizeX(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	Rect rect = GetActiveUIControl()->GetRect();
	rect.dx = value;
	
	SetActiveControlRect(rect, true);
}

float UIControlMetadata::GetSizeY() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
    
    return GetActiveUIControl()->GetSize().y;
}
    
void UIControlMetadata::SetSizeY(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	Rect rect = GetActiveUIControl()->GetRect();
	rect.dy = value;
	
	SetActiveControlRect(rect, true);
}

float UIControlMetadata::GetPivotX() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }

    return GetActiveUIControl()->pivotPoint.x;
}
    
void UIControlMetadata::SetPivotX(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUIControl()->pivotPoint.x = value;
    // DF-2009 - Re-set align properties if pivot point was changed
    RefreshAlign();
}

float UIControlMetadata::GetPivotY() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }

    return GetActiveUIControl()->pivotPoint.y;
}
    
void UIControlMetadata::SetPivotY(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUIControl()->pivotPoint.y = value;
    // DF-2009 - Re-set align properties if pivot point was changed
    RefreshAlign();
}

QPointF UIControlMetadata::GetPivot() const
{
    if (!VerifyActiveParamID())
    {
        return QPointF();
    }

    return QPointF(GetActiveUIControl()->pivotPoint.x, GetActiveUIControl()->pivotPoint.y);
}

void UIControlMetadata::SetPivot(const QPointF& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    GetActiveUIControl()->pivotPoint.x = value.x();
    GetActiveUIControl()->pivotPoint.y = value.y();

    // DF-2009 - Re-set align properties if pivot point was changed
    RefreshAlign();
}

void UIControlMetadata::RefreshAlign()
{
    SetTopAlign(GetTopAlign());
    SetVCenterAlign(GetVCenterAlign());
    SetBottomAlign(GetBottomAlign());
}

float UIControlMetadata::GetAngle() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0f;
    }
	
    // Angle is stored in radians, but displayed in degrees.
	// RedToDeg is rather inaccurate - so we should round ret value to higher value
    return qRound(RadToDeg(GetActiveUIControl()->GetAngle()));
}
    
void UIControlMetadata::SetAngle(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    // Angle is passed in degrees, but stored in radians.
    GetActiveUIControl()->SetAngle(DegToRad(value));
}

bool UIControlMetadata::GetVisible() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }

    return GetActiveUIControl()->GetVisible();
}

void UIControlMetadata::SetVisible(const bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    GetActiveUIControl()->SetVisible(value);
}

bool UIControlMetadata::GetInput() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }
    
    return GetActiveUIControl()->GetInputEnabled();
}

void UIControlMetadata::SetInput(const bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

   	// Yuri Coder, 2013/09/30. Don't update the hierarchy (see please DF-2147 for details).
    GetActiveUIControl()->SetInputEnabled(value, false);
}

bool UIControlMetadata::GetClipContents() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }
    
    return GetActiveUIControl()->GetClipContents();
}

void UIControlMetadata::SetClipContents(const bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUIControl()->SetClipContents(value);
}

void UIControlMetadata::ApplyMove(const Vector2& moveDelta, bool alignControlsToIntegerPos)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    float32 parentsTotalAngle = GetActiveUIControl()->GetParent()->GetGeometricData().angle;
    Vector2 controlPosition = GetActiveUIControl()->GetPosition();
    if(parentsTotalAngle != 0)
    {
        Matrix3 tmp;
        tmp.BuildRotation(-parentsTotalAngle);
        Vector2 rotatedVec = moveDelta * tmp;
        controlPosition += rotatedVec;
    }
    else
    {
        controlPosition += moveDelta;
    }
	
	Rect rect = GetActiveUIControl()->GetRect();
	rect.x = controlPosition.x;
	rect.y = controlPosition.y;
	
	// Apply the pivot point.
	Vector2 pivotPoint = GetActiveUIControl()->pivotPoint;
	rect.x -= pivotPoint.x;
	rect.y -= pivotPoint.y;

	SetActiveControlRect(rect, false, alignControlsToIntegerPos);
}

void UIControlMetadata::ApplyResize(const Rect& /*originalRect*/, const Rect& newRect)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
	SetActiveControlRect(newRect, false);
}
                 
QColor UIControlMetadata::GetColor() const
{
    if (!VerifyActiveParamID())
    {
        return QColor();
    }

    return ColorHelper::DAVAColorToQTColor(GetActiveUIControl()->GetBackground()->color);
}
    
void UIControlMetadata::SetColor(const QColor& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

    GetActiveUIControl()->GetBackground()->SetColor(ColorHelper::QTColorToDAVAColor(value));
}
    
int UIControlMetadata::GetDrawType() const
{
    if (!VerifyActiveParamID())
    {
        return UIControlBackground::DRAW_ALIGNED;
    }

    return GetActiveUIControl()->GetBackground()->GetDrawType();
}
    
void UIControlMetadata::SetDrawType(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUIControl()->GetBackground()->SetDrawType((UIControlBackground::eDrawType)value);
}
    
int UIControlMetadata::GetColorInheritType() const
{
    if (!VerifyActiveParamID())
    {
        return UIControlBackground::COLOR_MULTIPLY_ON_PARENT;
    }
    
    return (int)GetActiveUIControl()->GetBackground()->GetColorInheritType();
}
    
void UIControlMetadata::SetColorInheritType(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUIControl()->GetBackground()->SetColorInheritType((UIControlBackground::eColorInheritType)value);
}

int UIControlMetadata::GetPerPixelAccuracyType() const
{
    if (!VerifyActiveParamID())
    {
        return UIControlBackground::PER_PIXEL_ACCURACY_DISABLED;
    }
    
    return (int)GetActiveUIControl()->GetBackground()->GetPerPixelAccuracyType();
}
void UIControlMetadata::SetPerPixelAccuracyType(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUIControl()->GetBackground()->SetPerPixelAccuracyType((UIControlBackground::ePerPixelAccuracyType)value);
}
    
int UIControlMetadata::GetAlign() const
{
    if (!VerifyActiveParamID())
    {
        return ALIGN_LEFT;
    }
    
    return (int)GetActiveUIControl()->GetBackground()->GetAlign();
}
    
void UIControlMetadata::SetAlign(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUIControl()->GetBackground()->SetAlign((eAlign)value);
}

float UIControlMetadata::GetLeftRightStretchCap() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0;
    }
    
    return GetActiveUIControl()->GetBackground()->GetLeftRightStretchCap();
}

float UIControlMetadata::GetTopBottomStretchCap() const
{
    if (!VerifyActiveParamID())
    {
        return -1.0;
    }
    
    return GetActiveUIControl()->GetBackground()->GetTopBottomStretchCap();
}

void UIControlMetadata::SetLeftRightStretchCap(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUIControl()->GetBackground()->SetLeftRightStretchCap(value);
}

void UIControlMetadata::SetTopBottomStretchCap(float value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    GetActiveUIControl()->GetBackground()->SetTopBottomStretchCap(value);
}

void UIControlMetadata::SetSprite(const QString& value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    //If empty string value is used - remove sprite
    if (value.isEmpty() || value == StringConstants::NO_SPRITE_IS_SET)
    {
        GetActiveUIControl()->GetBackground()->SetSprite(NULL, 0); 
    }
    else
    {
        Sprite* sprite = Sprite::Create(value.toStdString());
        if (sprite)
        {
            GetActiveUIControl()->GetBackground()->SetSprite(sprite, 0);
            SafeRelease(sprite);

            // Specific case if the sprite is set to UISlider thumbSprite (see please DF-2834).
            UpdateThumbSizeForUIControlThumb();
        }
    }
}

void UIControlMetadata::UpdateThumbSizeForUIControlThumb()
{
    UIControl* activeUIControl = GetActiveUIControl();
    if (activeUIControl && activeUIControl->GetParent())
    {
        UISlider* parentIsSlider = dynamic_cast<UISlider*>(activeUIControl->GetParent());
        if (parentIsSlider && parentIsSlider->GetThumb() == activeUIControl)
        {
            parentIsSlider->SyncThumbWithSprite();
        }
    }
}
    
QString UIControlMetadata::GetSprite() const
{
    if (!VerifyActiveParamID())
    {
        return QString();
    }

    Sprite* sprite = GetActiveUIControl()->GetBackground()->GetSprite();
    if (sprite == NULL)
    {
        return StringConstants::NO_SPRITE_IS_SET;
    }
    
    return QString::fromStdString(sprite->GetRelativePathname().GetAbsolutePathname());
}
    
void UIControlMetadata::SetSpriteFrame(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    Sprite* sprite = GetActiveUIControl()->GetBackground()->GetSprite();
    if (sprite == NULL)
    {
        return;
    }

    if (sprite->GetFrameCount() <= value)
    {
        // No way to set this frame.
        return;
    }

    GetActiveUIControl()->GetBackground()->SetFrame(value);
}
    
int UIControlMetadata::GetSpriteFrame() const
{
    if (!VerifyActiveParamID())
    {
        return -1;
    }
    
    Sprite* sprite = GetActiveUIControl()->GetBackground()->GetSprite();
    if (sprite == NULL)
    {
        return 0;
    }
    
    return GetActiveUIControl()->GetBackground()->GetFrame();
}

void UIControlMetadata::SetSpriteModification(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
    
    Sprite* sprite = GetActiveUIControl()->GetBackground()->GetSprite();
    if (sprite == NULL)
    {
        return;
    }

    GetActiveUIControl()->GetBackground()->SetModification(value);
}

int UIControlMetadata::GetSpriteModification() const
{
    if (!VerifyActiveParamID())
    {
        return -1;
    }
    
    Sprite* sprite = GetActiveUIControl()->GetBackground()->GetSprite();
    if (sprite == NULL)
    {
        return 0;
    }
    
    return GetActiveUIControl()->GetBackground()->GetModification();
}

int UIControlMetadata::GetLeftAlign() const
{
    if (!VerifyActiveParamID())
    {
        return 0;
    }
	return GetActiveUIControl()->GetLeftAlign();
}

void UIControlMetadata::SetLeftAlign(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUIControl()->SetLeftAlign(value);
}
	
int UIControlMetadata::GetHCenterAlign() const
{
    if (!VerifyActiveParamID())
    {
        return 0;
    }
	
	return GetActiveUIControl()->GetHCenterAlign();
}

void UIControlMetadata::SetHCenterAlign(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }

	GetActiveUIControl()->SetHCenterAlign(value);
}

int UIControlMetadata::GetRightAlign() const
{
    if (!VerifyActiveParamID())
    {
        return 0;
    }
	
	return GetActiveUIControl()->GetRightAlign();
}

void UIControlMetadata::SetRightAlign(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUIControl()->SetRightAlign(value);
}

int UIControlMetadata::GetTopAlign() const
{
    if (!VerifyActiveParamID())
    {
        return 0;
    }
	
	return GetActiveUIControl()->GetTopAlign();
}

void UIControlMetadata::SetTopAlign(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUIControl()->SetTopAlign(value);
}

int UIControlMetadata::GetVCenterAlign() const
{
    if (!VerifyActiveParamID())
    {
        return 0;
    }
	
	return GetActiveUIControl()->GetVCenterAlign();
}

void UIControlMetadata::SetVCenterAlign(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUIControl()->SetVCenterAlign(value);
}

int UIControlMetadata::GetBottomAlign() const
{
    if (!VerifyActiveParamID())
    {
        return 0;
    }
	
	return GetActiveUIControl()->GetBottomAlign();
}

void UIControlMetadata::SetBottomAlign(int value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUIControl()->SetBottomAlign(value);
}

bool UIControlMetadata::GetLeftAlignEnabled() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }
	
	return GetActiveUIControl()->GetLeftAlignEnabled();
}

void UIControlMetadata::SetLeftAlignEnabled(const bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUIControl()->SetLeftAlignEnabled(value);
}
	
bool UIControlMetadata::GetHCenterAlignEnabled() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }
	
	return GetActiveUIControl()->GetHCenterAlignEnabled();
}

void UIControlMetadata::SetHCenterAlignEnabled(const bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUIControl()->SetHCenterAlignEnabled(value);
}
	
bool UIControlMetadata::GetRightAlignEnabled() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }
	
	return GetActiveUIControl()->GetRightAlignEnabled();
}

void UIControlMetadata::SetRightAlignEnabled(const bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUIControl()->SetRightAlignEnabled(value);
}
	
bool UIControlMetadata::GetTopAlignEnabled() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }
	
	return GetActiveUIControl()->GetTopAlignEnabled();
}

void UIControlMetadata::SetTopAlignEnabled(const bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUIControl()->SetTopAlignEnabled(value);
}
	
bool UIControlMetadata::GetVCenterAlignEnabled() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }
	
	return GetActiveUIControl()->GetVCenterAlignEnabled();
}

void UIControlMetadata::SetVCenterAlignEnabled(const bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUIControl()->SetVCenterAlignEnabled(value);
}
	
bool UIControlMetadata::GetBottomAlignEnabled() const
{
    if (!VerifyActiveParamID())
    {
        return false;
    }
	
	return GetActiveUIControl()->GetBottomAlignEnabled();
}

void UIControlMetadata::SetBottomAlignEnabled(const bool value)
{
    if (!VerifyActiveParamID())
    {
        return;
    }
	
	GetActiveUIControl()->SetBottomAlignEnabled(value);
}

void UIControlMetadata::SetActiveControlRect(const Rect& rect, bool restoreAlign, bool alignToIntegerPos)
{
	// Save/restore Align Data before changing the Control Rect, if requested.
	UIControl* activeControl = GetActiveUIControl();

	HierarchyTreeNode::AlignData alignData;
	if (restoreAlign)
	{
		alignData = HierarchyTreeNode::SaveAlignData(activeControl);
	}

	activeControl->SetRect(rect);
    
    if (alignToIntegerPos)
    {
        Vector2 controlPos = activeControl->GetPosition();
        controlPos.x = Round(controlPos.x);
        controlPos.y = Round(controlPos.y);
        activeControl->SetPosition(controlPos);
    }

	if (restoreAlign)
	{
		HierarchyTreeNode::RestoreAlignData(activeControl, alignData);
	}

	ResizeScrollViewContent(GetActiveUIControl());
}

QString UIControlMetadata::GetCustomControlName() const
{
	if (!VerifyActiveParamID())
	{
		return QString();
	}
	
	return QString::fromStdString(GetActiveUIControl()->GetCustomControlClassName());
}
	
void UIControlMetadata::SetCustomControlName(const QString& value)
{
	if (!VerifyActiveParamID())
	{
		return;
	}

	GetActiveUIControl()->SetCustomControlClassName(value.toStdString());
}

int UIControlMetadata::GetInitialState() const
{
	if (!VerifyActiveParamID())
	{
		return -1;
	}
	
	return (int)GetActiveUIControl()->GetInitialState();
}

void UIControlMetadata::SetInitialState(int value)
{
	if (!VerifyActiveParamID())
	{
		return;
	}
	
	GetActiveUIControl()->SetInitialState((int)value);
}

void UIControlMetadata::ResizeScrollViewContent(UIControl * control)
{
	UIControl *parentControl = control->GetParent();
	
	UIScrollView *scrollView = dynamic_cast<UIScrollView*>(parentControl);
	UIScreen *screen = dynamic_cast<UIScreen*>(parentControl);
	
	if (screen || !parentControl)
	{
		return;
	}
	
	if (scrollView)
	{
		scrollView->RecalculateContentSize();
	}
	else
	{
		ResizeScrollViewContent(parentControl);
	}
}

QRectF UIControlMetadata::GetMargins() const
{
    if (!VerifyActiveParamID() || !GetActiveUIControl()->GetBackground())
    {
        return QRectF();
    }
    
    const UIControlBackground::UIMargins* margins = GetActiveUIControl()->GetBackground()->GetMargins();
    if (!margins)
    {
        return QRectF();
    }

    return UIMarginsToQRectF(margins);
}
    
void UIControlMetadata::SetMargins(const QRectF& value)
{
    if (!VerifyActiveParamID() || !GetActiveUIControl()->GetBackground())
    {
        return;
    }

    UIControlBackground::UIMargins margins = QRectFToUIMargins(value);
    GetActiveUIControl()->GetBackground()->SetMargins(&margins);
}

float UIControlMetadata::GetLeftMargin() const
{
    return GetMargins().left();
}

void UIControlMetadata::SetLeftMargin(float value)
{
    if (!VerifyActiveParamID() || !GetActiveUIControl()->GetBackground())
    {
        return;
    }

    UIControlBackground::UIMargins margins = GetMarginsToUpdate();
    margins.left = value;
    GetActiveUIControl()->GetBackground()->SetMargins(&margins);
}
    
float UIControlMetadata::GetTopMargin() const
{
    return GetMargins().top();
}

void UIControlMetadata::SetTopMargin(float value)
{
    if (!VerifyActiveParamID() || !GetActiveUIControl()->GetBackground())
    {
        return;
    }
    
    UIControlBackground::UIMargins margins = GetMarginsToUpdate();
    margins.top = value;
    GetActiveUIControl()->GetBackground()->SetMargins(&margins);
}
    
float UIControlMetadata::GetRightMargin() const
{
    return GetMargins().width();
}
    
void UIControlMetadata::SetRightMargin(float value)
{
    if (!VerifyActiveParamID() || !GetActiveUIControl()->GetBackground())
    {
        return;
    }
    
    UIControlBackground::UIMargins margins = GetMarginsToUpdate();
    margins.right = value;
    GetActiveUIControl()->GetBackground()->SetMargins(&margins);
}

float UIControlMetadata::GetBottomMargin() const
{
    return GetMargins().height();
}

void UIControlMetadata::SetBottomMargin(float value)
{
    if (!VerifyActiveParamID() || !GetActiveUIControl()->GetBackground())
    {
        return;
    }
    
    UIControlBackground::UIMargins margins = GetMarginsToUpdate();
    margins.bottom = value;
    GetActiveUIControl()->GetBackground()->SetMargins(&margins);
}

UIControlBackground::UIMargins UIControlMetadata::GetMarginsToUpdate(UIControl::eControlState /* state */) const
{
    if (!VerifyActiveParamID() || !GetActiveUIControl()->GetBackground())
    {
        return UIControlBackground::UIMargins();
    }

    const UIControlBackground::UIMargins* margins = GetActiveUIControl()->GetBackground()->GetMargins();
    if (!margins)
    {
        return UIControlBackground::UIMargins();
    }

    return *margins;
}

QRectF UIControlMetadata::UIMarginsToQRectF(const UIControlBackground::UIMargins* margins) const
{
    if (!margins)
    {
        return QRectF();
    }

    QRectF resultRect;
    resultRect.setLeft(margins->left);
    resultRect.setTop(margins->top);
    resultRect.setWidth(margins->right);
    resultRect.setHeight(margins->bottom);

    return resultRect;
}
    
UIControlBackground::UIMargins UIControlMetadata::QRectFToUIMargins(const QRectF& rect) const
{
    UIControlBackground::UIMargins resultMargins;
    resultMargins.left = rect.left();
    resultMargins.top = rect.top();
    resultMargins.right = rect.width();
    resultMargins.bottom = rect.height();
 
    return resultMargins;
} 

};