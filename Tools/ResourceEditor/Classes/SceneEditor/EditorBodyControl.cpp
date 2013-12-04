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



#include "EditorBodyControl.h"
#include "ControlsFactory.h"
#include "../BeastProxy.h"
#include "../SceneNodeUserData.h"
#include "PropertyControlCreator.h"
#include "EditorSettings.h"
#include "../config.h"

#include "SceneValidator.h"
#include "../LightmapsPacker.h"

#include "LandscapeEditorColor.h"
#include "LandscapeEditorHeightmap.h"
#include "SceneEditorScreenMain.h"
#include "LandscapeToolsSelection.h"
#include "LandscapeEditorCustomColors.h"
#include "LandscapeEditorVisibilityCheckTool.h"


#include "SceneGraph.h"
#include "DataGraph.h"
//#include "EntitiesGraph.h"

#include "../Qt/Scene/SceneDataManager.h"
#include "../Qt/Scene/SceneData.h"
#include "../Qt/Main/QtUtils.h"
#include "../Qt/Main/QtMainWindowHandler.h"
#include "../RulerTool/RulerTool.h"

#include "../SceneEditor/EditorConfig.h"

#include "../Commands/CommandsManager.h"
#include "../Commands/EditorBodyControlCommands.h"

#include "../StringConstants.h"
#include "../Commands/FileCommands.h"

#include "../CommandLine/CommandLineManager.h"
#include "TexturePacker/CommandLineParser.h"

#include "Scene3D/Components/CustomPropertiesComponent.h"

#define ARROWS_NODE_NAME "editor.arrows-node"
#include "ArrowsNode.h"

EditorBodyControl::EditorBodyControl(const Rect & rect)
    :   UIControl(rect)
	, beastManager(0)
	, originalNode(0)
	, modifiedNode(0)
{
    currentViewportType = ResourceEditor::VIEWPORT_DEFAULT;
    
    scene = NULL;
    
    landscapeRulerTool = NULL;
	
    ControlsFactory::CusomizeBottomLevelControl(this);

    sceneGraph = new SceneGraph(this, rect);
    currentGraph = sceneGraph;

    InitControls();

    scene3dView->SetDebugDraw(true);
    scene3dView->SetInputEnabled(false);
    AddControl(scene3dView);

	CreateModificationPanel();
    CreateLandscapeEditor();

    scene = NULL;
    cameraController = NULL;

	modificationMode = ResourceEditor::MODIFY_NONE;
	landscapeRelative = false;
    
    propertyShowState = EPSS_ONSCREEN;

    AddControl(currentGraph->GetPropertyPanel());
}

void EditorBodyControl::InitControls()
{
    Rect rect = GetRect();

    int32 rightSideWidth = EditorSettings::Instance()->GetRightPanelWidth();
    scene3dView = new UI3DView(Rect(SCENE_OFFSET, SCENE_OFFSET,
                                    rect.dx - 2 * SCENE_OFFSET - rightSideWidth,
                                    rect.dy - 2 * SCENE_OFFSET));
}

EditorBodyControl::~EditorBodyControl()
{
	for_each(poppedEditorEntitiesForSave.begin(), poppedEditorEntitiesForSave.end(), SafeRelease<Entity>);
	poppedEditorEntitiesForSave.clear();

    SafeRelease(landscapeRulerTool);
    
    SafeRelease(sceneGraph);
    currentGraph = NULL;
    
    ReleaseModificationPanel();
    
    
    ReleaseLandscapeEditor();
    
    SafeRelease(scene);
    SafeRelease(cameraController);
  
    SafeRelease(scene3dView);
}

void EditorBodyControl::UpdateModificationPanel(void)
{
	modificationPanel->UpdateCollisionTypes();
}

void EditorBodyControl::SetBrushRadius(uint32 newSize)
{
	if(RulerToolIsActive())
        return;
    
	if(!currentLandscapeEditor || (currentLandscapeEditor != landscapeEditorCustomColors))
	{
		return;
	}

	landscapeEditorCustomColors->SetRadius(newSize);
}

void EditorBodyControl::SetColorIndex(uint32 indexInSet)
{
	if(RulerToolIsActive())
        return;
    
	if(!currentLandscapeEditor || (currentLandscapeEditor != landscapeEditorCustomColors))
	{
		return;
	}

	const Vector<Color> &colorVector = EditorConfig::Instance()->GetColorPropertyValues("LandscapeCustomColors");
	if(colorVector.size() == 0 || colorVector.size() <= indexInSet)
	{
		return;
	}
	landscapeEditorCustomColors->SetColor(colorVector[indexInSet]);
}

void EditorBodyControl::SaveTexture(const FilePath &path)
{
	if(RulerToolIsActive())
        return;
    
	if(!currentLandscapeEditor)
	{
		return;
	}
	
	if(currentLandscapeEditor == landscapeEditorCustomColors)
		landscapeEditorCustomColors->SaveColorLayer(path);
	else if(currentLandscapeEditor == landscapeEditorVisibilityTool)
		landscapeEditorVisibilityTool->SaveColorLayer(path);
}

