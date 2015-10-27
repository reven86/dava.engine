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


#ifndef __DAVAENGINE_SCENE3D_RENDER_PASS_H__
#define	__DAVAENGINE_SCENE3D_RENDER_PASS_H__

#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/RenderPassNames.h"

namespace DAVA
{
class Camera;
class RenderPass
{
public:
    RenderPass(const FastName& name);
    virtual ~RenderPass();
    
    inline const FastName & GetName() const;

    void AddRenderLayer(RenderLayer* layer, RenderLayer::eRenderLayerID afterLayer = RenderLayer::RENDER_LAYER_INVALID_ID);
    void RemoveRenderLayer(RenderLayer * layer);

    virtual void Draw(RenderSystem* renderSystem);

    inline uint32 GetRenderLayerCount() const;
    inline RenderLayer * GetRenderLayer(uint32 index) const;

    inline rhi::RenderPassConfig& GetPassConfig();
    inline void SetViewport(const Rect& viewPort);

protected:
    FastName passName;
    rhi::RenderPassConfig passConfig;
    Rect viewport;

    Vector2 viewportSize, rcpViewportSize, viewportOffset; //storage fro dynamic bindings

    /*convinience*/
    void PrepareVisibilityArrays(Camera *camera, RenderSystem * renderSystem);
    void PrepareLayersArrays(const Vector<RenderObject*> objectsArray, Camera* camera);
    void ClearLayersArrays();

    void SetupCameraParams(Camera* mainCamera, Camera* drawCamera, Vector4* externalClipPlane = NULL);
    void DrawLayers(Camera *camera);
    void DrawDebug(Camera* camera, RenderSystem* renderSystem);

    void BeginRenderPass();
    void EndRenderPass();

    Vector<RenderLayer*> renderLayers;
    std::array<RenderBatchArray, RenderLayer::RENDER_LAYER_ID_COUNT> layersBatchArrays;
    Vector<RenderObject*> visibilityArray;

    rhi::HPacketList packetList;
    rhi::HRenderPass renderPass;

public:
    INTROSPECTION(RenderPass,
                  COLLECTION(renderLayers, "Render Layers", I_VIEW | I_EDIT)
                  MEMBER(passName, "Name", I_VIEW));

    friend class RenderSystem;
};

inline rhi::RenderPassConfig& RenderPass::GetPassConfig()
{
    return passConfig;
}
inline void RenderPass::SetViewport(const Rect& _viewport)
{
    viewport = _viewport;
    passConfig.viewport.x = (int32)viewport.x;
    passConfig.viewport.y = (int32)viewport.y;
    passConfig.viewport.width = (int32)viewport.dx;
    passConfig.viewport.height = (int32)viewport.dy;
}

inline const FastName & RenderPass::GetName() const
{
    return passName;
}

inline uint32 RenderPass::GetRenderLayerCount() const
{
    return (uint32)renderLayers.size();
}
    
inline RenderLayer * RenderPass::GetRenderLayer(uint32 index) const
{
    return renderLayers[index];
}

class WaterPrePass : public RenderPass
{    
public:
    inline void SetWaterLevel(float32 level){waterLevel = level;}
    WaterPrePass(const FastName& name);
    ~WaterPrePass();
protected:
    Camera *passMainCamera, *passDrawCamera;
    float32 waterLevel;
};
class WaterReflectionRenderPass  : public WaterPrePass
{        
public:
    WaterReflectionRenderPass(const FastName& name);
    virtual void Draw(RenderSystem* renderSystem);

private:
    void UpdateCamera(Camera *camera);
};

class WaterRefractionRenderPass  : public WaterPrePass
{       
public:
    WaterRefractionRenderPass(const FastName& name);
    virtual void Draw(RenderSystem* renderSystem);
};

class MainForwardRenderPass : public RenderPass
{	

public:
    MainForwardRenderPass(const FastName& name);
    ~MainForwardRenderPass();
    virtual void Draw(RenderSystem* renderSystem);

private:
	WaterReflectionRenderPass *reflectionPass;
    WaterRefractionRenderPass* refractionPass;

    AABBox3 waterBox;

    void InitReflectionRefraction();
    void PrepareReflectionRefractionTextures(RenderSystem * renderSystem);
};

} // ns

#endif	/* __DAVAENGINE_SCENE3D_RENDERLAYER_H__ */

