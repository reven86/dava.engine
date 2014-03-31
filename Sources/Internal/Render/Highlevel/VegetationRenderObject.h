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

#ifndef __DAVAENGINE_VEGETATIONRENDEROBJECT_H__
#define __DAVAENGINE_VEGETATIONRENDEROBJECT_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/FastName.h"
#include "Render/RenderBase.h"
#include "Base/BaseMath.h"
#include "Base/AbstractQuadTree.h"

#include "Render/3D/PolygonGroup.h"
#include "Render/RenderDataObject.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Material.h"
#include "Render/Material/NMaterial.h"

#include "Scene3D/SceneFile/SerializationContext.h"

#define MAX_CELL_TEXTURE_COORDS 4

namespace DAVA
{
    
typedef Image VegetationMap;
    
struct TextureSheetCell
{
    Vector2 coords[MAX_CELL_TEXTURE_COORDS];
    uint32 geometryId;
    Vector2 geometryScale;
    
    inline TextureSheetCell();
    
    inline TextureSheetCell& operator=(const TextureSheetCell& src);
};
 
class Heightmap;
class TextureSheet
{
public:
    
    Vector<TextureSheetCell> cells;
    
    inline TextureSheet& operator=(const TextureSheet& src);

    void Load(const FilePath &yamlPath);
};

class VegetationRenderObject : public RenderObject
{
public:
        
    VegetationRenderObject();
    virtual ~VegetationRenderObject();
        
    RenderObject * Clone(RenderObject *newObject);
    virtual void Save(KeyedArchive *archive, SerializationContext *serializationContext);
    virtual void Load(KeyedArchive *archive, SerializationContext *serializationContext);
    void PrepareRenderData();
    
    virtual void PrepareToRender(Camera *camera);
    
    void SetHeightmap(Heightmap* _heightmap);
    Heightmap* GetHeightmap() const;
    const FilePath& GetHeightmapPath() const;
    void SetHeightmapPath(const FilePath& path);

    void SetVegetationMap(VegetationMap* map);
    void SetVegetationMap(const FilePath& path);
    VegetationMap* GetVegetationMap() const;
    void SetVegetationMapPath(const FilePath& path);
    const FilePath& GetVegetationMapPath() const;
    
    void SetLightmap(const FilePath& filePath);
    Texture* GetLightmap() const;
    const FilePath& GetLightmapPath() const;
    
    const TextureSheet& GetTextureSheet() const;
    void SetTextureSheet(const FilePath& path);
    const FilePath& GetTextureSheetPath() const;
    
    void SetVegetationTexture(const FilePath& texture);
    const FilePath& GetVegetationTexture() const;
    
    void SetClusterLimit(const uint32& maxClusters);
    uint32 GetClusterLimit() const;
    
    void SetWorldSize(const Vector3 size);
    const Vector3& GetWorldSize() const;
    
    void SetVisibilityDistance(const Vector2& distances);
    const Vector2& GetVisibilityDistance() const;
    void ResetVisibilityDistance();
    
    void SetLodRange(const Vector3& distances);
    const Vector3& GetLodRange() const;
    void ResetLodRanges();
    
    void SetMaxVisibleQuads(const uint32& _maxVisibleQuads);
    const uint32& GetMaxVisibleQuads() const;
    
    virtual void GetDataNodes(Set<DataNode*> & dataNodes);
    
    void SetPerturbation(const Vector3& point, const Vector3& force, float32 distance);
    float32 GetPerturbationDistance() const;
    const Vector3& GetPerturbationForce() const;
    void SetPerturbationPoint(const Vector3& point);
    const Vector3& GetPerturbationPoint() const;
    
    void SetLayerVisibilityMask(const uint8& mask);
    const uint8& GetLayerVisibilityMask() const;
    
    void SetVegetationVisible(bool show);
    bool GetVegetationVisible() const;
    
private:

    struct VegetationVertex
    {
        Vector3 coord;
        Vector3 normal;
        Vector3 binormal;
        Vector3 tangent;
        Vector2 texCoord0;
        Vector2 texCoord1;
    };
    
