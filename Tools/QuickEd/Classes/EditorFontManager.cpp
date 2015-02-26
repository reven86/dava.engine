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



#include "EditorFontManager.h"
#include "StringUtils.h"

#include "UI/UIYamlLoader.h"

#include "Helpers/LocalizationSystemHelper.h"

#include "DavaEngine.h"

using namespace DAVA;

static const String DEFAULT_FONT_PRESET = "Font_default";

static const String DEFAULT_FONT_PATH = "~res:/Fonts/MyriadPro-Regular.otf";

EditorFontManager::EditorFontManager()
{
	defaultFont = NULL;
	baseFont = NULL;
	Init();
}

EditorFontManager::~EditorFontManager()
{
	Reset();
    SafeRelease(baseFont);
}

void EditorFontManager::Init()
{
    Font* font = CreateDefaultFont(DEFAULT_FONT_PATH, DEFAULT_FONT_PRESET);
    SafeRelease(baseFont);
    baseFont = font;
}

void LogRegisteredFonts(Map<Font*, String> &registeredFonts, const String &message)
{
    Map<Font*, String>::const_iterator it = registeredFonts.begin();
    Map<Font*, String>::const_iterator endIt = registeredFonts.end();
    for(; it != endIt; ++it)
    {
        Logger::FrameworkDebug("%s[%p] = %s", message.c_str(), it->first, it->second.c_str());
    }
}

void LogFonts(Map<String, Font*> &fonts, const String &message)
{
    Map<String, Font*>::const_iterator it = fonts.begin();
    Map<String, Font*>::const_iterator endIt = fonts.end();
    for(; it != endIt; ++it)
    {
        Logger::FrameworkDebug("%s[%s] = %p", message.c_str(), it->first.c_str(), it->second);
    }
}

void EditorFontManager::OnProjectLoaded()
{
    // all fonts are loaded and registered in FontManager by now
    
    // OnProjectLoaded is used for backward-compatibility with versions, where fonts were part of ui yaml files
    
    const Map<Font*, String> registeredFonts = FontManager::Instance()->GetRegisteredFonts();
    Map<Font*, String>::const_iterator it = registeredFonts.begin();
    Map<Font*, String>::const_iterator endIt = registeredFonts.end();
    
    Logger::FrameworkDebug("EditorFontManager::OnProjectLoaded registeredFonts.size()=%d", registeredFonts.size());
    
    int32 localesCount = locales.size();
    for(; it != endIt; ++it)
    {
        Font* font = it->first;
        String fontName = it->second;
        
        Map<Font*, String>::const_iterator defaultRegisteredEndIt = defaultRegisteredFonts.end();
        Map<Font*, String>::const_iterator defaultRegisteredFindIt = defaultRegisteredFonts.find(font);
        if(defaultRegisteredFindIt == defaultRegisteredEndIt)
        {
            Logger::FrameworkDebug("EditorFontManager::OnProjectLoaded defaultRegisteredFonts[%p] = %s", font, fontName.c_str());
            defaultRegisteredFonts[font] = fontName;
            
            Map<String, Font*>::const_iterator defaultEndIt = defaultFonts.end();
            Map<String, Font*>::const_iterator defaultFindIt = defaultFonts.find(fontName);
            if(defaultFindIt != defaultEndIt)
            {
                font = defaultFindIt->second;
                Logger::FrameworkDebug("EditorFontManager::OnProjectLoaded defaultRegisteredFonts[%p] = %s", font, fontName.c_str());
                defaultRegisteredFonts[font] = fontName;
            }
            else
            {
                defaultFonts[fontName] = SafeRetain(font);
            }
            
            for(int32 i = 0; i < localesCount; ++i)
            {
                String locale = locales[i];

                Font* localizedFont = font;
                
                Logger::FrameworkDebug("EditorFontManager::OnProjectLoaded localizedRegisteredFonts[%s][%p] = %s", locale.c_str(), localizedFont, fontName.c_str());
                //localizedRegisteredFonts[locale][localizedFont] = fontName;
                
                Map<String, Font*>::const_iterator localizedEndIt = localizedFonts[locale].end();
                Map<String, Font*>::const_iterator localizedFindIt = localizedFonts[locale].find(fontName);
                if(localizedFindIt != localizedEndIt)
                {
                    localizedFont = localizedFindIt->second;
                    Logger::FrameworkDebug("EditorFontManager::OnProjectLoaded localizedRegisteredFonts[%s][%p] = %s", locale.c_str(), localizedFont, fontName.c_str());
                    //localizedRegisteredFonts[locale][localizedFont] = fontName;
                    defaultRegisteredFonts[font] = fontName;
                }
                else
                {
                    localizedFonts[locale][fontName] = SafeRetain(localizedFont);
                }
            }
        }
        else
        {
            if(it->second != defaultRegisteredFindIt->second)
            {
                Logger::Warning("EditorFontManager::OnProjectLoaded default font %p is already registred, but as %s instead of %s", it->first, it->second.c_str(), defaultRegisteredFindIt->second.c_str());
            }
            else
            {
                Logger::FrameworkDebug("EditorFontManager::OnProjectLoaded default font %p is already registred as %s", it->first, it->second.c_str());
            }
        }
    }
}

