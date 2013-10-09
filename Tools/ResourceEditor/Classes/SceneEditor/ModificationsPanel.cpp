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



#include "ModificationsPanel.h"
#include "ControlsFactory.h"

#include "ModificationPopUp.h"
#include "../EditorScene.h"
#include "EditorConfig.h"
#include "Scene3D/Components/DebugRenderComponent.h"

#include "Scene3D/Components/CustomPropertiesComponent.h"


static const float32 BUTTON_W = 20.0f;
static const float32 BUTTON_B = 5.0f;

static const WideString mods[3] = { L"M", L"R", L"S"};
static const WideString axises[3] = { L"X", L"Y", L"Z"};

ModificationsPanel::ModificationsPanel(ModificationsPanelDelegate *newDelegate, const Rect &rect, bool rectInAbsoluteCoordinates)
    :   UIControl(rect, rectInAbsoluteCoordinates)
    ,   delegate(newDelegate)
    ,   workingScene(NULL)

{
    SetInputEnabled(false, false);
    
	modState = MOD_MOVE;
	modAxis = AXIS_X;
	
	modificationPanel = ControlsFactory::CreatePanelControl(Rect(0.f, 5.f, 160.f, 45.f));
    modificationPanel->GetBackground()->SetColor(Color(1.0f, 1.0f, 1.0f, 0.2f));
	
	for (int32 i = 0; i < 3; ++i)
	{
		btnMod[i] = ControlsFactory::CreateButton(Rect((float32)(BUTTON_W + BUTTON_B) * i, 0.f, (float32)BUTTON_W, (float32)BUTTON_W), mods[i]);
		btnMod[i]->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ModificationsPanel::OnModificationPressed));
		modificationPanel->AddControl(btnMod[i]);
        
		btnAxis[i] = ControlsFactory::CreateButton(Rect((float32)(BUTTON_W + BUTTON_B) * i, (float32)(BUTTON_W + BUTTON_B), (float32)BUTTON_W, (float32)BUTTON_W), axises[i]);
		btnAxis[i]->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ModificationsPanel::OnModificationPressed));
		modificationPanel->AddControl(btnAxis[i]);
	}
	UIStaticText * st = new UIStaticText(Rect(55.f, 0, 70.f, (float32)BUTTON_W));
    st->SetFont(ControlsFactory::GetFont12());
	st->SetTextColor(ControlsFactory::GetColorLight());
	st->SetText(L"w, e, r");
    modificationPanel->AddControl(st);
    
	st = new UIStaticText(Rect(55.f, (float32)(BUTTON_W + BUTTON_B), 80.f, (float32)BUTTON_W));
    st->SetFont(ControlsFactory::GetFont12());
	st->SetTextColor(ControlsFactory::GetColorLight());
	st->SetText(L"5, 6, 7, 8");
    modificationPanel->AddControl(st);
	
	btnPlaceOn = ControlsFactory::CreateButton(Rect(115.f, 0, (float32)BUTTON_W, (float32)BUTTON_W), L"P");
	btnPlaceOn->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ModificationsPanel::OnModificationPressed));
	modificationPanel->AddControl(btnPlaceOn);
    
	btnLandscape = ControlsFactory::CreateButton(Rect(115, BUTTON_W + BUTTON_B, BUTTON_W, BUTTON_W), L"L");
	btnLandscape->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ModificationsPanel::OnLandscapeRelative));
	modificationPanel->AddControl(btnLandscape);
	
	btnPopUp = ControlsFactory::CreateButton(Rect(140.f, 0, (float32)BUTTON_W, (float32)BUTTON_W), L"#");
	btnPopUp->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ModificationsPanel::OnModificationPopUpPressed));
	modificationPanel->AddControl(btnPopUp);
	
	modificationPopUp = new ModificationPopUp();
	
	btnModeSelection = ControlsFactory::CreateButton(Rect(170.f, 5.f, (float32)BUTTON_W, (float32)BUTTON_W), L"S");
	btnModeSelection->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ModificationsPanel::OnModePressed));
	AddControl(btnModeSelection);
    
	btnModeModification = ControlsFactory::CreateButton(Rect(195.f, 5.f, (float32)BUTTON_W, (float32)BUTTON_W), L"M");
	btnModeModification->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ModificationsPanel::OnModePressed));
	AddControl(btnModeModification);

	btnModeCollision = new ComboBox(Rect(220.f, 5.f, 80.0f, (float32)BUTTON_W), this,
									EditorConfig::Instance()->GetComboPropertyValues("CollisionType"));
	btnModeCollision->SetMaxVisibleItemsCount(20);
	AddControl(btnModeCollision);

    isLandscapeRelative = false;
	modeCollision = 0;
    
	UpdateModState();
	OnModePressed(btnModeSelection, 0, 0);
}

