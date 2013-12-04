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
#include "Render/Image.h"
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
#include "Render/ImageLoader.h"
#include "LandscapeChunk.h"
#include "Debug/Stats.h"


namespace DAVA
{

//#define DRAW_OLD_STYLE
// const float32 LandscapeNode::TEXTURE_TILE_FULL_SIZE = 2048;

Landscape::Landscape()
    : indices(0)
{
    textureNames.resize(TEXTURE_COUNT);
    
    type = TYPE_LANDSCAPE;
    
    for (int32 t = 0; t < TEXTURE_COUNT; ++t)
        textures[t] = 0;
    
    frustum = 0; //new Frustum();
    
    tileMaskShader = NULL;
    fullTiledShader = NULL;
    nearLodIndex = 0;
    farLodIndex = 1;
    
	cursor = 0;
    uniformCameraPosition = -1;
	textureTiling.resize(TEXTURE_COUNT);
	tileColor.resize(TEXTURE_COUNT);
    for (int32 k = 0; k < TEXTURE_COUNT; ++k)
    {
        uniformTextures[k] = -1;
        uniformTextureTiling[k] = -1;
        textureTiling[k] = Vector2(1.0f, 1.0f);
        uniformTileColor[k] = -1;
        tileColor[k] = Color::White();
    }
    uniformFogDensity = -1;
    uniformFogColor = -1;
    uniformFogDensityFT = -1;
    uniformFogColorFT = -1;
    
    heightmap = new Heightmap();
        
    prevLodLayer = -1;
    
    isFogEnabled = false;
    fogDensity = 0.006f;
    fogColor = Color::White();
	
	//VI: init shaders AFTER all member variables were initialized
	SetTiledShaderMode(TILED_MODE_MIXED);
    
    LandscapeChunk * chunk = new LandscapeChunk(this);
    AddRenderBatch(chunk);
    SafeRelease(chunk);
    
    drawIndices = 0;
}

Landscape::~Landscape()
{
    ReleaseShaders();
    ReleaseAllRDOQuads();
    
    for (int32 t = 0; t < TEXTURE_COUNT; ++t)
    {
        SafeRelease(textures[t]);
    }
    SafeDeleteArray(indices);

    SafeRelease(heightmap);
	SafeDelete(cursor);
}
    
void Landscape::InitShaders()
{
    ReleaseShaders();
    
    tileMaskShader = new Shader();
    tileMaskShader->LoadFromYaml("~res:/Shaders/Landscape/tilemask.shader");
    
    String defines = "";
    
	if(isFogEnabled && RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::FOG_ENABLE))
    {
        defines += "VERTEX_FOG";
    }
    if (TILED_MODE_TILE_DETAIL_MASK == tiledShaderMode)
    {
        if (defines.size() != 0)defines += ";";
        defines += "DETAILMASK";
    }
    tileMaskShader->SetDefineList(defines);
    
    tileMaskShader->Recompile();
    
    uniformTextures[TEXTURE_TILE0] = tileMaskShader->FindUniformIndexByName("tileTexture0");
    uniformTextures[TEXTURE_TILE1] = tileMaskShader->FindUniformIndexByName("tileTexture1");
    uniformTextures[TEXTURE_TILE2] = tileMaskShader->FindUniformIndexByName("tileTexture2");
    uniformTextures[TEXTURE_TILE3] = tileMaskShader->FindUniformIndexByName("tileTexture3");
    uniformTextures[TEXTURE_TILE_MASK] = tileMaskShader->FindUniformIndexByName("tileMask");
    uniformTextures[TEXTURE_COLOR] = tileMaskShader->FindUniformIndexByName("colorTexture");
    
    uniformCameraPosition = tileMaskShader->FindUniformIndexByName("cameraPosition");
    
    uniformTextureTiling[TEXTURE_TILE0] = tileMaskShader->FindUniformIndexByName("texture0Tiling");
    uniformTextureTiling[TEXTURE_TILE1] = tileMaskShader->FindUniformIndexByName("texture1Tiling");
    uniformTextureTiling[TEXTURE_TILE2] = tileMaskShader->FindUniformIndexByName("texture2Tiling");
    uniformTextureTiling[TEXTURE_TILE3] = tileMaskShader->FindUniformIndexByName("texture3Tiling");
    
    uniformTileColor[TEXTURE_TILE0] = tileMaskShader->FindUniformIndexByName("tileColor0");
    uniformTileColor[TEXTURE_TILE1] = tileMaskShader->FindUniformIndexByName("tileColor1");
    uniformTileColor[TEXTURE_TILE2] = tileMaskShader->FindUniformIndexByName("tileColor2");
    uniformTileColor[TEXTURE_TILE3] = tileMaskShader->FindUniformIndexByName("tileColor3");

    
	if(isFogEnabled && RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::FOG_ENABLE))
    {
        uniformFogColor = tileMaskShader->FindUniformIndexByName("fogColor");
        uniformFogDensity = tileMaskShader->FindUniformIndexByName("fogDensity");   
    }
    
    fullTiledShader = new Shader();
    fullTiledShader->LoadFromYaml("~res:/Shaders/Landscape/fulltiled_texture.shader");
	if(isFogEnabled && RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::FOG_ENABLE))
    {
        fullTiledShader->SetDefineList("VERTEX_FOG");   
    }
    
    fullTiledShader->Recompile();
    
    if(isFogEnabled && RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::FOG_ENABLE))
    {
        uniformFogColorFT = fullTiledShader->FindUniformIndexByName("fogColor");
        uniformFogDensityFT = fullTiledShader->FindUniformIndexByName("fogDensity");   
    }
}
    
void Landscape::ReleaseShaders()
{
    SafeRelease(tileMaskShader);
    SafeRelease(fullTiledShader);
    
    uniformCameraPosition = -1;
    for (int32 k = 0; k < TEXTURE_COUNT; ++k)
    {
        uniformTextures[k] = -1;
        uniformTextureTiling[k] = -1;
    }
    
    uniformFogColor = -1;
    uniformFogDensity = -1;   
    uniformFogColorFT = -1;
    uniformFogDensityFT = -1;   
}


int16 Landscape::AllocateRDOQuad(LandscapeQuad * quad)
{
//    Logger::FrameworkDebug("AllocateRDOQuad: %d %d size: %d", quad->x, quad->y, quad->size);
    DVASSERT(quad->size == RENDER_QUAD_WIDTH - 1);
    LandscapeVertex * landscapeVertices = new LandscapeVertex[(quad->size + 1) * (quad->size + 1)];
    
    int32 index = 0;
    for (int32 y = quad->y; y < quad->y + quad->size + 1; ++y)
        for (int32 x = quad->x; x < quad->x + quad->size + 1; ++x)
        {
            landscapeVertices[index].position = GetPoint(x, y, heightmap->Data()[y * heightmap->Size() + x]);
            Vector2 texCoord = Vector2((float32)(x) / (float32)(heightmap->Size() - 1), (float32)(y) / (float32)(heightmap->Size() - 1));           

            landscapeVertices[index].texCoord = texCoord;
            //landscapeVertices[index].texCoord -= Vector2(0.5f, 0.5f);
//            Logger::FrameworkDebug("AllocateRDOQuad: %d pos(%f, %f)", index, landscapeVertices[index].texCoord.x, landscapeVertices[index].texCoord.y);
            index++;
        }
    
    // setup a base RDO
    RenderDataObject * landscapeRDO = new RenderDataObject();
    landscapeRDO->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, sizeof(LandscapeVertex), &landscapeVertices[0].position); 
    landscapeRDO->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, sizeof(LandscapeVertex), &landscapeVertices[0].texCoord); 
    landscapeRDO->BuildVertexBuffer((quad->size + 1) * (quad->size + 1));
