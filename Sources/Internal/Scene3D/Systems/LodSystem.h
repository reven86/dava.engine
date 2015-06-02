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


#ifndef __DAVAENGINE_SCENE3D_LODSYSTEM_H__
#define __DAVAENGINE_SCENE3D_LODSYSTEM_H__

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/LodComponent.h"
#include "Render/Highlevel/RenderObject.h"


namespace DAVA
{

class Camera;
class LodComponent;

class LodSystem : public SceneSystem
{
public:
	LodSystem(Scene * scene);

	virtual void Process(float32 timeElapsed);
	virtual void AddEntity(Entity * entity);
	virtual void RemoveEntity(Entity * entity);

	virtual void SetCamera(Camera * camera);
	inline Camera* GetCamera() const;
	
	inline void SetForceUpdateAll();

	static void UpdateEntitiesAfterLoad(Entity * entity);
	static void UpdateEntityAfterLoad(Entity * entity);

	static void MergeChildLods(Entity * toEntity);
	
	static void ForceUpdate(Entity* entity, Camera* camera, float32 timeElapsed);

	class LodMerger
	{
	public:
		LodMerger(Entity * toEntity);
		void MergeChildLods();

	private:
		void GetLodComponentsRecursive(Entity * fromEntity, Vector<Entity*> & allLods);
		Entity * toEntity;
	};

	

private:
	//partial update per frame
	static const int32 UPDATE_PART_PER_FRAME = 10;
	Vector<int32> partialUpdateIndices;
	int32 currentPartialUpdateIndex;
	void UpdatePartialUpdateIndices();
	bool forceUpdateAll;
	
	
	Vector<Entity*> entities;

	static void UpdateLod(Entity * entity, LodComponent* lodComponent, float32 psLodOffsetSq, float32 psLodMultSq, Camera* camera);
	static bool RecheckLod(Entity * entity, LodComponent* lodComponent, float32 psLodOffsetSq, float32 psLodMultSq, Camera* camera);

	static float32 CalculateDistanceToCamera(const Entity * entity, const LodComponent *lodComponent, Camera* camera);
	static int32 FindProperLayer(float32 distance, const LodComponent *lodComponent, int32 requestedLayersCount);
	
	static inline void ProcessEntity(Entity * entity, float32 psLodOffsetSq, float32 psLodMultSq, Camera* camera);
	static inline void PorcessEntityRecursive(Entity * entity, float32 psLodOffsetSq, float32 psLodMultSq, Camera* camera);
    
    static void SetEntityLodRecursive(Entity * entity, int32 currentLod);
    static void SetEntityLod(Entity * entity, int32 currentLod);

	Camera * camera;
};
	
	
void LodSystem::ProcessEntity(Entity * entity, float32 psLodOffsetSq, float32 psLodMultSq, Camera* camera)
{
	LodComponent * lod = GetLodComponent(entity);
	if(lod->flags & LodComponent::NEED_UPDATE_AFTER_LOAD)
	{
		UpdateEntityAfterLoad(entity);
	}
	
	UpdateLod(entity, lod, psLodOffsetSq, psLodMultSq, camera);
}
	
void LodSystem::SetForceUpdateAll()
{
	forceUpdateAll = true;
}
	
Camera* LodSystem::GetCamera() const
{
	return camera;
}
    
inline void LodSystem::SetEntityLod(Entity * entity, int32 currentLod)
{
    RenderObject * ro = GetRenderObject(entity);
    if(ro)
    {
        ro->SetLodIndex(currentLod);
    }
}
	
}

#endif //__DAVAENGINE_SCENE3D_LODSYSTEM_H__