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


#include "Debug/Stats.h"
#include "Platform/SystemTimer.h"
#include "FileSystem/FileSystem.h"
#include "Utils/StringFormat.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneFileV2.h"
#include "Scene3D/Systems/FoliageSystem.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/Heightmap.h"
#include "Render/Highlevel/RenderPassNames.h"
#include "Render/Image/Image.h"
#include "Render/Image/ImageSystem.h"
#include "Render/Material/NMaterial.h"
#include "Render/Material/NMaterialNames.h"
#include "Render/RenderHelper.h"
#include "Render/Texture.h"
#include "Render/Renderer.h"
#include "Render/Shader.h"
#include "Render/ShaderCache.h"
#include "Render/TextureDescriptor.h"
#include "Render/DynamicBufferAllocator.h"
#include "Render/RenderCallbacks.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"

namespace DAVA
{
const FastName Landscape::PARAM_TEXTURE_TILING("textureTiling");
const FastName Landscape::PARAM_TILE_COLOR0("tileColor0");
const FastName Landscape::PARAM_TILE_COLOR1("tileColor1");
const FastName Landscape::PARAM_TILE_COLOR2("tileColor2");
const FastName Landscape::PARAM_TILE_COLOR3("tileColor3");

const FastName Landscape::TEXTURE_COLOR("colorTexture");
const FastName Landscape::TEXTURE_TILE("tileTexture0");
const FastName Landscape::TEXTURE_TILEMASK("tileMask");
const FastName Landscape::TEXTURE_SPECULAR("specularMap");

const FastName Landscape::LANDSCAPE_QUALITY_NAME("Landscape");
const FastName Landscape::LANDSCAPE_QUALITY_VALUE_HIGH("HIGH");

const uint32 LANDSCAPE_BATCHES_POOL_SIZE = 32;

Landscape::Landscape()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    type = TYPE_LANDSCAPE;

    heightmap = new Heightmap;

    solidAngleError = (26.5f * PI / 180.0f);
    geometryAngleError = (1.0f * PI / 180.0f);
    absHeightError = 3.0f;

    zoomSolidAngleError = solidAngleError;
    zoomGeometryAngleError = (0.1f * PI / 180.0f);
    zoomAbsHeightError = 0.5f;

    zoomFov = 6.5f;
    normalFov = 70.0f;

    AddFlag(RenderObject::CUSTOM_PREPARE_TO_RENDER);

    RenderCallbacks::RegisterResourceRestoreCallback(MakeFunction(this, &Landscape::RestoreGeometry));
}

Landscape::~Landscape()
{
    ReleaseGeometryData();

    SafeRelease(heightmap);

    SafeRelease(landscapeMaterial);
    RenderCallbacks::UnRegisterResourceRestoreCallback(MakeFunction(this, &Landscape::RestoreGeometry));
}

int16 Landscape::AllocateQuadVertexBuffer(uint32 quadX, uint32 quadY, uint32 quadSize)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    uint32 verticesCount = (quadSize + 1) * (quadSize + 1);
    uint32 vertexSize = sizeof(LandscapeVertex);
    if (!isRequireTangentBasis)
    {
        vertexSize -= sizeof(Vector3); // (LandscapeVertex::normal);
        vertexSize -= sizeof(Vector3); // (LandscapeVertex::tangent);
    }

    uint8* landscapeVertices = new uint8[verticesCount * vertexSize];
    uint32 index = 0;
    for (uint32 y = quadY; y < quadY + quadSize + 1; ++y)
    {
        for (uint32 x = quadX; x < quadX + quadSize + 1; ++x)
        {
            LandscapeVertex* vertex = reinterpret_cast<LandscapeVertex*>(&landscapeVertices[index * vertexSize]);
            vertex->position = GetPoint(x, y, heightmap->Data()[y * heightmap->Size() + x]);

            Vector2 texCoord = Vector2((float32)(x) / (float32)(heightmap->Size() - 1), 1.0f - (float32)(y) / (float32)(heightmap->Size() - 1));
            vertex->texCoord = texCoord;

            if (isRequireTangentBasis)
            {
                //VI: calculate normal for the point.
                uint32 xx = 0;
                uint32 yy = 0;

                xx = (x < uint32(heightmap->Size()) - 1) ? x + 1 : x;
                Vector3 right = GetPoint(xx, y, heightmap->Data()[y * heightmap->Size() + xx]);

                xx = (x > 0) ? x - 1 : x;
                Vector3 left = GetPoint(xx, y, heightmap->Data()[y * heightmap->Size() + xx]);

                yy = (y < uint32(heightmap->Size()) - 1) ? y + 1 : y;
                Vector3 bottom = GetPoint(x, yy, heightmap->Data()[yy * heightmap->Size() + x]);
                yy = (y > 0) ? y - 1 : y;
                Vector3 top = GetPoint(x, yy, heightmap->Data()[yy * heightmap->Size() + x]);

                Vector3 position = vertex->position;
                Vector3 normal0 = (top != position && right != position) ? CrossProduct(top - position, right - position) : Vector3(0, 0, 0);
                Vector3 normal1 = (right != position && bottom != position) ? CrossProduct(right - position, bottom - position) : Vector3(0, 0, 0);
                Vector3 normal2 = (bottom != position && left != position) ? CrossProduct(bottom - position, left - position) : Vector3(0, 0, 0);
                Vector3 normal3 = (left != position && top != position) ? CrossProduct(left - position, top - position) : Vector3(0, 0, 0);

                Vector3 normalAverage = normal0 + normal1 + normal2 + normal3;
                normalAverage.Normalize();
                vertex->normal = normalAverage;
                vertex->tangent = Normalize(right - position);

                /*
                VS: Algorithm
                // # P.xy store the position for which we want to calculate the normals
                // # height() here is a function that return the height at a point in the terrain

                // read neighbor heights using an arbitrary small offset
                vec3 off = vec3(1.0, 1.0, 0.0);
                float hL = height(P.xy - off.xz);
                float hR = height(P.xy + off.xz);
                float hD = height(P.xy - off.zy);
                float hU = height(P.xy + off.zy);

                // deduce terrain normal
                N.x = hL - hR;
                N.y = hD - hU;
                N.z = 2.0;
                N = normalize(N);
                */
            }

            index++;
        }
    }

    uint32 vBufferSize = static_cast<uint32>(verticesCount * vertexSize);

    rhi::VertexBuffer::Descriptor desc;
    desc.size = vBufferSize;
    desc.initialData = landscapeVertices;
    if (updatable)
        desc.usage = rhi::USAGE_DYNAMICDRAW;
    else
        desc.usage = rhi::USAGE_STATICDRAW;

    rhi::HVertexBuffer vertexBuffer = rhi::CreateVertexBuffer(desc);
    vertexBuffers.push_back(vertexBuffer);
    
