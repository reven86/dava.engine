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


#ifndef __DAVAENGINE_STATIC_OCCLUSION__
#define __DAVAENGINE_STATIC_OCCLUSION__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/BaseMath.h"
#include "Render/RenderBase.h"
#include "Render/Texture.h"
#include "Render/OcclusionQuery.h"

namespace DAVA
{
class Camera;
class StaticOcclusionRenderPass;
class RenderObject;
class RenderHierarchy;
class OcclusionQueryPool;
class RenderBatch;
class RenderSystem;
class Scene;
class Sprite;
class Landscape;
    
class StaticOcclusionData
{
public:
    StaticOcclusionData();
    ~StaticOcclusionData();
    
    void Init(uint32 sizeX, uint32 sizeY, uint32 sizeZ, uint32 objectCount, const AABBox3 & bbox, const float32 *_cellHeightOffset);
    void EnableVisibilityForObject(uint32 blockIndex, uint32 objectIndex);
    void DisableVisibilityForObject(uint32 blockIndex, uint32 objectIndex);

    bool IsObjectVisibleFromBlock(uint32 blockIndex, uint32 objectIndex) const;

    uint32* GetBlockVisibilityData(uint32 blockIndex);
    StaticOcclusionData & operator= (const StaticOcclusionData & other);

    void SetData(const uint32* _data, uint32 dataSize);
    const uint32* GetData() const;

public:
    AABBox3 bbox;
    uint32 sizeX = 0;
    uint32 sizeY = 0;
    uint32 sizeZ = 0;
    uint32 blockCount = 0;
    uint32 objectCount = 0;
    float32* cellHeightOffset = nullptr;

private:
    Vector<uint32> dataHolder;
};

struct StaticOcclusionFrameResult
{
    uint32 blockIndex = 0;
    rhi::HQueryBuffer queryBuffer = rhi::HQueryBuffer(rhi::InvalidHandle);
    Vector<RenderObject*> frameRequests;
};

class StaticOcclusion
{
public:    
    StaticOcclusion();
    ~StaticOcclusion();

    void StartBuildOcclusion(StaticOcclusionData* currentData, RenderSystem* renderSystem, Landscape* landscape);
    bool ProccessBlock(); // returns true if finished building
    void AdvanceToNextBlock();

    uint32 GetCurrentStepsCount();
    uint32 GetTotalStepsCount();

    const String& GetInfoMessage() const;

private:    
    AABBox3 GetCellBox(uint32 x, uint32 y, uint32 z);

    void MarkQueriesAsCompletedForObjectInBlock(uint16 objectIndex, uint32 blockIndex);
    bool ProcessRecorderQueries();

    struct RenderPassCameraConfig
    {
        Vector3 position;
        Vector3 left;
        Vector3 up;
        Vector3 direction;
        uint32 side = 0;
        uint32 blockIndex = 0;
    };

    struct Statistics
    {
        uint64 blockProcessingTime = 0;
        double buildDuration = 0.0;
        uint64 buildStartTime = 0;
        uint64 totalRenderPasses = 0;
    } stats;

    void UpdateInfoString();
    void BuildRenderPassConfigsForCurrentBlock();
    bool RenderCurrentBlock(); // returns true, if all passes for block completed
    bool PerformRender(const RenderPassCameraConfig&);

private:
    std::array<Camera*, 6> cameras;
    StaticOcclusionRenderPass* staticOcclusionRenderPass = nullptr;
    StaticOcclusionData* currentData = nullptr;
    RenderSystem* renderSystem = nullptr;
    Landscape* landscape = nullptr;
    float32* cellHeightOffset = nullptr;
    Vector<StaticOcclusionFrameResult> occlusionFrameResults;
    Vector<RenderPassCameraConfig> renderPassConfigs;
    String lastInfoMessage;
    AABBox3 occlusionAreaRect;
    uint32 xBlockCount = 0;
    uint32 yBlockCount = 0;
    uint32 zBlockCount = 0;
    uint32 currentFrameX = 0;
    uint32 currentFrameY = 0;
    uint32 currentFrameZ = 0;
};

};

#endif //__DAVAENGINE_STATIC_OCCLUSION__