void EditorFontManager::LoadLocalizedFonts()
{
    Logger::FrameworkDebug("EditorFontManager::LoadLocalizedFonts");
    
    locales.clear();
    ClearLocalizedFonts();
    
    // get locales from fonts.yaml only from existing locale folders
    uint32 languagesCount = LocalizationSystemHelper::GetSupportedLanguagesCount();
    for (uint32 i = 0; i < languagesCount; ++i)
    {
        String locale = LocalizationSystemHelper::GetSupportedLanguageID(i);
        if(FileSystem::Instance()->IsFile(GetLocalizedFontsPath(locale)))
        {
            locales.push_back(locale);
        }
    }

    FontManager::Instance()->UnregisterFonts();
    
    int32 localesCount = locales.size();
    for(int32 i = 0; i < localesCount; ++i)
    {
        String locale = locales[i];
        UIYamlLoader::LoadFonts(GetLocalizedFontsPath(locale));
    
        // save loaded fonts to localizedFonts
        const Map<Font*, String> registeredFonts = FontManager::Instance()->GetRegisteredFonts();
        Map<Font*, String>::const_iterator it = registeredFonts.begin();
        Map<Font*, String>::const_iterator endIt = registeredFonts.end();
        for(; it != endIt; ++it)
        {
            //localizedRegisteredFonts[locale][it->first] = it->second;
            defaultRegisteredFonts[it->first] = it->second;
            
            localizedFonts[locale][it->second] = SafeRetain(it->first);
            
            Logger::FrameworkDebug("EditorFontManager::LoadLocalizedFonts localizedRegisteredFonts[%s][%p] = %s", locale.c_str(), it->first, it->second.c_str());
        }
        
        FontManager::Instance()->UnregisterFonts();
    }

    {
        UIYamlLoader::LoadFonts(GetDefaultFontsPath());
        
        const Map<Font*, String> registeredFonts = FontManager::Instance()->GetRegisteredFonts();
        Map<Font*, String>::const_iterator it = registeredFonts.begin();
        Map<Font*, String>::const_iterator endIt = registeredFonts.end();
        for(; it != endIt; ++it)
        {
            defaultRegisteredFonts[it->first] = it->second;
            defaultFonts[it->second] = SafeRetain(it->first);
            Logger::FrameworkDebug("EditorFontManager::LoadLocalizedFonts defaultRegisteredFonts[%p] = %s", it->first, it->second.c_str());
        }
    }
    
    //if font DEFAULT_FONT_PRESET exists in saved fonts, replace default font by saved one
    Init();
}

