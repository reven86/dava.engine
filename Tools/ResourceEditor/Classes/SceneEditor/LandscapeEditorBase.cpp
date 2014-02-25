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



#include "LandscapeEditorBase.h"

#include "HeightmapNode.h"
#include "EditorSettings.h"
#include "../EditorScene.h"
#include "EditorBodyControl.h"

#include "../Qt/Main/QtUtils.h"
#include "Scene3D/Components/DebugRenderComponent.h"


LandscapeEditorBase::LandscapeEditorBase(LandscapeEditorDelegate *newDelegate, EditorBodyControl *parentControl)
    :   delegate(newDelegate)
    ,   state(ELE_NONE)
    ,   workingScene(NULL)
    ,   parent(parentControl)
    ,   inverseDrawingEnabled(false)
    ,   touchID(INVALID_TOUCH_ID)
{
    fileSystemDialogOpMode = DIALOG_OPERATION_NONE;
    fileSystemDialog = new UIFileSystemDialog("~res:/Fonts/MyriadPro-Regular.otf");
    fileSystemDialog->SetDelegate(this);

    FilePath path = EditorSettings::Instance()->GetDataSourcePath();
    if(!path.IsEmpty())
    {
        fileSystemDialog->SetCurrentDir(path);   
    }

    workingLandscape = NULL;
	workingLandscapeEntity = NULL;
    workingScene = NULL;

    currentTool = NULL;
    heightmapNode = NULL;
    
    toolsPanel = NULL;
    
    landscapeSize = 0;

	cursorTexture = Texture::CreateFromFile("~res:/LandscapeEditor/Tools/cursor/cursor.tex");
	cursorTexture->SetWrapMode(Texture::WRAP_CLAMP_TO_EDGE, Texture::WRAP_CLAMP_TO_EDGE);
    
    savedShaderMode = Landscape::TILED_MODE_TILE_DETAIL_MASK;
}

LandscapeEditorBase::~LandscapeEditorBase()
{
    SafeRelease(toolsPanel);
    
    SafeRelease(heightmapNode);
    SafeRetain(workingLandscape);
	SafeRelease(workingLandscapeEntity);
    SafeRelease(workingScene);
    
    SafeRelease(fileSystemDialog);

	SafeRelease(cursorTexture);
}


void LandscapeEditorBase::Draw(const DAVA::UIGeometricData &geometricData)
{
}

void LandscapeEditorBase::Update(float32 timeElapsed)
{
}

bool LandscapeEditorBase::SetScene(EditorScene *newScene)
{
    SafeRelease(workingScene);
    
    workingLandscape = SafeRetain(FindLandscape(newScene));
	workingLandscapeEntity = SafeRetain(FindLandscapeEntity(newScene));

    if(!workingLandscape)
    {
        ShowErrorDialog(String("No landscape at level."));
        return false;
    }
    
    savedShaderMode = workingLandscape->GetTiledShaderMode();
    if(savedShaderMode == Landscape::TILED_MODE_TEXTURE || savedShaderMode == Landscape::TILED_MODE_MIXED)
    {
        workingLandscape->SetTiledShaderMode(Landscape::TILED_MODE_TILEMASK);
    }
    
    workingScene = SafeRetain(newScene);
    return true;
}

void LandscapeEditorBase::SetTool(LandscapeTool *newTool)
{
    currentTool = newTool;
}

Landscape *LandscapeEditorBase::GetLandscape()
{
    return workingLandscape;
}

bool LandscapeEditorBase::IsActive()
{
    return (ELE_NONE != state);
}

void LandscapeEditorBase::Toggle()
{
    if(ELE_ACTIVE == state)
    {
        state = ELE_CLOSING;
        
        SaveTexture();
    }
    else if(ELE_NONE == state)
    {
        touchID = INVALID_TOUCH_ID;
        
        state = ELE_ACTIVE;
        
        SetTool(toolsPanel->CurrentTool());
        
        if(delegate)
        {
            delegate->LandscapeEditorStarted();
        }
        
        ShowAction();
        
        RecreateHeightmapNode();
    }
}

void LandscapeEditorBase::Close()
{
    HideAction();
    
    // RETURN TO THIS CODE LATER
    // workingLandscape->SetDebugFlags(workingLandscape->GetDebugFlags() & ~DebugRenderComponent::DEBUG_DRAW_GRID);
    
    
	workingLandscape->SetTiledShaderMode(savedShaderMode);
    workingLandscape->UpdateFullTiledTexture();
    savedShaderMode = Landscape::TILED_MODE_TILE_DETAIL_MASK;
    
    SafeRelease(workingLandscape);

    workingScene->RemoveNode(heightmapNode);
    SafeRelease(heightmapNode);

    SafeRelease(workingScene);
    
    state = ELE_NONE;
    
    if(delegate)
    {
        delegate->LandscapeEditorFinished();
    }
}

