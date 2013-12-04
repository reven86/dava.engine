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



#include "SceneGraph.h"
#include "ControlsFactory.h"

#include "../EditorScene.h"
#include "EditorSettings.h"
#include "PropertyControlCreator.h"

#include "SceneValidator.h"

#include "../Qt/Scene/SceneData.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "../Qt/Main/QtUtils.h"
#include "Scene3D/Components/DebugRenderComponent.h"

#include "../Commands/CommandsManager.h"
#include "../Commands/SceneGraphCommands.h"
#include "../StringConstants.h"

#include "Scene3D/Components/CustomPropertiesComponent.h"

SceneGraph::SceneGraph(GraphBaseDelegate *newDelegate, const Rect &rect)
    :   GraphBase(newDelegate, rect)
    ,   workingNode(NULL)
{
    CreateGraphPanel(rect);
}

SceneGraph::~SceneGraph()
{
}

void SceneGraph::SelectNode(BaseObject *node)
{
    workingNode = dynamic_cast<Entity *>(node);
    UpdatePropertyPanel();
}


void SceneGraph::CreateGraphPanel(const Rect &rect)
{
    GraphBase::CreateGraphPanel(rect);
    
    Rect graphRect = graphPanel->GetRect();
    graphRect.dy -= (ControlsFactory::BUTTON_HEIGHT * 7);
    graphTree = new UIHierarchy(graphRect);
    ControlsFactory::CusomizeListControl(graphTree);
    ControlsFactory::SetScrollbar(graphTree);
    graphTree->SetCellHeight(ControlsFactory::CELL_HEIGHT);
    graphTree->SetDelegate(this);
    graphTree->SetClipContents(true);
    graphPanel->AddControl(graphTree);
    
    int32 leftSideWidth = EditorSettings::Instance()->GetLeftPanelWidth();
    int32 y = graphRect.dy;
    
    UIButton * removeRootNodes = ControlsFactory::CreateButton(Rect(0, y, leftSideWidth,ControlsFactory::BUTTON_HEIGHT), 
                                                                       LocalizedString(L"scenegraph.removerootnode"));
    y += ControlsFactory::BUTTON_HEIGHT;
    
    UIButton * refreshSceneGraphButton = ControlsFactory::CreateButton(Rect(0, y, leftSideWidth,ControlsFactory::BUTTON_HEIGHT), 
                                                                       LocalizedString(L"panel.refresh"));
    y += ControlsFactory::BUTTON_HEIGHT;
    
    UIButton * lookAtButton = ControlsFactory::CreateButton(Rect(0, y, leftSideWidth,ControlsFactory::BUTTON_HEIGHT), 
                                                            LocalizedString(L"scenegraph.lookatobject"));
    y += ControlsFactory::BUTTON_HEIGHT;
    UIButton * removeNodeButton = ControlsFactory::CreateButton(Rect(0, y, leftSideWidth, ControlsFactory::BUTTON_HEIGHT), 
                                                                LocalizedString(L"scenegraph.removeobject"));
    y += ControlsFactory::BUTTON_HEIGHT;
    UIButton * enableDebugFlagsButton = ControlsFactory::CreateButton(Rect(0, y, leftSideWidth, ControlsFactory::BUTTON_HEIGHT), 
                                                                      LocalizedString(L"scenegraph.debugflags"));
    y += ControlsFactory::BUTTON_HEIGHT;
    UIButton * bakeMatrices = ControlsFactory::CreateButton(Rect(0, y, leftSideWidth, ControlsFactory::BUTTON_HEIGHT), 
                                                            LocalizedString(L"scenegraph.bakematrices"));
    
    y += ControlsFactory::BUTTON_HEIGHT;
    UIButton * buildQuadTree = ControlsFactory::CreateButton(Rect(0, y, leftSideWidth, ControlsFactory::BUTTON_HEIGHT), 
                                                            LocalizedString(L"scenegraph.buildquadtree"));
    
    graphPanel->AddControl(removeRootNodes);
    graphPanel->AddControl(refreshSceneGraphButton);
    graphPanel->AddControl(lookAtButton);
    graphPanel->AddControl(removeNodeButton);
    graphPanel->AddControl(enableDebugFlagsButton);
    graphPanel->AddControl(bakeMatrices);
    graphPanel->AddControl(buildQuadTree);
    
    SafeRelease(removeRootNodes);
    SafeRelease(refreshSceneGraphButton);
    SafeRelease(lookAtButton);
    SafeRelease(removeNodeButton);
    SafeRelease(enableDebugFlagsButton);
    SafeRelease(bakeMatrices);
    SafeRelease(buildQuadTree);
}

