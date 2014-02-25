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



#include "Scene/System/CollisionSystem.h"
#include "Scene/System/CollisionSystem/CollisionRenderObject.h"
#include "Scene/System/CollisionSystem/CollisionLandscape.h"
#include "Scene/System/CollisionSystem/CollisionParticleEmitter.h"
#include "Scene/System/CollisionSystem/CollisionBox.h"
#include "Scene/System/CameraSystem.h"
#include "Scene/System/SelectionSystem.h"
#include "Scene/SceneEditor2.h"

#include "Commands2/EntityRemoveCommand.h"
#include "Commands2/EntityParentChangeCommand.h"

// framework
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Scene.h"

SceneCollisionSystem::SceneCollisionSystem(DAVA::Scene * scene)
	: DAVA::SceneSystem(scene)
	, rayIntersectCached(false)
	, landIntersectCached(false)
	, landIntersectCachedResult(false)
	, drawMode(ST_COLL_DRAW_NOTHING)
	, curLandscapeEntity(NULL)
{
	btVector3 worldMin(-1000,-1000,-1000);
	btVector3 worldMax(1000,1000,1000);

	objectsCollConf = new btDefaultCollisionConfiguration();
	objectsCollDisp = new btCollisionDispatcher(objectsCollConf);
	objectsBroadphase = new btAxisSweep3(worldMin,worldMax);
	objectsDebugDrawer = new SceneCollisionDebugDrawer();
	objectsDebugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
	objectsCollWorld = new btCollisionWorld(objectsCollDisp, objectsBroadphase, objectsCollConf);
	objectsCollWorld->setDebugDrawer(objectsDebugDrawer);

	landCollConf = new btDefaultCollisionConfiguration();
	landCollDisp = new btCollisionDispatcher(landCollConf);
	landBroadphase = new btAxisSweep3(worldMin,worldMax);
	landDebugDrawer = new SceneCollisionDebugDrawer();
	landDebugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
	landCollWorld = new btCollisionWorld(landCollDisp, landBroadphase, landCollConf);
	landCollWorld->setDebugDrawer(landDebugDrawer);

	renderState = DAVA::RenderManager::Instance()->Subclass3DRenderState(RenderStateData::STATE_COLORMASK_ALL |
												   RenderStateData::STATE_DEPTH_WRITE |
												   RenderStateData::STATE_DEPTH_TEST);
    
    objectsDebugDrawer->SetRenderState(renderState);
    landDebugDrawer->SetRenderState(renderState);
}

SceneCollisionSystem::~SceneCollisionSystem()
{
	QMapIterator<DAVA::Entity*, CollisionBaseObject*> i(entityToCollision);
	while(i.hasNext())
	{
		i.next();

		CollisionBaseObject *cObj = i.value();
		delete cObj;
	}

	DAVA::SafeDelete(objectsCollWorld);
	DAVA::SafeDelete(objectsBroadphase);
    DAVA::SafeDelete(objectsDebugDrawer);
	DAVA::SafeDelete(objectsCollDisp);
	DAVA::SafeDelete(objectsCollConf);

	DAVA::SafeDelete(landCollWorld); 
	DAVA::SafeDelete(landDebugDrawer);
	DAVA::SafeDelete(landBroadphase);
	DAVA::SafeDelete(landCollDisp);
	DAVA::SafeDelete(landCollConf);
}

void SceneCollisionSystem::SetDrawMode(int mode)
{
	drawMode = mode;
}

int SceneCollisionSystem::GetDebugDrawFlags()
{
	return drawMode;
}

