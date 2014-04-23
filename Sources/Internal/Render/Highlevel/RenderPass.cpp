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
#include "Render/ImageLoader.h"

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

void RenderPass::Draw(Camera * camera, RenderSystem * renderSystem)
{    
    PrepareVisibilityArrays(camera, renderSystem);
    DrawLayers(camera);
}

void RenderPass::PrepareVisibilityArrays(Camera *camera, RenderSystem * renderSystem)
{
    visibilityArray.Clear();
    renderSystem->GetRenderHierarchy()->Clip(camera, &visibilityArray, RenderObject::CLIPPING_VISIBILITY_CRITERIA);
    renderPassBatchArray->Clear();
    renderPassBatchArray->PrepareVisibilityArray(&visibilityArray, camera); 
}

void RenderPass::DrawLayers(Camera *camera)
{
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


MainForwardRenderPass::MainForwardRenderPass(const FastName & name, RenderPassID id):RenderPass(name, id),
    reflectionTexture(NULL), reflectionSprite(NULL), refractionTexture(NULL), refractionSprite(NULL),
    reflectionPass(NULL), refractionPass(NULL)
{
    const RenderLayerManager * renderLayerManager = RenderLayerManager::Instance();
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_OPAQUE), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_AFTER_OPAQUE), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_ALPHA_TEST_LAYER), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_SHADOW_VOLUME), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_WATER), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_TRANSLUCENT), LAST_LAYER);
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_AFTER_TRANSLUCENT), LAST_LAYER);    
}

