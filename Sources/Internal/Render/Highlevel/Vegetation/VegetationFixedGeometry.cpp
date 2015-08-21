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
#include "Render/Highlevel/Vegetation/VegetationFixedGeometry.h"
#include "Utils/Random.h"
#include "FileSystem/KeyedArchive.h"
#include "Render/Highlevel/Vegetation/VegetationPropertyNames.h"

namespace DAVA
{

using CLUSTER_TYPE_ARRAY = Array<Vector3, 4>;

static const CLUSTER_TYPE_ARRAY CLUSTER_TYPE_0 =
{
    Vector3(-0.5f, 0.0f, 1.0f),
    Vector3(0.5f, 0.0f, 1.0f),
    Vector3(0.5f, 0.0f, 0.0f),
    Vector3(-0.5f, 0.0f, 0.0f),
};


static const CLUSTER_TYPE_ARRAY CLUSTER_TYPE_1 =
{
    Vector3(-0.5f, 0.1f, 1.0f),
    Vector3(0.5f, 0.1f, 1.0f),
    Vector3(0.5f, 0.1f, 0.0f),
    Vector3(-0.5f, 0.1f, 0.0f),
};

static const CLUSTER_TYPE_ARRAY CLUSTER_TYPE_0_NORMALS =
{
    Vector3(0.0f, 0.0f, 1.0f),
    Vector3(0.0f, 0.0f, -1.0f),
    Vector3(0.0f, 0.0f, -1.0f),
    Vector3(0.0f, 0.0f, 1.0f),
};

static const CLUSTER_TYPE_ARRAY CLUSTER_TYPE_1_NORMALS =
{
    Vector3(0.0f, 0.0f, 1.0f),
    Vector3(0.0f, 0.0f, -1.0f),
    Vector3(0.0f, 0.0f, -1.0f),
    Vector3(0.0f, 0.0f, 1.0f),
};

using CLUSTER_INDICES_ARRAY = Array<VegetationIndex, 6>;
static const CLUSTER_INDICES_ARRAY CLUSTER_INDICES =
{
    0, 3,  1, 1, 3,  2
};

static const Array<CLUSTER_TYPE_ARRAY, 2> VEGETATION_CLUSTER =
{
    CLUSTER_TYPE_0,
    CLUSTER_TYPE_1
};

static const Array<CLUSTER_INDICES_ARRAY, 2> VEGETATION_CLUSTER_INDICES =
{
    CLUSTER_INDICES,
    CLUSTER_INDICES
};

static const Array<CLUSTER_TYPE_ARRAY, 2> VEGETATION_CLUSTER_NORMALS =
{
    CLUSTER_TYPE_0_NORMALS,
    CLUSTER_TYPE_1_NORMALS
};

static const size_t VEGETATION_CLUSTER_SIZE[] =
{
    CLUSTER_TYPE_0.size(),
    CLUSTER_TYPE_1.size()
};

static const size_t VEGETATION_CLUSTER_INDEX_SIZE[] =
{
    CLUSTER_INDICES.size(),
    CLUSTER_INDICES.size()
};

int32 VegetationFixedGeometry::RandomShuffleFunc(int32 limit)
{
    return (Random::Instance()->Rand() % limit);
}
    
VegetationFixedGeometry::VegetationFixedGeometry(uint32 _maxClusters,
                                                 uint32 _maxDensityLevels,
                                                 uint32 _maxLayerTypes,
                                                 Vector2 _unitSize,
                                                 const FilePath& textureSheetPath,
                                                 const Vector<uint32> & _resolutionCellSquare,
                                                 const Vector<float32> & _resolutionScale,
                                                 const Vector<Vector2> & _resolutionRanges,
                                                 const Vector<uint32> & _resolutionTilesPerRow,
                                                 Vector3 _worldSize)
    : maxClusters(_maxClusters)
    , maxDensityLevels(_maxDensityLevels)
    , maxLayerTypes(_maxLayerTypes)
    , resolutionCellSquare(_resolutionCellSquare)
    , resolutionScale(_resolutionScale)
    , resolutionRanges(_resolutionRanges)
    , resolutionTilesPerRow(_resolutionTilesPerRow)
    , unitSize(_unitSize)
    , worldSize(_worldSize)
{
    textureSheet.Load(textureSheetPath);
    
    materialTransform = new FixedMaterialTransformer();
}


void VegetationFixedGeometry::Build(Vector<VegetationRenderData*>& renderDataArray, const FastNameSet& materialFlags)
{
    ReleaseRenderData(renderDataArray);
    renderDataArray.push_back(new VegetationRenderData());
    VegetationRenderData& renderData = *renderDataArray[0];
    
    NMaterial* vegetationMaterial = NMaterial::CreateMaterial(FastName("Vegetation_Material"),
                                                   NMaterialName::GRASS,
                                                   NMaterial::DEFAULT_QUALITY_NAME);
    vegetationMaterial->SetRuntime(true);
    
    FastNameSet::iterator end = materialFlags.end();
    for(FastNameSet::iterator it = materialFlags.begin(); it != end; ++it)
    {
        vegetationMaterial->SetFlag(it->first, NMaterial::FlagOn);
    }
    
    if(RenderManager::Instance()->GetCaps().isFramebufferFetchSupported)
    {
        vegetationMaterial->SetFlag(VegetationPropertyNames::FLAG_FRAMEBUFFER_FETCH, NMaterial::FlagOn);
    }
    
    vegetationMaterial->SetPropertyValue(VegetationPropertyNames::UNIFORM_WORLD_SIZE,
                                         Shader::UT_FLOAT_VEC3,
                                         1,
                                         &worldSize);
    
    renderData.SetMaterial(vegetationMaterial);
    SafeRelease(vegetationMaterial);
    
    size_t resolutionCount = resolutionScale.size();
    uint32 sortDirectionCount = GetSortDirectionCount();
    Vector<VegetationIndex>& indexData = renderData.GetIndices();
    Vector<VegetationVertex>& vertexData = renderData.GetVertices();
    
    uint32 tilesPerRow = (uint32)resolutionScale[resolutionCount - 1];
    uint32 maxClusterRowSize = (tilesPerRow * maxClusters);
    size_t maxTotalClusters = maxClusterRowSize * maxClusterRowSize;
    
    size_t layerDataCount = 0;
    size_t indexDataCount = 0;
    for(uint32 layerIndex = 0; layerIndex < maxLayerTypes; ++layerIndex)
    {
        TextureSheetCell& cellData = textureSheet.cells[layerIndex];
        layerDataCount += VEGETATION_CLUSTER_SIZE[cellData.geometryId];
        indexDataCount += VEGETATION_CLUSTER_INDEX_SIZE[cellData.geometryId];
    }
    
    size_t totalIndexCount = 0;
    for(size_t i = 0; i < resolutionCount; ++i)
    {
        totalIndexCount += indexDataCount * (maxTotalClusters / (uint32)resolutionScale[i]);
    }
    
    totalIndexCount *= sortDirectionCount;
    
    indexData.resize(totalIndexCount);
    vertexData.resize(maxTotalClusters * layerDataCount);
    
    Vector<uint32> layerOffsets(maxLayerTypes);
    
    GenerateVertices(maxClusters,
                     maxTotalClusters,
                     maxClusterRowSize,
                     tilesPerRow,
                     unitSize,
                     layerOffsets,
                     renderData);
    
    GenerateIndices(maxClusters, maxClusterRowSize, layerOffsets, renderData);
    
    //VI: need to build vertex & index objects AFTER initialization
    GenerateRenderDataObjects(renderData);
    
}
    
void VegetationFixedGeometry::GenerateVertices(uint32 maxClusters,
                    size_t maxTotalClusters,
                    uint32 maxClusterRowSize,
                    uint32 tilesPerRow,
                    Vector2 unitSize,
                    Vector<uint32>& layerOffsets,
                    VegetationRenderData& renderData)
{
    uint32 clustersPerTile = maxClusters * maxClusters;
    float32 atomicOffsetY = unitSize.y / clustersPerTile;
    
    Vector<uint32> shuffleDepth;
    shuffleDepth.reserve(maxTotalClusters);
    
    Vector<uint32> shuffleDensity;
    shuffleDensity.reserve(maxTotalClusters);
    
    Vector<uint32> depthScratchArray;
    depthScratchArray.reserve(clustersPerTile);
    
    Vector<uint32> densityScratchArray;
    densityScratchArray.reserve(clustersPerTile);
    
    for(uint32 i = 0; i < clustersPerTile; ++i)
    {
        depthScratchArray[i] = i;
        densityScratchArray[i] = (i % maxDensityLevels) + 1;
    }
    
    Vector<VegetationVertex>& vertexData = renderData.GetVertices();
    
    size_t vertexIndex = 0;
    uint32 totalTiles = tilesPerRow * tilesPerRow;
    for(uint32 layerIndex = 0; layerIndex < maxLayerTypes; ++layerIndex)
    {
        layerOffsets[layerIndex] = static_cast<uint32>(vertexIndex);
        
        TextureSheetCell& cellData = textureSheet.cells[layerIndex];
        
        const auto clusterVertices = VEGETATION_CLUSTER[cellData.geometryId];
        const auto clusterNormals = VEGETATION_CLUSTER_NORMALS[cellData.geometryId];
        size_t clusterVertexCount = VEGETATION_CLUSTER_SIZE[cellData.geometryId];
        
        uint32 clusterCounter = 0;
        for(uint32 i = 0; i < totalTiles; ++i)
        {
            std::random_shuffle(densityScratchArray.begin(), densityScratchArray.end(), RandomShuffleFunc);
            std::random_shuffle(depthScratchArray.begin(), depthScratchArray.end(), RandomShuffleFunc);
            for(uint32 k = 0; k < clustersPerTile; ++k)
            {
                shuffleDensity[clusterCounter] = densityScratchArray[k];
                shuffleDepth[clusterCounter] = depthScratchArray[k];
                clusterCounter++;
            }
        }
        
        DVASSERT(clusterCounter == maxTotalClusters);
        
        AABBox3 clusterBBox;
        for(uint32 clusterVertexIndex = 0; clusterVertexIndex < clusterVertexCount; ++clusterVertexIndex)
        {
            clusterBBox.AddPoint(clusterVertices[clusterVertexIndex]);
        }
        
        float32 clusterVisualSize = cellData.geometryScale.x * (clusterBBox.max.x - clusterBBox.min.x);
        uint32 clustersInRow = (uint32)(unitSize.x / clusterVisualSize);
        if(clustersInRow <= 0)
        {
            clustersInRow = 1;
        }
        
        for(size_t clusterIndex = 0; clusterIndex < maxTotalClusters; ++clusterIndex)
        {
            uint32 clusterIndexX = clusterIndex % maxClusterRowSize;
            uint32 clusterIndexY = static_cast<uint32>(clusterIndex / maxClusterRowSize);
            
            uint32 matrixIndex = (clusterIndexX / maxClusters) + tilesPerRow * (clusterIndexY / maxClusters); //0...15
            DVASSERT(matrixIndex < (tilesPerRow * tilesPerRow));
            
            uint32 matrixIndexX = matrixIndex % tilesPerRow;
            uint32 matrixIndexY = matrixIndex / tilesPerRow;
            
            Vector2 matrixCellStart(unitSize.x * matrixIndexX, unitSize.y * matrixIndexY);
            
            float32 randomOffsetX = clusterVisualSize * (clusterIndex % clustersInRow) + clusterVisualSize * (0.5f - (float32)Random::Instance()->RandFloat());//unitSize.x * Random::Instance()->RandFloat();
            float32 randomOffsetY = atomicOffsetY * shuffleDepth[clusterIndex]; //unitSize.y * Random::Instance()->RandFloat();
            
            Vector3 clusterCenter(matrixCellStart.x + randomOffsetX, matrixCellStart.y + randomOffsetY, 0.0f);
            
            uint32 densityId = shuffleDensity[clusterIndex];
            
            for(uint32 clusterVertexIndex = 0; clusterVertexIndex < clusterVertexCount; ++clusterVertexIndex)
            {
                DVASSERT(vertexIndex < vertexData.size());
                VegetationVertex& vertex = vertexData[vertexIndex];
                
                vertex.coord.x = clusterCenter.x + clusterVertices[clusterVertexIndex].x * cellData.geometryScale.x;
                vertex.coord.y = clusterCenter.y + clusterVertices[clusterVertexIndex].y * cellData.geometryScale.x;
                vertex.coord.z = clusterCenter.z + clusterVertices[clusterVertexIndex].z * cellData.geometryScale.y;
                
                vertex.normal = clusterNormals[clusterVertexIndex];
                
                vertex.binormal = clusterCenter;
                
                vertex.tangent.y = (float32)layerIndex;
                vertex.tangent.z = (float32)densityId;
                
                vertex.texCoord0.x = cellData.coords[clusterVertexIndex % MAX_CELL_TEXTURE_COORDS].x;
                vertex.texCoord0.y = cellData.coords[clusterVertexIndex % MAX_CELL_TEXTURE_COORDS].y;
                
                vertexIndex++;
            }
        }
    }
}
    
void VegetationFixedGeometry::GenerateIndices(uint32 maxClusters,
                    uint32 maxClusterRowSize,
                    Vector<uint32>& layerOffsets,
                    VegetationRenderData& renderData)
{
    Vector<PolygonSortData> sortingArray(1);
    Vector<VegetationIndex> preparedIndices;
    const size_t polygonElementCount = sortingArray[0].indices.size();
    
    //generate indices
    const size_t totalResolutionCount = resolutionRanges.size();
    size_t currentIndexIndex = 0;
    
    Vector<Vector<Vector<VegetationSortedBufferItem> > >& indexRenderDataObject = renderData.GetIndexBuffers();
    Vector<Vector3> directionPoints;
    
    for(size_t resolutionIndex = 0; resolutionIndex < totalResolutionCount; ++resolutionIndex)
    {
        uint32 resolutionOffset = (uint32)resolutionScale[resolutionIndex];
        uint32 indexBufferCount = resolutionCellSquare[resolutionCellSquare.size() - 1] / resolutionCellSquare[resolutionIndex];
        
        indexRenderDataObject.push_back(Vector<Vector<VegetationSortedBufferItem> >());
        
        Vector<Vector<VegetationSortedBufferItem> >& currentResolutionIndexArray = indexRenderDataObject[resolutionIndex];
        
        for(uint32 i = 0; i < indexBufferCount; ++i)
        {
            AABBox3 indexBufferBBox;
            
            PrepareIndexBufferData(i,
                                   maxClusters,
                                   maxClusterRowSize,
                                   resolutionIndex,
                                   resolutionOffset,
                                   layerOffsets,
                                   preparedIndices,
                                   indexBufferBBox,
                                   renderData);
            
            directionPoints.clear();
            SetupCameraPositions(indexBufferBBox, directionPoints);
            
            PrepareSortedIndexBufferVariations(currentIndexIndex,
                                               i,
                                               polygonElementCount,
                                               indexBufferBBox,
                                               directionPoints,
                                               currentResolutionIndexArray,
                                               sortingArray,
                                               preparedIndices,
                                               renderData);
        }
    }
}
    
void VegetationFixedGeometry::PrepareIndexBufferData(uint32 indexBufferIndex,
                        uint32 maxClusters,
                        uint32 maxClusterRowSize,
                        size_t resolutionIndex,
                        uint32 resolutionOffset,
                        Vector<uint32>& layerOffsets,
                        Vector<VegetationIndex>& preparedIndices,
                        AABBox3& indexBufferBBox,
                        VegetationRenderData& renderData)
{
    preparedIndices.clear();
    
    Vector<VegetationVertex>& vertexData = renderData.GetVertices();
    
    uint32 startX = (indexBufferIndex % resolutionTilesPerRow[resolutionIndex]) * maxClusters * resolutionOffset;
    uint32 startY = (indexBufferIndex / resolutionTilesPerRow[resolutionIndex]) * maxClusters * resolutionOffset;
    uint32 endX = startX + maxClusters * resolutionOffset;
    uint32 endY = startY + maxClusters * resolutionOffset;
    
    for(uint32 layerIndex = 0; layerIndex < maxLayerTypes; ++layerIndex)
    {
        TextureSheetCell& cellData = textureSheet.cells[layerIndex];
        
        const auto clusterIndices = VEGETATION_CLUSTER_INDICES[cellData.geometryId];
        const size_t clusterIndexCount = VEGETATION_CLUSTER_INDEX_SIZE[cellData.geometryId];
        const size_t clusterVertexCount = VEGETATION_CLUSTER_SIZE[cellData.geometryId];
        
        for(uint32 y = startY; y < endY; y += resolutionOffset)
        {
            for(uint32 x = startX; x < endX; x += resolutionOffset)
            {
                size_t baseIndex = layerOffsets[layerIndex] + (y * maxClusterRowSize + x) * clusterVertexCount;
                
                for(uint32 clusterIndexIndex = 0; clusterIndexIndex < clusterIndexCount; ++clusterIndexIndex)
                {
                    size_t vertexIndex = baseIndex + clusterIndices[clusterIndexIndex];
                    
                    DVASSERT(vertexIndex < vertexData.size());
                    
                    VegetationVertex& vertex = vertexData[vertexIndex];
                    vertex.tangent.x = (float32)resolutionIndex;
                    
                    indexBufferBBox.AddPoint(vertex.coord);
                    
                    preparedIndices.push_back(static_cast<int32>(vertexIndex));
                }
            }
        }
    }
}
    
void VegetationFixedGeometry::PrepareSortedIndexBufferVariations(size_t& currentIndexIndex,
                                    uint32 indexBufferIndex,
                                    size_t polygonElementCount,
                                    AABBox3& indexBufferBBox,
                                    Vector<Vector3>& directionPoints,
                                    Vector<Vector<VegetationSortedBufferItem> >& currentResolutionIndexArray,
                                    Vector<PolygonSortData>& sortingArray,
                                    Vector<VegetationIndex>& preparedIndices,
                                    VegetationRenderData& renderData)
{
    size_t sortItemCount = preparedIndices.size() / polygonElementCount;
    sortingArray.resize(sortItemCount);
    for(size_t sortItemIndex = 0; sortItemIndex < sortItemCount; ++sortItemIndex)
    {
        PolygonSortData& sortData = sortingArray[sortItemIndex];
        
        sortData.indices[0] = preparedIndices[sortItemIndex * polygonElementCount + 0];
        sortData.indices[1] = preparedIndices[sortItemIndex * polygonElementCount + 1];
        sortData.indices[2] = preparedIndices[sortItemIndex * polygonElementCount + 2];
    }
    
    Vector<VegetationIndex>& indexData = renderData.GetIndices();
    Vector<VegetationVertex>& vertexData = renderData.GetVertices();
    
    currentResolutionIndexArray.push_back(Vector<VegetationSortedBufferItem>());
    Vector<VegetationSortedBufferItem>& currentDirectionBuffers = currentResolutionIndexArray[indexBufferIndex];
    
    uint32 sortDirectionCount = (uint32)directionPoints.size();
    for(uint32 sortDirectionIndex = 0; sortDirectionIndex < sortDirectionCount; ++sortDirectionIndex)
    {
        Vector3 cameraPosition = directionPoints[sortDirectionIndex];
        
        for(size_t sortItemIndex = 0; sortItemIndex < sortItemCount; ++sortItemIndex)
        {
            PolygonSortData& sortData = sortingArray[sortItemIndex];
            sortData.cameraDistance = FLT_MAX;
            
            for(uint32 polygonIndex = 0; polygonIndex < polygonElementCount; ++polygonIndex)
            {
                float32 distance = (vertexData[sortData.indices[polygonIndex]].coord - cameraPosition).SquareLength();
                if(distance < sortData.cameraDistance)
                {
                    sortData.cameraDistance = distance;
                }
            }
        }
        
        std::stable_sort(sortingArray.begin(), sortingArray.end(), PolygonByDistanceCompareFunction);
        
        size_t prevIndexIndex = currentIndexIndex;
        for(size_t sortItemIndex = 0; sortItemIndex < sortItemCount; ++sortItemIndex)
        {
            PolygonSortData& sortData = sortingArray[sortItemIndex];
            
            DVASSERT(currentIndexIndex < indexData.size());
            
            indexData[currentIndexIndex] = sortData.indices[0];
            currentIndexIndex++;
            
            indexData[currentIndexIndex] = sortData.indices[1];
            currentIndexIndex++;
            
            indexData[currentIndexIndex] = sortData.indices[2];
            currentIndexIndex++;
        }
        
        RenderDataObject* indexBuffer = new RenderDataObject();
        indexBuffer->SetIndices(VEGETATION_INDEX_TYPE, (uint8*)(&indexData[prevIndexIndex]), static_cast<int32>(currentIndexIndex - prevIndexIndex));
        
        VegetationSortedBufferItem sortedBufferItem;
        sortedBufferItem.SetRenderDataObject(indexBuffer);
        sortedBufferItem.sortDirection = indexBufferBBox.GetCenter() - cameraPosition;
        sortedBufferItem.sortDirection.Normalize();
        
        SafeRelease(indexBuffer);
        
        currentDirectionBuffers.push_back(sortedBufferItem);
    }
}
    
void VegetationFixedGeometry::GenerateRenderDataObjects(VegetationRenderData& renderData)
{
    renderData.CreateRenderData();
    
    RenderDataObject* vertexRenderDataObject = renderData.GetRenderDataObject();
    Vector<Vector<Vector<VegetationSortedBufferItem> > >& indexRenderDataObject = renderData.GetIndexBuffers();
    Vector<VegetationVertex>& vertexData = renderData.GetVertices();
    
    vertexRenderDataObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, sizeof(VegetationVertex), &(vertexData[0].coord));
    vertexRenderDataObject->SetStream(EVF_NORMAL, TYPE_FLOAT, 3, sizeof(VegetationVertex), &(vertexData[0].normal));
    vertexRenderDataObject->SetStream(EVF_BINORMAL, TYPE_FLOAT, 3, sizeof(VegetationVertex), &(vertexData[0].binormal));
    vertexRenderDataObject->SetStream(EVF_TANGENT, TYPE_FLOAT, 3, sizeof(VegetationVertex), &(vertexData[0].tangent));
    vertexRenderDataObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, sizeof(VegetationVertex), &(vertexData[0].texCoord0));
    vertexRenderDataObject->BuildVertexBuffer(static_cast<uint32>(vertexData.size()), BDT_STATIC_DRAW, true);
    
    size_t totalIndexObjectArrayCount = indexRenderDataObject.size();
    for(size_t indexArrayIndex = 0; indexArrayIndex < totalIndexObjectArrayCount; ++indexArrayIndex)
    {
        Vector<Vector<VegetationSortedBufferItem> >& indexObjectArray = indexRenderDataObject[indexArrayIndex];
        size_t totalIndexObjectCount = indexObjectArray.size();
        
        for(size_t i = 0; i < totalIndexObjectCount; ++i)
        {
            Vector<VegetationSortedBufferItem>& directionArray = indexObjectArray[i];
            size_t directionCount = directionArray.size();
            for(size_t directionIndex = 0; directionIndex < directionCount; ++directionIndex)
            {
                directionArray[directionIndex].rdo->BuildIndexBuffer(BDT_STATIC_DRAW, true);
                directionArray[directionIndex].rdo->AttachVertices(vertexRenderDataObject);
            }
        }
    }
    
}

