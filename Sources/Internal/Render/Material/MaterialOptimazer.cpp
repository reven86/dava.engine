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


#include "MaterialOptimazer.h"

namespace DAVA
{
	
uint32 MaterialOptimizer::GetOptimizedVertexFormat(Material::eType type)
{
	uint32 optimazedFormat = EVF_FORCE_DWORD;
	switch (type)
	{
		case Material::MATERIAL_UNLIT_TEXTURE:
		{
			optimazedFormat = EVF_VERTEX | EVF_TEXCOORD0;
		}break;
		case Material::MATERIAL_UNLIT_TEXTURE_DETAIL:
		case Material::MATERIAL_UNLIT_TEXTURE_DECAL:
		case Material::MATERIAL_UNLIT_TEXTURE_LIGHTMAP:
		{
			optimazedFormat = EVF_VERTEX | EVF_TEXCOORD0 | EVF_TEXCOORD1;
		}break;
		case Material::MATERIAL_VERTEX_LIT_TEXTURE:
		{
			optimazedFormat = EVF_VERTEX | EVF_NORMAL | EVF_TEXCOORD0;
		}break;
		case Material::MATERIAL_VERTEX_LIT_DETAIL:
		case Material::MATERIAL_VERTEX_LIT_DECAL:
		case Material::MATERIAL_VERTEX_LIT_LIGHTMAP:
		{
			optimazedFormat = EVF_VERTEX | EVF_NORMAL | EVF_TEXCOORD0 | EVF_TEXCOORD1;
		}break;
		case Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE:
		case Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR:
		case Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP:
		{
			optimazedFormat = EVF_VERTEX | EVF_NORMAL | EVF_TANGENT | EVF_TEXCOORD0;
		}break;
		case Material::MATERIAL_VERTEX_COLOR_ALPHABLENDED:
		{
			optimazedFormat = EVF_VERTEX | EVF_TEXCOORD0 | EVF_COLOR;
		}break;
		case Material::MATERIAL_FLAT_COLOR:
		{
			optimazedFormat = EVF_VERTEX | EVF_TEXCOORD0;
		}break;
			
		case Material::MATERIAL_SKYBOX:
		{
			optimazedFormat = EVF_VERTEX | EVF_TEXCOORD0;
		}break;

        case Material::MATERIAL_SPEED_TREE_LEAF:
        {
            optimazedFormat = EVF_VERTEX | EVF_COLOR | EVF_TEXCOORD0 | EVF_PIVOT | EVF_FLEXIBILITY | EVF_ANGLE_SIN_COS;
        }break;

		default:
			DVASSERT(false);
			Logger::Error("Unknown material format");
			break;
	}
	return optimazedFormat;
}

}