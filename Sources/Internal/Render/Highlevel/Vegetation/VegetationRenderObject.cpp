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

#include "Render/Highlevel/Vegetation/VegetationRenderObject.h"
#include "Render/Material/NMaterialNames.h"
#include "Render/Material/NMaterial.h"
#include "Utils/Random.h"
#include "Render/Image/ImageSystem.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Scene3D/Systems/FoliageSystem.h"
#include "Render/RenderHelper.h"
#include "Platform/SystemTimer.h"
#include "Job/JobManager.h"

#include "Render/Highlevel/Vegetation/VegetationGeometry.h"
#include "Render/Highlevel/RenderPassNames.h"
#include "Render/RenderCallbacks.h"

namespace DAVA
{

static const uint32 MAX_CLUSTER_TYPES = 4;
static const uint32 MAX_DENSITY_LEVELS = 16;
//static const float32 CLUSTER_SCALE_NORMALIZATION_VALUE = 15.0f;
    
static const size_t MAX_RENDER_CELLS = 512;
static const float32 MAX_VISIBLE_CLIPPING_DISTANCE = 50.0f * 50.0f; //meters * meters (square length)
static const float32 MAX_VISIBLE_SCALING_DISTANCE = 40.0f * 40.0f;

static const uint32 DENSITY_MAP_SIZE = 128;
static const float32 DENSITY_THRESHOLD = 0.0f;

//static const float32 MAX_VISIBLE_CLIPPING_DISTANCE = 130.0f * 130.0f; //meters * meters (square length)
//static const float32 MAX_VISIBLE_SCALING_DISTANCE = 100.0f * 100.0f;
    
//static const uint32 FULL_BRUSH_VALUE = 0xFFFFFFFF;
  
static Vector3 LOD_RANGES_SCALE = Vector3(0.0f, 2.0f, 6.0f);
//static Vector3 LOD_RANGES_SCALE = Vector3(0.0f, 2.0f, 6.0f);

static Vector<float32> RESOLUTION_SCALE =
{
    1.0f,
    2.0f,
    4.0f,
};

static Vector<uint32> RESOLUTION_CELL_SQUARE =
{
    1,
    4,
    16
};

static Vector<uint32> RESOLUTION_TILES_PER_ROW =
{
    4,
    2,
    1
};

static Vector<uint32> RESOLUTION_CLUSTER_STRIDE =
{
    1,
    2,
    4
};

//#define VEGETATION_DRAW_LOD_COLOR

static Vector<Color> RESOLUTION_COLOR =
{
    Color(0.5f, 0.0f, 0.0f, 1.0f),
    Color(0.0f, 0.5f, 0.0f, 1.0f),
    Color(1.0f, 0.0f, 1.0f, 1.0f),
};

#ifdef VEGETATION_DRAW_LOD_COLOR
static const FastName UNIFORM_LOD_COLOR = FastName("lodColor");
#endif
    
VegetationRenderObject::VegetationRenderObject() :
    heightmap(nullptr),
    halfWidth(0),
    halfHeight(0),
    maxPerturbationDistance(1000000.0f),
    layerVisibilityMask(0xFF),
    vegetationVisible(true),
    vegetationGeometry(NULL),
    heightmapTexture(NULL),
    //cameraBias(25.0f)
    cameraBias(0.0f),
    layersAnimationSpring(2.f, 2.f, 2.f, 2.f),
    layersAnimationDrag(1.4f, 1.4f, 1.4f, 1.4f),
    renderData(nullptr),
    vertexCount(0),
    indexCount(0)
{
    bbox.AddPoint(Vector3(0, 0, 0));
    bbox.AddPoint(Vector3(1, 1, 1));
    
    layerParams.resize(MAX_CLUSTER_TYPES);
    for(size_t i = 0; i < MAX_CLUSTER_TYPES; ++i)
    {
        layerParams[i].maxClusterCount = 1;
        layerParams[i].instanceScaleVariation = 0.0f;
        layerParams[i].instanceRotationVariation = 0.0f;
    }
    
    type = RenderObject::TYPE_VEGETATION;
    AddFlag(RenderObject::ALWAYS_CLIPPING_VISIBLE);
    AddFlag(RenderObject::CUSTOM_PREPARE_TO_RENDER);
    
    unitWorldSize.resize(RESOLUTION_SCALE.size());
    resolutionRanges.resize(RESOLUTION_CELL_SQUARE.size());
    
    maxVisibleQuads = MAX_RENDER_CELLS;
    lodRanges = LOD_RANGES_SCALE;
    ResetVisibilityDistance();
    RenderCallbacks::RegisterResourceRestoreCallback(MakeFunction(this, &VegetationRenderObject::RestoreRenderData));
}

VegetationRenderObject::~VegetationRenderObject()
{
    if (renderData)
    {
        delete renderData;
        rhi::DeleteVertexBuffer(vertexBuffer);
        rhi::DeleteIndexBuffer(indexBuffer);
    }

    SafeDelete(vegetationGeometry);

    SafeRelease(heightmap);
    SafeRelease(heightmapTexture);
    RenderCallbacks::UnRegisterResourceRestoreCallback(MakeFunction(this, &VegetationRenderObject::RestoreRenderData));
}

RenderBatch * VegetationRenderObject::CreateRenderBatch()
{
    DVASSERT(renderData);

    NMaterial * batchMaterial = new NMaterial();
    batchMaterial->SetParent(renderData->GetMaterial());

    float32 fakeData[4];
    Memset(fakeData, 0, sizeof(float32) * 4);
    batchMaterial->AddProperty(VegetationPropertyNames::UNIFORM_SWITCH_LOD_SCALE, fakeData, rhi::ShaderProp::TYPE_FLOAT2);
    batchMaterial->AddProperty(VegetationPropertyNames::UNIFORM_TILEPOS, fakeData, rhi::ShaderProp::TYPE_FLOAT3);
    batchMaterial->AddProperty(VegetationPropertyNames::UNIFORM_VEGWAVEOFFSET_X, fakeData, rhi::ShaderProp::TYPE_FLOAT4);
    batchMaterial->AddProperty(VegetationPropertyNames::UNIFORM_VEGWAVEOFFSET_Y, fakeData, rhi::ShaderProp::TYPE_FLOAT4);

    RenderBatch * batch = new RenderBatch();
    batch->SetMaterial(batchMaterial);
    batch->vertexBuffer = vertexBuffer;
    batch->indexBuffer = indexBuffer;
    batch->vertexCount = vertexCount;
    batch->vertexLayoutId = vertexLayoutUID;

    return batch;
}

RenderObject* VegetationRenderObject::Clone(RenderObject *newObject)
{
    if(!newObject)
    {
        DVASSERT_MSG(IsPointerToExactClass<VegetationRenderObject>(this), "Can clone only from VegetationRenderObject");
        newObject = new VegetationRenderObject();
    }
    else
    {
        DVASSERT_MSG(IsPointerToExactClass<VegetationRenderObject>(this), "Can clone only from VegetationRenderObject");
        DVASSERT_MSG(IsPointerToExactClass<VegetationRenderObject>(newObject), "Can clone only to VegetationRenderObject");
    }
    
    VegetationRenderObject* vegetationRenderObject = static_cast<VegetationRenderObject*>(newObject);
    
    vegetationRenderObject->customGeometryData.reset();
    if(customGeometryData)
    {
        vegetationRenderObject->customGeometryData.reset(new VegetationGeometryData(*customGeometryData));
    }
    
    vegetationRenderObject->densityMap.clear();
    
    vegetationRenderObject->SetVisibilityDistance(GetVisibilityDistance()); //VI: must be copied before lod ranges
    vegetationRenderObject->SetHeightmap(GetHeightmap());
    vegetationRenderObject->SetLayerClusterLimit(GetLayerClusterLimit());
    vegetationRenderObject->SetScaleVariation(GetScaleVariation());
    vegetationRenderObject->SetRotationVariation(GetRotationVariation());
    vegetationRenderObject->SetHeightmapPath(GetHeightmapPath());
    vegetationRenderObject->SetLightmap(GetLightmapPath());
    vegetationRenderObject->SetWorldSize(GetWorldSize());
    vegetationRenderObject->SetCustomGeometryPathInternal(GetCustomGeometryPath());
    vegetationRenderObject->SetCameraBias(GetCameraBias());
    vegetationRenderObject->SetLayersAnimationAmplitude(GetLayersAnimationAmplitude());
    vegetationRenderObject->SetLayersAnimationSpring(GetLayersAnimationSpring());
    vegetationRenderObject->SetDensityMap(densityMap);
    vegetationRenderObject->SetLayerAnimationDragCoefficient(GetLayerAnimationDragCoefficient());
    vegetationRenderObject->SetLodRange(GetLodRange());
    
    vegetationRenderObject->AddFlag(RenderObject::ALWAYS_CLIPPING_VISIBLE);
    vegetationRenderObject->AddFlag(RenderObject::CUSTOM_PREPARE_TO_RENDER);


    return vegetationRenderObject;
}

void VegetationRenderObject::Save(KeyedArchive *archive, SerializationContext *serializationContext)
{
    //VI: need to remove render batches since they are temporary
    ClearRenderBatches();
    
    RenderObject::Save(archive, serializationContext);
    
    archive->SetVector4("vro.clusterLayerLimit", GetLayerClusterLimit());
    archive->SetVector4("vro.scaleVariation", GetScaleVariation());
    archive->SetVector4("vro.rotationVariation", GetRotationVariation());

    if(lightmapTexturePath.IsEmpty() == false)
	{
		archive->SetString("vro.lightmap", lightmapTexturePath.GetRelativePathname(serializationContext->GetScenePath()));
	}
    
    if(customGeometryPath.IsEmpty() == false)
    {
        archive->SetString("vro.customGeometry", customGeometryPath.GetRelativePathname(serializationContext->GetScenePath()));
    }
    
    archive->SetFloat("vro.cameraBias", cameraBias);
    
    if(customGeometryData)
    {
        KeyedArchive* customGeometryArchive = new KeyedArchive();
        SaveCustomGeometryData(serializationContext, customGeometryArchive, customGeometryData);
        archive->SetArchive("vro.geometryData", customGeometryArchive);
        
        SafeRelease(customGeometryArchive);
    }

    archive->SetVector4("vro.layerAnimationAmplitude", GetLayersAnimationAmplitude());
    archive->SetVector4("vro.layersAnimationSpring", GetLayersAnimationSpring());
    archive->SetVector4("vro.layersAnimationDrug", GetLayerAnimationDragCoefficient());
    
    uint32 bitCount = static_cast<uint32>(densityMap.size());
    archive->SetUInt32("vro.flippedDensityBitCount", bitCount);
    for(uint32 i = 0; i < bitCount; ++i)
    {
        archive->SetBool(Format("vro.flippedDensityBit.%d", i), densityMap[i]);
    }
    
    const Vector3& savingLodRanges = GetLodRange();
    archive->SetVector3("vro.lodRanges", savingLodRanges);
    
    const Vector2& savingVisibilityDistance = GetVisibilityDistance();
    archive->SetVector2("vro.visibilityDistance", savingVisibilityDistance);
}
    
void VegetationRenderObject::Load(KeyedArchive *archive, SerializationContext *serializationContext)
{
    RenderObject::Load(archive, serializationContext);
    
    if(IsDataLoadNeeded())
    {
        //VI: must be loaded before lod ranges
        if(archive->IsKeyExists("vro.visibilityDistance"))
        {
            Vector2 savedVisibilityDistance = archive->GetVector2("vro.visibilityDistance");
            SetVisibilityDistance(savedVisibilityDistance);
        }
    
        if(archive->IsKeyExists("vro.lodRanges"))
        {
            Vector3 savedLodRanges = archive->GetVector3("vro.lodRanges");
            SetLodRange(savedLodRanges);
        }

        if(archive->IsKeyExists("vro.geometryData"))
        {
            KeyedArchive* customGeometryArchive = archive->GetArchive("vro.geometryData");
            customGeometryData = LoadCustomGeometryData(serializationContext, customGeometryArchive);
        }
    
        String customGeometry = archive->GetString("vro.customGeometry");
		if(customGeometry.empty() == false)
		{
            if(customGeometryData)
            {
                SetCustomGeometryPathInternal(serializationContext->GetScenePath() + customGeometry);
            }
            else
            {
                SetCustomGeometryPath(serializationContext->GetScenePath() + customGeometry);
            }
		}
    
        if(archive->IsKeyExists("vro.clusterLimit"))
        {
            SetClusterLimit(archive->GetUInt32("vro.clusterLimit"));
        }
        else
        {
            SetLayerClusterLimit(archive->GetVector4("vro.clusterLayerLimit"));
            //Vector4 fakeClusterLimit(6, 16, 16, 1);
            //SetLayerClusterLimit(fakeClusterLimit);
        }
        
        if(archive->IsKeyExists("vro.scaleVariation"))
        {
            SetScaleVariation(archive->GetVector4("vro.scaleVariation"));
        }
        else
        {
            SetScaleVariation(Vector4(0.2f, 0.2f, 0.2f, 0.2f));
        }
        
        if(archive->IsKeyExists("vro.rotationVariation"))
        {
            SetRotationVariation(archive->GetVector4("vro.rotationVariation"));
        }
        else
        {
            SetRotationVariation(Vector4(180.0f, 180.0f, 180.0f, 180.0f));
        }

		String lightmap = archive->GetString("vro.lightmap");
		if(lightmap.empty() == false)
		{
			SetLightmap(serializationContext->GetScenePath() + lightmap);
		}
        
        if(archive->IsKeyExists("vro.cameraBias"))
        {
            SetCameraBias(archive->GetFloat("vro.cameraBias"));
        }
        
        SetLayersAnimationAmplitude(archive->GetVector4("vro.layerAnimationAmplitude", GetLayersAnimationAmplitude()));
        SetLayersAnimationSpring(archive->GetVector4("vro.layersAnimationSpring", GetLayersAnimationSpring()));
        
        Vector<bool> densityBits;
        if(archive->IsKeyExists("vro.flippedDensityBitCount"))
        {
            uint32 bitCount = archive->GetUInt32("vro.flippedDensityBitCount");
            densityBits.resize(bitCount);
            for(uint32 i = 0; i < bitCount; ++i)
            {
                densityBits[i] = archive->GetBool(Format("vro.flippedDensityBit.%d", i));
            }
        }
        else
        {
            if(lightmapTexturePath.Exists())
            {
                GenerateDensityMapFromTransparencyMask(lightmapTexturePath, densityBits);
            }
        }
        
        if(densityBits.size() == 0)
        {
            densityBits.resize(DENSITY_MAP_SIZE * DENSITY_MAP_SIZE);
            uint32 bitCount = static_cast<uint32>(densityBits.size());
            for(size_t bitIndex = 0; bitIndex < bitCount; ++bitIndex)
            {
                densityBits[bitIndex] = true;
            }
        }

        SetDensityMap(densityBits);
        
        if(archive->IsKeyExists("vro.layersAnimationDrug"))
        {
            SetLayerAnimationDragCoefficient(archive->GetVector4("vro.layersAnimationDrug"));
        }
    }
    
    AddFlag(RenderObject::ALWAYS_CLIPPING_VISIBLE);
    AddFlag(RenderObject::CUSTOM_PREPARE_TO_RENDER);
}

bool VegetationRenderObject::IsDataLoadNeeded()
{
    bool shouldLoadData = IsHardwareCapableToRenderVegetation();
    
    FastName currentQuality = QualitySettingsSystem::Instance()->GetCurMaterialQuality(VegetationPropertyNames::VEGETATION_QUALITY_GROUP_NAME);
    bool qualityAllowsVegetation = (VegetationPropertyNames::VEGETATION_QUALITY_NAME_HIGH == currentQuality);
    
    shouldLoadData = shouldLoadData && qualityAllowsVegetation;
    
    Renderer::GetOptions()->SetOption(RenderOptions::VEGETATION_DRAW, shouldLoadData);
    
#if defined(__DAVAENGINE_MACOS__)  || defined(__DAVAENGINE_WINDOWS__)
    shouldLoadData = true;
#endif

    return shouldLoadData;
}
    
void VegetationRenderObject::PrepareToRender(Camera * camera)
{
    activeRenderBatchArray.clear();
    if(!ReadyToRender())
    {
        return;
    }

    size_t visibleCellCount = visibleCells.size();
    size_t renderBatchCount = GetRenderBatchCount();
    while (renderBatchCount < visibleCellCount)
    {
        AddRenderBatch(ScopedPtr<RenderBatch>(CreateRenderBatch()));
        ++renderBatchCount;
    }
    

    Vector<Vector<Vector<VegetationSortedBufferItem> > >& indexRenderDataObject = renderData->GetIndexBuffers();
    
    Vector3 posScale(0.0f, 0.0f, 0.0f);
    Vector2 switchLodScale;
    Vector4 vegetationAnimationOffset[2];
    
    Vector3 cameraDirection = camera->GetDirection();
    cameraDirection.Normalize();

    for(size_t cellIndex = 0; cellIndex < visibleCellCount; ++cellIndex)
    {
        AbstractQuadTreeNode<VegetationSpatialData>* treeNode = visibleCells[cellIndex];
        
        RenderBatch* rb = GetRenderBatch(static_cast<uint32>(cellIndex));
        NMaterial* mat = rb->GetMaterial();
        
        uint32 resolutionIndex = MapCellSquareToResolutionIndex(treeNode->data.width * treeNode->data.height);
        
        Vector<Vector<VegetationSortedBufferItem> >& rdoVector = indexRenderDataObject[resolutionIndex];
        
        uint32 indexBufferIndex = treeNode->data.rdoIndex;
        Vector<VegetationSortedBufferItem>& indexBufferVector = rdoVector[indexBufferIndex];
        
        DVASSERT(indexBufferIndex < rdoVector.size());
        
        size_t directionIndex = SelectDirectionIndex(cameraDirection, indexBufferVector);

        VegetationSortedBufferItem & bufferItem = indexBufferVector[directionIndex];

        rb->startIndex = bufferItem.startIndex;
        rb->indexCount = bufferItem.indexCount;
        
        activeRenderBatchArray.push_back(rb);

        float32 distanceScale = 1.0f;
        
        if(treeNode->data.cameraDistance > visibleClippingDistances.y)
        {
            distanceScale = Clamp(1.0f - ((treeNode->data.cameraDistance - visibleClippingDistances.y) / (visibleClippingDistances.x - visibleClippingDistances.y)), 0.0f, 1.0f);
        }
        
        posScale.x = treeNode->data.bbox.min.x - unitWorldSize[resolutionIndex].x * (indexBufferIndex % RESOLUTION_TILES_PER_ROW[resolutionIndex]);
        posScale.y = treeNode->data.bbox.min.y - unitWorldSize[resolutionIndex].y * (indexBufferIndex / RESOLUTION_TILES_PER_ROW[resolutionIndex]);
        posScale.z = distanceScale;
        
        switchLodScale.x = (float32)resolutionIndex;
        switchLodScale.y = Clamp(1.0f - (treeNode->data.cameraDistance / resolutionRanges[resolutionIndex].y), 0.0f, 1.0f);
        
        for(uint32 i = 0; i < 4; ++i)
        {
            Vector2 animationOffset = treeNode->data.animationOffset[i] * layersAnimationAmplitude.data[i];
            vegetationAnimationOffset[0].data[i] = animationOffset.x;
            vegetationAnimationOffset[1].data[i] = animationOffset.y;
        }

        mat->SetPropertyValue(VegetationPropertyNames::UNIFORM_SWITCH_LOD_SCALE, switchLodScale.data);
        mat->SetPropertyValue(VegetationPropertyNames::UNIFORM_TILEPOS, posScale.data);
        mat->SetPropertyValue(VegetationPropertyNames::UNIFORM_VEGWAVEOFFSET_X, vegetationAnimationOffset[0].data);
        mat->SetPropertyValue(VegetationPropertyNames::UNIFORM_VEGWAVEOFFSET_Y, vegetationAnimationOffset[1].data);
#ifdef VEGETATION_DRAW_LOD_COLOR
        mat->SetPropertyValue(VegetationPropertyNames::UNIFORM_LOD_COLOR, RESOLUTION_COLOR[resolutionIndex].color);
#endif

    }
}
        
Vector2 VegetationRenderObject::GetVegetationUnitWorldSize(float32 resolution) const
{
    return Vector2((worldSize.x / DENSITY_MAP_SIZE) * resolution,
                   (worldSize.y / DENSITY_MAP_SIZE) * resolution);
}
    
void VegetationRenderObject::BuildSpatialStructure()
{
    DVASSERT(heightmap);
    
    uint32 mapSize = DENSITY_MAP_SIZE;
    uint32 heightmapSize = heightmap->Size();
    
    halfWidth = mapSize / 2;
    halfHeight = mapSize / 2;
    
    heightmapToVegetationMapScale = Vector2((1.0f * heightmapSize) / mapSize,
                                            (1.0f * heightmapSize) / mapSize);
    
    uint32 treeDepth = FastLog2(mapSize);
    
    quadTree.Init(treeDepth);
    AbstractQuadTreeNode<VegetationSpatialData>* node = quadTree.GetRoot();
    
    uint32 halfSize = mapSize >> 1;
    BuildSpatialQuad(node, NULL, -1 * halfSize, -1 * halfSize, mapSize, mapSize, node->data.bbox);
}
    
void VegetationRenderObject::BuildSpatialQuad(AbstractQuadTreeNode<VegetationSpatialData>* node, AbstractQuadTreeNode<VegetationSpatialData>* firstRenderableParent,
                                                int16 x, int16 y, uint16 width, uint16 height, AABBox3& parentBox)
{
    DVASSERT(node);
    
    node->data.x = x;
    node->data.y = y;
    
    if(width <= RESOLUTION_SCALE[RESOLUTION_SCALE.size() - 1])
    {
        node->data.width = width;
        node->data.height = height;
        node->data.isVisible = !IsNodeEmpty(node, densityMap);
        
        if(width == RESOLUTION_SCALE[RESOLUTION_SCALE.size() - 1])
        {
            firstRenderableParent = node;
            node->data.rdoIndex = 0;
        }
        else
        {
            int16 offsetX = abs(node->data.x - firstRenderableParent->data.x) / width;
            int16 offsetY = abs(node->data.y - firstRenderableParent->data.y) / height;
            
            node->data.rdoIndex = offsetX + (offsetY * (firstRenderableParent->data.width / width));
        }
    }
    else
    {
        node->data.width = -1;
        node->data.height = -1;
        node->data.rdoIndex = -1;
    }
    
    if(node->IsTerminalLeaf())
    {
        int32 mapX = x + halfWidth;
        int32 mapY = y + halfHeight;

        float32 heightmapHeight = SampleHeight(mapX, mapY);
        node->data.bbox.AddPoint(Vector3(x * unitWorldSize[0].x, y * unitWorldSize[0].y, (heightmapHeight - 0.5f)));
        node->data.bbox.AddPoint(Vector3((x + width) * unitWorldSize[0].x, (y + height) * unitWorldSize[0].y, (heightmapHeight + 0.5f)));
        
        parentBox.AddPoint(node->data.bbox.min);
        parentBox.AddPoint(node->data.bbox.max);
    }
    else
    {
        int16 cellHalfWidth = width >> 1;
        int16 cellHalfHeight = height >> 1;
        
        BuildSpatialQuad(node->children[0], firstRenderableParent, x, y, cellHalfWidth, cellHalfHeight, node->data.bbox);
        BuildSpatialQuad(node->children[1], firstRenderableParent, x + cellHalfWidth, y, cellHalfWidth, cellHalfHeight, node->data.bbox);
        BuildSpatialQuad(node->children[2], firstRenderableParent, x, y + cellHalfHeight, cellHalfWidth, cellHalfHeight, node->data.bbox);
        BuildSpatialQuad(node->children[3], firstRenderableParent, x + cellHalfWidth, y + cellHalfHeight, cellHalfWidth, cellHalfHeight, node->data.bbox);
        
        parentBox.AddPoint(node->data.bbox.min);
        parentBox.AddPoint(node->data.bbox.max);
    }
}
    
Vector<AbstractQuadTreeNode<VegetationSpatialData>*> & VegetationRenderObject::BuildVisibleCellList(Camera * forCamera)
{
    Vector3 camPos = forCamera->GetPosition();
    Vector3 camDir = forCamera->GetDirection();
    camDir.z = 0.0f;
    
    camDir.Normalize();
    camPos = camPos + camDir * cameraBias;
    
    uint8 planeMask = 0x3F;
    Vector3 cameraPosXY = camPos;
    cameraPosXY.z = 0.0f;
    
    visibleCells.clear();
    
    BuildVisibleCellList(cameraPosXY, forCamera->GetFrustum(), planeMask, quadTree.GetRoot(), visibleCells, true);

    return visibleCells;
}
    
void VegetationRenderObject::BuildVisibleCellList(const Vector3& cameraPoint, Frustum* frustum, uint8 planeMask,
                                                  AbstractQuadTreeNode<VegetationSpatialData>* node, Vector<AbstractQuadTreeNode<VegetationSpatialData>*>& cellList, bool evaluateVisibility)
{
    static Array<Vector3, 4> corners;
    if(node)
    {
        Frustum::eFrustumResult result = Frustum::EFR_INSIDE;
        
        if(evaluateVisibility)
        {
            result = frustum->Classify(node->data.bbox, planeMask, node->data.clippingPlane);
        }
        
        if(Frustum::EFR_OUTSIDE != result)
        {
            bool needEvalClipping = (Frustum::EFR_INTERSECT == result);
            
            if(node->data.IsRenderable())
            {
                corners[0].x = node->data.bbox.min.x;
                corners[0].y = node->data.bbox.min.y;
                
                corners[1].x = node->data.bbox.max.x;
                corners[1].y = node->data.bbox.max.y;
                
                corners[2].x = node->data.bbox.max.x;
                corners[2].y = node->data.bbox.min.y;
                
                corners[3].x = node->data.bbox.min.x;
                corners[3].y = node->data.bbox.max.y;
                
                float32& refDistance = node->data.cameraDistance;
                
                refDistance = FLT_MAX;
                size_t cornersCount = corners.size();
                for(uint32 cornerIndex = 0; cornerIndex < cornersCount; ++cornerIndex)
                {
                    float32 cornerDistance = (cameraPoint - corners[cornerIndex]).SquareLength();
                    if(cornerDistance < refDistance)
                    {
                        refDistance = cornerDistance;
                    }
                }
                
                uint32 resolutionId = MapToResolution(refDistance);
                if(node->IsTerminalLeaf() || RESOLUTION_CELL_SQUARE[resolutionId] >= (uint32)node->data.GetResolutionId())
                {
                    AddVisibleCell(node, visibleClippingDistances.x, cellList);
                }
                else if(!node->IsTerminalLeaf())
                {
                    BuildVisibleCellList(cameraPoint, frustum, planeMask, node->children[0], cellList, needEvalClipping);
                    BuildVisibleCellList(cameraPoint, frustum, planeMask, node->children[1], cellList, needEvalClipping);
                    BuildVisibleCellList(cameraPoint, frustum, planeMask, node->children[2], cellList, needEvalClipping);
                    BuildVisibleCellList(cameraPoint, frustum, planeMask, node->children[3], cellList, needEvalClipping);
                }
            }
            else
            {
                
                BuildVisibleCellList(cameraPoint, frustum, planeMask, node->children[0], cellList, needEvalClipping);
                BuildVisibleCellList(cameraPoint, frustum, planeMask, node->children[1], cellList, needEvalClipping);
                BuildVisibleCellList(cameraPoint, frustum, planeMask, node->children[2], cellList, needEvalClipping);
                BuildVisibleCellList(cameraPoint, frustum, planeMask, node->children[3], cellList, needEvalClipping);
            }
        }
    }
}
        
bool VegetationRenderObject::CellByDistanceCompareFunction(const AbstractQuadTreeNode<VegetationSpatialData>* a,
                                                           const AbstractQuadTreeNode<VegetationSpatialData>*  b)
{
    return (a->data.cameraDistance > b->data.cameraDistance);
}
    
void VegetationRenderObject::InitHeightTextureFromHeightmap(Heightmap* heightMap)
{
    SafeRelease(heightmapTexture);
    
    if(IsDataLoadNeeded())
    {
        Image* originalImage = Image::CreateFromData(heightMap->Size(), heightMap->Size(), FORMAT_A16, (uint8*)heightMap->Data());
        
        int32 pow2Size = heightmap->Size();
        if(!IsPowerOf2(heightmap->Size()))
        {
            EnsurePowerOf2(pow2Size);
            
            if(pow2Size > heightmap->Size())
            {
                pow2Size = pow2Size >> 1;
            }
        }
        
        Texture* tx = NULL;
        if(pow2Size != heightmap->Size())
        {
            Image* croppedImage = Image::CopyImageRegion(originalImage, pow2Size, pow2Size);
            tx = Texture::CreateFromData(FORMAT_RGBA4444, croppedImage->GetData(), pow2Size, pow2Size, false);
            
            SafeRelease(croppedImage);
        }
        else
        {
            tx = Texture::CreateFromData(FORMAT_RGBA4444, originalImage->GetData(), pow2Size, pow2Size, false);
        }
        
        SafeRelease(originalImage);
        
        heightmapScale = Vector2((1.0f * heightmap->Size()) / pow2Size,
                                 (1.0f * heightmap->Size()) / pow2Size);
        
        tx->SetWrapMode(rhi::TEXADDR_CLAMP, rhi::TEXADDR_CLAMP);
        tx->SetMinMagFilter(rhi::TEXFILTER_NEAREST, rhi::TEXFILTER_NEAREST, rhi::TEXMIPFILTER_NONE);

        heightmapTexture = SafeRetain(tx);
        
        if(vegetationGeometry != NULL)
        {
            KeyedArchive* props = new KeyedArchive();
            props->SetUInt64(NMaterialTextureName::TEXTURE_HEIGHTMAP.c_str(), (uint64)heightmapTexture);
            props->SetVector2(VegetationPropertyNames::UNIFORM_HEIGHTMAP_SCALE.c_str(), heightmapScale);
            
            vegetationGeometry->OnVegetationPropertiesChanged(renderData->GetMaterial(), props);
            
            SafeRelease(props);
        }
        
        SafeRelease(tx);
    }
}
    
float32 VegetationRenderObject::SampleHeight(int16 x, int16 y)
{
    uint32 hX = (uint32)(heightmapToVegetationMapScale.x * x);
    uint32 hY = (uint32)(heightmapToVegetationMapScale.y * y);
    
    uint16 left = (hX > 0) ? *(heightmap->Data() + ((hY * heightmap->Size()) + hX - 1)) : *(heightmap->Data() + ((hY * heightmap->Size()) + hX));
    uint16 right = (hX < halfWidth) ? *(heightmap->Data() + ((hY * heightmap->Size()) + hX + 1)) : *(heightmap->Data() + ((hY * heightmap->Size()) + hX));
    uint16 top = (hY > 0) ? *(heightmap->Data() + (((hY - 1) * heightmap->Size()) + hX)) : *(heightmap->Data() + ((hY * heightmap->Size()) + hX));
    uint16 down = (hY < halfHeight) ? *(heightmap->Data() + (((hY + 1) * heightmap->Size()) + hX)) : *(heightmap->Data() + ((hY * heightmap->Size()) + hX));
    uint16 center = *(heightmap->Data() + ((hY * heightmap->Size()) + hX));
    
    uint16 heightmapValue = (left + right + top + down + center) / 5;
    
    float32 height = ((float32)heightmapValue / (float32)Heightmap::MAX_VALUE) * worldSize.z;
    
    return height;
}

bool VegetationRenderObject::IsHardwareCapableToRenderVegetation()
{
    const rhi::RenderDeviceCaps& deviceCaps = rhi::DeviceCaps();
    bool result = deviceCaps.isVertexTextureUnitsSupported && deviceCaps.is32BitIndicesSupported;
    
    return result;
}

bool VegetationRenderObject::IsValidGeometryData() const
{
     return (worldSize.Length() > 0 &&
             heightmap != nullptr &&
             heightmap->Size() > 0 && 
             densityMap.size() > 0 &&
             customGeometryData);
}
    
bool VegetationRenderObject::IsValidSpatialData() const
{
    return (worldSize.Length() > 0 &&
            heightmap != nullptr &&
            heightmap->Size() > 0 &&
            densityMap.size() > 0);
}

void VegetationRenderObject::UpdateVegetationSetup()
{
    if(densityMap.size() > 0)
    {
        size_t resolutionScaleSize = RESOLUTION_SCALE.size();
        for(size_t i = 0; i < resolutionScaleSize; ++i)
        {
            unitWorldSize[i] = GetVegetationUnitWorldSize(RESOLUTION_SCALE[i]);
        }
    }
    
    if(IsValidGeometryData())
    {
        CreateRenderData();
        renderData->GetMaterial()->PreBuildMaterial(PASS_FORWARD);
    }
    
    if(IsValidSpatialData())
    {
        BuildSpatialStructure();
    }
    
    ClearRenderBatches();
}

void VegetationRenderObject::ResetVisibilityDistance()
{
    visibleClippingDistances.x = MAX_VISIBLE_CLIPPING_DISTANCE;
    visibleClippingDistances.y = MAX_VISIBLE_SCALING_DISTANCE;
}

void VegetationRenderObject::ResetLodRanges()
{
   lodRanges = LOD_RANGES_SCALE;
 
    if(IsValidSpatialData())
    {
        InitLodRanges();
    }
}

void VegetationRenderObject::InitLodRanges()
{
    Vector2 smallestUnitSize = GetVegetationUnitWorldSize(RESOLUTION_SCALE[0]);
    
    resolutionRanges[0].x = lodRanges.x * smallestUnitSize.x;
    resolutionRanges[0].y = lodRanges.y * smallestUnitSize.x;

    resolutionRanges[1].x = lodRanges.y * smallestUnitSize.x;
    resolutionRanges[1].y = lodRanges.z * smallestUnitSize.x;

    resolutionRanges[2].x = lodRanges.z * smallestUnitSize.x;
    resolutionRanges[2].y = visibleClippingDistances.x;//RESOLUTION_RANGES[2].x * 1000.0f;

    size_t resolutionCount = resolutionRanges.size();
    for(size_t i = 0; i < resolutionCount; ++i)
    {
        resolutionRanges[i].x *= resolutionRanges[i].x;
        resolutionRanges[i].y *= resolutionRanges[i].y;
    }
}

void VegetationRenderObject::GetDataNodes(Set<DataNode*> & dataNodes)
{
    if(customGeometryData)
    {
        size_t layerCount = customGeometryData->GetLayerCount();
        for(uint32 i = 0; i < layerCount; ++i)
        {
            dataNodes.insert(customGeometryData->GetMaterial(i));
        }
    }
}



void VegetationRenderObject::CreateRenderData()
{
    InitLodRanges();
    
    SafeDelete(vegetationGeometry);
    vegetationGeometry = new VegetationGeometry(layerParams,
                                                MAX_DENSITY_LEVELS,
                                                GetVegetationUnitWorldSize(RESOLUTION_SCALE[0]),
                                                customGeometryPath,
                                                RESOLUTION_CELL_SQUARE.data(),
                                                RESOLUTION_CELL_SQUARE.size(),
                                                RESOLUTION_SCALE.data(),
                                                RESOLUTION_SCALE.size(),
                                                RESOLUTION_TILES_PER_ROW.data(),
                                                RESOLUTION_TILES_PER_ROW.size(),
                                                RESOLUTION_CLUSTER_STRIDE.data(),
                                                RESOLUTION_CLUSTER_STRIDE.size(),
                                                worldSize,
                                                customGeometryData);

    if (renderData)
    {
        delete renderData;
        rhi::DeleteVertexBuffer(vertexBuffer);
        rhi::DeleteIndexBuffer(indexBuffer);
    }

    renderData = new VegetationRenderData();
    vegetationGeometry->Build(renderData);

    const Vector<VegetationVertex>& vertexData = renderData->GetVertices();
    const Vector<VegetationIndex>& indexData = renderData->GetIndices();

    vertexCount = (uint32)vertexData.size();
    indexCount = (uint32)indexData.size();

    uint32 vertexBufferSize = vertexData.size() * sizeof(VegetationVertex);
    vertexBuffer = rhi::CreateVertexBuffer(vertexBufferSize);
    rhi::UpdateVertexBuffer(vertexBuffer, &vertexData.front(), 0, vertexBufferSize);

    uint32 indexBufferSize = indexData.size() * sizeof(VegetationIndex);
    rhi::IndexBuffer::Descriptor indexDesc;
    indexDesc.size = indexBufferSize;
    indexDesc.indexSize = rhi::INDEX_SIZE_32BIT;
    indexBuffer = rhi::CreateIndexBuffer(indexDesc);
    rhi::UpdateIndexBuffer(indexBuffer, &indexData.front(), 0, indexBufferSize);

#if defined(__DAVAENGINE_IPHONE__)
    renderData->ReleaseRenderData(); //release vertex and index buffers data
#endif

    KeyedArchive* props = new KeyedArchive();
    props->SetUInt64(NMaterialTextureName::TEXTURE_HEIGHTMAP.c_str(), (uint64)heightmapTexture);
    props->SetVector2(VegetationPropertyNames::UNIFORM_HEIGHTMAP_SCALE.c_str(), heightmapScale);
    props->SetVector3(VegetationPropertyNames::UNIFORM_PERTURBATION_FORCE.c_str(), perturbationForce);
    props->SetFloat(VegetationPropertyNames::UNIFORM_PERTURBATION_FORCE_DISTANCE.c_str(), maxPerturbationDistance);
    props->SetVector3(VegetationPropertyNames::UNIFORM_PERTURBATION_POINT.c_str(), perturbationPoint);
    props->SetString(VegetationPropertyNames::UNIFORM_SAMPLER_VEGETATIONMAP.c_str(), lightmapTexturePath.GetStringValue());
    
    vegetationGeometry->OnVegetationPropertiesChanged(renderData->GetMaterial(), props);
    
    SafeRelease(props);

    rhi::VertexLayout vertexLayout;
    vertexLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    //vertexLayout.AddElement(rhi::VS_NORMAL, 0, rhi::VDT_FLOAT, 3); uncomment, when normals will be used for vertex lit implementation
    vertexLayout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2);
    vertexLayout.AddElement(rhi::VS_TEXCOORD, 1, rhi::VDT_FLOAT, 3);
    vertexLayout.AddElement(rhi::VS_TEXCOORD, 2, rhi::VDT_FLOAT, 3);
    vertexLayoutUID = rhi::VertexLayout::UniqueId(vertexLayout);

