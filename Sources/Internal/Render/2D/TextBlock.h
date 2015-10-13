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

namespace DAVA
{

class TextBlockRender;
class TextBlockSoftwareRender;
class TextBlockGraphicRender;
    
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
    
    enum eUseRtlAlign
    {
        RTL_DONT_USE,
        RTL_USE_BY_CONTENT,
        RTL_USE_BY_SYSTEM
    };
    
    static void ScreenResolutionChanged();
    
    static TextBlock * Create(const Vector2 & size);
    
    virtual void SetFont(Font * font);
    virtual void SetScale(const Vector2 & scale);
    virtual void SetRectSize(const Vector2 & size);
    virtual void SetPosition(const Vector2& position);
    virtual void SetAlign(int32 align);
    virtual int32 GetAlign();
	virtual int32 GetVisualAlign(); // Return align for displaying BiDi-text (w/ mutex lock)
    virtual void SetUseRtlAlign(eUseRtlAlign useRtlAlign);
    virtual eUseRtlAlign GetUseRtlAlign();
    virtual bool IsRtl();

    
    //[DO NOT ACTUAL ANYMORE] if requested size is 0 - text creates in the rect with size of the drawRect on draw phase
    //if requested size is >0 - text creates int the rect with the requested size
    //if requested size in <0 - rect creates for the all text size	
    virtual void SetText(const WideString & string, const Vector2 &requestedTextRectSize = Vector2(0,0));	
    virtual void SetMultiline(bool isMultilineEnabled, bool bySymbol = false);
    virtual void SetFittingOption(int32 fittingType);//may be FITTING_DISABLED, FITTING_ENLARGE, FITTING_REDUCE, FITTING_ENLARGE | FITTING_REDUCE, FITTING_POINTS

    Vector2 GetPreferredSizeForWidth(float32 width);
    
    virtual Font * GetFont();
    virtual const WideString & GetText();
    virtual const WideString & GetVisualText();
    virtual const Vector<WideString> & GetMultilineStrings();
    virtual bool GetMultiline();
    virtual bool GetMultilineBySymbol();
    virtual int32 GetFittingOption();
    
    /**
    \brief Get the render size.
    \returns size in pixels
    */
    virtual float32	GetRenderSize();

    /**
    \brief Set the render size.
    \param[in] size in points
    */
    virtual void SetRenderSize(float32 renderSize);
	
    Sprite * GetSprite();
    bool IsSpriteReady();
    const Vector2& GetSpriteOffset();
    
    const Vector2 & GetTextSize();

    void PreDraw();
    void Draw(const Color& textColor, const Vector2* offset = NULL);

    TextBlock * Clone();

    const Vector<int32>& GetStringSizes();
        

#if defined(LOCALIZATION_DEBUG)
    int32 GetFittingOptionUsed();
	bool IsVisualTextCroped();
#endif

    /**
     * \brief Sets BiDi transformation support enabled.
     * \param value true to enable BiDi support.
     */
    static void SetBiDiSupportEnabled(bool value);

    /**
     * \brief Is BiDi transformations support enabled.
     * \return true if BiDi transformations supported.
     */
    static bool IsBiDiSupportEnabled();
    TextBlockRender* GetRenderer(){ return textBlockRender; }

    void SetAngle(const float32 _angle);
    void SetPivot(const Vector2& _pivot);

protected:

	TextBlock();
    TextBlock(const TextBlock& src);
    virtual ~TextBlock();

    void NeedPrepare(Texture* texture = NULL);
    void PrepareInternal();

    void CalculateCacheParams();
    void CalculateCacheParamsIfNeed();

    void SetFontInternal(Font* _font);

    Vector2 scale;
    Vector2 rectSize;
    Vector2 position;
    Vector2 requestedSize;

    Vector2 cacheFinalSize;
    Vector2 cacheSpriteOffset;
    Vector2 cacheTextSize;

    float32 originalFontSize;
    float32 renderSize;
    
    int32 cacheDx;
    int32 cacheDy;
    int32 cacheW;
    int32 cacheOx;
    int32 cacheOy;

    int32 fittingType;
#if defined(LOCALIZATION_DEBUG)
    int32 fittingTypeUsed;
    bool visualTextCroped;
#endif //LOCALIZATION_DEBUG
    int32 align;
    eUseRtlAlign useRtlAlign;

    Font * font;
    WideString logicalText;
    WideString visualText;
    Vector<WideString> multilineStrings;
    Vector<int32> stringSizes;
    
    bool isMultilineEnabled:1;
    bool isMultilineBySymbolEnabled:1;
    bool isPredrawed:1;
    bool cacheUseJustify:1;
    bool treatMultilineAsSingleLine:1;
	bool needPrepareInternal:1;
    bool isRtl : 1;
    bool needCalculateCacheParams : 1;

    static bool isBiDiSupportEnabled;   //!< true if BiDi transformation support enabled

    friend class TextBlockRender;
    friend class TextBlockSoftwareRender;
    friend class TextBlockGraphicRender;
    
    TextBlockRender* textBlockRender;    	

	float angle;
	Vector2 pivot;
};

inline void TextBlock::CalculateCacheParamsIfNeed()
{
    if (needCalculateCacheParams)
    {
        CalculateCacheParams();
    }
}

inline void TextBlock::SetPosition(const Vector2& _position)
{
    position = _position;
}

inline void TextBlock::SetAngle(const float32 _angle)
{
    angle = _angle;
}

inline void TextBlock::SetPivot(const Vector2& _pivot)
{
    pivot = _pivot;
}

inline Font* TextBlock::GetFont()
{
    return font;
}

inline const WideString& TextBlock::GetText()
{
    return logicalText;
}

inline bool TextBlock::GetMultiline()
{
    return isMultilineEnabled;
}

inline bool TextBlock::GetMultilineBySymbol()
{
    return isMultilineBySymbolEnabled;
}

inline int32 TextBlock::GetFittingOption()
{
    return fittingType;
}

inline TextBlock::eUseRtlAlign TextBlock::GetUseRtlAlign()
{
    return useRtlAlign;
}

inline int32 TextBlock::GetAlign()
{
    return align;
}

inline bool TextBlock::IsSpriteReady()
{
    return (GetSprite() != nullptr);
}

inline void TextBlock::SetBiDiSupportEnabled(bool value)
{
    isBiDiSupportEnabled = value;
}

inline bool TextBlock::IsBiDiSupportEnabled()
{
    return isBiDiSupportEnabled;
}

}; //end of namespace

#endif // __DAVAENGINE_TEXTBLOCK_H__