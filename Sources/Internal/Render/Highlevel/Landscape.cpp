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

const FastName Landscape::FLAG_USE_INSTANCING("USE_INSTANCING");
const FastName Landscape::FLAG_LOD_MORPHING("LOD_MORPHING");

const FastName Landscape::LANDSCAPE_QUALITY_NAME("Landscape");
const FastName Landscape::LANDSCAPE_QUALITY_VALUE_HIGH("HIGH");

const uint32 LANDSCAPE_BATCHES_POOL_SIZE = 32;

Landscape::Landscape()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    type = TYPE_LANDSCAPE;

    heightmap = new Heightmap();

    solidAngleError = (26.5f * PI / 180.0f);
    geometryAngleError = (1.0f * PI / 180.0f);
    absHeightError = 3.0f;

    zoomSolidAngleError = solidAngleError;
    zoomGeometryAngleError = (0.1f * PI / 180.0f);
    zoomAbsHeightError = 0.5f;

    zoomFov = 6.5f;
    normalFov = 70.0f;

    maxHeightError = 0.02f;
    maxPatchRadiusError = .65f;

    useInstancing = rhi::DeviceCaps().isInstancingSupported;
    useLodMorphing = useInstancing;

    instanceDataSize = useLodMorphing ? INSTANCE_DATA_SIZE_MORPHING : INSTANCE_DATA_SIZE;

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

void Landscape::RestoreGeometry()
{
    for (auto& restoreData : bufferRestoreData)
    {
        switch (restoreData.bufferType)
        {
        case RestoreBufferData::RESTORE_BUFFER_VERTEX:
            if (rhi::NeedRestoreVertexBuffer(static_cast<rhi::HVertexBuffer>(restoreData.buffer)))
                rhi::UpdateVertexBuffer(static_cast<rhi::HVertexBuffer>(restoreData.buffer), restoreData.data, 0, restoreData.dataSize);
            break;

        case RestoreBufferData::RESTORE_BUFFER_INDEX:
            if (rhi::NeedRestoreIndexBuffer(static_cast<rhi::HIndexBuffer>(restoreData.buffer)))
                rhi::UpdateIndexBuffer(static_cast<rhi::HIndexBuffer>(restoreData.buffer), restoreData.data, 0, restoreData.dataSize);
            break;
        }
    }
}

void Landscape::ReleaseGeometryData()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    ////General
    for (IndexedRenderBatch& batch : renderBatchArray)
        batch.renderBatch->Release();
    renderBatchArray.clear();
    activeRenderBatchArray.clear();

    for (auto& restoreData : bufferRestoreData)
        SafeDeleteArray(restoreData.data);
    bufferRestoreData.clear();

    subdivPatchArray.clear();
    patchQuadArray.clear();

    subdivLevelCount = 0;
    subdivPatchCount = 0;

    ////Non-instanced data
    for (rhi::HVertexBuffer handle : vertexBuffers)
        rhi::DeleteVertexBuffer(handle);
    vertexBuffers.clear();

    indices.clear();

    quadsInWidthPow2 = 0;

    ////Instanced data
    if (patchVertexBuffer)
        rhi::DeleteVertexBuffer(patchVertexBuffer);

    if (patchIndexBuffer)
        rhi::DeleteIndexBuffer(patchIndexBuffer);

    for (InstanceDataBuffer* buffer : freeInstanceDataBuffers)
    {
        rhi::DeleteVertexBuffer(buffer->buffer);
        SafeDelete(buffer);
    }
    freeInstanceDataBuffers.clear();

    for (InstanceDataBuffer* buffer : usedInstanceDataBuffers)
    {
        rhi::DeleteVertexBuffer(buffer->buffer);
        SafeDelete(buffer);
    }
    usedInstanceDataBuffers.clear();
}

void Landscape::BuildLandscapeFromHeightmapImage(const FilePath& heightmapPathname, const AABBox3& _box)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    heightmapPath = heightmapPathname;
    BuildHeightmap();

    bbox = _box;

    RebuildLandscape();

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
        Vector<Image*> imageSet;
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

    if (!heightmap->Size())
    {
        subdivLevelCount = 0;
        return;
    }

    uint32 heightmapSize = heightmap->Size();
    uint32 maxLevels = FastLog2(heightmapSize / PATCH_SIZE_QUADS) + 1;
    subdivLevelCount = Min(maxLevels, (uint32)MAX_LANDSCAPE_SUBDIV_LEVELS);
    minSubdivLevelSize = useInstancing ? 0 : heightmapSize / RENDER_PARCEL_SIZE_QUADS;
    heightmapSizePow2 = uint32(HighestBitIndex(heightmapSize));

    subdivPatchCount = 0;
    uint32 size = 1;
    for (uint32 k = 0; k < subdivLevelCount; ++k)
    {
        subdivLevelInfoArray[k].offset = subdivPatchCount;
        subdivLevelInfoArray[k].size = size;
        subdivPatchCount += size * size;
        Logger::FrameworkDebug("level: %d size: %d quadCount: %d", k, size, heightmapSize / size);
        size *= 2;
    }

    subdivPatchArray.resize(subdivPatchCount);
    patchQuadArray.resize(subdivPatchCount);

    if (landscapeMaterial->HasLocalFlag(FLAG_USE_INSTANCING))
        landscapeMaterial->SetFlag(FLAG_USE_INSTANCING, useInstancing ? 1 : 0);
    else
        landscapeMaterial->AddFlag(FLAG_USE_INSTANCING, useInstancing ? 1 : 0);

    if (landscapeMaterial->HasLocalFlag(FLAG_LOD_MORPHING))
        landscapeMaterial->SetFlag(FLAG_LOD_MORPHING, useLodMorphing ? 1 : 0);
    else
        landscapeMaterial->AddFlag(FLAG_LOD_MORPHING, useLodMorphing ? 1 : 0);

    if (useInstancing)
    {
        AllocateGeometryDataInstancing();
    }
    else
    {
        AllocateGeometryDataNoInstancing();
    }
}

void Landscape::RebuildLandscape()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    ReleaseGeometryData();
    AllocateGeometryData();

    UpdatePatchInfo(0, 0, 0);
}

