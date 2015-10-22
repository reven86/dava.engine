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
#include "Render/RenderHelper.h"
#include "Render/Highlevel/StaticOcclusion.h"
#include "Render/Highlevel/StaticOcclusionRenderPass.h"
#include "Render/Highlevel/RenderBatchArray.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Image/Image.h"
#include "Utils/StringFormat.h"
#include "Platform/SystemTimer.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Image/ImageSystem.h"
#include "Render/2D/Systems/RenderSystem2D.h"

namespace DAVA
{
const Vector3 viewDirections[6] =
{
  Vector3(1.0f, 0.0f, 0.0f), //  x 0
  Vector3(0.0f, 1.0f, 0.0f), //  y 1
  Vector3(-1.0f, 0.0f, 0.0f), // -x 2
  Vector3(0.0f, -1.0f, 0.0f), // -y 3
  Vector3(0.0f, 0.0f, 1.0f), // +z 4
  Vector3(0.0f, 0.0f, -1.0f), // -z 5
};

const uint32 effectiveSideCount[6] = { 3, 3, 3, 3, 1, 1 };

const uint32 effectiveSides[6][3] =
{
  { 0, 1, 3 },
  { 1, 0, 2 },
  { 2, 1, 3 },
  { 3, 0, 2 },
  { 4, 4, 4 },
  { 5, 5, 5 },
};

namespace stats
{
static uint64 blockProcessingTime = 0;
static double buildDuration = 0.0;
static uint64 buildStartTime = 0;
static uint64 totalRenderPasses = 0;
static uint64 actualRenderPasses = 0;
}

StaticOcclusion::StaticOcclusion()    
{
    for (uint32 k = 0; k < 6; ++k)
    {
        cameras[k] = new Camera();
        cameras[k]->SetupPerspective(95.0f, 1.0f, 1.0f, 2500.0f); //aspect of one is anyway required to avoid side occlusion errors
    }
}
    
StaticOcclusion::~StaticOcclusion()
{
    for (uint32 k = 0; k < 6; ++k)
    {
        SafeRelease(cameras[k]);
    }
    SafeDelete(staticOcclusionRenderPass);        
}

void StaticOcclusion::StartBuildOcclusion(StaticOcclusionData* _currentData, RenderSystem* _renderSystem, Landscape* _landscape)
{
    staticOcclusionRenderPass = new StaticOcclusionRenderPass(PASS_FORWARD);
        
    currentData = _currentData;
    occlusionAreaRect = currentData->bbox;
    cellHeightOffset = currentData->cellHeightOffset;
    xBlockCount = currentData->sizeX;
    yBlockCount = currentData->sizeY;
    zBlockCount = currentData->sizeZ;

    stats::buildStartTime = SystemTimer::Instance()->GetAbsoluteNano();
    stats::buildDuration = 0.0;
    stats::blockProcessingTime = 0;
    stats::actualRenderPasses = 0;
    stats::totalRenderPasses = 0;

    currentFrameX = -1; // we increasing this, before rendering, so we will start from zero
    currentFrameY = 0;
    currentFrameZ = 0;            
                
    renderSystem = _renderSystem;
    landscape = _landscape;        
}       
    
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

void StaticOcclusion::AdvanceToNextBlock()
{
    currentFrameX++;
    if (currentFrameX >= xBlockCount)
    {
        currentFrameX = 0;
        currentFrameY++;
        if (currentFrameY >= yBlockCount)
        {
            currentFrameY = 0;
            currentFrameZ++;
        }
    }
}

bool StaticOcclusion::ProccessBlock()
{
    if (!ProcessRecorderQueries())
    {
        RenderCurrentBlock();
        return false;
    }

    auto currentTime = SystemTimer::Instance()->GetAbsoluteNano();

    if (renderPassConfigs.empty())
    {
        AdvanceToNextBlock();
        if (currentFrameZ < zBlockCount)
        {
            auto blockIndex = currentFrameX + currentFrameY * xBlockCount + currentFrameZ * xBlockCount * yBlockCount;
            auto totalBlocks = xBlockCount * yBlockCount * zBlockCount;

            if (stats::blockProcessingTime == 0)
                stats::blockProcessingTime = currentTime;

            auto dt = static_cast<double>(currentTime - stats::blockProcessingTime) / 1e+9;
            stats::buildDuration += dt;
            Logger::Info("Block %u/%u, dt: %.4llfs, duration: %.4llf, renders: %llu/%llu",
                         blockIndex, totalBlocks, static_cast<double>(dt), stats::buildDuration,
                         stats::actualRenderPasses, stats::totalRenderPasses);
            stats::blockProcessingTime = currentTime;

            BuildRenderPassConfigsForCurrentBlock();
        }
        else
        {
            // all blocks processed
            auto dt = static_cast<double>(currentTime - stats::buildStartTime) / 1e+9;
            Logger::Info("Occlusion build time: %.4llfs", dt);
            return true;
        }
    }

    RenderCurrentBlock();

    return false;
}

uint32 StaticOcclusion::GetCurrentStepsCount()
{
    return currentFrameX + (currentFrameY * xBlockCount) + (currentFrameZ * xBlockCount * yBlockCount);
    
}

uint32 StaticOcclusion::GetTotalStepsCount()
{
    return xBlockCount * yBlockCount * zBlockCount;
}

void StaticOcclusion::BuildRenderPassConfigsForCurrentBlock()
{
    const uint32 stepCount = 3;

    uint32 blockIndex = currentFrameX + currentFrameY * xBlockCount + currentFrameZ * xBlockCount * yBlockCount;
    AABBox3 cellBox = GetCellBox(currentFrameX, currentFrameY, currentFrameZ);
    Vector3 stepSize = cellBox.GetSize();
    stepSize /= float32(stepCount);

    DVASSERT(occlusionFrameResults.size() == 0); // previous results are processed - at least for now

    for (uint32 side = 0; side < 6; ++side)
    {
        Vector3 startPosition, directionX, directionY;
        if (side == 0) // +x
        {
            startPosition = Vector3(cellBox.max.x, cellBox.min.y, cellBox.min.z);
            directionX = Vector3(0.0f, 1.0f, 0.0f);
            directionY = Vector3(0.0f, 0.0f, 1.0f);
        }
        else if (side == 2) // -x
        {
            startPosition = Vector3(cellBox.min.x, cellBox.min.y, cellBox.min.z);
            directionX = Vector3(0.0f, 1.0f, 0.0f);
            directionY = Vector3(0.0f, 0.0f, 1.0f);
        }
        else if (side == 1) // +y
        {
            startPosition = Vector3(cellBox.min.x, cellBox.max.y, cellBox.min.z);
            directionX = Vector3(1.0f, 0.0f, 0.0f);
            directionY = Vector3(0.0f, 0.0f, 1.0f);
        }
        else if (side == 3) // -y
        {
            startPosition = Vector3(cellBox.min.x, cellBox.min.y, cellBox.min.z);
            directionX = Vector3(1.0f, 0.0f, 0.0f);
            directionY = Vector3(0.0f, 0.0f, 1.0f);
        }
        else if (side == 4) // +z
        {
            startPosition = Vector3(cellBox.min.x, cellBox.min.y, cellBox.max.z);
            directionX = Vector3(1.0f, 0.0f, 0.0f);
            directionY = Vector3(0.0f, 1.0f, 0.0f);
        }
        else if (side == 5) // -z
        {
            startPosition = Vector3(cellBox.min.x, cellBox.min.y, cellBox.min.z);
            directionX = Vector3(1.0f, 0.0f, 0.0f);
            directionY = Vector3(0.0f, 1.0f, 0.0f);
        }

        for (uint32 realSideIndex = 0; realSideIndex < effectiveSideCount[side]; ++realSideIndex)
        {
            for (uint32 stepX = 0; stepX <= stepCount; ++stepX)
            {
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
                        }
                    }

                    RenderPassCameraConfig config;
                    config.blockIndex = blockIndex;
                    config.side = side;
                    config.position = renderPosition;
                    config.direction = viewDirections[effectiveSides[side][realSideIndex]];
                    if (effectiveSides[side][realSideIndex] == 4 || effectiveSides[side][realSideIndex] == 5)
                    {
                        config.up = Vector3(0.0f, 1.0f, 0.0f);
                        config.left = Vector3(1.0f, 0.0f, 0.0f);
                    }
                    else
                    {
                        config.up = Vector3(0.0f, 0.0f, 1.0f);
                        config.left = Vector3(1.0f, 0.0f, 0.0f);
                    }
                    renderPassConfigs.push_back(config);
                }
            }
        }
    }
}

