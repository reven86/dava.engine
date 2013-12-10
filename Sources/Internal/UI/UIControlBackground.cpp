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



#include "UI/UIControlBackground.h"
#include "Debug/DVAssert.h"
#include "UI/UIControl.h"
#include "Core/Core.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"

namespace DAVA
{
//Shared render data
RenderDataObject* UIControlBackground::rdoObject = NULL;
RenderDataStream* UIControlBackground::vertexStream = NULL;
RenderDataStream* UIControlBackground::texCoordStream = NULL;

UIControlBackground::UIControlBackground()
:	spr(NULL)
,	frame(0)
,	align(ALIGN_HCENTER|ALIGN_VCENTER)
,	type(DRAW_ALIGNED)
,	color(1.0f, 1.0f, 1.0f, 1.0f)
,	drawColor(1.0f, 1.0f, 1.0f, 1.0f)
,	leftStretchCap(0)
,	topStretchCap(0)
,	spriteModification(0)
,	colorInheritType(COLOR_IGNORE_PARENT)
,	perPixelAccuracyType(PER_PIXEL_ACCURACY_DISABLED)
,	lastDrawPos(0, 0)
,	tiledDrawData(NULL)
{
	if(rdoObject == NULL)
	{
		rdoObject = new RenderDataObject();
		vertexStream = rdoObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 2, 0, 0);
		texCoordStream = rdoObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, 0);
	}
	//rdoObject->SetStream()
}
	
UIControlBackground *UIControlBackground::Clone()
{
	UIControlBackground *cb = new UIControlBackground();
	cb->CopyDataFrom(this);
	return cb;
}
	
void UIControlBackground::CopyDataFrom(UIControlBackground *srcBackground)
{
	SafeRelease(spr);
	spr = SafeRetain(srcBackground->spr);
	frame = srcBackground->frame;
	align = srcBackground->align;
	type = srcBackground->type;
	color = srcBackground->color;
	spriteModification = srcBackground->spriteModification;
	colorInheritType = srcBackground->colorInheritType;
	perPixelAccuracyType = srcBackground->perPixelAccuracyType;
	leftStretchCap = srcBackground->leftStretchCap;
	topStretchCap = srcBackground->topStretchCap;
}


UIControlBackground::~UIControlBackground()
{
	SafeRelease(rdoObject);
	SafeRelease(spr);
	ReleaseSpecDrawData();
}
	
Sprite*	UIControlBackground::GetSprite()
{
	return spr;	
}
int32	UIControlBackground::GetFrame() const
{
	return frame;
}
int32	UIControlBackground::GetAlign() const
{
	return align;
}

int32	UIControlBackground::GetModification() const
{
	return spriteModification;
}

UIControlBackground::eColorInheritType UIControlBackground::GetColorInheritType() const
{
	return (eColorInheritType)colorInheritType;
}


UIControlBackground::eDrawType	UIControlBackground::GetDrawType() const
{
	return (UIControlBackground::eDrawType)type;
}
	
	
void UIControlBackground::SetSprite(const FilePath &spriteName, int32 drawFrame)
{
	Sprite *tempSpr = Sprite::Create(spriteName);
	SetSprite(tempSpr, drawFrame);
	SafeRelease(tempSpr);
}

void UIControlBackground::SetSprite(Sprite* drawSprite, int32 drawFrame)
{
	if (drawSprite == this->spr)
	{
		// Sprite is not changed - update frame only.
		frame = drawFrame;
		return;
	}

	SafeRelease(spr);
	spr = SafeRetain(drawSprite);
	frame =  drawFrame;
}
void UIControlBackground::SetFrame(int32 drawFrame)
{
	DVASSERT(spr);
	frame = drawFrame;
}

void UIControlBackground::SetAlign(int32 drawAlign)
{
	align = drawAlign;
}
void UIControlBackground::SetDrawType(UIControlBackground::eDrawType drawType)
{
	type = drawType;
	ReleaseSpecDrawData();
}

void UIControlBackground::SetModification(int32 modification)
{
	spriteModification = modification;
}
	