Texture* Landscape::CreateHeightTexture(Heightmap* heightmap)
{
    const uint32 hmSize = heightmap->Size();
    DVASSERT(IsPowerOf2(heightmap->Size()));

    Vector<Image*> textureData;
    if (useLodMorphing)
    {
        textureData.reserve(HighestBitIndex(hmSize));

        uint32 mipSize = hmSize;
        uint32 step = 1;
        uint32 mipLevel = 0;
        uint32* mipData = new uint32[mipSize * mipSize]; //RGBA8888

        while (mipSize)
        {
            uint16* mipDataPtr = reinterpret_cast<uint16*>(mipData);
            for (uint32 y = 0; y < mipSize; ++y)
            {
                uint32 mipLastIndex = mipSize - 1;
                uint16 yy = y * step;
                uint16 y1 = yy;
                uint16 y2 = yy;
                if (y & 0x1 && y != mipLastIndex)
                {
                    y1 -= step;
                    y2 += step;
                }

                for (uint32 x = 0; x < mipSize; ++x)
                {
                    uint16 xx = x * step;
                    uint16 x1 = xx;
                    uint16 x2 = xx;
                    if (x & 0x1 && x != mipLastIndex)
                    {
                        x1 -= step;
                        x2 += step;
                    }

                    *mipDataPtr++ = heightmap->GetHeight(xx, yy);

                    uint16 h1 = heightmap->GetHeight(x1, y1);
                    uint16 h2 = heightmap->GetHeightClamp(x2, y2);
                    *mipDataPtr++ = (h1 + h2) >> 1;
                }
            }

            Image* mipImg = Image::CreateFromData(mipSize, mipSize, FORMAT_RGBA8888, reinterpret_cast<uint8*>(mipData));
            mipImg->mipmapLevel = mipLevel;
            textureData.push_back(mipImg);

            mipSize >>= 1;
            step <<= 1;
            mipLevel++;
        }

        SafeDeleteArray(mipData);
    }
    else
    {
        Image* heightImage = Image::CreateFromData(hmSize, hmSize, FORMAT_RGBA4444, reinterpret_cast<uint8*>(heightmap->Data()));
        textureData.push_back(heightImage);
    }

    Texture* tx = Texture::CreateFromData(textureData);
    tx->SetWrapMode(rhi::TEXADDR_CLAMP, rhi::TEXADDR_CLAMP);
    tx->SetMinMagFilter(rhi::TEXFILTER_NEAREST, rhi::TEXFILTER_NEAREST, useLodMorphing ? rhi::TEXMIPFILTER_NEAREST : rhi::TEXMIPFILTER_NONE);

    for (Image* img : textureData)
        img->Release();

    return tx;
}

Vector3 Landscape::GetPoint(int16 x, int16 y, uint16 height) const
{
    Vector3 res;
    res.x = (bbox.min.x + (float32)x / (float32)(heightmap->Size()) * (bbox.max.x - bbox.min.x));
    res.y = (bbox.min.y + (float32)y / (float32)(heightmap->Size()) * (bbox.max.y - bbox.min.y));
    res.z = (bbox.min.z + ((float32)height / (float32)Heightmap::MAX_VALUE) * (bbox.max.z - bbox.min.z));
    return res;
}

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

    //HEIGHTMAP_COMPLETE
    auto hmSize = heightmap->Size();
    float32 fx = static_cast<float>(hmSize) * (point.x - bbox.min.x) / (bbox.max.x - bbox.min.x);
    float32 fy = static_cast<float>(hmSize) * (point.y - bbox.min.y) / (bbox.max.y - bbox.min.y);
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
        return &subdivPatchArray[levelInfo.offset + (y << level) + x];
    else
        return 0;
}

void Landscape::UpdatePatchInfo(uint32 level, uint32 x, uint32 y)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    if (level >= subdivLevelCount)
        return;

    SubdivisionLevelInfo& levelInfo = subdivLevelInfoArray[level];
    PatchQuadInfo* patch = &patchQuadArray[levelInfo.offset + (y << level) + x];

    // Calculate patch bounding box
    int32 hmSize = heightmap->Size();
    uint32 realQuadCountInPatch = hmSize >> level;
    uint32 heightMapStartX = x * realQuadCountInPatch;
    uint32 heightMapStartY = y * realQuadCountInPatch;

    {
        patch->bbox = AABBox3();
        // Brute force / Think about recursive approach

        patch->maxError = 0.0f;

        uint16 patchMod = realQuadCountInPatch / PATCH_SIZE_QUADS;

        for (uint16 xx = heightMapStartX; xx <= heightMapStartX + realQuadCountInPatch; ++xx)
        {
            for (uint16 yy = heightMapStartY; yy <= heightMapStartY + realQuadCountInPatch; ++yy)
            {
                uint16 hmValue = heightmap->GetHeightClamp(xx, yy);

                Vector3 pos = GetPoint(xx, yy, hmValue);
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

                    uint16 x1 = x0 + (realQuadCountInPatch / PATCH_SIZE_QUADS);
                    uint16 y1 = y0 + (realQuadCountInPatch / PATCH_SIZE_QUADS);

                    DVASSERT(x0 >= heightMapStartX && x0 <= heightMapStartX + realQuadCountInPatch);
                    DVASSERT(x1 >= heightMapStartX && x1 <= heightMapStartX + realQuadCountInPatch);
                    DVASSERT(y0 >= heightMapStartY && y0 <= heightMapStartY + realQuadCountInPatch);
                    DVASSERT(y1 >= heightMapStartY && y1 <= heightMapStartY + realQuadCountInPatch);

                    float xin = (float32)(xx - x0) / (float32)(x1 - x0);
                    float yin = (float32)(yy - y0) / (float32)(y1 - y0);

                    Vector3 p00 = GetPoint(x0, y0, heightmap->GetHeightClamp(x0, y0));
                    Vector3 p01 = GetPoint(x0, y1, heightmap->GetHeightClamp(x0, y1));
                    Vector3 p10 = GetPoint(x1, y0, heightmap->GetHeightClamp(x1, y0));
                    Vector3 p11 = GetPoint(x1, y1, heightmap->GetHeightClamp(x1, y1));

                    Vector3 lodPos = p00 * (1.0f - xin) * (1.0f - yin) + p01 * (1.0f - xin) * yin + p10 * xin * (1.0f - yin) + p11 * xin * yin;

                    DVASSERT(FLOAT_EQUAL(lodPos.x, pos.x) && FLOAT_EQUAL(lodPos.y, pos.y));

                    if (Abs(lodPos.z - pos.z) > Abs(patch->maxError))
                    {
                        patch->maxError = Abs(pos.z - lodPos.z);
                        patch->positionOfMaxError = pos;
                    }
                }
            }
        }

        patch->radius = Distance(patch->bbox.GetCenter(), patch->bbox.max);
    }

    uint32 x2 = x * 2;
    uint32 y2 = y * 2;

    UpdatePatchInfo(level + 1, x2 + 0, y2 + 0);
    UpdatePatchInfo(level + 1, x2 + 1, y2 + 0);
    UpdatePatchInfo(level + 1, x2 + 0, y2 + 1);
    UpdatePatchInfo(level + 1, x2 + 1, y2 + 1);
}