    struct SpatialData
    {
        int16 x;
        int16 y;
        int16 width;
        int16 height;
        int8 rdoIndex;
        
        AABBox3 bbox;
        //Vector3 refPoint;
        float32 cameraDistance;
        uint8 clippingPlane;
        
        inline SpatialData();
        inline SpatialData& operator=(const SpatialData& src);
        inline bool IsEmpty(uint32 cellValue) const;
        inline bool IsRenderable() const;
        inline int16 GetResolutionId() const;
        inline bool IsVisibleInResolution(uint32 resolutionId, uint32 maxResolutions) const;
    };
    
    //void BuildVegetationBrush(uint32 maxClusters);
    RenderBatch* GetRenderBatchFromPool(NMaterial* material);
    void ReturnToPool(int32 batchCount);

    bool IsValidGeometryData() const;
    bool IsValidSpatialData() const;
    
    Vector2 GetVegetationUnitWorldSize(float32 resolution) const;
    
    void BuildSpatialStructure(VegetationMap* vegMap);
    void BuildSpatialQuad(AbstractQuadTreeNode<SpatialData>* node,
                          AbstractQuadTreeNode<SpatialData>* firstRenderableParent,
                          int16 x, int16 y,
                          uint16 width, uint16 height,
                          AABBox3& parentBox);
    
    void BuildVisibleCellList(const Vector3& cameraPoint,
                              Frustum* frustum,
                              Vector<AbstractQuadTreeNode<SpatialData>*>& cellList);
    void BuildVisibleCellList(const Vector3& cameraPoint,
                              Frustum* frustum,
                              uint8 planeMask,
                              AbstractQuadTreeNode<SpatialData>* node,
                              Vector<AbstractQuadTreeNode<SpatialData>*>& cellList);
    inline void AddVisibleCell(AbstractQuadTreeNode<SpatialData>* node,
                               float32 refDistance,
                               uint32 cellValue,
                               Vector<AbstractQuadTreeNode<SpatialData>*>& cellList);
    
    static bool CellByDistanceCompareFunction(const AbstractQuadTreeNode<SpatialData>* a, const AbstractQuadTreeNode<SpatialData>*  b);
    
    void InitHeightTextureFromHeightmap(Heightmap* heightMap);
    
    float32 SampleHeight(int16 x, int16 y);
    
    void UpdateVegetationSetup();
    void InitLodRanges();
    
    void SetupHeightmapParameters(BaseObject * caller, void * param, void *callerData);
    
    void CreateRenderData(uint32 maxClusters);
    void ReleaseRenderData();
    bool ReadyToRender(bool externalRenderFlag);
    
    void SetupNodeUniforms(AbstractQuadTreeNode<SpatialData>* node,
                           float32 cameraDistance,
                           Vector<float32>& uniforms);
    
private:
    
    Heightmap* heightmap;
    VegetationMap* vegetationMap;
    TextureSheet textureSheet;
    uint32 clusterLimit;
    Vector3 worldSize;
    Vector<Vector2> unitWorldSize;
    Vector2 heightmapScale;
    Vector2 heightmapToVegetationMapScale;
    uint16 halfWidth;
    uint16 halfHeight;
    
    Vector<RenderBatch*> renderBatchPool;
    int32 renderBatchPoolLine;
    
    NMaterial* vegetationMaterial;
    Vector<float32> shaderScaleDensityUniforms;
    
    Vector<VegetationVertex> vertexData;
    Vector<int16> indexData;
    RenderDataObject* vertexRenderDataObject;
    Vector<Vector<RenderDataObject*> > indexRenderDataObject;
    
    AbstractQuadTree<SpatialData> quadTree;
    Vector<AbstractQuadTreeNode<SpatialData>*> visibleCells;
    
    FilePath heightmapPath;
    FilePath vegetationMapPath;
    FilePath textureSheetPath;
    
    Vector2 visibleClippingDistances;
    Vector3 lodRanges;
    uint32 maxVisibleQuads;
    
    Vector3 perturbationForce;
    Vector3 perturbationPoint;
    float32 maxPerturbationDistance;
    