bool StaticOcclusion::PerformRender(const RenderPassCameraConfig& rpc)
{
    Camera* camera = cameras[rpc.side];
    camera->SetPosition(rpc.position);
    camera->SetLeft(rpc.left);
    camera->SetUp(rpc.up);
    camera->SetDirection(rpc.direction);

    occlusionFrameResults.emplace_back();
    StaticOcclusionFrameResult& res = occlusionFrameResults.back();
    staticOcclusionRenderPass->DrawOcclusionFrame(renderSystem, camera, res, *currentData, rpc.blockIndex);
    if (res.queryBuffer == rhi::InvalidHandle)
    {
        occlusionFrameResults.pop_back();
        return false;
    }

    return true;
}

bool StaticOcclusion::RenderCurrentBlock()
{
    uint64 renders = 0;
    uint64 actualRenders = 0;

#if (SAVE_OCCLUSION_IMAGES)
    uint64 maxRenders = 1; // Max(256u, renderPassConfigs.size() / 4);
#else
    uint64 maxRenders = 16; // Max(256u, renderPassConfigs.size() / 4);
#endif

    while ((renders < maxRenders) && !renderPassConfigs.empty())
    {
        auto i = renderPassConfigs.begin();
        std::advance(i, rand() % renderPassConfigs.size());
        if (PerformRender(*i))
        {
            ++actualRenders;
        }
        renderPassConfigs.erase(i);
        ++renders;
    }

    stats::totalRenderPasses += renders;
    stats::actualRenderPasses += actualRenders;

    return renderPassConfigs.empty();
}

