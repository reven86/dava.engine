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



#ifndef __COLOR_PICKER_H__
#define __COLOR_PICKER_H__

#include "DAVAEngine.h"
#include "ExtendedDialog.h"
#include "PropertyList.h"

using namespace DAVA;

class ColorDetailControl: public UIControl
{
protected:
    
    static const int32 SECTIONS_COUNT = 6;
    virtual ~ColorDetailControl();
public:    
    ColorDetailControl(const Rect &rect);

    
	virtual void DrawAfterChilds(const UIGeometricData &geometricData);
    virtual void Input(UIEvent *currentInput);
    
    const Color & GetColor();


protected:
    virtual void ColorSelected(const Vector2 &point) = 0;
    
    Sprite *colorMap; 
    Color selectedColor;

    Vector2 markerPoint;
};


class ColorSelectorControl: public ColorDetailControl
{
public:    
    ColorSelectorControl(const Rect &rect);

    void SetColor(const Color &color);
    
protected:
    virtual void ColorSelected(const Vector2 &point);
    
    void SetInitialColors();
    
    Color sections[SECTIONS_COUNT];
    
};


class ColorMapControl: public ColorDetailControl
{
public:    
    ColorMapControl(const Rect &rect);
    void SetColor(const Color &color);
    
protected:
    virtual void ColorSelected(const Vector2 &point);
    
    Color HSBToRgb(float32 s, float32 b);
    int32 RGBToH(const Color &color);
    int32 hue;
};


class ColorPickerDelegate;
class ColorPicker : public ExtendedDialog, public PropertyListDelegate
{
public:
    ColorPicker(ColorPickerDelegate *newDelegate);
    virtual ~ColorPicker();

    virtual void OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue);
    
    void SetColor(const Color & newColor);
    void Show();
    
protected:

    virtual const Rect GetDialogRect() const;

    void SetColor(const Color & newColor, bool updateColorMap, bool updateSelector);

    void OnOk(BaseObject * owner, void * userData, void * callerData);
    void OnCancel(BaseObject * owner, void * userData, void * callerData);
    
    void OnMapColorChanged(BaseObject * owner, void * userData, void * callerData);
    void OnSelectorColorChanged(BaseObject * owner, void * userData, void * callerData);

    void OnAlphaChanged(BaseObject * owner, void * userData, void * callerData);

    UISlider *alphaValue;

    UIControl *colorPreviewCurrent;
    UIControl *colorPreviewPrev;
    
    ColorPickerDelegate *delegate;
    
    ColorMapControl *colorMapControl;
    ColorSelectorControl *colorSelectorControl;

    PropertyList *colorList;

    Color prevColor;
    Color currentColor;
};



#endif // __SCENE_INFO_CONTROL_H__