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


#ifndef __DAVAENGINE_RENDER_RENDERSYSTEM_H__
#define	__DAVAENGINE_RENDER_RENDERSYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/HashMap.h"
#include "Base/FastNameMap.h"
#include "Entity/SceneSystem.h"
#include "Render/Highlevel/IRenderUpdatable.h"
#include "Render/Highlevel/SpatialTree.h"
#include "Render/RenderHelper.h"

namespace DAVA
{
class RenderPass;
class RenderLayer;
class RenderObject;
class RenderBatch;
class Entity;
class Camera;
class Light;
class ParticleEmitterSystem;
class RenderHierarchy;
class NMaterial;

class RenderSystem
{
public:
    RenderSystem();
    virtual ~RenderSystem();
    
    /**
        \brief Get Render Hierarchy. It allow you to work with current render hierarchy and perform all main tasks with geometry on the level.
     */
    inline RenderHierarchy * GetRenderHierarchy() const;

    /**
        \brief Register render objects for permanent rendering
     */
    void RenderPermanent(RenderObject * renderObject);

    /**
        \brief Unregister render objects for permanent rendering
     */
    void RemoveFromRender(RenderObject * renderObject);
    
    /**
        \brief Register batch
     */
    void RegisterBatch(RenderBatch * batch);
    /**
        \brief Unregister batch
     */
    void UnregisterBatch(RenderBatch * batch);
    
    void RegisterMaterial(NMaterial * material);
    void UnregisterMaterial(NMaterial * material);
    

    /**
        \brief Set main camera
     */
    inline void SetMainCamera(Camera * camera);
	inline Camera * GetMainCamera() const;
    inline void SetDrawCamera(Camera * camera);
	inline Camera * GetDrawCamera() const;
    
    void SetGlobalMaterial(NMaterial *material);
    NMaterial *GetGlobalMaterial() const;
    
    void Update(float32 timeElapsed);
    void Render();
    
    void MarkForUpdate(RenderObject * renderObject);
    void MarkForUpdate(Light * lightNode);
    //void MarkForMaterialSort(Material * material);
    
    /**
        \brief This is required for objects that needs permanent update every frame like 
        Landscape and Particles.
     */
    void RegisterForUpdate(IRenderUpdatable * renderObject);
    void UnregisterFromUpdate(IRenderUpdatable * renderObject);
    
    
    void AddLight(Light * light);
    void RemoveLight(Light * light);
    Vector<Light*> & GetLights();
    void SetForceUpdateLights();
    
	void DebugDrawHierarchy(const Matrix4& cameraMatrix);

    RenderHierarchy * GetRenderHierarchy(){return renderHierarchy;}

    inline bool IsRenderHierarchyInitialized() const {return hierarchyInitialized;}
    
    inline RenderPass* GetMainRenderPass() const;
    
    inline RenderHelper * GetDebugDrawer() const { return debugDrawer; }

private:
	void CreateSpatialTree();
    void ProcessClipping();
    void FindNearestLights();
    void FindNearestLights(RenderObject * renderObject);
    void AddRenderObject(RenderObject * renderObject);
    
    void RemoveRenderObject(RenderObject * renderObject);
    
    Vector<IRenderUpdatable*> objectsForUpdate;
    Vector<RenderObject*> objectsForPermanentUpdate;
    Vector<RenderObject*> markedObjects;
    Vector<Light*> movedLights;
    RenderPass* mainRenderPass;
    
    Vector<RenderObject*> renderObjectArray;	
    Vector<Light*> lights;
    bool forceUpdateLights;
    
    RenderHierarchy * renderHierarchy;
	bool hierarchyInitialized;    
    
    Camera * mainCamera;
    Camera * drawCamera;

    NMaterial *globalMaterial;

    RenderHelper * debugDrawer;

    friend class RenderPass;
};

inline RenderHierarchy * RenderSystem::GetRenderHierarchy() const
{
    return renderHierarchy;
}
    
inline void RenderSystem::SetMainCamera(Camera * _camera)
{
    SafeRelease(mainCamera);
    mainCamera = SafeRetain(_camera);
}

inline void RenderSystem::SetDrawCamera(Camera * _camera)
{
    SafeRelease(drawCamera);
    drawCamera = SafeRetain(_camera);
}

inline Camera * RenderSystem::GetMainCamera() const
{
    return mainCamera;
}

inline Camera * RenderSystem::GetDrawCamera() const
{
    return drawCamera;
}

inline RenderPass* RenderSystem::GetMainRenderPass() const
{
    return mainRenderPass;
}
    
} // ns

#endif	/* __DAVAENGINE_RENDER_RENDERSYSTEM_H__ */

