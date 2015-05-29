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



#ifndef __DAVAENGINE_UI_JOYPAD__
#define __DAVAENGINE_UI_JOYPAD__

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"

namespace DAVA 
{
	/**
	 \ingroup controlsystem
	 \brief Joypad realisation for the touch screen supported platforms.
		Incomplete!!!.
	 */
	
class UIJoypad : public UIControl 
{
    enum eTouchID
    {
        TOUCH_INVALID_ID = -1
    };
protected:
    virtual ~UIJoypad();
public:
	UIJoypad(const Rect &rect = Rect(), bool rectInAbsoluteCoordinates = FALSE);
	
	const Vector2 & GetDigitalPosition();
	const Vector2 & GetAnalogPosition();
    
    virtual Sprite* GetStickSprite() const;
    virtual int32 GetStickSpriteFrame() const;

    virtual void SetStickSprite(Sprite *stickSprite, int32 frame);
    virtual void SetStickSprite(const FilePath &stickSpriteName, int32 frame);
    virtual void SetStickSpriteFrame(int32 frame);

	virtual void Input(UIEvent *currentInput); // Can be overrided for control additioanl functionality implementation
	virtual void InputCancelled(UIEvent *currentInput); // Can be overrided for control additioanl functionality implementation

	float32 GetDeadAreaSize() const { return deadAreaSize; }
	void SetDeadAreaSize(float newDeadAreaSize) { deadAreaSize = newDeadAreaSize; }//!< Size of the middle joypad area where the tuches do not come.
	float32 GetDigitalSense() const { return digitalSense; }
	void SetDigitalSense(float32 newDigitalSense) { digitalSense = newDigitalSense; }//!< Sense of the diagonal joypad ways. 0.5 by default.

    float32 GetStickAngle() const;

    // Load/save functionality.
    virtual void LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader);
    virtual YamlNode * SaveToYamlNode(UIYamlLoader * loader);

    virtual List<UIControl* >& GetRealChildren();

    FilePath GetStickSpritePath() const;
    void SetStickSpritePath(const FilePath &path);
    
    void SetStickDebugDraw(bool debugDraw);
    
protected:
	void RecalcDigitalPosition();
	void RecalcAnalogPosition();

    void CreateStickControl();

    UIControl *stick;
    
private:
	int32 mainTouch;
	float deadAreaSize;// dead area size in pixels (must be positive value)
	float32 digitalSense;
	bool needRecalcDigital;
	bool needRecalcAnalog;
	Vector2 currentPos;
	
	Vector2 digitalVector;
	Vector2 analogVector;
public:
    INTROSPECTION_EXTEND(UIJoypad, UIControl,
        PROPERTY("stickSprite", "Stick Sprite", GetStickSpritePath, SetStickSpritePath, I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("stickFrame", "Stick Frame", GetStickSpriteFrame, SetStickSpriteFrame, I_SAVE | I_VIEW | I_EDIT)

        PROPERTY("deadAreaSize", "Dead Area Size", GetDeadAreaSize, SetDeadAreaSize, I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("digitalSense", "Digital Sense", GetDigitalSense, SetDigitalSense, I_SAVE | I_VIEW | I_EDIT)
        );

};
	
};

#endif