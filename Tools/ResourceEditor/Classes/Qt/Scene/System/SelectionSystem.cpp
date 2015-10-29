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


#include "Scene/System/SelectionSystem.h"
#include "Scene/System/CameraSystem.h"
#include "Scene/System/CollisionSystem.h"
#include "Scene/System/HoodSystem.h"
#include "Scene/System/ModifSystem.h"
#include "Scene/SceneSignals.h"
#include "Settings/SettingsManager.h"

#include <QApplication>
#include "Scene/SceneEditor2.h"

ENUM_DECLARE(SelectionSystemDrawMode)
{
	ENUM_ADD(SS_DRAW_SHAPE);
	ENUM_ADD(SS_DRAW_CORNERS);
	ENUM_ADD(SS_DRAW_BOX);
	ENUM_ADD(SS_DRAW_NO_DEEP_TEST);
}

SceneSelectionSystem::SceneSelectionSystem(DAVA::Scene * scene, SceneCollisionSystem *collSys, HoodSystem *hoodSys)
	: DAVA::SceneSystem(scene)
	, selectionAllowed(true)
	, componentMaskForSelection(ALL_COMPONENTS_MASK)
	, applyOnPhaseEnd(false)
	, invalidSelectionBoxes(false)
	, collisionSystem(collSys)
	, hoodSystem(hoodSys)
	, selectionHasChanges(false)
	, curPivotPoint(ST_PIVOT_COMMON_CENTER)
{
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::SWITCH_CHANGED);
}

SceneSelectionSystem::~SceneSelectionSystem()
{
	if(GetScene())
	{
		GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, EventSystem::SWITCH_CHANGED);
	}
}

void SceneSelectionSystem::ImmediateEvent(DAVA::Entity * entity, DAVA::uint32 event)
{
    if(EventSystem::SWITCH_CHANGED == event)
    {
        for(DAVA::uint32 i = 0; i < curSelections.Size(); i++)
        {
            DAVA::Entity *selectedEntity = curSelections.GetEntity(i);

            // if switched entity selected - update it bounding box
            if(selectedEntity == entity)
            {
                invalidSelectionBoxes = true;
            }
        }
    }
}

void SceneSelectionSystem::Process(DAVA::float32 timeElapsed)
{
	ForceEmitSignals();

	if (IsLocked())
	{
		return;
	}

    // if boxes are invalid we should request them from collision system
    // and store them in selection entityGroup
    if(invalidSelectionBoxes)
    {
        for(DAVA::uint32 i = 0; i < curSelections.Size(); i++)
        {
            EntityGroupItem *item = curSelections.GetItem(i);
            item->bbox = GetSelectionAABox(curSelections.GetEntity(i));
        }

        invalidSelectionBoxes = false;
    }


	UpdateHoodPos();
}

void SceneSelectionSystem::ForceEmitSignals()
{
	if (selectionHasChanges)
	{
		// emit signals
		SceneSignals::Instance()->EmitSelectionChanged((SceneEditor2 *) GetScene(), &curSelections, &curDeselections);

		selectionHasChanges = false;
		curDeselections.Clear();
	}
}

