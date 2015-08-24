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


#include <cfloat>

#include "Render/Material/NMaterial.h"
#include "Render/Material/NMaterialNames.h"
#include "Render/Highlevel/Vegetation/TextureSheet.h"
#include "Render/Highlevel/Vegetation/VegetationCustomSLGeometry.h"
#include "Utils/Random.h"
#include "FileSystem/KeyedArchive.h"
#include "Render/Highlevel/Vegetation/VegetationPropertyNames.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"

namespace DAVA
{
//static const float32 MAX_ROTATION_ANGLE = 180.0f;
//static const float32 HEIGHT_VARIATION = 0.2f;


void VegetationCustomSLGeometry::CustomMaterialTransformer::TransformMaterialOnCreate(NMaterial* mat)
{
    mat->SetRuntime(true);
    mat->SetRenderLayers(1 << RENDER_LAYER_VEGETATION_ID);
}

void VegetationCustomSLGeometry::CustomGeometryLayerData::BuildBBox()
{
    bbox.Empty();
    
    size_t sourcePositionsCount = sourcePositions.size();
    for(size_t i = 0; i < sourcePositionsCount; ++i)
    {
        bbox.AddPoint(sourcePositions[i]);
    }
}

VegetationCustomSLGeometry::CustomGeometryEntityData::CustomGeometryEntityData() : material(NULL)
{
}

VegetationCustomSLGeometry::CustomGeometryEntityData::CustomGeometryEntityData(const CustomGeometryEntityData& src)
{
    material = NULL;
    SetMaterial(src.material);
    
    lods = src.lods;
}

VegetationCustomSLGeometry::CustomGeometryEntityData::~CustomGeometryEntityData()
{
    SafeRelease(material);
}

void VegetationCustomSLGeometry::CustomGeometryEntityData::SetMaterial(NMaterial* mat)
{
    if(mat != material)
    {
        SafeRelease(material);
        material = SafeRetain(mat);
    }
}

int32 VegetationCustomSLGeometry::RandomShuffleFunc(int32 limit)
{
    return (Random::Instance()->Rand() % limit);
}

bool VegetationCustomSLGeometry::PolygonByDistanceCompareFunction(const PolygonSortData& a,
                                                                const PolygonSortData& b)
{
    return a.cameraDistance > b.cameraDistance; //back to front order
}

bool VegetationCustomSLGeometry::ClusterByMatrixCompareFunction(const ClusterResolutionData& a,
                                                              const ClusterResolutionData& b)
{
    return a.cellIndex < b.cellIndex;
}

VegetationCustomSLGeometry::VegetationCustomSLGeometry(const Vector<VegetationLayerParams> & _maxClusters,
                                                   uint32 _maxDensityLevels,
                                                   const Vector2 & _unitSize,
                                                   const FilePath & _dataPath,
                                                   const Vector<uint32> & _resolutionTilesPerRow,
                                                   const Vector<uint32> & _resolutionClusterStride,
                                                   const Vector3 & _worldSize,
                                                   const VegetationCustomGeometrySerializationDataPtr& geometryData)
    : maxClusters(_maxClusters)
    , maxDensityLevels(_maxDensityLevels)
    , unitSize(_unitSize)
    , sourceDataPath(_dataPath)
    , resolutionTilesPerRow(_resolutionTilesPerRow)
    , resolutionClusterStride(_resolutionClusterStride)
    , worldSize(_worldSize)
{
    resolutionCount = static_cast<uint32>(resolutionClusterStride.size());
    
    materialTransform = new CustomMaterialTransformer();
    
    InitCustomGeometry(geometryData);
}

VegetationCustomSLGeometry::~VegetationCustomSLGeometry()
{
}

void VegetationCustomSLGeometry::Build(Vector<VegetationRenderData*>& renderDataArray,
                                       const FastNameSet& materialFlags)
{
    renderDataArray.clear();
    
    size_t customGeometryDataCount = customGeometryData.size();
    if(customGeometryDataCount == 0)
    {
        return;
    }
    
    PrepareBoundingBoxes();
    
    VegetationRenderData* renderData = new VegetationRenderData();
    renderDataArray.push_back(renderData);
    
    Vector<VegetationVertex>& vertexData = renderData->GetVertices();
    Vector<VegetationIndex>& indexData = renderData->GetIndices();
    
    Vector<ClusterPositionData> clusterPositions;
    Vector<VertexRangeData> layerClusterRanges;
    
    GenerateClusterPositionData(maxClusters,
                                clusterPositions,
                                layerClusterRanges);
    
    Vector<Vector<Vector<SortBufferData> > > resolutionDataArray;
    for(uint32 resolutionIndex = 0;
        resolutionIndex < resolutionCount;
        ++resolutionIndex)
    {
        resolutionDataArray.push_back(Vector<Vector<SortBufferData> >());
        Vector<Vector<SortBufferData> >& cellDataArray = resolutionDataArray[resolutionDataArray.size() - 1];
    
        Vector<ClusterResolutionData> clusterResolution;
        Vector<BufferCellData> cellOffsets;
        
        GenerateClusterResolutionData(resolutionIndex,
                                      maxClusters,
                                      clusterPositions,
                                      layerClusterRanges,
                                      clusterResolution);
        std::stable_sort(clusterResolution.begin(), clusterResolution.end(), ClusterByMatrixCompareFunction);
        
        GenerateVertexData(customGeometryData,
                           clusterResolution,
                           vertexData,
                           cellOffsets);
        
        size_t cellCount = cellOffsets.size();
        for(size_t cellIndex = 0; cellIndex < cellCount; ++cellIndex)
        {
            cellDataArray.push_back(Vector<SortBufferData>());
            Vector<SortBufferData>& directionOffsets = cellDataArray[cellDataArray.size() - 1];
            GenerateIndexData(customGeometryData,
                              clusterResolution,
                              cellOffsets[cellIndex],
                              vertexData,
                              indexData,
                              directionOffsets);
        }
    }
    
    RenderDataObject* vertexRDO = new RenderDataObject();
    vertexRDO->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, sizeof(VegetationVertex), &(vertexData[0].coord));
    vertexRDO->SetStream(EVF_NORMAL, TYPE_FLOAT, 3, sizeof(VegetationVertex), &(vertexData[0].normal));
    vertexRDO->SetStream(EVF_BINORMAL, TYPE_FLOAT, 3, sizeof(VegetationVertex), &(vertexData[0].binormal));
    vertexRDO->SetStream(EVF_TANGENT, TYPE_FLOAT, 3, sizeof(VegetationVertex), &(vertexData[0].tangent));
    vertexRDO->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, sizeof(VegetationVertex), &(vertexData[0].texCoord0));
    vertexRDO->BuildVertexBuffer(static_cast<int32>(vertexData.size()), BDT_STATIC_DRAW, true);
    
    Vector<Vector<Vector<VegetationSortedBufferItem> > >& indexBuffers = renderData->GetIndexBuffers();
    
    for(size_t resolutionIndex = 0; resolutionIndex < resolutionCount; ++resolutionIndex)
    {
        Vector<Vector<SortBufferData> >& sortedIndexBuffers = resolutionDataArray[resolutionIndex];
        
        indexBuffers.push_back(Vector<Vector<VegetationSortedBufferItem> >());
        Vector<Vector<VegetationSortedBufferItem> >& currentResolutionIndexBuffers = indexBuffers[indexBuffers.size() - 1];
        
        size_t cellCount = sortedIndexBuffers.size();
        for(size_t cellIndex = 0; cellIndex < cellCount; ++cellIndex)
        {
            currentResolutionIndexBuffers.push_back(Vector<VegetationSortedBufferItem>());
            Vector<VegetationSortedBufferItem>& sortedIndexBufferItems = currentResolutionIndexBuffers[currentResolutionIndexBuffers.size() - 1];
            
            Vector<SortBufferData>& directionIndexBuffers = sortedIndexBuffers[cellIndex];
            
            size_t directionCount = directionIndexBuffers.size();
            for(size_t directionIndex = 0; directionIndex < directionCount; ++directionIndex)
            {
                SortBufferData& sortData = directionIndexBuffers[directionIndex];
                
                sortedIndexBufferItems.push_back(VegetationSortedBufferItem());
                VegetationSortedBufferItem& sortBufferItem = sortedIndexBufferItems[directionIndex];
                
                RenderDataObject* indexBuffer = new RenderDataObject();
                indexBuffer->SetIndices(VEGETATION_INDEX_TYPE, (uint8*)(&indexData[sortData.indexOffset]), sortData.size);
                
                sortBufferItem.SetRenderDataObject(indexBuffer);
                sortBufferItem.SetRenderDataObjectAttachment(vertexRDO);
                sortBufferItem.sortDirection = sortData.sortDirection;
                
                sortBufferItem.rdo->BuildIndexBuffer(BDT_STATIC_DRAW, true);
                sortBufferItem.rdo->AttachVertices(vertexRDO);
                
                SafeRelease(indexBuffer);
            }
        }
    }
    
    SafeRelease(vertexRDO);
    
    NMaterial* material = customGeometryData[0].material;
    renderData->SetMaterial(material);
    
    material->SetFlag(VegetationPropertyNames::FLAG_GRASS_OPAQUE, NMaterial::FlagOn);
    material->SetFlag(VegetationPropertyNames::FLAG_GRASS_TRANSFORM_WAVE, NMaterial::FlagOn);
    material->SetRenderLayers(1 << RENDER_LAYER_VEGETATION_ID);
    
    FastNameSet::iterator end = materialFlags.end();
    for(FastNameSet::iterator it = materialFlags.begin(); it != end; ++it)
    {
        material->SetFlag(it->first, NMaterial::FlagOn);
    }
    
    material->SetPropertyValue(VegetationPropertyNames::UNIFORM_WORLD_SIZE,
                                                 Shader::UT_FLOAT_VEC3,
                                                 1,
                                                 &worldSize);
    
    //fill in metrics data
    size_t layerCount = customGeometryData.size();
    for(size_t layerIndex = 0; layerIndex < layerCount; ++layerIndex)
    {
        renderData->instanceCount.push_back(Vector<uint32>());
        renderData->vertexCountPerInstance.push_back(Vector<uint32>());
        renderData->polyCountPerInstance.push_back(Vector<uint32>());
        
        Vector<uint32>& layerInstanceCount = renderData->instanceCount[renderData->instanceCount.size() - 1];
        Vector<uint32>& layerVertexCount = renderData->vertexCountPerInstance[renderData->vertexCountPerInstance.size() - 1];
        Vector<uint32>& polyVertexCount = renderData->polyCountPerInstance[renderData->polyCountPerInstance.size() - 1];
    
        CustomGeometryEntityData& layerGeometryInfo = customGeometryData[layerIndex];
        size_t lodCount = layerGeometryInfo.lods.size();
        for(size_t lodIndex = 0; lodIndex < lodCount; ++lodIndex)
        {
            layerInstanceCount.push_back(maxClusters[layerIndex].maxClusterCount * maxClusters[layerIndex].maxClusterCount);
            layerVertexCount.push_back(static_cast<uint32>(layerGeometryInfo.lods[lodIndex].sourcePositions.size()));
            polyVertexCount.push_back(static_cast<uint32>(layerGeometryInfo.lods[lodIndex].sourceIndices.size() / 3));
        }
    }
}

