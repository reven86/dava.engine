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


#ifndef __DAVAENGINE_LANDSCAPE_NODE_H__
#define __DAVAENGINE_LANDSCAPE_NODE_H__

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Render/RenderBase.h"
#include "Render/Highlevel/RenderObject.h"
#include "FileSystem/FilePath.h"
#include "MemoryManager/MemoryProfiler.h"

namespace DAVA
{

/**    
    \brief Implementation of cdlod algorithm to render landscapes
    This class is base of the landscape code on all platforms
    Landscape node is always axial aligned for simplicity of frustum culling calculations
    Keep in mind that landscape orientation cannot be changed using localTransform and worldTransform matrices. 
 */

class FoliageSystem;
class NMaterial;
class SerializationContext;
class Heightmap;
class Frustum;

class Landscape : public RenderObject
{
    DAVA_ENABLE_CLASS_ALLOCATION_TRACKING(ALLOC_POOL_LANDSCAPE)

public:
    Landscape();
    virtual ~Landscape();

    static const int32 PATCH_VERTEX_COUNT = 17;
    static const int32 PATCH_QUAD_COUNT = (PATCH_VERTEX_COUNT - 1);
    static const int32 MAX_LANDSCAPE_SUBDIV_LEVELS = 9;
    static const int32 MAX_QUAD_COUNT_IN_VB = 128;
    static const int32 RENDER_QUAD_WIDTH = 129;
    static const int32 RENDER_QUAD_AND = RENDER_QUAD_WIDTH - 2;

    static const int32 TEXTURE_SIZE_FULL_TILED = 2048;
    static const int32 INITIAL_INDEX_BUFFER_CAPACITY = 20000;

    const static FastName PARAM_TEXTURE_TILING;
    const static FastName PARAM_TILE_COLOR0;
    const static FastName PARAM_TILE_COLOR1;
    const static FastName PARAM_TILE_COLOR2;
    const static FastName PARAM_TILE_COLOR3;

    const static FastName TEXTURE_COLOR;
    const static FastName TEXTURE_TILE;
    const static FastName TEXTURE_TILEMASK;
    const static FastName TEXTURE_SPECULAR;

    const static FastName FLAG_PATCH_SIZE_QUADS;
    const static FastName FLAG_USE_INSTANCING;

    const static FastName LANDSCAPE_QUALITY_NAME;
    const static FastName LANDSCAPE_QUALITY_VALUE_HIGH;

    //LandscapeVertex used in GetLevel0Geometry() only
    struct LandscapeVertex 
    {
        Vector3 position;
        Vector2 texCoord;
    };

    /**
    \brief Get landscape mesh geometry.
    Unoptimized lod0 mesh is returned.
    \param[out] vertices landscape vertices
    \param[out] indices landscape indices
    */
    bool GetLevel0Geometry(Vector<LandscapeVertex>& vertices, Vector<int32>& indices) const;

    /**
        \brief Builds landscape from heightmap image and bounding box of this landscape block
        \param[in] landscapeBox axial-aligned bounding box of the landscape block
     */
    virtual void BuildLandscapeFromHeightmapImage(const FilePath& heightmapPathname, const AABBox3& landscapeBox);

    /**
        \brief Function to receive pathname of heightmap object
        \returns pathname of heightmap
     */
    const FilePath& GetHeightmapPathname();
    void SetHeightmapPathname(const FilePath& newPath);

    float32 GetLandscapeSize() const;
    void SetLandscapeSize(float32 newSize);

    float32 GetLandscapeHeight() const;
    void SetLandscapeHeight(float32 newHeight);

    void GetDataNodes(Set<DataNode*>& dataNodes) override;

    void Save(KeyedArchive* archive, SerializationContext* serializationContext);
    void Load(KeyedArchive* archive, SerializationContext* serializationContext);

