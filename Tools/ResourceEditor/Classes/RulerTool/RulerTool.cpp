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





#include "RulerTool.h"

#include "../EditorScene.h"
#include "../SceneEditor/EditorBodyControl.h"

#include "../Qt/Main/QtMainWindowHandler.h"
#include "../Qt/Main/QtUtils.h"
#include "../SceneEditor/HeightmapNode.h"
#include "../LandscapeEditor/RulerToolLandscape.h"
#include "../LandscapeEditor/LandscapesController.h"


#include "../Qt/Scene/SceneData.h"
#include "../Qt/Scene/SceneDataManager.h"

using namespace DAVA;


RulerTool::RulerTool(EditorBodyControl *parent)
    :   BaseObject()
{
    editorScene = NULL;
    heightmapNode = NULL;
    landscapesController = NULL;
    rulerToolLandscape = NULL;
    
    parentControl = parent;
    
    length = 0;
    landscapeSize = 0;
}

RulerTool::~RulerTool()
{
    DisableTool();
}

bool RulerTool::EnableTool(EditorScene *scene)
{
    DisableTool();
    
    length = 0;
    landscapeSize = 0;
    linePoints.clear();
    
    editorScene = SafeRetain(scene);
    if(scene)
    {
		if (FindLandscape(scene))
        {
            SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
            landscapesController = SafeRetain(activeScene->GetLandscapesController());
            rulerToolLandscape = SafeRetain(landscapesController->CreateRulerToolLandscape());
            rulerToolLandscape->SetPoints(linePoints);

            Heightmap *heightmap = rulerToolLandscape->GetHeightmap();
            
            DVASSERT(heightmap && "Need Heightmap at landscape");
            landscapeSize = heightmap->Size();
            
            heightmapNode = new HeightmapNode(editorScene, rulerToolLandscape);
            editorScene->AddNode(heightmapNode);
        }
    }
    
    return (NULL != rulerToolLandscape);
}

void RulerTool::DisableTool()
{
	if(SceneDataManager::Instance())
	{
		SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
		activeScene->ResetLandsacpeSelection();
	}

    if(landscapesController)
    {
        landscapesController->ReleaseRulerToolLandscape();
    }
    
    if(heightmapNode && editorScene)
    {
        editorScene->RemoveNode(heightmapNode);
    }
    
    SafeRelease(landscapesController);
    SafeRelease(rulerToolLandscape);
    SafeRelease(editorScene);
    SafeRelease(heightmapNode);
}

bool RulerTool::Input(DAVA::UIEvent *touch)
{
    if(UIEvent::BUTTON_1 == touch->tid)
    {
        if(UIEvent::PHASE_ENDED == touch->phase)
        {
            Vector3 point;
            bool isIntersect = GetIntersectionPoint(touch->point, point);
            if(isIntersect)
            {
                if(IsKeyModificatorPressed(DVKEY_SHIFT))
                {
                    SetStartPoint(point);
                }
                else
                {
                    AddPoint(point);
                }
                rulerToolLandscape->SetPoints(linePoints);
            }
            return true;
        }
    }
    
    return false;
}

bool RulerTool::GetIntersectionPoint(const DAVA::Vector2 &touchPoint, DAVA::Vector3 &pointOnLandscape)
{
    DVASSERT(parentControl);

    Vector3 from, dir;
    parentControl->GetCursorVectors(&from, &dir, touchPoint);
    Vector3 to = from + dir * (float32)RAY_TRACING_DISTANCE;

    bool isIntersect = editorScene->LandscapeIntersection(from, to, pointOnLandscape);
    return isIntersect;
}

void RulerTool::SetStartPoint(const DAVA::Vector3 &point)
{
    linePoints.clear();
    linePoints.push_back(point);

    length = 0;
    landscapeSize = 0;
	// TODO: mainwindow
    //QtMainWindowHandler::Instance()->ShowStatusBarMessage(Format("Length: %f", length));
}

void RulerTool::AddPoint(const DAVA::Vector3 &point)
{
    if(0 < linePoints.size())
    {
        Vector3 prevPoint = *(linePoints.rbegin());
        length += GetLength(prevPoint, point);
        
        linePoints.push_back(point);
        
		// TODO: mainwindow
        //QtMainWindowHandler::Instance()->ShowStatusBarMessage(Format("Length: %f", length));
    }
}


DAVA::float32 RulerTool::GetLength(const DAVA::Vector3 &startPoint, const DAVA::Vector3 &endPoint)
{
    float32 lineSize = 0.f;
    
    
    Vector3 prevPoint = startPoint;
    Vector3 prevLandscapePoint = LandscapePoint(prevPoint); //
    
    for(int32 i = 1; i <= APPROXIMATION_COUNT; ++i)
    {
        Vector3 point = startPoint + (endPoint - startPoint) * i / (float32)APPROXIMATION_COUNT;
        Vector3 landscapePoint = LandscapePoint(point); //
        
        lineSize += (landscapePoint - prevLandscapePoint).Length();
        
        prevPoint = point;
        prevLandscapePoint = landscapePoint;
    }
    
    return lineSize;
}

DAVA::Vector3 RulerTool::LandscapePoint(const DAVA::Vector3 &point)
{
    Vector3 landscapePoint;
    bool res = rulerToolLandscape->PlacePoint(point, landscapePoint);
    return landscapePoint;
}