void Landscape::SubdividePatch(uint32 level, uint32 x, uint32 y, uint8 clippingFlags, float32 hError0, float32 rError0)
{
    if (level == subdivLevelCount)
    {
        DVASSERT(false);
        return;
    }

    SubdivisionLevelInfo& levelInfo = subdivLevelInfoArray[level];
    uint32 offset = levelInfo.offset + (y << level) + x;
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

    /*
    float32 geometryRadius = Abs(patch->maxError);
    float32 geometryDistance = Distance(cameraPos, patch->positionOfMaxError);
    float32 geometryError = atanf(geometryRadius / geometryDistance);

    Vector3 max = patch->bbox.max;
    Vector3 min = patch->bbox.min;
    Vector3 origin = patch->bbox.GetCenter();

    float32 distance = Distance(origin, cameraPos);
    float32 radius = Distance(origin, max);
    float32 solidAngle = atanf(radius / distance);
    */

    ////////////////////////////////////////////////////////////////////////////////////

    float32 distance = Distance(cameraPos, patch->positionOfMaxError);
    float32 hError = patch->maxError / (distance * tanFovY);

    Vector3 patchOrigin = patch->bbox.GetCenter();
    float32 patchDistance = Distance(cameraPos, patchOrigin);
    float32 rError = patch->radius / (patchDistance * tanFovY);

    //if ((minSubdivLevelSize > levelInfo.size) || (solidAngle > fovSolidAngleError) || (geometryError > fovGeometryAngleError) || (patch->maxError > fovAbsHeightError))
    //if ((minSubdivLevelSize > levelInfo.size) || (supDistance < patchSize))
    if ((level < subdivLevelCount - 1) && ((minSubdivLevelSize > levelInfo.size) || (maxHeightError <= hError) || (maxPatchRadiusError <= rError)))
    {
        subdivPatchInfo->subdivisionState = SubdivisionPatchInfo::SUBDIVIDED;
        subdivPatchInfo->lastSubdivLevel = level;

        uint32 x2 = x << 1;
        uint32 y2 = y << 1;

        SubdividePatch(level + 1, x2 + 0, y2 + 0, clippingFlags, hError, rError);
        SubdividePatch(level + 1, x2 + 1, y2 + 0, clippingFlags, hError, rError);
        SubdividePatch(level + 1, x2 + 0, y2 + 1, clippingFlags, hError, rError);
        SubdividePatch(level + 1, x2 + 1, y2 + 1, clippingFlags, hError, rError);
    }
    else
    {
        float32 morphAmount = 1.f;
        if (useLodMorphing)
        {
            float32 rError0Rel = rError0 / maxPatchRadiusError;
            float32 rErrorRel = Min(rError, maxPatchRadiusError) / maxPatchRadiusError;

            float32 hError0Rel = hError0 / maxHeightError;
            float32 hErrorRel = Min(hError, maxHeightError) / maxHeightError;

            float32 error0Delta = Max(rError0Rel, hError0Rel) - 1.f;
            float32 errorDelta = 1.f - Max(rErrorRel, hErrorRel);
            morphAmount = 1.f - errorDelta / (error0Delta + errorDelta);
        }

        TerminateSubdivision(level, x, y, level, morphAmount);
        subdivPatchesDrawCount++;
    }
}

void Landscape::TerminateSubdivision(uint32 level, uint32 x, uint32 y, uint32 lastSubdivLevel, float32 morph)
{
    if (level == subdivLevelCount)
    {
        return;
    }

    SubdivisionLevelInfo& levelInfo = subdivLevelInfoArray[level];
    SubdivisionPatchInfo* subdivPatchInfo = &subdivPatchArray[levelInfo.offset + (y << level) + x];

    subdivPatchInfo->subdivMorph = morph;
    subdivPatchInfo->lastSubdivLevel = lastSubdivLevel;
    subdivPatchInfo->subdivisionState = SubdivisionPatchInfo::TERMINATED;

    uint32 x2 = x * 2;
    uint32 y2 = y * 2;

    TerminateSubdivision(level + 1, x2 + 0, y2 + 0, lastSubdivLevel, morph);
    TerminateSubdivision(level + 1, x2 + 1, y2 + 0, lastSubdivLevel, morph);
    TerminateSubdivision(level + 1, x2 + 0, y2 + 1, lastSubdivLevel, morph);
    TerminateSubdivision(level + 1, x2 + 1, y2 + 1, lastSubdivLevel, morph);
}