    ClearRenderBatches();
}

void VegetationRenderObject::RestoreRenderData()
{
#if defined(__DAVAENGINE_IPHONE__)
    DVASSERT_MSG(false, "Should not even try to restore on iphone - render data is released");
#endif
    if (!renderData)
        return;
    
    if (rhi::NeedRestoreVertexBuffer(vertexBuffer))
    {
        const Vector<VegetationVertex>& vertexData = renderData->GetVertices();
        uint32 vertexBufferSize = vertexData.size() * sizeof(VegetationVertex);
        rhi::UpdateVertexBuffer(vertexBuffer, &vertexData.front(), 0, vertexBufferSize);
    }
    if (rhi::NeedRestoreIndexBuffer(indexBuffer))
    {
        const Vector<VegetationIndex>& indexData = renderData->GetIndices();
        uint32 indexBufferSize = indexData.size() * sizeof(VegetationIndex);
        rhi::UpdateIndexBuffer(indexBuffer, &indexData.front(), 0, indexBufferSize);
    }
    if (heightmap && heightmapTexture) //RHI_COMPLETE later change it to normal restoration and change init heightmap texture to normal logic

    {
        Image* originalImage = Image::CreateFromData(heightmap->Size(), heightmap->Size(), FORMAT_A16, (uint8*)heightmap->Data());
        int32 pow2Size = heightmap->Size();
        if (!IsPowerOf2(heightmap->Size()))
        {
            EnsurePowerOf2(pow2Size);

            if (pow2Size > heightmap->Size())
            {
                pow2Size = pow2Size >> 1;
            }
        }        
        if (pow2Size != heightmap->Size())
        {
            Image* croppedImage = Image::CopyImageRegion(originalImage, pow2Size, pow2Size);
            heightmapTexture->TexImage(0, pow2Size, pow2Size, croppedImage->GetData(), croppedImage->dataSize, 0);
            SafeRelease(croppedImage);
        }
        else
        {
            heightmapTexture->TexImage(0, pow2Size, pow2Size, originalImage->GetData(), originalImage->dataSize, 0);
        }
    }
}

