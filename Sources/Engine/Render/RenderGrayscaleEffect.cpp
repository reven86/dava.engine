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


#include "Render/RenderGrayscaleEffect.h"
#include "Render/RenderManager.h"
#include "Utils/Utils.h"


namespace DAVA 
{
#if defined(__DAVAENGINE_OPENGL__)
void RenderGrayscaleEffect::StartEffect()
{
	//VI: seems this code never used
	DVASSERT(false);
/*
	RENDER_VERIFY(glColor4f(0.5f + 0.3f * RenderManager::Instance()->GetColorR(), 0.5f + 59.0f  * RenderManager::Instance()->GetColorG(), 0.5f + 0.11f  * RenderManager::Instance()->GetColorB(), RenderManager::Instance()->GetColorA()));
	
	float constColor[] = {0.67f, 0.67f, 0.67f, 0.25f};
	RENDER_VERIFY(glActiveTexture(GL_TEXTURE0));
	RenderManager::Instance()->HWglBindTexture(RenderManager::Instance()->GetTexture()->id, RenderManager::Instance()->GetTexture()->textureType);
	
	RENDER_VERIFY(glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constColor));
	RENDER_VERIFY(glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE));
	RENDER_VERIFY(glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE));
	RENDER_VERIFY(glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE));
	RENDER_VERIFY(glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_CONSTANT));
	RENDER_VERIFY(glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, GL_CONSTANT));
	RENDER_VERIFY(glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR));
	RENDER_VERIFY(glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA));
	
	
	RENDER_VERIFY(glActiveTexture(GL_TEXTURE1));
	RENDER_VERIFY(glEnable(GL_TEXTURE_2D));
	RenderManager::Instance()->HWglBindTexture(RenderManager::Instance()->GetTexture()->id, RenderManager::Instance()->GetTexture()->textureType);
	RENDER_VERIFY(glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE));
	RENDER_VERIFY(glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_DOT3_RGB));
	RENDER_VERIFY(glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS));
	RENDER_VERIFY(glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR));
	
	RENDER_VERIFY(glClientActiveTexture(GL_TEXTURE1));
	RENDER_VERIFY(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
	RENDER_VERIFY(glClientActiveTexture(GL_TEXTURE0));
	*/
}

void RenderGrayscaleEffect::StopEffect()
{
	//VI: seems this code never used
	DVASSERT(false);
/*
	RENDER_VERIFY(glActiveTexture(GL_TEXTURE1));
	RENDER_VERIFY(glDisable(GL_TEXTURE_2D));
	RENDER_VERIFY(glClientActiveTexture(GL_TEXTURE1));
	RENDER_VERIFY(glDisableClientState(GL_TEXTURE_COORD_ARRAY));

	RENDER_VERIFY(glClientActiveTexture(GL_TEXTURE0));
	RENDER_VERIFY(glActiveTexture(GL_TEXTURE0));
	RenderManager::Instance()->HWglBindTexture(RenderManager::Instance()->GetTexture()->id, RenderManager::Instance()->GetTexture()->textureType);
	RENDER_VERIFY(glEnable(GL_TEXTURE_2D));
	RENDER_VERIFY(glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE));
	float constColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
	RENDER_VERIFY(glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constColor));
	RENDER_VERIFY(glColor4f(1.0f, 1.0f, 1.0f, 1.0f));
*/
}

void RenderGrayscaleEffect::SetColor(float r, float g, float b, float a)
{
	//VI: seems this code never used
	DVASSERT(false);
/*
	RENDER_VERIFY(glColor4f(0.5f + 0.3f * r, 0.5f + 59.0f  * g, 0.5f + 0.11f  * b, a));
 */
}

void RenderGrayscaleEffect::SetTexture(Texture *texture)
{
	//VI: seems this code never used
	DVASSERT(false);
/*
	RENDER_VERIFY(glActiveTexture(GL_TEXTURE1));
	RenderManager::Instance()->HWglBindTexture(texture->id, texture->textureType);
	RENDER_VERIFY(glActiveTexture(GL_TEXTURE0));
	RenderManager::Instance()->HWglBindTexture(texture->id, texture->textureType);
*/
}
	
void RenderGrayscaleEffect::SetVertexPointer(int size, int type, int stride, const void *pointer)
{
	//VI: seems this code never used
	DVASSERT(false);
/*
    RENDER_VERIFY(glVertexPointer(size, type, stride, pointer));
 */
}
	

void RenderGrayscaleEffect::SetTexCoordPointer(int size, int type, int stride, const void *pointer)
{
	//VI: seems this code never used
	DVASSERT(false);
/*
	RENDER_VERIFY(glClientActiveTexture(GL_TEXTURE1));
	RENDER_VERIFY(glTexCoordPointer(size, type, stride, pointer));
	RENDER_VERIFY(glClientActiveTexture(GL_TEXTURE0));
	RENDER_VERIFY(glTexCoordPointer(size, type, stride, pointer));
 */
}
#elif defined(__DAVAENGINE_DIRECTX9__)


#endif

};

