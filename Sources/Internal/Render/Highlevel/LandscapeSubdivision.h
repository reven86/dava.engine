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


#ifndef __DAVAENGINE_HEIGHTMAP_SUBDIVISION_H__
#define __DAVAENGINE_HEIGHTMAP_SUBDIVISION_H__

#include "Base/BaseTypes.h"
#include "Base/IntrospectionBase.h"
#include "MemoryManager/MemoryProfiler.h"

namespace DAVA
{
class Frustum;
class Heightmap;

class LandscapeSubdivision : public InspBase
{
    DAVA_ENABLE_CLASS_ALLOCATION_TRACKING(ALLOC_POOL_LANDSCAPE)

public:
    LandscapeSubdivision();
    ~LandscapeSubdivision();

    struct SubdivisionPatchInfo
    {
        enum
        {
            CLIPPED = 1,
            SUBDIVIDED = 2,
            TERMINATED = 3,
        };

        uint32 lastSubdivLevel = 0;
        float32 subdivMorph = 0.f;
        uint8 subdivisionState = CLIPPED;
        uint8 startClipPlane = 0;
    };

    struct SubdivisionLevelInfo
    {
        uint32 offset;
        uint32 size;
    };

    struct SubdivisionMetrics : public InspBase
    {
        float32 normalFov = 70.f;
        float32 zoomFov = 6.5f;

        float32 normalMaxHeightError = 0.02f;
        float32 normalMaxPatchRadiusError = 0.8f;
        float32 normalMaxAbsoluteHeightError = 3.f;

        float32 zoomMaxHeightError = 0.04f;
        float32 zoomMaxPatchRadiusError = 1.6f;
        float32 zoomMaxAbsoluteHeightError = 3.f;

        INTROSPECTION(SubdivisionMetrics,
                      MEMBER(normalMaxHeightError, "normalMaxHeightError", I_VIEW | I_EDIT)
                      MEMBER(normalMaxPatchRadiusError, "normalMaxPatchRadiusError", I_VIEW | I_EDIT)
                      MEMBER(normalMaxAbsoluteHeightError, "normalMaxAbsoluteHeightError", I_VIEW | I_EDIT)
                      MEMBER(zoomMaxHeightError, "zoomMaxHeightError", I_VIEW | I_EDIT)
                      MEMBER(zoomMaxPatchRadiusError, "zoomMaxPatchRadiusError", I_VIEW | I_EDIT)
                      MEMBER(zoomMaxAbsoluteHeightError, "zoomMaxAbsoluteHeightError", I_VIEW | I_EDIT)
                      );
    };

    void BuildSubdivision(Heightmap* heightmap, const AABBox3& bbox, uint32 patchSizeQuads, uint32 minSubdivideLevel, bool calculateMorph);
    void PrepareSubdivision(Camera* camera, const Matrix4* worldTransform);

    const SubdivisionLevelInfo& GetLevelInfo(uint32 level) const;
    const SubdivisionPatchInfo& GetPatchInfo(uint32 level, uint32 x, uint32 y) const;
    SubdivisionMetrics& GetMetrics();

    uint32 GetLevelCount() const;
    uint32 GetPatchCount() const;
    uint32 GetTerminatedPatchesCount() const;

    void UpdatePatchInfo(const Rect2i& heighmapRect);
    void SetForceMaxSubdivision(bool forceSubdivide);

private:
    struct PatchQuadInfo
    {
        AABBox3 bbox;
        Vector3 positionOfMaxError;
        float32 maxError;
        float32 radius;
    };

    void UpdatePatchInfo(uint32 level, uint32 x, uint32 y, const Rect2i& updateRect);
    void SubdividePatch(uint32 level, uint32 x, uint32 y, uint8 clippingFlags, float32 heightError0, float32 radiusError0);
    void TerminateSubdivision(uint32 level, uint32 x, uint32 y, uint32 lastSubdivLevel, float32 lastSubdivMorph);

    const PatchQuadInfo& GetPatchQuadInfo(uint32 level, uint32 x, uint32 y) const;

    Vector<SubdivisionLevelInfo> subdivLevelInfoArray;
    Vector<PatchQuadInfo> patchQuadArray;
    Vector<SubdivisionPatchInfo> subdivPatchArray;
    uint32 terminatedPatchesCount = 0;

    uint32 minSubdivLevel = 0;
    uint32 subdivLevelCount = 0;
    uint32 subdivPatchCount = 0;
    uint32 patchSizeQuads = 8;

    SubdivisionMetrics metrics;

    float32 maxHeightError;
    float32 maxPatchRadiusError;
    float32 maxAbsoluteHeightError;

    Vector3 cameraPos;
    float32 tanFovY;

    Frustum* frustum = nullptr;
    Heightmap* heightmap = nullptr;

    AABBox3 bbox;

    bool calculateMorph;
    bool forceMaxSubdiv;

    friend class LandscapeSystem;

public:
    INTROSPECTION(LandscapeSubdivision,
                  MEMBER(metrics, "metrics", I_VIEW | I_EDIT)
                  );
};

inline const LandscapeSubdivision::SubdivisionLevelInfo& LandscapeSubdivision::GetLevelInfo(uint32 level) const
{
    DVASSERT(level < subdivLevelInfoArray.size());
    return subdivLevelInfoArray[level];
}

inline const LandscapeSubdivision::SubdivisionPatchInfo& LandscapeSubdivision::GetPatchInfo(uint32 level, uint32 x, uint32 y) const
{
    const SubdivisionLevelInfo& levelInfo = GetLevelInfo(level);
    DVASSERT(x < levelInfo.size && y < levelInfo.size);
    return subdivPatchArray[levelInfo.offset + (y << level) + x];
}

inline const LandscapeSubdivision::PatchQuadInfo& LandscapeSubdivision::GetPatchQuadInfo(uint32 level, uint32 x, uint32 y) const
{
    const SubdivisionLevelInfo& levelInfo = GetLevelInfo(level);
    DVASSERT(x < levelInfo.size && y < levelInfo.size);
    return patchQuadArray[levelInfo.offset + (y << level) + x];
}

inline void LandscapeSubdivision::SetForceMaxSubdivision(bool forceSubdivide)
{
    forceMaxSubdiv = forceSubdivide;
}

inline uint32 LandscapeSubdivision::GetLevelCount() const
{
    return subdivLevelCount;
}

inline uint32 LandscapeSubdivision::GetPatchCount() const
{
    return subdivPatchCount;
}

inline uint32 LandscapeSubdivision::GetTerminatedPatchesCount() const
{
    return terminatedPatchesCount;
}

inline LandscapeSubdivision::SubdivisionMetrics& LandscapeSubdivision::GetMetrics()
{
    return metrics;
}
};

#endif //__DAVAENGINE_HEIGHTMAP_SUBDIVISION_H__