    uint8 layerVisibilityMask;
    
    bool vegetationVisible;
    
public:
    
    INTROSPECTION_EXTEND(VegetationRenderObject, RenderObject,
                         PROPERTY("density", "Base density", GetClusterLimit, SetClusterLimit, I_SAVE | I_EDIT | I_VIEW)
                         PROPERTY("densityMap", "Density map", GetVegetationMapPath, SetVegetationMap, I_SAVE | I_EDIT | I_VIEW)
                         PROPERTY("lightmap", "Lightmap", GetLightmapPath, SetLightmap, I_SAVE | I_EDIT | I_VIEW)
                         PROPERTY("textureSheet", "Texture sheet", GetTextureSheetPath, SetTextureSheet, I_SAVE | I_EDIT | I_VIEW)
                         PROPERTY("vegetationTexture", "Vegetation texture", GetVegetationTexture, SetVegetationTexture, I_SAVE | I_EDIT | I_VIEW)
                         PROPERTY("lodRanges", "Lod ranges", GetLodRange, SetLodRange, I_EDIT | I_VIEW)
                         PROPERTY("visibilityDistance", "Visibility distances", GetVisibilityDistance, SetVisibilityDistance, I_EDIT | I_VIEW)
                         PROPERTY("maxVisibleQuads", "Max visible quads", GetMaxVisibleQuads, SetMaxVisibleQuads, I_EDIT | I_VIEW)
                         );
    
};
    
    
inline TextureSheetCell::TextureSheetCell() :
        geometryId(0),
        geometryScale(1.0f, 1.0f)
{
}
    
inline TextureSheetCell& TextureSheetCell::operator=(const TextureSheetCell& src)
{
    coords[0] = src.coords[0];
    coords[1] = src.coords[1];
    coords[2] = src.coords[2];
    coords[3] = src.coords[3];
    
    geometryId = src.geometryId;
    geometryScale = src.geometryScale;
    
    return *this;
}
    
inline TextureSheet& TextureSheet::operator=(const TextureSheet& src)
{
    cells.resize(src.cells.size());
    
    size_t size = cells.size();
    for(size_t i = 0; i < size; ++i)
    {
        cells[i] = src.cells[i];
    }
    
    return *this;
}
    
inline VegetationRenderObject::SpatialData::SpatialData()  :
        x(-1),
        y(-1),
        cameraDistance(0.0f),
        clippingPlane(0),
        rdoIndex(-1)
{
}
    
inline VegetationRenderObject::SpatialData& VegetationRenderObject::SpatialData::operator=(const VegetationRenderObject::SpatialData& src)
{
    x = src.x;
    y = src.y;
    bbox = src.bbox;
    cameraDistance = src.cameraDistance;
    clippingPlane = src.clippingPlane;
    width = src.width;
    height = src.height;
    rdoIndex = src.rdoIndex;
    
    return *this;
}

inline bool VegetationRenderObject::SpatialData::IsEmpty(uint32 cellValue) const
{
    return (0 == (cellValue & 0x0F0F0F0F));
}

inline bool VegetationRenderObject::SpatialData::IsVisibleInResolution(uint32 resolutionId, uint32 maxResolutions) const
{
    uint32 refResolution = ((x * y) % maxResolutions);
    return (refResolution >= resolutionId);
}

inline bool VegetationRenderObject::SpatialData::IsRenderable() const
{
    return (width > 0 && height > 0);
}

inline int16 VegetationRenderObject::SpatialData::GetResolutionId() const
{
    return (width * height);
}
    
inline void VegetationRenderObject::AddVisibleCell(AbstractQuadTreeNode<SpatialData>* node,
                                                   float32 refDistance,
                                                   uint32 cellValue,
                                                   Vector<AbstractQuadTreeNode<SpatialData>*>& cellList)
{
    if(!node->data.IsEmpty(cellValue))
    {
        if(node->data.cameraDistance <= refDistance)
        {
                //Logger::FrameworkDebug("VegetationRenderObject::AddVisibleCell x = %d, y = %d", data->x, data->y);
            cellList.push_back(node);
        }
    }
}

};

#endif