    bool PlacePoint(const Vector3& point, Vector3& result, Vector3* normal = 0) const;
    bool GetHeightAtPoint(const Vector3& point, float&) const;
    Vector3 GetPoint(int16 x, int16 y, uint16 height) const;

    Heightmap* GetHeightmap();
    virtual void SetHeightmap(Heightmap* height);

    NMaterial* GetMaterial();
    void SetMaterial(NMaterial* material);

    virtual RenderObject* Clone(RenderObject* newObject);
    virtual void RecalcBoundingBox();

    int32 GetDrawIndices() const;

    void SetFoliageSystem(FoliageSystem* _foliageSystem);

    void BindDynamicParameters(Camera* camera) override;
    void PrepareToRender(Camera* camera) override;

    void SetDebugDraw(bool isDebug);
    bool IsDebugDraw() const;

    void UpdatePart(Heightmap* fromHeightmap, const Rect2i& rect);
    void SetUpdatable(bool isUpdatable);
    bool IsUpdatable() const;

    void SetForceFirstLod(bool force);

protected:

    void AllocateGeometryData();
    void ReleaseGeometryData();

    void RestoreGeometry();

    void SetLandscapeSize(const Vector3& newSize);

    bool BuildHeightmap();
    void RebuildLandscape();

    struct RestoreBufferData
    {
        enum eBufferType
        {
            RESTORE_BUFFER_VERTEX,
            RESTORE_BUFFER_INDEX
        };

        rhi::Handle buffer;
        uint8* data;
        uint32 dataSize;
        eBufferType bufferType;
    };

    Vector<RestoreBufferData> bufferRestoreData;

    FilePath heightmapPath;

    Frustum* frustum = nullptr;
    Heightmap* heightmap = nullptr;
    NMaterial* landscapeMaterial = nullptr;
    FoliageSystem* foliageSystem = nullptr;

    uint32 drawIndices = 0;

    bool forceFirstLod = false;
    bool updatable = false;

    bool isInstancingUsed = false;

////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Subdivision

    struct PatchQuadInfo
    {
        uint32 vdoQuad; //TODO: remove this
        AABBox3 bbox;
        Vector3 positionOfMaxError;
        float32 maxError;
    };

    struct SubdivisionPatchInfo
    {
        enum
        {
            CLIPPED = 1,
            SUBDIVIDED = 2,
            TERMINATED = 3,
        };

        uint32 lastSubdividedSize = 0;
        uint8 subdivisionState = CLIPPED;
        uint8 startClipPlane = 0;
    };

    struct SubdivisionLevelInfo
    {
        uint32 offset;
        uint32 size;
    };

    SubdivisionPatchInfo* GetSubdivPatch(uint32 level, uint32 x, uint32 y);
    void UpdatePatchInfo(uint32 level, uint32 x, uint32 y);
    void SubdividePatch(uint32 level, uint32 x, uint32 y, uint8 clippingFlags);
    void TerminateSubdivision(uint32 level, uint32 x, uint32 y, uint32 lastSubdividedSize);
    void AddPatchToRender(uint32 level, uint32 x, uint32 y);

    uint32 subdivLevelCount = 0;
    uint32 subdivPatchCount = 0;
    uint32 quadsInWidth = 0;

    SubdivisionLevelInfo subdivLevelInfoArray[MAX_LANDSCAPE_SUBDIV_LEVELS];
    Vector<PatchQuadInfo> patchQuadArray;
    Vector<SubdivisionPatchInfo> subdivPatchArray;
    uint32 subdivPatchesDrawCount = 0;

//////Metrics
    Vector3 cameraPos;
    float32 fovCorrection;

    float32 defaultFov;

    float32 solidAngleError;
    float32 geometryAngleError;
    float32 absHeightError;

    float32 zoomSolidAngleError;
    float32 zoomGeometryAngleError;
    float32 zoomAbsHeightError;

    float32 fovSolidAngleError;
    float32 fovGeometryAngleError;
    float32 fovAbsHeightError;

    float32 zoomFov;
    float32 normalFov;

////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Non-instancing render