#if defined(__DAVAENGINE_IPHONE__)
    SafeDeleteArray(landscapeVertices);  
#else
    bufferRestoreData.push_back({ vertexBuffer, landscapeVertices, vBufferSize });
#endif

    return (int16)(vertexBuffers.size() - 1);
}

void Landscape::RestoreGeometry()
{
    for (auto& restoreData : bufferRestoreData)
    {
        if (rhi::NeedRestoreVertexBuffer(restoreData.buffer))
            rhi::UpdateVertexBuffer(restoreData.buffer, restoreData.data, 0, restoreData.dataSize);
    }
}

void Landscape::ReleaseGeometryData()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    for (IndexedRenderBatch& batch : renderBatchArray)
        batch.renderBatch->Release();
    renderBatchArray.clear();
    activeRenderBatchArray.clear();

    for (rhi::HVertexBuffer handle : vertexBuffers)
        rhi::DeleteVertexBuffer(handle);
    vertexBuffers.clear();

    for (auto& restoreData : bufferRestoreData)
        SafeDeleteArray(restoreData.data);
    bufferRestoreData.clear();

    indices.clear();

    SafeDeleteArray(subdivPatchArray);
    SafeDeleteArray(patchQuadArray);
}
    
void Landscape::BuildLandscapeFromHeightmapImage(const FilePath & heightmapPathname, const AABBox3 & _box)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    heightmapPath = heightmapPathname;
    BuildHeightmap();

    bbox = _box;

    BuildLandscape();

    if (foliageSystem)
    {
        foliageSystem->SyncFoliageWithLandscape();
    }
}

void Landscape::RecalcBoundingBox()
{
    //do nothing, bbox setup in BuildLandscapeFromHeightmapImage()
}

bool Landscape::BuildHeightmap()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    bool retValue = false;

    if (DAVA::TextureDescriptor::IsSourceTextureExtension(heightmapPath.GetExtension()))
    {
        Vector<Image *> imageSet;
        ImageSystem::Instance()->Load(heightmapPath, imageSet);
        if (0 != imageSet.size())
        {
            if ((imageSet[0]->GetPixelFormat() != FORMAT_A8) && (imageSet[0]->GetPixelFormat() != FORMAT_A16))
            {
                Logger::Error("Image for landscape should be gray scale");
            }
            else
            {
                DVASSERT(imageSet[0]->GetWidth() == imageSet[0]->GetHeight());
                heightmap->BuildFromImage(imageSet[0]);
                retValue = true;
            }
            
            for_each(imageSet.begin(), imageSet.end(), SafeRelease<Image>);
        }
    }
    else if (heightmapPath.IsEqualToExtension(Heightmap::FileExtension()))
    {
        retValue = heightmap->Load(heightmapPath);
    }
	else
	{
		// SZ: don't assert here, and it will be possible to load landscape in editor and
		// fix wrong path to heightmap
		// 
		//DVASSERT(false && "wrong extension");
        heightmap->ReleaseData();
	}

    return retValue;
}

void Landscape::AllocateGeometryData()
{
    if (!landscapeMaterial)
    {
        landscapeMaterial = new NMaterial();
        landscapeMaterial->SetMaterialName(FastName("Landscape_TileMask_Material"));
        landscapeMaterial->SetFXName(NMaterialName::TILE_MASK);
    }

    for (int32 i = 0; i < LANDSCAPE_BATCHES_POOL_SIZE; i++)
    {
        AllocateRenderBatch();
    }

    if (!heightmap->Size())
    {
        subdivLevelCount = 0;
        return;
    }

    isRequireTangentBasis = (QualitySettingsSystem::Instance()->GetCurMaterialQuality(LANDSCAPE_QUALITY_NAME) == LANDSCAPE_QUALITY_VALUE_HIGH);

    rhi::VertexLayout vLayout;
    vLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    vLayout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2);
    if (isRequireTangentBasis)
    {
        vLayout.AddElement(rhi::VS_NORMAL, 0, rhi::VDT_FLOAT, 3);
        vLayout.AddElement(rhi::VS_TANGENT, 0, rhi::VDT_FLOAT, 3);
    }
    vertexLayoutUID = rhi::VertexLayout::UniqueId(vLayout);

    uint32 heightmapSizeMinus1 = heightmap->Size() - 1;
    uint32 maxLevels = FastLog2(heightmapSizeMinus1 / PATCH_QUAD_COUNT) + 1;
    subdivLevelCount = Min(maxLevels, (uint32)MAX_LANDSCAPE_SUBDIV_LEVELS);

    subdivPatchCount = 0;
    uint32 size = 1;
    for (uint32 k = 0; k < subdivLevelCount; ++k)
    {
        subdivLevelInfoArray[k].offset = subdivPatchCount;
        subdivLevelInfoArray[k].size = size;
        subdivPatchCount += size * size;
        Logger::FrameworkDebug("level: %d size: %d quadCount: %d", k, size, heightmapSizeMinus1 / size);
        size *= 2;
    }

    indices.resize(INITIAL_INDEX_BUFFER_CAPACITY);
    subdivPatchArray = new SubdivisionPatchInfo[subdivPatchCount];
    patchQuadArray = new PatchQuadInfo[subdivPatchCount];

    const uint32 quadSize = RENDER_QUAD_WIDTH - 1;
    quadsInWidth = heightmapSizeMinus1 / quadSize;
    // For cases where landscape is very small allocate 1 VBO.
    if (quadsInWidth == 0)
        quadsInWidth = 1;

    for (uint32 y = 0; y < quadsInWidth; ++y)
    {
        for (uint32 x = 0; x < quadsInWidth; ++x)
        {
            uint16 check = AllocateQuadVertexBuffer(x * quadSize, y * quadSize, quadSize);
            DVASSERT(check == (uint16)(x + y * quadsInWidth));
        }
    }
}

