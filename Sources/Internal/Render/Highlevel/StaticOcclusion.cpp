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
#include "Render/Image/Image.h"
#include "Utils/StringFormat.h"
#include "Platform/SystemTimer.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Image/ImageSystem.h"
#include "Render/2D/Systems/RenderSystem2D.h"

namespace DAVA
{
    
    static const uint32 RENDER_TARGET_WIDTH = 1024;// / 4;
    static const uint32 RENDER_TARGET_HEIGHT = 1024;// / 4;
    
    
    StaticOcclusion::StaticOcclusion()
    : queryPool(10000)
    {
        staticOcclusionRenderPass = 0;
        renderTargetTexture = 0;
        currentData = 0;
        landscape = 0;
    }
    
    StaticOcclusion::~StaticOcclusion()
    {
        for (uint32 k = 0; k < 6; ++k)
        {
            SafeRelease(cameras[k]);
        }
        SafeDelete(staticOcclusionRenderPass);
        SafeRelease(renderTargetTexture);
    }
    
    
    void StaticOcclusion::BuildOcclusionInParallel(Vector<RenderObject*> & renderObjects,
                                                   Landscape * _landscape,
                                                   StaticOcclusionData * _currentData,
                                                   eIndexRenew renewIndexEnum)
    {
        staticOcclusionRenderPass = new StaticOcclusionRenderPass(PASS_FORWARD, this, RENDER_PASS_FORWARD_ID);
        
        
        currentData = _currentData;
        occlusionAreaRect = currentData->bbox;
        cellHeightOffset = currentData->cellHeightOffset;
        xBlockCount = currentData->sizeX;
        yBlockCount = currentData->sizeY;
        zBlockCount = currentData->sizeZ;
        
        currentFrameX = 0; //xBlockCount - 1;
        currentFrameY = 0; //yBlockCount - 1;
        currentFrameZ = 0; //zBlockCount - 1;
        
        
        for (uint32 k = 0; k < 6; ++k)
        {
            cameras[k] = new Camera();
            cameras[k]->SetupPerspective(95.0f, (float32)RENDER_TARGET_WIDTH / (float32)RENDER_TARGET_HEIGHT, 1.0f, 2500.0f);
        }
        
        
        if (!renderTargetTexture)
            renderTargetTexture = Texture::CreateFBO(RENDER_TARGET_WIDTH, RENDER_TARGET_HEIGHT, FORMAT_RGBA8888, Texture::DEPTH_RENDERBUFFER);
        
        /* Set<uint16> busyIndices;
         for (uint32 k = 0; k < renderObjects.size(); ++k)
         {
         busyIndices.insert(renderObjects[k]->GetStaticOcclusionIndex());
         }*/
        
        if (renewIndexEnum == RENEW_OCCLUSION_INDICES)
        {
            for (uint32 k = 0; k < renderObjects.size(); ++k)
            {
                DVASSERT(renderObjects[k]->GetStaticOcclusionIndex()==INVALID_STATIC_OCCLUSION_INDEX); //if we are going to renew indices they should be cleared prior to it
                renderObjects[k]->SetStaticOcclusionIndex((uint16)k);
            }
        }
        
        renderObjectsArray = renderObjects;
        landscape = _landscape;
        recordedBatches.reserve(10000);
    }
    
    void StaticOcclusion::SetEqualVisibilityVector(Map<RenderObject*, Vector<RenderObject*> > & equalVisibility)
    {
        equalVisibilityArray = equalVisibility;
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
        
        Vector3 min(occlusionAreaRect.min.x + x * size.x,
                    occlusionAreaRect.min.y + y * size.y,
                    occlusionAreaRect.min.z + z * size.z);
        if (cellHeightOffset)
        {
            min.z += cellHeightOffset[x+y*xBlockCount];
        }
        AABBox3 blockBBox(min, Vector3(min.x + size.x, min.y + size.y, min.z + size.z));
        return blockBBox;
    }
    