//    SafeDeleteArray(landscapeVertices);
    
    landscapeVerticesArray.push_back(landscapeVertices);
    
    landscapeRDOArray.push_back(landscapeRDO);
    
//    Logger::FrameworkDebug("Allocated vertices: %d KB", sizeof(LandscapeVertex) * (quad->size + 1) * (quad->size + 1) / 1024);
    
    return (int16)landscapeRDOArray.size() - 1;
}

void Landscape::ReleaseAllRDOQuads()
{
    for (size_t k = 0; k < landscapeRDOArray.size(); ++k)
    {
        SafeRelease(landscapeRDOArray[k]);
        SafeDeleteArray(landscapeVerticesArray[k]);
    }
    landscapeRDOArray.clear();
    landscapeVerticesArray.clear();
}

void Landscape::SetLods(const Vector4 & lods)
{
    lodLevelsCount = 4;
    
    lodDistance[0] = lods.x;
    lodDistance[1] = lods.y;
    lodDistance[2] = lods.z;
    lodDistance[3] = lods.w;
    
    for (int32 ll = 0; ll < lodLevelsCount; ++ll)
        lodSqDistance[ll] = lodDistance[ll] * lodDistance[ll];
}
    
void Landscape::BuildLandscapeFromHeightmapImage(const FilePath & heightmapPathname, const AABBox3 & _box)
{
    ReleaseShaders(); // release previous shaders
    ReleaseAllRDOQuads();
    SafeDeleteArray(indices); //TODO: need here or no?

	heightmapPath = heightmapPathname;
	bbox = _box;

    InitShaders(); // init new shaders according to the selected rendering mode
    BuildHeightmap();
    BuildLandscape();
}