void EditorFontManager::SaveLocalizedFonts()
{
    Logger::FrameworkDebug("EditorFontManager::SaveLocalizedFonts");
    
    const FilePath &defaultFontsPath = EditorFontManager::Instance()->GetDefaultFontsPath();
    Logger::FrameworkDebug("EditorFontManager::SaveLocalizedFonts defaultFontsPath=%s", defaultFontsPath.GetAbsolutePathname().c_str());
    
    if(!FileSystem::Instance()->IsDirectory(defaultFontsPath.GetDirectory()))
    {
        FileSystem::Instance()->CreateDirectory(defaultFontsPath.GetDirectory());
    }
    
    int32 localesCount = locales.size();
    for(int32 i = 0; i < localesCount; ++i)
    {
        String locale = locales[i];
        
        //load localized fonts into FontManager
        
        FontManager::Instance()->UnregisterFonts();
        Map<String, Font*>::const_iterator it = localizedFonts[locale].begin();
        Map<String, Font*>::const_iterator endIt = localizedFonts[locale].end();
        for(; it != endIt; ++it)
        {
            FontManager::Instance()->RegisterFont(it->second);
            FontManager::Instance()->SetFontName(it->second, it->first);
        }
        
        LogFonts(localizedFonts[locale], Format("EditorFontManager::SaveLocalizedFonts localizedFonts[%s]", locale.c_str()));

        FontManager::Instance()->PrepareToSaveFonts(true);
        
        const FilePath &localizedFontsPath = EditorFontManager::Instance()->GetLocalizedFontsPath(locale);
        Logger::FrameworkDebug("EditorFontManager::SaveLocalizedFonts locale=%s defaultFontsPath=%s", locale.c_str(), localizedFontsPath.GetAbsolutePathname().c_str());
        
        if(!FileSystem::Instance()->IsDirectory(localizedFontsPath.GetDirectory()))
        {
            FileSystem::Instance()->CreateDirectory(localizedFontsPath.GetDirectory());
        }
        UIYamlLoader::SaveFonts(localizedFontsPath);
    }
    
    //load default fonts into FontManager
    Logger::FrameworkDebug("EditorFontManager::SaveLocalizedFonts locale=default");
    
    FontManager::Instance()->RegisterFonts(defaultRegisteredFonts, defaultFonts);
    
    LogFonts(defaultFonts, "EditorFontManager::SaveLocalizedFonts defaultFonts");
    LogRegisteredFonts(defaultRegisteredFonts, "EditorFontManager::SaveLocalizedFonts defaultRegisteredFonts");
    
    FontManager::Instance()->PrepareToSaveFonts(true);
    
    UIYamlLoader::SaveFonts(defaultFontsPath);
}

void EditorFontManager::ClearLocalizedFonts()
{
    defaultRegisteredFonts.clear();

    Logger::FrameworkDebug("EditorFontManager::ClearLocalizedFonts defaultFonts.size()=%d", defaultFonts.size());
    ClearFonts(defaultFonts);
    
    Map<String, Map<String, Font*> >::iterator fontsIt = localizedFonts.begin();
    Map<String, Map<String, Font*> >::const_iterator fontsEndIt = localizedFonts.end();
    
    for(; fontsIt != fontsEndIt; ++fontsIt)
    {
        Logger::FrameworkDebug("EditorFontManager::ClearLocalizedFonts locale=%s localeFonts.size()=%d", fontsIt->first.c_str(), fontsIt->second.size());
        ClearFonts(fontsIt->second);
    }
}

void EditorFontManager::ClearFonts(Map<String, Font*>& fonts)
{
    Map<String, Font*>::iterator it = fonts.begin();
    Map<String, Font*>::const_iterator endIt = fonts.end();
    for(; it != endIt; ++it)
    {
        SafeRelease(it->second);
    }
    fonts.clear();
}

void EditorFontManager::Reset()
{
	SafeRelease(defaultFont);

    FontManager::Instance()->UnregisterFonts();
    
    ClearLocalizedFonts();
    
    ResetLocalizedFontsPath();
    
    RegisterDefaultFont(baseFont);
}

void EditorFontManager::RegisterDefaultFont(Font* font)
{
    Map<String, Map<String, Font*> >::iterator it = localizedFonts.begin();
    Map<String, Map<String, Font*> >::iterator endIt = localizedFonts.end();
    
    for (; it != endIt; ++it) {
        SetLocalizedFont(DEFAULT_FONT_PRESET, font, DEFAULT_FONT_PRESET, true, it->first);
    }
    SetLocalizedFont(DEFAULT_FONT_PRESET, font, DEFAULT_FONT_PRESET, true, "default");
}

Font* EditorFontManager::CreateDefaultFont(const String& fontPath, const String& fontName)
{
	Font* font = SafeRetain(GetLocalizedFont(fontName, "default"));
	if (font)
		return font;
	
    static const float defaultFontSize = 12.0f;
	font = FTFont::Create(fontPath);
	if (font)
	{
		font->SetSize(defaultFontSize);
		
        //TODO: remove default font or create also default localized font
//		fonts[fontName] = font;
//        FontManager::Instance()->RegisterFont(font);
        RegisterDefaultFont(font);
        
        //If font was successfully loaded - emit the signal
        emit FontLoaded();
	}
	
	return font;
}

