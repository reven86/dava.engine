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



#include "ColorControl.h"
#include "ControlsFactory.h"

#include "EditorSettings.h"

ColorControl::ColorControl(const Rect & rect, ColorControlDelegate *newDelegate)
    :   UIControl(rect)
{
    ControlsFactory::CustomizeDialog(this);
    
    delegate = newDelegate;
    
    Rect propertyRect(0, 0, rect.dx, rect.dy - ControlsFactory::BUTTON_HEIGHT);
    colorProperties = new PropertyList(propertyRect, NULL);
    AddControl(colorProperties);
    
	SetupProperties();
    
    Rect buttonRect(0, rect.dy - ControlsFactory::BUTTON_HEIGHT, rect.dx / 2, ControlsFactory::BUTTON_HEIGHT);
    UIButton *btnCancel = ControlsFactory::CreateButton(buttonRect, LocalizedString(L"dialog.cancel"));
    btnCancel->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ColorControl::OnCancel));
    AddControl(btnCancel);
    SafeRelease(btnCancel);

    buttonRect.x = rect.dx - buttonRect.dx;
    UIButton *btnCreate = ControlsFactory::CreateButton(buttonRect, LocalizedString(L"dialog.set"));
    btnCreate->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ColorControl::OnSet));
    AddControl(btnCreate);
    SafeRelease(btnCreate);
}
    
ColorControl::~ColorControl()
{
    delegate = 0;
    SafeRelease(colorProperties);
}

void ColorControl::SetupProperties()
{
	colorProperties->AddSubsection(String("Color Settings"));

	colorProperties->AddColorProperty("property.material.ambientcolor");
	colorProperties->SetColorPropertyValue("property.material.ambientcolor", EditorSettings::Instance()->GetMaterialAmbientColor());

	colorProperties->AddColorProperty("property.material.diffusecolor");
	colorProperties->SetColorPropertyValue("property.material.diffusecolor", EditorSettings::Instance()->GetMaterialDiffuseColor());

	colorProperties->AddColorProperty("property.material.specularcolor");
	colorProperties->SetColorPropertyValue("property.material.specularcolor", EditorSettings::Instance()->GetMaterialSpecularColor());
}

void ColorControl::WillAppear()
{
}

void ColorControl::OnCancel(BaseObject * object, void * userData, void * callerData)
{
    if(GetParent())
    {
        GetParent()->RemoveControl(this);
    }
}

void ColorControl::OnSet(BaseObject * object, void * userData, void * callerData)
{
    if(delegate && colorProperties)
    {
		Color ambient = colorProperties->GetColorPropertyValue(String("property.material.ambientcolor"));
		Color diffuse = colorProperties->GetColorPropertyValue(String("property.material.diffusecolor"));
		Color specular = colorProperties->GetColorPropertyValue(String("property.material.specularcolor"));

		EditorSettings::Instance()->SetMaterialsColor(ambient, diffuse, specular);
		EditorSettings::Instance()->Save();

        delegate->SetupColor(ambient, diffuse, specular);
    }
    
    OnCancel(NULL, NULL, NULL);
}