bool Landscape::BuildHeightmap()
{
    bool retValue = false;
    if(heightmapPath.IsEqualToExtension(".png"))
    {
        Vector<Image *> imageSet = ImageLoader::CreateFromFile(heightmapPath);
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
    
void Landscape::BuildLandscape()
{
    quadTreeHead.data.x = quadTreeHead.data.y = quadTreeHead.data.lod = 0;
    //quadTreeHead.data.xbuf = quadTreeHead.data.ybuf = 0;
    quadTreeHead.data.rdoQuad = -1;
    
    SetLods(Vector4(60.0f, 120.0f, 240.0f, 480.0f));
 
    allocatedMemoryForQuads = 0;

    if(heightmap->Size())
    {
        quadTreeHead.data.size = heightmap->Size() - 1;
        
        RecursiveBuild(&quadTreeHead, 0, lodLevelsCount);
        FindNeighbours(&quadTreeHead);
        
        indices = new uint16[INDEX_ARRAY_COUNT];
    }
    else
    {
        quadTreeHead.ReleaseChildren();
        quadTreeHead.data.size = 0;
    }
    
//    Logger::FrameworkDebug("Allocated indices: %d KB", RENDER_QUAD_WIDTH * RENDER_QUAD_WIDTH * 6 * 2 / 1024);
//    Logger::FrameworkDebug("Allocated memory for quads: %d KB", allocatedMemoryForQuads / 1024);
//    Logger::FrameworkDebug("sizeof(LandscapeQuad): %d bytes", sizeof(LandscapeQuad));
//    Logger::FrameworkDebug("sizeof(QuadTreeNode): %d bytes", sizeof(QuadTreeNode<LandscapeQuad>));
}
    
/*
    level 0 = full landscape
    level 1 = first set of quads
    level 2 = 2
    level 3 = 3
    level 4 = 4
 */
    
//float32 LandscapeNode::BitmapHeightToReal(uint8 height)
Vector3 Landscape::GetPoint(int16 x, int16 y, uint16 height)
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
	
	
	
void Landscape::RecursiveBuild(LandQuadTreeNode<LandscapeQuad> * currentNode, int32 level, int32 maxLevels)
{
    allocatedMemoryForQuads += sizeof(LandQuadTreeNode<LandscapeQuad>);
    currentNode->data.lod = level;
    
    // if we have parrent get rdo quad 
    if (currentNode->parent)
    {
        currentNode->data.rdoQuad = currentNode->parent->data.rdoQuad;
    }
    
    if ((currentNode->data.rdoQuad == -1) && (currentNode->data.size == RENDER_QUAD_WIDTH - 1))
    {
        currentNode->data.rdoQuad = AllocateRDOQuad(&currentNode->data);
        //currentNode->data.xbuf = 0;
        //currentNode->data.ybuf = 0;
    }
    
    // 
    // Check if we can build tree with less number of nodes
    // I think we should stop much earlier to perform everything faster
    //
    
    if (currentNode->data.size == 2)
    {
        // compute node bounding box
        uint16 * data = heightmap->Data();
        for (int16 x = currentNode->data.x; x <= currentNode->data.x + currentNode->data.size; ++x)
            for (int16 y = currentNode->data.y; y <= currentNode->data.y + currentNode->data.size; ++y)
            {
                uint16 value = data[heightmap->Size() * y + x];
                Vector3 pos = GetPoint(x, y, value);
                
                currentNode->data.bbox.AddPoint(pos);
            }
        return;
    }
    
    // alloc and process childs
    currentNode->AllocChildren();
    
    int16 minIndexX = currentNode->data.x;
    int16 minIndexY = currentNode->data.y;
    
    //int16 bufMinIndexX = currentNode->data.xbuf;
    //int16 bufMinIndexY = currentNode->data.ybuf;
    
    int16 size = currentNode->data.size;
    
    // We should be able to divide landscape by 2 here
    DVASSERT((size & 1) == 0);

    LandQuadTreeNode<LandscapeQuad> * child0 = &currentNode->children[0];
    child0->data.x = minIndexX;
    child0->data.y = minIndexY;
    //child0->data.xbuf = bufMinIndexX;
    //child0->data.ybuf = bufMinIndexY;
    child0->data.size = size / 2;
    
    LandQuadTreeNode<LandscapeQuad> * child1 = &currentNode->children[1];
    child1->data.x = minIndexX + size / 2;
    child1->data.y = minIndexY;
    //child1->data.xbuf = bufMinIndexX + size / 2;
    //child1->data.ybuf = bufMinIndexY;
    child1->data.size = size / 2;

    LandQuadTreeNode<LandscapeQuad> * child2 = &currentNode->children[2];
    child2->data.x = minIndexX;
    child2->data.y = minIndexY + size / 2;
    //child2->data.xbuf = bufMinIndexX;
    //child2->data.ybuf = bufMinIndexY + size / 2;
    child2->data.size = size / 2;

    LandQuadTreeNode<LandscapeQuad> * child3 = &currentNode->children[3];
    child3->data.x = minIndexX + size / 2;
    child3->data.y = minIndexY + size / 2;
    //child3->data.xbuf = bufMinIndexX + size / 2;
    //child3->data.ybuf = bufMinIndexY + size / 2;
    child3->data.size = size / 2;
    
    for (int32 index = 0; index < 4; ++index)
    {
        LandQuadTreeNode<LandscapeQuad> * child = &currentNode->children[index];
        child->parent = currentNode;
        RecursiveBuild(child, level + 1, maxLevels);
        
        currentNode->data.bbox.AddPoint(child->data.bbox.min);
        currentNode->data.bbox.AddPoint(child->data.bbox.max);
    }


}
/*
    Neighbours looks up
    *********
    *0*1*0*1*
    **0***1**
    *2*3*2*3*
    ****0****
    *0*1*0*1*
    **2***3**
    *2*3*2*3*
    *********
    *0*1*0*1*
    **0***1**
    *2*3*2*3*
    ****2****
    *0*1*0*1*
    **2***3**
    *2*3*2*3*
    *********
 */

LandQuadTreeNode<Landscape::LandscapeQuad> * Landscape::FindNodeWithXY(LandQuadTreeNode<LandscapeQuad> * currentNode, int16 quadX, int16 quadY, int16 quadSize)
{
    if ((currentNode->data.x <= quadX) && (quadX < currentNode->data.x + currentNode->data.size))
        if ((currentNode->data.y <= quadY) && (quadY < currentNode->data.y + currentNode->data.size))
    {
        if (currentNode->data.size == quadSize)return currentNode;
        if (currentNode->children)
        {
            for (int32 index = 0; index < 4; ++index)
            {
                LandQuadTreeNode<LandscapeQuad> * child = &currentNode->children[index];
                LandQuadTreeNode<LandscapeQuad> * result = FindNodeWithXY(child, quadX, quadY, quadSize);
                if (result)
                    return result;
            } 
        }
    }
    
    return 0;
}
    
void Landscape::FindNeighbours(LandQuadTreeNode<LandscapeQuad> * currentNode)
{
    currentNode->neighbours[LEFT] = FindNodeWithXY(&quadTreeHead, currentNode->data.x - 1, currentNode->data.y, currentNode->data.size);
    currentNode->neighbours[RIGHT] = FindNodeWithXY(&quadTreeHead, currentNode->data.x + currentNode->data.size, currentNode->data.y, currentNode->data.size);
    currentNode->neighbours[TOP] = FindNodeWithXY(&quadTreeHead, currentNode->data.x, currentNode->data.y - 1, currentNode->data.size);
    currentNode->neighbours[BOTTOM] = FindNodeWithXY(&quadTreeHead, currentNode->data.x, currentNode->data.y + currentNode->data.size, currentNode->data.size);
    
    if (currentNode->children)
    {
        for (int32 index = 0; index < 4; ++index)
        {
            LandQuadTreeNode<LandscapeQuad> * child = &currentNode->children[index];
            FindNeighbours(child);
        }
    }
}

void Landscape::MarkFrames(LandQuadTreeNode<LandscapeQuad> * currentNode, int32 & depth)
{
    if (--depth <= 0)
    {
        currentNode->data.frame = Core::Instance()->GetGlobalFrameIndex();
        depth++;
        return;
    }
    if (currentNode->children)
    {
        for (int32 index = 0; index < 4; ++index)
        {
            LandQuadTreeNode<LandscapeQuad> * child = &currentNode->children[index];
            MarkFrames(child, depth);
        }
    }
    depth++;
}
    
void Landscape::SetTextureTiling(eTextureLevel level, const Vector2 & tiling)
{
    textureTiling[level] = tiling;
}
    
const Vector2 & Landscape::GetTextureTiling(eTextureLevel level)
{
    return textureTiling[level];
}
    
void Landscape::SetTileColor(eTextureLevel level, const Color & color)
{
    tileColor[level] = color;
}

const Color & Landscape::GetTileColor(eTextureLevel level)
{
    return tileColor[level];
}

    
void Landscape::SetTexture(eTextureLevel level, const FilePath & textureName)
{
	SafeRelease(textures[level]);
	textureNames[level] = String("");

	if((TILED_MODE_TILEMASK == tiledShaderMode || TILED_MODE_TILE_DETAIL_MASK == tiledShaderMode) && TEXTURE_TILE_FULL == level)
		return;

    Texture * texture = CreateTexture(level, textureName);
    if (texture)
    {
        textureNames[level] = textureName;
    }
    textures[level] = texture;
    
    if((TEXTURE_TILE_FULL == level) && ((tiledShaderMode == TILED_MODE_MIXED) || (tiledShaderMode == TILED_MODE_TEXTURE)))
    {
        UpdateFullTiledTexture();
    }
}
    
Texture * Landscape::CreateTexture(eTextureLevel level, const FilePath & textureName)
{
    if(TEXTURE_TILE_FULL == level)
    {
        //must not zero only for finalized maps
        if(!textureName.IsEmpty())
        {
            return Texture::PureCreate(textureName);
        }
        return NULL;
    }

    return Texture::CreateFromFile(textureName);
}


void Landscape::SetTexture(eTextureLevel level, Texture *texture)
{
    SafeRelease(textures[level]);
	textureNames[level] = String("");

	textures[level] = SafeRetain(texture);
    if(textures[level])
    {
        if(!textures[level]->isRenderTarget)
        {
            textureNames[level] = textures[level]->GetPathname();
        }
    }
}

    
Texture * Landscape::GetTexture(eTextureLevel level)
{
	return textures[level];
}
    
void Landscape::FlushQueue()
{
    if (queueRenderCount == 0)return;
    
    RenderManager::Instance()->SetRenderData(landscapeRDOArray[queueRdoQuad]);
    RenderManager::Instance()->FlushState();
	RenderManager::Instance()->AttachRenderData();
    RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, queueRenderCount, EIF_16, indices); 

    drawIndices += queueRenderCount;
    
    ClearQueue();
    
    ++flashQueueCounter;
}
    
void Landscape::ClearQueue()
{
    queueRenderCount = 0;
    queueRdoQuad = -1;
    queueDrawIndices = indices;
}

void Landscape::DrawQuad(LandQuadTreeNode<LandscapeQuad> * currentNode, int8 lod)
{
    int32 depth = currentNode->data.size / (1 << lod);
    if (depth == 1)
    {
        currentNode->parent->data.frame = Core::Instance()->GetGlobalFrameIndex();
    }else
    {
        //int32 newdepth = (int)(logf((float)depth) / logf(2.0f) + 0.5f);
        int32 newdepth2 = CountLeadingZeros(depth);
        //Logger::FrameworkDebug("dp: %d %d %d", depth, newdepth, newdepth2);
        //DVASSERT(newdepth == newdepth2); // Check of math, we should use optimized version with depth2
        
        MarkFrames(currentNode, newdepth2);
    }
    
    int32 step = Min((int16)(1 << lod), currentNode->data.size);
    
    if ((currentNode->data.rdoQuad != queueRdoQuad) && (queueRdoQuad != -1))
    {
        FlushQueue();
    }
    
    queueRdoQuad = currentNode->data.rdoQuad;
    
    //int16 width = heightmap->GetWidth();
    for (uint16 y = (currentNode->data.y & RENDER_QUAD_AND); y < (currentNode->data.y & RENDER_QUAD_AND) + currentNode->data.size; y += step)
        for (uint16 x = (currentNode->data.x & RENDER_QUAD_AND); x < (currentNode->data.x & RENDER_QUAD_AND) + currentNode->data.size; x += step)
        {
            *queueDrawIndices++ = x + y * RENDER_QUAD_WIDTH;
            *queueDrawIndices++ = (x + step) + y * RENDER_QUAD_WIDTH;
            *queueDrawIndices++ = x + (y + step) * RENDER_QUAD_WIDTH;
            
            *queueDrawIndices++ = (x + step) + y * RENDER_QUAD_WIDTH;
            *queueDrawIndices++ = (x + step) + (y + step) * RENDER_QUAD_WIDTH;
            *queueDrawIndices++ = x + (y + step) * RENDER_QUAD_WIDTH;     
 
            queueRenderCount += 6;
        }
    
    DVASSERT(queueRenderCount < INDEX_ARRAY_COUNT);
}
    
void Landscape::DrawFans()
{
    uint32 currentFrame = Core::Instance()->GetGlobalFrameIndex();;
    int16 width = RENDER_QUAD_WIDTH;//heightmap->GetWidth();
    
    ClearQueue();
    
    List<LandQuadTreeNode<LandscapeQuad>*>::const_iterator end = fans.end();
    for (List<LandQuadTreeNode<LandscapeQuad>*>::iterator t = fans.begin(); t != end; ++t)
    {
        //uint16 * drawIndices = indices;
        LandQuadTreeNode<LandscapeQuad>* node = *t;
        
        //RenderManager::Instance()->SetRenderData(landscapeRDOArray[node->data.rdoQuad]);
        
        if ((node->data.rdoQuad != queueRdoQuad) && (queueRdoQuad != -1))
        {
            
            FlushQueue();
        }
        queueRdoQuad = node->data.rdoQuad;
        
        //int32 count = 0;
        int16 halfSize = (node->data.size >> 1);
        int16 xbuf = node->data.x & RENDER_QUAD_AND;
        int16 ybuf = node->data.y & RENDER_QUAD_AND;
        
        
#define ADD_VERTEX(index) queueDrawIndices[queueRenderCount++] = (index);
        
        //drawIndices[count++] = (xbuf + halfSize) + (ybuf + halfSize) * width;
        //drawIndices[count++] = (xbuf) + (ybuf) * width;

        ADD_VERTEX((xbuf + halfSize) + (ybuf + halfSize) * width);
        ADD_VERTEX((xbuf) + (ybuf) * width);
        
        if ((node->neighbours[TOP]) && (node->neighbours[TOP]->data.frame == currentFrame))
        {
            ADD_VERTEX((xbuf + halfSize) + (ybuf) * width);
            ADD_VERTEX((xbuf + halfSize) + (ybuf + halfSize) * width);
            ADD_VERTEX((xbuf + halfSize) + (ybuf) * width);
        }
        
        ADD_VERTEX((xbuf + node->data.size) + (ybuf) * width);
        ADD_VERTEX((xbuf + halfSize) + (ybuf + halfSize) * width);
        ADD_VERTEX((xbuf + node->data.size) + (ybuf) * width);

        
        if ((node->neighbours[RIGHT]) && (node->neighbours[RIGHT]->data.frame == currentFrame))
        {
            ADD_VERTEX((xbuf + node->data.size) + (ybuf + halfSize) * width);
            ADD_VERTEX((xbuf + halfSize) + (ybuf + halfSize) * width);
            ADD_VERTEX((xbuf + node->data.size) + (ybuf + halfSize) * width);
        }

        ADD_VERTEX((xbuf + node->data.size) + (ybuf + node->data.size) * width);
        ADD_VERTEX((xbuf + halfSize) + (ybuf + halfSize) * width);
        ADD_VERTEX((xbuf + node->data.size) + (ybuf + node->data.size) * width);
        
        if ((node->neighbours[BOTTOM]) && (node->neighbours[BOTTOM]->data.frame == currentFrame))
        {
            ADD_VERTEX((xbuf + halfSize) + (ybuf + node->data.size) * width);
            ADD_VERTEX((xbuf + halfSize) + (ybuf + halfSize) * width);
            ADD_VERTEX((xbuf + halfSize) + (ybuf + node->data.size) * width);
        }
        
        ADD_VERTEX((xbuf) + (ybuf + node->data.size) * width);
        ADD_VERTEX((xbuf + halfSize) + (ybuf + halfSize) * width);
        ADD_VERTEX((xbuf) + (ybuf + node->data.size) * width);
        
        if ((node->neighbours[LEFT]) && (node->neighbours[LEFT]->data.frame == currentFrame))
        {
            ADD_VERTEX((xbuf) + (ybuf + halfSize) * width);
            ADD_VERTEX((xbuf + halfSize) + (ybuf + halfSize) * width);
            ADD_VERTEX((xbuf) + (ybuf + halfSize) * width);
        }

        ADD_VERTEX((xbuf) + (ybuf) * width);
        
#undef ADD_VERTEX
        //RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
//        RenderManager::Instance()->SetRenderData(landscapeRDOArray[node->data.rdoQuad]);
//        RenderManager::Instance()->FlushState();
//        RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, count, EIF_16, indices); 
    }
    
    FlushQueue();
    
/*  DRAW TRIANGLE FANS
    List<LandQuadTreeNode<LandscapeQuad>*>::const_iterator end = fans.end();
    for (List<LandQuadTreeNode<LandscapeQuad>*>::iterator t = fans.begin(); t != end; ++t)
    {
        uint16 * drawIndices = indices;
        LandQuadTreeNode<LandscapeQuad>* node = *t;
        
        RenderManager::Instance()->SetRenderData(landscapeRDOArray[node->data.rdoQuad]);

        int32 count = 0;
        int16 halfSize = (node->data.size >> 1);
        int16 xbuf = node->data.x & RENDER_QUAD_AND;
        int16 ybuf = node->data.y & RENDER_QUAD_AND;
        
        drawIndices[count++] = (xbuf + halfSize) + (ybuf + halfSize) * width;
        drawIndices[count++] = (xbuf) + (ybuf) * width;
        
        if ((node->neighbours[TOP]) && (node->neighbours[TOP]->data.frame == currentFrame))
            drawIndices[count++] = (xbuf + halfSize) + (ybuf) * width;
        
        drawIndices[count++] = (xbuf + node->data.size) + (ybuf) * width;
        
        if ((node->neighbours[RIGHT]) && (node->neighbours[RIGHT]->data.frame == currentFrame))
            drawIndices[count++] = (xbuf + node->data.size) + (ybuf + halfSize) * width;
            
        drawIndices[count++] = (xbuf + node->data.size) + (ybuf + node->data.size) * width;
        
        if ((node->neighbours[BOTTOM]) && (node->neighbours[BOTTOM]->data.frame == currentFrame))
            drawIndices[count++] = (xbuf + halfSize) + (ybuf + node->data.size) * width;

        drawIndices[count++] = (xbuf) + (ybuf + node->data.size) * width;
        
        if ((node->neighbours[LEFT]) && (node->neighbours[LEFT]->data.frame == currentFrame))
            drawIndices[count++] = (xbuf) + (ybuf + halfSize) * width;

        drawIndices[count++] = (xbuf) + (ybuf) * width;
        
        //RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
        RenderManager::Instance()->FlushState();
        RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLEFAN, count, EIF_16, indices); 
    }
 */
}
	
int Landscape::GetMaxLod(float32 quadDistance)
{
	int32 maxLod = 0;
		
	for (int32 k = 0; k < lodLevelsCount; ++k)
	{
		if (quadDistance > lodSqDistance[k])
		{
			maxLod = k + 1;
		}
		else //DF-1863 (stop checking for max lod when distance is less than in lodSqDistance since lodSqDistance is sorted)
		{
			break;
		}
	}
		
	return maxLod;
}
	
float32 Landscape::GetQuadToCameraDistance(const Vector3& camPos, const LandscapeQuad& quad)
{
	Vector3 v = camPos - quad.bbox.max;
	float32 dist0 = v.SquareLength();
	
	v = camPos - quad.bbox.min;
	float32 dist1 = v.SquareLength();
	
	dist0 = Max(dist0, dist1);
	return dist0;
}
	
void Landscape::Draw(LandQuadTreeNode<LandscapeQuad> * currentNode, uint8 clippingFlags)
{
    //Frustum * frustum = scene->GetClipCamera()->GetFrustum();
    // if (!frustum->IsInside(currentNode->data.bbox))return;
    Frustum::eFrustumResult frustumRes = Frustum::EFR_INSIDE; 
    
    if (currentNode->data.size >= 2)
		if (clippingFlags)
			frustumRes = frustum->Classify(currentNode->data.bbox, clippingFlags, currentNode->data.startClipPlane);		
    
    if (frustumRes == Frustum::EFR_OUTSIDE)return;
    
    /*
        If current quad do not have geometry just traverse it childs. 
        Magic starts when we have a geometry
     */
    if (currentNode->data.rdoQuad == -1)
    {
        if (currentNode->children)
        {
            for (int32 index = 0; index < 4; ++index)
            {
                LandQuadTreeNode<LandscapeQuad> * child = &currentNode->children[index];
                Draw(child, clippingFlags); 
            }
        }
        return;
    }
    /*
        // UNCOMMENT THIS TO CHECK GEOMETRY WITH 0 LEVEL OF DETAIL
        else
        {
            DrawQuad(currentNode, 0);
            return;
        }
     */
    
    /*
        We can be here only if we have a geometry in the node. 
        Here we use Geomipmaps rendering algorithm. 
        These quads are 129x129.
     */
//    Camera * cam = clipCamera;
    
    Vector3 corners[8];
    currentNode->data.bbox.GetCorners(corners);
    
    float32 minDist =  100000000.0f;
    float32 maxDist = -100000000.0f;
    for (int32 k = 0; k < 8; ++k)
    {
        Vector3 v = cameraPos - corners[k];
        float32 dist = v.SquareLength();
        if (dist < minDist)
            minDist = dist;
        if (dist > maxDist)
            maxDist = dist;
    };
    
    int32 minLod = 0;
    int32 maxLod = 0;
    
    for (int32 k = 0; k < lodLevelsCount; ++k)
    {
        if (minDist > lodSqDistance[k])
            minLod = k + 1;
        if (maxDist > lodSqDistance[k])
            maxLod = k + 1;
    }
	
	//VI: this check should fix occasional cracks on the landscape
	//VI: DF-1864 - select max distance from camera to quad and calculate max lod for that distance only
	/*if(minLod == maxLod)
	{
		LandQuadTreeNode<LandscapeQuad> * parentNode = currentNode->parent;
		float32 maxQuadDistance = -1.0f;
		
		if(parentNode)
		{
			if(parentNode->neighbours[LEFT])
			{
				maxQuadDistance = GetQuadToCameraDistance(cameraPos, parentNode->neighbours[LEFT]->data);
			}
			
			if(parentNode->neighbours[RIGHT])
			{
				maxQuadDistance = Max(maxQuadDistance, GetQuadToCameraDistance(cameraPos, parentNode->neighbours[RIGHT]->data));
			}
			
			if(parentNode->neighbours[TOP])
			{
				maxQuadDistance = Max(maxQuadDistance, GetQuadToCameraDistance(cameraPos, parentNode->neighbours[TOP]->data));
			}
			
			if(parentNode->neighbours[BOTTOM])
			{
				maxQuadDistance = Max(maxQuadDistance, GetQuadToCameraDistance(cameraPos, parentNode->neighbours[BOTTOM]->data));
			}
			
			if(maxQuadDistance >= 0.0f)
			{
				maxLod = Max(maxLod, GetMaxLod(maxQuadDistance));
			}
		}
    }*/
    
    // debug block
#if 1
    if (currentNode == &quadTreeHead)
    {
        //Logger::FrameworkDebug("== draw start ==");
    }
    //Logger::FrameworkDebug("%f %f %d %d", minDist, maxDist, minLod, maxLod);
#endif

//    if (frustum->IsFullyInside(currentNode->data.bbox))
//    {
//        RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
//        RenderHelper::Instance()->DrawBox(currentNode->data.bbox);
//    }
    
    
    if ((minLod == maxLod) && (/*frustum->IsFullyInside(currentNode->data.bbox)*/(frustumRes == Frustum::EFR_INSIDE) || currentNode->data.size <= (1 << maxLod) + 1) )
    {
        //Logger::FrameworkDebug("lod: %d depth: %d pos(%d, %d)", minLod, currentNode->data.lod, currentNode->data.x, currentNode->data.y);
        
//        if (currentNode->data.size <= (1 << maxLod))
//            RenderManager::Instance()->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
//        if (frustum->IsFullyInside(currentNode->data.bbox))
//            RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
        //if (frustum->IsFullyInside(currentNode->data.bbox) && (currentNode->data.size <= (1 << maxLod)))
        //    RenderManager::Instance()->SetColor(1.0f, 1.0f, 0.0f, 1.0f);

            
        //RenderManager::Instance()->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
#if defined (DRAW_OLD_STYLE)        
        DrawQuad(currentNode, maxLod);
#else //#if defined (DRAW_OLD_STYLE)        
        currentNode->data.lod = maxLod;
        if(maxLod)
        {
            lodNot0quads.push_back(currentNode);
        }
        else 
        {
            lod0quads.push_back(currentNode);
        }
#endif //#if defined (DRAW_OLD_STYLE)
        
        //RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
        //RenderHelper::Instance()->DrawBox(currentNode->data.bbox);

        return;
    }

    
    if ((minLod != maxLod) && (currentNode->data.size <= (1 << maxLod) + 1))
    {
//        RenderManager::Instance()->SetColor(0.0f, 0.0f, 1.0f, 1.0f);
        //DrawQuad(currentNode, minLod);
        //RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
        //RenderHelper::Instance()->DrawBox(currentNode->data.bbox);
        fans.push_back(currentNode);
        return;
    }
    
    //
    // Check performance and identify do we need a sorting here. 
    // Probably sorting algorithm can be helpfull to render on Mac / Windows, but will be useless for iOS and Android
    //
    
    {
        if (currentNode->children)
        {
            for (int32 index = 0; index < 4; ++index)
            {
                LandQuadTreeNode<LandscapeQuad> * child = &currentNode->children[index];
                Draw(child, clippingFlags); 
            }
        }
        /* EXPERIMENTAL => reduce level of quadtree, results was not successfull 
         else
        {
            DrawQuad(currentNode, maxLod);  
        }*/
    }
}

    
void Landscape::BindMaterial(int32 lodLayer)
{
    if(-1 != prevLodLayer)
    {
        UnbindMaterial();
    }

    if(0 == lodLayer)
    {
        if (textures[TEXTURE_TILE0])
            RenderManager::Instance()->SetTexture(textures[TEXTURE_TILE0], 0);
        if (textures[TEXTURE_TILE1])
            RenderManager::Instance()->SetTexture(textures[TEXTURE_TILE1], 1);
        if (textures[TEXTURE_TILE2])
            RenderManager::Instance()->SetTexture(textures[TEXTURE_TILE2], 2);
        if (textures[TEXTURE_TILE3])
            RenderManager::Instance()->SetTexture(textures[TEXTURE_TILE3], 3);
        if (textures[TEXTURE_TILE_MASK])
            RenderManager::Instance()->SetTexture(textures[TEXTURE_TILE_MASK], 4);
        if (textures[TEXTURE_COLOR])
            RenderManager::Instance()->SetTexture(textures[TEXTURE_COLOR], 5);
        
        RenderManager::Instance()->SetShader(tileMaskShader);
        RenderManager::Instance()->FlushState();
        
        if (uniformTextures[TEXTURE_TILE0] != -1)
            tileMaskShader->SetUniformValueByIndex(uniformTextures[TEXTURE_TILE0], 0);
        
        if (uniformTextures[TEXTURE_TILE1] != -1)
            tileMaskShader->SetUniformValueByIndex(uniformTextures[TEXTURE_TILE1], 1);
        
        if (uniformTextures[TEXTURE_TILE2] != -1)
            tileMaskShader->SetUniformValueByIndex(uniformTextures[TEXTURE_TILE2], 2);
        
        if (uniformTextures[TEXTURE_TILE3] != -1)
            tileMaskShader->SetUniformValueByIndex(uniformTextures[TEXTURE_TILE3], 3);
        
        if (uniformTextures[TEXTURE_TILE_MASK] != -1)
            tileMaskShader->SetUniformValueByIndex(uniformTextures[TEXTURE_TILE_MASK], 4);
        
        if (uniformTextures[TEXTURE_COLOR] != -1)
            tileMaskShader->SetUniformValueByIndex(uniformTextures[TEXTURE_COLOR], 5);
        
        if (uniformCameraPosition != -1)
            tileMaskShader->SetUniformValueByIndex(uniformCameraPosition, cameraPos);
        
        if (uniformTextureTiling[TEXTURE_TILE0] != -1)
            tileMaskShader->SetUniformValueByIndex(uniformTextureTiling[TEXTURE_TILE0], textureTiling[TEXTURE_TILE0]);
        
        if (uniformTextureTiling[TEXTURE_TILE1] != -1)
            tileMaskShader->SetUniformValueByIndex(uniformTextureTiling[TEXTURE_TILE1], textureTiling[TEXTURE_TILE1]);
        
        if (uniformTextureTiling[TEXTURE_TILE2] != -1)
            tileMaskShader->SetUniformValueByIndex(uniformTextureTiling[TEXTURE_TILE2], textureTiling[TEXTURE_TILE2]);
        
        if (uniformTextureTiling[TEXTURE_TILE3] != -1)
            tileMaskShader->SetUniformValueByIndex(uniformTextureTiling[TEXTURE_TILE3], textureTiling[TEXTURE_TILE3]);

        
        if (uniformTileColor[TEXTURE_TILE0] != -1)
            tileMaskShader->SetUniformColor3ByIndex(uniformTileColor[TEXTURE_TILE0], tileColor[TEXTURE_TILE0]);
        if (uniformTileColor[TEXTURE_TILE1] != -1)
            tileMaskShader->SetUniformColor3ByIndex(uniformTileColor[TEXTURE_TILE1], tileColor[TEXTURE_TILE1]);
        if (uniformTileColor[TEXTURE_TILE2] != -1)
            tileMaskShader->SetUniformColor3ByIndex(uniformTileColor[TEXTURE_TILE2], tileColor[TEXTURE_TILE2]);
        if (uniformTileColor[TEXTURE_TILE3] != -1)
            tileMaskShader->SetUniformColor3ByIndex(uniformTileColor[TEXTURE_TILE3], tileColor[TEXTURE_TILE3]);
                
        
        if (uniformFogColor != -1)
            tileMaskShader->SetUniformColor3ByIndex(uniformFogColor, fogColor);
        if (uniformFogDensity != -1)
            tileMaskShader->SetUniformValueByIndex(uniformFogDensity, fogDensity);
    }
    else 
    {
        if (textures[TEXTURE_TILE_FULL])
            RenderManager::Instance()->SetTexture(textures[TEXTURE_TILE_FULL], 0);

        RenderManager::Instance()->SetShader(fullTiledShader);
        RenderManager::Instance()->FlushState();
        
        if (uniformFogColorFT != -1)
            fullTiledShader->SetUniformColor3ByIndex(uniformFogColorFT, fogColor);
        if (uniformFogDensityFT != -1)
            fullTiledShader->SetUniformValueByIndex(uniformFogDensityFT, fogDensity);
    }
    
    prevLodLayer = lodLayer;
}

void Landscape::UnbindMaterial()
{
    if(-1 != prevLodLayer)
    {
        if(0 == prevLodLayer)
        {
            RenderManager::Instance()->SetTexture(0, 0);
            RenderManager::Instance()->SetTexture(0, 1);
            RenderManager::Instance()->SetTexture(0, 2);
            RenderManager::Instance()->SetTexture(0, 3);
            RenderManager::Instance()->SetTexture(0, 4);
            RenderManager::Instance()->SetTexture(0, 5);
            
            RenderManager::Instance()->SetShader(NULL);
            RenderManager::Instance()->FlushState();
        }
        else
        {
            RenderManager::Instance()->SetTexture(0, 0);
            
            RenderManager::Instance()->SetShader(NULL);
            RenderManager::Instance()->FlushState();
        }
        
        prevLodLayer = -1;
    }
}

    
    
void Landscape::Draw(Camera * camera)
{
    drawIndices = 0;
    
    TIME_PROFILE("LandscapeNode.Draw");

	if(!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::LANDSCAPE_DRAW))
	{
		return;
	}

	//Dizz: uniformFogDensity != -1 is a check if fog is inabled in shader
	if(isFogEnabled && (uniformFogDensity != -1) && !RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::FOG_ENABLE))
	{
		InitShaders();
	}

	if(isFogEnabled && (uniformFogDensity == -1) && RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::FOG_ENABLE))
	{
		InitShaders();
	}
    
