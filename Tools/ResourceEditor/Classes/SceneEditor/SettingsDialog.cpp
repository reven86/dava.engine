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



#include "SettingsDialog.h"
#include "ControlsFactory.h"

#include "EditorSettings.h"
#include "../Qt/Main/QtUtils.h"

SettingsDialog::SettingsDialog(const Rect & rect, SettingsDialogDelegate *newDelegate)
    :   UIControl(rect)
    ,   delegate(newDelegate)
{
    ControlsFactory::CustomizeDialogFreeSpace(this);

    Rect dialogRect(rect.dx/4, rect.dy/8, rect.dx / 2, rect.dy * 3 /4);
    dialogPanel = new DraggableDialog(dialogRect);
    ControlsFactory::CustomizeDialog(dialogPanel);
    AddControl(dialogPanel);

    UIStaticText * header = new UIStaticText(Rect(0, 0, dialogRect.dx, ControlsFactory::BUTTON_HEIGHT));
    header->SetFont(ControlsFactory::GetFont12());
	header->SetTextColor(ControlsFactory::GetColorLight());
    header->SetText(LocalizedString(L"settings.header"));
    header->SetAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    dialogPanel->AddControl(header);
    SafeRelease(header);
    
    propertyList = new PropertyList(
                            Rect(0, ControlsFactory::BUTTON_HEIGHT, dialogRect.dx, 
                            dialogRect.dy - ControlsFactory::BUTTON_HEIGHT * 2), this);
    dialogPanel->AddControl(propertyList);
    
    
    float32 buttonY = dialogRect.dy - ControlsFactory::BUTTON_HEIGHT;
    float32 buttonX = (dialogRect.dx - ControlsFactory::BUTTON_WIDTH) / 2.f;
    UIButton *btnClose = ControlsFactory::CreateButton(Vector2(buttonX, buttonY), LocalizedString(L"dialog.close"));
    btnClose->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SettingsDialog::OnClose));
    dialogPanel->AddControl(btnClose);
    SafeRelease(btnClose);
    
    propertyList->AddIntProperty("settingsdialog.screenwidth", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddIntProperty("settingsdialog.screenheight", PropertyList::PROPERTY_IS_EDITABLE);
    languages.push_back("en");
    languages.push_back("ru");
    propertyList->AddComboProperty("settingsdialog.language", languages);
    propertyList->AddBoolProperty("settingsdialog.output", PropertyList::PROPERTY_IS_EDITABLE);
    
    
    propertyList->AddFloatProperty("settingsdialog.cameraspeed1", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("settingsdialog.cameraspeed2", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("settingsdialog.cameraspeed3", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("settingsdialog.cameraspeed4", PropertyList::PROPERTY_IS_EDITABLE);
    
    propertyList->AddIntProperty("settingsdialog.leftpanelwidth", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddIntProperty("settingsdialog.rightpanelwidth", PropertyList::PROPERTY_IS_EDITABLE);
    
    propertyList->AddBoolProperty("settingsdialog.drawgrid", PropertyList::PROPERTY_IS_EDITABLE);

	propertyList->AddBoolProperty("settingsdialog.imposters", PropertyList::PROPERTY_IS_EDITABLE);

	propertyList->AddStringProperty("settingsdialog.designername", PropertyList::PROPERTY_IS_EDITABLE);
    
    propertyList->AddBoolProperty("Enable Preview at Library", PropertyList::PROPERTY_IS_EDITABLE);
}
    
SettingsDialog::~SettingsDialog()
{
    SafeRelease(propertyList);
    SafeRelease(dialogPanel);
}

void SettingsDialog::OnClose(BaseObject * , void * , void * )
{
    for(int32 iLod = 1; iLod < LodComponent::MAX_LOD_LAYERS; ++iLod)
    {
        float32 prev = EditorSettings::Instance()->GetLodLayerDistance(iLod - 1);
        float32 cur = EditorSettings::Instance()->GetLodLayerDistance(iLod);
        
        if(cur <= prev)
        {
            ShowErrorDialog("Lod layers have wrong values.");
            return;
        }
    }
    
    GetParent()->RemoveControl(this);
    if(delegate)
    {
        delegate->SettingsChanged();
    }
}

void SettingsDialog::WillAppear()
{
    propertyList->SetIntPropertyValue("settingsdialog.screenwidth", EditorSettings::Instance()->GetScreenWidth());
    propertyList->SetIntPropertyValue("settingsdialog.screenheight", EditorSettings::Instance()->GetScreenHeight());
    
    String language = EditorSettings::Instance()->GetLanguage();
    int32 index = 0;
    for(int32 i = 0; i < (int32)languages.size(); ++i)
    {
        if(language == languages[i])
        {
            index = i;
            break;
        }
    }
    propertyList->SetComboPropertyIndex("settingsdialog.language", index);
    propertyList->SetBoolPropertyValue("settingsdialog.output", EditorSettings::Instance()->GetShowOutput());
    
    propertyList->SetFloatPropertyValue("settingsdialog.cameraspeed1", EditorSettings::Instance()->GetCameraSpeed(0));
    propertyList->SetFloatPropertyValue("settingsdialog.cameraspeed2", EditorSettings::Instance()->GetCameraSpeed(1));
    propertyList->SetFloatPropertyValue("settingsdialog.cameraspeed3", EditorSettings::Instance()->GetCameraSpeed(2));
    propertyList->SetFloatPropertyValue("settingsdialog.cameraspeed4", EditorSettings::Instance()->GetCameraSpeed(3));
    
    propertyList->SetIntPropertyValue("settingsdialog.leftpanelwidth", EditorSettings::Instance()->GetLeftPanelWidth());
    propertyList->SetIntPropertyValue("settingsdialog.rightpanelwidth", EditorSettings::Instance()->GetRightPanelWidth());
    
    propertyList->SetBoolPropertyValue("settingsdialog.drawgrid", EditorSettings::Instance()->GetDrawGrid());
	propertyList->SetBoolPropertyValue("settingsdialog.imposters", EditorSettings::Instance()->GetEnableImposters());
	propertyList->SetStringPropertyValue("settingsdialog.designername", EditorSettings::Instance()->GetDesignerName());
    
    propertyList->SetBoolPropertyValue("Enable Preview at Library", EditorSettings::Instance()->GetPreviewDialogEnabled());

    
    UIScreen *activeScreen = UIScreenManager::Instance()->GetScreen();
    if(activeScreen)
    {
        Vector2 screenSize = activeScreen->GetSize();
        Vector2 dialogSize = dialogPanel->GetSize();
        dialogPanel->SetPosition((screenSize - dialogSize) / 2);

        this->SetSize(screenSize);
    }
}

void SettingsDialog::OnStringPropertyChanged(PropertyList *, const String &forKey, const String &newValue)
{
    if("settingsdialog.designername" == forKey)
    {
        EditorSettings::Instance()->SetDesignerName(newValue);
        EditorSettings::Instance()->Save();
    }
}

void SettingsDialog::OnFloatPropertyChanged(PropertyList *, const String &forKey, float newValue)
{
    if("settingsdialog.cameraspeed1" == forKey)
    {
        EditorSettings::Instance()->SetCameraSpeed(0, newValue);
        EditorSettings::Instance()->Save();
    }
    else if("settingsdialog.cameraspeed2" == forKey)
    {
        EditorSettings::Instance()->SetCameraSpeed(1, newValue);
        EditorSettings::Instance()->Save();
    }
    else if("settingsdialog.cameraspeed3" == forKey)
    {
        EditorSettings::Instance()->SetCameraSpeed(2, newValue);
        EditorSettings::Instance()->Save();
    }
    else if("settingsdialog.cameraspeed4" == forKey)
    {
        EditorSettings::Instance()->SetCameraSpeed(3, newValue);
        EditorSettings::Instance()->Save();
    }
}

void SettingsDialog::OnIntPropertyChanged(PropertyList *, const String &forKey, int newValue)
{
    if("settingsdialog.screenwidth" == forKey)
    {
        EditorSettings::Instance()->SetScreenWidth(newValue);
        EditorSettings::Instance()->Save();
    }
    else if("settingsdialog.screenheight" == forKey)
    {
        EditorSettings::Instance()->SetScreenHeight(newValue);
        EditorSettings::Instance()->Save();
    }
    else if("settingsdialog.leftpanelwidth" == forKey)
    {
        EditorSettings::Instance()->SetLeftPanelWidth(newValue);
        EditorSettings::Instance()->Save();
    }
    else if("settingsdialog.rightpanelwidth" == forKey)
    {
        EditorSettings::Instance()->SetRightPanelWidth(newValue);
        EditorSettings::Instance()->Save();
    }
}

void SettingsDialog::OnBoolPropertyChanged(PropertyList *, const String &forKey, bool newValue)
{
    if("settingsdialog.output" == forKey)
    {
        EditorSettings::Instance()->SetShowOuput(newValue);
        EditorSettings::Instance()->Save();
    }
    else if("settingsdialog.drawgrid" == forKey)
    {
        EditorSettings::Instance()->SetDrawGrid(newValue);
        EditorSettings::Instance()->Save();
    }
	else if("settingsdialog.imposters" == forKey)
	{
		EditorSettings::Instance()->SetEnableImposters(newValue);
		EditorSettings::Instance()->Save();
	}
    else if("Enable Preview at Library" == forKey)
    {
		EditorSettings::Instance()->SetPreviewDialogEnabled(newValue);
		EditorSettings::Instance()->Save();
    }
}

void SettingsDialog::OnComboIndexChanged(PropertyList *, const String &forKey, int32 , const String &newItemKey)
{
    if("settingsdialog.language" == forKey)
    {
        EditorSettings::Instance()->SetLanguage(newItemKey);
        EditorSettings::Instance()->Save();
    }
}

