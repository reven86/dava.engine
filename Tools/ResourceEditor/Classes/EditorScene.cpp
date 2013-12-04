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




#include "EditorScene.h"
#include "SceneNodeUserData.h"
#include "SceneEditor/SceneValidator.h"
#include "SceneEditor/EditorSettings.h"
#include "SceneEditor/HeightmapNode.h"
#include "Scene3D/Components/DebugRenderComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/BulletComponent.h"
#include "Render/Highlevel/RenderObject.h"

#include "StringConstants.h"

/*
    This means that if we'll call GameScene->GetClassName() it'll return "Scene"
    This is for correct serialization of framework subclasses.
 */
REGISTER_CLASS_WITH_ALIAS(EditorScene, "Scene");

EditorScene::EditorScene()
:Scene()
{
    selectedEntity = NULL;
    originalHandler = NULL;
	selection = 0;
	lastSelectedPhysics = 0;
	proxy = 0;
    btVector3 worldMin(-1000,-1000,-1000);
	btVector3 worldMax(1000,1000,1000);
        
    collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	broadphase = new btAxisSweep3(worldMin,worldMax);
	collisionWorld = new btCollisionWorld(dispatcher, broadphase, collisionConfiguration);

	landCollisionConfiguration = new btDefaultCollisionConfiguration();
	landDispatcher = new btCollisionDispatcher(landCollisionConfiguration);
	landBroadphase = new btAxisSweep3(worldMin,worldMax);
	landCollisionWorld = new btCollisionWorld(landDispatcher, landBroadphase, landCollisionConfiguration);

	renderSystem->AddRenderLayer(LAYER_ARROWS, PASS_FORWARD, LAST_LAYER);

    SetDrawGrid(true);

    editorLightSystem = new EditorLightSystem(this);
	AddSystem(editorLightSystem, 1 << Component::LIGHT_COMPONENT);
    
	SetShadowBlendMode(ShadowVolumeRenderPass::MODE_BLEND_MULTIPLY);
}

EditorScene::~EditorScene()
{
	RemoveSystem(editorLightSystem);
	SafeDelete(editorLightSystem);
    
	ReleaseUserData(this);
	SafeDelete(collisionWorld);
	SafeDelete(broadphase);
	SafeDelete(dispatcher);
	SafeDelete(collisionConfiguration);

	SafeDelete(landCollisionWorld);
	SafeDelete(landBroadphase);
	SafeDelete(landDispatcher);
	SafeDelete(landCollisionConfiguration);
}

void EditorScene::Update(float32 timeElapsed)
{    
	Scene::Update(timeElapsed);

	editorLightSystem->Process();

	CheckNodes(this);
	UpdateBullet(this);

	collisionWorld->updateAabbs();
}

void EditorScene::RemoveNode(Entity * node)
{
	RemoveBullet(node);
	Scene::RemoveNode(node);
}

void EditorScene::RemoveBullet(Entity * node)
{
	if(NULL != node)
	{
		node->RemoveComponent(Component::BULLET_COMPONENT);

		int size = node->GetChildrenCount();
		for (int i = 0; i < size; i++)
		{
			RemoveBullet(node->GetChild(i));
		}
	}
}

void EditorScene::UpdateBullet(Entity * curr)
{
	if(NULL != curr)
	{
		BulletComponent *bulletComponent = (BulletComponent*) curr->GetComponent(Component::BULLET_COMPONENT);
		if(NULL != bulletComponent && NULL != bulletComponent->GetBulletObject())
		{
			((BulletObject*)bulletComponent->GetBulletObject())->UpdateCollisionObject();
		}

		int size = curr->GetChildrenCount();
		for (int i = 0; i < size; i++)
		{
			UpdateBullet(curr->GetChild(i));
		}
	}
}

