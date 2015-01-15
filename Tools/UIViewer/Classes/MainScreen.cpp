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


#include "MainScreen.h"

MainScreen::MainScreen()
{
    projectPath = "";

    resourcePackerDirectory = "./../Bin"; //TODO: set from config, depend on path to framework

    Logger::Debug("MainScreen::MainScreen resourcePackerPath = %s", resourcePackerDirectory.c_str());

    keyNames[InfoControl::IT_NAME] = L"Name:";
    keyNames[InfoControl::IT_TYPE] = L"Type:";
    keyNames[InfoControl::IT_RECT] = L"Rect:";
    keyNames[InfoControl::IT_PIVOT] = L"Pivot:";
    
    selectedControl = NULL;
    //defaultColor = Color(1.0f, 1.0f, 1.0f, 0.2f);
    
    //selectedColor = Color(0.0f, 1.0f, 0.0f, 0.2f);
    
    defaultColor = Color(1.0f, 1.0f, 1.0f, 0.0f);
    selectedColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
    
//    for (int32 i = 0; i < InfoControl::INFO_TYPES_COUNT; ++i) 
//    {
//        keyNames[i] = Format(L"key%02d",i);
//    }
}

void MainScreen::SafeAddControl(UIControl *control)
{
    if(!control->GetParent())
        AddControl(control);
}

void MainScreen::SafeRemoveControl(UIControl *control)
{
    if(control->GetParent())
        RemoveControl(control);
}