void SceneSelectionSystem::Input(DAVA::UIEvent *event)
{
	if (IsLocked() || !selectionAllowed || (0 == componentMaskForSelection))
	{
		return;
	}

    if (DAVA::UIEvent::Phase::BEGAN == event->phase)
    {
		// we can select only if mouse isn't over hood axis
		// or if hood is invisible now
		// or if current mode is NORMAL (no modification)
		if(!hoodSystem->IsVisible() ||
			ST_MODIF_OFF == hoodSystem->GetModifMode() ||
			ST_AXIS_NONE == hoodSystem->GetPassingAxis())
		{
			if(event->tid == DAVA::UIEvent::BUTTON_1)
			{
				const EntityGroup* collisionEntities = collisionSystem->ObjectsRayTestFromCamera();
				EntityGroup selectableItems = GetSelecetableFromCollision(collisionEntities);

				DAVA::Entity *firstEntity = selectableItems.GetEntity(0);
				DAVA::Entity *nextEntity = selectableItems.GetEntity(0);

                // sequent selection?
                if(SettingsManager::GetValue(Settings::Scene_SelectionSequent).AsBool())
                {
				    // search possible next item only if now there is no selection or is only single selection
				    if(curSelections.Size() <= 1)
				    {
					    // find first after currently selected items
					    for(size_t i = 0; i < selectableItems.Size(); i++)
					    {
						    DAVA::Entity *entity = selectableItems.GetEntity(i);
						    if(curSelections.ContainsEntity(entity))
						    {
							    if((i + 1) < selectableItems.Size())
							    {
								    nextEntity = selectableItems.GetEntity(i + 1);
								    break;
							    }
						    }
					    }
				    }
                }

				int curKeyModifiers = QApplication::keyboardModifiers();
				if(curKeyModifiers & Qt::ControlModifier)
				{
					AddSelection(firstEntity);
				}
				else if(curKeyModifiers & Qt::AltModifier)
				{
					RemSelection(firstEntity);
				}
				else
				{
					// if new selection is NULL or is one of already selected items
					// we should change current selection only on phase end
					if(nextEntity == NULL || NULL != curSelections.IntersectedEntity(&selectableItems))
					{
						applyOnPhaseEnd = true;
						lastSelection = nextEntity;
					}
					else
					{
						SetSelection(nextEntity);
					}
				}
			}
		}
	}
    else if (DAVA::UIEvent::Phase::ENDED == event->phase)
    {
		if(event->tid == DAVA::UIEvent::BUTTON_1)
		{
			if(applyOnPhaseEnd)
			{
				applyOnPhaseEnd = false;
				SetSelection(lastSelection);
			}
		}
	}
}

void SceneSelectionSystem::Draw()
{
	if (IsLocked())
	{
		return;
	}

	if(curSelections.Size() > 0)
    {
        DAVA::int32 drawMode = SettingsManager::GetValue(Settings::Scene_SelectionDrawMode).AsInt32();

        DAVA::RenderHelper::eDrawType wireDrawType = (!(drawMode & SS_DRAW_NO_DEEP_TEST)) ? DAVA::RenderHelper::DRAW_WIRE_DEPTH : DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH;
        DAVA::RenderHelper::eDrawType solidDrawType = (!(drawMode & SS_DRAW_NO_DEEP_TEST)) ? DAVA::RenderHelper::DRAW_SOLID_DEPTH : DAVA::RenderHelper::DRAW_SOLID_NO_DEPTH;

        for (DAVA::uint32 i = 0; i < curSelections.Size(); i++)
		{
            DAVA::AABBox3 selectionBox = curSelections.GetBbox(i);

			// draw selection share
			if(drawMode & SS_DRAW_SHAPE)
			{
                GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABox(selectionBox, DAVA::Color(1.0f, 1.0f, 1.0f, 1.0f), wireDrawType);
            }
			// draw selection share
			else if(drawMode & SS_DRAW_CORNERS)
			{
                GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABoxCorners(selectionBox, DAVA::Color(1.0f, 1.0f, 1.0f, 1.0f), wireDrawType);
            }

			// fill selection shape
			if(drawMode & SS_DRAW_BOX)
			{
                GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABox(selectionBox, DAVA::Color(1.0f, 1.0f, 1.0f, 0.15f), solidDrawType);
            }
        }
	}
}

void SceneSelectionSystem::ProcessCommand(const Command2 *command, bool redo)
{
	if(NULL != command)
	{
        auto commandId = command->GetId();
        
		if((CMDID_ENTITY_REMOVE == commandId))
		{
			// remove from selection entity that was removed by command
			RemSelection(command->GetEntity());
		}
		else if((CMDID_ENTITY_CHANGE_PARENT == commandId) || (CMDID_TRANSFORM == commandId))
		{
            invalidSelectionBoxes = true;
        }
    }
}

void SceneSelectionSystem::SetSelection(const EntityGroup &newSelection)
{
	if (!IsLocked())
	{
		Clear();

        uint32 count = newSelection.Size();
        for (uint32 i = 0; i < count; ++i)
        {
			auto entity = newSelection.GetEntity(i);
			if (IsEntitySelectable(entity) && !curSelections.ContainsEntity(entity))
			{
				curSelections.Add(entity, GetSelectionAABox(entity));
				selectionHasChanges = true;
			}
		}

		if (selectionHasChanges)
		{
			invalidSelectionBoxes = true;
			UpdateHoodPos();
		}
	}
}


