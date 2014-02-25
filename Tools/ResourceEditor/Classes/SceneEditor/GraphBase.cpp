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



#include "GraphBase.h"
#include "ControlsFactory.h"

#include "EditorSettings.h"
#include "SceneNodePropertyNames.h"

#include "../Qt/Scene/SceneData.h"
#include "../Qt/Scene/SceneDataManager.h"

GraphBase::GraphBase(GraphBaseDelegate *newDelegate, const Rect &rect)
    :   delegate(newDelegate)
    ,   workingScene(NULL)
{
    CreatePropertyPanel(rect);
}

GraphBase::~GraphBase()
{
    SafeRelease(workingScene);

    SafeRelease(graphPanel);
    SafeRelease(propertyPanel);
    
    SafeRelease(graphTree);
    SafeRelease(propertyControl);
}

void GraphBase::CreateGraphPanel(const Rect &rect)
{
    int32 leftSideWidth = EditorSettings::Instance()->GetLeftPanelWidth();

    Rect leftRect = Rect(0, 0, leftSideWidth, rect.dy);
    graphPanel = ControlsFactory::CreatePanelControl(leftRect);
}

void GraphBase::CreatePropertyPanel(const Rect &rect)
{
    propertyControl = NULL;
    
    int32 rightSideWidth = EditorSettings::Instance()->GetRightPanelWidth(); 
    
    propertyPanelRect = Rect(rect.dx - rightSideWidth, 0, rightSideWidth, rect.dy);
    propertyPanel = ControlsFactory::CreatePanelControl(propertyPanelRect, false);

    propertyPanelRect.x = propertyPanelRect.y = 0;
}

UIControl * GraphBase::GetGraphPanel()
{
    return graphPanel;
}

UIControl * GraphBase::GetPropertyPanel()
{
    return propertyPanel;
}


void GraphBase::SetScene(EditorScene *scene)
{
    SafeRelease(workingScene);
    workingScene = SafeRetain(scene);
}

bool GraphBase::GraphOnScreen()
{
    return (graphPanel->GetParent() != NULL);
}

bool GraphBase::PropertiesOnScreen()
{
    return (propertyPanel->GetParent() != NULL);
}

void GraphBase::UpdateMatricesForCurrentNode()
{
    if(propertyControl)
    {
        propertyControl->UpdateMatricesForCurrentNode();
    }
}

void GraphBase::RefreshGraph()
{
    graphTree->Refresh();
}


UIHierarchyCell * GraphBase::CellForNode(UIHierarchy *forHierarchy, void *node)
{
    UIHierarchyCell *c= forHierarchy->GetReusableCell("Graph cell"); //try to get cell from the reusable cells store
    if(!c)
    { 
        //if cell of requested type isn't find in the store create new cell
        int32 leftSideWidth = EditorSettings::Instance()->GetLeftPanelWidth();
        c = new UIHierarchyCell(Rect(0, 0, leftSideWidth, ControlsFactory::CELL_HEIGHT), "Graph cell");
        
        UIControl *icon = new UIControl(Rect(0, 0, ControlsFactory::CELL_HEIGHT, ControlsFactory::CELL_HEIGHT));
        icon->SetName("_Icon_");
        icon->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_PROPORTIONAL);
        c->text->AddControl(icon);

        UIControl *marker = new UIControl(Rect(0, 0, ControlsFactory::CELL_HEIGHT, ControlsFactory::CELL_HEIGHT));
        marker->SetName("_Marker_");
        marker->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_PROPORTIONAL);
        c->text->AddControl(marker);
        
        UIStaticText *text = new UIStaticText(Rect(ControlsFactory::CELL_HEIGHT, 0, leftSideWidth - ControlsFactory::CELL_HEIGHT, ControlsFactory::CELL_HEIGHT));
        Font *font = ControlsFactory::GetFont12();
        text->SetFont(font);
        text->SetAlign(ALIGN_LEFT|ALIGN_VCENTER);
        text->SetName("_Text_");
		text->SetTextColor(ControlsFactory::GetColorDark());
        c->text->AddControl(text);
    }
    
    FillCell(c, node);
    
    ControlsFactory::CustomizeExpandButton(c->openButton);
    ControlsFactory::CustomizeSceneGraphCell(c);
    
    return c;
}

void GraphBase::OnCellSelected(UIHierarchy *forHierarchy, UIHierarchyCell *selectedCell)
{
    DVASSERT(delegate);
    if(!delegate->LandscapeEditorActive())
    {
        SelectHierarchyNode(selectedCell->GetNode());
        
        //select 
        List<UIControl*> children = forHierarchy->GetVisibleCells();
        for(List<UIControl*>::iterator it = children.begin(); it != children.end(); ++it)
        {
            UIControl *ctrl = (*it);
            ctrl->SetSelected(false, false);
        }
        
        selectedCell->SetSelected(true, false);
    }
}


void GraphBase::NodesPropertyChanged(const String &forKey)
{
	SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();

	// For some properties rebuilding of selected node only is enough. For other ones,
	// the whole Scene Graph refresh is needed.
	if (IsRebuildSelectedNodeEnough(forKey))
	{
		Entity* selectedNode = activeScene->GetSelectedNode();
		activeScene->RebuildSceneGraphNode(selectedNode);
	}
	else
	{
		activeScene->RebuildSceneGraph();
	}
}

bool GraphBase::IsRebuildSelectedNodeEnough(const String& propertyName)
{
	static const char* supportedPropertyNames[] =
	{
		SCENE_NODE_IS_SOLID_PROPERTY_NAME,
		SCENE_NODE_NAME_PROPERTY_NAME,
		SCENE_NODE_IS_VISIBLE_PROPERTY_NAME,
		
		SCENE_NODE_USED_IN_STATIC_LIGHTING_PROPERTY_NAME,
		SCENE_NODE_CAST_SHADOWS_PROPERTY_NAME,
		SCENE_NODE_RECEIVE_SHADOWS_PROPERTY_NAME
	};

	int32 supportedPropertyNamesCount = sizeof(supportedPropertyNames) / sizeof(*supportedPropertyNames);
	for (int32 i = 0; i < supportedPropertyNamesCount; i ++)
	{
		if (propertyName.compare(supportedPropertyNames[i]) == 0)
		{
			return true;
		}
	}
	
	return false;
}