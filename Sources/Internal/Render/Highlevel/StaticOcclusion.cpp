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
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"
#include "Render/Highlevel/StaticOcclusion.h"
#include "Render/Highlevel/StaticOcclusionRenderPass.h"
#include "Render/Highlevel/RenderBatchArray.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/RenderFastNames.h"
#include "Render/Image.h"
#include "Render/ImageLoader.h"
#include "Utils/StringFormat.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{
    
StaticOcclusion::StaticOcclusion()
    : manager(5000)
{
    staticOcclusionRenderPass = 0;
    renderTargetSprite = 0;
    renderTargetTexture = 0;
    currentData = 0;
}
    
StaticOcclusion::~StaticOcclusion()
{
    SafeDelete(renderPassBatchArray);
    SafeDelete(staticOcclusionRenderPass);
    SafeRelease(renderTargetSprite);
    SafeRelease(renderTargetTexture);
}

    
void StaticOcclusion::BuildOcclusionInParallel(const AABBox3 & _occlusionAreaRect,
                                               uint32 _xBlockCount,
                                               uint32 _yBlockCount,
                                               uint32 _zBlockCount,
                                               Vector<RenderObject*> & renderObjects,
                                               StaticOcclusionData * _currentData,
                                               RenderHierarchy * _renderHierarchy)
{
    staticOcclusionRenderPass = new StaticOcclusionRenderPass(PASS_FORWARD, this);
    
    staticOcclusionRenderPass->AddRenderLayer(new StaticOcclusionRenderLayer(LAYER_OPAQUE, RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_MATERIAL, this), LAST_LAYER);
	staticOcclusionRenderPass->AddRenderLayer(new StaticOcclusionRenderLayer(LAYER_AFTER_OPAQUE, RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_MATERIAL, this), LAST_LAYER);
	staticOcclusionRenderPass->AddRenderLayer(new StaticOcclusionRenderLayer(LAYER_ALPHA_TEST_LAYER, RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_MATERIAL, this), LAST_LAYER);
    staticOcclusionRenderPass->AddRenderLayer(new StaticOcclusionRenderLayer(LAYER_TRANSLUCENT, RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_DISTANCE, this), LAST_LAYER);
	staticOcclusionRenderPass->AddRenderLayer(new StaticOcclusionRenderLayer(LAYER_AFTER_TRANSLUCENT, RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_MATERIAL, this), LAST_LAYER);

    
    renderPassBatchArray = new RenderPassBatchArray();

    currentData = _currentData;
    renderHierarchy = _renderHierarchy;
    occlusionAreaRect = _occlusionAreaRect;
    xBlockCount = _xBlockCount;
    yBlockCount = _yBlockCount;
    zBlockCount = _zBlockCount;
    
    currentFrameX = 0; //xBlockCount - 1;
    currentFrameY = 0; //yBlockCount - 1;
    currentFrameZ = 0; //zBlockCount - 1;
    
    
    for (uint32 k = 0; k < 6; ++k)
    {
        cameras[k] = new Camera();
        cameras[k]->SetupPerspective(90.0f, 1.0, 1.0f, 2500.0f);
    }
    
    
    if (!renderTargetTexture)
        renderTargetTexture = Texture::CreateFBO(512, 512, FORMAT_RGBA8888, Texture::DEPTH_RENDERBUFFER);
    if (!renderTargetSprite)
        renderTargetSprite = Sprite::CreateFromTexture(renderTargetTexture, 0, 0, 512, 512);
    
    for (uint32 k = 0; k < renderObjects.size(); ++k)
        renderObjects[k]->SetStaticOcclusionIndex((uint16)k);
    
    renderObjectsArray = renderObjects;
}
    
//uint32 StaticOcclusion::CameraToCellIndex(Camera * camera)
//{
//    const Vector3 & position = camera->GetPosition();
//    if (!occlusionAreaRect.IsInside(position))return 0xFFFFFFFF;
//    
//    uint32 x = (uint32)(position.x / (float32)xBlockCount);
//    uint32 y = (uint32)(position.y / (float32)yBlockCount);
//    uint32 z = (uint32)(position.z / (float32)zBlockCount);
//    
//    
//}

//uint32 * StaticOcclusion::GetCellVisibilityData(Camera * camera)
//{
//    const Vector3 & position = camera->GetPosition();
//    if (!occlusionAreaRect.IsInside(position))return 0;
//    
//    uint32 x = (uint32)(position.x / (float32)xBlockCount);
//    uint32 y = (uint32)(position.y / (float32)yBlockCount);
//    uint32 z = (uint32)(position.z / (float32)zBlockCount);
//    
//    uint32 * objectsVisibility = visibilityData + z * (xBlockCount * yBlockCount * objectCount / 32) + y * (xBlockCount * objectCount / 32) + (x * objectCount / 32);
//    return objectsVisibility;
//}
    
