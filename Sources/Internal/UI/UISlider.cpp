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


#include "UI/UISlider.h"
#include "UI/UIButton.h"
#include "Render/RenderHelper.h"
#include "FileSystem/YamlNode.h"
#include "Base/ObjectFactory.h"
#include "Utils/Utils.h"
#include "Core/Core.h"
#include "UI/UIEvent.h"
#include "UI/UIYamlLoader.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

namespace DAVA 
{
	
	
// Use these names for children buttons to define UISlider in .yaml
static const String UISLIDER_THUMB_SPRITE_CONTROL_NAME = "thumbSpriteControl";
static const String UISLIDER_MIN_SPRITE_CONTROL_NAME = "minSpriteControl";
static const String UISLIDER_MAX_SPRITE_CONTROL_NAME = "maxSpriteControl";

UISlider::UISlider(const Rect& rect)
    : UIControl(rect)
    , minBackground(NULL)
    , maxBackground(NULL)
    , thumbButton(NULL)
    , spritesEmbedded(false)
{
    SetInputEnabled(true, false);
	isEventsContinuos = true;
	
	leftInactivePart = 0;
	rightInactivePart = 0;
	minValue = 0.0f;
	maxValue = 1.0f;
	currentValue = 0.5f;

    minBackground = new UIControlBackground();
    maxBackground = new UIControlBackground();
    InitThumb();
}
	
void UISlider::InitThumb()
{
    thumbButton = new UIControl(Rect(0, 0, 40.f, 40.f));
    thumbButton->SetName(UISLIDER_THUMB_SPRITE_CONTROL_NAME);
    AddControl(thumbButton);

	thumbButton->SetInputEnabled(false);
	thumbButton->relativePosition.y = size.y * 0.5f;
    thumbButton->SetPivot(Vector2(0.5f, 0.5f));
	
	SetValue(currentValue);
}

void UISlider::InitInactiveParts(Sprite* spr)
{
	if(NULL == spr)
	{
		return;
	}

	leftInactivePart = rightInactivePart = (int32)((spr->GetWidth() / 2.0f));
}

void UISlider::SetThumb(UIControl *newThumb)
{
    if (thumbButton == newThumb)
    {
        return;
    }

    RemoveControl(thumbButton);
    SafeRelease(thumbButton);

    thumbButton = SafeRetain(newThumb);
	thumbButton->SetName(UISLIDER_THUMB_SPRITE_CONTROL_NAME);
	thumbButton->SetInputEnabled(false);
	
	thumbButton->relativePosition.y = size.y * 0.5f;
    thumbButton->SetPivot(Vector2(0.5f, 0.5f));
	
	UIControl::AddControl(thumbButton);
	
	SetValue(currentValue);
}
    
    
UISlider::~UISlider()
{
	SafeRelease(minBackground);
	SafeRelease(maxBackground);
	SafeRelease(thumbButton);
}
	
void UISlider::RecalcButtonPos()
{
	if (thumbButton)
	{
		thumbButton->relativePosition.x = Interpolation::Linear((float32)leftInactivePart, size.x - rightInactivePart, minValue, currentValue, maxValue);
        thumbButton->relativePosition.y = GetSize().y / 2; // thumb button pivot point is on center.
	}
}

void UISlider::SyncThumbWithSprite()
{
   	RecalcButtonPos();
}

void UISlider::SetValue(float32 value)
{
    bool needSendEvent = !FLOAT_EQUAL(currentValue, value);
	currentValue = value;
	RecalcButtonPos();
    
    if (needSendEvent)
    {
        PerformEvent(EVENT_VALUE_CHANGED);
    }
}

void UISlider::SetMinValue(float32 value)
{
    minValue = value;
    if (currentValue < minValue)
    {
        SetValue(minValue);
    }
    else
    {
        RecalcButtonPos();
    }
}
    
void UISlider::SetMaxValue(float32 value)
{
    maxValue = value;
    if (currentValue > maxValue)
    {
        SetValue(maxValue);
    }
    else
    {
        RecalcButtonPos();
    }
}
    
void UISlider::SetMinMaxValue(float32 _minValue, float32 _maxValue)
{
	minValue = _minValue;
	maxValue = _maxValue;

    if (currentValue < minValue)
    {
        SetValue(minValue);
    }
    else if (currentValue > maxValue)
    {
        SetValue(maxValue);
    }
    else
    {
        RecalcButtonPos();
    }
}

void UISlider::AddControl(UIControl *control)
{
    // Synchronize the pointers to the thumb each time new control is added.
    UIControl::AddControl(control);

    if (control->GetName() == UISLIDER_THUMB_SPRITE_CONTROL_NAME && thumbButton != control)
    {
        SafeRelease(thumbButton);
        thumbButton = SafeRetain(control);
    }
}

void UISlider::RemoveControl(UIControl *control)
{
    if (control == thumbButton)
    {
        SafeRelease(thumbButton);
    }
    
    UIControl::RemoveControl(control);
}

void UISlider::Input(UIEvent *currentInput)
{
#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)
    if (currentInput->phase == UIEvent::Phase::MOVE || currentInput->phase == UIEvent::Phase::CHAR)
        return;
#endif
	
	const Rect & absRect = GetGeometricData().GetUnrotatedRect();
	//absTouchPoint = currentInput->point;
	
	relTouchPoint = currentInput->point;
	relTouchPoint -= absRect.GetPosition();
	
	
	float oldVal = currentValue;
	currentValue = Interpolation::Linear(minValue, maxValue, (float32)leftInactivePart, relTouchPoint.x, size.x - (float32)rightInactivePart);
	
	if(currentValue < minValue)
	{
		currentValue = minValue;
	}
	if(currentValue > maxValue)
	{
		currentValue = maxValue;
	}

	if (isEventsContinuos) // if continuos events
	{
		if(oldVal != currentValue)
		{
			PerformEventWithData(EVENT_VALUE_CHANGED, currentInput);
		}
    }
    else if (currentInput->phase == UIEvent::Phase::ENDED)
    {
        /* if not continuos always perform event because last move position almost always the same as end pos */
        PerformEventWithData(EVENT_VALUE_CHANGED, currentInput);
    }

    RecalcButtonPos();
	currentInput->SetInputHandledType(UIEvent::INPUT_HANDLED_HARD); // Drag is handled - see please DF-2508.
}

void UISlider::Draw(const UIGeometricData &geometricData)
{
	const Rect & aRect =  thumbButton->GetGeometricData().GetUnrotatedRect();
	float32 clipPointAbsolute = aRect.x + aRect.dx * 0.5f;

    Rect fullVirtualScreen = VirtualCoordinatesSystem::Instance()->GetFullScreenVirtualRect();
    float32 screenXMin = fullVirtualScreen.x;
    float32 screenXMax = fullVirtualScreen.x + fullVirtualScreen.dx;
    float32 screenYMin = 0.f;
    float32 screenYMax = (float32)VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dy;

    if (minBackground)
	{
		minBackground->SetParentColor(GetBackground()->GetDrawColor());
		RenderSystem2D::Instance()->PushClip();
        RenderSystem2D::Instance()->IntersectClipRect(Rect(screenXMin, screenYMin, clipPointAbsolute - screenXMin, screenYMax));
		minBackground->Draw(geometricData);
		RenderSystem2D::Instance()->PopClip();
	}
	if (maxBackground)
	{
		maxBackground->SetParentColor(GetBackground()->GetDrawColor());
		RenderSystem2D::Instance()->PushClip();
        RenderSystem2D::Instance()->IntersectClipRect(Rect(clipPointAbsolute, screenYMin, screenXMax - clipPointAbsolute, screenYMax));
		maxBackground->Draw(geometricData);
		RenderSystem2D::Instance()->PopClip();
	}

	if (!minBackground && !maxBackground)
	{
		UIControl::Draw(geometricData);
	}
}

void UISlider::LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader)
{
    RemoveControl(thumbButton);
    SafeRelease(thumbButton);

	UIControl::LoadFromYamlNode(node, loader);

	// Values
	const YamlNode * valueNode = node->Get("value");
	
	if (valueNode)
		SetValue(valueNode->AsFloat());
		
	const YamlNode * minValueNode= node->Get("minValue");
	
	if (minValueNode)
		SetMinValue(minValueNode->AsFloat());
		
	const YamlNode * maxValueNode= node->Get("maxValue");
	
	if (maxValueNode)
		SetMaxValue(maxValueNode->AsFloat());

    const YamlNode* spritesEmbeddedNode = node->Get("spritesEmbedded");
    if (spritesEmbeddedNode)
    {
        this->spritesEmbedded = spritesEmbeddedNode->AsBool();
    }
    
    if (this->spritesEmbedded)
    {
        // File is saved in new format - load the backgrounds.
        LoadBackgound("min", minBackground, node, loader);
        LoadBackgound("max", maxBackground, node, loader);
    }
}