bool VegetationRenderObject::ReadyToRender()
{
    bool renderFlag = IsHardwareCapableToRenderVegetation() && Renderer::GetOptions()->IsOptionEnabled(RenderOptions::VEGETATION_DRAW);
    
#if defined(__DAVAENGINE_MACOS__)  || defined(__DAVAENGINE_WINDOWS__)
    //VI: case when vegetation was turned off and then qualit changed from low t high is not a real-world scenario
    //VI: real-world scenario is in resource editor when quality has been changed.
    FastName currentQuality = QualitySettingsSystem::Instance()->GetCurMaterialQuality(VegetationPropertyNames::VEGETATION_QUALITY_GROUP_NAME);
    bool qualityAllowsVegetation = (VegetationPropertyNames::VEGETATION_QUALITY_NAME_HIGH == currentQuality);
    
    renderFlag = (renderFlag && qualityAllowsVegetation);
#endif
    
    return renderFlag && vegetationVisible && renderData;
}

size_t VegetationRenderObject::SelectDirectionIndex(const Vector3& cameraDirection, Vector<VegetationSortedBufferItem>& buffers)
{
    size_t index = 0;
    float32 currentCosA = 0.0f;
    size_t directionCount = buffers.size();
    for(size_t i = 0; i < directionCount; ++i)
    {
        VegetationSortedBufferItem& item = buffers[i];
        //cos (angle) = dotAB / (length A * lengthB)
        //no need to calculate (length A * lengthB) since vectors are normalized
        
        if(item.sortDirection == cameraDirection)
        {
            index = i;
            break;
        }
        else
        {
            float32 cosA = cameraDirection.DotProduct(item.sortDirection);
            if(cosA > currentCosA)
            {
                index = i;
                currentCosA = cosA;
            }
        }
    }
    
    return index;
}