void EditorScene::CheckNodes(Entity * curr)
{
	if(NULL != curr)
	{
		bool newDebugComp = false;
		DebugRenderComponent *dbgComp = NULL;

		BaseObject *bulletObject = NULL;
		BulletComponent * bulletComponent = (BulletComponent*)curr->GetComponent(Component::BULLET_COMPONENT);

		if(NULL != bulletComponent)
		{
			bulletObject = bulletComponent->GetBulletObject();
		}

		// create debug render component for all nodes
		dbgComp = (DebugRenderComponent *) curr->GetComponent(Component::DEBUG_RENDER_COMPONENT);
		if(NULL == dbgComp)
		{
			dbgComp = new DebugRenderComponent();
			newDebugComp = true;
			curr->AddComponent(dbgComp);
			dbgComp->Release();
		}

		// check other debug settings

		// is camera?
		CameraComponent *camComp = (CameraComponent *) curr->GetComponent(Component::CAMERA_COMPONENT);
		if(NULL != camComp)
		{
			// set flags to show it
			if(newDebugComp)
			{
				dbgComp->SetDebugFlags(dbgComp->GetDebugFlags() | DebugRenderComponent::DEBUG_DRAW_CAMERA);
			}

			// create bullet object for camera (allow selecting it)
			/*
			if(NULL == bulletComponent)
			{
				bulletComponent = (BulletComponent*) curr->GetOrCreateComponent(Component::BULLET_COMPONENT);
				bulletComponent->SetBulletObject(new BulletObject(this, collisionWorld, camComp->GetCamera(), camComp->GetCamera()->GetMatrix()));
			}
			*/
		}

		// is light?
		if(NULL != curr->GetComponent(Component::LIGHT_COMPONENT))
		{
			if(newDebugComp)
			{
				dbgComp->SetDebugFlags(dbgComp->GetDebugFlags() | DebugRenderComponent::DEBUG_DRAW_LIGHT_NODE);
			}

			if(NULL == bulletComponent || NULL == bulletObject)
			{
				BulletObject *bObj = new BulletObject(this, collisionWorld, curr, AABBox3(Vector3(), 2.5f), curr->GetWorldTransform());
				bulletComponent = (BulletComponent*) curr->GetOrCreateComponent(Component::BULLET_COMPONENT);
				bulletComponent->SetBulletObject(bObj);
				SafeRelease(bObj);
			}
		}

		// is user node
		if(NULL != curr->GetComponent(Component::USER_COMPONENT))
		{
			if(newDebugComp)
			{
				dbgComp->SetDebugFlags(dbgComp->GetDebugFlags() | DebugRenderComponent::DEBUG_DRAW_USERNODE);
			}

			if(NULL == bulletComponent || NULL == bulletObject)
			{
				BulletObject *bObj = new BulletObject(this, collisionWorld, curr, AABBox3(Vector3(), 2.5f), curr->GetWorldTransform());
				bulletComponent = (BulletComponent*) curr->GetOrCreateComponent(Component::BULLET_COMPONENT);
				bulletComponent->SetBulletObject(bObj);
				SafeRelease(bObj);
			}
		}

		// is render object?
		RenderComponent * renderComponent = (RenderComponent*) curr->GetComponent(Component::RENDER_COMPONENT);
		if(NULL != renderComponent)
		{
			RenderObject *rObj = renderComponent->GetRenderObject();

			if(NULL != rObj && rObj->GetType() != RenderObject::TYPE_LANDSCAPE && curr->IsLodMain(0))
			{
				if(NULL == bulletComponent || NULL == bulletObject)
				{
					BulletObject *bObj = new BulletObject(this, collisionWorld, curr, curr->GetWorldTransform());
					bulletComponent = (BulletComponent*) curr->GetOrCreateComponent(Component::BULLET_COMPONENT);
					bulletComponent->SetBulletObject(bObj);
					SafeRelease(bObj);
				}
			}
		}
	}

	int size = curr->GetChildrenCount();
	for (int i = 0; i < size; i++)
	{
		CheckNodes(curr->GetChild(i));
	}
}

void EditorScene::ReleaseUserData(Entity * curr)
{
	if(curr)
	{
		curr->RemoveComponent(Component::BULLET_COMPONENT);
	}

	int size = curr->GetChildrenCount();
	for (int i = 0; i < size; i++)
	{
		ReleaseUserData(curr->GetChild(i));
	}
}

Entity * GetSolidParent(Entity* curr)
{
	if (curr == 0)
		return 0;
	
	Entity * parentResult = GetSolidParent(curr->GetParent());
	
	if (curr->GetSolid() && parentResult == 0)
	{
		return curr;
	}
	else
		return parentResult;
}

Entity * GetLodParent(Entity * curr)
{
	bool hasLod = (curr->GetComponent(Component::LOD_COMPONENT) != 0);
	if(hasLod)
	{
		return curr;
	}
	else 
	{
		Entity * parent = curr->GetParent();
		if (parent == 0)
			return 0;
		return GetLodParent(parent);
	}
}