void UIControlBackground::SetColorInheritType(UIControlBackground::eColorInheritType inheritType)
{
	DVASSERT(inheritType >= 0 && inheritType < COLOR_INHERIT_TYPES_COUNT);
	colorInheritType = inheritType;
}
    
void UIControlBackground::SetPerPixelAccuracyType(ePerPixelAccuracyType accuracyType)
{
    perPixelAccuracyType = accuracyType;
}
    
UIControlBackground::ePerPixelAccuracyType UIControlBackground::GetPerPixelAccuracyType() const
{
    return perPixelAccuracyType;
}
	
const Color &UIControlBackground::GetDrawColor() const
{
	return drawColor;
}

void UIControlBackground::SetDrawColor(const Color &c)
{
	drawColor = c;
}

void UIControlBackground::SetParentColor(const Color &parentColor)
{
	switch (colorInheritType) 
	{
		case COLOR_MULTIPLY_ON_PARENT:
		{
			drawColor.r = color.r * parentColor.r;
			drawColor.g = color.g * parentColor.g;
			drawColor.b = color.b * parentColor.b;
			drawColor.a = color.a * parentColor.a;
		}
			break;
		case COLOR_ADD_TO_PARENT:
		{
			drawColor.r = Min(color.r + parentColor.r, 1.0f);
			drawColor.g = Min(color.g + parentColor.g, 1.0f);
			drawColor.b = Min(color.b + parentColor.b, 1.0f);
			drawColor.a = Min(color.a + parentColor.a, 1.0f);
		}
			break;
		case COLOR_REPLACE_TO_PARENT:
		{
			drawColor = parentColor;
		}
			break;
		case COLOR_IGNORE_PARENT:
		{
			drawColor = color;
		}
			break;
		case COLOR_MULTIPLY_ALPHA_ONLY:
		{
			drawColor = color;
			drawColor.a = color.a * parentColor.a;
		}
			break;
		case COLOR_REPLACE_ALPHA_ONLY:
		{
			drawColor = color;
			drawColor.a = parentColor.a;
		}
			break;
	}	
}