#if defined(__DAVAENGINE_OPENGL__) && (defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__))
//    if (debugFlags & DEBUG_DRAW_GRID)
//    {
//        RenderManager::Instance()->SetColor(1.0f, 0.f, 0.f, 1.f);
//        RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);
//        RenderManager::Instance()->SetShader(0);
//        RenderManager::Instance()->FlushState();
//
//        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//    }
#endif //#if defined(__DAVAENGINE_OPENGL__)
    
    //SceneNode::Draw();

#if defined(__DAVAENGINE_OPENGL__) && (defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__))
//    if (!(debugFlags & DEBUG_DRAW_GRID))
//    {
//        RenderManager::Instance()->ResetColor();
//    }
#else
    RenderManager::Instance()->ResetColor();
#endif //#if defined(__DAVAENGINE_OPENGL__)


    ClearQueue();

    
    
/*
    Boroda: I do not understand why, but this code breaks frustrum culling on landscape.
    I've spent an hour, trying to understand what's going on, without luck. 
 */
    
//	Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW);
//	Matrix4 meshFinalMatrix = worldTransform * prevMatrix;
//    //frustum->Set(meshFinalMatrix * RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_PROJECTION));
//    cameraPos = scene->GetClipCamera()->GetPosition() * worldTransform;
//
//    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, meshFinalMatrix);
//    frustum->Set();

    frustum = camera->GetFrustum();
    cameraPos = camera->GetPosition();        
    
    flashQueueCounter = 0;
    
