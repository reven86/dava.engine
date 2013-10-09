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
#include "Scene/System/CameraSystem.h"
#include "Scene/System/SelectionSystem.h"
#include "Scene/SceneEditor2.h"

#include "Commands2/EntityRemoveCommand.h"
#include "Commands2/EntityMoveCommand.h"

// framework
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Scene.h"

SceneCollisionSystem::SceneCollisionSystem(DAVA::Scene * scene)
	: DAVA::SceneSystem(scene)
	, rayIntersectCached(false)
	, drawMode(ST_COLL_DRAW_NOTHING)
	, curLandscape(NULL)
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
	DAVA::SafeDelete(objectsCollDisp);
	DAVA::SafeDelete(objectsCollConf);

	DAVA::SafeDelete(landCollWorld); 
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
		return landIntersectCached;
	}

	// no cache. start new ray test
	lastLandRayFrom = from;
	lastLandRayTo = to;

	DAVA::Vector3 rayDirection = to - from;
	DAVA::float32 rayLength = rayDirection.Length();
	DAVA::Vector3 rayStep = rayDirection / rayLength;

	btVector3 btFrom(from.x, from.y, from.z);

	landIntersectCached = false;

	while (rayLength > 0)
	{
		btVector3 btTo(btFrom.x() + rayStep.x, btFrom.y() + rayStep.y, btFrom.z() + rayStep.z);

		btCollisionWorld::ClosestRayResultCallback btCallback(btFrom, btTo);
		landCollWorld->rayTest(btFrom, btTo, btCallback);
		if(btCallback.hasHit()) 
		{
			btVector3 hitPoint = btCallback.m_hitPointWorld;
			ret = DAVA::Vector3(hitPoint.x(), hitPoint.y(), hitPoint.z());

			landIntersectCached = true;
			break;
		}

		btFrom = btTo;
		rayLength -= 1.0f;
	}

	lastLandCollision = ret;
	intersectionPoint = ret;
	return landIntersectCached;
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
	return curLandscape;
}

void SceneCollisionSystem::UpdateCollisionObject(DAVA::Entity *entity)
{
	if(NULL != entity)
	{
		// make sure that WorldTransform is up to date
		if(NULL != entity->GetScene())
		{
			entity->GetScene()->transformSystem->Process();
		}
	}

	RemoveEntity(entity);
	AddEntity(entity);
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
	// reset cache on new frame
	rayIntersectCached = false;

	if(drawMode & ST_COLL_DRAW_LAND_COLLISION)
	{
		DAVA::Vector3 tmp;
		LandRayTestFromCamera(tmp);
	}
}

void SceneCollisionSystem::ProcessUIEvent(DAVA::UIEvent *event)
{
	lastMousePos = event->point;
}

void SceneCollisionSystem::Draw()
{
	int oldState = DAVA::RenderManager::Instance()->GetState();
	DAVA::RenderManager::Instance()->SetState(DAVA::RenderState::STATE_COLORMASK_ALL | DAVA::RenderState::STATE_DEPTH_WRITE | DAVA::RenderState::STATE_DEPTH_TEST);

	if(drawMode & ST_COLL_DRAW_LAND)
	{
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0, 0.5f, 0, 1.0f));
		landCollWorld->debugDrawWorld();
	}

	if(drawMode & ST_COLL_DRAW_LAND_RAYTEST)
	{
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0, 1.0f, 0, 1.0f));
		DAVA::RenderHelper::Instance()->DrawLine(lastLandRayFrom, lastLandRayTo);
	}

	if(drawMode & ST_COLL_DRAW_LAND_COLLISION)
	{
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0, 1.0f, 0, 1.0f));
		DAVA::RenderHelper::Instance()->DrawPoint(lastLandCollision, 7.0f);
	}

	if(drawMode & ST_COLL_DRAW_OBJECTS)
	{
		objectsCollWorld->debugDrawWorld();
	}

	if(drawMode & ST_COLL_DRAW_OBJECTS_RAYTEST)
	{
		DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 0, 0, 1.0f));
		DAVA::RenderHelper::Instance()->DrawLine(lastRayFrom, lastRayTo);
	}

	if(drawMode & ST_COLL_DRAW_OBJECTS_SELECTED)
	{
		// current selected entities
		SceneSelectionSystem *selectionSystem = ((SceneEditor2 *) GetScene())->selectionSystem;
		if(NULL != selectionSystem)
		{
			const EntityGroup *selectedEntities = selectionSystem->GetSelection();
			for (size_t i = 0; i < selectedEntities->Size(); i++)
			{
				// get collision object for solid selected entity
				CollisionBaseObject *cObj = entityToCollision.value(selectedEntities->GetEntity(i), NULL);

				// if no collision object for solid selected entity,
				// try to get collision object for real selected entity
				if(NULL == cObj)
				{
					cObj = entityToCollision.value(selectedEntities->GetEntity(i), NULL);
				}

				if(NULL != cObj && NULL != cObj->btObject)
				{
					objectsCollWorld->debugDrawObject(cObj->btObject->getWorldTransform(), cObj->btObject->getCollisionShape(), btVector3(1.0f, 0.65f, 0.0f));
				}
			}
		}
	}

	DAVA::RenderManager::Instance()->SetState(oldState);
}