void UIControlBackground::Draw(const UIGeometricData &geometricData)
{
	

	Rect drawRect = geometricData.GetUnrotatedRect();
//	if(drawRect.x > RenderManager::Instance()->GetScreenWidth() || drawRect.y > RenderManager::Instance()->GetScreenHeight() || drawRect.x + drawRect.dx < 0 || drawRect.y + drawRect.dy < 0)
//	{//TODO: change to screen size from control system and sizes from sprite
//		return;
//	}
	
	RenderManager::Instance()->SetColor(drawColor.r, drawColor.g, drawColor.b, drawColor.a);
	
	Sprite::DrawState drawState;
	if (spr)
	{
		drawState.frame = frame;
		if (spriteModification) 
		{
			drawState.flags = spriteModification;
		}
//		spr->Reset();
//		spr->SetFrame(frame);
//		spr->SetModification(spriteModification);
	}
	switch (type) 
	{
		case DRAW_ALIGNED:
		{
			if (!spr)break;
			if(align & ALIGN_LEFT)
			{
				drawState.position.x = drawRect.x;
			}
			else if(align & ALIGN_RIGHT)
			{
				drawState.position.x = drawRect.x + drawRect.dx - spr->GetWidth() * geometricData.scale.x;
			}
			else
			{
				drawState.position.x = drawRect.x + ((drawRect.dx - spr->GetWidth() * geometricData.scale.x) * 0.5f) ;
			}
			if(align & ALIGN_TOP)
			{
				drawState.position.y = drawRect.y;
			}
			else if(align & ALIGN_BOTTOM)
			{
				drawState.position.y = drawRect.y + drawRect.dy - spr->GetHeight() * geometricData.scale.y;
			}
			else
			{
				drawState.position.y = drawRect.y + ((drawRect.dy - spr->GetHeight() * geometricData.scale.y + spr->GetDefaultPivotPoint().y * geometricData.scale.y) * 0.5f) ;
			}
			if(geometricData.angle != 0)
			{
				float tmpX = drawState.position.x;
				drawState.position.x = (tmpX - geometricData.position.x) * geometricData.cosA  + (geometricData.position.y - drawState.position.y) * geometricData.sinA + geometricData.position.x;
				drawState.position.y = (tmpX - geometricData.position.x) * geometricData.sinA  + (drawState.position.y - geometricData.position.y) * geometricData.cosA + geometricData.position.y;
//				spr->SetAngle(geometricData.angle);
				drawState.angle = geometricData.angle;
			}
//			spr->SetPosition(x, y);
			drawState.scale = geometricData.scale;
			drawState.pivotPoint = spr->GetDefaultPivotPoint();
//			spr->SetScale(geometricData.scale);
            //if (drawState.scale.x == 1.0 && drawState.scale.y == 1.0)
            {
                switch(perPixelAccuracyType)
                {
                    case PER_PIXEL_ACCURACY_ENABLED:
                        if(lastDrawPos == drawState.position)
                        {
                            drawState.usePerPixelAccuracy = true;
                        }
                        break;
                    case PER_PIXEL_ACCURACY_FORCED:
                        drawState.usePerPixelAccuracy = true;
                        break;
                    default:
                        break;
                }
            }
			
			lastDrawPos = drawState.position;

			spr->Draw(&drawState);
		}
		break;

		case DRAW_SCALE_TO_RECT:
		{
			if (!spr)break;

			drawState.position = geometricData.position;
			drawState.flags = spriteModification;
			drawState.scale.x = drawRect.dx / spr->GetSize().dx;
			drawState.scale.y = drawRect.dy / spr->GetSize().dy;
			drawState.pivotPoint.x = geometricData.pivotPoint.x / (geometricData.size.x / spr->GetSize().dx);
			drawState.pivotPoint.y = geometricData.pivotPoint.y / (geometricData.size.y / spr->GetSize().dy);
			drawState.angle = geometricData.angle;
			{
				switch(perPixelAccuracyType)
				{
				case PER_PIXEL_ACCURACY_ENABLED:
					if(lastDrawPos == drawState.position)
					{
						drawState.usePerPixelAccuracy = true;
					}
					break;
				case PER_PIXEL_ACCURACY_FORCED:
					drawState.usePerPixelAccuracy = true;
					break;
				default:
					break;
				}
			}

			lastDrawPos = drawState.position;

//			spr->SetPosition(geometricData.position);
//			spr->SetScale(drawRect.dx / spr->GetSize().dx, drawRect.dy / spr->GetSize().dy);
//			spr->SetPivotPoint(geometricData.pivotPoint.x / (geometricData.size.x / spr->GetSize().dx), geometricData.pivotPoint.y / (geometricData.size.y / spr->GetSize().dy));
//			spr->SetAngle(geometricData.angle);
			
			spr->Draw(&drawState);
		}
		break;
		
		case DRAW_SCALE_PROPORTIONAL:
        case DRAW_SCALE_PROPORTIONAL_ONE:
		{
			if (!spr)break;
			float32 w, h;
			w = drawRect.dx / (spr->GetWidth() * geometricData.scale.x);
			h = drawRect.dy / (spr->GetHeight() * geometricData.scale.y);
			float ph = spr->GetDefaultPivotPoint().y;
            
            if(w < h)
            {
                if(type==DRAW_SCALE_PROPORTIONAL_ONE)
                {
                    w = spr->GetWidth() * h * geometricData.scale.y;
                    ph *= h;
                    h = drawRect.dy;
                }
                else
                {
                    h = spr->GetHeight() * w * geometricData.scale.x;
                    ph *= w;
                    w = drawRect.dx;
                }
            }
            else
            {
                if(type==DRAW_SCALE_PROPORTIONAL_ONE)
                {
                    h = spr->GetHeight() * w * geometricData.scale.x;
                    ph *= w;
                    w = drawRect.dx;
                }
                else
                {
                    w = spr->GetWidth() * h * geometricData.scale.y;
                    ph *= h;
                    h = drawRect.dy;
                }
            }
            
			if(align & ALIGN_LEFT)
			{
				drawState.position.x = drawRect.x;
			}
			else if(align & ALIGN_RIGHT)
			{
				drawState.position.x = (drawRect.x + drawRect.dx - w);
			}
			else
			{
				drawState.position.x = drawRect.x + (int32)((drawRect.dx - w) * 0.5) ;
			}
			if(align & ALIGN_TOP)
			{
				drawState.position.y = drawRect.y;
			}
			else if(align & ALIGN_BOTTOM)
			{
				drawState.position.y = (drawRect.y + drawRect.dy - h);
			}
			else
			{
				drawState.position.y = (drawRect.y) + (int32)((drawRect.dy - h + ph) * 0.5) ;
			}
			drawState.scale.x = w / spr->GetWidth();
			drawState.scale.y = h / spr->GetHeight();
//			spr->SetScaleSize(w, h);
			if(geometricData.angle != 0)
			{
				float32 tmpX = drawState.position.x;
				drawState.position.x = ((tmpX - geometricData.position.x) * geometricData.cosA  + (geometricData.position.y - drawState.position.y) * geometricData.sinA + geometricData.position.x);
				drawState.position.y = ((tmpX - geometricData.position.x) * geometricData.sinA  + (drawState.position.y - geometricData.position.y) * geometricData.cosA + geometricData.position.y);
				drawState.angle = geometricData.angle;
//				spr->SetAngle(geometricData.angle);
			}
//			spr->SetPosition((float32)x, (float32)y);
			{
				switch(perPixelAccuracyType)
				{
				case PER_PIXEL_ACCURACY_ENABLED:
					if(lastDrawPos == drawState.position)
					{
						drawState.usePerPixelAccuracy = true;
					}
					break;
				case PER_PIXEL_ACCURACY_FORCED:
					drawState.usePerPixelAccuracy = true;
					break;
				default:
					break;
				}
			}

			lastDrawPos = drawState.position;
			
			spr->Draw(&drawState);
		}
		break;
		
		case DRAW_FILL:
		{
			DrawFilled( geometricData );
		}	
		break;
			
		case DRAW_STRETCH_BOTH:
		case DRAW_STRETCH_HORIZONTAL:
		case DRAW_STRETCH_VERTICAL:
			DrawStretched(drawRect);
		break;
		
		case DRAW_TILED:
			DrawTiled( geometricData );
		break;
	}
	
	RenderManager::Instance()->ResetColor();
	
}
	