#if defined (DRAW_OLD_STYLE)    
    BindMaterial(0);
	Draw(&quadTreeHead);
#else //#if defined (DRAW_OLD_STYLE)   
    


    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, camera->GetMatrix());

	if(RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_LANDSCAPE_LODS))
	{
		fans.clear();
		lod0quads.clear();
		lodNot0quads.clear();

		Draw(&quadTreeHead, 0x3f);
	}
    
    BindMaterial(nearLodIndex);
    int32 count0 = lod0quads.size();
    for(int32 i = 0; i < count0; ++i)
    {
        DrawQuad(lod0quads[i], 0);
    }
	FlushQueue();
    
    if(nearLodIndex != farLodIndex)     
	{
		BindMaterial(farLodIndex);
	}

    int32 countNot0 = lodNot0quads.size();
    for(int32 i = 0; i < countNot0; ++i)
    {
        DrawQuad(lodNot0quads[i], lodNot0quads[i]->data.lod);
    }
#endif //#if defined (DRAW_OLD_STYLE)    

    
	FlushQueue();
    //    Logger::FrameworkDebug("[LN] flashQueueCounter = %d", flashQueueCounter);
	DrawFans();
    
#if defined(__DAVAENGINE_MACOS__)
//    if (debugFlags & DEBUG_DRAW_ALL)
//    {
//        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//    }   
#endif

	if(cursor)
	{
		RenderManager::Instance()->AppendState(RenderState::STATE_BLEND);
		eBlendMode src = RenderManager::Instance()->GetSrcBlend();
		eBlendMode dst = RenderManager::Instance()->GetDestBlend();
		RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		RenderManager::Instance()->SetDepthFunc(CMP_LEQUAL);		
		cursor->Prepare();
		ClearQueue();

        
#if defined (DRAW_OLD_STYLE)    
        Draw(&quadTreeHead);
#else //#if defined (DRAW_OLD_STYLE)            
        
        if(nearLodIndex != farLodIndex)     
		{
			BindMaterial(nearLodIndex);
		}
        int32 count0 = lod0quads.size();
        for(int32 i = 0; i < count0; ++i)
        {
            DrawQuad(lod0quads[i], 0);
        }
        FlushQueue();
        
        if(nearLodIndex != farLodIndex)
		{
			BindMaterial(farLodIndex);
		}
        
        int32 countNot0 = lodNot0quads.size();
        for(int32 i = 0; i < countNot0; ++i)
        {
            DrawQuad(lodNot0quads[i], lodNot0quads[i]->data.lod);
        }
#endif //#if defined (DRAW_OLD_STYLE)    

        
		FlushQueue();
		DrawFans();
		RenderManager::Instance()->SetDepthFunc(CMP_LESS);
		RenderManager::Instance()->RemoveState(RenderState::STATE_BLEND);
		RenderManager::Instance()->SetBlendMode(src, dst);
	}
    
    UnbindMaterial();