void EditorBodyControl::CustomColorsLoadTexture(const FilePath &path)
{
	if(RulerToolIsActive())
		return;

	if(!currentLandscapeEditor || currentLandscapeEditor != landscapeEditorCustomColors)
		return;

	landscapeEditorCustomColors->LoadColorLayer(path);
}

FilePath EditorBodyControl::CustomColorsGetCurrentSaveFileName()
{
	if(RulerToolIsActive())
		return FilePath();

	if(!currentLandscapeEditor || currentLandscapeEditor != landscapeEditorCustomColors)
		return FilePath();

	return landscapeEditorCustomColors->GetCurrentSaveFileName();
}



void EditorBodyControl::CreateModificationPanel(void)
{
    modificationPanel = new ModificationsPanel(this, scene3dView->GetRect());
    modificationPanel->SetScene(scene);
    
    AddControl(modificationPanel);
}

void EditorBodyControl::ReleaseModificationPanel()
{
    SafeRelease(modificationPanel);
}


void EditorBodyControl::PlaceOnLandscape()
{
	Entity * selection = scene->GetProxy();
	if (selection && InModificationMode())
	{
        PlaceOnLandscape(selection);
	}
}

void EditorBodyControl::PlaceOnLandscape(Entity *node)
{
	if (node)
	{
		CommandsManager::Instance()->ExecuteAndRelease(new CommandPlaceOnLandscape(node, this), scene);
	}
}


void EditorBodyControl::Input(DAVA::UIEvent *event)
{
    bool inputDone = LandscapeEditorInput(event);
    if(!inputDone)
    {
        inputDone = RulerToolInput(event);
    }
    
    if(!inputDone)
    {
        ProcessKeyboard(event);
        ProcessMouse(event);
    }
    
    UIControl::Input(event);
}

bool EditorBodyControl::LandscapeEditorInput(UIEvent *event)
{
    if(LandscapeEditorActive())
    {
        bool processed = currentLandscapeEditor->Input(event);
        if(!processed)
        {
            cameraController->Input(event);
        }
        return true;
    }
    
    return false;
}

bool EditorBodyControl::RulerToolInput(UIEvent *event)
{
    if(RulerToolIsActive())
    {
        bool processed = landscapeRulerTool->Input(event);
        if(!processed)
        {
            cameraController->Input(event);
        }
        return true;
    }

    return false;
}

bool EditorBodyControl::ProcessKeyboard(UIEvent *event)
{
    if (event->phase == UIEvent::PHASE_KEYCHAR)
    {
        UITextField *tf = dynamic_cast<UITextField *>(UIControlSystem::Instance()->GetFocusedControl());
        if(!tf)
        {
            modificationPanel->Input(event);
            
			if(!IsKeyModificatorsPressed())
			{
				Camera * newCamera = 0;
				switch(event->tid)
				{
				case DVKEY_ESCAPE:
					{
						UIControl *c = UIControlSystem::Instance()->GetFocusedControl();
						if(c == this || c == scene3dView)
						{
                        SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
							activeScene->SelectNode(NULL);
						}

						break;
					}

				case DVKEY_C:
					newCamera = scene->GetCamera(2);
					break;

				case DVKEY_V:
					newCamera = scene->GetCamera(3);
					break;

				case DVKEY_B:
					newCamera = scene->GetCamera(4);
					break;

				case DVKEY_X:
					{
						bool Z = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_Z);
						bool C = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_C);
						if(!Z && !C)
						{
							ProcessIsSolidChanging();
						}

						break;
					}

				default:
					break;
				}

				if (newCamera)
				{
					scene->SetCurrentCamera(newCamera);
					scene->SetClipCamera(scene->GetCamera(0));
				}
			}
            else if(InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_CTRL) && (event->tid == DVKEY_BACKSPACE))
            {
                sceneGraph->RemoveWorkingNode();
                
                SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
                activeScene->SelectNode(NULL);
                activeScene->RebuildSceneGraph();
            }
        }
	}
	
    return true;
}