Font* EditorFontManager::GetDefaultFont() const
{
	return defaultFont ? defaultFont : baseFont;
}

void EditorFontManager::SetDefaultFont(Font *font)
{
    SafeRelease(defaultFont);
    Font* localizedDefaultFont = GetLocalizedFont(font);
    if(localizedDefaultFont)
    {
        defaultFont = localizedDefaultFont->Clone();
    }
    else
    {
        defaultFont = font->Clone();
    }
    RegisterDefaultFont(defaultFont);
}

void EditorFontManager::ResetDefaultFont()
{
	SafeRelease(defaultFont);
}

void EditorFontManager::SetProjectDataPath(const FilePath& path)
{
    Logger::FrameworkDebug("EditorFontManager::SetProjectDataPath %s", path.GetAbsolutePathname().c_str());
    projectDataPath = path;
}

void EditorFontManager::SetDefaultFontsPath(const FilePath& path)
{
    Logger::FrameworkDebug("EditorFontManager::SetDefaultFontsPath %s", path.GetAbsolutePathname().c_str());
    defaultFontsPath = path;
}

const FilePath& EditorFontManager::GetDefaultFontsPath()
{
    return defaultFontsPath;
}

String EditorFontManager::GetDefaultFontsFrameworkPath()
{
    return "~res:/" + defaultFontsPath.GetRelativePathname(projectDataPath);
}

FilePath EditorFontManager::GetLocalizedFontsPath(const String &locale)
{
    FilePath localizedFontsPath(defaultFontsPath);
    //localizedFontsPath.ReplaceDirectory(defaultFontsPath.GetDirectory() + (LocalizationSystem::Instance()->GetCurrentLocale() + "/"));
    localizedFontsPath.ReplaceDirectory(defaultFontsPath.GetDirectory() + (locale + "/"));
    
    return localizedFontsPath;
}

void EditorFontManager::ResetLocalizedFontsPath()
{
    // TODO: reset localized fonts path (unload localized fonts)
    defaultFontsPath = FilePath();
}

String EditorFontManager::GetFontDisplayName(DAVA::Font *font)
{
    if(!font)
    {
        return "NULL";
    }
    
    String fontDisplayName;

    Font::eFontType fontType = font->GetFontType();
    
    switch(fontType)
    {
        case Font::TYPE_FT:
        {
            FTFont *ftFont = static_cast<FTFont*>(font);
            //Set pushbutton widget text
            fontDisplayName = ftFont->GetFontPath().GetFrameworkPath();
            break;
        }
        case Font::TYPE_GRAPHICAL:
        {
            GraphicsFont *gFont = static_cast<GraphicsFont*>(font);
            //Put into result string font definition and font sprite path
            Sprite *fontSprite = gFont->GetFontSprite();
            if (fontSprite) //If no sprite available - quit
            {
                //Get font definition and sprite relative path
                String fontDefinitionName = gFont->GetFontDefinitionName().GetFrameworkPath();
                String fontSpriteName = fontSprite->GetRelativePathname().GetFrameworkPath();
                //Set push button widget text - for grapics font it contains font definition and sprite names
                fontDisplayName = Format("%s\n%s", fontDefinitionName.c_str(), fontSpriteName.c_str());
            }
            break;
        }
        default:
        {
            //Do nothing if we can't determine font type
            //TODO: do we need to return a name of preset or maybe "unknown"?
            return "unknown";
        }
    }

    return fontDisplayName;
}

const Map<String, Font*> &EditorFontManager::GetLocalizedFonts(const String& locale) const
{
    Map<String, Map<String, Font*> >::const_iterator findIt = localizedFonts.find(locale);
    if(findIt != localizedFonts.end())
    {
        //Logger::FrameworkDebug("EditorFontManager::GetLocalizedFonts (locale=%s) found %d fonts", locale.c_str(), findIt->second.size());
        return findIt->second;
    }
    //Logger::FrameworkDebug("EditorFontManager::GetLocalizedFonts (locale=%s) not found, returning default %d fonts", locale.c_str(), defaultFonts.size());
    return defaultFonts;
}


Font* EditorFontManager::GetLocalizedFont(Font* font) const
{
    return GetLocalizedFont(GetLocalizedFontName(font), LocalizationSystem::Instance()->GetCurrentLocale());
}