//#if defined(__DAVAENGINE_MACOS__)
//    if (debugFlags & DEBUG_DRAW_ALL)
//    {
//        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//        glEnable(GL_POLYGON_OFFSET_LINE);
//        glPolygonOffset(0.0f, 1.0f);
//        RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);
//        fans.clear();
//        Draw(&quadTreeHead);
//        DrawFans();
//        glDisable(GL_POLYGON_OFFSET_LINE);
//        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//    }   
//#endif
    
    //RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);
    //uint64 drawTime = SystemTimer::Instance()->AbsoluteMS() - time;
    //Logger::FrameworkDebug("landscape draw time: %lld", drawTime);
}


void Landscape::GetGeometry(Vector<LandscapeVertex> & landscapeVertices, Vector<int32> & indices)
{
	LandQuadTreeNode<LandscapeQuad> * currentNode = &quadTreeHead;
	LandscapeQuad * quad = &currentNode->data;
	
	landscapeVertices.resize((quad->size + 1) * (quad->size + 1));

	int32 index = 0;
	for (int32 y = quad->y; y < quad->y + quad->size + 1; ++y)
	{
		for (int32 x = quad->x; x < quad->x + quad->size + 1; ++x)
		{
			landscapeVertices[index].position = GetPoint(x, y, heightmap->Data()[y * heightmap->Size() + x]);
			landscapeVertices[index].texCoord = Vector2((float32)x / (float32)(heightmap->Size() - 1), (float32)y / (float32)(heightmap->Size() - 1));           
			index++;
		}
	}

	indices.resize(heightmap->Size()*heightmap->Size()*6);
	int32 step = 1;
	int32 indexIndex = 0;
	int32 quadWidth = heightmap->Size();
	for(int32 y = 0; y < currentNode->data.size-1; y += step)
	{
		for(int32 x = 0; x < currentNode->data.size-1; x += step)
		{
			indices[indexIndex++] = x + y * quadWidth;
			indices[indexIndex++] = (x + step) + y * quadWidth;
			indices[indexIndex++] = x + (y + step) * quadWidth;

			indices[indexIndex++] = (x + step) + y * quadWidth;
			indices[indexIndex++] = (x + step) + (y + step) * quadWidth;
			indices[indexIndex++] = x + (y + step) * quadWidth;     
		}
	}
}

