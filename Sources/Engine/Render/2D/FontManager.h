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


#ifndef __DAVAENGINE_FONTMANAGER_H__
#define __DAVAENGINE_FONTMANAGER_H__

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "Base/BaseMath.h"

struct FT_LibraryRec_;
typedef struct FT_LibraryRec_  *FT_Library;

namespace DAVA
{
	
class Font;
class FTFont;
class FTInternalFont;
class Sprite;
class UIStaticText;
	
class FontManager : public Singleton<FontManager>
{
	FT_Library		library;

	
public:
	FontManager();
	virtual ~FontManager();
	
	FT_Library		GetFTLibrary() { return library; }
	
	/**
	 \brief Register font.
	 */
	void RegisterFont(Font* font);
	/**
	 \brief Unregister font.
	 */
	void UnregisterFont(Font *font);
    /**
	 \brief Register all fonts.
	 */
    void RegisterFonts(const Map<Font*, String>& _registeredFonts, const Map<String, Font*> &fonts);
    /**
	 \brief Unregister all fonts.
	 */
    void UnregisterFonts();
    
	/**
	 \brief Set font name.
	 */
	void SetFontName(Font* font, const String& name);

	/**
	 \brief Get traked font name. Add font to track list.
	 */
	String GetFontName(Font *font);
    
    /**
	 \brief Get font by name.
	 */
	Font* GetFont(const String &name);
	
	/**
	 \brief Reset tracked fonts added by GetFontName().
	 */
	void PrepareToSaveFonts(bool saveAllFonts = false);
    
    typedef Set<Font*> TRACKED_FONTS;
	/**
	 \brief Get tracked fonts added by GetFontName().
	 */
	const TRACKED_FONTS& GetTrackedFont() const;
	
    /**
	 \brief Get registered fonts.
	 */
	const Map<Font*, String>& GetRegisteredFonts() const;
	
    
private:
	void Clear();
	String GetFontHashName(Font* font);

private:
	typedef Map<Font*, String> REGISTERED_FONTS;
	REGISTERED_FONTS registeredFonts;

	struct FONT_NAME
	{
		String name;
		TRACKED_FONTS fonts;
	};
	typedef Set<FONT_NAME*> FONTS_NAME;
	FONTS_NAME fontsName;
	
	TRACKED_FONTS trackedFonts;
	
    //TODO: remove this extra map (UIYamlLoader legacy)
    Map<String, Font*> fontMap;
};
	
};


#endif //__DAVAENGINE_FONTMANAGER_H__

