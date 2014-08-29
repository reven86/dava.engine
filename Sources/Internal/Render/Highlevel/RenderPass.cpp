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


#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/RenderFastNames.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/RenderBatchArray.h"
#include "Render/Highlevel/Camera.h"
#include "Render/ShaderCache.h"

#include "Render/Image/ImageSystem.h"

#include "Render/RenderTarget/RenderTargetFactory.h"

namespace DAVA
{
    
RenderPass::RenderPass(const FastName & _name, RenderPassID _id)    
    :   name(_name)
    ,   id(_id)
{
    renderPassBatchArray = new RenderPassBatchArray();
    renderLayers.reserve(RENDER_LAYER_ID_COUNT);
}

RenderPass::~RenderPass()
{
    SafeDelete(renderPassBatchArray);
}
    
void RenderPass::AddRenderLayer(RenderLayer * layer, const FastName & afterLayer)
{
	if(LAST_LAYER != afterLayer)
	{
		uint32 size = renderLayers.size();
		for(uint32 i = 0; i < size; ++i)
		{
			const FastName & name = renderLayers[i]->GetName();
			if(afterLayer == name)
			{
				renderLayers.insert(renderLayers.begin() +i+1, layer);
				return;
			}
		}
		DVASSERT(0 && "RenderPass::AddRenderLayer afterLayer not found");
	}
	else
	{
		renderLayers.push_back(layer);
	}
}
    
void RenderPass::RemoveRenderLayer(RenderLayer * layer)
{
	Vector<RenderLayer*>::iterator it = std::find(renderLayers.begin(), renderLayers.end(), layer);
	DVASSERT(it != renderLayers.end());

	renderLayers.erase(it);
}

void RenderPass::Draw(RenderSystem * renderSystem, uint32 clearBuffers)
{   
    Camera *mainCamera = renderSystem->GetMainCamera();        
    Camera *drawCamera = renderSystem->GetDrawCamera();   
    
    DVASSERT(drawCamera);
    DVASSERT(mainCamera);
    drawCamera->SetupDynamicParameters();            
    if (mainCamera!=drawCamera)    
        mainCamera->PrepareDynamicParameters();
    
    PrepareVisibilityArrays(mainCamera, renderSystem);
    
    ClearBuffers(clearBuffers);

    DrawLayers(mainCamera);
}

void RenderPass::PrepareVisibilityArrays(Camera *camera, RenderSystem * renderSystem)
{
    uint32 currVisibilityCriteria = RenderObject::CLIPPING_VISIBILITY_CRITERIA;
    if (!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::ENABLE_STATIC_OCCLUSION))
        currVisibilityCriteria&=~RenderObject::VISIBLE_STATIC_OCCLUSION;
    visibilityArray.Clear();
    renderSystem->GetRenderHierarchy()->Clip(camera, &visibilityArray, currVisibilityCriteria);    
    renderPassBatchArray->Clear();
    renderPassBatchArray->PrepareVisibilityArray(&visibilityArray, camera); 
}

void RenderPass::DrawLayers(Camera *camera)
{    
    ShaderCache::Instance()->ClearAllLastBindedCaches();

    // Draw all layers with their materials
    uint32 size = (uint32)renderLayers.size();
    for (uint32 k = 0; k < size; ++k)
    {
        RenderLayer * layer = renderLayers[k];
        RenderLayerBatchArray * renderLayerBatchArray = renderPassBatchArray->Get(layer->GetRenderLayerID());
        if (renderLayerBatchArray)
        {
            layer->Draw(name, camera, renderLayerBatchArray); //why camera here?
        }
    }
}

void RenderPass::ClearBuffers(uint32 clearBuffers)
{
    RenderManager::Instance()->SetRenderState(RenderState::RENDERSTATE_3D_BLEND);
    RenderManager::Instance()->FlushState();    
    if (clearBuffers == RenderManager::ALL_BUFFERS)
        RenderManager::Instance()->Clear(Color(0,0,0,0), 1.0f, 0);
    else
    {
        if (clearBuffers&RenderManager::COLOR_BUFFER)
            RenderManager::Instance()->ClearWithColor(0,0,0,0);
        if (clearBuffers&RenderManager::DEPTH_BUFFER)
            RenderManager::Instance()->ClearDepthBuffer();
        if (clearBuffers&RenderManager::STENCIL_BUFFER)
            RenderManager::Instance()->ClearStencilBuffer();
    }
}