void VegetationCustomSLGeometry::OnVegetationPropertiesChanged(Vector<VegetationRenderData*>& renderDataArray, KeyedArchive* props)
{
    NMaterial* mat = renderDataArray[0]->GetMaterial();
    
    if(mat)
    {
        String lightmapKeyName = VegetationPropertyNames::UNIFORM_SAMPLER_VEGETATIONMAP.c_str();
        if(props->IsKeyExists(lightmapKeyName))
        {
            FilePath lightmapPath = props->GetString(lightmapKeyName);
            mat->SetTexture(VegetationPropertyNames::UNIFORM_SAMPLER_VEGETATIONMAP, lightmapPath);
        }
        
        String heightmapKeyName = NMaterial::TEXTURE_HEIGHTMAP.c_str();
        if(props->IsKeyExists(heightmapKeyName))
        {
            Texture* heightmap = (Texture*)props->GetUInt64(heightmapKeyName);
            mat->SetTexture(NMaterial::TEXTURE_HEIGHTMAP, heightmap);
        }
        
        String heightmapScaleKeyName = VegetationPropertyNames::UNIFORM_HEIGHTMAP_SCALE.c_str();
        if(props->IsKeyExists(heightmapScaleKeyName))
        {
            Vector2 heightmapScale = props->GetVector2(heightmapScaleKeyName);
            mat->SetPropertyValue(VegetationPropertyNames::UNIFORM_HEIGHTMAP_SCALE,
                                  Shader::UT_FLOAT_VEC2,
                                  1,
                                  heightmapScale.data);
        }
        
        String densityMapPathKey = VegetationPropertyNames::UNIFORM_SAMPLER_DENSITYMAP.c_str();
        if(props->IsKeyExists(densityMapPathKey))
        {
            FilePath densityMapPath = props->GetString(densityMapPathKey);
            densityMapPath.ReplaceExtension(".tex");
            mat->SetTexture(VegetationPropertyNames::UNIFORM_SAMPLER_DENSITYMAP, densityMapPath);
        }
    }
}

