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


#include "FileSystem/LocalizationSystem.h"
#include "Utils/Utils.h"
#include "FileSystem/Logger.h"
#include "yaml/yaml.h"
#include "Utils/UTF8Utils.h"
#include "Debug/DVAssert.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/YamlEmitter.h"
#include "Sound/SoundSystem.h"
#if defined(__DAVAENGINE_IPHONE__)
#include "FileSystem/LocalizationIPhone.h"
#elif defined(__DAVAENGINE_ANDROID__)
#include "FileSystem/LocalizationAndroid.h"
#elif defined(__DAVAENGINE_WIN_UAP__)
#include "FileSystem/LocalizationWinUAP.h"
#else
#include "Core/Core.h"
#endif


namespace DAVA 
{
//TODO: move it to DateTimeWin32 or remove
const Vector<LocalizationSystem::LanguageLocalePair> LocalizationSystem::languageLocaleMap =
{
    { "en", "en_US" },
    { "ru", "ru_RU" },
    { "de", "de_DE" },
    { "it", "it_IT" },
    { "fr", "fr_FR" },
    { "es", "es_ES" },
    { "zh", "zh_CN" },
    { "ja", "ja_JP" },
    { "uk", "uk_UA" }
};

const char* LocalizationSystem::DEFAULT_LOCALE = "en";


LocalizationSystem::LocalizationSystem()
{
	langId = DEFAULT_LOCALE;

	dataHolder = new YamlParser::YamlDataHolder();
	dataHolder->data = 0;
}

LocalizationSystem::~LocalizationSystem()
{
    Cleanup();
	SafeDelete(dataHolder);
}
	
void LocalizationSystem::InitWithDirectory(const FilePath &directoryPath)
{
    SetDirectory(directoryPath);
    Init();
}

void LocalizationSystem::SetDirectory(const FilePath& dirPath)
{
    DVASSERT(dirPath.IsDirectoryPathname());
    directoryPath = dirPath;
#if defined(__DAVAENGINE_IPHONE__)
	LocalizationIPhone::SelectPreferedLocalizationForPath(directoryPath);
#elif defined(__DAVAENGINE_ANDROID__)
    LocalizationAndroid::SelectPreferedLocalization();
#elif defined(__DAVAENGINE_WIN_UAP__)
    LocalizationWinUAP::SelectPreferedLocalization();
#else
    String loc = Core::Instance()->GetOptions()->GetString("locale", DEFAULT_LOCALE);
    SetCurrentLocale(loc);
#endif
}

void LocalizationSystem::Init()
{
	LoadStringFile(langId, directoryPath + (langId + ".yaml"));
}   

String LocalizationSystem::GetDeviceLocale() const
{
#if defined(__DAVAENGINE_IPHONE__)
	return String(LocalizationIPhone::GetDeviceLang());
#elif defined(__DAVAENGINE_ANDROID__)
    return LocalizationAndroid::GetDeviceLang();
#elif defined(__DAVAENGINE_WIN_UAP__)
    return LocalizationWinUAP::GetDeviceLang();
#else
    return DEFAULT_LOCALE;
#endif
}
    
const String &LocalizationSystem::GetCurrentLocale() const
{
	return langId;
}
	
const FilePath &LocalizationSystem::GetDirectoryPath() const
{
    return directoryPath;
}

void LocalizationSystem::SetCurrentLocale(const String &requestedLangId)
{
    String actualLangId;
    
    FilePath localeFilePath(directoryPath + (requestedLangId + ".yaml"));
    if (FileSystem::Instance()->Exists(localeFilePath))
    {
        actualLangId = requestedLangId;
    }
    else if(requestedLangId.size() > 2)
    {
        String langPart = requestedLangId.substr(0, 2);
        String::size_type posPartStart = 3;
        // ex. zh-Hans, zh-Hans-CN, zh-Hans_CN, zh_Hans_CN, zh_CN, zh
        
        String::size_type posScriptEnd = requestedLangId.find('-', posPartStart);
        if(posScriptEnd == String::npos)
        {
            // ex. not zh-Hans-CN, but can be zh-Hans_CN
            posScriptEnd = requestedLangId.find('_', posPartStart);
        }
        
        if(posScriptEnd != String::npos)
        {
            // ex. zh-Hans-CN or zh-Hans_CN try zh-Hans
            String scriptPart = requestedLangId.substr(posPartStart, posScriptEnd - posPartStart);
#if defined(__DAVAENGINE_ANDROID__)
            if (scriptPart == "CN" || (langPart == "zh" && scriptPart == ""))
            {
                scriptPart = "Hans";
            }
            else if(scriptPart == "TW")
            {
                scriptPart = "Hant";
            }
#endif
            langPart = Format("%s-%s", langPart.c_str(), scriptPart.c_str());
        }
        
        Logger::FrameworkDebug("LocalizationSystem requested locale %s is not supported, trying to check part %s", requestedLangId.c_str(), langPart.c_str());
        localeFilePath = directoryPath + (langPart + ".yaml");
        if (FileSystem::Instance()->Exists(localeFilePath))
        {
            actualLangId = langPart;
        }
#if defined(__DAVAENGINE_ANDROID__)
        else if(langPart == "zh")
        {
            // in case zh is returned without country code and no zh.yaml is found - try zh-Hans
            langPart = "zh-Hans";
            localeFilePath = directoryPath + (langPart + ".yaml");
            if(localeFilePath.Exists())
            {
                actualLangId = langPart;
            }
        }
#endif
    }
    
    if(actualLangId.empty())
    {
        localeFilePath = directoryPath + (String(DEFAULT_LOCALE) + ".yaml");
        if (FileSystem::Instance()->Exists(localeFilePath))
        {
            actualLangId = DEFAULT_LOCALE;
        }
        else
        {
            Logger::Warning("LocalizationSystem requested locale %s is not supported, failed to set default lang, locale will not be changed", requestedLangId.c_str(), actualLangId.c_str());
            return;
        }
    }
    
    //TODO: add reloading strings data on langId changing
    Logger::FrameworkDebug("LocalizationSystem requested locale: %s, set locale: %s", requestedLangId.c_str(), actualLangId.c_str());
    langId = actualLangId;
    SoundSystem::Instance()->SetCurrentLocale(langId);
}
	
LocalizationSystem::StringFile * LocalizationSystem::LoadFromYamlFile(const String & langID, const FilePath & pathName)
{
	yaml_parser_t parser;
	yaml_event_t event;
	
	int done = 0;
	
	/* Create the Parser object. */
	yaml_parser_initialize(&parser);
	
	yaml_parser_set_encoding(&parser, YAML_UTF8_ENCODING);
	
	File * yamlFile = File::Create(pathName, File::OPEN | File::READ);
    if(!yamlFile) return NULL;
    
	dataHolder->fileSize = yamlFile->GetSize();
	dataHolder->data = new uint8[dataHolder->fileSize];
	dataHolder->dataOffset = 0;
	yamlFile->Read(dataHolder->data, dataHolder->fileSize);
	yamlFile->Release();
	
	yaml_parser_set_input(&parser, read_handler, dataHolder);
	
	WideString key;
	WideString value;
	bool isKey = true;
	StringFile * strFile = new StringFile();
	
	/* Read the event sequence. */
	while (!done) 
	{
		
		/* Get the next event. */
		if (!yaml_parser_parse(&parser, &event))
		{
			Logger::Error("parsing error: type: %d %s line: %d pos: %d", parser.error, parser.problem, parser.problem_mark.line, parser.problem_mark.column);
			SafeDelete(strFile);
			break;
		}
				
		switch(event.type)
		{
			case YAML_ALIAS_EVENT:
				//Logger::FrameworkDebug("alias: %s", event.data.alias.anchor);
				break;
				
			case YAML_SCALAR_EVENT:
			{
				const uint8* str = reinterpret_cast<uint8*>(event.data.scalar.value);
				size_t size = static_cast<size_t>(event.data.scalar.length);
				if (isKey)
				{
					UTF8Utils::EncodeToWideString(str, size, key);
				}else 
				{
					UTF8Utils::EncodeToWideString(str, size, value);
					strFile->strings[key] = value;
				}
				
				isKey = !isKey;
			}
			break;
				
			case YAML_DOCUMENT_START_EVENT:
			{
				//Logger::FrameworkDebug("document start:");
			}break;
				
			case YAML_DOCUMENT_END_EVENT:
			{	//Logger::FrameworkDebug("document end:");
			}
				break;
				
			case YAML_SEQUENCE_START_EVENT:
			{
			}break;
				
			case YAML_SEQUENCE_END_EVENT:
			{
			}break;
				
			case YAML_MAPPING_START_EVENT:
			{
			}
			break;
				
			case YAML_MAPPING_END_EVENT:
			{
			}
			break;
                default:
                break;
		};
		
		/* Are we finished? */
		done = (event.type == YAML_STREAM_END_EVENT);
		
		/* The application is responsible for destroying the event object. */
		yaml_event_delete(&event);
		
	}
	
	yaml_parser_delete(&parser);	
	if (strFile)
	{
		strFile->pathName = pathName;
		strFile->langId = langID;
	}

	SafeDeleteArray(dataHolder->data);
	return strFile;
}
	
bool LocalizationSystem::SaveToYamlFile(const StringFile* stringFile)
{
	if (!stringFile)
	{
		return false;
	}

	YamlNode *node = YamlNode::CreateMapNode(true, YamlNode::MR_BLOCK_REPRESENTATION, YamlNode::SR_DOUBLE_QUOTED_REPRESENTATION);
	for (auto iter = stringFile->strings.begin(); iter != stringFile->strings.end(); ++iter)
	{
		node->Add(UTF8Utils::EncodeToUTF8(iter->first), iter->second);
	}
	
	bool result = YamlEmitter::SaveToYamlFile(stringFile->pathName, node);

	SafeRelease(node);
	return result;
}

void LocalizationSystem::LoadStringFile(const String & langID, const FilePath & fileName)
{
	StringFile * file = LoadFromYamlFile(langID, fileName);
	if (file)
	{
		stringsList.push_back(file);
	}	
}
	
void LocalizationSystem::UnloadStringFile(const FilePath & fileName)
{
	DVASSERT(0 && "Method do not implemented");
}

WideString LocalizationSystem::GetLocalizedString(const WideString & key) const
{
	for (auto it = stringsList.rbegin(); it != stringsList.rend(); ++it)
	{
		StringFile * file = *it;

		auto res = file->strings.find(key);
		if (res != file->strings.end())
		{
			return res->second;
		}
	}
	return key;
}

WideString LocalizationSystem::GetLocalizedString(const WideString & key, const String &langId) const
{
    for (auto it = stringsList.rbegin(); it != stringsList.rend(); ++it)
    {
        StringFile * file = *it;

        if(file->langId.compare(langId) == 0)
        {
            auto res = file->strings.find(key);
            if (res != file->strings.end())
            {
                return res->second;
            }
        }
    }
    return key;
}

void LocalizationSystem::SetLocalizedString(const WideString & key, const WideString & value)
{
	// Update in all files currently loaded.
    for (auto it = stringsList.rbegin(); it != stringsList.rend(); ++it)
	{
		StringFile * file = *it;
		file->strings[key] = value;
	}
}

void LocalizationSystem::RemoveLocalizedString(const WideString & key)
{
	// Update in all files currently loaded.
    for (auto it = stringsList.rbegin(); it != stringsList.rend(); ++it)
	{
		StringFile * file = *it;
		file->strings.erase(key);
	}
}

bool LocalizationSystem::SaveLocalizedStrings()
{
	bool saveResult = true;
    for (auto it = stringsList.rbegin(); it != stringsList.rend(); ++it)
	{
		StringFile * file = *it;
		saveResult &= SaveToYamlFile(file);
	}
	
	return saveResult;
}
	
void LocalizationSystem::Cleanup()
{
	// release all memory allocated by strings
    for (auto it = stringsList.rbegin(); it != stringsList.rend(); ++it)
	{
		StringFile * file = *it;
		SafeDelete(file);
	}
	stringsList.clear();
    
    directoryPath = FilePath();
    langId.clear();
	SafeDeleteArray(dataHolder->data);
}

bool LocalizationSystem::GetStringsForCurrentLocale(Map<WideString, WideString>& strings) const
{
    for (auto iter = stringsList.begin(); iter != stringsList.end();
		 ++iter)
	{
		if ((*iter)->langId == GetCurrentLocale())
		{
			strings = (*iter)->strings;
			return true;
		}
	}
	
	// No strings found.
	return false;
}
    
String LocalizationSystem::GetCountryCode() const
{
    auto iter = std::find_if(languageLocaleMap.begin(), languageLocaleMap.end(), [&](const LocalizationSystem::LanguageLocalePair & langPair)
    {
        return langPair.languageCode == langId;
    });

    if (iter != languageLocaleMap.end())
    {
        return (*iter).localeCode;
    }

    return "en_US";
}
	
};
