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
    static const FastName TEXTURED_VERTEXCOLOR_ALPHATEST;
	static const FastName DECAL_OPAQUE;
    static const FastName TEXTURED_OPAQUE;
    static const FastName TEXTURED_VERTEXCOLOR_OPAQUE;
	static const FastName DETAIL_ALPHABLEND;
    static const FastName SHADOWRECT;
    static const FastName TILE_MASK;
    static const FastName DETAIL_ALPHATEST;
	static const FastName SHADOW_VOLUME;
    static const FastName VERTEXCOLOR_ALPHABLEND;
    static const FastName VERTEXCOLOR_ALPHABLEND_NODEPTHTEST;
    static const FastName VERTEXCOLOR_ALPHABLEND_TEXTURED;
    static const FastName DETAIL_OPAQUE;
    static const FastName SILHOUETTE;
	static const FastName VERTEXCOLOR_FRAMEBLEND_ALPHABLEND;
    static const FastName SKYOBJECT;
	static const FastName VERTEXCOLOR_FRAMEBLEND_OPAQUE;
	static const FastName PIXELLIT_ALPHATEST;
	static const FastName SPEEDTREE_LEAF;
    static const FastName SPEEDTREE_LEAF_ALPHABLEND;
    static const FastName SPHERICLIT_SPEEDTREE_LEAF;
    static const FastName SPHERICLIT_SPEEDTREE_LEAF_ALPHABLEND;
    static const FastName SPHERICLIT_TEXTURED_OPAQUE;
    static const FastName SPHERICLIT_TEXTURED_ALPHATEST;
    static const FastName SPHERICLIT_TEXTURED_ALPHABLEND;
    static const FastName SPHERICLIT_TEXTURED_VERTEXCOLOR_OPAQUE;
    static const FastName SPHERICLIT_TEXTURED_VERTEXCOLOR_ALPHATEST;
    static const FastName SPHERICLIT_TEXTURED_VERTEXCOLOR_ALPHABLEND;
    static const FastName VERTEXCOLOR_OPAQUE;
    static const FastName VERTEXCOLOR_OPAQUE_NODEPTHTEST;
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
    static const FastName DEBUG_DRAW_WIREFRAME;
};

class NMaterialTextureName
{
public:
    static const FastName TEXTURE_ALBEDO;
    static const FastName TEXTURE_NORMAL;
    static const FastName TEXTURE_DETAIL;
    static const FastName TEXTURE_LIGHTMAP;
    static const FastName TEXTURE_DECAL;
    static const FastName TEXTURE_CUBEMAP;
    static const FastName TEXTURE_HEIGHTMAP;
    static const FastName TEXTURE_DECALMASK;
    static const FastName TEXTURE_DECALTEXTURE;

    static const FastName TEXTURE_DYNAMIC_REFLECTION;
    static const FastName TEXTURE_DYNAMIC_REFRACTION;
};

class NMaterialParamName
{
public:
    static const FastName PARAM_LIGHT_POSITION0;
    static const FastName PARAM_PROP_AMBIENT_COLOR;
    static const FastName PARAM_PROP_DIFFUSE_COLOR;
    static const FastName PARAM_PROP_SPECULAR_COLOR;
    static const FastName PARAM_LIGHT_AMBIENT_COLOR;
    static const FastName PARAM_LIGHT_DIFFUSE_COLOR;
    static const FastName PARAM_LIGHT_SPECULAR_COLOR;
    static const FastName PARAM_LIGHT_INTENSITY0;
    static const FastName PARAM_MATERIAL_SPECULAR_SHININESS;
    static const FastName PARAM_FOG_LIMIT;
    static const FastName PARAM_FOG_COLOR;
    static const FastName PARAM_FOG_DENSITY;
    static const FastName PARAM_FOG_START;
    static const FastName PARAM_FOG_END;
    static const FastName PARAM_FOG_ATMOSPHERE_COLOR_SUN;
    static const FastName PARAM_FOG_ATMOSPHERE_COLOR_SKY;
    static const FastName PARAM_FOG_ATMOSPHERE_SCATTERING;
    static const FastName PARAM_FOG_ATMOSPHERE_DISTANCE;
    static const FastName PARAM_FOG_HALFSPACE_HEIGHT;
    static const FastName PARAM_FOG_HALFSPACE_DENSITY;
    static const FastName PARAM_FOG_HALFSPACE_FALLOFF;
    static const FastName PARAM_FOG_HALFSPACE_LIMIT;
    static const FastName PARAM_FLAT_COLOR;
    static const FastName PARAM_TEXTURE0_SHIFT;
    static const FastName PARAM_UV_OFFSET;
    static const FastName PARAM_UV_SCALE;
    static const FastName PARAM_LIGHTMAP_SIZE;
    static const FastName PARAM_DECAL_TILE_SCALE;
    static const FastName PARAM_DECAL_TILE_COLOR;
    static const FastName PARAM_DETAIL_TILE_SCALE;
    static const FastName PARAM_RCP_SCREEN_SIZE;
    static const FastName PARAM_SCREEN_OFFSET;
    static const FastName PARAM_ALPHATEST_THRESHOLD;
    static const FastName PARAM_LANDSCAPE_TEXTURE_TILING;
    static const FastName DEPRECATED_SHADOW_COLOR_PARAM;
    static const FastName DEPRECATED_LANDSCAPE_TEXTURE_0_TILING;
};