void MainScreen::LoadResources()
{
    Vector2 screenSize(GetScreenWidth(), GetScreenHeight());
    
    cellH = GetScreenHeight() / 25;
    buttonW = GetScreenWidth() / 10;
    
    
    float32 infoAreaWidth = screenSize.x - 960.0f;

    Vector2 previewSize(screenSize.x - infoAreaWidth, screenSize.y - cellH);
    //Vector2 previewScale(previewSize.x/screenSize.x, previewSize.y/screenSize.y);
    
    preview = new PreviewControl(Rect(0.0f, cellH, previewSize.x, previewSize.y));
    //preview->SetScaledRect(Rect(0.0f, cellH, previewSize.x*0.9f, previewSize.y*0.9f));
    //preview->SetDebugDraw(true);
    
    AddControl(preview);
    
    FTFont* font = FTFont::Create("~res:/Fonts/MyriadPro-Regular.otf");
    font->SetSize(20.0f);
    font->SetColor(0.8f, 0.8f, 0.8f, 1.0f);
    
    chooseProject = new UIButton(Rect(0.0f, 0.0f, buttonW, cellH));
    chooseProject->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    chooseProject->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(0.0f, 0.0f, 0.0f, 0.5f));
    chooseProject->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    chooseProject->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->SetColor(Color(0.5, 0.5, 0.5, 0.5));
    chooseProject->SetStateFont(UIControl::STATE_NORMAL, font);
    chooseProject->SetStateText(UIControl::STATE_NORMAL, LocalizedString("Project"));
	chooseProject->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &MainScreen::OnButtonPressed));
    AddControl(chooseProject);
    
	loadUI = new UIButton(Rect(2*buttonW, 0, buttonW, cellH));
    loadUI->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    loadUI->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(0.0f, 0.0f, 0.0f, 0.5f));
    loadUI->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    loadUI->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->SetColor(Color(0.5f, 0.5f, 0.5f, 0.5f));
    loadUI->SetStateFont(UIControl::STATE_NORMAL, font);
    loadUI->SetStateText(UIControl::STATE_NORMAL, LocalizedString("Load"));
	loadUI->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &MainScreen::OnButtonPressed));
    AddControl(loadUI);
    loadUI->SetVisible(false);
    
    saveUI = new UIButton(Rect(buttonW*3, 0, buttonW, cellH));
    saveUI->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    saveUI->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(0.0f, 0.0f, 0.0f, 0.5f));
    saveUI->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    saveUI->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->SetColor(Color(0.5f, 0.5f, 0.5f, 0.5f));
    saveUI->SetStateFont(UIControl::STATE_NORMAL, font);
    saveUI->SetStateText(UIControl::STATE_NORMAL, LocalizedString("Save"));
	saveUI->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &MainScreen::OnButtonPressed));
    //TODO: add saveUI when uiviewer becomes editor
    //AddControl(saveUI);
    
    selectHoverModeButton = new UIButton(Rect(buttonW*4, 0, 2*buttonW, cellH));
    
    selectHoverModeButton->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    selectHoverModeButton->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(0.0f, 0.0f, 0.0f, 0.5f));
    selectHoverModeButton->SetStateFont(UIControl::STATE_NORMAL, font);
    selectHoverModeButton->SetStateText(UIControl::STATE_NORMAL, LocalizedString("selection.mode.click"));
    
    selectHoverModeButton->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    selectHoverModeButton->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->SetColor(Color(0.5f, 0.5f, 0.5f, 0.5f));
    
    selectHoverModeButton->SetStateDrawType(UIControl::STATE_SELECTED, UIControlBackground::DRAW_FILL);
    selectHoverModeButton->GetStateBackground(UIControl::STATE_SELECTED)->SetColor(Color(0.0f, 0.0f, 0.0f, 0.5f));
    selectHoverModeButton->SetStateFont(UIControl::STATE_SELECTED, font);
    selectHoverModeButton->SetStateText(UIControl::STATE_SELECTED, LocalizedString("selection.mode.hover"));
    
	selectHoverModeButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &MainScreen::OnButtonPressed));
    AddControl(selectHoverModeButton);
    selectHoverModeButton->SetVisible(false);
    
    for (int32 i = 0; i < InfoControl::INFO_TYPES_COUNT; ++i) 
    {
        keyTexts[i] = new UIStaticText(Rect(screenSize.x - infoAreaWidth, cellH*(2*i+1), infoAreaWidth, cellH));
        keyTexts[i]->SetFont(font);
        keyTexts[i]->SetText(keyNames[i]);
        AddControl(keyTexts[i]);
        
        valueTexts[i] = new UIStaticText(Rect(screenSize.x - infoAreaWidth, cellH*(2*i+2), infoAreaWidth, cellH));
        valueTexts[i]->SetFont(font);
        AddControl(valueTexts[i]);
    }
    
    additionalInfoList = new UIList(Rect(screenSize.x - infoAreaWidth, cellH*(2*InfoControl::INFO_TYPES_COUNT+2), infoAreaWidth, screenSize.y - cellH*(2*InfoControl::INFO_TYPES_COUNT+2) ), UIList::ORIENTATION_VERTICAL);
    additionalInfoList->SetDelegate(this);
    AddControl(additionalInfoList);
    
    fsDlg = new UIFileSystemDialog("~res:/Fonts/MyriadPro-Regular.otf");
    fsDlg->SetDelegate(this);
    Vector<String> filter;
    filter.push_back(".yaml");
    filter.push_back(".YAML");
    fsDlg->SetExtensionFilter(filter);
    fsDlg->SetTitle(LocalizedString("Dlg.Load"));
    fsDlg->SetCurrentDir(FileSystem::Instance()->SystemPathForFrameworkPath("~res:/")); 

    fsDlgProject = new UIFileSystemDialog("~res:/Fonts/MyriadPro-Regular.otf"); //default = GetCurrentWorkingDirectory 
    fsDlgProject->SetDelegate(this);
    fsDlgProject->SetOperationType(UIFileSystemDialog::OPERATION_CHOOSE_DIR);
    fsDlgProject->SetTitle(LocalizedString("Dlg.ChoosePrj"));
    
    KeyedArchive* archive = new KeyedArchive();
	if(archive->Load("~doc:/uiviewer.archive"))
    {
        if(archive->IsKeyExists("projectPath"))
        {
            projectPath = archive->GetString("projectPath");
            //TODO: different path formats on different OS
            OnLoadProject();
            fsDlgProject->SetCurrentDir(projectPath);
        }
    }
    SafeRelease(archive);

    
    SafeRelease(font);
}

void MainScreen::ClearInfoValues()
{
    for (int32 i = 0; i < InfoControl::INFO_TYPES_COUNT; ++i) 
    {
        valueTexts[i]->SetText(L"");
    }    
}

void MainScreen::OnButtonPressed(BaseObject *obj, void *data, void *callerData)
{
    if(obj == loadUI)
    {
        fsDlg->SetOperationType(UIFileSystemDialog::OPERATION_LOAD);
        fsDlg->Show(this);
    }
    if(obj == saveUI)
    {
        fsDlg->SetOperationType(UIFileSystemDialog::OPERATION_SAVE);
        fsDlg->Show(this);
    }
    if(obj == chooseProject)
    {
        SetDisabled(true);
        fsDlgProject->Show(this);
    }
    if(obj == selectHoverModeButton)
    {
        selectHoverModeButton->SetSelected(!selectHoverModeButton->GetSelected());
    }
}

bool MainScreen::IsSelectModeHover()
{
    bool isHover = false;
    if(selectHoverModeButton && selectHoverModeButton->GetSelected())
    {
        isHover = true;
    }
    return isHover;
}

void MainScreen::OnControlHoveredSet(BaseObject *obj, void *data, void *callerData)
{
    if(IsSelectModeHover())
    {
        OnControlSelected(obj, data, callerData);
    }
}

