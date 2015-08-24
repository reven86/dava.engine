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
#include "Render/Highlevel/Vegetation/VegetationCustomGeometry.h"
#include "Utils/Random.h"
#include "FileSystem/KeyedArchive.h"
#include "Render/Highlevel/Vegetation/VegetationPropertyNames.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"

namespace DAVA
{

static const float32 MAX_ROTATION_ANGLE = 180.0f;

void VegetationCustomGeometry::CustomMaterialTransformer::TransformMaterialOnCreate(NMaterial* mat)
{
    mat->SetRuntime(true);
    mat->SetRenderLayers(1 << RENDER_LAYER_VEGETATION_ID);
}

VegetationCustomGeometry::CustomGeometryEntityData::CustomGeometryEntityData() : material(NULL)
{
}

VegetationCustomGeometry::CustomGeometryEntityData::CustomGeometryEntityData(const CustomGeometryEntityData& src)
{
    material = NULL;
    SetMaterial(src.material);
    
    lods = src.lods;
}
    
VegetationCustomGeometry::CustomGeometryEntityData::~CustomGeometryEntityData()
{
    SafeRelease(material);
}
    
void VegetationCustomGeometry::CustomGeometryEntityData::SetMaterial(NMaterial* mat)
{
    if(mat != material)
    {
        SafeRelease(material);
        material = SafeRetain(mat);
    }
}

int32 VegetationCustomGeometry::RandomShuffleFunc(int32 limit)
{
    return (Random::Instance()->Rand() % limit);
}

bool VegetationCustomGeometry::PolygonByDistanceCompareFunction(const PolygonSortData& a,
                                                                const PolygonSortData& b)
{
    return a.cameraDistance > b.cameraDistance; //back to front order
}

bool VegetationCustomGeometry::ClusterByMatrixCompareFunction(const ClusterResolutionData& a,
                                                              const ClusterResolutionData& b)
{
    return a.cellIndex < b.cellIndex;
}

VegetationCustomGeometry::VegetationCustomGeometry(const Vector<uint32> & _maxClusters,
                                                   uint32 _maxDensityLevels,
                                                   const Vector2 & _unitSize,
                                                   const FilePath & _dataPath,
                                                   const Vector<uint32> & _resolutionTilesPerRow,
                                                   const Vector<uint32> & _resolutionClusterStride,
                                                   const Vector3 & _worldSize,
                                                   VegetationCustomGeometrySerializationData* geometryData)
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
    
VegetationCustomGeometry::~VegetationCustomGeometry()
{
    markedRenderData.clear();
}

void VegetationCustomGeometry::ReleaseRenderData(Vector<VegetationRenderData*>& renderDataArray)
{
    markedRenderData.clear(); //marked render data only stores references
        
    VegetationGeometry::ReleaseRenderData(renderDataArray);
}

    
void VegetationCustomGeometry::Build(Vector<VegetationRenderData*>& renderDataArray, const FastNameSet& materialFlags)
{
    markedRenderData.resize(customGeometryData.size());
    renderDataArray.resize(customGeometryData.size());
    
    size_t layerCount = customGeometryData.size();
    for(size_t layerIndex = 0; layerIndex < layerCount; ++layerIndex)
    {
        CustomGeometryEntityData& layerGeometryData = customGeometryData[layerIndex];
        
        VegetationRenderData* renderData = new VegetationRenderData();
        renderData->SetMaterial(layerGeometryData.material);
        
        layerGeometryData.material->SetFlag(VegetationPropertyNames::FLAG_GRASS_OPAQUE, NMaterial::FlagOn);
        layerGeometryData.material->SetRenderLayers(1 << RENDER_LAYER_VEGETATION_ID);
        
        FastNameSet::iterator end = materialFlags.end();
        for(FastNameSet::iterator it = materialFlags.begin(); it != end; ++it)
        {
            layerGeometryData.material->SetFlag(it->first, NMaterial::FlagOn);
        }

        layerGeometryData.material->SetPropertyValue(VegetationPropertyNames::UNIFORM_WORLD_SIZE,
                                             Shader::UT_FLOAT_VEC3,
                                             1,
                                             &worldSize);
        
        renderDataArray[layerIndex] = renderData;
        markedRenderData[layerIndex].renderData = renderData;
        
        Vector<VegetationVertex>& vertexData = renderData->GetVertices();
        Vector<VegetationIndex>& indexData = renderData->GetIndices();
        
        BuildLayer(static_cast<uint32>(layerIndex),
                   layerGeometryData,
                   vertexData,
                   indexData,
                   markedRenderData[layerIndex].vertexOffset,
                   markedRenderData[layerIndex].indexOffset);
    }
    
    for(size_t layerIndex = 0; layerIndex < layerCount; ++layerIndex)
    {
        VegetationRenderData* renderData = renderDataArray[layerIndex];
        Vector<VegetationVertex>& vertexData = renderData->GetVertices();
        Vector<VegetationIndex>& indexData = renderData->GetIndices();
        
        RenderDataObject* vertexRDO = new RenderDataObject();
        vertexRDO->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, sizeof(VegetationVertex), &(vertexData[0].coord));
        vertexRDO->SetStream(EVF_NORMAL, TYPE_FLOAT, 3, sizeof(VegetationVertex), &(vertexData[0].normal));
        vertexRDO->SetStream(EVF_BINORMAL, TYPE_FLOAT, 3, sizeof(VegetationVertex), &(vertexData[0].binormal));
        vertexRDO->SetStream(EVF_TANGENT, TYPE_FLOAT, 3, sizeof(VegetationVertex), &(vertexData[0].tangent));
        vertexRDO->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, sizeof(VegetationVertex), &(vertexData[0].texCoord0));
        vertexRDO->BuildVertexBuffer(static_cast<uint32>(vertexData.size()), BDT_STATIC_DRAW, true);
        