void EditorScene::TrySelection(Vector3 from, Vector3 direction)
{
	if (selection)
		selection->SetDebugFlags(selection->GetDebugFlags() & (~DebugRenderComponent::DEBUG_DRAW_AABOX_CORNERS));

	btVector3 pos(from.x, from.y, from.z);
    btVector3 to(direction.x, direction.y, direction.z);
		
    btCollisionWorld::AllHitsRayResultCallback cb(pos, to);
    collisionWorld->rayTest(pos, to, cb);
	btCollisionObject * coll = 0;
	if (cb.hasHit()) 
    {
		//Logger::FrameworkDebug("Has Hit");
		int findedIndex = cb.m_collisionObjects.size() - 1;
		if(lastSelectedPhysics)
		{
			BulletComponent * bulletComponent = (BulletComponent*)lastSelectedPhysics->GetComponent(Component::BULLET_COMPONENT);
			BulletObject * bulletObject = (BulletObject*)bulletComponent->GetBulletObject();
			if (bulletObject)
			{
				for (int i = cb.m_collisionObjects.size() - 1; i >= 0 ; i--)
				{					
					if (bulletObject->GetCollisionObject() == cb.m_collisionObjects[i])
					{
						findedIndex = i;
						break;
					}
				}
				while (findedIndex >= 0 && bulletObject->GetCollisionObject() == cb.m_collisionObjects[findedIndex])
					findedIndex--;
				findedIndex = findedIndex % cb.m_collisionObjects.size();
			}
		}
			//		Logger::FrameworkDebug("size:%d selIndex:%d", cb.m_collisionObjects.size(), findedIndex);
		
		if (findedIndex == -1)
			findedIndex = cb.m_collisionObjects.size() - 1;
		coll = cb.m_collisionObjects[findedIndex];
		selection = FindSelected(this, coll);
		lastSelectedPhysics = selection;
		SetSelection(selection);
	}
	else 
	{
		SetSelection(0);
	}
}

void EditorScene::JuncCollWorldToLandscapeCollWorld()
{
	
	btCollisionObjectArray& landscapeObjects = landCollisionWorld->getCollisionObjectArray();
	if(landscapeObjects.size() > 1)
	{
		return;
	}

	for(int32 i = 0; i < landscapeObjects.size(); ++i)
	{
		originalHandler = landscapeObjects[i]->getBroadphaseHandle();
		collisionWorld->addCollisionObject(landscapeObjects[i]);
	}
}

void EditorScene::SeparateCollWorldFromLandscapeCollWorld()
{
	btCollisionObjectArray& landscapeObjects = landCollisionWorld->getCollisionObjectArray();
	if(landscapeObjects.size() > 1)
	{
		return;
	}
	for(int32 i = 0; i < landscapeObjects.size(); ++i)
	{
		collisionWorld->removeCollisionObject(landscapeObjects[i]);
		landscapeObjects[i]->setBroadphaseHandle(originalHandler);
	}
}

bool EditorScene::TryIsTargetAccesible(Vector3 from, Vector3 target)
{
	if (selection)
		selection->SetDebugFlags(selection->GetDebugFlags() & (~DebugRenderComponent::DEBUG_DRAW_AABOX_CORNERS));

	btVector3 pos(from.x, from.y, from.z);
    btVector3 to(target.x, target.y, target.z);

    btCollisionWorld::AllHitsRayResultCallback cb(pos, to);
    
	collisionWorld->rayTest(pos, to, cb);

	btCollisionObject * coll = 0;
	if (cb.hasHit()) 
    {
		//there is some obj before target
		//cb.m_hitPointWorld - contain coord of breaker
		return false;
	}
	else
	{
		return true;
	}
	
}

bool EditorScene::LandscapeIntersection(const DAVA::Vector3 &from, const DAVA::Vector3 &direction, DAVA::Vector3 &point)
{
	btVector3 pos(from.x, from.y, from.z);
    btVector3 to(direction.x, direction.y, direction.z);
    
    btCollisionWorld::ClosestRayResultCallback cb(pos, to);
	uint64 time1 = SystemTimer::Instance()->AbsoluteMS();
	uint64 time2;
    landCollisionWorld->rayTest(pos, to, cb);
	time2 = SystemTimer::Instance()->AbsoluteMS();
	//Logger::FrameworkDebug("raytest %lld", time2-time1);
	btCollisionObject * coll = 0;
	if (cb.hasHit()) 
    {
		//int findedIndex = cb.m_collisionObjects.size() - 1;
		//
		//if (findedIndex == -1)
		//	findedIndex = cb.m_collisionObjects.size() - 1;
		//coll = cb.m_collisionObjects[findedIndex];
        
		//HeightmapNode *hm = FindHeightmap(this, coll);
        //if(hm)
        {
            point.x = cb.m_hitPointWorld.x();
            point.y = cb.m_hitPointWorld.y();
            point.z = cb.m_hitPointWorld.z();
            
            return true;
        }
    }
    
    return false;
}