void MainScreen::OnControlHoveredRemoved(BaseObject *obj, void *data, void *callerData)
{
    if(IsSelectModeHover())
    {
        OnControlDeselected(obj, data, callerData);
    }
}

void MainScreen::OnControlSelected(BaseObject *obj, void *data, void *callerData)
{
    if(selectedControl == obj) return;
    
    OnControlDeselected((BaseObject*)selectedControl, data, callerData);
    
    InfoControl* infoControl = dynamic_cast<InfoControl*>(obj);
    selectedControl = SafeRetain(infoControl);
    if(infoControl)
    {
        infoControl->GetBackground()->SetColor(selectedColor);
        for(int32 i = 0; i < InfoControl::INFO_TYPES_COUNT; ++i)
        {
            valueTexts[i]->SetText(infoControl->GetInfoValueText(i));
        }
        
        additionalInfoList->Refresh();
    }
}

void MainScreen::OnControlDeselected(BaseObject *obj, void *data, void *callerData)
{
    ClearInfoValues();
    
    InfoControl* infoControl = dynamic_cast<InfoControl*>(obj);
    
    if(selectedControl && (selectedControl == infoControl))
    {
        selectedControl->GetBackground()->SetColor(defaultColor);
    }
    SafeRelease(selectedControl);
}

void MainScreen::OnFileSelected(UIFileSystemDialog *forDialog, const String &pathToFile)
{
    if(forDialog == fsDlg)
    {
        if(forDialog->GetOperationType() == UIFileSystemDialog::OPERATION_LOAD)
        {
            preview->Load(pathToFile);
            OnLoadUI();
        }
        if(forDialog->GetOperationType() == UIFileSystemDialog::OPERATION_SAVE)
        {
            //TODO: save modified yaml (if viewer becomes editor)
        }
    }
    if(forDialog == fsDlgProject)
    {
        projectPath = pathToFile;
        Logger::Debug("MainScreen::OnFileSelected projectPath = %s", projectPath.c_str());
        OnLoadProject();
    }
}

void MainScreen::OnLoadUI()
{
    // for each info control set callback
    for (int32 i = 0; i < preview->GetInfoControlsCount(); ++i) 
    {
        InfoControl* infoControl = preview->GetInfoControl(i);
        
        //infoControl->SetSpriteDrawType(UIControlBackground::DRAW_FILL);
        infoControl->GetBackground()->SetColor(defaultColor);
        
        infoControl->AddEvent(UIControl::EVENT_HOVERED_SET, Message(this, &MainScreen::OnControlHoveredSet));
        infoControl->AddEvent(UIControl::EVENT_HOVERED_REMOVED, Message(this, &MainScreen::OnControlHoveredRemoved));
        
        infoControl->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &MainScreen::OnControlSelected));
    }

    selectHoverModeButton->SetVisible(true);
}

void MainScreen::OnLoadProject()
{
    KeyedArchive* archive = new KeyedArchive();
    archive->SetString("projectPath", projectPath);
    //TODO: different path formats for different OS
    archive->Save("~doc:/uiviewer.archive");
    SafeRelease(archive);
    
    Logger::Debug("MainScreen::OnLoadProject %s", projectPath.c_str());
    FileSystem::ReplaceBundleName(projectPath + "/Data");
    fsDlg->SetCurrentDir(FileSystem::Instance()->SystemPathForFrameworkPath("~res:/")); 
    
    ConvertGraphics(projectPath + "/DataSource");
    
    SetDisabled(false);

    preview->Unload();
    loadUI->SetVisible(true);
    selectHoverModeButton->SetVisible(false);
}

void MainScreen::ConvertGraphics(const String &path)
{
    // try to execute convert_graphics.py script of project
    // if no script found - run packer
    String convertGraphicsPath = path + "/convert_graphics.py";

    // remember current directory
    String currentWorkingDirectory = FileSystem::Instance()->GetCurrentWorkingDirectory();

    Logger::Debug("MainScreen::ConvertGraphics find %s", convertGraphicsPath.c_str());
    File* convertGraphicsFile = File::Create(convertGraphicsPath, File::OPEN | File::READ);
    if(convertGraphicsFile)
    {
        // found convert_graphics.py

        // cd to project DataSource folder
        Logger::Debug("MainScreen::ConvertGraphics cd %s", path.c_str());
        FileSystem::Instance()->SetCurrentWorkingDirectory(path);
        // execute convert_graphics.py
        FileSystem::Instance()->Spawn("python ./convert_graphics.py");
        // cd back
        Logger::Debug("MainScreen::ConvertGraphics cd %s", currentWorkingDirectory.c_str());
        FileSystem::Instance()->SetCurrentWorkingDirectory(currentWorkingDirectory);
    }
    else
    {
        // convert_graphics.py not found

        // cd to resource packer folder
        Logger::Debug("MainScreen::ConvertGraphics cd %s", resourcePackerDirectory.c_str());
        FileSystem::Instance()->SetCurrentWorkingDirectory(resourcePackerDirectory);
        // run packer
        ExecutePacker(path);
        // cd back
        Logger::Debug("MainScreen::ConvertGraphics cd %s", currentWorkingDirectory.c_str());
        FileSystem::Instance()->SetCurrentWorkingDirectory(currentWorkingDirectory);
    }
    SafeRelease(convertGraphicsFile);

}

