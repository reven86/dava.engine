#include "EditorFontSystem.h"
#include "QtTools/Utils/Utils.h"

#include "UI/UIYamlLoader.h"
#include "Engine/Engine.h"

#include "DAVAEngine.h"

using namespace DAVA;

EditorFontSystem::EditorFontSystem(QObject* parent)
    : QObject(parent)
    , defaultFontLocale("default")
    , currentFontLocale(defaultFontLocale)
{
}

EditorFontSystem::~EditorFontSystem()
{
    ClearAllFonts();
}

Font* EditorFontSystem::GetFont(const String& presetName, const String& locale) const
{
    auto fontsIt = localizedFonts.find(locale);
    if (fontsIt == localizedFonts.end())
    {
        return nullptr;
    }
    const auto& fonts = fontsIt->second;
    auto it = fonts.find(presetName);
    return it != fonts.end() ? it->second : nullptr;
}

void EditorFontSystem::SetFont(const String& presetName, const String& locale, Font* font)
{
    if (nullptr == font)
    {
        DVASSERT(false, "wrong argument: font = nullptr");
        return;
    }
    auto fonstIt = localizedFonts.find(locale);
    if (fonstIt == localizedFonts.end())
    {
        DVASSERT(false, Format("wrong argument: locale = %s passed to this function not found", locale.c_str()).c_str());
        return;
    }
    auto& fonts = fonstIt->second;
    auto it = fonts.find(presetName);

    if (it == fonts.end())
    {
        DVASSERT(false, Format("wrong argument: presetName = %s passed to this function not found for locale %s", presetName.c_str(), locale.c_str()).c_str());
        return;
    }

    auto oldFont = it->second;
    DVASSERT(nullptr != oldFont);
    oldFont->Release();
    it->second = font;
    it->second->Retain();
    if (locale == currentFontLocale)
    {
        const EngineContext* engineContext = GetEngineContext();
        FontManager* fontManager = engineContext->fontManager;
        fontManager->UnregisterFont(oldFont);
        fontManager->RegisterFont(font);
        fontManager->SetFontName(font, presetName);
        emit FontPresetChanged(presetName);
    }
    font->Release();
}

void EditorFontSystem::LoadLocalizedFonts()
{
    ClearAllFonts();
    RefreshAvailableFontLocales();
    const EngineContext* engineContext = GetEngineContext();
    FontManager* fontManager = engineContext->fontManager;
    fontManager->UnregisterFonts();
    for (auto& locale : availableFontLocales)
    {
        UIYamlLoader::LoadFonts(GetLocalizedFontsPath(locale.toStdString()));
        for (auto& pair : fontManager->GetRegisteredFonts())
        {
            localizedFonts[locale.toStdString()][pair.second] = SafeRetain(pair.first);
        }
        fontManager->UnregisterFonts();
    }
    UIYamlLoader::LoadFonts(GetDefaultFontsPath());
    for (auto& pair : fontManager->GetRegisteredFonts())
    {
        defaultPresetNames.append(QString::fromStdString(pair.second));
        localizedFonts[defaultFontLocale][pair.second] = SafeRetain(pair.first);
    }
    //now check that all font are correct
    for (auto& pair : localizedFonts[defaultFontLocale])
    {
        for (auto& locale : availableFontLocales)
        {
            const auto& localizedMap = localizedFonts.at(locale.toStdString());
            DVASSERT(localizedMap.find(pair.first) != localizedMap.end());
        }
    }
    defaultPresetNames.sort();
    RegisterCurrentLocaleFonts();
}

void EditorFontSystem::SaveLocalizedFonts()
{
    const EngineContext* engineContext = GetEngineContext();
    FontManager* fontManager = engineContext->fontManager;
    FileSystem* fileSystem = engineContext->fileSystem;
    for (auto& localizedFontsIt : localizedFonts)
    {
        fontManager->RegisterFonts(localizedFontsIt.second);
        //load localized fonts into FontManager
        const FilePath& localizedFontsPath = GetLocalizedFontsPath(localizedFontsIt.first);
        if (!fileSystem->IsDirectory(localizedFontsPath.GetDirectory()))
        {
            fileSystem->CreateDirectory(localizedFontsPath.GetDirectory());
        }
        UIYamlLoader::SaveFonts(localizedFontsPath);
    }
    RegisterCurrentLocaleFonts();
}

void EditorFontSystem::ClearAllFonts()
{
    for (auto& map : localizedFonts)
    {
        ClearFonts(map.second);
    }
    localizedFonts.clear();
    defaultPresetNames.clear();
}

void EditorFontSystem::RegisterCurrentLocaleFonts()
{
    const EngineContext* engineContext = GetEngineContext();
    const auto& locale = engineContext->localizationSystem->GetCurrentLocale();
    currentFontLocale = availableFontLocales.contains(QString::fromStdString(locale)) ? locale : defaultFontLocale;
    auto it = localizedFonts.find(currentFontLocale);
    const auto& fonts = it != localizedFonts.end() ? it->second : localizedFonts.at(defaultFontLocale);
    engineContext->fontManager->RegisterFonts(fonts);
    emit FontPresetChanged(String());
}

void EditorFontSystem::ClearFonts(Map<String, Font*>& fonts)
{
    for (auto& pair : fonts)
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

void EditorFontSystem::RefreshAvailableFontLocales()
{
    availableFontLocales.clear();
    if (!defaultFontsPath.IsDirectoryPathname())
    {
        return;
    }

    FileList* fileList = new FileList(defaultFontsPath);
    for (uint32 count = fileList->GetCount(), i = 0u; i < count; ++i)
    {
        if (fileList->IsDirectory(i) && !fileList->IsNavigationDirectory(i))
        {
            availableFontLocales.push_back(QString::fromStdString(fileList->GetFilename(i)));
        }
    }
    SafeRelease(fileList);
}

void EditorFontSystem::SetDefaultFontsPath(const FilePath& path)
{
    defaultFontsPath = path.GetType() == FilePath::PATH_IN_RESOURCES ? path.GetAbsolutePathname() : path;
    RefreshAvailableFontLocales();
}

FilePath EditorFontSystem::GetDefaultFontsPath() const
{
    return defaultFontsPath + "fonts.yaml";
}

FilePath EditorFontSystem::GetLocalizedFontsPath(const String& locale) const
{
    return locale == defaultFontLocale ? GetDefaultFontsPath() : (defaultFontsPath + locale + "/fonts.yaml");
}

void EditorFontSystem::CreateNewPreset(const String& originalPresetName, const String& newPresetName)
{
    DVASSERT(localizedFonts[defaultFontLocale].size() > 0);
    const auto& presetName = defaultPresetNames.contains(QString::fromStdString(originalPresetName)) ? originalPresetName : localizedFonts[defaultFontLocale].begin()->first;
    for (auto& localizedFontsPairs : localizedFonts)
    {
        auto& fonts = localizedFontsPairs.second;
        fonts[newPresetName] = fonts.at(presetName)->Clone();
    }
    defaultPresetNames.append(QString::fromStdString(newPresetName));
    defaultPresetNames.sort();
}