//AABBox3 LandscapeNode::GetWTMaximumBoundingBox()
//{
////    AABBox3 retBBox = box;
////    box.GetTransformedBox(GetWorldTransform(), retBBox);
////
////    const Vector<SceneNode*>::iterator & itEnd = children.end();
////    for (Vector<SceneNode*>::iterator it = children.begin(); it != itEnd; ++it)
////    {
////        AABBox3 lbox = (*it)->GetWTMaximumBoundingBoxSlow();
////        if(  (AABBOX_INFINITY != lbox.min.x && AABBOX_INFINITY != lbox.min.y && AABBOX_INFINITY != lbox.min.z)
////           &&(-AABBOX_INFINITY != lbox.max.x && -AABBOX_INFINITY != lbox.max.y && -AABBOX_INFINITY != lbox.max.z))
////        {
////            retBBox.AddAABBox(lbox);
////        }
////    }
//    
//    return retBBox;
//}

const FilePath & Landscape::GetHeightmapPathname()
{
    return heightmapPath;
}
    
void Landscape::Save(KeyedArchive * archive, SceneFileV2 * sceneFile)
{
    RenderObject::Save(archive, sceneFile);
        
    //TODO: remove code in future. Need for transition from *.png to *.heightmap
    if(!heightmapPath.IsEqualToExtension(Heightmap::FileExtension()))
    {
        heightmapPath.ReplaceExtension(Heightmap::FileExtension());
    }
    //

	heightmap->Save(heightmapPath);

    archive->SetString("hmap", heightmapPath.GetRelativePathname(sceneFile->GetScenePath()));
    archive->SetInt32("tiledShaderMode", tiledShaderMode);
    
    archive->SetByteArrayAsType("bbox", bbox);
    for (int32 k = 0; k < TEXTURE_COUNT; ++k)
    {
        if(TEXTURE_DETAIL == k) continue;

        String relPath  = textureNames[k].GetRelativePathname(sceneFile->GetScenePath());
        
        if(sceneFile->DebugLogEnabled())
            Logger::FrameworkDebug("landscape tex save: %s rel: %s", textureNames[k].GetAbsolutePathname().c_str(), relPath.c_str());
        
        archive->SetString(Format("tex_%d", k), relPath);
        archive->SetByteArrayAsType(Format("tiling_%d", k), textureTiling[k]);
		archive->SetByteArrayAsType(Format("tilecolor_%d", k), tileColor[k]);
    }
    
    archive->SetByteArrayAsType("fogcolor", fogColor);
    archive->SetFloat("fogdencity", fogDensity);
    archive->SetBool("isFogEnabled", isFogEnabled);
}
    
void Landscape::Load(KeyedArchive * archive, SceneFileV2 * sceneFile)
{
	RenderObject::Load(archive, sceneFile);

    FilePath path(sceneFile->GetScenePath());
    path += archive->GetString("hmap");

    AABBox3 boxDef;
    boxDef = archive->GetByteArrayAsType("bbox", boxDef);
    
    eTiledShaderMode tiledMode = (eTiledShaderMode)archive->GetInt32("tiledShaderMode", TILED_MODE_MIXED);
    SetTiledShaderMode(tiledMode);
    
    fogColor = archive->GetByteArrayAsType("fogcolor", fogColor);
	isFogEnabled = archive->GetBool("isFogEnabled", isFogEnabled);
    fogDensity = archive->GetFloat("fogdencity", fogDensity);

    BuildLandscapeFromHeightmapImage(path, boxDef);
        
    for (int32 k = 0; k < TEXTURE_COUNT; ++k)
    {
        if(TEXTURE_DETAIL == k) continue;
        
        if(TEXTURE_TILE_FULL == k && tiledShaderMode != TILED_MODE_TEXTURE && tiledShaderMode != TILED_MODE_MIXED)
            continue;

        // load textures
        if(!(tiledShaderMode == TILED_MODE_TILE_DETAIL_MASK && (TEXTURE_TILE1 == k || TEXTURE_TILE2 == k || TEXTURE_TILE3 == k)))
        {
            String textureName = archive->GetString(Format("tex_%d", k));

            FilePath absPath;
            if(!textureName.empty())
            {
                absPath = sceneFile->GetScenePath();
                absPath += textureName;
            }

            if(sceneFile->DebugLogEnabled())
                Logger::FrameworkDebug("landscape tex %d load: %s abs:%s", k, textureName.c_str(), absPath.GetAbsolutePathname().c_str());
            
            if (sceneFile->GetVersion() >= 4)
            {
                SetTexture((eTextureLevel)k, absPath);
            }
            else
            {
                if ((k == 0) || (k == 1)) // if texture 0 or texture 1, move them to TILE0, TILE1
                    SetTexture((eTextureLevel)(k + 2), absPath);
                
                if (k == 3)
                    SetTexture(TEXTURE_COLOR, absPath);
            }
        }
        
        //load tiles
        if (sceneFile->GetVersion() >= 4)
        {
            textureTiling[k] = archive->GetByteArrayAsType(Format("tiling_%d", k), textureTiling[k]);
			tileColor[k] = archive->GetByteArrayAsType(Format("tilecolor_%d", k), tileColor[k]);
        }
        else
        {
            if ((k == 0) || (k == 1))
                textureTiling[k] = archive->GetByteArrayAsType(Format("tiling_%d", k), textureTiling[k]);
        }
    }
}