HeightmapNode * EditorScene::FindHeightmap(Entity * curr, btCollisionObject * coll)
{
	HeightmapNode * node = dynamic_cast<HeightmapNode *> (curr);
	if (node && node->collisionObject == coll)
	{
        return node;
	}
    
	int size = curr->GetChildrenCount();
//	for (int i = 0; i < size; i++)
	for (int i = size-1; i >= 0; --i)
	{
		HeightmapNode * result = FindHeightmap(curr->GetChild(i), coll);
		if (result)
			return result;
	}
	return 0;
}


Entity * EditorScene::FindSelected(Entity * curr, btCollisionObject * coll)
{
	Entity * node = curr;
// LIGHT
//	if (node == 0)
//		node = dynamic_cast<LightNode *> (curr);
    
	if (node == 0)
		node = dynamic_cast<UserNode *> (curr);
	
	BulletComponent * bulletComponent = (BulletComponent*)node->GetComponent(Component::BULLET_COMPONENT);
	if (bulletComponent && bulletComponent->GetBulletObject())
	{
		BulletObject * bulletObject = (BulletObject*)bulletComponent->GetBulletObject();
		if (bulletObject->GetCollisionObject() == coll)
			return curr;
	}
    
	int size = curr->GetChildrenCount();
	for (int i = 0; i < size; i++)
	{
		Entity * result = FindSelected(curr->GetChild(i), coll);
		if (result)
			return result;
	}
	return 0;
}

void EditorScene::SetSelection(Entity *newSelection)
{
    if (selection)
    {
        uint32 flags = selection->GetDebugFlags();
        uint32 newFlags = flags & ~DebugRenderComponent::DEBUG_DRAW_AABOX_CORNERS;
		selection->SetDebugFlags(newFlags);
    }
    
	selection = newSelection;
    selectedEntity = newSelection;
    
	if (selection)
	{
		Entity * solid = GetSolidParent(selection);
        if(solid == 0 && (selection->GetComponent(Component::PARTICLE_EFFECT_COMPONENT) || GetEmitter(selection)))
        {
            // Magic fix for correct node selection
            solid = selection;
        }
		if(solid == 0)
		{
			solid = GetLodParent(selection);
		}
		if(solid)
		{
			selection = solid;
		}

        proxy = selection;
	}
	else
	{
		proxy = 0;
		lastSelectedPhysics = 0;
	}

	if(selection)
    {
        uint32 flags = selection->GetDebugFlags();
        uint32 newFlags = flags | DebugRenderComponent::DEBUG_DRAW_AABOX_CORNERS;
		selection->SetDebugFlags(newFlags);
    }
}

void EditorScene::SetBulletUpdate(Entity* curr, bool value)
{
	if(lastSelectedPhysics)
	{
		BulletComponent * bulletComponent = (BulletComponent*)lastSelectedPhysics->GetComponent(Component::BULLET_COMPONENT);
		if (bulletComponent)
		{
			BulletObject * bulletObject = (BulletObject*)bulletComponent->GetBulletObject();
			bulletObject->SetUpdateFlag(value);
		}
	}
	
	for (int32 i = 0; i < curr->GetChildrenCount(); i++)
	{
		SetBulletUpdate(curr->GetChild(i), value);
	}
}


void EditorScene::Draw()
{
//	DrawDebugNodes(this);
    
    RenderManager::Instance()->ClearStats();
	Scene::Draw();
    renderStats = RenderManager::Instance()->GetStats();
    
    if(drawGrid)
    {
        DrawGrid();
    }
}

