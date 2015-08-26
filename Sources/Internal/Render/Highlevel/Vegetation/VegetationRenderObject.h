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

#include <memory>

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/FastName.h"
#include "Render/RenderBase.h"
#include "Base/BaseMath.h"
#include "Base/AbstractQuadTree.h"
#include "Render/Image/Image.h"

#include "Render/3D/PolygonGroup.h"
#include "Render/RenderDataObject.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Material.h"
#include "Render/Material/NMaterial.h"
#include "Render/Highlevel/Heightmap.h"

#include "Scene3D/SceneFile/SerializationContext.h"

#include "Render/Highlevel/Vegetation/VegetationPropertyNames.h"
#include "Render/Highlevel/Vegetation/RenderBatchPool.h"
#include "Render/Highlevel/Vegetation/VegetationRenderData.h"
#include "Render/Highlevel/Vegetation/VegetationSpatialData.h"
#include "Render/Highlevel/Vegetation/VegetationCustomGeometrySerializationData.h"
#include "Render/Highlevel/Vegetation/VegetationGeometry.h"

namespace DAVA
{
    
using VegetationMap = Image;
class FoliageSystem;

/**
 \brief This structure stores various vegetation metrics useful for performance
    analysis.
 */
struct VegetationMetrics
{
    Vector<uint32> visibleInstanceCountPerLayer;
    Vector<uint32> visibleInstanceCountPerLOD;
    Vector<uint32> instanceCountPerLOD;
    Vector<uint32> instanceCountPerLayer;
    
    Vector<uint32> visiblePolyCountPerLayer;
    Vector<uint32> visiblePolyCountPerLOD;
    Vector<uint32> polyCountPerLOD;
    Vector<uint32> polyCountPerLayer;
    
    Vector<Vector<uint32> > polyCountPerLayerPerLod; //layer-lod

    uint32 totalQuadTreeLeafCount;
    Vector<uint32> quadTreeLeafCountPerLOD;
    
    uint32 renderBatchCount;
    
    bool isValid;
};

/**
 \brief Vegetation rendering implementation. Performs frustum culling, selects geometry to render,
    provides list of render batches for render system.
    Use of external vegetation geometry (subclasses of VegetationGeometry) allows to easily switch
    between vegetation render modes.
 */
class VegetationRenderObject : public RenderObject
{
public:
        
    VegetationRenderObject();
    virtual ~VegetationRenderObject();
        
    RenderObject * Clone(RenderObject *newObject);
    virtual void Save(KeyedArchive *archive, SerializationContext *serializationContext);
    virtual void Load(KeyedArchive *archive, SerializationContext *serializationContext);
    
    virtual void PrepareToRender(Camera *camera);
    virtual void RecalcBoundingBox();
    
    void CollectMetrics(VegetationMetrics& metrics);
    void DebugDrawVisibleNodes();
    virtual void GetDataNodes(Set<DataNode*> & dataNodes);
    
    inline void SetHeightmap(Heightmap* _heightmap);
    inline Heightmap* GetHeightmap() const;
    inline const FilePath& GetHeightmapPath() const;
    inline void SetHeightmapPath(const FilePath& path);
    
    inline void SetLightmap(const FilePath& filePath);
    inline void SetLightmapAndGenerateDensityMap(const FilePath& filePath);
    inline const FilePath& GetLightmapPath() const;
    
    inline void SetTextureSheet(const FilePath& path);
    inline const FilePath& GetTextureSheetPath() const;
    
    inline void SetVegetationTexture(const FilePath& texture);
    inline const FilePath& GetVegetationTexture() const;
    
    inline void SetClusterLimit(const uint32& maxClusters);
    inline uint32 GetClusterLimit() const;
    
    inline void SetWorldSize(const Vector3 size);
    inline const Vector3& GetWorldSize() const;
    
    inline void SetVisibilityDistance(const Vector2& distances);
    inline const Vector2& GetVisibilityDistance() const;
    void ResetVisibilityDistance();
    
    inline void SetLodRange(const Vector3& distances);
    inline const Vector3& GetLodRange() const;
    inline void ResetLodRanges();
    
