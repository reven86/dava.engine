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
    if (EventSystem::SWITCH_CHANGED == event)
    {
        for (const auto& item : curSelections.GetContent())
        {
            if (entity == item.first)
            {
                invalidSelectionBoxes = true;
                break;
            }
        }
        /*
        for(DAVA::uint32 i = 0, e = curSelections.Size(); i < e; i++)
        {
            DAVA::Entity *selectedEntity = curSelections.GetEntity(i);

            // if switched entity selected - update it bounding box
            if(selectedEntity == entity)
            {
                invalidSelectionBoxes = true;
            }
        }
		*/
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
    if (invalidSelectionBoxes)
    {
        for (auto& item : curSelections.GetContent())
        {
            item.second = GetSelectionAABox(item.first);
        }
        /*
        for(DAVA::uint32 i = 0, e = curSelections.Size(); i < e; i++)
        {
            EntityGroup::Item *item = curSelections.GetItem(i);
            item->bbox = GetSelectionAABox(curSelections.GetEntity(i));
        }
		*/
        invalidSelectionBoxes = false;
    }


	UpdateHoodPos();
}

void SceneSelectionSystem::ForceEmitSignals()
{
	if (selectionHasChanges)
	{
		// emit signals
        // SceneSignals::Instance()->EmitSelectionChanged((SceneEditor2 *) GetScene(), &curSelections, &curDeselections);

        selectionHasChanges = false;
		curDeselections.Clear();
	}
}