MainForwardRenderPass::MainForwardRenderPass(const FastName & name, RenderPassID id):RenderPass(name, id),
    reflectionTarget(NULL),
    refractionTarget(NULL),
    reflectionPass(NULL), 
    refractionPass(NULL), 
    needWaterPrepass(false)
{
    const RenderLayerManager * renderLayerManager = RenderLayerManager::Instance();
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_OPAQUE), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_AFTER_OPAQUE), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_VEGETATION), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_ALPHA_TEST_LAYER), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_SHADOW_VOLUME), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_WATER), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_TRANSLUCENT), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_AFTER_TRANSLUCENT), LAST_LAYER);    
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_DEBUG_DRAW), LAST_LAYER);
}


void MainForwardRenderPass::PrepareReflectionRefractionTextures(RenderSystem * renderSystem)
{
    if (!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::WATER_REFLECTION_REFRACTION_DRAW))
        return;

    RenderLayerBatchArray *waterLayer = renderPassBatchArray->Get(RenderLayerManager::Instance()->GetLayerIDByName(LAYER_WATER));
    uint32 waterBatchesCount = waterLayer->GetRenderBatchCount();

    const static uint32 REFLECTION_TEX_SIZE = 512;
    const static uint32 REFRACTION_TEX_SIZE = 512;
    if (!reflectionPass)
    {
        FramebufferDescriptor colorDesc;
        colorDesc.SetFramebufferType(FramebufferDescriptor::FRAMEBUFFER_COLOR0);
        colorDesc.SetFramebufferFormat(FramebufferDescriptor::FORMAT_RGB565);
        colorDesc.SetFramebufferHeight(REFLECTION_TEX_SIZE);
        colorDesc.SetFramebufferWidth(REFLECTION_TEX_SIZE);
        colorDesc.SetPreRenderAction(FramebufferDescriptor::PRE_ACTION_DONTCARE);
        colorDesc.SetPostRenderAction(FramebufferDescriptor::POST_ACTION_RESOLVE);

        RenderTextureDescriptor colorTextureDesc(Texture::TEXTURE_2D,
                                                 Texture::WRAP_CLAMP_TO_EDGE,
                                                 Texture::WRAP_CLAMP_TO_EDGE,
                                                 Texture::FILTER_LINEAR_MIPMAP_LINEAR,
                                                 Texture::FILTER_LINEAR_MIPMAP_LINEAR);

        FramebufferDescriptor depthDesc;
        depthDesc.SetFramebufferType(FramebufferDescriptor::FRAMEBUFFER_DEPTH);
        depthDesc.SetFramebufferFormat(FramebufferDescriptor::FORMAT_DEPTH24);
        depthDesc.SetFramebufferHeight(REFLECTION_TEX_SIZE);
        depthDesc.SetFramebufferWidth(REFLECTION_TEX_SIZE);
        depthDesc.SetPreRenderAction(FramebufferDescriptor::PRE_ACTION_DONTCARE);
        depthDesc.SetPostRenderAction(FramebufferDescriptor::POST_ACTION_DONTCARE);

        FramebufferDescriptor stencilDesc;
        stencilDesc.SetFramebufferType(FramebufferDescriptor::FRAMEBUFFER_STENCIL);
        stencilDesc.SetFramebufferFormat(FramebufferDescriptor::FORMAT_STENCIL8);
        stencilDesc.SetFramebufferHeight(REFLECTION_TEX_SIZE);
        stencilDesc.SetFramebufferWidth(REFLECTION_TEX_SIZE);
        stencilDesc.SetPreRenderAction(FramebufferDescriptor::PRE_ACTION_DONTCARE);
        stencilDesc.SetPostRenderAction(FramebufferDescriptor::POST_ACTION_DONTCARE);

        RenderTargetDescriptor rtDesc;
        rtDesc.SetClearColor(Color(0.0f, 0.0f, 0.0f, 0.0f));
        rtDesc.SetClearDepth(1.0f);
        rtDesc.SetClearStencil(0);

        rtDesc.AddFramebuffer(colorDesc, colorTextureDesc);
        rtDesc.AddFramebuffer(depthDesc);
        rtDesc.AddFramebuffer(stencilDesc);

        reflectionPass = new WaterReflectionRenderPass(PASS_FORWARD, RENDER_PASS_WATER_REFLECTION);
        reflectionTarget = RenderTargetFactory::Instance()->CreateRenderTarget(rtDesc);

        colorDesc.SetFramebufferHeight(REFRACTION_TEX_SIZE);
        colorDesc.SetFramebufferWidth(REFRACTION_TEX_SIZE);
        depthDesc.SetFramebufferHeight(REFRACTION_TEX_SIZE);
        depthDesc.SetFramebufferWidth(REFRACTION_TEX_SIZE);
        stencilDesc.SetFramebufferHeight(REFRACTION_TEX_SIZE);
        stencilDesc.SetFramebufferWidth(REFRACTION_TEX_SIZE);

        refractionPass = new WaterRefractionRenderPass(PASS_FORWARD, RENDER_PASS_WATER_REFRACTION);
        refractionTarget = RenderTargetFactory::Instance()->CreateRenderTarget(rtDesc);
    }   

    reflectionTarget->BeginRender();
    //discard everything here

    reflectionPass->SetWaterLevel(waterBox.max.z);
    reflectionPass->Draw(renderSystem, RenderManager::ALL_BUFFERS);

        
    //discrad depth(everything?) here
    reflectionTarget->EndRender();
        

    refractionTarget->BeginRender();

    refractionPass->SetWaterLevel(waterBox.min.z);
    refractionPass->Draw(renderSystem, RenderManager::ALL_BUFFERS);

    //discrad depth(everything?) here
    refractionTarget->EndRender();

    renderSystem->GetDrawCamera()->SetupDynamicParameters();    		

    Rect viewportSave = RenderManager::Instance()->GetViewport();
    Vector2 rssVal(1.0f/viewportSave.dx, 1.0f/viewportSave.dy);
    Vector2 screenOffsetVal(viewportSave.x, viewportSave.y);

    Texture* reflectionTexture = reflectionTarget->GetColorAttachment()->Lock();
    Texture* refractionTexture = refractionTarget->GetColorAttachment()->Lock();

	for (uint32 i=0; i<waterBatchesCount; ++i)
	{
        NMaterial *mat = waterLayer->Get(i)->GetMaterial();
        mat->SetPropertyValue(NMaterial::PARAM_RCP_SCREEN_SIZE, Shader::UT_FLOAT_VEC2, 1, &rssVal);
        mat->SetPropertyValue(NMaterial::PARAM_SCREEN_OFFSET, Shader::UT_FLOAT_VEC2, 1, &screenOffsetVal);
        mat->SetTexture(NMaterial::TEXTURE_DYNAMIC_REFLECTION, reflectionTexture);
        mat->SetTexture(NMaterial::TEXTURE_DYNAMIC_REFRACTION, refractionTexture);
	}

    reflectionTarget->GetColorAttachment()->Unlock(reflectionTexture);
    refractionTarget->GetColorAttachment()->Unlock(refractionTexture);
}

