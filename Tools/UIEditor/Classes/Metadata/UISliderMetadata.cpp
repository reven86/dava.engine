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