const EntityGroup* SceneCollisionSystem::ObjectsRayTest(const DAVA::Vector3 &from, const DAVA::Vector3 &to)
{
	DAVA::Entity *retEntity = NULL;

	// check if cache is available 
	if(rayIntersectCached && lastRayFrom == from && lastRayTo == to)
	{
		return &rayIntersectedEntities;
	}

	// no cache. start ray new ray test
	lastRayFrom = from;
	lastRayTo = to;
	rayIntersectedEntities.Clear();

	btVector3 btFrom(from.x, from.y, from.z);
	btVector3 btTo(to.x, to.y, to.z);

	btCollisionWorld::AllHitsRayResultCallback btCallback(btFrom, btTo);
	objectsCollWorld->rayTest(btFrom, btTo, btCallback);

	if(btCallback.hasHit()) 
	{
		int foundCount = btCallback.m_collisionObjects.size();
		if(foundCount > 0)
		{
			for(int i = 0; i < foundCount; ++i)
			{
				DAVA::float32 lowestFraction = 1.0f;
				DAVA::Entity *lowestEntity = NULL;

				for(int j = 0; j < foundCount; ++j)
				{
					if(btCallback.m_hitFractions[j] < lowestFraction)
					{
						btCollisionObject *btObj = btCallback.m_collisionObjects[j];
						DAVA::Entity *entity = collisionToEntity.value(btObj, NULL);

						if(!rayIntersectedEntities.HasEntity(entity))
						{
							lowestFraction = btCallback.m_hitFractions[j];
							lowestEntity = entity;
						}
					}
				}

				if(NULL != lowestEntity)
				{
					rayIntersectedEntities.Add(lowestEntity, GetBoundingBox(lowestEntity));
				}
			}
		}
	}

	rayIntersectCached = true;
	return &rayIntersectedEntities;
}

const EntityGroup* SceneCollisionSystem::ObjectsRayTestFromCamera()
{
	SceneCameraSystem *cameraSystem	= ((SceneEditor2 *) GetScene())->cameraSystem;

	DAVA::Vector3 camPos = cameraSystem->GetCameraPosition();
	DAVA::Vector3 camDir = cameraSystem->GetPointDirection(lastMousePos);

	DAVA::Vector3 traceFrom = camPos;
	DAVA::Vector3 traceTo = traceFrom + camDir * 1000.0f;

	return ObjectsRayTest(traceFrom, traceTo);
}

bool SceneCollisionSystem::LandRayTest(const DAVA::Vector3 &from, const DAVA::Vector3 &to, DAVA::Vector3& intersectionPoint)
{
	DAVA::Vector3 ret;

	// check if cache is available 
	if(landIntersectCached && lastLandRayFrom == from && lastLandRayTo == to)
	{
		intersectionPoint = lastLandCollision;
		return landIntersectCachedResult;
	}

	// no cache. start new ray test
	lastLandRayFrom = from;
	lastLandRayTo = to;
	landIntersectCached = true;
	landIntersectCachedResult = false;

	DAVA::Vector3 rayDirection = to - from;
	DAVA::float32 rayLength = rayDirection.Length();
	DAVA::Vector3 rayStep = (rayDirection / rayLength * 5.0f);

	btVector3 btFrom(from.x, from.y, from.z);
	while (rayLength > 0)
	{
		btVector3 btTo(btFrom.x() + rayStep.x, btFrom.y() + rayStep.y, btFrom.z() + rayStep.z);

		btCollisionWorld::ClosestRayResultCallback btCallback(btFrom, btTo);
		landCollWorld->rayTest(btFrom, btTo, btCallback);
		if(btCallback.hasHit()) 
		{
			btVector3 hitPoint = btCallback.m_hitPointWorld;
			ret = DAVA::Vector3(hitPoint.x(), hitPoint.y(), hitPoint.z());

			landIntersectCachedResult = true;
			break;
		}

		btFrom = btTo;
		rayLength -= rayStep.Length();
	}

	lastLandCollision = ret;
	intersectionPoint = ret;

	return landIntersectCachedResult;
}