void VegetationRenderObject::DebugDrawVisibleNodes(RenderHelper * drawer)
{
    uint32 requestedBatchCount = static_cast<uint32>(Min(visibleCells.size(), (size_t)maxVisibleQuads));
    for(uint32 i = 0; i < requestedBatchCount; ++i)
    {
        AbstractQuadTreeNode<VegetationSpatialData>* treeNode = visibleCells[i];
        uint32 resolutionIndex = MapCellSquareToResolutionIndex(treeNode->data.width * treeNode->data.height);

        drawer->DrawAABox(treeNode->data.bbox, RESOLUTION_COLOR[resolutionIndex], RenderHelper::DRAW_WIRE_DEPTH);
    }
}

void VegetationRenderObject::ClearRenderBatches()
{
    int32 batchesToRemove = GetRenderBatchCount();
    while(batchesToRemove > 0)
    {
        RemoveRenderBatch(batchesToRemove - 1);
        batchesToRemove = GetRenderBatchCount();
    }
}

void VegetationRenderObject::SetCustomGeometryPath(const FilePath& path)
{
    if (!path.IsEmpty() && path.Exists())
    {
        VegetationGeometryDataPtr fetchedData = 
            VegetationGeometryDataReader::ReadScene(path);

        if (fetchedData)
        {
            customGeometryData = std::move(fetchedData);
            SetCustomGeometryPathInternal(path);
        }
    }
}