    inline void SetMaxVisibleQuads(const uint32& _maxVisibleQuads);
    inline const uint32& GetMaxVisibleQuads() const;
    
    inline void SetPerturbation(const Vector3& point, const Vector3& force, float32 distance);
    inline float32 GetPerturbationDistance() const;
    inline const Vector3& GetPerturbationForce() const;
    inline void SetPerturbationPoint(const Vector3& point);
    inline const Vector3& GetPerturbationPoint() const;
    
    inline void SetLayerVisibilityMask(const uint8& mask);
    inline const uint8& GetLayerVisibilityMask() const;
    
    inline void SetVegetationVisible(bool show);
    inline bool GetVegetationVisible() const;
    
    inline const FilePath& GetCustomGeometryPath() const;
    void SetCustomGeometryPath(const FilePath& path);
    
    inline void SetCameraBias(const float32& bias);
    inline float32 GetCameraBias() const;
    
    inline void SetLayerClusterLimit(const Vector4& maxClusters);
    inline Vector4 GetLayerClusterLimit() const;

    inline void SetScaleVariation(const Vector4& scaleVariation);
    inline Vector4 GetScaleVariation() const;
    
    inline void SetRotationVariation(const Vector4& rotationVariation);
    inline Vector4 GetRotationVariation() const;

    inline void SetLayersAnimationAmplitude(const Vector4 & ampitudes);
    inline Vector4 GetLayersAnimationAmplitude() const;
    
    inline  void SetLayersAnimationSpring(const Vector4 & spring);
    inline Vector4 GetLayersAnimationSpring() const;
    
    inline void SetLayerAnimationDragCoefficient(const Vector4& drag);
    inline const Vector4& GetLayerAnimationDragCoefficient() const;
    
    static bool IsHardwareCapableToRenderVegetation();
private:
    
    bool IsValidGeometryData() const;
    bool IsValidSpatialData() const;
    
    Vector2 GetVegetationUnitWorldSize(float32 resolution) const;
    
    void BuildSpatialStructure();
    void BuildSpatialQuad(AbstractQuadTreeNode<VegetationSpatialData>* node,
                          AbstractQuadTreeNode<VegetationSpatialData>* firstRenderableParent,
                          int16 x, int16 y,
                          uint16 width, uint16 height,
                          AABBox3& parentBox);
    
    Vector<AbstractQuadTreeNode<VegetationSpatialData>*> & BuildVisibleCellList(Camera * forCamera);
    
    void BuildVisibleCellList(const Vector3& cameraPoint,
                              Frustum* frustum,
                              uint8 planeMask,
                              AbstractQuadTreeNode<VegetationSpatialData>* node,
                              Vector<AbstractQuadTreeNode<VegetationSpatialData>*>& cellList,
                              bool evaluateVisibility);
    inline void AddVisibleCell(AbstractQuadTreeNode<VegetationSpatialData>* node,
                               float32 refDistance,
                               Vector<AbstractQuadTreeNode<VegetationSpatialData>*>& cellList);
    
    static bool CellByDistanceCompareFunction(const AbstractQuadTreeNode<VegetationSpatialData>* a, const AbstractQuadTreeNode<VegetationSpatialData>*  b);
    
    void InitHeightTextureFromHeightmap(Heightmap* heightMap);
    
    float32 SampleHeight(int16 x, int16 y);
    
    void UpdateVegetationSetup();
    void InitLodRanges();
    
	void SetupHeightmapParameters(Texture* tx);
    
    void CreateRenderData();
    
    bool ReadyToRender();
    
    size_t SelectDirectionIndex(const Vector3& cameraDirection, Vector<VegetationSortedBufferItem>& buffers);
    
    inline uint32 MapToResolution(float32 squareDistance);
    
    inline bool IsNodeEmpty(AbstractQuadTreeNode<VegetationSpatialData>* node,
                            const Vector<bool>& map) const;
    
    void ClearRenderBatches();
    
    void InitWithFixedGeometry(FastNameSet& materialFlags);
    void InitWithCustomGeometry(FastNameSet& materialFlags);
    
