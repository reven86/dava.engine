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

namespace DAVA
{

class Camera;
class LodComponent;

class LodSystem : public SceneSystem
{
public:
	LodSystem(Scene * scene);

	virtual void Process();
	virtual void AddEntity(Entity * entity);
	virtual void RemoveEntity(Entity * entity);

	virtual void SetCamera(Camera * camera);

	void UpdateEntitiesAfterLoad();
	static void UpdateEntityAfterLoad(Entity * entity);
	static void MergeChildLods(Entity * toEntity);

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
	static const int32 UPDATE_PART_PER_FRAME = 1;
	Vector<int32> partialUpdateIndices;
	int32 currentPartialUpdateIndex;
	void UpdatePartialUpdateIndices();

	
	Vector<Entity*> entities;

	void UpdateLod(Entity * entity, LodComponent* lodComponent, float32 psLodOffsetSq, float32 psLodMultSq);
	bool RecheckLod(Entity * entity, LodComponent* lodComponent, float32 psLodOffsetSq, float32 psLodMultSq);

	float32 CalculateDistanceToCamera(const Entity * entity, const LodComponent *lodComponent) const;
	int32 FindProperLayer(float32 distance, const LodComponent *lodComponent, int32 requestedLayersCount);

	Camera * camera;
};

}

#endif //__DAVAENGINE_SCENE3D_LODSYSTEM_H__