void VegetationRenderObject::SetCustomGeometryPathInternal(const FilePath& path)
{
    customGeometryPath = path;
    
    if(IsValidGeometryData())
    {
        CreateRenderData();
    }
}

VegetationGeometryDataPtr VegetationRenderObject::LoadCustomGeometryData(SerializationContext* context, KeyedArchive* srcArchive)
{
    uint32 layerCount = srcArchive->GetUInt32("cgsd.layerCount");
    
    Vector<NMaterial*> materials;
    Vector<Vector<Vector<Vector3> > > positions;
    Vector<Vector<Vector<Vector2> > > texCoords;
    Vector<Vector<Vector<Vector3> > > normals;
    Vector<Vector<Vector<VegetationIndex> > > indices;
    
    for(uint32 layerIndex = 0; layerIndex < layerCount; ++layerIndex)
    {
        KeyedArchive* layerArchive = srcArchive->GetArchive(Format("cgsd.layer.%d", layerIndex));
        
        uint64 materialId = layerArchive->GetUInt64("cgsd.layer.materialId");
        NMaterial* mat = (NMaterial*)context->GetDataBlock(materialId);
        
        DVASSERT(mat);
        
        materials.push_back(mat);
        
        positions.push_back(Vector<Vector<Vector3> >());
        texCoords.push_back(Vector<Vector<Vector2> >());
        normals.push_back(Vector<Vector<Vector3> >());
        indices.push_back(Vector<Vector<VegetationIndex> >());
        
        Vector<Vector<Vector3> >& layerPositions = positions[positions.size() - 1];
        Vector<Vector<Vector2> >& layerTexCoords = texCoords[texCoords.size() - 1];
        Vector<Vector<Vector3> >& layerNormals = normals[normals.size() - 1];
        Vector<Vector<VegetationIndex> >& layerIndices = indices[indices.size() - 1];
        
        uint32 lodCount = layerArchive->GetUInt32("cgsd.layer.lodCount");
        for(uint32 lodIndex = 0; lodIndex < lodCount; ++lodIndex)
        {
            layerPositions.push_back(Vector<Vector3>());
            layerTexCoords.push_back(Vector<Vector2>());
            layerNormals.push_back(Vector<Vector3>());
            layerIndices.push_back(Vector<VegetationIndex>());
            
            Vector<Vector3>& lodPositions = layerPositions[layerPositions.size() - 1];
            Vector<Vector2>& lodTexCoords = layerTexCoords[layerTexCoords.size() - 1];
            Vector<Vector3>& lodNormals = layerNormals[layerNormals.size() - 1];
            Vector<VegetationIndex>& lodIndices = layerIndices[layerIndices.size() - 1];
            
            KeyedArchive* lodArchive = layerArchive->GetArchive(Format("cgsd.lod.%d", lodIndex));
            
            uint32 posCount = lodArchive->GetUInt32("cgsd.lod.posCount");
            for(uint32 i = 0; i < posCount; ++i)
            {
                Vector3 pos = lodArchive->GetVector3(Format("cgsd.lod.pos.%d", i));
                lodPositions.push_back(pos);
            }
            
            uint32 texCount = lodArchive->GetUInt32("cgsd.lod.texCount");
            for(uint32 i = 0; i < texCount; ++i)
            {
                Vector2 texCoord = lodArchive->GetVector2(Format("cgsd.lod.tex.%d", i));
                lodTexCoords.push_back(texCoord);
            }
            
            uint32 normalCount = lodArchive->GetUInt32("cgsd.lod.normalCount");
            for(uint32 i = 0; i < normalCount; ++i)
            {
                Vector3 normal = lodArchive->GetVector3(Format("cgsd.lod.normal.%d", i));
                lodNormals.push_back(normal);
            }

            uint32 indexCount = lodArchive->GetUInt32("cgsd.lod.indexCount");
            for(uint32 i = 0; i < indexCount; ++i)
            {
                uint32 index = lodArchive->GetInt32(Format("cgsd.lod.index.%d", i));
                lodIndices.push_back(index);
            }
        }
    }

    VegetationGeometryDataPtr data(new VegetationGeometryData(materials, positions, texCoords, normals, indices));
    
    return data;
}
    