void UISlider::SetSize(const DAVA::Vector2 &newSize)
{
    UIControl::SetSize(newSize);
    RecalcButtonPos();
}
    
void UISlider::LoadFromYamlNodeCompleted()
{
    AttachToSubcontrols();
    if (!spritesEmbedded)
    {
        // Old Yaml format is used - have to take their data and remove subcontrols.
        UIControl* minBgControl = FindByName(UISLIDER_MIN_SPRITE_CONTROL_NAME, false);
        CopyBackgroundAndRemoveControl(minBgControl, minBackground);
        
        UIControl* maxBgControl = FindByName(UISLIDER_MAX_SPRITE_CONTROL_NAME, false);
        CopyBackgroundAndRemoveControl(maxBgControl, maxBackground);
    }

    SyncThumbWithSprite();
}
	
YamlNode * UISlider::SaveToYamlNode(UIYamlLoader * loader)
{
    thumbButton->SetName(UISLIDER_THUMB_SPRITE_CONTROL_NAME);

    YamlNode *node = UIControl::SaveToYamlNode(loader);

	// Sprite value
	float32 value = this->GetValue();
	node->Set("value", value);
	
	// Sprite min value
	value = this->GetMinValue();
	node->Set("minValue", value);
	
	// Sprite max value
	value = this->GetMaxValue();
	node->Set("maxValue", value);

    // Min/max background sprites.
    SaveBackground("min", minBackground, node, loader);
    SaveBackground("max", maxBackground, node, loader);

    // Sprites are now embedded into UISlider.
    node->Set("spritesEmbedded", true);

    return node;
}

