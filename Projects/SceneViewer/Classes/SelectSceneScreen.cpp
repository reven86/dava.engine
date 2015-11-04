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


#include "SelectSceneScreen.h"
#include "GameCore.h"


#define SETTINGS_PATH "~doc:/SceneViewerSettings.bin"

SelectSceneScreen::SelectSceneScreen()
    : BaseScreen()
    , fileNameText(NULL)
    , fileSystemDialog(NULL)
{
    LoadSettings();
}

void SelectSceneScreen::LoadResources()
{
    BaseScreen::LoadResources();
    
    Rect screenRect = GetRect();
    Size2i screenSize = VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize();
    screenRect.dx = static_cast<float32>(screenSize.dx);
    screenRect.dy = static_cast<float32>(screenSize.dy);
    SetRect(screenRect);
    const float32 buttonSize = 30.f;
    
    fileNameText = new UIStaticText(Rect(0, 0, screenRect.dx - buttonSize * 10, buttonSize));
    fileNameText->SetTextColor(Color::White);
    fileNameText->SetFont(font);
    
    if(scenePath.IsEmpty())
        fileNameText->SetText(L"Select scene file");
    else
        fileNameText->SetText(StringToWString(scenePath.GetStringValue()));
    
    ScopedPtr<UIButton> selectButtonRes(CreateButton(Rect(screenRect.dx - buttonSize * 10, 0, buttonSize * 3, buttonSize), L"~res:/"));
    selectButtonRes->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SelectSceneScreen::OnSelectResourcesPath));

    ScopedPtr<UIButton> selectButtonDoc(CreateButton(Rect(screenRect.dx - buttonSize * 7, 0, buttonSize * 3, buttonSize), L"~doc:/"));
    selectButtonDoc->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SelectSceneScreen::OnSelectDocumentsPath));

    ScopedPtr<UIButton> selectButtonExt(CreateButton(Rect(screenRect.dx - buttonSize * 4, 0, buttonSize * 3, buttonSize), L"ext"));
    selectButtonExt->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SelectSceneScreen::OnSelectExternalStoragePath));
    
    ScopedPtr<UIButton> clearPathButton(CreateButton(Rect(screenRect.dx - buttonSize, 0, buttonSize, buttonSize), L"X"));
    clearPathButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SelectSceneScreen::OnClearPath));

    ScopedPtr<UIButton> startButton(CreateButton(Rect(0, buttonSize, screenRect.dx, buttonSize), L"Start"));
    startButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SelectSceneScreen::OnStart));
    
    AddControl(fileNameText);
    AddControl(selectButtonRes);
    AddControl(selectButtonDoc);
    AddControl(selectButtonExt);
    AddControl(clearPathButton);
    AddControl(startButton);
    
    DVASSERT(fileSystemDialog == NULL);
    fileSystemDialog = new UIFileSystemDialog("~res:/Fonts/korinna.ttf");
	fileSystemDialog->SetDelegate(this);
	fileSystemDialog->SetExtensionFilter(".sc2");
    fileSystemDialog->SetOperationType(UIFileSystemDialog::OPERATION_LOAD);
    
    SetDebugDraw(true, true);
}

void SelectSceneScreen::UnloadResources()
{
    SafeRelease(fileSystemDialog);
    SafeRelease(fileNameText);
    BaseScreen::UnloadResources();
}

void SelectSceneScreen::OnSelectResourcesPath(BaseObject *caller, void *param, void *callerData)
{
    DVASSERT(fileSystemDialog);
    fileSystemDialog->SetCurrentDir("~res:/3d/");

    fileSystemDialog->Show(this);
}

void SelectSceneScreen::OnSelectDocumentsPath(BaseObject *caller, void *param, void *callerData)
{
    DVASSERT(fileSystemDialog);
    fileSystemDialog->SetCurrentDir("~doc:/");
    
    fileSystemDialog->Show(this);
}

void SelectSceneScreen::OnSelectExternalStoragePath(BaseObject *caller, void *param, void *callerData)
{
    DVASSERT(fileSystemDialog);

    auto storageList = DeviceInfo::GetStoragesList();
    for(const auto& storage : storageList)
    {
        if (storage.type == DeviceInfo::STORAGE_TYPE_PRIMARY_EXTERNAL || 
            storage.type == DeviceInfo::STORAGE_TYPE_SECONDARY_EXTERNAL)
        {
            fileSystemDialog->SetCurrentDir(storage.path);
            fileSystemDialog->Show(this);
            return;
        }
    }

    DVASSERT_MSG(false, "No external storages found");
}

void SelectSceneScreen::OnClearPath(BaseObject *caller, void *param, void *callerData)
{
    SetScenePath(FilePath());
    
    fileNameText->SetText(L"Select scene file");
}

void SelectSceneScreen::OnStart(BaseObject *caller, void *param, void *callerData)
{
    if(scenePath.IsEmpty())
    {
        Logger::Error("Scene not selected. Please select scene");
        return;
    }
    
    GameCore::Instance()->SetScenePath(scenePath);
    SetNextScreen();
}

void SelectSceneScreen::OnFileSelected(UIFileSystemDialog *forDialog, const FilePath &pathToFile)
{
    SetScenePath(pathToFile);
    fileNameText->SetText(StringToWString(scenePath.GetStringValue()));
}

void SelectSceneScreen::OnFileSytemDialogCanceled(UIFileSystemDialog *forDialog)
{
}

void SelectSceneScreen::SetScenePath(const DAVA::FilePath &path)
{
    scenePath = path;
    SaveSettings();
}

void SelectSceneScreen::LoadSettings()
{
    ScopedPtr<KeyedArchive> settings(new KeyedArchive());
    settings->Load(SETTINGS_PATH);
    scenePath = settings->GetString("ScenePath", "");
}

void SelectSceneScreen::SaveSettings()
{
    ScopedPtr<KeyedArchive> settings(new KeyedArchive());
    settings->SetString("ScenePath", scenePath.GetStringValue());
    settings->Save(SETTINGS_PATH);
}