bool EditorBodyControl::ProcessMouse(UIEvent *event)
{
	Entity * selection = scene->GetProxy();
	//selection with second mouse button
    
	if (event->tid == UIEvent::BUTTON_1)
	{
		if (event->phase == UIEvent::PHASE_BEGAN)
		{
			isDrag = false;
			inTouch = true;
			touchStart = event->point;

			if (selection)
			{
				modifiedNode = selection;
				transformBeforeModification = selection->GetLocalTransform();
			}
		}
		else if (event->phase == UIEvent::PHASE_DRAG)
		{
			if (!isDrag)
			{
				Vector2 d = event->point - touchStart;
				
				if (selection && d.Length() > 5 && InModificationMode())
				{
					ArrowsNode* arrowsNode = GetArrowsNode(false);
					if (arrowsNode && arrowsNode->GetModAxis() != ArrowsNode::AXIS_NONE)
					{
						isDrag = true;
						if (InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_SHIFT))
						{
							originalNode = scene->GetProxy();

							//create temporary node to calculate transform
							modifiedNode = originalNode->Clone();
							originalNode->GetParent()->AddNode(modifiedNode);
							SelectNode(modifiedNode);
							selection = modifiedNode;

							//store original transform
							transformBeforeModification = modifiedNode->GetLocalTransform();
						}

						if (selection)
						{
							scene->SetBulletUpdate(selection, false);

							inTouch = true;
							touchStart = event->point;
						
							startTransform = selection->GetLocalTransform();
						
							InitMoving(event->point);
						
							translate1.CreateTranslation(-rotationCenter);
							translate2.CreateTranslation(rotationCenter);
						
							//calculate koefficient for moving
							Camera * cam = scene->GetCurrentCamera();
							const Vector3 & camPos = cam->GetPosition();
							const Matrix4 & wt = selection->GetWorldTransform();
							Vector3 objPos = Vector3(0,0,0) * wt;
							Vector3 dir = objPos - camPos;
							moveKf = (dir.Length() - cam->GetZNear()) * 0.003;
						}
					}
				}
			}
			else
			{
				if (selection && InModificationMode())
				{
                    Landscape *landscape = dynamic_cast<Landscape *>(selection);
                    if(!landscape)
                    {
                        PrepareModMatrix(event->point);

						if (IsLandscapeRelative())
						{
							currTransform = currTransform * GetLandscapeOffset(currTransform);
						}

						selection->SetLocalTransform(currTransform);

                        if(currentGraph)
                        {
                            currentGraph->UpdateMatricesForCurrentNode();
                        }
                    }
				}
			}
		}
		else if (event->phase == UIEvent::PHASE_ENDED)
		{
			inTouch = false;
			if (isDrag)
			{
				// originalNode should be non-zero only when clone node
				if (originalNode)
				{
					// Get final transform from temporary node
					Matrix4 transform = modifiedNode->GetLocalTransform();

					// Remove temporary node
					RemoveSelectedSGNode();
					SafeRelease(modifiedNode);
					
					CommandCloneAndTransform* cmd = new CommandCloneAndTransform(originalNode,
																				 transform,
																				 this,
																				 scene->collisionWorld);
					CommandsManager::Instance()->ExecuteAndRelease(cmd, scene);
					originalNode = NULL;

					// update selection to newly created node
					selection = scene->GetProxy();
				}
				else
				{
					CommandsManager::Instance()->ExecuteAndRelease(new CommandTransformObject(modifiedNode,
																							  transformBeforeModification,
																							  modifiedNode->GetLocalTransform()),
																   scene);
				}

				if (selection)
				{
					scene->SetBulletUpdate(selection, true);
				}
			}
			else
			{
				Vector3 from, dir;
				GetCursorVectors(&from, &dir, event->point);
				Vector3 to = from + dir * 1000.0f;
				scene->TrySelection(from, to);
				selection = scene->GetProxy();
				SelectNodeAtTree(selection);
			}
		}
	}
	else
	{
		cameraController->SetSelection(selection);
        
        if (event->phase == UIEvent::PHASE_KEYCHAR)
        {
            UITextField *tf = dynamic_cast<UITextField *>(UIControlSystem::Instance()->GetFocusedControl());
            if(!tf)
            {
                cameraController->Input(event);
            }
        }
        else
        {
            cameraController->Input(event);
        }
	}

	ArrowsNode* arrowsNode = GetArrowsNode(false);
	if (arrowsNode && arrowsNode->GetVisible() && !inTouch && event->phase != UIEvent::PHASE_KEYCHAR)
	{
		Vector3 from, dir;
		GetCursorVectors(&from, &dir, event->point);
		Vector3 to = from + dir * 1000.0f;
		arrowsNode->ProcessMouse(event, from, dir);
	}
	
    return true;
}



void EditorBodyControl::InitMoving(const Vector2 & point)
{
	//init planeNormal
	ArrowsNode* arrowsNode = GetArrowsNode(false);
	if (!arrowsNode) return;
	switch (arrowsNode->GetModAxis())
	{
		case ArrowsNode::AXIS_X:
		case ArrowsNode::AXIS_Y:
		case ArrowsNode::AXIS_XY:
			planeNormal = Vector3(0,0,1);
			break;
		case ArrowsNode::AXIS_Z:
		case ArrowsNode::AXIS_YZ:
			planeNormal = Vector3(1,0,0);
			break;
		case ArrowsNode::AXIS_XZ:
			planeNormal = Vector3(0,1,0);
			break;
		default:
			break;
	}

	Vector3 from, dir;
	GetCursorVectors(&from, &dir, point);
    GetIntersectionVectorWithPlane(from, dir, planeNormal, rotationCenter, startDragPoint);
}	