void Landscape::AddPatchToRender(uint32 level, uint32 x, uint32 y)
{
    DVASSERT(level < subdivLevelCount);

    SubdivisionLevelInfo& levelInfo = subdivLevelInfoArray[level];

    // TODO: optimize all offset calculations in all places
    SubdivisionPatchInfo* subdivPatchInfo = &subdivPatchArray[levelInfo.offset + (y << level) + x];

    uint32 state = subdivPatchInfo->subdivisionState;
    if (state == SubdivisionPatchInfo::CLIPPED)
        return;

    if (state == SubdivisionPatchInfo::SUBDIVIDED)
    {
        uint32 x2 = x * 2;
        uint32 y2 = y * 2;

        AddPatchToRender(level + 1, x2 + 0, y2 + 0);
        AddPatchToRender(level + 1, x2 + 1, y2 + 0);
        AddPatchToRender(level + 1, x2 + 0, y2 + 1);
        AddPatchToRender(level + 1, x2 + 1, y2 + 1);
    }
    else
    {
        if (useInstancing)
        {
            float32 levelf = float32(level);
            Vector4 nearLevel(levelf, levelf, levelf, levelf);

            SubdivisionPatchInfo* nearPatch[4] = {
                GetSubdivPatch(level, x - 1, y),
                GetSubdivPatch(level, x, y - 1),
                GetSubdivPatch(level, x + 1, y),
                GetSubdivPatch(level, x, y + 1)
            };

            if (useLodMorphing)
            {
                float32 morph = subdivPatchInfo->subdivMorph;
                Vector4 nearMorph(morph, morph, morph, morph);

                for (int32 i = 0; i < 4; ++i)
                {
                    SubdivisionPatchInfo* patch = nearPatch[i];
                    if (patch && patch->subdivisionState != SubdivisionPatchInfo::CLIPPED)
                    {
                        if (patch->lastSubdivLevel < level)
                        {
                            nearMorph.data[i] = patch->subdivMorph;
                        }
                        else if (patch->lastSubdivLevel == level && patch->subdivisionState == SubdivisionPatchInfo::TERMINATED)
                        {
                            nearMorph.data[i] = Max(patch->subdivMorph, morph);
                        }

                        nearLevel.data[i] = float32(patch->lastSubdivLevel);
                    }
                }

                DrawPatchInstancing(level, x, y, nearLevel, morph, nearMorph);
            }
            else
            {
                for (int32 i = 0; i < 4; ++i)
                {
                    SubdivisionPatchInfo* patch = nearPatch[i];
                    if (patch && patch->subdivisionState != SubdivisionPatchInfo::CLIPPED)
                    {
                        nearLevel.data[i] = float32(patch->lastSubdivLevel);
                    }
                }

                DrawPatchInstancing(level, x, y, nearLevel);
            }
        }
        else
        {
            SubdivisionPatchInfo* xNeg = GetSubdivPatch(level, x - 1, y);
            SubdivisionPatchInfo* xPos = GetSubdivPatch(level, x + 1, y);
            SubdivisionPatchInfo* yNeg = GetSubdivPatch(level, x, y - 1);
            SubdivisionPatchInfo* yPos = GetSubdivPatch(level, x, y + 1);

            uint32 xNegSizePow2 = level;
            uint32 xPosSizePow2 = level;
            uint32 yNegSizePow2 = level;
            uint32 yPosSizePow2 = level;

            if (xNeg && xNeg->subdivisionState != SubdivisionPatchInfo::CLIPPED)
                xNegSizePow2 = xNeg->lastSubdivLevel;
            if (xPos && xPos->subdivisionState != SubdivisionPatchInfo::CLIPPED)
                xPosSizePow2 = xPos->lastSubdivLevel;
            if (yNeg && yNeg->subdivisionState != SubdivisionPatchInfo::CLIPPED)
                yNegSizePow2 = yNeg->lastSubdivLevel;
            if (yPos && yPos->subdivisionState != SubdivisionPatchInfo::CLIPPED)
                yPosSizePow2 = yPos->lastSubdivLevel;

            DrawPatchNoInstancing(level, x, y, xNegSizePow2, xPosSizePow2, yNegSizePow2, yPosSizePow2);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////Non-instancing render

void Landscape::AllocateGeometryDataNoInstancing()
{
    isRequireTangentBasis = (QualitySettingsSystem::Instance()->GetCurMaterialQuality(LANDSCAPE_QUALITY_NAME) == LANDSCAPE_QUALITY_VALUE_HIGH);

    rhi::VertexLayout vLayout;
    vLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    vLayout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2);
    if (isRequireTangentBasis)
    {
        vLayout.AddElement(rhi::VS_NORMAL, 0, rhi::VDT_FLOAT, 3);
        vLayout.AddElement(rhi::VS_TANGENT, 0, rhi::VDT_FLOAT, 3);
    }
    vLayoutUIDNoInstancing = rhi::VertexLayout::UniqueId(vLayout);

    for (int32 i = 0; i < LANDSCAPE_BATCHES_POOL_SIZE; i++)
    {
        AllocateRenderBatch();
    }

    indices.resize(INITIAL_INDEX_BUFFER_CAPACITY);

    uint32 quadsInWidth = heightmap->Size() / RENDER_PARCEL_SIZE_QUADS;
    // For cases where landscape is very small allocate 1 VBO.
    if (quadsInWidth == 0)
        quadsInWidth = 1;

    quadsInWidthPow2 = HighestBitIndex(quadsInWidth);

    for (uint32 y = 0; y < quadsInWidth; ++y)
    {
        for (uint32 x = 0; x < quadsInWidth; ++x)
        {
            uint16 check = AllocateParcelVertexBuffer(x * RENDER_PARCEL_SIZE_QUADS, y * RENDER_PARCEL_SIZE_QUADS, RENDER_PARCEL_SIZE_QUADS);
            DVASSERT(check == (uint16)(x + y * quadsInWidth));
        }
    }
}

void Landscape::AllocateRenderBatch()
{
    ScopedPtr<RenderBatch> batch(new RenderBatch());
    AddRenderBatch(batch);

    batch->SetMaterial(landscapeMaterial);
    batch->SetSortingKey(10);

    batch->vertexLayoutId = vLayoutUIDNoInstancing;
    batch->vertexCount = RENDER_PARCEL_SIZE_VERTICES * RENDER_PARCEL_SIZE_VERTICES;
}

int16 Landscape::AllocateParcelVertexBuffer(uint32 quadX, uint32 quadY, uint32 quadSize)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    uint32 verticesCount = (quadSize + 1) * (quadSize + 1);
    uint32 vertexSize = sizeof(VertexNoInstancing);
    if (!isRequireTangentBasis)
    {
        vertexSize -= sizeof(Vector3); // (Vertex::normal);
        vertexSize -= sizeof(Vector3); // (Vertex::tangent);
    }

    uint8* landscapeVertices = new uint8[verticesCount * vertexSize];
    uint32 index = 0;
    for (uint32 y = quadY; y < quadY + quadSize + 1; ++y)
    {
        for (uint32 x = quadX; x < quadX + quadSize + 1; ++x)
        {
            VertexNoInstancing* vertex = reinterpret_cast<VertexNoInstancing*>(&landscapeVertices[index * vertexSize]);
            vertex->position = GetPoint(x, y, heightmap->GetHeightClamp(x, y));

            Vector2 texCoord = Vector2((float32)(x) / (float32)(heightmap->Size()), 1.0f - (float32)(y) / (float32)(heightmap->Size()));
            vertex->texCoord = texCoord;

            if (isRequireTangentBasis)
            {
                //VI: calculate normal for the point.
                uint32 xx = x + 1;
                uint32 yy = y + 1;
                Vector3 right = GetPoint(xx, y, heightmap->GetHeightClamp(xx, y));
                Vector3 bottom = GetPoint(x, yy, heightmap->GetHeightClamp(x, yy));

                xx = Min(x, x - 1);
                yy = Min(y, y - 1);
                Vector3 left = GetPoint(xx, y, heightmap->GetHeightClamp(xx, y));
                Vector3 top = GetPoint(x, yy, heightmap->GetHeightClamp(x, yy));

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
    bufferRestoreData.push_back({ vertexBuffer, landscapeVertices, vBufferSize, RestoreBufferData::RESTORE_BUFFER_VERTEX });
#endif

    return (int16)(vertexBuffers.size() - 1);
}

void Landscape::DrawLandscapeNoInstancing()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    drawIndices = 0;
    flushQueueCounter = 0;
    activeRenderBatchArray.clear();

    ClearQueue();
    AddPatchToRender(0, 0, 0);
    FlushQueue();
}

void Landscape::DrawPatchNoInstancing(uint32 level, uint32 xx, uint32 yy, uint32 xNegSizePow2, uint32 xPosSizePow2, uint32 yNegSizePow2, uint32 yPosSizePow2)
{
    SubdivisionLevelInfo& levelInfo = subdivLevelInfoArray[level];
    PatchQuadInfo* patch = &patchQuadArray[levelInfo.offset + (yy << level) + xx];

    int32 dividerPow2 = level - quadsInWidthPow2;
    DVASSERT(dividerPow2 >= 0);
    uint32 quadBuffer = ((yy >> dividerPow2) << quadsInWidthPow2) + (xx >> dividerPow2);

    if ((quadBuffer != queuedQuadBuffer) && (queuedQuadBuffer != -1))
    {
        FlushQueue();
    }

    queuedQuadBuffer = quadBuffer;

    // Draw Middle
    uint32 realVertexCountInPatch = heightmap->Size() >> level;
    uint32 step = realVertexCountInPatch / PATCH_SIZE_QUADS;
    uint32 heightMapStartX = xx * realVertexCountInPatch;
    uint32 heightMapStartY = yy * realVertexCountInPatch;

    ResizeIndicesBufferIfNeeded(queueIndexCount + PATCH_SIZE_QUADS * PATCH_SIZE_QUADS * 6);

    uint16* indicesPtr = indices.data() + queueIndexCount;
    // Draw middle block
    {
        for (uint16 y = (heightMapStartY & RENDER_PARCEL_AND); y < (heightMapStartY & RENDER_PARCEL_AND) + realVertexCountInPatch; y += step)
        {
            for (uint16 x = (heightMapStartX & RENDER_PARCEL_AND); x < (heightMapStartX & RENDER_PARCEL_AND) + realVertexCountInPatch; x += step)
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

                if (x == (heightMapStartX & RENDER_PARCEL_AND))
                {
                    uint16 alignMod = levelInfo.size >> xNegSizePow2;
                    if (alignMod > 1)
                    {
                        y0aligned = y0 / (alignMod * step) * (alignMod * step);
                        y1aligned = y1 / (alignMod * step) * (alignMod * step);
                    }
                }

                if (y == (heightMapStartY & RENDER_PARCEL_AND))
                {
                    uint16 alignMod = levelInfo.size >> yNegSizePow2;
                    if (alignMod > 1)
                    {
                        x0aligned = x0 / (alignMod * step) * (alignMod * step);
                        x1aligned = x1 / (alignMod * step) * (alignMod * step);
                    }
                }

                if (x == ((heightMapStartX & RENDER_PARCEL_AND) + realVertexCountInPatch - step))
                {
                    uint16 alignMod = levelInfo.size >> xPosSizePow2;
                    if (alignMod > 1)
                    {
                        y0aligned2 = y0 / (alignMod * step) * (alignMod * step);
                        y1aligned2 = y1 / (alignMod * step) * (alignMod * step);
                    }
                }

                if (y == ((heightMapStartY & RENDER_PARCEL_AND) + realVertexCountInPatch - step))
                {
                    uint16 alignMod = levelInfo.size >> yPosSizePow2;
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

    DVASSERT(queuedQuadBuffer != -1);

    DynamicBufferAllocator::AllocResultIB indexBuffer = DynamicBufferAllocator::AllocateIndexBuffer(queueIndexCount);
    DVASSERT(indexBuffer.allocatedindices == queueIndexCount);

    Memcpy(indexBuffer.data, indices.data(), queueIndexCount * sizeof(uint16));
    RenderBatch* batch = renderBatchArray[flushQueueCounter].renderBatch;
    batch->indexBuffer = indexBuffer.buffer;
    batch->indexCount = queueIndexCount;
    batch->startIndex = indexBuffer.baseIndex;
    batch->vertexBuffer = vertexBuffers[queuedQuadBuffer];

    activeRenderBatchArray.push_back(batch);
    ClearQueue();

    drawIndices += batch->indexCount;
    ++flushQueueCounter;
}

void Landscape::ClearQueue()
{
    queueIndexCount = 0;
    queuedQuadBuffer = -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////Instancing render

void Landscape::AllocateGeometryDataInstancing()
{
    ScopedPtr<Texture> heightTexture(CreateHeightTexture(heightmap));
    if (landscapeMaterial->HasLocalTexture(NMaterialTextureName::TEXTURE_HEIGHTMAP))
        landscapeMaterial->SetTexture(NMaterialTextureName::TEXTURE_HEIGHTMAP, heightTexture);
    else
        landscapeMaterial->AddTexture(NMaterialTextureName::TEXTURE_HEIGHTMAP, heightTexture);

    /////////////////////////////////////////////////////////////////

    const uint32 VERTICES_COUNT = PATCH_SIZE_VERTICES * PATCH_SIZE_VERTICES;
    const uint32 INDICES_COUNT = PATCH_SIZE_QUADS * PATCH_SIZE_QUADS * 6;

    VertexInstancing* patchVertices = new VertexInstancing[VERTICES_COUNT];
    uint16* patchIndices = new uint16[INDICES_COUNT];
    uint16* indicesPtr = patchIndices;

    float32 quadSize = 1.f / PATCH_SIZE_QUADS;

    for (uint32 y = 0; y < PATCH_SIZE_VERTICES; ++y)
    {
        for (uint32 x = 0; x < PATCH_SIZE_VERTICES; ++x)
        {
            VertexInstancing& vertex = patchVertices[y * PATCH_SIZE_VERTICES + x];
            vertex.position = Vector2(x * quadSize, y * quadSize);
            vertex.edgeMask = Vector4(0.f, 0.f, 0.f, 0.f);
            vertex.edgeShiftDirection = Vector2(0.f, 0.f);
            vertex.edgeVertexIndex = 0.f;
            vertex.edgeMaskNull = 1.f;

            if (x < (PATCH_SIZE_VERTICES - 1) && y < (PATCH_SIZE_VERTICES - 1))
            {
                *indicesPtr++ = (y + 0) * PATCH_SIZE_VERTICES + (x + 0);
                *indicesPtr++ = (y + 1) * PATCH_SIZE_VERTICES + (x + 1);
                *indicesPtr++ = (y + 1) * PATCH_SIZE_VERTICES + (x + 0);

                *indicesPtr++ = (y + 0) * PATCH_SIZE_VERTICES + (x + 0);
                *indicesPtr++ = (y + 0) * PATCH_SIZE_VERTICES + (x + 1);
                *indicesPtr++ = (y + 1) * PATCH_SIZE_VERTICES + (x + 1);
            }
        }
    }

    for (uint32 i = 1; i < PATCH_SIZE_QUADS; ++i)
    {
        //x = 0; y = i; left side of patch without corners
        patchVertices[i * PATCH_SIZE_VERTICES].edgeMask = Vector4(1.f, 0.f, 0.f, 0.f);
        patchVertices[i * PATCH_SIZE_VERTICES].edgeShiftDirection = Vector2(0.f, 1.f) / PATCH_SIZE_QUADS;
        patchVertices[i * PATCH_SIZE_VERTICES].edgeVertexIndex = float32(PATCH_SIZE_QUADS - i);
        patchVertices[i * PATCH_SIZE_VERTICES].edgeMaskNull = 0.f;

        //x = i; y = 0; bottom side of patch without corners
        patchVertices[i].edgeMask = Vector4(0.f, 1.f, 0.f, 0.f);
        patchVertices[i].edgeShiftDirection = Vector2(1.f, 0.f) / PATCH_SIZE_QUADS;
        patchVertices[i].edgeVertexIndex = float32(PATCH_SIZE_QUADS - i);
        patchVertices[i].edgeMaskNull = 0.f;

        //x = PATCH_QUAD_COUNT; y = i; right side of patch without corners
        patchVertices[i * PATCH_SIZE_VERTICES + PATCH_SIZE_QUADS].edgeMask = Vector4(0.f, 0.f, 1.f, 0.f);
        patchVertices[i * PATCH_SIZE_VERTICES + PATCH_SIZE_QUADS].edgeShiftDirection = Vector2(0.f, -1.f) / PATCH_SIZE_QUADS;
        patchVertices[i * PATCH_SIZE_VERTICES + PATCH_SIZE_QUADS].edgeVertexIndex = float32(i);
        patchVertices[i * PATCH_SIZE_VERTICES + PATCH_SIZE_QUADS].edgeMaskNull = 0.f;

        //x = i; y = PATCH_QUAD_COUNT; top side of patch without corners
        patchVertices[PATCH_SIZE_QUADS * PATCH_SIZE_VERTICES + i].edgeMask = Vector4(0.f, 0.f, 0.f, 1.f);
        patchVertices[PATCH_SIZE_QUADS * PATCH_SIZE_VERTICES + i].edgeShiftDirection = Vector2(-1.f, 0.f) / PATCH_SIZE_QUADS;
        patchVertices[PATCH_SIZE_QUADS * PATCH_SIZE_VERTICES + i].edgeVertexIndex = float32(i);
        patchVertices[PATCH_SIZE_QUADS * PATCH_SIZE_VERTICES + i].edgeMaskNull = 0.f;
    }

    /////////////////////////////////////////////////////////////////

    rhi::VertexBuffer::Descriptor vdesc;
    vdesc.size = VERTICES_COUNT * sizeof(VertexInstancing);
    vdesc.initialData = patchVertices;
    vdesc.usage = rhi::USAGE_STATICDRAW;
    patchVertexBuffer = rhi::CreateVertexBuffer(vdesc);

    rhi::IndexBuffer::Descriptor idesc;
    idesc.size = INDICES_COUNT * sizeof(uint16);
    idesc.initialData = patchIndices;
    idesc.usage = rhi::USAGE_STATICDRAW;
    patchIndexBuffer = rhi::CreateIndexBuffer(idesc);

#if defined(__DAVAENGINE_IPHONE__)
    SafeDeleteArray(patchVertices);
    SafeDeleteArray(patchIndices);
#else
    bufferRestoreData.push_back({ patchVertexBuffer, reinterpret_cast<uint8*>(patchVertices), vdesc.size, RestoreBufferData::RESTORE_BUFFER_VERTEX });
    bufferRestoreData.push_back({ patchIndexBuffer, reinterpret_cast<uint8*>(patchIndices), idesc.size, RestoreBufferData::RESTORE_BUFFER_INDEX });
#endif

    RenderBatch* batch = new RenderBatch();
    batch->SetMaterial(landscapeMaterial);
    batch->SetSortingKey(10);
    batch->vertexBuffer = patchVertexBuffer;
    batch->indexBuffer = patchIndexBuffer;
    batch->primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
    batch->indexCount = INDICES_COUNT;
    batch->vertexCount = VERTICES_COUNT;

    rhi::VertexLayout vLayout;
    vLayout.AddStream(rhi::VDF_PER_VERTEX);
    vLayout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 4); //position + gluDirection
    vLayout.AddElement(rhi::VS_TEXCOORD, 1, rhi::VDT_FLOAT, 4); //edge mask
    vLayout.AddElement(rhi::VS_TEXCOORD, 2, rhi::VDT_FLOAT, 2); //vertex index + edgeMaskNull
    vLayout.AddStream(rhi::VDF_PER_INSTANCE);
    vLayout.AddElement(rhi::VS_TEXCOORD, 3, rhi::VDT_FLOAT, 3); //patch position + scale
    vLayout.AddElement(rhi::VS_TEXCOORD, 4, rhi::VDT_FLOAT, 4); //near patch lodOffset
    if (useLodMorphing)
    {
        vLayout.AddElement(rhi::VS_TEXCOORD, 5, rhi::VDT_FLOAT, 4); //near patch morph
        vLayout.AddElement(rhi::VS_TEXCOORD, 6, rhi::VDT_FLOAT, 3); //patch lod + morph + pixelMappingOffset
    }

    batch->vertexLayoutId = rhi::VertexLayout::UniqueId(vLayout);

    AddRenderBatch(batch);
    SafeRelease(batch);
}

void Landscape::DrawLandscapeInstancing()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    drawIndices = 0;
    activeRenderBatchArray.clear();

    for (int32 i = static_cast<int32>(usedInstanceDataBuffers.size()) - 1; i >= 0; --i)
    {
        if (rhi::SyncObjectSignaled(usedInstanceDataBuffers[i]->syncObject))
        {
            freeInstanceDataBuffers.push_back(usedInstanceDataBuffers[i]);
            RemoveExchangingWithLast(usedInstanceDataBuffers, i);
        }
    }

    if (subdivPatchesDrawCount)
    {
        InstanceDataBuffer* instanceDataBuffer = nullptr;
        if (freeInstanceDataBuffers.size())
        {
            instanceDataBuffer = freeInstanceDataBuffers.back();
            if (instanceDataBuffer->bufferSize < subdivPatchesDrawCount * instanceDataSize)
            {
                instanceDataMaxCount = Max(instanceDataMaxCount, subdivPatchesDrawCount);

                rhi::DeleteVertexBuffer(instanceDataBuffer->buffer);
                SafeDelete(instanceDataBuffer);
            }
            freeInstanceDataBuffers.pop_back();
        }

        if (!instanceDataBuffer)
        {
            rhi::VertexBuffer::Descriptor instanceBufferDesc;
            instanceBufferDesc.size = instanceDataMaxCount * instanceDataSize;
            instanceBufferDesc.usage = rhi::USAGE_DYNAMICDRAW;

            instanceDataBuffer = new InstanceDataBuffer();
            instanceDataBuffer->bufferSize = instanceBufferDesc.size;
            instanceDataBuffer->buffer = rhi::CreateVertexBuffer(instanceBufferDesc);
        }
        usedInstanceDataBuffers.push_back(instanceDataBuffer);
        instanceDataBuffer->syncObject = rhi::GetCurrentFrameSyncObject();

        renderBatchArray[0].renderBatch->instanceBuffer = instanceDataBuffer->buffer;
        renderBatchArray[0].renderBatch->instanceCount = subdivPatchesDrawCount;
        activeRenderBatchArray.push_back(renderBatchArray[0].renderBatch);

        instanceDataPtr = static_cast<uint8*>(rhi::MapVertexBuffer(instanceDataBuffer->buffer, 0, subdivPatchesDrawCount * instanceDataSize));

        AddPatchToRender(0, 0, 0);

        rhi::UnmapVertexBuffer(instanceDataBuffer->buffer);
        instanceDataPtr = nullptr;

        drawIndices = activeRenderBatchArray[0]->indexCount * activeRenderBatchArray[0]->instanceCount;
    }
}

void Landscape::DrawPatchInstancing(uint32 level, uint32 xx, uint32 yy, const Vector4& nearLevel, float32 patchMorph /* = 0.f */, const Vector4& nearMorph /* = Vector4() */)
{
    SubdivisionLevelInfo& levelInfo = subdivLevelInfoArray[level];
    PatchQuadInfo* patch = &patchQuadArray[levelInfo.offset + (yy << level) + xx];

    float32 levelf = float32(level);
    InstanceData* instanceData = reinterpret_cast<InstanceData*>(instanceDataPtr);

    instanceData->patchOffset = Vector2(float32(xx) / levelInfo.size, float32(yy) / levelInfo.size);
    instanceData->patchScale = 1.f / levelInfo.size;
    instanceData->nearPatchLodOffset = Vector4(levelf - nearLevel.x,
                                               levelf - nearLevel.y,
                                               levelf - nearLevel.z,
                                               levelf - nearLevel.w);

    if (useLodMorphing)
    {
        int32 baseLod = subdivLevelCount - level - 1;

        instanceData->nearPatchMorph = nearMorph;
        instanceData->patchLod = float32(baseLod);
        instanceData->patchMorph = patchMorph;
        instanceData->centerPixelOffset = .5f / (1 << (heightmapSizePow2 - baseLod));
    }

    instanceDataPtr += instanceDataSize;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

void Landscape::BindDynamicParameters(Camera* camera)
{
    RenderObject::BindDynamicParameters(camera);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_WORLD, &Matrix4::IDENTITY, (pointer_size)&Matrix4::IDENTITY);
}

void Landscape::PrepareToRender(Camera* camera)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    RenderObject::PrepareToRender(camera);

    TIME_PROFILE("Landscape.PrepareToRender");

    if (!subdivLevelCount || !Renderer::GetOptions()->IsOptionEnabled(RenderOptions::LANDSCAPE_DRAW))
    {
        return;
    }

    if (Renderer::GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_LANDSCAPE_LODS))
    {
        camera = GetRenderSystem()->GetMainCamera();
        frustum = camera->GetFrustum();
        cameraPos = camera->GetPosition();

        /*
        float32 fovLerp = Clamp((camera->GetFOV() - zoomFov) / (normalFov - zoomFov), 0.0f, 1.0f);
        fovSolidAngleError = zoomSolidAngleError + (solidAngleError - zoomSolidAngleError) * fovLerp;
        fovGeometryAngleError = zoomGeometryAngleError + (geometryAngleError - zoomGeometryAngleError) * fovLerp;
        fovAbsHeightError = zoomAbsHeightError + (absHeightError - zoomAbsHeightError) * fovLerp;
        */

        tanFovY = tanf(camera->GetFOV() * PI / 360.f) * camera->GetAspect();

        subdivPatchesDrawCount = 0;
        SubdividePatch(0, 0, 0, 0x3f, maxHeightError, maxPatchRadiusError);
    }

    if (useInstancing)
        DrawLandscapeInstancing();
    else
        DrawLandscapeNoInstancing();
}

bool Landscape::GetLevel0Geometry(Vector<LandscapeVertex>& vertices, Vector<int32>& indices) const
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();
    if (heightmap->Data() == nullptr)
    {
        return false;
    }

    uint32 gridWidth = heightmap->Size() + 1;
    uint32 gridHeight = heightmap->Size() + 1;
    vertices.resize(gridWidth * gridHeight);
    for (uint32 y = 0, index = 0; y < gridHeight; ++y)
    {
        float32 ny = static_cast<float32>(y) / static_cast<float32>(gridHeight - 1);
        for (uint32 x = 0; x < gridWidth; ++x)
        {
            float32 nx = static_cast<float32>(x) / static_cast<float32>(gridWidth - 1);
            vertices[index].position = GetPoint(x, y, heightmap->GetHeightClamp(x, y));
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

const FilePath& Landscape::GetHeightmapPathname()
{
    return heightmapPath;
}

void Landscape::SetHeightmapPathname(const FilePath& newHeightMapPath)
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

void Landscape::SetLandscapeSize(const Vector3& newLandscapeSize)
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
    bbox.AddPoint(Vector3(-newLandscapeSize.x / 2.f, -newLandscapeSize.y / 2.f, 0.f));
    bbox.AddPoint(Vector3(newLandscapeSize.x / 2.f, newLandscapeSize.y / 2.f, newLandscapeSize.z));
    RebuildLandscape();

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

void Landscape::Save(KeyedArchive* archive, SerializationContext* serializationContext)
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

void Landscape::Load(KeyedArchive* archive, SerializationContext* serializationContext)
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
    NMaterial* material = static_cast<NMaterial*>(serializationContext->GetDataBlock(matKey));
    if (material)
    {
        //Import old params
        if (!material->HasLocalProperty(NMaterialParamName::PARAM_LANDSCAPE_TEXTURE_TILING))
        {
            if (archive->IsKeyExists("tiling_0"))
            {
                Vector2 tilingValue;
                tilingValue = archive->GetByteArrayAsType("tiling_0", tilingValue);
                material->AddProperty(NMaterialParamName::PARAM_LANDSCAPE_TEXTURE_TILING, tilingValue.data, rhi::ShaderProp::TYPE_FLOAT2);
            }
            else if (material->HasLocalProperty(NMaterialParamName::DEPRECATED_LANDSCAPE_TEXTURE_0_TILING))
            {
                material->AddProperty(NMaterialParamName::PARAM_LANDSCAPE_TEXTURE_TILING, material->GetLocalPropValue(NMaterialParamName::DEPRECATED_LANDSCAPE_TEXTURE_0_TILING), rhi::ShaderProp::TYPE_FLOAT2);
                for (int32 i = 0; i < 4; i++)
                {
                    FastName propName(Format("texture%dTiling", i));
                    if (material->HasLocalProperty(propName))
                        material->RemoveProperty(propName);
                }
            }
        }

        material->PreBuildMaterial(PASS_FORWARD);

        SetMaterial(material);
    }

    FilePath heightmapPath = serializationContext->GetScenePath() + archive->GetString("hmap");
    AABBox3 loadedBbox = archive->GetByteArrayAsType("bbox", AABBox3());

    BuildLandscapeFromHeightmapImage(heightmapPath, loadedBbox);
}

Heightmap* Landscape::GetHeightmap()
{
    return heightmap;
}

void Landscape::SetHeightmap(DAVA::Heightmap* height)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    SafeRelease(heightmap);
    heightmap = SafeRetain(height);

    RebuildLandscape();
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

    Landscape* newLandscape = static_cast<Landscape*>(newObject);

    RefPtr<NMaterial> material(landscapeMaterial->Clone());
    newLandscape->SetMaterial(material.Get());

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

void Landscape::SetForceFirstLod(bool force)
{
    forceFirstLod = force;
}

void Landscape::SetUpdatable(bool isUpdatable)
{
    if (updatable != isUpdatable)
    {
        updatable = isUpdatable;
        RebuildLandscape();
    }
}

bool Landscape::IsUpdatable() const
{
    return updatable;
}

void Landscape::SetDebugDraw(bool isDebug)
{
    landscapeMaterial->SetFXName(isDebug ? NMaterialName::TILE_MASK_DEBUG : NMaterialName::TILE_MASK);
}

bool Landscape::IsDebugDraw() const
{
    return landscapeMaterial->GetEffectiveFXName() == NMaterialName::TILE_MASK_DEBUG;
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
    DVASSERT(!useInstancing);
    /*
    int32 heightmapSize = fromHeightmap->Size();
    DVASSERT(heightmap->Size() == heightmapSize);

    DVASSERT(!isRequireTangentBasis && "TODO: Landscape::UpdatePart() for HIGH Quality");

    uint32 vertexSize = sizeof(VertexNoInstancing);
    if (!isRequireTangentBasis)
    {
        vertexSize -= sizeof(Vector3); // (Vertex::normal);
        vertexSize -= sizeof(Vector3); // (Vertex::tangent);
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
                        VertexNoInstancing* vertex = reinterpret_cast<VertexNoInstancing*>(&quadVertices[index * vertexSize]);

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
    */
}
}