void UIControlBackground::DrawStretched(const Rect &drawRect)
{
	if (!spr)return;
	Texture *texture = spr->GetTexture(frame);
	
	float32 texX = spr->GetRectOffsetValueForFrame(frame, Sprite::X_POSITION_IN_TEXTURE);
	float32 texY = spr->GetRectOffsetValueForFrame(frame, Sprite::Y_POSITION_IN_TEXTURE);
	float32 texDx = spr->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_WIDTH);
	float32 texDy = spr->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_HEIGHT);
    float32 texOffX = spr->GetRectOffsetValueForFrame(frame, Sprite::X_OFFSET_TO_ACTIVE);
    float32 texOffY = spr->GetRectOffsetValueForFrame(frame, Sprite::Y_OFFSET_TO_ACTIVE);

    const float32 spriteWidth = spr->GetWidth();
    const float32 spriteHeight = spr->GetHeight();

    const float32 leftOffset  = leftStretchCap - texOffX;
    const float32 rightOffset = leftStretchCap - ( spriteWidth - texDx - texOffX );
    const float32 topOffset   = topStretchCap  - texOffY;
    const float32 bottomOffset= topStretchCap  - ( spriteHeight - texDy - texOffY );

    const float32 realLeftStretchCap  = Max( 0.0f, leftOffset );
    const float32 realRightStretchCap = Max( 0.0f, rightOffset );
    const float32 realTopStretchCap   = Max( 0.0f, topOffset );
    const float32 realBottomStretchCap= Max( 0.0f, bottomOffset );

    const float32 scaleFactorX = drawRect.dx / spriteWidth;
    const float32 scaleFactorY = drawRect.dy / spriteHeight;
    float32 x = drawRect.x + Max( 0.0f, -leftOffset ) * scaleFactorX;
    float32 y = drawRect.y + Max( 0.0f, -topOffset  ) * scaleFactorY;
    float32 dx = drawRect.dx - ( Max( 0.0f, -leftOffset ) + Max( 0.0f, -rightOffset  ) ) * scaleFactorX;
    float32 dy = drawRect.dy - ( Max( 0.0f, -topOffset  ) + Max( 0.0f, -bottomOffset ) ) * scaleFactorY;

    const float32 resMulFactor = 1.0f / Core::Instance()->GetResourceToVirtualFactor(spr->GetResourceSizeIndex());
