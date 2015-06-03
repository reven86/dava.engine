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


#include "Render/Highlevel/Landscape.h"
#include "Render/Image/Image.h"
#include "Render/Image/ImageSystem.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"
#include "Render/RenderDataObject.h"
#include "Render/Texture.h"
#include "Scene3D/Scene.h"
#include "Render/Shader.h"
#include "Platform/SystemTimer.h"
#include "Utils/StringFormat.h"
#include "Scene3D/SceneFileV2.h"
#include "Render/Highlevel/Heightmap.h"
#include "FileSystem/FileSystem.h"
#include "Render/TextureDescriptor.h"
#include "LandscapeChunk.h"
#include "Debug/Stats.h"
#include "Render/Material/NMaterial.h"
#include "Scene3D/Systems/MaterialSystem.h"
#include "Scene3D/Systems/FoliageSystem.h"
#include "Render/Material/NMaterialNames.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"

namespace DAVA
{
    
/*
    TODO (for next iteration of landscape rendering)
    - Error metric computation using GPU, now it takes about 1 second on CPU
    - Instancing version for high end devices
    - Vertex morphing for high end devices
    - Remove old functions like SetTexture and etc
    - Optimise SubdividePatch function (can be executed, not every render, we can calc 1/4 or 1/16 per frame)
    - Change cursor rendering shader (can be done in landscape shader, instead of using of texture)
    - Incremental update of index-buffer for low-end devices, instead of per-frame update. Only changed patches can be replaced.
 */

const FastName Landscape::PARAM_CAMERA_POSITION("cameraPosition");
const FastName Landscape::PARAM_TEXTURE0_TILING("texture0Tiling");
const FastName Landscape::PARAM_TEXTURE1_TILING("texture1Tiling");
const FastName Landscape::PARAM_TEXTURE2_TILING("texture2Tiling");
const FastName Landscape::PARAM_TEXTURE3_TILING("texture3Tiling");
const FastName Landscape::PARAM_TILE_COLOR0("tileColor0");
const FastName Landscape::PARAM_TILE_COLOR1("tileColor1");
const FastName Landscape::PARAM_TILE_COLOR2("tileColor2");
const FastName Landscape::PARAM_TILE_COLOR3("tileColor3");
const FastName Landscape::PARAM_PROP_SPECULAR_COLOR("prop_specularColor");
const FastName Landscape::PARAM_SPECULAR_SHININESS("materialSpecularShininess");
const FastName Landscape::TEXTURE_SPECULAR_MAP("specularMap");
const FastName Landscape::TECHNIQUE_TILEMASK_NAME("ForwardPass");
	   
const FastName INVALID_PROPERTY_NAME = FastName("");
	
static FastName TILEMASK_TEXTURE_PROPS_NAMES[] =
{
	FastName("colorTexture"),
	FastName("tileMask"),
	FastName("tileTexture0"),
	FastName("tileTexture1"),
	FastName("tileTexture2"),
	FastName("tileTexture3"),
    FastName("textureDetail"),
    FastName("textureTileFull"),
};

static FastName TILEMASK_TILING_PROPS_NAMES[] =
{
	INVALID_PROPERTY_NAME,
	INVALID_PROPERTY_NAME,
	FastName("texture0Tiling"),
	FastName("texture1Tiling"),
	FastName("texture2Tiling"),
	FastName("texture3Tiling"),
	INVALID_PROPERTY_NAME,
	INVALID_PROPERTY_NAME
};

static FastName TILEMASK_COLOR_PROPS_NAMES[] =
{
	INVALID_PROPERTY_NAME,
	INVALID_PROPERTY_NAME,
	FastName("tileColor0"),
	FastName("tileColor1"),
	FastName("tileColor2"),
	FastName("tileColor3"),
	INVALID_PROPERTY_NAME,
	INVALID_PROPERTY_NAME
};

	
Landscape::Landscape()
    : indices(0)
    , camera(nullptr)
    , tileMaskMaterial(nullptr)
    , foliageSystem(nullptr)
    , LANDSCAPE_QUALITY_NAME("Landscape")
    , LANDSCAPE_QUALITY_VALUE_HIGH("HIGH")
{
	drawIndices = 0;

    subdivPatchArray = 0;
    patchQuadArray = 0;
    rdoArray = 0;

    frustum = 0; //new Frustum();
    cursor = 0;
    type = TYPE_LANDSCAPE;
    heightmap = new Heightmap();
    
    solidAngleError = (20.0f * PI / 180.0f);
    geometryAngleError = (1.0f * PI / 180.0f);
    absHeightError = 3.0f;
    
    zoomSolidAngleError = solidAngleError;
    zoomGeometryAngleError = geometryAngleError;
    zoomAbsHeightError = 0.5f;
    
    zoomFov = 6.5f;
    normalFov = 70.0f;
    
    isDebugDraw = false;
    isRequireTangentBasis = false;
}

Landscape::~Landscape()
{
    ReleaseLandscape();

    SafeRelease(heightmap);
	SafeDelete(cursor);
	SafeRelease(tileMaskMaterial);
}

int16 Landscape::AllocateRDOQuad(LandscapeQuad * quad)
{
//    Logger::FrameworkDebug("AllocateRDOQuad: %d %d size: %d", quad->x, quad->y, quad->size);
    DVASSERT(quad->size <= RENDER_QUAD_WIDTH - 1);
    
    uint32 vertexSize = sizeof(LandscapeVertex);
    if (!isRequireTangentBasis)
    {
        vertexSize -= sizeof(Vector3); // (LandscapeVertex::normal);
        vertexSize -= sizeof(Vector3); // (LandscapeVertex::tangent);
    }
    uint8 * landscapeVertices = new uint8[(quad->size + 1) * (quad->size + 1) * vertexSize];
    
    int32 index = 0;
    for (int32 y = quad->y; y < quad->y + quad->size + 1; ++y)
        for (int32 x = quad->x; x < quad->x + quad->size + 1; ++x)
        {
            LandscapeVertex * vertex = reinterpret_cast<LandscapeVertex *>(&landscapeVertices[index * vertexSize]);
            
            vertex->position = GetPoint(x, y, heightmap->Data()[y * heightmap->Size() + x]);

            Vector2 texCoord = Vector2((float32)(x) / (float32)(heightmap->Size() - 1), 1.0f - (float32)(y) / (float32)(heightmap->Size() - 1));
            vertex->texCoord = texCoord;
			
            if (landscapeQuality == LANDSCAPE_QUALITY_VALUE_HIGH)
            {
                //VI: calculate normal for the point.
                uint32 xx = 0;
                uint32 yy = 0;
                
                xx = (x < heightmap->Size() - 1) ? x + 1 : x;
                Vector3 right = GetPoint(xx, y, heightmap->Data()[y * heightmap->Size() + xx]);
                
                xx = (x > 0) ? x - 1 : x;
                Vector3 left = GetPoint(xx, y, heightmap->Data()[y * heightmap->Size() + xx]);
                
                yy = (y < heightmap->Size() - 1) ? y + 1 : y;
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
            }
            
            /*
                VS: Algorithm
                 // # P.xy store the position for which we want to calculate the normals
                 // # height() here is a function that return the height at a point in the terrain
                 
                 // read neightbor heights using an arbitrary small offset
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
            index++;
        }
    
    // setup a base RDO
    RenderDataObject * landscapeRDO = new RenderDataObject();

    LandscapeVertex * startStreamPtr = (LandscapeVertex *)landscapeVertices;
    landscapeRDO->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, vertexSize, &startStreamPtr[0].position);
    landscapeRDO->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, vertexSize, &startStreamPtr[0].texCoord);
	
    // TODO: Objects should be notified about quality change, and should be able to react
    if (landscapeQuality == LANDSCAPE_QUALITY_VALUE_HIGH)
    {
        landscapeRDO->SetStream(EVF_NORMAL, TYPE_FLOAT, 3, vertexSize, &startStreamPtr[0].normal);
        landscapeRDO->SetStream(EVF_TANGENT, TYPE_FLOAT, 3, vertexSize, &startStreamPtr[0].tangent);
    }
        
    landscapeRDO->BuildVertexBuffer((quad->size + 1) * (quad->size + 1), BDT_STATIC_DRAW, true);
#if defined(__DAVAENGINE_IPHONE__)
    SafeDeleteArray(landscapeVertices);
#endif
    
    landscapeVerticesArray.push_back(landscapeVertices);
    landscapeRDOArray.push_back(landscapeRDO);
    
//    Logger::FrameworkDebug("Allocated vertices: %d KB", sizeof(LandscapeVertex) * (quad->size + 1) * (quad->size + 1) / 1024);
    
    return (int16)landscapeRDOArray.size() - 1;
}

void Landscape::BuildLandscapeFromHeightmapImage(const FilePath & heightmapPathname, const AABBox3 & _box)
{
	heightmapPath = heightmapPathname;
    BuildHeightmap();

	bbox = _box;

    ReallocateLandscape();
    
    if(foliageSystem)
    {
        foliageSystem->SyncFoliageWithLandscape();
    }
}

bool Landscape::BuildHeightmap()
{
    bool retValue = false;

    if(DAVA::TextureDescriptor::IsSourceTextureExtension(heightmapPath.GetExtension()))
    {
        Vector<Image *> imageSet;
        ImageSystem::Instance()->Load(heightmapPath, imageSet);
        if(0 != imageSet.size())
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
    else if(heightmapPath.IsEqualToExtension(Heightmap::FileExtension()))
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

    
Landscape::SubdivisionPatchInfo * Landscape::GetSubdivPatch(uint32 level, uint32 x, uint32 y)
{
    SubdivisionLevelInfo & levelInfo = subdivLevelInfoArray[level];
    
    if (x < levelInfo.size && y < levelInfo.size)
        return &subdivPatchArray[levelInfo.offset + levelInfo.size * y + x];
    else return 0;
}
    
void Landscape::UpdatePatchInfo(uint32 level, uint32 x, uint32 y)
{
    if (level >= subdivLevelCount)return;
    
    SubdivisionLevelInfo & levelInfo = subdivLevelInfoArray[level];
    PatchQuadInfo * patch = &patchQuadArray[levelInfo.offset + levelInfo.size * y + x];
    
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
            for (uint16 yy = heightMapStartY; yy <= heightMapStartY + realQuadCountInPatch; ++yy)
            {
                uint16 value = data[heightmap->Size() * yy + xx];
                Vector3 pos = GetPoint(xx, yy, value);
                patch->bbox.AddPoint(pos);

                //float32 xt = ((float)xx - (float)heightMapStartX) / realQuadCountInPatch;
                //float32 yt = ((float)yy - (float)heightMapStartY) / realQuadCountInPatch;

                if (patchMod == 1)continue;
                if ((xx % patchMod) == 0)continue;
                if ((yy % patchMod) == 0)continue;
                
                {
                    uint16 x0 = (xx / (patchMod)) * patchMod;
                    uint16 y0 = (yy / (patchMod)) * patchMod;
                    
                    uint16 x1 = x0 + (realQuadCountInPatch / PATCH_QUAD_COUNT);
                    uint16 y1 = y0 + (realQuadCountInPatch / PATCH_QUAD_COUNT);
                    
                    DVASSERT(x0 >= heightMapStartX && x0 <= heightMapStartX + realQuadCountInPatch);
                    DVASSERT(x1 >= heightMapStartX && x1 <= heightMapStartX + realQuadCountInPatch);
                    DVASSERT(y0 >= heightMapStartY && y0 <= heightMapStartY + realQuadCountInPatch);
                    DVASSERT(y1 >= heightMapStartY && y1 <= heightMapStartY + realQuadCountInPatch);
                    
                    float xin = (float32)(xx - x0) / (float32) (x1 - x0);
                    float yin = (float32)(yy - y0) / (float32) (y1 - y0);
                    
                    Vector3 p00 = GetPoint(x0, y0, data[heightmap->Size() * y0 + x0]);
                    Vector3 p01 = GetPoint(x0, y1, data[heightmap->Size() * y1 + x0]);
                    Vector3 p10 = GetPoint(x1, y0, data[heightmap->Size() * y0 + x1]);
                    Vector3 p11 = GetPoint(x1, y1, data[heightmap->Size() * y1 + x1]);
                    
                    Vector3 lodPos =  p00 * (1.0f - xin) * (1.0f - yin)
                                    + p01 * (1.0f - xin) * yin
                                    + p10 * xin * (1.0f - yin)
                                    + p11 * xin * yin;
                    
                    
                    DVASSERT(FLOAT_EQUAL(lodPos.x, pos.x) && FLOAT_EQUAL(lodPos.y, pos.y));
                    
                    if (Abs(lodPos.z - pos.z) > patch->maxError)
                    {
                        patch->maxError = pos.z - lodPos.z;
                        patch->positionOfMaxError = pos;
                    }
                }
            }
    }
    
    // Logger::FrameworkDebug("%d - (%d, %d) - %d", level, x, y, realQuadCountInPatch);
    
    if (realQuadCountInPatch > MAX_QUAD_COUNT_IN_VBO)
    {
        patch->rdoQuad = -1;
    }else
    {
        uint32 x = heightMapStartX / MAX_QUAD_COUNT_IN_VBO;
        uint32 y = heightMapStartY / MAX_QUAD_COUNT_IN_VBO;

//        uint32 xCheck = (heightMapStartX + realQuadCountInPatch) / MAX_QUAD_COUNT_IN_VBO;
//        uint32 yCheck = (heightMapStartY + realQuadCountInPatch) / MAX_QUAD_COUNT_IN_VBO;
//        
//        DVASSERT(x == xCheck && y == yCheck);
        
        patch->rdoQuad = y * rdoQuadWidth + x;
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
        DVASSERT(true);
        return;
    }
    
    SubdivisionLevelInfo & levelInfo = subdivLevelInfoArray[level];
    uint32 offset = levelInfo.offset + levelInfo.size * y + x;
    PatchQuadInfo * patch = &patchQuadArray[offset];
    SubdivisionPatchInfo * subdivPatchInfo = &subdivPatchArray[offset];
    
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
    
    if ((patch->rdoQuad == -1)
        || (solidAngle > fovSolidAngleError)
        || (geometryError > fovGeometryAngleError)
        || (patch->maxError > fovAbsHeightError) )
    {
        subdivPatchInfo->subdivisionState = SubdivisionPatchInfo::SUBDIVIDED;
        subdivPatchInfo->lastSubdividedSize = levelInfo.size;
        
        uint32 x2 = x * 2;
        uint32 y2 = y * 2;
        
        SubdividePatch(level + 1, x2 + 0, y2 + 0, clippingFlags);
        SubdividePatch(level + 1, x2 + 1, y2 + 0, clippingFlags);
        SubdividePatch(level + 1, x2 + 0, y2 + 1, clippingFlags);
        SubdividePatch(level + 1, x2 + 1, y2 + 1, clippingFlags);
    }else
    {
        //DrawPatch(level, x, y, 0, 0, 0, 0);
        TerminateSubdivision(level, x, y, levelInfo.size);
        subdivPatchInfo->subdivisionState = SubdivisionPatchInfo::DRAW;
    }
}
    
void Landscape::TerminateSubdivision(uint32 level, uint32 x, uint32 y, uint32 lastSubdividedSize)
{
    if (level == subdivLevelCount)
    {
        return;
    }
    
    SubdivisionLevelInfo & levelInfo = subdivLevelInfoArray[level];
    SubdivisionPatchInfo * subdivPatchInfo = &subdivPatchArray[levelInfo.offset + levelInfo.size * y + x];

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
    
    SubdivisionLevelInfo & levelInfo = subdivLevelInfoArray[level];
    
    // TODO: optimise all offset calculations in all places
    SubdivisionPatchInfo * subdivPatchInfo = &subdivPatchArray[levelInfo.offset + levelInfo.size * y + x];

    uint32 state = subdivPatchInfo->subdivisionState;
    if (state == SubdivisionPatchInfo::CLIPPED)return;
    
    if (state == SubdivisionPatchInfo::SUBDIVIDED)
    {
        uint32 x2 = x * 2;
        uint32 y2 = y * 2;
        
        AddPatchToRenderNoInstancing(level + 1, x2 + 0, y2 + 0);
        AddPatchToRenderNoInstancing(level + 1, x2 + 1, y2 + 0);
        AddPatchToRenderNoInstancing(level + 1, x2 + 0, y2 + 1);
        AddPatchToRenderNoInstancing(level + 1, x2 + 1, y2 + 1);
    }else
    {
        SubdivisionPatchInfo * xNeg = GetSubdivPatch(level, x - 1, y);
        SubdivisionPatchInfo * xPos = GetSubdivPatch(level, x + 1, y);
        SubdivisionPatchInfo * yNeg = GetSubdivPatch(level, x, y - 1);
        SubdivisionPatchInfo * yPos = GetSubdivPatch(level, x, y + 1);
        
        uint32 xNegSize = levelInfo.size;
        uint32 xPosSize = levelInfo.size;
        uint32 yNegSize = levelInfo.size;
        uint32 yPosSize = levelInfo.size;
        
        
        if (xNeg)xNegSize = xNeg->lastSubdividedSize;
        if (xPos)xPosSize = xPos->lastSubdividedSize;
        if (yNeg)yNegSize = yNeg->lastSubdividedSize;
        if (yPos)yPosSize = yPos->lastSubdividedSize;
        
        DrawPatch(level, x, y, xNegSize, xPosSize, yNegSize, yPosSize);
    }
}
    
void Landscape::DrawNoInstancing()
{
    AddPatchToRenderNoInstancing(0, 0, 0);
}

void Landscape::DrawPatch(uint32 level, uint32 xx, uint32 yy,
                          uint32 xNegSize, uint32 xPosSize, uint32 yNegSize, uint32 yPosSize)
{
    SubdivisionLevelInfo & levelInfo = subdivLevelInfoArray[level];
    PatchQuadInfo * patch = &patchQuadArray[levelInfo.offset + levelInfo.size * yy + xx];
    
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
    
    // Draw middle block
    {
        for (uint16 y = (heightMapStartY & RENDER_QUAD_AND); y < (heightMapStartY & RENDER_QUAD_AND) + realVertexCountInPatch; y += step)
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
            
                
                *queueDrawIndices++ = GetVertexIndex(x0aligned, y0aligned);
                *queueDrawIndices++ = GetVertexIndex(x1aligned, y0aligned2);
                *queueDrawIndices++ = GetVertexIndex(x0aligned2, y1aligned);
                
                *queueDrawIndices++ = GetVertexIndex(x1aligned, y0aligned2);
                *queueDrawIndices++ = GetVertexIndex(x1aligned2, y1aligned2);
                *queueDrawIndices++ = GetVertexIndex(x0aligned2, y1aligned);
                
                queueRenderCount += 6;
            }
    }
   
    DVASSERT(queueRenderCount < INDEX_ARRAY_COUNT);
}
    
void Landscape::ReleaseLandscape()
{
    SafeDeleteArray(indices);
    SafeDeleteArray(subdivPatchArray);
    SafeDeleteArray(patchQuadArray);
    SafeDeleteArray(rdoArray);
    
    for (size_t k = 0; k < landscapeRDOArray.size(); ++k)
    {
        SafeRelease(landscapeRDOArray[k]);
        SafeDeleteArray(landscapeVerticesArray[k]);
    }
    landscapeRDOArray.clear();
    landscapeVerticesArray.clear();
}

void Landscape::ReallocateLandscape()
{
    ReleaseLandscape();
 
    landscapeQuality = QualitySettingsSystem::Instance()->GetCurMaterialQuality(LANDSCAPE_QUALITY_NAME);
    isRequireTangentBasis = (landscapeQuality == LANDSCAPE_QUALITY_VALUE_HIGH);
    
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
    
    //TODO: Release those arrays
    indices = new uint16[INDEX_ARRAY_COUNT];
    subdivPatchArray = new SubdivisionPatchInfo[subdivPatchCount];
    patchQuadArray = new PatchQuadInfo[subdivPatchCount];
    
    rdoQuadWidth = heightmapSizeMinus1 / (RENDER_QUAD_WIDTH - 1);

    // For cases where landscape is very small allocate 1 RDO.
    if (rdoQuadWidth == 0)rdoQuadWidth = 1;
    
    uint32 quadCountInOneRDO = heightmapSizeMinus1 / rdoQuadWidth;
    
    rdoArray = new LandscapeQuad[rdoQuadWidth * rdoQuadWidth];
    
    for (uint32 y = 0; y < rdoQuadWidth; ++y)
        for (uint32 x = 0; x  < rdoQuadWidth; ++x)
        {
            LandscapeQuad * quad = &rdoArray[x + y * rdoQuadWidth];
            quad->x = x * 128;
            quad->y = y * 128;
            quad->size = quadCountInOneRDO;
            
            uint16 check = AllocateRDOQuad(quad);
            DVASSERT(check == (uint16)(x + y * rdoQuadWidth));
        }

    UpdatePatchInfo(0, 0, 0);
}
    
    

    
/*
    level 0 = full landscape
    level 1 = first set of quads
    level 2 = 2
    level 3 = 3
    level 4 = 4
 */
    
//float32 LandscapeNode::BitmapHeightToReal(uint8 height)
Vector3 Landscape::GetPoint(int16 x, int16 y, uint16 height) const
{
    Vector3 res;
    res.x = (bbox.min.x + (float32)x / (float32)(heightmap->Size() - 1) * (bbox.max.x - bbox.min.x));
    res.y = (bbox.min.y + (float32)y / (float32)(heightmap->Size() - 1) * (bbox.max.y - bbox.min.y));
    res.z = (bbox.min.z + ((float32)height / (float32)Heightmap::MAX_VALUE) * (bbox.max.z - bbox.min.z));
    return res;
};

bool Landscape::PlacePoint(const Vector3 & point, Vector3 & result, Vector3 * normal) const
{
	if (point.x > bbox.max.x ||
		point.x < bbox.min.x ||
		point.y > bbox.max.y ||
		point.y < bbox.min.y)
	{
		return false;
	}
	
	if(heightmap->Data() == NULL)
	{
		Logger::Error("[Landscape::PlacePoint] Trying to place point on empty heightmap data!");
		return false;
	}
	
	float32 kW = (float32)(heightmap->Size() - 1) / (bbox.max.x - bbox.min.x);
	
	float32 x = (point.x - bbox.min.x) * kW;
	float32 y = (point.y - bbox.min.y) * kW;

	float32 x1 = floor(x);
	float32 y1 = floor(y);

	float32 x2 = ceil(x);
	float32 y2 = ceil(y);

	if (x1 == x2)
		x2 += 1.0f;

	if (y1 == y2)
		y2 += 1.0f;

	uint16 * data = heightmap->Data();
	int32 imW = heightmap->Size();

	Vector3 p1(x1, y1, 0);
	p1.z = data[(int32)p1.y * imW + (int32)p1.x];

	Vector3 p2(x2, y2, 0);
	p2.z = data[(int32)p2.y * imW + (int32)p2.x];

	Vector3 p3;
	if (x - x1 >= y - y1)
		p3 = Vector3(x2, y1, 0);
	else
		p3 = Vector3(x1, y2, 0);
	p3.z = data[(int32)p3.y * imW + (int32)p3.x];

	//http://algolist.manual.ru/maths/geom/equation/plane.php
	float32 A = p1.y * (p2.z - p3.z) + p2.y * (p3.z - p1.z) + p3.y * (p1.z - p2.z); 
	float32 B = p1.z * (p2.x - p3.x) + p2.z * (p3.x - p1.x) + p3.z * (p1.x - p2.x);
	float32 C = p1.x * (p2.y - p3.y) + p2.x * (p3.y - p1.y) + p3.x * (p1.y - p2.y);
	float32 D = p1.x * (p2.y * p3.z - p3.y * p2.z) + p2.x * (p3.y * p1.z - p1.y * p3.z) + p3.x * (p1.y * p2.z - p2.y * p1.z);

	result.x = point.x;
	result.y = point.y;

	result.z = (D - B * y - A * x) / C;
	result.z = bbox.min.z + result.z / ((float32)Heightmap::MAX_VALUE) * (bbox.max.z - bbox.min.z);

	if (normal != 0)
	{
		normal->x = A;
		normal->y = B;
		normal->z = C;
		normal->Normalize();
	}
	return true;
};
	  
void Landscape::SetTextureTiling(eTextureLevel level, const Vector2 & tiling)
{
    if(TILEMASK_TILING_PROPS_NAMES[level] != INVALID_PROPERTY_NAME)
    {
        tileMaskMaterial->SetPropertyValue(TILEMASK_TILING_PROPS_NAMES[level], Shader::UT_FLOAT_VEC2, 1, &tiling);
    }
}

Vector2 Landscape::GetTextureTiling(eTextureLevel level)
{
    Vector2 propValue;
    NMaterialProperty* prop = tileMaskMaterial->GetPropertyValue(TILEMASK_TILING_PROPS_NAMES[level]);
    
    if(prop)
    {
        memcpy(&propValue, prop->data, sizeof(Vector2));
    }
    
    return propValue;
}
    
void Landscape::SetTileColor(eTextureLevel level, const Color & color)
{
    if(TILEMASK_COLOR_PROPS_NAMES[level] != INVALID_PROPERTY_NAME)
    {
        tileMaskMaterial->SetPropertyValue(TILEMASK_COLOR_PROPS_NAMES[level], Shader::UT_FLOAT_VEC3, 1, &color);
    }
}

Color Landscape::GetTileColor(eTextureLevel level)
{
    Color propValue;
    NMaterialProperty* prop = tileMaskMaterial->GetPropertyValue(TILEMASK_COLOR_PROPS_NAMES[level]);
    
    if(prop)
    {
        size_t dataSize = Shader::GetUniformTypeSize(prop->type) * prop->size;
        memcpy(&propValue, prop->data, dataSize);
    }
    
    return propValue;
}

    
void Landscape::SetTexture(eTextureLevel level, const FilePath & textureName)
{
	if(TEXTURE_TILE_FULL != level &&
       TILEMASK_TEXTURE_PROPS_NAMES[level] != INVALID_PROPERTY_NAME)
	{
		tileMaskMaterial->SetTexture(TILEMASK_TEXTURE_PROPS_NAMES[level], textureName);
	}
}
    
//Texture * Landscape::CreateTexture(eTextureLevel level, const FilePath & textureName)
//{
//    if(TEXTURE_TILE_FULL == level)
//    {
//        //must not zero only for finalized maps
//        if(!textureName.IsEmpty())
//        {
//            return Texture::PureCreate(textureName);
//        }
//        return NULL;
//    }
//
//    return Texture::CreateFromFile(textureName);
//}


void Landscape::SetTexture(eTextureLevel level, Texture *texture)
{
 	//textureNames[level] = String("");

	if(TILEMASK_TEXTURE_PROPS_NAMES[level] != INVALID_PROPERTY_NAME)
	{
		tileMaskMaterial->SetTexture(TILEMASK_TEXTURE_PROPS_NAMES[level], texture);
	}
}

    
Texture * Landscape::GetTexture(eTextureLevel level)
{
	return tileMaskMaterial->GetEffectiveTexture(TILEMASK_TEXTURE_PROPS_NAMES[level]);
}
    
void Landscape::FlushQueue()
{
    if (queueRenderCount == 0) return;
    if (queueRdoQuad == -1) return;
    
	//currentMaterial->Draw(landscapeRDOArray[queueRdoQuad], indices, queueRenderCount);
	tileMaskMaterial->Draw(landscapeRDOArray[queueRdoQuad], indices, queueRenderCount);
	
	drawIndices += queueRenderCount;
    
    //Logger::FrameworkDebug("flush: %d %d", queueRdoQuad, queueRenderCount);

    ClearQueue();
    
    ++flushQueueCounter;
}
    
void Landscape::ClearQueue()
{
    queueRenderCount = 0;
    queueRdoQuad = -1;
    queueDrawIndices = indices;
}

void Landscape::BindMaterial(Camera* camera)
{	
	tileMaskMaterial->SetPropertyValue(Landscape::PARAM_CAMERA_POSITION, Shader::UT_FLOAT_VEC3, 1, &cameraPos);
	tileMaskMaterial->BindMaterialTechnique(TECHNIQUE_TILEMASK_NAME, camera);
}

    
void Landscape::DrawLandscape()
{
    ClearQueue();
    RenderManager::Instance()->SetDynamicParam(PARAM_WORLD, &Matrix4::IDENTITY, (pointer_size)&Matrix4::IDENTITY);
    BindMaterial(camera);
    DrawNoInstancing();
    FlushQueue();
}
    
void Landscape::Draw(Camera * drawCamera)
{
    FastName newLandscapeQuality = QualitySettingsSystem::Instance()->GetCurMaterialQuality(LANDSCAPE_QUALITY_NAME);
    if (newLandscapeQuality != landscapeQuality)
    {
        // If landscape quality is changed we need to reallocate landscape.
        // It's required only for editor
        ReallocateLandscape();
    }
    
    TIME_PROFILE("LandscapeNode.Draw");
	
	drawIndices = 0;

	if(!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::LANDSCAPE_DRAW))
	{
		return;
	}
	
#if defined(__DAVAENGINE_OPENGL__) && (defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__))
    if (isDebugDraw)
    {
//        RenderManager::Instance()->SetColor(1.0f, 0.f, 0.f, 1.f);
//        RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);
//        RenderManager::Instance()->SetShader(0);
//        RenderManager::Instance()->FlushState();

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
#endif //#if defined(__DAVAENGINE_OPENGL__)
    

    flushQueueCounter = 0;
    camera = GetRenderSystem()->GetMainCamera();
    frustum = camera->GetFrustum();
    cameraPos = camera->GetPosition();

    float32 fovLerp = Clamp((camera->GetFOV() - zoomFov) /  (normalFov - zoomFov), 0.0f, 1.0f);
    fovSolidAngleError = zoomSolidAngleError + (solidAngleError - zoomSolidAngleError) * fovLerp;
    fovGeometryAngleError = zoomGeometryAngleError + (geometryAngleError - zoomGeometryAngleError) * fovLerp;
    fovAbsHeightError = zoomAbsHeightError + (absHeightError - zoomAbsHeightError) * fovLerp;

    SubdividePatch(0, 0, 0, 0x3f);
    
    DrawLandscape();
    
    
#if defined(__DAVAENGINE_OPENGL__) && defined(__DAVAENGINE_MACOS__)
    if (isDebugDraw)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
#endif

	if(cursor)
	{
		//TODO: setup appropriate cursor state and set it
		//TODO: RenderManager::Instance()->SetRenderState(cursorStateHandle);
		RenderManager::Instance()->SetRenderState(cursor->GetRenderState());
		RenderManager::Instance()->FlushState();
		
		
		cursor->Prepare();
        
        DrawLandscape();
    }
}


bool Landscape::GetGeometry(Vector<LandscapeVertex> & landscapeVertices, Vector<int32> & indices) const
{
    if (heightmap->Data() == nullptr)
    {
        return false;
    }

    int32 quadCount = heightmap->Size() - 1;	
	landscapeVertices.resize((quadCount + 1) * (quadCount + 1));

	int32 index = 0;
	for (int32 y = 0; y < quadCount + 1; ++y)
	{
		for (int32 x = 0; x < quadCount + 1; ++x)
		{
			landscapeVertices[index].position = GetPoint(x, y, heightmap->Data()[y * heightmap->Size() + x]);
			landscapeVertices[index].texCoord = Vector2((float32)x / (float32)(heightmap->Size() - 1), 1.0f - (float32)y / (float32)(heightmap->Size() - 1));
			index++;
		}
	}

	indices.resize(heightmap->Size()*heightmap->Size()*6);
	int32 step = 1;
	int32 indexIndex = 0;
	int32 vertexCount = heightmap->Size();
    
	for(int32 y = 0; y < quadCount; y += step)
	{
		for(int32 x = 0; x < quadCount; x += step)
		{
			indices[indexIndex++] = x + y * vertexCount;
			indices[indexIndex++] = (x + step) + y * vertexCount;
			indices[indexIndex++] = x + (y + step) * vertexCount;

			indices[indexIndex++] = (x + step) + y * vertexCount;
			indices[indexIndex++] = (x + step) + (y + step) * vertexCount;
			indices[indexIndex++] = x + (y + step) * vertexCount;
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
	if(newHeightMapPath == heightmapPath)
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
	Vector3 newLandscapeSize(newSize, newSize, bbox.GetSize().z);
	SetLandscapeSize(newLandscapeSize);
}
	
float32 Landscape::GetLandscapeHeight() const
{
	return bbox.GetSize().z;
}
	
void Landscape::SetLandscapeHeight(float32 newHeight)
{
	Vector3 newLandscapeSize(bbox.GetSize().x, bbox.GetSize().y, newHeight);
	SetLandscapeSize(newLandscapeSize);
}

void Landscape::SetLandscapeSize(const Vector3 & newLandscapeSize)
{
    if(newLandscapeSize.z < 0.0f || newLandscapeSize.x <0 || newLandscapeSize.y < 0)
	{
		return;
	}
    if(newLandscapeSize == bbox.GetSize())
	{
		return;
	}
    bbox.Empty();
	bbox.AddPoint(Vector3(-newLandscapeSize.x/2.f, -newLandscapeSize.y/2.f, 0.f));
	bbox.AddPoint(Vector3(newLandscapeSize.x/2.f, newLandscapeSize.y/2.f, newLandscapeSize.z));
    ReallocateLandscape();
    
    if(foliageSystem)
    {
        foliageSystem->SyncFoliageWithLandscape();
    }
}

void Landscape::Create(NMaterial *fromMaterial/* = NULL */)
{
    DVASSERT(NULL == tileMaskMaterial);
    DVASSERT(0 == GetRenderBatchCount());

    if(NULL == fromMaterial)
    {
        NMaterial* landscapeParent = NMaterial::CreateMaterial(FastName("Landscape_Tilemask_Material"), NMaterialName::TILE_MASK, NMaterial::DEFAULT_QUALITY_NAME);
	    tileMaskMaterial = NMaterial::CreateMaterialInstance();
	    tileMaskMaterial->SetParent(landscapeParent);
    	SafeRelease(landscapeParent);

    	SetDefaultValues();
    }
    else
    {
        tileMaskMaterial = fromMaterial->Clone();
    }
	
	LandscapeChunk * chunk = new LandscapeChunk(this);
	chunk->SetMaterial(tileMaskMaterial);
	chunk->SetSortingKey(10);
	AddRenderBatch(chunk);
	SafeRelease(chunk);
}
    
void Landscape::Save(KeyedArchive * archive, SerializationContext * serializationContext)
{
    RenderObject::Save(archive, serializationContext);

    //TODO: remove code in future. Need for transition from *.png to *.heightmap
    if(!heightmapPath.IsEqualToExtension(Heightmap::FileExtension()))
    {
        heightmapPath.ReplaceExtension(Heightmap::FileExtension());
    }

	heightmap->Save(heightmapPath);
    archive->SetString("hmap", heightmapPath.GetRelativePathname(serializationContext->GetScenePath()));
    archive->SetByteArrayAsType("bbox", bbox);
    
    DVASSERT(GetRenderBatch(0));
    IlluminationParams * illuminationParams = GetRenderBatch(0)->GetMaterial()->GetIlluminationParams(false);
    if(illuminationParams)
    {
        archive->SetBool("illumination.isUsed", illuminationParams->isUsed);
        archive->SetBool("illumination.castShadow", illuminationParams->castShadow);
        archive->SetBool("illumination.receiveShadow", illuminationParams->receiveShadow);
        archive->SetInt32("illumination.lightmapSize", illuminationParams->lightmapSize);
    }
}
    
void Landscape::Load(KeyedArchive * archive, SerializationContext * serializationContext)
{
	RenderObject::Load(archive, serializationContext);
	
    DVASSERT(serializationContext->GetVersion() >= 4);
		
    AABBox3 boxDef;
    boxDef = archive->GetByteArrayAsType("bbox", boxDef);
    LoadFog(archive, serializationContext);

    // check if this landscape was saved in old format, when all material properties
    // were saved as part of landscape object
    if(0 == GetRenderBatchCount())
    {
        // create landscape render object and load material properties
        // from current serialization context
        Create();
        LoadMaterialProps(archive, serializationContext);
    }
    else
    {
        LandscapeChunk *landCunk = dynamic_cast<LandscapeChunk *>(GetRenderBatch(0));
        DVASSERT(NULL != landCunk);
        
        // remember pointer on loaded landscape material
        tileMaskMaterial = SafeRetain(landCunk->GetMaterial());

        // remember this landscape in createad landscapeChunk
        landCunk->landscape = this;
    }
	
	FilePath heightmapPath = serializationContext->GetScenePath() + archive->GetString("hmap");
    BuildLandscapeFromHeightmapImage(heightmapPath, boxDef);

    if(archive->IsKeyExists("illumination.isUsed"))
    {
        DVASSERT(GetRenderBatch(0));
        IlluminationParams * illuminationParams = GetRenderBatch(0)->GetMaterial()->GetIlluminationParams();

        illuminationParams->isUsed = archive->GetBool("illumination.isUsed", illuminationParams->isUsed);
        illuminationParams->castShadow = archive->GetBool("illumination.castShadow", illuminationParams->castShadow);
        illuminationParams->receiveShadow = archive->GetBool("illumination.receiveShadow", illuminationParams->receiveShadow);
        illuminationParams->SetLightmapSize(archive->GetInt32("illumination.lightmapSize", illuminationParams->lightmapSize));
    }
}

void Landscape::LoadFog(KeyedArchive * archive, SerializationContext * serializationContext)
{
    DAVA::NMaterial *globalMaterial = serializationContext->GetScene()->GetGlobalMaterial();

    if(NULL != globalMaterial)
    {
        if(archive->IsKeyExists("fogcolor"))
        {
            Color fogColorValue = archive->GetByteArrayAsType("fogcolor", Color(1.0f, 0, 0, 1.0f));
            globalMaterial->SetPropertyValue(NMaterial::PARAM_FOG_COLOR, Shader::UT_FLOAT_VEC4, 1, &fogColorValue);
        }

        if(archive->IsKeyExists("isFogEnabled"))
        {
            NMaterial::eFlagValue flag = (archive->GetBool("isFogEnabled") ? NMaterial::FlagOn : NMaterial::FlagOff);
            globalMaterial->SetFlag(NMaterial::FLAG_VERTEXFOG, flag);
        }

        if(archive->IsKeyExists("fogdencity"))
        {
            float32 fogDensityValue = archive->GetFloat("fogdencity", 0.05f);
            globalMaterial->SetPropertyValue(NMaterial::PARAM_FOG_DENSITY, Shader::UT_FLOAT, 1, &fogDensityValue);
        }
    }
}

void Landscape::LoadMaterialProps(KeyedArchive * archive, SerializationContext * serializationContext)
{
    for (int32 k = 0; k < TEXTURE_COUNT; ++k)
    {
        if(TEXTURE_DETAIL == k) continue;
        
		if(TEXTURE_TILE_FULL == k)
			continue;

        // load textures
		if(!(TEXTURE_TILE1 == k || TEXTURE_TILE2 == k || TEXTURE_TILE3 == k))
		{
			String textureName = archive->GetString(Format("tex_%d", k));
			if(!textureName.empty())
			{
				FilePath absPath = serializationContext->GetScenePath() + textureName;
                SetTexture((eTextureLevel)k, absPath);
			}
		}

        Vector2 tilingValue;
        tilingValue = archive->GetByteArrayAsType(Format("tiling_%d", k), tilingValue);
        SetTextureTiling((eTextureLevel)k, tilingValue);
            
        Color colorValue;
        colorValue = archive->GetByteArrayAsType(Format("tilecolor_%d", k), colorValue);
        SetTileColor((eTextureLevel)k, colorValue);
    }
	
	SetupMaterialProperties();
}

const FilePath & Landscape::GetTextureName(DAVA::Landscape::eTextureLevel level)
{
    DVASSERT(0 <= level && level < TEXTURE_COUNT);
    return tileMaskMaterial->GetEffectiveTexturePath(TILEMASK_TEXTURE_PROPS_NAMES[level]);
}
    
void Landscape::SetTextureName(eTextureLevel level, const FilePath &newTextureName)
{
    DVASSERT(0 <= level && level < TEXTURE_COUNT);
    tileMaskMaterial->SetTexturePath(TILEMASK_TEXTURE_PROPS_NAMES[level], newTextureName);
}


void Landscape::CursorEnable()
{
	DVASSERT(0 == cursor);
	cursor = new LandscapeCursor();
}

void Landscape::CursorDisable()
{
	SafeDelete(cursor);
}
    
Heightmap * Landscape::GetHeightmap()
{
    return heightmap;
}

void Landscape::SetHeightmap(DAVA::Heightmap *height)
{
    SafeRelease(heightmap);
    heightmap = SafeRetain(height);
    
    ReallocateLandscape();
}
    
    
Texture * Landscape::CreateLandscapeTexture()
{
    //Set indexes
    Vector<float32> ftVertexes;
    Vector<float32> ftTextureCoords;
    
    float32 x0 = 0.f;
    float32 y0 = 0.f;
    float32 x1 = 1.f;
    float32 y1 = 1.f;
    
    //triangle 1
    //0, 0
    ftVertexes.push_back(x0);
    ftVertexes.push_back(y0);
    ftVertexes.push_back(0);
    ftTextureCoords.push_back(0);
    ftTextureCoords.push_back(0);
    
    
    //1, 0
    ftVertexes.push_back(x1);
    ftVertexes.push_back(y0);
    ftVertexes.push_back(0);
    ftTextureCoords.push_back(1);
    ftTextureCoords.push_back(0);
    
    
    //0, 1
    ftVertexes.push_back(x0);
    ftVertexes.push_back(y1);
    ftVertexes.push_back(0);
    ftTextureCoords.push_back(0);
    ftTextureCoords.push_back(1);
    
    //1, 1
    ftVertexes.push_back(x1);
    ftVertexes.push_back(y1);
    ftVertexes.push_back(0);
    ftTextureCoords.push_back(1);
    ftTextureCoords.push_back(1);
    
    RenderDataObject *ftRenderData = new RenderDataObject();
    ftRenderData->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, 0, &ftVertexes.front());
    ftRenderData->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, &ftTextureCoords.front());

    //Draw landscape to texture
    Rect oldViewport = RenderManager::Instance()->GetViewport();
    
    Texture *fullTiled = Texture::CreateFBO(TEXTURE_TILE_FULL_SIZE, TEXTURE_TILE_FULL_SIZE, FORMAT_RGBA8888, Texture::DEPTH_NONE);
    RenderManager::Instance()->SetRenderTarget(fullTiled);
    RenderManager::Instance()->SetViewport(Rect(0.f, 0.f, (float32)fullTiled->GetWidth(), (float32)fullTiled->GetHeight()));
    RenderManager::Instance()->SetClip(Rect(0.f, 0.f, -1.f, -1.f));

	RenderManager::Instance()->ClearWithColor(1.f, 0.f, 1.f, 1.f);
 
    RenderManager::SetDynamicParam(PARAM_WORLD, &Matrix4::IDENTITY, (pointer_size)&Matrix4::IDENTITY);
    RenderManager::SetDynamicParam(PARAM_VIEW, &Matrix4::IDENTITY, (pointer_size)&Matrix4::IDENTITY);
    Matrix4 projection;
    projection.glOrtho(0.f, 1.f, 0.f, 1.f, -1.f, 1.f);
    
    Matrix4 *oldProjection = (Matrix4*)RenderManager::GetDynamicParam(PARAM_PROJ);
    RenderManager::SetDynamicParam(PARAM_PROJ, &projection, UPDATE_SEMANTIC_ALWAYS);
    
    NMaterial* tmpLandscapeParent = NMaterial::CreateMaterial(FastName("Landscape_Tilemask_Material_TMP"), FastName("~res:/Materials/TileMask.material"), NMaterial::DEFAULT_QUALITY_NAME);
    NMaterial* tmpTileMaskMaterial = tileMaskMaterial->Clone();
    

    //MAGIC: This magic code is workaround for situation when fbo textures are present in tileMaskMaterial with pathname. Because NMaterial::Clone() use pathnames for cloning textures.
    const uint32 texturesCount = tileMaskMaterial->GetTextureCount();
    for(uint32 t = 0; t < texturesCount; ++t)
    {
        FastName tName = tileMaskMaterial->GetTextureName(t);
        Texture *tex = tileMaskMaterial->GetTexture(t);
        if(tex && tex->isRenderTarget)
        {
            tmpTileMaskMaterial->SetTexture(tName, tex);
        }
    }
    //END of MAGIC
    
    tmpTileMaskMaterial->SetFlag(NMaterial::FLAG_VERTEXFOG, NMaterial::FlagOff);
    tmpTileMaskMaterial->SetParent(tmpLandscapeParent);
    tmpTileMaskMaterial->BindMaterialTechnique(TECHNIQUE_TILEMASK_NAME, NULL);

	RenderManager::Instance()->SetRenderData(ftRenderData);
	RenderManager::Instance()->AttachRenderData();
	RenderManager::Instance()->HWDrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, 0, 4);

    RenderManager::Instance()->SetRenderTarget(0);
    
    RenderManager::SetDynamicParam(PARAM_PROJ, &oldProjection, UPDATE_SEMANTIC_ALWAYS);
	RenderManager::Instance()->SetViewport(oldViewport);
    SafeRelease(ftRenderData);

    SafeRelease(tmpTileMaskMaterial);
    SafeRelease(tmpLandscapeParent);
    
    return fullTiled;
}

    
LandscapeCursor * Landscape::GetCursor()
{
    return cursor;
}

//TODO: review landscape cloning
RenderObject * Landscape::Clone( RenderObject *newObject )
{
	if(!newObject)
	{
		DVASSERT_MSG(IsPointerToExactClass<Landscape>(this), "Can clone only Landscape");
		newObject = new Landscape();
	}
    
    Landscape *newLandscape = static_cast<Landscape *>(newObject);
    newLandscape->Create(tileMaskMaterial);

    newLandscape->defaultFov = defaultFov;
    
    newLandscape->solidAngleError = solidAngleError;
    newLandscape->geometryAngleError = geometryAngleError;
    newLandscape->absHeightError = absHeightError;
    
    newLandscape->zoomSolidAngleError = zoomSolidAngleError;
    newLandscape->zoomGeometryAngleError = zoomGeometryAngleError;
    newLandscape->zoomAbsHeightError = zoomAbsHeightError;
    newLandscape->zoomFov = zoomFov;
    newLandscape->normalFov = normalFov;
    
    newLandscape->flags = flags;
    newLandscape->BuildLandscapeFromHeightmapImage(heightmapPath, bbox);
	newLandscape->SetupMaterialProperties();

    IlluminationParams * params = GetRenderBatch(0)->GetMaterial()->GetIlluminationParams(false);
    if(params)
    {
        IlluminationParams * newParams = newLandscape->GetRenderBatch(0)->GetMaterial()->GetIlluminationParams();
        newParams->SetLightmapSize(params->GetLightmapSize());
        newParams->isUsed = params->isUsed;
        newParams->castShadow = params->castShadow;
        newParams->receiveShadow = params->receiveShadow;
    }

	return newObject;
}
	
int32 Landscape::GetDrawIndices() const
{
    return drawIndices;
}

void Landscape::SetDefaultValues()
{
    Color color;
    SetTileColor(TEXTURE_TILE0, color);
    SetTileColor(TEXTURE_TILE1, color);
    SetTileColor(TEXTURE_TILE2, color);
    SetTileColor(TEXTURE_TILE3, color);
}

void Landscape::SetupMaterialProperties()
{
	if(tileMaskMaterial)
	{
		tileMaskMaterial->SetPropertyValue(Landscape::PARAM_CAMERA_POSITION, Shader::UT_FLOAT_VEC3, 1, &cameraPos);
	}
}
	
void Landscape::SetFoliageSystem(FoliageSystem* _foliageSystem)
{
    foliageSystem = _foliageSystem;
}

};
