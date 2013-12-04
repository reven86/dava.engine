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



#include "LandscapesController.h"

#include "EditorHeightmap.h"
#include "NotPassableTerrain.h"
#include "LandscapeRenderer.h"
#include "RulerToolLandscape.h"

#include "../Qt/Scene/SceneData.h"
#include "../Qt/Scene/SceneDataManager.h"

using namespace DAVA;

LandscapesController::LandscapesController()
    :   BaseObject()
{
    scene = NULL;
    renderedHeightmap = NULL;
    notPassableTerrain = NULL;
    landscapeRenderer = NULL;
    editorLandscape = NULL;
    rulerToolLandscape = NULL;
    
    currentLandscape = NULL;
    
    savedLandscape = NULL;
    savedHeightmap = NULL;
}

LandscapesController::~LandscapesController()
{
    ReleaseScene();
}

void LandscapesController::SetScene(DAVA::Scene *scene)
{
    ReleaseScene();

    this->scene = SafeRetain(scene);
    
    if(scene)
    {
        EditorScene *editorScene = dynamic_cast<EditorScene *>(scene);
        if(editorScene)
        {
           Landscape *landscape = FindLandscape(editorScene);
            if(landscape)
            {
                SaveLandscape(landscape);
            }
        }
    }
}

void LandscapesController::ReleaseScene()
{
    ReleaseLandscape(notPassableTerrain);
    notPassableTerrain = NULL;
    

    ReleaseLandscape(editorLandscape);
    editorLandscape = NULL;


    ReleaseLandscape(rulerToolLandscape);
    rulerToolLandscape = NULL;

    
    SafeRelease(renderedHeightmap);
    SafeRelease(landscapeRenderer);
    

    if(savedHeightmap && savedLandscape)
    {
        savedLandscape->SetHeightmap(savedHeightmap);
    }
    SafeRelease(savedHeightmap);
    SafeRelease(savedLandscape);
    
    
    SafeRelease(scene);
    
    currentLandscape = NULL;
}

void LandscapesController::ReleaseLandscape(EditorLandscape *landscapeNode)
{
    // RETURN TO THIS CODE LATER
    //    if(landscapeNode && landscapeNode->GetParent())
    //    {
    //        landscapeNode->GetParent()->RemoveNode(landscapeNode);
    //    }
    SafeRelease(landscapeNode);
}


void LandscapesController::SaveLandscape(DAVA::Landscape *landscape)
{
    SafeRelease(savedHeightmap);
    SafeRelease(savedLandscape);
    
    if(landscape)
    {
        savedLandscape = SafeRetain(landscape);
        savedHeightmap = SafeRetain(landscape->GetHeightmap());
    }
    
    currentLandscape = savedLandscape;
}


void LandscapesController::ToggleNotPassableLandscape()
{
    DVASSERT(scene && "Need set scene before");
    
    if(notPassableTerrain)
    {
		SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
		activeScene->ResetLandsacpeSelection();


        bool hidden = HideEditorLandscape(notPassableTerrain);
        if(hidden)
        {
            notPassableTerrain = NULL;
        }
    }
    else
    {
        notPassableTerrain = new NotPassableTerrain();
        bool showed = ShowEditorLandscape(notPassableTerrain);
        if(!showed)
        {
            SafeRelease(notPassableTerrain);
        }
    }
}

bool LandscapesController::ShowEditorLandscape(EditorLandscape *displayingLandscape)
{
	Landscape *landscape = FindLandscape(scene);
	if (!landscape)
    {
        Logger::Error("[LandscapesController::ShowEditorLandscape] Can be only one landscape");
        return false;
    }
	
    displayingLandscape->SetNestedLandscape(landscape);
    
    if(!landscapeRenderer)
    {
        renderedHeightmap = new EditorHeightmap(landscape->GetHeightmap());
        landscapeRenderer = new LandscapeRenderer(renderedHeightmap, landscape->GetBoundingBox());

        displayingLandscape->SetHeightmap(renderedHeightmap);
    }
    displayingLandscape->SetRenderer(landscapeRenderer);
	
	//TODO: remove SetWorldTransformPtr
	displayingLandscape->SetWorldTransformPtr(landscape->GetWorldTransformPtr());
	Entity* lanscapeNode = FindLandscapeEntity(scene);
	
	lanscapeNode->RemoveComponent(Component::RENDER_COMPONENT);
	lanscapeNode->AddComponent(ScopedPtr<RenderComponent> (new RenderComponent(displayingLandscape)));

    currentLandscape = displayingLandscape;
    return true;
}