String EditorFontManager::GetLocalizedFontName(Font* font) const
{
    String fontName;
    
    bool isFound = false;
    
    //TODO: remove defaultFonts
    if(!isFound)
    {
        const Map<Font*, String> *fonts = &defaultRegisteredFonts;
        Map<Font*, String>::const_iterator findIt = fonts->find(font);
        Map<Font*, String>::const_iterator endIt = fonts->end();
        
        if(findIt != endIt)
        {
            fontName = findIt->second;
            
            //Logger::FrameworkDebug("EditorFontManager::GetLocalizedFontName (locale=%s) font %p (%s) found in %d default fonts", locale.c_str(), font, fontName.c_str(), fonts->size());
            
            isFound = true;
        }
    }
    
    return fontName;
}

Font* EditorFontManager::GetLocalizedFont(const String& fontName, const String& locale) const
{
    Font* font = NULL;
    const Map<String, Font*> &fonts = GetLocalizedFonts(locale);
    Map<String, Font*>::const_iterator findIt = fonts.find(fontName);
    Map<String, Font*>::const_iterator endIt = fonts.end();
    if(findIt != endIt)
    {
        font = findIt->second;
    }
    return font;
}

String EditorFontManager::SetLocalizedFont(const String& fontOriginalName, Font* font, const String& fontName, bool replaceExisting, const String& locale)
{
    String newFontName = fontName;
    if(!font)
    {
        Logger::Warning("EditorFontManager::SetLocalizedFont (locale=%s) attempted to set font=NULL for %s (former %s)", locale.c_str(), fontName.c_str(), fontOriginalName.c_str());
        return newFontName;
    }
    Map<String, Font*> *fonts = NULL;
    Map<Font*, String> *registeredFonts = NULL;
    
    Map<String, Map<String, Font*> >::iterator findFontsIt = localizedFonts.find(locale);
    
    if(findFontsIt != localizedFonts.end())
    {
        fonts = &findFontsIt->second;
    }
    else if(locale == "default")
    {
        fonts = &defaultFonts;
    }
    
    registeredFonts = &defaultRegisteredFonts;
    
    if(!fonts)
    {
        Logger::Error("EditorFontManager::SetLocalizedFont (locale=%s) fonts=NULL", locale.c_str());
        return newFontName;
    }
    
    if(!registeredFonts)
    {
        Logger::Error("EditorFontManager::SetLocalizedFont (locale=%s) registeredFonts=NULL", locale.c_str());
        return newFontName;
    }
    
    bool isSameFontName = (fontOriginalName == newFontName);
    
    Map<String, Font*>::const_iterator endIt = fonts->end();
    
    Map<String, Font*>::iterator findIt = fonts->find(newFontName);
    
    if(replaceExisting)
    {
        Map<String, Font*>::iterator findOriginalIt = fonts->find(fontOriginalName);
        if(findOriginalIt != endIt)
        {
            if(!isSameFontName)
            {
                Logger::FrameworkDebug("EditorFontManager::SetLocalizedFont (locale=%s) erase fonts[%s] = %p", locale.c_str(), findOriginalIt->first.c_str(), findOriginalIt->second);
            }
            
            
            if(isSameFontName)
            {
                if(findOriginalIt->second != font)
                {
                    SafeRelease(findOriginalIt->second);
                    findOriginalIt->second = SafeRetain(font);
                }
            }
            else
            {
                SafeRelease(findOriginalIt->second);
                fonts->erase(findOriginalIt);
            }
        }
        else if(isSameFontName)
        {
            // need to replace existing, but it does not exist (this can happen on default font) - add new font
            Logger::FrameworkDebug("EditorFontManager::SetLocalizedFont (locale=%s) fonts[%s] = %p", locale.c_str(), newFontName.c_str(), font);
            (*fonts)[newFontName] = SafeRetain(font);
        }
        
        if(!isSameFontName)
        {
            // rename in registered fonts
            Map<Font*, String>::iterator registeredIt = registeredFonts->begin();
            Map<Font*, String>::const_iterator registeredEndIt = registeredFonts->end();
            for(; registeredIt != registeredEndIt; ++registeredIt)
            {
                if(registeredIt->second == fontOriginalName)
                {
                    registeredIt->second = newFontName;
                }
            }
            
            // rename existing font
            if(findIt != fonts->end())
            {
                Logger::Warning("EditorFontManager::SetLocalizedFont (locale=%s) name %s is already used by font %p (will be replaced by %p)",  locale.c_str(), findIt->first.c_str(), findIt->second, font);
                if(findIt->second != font)
                {
                    SafeRelease(findIt->second);
                    findIt->second = SafeRetain(font);
                }
            }
            else
            {
                // add new font
                Logger::FrameworkDebug("EditorFontManager::SetLocalizedFont (locale=%s) fonts[%s] = %p", locale.c_str(), newFontName.c_str(), font);
                (*fonts)[newFontName] = SafeRetain(font);
            }
        }
    }
    else
    {
        // we are creating a copy of font, so generate a valid name for it
        if(findIt != fonts->end())
        {
            int32 nameIndex = 0;
            while(fonts->find(newFontName) != fonts->end())
            {
                newFontName = Format("%s_%d", fontName.c_str(), ++nameIndex);
            }
        }
        // add new font
        Logger::FrameworkDebug("EditorFontManager::SetLocalizedFont (locale=%s) fonts[%s] = %p", locale.c_str(), newFontName.c_str(), font);
        (*fonts)[newFontName] = SafeRetain(font);
    }
    
    (*registeredFonts)[font] = newFontName;
    
    //TODO: check if it is really needed to register fonts for all locales or only current or default locale can be used
    //if(locale == "default")
    {
        FontManager::Instance()->RegisterFont(font);
        FontManager::Instance()->SetFontName(font, newFontName);
    }
    Logger::FrameworkDebug("EditorFontManager::SetLocalizedFont (locale=%s) registered font %p with name %s", locale.c_str(), font, newFontName.c_str());
    
    return newFontName;
}

