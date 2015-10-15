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


#include "Qt/Scene/System/ModifSystem.h"
#include "Qt/Scene/System/HoodSystem.h"
#include "Qt/Scene/System/CameraSystem.h"
#include "Qt/Scene/System/CollisionSystem.h"
#include "Qt/Scene/System/SelectionSystem.h"
#include "Qt/Scene/System/TextDrawSystem.h"
#include "Qt/Scene/SceneSignals.h"

#include "Scene/EntityGroup.h"
#include "Scene/SceneEditor2.h"

#include "Commands2/TransformCommand.h"
#include "Commands2/BakeTransformCommand.h"
#include "Commands2/EntityAddCommand.h"
#include "Commands2/EntityLockCommand.h"
#include <QApplication>

EntityModificationSystem::EntityModificationSystem(DAVA::Scene * scene, SceneCollisionSystem *colSys, SceneCameraSystem *camSys, HoodSystem *hoodSys)
	: DAVA::SceneSystem(scene)
	, collisionSystem(colSys)
	, cameraSystem(camSys)
	, hoodSystem(hoodSys)
	, cloneState(CLONE_DONT)
	, inModifState(false)
	, modified(false)
	, snapToLandscape(false)
{
	SetModifMode(ST_MODIF_OFF);
	SetModifAxis(ST_AXIS_Z);
}

EntityModificationSystem::~EntityModificationSystem()
{ }

void EntityModificationSystem::SetModifAxis(ST_Axis axis)
{
	if(axis != ST_AXIS_NONE)
	{
		curAxis = axis;
		hoodSystem->SetModifAxis(axis);
	}
}

ST_Axis EntityModificationSystem::GetModifAxis() const
{
	return curAxis;
}

void EntityModificationSystem::SetModifMode(ST_ModifMode mode)
{
	curMode = mode;
	hoodSystem->SetModifMode(mode);
}

ST_ModifMode EntityModificationSystem::GetModifMode() const
{
	return curMode;
}

bool EntityModificationSystem::GetLandscapeSnap() const
{
	return snapToLandscape;
}

void EntityModificationSystem::SetLandscapeSnap(bool snap)
{
	snapToLandscape = snap;
}

void EntityModificationSystem::PlaceOnLandscape(const EntityGroup &entities)
{
	if(ModifCanStart(entities))
	{
		bool prevSnapToLandscape = snapToLandscape;

		snapToLandscape = true;
		BeginModification(entities);

		// move by z axis, so we will snap to landscape and keep x,y coords unmodified
		DAVA::Vector3 newPos3d = modifStartPos3d;
		newPos3d.z += 1.0f;
		Move(newPos3d);

		ApplyModification();
		EndModification();

		snapToLandscape = prevSnapToLandscape;
	}
}

void EntityModificationSystem::ResetTransform(const EntityGroup &entities)
{
	SceneEditor2 *sceneEditor = ((SceneEditor2 *) GetScene());

	if(NULL != sceneEditor && ModifCanStart(entities))
	{
		bool isMultiple = (entities.Size() > 1);
		
		DAVA::Matrix4 zeroTransform;
		zeroTransform.Identity();

		if(isMultiple)
		{
			sceneEditor->BeginBatch("Multiple transform");
		}

		for (size_t i = 0; i < entities.Size(); ++i)
		{
			DAVA::Entity *entity = entities.GetEntity(i);
			if(NULL != entity)
			{
				sceneEditor->Exec(new TransformCommand(entity,	entity->GetLocalTransform(), zeroTransform));
			}
		}

		if(isMultiple)
		{
			sceneEditor->EndBatch();
		}
	}
}

bool EntityModificationSystem::InModifState() const
{
	return inModifState;
}

bool EntityModificationSystem::InCloneState() const
{
	return (cloneState == CLONE_NEED);
}

bool EntityModificationSystem::InCloneDoneState() const
{
    return (cloneState == CLONE_DONE);
}

void EntityModificationSystem::Process(DAVA::float32 timeElapsed)
{ }