void VegetationCustomSLGeometry::GenerateClusterPositionData(const Vector<VegetationLayerParams>& layerClusterCount,
                                 Vector<ClusterPositionData>& clusters,
                                 Vector<VertexRangeData>& layerRanges)
{
    clusters.clear();
    
    size_t layerCount = layerClusterCount.size();
    uint32 clustersSize = 0;
    for(size_t layerIndex = 0; layerIndex < layerCount; ++layerIndex)
    {
        uint32 clusterRowSize = resolutionTilesPerRow[0] * layerClusterCount[layerIndex].maxClusterCount;
        clustersSize += clusterRowSize * clusterRowSize;
    }
    
    clusters.reserve(clustersSize);
    layerRanges.resize(layerCount);

    uint32 currentIndex = 0;
    for(size_t layerIndex = 0; layerIndex < layerCount; ++layerIndex)
    {
        const VegetationLayerParams& layerParamsData = layerClusterCount[layerIndex];
        
        uint32 layerMaxClusters = layerParamsData.maxClusterCount;
        
        uint32 clusterRowSize = resolutionTilesPerRow[0] * layerMaxClusters;
        uint32 totalClusterCount = clusterRowSize * clusterRowSize;
        Vector2 vegetationInstanceSize(unitSize.x / layerMaxClusters, unitSize.y / layerMaxClusters);
        
        Vector<uint32> densityId;
        densityId.resize(totalClusterCount);
        for(uint32 i = 0; i < totalClusterCount; ++i)
        {
            densityId[i] = (i % maxDensityLevels);
        }
        std::random_shuffle(densityId.begin(), densityId.end(), RandomShuffleFunc);
        
        layerRanges[layerIndex].index = currentIndex;
        layerRanges[layerIndex].size = totalClusterCount;
        layerRanges[layerIndex].rowSize = clusterRowSize;
        
        for(uint32 clusterIndex = 0; clusterIndex < totalClusterCount; ++clusterIndex)
        {
            uint32 cellX = clusterIndex % clusterRowSize;
            uint32 cellY = clusterIndex / clusterRowSize;
            
            uint32 matrixCellX = cellX / layerMaxClusters;
            uint32 matrixCellY = cellY / layerMaxClusters;
            
            float32 randomX = unitSize.x * (float32)Random::Instance()->RandFloat();
            float32 randomY = unitSize.y * (float32)Random::Instance()->RandFloat();
            
            
            
            
            clusters.push_back(ClusterPositionData());
            ClusterPositionData& cluster = clusters[clusters.size() - 1];
            
            cluster.pos = Vector3((matrixCellX * unitSize.x) + randomX,
                                  (matrixCellY * unitSize.y) + randomY,
                                  0.0f);
            cluster.rotation = layerParamsData.instanceRotationVariation * ((float32)Random::Instance()->RandFloat() - 0.5f);
            cluster.scale = 1.0f - layerParamsData.instanceScaleVariation * (float32)Random::Instance()->RandFloat();
            cluster.densityId = densityId[clusterIndex];
            cluster.layerId = static_cast<uint32>(layerIndex);
            
            currentIndex++;
        }
    }
}

