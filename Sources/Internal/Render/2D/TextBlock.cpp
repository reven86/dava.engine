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


#include "Render/2D/Sprite.h"
#include "Debug/DVAssert.h"
#include "Utils/Utils.h"
#include "Render/RenderManager.h"
#include "Utils/StringFormat.h"
#include "Platform/SystemTimer.h"
#include "FileSystem/File.h"
#include "Render/2D/TextBlock.h"
#include "Core/Core.h"
#include "Job/JobManager.h"
#include "Job/JobWaiter.h"

#include "Render/2D/TextBlockSoftwareRender.h"
#include "Render/2D/TextBlockGraphicsRender.h"
#include "Render/2D/TextBlockDistanceRender.h"

namespace DAVA 
{
    
struct TextBlockData
{
    TextBlockData(): font(NULL) { };
    ~TextBlockData() { SafeRelease(font); };
    
    Font *font;
};
    
    
    
    
//TODO: использовать мапу	
static	Vector<TextBlock *> registredBlocks;
	
#define NEW_RENDER 1
	
void RegisterTextBlock(TextBlock *tbl)
{
	registredBlocks.push_back(tbl);
}
	
void UnregisterTextBlock(TextBlock *tbl)
{
	for(Vector<TextBlock *>::iterator it = registredBlocks.begin(); it != registredBlocks.end(); it++)
	{
		if (tbl == *it) 
		{
			registredBlocks.erase(it);
			return;
		}
	}
}

void TextBlock::ScreenResolutionChanged()
{
	Logger::FrameworkDebug("Regenerate text blocks");
	for(Vector<TextBlock *>::iterator it = registredBlocks.begin(); it != registredBlocks.end(); it++)
	{
		(*it)->Prepare();
	}
}

TextBlock * TextBlock::Create(const Vector2 & size)
{
	TextBlock * textSprite = new TextBlock();
	textSprite->SetRectSize(size);
	return textSprite;
}

	
TextBlock::TextBlock()
    : cacheFinalSize(0.f, 0.f)
    , cacheW(0)
    , cacheDx(0)
    , cacheDy(0)
{
	font = NULL;
	isMultilineEnabled = false;
	fittingType = FITTING_DISABLED;

	needRedraw = true;

	originalFontSize = 0.1f;
	align = ALIGN_HCENTER|ALIGN_VCENTER;
	RegisterTextBlock(this);
    isMultilineBySymbolEnabled = false;
    treatMultilineAsSingleLine = false;
    
	textBlockRender = NULL;
}

TextBlock::~TextBlock()
{
	SafeRelease(textBlockRender);
	SafeRelease(font);
	UnregisterTextBlock(this);
}

// Setters // Getters
	
void TextBlock::SetFont(Font * _font)
{
    mutex.Lock();
    
	if (!_font || _font == font)
	{
        mutex.Unlock();
		return;
	}

	SafeRelease(font);
	font = SafeRetain(_font);

	originalFontSize = font->GetSize();
	
	SafeRelease(textBlockRender);
	switch (font->GetFontType()) {
		case Font::TYPE_FT:
			textBlockRender = new TextBlockSoftwareRender(this);
			break;
		case Font::TYPE_GRAPHICAL:
			textBlockRender = new TextBlockGraphicsRender(this);
			break;
		case Font::TYPE_DISTANCE:
			textBlockRender = new TextBlockDistanceRender(this);
			break;
			
		default:
			DVASSERT(!"Unknown font type");
			break;
	}
	
	needRedraw = true;

    mutex.Unlock();
	Prepare();
}
   
void TextBlock::SetRectSize(const Vector2 & size)
{
    mutex.Lock();
	if (rectSize != size)
	{
		rectSize = size;
        needRedraw = true;

        mutex.Unlock();
		Prepare();
        return;
	}
    mutex.Unlock();
}
	
void TextBlock::SetPosition(const Vector2& position)
{
	this->position = position;
}
	
void TextBlock::SetPivotPoint(const Vector2& pivotPoint)
{
	this->pivotPoint = pivotPoint;
}

void TextBlock::SetText(const WideString & _string, const Vector2 &requestedTextRectSize)
{
    mutex.Lock();
	if(text == _string && requestedSize == requestedTextRectSize)
	{
        mutex.Unlock();
		return;
	}
	requestedSize = requestedTextRectSize;
	text = _string;
    needRedraw = true;

    mutex.Unlock();
	Prepare();
}

void TextBlock::SetMultiline(bool _isMultilineEnabled, bool bySymbol)
{
    mutex.Lock();
	if (isMultilineEnabled != _isMultilineEnabled || isMultilineBySymbolEnabled != bySymbol)
	{
        isMultilineBySymbolEnabled = bySymbol;
		isMultilineEnabled = _isMultilineEnabled;
        needRedraw = true;

        mutex.Unlock();
		Prepare();
        return;
	}
    mutex.Unlock();
}

void TextBlock::SetFittingOption(int32 _fittingType)
{
    mutex.Lock();
	if (fittingType != _fittingType)
	{
		fittingType = _fittingType;
        needRedraw = true;

        mutex.Unlock();
		Prepare();
        return;
	}
    mutex.Unlock();
}
	
	
Font * TextBlock::GetFont()
{
    mutex.Lock();
    mutex.Unlock();
    
	return font;
}
    
const Vector<WideString> & TextBlock::GetMultilineStrings()
{
    mutex.Lock();
    mutex.Unlock();

    return multilineStrings;
}
    
const WideString & TextBlock::GetText()
{
    mutex.Lock();
    mutex.Unlock();

	return text;
}

bool TextBlock::GetMultiline()
{
    mutex.Lock();
    mutex.Unlock();

	return isMultilineEnabled;
}
    
bool TextBlock::GetMultilineBySymbol()
{
    mutex.Lock();
    mutex.Unlock();

    return isMultilineBySymbolEnabled;
}

int32 TextBlock::GetFittingOption()
{
    mutex.Lock();
    mutex.Unlock();

	return fittingType;
}
	
void TextBlock::SetAlign(int32 _align)
{
    mutex.Lock();
	if (align != _align) 
	{
		align = _align;
        needRedraw = true;

        mutex.Unlock();
		Prepare();
        return;
	}
    mutex.Unlock();
}

int32 TextBlock::GetAlign()
{
    mutex.Lock();
    mutex.Unlock();

	return align;
}

Sprite * TextBlock::GetSprite()
{
    mutex.Lock();

	Sprite* sprite = NULL;
	if (textBlockRender)
		sprite = textBlockRender->GetSprite();

	DVASSERT(sprite);
	if (!sprite) 
	{
		sprite = Sprite::CreateAsRenderTarget(8, 8, FORMAT_RGBA4444);
        Logger::Error("[Textblock] getting NULL sprite");
	}

    mutex.Unlock();
	
    return sprite;
}
	
bool TextBlock::IsSpriteReady()
{
	mutex.Lock();
    Sprite* sprite = NULL;
	if (textBlockRender)
    {
        sprite = textBlockRender->GetSprite();
    }

    mutex.Unlock();
	return sprite != NULL;
}

void TextBlock::Prepare()
{
	Retain();
	ScopedPtr<Job> job = JobManager::Instance()->CreateJob(JobManager::THREAD_MAIN, Message(this, &TextBlock::PrepareInternal));
}

void TextBlock::PrepareInternal(BaseObject * caller, void * param, void *callerData)
{
	if(!font || text == L"")
	{
        Release();
		return;
	}
	
	if(needRedraw)
	{
		bool useJustify = ((align & ALIGN_HJUSTIFY) != 0);
		font->SetSize(originalFontSize);
		Vector2 drawSize = rectSize;
		
		if(requestedSize.dx > 0)
		{
			drawSize.x = requestedSize.dx;
		}
		if(requestedSize.dy > 0)
		{
			drawSize.y = requestedSize.dy;
		}
		
		int32 w = (int32)drawSize.x;
		int32 h = (int32)drawSize.y;
		
		Size2i textSize;
		stringSizes.clear();

        // This is a temporary fix to correctly handle long multiline texts
        // which can't be broken to the separate lines.
        if (isMultilineEnabled)
        {
            if(isMultilineBySymbolEnabled)
            {
                font->SplitTextBySymbolsToStrings(text, drawSize, multilineStrings);
            }
            else
            {
                font->SplitTextToStrings(text, drawSize, multilineStrings);
            }
            
            treatMultilineAsSingleLine = multilineStrings.size() == 1;
        }

		if(!isMultilineEnabled || treatMultilineAsSingleLine)
		{
			textSize = font->GetStringSize(text);
            pointsStr.clear();
            if(fittingType & FITTING_POINTS)
            {
                if(drawSize.x < textSize.dx)
                {
                    Size2i textSizePoints;
                    
                    int32 length = (int32)text.length();
                    for(int32 i = length - 1; i > 0; --i)
                    {
                        pointsStr.clear();
                        pointsStr.append(text, 0, i);
                        pointsStr += L"...";
                        
                        textSize = font->GetStringSize(pointsStr);
                        if(textSize.dx <= drawSize.x)
                        {
                            break;
                        }
                    }
                }
            }
            else if(!((fittingType & FITTING_REDUCE) || (fittingType & FITTING_ENLARGE)) && (drawSize.x < textSize.dx) && (requestedSize.x >= 0))
            {
                Size2i textSizePoints;
                int32 length = (int32)text.length();
                if(ALIGN_RIGHT & align)
                {
                    for(int32 i = 1; i < length - 1; ++i)
                    {
                        pointsStr.clear();
                        pointsStr.append(text, i, length - i);
                        
                        textSize = font->GetStringSize(pointsStr);
                        if(textSize.dx <= drawSize.x)
                        {
                            break;
                        }
                    }
                }
                else if(ALIGN_HCENTER & align)
                {
                    int32 endPos = length / 2;
                    int32 startPos = endPos - 1;
                    
                    int32 count = endPos;
                    WideString savedStr = L"";
                    
                    for(int32 i = 1; i < count; ++i)
                    {
                        pointsStr.clear();
                        pointsStr.append(text, startPos, endPos - startPos);
                        
                        textSize = font->GetStringSize(pointsStr);
                        if(drawSize.x <= textSize.dx)
                        {
                            break;
                        }
                        
                        --startPos;
                        ++endPos;
                    }
                }
            }
			else if(((fittingType & FITTING_REDUCE) || (fittingType & FITTING_ENLARGE)) && (requestedSize.dy >= 0 || requestedSize.dx >= 0))
			{
				bool isChanged = false;
				float prevFontSize = font->GetRenderSize();
				while (true)
				{
					float yMul = 1.0f;
					float xMul = 1.0f;
					
					bool xBigger = false;
					bool xLower = false;
					bool yBigger = false;
					bool yLower = false;
					if(requestedSize.dy >= 0)
					{
						h = textSize.dy;
						if((isChanged || fittingType & FITTING_REDUCE) && textSize.dy > drawSize.y)
						{
							if (prevFontSize < font->GetRenderSize())
							{
								font->SetRenderSize(prevFontSize);
								textSize = font->GetStringSize(text);
								h = textSize.dy;
								if (requestedSize.dx >= 0)
								{
									w = textSize.dx;
								}
								break;
							}
							yBigger = true;
							yMul = drawSize.y / textSize.dy;
						}
						else if((isChanged || fittingType & FITTING_ENLARGE) && textSize.dy < drawSize.y * 0.9)
						{
							yLower = true;
							yMul = (drawSize.y * 0.9f) / textSize.dy;
							if(yMul < 1.01f)
							{
								yLower = false;
							}
						}
					}
					
					if(requestedSize.dx >= 0)
					{
						w = textSize.dx;
						if((isChanged || fittingType & FITTING_REDUCE) && textSize.dx > drawSize.x)
						{
							if (prevFontSize < font->GetRenderSize())
							{
								font->SetRenderSize(prevFontSize);
								textSize = font->GetStringSize(text);
								w = textSize.dx;
								if (requestedSize.dy >= 0)
								{
									h = textSize.dy;
								}
								break;
							}
							xBigger = true;
							xMul = drawSize.x / textSize.dx;
						}
						else if((isChanged || fittingType & FITTING_ENLARGE) && textSize.dx < drawSize.x * 0.95)
						{
							xLower = true;
							xMul = (drawSize.x * 0.95f) / textSize.dx;
							if(xMul < 1.01f)
							{
								xLower = false;
							}
						}
					}
					
					
					if((!xBigger && !yBigger) && (!xLower || !yLower))
					{
						break;
					}
					
					float finalSize = font->GetRenderSize();
					prevFontSize = finalSize;
					isChanged = true;
					if(xMul < yMul)
					{
						finalSize *= xMul;
					}
					else
					{
						finalSize *= yMul;
					}
					font->SetRenderSize(finalSize);
					textSize = font->GetStringSize(text);
				};
			}
            
            if (treatMultilineAsSingleLine)
            {
                // Another temporary solution to return correct multiline strings/
                // string sizes.
                multilineStrings.clear();
                stringSizes.clear();
                multilineStrings.push_back(text);
				stringSizes.push_back(font->GetStringSize(text).dx);
            }
		}
		else //if(!isMultilineEnabled)
		{
			if(fittingType && (requestedSize.dy >= 0/* || requestedSize.dx >= 0*/) && text.size() > 3)
			{
                //				Logger::FrameworkDebug("Fitting enabled");
				Vector2 rectSz = rectSize;
				if(requestedSize.dx > 0)
				{
					rectSz.dx = requestedSize.dx;
				}
                if(isMultilineBySymbolEnabled)
                    font->SplitTextBySymbolsToStrings(text, rectSz, multilineStrings);
                else
                    font->SplitTextToStrings(text, rectSz, multilineStrings);
				
				textSize.dx = 0;
				textSize.dy = 0;
				
				int32 yOffset = font->GetVerticalSpacing();
                //				int32 fontHeight = font->GetFontHeight() + 1 + yOffset;
                //				textSize.dy = yOffset*2 + fontHeight * (int32)multilineStrings.size();
				int32 fontHeight = font->GetFontHeight() + yOffset;
				textSize.dy = fontHeight * (int32)multilineStrings.size() - yOffset;
				float lastSize = font->GetRenderSize();
				float lastHeight = (float32)textSize.dy;
				
				bool isChanged = false;
				while (true)
				{
					float yMul = 1.0f;
					
					bool yBigger = false;
					bool yLower = false;
					if(requestedSize.dy >= 0)
					{
						h = textSize.dy;
						if((isChanged || fittingType & FITTING_REDUCE) && textSize.dy > drawSize.y)
						{
							yBigger = true;
							yMul = drawSize.y / textSize.dy;
							if(lastSize < font->GetRenderSize())
							{
								font->SetRenderSize(lastSize);
								h = (int32)lastHeight;
								break;
							}
						}
						else if((isChanged || fittingType & FITTING_ENLARGE) && textSize.dy < drawSize.y * 0.95)
						{
							yLower = true;
							if(textSize.dy < drawSize.y * 0.75f)
							{
								yMul = (drawSize.y * 0.75f) / textSize.dy;
							}
							else if(textSize.dy < drawSize.y * 0.8f)
							{
								yMul = (drawSize.y * 0.8f) / textSize.dy;
							}
							else if(textSize.dy < drawSize.y * 0.85f)
							{
								yMul = (drawSize.y * 0.85f) / textSize.dy;
							}
							else if(textSize.dy < drawSize.y * 0.9f)
							{
								yMul = (drawSize.y * 0.9f) / textSize.dy;
							}
							else
							{
								yMul = (drawSize.y * 0.95f) / textSize.dy;
							}
                            if (yMul == 1.0f)
                            {
                                yMul = 1.05f;
                            }
						}
					}
					
					if(!yBigger && !yLower)
					{
						break;
					}
					
					lastHeight = (float32)textSize.dy;
					
					float finalSize = lastSize = font->GetRenderSize();
					isChanged = true;
					finalSize *= yMul;
					
					font->SetRenderSize(finalSize);
                    //					textSize = font->GetStringSize(text);
                    
                    if(isMultilineBySymbolEnabled)
                        font->SplitTextBySymbolsToStrings(text, rectSz, multilineStrings);
                    else
                        font->SplitTextToStrings(text, rectSz, multilineStrings);
					
					textSize.dy = 0;
					
					int32 yOffset = font->GetVerticalSpacing();
                    //					int32 fontHeight = font->GetFontHeight() + 1 + yOffset;
                    //					textSize.dy = yOffset*2 + fontHeight * (int32)multilineStrings.size();
					int32 fontHeight = font->GetFontHeight() + yOffset;
					textSize.dy = fontHeight * (int32)multilineStrings.size() - yOffset;
					
				};
				
			}
            //			Logger::FrameworkDebug("Font size: %.4f", font->GetSize());
            
            
			Vector2 rectSz = rectSize;
			if(requestedSize.dx > 0)
			{
				rectSz.dx = requestedSize.dx;
			}
            if(isMultilineBySymbolEnabled)
                font->SplitTextBySymbolsToStrings(text, rectSz, multilineStrings);
            else
                font->SplitTextToStrings(text, rectSz, multilineStrings);
			
			textSize.dx = 0;
			textSize.dy = 0;
			
			int32 yOffset = font->GetVerticalSpacing();
            //			Logger::FrameworkDebug("yOffset = %.4d", yOffset);
            //			int32 fontHeight = font->GetFontHeight() + 1 + yOffset;
            //			textSize.dy = yOffset*2 + fontHeight * (int32)multilineStrings.size();
			int32 fontHeight = font->GetFontHeight() + yOffset;
            //			Logger::FrameworkDebug("fontHeight = %.4d", fontHeight);
			textSize.dy = fontHeight * (int32)multilineStrings.size() - yOffset;

            stringSizes.reserve(multilineStrings.size());
			for (int32 line = 0; line < (int32)multilineStrings.size(); ++line)
			{
				Size2i stringSize = font->GetStringSize(multilineStrings[line]);
				stringSizes.push_back(stringSize.dx);
				if(requestedSize.dx >= 0)
				{
					textSize.dx = Max(textSize.dx, Min(stringSize.dx, (int)drawSize.x));
				}
				else
				{
					textSize.dx = Max(textSize.dx, stringSize.dx);
				}
			}
		}
		
		if(requestedSize.dx == 0)
		{
			w = Min(w, textSize.dx);
            //			Logger::FrameworkDebug("On size not requested: w = %d", w);
		}
		else if(requestedSize.dx < 0)
		{
			w = textSize.dx;
            //			Logger::FrameworkDebug("On size automated: w = %d", w);
		}
		if(requestedSize.dy == 0)
		{
			h = Min(h, textSize.dy);
            //			Logger::FrameworkDebug("On size not requested: h = %d", h);
		}
		else if(requestedSize.dy < 0)
		{
			h = textSize.dy;
            //			Logger::FrameworkDebug("On size automated: h = %d", w);
		}
		
		if (requestedSize.dx >= 0 && useJustify)
		{
			w = (int32)drawSize.dx;
		}
		
		
		
		//calc texture size
		int32 dx = (int32)ceilf(Core::GetVirtualToPhysicalFactor() * w);
		int32 dy = (int32)ceilf(Core::GetVirtualToPhysicalFactor() * h);
		
		cacheUseJustify = useJustify;
		cacheDx = dx;
        EnsurePowerOf2(cacheDx);
        
		cacheDy = dy;
        EnsurePowerOf2(cacheDy);
        
		cacheW = w;
		cacheFinalSize.x = (float32)dx / Core::GetVirtualToPhysicalFactor();
        cacheFinalSize.y = (float32)dy / Core::GetVirtualToPhysicalFactor();
        
        if (textBlockRender)
        {
			textBlockRender->Prepare();
        }

        needRedraw = false;
    }
    
    mutex.Unlock();
	Release();
}
	
void TextBlock::PreDraw()
{
	if (textBlockRender)
	{
		textBlockRender->PreDraw();
	}
}
	
void TextBlock::Draw(const Color& textColor, const Vector2* offset/* = NULL*/)
{
	if (textBlockRender)
	{
		textBlockRender->Draw(textColor, offset);
	}
}
    
TextBlock * TextBlock::Clone()
{
    TextBlock *block = new TextBlock();

    block->SetRectSize(rectSize);
    block->SetMultiline(GetMultiline(), GetMultilineBySymbol());
    block->SetAlign(GetAlign());
    block->SetFittingOption(fittingType);
    
    if (GetFont())
    {
        block->SetFont(GetFont());
    }
    block->SetText(GetText(), requestedSize);
    
    return block;
}

const Vector2 & TextBlock::GetTextSize()
{
    mutex.Lock();
    mutex.Unlock();
    
    return cacheFinalSize;
}

const Vector<int32> & TextBlock::GetStringSizes() const
{
	return stringSizes;
}

};