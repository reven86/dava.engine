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


#include "UI/UIFileSystemDialog.h"
#include "UI/UIList.h"
#include "UI/UITextField.h"
#include "UI/UIStaticText.h"
#include "FileSystem/FileList.h"
#include "Utils/Utils.h"
#include "Core/Core.h"
#include "Platform/SystemTimer.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include <algorithm>
#include "Render/2D/FTFont.h"

namespace DAVA
{

UIFileSystemDialog::UIFileSystemDialog(const FilePath &_fontPath)
    : UIControl(Rect(VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dx / 2.f,
                     VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dy / 2.f,
                     VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dx * 2.f / 3.f,
                     VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dy * 4.f / 5.f
                     )
                )
{
    fontPath = _fontPath;
    
    background->SetDrawType(UIControlBackground::DRAW_FILL);
    background->SetColor(Color(0.5, 0.5, 0.5, 0.75));
    SetPivot(Vector2(0.5f, 0.5f));
    
    operationType = OPERATION_LOAD;
    delegate = NULL;
    extensionFilter.push_back(".*");
    
    
    cellH = VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dy / 20.0f;
    cellH = Max(cellH, 32.0f);
    float32 border = VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dy / 64.0f;
    float32 halfBorder = float32(int32(border/2.0f));
    fileListView = new UIList(Rect(border, border + cellH, size.x - border*2.0f, size.y - cellH*3.0f - border*3.0f), UIList::ORIENTATION_VERTICAL);
    fileListView->SetDelegate(this);
    fileListView->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    fileListView->GetBackground()->SetColor(Color(0.25, 0.25, 0.25, 0.25));
    AddControl(fileListView);
    
    lastSelectionTime = 0;
    
    Font *f = FTFont::Create(fontPath);
    f->SetSize((float32)(int32(cellH * 2.0f) / 3));
    
    title = new UIStaticText(Rect(border, halfBorder, size.x - border*2.0f, cellH));
    title->SetFont(f);
	title->SetTextColorInheritType(UIControlBackground::COLOR_IGNORE_PARENT);
	title->SetTextColor(Color(1.f, 1.f, 1.f, 1.f));
    title->SetFittingOption(TextBlock::FITTING_REDUCE);
    title->SetText(L"Select file:");
    AddControl(title);

    workingPath = new UIStaticText(Rect(border, halfBorder + fileListView->size.y + fileListView->relativePosition.y, size.x - border*2.0f, cellH));
	workingPath->SetTextColorInheritType(UIControlBackground::COLOR_IGNORE_PARENT);
    workingPath->SetFont(f);
    workingPath->SetAlign(ALIGN_LEFT|ALIGN_VCENTER);
    workingPath->SetFittingOption(TextBlock::FITTING_REDUCE);
    workingPath->SetText(L"c:");
    AddControl(workingPath);
    
    
    float32 buttonW = cellH * 3.0f;
    positiveButton = new UIButton(Rect(size.x - border - buttonW, workingPath->relativePosition.y + halfBorder + cellH, buttonW, cellH));
    positiveButton->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    positiveButton->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(0.5f, 0.6f, 0.5f, 0.5f));
    positiveButton->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    positiveButton->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->SetColor(Color(0.75f, 0.85f, 0.75f, 0.5f));
    positiveButton->SetStateDrawType(UIControl::STATE_DISABLED, UIControlBackground::DRAW_FILL);
    positiveButton->GetStateBackground(UIControl::STATE_DISABLED)->SetColor(Color(0.2f, 0.2f, 0.2f, 0.2f));
    positiveButton->SetStateFont(UIControl::STATE_NORMAL, f);
    positiveButton->SetStateText(UIControl::STATE_NORMAL, L"OK");
	positiveButton->SetStateTextColorInheritType(UIControl::STATE_NORMAL, UIControlBackground::COLOR_IGNORE_PARENT);
	positiveButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIFileSystemDialog::ButtonPressed));
    AddControl(positiveButton);

    negativeButton = new UIButton(Rect((float32)positiveButton->relativePosition.x - buttonW - border, positiveButton->relativePosition.y, buttonW, cellH));
    negativeButton->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    negativeButton->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(0.6f, 0.5f, 0.5f, 0.5f));
    negativeButton->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    negativeButton->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->SetColor(Color(0.85f, 0.75f, 0.75f, 0.5f));
    negativeButton->SetStateDrawType(UIControl::STATE_DISABLED, UIControlBackground::DRAW_FILL);
    negativeButton->GetStateBackground(UIControl::STATE_DISABLED)->SetColor(Color(0.2f, 0.2f, 0.2f, 0.2f));
    negativeButton->SetStateFont(UIControl::STATE_NORMAL, f);
    negativeButton->SetStateText(UIControl::STATE_NORMAL, L"Cancel");
	negativeButton->SetStateTextColorInheritType(UIControl::STATE_NORMAL, UIControlBackground::COLOR_IGNORE_PARENT);
	negativeButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIFileSystemDialog::ButtonPressed));
    AddControl(negativeButton);
    
    
    historyPosition = 0;
    historyBackwardButton = new UIButton(Rect(border, positiveButton->relativePosition.y, cellH, cellH));
    historyBackwardButton->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    historyBackwardButton->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(0.5f, 0.6f, 0.5f, 0.5f));
    historyBackwardButton->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    historyBackwardButton->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->SetColor(Color(0.75f, 0.85f, 0.75f, 0.5f));
    historyBackwardButton->SetStateDrawType(UIControl::STATE_DISABLED, UIControlBackground::DRAW_FILL);
    historyBackwardButton->GetStateBackground(UIControl::STATE_DISABLED)->SetColor(Color(0.2f, 0.2f, 0.2f, 0.2f));
    historyBackwardButton->SetStateFont(UIControl::STATE_NORMAL, f);
    historyBackwardButton->SetStateText(UIControl::STATE_NORMAL, L"<");
	historyBackwardButton->SetStateTextColorInheritType(UIControl::STATE_NORMAL, UIControlBackground::COLOR_IGNORE_PARENT);
	historyBackwardButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIFileSystemDialog::HistoryButtonPressed));
    AddControl(historyBackwardButton);
    
    historyForwardButton = new UIButton(Rect(historyBackwardButton->relativePosition.x + 
                                             historyBackwardButton->size.x + border,
                                             historyBackwardButton->relativePosition.y, 
                                             cellH, cellH));
    historyForwardButton->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    historyForwardButton->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(0.5f, 0.6f, 0.5f, 0.5f));
    historyForwardButton->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    historyForwardButton->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->SetColor(Color(0.75f, 0.85f, 0.75f, 0.5f));
    historyForwardButton->SetStateDrawType(UIControl::STATE_DISABLED, UIControlBackground::DRAW_FILL);
    historyForwardButton->GetStateBackground(UIControl::STATE_DISABLED)->SetColor(Color(0.2f, 0.2f, 0.2f, 0.2f));
    historyForwardButton->SetStateFont(UIControl::STATE_NORMAL, f);
    historyForwardButton->SetStateText(UIControl::STATE_NORMAL, L">");
	historyForwardButton->SetStateTextColorInheritType(UIControl::STATE_NORMAL, UIControlBackground::COLOR_IGNORE_PARENT);
	historyForwardButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIFileSystemDialog::HistoryButtonPressed));
    AddControl(historyForwardButton);
    

