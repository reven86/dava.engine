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


#ifndef __DAVAENGINE_SCENE3D_STATIC_OCCLUSION_BUILD_SYSTEM_H__
#define	__DAVAENGINE_SCENE3D_STATIC_OCCLUSION_BUILD_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"
#include "Base/Message.h"

namespace DAVA
{
class Camera;
class Landscape;
class RenderObject;
class StaticOcclusion;
class StaticOcclusionComponent;
class StaticOcclusionData;
class StaticOcclusionDataComponent;
class StaticOcclusionDebugDrawComponent;
class NMaterial;

// System that allow to build occlusion information. Required only in editor.
class StaticOcclusionBuildSystem : public SceneSystem
{
public:
    StaticOcclusionBuildSystem(Scene * scene);
    virtual ~StaticOcclusionBuildSystem();
    
    virtual void AddEntity(Entity * entity);
    virtual void RemoveEntity(Entity * entity);
    virtual void Process(float32 timeElapsed);
    void ImmediateEvent(Component * component, uint32 event) override;
    
    inline void SetCamera(Camera * camera);

    void Build();
    void Cancel();

    bool IsInBuild() const;
    uint32 GetBuildStatus() const;
    const String& GetBuildStatusInfo() const;

private:
    void PrepareRenderObjects();
    void StartBuildOcclusion();    
    void FinishBuildOcclusion();

    void StartOcclusionComponent();
    void FinishOcclusionComponent();

    
    void SceneForceLod(int32 layerIndex);
    void CollectEntitiesForOcclusionRecursively(Vector<Entity*>& dest, Entity *entity);

    Camera* camera = nullptr;
    Landscape* landscape = nullptr;
    Vector<Entity*> occlusionEntities;
    StaticOcclusion* staticOcclusion = nullptr;
    StaticOcclusionDataComponent* componentInProgress = nullptr;
    uint32 activeIndex = -1;
    uint32 objectsCount = 0;
};
   
    
inline void StaticOcclusionBuildSystem::SetCamera(Camera * _camera)
{
    camera = _camera;
}

} // ns

#endif	/* __DAVAENGINE_SCENE3D_STATIC_OCCLUSION_SYSTEM_H__ */

