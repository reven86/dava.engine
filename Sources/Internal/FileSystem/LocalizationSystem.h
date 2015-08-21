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
	
	void InitWithDirectory(const FilePath &directoryPath);
	void SetDirectory(const FilePath &directoryPath);

	void Init();
	
	const String &GetCurrentLocale() const;
	void SetCurrentLocale(const String &newLangId);
    String GetDeviceLocale() const;
    
    String GetCountryCode() const;
	
	WideString GetLocalizedString(const WideString & key) const;
    WideString GetLocalizedString(const WideString & key, const String &langId) const;
	void SetLocalizedString(const WideString & key, const WideString & value);
	void RemoveLocalizedString(const WideString & key);

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
		String languageCode;// in ISO 639-1, like en,ru,uk
		String localeCode;// like en_US, ru_RU
	};
    static const Vector<LanguageLocalePair> languageLocaleMap;
    
	void LoadStringFile(const String & langId, const FilePath & fileName);
	void UnloadStringFile(const FilePath & fileName); 

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
	StringFile* LoadFromYamlFile(const String & langID, const FilePath & fileName);
	bool SaveToYamlFile(const StringFile* stringFile);

	YamlParser::YamlDataHolder *dataHolder;
};

inline WideString LocalizedString(const WideString & key)
{
	return LocalizationSystem::Instance()->GetLocalizedString(key);
}
inline WideString LocalizedString(const String & key)
{
	return LocalizationSystem::Instance()->GetLocalizedString(WideString(key.begin(), key.end()));
}
	
	
};

#endif // __DAVAENGINE_LOCALIZATION_SYSTEM_H__