ModificationsPanel::~ModificationsPanel()
{
    SafeRelease(workingScene);
    
    SafeRelease(btnLandscape);
	SafeRelease(btnPopUp);
	SafeRelease(btnModeSelection);
	SafeRelease(btnModeModification);
	SafeRelease(btnModeCollision);

	for (int32 i = 0; i < 3; ++i)
	{
		SafeRelease(btnMod[i]);
		SafeRelease(btnAxis[i]);
	}
	SafeRelease(modificationPanel);
    SafeRelease(modificationPopUp);
}

void ModificationsPanel::SetScene(EditorScene *scene)
{
    SafeRelease(workingScene);
    workingScene = SafeRetain(scene);
}


void ModificationsPanel::OnModePressed(BaseObject * object, void *, void *)
{
	isModeModification = (object == btnModeModification);
	
	if (isModeModification)
	{
		btnModeModification->SetState(UIControl::STATE_SELECTED);
		btnModeSelection->SetState(UIControl::STATE_NORMAL);
	}
	else
	{
		btnModeModification->SetState(UIControl::STATE_NORMAL);
		btnModeSelection->SetState(UIControl::STATE_SELECTED);
	}
}

void ModificationsPanel::OnItemSelected(ComboBox *forComboBox, const String &itemKey, int itemIndex)
{
	if (modeCollision != itemIndex)
	{
		modeCollision = itemIndex;
		ChangeCollisionModeShow(workingScene);
	}
}

void ModificationsPanel::UpdateCollisionTypes(void)
{
	btnModeCollision->SetNewItemsSet(EditorConfig::Instance()->GetComboPropertyValues("CollisionType"));
}

void ModificationsPanel::OnLandscapeRelative(BaseObject *, void *, void *)
{
    isLandscapeRelative = !isLandscapeRelative;
	if (isLandscapeRelative)
	{
		btnLandscape->SetState(UIControl::STATE_SELECTED);
	}
	else
	{
		btnLandscape->SetState(UIControl::STATE_NORMAL);
	}
}

void ModificationsPanel::ChangeCollisionModeShow(Entity * node)
{
	if (!node)
		return;

	if (modeCollision)
	{
		KeyedArchive * customProperties = node->GetCustomProperties();
		if(customProperties && customProperties->IsKeyExists("CollisionType") && customProperties->GetInt32("CollisionType", 0) == modeCollision)
		{
			node->SetDebugFlags(node->GetDebugFlags() | (DebugRenderComponent::DEBUG_DRAW_RED_AABBOX));
			return;
		}
	}
	else
	{
		node->SetDebugFlags(node->GetDebugFlags() & (~DebugRenderComponent::DEBUG_DRAW_RED_AABBOX));
	}

	int size = node->GetChildrenCount();
	for (int i = 0; i < size; i++)
		ChangeCollisionModeShow(node->GetChild(i));
 }

void ModificationsPanel::OnReloadScene()
{
	bool val = (modeCollision != 0);
	modeCollision = 0;
	ChangeCollisionModeShow(workingScene);
	if (val)
	{
		modeCollision = true;
		ChangeCollisionModeShow(workingScene);
	}
}


void ModificationsPanel::OnModificationPopUpPressed(BaseObject *, void *, void *)
{
	if(modificationPopUp->GetParent())
	{
		RemoveControl(modificationPopUp);
		modificationPopUp->SetSelection(0);
	}
	else
	{
		modificationPopUp->SetSelection(workingScene->GetProxy());
		AddControl(modificationPopUp);
	}
}

void ModificationsPanel::OnModificationPressed(BaseObject * object, void *, void *)
{
	if (object == btnPlaceOn)
	{
        PlaceOnLandscape();
	}
    else if(object == btnLandscape)
    {
        
    }
    else 
    {
        for (int32 i = 0; i < 3; ++i)
        {
            if (object == btnMod[i])
            {
                modState = (eModState)i;
            }
            else if (object == btnAxis[i])
            {
                modAxis = (eModAxis)i;
            }
        }
        UpdateModState();
    }
}

