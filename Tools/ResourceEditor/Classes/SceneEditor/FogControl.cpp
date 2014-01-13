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



#include "FogControl.h"
#include "ControlsFactory.h"

FogControl::FogControl(const Rect & rect, FogControlDelegate *newDelegate)
    :   UIControl(rect)
{
    ControlsFactory::CustomizeDialog(this);
    
    delegate = newDelegate;
    
    
    Rect propertyRect(0, 0, rect.dx, rect.dy - ControlsFactory::BUTTON_HEIGHT);
    fogProperties = new PropertyList(propertyRect, NULL);
    AddControl(fogProperties);
    
    ControlsFactory::AddFogSubsection(fogProperties, true, 0.006f, Color::White);
    
    Rect buttonRect(0, rect.dy - ControlsFactory::BUTTON_HEIGHT, rect.dx / 2, ControlsFactory::BUTTON_HEIGHT);
    UIButton *btnCancel = ControlsFactory::CreateButton(buttonRect, LocalizedString(L"dialog.cancel"));
    btnCancel->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &FogControl::OnCancel));
    AddControl(btnCancel);
    SafeRelease(btnCancel);

    buttonRect.x = rect.dx - buttonRect.dx;
    UIButton *btnCreate = ControlsFactory::CreateButton(buttonRect, LocalizedString(L"dialog.set"));
    btnCreate->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &FogControl::OnSet));
    AddControl(btnCreate);
    SafeRelease(btnCreate);
}
    
FogControl::~FogControl()
{
    delegate = 0;
    SafeRelease(fogProperties);
}

void FogControl::WillAppear()
{
}

void FogControl::OnCancel(BaseObject * object, void * userData, void * callerData)
{
    if(GetParent())
    {
        GetParent()->RemoveControl(this);
    }
}

void FogControl::OnSet(BaseObject * object, void * userData, void * callerData)
{
    if(delegate && fogProperties)
    {
        delegate->SetupFog(fogProperties->GetBoolPropertyValue(String("property.material.fogenabled")), 
                           fogProperties->GetFloatPropertyValue(String("property.material.dencity")), 
                           fogProperties->GetColorPropertyValue(String("property.material.fogcolor")));
    }
    
    OnCancel(NULL, NULL, NULL);
}