bool StaticOcclusion::ProcessRecorderQueries()
{
    auto fr = occlusionFrameResults.begin();

    while (fr != occlusionFrameResults.end())
    {
        uint32 processedRequests = 0;
        uint32 index = 0;
        for (auto& req : fr->frameRequests)
        {
            if (req == nullptr)
            {
                ++processedRequests;
                ++index;
                continue;
            }

            DVASSERT(req->GetStaticOcclusionIndex() != INVALID_STATIC_OCCLUSION_INDEX);

            if (rhi::QueryIsReady(fr->queryBuffer, index))
            {
                if (rhi::QueryValue(fr->queryBuffer, index))
                {
                    /*
                    if (!currentData->IsObjectVisibleFromBlock(fr->blockIndex, req->GetStaticOcclusionIndex()))
                    {
                        Logger::Info("Object: %u is visible from block %u", uint32(req->GetStaticOcclusionIndex()), fr->blockIndex);
                    }
					*/
                    currentData->EnableVisibilityForObject(fr->blockIndex, req->GetStaticOcclusionIndex());
                }
                ++processedRequests;
                req = nullptr;
            }

            ++index;
        }

        if (processedRequests == static_cast<uint32>(fr->frameRequests.size()))
        {
            rhi::DeleteQueryBuffer(fr->queryBuffer, true);
            fr = occlusionFrameResults.erase(fr);
        }
        else
        {
            ++fr;
        }
    }

    return occlusionFrameResults.empty();
}
   
StaticOcclusionData::StaticOcclusionData()
: sizeX(5)
, sizeY(5)
, sizeZ(2)
, blockCount(0)
, objectCount(0)
, cellHeightOffset(0)
{
        
}
    
StaticOcclusionData::~StaticOcclusionData()
{
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
    dataHolder = other.dataHolder;

    SafeDeleteArray(cellHeightOffset);
    if (other.cellHeightOffset)
    {
        cellHeightOffset = new float32[sizeX*sizeY];
        memcpy(cellHeightOffset, other.cellHeightOffset, sizeof(float32)*sizeX*sizeY);
    }

    return *this;
}

void StaticOcclusionData::Init(uint32 _sizeX, uint32 _sizeY, uint32 _sizeZ, uint32 _objectCount,
                               const AABBox3& _bbox, const float32* _cellHeightOffset)
{
    SafeDeleteArray(cellHeightOffset);    
        
    objectCount = _objectCount;
    sizeX = _sizeX;
    sizeY = _sizeY;
    sizeZ = _sizeZ;
    blockCount = sizeX * sizeY * sizeZ;
    bbox = _bbox;
        
    objectCount += (32 - objectCount & 31);

    auto numElements = blockCount * objectCount / 32;
    dataHolder.resize(numElements);
    std::fill(dataHolder.begin(), dataHolder.end(), 0);

    if (_cellHeightOffset)
    {
        cellHeightOffset = new float32[sizeX*sizeY];
        memcpy(cellHeightOffset, _cellHeightOffset, sizeof(float32)*sizeX*sizeY);
    }
}

bool StaticOcclusionData::IsObjectVisibleFromBlock(uint32 blockIndex, uint32 objectIndex) const
{
    auto objIndex = 1 << (objectIndex & 31);
    auto index = (blockIndex * objectCount / 32) + (objectIndex / 32);
    DVASSERT(index < dataHolder.size());
    return (dataHolder[index] & objIndex) != 0;
}

void StaticOcclusionData::EnableVisibilityForObject(uint32 blockIndex, uint32 objectIndex)
{
    auto index = (blockIndex * objectCount / 32) + (objectIndex / 32);
    DVASSERT(index < dataHolder.size());
    dataHolder[index] |= 1 << (objectIndex & 31);
}
    
void StaticOcclusionData::DisableVisibilityForObject(uint32 blockIndex, uint32 objectIndex)
{
    auto index = (blockIndex * objectCount / 32) + (objectIndex / 32);
    DVASSERT(index < dataHolder.size());
    dataHolder[index] &= ~(1 << (objectIndex & 31));
}
    
    
uint32 * StaticOcclusionData::GetBlockVisibilityData(uint32 blockIndex)
{
    auto index = blockIndex * objectCount / 32;
    DVASSERT(index < dataHolder.size());
    return dataHolder.data() + index;
}

void StaticOcclusionData::SetData(const uint32* _data, uint32 dataSize)
{
    auto elements = dataSize / sizeof(uint32);
    dataHolder.resize(elements);
    std::copy(_data, _data + elements, dataHolder.begin());
}
    
};