//    textField = new UITextField(Rect((float32)border, (float32)positiveButton->relativePosition.y, (float32)negativeButton->relativePosition.x - border*2, (float32)cellH));
    float32 textFieldOffset = historyForwardButton->relativePosition.x + historyForwardButton->size.x + border;
    textField = new UITextField(Rect(textFieldOffset,
                                     positiveButton->relativePosition.y, 
                                     negativeButton->relativePosition.x - border - textFieldOffset, cellH));
    textField->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    textField->GetBackground()->SetColor(Color(0.25f, 0.25f, 0.25f, 0.25f));
    textField->SetFont(f);
    textField->SetDelegate(this);
    
    SafeRelease(f);
    
    files = NULL;
    
    SetCurrentDir(FileSystem::Instance()->GetCurrentWorkingDirectory());
}

void UIFileSystemDialog::ButtonPressed(BaseObject *obj, void *data, void *callerData)
{
    if (obj == negativeButton) 
    {
        Retain();
        GetParent()->RemoveControl(this);
        if (delegate)
        {
            delegate->OnFileSytemDialogCanceled(this);
        }
        Release();
    }
    else if (obj == positiveButton)
    {
        if (operationType == OPERATION_LOAD)
        {
            OnIndexSelected(lastSelectedIndex);
        }
        else if(operationType == OPERATION_CHOOSE_DIR)
        {
            if (lastSelectedIndex >= 0)
            {
                OnFileSelected(files->GetPathname(fileUnits[lastSelectedIndex].indexInFileList));
            }
            else 
            {
                OnFileSelected(currentDir);
            }

            GetParent()->RemoveControl(this);
        }
        else if(operationType == OPERATION_SAVE)
        {
            SaveFinishing();
        }
            

    }

}