//	if (spr->IsUseContentScale()) 
//	{
		texDx *= resMulFactor;
		texDy *= resMulFactor;
//	}

    const float32 leftCap  = realLeftStretchCap   * resMulFactor;
    const float32 rightCap = realRightStretchCap  * resMulFactor;
    const float32 topCap   = realTopStretchCap    * resMulFactor;
    const float32 bottomCap= realBottomStretchCap * resMulFactor;

	float32 vertices[16 * 2];
	float32 texCoords[16 * 2];
	
    float32 textureWidth = (float32)texture->GetWidth();
    float32 textureHeight = (float32)texture->GetHeight();
	
	int32 vertInTriCount = 18;
	int32 vertCount = 16;

	switch (type) 
	{
		case DRAW_STRETCH_HORIZONTAL:
		{
			float32 ddy = (spriteHeight - dy);
			y -= ddy * 0.5f;
			dy += ddy;
			
            vertices[0] = vertices[8]  = x;
            vertices[1] = vertices[3]  = vertices[5]  = vertices[7]  = y;
            vertices[4] = vertices[12] = x + dx - realRightStretchCap;

            vertices[2] = vertices[10] = x + realLeftStretchCap;
            vertices[9] = vertices[11] = vertices[13] = vertices[15] = y + dy;
            vertices[6] = vertices[14] = x + dx;

            texCoords[0] = texCoords[8]  = texX / textureWidth;
            texCoords[1] = texCoords[3]  = texCoords[5]  = texCoords[7]  = texY / textureHeight;
            texCoords[4] = texCoords[12] = (texX + texDx - rightCap) / textureWidth;

            texCoords[2] = texCoords[10] = (texX + leftCap) / textureWidth;
            texCoords[9] = texCoords[11] = texCoords[13] = texCoords[15] = (texY + texDy) / textureHeight;
            texCoords[6] = texCoords[14] = (texX + texDx) / textureWidth;
		}
		break;
		case DRAW_STRETCH_VERTICAL:
		{
			float32 ddx = (spriteWidth - dx);
			x -= ddx * 0.5f;
			dx += ddx;
			
            vertices[0] = vertices[2]  = vertices[4]  = vertices[6]  = x;
            vertices[8] = vertices[10] = vertices[12] = vertices[14] = x + dx;

            vertices[1] = vertices[9]  = y;
            vertices[3] = vertices[11] = y + realTopStretchCap;
            vertices[5] = vertices[13] = y + dy - realBottomStretchCap;
            vertices[7] = vertices[15] = y + dy;

            texCoords[0] = texCoords[2]  = texCoords[4]  = texCoords[6]  = texX / textureWidth;
            texCoords[8] = texCoords[10] = texCoords[12] = texCoords[14] = (texX + texDx) / textureWidth;

            texCoords[1] = texCoords[9]  = texY / textureHeight;
            texCoords[3] = texCoords[11] = (texY + topCap) / textureHeight;
            texCoords[5] = texCoords[13] = (texY + texDy - bottomCap) / textureHeight;
            texCoords[7] = texCoords[15] = (texY + texDy) / textureHeight;
		}
		break;
		case DRAW_STRETCH_BOTH:
		{
            vertInTriCount = 18 * 3;
            vertCount = 32;

            vertices[0] = vertices[8]  = vertices[16] = vertices[24] = x;
            vertices[2] = vertices[10] = vertices[18] = vertices[26] = x + realLeftStretchCap;
            vertices[4] = vertices[12] = vertices[20] = vertices[28] = x + dx - realRightStretchCap;
            vertices[6] = vertices[14] = vertices[22] = vertices[30] = x + dx;

            vertices[1] = vertices[3]  = vertices[5]  = vertices[7]  = y;
            vertices[9] = vertices[11] = vertices[13] = vertices[15] = y + realTopStretchCap;
            vertices[17]= vertices[19] = vertices[21] = vertices[23] = y + dy - realBottomStretchCap;
            vertices[25]= vertices[27] = vertices[29] = vertices[31] = y + dy;

            texCoords[0] = texCoords[8]  = texCoords[16] = texCoords[24] = texX / textureWidth;
            texCoords[2] = texCoords[10] = texCoords[18] = texCoords[26] = (texX + leftCap) / textureWidth;
            texCoords[4] = texCoords[12] = texCoords[20] = texCoords[28] = (texX + texDx - rightCap) / textureWidth;
            texCoords[6] = texCoords[14] = texCoords[22] = texCoords[30] = (texX + texDx) / textureWidth;

            texCoords[1]  = texCoords[3]  = texCoords[5]  = texCoords[7]  = texY / textureHeight;
            texCoords[9]  = texCoords[11] = texCoords[13] = texCoords[15] = (texY + topCap) / textureHeight;
            texCoords[17] = texCoords[19] = texCoords[21] = texCoords[23] = (texY + texDy - bottomCap)  / textureHeight;
            texCoords[25] = texCoords[27] = texCoords[29] = texCoords[31] = (texY + texDy) / textureHeight;
		}
		break;
	}
	
