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



#include "DAVAEngine.h"
#include "PreviewControl.h"

using namespace DAVA;

class MainScreen : public UIScreen, public UIFileSystemDialogDelegate, public UIListDelegate
{
protected:
    ~MainScreen(){}
public:
    
    MainScreen();
    
	virtual void LoadResources();
	virtual void UnloadResources();
	virtual void WillAppear();
	virtual void WillDisappear();

	virtual void Update(float32 timeElapsed);
	virtual void Draw(const UIGeometricData &geometricData);
	
	virtual void Input(UIEvent * touch);
    
    virtual int32 ElementsCount(UIList * list);
	virtual UIListCell *CellAtIndex(UIList *list, int32 index);
	virtual int32 CellHeight(UIList * list, int32 index);  //control calls this method only when it's in vertical orientation
    
protected:
    virtual void OnFileSelected(UIFileSystemDialog *forDialog, const String &pathToFile);
    virtual void OnFileSytemDialogCanceled(UIFileSystemDialog *forDialog);
    
    void OnButtonPressed(BaseObject *obj, void *data, void *callerData);
    
    void OnControlHoveredSet(BaseObject *obj, void *data, void *callerData);
    void OnControlHoveredRemoved(BaseObject *obj, void *data, void *callerData);
    
    void OnControlSelected(BaseObject *obj, void *data, void *callerData);
    void OnControlDeselected(BaseObject *obj, void *data, void *callerData);
    
    void ConvertGraphics(const String &path);
    void ExecutePacker(const String &path);
    
    void SafeAddControl(UIControl *control);
    void SafeRemoveControl(UIControl *control);

    void OnLoadProject();
    void OnLoadUI();
    
    void ClearInfoValues();
    
    bool IsSelectModeHover();

    String resourcePackerDirectory;
    String projectPath;
    
    float32 buttonW;
    float32 cellH;
    
    WideString keyNames[InfoControl::INFO_TYPES_COUNT];
    UIStaticText* keyTexts[InfoControl::INFO_TYPES_COUNT];
    UIStaticText* valueTexts[InfoControl::INFO_TYPES_COUNT];
    
    //TODO: additional params (specific for control types)
    UIList* additionalInfoList;
    
    UIButton *loadUI;
    UIButton *saveUI;
    
    UIButton* selectHoverModeButton;

    UIButton *chooseProject;
    
    UIFileSystemDialog *fsDlg;
    UIFileSystemDialog *fsDlgProject;
    
    PreviewControl *preview;
    
    Color defaultColor;
    Color selectedColor;
    InfoControl* selectedControl;
};

