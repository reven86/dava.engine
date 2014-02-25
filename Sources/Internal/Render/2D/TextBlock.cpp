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
	constFont = NULL;
	isMultilineEnabled = false;
	fittingType = FITTING_DISABLED;
	sprite = NULL;
	originalFontSize = 0.1f;
	align = ALIGN_HCENTER|ALIGN_VCENTER;
	RegisterTextBlock(this);
	isPredrawed = true;
    isMultilineBySymbolEnabled = false;
    
    pointsStr = L"";
}

TextBlock::~TextBlock()
{
	SafeRelease(sprite);
	SafeRelease(font);
	SafeRelease(constFont);
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

    mutex.Unlock();
	Prepare();
}
   
void TextBlock::SetRectSize(const Vector2 & size)
{
    mutex.Lock();
	if (rectSize != size)
	{
		rectSize = size;

        mutex.Unlock();
		Prepare();
        return;
	}
    mutex.Unlock();
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
    mutex.Unlock();

	return sprite != NULL;
}



void TextBlock::Prepare()
{
    mutex.Lock();

    if(!font || text == L"")
    {
        cacheFinalSize = Vector2(0.f, 0.f);
        cacheW = 0;
        cacheDx = 0;
        cacheDy = 0;
    }
    else
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
        
		if(!isMultilineEnabled)
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
            else if(!((fittingType & FITTING_REDUCE) || (fittingType & FITTING_ENLARGE)) && (drawSize.x < textSize.dx))
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
                        
                        savedStr = pointsStr;
                    }
                }
            }
			else if(((fittingType & FITTING_REDUCE) || (fittingType & FITTING_ENLARGE)) && (requestedSize.dy >= 0 || requestedSize.dx >= 0))
			{
				bool isChanged = false;
				float prevFontSize = font->GetSize();
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
							if (prevFontSize < font->GetSize())
							{
								font->SetSize(prevFontSize);
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
						else if((isChanged || fittingType & FITTING_ENLARGE) && textSize.dy < drawSize.y * 0.95)
						{
							yLower = true;
							yMul = (drawSize.y * 0.95f) / textSize.dy;
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
							if (prevFontSize < font->GetSize())
							{
								font->SetSize(prevFontSize);
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
					
					float finalSize = font->GetSize();
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
					font->SetSize(finalSize);
					textSize = font->GetStringSize(text);
				};
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
				float lastSize = font->GetSize();
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
							if(lastSize < font->GetSize())
							{
								font->SetSize(lastSize);
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
						}
					}
					
					if(!yBigger && !yLower)
					{
						break;
					}
					
					lastHeight = (float32)textSize.dy;
					
					float finalSize = lastSize = font->GetSize();
					isChanged = true;
					finalSize *= yMul;
					
					font->SetSize(finalSize);
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
    }

    TextBlockData *jobData = new TextBlockData();
    jobData->font = SafeRetain(font);
    
    mutex.Unlock();

	Retain();
	ScopedPtr<Job> job = JobManager::Instance()->CreateJob(JobManager::THREAD_MAIN, Message(this, &TextBlock::PrepareInternal, jobData));
}

void TextBlock::PrepareInternal(BaseObject * caller, void * param, void *callerData)
{
#if 1
    
    TextBlockData *jobData = (TextBlockData *)param;
    
    mutex.Lock();

    SafeRelease(sprite);
	if(!jobData->font || text == L"")
	{
        SafeDelete(jobData);
        mutex.Unlock();
        Release();
		return;
	}
    else
	{
		int16 * buf = 0;
		if (jobData->font->IsTextSupportsSoftwareRendering())
		{
			int bsz = cacheDx * cacheDy;
			buf = new int16[bsz];
			memset(buf, 0, bsz * sizeof(int16));
			
			DrawToBuffer(jobData->font, buf);
            
            String addInfo;
			if(!isMultilineEnabled)
			{
				addInfo = WStringToString(text.c_str());
			}
			else
			{
				if (multilineStrings.size() >= 1)
				{
					addInfo = WStringToString(multilineStrings[0].c_str());
				}else
				{
					addInfo = "empty";
				}
			}
			
			Texture *tex = Texture::CreateTextFromData(FORMAT_RGBA4444, (uint8*)buf, cacheDx, cacheDy, false, addInfo.c_str());
			delete[] buf;
			sprite = Sprite::CreateFromTexture(tex, 0, 0, cacheFinalSize.x, cacheFinalSize.y);
			SafeRelease(tex);
		}
		else 
		{
			//omg 8888!
			sprite = Sprite::CreateAsRenderTarget(cacheFinalSize.x, cacheFinalSize.y, FORMAT_RGBA8888);
			if (sprite && sprite->GetTexture())
			{
				if (!isMultilineEnabled)
					sprite->GetTexture()->SetDebugInfo(WStringToString(text));
				else if (isMultilineEnabled)
				{
					if (multilineStrings.size() > 0)
						sprite->GetTexture()->SetDebugInfo(WStringToString(multilineStrings[0]));
				}
			}				
		}
        
        isPredrawed = false;
	}
#endif 
    

    SafeDelete(jobData);
    mutex.Unlock();
	Release();
}

