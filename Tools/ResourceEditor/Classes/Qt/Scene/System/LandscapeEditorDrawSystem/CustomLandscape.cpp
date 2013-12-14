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



#include "CustomLandscape.h"
#include "Deprecated/LandscapeRenderer.h"

CustomLandscape::CustomLandscape()
:	landscapeRenderer(NULL)
{
}

CustomLandscape::~CustomLandscape()
{
	SafeRelease(landscapeRenderer);
}

void CustomLandscape::SetRenderer(LandscapeRenderer *renderer)
{
	SafeRelease(landscapeRenderer);
	landscapeRenderer = SafeRetain(renderer);
}

LandscapeRenderer* CustomLandscape::GetRenderer()
{
	return landscapeRenderer;
}

void CustomLandscape::Draw(DAVA::Camera *camera)
{
	if(!landscapeRenderer)
	{
		return;
	}
	
	RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, camera->GetMatrix());
	
	//VI: texture state
	//landscapeRenderer->BindMaterial(GetTexture(Landscape::TEXTURE_TILE_FULL));
	landscapeRenderer->DrawLandscape();
	
	if (cursor)
	{
		RenderManager::Instance()->SetRenderState(cursor->GetRenderState());
		RenderManager::Instance()->FlushState();
		
		//RenderManager::Instance()->AppendState(RenderState::STATE_BLEND);
		//eBlendMode src = RenderManager::Instance()->GetSrcBlend();
		//eBlendMode dst = RenderManager::Instance()->GetDestBlend();
		//RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		//RenderManager::Instance()->SetDepthFunc(CMP_LEQUAL);
		cursor->Prepare();
		
		RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, (heightmap->Size() - 1) * (heightmap->Size() - 1) * 6, EIF_32, landscapeRenderer->Indicies());
		
		//RenderManager::Instance()->SetDepthFunc(CMP_LESS);
		//RenderManager::Instance()->RemoveState(RenderState::STATE_BLEND);
		//RenderManager::Instance()->SetBlendMode(src, dst);
	}
	
	landscapeRenderer->UnbindMaterial();
}