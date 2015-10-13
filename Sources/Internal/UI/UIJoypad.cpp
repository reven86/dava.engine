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


#include "UI/UIJoypad.h"
#include "UI/UIEvent.h"
#include "FileSystem/Logger.h"
#include "FileSystem/YamlNode.h"


namespace DAVA 
{
    
UIJoypad::UIJoypad(const Rect &rect, bool rectInAbsoluteCoordinates/* = FALSE*/)
:	UIControl(rect, rectInAbsoluteCoordinates)
,	stick(NULL)
,	mainTouch(TOUCH_INVALID_ID)
,	deadAreaSize(10.0f)
,	digitalSense(0.5f)
,	needRecalcDigital(true)
,	needRecalcAnalog(true)
,	currentPos(Vector2(0,0))
{
    SetInputEnabled(true, false);
}
    
UIJoypad::~UIJoypad()
{
    SafeRelease(stick);
}

const Vector2 &UIJoypad::GetDigitalPosition()
{
//	if(needRecalcDigital)
//	{
//		RecalcDigitalPosition();
//	}
	if (currentPos.x == 0.f && currentPos.y == 0.f)
	{
		digitalVector.x = 0.0f;
		digitalVector.y = 0.0f;	
		return digitalVector;
	}
	if(needRecalcAnalog)
	{
		RecalcAnalogPosition();
	}

	Vector2 v = analogVector;
	if (fabs(v.x) > fabs(v.y))
	{
		float32 f = fabs(1.f / v.x);
		v.y *= f;
		v.x *= f;
	}
	else
	{
		float32 f = fabs(1.f / v.y);
		v.x *= f;
		v.y *= f;
	}

	//Logger::Info("V pos x = %f, y = %f", v.x, v.y);
	
	float32 xSign = v.x >= 0.0f ? digitalSense : -digitalSense;
	float32 ySign = v.y >= 0.0f ? digitalSense : -digitalSense;
	
	digitalVector.x = 0.0f + (int32)(v.x + xSign);
	digitalVector.y = 0.0f + (int32)(v.y + ySign);	
	
	//Logger::Info("Digital joy pos x = %f, y = %f", digitalVector.x, digitalVector.y);
	
	return digitalVector;
}
const Vector2 &UIJoypad::GetAnalogPosition()
{
	if(needRecalcAnalog)
	{
		RecalcAnalogPosition();
	}
	return analogVector;
}
    
void UIJoypad::CreateStickControl()
{
    if (!stick) 
    {
        stick = new UIControl(Rect(0, 0, 10, 10));
        stick->GetBackground()->SetAlign(ALIGN_HCENTER|ALIGN_VCENTER);
        stick->SetInputEnabled(false);
        stick->SetPivotPoint(Vector2(5, 5));
        stick->relativePosition = Vector2(size.x/2, size.y/2);
        AddControl(stick);
    }
}
    
float32 UIJoypad::GetStickAngle() const
{
    const Vector2 &v = currentPos;

	const float32 len = sqrtf(v.x*v.x + v.y*v.y);
	float32 ang = asinf(v.x / len);
	
    if(v.y > 0)
	{
		ang = PI - ang;
	}
	
    if(ang < 0)
	{
		ang += PI*2;
	}
	
    if(ang > PI*2)
	{
		ang -= PI*2;
	}
    
    return ang;
}

void UIJoypad::RecalcDigitalPosition()
{
	needRecalcDigital = false;
	if(!currentPos.x && !currentPos.y)
	{
		digitalVector.x = 0;
		digitalVector.y = 0;
		return;
	}

	float ang = GetStickAngle();
	
	if(ang > PI/8 && ang < PI - PI/8)
	{
		digitalVector.x = 1.0f;
	}
	else if(ang < PI*2 - PI/8 && ang > PI + PI/8)
	{
		digitalVector.x = -1.0f;
	}
	else
	{
		digitalVector.x = 0;
	}
    
	if(ang < PI/2 - PI/8 || ang > PI*2 - PI/2 + PI/8)
	{
		digitalVector.y = -1.0f;
	}
	else if(ang < PI*2 - PI/2 - PI/8 && ang > PI/2 + PI/8)
	{
		digitalVector.y = 1.0f;
	}
	else
	{
		digitalVector.y = 0;
	}
//	Logger::Info("x = %f, y = %f", digitalVector.x, digitalVector.y);
}
    
void UIJoypad::RecalcAnalogPosition()
{
	needRecalcAnalog = false;
    analogVector.x = currentPos.x/(size.x/2);
    analogVector.y = currentPos.y/(size.y/2);
    //Logger::Info("Analog joy pos x = %f, y = %f", analogVector.x, analogVector.y);
}

Sprite* UIJoypad::GetStickSprite() const
{
    return stick ? stick->GetSprite() : NULL;
}

int32 UIJoypad::GetStickSpriteFrame() const
{
    if (stick && stick->GetSprite())
    {
        return stick->GetFrame();
    }
    
    return 0;
}

void UIJoypad::SetStickSprite(Sprite *stickSprite, int32 frame)
{
    CreateStickControl();
    stick->SetSprite(stickSprite, frame);
}
    
void UIJoypad::SetStickSprite(const FilePath &stickSpriteName, int32 frame)
{
    CreateStickControl();
    stick->SetSprite(stickSpriteName, frame);
}

void UIJoypad::SetStickSpriteFrame(int32 frame)
{
    if (stick && stick->GetSprite())
    {
        stick->SetSpriteFrame(frame);
    }
}

void UIJoypad::Input(UIEvent *currentInput)
{
    if ((TOUCH_INVALID_ID == mainTouch) && currentInput->phase == UIEvent::Phase::BEGAN)
    {
		mainTouch = currentInput->tid;
	}
	
	if(mainTouch != currentInput->tid)
	{
		return;
	}

    if (currentInput->phase == UIEvent::Phase::ENDED)
    {
		currentPos.x = 0;
		currentPos.y = 0;
        mainTouch = TOUCH_INVALID_ID;
	}
	else 
	{
		Rect r = GetGeometricData().GetUnrotatedRect();//GetRect(true);
		currentPos = currentInput->point - r.GetPosition();
		
		currentPos -= Vector2(r.dx * 0.5f, r.dy * 0.5f);

		if(currentPos.x < deadAreaSize &&  currentPos.x > -deadAreaSize && currentPos.y < deadAreaSize &&  currentPos.y > -deadAreaSize)
		{
			currentPos.x = 0;
			currentPos.y = 0;
		}
        currentPos.x = Max(currentPos.x, -size.x/2);
        currentPos.x = Min(currentPos.x, size.x/2);
        currentPos.y = Max(currentPos.y, -size.y/2);
        currentPos.y = Min(currentPos.y, size.y/2);
	}
	if (stick)
	{
		stick->relativePosition.x = size.x/2 + currentPos.x;
		stick->relativePosition.y = size.y/2 + currentPos.y;
	}

	needRecalcAnalog = true;
	needRecalcDigital = true;
	currentInput->SetInputHandledType(UIEvent::INPUT_HANDLED_HARD); // Drag is handled - see please DF-2508.
}
void UIJoypad::InputCancelled(UIEvent *currentInput)
{
	if(currentInput->tid == mainTouch)
	{
		mainTouch = TOUCH_INVALID_ID;

		currentPos.x = 0;
		currentPos.y = 0;

		if (stick)
		{
			stick->relativePosition.x = size.x/2 + currentPos.x;
			stick->relativePosition.y = size.y/2 + currentPos.y;
		}

		needRecalcAnalog = true;
		needRecalcDigital = true;
	}
}

void UIJoypad::LoadFromYamlNode(const DAVA::YamlNode *node, DAVA::UIYamlLoader *loader)
{
    UIControl::LoadFromYamlNode(node, loader);

    const YamlNode * stickSpriteNode = node->Get("stickSprite");
    const YamlNode * stickFrameNode = node->Get("stickFrame");
    const YamlNode * deadAreaSizeNode = node->Get("deadAreaSize");
    const YamlNode * digitalSenseNode = node->Get("digitalSense");

    if (stickSpriteNode)
    {
        int32 spriteFrame = 0;
        if (stickFrameNode)
        {
            spriteFrame = stickFrameNode->AsInt32();
        }
        
        SetStickSprite(stickSpriteNode->AsString(), spriteFrame);
    }

    if (deadAreaSizeNode)
    {
        SetDeadAreaSize(deadAreaSizeNode->AsFloat());
    }

    if (digitalSenseNode)
    {
        SetDigitalSense(digitalSenseNode->AsFloat());
    }
}

YamlNode*  UIJoypad::SaveToYamlNode(DAVA::UIYamlLoader *loader)
{
    ScopedPtr<UIJoypad> baseControl(new UIJoypad());

    YamlNode *node = UIControl::SaveToYamlNode(loader);
    
    // Sprite
    if (stick && stick->GetSprite())
    {
        node->Set("stickSprite", Sprite::GetPathString(stick->GetSprite()));
        node->Set("stickFrame", stick->GetFrame());
    }

    if (baseControl->GetDeadAreaSize() != GetDeadAreaSize())
    {
        node->Set("deadAreaSize", GetDeadAreaSize());
    }

    if (baseControl->GetDigitalSense() != GetDigitalSense())
    {
        node->Set("digitalSense", GetDigitalSense());
    }

    return node;
}
    
List<UIControl* >& UIJoypad::GetRealChildren()
{
    realChilds = UIControl::GetRealChildren();
    realChilds.remove(stick);
    
    return realChilds;
}

DAVA::FilePath UIJoypad::GetStickSpritePath() const
{
    Sprite *sprite = GetStickSprite();
    if (sprite == nullptr)
        return "";
    else if (sprite->GetRelativePathname().GetType() == FilePath::PATH_IN_MEMORY)
        return "";

    return Sprite::GetPathString(sprite);
}

void UIJoypad::SetStickSpritePath(const FilePath &path)
{
    SetStickSprite(path, GetStickSpriteFrame());
}

};