void EntityModificationSystem::Input(DAVA::UIEvent *event)
{
	if (IsLocked())
	{
		return;
	}

	if(NULL != collisionSystem)
	{
		// current selected entities
		SceneSelectionSystem *selectionSystem = ((SceneEditor2 *) GetScene())->selectionSystem;
		EntityGroup selectedEntities = selectionSystem->GetSelection();

        DAVA::Camera *camera = cameraSystem->GetCurCamera();

		// if we are not in modification state, try to find some selected item
		// that have mouse cursor at the top of it
		if(!inModifState)
		{
			// can we start modification???
			if(ModifCanStartByMouse(selectedEntities))
			{
				SceneSignals::Instance()->EmitMouseOverSelection((SceneEditor2 *) GetScene(), &selectedEntities);

                if (DAVA::UIEvent::Phase::BEGAN == event->phase)
                {
					if(event->tid == DAVA::UIEvent::BUTTON_1)
					{
						// go to modification state
						inModifState = true;

						// select current hood axis as active
						if(curMode == ST_MODIF_MOVE || curMode == ST_MODIF_ROTATE)
						{
							SetModifAxis(hoodSystem->GetPassingAxis());
						}

						// set entities to be modified
						BeginModification(selectedEntities);

						// init some values, needed for modifications
						modifStartPos3d = CamCursorPosToModifPos(camera, event->point);
						modifStartPos2d = event->point;

						// check if this is move with copy action
						int curKeyModifiers = QApplication::keyboardModifiers();
						if(curKeyModifiers & Qt::ShiftModifier && curMode == ST_MODIF_MOVE)
						{
							cloneState = CLONE_NEED;
						}
					}
				}
			}
			else
			{
				SceneSignals::Instance()->EmitMouseOverSelection((SceneEditor2 *) GetScene(), NULL);
			}
		}
		// or we are already in modification state
		else
		{
			// phase still continue
            if (event->phase == DAVA::UIEvent::Phase::DRAG)
            {
				DAVA::Vector3 moveOffset;
				DAVA::float32 rotateAngle;
				DAVA::float32 scaleForce;

				switch (curMode)
				{
				case ST_MODIF_MOVE:
					{
						DAVA::Vector3 newPos3d = CamCursorPosToModifPos(camera, event->point);
						moveOffset = Move(newPos3d);
						modified = true;
					}
					break;
				case ST_MODIF_ROTATE:
					{
						rotateAngle = Rotate(event->point);
						modified = true;
					}
					break;
				case ST_MODIF_SCALE:
					{
						scaleForce = Scale(event->point);
						modified = true;
					}
					break;
				default:
					break;
				}

				if(modified)
				{
					if(cloneState == CLONE_NEED)
					{
						CloneBegin();
						cloneState = CLONE_DONE;
					}

					// say to selection system, that selected items were modified
					selectionSystem->SelectedItemsWereModified();

					// lock hood, so it wont process ui events, wont calc. scale depending on it current position
					hoodSystem->LockScale(true);
					hoodSystem->SetModifOffset(moveOffset);
					hoodSystem->SetModifRotate(rotateAngle);
					hoodSystem->SetModifScale(scaleForce);
				}
			}
			// phase ended
            else if (event->phase == DAVA::UIEvent::Phase::ENDED)
            {
				if(event->tid == DAVA::UIEvent::BUTTON_1)
				{
					if(modified)
					{
						if(cloneState == CLONE_DONE)
						{
							CloneEnd();
						}
						else
						{
							ApplyModification();
						}
					}

					hoodSystem->SetModifOffset(DAVA::Vector3(0, 0, 0));
					hoodSystem->SetModifRotate(0);
					hoodSystem->SetModifScale(0);
					hoodSystem->LockScale(false);

					EndModification();
					inModifState = false;
					modified = false;
					cloneState = CLONE_DONT;
				}
			}
		}
	}
}

void EntityModificationSystem::AddDelegate(EntityModificationSystemDelegate *delegate)
{
    delegates.push_back(delegate);
}

void EntityModificationSystem::RemoveDelegate(EntityModificationSystemDelegate *delegate)
{
    delegates.remove(delegate);
}

void EntityModificationSystem::Draw()
{ }

void EntityModificationSystem::ProcessCommand(const Command2 *command, bool redo)
{

}

