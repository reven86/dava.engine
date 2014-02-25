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



#ifndef __LANDSCAPE_EDITOR_BASE_H__
#define __LANDSCAPE_EDITOR_BASE_H__

#include "DAVAEngine.h"
#include "LandscapeToolsPanel.h"

using namespace DAVA;


class LandscapeEditorDelegate
{
public:
    
    virtual void LandscapeEditorStarted() = 0;  //Show LE Controls
    virtual void LandscapeEditorFinished() = 0; //Hide LE Controls
};

class EditorBodyControl;
class EditorScene;
class LandscapeTool;
class HeightmapNode;
class NodesPropertyControl;
class LandscapeEditorBase
    :   public BaseObject
    ,   public UIFileSystemDialogDelegate
    ,   public LandscapeToolsPanelDelegate
{
    enum eLEState
    {
        ELE_NONE = -1,
        ELE_ACTIVE,
        ELE_CLOSING,
        ELE_SAVING_TEXTURE,
        ELE_TEXTURE_SAVED
    };
    
    enum DIALOG_OPERATION
    {
        DIALOG_OPERATION_NONE = -1,
        DIALOG_OPERATION_SAVE,
    };

    static const int32 INVALID_TOUCH_ID = -1;
    static const int32 RAY_TRACING_DISTANCE = 1000;
    
protected:
    virtual ~LandscapeEditorBase();
public:
    LandscapeEditorBase(LandscapeEditorDelegate *newDelegate, EditorBodyControl *parentControl); 
    
	virtual void Draw(const UIGeometricData &geometricData);
    virtual void Update(float32 timeElapsed);
    bool Input(UIEvent * touch);

    virtual bool SetScene(EditorScene *newScene);
    void SetTool(LandscapeTool *newTool);

    void Toggle();
    bool IsActive();
    
    Landscape *GetLandscape();
    
    LandscapeToolsPanel *GetToolPanel();
    virtual NodesPropertyControl *GetPropertyControl(const Rect &rect) = 0;
    
    //file dialog delegate
    virtual void OnFileSelected(UIFileSystemDialog *forDialog, const FilePath &pathToFile);
    virtual void OnFileSytemDialogCanceled(UIFileSystemDialog *forDialog);

    //Tools Panel delegate
    virtual void OnToolSelected(LandscapeTool *newTool);
    virtual void OnShowGrid(bool show);

	virtual void ClearSceneResources();

	virtual void UpdateLandscapeTilemap(Texture* texture) {};

protected:

    virtual void SaveTexture();
    void SaveTextureAs(const FilePath &pathToFile, bool closeLE);

    virtual void InputAction(int32 phase, bool intersects) = 0;
    virtual void HideAction() = 0;
    virtual void ShowAction() = 0;
    virtual void SaveTextureAction(const FilePath &pathToFile) = 0;
	virtual void UpdateCursor() = 0;
    
    virtual void RecreateHeightmapNode() = 0;
    
    void Close();
    LandscapeEditorDelegate *delegate;

    bool GetLandscapePoint(const Vector2 &touchPoint, Vector2 &landscapePoint);
    
    UIFileSystemDialog * fileSystemDialog;
    uint32 fileSystemDialogOpMode;
    
    eLEState state;

    HeightmapNode *heightmapNode;
    Landscape *workingLandscape;
	Entity *workingLandscapeEntity;

    LandscapeTool *currentTool;

    FilePath savedPath;
    int32 landscapeSize;

    EditorScene *workingScene;    
    EditorBodyControl *parent;

    Vector2 landscapePoint;
    Vector2 prevDrawPos;
    
    LandscapeToolsPanel *toolsPanel;

    bool inverseDrawingEnabled;
    int32 touchID;
    
	Texture * cursorTexture;

    
    Landscape::eTiledShaderMode savedShaderMode;
};


#endif //__LANDSCAPE_EDITOR_BASE_H__