void SceneGraph::FillCell(UIHierarchyCell *cell, void *node)
{
    //Temporary fix for loading of UI Interface to avoid reloading of texrures to different formates.
    // 1. Reset default format before loading of UI
    // 2. Restore default format after loading of UI from stored settings.
    Texture::SetDefaultGPU(GPU_UNKNOWN);

    Entity *n = (Entity *)node;
    UIStaticText *text =  (UIStaticText *)cell->FindByName("_Text_");
    text->SetText(StringToWString(n->GetName()));
    
    UIControl *icon = cell->FindByName("_Icon_");
    icon->SetSprite("~res:/Gfx/UI/Entity/scenenode", 0);
    
    UIControl *marker = cell->FindByName("_Marker_");
    if(n->GetFlags() & Entity::NODE_INVALID)
    {
        marker->SetSprite("~res:/Gfx/UI/Entity/scenenode_invalid", 0);
        marker->SetVisible(true);
    }
    else 
    {
        marker->SetVisible(false);
    }
    
    
    if(n == workingNode)
    {
        cell->SetSelected(true, false);
    }
    else
    {
        cell->SetSelected(false, false);
    }
    
    Texture::SetDefaultGPU(EditorSettings::Instance()->GetTextureViewGPU());
}

void SceneGraph::SelectHierarchyNode(UIHierarchyNode * node)
{
    Entity * sceneNode = dynamic_cast<Entity*>((BaseObject*)node->GetUserNode());
    if (sceneNode)
    {
        workingNode = sceneNode;
        
        workingScene->SetSelection(0);
        workingScene->SetSelection(workingNode);
        
        UpdatePropertyPanel();
        
        Camera * cam = GetCamera(workingNode);
        if (cam)
        {
            if (IsKeyModificatorPressed(DVKEY_ALT))
            {
                workingScene->SetClipCamera(cam);
            }
            else 
            {
                workingScene->SetCurrentCamera(cam);
            }
        }
    }

}


void SceneGraph::UpdatePropertyPanel()
{
    if(workingNode && (NULL != graphTree->GetParent()))
    {
		RecreatePropertiesPanelForNode(workingNode);
        
        if(propertyControl->GetParent() && propertyControl->GetParent() != propertyPanel)
        {
            propertyControl->GetParent()->RemoveControl(propertyControl);
        }
        
        if(!propertyControl->GetParent())
        {
            propertyPanel->AddControl(propertyControl);
        }
    }
    else
    {
        if(propertyControl && propertyControl->GetParent())
        {
            propertyPanel->RemoveControl(propertyControl);
        }
    }
}

void SceneGraph::RecreatePropertiesPanelForNode(Entity * node)
{
	UIControlSystem::Instance()->SetFocusedControl(0, true);
	if(propertyControl && propertyControl->GetParent())
	{
		propertyPanel->RemoveControl(propertyControl);
	}
	SafeRelease(propertyControl);
    
    DVASSERT(delegate);
    if(delegate->LandscapeEditorActive())
    {
        propertyControl = delegate->GetPropertyControl(propertyPanelRect);
        //PropertyControlCreator::Instance()->CreateControlForLandscapeEditor(node, propertyPanelRect);
    }
    else 
    {
        propertyControl = PropertyControlCreator::Instance()->CreateControlForNode(node, propertyPanelRect, false);
    }
    
    SafeRetain(propertyControl);
	propertyControl->SetDelegate(this);
	propertyControl->SetWorkingScene(workingScene);
    propertyControl->ReadFrom(node);
}


void SceneGraph::OnRemoveNodeButtonPressed(BaseObject *, void *, void *)
{
    SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
    activeScene->RemoveSceneNode(workingNode);
}

void SceneGraph::OnRemoveRootNodesButtonPressed(BaseObject *, void *, void *)
{
    RemoveRootNodes();
}



void SceneGraph::OnBakeMatricesPressed(BaseObject *, void *, void *)
{
    if (workingNode)
    {
        workingNode->BakeTransforms();
    }
// TODO: if node is not selected scene should be selected by default ??? or probably we should show 
//    else
//    {
//        scene->BakeTransforms(); 
//    }
}

void SceneGraph::OnBuildQuadTreePressed(BaseObject *, void *, void *)
{
    if (workingNode)
    {
//        Scene * scene = workingNode->GetScene();
//        QuadTree * quadTree = dynamic_cast<QuadTree*>(scene->GetBVHierarchy());
//        if (!quadTree)
//        {
//            quadTree = new QuadTree();
//            scene->SetBVHierarchy(quadTree);
//        }
//        quadTree->Build(scene);
    }
}

void SceneGraph::OnEnableDebugFlagsPressed(BaseObject *, void *, void *)
{
    if (workingNode)
    {
        if (workingNode->GetDebugFlags() & DebugRenderComponent::DEBUG_DRAW_ALL)
        {
            workingNode->SetDebugFlags(0, true);
        }
        else
        {
            workingNode->SetDebugFlags(DebugRenderComponent::DEBUG_DRAW_ALL, true);
        }
    }
}


