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
#include "Render/Highlevel/LandscapeSubdivision.h"

namespace DAVA
{
/**    
    \brief Implementation of cdlod algorithm to render landscapes
    This class is base of the landscape code on all platforms
 */

class LandscapeSystem;
class FoliageSystem;
class NMaterial;
class SerializationContext;
class Heightmap;
class LandscapeSubdivision;

class Landscape : public RenderObject
{
    DAVA_ENABLE_CLASS_ALLOCATION_TRACKING(ALLOC_POOL_LANDSCAPE)

public:
    Landscape();
    virtual ~Landscape();

    static const int32 RENDER_PARCEL_SIZE_VERTICES = 129;
    static const int32 RENDER_PARCEL_SIZE_QUADS = (RENDER_PARCEL_SIZE_VERTICES - 1);
    static const int32 RENDER_PARCEL_AND = RENDER_PARCEL_SIZE_VERTICES - 2;
    static const int32 INITIAL_INDEX_BUFFER_CAPACITY = 20000;

    static const int32 TEXTURE_SIZE_FULL_TILED = 2048;

    const static FastName PARAM_TEXTURE_TILING;
    const static FastName PARAM_TILE_COLOR0;
    const static FastName PARAM_TILE_COLOR1;
    const static FastName PARAM_TILE_COLOR2;
    const static FastName PARAM_TILE_COLOR3;

    const static FastName TEXTURE_COLOR;
    const static FastName TEXTURE_TILE;
    const static FastName TEXTURE_TILEMASK;
    const static FastName TEXTURE_SPECULAR;

    const static FastName LANDSCAPE_QUALITY_NAME;
    const static FastName LANDSCAPE_QUALITY_VALUE_HIGH;

    //LandscapeVertex used in GetLevel0Geometry() only
    struct LandscapeVertex
    {
        Vector3 position;
        Vector2 texCoord;
    };

    //TODO: move to Beast
    DAVA_DEPRECATED(bool GetLevel0Geometry(Vector<LandscapeVertex>& vertices, Vector<int32>& indices) const);

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