bool SceneCollisionSystem::LandRayTestFromCamera(DAVA::Vector3& intersectionPoint)
{
	SceneCameraSystem *cameraSystem	= ((SceneEditor2 *) GetScene())->cameraSystem;

	DAVA::Vector3 camPos = cameraSystem->GetCameraPosition();
	DAVA::Vector3 camDir = cameraSystem->GetPointDirection(lastMousePos);

	DAVA::Vector3 traceFrom = camPos;
	DAVA::Vector3 traceTo = traceFrom + camDir * 1000.0f;

	return LandRayTest(traceFrom, traceTo, intersectionPoint);
}

DAVA::Landscape* SceneCollisionSystem::GetLandscape() const
{
	return DAVA::GetLandscape(curLandscapeEntity);
}

void SceneCollisionSystem::UpdateCollisionObject(DAVA::Entity *entity)
{
	if(NULL != entity)
	{
		// make sure that WorldTransform is up to date
		if(NULL != entity->GetScene())
		{
			entity->GetScene()->transformSystem->Process(.001f);
		}
	}

	RemoveEntity(entity);
	AddEntity(entity);
}

void SceneCollisionSystem::RemoveCollisionObject(DAVA::Entity *entity)
{
	RemoveEntity(entity);
}

DAVA::AABBox3 SceneCollisionSystem::GetBoundingBox(DAVA::Entity *entity)
{
	DAVA::AABBox3 aabox;

	if(NULL != entity)
	{
		CollisionBaseObject* collObj = entityToCollision.value(entity, NULL);
		if(NULL != collObj)
		{
			aabox = collObj->boundingBox;
		}
	}

	return aabox;
}

void SceneCollisionSystem::Update(DAVA::float32 timeElapsed)
{
	// check in there are entities that should be added or removed
	if(entitiesToAdd.size() > 0 || entitiesToRemove.size() > 0)
	{
		DAVA::Set<DAVA::Entity*>::iterator i = entitiesToRemove.begin();
		DAVA::Set<DAVA::Entity*>::iterator end = entitiesToRemove.end();

		for(; i != end; ++i)
		{
			DestroyFromEntity(*i);
		}

		i = entitiesToAdd.begin();
		end = entitiesToAdd.end();

		for(; i != end; ++i)
		{
			BuildFromEntity(*i);
		}

		entitiesToAdd.clear();
		entitiesToRemove.clear();
	}

	// reset ray cache on new frame
	rayIntersectCached = false;

	if(drawMode & ST_COLL_DRAW_LAND_COLLISION)
	{
		DAVA::Vector3 tmp;
		LandRayTestFromCamera(tmp);
	}
}

void SceneCollisionSystem::ProcessUIEvent(DAVA::UIEvent *event)
{
	// don't have to update last mouse pos when event is not from the mouse
	if (event->phase != UIEvent::PHASE_KEYCHAR && event->phase != UIEvent::PHASE_JOYSTICK)
	{
		lastMousePos = event->point;
	}
}

void SceneCollisionSystem::Draw()
{
	if(drawMode & ST_COLL_DRAW_LAND)
	{
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0, 0.5f, 0, 1.0f));
		landCollWorld->debugDrawWorld();
	}

	if(drawMode & ST_COLL_DRAW_LAND_RAYTEST)
	{
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0, 1.0f, 0, 1.0f));
		DAVA::RenderHelper::Instance()->DrawLine(lastLandRayFrom, lastLandRayTo, 1.0f, renderState);
	}

	if(drawMode & ST_COLL_DRAW_LAND_COLLISION)
	{
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0, 1.0f, 0, 1.0f));
		DAVA::RenderHelper::Instance()->DrawPoint(lastLandCollision, 7.0f, renderState);
	}

	if(drawMode & ST_COLL_DRAW_OBJECTS)
	{
		objectsCollWorld->debugDrawWorld();
	}

	if(drawMode & ST_COLL_DRAW_OBJECTS_RAYTEST)
	{
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 0, 0, 1.0f));
		DAVA::RenderHelper::Instance()->DrawLine(lastRayFrom, lastRayTo, 1.0f, renderState);
	}

	if(drawMode & ST_COLL_DRAW_OBJECTS_SELECTED)
	{
		// current selected entities
		SceneSelectionSystem *selectionSystem = ((SceneEditor2 *) GetScene())->selectionSystem;
		if(NULL != selectionSystem)
		{
			for (size_t i = 0; i < selectionSystem->GetSelectionCount(); i++)
			{
				// get collision object for solid selected entity
				CollisionBaseObject *cObj = entityToCollision.value(selectionSystem->GetSelectionEntity(i), NULL);

				// if no collision object for solid selected entity,
				// try to get collision object for real selected entity
				if(NULL == cObj)
				{
					cObj = entityToCollision.value(selectionSystem->GetSelectionEntity(i), NULL);
				}

				if(NULL != cObj && NULL != cObj->btObject)
				{
					objectsCollWorld->debugDrawObject(cObj->btObject->getWorldTransform(), cObj->btObject->getCollisionShape(), btVector3(1.0f, 0.65f, 0.0f));
				}
			}
		}
	}
}

