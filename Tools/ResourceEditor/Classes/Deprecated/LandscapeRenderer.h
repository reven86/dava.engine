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


#ifndef __LANDSCAPE_RENDERER_H__
#define __LANDSCAPE_RENDERER_H__

#include "DAVAEngine.h"

class LandscapeRenderer: public DAVA::BaseObject
{

public:
	LandscapeRenderer(DAVA::Heightmap *heightmap, const DAVA::AABBox3 &box);
	virtual ~LandscapeRenderer();

    void RebuildVertexes(const DAVA::Rect &rebuildForRect);
    
    void BindMaterial(DAVA::UniqueHandle textureStateHandle);
    void UnbindMaterial();
    
    void DrawLandscape();
    
    DAVA::uint32 * Indicies();
    
protected:

    void InitShader();
    void RebuildIndexes();
    void SetHeightmap(DAVA::Heightmap *heightmap, const DAVA::AABBox3 &box);
    void SetBoundingBox(const DAVA::AABBox3 &box);
    DAVA::Vector3 GetPoint(DAVA::int16 x, DAVA::int16 y, DAVA::uint16 height);
    
    DAVA::int32 uniformFogDensity;
    DAVA::int32 uniformFogColor;
    DAVA::Shader * shader;
    
    DAVA::Vector<DAVA::Landscape::LandscapeVertex> vertices;
    DAVA::Vector<DAVA::uint32> indices;    
    
    DAVA::Heightmap *heightmap;
    DAVA::AABBox3 boundingBox;
    
    DAVA::Vector3 pointCoefficients;
};


#endif // __LANDSCAPE_RENDERER_H__


