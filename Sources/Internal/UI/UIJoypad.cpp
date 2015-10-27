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
static const String UIJOYPAD_STICK_NAME = "stick";

UIJoypad::UIJoypad(const Rect& rect)
    : UIControl(rect)
    , stick(nullptr)
    , mainTouch(TOUCH_INVALID_ID)
    , deadAreaSize(10.0f)
    , digitalSense(0.5f)
    , needRecalcDigital(true)
    , needRecalcAnalog(true)
    , currentPos(Vector2(0, 0))
{
    SetInputEnabled(true);

    RefPtr<UIControl> stickCtrl(new UIControl(Rect(0.0f, 0.0f, 10.0f, 10.0f)));
    stickCtrl->SetName(UIJOYPAD_STICK_NAME);
    stickCtrl->GetBackground()->SetAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    stickCtrl->SetInputEnabled(false);
    stickCtrl->SetPivot(Vector2(0.5f, 0.5f));
    stickCtrl->SetPosition(GetSize() / 2.0f);
    AddControl(stickCtrl.Get());
}
    
UIJoypad::~UIJoypad()
{
}

UIJoypad* UIJoypad::Clone()
{
    UIJoypad* control = new UIJoypad();
    control->CopyDataFrom(this);
    return control;
}

void UIJoypad::CopyDataFrom(UIControl* srcControl)
{
    UIControl::CopyDataFrom(srcControl);
    UIJoypad* src = DynamicTypeCheck<UIJoypad*>(srcControl);

    mainTouch = TOUCH_INVALID_ID;
    deadAreaSize = src->deadAreaSize;
    digitalSense = src->digitalSense;
    needRecalcDigital = true;
    needRecalcAnalog = true;
    currentPos = Vector2();
}

void UIJoypad::AddControl(UIControl* control)
{
    UIControl::AddControl(control);
    if (control->GetName() == UIJOYPAD_STICK_NAME && stick.Get() != control)
    {
        stick = control;
    }
}

void UIJoypad::RemoveControl(UIControl* control)
{
    if (control == stick.Get())
    {
        stick = nullptr;
    }

    UIControl::RemoveControl(control);
}

const Vector2 &UIJoypad::GetDigitalPosition()
{
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
    DVASSERT(stick.Valid());
    if (!stick.Valid())
        return;

    stick->SetSprite(stickSprite, frame);
}
    
void UIJoypad::SetStickSprite(const FilePath &stickSpriteName, int32 frame)
{
    DVASSERT(stick.Valid());
    if (!stick.Valid())
        return;

    stick->SetSprite(stickSpriteName, frame);
}

void UIJoypad::SetStickSpriteFrame(int32 frame)
{
    DVASSERT(stick.Valid());

    if (stick.Valid() && stick->GetSprite())
    {
        stick->SetSpriteFrame(frame);
    }
}

void UIJoypad::Input(UIEvent *currentInput)
{
	if((TOUCH_INVALID_ID == mainTouch) && currentInput->phase == UIEvent::PHASE_BEGAN)
	{
		mainTouch = currentInput->tid;
	}
	
	if(mainTouch != currentInput->tid)
	{
		return;
	}
	
	if(currentInput->phase == UIEvent::PHASE_ENDED)
	{
		currentPos.x = 0;
		currentPos.y = 0;
        mainTouch = TOUCH_INVALID_ID;
	}
	else 
	{
        Rect r = GetGeometricData().GetUnrotatedRect();
        currentPos = currentInput->point - r.GetPosition();

        currentPos -= Vector2(r.dx * 0.5f, r.dy * 0.5f);

        if (currentPos.x < deadAreaSize && currentPos.x > -deadAreaSize && currentPos.y < deadAreaSize && currentPos.y > -deadAreaSize)
        {
            currentPos.x = 0;
            currentPos.y = 0;
        }
        currentPos.x = Max(currentPos.x, -size.x/2);
        currentPos.x = Min(currentPos.x, size.x/2);
        currentPos.y = Max(currentPos.y, -size.y/2);
        currentPos.y = Min(currentPos.y, size.y/2);
	}

    if (stick.Valid())
    {
        stick->SetPosition(GetSize() / 2.0f + currentPos);
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

        if (stick.Valid())
        {
            stick->SetPosition(GetSize() / 2.0f + currentPos);
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

float32 UIJoypad::GetDeadAreaSize() const
{
    return deadAreaSize;
}

void UIJoypad::SetDeadAreaSize(float32 newDeadAreaSize)
{
    deadAreaSize = newDeadAreaSize;
}

float32 UIJoypad::GetDigitalSense() const
{
    return digitalSense;
}

void UIJoypad::SetDigitalSense(float32 newDigitalSense)
{
    digitalSense = newDigitalSense;
}
};