void EditorBodyControl::GetCursorVectors(Vector3 * from, Vector3 * dir, const Vector2 &point)
{
	Camera * cam = scene->GetCurrentCamera();
	if (cam)
	{
		const Rect & rect = scene3dView->GetLastViewportRect();
		*from = cam->GetPosition();
		Vector3 to = cam->UnProject(point.x, point.y, 0, rect);
		to -= *from;
		*dir = to;
	}
}

static Vector3 vect[3] = {Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1)};

void EditorBodyControl::PrepareModMatrix(const Vector2 & point)
{
	float32 winx = point.x - touchStart.x;
	float32 winy = point.y - touchStart.y;
	
	Matrix4 modification;
	modification.Identity();

	ArrowsNode* arrowsNode = GetArrowsNode(false);
	if (!arrowsNode)
		return;

	if (GetModificationMode() == ResourceEditor::MODIFY_MOVE)
	{
		Vector3 from, dir;
		GetCursorVectors(&from, &dir, point);
		
		Vector3 currPoint;
		bool result = GetIntersectionVectorWithPlane(from, dir, planeNormal, rotationCenter, currPoint);
		
		if (result)
		{
			
			if (arrowsNode)
			{
				switch (arrowsNode->GetModAxis())
				{
					case ArrowsNode::AXIS_X:
						currPoint.y = startDragPoint.y;
						currPoint.z = startDragPoint.z;
						break;
					case ArrowsNode::AXIS_Y:
						currPoint.x = startDragPoint.x;
						currPoint.z = startDragPoint.z;
						break;
					case ArrowsNode::AXIS_Z:
						currPoint.x = startDragPoint.x;
						currPoint.y = startDragPoint.y;
						break;
                    
					default:
						break;
				}
				modification.CreateTranslation(currPoint - startDragPoint);
			}
		}
	}
	else if (GetModificationMode() == ResourceEditor::MODIFY_ROTATE)
	{
		Matrix4 d;
		switch (arrowsNode->GetModAxis())
		{
			case ArrowsNode::AXIS_X:
			case ArrowsNode::AXIS_Y:
				modification.CreateRotation(vect[arrowsNode->GetModAxis()], winy / 100.0f);
				break;
			case ArrowsNode::AXIS_Z:
				modification.CreateRotation(vect[arrowsNode->GetModAxis()], winx / 100.0f);
				break;
			case ArrowsNode::AXIS_XY:
				modification.CreateRotation(vect[ArrowsNode::AXIS_X], winx / 100.0f);
				d.CreateRotation(vect[ArrowsNode::AXIS_Y], winy / 100.0f);
				modification *= d;
				break;
			case ArrowsNode::AXIS_YZ:
				modification.CreateRotation(vect[ArrowsNode::AXIS_Y], winx / 100.0f);
				d.CreateRotation(vect[ArrowsNode::AXIS_Z], winy / 100.0f);
				modification *= d;
				break;
			case ArrowsNode::AXIS_XZ:
				modification.CreateRotation(vect[ArrowsNode::AXIS_X], winx / 100.0f);
				d.CreateRotation(vect[ArrowsNode::AXIS_Z], winy / 100.0f);
				modification *= d;
				break;
			default:
				break;
		}
		modification = (translate1 * modification) * translate2;
		
	}
	else if (GetModificationMode() == ResourceEditor::MODIFY_SCALE)
	{
		float kf = winx / 100.0f;
		if (kf < -1.0)
			kf = - kf - 2.0;
		modification.CreateScale(Vector3(1,1,1) + Vector3(1,1,1) * kf);
		modification = (translate1 * modification) * translate2;
	}
	currTransform = startTransform * modification;
}


void EditorBodyControl::DrawAfterChilds(const UIGeometricData &geometricData)
{
	UIControl::DrawAfterChilds(geometricData);
}

void EditorBodyControl::Update(float32 timeElapsed)
{
	Entity * selection = scene->GetProxy();
	if (selection)
	{
		rotationCenter = selection->GetWorldTransform().GetTranslationVector();

		ArrowsNode* arrowsNode = GetArrowsNode(true);
		if (arrowsNode)
			UpdateArrowsNode(selection);
	}
	else
	{
		ArrowsNode* arrowsNode = GetArrowsNode(false);
		if (arrowsNode)
		{
			arrowsNode->SetVisible(false);
			SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
			activeScene->RemoveSceneNode(arrowsNode);
		}
	}
	
    if(cameraController)
    {
        cameraController->Update(timeElapsed);
    }
    
    if(currentLandscapeEditor)
    {
        currentLandscapeEditor->Update(timeElapsed);
    }
    
    
    UIControl::Update(timeElapsed);
}


void EditorBodyControl::WillAppear()
{
    cameraController->SetSpeed(EditorSettings::Instance()->GetCameraSpeed());
    
    sceneGraph->SelectNode(NULL);
}




void EditorBodyControl::BeastProcessScene()
{
	BeastProxy::Instance()->SafeDeleteManager(&beastManager);
	beastManager = BeastProxy::Instance()->CreateManager();

	//String path = GetFilePath();
	//if(String::npos == path.find("DataSource"))
	//{
	//	return;
	//}

	FilePath path(EditorSettings::Instance()->GetProjectPath()+"DataSource/lightmaps_temp/");
	FileSystem::Instance()->CreateDirectory(path, false);

	BeastProxy::Instance()->SetLightmapsDirectory(beastManager, path);
	BeastProxy::Instance()->Run(beastManager, scene);
}