void VegetationRenderObject::SaveCustomGeometryData(SerializationContext* context, KeyedArchive* dstArchive, const VegetationGeometryDataPtr & data)
{
    uint32 layerCount = data->GetLayerCount();
    dstArchive->SetUInt32("cgsd.layerCount", layerCount);
    
    for(uint32 layerIndex = 0; layerIndex < layerCount; ++layerIndex)
    {
        uint32 lodCount = data->GetLodCount(layerIndex);
        KeyedArchive* layerArchive = new KeyedArchive();
        
        layerArchive->SetUInt64("cgsd.layer.materialId", data->GetMaterial(layerIndex)->GetNodeID());
        layerArchive->SetUInt32("cgsd.layer.lodCount", lodCount);
        
        for(uint32 lodIndex = 0; lodIndex < lodCount; ++lodIndex)
        {
            KeyedArchive* lodArchive = new KeyedArchive();
        
            Vector<Vector3>& positions = data->GetPositions(layerIndex, lodIndex);
            Vector<Vector2>& texCoords = data->GetTextureCoords(layerIndex, lodIndex);
            Vector<Vector3>& normals = data->GetNormals(layerIndex, lodIndex);
            Vector<VegetationIndex>& indices = data->GetIndices(layerIndex, lodIndex);
            
            uint32 posCount = static_cast<uint32>(positions.size());
            lodArchive->SetUInt32("cgsd.lod.posCount", posCount);
            for(uint32 i = 0; i < posCount; ++i)
            {
                lodArchive->SetVector3(Format("cgsd.lod.pos.%d", i), positions[i]);
            }
            
            uint32 texCount = static_cast<uint32>(texCoords.size());
            lodArchive->SetUInt32("cgsd.lod.texCount", texCount);
            for(uint32 i = 0; i < texCount; ++i)
            {
                lodArchive->SetVector2(Format("cgsd.lod.tex.%d", i), texCoords[i]);
            }

            uint32 normalCount = static_cast<uint32>(normals.size());
            lodArchive->SetUInt32("cgsd.lod.normalCount", normalCount);
            for(uint32 i = 0; i < normalCount; ++i)
            {
                lodArchive->SetVector3(Format("cgsd.lod.normal.%d", i), normals[i]);
            }

            uint32 indexCount = static_cast<uint32>(indices.size());
            lodArchive->SetUInt32("cgsd.lod.indexCount", indexCount);
            for(uint32 i = 0; i < indexCount; ++i)
            {
                lodArchive->SetInt32(Format("cgsd.lod.index.%d", i), indices[i]);
            }
            
            layerArchive->SetArchive(Format("cgsd.lod.%d", lodIndex), lodArchive);
            
            SafeRelease(lodArchive);
        }
        
        dstArchive->SetArchive(Format("cgsd.layer.%d", layerIndex), layerArchive);
        
        SafeRelease(layerArchive);
    }
}