AABBox3 StaticOcclusion::GetCellBox(uint32 x, uint32 y, uint32 z)
{
    Vector3 size = occlusionAreaRect.GetSize();
    
    size.x /= xBlockCount;
    size.y /= yBlockCount;
    size.z /= zBlockCount;
    
    Vector3 min(occlusionAreaRect.min.x + currentFrameX * size.x,
                occlusionAreaRect.min.y + currentFrameY * size.y,
                occlusionAreaRect.min.z + currentFrameZ * size.z);
    AABBox3 blockBBox(min, Vector3(min.x + size.x, min.y + size.y, min.z + size.z));
    return blockBBox;
}

uint32 StaticOcclusion::RenderFrame()
{
    uint64 t1 = SystemTimer::Instance()->GetAbsoluteNano();
    const uint32 stepCount = 10;
    //uint32 renderFrameCount = xBlockCount * yBlockCount * zBlockCount * 4 * (stepSizeX * stepSizeY);
    
    AABBox3 cellBox = GetCellBox(currentFrameX, currentFrameY, currentFrameZ);
    Vector3 stepSize = cellBox.GetSize();
    stepSize /= stepCount;
    
    Vector3 directions[6] =
    {
        Vector3(1.0f, 0.0f, 0.0f),  // x
        Vector3(0.0f, 1.0f, 0.0f),  // y
        Vector3(-1.0f, 0.0f, 0.0f), // -x
        Vector3(0.0f, -1.0f, 0.0f), // -y
        Vector3(0.0f, 0.0f, 1.0f),
        Vector3(0.0f, 0.0f, -1.0f),
    };
    
    // Render One Block (100%)
    frameGlobalVisibleInfo.clear();

    for (uint32 side = 0; side < 4; ++side)
    {
        Camera * camera = cameras[side];

        Vector3 startPosition, directionX, directionY;
        if (side == 0) // +x
        {
            startPosition = Vector3(cellBox.max.x, cellBox.min.y, cellBox.min.z);
            directionX = Vector3(0.0f, 1.0f, 0.0f);
            directionY = Vector3(0.0f, 0.0f, 1.0f);
        }else if (side == 2) // -x
        {
            startPosition = Vector3(cellBox.min.x, cellBox.min.y, cellBox.min.z);
            directionX = Vector3(0.0f, 1.0f, 0.0f);
            directionY = Vector3(0.0f, 0.0f, 1.0f);
        }else if (side == 1) // +y
        {
            startPosition = Vector3(cellBox.min.x, cellBox.max.y, cellBox.min.z);
            directionX = Vector3(1.0f, 0.0f, 0.0f);
            directionY = Vector3(0.0f, 0.0f, 1.0f);
        }else if (side == 3) // -y
        {
            startPosition = Vector3(cellBox.min.x, cellBox.min.y, cellBox.min.z);
            directionX = Vector3(1.0f, 0.0f, 0.0f);
            directionY = Vector3(0.0f, 0.0f, 1.0f);
        }
        
        //Vector3 centerOnTargetSide = cellBox.GetCenter() + camera->GetDirection() * halfSize;
        //Vector3 crossProduct = CrossProduct(camera->GetDirection(), )

        
        
        for (uint32 stepX = 0; stepX < stepCount; ++stepX)
            for (uint32 stepY = 0; stepY < stepCount; ++stepY)
            {
                Vector3 renderPosition = startPosition + directionX * stepX * stepSize + directionY * stepY * stepSize;
                camera->SetPosition(renderPosition);
                camera->SetDirection(directions[side]);
                camera->SetUp(Vector3(0.0f, 0.0f, 1.0f));
                camera->SetLeft(Vector3(1.0f, 0.0f, 1.0f));
                camera->Set();
                // Do Render
                
                RenderManager::Instance()->SetRenderTarget(renderTargetSprite);
                RenderManager::Instance()->SetViewport(Rect(0, 0, 512, 512), false);
                //RenderManager::Instance()->ClearDepthBuffer();
                //RenderManager::Instance()->ClearStencilBuffer(0);
                
//                RenderManager::Instance()->SetDefault2DState();
//                
//                glColorMask(true, true, true, true);
//
//                RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 0.5f);
//                RenderHelper::Instance()->FillRect(Rect(0.0f, 0.0f, 256.0f, 256.0f));
//
//                RenderManager::Instance()->SetColor(0.0f, 0.0f, 1.0f, 1.0f);
//                RenderHelper::Instance()->FillRect(Rect(0.0f, 256.0f, 256.0f, 256.0f));
//
//                RenderManager::Instance()->SetColor(1.0f, 1.0f, 0.0f, 1.0f);
//                RenderHelper::Instance()->FillRect(Rect(128.0f, 0.0f, 256.0f, 256.0f));

                //glColorMaterial(0, 0);
                
//                renderPassBatchArray->Clear();
//                renderHierarchy->Clip(camera, renderPassBatchArray);
//                staticOcclusionRenderPass->Draw(camera, renderPassBatchArray);
                
                
                RenderManager::Instance()->SetDefault3DState();
                RenderManager::Instance()->FlushState();
                RenderManager::Instance()->Clear(Color(0.5f, 0.5f, 0.5f, 1.0f), 1.0f, 0);

                //RenderManager::Instance()->ClearDepthBuffer();
                
                camera->Set();
                
                recordedBatches.clear();
                renderPassBatchArray->Clear();
                renderHierarchy->Clip(camera, renderPassBatchArray);
                staticOcclusionRenderPass->Draw(camera, renderPassBatchArray);
                
                size_t size = recordedBatches.size();
                for (size_t k = 0; k < size; ++k)
                {
                    std::pair<RenderBatch*, OcclusionQueryManagerHandle> & batchInfo = recordedBatches[k];
                    OcclusionQuery & query = manager.Get(batchInfo.second);
                    while (!query.IsResultAvailable())
                    {
                    }
                    uint32 result;
                    query.GetQuery(&result);
                    if (result != 0)
                    {
                        frameGlobalVisibleInfo.insert(batchInfo.first->GetRenderObject());
                    }
                    manager.ReleaseQueryObject(batchInfo.second);
                }
                //recordedBatches.clear();

                RenderManager::Instance()->RestoreRenderTarget();
                

                //renderTargetTexture->
//                if ((stepX == 0) && (stepY == 0))
//                {
//                    Image * image = renderTargetTexture->CreateImageFromMemory();
//                    ImageLoader::Save(image, Format("~doc:/renderimage_%d_%d_%d.png", side, stepX, stepY));
//                    SafeRelease(image);
//                }
            }
        
    }
    
    // Invisible on every frame
    uint32 invisibleObjectCount =  (uint32)renderObjectsArray.size() - frameGlobalVisibleInfo.size();
//    for (Map<RenderObject*, uint32>::iterator it = frameGlobalOccludedInfo.begin(), end = frameGlobalOccludedInfo.end(); it != end; ++it)
//    {
//        if (renderFrameCount == it->second)
//            invisibleObjectCount++;
//    }
    
    uint32 blockIndex = currentFrameX * currentFrameY * currentFrameZ;
    
    for (Set<RenderObject*>::iterator it = frameGlobalVisibleInfo.begin(), end = frameGlobalVisibleInfo.end(); it != end; ++it)
    {
        RenderObject * obj = *it;
        currentData->SetVisibilityForObject(blockIndex, obj->GetStaticOcclusionIndex(), 1);
    }
    
    t1 = SystemTimer::Instance()->GetAbsoluteNano() - t1;

    Logger::FrameworkDebug(Format("Object count:%d Vis Count: %d Invisible Object Count:%d time: %0.9llf", renderObjectsArray.size(), frameGlobalVisibleInfo.size(), invisibleObjectCount, (double)t1 / 1e+9));
    
    //RenderManager::Instance()->SetRenderTarget((Texture*)0);
    
    //
    currentFrameX++;
    if (currentFrameX >= xBlockCount)
    {
        currentFrameX = 0;
        currentFrameY++;
        if (currentFrameY >= yBlockCount)
        {
            currentFrameY = 0;
            currentFrameZ++;
            if (currentFrameZ >= zBlockCount)
            {
                return 0;
            }
        }
    }
    return (xBlockCount * yBlockCount * zBlockCount - currentFrameX * currentFrameY * currentFrameZ);
}
    