//	if (Core::GetContentScaleFactor() != 1.0 && RenderManager::IsRenderTarget()) 
//	{
//		for (int i = 0; i < vertCount; i++) 
//		{
//			vertices[i] *= Core::GetVirtualToPhysicalFactor();
//	}
//	}


	uint16 indeces[18 * 3] = 
	{
		0, 1, 4, 
		1, 5, 4,
		1, 2, 5,
		2, 6, 5, 
		2, 3, 6,
		3, 7, 6,

		4, 5, 8,
		5, 9, 8,
		5, 6, 9,
		6, 10, 9,
		6, 7, 10,
		7, 11, 10,

		8, 9, 12,
		9, 12, 13,
		9, 10, 13,
		10, 14, 13,
		10, 11, 14,
		11, 15, 14
	};

	vertexStream->Set(TYPE_FLOAT, 2, 0, vertices);
	texCoordStream->Set(TYPE_FLOAT, 2, 0, texCoords);

	RenderManager::Instance()->SetTexture(texture);
	RenderManager::Instance()->SetRenderEffect(RenderManager::TEXTURE_MUL_FLAT_COLOR);
    RenderManager::Instance()->SetRenderData(rdoObject);
	RenderManager::Instance()->DrawElements(PRIMITIVETYPE_TRIANGLELIST, vertInTriCount, EIF_16, indeces);

	/*GLenum glErr = glGetError();
	if (glErr != GL_NO_ERROR)
	{
		Logger::FrameworkDebug("GLError: 0x%x", glErr);
	}*/
}

void UIControlBackground::ReleaseSpecDrawData()
{
	SafeDelete(tiledDrawData);
}