void UIFileSystemDialog::SaveFinishing()
{
    if (!textField->GetText().empty())
    {
        FilePath selectedFile(currentDir);
        if (textField->GetText().find(L".") != textField->GetText().npos)
        {
            selectedFile += WStringToString(textField->GetText());
        }
        else 
        {
            selectedFile += (WStringToString(textField->GetText()) + extensionFilter[0]);
        }
        OnFileSelected(selectedFile);
        GetParent()->RemoveControl(this);
    }
}


void UIFileSystemDialog::Show(UIControl *parentControl)
{
    parentControl->AddControl(this);
    RemoveControl(textField);
    switch (operationType) 
    {
        case OPERATION_LOAD:
            positiveButton->SetStateText(UIControl::STATE_NORMAL, L"Load");
            break;
        case OPERATION_SAVE:
            positiveButton->SetStateText(UIControl::STATE_NORMAL, L"Save");
            AddControl(textField);
            break;
        case OPERATION_CHOOSE_DIR:
            positiveButton->SetStateText(UIControl::STATE_NORMAL, L"Choose");
            break;
    }

    RefreshList();
}

void UIFileSystemDialog::SetTitle(const WideString& newTitle)
{
    title->SetText(newTitle);
}

void UIFileSystemDialog::SetCurrentDir(const FilePath &newDirPath, bool rebuildHistory /* = false*/)
{

    //int32 ppos = newDirPath.rfind(".");
    //int32 spos = newDirPath.rfind("/");
    //if (ppos != newDirPath.npos && ppos > spos)
    currentDir = FilePath(newDirPath.GetDirectory());
    selectedFileName = newDirPath.GetFilename();
    
    if(rebuildHistory)
        CreateHistoryForPath(currentDir);
    
    //find current dir at folders history
    bool isInHistory = false;
    for(int32 iFolder = static_cast<int32>(foldersHistory.size() - 1); iFolder >= 0 ; --iFolder)
    {
        if(foldersHistory[iFolder] == currentDir)
        {
            isInHistory = true;
            historyPosition = iFolder;
            break;
        }
    }
    
    // update folders history for current dir
    if(!isInHistory)
        CreateHistoryForPath(currentDir);
    
    // enable/disable navigation buttons
    historyBackwardButton->SetDisabled(0 == historyPosition, false);
    historyForwardButton->SetDisabled(historyPosition == (int32)foldersHistory.size() - 1, false);

//    Logger::Info("Setting path: %s", currentDir.c_str());
//    Logger::Info("Setting file: %s", selectedFile.c_str());
    if (GetParent())
    {
        RefreshList();
    }
}

const FilePath &UIFileSystemDialog::GetCurrentDir()
{
    return currentDir;
}
    
void UIFileSystemDialog::SetExtensionFilter(const String & extensionFilter)
{
    Vector<String>  newExtensionFilter;
    Split(extensionFilter, ";", newExtensionFilter);
    SetExtensionFilter(newExtensionFilter);
}

void UIFileSystemDialog::SetExtensionFilter(const Vector<String> &newExtensionFilter)
{
    DVASSERT(!GetParent());
    extensionFilter.clear();
    extensionFilter = newExtensionFilter;
    
    int32 size = static_cast<int32>(extensionFilter.size());
    for (int32 k = 0; k < size; ++k)
        std::transform(extensionFilter[k].begin(), extensionFilter[k].end(), extensionFilter[k].begin(), ::tolower);
}

const Vector<String> & UIFileSystemDialog::GetExtensionFilter()
{
    return extensionFilter;
}