void SceneSelectionSystem::ProcessSelectedGroup(const EntityGroup* collisionEntities)
{
    EntityGroup selectableItems = GetSelecetableFromCollision(collisionEntities);
    DAVA::Entity* firstEntity = selectableItems.GetFirstEntity();
    DAVA::Entity* nextEntity = selectableItems.GetFirstEntity();

    // sequent selection?
    if (SettingsManager::GetValue(Settings::Scene_SelectionSequent).AsBool())
    {
        // search possible next item only if now there is no selection or is only single selection
        if (curSelections.Size() <= 1)
        {
            // find first after currently selected items
            for (size_t i = 0, e = selectableItems.Size(); i < e; i++)
            {
                DAVA::Entity* entity = selectableItems.GetEntitySlow(i);
                if (curSelections.ContainsEntity(entity))
                {
                    if ((i + 1) < e)
                    {
                        nextEntity = selectableItems.GetEntitySlow(i + 1);
                        break;
                    }
                }
            }
        }
    }

    Qt::KeyboardModifiers curKeyModifiers = QApplication::keyboardModifiers();
    if (curKeyModifiers & Qt::ControlModifier)
    {
        AddSelection(firstEntity);
    }
    else if (curKeyModifiers & Qt::AltModifier)
    {
        ExcludeSelection(firstEntity);
    }
    else
    {
        // if new selection is NULL or is one of already selected items
        // we should change current selection only on phase end
        if ((nextEntity == nullptr) || (curSelections.IntersectedEntity(&selectableItems) != nullptr))
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

void SceneSelectionSystem::PerformSelectionAtPoint(const DAVA::Vector2& point)
{
    DAVA::Vector3 traceFrom;
    DAVA::Vector3 traceTo;
    SceneCameraSystem* cameraSystem = ((SceneEditor2*)GetScene())->cameraSystem;
    cameraSystem->GetRayTo2dPoint(point, 1000.0f, traceFrom, traceTo);
    ProcessSelectedGroup(collisionSystem->ObjectsRayTest(traceFrom, traceTo));
}

void SceneSelectionSystem::PerformSelectionInCurrentBox()
{
    const float32 minSelectionSize = 2.0f;

    Vector2 selectionSize = selectionEndPoint - selectionStartPoint;
    if ((std::abs(selectionSize.x) < minSelectionSize) || (std::abs(selectionSize.y) < minSelectionSize))
    {
        return;
    };

    float minX = std::min(selectionStartPoint.x, selectionEndPoint.x);
    float minY = std::min(selectionStartPoint.y, selectionEndPoint.y);
    float maxX = std::max(selectionStartPoint.x, selectionEndPoint.x);
    float maxY = std::max(selectionStartPoint.y, selectionEndPoint.y);

    Vector3 p0;
    Vector3 p1;
    Vector3 p2;
    Vector3 p3;
    Vector3 p4;
    SceneCameraSystem* cameraSystem = ((SceneEditor2*)GetScene())->cameraSystem;
    cameraSystem->GetRayTo2dPoint(Vector2(minX, minY), 1000.0f, p0, p1);
    cameraSystem->GetRayTo2dPoint(Vector2(maxX, minY), 1000.0f, p0, p2);
    cameraSystem->GetRayTo2dPoint(Vector2(minX, maxY), 1000.0f, p0, p4);
    cameraSystem->GetRayTo2dPoint(Vector2(maxX, maxY), 1000.0f, p0, p3);

    Plane planes[4];
    planes[0] = Plane(p2, p1, p0);
    planes[1] = Plane(p3, p2, p0);
    planes[2] = Plane(p4, p3, p0);
    planes[3] = Plane(p1, p4, p0);

    auto selectedObjects = collisionSystem->ClipObjectsToPlanes(planes, 4);
    for (const auto& item : selectedObjects->GetContent())
    {
        if (!lastGroupSelection.ContainsEntity(item.first))
        {
            AddSelection(item.first);
        }
    }

    for (const auto& item : lastGroupSelection.GetContent())
    {
        if (!selectedObjects->ContainsEntity(item.first))
        {
            ExcludeSelection(item.first);
        }
    }

    lastGroupSelection = *selectedObjects;
    applyOnPhaseEnd = false;
}

void SceneSelectionSystem::Input(DAVA::UIEvent *event)
{
    if (IsLocked() || !selectionAllowed || (0 == componentMaskForSelection) || (event->tid != DAVA::UIEvent::BUTTON_1))
    {
		return;
	}

    if (DAVA::UIEvent::Phase::BEGAN == event->phase)
    {
        // we can select only if mouse isn't over hood axis
		// or if hood is invisible now
		// or if current mode is NORMAL (no modification)
        if (!hoodSystem->IsVisible() || (ST_MODIF_OFF == hoodSystem->GetModifMode()) || (ST_AXIS_NONE == hoodSystem->GetPassingAxis()))
        {
            selecting = true;
            selectionStartPoint = event->physPoint;
            selectionEndPoint = selectionStartPoint;
            lastGroupSelection.Clear();

            PerformSelectionAtPoint(selectionStartPoint);
        }
	}
    else if (selecting && (DAVA::UIEvent::Phase::DRAG == event->phase))
    {
        selectionEndPoint = event->physPoint;
        PerformSelectionInCurrentBox();
    }
    else if (DAVA::UIEvent::Phase::ENDED == event->phase)
    {
        if (applyOnPhaseEnd)
        {
            SetSelection(lastSelection);
        }
        applyOnPhaseEnd = false;
        selecting = false;
    }
}

void SceneSelectionSystem::Draw()
{
	if (IsLocked())
	{
		return;
	}

    Vector2 selectionSize = selectionEndPoint - selectionStartPoint;
    if (selecting && (selectionSize.Length() >= 1.0f))
    {
        DAVA::Rect targetRect(selectionStartPoint, selectionSize);
        RenderSystem2D::Instance()->FillRect(targetRect, DAVA::Color(1.0f, 1.0f, 1.0f, 1.0f / 3.0f));
        RenderSystem2D::Instance()->DrawRect(targetRect, DAVA::Color::White);
    }

    if (!curSelections.GetContent().empty())
    {
        DAVA::int32 drawMode = SettingsManager::GetValue(Settings::Scene_SelectionDrawMode).AsInt32();

        DAVA::RenderHelper::eDrawType wireDrawType =
        (!(drawMode & SS_DRAW_NO_DEEP_TEST)) ? DAVA::RenderHelper::DRAW_WIRE_DEPTH : DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH;

        DAVA::RenderHelper::eDrawType solidDrawType =
        (!(drawMode & SS_DRAW_NO_DEEP_TEST)) ? DAVA::RenderHelper::DRAW_SOLID_DEPTH : DAVA::RenderHelper::DRAW_SOLID_NO_DEPTH;

        for (const auto& item : curSelections.GetContent())
        {
            DAVA::AABBox3 bbox = EntityGroup::TransformItemBoundingBox(item);
            // draw selection share
            if (drawMode & SS_DRAW_SHAPE)
            {
                GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABox(bbox, DAVA::Color(1.0f, 1.0f, 1.0f, 1.0f), wireDrawType);
            }
            // draw selection share
            else if (drawMode & SS_DRAW_CORNERS)
            {
                GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABoxCorners(bbox, DAVA::Color(1.0f, 1.0f, 1.0f, 1.0f), wireDrawType);
            }
            // fill selection shape
            if (drawMode & SS_DRAW_BOX)
            {
                GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABox(bbox, DAVA::Color(1.0f, 1.0f, 1.0f, 0.15f), solidDrawType);
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
            ExcludeSelection(command->GetEntity());
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

        for (const auto& item : newSelection.GetContent())
        {
            if (IsEntitySelectable(item.first) && !curSelections.ContainsEntity(item.first))
            {
                curSelections.Add(item.first, item.second);
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
    if (!IsLocked())
    {
		Clear();
        if (entity != nullptr)
        {
            AddSelection(entity);
		}
		UpdateHoodPos();
	}
}

void SceneSelectionSystem::AddSelection(DAVA::Entity *entity)
{
    if (!IsLocked() && IsEntitySelectable(entity) && !curSelections.ContainsEntity(entity))
    {
        curSelections.Add(entity, GetSelectionAABox(entity));
        selectionHasChanges = true;
        invalidSelectionBoxes = true;
        UpdateHoodPos();
    }
}

void SceneSelectionSystem::AddSelection(const EntityGroup &entities)
{
    if (IsLocked())
    {
        return;
    }

    for (const auto& item : entities.GetContent())
    {
        if (IsEntitySelectable(item.first) && !curSelections.ContainsEntity(item.first))
        {
            curSelections.Add(item.first, GetSelectionAABox(item.first));
            selectionHasChanges = true;
            invalidSelectionBoxes = true;
        }
    }
    UpdateHoodPos();
}

bool SceneSelectionSystem::IsEntitySelectable(DAVA::Entity *entity) const
{
    if(!IsLocked() && entity)
    {
        return (componentMaskForSelection & entity->GetAvailableComponentFlags());
    }
    
    return false;
}

void SceneSelectionSystem::ExcludeSelection(DAVA::Entity* entity)
{
    if (!IsLocked())
    {
        if (curSelections.ContainsEntity(entity))
        {
            curSelections.Remove(entity);
            curDeselections.Add(entity);
			selectionHasChanges = true;
		}

		UpdateHoodPos();
	}
}

void SceneSelectionSystem::ExcludeSelection(const EntityGroup& entities)
{
    if (IsLocked())
    {
        return;
    }

    for (const auto& item : entities.GetContent())
    {
        if (curSelections.ContainsEntity(item.first))
        {
            curSelections.Remove(item.first);
            curDeselections.Add(item.first);

            selectionHasChanges = true;
        }
    }
    UpdateHoodPos();
}

void SceneSelectionSystem::Clear()
{
	if(!IsLocked())
	{
        for (const auto& item : curSelections.GetContent())
        {
            curDeselections.Add(item.first, item.second);
            selectionHasChanges = true;
		}
        curSelections.GetContent().clear();
        UpdateHoodPos();
	}
}

EntityGroup SceneSelectionSystem::GetSelection() const
{
    return EntityGroup();
}

size_t SceneSelectionSystem::GetSelectionCount() const
{
    return curSelections.GetContent().size();
}

DAVA::Entity* SceneSelectionSystem::GetSelectionEntity(int index) const
{
    return curSelections.GetEntitySlow(index);
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
    if (curSelections.GetContent().empty())
    {
        hoodSystem->LockModif(false);
        hoodSystem->SetVisible(false);
    }
    else
    {
		DAVA::Vector3 p;
		bool lockHoodModif = false;

		switch (curPivotPoint)
		{
		case ST_PIVOT_ENTITY_CENTER:
            p = curSelections.GetFirstZeroPos();
            break;
		default:
			p = curSelections.GetCommonZeroPos();
			break;
		}

		// check if we have locked entities in selection group
		// if so - lock modification hood
        for (const auto& item : curSelections.GetContent())
        {
            if (item.first->GetLocked())
            {
				lockHoodModif = true;
				break;
			}
		}

		hoodSystem->LockModif(lockHoodModif);
		hoodSystem->SetPosition(p);
		hoodSystem->SetVisible(true);
	}
    
    SceneEditor2 *sc = (SceneEditor2 *)GetScene();
    sc->cameraSystem->UpdateDistanceToCamera();
}

EntityGroup SceneSelectionSystem::GetSelecetableFromCollision(const EntityGroup* collisionEntities)
{
    DVASSERT(collisionEntities != nullptr);

    EntityGroup ret;
    for (const auto& item : collisionEntities->GetContent())
    {
        if (componentMaskForSelection & item.first->GetAvailableComponentFlags())
        {
            ret.Add(item.first, GetSelectionAABox(item.first));
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
    return curSelections.GetBboxSlow(index);
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

    if (curSelections.GetContent().empty())
    {
        selectionHasChanges = true; // magic to say to selectionModel() of scene tree to reset selection
    }
    else
    {
        Clear();
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


