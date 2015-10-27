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
StaticOcclusion::StaticOcclusion()
{
    staticOcclusionRenderPass = 0;
    currentData = 0;
    landscape = 0;
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

    currentFrameX = 0;
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
        min.z += cellHeightOffset[x + y * xBlockCount];
    }
    AABBox3 blockBBox(min, Vector3(min.x + size.x, min.y + size.y, min.z + size.z));
    return blockBBox;
}

bool StaticOcclusion::ProccessBlock()
{
    ProcessRecorderQueries();
    if (currentFrameZ >= zBlockCount)
    {
        return true;
    }
    RenderCurrentBlock();

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

    /*uint32 remain = xBlockCount * yBlockCount * zBlockCount;
    remain -= currentFrameX;
    remain -= (currentFrameY * xBlockCount);
    remain -= (currentFrameZ * xBlockCount * yBlockCount);
    return remain;*/

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

void StaticOcclusion::RenderCurrentBlock()
{
    uint64 t1 = SystemTimer::Instance()->GetAbsoluteNano();
    const uint32 stepCount = 10;

    AABBox3 cellBox = GetCellBox(currentFrameX, currentFrameY, currentFrameZ);
    Vector3 stepSize = cellBox.GetSize();
    stepSize /= float32(stepCount);

    Vector3 directions[6] =
    {
      Vector3(1.0f, 0.0f, 0.0f), // x 0
      Vector3(0.0f, 1.0f, 0.0f), // y 1
      Vector3(-1.0f, 0.0f, 0.0f), // -x 2
      Vector3(0.0f, -1.0f, 0.0f), // -y 3
      Vector3(0.0f, 0.0f, 1.0f), // +z 4
      Vector3(0.0f, 0.0f, -1.0f), // -z 5
    };

    uint32 blockIndex = currentFrameZ * (xBlockCount * yBlockCount) + currentFrameY * (xBlockCount) + currentFrameX;

    uint32 effectiveSideCount[6] = { 3, 3, 3, 3, 1, 1 };

    uint32 effectiveSides[6][3] =
    {
      { 0, 1, 3 },
      { 1, 0, 2 },
      { 2, 1, 3 },
      { 3, 0, 2 },
      { 4, 4, 4 },
      { 5, 5, 5 },
    };

    DVASSERT(occlusionFrameResults.size() == 0); //previous results are processed - at least for now
    uint64 renderingStart = SystemTimer::Instance()->GetAbsoluteNano();
    for (uint32 side = 0; side < 6; ++side)
    {
        Camera* camera = cameras[side];

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

        // Render 360 ??????? from each point

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
                        }
                    }

                    camera->SetPosition(renderPosition);
                    camera->SetDirection(directions[effectiveSides[side][realSideIndex]]);
                    if (effectiveSides[side][realSideIndex] == 4 || effectiveSides[side][realSideIndex] == 5)
                    {
                        camera->SetUp(Vector3(0.0f, 1.0f, 0.0f));
                        camera->SetLeft(Vector3(1.0f, 0.0f, 0.0f));
                    }
                    else
                    {
                        camera->SetUp(Vector3(0.0f, 0.0f, 1.0f));
                        camera->SetLeft(Vector3(1.0f, 0.0f, 0.0f));
                    }

                    StaticOcclusionFrameResult res;
                    res.blockIndex = blockIndex;
                    occlusionFrameResults.push_back(res);
                    staticOcclusionRenderPass->DrawOcclusionFrame(renderSystem, camera, occlusionFrameResults.back());
                }
    }

    uint64 timeTotalRendering = SystemTimer::Instance()->GetAbsoluteNano() - renderingStart;
    Logger::FrameworkDebug(Format("Block:%d renderTime: %0.9llf", blockIndex, (double)timeTotalRendering / 1e+9).c_str());
}

void StaticOcclusion::ProcessRecorderQueries()
{
    for (auto& frameResult : occlusionFrameResults)
    {
        for (size_t i = 0, sz = frameResult.frameRequests.size(); i < sz; ++i)
        {
            if (frameResult.frameRequests[i] == nullptr)
                continue;

            while (!rhi::QueryIsReady(frameResult.queryBuffer, i)) //wait query
            {
            }

            if (rhi::QueryValue(frameResult.queryBuffer, i) != 0)
                currentData->EnableVisibilityForObject(frameResult.blockIndex, frameResult.frameRequests[i]->GetStaticOcclusionIndex());
        }
        rhi::DeleteQueryBuffer(frameResult.queryBuffer);
    }
    occlusionFrameResults.clear();
}

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

StaticOcclusionData& StaticOcclusionData::operator=(const StaticOcclusionData& other)
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
        cellHeightOffset = new float32[sizeX * sizeY];
        memcpy(cellHeightOffset, other.cellHeightOffset, sizeof(float32) * sizeX * sizeY);
    }

    return *this;
}

void StaticOcclusionData::Init(uint32 _sizeX, uint32 _sizeY, uint32 _sizeZ, uint32 _objectCount, const AABBox3& _bbox, const float32* _cellHeightOffset)
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
        cellHeightOffset = new float32[sizeX * sizeY];
        memcpy(cellHeightOffset, _cellHeightOffset, sizeof(float32) * sizeX * sizeY);
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

uint32* StaticOcclusionData::GetBlockVisibilityData(uint32 blockIndex)
{
    return &data[(blockIndex * objectCount / 32)];
}
};