    void SetCustomGeometryPathInternal(const FilePath& path);
    VegetationCustomGeometrySerializationDataPtr LoadCustomGeometryData(SerializationContext* context, KeyedArchive* srcArchive);
    void SaveCustomGeometryData(SerializationContext* context, KeyedArchive* dstArchive, const VegetationCustomGeometrySerializationDataPtr& data);
    
    void GenerateDensityMapFromTransparencyMask(FilePath lightmapPath,
                                           Vector<bool>& densityMapBits);
    Image* LoadSingleImage(const FilePath& path) const;
    float32 GetMeanAlpha(uint32 x, uint32 y,
                        uint32 ratio,
                        uint32 stride,
                        Image* src) const;
    
    void SetDensityMap(const Vector<bool>& densityBits);
    
    void PrepareToRenderMultipleMaterials(Camera *camera);
    void PrepareToRenderSingleMaterial(Camera *camera);
    
    bool IsDataLoadNeeded();
    
private:
    uint32 MapCellSquareToResolutionIndex(uint32 cellSquare);

    Heightmap* heightmap;
    Vector3 worldSize;
    Vector<Vector2> unitWorldSize;
    Vector2 heightmapScale;
    Vector2 heightmapToVegetationMapScale;
    uint16 halfWidth;
    uint16 halfHeight;
    
    RenderBatchPool renderBatchPool;
    
    //Vector<float32> shaderScaleDensityUniforms;
    
    Vector<VegetationRenderData*> renderData;
    
    AbstractQuadTree<VegetationSpatialData> quadTree;
    Vector<AbstractQuadTreeNode<VegetationSpatialData>*> visibleCells;
    
    FilePath heightmapPath;
    FilePath textureSheetPath;
    FilePath albedoTexturePath;
    FilePath lightmapTexturePath;
    
    FilePath customGeometryPath;
    
    Vector2 visibleClippingDistances;
    Vector3 lodRanges;
    uint32 maxVisibleQuads;
    
    Vector3 perturbationForce;
    Vector3 perturbationPoint;
    float32 maxPerturbationDistance;
    
    uint8 layerVisibilityMask;
    
    bool vegetationVisible;
    
    Vector<Vector2> resolutionRanges;
    
    VegetationGeometry* vegetationGeometry;
    
    Texture* heightmapTexture;
    
    float32 cameraBias;
    
    VegetationCustomGeometrySerializationDataPtr customGeometryData;
    
    Vector4 layersAnimationAmplitude;
    Vector4 layersAnimationSpring;
    Vector4 layersAnimationDrag;
    
    Vector<bool> densityMap;
    
    Vector<VegetationLayerParams> layerParams;
    
    
public:
    
    INTROSPECTION_EXTEND(VegetationRenderObject, RenderObject,
                         PROPERTY("density", "Base density", GetLayerClusterLimit, SetLayerClusterLimit, I_SAVE | I_EDIT | I_VIEW)
                         PROPERTY("scaleVariation", "Scale variation", GetScaleVariation, SetScaleVariation, I_SAVE | I_EDIT | I_VIEW)
                         PROPERTY("rotationVariation", "Rotation variation", GetRotationVariation, SetRotationVariation, I_SAVE | I_EDIT | I_VIEW)
                         PROPERTY("lightmap", "Lightmap", GetLightmapPath, SetLightmapAndGenerateDensityMap, I_SAVE | I_EDIT | I_VIEW)
                         //PROPERTY("textureSheet", "Texture sheet", GetTextureSheetPath, SetTextureSheet, I_SAVE | I_EDIT | I_VIEW)
                         //PROPERTY("vegetationTexture", "Vegetation texture", GetVegetationTexture, SetVegetationTexture, I_SAVE | I_EDIT | I_VIEW)
                         PROPERTY("lodRanges", "Lod ranges", GetLodRange, SetLodRange, I_EDIT | I_VIEW)
                         PROPERTY("visibilityDistance", "Visibility distances", GetVisibilityDistance, SetVisibilityDistance, I_EDIT | I_VIEW)
                         PROPERTY("maxVisibleQuads", "Max visible quads", GetMaxVisibleQuads, SetMaxVisibleQuads, I_EDIT | I_VIEW)
                         PROPERTY("customGeometry", "Custom geometry", GetCustomGeometryPath, SetCustomGeometryPath, I_SAVE | I_EDIT | I_VIEW)
                         PROPERTY("cameraBias", "Camera Bias", GetCameraBias, SetCameraBias, I_EDIT | I_VIEW)
                         PROPERTY("animationAmplitude", "Animation Amplitude", GetLayersAnimationAmplitude, SetLayersAnimationAmplitude, I_SAVE | I_EDIT | I_VIEW)
                         PROPERTY("animationSpring", "Animation Spring", GetLayersAnimationSpring, SetLayersAnimationSpring, I_SAVE | I_EDIT | I_VIEW)
                         PROPERTY("animationDrag", "Animation Drag", GetLayerAnimationDragCoefficient, SetLayerAnimationDragCoefficient, I_SAVE | I_EDIT | I_VIEW)
                         );
    