void VegetationRenderObject::RecalcBoundingBox()
{
}

void VegetationRenderObject::CollectMetrics(VegetationMetrics& metrics)
{
    metrics.renderBatchCount = 0;
    metrics.totalQuadTreeLeafCount = 0;
    
    metrics.quadTreeLeafCountPerLOD.clear();
    
    metrics.instanceCountPerLOD.clear();
    metrics.polyCountPerLOD.clear();
    metrics.instanceCountPerLayer.clear();
    metrics.polyCountPerLayer.clear();
    
    metrics.visibleInstanceCountPerLayer.clear();
    metrics.visibleInstanceCountPerLOD.clear();
    metrics.visiblePolyCountPerLayer.clear();
    metrics.visiblePolyCountPerLOD.clear();
    
    metrics.polyCountPerLayerPerLod.clear();
    
    metrics.isValid = false;
    
    if (renderData)
    {
        metrics.isValid = true;
        
        size_t visibleCellCount = visibleCells.size();
        
        metrics.renderBatchCount = static_cast<uint32>(visibleCells.size());
        metrics.totalQuadTreeLeafCount = static_cast<uint32>(visibleCellCount);
        
        size_t maxLodCount = RESOLUTION_CELL_SQUARE.size();
        metrics.quadTreeLeafCountPerLOD.resize(maxLodCount, 0);
        metrics.instanceCountPerLOD.resize(maxLodCount, 0);
        metrics.polyCountPerLOD.resize(maxLodCount, 0);
        metrics.visibleInstanceCountPerLOD.resize(maxLodCount, 0);
        metrics.visiblePolyCountPerLOD.resize(maxLodCount, 0);
        
        uint32 maxLayerCount = (uint32)renderData->instanceCount.size();
        
        metrics.visibleInstanceCountPerLayer.resize(maxLayerCount, 0);
        metrics.visiblePolyCountPerLayer.resize(maxLayerCount, 0);
        metrics.instanceCountPerLayer.resize(maxLayerCount, 0);
        metrics.polyCountPerLayer.resize(maxLayerCount, 0);
        
        for(size_t i = 0; i < visibleCellCount; ++i)
        {
            AbstractQuadTreeNode<VegetationSpatialData>* spatialData = visibleCells[i];
            uint32 lodIndex = MapCellSquareToResolutionIndex(spatialData->data.width * spatialData->data.height);
            
            metrics.quadTreeLeafCountPerLOD[lodIndex] += 1;
        }

        size_t layerCount = renderData->instanceCount.size();

        if (metrics.polyCountPerLayerPerLod.size() < layerCount)
        {
            metrics.polyCountPerLayerPerLod.resize(layerCount);
        }

        for (size_t layerIndex = 0; layerIndex < layerCount; ++layerIndex)
        {
            if (metrics.polyCountPerLayerPerLod[layerIndex].size() < renderData->polyCountPerInstance[layerIndex].size())
            {
                metrics.polyCountPerLayerPerLod[layerIndex].resize(renderData->polyCountPerInstance[layerIndex].size());
            }

            for (size_t lodIndex = 0; lodIndex < maxLodCount; ++lodIndex)
            {
                metrics.instanceCountPerLOD[lodIndex] += renderData->instanceCount[layerIndex][lodIndex];
                metrics.polyCountPerLOD[lodIndex] += (renderData->polyCountPerInstance[layerIndex][lodIndex] * renderData->instanceCount[layerIndex][lodIndex]);

                metrics.instanceCountPerLayer[layerIndex] += renderData->instanceCount[layerIndex][lodIndex];
                metrics.polyCountPerLayer[layerIndex] += (renderData->polyCountPerInstance[layerIndex][lodIndex] * renderData->instanceCount[layerIndex][lodIndex]);

                metrics.polyCountPerLayerPerLod[layerIndex][lodIndex] += renderData->polyCountPerInstance[layerIndex][lodIndex];
            }
        }

        for (size_t i = 0; i < visibleCellCount; ++i)
        {
            AbstractQuadTreeNode<VegetationSpatialData>* spatialData = visibleCells[i];
            uint32 lodIndex = MapCellSquareToResolutionIndex(spatialData->data.width * spatialData->data.height);

            for (size_t layerIndex = 0; layerIndex < layerCount; ++layerIndex)
            {
                metrics.visibleInstanceCountPerLOD[lodIndex] += renderData->instanceCount[layerIndex][lodIndex];
                metrics.visiblePolyCountPerLOD[lodIndex] += (renderData->polyCountPerInstance[layerIndex][lodIndex] * renderData->instanceCount[layerIndex][lodIndex]);

                metrics.visibleInstanceCountPerLayer[layerIndex] += renderData->instanceCount[layerIndex][lodIndex];
                metrics.visiblePolyCountPerLayer[layerIndex] += (renderData->polyCountPerInstance[layerIndex][lodIndex] * renderData->instanceCount[layerIndex][lodIndex]);
            }
        }
    }
}

