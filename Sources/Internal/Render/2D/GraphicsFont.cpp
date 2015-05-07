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


#include "Base/BaseTypes.h"
#include "Render/2D/GraphicsFont.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "FileSystem/File.h"
#include "Debug/DVAssert.h"
#include "FileSystem/LocalizationSystem.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"
#include "FontManager.h"
#include "Utils/Utils.h"

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
	fontType = TYPE_GRAPHICAL;
	fdef = 0;
	fontScaleCoeff = 1.0f;
    horizontalSpacing = 0;
    
	FontManager::Instance()->RegisterFont(this);
}
    
bool GraphicsFont::IsEqual(const Font *font) const
{
	if (font->GetFontType() != this->GetFontType())
	{
		return false;
	}

    const GraphicsFont * gfont = DynamicTypeCheck<const GraphicsFont*>(font);
    if (!Font::IsEqual(font) ||
        fontDefinitionName != gfont->fontDefinitionName ||
        fontSprite != gfont->fontSprite)
    {
        return false;
    }
    return true;
}

String GraphicsFont::GetRawHashString()
{
    return fontDefinitionName.GetFrameworkPath() + "_" +
           fontSprite->GetRelativePathname().GetFrameworkPath() + "_" +
           Font::GetRawHashString();
}

GraphicsFont::~GraphicsFont()
{
	SafeRelease(fdef);
	SafeRelease(fontSprite);
	FontManager::Instance()->UnregisterFont(this);
}	

int32 GraphicsFont::GetHorizontalSpacing() const
{
    return horizontalSpacing;
}
    
Font * GraphicsFont::Clone() const
{
	GraphicsFont * cloneFont = new GraphicsFont();

	cloneFont->fdef = SafeRetain(this->fdef);	
	cloneFont->fontSprite = SafeRetain(this->fontSprite);

	cloneFont->SetVerticalSpacing(this->GetVerticalSpacing());
    cloneFont->SetHorizontalSpacing(this->GetHorizontalSpacing());
	cloneFont->SetSize(this->GetSize());

    cloneFont->SetAscendScale(this->GetAscendScale());
    cloneFont->SetDescendScale(this->GetDescendScale());

    cloneFont->fontDefinitionName = this->GetFontDefinitionName();
	
	return cloneFont;
}

bool GraphicsFont::IsCharAvaliable(char16 ch) const
{
	return (fdef->CharacterToIndex(ch) != GraphicsFontDefinition::INVALID_CHARACTER_INDEX);
}

uint32 GraphicsFont::GetFontHeight() const
{
	return (uint32)((fdef->fontHeight) * fontScaleCoeff);
}
	
void GraphicsFont::SetHorizontalSpacing(int32 _horizontalSpacing)
{
    horizontalSpacing = _horizontalSpacing;
}

void GraphicsFont::SetSize(float32 _size)
{
    Font::SetSize(_size);
	fontScaleCoeff = size / (fdef->fontAscent + fdef->fontDescent);
}

YamlNode * GraphicsFont::SaveToYamlNode() const
{
    YamlNode *node = Font::SaveToYamlNode();
    
    //Type
    node->Set("type", "GraphicsFont");
    //horizontalSpacing
    node->Set("horizontalSpacing", this->GetHorizontalSpacing());
    //Sprite    
    Sprite *sprite = this->fontSprite;
    if (sprite)
    {
        //Truncate sprite ".txt" extension before save
        FilePath path(sprite->GetRelativePathname());
        path.TruncateExtension();
        node->Set("sprite", path.GetAbsolutePathname());
    }    
    //Font Definition
    node->Set("definition", this->GetFontDefinitionName().GetAbsolutePathname());

    return node;
}


Sprite *GraphicsFont::GetFontSprite()
{
    return fontSprite;
}
    
const FilePath & GraphicsFont::GetFontDefinitionName() const
{
    return fontDefinitionName;
}

