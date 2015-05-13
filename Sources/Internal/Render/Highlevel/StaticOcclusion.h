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
    
    uint32 * GetBlockVisibilityData(uint32 blockIndex);
    StaticOcclusionData & operator= (const StaticOcclusionData & other);
    
    AABBox3 bbox;
    uint32 sizeX;
    uint32 sizeY;
    uint32 sizeZ;
    uint32  blockCount;
    uint32  objectCount;
    uint32 * data;
    float32* cellHeightOffset;
};

class StaticOcclusion
{
public:
    enum eIndexRenew
    {
        RENEW_OCCLUSION_INDICES,
        LEAVE_OLD_INDICES,
    };
    
    StaticOcclusion();
    ~StaticOcclusion();
    
    inline void SetScene(Scene * _scene);
    inline void SetRenderSystem(RenderSystem * _renderSystem);
    
    void BuildOcclusionInParallel(Vector<RenderObject*> & renderObjects,
                                  Landscape * landscape,
                                  StaticOcclusionData * currentData,
                                  eIndexRenew renewIndexEnum);
    
    void SetEqualVisibilityVector(Map<RenderObject*,
                                  Vector<RenderObject*> > & equalVisibility);

    
    inline OcclusionQueryPool & GetOcclusionQueryPool();
    //uint32 * GetCellVisibilityData(Camera * camera);
    
    uint32 RenderFrame();
    void RenderFrame(uint32 cellX, uint32 cellY, uint32 cellZ);

    void FillOcclusionDataObject(StaticOcclusionData * data);
    
    void RecordFrameQuery(RenderBatch * batch, OcclusionQueryPoolHandle handle);
    
    //Vector<Vector3> renderPositions;
    //Vector<Vector3> renderDirections;
    
    inline Texture * GetRTTexture() const;
    
private:
    void ProcessRecordedBatches();
    AABBox3 GetCellBox(uint32 x, uint32 y, uint32 z);
        
    OcclusionQueryPool queryPool;
    Vector<std::pair<RenderBatch*, OcclusionQueryPoolHandle> > recordedBatches;
    Set<RenderObject*> frameGlobalVisibleInfo;
    
    AABBox3  occlusionAreaRect;
    float32 *cellHeightOffset;
    uint32 xBlockCount;
    uint32 yBlockCount;
    uint32 zBlockCount;
    uint32 objectCount;
    uint32 currentFrameX;
    uint32 currentFrameY;
    uint32 currentFrameZ;
    Camera * cameras[6];
    StaticOcclusionRenderPass * staticOcclusionRenderPass;
    Texture * renderTargetTexture;

    StaticOcclusionData * currentData;
    
    // for testing purposes
    RenderSystem * renderSystem;
    Scene * scene;
    Vector<RenderObject*> renderObjectsArray;
    Landscape * landscape;
    Map<RenderObject*, Vector<RenderObject*> > equalVisibilityArray;
};
    
inline OcclusionQueryPool & StaticOcclusion::GetOcclusionQueryPool()
{
    return queryPool;
}

inline void StaticOcclusion::SetScene(Scene * _scene) { scene = _scene; };
inline void StaticOcclusion::SetRenderSystem(RenderSystem * _renderSystem) {renderSystem = _renderSystem; };
inline Texture * StaticOcclusion::GetRTTexture() const { return renderTargetTexture; };

};

#endif //__DAVAENGINE_STATIC_OCCLUSION__