    void Save(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Load(KeyedArchive* archive, SerializationContext* serializationContext) override;

    bool PlacePoint(const Vector3& point, Vector3& result, Vector3* normal = 0) const;
    bool GetHeightAtPoint(const Vector3& point, float&) const;

    Heightmap* GetHeightmap();
    virtual void SetHeightmap(Heightmap* height);

    NMaterial* GetMaterial();
    void SetMaterial(NMaterial* material);

    RenderObject* Clone(RenderObject* newObject) override;
    void RecalcBoundingBox() override;

    int32 GetDrawIndices() const;

    void SetFoliageSystem(FoliageSystem* _foliageSystem);

    void BindDynamicParameters(Camera* camera) override;
    void PrepareToRender(Camera* camera) override;

    void UpdatePart(const Rect2i& rect);
    void SetUpdatable(bool isUpdatable);
    bool IsUpdatable() const;

    void SetForceMaxSubdiv(bool force);

    void SetUseInstancing(bool useInstancing);
    bool IsUseInstancing() const;

    LandscapeSubdivision* GetSubdivision();

protected:
    enum RenderMode
    {
        RENDERMODE_NO_INSTANCING,
        RENDERMODE_INSTANCING,
        RENDERMODE_INSTANCING_MORPHING,
    };

    void AddPatchToRender(uint32 level, uint32 x, uint32 y);

    void AllocateGeometryData();
    void ReleaseGeometryData();

    void RestoreGeometry();

    void SetLandscapeSize(const Vector3& newSize);
    bool BuildHeightmap();
    void RebuildLandscape();

    void PrepareMaterial(NMaterial* material);

    void SetDrawWired(bool isWire);
    bool IsDrawWired() const;

    void SetDrawMorphing(bool drawMorph);
    bool IsDrawMorphing() const;

    void SetUseMorphing(bool useMorph);
    bool IsUseMorphing() const;

    void GetTangentBasis(uint32 x, uint32 y, Vector3& normalOut, Vector3& tangentOut) const;

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

    RenderMode renderMode;

    FilePath heightmapPath;
    Heightmap* heightmap = nullptr;
    LandscapeSubdivision* subdivision = nullptr;

    NMaterial* landscapeMaterial = nullptr;
    FoliageSystem* foliageSystem = nullptr;

    uint32 heightmapSizePow2 = 0;
    float32 heightmapSizef = 0.f;

    uint32 drawIndices = 0;

    bool updatable = false;

    bool debugDrawMetrics = false;
    bool debugDrawMorphing = false;

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
    int16 AllocateParcelVertexBuffer(uint32 x, uint32 y, uint32 size);

    void DrawLandscapeNoInstancing();
    void DrawPatchNoInstancing(uint32 level, uint32 x, uint32 y, uint32 xNegSizePow2, uint32 yNegSizePow2, uint32 xPosSizePow2, uint32 yPosSizePow2);

    void FlushQueue();
    void ClearQueue();

    inline uint16 GetVertexIndex(uint16 x, uint16 y);

    void ResizeIndicesBufferIfNeeded(DAVA::uint32 newSize);

    Vector<rhi::HVertexBuffer> vertexBuffers;
    std::vector<uint16> indices;

    uint32 vLayoutUIDNoInstancing = rhi::VertexLayout::InvalidUID;

    uint32 queueIndexCount = 0;
    int16 queuedQuadBuffer = 0;
    int32 flushQueueCounter = 0;

    uint32 quadsInWidthPow2 = 0;

    bool isRequireTangentBasis = false;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Instancing render

    struct VertexInstancing
    {
        Vector2 position;
        Vector2 edgeShiftDirection;
        Vector4 edgeMask;

        float32 edgeVertexIndex;
        float32 edgeMaskNull;
    };

    struct InstanceData
    {
        Vector2 patchOffset;
        float32 patchScale;
        Vector4 neighbourPatchLodOffset; // per edge: left, right, bottom, top

        //Members for morphing case
        Vector4 neighbourPatchMorph;
        float32 patchLod;
        float32 patchMorph;
        float32 centerPixelOffset;
    };

    static const int32 INSTANCE_DATA_SIZE_MORPHING = sizeof(InstanceData);
    static const int32 INSTANCE_DATA_SIZE = INSTANCE_DATA_SIZE_MORPHING - sizeof(Vector4) - 3 * sizeof(float32);

    struct InstanceDataBuffer
    {
        rhi::HVertexBuffer buffer;
        rhi::HSyncObject syncObject;
        uint32 bufferSize;
    };

    void AllocateGeometryDataInstancing();

    static Texture* CreateHeightTexture(Heightmap* heightmap, RenderMode renderMode);
    static Vector<Image*> CreateHeightTextureData(Heightmap* heightmap, RenderMode renderMode);

    Texture* CreateTangentTexture();
    Vector<Image*> CreateTangentBasisTextureData();

    void DrawLandscapeInstancing();
    void DrawPatchInstancing(uint32 level, uint32 xx, uint32 yy, const Vector4& neighborLevel, float32 patchMorph = 0.f, const Vector4& neighborMorph = Vector4());

    Texture* heightTexture = nullptr;
    Texture* tangentTexture = nullptr;

    rhi::HVertexBuffer patchVertexBuffer;
    rhi::HIndexBuffer patchIndexBuffer;
    uint8* instanceDataPtr = nullptr;
    uint32 instanceDataMaxCount = 64; //64 instances - initial value. It's will automatic enhanced if needed.
    uint32 instanceDataSize = 0;

    Vector<InstanceDataBuffer*> freeInstanceDataBuffers;
    Vector<InstanceDataBuffer*> usedInstanceDataBuffers;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////

    friend class LandscapeSystem;

public:
    INTROSPECTION_EXTEND(Landscape, RenderObject,
                         PROPERTY("heightmapPath", "Height Map Path", GetHeightmapPathname, SetHeightmapPathname, I_VIEW | I_EDIT)
                         PROPERTY("size", "Size", GetLandscapeSize, SetLandscapeSize, I_VIEW | I_EDIT)
                         PROPERTY("height", "Height", GetLandscapeHeight, SetLandscapeHeight, I_VIEW | I_EDIT)
                         PROPERTY("useMorphing", "useMorphing", IsUseMorphing, SetUseMorphing, I_VIEW | I_EDIT)
                         PROPERTY("isDrawWired", "isDrawWired", IsDrawWired, SetDrawWired, I_VIEW | I_EDIT)
                         PROPERTY("debugDrawMorphing", "debugDrawMorphing", IsDrawMorphing, SetDrawMorphing, I_VIEW | I_EDIT)
                         MEMBER(debugDrawMetrics, "debugDrawMetrics", I_VIEW | I_EDIT)
                         MEMBER(subdivision, "subdivision", I_VIEW | I_EDIT)
                         );
};

// Inline functions
inline uint16 Landscape::GetVertexIndex(uint16 x, uint16 y)
{
    return x + y * RENDER_PARCEL_SIZE_VERTICES;
}

inline LandscapeSubdivision* Landscape::GetSubdivision()
{
    return subdivision;
}

};

#endif // __DAVAENGINE_LANDSCAPE_NODE_H__
