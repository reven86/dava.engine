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


#include "Base/Platform.h"

#ifdef __DAVAENGINE_WIN_UAP__
#define generic GenericFromFreeTypeLibrary
#endif

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef __DAVAENGINE_WIN_UAP__
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

    if (registeredFonts.find(font) == registeredFonts.end())
    {
        registeredFonts.insert({ font, DAVA::String() });
    }
}

void FontManager::UnregisterFont(Font *font)
{
	registeredFonts.erase(font);
}
    
void FontManager::RegisterFonts(const Map<String, Font*> &fonts)
{
    UnregisterFonts();
        
    for (auto it = fonts.begin(); it != fonts.end(); ++it)
    {
        auto findIt = fontMap.find(it->first);
        if (findIt != fontMap.end())
        {
            SafeRelease(findIt->second);
        }

        fontMap[it->first] = SafeRetain(it->second);
        registeredFonts[it->second] = it->first;
    }
}
    
void FontManager::UnregisterFonts()
{
    registeredFonts.clear();
    
    for (auto it = fontMap.begin(); it != fontMap.end(); ++it)
	{
		SafeRelease(it->second);
	}
	fontMap.clear();
}
	
void FontManager::SetFontName(Font* font, const String& name)
{
    auto findIt = fontMap.find(name);
    if(findIt != fontMap.end())
    {
        SafeRelease(findIt->second);
    }
    fontMap[name] = SafeRetain(font);

    // check if font already registered
    if (registeredFonts.find(font) != registeredFonts.end())
    {
        registeredFonts[font] = name;
    }
}
	
Font* FontManager::GetFont(const String &name) const
{
	auto it = fontMap.find(name);
	if (it != fontMap.end())
	{
		return it->second;
	}
	return nullptr;
}
    
String FontManager::GetFontName(Font *font) const
{
	auto fontIter = registeredFonts.find(font);
	if (fontIter == registeredFonts.end())
    {
        Logger::Warning("FontManager::GetFontName %x not found in registeredFonts", font);
		return String();
    }
    else 
    {
        return fontIter->second;
    }
}

const Map<Font*, String>& FontManager::GetRegisteredFonts() const
{
    return registeredFonts;
}
    
String FontManager::GetFontHashName(Font* font) const
{
	return Format("Font_%X", font->GetHashCode());
}
    
const Map<String, Font*>& FontManager::GetFontMap() const
{
    return fontMap;
}

};