void EntityModificationSystem::BeginModification(const EntityGroup &entities)
{
	// clear any priv. selection
	EndModification();

	if(entities.Size() > 0)
	{
        modifEntities.reserve(entities.Size());
		for(size_t i = 0; i < entities.Size(); ++i)
		{
			DAVA::Entity *en = entities.GetEntity(i);
			if(NULL != en)
			{
				EntityToModify etm;
				etm.entity = en;
				etm.originalCenter = en->GetLocalTransform().GetTranslationVector();
				etm.originalTransform = en->GetLocalTransform();
				etm.moveToZeroPos.CreateTranslation(-etm.originalCenter);
				etm.moveFromZeroPos.CreateTranslation(etm.originalCenter);

				// inverse parent world transform, and remember it
				if(NULL != en->GetParent())
				{
					etm.originalParentWorldTransform = en->GetParent()->GetWorldTransform();
					etm.inversedParentWorldTransform = etm.originalParentWorldTransform;
					etm.inversedParentWorldTransform.SetTranslationVector(DAVA::Vector3(0, 0, 0));
					if(!etm.inversedParentWorldTransform.Inverse())
					{
						etm.inversedParentWorldTransform.Identity();
					}
				}
				else
				{
					etm.inversedParentWorldTransform.Identity();
					etm.originalParentWorldTransform.Identity();
				}

				modifEntities.push_back(etm);
			}
		}

		// remember current selection pivot point
		SceneSelectionSystem *selectionSystem = ((SceneEditor2 *)GetScene())->selectionSystem;
		modifPivotPoint = selectionSystem->GetPivotPoint();

		// center of this bbox will modification center, common for all entities
		modifEntitiesCenter = entities.GetCommonZeroPos();

		// prepare translation matrix's, used before and after rotation
		moveToZeroPosRelativeCenter.CreateTranslation(-modifEntitiesCenter);
		moveFromZeroPosRelativeCenter.CreateTranslation(modifEntitiesCenter);

		// remember axis vector we are rotating around
		switch(curAxis)
		{
		case ST_AXIS_X:
		case ST_AXIS_YZ:
			rotateAround = DAVA::Vector3(1, 0, 0);
			break;
		case ST_AXIS_Y:
		case ST_AXIS_XZ:
			rotateAround = DAVA::Vector3(0, 1, 0);
			break;
		case ST_AXIS_XY:
		case ST_AXIS_Z:
			rotateAround = DAVA::Vector3(0, 0, 1);
			break;
                
            default: break;
		}

		// 2d axis projection we are rotating around
		DAVA::Vector2 rotateAxis = Cam2dProjection(modifEntitiesCenter, modifEntitiesCenter + rotateAround);

		// axis dot products
		DAVA::Vector2 zeroPos = cameraSystem->GetScreenPos(modifEntitiesCenter);
		DAVA::Vector2 xPos = cameraSystem->GetScreenPos(modifEntitiesCenter + DAVA::Vector3(1, 0, 0));
		DAVA::Vector2 yPos = cameraSystem->GetScreenPos(modifEntitiesCenter + DAVA::Vector3(0, 1, 0));
		DAVA::Vector2 zPos = cameraSystem->GetScreenPos(modifEntitiesCenter + DAVA::Vector3(0, 0, 1));

		DAVA::Vector2 vx = xPos - zeroPos;
		DAVA::Vector2 vy = yPos - zeroPos;
		DAVA::Vector2 vz = zPos - zeroPos;

		crossXY = Abs(vx.CrossProduct(vy));
		crossXZ = Abs(vx.CrossProduct(vz));
		crossYZ = Abs(vy.CrossProduct(vz));

		// real rotate should be done in direction of 2dAxis normal,
		// so calculate this normal
		rotateNormal = DAVA::Vector2(-rotateAxis.y, rotateAxis.x);
        if(!rotateNormal.IsZero())
        {
            rotateNormal.Normalize();
        }

        DAVA::Camera *camera = cameraSystem->GetCurCamera();
        if(NULL != camera)
        {
            isOrthoModif = camera->GetIsOrtho();
        }
	}
}

void EntityModificationSystem::EndModification()
{
	modifEntitiesCenter.Set(0, 0, 0);
	modifEntities.clear();
    isOrthoModif = false;
}

bool EntityModificationSystem::ModifCanStart(const EntityGroup &selectedEntities) const
{
	bool modifCanStart = false;

	if(selectedEntities.Size() > 0)
	{
		bool hasLocked = false;

		// check if we have some locked items in selection
		for(size_t i = 0; i < selectedEntities.Size(); ++i)
		{
			if(selectedEntities.GetEntity(i)->GetLocked())
			{
				hasLocked = true;
				break;
			}
		}

        modifCanStart = !hasLocked;
    }

    return modifCanStart;
}