        Vector<Vector<Vector<VegetationSortedBufferItem> > >& indexBuffers = renderData->GetIndexBuffers();
        
        for(size_t resolutionIndex = 0; resolutionIndex < resolutionCount; ++resolutionIndex)
        {
            Vector<Vector<SortBufferData> >& sortedIndexBuffers = markedRenderData[layerIndex].indexOffset[resolutionIndex];
            
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
    }
}
    
void VegetationCustomGeometry::OnVegetationPropertiesChanged(Vector<VegetationRenderData*>& renderDataArray, KeyedArchive* props)
{
    size_t markedArraySize = markedRenderData.size();
    for(size_t renderDataIndex = 0; renderDataIndex < markedArraySize; ++renderDataIndex)
    {
        NMaterial* mat = markedRenderData[renderDataIndex].renderData->GetMaterial();
        
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
}

void VegetationCustomGeometry::GenerateClusterPositionData(uint32 layerMaxClusters, Vector<ClusterPositionData>& clusters)
{
    clusters.clear();
    
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
    
    clusters.resize(totalClusterCount);
    for(uint32 clusterIndex = 0; clusterIndex < totalClusterCount; ++clusterIndex)
    {
        ClusterPositionData& cluster = clusters[clusterIndex];
        
        uint32 cellX = clusterIndex % clusterRowSize;
        uint32 cellY = clusterIndex / clusterRowSize;
        
        uint32 matrixCellX = cellX / layerMaxClusters;
        uint32 matrixCellY = cellY / layerMaxClusters;
        
        float32 randomX = unitSize.x * (float32)Random::Instance()->RandFloat();
        float32 randomY = unitSize.y * (float32)Random::Instance()->RandFloat();
        
        cluster.pos = Vector3((matrixCellX * unitSize.x) + randomX,
                              (matrixCellY * unitSize.y) + randomY,
                              0.0f);
        cluster.rotation = MAX_ROTATION_ANGLE * ((float32)Random::Instance()->RandFloat() - 0.5f);
        cluster.densityId = densityId[clusterIndex];
        cluster.matrixIndex = matrixCellX + matrixCellY * resolutionTilesPerRow[0];
        
        DVASSERT(cluster.matrixIndex < (resolutionTilesPerRow[0] * resolutionTilesPerRow[0]));
    }
}

void VegetationCustomGeometry::GenerateClusterResolutionData(uint32 layerId,
                                                             uint32 layerMaxClusters,
                                                             uint32 resolutionId,
                                                             Vector<ClusterPositionData>& clusterPosition,
                                                             Vector<ClusterResolutionData>& clusterResolution)
{
    clusterResolution.clear();
    
    uint32 clusterRowSize = resolutionTilesPerRow[0] * layerMaxClusters;
    uint32 currentTilesPerRowCount = resolutionTilesPerRow[resolutionId];
    uint32 resolutionRowSize = (resolutionTilesPerRow[0] / currentTilesPerRowCount) * layerMaxClusters;
    
    size_t totalClusterCount = clusterPosition.size();
    size_t currentResolutionStride = resolutionClusterStride[resolutionId];
    size_t clusterCountInResolution = totalClusterCount / (currentResolutionStride * currentResolutionStride);
    
    clusterResolution.resize(clusterCountInResolution);
    size_t clusterResolutionIndex = 0;
    for(size_t clusterY = 0; clusterY < clusterRowSize; clusterY += currentResolutionStride)
    {
        for(size_t clusterX = 0; clusterX < clusterRowSize; clusterX += currentResolutionStride)
        {
            size_t clusterIndex = clusterX + clusterY * clusterRowSize;
            
            ClusterResolutionData& resolutionData = clusterResolution[clusterResolutionIndex];
            
            uint32 absoluteCellX = clusterIndex % clusterRowSize;
            uint32 absoluteCellY = static_cast<uint32>(clusterIndex / clusterRowSize);
            
            uint32 resolutionCellX = absoluteCellX / resolutionRowSize;
            uint32 resolutionCellY = absoluteCellY / resolutionRowSize;
            
            resolutionData.position = clusterPosition[clusterIndex];
            resolutionData.resolutionId = static_cast<uint32>(PrepareResolutionId(resolutionId, static_cast<uint32>(clusterX), static_cast<uint32>(clusterY)));
            resolutionData.layerId = layerId;
            resolutionData.cellIndex = resolutionCellX + resolutionCellY * currentTilesPerRowCount;
            
            clusterResolutionIndex++;
            
            DVASSERT(resolutionData.cellIndex < (currentTilesPerRowCount * currentTilesPerRowCount));
        }
    }
    
    DVASSERT(clusterResolution.size() == clusterResolutionIndex);
}

uint32 VegetationCustomGeometry::PrepareResolutionId(uint32 currentResolutionId, uint32 cellX, uint32 cellY) const
{
    uint32 nextResolutionId = Min(resolutionCount - 1, currentResolutionId + 1);
    uint32 rowStride = resolutionClusterStride[nextResolutionId];
    
    bool isNextResolution = (((cellX % rowStride) == 0) && ((cellY % rowStride) == 0));
    return (isNextResolution) ? nextResolutionId : currentResolutionId;
}

void VegetationCustomGeometry::GenerateVertexData(Vector<Vector3>& sourcePositions,
                                                  Vector<Vector2>& sourceTextureCoords,
                                                  Vector<Vector3>& sourceNormals,
                                                  Vector<ClusterResolutionData>& clusterResolution,
                                                  
                                                  uint32& startIndex,
                                                  uint32& endIndex,
                                                  Vector<VegetationVertex>& vertexData,
                                                  Vector<VertexRangeData>& perCellOffsets,
                                                  Vector<uint32>& perCellClusterCount)
{
    DVASSERT(sourcePositions.size() == sourceNormals.size() &&
             sourcePositions.size() == sourceTextureCoords.size());
    
    startIndex = static_cast<uint32>(vertexData.size());
    
    perCellOffsets.clear();
    uint32 currentMatrix = 0;
    uint32 currentCluster = 0;
    
    uint32 currentVertexIndex = startIndex;
    
    size_t vertexCount = sourcePositions.size();
    size_t clusterCount = clusterResolution.size();
    
    Vector<Vector3> transformedVertices;
    for(size_t clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
    {
        ClusterResolutionData& clusterData = clusterResolution[clusterIndex];
        
        Rotate(clusterData.position.rotation, sourcePositions, transformedVertices);
        
        for(size_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
        {
            VegetationVertex vertex;
         
            vertex.coord.x = clusterData.position.pos.x + transformedVertices[vertexIndex].x;
            vertex.coord.y = clusterData.position.pos.y + transformedVertices[vertexIndex].y;
            vertex.coord.z = clusterData.position.pos.z + transformedVertices[vertexIndex].z;
            
            vertex.normal = sourceNormals[vertexIndex];
            
            vertex.binormal = clusterData.position.pos;
            
            vertex.tangent.x = (float32)clusterData.resolutionId;
            vertex.tangent.y = (float32)clusterData.layerId;
            vertex.tangent.z = (float32)clusterData.position.densityId;
            
            vertex.texCoord0 = sourceTextureCoords[vertexIndex];
            
            if(currentMatrix != clusterData.cellIndex)
            {
                VertexRangeData rangeData;
                rangeData.index = currentVertexIndex;
                rangeData.size = static_cast<uint32>(vertexData.size() - currentVertexIndex);
            
                perCellOffsets.push_back(rangeData);
                perCellClusterCount.push_back(static_cast<uint32>(clusterIndex - currentCluster));
                
                currentCluster = static_cast<uint32>(clusterIndex);
                currentMatrix = clusterData.cellIndex;
                currentVertexIndex = static_cast<uint32>(vertexData.size());
            }
            
            vertexData.push_back(vertex);
        }
    }
    
    endIndex = static_cast<uint32>(vertexData.size() - 1);
    
    VertexRangeData rangeData;
    rangeData.index = currentVertexIndex;
    rangeData.size =  static_cast<uint32>(vertexData.size() - currentVertexIndex);
        
    perCellOffsets.push_back(rangeData);
    perCellClusterCount.push_back(static_cast<uint32>(clusterCount - currentCluster));

}

void VegetationCustomGeometry::Rotate(float32 angle, Vector<Vector3>& sourcePositions, Vector<Vector3>& rotatedPositions)
{
    rotatedPositions.clear();

    Matrix4 rotMat;
    rotMat.CreateRotation(Vector3(0.0f, 0.0f, 1.0f), DegToRad(angle));
    
    size_t sourceVertexCount = sourcePositions.size();
    for(size_t vertexIndex = 0; vertexIndex < sourceVertexCount; ++vertexIndex)
    {
        Vector3 transformedVertex = sourcePositions[vertexIndex] * rotMat;
        rotatedPositions.push_back(transformedVertex);
    }
}

void VegetationCustomGeometry::GenerateIndexData(Vector<VegetationIndex>& sourceIndices,
                                                 VegetationIndex startIndex,
                                                 uint32 clusterCount,
                                                 uint32 clusterVertexCount,
                                                 Vector<VegetationVertex>& vertexData,
                                                 Vector<VegetationIndex>& indexData,
                                                 Vector<SortBufferData>& directionOffsets)
{
    uint32 totalIndexCount = static_cast<uint32>(sourceIndices.size() * clusterCount);
    
    size_t clusterIndexCount = sourceIndices.size();
    Vector<VegetationIndex> sourceCellIndices;
    sourceCellIndices.resize(totalIndexCount);
    for(uint32 clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
    {
        for(size_t i = 0; i < clusterIndexCount; ++i)
        {
            sourceCellIndices[clusterIndex * clusterIndexCount + i] = startIndex + clusterIndex * clusterVertexCount + sourceIndices[i];
        }
    }

    AABBox3 boundingBox;
    Vector<PolygonSortData> sortDataArray;
    size_t sortItemCount = totalIndexCount / 3; //triangle is described by 3 vertices
    sortDataArray.resize(sortItemCount);
    for(size_t sortItemIndex = 0; sortItemIndex < sortItemCount; ++sortItemIndex)
    {
        PolygonSortData& sortData = sortDataArray[sortItemIndex];
        
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
            
            //indexData.push_back(sortData.indices[0] - startIndex);
            //indexData.push_back(sortData.indices[1] - startIndex);
            //indexData.push_back(sortData.indices[2] - startIndex);
            
            indexData.push_back(sortData.indices[0]);
            indexData.push_back(sortData.indices[1]);
            indexData.push_back(sortData.indices[2]);
        }
    }
}
    
void VegetationCustomGeometry::GenerateSortedClusterIndexData(Vector3& cameraPosition,
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

void VegetationCustomGeometry::BuildLayer(uint32 layerId,
                                          CustomGeometryEntityData& sourceLayerData,
                                          Vector<VegetationVertex>& vertexData,
                                          Vector<VegetationIndex>& indexData,
                                          Vector<Vector<VertexRangeData> >& vertexOffsets, //resolution-cell
                                          Vector<Vector<Vector<SortBufferData> > >& indexOffsets) //resolution-cell-direction
{
    DVASSERT(layerId < maxClusters.size());
    
    uint32 layerMaxClusters = maxClusters[layerId];
    Vector<ClusterPositionData> clusters;
    GenerateClusterPositionData(layerMaxClusters, clusters);
    
    for(uint32 resolutionIndex = 0;
        resolutionIndex < resolutionCount;
        ++resolutionIndex)
    {
        indexOffsets.push_back(Vector<Vector<SortBufferData> >());
        vertexOffsets.push_back(Vector<VertexRangeData>());
        
        Vector<Vector<SortBufferData> >& resolutionOffsets = indexOffsets[indexOffsets.size() - 1];
        Vector<VertexRangeData>& resolutionVertexOffsets = vertexOffsets[vertexOffsets.size() - 1];
        
        Vector<ClusterResolutionData> clusterResolution;
        
        GenerateClusterResolutionData(layerId,
                                      layerMaxClusters,
                                      resolutionIndex,
                                      clusters,
                                      clusterResolution);
        std::stable_sort(clusterResolution.begin(), clusterResolution.end(), ClusterByMatrixCompareFunction);
        
        uint32 vertexStartIndex = 0;
        uint32 vertexEndIndex = 0;
        Vector<uint32> perCellClusterCount;
        GenerateVertexData(sourceLayerData.lods[resolutionIndex].sourcePositions,
                           sourceLayerData.lods[resolutionIndex].sourceTextureCoords,
                           sourceLayerData.lods[resolutionIndex].sourceNormals,
                           clusterResolution,
                           vertexStartIndex,
                           vertexEndIndex,
                           vertexData,
                           vertexOffsets[resolutionIndex],
                           perCellClusterCount);
        
        uint32 cellCount = static_cast<uint32>(resolutionVertexOffsets.size());
        for(uint32 cellIndex = 0; cellIndex < cellCount; cellIndex++)
        {
            resolutionOffsets.push_back(Vector<SortBufferData>());
            Vector<SortBufferData>& currentIndexOffsets = resolutionOffsets[resolutionOffsets.size() - 1];
            
            GenerateIndexData(sourceLayerData.lods[resolutionIndex].sourceIndices,
                              vertexOffsets[resolutionIndex][cellIndex].index,
                              perCellClusterCount[cellIndex],
                              static_cast<uint32>(sourceLayerData.lods[resolutionIndex].sourcePositions.size()),
                              vertexData,
                              indexData,
                              currentIndexOffsets);
        }
    }
}

void VegetationCustomGeometry::InitCustomGeometry(VegetationCustomGeometrySerializationData* geometryData)
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
    }
}

};