bool LandscapeEditorBase::GetLandscapePoint(const Vector2 &touchPoint, Vector2 &landscapePoint)
{
    DVASSERT(parent);

    Vector3 from, dir;
    parent->GetCursorVectors(&from, &dir, touchPoint);
    Vector3 to = from + dir * (float32)RAY_TRACING_DISTANCE;
    
    Vector3 point;
    bool isIntersect = workingScene->LandscapeIntersection(from, to, point);
    
    if(isIntersect)
    {
        AABBox3 box = workingLandscape->GetBoundingBox();
            
        //TODO: use 
        landscapePoint.x = (point.x - box.min.x) * (landscapeSize - 1) / (box.max.x - box.min.x);
        landscapePoint.y = (point.y - box.min.y) * (landscapeSize - 1) / (box.max.y - box.min.y);
    }
    
    return isIntersect;
}


bool LandscapeEditorBase::Input(DAVA::UIEvent *touch)
{
	Vector2 point;
	bool isIntersect = GetLandscapePoint(touch->point, point);
    
    point.x = (float32)((int32)point.x);
    point.y = (float32)((int32)point.y);
    
	landscapePoint = point;
	UpdateCursor();
	
    if(INVALID_TOUCH_ID == touchID || touchID == touch->tid)
    {
        if(UIEvent::BUTTON_1 == touch->tid)
        {
            inverseDrawingEnabled = IsKeyModificatorPressed(DVKEY_ALT);
            
            if(UIEvent::PHASE_BEGAN == touch->phase)
            {
                touchID = touch->tid;
                if(isIntersect)
                {
                    prevDrawPos = Vector2(-100, -100);
                    InputAction(touch->phase, isIntersect);
                }
                return true;
            }
            else if(UIEvent::PHASE_DRAG == touch->phase)
            {
                InputAction(touch->phase, isIntersect);
                if(!isIntersect)
                {
                    prevDrawPos = Vector2(-100, -100);
                }
                return true;
            }
            else if(UIEvent::PHASE_ENDED == touch->phase || UIEvent::PHASE_CANCELLED == touch->phase)
            {
                touchID = INVALID_TOUCH_ID;
                
                if(isIntersect)
                {
                    InputAction(touch->phase, isIntersect);
                    prevDrawPos = Vector2(-100, -100);
                }
                return true;
            }
        }
    }

    return false;
}

void LandscapeEditorBase::SaveTexture()
{
    state = ELE_SAVING_TEXTURE;
    
    if(!savedPath.IsEmpty())
    {
        FilePath pathToSave = savedPath;
        pathToSave.ReplaceExtension(".png");
        SaveTextureAs(pathToSave, true);
    }
    else if(!fileSystemDialog->GetParent())
    {
        fileSystemDialog->SetExtensionFilter(String(".png"));
        fileSystemDialog->SetOperationType(UIFileSystemDialog::OPERATION_SAVE);
        
        fileSystemDialog->SetCurrentDir(EditorSettings::Instance()->GetDataSourcePath());
        
        fileSystemDialog->Show(UIScreenManager::Instance()->GetScreen());
        fileSystemDialogOpMode = DIALOG_OPERATION_SAVE;
    }
}

void LandscapeEditorBase::SaveTextureAs(const FilePath &pathToFile, bool closeLE)
{
    SaveTextureAction(pathToFile);
    
    if(closeLE)
    {
        state = ELE_TEXTURE_SAVED;
        Close();
    }
}

LandscapeToolsPanel * LandscapeEditorBase::GetToolPanel()
{
    return toolsPanel;
}


void LandscapeEditorBase::OnToolSelected(LandscapeTool *newTool)
{
    SetTool(newTool);
}

void LandscapeEditorBase::OnShowGrid(bool show)
{
    if(workingLandscape)
    {
// RETURN TO THIS CODE LATER
//        if(show)
//        {
//            workingLandscape->SetDebugFlags(workingLandscape->GetDebugFlags() | DebugRenderComponent::DEBUG_DRAW_GRID);
//        }
//        else 
//        {
//            workingLandscape->SetDebugFlags(workingLandscape->GetDebugFlags() & ~DebugRenderComponent::DEBUG_DRAW_GRID);
//        }
    }
    else 
    {
        DVASSERT(false);
    }
}

void LandscapeEditorBase::ClearSceneResources()
{
	if(IsActive())
	{
		HideAction();
	}
}

void LandscapeEditorBase::OnFileSelected(UIFileSystemDialog *forDialog, const FilePath &pathToFile)
{
    switch (fileSystemDialogOpMode) 
    {
        case DIALOG_OPERATION_SAVE:
        {
            savedPath = pathToFile;
            SaveTextureAs(pathToFile, true);
            break;
        }
            
        default:
            break;
    }
    
    fileSystemDialogOpMode = DIALOG_OPERATION_NONE;
}

void LandscapeEditorBase::OnFileSytemDialogCanceled(UIFileSystemDialog *forDialog)
{
    fileSystemDialogOpMode = DIALOG_OPERATION_NONE;
    
    Close();
}

