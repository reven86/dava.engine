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


#ifndef __COLLADALOADER_COLLADALIGHT_H__
#define __COLLADALOADER_COLLADALIGHT_H__

#include "ColladaIncludes.h"

namespace DAVA
{

struct ColladaLightState
{
	ColladaLightState()
	{
		lightIndex = 0;
		globalAmbientalLight[0] = 0.0f;
		globalAmbientalLight[1] = 0.0f;
		globalAmbientalLight[2] = 0.0f;
		globalAmbientalLight[3] = 1.0f;
	}

	int lightIndex;
	GLfloat globalAmbientalLight[4];
};


class ColladaLight
{
public:
	ColladaLight(FCDLight * _light);
	~ColladaLight();
	
	void ApplyLight(ColladaLightState & state);

	enum eType
	{
		AMBIENT = 0,
		SPOT,
		DIRECTIONAL,
		POINT,
	};

	FCDLight * light;

	eType		type;
	Vector4		color;				// for all lights
	Vector4		direction;			// for directional
	Vector4		attenuation;		// for point 
};

};

#endif // __COLLADALOADER_COLLADALIGHT_H__
