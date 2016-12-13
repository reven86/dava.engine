#include "Base/Platform.h"
#include "Render/2D/FontManager.h"
#include "Render/2D/FTFont.h"
#include "Render/2D/Private/FTManager.h"
#include "Logger/Logger.h"
#include "Render/2D/Sprite.h"
#include "Core/Core.h"
#include "Utils/StringFormat.h"

#include "Engine/Engine.h"

namespace DAVA
{
FontManager::FontManager()
    : ftmanager(std::make_unique<FTManager>())
{
}

FontManager::~FontManager()
{
    FTFont::ClearCache();
    UnregisterFonts();
}

void FontManager::RegisterFont(Font* font)
{
#if defined(__DAVAENGINE_COREV2__)
    if (!Engine::Instance()->GetOptions()->GetBool("trackFont"))
        return;
#else
    if (!Core::Instance()->GetOptions()->GetBool("trackFont"))
        return;
#endif

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