EditorFontManager::DefaultFontPath EditorFontManager::GetDefaultFontPath()
{
	FilePath defFontPath;
	FilePath defFontSpritePath;

	if (defaultFont)
	{
		Font::eFontType fontType = defaultFont->GetFontType();		
        switch (fontType)
        {
            case Font::TYPE_FT:
            {
                FTFont *ftFont = static_cast<FTFont*>(defaultFont);
				FilePath ftFontPath = ftFont->GetFontPath();
                defFontPath = ftFontPath;
                break;
            }
            case Font::TYPE_GRAPHICAL:
            {
                GraphicsFont *gFont = static_cast<GraphicsFont*>(defaultFont);
                // Try to get font sprite
                Sprite *fontSprite = gFont->GetFontSprite();
				// Save font only if sprite is available
                if (fontSprite)
                {
					// Set font definition and sprite relative path
					defFontPath = gFont->GetFontDefinitionName();
					defFontSpritePath = fontSprite->GetRelativePathname();
                }
				break;
            }
        }	
	}
	
	return DefaultFontPath(defFontPath, defFontSpritePath);
}

void EditorFontManager::InitDefaultFontFromPath(const EditorFontManager::DefaultFontPath& defaultFontPath)
{
	FilePath fontPath = defaultFontPath.fontPath;
	FilePath fontSpritePath = defaultFontPath.fontSpritePath;
	Font* loadedFont = NULL;
	// Create font from loaded paths
	if (!fontPath.IsEmpty())
	{
		// Grpahics font
		if (!fontSpritePath.IsEmpty())
		{
            fontSpritePath.TruncateExtension();
			loadedFont = GraphicsFont::Create(fontPath, fontSpritePath);
		}
		else // True type font
		{
			loadedFont = FTFont::Create(fontPath);
		}
	}
	// Set default font only if font was really created
	if (loadedFont)
	{
		// Reset default font
		SafeRelease(defaultFont);
		defaultFont = loadedFont;
	}	
}

QString EditorFontManager::GetDefaultFontName() const
{		
	if (defaultFont)
	{
		Font::eFontType fontType = defaultFont->GetFontType();
		switch (fontType)
		{
			case Font::TYPE_FT:
			{
				FTFont *ftFont = static_cast<FTFont*>(defaultFont);
				return QString::fromStdString(ftFont->GetFontPath().GetAbsolutePathname());
			}
			case Font::TYPE_GRAPHICAL:
			{
				GraphicsFont *gFont = static_cast<GraphicsFont*>(defaultFont);
				return QString::fromStdString(gFont->GetFontDefinitionName().GetAbsolutePathname());
			}
		}
	}
	return QString::fromStdString(DEFAULT_FONT_PATH);
}