void UIFileSystemDialog::OnIndexSelected(int32 index)
{
    if (fileUnits[index].type == FUNIT_DIR_INSIDE || fileUnits[index].type == FUNIT_DIR_OUTSIDE)
    {
        SetCurrentDir(files->GetPathname(fileUnits[index].indexInFileList));
    }
    else if (fileUnits[index].type == FUNIT_FILE)
    {
        if (operationType == OPERATION_LOAD) 
        {
            OnFileSelected(files->GetPathname(fileUnits[index].indexInFileList));
            GetParent()->RemoveControl(this);
        }
        else
        {
            SaveFinishing();
        }
        
    }
}

void UIFileSystemDialog::RefreshList()
{
    workingPath->SetText(StringToWString(currentDir.GetAbsolutePathname()));
    if (operationType != OPERATION_CHOOSE_DIR) 
    {
        positiveButton->SetDisabled(true);
    }
    else 
    {
        positiveButton->SetDisabled(false);
    }

    SafeRelease(files);
    lastSelected = NULL;
    lastSelectedIndex = -1;
    Logger::FrameworkDebug("Cur Dir: %s", currentDir.GetAbsolutePathname().c_str());
    if (currentDir.IsEmpty())
    {
        currentDir += "/";
    }
    
    files = new FileList(currentDir);
    files->Sort();
    
    fileUnits.clear();
    int32 cnt = files->GetCount();
    int32 outCnt = 0;
    int32 p = -1;
    
    String curDirPath = currentDir.GetDirectory().GetAbsolutePathname();
    while (true) 
    {
        p = static_cast<int32>(curDirPath.rfind("/", p));
        if(p != static_cast<int32>(curDirPath.npos))
        {
            p--;
            outCnt++;
            if (p <= 0) 
            {
                break;
            }
        }
        else 
        {
            break;
        }
    }

    for (int i = 0; i < cnt; i++)
    {
        if (!files->IsNavigationDirectory(i))
        {
            DialogFileUnit fu;
            fu.name = files->GetFilename(i);
            fu.path = files->GetPathname(i);
            fu.indexInFileList = i;
            fu.type = FUNIT_FILE;
            if (files->IsDirectory(i)) 
            {
                fu.type = FUNIT_DIR_INSIDE;
            }
            else 
            {
                if (operationType == OPERATION_CHOOSE_DIR) 
                {
                    continue;
                }
                if (fu.name == selectedFileName)
                {
                    lastSelectedIndex = static_cast<int32>(fileUnits.size());
                    positiveButton->SetDisabled(false);
                    textField->SetText(StringToWString(files->GetFilename(fu.indexInFileList)));
                }
                String ext = fu.path.GetExtension();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                bool isPresent = false;
                int32 size = static_cast<uint32>(extensionFilter.size());
                for (int32 n = 0; n < size; n++) 
                {
                    if (extensionFilter[n] == ".*" || ext == extensionFilter[n])
                    {
                        isPresent = true;
                        break;
                    }
                }
                if (!isPresent) 
                {
                    continue;
                }
            }

            
            fileUnits.push_back(fu);
        }
        else if(outCnt >= 1 && files->GetFilename(i) == "..")
        {
            DialogFileUnit fud;
            fud.name = "..";
            fud.path = currentDir;
            fud.type = FUNIT_DIR_OUTSIDE;
            fud.indexInFileList = i;
            fileUnits.push_back(fud);
        }
    }
    fileListView->ResetScrollPosition();
    fileListView->Refresh();
}


void UIFileSystemDialog::TextFieldShouldReturn(UITextField * textField)
{
    SaveFinishing();
}

bool UIFileSystemDialog::TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, int32 replacementLength, WideString & replacementString)
{
    if (textField->GetText().size() + replacementLength > 0) 
    {
        positiveButton->SetDisabled(false);
    }
    else 
    {
        positiveButton->SetDisabled(true);
    }

    return true;
}


int32 UIFileSystemDialog::ElementsCount(UIList *forList)
{
    return static_cast<int32>(fileUnits.size());
}

