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


#include "Platform/PlatformDetection.h"

#ifdef __DAVAENGINE_WINDOWS_STORE__
#define generic GenericFromFreeTypeLibrary
#endif

#include <ft2build.h>
#include <freetype/ftglyph.h>
//#include "ftglyph.h"
#include FT_FREETYPE_H

#ifdef __DAVAENGINE_WINDOWS_STORE__
#undef generic
#endif

#include "Render/2D/FontManager.h"
#include "Render/2D/FTFont.h"
#include "FileSystem/Logger.h"
#include "Render/2D/Sprite.h"
#include "Core/Core.h"
#include "Utils/StringFormat.h"

namespace DAVA
{
	
FontManager::FontManager()
{
	FT_Error error = FT_Init_FreeType(&library);
	if(error)
	{
		Logger::Error("FontManager FT_Init_FreeType failed");
	}
}
	
FontManager::~FontManager()
{
	FTFont::ClearCache();

	UnregisterFonts();

	FT_Error error = FT_Done_FreeType(library);
	if(error)
	{
		Logger::Error("FontManager FT_Done_FreeType failed");
	}
}
	
void FontManager::RegisterFont(Font* font)
{
	if (!Core::Instance()->GetOptions()->GetBool("trackFont"))
		return;

	if (registeredFonts.find(font) != registeredFonts.end())
		return;
	
	registeredFonts[font] = "";
}

void FontManager::UnregisterFont(Font *font)
{
	registeredFonts.erase(font);
}
    
void FontManager::RegisterFonts(const Map<Font*, String>& _registeredFonts, const Map<String, Font*> &fonts)
{
    UnregisterFonts();
    
    registeredFonts = _registeredFonts;
    
    Map<String, Font*>::const_iterator it = fonts.begin();
    Map<String, Font*>::const_iterator endIt = fonts.end();
    Map<String, Font*>::const_iterator endMapIt = fontMap.end();
    for(; it != endIt; ++it)
    {
        Map<String, Font*>::iterator findIt = fontMap.find(it->first);
        if(findIt != endMapIt)
        {
            if(findIt->second != it->second)
            {
                SafeRelease(findIt->second);
                findIt->second = SafeRetain(it->second);
            }
        }
        else
        {
            fontMap[it->first] = SafeRetain(it->second);
        }
    }
}
    
void FontManager::UnregisterFonts()
{
    registeredFonts.clear();
    
    //TODO: remove fontMap legacy from UIYamlLoader?
    for (Map<String, Font *>::iterator t = fontMap.begin(); t != fontMap.end(); ++t)
	{
		SafeRelease(t->second);
	}
	fontMap.clear();
    //
    
    Clear();
}
	
void FontManager::SetFontName(Font* font, const String& name)
{
    Map<String, Font*>::iterator findIt = fontMap.find(name);
    if(findIt != fontMap.end())
    {
        if(findIt->second != font)
        {
            SafeRelease(findIt->second);
            findIt->second = SafeRetain(font);
        }
    }
    else
    {
        fontMap[name] = SafeRetain(font);
    }
    
	if (registeredFonts.find(font) == registeredFonts.end())
		return;
    
    //TODO: check if font hash is still needed for anything
    registeredFonts[font] = name;
    //Logger::Debug("FontManager::SetFontName %x %s", font, name.c_str());
    
	// The names of all fonts should coincide with their hashed names (see DF-2316).
	//String fontHashName = GetFontHashName(font);
	//registeredFonts[font] = fontHashName;
}
	
Font* FontManager::GetFont(const String &name)
{
	Map<String, Font*>::iterator it = fontMap.find(name);
	if (it != fontMap.end())
	{
		Font * font = it->second;
		return font;
	}
	return 0;
}
    
String FontManager::GetFontName(Font *font)
{
	REGISTERED_FONTS::iterator fontIter = registeredFonts.find(font);
	if (fontIter == registeredFonts.end())
    {
        Logger::Warning("FontManager::GetFontName %x not found in registeredFonts", font);
		return "";
    }
	
	for (FONTS_NAME::iterator iter = fontsName.begin();
		 iter != fontsName.end();
		 ++iter)
	{
		FONT_NAME* fontName = (*iter);
		
		TRACKED_FONTS::iterator fontNameIter = fontName->fonts.find(font);
		if (fontNameIter == fontName->fonts.end())
			continue;
		
		trackedFonts.insert(font);
		
		if (!fontName->name.empty())
			return fontName->name;
		
		String name = fontIter->second;
		if (name.empty())
		{
			// YuriCoder, 2013/10/18. Font name HAVE TO BE unique, otherwise it might not be saved correctly.
			name = GetFontHashName(font);
		}

		fontName->name = name;
		return name;
	}
	return "";
}

void FontManager::PrepareToSaveFonts(bool saveAllFonts)
{
    Clear();
	fontsName.clear();
	trackedFonts.clear();

	for (REGISTERED_FONTS::iterator iter = registeredFonts.begin();
		 iter != registeredFonts.end();
		 ++iter)
	{
		Font* font = iter->first;
		
        if(saveAllFonts)
        {
            trackedFonts.insert(font);
        }
        else
        {
            bool fontAdded = false;
            for (FONT_NAME* fontName : fontsName)
            {
                Font* firstFont = (*fontName->fonts.begin());
                if (firstFont && firstFont->IsEqual(font))
                {
                    fontName->fonts.insert(font);
                    fontAdded = true;
                    break;
                }
            }
            
            if (fontAdded)
                continue;
        }
        
        FONT_NAME* fontName = new FONT_NAME();
        fontName->fonts.insert(font);
        fontsName.insert(fontName);
	}
}
	
void FontManager::Clear()
{
	for (FONTS_NAME::iterator iter = fontsName.begin();
		 iter != fontsName.end();
		 ++iter)
	{
		FONT_NAME* fontName = (*iter);
		SAFE_DELETE(fontName);
	}
	fontsName.clear();
}

const FontManager::TRACKED_FONTS& FontManager::GetTrackedFont() const
{
	return trackedFonts;
}

const Map<Font*, String>& FontManager::GetRegisteredFonts() const
{
    return registeredFonts;
}
    
String FontManager::GetFontHashName(Font* font)
{
	return Format("Font_%X", font->GetHashCode());
}
    
const Map<String, Font*>& FontManager::GetFontMap() const
{
    return fontMap;
}

};