void VegetationRenderObject::GenerateDensityMapFromTransparencyMask(FilePath lightmapPath, Vector<bool>& densityMapBits)
{
    lightmapPath.ReplaceExtension(".png");
    
    if(lightmapPath.Exists())
    {
        Image* lightmapImage = LoadSingleImage(lightmapPath);
        
        if(lightmapImage != NULL)
        {
            uint32 ratio = lightmapImage->width / DENSITY_MAP_SIZE;
            
            DVASSERT(lightmapImage->GetPixelFormat() == FORMAT_RGBA8888);
            DVASSERT(ratio > 0);
            
            if(ratio > 0 && lightmapImage->GetPixelFormat() == FORMAT_RGBA8888)
            {
                densityMapBits.resize(DENSITY_MAP_SIZE * DENSITY_MAP_SIZE);
                uint32 stride = sizeof(uint32);
                for(uint32 y = 0; y < DENSITY_MAP_SIZE; ++y)
                {
                    for(uint32 x = 0; x < DENSITY_MAP_SIZE; ++x)
                    {
                        //VI: flip Y in order to match landscape and vegetation light mask
                        uint32 flippedY = DENSITY_MAP_SIZE - y - 1;

                        float32 meanAlpha = GetMeanAlpha(x, flippedY, ratio, stride, lightmapImage);
                        
                        uint32 bitIndex = x + y * DENSITY_MAP_SIZE;
                        densityMapBits[bitIndex] = (meanAlpha > DENSITY_THRESHOLD);
                    }
                }
                
            }
        }
        
        SafeRelease(lightmapImage);
    }
    
    /*Image* outputImage = Image::Create(DENSITY_MAP_SIZE, DENSITY_MAP_SIZE, FORMAT_RGBA8888);
    
    for(size_t i = 0; i < densityMapBits.size(); ++i)
    {
        if(false == densityMapBits[i])
        {
            outputImage->data[i * 4 + 0] = 0xFF;
            outputImage->data[i * 4 + 1] = 0;
            outputImage->data[i * 4 + 2] = 0;
            outputImage->data[i * 4 + 3] = 0xFF;
        }
        else
        {
            outputImage->data[i * 4 + 0] = 0xFF;
            outputImage->data[i * 4 + 1] = 0xFF;
            outputImage->data[i * 4 + 2] = 0xFF;
            outputImage->data[i * 4 + 3] = 0xFF;
        }
    }
    
    lightmapPath.ReplaceFilename("density_debug_output.png");
    outputImage->Save(lightmapPath);
    
    SafeRelease(outputImage);*/
}

Image* VegetationRenderObject::LoadSingleImage(const FilePath& path) const
{
    Vector<Image*> images;
    
    ImageSystem::Instance()->Load(path, images);
    
    Image* image = NULL;
    if(images.size() > 0)
    {
        image = SafeRetain(images[0]);
        size_t imageCount = images.size();
        for(size_t i = 0; i < imageCount; ++i)
        {
            SafeRelease(images[i]);
        }
    }
    
    return image;
}

float32 VegetationRenderObject::GetMeanAlpha(uint32 x, uint32 y, uint32 ratio, uint32 stride, Image* src) const
{
    uint32 actualStartX = x * ratio;
    uint32 actualStartY = y * ratio;
    uint32 actualEndX = actualStartX + ratio;
    uint32 actualEndY = actualStartY + ratio;
    
    float32 medianAlpha = 0.0f;
    uint32 fragmentCount = 0;
    for(uint32 yy = actualStartY; yy < actualEndY; ++yy)
    {
        for(uint32 xx = actualStartX; xx < actualEndX; ++xx)
        {
            uint32 fragmentIndex = xx + yy * src->width;
            uint32 fragmentOffset = fragmentIndex * stride;
            
            uint8* fragmentData = src->GetData() + fragmentOffset;
            
            medianAlpha += (((float32)fragmentData[3]) / 255.0f);
            fragmentCount++;
        }
    }
    
    return (medianAlpha / fragmentCount);
}

void VegetationRenderObject::SetDensityMap(const Vector<bool>& densityBits)
{
    size_t densityBitCount = densityBits.size();
    densityMap.resize(densityBitCount);
    
    for(size_t i = 0; i < densityBitCount; ++i)
    {
        densityMap[i] = densityBits[i];
    }
    
    UpdateVegetationSetup();
}

uint32 VegetationRenderObject::MapCellSquareToResolutionIndex(uint32 cellSquare)
{
    uint32 index = 0;
    size_t resolutionCount = RESOLUTION_CELL_SQUARE.size();
    for(uint32 i = 0; i < resolutionCount; ++i)
    {
        if(cellSquare == RESOLUTION_CELL_SQUARE[i])
        {
            index = i;
            break;
        }
    }
    
    return index;
}
    
    
};