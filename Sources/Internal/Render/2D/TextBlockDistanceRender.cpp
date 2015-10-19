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


#include "Render/2D/TextBlockDistanceRender.h"
#include "Render/RenderManager.h"
#include "Core/Core.h"
#include "Render/ShaderCache.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

namespace DAVA 
{
    
    
static uint16* InitIndexBuffer()
{
    static uint16 buffer[DF_FONT_INDEX_BUFFER_SIZE];
    
    uint16 a = 0;
    for (int32 i = 0; i < DF_FONT_INDEX_BUFFER_SIZE;)
    {
        buffer[i] = buffer[i+3] = a;
        buffer[i+1] = a+1;
        buffer[i+2] = buffer[i+4] = a+2;
        buffer[i+5] = a+3;
        i+=6;
        a+=4;
    }
    return buffer;
}
    
uint16* TextBlockDistanceRender::indexBuffer = InitIndexBuffer();
	
FastName TextBlockDistanceRender::textureUniform("texture0");
FastName TextBlockDistanceRender::smoothingUniform("smoothing");
FastName TextBlockDistanceRender::colorUniform("color");

TextBlockDistanceRender::TextBlockDistanceRender(TextBlock* textBlock) :
	TextBlockRender(textBlock)
{
	charDrawed = 0;
	renderObject = new RenderDataObject();
	
	dfFont = (DFFont*)textBlock->font;
	
    shader = ShaderCache::Instance()->Get(FastName("~res:/Shaders/Default/df_font"), FastNameSet());
    shader->Retain();
}
	
TextBlockDistanceRender::~TextBlockDistanceRender()
{
    shader->Release();
    shader = NULL;

	SafeRelease(renderObject);
}
	
void TextBlockDistanceRender::Prepare(Texture *texture /*= NULL*/)
{
	charDrawed = 0;
	renderRect = Rect(0, 0, 0, 0);
    
    size_t charCount = 0;
    if (!textBlock->isMultilineEnabled || textBlock->treatMultilineAsSingleLine)
    {
        charCount = textBlock->visualText.length();
    }
    else
    {
        size_t stringsCnt = textBlock->multilineStrings.size();
        for (size_t line = 0; line < stringsCnt; ++line)
        {
            charCount += textBlock->multilineStrings[line].length();
        }
    }
    size_t vertexCount = charCount * 4;
    
    if (vertexBuffer.size() != vertexCount)
    {
        vertexBuffer.resize(vertexCount);
    }
    
	DrawText();
    
	if (charDrawed == 0)
		return;
	
	renderObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, sizeof(DFFont::DFFontVertex), &vertexBuffer[0].position);
	renderObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, sizeof(DFFont::DFFontVertex), &vertexBuffer[0].texCoord);
	renderObject->BuildVertexBuffer(charDrawed * 4);
}