void SceneCollisionSystem::ProcessCommand(const Command2 *command, bool redo)
{
	if(NULL != command)
	{
		DAVA::Entity *entity = command->GetEntity();
		switch(command->GetId())
		{
		case CMDID_TRANSFORM:
		case CMDID_ENTITY_MOVE:
			// update bullet object
			UpdateCollisionObject(entity);
			break;
			/*
		case CMDID_ENTITY_MOVE:
			{
				const EntityMoveCommand* moveCommand = (EntityMoveCommand*) command;

				if(entityToCollision.contains(moveCommand->entity))
				{
					UpdateCollisionObject(moveCommand->entity);
				}
			}
			break;
			*/
		default:
			break;
		}
	}
}

void SceneCollisionSystem::AddEntity(DAVA::Entity * entity)
{
	if(NULL != entity)
	{
		// check if we still don't have this entity in our collision world
		CollisionBaseObject *cObj = entityToCollision.value(entity, NULL);
		if(NULL == cObj)
		{
			// build collision object for entity
			cObj = BuildFromEntity(entity);
		}

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
		// destroy collision object from entity
		DestroyFromEntity(entity);

		// destroy collision object for entitys childs
		for(int i = 0; i < entity->GetChildrenCount(); ++i)
		{
			RemoveEntity(entity->GetChild(i));
		}
	}
}

CollisionBaseObject* SceneCollisionSystem::BuildFromEntity(DAVA::Entity * entity)
{
	CollisionBaseObject *collObj = NULL;

	// check if this entity is landscape
	DAVA::Landscape *landscape = DAVA::GetLandscape(entity);
	DAVA::RenderObject *renderObject = DAVA::GetRenderObject(entity);
	DAVA::ParticleEmitter* particleEmitter = DAVA::GetEmitter(entity);

	if(NULL != landscape)
	{
		collObj = new CollisionLandscape(entity, landCollWorld, landscape);
		curLandscape = landscape;
	}
	else if(NULL != particleEmitter)
	{
		collObj = new CollisionParticleEmitter(entity, objectsCollWorld, particleEmitter);
	}
	else if(NULL != renderObject && entity->IsLodMain(0))
	{
		collObj = new CollisionRenderObject(entity, objectsCollWorld, renderObject);
	}

	if(NULL != collObj)
	{
		entityToCollision[entity] = collObj;
		collisionToEntity[collObj->btObject] = entity;
	}

	return collObj;
}

void SceneCollisionSystem::DestroyFromEntity(DAVA::Entity * entity)
{
	CollisionBaseObject *cObj = entityToCollision.value(entity, NULL);

	if(curLandscape == DAVA::GetLandscape(entity))
	{
		curLandscape = NULL;
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
{ }

void SceneCollisionDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	DAVA::Vector3 davaFrom(from.x(), from.y(), from.z());
	DAVA::Vector3 davaTo(to.x(), to.y(), to.z());
	DAVA::Color davaColor(color.x(), color.y(), color.z(), 1.0f);

	manager->SetColor(davaColor);
	helper->DrawLine(davaFrom, davaTo);
}

void SceneCollisionDebugDrawer::drawContactPoint( const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color )
{
	DAVA::Color davaColor(color.x(), color.y(), color.z(), 1.0f);

	manager->SetColor(davaColor);
	helper->DrawPoint(DAVA::Vector3(PointOnB.x(), PointOnB.y(), PointOnB.z()));
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