void Landscape::BuildLandscape()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    ReleaseGeometryData();
    AllocateGeometryData();

    UpdatePatchInfo(0, 0, 0);
}

Vector3 Landscape::GetPoint(int16 x, int16 y, uint16 height) const
{
    Vector3 res;
    res.x = (bbox.min.x + (float32)x / (float32)(heightmap->Size() - 1) * (bbox.max.x - bbox.min.x));
    res.y = (bbox.min.y + (float32)y / (float32)(heightmap->Size() - 1) * (bbox.max.y - bbox.min.y));
    res.z = (bbox.min.z + ((float32)height / (float32)Heightmap::MAX_VALUE) * (bbox.max.z - bbox.min.z));
    return res;
};

bool Landscape::GetHeightAtPoint(const Vector3& point, float& value) const
{
    if ((point.x > bbox.max.x) || (point.x < bbox.min.x) || (point.y > bbox.max.y) || (point.y < bbox.min.y))
    {
        return false;
    }

    const auto hmData = heightmap->Data();
    if (hmData == nullptr)
    {
        Logger::Error("[Landscape::GetHeightAtPoint] Trying to get height at point using empty heightmap data!");
        return false;
    }

    auto hmSize = heightmap->Size();
    float32 fx = static_cast<float>(hmSize - 1) * (point.x - bbox.min.x) / (bbox.max.x - bbox.min.x);
    float32 fy = static_cast<float>(hmSize - 1) * (point.y - bbox.min.y) / (bbox.max.y - bbox.min.y);
    int32 x = static_cast<int32>(fx);
    int32 y = static_cast<int32>(fy);
    int nextX = DAVA::Min(x + 1, hmSize - 1);
    int nextY = DAVA::Min(y + 1, hmSize - 1);
    int i00 = x + y * hmSize;
    int i01 = nextX + y * hmSize;
    int i10 = x + nextY * hmSize;
    int i11 = nextX + nextY * hmSize;
    float h00 = static_cast<float>(hmData[i00]);
    float h01 = static_cast<float>(hmData[i01]);
    float h10 = static_cast<float>(hmData[i10]);
    float h11 = static_cast<float>(hmData[i11]);
    float dx = fx - static_cast<float>(x);
    float dy = fy - static_cast<float>(y);
    float h0 = h00 * (1.0f - dx) + h01 * dx;
    float h1 = h10 * (1.0f - dx) + h11 * dx;
    value = (h0 * (1.0f - dy) + h1 * dy) * GetLandscapeHeight() / static_cast<float>(Heightmap::MAX_VALUE);

    return true;
}

bool Landscape::PlacePoint(const Vector3& worldPoint, Vector3& result, Vector3* normal) const
{
    result = worldPoint;

    if (GetHeightAtPoint(worldPoint, result.z) == false)
    {
        return false;
    }

    if (normal != nullptr)
    {
        const float32 normalDelta = 0.01f;
        Vector3 dx = result + Vector3(normalDelta, 0.0f, 0.0f);
        Vector3 dy = result + Vector3(0.0f, normalDelta, 0.0f);
        GetHeightAtPoint(dx, dx.z);
        GetHeightAtPoint(dy, dy.z);
        *normal = (dx - result).CrossProduct(dy - result);
        normal->Normalize();
    }

    return true;
};

Landscape::SubdivisionPatchInfo* Landscape::GetSubdivPatch(uint32 level, uint32 x, uint32 y)
{
    SubdivisionLevelInfo& levelInfo = subdivLevelInfoArray[level];

    if (x < levelInfo.size && y < levelInfo.size)
        return &subdivPatchArray[levelInfo.offset + levelInfo.size * y + x];
    else
        return 0;
}