bool EntityModificationSystem::ModifCanStartByMouse(const EntityGroup &selectedEntities) const
{
	bool modifCanStart = false;

	// we can start modif only if there is no locked entities
	if(ModifCanStart(selectedEntities))
	{
        const bool modificationByGizmoOnly = SettingsManager::GetValue(Settings::Scene_ModificationByGizmoOnly).AsBool();
        
		// we can start modification only if mouse is over hood
		// on mouse is over one of currently selected items
		if(hoodSystem->GetPassingAxis() != ST_AXIS_NONE)
		{
			// allow starting modification
			modifCanStart = true;
		}
		else if(!modificationByGizmoOnly)
		{
			// send this ray to collision system and get collision objects
			const EntityGroup *collisionEntities = collisionSystem->ObjectsRayTestFromCamera();

			// check if one of got collision objects is intersected with selected items
			// if so - we can start modification
			if(collisionEntities->Size() > 0)
			{
				for(size_t i = 0; !modifCanStart && i < collisionEntities->Size(); ++i)
				{
					DAVA::Entity *collisionedEntity = collisionEntities->GetEntity(i);

					for(size_t j = 0; !modifCanStart && j < selectedEntities.Size(); ++j)
					{
						DAVA::Entity *selectedEntity = selectedEntities.GetEntity(j);

						if(selectedEntity == collisionedEntity)
						{
							modifCanStart = true;
						}
						else
						{
							if(selectedEntity->GetSolid())
							{
								modifCanStart = IsEntityContainRecursive(selectedEntity, collisionedEntity);
							}
						}
					}
				}
			}
		}
	}

	return modifCanStart;
}

void EntityModificationSystem::ApplyModification()
{
	SceneEditor2 *sceneEditor = ((SceneEditor2 *) GetScene());

	if(NULL != sceneEditor)
	{
		bool transformChanged = false;
		for (size_t i = 0; i < modifEntities.size(); ++i)
		{
			if(modifEntities[i].originalTransform != modifEntities[i].entity->GetLocalTransform())
			{
				transformChanged = true;
				break;
			}
		}

		if(transformChanged)
		{
			bool isMultiple = (modifEntities.size() > 1);

			if(isMultiple)
			{
				sceneEditor->BeginBatch("Multiple transform");
			}

			for (size_t i = 0; i < modifEntities.size(); ++i)
			{
				sceneEditor->Exec(new TransformCommand(modifEntities[i].entity,	modifEntities[i].originalTransform, modifEntities[i].entity->GetLocalTransform()));
			}

			if(isMultiple)
			{
				sceneEditor->EndBatch();
			}
		}
	}
}

//DAVA::Vector3 EntityModificationSystem::CamCursorPosToModifPos(const DAVA::Vector3 &camPosition, const DAVA::Vector3 &camPointDirection, const DAVA::Vector3 &planePoint)
DAVA::Vector3 EntityModificationSystem::CamCursorPosToModifPos(DAVA::Camera *camera, DAVA::Vector2 pos)
{
    DAVA::Vector3 ret;

    if(NULL != camera)
    {
        if(camera->GetIsOrtho())
        {
            DAVA::Vector3 dir = cameraSystem->GetPointDirection(pos);
            ret = DAVA::Vector3(dir.x, dir.y, 0);
        }
        else
        {
            DAVA::Vector3 planeNormal;
            DAVA::Vector3 camPosition = cameraSystem->GetCameraPosition();
            DAVA::Vector3 camToPointDirection = cameraSystem->GetPointDirection(pos);

            switch(curAxis)
            {
                case ST_AXIS_X:
                    {
                        if(crossXY > crossXZ) planeNormal = DAVA::Vector3(0, 0, 1);
                        else planeNormal = DAVA::Vector3(0, 1, 0);
                    }
                    break;
                case ST_AXIS_Y:
                    {
                        if(crossXY > crossYZ) planeNormal = DAVA::Vector3(0, 0, 1);
                        else planeNormal = DAVA::Vector3(1, 0, 0);
                    }
                    break;
                case ST_AXIS_Z:
                    {
                        if(crossXZ > crossYZ) planeNormal = DAVA::Vector3(0, 1, 0);
                        else planeNormal = DAVA::Vector3(1, 0, 0);
                    }
                    break;
                case ST_AXIS_XZ:
                    planeNormal = DAVA::Vector3(0, 1, 0);
                    break;
                case ST_AXIS_YZ:
                    planeNormal = DAVA::Vector3(1, 0, 0);
                    break;
                case ST_AXIS_XY:
                default:
                    planeNormal = DAVA::Vector3(0, 0, 1);
                    break;
            }

            DAVA::Plane plane(planeNormal, modifEntitiesCenter);
            DAVA::float32 distance = FLT_MAX;

            plane.IntersectByRay(camPosition, camToPointDirection, distance);
            ret = camPosition + (camToPointDirection * distance);
        }
    }
	
	return ret;
}

