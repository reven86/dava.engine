/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#include "Base/BaseTypes.h"
#include "Render/2D/GraphicsFont.h"
#include "Render/RenderManager.h"
#include "FileSystem/File.h"
#include "Debug/DVAssert.h"
#include "FileSystem/LocalizationSystem.h"

namespace DAVA
{
	
GraphicsFontDefinition::GraphicsFontDefinition()
{
	characterTable = 0;
	characterPreShift = 0;
	characterWidthTable = 0;
	kerningBaseShift = 0;
	kerningTable = 0;
	
}

GraphicsFontDefinition::~GraphicsFontDefinition()
{
	SafeDeleteArray(characterTable);
	SafeDeleteArray(characterPreShift);
	SafeDeleteArray(characterWidthTable);
	SafeDeleteArray(kerningBaseShift);
	SafeDeleteArray(kerningTable);
}
	
GraphicsFont::GraphicsFont()
	:fontSprite(0)
{
	color = Color::White();
	fontType = TYPE_GRAPHICAL;
	fdef = 0;
	fontScaleCoeff = 1.0f;
    horizontalSpacing = 0;
}

GraphicsFont::~GraphicsFont()
{
	SafeRelease(fdef);
	SafeRelease(fontSprite);
}	

int32 GraphicsFont::GetHorizontalSpacing()
{
    return horizontalSpacing;
}
    
Font * GraphicsFont::Clone()
{
	GraphicsFont * cloneFont = new GraphicsFont();

	cloneFont->fdef = SafeRetain(this->fdef);	
	cloneFont->fontSprite = SafeRetain(this->fontSprite);

	cloneFont->SetColor(this->GetColor());
	cloneFont->SetVerticalSpacing(this->GetVerticalSpacing());
    cloneFont->SetHorizontalSpacing(this->GetHorizontalSpacing());
	cloneFont->SetSize(this->GetSize());
	
	return cloneFont;
}

Size2i GraphicsFont::GetStringSize(const WideString & string, Vector<int32> *charSizes)
{
	uint32 length = (uint32)string.length();
	uint16 prevChIndex = GraphicsFontDefinition::INVALID_CHARACTER_INDEX;
	Sprite::DrawState state;
	
	float32 currentX = 0;
	float32 prevX = 0;
	float32 sizeFix = 0.0f;
	for (uint32 indexInString = 0; indexInString < length; ++indexInString)
	{
		char16 c = string[indexInString];
		uint16 chIndex = fdef->CharacterToIndex(c);
		
		
		if (indexInString == 0)
		{
			DVASSERT(chIndex < fdef->tableLenght);
            if(Core::Instance()->GetPhysicalScreenWidth() == 2048)
                sizeFix = fontSprite->GetRectOffsetValueForFrame(chIndex, Sprite::X_OFFSET_TO_ACTIVE) * fontScaleCoeff;
            else
                sizeFix = fontSprite->GetRectOffsetValueForFrame(chIndex, Sprite::X_OFFSET_TO_ACTIVE) * fontScaleCoeff / Core::Instance()->GetResourceToVirtualFactor(Core::Instance()->GetDesirableResourceIndex());
			currentX -= sizeFix;
		}
		
		if (chIndex == GraphicsFontDefinition::INVALID_CHARACTER_INDEX)
		{
            if(c != '\n')
                Logger::Debug("*** Error: can't find character %c in font", c);
			if (charSizes)charSizes->push_back(0); // push zero size if character is not available
			continue;
		}
		
		if (prevChIndex != GraphicsFontDefinition::INVALID_CHARACTER_INDEX)
		{
			//Logger::Debug("kern: %c-%c = %f", string[indexInString - 1], c,  GetDistanceFromAtoB(prevChIndex, chIndex));
			DVASSERT(chIndex < fdef->tableLenght);
			currentX += GetDistanceFromAtoB(prevChIndex, chIndex);
		}
			
		currentX += (fdef->characterWidthTable[chIndex] + horizontalSpacing) * fontScaleCoeff;
		
		// BORODA: Probably charSizes should be float??? 
		int32 newSize = (int32)(currentX + sizeFix - prevX);
        
        if(Core::Instance()->GetPhysicalScreenWidth() == 2048)
            newSize = (int32)((float32)newSize*Core::GetVirtualToPhysicalFactor());
        else
            newSize = (int32)((float32)newSize);

		if (charSizes)charSizes->push_back(newSize);
		prevX = currentX;
		prevChIndex = chIndex;
	}
	prevX = currentX;
	
	DVASSERT(prevChIndex < fdef->tableLenght);

    
    if(Core::Instance()->GetPhysicalScreenWidth() == 2048)
    {
        currentX -= (fdef->characterWidthTable[prevChIndex] + horizontalSpacing) * fontScaleCoeff;
        currentX += (fdef->characterPreShift[prevChIndex] + fontSprite->GetRectOffsetValueForFrame(prevChIndex, Sprite::ACTIVE_WIDTH)) * fontScaleCoeff; // characterWidthTable[prevChIndex];
    }
    else
    {
        currentX += fdef->characterPreShift[prevChIndex] * fontScaleCoeff;
        currentX -= (fdef->characterWidthTable[prevChIndex] + horizontalSpacing) * fontScaleCoeff;
        currentX = floorf(currentX + 0.5f);
        currentX += (fdef->characterPreShift[prevChIndex] + fontSprite->GetRectOffsetValueForFrame(prevChIndex, Sprite::ACTIVE_WIDTH)/ Core::Instance()->GetResourceToVirtualFactor(Core::Instance()->GetDesirableResourceIndex())) * fontScaleCoeff;
    }

//	float32 lastCharSize = fdef->characterWidthTable[prevChIndex] * fontScaleCoeff;
//	float32 lastCharSize = fontSprite->GetRectOffsetValueForFrame(prevChIndex, Sprite::ACTIVE_WIDTH) * fontScaleCoeff;
//	currentX += lastCharSize; // characterWidthTable[prevChIndex];
	if (charSizes)charSizes->push_back((int32)(currentX + sizeFix - prevX));
    
    if(Core::Instance()->GetPhysicalScreenWidth() == 2048)
        return Size2i((int32)(currentX + sizeFix + 1.5f), GetFontHeight());
    else
        return Size2i((int32)(ceilf(currentX) + sizeFix), GetFontHeight());
}
	
bool GraphicsFont::IsCharAvaliable(char16 ch)
{
	return (fdef->CharacterToIndex(ch) != GraphicsFontDefinition::INVALID_CHARACTER_INDEX);
}

uint32 GraphicsFont::GetFontHeight()
{
	return (uint32)((fdef->fontHeight + LocalizationSystem::Instance()->graphicsFontDrawYoffset1) * fontScaleCoeff);
}

void GraphicsFont::SetHorizontalSpacing(int32 _horizontalSpacing)
{
    horizontalSpacing = _horizontalSpacing;
}

void GraphicsFont::SetSize(float32 _size)
{
	size = _size;
	fontScaleCoeff = size / (fdef->fontAscent + fdef->fontDescent);	
}


bool GraphicsFontDefinition::LoadFontDefinition(const String & fontDefName)
{
    File * file = 0;
    size_t pos = fontDefName.rfind("/");
    String fileName = fontDefName.substr(pos + 1);
    String pathName = fontDefName.substr(0, pos + 1) + LocalizationSystem::Instance()->GetCurrentLocale() + "/" + fileName;
    
    file = File::Create(pathName, File::READ|File::OPEN);
    
    if (!file)
    {
        file = File::Create(fontDefName, File::READ|File::OPEN);
        if (!file)
        {
            return false;
        }
    }
    
	char header[4];
	DVVERIFY(file->Read(header, 4) == 4);
	if ((header[0] != 'F') || (header[1] != 'D') || (header[2] != 'E') || (header[3] != 'F'))
	{
		SafeRelease(file);
		return false;
	}
	uint32 version = 0;
	DVVERIFY(file->Read(&version, 4) == 4);
	if (version != 1)
	{
		SafeRelease(file);
		return false;
	}
	
	DVVERIFY(file->Read(&fontAscent, 4) == 4);
	DVVERIFY(file->Read(&fontDescent, 4) == 4);
	DVVERIFY(file->Read(&fontLeading, 4) == 4);
	DVVERIFY(file->Read(&fontXHeight, 4) == 4);
	DVVERIFY(file->Read(&charLeftRightPadding, 4) == 4);
	DVVERIFY(file->Read(&charTopBottomPadding, 4) == 4);	
	
	fontHeight = (uint32)(fontAscent + fontDescent + fontLeading + 0.5f);
	
	DVVERIFY(file->Read(&tableLenght, 4) == 4);
	characterTable = new char16[tableLenght];
	characterPreShift = new float32[tableLenght];
	characterWidthTable = new float32[tableLenght];
	kerningBaseShift = new float32[tableLenght];
	kerningTable = new KerningPair*[tableLenght];
	
	for (int32 t = 0; t < tableLenght; ++t)
	{
		// BORODA: THIS IS FIX BECAUSE CHAR16 isn't char16 on MacOS and iPhone
		unsigned short c = 0;
		DVVERIFY(file->Read(&c, 2) == 2);
		characterTable[t] = c;
		DVVERIFY(file->Read(&characterPreShift[t], 4) == 4);
		DVVERIFY(file->Read(&characterWidthTable[t], 4) == 4);
		//Logger::Debug("char: %c idx: %d",  characterTable[t], t);
	}
	
	DVVERIFY(file->Read(&defaultShiftValue, 4) == 4);
	
	for (int t = 0; t < tableLenght; ++t)
	{
		DVVERIFY(file->Read(&kerningBaseShift[t], 4) == 4);
		//Logger::Debug("base: %c baseshift:%f preshift:%f", characterTable[t], kerningBaseShift[t], characterPreShift[t]);
	}
	
	DVVERIFY(file->Read(&kerningPairCount, 4) == 4);
	for (int32 k = 0; k < tableLenght; ++k)
		kerningTable[k] = 0;
	
	for (int32 kp = 0; kp < kerningPairCount; ++kp)
	{
		unsigned short s1short;
		DVVERIFY(file->Read(&s1short, 2) == 2); 
		unsigned short s2short;
		DVVERIFY(file->Read(&s2short, 2) == 2); 
		float32 shift;
		DVVERIFY(file->Read(&shift, 4) == 4); 
		
		KerningPair * p = new KerningPair();
		p->ch1Index = s1short;
		p->ch2Index = s2short;
		p->shift = shift;
		AddKerningPair(p);
		//file->Read(&kerningTable[s1][s2], 4, 1, fontFP);
	}
	
//	for (int32 t = 0; t < tableLenght; ++t)
//	{
//		//Logger::Debug("char check: %c idx: %d",  characterTable[t], t);
//	}
	
	
	SafeRelease(file);
	return true;
}

void GraphicsFontDefinition::AddKerningPair(KerningPair * kpair)
{
	kpair->next = kerningTable[kpair->ch1Index];
	kerningTable[kpair->ch1Index] = kpair;
}
	
GraphicsFontDefinition::KerningPair * GraphicsFontDefinition::FindKerningPair(uint16 ch1Index, uint16 ch2Index)
{
	KerningPair * current = kerningTable[ch1Index];	
	while(current != 0)
	{
		if (current->ch2Index == ch2Index)return current;
		current = current->next;
	}
	return 0;
}

uint16 GraphicsFontDefinition::CharacterToIndex(char16 c)
{
	for (int32 ci = 0; ci < tableLenght; ++ci)
	{
		if (characterTable[ci] == c)
		{
			return (uint16)ci;
		}
	}
	return INVALID_CHARACTER_INDEX;
}
	
GraphicsFont * GraphicsFont::Create(const String & fontDefName, const String & spriteName)
{
	GraphicsFont * font = new GraphicsFont();
	font->fdef = new GraphicsFontDefinition();
	if (!font->fdef->LoadFontDefinition(fontDefName))
	{
		Logger::Error("Failed to create font from definition: %s", fontDefName.c_str());
		SafeRelease(font->fdef);
		SafeRelease(font);
		return 0;
	}
	font->size = font->fdef->fontAscent + font->fdef->fontDescent;
	font->fontSprite = Sprite::Create(spriteName);
	if (!font->fontSprite)
	{
		Logger::Error("Failed to create font because sprite is not available: %s", spriteName.c_str());
		SafeRelease(font);
		return 0;
	}
	return font;
}

bool GraphicsFont::IsTextSupportsHardwareRendering() 
{ 
	return true; 
};

float32 GraphicsFont::GetDistanceFromAtoB(int32 prevChIndex, int32 chIndex)
{
	float32 currentX = 0.0f;
	currentX += fdef->defaultShiftValue;
	currentX += fdef->kerningBaseShift[prevChIndex];
	GraphicsFontDefinition::KerningPair * kpair = fdef->FindKerningPair(prevChIndex, chIndex);
	if (kpair)
		currentX += kpair->shift;
	return currentX * fontScaleCoeff;
}

Size2i GraphicsFont::DrawString(float32 x, float32 y, const WideString & string, int32 justifyWidth)
{
	uint32 length = (uint32)string.length();
    if(length==0) return Size2i();
    
	uint16 prevChIndex = GraphicsFontDefinition::INVALID_CHARACTER_INDEX;
	Sprite::DrawState state;

	state.SetPerPixelAccuracyUsage(true);
	
	float32 currentX = x;
	float32 currentY = y;
	float32 sizeFix = 0.0f;
	//Logger::Debug("%S startX:%f", string.c_str(), currentX);
    RenderManager::Instance()->SetColor(color);
	for (uint32 indexInString = 0; indexInString < length; ++indexInString)
	{
		char16 c = string[indexInString];
		uint16 chIndex = fdef->CharacterToIndex(c);
		
		if (indexInString == 0)
		{
            if(Core::Instance()->GetPhysicalScreenWidth() == 2048)
                sizeFix = fontSprite->GetRectOffsetValueForFrame(chIndex, Sprite::X_OFFSET_TO_ACTIVE) * fontScaleCoeff;
            else
                sizeFix = fontSprite->GetRectOffsetValueForFrame(chIndex, Sprite::X_OFFSET_TO_ACTIVE) * fontScaleCoeff / Core::Instance()->GetResourceToVirtualFactor(Core::Instance()->GetDesirableResourceIndex());
                
			currentX -= sizeFix;
		}
		
		if (chIndex == GraphicsFontDefinition::INVALID_CHARACTER_INDEX)
		{
            if(c != '\n')
                Logger::Debug("*** Error: can't find character %c in font", c);
			continue;
		}
		
		if (prevChIndex != GraphicsFontDefinition::INVALID_CHARACTER_INDEX)
		{
			//Logger::Debug("kern: %c-%c = %f", string[indexInString - 1], c,  GetDistanceFromAtoB(prevChIndex, chIndex));
			currentX += GetDistanceFromAtoB(prevChIndex, chIndex);
		}
		
		state.SetFrame(chIndex);
		// draw on baseline Y = currentY - fontSprite->GetHeight() + charTopBottomPadding + fontDescent

		float32 drawX = currentX;
		float32 drawY = 0;
        
        if(Core::Instance()->GetPhysicalScreenWidth() == 2048)
            drawY = currentY - (fontSprite->GetHeight() - LocalizationSystem::Instance()->graphicsFontDrawYoffset1) * fontScaleCoeff + (fdef->charTopBottomPadding + fdef->fontDescent + fdef->fontAscent - LocalizationSystem::Instance()->graphicsFontDrawYoffset2) * fontScaleCoeff;
        else
            drawY = currentY - (fontSprite->GetHeight()*Core::Instance()->GetVirtualToPhysicalFactor() - LocalizationSystem::Instance()->graphicsFontDrawYoffset1) * fontScaleCoeff + (fdef->charTopBottomPadding + fdef->fontDescent + fdef->fontAscent - LocalizationSystem::Instance()->graphicsFontDrawYoffset2) * fontScaleCoeff;
        
        if(Core::Instance()->GetPhysicalScreenWidth() == 2048)
            drawX += fdef->characterPreShift[chIndex] * fontScaleCoeff;
        else if (indexInString != 0)
            drawX += fdef->characterPreShift[chIndex] * fontScaleCoeff;
		
        if(Core::Instance()->GetPhysicalScreenWidth() == 2048)
            state.SetScale(fontScaleCoeff, fontScaleCoeff);
        else
            state.SetScale(fontScaleCoeff*Core::Instance()->GetVirtualToPhysicalFactor(), fontScaleCoeff*Core::Instance()->GetVirtualToPhysicalFactor());
        
		state.SetPosition(drawX, drawY);
        
		fontSprite->Draw(&state);
		
		currentX += (fdef->characterWidthTable[chIndex] + horizontalSpacing) * fontScaleCoeff;

		prevChIndex = chIndex;
	}
    RenderManager::Instance()->ResetColor();

	currentX -= (fdef->characterWidthTable[prevChIndex] + horizontalSpacing) * fontScaleCoeff;
    
    if(Core::Instance()->GetPhysicalScreenWidth() == 2048)
        currentX += (fdef->characterPreShift[prevChIndex] + fontSprite->GetRectOffsetValueForFrame(prevChIndex, Sprite::ACTIVE_WIDTH)) * fontScaleCoeff; // characterWidthTable[prevChIndex];
    else
        currentX += (fdef->characterPreShift[prevChIndex] + fontSprite->GetRectOffsetValueForFrame(prevChIndex, Sprite::ACTIVE_WIDTH)/ Core::Instance()->GetResourceToVirtualFactor(Core::Instance()->GetDesirableResourceIndex())) * fontScaleCoeff;
	
//	Sprite::DrawState drwState;
//	float32 drawX = 100;
//	float32 drawY = 100;
//	drwState.SetFrame(1);
//	drwState.SetPosition(drawX, drawY);
//	fontSprite->Draw(&drwState);
//	
//	RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
//	RenderManager::Instance()->DrawRect(Rect(drawX + fontSprite->GetRectOffsetValueForFrame(1, Sprite::X_OFFSET_TO_ACTIVE) * fontScaleCoeff
//											 , drawY + fontSprite->GetRectOffsetValueForFrame(1, Sprite::Y_OFFSET_TO_ACTIVE) * fontScaleCoeff
//											 , fdef->characterWidthTable[1]
//											 , fontSprite->GetHeight() * fontScaleCoeff));
//	RenderManager::Instance()->ResetColor();
	
    if(Core::Instance()->GetPhysicalScreenWidth() == 2048)
        return Size2i((int32)(currentX + sizeFix + 1.5f - x), GetFontHeight());
    else
        return Size2i((int32)(currentX - x), GetFontHeight());
}

};