const FilePath & Landscape::GetTextureName(DAVA::Landscape::eTextureLevel level)
{
    DVASSERT(0 <= level && level < TEXTURE_COUNT);
    return textureNames[level];
}
    
void Landscape::SetTextureName(eTextureLevel level, const FilePath &newTextureName)
{
    DVASSERT(0 <= level && level < TEXTURE_COUNT);
    textureNames[level] = newTextureName;
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

void Landscape::SetCursorTexture(Texture * texture)
{
	cursor->SetCursorTexture(texture);
}

void Landscape::SetCursorPosition(const Vector2 & position)
{
	cursor->SetPosition(position);
}

void Landscape::SetCursorScale(float32 scale)
{
	cursor->SetScale(scale);
}

void Landscape::SetBigTextureSize(float32 bigSize)
{
	cursor->SetBigTextureSize(bigSize);
}
    
Heightmap * Landscape::GetHeightmap()
{
    return heightmap;
}

void Landscape::SetHeightmap(DAVA::Heightmap *height)
{
    SafeRelease(heightmap);
    
    ReleaseShaders(); // release previous shaders
    ReleaseAllRDOQuads();
    InitShaders(); // init new shaders according to the selected rendering mode
    
    SafeDeleteArray(indices);

    heightmap = SafeRetain(height);
    BuildLandscape();
}
    
    
Texture * Landscape::CreateFullTiledTexture()
{
    bool savedIsFogEnabled = isFogEnabled;
    SetFog(false);

    //Set indexes
    Vector<float32> ftVertexes;
    Vector<float32> ftTextureCoords;
    
    float32 x0 = 0;
    float32 y0 = 0;
    float32 x1 = TEXTURE_TILE_FULL_SIZE;
    float32 y1 = TEXTURE_TILE_FULL_SIZE;
    
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
    RenderManager::Instance()->SetViewport(Rect(0.f, 0.f, (float32)fullTiled->GetWidth(), (float32)fullTiled->GetHeight()), true);


	RenderManager::Instance()->ClearWithColor(1.f, 1.f, 1.f, 1.f);
 
    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, Matrix4::IDENTITY);
    Matrix4 projection;
    projection.glOrtho(0, (float32)TEXTURE_TILE_FULL_SIZE, 0, (float32)TEXTURE_TILE_FULL_SIZE, 0, 1);
    
    Matrix4 oldProjection = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_PROJECTION);
    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_PROJECTION, projection);
    RenderManager::Instance()->SetState(RenderState::DEFAULT_2D_STATE);
    
    prevLodLayer = -1;
    BindMaterial(0);
    RenderManager::Instance()->SetRenderData(ftRenderData);
    RenderManager::Instance()->FlushState();
	RenderManager::Instance()->AttachRenderData();
    RenderManager::Instance()->HWDrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, 0, 4);
    UnbindMaterial();

#ifdef __DAVAENGINE_OPENGL__
	RenderManager::Instance()->HWglBindFBO(RenderManager::Instance()->GetFBOViewFramebuffer());
#endif //#ifdef __DAVAENGINE_OPENGL__
    
    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_PROJECTION, oldProjection);
	RenderManager::Instance()->SetViewport(oldViewport, true);
    SafeRelease(ftRenderData);

    SetFog(savedIsFogEnabled);
    return fullTiled;
}
    
FilePath Landscape::SaveFullTiledTexture()
{
    FilePath pathToSave;
    
    if(textures[TEXTURE_TILE_FULL])
    {
        if(textures[TEXTURE_TILE_FULL]->isRenderTarget)
        {
            pathToSave = GetTextureName(TEXTURE_COLOR);
            pathToSave.ReplaceExtension(".thumbnail.png");
            Image *image = textures[TEXTURE_TILE_FULL]->CreateImageFromMemory();
            if(image)
            {
                ImageLoader::Save(image, pathToSave);
                SafeRelease(image);
            }
        }
        else
        {
            pathToSave = textureNames[TEXTURE_TILE_FULL];
        }
    }
    
    Logger::FrameworkDebug("[LN] SaveFullTiledTexture: %s", pathToSave.GetAbsolutePathname().c_str());
    return pathToSave;
}
    
void Landscape::UpdateFullTiledTexture()
{
    if(textureNames[TEXTURE_TILE_FULL].IsEmpty())
    {
        Texture *t = CreateFullTiledTexture();
        t->GenerateMipmaps();
        SetTexture(TEXTURE_TILE_FULL, t);
        SafeRelease(t);
    }
}
    
void Landscape::SetTiledShaderMode(DAVA::Landscape::eTiledShaderMode _tiledShaderMode)
{
    tiledShaderMode = _tiledShaderMode;
    
    switch (tiledShaderMode)
    {
        case TILED_MODE_TILE_DETAIL_MASK:
            nearLodIndex = 0;
            farLodIndex = 0;
            break;
        case TILED_MODE_TILEMASK:
            nearLodIndex = 0;
            farLodIndex = 0;
            break;
            
        case TILED_MODE_MIXED:
            nearLodIndex = 0;
            farLodIndex = 1;
            break;

        case TILED_MODE_TEXTURE:
            nearLodIndex = 1;
            farLodIndex = 1;
            break;

        default:
            break;
    }
    // Reload shaders to
    InitShaders();
}
    
void Landscape::SetFog(const bool& fogState)
{
    if(isFogEnabled != fogState)
    {
        isFogEnabled = fogState;
        
        InitShaders();
    }
}

bool Landscape::IsFogEnabled() const
{
    return isFogEnabled;
}

void Landscape::SetFogDensity(float32 _fogDensity)
{
    fogDensity = _fogDensity;
}

float32 Landscape::GetFogDensity() const
{
    return fogDensity;
}

void Landscape::SetFogColor(const Color & _fogColor)
{
    fogColor = _fogColor;
}

const Color & Landscape::GetFogColor() const
{
    return fogColor;
}

LandscapeCursor * Landscape::GetCursor()
{
    return cursor;
}

RenderObject * Landscape::Clone( RenderObject *newObject )
{
	if(!newObject)
	{
		DVASSERT_MSG(IsPointerToExactClass<Landscape>(this), "Can clone only Landscape");
		newObject = new Landscape();
	}
    
    Landscape *newLandscape = static_cast<Landscape *>(newObject);

    newLandscape->SetTiledShaderMode((eTiledShaderMode)tiledShaderMode);
    
    newLandscape->fogColor = fogColor;
	newLandscape->isFogEnabled = isFogEnabled;
    newLandscape->fogDensity = fogDensity;
    
    newLandscape->BuildLandscapeFromHeightmapImage(heightmapPath, bbox);
    
    for (int32 k = 0; k < TEXTURE_COUNT; ++k)
    {
        newLandscape->textureNames[k] = textureNames[k];
		SafeRelease(newLandscape->textures[k]);
        newLandscape->textures[k] = SafeRetain(textures[k]);
        newLandscape->textureTiling[k] = textureTiling[k];
        newLandscape->tileColor[k] = tileColor[k];
    }

	return newObject;
}
	
int32 Landscape::GetDrawIndices() const
{
    return drawIndices;
}


};