void Landscape::UpdatePatchInfo(uint32 level, uint32 x, uint32 y)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    if (level >= subdivLevelCount)
        return;

    SubdivisionLevelInfo& levelInfo = subdivLevelInfoArray[level];
    PatchQuadInfo* patch = &patchQuadArray[levelInfo.offset + levelInfo.size * y + x];

    // Calculate patch bounding box
    uint32 realQuadCountInPatch = (heightmap->Size() - 1) / levelInfo.size;
    uint32 heightMapStartX = x * realQuadCountInPatch;
    uint32 heightMapStartY = y * realQuadCountInPatch;

    {
        patch->bbox = AABBox3();
        // Brute force / Think about recursive approach

        patch->maxError = 0.0f;

        uint16 * data = heightmap->Data();

        uint16 patchMod = realQuadCountInPatch / PATCH_QUAD_COUNT;

        for (uint16 xx = heightMapStartX; xx <= heightMapStartX + realQuadCountInPatch; ++xx)
        {
            for (uint16 yy = heightMapStartY; yy <= heightMapStartY + realQuadCountInPatch; ++yy)
            {
                uint16 value = data[heightmap->Size() * yy + xx];
                Vector3 pos = GetPoint(xx, yy, value);
                patch->bbox.AddPoint(pos);

                if (patchMod == 1)
                    continue;
                if ((xx % patchMod) == 0)
                    continue;
                if ((yy % patchMod) == 0)
                    continue;

                {
                    uint16 x0 = (xx / (patchMod)) * patchMod;
                    uint16 y0 = (yy / (patchMod)) * patchMod;

                    uint16 x1 = x0 + (realQuadCountInPatch / PATCH_QUAD_COUNT);
                    uint16 y1 = y0 + (realQuadCountInPatch / PATCH_QUAD_COUNT);

                    DVASSERT(x0 >= heightMapStartX && x0 <= heightMapStartX + realQuadCountInPatch);
                    DVASSERT(x1 >= heightMapStartX && x1 <= heightMapStartX + realQuadCountInPatch);
                    DVASSERT(y0 >= heightMapStartY && y0 <= heightMapStartY + realQuadCountInPatch);
                    DVASSERT(y1 >= heightMapStartY && y1 <= heightMapStartY + realQuadCountInPatch);

                    float xin = (float32)(xx - x0) / (float32)(x1 - x0);
                    float yin = (float32)(yy - y0) / (float32)(y1 - y0);

                    Vector3 p00 = GetPoint(x0, y0, data[heightmap->Size() * y0 + x0]);
                    Vector3 p01 = GetPoint(x0, y1, data[heightmap->Size() * y1 + x0]);
                    Vector3 p10 = GetPoint(x1, y0, data[heightmap->Size() * y0 + x1]);
                    Vector3 p11 = GetPoint(x1, y1, data[heightmap->Size() * y1 + x1]);

                    Vector3 lodPos = p00 * (1.0f - xin) * (1.0f - yin) + p01 * (1.0f - xin) * yin + p10 * xin * (1.0f - yin) + p11 * xin * yin;

                    DVASSERT(FLOAT_EQUAL(lodPos.x, pos.x) && FLOAT_EQUAL(lodPos.y, pos.y));

                    if (Abs(lodPos.z - pos.z) > Abs(patch->maxError))
                    {
                        patch->maxError = pos.z - lodPos.z;
                        patch->positionOfMaxError = pos;
                    }
                }
            }
        }
    }

    if (realQuadCountInPatch > MAX_QUAD_COUNT_IN_VBO)
    {
        patch->rdoQuad = -1;
    }
    else
    {
        uint32 x = heightMapStartX / MAX_QUAD_COUNT_IN_VBO;
        uint32 y = heightMapStartY / MAX_QUAD_COUNT_IN_VBO;

        patch->rdoQuad = y * quadsInWidth + x;
    }

    uint32 x2 = x * 2;
    uint32 y2 = y * 2;

    UpdatePatchInfo(level + 1, x2 + 0, y2 + 0);
    UpdatePatchInfo(level + 1, x2 + 1, y2 + 0);
    UpdatePatchInfo(level + 1, x2 + 0, y2 + 1);
    UpdatePatchInfo(level + 1, x2 + 1, y2 + 1);
}

void Landscape::SubdividePatch(uint32 level, uint32 x, uint32 y, uint8 clippingFlags)
{
    if (level == subdivLevelCount)
    {
        DVASSERT(false);
        return;
    }

    SubdivisionLevelInfo& levelInfo = subdivLevelInfoArray[level];
    uint32 offset = levelInfo.offset + levelInfo.size * y + x;
    PatchQuadInfo* patch = &patchQuadArray[offset];
    SubdivisionPatchInfo* subdivPatchInfo = &subdivPatchArray[offset];

    // Calculate patch bounding box
    Frustum::eFrustumResult frustumRes = Frustum::EFR_INSIDE;

    if (clippingFlags)
        frustumRes = frustum->Classify(patch->bbox, clippingFlags, subdivPatchInfo->startClipPlane);

    if (frustumRes == Frustum::EFR_OUTSIDE)
    {
        subdivPatchInfo->subdivisionState = SubdivisionPatchInfo::CLIPPED;
        return;
    }

    if (level == subdivLevelCount - 1)
    {
        TerminateSubdivision(level, x, y, levelInfo.size);
        return;
    }

    //Vector3 error = patch->positionOfMaxError + Vector3(0.0f, )
    float32 geometryRadius = Abs(patch->maxError);
    float32 geometryDistance = Distance(cameraPos, patch->positionOfMaxError);
    float32 geometryError = atanf(geometryRadius / geometryDistance);

    Vector3 max = patch->bbox.max;
    Vector3 origin = patch->bbox.GetCenter();

    float32 distance = Distance(origin, cameraPos);
    float32 radius = Distance(origin, max);
    float32 solidAngle = atanf(radius / distance);

    if ((patch->rdoQuad == -1) || (solidAngle > fovSolidAngleError) || (geometryError > fovGeometryAngleError) || (patch->maxError > fovAbsHeightError))
    {
        subdivPatchInfo->subdivisionState = SubdivisionPatchInfo::SUBDIVIDED;
        subdivPatchInfo->lastSubdividedSize = levelInfo.size;

        uint32 x2 = x * 2;
        uint32 y2 = y * 2;

        SubdividePatch(level + 1, x2 + 0, y2 + 0, clippingFlags);
        SubdividePatch(level + 1, x2 + 1, y2 + 0, clippingFlags);
        SubdividePatch(level + 1, x2 + 0, y2 + 1, clippingFlags);
        SubdividePatch(level + 1, x2 + 1, y2 + 1, clippingFlags);
    }
    else
    {
        //DrawPatch(level, x, y, 0, 0, 0, 0);
        TerminateSubdivision(level, x, y, levelInfo.size);
    }
}