void MainForwardRenderPass::Draw(RenderSystem * renderSystem, uint32 clearBuffers)
{
    Camera *mainCamera = renderSystem->GetMainCamera();        
    Camera *drawCamera = renderSystem->GetDrawCamera();   
    DVASSERT(mainCamera);
    DVASSERT(drawCamera);
    drawCamera->SetupDynamicParameters();            
    if (mainCamera!=drawCamera)    
        mainCamera->PrepareDynamicParameters();

    if (needWaterPrepass)
    {
        /*water presence is cached from previous frame in optimization purpose*/
        /*if on previous frame there was water - reflection and refraction textures are rendered first (it helps to avoid excessive renderPassBatchArray->PrepareVisibilityArray)*/
        /* if there was no water on previous frame, and it appears on this frame - reflection and refractions textures are still to be rendered*/
        PrepareReflectionRefractionTextures(renderSystem);
    }
    
	//important: FoliageSystem also using main camera for cliping vegetation cells
    PrepareVisibilityArrays(mainCamera, renderSystem);
	
	RenderLayerBatchArray *waterLayer = renderPassBatchArray->Get(RenderLayerManager::Instance()->GetLayerIDByName(LAYER_WATER));
	
	uint32 waterBatchesCount = waterLayer->GetRenderBatchCount();	
	if (waterBatchesCount)
	{        
        waterBox.Empty();
		for (uint32 i=0; i<waterBatchesCount; ++i)
		{
			RenderBatch *batch = waterLayer->Get(i);						
			waterBox.AddAABBox(batch->GetRenderObject()->GetWorldBoundingBox());
			
		}
	}    
    
	if (!needWaterPrepass&&waterBatchesCount)
	{
        PrepareReflectionRefractionTextures(renderSystem); 
        /*as PrepareReflectionRefractionTextures builds render batches according to reflection/refraction camera - render batches in main pass list are not valid anymore*/
        /*to avoid this happening every frame water visibility is cached from previous frame (needWaterPrepass)*/
        /*however if there was no water on previous frame and there is water on this frame visibilityArray should be re-prepared*/
        renderPassBatchArray->Clear();
        renderPassBatchArray->PrepareVisibilityArray(&visibilityArray, mainCamera);
                
	}	
    needWaterPrepass = (waterBatchesCount!=0); //for next frame;

    ClearBuffers(clearBuffers);

	DrawLayers(mainCamera);   
}