EditorScene * EditorBodyControl::GetScene()
{
    return scene;
}

void EditorBodyControl::AddNode(Entity *node)
{
    SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
    activeScene->AddSceneNode(node);
}

Entity * EditorBodyControl::GetSelectedSGNode()
{
    return scene->GetSelection();
}

void EditorBodyControl::RemoveSelectedSGNode()
{
    SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
    activeScene->RemoveSceneNode(GetSelectedSGNode());
}


void EditorBodyControl::Refresh()
{
    sceneGraph->RefreshGraph();
}


void EditorBodyControl::SelectNodeAtTree(DAVA::Entity *node)
{
    SceneData *sceneData = SceneDataManager::Instance()->SceneGetActive();
    sceneData->SelectNode(node);
}

void EditorBodyControl::SetViewportSize(ResourceEditor::eViewportType viewportType)
{
    if(currentViewportType == viewportType)
        return;
    
    currentViewportType = viewportType;

    Rect fullRect = GetRect();
    Rect viewRect = Rect(SCENE_OFFSET, SCENE_OFFSET, fullRect.dx - 2 * SCENE_OFFSET, fullRect.dy - 2 * SCENE_OFFSET);
    
    viewRect.dx -= EditorSettings::Instance()->GetRightPanelWidth();
    
    Rect newRect = viewRect;
    switch (viewportType)
    {
        case ResourceEditor::VIEWPORT_IPHONE:
            newRect = Rect(SCENE_OFFSET, SCENE_OFFSET, 480, 320);
            break;

        case ResourceEditor::VIEWPORT_RETINA:
            newRect = Rect(SCENE_OFFSET, SCENE_OFFSET, 960, 640);
            break;

        case ResourceEditor::VIEWPORT_IPAD:
            newRect = Rect(SCENE_OFFSET, SCENE_OFFSET, 1024, 768);
            break;

        default:
            break;
    }
    
    if((newRect.dx <= viewRect.dx) && (newRect.dy <= viewRect.dy))
    {
        viewRect = newRect;
    }
    else
    {
        currentViewportType = ResourceEditor::VIEWPORT_DEFAULT;
    }
    
    scene3dView->SetRect(viewRect);
}

bool EditorBodyControl::ControlsAreLocked()
{
    return (ResourceEditor::VIEWPORT_DEFAULT != currentViewportType);
}

void EditorBodyControl::PackLightmaps()
{
	SceneData *sceneData = SceneDataManager::Instance()->SceneGetActive();

	FilePath inputDir(EditorSettings::Instance()->GetProjectPath()+"DataSource/lightmaps_temp/");

 	FilePath outputDir = FilePath::CreateWithNewExtension(sceneData->GetScenePathname(),  + ".sc2_lightmaps/");

	FileSystem::Instance()->MoveFile(inputDir+"landscape.png", "test_landscape.png", true);

	LightmapsPacker packer;
	packer.SetInputDir(inputDir);

	packer.SetOutputDir(outputDir);
	packer.PackLightmaps(EditorSettings::Instance()->GetTextureViewGPU());
	packer.CreateDescriptors();
	packer.ParseSpriteDescriptors();

	BeastProxy::Instance()->UpdateAtlas(beastManager, packer.GetAtlasingData());

	FileSystem::Instance()->MoveFile("test_landscape.png", outputDir+"landscape.png", true);
}

void EditorBodyControl::Draw(const UIGeometricData &geometricData)
{
    if(LandscapeEditorActive())
    {
        currentLandscapeEditor->Draw(geometricData);
    }
    
    UIControl::Draw(geometricData);
}

void EditorBodyControl::RecreteFullTilingTexture()
{
    Landscape *landscape = FindLandscape(scene);
    if (landscape)
    {
        landscape->UpdateFullTiledTexture();
    }
}

void EditorBodyControl::CreateLandscapeEditor()
{
    int32 leftSideWidth = EditorSettings::Instance()->GetLeftPanelWidth();
    int32 rightSideWidth = EditorSettings::Instance()->GetRightPanelWidth();
    Rect toolsRect(leftSideWidth, 0, GetRect().dx - (leftSideWidth + rightSideWidth), ControlsFactory::TOOLS_HEIGHT);
    landscapeEditorColor = new LandscapeEditorColor(this, this, toolsRect);
    
    toolsRect.dy += ControlsFactory::TOOLS_HEIGHT;
    landscapeEditorHeightmap = new LandscapeEditorHeightmap(this, this, toolsRect);

    landscapeEditorCustomColors = new LandscapeEditorCustomColors(this, this, toolsRect);
	landscapeEditorVisibilityTool = new LandscapeEditorVisibilityCheckTool(this, this, toolsRect);
    
    Rect rect = GetRect();
    landscapeToolsSelection = new LandscapeToolsSelection(NULL, 
                                                          Rect(leftSideWidth, rect.dy - ControlsFactory::OUTPUT_PANEL_HEIGHT, 
                                                               rect.dx - leftSideWidth - rightSideWidth, 
                                                               ControlsFactory::OUTPUT_PANEL_HEIGHT));
    landscapeToolsSelection->SetBodyControl(this);
    
    currentLandscapeEditor = NULL;
}