bool VegetationFixedGeometry::PolygonByDistanceCompareFunction(const PolygonSortData& a, const PolygonSortData&  b)
{
    return a.cameraDistance > b.cameraDistance; //back to front order
}

void VegetationFixedGeometry::OnVegetationPropertiesChanged(Vector<VegetationRenderData*>& renderDataArray, KeyedArchive* props)
{
    DVASSERT(renderDataArray.size() <= 1);
    
    if(renderDataArray.size() > 0)
    {
        NMaterial* mat = renderDataArray[0]->GetMaterial();
    
        String albedoKey = NMaterial::TEXTURE_ALBEDO.c_str();
        if(props->IsKeyExists(albedoKey))
        {
            FilePath albedoPath = props->GetString(albedoKey);
            mat->SetTexture(NMaterial::TEXTURE_ALBEDO, albedoPath);
        }
        
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
        
        String perturbationForceKeyName = VegetationPropertyNames::UNIFORM_PERTURBATION_FORCE.c_str();
        if(props->IsKeyExists(perturbationForceKeyName))
        {
            Vector3 perturbationForce = props->GetVector3(perturbationForceKeyName);
            mat->SetPropertyValue(VegetationPropertyNames::UNIFORM_PERTURBATION_FORCE,
                                  Shader::UT_FLOAT_VEC3,
                                  1,
                                  perturbationForce.data);
        }
        
        String perturbationForceDistanceKeyName = VegetationPropertyNames::UNIFORM_PERTURBATION_FORCE_DISTANCE.c_str();
        if(props->IsKeyExists(perturbationForceDistanceKeyName))
        {
            float32 perturbationForceDistance = props->GetFloat(perturbationForceDistanceKeyName);
            mat->SetPropertyValue(VegetationPropertyNames::UNIFORM_PERTURBATION_FORCE_DISTANCE,
                                  Shader::UT_FLOAT,
                                  1,
                                  &perturbationForceDistance);
        }
        
        String perturbationPointKeyName = VegetationPropertyNames::UNIFORM_PERTURBATION_POINT.c_str();
        if(props->IsKeyExists(perturbationPointKeyName))
        {
            Vector3 perturbationPoint = props->GetVector3(perturbationPointKeyName);
            mat->SetPropertyValue(VegetationPropertyNames::UNIFORM_PERTURBATION_POINT,
                                  Shader::UT_FLOAT_VEC3,
                                  1,
                                  perturbationPoint.data);
        }

    }
}

////////////////////////////////////////////////////////////////////////////////

void VegetationFixedGeometry::FixedMaterialTransformer::TransformMaterialOnCreate(NMaterial* mat)
{
    if(false == RenderManager::Instance()->GetCaps().isFramebufferFetchSupported)
    {
        NMaterialHelper::EnableStateFlags(DAVA::PASS_FORWARD,
                                          mat,
                                          RenderStateData::STATE_BLEND);
    }
};

};