void MainScreen::ExecutePacker(const String &path)
{
    ScopedPtr<FileList> fl( new FileList(path) );
    for(int i = 0; i < fl->GetCount(); i++)
    {
        if(fl->IsDirectory(i) && !fl->IsNavigationDirectory(i))
        {
            String name = fl->GetFilename(i);
            size_t find = name.find("Gfx");
		    if(find != name.npos)
		    {
                // convert only Gfx directories
                String gfxSrcPath = fl->GetPathname(i);
                // ResourcePacker
#ifdef __DAVAENGINE_WIN32__
                String spawnCommand = Format("ResourcePacker %s", gfxSrcPath.c_str());
#else
                String spawnCommand = Format("./ResourcePacker %s", gfxSrcPath.c_str());
#endif

                Logger::Debug("MainScreen::ExecutePacker spawn %s", spawnCommand.c_str());
                FileSystem::Instance()->Spawn(spawnCommand);
            }
        }
    }

}

void MainScreen::OnFileSytemDialogCanceled(UIFileSystemDialog *forDialog)
{
    if(forDialog == fsDlgProject)
    {
        SetDisabled(false);
    }
}


void MainScreen::UnloadResources()
{
    RemoveAllControls();
    
    for(int32 i = 0; i < InfoControl::INFO_TYPES_COUNT; ++i)
    {
        SafeRelease(keyTexts[i]);
        SafeRelease(valueTexts[i]);
    }
    SafeRelease(additionalInfoList);
    
    SafeRelease(loadUI);
    SafeRelease(saveUI);
    SafeRelease(chooseProject);

    SafeRelease(fsDlg);
    SafeRelease(fsDlgProject);
    
    SafeRelease(preview);
    
    SafeRelease(selectedControl);
}


void MainScreen::WillAppear()
{
    
}

void MainScreen::WillDisappear()
{
    
}

void MainScreen::Input(UIEvent * event)
{
    if(UIEvent::PHASE_KEYCHAR == event->phase)
    {
        if (event->tid == DVKEY_ESCAPE)
		{
            OnControlDeselected(selectedControl, NULL, NULL);
        }
    }
}

void MainScreen::Update(float32 timeElapsed)
{

}

void MainScreen::Draw(const UIGeometricData &geometricData)
{
    
}

//UIListDelegate
int32 MainScreen::ElementsCount(UIList * list)
{
    if(selectedControl)
    {
        return selectedControl->GetSpecificInfoCount();
    }
    else 
    {
        return 0;
    }
}

UIListCell *MainScreen::CellAtIndex(UIList *list, int32 index)
{
    UIListCell *c = list->GetReusableCell("UI info cell"); //try to get cell from the reusable cells store
    if(!c)
    {
        c = new UIListCell(Rect(0.0f, 0.0f, 2*buttonW, cellH), "UI info cell");
    }
    
    c->RemoveAllControls();
    
    WideString keyStr = (selectedControl ? selectedControl->GetSpecificInfoKeyText(index) : L"");
    WideString valueStr = (selectedControl ? selectedControl->GetSpecificInfoValueText(index) : L"");
    
    FTFont* font = FTFont::Create("~res:/Fonts/MyriadPro-Regular.otf");
    font->SetSize(20.0f);
    font->SetColor(0.8f, 0.8f, 0.8f, 1.0f);
    
    c->SetStateFont(UIControl::STATE_NORMAL, font);
    
    UIStaticText* cellKeyText = new UIStaticText(Rect(0.0f, 0.0f, buttonW, cellH));
    cellKeyText->SetFont(font);
    cellKeyText->SetText(keyStr);
    c->AddControl(cellKeyText);
    
    UIStaticText* cellValueText = new UIStaticText(Rect(buttonW, 0.0f, buttonW, cellH));
    cellValueText->SetFont(font);
    cellValueText->SetText(valueStr);
    c->AddControl(cellValueText);
    
    SafeRelease(font);
    
    return c;//returns cell
}

int32 MainScreen::CellHeight(UIList * list, int32 index)
{
    //control calls this method only when it's in vertical orientation
    return cellH;
}