#define GRIDMAX 500.0f
#define GRIDSTEP 10.0f
void EditorScene::DrawGrid()
{
    uint32 oldState = RenderManager::Instance()->GetState();	
	RenderManager::Instance()->SetState(RenderState::STATE_COLORMASK_ALL | RenderState::STATE_DEPTH_WRITE | RenderState::STATE_DEPTH_TEST); 
	RenderManager::Instance()->SetColor(0.7f, 0.7f, 0.7f, 1.0f);
	RenderManager::Instance()->FlushState();
	for (float32 x = -GRIDMAX; x <= GRIDMAX; x+=GRIDSTEP)
	{
		Vector3 v1(x, -GRIDMAX, 0);
		Vector3 v2(x, GRIDMAX, 0);
		
		Vector3 v3(-GRIDMAX, x, 0);
		Vector3 v4(GRIDMAX, x, 0);
		if (x!= 0.0f)
		{
			RenderHelper::Instance()->DrawLine(v1, v2);
			RenderHelper::Instance()->DrawLine(v3, v4);		
		}
	}
	RenderManager::Instance()->SetColor(0.0f, 0.0f, 0.0f, 1.0f);
	RenderHelper::Instance()->DrawLine(Vector3(-GRIDMAX, 0, 0), Vector3(GRIDMAX, 0, 0));
	RenderHelper::Instance()->DrawLine(Vector3(0, -GRIDMAX, 0), Vector3(0, GRIDMAX, 0));
	RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	
	RenderManager::Instance()->SetState(oldState);
}

void EditorScene::SetDrawGrid(bool newDrawGrid)
{
    drawGrid = newDrawGrid;
}

void EditorScene::SetForceLodLayer(Entity *node, int32 layer)
{
    if(!node)   return;
    
    Entity *n = node;
    
    do {
        LodComponent *lc = GetLodComponent(n);
        if(lc)
        {
            lc->SetForceLodLayer(layer);
        }
        
        n = n->GetParent();
    } while (n);
    
    SetForceLodLayerRecursive(node, layer);
}

void EditorScene::SetForceLodLayerRecursive(Entity *node, int32 layer)
{
    LodComponent *lc = GetLodComponent(node);
    if(lc)
    {
        lc->SetForceLodLayer(layer);
    }
    
    int32 count = node->GetChildrenCount();
    for(int32 i = 0; i < count; ++i)
    {
        SetForceLodLayerRecursive(node->GetChild(i), layer);
    }
}


int32 EditorScene::GetForceLodLayer(Entity *node)
{
    if(!node)   return -1;

    LodComponent *lc = GetLodComponent(node);
    if(lc)
        return lc->GetForceLodLayer();
    
    int32 count = node->GetChildrenCount();
    for(int32 i = 0; i < count; ++i)
    {
        int32 layer = GetForceLodLayer(node->GetChild(i));
        if(-1 != layer)
            return layer;
    }
    
    return -1;
}

const RenderManager::Stats & EditorScene::GetRenderStats() const
{
    return renderStats;
}



void EditorScene::AddEditorEntity(Entity *editorEntity)
{
    if(GetChildrenCount())
    {
        InsertBeforeNode(editorEntity, GetChild(0));
    }
    else
    {
        AddNode(editorEntity);
    }
}


void EditorScene::UpdateShadowColorFromLandscape()
{
	// try to get shadow color for landscape
	Entity *land = FindLandscapeEntity(this);
	if(!land) return;

	KeyedArchive * props = land->GetCustomProperties();
	if (props->IsKeyExists("ShadowColor"))
	{
		GetRenderSystem()->SetShadowRectColor(props->GetVariant("ShadowColor")->AsColor());
	}
}

void EditorScene::SetShadowColor( const Color &color )
{
	Entity *land = FindLandscapeEntity(this);
	if(!land) return;

	KeyedArchive * props = land->GetCustomProperties();
	if(!props) return;
	
	props->SetVariant("ShadowColor", VariantType(color));

	UpdateShadowColorFromLandscape();
}


Color EditorScene::GetShadowColor()
{
	return GetRenderSystem()->GetShadowRectColor();
}

void EditorScene::SetShadowBlendMode( ShadowVolumeRenderPass::eBlend blend )
{
	ShadowVolumeRenderPass *shadowPass = DynamicTypeCheck<ShadowVolumeRenderPass*>( GetRenderSystem()->GetRenderPass(PASS_SHADOW_VOLUME) );
	shadowPass->SetBlendMode(blend);

}

ShadowVolumeRenderPass::eBlend EditorScene::GetShadowBlendMode()
{
	ShadowVolumeRenderPass *shadowPass = DynamicTypeCheck<ShadowVolumeRenderPass*>( GetRenderSystem()->GetRenderPass(PASS_SHADOW_VOLUME) );
	return shadowPass->GetBlendMode();
}