MainForwardRenderPass::~MainForwardRenderPass()
{	
	SafeRelease(reflectionTarget);
	SafeRelease(refractionTarget);
	SafeDelete(reflectionPass);
	SafeDelete(refractionPass);
}

WaterPrePass::WaterPrePass(const FastName & name, RenderPassID id):RenderPass(name, id), passMainCamera(NULL), passDrawCamera(NULL)
{
    const RenderLayerManager * renderLayerManager = RenderLayerManager::Instance();
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_OPAQUE), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_AFTER_OPAQUE), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_ALPHA_TEST_LAYER), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_TRANSLUCENT), LAST_LAYER);    
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_AFTER_TRANSLUCENT), LAST_LAYER);
    
}
WaterPrePass::~WaterPrePass()
{
    SafeRelease(passMainCamera);
    SafeRelease(passDrawCamera);
}

WaterReflectionRenderPass::WaterReflectionRenderPass(const FastName & name, RenderPassID id):WaterPrePass(name, id)
{	
}

void WaterReflectionRenderPass::UpdateCamera(Camera *camera)
{
    Vector3 v;
    v = camera->GetPosition();
    v.z = waterLevel - (v.z - waterLevel);
    camera->SetPosition(v);
    v = camera->GetTarget();
    v.z = waterLevel - (v.z - waterLevel);
    camera->SetTarget(v);        
}

void WaterReflectionRenderPass::Draw(RenderSystem * renderSystem, uint32 clearBuffers)
{    
    Camera *mainCamera = renderSystem->GetMainCamera();        
    Camera *drawCamera = renderSystem->GetDrawCamera();    
            

    if (!passDrawCamera)
    {
        passMainCamera = new Camera();    
        passDrawCamera = new Camera();            
    }

    passMainCamera->CopyMathOnly(*mainCamera);        
    UpdateCamera(passMainCamera);

    Vector4 clipPlane(0,0,1, -(waterLevel-0.1f));

    Camera* currMainCamera = passMainCamera;
    Camera* currDrawCamera;
    
    if (drawCamera==mainCamera)
    {
        currDrawCamera = currMainCamera;    
    }
    else
    {
        passDrawCamera->CopyMathOnly(*drawCamera);        
        UpdateCamera(passDrawCamera);
        currDrawCamera = passDrawCamera;
        currMainCamera->PrepareDynamicParameters(&clipPlane);
    }
    currDrawCamera->SetupDynamicParameters(&clipPlane);
    
    //add clipping plane
    
    
    
	visibilityArray.Clear();
	renderSystem->GetRenderHierarchy()->Clip(currMainCamera, &visibilityArray, RenderObject::CLIPPING_VISIBILITY_CRITERIA | RenderObject::VISIBLE_REFLECTION);	
	renderPassBatchArray->Clear();
	renderPassBatchArray->PrepareVisibilityArray(&visibilityArray, currMainCamera); 

    ClearBuffers(clearBuffers);

    DrawLayers(currMainCamera);
}


WaterRefractionRenderPass::WaterRefractionRenderPass(const FastName & name, RenderPassID id) : WaterPrePass(name, id)
{
    /*const RenderLayerManager * renderLayerManager = RenderLayerManager::Instance();
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_SHADOW_VOLUME), LAST_LAYER);*/
}

void WaterRefractionRenderPass::Draw(RenderSystem * renderSystem, uint32 clearBuffers)
{
    Camera *mainCamera = renderSystem->GetMainCamera();        
    Camera *drawCamera = renderSystem->GetDrawCamera();    


    if (!passDrawCamera)
    {
        passMainCamera = new Camera();    
        passDrawCamera = new Camera();            
    }

    passMainCamera->CopyMathOnly(*mainCamera);                    

    Vector4 clipPlane(0,0,-1, waterLevel+0.1f);

    Camera* currMainCamera = passMainCamera;
    Camera* currDrawCamera;

    if (drawCamera==mainCamera)
    {
        currDrawCamera = currMainCamera;    
    }
    else
    {
        passDrawCamera->CopyMathOnly(*drawCamera);                
        currDrawCamera = passDrawCamera;
        currMainCamera->PrepareDynamicParameters(&clipPlane);
    }
    currDrawCamera->SetupDynamicParameters(&clipPlane);

    //add clipping plane



    visibilityArray.Clear();
    renderSystem->GetRenderHierarchy()->Clip(currMainCamera, &visibilityArray, RenderObject::CLIPPING_VISIBILITY_CRITERIA | RenderObject::VISIBLE_REFRACTION);	
    renderPassBatchArray->Clear();
    renderPassBatchArray->PrepareVisibilityArray(&visibilityArray, currMainCamera); 

    ClearBuffers(clearBuffers);

    DrawLayers(currMainCamera);       
    
}


};