void ModificationsPanel::PlaceOnLandscape()
{
    if(delegate)
    {
        delegate->OnPlaceOnLandscape();
    }
}


void ModificationsPanel::UpdateModState(void)
{
	for (int32 i = 0; i < 3; ++i)
	{
		btnMod[i]->SetState(UIControl::STATE_NORMAL);
		btnAxis[i]->SetState(UIControl::STATE_NORMAL);
	}
    
	btnMod[modState]->SetState(UIControl::STATE_SELECTED);

	switch (modAxis) 
	{
	case AXIS_X:
	case AXIS_Y:
	case AXIS_Z:
		btnAxis[modAxis]->SetState(UIControl::STATE_SELECTED);
		break;
	case AXIS_XY:
		btnAxis[AXIS_X]->SetState(UIControl::STATE_SELECTED);
		btnAxis[AXIS_Y]->SetState(UIControl::STATE_SELECTED);
		break;
	case AXIS_YZ:
		btnAxis[AXIS_Y]->SetState(UIControl::STATE_SELECTED);
		btnAxis[AXIS_Z]->SetState(UIControl::STATE_SELECTED);
		break;
	case AXIS_XZ:
		btnAxis[AXIS_X]->SetState(UIControl::STATE_SELECTED);
		btnAxis[AXIS_Z]->SetState(UIControl::STATE_SELECTED);
		break;
	default:
		break;
	}
}


void ModificationsPanel::Input(DAVA::UIEvent *event)
{    
    if (event->phase == UIEvent::PHASE_KEYCHAR)
    {
        UITextField *tf = dynamic_cast<UITextField *>(UIControlSystem::Instance()->GetFocusedControl());
        if(!tf)
        {
            switch(event->tid)
            {
                case DVKEY_Q:
                    modState = MOD_MOVE;
                    break;

                case DVKEY_E:
                    modState = MOD_ROTATE;
                    break;

                case DVKEY_R:
                    modState = MOD_SCALE;
                    break;

                case DVKEY_5:
                {
                    bool altIsPressed = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_ALT);
                    if(!altIsPressed)
                    {
                        modAxis = AXIS_X;
                    }
                    break;
                }

                case DVKEY_6:
                {
                    bool altIsPressed = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_ALT);
                    if(!altIsPressed)
                    {
                        modAxis = AXIS_Y;
                    }
                    break;
                }

                case DVKEY_7:
                {
                    bool altIsPressed = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_ALT);
                    if(!altIsPressed)
                    {
                        modAxis = AXIS_Z;
                    }
                    break;
                }

                case DVKEY_8:
                {
                    bool altIsPressed = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_ALT);
                    if(!altIsPressed)
                    {
                        if (modAxis < AXIS_XY) modAxis = AXIS_XY;
                        else modAxis = (eModAxis)(AXIS_XY + ((modAxis + 1 - AXIS_XY) % 3));
                    }
                    
                    break;
                }

                case DVKEY_P:
                {
					PlaceOnLandscape();
					break;
                }
					
					
                default:
                    break;
            }
            
			UpdateModState();
        }
	}
	
	UIControl::Input(event);
}

ModificationsPanel::eModState ModificationsPanel::GetModState()
{
    return modState;
}

ModificationsPanel::eModAxis ModificationsPanel::GetModAxis()
{
    return modAxis;
}

bool ModificationsPanel::IsModificationMode()
{
    return isModeModification;
}

void ModificationsPanel::IsModificationMode(bool value)
{
    isModeModification = value;
}

bool ModificationsPanel::IsLandscapeRelative()
{
    return isLandscapeRelative;
}
void ModificationsPanel::IsLandscapeRelative(bool value)
{
    isLandscapeRelative = value;
}


void ModificationsPanel::Update(float32 timeElapsed)
{
    if(workingScene)
    {
        Entity * selection = workingScene->GetProxy();
        modificationPopUp->SetSelection(selection);
        if (isModeModification && selection && modificationPanel->GetParent() == 0)
        {
            AddControl(modificationPanel);
        }
        else if ((selection == 0 && modificationPanel->GetParent() != 0) || !isModeModification)
        {
            RemoveControl(modificationPanel);
            modificationPopUp->SetSelection(0);
            if (modificationPopUp->GetParent())
            {
                modificationPopUp->GetParent()->RemoveControl(modificationPopUp);
            }
        }
    }
    
	
    UIControl::Update(timeElapsed);
}