void SceneGraph::OnRefreshGraph(BaseObject *, void *, void *)
{
    RefreshGraph();
}

void SceneGraph::RemoveWorkingNode()
{
	if (workingNode)
	{
		CommandsManager::Instance()->ExecuteAndRelease(new CommandInternalRemoveSceneNode(workingNode),
													   workingNode->GetScene());
    }
}

void SceneGraph::RemoveRootNodes()
{
    if (workingNode && workingScene)
    {
        if(workingNode->GetParent() == workingScene) 
        {
            String referenceToOwner;
            
            KeyedArchive *customProperties = workingNode->GetCustomProperties();
            if(customProperties && customProperties->IsKeyExists(ResourceEditor::EDITOR_REFERENCE_TO_OWNER))
            {
                referenceToOwner = customProperties->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER);
            }

            
            Vector<Entity *>nodesForDeletion;
            nodesForDeletion.reserve(workingScene->GetChildrenCount());
            
            for(int32 i = 0; i < workingScene->GetChildrenCount(); ++i)
            {
                Entity *node = workingScene->GetChild(i);

                customProperties = node->GetCustomProperties();
                if(customProperties && customProperties->IsKeyExists(ResourceEditor::EDITOR_REFERENCE_TO_OWNER))
                {
                    if(customProperties->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER) == referenceToOwner)
                    {
                        nodesForDeletion.push_back(SafeRetain(node));
                    }
                }
            }
            
            workingScene->SetSelection(NULL);
            workingNode = NULL;
            for(int32 i = 0; i < (int32)nodesForDeletion.size(); ++i)
            {
                Entity *node = nodesForDeletion[i];
                
                workingScene->ReleaseUserData(node);
                workingScene->RemoveNode(node);
                
                SafeRelease(node);
            }
            nodesForDeletion.clear();
            
            UpdatePropertyPanel();
            graphTree->Refresh();
//            SceneValidator::Instance()->EnumerateSceneTextures();
        }
    }
}



bool SceneGraph::IsNodeExpandable(UIHierarchy *, void *forNode)
{
    if (forNode) 
    {
        Entity *node = (Entity*)forNode;
        if(node->GetSolid())
        {
            return false;
        }
        else
        {
            return node->GetChildrenCount() > 0;
        }
    }
    
    return workingScene->GetChildrenCount() > 0;
}

int32 SceneGraph::ChildrenCount(UIHierarchy *, void *forParent)
{
    if (forParent) 
    {
        Entity *node = (Entity*)forParent;
        if(node->GetSolid())
        {
            return 0;
        }
        else
        {
            return node->GetChildrenCount();
        }
        
    }
    
    if(workingScene)    return workingScene->GetChildrenCount();

    return 0;
}

void * SceneGraph::ChildAtIndex(UIHierarchy *, void *forParent, int32 index)
{
    if (forParent) 
    {
        return ((Entity*)forParent)->GetChild(index);
    }
    
    return workingScene->GetChild(index);
}

void SceneGraph::DragAndDrop(void *who, void *target, int32 mode)
{
    Entity *whoNode = SafeRetain((Entity *)who);
    Entity *targetNode = SafeRetain((Entity *)target);
    
    if(whoNode)
    {
        if(UIHierarchy::DRAG_CHANGE_PARENT == mode)
        {
            // select new parent for dragged node
            Entity *newParent = (targetNode) ? targetNode : workingScene;
            
            //skip unused drag
            if(whoNode->GetParent() != newParent)
            {
                // check correct hierarhy (can't drag to child)
                Entity *nd = newParent->GetParent();
                while(nd && nd != whoNode)
                {
                    nd = nd->GetParent();
                }
                
                if(!nd)
                {
                    //drag
                    //whoNode->GetParent()->RemoveNode(whoNode);
                    newParent->AddNode(whoNode);
                }
            }
        }
        else if(UIHierarchy::DRAG_CHANGE_ORDER == mode)
        {
            if(targetNode && whoNode->GetParent() == targetNode->GetParent())
            {
                //whoNode->GetParent()->RemoveNode(whoNode);
                targetNode->GetParent()->InsertBeforeNode(whoNode, targetNode);
            }
        }

        SelectNode(NULL);
    }
    
    SafeRelease(whoNode);
    SafeRelease(targetNode);
}

void SceneGraph::RefreshGraph()
{
    GraphBase::RefreshGraph();
}

void SceneGraph::SetSize(const Vector2 &newSize)
{
    int32 rightSideWidth = EditorSettings::Instance()->GetRightPanelWidth();
    
    propertyPanelRect = Rect(newSize.x - rightSideWidth, 0, rightSideWidth, newSize.y);
    propertyPanel->SetRect(propertyPanelRect);
    
    propertyPanelRect.x = propertyPanelRect.y = 0;
    
    if(propertyControl)
    {
        propertyControl->SetSize(propertyPanelRect.GetSize());
    }
}

