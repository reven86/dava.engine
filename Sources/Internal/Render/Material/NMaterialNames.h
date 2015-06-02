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


#ifndef __DAVAENGINE_NMATERIAL_NAMES_H__
#define __DAVAENGINE_NMATERIAL_NAMES_H__

#include "Base/FastName.h"

namespace DAVA
{
class NMaterialName
{
public:

	static const FastName DECAL_ALPHABLEND;
	static const FastName PIXELLIT_SPECULARMAP_ALPHATEST;
	static const FastName TEXTURED_ALPHABLEND;
	static const FastName DECAL_ALPHATEST;
	static const FastName PIXELLIT_SPECULARMAP_OPAQUE;
	static const FastName TEXTURED_ALPHATEST;
	static const FastName DECAL_OPAQUE;
	static const FastName SHADOWRECT_ALPHA;
	static const FastName TEXTURED_OPAQUE;
	static const FastName DETAIL_ALPHABLEND;
	static const FastName SHADOWRECT_MULTIPLY;
	static const FastName TILE_MASK;
	static const FastName DETAIL_ALPHATEST;
	static const FastName SHADOW_VOLUME;
	static const FastName VERTEXCOLOR_ALPHABLEND;
	static const FastName DETAIL_OPAQUE;
	static const FastName SILHOUETTE;
	static const FastName VERTEXCOLOR_FRAMEBLEND_ALPHABLEND;
	static const FastName SKYBOX;
    static const FastName SKYOBJECT;
	static const FastName VERTEXCOLOR_FRAMEBLEND_OPAQUE;
	static const FastName PIXELLIT_ALPHATEST;
	static const FastName SPEEDTREE_LEAF;
    static const FastName SPHERICLIT_SPEEDTREE_LEAF;
	static const FastName VERTEXCOLOR_OPAQUE;
	static const FastName PIXELLIT_OPAQUE;
	static const FastName TEXTURE_LIGHTMAP_ALPHABLEND;
	static const FastName VERTEXLIT_ALPHATEST;
	static const FastName PIXELLIT_SPECULAR_ALPHATEST;
	static const FastName TEXTURE_LIGHTMAP_ALPHATEST;
	static const FastName VERTEXLIT_OPAQUE;
	static const FastName PIXELLIT_SPECULAR_OPAQUE;
	static const FastName TEXTURE_LIGHTMAP_OPAQUE;
    static const FastName GRASS;
	
	static const FastName PARTICLES;
	static const FastName PARTICLES_FRAMEBLEND;

    static const FastName DEBUG_DRAW_OPAQUE;
    static const FastName DEBUG_DRAW_ALPHABLEND;
};
};

#endif /* defined(__DAVAENGINE_NMATERIAL_NAMES_H__) */
