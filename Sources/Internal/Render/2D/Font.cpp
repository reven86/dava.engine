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


#include "Render/2D/Font.h"
#include "Core/Core.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"
#include "FontManager.h"

#include "Utils/StringFormat.h"
#include "Utils/CRC32.h"

namespace DAVA
{
	
	
int32 Font::globalFontDPI = 72;

void Font::SetDPI(int32 dpi)
{
	globalFontDPI = dpi;
}

int32 Font::GetDPI()
{
	return globalFontDPI;
}
	
Font::Font()
:	size(14.0f)
,   renderSize(14.0f)
,	verticalSpacing(0)
{
	FontManager::Instance()->RegisterFont(this);
}

Font::~Font()
{
	FontManager::Instance()->UnregisterFont(this);
}


bool Font::IsEqual(const Font *font) const
{
    if(!font)
    {
        return false;
    }
    
	if (fontType != font->fontType) 
	{
		return false;
	}
	if (size != font->size || verticalSpacing != font->verticalSpacing)
	{
		return false;
	}
	
	return true;
}

uint32 Font::GetHashCode()
{
	String rawHashString = GetRawHashString();
	return CRC32::ForBuffer(rawHashString.c_str(), rawHashString.length());
}

String Font::GetRawHashString()
{
	return Format("%i_%.0f_%i", fontType, size, verticalSpacing);
}

void Font::SetSize(float32 _size)
{
	size = _size;
    renderSize = _size;
}

float32	Font::GetSize() const
{
	return size;
}

void Font::SetRenderSize(float32 _originalSize)
{
    // Fots with zero render size are incorrectly handled by
    // TextBlock::Prepare(), so define the minimum size.
    renderSize = Max(_originalSize, 0.1f);
}

float32	Font::GetRenderSize() const
{
    return renderSize;
}

void Font::SetVerticalSpacing(int32 _verticalSpacing)
{
	verticalSpacing = _verticalSpacing;
}

int32 Font::GetVerticalSpacing() const
{
	return verticalSpacing;
}

    
    
void Font::SplitTextBySymbolsToStrings(const WideString & text, const Vector2 & targetRectSize, Vector<WideString> & resultVector)
{
	int32 targetWidth = (int32)(targetRectSize.dx * Core::GetVirtualToPhysicalFactor());
    int32 totalSize = (int)text.length();
    
    int32 currentLineStart = 0;
	int32 currentLineEnd = 0;
    int32 currentLineDx = 0;
    
	resultVector.clear();
    
    Vector<int32> sizes;
	GetStringSize(text, &sizes);
	if(sizes.size() == 0)
	{
		return;
	}

    for(int pos = 0; pos < totalSize; pos++)
    {
        char16 t = text[pos];
        char16 tNext = 0;
        if(pos+1 < totalSize)
            tNext = text[pos+1];
        
        currentLineEnd = pos;
        
        if(t == '\n')
        {
            WideString currentLine = text.substr(currentLineStart, currentLineEnd - currentLineStart);
            resultVector.push_back(currentLine);
            
            currentLineStart = pos + 1;
            currentLineDx = 0;
        }
        if(t == '\\' && tNext == 'n')
        {
            WideString currentLine = text.substr(currentLineStart, currentLineEnd - currentLineStart);
            resultVector.push_back(currentLine);
            
            currentLineStart = pos + 2;
            currentLineDx = 0;
        }
		
		// Use additional condition to prevent endless loop, when target size is less than
		// size of one symbol (sizes[pos] > targetWidth)
		// To keep initial index logic we should always perform action currentLineDx += sizes[pos]
		// before entering this condition, so currentLineDx > 0.
        if((currentLineDx > 0) && (currentLineDx + sizes[pos] > targetWidth))
        {
            WideString currentLine = text.substr(currentLineStart, currentLineEnd - currentLineStart);
            resultVector.push_back(currentLine);
            
            currentLineStart = pos;
            currentLineDx = 0;
            pos--;
        }
        else
        {
            currentLineDx += sizes[pos];
        }
    }
    
    WideString currentLine = text.substr(currentLineStart, currentLineEnd - currentLineStart + 1);
    resultVector.push_back(currentLine);
}
    
    
void Font::SeparatorPositions::Reset()
{
    lastWordStart = lastWordEnd = currentLineEnd = 0;
    currentLineStart = -1;
}
    
bool Font::SeparatorPositions::IsLineInitialized() const
{
    return (currentLineStart != -1);
}


bool Font::IsWordSeparator(char16 t) const
{
    switch(t)
    {
        case 183: // interpunkt
        // japanese characters that cannot start line ヽヾーァィゥェォッャュョヮヵヶぁぃぅぇぉっゃゅょゎゕゖㇰㇱㇲㇳㇴㇵㇶㇷㇸㇹㇺㇻㇼㇽㇾㇿ々〻
        case 12541:
        case 12542:
        case 12540:
        case 12449:
        case 12451:
        case 12453:
        case 12455:
        case 12457:
        case 12483:
        case 12515:
        case 12517:
        case 12519:
        case 12526:
        case 12533:
        case 12534:
        case 12353:
        case 12355:
        case 12357:
        case 12359:
        case 12361:
        case 12387:
        case 12419:
        case 12421:
        case 12423:
        case 12430:
        case 12437:
        case 12438:
        case 12784:
        case 12785:
        case 12786:
        case 12787:
        case 12788:
        case 12789:
        case 12790:
        case 12791:
        case 12792:
        case 12793:
        case 12794:
        case 12795:
        case 12796:
        case 12797:
        case 12798:
        case 12799:
        case 12293:
        case 12347:
        // brackets )]｝〕〉》」』】〙〗〟’”｠»
        case 41:
        case 93:
        case 65373:
        case 12309:
        case 12297:
        case 12299:
        case 12301:
        case 12303:
        case 12305:
        case 12313:
        case 12311:
        case 12319:
        case 8217:
        case 8221:
        case 65376:
        case 187:
        // hyphens ‐゠–〜
        case 8208:
        case 12448:
        case 8211:
        case 12316:
        // delimeters ?!‼⁇⁈⁉
        case 63:
        case 33:
        case 8252:
        case 8263:
        case 8264:
        case 8265:
        // punctuation mid ・、:;,
        case 12539:
        case 12289: // ideographic comma
        case 58:
        case 59:
        case 44:
        // punctuation end 。.
        case 12290:
        case 46:
            return true;
        // chinese simplified and traditional
        case 37:
        case 62:
        case 125: 
        case 162: 
        case 168: 
        case 176:
        case 711: 
        case 713: 
        case 8213: 
        case 8214:
        case 8222: 
        case 8223: 
        case 8224: 
        case 8225: 
        case 8250: 
        case 8451: 
        case 8758:
        case 12291: 
        case 12294: 
        case 12296: 
        case 12298:
        case 12300:
        case 12302:
        case 12318:
        case 65077:
        case 65081:
        case 65085:
        case 65087:
        case 65091:
        case 65112:
        case 65114:
        case 65116:
        case 65281:
        case 65282:
        case 65285:
        case 65287:
        case 65289:
        case 65292:
        case 65294:
        case 65306:
        case 65307:
        case 65311:
        case 65341:
        case 65344:
        case 65372:
        case 65374:
        case 8212:
        case 8226:
        case 8229:
        case 8231:
        case 9588:
        case 65072:
        case 65073:
        case 65074:
        case 65075:
        case 65079:
        case 65083:
        case 65089:
        case 65103:
        case 65104:
        case 65105:
        case 65106:
        case 65107:
        case 65108:
        case 65109:
        case 65110:
        case 65380:
            return true;
    }
    
    return false;
}
    
void Font::SplitTextToStrings(const WideString & text, const Vector2 & targetRectSize, Vector<WideString> & resultVector)
{
	int32 targetWidth = (int32)(targetRectSize.dx * Core::GetVirtualToPhysicalFactor());

	enum
	{
		SKIP = 0,
		GOODCHAR,	// all characters we like (symbols, special chars, except \n and space
		FINISH,	// process last line
		EXIT,
	};

    // Yuri Coder, 2013/12/10. Replace "\n" occurrences (two chars) to '\n' (one char) is done by Yaml parser,
    // so appropriate code (state NEXTLINE)is removed from here. See please MOBWOT-6499.

    
    SeparatorPositions separator;
    
	resultVector.clear();
	int state = SKIP;
	int totalSize = (int)text.length();
	
	Vector<int32> sizes;
	GetStringSize(text, &sizes);
    if(sizes.size() == 0)
    {
        return;
    }

    bool wasSeparator = false;
	
	for(int pos = 0; state != EXIT; pos++)
	{
		char16 t = 0;
		if(pos < totalSize)
		{
			t = text[pos];
		}
		switch (state) 
		{
			case SKIP:
				if (t == 0){ state = FINISH; break; } // if end of string process FINISH state and exit
				else if (IsSpace(t))break; // if space continue with the same state
                else if(IsLineEnd(t))
				{
					// this block is copied from case NEXTLINE: if(t == 'n')
					// unlike in NEXTLINE where we ignore 2 symbols, here we ignore only one
					// so last position is pos instead of (pos-1)
					if (separator.IsLineInitialized()) // if we already have something in current line we add to result
					{
                        AddCurrentLine(text, pos, separator, resultVector);
					}else
					{
						resultVector.push_back(L""); // here we add empty line if there was no characters in current line
					}
					state = SKIP; //always switch to SKIP because we do not know here what will be next
					break;
				}
				else // everything else is good characters
				{
					state = GOODCHAR;
					separator.lastWordStart = pos;
					separator.lastWordEnd = pos;
					if (!separator.IsLineInitialized()) separator.currentLineStart = pos;
				}
				break;
			case GOODCHAR:
                {
                    bool isSeparator = IsWordSeparator(t);
                    if(IsSpace(t) || IsLineEnd(t) || t == 0 || (wasSeparator && !isSeparator)) // if we've found any possible separator process current line
				    {
                    
                        //calculate current line width
					    int currentLineWidth = 0;

					    int32 startPos = (separator.IsLineInitialized()) ? separator.currentLineStart : 0;
					    for (int i = startPos; i < pos ; i++)
					    {
						    currentLineWidth += sizes[i];
					    }
                    
                        if((currentLineWidth < targetWidth) || ((IsSpace(t) || isSeparator) && 0 == targetWidth))
                        {   // pos could be the end of line. We need to save it
                            if(IsLineEnd(t) || t == 0)
                            {
                                AddCurrentLine(text, pos, separator, resultVector);
                            }
                            else
                            {
                                separator.currentLineEnd = pos;
                                separator.lastWordEnd = pos;
                            }
                        }
                        else if(currentLineWidth == targetWidth)
                        {   // line fit all available space
                            DVASSERT(pos > separator.currentLineStart);
                        
                            AddCurrentLine(text, pos, separator, resultVector);
                        }
                        else
                        {   //currentLineWidth > targetWidth
                            int32 currentLineLength = separator.currentLineEnd - separator.currentLineStart;
                            if((currentLineLength > 0))
                            {   // use previous position of separator to split text
                            
                                pos = separator.currentLineEnd;
                                AddCurrentLine(text, pos, separator, resultVector);
							    state = SKIP;
							    break;
                            }
                            else if(pos)
                            {   // truncate text by symbol for very long word
                                if(0 == targetWidth)
                                {
                                    AddCurrentLine(text, pos, separator, resultVector);
                                }
                                else
                                {
								    int32 endPos = (separator.IsLineInitialized()) ? separator.currentLineStart : 0;
                                    for (int i = pos-1; i >= endPos; --i)
                                    {
                                        currentLineWidth -= sizes[i];
                                        if(currentLineWidth <= targetWidth)
                                        {
                                            separator.currentLineEnd = i;
                                            int32 currentLineLength = separator.currentLineEnd - separator.currentLineStart;
                                            if((currentLineLength > 0)) // use previous position of separator to split text
                                            {
                                                pos = separator.currentLineEnd-1;
                                            
                                                AddCurrentLine(text, separator.currentLineEnd, separator, resultVector);
                                            }
                                        
                                            break;
                                        }
                                    
                                        DVASSERT(i);
                                    }
                                }

							    state = SKIP;
							    break;
                            }
                            else
                            {
                                DVASSERT(0);
                            }
                        }

					    if (IsSpace(t) || IsLineEnd(t)) state = SKIP; // if cur char is space go to skip
					    else if (t == 0) state = FINISH;
                        else if(wasSeparator && !isSeparator)
                        {
                            // good char after separator
					        separator.lastWordStart = pos;
					        separator.lastWordEnd = pos;
					        if (!separator.IsLineInitialized()) separator.currentLineStart = pos;
                        }
                        wasSeparator = isSeparator;
				    }
                }
				break;
			case FINISH:
				if (separator.IsLineInitialized()) // we check if we have something left in currentline and add this line to results
				{
                    DVASSERT(separator.currentLineEnd > separator.currentLineStart);
                    AddCurrentLine(text, separator.currentLineEnd, separator, resultVector);
				}
				state = EXIT; // always exit from here
				break;
		};
	};
}

void Font::AddCurrentLine(const WideString & text, const int32 pos, SeparatorPositions & separatorPosition, Vector<WideString> & resultVector) const
{
    WideString currentLine = text.substr(separatorPosition.currentLineStart, pos - separatorPosition.currentLineStart);
    resultVector.push_back(currentLine);
    
    separatorPosition.Reset();
}

    
Font::eFontType Font::GetFontType() const
{
    return  fontType;
}

YamlNode * Font::SaveToYamlNode() const
{
    YamlNode *node = new YamlNode(YamlNode::TYPE_MAP);
    
    VariantType *nodeValue = new VariantType();
    //Type
    node->Set("type", "Font");
    //Font size
    node->Set("size", this->GetSize());
    //Vertical Spacing
    node->Set("verticalSpacing", this->GetVerticalSpacing());

    SafeDelete(nodeValue);
    
    return node;
}

Size2i Font::DrawString(float32 /*offsetX*/, float32 /*offsetY*/, const WideString & /*str*/, int32 /*justifyWidth*/)
{
	return Size2i(0, 0);
}

Size2i Font::DrawStringToBuffer(void * /*buffer*/, int32 /*bufWidth*/, int32 /*bufHeight*/, int32 /*offsetX*/, int32 /*offsetY*/, int32 /*justifyWidth*/, int32 /*spaceAddon*/, const WideString & /*str*/, bool /*contentScaleIncluded*/)
{
	return  Size2i(0, 0);
}

};