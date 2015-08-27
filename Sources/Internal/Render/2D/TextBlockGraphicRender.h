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


#ifndef __DAVAENGINE_TEXTBLOCK_GRAPHIC_RENDER_H__
#define __DAVAENGINE_TEXTBLOCK_GRAPHIC_RENDER_H__

#include "Render/2D/TextBlockRender.h"
#include "Render/2D/GraphicFont.h"
#include "Base/FastName.h"
#include "Render/UniqueStateSet.h"

namespace DAVA
{

class TextBlockGraphicRender: public TextBlockRender
{
public:
	enum
	{
		TextVerticesDefaultStride = 5
	};

public:
	TextBlockGraphicRender(TextBlock*);
	~TextBlockGraphicRender();
	
	virtual void Prepare(Texture *texture = nullptr);
	virtual void Draw(const Color& textColor, const Vector2* offset);

	static const uint16* GetSharedIndexBuffer();
	static const uint32 GetSharedIndexBufferCapacity();
	
protected:
	virtual Font::StringMetrics DrawTextSL(const WideString& drawText, int32 x, int32 y, int32 w);
	virtual Font::StringMetrics DrawTextML(const WideString& drawText,
							  int32 x, int32 y, int32 w,
							  int32 xOffset, uint32 yOffset,
							  int32 lineSize);
	
private:
	Font::StringMetrics InternalDrawText(const WideString& drawText, int32 x, int32 y, int32 w, int32 lineSize);
	
private:
	GraphicFont* graphicFont;

	static uint16* indexBuffer;
    Vector<GraphicFont::GraphicFontVertex> vertexBuffer;
	
	uint32 charDrawed;
	Rect renderRect;

    NMaterial* dfMaterial;
    float32 cachedSpread;
};

}; //end of namespace

#endif //__DAVAENGINE_TEXTBLOCK_DISTANCE_RENDER_H__