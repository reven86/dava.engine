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



#include "EntitiesGraph.h"
#include "ControlsFactory.h"
#include "EditorSettings.h"
#include "PropertyControlCreator.h"
#include "Entity/EntityManager.h"
#include "Entity/Component.h"
#include "Entity/Entity.h"

EntitiesGraph::EntitiesGraph(GraphBaseDelegate *newDelegate, const Rect &rect)
:   GraphBase(newDelegate, rect),
	workingEntity(0)
{
	CreateGraphPanel(rect);
}

EntitiesGraph::~EntitiesGraph()
{

}

void EntitiesGraph::CreateGraphPanel(const Rect &rect)
{
	GraphBase::CreateGraphPanel(rect);

	Rect graphRect = graphPanel->GetRect();
	graphTree = new UIHierarchy(graphRect);
	ControlsFactory::CusomizeListControl(graphTree);
	ControlsFactory::SetScrollbar(graphTree);
	graphTree->SetCellHeight(ControlsFactory::CELL_HEIGHT);
	graphTree->SetDelegate(this);
	graphTree->SetClipContents(true);
	graphPanel->AddControl(graphTree);
}

int32 EntitiesGraph::ChildrenCount(UIHierarchy *forHierarchy, void *forParent)
{
    if(workingScene)
        return workingScene->entityManager->GetAllEntities().size();
    
    return 0;
}

void * EntitiesGraph::ChildAtIndex(UIHierarchy *forHierarchy, void *forParent, int32 index)
{
	return workingScene->entityManager->GetAllEntities()[index];
}

void EntitiesGraph::FillCell(UIHierarchyCell *cell, void *node)
{
	Entity *n = (Entity *)node;
	UIStaticText *text =  (UIStaticText *)cell->FindByName("_Text_");
	text->SetText(Format(L"Entity %d", n->GetIndexInFamily()));
}

void EntitiesGraph::SelectNode(BaseObject *node)
{
	if(!node)
	{
		workingEntity = 0;
		RefreshGraph();
	}
}

void EntitiesGraph::SelectHierarchyNode(UIHierarchyNode * node)
{
	Entity * sceneEntity = (Entity*)(node->GetUserNode());
	workingEntity = sceneEntity;
	//workingNode = sceneNode;
	//workingScene->SetSelection(0);
	//workingScene->SetSelection(workingNode);

	UpdatePropertyPanel();
}

void EntitiesGraph::UpdatePropertyPanel()
{
	if(workingEntity)
	{
		RecreatePropertiesPanelForEntity(workingEntity);

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

void EntitiesGraph::RecreatePropertiesPanelForEntity(Entity * entity)
{
	if(propertyControl && propertyControl->GetParent())
	{
		propertyPanel->RemoveControl(propertyControl);
	}
	SafeRelease(propertyControl);

	propertyControl = PropertyControlCreator::Instance()->CreateControlForEntity(entity, propertyPanelRect);

	SafeRetain(propertyControl);
	propertyControl->SetDelegate(this);
	propertyControl->SetWorkingScene(workingScene);
	propertyControl->ReadFrom(entity);
}