DAVA::Vector2 EntityModificationSystem::Cam2dProjection(const DAVA::Vector3 &from, const DAVA::Vector3 &to)
{
	DAVA::Vector2 axisBegin = cameraSystem->GetScreenPos(from);
	DAVA::Vector2 axisEnd = cameraSystem->GetScreenPos(to);
	DAVA::Vector2 ret = axisEnd - axisBegin;

    if(ret.IsZero())
    {
        ret = DAVA::Vector2(1.0f, 1.0f);
    }

    ret.Normalize();
	return ret;
}

DAVA::Vector3 EntityModificationSystem::Move(const DAVA::Vector3 &newPos3d)
{
	DAVA::Vector3 moveOffset;
	DAVA::Vector3 modifPosWithLocedAxis = modifStartPos3d;
	DAVA::Vector3 deltaPos3d = newPos3d - modifStartPos3d;

	switch(curAxis)
	{
	case ST_AXIS_X:
		modifPosWithLocedAxis.x += DAVA::Vector3(1, 0, 0).DotProduct(deltaPos3d);
		break;
	case ST_AXIS_Y:
		modifPosWithLocedAxis.y += DAVA::Vector3(0, 1, 0).DotProduct(deltaPos3d);
		break;
	case ST_AXIS_Z:
        if(!isOrthoModif)
        {
            modifPosWithLocedAxis.z += DAVA::Vector3(0, 0, 1).DotProduct(deltaPos3d);
        }
		break;
	case ST_AXIS_XY:
		modifPosWithLocedAxis.x = newPos3d.x;
		modifPosWithLocedAxis.y = newPos3d.y;
		break;
	case ST_AXIS_XZ:
        if(!isOrthoModif)
        {
            modifPosWithLocedAxis.x = newPos3d.x;
            modifPosWithLocedAxis.z = newPos3d.z;
        }
		break;
	case ST_AXIS_YZ:
        if(!isOrthoModif)
        {
            modifPosWithLocedAxis.z = newPos3d.z;
            modifPosWithLocedAxis.y = newPos3d.y;
        }
		break;
    default: break;
	}

	moveOffset = modifPosWithLocedAxis - modifStartPos3d;

	for (size_t i = 0; i < modifEntities.size(); ++i)
	{
		DAVA::Matrix4 moveModification;
		moveModification.Identity();
		moveModification.CreateTranslation(moveOffset * modifEntities[i].inversedParentWorldTransform);

		DAVA::Matrix4 newLocalTransform = modifEntities[i].originalTransform * moveModification;

		if(snapToLandscape)
		{
			newLocalTransform = newLocalTransform * SnapToLandscape(newLocalTransform.GetTranslationVector(), modifEntities[i].originalParentWorldTransform);
		}

		modifEntities[i].entity->SetLocalTransform(newLocalTransform);
	}

	return moveOffset;
}

DAVA::float32 EntityModificationSystem::Rotate(const DAVA::Vector2 &newPos2d)
{
	DAVA::Vector2 rotateLength = newPos2d - modifStartPos2d;
	DAVA::float32 rotateForce = -(rotateNormal.DotProduct(rotateLength)) / 70.0f;

	for (size_t i = 0; i < modifEntities.size(); ++i)
	{
		DAVA::Matrix4 rotateModification;
		rotateModification.Identity();
		rotateModification.CreateRotation(rotateAround * modifEntities[i].inversedParentWorldTransform, rotateForce);

		switch(modifPivotPoint)
		{
		case ST_PIVOT_ENTITY_CENTER:
			// move to zero, rotate, move back to original center point
			rotateModification = (modifEntities[i].moveToZeroPos * rotateModification) * modifEntities[i].moveFromZeroPos;
			break;
		case ST_PIVOT_COMMON_CENTER:
			// move to zero relative selection center, rotate, move back to original center point
			rotateModification = (moveToZeroPosRelativeCenter * rotateModification) * moveFromZeroPosRelativeCenter;
			break;
		default:
			rotateModification.Identity();
			break;
		}

		modifEntities[i].entity->SetLocalTransform(modifEntities[i].originalTransform * rotateModification);
	}

	return rotateForce;
}

