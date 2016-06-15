#ifndef __DAVAENGINE_LOCALIZATION_SYSTEM_H__
#define __DAVAENGINE_LOCALIZATION_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"

#include "FileSystem/YamlParser.h"

namespace DAVA
{
class LocalizationSystem : public Singleton<LocalizationSystem>
{
public:
    static const char* DEFAULT_LOCALE;

    LocalizationSystem();
    virtual ~LocalizationSystem();

    void InitWithDirectory(const FilePath& directoryPath);

    void Init();

    const String& GetCurrentLocale() const;
    void SetCurrentLocale(const String& newLangId);
    String GetDeviceLocale() const;

    String GetCountryCode() const;

    WideString GetLocalizedString(const WideString& key) const;
    WideString GetLocalizedString(const WideString& key, const String& langId) const;
    void SetLocalizedString(const WideString& key, const WideString& value);
    void RemoveLocalizedString(const WideString& key);

    void SetDirectory(const FilePath& dirPath);
    const FilePath& GetDirectoryPath() const;

    void Cleanup();

    // Access to the whole strings list for the current locale.
    // Returns FALSE if no strings found.
    bool GetStringsForCurrentLocale(Map<WideString, WideString>& strings) const;

    // Save the current localization data to the files they were loaded from.
    bool SaveLocalizedStrings();

private:
    struct LanguageLocalePair
    {
        String languageCode; // in ISO 639-1, like en,ru,uk
        String localeCode; // like en_US, ru_RU
    };
    static const Vector<LanguageLocalePair> languageLocaleMap;

    void LoadStringFile(const String& langId, const FilePath& fileName);
    void UnloadStringFile(const FilePath& fileName);

    String langId;
    FilePath directoryPath;

    struct StringFile
    {
        FilePath pathName;
        String langId;
        Map<WideString, WideString> strings;
    };
    List<StringFile*> stringsList;

    // Load/Save functionality.
    StringFile* LoadFromYamlFile(const String& langID, const FilePath& fileName);
    bool SaveToYamlFile(const StringFile* stringFile);

    YamlParser::YamlDataHolder* dataHolder;
};

inline WideString LocalizedString(const WideString& key)
{
    return LocalizationSystem::Instance()->GetLocalizedString(key);
}
inline WideString LocalizedString(const String& key)
{
    return LocalizationSystem::Instance()->GetLocalizedString(WideString(key.begin(), key.end()));
}
};

#endif // __DAVAENGINE_LOCALIZATION_SYSTEM_H__