void Landscape::TerminateSubdivision(uint32 level, uint32 x, uint32 y, uint32 lastSubdividedSize)
{
    if (level == subdivLevelCount)
    {
        return;
    }

    SubdivisionLevelInfo& levelInfo = subdivLevelInfoArray[level];
    SubdivisionPatchInfo* subdivPatchInfo = &subdivPatchArray[levelInfo.offset + levelInfo.size * y + x];

    subdivPatchInfo->lastSubdividedSize = lastSubdividedSize;
    subdivPatchInfo->subdivisionState = SubdivisionPatchInfo::TERMINATED;

    uint32 x2 = x * 2;
    uint32 y2 = y * 2;

    TerminateSubdivision(level + 1, x2 + 0, y2 + 0, lastSubdividedSize);
    TerminateSubdivision(level + 1, x2 + 1, y2 + 0, lastSubdividedSize);
    TerminateSubdivision(level + 1, x2 + 0, y2 + 1, lastSubdividedSize);
    TerminateSubdivision(level + 1, x2 + 1, y2 + 1, lastSubdividedSize);
}

void Landscape::AddPatchToRenderNoInstancing(uint32 level, uint32 x, uint32 y)
{
    DVASSERT(level < subdivLevelCount);

    SubdivisionLevelInfo& levelInfo = subdivLevelInfoArray[level];

    // TODO: optimise all offset calculations in all places
    SubdivisionPatchInfo* subdivPatchInfo = &subdivPatchArray[levelInfo.offset + levelInfo.size * y + x];

    uint32 state = subdivPatchInfo->subdivisionState;
    if (state == SubdivisionPatchInfo::CLIPPED)
        return;

    if (state == SubdivisionPatchInfo::SUBDIVIDED)
    {
        uint32 x2 = x * 2;
        uint32 y2 = y * 2;

        AddPatchToRenderNoInstancing(level + 1, x2 + 0, y2 + 0);
        AddPatchToRenderNoInstancing(level + 1, x2 + 1, y2 + 0);
        AddPatchToRenderNoInstancing(level + 1, x2 + 0, y2 + 1);
        AddPatchToRenderNoInstancing(level + 1, x2 + 1, y2 + 1);
    }
    else
    {
        SubdivisionPatchInfo* xNeg = GetSubdivPatch(level, x - 1, y);
        SubdivisionPatchInfo* xPos = GetSubdivPatch(level, x + 1, y);
        SubdivisionPatchInfo* yNeg = GetSubdivPatch(level, x, y - 1);
        SubdivisionPatchInfo* yPos = GetSubdivPatch(level, x, y + 1);

        uint32 xNegSize = levelInfo.size;
        uint32 xPosSize = levelInfo.size;
        uint32 yNegSize = levelInfo.size;
        uint32 yPosSize = levelInfo.size;

        if (xNeg)
            xNegSize = xNeg->lastSubdividedSize;
        if (xPos)
            xPosSize = xPos->lastSubdividedSize;
        if (yNeg)
            yNegSize = yNeg->lastSubdividedSize;
        if (yPos)
            yPosSize = yPos->lastSubdividedSize;

        DrawPatch(level, x, y, xNegSize, xPosSize, yNegSize, yPosSize);
    }
}

void Landscape::DrawLandscape()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    ClearQueue();
    AddPatchToRenderNoInstancing(0, 0, 0);
    FlushQueue();
}

void Landscape::DrawPatch(uint32 level, uint32 xx, uint32 yy, uint32 xNegSize, uint32 xPosSize, uint32 yNegSize, uint32 yPosSize)
{
    SubdivisionLevelInfo& levelInfo = subdivLevelInfoArray[level];
    PatchQuadInfo* patch = &patchQuadArray[levelInfo.offset + levelInfo.size * yy + xx];

    if ((patch->rdoQuad != queueRdoQuad) && (queueRdoQuad != -1))
    {
        FlushQueue();
    }

    queueRdoQuad = patch->rdoQuad;

    // Draw Middle
    uint32 realVertexCountInPatch = (heightmap->Size() - 1) / levelInfo.size;
    uint32 step = realVertexCountInPatch / PATCH_QUAD_COUNT;
    uint32 heightMapStartX = xx * realVertexCountInPatch;
    uint32 heightMapStartY = yy * realVertexCountInPatch;

    ResizeIndicesBufferIfNeeded(queueIndexCount + PATCH_QUAD_COUNT * PATCH_QUAD_COUNT);

    uint16* indicesPtr = indices.data() + queueIndexCount;
    // Draw middle block
    {
        for (uint16 y = (heightMapStartY & RENDER_QUAD_AND); y < (heightMapStartY & RENDER_QUAD_AND) + realVertexCountInPatch; y += step)
        {
            for (uint16 x = (heightMapStartX & RENDER_QUAD_AND); x < (heightMapStartX & RENDER_QUAD_AND) + realVertexCountInPatch; x += step)
            {
                uint16 x0 = x;
                uint16 y0 = y;
                uint16 x1 = x + step;
                uint16 y1 = y + step;

                uint16 x0aligned = x0;
                uint16 y0aligned = y0;
                uint16 x1aligned = x1;
                uint16 y1aligned = y1;

                uint16 x0aligned2 = x0;
                uint16 y0aligned2 = y0;
                uint16 x1aligned2 = x1;
                uint16 y1aligned2 = y1;

                if (x == (heightMapStartX & RENDER_QUAD_AND) && (xNegSize != 0))
                {
                    uint16 alignMod = levelInfo.size / xNegSize;
                    if (alignMod > 1)
                    {
                        y0aligned = y0 / (alignMod * step) * (alignMod * step);
                        y1aligned = y1 / (alignMod * step) * (alignMod * step);
                    }
                }

                if (y == (heightMapStartY & RENDER_QUAD_AND) && (yNegSize != 0))
                {
                    uint16 alignMod = levelInfo.size / yNegSize;
                    if (alignMod > 1)
                    {
                        x0aligned = x0 / (alignMod * step) * (alignMod * step);
                        x1aligned = x1 / (alignMod * step) * (alignMod * step);
                    }
                }

                if ((x == ((heightMapStartX & RENDER_QUAD_AND) + realVertexCountInPatch - step)) && (xPosSize != 0))
                {
                    uint16 alignMod = levelInfo.size / xPosSize;
                    if (alignMod > 1)
                    {
                        y0aligned2 = y0 / (alignMod * step) * (alignMod * step);
                        y1aligned2 = y1 / (alignMod * step) * (alignMod * step);
                    }
                }

                if ((y == ((heightMapStartY & RENDER_QUAD_AND) + realVertexCountInPatch - step)) && (yPosSize != 0))
                {
                    uint16 alignMod = levelInfo.size / yPosSize;
                    if (alignMod > 1)
                    {
                        x0aligned2 = x0 / (alignMod * step) * (alignMod * step);
                        x1aligned2 = x1 / (alignMod * step) * (alignMod * step);
                    }
                }

                *indicesPtr++ = GetVertexIndex(x0aligned, y0aligned);
                *indicesPtr++ = GetVertexIndex(x1aligned, y0aligned2);
                *indicesPtr++ = GetVertexIndex(x0aligned2, y1aligned);

                *indicesPtr++ = GetVertexIndex(x1aligned, y0aligned2);
                *indicesPtr++ = GetVertexIndex(x1aligned2, y1aligned2);
                *indicesPtr++ = GetVertexIndex(x0aligned2, y1aligned);

                queueIndexCount += 6;
            }
        }
    }
}

