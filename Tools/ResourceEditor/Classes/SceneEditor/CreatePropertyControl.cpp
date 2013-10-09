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



#include "CreatePropertyControl.h"
#include "ControlsFactory.h"

#include "EditorConfig.h"

CreatePropertyControl::CreatePropertyControl(const Rect & rect, CreatePropertyControlDelegate *newDelegate)
    :   UIControl(rect)
	, presetText(NULL)
	, presetCombo(NULL)
	, isPresetMode(false)
{
    ControlsFactory::CustomizeDialog(this);
    
    delegate = newDelegate;
    
    Vector<String> types;
    types.push_back("String");
    types.push_back("Integer");
    types.push_back("Float");
    types.push_back("Bool");

    Rect buttonRect(0, rect.dy - ControlsFactory::BUTTON_HEIGHT, rect.dx / 2, ControlsFactory::BUTTON_HEIGHT);
    UIButton *btnCancel = ControlsFactory::CreateButton(buttonRect, LocalizedString(L"dialog.cancel"));
    btnCancel->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &CreatePropertyControl::OnCancel));
    AddControl(btnCancel);
    SafeRelease(btnCancel);

    buttonRect.x = rect.dx - buttonRect.dx;
    UIButton *btnCreate = ControlsFactory::CreateButton(buttonRect, LocalizedString(L"dialog.create"));
    btnCreate->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &CreatePropertyControl::OnCreate));
    AddControl(btnCreate);
    SafeRelease(btnCreate);
    
    //Rect textRect(0, 0, rect.dx / 3, (rect.dy - buttonRect.dy) / 3);
	Rect textRect(0, 0, rect.dx / 3, buttonRect.dy);
    Rect controlRect(textRect.dx, 0, rect.dx - textRect.dx, textRect.dy);
    
	emptyPresetName = "(none)";

	Vector<String> presetNames = EditorConfig::Instance()->GetProjectPropertyNames();
	presetNames.insert(presetNames.begin(), emptyPresetName);

	presetText = new UIStaticText(textRect);
	presetText->SetText(LocalizedString(L"createproperty.preset"));
	presetText->SetFont(ControlsFactory::GetFont12());
	presetText->SetTextColor(ControlsFactory::GetColorLight());
	AddControl(presetText);

	presetCombo = new ComboBox(controlRect, this, presetNames);
	presetCombo->SetMaxVisibleItemsCount(10);
	AddControl(presetCombo);

	textRect.y = textRect.dy;
    controlRect.y = controlRect.dy;

    UIStaticText *t = new UIStaticText(textRect);
    t->SetText(LocalizedString(L"createproperty.type"));
    t->SetFont(ControlsFactory::GetFont12());
	t->SetTextColor(ControlsFactory::GetColorLight());
    AddControl(t);
    SafeRelease(t);
    
    typeCombo = new ComboBox(controlRect, NULL, types);
    AddControl(typeCombo);

    textRect.y = 2*textRect.dy;
    controlRect.y = 2*controlRect.dy;
    
    t = new UIStaticText(textRect);
    t->SetText(LocalizedString(L"createproperty.name"));
    t->SetFont(ControlsFactory::GetFont12());
	t->SetTextColor(ControlsFactory::GetColorLight());
    AddControl(t);
    SafeRelease(t);
    
    nameField = new UITextField(controlRect);
    ControlsFactory::CustomizeEditablePropertyCell(nameField);
    nameField->SetDelegate(this);
    nameField->SetFont(ControlsFactory::GetFont12());
	nameField->SetTextColor(ControlsFactory::GetColorLight());
    AddControl(nameField);
}
    
CreatePropertyControl::~CreatePropertyControl()
{
	SafeRelease(presetText);
	SafeRelease(presetCombo);
    SafeRelease(typeCombo);
    SafeRelease(nameField);
}