UIListCell *UIFileSystemDialog::CellAtIndex(UIList *forList, int32 index)
{
    UIListCell *c = forList->GetReusableCell("File cell"); //try to get cell from the reusable cells store
    if(!c)
    { //if cell of requested type isn't find in the store create new cell
        c = new UIListCell(Rect(0, 0, (float32)forList->size.x, (float32)cellH), "File cell");
        UIStaticText *text = new UIStaticText(Rect(0, 0, (float32)forList->size.x, (float32)cellH));
        c->AddControl(text);
        text->SetName("CellText");
		text->SetTextColorInheritType(UIControlBackground::COLOR_IGNORE_PARENT);
        text->SetFittingOption(TextBlock::FITTING_REDUCE);
        text->SetAlign(ALIGN_LEFT|ALIGN_VCENTER);

        Font *f = FTFont::Create(fontPath);
        f->SetSize((float32)cellH * 2 / 3);
        text->SetFont(f);
		text->SetTextColor(Color(1.f, 1.f, 1.f, 1.f));
        SafeRelease(f);
        c->GetBackground()->SetColor(Color(0.75, 0.75, 0.75, 0.5));
    }
    UIStaticText *t = (UIStaticText *)c->FindByName("CellText");
    if (fileUnits[index].type == FUNIT_FILE) 
    {
        t->SetText(StringToWString(fileUnits[index].name));
    }
    else 
    {
        t->SetText(StringToWString("[" + fileUnits[index].name + "]"));
    }
    
    if (index != lastSelectedIndex) 
    {
        c->GetBackground()->SetDrawType(UIControlBackground::DRAW_ALIGNED);
    }
    else
    {
        c->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
        lastSelected = c;
    }

    return c;//returns cell
}

float32 UIFileSystemDialog::CellHeight(UIList * /*forList*/, int32 /*index*/)
{
    return cellH;
}

float32 UIFileSystemDialog::CellWidth(UIList* /*forList*/, int32 /*index*/)
{
	return 20.0f;
}

void UIFileSystemDialog::OnCellSelected(UIList *forList, UIListCell *selectedCell)
{
    if (lastSelected) 
    {
        lastSelected->GetBackground()->SetDrawType(UIControlBackground::DRAW_ALIGNED);
    }
    uint64 curTime = SystemTimer::Instance()->AbsoluteMS();
    if (curTime - lastSelectionTime < 330 && lastSelected == selectedCell)
    {
        lastSelected = selectedCell;
        lastSelectedIndex = lastSelected->GetIndex();
        lastSelected->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
        lastSelectionTime = curTime;
        OnIndexSelected(lastSelectedIndex);
    }
    else 
    {
        lastSelected = selectedCell;
        lastSelectedIndex = lastSelected->GetIndex();
        lastSelected->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
        lastSelectionTime = curTime;
        if (operationType == OPERATION_LOAD) 
        {
            if (fileUnits[selectedCell->GetIndex()].type == FUNIT_FILE)
            {
                positiveButton->SetDisabled(false);
            }
            else 
            {
                positiveButton->SetDisabled(true);
            }
        }
        else if (operationType == OPERATION_SAVE)
        {
            if (fileUnits[selectedCell->GetIndex()].type == FUNIT_FILE)
            {
                textField->SetText(StringToWString(files->GetPathname(fileUnits[lastSelectedIndex].indexInFileList).GetFilename()));
                positiveButton->SetDisabled(false);
            }
        }
    }
}

void UIFileSystemDialog::HistoryButtonPressed(BaseObject *obj, void *data, void *callerData)
{
    if (obj == historyBackwardButton) 
    {
        if(historyPosition)
        {
            SetCurrentDir(foldersHistory[historyPosition - 1]);
        }
    }
    else if (obj == historyForwardButton)
    {
        if(historyPosition < (int32)foldersHistory.size() - 1)
        {
            SetCurrentDir(foldersHistory[historyPosition + 1]);
        }
    }
}
    
void UIFileSystemDialog::CreateHistoryForPath(const FilePath &pathToFile)
{
    foldersHistory.clear();

    String absPath = pathToFile.GetAbsolutePathname();
    String::size_type pos = absPath.find("/");
    if(pos == String::npos)
        return;

    String prefix = absPath.substr(0, pos + 1);
    foldersHistory.push_back(prefix);

    Vector<String> folders;
    Split(absPath.substr(pos), "/", folders);

    for(int32 iFolder = 0; iFolder < (int32)folders.size(); ++iFolder)
    {
        FilePath f = foldersHistory[iFolder] + folders[iFolder];
        f.MakeDirectoryPathname();
        foldersHistory.push_back(f);
    }
    historyPosition = static_cast<int32>(foldersHistory.size() - 1);
}

void UIFileSystemDialog::OnFileSelected(const FilePath &pathToFile)
{
    if(delegate)
    {
        delegate->OnFileSelected(this, pathToFile);
    }
}

    
};

