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


#include "LandscapeCursor.h"
#include "Render/RenderManager.h"
#include "FileSystem/FilePath.h"
#include "Render/ShaderCache.h"

namespace DAVA
{

LandscapeCursor::LandscapeCursor()
{
	textureHandle = InvalidUniqueHandle;
    cursorTexture = NULL;

	shader = SafeRetain(ShaderCache::Instance()->Get(FastName("~res:/Materials/Shaders/Landscape/cursor"), FastNameSet()));

	uniformTexture = shader->FindUniformIndexByName(FastName("texture0"));
	uniformPosition = shader->FindUniformIndexByName(FastName("position"));
	uniformScale = shader->FindUniformIndexByName(FastName("scale"));
	
	RenderManager* rm = RenderManager::Instance();
	RenderStateData renderStateData;
	rm->GetRenderStateData(RenderState::RENDERSTATE_3D_BLEND, renderStateData);
    
	renderStateData.depthFunc = CMP_LEQUAL;
	renderState = rm->CreateRenderState(renderStateData);
}

void LandscapeCursor::Prepare()
{
	if(InvalidUniqueHandle == textureHandle)
	{
		return;
	}

	RenderManager::Instance()->SetTextureState(textureHandle);
	RenderManager::Instance()->SetShader(shader);
	RenderManager::Instance()->FlushState();
	RenderManager::Instance()->AttachRenderData();
	shader->SetUniformValueByIndex(uniformTexture, 0);
	shader->SetUniformValueByIndex(uniformScale, bigSize/scale);

	Vector2 actualPosition = position/bigSize;
	shader->SetUniformValueByIndex(uniformPosition, actualPosition);
}

LandscapeCursor::~LandscapeCursor()
{
	SafeRelease(shader);
    SafeRelease(cursorTexture);
    
    if(InvalidUniqueHandle != textureHandle)
    {
        RenderManager::Instance()->ReleaseTextureState(textureHandle);
    }
    
    if(InvalidUniqueHandle != renderState)
    {
        RenderManager::Instance()->ReleaseRenderState(renderState);
    }
}

void LandscapeCursor::SetPosition(const Vector2 & _posistion)
{
	position = _posistion;
}

void LandscapeCursor::SetScale(float32 _scale)
{
	scale = _scale;
}

void LandscapeCursor::SetCursorTexture(Texture* _texture)
{
    if(_texture != cursorTexture)
    {
        SafeRelease(cursorTexture);
        cursorTexture = SafeRetain(_texture);
        
        if(InvalidUniqueHandle != textureHandle)
        {
            RenderManager::Instance()->ReleaseTextureState(textureHandle);
        }
        
        TextureStateData stateData;
        stateData.SetTexture(0, cursorTexture);
        
        textureHandle = RenderManager::Instance()->CreateTextureState(stateData);
    }
}

void LandscapeCursor::SetBigTextureSize(float32 _bigSize)
{
	bigSize = _bigSize;
}
    
    
Texture* LandscapeCursor::GetCursorTexture()
{
    return cursorTexture;
}

float32 LandscapeCursor::GetBigTextureSize()
{
    return bigSize;
}

Vector2 LandscapeCursor::GetCursorPosition()
{
    return position;
}

float32 LandscapeCursor::GetCursorScale()
{
    return scale;
}

    

};