DAVA::float32 EntityModificationSystem::Scale(const DAVA::Vector2 &newPos2d)
{
	DAVA::Vector2 scaleDir = (newPos2d - modifStartPos2d);
	DAVA::float32 scaleForce;

	scaleForce = 1.0f - (scaleDir.y / 70.0f);

	if(scaleForce >= 0)
	{
		for (size_t i = 0; i < modifEntities.size(); ++i)
		{
			DAVA::Matrix4 scaleModification;
			scaleModification.Identity();
			scaleModification.CreateScale(DAVA::Vector3(scaleForce, scaleForce, scaleForce) * modifEntities[i].inversedParentWorldTransform);

			switch(modifPivotPoint)
			{
			case ST_PIVOT_ENTITY_CENTER:
				// move to zero, rotate, move back to original center point
				scaleModification = (modifEntities[i].moveToZeroPos * scaleModification) * modifEntities[i].moveFromZeroPos;
				break;
			case ST_PIVOT_COMMON_CENTER:
				// move to zero relative selection center, rotate, move back to original center point
				scaleModification = (moveToZeroPosRelativeCenter * scaleModification) * moveFromZeroPosRelativeCenter;
				break;
			default:
				scaleModification.Identity();
				break;
			}
			
			modifEntities[i].entity->SetLocalTransform(modifEntities[i].originalTransform * scaleModification);
		}
	}

	return scaleForce;
}

DAVA::Matrix4 EntityModificationSystem::SnapToLandscape(const DAVA::Vector3 &point, const DAVA::Matrix4 &originalParentTransform) const
{
	DAVA::Matrix4 ret;
	ret.Identity();

	DAVA::Landscape *landscape = collisionSystem->GetLandscape();
	if(NULL != landscape)
	{
		DAVA::Vector3 resPoint;
		DAVA::Vector3 realPoint = point * originalParentTransform;

        if (landscape->PlacePoint(realPoint, resPoint))
		{
            resPoint = resPoint - realPoint;
			ret.SetTranslationVector(resPoint);
		}
	}

	return ret;
}

bool EntityModificationSystem::IsEntityContainRecursive(const DAVA::Entity *entity, const DAVA::Entity *child) const
{
	bool ret = false;

	if(NULL != entity && NULL != child)
	{
		for(int i = 0; !ret && i < entity->GetChildrenCount(); ++i)
		{
			if(child == entity->GetChild(i))
			{
				ret = true;
			}
			else
			{
				ret = IsEntityContainRecursive(entity->GetChild(i), child);
			}
		}
	}

	return ret;
}

void EntityModificationSystem::CloneBegin()
{
    // remove modif entities that are children for other modif entities
    for (uint32 i = 0; i < modifEntities.size(); ++i)
    {
        auto checkedEntity = modifEntities[i].entity;
        for (uint32 j = 0; j < modifEntities.size(); ++j)
        {
            if (i == j)
                continue;

            if (modifEntities[j].entity->IsMyChildRecursive(checkedEntity))
            {
                RemoveExchangingWithLast(modifEntities, i);
                --i;
                break;
            }
        }
        
    }

	if(modifEntities.size() > 0)
	{
        clonedEntities.reserve(modifEntities.size());
		for(size_t i = 0; i < modifEntities.size(); ++i)
		{
            DAVA::Entity *origEntity = modifEntities[i].entity;
            
            for (auto delegate : delegates)
            {
                delegate->WillClone(origEntity);
            }

			DAVA::Entity *newEntity = origEntity->Clone();

            for (auto delegate : delegates)
            {
                delegate->DidCloned(origEntity, newEntity);
            }

            newEntity->SetLocalTransform(modifEntities[i].originalTransform);

            Scene *scene = origEntity->GetScene();
            if(scene)
            {
                StaticOcclusionSystem *sosystem = scene->staticOcclusionSystem;
                DVASSERT(sosystem);
                
                sosystem->InvalidateOcclusionIndicesRecursively(newEntity);
            }

			origEntity->GetParent()->AddNode(newEntity);

			clonedEntities.push_back(newEntity);
		}
	}
}