void CreatePropertyControl::WillAppear()
{
    nameField->SetText(L"");
    typeCombo->SetSelectedIndex(0, false);

	Vector<String> presetNames = EditorConfig::Instance()->GetProjectPropertyNames();
	presetNames.insert(presetNames.begin(), emptyPresetName);

	presetCombo->SetNewItemsSet(presetNames);
	presetCombo->SetSelectedIndex(0, false);

	isPresetMode = false;
	
	UpdateEditableControls();
}

void CreatePropertyControl::UpdateEditableControls()
{
	typeCombo->SetDisabled(isPresetMode);
	nameField->SetDisabled(isPresetMode);
	if(isPresetMode)
	{
		int32 index = GetTypeIndexFromValueType(EditorConfig::Instance()->GetPropertyValueType(selectedPresetName));
		typeCombo->SetSelectedIndex(index, false);

		nameField->SetText(StringToWString(selectedPresetName));
		nameField->ReleaseFocus();
	}
}

void CreatePropertyControl::TextFieldShouldReturn(UITextField * textField)
{
    nameField->ReleaseFocus();
}

void CreatePropertyControl::TextFieldShouldCancel(UITextField * textField)
{
    nameField->SetText(L"");
    nameField->ReleaseFocus();
}

void CreatePropertyControl::TextFieldLostFocus(UITextField * textField)
{
}

bool CreatePropertyControl::TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, int32 replacementLength, const WideString & replacementString)
{
    return true;
}

void CreatePropertyControl::OnItemSelected(ComboBox *forComboBox, const String &itemKey, int itemIndex)
{
    if(delegate)
    {
		isPresetMode = (emptyPresetName != itemKey);
		selectedPresetName = itemKey;

		UpdateEditableControls();
    }
}

// const String CreatePropertyControl::GetPropName() const
// {
//     return WStringToString(nameField->GetText());
// }
// 
// int32 CreatePropertyControl::GetPropType() const
// {
//     return typeCombo->GetSelectedIndex();
// }

int32 CreatePropertyControl::GetValueTypeFromTypeIndex(int32 typeIndex)
{
	int32 type = VariantType::TYPE_NONE;
	switch (typeIndex) 
    {
        case CreatePropertyControl::EPT_STRING:
			type = VariantType::TYPE_STRING;
            break;
        case CreatePropertyControl::EPT_INT:
			type = VariantType::TYPE_INT32;
            break;
        case CreatePropertyControl::EPT_FLOAT:
			type = VariantType::TYPE_FLOAT;
            break;
        case CreatePropertyControl::EPT_BOOL:
			type = VariantType::TYPE_BOOLEAN;
            break;
        default:
            break;
    }
	return type;
}

int32 CreatePropertyControl::GetTypeIndexFromValueType(int32 type)
{
	int32 index = 0;
	switch (type) 
    {
		case  VariantType::TYPE_STRING:
			index = CreatePropertyControl::EPT_STRING;
            break;
        case VariantType::TYPE_INT32:
			index = CreatePropertyControl::EPT_INT;
            break;
        case VariantType::TYPE_FLOAT:
			index = CreatePropertyControl::EPT_FLOAT;
            break;
        case VariantType::TYPE_BOOLEAN:
			index = CreatePropertyControl::EPT_BOOL;
            break;
        default:
            break;
    }
	return index;
}

void CreatePropertyControl::OnCancel(BaseObject * object, void * userData, void * callerData)
{
    if(delegate)
    {
        delegate->NodeCreated(false, "", VariantType::TYPE_NONE);
    }
}

void CreatePropertyControl::OnCreate(BaseObject * object, void * userData, void * callerData)
{
    if(delegate)
    {
		if(isPresetMode)
		{
			int32 propertyValueType = EditorConfig::Instance()->GetPropertyValueType(selectedPresetName);
			VariantType *defaultValue = EditorConfig::Instance()->GetPropertyDefaultValue(selectedPresetName);
			delegate->NodeCreated(true, selectedPresetName, propertyValueType, defaultValue);
		}
		else
		{
			int32 valueType = GetValueTypeFromTypeIndex(typeCombo->GetSelectedIndex());
			delegate->NodeCreated(true, WStringToString(nameField->GetText()), valueType);
		}
    }
}