void SceneSelectionSystem::SetSelection(DAVA::Entity *entity)
{
	if(!IsLocked())
	{
		Clear();

		// add new selection
		if(NULL != entity)
		{
			AddSelection(entity);
		}

		UpdateHoodPos();
	}
}

void SceneSelectionSystem::AddSelection(DAVA::Entity *entity)
{
    if(!IsLocked())
    {
        if(IsEntitySelectable(entity) && !curSelections.ContainsEntity(entity))
        {
            EntityGroupItem selectableItem;
            
            selectableItem.entity = entity;
            selectableItem.bbox = GetSelectionAABox(entity);
            curSelections.Add(selectableItem);
            
            selectionHasChanges = true;
            UpdateHoodPos();
            
            invalidSelectionBoxes = true;
        }
    }
}

void SceneSelectionSystem::AddSelection(const EntityGroup &entities)
{
    if(!IsLocked())
    {
        for (size_t i = 0; i < entities.Size(); ++i)
        {
            const auto entity = entities.GetEntity(i);
            if (IsEntitySelectable(entity) && !curSelections.ContainsEntity(entity))
            {
                EntityGroupItem selectableItem;

                selectableItem.entity = entity;
                selectableItem.bbox = GetSelectionAABox(entity);
                curSelections.Add(selectableItem);
                selectionHasChanges = true;
                invalidSelectionBoxes = true;
            }
        }
        UpdateHoodPos();
    }
}

bool SceneSelectionSystem::IsEntitySelectable(DAVA::Entity *entity) const
{
    if(!IsLocked() && entity)
    {
        return (componentMaskForSelection & entity->GetAvailableComponentFlags());
    }
    
    return false;
}

void SceneSelectionSystem::RemSelection(DAVA::Entity *entity)
{
	if(!IsLocked())
	{
		if(curSelections.ContainsEntity(entity))
		{
			curSelections.Rem(entity);
			curDeselections.Add(entity);

			selectionHasChanges = true;
		}

		UpdateHoodPos();
	}
}

void SceneSelectionSystem::RemSelection(const EntityGroup& entities)
{
    if (!IsLocked())
    {
        for (size_t i = 0; i < entities.Size(); ++i)
        {
            auto entity = entities.GetEntity(i);
            if (curSelections.ContainsEntity(entity))
            {
                curSelections.Rem(entity);
                curDeselections.Add(entity);

                selectionHasChanges = true;
            }
        }
        UpdateHoodPos();
    }
}

void SceneSelectionSystem::Clear()
{
	if(!IsLocked())
	{
		while(curSelections.Size() > 0)
		{
			DAVA::Entity *entity = curSelections.GetEntity(0);

			curSelections.Rem(entity);
			curDeselections.Add(entity);

			selectionHasChanges = true;
		}

        UpdateHoodPos();
	}
}

EntityGroup SceneSelectionSystem::GetSelection() const
{
	return curSelections;
}

size_t SceneSelectionSystem::GetSelectionCount() const
{
	return curSelections.Size();
}

DAVA::Entity* SceneSelectionSystem::GetSelectionEntity(int index) const
{
	return curSelections.GetEntity(index);
}

bool SceneSelectionSystem::IsEntitySelected(Entity *entity)
{
    return curSelections.ContainsEntity(entity);
}

bool SceneSelectionSystem::IsEntitySelectedHierarchically(Entity *entity)
{
    while (entity)
    {
        if (curSelections.ContainsEntity(entity))
            return true;

        entity = entity->GetParent();
    }
    return false;
}

void SceneSelectionSystem::SelectedItemsWereModified()
{
	// don't change selection on phase end
	applyOnPhaseEnd = false;
}

void SceneSelectionSystem::SetPivotPoint(ST_PivotPoint pp)
{
	curPivotPoint = pp;
}

ST_PivotPoint SceneSelectionSystem::GetPivotPoint() const
{
	return curPivotPoint;
}


void SceneSelectionSystem::SetLocked(bool lock)
{
	SceneSystem::SetLocked(lock);

	hoodSystem->LockAxis(lock);
	hoodSystem->SetVisible(!lock);

	if(!lock)
	{
		UpdateHoodPos();
	}
}