UISlider* UISlider::Clone()
{
	UISlider *t = new UISlider(GetRect());
	t->CopyDataFrom(this);
	return t;
}
	
void UISlider::CopyDataFrom(UIControl *srcControl)
{
    RemoveControl(thumbButton);
    SafeRelease(thumbButton);

	UIControl::CopyDataFrom(srcControl);
	UISlider* t = (UISlider*) srcControl;

	isEventsContinuos = t->isEventsContinuos;
	
	leftInactivePart = t->leftInactivePart;
	rightInactivePart = t->rightInactivePart;
	
	minValue = t->minValue;
	maxValue = t->maxValue;
	
	currentValue = t->currentValue;
    
    SafeRelease(minBackground);
	if (t->minBackground)
	{
        minBackground = t->minBackground->Clone();
	}

    SafeRelease(maxBackground);
	if (t->maxBackground)
	{
        maxBackground = t->maxBackground->Clone();
	}
	
	relTouchPoint = t->relTouchPoint;
}
	
void UISlider::AttachToSubcontrols()
{
	if (!thumbButton)
	{
		thumbButton = FindByName(UISLIDER_THUMB_SPRITE_CONTROL_NAME);
		DVASSERT(thumbButton);
        thumbButton->Retain();
	}

    InitInactiveParts(thumbButton->GetBackground()->GetSprite());
}

