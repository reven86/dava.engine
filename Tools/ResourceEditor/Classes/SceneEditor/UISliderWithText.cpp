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



#include "UISliderWithText.h"
#include "ControlsFactory.h"

REGISTER_CLASS(UISliderWithText);

UISliderWithText::UISliderWithText()
	: UISlider()
{
    sliderText = new UIStaticText(Rect(0, -40, 100, 40));
    sliderText->SetFont(ControlsFactory::GetFont12());
	sliderText->SetTextColor(ControlsFactory::GetColorLight());
    sliderText->SetAlign(ALIGN_HCENTER | ALIGN_BOTTOM);
    sliderText->SetInputEnabled(false);
    
    AddControl(sliderText);
    
    SetSliderText(oldValue = 0.f);
}

UISliderWithText::UISliderWithText(const Rect & rect)
:	UISlider(rect)
{
    sliderText = new UIStaticText(Rect(0, -40, 100, 40));
    sliderText->SetFont(ControlsFactory::GetFont12());
	sliderText->SetTextColor(ControlsFactory::GetColorLight());
    sliderText->SetAlign(ALIGN_HCENTER | ALIGN_BOTTOM);
    sliderText->SetInputEnabled(false);
    
    AddControl(sliderText);
    
    SetSliderText(oldValue = 0.f);
}

UISliderWithText::~UISliderWithText()
{
    SafeRelease(sliderText);
}

void UISliderWithText::Draw( const UIGeometricData &geometricData )
{
    UISlider::Draw(geometricData);
    
    if(oldValue != this->GetValue())
    {
        SetSliderText(oldValue = this->GetValue());
        
        //Draw text
        const Rect & tRect =  thumbButton->GetRect();
        Rect r = sliderText->GetRect();
        float32 x = tRect.x + (tRect.dx / 2.0f) - (r.dx / 2.0f);
        sliderText->SetRect(Rect(x, r.y, r.dx, r.dy));
    }
}

void UISliderWithText::SetSliderText(float32 value)
{
    sliderText->SetText(Format(L"%f", value));
}