void SceneSelectionSystem::UpdateHoodPos() const
{
	if(curSelections.Size() > 0)
	{
		DAVA::Vector3 p;
		bool lockHoodModif = false;

		switch (curPivotPoint)
		{
		case ST_PIVOT_ENTITY_CENTER:
			p = curSelections.GetZeroPos(0);
			break;
		default:
			p = curSelections.GetCommonZeroPos();
			break;
		}

		// check if we have locked entities in selection group
		// if so - lock modification hood
		for(size_t i = 0; i < curSelections.Size(); ++i)
		{
			if(curSelections.GetEntity(i)->GetLocked())
			{
				lockHoodModif = true;
				break;
			}
		}

		hoodSystem->LockModif(lockHoodModif);
		hoodSystem->SetPosition(p);
		hoodSystem->SetVisible(true);
	}
	else
	{
		hoodSystem->LockModif(false);
		hoodSystem->SetVisible(false);
	}
    
    SceneEditor2 *sc = (SceneEditor2 *)GetScene();
    sc->cameraSystem->UpdateDistanceToCamera();
}

EntityGroup SceneSelectionSystem::GetSelecetableFromCollision(const EntityGroup *collisionEntities)
{
	EntityGroup ret;

	if(NULL != collisionEntities)
	{
		for(size_t i = 0; i < collisionEntities->Size(); ++i)
		{
			DAVA::Entity *entity = collisionEntities->GetEntity(i);
            
            if(componentMaskForSelection & entity->GetAvailableComponentFlags())
            {
                EntityGroupItem item;
                
                item.entity = GetSelectableEntity(entity);
                item.bbox = GetSelectionAABox(item.entity);
                
                ret.Add(item);
            }
		}
	}

	return ret;
}

DAVA::Entity* SceneSelectionSystem::GetSelectableEntity(DAVA::Entity* entity)
{
	DAVA::Entity *selectableEntity = NULL;
	
	if(NULL != entity)
	{
		// find most top solid entity
		DAVA::Entity *parent = entity;
		while(NULL != parent)
		{
			if(parent->GetSolid())
			{
				selectableEntity = parent;
			}

			parent = parent->GetParent();
		}

		// still not found?
		if(NULL == selectableEntity)
		{
			// let it current entity to be tread as solid
			selectableEntity = entity;
		}
	}

	return selectableEntity;
}

DAVA::AABBox3 SceneSelectionSystem::GetSelectionAABox(int index) const
{
	return curSelections.GetBbox(index);
}

DAVA::AABBox3 SceneSelectionSystem::GetSelectionAABox(DAVA::Entity *entity) const
{
	DAVA::Matrix4 beginTransform;
	beginTransform.Identity();

	return GetSelectionAABox(entity, beginTransform);
}

DAVA::AABBox3 SceneSelectionSystem::GetSelectionAABox(DAVA::Entity *entity, const DAVA::Matrix4 &transform) const
{
	DAVA::AABBox3 ret = DAVA::AABBox3(DAVA::Vector3(0, 0, 0), 0);

	if(NULL != entity)
	{
		// we will get selection bbox from collision system 
		DAVA::AABBox3 entityBox = collisionSystem->GetBoundingBox(entity);

		// add childs boxes into entity box
		for (DAVA::int32 i = 0; i < entity->GetChildrenCount(); i++)
		{
			DAVA::Entity *childEntity = entity->GetChild(i);
			DAVA::AABBox3 childBox = GetSelectionAABox(childEntity, childEntity->GetLocalTransform());

			if(entityBox.IsEmpty())
			{
				entityBox = childBox;
			}
			else
			{
				if(!childBox.IsEmpty())
				{
					entityBox.AddAABBox(childBox);
				}
			}
		}

		// we should return box with specified transformation
		if(!entityBox.IsEmpty())
		{
			entityBox.GetTransformedBox(transform, ret);
		}
	}

	return ret;
}

void SceneSelectionSystem::SetSelectionComponentMask(DAVA::uint64 mask)
{
    componentMaskForSelection = mask;

    if(curSelections.Size() != 0)
    {
        Clear();
    }
    else
    {
        selectionHasChanges = true; // magic to say to selectionModel() of scene tree to reset selection
    }
}

void SceneSelectionSystem::Activate()
{
    SetLocked(false);
}

void SceneSelectionSystem::Deactivate()
{
    SetLocked(true);
}