bool LandscapesController::HideEditorLandscape(EditorLandscape *hiddingLandscape)
{
    hiddingLandscape->FlushChanges();
    
    EditorLandscape *parentLandscape = hiddingLandscape->GetParentLandscape();
    Landscape *nestedLandscape = hiddingLandscape->GetNestedLandscape();
    
    if(parentLandscape)
    {
        Heightmap *hmap = SafeRetain(parentLandscape->GetHeightmap());
        parentLandscape->SetNestedLandscape(nestedLandscape);
        parentLandscape->SetHeightmap(hmap);
        SafeRelease(hmap);

        currentLandscape = parentLandscape;
    }
    else
    {
        EditorLandscape *editorLandscape = dynamic_cast<EditorLandscape *>(nestedLandscape);
        if(editorLandscape)
        {
            editorLandscape->SetParentLandscape(NULL);
        }
		
		Entity* lanscapeNode = FindLandscapeEntity(scene);
		lanscapeNode->RemoveComponent(Component::RENDER_COMPONENT);
		lanscapeNode->AddComponent(ScopedPtr<RenderComponent> (new RenderComponent(nestedLandscape)));
        
        if(NeedToKillRenderer(nestedLandscape))
        {
            SafeRelease(renderedHeightmap);
            SafeRelease(landscapeRenderer);
        }
        
        currentLandscape = nestedLandscape;
    }
    

    SafeRelease(hiddingLandscape);
    return true;
}


bool LandscapesController::NeedToKillRenderer(DAVA::Landscape *landscapeForDetection)
{
	return !(dynamic_cast<EditorLandscape *>(landscapeForDetection));
}


bool LandscapesController::EditorLandscapeIsActive()
{
    return (NULL != notPassableTerrain) || (NULL != landscapeRenderer) || (NULL != renderedHeightmap) || (NULL != rulerToolLandscape);
}

EditorLandscape * LandscapesController::CreateEditorLandscape()
{
    editorLandscape = new EditorLandscape();
    bool showed = ShowEditorLandscape(editorLandscape);
    if(!showed)
    {
        SafeRelease(editorLandscape);
    }
    
    return editorLandscape;
}

void LandscapesController::ReleaseEditorLandscape()
{
    bool hidden = HideEditorLandscape(editorLandscape);
    if(hidden)
    {
        editorLandscape = NULL;
    }
}

DAVA::Landscape * LandscapesController::GetCurrentLandscape()
{
    return currentLandscape;
}

DAVA::Heightmap * LandscapesController::GetCurrentHeightmap()
{
    if(currentLandscape)
    {
        return currentLandscape->GetHeightmap();
    }
    
    return NULL;
}


void LandscapesController::HeghtWasChanged(const DAVA::Rect &changedRect)
{
    landscapeRenderer->RebuildVertexes(changedRect);
    renderedHeightmap->HeghtWasChanged(changedRect);

    EditorLandscape *editorLandscape = dynamic_cast<EditorLandscape *>(currentLandscape);
    if(editorLandscape)
    {
        editorLandscape->HeihghtmapUpdated(changedRect);
    }
}


void LandscapesController::CursorEnable()
{
    if(currentLandscape)
        currentLandscape->CursorEnable();
}

void LandscapesController::CursorDisable()
{
    if(currentLandscape)
        currentLandscape->CursorDisable();
}

RulerToolLandscape *LandscapesController::CreateRulerToolLandscape()
{
    rulerToolLandscape = new RulerToolLandscape();
    bool showed = ShowEditorLandscape(rulerToolLandscape);
    if(!showed)
    {
        SafeRelease(rulerToolLandscape);
    }
    
    return rulerToolLandscape;
}

void LandscapesController::ReleaseRulerToolLandscape()
{
	if(!rulerToolLandscape)
	{
		return;
	}
    bool hidden = HideEditorLandscape(rulerToolLandscape);
    if(hidden)
    {
        rulerToolLandscape = NULL;
    }
}