void TextBlockDistanceRender::Draw(const Color& textColor, const Vector2* offset)
{
	if (charDrawed == 0)
		return;
	
	int32 xOffset = 0;// (int32)(textBlock->position.x);
	int32 yOffset = 0;// (int32)(textBlock->position.y);

	if (offset)
	{
		xOffset += (int32)offset->x;
		yOffset += (int32)offset->y;
	}

    int32 align = textBlock->GetVisualAlign();
    if (align & ALIGN_RIGHT)
    {
        xOffset += (int32)(textBlock->rectSize.dx - renderRect.dx);
	}
	else if ((align & ALIGN_HCENTER) || (align & ALIGN_HJUSTIFY))
	{
		xOffset += (int32)((textBlock->rectSize.dx - renderRect.dx) * 0.5f);
	}
	
	if (align & ALIGN_BOTTOM)
	{
		yOffset += (int32)(textBlock->rectSize.dy - renderRect.dy);
	}
	else if ((align & ALIGN_VCENTER) || (align & ALIGN_HJUSTIFY))
	{
		yOffset += (int32)((textBlock->rectSize.dy - renderRect.dy) * 0.5f);
	}

    RenderSystem2D::Instance()->Flush();
    
    //TODO: temporary crutch until 2D render became fully stateless
    const Matrix4 * oldMatrix = (Matrix4 *)RenderManager::GetDynamicParam(PARAM_WORLD);
    
	//NOTE: correct affine transformations

    Matrix4 offsetMatrix;
	offsetMatrix.glTranslate((float32)xOffset - textBlock->pivot.x, (float32)yOffset - textBlock->pivot.y, 0.f);

	Matrix4 rotateMatrix;
	rotateMatrix.glRotate(RadToDeg(textBlock->angle), 0.f, 0.f, 1.f);

	Matrix4 scaleMatrix;
	//recalculate x scale - for non-uniform scale
	const float difX = 1.0f - (textBlock->scale.dy - textBlock->scale.dx);
	scaleMatrix.glScale(difX, 1.f, 1.0f);

	Matrix4 worldMatrix;
	worldMatrix.glTranslate(textBlock->position.x, textBlock->position.y, 0.f);

	offsetMatrix = (scaleMatrix*offsetMatrix*rotateMatrix)*worldMatrix;
	//offsetMatrix = (scaleMatrix * rotateMatrix * offsetMatrix)*worldMatrix;

	RenderManager::SetDynamicParam(PARAM_WORLD, &offsetMatrix, UPDATE_SEMANTIC_ALWAYS);

	RenderManager::Instance()->SetRenderState(RenderState::RENDERSTATE_2D_BLEND);
    RenderManager::Instance()->SetTextureState(dfFont->GetTextureHandler());
	RenderManager::Instance()->SetShader(shader);
    RenderManager::Instance()->SetRenderData(renderObject);
	RenderManager::Instance()->FlushState();
	RenderManager::Instance()->AttachRenderData();
    
    int idx;
    idx = shader->FindUniformIndexByName(textureUniform);
    shader->SetUniformValueByIndex(idx, 0);
    idx = shader->FindUniformIndexByName(smoothingUniform);
    shader->SetUniformValueByIndex(idx, dfFont->GetSpread());
    idx = shader->FindUniformIndexByName(colorUniform);
    shader->SetUniformColor4ByIndex(idx, textColor);
    
	RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, charDrawed * 6, EIF_16, this->indexBuffer);
    
    RenderManager::SetDynamicParam(PARAM_WORLD, oldMatrix, (pointer_size)oldMatrix);
}
	
Font::StringMetrics TextBlockDistanceRender::DrawTextSL(const WideString& drawText, int32 x, int32 y, int32 w)
{
	return InternalDrawText(drawText, 0, 0, 0, 0);
}
		
Font::StringMetrics TextBlockDistanceRender::DrawTextML(const WideString& drawText,
										   int32 x, int32 y, int32 w,
										   int32 xOffset, uint32 yOffset,
										   int32 lineSize)
{
    return InternalDrawText(drawText, xOffset, yOffset, (int32)ceilf(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX((float32)w)), lineSize);
}
	
Font::StringMetrics TextBlockDistanceRender::InternalDrawText(const WideString& drawText, int32 x, int32 y, int32 w, int32 lineSize)
{
	if (drawText.empty())
		return Font::StringMetrics();
	
	int32 lastDrawed = 0;
	
	Font::StringMetrics metrics = dfFont->DrawStringToBuffer(drawText, x, y, &vertexBuffer[0] + (charDrawed * 4), lastDrawed, NULL, w, lineSize);
	if (metrics.drawRect.dx <= 0 && metrics.drawRect.dy <= 0)
		return metrics;
	
	renderRect = renderRect.Combine(Rect((float32)metrics.drawRect.x, (float)metrics.drawRect.y, (float32)metrics.drawRect.dx, (float32)metrics.drawRect.dy));
	this->charDrawed += lastDrawed;
	return metrics;
}
	
};