void UIControlBackground::DrawTiled(const UIGeometricData &gd)
{
	if (!spr)return;

	if( leftStretchCap < 0.0f || topStretchCap < 0.0f )
		return;

	Vector2 spriteSize( spr->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_WIDTH), spr->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_HEIGHT) );

	Vector2 stretchCap( Min( leftStretchCap, spriteSize.x * 0.5f ),
						Min( topStretchCap , spriteSize.y * 0.5f ) );

	const Vector2 &size = gd.size;

	stretchCap.x = Min( stretchCap.x, size.x * 0.5f );
	stretchCap.y = Min( stretchCap.y, size.y * 0.5f );

	Texture *texture = spr->GetTexture(frame);

	bool needGenerateTileData = false;
	if( !tiledDrawData )
	{
		tiledDrawData = new TiledDrawData();
		needGenerateTileData = true;
	}
	else
	{
		needGenerateTileData |= stretchCap != tiledDrawData->lastStretchCap;
		needGenerateTileData |= frame != tiledDrawData->lastFrame;
		needGenerateTileData |= spr != tiledDrawData->lastSprite;
		needGenerateTileData |= size != tiledDrawData->lastSize;
	}

	TiledDrawData &td = *tiledDrawData;

	if( needGenerateTileData )
	{
		Vector2 textureSize( (float32)texture->GetWidth(), (float32)texture->GetHeight() );

		Vector2 sideCellSize = stretchCap;

		Vector2 centerCellSize( spriteSize.x - sideCellSize.x * 2.0f,
								spriteSize.y - sideCellSize.y * 2.0f );

		Vector2 tileAreaSize( size.x - sideCellSize.x * 2.0f,
							  size.y - sideCellSize.y * 2.0f );

		Vector2 partCellSize( tileAreaSize.x - floorf( tileAreaSize.x / centerCellSize.x ) * centerCellSize.x,
							  tileAreaSize.y - floorf( tileAreaSize.y / centerCellSize.y ) * centerCellSize.y );

		Vector2 centerCellTexSize( centerCellSize.x / textureSize.x,
								   centerCellSize.y / textureSize.y );

		Vector2 sideCellTexSize( sideCellSize.x / textureSize.x,
								 sideCellSize.y / textureSize.y );

		Size2i gridSize( (int32)ceilf( ( size.x - sideCellSize.x * 2.0f ) / centerCellSize.x ),
						 (int32)ceilf( ( size.y - sideCellSize.y * 2.0f ) / centerCellSize.y ) );

		if( sideCellSize.x > 0.0f )
			gridSize.dx += 2;
		if( sideCellSize.y > 0.0f )
			gridSize.dy += 2;

		Vector< Vector3 > cellsWidth;
		cellsWidth.resize( gridSize.dx );

		int32 beginOffset = 0;
		int32 endOffset = 0;
		if( sideCellSize.x > 0.0f )
		{
			cellsWidth.front() = Vector3( sideCellSize.x, sideCellTexSize.x, 0.0f );
			cellsWidth.back() = Vector3( sideCellSize.x, sideCellTexSize.x, sideCellTexSize.x + centerCellTexSize.x );
			beginOffset = 1;
			endOffset = 1;
		}

		if( partCellSize.x > 0.0f )
		{
			++endOffset;
			const int32 index = gridSize.dx - endOffset;
			cellsWidth[index].x = partCellSize.x;
			cellsWidth[index].y = partCellSize.x / textureSize.x;
			cellsWidth[index].z = sideCellTexSize.x;
		}

		std::fill( cellsWidth.begin() + beginOffset, cellsWidth.begin() + gridSize.dx - endOffset, Vector3( centerCellSize.x, centerCellTexSize.x, sideCellTexSize.x ) );

		Vector< Vector3 > cellsHeight;
		cellsHeight.resize( gridSize.dy );

		beginOffset = 0;
		endOffset = 0;
		if( sideCellSize.y > 0.0f )
		{
			cellsHeight.front() = Vector3( sideCellSize.y, sideCellTexSize.y, 0.0f );
			cellsHeight.back() = Vector3( sideCellSize.y, sideCellTexSize.y, sideCellTexSize.y + centerCellTexSize.y );
			beginOffset = 1;
			endOffset = 1;
		}

		if( partCellSize.y > 0.0f )
		{
			++endOffset;
			const int32 index = gridSize.dy - endOffset;
			cellsHeight[index].x = partCellSize.y;
			cellsHeight[index].y = partCellSize.y / textureSize.y;
			cellsHeight[index].z = sideCellTexSize.y;
		}
		std::fill( cellsHeight.begin() + beginOffset, cellsHeight.begin() + gridSize.dy - endOffset, Vector3( centerCellSize.y, centerCellTexSize.y, sideCellTexSize.y ) );

		int32 vertexCount = 4 *gridSize.dx * gridSize.dy;
		td.tilesVerticesList.resize( vertexCount );
		td.transformedVerList.resize( vertexCount );
		td.tilesTexCoordsList.resize( vertexCount );

		int32 indecesCount = 6 *gridSize.dx * gridSize.dy;
		td.tilesIndecesList.resize( indecesCount );

		int32 offsetIndex = 0;
		const float32 * textCoords = spr->GetTextureCoordsForFrame(frame);
		Vector2 trasformOffset;
		Vector2 cellSize;
		Vector2 texTrasformOffset;
		Vector2 texCellSize;
		Vector2 tempTexCoordsPt( textCoords[0], textCoords[1] );
		for( int32 row = 0; row < gridSize.dy; ++row )
		{
			cellSize.y = cellsHeight[row].x;
			texCellSize.y = cellsHeight[row].y;
			texTrasformOffset.y = cellsHeight[row].z;
			trasformOffset.x = 0.0f;

			for( int32 column = 0; column < gridSize.dx; ++column, ++offsetIndex )
			{
				cellSize.x = cellsWidth[column].x;
				texCellSize.x = cellsWidth[column].y;
				texTrasformOffset.x = cellsWidth[column].z;

				int32 vertIndex = offsetIndex*4;
				td.tilesVerticesList[vertIndex + 0] = trasformOffset;
				td.tilesVerticesList[vertIndex + 1] = trasformOffset + Vector2( cellSize.x, 0.0f );
				td.tilesVerticesList[vertIndex + 2] = trasformOffset + Vector2( 0.0f, cellSize.y );
				td.tilesVerticesList[vertIndex + 3] = trasformOffset + cellSize;

				const Vector2 texel = tempTexCoordsPt + texTrasformOffset;
				td.tilesTexCoordsList[vertIndex + 0] = texel;
				td.tilesTexCoordsList[vertIndex + 1] = texel + Vector2( texCellSize.x, 0.0f );
				td.tilesTexCoordsList[vertIndex + 2] = texel + Vector2( 0.0f, texCellSize.y );
				td.tilesTexCoordsList[vertIndex + 3] = texel + texCellSize;

				int32 indecesIndex = offsetIndex*6;
				td.tilesIndecesList[indecesIndex + 0] = vertIndex;
				td.tilesIndecesList[indecesIndex + 1] = vertIndex + 1;
				td.tilesIndecesList[indecesIndex + 2] = vertIndex + 2;

				td.tilesIndecesList[indecesIndex + 3] = vertIndex + 1;
				td.tilesIndecesList[indecesIndex + 4] = vertIndex + 3;
				td.tilesIndecesList[indecesIndex + 5] = vertIndex + 2;

				trasformOffset.x += cellSize.x;
			}
			trasformOffset.y += cellSize.y;
		}
		td.lastStretchCap = stretchCap;
		td.lastSize = size;
		td.lastFrame = frame;
		td.lastSprite = spr;
	}

	Matrix3 transformMatr;
	gd.BuildTransformMatrix( transformMatr );

	if( needGenerateTileData || td.lastTransformMatr != transformMatr )
	{
		for( uint32 index = 0; index < td.tilesVerticesList.size(); ++index )
		{
			td.transformedVerList[index] = td.tilesVerticesList[index] * transformMatr;
		}
		td.lastTransformMatr = transformMatr;
	}

	vertexStream->Set(TYPE_FLOAT, 2, 0, &td.transformedVerList[0]);
	texCoordStream->Set(TYPE_FLOAT, 2, 0, &td.tilesTexCoordsList[0]);

	RenderManager::Instance()->SetTexture( texture );
	RenderManager::Instance()->SetRenderEffect(RenderManager::TEXTURE_MUL_FLAT_COLOR);
	RenderManager::Instance()->SetRenderData(rdoObject);
	RenderManager::Instance()->DrawElements(PRIMITIVETYPE_TRIANGLELIST, td.tilesIndecesList.size(), EIF_32, &td.tilesIndecesList[0]);
}

void UIControlBackground::DrawFilled( const UIGeometricData &gd )
{
	if( gd.angle != 0.0f ) 
	{
		Polygon2 poly;
		gd.GetPolygon( poly );

		RenderHelper::Instance()->FillPolygon( poly );
	}
	else
	{
		RenderHelper::Instance()->FillRect( gd.GetUnrotatedRect() );
	}
}

void UIControlBackground::SetLeftRightStretchCap(float32 _leftStretchCap)
{
	leftStretchCap = _leftStretchCap;
}

void UIControlBackground::SetTopBottomStretchCap(float32 _topStretchCap)
{
	topStretchCap = _topStretchCap;
}
	
float32 UIControlBackground::GetLeftRightStretchCap() const
{
    return leftStretchCap;
}
	
float32 UIControlBackground::GetTopBottomStretchCap() const
{
    return topStretchCap;
}	

};