class NMaterialFlagName
{
public:
    static const FastName FLAG_BLENDING;

    static const FastName FLAG_VERTEXFOG;
    static const FastName FLAG_FOG_LINEAR;
    static const FastName FLAG_FOG_HALFSPACE;
    static const FastName FLAG_FOG_HALFSPACE_LINEAR;
    static const FastName FLAG_FOG_ATMOSPHERE;
    static const FastName FLAG_TEXTURESHIFT;
    static const FastName FLAG_TEXTURE0_ANIMATION_SHIFT;
    static const FastName FLAG_WAVE_ANIMATION;
    static const FastName FLAG_FAST_NORMALIZATION;
    static const FastName FLAG_TILED_DECAL_MASK;
    static const FastName FLAG_FLATCOLOR;
    static const FastName FLAG_DISTANCEATTENUATION;
    static const FastName FLAG_SPECULAR;
    static const FastName FLAG_SEPARATE_NORMALMAPS;

    static const FastName FLAG_SPEED_TREE_LEAF;
    static const FastName FLAG_SPHERICAL_LIT;

    static const FastName FLAG_TANGENT_SPACE_WATER_REFLECTIONS;

    static const FastName FLAG_DEBUG_UNITY_Z_NORMAL;
    static const FastName FLAG_DEBUG_Z_NORMAL_SCALE;
    static const FastName FLAG_DEBUG_NORMAL_ROTATION;

    static const FastName FLAG_SKINNING;

    static const FastName FLAG_LIGHTMAPONLY;
    static const FastName FLAG_TEXTUREONLY; //VI: this flag is for backward compatibility with old materials. See FLAG_ALBEDOONLY
    static const FastName FLAG_SETUPLIGHTMAP;
    static const FastName FLAG_VIEWALBEDO;
    static const FastName FLAG_VIEWAMBIENT;
    static const FastName FLAG_VIEWDIFFUSE;
    static const FastName FLAG_VIEWSPECULAR;

    static const FastName FLAG_FRAME_BLEND;
    static const FastName FLAG_FORCE_2D_MODE;

    static const FastName FLAG_ALPHATESTVALUE;

    //Illumination params
    static const FastName FLAG_ILLUMINATION_USED;
    static const FastName FLAG_ILLUMINATION_SHADOW_CASTER;
    static const FastName FLAG_ILLUMINATION_SHADOW_RECEIVER;

    static bool IsRuntimeFlag(const FastName& flag);
};

class NMaterialSerializationKey
{
public:
    static const DAVA::String MaterialKey;
    static const DAVA::String ParentMaterialKey;
    static const DAVA::String FXName;
    static const DAVA::String QualityGroup;
    static const DAVA::String MaterialName;
};

class NMaterialQualityName
{
public:
    static const FastName QUALITY_FLAG_NAME;
    static const FastName QUALITY_GROUP_FLAG_NAME;
    static const FastName DEFAULT_QUALITY_NAME;
};
};

#endif /* defined(__DAVAENGINE_NMATERIAL_NAMES_H__) */
