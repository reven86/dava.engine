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


#include "LandscapeRenderer.h"

using namespace DAVA;

LandscapeRenderer::LandscapeRenderer(DAVA::Heightmap *heightmap, const DAVA::AABBox3 &box)
    : BaseObject()
{
    landscapeRenderObject = new RenderDataObject();
    DVASSERT(landscapeRenderObject);
    
    InitShader();

    SetHeightmap(heightmap, box);
}

LandscapeRenderer::~LandscapeRenderer()
{
    SafeRelease(shader);
    uniformFogColor = -1;
    uniformFogDensity = -1;
    
    SafeRelease(landscapeRenderObject);
    SafeRelease(heightmap);
}

void LandscapeRenderer::InitShader()
{
    bool isFogEnabled = false;
    
    shader = SafeRetain(ShaderCache::Instance()->Get(FastName("~res:/Materials/Shaders/Landscape/fulltiled_texture"), FastNameSet()));
    
    if(isFogEnabled && RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::FOG_ENABLE))
    {
        uniformFogColor = shader->FindUniformIndexByName(DAVA::FastName("fogColor"));
		uniformFogDensity = shader->FindUniformIndexByName(DAVA::FastName("fogDensity"));
    }
    else
    {
        uniformFogColor = -1;
        uniformFogDensity = -1;
    }
}

void LandscapeRenderer::SetHeightmap(DAVA::Heightmap *heightmap, const DAVA::AABBox3 &box)
{
    DVASSERT(heightmap && "Cannot work without heightmap");

    this->heightmap = SafeRetain(heightmap);
    SetBoundingBox(box);
    
    vertices.resize(heightmap->Size() * heightmap->Size());
	indices.resize(heightmap->Size() * heightmap->Size() * 6);
    
    landscapeRenderObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, sizeof(Landscape::LandscapeVertex), &vertices[0].position);
	landscapeRenderObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, sizeof(Landscape::LandscapeVertex), &vertices[0].texCoord);
    
    RebuildVertexes(Rect(0.f, 0.f, (float32)heightmap->Size(), (float32)heightmap->Size()));
    RebuildIndexes();
}

void LandscapeRenderer::SetBoundingBox(const DAVA::AABBox3 &box)
{
    boundingBox = box;
    
    pointCoefficients.x = (boundingBox.max.x - boundingBox.min.x) / (float32)(heightmap->Size() - 1);
    pointCoefficients.y = (boundingBox.max.y - boundingBox.min.y) / (float32)(heightmap->Size() - 1);
    pointCoefficients.z = (boundingBox.max.z - boundingBox.min.z) / (float32)Heightmap::MAX_VALUE;

}

void LandscapeRenderer::RebuildVertexes(const DAVA::Rect &rebuildForRect)
{
    int32 lastY = (int32)(rebuildForRect.y + rebuildForRect.dy);
    int32 lastX = (int32)(rebuildForRect.x + rebuildForRect.dx);
    for (int32 y = (int32)rebuildForRect.y; y < lastY; ++y)
    {
        int32 index = y * heightmap->Size();
        float32 deltaY =  (float32)y / (float32)(heightmap->Size() - 1);
        for (int32 x = (int32)rebuildForRect.x; x < lastX; ++x)
        {
            vertices[index + x].position = GetPoint(x, y, heightmap->Data()[index + x]);
            vertices[index + x].texCoord = Vector2((float32)x / (float32)(heightmap->Size() - 1), 1.0f - deltaY);
        }
    }

}

Vector3 LandscapeRenderer::GetPoint(int16 x, int16 y, uint16 height)
{
    Vector3 res;
    res.x = boundingBox.min.x + (float32)x * pointCoefficients.x;
    res.y = boundingBox.min.y + (float32)y * pointCoefficients.y;
    res.z = boundingBox.min.z + (float32)height * pointCoefficients.z;
    return res;
};


void LandscapeRenderer::RebuildIndexes()
{
    int32 step = 1;
    int32 indexIndex = 0;
    int32 quadWidth = heightmap->Size();
    for(int32 y = 0; y < heightmap->Size() - 1; y += step)
    {
        for(int32 x = 0; x < heightmap->Size() - 1; x += step)
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

void LandscapeRenderer::DrawLandscape()
{
    RenderManager::Instance()->SetRenderData(landscapeRenderObject);
	RenderManager::Instance()->FlushState();
	RenderManager::Instance()->AttachRenderData();
	RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, (heightmap->Size() - 1) * (heightmap->Size() - 1) * 6, EIF_32, &indices.front());
}
    
void LandscapeRenderer::BindMaterial(DAVA::UniqueHandle textureStateHandle)
{
    RenderManager::Instance()->SetTextureState(textureStateHandle);
    
    RenderManager::Instance()->SetShader(shader);
    RenderManager::Instance()->FlushState();
    
    // TODO: Check what does it mean? 
    float32 fogDensity = 0.006f;
    Color fogColor(Color::White);

    if (uniformFogColor != -1)
        shader->SetUniformColor3ByIndex(uniformFogColor, fogColor);
    if (uniformFogDensity != -1)
        shader->SetUniformValueByIndex(uniformFogDensity, fogDensity);
}

void LandscapeRenderer::UnbindMaterial()
{
    RenderManager::Instance()->SetShader(nullptr);
    RenderManager::Instance()->FlushState();
}

DAVA::uint32 * LandscapeRenderer::Indicies()
{
    return &indices.front();
}