void VegetationCustomSLGeometry::GenerateClusterResolutionData(uint32 resolutionId,
                                                               const Vector<VegetationLayerParams>& layerClusterCount,
                                                               const Vector<ClusterPositionData>& clusterPosition,
                                                               const Vector<VertexRangeData>& layerRanges,
                                                               Vector<ClusterResolutionData>& clusterResolution)
{
    uint32 layerCount = static_cast<uint32>(layerRanges.size());
    uint32 totalClusterCountInResolution = 0;
    for(size_t layerIndex = 0; layerIndex < layerCount; ++layerIndex)
    {
        const VertexRangeData& layerRangeData = layerRanges[layerIndex];
        uint32 resolutionStride = resolutionClusterStride[resolutionId];
        
        totalClusterCountInResolution += layerRangeData.size / (resolutionStride * resolutionStride);
    }
    
    clusterResolution.clear();
    clusterResolution.reserve(totalClusterCountInResolution);
    
    uint32 currentTilesPerRowCount = resolutionTilesPerRow[resolutionId];
    
    for(size_t layerIndex = 0; layerIndex < layerCount; ++layerIndex)
    {
        const VertexRangeData& layerRangeData = layerRanges[layerIndex];
        
        uint32 clusterRowCount = layerRangeData.size / layerRangeData.rowSize;
        uint32 resolutionRowSize = (resolutionTilesPerRow[0] / currentTilesPerRowCount) * layerClusterCount[layerIndex].maxClusterCount;
        
        DVASSERT(layerRangeData.rowSize == clusterRowCount);
        
        uint32 resolutionStride = resolutionClusterStride[resolutionId];;
        for(uint32 clusterY = 0; clusterY < clusterRowCount; clusterY += resolutionStride)
        {
            for(uint32 clusterX = 0; clusterX < layerRangeData.rowSize; clusterX += resolutionStride)
            {
                clusterResolution.push_back(ClusterResolutionData());
                ClusterResolutionData& resolutionData = clusterResolution[clusterResolution.size() - 1];
                
                size_t clusterIndex = clusterX + clusterY * layerRangeData.rowSize;
                
                uint32 absoluteCellX = clusterIndex % layerRangeData.rowSize;
                uint32 absoluteCellY = static_cast<uint32>(clusterIndex / layerRangeData.rowSize);
                
                uint32 resolutionCellX = absoluteCellX / resolutionRowSize;
                uint32 resolutionCellY = absoluteCellY / resolutionRowSize;
                
                resolutionData.position = clusterPosition[layerRangeData.index + clusterIndex];
                resolutionData.resolutionId = PrepareResolutionId(resolutionId, clusterX, clusterY);
                resolutionData.effectiveResolutionId = resolutionId;
                resolutionData.cellIndex = resolutionCellX + resolutionCellY * currentTilesPerRowCount;
            }
        }
    }
}

