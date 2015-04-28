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

#include "DavaEngine.h"

using namespace DAVA;

EditorFontManager::EditorFontManager(QObject* parent)
    : QObject(parent)
{
}
 
void EditorFontManager::LoadLocalizedFonts()
{
    Logger::FrameworkDebug("EditorFontManager::LoadLocalizedFonts");
    
    ClearLocalizedFonts();
    
    FontManager::Instance()->UnregisterFonts();
    for (auto &locale : GetAvailableFonts())
    {
        UIYamlLoader::LoadFonts(GetLocalizedFontsPath(locale));
        for (auto &pair : FontManager::Instance()->GetRegisteredFonts())
        {            
            localizedFonts[locale][pair.second] = SafeRetain(pair.first);
            Logger::FrameworkDebug("EditorFontManager::LoadLocalizedFonts localizedRegisteredFonts[%s][%p] = %s", locale.c_str(), pair.first, pair.second.c_str());
        }
        FontManager::Instance()->UnregisterFonts();
    }
    UIYamlLoader::LoadFonts(GetDefaultFontsPath());
    for (auto &pair : FontManager::Instance()->GetRegisteredFonts())
    {
        defaultFonts[pair.second] = SafeRetain(pair.first);
    }
}

void EditorFontManager::SaveLocalizedFonts()
{
    Logger::FrameworkDebug("EditorFontManager::SaveLocalizedFonts");
    
    Logger::FrameworkDebug("EditorFontManager::SaveLocalizedFonts defaultFontsPath=%s", defaultFontsPath.GetAbsolutePathname().c_str());
    
    if(!FileSystem::Instance()->IsDirectory(defaultFontsPath.GetDirectory()))
    {
        FileSystem::Instance()->CreateDirectory(defaultFontsPath.GetDirectory());
    }
    
    for (auto &locale : LocalizationSystem::Instance()->GetAvailableLocales())
    {       
        //load localized fonts into FontManager
        FontManager::Instance()->UnregisterFonts();
        for (auto pair : localizedFonts.at(locale))
        {
            FontManager::Instance()->RegisterFont(pair.second);
            FontManager::Instance()->SetFontName(pair.second, pair.first);
        }
        
        FontManager::Instance()->PrepareToSaveFonts(true);
        
        const FilePath &localizedFontsPath = GetLocalizedFontsPath(locale);
        Logger::FrameworkDebug("EditorFontManager::SaveLocalizedFonts locale=%s defaultFontsPath=%s", locale.c_str(), localizedFontsPath.GetAbsolutePathname().c_str());
        
        if(!FileSystem::Instance()->IsDirectory(localizedFontsPath.GetDirectory()))
        {
            FileSystem::Instance()->CreateDirectory(localizedFontsPath.GetDirectory());
        }
        UIYamlLoader::SaveFonts(localizedFontsPath);
    }
    
    //load default fonts into FontManager
    Logger::FrameworkDebug("EditorFontManager::SaveLocalizedFonts locale=default");
    
    FontManager::Instance()->RegisterFonts(FontManager::Instance()->GetRegisteredFonts(), defaultFonts);
       
    FontManager::Instance()->PrepareToSaveFonts(true);
    
    UIYamlLoader::SaveFonts(defaultFontsPath);
}

void EditorFontManager::ClearLocalizedFonts()
{
    Logger::FrameworkDebug("EditorFontManager::ClearLocalizedFonts defaultFonts.size()=%d", defaultFonts.size());
    ClearFonts(defaultFonts);
    for (auto &pair : localizedFonts)
    {
        Logger::FrameworkDebug("EditorFontManager::ClearLocalizedFonts locale=%s localeFonts.size()=%d", pair.first.c_str(), pair.second.size());
        ClearFonts(pair.second);
    }
}

void EditorFontManager::ClearFonts(Map<String, Font*>& fonts)
{
    for (auto &pair : fonts)
    {
        SafeRelease(pair.second);
    }
    fonts.clear();
}