void SceneCollisionSystem::ProcessCommand(const Command2 *command, bool redo)
{
	if(NULL != command)
	{
		DAVA::Entity *entity = command->GetEntity();
		switch(command->GetId())
		{
		case CMDID_TRANSFORM:
			UpdateCollisionObject(entity);
			break;
		case CMDID_ENTITY_CHANGE_PARENT:
			{
				EntityParentChangeCommand *cmd = (EntityParentChangeCommand *) command;
				if(redo)
				{
					if(NULL != cmd->newParent)
					{
						UpdateCollisionObject(entity);
					}
				}
				else
				{
					if(NULL != cmd->oldParent)
					{
						UpdateCollisionObject(entity);
					}
					else
					{
						RemoveCollisionObject(entity);
					}
				}
			}
			break;
		case CMDID_LANDSCAPE_SET_HEIGHTMAP:
		case CMDID_HEIGHTMAP_MODIFY:
			UpdateCollisionObject(curLandscapeEntity);
			break;

        case CMDID_LOD_CREATE_PLANE:
        case CMDID_LOD_DELETE:
            {
                UpdateCollisionObject(command->GetEntity());
                break;
            }
		default:
			break;
		}
	}
}

void SceneCollisionSystem::AddEntity(DAVA::Entity * entity)
{
	if(NULL != entity)
	{
		entitiesToRemove.erase(entity);
		entitiesToAdd.insert(entity);

		// build collision object for entity childs
		for(int i = 0; i < entity->GetChildrenCount(); ++i)
		{
			AddEntity(entity->GetChild(i));
		}
	}
}

void SceneCollisionSystem::RemoveEntity(DAVA::Entity * entity)
{
	if(NULL != entity)
	{
		entitiesToAdd.erase(entity);
		entitiesToRemove.insert(entity);

		// destroy collision object for entities childs
		for(int i = 0; i < entity->GetChildrenCount(); ++i)
		{
			RemoveEntity(entity->GetChild(i));
		}
	}
}