void StaticOcclusion::RecordFrameQuery(RenderBatch * batch, OcclusionQueryManagerHandle handle)
{
    recordedBatches.push_back(std::pair<RenderBatch*, OcclusionQueryManagerHandle>(batch, handle));
}

    
StaticOcclusionData::StaticOcclusionData()
    : data(0)
    , objectCount(0)
    , blockCount(0)
{
    
}
    
StaticOcclusionData::~StaticOcclusionData()
{
    SafeDeleteArray(data);
}

void StaticOcclusionData::Init(uint32 _sizeX, uint32 _sizeY, uint32 _sizeZ, uint32 _objectCount)
{
    objectCount = _objectCount;
    sizeX = _sizeX;
    sizeY = _sizeY;
    sizeZ = _sizeZ;
    blockCount = sizeX * sizeY * sizeZ;
    
    objectCount += (32 - objectCount & 31);
    
    data = new uint32[(blockCount * objectCount / 32)];
    memset(data, 0, (blockCount * objectCount / 32));
}

void StaticOcclusionData::SetVisibilityForObject(uint32 blockIndex, uint32 objectIndex, uint32 visible)
{
    data[(blockIndex * objectCount / 32) + (objectIndex / 32)] |= visible << (objectIndex & 31);
}
    
uint32 * StaticOcclusionData::GetBlockVisibilityData(uint32 blockIndex)
{
    return &data[(blockIndex * objectCount / 32)];
}
    
};
