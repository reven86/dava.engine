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


#ifndef __DAVAENGINE_SCENE3D_STATIC_OCCLUSION_SYSTEM_H__
#define	__DAVAENGINE_SCENE3D_STATIC_OCCLUSION_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"
#include "Base/Message.h"

namespace DAVA
{
class Camera;
class RenderObject;
class StaticOcclusionComponent;
class StaticOcclusionData;
class StaticOcclusionDataComponent;
class StaticOcclusionDebugDrawComponent;
class NMaterial;
class PolygonGroup;
    
// System that allow to use occlusion information during rendering
class StaticOcclusionSystem : public SceneSystem
{
public:
    StaticOcclusionSystem(Scene * scene);
    virtual ~StaticOcclusionSystem();
    
    inline void SetCamera(Camera * camera);
    
    virtual void RegisterEntity(Entity *entity);
    virtual void UnregisterEntity(Entity *entity);
    virtual void RegisterComponent(Entity *entity, Component * component);
    virtual void UnregisterComponent(Entity *entity, Component * component);
    
    virtual void AddEntity(Entity * entity);
    virtual void RemoveEntity(Entity * entity);
    virtual void Process(float32 timeElapsed);
    
    void AddRenderObjectToOcclusion(RenderObject * renderObject);
    void RemoveRenderObjectFromOcclusion(RenderObject * renderObject);

    void ClearOcclusionObjects();
    void CollectOcclusionObjectsRecursively(Entity *entity);

    void InvalidateOcclusion();
    void InvalidateOcclusionIndicesRecursively(Entity *entity);

private:
    // Final system part
    void ProcessStaticOcclusionForOneDataSet(uint32 blockIndex, StaticOcclusionData * data);
    void UndoOcclusionVisibility();

private:
    Camera* camera = nullptr;
    StaticOcclusionData* activePVSSet = nullptr;
    uint32 activeBlockIndex = 0;
    Vector<StaticOcclusionDataComponent*> staticOcclusionComponents;
    Vector<RenderObject*> indexedRenderObjects;
    bool isInPvs = false;
};


class StaticOcclusionDebugDrawSystem : public SceneSystem
{
public:
    StaticOcclusionDebugDrawSystem(Scene *scene);    

    virtual void AddEntity(Entity * entity);
    virtual void RemoveEntity(Entity * entity);
    void ImmediateEvent(Component * component, uint32 event) override;

    /*HVertexBuffer CreateStaticOcclusionDebugDrawGrid(const AABBox3& boundingBox, uint32 xSubdivisions, uint32 ySubdivisions, uint32 zSubdivisions, const float32 *cellHeightOffset);
    PolygonGroup* CreateStaticOcclusionDebugDrawCover(const AABBox3& boundingBox, uint32 xSubdivisions, uint32 ySubdivisions, uint32 zSubdivisions, PolygonGroup *gridPolygonGroup);*/

    ~StaticOcclusionDebugDrawSystem();    
private:
    void UpdateGeometry(StaticOcclusionDebugDrawComponent* component);

    void CreateStaticOcclusionDebugDrawVertices(StaticOcclusionDebugDrawComponent* target, StaticOcclusionComponent* source);
    void CreateStaticOcclusionDebugDrawGridIndice(StaticOcclusionDebugDrawComponent* target, StaticOcclusionComponent* source);
    void CreateStaticOcclusionDebugDrawCoverIndice(StaticOcclusionDebugDrawComponent* target, StaticOcclusionComponent* source);

    NMaterial *gridMaterial, *coverMaterial;
    uint32 vertexLayoutId;
};
    

inline void StaticOcclusionSystem::SetCamera(Camera * _camera)
{
    camera = _camera;
}

} // ns

#endif	/* __DAVAENGINE_SCENE3D_STATIC_OCCLUSION_SYSTEM_H__ */