void MainForwardRenderPass::Draw(Camera * camera, RenderSystem * renderSystem)
{
	PrepareVisibilityArrays(camera, renderSystem);
	
	RenderLayerBatchArray *waterLayer = renderPassBatchArray->Get(RenderLayerManager::Instance()->GetLayerIDByName(LAYER_WATER));
	bool needWaterPrepass = false;
	uint32 waterBatchesCount = waterLayer->GetRenderBatchCount();
	AABBox3 waterBox;
	if (waterBatchesCount)
	{
        needWaterPrepass = true;
		for (uint32 i=0; i<waterBatchesCount; ++i)
		{
			RenderBatch *batch = waterLayer->Get(i);						
			waterBox.AddAABBox(batch->GetRenderObject()->GetWorldBoundingBox());
			
		}
	}    
    
	if (needWaterPrepass&&RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::WATER_REFLECTION_REFRACTION_DRAW))
	{
        const static int32 REFLECTION_TEX_SIZE = 512;
        const static int32 REFRACTION_TEX_SIZE = 512;
        if (!reflectionPass)
        {             
            reflectionPass = new WaterReflectionRenderPass(PASS_FORWARD, RENDER_PASS_WATER_REFLECTION);
            reflectionTexture = Texture::CreateFBO(REFLECTION_TEX_SIZE, REFLECTION_TEX_SIZE, FORMAT_RGB565, Texture::DEPTH_RENDERBUFFER);          
            //reflectionTexture->SetMinMagFilter(Texture::FILTER_LINEAR, Texture::FILTER_LINEAR);
            reflectionSprite = Sprite::CreateFromTexture(reflectionTexture, 0, 0, REFLECTION_TEX_SIZE, REFLECTION_TEX_SIZE);
            
            refractionPass = new WaterRefractionRenderPass(PASS_FORWARD, RENDER_PASS_WATER_REFRACTION);
            refractionTexture = Texture::CreateFBO(REFRACTION_TEX_SIZE, REFRACTION_TEX_SIZE, FORMAT_RGB565, Texture::DEPTH_RENDERBUFFER);          
            //refractionTexture->SetMinMagFilter(Texture::FILTER_LINEAR, Texture::FILTER_LINEAR);
            refractionSprite = Sprite::CreateFromTexture(refractionTexture, 0, 0, REFRACTION_TEX_SIZE, REFRACTION_TEX_SIZE);
        }

        /*Rect viewportSave = RenderManager::Instance()->GetViewport();
        RenderManager::Instance()->SetRenderTarget(reflectionSprite);
        RenderManager::Instance()->DiscardFramebufferHW(RenderManager::COLOR_ATTACHMENT|RenderManager::DEPTH_ATTACHMENT|RenderManager::STENCIL_ATTACHMENT);
        RenderManager::Instance()->SetViewport(Rect(0, 0, REFLECTION_TEX_SIZE, REFLECTION_TEX_SIZE), true);        
        RenderManager::Instance()->SetRenderState(RenderState::RENDERSTATE_3D_BLEND);
        RenderManager::Instance()->FlushState();
        RenderManager::Instance()->Clear(Color(0,0,0,0), 1.0f, 0);
        //RenderManager::Instance()->ClearDepthBuffer();
        
        reflectionPass->SetWaterLevel(waterBox.max.z);
        reflectionPass->Draw(camera, renderSystem);
        RenderManager::Instance()->DiscardFramebufferHW(RenderManager::DEPTH_ATTACHMENT|RenderManager::STENCIL_ATTACHMENT);
        RenderManager::Instance()->RestoreRenderTarget();
                
        RenderManager::Instance()->SetRenderTarget(refractionSprite);
        RenderManager::Instance()->DiscardFramebufferHW(RenderManager::COLOR_ATTACHMENT|RenderManager::DEPTH_ATTACHMENT|RenderManager::STENCIL_ATTACHMENT);
        RenderManager::Instance()->SetViewport(Rect(0, 0, REFRACTION_TEX_SIZE, REFRACTION_TEX_SIZE), true);        
        RenderManager::Instance()->SetRenderState(RenderState::RENDERSTATE_3D_BLEND);
        RenderManager::Instance()->FlushState();
        RenderManager::Instance()->Clear(Color(0,0,0,0), 1.0f, 0);
        //RenderManager::Instance()->ClearDepthBuffer();
        refractionPass->SetWaterLevel(waterBox.min.z);
        refractionPass->Draw(camera, renderSystem);
        RenderManager::Instance()->RestoreRenderTarget();
        RenderManager::Instance()->DiscardFramebufferHW(RenderManager::DEPTH_ATTACHMENT|RenderManager::STENCIL_ATTACHMENT);
        RenderManager::Instance()->SetViewport(viewportSave, true);*/
        
        RenderManager::Instance()->ClipPush();
        Rect viewportSave = RenderManager::Instance()->GetViewport();
        uint32 currFboId = RenderManager::Instance()->HWglGetLastFBO();
        int32 currRenderOrientation = RenderManager::Instance()->GetRenderOrientation();
        //RenderManager::Instance()->SetRenderOrientation(Core::SCREEN_ORIENTATION_TEXTURE);
        
        RenderManager::Instance()->SetHWRenderTargetTexture(reflectionTexture);
        //discard everything here
        RenderManager::Instance()->SetViewport(Rect(0, 0, REFLECTION_TEX_SIZE, REFLECTION_TEX_SIZE), true);
        RenderManager::Instance()->SetRenderState(RenderState::RENDERSTATE_3D_BLEND);
        RenderManager::Instance()->FlushState();
        RenderManager::Instance()->Clear(Color(0,0,0,0), 1.0f, 0);
        
        reflectionPass->SetWaterLevel(waterBox.max.z);
        reflectionPass->Draw(camera, renderSystem);
        
        //discrad depth(everything?) here
        RenderManager::Instance()->DiscardFramebufferHW(RenderManager::DEPTH_ATTACHMENT|RenderManager::STENCIL_ATTACHMENT);
        
        
        RenderManager::Instance()->SetHWRenderTargetTexture(refractionTexture);
        
        RenderManager::Instance()->SetViewport(Rect(0, 0, REFLECTION_TEX_SIZE, REFLECTION_TEX_SIZE), true);
        RenderManager::Instance()->SetRenderState(RenderState::RENDERSTATE_3D_BLEND);
        RenderManager::Instance()->FlushState();
        RenderManager::Instance()->Clear(Color(0,0,0,0), 1.0f, 0);
        
        refractionPass->SetWaterLevel(waterBox.min.z);
        refractionPass->Draw(camera, renderSystem);

        //discrad depth(everything?) here
        RenderManager::Instance()->DiscardFramebufferHW(RenderManager::DEPTH_ATTACHMENT|RenderManager::STENCIL_ATTACHMENT);
        
        RenderManager::Instance()->HWglBindFBO(currFboId?currFboId:RenderManager::Instance()->GetFBOViewFramebuffer());
        RenderManager::Instance()->SetRenderOrientation(currRenderOrientation);
        RenderManager::Instance()->SetViewport(viewportSave, true);
        RenderManager::Instance()->ClipPop();

        camera->SetupDynamicParameters();
        //camera->SetupDynamicParameters(true, Vector4(0,0,-1, waterBox.min.z));
		
        
        Vector2 rssVal(1.0f/viewportSave.dx, 1.0f/viewportSave.dy);
        Vector2 screenOffsetVal(viewportSave.x, viewportSave.y);
		for (uint32 i=0; i<waterBatchesCount; ++i)
		{
            NMaterial *mat = waterLayer->Get(i)->GetMaterial();
            mat->SetPropertyValue(NMaterial::PARAM_RCP_SCREEN_SIZE, Shader::UT_FLOAT_VEC2, 1, &rssVal);
            mat->SetPropertyValue(NMaterial::PARAM_SCREEN_OFFSET, Shader::UT_FLOAT_VEC2, 1, &screenOffsetVal);
            mat->SetTexture(NMaterial::TEXTURE_DYNAMIC_REFLECTION, reflectionTexture);
            mat->SetTexture(NMaterial::TEXTURE_DYNAMIC_REFRACTION, refractionTexture);
		}

        /*hack for now*/
        renderPassBatchArray->Clear();
        renderPassBatchArray->PrepareVisibilityArray(&visibilityArray, camera);
        
        RenderManager::Instance()->SetRenderState(RenderState::RENDERSTATE_3D_BLEND);
        RenderManager::Instance()->FlushState();
        RenderManager::Instance()->Clear(Color(0,0,0,0), 1.0f, 0);
	}	

	DrawLayers(camera);

    /*if (needWaterPrepass)
    {
        static int t=0;        
        if ((t%700) == 0)
        {
            Image * img = reflectionTexture->CreateImageFromMemory(RenderState::RENDERSTATE_2D_BLEND);
            ImageLoader::Save(img, "reflectionTexture.png");
            img = refractionTexture->CreateImageFromMemory(RenderState::RENDERSTATE_2D_BLEND);
            ImageLoader::Save(img, "refractiontest.png");
        }
        t++;
    }*/
}