void Landscape::FlushQueue()
{
    if (queueIndexCount == 0)
        return;

    DVASSERT(flushQueueCounter <= static_cast<int32>(renderBatchArray.size()));
    if (static_cast<int32>(renderBatchArray.size()) == flushQueueCounter)
    {
        AllocateRenderBatch();
    }

    DVASSERT(queueRdoQuad != -1);

    DynamicBufferAllocator::AllocResultIB indexBuffer = DynamicBufferAllocator::AllocateIndexBuffer(queueIndexCount);
    DVASSERT(indexBuffer.allocatedindices == queueIndexCount);

    Memcpy(indexBuffer.data, indices.data(), queueIndexCount * sizeof(uint16));
    RenderBatch* batch = renderBatchArray[flushQueueCounter].renderBatch;
    batch->indexBuffer = indexBuffer.buffer;
    batch->indexCount = queueIndexCount;
    batch->startIndex = indexBuffer.baseIndex;
    batch->vertexBuffer = vertexBuffers[queueRdoQuad];

    activeRenderBatchArray.push_back(batch);
    ClearQueue();

    drawIndices += batch->indexCount;
    ++flushQueueCounter;
}

void Landscape::ClearQueue()
{
    queueIndexCount = 0;
    queueRdoQuad = -1;
}

void Landscape::PrepareToRender(Camera* camera)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    RenderObject::PrepareToRender(camera);

    TIME_PROFILE("Landscape.PrepareToRender");

    drawIndices = 0;
    activeRenderBatchArray.clear();

    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::LANDSCAPE_DRAW))
    {
        return;
    }

    if (!subdivLevelCount)
        return;

    flushQueueCounter = 0;
    camera = GetRenderSystem()->GetMainCamera();
    frustum = camera->GetFrustum();
    cameraPos = camera->GetPosition();

    float32 fovLerp = Clamp((camera->GetFOV() - zoomFov) / (normalFov - zoomFov), 0.0f, 1.0f);
    fovSolidAngleError = zoomSolidAngleError + (solidAngleError - zoomSolidAngleError) * fovLerp;
    fovGeometryAngleError = zoomGeometryAngleError + (geometryAngleError - zoomGeometryAngleError) * fovLerp;
    fovAbsHeightError = zoomAbsHeightError + (absHeightError - zoomAbsHeightError) * fovLerp;

    if (Renderer::GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_LANDSCAPE_LODS))
    {
        SubdividePatch(0, 0, 0, 0x3f);
    }

    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_WORLD, &Matrix4::IDENTITY, (pointer_size)&Matrix4::IDENTITY);

    DrawLandscape();
}

bool Landscape::GetLevel0Geometry(Vector<LandscapeVertex>& vertices, Vector<int32>& indices) const
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();
    if (heightmap->Data() == nullptr)
    {
        return false;
    }

    uint32 gridWidth = heightmap->Size();
    uint32 gridHeight = heightmap->Size();
    vertices.resize(gridWidth * gridHeight);
    for (uint32 y = 0, index = 0; y < gridHeight; ++y)
    {
        uint32 row = y * heightmap->Size();
        float32 ny = static_cast<float32>(y) / static_cast<float32>(gridHeight - 1);
        for (uint32 x = 0; x < gridWidth; ++x)
        {
            float32 nx = static_cast<float32>(x) / static_cast<float32>(gridWidth - 1);
            vertices[index].position = GetPoint(x, y, heightmap->Data()[row + x]);
            vertices[index].texCoord = Vector2(nx, ny);
            index++;
        }
    }

    indices.resize((gridWidth - 1) * (gridHeight - 1) * 6);
    for (uint32 y = 0, index = 0; y < gridHeight - 1; ++y)
    {
        for (uint32 x = 0; x < gridWidth - 1; ++x)
        {
            indices[index++] = x + y * gridWidth;
            indices[index++] = (x + 1) + y * gridWidth;
            indices[index++] = x + (y + 1) * gridWidth;
            indices[index++] = (x + 1) + y * gridWidth;
            indices[index++] = (x + 1) + (y + 1) * gridWidth;
            indices[index++] = x + (y + 1) * gridWidth;
        }
    }
    return true;
}

const FilePath & Landscape::GetHeightmapPathname()
{
    return heightmapPath;
}
	
void Landscape::SetHeightmapPathname(const FilePath & newHeightMapPath)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    if (newHeightMapPath == heightmapPath)
    {
        return;
    }
    BuildLandscapeFromHeightmapImage(newHeightMapPath, bbox);
}
	