bool GraphicsFontDefinition::LoadFontDefinition(const FilePath & fontDefName)
{
    File * file = 0;
    
//    size_t pos = fontDefName.rfind("/");
//    String fileName = fontDefName.substr(pos + 1);
//    String pathName = fontDefName.substr(0, pos + 1) + LocalizationSystem::Instance()->GetCurrentLocale() + "/" + fileName;
    
    FilePath pathName = fontDefName.GetDirectory() + (LocalizationSystem::Instance()->GetCurrentLocale() + "/" + fontDefName.GetFilename());
    
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
		//Logger::FrameworkDebug("char: %c idx: %d",  characterTable[t], t);
	}
	
	DVVERIFY(file->Read(&defaultShiftValue, 4) == 4);
	
	for (int t = 0; t < tableLenght; ++t)
	{
		DVVERIFY(file->Read(&kerningBaseShift[t], 4) == 4);
		//Logger::FrameworkDebug("base: %c baseshift:%f preshift:%f", characterTable[t], kerningBaseShift[t], characterPreShift[t]);
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
//		//Logger::FrameworkDebug("char check: %c idx: %d",  characterTable[t], t);
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
	
GraphicsFont * GraphicsFont::Create(const FilePath & fontDefName, const FilePath & spriteName)
{
	GraphicsFont * font = new GraphicsFont();
	font->fdef = new GraphicsFontDefinition();
    //Set font Definition path
    font->fontDefinitionName = fontDefName;
	if (!font->fdef->LoadFontDefinition(fontDefName))
	{
		Logger::Error("Failed to create font from definition: %s", fontDefName.GetAbsolutePathname().c_str());
		SafeRelease(font->fdef);
		SafeRelease(font);
		return 0;
	}
    font->SetSize(font->fdef->fontAscent + font->fdef->fontDescent);
	font->fontSprite = Sprite::Create(spriteName);
	if (!font->fontSprite)
	{
		Logger::Error("Failed to create font because sprite is not available: %s", spriteName.GetAbsolutePathname().c_str());
		SafeRelease(font);
		return 0;
	}
	return font;
}

bool GraphicsFont::IsTextSupportsHardwareRendering() const
{ 
	return true; 
};

float32 GraphicsFont::GetDistanceFromAtoB(int32 prevChIndex, int32 chIndex) const
{
	float32 currentX = 0.0f;
	currentX += fdef->defaultShiftValue;
	currentX += fdef->kerningBaseShift[prevChIndex];
	GraphicsFontDefinition::KerningPair * kpair = fdef->FindKerningPair(prevChIndex, chIndex);
	if (kpair)
		currentX += kpair->shift;
	return currentX * fontScaleCoeff;
}

Font::StringMetrics GraphicsFont::GetStringMetrics(const WideString& str, Vector<float32> *charSizes) const
{
	return DrawString(0.f, 0.f, str, 0, 0, charSizes, false);
}

Font::StringMetrics GraphicsFont::DrawString(float32 x, float32 y, const WideString & string, int32 justifyWidth, int32 spaceAddon, Vector<float32> *charSizes, bool draw) const
{
	const uint32 length = static_cast<uint32>(string.length());
	if(length == 0) return Font::StringMetrics();

	uint32 countSpace = 0;
	for(uint32 i = 0; i < length; ++i)
	{
		if( L' ' == string[i])
		{
			countSpace++;
		}
    }

    int32 justifyOffset = 0;
    int32 fixJustifyOffset = 0;
    if (countSpace > 0 && justifyWidth > 0 && spaceAddon > 0)
    {
        int32 diff = justifyWidth - spaceAddon;
        justifyOffset =  diff / countSpace;
        fixJustifyOffset = diff - justifyOffset*countSpace;
    }
    
	uint16 prevChIndex = GraphicsFontDefinition::INVALID_CHARACTER_INDEX;

	Sprite::DrawState state;
	state.SetPerPixelAccuracyUsage(true);
	
	float32 currentX = x;
	float32 prevX = currentX;
	float32 currentY = y;
	float32 sizeFix = 0.0f;
	for (uint32 indexInString = 0; indexInString < length; ++indexInString)
	{
		const char16 c = string[indexInString];
		const uint16 chIndex = fdef->CharacterToIndex(c);
		
        if (justifyOffset > 0 && c == L' ')
        {
            currentX += justifyOffset;
            if (fixJustifyOffset > 0)
            {
                currentX++;
                fixJustifyOffset--;
            }
        }
        
		if (indexInString == 0)
		{
			DVASSERT(chIndex < fdef->tableLenght);
			sizeFix = fontSprite->GetRectOffsetValueForFrame(chIndex, Sprite::X_OFFSET_TO_ACTIVE) * fontScaleCoeff;
			currentX -= sizeFix;
			prevX = currentX;
		}
		
		if (chIndex == GraphicsFontDefinition::INVALID_CHARACTER_INDEX)
		{
            if(c != '\n')
                Logger::Error("*** Error: can't find character %c in font", c);

			if (charSizes) 
				charSizes->push_back(0); // push zero size if character is not available

			continue;
		}
		
		if (prevChIndex != GraphicsFontDefinition::INVALID_CHARACTER_INDEX)
		{
			DVASSERT(chIndex < fdef->tableLenght);
			currentX += GetDistanceFromAtoB(prevChIndex, chIndex);
		}

		if(draw)
		{
			float32 drawX = currentX + fdef->characterPreShift[chIndex] * fontScaleCoeff;
			float32 drawY = currentY - fontSprite->GetHeight() * fontScaleCoeff + (fdef->charTopBottomPadding + fdef->fontDescent + fdef->fontAscent) * fontScaleCoeff;
		
			state.SetFrame(chIndex);
			state.SetScale(fontScaleCoeff, fontScaleCoeff);
			state.SetPosition(drawX, drawY);
        
            RenderSystem2D::Instance()->Draw(fontSprite, &state);
		}

		currentX += (fdef->characterWidthTable[chIndex] + horizontalSpacing) * fontScaleCoeff;

        float32 newSize = Round(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysicalX(currentX - prevX));
		
		if(charSizes)
			charSizes->push_back(newSize);
		
		prevX += newSize;
		prevChIndex = chIndex;
	}

	Font::StringMetrics metrics;
	metrics.drawRect = Rect2i(0, 0, (int32)(ceilf(currentX + sizeFix - x)), GetFontHeight());
    metrics.width = metrics.drawRect.dx;
	metrics.height = metrics.drawRect.dy;
	metrics.baseline = 0;
	return metrics;
}

};