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



#ifndef __DAVAENGINE_LANDSCAPE_CURSOR_H__
#define __DAVAENGINE_LANDSCAPE_CURSOR_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Render/Shader.h"
#include "Render/Texture.h"


namespace DAVA
{

class LandscapeCursor
{
public:
	LandscapeCursor();
	~LandscapeCursor();

	void Prepare();
	void SetPosition(const Vector2 & posistion);
	void SetScale(float32 scale);
	void SetCursorTexture(Texture * texture);
	void SetEditedTexture(Texture * texture);
	void SetBigTextureSize(float32 bigSize);

	UniqueHandle GetRenderState() {return renderState;}
    
    Texture * GetCursorTexture();
    float32 GetBigTextureSize();
    Vector2 GetCursorPosition();
    float32 GetCursorScale();
    
private:

#if RHI_COMPLETE
	Shader * shader;
#endif RHI_COMPLETE
	Texture * cursorTexture;
	UniqueHandle textureHandle;
	Vector2 position;
	float32 bigSize;
	float32 scale;

	int32 uniformTexture;
	int32 uniformPosition;
	int32 uniformScale;
	
	UniqueHandle renderState;
};

};

#endif //__DAVAENGINE_LANDSCAPE_CURSOR_H__