void EditorBodyControl::ReleaseLandscapeEditor()
{
    currentLandscapeEditor = NULL;
    SafeRelease(landscapeEditorColor);
    SafeRelease(landscapeEditorHeightmap);
    SafeRelease(landscapeToolsSelection);
	SafeRelease(landscapeEditorCustomColors);
	SafeRelease(landscapeEditorVisibilityTool);
}

bool EditorBodyControl::ToggleLandscapeEditor(int32 landscapeEditorMode)
{
    if(RulerToolIsActive())
        return false;

    LandscapeEditorBase *requestedEditor = GetLandscapeEditor(landscapeEditorMode);

    if(currentLandscapeEditor && (currentLandscapeEditor != requestedEditor))
        return false;
    
    if(currentLandscapeEditor == requestedEditor)
    {
        currentLandscapeEditor->Toggle();
    }
    else
    {
        currentLandscapeEditor = requestedEditor;

        bool ret = currentLandscapeEditor->SetScene(scene);
        if(ret)
        {
            currentLandscapeEditor->GetToolPanel()->SetSelectionPanel(landscapeToolsSelection);
            currentLandscapeEditor->Toggle();
        }
        else
        {
            currentLandscapeEditor = NULL;
            return false;
        }
    }
    return true;
}

LandscapeEditorBase* EditorBodyControl::GetLandscapeEditor(int32 landscapeEditorMode)
{
	LandscapeEditorBase* editor = NULL;
	switch ((SceneEditorScreenMain::eLandscapeEditorModeIDS)landscapeEditorMode)
	{
		case SceneEditorScreenMain::ELEMID_COLOR_MAP:
			editor = landscapeEditorColor;
			break;

		case SceneEditorScreenMain::ELEMID_CUSTOM_COLORS:
			editor = landscapeEditorCustomColors;
			break;

		case SceneEditorScreenMain::ELEMID_HEIGHTMAP:
			editor = landscapeEditorHeightmap;
			break;

		case SceneEditorScreenMain::ELEMID_VISIBILITY_CHECK_TOOL:
			editor = landscapeEditorVisibilityTool;
			break;

		default:
			editor = NULL;
			break;
	}

	return editor;
}

LandscapeEditorBase* EditorBodyControl::GetCurrentLandscapeEditor()
{
	return currentLandscapeEditor;
}

void EditorBodyControl::LandscapeEditorStarted()
{
    RemoveControl(modificationPanel);
    savedModificatioMode = modificationPanel->IsModificationMode();
    
    UIControl *toolsPanel = currentLandscapeEditor->GetToolPanel();
    if(!toolsPanel->GetParent())
    {
        AddControl(toolsPanel);
    }
    
	Entity* sceneNode = FindLandscapeEntity(scene);
	if (sceneNode)
	{
		scene->SetSelection(sceneNode);
		SelectNodeAtTree(NULL);
		SelectNodeAtTree(sceneNode);
	}
    landscapeToolsSelection->Show();

	ArrowsNode* arrowsNode = GetArrowsNode(false);
	if (arrowsNode)
		arrowsNode->SetVisible(false);
}

void EditorBodyControl::LandscapeEditorFinished()
{
    landscapeToolsSelection->Close();

    UIControl *toolsPanel = currentLandscapeEditor->GetToolPanel();
    RemoveControl(toolsPanel);

    modificationPanel->IsModificationMode(savedModificatioMode);
    AddControl(modificationPanel);
    
    scene->SetSelection(NULL);
    SelectNodeAtTree(NULL);
    
    currentLandscapeEditor = NULL;
}


void EditorBodyControl::OnPlaceOnLandscape()
{
    PlaceOnLandscape();
}

bool EditorBodyControl::LandscapeEditorActive()
{
    return (currentLandscapeEditor && currentLandscapeEditor->IsActive());
}

bool EditorBodyControl::TileMaskEditorEnabled()
{
    return LandscapeEditorActive() && (currentLandscapeEditor == landscapeEditorColor);
}

NodesPropertyControl *EditorBodyControl::GetPropertyControl(const Rect &rect)
{
    return currentLandscapeEditor->GetPropertyControl(rect);
}