DAVA::Vector<DAVA::String> EditorFontManager::GetAvailableFonts() const
{
    Vector<String> availableFonts;
    FileList * fileList = new FileList(defaultFontsPath);
    auto count = fileList->GetCount();
    for (auto i = count, k = 0; k < i; ++k)
    {
        if (fileList->IsDirectory(k) && !fileList->IsNavigationDirectory(k))
        {
            availableFonts.push_back(fileList->GetFilename(k));
        }
    }

    SafeRelease(fileList);
    return availableFonts;
}


void EditorFontManager::SetDefaultFontsPath(const FilePath& path)
{
    Logger::FrameworkDebug("EditorFontManager::SetDefaultFontsPath %s", path.GetAbsolutePathname().c_str());
    defaultFontsPath = path;
}

FilePath EditorFontManager::GetDefaultFontsPath()
{
    return defaultFontsPath + "fonts.yaml";
}

FilePath EditorFontManager::GetLocalizedFontsPath(const String &locale)
{
    return defaultFontsPath + locale + "/fonts.yaml";
}

const Map<String, Font*> &EditorFontManager::GetLocalizedFonts(const String& locale) const
{
    Map<String, Map<String, Font*> >::const_iterator findIt = localizedFonts.find(locale);
    return findIt != localizedFonts.end() ? findIt->second : defaultFonts;
}

Font* EditorFontManager::GetLocalizedFont(const String& fontName, const String& locale) const
{
    const Map<String, Font*> &fonts = GetLocalizedFonts(locale);
    return fonts.at(fontName);
}

String EditorFontManager::UseNewPreset(const String& oldPresetName, Font* font, const String& newPresetName, const String& locale)
{
    DVASSERT(nullptr != font);
    DVASSERT(locale == "default" || localizedFonts.find(locale) != localizedFonts.end());
    auto findFontsIt = localizedFonts.find(locale);
    Map<String, Font*> *fonts = findFontsIt != localizedFonts.end() ? fonts = &findFontsIt->second : &defaultFonts;
       
    if (oldPresetName != newPresetName)
    {
        auto findOriginalIt = fonts->find(oldPresetName);
        if (findOriginalIt != fonts->end())
        {            
            Logger::FrameworkDebug("EditorFontManager::SetLocalizedFont (locale=%s) erase fonts[%s] = %p", locale.c_str(), findOriginalIt->first.c_str(), findOriginalIt->second);
            SafeRelease(findOriginalIt->second);
            fonts->erase(findOriginalIt);
        }
    }
    auto it = fonts->find(newPresetName);
    if (it != fonts->end())
    {
        SafeRelease(it->second);
    }
    (*fonts)[newPresetName] = SafeRetain(font);

    FontManager::Instance()->RegisterFont(font);
    FontManager::Instance()->SetFontName(font, newPresetName);

    Logger::FrameworkDebug("EditorFontManager::SetLocalizedFont (locale=%s) registered font %p with name %s", locale.c_str(), font, newPresetName.c_str());
    emit UpdateFontPreset(QString::fromStdString(oldPresetName), QString::fromStdString(newPresetName));
    return newPresetName;
}

String EditorFontManager::CreateNewPreset(const String& presetName, Font *font, const String& locale)
{
    DVASSERT(nullptr != font);
    DVASSERT(locale == "default" || localizedFonts.find(locale) != localizedFonts.end());
    auto findFontsIt = localizedFonts.find(locale);
    Map<String, Font*> *fonts = findFontsIt != localizedFonts.end() ? fonts = &findFontsIt->second : &defaultFonts;

    // we are creating a copy of font, so generate a valid name for it
    int nameIndex = 0;
    String newFontName = presetName;
    while (fonts->find(newFontName) != fonts->end())
    {
        newFontName = Format("%s_%d", presetName.c_str(), ++nameIndex);
    }
    // add new font
    (*fonts)[newFontName] = SafeRetain(font);
    FontManager::Instance()->RegisterFont(font);
    FontManager::Instance()->SetFontName(font, newFontName);

    Logger::FrameworkDebug("EditorFontManager::SetLocalizedFont (locale=%s) registered font %p with name %s", locale.c_str(), font, newFontName.c_str());
    return newFontName;
}