void EntityModificationSystem::CloneEnd()
{
	if(modifEntities.size() > 0 && clonedEntities.size() == modifEntities.size())
	{
		SceneEditor2 *sceneEditor = ((SceneEditor2 *) GetScene());
		SceneSelectionSystem *selectionSystem = sceneEditor->selectionSystem;

		sceneEditor->BeginBatch("Clone");

		// we just moved original objects. Now we should return them back
		// to there original positions and move cloned object to the new positions
		// and only after that perform "add cloned entities to scene" commands
		for(size_t i = 0; i < modifEntities.size(); ++i)
		{
			// remember new transform
			Matrix4 newLocalTransform = modifEntities[i].entity->GetLocalTransform();

			// return original entity to original pos
			modifEntities[i].entity->SetLocalTransform(modifEntities[i].originalTransform);
			
			// move cloned entity to new pos
			clonedEntities[i]->SetLocalTransform(newLocalTransform);

			// remove entity from scene
			DAVA::Entity *cloneParent = clonedEntities[i]->GetParent();

            if (cloneParent)
            {
                cloneParent->RemoveNode(clonedEntities[i]);

                // and add it once again with command
                sceneEditor->Exec(new EntityAddCommand(clonedEntities[i], cloneParent));
            }

			// make cloned entiti selected
			SafeRelease(clonedEntities[i]);
		}

		sceneEditor->EndBatch();
	}

	clonedEntities.clear();
}

void EntityModificationSystem::RemoveEntity(DAVA::Entity * entity)
{
	if (GetLandscape(entity) != NULL)
	{
		SetLandscapeSnap(false);

		SceneEditor2 *sceneEditor = ((SceneEditor2 *) GetScene());
		SceneSignals::Instance()->EmitSnapToLandscapeChanged(sceneEditor, false);
	}
}

void EntityModificationSystem::MovePivotZero(const EntityGroup &entities)
{
    if(ModifCanStart(entities))
    {
        BakeGeometry(entities, BAKE_ZERO_PIVOT);
    }
}

void EntityModificationSystem::MovePivotCenter(const EntityGroup &entities)
{
    if(ModifCanStart(entities))
    {
        BakeGeometry(entities, BAKE_CENTER_PIVOT);
    }
}

void EntityModificationSystem::LockTransform(const EntityGroup &entities, bool lock)
{
    SceneEditor2 *sceneEditor = ((SceneEditor2 *) GetScene());
	if(NULL != sceneEditor)
	{
		bool isMultiple = (entities.Size() > 1);

        if(isMultiple)
		{
			sceneEditor->BeginBatch("Lock entities");
		}

 	    for(size_t i = 0; i < entities.Size(); ++i)
 	    {
            sceneEditor->Exec(new EntityLockCommand(entities.GetEntity(i), lock));
 	    }

        if(isMultiple)
		{
			sceneEditor->EndBatch();
		}
    }
}