    struct VertexNoInstancing
    {
        Vector3 position;
        Vector2 texCoord;
        Vector3 normal;
        Vector3 tangent;
    };

    void AllocateGeometryDataNoInstancing();

    void AllocateRenderBatch();
    int16 AllocateQuadVertexBuffer(uint32 x, uint32 y, uint32 size);

    void DrawLandscapeNoInstancing();
    void DrawPatchNoInstancing(uint32 level, uint32 x, uint32 y, uint32 xNegSize, uint32 xPosSize, uint32 yNegSize, uint32 yPosSize);

    void FlushQueue();
    void ClearQueue();

    inline uint16 GetVertexIndex(uint16 x, uint16 y);

    void ResizeIndicesBufferIfNeeded(DAVA::uint32 newSize);

    Vector<rhi::HVertexBuffer> vertexBuffers;
    std::vector<uint16> indices;
    
    uint32 vLayoutUIDNoInstancing = rhi::VertexLayout::InvalidUID;

    int32 queueIndexCount = 0;
    int16 queuedQuadBuffer = 0;
    int32 flushQueueCounter = 0;

    bool isRequireTangentBasis = false;

////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Instancing render

    struct VertexInstancing
    {
        Vector2 position;
        Vector2 morphDir;
        Vector4 edgeMask;
    };

    struct InstanceData
    {
        Vector2 patchOffset;
        float32 patchScale;
        Vector4 lodOffset; // per edge
    };

    struct InstanceDataBuffer
    {
        rhi::HVertexBuffer buffer;
        rhi::HSyncObject syncObject;
    };

    void AllocateGeometryDataInstancing();

    Texture* CreateHeightTexture(Heightmap* heightmap);

    void DrawLandscapeInstancing();
    void DrawPatchInstancing(uint32 level, uint32 x, uint32 y, uint32 xNegSize, uint32 xPosSize, uint32 yNegSize, uint32 yPosSize);

    rhi::HVertexBuffer patchVertexBuffer;
    rhi::HIndexBuffer patchIndexBuffer;
    InstanceData* instanceDataPtr = nullptr;

    //CircularArray<rhi::HVertexBuffer, 9> instanceDataBuffers;
    Vector<InstanceDataBuffer> freeInstanceDataBuffers;
    Vector<InstanceDataBuffer> usedInstanceDataBuffers;

////////////////////////////////////////////////////////////////////////////////////////////////////////

public:
    INTROSPECTION_EXTEND(Landscape, RenderObject,
                         PROPERTY("heightmapPath", "Height Map Path", GetHeightmapPathname, SetHeightmapPathname, I_VIEW | I_EDIT)
                         PROPERTY("size", "Size", GetLandscapeSize, SetLandscapeSize, I_VIEW | I_EDIT)
                         PROPERTY("height", "Height", GetLandscapeHeight, SetLandscapeHeight, I_VIEW | I_EDIT)
                         PROPERTY("isDebugDraw", "isDebugDraw", IsDebugDraw, SetDebugDraw, I_VIEW | I_EDIT)
                         MEMBER(solidAngleError, "solidAngleError", I_VIEW | I_EDIT)
                         MEMBER(geometryAngleError, "geometryAngleError", I_VIEW | I_EDIT)
                         MEMBER(absHeightError, "absHeightError", I_VIEW | I_EDIT)

                         MEMBER(zoomSolidAngleError, "solidAngleError", I_VIEW | I_EDIT)
                         MEMBER(zoomGeometryAngleError, "geometryAngleError", I_VIEW | I_EDIT)
                         MEMBER(zoomAbsHeightError, "absHeightError", I_VIEW | I_EDIT));
};

// Inline functions
inline uint16 Landscape::GetVertexIndex(uint16 x, uint16 y)
{
    return x + y * RENDER_QUAD_WIDTH;
}
};

#endif // __DAVAENGINE_LANDSCAPE_NODE_H__