CollisionBaseObject* SceneCollisionSystem::BuildFromEntity(DAVA::Entity * entity)
{
	CollisionBaseObject *cObj = NULL;
	bool isLandscape = false;

	// check if this entity is landscape
	DAVA::Landscape *landscape = DAVA::GetLandscape(entity);
	if( NULL == cObj &&
		NULL != landscape)
	{
		cObj = new CollisionLandscape(entity, landCollWorld, landscape);
		isLandscape = true;
	}

	
	DAVA::ParticleEffectComponent* particleEffect = DAVA::GetEffectComponent(entity);
	if( NULL == cObj &&
		NULL != particleEffect)
	{
		cObj = new CollisionParticleEffect(entity, objectsCollWorld);
	}


	DAVA::RenderObject *renderObject = DAVA::GetRenderObject(entity);
	if( NULL == cObj &&
		NULL != renderObject && entity->IsLodMain(0))
	{
		cObj = new CollisionRenderObject(entity, objectsCollWorld, renderObject);
	}

	DAVA::Camera *camera = DAVA::GetCamera(entity);
	if( NULL == cObj && 
		NULL != camera)
	{
		cObj = new CollisionBox(entity, objectsCollWorld, camera->GetPosition(), 0.75f);
	}

	// build simple collision box for all other entities, that has more than two components
	if( NULL == cObj &&
		NULL != entity)
	{
		if( NULL != entity->GetComponent(DAVA::Component::USER_COMPONENT) ||
			NULL != entity->GetComponent(DAVA::Component::SOUND_COMPONENT) ||
			NULL != entity->GetComponent(DAVA::Component::LIGHT_COMPONENT))
		{
			cObj = new CollisionBox(entity, objectsCollWorld, entity->GetWorldTransform().GetTranslationVector(), 0.5f);
		}
	}

	if(NULL != cObj)
	{
		if(entityToCollision.count(entity) > 0)
		{
			DestroyFromEntity(entity);
		}

		entityToCollision[entity] = cObj;
		collisionToEntity[cObj->btObject] = entity;
	}

	if(isLandscape)
	{
		curLandscapeEntity = entity;
	}

	return cObj;
}

void SceneCollisionSystem::DestroyFromEntity(DAVA::Entity * entity)
{
	CollisionBaseObject *cObj = entityToCollision.value(entity, NULL);

	if(curLandscapeEntity == entity)
	{
		curLandscapeEntity = NULL;
	}

	if(NULL != cObj)
	{
		entityToCollision.remove(entity);
		collisionToEntity.remove(cObj->btObject);

		delete cObj;
	}
}

// -----------------------------------------------------------------------------------------------
// debug draw
// -----------------------------------------------------------------------------------------------

SceneCollisionDebugDrawer::SceneCollisionDebugDrawer()
	: manager(DAVA::RenderManager::Instance())
	, helper(DAVA::RenderHelper::Instance())
	, dbgMode(0)
{
    renderState = DAVA::RenderState::RENDERSTATE_2D_BLEND;
    manager->RetainRenderState(renderState);
}

SceneCollisionDebugDrawer::~SceneCollisionDebugDrawer()
{
    if(InvalidUniqueHandle != renderState)
    {
        manager->ReleaseRenderState(renderState);
    }
}

void SceneCollisionDebugDrawer::SetRenderState(UniqueHandle _renderState)
{
    if(_renderState != renderState)
    {
        if(InvalidUniqueHandle != renderState)
        {
            manager->ReleaseRenderState(renderState);
        }

        renderState = _renderState;
        
        if(InvalidUniqueHandle != renderState)
        {
            manager->RetainRenderState(renderState);
        }
    }
}

void SceneCollisionDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	DAVA::Vector3 davaFrom(from.x(), from.y(), from.z());
	DAVA::Vector3 davaTo(to.x(), to.y(), to.z());
	DAVA::Color davaColor(color.x(), color.y(), color.z(), 1.0f);

	manager->SetColor(davaColor);
	helper->DrawLine(davaFrom, davaTo, 1.0f, renderState);
}

void SceneCollisionDebugDrawer::drawContactPoint( const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color )
{
	DAVA::Color davaColor(color.x(), color.y(), color.z(), 1.0f);

	manager->SetColor(davaColor);
	helper->DrawPoint(DAVA::Vector3(PointOnB.x(), PointOnB.y(), PointOnB.z()), 1.0f, renderState);
}

void SceneCollisionDebugDrawer::reportErrorWarning( const char* warningString )
{ }

void SceneCollisionDebugDrawer::draw3dText( const btVector3& location,const char* textString )
{ }

void SceneCollisionDebugDrawer::setDebugMode( int debugMode )
{
	dbgMode = debugMode;
}

int SceneCollisionDebugDrawer::getDebugMode() const
{
	return dbgMode;
}
