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



#include "EditorFontSystem.h"
#include "StringUtils.h"

#include "UI/UIYamlLoader.h"

#include "DavaEngine.h"

using namespace DAVA;

EditorFontSystem::EditorFontSystem(QObject* parent)
    : QObject(parent)
{
}

EditorFontSystem::~EditorFontSystem()
{
    ClearAllFonts();
}

Font* EditorFontSystem::GetFont(const String &presetName, const String &locale) const
{
    const auto &fonts = locale == "default" ? defaultFonts : localizedFonts.at(locale);
    auto it = fonts.find(presetName);
    return it != fonts.end() ? it->second : nullptr;
}

void EditorFontSystem::SetFont(const String &presetName, const String &locale, Font *font)
{
    auto *fonts = locale == "default" ? &defaultFonts : &localizedFonts.at(locale);
    auto it = fonts->find(presetName);
    DVASSERT(it != fonts->end());
    auto oldFont = it->second;
    FontManager::Instance()->UnregisterFont(oldFont);
    oldFont->Release();
    it->second = SafeRetain(font);

    FontManager::Instance()->RegisterFont(font);
    FontManager::Instance()->SetFontName(font, presetName);
    font->Release();
}
 
void EditorFontSystem::LoadLocalizedFonts()
{
    ClearAllFonts();
    
    FontManager::Instance()->UnregisterFonts();
    for (auto &locale : availableFontLocales)
    {
        UIYamlLoader::LoadFonts(GetLocalizedFontsPath(locale.toStdString()));
        for (auto &pair : FontManager::Instance()->GetRegisteredFonts())
        {            
            localizedFonts[locale.toStdString()][pair.second] = SafeRetain(pair.first);
        }
        FontManager::Instance()->UnregisterFonts();
    }
    UIYamlLoader::LoadFonts(GetDefaultFontsPath());
    for (auto &pair : FontManager::Instance()->GetRegisteredFonts())
    {
        defaultPresetNames.append(QString::fromStdString(pair.second));
        defaultFonts[pair.second] = SafeRetain(pair.first);
    }
    //now check thhat all font are correct
    for (auto &pair : defaultFonts)
    {
        for (auto &locale : availableFontLocales)
        {
            const auto &localizedMap = localizedFonts.at(locale.toStdString());
            DVASSERT(localizedMap.find(pair.first) != localizedMap.end());
        }
    }
}

void EditorFontSystem::SaveLocalizedFonts()
{   
    if(!FileSystem::Instance()->IsDirectory(defaultFontsPath.GetDirectory()))
    {
        FileSystem::Instance()->CreateDirectory(defaultFontsPath.GetDirectory());
    }
    for (auto &localizedFontsIt : localizedFonts)
    {       
        //load localized fonts into FontManager
        FontManager::Instance()->UnregisterFonts();
        for (auto &pair : localizedFontsIt.second)
        {
            FontManager::Instance()->RegisterFont(pair.second);
            FontManager::Instance()->SetFontName(pair.second, pair.first);
        }
        FontManager::Instance()->PrepareToSaveFonts(true);
        const FilePath &localizedFontsPath = GetLocalizedFontsPath(localizedFontsIt.first);
        if(!FileSystem::Instance()->IsDirectory(localizedFontsPath.GetDirectory()))
        {
            FileSystem::Instance()->CreateDirectory(localizedFontsPath.GetDirectory());
        }
        UIYamlLoader::SaveFonts(localizedFontsPath);
    }
    FontManager::Instance()->UnregisterFonts();
    for (auto &pair : defaultFonts)
    {
        FontManager::Instance()->RegisterFont(pair.second);
        FontManager::Instance()->SetFontName(pair.second, pair.first);
    }
    FontManager::Instance()->PrepareToSaveFonts(true);
    UIYamlLoader::SaveFonts(defaultFontsPath + "fonts.yaml");
}

void EditorFontSystem::ClearAllFonts()
{
    ClearFonts(defaultFonts);
    for (auto &map : localizedFonts)
    {
        ClearFonts(map.second);
    }
}

void EditorFontSystem::ClearFonts(Map<String, Font*>& fonts)
{
    for (auto &pair : fonts)
    {
        SafeRelease(pair.second);
    }
    fonts.clear();
}

void EditorFontSystem::RemoveFont(Map<String, Font*>* fonts, const String& fontName)
{
    if (fonts->find(fontName) != fonts->end())
    {
        auto findOriginalIt = fonts->find(fontName);
        SafeRelease(findOriginalIt->second);
        fonts->erase(findOriginalIt);
    }
}

void EditorFontSystem::SetDefaultFontsPath(const FilePath& path)
{
    defaultFontsPath = path;
    availableFontLocales.clear();
    FileList * fileList = new FileList(defaultFontsPath);
    for (auto count = fileList->GetCount(), i = 0; i < count; ++i)
    {
        if (fileList->IsDirectory(i) && !fileList->IsNavigationDirectory(i))
        {
            availableFontLocales.push_back(QString::fromStdString(fileList->GetFilename(i)));
        }
    }

    SafeRelease(fileList);
}

FilePath EditorFontSystem::GetDefaultFontsPath()
{
    return defaultFontsPath + "fonts.yaml";
}

FilePath EditorFontSystem::GetLocalizedFontsPath(const String &locale)
{
    return defaultFontsPath + locale + "/fonts.yaml";
}

void EditorFontSystem::UseNewPreset(const String& originalPresetName, const String& newPresetName)
{
    Font *font = defaultFonts.at(originalPresetName);
    DVASSERT(nullptr != font);
    if (defaultFonts.find(newPresetName) == defaultFonts.end())
    {
        CreateNewPreset(originalPresetName, newPresetName);
    }
    //TODO : iterate file system to replace preset, remove old preset from system
}

void EditorFontSystem::CreateNewPreset(const String& originalPresetName, const String& newPresetName)
{
    Font *font = defaultFonts.at(originalPresetName);
    DVASSERT(nullptr != font);

    defaultFonts[newPresetName] = defaultFonts.at(originalPresetName)->Clone();

    for (auto &localizedFontsPairs : localizedFonts)
    {
        localizedFontsPairs.second[newPresetName] = localizedFonts.at(localizedFontsPairs.first).at(originalPresetName)->Clone();
    }
    defaultPresetNames.append(QString::fromStdString(newPresetName));
}