uint32 VegetationCustomSLGeometry::PrepareResolutionId(uint32 currentResolutionId, uint32 cellX, uint32 cellY) const
{
    uint32 nextResolutionId = Min(resolutionCount - 1, currentResolutionId + 1);
    uint32 rowStride = resolutionClusterStride[nextResolutionId];
    
    bool isNextResolution = (((cellX % rowStride) == 0) && ((cellY % rowStride) == 0));
    return (isNextResolution) ? nextResolutionId : currentResolutionId;
}

void VegetationCustomSLGeometry::GenerateVertexData(const Vector<CustomGeometryEntityData>& sourceGeomData,
                                                    const Vector<ClusterResolutionData>& clusterResolution,
                                                    
                                                    Vector<VegetationVertex>& vertexData,
                                                    Vector<BufferCellData>& cellOffsets)
{
    cellOffsets.clear();
    
    uint32 currentMatrix = 0;
    uint32 currentCluster = 0;
    uint32 currentVertexIndex = static_cast<uint32>(vertexData.size());

    Vector<Vector3> transformedVertices;
    Vector<Vector3> transformedNormals;
    
    Vector<Vector3> scaledVertices;
    Vector<Vector3> scaledNormals;
    
    size_t clusterCount = clusterResolution.size();
    for(size_t clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
    {
        const ClusterResolutionData& clusterData = clusterResolution[clusterIndex];
        
        if(currentMatrix != clusterData.cellIndex)
        {
            BufferCellData rangeData;
            rangeData.vertexStartIndex = currentVertexIndex;
            rangeData.vertexCount = static_cast<uint32>(vertexData.size() - currentVertexIndex);
            rangeData.clusterStartIndex = currentCluster;
            rangeData.clusterCount = static_cast<uint32>(clusterIndex - currentCluster);
            
            cellOffsets.push_back(rangeData);
            
            currentCluster = static_cast<uint32>(clusterIndex);
            currentMatrix = clusterData.cellIndex;
            currentVertexIndex = static_cast<uint32>(vertexData.size());
        }
        
        const CustomGeometryLayerData& clusterGeometry = sourceGeomData[clusterData.position.layerId].lods[clusterData.effectiveResolutionId];
        
        Scale(clusterGeometry.pivot,
              clusterData.position.scale,
              clusterGeometry.sourcePositions,
              clusterGeometry.sourceNormals,
              scaledVertices,
              scaledNormals);
        
        
        Rotate(clusterData.position.rotation,
               scaledVertices,
               scaledNormals,
               transformedVertices,
               transformedNormals);

        size_t vertexCount = clusterGeometry.sourcePositions.size();
        for(size_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
        {
            VegetationVertex vertex;
            
            vertex.coord.x = clusterData.position.pos.x + transformedVertices[vertexIndex].x;
            vertex.coord.y = clusterData.position.pos.y + transformedVertices[vertexIndex].y;
            vertex.coord.z = clusterData.position.pos.z + transformedVertices[vertexIndex].z;
            
            vertex.normal = transformedNormals[vertexIndex];
            
            vertex.binormal.x = clusterData.position.pos.x + clusterGeometry.pivot.x;
            vertex.binormal.y = clusterData.position.pos.y + clusterGeometry.pivot.y;
            vertex.binormal.z = clusterData.position.pos.z + clusterGeometry.pivot.z;
            
            vertex.tangent.x = (float32)clusterData.resolutionId;
            vertex.tangent.y = (float32)clusterData.position.layerId;
            vertex.tangent.z = Max(0.0f, ((vertex.coord.z - clusterGeometry.pivot.z) / (clusterGeometry.bbox.max.z - clusterGeometry.pivot.z)));
            
            vertex.texCoord0 = clusterGeometry.sourceTextureCoords[vertexIndex];
            
            vertexData.push_back(vertex);
        }
    }
    
    BufferCellData rangeData;
    rangeData.vertexStartIndex = currentVertexIndex;
    rangeData.vertexCount = static_cast<uint32>(vertexData.size() - currentVertexIndex);
    rangeData.clusterStartIndex = currentCluster;
    rangeData.clusterCount = static_cast<uint32>(clusterCount - currentCluster);
    
    cellOffsets.push_back(rangeData);
}

void VegetationCustomSLGeometry::Rotate(float32 angle,
                                        const Vector<Vector3>& sourcePositions,
                                        const Vector<Vector3>& sourceNormals,
                                        Vector<Vector3>& rotatedPositions,
                                        Vector<Vector3>& rotatedNormals)
{
    rotatedPositions.clear();
    rotatedNormals.clear();
    
    Matrix4 rotMat;
    rotMat.CreateRotation(Vector3(0.0f, 0.0f, 1.0f), DegToRad(angle));
    
    Matrix4 normalMatrix4;
    rotMat.GetInverse(normalMatrix4);
    normalMatrix4.Transpose();
    Matrix3 normalMatrix3;
    normalMatrix3 = normalMatrix4;

    size_t sourceVertexCount = sourcePositions.size();
    for(size_t vertexIndex = 0; vertexIndex < sourceVertexCount; ++vertexIndex)
    {
        Vector3 transformedVertex = sourcePositions[vertexIndex] * rotMat;
        rotatedPositions.push_back(transformedVertex);
        
        Vector3 transformedNormal = sourceNormals[vertexIndex] * normalMatrix3;
        transformedNormal.Normalize();
        
        rotatedNormals.push_back(transformedNormal);
    }
}

void VegetationCustomSLGeometry::Scale(const Vector3& clusterPivot,
                                       float32 scale,
                                       const Vector<Vector3>& sourcePositions,
                                       const Vector<Vector3>& sourceNormals,
                                       Vector<Vector3>& scaledPositions,
                                       Vector<Vector3>& scaledNormals)
{
    scaledPositions.clear();
    scaledNormals.clear();
    
    size_t sourceVertexCount = sourcePositions.size();
    for(size_t vertexIndex = 0; vertexIndex < sourceVertexCount; ++vertexIndex)
    {
        Vector3 transformedVertex;
        transformedVertex.Lerp(clusterPivot, sourcePositions[vertexIndex], scale);
        
        scaledPositions.push_back(transformedVertex);
        scaledNormals.push_back(sourceNormals[vertexIndex]);
    }
    
}

void VegetationCustomSLGeometry::GenerateIndexData(const Vector<CustomGeometryEntityData>& sourceGeomData,
                                                   const Vector<ClusterResolutionData>& clusterResolution,
                                                   const BufferCellData& rangeData,
                                                 
                                                   Vector<VegetationVertex>& vertexData,
                                                   Vector<VegetationIndex>& indexData,
                                                   Vector<SortBufferData>& directionOffsets)
{
    uint32 lastClusterIndex = rangeData.clusterStartIndex + rangeData.clusterCount;
    size_t vertexIndexOffset = 0;
    Vector<VegetationIndex> sourceCellIndices;
    for(uint32 clusterIndex = rangeData.clusterStartIndex; clusterIndex < lastClusterIndex; ++clusterIndex)
    {
        const ClusterResolutionData& resolutionData = clusterResolution[clusterIndex];
    
        const CustomGeometryLayerData& layerGeometry = sourceGeomData[resolutionData.position.layerId].lods[resolutionData.effectiveResolutionId];
        size_t clusterIndexCount = layerGeometry.sourceIndices.size();
        
        for(size_t i = 0; i < clusterIndexCount; ++i)
        {
            sourceCellIndices.push_back(rangeData.vertexStartIndex + static_cast<int32>(vertexIndexOffset) + layerGeometry.sourceIndices[i]);
        }
        
        vertexIndexOffset += layerGeometry.sourcePositions.size();
    }
    
    AABBox3 boundingBox;
    Vector<PolygonSortData> sortDataArray;
    size_t sortItemCount = sourceCellIndices.size() / 3;
    for(size_t sortItemIndex = 0; sortItemIndex < sortItemCount; ++sortItemIndex)
    {
        sortDataArray.push_back(PolygonSortData());
        PolygonSortData& sortData = sortDataArray[sortDataArray.size() - 1];
        
        sortData.indices[0] = sourceCellIndices[sortItemIndex * 3 + 0];
        
        sortData.indices[1] = sourceCellIndices[sortItemIndex * 3 + 1];
        
        sortData.indices[2] = sourceCellIndices[sortItemIndex * 3 + 2];
        
        boundingBox.AddPoint(vertexData[sortData.indices[0]].coord);
        boundingBox.AddPoint(vertexData[sortData.indices[1]].coord);
        boundingBox.AddPoint(vertexData[sortData.indices[2]].coord);
    }
    
    Vector<Vector3> cameraPositions;
    SetupCameraPositions(boundingBox, cameraPositions);
    size_t cameraPositionCount = cameraPositions.size();
    
    for(uint32 cameraPositionIndex = 0; cameraPositionIndex < cameraPositionCount; ++cameraPositionIndex)
    {
        GenerateSortedClusterIndexData(cameraPositions[cameraPositionIndex],
                                       sortDataArray,
                                       vertexData);
        
        SortBufferData bufferData;
        bufferData.sortDirection = boundingBox.GetCenter() - cameraPositions[cameraPositionIndex];
        bufferData.indexOffset = static_cast<uint32>(indexData.size());
        bufferData.size = static_cast<uint32>(sortItemCount * 3);
        
        directionOffsets.push_back(bufferData);
        
        for(size_t sortItemIndex = 0; sortItemIndex < sortItemCount; ++sortItemIndex)
        {
            PolygonSortData& sortData = sortDataArray[sortItemIndex];
            
            indexData.push_back(sortData.indices[0]);
            indexData.push_back(sortData.indices[1]);
            indexData.push_back(sortData.indices[2]);
        }
    }
}

void VegetationCustomSLGeometry::GenerateSortedClusterIndexData(Vector3& cameraPosition,
                                                              Vector<PolygonSortData>& sourceIndices,
                                                              Vector<VegetationVertex>& vertexData)
{
    size_t sortedIndicesCount = sourceIndices.size();
    for(size_t sortItemIndex = 0;
        sortItemIndex < sortedIndicesCount;
        ++sortItemIndex)
    {
        PolygonSortData& sortData = sourceIndices[sortItemIndex];
        sortData.cameraDistance = FLT_MAX;
        
        for(uint32 polygonIndex = 0; polygonIndex < 3; ++polygonIndex)
        {
            float32 distance = (vertexData[sortData.indices[polygonIndex]].coord - cameraPosition).SquareLength();
            if(distance < sortData.cameraDistance)
            {
                sortData.cameraDistance = distance;
            }
        }
    }
    
    std::stable_sort(sourceIndices.begin(), sourceIndices.end(), PolygonByDistanceCompareFunction);
}

void VegetationCustomSLGeometry::InitCustomGeometry(const VegetationCustomGeometrySerializationDataPtr& geometryData)
{
    uint32 layerCount = geometryData->GetLayerCount();
    for(uint32 layerIndex = 0; layerIndex < layerCount; ++layerIndex)
    {
        customGeometryData.push_back(CustomGeometryEntityData());
        CustomGeometryEntityData& customGeometryDataItem = customGeometryData[customGeometryData.size() - 1];
        
        customGeometryDataItem.SetMaterial(geometryData->GetMaterial(layerIndex));
        
        uint32 lodCount = geometryData->GetLodCount(layerIndex);
        for(uint32 lodIndex = 0; lodIndex < lodCount; ++lodIndex)
        {
            customGeometryDataItem.lods.push_back(CustomGeometryLayerData());
            CustomGeometryLayerData& geometryLayerData = customGeometryDataItem.lods[customGeometryDataItem.lods.size() - 1];
            
            Vector<Vector3>& positions = geometryLayerData.sourcePositions;
            Vector<Vector2>& texCoords = geometryLayerData.sourceTextureCoords;
            Vector<Vector3>& normals = geometryLayerData.sourceNormals;
            Vector<VegetationIndex>& indices = geometryLayerData.sourceIndices;
            
            Vector<Vector3>& srcPositions = geometryData->GetPositions(layerIndex, lodIndex);
            Vector<Vector2>& srcTexCoords = geometryData->GetTextureCoords(layerIndex, lodIndex);
            Vector<Vector3>& srcNormals = geometryData->GetNormals(layerIndex, lodIndex);;
            Vector<VegetationIndex>& srcIndices = geometryData->GetIndices(layerIndex, lodIndex);;
            
            size_t posCount = srcPositions.size();
            for(size_t posIndex = 0; posIndex < posCount; ++posIndex)
            {
                positions.push_back(srcPositions[posIndex]);
            }
            
            size_t texCoordCount = srcTexCoords.size();
            for(size_t texCoordIndex = 0; texCoordIndex < texCoordCount; ++texCoordIndex)
            {
                texCoords.push_back(srcTexCoords[texCoordIndex]);
            }
            
            size_t normalsCount = srcNormals.size();
            for(size_t normalIndex = 0; normalIndex < normalsCount; ++normalIndex)
            {
                normals.push_back(srcNormals[normalIndex]);
            }
            
            size_t indexCount = srcIndices.size();
            for(size_t indexIndex = 0; indexIndex < indexCount; ++indexIndex)
            {
                indices.push_back(srcIndices[indexIndex]);
            }
        }
        
        if(lodCount < resolutionCount)
        {
            uint32 stubGeometryCount = resolutionCount - lodCount;
            for(uint32 stubIndex = 0; stubIndex < stubGeometryCount; ++stubIndex)
            {
                customGeometryDataItem.lods.push_back(customGeometryDataItem.lods[customGeometryDataItem.lods.size() - 1]);
            }
        }
    }
}

void VegetationCustomSLGeometry::PrepareBoundingBoxes()
{
    size_t customGeometryDataCount = customGeometryData.size();
    for(size_t geometryIndex = 0; geometryIndex < customGeometryDataCount; ++geometryIndex)
    {
        CustomGeometryEntityData& geometryEntityData = customGeometryData[geometryIndex];
        
        AABBox3 bbox;
        Vector3 pivot(0.0f, 0.0f, FLT_MAX);
        
        size_t lodCount = geometryEntityData.lods.size();
        for(size_t lodIndex = 0; lodIndex < lodCount; ++lodIndex)
        {
            CustomGeometryLayerData& lodData = geometryEntityData.lods[lodIndex];
            
            size_t vertexCount = lodData.sourcePositions.size();
            for(size_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
            {
                Vector3& pt = lodData.sourcePositions[vertexIndex];
                bbox.AddPoint(pt);
                
                if(pt.z < pivot.z)
                {
                    pivot = pt;
                }
            }
        }
        
        for(size_t lodIndex = 0; lodIndex < lodCount; ++lodIndex)
        {
            CustomGeometryLayerData& lodData = geometryEntityData.lods[lodIndex];
            
            lodData.bbox = bbox;
            lodData.pivot = pivot;
        }
    }
}

};
