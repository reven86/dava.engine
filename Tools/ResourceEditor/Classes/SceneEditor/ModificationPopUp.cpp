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




#include "ModificationPopUp.h"
#include "ControlsFactory.h"

ModificationPopUp::ModificationPopUp()
: DraggableDialog(Rect(100, 100, 150, 150))
{
	ControlsFactory::CustomizeDialog(this);
    
    selection = 0;
	
    UIStaticText *text = new UIStaticText(Rect(0, 0, 80, 20));
    text->SetFont(ControlsFactory::GetFont12());
	text->SetTextColor(ControlsFactory::GetColorLight());
    text->SetText(LocalizedString(L"modificationpanel.modification"));
    AddControl(text);
    SafeRelease(text);
    
	parameters = new PropertyList(Rect(0.0f, 20.0f, 100.0f, 100.0f), this);
	parameters->AddFloatProperty("x");
	parameters->AddFloatProperty("y");
	parameters->AddFloatProperty("z");
	AddControl(parameters);
	
	btnReset = ControlsFactory::CreateButton(Rect(0, 130, 80, 20), L"Reset");
	btnReset->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ModificationPopUp::OnReset));
	AddControl(btnReset);
}

ModificationPopUp::~ModificationPopUp()
{
    SafeRelease(parameters);
	SafeRelease(btnReset);
}

void ModificationPopUp::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
	if (selection)
	{
		Matrix4 modification;
		modification.Identity();
	
		modification.CreateTranslation(Vector3(forList->GetFloatPropertyValue("x"), 
                                               forList->GetFloatPropertyValue("y"), 
                                               forList->GetFloatPropertyValue("z")));	
		selection->SetLocalTransform(selection->GetLocalTransform() * modification);

		forList->SetFloatPropertyValue("x", 0.0f);
		forList->SetFloatPropertyValue("y", 0.0f);
		forList->SetFloatPropertyValue("z", 0.0f);
	}
}

void ModificationPopUp::OnReset(BaseObject * object, void * userData, void * callerData)
{
	if (selection)
	{
		selection->RestoreOriginalTransforms();
	}
}