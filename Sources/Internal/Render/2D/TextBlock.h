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


#ifndef __DAVAENGINE_TEXTBLOCK_H__
#define __DAVAENGINE_TEXTBLOCK_H__

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Render/RenderBase.h"
#include "Render/Texture.h"
#include "Render/2D/Sprite.h"
#include "Render/2D/Font.h"
#include "Platform/Mutex.h"

namespace DAVA
{

class TextBlockRender;
class TextBlockSoftwareRender;
class TextBlockGraphicsRender;
class TextBlockDistanceRender;
	
/**
	\ingroup render_2d
	\brief Class to render text on the screen. 
	This class support to draw singleline / multiline text to sprites using font objects that available in SDK.
	Normally you do not need it directly and you can use UIStaticText or TextGameObject. 
 */
class TextBlock : public BaseObject
{
public:
	enum eFitType 
	{
			FITTING_DISABLED = 0
		,	FITTING_ENLARGE	= 1
		,	FITTING_REDUCE = 2
        ,   FITTING_POINTS = 4
	};
	
	static void ScreenResolutionChanged();
	
	static TextBlock * Create(const Vector2 & size);
	
	virtual void SetFont(Font * font);
	virtual void SetRectSize(const Vector2 & size);
	virtual void SetPosition(const Vector2& position);
	virtual void SetPivotPoint(const Vector2& pivotPoint);
	virtual void SetAlign(int32 align);
	virtual int32 GetAlign();
	
	//[DO NOT ACTUAL ANYMORE] if requested size is 0 - text creates in the rect with size of the drawRect on draw phase
	//if requested size is >0 - text creates int the rect with the requested size
	//if requested size in <0 - rect creates for the all text size	
	virtual void SetText(const WideString & string, const Vector2 &requestedTextRectSize = Vector2(0,0));	
	virtual void SetMultiline(bool isMultilineEnabled, bool bySymbol = false);
	virtual void SetFittingOption(int32 fittingType);//may be FITTING_DISABLED, FITTING_ENLARGE, FITTING_REDUCE, FITTING_ENLARGE | FITTING_REDUCE, FITTING_POINTS

	virtual Font * GetFont();
	virtual const WideString & GetText();
    virtual const Vector<WideString> & GetMultilineStrings();
	virtual bool GetMultiline();
    virtual bool GetMultilineBySymbol();
	virtual int32 GetFittingOption();
	
	Sprite * GetSprite();
	bool IsSpriteReady();
    
    const Vector2 & GetTextSize();

	void PreDraw();
	void Draw(const Color& textColor, const Vector2* offset = NULL);

    TextBlock * Clone();

	const Vector<int32> & GetStringSizes() const;
    
protected:
	TextBlock();
	~TextBlock();
	
	void Prepare();
	void PrepareInternal(BaseObject * caller, void * param, void *callerData);
	
	void DrawToBuffer(Font *font, uint8 *buf);

	void ProcessAlign();
	

	Vector2 rectSize;
        bool needRedraw;
	Vector2 requestedSize;

    Vector2 cacheFinalSize;

	float32 originalFontSize;
    
	int32 cacheDx;
	int32 cacheDy;
	int32 cacheW;

    int32 fittingType;
	Vector2 position;
	Vector2 pivotPoint;
	int32 align;

	Font * font;
	WideString text;
    WideString pointsStr;
	Vector<WideString> multilineStrings;
	Vector<int32> stringSizes;
    
    Mutex mutex;

	bool isMultilineEnabled:1;
    bool isMultilineBySymbolEnabled:1;
	bool isPredrawed:1;
	bool cacheUseJustify:1;
    bool treatMultilineAsSingleLine:1;

	friend class TextBlockRender;
	friend class TextBlockSoftwareRender;
	friend class TextBlockGraphicsRender;
	friend class TextBlockDistanceRender;
	TextBlockRender* textBlockRender;
};

}; //end of namespace

#endif // __DAVAENGINE_TEXTBLOCK_H__