MainForwardRenderPass::~MainForwardRenderPass()
{
	SafeRelease(reflectionSprite);
	SafeRelease(refractionSprite);
	SafeRelease(reflectionTexture);
	SafeRelease(refractionTexture);
	SafeDelete(reflectionPass);
	SafeDelete(refractionPass);
}

WaterPrePass::WaterPrePass(const FastName & name, RenderPassID id):RenderPass(name, id), passCamera(NULL)
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
    SafeRelease(passCamera);
}

WaterReflectionRenderPass::WaterReflectionRenderPass(const FastName & name, RenderPassID id):WaterPrePass(name, id)
{	
}



void WaterReflectionRenderPass::Draw(Camera * camera, RenderSystem * renderSystem)
{    
    if (!passCamera)
        passCamera = new Camera();    
    passCamera->CopyMathOnly(*camera);    
    Vector3 v;
    v = passCamera->GetPosition();
    v.z = waterLevel - (v.z - waterLevel);
    passCamera->SetPosition(v);
    v = passCamera->GetTarget();
    v.z = waterLevel - (v.z - waterLevel);
    passCamera->SetTarget(v);    
    //v = passCamera->GetUp();
    //v*=-1;
    //passCamera->SetUp(v);
    //passCamera->SetupDynamicParameters();  
    Vector4 clipPlane(0,0,1, -(waterLevel-0.1f));
    passCamera->SetupDynamicParameters(&clipPlane);
    //add clipping plane
    
	visibilityArray.Clear();
	renderSystem->GetRenderHierarchy()->Clip(passCamera, &visibilityArray, RenderObject::CLIPPING_VISIBILITY_CRITERIA | RenderObject::VISIBLE_REFLECTION);	
	renderPassBatchArray->Clear();
	renderPassBatchArray->PrepareVisibilityArray(&visibilityArray, passCamera); 
    DrawLayers(passCamera);
}


WaterRefractionRenderPass::WaterRefractionRenderPass(const FastName & name, RenderPassID id) : WaterPrePass(name, id)
{
    /*const RenderLayerManager * renderLayerManager = RenderLayerManager::Instance();
    AddRenderLayer(renderLayerManager->GetRenderLayer(LAYER_SHADOW_VOLUME), LAST_LAYER);*/
}

void WaterRefractionRenderPass::Draw(Camera * camera, RenderSystem * renderSystem)
{
    if (!passCamera)
        passCamera = new Camera();        
    passCamera->CopyMathOnly(*camera);    
    //passCamera->SetupDynamicParameters();
    Vector4 clipPlane(0,0,-1, waterLevel+0.1f);
    passCamera->SetupDynamicParameters(&clipPlane);
    //add clipping plane
    visibilityArray.Clear();
    renderSystem->GetRenderHierarchy()->Clip(passCamera, &visibilityArray, RenderObject::CLIPPING_VISIBILITY_CRITERIA | RenderObject::VISIBLE_REFRACTION);	
    renderPassBatchArray->Clear();
    renderPassBatchArray->PrepareVisibilityArray(&visibilityArray, passCamera); 
    DrawLayers(passCamera);
    
}


};