void TextBlock::DrawToBuffer(Font *realFont, int16 *buf)
{
	Size2i realSize;
	if(!isMultilineEnabled)
	{
        WideString drawText = text;
        if(pointsStr.length())
        {
            drawText = pointsStr;
        }
        
		if (buf)
		{
			realSize = realFont->DrawStringToBuffer(buf, cacheDx, cacheDy, 0, 0, 0, 0, drawText, true);
		}
		else
		{
			if (cacheUseJustify) 
			{
                realSize = realFont->DrawString(0, 0, drawText, (int32)ceilf(Core::GetVirtualToPhysicalFactor() * cacheW));
			}
			else 
			{
                realSize = realFont->DrawString(0, 0, drawText);
			}
		}
	}
	else
	{
		uint32 yOffset = 0;
		int32 fontHeight = realFont->GetFontHeight() + realFont->GetVerticalSpacing();
		for (int32 line = 0; line < (int32)multilineStrings.size(); ++line)
		{
			if (line >= (int32)multilineStrings.size() - 1) 
			{
				cacheUseJustify = false;
			}
			int32 xo = 0;
			if(align & ALIGN_RIGHT)
			{
				xo = (int32)(cacheFinalSize.x - stringSizes[line]);
				if(xo < 0)
				{
					xo = 0;
				}
			}
			else if(align & ALIGN_HCENTER)
			{
				xo = (int32)(cacheFinalSize.x - stringSizes[line]) / 2;
				if(xo < 0)
				{
					xo = 0;
				}
			}
			Size2i ds;
			if (buf)
			{
				if (cacheUseJustify) 
				{
					ds = realFont->DrawStringToBuffer(buf, cacheDx, cacheDy, (int32)(Core::GetVirtualToPhysicalFactor() * xo), (int32)(Core::GetVirtualToPhysicalFactor() * yOffset),
						(int32)ceilf(Core::GetVirtualToPhysicalFactor() * cacheW), (int32)ceilf(Core::GetVirtualToPhysicalFactor() * stringSizes[line]), multilineStrings[line], true);
				}
				else 
				{
					ds = realFont->DrawStringToBuffer(buf, cacheDx, cacheDy, (int32)(Core::GetVirtualToPhysicalFactor() * xo), (int32)(Core::GetVirtualToPhysicalFactor() * yOffset),
						0, 0, multilineStrings[line], true);
				}
				
			}
			else 
			{
				if (cacheUseJustify) 
				{
					ds = realFont->DrawString((float32)xo, (float32)yOffset, multilineStrings[line], (int32)ceilf(Core::GetVirtualToPhysicalFactor() * cacheW));
				}
				else 
				{
					ds = realFont->DrawString((float32)xo, (float32)yOffset, multilineStrings[line], 0);
				}
				
			}
			
			
			realSize.dx = Max(realSize.dx, (int32)(Core::GetPhysicalToVirtualFactor() * ds.dx));
			yOffset += fontHeight;
			realSize.dy = yOffset;
		}	
	}
}

	
void TextBlock::PreDraw()
{
	if (isPredrawed)
	{
		return;
	}
	
	isPredrawed = true;
	
	if (!font->IsTextSupportsSoftwareRendering())
	{
		RenderManager::Instance()->SetRenderTarget(sprite);

		DrawToBuffer(font, NULL);
		
		RenderManager::Instance()->RestoreRenderTarget();
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