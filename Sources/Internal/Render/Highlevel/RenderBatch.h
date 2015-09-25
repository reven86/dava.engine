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

#ifndef __DAVAENGINE_SCENE3D_RENDER_BATCH_H__
#define __DAVAENGINE_SCENE3D_RENDER_BATCH_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/FastName.h"
#include "Render/RenderBase.h"
#include "Base/BaseMath.h"

#include "Render/3D/PolygonGroup.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Material/NMaterial.h"

#include "Scene3D/SceneFile/SerializationContext.h"
namespace DAVA
{
/*
class RenderCallInstance
{
public:
    VBO *
    IBO *
    NMaterialInstance *
    NMaterial *
    uint32 start;
    uint32 count;
    uint32 primitiveType;
};
*/
class RenderLayer;
class Camera;
class RenderObject;
class RenderBatch;
class NMaterial;
class NMaterialInstance;
class OcclusionQuery;

/*
    Not finished. We'll return to it when we start batching functionality.
 */
class IRenderBatchDataSource
{
public:
    virtual void InitBatch(RenderBatch* renderBatch) = 0;
    virtual void FillData(RenderBatch* renderBatch) = 0;
    virtual void ReleaseBatch(RenderBatch* renderBatch) = 0;
};

class RenderBatch : public BaseObject
{
protected:
    virtual ~RenderBatch();

public:
    RenderBatch();

    void SetPolygonGroup(PolygonGroup* _polygonGroup);
    inline PolygonGroup* GetPolygonGroup();

    void SetMaterial(NMaterial* _material);
    inline NMaterial* GetMaterial();

    void SetRenderObject(RenderObject* renderObject);
    inline RenderObject* GetRenderObject() const;

    inline void SetStartIndex(uint32 _startIndex);
    inline void SetIndexCount(uint32 _indexCount);

    const AABBox3& GetBoundingBox() const;

    virtual void GetDataNodes(Set<DataNode*>& dataNodes);
    virtual RenderBatch* Clone(RenderBatch* destination = 0);
    virtual void Save(KeyedArchive* archive, SerializationContext* serializationContext);
    virtual void Load(KeyedArchive* archive, SerializationContext* serializationContext);

    /*
        \brief This is additional sorting key. It should be from 0 to 15.
     */
    void SetSortingKey(uint32 key);
    inline uint32 GetSortingKey() const;

    /*sorting offset allowed in 0..31 range, 15 default, more - closer to camera*/
    void SetSortingOffset(uint32 offset);
    inline uint32 GetSortingOffset();

    void BindGeometryData(rhi::Packet& packet);

    void UpdateAABBoxFromSource();

    pointer_size layerSortingKey;

    rhi::HVertexBuffer vertexBuffer;
    uint32 vertexCount;
    uint32 vertexBase;
    rhi::HIndexBuffer indexBuffer;
    uint32 startIndex;
    uint32 indexCount;

    rhi::PrimitiveType primitiveType;
    uint32 vertexLayoutId;

private:
    PolygonGroup* dataSource;

    NMaterial* material; // Should be replaced to NMaterial
    RenderObject* renderObject;
    //Matrix4* sortingTransformPtr;

    uint32 sortingKey; //oooookkkk -where o is offset, k is key

    const static uint32 SORTING_KEY_MASK = 0x0f;
    const static uint32 SORTING_OFFSET_MASK = 0x1f0;
    const static uint32 SORTING_OFFSET_SHIFT = 4;
    const static uint32 SORTING_KEY_DEF_VALUE = 0xf8;

    AABBox3 aabbox;

    void InsertDataNode(DataNode* node, Set<DataNode*>& dataNodes);

public:
    INTROSPECTION_EXTEND(RenderBatch, BaseObject,
                         MEMBER(dataSource, "Data Source", I_SAVE | I_VIEW | I_EDIT)

                         MEMBER(startIndex, "Start Index", I_SAVE)
                         MEMBER(indexCount, "Index Count", I_SAVE)
                         //MEMBER(primitiveType, InspDesc("primitiveType", GlobalEnumMap<rhi::PrimitiveType>::Instance()), I_VIEW | I_EDIT | I_SAVE)

                         MEMBER(aabbox, "AABBox", I_SAVE | I_VIEW | I_EDIT)
                         MEMBER(material, "Material", I_VIEW | I_EDIT)

                         PROPERTY("sortingKey", "Key for the sorting inside render layer", GetSortingKey, SetSortingKey, I_SAVE | I_VIEW | I_EDIT));
};

inline PolygonGroup* RenderBatch::GetPolygonGroup()
{
    return dataSource;
}

inline NMaterial* RenderBatch::GetMaterial()
{
    return material;
}

inline RenderObject* RenderBatch::GetRenderObject() const
{
    return renderObject;
}

inline void RenderBatch::SetStartIndex(uint32 _startIndex)
{
    startIndex = _startIndex;
}

inline void RenderBatch::SetIndexCount(uint32 _indexCount)
{
    indexCount = _indexCount;
}

inline uint32 RenderBatch::GetSortingKey() const
{
    return sortingKey & SORTING_KEY_MASK;
}

inline uint32 RenderBatch::GetSortingOffset()
{
    return ((sortingKey & SORTING_OFFSET_MASK) >> SORTING_OFFSET_SHIFT);
}

inline void RenderBatch::BindGeometryData(rhi::Packet& packet)
{
    if (dataSource)
    {
        packet.vertexStreamCount = 1;
        packet.vertexStream[0] = dataSource->vertexBuffer;
        packet.baseVertex = 0;
        packet.vertexCount = dataSource->vertexCount;
        packet.indexBuffer = dataSource->indexBuffer;
        packet.primitiveType = dataSource->primitiveType;
        packet.primitiveCount = GetPrimitiveCount(dataSource->indexCount, dataSource->primitiveType); //later move it into pg!
        packet.vertexLayoutUID = dataSource->vertexLayoutId;
        packet.startIndex = 0;
    }
    else
    {
        packet.vertexStreamCount = 1;
        packet.vertexStream[0] = vertexBuffer;
        packet.baseVertex = vertexBase;
        packet.vertexCount = vertexCount;
        packet.indexBuffer = indexBuffer;
        packet.primitiveType = primitiveType;
        packet.primitiveCount = GetPrimitiveCount(indexCount, primitiveType);
        packet.vertexLayoutUID = vertexLayoutId;
        packet.startIndex = startIndex;
    }
}

} //

#endif /* __DAVAENGINE_SCENE3D_RENDER_BATCH_H__ */