void EditorBodyControl::SetScene(EditorScene *newScene)
{
	if(landscapeRulerTool)
    {
        landscapeRulerTool->DisableTool();
        SafeRelease(landscapeRulerTool);
    }

	if(landscapeEditorColor)
	{
		if(ColorIsActive())
		{
			ToggleLandscapeEditor(SceneEditorScreenMain::ELEMID_COLOR_MAP);
		}
		landscapeEditorColor->ClearSceneResources();
	}
	if(landscapeEditorHeightmap)
	{
		if(HightMapIsActive())
		{
			ToggleLandscapeEditor(SceneEditorScreenMain::ELEMID_HEIGHTMAP);
		}
		landscapeEditorHeightmap->ClearSceneResources();
	}
	if(landscapeEditorCustomColors)
	{
		if(CustomColorIsActive())
		{
			ToggleLandscapeEditor(SceneEditorScreenMain::ELEMID_CUSTOM_COLORS);
		}
		landscapeEditorCustomColors->ClearSceneResources();
	}
	if(landscapeEditorVisibilityTool)
	{
		if(VisibilityToolIsActive())
		{
			ToggleLandscapeEditor(SceneEditorScreenMain::ELEMID_VISIBILITY_CHECK_TOOL);
		}
		landscapeEditorVisibilityTool->ClearSceneResources();
	}

    
    SafeRelease(scene);
    scene = SafeRetain(newScene);
    
    scene3dView->SetScene(scene);
	sceneGraph->SetScene(scene);
    
    modificationPanel->SetScene(scene);
}

void EditorBodyControl::SetCameraController(CameraController *newCameraController)
{
    SafeRelease(cameraController);
    cameraController = SafeRetain(newCameraController);
}

void EditorBodyControl::SelectNodeQt(DAVA::Entity *node)
{
    sceneGraph->SelectNode(node);
}

void EditorBodyControl::OnReloadRootNodesQt()
{
    modificationPanel->OnReloadScene();
}

void EditorBodyControl::SetSize(const Vector2 &newSize)
{
    UIControl::SetSize(newSize);

    int32 rightSideWidth = EditorSettings::Instance()->GetRightPanelWidth();
    Vector2 viewSize = newSize - Vector2(2 * SCENE_OFFSET + rightSideWidth, 2 * SCENE_OFFSET);
    
    viewSize.dx = Max(1.f, viewSize.dx);
    viewSize.dy = Max(1.f, viewSize.dy);
    
    scene3dView->SetSize(viewSize);
    
    sceneGraph->SetSize(newSize);
}



void EditorBodyControl::ProcessIsSolidChanging()
{
    Entity *selectedNode = scene->GetSelection();
    if(selectedNode)
    {
        KeyedArchive *customProperties = selectedNode->GetCustomProperties();
        if(customProperties && customProperties->IsKeyExists(String(Entity::SCENE_NODE_IS_SOLID_PROPERTY_NAME)))
        {
            bool isSolid = selectedNode->GetSolid();
            selectedNode->SetSolid(!isSolid);
            
            SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
            activeScene->RebuildSceneGraphNode(selectedNode);
            
			/* #### dock -->
            KeyedArchive *properties = selectedNode->GetCustomProperties();
            if(properties && properties->IsKeyExists(String("editor.referenceToOwner")))
            {
                String filePathname = properties->GetString(String("editor.referenceToOwner"));
                activeScene->OpenLibraryForFile(filePathname);
            }
			<-- */
            
            sceneGraph->SelectNode(selectedNode);
        }
    }
}


bool EditorBodyControl::RulerToolTriggered()
{
    if(landscapeRulerTool)
    {
        landscapeRulerTool->DisableTool();
        SafeRelease(landscapeRulerTool);
    }
    else
    {
        if(!LandscapeEditorActive())
        {
            landscapeRulerTool = new RulerTool(this);
            bool enabled = landscapeRulerTool->EnableTool(scene);
            if(!enabled)
            {
                SafeRelease(landscapeRulerTool);
                return false;
            }
        }
    }
    
    return true;
}

bool EditorBodyControl::RulerToolIsActive()
{
    return (NULL != landscapeRulerTool);
}

bool EditorBodyControl::CustomColorIsActive()
{
	if (NULL != landscapeEditorCustomColors)
	{
		return landscapeEditorCustomColors->IsActive();
	}

	return false;
}

bool EditorBodyControl::VisibilityToolIsActive()
{
	if (NULL != landscapeEditorVisibilityTool)
	{
		return landscapeEditorVisibilityTool->IsActive();
	}

	return false;
}

bool EditorBodyControl::ColorIsActive()
{
	if (NULL != landscapeEditorColor)
	{
		return landscapeEditorColor->IsActive();
	}

	return false;
}

bool EditorBodyControl::HightMapIsActive()
{
	if (NULL != landscapeEditorHeightmap)
	{
		return landscapeEditorHeightmap->IsActive();
	}

	return false;
}

void EditorBodyControl::VisibilityToolSetPoint()
{
	landscapeEditorVisibilityTool->SetState(VCT_STATE_SET_POINT);
}

void EditorBodyControl::VisibilityToolSetArea()
{
	landscapeEditorVisibilityTool->SetState(VCT_STATE_SET_AREA);
}

