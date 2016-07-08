#include "Base/Platform.h"

#ifdef __DAVAENGINE_WIN_UAP__
#define generic GenericFromFreeTypeLibrary
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#endif

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifdef __DAVAENGINE_WIN_UAP__
#undef generic
#endif

#include "Render/2D/FontManager.h"
#include "Render/2D/FTFont.h"
#include "Logger/Logger.h"
#include "Render/2D/Sprite.h"
#include "Core/Core.h"
#include "Utils/StringFormat.h"

namespace DAVA
{
FontManager::FontManager()
{
    FT_Error error = FT_Init_FreeType(&library);
    if (error)
    {
        Logger::Error("FontManager FT_Init_FreeType failed");
    }
}

FontManager::~FontManager()
{
    FTFont::ClearCache();

    UnregisterFonts();

    FT_Error error = FT_Done_FreeType(library);
    if (error)
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

void FontManager::UnregisterFont(Font* font)
{
    registeredFonts.erase(font);
}

void FontManager::RegisterFonts(const Map<String, Font*>& fonts)
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
    if (findIt != fontMap.end())
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

Font* FontManager::GetFont(const String& name) const
{
    auto it = fontMap.find(name);
    if (it != fontMap.end())
    {
        return it->second;
    }
    return nullptr;
}

String FontManager::GetFontName(Font* font) const
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