float32 Landscape::GetLandscapeSize() const
{
	return bbox.GetSize().x;
}
	
void Landscape::SetLandscapeSize(float32 newSize)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    Vector3 newLandscapeSize(newSize, newSize, bbox.GetSize().z);
    SetLandscapeSize(newLandscapeSize);
}

float32 Landscape::GetLandscapeHeight() const
{
    return bbox.GetSize().z;
}
	
void Landscape::SetLandscapeHeight(float32 newHeight)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	Vector3 newLandscapeSize(bbox.GetSize().x, bbox.GetSize().y, newHeight);
	SetLandscapeSize(newLandscapeSize);
}

void Landscape::SetLandscapeSize(const Vector3 & newLandscapeSize)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    if (newLandscapeSize.z < 0.0f || newLandscapeSize.x < 0 || newLandscapeSize.y < 0)
    {
        return;
    }
    if (newLandscapeSize == bbox.GetSize())
    {
        return;
    }
    bbox.Empty();
	bbox.AddPoint(Vector3(-newLandscapeSize.x/2.f, -newLandscapeSize.y/2.f, 0.f));
	bbox.AddPoint(Vector3(newLandscapeSize.x/2.f, newLandscapeSize.y/2.f, newLandscapeSize.z));
    BuildLandscape();

    if (foliageSystem)
    {
        foliageSystem->SyncFoliageWithLandscape();
    }
}

void Landscape::GetDataNodes(Set<DataNode*>& dataNodes)
{
    NMaterial* curMaterialNode = landscapeMaterial;
    while (curMaterialNode != NULL)
    {
        dataNodes.insert(curMaterialNode);
        curMaterialNode = curMaterialNode->GetParent();
    }
}
    
void Landscape::Save(KeyedArchive * archive, SerializationContext * serializationContext)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    AnimatedObject::SaveObject(archive);

    archive->SetUInt32("ro.debugflags", debugFlags);
    archive->SetUInt32("ro.sOclIndex", staticOcclusionIndex);

    //VI: save only VISIBLE flag for now. May be extended in the future
    archive->SetUInt32("ro.flags", flags & RenderObject::SERIALIZATION_CRITERIA);

    uint64 matKey = landscapeMaterial->GetNodeID();
    archive->SetUInt64("matname", matKey);

    //TODO: remove code in future. Need for transition from *.png to *.heightmap
    if (!heightmapPath.IsEqualToExtension(Heightmap::FileExtension()))
    {
        heightmapPath.ReplaceExtension(Heightmap::FileExtension());
    }

    heightmap->Save(heightmapPath);
    archive->SetString("hmap", heightmapPath.GetRelativePathname(serializationContext->GetScenePath()));
    archive->SetByteArrayAsType("bbox", bbox);
}
    
void Landscape::Load(KeyedArchive * archive, SerializationContext * serializationContext)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    debugFlags = archive->GetUInt32("ro.debugflags", 0);
    staticOcclusionIndex = (uint16)archive->GetUInt32("ro.sOclIndex", INVALID_STATIC_OCCLUSION_INDEX);

    //VI: load only VISIBLE flag for now. May be extended in the future.
    uint32 savedFlags = RenderObject::SERIALIZATION_CRITERIA & archive->GetUInt32("ro.flags", RenderObject::SERIALIZATION_CRITERIA);
    flags = (savedFlags | (flags & ~RenderObject::SERIALIZATION_CRITERIA));

    uint64 matKey = archive->GetUInt64("matname");

    if (!matKey) //Load from old landscape format: get material from batch0

    {
        uint32 roBatchCount = archive->GetUInt32("ro.batchCount");
        DVASSERT(roBatchCount);
        KeyedArchive* batchesArch = archive->GetArchive("ro.batches");
        DVASSERT(batchesArch);
        KeyedArchive* batchArch = batchesArch->GetArchive(KeyedArchive::GenKeyFromIndex(0));
        DVASSERT(batchArch);

        matKey = batchArch->GetUInt64("rb.nmatname");
    }

    DVASSERT(matKey);
    landscapeMaterial = SafeRetain(static_cast<NMaterial*>(serializationContext->GetDataBlock(matKey)));
    if (landscapeMaterial)
    {
        //Import old params
        if (!landscapeMaterial->HasLocalProperty(NMaterialParamName::PARAM_LANDSCAPE_TEXTURE_TILING))
        {
            if (archive->IsKeyExists("tiling_0"))
            {
                Vector2 tilingValue;
                tilingValue = archive->GetByteArrayAsType("tiling_0", tilingValue);
                landscapeMaterial->AddProperty(NMaterialParamName::PARAM_LANDSCAPE_TEXTURE_TILING, tilingValue.data, rhi::ShaderProp::TYPE_FLOAT2);
            }
            else if (landscapeMaterial->HasLocalProperty(NMaterialParamName::DEPRECATED_LANDSCAPE_TEXTURE_0_TILING))
            {
                landscapeMaterial->AddProperty(NMaterialParamName::PARAM_LANDSCAPE_TEXTURE_TILING, landscapeMaterial->GetLocalPropValue(NMaterialParamName::DEPRECATED_LANDSCAPE_TEXTURE_0_TILING), rhi::ShaderProp::TYPE_FLOAT2);
                for (int32 i = 0; i < 4; i++)
                {
                    FastName propName(Format("texture%dTiling", i));
                    if (landscapeMaterial->HasLocalProperty(propName))
                        landscapeMaterial->RemoveProperty(propName);
                }
            }
        }

        landscapeMaterial->PreBuildMaterial(PASS_FORWARD);
    }

    FilePath heightmapPath = serializationContext->GetScenePath() + archive->GetString("hmap");
    AABBox3 loadedBbox = archive->GetByteArrayAsType("bbox", AABBox3());

    BuildLandscapeFromHeightmapImage(heightmapPath, loadedBbox);
}