    friend class FoliageSystem;
};
    
inline void VegetationRenderObject::AddVisibleCell(AbstractQuadTreeNode<VegetationSpatialData>* node,
                                                   float32 refDistance,
                                                   Vector<AbstractQuadTreeNode<VegetationSpatialData>*>& cellList)
{
    if(node->data.isVisible && node->data.cameraDistance <= refDistance)
    {
        cellList.push_back(node);
    }
}

inline uint32 VegetationRenderObject::MapToResolution(float32 squareDistance)
{
    uint32 resolutionId = 0;
        
    size_t rangesCount = resolutionRanges.size();
    for(size_t i = 0; i < rangesCount; ++i)
    {
        if(squareDistance > resolutionRanges[i].x &&
            squareDistance <= resolutionRanges[i].y)
        {
            resolutionId = (uint32)i;
            break;
        }
    }
    
    return resolutionId;
}

inline bool VegetationRenderObject::IsNodeEmpty(AbstractQuadTreeNode<VegetationSpatialData>* node,
                                                const Vector<bool>& map) const
{
    bool nodeEmpty = true;
    
    int32 maxX = node->data.x + node->data.width;
    int32 maxY = node->data.y + node->data.height;
    
    uint32 fullWidth = (halfWidth << 1);
    
    for(int32 y = node->data.y; y < maxY; ++y)
    {
        for(int32 x = node->data.x; x < maxX; ++x)
        {
            int32 mapX = x + halfWidth;
            int32 mapY = y + halfHeight;
            uint32 cellDescriptionIndex = (mapY * fullWidth) + mapX;
            
            if(map[cellDescriptionIndex])
            {
                nodeEmpty = false;
                break;
            }
        }
    }
    
    return nodeEmpty;
}

inline void VegetationRenderObject::SetHeightmap(Heightmap* _heightmap)
{
    if(heightmap != _heightmap)
    {
        SafeRelease(heightmap);
        heightmap = (_heightmap->Data()) ? SafeRetain(_heightmap) : NULL;
        
        if(heightmap)
        {
            InitHeightTextureFromHeightmap(heightmap);
        }
        
        UpdateVegetationSetup();
    }
}

inline Heightmap* VegetationRenderObject::GetHeightmap() const
{
    return heightmap;
}

inline const FilePath& VegetationRenderObject::GetHeightmapPath() const
{
    return heightmapPath;
}

inline void VegetationRenderObject::SetHeightmapPath(const FilePath& path)
{
    heightmapPath = path;
}

inline void VegetationRenderObject::SetLightmap(const FilePath& filePath)
{
    lightmapTexturePath = filePath;
    
    if(vegetationGeometry != NULL)
    {
        KeyedArchive* props = new KeyedArchive();
        props->SetString(VegetationPropertyNames::UNIFORM_SAMPLER_VEGETATIONMAP.c_str(), lightmapTexturePath.GetStringValue());
        
        vegetationGeometry->OnVegetationPropertiesChanged(renderData, props);
        
        SafeRelease(props);
    }
}

inline const FilePath& VegetationRenderObject::GetLightmapPath() const
{
    return lightmapTexturePath;
}

inline void VegetationRenderObject::SetLightmapAndGenerateDensityMap(const FilePath& filePath)
{
    SetLightmap(filePath);
    GenerateDensityMapFromTransparencyMask(filePath, densityMap);
    
    UpdateVegetationSetup();
}

inline void VegetationRenderObject::SetTextureSheet(const FilePath& path)
{
    textureSheetPath = path;
    
    UpdateVegetationSetup();
}

inline const FilePath& VegetationRenderObject::GetTextureSheetPath() const
{
    return textureSheetPath;
}

inline void VegetationRenderObject::SetVegetationTexture(const FilePath& texturePath)
{
    albedoTexturePath = texturePath;
    
    if(vegetationGeometry != NULL)
    {
        KeyedArchive* props = new KeyedArchive();
        props->SetString(NMaterial::TEXTURE_ALBEDO.c_str(), albedoTexturePath.GetStringValue());
        
        vegetationGeometry->OnVegetationPropertiesChanged(renderData, props);
        
        SafeRelease(props);
    }
}

inline const FilePath& VegetationRenderObject::GetVegetationTexture() const
{
    return albedoTexturePath;
}

inline void VegetationRenderObject::SetClusterLimit(const uint32& maxClusters)
{
    Vector4 tmpVec((float32)maxClusters, (float32)maxClusters, (float32)maxClusters, (float32)maxClusters);
    SetLayerClusterLimit(tmpVec);
}

inline uint32 VegetationRenderObject::GetClusterLimit() const
{
    return layerParams[0].maxClusterCount;
}

inline void VegetationRenderObject::SetWorldSize(const Vector3 size)
{
    worldSize = size;
    
    UpdateVegetationSetup();
}

inline const Vector3& VegetationRenderObject::GetWorldSize() const
{
    return worldSize;
}

inline void VegetationRenderObject::SetVisibilityDistance(const Vector2& distances)
{
    visibleClippingDistances = distances;
}

inline const Vector2& VegetationRenderObject::GetVisibilityDistance() const
{
    return visibleClippingDistances;
}

inline void VegetationRenderObject::SetLodRange(const Vector3& distances)
{
    lodRanges = distances;
    
    if(IsValidSpatialData())
    {
        InitLodRanges();
    }
}

inline const Vector3& VegetationRenderObject::GetLodRange() const
{
    return lodRanges;
}

inline void VegetationRenderObject::SetMaxVisibleQuads(const uint32& _maxVisibleQuads)
{
    maxVisibleQuads = _maxVisibleQuads;
}

inline const uint32& VegetationRenderObject::GetMaxVisibleQuads() const
{
    return maxVisibleQuads;
}

inline void VegetationRenderObject::SetPerturbation(const Vector3& point,
                                             const Vector3& force,
                                             float32 distance)
{
    perturbationForce = force;
    maxPerturbationDistance = distance;
    perturbationPoint = point;
    
    if(vegetationGeometry != NULL)
    {
        KeyedArchive* props = new KeyedArchive();
        props->SetVector3(VegetationPropertyNames::UNIFORM_PERTURBATION_FORCE.c_str(), perturbationForce);
        props->SetFloat(VegetationPropertyNames::UNIFORM_PERTURBATION_FORCE_DISTANCE.c_str(), maxPerturbationDistance);
        props->SetVector3(VegetationPropertyNames::UNIFORM_PERTURBATION_POINT.c_str(), perturbationPoint);
        
        vegetationGeometry->OnVegetationPropertiesChanged(renderData, props);
        
        SafeRelease(props);
    }
}

inline float32 VegetationRenderObject::GetPerturbationDistance() const
{
    return maxPerturbationDistance;
}

inline const Vector3& VegetationRenderObject::GetPerturbationForce() const
{
    return perturbationForce;
}

inline const Vector3& VegetationRenderObject::GetPerturbationPoint() const
{
    return perturbationPoint;
}

inline void VegetationRenderObject::SetPerturbationPoint(const Vector3& point)
{
    perturbationPoint = point;
    
    if(vegetationGeometry != NULL)
    {
        KeyedArchive* props = new KeyedArchive();
        props->SetVector3(VegetationPropertyNames::UNIFORM_PERTURBATION_POINT.c_str(), perturbationPoint);
        
        vegetationGeometry->OnVegetationPropertiesChanged(renderData, props);
        
        SafeRelease(props);
    }
}

inline void VegetationRenderObject::SetLayerVisibilityMask(const uint8& mask)
{
    layerVisibilityMask = mask;
}

inline const uint8& VegetationRenderObject::GetLayerVisibilityMask() const
{
    return layerVisibilityMask;
}

inline void VegetationRenderObject::SetVegetationVisible(bool show)
{
    vegetationVisible = show;
}

inline bool VegetationRenderObject::GetVegetationVisible() const
{
    return vegetationVisible;
}

inline const FilePath& VegetationRenderObject::GetCustomGeometryPath() const
{
    return customGeometryPath;
}

inline void VegetationRenderObject::SetCameraBias(const float32& bias)
{
    cameraBias = bias;
}

inline float32 VegetationRenderObject::GetCameraBias() const
{
    return cameraBias;
}

inline void VegetationRenderObject::SetLayerClusterLimit(const Vector4& maxClusters)
{
    size_t layerCount = layerParams.size();
    for(size_t i = 0; i < layerCount; ++i)
    {
        layerParams[i].maxClusterCount = Clamp((uint32)Abs(maxClusters.data[i]), (uint32)1, (uint32)0x00000FFF);
    }
    
    UpdateVegetationSetup();
}

inline Vector4 VegetationRenderObject::GetLayerClusterLimit() const
{
    return Vector4((float32)layerParams[0].maxClusterCount,
                   (float32)layerParams[1].maxClusterCount,
                   (float32)layerParams[2].maxClusterCount,
                   (float32)layerParams[3].maxClusterCount);
}

inline void VegetationRenderObject::SetScaleVariation(const Vector4& scaleVariation)
{
    size_t layerCount = layerParams.size();
    for(size_t i = 0; i < layerCount; ++i)
    {
        layerParams[i].instanceScaleVariation = Clamp(scaleVariation.data[i], 0.0f, 1.0f);
    }
    
    UpdateVegetationSetup();
}

inline Vector4 VegetationRenderObject::GetScaleVariation() const
{
    return Vector4(layerParams[0].instanceScaleVariation,
                   layerParams[1].instanceScaleVariation,
                   layerParams[2].instanceScaleVariation,
                   layerParams[3].instanceScaleVariation);
}

inline void VegetationRenderObject::SetRotationVariation(const Vector4& rotationVariation)
{
    size_t layerCount = layerParams.size();
    for(size_t i = 0; i < layerCount; ++i)
    {
        layerParams[i].instanceRotationVariation = Clamp(rotationVariation.data[i], 0.0f, 360.0f);
    }
    
    UpdateVegetationSetup();
}

inline Vector4 VegetationRenderObject::GetRotationVariation() const
{
    return Vector4(layerParams[0].instanceRotationVariation,
                   layerParams[1].instanceRotationVariation,
                   layerParams[2].instanceRotationVariation,
                   layerParams[3].instanceRotationVariation);
}

inline void VegetationRenderObject::SetLayersAnimationAmplitude(const Vector4 & ampitudes)
{
    layersAnimationAmplitude = ampitudes;
}

inline Vector4 VegetationRenderObject::GetLayersAnimationAmplitude() const
{
    return layersAnimationAmplitude;
}

inline void VegetationRenderObject::SetLayersAnimationSpring(const Vector4 &spring)
{
    layersAnimationSpring = spring;
    layersAnimationSpring.Clamp(.5f, 20.f);
}

inline Vector4 VegetationRenderObject::GetLayersAnimationSpring() const
{
    return layersAnimationSpring;
}

inline void VegetationRenderObject::SetLayerAnimationDragCoefficient(const Vector4& drag)
{
    layersAnimationDrag = drag;
}

inline const Vector4& VegetationRenderObject::GetLayerAnimationDragCoefficient() const
{
    return layersAnimationDrag;
}

};

#endif
