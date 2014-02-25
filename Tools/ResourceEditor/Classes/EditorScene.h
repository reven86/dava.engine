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




#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "DAVAEngine.h"
#include "bullet/btBulletDynamicsCommon.h"
#include "BulletObject.h"

#include "Scene/System/EditorLightSystem.h"
#include "Render/Highlevel/ShadowVolumeRenderPass.h"

using namespace DAVA;

static const FastName LAYER_ARROWS("ArrowsRenderLayer");

class HeightmapNode;
class EditorScene : public Scene
{
public:
	
    EditorScene();
    ~EditorScene();
    
    virtual void Update(float32 timeElapsed);
	virtual void RemoveNode(Entity * node);
	void UpdateBullet(Entity * curr);
	void RemoveBullet(Entity * curr);

    btCollisionWorld *collisionWorld;
	btCollisionWorld *landCollisionWorld;
	void CheckNodes(Entity * curr);
	
	void TrySelection(Vector3 from, Vector3 direction);
	bool TryIsTargetAccesible(Vector3 from, Vector3 target);
	void JuncCollWorldToLandscapeCollWorld();
	void SeparateCollWorldFromLandscapeCollWorld();
    bool LandscapeIntersection(const Vector3 &from, const Vector3 &direction, Vector3 &point); 

	inline Entity * GetSelection()
	{
		return selection;
	}

	inline Entity * GetProxy()
	{
		return proxy;
	}
	
	
	void SetSelection(Entity *newSelection);
    
	virtual void Draw();
	void DrawGrid();
	void SetBulletUpdate(Entity* curr, bool value);
	void ReleaseUserData(Entity * curr);
    
    void SetDrawGrid(bool newDrawGrid);
	
    void SetForceLodLayer(Entity *node, int32 layer);
    int32 GetForceLodLayer(Entity *node);
    
    const RenderManager::Stats & GetRenderStats() const;
    
	void AddEditorEntity(Entity *editorEntity);

	void SetShadowColor(const Color &color);
	Color GetShadowColor();
	void UpdateShadowColorFromLandscape();

	void SetShadowBlendMode(ShadowVolumeRenderPass::eBlend blend);
	ShadowVolumeRenderPass::eBlend GetShadowBlendMode();

protected:
    void SetForceLodLayerRecursive(Entity *node, int32 layer);
    
    btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btAxisSweep3* broadphase;
    int depth;

	btDefaultCollisionConfiguration* landCollisionConfiguration;
	btCollisionDispatcher* landDispatcher;
	btAxisSweep3* landBroadphase;

	Entity * selection;
	Entity * proxy;
    
    Entity *selectedEntity;
	
	Entity * FindSelected(Entity * curr, btCollisionObject * coll);
	HeightmapNode * FindHeightmap(Entity * curr, btCollisionObject * coll);
	Entity * lastSelectedPhysics;
    bool drawGrid;

	btBroadphaseProxy* originalHandler;
    
    RenderManager::Stats renderStats;
    
public:

    EditorLightSystem *editorLightSystem;

};

#endif