void UISlider::LoadBackgound(const char* prefix, UIControlBackground* background, const YamlNode* rootNode, const UIYamlLoader* loader)
{
    const YamlNode * colorNode = rootNode->Get(Format("%scolor", prefix));
    const YamlNode * spriteNode = rootNode->Get(Format("%ssprite", prefix));
    const YamlNode * frameNode = rootNode->Get(Format("%sframe", prefix));
    const YamlNode * alignNode = rootNode->Get(Format("%salign", prefix));
    const YamlNode * colorInheritNode = rootNode->Get(Format("%scolorInherit", prefix));
    const YamlNode * pixelAccuracyNode = rootNode->Get(Format("%spixelAccuracy", prefix));
    const YamlNode * drawTypeNode = rootNode->Get(Format("%sdrawType", prefix));
    const YamlNode * leftRightStretchCapNode = rootNode->Get(Format("%sleftRightStretchCap", prefix));
    const YamlNode * topBottomStretchCapNode = rootNode->Get(Format("%stopBottomStretchCap", prefix));
    const YamlNode * spriteModificationNode = rootNode->Get(Format("%sspriteModification", prefix));
    const YamlNode * marginsNode = rootNode->Get(Format("%smargins", prefix));

    if (colorNode)
    {
        background->SetColor(colorNode->AsColor());
    }

    if (spriteNode)
    {
        Sprite* sprite = Sprite::Create(spriteNode->AsString());
        background->SetSprite(sprite, 0);
        SafeRelease(sprite);
    }
    
    if (frameNode)
    {
        background->SetFrame(frameNode->AsInt32());
    }

    if (alignNode)
    {
        background->SetAlign(loader->GetAlignFromYamlNode(alignNode));
    }

    if (colorInheritNode)
    {
        background->SetColorInheritType((UIControlBackground::eColorInheritType)loader->GetColorInheritTypeFromNode(colorInheritNode));
    }

    if (pixelAccuracyNode)
    {
        background->SetPerPixelAccuracyType((UIControlBackground::ePerPixelAccuracyType)loader->GetPerPixelAccuracyTypeFromNode(pixelAccuracyNode));
    }
    
    if(drawTypeNode)
    {
        background->SetDrawType((UIControlBackground::eDrawType)loader->GetDrawTypeFromNode(drawTypeNode));
        
        if(leftRightStretchCapNode)
        {
            background->SetLeftRightStretchCap(leftRightStretchCapNode->AsFloat());
        }
        
        if(topBottomStretchCapNode)
        {
            background->SetTopBottomStretchCap(topBottomStretchCapNode->AsFloat());
        }
    }

    if (spriteModificationNode)
    {
        background->SetModification(spriteModificationNode->AsInt32());
    }
    
    if (marginsNode)
    {
        UIControlBackground::UIMargins margins(marginsNode->AsVector4());
        background->SetMargins(&margins);
    }
}

