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


#ifndef __DAVAENGINE_GRAPHICSFONT_H__
#define __DAVAENGINE_GRAPHICSFONT_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/Singleton.h"
#include "Base/EventDispatcher.h"
#include "Render/2D/Font.h"
#include "Render/2D/Sprite.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
	
class GraphicsFontDefinition;
class GraphicsFont : public Font
{
public:
	static GraphicsFont * Create(const FilePath & fontDefName, const FilePath & spriteName);
	
	virtual void	SetSize(float32 size);
	virtual StringMetrics GetStringMetrics(const WideString & str, Vector<float32> *charSizes = NULL) const;
	virtual bool	IsCharAvaliable(char16 ch) const;
	virtual uint32	GetFontHeight() const;
    virtual int32   GetHorizontalSpacing() const;
	
	virtual bool	IsTextSupportsHardwareRendering() const;
	virtual StringMetrics DrawString(float32 x, float32 y, const WideString & string, int32 justifyWidth = 0, int32 spaceAddon = 0, Vector<float32> *charSizes = NULL, bool draw = true) const;
	
    virtual void    SetHorizontalSpacing(int32 horizontalSpacing);
    
	virtual Font	* Clone() const;
	/**
	\brief Tests if two fonts are the same.
	*/
	virtual bool IsEqual(const Font *font) const;
	/* Put font properties into YamlNode */
	virtual YamlNode * SaveToYamlNode() const;
	//Additional functions which allow return needed values of protected properties
	Sprite *GetFontSprite();
	const FilePath & GetFontDefinitionName() const;
	
protected:
	GraphicsFont();
	virtual ~GraphicsFont();
	
	float32 GetDistanceFromAtoB(int32 aIndex, int32 bIndex) const;

	// Get the raw hash string (identical for identical fonts).
	virtual String GetRawHashString();

	int32 horizontalSpacing;
    
	Sprite * fontSprite;
	GraphicsFontDefinition * fdef;
	float32 fontScaleCoeff;

	//Additional variable to keep font definition
	FilePath fontDefinitionName;
};
		
class GraphicsFontDefinition : public BaseObject
{
protected:
	~GraphicsFontDefinition();
public:
	GraphicsFontDefinition();
	
	float32 fontAscent;		// in points
	float32 fontDescent;	// in points
	float32 fontLeading;	// in points
	float32 fontXHeight;	// in points
	uint32	fontHeight;		// in points
	float32 charLeftRightPadding;	// in points
	float32 charTopBottomPadding;	// in points
	
	int32 tableLenght;
	char16 * characterTable;
	float32 * characterPreShift;
	float32 * characterWidthTable;
	float32 defaultShiftValue;	
	float32 * kerningBaseShift;	// advance
	
	struct KerningPair
	{
		KerningPair()
		{
			ch1Index = ch2Index = 0;
			shift = 0.0f;
			next = 0;
		}
		uint16 ch1Index;
		uint16 ch2Index;
		float32 shift;
		KerningPair * next;
	};
	
	int32 kerningPairCount;
	KerningPair ** kerningTable;
	void AddKerningPair(KerningPair * kpair);
	KerningPair * FindKerningPair(uint16 ch1Index, uint16 c2Index);
	uint16 CharacterToIndex(char16 c);
    
	static const uint16 INVALID_CHARACTER_INDEX = 0xffff;
	
	bool LoadFontDefinition(const FilePath & fontDefName);
};
	

};

#endif // __DAVAENGINE_GRAPHICSFONT_H__
