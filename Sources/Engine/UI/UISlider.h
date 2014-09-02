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



#ifndef __DAVAENGINE_UI_SLIDER_H__
#define __DAVAENGINE_UI_SLIDER_H__

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"

namespace DAVA
{

class UIButton;
class UISlider : public UIControl
{
protected:
	virtual ~UISlider();
public:
	UISlider();
	
	UISlider(const Rect & rect);

	virtual void AddControl(DAVA::UIControl *control);

	virtual void SetThumbSprite(Sprite * sprite, int32 frame);
	virtual void SetThumbSprite(const FilePath & spriteName, int32 frame);
	
	virtual void SetMinSprite(Sprite * sprite, int32 frame);
	virtual void SetMinSprite(const FilePath & spriteName, int32 frame);
    virtual void SetMinDrawType(UIControlBackground::eDrawType drawType);
    virtual void SetMinLeftRightStretchCap(float32 stretchCap);

	virtual void SetMaxSprite(Sprite * sprite, int32 frame);
	virtual void SetMaxSprite(const FilePath & spriteName, int32 frame);
    virtual void SetMaxDrawType(UIControlBackground::eDrawType drawType);
    virtual void SetMaxLeftRightStretchCap(float32 stretchCap);

    virtual void SetSize(const DAVA::Vector2 &newSize);

	inline float32 GetMinValue();
	inline float32 GetMaxValue();
	void SetMinMaxValue(float32 _minValue, float32 _maxValue);
	
	virtual void Draw(const UIGeometricData &geometricData);
	virtual void SystemDraw(const UIGeometricData &geometricData);
	
	inline bool IsEventsContinuos();
	inline void SetEventsContinuos(bool isEventsContinuos);
	inline float32 GetValue();
    
    void SetMaxValue(float32 value);
    void SetMinValue(float32 value);
	void SetValue(float32 value);

    
    void SetThumb(UIControl *newThumb);
    inline UIControl *GetThumb();
	
	inline UIControlBackground *GetBgMin();
	inline UIControlBackground *GetBgMax();

	virtual void LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader);
	virtual void LoadFromYamlNodeCompleted();

	virtual YamlNode * SaveToYamlNode(UIYamlLoader * loader);

	virtual List<UIControl*> GetSubcontrols();

	virtual UIControl *Clone();
	virtual void CopyDataFrom(UIControl *srcControl);

	virtual void SetVisibleForUIEditor(bool value, bool hierarchic = true);

    // Synchronize thumb size/position according to the thumb sprite.
    void SyncThumbWithSprite();

protected:
	bool isEventsContinuos;
	
	int32 leftInactivePart;
	int32 rightInactivePart;
	
	float32 minValue;
	float32 maxValue;
	
	float32 currentValue;

	void Input(UIEvent *currentInput);
	
	void RecalcButtonPos();

	UIControl * thumbButton;
	UIControl * bgMin;
	UIControl * bgMax;

	Vector2 relTouchPoint;

	UIControlBackground::eDrawType minDrawType;
	UIControlBackground::eDrawType maxDrawType;

    bool needSetMinDrawType;
    bool needSetMaxDrawType;

	void InitThumb();
	void InitMinBackground();
	void InitMaxBackground();
	
	void InitSubcontrols();
	void AttachToSubcontrols();
	void ReleaseAllSubcontrols();
	void InitInactiveParts(Sprite* spr);

	void PostInitBackground(UIControl* backgroundControl);
    void RemoveAndReleaseControl(UIControl* &control);
};
    
    
inline UIControl *UISlider::GetThumb()
{
    return thumbButton;
}

inline UIControlBackground *UISlider::GetBgMin()
{
	return bgMin->GetBackground();
}

inline UIControlBackground *UISlider::GetBgMax()
{
	return bgMax->GetBackground();
}

inline bool UISlider::IsEventsContinuos()
{
	return isEventsContinuos;
}

inline void UISlider::SetEventsContinuos(bool _isEventsContinuos)
{
	isEventsContinuos = _isEventsContinuos;	
}
	
inline float32 UISlider::GetValue()
{
	return currentValue;
}

inline float32 UISlider::GetMinValue()
{
	return minValue;
}
	
inline float32 UISlider::GetMaxValue()
{
	return maxValue;
}

};

#endif