Heightmap * Landscape::GetHeightmap()
{
    return heightmap;
}

void Landscape::SetHeightmap(DAVA::Heightmap *height)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    SafeRelease(heightmap);
    heightmap = SafeRetain(height);
    
    BuildLandscape();
}

NMaterial* Landscape::GetMaterial()
{
    return landscapeMaterial;
}

void Landscape::SetMaterial(NMaterial* material)
{
    SafeRetain(material);
    SafeRelease(landscapeMaterial);
    landscapeMaterial = material;

    for (uint32 i = 0; i < GetRenderBatchCount(); ++i)
        GetRenderBatch(i)->SetMaterial(landscapeMaterial);
}

RenderObject* Landscape::Clone(RenderObject* newObject)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    if (!newObject)
    {
        DVASSERT_MSG(IsPointerToExactClass<Landscape>(this), "Can clone only Landscape");
        newObject = new Landscape();
    }

    Landscape *newLandscape = static_cast<Landscape *>(newObject);
    newLandscape->landscapeMaterial = landscapeMaterial->Clone();

    newLandscape->flags = flags;
    newLandscape->BuildLandscapeFromHeightmapImage(heightmapPath, bbox);

	return newObject;
}
	
int32 Landscape::GetDrawIndices() const
{
    return drawIndices;
}

void Landscape::SetFoliageSystem(FoliageSystem* _foliageSystem)
{
    foliageSystem = _foliageSystem;
}

void Landscape::ResizeIndicesBufferIfNeeded(DAVA::uint32 newSize)
{
    if (indices.size() < newSize)
    {
        indices.resize(2 * newSize);
    }
};

void Landscape::AllocateRenderBatch()
{
    ScopedPtr<RenderBatch> batch(new RenderBatch());
    AddRenderBatch(batch);

    batch->SetMaterial(landscapeMaterial);
    batch->SetSortingKey(10);

    batch->vertexLayoutId = vertexLayoutUID;
    batch->vertexCount = RENDER_QUAD_WIDTH * RENDER_QUAD_WIDTH;
}

void Landscape::SetForceFirstLod(bool force)
{
    forceFirstLod = force;
}

void Landscape::SetUpdatable(bool isUpdatable)
{
    if (updatable != isUpdatable)
    {
        updatable = isUpdatable;
        BuildLandscape();
    }
}

bool Landscape::IsUpdatable() const
{
    return updatable;
}

/*
void Landscape::UpdateNodeChildrenBoundingBoxesRecursive(LandQuadTreeNode<LandscapeQuad>& root, Heightmap* fromHeightmap)
{
    root.data.bbox.Empty();

    for (int32 y = root.data.y, yEnd = root.data.y + root.data.size + 1; y < yEnd; ++y)
    {
        auto row = y * fromHeightmap->Size();
        for (int32 x = root.data.x, xEnd = root.data.x + root.data.size + 1; x < xEnd; ++x)
        {
            root.data.bbox.AddPoint(GetPoint(x, y, fromHeightmap->Data()[x + row]));
        }
    }

    if (nullptr != root.children)
    {
        for (int i = 0; i < 4; ++i)
        {
            UpdateNodeChildrenBoundingBoxesRecursive(root.children[i], fromHeightmap);
        }
    }
}
*/

void Landscape::UpdatePart(Heightmap* fromHeightmap, const Rect2i& rect)
{
    //TODO: refactor this!

    int32 heightmapSize = fromHeightmap->Size();
    DVASSERT(heightmap->Size() == heightmapSize);

    DVASSERT(!isRequireTangentBasis && "TODO: Landscape::UpdatePart() for HIGH Quality");

    uint32 vertexSize = sizeof(LandscapeVertex);
    if (!isRequireTangentBasis)
    {
        vertexSize -= sizeof(Vector3); // (LandscapeVertex::normal);
        vertexSize -= sizeof(Vector3); // (LandscapeVertex::tangent);
    }

    const uint32 quadSize = RENDER_QUAD_WIDTH - 1;

    for (uint32 quadY = 0; quadY < quadsInWidth; ++quadY)
    {
        for (uint32 quadX = 0; quadX < quadsInWidth; ++quadX)
        {
            Rect2i quadRect(quadX * quadSize, quadY * quadSize, quadSize, quadSize);
            Rect2i intersect = quadRect.Intersection(rect);
            if (intersect.dx || intersect.dy)
            {
                rhi::HVertexBuffer vertexBuffer = vertexBuffers[quadX + quadY * quadsInWidth];

                uint32 verticesCount = (quadSize + 1) * (quadSize + 1);
                uint8* quadVertices = new uint8[verticesCount * vertexSize];

                uint32 index = 0;
                for (uint32 y = quadY * quadSize; y < quadY * quadSize + quadSize + 1; ++y)
                {
                    auto row = y * heightmapSize;
                    auto texCoordV = 1.0f - (float32)(y) / (float32)(heightmapSize - 1);

                    for (uint32 x = quadX * quadSize; x < quadX * quadSize + quadSize + 1; ++x)
                    {
                        LandscapeVertex* vertex = reinterpret_cast<LandscapeVertex*>(&quadVertices[index * vertexSize]);

                        auto texCoordU = (float32)(x) / (float32)(heightmapSize - 1);

                        vertex->position = GetPoint(x, y, fromHeightmap->Data()[x + row]);
                        vertex->texCoord = Vector2(texCoordU, texCoordV);

                        //TODO: fill tangent data

                        //TODO: need update bbox in subdivide path info
                        //quad.bbox.AddPoint(quadVertices[index].position);
                        ++index;
                    }
                }

                uint32 vBufferSize = verticesCount * vertexSize;
                rhi::UpdateVertexBuffer(vertexBuffer, quadVertices, 0, vBufferSize);
                SafeDeleteArray(quadVertices);
            }
        }
    }
}

}