void EditorBodyControl::VisibilityToolSetAreaSize(uint32 size)
{
	landscapeEditorVisibilityTool->SetVisibilityAreaSize(size);
}

void EditorBodyControl::RemoveNode(Entity *node)
{
	scene->RemoveNode(node);
}

void EditorBodyControl::SelectNode(Entity *node)
{
	scene->SetSelection(node);
	SelectNodeAtTree(node);
}

ArrowsNode* EditorBodyControl::GetArrowsNode(bool createIfNotExist)
{
	DVASSERT(scene);

	ArrowsNode* arrowsNode = dynamic_cast<ArrowsNode*>(scene->FindByName(ResourceEditor::EDITOR_ARROWS_NODE));
	if (!arrowsNode && createIfNotExist)
	{
		arrowsNode = new ArrowsNode();
        arrowsNode->SetName(ResourceEditor::EDITOR_ARROWS_NODE);

        EditorScene *scene = SceneDataManager::Instance()->SceneGetActive()->GetScene();
        scene->AddEditorEntity(arrowsNode);

		arrowsNode->Release();
	}

	return arrowsNode;
}

void EditorBodyControl::UpdateArrowsNode(Entity* node)
{
	ArrowsNode* arrowsNode = GetArrowsNode(false);
	if (node && arrowsNode)
	{
		if (node == arrowsNode)
		{
			arrowsNode->SetVisible(false);
			return;
		}

		Matrix4 nodeWT = node->GetWorldTransform();
		Matrix4 arrowsNodeTransform;
		arrowsNodeTransform.CreateTranslation(nodeWT.GetTranslationVector());
		arrowsNode->SetLocalTransform(arrowsNodeTransform);
		arrowsNode->SetVisible(true);
		arrowsNode->SetActive(InModificationMode());
	}
}

bool EditorBodyControl::InModificationMode()
{
	return GetModificationMode() != ResourceEditor::MODIFY_NONE;
}

ResourceEditor::eModificationActions EditorBodyControl::GetModificationMode()
{
	return modificationMode;
}

void EditorBodyControl::SetModificationMode(ResourceEditor::eModificationActions mode)
{
	modificationMode = mode;

	ArrowsNode* arrowsNode = GetArrowsNode(false);
	if (arrowsNode)
	{
		arrowsNode->SetActive(InModificationMode());
	}
}

bool EditorBodyControl::IsLandscapeRelative()
{
	return landscapeRelative;
}

void EditorBodyControl::SetLandscapeRelative(bool isLandscapeRelative)
{
	landscapeRelative = isLandscapeRelative;
}

void EditorBodyControl::RestoreOriginalTransform()
{
	if (!InModificationMode())
		return;

	Entity* selection = scene->GetProxy();
	CommandsManager::Instance()->ExecuteAndRelease(new CommandRestoreOriginalTransform(selection), scene);
}

void EditorBodyControl::ApplyTransform(float32 x, float32 y, float32 z)
{
	if (!InModificationMode())
		return;

    Entity *selectedNode = scene->GetProxy();
    if(selectedNode)
	{
		Matrix4 modification;
		modification.Identity();

		Matrix4 t1, t2;
		t1.CreateTranslation(-selectedNode->GetWorldTransform().GetTranslationVector());
		t2.CreateTranslation(selectedNode->GetWorldTransform().GetTranslationVector());

		switch (GetModificationMode())
		{
			case ResourceEditor::MODIFY_MOVE:
				modification.CreateTranslation(Vector3(x, y, z));
				break;

			case ResourceEditor::MODIFY_ROTATE:
				modification.CreateRotation(Vector3(1, 0, 0), DegToRad(x));
				modification *= Matrix4::MakeRotation(Vector3(0, 1, 0), DegToRad(y));
				modification *= Matrix4::MakeRotation(Vector3(0, 0, 1), DegToRad(z));

				modification = (t1 * modification) * t2;
				break;

			case ResourceEditor::MODIFY_SCALE:
				modification.CreateScale(Vector3(1, 1, 1) + Vector3(x + y + z, x + y + z, x + y + z) / 100.f);
				modification = (t1 * modification) * t2;
				break;

			default:
				break;
		}

		Matrix4 originalTransform = selectedNode->GetLocalTransform();
		modification = originalTransform * modification;

		if (IsLandscapeRelative())
		{
			modification = modification * GetLandscapeOffset(modification);
		}

		CommandsManager::Instance()->ExecuteAndRelease(new CommandTransformObject(selectedNode,
																				  originalTransform,
																				  modification),
													   scene);
	}
}

Matrix4 EditorBodyControl::GetLandscapeOffset(const Matrix4& transform)
{
	Matrix4 resTransform;
	resTransform.Identity();

	Landscape* landscape = FindLandscape(scene);
	if(!landscape) return resTransform;

	Vector3 p = Vector3(0, 0, 0) * transform;

	Vector3 result;
	bool res = landscape->PlacePoint(p, result);
	if (res)
	{
		Vector3 offset = result - p;
		resTransform.CreateTranslation(offset);
	}

	return resTransform;
}
