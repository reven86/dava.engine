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


#ifndef __COLLADALOADER_COLLADAMATERIAL_H__
#define __COLLADALOADER_COLLADAMATERIAL_H__

#include "ColladaIncludes.h"
#include "ColladaTexture.h"


namespace DAVA
{

class ColladaScene;

class ColladaMaterial
{
public:
	ColladaMaterial(ColladaScene * scene, FCDMaterial * _material);
	~ColladaMaterial();
		
	void SetDefaultMaterial();

	static ColladaMaterial * defaultMaterial;
	static ColladaMaterial * GetDefaultMaterial();

	FCDMaterial * material;

	Vector4 ambient;
	Vector4 diffuse;
	Vector4 specular;
	Vector4 emission;
	
	float	shininess;
	Vector4 reflective;
	float	reflectivity;
	
	Vector4 transparent;
	float	transparency; // use m_is_transparency_one_opaque to figure out
	float	indexOfRefraction;

	ColladaTexture* diffuseTexture;
	fm::string		diffuseTextureName;
	bool			hasDiffuseTexture;

	// pointer to texture reflective
	ColladaTexture* reflectiveTexture;
	fm::string		reflectiveTextureName;
	bool			hasReflectiveTexture;

    ColladaTexture* lightmapTexture;
	fm::string		lightmapTextureName;
	bool			hasLightmapTexture;

	// pointer to texture transparent
	ColladaTexture* transparentTexture;
	bool			hasTransparentTexture;

	
	bool			IsTransparent();

};

};

#endif // __COLLADALOADER_COLLADAMATERIAL_H__