void EntityModificationSystem::BakeGeometry(const EntityGroup &entities, BakeMode mode)
{
	SceneEditor2 *sceneEditor = ((SceneEditor2 *) GetScene());

	if(NULL != sceneEditor && entities.Size() == 1)
	{
        DAVA::Entity *entity = entities.GetEntity(0);
        DAVA::RenderObject *ro = GetRenderObject(entity);

        const char *commandMessage;
        switch(mode)
        {
            case BAKE_ZERO_PIVOT:
                commandMessage = "Move pivot point to zero";
                break;
            case BAKE_CENTER_PIVOT:
                commandMessage = "Move pivot point to center";
                break;
            default:
                DVASSERT(0 && "Unknown bake mode");
                return;
        }

        if(NULL != ro)
        {
            DAVA::Set<DAVA::Entity *> entityList;
            SearchEntitiesWithRenderObject(ro, sceneEditor, entityList);

            if(entityList.size() > 0)
            {
                DAVA::Matrix4 bakeTransform;

                switch(mode)
                {
                    case BAKE_ZERO_PIVOT:
                        bakeTransform = entity->GetLocalTransform();
                        break;
                    case BAKE_CENTER_PIVOT:
                        bakeTransform.SetTranslationVector(-ro->GetBoundingBox().GetCenter());
                        break;
                }

                sceneEditor->BeginBatch(commandMessage);

                // bake render object
                sceneEditor->Exec(new BakeGeometryCommand(ro, bakeTransform));

                // inverse bake to be able to move object on same place
                // after it geometry was baked
                DAVA::Matrix4 afterBakeTransform = bakeTransform;
                afterBakeTransform.Inverse();

                // for entities with same render object set new transform
                // to make them match their previous position
                DAVA::Set<DAVA::Entity *>::iterator it;
		        for (it = entityList.begin(); it != entityList.end(); ++it)
                {
                    DAVA::Entity *en = *it;
                    DAVA::Matrix4 origTransform = en->GetLocalTransform();
                    DAVA::Matrix4 newTransform = afterBakeTransform * origTransform;
                    
                    sceneEditor->Exec(new TransformCommand(en, origTransform, newTransform));

                    // also modify childs transform to make them be at
                    // right position after parent entity changed
                    for(size_t i = 0; i < (size_t)en->GetChildrenCount(); ++i)
                    {
                        DAVA::Entity *childEntity = en->GetChild(i);

                        DAVA::Matrix4 childOrigTransform = childEntity->GetLocalTransform();
                        DAVA::Matrix4 childNewTransform = childOrigTransform * bakeTransform;

                        sceneEditor->Exec(new TransformCommand(childEntity, childOrigTransform, childNewTransform));
                    }
                }

		        sceneEditor->EndBatch();
            }
        }
        // just modify child entities
        else
        {
            if(entity->GetChildrenCount() > 0)
            {
                DAVA::Vector3 newPivotPos;
                DAVA::Matrix4 transform;
                SceneSelectionSystem *selectionSystem = ((SceneEditor2 *) GetScene())->selectionSystem;

                switch(mode)
                {
                    case BAKE_ZERO_PIVOT:
                        newPivotPos = DAVA::Vector3(0, 0, 0);
                        break;
                    case BAKE_CENTER_PIVOT:
                        newPivotPos = selectionSystem->GetSelectionAABox(entity).GetCenter();
                        break;
                }

                sceneEditor->BeginBatch(commandMessage);

                // transform parent entity
                transform.SetTranslationVector(newPivotPos - entity->GetLocalTransform().GetTranslationVector());
                sceneEditor->Exec(new TransformCommand(entity, entity->GetLocalTransform(), entity->GetLocalTransform() * transform));

                // transform child entities with inversed parent transformation
                transform.Inverse();
                for(size_t i = 0; i < (size_t)entity->GetChildrenCount(); ++i)
                {
                    DAVA::Entity *childEntity = entity->GetChild(i);
                    sceneEditor->Exec(new TransformCommand(childEntity, childEntity->GetLocalTransform(), childEntity->GetLocalTransform() * transform));
                }

		        sceneEditor->EndBatch();
            }
        }
	}
}

void EntityModificationSystem::SearchEntitiesWithRenderObject(DAVA::RenderObject *ro, DAVA::Entity *root, DAVA::Set<DAVA::Entity *> &result)
{
    if(NULL != root)
    {
        DAVA::int32 count = root->GetChildrenCount();
        for(DAVA::int32 i = 0; i < count; ++i)
        {
            DAVA::Entity *en = root->GetChild(i);
            DAVA::RenderObject *enRenderObject = GetRenderObject(en);

            bool isSame = false;
            if(NULL != enRenderObject && ro->GetRenderBatchCount() == enRenderObject->GetRenderBatchCount())
            {
                // if renderObjects has same number of render batches we also should
                // check if polygon groups used inside that render batches are completely identical
                // but we should deal with the fact, that polygon groups order can differ 
                for(size_t j = 0; j < enRenderObject->GetRenderBatchCount(); ++j)
                {
                    bool found = false;
                    DAVA::PolygonGroup *pg = enRenderObject->GetRenderBatch(j)->GetPolygonGroup();

                    for(size_t k = 0; k < ro->GetRenderBatchCount(); ++k)
                    {
                        if(ro->GetRenderBatch(k)->GetPolygonGroup() == pg)
                        {
                            found = true;
                            break;
                        }
                    }

                    isSame = found;
                    if(!found)
                    {
                        break;
                    }
                }
            }

            if(isSame)
            {
                result.insert(en);
            }
            else if(en->GetChildrenCount() > 0)
            {
                SearchEntitiesWithRenderObject(ro, en, result);
            }
        }
    }
}