void UISlider::SaveBackground(const char* prefix, UIControlBackground* background, YamlNode* rootNode, const UIYamlLoader * loader)
{
    if (!background)
    {
        return;
    }

    ScopedPtr<UIControlBackground> baseBackground(new UIControlBackground());

    // Color.
    Color color = background->GetColor();
    if (baseBackground->GetColor() != color)
    {
        VariantType* nodeValue = new VariantType();
        nodeValue->SetColor(color);
        rootNode->Set(Format("%scolor", prefix), nodeValue);
        SafeDelete(nodeValue);
    }

    // Sprite.
    Sprite *sprite = background->GetSprite();
    if (sprite)
    {
        rootNode->Set(Format("%ssprite", prefix), Sprite::GetPathString(sprite));
    }
    int32 frame = background->GetFrame();
    if (baseBackground->GetFrame() != frame)
    {
        rootNode->Set(Format("%sframe", prefix), frame);
    }

    // Align
    int32 align = background->GetAlign();
    if (baseBackground->GetAlign() != align)
    {
        rootNode->AddNodeToMap(Format("%salign", prefix), loader->GetAlignNodeValue(align));
    }

    // Color inherit
    UIControlBackground::eColorInheritType colorInheritType =  background->GetColorInheritType();
    if (baseBackground->GetColorInheritType() != colorInheritType)
    {
        rootNode->Set(Format("%scolorInherit", prefix), loader->GetColorInheritTypeNodeValue(colorInheritType));
    }
    
    // Per pixel accuracy
    UIControlBackground::ePerPixelAccuracyType perPixelAccuracyType =  background->GetPerPixelAccuracyType();
    if (baseBackground->GetPerPixelAccuracyType() != perPixelAccuracyType)
    {
        rootNode->Set(Format("%spixelAccuracy", prefix), loader->GetPerPixelAccuracyTypeNodeValue(perPixelAccuracyType));
    }

    // Draw type.
    UIControlBackground::eDrawType drawType = background->GetDrawType();
    rootNode->Set(Format("%sdrawType", prefix), loader->GetDrawTypeNodeValue(drawType));

    // Stretch Cap.
    float32 leftRightStretchCap = background->GetLeftRightStretchCap();
    if (!FLOAT_EQUAL(baseBackground->GetLeftRightStretchCap(), leftRightStretchCap))
    {
        rootNode->Set(Format("%sleftRightStretchCap", prefix), leftRightStretchCap);
    }

    float32 topBottomStretchCap = background->GetTopBottomStretchCap();
    if (!FLOAT_EQUAL(baseBackground->GetTopBottomStretchCap(), topBottomStretchCap))
    {
        rootNode->Set(Format("%stopBottomStretchCap", prefix), topBottomStretchCap);
    }

    // spriteModification
    int32 modification = background->GetModification();
    if (baseBackground->GetModification() != modification)
    {
        rootNode->Set(Format("%sspriteModification", prefix), modification);
    }

    // margins.
    const UIControlBackground::UIMargins* margins = background->GetMargins();
    if (margins)
    {
        rootNode->Set(Format("%smargins", prefix), margins->AsVector4());
    }
}

void UISlider::CopyBackgroundAndRemoveControl(UIControl* from, UIControlBackground*& to)
{
    if (!from)
    {
        return;
    }
    
    if (to)
    {
        SafeRelease(to);
    }

    to = from->GetBackground()->Clone();
    RemoveControl(from);
}

int32 UISlider::GetBackgroundComponentsCount() const
{
    return BACKGROUND_COMPONENTS_COUNT;
}

UIControlBackground *UISlider::GetBackgroundComponent(int32 index) const
{
    switch (index)
    {
        case 0:
            return GetBackground();
            
        case 1:
            return minBackground;
            
        case 2:
            return maxBackground;
            
        default:
            DVASSERT(false);
            return NULL;
    }
}

UIControlBackground *UISlider::CreateBackgroundComponent(int32 index) const
{
    DVASSERT(0 <= index && index < BACKGROUND_COMPONENTS_COUNT);
    return new UIControlBackground();
}

void UISlider::SetBackgroundComponent(int32 index, UIControlBackground *bg)
{
    DVASSERT(false);
}

String UISlider::GetBackgroundComponentName(int32 index) const
{
    DVASSERT(0 <= index && index < BACKGROUND_COMPONENTS_COUNT);
    static const String names[BACKGROUND_COMPONENTS_COUNT] = {"Background", "min", "max"};
    return names[index];
}

} // ns