    uint32 StaticOcclusion::RenderFrame()
    {
        RenderFrame(currentFrameX, currentFrameY, currentFrameZ);
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
        
        uint32 remain = xBlockCount * yBlockCount * zBlockCount;
        remain -= currentFrameX;
        remain -= (currentFrameY * xBlockCount);
        remain -= (currentFrameZ * xBlockCount * yBlockCount);
        return remain;
    }
    
    void StaticOcclusion::RenderFrame(uint32 cellX, uint32 cellY, uint32 cellZ)
    {
        //    for (uint32 k = 0; k < 1000; ++k)
        //    {
        //        for (uint32 m = 0; m < 3000; ++m)
        //        {
        //            OcclusionQueryPoolHandle handle = queryPool.CreateQueryObject();
        //            queryPool.ReleaseQueryObject(handle);
        //        }
        //    }
        //
        
        
        
        uint64 t1 = SystemTimer::Instance()->GetAbsoluteNano();
        const uint32 stepCount = 10;
        //uint32 renderFrameCount = xBlockCount * yBlockCount * zBlockCount * 4 * (stepSizeX * stepSizeY);
        
        AABBox3 cellBox = GetCellBox(cellX, cellY, cellZ);
        Vector3 stepSize = cellBox.GetSize();
        stepSize /= stepCount;
        
        
        Vector3 directions[6] =
        {
            Vector3(1.0f, 0.0f, 0.0f),  // x 0
            Vector3(0.0f, 1.0f, 0.0f),  // y 1
            Vector3(-1.0f, 0.0f, 0.0f), // -x 2
            Vector3(0.0f, -1.0f, 0.0f), // -y 3
            Vector3(0.0f, 0.0f, 1.0f),  // +z 4
            Vector3(0.0f, 0.0f, -1.0f), // -z 5
        };
        
        // Render One Block (100%)
        frameGlobalVisibleInfo.clear();
        
        uint32 blockIndex = cellZ * (currentData->sizeX * currentData->sizeY)
        + cellY * (currentData->sizeX) + cellX;
        
        //renderPositions.clear();
        
        uint32 effectiveSideCount[6] = {3, 3, 3, 3, 1, 1};
        
        uint32 effectiveSides[6][3]=
        {
            {0, 1, 3},
            {1, 0, 2},
            {2, 1, 3},
            {3, 0, 2},
            {4, 4, 4},
            {5, 5, 5},
        };
        
        uint64 timeTotalWaiting = 0;
        uint64 timeTotalCulling = 0;
        uint64 timeTotalRendering = 0;
        
        for (uint32 side = 0; side < 6; ++side)
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
            }else if (side == 4) // +z
            {
                startPosition = Vector3(cellBox.min.x, cellBox.min.y, cellBox.max.z);
                directionX = Vector3(1.0f, 0.0f, 0.0f);
                directionY = Vector3(0.0f, 1.0f, 0.0f);
            }else if (side == 5) // -z
            {
                startPosition = Vector3(cellBox.min.x, cellBox.min.y, cellBox.min.z);
                directionX = Vector3(1.0f, 0.0f, 0.0f);
                directionY = Vector3(0.0f, 1.0f, 0.0f);
            }
            
            
            //Vector3 centerOnTargetSide = cellBox.GetCenter() + camera->GetDirection() * halfSize;
            //Vector3 crossProduct = CrossProduct(camera->GetDirection(), )
            
            
            // Render 360 from each point
            for (uint32 realSideIndex = 0; realSideIndex < effectiveSideCount[side]; ++realSideIndex)
                for (uint32 stepX = 0; stepX <= stepCount; ++stepX)
                    for (uint32 stepY = 0; stepY <= stepCount; ++stepY)
                    {
                        Vector3 renderPosition = startPosition + directionX * (float32)stepX * stepSize + directionY * (float32)stepY * stepSize;
                        
                        if (landscape)
                        {
                            Vector3 pointOnLandscape;
                            if (landscape->PlacePoint(renderPosition, pointOnLandscape))
                            {
                                if (renderPosition.z < pointOnLandscape.z)
                                    continue;
                                //renderPosition.z = pointOnLandscape.z + 0.5f;
                            }
                        }
                        
                        //renderPositions.push_back(renderPosition);
                        camera->SetPosition(renderPosition);
                        camera->SetDirection(directions[effectiveSides[side][realSideIndex]]);
                        if ( effectiveSides[side][realSideIndex] == 4 || effectiveSides[side][realSideIndex] == 5)
                        {
                            camera->SetUp(Vector3(0.0f, 1.0f, 0.0f));
                            camera->SetLeft(Vector3(1.0f, 0.0f, 0.0f));
                        }else
                        {
                            camera->SetUp(Vector3(0.0f, 0.0f, 1.0f));
                            camera->SetLeft(Vector3(1.0f, 0.0f, 0.0f));
                        }
                        //camera->SetupDynamicParameters();
                        // Do Render
                        
                        RenderManager::Instance()->SetRenderTarget(renderTargetTexture);
                        RenderManager::Instance()->SetViewport(Rect(0, 0, (float32)RENDER_TARGET_WIDTH, (float32)RENDER_TARGET_HEIGHT));
                        
                        //camera->SetupDynamicParameters();
                        
                        uint64 timeCulling = SystemTimer::Instance()->GetAbsoluteNano();
                        
                        timeCulling = SystemTimer::Instance()->GetAbsoluteNano() - timeCulling;
                        timeTotalCulling += timeCulling;
                        
                        uint64 timeRendering = SystemTimer::Instance()->GetAbsoluteNano();
                        staticOcclusionRenderPass->SetOcclusionCamera(camera);
                        staticOcclusionRenderPass->SetIndex(side, stepX, stepY, effectiveSides[side][realSideIndex] == side);
                        
                        staticOcclusionRenderPass->Draw(renderSystem, RenderManager::ALL_BUFFERS);
                        
                        timeRendering = SystemTimer::Instance()->GetAbsoluteNano() - timeRendering;
                        timeTotalRendering += timeRendering;

                        RenderManager::Instance()->SetRenderTarget(0);
                        
                        size_t size = recordedBatches.size();
                        /*
                         Explanation what is 8000.
                         Here we can wait until occlusion query will be finished by HW, or we can go to the next frame.
                         With 8000 we just skip several rendering frames and get results from HW later.
                         8000 is just experimental number. Just fast optimization.
                         */
                        //if (size > 8000)
                        {
                            for (size_t k = 0; k < size; ++k)
                            {
                                std::pair<RenderBatch*, OcclusionQueryPoolHandle> & batchInfo = recordedBatches[k];
                                OcclusionQuery & query = queryPool.Get(batchInfo.second);
                                
                                uint64 timeWaiting = SystemTimer::Instance()->GetAbsoluteNano();
                                while (!query.IsResultAvailable())
                                {
                                }
                                timeWaiting = SystemTimer::Instance()->GetAbsoluteNano() - timeWaiting;
                                timeTotalWaiting += timeWaiting;
                                
                                uint32 result;
                                query.GetQuery(&result);
                                if (result != 0)
                                {
                                    frameGlobalVisibleInfo.insert(batchInfo.first->GetRenderObject());
                                }
                                queryPool.ReleaseQueryObject(batchInfo.second);
                            }
                            recordedBatches.clear();
                        }
                        
//                                             if (/*(stepX == 0) && (stepY == 0) &&*/ effectiveSides[side][realSideIndex] == side)                                            
//                                             {
//                                                 Image * image = renderTargetTexture->CreateImageFromMemory(RenderState::RENDERSTATE_2D_OPAQUE);
//                                                 ImageSystem::Instance()->Save(FilePath(Format("~doc:/renderimage_b%d_s_%d_es_%d_%d_%d.png", blockIndex, side, effectiveSides[side][realSideIndex] ,stepX, stepY)),
//                                                                               image);
//                                                 SafeRelease(image);
//                                             }
                    }
            
        }
        
        //    size_t size = recordedBatches.size();
        //    for (size_t k = 0; k < size; ++k)
        //    {
        //        std::pair<RenderBatch*, OcclusionQueryPoolHandle> & batchInfo = recordedBatches[k];
        //        OcclusionQuery & query = queryPool.Get(batchInfo.second);
        //
        //        uint64 timeWaiting = SystemTimer::Instance()->GetAbsoluteNano();
        //        while (!query.IsResultAvailable())
        //        {
        //        }
        //        timeWaiting = SystemTimer::Instance()->GetAbsoluteNano() - timeWaiting;
        //        timeTotalWaiting += timeWaiting;
        //
        //        uint32 result;
        //        query.GetQuery(&result);
        //        if (result != 0)
        //        {
        //            frameGlobalVisibleInfo.insert(batchInfo.first->GetRenderObject());
        //        }
        //        manager.ReleaseQueryObject(batchInfo.second);
        //    }
        //    recordedBatches.clear();
        
        
        // Invisible on every frame
        uint32 invisibleObjectCount =  static_cast<uint32>((uint32)renderObjectsArray.size() - frameGlobalVisibleInfo.size());
        uint32 visibleCount = static_cast<uint32>(frameGlobalVisibleInfo.size());
        //    for (Map<RenderObject*, uint32>::iterator it = frameGlobalOccludedInfo.begin(), end = frameGlobalOccludedInfo.end(); it != end; ++it)
        //    {
        //        if (renderFrameCount == it->second)
        //            invisibleObjectCount++;
        //    }
        
        //uint32 blockIndex = z * (data->sizeX * data->sizeY) + y * (data->sizeX) + (x);
        for (uint32 k = 0; k < renderObjectsArray.size(); ++k)
        {
            currentData->DisableVisibilityForObject(blockIndex, k);
        }
        
        for (Set<RenderObject*>::iterator it = frameGlobalVisibleInfo.begin(), end = frameGlobalVisibleInfo.end(); it != end; ++it)
        {
            RenderObject * obj = *it;
            ///DVASSERT(obj->GetStaticOcclusionIndex() != INVALID_STATIC_OCCLUSION_INDEX);
            if (obj->GetStaticOcclusionIndex() == INVALID_STATIC_OCCLUSION_INDEX)
            {
                continue;
            }
            
            currentData->EnableVisibilityForObject(blockIndex, obj->GetStaticOcclusionIndex());
            
            Map<RenderObject*, Vector<RenderObject*> >::iterator findIt = equalVisibilityArray.find(obj);
            if (findIt != equalVisibilityArray.end())
            {
                Vector<RenderObject*> & equalObjects = findIt->second;
                uint32 size = static_cast<uint32>(equalObjects.size());
                for (uint32 k = 0; k < size; ++k)
                {
                    DVASSERT(equalObjects[k]->GetStaticOcclusionIndex() != INVALID_STATIC_OCCLUSION_INDEX);
                    currentData->EnableVisibilityForObject(blockIndex, equalObjects[k]->GetStaticOcclusionIndex());
                    invisibleObjectCount --;
                    visibleCount++;
                }
            }
        }
        
        // VERIFY
        uint32 checkVisCount = 0;
        uint32 checkInvisCount = 0;
        uint32 * bitdata = currentData->GetBlockVisibilityData(blockIndex);
        uint32 size = (uint32)renderObjectsArray.size();
        for (uint32 k = 0; k < size; ++k)
        {
            uint32 index = k / 32; // number of bits in uint32
            uint32 shift = k & 31; // bitmask for uint32
            if (bitdata[index] & (1 << shift))
            {
                checkVisCount++;
            }else
            {
                checkInvisCount++;
            }
        }
        
        
        t1 = SystemTimer::Instance()->GetAbsoluteNano() - t1;
        
        Logger::FrameworkDebug(Format("Block:%d Object count:%d Vis Count: %d(%d) Invisible Object Count:%d(%d) time: %0.9llf waitTime: %0.9llf cullTime: %0.9llf renderTime: %0.9llf",
                                      blockIndex,
                                      renderObjectsArray.size(), visibleCount, checkVisCount, invisibleObjectCount, checkInvisCount,
                                      (double)t1 / 1e+9,
                                      (double)timeTotalWaiting / 1e+9,
                                      (double)timeTotalCulling / 1e+9,
                                      (double)timeTotalRendering / 1e+9).c_str());
        
        //RenderManager::Instance()->SetRenderTarget((Texture*)0);
    }
    
    
    void StaticOcclusion::RecordFrameQuery(RenderBatch * batch, OcclusionQueryPoolHandle handle)
    {
        recordedBatches.push_back(std::pair<RenderBatch*, OcclusionQueryPoolHandle>(batch, handle));
    }
    
    AABBox3 bbox;
    uint32 sizeX;
    uint32 sizeY;
    uint32 sizeZ;
    uint32  blockCount;
    uint32  objectCount;
    uint32 * data;
    float32* cellHeightOffset;

    StaticOcclusionData::StaticOcclusionData()
    : sizeX(5)
    , sizeY(5)
    , sizeZ(2)
    , blockCount(0)
    , objectCount(0)
    , data(0)
    , cellHeightOffset(0)
    {
        
    }
    
    StaticOcclusionData::~StaticOcclusionData()
    {
        SafeDeleteArray(data);
        SafeDeleteArray(cellHeightOffset);
    }
    
    StaticOcclusionData & StaticOcclusionData::operator= (const StaticOcclusionData & other)
    {
        sizeX = other.sizeX;
        sizeY = other.sizeY;
        sizeZ = other.sizeZ;
        objectCount = other.objectCount;
        blockCount = other.blockCount;
        bbox = other.bbox;
        
        SafeDeleteArray(data);
        data = new uint32[(blockCount * objectCount / 32)];
        memcpy(data, other.data, (blockCount * objectCount / 32) * 4);
        
        SafeDeleteArray(cellHeightOffset);
        if (other.cellHeightOffset)
        {
            cellHeightOffset = new float32[sizeX*sizeY];
            memcpy(cellHeightOffset, other.cellHeightOffset, sizeof(float32)*sizeX*sizeY);
        }
        
        
        return *this;
    }
    
    
    void StaticOcclusionData::Init(uint32 _sizeX, uint32 _sizeY, uint32 _sizeZ, uint32 _objectCount, const AABBox3 & _bbox, const float32 *_cellHeightOffset)
    {
        //DVASSERT(data == 0);
        SafeDeleteArray(data);
        SafeDeleteArray(cellHeightOffset);    
        
        objectCount = _objectCount;
        sizeX = _sizeX;
        sizeY = _sizeY;
        sizeZ = _sizeZ;
        blockCount = sizeX * sizeY * sizeZ;
        bbox = _bbox;
        
        objectCount += (32 - objectCount & 31);
        
        data = new uint32[(blockCount * objectCount / 32)];
        memset(data, 0, (blockCount * objectCount / 32) * 4);
        
        if (_cellHeightOffset)
        {
            cellHeightOffset = new float32[sizeX*sizeY];
            memcpy(cellHeightOffset, _cellHeightOffset, sizeof(float32)*sizeX*sizeY);
        }
        
    }
    
    void StaticOcclusionData::EnableVisibilityForObject(uint32 blockIndex, uint32 objectIndex)
    {
        data[(blockIndex * objectCount / 32) + (objectIndex / 32)] |= 1 << (objectIndex & 31);
    }
    
    void StaticOcclusionData::DisableVisibilityForObject(uint32 blockIndex, uint32 objectIndex)
    {
        data[(blockIndex * objectCount / 32) + (objectIndex / 32)] &= ~(1 << (objectIndex & 31));
    }
    
    
    uint32 * StaticOcclusionData::GetBlockVisibilityData(uint32 blockIndex)
    {
        return &data[(blockIndex * objectCount / 32)];
    }
    
};